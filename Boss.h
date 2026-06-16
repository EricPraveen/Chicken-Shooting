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
          hitFlash(0)
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

        // Aura — Midpoint Circle (CG Concept 4)
        float auraA = 0.15f + 0.1f*std::sin(animTime*3);
        Color auraCol = (phase==2) ? Color(1,0.2f,0,auraA) : Color(0.5f,0,1,auraA);
        midpointCircle(0,0,72, auraCol, true);
        midpointCircle(0,0,65, Color(auraCol.r,auraCol.g,auraCol.b,auraA*1.5f), false);

        // Body polygon — Scan-Line Fill (CG Concept 5)
        std::vector<Vec2> body;
        for(int i=0;i<12;i++){
            float a  = 2*PI*i/12;
            float rx = 55*(0.9f+0.1f*std::sin(a*3));
            float ry = 50*(0.9f+0.1f*std::cos(a*2));
            body.push_back({rx*std::cos(a), ry*std::sin(a)});
        }
        Color bossCol = (phase==2)?Color(0.7f,0.1f,0.1f):Color(0.4f,0,0.6f);
        scanlineFill(body, bossCol);

        // Crown spikes
        for(int i=-2;i<=2;i++){
            float sx=i*18.0f;
            std::vector<Vec2> spike={{sx-8,35},{sx+8,35},{sx,50+std::abs(i)*5.0f}};
            scanlineFill(spike, Color(1,0.8f,0));
        }

        // Eyes (CG Concept 4)
        Color eyeCol=(phase==2)?Color(1,0.1f,0.1f):Color(0.2f,0.9f,1.0f);
        midpointCircle(-18,12,12,eyeCol,true); midpointCircle(18,12,12,eyeCol,true);
        midpointCircle(-18,12, 6,Color(1,1,1),true); midpointCircle(18,12,6,Color(1,1,1),true);
        midpointCircle(-16,14, 3,Color(0,0,0),true); midpointCircle(20,14,3,Color(0,0,0),true);

        // Beak
        std::vector<Vec2> beak={{-12,-5},{12,-5},{0,-22}};
        scanlineFill(beak, Color(1,0.6f,0));
        drawRectOutline(-30,-30,60,25, Color(0.8f,0.7f,0.9f),2.5f);
        ddaLine(-30,-18,30,-18, Color(0.7f,0.6f,0.8f));

        // Wings (CG Concept 8: Rotation)
        glPushMatrix(); glTranslatef(-55,0,0);
        glRotatef(20*std::sin(animTime*3),0,0,1);
        std::vector<Vec2> lw={{0,0},{-45,-10},{-50,25},{-20,40},{0,30}};
        scanlineFill(lw,(phase==2)?Color(0.6f,0,0):Color(0.35f,0,0.55f));
        glPopMatrix();

        glPushMatrix(); glTranslatef(55,0,0);
        glRotatef(-20*std::sin(animTime*3),0,0,1);
        std::vector<Vec2> rw={{0,0},{45,-10},{50,25},{20,40},{0,30}};
        scanlineFill(rw,(phase==2)?Color(0.6f,0,0):Color(0.35f,0,0.55f));
        glPopMatrix();

        if(hitFlash>0) drawRect(-55,-45,110,95,Color(1,0,0,0.3f));

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
