#include "Coin.h"
#include "FillEngine.h"
#include "TransformEngine.h"
#include <OpenGL/gl.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <cmath>

Coin::Coin() : x(0), y(0), radius(10), angle(0), rotationSpeed(180), value(10), active(true), lifetime(10.0f) {
}

void Coin::initialize(float startX, float startY, int pointValue) {
    x = startX;
    y = startY;
    value = pointValue;
    active = true;
    lifetime = 10.0f;
    angle = 0;
}

void Coin::update(float deltaTime) {
    // Rotation animation
    angle += rotationSpeed * deltaTime;
    if (angle > 360) angle -= 360;

    // Decrease lifetime
    lifetime -= deltaTime;
    if (lifetime <= 0) {
        active = false;
    }
}

void Coin::draw() {
    // Member 2 responsibility: Fill coin using boundary fill or flood fill
    if (active) {
        glPushMatrix();
        glTranslatef(x, y, 0);
        glRotatef(angle, 0, 0, 1);

        glColor3f(1.0f, 0.84f, 0.0f);
        glBegin(GL_POLYGON);
        for (int i = 0; i < 20; i++) {
            float ang = i * 3.14159f / 10;
            glVertex2f(cosf(ang) * radius, sinf(ang) * radius);
        }
        glEnd();

        glPopMatrix();
    }
}
