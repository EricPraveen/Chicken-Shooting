// =============================================================================
// Game.cpp — Game Manager Implementation
// =============================================================================

#include "Game.h"
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include "FontBitmap.h"
#undef GLUT_BITMAP_HELVETICA_12
#undef GLUT_BITMAP_HELVETICA_18
#undef GLUT_BITMAP_TIMES_ROMAN_24
#define GLUT_BITMAP_HELVETICA_12 nullptr
#define GLUT_BITMAP_HELVETICA_18 nullptr
#define GLUT_BITMAP_TIMES_ROMAN_24 nullptr
#endif

Game* g_game = nullptr;

#ifdef __EMSCRIPTEN__
extern "C" {
    EMSCRIPTEN_KEEPALIVE
    void wasm_restart_game() {
        if (g_game) {
            g_game->state = GameState::MENU;
            g_game->reset();
        }
    }

    EMSCRIPTEN_KEEPALIVE
    int wasm_get_score() {
        return g_game ? g_game->player.score : 0;
    }

    EMSCRIPTEN_KEEPALIVE
    void wasm_set_move_up(int active) {
        if (g_game) g_game->player.moveUp = (active != 0);
    }

    EMSCRIPTEN_KEEPALIVE
    void wasm_set_move_down(int active) {
        if (g_game) g_game->player.moveDown = (active != 0);
    }

    EMSCRIPTEN_KEEPALIVE
    void wasm_set_move_left(int active) {
        if (g_game) g_game->player.moveLeft = (active != 0);
    }

    EMSCRIPTEN_KEEPALIVE
    void wasm_set_move_right(int active) {
        if (g_game) g_game->player.moveRight = (active != 0);
    }

    EMSCRIPTEN_KEEPALIVE
    void wasm_set_shooting(int active) {
        if (g_game) g_game->player.isShooting = (active != 0);
    }

    EMSCRIPTEN_KEEPALIVE
    void wasm_press_start() {
        if (!g_game) return;
        if (g_game->state == GameState::MENU) {
            g_game->reset();
            g_game->state = GameState::PLAYING;
            g_game->playSfx("powerup");
            g_game->spawnWave();
        } else if (g_game->state == GameState::PLAYING) {
            g_game->state = GameState::PAUSED;
        } else if (g_game->state == GameState::PAUSED) {
            g_game->state = GameState::PLAYING;
        } else if (g_game->state == GameState::GAME_OVER || g_game->state == GameState::WIN) {
            g_game->reset();
            g_game->state = GameState::MENU;
        }
    }
}
#endif

// ---------------------------------------------------------------------------
// Constructor / Destructor
// ---------------------------------------------------------------------------
Game::Game()
    : state(GameState::MENU), level(1), globalTime(0),
      comboStreak(0), comboMultiplier(1), comboTimer(0),
      boss(nullptr), bossSpawned(false), waveSpawned(false),
      enemySpawnTimer(0), enemySpawnRate(120),
      powerupTimer(0), coinTimer(0),
      bgScroll(0), uiMessageTimer(0),
      notifiedEndGame(false),
      bossWarningActive(false), bossWarningTimer(0)
{
    srand((unsigned)time(nullptr));
    init();
}

Game::~Game() { delete boss; }

// ---------------------------------------------------------------------------
// init — set up stars and initial state
// ---------------------------------------------------------------------------
void Game::init(){
    stars.clear();
    for(int i=0;i<180;i++){
        Star s;
        s.x = randF(0, WIN_W);
        s.y = randF(0, WIN_H);
        s.speed = randF(0.3f, 2.0f);
        s.brightness = randF(0.3f, 1.0f);
        s.size = randF(0.5f, 2.5f);
        s.animPhase = randF(0.0f, 6.28f); // unique twinkle phase per star
        stars.push_back(s);
    }
}

// ---------------------------------------------------------------------------
// reset — restart game from level 1
// ---------------------------------------------------------------------------
void Game::reset(){
    player = Player();
    enemies.clear();
    powerups.clear();
    coins.clear();
    foods.clear();
    particles.clear();
    floatingTexts.clear();
    delete boss; boss=nullptr;
    bossSpawned=false;
    waveSpawned=false;
    bossWarningActive=false;
    bossWarningTimer=0;
    comboStreak=0;
    comboMultiplier=1;
    comboTimer=0;
    level=1;
    globalTime=0;
    enemySpawnTimer=0;
    enemySpawnRate=120;
    powerupTimer=0;
    coinTimer=0;
    bgScroll=0;
    uiMessage=""; uiMessageTimer=0;
    notifiedEndGame=false;
}

// ---------------------------------------------------------------------------
// checkEndGameNotification — notify frontend via Emscripten bridge
// ---------------------------------------------------------------------------
void Game::checkEndGameNotification(){
    if ((state == GameState::GAME_OVER || state == GameState::WIN) && !notifiedEndGame) {
        notifiedEndGame = true;
        int livesBonus = std::max(0, player.lives) * 200;
        int coinBonus = player.coins * 2;
        int baseCombatScore = player.score;
        int totalFinalScore = baseCombatScore + livesBonus + coinBonus;
#ifdef __EMSCRIPTEN__
        EM_ASM({
            if (window.onGameFinished) {
                window.onGameFinished($0, $1, $2, $3, $4, $5);
            }
        }, totalFinalScore, (state == GameState::WIN ? 1 : 0), player.coins, livesBonus, coinBonus, baseCombatScore);
#endif
    }
}

// ---------------------------------------------------------------------------
// playSfx — trigger chiptune audio via Web Audio API bridge
// ---------------------------------------------------------------------------
void Game::playSfx(const char* name){
#ifdef __EMSCRIPTEN__
    EM_ASM({
        if (window.playSfx) {
            window.playSfx(UTF8ToString($0));
        }
    }, name);
#endif
}

// ---------------------------------------------------------------------------
// Spawn a wave of enemies based on current level
// ---------------------------------------------------------------------------
void Game::spawnWave(){
    int rows = 2 + level;
    int cols = 5 + level;
    float startX = 80;
    float startY = WIN_H - 80;
    float spacingX = (WIN_W - 160.0f) / (cols-1);
    float spacingY = 60.0f;

    // Progressive entry pattern per level:
    // Level 1: Orderly Staggered Top-Down Descent (Row by row with V-wave)
    // Level 2: Dual-Wing Cross Swoop (Left & Right flanking crossing arcs)
    // Level 3: Spiral S-Curve Swarm (Continuous serpent wave)
    int pattern = std::min(level, 3);
    if(pattern < 1) pattern = 1;

    waveSpawned = true;
    for(int r=0;r<rows;r++){
        for(int c=0;c<cols;c++){
            EnemyType t;
            int roll = rand()%10;
            if(level>=3 && roll<2)       t=EnemyType::ARMORED;
            else if(level>=2 && roll<4)  t=EnemyType::FAST;
            else                          t=EnemyType::NORMAL;

            float ex = startX + c*spacingX;
            float ey = startY - r*spacingY;
            if(ey > WIN_H-40) ey=WIN_H-40;

            enemies.emplace_back(ex, ey, t);
            enemies.back().setupEntry(pattern, r, c, rows, cols, ex, ey);
        }
    }
}

// ---------------------------------------------------------------------------
// Spawn explosion particles
// ---------------------------------------------------------------------------
void Game::spawnExplosion(float x, float y, Color c, int count){
    if(particles.size() > 80) return; // Prevent particle flooding during multi-kills
    int pCount = std::min(count, 16);
    for(int i=0;i<pCount;i++){
        float angle = randF(0, 2*PI);
        float speed = randF(1.5f, 5.0f);
        float life  = randF(15, 35);
        float sz    = randF(2, 4.5f);
        particles.emplace_back(x,y,
            speed*std::cos(angle), speed*std::sin(angle),
            life, c, sz);
    }
    // Avian feather particle bursts
    int featherCount = std::min(count / 3 + 1, 5);
    for(int i=0;i<featherCount;i++){
        float angle = randF(0, 2*PI);
        float speed = randF(0.8f, 2.5f);
        float life  = randF(25, 45);
        float sz    = randF(2.5f, 4.5f);
        particles.emplace_back(x,y,
            speed*std::cos(angle), speed*std::sin(angle) + 0.8f,
            life, Color(0.96f, 0.94f, 0.88f), sz);
    }
}

// ---------------------------------------------------------------------------
// Floating score / popup text
// ---------------------------------------------------------------------------
void Game::spawnFloatingText(float x, float y, const std::string& txt, Color c, float scale){
    floatingTexts.emplace_back(x, y, txt, c, scale);
}

// ---------------------------------------------------------------------------
// Combo / Streak system
// ---------------------------------------------------------------------------
void Game::updateCombo(){
    comboStreak++;
    comboTimer = comboMaxTimer; // ~3.0 seconds
    int oldMult = comboMultiplier;
    if(comboStreak >= 20) comboMultiplier = 5;
    else if(comboStreak >= 15) comboMultiplier = 4;
    else if(comboStreak >= 10) comboMultiplier = 3;
    else if(comboStreak >= 5) comboMultiplier = 2;
    else comboMultiplier = 1;

    // Fanfare on tier level-up!
    if(comboMultiplier > oldMult){
        playSfx("powerup");
        if(comboMultiplier >= 5){
            uiMessage = "★ SUPER STREAK! x5 MULTIPLIER ★";
            uiMessageTimer = 120;
        } else {
            uiMessage = "COMBO x" + std::to_string(comboMultiplier) + "!";
            uiMessageTimer = 80;
        }
    }
}

void Game::resetCombo(bool showLostText){
    if(comboMultiplier > 1 && showLostText){
        spawnFloatingText(player.x, player.y + 25, "COMBO LOST!", Color(1.0f, 0.25f, 0.25f), 1.4f);
    }
    comboStreak = 0;
    comboMultiplier = 1;
    comboTimer = 0;
}

// ---------------------------------------------------------------------------
// Boss Warning and Spawn
// ---------------------------------------------------------------------------
void Game::triggerBossWarning(){
    bossWarningActive = true;
    bossWarningTimer = 160; // ~2.6 seconds
    uiMessage = "! WARNING ! BOSS DETECTED";
    uiMessageTimer = 160;
    playSfx("siren");
}

void Game::spawnBoss(){
    delete boss;
    boss = new Boss(WIN_W/2.0f, WIN_H-100.0f, level);
    bossSpawned=true;
    bossWarningActive=false;
    bossWarningTimer=0;
    uiMessage = "*** BOSS APPEARS! ***";
    uiMessageTimer=180;
    playSfx("bossroar");
}

// ---------------------------------------------------------------------------
// Bullet/Enemy collision detection (CG Concept 15: AABB)
// ---------------------------------------------------------------------------
void Game::handleBulletCollisions(){
    for(auto& b : player.bullets){
        if(!b.active) continue;
        AABB ba = b.getAABB();

        // vs enemies
        for(auto& e : enemies){
            if(!e.active) continue;
            if(ba.intersects(e.getAABB())){
                b.active=false;
                e.takeDamage(b.damage);
                if(!e.active){
                    playSfx("explosion");
                    updateCombo();
                    int earned = e.getCoinValue() * 2 * comboMultiplier;
                    player.score += earned;
                    std::string scoreTxt = "+" + std::to_string(earned);
                    if(comboMultiplier > 1) {
                        scoreTxt += " (x" + std::to_string(comboMultiplier) + ")";
                    }
                    Color txtCol = (comboMultiplier >= 5) ? Color(1.0f, 0.25f, 0.45f) :
                                   (comboMultiplier >= 3) ? Color(1.0f, 0.85f, 0.2f) :
                                                            Color(0.3f, 1.0f, 0.45f);
                    spawnFloatingText(e.x, e.y, scoreTxt, txtCol, comboMultiplier > 1 ? 1.45f : 1.25f);
                    spawnExplosion(e.x, e.y, Color(1,0.6f,0.1f));
                    // Drop coin
                    if(rand()%100 < 70)
                        coins.emplace_back(e.x, e.y+10, e.getCoinValue());
                    // Drop food
                    if(rand()%100 < 30){
                        FoodType ft=(FoodType)(rand()%3);
                        foods.emplace_back(e.x, e.y, ft);
                    }
                    // Drop power-up
                    if(rand()%100 < 15){
                        PowerUpType pt=(PowerUpType)(rand()%3);
                        powerups.emplace_back(e.x, e.y, pt);
                    }
                } else {
                    playSfx("hit");
                }
                break;
            }
        }

        // vs boss
        if(boss && boss->active && ba.intersects(boss->getAABB())){
            b.active=false;
            boss->takeDamage(b.damage);
            player.score += 5;
            spawnExplosion(b.x, b.y, Color(1,0.2f,0.8f), 8);
            if(!boss->active){
                playSfx("explosion");
                player.score += 500;
                spawnFloatingText(boss->x, boss->y + 30, "+500 BOSS!", Color(1.0f, 0.85f, 0.2f), 1.8f);
                spawnExplosion(boss->x, boss->y, Color(1,0.5f,0), 60);
                spawnExplosion(boss->x-30, boss->y+20, Color(1,0.8f,0), 40);
                spawnExplosion(boss->x+30, boss->y-20, Color(0.8f,0.2f,1), 40);
            } else {
                playSfx("hit");
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Egg/Player collision detection
// ---------------------------------------------------------------------------
void Game::handleEggCollisions(){
    AABB pa = player.getAABB();

    auto checkEgg=[&](Egg& eg){
        if(!eg.active) return;
        if(eg.getAABB().intersects(pa)){
            eg.active=false;
            player.takeDamage(10);
            resetCombo(true);
            playSfx("hurt");
            spawnExplosion(eg.x, eg.y, Color(0.9f,0.9f,0.6f), 10);
        }
    };

    for(auto& e : enemies)
        for(auto& eg : e.eggs) checkEgg(eg);

    if(boss && boss->active){
        for(auto& eg : boss->eggs) checkEgg(eg);
        // Boss laser damage
        if(boss->isPlayerInLaser(player.x, player.y)){
            player.takeDamage(2);
            resetCombo(true);
        }
    }
}

// ---------------------------------------------------------------------------
// Pickup collision
// ---------------------------------------------------------------------------
void Game::handlePickups(){
    AABB pa = player.getAABB();

    for(auto& c : coins){
        if(!c.active) continue;
        if(c.getAABB().intersects(pa)){
            c.active=false;
            player.coins += c.value;
            player.score += c.value;
            playSfx("coin");
            spawnFloatingText(c.x, c.y, "+" + std::to_string(c.value), Color(1.0f, 0.88f, 0.25f), 1.3f);
            spawnExplosion(c.x,c.y,Color(1,0.9f,0.1f),6);
        }
    }

    for(auto& f : foods){
        if(!f.active) continue;
        if(f.getAABB().intersects(pa)){
            f.active=false;
            player.foodCollected++;
            player.score += 30;
            spawnExplosion(f.x,f.y,Color(0.3f,1.0f,0.4f),14);
            if(player.foodCollected >= Player::foodForLife){
                player.foodCollected = 0;
                playSfx("extralife");
                if(player.lives < player.maxLives){
                    player.lives++;
                    player.score += 200;
                    uiMessage="\xE2\x98\x85 EXTRA LIFE! +200 PTS \xE2\x98\x85";
                    uiMessageTimer=200;
                    spawnFloatingText(player.x, player.y + 35, "+200 LIFE!", Color(0.35f, 1.0f, 0.5f), 1.6f);
                    // Triple burst for life gain
                    spawnExplosion(player.x,    player.y,    Color(1.0f,1.0f,0.3f),30);
                    spawnExplosion(player.x-25, player.y+10, Color(0.3f,1.0f,0.3f),15);
                    spawnExplosion(player.x+25, player.y+10, Color(0.3f,0.5f,1.0f),15);
                } else {
                    uiMessage="Lives FULL! +250 PTS";
                    uiMessageTimer=130;
                    player.score += 250;
                    spawnFloatingText(player.x, player.y + 35, "+250 MAX!", Color(1.0f, 0.85f, 0.2f), 1.6f);
                    spawnExplosion(player.x, player.y, Color(1,0.9f,0.1f),20);
                }
            } else {
                playSfx("coin");
                spawnFloatingText(f.x, f.y, "+30", Color(0.35f, 0.95f, 1.0f), 1.3f);
                int rem = Player::foodForLife - player.foodCollected;
                uiMessage="Food! +30 PTS ("+std::to_string(rem)+" more for LIFE)";
                uiMessageTimer=100;
            }
        }
    }

    for(auto& p : powerups){
        if(!p.active) continue;
        if(p.getAABB().intersects(pa)){
            p.active=false;
            player.score += 50;
            playSfx("powerup");
            std::string pTxt = "+50 ";
            switch(p.type){
                case PowerUpType::FIRE_RATE:
                    player.activateFireRate();
                    pTxt += "RAPID";
                    uiMessage="FIRE RATE UP! +50 PTS"; uiMessageTimer=120;
                    break;
                case PowerUpType::SHIELD:
                    player.activateShield();
                    pTxt += "SHIELD";
                    uiMessage="SHIELD ACTIVE! +50 PTS"; uiMessageTimer=120;
                    break;
                case PowerUpType::STRONG_BULLET:
                    player.activateStrongBullet();
                    pTxt += "STRONG";
                    uiMessage="STRONG BULLETS! +50 PTS"; uiMessageTimer=120;
                    break;
            }
            spawnFloatingText(p.x, p.y, pTxt, Color(0.4f, 1.0f, 0.9f), 1.4f);
            spawnExplosion(p.x,p.y,Color(0.5f,1,1),12);
        }
    }
}

// ---------------------------------------------------------------------------
// Update — main game loop tick
// ---------------------------------------------------------------------------
void Game::update(){
    if(state != GameState::PLAYING) return;

    globalTime += 0.016f;
    bgScroll += 1.2f;
    if(bgScroll > WIN_H) bgScroll -= WIN_H;

    // Update starfield (CG Concept 7a: translation — stars scroll down)
    for(auto& s : stars){
        s.y -= s.speed;
        s.animPhase += 0.04f + 0.03f*s.speed; // faster stars twinkle faster
        if(s.y < 0){ s.y=WIN_H; s.x=randF(0,WIN_W); s.animPhase=randF(0,6.28f); }
    }

    // Player
    player.update();
    if(player.shotFired){
        playSfx("laser");
    }

    // Enemies
    for(auto& e : enemies) e.update();
    enemies.erase(std::remove_if(enemies.begin(),enemies.end(),
        [](const Enemy& e){ return !e.active && e.eggs.empty(); }
    ), enemies.end());

    // Boss
    if(bossSpawned && boss && boss->active){
        boss->update(player.x, player.y);
        if(boss->laserJustFired){
            playSfx("bosslaser");
            boss->laserJustFired = false;
        }
    }

    // Pickups
    for(auto& c : coins)    c.update();
    for(auto& f : foods)    f.update();
    for(auto& p : powerups) p.update();

    coins.erase(std::remove_if(coins.begin(),coins.end(),
        [](const Coin& c){ return !c.active; }), coins.end());
    foods.erase(std::remove_if(foods.begin(),foods.end(),
        [](const Food& f){ return !f.active; }), foods.end());
    powerups.erase(std::remove_if(powerups.begin(),powerups.end(),
        [](const PowerUp& p){ return !p.active; }), powerups.end());

    // Particles
    for(auto& p : particles){
        p.x  += p.vx;
        p.y  += p.vy;
        p.vy -= 0.1f; // gravity
        p.life--;
        if(p.life<=0) p.active=false;
    }
    particles.erase(std::remove_if(particles.begin(),particles.end(),
        [](const Particle& p){ return !p.active; }), particles.end());

    // Floating score / popup texts
    for(auto& ft : floatingTexts) ft.update();
    floatingTexts.erase(std::remove_if(floatingTexts.begin(), floatingTexts.end(),
        [](const FloatingText& ft){ return !ft.active; }), floatingTexts.end());

    // Combo timer decay
    if(comboTimer > 0){
        comboTimer--;
        if(comboTimer <= 0){
            resetCombo(false);
        }
    }

    // Collisions
    handleBulletCollisions();
    handleEggCollisions();
    handlePickups();

    // UI message timeout
    if(uiMessageTimer>0) uiMessageTimer--;

    // Boss Warning Sequence & Spawn (when wave enemies are cleared)
    if(!waveSpawned && enemies.empty() && !bossSpawned && !bossWarningActive){
        spawnWave();
    } else if(waveSpawned && enemies.empty() && !bossSpawned && !bossWarningActive){
        triggerBossWarning();
    }

    if(bossWarningActive){
        if(--bossWarningTimer <= 0){
            bossWarningActive = false;
            spawnBoss();
        } else {
            // Pulse siren sound every 36 frames
            if(bossWarningTimer % 36 == 0){
                playSfx("siren");
            }
        }
    }

    // Level complete — boss dead
    if(bossSpawned && boss && !boss->active && enemies.empty()){
        if(level >= 3){
            state=GameState::WIN;
            checkEndGameNotification();
            return;
        }
        nextLevel();
    }

    // Enemy reaches bottom → game over
    for(auto& e : enemies){
        if(e.active && e.y < 50){
            resetCombo(false);
            state=GameState::GAME_OVER;
            checkEndGameNotification();
            return;
        }
    }

    // Player dead — lose a life or end the game
    if(player.hp <= 0 && player.respawnTimer <= 0){
        playSfx("hurt");
        resetCombo(false);
        if(player.lives > 0){
            player.lives--;
            player.hp = player.maxHp;
            player.respawnTimer = 160; // ~2.7 sec invincibility + blink
            // Stop movement so ship doesn't drift through enemies
            player.moveLeft=player.moveRight=player.moveUp=player.moveDown=false;
            spawnExplosion(player.x, player.y, Color(1,0.5f,0.2f), 35);
            uiMessage = (player.lives > 0)
                ? (std::to_string(player.lives)+" LIVES REMAINING!")
                : "LAST LIFE!";
            uiMessageTimer=150;
        } else {
            spawnExplosion(player.x, player.y, Color(1,0.2f,0.0f), 55);
            state=GameState::GAME_OVER;
            checkEndGameNotification();
        }
    }
}

// ---------------------------------------------------------------------------
// nextLevel
// ---------------------------------------------------------------------------
void Game::nextLevel(){
    level++;
    delete boss; boss=nullptr; bossSpawned=false;
    bossWarningActive=false; bossWarningTimer=0;
    waveSpawned=false;
    enemies.clear(); coins.clear(); foods.clear(); powerups.clear(); floatingTexts.clear();
    enemySpawnRate = std::max(60, 120 - level*15);
    uiMessage = "LEVEL " + std::to_string(level) + " !";
    uiMessageTimer = 180;
    playSfx("powerup");
    spawnWave();
}

// ---------------------------------------------------------------------------
// Keyboard handlers
// ---------------------------------------------------------------------------
void Game::onKeyDown(unsigned char key){
    if(state!=GameState::PLAYING) return;
    switch(key){
        case 'a': case 'A': player.moveLeft  =true; break;
        case 'd': case 'D': player.moveRight =true; break;
        case 'w': case 'W': player.moveUp    =true; break;
        case 's': case 'S': player.moveDown  =true; break;
        case ' ':           player.isShooting =true; break;
    }
}
void Game::onKeyUp(unsigned char key){
    switch(key){
        case 'a': case 'A': player.moveLeft  =false; break;
        case 'd': case 'D': player.moveRight =false; break;
        case 'w': case 'W': player.moveUp    =false; break;
        case 's': case 'S': player.moveDown  =false; break;
        case ' ':           player.isShooting =false; break;
    }
}
void Game::onSpecialDown(int key){
    if(state!=GameState::PLAYING) return;
    switch(key){
        case GLUT_KEY_LEFT:  player.moveLeft  =true; break;
        case GLUT_KEY_RIGHT: player.moveRight =true; break;
        case GLUT_KEY_UP:    player.moveUp    =true; break;
        case GLUT_KEY_DOWN:  player.moveDown  =true; break;
    }
}
void Game::onSpecialUp(int key){
    switch(key){
        case GLUT_KEY_LEFT:  player.moveLeft  =false; break;
        case GLUT_KEY_RIGHT: player.moveRight =false; break;
        case GLUT_KEY_UP:    player.moveUp    =false; break;
        case GLUT_KEY_DOWN:  player.moveDown  =false; break;
    }
}
void Game::onKeyPress(unsigned char key){
    switch(state){
        case GameState::MENU:
            if(key=='\r' || key==13){
                state=GameState::PLAYING;
                reset();
                playSfx("powerup");
                spawnWave();
            }
            break;
        case GameState::PLAYING:
            if(key=='p' || key=='P') state=GameState::PAUSED;
            if(key=='b' || key=='B'){ enemies.clear(); triggerBossWarning(); }
            break;
        case GameState::PAUSED:
            if(key=='p' || key=='P') state=GameState::PLAYING;
            if(key=='q' || key=='Q'){ state=GameState::MENU; reset(); }
            break;
        case GameState::GAME_OVER:
        case GameState::WIN:
            if(key=='\r' || key==13){ state=GameState::MENU; reset(); }
            break;
    }
}

// ---------------------------------------------------------------------------
// drawText helpers
// ---------------------------------------------------------------------------
#ifdef __EMSCRIPTEN__
void Game::drawText(float x, float y, const std::string& s, Color c, void* /*font*/){
    renderArcadeTextWithShadow(x, y, s, c, 1.6f, 1.2f);
}

void Game::drawTextLarge(float x, float y, const std::string& s, Color c){
    renderArcadeTextWithShadow(x, y, s, c, 2.6f, 2.0f);
}
#else
void Game::drawText(float x, float y, const std::string& s, Color c, void* font){
    c.apply();
    glRasterPos2f(x,y);
    for(char ch : s) glutBitmapCharacter(font, ch);
}

void Game::drawTextLarge(float x, float y, const std::string& s, Color c){
    drawText(x,y,s,c,GLUT_BITMAP_TIMES_ROMAN_24);
}
#endif

// ---------------------------------------------------------------------------
// drawStarfield — scrolling parallax background stars
// ---------------------------------------------------------------------------
void Game::drawStarfield(){
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    for(auto& s : stars){
        float twinkle = 0.55f + 0.45f*std::abs(std::sin(s.animPhase));
        float alpha = s.brightness * twinkle;
        float warm = 1.0f - 0.3f*(s.speed-0.3f)/1.7f;
        glColor4f(warm*alpha, warm*alpha*0.95f, alpha, alpha);
        glVertex2f(s.x, s.y);
    }
    glEnd();
    glPointSize(1.0f);

    // Subtle cross sparkle on the brightest large stars (at most a few per frame)
    for(auto& s : stars){
        if(s.size > 2.2f && std::sin(s.animPhase) > 0.90f){
            float sa = s.brightness * 0.45f;
            Color sparkC(sa,sa,sa,sa);
            bresenhamLine((int)s.x-2,(int)s.y,(int)s.x+2,(int)s.y, sparkC);
            bresenhamLine((int)s.x,(int)s.y-2,(int)s.x,(int)s.y+2, sparkC);
        }
    }
}

// ---------------------------------------------------------------------------
// drawBackground — deep space gradient + nebula
// ---------------------------------------------------------------------------
void Game::drawBackground(){
    // Vertical gradient (dark blue → near black)
    glBegin(GL_QUADS);
        glColor3f(0.0f, 0.0f, 0.05f); glVertex2i(0,0);
        glColor3f(0.0f, 0.0f, 0.05f); glVertex2i(WIN_W,0);
        glColor3f(0.02f,0.0f,0.12f);  glVertex2i(WIN_W,WIN_H);
        glColor3f(0.02f,0.0f,0.12f);  glVertex2i(0,WIN_H);
    glEnd();

    // Nebula blobs (soft circles)
    float t=globalTime;
    drawCircle(150+50*std::sin(t*0.3f), 500, 120, Color(0.1f,0.0f,0.2f,0.08f));
    drawCircle(700+40*std::cos(t*0.2f), 300, 100, Color(0.0f,0.05f,0.2f,0.08f));
    drawCircle(400, 200+30*std::sin(t*0.15f), 150, Color(0.05f,0.0f,0.15f,0.06f));

    drawStarfield();
}

// ---------------------------------------------------------------------------
// drawHUD — Retro Arcade HUD
// ---------------------------------------------------------------------------
void Game::drawHUD(){
    float t = globalTime;

    // ── Thin HUD separator line ───────────────────────────────────────────────
    ddaLine(0, WIN_H-35, WIN_W, WIN_H-35, Color(0.35f,0.30f,0.55f,0.45f));

    // ── HP bar — retro LCD style ──────────────────────────────────────────────
    float hpFrac=(float)player.hp/player.maxHp;
    // Bar track
    drawRect(22,10,202,17, Color(0.08f,0.08f,0.10f));
    drawRectOutline(22,10,202,17, Color(0.38f,0.32f,0.55f),1.5f);
    // Filled portion
    Color hpCol=(hpFrac>0.5f)?Color(0.12f,0.88f,0.22f):(hpFrac>0.25f)?Color(1.0f,0.72f,0.0f):Color(0.92f,0.10f,0.10f);
    drawRect(23,11,200*hpFrac,15,hpCol);
    // Scanline overlay on bar — every 3 pixels a semi-transparent dark stripe
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for(int scanY=11; scanY<26; scanY+=3){
        drawRect(23,scanY,200*hpFrac,1, Color(0,0,0,0.18f));
    }
    glDisable(GL_BLEND);
    // HP label
#ifdef __EMSCRIPTEN__
    renderArcadeText(26, 14, "HP "+std::to_string(player.hp), Color(0.9f,0.9f,1.0f), 1.4f);
#else
    drawText(24,13,"HP: "+std::to_string(player.hp)+"/"+std::to_string(player.maxHp),
        Color(1,1,1), GLUT_BITMAP_HELVETICA_12);
#endif

    // ── Score — arcade cyan with shadow ──────────────────────────────────────
#ifdef __EMSCRIPTEN__
    renderArcadeTextWithShadow(22, 40, "SCORE "+std::to_string(player.score),
        Color(0.0f,0.95f,0.85f), 1.7f, 1.5f, Color(0,0,0,0.7f));
    // Coins — retro amber
    renderArcadeTextWithShadow(22, 65, "COINS "+std::to_string(player.coins),
        Color(1.0f,0.70f,0.20f), 1.5f, 1.2f);
    // Level — outlined for emphasis
    renderArcadeTextOutlined(22, 90, "LVL "+std::to_string(level),
        Color(0.4f,0.92f,1.0f), 1.7f, Color(0.0f,0.25f,0.35f,0.85f));
#else
    drawText(22,40,"SCORE: "+std::to_string(player.score), Color(1,1,0.3f));
    drawText(22,65,"COINS: "+std::to_string(player.coins), Color(1,0.85f,0));
    drawText(22,90,"LEVEL: "+std::to_string(level), Color(0.5f,0.9f,1.0f));
#endif

    // ── LIVES display (mini ship icons) ──────────────────────────────────────
#ifdef __EMSCRIPTEN__
    renderArcadeText(22, 118, "LIVES", Color(1.0f,0.42f,0.42f), 1.4f);
#else
    drawText(22, 118, "LIVES:", Color(1.0f,0.55f,0.55f), GLUT_BITMAP_HELVETICA_12);
#endif
    for(int i=0;i<player.maxLives;i++){
        float lx = 74.0f + i*30.0f;
        float lcy = 118.0f;
        if(i < player.lives){
            std::vector<Vec2> shipMini={
                {lx,      lcy+13},
                {lx-9,    lcy-4},
                {lx-6,    lcy-10},
                {lx+6,    lcy-10},
                {lx+9,    lcy-4}
            };
            scanlineFill(shipMini, Color(0.3f,0.6f,1.0f));
            drawCircle(lx, lcy+4, 4, Color(0.6f,0.9f,1.0f,0.85f));
            // Small glow under active life
            glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            drawCircle(lx, lcy, 8, Color(0.3f,0.5f,1.0f,0.12f));
            glDisable(GL_BLEND);
        } else {
            std::vector<Vec2> shipMini={
                {lx,      lcy+13},
                {lx-9,    lcy-4},
                {lx-6,    lcy-10},
                {lx+6,    lcy-10},
                {lx+9,    lcy-4}
            };
            scanlineFill(shipMini, Color(0.18f,0.18f,0.22f));
        }
    }

    // ── FOOD progress bar toward next life ───────────────────────────────────
    float foodPct = (float)player.foodCollected / (float)Player::foodForLife;
#ifdef __EMSCRIPTEN__
    renderArcadeText(22, 148, "FOOD", Color(0.30f,0.88f,0.30f), 1.4f);
#else
    drawText(22, 148, "FOOD:", Color(0.4f,0.9f,0.4f), GLUT_BITMAP_HELVETICA_12);
#endif
    drawRect(64, 145, 96, 11, Color(0.06f,0.14f,0.06f));
    drawRect(64, 145, 96*foodPct, 11, Color(0.20f+0.25f*foodPct, 0.85f, 0.20f));
    // Scanline on food bar
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for(int scanY=145; scanY<156; scanY+=3)
        drawRect(64,scanY,96*foodPct,1, Color(0,0,0,0.18f));
    glDisable(GL_BLEND);
    drawRectOutline(64, 145, 96, 11, Color(0.30f,0.55f,0.30f),1.2f);
#ifdef __EMSCRIPTEN__
    renderArcadeText(165, 148,
        std::to_string(player.foodCollected)+"/"+std::to_string(Player::foodForLife),
        Color(0.6f,1.0f,0.6f), 1.3f);
#else
    drawText(165, 148,
        std::to_string(player.foodCollected)+"/"+std::to_string(Player::foodForLife)+" for LIFE",
        Color(0.7f,1.0f,0.7f), GLUT_BITMAP_HELVETICA_12);
#endif

    // ── Respawn invincibility — animated blink ────────────────────────────────
    if(player.respawnTimer > 0){
        float blinkAlpha = 0.55f + 0.45f*std::abs(std::sin(t*8.0f));
#ifdef __EMSCRIPTEN__
        float tw2 = 160.0f;
        glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        drawRect(WIN_W/2-tw2/2-4, WIN_H-58, tw2+8, 24, Color(0,0.1f,0.15f,0.6f*blinkAlpha));
        drawRectOutline(WIN_W/2-tw2/2-4, WIN_H-58, tw2+8, 24, Color(0.3f,0.8f,1.0f,0.7f*blinkAlpha),1.5f);
        glDisable(GL_BLEND);
        renderArcadeTextGlow(WIN_W/2-74, WIN_H-53, "INVINCIBLE!",
            Color(0.4f,0.95f,1.0f,blinkAlpha), 1.6f,
            Color(0.2f,0.8f,1.0f,blinkAlpha), 0.3f*blinkAlpha);
#else
        drawText(WIN_W/2-50, WIN_H-52, "INVINCIBLE!", Color(0.4f,0.9f,1.0f,blinkAlpha),
                 GLUT_BITMAP_HELVETICA_12);
#endif
    }

    // ── Power-up indicators (top-right) — with blinking border ───────────────
    float px=WIN_W-200;
    float borderPulse = 0.5f + 0.5f*std::abs(std::sin(t*5.0f));
    if(player.shieldActive){
        drawRect(px,WIN_H-32,88,24, Color(0.06f,0.18f,0.45f,0.80f));
        drawRectOutline(px,WIN_H-32,88,24, Color(0.35f,0.70f,1.0f,borderPulse),1.8f);
#ifdef __EMSCRIPTEN__
        renderArcadeText(px+5,WIN_H-27,"SHIELD", Color(0.40f,0.85f,1.0f), 1.5f);
#else
        drawText(px+4,WIN_H-26,"SHIELD", Color(0.4f,0.8f,1.0f));
#endif
        px+=98;
    }
    if(player.strongBulletActive){
        drawRect(px,WIN_H-32,88,24, Color(0.35f,0.08f,0.0f,0.80f));
        drawRectOutline(px,WIN_H-32,88,24, Color(1.0f,0.42f,0.10f,borderPulse),1.8f);
#ifdef __EMSCRIPTEN__
        renderArcadeText(px+5,WIN_H-27,"STRONG", Color(1.0f,0.45f,0.10f), 1.5f);
#else
        drawText(px+4,WIN_H-26,"STRONG", Color(1,0.4f,0.1f));
#endif
        px+=98;
    }
    if(player.fireRateActive){
        drawRect(px,WIN_H-32,104,24, Color(0.28f,0.28f,0.0f,0.80f));
        drawRectOutline(px,WIN_H-32,104,24, Color(1.0f,1.0f,0.15f,borderPulse),1.8f);
#ifdef __EMSCRIPTEN__
        renderArcadeText(px+5,WIN_H-27,"RAPID", Color(1.0f,1.0f,0.15f), 1.5f);
#else
        drawText(px+4,WIN_H-26,"FAST FIRE", Color(1,1,0.2f));
#endif
    }

    // ── UI message (center) — glowing dramatic text ───────────────────────────
    if(uiMessageTimer>0){
        float alpha=(float)uiMessageTimer/120.0f;
        if(alpha>1) alpha=1;
#ifdef __EMSCRIPTEN__
        float charW = 1.9f * 7.0f;
        float tw = uiMessage.size() * charW;
        glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        drawRect(WIN_W/2-tw/2-10, WIN_H/2-12, tw+20, 28, Color(0.02f,0.02f,0.05f,0.72f*alpha));
        drawRectOutline(WIN_W/2-tw/2-10, WIN_H/2-12, tw+20, 28, Color(1.0f,0.88f,0.0f,0.55f*alpha),1.5f);
        glDisable(GL_BLEND);
        renderArcadeTextGlow(WIN_W/2-tw/2, WIN_H/2-8, uiMessage,
            Color(1.0f,0.95f,0.20f,alpha), 1.9f,
            Color(1.0f,0.75f,0.0f,alpha), 0.35f*alpha);
#else
        float tw=uiMessage.size()*10.0f;
        drawRect(WIN_W/2-tw/2-8, WIN_H/2-16, tw+16, 28, Color(0,0,0,0.5f*alpha));
        drawTextLarge(WIN_W/2-tw/2, WIN_H/2-8, uiMessage, Color(1,1,0.3f,alpha));
#endif
    }

    // ── Pause hint ────────────────────────────────────────────────────────────
#ifdef __EMSCRIPTEN__
    renderArcadeText(WIN_W-96,14,"[P] PAUSE", Color(0.38f,0.35f,0.50f), 1.4f);
#else
    drawText(WIN_W-80,12,"[P] Pause", Color(0.5f,0.5f,0.5f), GLUT_BITMAP_HELVETICA_12);
#endif
}

// ---------------------------------------------------------------------------
// drawMenu — Retro Arcade Menu
// ---------------------------------------------------------------------------
void Game::drawMenu(){
    drawBackground();
    float t = globalTime;

    // ── Corner bracket decorations (arcade marquee feel) ──────────────────────
    float bw=180, bh=16;
    // Top-left bracket
    drawRect(18, WIN_H-22, bw, 2, Color(0.20f,0.65f,0.85f,0.45f));
    drawRect(18, WIN_H-22, 2, bh, Color(0.20f,0.65f,0.85f,0.45f));
    // Top-right bracket
    drawRect(WIN_W-18-bw, WIN_H-22, bw, 2, Color(0.20f,0.65f,0.85f,0.45f));
    drawRect(WIN_W-20, WIN_H-22, 2, bh, Color(0.20f,0.65f,0.85f,0.45f));
    // Bottom-left bracket
    drawRect(18, 18, bw, 2, Color(0.20f,0.65f,0.85f,0.35f));
    drawRect(18, 18, 2, bh, Color(0.20f,0.65f,0.85f,0.35f));
    // Bottom-right bracket
    drawRect(WIN_W-18-bw, 18, bw, 2, Color(0.20f,0.65f,0.85f,0.35f));
    drawRect(WIN_W-20, 18, 2, bh, Color(0.20f,0.65f,0.85f,0.35f));

    // ── Title — outlined arcade marquee style ────────────────────────────────
    float ty = WIN_H - 115;
#ifdef __EMSCRIPTEN__
    // Decorative lines above/below title
    float lineY1 = ty + 28, lineY2 = ty - 10;
    // Glowing title underline
    for(int i=0;i<3;i++){
        float la = 0.55f - i*0.16f;
        ddaLine((int)(WIN_W/2-185),(int)(lineY2-i),
                (int)(WIN_W/2+185),(int)(lineY2-i), Color(0.15f,0.85f,0.92f,la));
        ddaLine((int)(WIN_W/2-185),(int)(lineY1+i),
                (int)(WIN_W/2+185),(int)(lineY1+i), Color(0.15f,0.85f,0.92f,la));
    }
    // Star decorations flanking title
    drawCircle(WIN_W/2-175, ty+8, 3, Color(0.20f,0.95f,0.85f));
    drawCircle(WIN_W/2+175, ty+8, 3, Color(0.20f,0.95f,0.85f));
    // Main title — large outlined arcade cyan
    renderArcadeTextOutlined(WIN_W/2-168, ty, "CHICKEN INVADERS",
        Color(0.10f,0.95f,0.88f), 2.4f,
        Color(0.0f,0.22f,0.32f,0.95f));
#else
    drawText(WIN_W/2-160, ty, "CHICKEN INVADERS", Color(0.10f,0.95f,0.88f),
             GLUT_BITMAP_TIMES_ROMAN_24);
    ddaLine(WIN_W/2-165,(int)(ty-10), WIN_W/2+165,(int)(ty-10), Color(0,0.8f,0.8f));
    ddaLine(WIN_W/2-165,(int)(ty+26), WIN_W/2+165,(int)(ty+26), Color(0,0.8f,0.8f));
#endif

    // ── "PRESS ENTER" — classic arcade blinking ──────────────────────────────
    float blinkA = 0.45f + 0.55f * std::abs(std::sin(t * 2.8f));
#ifdef __EMSCRIPTEN__
    renderArcadeTextGlow(WIN_W/2-138, WIN_H/2+18, "PRESS START OR ENTER",
        Color(1.0f,1.0f,1.0f,blinkA), 1.7f,
        Color(0.2f,0.8f,1.0f,blinkA), 0.22f*blinkA);
    // Controls — dimmer secondary info
    renderArcadeText(WIN_W/2-122, WIN_H/2-10, "WASD / ARROWS = MOVE",
        Color(0.50f,0.50f,0.65f,0.85f), 1.4f);
    renderArcadeText(WIN_W/2-114, WIN_H/2-30, "HOLD SPACE=SHOOT  P=PAUSE",
        Color(0.42f,0.65f,0.55f,0.85f), 1.4f);
#else
    drawText(WIN_W/2-80, WIN_H/2+20,"PRESS ENTER TO START", Color(1,1,1,blinkA));
    drawText(WIN_W/2-100, WIN_H/2-10,"WASD or Arrow Keys to Move", Color(0.7f,0.7f,0.7f),
             GLUT_BITMAP_HELVETICA_12);
    drawText(WIN_W/2-100, WIN_H/2-28,"Hold Space = Shoot | P = Pause", Color(0.7f,0.7f,0.7f),
             GLUT_BITMAP_HELVETICA_12);
#endif

    // ── Decorative chicken — uses enhanced Enemy visuals automatically ────────
    float cx=WIN_W/2.0f, cy=WIN_H/2.0f-120;
    Enemy demo(cx+50*std::sin(t*0.5f), cy, EnemyType::NORMAL);
    demo.animTime = t*2;
    demo.wingFlap = 25.0f*std::sin(t*4);
    demo.draw();

    // ── Footer credit ─────────────────────────────────────────────────────────
#ifdef __EMSCRIPTEN__
    renderArcadeText(WIN_W/2-112, 28, "COMPUTER GRAPHICS PROJECT",
        Color(0.38f,0.35f,0.52f,0.75f), 1.3f);
#else
    drawText(WIN_W/2-80, 30, "Computer Graphics Project", Color(0.5f,0.5f,0.7f),
             GLUT_BITMAP_HELVETICA_12);
#endif
}

// ---------------------------------------------------------------------------
// drawPauseScreen — Retro Pause Overlay
// ---------------------------------------------------------------------------
void Game::drawPauseScreen(){
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    drawRect(0,0,WIN_W,WIN_H, Color(0,0,0,0.62f));
    glDisable(GL_BLEND);

    // Retro pause box (charcoal with arcade green borders)
    float bx=WIN_W/2-150, by=WIN_H/2-55;
    drawRect(bx, by, 300, 110, Color(0.04f,0.08f,0.06f,0.95f));
    drawRectOutline(bx, by, 300, 110, Color(0.0f,0.85f,0.45f,0.85f), 2.0f);
    // Inner border
    drawRectOutline(bx+4, by+4, 292, 102, Color(0.0f,0.45f,0.25f,0.50f), 1.0f);
    // Corner dots
    drawCircle(bx+5,   by+5,   3, Color(0.0f,0.95f,0.55f));
    drawCircle(bx+295, by+5,   3, Color(0.0f,0.95f,0.55f));
    drawCircle(bx+5,   by+105, 3, Color(0.0f,0.95f,0.55f));
    drawCircle(bx+295, by+105, 3, Color(0.0f,0.95f,0.55f));

#ifdef __EMSCRIPTEN__
    renderArcadeTextOutlined(WIN_W/2-60, WIN_H/2+26, "PAUSED",
        Color(0.0f,0.95f,0.55f), 2.4f, Color(0.0f,0.25f,0.12f,0.9f));
    renderArcadeText(WIN_W/2-132, WIN_H/2-6, "TAP START / PAUSE TO RESUME",
        Color(0.75f,0.90f,0.80f,0.92f), 1.35f);
    renderArcadeText(WIN_W/2-84, WIN_H/2-28, "P: RESUME  |  Q: MENU",
        Color(0.40f,0.65f,0.50f,0.70f), 1.1f);
#else
    drawTextLarge(WIN_W/2-50, WIN_H/2+20,"PAUSED", Color(0.0f,0.95f,0.55f));
    drawText(WIN_W/2-80, WIN_H/2-10,"[P] Resume  |  [Q] Quit to Menu", Color(0.8f,0.8f,0.8f));
#endif
}

// ---------------------------------------------------------------------------
// drawGameOver — Retro Dramatic Game Over Screen
// ---------------------------------------------------------------------------
void Game::drawGameOver(){
    drawBackground();

    // Explosion particles
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for(auto& p : particles){
        if(!p.active) continue;
        float alpha=p.life/p.maxLife;
        Color pc=p.color; pc.a*=alpha;
        drawCircle(p.x,p.y,p.size*alpha,pc);
    }
    glDisable(GL_BLEND);

    // Pulsing red glow behind box
    float pulse = 0.10f + 0.08f * std::abs(std::sin(globalTime * 2.5f));
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    drawCircle(WIN_W/2, WIN_H/2, 200, Color(0.5f,0.02f,0.02f, pulse));
    glDisable(GL_BLEND);

    // Main box — dark with layered red border
    float bx=WIN_W/2-185, by=WIN_H/2-65;
    drawRect(bx, by, 370, 140, Color(0.06f,0.00f,0.00f,0.95f));
    drawRectOutline(bx,   by,   370, 140, Color(0.75f,0.05f,0.05f,0.90f), 3.0f);
    drawRectOutline(bx+5, by+5, 360, 130, Color(0.45f,0.02f,0.02f,0.60f), 1.5f);
    // Corner flame markers
    drawCircle(bx+5,   by+5,   5, Color(0.9f,0.2f,0.0f,0.7f));
    drawCircle(bx+365, by+5,   5, Color(0.9f,0.2f,0.0f,0.7f));
    drawCircle(bx+5,   by+135, 5, Color(0.9f,0.2f,0.0f,0.7f));
    drawCircle(bx+365, by+135, 5, Color(0.9f,0.2f,0.0f,0.7f));

    // Pulsing "GAME OVER" text
    float textPulse = 0.80f + 0.20f * std::abs(std::sin(globalTime * 3.2f));
    int finalScore = player.score + std::max(0, player.lives)*200 + player.coins*2;
#ifdef __EMSCRIPTEN__
    renderArcadeTextOutlined(WIN_W/2-134, WIN_H/2+32, "GAME OVER",
        Color(0.95f,0.08f,0.08f,textPulse), 3.2f,
        Color(0.15f,0.0f,0.0f,0.95f));
    // Score
    renderArcadeTextWithShadow(WIN_W/2-80, WIN_H/2+1,
        "SCORE "+std::to_string(finalScore),
        Color(0.20f,0.95f,0.85f), 1.7f, 1.5f);
    // Blinking "PRESS START OR ENTER"
    float ba = 0.45f + 0.55f*std::abs(std::sin(globalTime*2.8f));
    renderArcadeText(WIN_W/2-138, WIN_H/2-22, "PRESS START OR ENTER",
        Color(0.62f,0.60f,0.70f,ba), 1.4f);
#else
    drawTextLarge(WIN_W/2-100, WIN_H/2+30,"GAME OVER", Color(0.9f,0.1f,0.1f,textPulse));
    drawText(WIN_W/2-70, WIN_H/2+5,"Score: "+std::to_string(finalScore), Color(0.2f,0.95f,0.85f));
    drawText(WIN_W/2-110, WIN_H/2-15,"PRESS ENTER to return to menu", Color(0.7f,0.7f,0.7f));
#endif
}

// ---------------------------------------------------------------------------
// drawWinScreen — Retro Victory Screen
// ---------------------------------------------------------------------------
void Game::drawWinScreen(){
    drawBackground();

    // Victory celebration: still-animating particles
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for(auto& p : particles){
        if(!p.active) continue;
        float alpha=p.life/p.maxLife;
        Color pc=p.color; pc.a*=alpha;
        drawCircle(p.x,p.y,p.size*alpha,pc);
    }
    // Emerald green glow behind box
    float victPulse = 0.08f + 0.06f*std::abs(std::sin(globalTime*2.0f));
    drawCircle(WIN_W/2, WIN_H/2, 220, Color(0.0f,0.5f,0.2f,victPulse));
    glDisable(GL_BLEND);

    // Main box — dark green with layered border
    float bx=WIN_W/2-185, by=WIN_H/2-65;
    drawRect(bx, by, 370, 150, Color(0.00f,0.06f,0.01f,0.95f));
    drawRectOutline(bx,   by,   370, 150, Color(0.18f,0.92f,0.28f,0.90f), 3.0f);
    drawRectOutline(bx+5, by+5, 360, 140, Color(0.10f,0.55f,0.16f,0.55f), 1.5f);
    // Corner stars
    drawCircle(bx+5,   by+5,   5, Color(0.8f,1.0f,0.2f,0.8f));
    drawCircle(bx+365, by+5,   5, Color(0.8f,1.0f,0.2f,0.8f));
    drawCircle(bx+5,   by+145, 5, Color(0.8f,1.0f,0.2f,0.8f));
    drawCircle(bx+365, by+145, 5, Color(0.8f,1.0f,0.2f,0.8f));

    int winScore = player.score + std::max(0, player.lives)*200 + player.coins*2;
#ifdef __EMSCRIPTEN__
    // "YOU WIN!" — outlined, bright
    renderArcadeTextOutlined(WIN_W/2-102, WIN_H/2+42, "YOU WIN!",
        Color(0.22f,1.0f,0.32f), 3.0f,
        Color(0.0f,0.15f,0.0f,0.95f));
    // Score — glowing arcade cyan
    renderArcadeTextGlow(WIN_W/2-104, WIN_H/2+10,
        "SCORE "+std::to_string(winScore),
        Color(0.0f,0.95f,0.85f), 1.7f,
        Color(0.0f,0.60f,0.50f), 0.28f);
    renderArcadeTextWithShadow(WIN_W/2-68, WIN_H/2-12,
        "COINS "+std::to_string(player.coins),
        Color(1.0f,0.70f,0.20f), 1.6f, 1.2f);
    // Blinking continue
    float baWin = 0.45f + 0.55f*std::abs(std::sin(globalTime*2.8f));
    renderArcadeText(WIN_W/2-138, WIN_H/2-32, "PRESS START OR ENTER",
        Color(0.55f,0.78f,0.55f,baWin), 1.4f);
#else
    drawTextLarge(WIN_W/2-100, WIN_H/2+40, "YOU WIN!", Color(0.2f,1.0f,0.3f));
    drawText(WIN_W/2-80, WIN_H/2+12,"Final Score: "+std::to_string(player.score), Color(1,1,0.3f));
    drawText(WIN_W/2-80, WIN_H/2-8, "Coins: "+std::to_string(player.coins), Color(1,0.85f,0));
    drawText(WIN_W/2-110, WIN_H/2-28,"PRESS ENTER to return to menu", Color(0.7f,0.7f,0.7f));
#endif
}

// ---------------------------------------------------------------------------
// drawBossWarning — Retro Arcade Hazard Warning Overlay
// ---------------------------------------------------------------------------
void Game::drawBossWarning(){
    if(!bossWarningActive) return;

    float t = globalTime;
    // Ambient red CRT strobe flash
    float strobe = 0.08f + 0.07f * std::sin(t * 14.0f);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    drawRect(0, 0, WIN_W, WIN_H, Color(0.85f, 0.05f, 0.05f, strobe));

    // Top & bottom animated hazard stripe bars
    auto drawHazardBar = [&](float yBase, float barH) {
        drawRect(0, yBase, WIN_W, barH, Color(0.08f, 0.08f, 0.08f, 0.95f));
        float stripeW = 28.0f;
        float offset = std::fmod(t * 65.0f, stripeW * 2.0f);
        for(float x = -stripeW * 2.0f + offset; x < WIN_W + stripeW * 2.0f; x += stripeW * 2.0f){
            glColor4f(1.0f, 0.78f, 0.0f, 0.92f);
            glBegin(GL_QUADS);
            glVertex2f(x, yBase);
            glVertex2f(x + stripeW * 0.85f, yBase);
            glVertex2f(x + stripeW * 0.85f + 18.0f, yBase + barH);
            glVertex2f(x + 18.0f, yBase + barH);
            glEnd();
        }
        drawRectOutline(0, yBase, WIN_W, barH, Color(1.0f, 0.2f, 0.2f, 0.90f), 2.0f);
    };

    drawHazardBar(WIN_H - 34, 34);
    drawHazardBar(0, 34);

    // Center warning alert box
    float bw = 380, bh = 110;
    float bx = WIN_W/2.0f - bw/2.0f, by = WIN_H/2.0f - bh/2.0f + 20;

    float boxPulse = 0.85f + 0.15f * std::abs(std::sin(t * 8.0f));
    drawRect(bx, by, bw, bh, Color(0.05f, 0.0f, 0.02f, 0.94f));
    drawRectOutline(bx, by, bw, bh, Color(1.0f, 0.15f, 0.15f, boxPulse), 3.0f);
    drawRectOutline(bx+4, by+4, bw-8, bh-8, Color(1.0f, 0.65f, 0.0f, 0.75f), 1.5f);

    float flashAlpha = 0.45f + 0.55f * std::abs(std::sin(t * 10.0f));
#ifdef __EMSCRIPTEN__
    renderArcadeTextOutlined(WIN_W/2 - 134, by + bh - 32, "! WARNING !",
        Color(1.0f, 0.18f, 0.18f, flashAlpha), 2.6f,
        Color(0.25f, 0.0f, 0.0f, 0.95f));

    renderArcadeTextGlow(WIN_W/2 - 160, by + bh - 64, "MOTHERSHIP ROOSTER DETECTED",
        Color(1.0f, 0.85f, 0.20f), 1.55f,
        Color(1.0f, 0.2f, 0.0f), 0.35f);

    renderArcadeText(WIN_W/2 - 110, by + 16, "PREPARE FOR BATTLE",
        Color(0.85f, 0.85f, 0.90f, boxPulse), 1.35f);
#else
    drawTextLarge(WIN_W/2 - 90, by + bh - 35, "! WARNING !", Color(1.0f, 0.1f, 0.1f));
    drawText(WIN_W/2 - 140, by + bh - 65, "MOTHERSHIP ROOSTER DETECTED", Color(1.0f, 0.85f, 0.2f));
    drawText(WIN_W/2 - 90, by + 18, "PREPARE FOR BATTLE", Color(0.8f, 0.8f, 0.85f));
#endif
    glDisable(GL_BLEND);
}

// ---------------------------------------------------------------------------
// Floating score / popup texts rendering
// ---------------------------------------------------------------------------
void Game::drawFloatingTexts(){
    if(floatingTexts.empty()) return;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for(const auto& ft : floatingTexts){
        if(!ft.active) continue;
        float alpha = ft.life / ft.maxLife;
        if(alpha > 1.0f) alpha = 1.0f;
        if(alpha < 0.0f) alpha = 0.0f;
        Color c = ft.color;
        c.a *= alpha;

        float charW = ft.scale * 6.5f;
        float tw = (float)ft.text.length() * charW;
        float drawX = ft.x - tw / 2.0f;
        float drawY = ft.y;

#ifdef __EMSCRIPTEN__
        renderArcadeTextWithShadow(drawX, drawY, ft.text, c, ft.scale, 1.2f, Color(0.0f, 0.0f, 0.0f, 0.7f * alpha));
#else
        drawText(drawX, drawY, ft.text, c);
#endif
    }
    glDisable(GL_BLEND);
}

// ---------------------------------------------------------------------------
// Combo / Streak HUD badge rendering
// ---------------------------------------------------------------------------
void Game::drawComboHUD(){
    if(comboStreak < 3 && comboMultiplier <= 1) return;

    float t = globalTime;
    float pulse = 0.85f + 0.15f * std::sin(t * 12.0f);

    Color badgeCol;
    Color glowCol;
    std::string streakText;
    if(comboMultiplier >= 5){
        badgeCol = Color(1.0f, 0.20f, 0.35f);  // Neon Crimson
        glowCol  = Color(1.0f, 0.85f, 0.20f);  // Gold glow
        streakText = "SUPER x" + std::to_string(comboMultiplier) + "!";
    } else if(comboMultiplier == 4){
        badgeCol = Color(1.0f, 0.55f, 0.05f);  // Fiery Orange
        glowCol  = Color(1.0f, 0.90f, 0.20f);
        streakText = "COMBO x4";
    } else if(comboMultiplier == 3){
        badgeCol = Color(0.95f, 0.85f, 0.10f); // Electric Yellow
        glowCol  = Color(1.0f, 1.0f, 0.40f);
        streakText = "COMBO x3";
    } else if(comboMultiplier == 2){
        badgeCol = Color(0.20f, 0.90f, 1.00f); // Bright Cyan
        glowCol  = Color(0.00f, 0.60f, 1.00f);
        streakText = "COMBO x2";
    } else {
        badgeCol = Color(0.70f, 0.75f, 0.95f); // Streak building (3-4 kills)
        glowCol  = Color(0.35f, 0.45f, 0.85f);
        streakText = "STREAK " + std::to_string(comboStreak);
    }

    float bx = 12.0f;
    float by = WIN_H - 33.0f;
    float bw = 114.0f;
    float bh = 24.0f;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Background pill badge
    drawRect(bx, by, bw, bh, Color(0.04f, 0.05f, 0.12f, 0.85f));
    drawRectOutline(bx, by, bw, bh, Color(badgeCol.r, badgeCol.g, badgeCol.b, pulse), 1.5f);

    // Mini timer countdown gauge inside the badge bottom
    float timerFrac = (float)comboTimer / (float)comboMaxTimer;
    if(timerFrac > 0.0f){
        drawRect(bx + 3, by + 2, (bw - 6) * timerFrac, 3, Color(glowCol.r, glowCol.g, glowCol.b, 0.95f));
    }
    glDisable(GL_BLEND);

#ifdef __EMSCRIPTEN__
    renderArcadeTextGlow(bx + 8, by + 8, streakText, badgeCol, 1.4f, glowCol, 0.35f * pulse);
#else
    drawText(bx + 8, by + 8, streakText, badgeCol);
#endif
}

// ---------------------------------------------------------------------------
// display — CG Concept 13: Double Buffering
// The scene is drawn into the BACK buffer.  glutSwapBuffers() atomically
// swaps it to the front, preventing screen tearing.
// ---------------------------------------------------------------------------
void Game::display(){
    // Clear back buffer
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    // CG Concept 11: Window-to-Viewport mapping set up via glOrtho in main
    // (world coords 0..WIN_W, 0..WIN_H map to NDC via orthographic projection)

    switch(state){
        case GameState::MENU:
            drawMenu();
            break;

        case GameState::PLAYING:
        case GameState::PAUSED: {
            drawBackground();

            // Draw pickups
            for(auto& c : coins)    c.draw();
            for(auto& f : foods)    f.draw();
            for(auto& p : powerups) p.draw();

            // Draw enemies
            for(auto& e : enemies) e.draw();

            // Draw boss
            if(boss && boss->active) boss->draw(player.x, player.y);

            // Draw player (includes bullets)
            player.draw();

            // Draw particles
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
            for(auto& p : particles){
                if(!p.active) continue;
                float alpha=p.life/p.maxLife;
                Color pc=p.color; pc.a*=alpha;
                drawCircle(p.x,p.y,p.size*alpha,pc, 8);
            }
            glDisable(GL_BLEND);

            drawHUD();
            drawComboHUD();
            drawFloatingTexts();

            if(bossWarningActive) drawBossWarning();

            if(state==GameState::PAUSED) drawPauseScreen();
            break;
        }

        case GameState::GAME_OVER:
            drawGameOver();
            break;

        case GameState::WIN:
            drawWinScreen();
            break;
    }

    // CG Concept 13: Double Buffering — swap back→front
    glutSwapBuffers();
}
