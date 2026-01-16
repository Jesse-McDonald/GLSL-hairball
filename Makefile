LDLIBS=-lglut -lGLEW -lGL -lm -g
all: hair
clean:
	rm -f *.o hair
hair: ./common/shader_utils.o
.PHONY: all clean
