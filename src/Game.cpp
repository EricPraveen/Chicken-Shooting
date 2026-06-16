#include "Game.h"
#include "Renderer.h"
#include "FillEngine.h"
#include "TransformEngine.h"
#include "ClipEngine.h"
#include "AnimationEngine.h"
#include "Player.h"
#include "Enemy.h"
#include "Boss.h"
#include "Bullet.h"
#include <OpenGL/gl.h>
#include "UI.h"

// Game coordinator that integrates all five engines:
// 1. Rendering Engine (Member 1) - Bresenham, Circle algorithms
// 2. Filling Engine (Member 2) - Flood fill, boundary fill
// 3. Transformation Engine (Member 3) - Translation, rotation, scaling
// 4. Viewing & Clipping Engine (Member 4) - Window-to-viewport, clipping
// 5. Animation & Physics Engine (Member 5) - Frame sync, animation principles

Game::Game()
    : renderer(nullptr), fillEngine(nullptr), transformEngine(nullptr),
      clipEngine(nullptr), animationEngine(nullptr), running(false),
      screenWidth(800), screenHeight(600) {
}

Game::~Game() {
    shutdown();
}

bool Game::initialize() {
    // Initialize all five engines in integration order
    
    // 1. Initialize Rendering Engine (Member 1)
    renderer = new Renderer();
    
    // 2. Initialize Filling Engine (Member 2)
    fillEngine = new FillEngine();
    
    // 3. Initialize Transformation Engine (Member 3)
    transformEngine = new TransformEngine();
    
    // 4. Initialize Viewing & Clipping Engine (Member 4)
    clipEngine = new ClipEngine();
    clipEngine->setClippingWindow(0, 0, screenWidth, screenHeight);
    clipEngine->setViewport(0, 0, screenWidth, screenHeight);
    
    // 5. Initialize Animation & Physics Engine (Member 5)
    animationEngine = new AnimationEngine();
    animationEngine->initialize(60);  // 60 FPS target
    
    running = true;
    return true;
}

void Game::run() {
    while (running) {
        animationEngine->updateFrame();
        
        handleInput();
        update(animationEngine->getDeltaTime());
        render();
        
        animationEngine->swapBuffers();
    }
}

void Game::update(float deltaTime) {
    // Update game logic using all engines
    
    // This would be where all game objects are updated
    // using the transform engine for movement
}

void Game::render() {
    // Clear screen
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Render using all engines in order
    
    // This would be where all game objects are rendered
    // using the renderer for primitives, fill engine for objects, etc.
    
    if (animationEngine) {
        animationEngine->swapBuffers();
    }
}

void Game::handleInput() {
    // Handle keyboard and mouse input
}

void Game::shutdown() {
    running = false;
    
    if (renderer) delete renderer;
    if (fillEngine) delete fillEngine;
    if (transformEngine) delete transformEngine;
    if (clipEngine) delete clipEngine;
    if (animationEngine) delete animationEngine;
}

bool Game::isRunning() const {
    return running;
}
