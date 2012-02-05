build:
	gcc -std=c99 -O0 -g -framework AppKit -framework OpenGL -framework QuartzCore main.m AppDelegate.m OpenGLView.m sim/sim.c -o quadsim 

clean:
	rm -f ./*.o quadsim
