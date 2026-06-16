// =============================================================================
// PowerUp.h — Power-Up items (fire rate, shield, strong bullets)
// =============================================================================
// CG Concepts:
//   Rotation  — power-up icon spins (rotation matrix applied)
//   Scaling   — pulsing size animation
//   Midpoint Circle — shield icon ring
// =============================================================================

#pragma once
#include "Utils.h"

enum class PowerUpType { FIRE_RATE, SHIELD, STRONG_BULLET };

struct PowerUp {
    float x, y;
    float speed;
    bool  active;
    PowerUpType type;
    float angle;      // CG Concept 8: Rotation animation state
    float scaleAnim;  // CG Concept 9: Scale animation state
    int   duration;   // frames the power-up lasts when collected

    PowerUp(float x, float y, PowerUpType t)
        : x(x), y(y), speed(2.0f), active(true), type(t),
          angle(0), scaleAnim(0), duration(600) {}

    void update(){
        y -= speed;
        // CG Concept 8: Rotation — spin the icon
        angle += 2.0f;
        if(angle > 360.0f) angle -= 360.0f;
        scaleAnim += 0.07f;
        if(y < -30) active=false;
    }

    void draw() const {
        if(!active) return;
        // CG Concept 9: Scaling — pulsing
        float s = 1.0f + 0.2f*std::sin(scaleAnim);

        // CG Concept 8: Rotation + Homogeneous transform — spin the star
        glPushMatrix();
        glTranslatef(x, y, 0);
        glRotatef(angle, 0, 0, 1);  // OpenGL uses homogeneous matrix internally
        glScalef(s, s, 1);

        switch(type){
        case PowerUpType::FIRE_RATE: {
            // Lightning bolt — DDA lines
            Color col(1.0f,0.9f,0.0f);
            ddaLine(-5,15,0,0, col);
            ddaLine(0,0,5,0, col);
            ddaLine(5,0,-3,-15, col);
            ddaLine(-5,15,-3,-15, col);
            // filled polygon
            std::vector<Vec2> bolt={{-5,15},{0,0},{5,0},{-3,-15},{3,-5},{-2,0}};
            scanlineFill(bolt, Color(1,0.85f,0,0.9f));
            break;
        }
        case PowerUpType::SHIELD: {
            // CG Concept 4: Midpoint Circle — shield ring
            midpointCircle(0,0,16, Color(0.2f,0.6f,1.0f), true);
            midpointCircle(0,0,14, Color(0.05f,0.1f,0.3f,0.5f), true);
            // S text lines
            bresenhamLine(-5,10,5,10, Color(1,1,1));
            bresenhamLine(-5,10,-5,2, Color(1,1,1));
            bresenhamLine(-5,2,5,2, Color(1,1,1));
            bresenhamLine(5,2,5,-6, Color(1,1,1));
            bresenhamLine(-5,-6,5,-6, Color(1,1,1));
            break;
        }
        case PowerUpType::STRONG_BULLET: {
            // Star shape — scanline
            std::vector<Vec2> star;
            for(int i=0;i<10;i++){
                float a = PI/2 + i*2*PI/10;
                float r = (i%2==0)?16.0f:7.0f;
                star.push_back({r*std::cos(a), r*std::sin(a)});
            }
            scanlineFill(star, Color(1.0f,0.3f,0.0f,0.95f));
            break;
        }
        }
        glPopMatrix();

        // Outer glow ring
        Color glowCol;
        switch(type){
            case PowerUpType::FIRE_RATE:     glowCol=Color(1,1,0,0.5f); break;
            case PowerUpType::SHIELD:        glowCol=Color(0,0.5f,1,0.5f); break;
            case PowerUpType::STRONG_BULLET: glowCol=Color(1,0.3f,0,0.5f); break;
        }
        midpointCircle((int)x,(int)y,20,glowCol,false);
    }

    AABB getAABB() const { return {x-16, y-16, 32, 32}; }
};
