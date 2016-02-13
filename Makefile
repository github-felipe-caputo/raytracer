CXX = 			g++
CXXFLAGS = 		-Wall
LDLIBS =		-lsfml-graphics -lsfml-window -lsfml-system

CPP_FILES = main.cpp canvas.cpp
OBJFILES = main.o canvas.o

main: $(OBJFILES)
	$(CXX) $(CXXFLAGS) -o main $(OBJFILES) $(LDLIBS)

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp $(LDLIBS)

canvas.o: canvas.cpp
	$(CXX) $(CXXFLAGS) -c canvas.cpp $(LDLIBS)

# Dependencies

main.o: canvas.h
canvas.o: canvas.h

# Clean

clean:
	rm *.o main