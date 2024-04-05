all:
	g++ \
	-o ./build/linux/crossover_2 \
	./src/core.cpp \
	./src/main.cpp \
	-I./deps/include -L./deps/lib/linux \
	-lraylib -limgui -lGL -lpthread -ldl \
	-O2 -march=native
