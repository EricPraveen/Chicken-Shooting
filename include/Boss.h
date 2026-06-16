#ifndef BOSS_H
#define BOSS_H

// Member 2 – Filling responsibilities:
// - Boss body filling using advanced fill algorithms

// Member 3 – Transformation responsibilities:
// - Boss movement, rotation, and scaling effects

struct Boss {
    float x, y;           // Position
    float width, height;  // Size
    float vx, vy;         // Velocity
    float angle;          // Rotation angle
    float scale;          // Scale factor
    int health;           // Health points
    float attackTimer;    // Attack cooldown
    bool active;          // Is boss active

    Boss();
    void initialize(float startX, float startY);
    void update(float deltaTime);
    void draw();
};

#endif // BOSS_H
