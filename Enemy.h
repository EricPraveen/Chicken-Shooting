// =============================================================================
// Enemy.h — Enemy chicken types: Normal, Fast, Armored
// =============================================================================
// CG Concepts:
//   Scan-Line Fill Algorithm — chicken body drawn by polygon fill
//   DDA/Bresenham Lines      — feathers, beak details
//   Keyframe Animation (14)  — horizontal oscillation via sin/cos keyframes
//   Translation              — horizontal movement + descent
//   AABB Collision           — enemy bounding box
// =============================================================================

#pragma once
#include "Utils.h"
#include "Bullet.h"
#include <vector>
#include <functional>

enum class EnemyType { NORMAL, FAST, ARMORED };

struct Enemy {
    float x, y;         // center
    float baseX;        // original x for oscillation reference
    float speed;        // horizontal speed
    float descentSpeed; // how fast it moves down
    int   hp, maxHp;
    bool  active;
    EnemyType type;

    // Keyframe animation state (CG Concept 14)
    float animTime;     // time accumulator for keyframe interpolation
    float wingFlap;     // wing angle for flapping animation

    // Egg shooting
    int   eggCooldown;
    int   eggTimer;
    std::vector<Egg> eggs;

    // Hit flash
    int hitFlash;

    // Formation entry state (CG Concept 14: Keyframe & Path Interpolation)
    bool  inFormation;
    float targetX, targetY;
    int   entryPattern;     // 1=Top-Down Row-by-Row, 2=Dual-Wing Cross, 3=Spiral Swarm
    float entryDelay;       // frame delay before flight begins
    float entryProgress;    // 0.0 -> 1.0
    float entryStartX, entryStartY;
    float entryCtrlX, entryCtrlY;

    Enemy(float x, float y, EnemyType t=EnemyType::NORMAL)
        : x(x), y(y), baseX(x), speed(2.0f), descentSpeed(0.08f),
          hp(3), maxHp(3), active(true), type(t),
          animTime(0), wingFlap(0),
          hitFlash(0),
          inFormation(true), targetX(x), targetY(y),
          entryPattern(1), entryDelay(0), entryProgress(1.0f),
          entryStartX(x), entryStartY(y), entryCtrlX(x), entryCtrlY(y)
    {
        switch(t){
            case EnemyType::NORMAL:  hp=3;  maxHp=3;  speed=2.0f; eggCooldown=360; break;
            case EnemyType::FAST:    hp=2;  maxHp=2;  speed=4.0f; eggCooldown=240; break;
            case EnemyType::ARMORED: hp=6;  maxHp=6;  speed=1.5f; eggCooldown=480; break;
        }
        eggTimer = rand()%eggCooldown;
    }

    void setupEntry(int pattern, int r, int c, int rows, int cols, float tx, float ty){
        targetX = tx;
        targetY = ty;
        inFormation = false;
        entryPattern = pattern;
        entryProgress = 0.0f;

        switch(pattern){
            case 1: {
                // Level 1: Orderly Staggered Top-Down Descent (Row by row with center wave)
                entryDelay = r * 22.0f + std::abs(c - (cols - 1) * 0.5f) * 4.0f;
                entryStartX = tx;
                entryStartY = WIN_H + 60.0f + r * 30.0f;
                x = entryStartX;
                y = entryStartY;
                baseX = tx;
                break;
            }
            case 2: {
                // Level 2: Dual-Wing Cross Swoop (Left & Right flanking cross arcs)
                bool isLeft = (c < (cols + 1) / 2);
                if(isLeft){
                    entryStartX = -60.0f - c * 20.0f;
                    entryStartY = WIN_H + 40.0f + r * 25.0f;
                    entryCtrlX  = WIN_W * 0.58f;
                    entryCtrlY  = WIN_H * 0.38f;
                    entryDelay  = r * 18.0f + c * 7.0f;
                } else {
                    int rIndex = (cols - 1) - c;
                    entryStartX = WIN_W + 60.0f + rIndex * 20.0f;
                    entryStartY = WIN_H + 40.0f + r * 25.0f;
                    entryCtrlX  = WIN_W * 0.42f;
                    entryCtrlY  = WIN_H * 0.38f;
                    entryDelay  = r * 18.0f + rIndex * 7.0f;
                }
                x = entryStartX;
                y = entryStartY;
                baseX = tx;
                break;
            }
            case 3:
            default: {
                // Level 3: Spiral S-Curve Swarm (Continuous snaking serpent swarm)
                int seqIndex = r * cols + c;
                bool alt = (c % 2 == 0);
                entryStartX = alt ? -50.0f : WIN_W + 50.0f;
                entryStartY = WIN_H + 60.0f + (seqIndex % cols) * 12.0f;
                entryDelay  = seqIndex * 4.2f;
                x = entryStartX;
                y = entryStartY;
                baseX = tx;
                break;
            }
        }
    }

    void update(){
        animTime += 0.028f * speed;

        // CG Concept 14: Keyframe Animation — natural wing flap
        float rawFlap = std::sin(animTime * 5.5f);
        wingFlap = (rawFlap > 0)
            ? 32.0f * std::pow(rawFlap, 0.7f)
            : 18.0f * rawFlap;

        // ── Check Entry Flight Phase ──────────────────────────────────────────
        if(!inFormation){
            if(entryDelay > 0.0f){
                entryDelay -= 1.0f;
                x = entryStartX;
                y = entryStartY;
                return;
            }

            switch(entryPattern){
                case 1: {
                    // Level 1: Smooth ease-out descent
                    entryProgress += 0.018f;
                    float t = std::min(entryProgress, 1.0f);
                    float ease = 1.0f - std::pow(1.0f - t, 3.0f);
                    y = entryStartY + (targetY - entryStartY) * ease;
                    x = targetX + 16.0f * (1.0f - ease) * std::sin(ease * PI * 2.0f);
                    break;
                }
                case 2: {
                    // Level 2: Quadratic Bézier crossing swoop
                    entryProgress += 0.016f;
                    float t = std::min(entryProgress, 1.0f);
                    float u = 1.0f - t;
                    x = u * u * entryStartX + 2.0f * u * t * entryCtrlX + t * t * targetX;
                    y = u * u * entryStartY + 2.0f * u * t * entryCtrlY + t * t * targetY;
                    break;
                }
                case 3:
                default: {
                    // Level 3: S-curve spiral wave
                    entryProgress += 0.016f;
                    float t = std::min(entryProgress, 1.0f);
                    float ease = 1.0f - std::pow(1.0f - t, 2.4f);
                    y = entryStartY + (targetY - entryStartY) * ease;
                    float side = (entryStartX < WIN_W * 0.5f) ? 1.0f : -1.0f;
                    x = entryStartX + (targetX - entryStartX) * ease +
                        side * 110.0f * (1.0f - ease) * std::sin(ease * PI * 3.0f);
                    break;
                }
            }

            if(entryProgress >= 1.0f){
                inFormation = true;
                x = targetX;
                y = targetY;
                baseX = targetX;
            }
            return; // don't shoot eggs or descend further during entry flight
        }

        // ── Normal Formation Movement & Egg Dropping ─────────────────────────
        x = baseX + 60.0f * std::sin(animTime);
        y -= descentSpeed;

        if(++eggTimer >= eggCooldown){
            eggTimer = 0;
            float eggSpd = (type==EnemyType::FAST) ? 6.0f : 4.0f;
            eggs.emplace_back(x, y-20, eggSpd);
        }
        for(auto& e : eggs) e.update();
        eggs.erase(std::remove_if(eggs.begin(),eggs.end(),
            [](const Egg& e){ return !e.active; }), eggs.end());

        if(hitFlash > 0) hitFlash--;
    }

    // ── Helper: draw a rounded comb bump (small filled semicircle) ────────────
    void drawCombBump(float cx, float cy, float r, const Color& col) const {
        // Top half arc using GL_TRIANGLE_FAN
        col.apply();
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy);
        int segs = 10;
        for(int i = 0; i <= segs; i++){
            float a = PI * i / segs; // 0..PI = top half
            glVertex2f(cx + r * std::cos(a), cy + r * std::sin(a));
        }
        glEnd();
    }

    void drawNormalChicken(float flash) const {
        // ── Vertical Flying Animation: lift oscillation + wing beat ──────────
        float wingCycle  = animTime * 6.0f;
        float flightBob  = 2.4f * std::sin(wingCycle);
        float flapAngle  = 28.0f * std::sin(wingCycle);
        float headMotion = 1.2f * std::sin(wingCycle - 0.5f);
        float tailSway   = 2.0f * std::sin(wingCycle * 0.5f);

        // Plumage colors: authentic retro arcade chicken (cream/ivory white)
        Color colBase     = (flash > 0) ? Color(1.0f,1.0f,1.0f) : Color(0.96f, 0.95f, 0.90f);
        Color colShadow   = (flash > 0) ? Color(1.0f,1.0f,1.0f) : Color(0.80f, 0.78f, 0.72f);
        Color colHighlight= (flash > 0) ? Color(1.0f,1.0f,1.0f) : Color(1.0f, 1.0f, 0.98f);
        Color colComb     = (flash > 0) ? Color(1.0f,0.6f,0.6f) : Color(0.92f, 0.12f, 0.14f);
        Color colCombDark = (flash > 0) ? Color(1.0f,0.8f,0.8f) : Color(0.72f, 0.08f, 0.10f);
        Color colBeak     = Color(1.0f, 0.58f, 0.0f);
        Color colBeakDark = Color(0.85f, 0.44f, 0.0f);
        Color colLeg      = Color(1.0f, 0.55f, 0.0f);

        float by = y + flightBob;

        // ── 1. Tail Plumes (Rooster sickle feathers trailing beneath) ────────
        // Center sickle plume
        std::vector<Vec2> tailCenter = {
            {x-4, by-10}, {x+4, by-10}, {x+tailSway+2, by-24}, {x+tailSway, by-26}, {x+tailSway-2, by-24}
        };
        scanlineFill(tailCenter, colShadow);
        ddaLine((int)(x+tailSway), (int)(by-26), (int)x, (int)(by-10), colHighlight);

        // Left sickle plume
        std::vector<Vec2> tailLeft = {
            {x-3, by-8}, {x-8, by-12}, {x-14+tailSway, by-22}, {x-11+tailSway, by-22}, {x-2, by-9}
        };
        scanlineFill(tailLeft, colBase);

        // Right sickle plume
        std::vector<Vec2> tailRight = {
            {x+3, by-8}, {x+8, by-12}, {x+14+tailSway, by-22}, {x+11+tailSway, by-22}, {x+2, by-9}
        };
        scanlineFill(tailRight, colBase);

        // ── 2. Tucked Flight Legs & Claws ────────────────────────────────────
        // Left leg & 3 claw toes (folded back aerodynamically)
        ddaLine((int)(x-7), (int)(by-12), (int)(x-10), (int)(by-20), colLeg);
        ddaLine((int)(x-10), (int)(by-20), (int)(x-14), (int)(by-23), colLeg);
        ddaLine((int)(x-10), (int)(by-20), (int)(x-10), (int)(by-24), colLeg);
        ddaLine((int)(x-10), (int)(by-20), (int)(x-6),  (int)(by-23), colLeg);

        // Right leg & 3 claw toes
        ddaLine((int)(x+7), (int)(by-12), (int)(x+10), (int)(by-20), colLeg);
        ddaLine((int)(x+10), (int)(by-20), (int)(x+14), (int)(by-23), colLeg);
        ddaLine((int)(x+10), (int)(by-20), (int)(x+10), (int)(by-24), colLeg);
        ddaLine((int)(x+10), (int)(by-20), (int)(x+6),  (int)(by-23), colLeg);

        // ── 3. Torso / Body (Layered organic avian silhouette) ───────────────
        std::vector<Vec2> body = {
            {x,        by+22},
            {x-13,     by+16},
            {x-20,     by+4},
            {x-19,     by-7},
            {x-11,     by-15},
            {x+11,     by-15},
            {x+19,     by-7},
            {x+20,     by+4},
            {x+13,     by+16}
        };
        scanlineFill(body, colBase);

        // Lower body shadow curve for 3D depth
        std::vector<Vec2> bodyUnder = {
            {x-19, by-7}, {x-11, by-15}, {x+11, by-15}, {x+19, by-7},
            {x+14, by-4}, {x,     by-7},  {x-14, by-4}
        };
        scanlineFill(bodyUnder, colShadow);

        // Scalloped breast plumage highlights
        std::vector<Vec2> chestHigh = {
            {x-7, by+17}, {x+7, by+17}, {x+10, by+6}, {x, by+2}, {x-10, by+6}
        };
        glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        scanlineFill(chestHigh, Color(colHighlight.r, colHighlight.g, colHighlight.b, 0.45f));
        glDisable(GL_BLEND);

        // ── 4. Articulated Wings with Flapping Motion ────────────────────────
        // Left Wing
        glPushMatrix();
        glTranslatef(x-18, by+2, 0);
        glRotatef(-flapAngle, 0, 0, 1);
        // Primary flight feathers (outer long feathers)
        std::vector<Vec2> lwPrim = {{0,0}, {-14,-4}, {-24,8}, {-16,18}, {0,12}};
        scanlineFill(lwPrim, colShadow);
        // Secondary wing feathers (main layer)
        std::vector<Vec2> lwSec  = {{0,0}, {-12,-2}, {-21,8}, {-14,16}, {0,10}};
        scanlineFill(lwSec, colBase);
        // Wing covert feather cap
        std::vector<Vec2> lwCov  = {{0,0}, {-8,0}, {-13,6}, {-7,10}, {0,6}};
        scanlineFill(lwCov, colHighlight);
        // Feather barb contour lines
        ddaLine(-12, -2, -21, 8, colShadow);
        ddaLine(-8, 3, -16, 12, colShadow);
        glPopMatrix();

        // Right Wing (mirrored)
        glPushMatrix();
        glTranslatef(x+18, by+2, 0);
        glRotatef(flapAngle, 0, 0, 1);
        std::vector<Vec2> rwPrim = {{0,0}, {14,-4}, {24,8}, {16,18}, {0,12}};
        scanlineFill(rwPrim, colShadow);
        std::vector<Vec2> rwSec  = {{0,0}, {12,-2}, {21,8}, {14,16}, {0,10}};
        scanlineFill(rwSec, colBase);
        std::vector<Vec2> rwCov  = {{0,0}, {8,0}, {13,6}, {7,10}, {0,6}};
        scanlineFill(rwCov, colHighlight);
        ddaLine(12, -2, 21, 8, colShadow);
        ddaLine(8, 3, 16, 12, colShadow);
        glPopMatrix();

        // ── 5. Feathered Neck Collar (Ruff connecting body to head) ──────────
        std::vector<Vec2> neckRuff = {
            {x-11, by+15}, {x+11, by+15}, {x+8, by+22}, {x-8, by+22}
        };
        scanlineFill(neckRuff, colHighlight);

        // ── 6. Head Profile ──────────────────────────────────────────────────
        float hy = by + 28 + headMotion;
        drawCircle(x, hy, 14, colBase);
        drawCircle(x, hy+3, 9, colHighlight); // directional light highlight

        // ── 7. Rooster Comb (3 organic crest lobes — NO rectangular band!) ────
        float combY = hy + 13;
        drawCombBump(x,    combY+2, 5.5f, colComb);
        drawCombBump(x-5,  combY,   4.0f, colCombDark);
        drawCombBump(x+5,  combY,   4.0f, colComb);

        // ── 8. Expressive Retro Eyes ─────────────────────────────────────────
        // Left eye
        drawCircle(x-5, hy+3, 4.5f, Color(1,1,1));
        midpointCircle((int)(x-5), (int)(hy+3), 4, Color(0.1f,0.1f,0.1f), false);
        midpointCircle((int)(x-5), (int)(hy+3), 2, Color(0.05f,0.05f,0.05f), true);
        midpointCircle((int)(x-4), (int)(hy+5), 1, Color(1,1,1), true); // shine

        // Right eye
        drawCircle(x+5, hy+3, 4.5f, Color(1,1,1));
        midpointCircle((int)(x+5), (int)(hy+3), 4, Color(0.1f,0.1f,0.1f), false);
        midpointCircle((int)(x+5), (int)(hy+3), 2, Color(0.05f,0.05f,0.05f), true);
        midpointCircle((int)(x+6), (int)(hy+5), 1, Color(1,1,1), true); // shine

        // ── 9. Sculpted Beak & Chin Wattle ───────────────────────────────────
        // Chin wattle (red double droplet)
        drawCircle(x-2, hy-8, 3.5f, colComb);
        drawCircle(x+2, hy-8, 3.5f, colComb);

        // Upper beak
        std::vector<Vec2> beakUp = {{x-5, hy-2}, {x+5, hy-2}, {x+2, hy-9}, {x-2, hy-9}};
        scanlineFill(beakUp, colBeak);
        // Lower beak
        std::vector<Vec2> beakLo = {{x-4, hy-5}, {x+4, hy-5}, {x, hy-9}};
        scanlineFill(beakLo, colBeakDark);
        // Beak outline line
        ddaLine((int)(x-5), (int)(hy-5), (int)(x+5), (int)(hy-5), colBeakDark);
    }

    void drawFastChicken(float flash) const {
        // ── Aerodynamic flight animation: rapid wing flutter ─────────────────
        float wingCycle  = animTime * 9.0f;
        float flightBob  = 1.8f * std::sin(wingCycle);
        float flapAngle  = 34.0f * std::sin(wingCycle);
        float tailSway   = 2.5f * std::sin(wingCycle * 0.6f);

        // Supersonic Cyan Raptor Palette
        Color colBase     = (flash > 0) ? Color(1.0f,1.0f,1.0f) : Color(0.24f, 0.76f, 0.94f);
        Color colShadow   = (flash > 0) ? Color(1.0f,1.0f,1.0f) : Color(0.12f, 0.54f, 0.72f);
        Color colHighlight= (flash > 0) ? Color(1.0f,1.0f,1.0f) : Color(0.65f, 0.92f, 1.00f);
        Color colComb     = (flash > 0) ? Color(1.0f,0.8f,0.8f) : Color(0.20f, 0.40f, 0.92f);
        Color colBeak     = Color(1.0f, 0.75f, 0.10f);
        Color colBeakDark = Color(0.90f, 0.52f, 0.05f);

        float by = y + flightBob;

        // Motion trails / afterburner exhaust
        glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        for(int i = 1; i <= 3; i++){
            float ty    = by - i*9.0f - 16.0f;
            float alpha = 0.40f - i*0.11f;
            float r     = 3.8f - i*0.7f;
            drawCircle(x, ty, r, Color(0.20f,0.70f,1.0f,alpha));
        }
        glDisable(GL_BLEND);

        // Tail rudders (swept back)
        std::vector<Vec2> tailR = {{x-4, by-8}, {x+4, by-8}, {x+tailSway, by-24}};
        scanlineFill(tailR, colShadow);

        // Body — sleek aerodynamic profile
        std::vector<Vec2> body = {
            {x, by+21}, {x-12, by+12}, {x-16, by-2}, {x-8, by-14},
            {x+8, by-14}, {x+16, by-2}, {x+12, by+12}
        };
        scanlineFill(body, colBase);

        // Aerodynamic dorsal speed stripe
        std::vector<Vec2> stripe = {
            {x-4, by+18}, {x+4, by+18}, {x+2, by-6}, {x-2, by-6}
        };
        scanlineFill(stripe, colHighlight);

        // ── Swept-back flight wings ──────────────────────────────────────────
        glPushMatrix();
        glTranslatef(x-15, by+1, 0);
        glRotatef(-flapAngle, 0, 0, 1);
        std::vector<Vec2> lw = {{0,0}, {-18,-3}, {-25,12}, {-10,16}, {0,8}};
        scanlineFill(lw, colBase);
        ddaLine(-18, -3, -25, 12, colHighlight);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(x+15, by+1, 0);
        glRotatef(flapAngle, 0, 0, 1);
        std::vector<Vec2> rw = {{0,0}, {18,-3}, {24,12}, {10,16}, {0,8}};
        scanlineFill(rw, colBase);
        ddaLine(18, -3, 24, 12, colHighlight);
        glPopMatrix();

        // ── Head & Visor ─────────────────────────────────────────────────────
        float hy = by + 27;
        drawCircle(x, hy, 12.5f, colBase);
        drawCircle(x, hy+3, 7, colHighlight);

        // Comb — sleek crest
        drawCombBump(x,    hy+12, 4.5f, colComb);
        drawCombBump(x-4,  hy+10, 3.2f, colComb);

        // Eyes — keen flight eyes
        drawCircle(x-5, hy+2, 4.0f, Color(1,1,1));
        midpointCircle((int)(x-5), (int)(hy+2), 3, Color(0.02f,0.15f,0.35f), true);
        midpointCircle((int)(x-4), (int)(hy+3), 1, Color(1,1,1), true);

        drawCircle(x+5, hy+2, 4.0f, Color(1,1,1));
        midpointCircle((int)(x+5), (int)(hy+2), 3, Color(0.02f,0.15f,0.35f), true);
        midpointCircle((int)(x+6), (int)(hy+3), 1, Color(1,1,1), true);

        // Sharp aerobeak
        std::vector<Vec2> beak = {{x-4, hy-2}, {x+4, hy-2}, {x, hy-9}};
        scanlineFill(beak, colBeak);
        ddaLine((int)(x-4), (int)(hy-4), (int)(x+4), (int)(hy-4), colBeakDark);
    }

    void drawArmoredChicken(float flash) const {
        // ── Heavy Flight Motion: heavy wing stroke ───────────────────────────
        float wingCycle  = animTime * 4.5f;
        float flightBob  = 1.6f * std::sin(wingCycle);
        float flapAngle  = 20.0f * std::sin(wingCycle);

        // Steel-Knight Palette
        Color colPlate     = (flash > 0) ? Color(1.0f,1.0f,1.0f) : Color(0.44f, 0.48f, 0.54f);
        Color colPlateDark = (flash > 0) ? Color(1.0f,1.0f,1.0f) : Color(0.32f, 0.35f, 0.40f);
        Color colPlateLight= (flash > 0) ? Color(1.0f,1.0f,1.0f) : Color(0.70f, 0.75f, 0.82f);
        Color colRivet     = (flash > 0) ? Color(1.0f,1.0f,1.0f) : Color(0.88f, 0.90f, 0.94f);
        Color colComb      = (flash > 0) ? Color(1.0f,0.8f,0.8f) : Color(0.85f, 0.12f, 0.14f);
        Color colVisorGlow = Color(0.0f, 0.92f, 0.35f);

        float by = y + flightBob;

        // ── 1. Avian Tail & Underbelly (Organic chicken base) ────────────────
        std::vector<Vec2> tail = {{x-5, by-8}, {x+5, by-8}, {x, by-22}};
        scanlineFill(tail, colPlateDark);

        // ── 2. Heavy Steel Breastplate (Anatomically sculpted) ───────────────
        std::vector<Vec2> plate = {
            {x, by+20}, {x-16, by+12}, {x-20, by-4}, {x-12, by-14},
            {x+12, by-14}, {x+20, by-4}, {x+16, by+12}
        };
        scanlineFill(plate, colPlate);

        // Center ridge reinforcement
        std::vector<Vec2> ridge = {
            {x-3, by+18}, {x+3, by+18}, {x+4, by-10}, {x-4, by-10}
        };
        scanlineFill(ridge, colPlateLight);

        // Steel rivets
        midpointCircle((int)(x-13), (int)(by+6),  2, colRivet, true);
        midpointCircle((int)(x+13), (int)(by+6),  2, colRivet, true);
        midpointCircle((int)(x-8),  (int)(by-9),  2, colRivet, true);
        midpointCircle((int)(x+8),  (int)(by-9),  2, colRivet, true);

        // ── 3. Armored Wings (Articulated shoulder pauldrons) ────────────────
        glPushMatrix();
        glTranslatef(x-18, by+1, 0);
        glRotatef(-flapAngle, 0, 0, 1);
        std::vector<Vec2> lw = {{0,0}, {-14,-4}, {-22,6}, {-12,16}, {0,12}};
        scanlineFill(lw, colPlate);
        ddaLine(-14, -4, -22, 6, colPlateLight);
        midpointCircle(-10, 4, 2, colRivet, true);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(x+18, by+1, 0);
        glRotatef(flapAngle, 0, 0, 1);
        std::vector<Vec2> rw = {{0,0}, {14,-4}, {22,6}, {12,16}, {0,12}};
        scanlineFill(rw, colPlate);
        ddaLine(14, -4, 22, 6, colPlateLight);
        midpointCircle(10, 4, 2, colRivet, true);
        glPopMatrix();

        // ── 4. Helmet, Comb & Knight Visor ───────────────────────────────────
        float hy = by + 28;
        drawCircle(x, hy, 14, colPlate);
        drawCircle(x, hy+3, 8, colPlateLight);

        // Rooster comb protruding proudly from helmet crest
        drawCombBump(x,    hy+13, 5.0f, colComb);
        drawCombBump(x-5,  hy+11, 3.5f, colComb);
        drawCombBump(x+5,  hy+11, 3.5f, colComb);

        // Visor slit with pulsing LED optical sensor
        std::vector<Vec2> visor = {
            {x-11, hy+4}, {x+11, hy+4}, {x+10, hy-1}, {x-10, hy-1}
        };
        scanlineFill(visor, colPlateDark);
        ddaLine((int)(x-8), (int)(hy+1), (int)(x+8), (int)(hy+1), colVisorGlow);
        midpointCircle((int)(x-4), (int)(hy+1), 2, colVisorGlow, true);
        midpointCircle((int)(x+4), (int)(hy+1), 2, colVisorGlow, true);

        // Armored Beak
        std::vector<Vec2> beak = {{x-4, hy-2}, {x+4, hy-2}, {x, hy-10}};
        scanlineFill(beak, Color(0.95f, 0.55f, 0.0f));
        ddaLine((int)(x-4), (int)(hy-5), (int)(x+4), (int)(hy-5), Color(0.70f, 0.38f, 0.0f));

        // NOTE: Red target / hit box completely removed!
        // When hit, the entire chicken sprite flashes bright white (via flash > 0)
    }

    // HP bar — display when damaged or armored for clean arcade presentation
    void drawHpBar() const {
        if(hp >= maxHp && type != EnemyType::ARMORED) return;
        float barW=32, barH=3;
        drawRect(x-barW/2, y-27, barW, barH, Color(0.18f,0.20f,0.25f,0.85f));
        float ratio=(float)hp/maxHp;
        Color barCol = (ratio>0.5f)?Color(0.15f,0.92f,0.30f):(ratio>0.25f)?Color(1.0f,0.70f,0.15f):Color(0.95f,0.15f,0.15f);
        drawRect(x-barW/2, y-27, barW*ratio, barH, barCol);
    }

    void draw() const {
        if(!active) return;
        // Don't render while waiting off-screen
        if(y > WIN_H + 35.0f || x < -55.0f || x > WIN_W + 55.0f) return;

        float flash = (hitFlash > 0) ? 1.0f : 0.0f;
        switch(type){
            case EnemyType::NORMAL:  drawNormalChicken(flash);  break;
            case EnemyType::FAST:    drawFastChicken(flash);    break;
            case EnemyType::ARMORED: drawArmoredChicken(flash); break;
        }
        drawHpBar();
        for(auto& e : eggs) e.draw();
    }

    void takeDamage(int dmg){
        hp -= dmg;
        hitFlash = 8;
        if(hp <= 0){ active=false; }
    }

    // AABB unchanged — gameplay collision boxes are identical to original
    AABB getAABB() const {
        if(y > WIN_H + 20.0f || x < -45.0f || x > WIN_W + 45.0f)
            return {-9999, -9999, 0, 0};
        return {x-22, y-18, 44, 55};
    }

    // Returns coins/food spawn chance based on type
    int getCoinValue() const {
        switch(type){
            case EnemyType::NORMAL:  return 10;
            case EnemyType::FAST:    return 20;
            case EnemyType::ARMORED: return 40;
        }
        return 10;
    }
};
