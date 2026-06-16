#include "Egg.h"
#include "TransformEngine.h"
#include <OpenGL/gl.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <cmath>

Egg::Egg() : x(0), y(0), width(8), height(12), vx(0), vy(0), angle(0), active(false), lifetime(4.0f) {
}

void Egg::initialize(float startX, float startY, float dirX, float dirY) {
    x = startX;
    y = startY;
    
    // Normalize direction and set velocity
    float len = sqrtf(dirX * dirX + dirY * dirY);
    vx = (dirX / len) * 150;
    vy = (dirY / len) * 150 - 50;  // Slight downward bias
    
    angle = atan2f(dirY, dirX) * 180 / 3.14159f;
    active = true;
    lifetime = 4.0f;
}

void Egg::update(float deltaTime) {
    // Update position
    x += vx * deltaTime;
    y += vy * deltaTime;

    // Apply gravity
    vy -= 9.8f * deltaTime * 50;

    // Rotation based on velocity
    angle = atan2f(vy, vx) * 180 / 3.14159f;

    // Decrease lifetime
    lifetime -= deltaTime;
    if (lifetime <= 0) {
        active = false;
    }

    // Boundary checking
    if (y < 0) {
        active = false;
    }
}

void Egg::draw() {
    // Member 3 & 4 responsibility: Transform and clip egg during projectile phase
    if (active) {
        glPushMatrix();
        glTranslatef(x, y, 0);
        glRotatef(angle, 0, 0, 1);

        glColor3f(0.8f, 0.8f, 0.0f);
        glBegin(GL_POLYGON);
        glVertex2f(0, height/2);
        glVertex2f(width/2, 0);
        glVertex2f(0, -height/2);
        glVertex2f(-width/2, 0);
        glEnd();

        glPopMatrix();
    }
}
