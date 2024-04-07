all:
	g++ \
	-std=c++17 \
	-o ./build/linux/crossover_2 \
	./src/crossover_2.cpp \
	./src/geometry.cpp \
	-I./deps/include -L./deps/lib/linux \
	-lraylib -limgui -lGL -lpthread -ldl \
	-O2 -march=native
