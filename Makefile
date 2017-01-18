all: sample2D

sample2D: angry_birds.cpp glad.c
	g++ -o sample2D angry_birds.cpp glad.c -lGL -lglfw -ldl

clean:
	rm sample2D
