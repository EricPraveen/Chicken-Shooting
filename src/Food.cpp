#include "Food.h"
#include "FillEngine.h"
#include <OpenGL/gl.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <cmath>

Food::Food() : x(0), y(0), width(15), height(15), angle(0), bobOffset(0), healthGain(20), active(true), lifetime(8.0f) {
}

void Food::initialize(float startX, float startY, int health) {
    x = startX;
    y = startY;
    healthGain = health;
    active = true;
    lifetime = 8.0f;
}

void Food::update(float deltaTime) {
    // Bobbing animation
    static float time = 0;
    time += deltaTime;
    bobOffset = sinf(time * 2) * 5;

    // Rotation animation
    angle += 90 * deltaTime;
    if (angle > 360) angle -= 360;

    // Decrease lifetime
    lifetime -= deltaTime;
    if (lifetime <= 0) {
        active = false;
    }
}

void Food::draw() {
    // Member 2 responsibility: Fill food using scan-line fill or boundary fill
    if (active) {
        glPushMatrix();
        glTranslatef(x, y + bobOffset, 0);
        glRotatef(angle, 0, 0, 1);

        glColor3f(0.0f, 1.0f, 0.0f);
        glBegin(GL_QUADS);
        glVertex2f(-width/2, -height/2);
        glVertex2f(width/2, -height/2);
        glVertex2f(width/2, height/2);
        glVertex2f(-width/2, height/2);
        glEnd();

        glPopMatrix();
    }
}
