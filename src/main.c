#include "object.h"
#include "vertexMath.h"
#include "screen.h"
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main(){

  struct winsize w;
  ioctl(STDOUT_FILENO,TIOCGWINSZ, &w);

  float height = (float)w.ws_col;
  int y_w = w.ws_col;
  float width = (float)w.ws_row;
  int x_w = w.ws_row;

  object sword;
  sword = parseFromFile("resources/Sword.obj");

  if(!sword.isRendered){
    printf("unable to import obj\n");
  }

  char output[x_w][y_w];
  for(int i=0;i<x_w;i++){
    for(int j=0; j<y_w; j++){
      output[i][j] = ' ';
    }
  }
  float projectMat[4][4];
  float viewMatrix[4][4];

  vec3 cameraPosition = ftovec3(0.0f,0.0f,-10.0f);
  //vec3 axis = ftovec3(1.0f,0.0f,0.0f);
  //float angle = 0.1f;
  vec3 lookAtPosition = getCOM(sword);
  vec3 upDirection = ftovec3(0.0f,1.0f,0.0f);

  vec2 *points2D;

  getProjectionMatrix(projectMat,90.0f,width/height,0.1f,1000.0f);
  lookAt(cameraPosition, lookAtPosition, upDirection, viewMatrix);
  point3DProjection(sword.verts,&points2D,sword.nVerts, projectMat, viewMatrix, width, height);

  for(int i=0; i<5;i++){
    int x = (int)points2D[i].x;
    int y = (int)points2D[i].y;
    printf("x: %f y: %f  z: %f \t ---> x: %d  |   y: %d \n",sword.verts[i].x,sword.verts[i].y,sword.verts[i].z,x,y);
    if(x > x_w || y > y_w || x < 0 || y < 0){
      continue;
    }
    output[x][y] = '.';
  }
  
  //clearScreen();
  /*for(int i=0;i<width;i++){
    for(int j=0;j<height;j++){
      putchar(output[i][j]);
    }
    putchar('\n');
  }*/
  freeObject(sword);
  return 0;
}
