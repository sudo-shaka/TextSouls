#include "vertexMath.h"
#include "object.h"
#include <stdio.h>
#include <stdlib.h>

object parseObjFromFile(const char *filename){

  object newO;
  newO.isRendered = 0;

  FILE *file = fopen(filename, "r");
  if (!file)
    return newO;

  int vCount = 0, fCount = 0;
  int maxSize = 4096;
  vec3 *tmpVerts = malloc(maxSize * sizeof(vec3));
  face *tmpFace = malloc(maxSize * sizeof(face));
  char *displayChars = malloc(maxSize * sizeof(char));

  char line[256];
  vec3 lowest_pos = {0.0f, 0.0f, 1e10f};
  while (fgets(line, sizeof(line), file)){
    if (vCount > maxSize || fCount > maxSize)
      return newO;
    if (line[0] == 'v' && line[1] == ' '){
      vec3 v;
      sscanf(line, "v %f %f %f", &v.x, &v.z, &v.y);
      if (v.z < lowest_pos.z) lowest_pos = v;
      tmpVerts[vCount++] = v;
      displayChars[vCount] = '.';
    } else if (line[0] == 'f' && line[1] == ' '){
      face f;
      f.vertCount = 0;
      int nVerts = sscanf(line, "f %d//%d %d//%d %d//%d %d//%d",
                          &f.vertIdx[0],
                          &f.normalIdx[0],
                          &f.vertIdx[1],
                          &f.normalIdx[1],
                          &f.vertIdx[2],
                          &f.normalIdx[2],
                          &f.vertIdx[3],
                          &f.normalIdx[3]);
      if (nVerts == 8){
        f.vertCount = 4;
        }
      else if (nVerts == 6){
        f.vertCount = 3;
        f.vertIdx[3] = -2;
        f.normalIdx[3] = -2;
      }
      else {
        printf("Cant parse string\n");
      }
      tmpFace[fCount++] = f;
    }
  }
  fclose(file);
  if (tmpVerts != NULL)
    newO.verts = realloc(tmpVerts, vCount * sizeof(vec3));
  if (tmpFace != NULL)
    newO.faces = realloc(tmpFace, fCount * sizeof(face));
  if (displayChars != NULL)
    newO.displayChar = realloc(displayChars, vCount * sizeof(char));
  newO.nVerts = vCount;
  newO.nFaces = fCount;
  newO.displayChar = displayChars;
  newO.lowestPos = lowest_pos;
  newO.isRendered = 1;
  return newO;
}

void updateDisplayChars(object o, vec3 lightSource){
  vec3 Faceverts[3];
  for (int fi = 0; fi < o.nFaces; fi++){
    Faceverts[0] = o.verts[o.faces[fi].vertIdx[0]];
    Faceverts[1] = o.verts[o.faces[fi].vertIdx[1]];
    Faceverts[2] = o.verts[o.faces[fi].vertIdx[2]];
    vec3 faceNormal = calc_face_normal(Faceverts[0], Faceverts[1], Faceverts[2]);
    float lumen = calc_luminesence(lightSource, Faceverts[0], faceNormal);
    char lChar = lumin_to_char(lumen);
    for (int vi = 0; vi < o.faces[fi].vertCount; vi++){
      o.displayChar[o.faces[fi].vertIdx[vi]] = lChar;
    }
  }
}

void freeObject(object o){
  free(o.faces);
  free(o.verts);
  free(o.displayChar);
}

vec3 getObjCOM(const object o){
  vec3 com;
  com.x = com.y = com.z = 0.0f;
  for (int i = 0; i < o.nVerts; i++){
    com.x += o.verts[i].x;
    com.y += o.verts[i].y;
    com.z += o.verts[i].z;
  }
  com.x /= (float)o.nVerts;
  com.y /= (float)o.nVerts;
  com.z /= (float)o.nVerts;
  return com;
}
