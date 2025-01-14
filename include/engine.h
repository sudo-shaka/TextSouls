#ifndef __ENGINEHEAD__
#define __ENGINEHEAD__

#include "vertexMath.h"
#include "object.h"
#include "player.h"

typedef struct engine{
  object* objects;
  player player;
  vec3 cameraPosition;
} engine;

void engineInit(engine* engine);
void engineRun(engine* engine);
void engineStop(engine* engine);
int processInput(engine* engine);
void render(engine* engine);
void draw_bar(int x, int y, int width, float percentage);
void generate_plane(const float xmin, const float xmax,const float ymin,const float ymax, vec3 verts[2500]);
void objectToScreen(object obj, vec3 location, float projectMat[4][4], float viewMatrix[4][4],vec3 lightS, float width, float height);
void print_location(vec3 v);
void draw_floor(float projectMat[4][4], float viewMatrix[4][4], float width, float height);

#endif
