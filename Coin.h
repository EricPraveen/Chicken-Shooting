// =============================================================================
// Coin.h — Collectible Coins (IMPROVED: 3D-spin + orbiting sparkles)
// =============================================================================
// CG Concepts:
//   Midpoint Circle Algorithm — coin ring drawn pixel-by-pixel
//   Scaling (CG 9)           — X-axis squash simulates 3D coin rotation
//   Translation              — coin falls downward each frame
//   Rotation (CG 8)          — sparkle dots orbit the coin
// =============================================================================

#pragma once
#include "Utils.h"

struct Coin {
    float x, y;
    float speed;
    bool  active;
    int   value;
    float animTime;  // drives all animation

    Coin(float x, float y, int val=10)
        : x(x), y(y), speed(2.0f), active(true), value(val), animTime(0) {}

    void update(){
        // CG Concept 7a: Translation — coin falls downward
        y -= speed;
        animTime += 0.12f;
        if(y < -20) active=false;
    }

    void draw() const {
        if(!active) return;

        float t = animTime;

        // CG Concept 9: Scaling — squash X axis with |cos(t)| to fake 3D spin.
        // When cos(t)≈1 the coin faces us fully; when cos(t)≈0 we see the edge.
        float spinX   = std::abs(std::cos(t));       // 0..1
        float spinDir = (std::cos(t) >= 0) ? 1.f : -1.f; // which face?

        // Slight bob up/down (translation micro-animation)
        float bobY = 3.0f * std::sin(t * 1.4f);

        // Radius with gentle pulse
        float r = 10.0f + 1.5f*std::sin(t * 2.2f);

        glPushMatrix();
        glTranslatef(x, y + bobY, 0);

        // CG Concept 9: apply X-scale for spin illusion
        glScalef(spinX + 0.04f, 1.0f, 1.0f);

        // Choose face colour: front=gold, back=dark gold
        Color faceCol = (spinDir > 0)
            ? Color(1.0f,  0.80f, 0.0f)   // front face — bright gold
            : Color(0.55f, 0.38f, 0.0f);  // back face  — dark gold

        // Fill body
        drawCircle(0, 0, r, faceCol);

        // CG Concept 4: Midpoint Circle — crisp outer ring
        midpointCircle(0, 0, (int)r, Color(1.0f, 0.92f, 0.15f), false);

        // Shine highlight (only on front face)
        if(spinDir > 0){
            drawCircle(-r*0.28f, r*0.30f, r*0.28f, Color(1.0f,1.0f,0.75f, 0.55f*spinX));
        }

        // $ symbol — two DDA lines (cross) + vertical bar
        if(spinX > 0.25f){
            float alpha = spinX;
            Color sym(0.45f, 0.28f, 0.0f, alpha);
            ddaLine(0, (int)(-r*0.38f), 0, (int)(r*0.38f), sym);        // vertical
            ddaLine((int)(-r*0.30f), (int)(r*0.20f),                     // top horiz
                    (int)( r*0.30f), (int)(r*0.20f), sym);
            ddaLine((int)(-r*0.30f), (int)(-r*0.20f),                    // bot horiz
                    (int)( r*0.30f), (int)(-r*0.20f), sym);
        }

        glPopMatrix();

        // CG Concept 8: Rotation — 3 sparkle dots orbit the coin
        for(int i=0;i<3;i++){
            float a  = t * 2.2f + i*(2.0f*PI/3.0f);
            float sx = x + (r + 6.0f)*std::cos(a);
            float sy = y + bobY + (r + 6.0f)*std::sin(a);
            float sparkAlpha = 0.4f + 0.6f*std::abs(std::sin(t*3.0f + i*PI));
            float sparkR     = 1.5f + 1.0f*std::abs(std::sin(t*4.0f + i));
            drawCircle(sx, sy, sparkR, Color(1.0f, 1.0f, 0.55f, sparkAlpha));
        }
    }

    AABB getAABB() const { return {x-12, y-12, 24, 24}; }
};
