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
        Color col = strong ? Color(1.0f, 0.45f, 0.0f) : Color(0.15f, 0.95f, 1.0f);
        bresenhamLine((int)x, (int)(y-10), (int)x, (int)(y+10), col);
        if(strong){
            bresenhamLine((int)x-1, (int)(y-12), (int)x-1, (int)(y+12), Color(1.0f, 0.70f, 0.1f));
            bresenhamLine((int)x+1, (int)(y-12), (int)x+1, (int)(y+12), Color(1.0f, 0.70f, 0.1f));
            drawCircle(x, y+12, 5.5f, Color(1.0f, 0.95f, 0.5f));
        } else {
            drawCircle(x, y+10, 3.5f, Color(0.80f, 1.0f, 1.0f));
        }
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
    float animTime;

    Egg(float x, float y, float spd=4.0f)
        : x(x), y(y), speed(spd), active(true), animTime(randF(0.0f, 6.28f)) {}

    void update(){
        animTime += 0.10f;
        float ex1=x-7, ey1=y-12, ex2=x+7, ey2=y+12;
        if(!cohenSutherland(ex1,ey1,ex2,ey2)){
            active=false; return;
        }
        y -= speed;
        if(y < 0) active=false;
    }

    void draw() const {
        if(!active) return;
        // Aerodynamic wobble tilt as the egg drops
        float wobble = 12.0f * std::sin(animTime * 7.5f);

        glPushMatrix();
        glTranslatef(x, y, 0);
        glRotatef(wobble, 0, 0, 1);

        // True ovoid egg silhouette (tapered top, bulbous bottom)
        std::vector<Vec2> eggPts;
        int segs = 18;
        for(int i = 0; i < segs; i++){
            float a = i * 2.0f * PI / segs;
            float cosA = std::cos(a);
            float sinA = std::sin(a);
            float rx = 6.2f * (1.0f - 0.20f * sinA);
            float ry = 9.5f;
            eggPts.push_back({rx * cosA, ry * sinA});
        }
        scanlineFill(eggPts, Color(0.96f, 0.92f, 0.78f)); // authentic eggshell cream

        // Subtle bottom shell shadow
        std::vector<Vec2> shellShadow = {
            {-5.5f, -2.0f}, {-4.5f, -7.0f}, {0.0f, -9.5f},
            {4.5f, -7.0f}, {5.5f, -2.0f}, {0.0f, -5.0f}
        };
        scanlineFill(shellShadow, Color(0.82f, 0.76f, 0.60f));

        // Specular sheen dot on upper-left egg curve
        drawCircle(-2.2f, 3.5f, 2.2f, Color(1.0f, 1.0f, 1.0f, 0.75f));

        // Crack line
        bresenhamLine(-1, 2, 2, -2, Color(0.68f, 0.60f, 0.48f));
        bresenhamLine(2, -2, 0, -5, Color(0.68f, 0.60f, 0.48f));

        glPopMatrix();
    }

    AABB getAABB() const { return {x-7, y-10, 14, 20}; }
};
