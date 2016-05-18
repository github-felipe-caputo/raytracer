CC =		    gcc
CCFLAGS = 		#-Wall
CXX = 			g++
CXXFLAGS = 		-Wall
LDLIBS =		-lsfml-graphics -lsfml-window -lsfml-system

CPP_FILES = main.cpp 
OBJFILES = main.o plyfile.o

main: $(OBJFILES)
	$(CXX) $(CXXFLAGS) -o main $(OBJFILES) $(LDLIBS)

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp $(LDLIBS)

# ply.h is a a file in c, the code here is different
# no warning here because the library is full of small
# little depreciated raning things
plyfile.o: libs/plyfile.c
	$(CC) $(CCFLAGS) -c libs/plyfile.c

# Dependencies

main.o: canvas.h mathHelper.h object.h world.h camera.h lightSource.h illuminationModel.h texture.h kdtree.h toneReproduction.h readPly.h

# Clean

clean:
	rm *.o main