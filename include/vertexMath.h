#ifndef __MATHS__
#define __MATHS__

typedef struct vec3{
  float x,y,z;
}vec3;

typedef struct vec2{
  float x,y;
} vec2;

typedef struct face{
  int v1,v2,v3;
} face;

vec3 ftovec3(float x, float y, float z);
void rotateMat(float rotationMatrix[4][4],float angle,vec3 axis);
vec3 rotateAround(vec3 point, const vec3 pivot, float angle, const vec3 axis);
float dot3(vec3 a,vec3 b);
vec3 add3(vec3 a, vec3 b);
vec3 normalize(vec3 point3D);
vec3 cross3(vec3 a,vec3 b);
void lookAt(
    const vec3 eye,
    const vec3 target,
    const vec3 up,
    float viewMatrix[4][4]
    );
void getProjectionMatrix(float projMat[4][4], float fov, float aspectR, float nearPlane, float farPlane);
void mat4xvec4(float result[4],const float mat[4][4], const float vec[4]);
void point3DProjection(
    const vec3 *points3D,
    vec2 **points2D,
    int numPoints,
    const float projMat[4][4],
    const float viewMatrix[4][4],
    float viewWidth,
    float viewHeight);

#endif
