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
#define USE_PTS		true
#include "Cartography.h"

//#define max(a, b) ((a) > (b) ? (a) : (b))
//#define min(a, b) ((a) > (b) ? (b) : (a))

/* STRING -------------------------------------- */

//static void showStringVector(StringVector sv, int n)
//{
//	int i;
//	for (i = 0; i < n; i++)
//	{
//		printf("%s\n", sv[i]);
//	}
//}

/* UTIL */

static void error(String message)
{
	fprintf(stderr, "%s.\n", message);
	exit(1); // Termina imediatamente a execução do programa
}

static void readLine(String line, FILE *f) // lê uma linha que existe obrigatoriamente
{
	if (fgets(line, MAX_STRING, f) == NULL)
		error("Ficheiro invalido");
	line[strlen(line) - 1] = '\0'; // elimina o '\n'
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
	if (pos >= 0) // pas zero interpretado como não mostrar
		printf("%4d ", pos);
	else
		printf("%4s ", "");
	if (z == 3)
		printf("%-25s %-13s %-22s", id.freguesia, id.concelho, id.distrito);
	else if (z == 2)
		printf("%-25s %-13s %-22s", "", id.concelho, id.distrito);
	else
		printf("%-25s %-13s %-22s", "", "", id.distrito);
}

static void showValue(int value)
{
	if (value < 0) // value negativo interpretado como char
		printf(" [%c]\n", -value);
	else
		printf(" [%3d]\n", value);
}

static bool sameIdentification(Identification id1, Identification id2, int z)
{
	if (z == 3)
		return strcmp(id1.freguesia, id2.freguesia) == 0
				&& strcmp(id1.concelho, id2.concelho) == 0
				&& strcmp(id1.distrito, id2.distrito) == 0;
	else if (z == 2)
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
	Rectangle r = {tl, br};
	return r;
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
	for (int i = 0; i < n; i++)
	{
		Coordinates c = vs[i];
		xmin = fmin(xmin, c.lon);
		xmax = fmax(xmax, c.lon);
		ymin = fmin(ymin, c.lat);
		ymax = fmax(ymax, c.lat);
	}
	return rect(coord(ymax, xmin), coord(ymin, xmax));
}

bool insideRectangle(Coordinates c, Rectangle r)
{
	return (c.lat <= r.topLeft.lat && c.lat >= r.bottomRight.lat
			&& c.lon >= r.topLeft.lon && c.lon <= r.bottomRight.lon);
}

/* RING -------------------------------------- */

static Ring readRing(FILE *f)
{
	int i, n = readInt(f);

	// Allocate size for the Ring
	Ring *r = malloc(sizeof(Ring));

	// Allocate size for the n vertexes of the Ring
	r->vertexes = malloc(sizeof(Coordinates) * n);

	r->nVertexes = n;
	for (i = 0; i < n; i++)
	{
		r->vertexes[i] = readCoordinates(f);
	}
	r->boundingBox =
		calculateBoundingBox(r->vertexes, r->nVertexes);
	return *r;
}

// http://alienryderflex.com/polygon/
bool insideRing(Coordinates c, Ring r)
{
	if (!insideRectangle(c, r.boundingBox)) // otimização
		return false;
	double x = c.lon, y = c.lat;
	int i, j;
	bool oddNodes = false;
	for (i = 0, j = r.nVertexes - 1; i < r.nVertexes; j = i++)
	{
		double xi = r.vertexes[i].lon, yi = r.vertexes[i].lat;
		double xj = r.vertexes[j].lon, yj = r.vertexes[j].lat;
		if (((yi < y && y <= yj) || (yj < y && y <= yi)) && (xi <= x || xj <= x))
		{
			oddNodes ^= (xi + (y - yi) / (yj - yi) * (xj - xi)) < x;
		}
	}
	return oddNodes;
}

bool adjacentRings(Ring a, Ring b)
{
	////// FAZER
	// Two Rings are adjacent if they share any vertexes
	int i;
	int j;
	for (i = 0; i < a.nVertexes; i++)
	{
		for (j = 0; j < b.nVertexes; j++) {
			if (sameCoordinates(a.vertexes[i], b.vertexes[j]))
			{
				return true;
			}
		}
	}

	return false;
}

/* PARCEL -------------------------------------- */

static Parcel readParcel(FILE *f)
{
	// Allocate size for the Parcel
	Parcel *p = malloc(sizeof(Parcel));
	p->identification = readIdentification(f);

	int i, n = readInt(f);

	p->edge = readRing(f);
	p->nHoles = n;
	// Allocate size for the n rings of the hole
	p->holes = malloc(sizeof(Ring) * n);
	for (i = 0; i < n; i++)
	{
		p->holes[i] = readRing(f);
	}
	return *p;
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
	if (insideRing(c, p.edge))
	{
		int i;
		for (i = 0; i < p.nHoles; i++)
		{
			// If it's inside the hole then it's not inside the parcel
			if (insideRing(c, p.holes[i]))
			{
				return false;
			}
		}
		return true;
	}
	return false;
}

/**
 * Tests if Parcel a is inside Parcel b
 */
static bool parcelInsideParcel(Parcel *a, Parcel *b) {
	int i;
	for ( i = 0; i < b->nHoles ; i++) {
		if (adjacentRings(a->edge, b->holes[i])) {
			return true;
		}
	}
	return false;
}

/*
 * Checks if Parcels a and b are nested i.e. one is inside the other
 * */
static bool nestedParcels(Parcel *a, Parcel *b) {
	return parcelInsideParcel(a, b) || parcelInsideParcel(b, a);
}

bool adjacentParcels(Parcel a, Parcel b)
{
	////// FAZER
	if (adjacentRings(a.edge, b.edge)) {
		return true;
	}

	return nestedParcels(&a, &b);

}

/* CARTOGRAPHY -------------------------------------- */

int loadCartography(String fileName, Cartography *cartography)
{
	FILE *f;
	int i;
	f = fopen(fileName, "r");
	if( f == NULL )
		error("Impossivel abrir ficheiro");

	int n = readInt(f); // Number of parcels

	// Cartography is a collection of parcels

	// Allocate size for the n parcels
	*cartography = malloc(sizeof(Parcel) * n);

	for( i = 0 ; i < n ; i++ ) {
		(*cartography)[i] = readParcel(f);
	}
	fclose(f);
	return n;
}

static int findLast(Cartography cartography, int n, int j, Identification id, int idNumZ)
{
	for (; j < n; j++)
	{
		if (!sameIdentification(cartography[j].identification, id, idNumZ))
			return j - 1;
	}
	return n-1;
}

void showCartography(Cartography cartography, int n)
{
	int last;
	Identification header = {"___FREGUESIA___", "___CONCELHO___", "___DISTRITO___"};
	showHeader(header);
	for (int i = 0; i < n; i = last + 1)
	{
		last = findLast(cartography, n, i, cartography[i].identification, 3);
		showParcel(i, cartography[i], last - i + 1);
	}
}

/* INTERPRETER -------------------------------------- */

static bool checkArgs(int arg)
{
	if (arg != -1)
		return true;
	else
	{
		printf("ERRO: FALTAM ARGUMENTOS!\n");
		return false;
	}
}

static bool checkPos(int pos, int n)
{
	if (0 <= pos && pos < n)
		return true;
	else
	{
		printf("ERRO: POSICAO INEXISTENTE!\n");
		return false;
	}
}

// L
static void commandListCartography(Cartography cartography, int n)
{
	showCartography(cartography, n);
}

static int mVertexes(Parcel p)
{
	int n = 0;
	n += p.edge.nVertexes;
	for (int i = 0; i < p.nHoles; i++)
		n += p.holes[i].nVertexes;
	return n;
}

static int calculateLength(Cartography cartography, int pos, int n, int mode)
{
	Identification id = cartography[pos].identification;
	int count = 1;
	for (int i = pos + 1; i < n
			&& sameIdentification(id, cartography[i].identification, mode); i++)
		count++;
	for (int i = pos - 1; i >= 0
			&& sameIdentification(id, cartography[i].identification, mode); i--)
		count++;
	return count;
}

// M pos
static void commandMaximum(int pos, Cartography cartography, int n)
{
	if (!checkArgs(pos) || !checkPos(pos, n))
		return;
	int maxPos, max = 0;
	Identification id = cartography[pos].identification;

	for (maxPos = pos; maxPos > 0 &&
					   sameIdentification(id, cartography[maxPos].identification, 3);
		 maxPos--)
		;
	max = mVertexes(cartography[maxPos]);

	for (int i = maxPos; i < n; i++)
	{
		Parcel p = cartography[i];
		if (sameIdentification(id, p.identification, 3))
		{
			int v = mVertexes(p);
			if (v > max)
			{
				max = v;
				maxPos = i;
			}
		}
		else
			break;
	}
	showParcel(maxPos, cartography[maxPos], max);
}

static void maxPos(int *pos, int i, double *prevmax, double m)
{
	if (*prevmax < m)
	{
		*prevmax = m;
		*pos = i;
	}
}

static void minPos(int *pos, int i, double *prevmin, double m)
{
	if (*prevmin > m)
	{
		*prevmin = m;
		*pos = i;
	}
}

// X
static void commandExtremes(Cartography cartography, int n)
{
	int north, south, west, east = 0;
	double xmax = -180, ymax = -90, xmin = 180, ymin = 90;
	for (int i = 0; i < n; i++)
	{
		Ring edge = cartography[i].edge;
		Rectangle r = calculateBoundingBox(edge.vertexes, edge.nVertexes);
		maxPos(&north, i, &ymax, r.topLeft.lat);
		maxPos(&east, i, &xmax, r.bottomRight.lon);
		minPos(&south, i, &ymin, r.bottomRight.lat);
		minPos(&west, i, &xmin, r.topLeft.lon);
	}
	showParcel(north, cartography[north], -'N');
	showParcel(east, cartography[east], -'E');
	showParcel(south, cartography[south], -'S');
	showParcel(west, cartography[west], -'W');
}


static void printHoles(Ring *holes, int n)
{
	int i;
	for (i = 0; i < n; i++)
	{
		int nVert = (*(holes + i)).nVertexes;
		printf("%d ", nVert);
	}
}

// R pos
static void commandResume(int pos, Cartography cartography, int n)
{
	if (!checkArgs(pos) || !checkPos(pos, n))
		return;
	Parcel p = cartography[pos];
	Identification id = p.identification;
	showIdentification(pos, id, 3);
	printf("\n");
	printf("%5s%d ", "", p.edge.nVertexes);
	printHoles(p.holes, p.nHoles);
	Rectangle boundingBox = p.edge.boundingBox;
	showRectangle(boundingBox);
	printf("\n");
}

// V lat lon pos
static void commandTrip(double lat, double lon, int pos, Cartography cartography, int n)
{
	if (!checkArgs(pos) || !checkPos(pos, n)) //TODO check lat and lon
		return;
	Coordinates c = coord(lat, lon);
	Coordinates *p = cartography[pos].edge.vertexes;
	int num = cartography[pos].edge.nVertexes;
	double minDist = haversine(c, *p);
	for (p++; p < (cartography[pos].edge.vertexes + num); p++)
	{
		minDist = fmin(minDist, haversine(c, *p));
	}
	printf(" %f\n", minDist);
}

static void showHowMany(int pos, Cartography cartography, int n, int mode)
{
	showIdentification(pos, cartography[pos].identification, mode);
	showValue(calculateLength(cartography, pos, n, mode));
}

// Q pos
static void commandHowMany(int pos, Cartography cartography, int n)
{
	if (!checkArgs(pos) || !checkPos(pos, n))
		return;
	showHowMany(pos, cartography, n, 3);
	showHowMany(pos, cartography, n, 2);
	showHowMany(pos, cartography, n, 1);
}

static int compareStrings(const void *av, const void *bv) {
	return strcmp(av, bv);
}

// C
static void commandCounties(Cartography cartography, int n) {
	// Allocate size for the copy
	String *counties = malloc(sizeof(String) * n);

	int count = 0;
	int last;
	// Copy each county without repeats onto the vector
	for (int i = 0; i < n; i = last + 1)
	{
		last = findLast(cartography, n, i, cartography[i].identification, 2);
		strcpy((char *) (counties+count++), cartography[i].identification.concelho);
	}

	// Sort the vector
	qsort(counties, count, sizeof(String), compareStrings);

	for ( int j = 0 ; j < count ; j ++ ) {
		printf("%s\n", counties[j]);
	}

	free(counties);
}

// D
static void commandDistricts(Cartography cartography, int n)
{
	// Allocate size for the copy
	String *districts = malloc(sizeof(String) * n);

	int count = 0;
	int last;
	// Copy each district without repeats onto the vector
	for (int i = 0; i < n; i = last + 1)
	{
		last = findLast(cartography, n, i, cartography[i].identification, 1);
		strcpy((char *) (districts+count++), cartography[i].identification.distrito);
	}

	// Sort the vector
	qsort(districts, count, sizeof(String), compareStrings);

	for (int j = 0 ; j < count ; j++) {
		printf("%s\n", districts[j]);
	}

	free(districts);
}

static int inParcel(double lat, double lon, Cartography cartography, int n)
{
	Coordinates c = coord(lat, lon);
	for (int i = 0; i < n; i++)
	{
		if (insideParcel(c, cartography[i]))
		{
			return i;
		}
	}
	return -1;
}

// P lat lon
static void commandParcel(double lat, double lon, Cartography cartography, int n)
{
	//TODO check lat & lon
	int res = inParcel(lat, lon, cartography, n);
	if (res == -1)
	{
		printf("FORA DO MAPA\n");
	}
	else
	{
		showIdentification(res, cartography[res].identification, 3);
		printf("\n");
	}
}

// A pos
static void commandAdjacent(int pos, Cartography cartography, int n)
{
	if (!checkArgs(pos) || !checkPos(pos, n))
		return ;

	Parcel p = cartography[pos];
	int emptyFlag = 1;
	int i;
	for (i = 0; i < n; i++) {
		if (i != pos && adjacentParcels(p, cartography[i])) {
			emptyFlag = 0;
			showIdentification(i, cartography[i].identification, 3);
			printf("\n");
		}
	}
	if (emptyFlag) {
		printf("NAO HA ADJACENCIAS\n");
	}
}

// F pos1 pos2
static void commandBoundaries(int pos1, int pos2, Cartography cartography, int n)
{
	if (!checkArgs(pos1) || !checkPos(pos1, n) || !checkArgs(pos2) || !checkPos(pos2, n))
			return ;

	if (pos1 == pos2) {
		printf(" 0\n");
		return ;
	}

	// BFS
	int queue[n];
	int add = 0, remove = 0;

	// We add the starting node to the queue
	queue[add++] = pos1;

	int distances[n];
	// All distances start at -1 since they haven't been discovered yet
	memset(distances, -1, n*sizeof(int));
	// The distances from the start to itself is 0
	distances[pos1] = 0;

	// Make the queue

	while(add != remove) {

		int v = queue[remove++];

		// For each neighbor
		for (int i = 0 ; i < n ; i++) {
			// If adjacent
			if (v != i && adjacentParcels(cartography[v], cartography[i]))
			{

				// If we find it, end straight away
				if (i == pos2) {
					printf(" %d\n", distances[v] + 1);
					return ;
				}

				// If it wasn't visited
				if (distances[i] == -1) {
					// Update its distance to start
					distances[i] = 1 + distances[v];
					// Add it to the queue
					queue[add++] = i;
				}
			}
		}
	}

	// If distances[pos2] was not updated to a number different than -1
	// Then we haven't found it. The graph is not connected.
	if (distances[pos2] == -1) {
		printf("NAO HA CAMINHO\n");
	}


}

static void printIndex(int n, int *indexes)
{
	printf(" ");
	for(int i = 0; i < n; i++){
		bool minimize = false;
		printf("%d", indexes[i]);
		while(i< n-1 && indexes[i+1] == indexes[i]+1){
			i++;
			minimize = true;
		}
		if (minimize) printf("-%d", indexes[i]);
		if(i != n-1) printf(" ");
	}
	printf("\n");
}

static bool pull(int * range, int r, int pos,double dist, Parcel * cartography) {
	for(int i = 0 ; i<r; i++){
		int posi = range[i];
		if(haversine(cartography[posi].edge.vertexes[0],
					cartography[pos].edge.vertexes[0]) <= dist)
		{
			return true;
		}
	}
	return false;
}
static bool belongsto(int * range, int r, int pos){
	for(int i = 0; i< r; i++){
		if(range[i] == pos)
			return true;
	}
	return false;
}

static int checkPull(int * inRange, int * outRange, int *out,
		int* outR, double dist, Parcel* cartography, int * in){
	int count = 0;
	int added = 0;
	for(int k = 0; k < *out; k++){
		if(!belongsto(inRange, *in, outR[k])){
			if(pull(inRange, *in, outR[k], dist, cartography)){
				inRange[(*in)++] = outR[k];
				added++;
			} else {
				outRange[count++] = outR[k];

			}
		}
	}
	*out = count;
	return added;
}

static int compareInt(const void *av, const void *bv)
{
	return *((int *)av)-*((int*)bv);
}

static int split(Parcel * cartography, int * start, int ns, int * outRange, double dist)
{
	int * inRange = malloc(ns*sizeof(int));
	int * outR = malloc(ns* sizeof(int));
	int in, out;
	for(int i = 0; i < ns; i++)
	{
		in = 0;
		out = 0;
		int posi= start[i];
		for(int j = 0; j < ns; j++){
			int posj= start[j];
			double d = haversine(cartography[posi].edge.vertexes[0],
					cartography[posj].edge.vertexes[0]);
			//printf("dist %f, posi: % d posj: %d\n i: %d j: %d \n", d, posi, posj, i, j);
			if(d <= dist){
				inRange[in++] = posj;
			} else {
				outR[out++] = posj;
			}

		}
		int added;
		do {
			added = checkPull(inRange, outRange, &out, outR, dist, cartography, &in);
		} while(added != 0);

		if(out != 0){
			qsort(inRange, in, sizeof(int), compareInt);
			printIndex(in, inRange);
			free(inRange);
			free(outR);
			return out;
		}
	}
	qsort(inRange, in, sizeof(int), compareInt);
	printIndex(in, inRange);
	free(inRange);
	free(outR);
	return 0;
}

// T dist
static void commandPartition(double dist, Cartography cartography, int n)
{
	int * outRange = malloc(n*sizeof(int));
	for(int i = 0; i<n; i++){
		outRange[i]= i;
	}
	int count = n;
	int flag;
	do {
		flag = split(cartography, outRange, count, outRange, dist);
		count = flag;
	} while(flag);
	free(outRange);
}

void interpreter(Cartography cartography, int n)
{
	String commandLine;
	for (;;)
	{ // ciclo infinito
		printf("> ");
		readLine(commandLine, stdin);
		char command = ' ';
		double arg1 = -1.0, arg2 = -1.0, arg3 = -1.0;
		sscanf(commandLine, "%c %lf %lf %lf", &command, &arg1, &arg2, &arg3);
		// printf("%c %lf %lf %lf\n", command, arg1, arg2, arg3);
		switch (commandLine[0])
		{
		case 'L':
		case 'l': // listar
			commandListCartography(cartography, n);
			break;

		case 'M':
		case 'm': // maximo
			commandMaximum(arg1, cartography, n);
			break;

		case 'X':
		case 'x': // extremos
			commandExtremes(cartography, n);
			break;

		case 'R':
		case 'r': // resumo
			commandResume(arg1, cartography, n);
			break;

		case 'V':
		case 'v': // viagem
			commandTrip(arg1, arg2, arg3, cartography, n);
			break;

		case 'Q':
		case 'q': // quantos
			commandHowMany(arg1, cartography, n);
			break;

		case 'C':
		case 'c': // concelhos
			commandCounties(cartography, n);
			break;

		case 'D':
		case 'd': // distritos
			commandDistricts(cartography, n);
			break;

		case 'P':
		case 'p': // parcela
			commandParcel(arg1, arg2, cartography, n);
			break;

		case 'A':
		case 'a': // adjacencias
			commandAdjacent(arg1, cartography, n);
			break;

		case 'F':
		case 'f': // fronteiras
			commandBoundaries(arg1, arg2, cartography, n);
			break;

		case 'T':
		case 't': // particao
			commandPartition(arg1, cartography, n);
			break;
		case 'Z':
		case 'z': // terminar
			printf("Fim de execucao! Volte sempre.\n");
			return;

		default:
			printf("Comando desconhecido: \"%s\"\n", commandLine);
		}
	}
}
