build:
	gcc \
		-O0 -g \
		-std=c99 \
		\
	 	-framework AppKit \
	 	-framework OpenGL \
	 	-framework QuartzCore \
	 	\
	 	main.m \
	 	AppDelegate.m \
	 	OpenGLView.m \
	 	\
	 	sim/*.c \
	 	\
	 	-o quadsim 

clean:
	rm -f ./*.o quadsim
