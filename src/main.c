#define _DEFAULT_SOURCE
#define CGLTF_IMPLEMENTATION
#include "player.h"
#include "object.h"
#include "vertexMath.h"
#include "screen.h"
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <ncurses.h>

int playerToScreen(){
  struct winsize w;
  ioctl(STDOUT_FILENO,TIOCGWINSZ, &w);

  float height = (float)w.ws_col;
  int y_w = w.ws_col;
  float width = (float)w.ws_row;
  int x_w = w.ws_row;

  cgltf_data* data = processGltf("resources/player3.glb");

  int nVerts = getNumVerts(data);
  vec3* verts = malloc(nVerts * sizeof(vec3));

  char **output = malloc(x_w * sizeof(char *));
  for(int i=0;i<x_w;i++){
    output[i] = malloc(y_w * sizeof(char));
  }
  float projectMat[4][4];
  float viewMatrix[4][4];

  //vec3 lightS = {0.0f,-3.0f,-5.0f};
  //vec3 axis = {1.0f,0.0f,0.0f};
  //float angle = 0.01f;
  vec3 upDirection = {0.0f,0.0f,-1.0f};
  vec2 points2D[nVerts];
  getProjectionMatrix(projectMat,90.0f,width/height,0.1f,1000.0f);
  vec3 cameraPosition = {0.0f,5.0f,2.0f};
  initscr();
  noecho();
  cbreak();
  nodelay(stdscr,TRUE);
  curs_set(0);
  
  //for(int i=0;i<1;i++){
  while(true){
    cgltf_animation* a = findAnimationName(data,"Idol");
    applyAnimation(data, a, (float)10);
    extract_animated_vertex_positions(data, verts);
    for(int x=0;x<x_w;x++){
      for(int y=0; y<y_w; y++){
        output[x][y] = ' ';
      }
    }
    vec3 lookAtPosition = getCOM(verts,nVerts);
    lookAt(cameraPosition, lookAtPosition, upDirection, viewMatrix);
    point3DProjection(verts,points2D,nVerts, projectMat, viewMatrix, width, height);
    for(int vi=0;vi<nVerts; vi++){
      int x = (int)points2D[vi].x;
      int y = (int)points2D[vi].y;
      if(x >= x_w || y >= y_w || x < 0 || y < 0){
        continue;
      }
      output[x][y] = '@';
    }
    int ch = getch();
    if(ch == 'a'){
      vec3 a = {0.0f,0.0f,0.0f};
      a.y-=1.0f;
      cameraPosition = rotateAround(cameraPosition,lookAtPosition,0.1f,a);
    }
    if(ch == 'd'){
      vec3 a = {0.0f,0.0f,0.0f};
      a.y+=1.0f;
      cameraPosition = rotateAround(cameraPosition,lookAtPosition,0.1f,a);
    }
    if(ch == 'w'){
      vec3 a = {0.0f,0.0f,0.0f};
      a.x+=1.0f;
      cameraPosition = rotateAround(cameraPosition,lookAtPosition,0.1f,a);
    }
    if(ch == 's'){
      vec3 a = {0.0f,0.0f,0.0f};
      a.x-=1.0f;
      cameraPosition = rotateAround(cameraPosition,lookAtPosition,0.1f,a);
    }
    if(ch == 'q'){
      break;
    }
    renderOnTerm(output,y_w,x_w);
    usleep(1000);
  }
  for(int i=0;i<x_w;i++){
    free(output[i]);
  }
  endwin();
  free(verts);
  free(output);
  return 0;
}

int objToScreen(){
  struct winsize w;
  ioctl(STDOUT_FILENO,TIOCGWINSZ, &w);

  float height = (float)w.ws_col;
  int y_w = w.ws_col;
  float width = (float)w.ws_row;
  int x_w = w.ws_row;

  object testObj;
  testObj = parseObjFromFile("resources/cube.obj");
  if(!testObj.isRendered){
    printf("unable to import obj\n");
    return 0;
  }

  char **output = malloc(x_w * sizeof(char *));
  for(int i=0;i<x_w;i++){
    output[i] = malloc(y_w * sizeof(char));
  }
  float projectMat[4][4];
  float viewMatrix[4][4];

  vec3 cameraPosition = ftovec3(10.0f,0.0f,1.0f);
  vec3 lightS = {0.0f,-3.0f,2.0f};
  vec3 lookAtPosition = getObjCOM(testObj);
  vec3 upDirection = {0.0f,0.0f,1.0f};
  vec2 points2D[testObj.nVerts];
  getProjectionMatrix(projectMat,90.0f,width/height,0.1f,1000.0f);
  initscr();
  timeout(1000);
  noecho();
  cbreak();
  nodelay(stdscr,TRUE);
  curs_set(0);

  for(;;){
    for(int x=0;x<x_w;x++){
      for(int y=0; y<y_w; y++){
        output[x][y] = ' ';
      }
    }
    lookAt(cameraPosition, lookAtPosition, upDirection, viewMatrix);
    point3DProjection(testObj.verts,points2D,testObj.nVerts, projectMat, viewMatrix, width, height);
    updateDisplayChars(testObj, lightS);
    for(int fi=0; fi<testObj.nFaces;fi++){
      for(int vi=0;vi<testObj.faces[fi].vertCount; vi++){
        int idx = testObj.faces[fi].vertIdx[vi];
        int x = (int)points2D[idx].x;
        int y = (int)points2D[idx].y;
        if(x >= x_w || y >= y_w || x < 0 || y < 0){
          continue;
        }
        output[x][y] = testObj.displayChar[idx];
      }
    }
    renderOnTerm(output,y_w,x_w);
    int ch = getch();
    if(ch == 'a'){
      vec3 a = {0.0f,0.0f,0.0f};
      a.y-=1.0f;
      cameraPosition = rotateAround(cameraPosition,lookAtPosition,0.1f,a);
    }
    if(ch == 'd'){
      vec3 a = {0.0f,0.0f,0.0f};
      a.y+=1.0f;
      cameraPosition = rotateAround(cameraPosition,lookAtPosition,0.1f,a);
    }
    if(ch == 'w'){
      vec3 a = {0.0f,0.0f,0.0f};
      a.x+=1.0f;
      cameraPosition = rotateAround(cameraPosition,lookAtPosition,0.1f,a);
    }
    if(ch == 's'){
      vec3 a = {0.0f,0.0f,0.0f};
      a.x-=1.0f;
      cameraPosition = rotateAround(cameraPosition,lookAtPosition,0.1f,a);
    }
    if(ch == 'q'){
      break;
    }
    usleep(500);
  }
  for(int i=0;i<x_w;i++){
    free(output[i]);
  }
  free(output);
  endwin();
  freeObject(testObj);
  return 0;
}

int main(){
  objToScreen();
  playerToScreen();
}
