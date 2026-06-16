# 🎮 Chicken Shooter - Team-Based OpenGL Project

A collaborative computer graphics project designed for 5 team members, each responsible for a distinct graphics pipeline component. This project demonstrates professional-level code organization with clear responsibilities and equal distribution of computer graphics concepts.

---

## 📋 Project Structure

```
ChickenShooter/
│
├── include/                    # Header files
│   ├── Renderer.h             # Member 1: Primitive rendering
│   ├── FillEngine.h           # Member 2: Region filling
│   ├── TransformEngine.h      # Member 3: Transformations
│   ├── ClipEngine.h           # Member 4: Viewing & clipping
│   ├── AnimationEngine.h      # Member 5: Animation & physics
│   ├── Game.h                 # Game coordinator
│   ├── Player.h, Enemy.h, Boss.h, Bullet.h
│   ├── Coin.h, Food.h, Egg.h, PowerUp.h
│   ├── UI.h, Utils.h
│   └── ...
│
├── src/                       # Implementation files
│   ├── main.cpp              # Entry point (Member 5)
│   ├── Game.cpp              # Game loop coordinator
│   ├── Renderer.cpp          # Member 1 implementation
│   ├── FillEngine.cpp        # Member 2 implementation
│   ├── TransformEngine.cpp   # Member 3 implementation
│   ├── ClipEngine.cpp        # Member 4 implementation
│   ├── AnimationEngine.cpp   # Member 5 implementation
│   └── [Game object implementations]
│
├── Makefile                  # Build system with team info
└── README.md                 # This file
```

---

## 👥 Team Member Assignments

### **MEMBER 1 – Rendering & Primitive Engine**

**Computer Graphics Concepts:**
- Bresenham Line Algorithm
- Midpoint Circle Algorithm
- Bresenham Circle Algorithm
- 8-Way Symmetry

**Primary Files:**
- `Renderer.h` / `Renderer.cpp`
- `Player.cpp`
- `Bullet.cpp`

**Responsibilities:**
- Implement Bresenham line drawing algorithm for all lines in the game
- Implement Midpoint Circle algorithm for circular objects
- Implement 8-way symmetry for efficient circle rendering
- Draw player character using primitive shapes
- Draw bullets/projectiles

**Key Methods:**
```cpp
void drawLine(int x0, int y0, int x1, int y1, float r, float g, float b);
void drawCircle(int centerX, int centerY, int radius, ...);
void drawSymmetricPoints(int centerX, int centerY, int x, int y, ...);
```

---

### **MEMBER 2 – Region Filling & Object Styling**

**Computer Graphics Concepts:**
- Scan-Line Fill
- Boundary Fill (4-connected & 8-connected)
- Flood Fill
- 8-Connected Fill

**Primary Files:**
- `FillEngine.h` / `FillEngine.cpp`
- `Enemy.cpp`
- `Coin.cpp`
- `Food.cpp`
- `UI.cpp` (partial)

**Responsibilities:**
- Implement Scan-Line fill for complex polygons
- Implement Boundary Fill algorithms (both 4 and 8-connected)
- Implement Flood Fill for interior coloring
- Fill enemy bodies with appropriate colors
- Fill coin and food objects
- Fill UI elements (health bars, backgrounds)

**Key Methods:**
```cpp
void scanLineFill(int x, int y, int width, int height, ...);
void boundaryFill4(int x, int y, ...);
void boundaryFill8(int x, int y, ...);
void floodFill4/8(int x, int y, ...);
void fillPolygon(int* xPoints, int* yPoints, int pointCount, ...);
```

---

### **MEMBER 3 – Transformation System**

**Computer Graphics Concepts:**
- Translation
- Rotation (around arbitrary pivot points)
- Scaling (non-uniform scaling)
- Composite Transformation (TRS order)

**Primary Files:**
- `TransformEngine.h` / `TransformEngine.cpp`
- `Boss.cpp`
- `Egg.cpp`

**Responsibilities:**
- Implement 2D translation matrix operations
- Implement 2D rotation around arbitrary points
- Implement 2D scaling transformations
- Combine transformations in correct order (Translate → Rotate → Scale)
- Apply transformations to boss character for animation
- Apply transformations to enemy projectiles

**Key Methods:**
```cpp
Vector2D translate(const Vector2D& point, float tx, float ty);
Vector2D rotate(const Vector2D& point, float angleDegrees, const Vector2D& pivotPoint);
Vector2D scale(const Vector2D& point, float scaleX, float scaleY, ...);
Vector2D compositeTransform(const Vector2D& point, float tx, float ty, ...);
```

---

### **MEMBER 4 – Viewing & Clipping System**

**Computer Graphics Concepts:**
- Window-to-Viewport Mapping
- Cohen-Sutherland Line Clipping
- Liang-Barsky Line Clipping
- Sutherland-Hodgman Polygon Clipping

**Primary Files:**
- `ClipEngine.h` / `ClipEngine.cpp`
- `UI.cpp` (partial)

**Responsibilities:**
- Implement window-to-viewport coordinate mapping
- Implement Cohen-Sutherland line clipping algorithm
- Implement Liang-Barsky line clipping algorithm (optimized)
- Implement Sutherland-Hodgman polygon clipping
- Manage camera/viewport transformations
- Optimize rendering by culling off-screen objects
- Handle UI viewport and text clipping

**Key Methods:**
```cpp
void mapWindowToViewport(float& x, float& y);
bool cohenSutherlandClip(Line& line);
bool liangBarskyClip(Line& line);
void sutherlandHodgmanClip(float* xPoints, float* yPoints, int& pointCount);
bool isPointInside(float x, float y) const;
```

---

### **MEMBER 5 – Animation & Physics Engine**

**Computer Graphics Concepts:**
- Double Buffering
- Frame Synchronization
- Squash & Stretch Animation
- Anticipation & Follow-Through
- Periodic Motion Loops

**Primary Files:**
- `AnimationEngine.h` / `AnimationEngine.cpp`
- `main.cpp`
- `Game.cpp`

**Responsibilities:**
- Implement OpenGL double buffering
- Manage frame timing and FPS control
- Implement animation principles (squash & stretch, anticipation)
- Implement periodic motion loops (sine wave oscillations)
- Manage main game loop and rendering pipeline
- Coordinate all five engines
- Handle window creation and event loop

**Key Methods:**
```cpp
void initialize(int targetFPS);
void updateFrame();
float getDeltaTime() const;
float squashAndStretch(float time, float amplitude);
float anticipation(float time, float duration);
float followThrough(float time, float duration, float damping);
float periodicMotion(float time, float frequency, float amplitude);
void swapBuffers();
```

---

## 🔄 Integration Order

The five modules should be integrated in the following order to ensure smooth development and testing:

### **Step 1: Rendering Engine (Member 1)**
- Build and test primitive drawing functions
- Verify Bresenham line and circle algorithms
- Create basic player and bullet rendering

### **Step 2: Filling Engine (Member 2)**
- Integrate with Renderer
- Implement all fill algorithms
- Add enemy, coin, and food object rendering

### **Step 3: Transformation Engine (Member 3)**
- Integrate matrix operations
- Implement movement for all game objects
- Add rotation and scaling effects

### **Step 4: Viewing & Clipping Engine (Member 4)**
- Implement viewport mapping
- Add line clipping algorithms
- Optimize rendering with culling
- Implement UI viewport management

### **Step 5: Animation & Physics Engine (Member 5)**
- Create main game loop
- Integrate frame synchronization
- Implement animation effects
- Coordinate all other engines

---

## 🛠️ Building the Project

### Prerequisites
- macOS with Apple Silicon (or Intel Mac)
- Xcode Command Line Tools
- OpenGL & GLUT frameworks (included in macOS)

### Build Commands

```bash
# Build the project
make

# Run the game
make run

# Clean build artifacts
make clean

# Display team member assignments
make show-assignments
```

### Compilation Details

The Makefile:
- Uses `clang++` compiler with C++17 standard
- Includes the `include/` directory for headers
- Compiles all 19 source files into object files
- Links against OpenGL and GLUT frameworks
- Optimizes for Apple Silicon (ARM64)

---

## 📚 Computer Graphics Concepts Covered

| Concept | Member | Files |
|---------|--------|-------|
| **Bresenham Line Algorithm** | 1 | Renderer.cpp |
| **Midpoint Circle Algorithm** | 1 | Renderer.cpp |
| **8-Way Symmetry** | 1 | Renderer.cpp |
| **Scan-Line Fill** | 2 | FillEngine.cpp |
| **Boundary Fill (4/8-connected)** | 2 | FillEngine.cpp |
| **Flood Fill** | 2 | FillEngine.cpp |
| **Translation** | 3 | TransformEngine.cpp |
| **Rotation** | 3 | TransformEngine.cpp |
| **Scaling** | 3 | TransformEngine.cpp |
| **Composite Transformation** | 3 | TransformEngine.cpp |
| **Window-to-Viewport Mapping** | 4 | ClipEngine.cpp |
| **Cohen-Sutherland Clipping** | 4 | ClipEngine.cpp |
| **Liang-Barsky Clipping** | 4 | ClipEngine.cpp |
| **Sutherland-Hodgman Clipping** | 4 | ClipEngine.cpp |
| **Double Buffering** | 5 | AnimationEngine.cpp |
| **Frame Synchronization** | 5 | AnimationEngine.cpp |
| **Squash & Stretch** | 5 | AnimationEngine.cpp |
| **Anticipation & Follow-Through** | 5 | AnimationEngine.cpp |
| **Periodic Motion** | 5 | AnimationEngine.cpp |

---

## 🎯 Learning Outcomes

Each team member will develop expertise in:

**Member 1:**
- Low-level graphics algorithms
- Pixel-perfect rendering
- Performance optimization for drawing

**Member 2:**
- Interior coloring techniques
- Connected component algorithms
- Memory-efficient fill operations

**Member 3:**
- Linear algebra and transformations
- Matrix operations
- Animation principles

**Member 4:**
- Coordinate system transformations
- Geometric clipping algorithms
- Camera and viewport management

**Member 5:**
- Game architecture and design patterns
- Frame-based animation
- System timing and synchronization

---

## 📝 Development Guidelines

1. **Each member works in their assigned files** - Minimize merge conflicts
2. **Use descriptive variable names** - Foster code readability
3. **Add comments** - Document your graphics algorithms
4. **Test independently** - Unit test your components
5. **Follow C++ best practices** - Use modern C++ features appropriately
6. **Respect API contracts** - Don't change function signatures without discussion

---

## 🎮 Game Features (To Be Implemented)

- Player character with jump mechanics
- Multiple enemy types
- Boss battle sequence
- Collision detection
- Score system with coins/food items
- Health management
- Power-up system
- Pause/Resume functionality
- Main menu
- Game over screen

---

## 📖 References

- **Bresenham Algorithms:** Computer Graphics by Donald D. Hearn and M. Pauline Baker
- **Clipping Algorithms:** Fundamentals of Computer Graphics by Shirley et al.
- **Animation Principles:** The Illusion of Life by Disney Animators
- **OpenGL Documentation:** [khronos.org](https://www.khronos.org/opengl/)

---

## 📞 Contact & Questions

For questions about:
- **Member 1 (Rendering):** Bresenham algorithms, primitive drawing
- **Member 2 (Filling):** Fill algorithms, region coloring
- **Member 3 (Transformations):** Movement, rotation, scaling
- **Member 4 (Clipping):** Viewport, camera, clipping algorithms
- **Member 5 (Animation):** Game loop, frame timing, physics

---

**Project Status:** 🚀 Ready for Team Development

**Last Updated:** 2024

**License:** Educational Use
