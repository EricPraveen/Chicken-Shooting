#ifndef FOOD_H
#define FOOD_H

// Member 2 – Filling responsibilities:
// - Food filling using scan-line fill or boundary fill

// Member 3 – Transformation responsibilities:
// - Food animation and movement

struct Food {
    float x, y;           // Position
    float width, height;  // Size
    float angle;          // Rotation angle
    float bobOffset;      // Bobbing animation offset
    int healthGain;       // Health points gained
    bool active;          // Is food active
    float lifetime;       // Time before despawn

    Food();
    void initialize(float startX, float startY, int health = 20);
    void update(float deltaTime);
    void draw();
};

#endif // FOOD_H
