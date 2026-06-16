// =============================================================================
// Bullet.h — Bullet and Egg (enemy projectile) declarations
// =============================================================================
// CG Concepts:
//   Bresenham Line Algorithm — bullet trails drawn with integer math
//   Cohen-Sutherland Clipping — bullets outside screen are removed
//   Translation — bullets move via position increment each frame
// =============================================================================

#pragma once
#include "Utils.h"

// ---------------------------------------------------------------------------
// Bullet — fired by the player upward
// ---------------------------------------------------------------------------
struct Bullet {
    float x, y;       // center position
    float speed;      // pixels/frame upward
    bool  active;
    bool  strong;     // power-up: stronger bullet (wider, more damage)
    int   damage;

    Bullet(float x, float y, bool strong=false)
        : x(x), y(y), speed(14.0f), active(true), strong(strong),
          damage(strong ? 2 : 1) {}

    // CG: Translation — bullet moves upward each frame
    void update(){
        // CG: Cohen-Sutherland Clipping — deactivate if outside window
        float bx1=x-2, by1=y, bx2=x+2, by2=y+20;
        if(!cohenSutherland(bx1,by1,bx2,by2)){
            active=false; return;
        }
        y += speed;
        if(y > WIN_H) active=false;
    }

    void draw() const {
        if(!active) return;
        // CG Concept 3: Bresenham Line — bullet body drawn as a raster line
        Color col = strong ? Color(1.0f,0.4f,0.0f) : Color(1.0f,1.0f,0.2f);
        bresenhamLine((int)x,(int)(y-10),(int)x,(int)(y+10), col);
        if(strong){
            bresenhamLine((int)x-1,(int)(y-10),(int)x-1,(int)(y+10), Color(1,0.6f,0));
            bresenhamLine((int)x+1,(int)(y-10),(int)x+1,(int)(y+10), Color(1,0.6f,0));
        }
        // Glowing tip (GL circle — fast path)
        drawCircle(x, y+10, strong?5.0f:3.0f, Color(1,1,0.6f));
    }

    AABB getAABB() const {
        float hw = strong ? 4.0f : 2.0f;
        return {x-hw, y-10, hw*2, 20};
    }
};

// ---------------------------------------------------------------------------
// Egg — fired by enemies downward
// ---------------------------------------------------------------------------
struct Egg {
    float x, y;
    float speed;
    bool  active;

    Egg(float x, float y, float spd=4.0f)
        : x(x), y(y), speed(spd), active(true) {}

    void update(){
        float ex1=x-6,ey1=y-10,ex2=x+6,ey2=y+10;
        if(!cohenSutherland(ex1,ey1,ex2,ey2)){
            active=false; return;
        }
        y -= speed;
        if(y < 0) active=false;
    }

    void draw() const {
        if(!active) return;
        // Egg body — oval using scanline fill on an ellipse approximation
        Color eggColor(0.95f,0.9f,0.7f);
        std::vector<Vec2> pts;
        int segs=16;
        for(int i=0;i<segs;i++){
            float a=2*PI*i/segs;
            pts.push_back({x+6*std::cos(a), y+9*std::sin(a)});
        }
        scanlineFill(pts, eggColor);
        // crack lines
        bresenhamLine((int)x-1,(int)y+3,(int)x+2,(int)y-2, Color(0.6f,0.5f,0.4f));
    }

    AABB getAABB() const { return {x-6, y-9, 12, 18}; }
};
