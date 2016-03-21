#
# AUTH:			Federico Francescon
# MATR:			146995
# LAUREA:		LT Informatica 2013/2014
# CORSO:		Sistemi Operativi 1
# 
# Progetto:	Football Tournament
#

# Compiler Options
CC = gcc
CCFLAGS = -lpthread -lrt

# Project files and folders
SRC_F = ./src
BIN_F = ./bin
BINFN = FootballTournament
AST_F = ./ast
ASTFN = config.txt
OUT_F = ./out
OUTFN = testOutput.txt

# Files passed to compiler
COMPILE_OBJECTS = $(SRC_F)/FootballTournament.c $(SRC_F)/libTournament.c $(SRC_F)/sharedFunc.c

# Declaring all targets Override problem with folder named equal to a target ( target bin and folder bin)
.PHONY: all bin assets test clean cleanBIN cleanAST cleanOUT

# TARGET: all
# Print a small help about makefile
all:
	@echo "___________________"
	@echo "Project Football Tournament ( FT )"
	@echo "This software creates and plays a Football Tournament, with your own defined teams."
	@echo ""
	@echo "Available make targets are:"
	@echo " » all       Prints this help"
	@echo " » bin       Compiles source files"
	@echo " » assets    Generate a FT config file example"
	@echo " » test      Compiles, generate assets and test FT"
	@echo " » clean     Cleans BIN folder and ASSETS folder"
	@echo " » cleanBIN  Cleans BIN folder"
	@echo " » cleanAST  Cleans ASSETS folder"
	@echo " » cleanOUT  Cleans OUTPUT folder ( when used for testing )"
	@echo ""

# TARGET: bin
# Compiles FT
bin:
	@make cleanBIN
	@echo "»»»"
	@echo "»»» COMPILING TO $(BIN_F)/$(BINFN)"
	@echo "»»»"
	@if [ ! -d $(BIN_F) ]; then mkdir $(BIN_F); fi
	$(CC) -o $(BIN_F)/$(BINFN) $(COMPILE_OBJECTS) $(CCFLAGS)

# TARGET: assets
# Create example log file to test FT
assets:
	@make cleanAST
	@echo "»»»"
	@echo "»»» CREATING ASSET: $(AST_F)/$(ASTFN)"
	@echo "»»»"
	@if [ ! -d $(AST_F) ]; then mkdir $(AST_F); fi
	@if [ -f $(AST_F)/$(ASTFN) ]; then rm $(AST_F)/$(ASTFN); fi
	@echo "# TEAMS N°" >> $(AST_F)/$(ASTFN)
	@echo "8" >> $(AST_F)/$(ASTFN)
	@echo "" >> $(AST_F)/$(ASTFN)
	@echo "# TEAMS" >> $(AST_F)/$(ASTFN)
	@echo "Marianna 10" >> $(AST_F)/$(ASTFN)
	@echo "Regina 12" >> $(AST_F)/$(ASTFN)
	@echo "Molly 4" >> $(AST_F)/$(ASTFN)
	@echo "Sandra 5" >> $(AST_F)/$(ASTFN)
	@echo "Fernando 6" >> $(AST_F)/$(ASTFN)
	@echo "Federico 10" >> $(AST_F)/$(ASTFN)
	@echo "Max 1" >> $(AST_F)/$(ASTFN)
	@echo "Baghera 12" >> $(AST_F)/$(ASTFN)

# TARGET: test
# Compile, create test files and execute FT
test:
	@make bin
	@make assets
	@echo "»»»"
	@echo "»»» TESTING APPLICATION"
	@echo "»»»"
	@if [ ! -d $(OUT_F) ]; then mkdir $(OUT_F); fi
	@$(BIN_F)/FootballTournament -c $(AST_F)/$(ASTFN) -o $(OUT_F)/$(OUTFN)

# TARGET: clean
# Clean, BIN folder, ASSETS folder and OUTPUT folder
clean:
	@make cleanBIN
	@make cleanAST
	@make cleanOUT

# TARGET: cleanBIN
# Clean BIN folder
cleanBIN:
	@echo "»»»"
	@echo "»»» CLEANING BIN FOLDER: $(BIN_F)"
	@echo "»»»"
	@if [ -d $(BIN_F) ]; then rm -f $(BIN_F)/*; fi

# TARGET: cleanAST
# Clean ASSETS folder
cleanAST:
	@echo "»»»"
	@echo "»»» CLEANING ASSETS FOLDER: $(AST_F)"
	@echo "»»»"
	@if [ -d $(AST_F) ]; then rm -rf $(AST_F)/*; fi

# TARGET: cleanOUT
# Clean OUT folder
cleanOUT:
	@echo "»»»"
	@echo "»»» CLEANING OUTPUT FOLDER: $(OUT_F)"
	@echo "»»»"
	@if [ -d $(OUT_F) ]; then rm -rf $(OUT_F)/*; fi
