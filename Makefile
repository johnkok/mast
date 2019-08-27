CC=clang++
OUT_DIR=build/
SRC_DIR=src/

CFLAGS=-Wall -std=c++14 -pthread -I. -I/usr/include/freetype2 -L. -L/usr/lib/freetype2 -L/usr/local/lib 

DEPS = $(SRC_DIR)mast.hpp \
       $(SRC_DIR)graphics.hpp \
       $(SRC_DIR)windex.hpp \
       $(SRC_DIR)gps.hpp

OBJ = $(SRC_DIR)mast.o \
      $(SRC_DIR)graphics.o \
      $(SRC_DIR)windex.o \
      $(SRC_DIR)gps.o

LIBS= -lstdc++ -ldrawtext -lm -lglut -lGL -lGLU -lfreetype

$(OUT_DIR)%.o: $(SRC_DIR)%.c $(DEPS) $(CFLAG)
	$(CC) -c -o $(OUT_DIR)$@ $< $(CFLAG)

mast: $(OBJ)  $(CFLAG)
	$(CC) -o $(OUT_DIR)$@ $^ $(CFLAGS) $(LIBS)

clean:
	rm $(OUT_DIR)mast ; rm $(SRC_DIR)*.o
