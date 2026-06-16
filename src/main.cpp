#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include "Game.h"

// Global game instance
Game* g_game = nullptr;

// Callback functions for GLUT
void displayCallback() {
    if (g_game) {
        g_game->render();
    }
}

void reshapeCallback(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, width, 0, height);
    glMatrixMode(GL_MODELVIEW);
}

void timerCallback(int value) {
    if (g_game && g_game->isRunning()) {
        glutPostRedisplay();
        glutTimerFunc(16, timerCallback, 0);  // ~60 FPS
    }
}

int main(int argc, char** argv) {
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Chicken Shooter - Team Integration");
    
    // Set up OpenGL
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 600);
    glMatrixMode(GL_MODELVIEW);
    
    // Initialize game
    g_game = new Game();
    if (!g_game->initialize()) {
        return 1;
    }
    
    // Set GLUT callbacks
    glutDisplayFunc(displayCallback);
    glutReshapeFunc(reshapeCallback);
    glutTimerFunc(16, timerCallback, 0);
    
    // Main loop
    glutMainLoop();
    
    // Cleanup
    if (g_game) {
        g_game->shutdown();
        delete g_game;
    }
    
    return 0;
}
