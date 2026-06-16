#include "AnimationEngine.h"
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

// Member 5 – Animation & Physics Engine
// Double Buffering, Frame Synchronization, Squash & Stretch, Animation Principles

AnimationEngine::AnimationEngine()
    : deltaTime(0.0f), frameTime(0.0), targetFPS(60), currentFPS(0),
      frameCount(0), doubleBufferingEnabled(true) {
    lastFrameTime = std::chrono::high_resolution_clock::now();
    fpsUpdateTime = lastFrameTime;
}

AnimationEngine::~AnimationEngine() {
}

void AnimationEngine::initialize(int fps) {
    targetFPS = fps;
    lastFrameTime = std::chrono::high_resolution_clock::now();
    fpsUpdateTime = lastFrameTime;
}

void AnimationEngine::updateFrame() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastFrameTime);
    deltaTime = duration.count() / 1000000.0f;
    frameTime = deltaTime;
    lastFrameTime = currentTime;

    // Update FPS counter
    frameCount++;
    auto fpsDuration = std::chrono::duration_cast<std::chrono::seconds>(currentTime - fpsUpdateTime);
    if (fpsDuration.count() >= 1) {
        currentFPS = frameCount;
        frameCount = 0;
        fpsUpdateTime = currentTime;
    }
}

float AnimationEngine::getDeltaTime() const {
    return deltaTime;
}

double AnimationEngine::getFrameTime() const {
    return frameTime;
}

int AnimationEngine::getFPS() const {
    return currentFPS;
}

float AnimationEngine::squashAndStretch(float time, float amplitude) {
    // Squash and stretch animation effect
    // Uses sinusoidal motion to create compression/expansion effect
    return 1.0f + amplitude * sinf(time * 6.28f); // 2*pi frequency
}

float AnimationEngine::anticipation(float time, float duration) {
    // Anticipation effect - slow start that accelerates
    float t = time / duration;
    if (t > 1.0f) t = 1.0f;
    return t * t; // Quadratic easing
}

float AnimationEngine::followThrough(float time, float duration, float damping) {
    // Follow-through effect with damping
    float t = time / duration;
    if (t > 1.0f) t = 1.0f;
    return (1.0f - powf(1.0f - t, 2.0f)) * powf(damping, time);
}

float AnimationEngine::periodicMotion(float time, float frequency, float amplitude) {
    // Periodic motion loop - sine wave oscillation
    return amplitude * sinf(time * frequency);
}

void AnimationEngine::enableDoubleBuffering(bool enable) {
    doubleBufferingEnabled = enable;
}

void AnimationEngine::swapBuffers() {
    // Swap OpenGL buffers for double buffering
    if (doubleBufferingEnabled) {
        glutSwapBuffers();
    }
}

void AnimationEngine::waitForNextFrame() {
    // Wait to maintain target FPS
    auto targetFrameTime = 1.0f / targetFPS;
    while (getDeltaTime() < targetFrameTime) {
        // Busy wait or sleep
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastFrameTime);
        float elapsed = duration.count() / 1000000.0f;
        
        if (elapsed >= targetFrameTime) break;
    }
}
