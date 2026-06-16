#ifndef EGG_H
#define EGG_H

// Member 3 – Transformation responsibilities:
// - Egg movement and rotation during projectile phase

// Member 4 – Clipping responsibilities:
// - Egg projectile clipping checks

struct Egg {
    float x, y;           // Position
    float width, height;  // Size
    float vx, vy;         // Velocity
    float angle;          // Rotation angle
    bool active;          // Is egg active
    float lifetime;       // Time before despawn

    Egg();
    void initialize(float startX, float startY, float dirX, float dirY);
    void update(float deltaTime);
    void draw();
};

#endif // EGG_H
