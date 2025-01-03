#include "vertexMath.h"
#include "object.h"
#include <stdio.h>
#include <stdlib.h>

object parseObjFromFile(const char *filename){

  object newO;

  FILE *file = fopen(filename,"r");
  if(!file){
    newO.isRendered = 0;
    return newO;
  }

  int vCount = 0, fCount=0;
  vec3 *tmpVerts = malloc(20000*sizeof(vec3));
  face *tmpFace = malloc(20000*sizeof(face));

  char line[128];
  while (fgets(line,sizeof(line),file)){
    if(vCount > 1024 || fCount > 1024){
      newO.isRendered = 0;
      break;
    }
    if(line[0]  == 'v' && line[1] == ' '){
      vec3 v;
      sscanf(line,"v %f %f %f",&v.x,&v.y,&v.z);
      tmpVerts[vCount++] = v;
    }
    else if (line[0] == 'f' && line[1] == ' '){
      face f;
      sscanf(line,"f %d %d %d", &f.v1,&f.v2,&f.v3);
      tmpFace[fCount++]=f;
    }
  }

  fclose(file);

  newO.verts = realloc(tmpVerts, vCount *sizeof(vec3));
  newO.faces = realloc(tmpFace, fCount*sizeof(face));
  newO.nVerts = vCount;
  newO.nFaces = fCount;
  newO.isRendered = 1;
  return newO;
}

void freeObject(object o){
  free(o.faces);
  free(o.verts);
}

vec3 getCOM(const object o){
  vec3 com;
  com.x = com.y = com.z = 0.0f;
  for(int i=0;i<o.nVerts;i++){
    com.x += o.verts[i].x;
    com.y += o.verts[i].y;
    com.z += o.verts[i].z;
  }
  com.x /= (float)o.nVerts;
  com.y /= (float)o.nVerts;
  com.z /= (float)o.nVerts;
  return com;
}
