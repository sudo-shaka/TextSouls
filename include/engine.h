#ifndef __ENGINEHEAD__
#define __ENGINEHEAD__

#include "vertexMath.h"
#include "object.h"
#include "player.h"

typedef struct engine{
  object *objects;
  player player;
  player boss;
  vec3 camera_position;
  vec3 look_at_position;
  float start_time;
} engine;

void engine_start(engine *engine);
void engine_run(engine *engine);
void engine_stop(engine *engine);
int process_input(engine *engine);
void render(engine *engine);
void draw_bar(int x, int y, int width, float percentage);
void generate_plane(const float xmin, const float xmax, const float ymin, const float ymax, vec3 verts[2500]);
void process_hits(engine *e);
int process_hits_on_player(player p1, player boss);
int process_hits_on_boss(player p1, player boss);
void play_animations(engine * engine);
void object_to_screen(object obj,
                    const vec3 location,
                    const float projectMat[4][4],
                    const float viewMatrix[4][4],
                    const vec3 lightS,
                    const float width,
                    const float height);
void player_to_screen(player p,
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
