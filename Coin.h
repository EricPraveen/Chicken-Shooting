// =============================================================================
// Coin.h — Collectible Coins (Enhanced 3D-spin, rim thickness, and starlight)
// =============================================================================
// CG Concepts:
//   Midpoint Circle Algorithm — coin relief rings & sparkle nodes
//   Scaling (CG 9)           — X-axis cosine squash simulates 3D coin rotation
//   Translation              — coin falls downward with buoyant levitation bob
//   Rotation (CG 8)          — 4-point starlight sparkles orbiting the coin
// =============================================================================

#pragma once
#include "Utils.h"

struct Coin {
    float x, y;
    float speed;
    bool  active;
    int   value;
    float animTime;

    Coin(float x, float y, int val=10)
        : x(x), y(y), speed(2.0f), active(true), value(val),
          animTime(randF(0.0f, 6.28f)) {}

    void update(){
        y -= speed;
        animTime += 0.09f;
        if(y < -25) active=false;
    }

    void draw() const {
        if(!active) return;

        float t = animTime;

        // 3D Spin projection factor via cosine
        float spinVal = std::cos(t * 3.8f);
        float spinX   = std::abs(spinVal);
        bool  isFront = (spinVal >= 0.0f);

        // Buoyant floating bobbing (translation micro-animation)
        float bobY = 3.0f * std::sin(t * 2.2f);
        float cy   = y + bobY;

        // Coin radius with gentle pulse
        float r = 11.0f + 0.8f * std::sin(t * 3.0f);

        // ── 0. Golden Corona Aura (CG Concept 4: Midpoint Circle) ───────────
        float auraAlpha = 0.14f + 0.08f * std::sin(t * 4.0f);
        glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        midpointCircle((int)x, (int)cy, (int)(r + 6), Color(1.0f, 0.75f, 0.10f, auraAlpha), true);
        glDisable(GL_BLEND);

        // ── 1. 3D Cylindrical Edge Rim Thickness ────────────────────────────
        // When coin is tilted at an angle, the coin's thickness rim is revealed
        if(spinX < 0.90f){
            float rimThickness = 3.5f * (1.0f - spinX);
            float rimDir = isFront ? 1.0f : -1.0f;
            float rx = x + rimDir * (r * spinX * 0.5f);

            // Rim slab behind face
            glPushMatrix();
            glTranslatef(rx, cy, 0);
            glScalef(spinX + 0.18f, 1.0f, 1.0f);
            drawCircle(0, 0, r, Color(0.68f, 0.44f, 0.04f)); // darker burnished edge
            midpointCircle(0, 0, (int)r, Color(0.85f, 0.60f, 0.10f), false);
            glPopMatrix();
        }

        // ── 2. Coin Front / Back Face ───────────────────────────────────────
        glPushMatrix();
        glTranslatef(x, cy, 0);
        glScalef(std::max(spinX, 0.06f), 1.0f, 1.0f);

        Color faceOuter = isFront ? Color(1.00f, 0.84f, 0.12f) : Color(0.82f, 0.58f, 0.08f);
        Color faceInner = isFront ? Color(0.92f, 0.70f, 0.06f) : Color(0.70f, 0.48f, 0.04f);

        // Outer coin plate
        drawCircle(0, 0, r, faceOuter);
        // Beveled inner rim ring
        midpointCircle(0, 0, (int)r, Color(1.0f, 0.96f, 0.40f), false);
        // Inner recessed coin face
        drawCircle(0, 0, r * 0.75f, faceInner);
        midpointCircle(0, 0, (int)(r * 0.75f), Color(0.75f, 0.50f, 0.05f), false);

        // Embossed Star / Currency Insignia
        if(spinX > 0.28f){
            Color symCol = isFront ? Color(1.0f, 0.95f, 0.55f) : Color(0.88f, 0.66f, 0.12f);
            // 4-point diamond star emblem
            std::vector<Vec2> starEmblem = {
                {0, r * 0.50f}, {r * 0.25f, 0}, {0, -r * 0.50f}, {-r * 0.25f, 0}
            };
            scanlineFill(starEmblem, symCol);
            midpointCircle(0, 0, 2, Color(1, 1, 1), true);
        }

        // Specular Sweep Shine (glint when facing viewer)
        if(isFront && spinX > 0.65f){
            float glintProgress = (spinX - 0.65f) / 0.35f;
            float gx = -r * 0.35f;
            drawCircle(gx, r * 0.30f, r * 0.30f, Color(1.0f, 1.0f, 0.95f, 0.65f * glintProgress));
        }

        glPopMatrix();

        // ── 3. Orbiting 4-Point Starlight Sparkles (CG Concept 8: Rotation) ─
        for(int i = 0; i < 3; i++){
            float a = t * 2.6f + i * (2.0f * PI / 3.0f);
            float dist = r + 7.0f + 2.0f * std::sin(t * 3.5f + i);
            float sx = x + dist * std::cos(a);
            float sy = cy + dist * std::sin(a) * 0.75f;
            float sparkPulse = 0.45f + 0.55f * std::abs(std::sin(t * 4.0f + i * 1.6f));
            float spR = 2.0f * sparkPulse;

            // 4-point sparkle cross
            ddaLine((int)(sx - spR - 1.5f), (int)sy, (int)(sx + spR + 1.5f), (int)sy, Color(1.0f, 0.95f, 0.50f, sparkPulse));
            ddaLine((int)sx, (int)(sy - spR - 1.5f), (int)sx, (int)(sy + spR + 1.5f), Color(1.0f, 0.95f, 0.50f, sparkPulse));
            drawCircle(sx, sy, spR * 0.6f, Color(1.0f, 1.0f, 1.0f, sparkPulse));
        }
    }

    AABB getAABB() const { return {x-12, y-12, 24, 24}; }
};
