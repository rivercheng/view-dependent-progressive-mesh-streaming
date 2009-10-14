LIBRARY = -lOpenMesh_Core #-lOpenMesh_Tools -lPocoFoundation -lPocoNet -lPocoUtil -lglut #-lprofiler	
OPTIONS =  -Wall -W -Wextra -g -O2 
CC = g++
ALL = test_ppmesh test_gfmesh
OBJ = ppmesh.o bitstring.o vertexid.o common_def.o gfmesh.o

all: $(ALL)
tags: *.cc *.hh
	$(CTAG) -R *
%:%.o $(OBJ)
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
common_def.o: common_def.hh
gfmesh.o: gfmesh.hh common_def.hh vertexid.hh bitstring.hh ppmesh.hh
gfmesh.o: huffman.hh
ppmesh.o: ppmesh.hh common_def.hh bitstring.hh huffman.hh vertexid.hh
test_gfmesh.o: gfmesh.hh common_def.hh vertexid.hh bitstring.hh
test_ppmesh.o: ppmesh.hh common_def.hh bitstring.hh huffman.hh vertexid.hh
vertexid.o: vertexid.hh bitstring.hh
