
#include "cgltf.h"
#include "vertexMath.h"
#include <stdio.h>
  
void processGltf(const char *filename, cgltf_data* data){
  cgltf_options options = {0};
  data = NULL;
  cgltf_result result = cgltf_parse_file(&options, filename, &data);
  
  if (result != cgltf_result_success){
    fprintf(stderr,"Error opening .gltf");
  }
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
