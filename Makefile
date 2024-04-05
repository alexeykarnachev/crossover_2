all:
	g++ \
	-o ./build/linux/crossover_2 \
	./src/core.cpp \
	./src/main.cpp \
	-I./deps/include -L./deps/lib/linux \
	-lraylib -limgui -lbox2d -lGL -lpthread -ldl \
	-O2 -march=native
