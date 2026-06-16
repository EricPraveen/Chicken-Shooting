// =============================================================================
// Food.h — Food drops from killed enemies (health restore items)
// =============================================================================
// CG Concepts:
//   Scan-Line Fill — food shapes filled via polygon scan-line algorithm
//   Translation    — food falls downward
// =============================================================================

#pragma once
#include "Utils.h"

enum class FoodType { DRUMSTICK, PIZZA, CAKE };

struct Food {
    float x, y;
    float speed;
    bool  active;
    FoodType type;
    int   healAmount;
    float animTime;

    Food(float x, float y, FoodType t=FoodType::DRUMSTICK)
        : x(x), y(y), speed(2.5f), active(true), type(t),
          healAmount(20), animTime(0) {}

    void update(){
        y -= speed;
        animTime += 0.08f;
        if(y < -30) active=false;
    }

    void drawDrumstick() const {
        // Bone — DDA lines
        ddaLine((int)x-12,(int)y-5, (int)x+8,(int)y+12, Color(0.9f,0.8f,0.6f));
        // Meat chunk — scanline filled diamond
        std::vector<Vec2> meat = {
            {x-5,y+14},{x+6,y+14},{x+10,y+5},{x+8,y-4},
            {x,y-8},{x-8,y-4},{x-10,y+5}
        };
        scanlineFill(meat, Color(0.8f,0.3f,0.1f));
        // Bone ends
        midpointCircle((int)x-12,(int)y-5,5, Color(0.95f,0.9f,0.75f), true);
        midpointCircle((int)x+8,(int)y+12,5, Color(0.95f,0.9f,0.75f), true);
    }

    void drawPizza() const {
        // Pizza triangle slice — scanline
        std::vector<Vec2> slice = {
            {x,y+15},{x-14,y-10},{x+14,y-10}
        };
        scanlineFill(slice, Color(1.0f,0.65f,0.0f));
        // Crust
        ddaLine((int)(x-14),(int)(y-10),(int)(x+14),(int)(y-10), Color(0.7f,0.4f,0.1f));
        // Toppings — circles
        midpointCircle((int)x,(int)(y+2),3, Color(0.8f,0.1f,0.1f), true);
        midpointCircle((int)(x-5),(int)(y-4),3, Color(0.8f,0.1f,0.1f), true);
        midpointCircle((int)(x+5),(int)(y-4),3, Color(0.8f,0.1f,0.1f), true);
    }

    void drawCake() const {
        // Cake body
        drawRect(x-14, y-12, 28, 20, Color(1.0f,0.8f,0.8f));
        // Frosting on top
        drawRect(x-14, y+8, 28, 6, Color(1.0f,0.95f,0.95f));
        // Candle
        drawRect(x-1, y+14, 3, 8, Color(1.0f,0.9f,0.3f));
        drawCircle(x+0.5f, y+22, 3, Color(1.0f,0.5f,0.0f));
        // Green stripe
        ddaLine((int)(x-14),(int)(y-4),(int)(x+14),(int)(y-4), Color(0.2f,0.8f,0.2f));
    }

    void draw() const {
        if(!active) return;
        float bob = std::sin(animTime)*3.0f; // gentle bobbing
        // Apply translation offset via GL matrix
        glPushMatrix();
        glTranslatef(0, bob, 0);
        switch(type){
            case FoodType::DRUMSTICK: drawDrumstick(); break;
            case FoodType::PIZZA:     drawPizza();     break;
            case FoodType::CAKE:      drawCake();      break;
        }
        glPopMatrix();
    }

    AABB getAABB() const { return {x-14, y-14, 28, 28}; }
};
