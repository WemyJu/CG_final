gcc -DGLEW_STATIC -g3 -I. -c glew.c
g++ -c -std=c++0x -g3 -DGLEW_STATIC Mesh.cpp
g++ -c -std=c++0x -g3 -DGLEW_STATIC Model.cpp
g++ -o HW4 -static -O2 -std=c++0x -g3 -DGLEW_STATIC main.cpp tiny_obj_loader.cc glew.o Mesh.o Model.o -L./lib/ -lSOIL -I. -lglfw3 -lopengl32 -lgdi32 -lassimp
