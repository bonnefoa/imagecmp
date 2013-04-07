SRCDIR  = src
IDIR    = include
ODIR    = obj
LDIR    = lib
TESTDIR = tests

CC      = clang
CCFLAGS = -I$(IDIR) -Wall -Wextra -std=c99 
CCFLAGS += `pkg-config --cflags zlib libpng eet`

TEST_CCFLAGS = -I$(IDIR) -Wall -Wextra -std=c99 
TEST_CCFLAGS += `pkg-config --cflags zlib libpng eet`

LIBS = -lOpenCL -ljpeg 
LIBS += `pkg-config --libs zlib libpng eet`

TEST_LIBS = $(LIBS) -lcheck

_OBJ = cl_util.o list.o image_util.o cl_histogram.o job_handler.o histogram.o util.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

MAIN = $(ODIR)/main.o $(OBJ) 

debug: CC += -DDEBUG -g -ggdb3 -O0
debug: main

$(ODIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(@D)
	$(CC) -c -o $@ $< $(CCFLAGS)

$(ODIR)/%.o: $(TESTDIR)/%.c
	@mkdir -p $(@D)
	$(CC) -c -o $@ $< $(TEST_CCFLAGS)

main: $(MAIN)
	$(CC) -o $@ $^ $(CCFLAGS) $(LIBS)

check_cl_image: $(ODIR)/check_cl_image.o $(OBJ) 
	$(CC) -o $@ $^ $(TEST_CCFLAGS) $(TEST_LIBS) 

check_job_handler: $(ODIR)/check_job_handler.o $(OBJ) 
	$(CC) -o $@ $^ $(TEST_CCFLAGS) $(TEST_LIBS) 

check_map: $(ODIR)/check_map.o $(OBJ) 
	$(CC) -o $@ $^ $(TEST_CCFLAGS) $(TEST_LIBS) 

check_histogram: $(ODIR)/check_histogram.o $(OBJ) 
	$(CC) -o $@ $^ $(TEST_CCFLAGS) $(TEST_LIBS) 

print_cl_infos: $(ODIR)/list_devices.o $(OBJ) 
	$(CC) -o $@ $^ $(CCFLAGS) $(LIBS)
	./print_cl_infos

.PHONY: clean

clean:
	rm -rf $(ODIR)/*.o 
