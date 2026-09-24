NAME        = gomoku

# Compilateur et flags
CXX         = c++
CXXFLAGS    = -Wall -Wextra -Werror -std=c++20 -O3 -Wno-unused-parameter

# Chemins d'include et bibliothèques
INCLUDES    = -Isrc -isystem libs/sfml/include -isystem libs/imgui -isystem libs/imgui-sfml
LDFLAGS     = -Llibs/sfml/lib -Wl,-rpath=./libs/sfml/lib -lsfml-graphics -lsfml-window -lsfml-system -lGL

# Fichiers sources
SRCS        = src/main.cpp \
              libs/imgui/imgui.cpp \
              libs/imgui/imgui_draw.cpp \
              libs/imgui/imgui_tables.cpp \
              libs/imgui/imgui_widgets.cpp \
              libs/imgui-sfml/imgui-SFML.cpp

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

.PHONY: all clean fclean re