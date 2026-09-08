// =============================================================================
// PowerUp.h — Power-Up items (fire rate, shield, strong bullets)
// =============================================================================
// CG Concepts:
//   Rotation       — orbiting energy satellites & spinning containment ring
//   Scaling (CG 9) — pulsing energy capsule breathing animation
//   Scan-Line Fill — custom crystal icons & cybernetic shields
//   Midpoint Circle — plasma corona & energy nodes
// =============================================================================

#pragma once
#include "Utils.h"

enum class PowerUpType { FIRE_RATE, SHIELD, STRONG_BULLET };

struct PowerUp {
    float x, y;
    float speed;
    bool  active;
    PowerUpType type;
    float animTime;
    int   duration;   // frames the power-up lasts when collected

    PowerUp(float x, float y, PowerUpType t)
        : x(x), y(y), speed(2.0f), active(true), type(t),
          animTime(randF(0.0f, 6.28f)), duration(600) {}

    void update(){
        y -= speed;
        animTime += 0.08f;
        if(y < -30) active=false;
    }

    void draw() const {
        if(!active) return;

        float t = animTime;
        float bobY = 3.5f * std::sin(t * 2.8f);
        float cy   = y + bobY;

        // Pulsing capsule breath (CG Concept 9: Scaling)
        float s = 1.0f + 0.08f * std::sin(t * 3.5f);

        // Define theme colors per power-up type
        Color mainCol, glowCol, coreCol;
        switch(type){
            case PowerUpType::FIRE_RATE:
                mainCol = Color(1.00f, 0.88f, 0.10f);
                glowCol = Color(1.00f, 0.65f, 0.00f, 0.22f);
                coreCol = Color(1.00f, 1.00f, 0.60f);
                break;
            case PowerUpType::SHIELD:
                mainCol = Color(0.15f, 0.82f, 1.00f);
                glowCol = Color(0.05f, 0.50f, 1.00f, 0.22f);
                coreCol = Color(0.70f, 0.95f, 1.00f);
                break;
            case PowerUpType::STRONG_BULLET:
                mainCol = Color(1.00f, 0.32f, 0.15f);
                glowCol = Color(1.00f, 0.15f, 0.05f, 0.22f);
                coreCol = Color(1.00f, 0.90f, 0.30f);
                break;
        }

        // ── 0. Multi-Layered Plasma Aura (CG Concept 4: Midpoint Circle) ────
        float auraPulse = 0.16f + 0.10f * std::sin(t * 4.0f);
        glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        midpointCircle((int)x, (int)cy, 24, Color(glowCol.r, glowCol.g, glowCol.b, auraPulse), true);
        midpointCircle((int)x, (int)cy, 20, Color(glowCol.r, glowCol.g, glowCol.b, auraPulse * 1.5f), false);
        glDisable(GL_BLEND);

        // ── 1. Rotating Containment Ring (CG Concept 8: Rotation) ───────────
        glPushMatrix();
        glTranslatef(x, cy, 0);
        glRotatef(t * 60.0f, 0, 0, 1);
        glScalef(s, s, 1);

        // Hexagonal containment bracket
        int hexPts = 6;
        for(int i = 0; i < hexPts; i++){
            float a1 = i * (2.0f * PI / hexPts);
            float a2 = (i + 1) * (2.0f * PI / hexPts);
            float hx1 = 17.5f * std::cos(a1), hy1 = 17.5f * std::sin(a1);
            float hx2 = 17.5f * std::cos(a2), hy2 = 17.5f * std::sin(a2);
            ddaLine((int)hx1, (int)hy1, (int)hx2, (int)hy2, Color(mainCol.r, mainCol.g, mainCol.b, 0.65f));
        }
        glPopMatrix();

        // ── 2. Glass Capsule Orb (Stable upright) ────────────────────────────
        glPushMatrix();
        glTranslatef(x, cy, 0);
        glScalef(s, s, 1);

        // Capsule background disc
        drawCircle(0, 0, 15.0f, Color(0.08f, 0.10f, 0.16f, 0.92f));
        midpointCircle(0, 0, 15, mainCol, false);

        // ── 3. Distinctive Emblems (Oriented upright with smooth tilt) ──────
        float iconTilt = 4.0f * std::sin(t * 2.2f);
        glRotatef(iconTilt, 0, 0, 1);

        switch(type){
            case PowerUpType::FIRE_RATE: {
                // Electric Lightning Bolt Crystal (Scan-Line Fill)
                std::vector<Vec2> bolt = {
                    {-3.0f,  12.0f},
                    { 3.0f,   2.0f},
                    {-1.0f,   2.0f},
                    { 4.0f, -12.0f},
                    {-4.0f,  -2.0f},
                    { 0.0f,  -2.0f}
                };
                scanlineFill(bolt, mainCol);
                // Hot core line
                ddaLine(-1, 10, 2, 2, coreCol);
                ddaLine(0, 0, 3, -10, coreCol);
                break;
            }
            case PowerUpType::SHIELD: {
                // Cyber Aegis Shield (Scan-Line Fill)
                std::vector<Vec2> shieldPlate = {
                    { 0.0f,  10.0f},
                    { 8.0f,   6.0f},
                    { 8.0f,  -3.0f},
                    { 0.0f, -11.0f},
                    {-8.0f,  -3.0f},
                    {-8.0f,   6.0f}
                };
                scanlineFill(shieldPlate, mainCol);
                // Inner core chevron
                std::vector<Vec2> innerChev = {
                    { 0.0f,   6.0f},
                    { 4.0f,   3.0f},
                    { 0.0f,  -7.0f},
                    {-4.0f,   3.0f}
                };
                scanlineFill(innerChev, coreCol);
                break;
            }
            case PowerUpType::STRONG_BULLET: {
                // Hyper Heavy Warhead / Nova Star
                std::vector<Vec2> star;
                for(int i = 0; i < 8; i++){
                    float a = i * (2.0f * PI / 8.0f) + PI / 8.0f;
                    float rad = (i % 2 == 0) ? 11.5f : 5.0f;
                    star.push_back({rad * std::cos(a), rad * std::sin(a)});
                }
                scanlineFill(star, mainCol);
                drawCircle(0, 0, 4.0f, coreCol);
                break;
            }
        }

        // Specular glass shine on top rim
        drawCircle(-4.0f, 6.0f, 4.0f, Color(1.0f, 1.0f, 1.0f, 0.45f));

        glPopMatrix();

        // ── 4. Orbiting Energy Satellites (CG Concept 8: Rotation) ──────────
        for(int i = 0; i < 3; i++){
            float orbAngle = t * 3.2f + i * (2.0f * PI / 3.0f);
            float orbDist  = 20.0f + 2.0f * std::sin(t * 4.0f + i);
            float ox = x + orbDist * std::cos(orbAngle);
            float oy = cy + orbDist * std::sin(orbAngle);
            float orbAlpha = 0.50f + 0.50f * std::abs(std::sin(t * 3.5f + i));

            // Satellite node
            drawCircle(ox, oy, 2.2f, Color(coreCol.r, coreCol.g, coreCol.b, orbAlpha));
            // Trailing tail spark
            float tx = x + (orbDist - 1.5f) * std::cos(orbAngle - 0.25f);
            float ty = cy + (orbDist - 1.5f) * std::sin(orbAngle - 0.25f);
            drawCircle(tx, ty, 1.3f, Color(mainCol.r, mainCol.g, mainCol.b, orbAlpha * 0.5f));
        }
    }

    AABB getAABB() const { return {x-16, y-16, 32, 32}; }
};
