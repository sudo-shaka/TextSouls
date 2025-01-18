#define _DEFAULT_SOURCE
#include "player.h"
#include "object.h"
#include "engine.h"
#include "vertexMath.h"
#include <stdio.h>
#include <unistd.h>
#include <ncurses.h>
#include <time.h>
#include <math.h>

#define CURRTIME(s,e)((e-s)/CLOCKS_PER_SEC)

void engine_start(engine *engine){
  initscr();
  noecho();
  cbreak();
  nodelay(stdscr, TRUE);
  keypad(stdscr, TRUE);
  raw();
  curs_set(0);
  mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
  mouseinterval(0);
  const char * bpath = "/home/shaka/Code/C/TextSouls/resources/BoxAnimated.glb";
  const char * ppath = "/home/shaka/Code/C/TextSouls/resources/example.gltf";
  cgltf_data * player_data = processGltf(ppath);
  cgltf_data * boss_data = processGltf(bpath);
  if(player_data == NULL || boss_data == NULL){
    return;
  }
  engine->player = initPlayer(100, 100, player_data);
  engine->boss = initPlayer(100,100,boss_data);
}
void engine_run(engine *engine){
  if(engine->player.data == NULL || engine->boss.data == NULL){
    engine_stop(engine);
    printf("player data is null\n");
    return;
  }
  // main game loop
  while (engine->player.currentHeath > 0){
    engine->start_time = clock();
    process_hits(engine);
    int success = process_input(engine);
    play_animations(engine,engine->start_time);
    if (!success){
      engine_stop(engine);
      printf("You quit... Have you gone hollow?\n");
      return;
    }
    render(engine);
    mvprintw(10,5,"%.f FPS",1/CURRTIME(engine->start_time,clock())/100);
  }
  engine->player.currentAnimation = findAnimationName(engine->player.data, "death");
  engine_stop(engine);
  printf("You died.\n");
}

void play_animations(engine * engine, float time){
  engine->boss.currentAnimation = &engine->boss.data->animations[0];
  apply_animation(engine->boss.currentAnimation,time);
}

void engine_stop(engine *engine){
  endwin();
  closePlayer(engine->player);
  closePlayer(engine->boss);
}

int process_hits_on_boss(player p, player b){
  return 0;
}

int process_hits_on_player(player p, player b){
  return 0;
}

void process_hits(engine *e){
  int bosshit = process_hits_on_boss(e->player, e->boss);
  int playerhit = process_hits_on_player(e->player, e->boss);
  if (bosshit){
    e->boss.currentHeath -= 10;
  }
  if (playerhit){
    e->player.currentHeath -= 10;
    e->player.currentAnimation = findAnimationName(e->player.data, "stagger");
  }
}

int process_input(engine *e){
  MEVENT event;
  int ch = getch();
  // prevent player from moving if endurance is below 0
  if (e->player.currEndurance < 0.0f){
    return 1;
  }
  vec3 diff = normalize(sub3(e->player.position,e->boss.position));
  diff.x /= 10;
  diff.y /= 10;
  diff.z /= 10;
  switch (ch){
  case 'a':
    if(!e->player.lockedon)
      e->player.position.x += 0.1f;
    else
     e->player.position = rotate_around_point(e->player.position, e->boss.position, 0.1f, (vec3){0,0,1});
    e->player.currentAnimation = findAnimationName(e->player.data, "walk");
    break;
  case 'd':
    if(!e->player.lockedon)
      e->player.position.x -= 0.1f;
    else
     e->player.position = rotate_around_point(e->player.position, e->boss.position, 0.1f, (vec3){0,0,-1});
    e->player.currentAnimation = findAnimationName(e->player.data, "walk");
    break;
  case 'w':
    if(!e->player.lockedon)
      e->player.position.y -= 0.1f;
    else
     e->player.position = sub3(e->player.position,diff);
    e->player.currentAnimation = findAnimationName(e->player.data, "walk");
    break;
  case 's':
    if(!e->player.lockedon)
      e->player.position.y += 0.1f;
    else
     e->player.position = add3(e->player.position,diff);
    e->player.currentAnimation = findAnimationName(e->player.data, "walk");
    break;
  case 'q':
    return 0;
  case ' ':
    e->player.currEndurance -= 15;
    e->player.blocking = 1;
    e->player.currentAnimation = findAnimationName(e->player.data, "Block");
    break;

  case KEY_MOUSE:
    if (getmouse(&event) == OK){
      if (event.bstate & BUTTON1_PRESSED){
        e->player.currEndurance -= 20;
        e->player.currentAnimation = findAnimationName(e->player.data, "Attack");
      }
      else if (event.bstate & BUTTON3_PRESSED){
        e->player.currEndurance -= 30;
        e->player.currentAnimation = findAnimationName(e->player.data, "HeavyAttack");
      }
      else if(event.bstate & BUTTON2_PRESSED){
        e->player.lockedon = !e->player.lockedon;
      }
    }
    break;
  default:
    if (e->player.currEndurance < e->player.maxEndurance) {
      e->player.currEndurance += 1000.0f * CURRTIME(e->start_time,clock());
    }
    e->player.currentAnimation = findAnimationName(e->player.data, "Idol");
    break;
  }
  return 1;
}

void render(engine *engine){
  // clear screen
  refresh();
  erase();
  // set up projection matrix
  int maxy, maxx;
  getmaxyx(stdscr, maxy, maxx);
  float height = (float)maxy;
  float width = (float)maxx;
  float aspect = width / height;
  float FOV = 82.0f;
  float projectMat[4][4];
  float viewMatrix[4][4];
  vec3 lightSource = {0.0f, 0.0f, 3.0f};
  vec3 upDirection = {0.0f, 0.0f, 1.0f};
  projection_matrix(projectMat, FOV, aspect, 0.1f, 500.0f);

  // update player display chars
  player p = engine->player;
  updateGltfDisplayChar(p, p.verts, lightSource);

  // set up view matrix
  engine->boss.position = getLtfCOM(engine->boss.data);
  vec3 ideal_lookat = (engine->player.lockedon) ? engine->boss.position : calculate_ideal_look_at(engine->player.position);
  vec3 ideal_camera = calculate_ideal_offset(engine->player.position);

  float dt = CURRTIME(engine->start_time,clock());
  const float t = powf(0.01,dt);
  
  engine->camera_position = lerp_vec3(engine->camera_position,ideal_camera,t);
  engine->look_at_position = lerp_vec3(engine->look_at_position,ideal_lookat,t);
  look_at(engine->camera_position,engine->look_at_position, upDirection, viewMatrix); 

  // draw character and objects
  //draw_floor(projectMat,viewMatrix,width,height);
  player_to_screen(engine->player, projectMat, viewMatrix, width, height);
  player_to_screen(engine->boss, projectMat, viewMatrix, width, height);
  draw_bar(1, 1, p.maxHealth, (float)p.currentHeath / (float)p.maxHealth);
  draw_bar(1, 2, p.maxEndurance, p.currEndurance / p.maxEndurance);
}

void player_to_screen(player p,
                    const float projectMat[4][4],
                    const float viewMatrix[4][4],
                    const float width,
                    const float height){
  vec2 points2D[p.numVerts];
  extract_animated_vertex_positions(p.data, p.verts);
  extract_face_indices(p.data, p.faces);
  point_3D_projection(p.verts, points2D, p.position,p.numVerts, projectMat, viewMatrix, width, height);
  for (int fi = 0; fi < p.numFaces; fi++){
    for (int vi = 0; vi < 3; vi++){
      int real_vi = p.faces[fi].vertIdx[vi];
      int x = (int)points2D[real_vi].x;
      int y = (int)points2D[real_vi].y;
      char ch = p.displayChar[real_vi];
      mvaddch(y, x, ch);
    }
  }
}

void draw_bar(int x, int y, int width, float percentage){
  // draws a bar at x,y with width and percentage filled for health & endurace display
  int fill = (int)(width * percentage);
  for (int i = 0; i < width; i++){
    mvaddch(y, x + i, (i < fill) ? '#' : '.');
  }
}

void generate_plane(const float xmin, const float xmax, const float ymin, const float ymax, vec3 verts[2500]){
  int resolution = 50;
  float x_step = (xmax - xmin) / resolution;
  float y_step = (ymax - ymin) / resolution;
  int count = 0;

  for (float y = ymin; y < ymax; y += y_step)
  {
    for (float x = xmin; x < xmax; x += x_step)
    {
      vec3 p = {x, y, 0.0f};
      if (count < 2500)
      {
        verts[count] = p;
      }
      count++;
    }
  }
}

void object_to_screen(object obj,
                    const vec3 location,
                    const float projectMat[4][4],
                    const float viewMatrix[4][4],
                    const vec3 lightS,
                    const float width,
                    const float height){
  vec2 points2D[obj.nVerts];
  updateDisplayChars(obj, lightS);
  vec3 offset = {0,0,0};
  point_3D_projection(obj.verts, points2D, offset, obj.nVerts, projectMat, viewMatrix, width, height);
  for (int fi = 0; fi < obj.nFaces; fi++){
    for (int vi = 0; vi < 3; vi++){
      int real_vi = obj.faces[fi].vertIdx[vi];
      int x = (int)points2D[real_vi].x;
      int y = (int)points2D[real_vi].y;
      char ch = obj.displayChar[real_vi];
      mvaddch(y, x, ch);
    }
  }
}

void draw_floor(const float projectMat[4][4],
                const float viewMatrix[4][4],
                const float width,
                const float height){
  vec3 floorVerts3D[2500];
  vec2 floorVerts2D[2500];
  generate_plane(-5.0f, 5.0f, -5.0f, 5.0f, floorVerts3D);
  point_3D_projection(floorVerts3D, floorVerts2D,(vec3){0,0,0},2500, projectMat, viewMatrix, width, height);
  for (int vi = 0; vi < 2500 - 1; vi++){
    int x = (int)floorVerts2D[vi].x;
    int y = (int)floorVerts2D[vi].y;
    mvaddch(y, x, '.');
  }
}

void print_location(vec3 v){
  mvprintw(10, 0, "%.2f", v.x);
  mvprintw(11, 0, "%.2f", v.y);
  mvprintw(12, 0, "%.2f", v.z);
}
