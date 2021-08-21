FLAG=-fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free -std=c++11 -O3 -Wall

all:
	g++ -c MallocHookHelper.cpp -o MallocHookHelper.o ${FLAG} 
	g++ -c callStack.cpp -o callStack.o ${FLAG}
	g++ -c MemCheckHelper.cpp -o MemCheckHelper.o ${FLAG}
	ar crv libmemcheck.a MallocHookHelper.o callStack.o MemCheckHelper.o
clean:
	rm -f *.o libmemcheck.a