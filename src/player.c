#include "cgltf.h"
#include "vertexMath.h"
#include <stdlib.h>
#include <stdio.h>
  
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

void applyAnimation(cgltf_data* data, float current_time){
  for (cgltf_size i = 0; i < data->animations_count; i++) {
  cgltf_animation* animation = &data->animations[i];
  for (cgltf_size j = 0; j < animation->channels_count; j++) {
      cgltf_animation_channel* channel = &animation->channels[j];
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
}

void animate(cgltf_data* data, float time){
  applyAnimation(data, time);
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
