#ifndef CLIPENGINE_H
#define CLIPENGINE_H

// Member 4 – Viewing & Clipping System
// Computer Graphics Concepts:
// - Window-to-Viewport Mapping
// - Cohen-Sutherland Clipping
// - Liang-Barsky Clipping
// - Sutherland-Hodgman Polygon Clipping

struct Rect {
    float x, y, width, height;
};

struct Line {
    float x0, y0, x1, y1;
};

class ClipEngine {
public:
    ClipEngine();
    ~ClipEngine();

    // Set the clipping window
    void setClippingWindow(float x, float y, float width, float height);

    // Set the viewport for mapping
    void setViewport(float x, float y, float width, float height);

    // Window-to-Viewport mapping
    void mapWindowToViewport(float& x, float& y);

    // Cohen-Sutherland line clipping
    bool cohenSutherlandClip(Line& line);

    // Liang-Barsky line clipping
    bool liangBarskyClip(Line& line);

    // Sutherland-Hodgman polygon clipping
    void sutherlandHodgmanClip(float* xPoints, float* yPoints, int& pointCount);

    // Simple rectangle clipping
    bool clipRectangle(Rect& rect);

    // Check if point is inside clipping window
    bool isPointInside(float x, float y) const;

    // Check if line is completely inside window
    bool isLineInside(const Line& line) const;

private:
    float windowX, windowY, windowWidth, windowHeight;
    float viewportX, viewportY, viewportWidth, viewportHeight;

    // Cohen-Sutherland region codes
    int computeRegionCode(float x, float y) const;

    const int INSIDE = 0;   // 0000
    const int LEFT = 1;     // 0001
    const int RIGHT = 2;    // 0010
    const int BOTTOM = 4;   // 0100
    const int TOP = 8;      // 1000
};

#endif // CLIPENGINE_H
