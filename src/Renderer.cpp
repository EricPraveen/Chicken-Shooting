#include "Renderer.h"
#include <OpenGL/gl.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <cmath>

// Member 1 – Rendering & Primitive Engine
// Bresenham Line Algorithm, Midpoint Circle Algorithm, Bresenham Circle, 8-Way Symmetry

Renderer::Renderer() {
}

Renderer::~Renderer() {
}

void Renderer::drawLine(int x0, int y0, int x1, int y1, float r, float g, float b) {
    // Bresenham Line Algorithm implementation
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        setPixelColor(x0, y0, r, g, b);
        
        if (x0 == x1 && y0 == y1) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void Renderer::drawCircle(int centerX, int centerY, int radius, float r, float g, float b) {
    // Midpoint Circle Algorithm
    int x = 0;
    int y = radius;
    int d = 3 - 2 * radius;

    while (x <= y) {
        drawSymmetricPoints(centerX, centerY, x, y, r, g, b);
        
        if (d < 0) {
            d = d + 4 * x + 6;
        } else {
            d = d + 4 * (x - y) + 10;
            y--;
        }
        x++;
    }
}

void Renderer::drawBresenhamCircle(int centerX, int centerY, int radius, float r, float g, float b) {
    // Bresenham Circle Algorithm
    int x = 0;
    int y = radius;
    int d = 3 - 2 * radius;

    while (x <= y) {
        drawSymmetricPoints(centerX, centerY, x, y, r, g, b);
        
        if (d < 0) {
            d = d + 4 * x + 6;
        } else {
            d = d + 4 * (x - y) + 10;
            y--;
        }
        x++;
    }
}

void Renderer::drawSymmetricPoints(int centerX, int centerY, int x, int y, float r, float g, float b) {
    // 8-Way Symmetry for circle drawing
    setPixelColor(centerX + x, centerY + y, r, g, b);
    setPixelColor(centerX - x, centerY + y, r, g, b);
    setPixelColor(centerX + x, centerY - y, r, g, b);
    setPixelColor(centerX - x, centerY - y, r, g, b);
    setPixelColor(centerX + y, centerY + x, r, g, b);
    setPixelColor(centerX - y, centerY + x, r, g, b);
    setPixelColor(centerX + y, centerY - x, r, g, b);
    setPixelColor(centerX - y, centerY - x, r, g, b);
}

void Renderer::drawRectangle(int x, int y, int width, int height, float r, float g, float b) {
    // Draw rectangle border using lines
    drawLine(x, y, x + width, y, r, g, b);                      // Top
    drawLine(x + width, y, x + width, y + height, r, g, b);     // Right
    drawLine(x + width, y + height, x, y + height, r, g, b);    // Bottom
    drawLine(x, y + height, x, y, r, g, b);                     // Left
}

void Renderer::setPixelColor(int x, int y, float r, float g, float b) {
    drawPixel(x, y, r, g, b);
}

void Renderer::drawPixel(int x, int y, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_POINTS);
    glVertex2i(x, y);
    glEnd();
}
