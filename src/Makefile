CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic
LDFLAGS = -lSDL2 -lSDL2_image

# List all source files explicitly
SRCS = main.cpp \
       imgui.cpp \
       imgui_demo.cpp \
       imgui_draw.cpp \
       imgui_tables.cpp \
       imgui_widgets.cpp \
       imgui_impl_sdl2.cpp \
       imgui_impl_sdlrenderer2.cpp

# Generate object files from source files
OBJS = $(SRCS:.cpp=.o)

all: game

game: $(OBJS)
	$(CXX) $(OBJS) -o game $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) game

.PHONY: all clean
