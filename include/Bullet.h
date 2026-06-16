#ifndef BULLET_H
#define BULLET_H

// Member 1 – Rendering responsibilities:
// - Bullet drawing using Renderer primitives

// Member 3 – Transformation responsibilities:
// - Bullet movement and trajectory

// Member 4 – Clipping responsibilities:
// - Bullet clipping to viewport

struct Bullet {
    float x, y;           // Position
    float width, height;  // Size
    float vx, vy;         // Velocity
    float angle;          // Rotation angle
    bool active;          // Is bullet active
    float lifetime;       // Time before despawn

    Bullet();
    void initialize(float startX, float startY, float dirX, float dirY);
    void update(float deltaTime);
    void draw();
};

#endif // BULLET_H
