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
            g_game->state = GameState::PLAYING;
            g_game->reset();
            g_game->spawnWave();
        }
    }

    EMSCRIPTEN_KEEPALIVE
    int wasm_get_score() {
        return g_game ? g_game->player.score : 0;
    }
}
#endif

// ---------------------------------------------------------------------------
// Constructor / Destructor
// ---------------------------------------------------------------------------
Game::Game()
    : state(GameState::MENU), level(1), globalTime(0),
      boss(nullptr), bossSpawned(false),
      enemySpawnTimer(0), enemySpawnRate(120),
      powerupTimer(0), coinTimer(0),
      bgScroll(0), uiMessageTimer(0),
      notifiedEndGame(false)
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
    delete boss; boss=nullptr;
    bossSpawned=false;
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
#ifdef __EMSCRIPTEN__
        EM_ASM({
            if (window.onGameFinished) {
                window.onGameFinished($0, $1, $2);
            }
        }, player.score, (state == GameState::WIN ? 1 : 0), player.coins);
#endif
    }
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
        }
    }
}

// ---------------------------------------------------------------------------
// Spawn explosion particles
// ---------------------------------------------------------------------------
void Game::spawnExplosion(float x, float y, Color c, int count){
    for(int i=0;i<count;i++){
        float angle = randF(0, 2*PI);
        float speed = randF(1.5f, 6.0f);
        float life  = randF(20, 50);
        float sz    = randF(2, 6);
        particles.emplace_back(x,y,
            speed*std::cos(angle), speed*std::sin(angle),
            life, c, sz);
    }
}

// ---------------------------------------------------------------------------
// Spawn boss
// ---------------------------------------------------------------------------
void Game::spawnBoss(){
    delete boss;
    boss = new Boss(WIN_W/2.0f, WIN_H-100.0f, level);
    bossSpawned=true;
    uiMessage = "*** BOSS APPEARS! ***";
    uiMessageTimer=180;
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
                    player.score += e.getCoinValue()*2;
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
                player.score += 500;
                spawnExplosion(boss->x, boss->y, Color(1,0.5f,0), 60);
                spawnExplosion(boss->x-30, boss->y+20, Color(1,0.8f,0), 40);
                spawnExplosion(boss->x+30, boss->y-20, Color(0.8f,0.2f,1), 40);
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
            spawnExplosion(c.x,c.y,Color(1,0.9f,0.1f),6);
        }
    }

    for(auto& f : foods){
        if(!f.active) continue;
        if(f.getAABB().intersects(pa)){
            f.active=false;
            player.foodCollected++;
            spawnExplosion(f.x,f.y,Color(0.3f,1.0f,0.4f),14);
            if(player.foodCollected >= Player::foodForLife){
                player.foodCollected = 0;
                if(player.lives < player.maxLives){
                    player.lives++;
                    uiMessage="\xE2\x98\x85 EXTRA LIFE! \xE2\x98\x85";
                    uiMessageTimer=200;
                    // Triple burst for life gain
                    spawnExplosion(player.x,    player.y,    Color(1.0f,1.0f,0.3f),30);
                    spawnExplosion(player.x-25, player.y+10, Color(0.3f,1.0f,0.3f),15);
                    spawnExplosion(player.x+25, player.y+10, Color(0.3f,0.5f,1.0f),15);
                } else {
                    uiMessage="Lives FULL! +500 Bonus Score";
                    uiMessageTimer=130;
                    player.score += 500;
                    spawnExplosion(player.x, player.y, Color(1,0.9f,0.1f),20);
                }
            } else {
                int rem = Player::foodForLife - player.foodCollected;
                uiMessage="Food! "+std::to_string(rem)+" more for LIFE";
                uiMessageTimer=100;
            }
        }
    }

    for(auto& p : powerups){
        if(!p.active) continue;
        if(p.getAABB().intersects(pa)){
            p.active=false;
            switch(p.type){
                case PowerUpType::FIRE_RATE:
                    player.activateFireRate();
                    uiMessage="FIRE RATE UP!"; uiMessageTimer=120;
                    break;
                case PowerUpType::SHIELD:
                    player.activateShield();
                    uiMessage="SHIELD ACTIVE!"; uiMessageTimer=120;
                    break;
                case PowerUpType::STRONG_BULLET:
                    player.activateStrongBullet();
                    uiMessage="STRONG BULLETS!"; uiMessageTimer=120;
                    break;
            }
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

    // Enemies
    for(auto& e : enemies) e.update();
    enemies.erase(std::remove_if(enemies.begin(),enemies.end(),
        [](const Enemy& e){ return !e.active && e.eggs.empty(); }
    ), enemies.end());

    // Boss
    if(bossSpawned && boss && boss->active)
        boss->update(player.x, player.y);

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

    // Collisions
    handleBulletCollisions();
    handleEggCollisions();
    handlePickups();

    // UI message timeout
    if(uiMessageTimer>0) uiMessageTimer--;

    // Enemy spawn (if wave cleared before boss)
    if(enemies.empty() && !bossSpawned){
        if(++enemySpawnTimer >= 180){
            enemySpawnTimer=0;
            spawnBoss();
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
            state=GameState::GAME_OVER;
            checkEndGameNotification();
            return;
        }
    }

    // Player dead — lose a life or end the game
    if(player.hp <= 0 && player.respawnTimer <= 0){
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
    enemies.clear(); coins.clear(); foods.clear(); powerups.clear();
    enemySpawnRate = std::max(60, 120 - level*15);
    uiMessage = "LEVEL " + std::to_string(level) + " !";
    uiMessageTimer = 180;
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
    }
}
void Game::onKeyUp(unsigned char key){
    switch(key){
        case 'a': case 'A': player.moveLeft  =false; break;
        case 'd': case 'D': player.moveRight =false; break;
        case 'w': case 'W': player.moveUp    =false; break;
        case 's': case 'S': player.moveDown  =false; break;
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
            if(key==' ' || key=='\r' || key==13){
                state=GameState::PLAYING;
                reset();
                spawnWave();
            }
            break;
        case GameState::PLAYING:
            if(key=='p' || key=='P') state=GameState::PAUSED;
            break;
        case GameState::PAUSED:
            if(key=='p' || key=='P') state=GameState::PLAYING;
            if(key=='q' || key=='Q'){ state=GameState::MENU; reset(); }
            break;
        case GameState::GAME_OVER:
        case GameState::WIN:
            if(key==' ' || key=='\r' || key==13){ state=GameState::MENU; reset(); }
            break;
    }
}

// ---------------------------------------------------------------------------
// drawText helpers
// ---------------------------------------------------------------------------
#ifdef __EMSCRIPTEN__
void Game::drawText(float x, float y, const std::string& s, Color c, void* /*font*/){
    renderArcadeText(x, y, s, c, 1.6f);
}

void Game::drawTextLarge(float x, float y, const std::string& s, Color c){
    renderArcadeText(x, y, s, c, 2.6f);
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
    for(auto& s : stars){
        // Twinkle: brightness oscillates with per-star phase offset
        float twinkle = 0.55f + 0.45f*std::abs(std::sin(s.animPhase));
        float alpha = s.brightness * twinkle;
        // Color temperature: fast stars are slightly blue, slow are warm
        float warm = 1.0f - 0.3f*(s.speed-0.3f)/1.7f;
        Color sc(warm*alpha, warm*alpha*0.95f, alpha, alpha);
        sc.apply();
        glPointSize(s.size * (0.8f + 0.4f*twinkle));
        glBegin(GL_POINTS);
            glVertex2f(s.x, s.y);
        glEnd();
        // Add a cross-shaped sparkle for bright large stars
        if(s.size > 2.0f && twinkle > 0.85f){
            float sa = alpha*0.5f;
            Color sparkC(sa,sa,alpha,sa);
            bresenhamLine((int)s.x-2,(int)s.y,(int)s.x+2,(int)s.y, sparkC);
            bresenhamLine((int)s.x,(int)s.y-2,(int)s.x,(int)s.y+2, sparkC);
        }
    }
    glPointSize(1.0f);
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
// drawHUD
// ---------------------------------------------------------------------------
void Game::drawHUD(){
    // HP bar
    float hpFrac=(float)player.hp/player.maxHp;
    drawRect(10,10,200,16, Color(0.2f,0.2f,0.2f));
    Color hpCol=(hpFrac>0.5f)?Color(0.1f,0.9f,0.2f):(hpFrac>0.25f)?Color(1,0.7f,0):Color(0.9f,0.1f,0.1f);
    drawRect(10,10,200*hpFrac,16,hpCol);
    drawRectOutline(10,10,200,16,Color(0.7f,0.7f,0.7f));
    drawText(12,13,"HP: "+std::to_string(player.hp)+"/"+std::to_string(player.maxHp),
        Color(1,1,1), GLUT_BITMAP_HELVETICA_12);

    // Score
    drawText(10,40,"SCORE: "+std::to_string(player.score), Color(1,1,0.3f));

    // Coins
    drawText(10,65,"COINS: "+std::to_string(player.coins), Color(1,0.85f,0));

    // Level
    drawText(10,90,"LEVEL: "+std::to_string(level), Color(0.5f,0.9f,1.0f));

    // ── LIVES display (mini ship icons) ─────────────────────────────────────
    drawText(10, 118, "LIVES:", Color(1.0f,0.55f,0.55f), GLUT_BITMAP_HELVETICA_12);
    for(int i=0;i<player.maxLives;i++){
        float lx = 62.0f + i*30.0f;
        float lcy = 118.0f;
        if(i < player.lives){
            // Active life — filled blue ship silhouette
            std::vector<Vec2> shipMini={
                {lx,      lcy+13},
                {lx-9,    lcy-4},
                {lx-6,    lcy-10},
                {lx+6,    lcy-10},
                {lx+9,    lcy-4}
            };
            scanlineFill(shipMini, Color(0.3f,0.6f,1.0f));
            drawCircle(lx, lcy+4, 4, Color(0.6f,0.9f,1.0f,0.85f));
        } else {
            // Lost life — dark ghost ship
            std::vector<Vec2> shipMini={
                {lx,      lcy+13},
                {lx-9,    lcy-4},
                {lx-6,    lcy-10},
                {lx+6,    lcy-10},
                {lx+9,    lcy-4}
            };
            scanlineFill(shipMini, Color(0.2f,0.2f,0.25f));
        }
    }

    // ── FOOD progress bar toward next life ──────────────────────────────────
    float foodPct = (float)player.foodCollected / (float)Player::foodForLife;
    drawText(10, 148, "FOOD:", Color(0.4f,0.9f,0.4f), GLUT_BITMAP_HELVETICA_12);
    drawRect(52, 145, 96, 11, Color(0.1f,0.18f,0.1f));
    drawRect(52, 145, 96*foodPct, 11, Color(0.25f+0.2f*foodPct, 0.85f, 0.25f));
    drawRectOutline(52, 145, 96, 11, Color(0.4f,0.65f,0.4f));
    drawText(153, 148,
        std::to_string(player.foodCollected)+"/"+std::to_string(Player::foodForLife)+" for LIFE",
        Color(0.7f,1.0f,0.7f), GLUT_BITMAP_HELVETICA_12);

    // Respawn invincibility indicator
    if(player.respawnTimer > 0){
        float blinkAlpha = 0.6f + 0.4f*std::sin(player.respawnTimer*0.3f);
        float tw2 = 130.0f;
        drawRect(WIN_W/2-tw2/2, WIN_H-55, tw2, 18, Color(0,0,0,0.5f*blinkAlpha));
        drawText(WIN_W/2-50, WIN_H-52, "INVINCIBLE!", Color(0.4f,0.9f,1.0f,blinkAlpha),
                 GLUT_BITMAP_HELVETICA_12);
    }

    // Power-up indicators (top-right)
    float px=WIN_W-180;
    if(player.shieldActive){
        drawRect(px,WIN_H-30,80,22, Color(0.1f,0.3f,0.6f,0.7f));
        drawText(px+4,WIN_H-26,"SHIELD", Color(0.4f,0.8f,1.0f));
        px+=90;
    }
    if(player.strongBulletActive){
        drawRect(px,WIN_H-30,100,22, Color(0.5f,0.1f,0.0f,0.7f));
        drawText(px+4,WIN_H-26,"STRONG", Color(1,0.4f,0.1f));
        px+=110;
    }
    if(player.fireRateActive){
        drawRect(px,WIN_H-30,100,22, Color(0.4f,0.4f,0.0f,0.7f));
        drawText(px+4,WIN_H-26,"FAST FIRE", Color(1,1,0.2f));
    }

    // UI message (center)
    if(uiMessageTimer>0){
        float alpha=(float)uiMessageTimer/120.0f;
        if(alpha>1) alpha=1;
        float tw=uiMessage.size()*10.0f;
        drawRect(WIN_W/2-tw/2-8, WIN_H/2-16, tw+16, 28, Color(0,0,0,0.5f*alpha));
        drawTextLarge(WIN_W/2-tw/2, WIN_H/2-8, uiMessage, Color(1,1,0.3f,alpha));
    }

    // Pause hint
    drawText(WIN_W-80,12,"[P] Pause", Color(0.5f,0.5f,0.5f), GLUT_BITMAP_HELVETICA_12);
}

// ---------------------------------------------------------------------------
// drawMenu
// ---------------------------------------------------------------------------
void Game::drawMenu(){
    drawBackground();

    // Title — large text with DDA line decoration
    float ty=WIN_H-120;
    drawText(WIN_W/2-160, ty, "CHICKEN INVADERS", Color(1.0f,0.85f,0.0f),
             GLUT_BITMAP_TIMES_ROMAN_24);
    ddaLine(WIN_W/2-165,(int)(ty-10), WIN_W/2+165,(int)(ty-10), Color(1,0.8f,0));
    ddaLine(WIN_W/2-165,(int)(ty+26), WIN_W/2+165,(int)(ty+26), Color(1,0.8f,0));

    drawText(WIN_W/2-80, WIN_H/2+20,"PRESS SPACE TO START", Color(1,1,1));
    drawText(WIN_W/2-100, WIN_H/2-10,"WASD or Arrow Keys to Move", Color(0.7f,0.7f,0.7f),
             GLUT_BITMAP_HELVETICA_12);
    drawText(WIN_W/2-90, WIN_H/2-28,"Auto-shooting  |  P = Pause", Color(0.7f,0.7f,0.7f),
             GLUT_BITMAP_HELVETICA_12);

    // Draw a decorative chicken
    float t=globalTime;
    float cx=WIN_W/2.0f, cy=WIN_H/2.0f-120;
    Enemy demo(cx+50*std::sin(t*0.5f), cy, EnemyType::NORMAL);
    demo.animTime = t*2;
    demo.wingFlap = 25.0f*std::sin(t*4);
    demo.draw();

    drawText(WIN_W/2-80, 30, "Computer Graphics Project", Color(0.5f,0.5f,0.7f),
             GLUT_BITMAP_HELVETICA_12);
}

// ---------------------------------------------------------------------------
// drawPauseScreen
// ---------------------------------------------------------------------------
void Game::drawPauseScreen(){
    // Dim overlay
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    drawRect(0,0,WIN_W,WIN_H, Color(0,0,0,0.55f));
    glDisable(GL_BLEND);

    drawTextLarge(WIN_W/2-50, WIN_H/2+20,"PAUSED", Color(1,1,0.5f));
    drawText(WIN_W/2-80, WIN_H/2-10,"[P] Resume  |  [Q] Quit to Menu", Color(0.8f,0.8f,0.8f));
}

// ---------------------------------------------------------------------------
// drawGameOver
// ---------------------------------------------------------------------------
void Game::drawGameOver(){
    drawBackground();
    drawRect(WIN_W/2-160, WIN_H/2-50, 320, 120, Color(0.1f,0.0f,0.0f,0.85f));
    drawRectOutline(WIN_W/2-160, WIN_H/2-50, 320, 120, Color(0.9f,0.1f,0.1f), 3.0f);
    drawTextLarge(WIN_W/2-100, WIN_H/2+30,"GAME OVER", Color(0.9f,0.1f,0.1f));
    drawText(WIN_W/2-70, WIN_H/2+5,"Score: "+std::to_string(player.score), Color(1,1,0.5f));
    drawText(WIN_W/2-110, WIN_H/2-15,"PRESS SPACE to return to menu", Color(0.7f,0.7f,0.7f));
    // Explosion particles still animating
    for(auto& p : particles){
        if(!p.active) continue;
        float alpha=p.life/p.maxLife;
        Color pc=p.color; pc.a*=alpha;
        drawCircle(p.x,p.y,p.size*alpha,pc);
    }
}

// ---------------------------------------------------------------------------
// drawWinScreen
// ---------------------------------------------------------------------------
void Game::drawWinScreen(){
    drawBackground();
    drawRect(WIN_W/2-170, WIN_H/2-50, 340, 130, Color(0.0f,0.05f,0.0f,0.85f));
    drawRectOutline(WIN_W/2-170, WIN_H/2-50, 340, 130, Color(0.2f,1.0f,0.3f), 3.0f);
    drawTextLarge(WIN_W/2-100, WIN_H/2+40, "YOU WIN!", Color(0.2f,1.0f,0.3f));
    drawText(WIN_W/2-80, WIN_H/2+12,"Final Score: "+std::to_string(player.score), Color(1,1,0.3f));
    drawText(WIN_W/2-80, WIN_H/2-8, "Coins: "+std::to_string(player.coins), Color(1,0.85f,0));
    drawText(WIN_W/2-110, WIN_H/2-28,"PRESS SPACE to return to menu", Color(0.7f,0.7f,0.7f));
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
            for(auto& p : particles){
                if(!p.active) continue;
                float alpha=p.life/p.maxLife;
                Color pc=p.color; pc.a*=alpha;
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
                drawCircle(p.x,p.y,p.size*alpha,pc);
                glDisable(GL_BLEND);
            }

            drawHUD();

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
