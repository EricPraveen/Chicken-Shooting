#include "TransformEngine.h"

// Member 3 – Transformation System
// Translation, Rotation, Scaling, Composite Transformation

TransformEngine::TransformEngine() {
}

TransformEngine::~TransformEngine() {
}

Vector2D TransformEngine::translate(const Vector2D& point, float tx, float ty) {
    // Translation transformation: P' = P + T
    return Vector2D(point.x + tx, point.y + ty);
}

Vector2D TransformEngine::rotate(const Vector2D& point, float angleDegrees, const Vector2D& pivotPoint) {
    // Rotation transformation around a pivot point
    // Translate to origin, rotate, translate back
    float angleRad = degreesToRadians(angleDegrees);
    float cosA = cosf(angleRad);
    float sinA = sinf(angleRad);

    // Translate to origin
    float x = point.x - pivotPoint.x;
    float y = point.y - pivotPoint.y;

    // Rotate
    float rotX = x * cosA - y * sinA;
    float rotY = x * sinA + y * cosA;

    // Translate back
    return Vector2D(rotX + pivotPoint.x, rotY + pivotPoint.y);
}

Vector2D TransformEngine::scale(const Vector2D& point, float scaleX, float scaleY, const Vector2D& pivotPoint) {
    // Scaling transformation around a pivot point
    // Translate to origin, scale, translate back
    float x = point.x - pivotPoint.x;
    float y = point.y - pivotPoint.y;

    float scaledX = x * scaleX + pivotPoint.x;
    float scaledY = y * scaleY + pivotPoint.y;

    return Vector2D(scaledX, scaledY);
}

Vector2D TransformEngine::compositeTransform(const Vector2D& point,
                                            float tx, float ty,
                                            float rotateDegrees,
                                            float scaleX, float scaleY) {
    // Composite transformation: Translate -> Rotate -> Scale
    Vector2D temp = translate(point, tx, ty);
    temp = rotate(temp, rotateDegrees);
    temp = scale(temp, scaleX, scaleY);
    return temp;
}

float TransformEngine::degreesToRadians(float degrees) {
    return degrees * 3.14159265359f / 180.0f;
}

float TransformEngine::radiansToDegrees(float radians) {
    return radians * 180.0f / 3.14159265359f;
}

void TransformEngine::multiplyMatrix(float a[3][3], float b[3][3], float result[3][3]) {
    // 3x3 matrix multiplication for 2D transformations
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            result[i][j] = 0;
            for (int k = 0; k < 3; k++) {
                result[i][j] += a[i][k] * b[k][j];
            }
        }
    }
}
