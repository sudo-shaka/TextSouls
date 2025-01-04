#define CGLTF_IMPLEMENTATION

#include "player.h"
#include "object.h"
#include "vertexMath.h"
#include "screen.h"
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <ncurses.h>


int objToScreen(){
  struct winsize w;
  ioctl(STDOUT_FILENO,TIOCGWINSZ, &w);

  float height = (float)w.ws_col;
  int y_w = w.ws_col;
  float width = (float)w.ws_row;
  int x_w = w.ws_row;

  object sword;
  sword = parseObjFromFile("resources/Shield.obj");

  if(!sword.isRendered){
    printf("unable to import obj\n");
    return 0;
  }

  char output[x_w][y_w];
  float projectMat[4][4];
  float viewMatrix[4][4];

  vec3 cameraPosition = ftovec3(0.0f,0.0f,-10.0f);
  vec3 axis = ftovec3(0.0f,1.0f,0.0f);
  float angle = 0.01f;
  vec3 lookAtPosition = getCOM(sword);
  vec3 upDirection = ftovec3(0.0f,1.0f,0.0f);
  vec2 points2D[sword.nVerts];
  getProjectionMatrix(projectMat,90.0f,width/height,0.1f,1000.0f);

  for(int i=0;i<1000;i++){
    for(int i=0;i<x_w;i++){
      for(int j=0; j<y_w; j++){
        output[i][j] = ' ';
      }
    }
    cameraPosition = rotateAround(cameraPosition,lookAtPosition,angle,axis);
    lookAt(cameraPosition, lookAtPosition, upDirection, viewMatrix);
    point3DProjection(sword.verts,points2D,sword.nVerts, projectMat, viewMatrix, width, height);
    for(int i=0; i<sword.nVerts;i++){
      int x = (int)points2D[i].x;
      int y = (int)points2D[i].y;
      if(x > x_w || y > y_w || x < 0 || y < 0){
        continue;
      }
      output[x][y] = '.';
    }
    
    clearScreen();
    for(int i=0;i<width;i++){
      for(int j=0;j<height;j++){
        putchar(output[i][j]);
      }
      putchar('\n');
    }
  }

  freeObject(sword);
  return 0;
}

int main(){
  //objToScreen();
  cgltf_data player_data;
  processGltf("resources/Gaurd.gltf", &player_data);
  animate(&player_data, 0.5f);
}
