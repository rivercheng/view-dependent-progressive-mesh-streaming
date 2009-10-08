LIBRARY = -lOpenMesh_Core -lOpenMesh_Tools -lPocoFoundation -lPocoNet -lPocoUtil -lglut #-lprofiler	
OPTIONS =  -Wall -W -Wextra -g -O2 
CC = g++
ALL = ppmesh.o bitstring.o vertexid.o

all: $(ALL)
tags: *.cc *.hh
	$(CTAG) -R *
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
vertexid.o: vertexid.hh bitstring.hh
