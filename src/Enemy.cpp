#include "Enemy.h"
#include "FillEngine.h"
#include <OpenGL/gl.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

Enemy::Enemy() : x(0), y(0), width(25), height(25), vx(0), vy(0), angle(0), health(50), active(true) {
}

void Enemy::initialize(float startX, float startY) {
    x = startX;
    y = startY;
    health = 50;
    active = true;
}

void Enemy::update(float deltaTime) {
    // Update position based on velocity
    x += vx * deltaTime;
    y += vy * deltaTime;

    // Simple AI behavior - move towards player (x = 400)
    if (x < 400) {
        vx = 30;
    } else {
        vx = -30;
    }
}

void Enemy::draw() {
    // Member 2 responsibility: Fill enemy body using FillEngine
    // This is where FillEngine.floodFill() or FillEngine.scanLineFill() would be called
    glColor3f(1.0f, 0.5f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(x - width/2, y - height/2);
    glVertex2f(x + width/2, y - height/2);
    glVertex2f(x + width/2, y + height/2);
    glVertex2f(x - width/2, y + height/2);
    glEnd();
}
