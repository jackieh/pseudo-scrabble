TOP_DIR = .
SRC_DIR = $(TOP_DIR)/src

CXX = g++ -std=c++17 -g -Wall -Wextra -Wno-unused-but-set-variable

# Have all .cpp files built into .o files
# to be linked into a library or executable.
%.o: %.cpp
	$(CXX) $(BUILD_FLAGS) -o $@ -c $<

# These files are created by the build.
LIB_OUT = libpseudoscrabble.a
BIN_OUT = pseudoscrabble

.PHONY: default
default: $(LIB_OUT) $(BIN_OUT)

.PHONY: clean
clean:
	rm -f $(SRC_DIR)/*.o
	rm -f $(LIB_OUT)
	rm -f $(BIN_OUT)

LIB_SRC = \
		  $(SRC_DIR)/board_state.cpp \
		  $(SRC_DIR)/word_validator.cpp
LIB_OBJ = $(LIB_SRC:.cpp=.o)
$(LIB_OBJ): BUILD_FLAGS := -I $(SRC_DIR)

$(LIB_OUT): $(LIB_OBJ)
	rm -rf $@
	ar cq $@ $(LIB_OBJ)

BIN_SRC = $(SRC_DIR)/main.cpp
BIN_OBJ = $(BIN_SRC:.cpp=.o)
$(BIN_OBJ): BUILD_FLAGS := -I $(SRC_DIR) -DEXEC_NAME=\"$(BIN_OUT)\"

$(BIN_OUT): $(LIB_OUT) $(BIN_OBJ)
	$(CXX) -o $@ $(BIN_OBJ) -L$(TOP_DIR) \
		-lboost_program_options -lpseudoscrabble -laspell
