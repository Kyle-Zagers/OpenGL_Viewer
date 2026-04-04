all:
	@mkdir -p ./build
	g++ src/main.cpp src/*.h -o ./build/OpenGL_Viewer -lGLEW -lglfw -lGL -lGLU