#include "cgltf.h"
#include "vertexMath.h"

cgltf_data* processGltf(const char *filename);
void getInterpolatedValue(cgltf_animation_sampler* sampler, float current_time, float* result, int count);
void applyAnimation(cgltf_data* data, float current_time);
void animate(cgltf_data* data, float time);
void clearPlayerData(cgltf_data* data);
void getPlayerVerts(cgltf_data* data,vec3* Pos);
int getNumVerts(cgltf_data* data);
vec3 getLtfCOM(cgltf_data* data);