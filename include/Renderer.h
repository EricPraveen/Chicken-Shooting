#ifndef RENDERER_H
#define RENDERER_H

// Member 1 – Rendering & Primitive Engine
// Computer Graphics Concepts:
// - Bresenham Line Algorithm
// - Midpoint Circle Algorithm
// - Bresenham Circle Algorithm
// - 8-Way Symmetry

// Forward declarations
struct Point {
    int x, y;
};

class Renderer {
public:
    Renderer();
    ~Renderer();

    // Line drawing using Bresenham algorithm
    void drawLine(int x0, int y0, int x1, int y1, float r = 1.0f, float g = 1.0f, float b = 1.0f);

    // Circle drawing using Midpoint Circle algorithm
    void drawCircle(int centerX, int centerY, int radius, float r = 1.0f, float g = 1.0f, float b = 1.0f);

    // Circle drawing using Bresenham algorithm
    void drawBresenhamCircle(int centerX, int centerY, int radius, float r = 1.0f, float g = 1.0f, float b = 1.0f);

    // 8-way symmetry helper for circle drawing
    void drawSymmetricPoints(int centerX, int centerY, int x, int y, float r = 1.0f, float g = 1.0f, float b = 1.0f);

    // Rectangle (border) drawing
    void drawRectangle(int x, int y, int width, int height, float r = 1.0f, float g = 1.0f, float b = 1.0f);

    // Set pixel color
    void setPixelColor(int x, int y, float r, float g, float b);

private:
    void drawPixel(int x, int y, float r, float g, float b);
};

#endif // RENDERER_H
