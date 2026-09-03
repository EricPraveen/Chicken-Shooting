// =============================================================================
// main.cpp — Entry Point + GLUT Setup
// =============================================================================
// This file contains the correct main() function signature required by the
// linker on macOS ARM64 (Apple Silicon).
//
// CG Concepts Used Here:
//   Double Buffering (13)       — GLUT_DOUBLE display mode
//   Window-to-Viewport (11)     — glOrtho sets up orthographic projection
//   Timer/Game Loop (60 FPS)    — glutTimerFunc drives update+render
// =============================================================================

#ifdef __EMSCRIPTEN__
#  include <emscripten.h>
#  include <GL/glut.h>
#elif defined(__APPLE__)
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#include "Game.h"

// ---------------------------------------------------------------------------
// GLUT Callbacks
// ---------------------------------------------------------------------------

// CG Concept 13: Double Buffering — display callback draws to back buffer,
// glutSwapBuffers() in Game::display() flips it to the front.
void display_cb(){
    g_game->display();
}

#ifdef __EMSCRIPTEN__
void emscripten_loop(){
    g_game->update();
    g_game->display();
}
#else
// CG Concept 14/Timer: glutTimerFunc drives a fixed 60 FPS game loop.
// update() advances simulation, then display() re-renders.
void timer_cb(int){
    g_game->update();
    glutPostRedisplay();  // request re-render next event cycle
    // Re-register the timer for next frame
    glutTimerFunc(FRAME_MS, timer_cb, 0);
}
#endif

void keyboard_down_cb(unsigned char key, int /*x*/, int /*y*/){
    g_game->onKeyDown(key);
    g_game->onKeyPress(key);
}

void keyboard_up_cb(unsigned char key, int /*x*/, int /*y*/){
    g_game->onKeyUp(key);
}

void special_down_cb(int key, int /*x*/, int /*y*/){
    g_game->onSpecialDown(key);
}

void special_up_cb(int key, int /*x*/, int /*y*/){
    g_game->onSpecialUp(key);
}

void reshape_cb(int w, int h){
    if(h==0) h=1;
    glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // CG Concept 11: Window-to-Viewport Mapping
    // glOrtho maps world coords [0,WIN_W] x [0,WIN_H] → NDC [-1,1]^2
    // This IS the window-to-viewport transformation for 2D.
    glOrtho(0, WIN_W, 0, WIN_H, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// ---------------------------------------------------------------------------
// main() — correct signature required by macOS linker
// ---------------------------------------------------------------------------
int main(int argc, char** argv){
    // CG Concept 1: Raster Graphics Pipeline initialization
    // glutInit sets up the GLUT library and connects to the windowing system.
    glutInit(&argc, argv);

    // CG Concept 13: GLUT_DOUBLE requests double-buffered rendering.
    // GLUT_RGB  — 24-bit color frame buffer
    // GLUT_DOUBLE — two frame buffers (front + back)
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);

    glutInitWindowSize(WIN_W, WIN_H);
    glutInitWindowPosition(100, 80);
    glutCreateWindow("Chicken Invaders — OpenGL CG Project");

    // OpenGL state
    glClearColor(0.0f, 0.0f, 0.05f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPointSize(2.0f);
    glLineWidth(1.5f);

    // CG Concept 11: initial projection setup
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WIN_W, 0, WIN_H, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Create game
    g_game = new Game();

    // Register GLUT callbacks
    glutDisplayFunc(display_cb);
    glutReshapeFunc(reshape_cb);
    glutKeyboardFunc(keyboard_down_cb);
    glutKeyboardUpFunc(keyboard_up_cb);
    glutSpecialFunc(special_down_cb);
    glutSpecialUpFunc(special_up_cb);

#ifdef __EMSCRIPTEN__
    // Emscripten loop: 0 = use requestAnimationFrame, 1 = simulate infinite loop
    emscripten_set_main_loop(emscripten_loop, 0, 1);
#else
    // Start the 60 FPS timer loop
    glutTimerFunc(FRAME_MS, timer_cb, 0);

    // Hand control to GLUT event loop (never returns)
    glutMainLoop();
#endif

    delete g_game;
    return 0;
}
