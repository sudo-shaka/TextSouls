#define _DEFAULT_SOURCE
#include "player.h"
#include "object.h"
#include "engine.h"
#include "vertexMath.h"
#include <stdio.h>
#include <unistd.h>
#include <ncurses.h>

void engineInit(engine* engine){
  initscr();
  noecho();
  cbreak();
  nodelay(stdscr,TRUE);
  curs_set(0);
  mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
  //cgltf_data* data = processGltf("/home/shaka/Code/C/TextSouls/resources/player3.glb");
  cgltf_data* data = processGltf("/home/shaka/Code/C/TextSouls/resources/example.gltf");
  engine->player = initPlayer(100,100,data);
  engine->cameraPosition = (vec3){0.0f,-5.0f,0.5f};
}
void engineRun(engine* engine){
  while(1){
    int success = processInput(engine);
    if(!success){
      engineStop(engine);
      break;
    }
    render(engine);
    usleep(5000);
  }
}

void engineStop(engine* engine){
  endwin();
  closePlayer(engine->player);
}

int processInput(engine* e){
  //MEVENT event;
  int ch = getch();
  switch(ch){
    case 'a':
      e->cameraPosition.x -= 0.1f;
      e->player.position.x -= 0.1f;
      break;
    case 'd':
      e->cameraPosition.x += 0.1f;
      e->player.position.x += 0.1f;
      break;
    case 'w':
      e->cameraPosition.y += 0.1f;
      e->player.position.y += 0.1f;
      break;
    case 's':
      e->cameraPosition.y -= 0.1f;
      e->player.position.y -= 0.1f;
      break;
    case 'z':
      e->cameraPosition.z -= 0.1f;
      break;
    case 'x':
      e->cameraPosition.z += 0.1f;
      break;
    case 'q':
      return 0;
    case ' ':
      e->player.currEndurance -= 20;
      e->player.currentAnimation = findAnimationName(e->player.data,"Attack");
      break;

    //get mouse input for heavy and light attacks
    /*case KEY_MOUSE:
      if(getmouse(&event) == OK){
        if(event.bstate & BUTTON1_CLICKED){
          e->player.currEndurance -= 5;
        } else if(event.bstate & BUTTON3_CLICKED){
          e->player.currEndurance -= 10;
        }
      }
      break;*/
    default:
      if(e->player.currEndurance < e->player.maxEndurance){e->player.currEndurance += 0.1;}
      e->player.currentAnimation = findAnimationName(e->player.data,"Idol");
      break;
  }
  refresh();
  return 1;
}

void render(engine* engine){
  erase();
  int maxy, maxx;
  getmaxyx(stdscr,maxy,maxx);

  float height = (float)maxy;
  float width = (float)maxx;
  float aspect = width/height;
  float FOV = 82.0f;
  
  float projectMat[4][4];
  float viewMatrix[4][4];
  vec3 lightSource = {0.0f,0.0f,3.0f};
  vec3 upDirection = {0.0f,0.0f,1.0f};
  vec3 lookAtPosition = {0.0f,0.0f,0.0f};
  getProjectionMatrix(projectMat,FOV,aspect,0.1f,500.0f);

  player p = engine->player;
  draw_bar(1,1,p.maxHealth,(float)p.currentHeath/(float)p.maxHealth);
  draw_bar(1,2,p.maxEndurance,p.currEndurance/p.maxEndurance);
  
  vec2 points2D[p.numVerts];
  if(p.currentAnimation == NULL){
    p.currentAnimation = &p.data->animations[0]; //fix this later for error handling
  }
  applyAnimation(p.data,p.currentAnimation,0.25f); //update the time input later
  extract_animated_vertex_positions(p.data,p.verts);
  extract_face_indices(p.data,p.faces);

  updateGltfDisplayChar(p, p.verts, lightSource);
  lookAt(engine->cameraPosition,lookAtPosition,upDirection,viewMatrix);
  point3DProjection(p.verts,points2D,p.numVerts,projectMat,viewMatrix,width,height);
  for(int fi=0;fi<p.numFaces;fi++){
    for(int vi=0;vi<3;vi++){
      int real_vi = p.faces[fi].vertIdx[vi];
      int x = (int)points2D[real_vi].x;
      int y = (int)points2D[real_vi].y;
      char ch = p.displayChar[real_vi];
      mvaddch(y,x,ch);
    }
  }

  draw_floor(projectMat,viewMatrix,width,height);
  print_location(engine->cameraPosition);
}

void draw_bar(int x, int y, int width, float percentage){
  int fill = (int)(width * percentage);
  for(int i=0;i<width;i++){
    mvaddch(y,x+i,(i<fill) ? '#' : '.');
  }
}

void generate_plane(const float xmin, const float xmax,const float ymin,const float ymax, vec3 verts[2500]){
  int resolution = 50;
  float x_step = (xmax - xmin) / resolution;
  float y_step = (ymax - ymin) / resolution;
  int count = 0;

  for(float y=ymin; y < ymax; y+=y_step){
    for(float x=xmin; x < xmax; x+= x_step){
      vec3 p = {x,y,0.0f};
      if(count < 2500){
        verts[count] = p;
      }
      count++;
    }
  }
}

void objectToScreen(object obj, vec3 location, float projectMat[4][4], float viewMatrix[4][4],vec3 lightS, float width, float height){
  vec2 points2D[obj.nVerts];
  updateDisplayChars(obj,lightS);
  point3DProjection(obj.verts,points2D,obj.nVerts,projectMat,viewMatrix,width,height);
  for(int fi=0;fi<obj.nFaces;fi++){
    for(int vi=0;vi<3;vi++){
      int real_vi = obj.faces[fi].vertIdx[vi];
      int x = (int)points2D[real_vi].x;
      int y = (int)points2D[real_vi].y;
      char ch = obj.displayChar[real_vi];
      mvaddch(y,x,ch);
    }
  }
}

void draw_floor(float projectMat[4][4], float viewMatrix[4][4], float width, float height){
  vec3 floorVerts3D[2500];
  vec2 floorVerts2D[2500];
  generate_plane(-5.0f,5.0f,-5.0f,5.0f,floorVerts3D);
  point3DProjection(floorVerts3D,floorVerts2D,2500, projectMat, viewMatrix, width, height);
  for(int vi=0;vi<2500-1;vi++){
    int x = (int)floorVerts2D[vi].x;
    int y = (int)floorVerts2D[vi].y;
    mvaddch(y,x,'.');
  }
}

void print_location(vec3 v){
  mvprintw(10,0,"%.2f",v.x);
  mvprintw(11,0,"%.2f",v.y);
  mvprintw(12,0,"%.2f",v.z);
}
