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

    Enemy(float x, float y, EnemyType t=EnemyType::NORMAL)
        : x(x), y(y), baseX(x), speed(2.0f), descentSpeed(0.08f),
          active(true), type(t),
          animTime(0), wingFlap(0),
          hitFlash(0)
    {
        switch(t){
            case EnemyType::NORMAL:  hp=3;  maxHp=3;  speed=2.0f; eggCooldown=360; break;
            case EnemyType::FAST:    hp=2;  maxHp=2;  speed=4.0f; eggCooldown=240; break;
            case EnemyType::ARMORED: hp=6;  maxHp=6;  speed=1.5f; eggCooldown=480; break;
        }
        eggTimer = rand()%eggCooldown;
    }

    void update(){
        // CG Concept 14: Keyframe Animation — horizontal oscillation
        // The enemy's x position is driven by a sinusoidal keyframe curve.
        // Keyframes: amplitude=60px, period depends on speed
        animTime += 0.025f * speed;
        x = baseX + 60.0f * std::sin(animTime); // keyframe position interpolation

        // CG Concept 7a: Translation — gradual descent
        y -= descentSpeed;

        // CG Concept 14: Keyframe Animation — improved natural wing flap
        // Birds flap DOWN fast (power stroke) and UP slow (recovery stroke).
        // We model this with a signed formula: snap down sharply, ease back up.
        float rawFlap = std::sin(animTime * 5.0f);
        // Asymmetric: emphasise downstroke amplitude
        wingFlap = (rawFlap > 0)
            ? 32.0f * std::pow(rawFlap, 0.7f)   // snappy downstroke
            : 18.0f * rawFlap;                   // smooth upstroke recovery

        // Egg shooting
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

    void drawNormalChicken(float flash) const {
        // CG Concept 5: Scan-Line Fill — chicken body polygon
        std::vector<Vec2> body = {
            {x,      y+22}, {x-20, y+10}, {x-22, y-8},
            {x-10,   y-18}, {x+10, y-18}, {x+22, y-8}, {x+20, y+10}
        };
        Color bodyCol(flash>0?1.0f:0.85f, flash>0?0.5f:0.8f, flash>0?0.5f:0.3f);
        scanlineFill(body, bodyCol);

        // Head
        drawCircle(x, y+30, 14, Color(0.9f,0.85f,0.3f));

        // Eyes — midpoint circles
        midpointCircle((int)(x-5),(int)(y+33),3, Color(0.1f,0.1f,0.1f), true);
        midpointCircle((int)(x+5),(int)(y+33),3, Color(0.1f,0.1f,0.1f), true);
        // Eye shines
        midpointCircle((int)(x-4),(int)(y+34),1, Color(1,1,1), true);
        midpointCircle((int)(x+6),(int)(y+34),1, Color(1,1,1), true);

        // Beak — DDA line triangle
        std::vector<Vec2> beak = {{x-4,y+26},{x+4,y+26},{x,y+21}};
        scanlineFill(beak, Color(1.0f,0.55f,0.0f));

        // Comb — Bresenham lines
        bresenhamLine((int)x,(int)(y+44),(int)x,(int)(y+50), Color(0.9f,0.2f,0.2f));
        bresenhamLine((int)(x-4),(int)(y+43),(int)(x-6),(int)(y+48), Color(0.9f,0.2f,0.2f));
        bresenhamLine((int)(x+4),(int)(y+43),(int)(x+6),(int)(y+48), Color(0.9f,0.2f,0.2f));

        // Wings with flap rotation (CG Concept 8: Rotation)
        // Left wing
        glPushMatrix();
        glTranslatef(x-22, y, 0);
        glRotatef(-wingFlap, 0, 0, 1);
        std::vector<Vec2> lw = {{0,0},{-18,-5},{-21,10},{-6,18}};
        scanlineFill(lw, Color(0.95f,0.88f,0.35f));
        // Feather tip accent lines (DDA)
        ddaLine(-18,-5,-22,3, Color(0.85f,0.70f,0.20f));
        ddaLine(-14,-3,-18,6, Color(0.85f,0.70f,0.20f));
        ddaLine(-10,-1,-13,8, Color(0.85f,0.70f,0.20f));
        glPopMatrix();

        // Right wing
        glPushMatrix();
        glTranslatef(x+22, y, 0);
        glRotatef(wingFlap, 0, 0, 1);
        std::vector<Vec2> rw = {{0,0},{18,-5},{21,10},{6,18}};
        scanlineFill(rw, Color(0.95f,0.88f,0.35f));
        ddaLine(18,-5,22,3, Color(0.85f,0.70f,0.20f));
        ddaLine(14,-3,18,6, Color(0.85f,0.70f,0.20f));
        ddaLine(10,-1,13,8, Color(0.85f,0.70f,0.20f));
        glPopMatrix();

        // Feet
        ddaLine((int)(x-8),(int)(y-18),(int)(x-10),(int)(y-28), Color(1.0f,0.55f,0.0f));
        ddaLine((int)(x+8),(int)(y-18),(int)(x+10),(int)(y-28), Color(1.0f,0.55f,0.0f));
        ddaLine((int)(x-10),(int)(y-28),(int)(x-16),(int)(y-26), Color(1.0f,0.55f,0.0f));
        ddaLine((int)(x+10),(int)(y-28),(int)(x+16),(int)(y-26), Color(1.0f,0.55f,0.0f));
        // Toe tips
        ddaLine((int)(x-16),(int)(y-26),(int)(x-19),(int)(y-22), Color(1.0f,0.55f,0.0f));
        ddaLine((int)(x+16),(int)(y-26),(int)(x+19),(int)(y-22), Color(1.0f,0.55f,0.0f));
    }

    void drawFastChicken(float flash) const {
        // Speed lines — DDA
        ddaLine((int)(x-40),(int)y,(int)(x-25),(int)y, Color(0.5f,0.5f,1.0f,0.5f));
        ddaLine((int)(x-38),(int)(y+5),(int)(x-25),(int)(y+5), Color(0.5f,0.5f,1.0f,0.3f));

        std::vector<Vec2> body = {
            {x,y+20},{x-15,y+8},{x-18,y-6},{x-8,y-16},{x+8,y-16},{x+18,y-6},{x+15,y+8}
        };
        Color bodyCol(flash>0?1.0f:0.5f, flash>0?0.5f:0.7f, flash>0?0.5f:1.0f);
        scanlineFill(body, bodyCol);

        drawCircle(x, y+28, 12, Color(0.4f,0.6f,0.9f));
        midpointCircle((int)(x-4),(int)(y+31),3, Color(0.05f,0.05f,0.1f), true);
        midpointCircle((int)(x+4),(int)(y+31),3, Color(0.05f,0.05f,0.1f), true);

        std::vector<Vec2> beak2 = {{x-3,y+24},{x+3,y+24},{x,y+19}};
        scanlineFill(beak2, Color(0.8f,0.4f,0.0f));

        glPushMatrix();
        glTranslatef(x-18, y, 0);
        glRotatef(-wingFlap*1.6f, 0, 0, 1);
        std::vector<Vec2> lw={{0,0},{-16,-3},{-19,9},{-4,15}};
        scanlineFill(lw, Color(0.35f,0.52f,0.92f));
        // Feather accents
        ddaLine(-16,-3,-20,4, Color(0.25f,0.38f,0.78f));
        ddaLine(-11,-2,-14,6, Color(0.25f,0.38f,0.78f));
        glPopMatrix();

        glPushMatrix();
        glTranslatef(x+18, y, 0);
        glRotatef(wingFlap*1.6f, 0, 0, 1);
        std::vector<Vec2> rw={{0,0},{16,-3},{19,9},{4,15}};
        scanlineFill(rw, Color(0.35f,0.52f,0.92f));
        ddaLine(16,-3,20,4, Color(0.25f,0.38f,0.78f));
        ddaLine(11,-2,14,6, Color(0.25f,0.38f,0.78f));
        glPopMatrix();

        // Speed trail dots (CG 7a: translation)
        for(int i=1;i<=3;i++){
            float tx = x - i*10.0f - 30.0f;
            float alpha = 0.4f - i*0.12f;
            drawCircle(tx, y+2, 3.0f-i*0.5f, Color(0.5f,0.5f,1.0f,alpha));
        }
    }

    void drawArmoredChicken(float flash) const {
        // Armor plates — DDA outlined rectangles
        drawRect(x-20, y-12, 40, 30, Color(0.5f,0.5f,0.55f));
        drawRectOutline(x-20, y-12, 40, 30, Color(0.7f,0.7f,0.75f), 2.0f);

        // Rivets — midpoint circles
        midpointCircle((int)(x-14),(int)(y-6),3, Color(0.9f,0.9f,0.9f), true);
        midpointCircle((int)(x+14),(int)(y-6),3, Color(0.9f,0.9f,0.9f), true);
        midpointCircle((int)(x-14),(int)(y+12),3, Color(0.9f,0.9f,0.9f), true);
        midpointCircle((int)(x+14),(int)(y+12),3, Color(0.9f,0.9f,0.9f), true);

        // Head with helmet
        drawCircle(x, y+28, 14, Color(0.6f,0.6f,0.6f));
        drawRect(x-16, y+26, 32, 10, Color(0.5f,0.5f,0.55f)); // visor
        ddaLine((int)(x-16),(int)(y+26),(int)(x+16),(int)(y+26), Color(0.3f,0.8f,1.0f,0.7f));

        midpointCircle((int)(x-5),(int)(y+30),3, Color(0.2f,0.9f,0.2f), true);
        midpointCircle((int)(x+5),(int)(y+30),3, Color(0.2f,0.9f,0.2f), true);

        std::vector<Vec2> beak3={{x-4,y+22},{x+4,y+22},{x,y+17}};
        scanlineFill(beak3, Color(0.9f,0.5f,0.0f));

        glPushMatrix();
        glTranslatef(x-22, y, 0);
        glRotatef(-wingFlap*0.7f, 0, 0, 1);
        drawRect(-20, -5, 20, 20, Color(0.45f,0.45f,0.5f));
        glPopMatrix();

        glPushMatrix();
        glTranslatef(x+22, y, 0);
        glRotatef(wingFlap*0.7f, 0, 0, 1);
        drawRect(0, -5, 20, 20, Color(0.45f,0.45f,0.5f));
        glPopMatrix();

        if(flash > 0) {
            drawRect(x-22, y-15, 44, 48, Color(1,0.3f,0.3f,0.4f));
        }
    }

    // HP bar
    void drawHpBar() const {
        float barW=36, barH=4;
        drawRect(x-barW/2, y-28, barW, barH, Color(0.3f,0.3f,0.3f));
        float ratio=(float)hp/maxHp;
        Color barCol = (ratio>0.5f)?Color(0.2f,0.9f,0.2f):(ratio>0.25f)?Color(1,0.8f,0):Color(0.9f,0.1f,0.1f);
        drawRect(x-barW/2, y-28, barW*ratio, barH, barCol);
        drawRectOutline(x-barW/2, y-28, barW, barH, Color(0.6f,0.6f,0.6f));
    }

    void draw() const {
        if(!active) return;
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

    AABB getAABB() const { return {x-22, y-18, 44, 55}; }

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
