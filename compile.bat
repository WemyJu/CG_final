gcc -DGLEW_STATIC -I./include -c -D__NO_INLINE__ glew.c
g++ -c -std=c++0x -DGLEW_STATIC -I./include Mesh.cpp
g++ -c -std=c++0x -DGLEW_STATIC -I./include Model.cpp
g++ -o final_project -static -O2 -std=c++0x -DGLEW_STATIC -D__NO_INLINE__ main.cpp glew.o Mesh.o Model.o -L./lib/ -lSOIL -I./lib -lglfw3 -lopengl32 -lgdi32 -lassimp
