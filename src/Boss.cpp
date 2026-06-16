#include "Boss.h"
#include "FillEngine.h"
#include "TransformEngine.h"
#include <OpenGL/gl.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <cmath>

Boss::Boss() : x(400), y(450), width(60), height(60), vx(0), vy(0), angle(0), scale(1.0f), health(200), attackTimer(0), active(false) {
}

void Boss::initialize(float startX, float startY) {
    x = startX;
    y = startY;
    health = 200;
    active = true;
    scale = 1.0f;
}

void Boss::update(float deltaTime) {
    // Boss movement pattern
    x += vx * deltaTime;
    y += vy * deltaTime;

    // Sinusoidal movement
    static float time = 0;
    time += deltaTime;
    vx = cosf(time) * 50;

    // Update attack timer
    attackTimer += deltaTime;

    // Rotation animation
    angle += 45 * deltaTime;
    if (angle > 360) angle -= 360;

    // Scale animation (breathing effect)
    scale = 1.0f + 0.2f * sinf(time * 2);
}

void Boss::draw() {
    // Member 2 responsibility: Fill boss body using advanced fill algorithms
    // Member 3 responsibility: Apply rotation and scaling transformations
    
    glPushMatrix();
    glTranslatef(x, y, 0);
    glRotatef(angle, 0, 0, 1);
    glScalef(scale, scale, 1);

    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_POLYGON);
    for (int i = 0; i < 8; i++) {
        float angle = i * 3.14159f / 4;
        glVertex2f(cosf(angle) * width/2, sinf(angle) * height/2);
    }
    glEnd();

    glPopMatrix();
}
