INCS = .
LIBS = .
libs = -lpthread

ROOTPATH := .
INCPATH := $(ROOTPATH)/inc
SRCPATH := $(ROOTPATH)/src
OBJPATH := $(ROOTPATH)/obj
BINPATH := $(ROOTPATH)/bin

CC := gcc
CPP := g++

SRC := $(wildcard $(SRCPATH)/*.cpp)
OBJ := $(patsubst  %.cpp, %.o, $(SRC))
OBJ := $(patsubst  $(SRCPATH)/%, $(OBJPATH)/%, $(OBJ))
TARGET := app
BIN := $(BINPATH)/$(TARGET)

$(info )
$(info [SRC] = $(SRC))
$(info )
$(info [OBJ] = $(OBJ))
$(info )
$(info [TARGET] = $(TARGET))
$(info -----------------------------------------------------------)

$(BIN) : $(OBJ)
	$(CPP) $^ -o $@ -Wall -L$(LIBS) $(libs)
	
$(OBJPATH)/%.o : $(SRCPATH)/%.cpp
	$(CPP) -c $^ -o $@ -Wall -I$(INCPATH) -I$(INCS) -std=c++11

.PHONY:
clean:
	rm -rf $(OBJ) $(BIN) && echo

