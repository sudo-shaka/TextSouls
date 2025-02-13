#include "cgltf.h"
#include "vertexMath.h"
#include "player.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <omp.h>

player initPlayer(int Health, int Endurance, cgltf_data *inputData){
  player p;
  p.maxHealth = Health;
  p.currentHeath = Health;
  p.maxEndurance = Endurance;
  p.currEndurance = Endurance;
  p.data = inputData;
  p.numFaces = count_faces(inputData);
  p.numVerts = count_verts(inputData);
  p.verts = malloc(p.numVerts * sizeof(vec3));
  p.displayChar = malloc(p.numVerts * sizeof(char));
  p.faces = malloc(p.numFaces * sizeof(face));
  extract_face_indices(inputData, p.faces);
  p.position = getLtfCOM(inputData);
  p.lockedon = 1;
  p.currentAnimation = findAnimationName(inputData, "Idol");
  p.translation = extract_translations(inputData);
  return p;
};

void closePlayer(player p){
  cgltf_free(p.data);
  free(p.faces);
  free(p.verts);
  free(p.displayChar);
  free(p.translation);
};

cgltf_data *processGltf(const char *filename){
  cgltf_options options = {0};
  cgltf_data *data = NULL;
  if (cgltf_parse_file(&options, filename, &data) != cgltf_result_success)
  {
    fprintf(stderr, "Failed to parse GLB file.\n");
    return NULL;
  }

  // Load buffers
  if (cgltf_load_buffers(&options, data, filename) != cgltf_result_success)
  {
    fprintf(stderr, "Failed to load buffers from GLB.\n");
    cgltf_free(data);
    return NULL;
  }
  return data;
}

void interpolate(cgltf_animation_sampler *sampler, float current_time, float *result, int count){
  float time_prev = 0, time_next = 0;
  float value_prev[4] = {0}, value_next[4] = {0};

  // Find the keyframe indices
  int index = 0;
  for (; index < sampler->input->count - 1; index++)
  {
    cgltf_accessor_read_float(sampler->input, index, &time_prev, 1);
    cgltf_accessor_read_float(sampler->input, index + 1, &time_next, 1);
    if (current_time >= time_prev && current_time <= time_next)
      break;
  }

  // Read values
  cgltf_accessor_read_float(sampler->output, index, value_prev, count);
  cgltf_accessor_read_float(sampler->output, index + 1, value_next, count);

  // Interpolate
  float t = (current_time - time_prev) / (time_next - time_prev);

  switch (sampler->interpolation)
  {
  case cgltf_interpolation_type_linear:
    (count == 4) ? slerp(result, value_prev, value_next, t) : lerp(result, value_prev, value_next, t, count);
    break;
  case cgltf_interpolation_type_step:
    memcpy(result, value_prev, sizeof(float) * count);
    break;
  case cgltf_interpolation_type_cubic_spline:
    // cubicSpline(result,time_prev,t);
    break;
  default:
    break;
  }
}

void apply_animation(player *p){
  cgltf_animation * animation = p->currentAnimation;
  if (!animation)
  {
    fprintf(stderr, "stderr: Animation is NULL\n");
    return;
  }
  for (cgltf_size i = 0; i < animation->channels_count; i++)
  {

    cgltf_animation_channel *channel = &animation->channels[i];
    cgltf_node *node = channel->target_node;
    cgltf_animation_sampler *sampler = channel->sampler;
    if (!channel->sampler || !node)
    {
      continue;
    }

    float result[4] = {0};
    size_t component_count = sampler->output->stride / sizeof(float);
    interpolate(sampler, p->animation_time, result, component_count);

    // Update the target node's transformation
    switch (channel->target_path){
      case cgltf_animation_path_type_translation:
        memcpy(node->translation, result, sizeof(float) * component_count);
        break;
      case cgltf_animation_path_type_rotation:
        memcpy(node->rotation, result, sizeof(float) * component_count);
        break;
      case cgltf_animation_path_type_scale:
        memcpy(node->scale, result, sizeof(float) * component_count);
        break;
      default:
        break;
    }
  }
}

float calculate_total_animation_time(const cgltf_animation *animation){
  if (animation == NULL)
  {
    return 0.0f;
  }
  float total_length = 0.0f;
  // Iterate through each channel of the animation
  for (cgltf_size j = 0; j < animation->channels_count; ++j)
  {
    const cgltf_animation_channel *channel = &animation->channels[j];
    const cgltf_animation_sampler *sampler = channel->sampler;

    if (sampler && sampler->input)
    {
      // Get the maximum time value from the input accessor
      const cgltf_accessor *input = sampler->input;

      if (input->count > 0)
      {
        float start_time = 0.0f;
        float end_time = 0.0f;

        cgltf_accessor_read_float(input, 0, &start_time, 1);
        cgltf_accessor_read_float(input, input->count - 1, &end_time, 1);

        if (end_time > total_length)
        {
          total_length = end_time;
        }
      }
    }
  }

  return total_length;
}

int count_verts(cgltf_data *data){
  int totalVerts = 0;
  if (data == NULL)
  {
    fprintf(stderr, "error getting vert data");
    return 0;
  }
  for (cgltf_size mesh_index = 0; mesh_index < data->meshes_count; ++mesh_index)
  {
    cgltf_mesh *mesh = &data->meshes[mesh_index];
    for (cgltf_size prim_index = 0; prim_index < mesh->primitives_count; ++prim_index)
    {
      cgltf_primitive *prim = &mesh->primitives[prim_index];
      for (cgltf_size attr_index = 0; attr_index < prim->attributes_count; ++attr_index)
      {
        cgltf_attribute *attr = &prim->attributes[attr_index];
        if (attr->type == cgltf_attribute_type_position)
        {
          totalVerts += attr->data->count;
        }
      }
    }
  }
  return totalVerts;
}

void extract_static_player_verts(cgltf_data *data, vec3 *Pos){
  int currentVert = 0;
  for (cgltf_size mesh_index = 0; mesh_index < data->meshes_count; ++mesh_index)
  {
    cgltf_mesh *mesh = &data->meshes[mesh_index];
    if(!mesh) continue;
    for (cgltf_size prim_index = 0; prim_index < mesh->primitives_count; ++prim_index)
    {
      cgltf_primitive *prim = &mesh->primitives[prim_index];
      for (cgltf_size attr_index = 0; attr_index < prim->attributes_count; ++attr_index)
      {
        cgltf_attribute *attr = &prim->attributes[attr_index];
        if (attr->type == cgltf_attribute_type_position)
        {
          for (cgltf_size i = 0; i < attr->data->count; ++i)
          {
            float position[3];
            cgltf_accessor_read_float(attr->data, i, position,3);
            Pos[currentVert] = (vec3){position[0],position[1],position[2]};
            currentVert++;
          }
        }
      }
    }
  }
}

int count_faces(const cgltf_data *data){
  if (!data || !data->meshes) return 0;
  size_t total_faces = 0;
  // Iterate through all meshes
  #pragma omp simd
  for (cgltf_size mesh_index = 0; mesh_index < data->meshes_count; ++mesh_index)
  {
    const cgltf_mesh *mesh = &data->meshes[mesh_index];

    // Iterate through all primitives in the mesh
    for (cgltf_size prim_index = 0; prim_index < mesh->primitives_count; ++prim_index)
    {
      const cgltf_primitive *primitive = &mesh->primitives[prim_index];
      const cgltf_accessor *indices_accessor = primitive->indices;

      if (indices_accessor)
      {
        // Use the indices accessor to determine the number of faces
        total_faces += indices_accessor->count / 3;
      }
      else
      {
        // No indices accessor; calculate faces directly from vertex count
        const cgltf_accessor *position_accessor = NULL;

        // Find the position attribute
        for (cgltf_size attr_index = 0; attr_index < primitive->attributes_count; ++attr_index)
        {
          if (primitive->attributes[attr_index].type == cgltf_attribute_type_position)
          {
            position_accessor = primitive->attributes[attr_index].data;
            break;
          }
        }
        if (position_accessor)
        {
          total_faces += position_accessor->count / 3;
        }
      }
    }
  }

  return total_faces;
}

void extract_face_indices(const cgltf_data *data, face *faces){
  if (!data || !data->meshes)
  {
    printf("No meshes available in the glTF file.\n");
    return;
  }

  #pragma omp simd
  for (cgltf_size mesh_index = 0; mesh_index < data->meshes_count; ++mesh_index)
  {
    const cgltf_mesh *mesh = &data->meshes[mesh_index];
    // Iterate through all primitives in the mesh
    for (cgltf_size prim_index = 0; prim_index < mesh->primitives_count; ++prim_index){
      const cgltf_primitive *primitive = &mesh->primitives[prim_index];
      if (!primitive->indices){
        printf("Primitive %zu has no indices.\n", prim_index);
        continue;
      }
      if (primitive->type != cgltf_primitive_type_triangles){
        printf("  Primitive %zu is not a triangle.\n", prim_index);
        continue;
      }
      for (cgltf_size fi = 0; fi < primitive->indices->count/3; fi++){
        faces[fi].vertIdx[0] = cgltf_accessor_read_index(primitive->indices,fi * 3+0);
        faces[fi].vertIdx[1] = cgltf_accessor_read_index(primitive->indices,fi * 3+1);
        faces[fi].vertIdx[2] = cgltf_accessor_read_index(primitive->indices,fi * 3+2);
        faces[fi].vertCount = 3;
      }
    }
  }
}

cgltf_animation *findAnimationName(cgltf_data *data, const char *name){
  for (cgltf_size i = 0; i < data->animations_count; ++i)
  {
    if (data->animations[i].name && strcmp(data->animations[i].name, name) == 0)
    {
      return &data->animations[i];
    }
  }
  return NULL;
}
void set_animation(player * p, cgltf_animation * animation){
  p->currentAnimation = animation;
  p->animation_time = 0.0f;
  p->total_animation_time = calculate_total_animation_time(animation);
}


void calculate_joint_matrix(const cgltf_skin *skin, const int index,float *joint_matrix){
  // Get the world matrix of the joint
  float joint_transform[16];
  if(skin->joints[index]->has_matrix){
    memcpy(joint_transform, skin->joints[index]->matrix, sizeof(float)*16);
  }
  else{
    cgltf_node_transform_local(skin->joints[index], joint_transform);
  }

  // Get the inverse bind matrix for the joint
  float inverse_bind_matrix[16]={1,0,0,0,0,1,0,0,0,0,1,0,0,0,1};
  if(skin->inverse_bind_matrices)
    cgltf_accessor_read_float(skin->inverse_bind_matrices, index, inverse_bind_matrix, 16);

  // Multiply the world matrix by the inverse bind matrix
  asVecmat4xmat4(joint_matrix, joint_transform, inverse_bind_matrix);
}

void apply_skinning(
    const cgltf_skin * skin,
    const cgltf_accessor *position_accessor, 
    const cgltf_accessor *joints_accessor,
    const cgltf_accessor *weights_accessor,
    vec3 *output_positions){
  #pragma omp simd
  for (cgltf_size i = 0; i < position_accessor->count; i++){
    float position[4];
    cgltf_accessor_read_float(position_accessor, i, position, 4);
    position[3] = 1.0f;

    unsigned int joint_indicies[4];
    cgltf_accessor_read_uint(joints_accessor, i, joint_indicies, 4);

    float weights[4];
    cgltf_accessor_read_float(weights_accessor, i , weights, 4);
  
    float skin_mat[4][4] = {{0.0f},{0.0f},{0.0f},{0.0f}};

    for(int accessor_index=0; accessor_index < 4; accessor_index++){
      const float weight = weights[accessor_index];
      if(weight == 0){
        continue;
      }

      float j_matrix[16];
      calculate_joint_matrix(skin, joint_indicies[accessor_index], j_matrix);

      for(int ii = 0; ii < 4; ii++){
        for(int jj =0; jj < 4; jj++){
          skin_mat[ii][jj] += weight * j_matrix[ii * 4 + jj];
        }
      }
    }
    float outp[4];
    mat4xvec4(outp, skin_mat, position);
    output_positions[i] = (vec3){outp[0],outp[1],outp[2]};
  }
}


void transform_skinned_mesh_positions(const cgltf_node * node, vec3 *output_positions){
  cgltf_mesh *mesh = node->mesh;
  if(!mesh) return;
  //float* joint_matrices = malloc(node->skin->joints_count * 16 * sizeof(float));
  for(cgltf_size pi=0; pi < mesh->primitives_count; pi++){
    cgltf_primitive *prim = &mesh->primitives[pi];
    cgltf_accessor *position_accessor = NULL;
    cgltf_accessor *joints_accessor = NULL;
    cgltf_accessor *weights_accessor = NULL;

    for(cgltf_size ai=0; ai<prim->attributes_count; ai++){
      cgltf_attribute * attr = &prim->attributes[ai];
      switch(attr->type){
        case cgltf_attribute_type_position:
          position_accessor = attr->data;
        case cgltf_attribute_type_joints:
          joints_accessor = attr->data;
        case cgltf_attribute_type_weights:
          weights_accessor = attr->data;
        default:
          continue;
      }
    }
    if (!position_accessor)
    {
      fprintf(stderr, "No POSITION attribute found for primitive %zu.\n", pi);
      continue;
    }

    if(!joints_accessor){
      fprintf(stderr, "no JOINT attribute found for primative %zu. \n", pi);
      continue;
    }

    if(!weights_accessor){
      fprintf(stderr, "No WEIGHTS arribute found for primative %zu\n", pi);
      continue;
    }
    //calculate_joint_matrices(node->skin, joint_matrices);
    apply_skinning(node->skin,position_accessor, joints_accessor, weights_accessor, output_positions);
  }
  //free(joint_matrices);
}

void transform_positions(const cgltf_node * node, vec3* output_positions){
  cgltf_mesh *mesh = node->mesh;
  if(!mesh) return;

  float transform_matrix_vector[16], transform_matrix[4][4];
  cgltf_node_transform_local(node, transform_matrix_vector);
  vMat4toMat4(transform_matrix, transform_matrix_vector);

  for(cgltf_size pi=0; pi < mesh->primitives_count; pi++){
    cgltf_primitive *prim = &mesh->primitives[pi];
    cgltf_accessor *position_accessor = NULL;

    for(cgltf_size ai=0; ai<prim->attributes_count; ai++){
      cgltf_attribute * attr = &prim->attributes[ai];
      switch(attr->type){
        case cgltf_attribute_type_position:
          position_accessor = attr->data;
        default:
          break;
      }
    }

    if(!position_accessor) continue;
    for(cgltf_size vi = 0; vi < position_accessor->count; vi++){ //for each vertex
      float positon[4], out_position[4];
      cgltf_accessor_read_float(position_accessor, vi, positon, 3);
      positon[3] = 1.0f;
      mat4xvec4(out_position, transform_matrix, positon);
      output_positions[vi] = (vec3){out_position[0],out_position[1],out_position[2]};
    }
  }
}

// Extract animated vertex positions for rendering
void extract_animated_vertex_positions(cgltf_data *data, vec3 *output_positions){
  size_t node_count = data->nodes_count;
  for (cgltf_size node_index = 0; node_index < node_count; ++node_index){
    cgltf_node *node = &data->nodes[node_index];
    if(node->skin){
      transform_skinned_mesh_positions(node,output_positions);
      continue;
    }
    transform_positions(node,output_positions);
  }
}

void updateGltfDisplayChar(player p, vec3 *verts, vec3 lightSource){
  vec3 Faceverts[3];
  for (int fi = 0; fi < p.numFaces; fi++){
    Faceverts[0] = verts[p.faces[fi].vertIdx[0]];
    Faceverts[1] = verts[p.faces[fi].vertIdx[1]];
    Faceverts[2] = verts[p.faces[fi].vertIdx[2]];
    vec3 faceNormal = calc_face_normal(Faceverts[0], Faceverts[1], Faceverts[2]);
    float lumen = calc_luminesence(lightSource, Faceverts[0], faceNormal);
    char lChar = lumin_to_char(lumen);
    for (int vi = 0; vi < p.faces[fi].vertCount; vi++){
      p.displayChar[p.faces[fi].vertIdx[vi]] = lChar;
    }
  }
}

vec3 getLtfCOM(cgltf_data *data){
  vec3 COM = {0.0f};
  int currentVert = 0;
  for (cgltf_size mesh_index = 0; mesh_index < data->meshes_count; ++mesh_index)
  {
    cgltf_mesh *mesh = &data->meshes[mesh_index];
    for (cgltf_size prim_index = 0; prim_index < mesh->primitives_count; ++prim_index)
    {
      cgltf_primitive *prim = &mesh->primitives[prim_index];
      for (cgltf_size attr_index = 0; attr_index < prim->attributes_count; ++attr_index)
      {
        cgltf_attribute *attr = &prim->attributes[attr_index];
        if (attr->type == cgltf_attribute_type_position)
        {
          cgltf_accessor *accessor = attr->data;
          cgltf_buffer_view *buffer_view = accessor->buffer_view;
          const uint8_t *buffer_data = (const uint8_t *)buffer_view->buffer->data;
          const uint8_t *vertex_data_ptr = buffer_data + buffer_view->offset + accessor->offset;

          for (cgltf_size i = 0; i < accessor->count; ++i)
          {
            const float *position = (const float *)(vertex_data_ptr + i * accessor->stride);
            vec3 currPos = {position[0], position[1], position[2]};
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

vec3 * extract_translations(cgltf_data *data){
  int count = data->nodes_count;
  vec3 *translation = malloc(count * sizeof(vec3));
  for(int ni=0;ni < count; ni++){
    cgltf_node* node = &data->nodes[ni];
    float x = node->has_translation ? node->translation[0] : 0.0f;
    float y = node->has_translation ? node->translation[1] : 0.0f;
    float z = node->has_translation ? node->translation[2] : 0.0f;
    translation[ni] = (vec3){x,y,z};
    printf("%lf,%lf,%lf\n",x,y,z);
  }
  return translation;
}
