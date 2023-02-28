# Project Structure
# cjong
# ├ build
# │ ├ cjong
# │ └ simulation
# ├ debug
# │ ├ cjong_dbg
# │ ├ test_game
# │ └ test_json
# ├ gamelog
# │ ├ 2023_1_1_Jade_Room_South.json
# │ ├ 2023020601gm-0009-0000-33fbd68e.log
# │ └ ...
# ├ include/cli
# | ├ cli.h
# | └ ...
# ├ index
# │ ├ index_dw_h.txt
# │ └ index_dw_s.txt
# ├ obj
# │ └ *.o
# ├ src
# │ ├ calsht_dw.cpp
# │ ├ calsht_dw.hpp
# │ ├ cjong.cpp
# │ ├ constant.hpp
# │ ├ conversion.cpp
# │ ├ conversion.hpp
# │ ├ game.cpp
# │ ├ game.hpp
# │ ├ hand.cpp
# │ ├ hand.hpp
# │ ├ judwin.cpp
# │ ├ judwin.hpp
# │ ├ kyoku.cpp
# │ ├ kyoku.hpp
# │ ├ mkind2.cpp
# │ ├ sample.cpp
# │ ├ simulation.cpp
# │ ├ test_game.cpp
# │ └ test_json.cpp
# └ tex
#   └ cjong.tex

#===================== Name of directories and executables =====================

EXEC = cjong
B_DIR = build
D_DIR = debug
O_DIR = obj
B_EXEC = $(B_DIR)/$(EXEC)
D_EXEC = $(D_DIR)/$(EXEC)
S_DIR = src
I_DIR = /opt/local/include include
IND_DIR = index

H_FILE := $(wildcard $(S_DIR)/*.hpp)
# change .hpp part to .cpp in H_FILE
C_FILE := $(patsubst %.hpp, %.cpp, $(H_FILE))
# filter out constant.cpp in C_FILE
C_FILE := $(filter-out $(S_DIR)/constant.cpp, $(C_FILE))
#O_FILE := $(addprefix $(O_DIR)/,$(addsuffix .o,$(basename $(notdir $(C_FILE)))))

CJONG_CPP = $(S_DIR)/$(EXEC).cpp

IND_FILE := index_dw_h.txt index_dw_s.txt
IND_FILE := $(addprefix $(IND_DIR)/, $(IND_FILE))

# subset of files related to simulation.cpp
C_FILE_SIM := calsht_dw hand simulation
C_FILE_SIM := $(addprefix $(S_DIR)/, $(addsuffix .cpp, $(C_FILE_SIM)))

CC = g++
CFLAGS = --std=c++17 -Wall -lcurl -DINDEX_PATH=\"index\"
DFLAGS = --std=c++17 -Wall -g3 -O0 -lcurl -DINDEX_PATH="index" -DDEBUG
IFLAGS = $(addprefix -I, $(I_DIR))

SHELL = /bin/sh
MAKE = make

.PHONY = all dbg clean sim test_game

#=================================== Targets ===================================

all: $(B_DIR)/$(EXEC)
	ln -fs $(B_DIR)/$(EXEC) .
	$(info A symbolic link `cjong` to the executable has been created in current directory.)

# update rule of executable depends on CPP files and existence of index files
$(B_DIR)/$(EXEC): $(C_FILE) $(H_FILE) $(CJONG_CPP) | $(IND_DIR) $(IND_FILE) $(B_DIR)
	$(CC) $(CFLAGS) $(IFLAGS) $(C_FILE) $(CJONG_CPP) -o $(B_DIR)/$(EXEC)

# update rule that only runs when the targets are non-existent
$(IND_DIR) $(IND_FILE):
	mkdir index
	tar zxvf index_dw.tar.gz
	mv index_dw_h.txt index
	mv index_dw_s.txt index

$(B_DIR):
	mkdir $(B_DIR)

sim: $(S_DIR)/simulation.cpp
	$(CC) $(CFLAGS) $(IFLAGS) $(C_FILE_SIM) -o $(B_DIR)/simulation

dbg: $(D_DIR)/$(EXEC)

$(D_DIR)/$(EXEC): $(C_FILE) $(H_FILE) $(CJONG_CPP) | $(IND_DIR) $(IND_FILE) $(D_DIR)
	$(CC) $(DFLAGS) $(IFLAGS) $(C_FILE) $(CJONG_CPP) -o $(D_DIR)/$(EXEC)

$(D_DIR):
	mkdir $(D_DIR)

test_game: $(D_DIR)/test_game

$(D_DIR)/test_game: $(S_DIR)/test_game.cpp $(H_FILE) | $(D_DIR)
	$(CC) $(DFLAGS) $(IFLAGS) $(C_FILE) $(S_DIR)/test_game.cpp -o $(D_DIR)/test_game

test_json: $(D_DIR)/test_json

$(D_DIR)/test_json: $(S_DIR)/test_json.cpp  $(H_FILE) | $(D_DIR)
	$(CC) $(DFLAGS) $(IFLAGS) $(C_FILE) $(S_DIR)/test_json.cpp -o $(D_DIR)/test_json

clean:
	rm -r build/*
	rm -r debug/*
