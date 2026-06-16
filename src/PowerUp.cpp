#include "PowerUp.h"
#include "FillEngine.h"
#include <OpenGL/gl.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

PowerUp::PowerUp() : x(0), y(0), width(15), height(15), type(0), active(true), lifetime(6.0f) {
}

void PowerUp::initialize(float startX, float startY, int powerUpType) {
    x = startX;
    y = startY;
    type = powerUpType;
    active = true;
    lifetime = 6.0f;
}

void PowerUp::update(float deltaTime) {
    // Decrease lifetime
    lifetime -= deltaTime;
    if (lifetime <= 0) {
        active = false;
    }
}

void PowerUp::draw() {
    // Member 2 responsibility: Fill power-up using styling algorithms
    if (active) {
        glColor3f(0.5f, 0.5f, 1.0f);
        glBegin(GL_QUADS);
        glVertex2f(x - width/2, y - height/2);
        glVertex2f(x + width/2, y - height/2);
        glVertex2f(x + width/2, y + height/2);
        glVertex2f(x - width/2, y + height/2);
        glEnd();
    }
}
