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
#define SWAP(x,y){int tmp=x;x=y,y=tmp;}

void engine_start(engine *engine){
  initscr();
  if(has_colors()){start_color();use_default_colors();}
  noecho();
  cbreak();
  nodelay(stdscr, TRUE);
  keypad(stdscr, TRUE);
  raw();
  curs_set(0);
  mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
  mouseinterval(0);
  const char * ppath = "/home/shaka/Code/C/TextSouls/resources/example.gltf";
  const char * bpath = "/home/shaka/Code/C/TextSouls/resources/example.gltf";
  cgltf_data * player_data = processGltf(ppath);
  cgltf_data * boss_data = processGltf(bpath);
  if(player_data == NULL || boss_data == NULL){
    return;
  }
  engine->player = initPlayer(100, 100, player_data);
  engine->boss = initPlayer(250,100,boss_data);
}
void engine_run(engine *engine){
  if(engine->player.data == NULL || engine->boss.data == NULL){
    engine_stop(engine);
    printf("player data is null\n");
    return;
  }
  // main game loop
  //set_animation(&engine->player, findAnimationName(engine->player.data,"walking"));
  set_animation(&engine->player, &engine->player.data->animations[0]);
  set_animation(&engine->boss, &engine->boss.data->animations[0]);
  while (engine->player.currentHeath > 0){
    engine->start_time = clock();
    process_hits(engine);
    int success = process_input(engine);
    play_animations(engine);
    if (!success){
      engine_stop(engine);
      printf("You quit... Have you gone hollow?\n");
      return;
    }
    render(engine);
    mvprintw(10,5,"%4.1f FPS",1/CURRTIME(engine->start_time,clock()));
  }
  set_animation(&engine->player, findAnimationName(engine->player.data, "death"));
  engine_stop(engine);
  printf("You died.\n");
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

void play_animations(engine *e){
  float current_time = CURRTIME(e->start_time,clock());
  if(e->player.animation_time > e->player.total_animation_time){
    e->player.animation_time = 0.0f;
  }
  if(e->boss.animation_time > e->boss.total_animation_time){
    e->boss.animation_time = 0.0f;
  }
  e->player.animation_time += current_time;
  e->boss.animation_time += current_time;
  apply_animation(&e->player);
  apply_animation(&e->boss);
}

void process_hits(engine *e){
  int bosshit = process_hits_on_boss(e->player, e->boss);
  int playerhit = process_hits_on_player(e->player, e->boss);
  if (bosshit){
    e->boss.currentHeath -= 10;
  }
  if (playerhit){
    e->player.currentHeath -= 10;
    //set_animation(&e->player, findAnimationName(e->player.data, "stagger"));
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
    //set_animation(&e->player, findAnimationName(e->player.data, "walk"));
    break;
  case 'd':
    if(!e->player.lockedon)
      e->player.position.x -= 0.1f;
    else
     e->player.position = rotate_around_point(e->player.position, e->boss.position, 0.1f, (vec3){0,0,-1});
    //set_animation(&e->player, findAnimationName(e->player.data, "walk"));
    break;
  case 'w':
    if(!e->player.lockedon)
      e->player.position.y -= 0.1f;
    else
     e->player.position = sub3(e->player.position,diff);
    //set_animation(&e->player, findAnimationName(e->player.data, "walk"));
    break;
  case 's':
    if(!e->player.lockedon)
      e->player.position.y += 0.1f;
    else
     e->player.position = add3(e->player.position,diff);
    //set_animation(&e->player, findAnimationName(e->player.data, "walk"));
    break;
  case 'q':
    return 0;
  case ' ':
    e->player.currEndurance -= 15;
    e->player.blocking = 1;
    //set_animation(&e->player, findAnimationName(e->player.data, "block"));
    break;

  case KEY_MOUSE:
    if (getmouse(&event) == OK){
      if (event.bstate & BUTTON1_PRESSED){
        e->player.currEndurance -= 20;
        //set_animation(&e->player, findAnimationName(e->player.data, "Attack"));
      }
      else if (event.bstate & BUTTON3_PRESSED){
        e->player.currEndurance -= 30;
        //set_animation(&e->player, findAnimationName(e->player.data, "HeavyAttack"));
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
    //set_animation(&e->player, findAnimationName(e->player.data, "Idol"));
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
  engine->screen_height = (float)maxy;
  engine->screen_width = (float)maxx;
  float aspect = engine->screen_width / engine->screen_height;
  engine->FOV = 82.0f;
  engine->lightS = (vec3){0.0f, 0.0f, 3.0f};
  engine->upDirection = (vec3){0.0f, 0.0f, 1.0f};
  projection_matrix(engine->projectMat, engine->FOV, aspect, 0.1f, 500.0f);

  // update player display chars
  player p = engine->player;
  updateGltfDisplayChar(p, p.verts, engine->lightS);

  // set up view matrix
  engine->boss.position = getLtfCOM(engine->boss.data);
  vec3 ideal_lookat = (engine->player.lockedon) ? engine->boss.position : calculate_ideal_look_at(engine->player.position);
  vec3 ideal_camera = calculate_ideal_offset(engine->player.position);

  float dt = CURRTIME(engine->start_time,clock());
  const float t = powf(0.01,dt);
  
  engine->camera_position = lerp_vec3(engine->camera_position,ideal_camera,t);
  engine->look_at_position = lerp_vec3(engine->look_at_position,ideal_lookat,t);
  look_at(engine->camera_position,engine->look_at_position, engine->upDirection, engine->viewMatrix); 

  // draw character and objects
  //draw_floor(engine->projectMat,engine->viewMatrix,engine->screen_width,engine->screen_height);
  player_to_screen(engine, &engine->player);
  player_to_screen(engine, &engine->boss);
  draw_bar(1, 1, p.maxHealth, (float)p.currentHeath / (float)p.maxHealth);
  draw_bar(1, 2, p.maxEndurance, p.currEndurance / p.maxEndurance);
}

void player_to_screen(engine *e,
                    player *p){
  vec2 points2D[p->numVerts];
  extract_animated_vertex_positions(p->data, p->verts);
  point_3D_projection(p->verts, 
    points2D, 
    p->position,
    p->numVerts, 
    e->projectMat,
    e->viewMatrix, 
    e->screen_width, 
    e->screen_height
  );
  for (int fi = 0; fi < p->numFaces; fi++){
    int idx[3] = {0}; 
    idx[0] = p->faces[fi].vertIdx[0];
    idx[1] = p->faces[fi].vertIdx[1];
    idx[2] = p->faces[fi].vertIdx[2];
    if(idx[0] >= p->numVerts || idx[1] >= p->numVerts || idx[2] >= p->numVerts){
      continue;
    }
    vec3 a3[3] = {p->verts[idx[0]],p->verts[idx[1]],p->verts[idx[2]]};
    vec2 a2[3] = {points2D[idx[0]],points2D[idx[1]],points2D[idx[2]]};
    int isfacing = is_face_facing_point(a3, e->camera_position);
    if(isfacing != -1){
      //fillshape(a2[0],a2[1],a2[2]);
      for (int vi = 0; vi < 3; vi++){
        int x = (int)a2[vi].x;
        int y = (int)a2[vi].y;
        char ch = p->displayChar[idx[vi]];
        mvaddch(y, x, ch);
      }
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

void object_to_screen(engine *e,
                    object obj,       
                    const vec3 location){
  vec2 points2D[obj.nVerts];
  updateDisplayChars(obj, e->lightS);
  vec3 offset = {0,0,0};
  point_3D_projection(obj.verts, points2D, offset, obj.nVerts, 
      e->projectMat, e->viewMatrix, e->screen_width, e->screen_height);
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

void fillshape(vec2 a, vec2 b, vec2 c){
  int i,j;
  if(a.y > b.y){
    SWAP(a.y,b.y);
    SWAP(a.x,b.x);
  }
  if(a.y > c.y){
    SWAP(a.y,c.y);
    SWAP(a.x,c.x);
  }
  if(b.y > c.y){
    SWAP(b.y,c.y);
    SWAP(b.x,c.x);
  }
  int x1 = (int)a.x;
  int y1 = (int)a.y;
  int x2 = (int)b.x;
  int y2 = (int)b.y;
  int x3 = (int)c.x;
  int y3 = (int)c.y;

  if(y3-y1 < 5 || y2-y1 < 5 || x3 - x1 < 5 || x2 - x1 < 5){
    return;
  }

  for(i=y1;i<=y3;i++){
    int x_left = x1 + (i-y1) * (x3-x1) / (y3-y1);
    int x_right = x1 + (i-y1) * (x2-x1) / (y2-y1);
    if(i > y2){
      x_right = x2 + (i-y2) * (x3-x2)/(y3-y2);
    }

    for(j=x_left;j<=x_right;j++){
      mvaddch(i,j,'#');
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
