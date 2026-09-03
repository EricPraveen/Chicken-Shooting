// =============================================================================
// Game.h — Central Game Manager
// =============================================================================
// CG Concepts Orchestrated Here:
//   Double Buffering (13)  — glutSwapBuffers called in display()
//   All entity updates     — collision, spawning, scoring
// =============================================================================

#pragma once
#include "Utils.h"
#include "Player.h"
#include "Enemy.h"
#include "Boss.h"
#include "PowerUp.h"
#include "Coin.h"
#include "Food.h"
#include <vector>
#include <string>
#include <ctime>

// ---------------------------------------------------------------------------
// Game State Machine
// ---------------------------------------------------------------------------
enum class GameState { MENU, PLAYING, PAUSED, GAME_OVER, WIN };

// ---------------------------------------------------------------------------
// Starfield particle for background
// ---------------------------------------------------------------------------
struct Star {
    float x, y, speed, brightness, size, animPhase;
};

// ---------------------------------------------------------------------------
// Explosion particle
// ---------------------------------------------------------------------------
struct Particle {
    float x, y, vx, vy, life, maxLife, size;
    Color color;
    bool active;
    Particle(float x,float y,float vx,float vy,float life,Color c,float sz=3)
        : x(x),y(y),vx(vx),vy(vy),life(life),maxLife(life),size(sz),color(c),active(true){}
};

// ---------------------------------------------------------------------------
// Game class
// ---------------------------------------------------------------------------
class Game {
public:
    GameState   state;
    int         level;
    float       globalTime;

    Player      player;
    std::vector<Enemy>   enemies;
    std::vector<PowerUp> powerups;
    std::vector<Coin>    coins;
    std::vector<Food>    foods;
    std::vector<Star>    stars;
    std::vector<Particle> particles;

    Boss*       boss;
    bool        bossSpawned;

    // Spawn timers
    int enemySpawnTimer;
    int enemySpawnRate;
    int powerupTimer;
    int coinTimer;

    // Background scroll
    float bgScroll;

    // UI message
    std::string uiMessage;
    int         uiMessageTimer;

    Game();
    ~Game();

    void init();
    void reset();
    void update();
    void display();

    void spawnWave();
    void spawnExplosion(float x, float y, Color c, int count=20);
    void spawnBoss();

    void handleBulletCollisions();
    void handleEggCollisions();
    void handlePickups();

    void drawBackground();
    void drawHUD();
    void drawMenu();
    void drawPauseScreen();
    void drawGameOver();
    void drawWinScreen();
    void drawStarfield();

#ifdef __EMSCRIPTEN__
    void drawText(float x, float y, const std::string& s, Color c={1,1,1},
                  void* font=nullptr);
#else
    void drawText(float x, float y, const std::string& s, Color c={1,1,1},
                  void* font=GLUT_BITMAP_HELVETICA_18);
#endif
    void drawTextLarge(float x, float y, const std::string& s, Color c={1,1,1});

    // Keyboard handlers
    void onKeyDown(unsigned char key);
    void onKeyUp(unsigned char key);
    void onSpecialDown(int key);
    void onSpecialUp(int key);
    void onKeyPress(unsigned char key); // menu/pause single-press

    void nextLevel();
    bool isComplete() const; // all enemies dead + boss dead
};

// Global game instance (accessed from GLUT callbacks)
extern Game* g_game;
