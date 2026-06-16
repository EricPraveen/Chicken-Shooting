#ifndef UTILS_H
#define UTILS_H

#include <cmath>

// Utility functions for math and collision detection

struct Rect {
    float x, y, width, height;
};

struct Circle {
    float x, y, radius;
};

class Utils {
public:
    // Collision detection
    static bool rectCollision(const Rect& a, const Rect& b);
    static bool circleCollision(const Circle& a, const Circle& b);
    static bool rectCircleCollision(const Rect& rect, const Circle& circle);

    // Math helpers
    static float distance(float x1, float y1, float x2, float y2);
    static float dot(float x1, float y1, float x2, float y2);
    static float normalize(float& x, float& y);
    static float clamp(float value, float min, float max);
    static float lerp(float a, float b, float t);

    // Angle helpers (in degrees)
    static float angleBetween(float x1, float y1, float x2, float y2);
    static float normalizeAngle(float angle);
};

#endif // UTILS_H
