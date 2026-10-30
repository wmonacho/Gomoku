NAME        = Gomoku

# Compilateur et flags
CXX         = c++
CXXFLAGS    = -Wall -Wextra -Werror -std=c++20 -O3 -Wno-unused-parameter

# Chemins d'include et bibliothèques
INCLUDES    = -Isrc -isystem libs/sfml/include -isystem libs/imgui -isystem libs/imgui-sfml
LDFLAGS     = -Llibs/sfml/lib -Wl,-rpath=./libs/sfml/lib -lsfml-graphics -lsfml-window -lsfml-system -lGL

# Fichiers sources
SRCS        = src/main.cpp \
              src/app/GameService.cpp \
              src/core/Board.cpp \
              src/core/Rules.cpp \
              src/core/Game.cpp \
              src/core/ai/Search.cpp \
              src/core/ai/Eval.cpp \
              libs/imgui/imgui.cpp \
              libs/imgui/imgui_draw.cpp \
              libs/imgui/imgui_tables.cpp \
              libs/imgui/imgui_widgets.cpp \
              libs/imgui-sfml/imgui-SFML.cpp \
			  src/ui/mainMenu.cpp \
              src/ui/BoardView.cpp 

# Dossier d'objets et mapping des fichiers .o
OBJ_DIR     = obj
OBJS        = $(SRCS:%.cpp=$(OBJ_DIR)/%.o)

all: $(NAME)

# Règle de compilation des objets avec création automatique du sous-dossier
$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Règle d'édition des liens
$(NAME): $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $(NAME)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

# Tests du moteur : seulement core/, sans SFML. Chaque tests/*Test.cpp donne un binaire.
TEST_DIR     = $(OBJ_DIR)/tests
TEST_BINS    = $(patsubst tests/%.cpp,$(TEST_DIR)/%,$(wildcard tests/*Test.cpp))
TEST_SUPPORT = $(wildcard tests/support/*.cpp)
CORE_SRCS    = $(filter src/core/%,$(SRCS))

$(TEST_DIR)/%: tests/%.cpp $(TEST_SUPPORT) $(CORE_SRCS)
	@mkdir -p $(TEST_DIR)
	$(CXX) $(CXXFLAGS) -Isrc -Itests $^ -o $@

test: $(TEST_BINS)
	@for test in $(TEST_BINS); do ./$$test || exit 1; done

.PHONY: all clean fclean re test
