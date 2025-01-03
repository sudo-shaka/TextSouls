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

void mat4Txvec4(float result[4], const float mat[4][4], const float vec[4]){
    for (int i = 0; i < 4; ++i) {
        result[i] = 0.0f;
        for (int j = 0; j < 4; ++j) {
            result[i] += mat[j][i] * vec[j];
        }
    }
}

void mat4xvec4(float result[4], const float mat[4][4], const float vec[4]){
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

vec3 normalize(const vec3 point3D){
  vec3 norm = point3D;
  float length = sqrt(dot3(point3D,point3D));
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
  mat4Txvec4(rotatedPoint, rotationMatrix, translatedPoint);

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
    mat4Txvec4(cameraPoint, viewMatrix, pointHomo);
    //apply ProjectionMatrix
    float clipSpacePoint[4];
    mat4Txvec4(clipSpacePoint,projMat,cameraPoint);
    //get normalized device coords (NDC)
    float ndcX = clipSpacePoint[0]/clipSpacePoint[3];
    float ndcY = clipSpacePoint[1]/clipSpacePoint[3];

    //set the point values
    points2D[i].x = (ndcX * 0.5f + 0.5f) * viewWidth;
    points2D[i].y = (1.0f - (ndcY * 0.5f + 0.5f)) * viewHeight;
  }
}
