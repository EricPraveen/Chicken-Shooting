# =============================================================================
# Makefile – Chicken Shooter (macOS ARM64 / Apple Silicon)
# Team-Based Architecture with 5 Member Contributions
# =============================================================================
# Build:   make
# Run:     make run
# Clean:   make clean
# =============================================================================

CXX         = clang++
TARGET      = ChickenShooter
SRCDIR      = src
INCDIR      = include
OBJDIR      = build

# FreeGLUT from Homebrew
FREEGLUT_PATH = /opt/homebrew/opt/freeglut

# Include paths
CXXFLAGS   = -std=c++17 -g -O0 -Wall -I$(INCDIR) -I/System/Library/Frameworks/OpenGL.framework/Headers

# macOS frameworks (Apple Silicon ARM64)
LDFLAGS    = -framework OpenGL \
             -framework GLUT \
             -Wno-deprecated

# Source files
SOURCES    = $(SRCDIR)/main.cpp \
             $(SRCDIR)/Game.cpp \
             $(SRCDIR)/Renderer.cpp \
             $(SRCDIR)/FillEngine.cpp \
             $(SRCDIR)/TransformEngine.cpp \
             $(SRCDIR)/ClipEngine.cpp \
             $(SRCDIR)/AnimationEngine.cpp \
             $(SRCDIR)/Player.cpp \
             $(SRCDIR)/Enemy.cpp \
             $(SRCDIR)/Boss.cpp \
             $(SRCDIR)/Bullet.cpp \
             $(SRCDIR)/Coin.cpp \
             $(SRCDIR)/Food.cpp \
             $(SRCDIR)/Egg.cpp \
             $(SRCDIR)/PowerUp.cpp \
             $(SRCDIR)/UI.cpp \
             $(SRCDIR)/Utils.cpp

# Object files
OBJECTS    = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

# Default target
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo ""
	@echo "✅ Build successful!"
	@echo "📦 Target: $(TARGET)"
	@echo "👥 Team Members: 5"
	@echo "🎮 Run with: ./$(TARGET)"
	@echo ""

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: all
	./$(TARGET)

clean:
	rm -rf $(OBJDIR) $(TARGET)
	@echo "✅ Clean complete"

# Team member assignments
show-assignments:
	@echo ""
	@echo "=== Chicken Shooter - Team Member Assignments ==="
	@echo ""
	@echo "MEMBER 1 – Rendering & Primitive Engine"
	@echo "  Concepts: Bresenham Line, Midpoint Circle, Bresenham Circle, 8-Way Symmetry"
	@echo "  Files: Renderer.h/cpp, Player.cpp, Bullet.cpp"
	@echo ""
	@echo "MEMBER 2 – Region Filling & Object Styling"
	@echo "  Concepts: Scan-Line Fill, Boundary Fill, Flood Fill, 8-Connected Fill"
	@echo "  Files: FillEngine.h/cpp, Enemy.cpp, Coin.cpp, Food.cpp"
	@echo ""
	@echo "MEMBER 3 – Transformation System"
	@echo "  Concepts: Translation, Rotation, Scaling, Composite Transformation"
	@echo "  Files: TransformEngine.h/cpp, Boss.cpp, Egg.cpp"
	@echo ""
	@echo "MEMBER 4 – Viewing & Clipping System"
	@echo "  Concepts: Window-to-Viewport, Cohen-Sutherland, Liang-Barsky, Sutherland-Hodgman"
	@echo "  Files: ClipEngine.h/cpp, UI.cpp"
	@echo ""
	@echo "MEMBER 5 – Animation & Physics Engine"
	@echo "  Concepts: Double Buffering, Frame Sync, Squash & Stretch, Anticipation, Periodic Motion"
	@echo "  Files: AnimationEngine.h/cpp, Game.cpp, main.cpp"
	@echo ""

# Target for the complete game
ChickenGame: main.cpp Game.cpp
	$(CXX) -std=c++17 -O2 main.cpp Game.cpp -o ChickenGame -framework OpenGL -framework GLUT -Wno-deprecated

run-complete: ChickenGame
	./ChickenGame

.PHONY: all run clean show-assignments run-complete
