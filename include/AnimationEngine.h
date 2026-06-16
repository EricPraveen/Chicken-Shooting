#ifndef ANIMATIONENGINE_H
#define ANIMATIONENGINE_H

// Member 5 – Animation & Physics Engine
// Computer Graphics Concepts:
// - Double Buffering
// - Frame Synchronization
// - Squash & Stretch
// - Anticipation & Follow-Through
// - Periodic Motion Loops

#include <chrono>

class AnimationEngine {
public:
    AnimationEngine();
    ~AnimationEngine();

    // Initialize animation engine
    void initialize(int targetFPS = 60);

    // Update frame timing and synchronization
    void updateFrame();

    // Get delta time (time since last frame)
    float getDeltaTime() const;

    // Get current frame time
    double getFrameTime() const;

    // Get FPS
    int getFPS() const;

    // Squash and stretch animation
    float squashAndStretch(float time, float amplitude = 0.2f);

    // Anticipation effect
    float anticipation(float time, float duration);

    // Follow-through effect
    float followThrough(float time, float duration, float damping = 0.95f);

    // Periodic motion loop (sine wave)
    float periodicMotion(float time, float frequency, float amplitude);

    // Enable/disable double buffering
    void enableDoubleBuffering(bool enable);

    // Swap buffers (for double buffering)
    void swapBuffers();

    // Wait for next frame based on target FPS
    void waitForNextFrame();

private:
    std::chrono::high_resolution_clock::time_point lastFrameTime;
    float deltaTime;
    float frameTime;
    int targetFPS;
    int currentFPS;
    int frameCount;
    bool doubleBufferingEnabled;
    std::chrono::high_resolution_clock::time_point fpsUpdateTime;
};

#endif // ANIMATIONENGINE_H
