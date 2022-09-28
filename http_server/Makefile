CC            = gcc
CFLAGS        = -O2
INCPATH       = -I/usr/local/include/opencv4
LFLAGS        = -Wl,-rpath,/usr/local/lib
LIBS          = -lstdc++ -lpthread -L/usr/local/lib -lopencv_videoio  -lopencv_highgui -lopencv_video -lopencv_imgcodecs -lopencv_imgproc -lopencv_core -lhv -lboost_system 
TARGET        = main

SOURCES       =  main.cpp \
		sha256.cpp \
		sha512.cpp

OBJECTS       =  main.o \
		sha256.o \
		sha512.o

first: all

all: $(TARGET)

build: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(LFLAGS) $(OBJECTS) -o $(TARGET) $(INCPATH) $(LIBS)

main.o: main.cpp
	$(CC) $(CFLAGS) $(LFLAGS) main.cpp -c $(INCPATH) $(LIBS)
	
sha256.o: sha256.cpp
	$(CC) $(CFLAGS) $(LFLAGS) sha256.cpp -c $(INCPATH) $(LIBS)
		
sha512.o: sha512.cpp
	$(CC) $(CFLAGS) $(LFLAGS) sha512.cpp -c $(INCPATH) $(LIBS)

clean:
	rm -rf *.o
