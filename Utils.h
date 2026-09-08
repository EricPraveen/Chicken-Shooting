// =============================================================================
// Utils.h — Shared Utilities, Math, and Computer Graphics Algorithms
// =============================================================================
// CG CONCEPTS COVERED HERE:
//   1. Raster Graphics Pipeline   — pixel-level drawing primitives
//   2. DDA Line Algorithm         — floating-point incremental line drawing
//   3. Bresenham Line Algorithm   — integer-based fast line drawing
//   4. Midpoint Circle Algorithm  — drawing circles pixel-by-pixel
//   5. Scan-Line Fill Algorithm   — filling polygons row by row
//   6. Homogeneous Coordinates    — 3x3 matrix math for 2D transforms
//   7. Translation/Rotation/Scale — affine transforms via matrices
//   8. Window-to-Viewport Mapping — coordinate system conversion
//   9. Cohen-Sutherland Clipping  — clip lines to screen boundary
// =============================================================================

#pragma once
#include <cmath>
#include <vector>
#include <algorithm>

#ifdef __EMSCRIPTEN__
#  include <GL/glut.h>
#elif defined(__APPLE__)
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------
constexpr int   WIN_W       = 900;
constexpr int   WIN_H       = 700;
constexpr float PI          = 3.14159265358979f;
constexpr int   TARGET_FPS  = 60;
constexpr int   FRAME_MS    = 1000 / TARGET_FPS;

// ---------------------------------------------------------------------------
// 2D Vector / Point
// ---------------------------------------------------------------------------
struct Vec2 {
    float x, y;
    Vec2(float x = 0, float y = 0) : x(x), y(y) {}
    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    Vec2 operator*(float s)       const { return {x * s,   y * s};   }
    float length() const { return std::sqrt(x*x + y*y); }
};

// ---------------------------------------------------------------------------
// Color (RGB float 0..1)
// ---------------------------------------------------------------------------
struct Color {
    float r, g, b, a;
    Color(float r=1,float g=1,float b=1,float a=1): r(r),g(g),b(b),a(a){}
    void apply() const { glColor4f(r, g, b, a); }
};

// ---------------------------------------------------------------------------
// AABB — Axis-Aligned Bounding Box (CG Concept 15: Collision Detection)
// AABB overlap test: two rectangles intersect when their projections on both
// axes overlap simultaneously.
// ---------------------------------------------------------------------------
struct AABB {
    float x, y, w, h; // x,y = top-left corner
    bool intersects(const AABB& o) const {
        return x < o.x+o.w && x+w > o.x &&
               y < o.y+o.h && y+h > o.y;
    }
};

// =============================================================================
// HOMOGENEOUS COORDINATE MATRIX (3x3 for 2D)
// CG Concept 10: Homogeneous Coordinates
// A 2D point P=(x,y) is represented as [x, y, 1]^T.
// Transforms (T, R, S) become 3×3 matrices that can be composed via
// matrix multiplication, enabling combined transformations in one step.
// =============================================================================
struct Mat3 {
    float m[3][3];
    Mat3() { // identity
        for(int i=0;i<3;i++) for(int j=0;j<3;j++) m[i][j]=(i==j)?1.0f:0.0f;
    }
    Mat3 operator*(const Mat3& o) const {
        Mat3 res;
        for(int i=0;i<3;i++)
            for(int j=0;j<3;j++){
                res.m[i][j]=0;
                for(int k=0;k<3;k++) res.m[i][j]+=m[i][k]*o.m[k][j];
            }
        return res;
    }
    Vec2 transform(Vec2 p) const {
        float nx = m[0][0]*p.x + m[0][1]*p.y + m[0][2];
        float ny = m[1][0]*p.x + m[1][1]*p.y + m[1][2];
        return {nx, ny};
    }
};

// CG Concept 7a: Translation Matrix
inline Mat3 translationMat(float tx, float ty){
    Mat3 M; M.m[0][2]=tx; M.m[1][2]=ty; return M;
}
// CG Concept 7b: Rotation Matrix (angle in degrees)
inline Mat3 rotationMat(float deg){
    float r=deg*PI/180.0f;
    Mat3 M;
    M.m[0][0]= std::cos(r); M.m[0][1]=-std::sin(r);
    M.m[1][0]= std::sin(r); M.m[1][1]= std::cos(r);
    return M;
}
// CG Concept 7c: Scale Matrix
inline Mat3 scaleMat(float sx, float sy){
    Mat3 M; M.m[0][0]=sx; M.m[1][1]=sy; return M;
}

// =============================================================================
// WINDOW-TO-VIEWPORT MAPPING (CG Concept 11)
// Maps a point from world window [wx1,wx2] x [wy1,wy2]
// to screen viewport [vx1,vx2] x [vy1,vy2].
// =============================================================================
inline Vec2 windowToViewport(Vec2 p,
    float wx1,float wx2,float wy1,float wy2,
    float vx1,float vx2,float vy1,float vy2)
{
    float x = vx1 + (p.x - wx1)*(vx2-vx1)/(wx2-wx1);
    float y = vy1 + (p.y - wy1)*(vy2-vy1)/(wy2-wy1);
    return {x, y};
}

// =============================================================================
// RASTER GRAPHICS PIPELINE — set a single pixel
// CG Concept 1: The raster pipeline ultimately places color values into a
// frame-buffer at discrete pixel positions.  glVertex2i is the equivalent
// of writing one pixel into the raster.
// =============================================================================
inline void putPixel(int x, int y, const Color& c) {
    c.apply();
    glBegin(GL_POINTS);
        glVertex2i(x, y);
    glEnd();
}

// =============================================================================
// DDA LINE ALGORITHM (CG Concept 2)
// Uses floating-point increments: step along the axis with larger span,
// incrementing the other axis by slope each step.
// Used for: drawing player ship outline, UI decorations.
// =============================================================================
inline void ddaLine(int x1,int y1,int x2,int y2, const Color& c){
    int dx = x2-x1, dy = y2-y1;
    int steps = std::max(std::abs(dx), std::abs(dy));
    if(steps == 0){ putPixel(x1,y1,c); return; }
    float xInc = (float)dx/steps;
    float yInc = (float)dy/steps;
    float x = x1, y = y1;
    c.apply();
    glBegin(GL_POINTS);
    for(int i=0;i<=steps;i++){
        glVertex2i((int)std::round(x),(int)std::round(y));
        x+=xInc; y+=yInc;
    }
    glEnd();
}

// =============================================================================
// BRESENHAM LINE ALGORITHM (CG Concept 3)
// Uses only integer arithmetic; very fast.
// Used for: bullet trails, laser beams, grid lines.
// =============================================================================
inline void bresenhamLine(int x1,int y1,int x2,int y2, const Color& c){
    int dx=std::abs(x2-x1), dy=std::abs(y2-y1);
    int sx=(x1<x2)?1:-1, sy=(y1<y2)?1:-1;
    int err=dx-dy;
    c.apply();
    glBegin(GL_POINTS);
    while(true){
        glVertex2i(x1,y1);
        if(x1==x2 && y1==y2) break;
        int e2=2*err;
        if(e2>-dy){ err-=dy; x1+=sx; }
        if(e2< dx){ err+=dx; y1+=sy; }
    }
    glEnd();
}

// =============================================================================
// MIDPOINT CIRCLE ALGORITHM (CG Concept 4)
// Exploits 8-fold symmetry; uses integer decision parameter p.
// Used for: shield bubble, coin outlines, boss aura.
// =============================================================================
inline void midpointCircle(int cx,int cy,int r, const Color& c, bool fill=false){
    c.apply();
    if(fill){
        glBegin(GL_LINES);
        for(int y=-r;y<=r;y++){
            int xSpan=(int)std::sqrt((float)(r*r - y*y));
            glVertex2i(cx - xSpan, cy + y);
            glVertex2i(cx + xSpan + 1, cy + y);
        }
        glEnd();
        return;
    }
    int x=0, y=r;
    int p=1-r;
    glBegin(GL_POINTS);
    auto plot8=[&](int px,int py){
        glVertex2i(cx+px,cy+py); glVertex2i(cx-px,cy+py);
        glVertex2i(cx+px,cy-py); glVertex2i(cx-px,cy-py);
        glVertex2i(cx+py,cy+px); glVertex2i(cx-py,cy+px);
        glVertex2i(cx+py,cy-px); glVertex2i(cx-py,cy-px);
    };
    plot8(x,y);
    while(x<y){
        x++;
        if(p<0) p+=2*x+1;
        else { y--; p+=2*(x-y)+1; }
        plot8(x,y);
    }
    glEnd();
}

// =============================================================================
// SCAN-LINE FILL ALGORITHM (CG Concept 5)
// Given a convex polygon as vertex list, fill it row by row.
// Used for: enemy chicken body, food items, power-up shapes.
// =============================================================================
inline void scanlineFill(const std::vector<Vec2>& verts, const Color& c){
    if(verts.size()<3) return;
    float yMin= verts[0].y, yMax=verts[0].y;
    for(auto& v:verts){ yMin=std::min(yMin,v.y); yMax=std::max(yMax,v.y); }
    int n=(int)verts.size();
    c.apply();
    glBegin(GL_LINES);
    for(int y=(int)yMin; y<=(int)yMax; y++){
        std::vector<float> xs;
        for(int i=0;i<n;i++){
            Vec2 a=verts[i], b=verts[(i+1)%n];
            if((a.y<=y && b.y>y)||(b.y<=y && a.y>y)){
                float t=(y-a.y)/(b.y-a.y);
                xs.push_back(a.x + t*(b.x-a.x));
            }
        }
        std::sort(xs.begin(),xs.end());
        for(int i=0;i+1<(int)xs.size();i+=2){
            glVertex2i((int)xs[i], y);
            glVertex2i((int)xs[i+1] + 1, y);
        }
    }
    glEnd();
}

// =============================================================================
// COHEN-SUTHERLAND LINE CLIPPING (CG Concept 12)
// Assigns a 4-bit outcode to each endpoint based on which side(s) of the
// clip window it falls outside.  Lines are clipped iteratively.
// Used for: clipping bullets/eggs that leave the screen.
// =============================================================================
namespace CS {
    const int INSIDE=0,LEFT=1,RIGHT=2,BOTTOM=4,TOP=8;
    inline int code(float x,float y,float xmin,float xmax,float ymin,float ymax){
        int c=INSIDE;
        if(x<xmin) c|=LEFT; else if(x>xmax) c|=RIGHT;
        if(y<ymin) c|=BOTTOM; else if(y>ymax) c|=TOP;
        return c;
    }
}
// Returns false if line is entirely outside, true if (partially) inside.
// x1,y1,x2,y2 are clipped in-place.
inline bool cohenSutherland(float& x1,float& y1,float& x2,float& y2,
    float xmin=0,float xmax=WIN_W,float ymin=0,float ymax=WIN_H)
{
    int c1=CS::code(x1,y1,xmin,xmax,ymin,ymax);
    int c2=CS::code(x2,y2,xmin,xmax,ymin,ymax);
    while(true){
        if(!(c1|c2))  return true;  // trivially inside
        if(c1&c2)     return false; // trivially outside
        int co=c1?c1:c2;
        float x,y;
        if(co&CS::TOP)         { x=x1+(x2-x1)*(ymax-y1)/(y2-y1); y=ymax; }
        else if(co&CS::BOTTOM) { x=x1+(x2-x1)*(ymin-y1)/(y2-y1); y=ymin; }
        else if(co&CS::RIGHT)  { y=y1+(y2-y1)*(xmax-x1)/(x2-x1); x=xmax; }
        else                   { y=y1+(y2-y1)*(xmin-x1)/(x2-x1); x=xmin; }
        if(co==c1){ x1=x; y1=y; c1=CS::code(x1,y1,xmin,xmax,ymin,ymax); }
        else      { x2=x; y2=y; c2=CS::code(x2,y2,xmin,xmax,ymin,ymax); }
    }
}

// ---------------------------------------------------------------------------
// Helper: draw a filled rectangle via GL quads (fast path for many entities)
// ---------------------------------------------------------------------------
inline void drawRect(float x,float y,float w,float h, const Color& c){
    c.apply();
    glBegin(GL_QUADS);
        glVertex2f(x,     y    );
        glVertex2f(x+w,   y    );
        glVertex2f(x+w,   y+h  );
        glVertex2f(x,     y+h  );
    glEnd();
}

// Helper: draw outlined rectangle
inline void drawRectOutline(float x,float y,float w,float h, const Color& c, float lw=1.5f){
    glLineWidth(lw); c.apply();
    glBegin(GL_LINE_LOOP);
        glVertex2f(x,   y  ); glVertex2f(x+w, y  );
        glVertex2f(x+w, y+h); glVertex2f(x,   y+h);
    glEnd();
    glLineWidth(1.0f);
}

// Fast unit-circle lookup tables for high-performance WebGL rendering
namespace CircleLUT {
    inline const std::vector<std::pair<float,float>>& getPoints(int segs) {
        static std::vector<std::pair<float,float>> lut8, lut12, lut16, lut22;
        static bool init = false;
        if (!init) {
            auto makeLut = [](int s) {
                std::vector<std::pair<float,float>> v;
                v.reserve(s + 1);
                for (int i = 0; i <= s; ++i) {
                    float a = 2.0f * PI * i / s;
                    v.push_back({std::cos(a), std::sin(a)});
                }
                return v;
            };
            lut8 = makeLut(8);
            lut12 = makeLut(12);
            lut16 = makeLut(16);
            lut22 = makeLut(22);
            init = true;
        }
        if (segs <= 8) return lut8;
        if (segs <= 12) return lut12;
        if (segs <= 16) return lut16;
        return lut22;
    }
}

// Helper: draw filled circle quickly via GL triangle fan using precomputed LUT
inline void drawCircle(float cx,float cy,float r, const Color& c, int segs=0){
    if (r <= 0.0f) return;
    if (segs <= 0) {
        segs = (r <= 5.0f) ? 8 : (r <= 14.0f) ? 12 : (r <= 28.0f) ? 16 : 22;
    }
    const auto& pts = CircleLUT::getPoints(segs);
    c.apply();
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx,cy);
        for (const auto& pt : pts) {
            glVertex2f(cx + r * pt.first, cy + r * pt.second);
        }
    glEnd();
}

// Helper: draw circle outline via midpoint algorithm (CG concept demo)
inline void drawCircleOutline(float cx,float cy,float r, const Color& c){
    midpointCircle((int)cx,(int)cy,(int)r,c,false);
}

// Helper: random float in [a,b]
inline float randF(float a, float b){
    return a + (b-a)*((float)rand()/(float)RAND_MAX);
}
