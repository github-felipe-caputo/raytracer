CXX = 			g++
CXXFLAGS = 		-Wall
LDLIBS =		-lsfml-graphics -lsfml-window -lsfml-system

CPP_FILES = main.cpp 
OBJFILES = main.o

main: $(OBJFILES)
	$(CXX) $(CXXFLAGS) -o main $(OBJFILES) $(LDLIBS)

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp $(LDLIBS)

# Dependencies

main.o: canvas.h mathHelper.h object.h world.h camera.h lightSource.h illuminationModel.h

# Clean

clean:
	rm *.o main