#include "cgltf.h"

void processGltf(const char *filename, cgltf_data* data);
void getInterpolatedValue(cgltf_animation_sampler* sampler, float current_time, float* result, int count);
void applyAnimation(cgltf_data* data, float current_time);
void animate(cgltf_data* data, float time);
