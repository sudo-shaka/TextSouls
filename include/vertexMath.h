#ifndef __MATHS__
#define __MATHS__

typedef struct vec3{
  float x, y, z;
} vec3;

typedef struct vec2{
  float x, y;
} vec2;

typedef struct face{
  int vertIdx[4];
  int normalIdx[4];
  int vertCount;
} face;

void vMat4toMat4(float result[4][4], const float a[16]);
void rotate_matrix(float rotationMatrix[4][4], float angle, vec3 axis);
vec3 rotate_around_point(vec3 point, const vec3 pivot, float angle, const vec3 axis);
float dot3(vec3 a, vec3 b);
vec3 add3(vec3 a, vec3 b);
vec3 sub3(vec3 a, vec3 b);
vec3 normalize(vec3 point3D);
float magnitude(vec3 v);
vec3 calc_face_normal(vec3 v1, vec3 v2, vec3 v3);
float calc_luminesence(const vec3 lightSource, const vec3 facePoint, vec3 faceNormal);
char lumin_to_char(float l);
vec3 cross3(vec3 a, vec3 b);
void look_at(
    const vec3 eye,
    const vec3 target,
    const vec3 up,
    float viewMatrix[4][4]);
void projection_matrix(float projMat[4][4], float fov, float aspectR, float nearPlane, float farPlane);
void mat4Txvec4(float result[4], const float mat[4][4], const float vec[4]);
void mat4xvec4(float result[4], const float mat[4][4], const float vec[4]);
void point_3D_projection(
    const vec3 *points3D,
    vec2 *points2D,
    vec3 offset,
    int numPoints,
    const float projMat[4][4],
    const float viewMatrix[4][4],
    float viewWidth,
    float viewHeight);

void lerp(float *result, const float *a, const float *b, float t, int count);
vec3 lerp_vec3(vec3 a, vec3 b, float t);
void slerp(float *result, const float *q1, const float *q2, float t);
vec3 apply_quanternion(vec3 vec, float q[4]);
void identityMat4(float mat[4][4]);
//void translate4(vec3 mat[4][4], vec3 vec);
void scale4(float mat[4][4], vec3 vec);
void mat4xmat4(float result[4][4], const float a[4][4], const float b[4][4]);
void asVecmat4xmat4(float *result, const float *a, const float *b);
vec3 getCOM(const vec3 *, const int numVerts);
int isFaceFacingPoint(vec3 faceNormal, vec3 cameraPos, vec3 pointOnFace);
vec3 calculate_ideal_look_at(vec3 target_pos);
vec3 calculate_ideal_offset(vec3 target_pos);
#endif
