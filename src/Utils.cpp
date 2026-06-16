#include "Utils.h"

// Collision detection utilities
bool Utils::rectCollision(const Rect& a, const Rect& b) {
    return !(a.x + a.width < b.x || a.x > b.x + b.width ||
             a.y + a.height < b.y || a.y > b.y + b.height);
}

bool Utils::circleCollision(const Circle& a, const Circle& b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float dist = sqrtf(dx * dx + dy * dy);
    return dist < (a.radius + b.radius);
}

bool Utils::rectCircleCollision(const Rect& rect, const Circle& circle) {
    float closestX = clamp(circle.x, rect.x, rect.x + rect.width);
    float closestY = clamp(circle.y, rect.y, rect.y + rect.height);
    float dx = circle.x - closestX;
    float dy = circle.y - closestY;
    return (dx * dx + dy * dy) < (circle.radius * circle.radius);
}

// Math helpers
float Utils::distance(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx * dx + dy * dy);
}

float Utils::dot(float x1, float y1, float x2, float y2) {
    return x1 * x2 + y1 * y2;
}

float Utils::normalize(float& x, float& y) {
    float len = sqrtf(x * x + y * y);
    if (len > 0) {
        x /= len;
        y /= len;
    }
    return len;
}

float Utils::clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

float Utils::lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

float Utils::angleBetween(float x1, float y1, float x2, float y2) {
    float angle = atan2f(y2 - y1, x2 - x1) * 180 / 3.14159265359f;
    return angle;
}

float Utils::normalizeAngle(float angle) {
    while (angle < 0) angle += 360;
    while (angle >= 360) angle -= 360;
    return angle;
}
