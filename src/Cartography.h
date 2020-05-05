/*
 * Cartography module interface
 * LAP AMD-2020
 */

#ifndef _Cartography_
#define _Cartography_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>



/* STRING -------------------------------------- */

#define MAX_STRING			256
#define MAX_STRING_VECTOR	1024

typedef char String[MAX_STRING];
typedef String StringVector[MAX_STRING_VECTOR];



/* IDENTIFICATION -------------------------------------- */

typedef struct {	// Identificação duma parcela
	String freguesia, concelho, distrito;
} Identification;



/* COORDINATES -------------------------------------- */

typedef struct {	// Coordenadas - latitude e longitude
	double lat, lon;
} Coordinates;

Coordinates coord(double lat, double lon);
bool sameCoordinates(Coordinates c1, Coordinates c2);
double haversine(Coordinates c1, Coordinates c2);

#define EARTH_RADIUS		6371.0
#define PI 3.14159265358979323846264338327950288



/* RECTANGLE -------------------------------------- */

typedef struct {	// Retangulo
	Coordinates topLeft, bottomRight;
} Rectangle;

Rectangle rect(Coordinates tl, Coordinates br);
bool insideRectangle(Coordinates c, Rectangle r);



/* RING -------------------------------------- */

#if USE_PTS
typedef struct {	// Anel - um caminho linear fechado que não se auto-intersecta
	Coordinates *vertexes;
	int nVertexes;
	Rectangle boundingBox;
} Ring;
#else
#define MAX_VERTEXES		30000
typedef struct {	// Anel - um caminho linear fechado que não se auto-intersecta
	Coordinates vertexes[MAX_VERTEXES];
	int nVertexes;
	Rectangle boundingBox;
} Ring;
#endif

bool insideRing(Coordinates c, Ring r);
bool adjacentRings(Ring a, Ring b);



/* PARCEL -------------------------------------- */

#if USE_PTS
typedef struct {	// Parcela duma freguesia. É um anel com eventuais buracos
	Identification identification;
	Ring edge;
	Ring *holes;
	int nHoles;
} Parcel;
#else
#define MAX_HOLES			2
typedef struct {	// Parcela duma freguesia. É um anel com eventuais buracos
	Identification identification;
	Ring edge;
	Ring holes[MAX_HOLES];
	int nHoles;
} Parcel;
#endif

bool insideParcel(Coordinates c, Parcel p);
bool adjacentParcels(Parcel a, Parcel b);



/* CARTOGRAPHY -------------------------------------- */

#if USE_PTS
typedef Parcel *Cartography;	// Mapa - uma coleção de parcelas
#else
#define MAX_PARCELS			200
typedef Parcel Cartography[MAX_PARCELS];	// Mapa - uma coleção de parcelas
#endif

int loadCartography(String fileName, Cartography *cartography);
void showCartography(Cartography cartography, int n);



/* INTERPRETER -------------------------------------- */

void interpreter(Cartography cartography, int n);

#endif
