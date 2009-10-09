LIBRARY = -lOpenMesh_Core -lOpenMesh_Tools -lPocoFoundation -lPocoNet -lPocoUtil -lglut #-lprofiler	
OPTIONS =  -Wall -W -Wextra -g -O2 
CC = g++
ALL = ppmesh.o bitstring.o vertexid.o test_ppmesh
OBJ = ppmesh.o bitstring.o vertexid.o common_def.o

all: $(ALL)
tags: *.cc *.hh
	$(CTAG) -R *
test_ppmesh:test_ppmesh.o $(OBJ)
	$(CC) $(OPTIONS) $(LIBRARY) -o $@ $< $(OBJ)
%.o:%.cc 
	$(CC) $(OPTIONS) -c $<
clean:
	rm *.o
	rm $(ALL)
depend:
	makedepend -Y *.cc
# DO NOT DELETE

bitstring.o: bitstring.hh
ppmesh.o: ppmesh.hh common_def.hh bitstring.hh huffman.hh vertexid.hh
test_ppmesh.o: ppmesh.hh common_def.hh bitstring.hh huffman.hh vertexid.hh
vertexid.o: vertexid.hh bitstring.hh
