#ifndef TRANSFORMENGINE_H
#define TRANSFORMENGINE_H

// Member 3 – Transformation System
// Computer Graphics Concepts:
// - Translation
// - Rotation
// - Scaling
// - Composite Transformation

#include <cmath>

struct Vector2D {
    float x, y;
    
    Vector2D() : x(0), y(0) {}
    Vector2D(float x, float y) : x(x), y(y) {}
    
    Vector2D operator+(const Vector2D& other) const {
        return Vector2D(x + other.x, y + other.y);
    }
    
    Vector2D operator-(const Vector2D& other) const {
        return Vector2D(x - other.x, y - other.y);
    }
    
    Vector2D operator*(float scalar) const {
        return Vector2D(x * scalar, y * scalar);
    }
};

class TransformEngine {
public:
    TransformEngine();
    ~TransformEngine();

    // Translation transformation
    Vector2D translate(const Vector2D& point, float tx, float ty);

    // Rotation transformation (angle in degrees)
    Vector2D rotate(const Vector2D& point, float angleDegrees, const Vector2D& pivotPoint = Vector2D(0, 0));

    // Scaling transformation
    Vector2D scale(const Vector2D& point, float scaleX, float scaleY, const Vector2D& pivotPoint = Vector2D(0, 0));

    // Composite transformation (translate, then rotate, then scale)
    Vector2D compositeTransform(const Vector2D& point,
                                float tx, float ty,
                                float rotateDegrees,
                                float scaleX, float scaleY);

    // Convert degrees to radians
    static float degreesToRadians(float degrees);

    // Convert radians to degrees
    static float radiansToDegrees(float radians);

private:
    // 2D transformation matrix operations
    void multiplyMatrix(float a[3][3], float b[3][3], float result[3][3]);
};

#endif // TRANSFORMENGINE_H
