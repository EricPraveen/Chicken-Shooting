// =============================================================================
// Boss.h — Boss enemy: orbit + WARNING→FIRE laser system + egg bursts
// =============================================================================
// CG Concepts:
//   Rotation (CG 8)    — boss orbits, beams radiate at fixed angles
//   Scaling (CG 9)     — spawn-in + throb animation
//   Bresenham Line     — solid laser beam core
//   DDA Line           — beam glow + dotted warning dashes
//   Midpoint Circle    — aura + beam origin flash
//   Scan-Line Fill     — boss body polygon
// =============================================================================

#pragma once
#include "Utils.h"
#include "Bullet.h"
#include <vector>
#include <cmath>

// ---------------------------------------------------------------------------
// Three-state laser state machine
// ---------------------------------------------------------------------------
enum class LaserPhase { IDLE, WARNING, FIRING };

struct Boss {
    float x, y;
    float orbitAngle, orbitRadius, orbitSpeed;

    int   hp, maxHp;
    bool  active;
    int   phase;      // 1=normal, 2=enraged (<50% HP)
    int   gameLevel;  // 1-3  (controls beam count)

    float animTime;
    float scale;
    bool  spawning;

    // ── Laser state machine ───────────────────────────────────────────────────
    // WARNING phase:  dotted lines appear, no damage, player can dodge
    // FIRING  phase:  solid beams fire along locked angles, deal damage
    LaserPhase laserPhase;
    int   laserTimer;           // frame countdown for the current phase
    int   warningDuration;      // frames the dotted warning is shown (≈1.5 s)
    int   firingDuration;       // frames the solid beam fires      (≈0.8 s)
    int   idleCooldown;         // frames between attacks
    int   beamCount;            // beams per attack: level1→3, 2→4, 3→5
    float beamAngles[5];        // directions locked at WARNING start
    float beamLength;           // pixel reach of each beam
    // ─────────────────────────────────────────────────────────────────────────

    // Egg bursts
    std::vector<Egg> eggs;
    int   eggBurstTimer;
    int   eggBurstCooldown;

    int hitFlash;
    bool laserJustFired;

    // ------------------------------------------------------------------------
    // Constructor — pass game level so beam count scales correctly
    // ------------------------------------------------------------------------
    Boss(float cx=WIN_W/2.0f, float cy=WIN_H-120.0f, int lvl=1)
        : x(cx), y(cy),
          orbitAngle(0), orbitRadius(110), orbitSpeed(1.2f),
          hp(80), maxHp(80), active(true), phase(1), gameLevel(lvl),
          animTime(0), scale(0.01f), spawning(true),
          laserPhase(LaserPhase::IDLE), laserTimer(260),
          warningDuration(90),          // 1.5 sec warning
          firingDuration(55),           // ~0.9 sec fire
          idleCooldown(280),
          beamCount(2 + lvl),           // 3 beams level-1, 4 level-2, 5 level-3
          beamLength(WIN_H * 1.1f),
          eggBurstTimer(0), eggBurstCooldown(110),
          hitFlash(0), laserJustFired(false)
    {
        if(beamCount > 5) beamCount = 5;
        for(int i=0;i<5;i++) beamAngles[i] = 0.0f;
    }

    // ------------------------------------------------------------------------
    // Lock beam angles symmetrically toward the player's current position.
    // Called once at WARNING start — angles are FIXED for the whole attack.
    // This gives the player the entire WARNING window to escape.
    // ------------------------------------------------------------------------
    void lockBeamAngles(float playerX, float playerY){
        // Central beam aims at where player is RIGHT NOW
        float base = std::atan2(playerY - y, playerX - x);
        // Spread beams evenly: 25° apart (PI/7.2 rad)
        float spread = PI / 7.2f;
        int   half   = beamCount / 2;
        for(int i=0;i<beamCount;i++){
            float offset = (i - half) * spread;
            if(beamCount % 2 == 0) offset += spread * 0.5f;
            beamAngles[i] = base + offset;
        }
    }

    // ------------------------------------------------------------------------
    // Returns true if (px,py) is inside any FIRING beam hitbox.
    // Beam hitbox: a narrow rectangle of width±12 along the beam direction.
    // ------------------------------------------------------------------------
    bool isPlayerInLaser(float px, float py) const {
        if(laserPhase != LaserPhase::FIRING) return false;
        for(int i=0;i<beamCount;i++){
            float dx    = std::cos(beamAngles[i]);
            float dy    = std::sin(beamAngles[i]);
            float rx    = px - x;
            float ry    = py - y;
            float along = rx*dx + ry*dy;                  // distance along beam
            float perp  = std::abs(-rx*dy + ry*dx);       // perp distance
            if(along >= 0 && along <= beamLength && perp <= 12.0f) return true;
        }
        return false;
    }

    // ------------------------------------------------------------------------
    // Update
    // ------------------------------------------------------------------------
    void update(float playerX, float playerY){
        if(!active) return;
        animTime += 0.04f;

        // CG Concept 9: Spawn-in scale animation
        if(spawning){
            scale += 0.03f;
            if(scale >= 1.0f){ scale=1.0f; spawning=false; }
            return;
        }
        scale = 1.0f + 0.05f*std::sin(animTime*2);

        // CG Concept 8: Orbit movement (rotation)
        orbitAngle += orbitSpeed;
        if(orbitAngle >= 360.0f) orbitAngle -= 360.0f;
        float rad = orbitAngle * PI / 180.0f;
        x = WIN_W/2.0f + orbitRadius * std::cos(rad);
        y = WIN_H - 150.0f + 30.0f * std::sin(rad * 2.0f);

        // Phase 2 at <50% HP
        if(hp < maxHp/2 && phase==1){
            phase=2;
            orbitSpeed=2.2f;
            eggBurstCooldown=75;
            firingDuration=70;
            idleCooldown=210;
        }

        // ── Laser state machine ───────────────────────────────────────────────
        if(--laserTimer <= 0){
            switch(laserPhase){
                case LaserPhase::IDLE:
                    // Transition to WARNING: lock angles NOW, player has full
                    // warning window to move.
                    laserPhase = LaserPhase::WARNING;
                    laserTimer = warningDuration;
                    lockBeamAngles(playerX, playerY);
                    break;
                case LaserPhase::WARNING:
                    // Warning over → FIRE
                    laserPhase = LaserPhase::FIRING;
                    laserTimer = firingDuration;
                    laserJustFired = true;
                    break;
                case LaserPhase::FIRING:
                    // Done → back to IDLE
                    laserPhase = LaserPhase::IDLE;
                    laserTimer = idleCooldown;
                    break;
            }
        }

        // Egg burst — reduced count vs. original
        if(++eggBurstTimer >= eggBurstCooldown){
            eggBurstTimer=0;
            int count = (phase==2) ? 5 : 3;  // was 8/4 — halved
            for(int i=0;i<count;i++){
                float a   = 2.0f*PI*i/count + animTime*15;
                Egg eg(x + 18*std::cos(a), y + 18*std::sin(a),
                       (phase==2) ? 4.0f : 3.2f);
                eggs.push_back(eg);
            }
        }
        for(auto& e : eggs) e.update();
        eggs.erase(std::remove_if(eggs.begin(),eggs.end(),
            [](const Egg& e){ return !e.active; }), eggs.end());

        if(hitFlash>0) hitFlash--;
    }

    // ── Draw a WARNING dotted line along one beam angle ──────────────────────
    // Beam "reveals" progressively from boss outward during warning window,
    // then fully visible and pulsing at the end.
    void drawWarningBeam(float angle) const {
        // Progress 0→1 as warning countdown drains
        float progress = 1.0f - (float)laserTimer / (float)warningDuration;
        float visLen   = beamLength * std::min(progress * 1.4f, 1.0f);
        if(visLen < 20) visLen = 20;

        float ex = x + visLen * std::cos(angle);
        float ey = y + visLen * std::sin(angle);

        // Pulsing alpha for the dashes
        float pulse = 0.50f + 0.50f * std::abs(std::sin(animTime * 14.0f));
        Color dashCol(1.0f, 0.55f + 0.25f*pulse, 0.0f, 0.75f * pulse);
        Color dotCol (1.0f, 0.90f,               0.2f, 0.50f * pulse);

        // Draw dashed segments using Bresenham: 12 px ON, 10 px OFF
        float totalLen = std::sqrt((ex-x)*(ex-x)+(ey-y)*(ey-y));
        if(totalLen < 1) return;
        float dashLen = 12.0f, gapLen = 10.0f, segCycle = dashLen+gapLen;
        float t = 0;
        while(t < totalLen){
            float t1 = t;
            float t2 = std::min(t + dashLen, totalLen);
            int sx1=(int)(x+(t1/totalLen)*(ex-x)), sy1=(int)(y+(t1/totalLen)*(ey-y));
            int sx2=(int)(x+(t2/totalLen)*(ex-x)), sy2=(int)(y+(t2/totalLen)*(ey-y));
            bresenhamLine(sx1,sy1,sx2,sy2, dashCol);
            // Small dot at dash start for visibility
            drawCircle(sx1,sy1,2.2f, dotCol);
            t += segCycle;
        }
        // Arrowhead tip dot
        drawCircle(ex, ey, 4.5f * pulse, Color(1.0f,0.8f,0.0f,0.8f*pulse));
    }

    // ── Draw a SOLID firing beam along one angle ──────────────────────────────
    // Three-layer: soft outer glow (DDA) + mid glow (DDA) + core (Bresenham)
    void drawFiringBeam(float angle) const {
        float ex  = x + beamLength * std::cos(angle);
        float ey  = y + beamLength * std::sin(angle);
        float flk = 0.70f + 0.30f * std::sin(animTime * 30.0f);

        // Perpendicular offsets for glow layers
        float nx = -std::sin(angle), ny = std::cos(angle);

        // Outer wide glow (DDA)
        ddaLine((int)(x+nx*7),(int)(y+ny*7),(int)(ex+nx*7),(int)(ey+ny*7),
                Color(1.0f,0.15f,0.0f, 0.18f*flk));
        ddaLine((int)(x-nx*7),(int)(y-ny*7),(int)(ex-nx*7),(int)(ey-ny*7),
                Color(1.0f,0.15f,0.0f, 0.18f*flk));

        // Mid glow (DDA)
        ddaLine((int)(x+nx*3),(int)(y+ny*3),(int)(ex+nx*3),(int)(ey+ny*3),
                Color(1.0f,0.40f,0.10f, 0.50f*flk));
        ddaLine((int)(x-nx*3),(int)(y-ny*3),(int)(ex-nx*3),(int)(ey-ny*3),
                Color(1.0f,0.40f,0.10f, 0.50f*flk));

        // Core beam — Bresenham (CG Concept 3)
        bresenhamLine((int)x,(int)y,(int)ex,(int)ey,
                      Color(1.0f, 0.88f*flk, 0.65f*flk));

        // Origin flash circle (Midpoint Circle — CG Concept 4)
        drawCircle(x, y, 11.0f*flk, Color(1.0f, 0.40f, 0.0f, 0.60f*flk));
        // End point glow
        drawCircle(ex, ey, 6.0f*flk, Color(1.0f, 0.60f, 0.2f, 0.40f*flk));
    }

    void drawAllBeams() const {
        if(laserPhase == LaserPhase::IDLE) return;
        for(int i=0;i<beamCount;i++){
            if(laserPhase == LaserPhase::WARNING)
                drawWarningBeam(beamAngles[i]);
            else
                drawFiringBeam(beamAngles[i]);
        }
    }

    void drawBody() const {
        glPushMatrix();
        glTranslatef(x, y, 0);
        glScalef(scale, scale, 1);

        // ── Dynamic Flight Animation Variables (CG Concept 8: Rotation) ─────
        float flapFreq  = (phase == 2) ? 5.4f : 3.8f;
        float flapAngle = (phase == 2 ? 26.0f : 20.0f) * std::sin(animTime * flapFreq);
        float flightBob = 4.2f * std::sin(animTime * flapFreq);
        float bankTilt  = 4.0f * std::sin(animTime * 1.8f);
        float tailSway  = 6.0f * std::sin(animTime * 2.4f);
        float combSway  = 2.2f * std::sin(animTime * flapFreq);
        float wattleSway= 3.5f * std::sin(animTime * flapFreq - 0.6f);

        // Banking tilt rotation
        glRotatef(bankTilt, 0, 0, 1);

        // ── Boss Dreadnought Palette ────────────────────────────────────────
        // Phase 1: Imperial Dreadnought Violet / Obsidian
        // Phase 2: Enraged Hellfire Crimson / Volcanic Ember
        bool p2 = (phase == 2);
        Color colBase       = p2 ? Color(0.72f, 0.12f, 0.10f) : Color(0.38f, 0.14f, 0.58f);
        Color colShadow     = p2 ? Color(0.36f, 0.05f, 0.05f) : Color(0.22f, 0.07f, 0.36f);
        Color colHighlight  = p2 ? Color(0.96f, 0.36f, 0.14f) : Color(0.56f, 0.28f, 0.82f);
        Color colChest      = p2 ? Color(0.86f, 0.22f, 0.08f) : Color(0.46f, 0.20f, 0.70f);
        Color colCore       = p2 ? Color(1.00f, 0.88f, 0.20f) : Color(0.12f, 0.88f, 0.96f);
        Color colComb       = p2 ? Color(1.00f, 0.22f, 0.05f) : Color(0.85f, 0.12f, 0.35f);
        Color colCombDark   = p2 ? Color(0.65f, 0.08f, 0.02f) : Color(0.52f, 0.06f, 0.20f);
        Color colCombHigh   = p2 ? Color(1.00f, 0.55f, 0.15f) : Color(1.00f, 0.35f, 0.55f);
        Color colBeak       = p2 ? Color(1.00f, 0.70f, 0.15f) : Color(0.96f, 0.64f, 0.12f);
        Color colBeakDark   = p2 ? Color(0.75f, 0.35f, 0.05f) : Color(0.65f, 0.36f, 0.06f);
        Color colClaw       = Color(0.28f, 0.30f, 0.36f);
        Color colEyeIris    = p2 ? Color(1.00f, 0.90f, 0.20f) : Color(0.10f, 0.92f, 1.00f);

        float by = flightBob;

        // ── 0. Plasma Energy Aura (CG Concept 4: Midpoint Circle) ───────────
        float auraPulse = 0.18f + 0.10f * std::sin(animTime * 3.5f);
        Color auraCol   = p2 ? Color(1.0f, 0.22f, 0.02f, auraPulse) : Color(0.48f, 0.08f, 0.92f, auraPulse);
        glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        midpointCircle(0, (int)by, 76, auraCol, true);
        midpointCircle(0, (int)by, 68, Color(auraCol.r, auraCol.g, auraCol.b, auraPulse * 1.6f), false);
        if(p2){
            midpointCircle(0, (int)by, 84, Color(1.0f, 0.45f, 0.0f, auraPulse * 0.7f), false);
        }
        glDisable(GL_BLEND);

        // ── 1. Aerodynamic Tail Sickle Plumes ───────────────────────────────
        std::vector<Vec2> tailL = {
            {-6.0f, by - 22.0f},
            {-28.0f + tailSway, by - 52.0f},
            {-16.0f + tailSway * 0.7f, by - 58.0f},
            {-2.0f, by - 30.0f}
        };
        scanlineFill(tailL, colShadow);
        ddaLine((int)-6, (int)(by - 22), (int)(-24 + tailSway), (int)(by - 54), colHighlight);

        std::vector<Vec2> tailR = {
            {6.0f, by - 22.0f},
            {28.0f - tailSway, by - 52.0f},
            {16.0f - tailSway * 0.7f, by - 58.0f},
            {2.0f, by - 30.0f}
        };
        scanlineFill(tailR, colShadow);
        ddaLine((int)6, (int)(by - 22), (int)(24 - tailSway), (int)(by - 54), colHighlight);

        std::vector<Vec2> tailC = {
            {-7.0f, by - 24.0f},
            {tailSway * 0.5f, by - 62.0f},
            {7.0f, by - 24.0f}
        };
        scanlineFill(tailC, colBase);
        ddaLine((int)0, (int)(by - 24), (int)(tailSway * 0.5f), (int)(by - 60), colHighlight);

        // ── 2. Tucked Flight Claws / Armored Talons ──────────────────────────
        std::vector<Vec2> legL = {{-25.0f, by - 20.0f}, {-17.0f, by - 20.0f}, {-16.0f, by - 30.0f}, {-26.0f, by - 30.0f}};
        scanlineFill(legL, colShadow);
        ddaLine((int)-25, (int)(by - 30), (int)-31, (int)(by - 40), colClaw);
        ddaLine((int)-21, (int)(by - 30), (int)-21, (int)(by - 42), colClaw);
        ddaLine((int)-17, (int)(by - 30), (int)-12, (int)(by - 39), colClaw);

        std::vector<Vec2> legR = {{17.0f, by - 20.0f}, {25.0f, by - 20.0f}, {26.0f, by - 30.0f}, {16.0f, by - 30.0f}};
        scanlineFill(legR, colShadow);
        ddaLine((int)17, (int)(by - 30), (int)12, (int)(by - 39), colClaw);
        ddaLine((int)21, (int)(by - 30), (int)21, (int)(by - 42), colClaw);
        ddaLine((int)25, (int)(by - 30), (int)31, (int)(by - 40), colClaw);

        // ── 3. Colossal Articulated Wings (CG Concept 8: Rotation) ───────────
        // Left Wing
        glPushMatrix();
        glTranslatef(-36.0f, by + 6.0f, 0.0f);
        glRotatef(-flapAngle, 0.0f, 0.0f, 1.0f);
        std::vector<Vec2> lwPrim = {
            {0.0f, -6.0f}, {-20.0f, -18.0f}, {-46.0f, -6.0f},
            {-56.0f, 14.0f}, {-40.0f, 30.0f}, {-16.0f, 26.0f}, {0.0f, 14.0f}
        };
        scanlineFill(lwPrim, colShadow);
        std::vector<Vec2> lwSec = {
            {0.0f, -4.0f}, {-16.0f, -14.0f}, {-38.0f, -2.0f},
            {-46.0f, 14.0f}, {-32.0f, 24.0f}, {-12.0f, 20.0f}, {0.0f, 10.0f}
        };
        scanlineFill(lwSec, colBase);
        std::vector<Vec2> lwCov = {
            {0.0f, -2.0f}, {-12.0f, -8.0f}, {-26.0f, 2.0f},
            {-22.0f, 14.0f}, {0.0f, 8.0f}
        };
        scanlineFill(lwCov, colHighlight);
        ddaLine(-16, -14, -38, -2, colShadow);
        ddaLine(-12, 5, -42, 14, colShadow);
        midpointCircle(-48, 14, 3, colCore, true);
        glPopMatrix();

        // Right Wing (mirrored)
        glPushMatrix();
        glTranslatef(36.0f, by + 6.0f, 0.0f);
        glRotatef(flapAngle, 0.0f, 0.0f, 1.0f);
        std::vector<Vec2> rwPrim = {
            {0.0f, -6.0f}, {20.0f, -18.0f}, {46.0f, -6.0f},
            {56.0f, 14.0f}, {40.0f, 30.0f}, {16.0f, 26.0f}, {0.0f, 14.0f}
        };
        scanlineFill(rwPrim, colShadow);
        std::vector<Vec2> rwSec = {
            {0.0f, -4.0f}, {16.0f, -14.0f}, {38.0f, -2.0f},
            {48.0f, 14.0f}, {32.0f, 24.0f}, {12.0f, 20.0f}, {0.0f, 10.0f}
        };
        scanlineFill(rwSec, colBase);
        std::vector<Vec2> rwCov = {
            {0.0f, -2.0f}, {12.0f, -8.0f}, {26.0f, 2.0f},
            {22.0f, 14.0f}, {0.0f, 8.0f}
        };
        scanlineFill(rwCov, colHighlight);
        ddaLine(16, -14, 38, -2, colShadow);
        ddaLine(12, 5, 42, 14, colShadow);
        midpointCircle(48, 14, 3, colCore, true);
        glPopMatrix();

        // ── 4. Main Torso Carapace (CG Concept 5: Scan-Line Fill) ───────────
        std::vector<Vec2> torso = {
            {0.0f, by + 20.0f},
            {-18.0f, by + 16.0f},
            {-36.0f, by + 8.0f},
            {-45.0f, by - 8.0f},
            {-38.0f, by - 24.0f},
            {-20.0f, by - 35.0f},
            {0.0f, by - 38.0f},
            {20.0f, by - 35.0f},
            {38.0f, by - 24.0f},
            {45.0f, by - 8.0f},
            {36.0f, by + 8.0f},
            {18.0f, by + 16.0f}
        };
        scanlineFill(torso, colBase);

        // Lower body underbelly shadow for 3D depth
        std::vector<Vec2> torsoUnder = {
            {-38.0f, by - 24.0f}, {-20.0f, by - 35.0f}, {0.0f, by - 38.0f},
            {20.0f, by - 35.0f}, {38.0f, by - 24.0f}, {25.0f, by - 16.0f},
            {0.0f, by - 22.0f}, {-25.0f, by - 16.0f}
        };
        scanlineFill(torsoUnder, colShadow);

        // ── 5. Scalloped Breastplate Armor & Reactor Core ────────────────────
        std::vector<Vec2> chest1 = {
            {-24.0f, by + 12.0f}, {24.0f, by + 12.0f},
            {28.0f, by - 2.0f}, {0.0f, by - 10.0f}, {-28.0f, by - 2.0f}
        };
        scanlineFill(chest1, colChest);

        std::vector<Vec2> chest2 = {
            {-18.0f, by + 4.0f}, {18.0f, by + 4.0f},
            {20.0f, by - 12.0f}, {0.0f, by - 18.0f}, {-20.0f, by - 12.0f}
        };
        scanlineFill(chest2, colHighlight);

        // Central Power Reactor Core (where laser beams emanate from)
        midpointCircle(0, (int)(by - 6), 9, colShadow, true);
        midpointCircle(0, (int)(by - 6), 7, colCore, true);
        midpointCircle(0, (int)(by - 6), 4, Color(1, 1, 1), true);

        // ── 6. Feathered Neck Ruff / Gorget ──────────────────────────────────
        std::vector<Vec2> neck = {
            {-20.0f, by + 14.0f}, {20.0f, by + 14.0f},
            {15.0f, by + 26.0f}, {-15.0f, by + 26.0f}
        };
        scanlineFill(neck, colHighlight);

        // ── 7. Massive Head Profile ──────────────────────────────────────────
        float hy = by + 28.0f;
        drawCircle(0.0f, hy, 18.0f, colBase);
        drawCircle(0.0f, hy + 4.0f, 12.0f, colHighlight);

        // Cheek feather tufts
        std::vector<Vec2> cheekL = {{-14.0f, hy - 4.0f}, {-25.0f, hy + 2.0f}, {-14.0f, hy + 8.0f}};
        scanlineFill(cheekL, colShadow);
        std::vector<Vec2> cheekR = {{14.0f, hy - 4.0f}, {25.0f, hy + 2.0f}, {14.0f, hy + 8.0f}};
        scanlineFill(cheekR, colShadow);

        // ── 8. Majestic Rooster Comb / Crown (Organic 5-lobe crest) ──────────
        std::vector<Vec2> crestBase = {
            {-17.0f, hy + 10.0f}, {17.0f, hy + 10.0f},
            {13.0f, hy + 18.0f}, {-13.0f, hy + 18.0f}
        };
        scanlineFill(crestBase, colComb);

        drawCircle(-16.0f, hy + 14.0f + combSway * 0.5f, 5.0f, colCombDark);
        drawCircle(-9.0f, hy + 18.0f + combSway * 0.8f, 6.2f, colComb);
        drawCircle(0.0f, hy + 21.0f + combSway, 7.8f, colComb);
        midpointCircle(0, (int)(hy + 21.0f + combSway), 5, colCombHigh, true);
        drawCircle(9.0f, hy + 18.0f + combSway * 0.8f, 6.2f, colComb);
        drawCircle(16.0f, hy + 14.0f + combSway * 0.5f, 5.0f, colCombDark);

        // ── 9. Menacing Arcade Boss Visor & Eyes ─────────────────────────────
        ddaLine((int)-22, (int)(hy + 10), (int)-7, (int)(hy + 5), colShadow);
        ddaLine((int)22, (int)(hy + 10), (int)7, (int)(hy + 5), colShadow);

        drawCircle(-13.0f, hy + 4.0f, 6.0f, Color(0.08f, 0.08f, 0.10f));
        midpointCircle(-13, (int)(hy + 4.0f), 5, colEyeIris, true);
        midpointCircle(-13, (int)(hy + 4.0f), 2, Color(0.02f, 0.02f, 0.02f), true);
        midpointCircle(-12, (int)(hy + 6.0f), 1, Color(1.0f, 1.0f, 1.0f), true);

        drawCircle(13.0f, hy + 4.0f, 6.0f, Color(0.08f, 0.08f, 0.10f));
        midpointCircle(13, (int)(hy + 4.0f), 5, colEyeIris, true);
        midpointCircle(13, (int)(hy + 4.0f), 2, Color(0.02f, 0.02f, 0.02f), true);
        midpointCircle(14, (int)(hy + 6.0f), 1, Color(1.0f, 1.0f, 1.0f), true);

        // ── 10. Sculpted Raptor Beak & Chin Wattles ──────────────────────────
        drawCircle(-5.0f + wattleSway, hy - 9.0f, 4.2f, colComb);
        drawCircle(5.0f + wattleSway, hy - 9.0f, 4.2f, colComb);

        std::vector<Vec2> beakUp = {
            {-10.0f, hy + 2.0f}, {10.0f, hy + 2.0f},
            {7.0f, hy - 7.0f}, {0.0f, hy - 16.0f}, {-7.0f, hy - 7.0f}
        };
        scanlineFill(beakUp, colBeak);

        std::vector<Vec2> beakLo = {
            {-6.0f, hy - 6.0f}, {6.0f, hy - 6.0f}, {0.0f, hy - 16.0f}
        };
        scanlineFill(beakLo, colBeakDark);

        ddaLine((int)-10, (int)(hy + 2), (int)0, (int)(hy - 16), colBeakDark);
        ddaLine((int)10, (int)(hy + 2), (int)0, (int)(hy - 16), colBeakDark);
        ddaLine((int)-4, (int)(hy), (int)-2, (int)(hy - 1), Color(0.2f, 0.1f, 0.0f));
        ddaLine((int)4, (int)(hy), (int)2, (int)(hy - 1), Color(0.2f, 0.1f, 0.0f));

        // ── 11. Hit Flash: Pure white silhouette overlay (NO red box) ────────
        if(hitFlash > 0){
            glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            drawCircle(0.0f, by + 6.0f, 54.0f, Color(1.0f, 1.0f, 1.0f, 0.40f));
            glDisable(GL_BLEND);
        }

        glPopMatrix();
    }

    void drawHpBar() const {
        float barW=200, barH=16;
        float bx=WIN_W/2.0f-barW/2, by=WIN_H-30;
        drawRect(bx,by,barW,barH,Color(0.2f,0.2f,0.2f));
        float ratio=(float)hp/maxHp;
        Color barCol=(ratio>0.5f)?Color(0.9f,0.1f,0.9f):(ratio>0.25f)?Color(1,0.5f,0):Color(0.9f,0.1f,0.1f);
        drawRect(bx,by,barW*ratio,barH,barCol);
        drawRectOutline(bx,by,barW,barH,Color(0.9f,0.7f,1.0f),2.0f);
        ddaLine((int)(bx-45),(int)(by+8),(int)(bx-5),(int)(by+8),Color(1,1,1));

        // Laser phase indicator strip above HP bar
        if(laserPhase == LaserPhase::WARNING){
            float pa = 0.55f+0.45f*std::abs(std::sin(animTime*12));
            drawRect(WIN_W/2-55, by-22, 110, 17, Color(0.25f,0.10f,0.0f, 0.85f));
            drawRectOutline(WIN_W/2-55, by-22, 110, 17, Color(1.0f,0.55f,0.0f,pa), 1.5f);
            // "DODGE!" text via DDA lines (simplified)
            float tx=WIN_W/2-28; float ty=by-14;
            ddaLine((int)tx,(int)ty,(int)(tx+56),(int)ty,Color(1,0.7f,0,pa));
        } else if(laserPhase == LaserPhase::FIRING){
            float fa = 0.7f+0.3f*std::sin(animTime*20);
            drawRect(WIN_W/2-45, by-22, 90, 17, Color(0.3f,0.0f,0.0f, 0.90f));
            drawRectOutline(WIN_W/2-45, by-22, 90, 17, Color(1.0f,0.1f,0.0f,fa), 2.0f);
        }
    }

    void draw(float /*px*/, float /*py*/) const {
        if(!active) return;
        drawAllBeams();  // beams drawn BEHIND boss body
        drawBody();
        for(auto& e : eggs) e.draw();
        drawHpBar();
    }

    void takeDamage(int dmg){
        hp-=dmg; hitFlash=8;
        if(hp<=0){ hp=0; active=false; }
    }

    AABB getAABB() const { return {x-55*scale,y-45*scale,110*scale,90*scale}; }
};
