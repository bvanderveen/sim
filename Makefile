build:
	gcc -O0 -g -framework AppKit -framework OpenGL -framework QuartzCore main.m AppDelegate.m OpenGLView.m sim.c -o quadsim 

clean:
	rm -f ./*.o quadsim