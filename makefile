ASIO_INC = ./asio-1.12.2/include
ASIO = asio-1.12.2
ASIO_TAR = $(ASIO:=.tar.gz)

CC      = g++
CFLAGS  = -O3
OPTION  = -std=c++14 -I$(ASIO_INC) -Wall -g -O0
LIBS    = -lboost_system -lboost_thread -pthread
GTKFLAGS = `/usr/bin/pkg-config gtkmm-3.0 --cflags --libs`

SRC1 = player_main.cpp
SRC2 = dealer.cpp
SRC3 = player_gui.cpp
SRC4 = active_player.cpp
SRC5 = card.cpp
SRC6 = session.cpp

OBJ1 = $(SRC1:.cpp=.o)
OBJ2 = $(SRC2:.cpp=.o)
OBJ3 = $(SRC3:.cpp=.o)
OBJ4 = $(SRC4:.cpp=.o)
OBJ5 = $(SRC5:.cpp=.o)
OBJ6 = $(SRC6:.cpp=.o)

EXE1 = player
EXE2 = dealer

HFILES = player_client.h chat_message.hpp active_player.h card.h json.hpp session.h

all : $(ASIO) $(EXE2) $(EXE1)

$(ASIO):
	tar xzf $(ASIO_TAR)

$(EXE1): $(OBJ1) $(OBJ3) $(OBJ4) $(OBJ5) $(OBJ6)
	$(CC) $(OPTION) -o $(EXE1) $(OBJ1) $(OBJ3) $(OBJ4) $(OBJ5) $(OBJ6) $(GTKFLAGS) $(LIBS)
$(EXE2): $(OBJ2) $(OBJ4) $(OBJ5) $(OBJ6)
	$(CC) $(OPTION) -o $(EXE2) $(OBJ2) $(OBJ4) $(OBJ5) $(OBJ6) $(LIBS)
$(OBJ1): $(SRC1) $(HFILES)
	$(CC) $(OPTION) -c $(SRC1) $(GTKFLAGS) $(LIBS)
$(OBJ2): $(SRC2) $(HFILES)
	$(CC) $(OPTION) -c $(SRC2) $(GTKFLAGS) $(LIBS)
$(OBJ3): $(SRC3) $(HFILES)
	$(CC) $(OPTION) -c $(SRC3) $(GTKFLAGS) $(LIBS)
$(OBJ4): $(SRC4) $(HFILES)
	$(CC) $(OPTION) -c $(SRC4) $(GTKFLAGS) $(LIBS)
$(OBJ5): $(SRC3) $(HFILES)
	$(CC) $(OPTION) -c $(SRC5) $(GTKFLAGS) $(LIBS)
$(OBJ6): $(SRC3) $(HFILES)
	$(CC) $(OPTION) -c $(SRC6) $(GTKFLAGS) $(LIBS)

clean:
	-rm -rf $(ASIO)
	-rm -f $(OBJ1)
	-rm -f $(OBJ2)
	-rm -f $(OBJ3)
	-rm -f $(OBJ4)
	-rm -f $(OBJ5)
	-rm -f $(OBJ6)
	-rm -f $(EXE1)
	-rm -f $(EXE2)
