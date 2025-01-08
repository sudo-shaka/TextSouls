#include "cgltf.h"
#include "vertexMath.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

cgltf_data* processGltf(const char *filename){
  cgltf_options options ={0};
  cgltf_data* data=NULL;
  if (cgltf_parse_file(&options, filename, &data) != cgltf_result_success) {
      fprintf(stderr, "Failed to parse GLB file.\n");
      return NULL;
  }

  // Load buffers
  if (cgltf_load_buffers(&options, data, "filename") != cgltf_result_success) {
      fprintf(stderr, "Failed to load buffers from GLB.\n");
      cgltf_free(data);
      return NULL;
  }
  return data;
}

void getInterpolatedValue(cgltf_animation_sampler* sampler, float current_time, float* result, int count){
  float time_prev = 0, time_next = 0;
  float value_prev[4] = {0}, value_next[4] = {0};

  // Find the keyframe indices
  int index = 0;
  for (; index < sampler->input->count - 1; index++) {
    cgltf_accessor_read_float(sampler->input, index, &time_prev, 1);
    cgltf_accessor_read_float(sampler->input, index + 1, &time_next, 1);
    if (current_time >= time_prev && current_time <= time_next) break;
  }

  // Read values
  cgltf_accessor_read_float(sampler->output, index, value_prev, count);
  cgltf_accessor_read_float(sampler->output, index + 1, value_next, count);

  // Interpolate
  float t = (current_time - time_prev) / (time_next - time_prev);
  if (count == 4) { // Quaternion (rotation)
    sphericalLinInterp(result, value_prev, value_next, t);
  } else { // Translation or scale
    linearInterolation(result, value_prev, value_next, t, count);
  }
}

void applyAnimation(cgltf_data* data, cgltf_animation* animation ,float current_time){
  if(!animation){
    fprintf(stderr,"stderr: Animation is NULL\n");
    return;
  }
  for (cgltf_size i = 0; i < animation->channels_count; i++) {
    cgltf_animation_channel* channel = &animation->channels[i];
    cgltf_animation_sampler* sampler = channel->sampler;

    float result[4] = {0};
    size_t value_count = sampler->output->stride / sizeof(float);
    getInterpolatedValue(sampler, current_time, result, value_count);

    // Update the target node's transformation
    cgltf_node* node = channel->target_node;
    if (channel->target_path == cgltf_animation_path_type_translation) {
        for (size_t k = 0; k < 3; k++) {
            node->translation[k] = result[k];
        }
    } else if (channel->target_path == cgltf_animation_path_type_rotation) {
        for (size_t k = 0; k < 4; k++) {
            node->rotation[k] = result[k];
        }
    } else if (channel->target_path == cgltf_animation_path_type_scale) {
        for (size_t k = 0; k < 3; k++) {
            node->scale[k] = result[k];
      }
    }
  }
}

void clearPlayerData(cgltf_data* data){
  cgltf_free(data);
}

int getNumVerts(cgltf_data* data){
  int totalVerts=0;
  if(data == NULL){
    fprintf(stderr,"error getting vert data");
    return 0;
  }
  for (cgltf_size mesh_index = 0; mesh_index < data->meshes_count; ++mesh_index) {
    cgltf_mesh* mesh = &data->meshes[mesh_index];
    for (cgltf_size prim_index = 0; prim_index < mesh->primitives_count; ++prim_index) {
      cgltf_primitive* prim = &mesh->primitives[prim_index];
      for (cgltf_size attr_index = 0; attr_index < prim->attributes_count; ++attr_index) {
        cgltf_attribute* attr = &prim->attributes[attr_index];
        if (attr->type == cgltf_attribute_type_position) {
            totalVerts += attr->data->count;
        }
      }
    }
  }
  return totalVerts;
}

void getPlayerVerts(cgltf_data* data,vec3* Pos){
  int currentVert = 0;
  for (cgltf_size mesh_index = 0; mesh_index < data->meshes_count; ++mesh_index) {
    cgltf_mesh* mesh = &data->meshes[mesh_index];
    for (cgltf_size prim_index = 0; prim_index < mesh->primitives_count; ++prim_index) {
      cgltf_primitive* prim = &mesh->primitives[prim_index];
      for (cgltf_size attr_index = 0; attr_index < prim->attributes_count; ++attr_index) {
        cgltf_attribute* attr = &prim->attributes[attr_index];
        if (attr->type == cgltf_attribute_type_position) {
          cgltf_accessor* accessor = attr->data;
          cgltf_buffer_view* buffer_view = accessor->buffer_view;
          const uint8_t* buffer_data = (const uint8_t*)buffer_view->buffer->data;
          const uint8_t* vertex_data_ptr = buffer_data + buffer_view->offset + accessor->offset;

          for (cgltf_size i = 0; i < accessor->count; ++i) {
              const float* position = (const float*)(vertex_data_ptr + i * accessor->stride);
              vec3 currPos = {position[0],position[1],position[2]};
              Pos[currentVert] = currPos;
              currentVert++;
          }
        }
      }
    }
  }
}
cgltf_animation* findAnimationName(cgltf_data* data, const char* name){
    for (cgltf_size i = 0; i < data->animations_count; ++i) {
        if (data->animations[i].name && strcmp(data->animations[i].name, name) == 0) {
            return &data->animations[i];
        }
    }
    return NULL; // Animation not found
}

void mat4_multiply(const float* a, const float* b, float* result) {
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            result[row * 4 + col] = a[row * 4 + 0] * b[0 * 4 + col] +
                                    a[row * 4 + 1] * b[1 * 4 + col] +
                                    a[row * 4 + 2] * b[2 * 4 + col] +
                                    a[row * 4 + 3] * b[3 * 4 + col];
        }
    }
}

void calculate_joint_matrices(const cgltf_skin* skin, const float* node_world_matrices, float* joint_matrices) {
    // Loop through each joint in the skin
    for (cgltf_size i = 0; i < skin->joints_count; ++i) {
        cgltf_node* joint_node = skin->joints[i];

        // Get the world matrix of the joint
        float joint_world_matrix[16];
        memcpy(joint_world_matrix, &node_world_matrices[(joint_node - skin->joints[0]) * 16], sizeof(float) * 16);

        // Get the inverse bind matrix for the joint
        float inverse_bind_matrix[16];
        cgltf_accessor_read_float(skin->inverse_bind_matrices, i, inverse_bind_matrix, 16);

        // Multiply the world matrix by the inverse bind matrix
        mat4_multiply(joint_world_matrix, inverse_bind_matrix, &joint_matrices[i * 16]);
    }
}

// Compute world matrix for a node
void compute_node_world_matrix(cgltf_node* node, const float* parent_world_matrix, float* world_matrix) {
    float local_matrix[16];
    cgltf_node_transform_local(node, local_matrix);

    if (parent_world_matrix) {
        mat4_multiply(parent_world_matrix, local_matrix, world_matrix);
    } else {
        memcpy(world_matrix, local_matrix, sizeof(float) * 16);
    }
}

// Compute world matrices for all nodes
void compute_all_world_matrices(cgltf_data* data, float* node_world_matrices) {
    for (cgltf_size i = 0; i < data->nodes_count; ++i) {
        cgltf_node* node = &data->nodes[i];
        float* parent_world_matrix = node->parent ? &node_world_matrices[(node->parent - data->nodes) * 16] : NULL;
        compute_node_world_matrix(node, parent_world_matrix, &node_world_matrices[i * 16]);
    }
}

// Apply skinning to transform vertex positions
void apply_skinning(cgltf_accessor* position_accessor, cgltf_accessor* joints_accessor,
                    cgltf_accessor* weights_accessor, float* joint_matrices,
                    float* output_positions, size_t vertex_count) {
    for (cgltf_size i = 0; i < vertex_count; ++i) {
        float position[3];
        cgltf_accessor_read_float(position_accessor, i, position, 3);

        unsigned int joints[4];
        cgltf_accessor_read_uint(joints_accessor, i, joints, 4);

        float weights[4];
        cgltf_accessor_read_float(weights_accessor, i, weights, 4);

        float transformed_position[3] = {0, 0, 0};
        for (int j = 0; j < 4; ++j) {
            float weight = weights[j];
            if (weight > 0) {
                const float* matrix = &joint_matrices[joints[j] * 16];
                for (int k = 0; k < 3; ++k) {
                    transformed_position[k] += weight * (matrix[k * 4 + 0] * position[0] +
                                                         matrix[k * 4 + 1] * position[1] +
                                                         matrix[k * 4 + 2] * position[2] +
                                                         matrix[k * 4 + 3]);
                }
            }
        }
        output_positions[i * 3 + 0] = transformed_position[0];
        output_positions[i * 3 + 1] = transformed_position[1];
        output_positions[i * 3 + 2] = transformed_position[2];
    }
}

// Extract animated vertex positions for rendering
void extract_animated_vertex_positions(cgltf_data* data, vec3* output_positions) {
    size_t node_count = data->nodes_count;
    float* node_world_matrices = malloc(node_count * 16 * sizeof(float));
    compute_all_world_matrices(data, node_world_matrices);

    for (cgltf_size node_index = 0; node_index < node_count; ++node_index) {
        cgltf_node* node = &data->nodes[node_index];
        if (!node->mesh) {
            continue; // Skip nodes without meshes
        }

        cgltf_mesh* mesh = node->mesh;
        float* node_world_matrix = &node_world_matrices[node_index * 16];

        for (cgltf_size prim_index = 0; prim_index < mesh->primitives_count; ++prim_index) {
            cgltf_primitive* prim = &mesh->primitives[prim_index];

            cgltf_accessor* position_accessor = NULL;
            cgltf_accessor* joints_accessor = NULL;
            cgltf_accessor* weights_accessor = NULL;

            for (cgltf_size attr_index = 0; attr_index < prim->attributes_count; ++attr_index) {
                cgltf_attribute* attr = &prim->attributes[attr_index];
                if (attr->type == cgltf_attribute_type_position) {
                    position_accessor = attr->data;
                } else if (attr->type == cgltf_attribute_type_joints) {
                    joints_accessor = attr->data;
                } else if (attr->type == cgltf_attribute_type_weights) {
                    weights_accessor = attr->data;
                }
            }

            if (!position_accessor) {
                fprintf(stderr, "No POSITION attribute found for primitive %zu.\n", prim_index);
                continue;
            }

            cgltf_size vertex_count = position_accessor->count;
            float* skinned_positions = malloc(vertex_count * 3 * sizeof(float));

            if (node->skin && joints_accessor && weights_accessor) {
                float* joint_matrices = malloc(node->skin->joints_count * 16 * sizeof(float));
                calculate_joint_matrices(node->skin, node_world_matrices, joint_matrices);
                apply_skinning(position_accessor, joints_accessor, weights_accessor, joint_matrices, skinned_positions, vertex_count);
                free(joint_matrices);
            } else {
                // Transform positions without skinning
                for (cgltf_size i = 0; i < vertex_count; ++i) {
                    float position[3];
                    cgltf_accessor_read_float(position_accessor, i, position, 3);

                    float transformed_position[3] = {0};
                    for (int j = 0; j < 3; ++j) {
                        transformed_position[j] = node_world_matrix[j * 4 + 0] * position[0] +
                                                  node_world_matrix[j * 4 + 1] * position[1] +
                                                  node_world_matrix[j * 4 + 2] * position[2] +
                                                  node_world_matrix[j * 4 + 3];
                    }

                    skinned_positions[i * 3 + 0] = transformed_position[0];
                    skinned_positions[i * 3 + 1] = transformed_position[1];
                    skinned_positions[i * 3 + 2] = transformed_position[2];
                }
            }

            // Store skinned positions in the output array
            int pi=0;
            for (cgltf_size i = 0; i < vertex_count; i+=3) {
              vec3 p = {skinned_positions[0],skinned_positions[1],skinned_positions[2]};
              output_positions[pi] = p;
              pi++;
            }

            free(skinned_positions);
        }
    }

    free(node_world_matrices);
}


vec3 getLtfCOM(cgltf_data* data){
  vec3 COM = {0.0f};
  int currentVert = 0;
  for (cgltf_size mesh_index = 0; mesh_index < data->meshes_count; ++mesh_index) {
    cgltf_mesh* mesh = &data->meshes[mesh_index];
    for (cgltf_size prim_index = 0; prim_index < mesh->primitives_count; ++prim_index) {
      cgltf_primitive* prim = &mesh->primitives[prim_index];
      for (cgltf_size attr_index = 0; attr_index < prim->attributes_count; ++attr_index) {
        cgltf_attribute* attr = &prim->attributes[attr_index];
        if (attr->type == cgltf_attribute_type_position) {
          cgltf_accessor* accessor = attr->data;
          cgltf_buffer_view* buffer_view = accessor->buffer_view;
          const uint8_t* buffer_data = (const uint8_t*)buffer_view->buffer->data;
          const uint8_t* vertex_data_ptr = buffer_data + buffer_view->offset + accessor->offset;

          for (cgltf_size i = 0; i < accessor->count; ++i) {
              const float* position = (const float*)(vertex_data_ptr + i * accessor->stride);
              vec3 currPos = {position[0],position[1],position[2]};
              COM.x += currPos.x;
              COM.y += currPos.y;
              COM.z += currPos.z;
              currentVert++;
          }
        }
      }
    }
    COM.x /= (float)currentVert;
    COM.y /= (float)currentVert;
    COM.z /= (float)currentVert;
  }
  return COM;
}
