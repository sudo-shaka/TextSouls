
#ifndef __OBJECT_HEAD__
#define __OBJECT_HEAD__
#include "vertexMath.h"

typedef struct object{
  vec3 *verts;
  face *faces;
  char *displayChar;
  int nFaces;
  int nVerts;
  int isRendered;
} object;

#include "vertexMath.h"
object parseObjFromFile(const char *filename);
vec3 getObjCOM(object o);
void freeObject(object o);
void updateDisplayChars(object o,const vec3 lightSource);

#endif
