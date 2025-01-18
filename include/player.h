#include "cgltf.h"
#include "vertexMath.h"

#ifndef __PLAYER__
#define __PLAYER__
typedef struct player{
  int maxHealth;
  int currentHeath;
  float maxEndurance;
  float currEndurance;
  int numFaces;
  int numVerts;
  int blocking;
  char *displayChar;
  vec3 position;
  cgltf_data *data;
  face *faces;
  vec3 *verts;
  vec3 direction;
  int lockedon;
  vec3 bodyHitbox[8];
  vec3 weaponHitbox[8];
  cgltf_animation *currentAnimation;
} player;

player initPlayer(int Health, int Endurance, cgltf_data *inputData);
void closePlayer(player p);
cgltf_data *processGltf(const char *filename);
void interpolate(cgltf_animation_sampler *sampler, float current_time, float *result, int count);
void apply_animation(cgltf_animation *animation, float current_time);
void getPlayerVerts(cgltf_data *data, vec3 *Pos);
void extract_face_indices(const cgltf_data *data, face *faces);
int count_faces(const cgltf_data *data);
float calculate_total_animation_time(const cgltf_animation *animation);
int count_verts(cgltf_data *data);
vec3 getLtfCOM(cgltf_data *data);
cgltf_animation *findAnimationName(cgltf_data *data, const char *name);
void readAnimatedVertPositions(cgltf_data *data, vec3 *outputPos);
void readVertexPosition(const cgltf_accessor *accessor, vec3 pos, int vertCount);
cgltf_animation *findAnimationName(cgltf_data *data, const char *name);
void extract_animated_vertex_positions(cgltf_data *data, vec3 *output_positions);
void updateGltfDisplayChar(player p, vec3 *verts, vec3 lightSource);
#endif
