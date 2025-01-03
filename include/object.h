
#ifndef __OBJECT_HEAD__
#define __OBJECT_HEAD__
#include "vertexMath.h"

typedef struct object{
  vec3 *verts;
  face *faces;
  int nFaces;
  int nVerts;
  int isRendered;
} object;

#include "vertexMath.h"
object parseFromFile(const char *filename);
vec3 getCOM(object o);
void freeObject(object o);

#endif
