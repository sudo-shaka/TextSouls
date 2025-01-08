#include "vertexMath.h"
#include "math.h"
#ifndef M_PI
#define M_PI 3.1425926535
#endif

vec3 ftovec3(const float x,const float y, const float z){
  vec3 vec;
  vec.x = x; vec.y = y; vec.z = z;
  return vec;
}

vec3 add3(const vec3 a,const vec3 b){
  vec3 sum;
  sum.x = a.x+b.x;
  sum.y = a.y+b.y;
  sum.z = a.z+b.z;
  return sum;
}

float dot3(const vec3 a,const vec3 b){
  return (a.x*b.x) + (a.y*b.y) + (a.z*b.z);
}

float magnitude(vec3 v){
  return sqrt(dot3(v,v));
}

vec3 normalize(const vec3 point3D){
  vec3 norm = point3D;
  float length = magnitude(point3D);
  if(length != 0.0f){
    norm.x = point3D.x/length;
    norm.y = point3D.y/length;
    norm.z = point3D.z/length;
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

void mat4xvec4(float result[4], const float mat[4][4], const float vec[4]){
    for (int i = 0; i < 4; ++i) {
        result[i] = 0.0f;
        for (int j = 0; j < 4; ++j) {
            result[i] += mat[j][i] * vec[j];
        }
    }
}

void mat4Txvec4(float result[4], const float mat[4][4], const float vec[4]){
    for (int i = 0; i < 4; ++i) {
        result[i] = 0.0f;
        for (int j = 0; j < 4; ++j) {
            result[i] += mat[i][j] * vec[j];
        }
    }
}

void getProjectionMatrix(float projMat[4][4], float fov, float aspectR, float nearPlane, float farPlane){
  float f = 1.0f / tanf(fov * 0.5f * (M_PI/180.0f));
  for(int i=0;i<4;i++){
    for(int j=0;j<4;j++){
      projMat[i][j] = 0.0f;
    }
  }
  projMat[0][0] = f/aspectR;
  projMat[1][1] = f;
  projMat[2][2] = (farPlane + nearPlane)/(nearPlane - farPlane);
  projMat[2][3] = -1.0f;
  projMat[3][2] = (2.0f*farPlane*nearPlane)/(nearPlane-farPlane);
}

void linearInterolation(float* result, const float*a, const float* b, float t, int count){
  for(int i = 0;i<count;i++){
    result[i] = a[i] + t * (b[i] - a[i]);
  }
}

void sphericalLinInterp(float* result, const float* q1, const float* q2, float t){
  float dot = q1[0] * q2[0] + q1[1] * q2[1] + q1[2] * q2[2] + q1[3] * q2[3];
  if (dot < 0.0f) {
      dot = -dot;
      q2 = (float[]){ -q2[0], -q2[1], -q2[2], -q2[3] };
  }
  float theta = acosf(dot);
  float sin_theta = sinf(theta);
  float weight1 = sinf((1.0f - t) * theta) / sin_theta;
  float weight2 = sinf(t * theta) / sin_theta;

  for (int i = 0; i < 4; i++) {
      result[i] = weight1 * q1[i] + weight2 * q2[i];
  }
}

void rotateMat(float rotationMatrix[4][4],const float angle,const vec3 axis){
  normalize(axis);
  float cosAngle = cosf(angle);
  float sinAngle = sinf(angle);
  float oneMinusCos = 1.0f-cosAngle;

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

vec3 sub3(const vec3 a,const vec3 b){
  vec3 sub;
  sub.x = a.x - b.x;
  sub.y = a.y - b.y;
  sub.z = a.z - b.z;
  return sub;
}

double DegToRad(const float angle){
  return angle * 180.0f / M_PI;
}

vec3 rotateAround(const vec3 point, const vec3 pivot,const float angle, const vec3 axis){
  vec3 transP = sub3(point, pivot);
  float translatedPoint[4] =  {transP.x, transP.y, transP.z, 0.0f};
  float rotationMatrix[4][4], rotatedPoint[4];
  rotateMat(rotationMatrix, angle,axis);
  mat4xvec4(rotatedPoint, rotationMatrix, translatedPoint);

  vec3 result;
  result.x = rotatedPoint[0] + pivot.x;
  result.y = rotatedPoint[1] + pivot.y;
  result.z = rotatedPoint[2] + pivot.z;
  return result;
}

void lookAt(
    const vec3 eye,
    const vec3 target,
    const vec3 up,
    float viewMatrix[4][4]
    ){
  vec3 Z = sub3(eye,target);
  Z = normalize(Z);
  vec3 Y = up;
  vec3 X = cross3(Y,Z);
  Y = cross3(Z,X);
  X = normalize(X);
  Y = normalize(Y);

  //make rotation part of view matrix
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
  viewMatrix[3][0] = -dot3(X,eye);
  viewMatrix[3][1] = -dot3(Y,eye);
  viewMatrix[3][2] = -dot3(Z,eye);
  viewMatrix[3][3] = 1.0f;
}

void point3DProjection(
    const vec3 points3D[],
    vec2 points2D[],
    int numPoints,
    const float projMat[4][4],
    const float viewMatrix[4][4],
    const float viewWidth,
    const float viewHeight
    ){
  for(int i=0;i<numPoints;i++){
    //convert 3d point to 3f homogenous coordinate
    float pointHomo[4] = {points3D[i].x, points3D[i].y, points3D[i].z, 1.0f};
    //camera transformation
    float cameraPoint[4];
    mat4xvec4(cameraPoint, viewMatrix, pointHomo);
    //apply ProjectionMatrix
    float clipSpacePoint[4];
    mat4xvec4(clipSpacePoint,projMat,cameraPoint);
    //get normalized device coords (NDC)
    float ndcX = clipSpacePoint[0]/clipSpacePoint[3];
    float ndcY = clipSpacePoint[1]/clipSpacePoint[3];

    //set the point values
    points2D[i].x = (ndcX * 0.5f + 0.5f) * viewWidth;
    points2D[i].y = (1.0f - (ndcY * 0.5f + 0.5f)) * viewHeight;
  }
}

vec3 calcFaceNormal(vec3 v1,vec3 v2, vec3 v3){
  vec3 edge1 = {v2.x - v1.x, v2.y - v1.y, v2.z - v1.z};
  vec3 edge2 = {v3.x - v1.x, v3.y - v1.y, v3.z - v1.z};

  // Compute the cross product of the edges
  vec3 normal = cross3(edge1, edge2);

  // Normalize the normal vector
  return normalize(normal);
}

void identityMat4(float mat[4][4]){
  for(int i=0;i<4;i++){
    for(int j=0;j<4;j++){
      mat[i][j] = 0.0f;
    }
  }
  for(int i=0;i<4;i++){
    mat[i][i] = 1.0f;
  }
}
void translate4(float mat[4][4], vec3 vec){
  identityMat4(mat);
  mat[0][3] = vec.x;
  mat[1][3] = vec.y;
  mat[2][3] = vec.z;
}
void mat4xmat4(float result[4][4], const float a[4][4],const float b[4][4]){
  for(int i=0;i<4;i++){
    for(int j=0;j<4;j++){
      result[i][j] = 0;
      for(int k=0; k<4;k++){
        result[i][j] = a[i][k] * b[k][j];
      }
    }
  }
}
void scale4(float mat[4][4],vec3 vec){
  mat[0][0] = vec.x;
  mat[1][1] = vec.y;
  mat[2][2] = vec.z;
}

float calcLumin(const vec3 lightSource,const vec3 facePoint,vec3 faceNormal){
  vec3 lightDir = {
      lightSource.x - facePoint.x,
      lightSource.y - facePoint.y,
      lightSource.z - facePoint.z};
  lightDir = normalize(lightDir);
  faceNormal = normalize(faceNormal);
  float lumin = dot3(faceNormal,lightDir);
  return fmaxf(lumin,0.0f);
}
char luminToChar(float l){
  l *= 11;
  int idx = (int)l;
  char val = ".,-~:;=!*#$@"[idx];
  return val;
}

int isFaceFacingPoint(vec3 faceNormal,vec3 cameraPos, vec3 pointOnFace){
  vec3 viewVector = sub3(cameraPos, pointOnFace);
  float dot = dot3(faceNormal,viewVector);
  return dot > 0;
}

vec3 getCOM(const vec3* verts, const int numVerts){
  vec3 com = {0.0f,0.0f,0.0f};
  for(int i=0;i<numVerts;i++){
    com.x += verts[i].x;
    com.y += verts[i].y;
    com.z += verts[i].z;
  }
  com.x /= (float)numVerts;
  com.y /= (float)numVerts;
  com.z /= (float)numVerts;
  return com;
}
