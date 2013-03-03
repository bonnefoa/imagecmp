SRCDIR  = src
IDIR    = include
ODIR    = obj
LDIR    = lib
TESTDIR = tests

CC      = clang
CCFLAGS = -I$(IDIR) -Wall 

LIBS = -ljpeg -lOpenCL
TEST_LIBS = $(LIBS) -lcheck

_OBJ = cl_image.o cl_util.o list.o image_utils.o cl_histogram.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

_TESTS = check_cl_image.o
TESTS = $(patsubst %,$(ODIR)/%,$(_TESTS))

MAIN = $(ODIR)/main.o $(OBJ) 

debug: CC += -DDEBUG -g
debug: main

$(ODIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(@D)
	$(CC) -c -o $@ $< $(CCFLAGS)

$(ODIR)/%.o: $(TESTDIR)/%.c
	@mkdir -p $(@D)
	$(CC) -c -o $@ $< $(CCFLAGS)

main: $(MAIN)
	$(CC) -o $@ $^ $(CCFLAGS) $(LIBS)

check_cl_image: $(TESTS) $(OBJ) 
	$(CC) -o $@ $^ $(CCFLAGS) $(TEST_LIBS) 

.PHONY: clean

clean:
	rm -rf $(ODIR)/*.o 
