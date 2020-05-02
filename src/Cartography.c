/*
largura maxima = 100 colunas
tab = 4 espaços
0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789

	Linguagens e Ambientes de Programação (B) - Projeto de 2019/20

	Cartography.c

	Este ficheiro constitui apenas um ponto de partida para o
	seu trabalho. Todo este ficheiro pode e deve ser alterado
	à vontade, a começar por este comentário. É preciso inventar
	muitas funções novas.

COMPILAÇÃO

  gcc -std=c11 -o Main Cartography.c Main.c -lm

IDENTIFICAÇÃO DOS AUTORES

  Aluno 1: 55075 Jacinta Sousa
  Aluno 2: 55697 Joao Bordalo

COMENTÁRIO

 Coloque aqui a identificação do grupo, mais os seus comentários, como
 se pede no enunciado.

*/

#include "Cartography.h"

#define max(a,b)     ((a) > (b) ? (a) : (b))
#define min(a,b)     ((a) > (b) ? (b) : (a))

/* STRING -------------------------------------- */

static void showStringVector(StringVector sv, int n) {
	int i;
	for( i = 0 ; i < n ; i++ ) {
		printf("%s\n", sv[i]);
	}
}

/* UTIL */

static void error(String message)
{
	fprintf(stderr, "%s.\n", message);
	exit(1);	// Termina imediatamente a execução do programa
}

static void readLine(String line, FILE *f)	// lê uma linha que existe obrigatoriamente
{
	if( fgets(line, MAX_STRING, f) == NULL )
		error("Ficheiro invalido");
	line[strlen(line) - 1] = '\0';	// elimina o '\n'
}

static int readInt(FILE *f)
{
	int i;
	String line;
	readLine(line, f);
	sscanf(line, "%d", &i);
	return i;
}


/* IDENTIFICATION -------------------------------------- */

static Identification readIdentification(FILE *f)
{
	Identification id;
	String line;
	readLine(line, f);
	sscanf(line, "%s %s %s", id.freguesia, id.concelho, id.distrito);
	return id;
}

static void showIdentification(int pos, Identification id, int z)
{
	if( pos >= 0 ) // pas zero interpretado como não mostrar
		printf("%4d ", pos);
	else
		printf("%4s ", "");
	if( z == 3 )
		printf("%-25s %-13s %-22s", id.freguesia, id.concelho, id.distrito);
	else if( z == 2 )
		printf("%-25s %-13s %-22s", "", id.concelho, id.distrito);
	else
		printf("%-25s %-13s %-22s", "", "", id.distrito);
}

static void showValue(int value)
{
	if( value < 0 ) // value negativo interpretado como char
		printf(" [%c]\n", -value);
	else
		printf(" [%3d]\n", value);
}

static bool sameIdentification(Identification id1, Identification id2, int z)
{
	if( z == 3 )
		return strcmp(id1.freguesia, id2.freguesia) == 0
			&& strcmp(id1.concelho, id2.concelho) == 0
			&& strcmp(id1.distrito, id2.distrito) == 0;
	else if( z == 2 )
		return strcmp(id1.concelho, id2.concelho) == 0
			&& strcmp(id1.distrito, id2.distrito) == 0;
	else
		return strcmp(id1.distrito, id2.distrito) == 0;
}


/* COORDINATES -------------------------------------- */

Coordinates coord(double lat, double lon)
{
	Coordinates c = {lat, lon};
	return c;
}

static Coordinates readCoordinates(FILE *f)
{
	double lat, lon;
	String line;
	readLine(line, f);
	sscanf(line, "%lf %lf", &lat, &lon);
	return coord(lat, lon);
}

bool sameCoordinates(Coordinates c1, Coordinates c2)
{
	return c1.lat == c2.lat && c1.lon == c2.lon;
}

static double toRadians(double deg)
{
	return deg * PI / 180.0;
}

// https://en.wikipedia.org/wiki/Haversine_formula
double haversine(Coordinates c1, Coordinates c2)
{
	double dLat = toRadians(c2.lat - c1.lat);
	double dLon = toRadians(c2.lon - c1.lon);

	double sa = sin(dLat / 2.0);
	double so = sin(dLon / 2.0);

	double a = sa * sa + so * so * cos(toRadians(c1.lat)) * cos(toRadians(c2.lat));
	return EARTH_RADIUS * (2 * asin(sqrt(a)));
}


/* RECTANGLE -------------------------------------- */

Rectangle rect(Coordinates tl, Coordinates br)
{
	Rectangle rect = {tl, br};
	return rect;
}

static void showRectangle(Rectangle r)
{
	printf("{%lf, %lf, %lf, %lf}",
			r.topLeft.lat, r.topLeft.lon,
			r.bottomRight.lat, r.bottomRight.lon);
}

static Rectangle calculateBoundingBox(Coordinates vs[], int n)
{

	double xmin = 181, xmax = -181, ymin = 91, ymax = -91;
	for (int i = 0; i < n; i++) {
		Coordinates c = vs[i];
		xmin = min(xmin, c.lon);
		xmax = max(xmax, c.lon);
		ymin = min(ymin, c.lat);
		ymax = max(ymax, c.lat);
	}
	return rect(coord(ymax,xmin), coord(ymin,xmax));
}

bool insideRectangle(Coordinates c, Rectangle r)
{
	//TODO may want to do this for better readability
	// Coordinates tl = r.topLeft, br = r.bottomRight;
	// return (c.lat >= tl.lat && c.lat <= br.lat
	// 		&& c.lon <= tl.lon && c.lon >= br.lon);
	return (c.lat >= r.topLeft.lat && c.lat <= r.bottomRight.lat
			&& c.lon <= r.topLeft.lon && c.lon >= r.bottomRight.lon);
}


/* RING -------------------------------------- */

static Ring readRing(FILE *f)
{
	Ring r;
	int i, n = readInt(f);
	if( n > MAX_VERTEXES )
		error("Anel demasiado extenso");
	r.nVertexes = n;
	for( i = 0 ; i < n ; i++ ) {
		r.vertexes[i] = readCoordinates(f);
	}
	r.boundingBox =
		calculateBoundingBox(r.vertexes, r.nVertexes);
	return r;
}


// http://alienryderflex.com/polygon/
bool insideRing(Coordinates c, Ring r)
{
	if( !insideRectangle(c, r.boundingBox) )	// otimização
		return false;
	double x = c.lon, y = c.lat;
	int i, j;
	bool oddNodes = false;
	for( i = 0, j = r.nVertexes - 1 ; i < r.nVertexes ; j = i++ ) {
		double xi = r.vertexes[i].lon, yi = r.vertexes[i].lat;
		double xj = r.vertexes[j].lon, yj = r.vertexes[j].lat;
		if( ((yi < y && y <= yj) || (yj < y && y <= yi))
								&& (xi <= x || xj <= x) ) {
			oddNodes ^= (xi + (y-yi)/(yj-yi) * (xj-xi)) < x;
		}
	}
	return oddNodes;
}

bool adjacentRings(Ring a, Ring b)
{
////// FAZER
	// Two Rings are adjacent if they share vertexes
	int i;
	for (i = 0; i < a.nVertexes; i++) {
		// Assuming the border counts in insideRing
		if (insideRing(a.vertexes[i], b)) {
			return true;
		}
	}
	return false;
}


/* PARCEL -------------------------------------- */

static Parcel readParcel(FILE *f)
{
	Parcel p;
	p.identification = readIdentification(f);
	int i, n = readInt(f);
	if( n > MAX_HOLES )
		error("Poligono com demasiados buracos");
	p.edge = readRing(f);
	p.nHoles = n;
	for( i = 0 ; i < n ; i++ ) {
		p.holes[i] = readRing(f);
	}
	return p;
}

static void showHeader(Identification id)
{
	showIdentification(-1, id, 3);
	printf("\n");
}

static void showParcel(int pos, Parcel p, int lenght)
{
	showIdentification(pos, p.identification, 3);
	showValue(lenght);
}

bool insideParcel(Coordinates c, Parcel p)
{
////// FAZER
	// A point is inside a parcel if it's inside the edge but outside the holes
	if (insideRing(c, p.edge)) {
		int i;
		for (i = 0; i < p.nHoles; i++) {
			// If it's inside the hole then it's not inside the parcel
			if (insideRing(c, p.holes[i])) {
				return false;
			}
		}
		return true;
	}
	return false;
}

// Tests if Parcel a is inside Parcel b
bool parcelInParcel(Parcel a, Parcel b) {
	int i, j;
	// For each hole in b
	for (i = 0; i < b.nHoles ; i++) {
		Ring bHole = b.holes[i];
		Ring aEdge = a.edge;
		// Check if a is inside the hole
		for (j = 0 ; j < aEdge.nVertexes ; j++) {
			if ( insideRing(aEdge.vertexes[j], bHole) ) {
				return true;
			}
		}
	}
	return false;
}

// Tests if Parcel A is inside Parcel B or the other way around
bool nestedParcels(Parcel a, Parcel b) {
	return parcelInParcel(a, b) || parcelInParcel(b, a);
}

/*
 * TODO
 * Only problem with this function is that B may be inside A which means we need to test if
 *	1) B's edge is inside A for any point (will work for edges touching)
 *	2) If B's edge is totally inside A then it would be false since it would be a hole
 *	However it's still adjacent
 *	3) So we need to check if B's edge is a hole of A
 *	4) Also need to check if A is a hole of B
 *	If we assume only one parcel can be inside another then we can use insideRing to simplify this a lot
 * 	And that's what really happens in this context
 * 	insideRing would account for a parcel being fully inside the other parcel, no need to
 * 	check holes individually
 * */

bool adjacentParcels(Parcel a, Parcel b)
{
	////// FAZER
		/* TODO
		 * Might be wrong in terms of the holes
		 * Needs testing
		 * */
		// Two parcels are adjacent if they share a vertex
		Ring aRing = a.edge;
		// A parcel inside another is adjacent to the former
		if (nestedParcels(a, b)) {
			return true;
		}
		// Test edges touching
		int	i;
		for (i = 0; i < aRing.nVertexes ; i++) {
			if (insideParcel(aRing.vertexes[i], b)) {
				return true;
			}
		}
		return false;
}


/* CARTOGRAPHY -------------------------------------- */

int loadCartography(String fileName, Cartography cartography)
{
	FILE *f;
	int i;
	f = fopen(fileName, "r");
	if( f == NULL )
		error("Impossivel abrir ficheiro");
	int n = readInt(f);
	if( n > MAX_PARCELS )
		error("Demasiadas parcelas no ficheiro");
	for( i = 0 ; i < n ; i++ ) {
		cartography[i] = readParcel(f);
	}
	fclose(f);
	return n;
}

static int findLast(Cartography cartography, int n, int j, Identification id)
{
	for(  ; j < n ; j++ ) {
		if( !sameIdentification(cartography[j].identification, id, 3) )
			return j-1;
	}
	return n;
}

void showCartography(Cartography cartography, int n)
{
	int last;
	Identification header = {"___FREGUESIA___", "___CONCELHO___", "___DISTRITO___"};
	showHeader(header);
	for( int i = 0 ; i < n ; i = last + 1 ) {
		last = findLast(cartography, n, i, cartography[i].identification);
		showParcel(i, cartography[i], last - i + 1);
	}
}


/* INTERPRETER -------------------------------------- */

static bool checkArgs(int arg)
{
	if( arg != -1 )
		return true;
	else {
		printf("ERRO: FALTAM ARGUMENTOS!\n");
		return false;
	}
}

static bool checkPos(int pos, int n)
{
	if( 0 <= pos && pos < n )
		return true;
	else {
		printf("ERRO: POSICAO INEXISTENTE!\n");
		return false;
	}
}

// L
static void commandListCartography(Cartography cartography, int n)
{
	showCartography(cartography, n);
}

int nVertexes(Parcel p){
	int n = 0;
	n+= p.edge.nVertexes;
	for(int i = 0; i < p.nHoles; i++)
		n+=p.holes[i].nVertexes;
	return n;
}


// M pos
static void commandMaximum(int pos, Cartography cartography, int n)
{
	if( !checkArgs(pos) || !checkPos(pos, n) )
		return ;
	int maxPos = pos, max, lenght = 0;
	Identification id = cartography[pos].identification;

	for(maxPos; maxPos > 0 &&
		sameIdentification(id, cartography[maxPos].identification, 3); maxPos--);
	max = nVertexes(cartography[maxPos]);

	for(int i = maxPos; i < n; i++){
		lenght++;
		Parcel p = cartography[i];
		if(sameIdentification(id, p.identification, 3)){
			int v = nVertexes(p);
			if(v > max){
				max = v;
				maxPos = i;
			}
		} else break;
	}
	showParcel(maxPos, cartography[maxPos], lenght);
}

void maxPos (int * pos, int i, double * prevmax, double m){
	if(*prevmax < m){
		*prevmax = m;
		*pos = i;
	}
}

void minPos (int * pos, int i, double *prevmin, double m){
	if(*prevmin > m){
		*prevmin = m;
		*pos = i;
	}

}

int calculateLength (Cartography cartography, int pos, int n){
	Identification id = cartography[pos].identification;
	int count = 1;
	for(int i = pos+1; i < n && sameIdentification(id, cartography[i].identification, 3); i++)
		count++;
	for(int i = pos-1; i > 0 && sameIdentification(id, cartography[i].identification, 3); i--)
		count++;
	return count;
}

static void commandExtremes(Cartography cartography, int n){
	int north, south, west, east = 0;
	double xmax = -180, ymax = -90, xmin = 180, ymin = 90;
	for (int i = 1; i < n; i++){
		Ring edge = cartography[i].edge;
		Rectangle r = calculateBoundingBox(edge.vertexes,edge.nVertexesn);
		maxPos(&north, i, &ymax, r.topLeft.lat);
		maxPos(&east, i, &xmax, r.bottomRight.lon);
		minPos(&south, i, &ymin, r.bottomRigth.lat);
		minPos(&west, i, &xmin, r.topLeft.lon);
	}
	showParcel(north, cartography[north], calculateLength(cartography, north, n));
	showParcel(east, cartography[east], calculateLength(cartography, east, n));
	showParcel(south, cartography[south], calculateLength(cartography, south, n));
	showParcel(west, cartography[west], calculateLength(cartography, west, n));
}

/*
static double calculateRingLength(Ring ring) {
	double length;
	int i;
	for (i = 0; i < ring.nVertexes; i++) {
		length += haversine(ring.vertexes[i], ring.vertexes[ (i + 1) % ring.nVertexes ]);
	}
	return length;
}
*/

/*
 * R pos

		Comando Resumo - Dada uma parcela indicada através duma posição no vetor,
		mostra um resumo dessa parcela. Apresentar: identificação,
		comprimento do anel exterior (inteiro numa nova linha e alinhado com a
		identificação que aparece por cima), comprimento dos vários
		buracos (inteiros separados por um espaço), bounding box do
		anel exterior (delimitada por chavetas).
		*/
static void commandResume(int pos, Cartography cartography, int n) {
	if( !checkArgs(pos) || !checkPos(pos, n) )
			return ;
	Parcel p = cartography[pos];
	Identification id = p.identification;
	showIdentification(pos, id, 3);
	printf("\n");
	Rectangle boundingBox = p.edge.boundingBox;
	printf("%9d ", p.edge.nVertexes);
	// TODO Add holes
	showRectangle(boundingBox);
	printf("\n");
}

static void commandTrip(double lat, double lon, int pos, Cartography cartography, int n) {

}

static void commandHowMany(int pos, Cartography cartography, int n) {

}

static void commandCounties(Cartography cartography, int n){

}

static void commandDistricts(Cartography cartography, int n) {

}

static void commandParcel(double lat, double lon, Cartography cartography, int n) {

}

static void commandAdjacent(int pos, Cartography cartography, int n) {

}

static void commandBoundaries(int pos1, int pos2, Cartography cartography, int n) {

}

static void commandPartition(double dist, Cartography cartography, int n) {

}

void interpreter(Cartography cartography, int n)
{
	String commandLine;
	for(;;) {	// ciclo infinito
		printf("> ");
		readLine(commandLine, stdin);
		char command = ' ';
		double arg1 = -1.0, arg2 = -1.0, arg3 = -1.0;
		sscanf(commandLine, "%c %lf %lf %lf", &command, &arg1, &arg2, &arg3);
		// printf("%c %lf %lf %lf\n", command, arg1, arg2, arg3);
		switch( commandLine[0] ) {
			case 'L': case 'l':	// listar
				commandListCartography(cartography, n);
				break;

			case 'M': case 'm':	// maximo
				commandMaximum(arg1, cartography, n);
				break;

			case 'X': case 'x':
				commandExtremes(cartography, n);
				break;

			case 'R': case 'r':
				commandResume(arg1, cartography, n);
				break;

			case 'V': case 'v':
				commandTrip(arg1, arg2, arg3, cartography, n);
				break;

			case 'Q': case 'q':
				commandHowMany(arg1, cartography, n);
				break;

			case 'C': case 'c':
				commandCounties(cartography, n);
				break;

			case 'D': case 'd':
				commandDistricts(cartography, n);
				break;

			case 'P': case 'p':
				commandParcel(arg1, arg2, cartography, n);
				break;

			case 'A': case 'a':
				commandAdjacent(arg1, cartography, n);
				break;

			case 'F': case 'f':
				commandBoundaries(arg1, arg2, cartography, n);
				break;

			case 'T': case 't':
				commandPartition(arg1, cartography, n);
				break;
			case 'Z': case 'z':	// terminar
				printf("Fim de execucao! Volte sempre.\n");
				return;


			default:
				printf("Comando desconhecido: \"%s\"\n", commandLine);
		}
	}
}
