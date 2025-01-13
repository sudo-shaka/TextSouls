#define _DEFAULT_SOURCE
#define CGLTF_IMPLEMENTATION
#include "player.h"
#include "object.h"
#include "vertexMath.h"
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <ncurses.h>

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


int playerToScreen(){
  struct winsize w;
  ioctl(STDOUT_FILENO,TIOCGWINSZ, &w);

  float height = (float)w.ws_row;
  float width = (float)w.ws_col;

  //making floor
  vec3 floorVerts[2500];
  generate_plane(-5.0f,5.0f,-5.0f,5.0f,floorVerts);
  vec2 floorVerts2D[2500];

  object obj = parseObjFromFile("resources/CommonTree_2.obj");
  vec3 objCOM = getObjCOM(obj);
  for(int vi=0;vi<obj.nVerts;vi++){
    obj.verts[vi].x -= 3;
  }
  vec2 obj2D[obj.nVerts];

  //cgltf_data* data = processGltf("resources/player3.glb");
  cgltf_data* data = processGltf("/home/shaka/Code/C/TextSouls/resources/example.gltf");
  //cgltf_data* data = processGltf("/home/shaka/Code/C/TextSouls/resources/player3.glb");
  player p = initPlayer(100,100,data);
  p.faces = malloc(p.numFaces * sizeof(face));
  extract_face_indices(p.data,p.faces);
  int nVerts = getNumVerts(p.data);
  vec3* verts = malloc(nVerts * sizeof(vec3));
  p.displayChar = malloc(nVerts * sizeof(char));

  float projectMat[4][4];
  float viewMatrix[4][4];

  vec3 lightS = {0.0f,-3.0f,2.0f};
  vec3 upDirection = {0.0f,0.0f,1.0f};
  vec2 points2D[nVerts];
  getProjectionMatrix(projectMat,90.0f,height/width,0.1f,1000.0f);
  vec3 cameraPosition = {0.0f,-3.0f,0.5f};
  vec3 lookAtPosition = {0.0f,0.0f,0.0f};
  initscr();
  noecho();
  cbreak();
  nodelay(stdscr,TRUE);
  curs_set(0);
  MEVENT event;
  mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);

  float ti = 0.0;
  while(true){
    //cgltf_animation* a = findAnimationName(data,"Idol");
    cgltf_animation *a = &data->animations[0];
    if(a == NULL){
      closePlayer(p);
      freeObject(obj);
      free(verts);
      endwin();
      printf("Animation is null: Exiting...\n");
      return 1;
    }
    ti += 0.01f; float totalTime = calculate_total_animation_time(a);
    if(ti > totalTime){ti = 0.0f;}
    applyAnimation(p.data, a, ti);
    extract_animated_vertex_positions(p.data, verts);
    getPlayerVerts(p.data,verts);
    updateGltfDisplayChar(p,verts,lightS);
    updateDisplayChars(obj,lightS);
    lookAt(cameraPosition, lookAtPosition, upDirection, viewMatrix);
    point3DProjection(verts,points2D,nVerts, projectMat, viewMatrix, width, height);
    point3DProjection(obj.verts,obj2D,obj.nVerts, projectMat, viewMatrix, width, height);
    point3DProjection(floorVerts,floorVerts2D,2500, projectMat, viewMatrix, width, height);
    erase();
    for(int vi=0;vi<nVerts; vi++){
      int x = (int)points2D[vi].x;
      int y = (int)points2D[vi].y;
      char c = p.displayChar[vi];
      mvaddch(y,x,c);
    }
    for(int vi=0;vi<obj.nVerts; vi++){
      int x = (int)obj2D[vi].x;
      int y = (int)obj2D[vi].y;
      mvaddch(y,x,obj.displayChar[vi]);
    }

    for(int vi=0;vi<2500-1;vi++){
      int x = (int)floorVerts2D[vi].x;
      int y = (int)floorVerts2D[vi].y;
      mvaddch(y,x,'.');
    }
    mvprintw(10,0,"%.2f",cameraPosition.x);
    mvprintw(11,0,"%.2f",cameraPosition.y);
    mvprintw(12,0,"%.2f",cameraPosition.z);
    
    float pEnd = (float)p.currEndurance/(float)p.maxEndurace;
    draw_bar(1,1,p.maxHeath,0.75f);
    draw_bar(1,2,p.maxEndurace,pEnd);
    int ch = getch();
    if(ch == 'a'){
      cameraPosition.x -= 0.1f;
    }
    if(ch == 'd'){
     cameraPosition.x += 0.1f;
    }
    if(ch == 'w'){
     cameraPosition.y += 0.1f;
    }
    if(ch == 's'){
     cameraPosition.y -= 0.1f;
    }
    if(ch == 'z'){
      cameraPosition.z -= 0.1f;
    }
    if(ch == 'x'){
      cameraPosition.z += 0.1f;
    }
    if(ch == 'q'){
      break;
    }
    if(ch == KEY_MOUSE){
      if(getmouse(&event) == OK){
        if(event.bstate & BUTTON1_CLICKED){
          p.currEndurance -= 5;
        } else if(event.bstate & BUTTON3_CLICKED){
          p.currEndurance -= 10;
        }
      }
    }
    usleep(5000);
    refresh();
  }
  endwin();
  closePlayer(p);
  free(verts);
  return 0;
}

int main(){
  playerToScreen();
}
