all:
	gcc \
	-Wall \
	-o ./boids_demo \
	./src/*.c \
	-I ./include/ -L ./lib/ \
	-lraylib -lpthread -lm -ldl
