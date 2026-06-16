#ifndef FILLENGINE_H
#define FILLENGINE_H

// Member 2 – Region Filling & Object Styling
// Computer Graphics Concepts:
// - Scan-Line Fill
// - Boundary Fill
// - Flood Fill
// - 8-Connected Fill

class FillEngine {
public:
    FillEngine();
    ~FillEngine();

    // Scan-line fill algorithm
    void scanLineFill(int x, int y, int width, int height,
                      float fillR, float fillG, float fillB);

    // Boundary fill algorithm (4-connected)
    void boundaryFill4(int x, int y,
                       float boundaryR, float boundaryG, float boundaryB,
                       float fillR, float fillG, float fillB);

    // Boundary fill algorithm (8-connected)
    void boundaryFill8(int x, int y,
                       float boundaryR, float boundaryG, float boundaryB,
                       float fillR, float fillG, float fillB);

    // Flood fill algorithm (4-connected)
    void floodFill4(int x, int y,
                    float originalR, float originalG, float originalB,
                    float fillR, float fillG, float fillB);

    // Flood fill algorithm (8-connected)
    void floodFill8(int x, int y,
                    float originalR, float originalG, float originalB,
                    float fillR, float fillG, float fillB);

    // Fill polygon with scan-line fill
    void fillPolygon(int* xPoints, int* yPoints, int pointCount,
                     float fillR, float fillG, float fillB);

private:
    void fillConnected4(int x, int y,
                        float origR, float origG, float origB,
                        float fillR, float fillG, float fillB);

    void fillConnected8(int x, int y,
                        float origR, float origG, float origB,
                        float fillR, float fillG, float fillB);
};

#endif // FILLENGINE_H
