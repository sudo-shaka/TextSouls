#include "cgltf.h"
#include "vertexMath.h"

cgltf_data* processGltf(const char *filename);
void getInterpolatedValue(cgltf_animation_sampler* sampler, float current_time, float* result, int count);
void applyAnimation(cgltf_data* data, cgltf_animation* animation, float current_time);
void animate(cgltf_data* data, float time);
void clearPlayerData(cgltf_data* data);
void getPlayerVerts(cgltf_data* data,vec3* Pos);
int getNumVerts(cgltf_data* data);
vec3 getLtfCOM(cgltf_data* data);
cgltf_animation* findAnimationName(cgltf_data* data, const char* name);
void readAnimatedVertPositions(cgltf_data* data, vec3* outputPos);
void readVertexPosition(const cgltf_accessor* accessor, vec3 pos, int vertCount);
cgltf_animation* findAnimationName(cgltf_data* data, const char* name);
void extract_animated_vertex_positions(cgltf_data* data, vec3* output_positions);
