EXEC = HW4
.PHONY: all
all: $(EXEC)

CXXFLAGS = -I./include -std=c++0x -DGLEW_STATIC
CFLAGS = -I./include -DGLEW_STATIC
# If you can't compile, use this line instead
#LFLAGS = -lGL -lglfw3 -lX11 -lXxf86vm -lXinerama -lXrandr -lpthread -lXi -lXcursor -ldl
LFLAGS = `pkg-config glfw3 --libs --static` -g3 -lSOIL -L/opt/local/lib -lassimp -framework OpenGL

OBJS := \
	main.o \
	glew.o \
	Mesh.o \
	Model.o

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<
%.o: %.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(EXEC): $(OBJS)
	$(CXX) -o $@ $^ $(LFLAGS)

clean:
	rm -rf $(OBJS) $(EXEC)
