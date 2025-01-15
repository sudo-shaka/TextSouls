#ifndef __ENGINEHEAD__
#define __ENGINEHEAD__

#include "vertexMath.h"
#include "object.h"
#include "player.h"

typedef struct engine{
  object *objects;
  player player;
  player boss;
  vec3 cameraPosition;
} engine;

void engineInit(engine *engine);
void engineRun(engine *engine);
void engineStop(engine *engine);
int processInput(engine *engine);
void render(engine *engine);
void draw_bar(int x, int y, int width, float percentage);
void generate_plane(const float xmin, const float xmax, const float ymin, const float ymax, vec3 verts[2500]);
void startPlayerAnimation(engine *e);
void processHits(engine *e);
int processHitOnPlayer(player p1, player boss);
int processHitOnBoss(player p1, player boss);
void objectToScreen(object obj,
                    const vec3 location,
                    const float projectMat[4][4],
                    const float viewMatrix[4][4],
                    const vec3 lightS,
                    const float width,
                    const float height);
void playerToScreen(engine *e,
                    const float projectMat[4][4],
                    const float viewMatrix[4][4],
                    const float width,
                    const float height);
void print_location(vec3 v);
void draw_floor(const float projectMat[4][4],
                const float viewMatrix[4][4],
                const float width,
                const float height);

#endif
