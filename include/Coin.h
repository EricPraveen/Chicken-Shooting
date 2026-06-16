#ifndef COIN_H
#define COIN_H

// Member 2 – Filling responsibilities:
// - Coin filling using boundary fill or flood fill

// Member 3 – Transformation responsibilities:
// - Coin rotation and scaling for animation

struct Coin {
    float x, y;           // Position
    float radius;         // Radius
    float angle;          // Rotation angle
    float rotationSpeed;  // Rotation speed
    int value;            // Point value
    bool active;          // Is coin active
    float lifetime;       // Time before despawn

    Coin();
    void initialize(float startX, float startY, int pointValue = 10);
    void update(float deltaTime);
    void draw();
};

#endif // COIN_H
