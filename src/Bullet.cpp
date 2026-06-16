#include "Bullet.h"
#include "Renderer.h"
#include <OpenGL/gl.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <cmath>

Bullet::Bullet() : x(0), y(0), width(5), height(5), vx(0), vy(0), angle(0), active(false), lifetime(5.0f) {
}

void Bullet::initialize(float startX, float startY, float dirX, float dirY) {
    x = startX;
    y = startY;
    
    // Normalize direction and set velocity
    float len = sqrtf(dirX * dirX + dirY * dirY);
    vx = (dirX / len) * 200;
    vy = (dirY / len) * 200;
    
    angle = atan2f(dirY, dirX) * 180 / 3.14159f;
    active = true;
    lifetime = 5.0f;
}

void Bullet::update(float deltaTime) {
    // Update position
    x += vx * deltaTime;
    y += vy * deltaTime;

    // Decrease lifetime
    lifetime -= deltaTime;
    if (lifetime <= 0) {
        active = false;
    }
}

void Bullet::draw() {
    // Member 1 responsibility: Draw bullet using Renderer primitives (circle)
    if (active) {
        glColor3f(1.0f, 1.0f, 0.0f);
        glBegin(GL_TRIANGLES);
        glVertex2f(x + 3, y);
        glVertex2f(x - 3, y - 2);
        glVertex2f(x - 3, y + 2);
        glEnd();
    }
}
