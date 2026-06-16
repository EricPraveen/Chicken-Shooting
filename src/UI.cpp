#include "UI.h"
#include "FillEngine.h"
#include "ClipEngine.h"
#include <OpenGL/gl.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <cstdio>

UI::UI() : score(0), health(100), level(1), showPauseMenu(false), showGameOver(false) {
}

void UI::initialize() {
    score = 0;
    health = 100;
    level = 1;
    showPauseMenu = false;
    showGameOver = false;
}

void UI::update(float deltaTime) {
    // UI animations and updates would go here
}

void UI::draw() {
    // Member 2 responsibility: Fill UI elements
    // Member 4 responsibility: Manage UI viewport and clipping

    // Draw health bar background
    drawHealthBar(10, 570, 100, 20, health, 100);

    // Draw score
    drawScore(550, 570);
}

void UI::drawHealthBar(float x, float y, float width, float height, int currentHealth, int maxHealth) {
    // Draw background (red)
    glColor3f(0.5f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();

    // Draw health bar (green)
    float healthPercent = (float)currentHealth / maxHealth;
    glColor3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width * healthPercent, y);
    glVertex2f(x + width * healthPercent, y + height);
    glVertex2f(x, y + height);
    glEnd();

    // Draw border
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

void UI::drawScore(float x, float y) {
    // Simple score display
    glColor3f(1.0f, 1.0f, 1.0f);
    
    // In a real implementation, this would use text rendering
    // For now, just render a rectangle as placeholder
    glBegin(GL_QUADS);
    glVertex2f(x - 50, y);
    glVertex2f(x + 50, y);
    glVertex2f(x + 50, y + 30);
    glVertex2f(x - 50, y + 30);
    glEnd();
}
