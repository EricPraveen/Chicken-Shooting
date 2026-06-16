#ifndef UI_H
#define UI_H

// Member 2 – Filling responsibilities:
// - UI element filling (health bars, score backgrounds)

// Member 4 – Clipping responsibilities:
// - UI viewport management and text clipping

struct UI {
    int score;
    int health;
    int level;
    bool showPauseMenu;
    bool showGameOver;

    UI();
    void initialize();
    void update(float deltaTime);
    void draw();
    void drawHealthBar(float x, float y, float width, float height, int currentHealth, int maxHealth);
    void drawScore(float x, float y);
};

#endif // UI_H
