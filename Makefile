CC = gcc

CCWARNINGS = -W -Wall -Wno-unused-parameter -Wno-unused-variable \
		-Wno-unused-function
CCOPTS     = -g -O0 

CFLAGS = $(CCWARNINGS) $(CCOPTS)

LIB_SOURCES = aq_tsafe.c
LIB_OBJECTS = $(LIB_SOURCES:.c=.o)
LIB         = aq
LIB_DIR     = mylib
LIB_NAME     = lib$(LIB).a

LIB_SEQ_SOURCES = aq_seq.c
LIB_SEQ_OBJECTS = $(LIB_SEQ_SOURCES:.c=.o)
LIB_SEQ         = aq_seq
LIB_SEQ_NAME    = lib$(LIB_SEQ).a

DEMO_SOURCES = aq_demo.c _aux.c
DEMO_OBJECTS = $(DEMO_SOURCES:.c=.o)

TEST_SOURCES = aq_test.c _aux.c
TEST_OBJECTS = $(TEST_SOURCES:.c=.o)

TEST1_SOURCES = test1.c _aux.c
TEST1_OBJECTS = $(TEST1_SOURCES:.c=.o)

TEST2_SOURCES = test2.c _aux.c
TEST2_OBJECTS = $(TEST2_SOURCES:.c=.o)

DEMO_EXECUTABLE = demo_aq
TEST_EXECUTABLE = test_aq
TEST1_EXECUTABLE = test1
TEST2_EXECUTABLE = test2

EXECUTABLES = $(DEMO_EXECUTABLE) $(TEST_EXECUTABLE) $(TEST1_EXECUTABLE) $(TEST2_EXECUTABLE)

.PHONY:  all lib lib-seq demo test test1 test2 clean clean-all

all: lib lib-seq demo test test1 test2

lib-seq: $(LIB_DIR)/$(LIB_SEQ_NAME)
 
lib: $(LIB_DIR)/$(LIB_NAME)

demo: lib-seq $(DEMO_EXECUTABLE)

test: lib $(TEST_EXECUTABLE)

test1: lib $(TEST1_EXECUTABLE)

test2: lib $(TEST2_EXECUTABLE)

%.o: %.c 
	$(CC) $(CFLAGS) -c $< -o $@

$(LIB_DIR)/$(LIB_NAME): $(LIB_OBJECTS)
	mkdir -p $(LIB_DIR)
	ar -rcs $@ $^

$(LIB_DIR)/$(LIB_SEQ_NAME): $(LIB_SEQ_OBJECTS)
	mkdir -p $(LIB_DIR)
	ar -rcs $@ $^

$(DEMO_EXECUTABLE): $(DEMO_OBJECTS)
	$(CC) $(CFLAGS) $(DEMO_OBJECTS) -L$(LIB_DIR) -l$(LIB_SEQ) -o $@ 

$(TEST_EXECUTABLE): $(TEST_OBJECTS)
	$(CC) $(CFLAGS) $(TEST_OBJECTS) -lpthread -L$(LIB_DIR) -l$(LIB) -o $@ 

$(TEST1_EXECUTABLE): $(TEST1_OBJECTS)
	$(CC) $(CFLAGS) $(TEST1_OBJECTS) -lpthread -L$(LIB_DIR) -l$(LIB) -o $@ 

$(TEST2_EXECUTABLE): $(TEST2_OBJECTS)
	$(CC) $(CFLAGS) $(TEST2_OBJECTS) -lpthread -L$(LIB_DIR) -l$(LIB) -o $@ 

clean:
	rm -rf *.o *~ 

clean-all: clean
	rm -rf $(LIB_DIR) $(EXECUTABLES)
