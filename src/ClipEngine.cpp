#include "ClipEngine.h"

// Member 4 – Viewing & Clipping System
// Window-to-Viewport Mapping, Cohen-Sutherland, Liang-Barsky, Sutherland-Hodgman

ClipEngine::ClipEngine() 
    : windowX(0), windowY(0), windowWidth(800), windowHeight(600),
      viewportX(0), viewportY(0), viewportWidth(800), viewportHeight(600) {
}

ClipEngine::~ClipEngine() {
}

void ClipEngine::setClippingWindow(float x, float y, float width, float height) {
    windowX = x;
    windowY = y;
    windowWidth = width;
    windowHeight = height;
}

void ClipEngine::setViewport(float x, float y, float width, float height) {
    viewportX = x;
    viewportY = y;
    viewportWidth = width;
    viewportHeight = height;
}

void ClipEngine::mapWindowToViewport(float& x, float& y) {
    // Window-to-Viewport mapping
    // Linear transformation from window coordinates to viewport coordinates
    x = viewportX + (x - windowX) * (viewportWidth / windowWidth);
    y = viewportY + (y - windowY) * (viewportHeight / windowHeight);
}

bool ClipEngine::cohenSutherlandClip(Line& line) {
    // Cohen-Sutherland line clipping algorithm
    int code0 = computeRegionCode(line.x0, line.y0);
    int code1 = computeRegionCode(line.x1, line.y1);

    bool accept = false;
    while (true) {
        if ((code0 | code1) == 0) {
            // Both endpoints inside
            accept = true;
            break;
        } else if ((code0 & code1) != 0) {
            // Both endpoints outside (same side)
            break;
        } else {
            // One inside, one outside - clip
            float x, y;
            int codeOut = (code0 != 0) ? code0 : code1;

            if (codeOut & TOP) {
                x = line.x0 + (line.x1 - line.x0) * (windowY + windowHeight - line.y0) / (line.y1 - line.y0);
                y = windowY + windowHeight;
            } else if (codeOut & BOTTOM) {
                x = line.x0 + (line.x1 - line.x0) * (windowY - line.y0) / (line.y1 - line.y0);
                y = windowY;
            } else if (codeOut & RIGHT) {
                y = line.y0 + (line.y1 - line.y0) * (windowX + windowWidth - line.x0) / (line.x1 - line.x0);
                x = windowX + windowWidth;
            } else if (codeOut & LEFT) {
                y = line.y0 + (line.y1 - line.y0) * (windowX - line.x0) / (line.x1 - line.x0);
                x = windowX;
            }

            if (codeOut == code0) {
                line.x0 = x;
                line.y0 = y;
                code0 = computeRegionCode(line.x0, line.y0);
            } else {
                line.x1 = x;
                line.y1 = y;
                code1 = computeRegionCode(line.x1, line.y1);
            }
        }
    }
    return accept;
}

bool ClipEngine::liangBarskyClip(Line& line) {
    // Liang-Barsky line clipping algorithm
    float t0 = 0.0f, t1 = 1.0f;
    float dx = line.x1 - line.x0;
    float dy = line.y1 - line.y0;

    float p, q;

    // Check x bounds
    p = -dx;
    q = line.x0 - windowX;
    if (p == 0 && q < 0) return false;
    if (p != 0) {
        float t = q / p;
        if (p < 0) {
            if (t > t1) return false;
            if (t > t0) t0 = t;
        } else {
            if (t < t0) return false;
            if (t < t1) t1 = t;
        }
    }

    p = dx;
    q = windowX + windowWidth - line.x0;
    if (p == 0 && q < 0) return false;
    if (p != 0) {
        float t = q / p;
        if (p < 0) {
            if (t > t1) return false;
            if (t > t0) t0 = t;
        } else {
            if (t < t0) return false;
            if (t < t1) t1 = t;
        }
    }

    // Check y bounds
    p = -dy;
    q = line.y0 - windowY;
    if (p == 0 && q < 0) return false;
    if (p != 0) {
        float t = q / p;
        if (p < 0) {
            if (t > t1) return false;
            if (t > t0) t0 = t;
        } else {
            if (t < t0) return false;
            if (t < t1) t1 = t;
        }
    }

    p = dy;
    q = windowY + windowHeight - line.y0;
    if (p == 0 && q < 0) return false;
    if (p != 0) {
        float t = q / p;
        if (p < 0) {
            if (t > t1) return false;
            if (t > t0) t0 = t;
        } else {
            if (t < t0) return false;
            if (t < t1) t1 = t;
        }
    }

    // Compute clipped endpoints
    line.x0 = line.x0 + dx * t0;
    line.y0 = line.y0 + dy * t0;
    line.x1 = line.x0 + dx * (t1 - t0);
    line.y1 = line.y0 + dy * (t1 - t0);

    return true;
}

void ClipEngine::sutherlandHodgmanClip(float* xPoints, float* yPoints, int& pointCount) {
    // Sutherland-Hodgman polygon clipping algorithm
    // TODO: Implement full polygon clipping
}

bool ClipEngine::clipRectangle(Rect& rect) {
    // Simple rectangle clipping
    if (rect.x + rect.width < windowX || rect.x > windowX + windowWidth ||
        rect.y + rect.height < windowY || rect.y > windowY + windowHeight) {
        return false; // Completely outside
    }

    // Clip to window bounds
    if (rect.x < windowX) {
        rect.width -= (windowX - rect.x);
        rect.x = windowX;
    }
    if (rect.x + rect.width > windowX + windowWidth) {
        rect.width = windowX + windowWidth - rect.x;
    }
    if (rect.y < windowY) {
        rect.height -= (windowY - rect.y);
        rect.y = windowY;
    }
    if (rect.y + rect.height > windowY + windowHeight) {
        rect.height = windowY + windowHeight - rect.y;
    }

    return true;
}

bool ClipEngine::isPointInside(float x, float y) const {
    return x >= windowX && x <= windowX + windowWidth &&
           y >= windowY && y <= windowY + windowHeight;
}

bool ClipEngine::isLineInside(const Line& line) const {
    return isPointInside(line.x0, line.y0) && isPointInside(line.x1, line.y1);
}

int ClipEngine::computeRegionCode(float x, float y) const {
    int code = INSIDE;
    if (x < windowX) code |= LEFT;
    if (x > windowX + windowWidth) code |= RIGHT;
    if (y < windowY) code |= BOTTOM;
    if (y > windowY + windowHeight) code |= TOP;
    return code;
}
