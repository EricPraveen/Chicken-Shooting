#ifndef PLAYER_H
#define PLAYER_H

// Member 1 – Rendering responsibilities:
// - Player drawing using Renderer primitives (lines, circles)

// Member 3 – Transformation responsibilities:
// - Player movement and positioning
// - Rotation effects

struct Player {
    float x, y;           // Position
    float width, height;  // Size
    float vx, vy;         // Velocity
    float angle;          // Rotation angle
    int health;           // Health points
    bool canJump;         // Jump state

    Player();
    void initialize(float startX, float startY);
    void update(float deltaTime);
    void draw();
};

#endif // PLAYER_H
