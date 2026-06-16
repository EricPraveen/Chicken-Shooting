#ifndef ENEMY_H
#define ENEMY_H

// Member 2 – Filling responsibilities:
// - Enemy body filling using FillEngine (flood fill, scan-line fill)

// Member 3 – Transformation responsibilities:
// - Enemy movement and positioning

struct Enemy {
    float x, y;           // Position
    float width, height;  // Size
    float vx, vy;         // Velocity
    float angle;          // Rotation angle
    int health;           // Health points
    bool active;          // Is enemy active

    Enemy();
    void initialize(float startX, float startY);
    void update(float deltaTime);
    void draw();
};

#endif // ENEMY_H
