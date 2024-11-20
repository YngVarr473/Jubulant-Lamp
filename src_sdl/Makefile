CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic -I/usr/include/GL -I/usr/include/python3.12
LDFLAGS = -lSDL2 -lSDL2_image -lGLEW -lGL -lpython3.12

# List all source files explicitly
SRCS = main.cpp \
       imgui/imgui.cpp \
       imgui/imgui_demo.cpp \
       imgui/imgui_draw.cpp \
       imgui/imgui_tables.cpp \
       imgui/imgui_widgets.cpp \
       imgui/imgui_impl_sdl2.cpp \
       imgui/imgui_impl_sdlrenderer2.cpp \

# Generate object files from source files
OBJS = $(SRCS:.cpp=.o)

# Enable parallel compilation
MAKEFLAGS += -j$(shell nproc)

all: game

game: $(OBJS)
	$(CXX) $(OBJS) -o game $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) game

.PHONY: all clean
