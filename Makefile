# Source files
SRC = main.cpp GameManager.cpp \
      UI/menu.cpp UI/input.cpp UI/leaderboard.cpp \
      common/utils.cpp common/game_state.cpp common/timer.cpp common/player.cpp \
      floors/floor1/floor1.cpp floors/floor1/puzzle_game.cpp floors/floor1/rsa_game.cpp \
      floors/floor2/floor2.cpp floors/floor2/tetris_game.cpp floors/floor2/circuit_game.cpp floors/floor2/projection_game.cpp \
      floors/floor3/floor3.cpp floors/floor3/space_shooter.cpp floors/floor3/monster_game.cpp

# Object files
OBJS = $(SRC:.cpp=.o)

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall \
           -Icommon -IUI -Ifloors/floor1 -Ifloors/floor2 -Ifloors/floor3 \
           `pkg-config --cflags sdl2 SDL2_image SDL2_ttf SDL2_mixer`

# SDL flags for linking
SDL_FLAGS = `pkg-config --libs sdl2 SDL2_image SDL2_ttf SDL2_mixer`
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Main game target
escape-room-game: $(OBJS)
	$(CXX) $(OBJS) $(SDL_FLAGS) -o escape-room-game

# Separate build for puzzle_game as executable
floors/floor1/puzzle_game: floors/floor1/puzzle_game.cpp
	$(CXX) $(CXXFLAGS) floors/floor1/puzzle_game.cpp -o floors/floor1/puzzle_game $(SDL_FLAGS)

# Optional: rsa_game as standalone too
floors/floor1/rsa_game: floors/floor1/rsa_game.cpp
	$(CXX) $(CXXFLAGS) floors/floor1/rsa_game.cpp -o floors/floor1/rsa_game $(SDL_FLAGS)

# Clean
clean:
	rm -f $(OBJS) escape-room-game floors/floor1/puzzle_game floors/floor1/rsa_game
