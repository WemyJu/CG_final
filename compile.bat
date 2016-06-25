gcc -DGLEW_STATIC -g3 -I. -c -D__NO_INLINE__ glew.c
g++ -c -std=c++0x -g3 -DGLEW_STATIC -I. Mesh.cpp
g++ -c -std=c++0x -g3 -DGLEW_STATIC -I. Model.cpp
g++ -o final_project -static -O2 -std=c++0x -g3 -DGLEW_STATIC -D__NO_INLINE__ main.cpp glew.o Mesh.o Model.o -L./lib/ -lSOIL -I. -lglfw3 -lopengl32 -lgdi32 -lassimp
