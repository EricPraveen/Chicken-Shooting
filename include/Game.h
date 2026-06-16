#ifndef GAME_H
#define GAME_H

// Game coordinator that integrates all five engines in order:
// 1. Rendering Engine (Member 1)
// 2. Filling Engine (Member 2)
// 3. Transformation Engine (Member 3)
// 4. Viewing & Clipping Engine (Member 4)
// 5. Animation & Physics Engine (Member 5)

class Renderer;
class FillEngine;
class TransformEngine;
class ClipEngine;
class AnimationEngine;

class Game {
public:
    Game();
    ~Game();

    // Initialize the game and all engines
    bool initialize();

    // Main game loop
    void run();

    // Update game state
    void update(float deltaTime);

    // Render frame
    void render();

    // Handle input
    void handleInput();

    // Shutdown
    void shutdown();

    // Check if game is running
    bool isRunning() const;

private:
    Renderer* renderer;
    FillEngine* fillEngine;
    TransformEngine* transformEngine;
    ClipEngine* clipEngine;
    AnimationEngine* animationEngine;

    bool running;
    int screenWidth;
    int screenHeight;
};

#endif // GAME_H
