#ifndef POWERUP_H
#define POWERUP_H

// Member 2 – Filling responsibilities:
// - PowerUp filling and styling

struct PowerUp {
    float x, y;           // Position
    float width, height;  // Size
    int type;             // PowerUp type
    bool active;          // Is active
    float lifetime;       // Time before despawn

    PowerUp();
    void initialize(float startX, float startY, int powerUpType);
    void update(float deltaTime);
    void draw();
};

#endif // POWERUP_H
