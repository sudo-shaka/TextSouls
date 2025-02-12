#include "vertexMath.h"
#include "math.h"
#include <omp.h>
#ifndef M_PI
#define M_PI 3.1425926535
#endif

vec3 add3(const vec3 a, const vec3 b){
  return (vec3){a.x+b.x,a.y+b.y,a.z+b.z};
}

float dot3(const vec3 a, const vec3 b){
  return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

float magnitude(vec3 v){
  return sqrt(dot3(v, v));
}

vec3 normalize(const vec3 point3D){
  vec3 norm = point3D;
  float length = magnitude(point3D);
  if (length != 0.0f){
    norm.x = point3D.x / length;
    norm.y = point3D.y / length;
    norm.z = point3D.z / length;
  }
  return norm;
}

vec3 cross3(const vec3 a, const vec3 b){
  vec3 result;
  result.x = a.y * b.z - a.z * b.y;
  result.y = a.z * b.x - a.x * b.z;
  result.z = a.x * b.y - a.y * b.x;
  return result;
}

void vMat4toMat4(float result[4][4], const float a[16]){
  for (int i = 0; i < 4; i++){
    for (int j = 0; j < 4; j++){
      result[i][j] = a[i * 4 + j];
    }
  }
}

void mat4xvec4(float result[4], const float mat[4][4], const float vec[4]){
  #pragma omp parallel for
  for (int i = 0; i < 4; ++i){
    result[i] = 0.0f;
    for (int j = 0; j < 4; ++j){
      result[i] += mat[j][i] * vec[j];
    }
  }
}

void mat4Txvec4(float result[4], const float mat[4][4], const float vec[4]){
  for (int i = 0; i < 4; ++i){
    result[i] = 0.0f;
    for (int j = 0; j < 4; ++j){
      result[i] += mat[i][j] * vec[j];
    }
  }
}

void projection_matrix(float projMat[4][4], float fov, float aspectR, float nearPlane, float farPlane){
  float f = 1.0f / tanf(fov * 0.5f * (M_PI / 180.0f));
  for (int i = 0; i < 4; i++){
    for (int j = 0; j < 4; j++){
      projMat[i][j] = 0.0f;
    }
  }
  projMat[0][0] = f / aspectR;
  projMat[1][1] = f;
  projMat[2][2] = (farPlane + nearPlane) / (nearPlane - farPlane);
  projMat[2][3] = -1.0f;
  projMat[3][2] = (2.0f * farPlane * nearPlane) / (nearPlane - farPlane);
}

void lerp(float *result, const float *a, const float *b, float t, int count){
  #pragma omp simd
  for (int i = 0; i < count; i++){
    result[i] = a[i] + t * (b[i] - a[i]);
  }
}

vec3 lerp_vec3(vec3 a, vec3 b, float t){
  vec3 result;
  result.x = a.x + t * (b.x - a.x);
  result.y = a.y + t * (b.y - a.y);
  result.z = a.z + t * (b.z - a.z);
  return result;
}

void slerp(float *result, const float *q1, const float *q2, float t){
  float dot = q1[0] * q2[0] + q1[1] * q2[1] + q1[2] * q2[2] + q1[3] * q2[3];
  if (dot < 0.0f){
    dot = -dot;
    q2 = (float[]){-q2[0], -q2[1], -q2[2], -q2[3]};
  }
  float theta = acosf(dot);
  float sin_theta = sinf(theta);
  float weight1 = sinf((1.0f - t) * theta) / sin_theta;
  float weight2 = sinf(t * theta) / sin_theta;

  for (int i = 0; i < 4; i++){
    result[i] = weight1 * q1[i] + weight2 * q2[i];
  }
}

void rotate_matrix(float rotationMatrix[4][4], const float angle, const vec3 axis){
  normalize(axis);
  float cosAngle = cosf(angle);
  float sinAngle = sinf(angle);
  float oneMinusCos = 1.0f - cosAngle;

  // Compute the rotation matrix
  rotationMatrix[0][0] = cosAngle + axis.x * axis.x * oneMinusCos;
  rotationMatrix[0][1] = axis.x * axis.y * oneMinusCos - axis.z * sinAngle;
  rotationMatrix[0][2] = axis.x * axis.z * oneMinusCos + axis.y * sinAngle;
  rotationMatrix[0][3] = 0.0f;

  rotationMatrix[1][0] = axis.y * axis.x * oneMinusCos + axis.z * sinAngle;
  rotationMatrix[1][1] = cosAngle + axis.y * axis.y * oneMinusCos;
  rotationMatrix[1][2] = axis.y * axis.z * oneMinusCos - axis.x * sinAngle;
  rotationMatrix[1][3] = 0.0f;

  rotationMatrix[2][0] = axis.z * axis.x * oneMinusCos - axis.y * sinAngle;
  rotationMatrix[2][1] = axis.z * axis.y * oneMinusCos + axis.x * sinAngle;
  rotationMatrix[2][2] = cosAngle + axis.z * axis.z * oneMinusCos;
  rotationMatrix[2][3] = 0.0f;

  rotationMatrix[3][0] = 0.0f;
  rotationMatrix[3][1] = 0.0f;
  rotationMatrix[3][2] = 0.0f;
  rotationMatrix[3][3] = 1.0f;
}

vec3 sub3(const vec3 a, const vec3 b){
  return (vec3){a.x-b.x,a.y-b.y,a.z-b.z};
}

vec3 rotate_around_point(const vec3 point, const vec3 pivot, const float angle, const vec3 axis){
  vec3 transP = sub3(point, pivot);
  float translatedPoint[4] = {transP.x, transP.y, transP.z, 0.0f};
  float rotationMatrix[4][4], rotatedPoint[4];
  rotate_matrix(rotationMatrix, angle, axis);
  mat4xvec4(rotatedPoint, rotationMatrix, translatedPoint);

  vec3 result;
  result.x = rotatedPoint[0] + pivot.x;
  result.y = rotatedPoint[1] + pivot.y;
  result.z = rotatedPoint[2] + pivot.z;
  return result;
}

void look_at(const vec3 eye, const vec3 target, const vec3 up, float viewMatrix[4][4]){
  vec3 Z = sub3(eye, target);
  Z = normalize(Z);
  vec3 Y = up;
  vec3 X = cross3(Y, Z);
  Y = cross3(Z, X);
  X = normalize(X);
  Y = normalize(Y);

  // make rotation part of view matrix
  viewMatrix[0][0] = X.x;
  viewMatrix[0][1] = Y.x;
  viewMatrix[0][2] = Z.x;
  viewMatrix[0][3] = 0;
  viewMatrix[1][0] = X.y;
  viewMatrix[1][1] = Y.y;
  viewMatrix[1][2] = Z.y;
  viewMatrix[1][3] = 0;
  viewMatrix[2][0] = X.z;
  viewMatrix[2][1] = Y.z;
  viewMatrix[2][2] = Z.z;
  viewMatrix[2][3] = 0;
  viewMatrix[3][0] = -dot3(X, eye);
  viewMatrix[3][1] = -dot3(Y, eye);
  viewMatrix[3][2] = -dot3(Z, eye);
  viewMatrix[3][3] = 1.0f;
}

void point_3D_projection(
    const vec3 points3D[],
    vec2 points2D[],
    vec3 offset,
    int numPoints,
    const float projMat[4][4],
    const float viewMatrix[4][4],
    const float viewWidth,
    const float viewHeight){
  for (int i = 0; i < numPoints; i++){
    // convert 3d point to 3f homogenous coordinate
    float pointHomo[4] = {points3D[i].x+offset.x, points3D[i].y+offset.y, points3D[i].z+offset.z, 1.0f};
    // camera transformation
    float cameraPoint[4];
    mat4xvec4(cameraPoint, viewMatrix, pointHomo);
    // apply ProjectionMatrix
    float clipSpacePoint[4];
    mat4xvec4(clipSpacePoint, projMat, cameraPoint);
    // get normalized device coords (NDC)
    float ndcX = clipSpacePoint[0] / clipSpacePoint[3];
    float ndcY = clipSpacePoint[1] / clipSpacePoint[3];

    // set the point values
    points2D[i].x = (ndcX * 0.5f + 0.5f) * viewWidth;
    points2D[i].y = (1.0f - (ndcY * 0.5f + 0.5f)) * viewHeight;
  }
}

vec3 calc_face_normal(vec3 v1, vec3 v2, vec3 v3){
  vec3 edge1 = sub3(v2,v1);
  vec3 edge2 = sub3(v3,v1);
  //vec3 edge1 = {v2.x - v1.x, v2.y - v1.y, v2.z - v1.z};
  //vec3 edge2 = {v3.x - v1.x, v3.y - v1.y, v3.z - v1.z};
  vec3 normal = cross3(edge1, edge2);
  return normalize(normal);
}

void identityMat4(float mat[4][4]){
  for (int i = 0; i < 4; i++){
    for (int j = 0; j < 4; j++){
      mat[i][j] = 0.0f;
    }
  }
  for (int i = 0; i < 4; i++){
    mat[i][i] = 1.0f;
  }
}
void translate4(float mat[4][4], vec3 vec){
  identityMat4(mat);
  mat[0][3] = vec.x;
  mat[1][3] = vec.y;
  mat[2][3] = vec.z;
}

void mat4xmat4(float result[4][4], const float a[4][4], const float b[4][4]){
  for (int i = 0; i < 4; i++){
    for (int j = 0; j < 4; j++){
      result[i][j] = 0;
      for (int k = 0; k < 4; k++){
        result[i][j] = a[i][k] * b[k][j];
      }
    }
  }
}

void asVecmat4xmat4(float *result, const float *a, const float *b){
  for (int row = 0; row < 4; ++row){
    for (int col = 0; col < 4; ++col){
      result[row * 4 + col] = a[row * 4 + 0] * b[0 * 4 + col] +
                              a[row * 4 + 1] * b[1 * 4 + col] +
                              a[row * 4 + 2] * b[2 * 4 + col] +
                              a[row * 4 + 3] * b[3 * 4 + col];
    }
  }
}

void asVecTranspose(float *result, const float *input, int x, int y){
  for(int i=0;i<x;i++){
    for(int j=0;j<y;j++){
      result[j * x + i] = input[i * y + j];
    }
  }
}

void scale4(float mat[4][4], vec3 vec){
  mat[0][0] = vec.x;
  mat[1][1] = vec.y;
  mat[2][2] = vec.z;
}

float calc_luminesence(const vec3 lightSource, const vec3 facePoint, vec3 faceNormal){
  vec3 lightDir = {
      lightSource.x - facePoint.x,
      lightSource.y - facePoint.y,
      lightSource.z - facePoint.z};
  lightDir = normalize(lightDir);
  faceNormal = normalize(faceNormal);
  float lumin = dot3(faceNormal, lightDir);
  lumin /= 3.0f;
  return fmaxf(lumin, 0.0f);
}
char lumin_to_char(float l){
  l *= 11;
  int idx = (int)floor(l);
  const char *val_arr = ".,-~:;=!*#$@";
  const char val = val_arr[idx];
  return val;
}

int is_face_facing_point(vec3 v[3], vec3 p){
  vec3 edge1 = sub3(v[1],v[0]);
  vec3 edge2 = sub3(v[2],v[1]);
  vec3 norm = cross3(edge1,edge2);
  vec3 ap = sub3(p,v[0]);
  float dot = dot3(norm,ap);
  //return 1 for facing, -1 for away, 0 for on plane
  return dot > 0 ? 1 : (dot < 0 ? -1 : 0);
}

vec3 getCOM(const vec3 *verts, const int numVerts){
  vec3 com = {0.0f, 0.0f, 0.0f};
  for (int i = 0; i < numVerts; i++){
    com.x += verts[i].x;
    com.y += verts[i].y;
    com.z += verts[i].z;
  }
  com.x /= (float)numVerts;
  com.y /= (float)numVerts;
  com.z /= (float)numVerts;
  return com;
}

vec3 apply_quanternion(vec3 vec, float q[4]){
  float qConj[4] = {q[0],-q[1],-q[2],-q[3]};
  float qVector[4] = {0.0f,vec.x,vec.y,vec.z};

  float tmp[4];
  tmp[0] = q[0] * qVector[0] - q[1] * qVector[1] - q[2] * qVector[2] - q[3] * qVector[3];
  tmp[1] = q[0] * qVector[1] + q[1] * qVector[0] + q[2] * qVector[3] - q[3] * qVector[2];
  tmp[2] = q[0] * qVector[2] - q[1] * qVector[3] + q[2] * qVector[0] + q[3] * qVector[1];
  tmp[3] = q[0] * qVector[3] + q[1] * qVector[2] - q[2] * qVector[1] + q[3] * qVector[0];

  vec3 result;
  result.x = tmp[0] * qConj[1] + tmp[1] * qConj[0] + tmp[2] * qConj[3] - tmp[3] * qConj[2];
  result.y = tmp[0] * qConj[2] - tmp[1] * qConj[3] + tmp[2] * qConj[0] + tmp[3] * qConj[1];
  result.z = tmp[0] * qConj[3] + tmp[1] * qConj[2] - tmp[2] * qConj[1] + tmp[3] * qConj[0];
  return result;
}

vec3 calculate_ideal_look_at(vec3 target_position){
  vec3 result = {0.0f,0.5f,1.0f};
  result = add3(result,target_position);
  return result;
}

vec3 calculate_ideal_offset(vec3 target_position){
  vec3 result = {0.0f,1.0f,2.0f};
  result = add3(result, target_position);
  return result;
}
