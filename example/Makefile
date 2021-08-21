FLAG=-fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free -std=c++11 -Wall -g
LINKLIB= ../src/libmemcheck.a -ltcmalloc -ldl
INC=-I ../src/
all:
	g++ hookMallocExample.cpp -o hookMallocExample ${INC} ${FLAG} ${LINKLIB}
	g++ callStackExample.cpp -o callStackExample ${INC} ${FLAG} ${LINKLIB}
	g++ memCheckExample.cpp -o memCheckExample ${INC} ${FLAG} ${LINKLIB}
clean:
	rm -f hookMallocExample callStackExample memCheckExample