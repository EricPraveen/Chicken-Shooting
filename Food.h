// =============================================================================
// Food.h — Food drops from killed enemies (health restore items)
// =============================================================================
// CG Concepts:
//   Scan-Line Fill — food shapes filled via polygon scan-line algorithm
//   Translation    — food falls downward and bobs dynamically
//   Rotation       — gentle aerodynamic sway tilt while falling
//   Midpoint Circle — warm golden aroma aura + steam particles
// =============================================================================

#pragma once
#include "Utils.h"

enum class FoodType { DRUMSTICK, CHICKEN_PIECE, CAKE };

struct Food {
    float x, y;
    float speed;
    bool  active;
    FoodType type;
    int   healAmount;
    float animTime;

    Food(float x, float y, FoodType t=FoodType::DRUMSTICK)
        : x(x), y(y), speed(2.5f), active(true), type(t),
          healAmount(20), animTime(randF(0.0f, 10.0f)) {}

    void update(){
        y -= speed;
        animTime += 0.08f;
        if(y < -30) active=false;
    }

    void drawSteamWisps(float sx, float sy) const {
        glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        for(int i = 0; i < 2; i++){
            float phase = animTime * 3.2f + i * PI;
            float steamY = sy + std::fmod(animTime * 15.0f + i * 11.0f, 24.0f);
            float steamX = sx + (i == 0 ? -4.0f : 4.0f) + 3.0f * std::sin(phase);
            float progress = (steamY - sy) / 24.0f;
            float steamAlpha = std::max(0.0f, 0.45f * (1.0f - progress));
            float steamR = 2.0f + progress * 2.5f;
            drawCircle(steamX, steamY, steamR, Color(1.0f, 1.0f, 0.95f, steamAlpha));
        }
        glDisable(GL_BLEND);
    }

    void drawDrumstick() const {
        // White bone shaft angled down-left
        ddaLine((int)(x - 14), (int)(y - 12), (int)(x + 4), (int)(y + 2), Color(0.96f, 0.94f, 0.88f));
        ddaLine((int)(x - 15), (int)(y - 11), (int)(x + 3), (int)(y + 3), Color(0.96f, 0.94f, 0.88f));
        // Dual cartilage bone knobs at lower tip
        drawCircle(x - 14, y - 13, 3.8f, Color(0.96f, 0.94f, 0.88f));
        drawCircle(x - 11, y - 16, 3.8f, Color(0.96f, 0.94f, 0.88f));
        midpointCircle((int)(x - 14), (int)(y - 13), 3, Color(0.78f, 0.74f, 0.65f), false);
        midpointCircle((int)(x - 11), (int)(y - 16), 3, Color(0.78f, 0.74f, 0.65f), false);

        // Plump roasted meat drumstick bulb
        std::vector<Vec2> drumMeat = {
            {x - 3, y - 3}, {x + 6, y - 8}, {x + 15, y - 2},
            {x + 16, y + 7}, {x + 10, y + 16}, {x + 1, y + 16},
            {x - 7, y + 9}, {x - 8, y + 1}
        };
        scanlineFill(drumMeat, Color(0.86f, 0.44f, 0.08f)); // golden crispy skin

        // Underbelly shadow for 3D roundness
        std::vector<Vec2> drumShadow = {
            {x - 3, y - 3}, {x + 6, y - 8}, {x + 15, y - 2},
            {x + 13, y + 3}, {x + 5, y - 2}
        };
        scanlineFill(drumShadow, Color(0.60f, 0.25f, 0.04f));

        // Savory golden glaze sheen on bulb top
        drawCircle(x + 6, y + 10, 5.0f, Color(1.00f, 0.72f, 0.20f));

        // Seasoning crack lines
        ddaLine((int)(x + 2), (int)(y + 12), (int)(x + 10), (int)(y + 6), Color(0.55f, 0.22f, 0.04f));
        drawCircle(x + 4, y + 5, 1.2f, Color(0.25f, 0.40f, 0.12f));

        // Savory rising steam wisps
        drawSteamWisps(x + 5, y + 15);
    }

    void drawChickenPiece() const {
        // Plump golden roasted chicken thigh / cutlet piece
        std::vector<Vec2> piece = {
            {x - 12, y + 6}, {x - 6, y + 14}, {x + 8, y + 15},
            {x + 15, y + 8}, {x + 16, y - 4}, {x + 10, y - 12},
            {x - 4, y - 14}, {x - 14, y - 8}, {x - 16, y - 1}
        };
        scanlineFill(piece, Color(0.86f, 0.44f, 0.08f)); // golden roast skin

        // Lower crispy roast shadow
        std::vector<Vec2> shadow = {
            {x - 14, y - 8}, {x - 4, y - 14}, {x + 10, y - 12},
            {x + 16, y - 4}, {x + 8, y - 6}, {x - 6, y - 8}
        };
        scanlineFill(shadow, Color(0.60f, 0.25f, 0.04f));

        // Golden caramelized glaze highlight on upper curve
        std::vector<Vec2> glaze = {
            {x - 6, y + 14}, {x + 8, y + 15}, {x + 13, y + 9},
            {x + 5, y + 7}, {x - 7, y + 6}
        };
        scanlineFill(glaze, Color(1.00f, 0.72f, 0.20f));

        // Small exposed bone nub at the top-left
        drawCircle(x - 14, y + 10, 3.5f, Color(0.96f, 0.94f, 0.86f));
        midpointCircle((int)(x - 14), (int)(y + 10), 3, Color(0.78f, 0.74f, 0.65f), false);

        // Roast seasoning specks & crispy grill marks
        ddaLine((int)(x - 4), (int)(y + 8), (int)(x + 4), (int)(y + 2), Color(0.55f, 0.22f, 0.04f));
        ddaLine((int)(x - 1), (int)(y + 3), (int)(x + 7), (int)(y - 3), Color(0.55f, 0.22f, 0.04f));
        drawCircle(x + 2, y + 10, 1.2f, Color(0.25f, 0.40f, 0.12f)); // rosemary herb speck
        drawCircle(x - 2, y - 2, 1.2f, Color(0.25f, 0.40f, 0.12f));

        // Savory rising steam wisps
        drawSteamWisps(x, y + 14);
    }

    void drawCake() const {
        // Retro arcade treat (layered snack with strawberry frosting)
        drawRect(x-14, y-12, 28, 20, Color(0.95f,0.75f,0.60f));
        drawRect(x-14, y+6, 28, 8, Color(1.0f,0.30f,0.50f));
        // Cherry on top
        drawCircle(x, y+18, 4.0f, Color(0.90f,0.08f,0.15f));
        drawCircle(x-1, y+19, 1.2f, Color(1.0f,1.0f,1.0f,0.8f));
        ddaLine((int)x, (int)(y+20), (int)(x+4), (int)(y+25), Color(0.20f,0.60f,0.15f));
        // Cream layer stripe
        ddaLine((int)(x-14),(int)(y-2),(int)(x+14),(int)(y-2), Color(1.0f,1.0f,1.0f));
    }

    void draw() const {
        if(!active) return;
        float bob = std::sin(animTime * 2.8f) * 3.5f; // gentle floating bob
        float tilt = 6.0f * std::sin(animTime * 2.2f); // gentle aerodynamic sway

        // Warm appetizing golden aroma aura (CG Concept 4)
        float auraPulse = 0.14f + 0.08f * std::sin(animTime * 3.0f);
        glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        midpointCircle((int)x, (int)(y + bob), 19, Color(1.0f, 0.65f, 0.15f, auraPulse), true);
        glDisable(GL_BLEND);

        // Apply translation offset & aerodynamic tilt via GL matrix
        glPushMatrix();
        glTranslatef(x, y + bob, 0);
        glRotatef(tilt, 0, 0, 1);
        glTranslatef(-x, -y, 0);

        switch(type){
            case FoodType::DRUMSTICK:     drawDrumstick();     break;
            case FoodType::CHICKEN_PIECE: drawChickenPiece(); break;
            case FoodType::CAKE:          drawCake();          break;
        }
        glPopMatrix();
    }

    AABB getAABB() const { return {x-14, y-14, 28, 28}; }
};
