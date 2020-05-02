/*
 gcc -c src/Cartography.c -o build/Cartography.o
 ar rcs lib/libcartography.a build/Cartography.o
 gcc src/Main.c -L -lcartography -o bin/Main
 */

/*
 *  Cartography main
 * LAP AMD-2020
 *
 * COMPILAÇÃO: gcc -std=c11 -o Main Cartography.c Main.c -lm
 */

#include "Cartography.h"

#include <assert.h>
#include <malloc.h>

static void internalTests(bool show)
{
	if( show ) {
		assert( fabs( haversine(coord(36.12, -86.67), coord(33.94, -118.40))
				- 2886.444442837983 ) < 0.00000000001 );

		Ring r = {{{0,0}, {0,1}, {1, 1}, {1,0}}, 4, {{1,0}, {0,1}}};
		assert( insideRing(coord(0.5, 0.5), r) );
		assert( insideRing(coord(0.0000001, 0.0000001), r) );
		assert( insideRing(coord(0.0000001, 0.9999999), r) );
		assert( insideRing(coord(0.9999999, 0.9999999), r) );
		assert( !insideRing(coord(1.0000001, 0.5), r) );
		assert( !insideRing(coord(0.5, 1.0000001), r) );
	}
}

static Cartography cartography;	// variável gigante
static int nCartography = 0;

static void showMemory(bool show)
{
	if( show ) {
		long staticMemory = sizeof(cartography);
		printf("static memory = %ldMB\n", staticMemory/1024/1024);
	#if __GNUC__
		struct mallinfo m = mallinfo();
		long heapMemory = m.hblkhd;
		printf("heap memory = %ldMB\n", heapMemory/1024/1024);
	#endif
	}
}

int main(void)
{
	internalTests(false);
	nCartography = loadCartography("map.txt", cartography);
	showMemory(false);
	showCartography(cartography, nCartography);
	interpreter(cartography, nCartography);
	return 0;
}
