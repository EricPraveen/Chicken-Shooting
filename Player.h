// =============================================================================
// Player.h — Player Spaceship (UPDATED: Lives system + Improved animations)
// =============================================================================
// CG Concepts:
//   Scan-Line Fill    — ship body polygon
//   Midpoint Circle   — cockpit window + shield bubble
//   DDA Line          — wing accents
//   Bresenham Line    — (moved to engine flame triangles)
//   Translation       — WASD movement
// =============================================================================

#pragma once
#include "Utils.h"
#include "Bullet.h"
#include <vector>

struct Player {
    float x, y;
    float speed;
    int   hp, maxHp;
    int   score;
    int   coins;

    // ── LIVES SYSTEM ─────────────────────────────────────────────────────────
    int  lives, maxLives;
    static const int foodForLife = 5;   // food items needed to earn 1 extra life
    int  foodCollected;                 // food eaten toward next life
    int  respawnTimer;                  // invincibility frames after losing a life

    // Power-up states
    bool  shieldActive;
    int   shieldTimer;
    bool  strongBulletActive;
    int   strongBulletTimer;
    bool  fireRateActive;
    int   fireRateTimer;

    int   shootCooldown;
    int   shootTimer;

    // Animation timers
    float engineFlicker;   // drives flame/exhaust animation
    float engineAccel;     // ramps up when moving

    std::vector<Bullet> bullets;

    bool moveLeft, moveRight, moveUp, moveDown;
    bool isShooting;
    bool shotFired;

    Player(float startX=WIN_W/2.0f, float startY=80.0f)
        : x(startX), y(startY), speed(5.0f),
          hp(100), maxHp(100), score(0), coins(0),
          lives(3), maxLives(3), foodCollected(0), respawnTimer(0),
          shieldActive(false), shieldTimer(0),
          strongBulletActive(false), strongBulletTimer(0),
          fireRateActive(false), fireRateTimer(0),
          shootCooldown(8), shootTimer(8),
          engineFlicker(0), engineAccel(0),
          moveLeft(false), moveRight(false), moveUp(false), moveDown(false),
          isShooting(false), shotFired(false)
    {}

    void activateShield(int dur=600)       { shieldActive=true; shieldTimer=dur; }
    void activateStrongBullet(int dur=400) { strongBulletActive=true; strongBulletTimer=dur; }
    void activateFireRate(int dur=500)     { fireRateActive=true; fireRateTimer=dur; shootCooldown=3; }

    void update(){
        shotFired = false;
        // Invincibility countdown after losing a life
        if(respawnTimer > 0) respawnTimer--;

        // CG Concept 7a: Translation — move ship
        if(moveLeft  && x > 30)        x -= speed;
        if(moveRight && x < WIN_W-30)  x += speed;
        if(moveUp    && y < WIN_H-50)  y += speed;
        if(moveDown  && y > 30)        y -= speed;

        // Engine thrust ramps when moving, idles at rest
        bool moving = moveLeft||moveRight||moveUp||moveDown;
        float accelTarget = moving ? 1.0f : 0.4f;
        engineAccel += (accelTarget - engineAccel) * 0.08f;

        // Power-up countdowns
        if(shieldActive       && --shieldTimer<=0)       { shieldActive=false; }
        if(strongBulletActive && --strongBulletTimer<=0) { strongBulletActive=false; }
        if(fireRateActive     && --fireRateTimer<=0)     { fireRateActive=false; shootCooldown=8; }

        // Controlled shooting: ONLY fires when Spacebar is held down
        if(isShooting){
            if(++shootTimer >= shootCooldown){
                shootTimer = 0;
                shotFired = true;
                bool s = strongBulletActive;
                if(s){
                    bullets.emplace_back(x-6, y+30, true);
                    bullets.emplace_back(x+6, y+30, true);
                } else {
                    bullets.emplace_back(x, y+30);
                }
            }
        } else {
            // Ready to fire immediately on next press
            shootTimer = shootCooldown;
        }

        for(auto& b : bullets) b.update();
        bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
            [](const Bullet& b){ return !b.active; }), bullets.end());

        engineFlicker += 0.18f;
    }

    // ── IMPROVED ENGINE EXHAUST ANIMATION ────────────────────────────────────
    // Uses animated triangle "flame" polygons that flicker in size and lean
    // left/right for a realistic thruster fire effect.
    void drawEngineFlames() const {
        float t  = engineFlicker;
        float boost = (fireRateActive ? 1.55f : 1.0f) * engineAccel;

        // Two independent flicker frequencies per thruster
        float f1 = 0.60f + 0.40f * std::sin(t * 2.3f);
        float f2 = 0.60f + 0.40f * std::sin(t * 1.8f + 1.2f);

        // ── Left thruster ────────────────────────────────────────────────────
        {
            float cx = x - 8, base = y - 20;
            float len = (24 + 16*f1) * boost;
            float lean = 4.0f * std::sin(t * 1.5f); // side wobble

            // Outer flame (wide, cooler blue-white)
            std::vector<Vec2> outer = {
                {cx-6, base}, {cx+6, base},
                {cx+lean+2.5f, base-len*0.55f},
                {cx+lean,      base-len}
            };
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            scanlineFill(outer, Color(0.35f, 0.50f+0.25f*f1, 1.0f, 0.75f));

            // Core flame (narrow, hot white)
            std::vector<Vec2> core = {
                {cx-2, base}, {cx+2, base},
                {cx+lean, base - len*0.65f}
            };
            scanlineFill(core, Color(0.92f, 0.96f, 1.0f, 0.92f));
            glDisable(GL_BLEND);

            // Nozzle glow circle
            drawCircle(cx, base, 5.5f*f1, Color(0.5f, 0.65f, 1.0f, 0.35f*f1));
            // Outer glow halo
            drawCircle(cx, base, 9.0f*f1, Color(0.3f, 0.4f, 1.0f, 0.12f*f1));
        }

        // ── Right thruster ───────────────────────────────────────────────────
        {
            float cx = x + 8, base = y - 20;
            float len = (24 + 16*f2) * boost;
            float lean = 4.0f * std::sin(t * 1.5f + 0.8f);

            std::vector<Vec2> outer = {
                {cx-6, base}, {cx+6, base},
                {cx+lean+2.5f, base-len*0.55f},
                {cx+lean,      base-len}
            };
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            scanlineFill(outer, Color(0.35f, 0.50f+0.25f*f2, 1.0f, 0.75f));

            std::vector<Vec2> core = {
                {cx-2, base}, {cx+2, base},
                {cx+lean, base - len*0.65f}
            };
            scanlineFill(core, Color(0.92f, 0.96f, 1.0f, 0.92f));
            glDisable(GL_BLEND);

            drawCircle(cx, base, 5.5f*f2, Color(0.5f, 0.65f, 1.0f, 0.35f*f2));
            drawCircle(cx, base, 9.0f*f2, Color(0.3f, 0.4f, 1.0f, 0.12f*f2));
        }
    }

    void drawShip() const {
        // Draw engine flames behind the ship body
        drawEngineFlames();

        // ── Main body — Scan-Line Fill polygon (CG Concept 5) ────────────────
        Color bodyCol = shieldActive ? Color(0.28f,0.50f,1.0f) : Color(0.20f,0.40f,0.90f);
        std::vector<Vec2> body = {
            {x,      y+36}, {x-18, y+10}, {x-22, y-10},
            {x-8,    y-21}, {x+8,  y-21}, {x+22, y-10}, {x+18, y+10}
        };
        scanlineFill(body, bodyCol);

        // Wing fills (darker shade)
        std::vector<Vec2> lwing = {{x-18, y+10},{x-30, y+4},{x-22, y-10}};
        std::vector<Vec2> rwing = {{x+18, y+10},{x+30, y+4},{x+22, y-10}};
        scanlineFill(lwing, Color(0.14f,0.33f,0.78f));
        scanlineFill(rwing, Color(0.14f,0.33f,0.78f));

        // Wing edge highlights — DDA Line (CG Concept 2)
        ddaLine((int)(x-22),(int)(y-10),(int)(x-30),(int)(y+4), Color(0.30f,0.55f,1.0f));
        ddaLine((int)(x+22),(int)(y-10),(int)(x+30),(int)(y+4), Color(0.30f,0.55f,1.0f));

        // Body centre stripe (DDA)
        ddaLine((int)x,(int)(y+36),(int)x,(int)(y-21), Color(0.40f,0.65f,1.0f,0.45f));

        // ── Cockpit window — Midpoint Circle (CG Concept 4) ──────────────────
        float ct = engineFlicker * 0.35f;
        Color cockpit(0.50f+0.12f*std::sin(ct), 0.85f+0.10f*std::cos(ct), 1.0f, 0.88f);
        drawCircle(x, y+15, 10, cockpit);
        midpointCircle((int)x,(int)(y+15), 10, Color(0.40f,0.75f,1.0f), false);
        // Shine dot
        drawCircle(x-3, y+18, 3, Color(1,1,1,0.60f));

        // Thruster nozzle rings
        drawRectOutline(x-12, y-22, 8, 4, Color(0.55f,0.65f,0.90f));
        drawRectOutline(x+4,  y-22, 8, 4, Color(0.55f,0.65f,0.90f));

        // Cannon barrel
        drawRect(x-3, y+30, 6, 14, Color(0.65f,0.65f,0.85f));
        drawRect(x-2, y+41, 4, 4,  Color(0.82f,0.82f,1.00f));

        // Side accent stripes
        ddaLine((int)(x-15),(int)(y+6),(int)(x-20),(int)(y-9), Color(0.42f,0.70f,1.0f,0.55f));
        ddaLine((int)(x+15),(int)(y+6),(int)(x+20),(int)(y-9), Color(0.42f,0.70f,1.0f,0.55f));
    }

    void drawShield() const {
        if(!shieldActive) return;
        float t = engineFlicker;

        // CG Concept 4: Midpoint Circle Algorithm — shield bubble
        float pulse = 0.20f + 0.12f * std::sin(t * 3.5f);
        float r     = 42.0f + 1.6f * std::sin(t * 2.8f);

        // 1. Soft inner energy plasma fill
        glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        midpointCircle((int)x, (int)y, (int)r, Color(0.12f, 0.65f, 1.0f, pulse), true);
        midpointCircle((int)x, (int)y, (int)(r - 5), Color(0.35f, 0.85f, 1.0f, pulse * 0.7f), true);

        // 2. Hexagonal energy grid lattice (CG Concept 2: DDA Line)
        float hexRot = -t * 0.6f;
        int sides = 6;
        for(int i = 0; i < sides; i++){
            float a1 = hexRot + i * (2.0f * PI / sides);
            float a2 = hexRot + (i + 1) * (2.0f * PI / sides);
            float px1 = x + (r - 6) * std::cos(a1), py1 = y + (r - 6) * std::sin(a1);
            float px2 = x + (r - 6) * std::cos(a2), py2 = y + (r - 6) * std::sin(a2);
            ddaLine((int)px1, (int)py1, (int)px2, (int)py2, Color(0.55f, 0.90f, 1.0f, 0.45f));
            ddaLine((int)x, (int)y, (int)px1, (int)py1, Color(0.35f, 0.75f, 1.0f, 0.20f));
        }

        // 3. Crisp outer energy perimeter & refraction rings
        midpointCircle((int)x, (int)y, (int)r, Color(0.40f, 0.92f, 1.0f, 0.95f), false);
        midpointCircle((int)x, (int)y, (int)(r - 1), Color(0.70f, 0.98f, 1.0f, 0.70f), false);

        // 4. CG Concept 8: 4 Orbiting Aegis Energy Nodes
        float arcAngle = t * 2.6f;
        for(int i = 0; i < 4; i++){
            float a = arcAngle + i * (PI / 2.0f);
            float ox = x + r * std::cos(a);
            float oy = y + r * std::sin(a);

            drawCircle(ox, oy, 3.5f, Color(0.20f, 0.85f, 1.0f, 0.90f));
            midpointCircle((int)ox, (int)oy, 2, Color(1.0f, 1.0f, 1.0f), true);

            float a2 = a + 0.38f;
            float ax2 = x + r * std::cos(a2);
            float ay2 = y + r * std::sin(a2);
            ddaLine((int)ox, (int)oy, (int)ax2, (int)ay2, Color(0.85f, 0.98f, 1.0f, 0.80f));
        }
        glDisable(GL_BLEND);
    }

    void draw() const {
        // Always draw active bullets
        for(auto& b : bullets) b.draw();

        // CG: During respawn, ship blinks (invincibility visual feedback)
        if(respawnTimer > 0){
            // Every 5 frames, alternate between ghost-ship and invisible
            if((respawnTimer / 5) % 2 == 1){
                // Ghost outline only
                std::vector<Vec2> ghost={
                    {x,y+36},{x-18,y+10},{x-22,y-10},
                    {x-8,y-21},{x+8,y-21},{x+22,y-10},{x+18,y+10}
                };
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
                scanlineFill(ghost, Color(0.5f,0.7f,1.0f,0.20f));
                glDisable(GL_BLEND);
            } else {
                drawShip();
                drawShield();
            }
            return;
        }

        drawShip();
        drawShield();
    }

    AABB getAABB() const {
        if(shieldActive)     return {x-42, y-42, 84, 84};
        if(respawnTimer > 0) return {x,    y,    0,  0};  // no-hit during blink
        return {x-20, y-20, 40, 40};
    }

    void takeDamage(int dmg){
        if(shieldActive)     return; // shield absorbs
        if(respawnTimer > 0) return; // invincible while blinking
        hp -= dmg;
        if(hp < 0) hp = 0;
    }

    bool isDead() const { return lives == 0 && hp <= 0; }
};
