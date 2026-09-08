# =============================================================================
# Makefile — Chicken Invaders (macOS ARM64 / Apple Silicon)
# =============================================================================
# Build:   make
# Run:     make run
# Clean:   make clean
# =============================================================================

CXX      = clang++
TARGET   = ChickenGame
SRCDIR   = .

# All source files
SRCS     = main.cpp Game.cpp

# Compiler flags
CXXFLAGS = -std=c++17 -O2 -Wall \
           -I$(SRCDIR)

# macOS frameworks (Apple Silicon ARM64)
LDFLAGS  = -framework OpenGL \
           -framework GLUT \
           -Wno-deprecated

# =============================================================================
# Web (Emscripten / WebAssembly) Configuration
# =============================================================================
EMCC        = em++
WEB_DIR     = web
WEB_TARGET  = $(WEB_DIR)/index.html
SHELL_FILE  = $(WEB_DIR)/shell.html

EMCC_FLAGS  = -std=c++17 -O3 \
              -I$(SRCDIR) \
              -s LEGACY_GL_EMULATION=1 \
              -s GL_UNSAFE_OPTS=1 \
              -s EXPORTED_FUNCTIONS="['_main','_wasm_restart_game','_wasm_get_score','_wasm_set_move_up','_wasm_set_move_down','_wasm_set_move_left','_wasm_set_move_right','_wasm_set_shooting','_wasm_press_start']" \
              -s EXPORTED_RUNTIME_METHODS="['ccall','cwrap']" \
              --shell-file $(SHELL_FILE)

# Default desktop target
all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)
	@echo ""
	@echo "✅ Build successful! Run with: ./$(TARGET)"
	@echo ""

run: all
	./$(TARGET)

# WebAssembly target
web: $(SRCS) $(SHELL_FILE)
	@mkdir -p $(WEB_DIR)
	$(EMCC) $(EMCC_FLAGS) $(SRCS) -o $(WEB_TARGET)
	@echo ""
	@echo "✅ WebAssembly build successful!"
	@echo "👉 Run 'make serve' to test locally at http://localhost:8000"
	@echo ""

# Local HTTP test server
serve:
	@echo "🚀 Starting local web server at http://localhost:8000..."
	@echo "👉 Open http://localhost:8000 in your browser. Press Ctrl+C to stop."
	cd $(WEB_DIR) && python3 -m http.server 8000

clean:
	rm -f $(TARGET) $(WEB_DIR)/index.html $(WEB_DIR)/index.js $(WEB_DIR)/index.wasm

.PHONY: all run web serve clean

