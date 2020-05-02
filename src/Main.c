#include <stdio.h>
#include <stdlib.h>

int main(void) {
	puts(""); /* prints  */
	return EXIT_SUCCESS;
}

/*
 gcc -c src/Cartography.c -o build/Cartography.o
 ar rcs lib/libcartography.a build/Cartography.o
 gcc src/Main.c -L -lcartography -o bin/Main
 */
