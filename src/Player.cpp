#include "Player.h"
#include "Renderer.h"
#include <OpenGL/gl.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

Player::Player() : x(0), y(0), width(20), height(20), vx(0), vy(0), angle(0), health(100), canJump(true) {
}

void Player::initialize(float startX, float startY) {
    x = startX;
    y = startY;
    health = 100;
    canJump = true;
}

void Player::update(float deltaTime) {
    // Update position based on velocity
    x += vx * deltaTime;
    y += vy * deltaTime;

    // Apply gravity
    vy -= 9.8f * deltaTime * 50;

    // Boundary checking
    if (y < 0) {
        y = 0;
        vy = 0;
        canJump = true;
    }
}

void Player::draw() {
    // Member 1 responsibility: Draw player using Renderer primitives
    // This is where Renderer.drawCircle() and Renderer.drawLine() would be called
    glColor3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_TRIANGLES);
    glVertex2f(x, y + height);
    glVertex2f(x - width/2, y);
    glVertex2f(x + width/2, y);
    glEnd();
}
