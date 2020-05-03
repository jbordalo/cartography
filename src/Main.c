/*
 *  Cartography main
 * LAP AMD-2020
 *
 * COMPILAÇÃO: gcc -std=c11 -o Main Cartography.c Main.c -lm
 */

#include "Cartography.h"

static Cartography cartography;	// variável gigante
static int nCartography = 0;

int main(void)
{
	nCartography = loadCartography("map.txt", &cartography);
	showCartography(cartography, nCartography);
	interpreter(cartography, nCartography);
	return 0;
}
