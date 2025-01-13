#ifndef __MATHS__
#define __MATHS__

typedef struct vec3{
  float x,y,z;
}vec3;

typedef struct vec2{
  float x,y;
} vec2;

typedef struct face{
  int vertIdx[4];
  int normalIdx[4];
  int vertCount;
} face;

vec3 ftovec3(float x, float y, float z);
void rotateMat(float rotationMatrix[4][4],float angle,vec3 axis);
vec3 rotateAround(vec3 point, const vec3 pivot, float angle, const vec3 axis);
float dot3(vec3 a,vec3 b);
vec3 add3(vec3 a, vec3 b);
vec3 normalize(vec3 point3D);
float magnitude(vec3 v);
vec3 calcFaceNormal(vec3 v1,vec3 v2, vec3 v3);
float calcLumin(const vec3 lightSource,const vec3 facePoint,vec3 faceNormal);
char luminToChar(float l);
vec3 cross3(vec3 a,vec3 b);
void lookAt(
    const vec3 eye,
    const vec3 target,
    const vec3 up,
    float viewMatrix[4][4]
    );
void getProjectionMatrix(float projMat[4][4], float fov, float aspectR, float nearPlane, float farPlane);
void mat4Txvec4(float result[4],const float mat[4][4], const float vec[4]);
void mat4xvec4(float result[4],const float mat[4][4], const float vec[4]);
void point3DProjection(
    const vec3 *points3D,
    vec2 *points2D,
    int numPoints,
    const float projMat[4][4],
    const float viewMatrix[4][4],
    float viewWidth,
    float viewHeight);

void linearInterolation(float* result, const float*a, const float* b, float t, int count);
void sphericalLinInterp(float* result, const float* q1, const float* q2, float t);
void identityMat4(float mat[4][4]);
void translate4(float mat[4][4], vec3 vec);
void scale4(float mat[4][4],vec3 vec);
void mat4xmat4(float result[4][4], const float a[4][4],const float b[4][4]);
void asVecmat4xmat4(float* result, const float* a, const float* b);
vec3 getCOM(const vec3*, const int numVerts);
int isFaceFacingPoint(vec3 faceNormal,vec3 cameraPos, vec3 pointOnFace);
#endif
