CC            = gcc
CFLAGS        = -O2
INCPATH       = -I/usr/local/include/opencv4
LFLAGS        = -Wl,-rpath,/usr/local/lib
LIBS          = -lstdc++ -lpthread -L/usr/local/lib -lopencv_videoio  -lopencv_highgui -lopencv_video -lopencv_imgcodecs -lopencv_imgproc -lopencv_core -lhv
TARGET        = main

SOURCES       =  main.cpp \
		worker.cpp

OBJECTS       =  main.o \
		worker.o

first: all

all: $(TARGET)

build: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(LFLAGS) $(OBJECTS) -o $(TARGET) $(INCPATH) $(LIBS)

main.o: main.cpp
	$(CC) $(CFLAGS) $(LFLAGS) main.cpp -c $(INCPATH) $(LIBS)

worker.o: worker.cpp
	$(CC) $(CFLAGS) $(LFLAGS) worker.cpp -c $(INCPATH) $(LIBS)

clean:
	rm -rf *.o
