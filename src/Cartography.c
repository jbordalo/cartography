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

//#define max(a, b) ((a) > (b) ? (a) : (b))
//#define min(a, b) ((a) > (b) ? (b) : (a))

/* STRING -------------------------------------- */

static void showStringVector(StringVector sv, int n)
{
	int i;
	for (i = 0; i < n; i++)
	{
		printf("%s\n", sv[i]);
	}
}

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
		return strcmp(id1.freguesia, id2.freguesia) == 0 && strcmp(id1.concelho, id2.concelho) == 0 && strcmp(id1.distrito, id2.distrito) == 0;
	else if (z == 2)
		return strcmp(id1.concelho, id2.concelho) == 0 && strcmp(id1.distrito, id2.distrito) == 0;
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
	//TODO may want to do this for better readability
	// Coordinates tl = r.topLeft, br = r.bottomRight;
	// return (c.lat >= tl.lat && c.lat <= br.lat
	// 		&& c.lon <= tl.lon && c.lon >= br.lon);
	return (c.lat <= r.topLeft.lat && c.lat >= r.bottomRight.lat && c.lon >= r.topLeft.lon && c.lon <= r.bottomRight.lon);
}

/* RING -------------------------------------- */

static Ring readRing(FILE *f)
{
	Ring r;
	int i, n = readInt(f);
	if (n > MAX_VERTEXES)
		error("Anel demasiado extenso");
	r.nVertexes = n;
	for (i = 0; i < n; i++)
	{
		r.vertexes[i] = readCoordinates(f);
	}
	r.boundingBox =
		calculateBoundingBox(r.vertexes, r.nVertexes);
	return r;
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
	Parcel p;
	p.identification = readIdentification(f);
	int i, n = readInt(f);
	if (n > MAX_HOLES)
		error("Poligono com demasiados buracos");
	p.edge = readRing(f);
	p.nHoles = n;
	for (i = 0; i < n; i++)
	{
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

static bool parcelInsideParcel(Parcel *a, Parcel *b) {
	int i;
	for ( i = 0; i < b->nHoles ; i++) {
		if (adjacentRings(a->edge, b->holes[i])) {
			return true;
		}
	}
	return false;
}

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
	int n = readInt(f);
	if( n > MAX_PARCELS )
		error("Demasiadas parcelas no ficheiro");
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

int nVertexes(Parcel p)
{
	int n = 0;
	n += p.edge.nVertexes;
	for (int i = 0; i < p.nHoles; i++)
		n += p.holes[i].nVertexes;
	return n;
}

int calculateLength(Cartography cartography, int pos, int n, int mode)
{
	Identification id = cartography[pos].identification;
	int count = 1;
	for (int i = pos + 1; i < n && sameIdentification(id, cartography[i].identification, mode); i++)
		count++;
	for (int i = pos - 1; i >= 0 && sameIdentification(id, cartography[i].identification, mode); i--)
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
	max = nVertexes(cartography[maxPos]);

	for (int i = maxPos; i < n; i++)
	{
		Parcel p = cartography[i];
		if (sameIdentification(id, p.identification, 3))
		{
			int v = nVertexes(p);
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

void maxPos(int *pos, int i, double *prevmax, double m)
{
	if (*prevmax < m)
	{
		*prevmax = m;
		*pos = i;
	}
}

void minPos(int *pos, int i, double *prevmin, double m)
{
	if (*prevmin > m)
	{
		*prevmin = m;
		*pos = i;
	}
}

static void commandExtremes(Cartography cartography, int n)
{
	int north, south, west, east = 0;
	double xmax = -180, ymax = -90, xmin = 180, ymin = 90;
	for (int i = 1; i < n; i++)
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
	printf("%f\n", minDist);
}

void showHowMany(int pos, Cartography cartography, int n, int mode)
{
	showIdentification(pos, cartography[pos].identification, mode);
	showValue(calculateLength(cartography, pos, n, mode));
}

static void commandHowMany(int pos, Cartography cartography, int n)
{
	if (!checkArgs(pos) || !checkPos(pos, n))
		return;
	showHowMany(pos, cartography, n, 3);
	showHowMany(pos, cartography, n, 2);
	showHowMany(pos, cartography, n, 1);
}

//static int compareCounties(const void *av, const void *bv)
//{
//	const Parcel *a = av, *b = bv;
//	return strcmp(a->identification.concelho, b->identification.concelho);
//}

static int compareStrings(const void *av, const void *bv) {
	return strcmp(av, bv);
}

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

/* VERY WELL WRITTEN AND I READ IT WITH GREAT ATTENTION, <3 YOU
//command Counties TODO
static void c (Cartography cartography, int n){
	Parcel * counties = memcpy(malloc(n*sizeof(Parcel)), cartography, n*sizeof(Parcel));
	qsort(counties, n, sizeof(Parcel), compareCounties);
	Parcel new = *counties;
	printf("%s\n", new.identification.concelho);
	for(int i = 0 ; i < n; i++){
		if(compareCounties(&new, x+i) != 0){
			new = *(x+i);
			printf("%s\n", new.identification.concelho);
		}
	}
	free(counties);
}
*/

//static int compareDistricts(const void *av, const void *bv)
//{
//	const Parcel *a = av, *b = bv;
//	return strcmp(a->identification.distrito, b->identification.distrito);
//}

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

int inParcel(double lat, double lon, Cartography cartography, int n)
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

static void commandAdjacent(int pos, Cartography cartography, int n)
{
	if (!checkArgs(pos) || !checkPos(pos, n))
		return ;

	Parcel p = cartography[pos];
	int emptyFlag = 1;
	int i;
	for (i = 0; i < n; i++) {
		if (!sameIdentification(p.identification, cartography[i].identification, 3)
				&& adjacentParcels(p, cartography[i])) {
			emptyFlag = 0;
			showIdentification(i, cartography[i].identification, 3);
			printf("\n");
		}
	}
	if (emptyFlag) {
		printf("NAO HA ADJACENCIAS\n");
	}
}

/*
static void generateAdjacencies(Parcel *parcel, Cartography cartography, int n, Parcel *adj, int *countReturn) {

		Parcel initial = *parcel;

		int i;
		for (i = 0; i < n; i++) {
			if (!sameIdentification(initial.identification, cartography[i].identification, 3)
					&& adjacentParcels(initial, cartography[i]))
			// TODO f 152 153 CRASHED IN THIS adjacentParcels FIRST ITERATION
			{

				memcpy(adj, &cartography[i], sizeof(Parcel));

				adj++;
				(*countReturn)++;
			}
		}
}
*/
/*
static bool discovered(Parcel *a, Parcel *v, int size) {
	for (int i = 0; i < size ; i++) {
		Parcel current = *(v+i);
		if (sameIdentification((*a).identification, current.identification, 3)) {
//			printf("Has been discovered\n");
			return true;
		}
	}
	return false;
}
*/
/*
static void commandoundaries(int pos1, int pos2, Cartography cartography, int n)
{
	if (!checkArgs(pos1) || !checkPos(pos1, n) || !checkArgs(pos2) || !checkPos(pos2, n))
			return ;

	// BFS
	int path = 0;
	// Define start and goal
	Parcel start = cartography[pos1];
	Parcel goal = cartography[pos2];

	Parcel *visited = malloc(n*sizeof(Parcel));
	int visits = 0;

	// Make the queue
	Parcel *queue = malloc(n*sizeof(Parcel));
	// int rear, front = 0;
	int add = 0, remove = 0;
	queue[add++] = start;
	visited[visits++] = start;

	printf("%d\n", discovered(&start, visited, visits));

	while(add!=remove) {

		Parcel v = queue[remove++];
		printf("v: %s\n", v.identification.freguesia);

		if (sameIdentification(v.identification, goal.identification, 3)) {
			printf("Goal found at %d\n", path);
			return ;
		} else {
//			printf("Goal not found\n");
		}

		Parcel *adj = malloc(n*sizeof(Parcel));
//		printf("Allocating adjacencies\n");

		int count = 0;
//		printf("Getting neighbors\n");
		generateAdjacencies(&v, cartography, n, adj, &count);
//		printf("Got neighbors\n");
		for (int j = 0 ; j < count; j++) {
			// TODO CARE ABOUT THIS ADDED LATE AT NIGHT
			// TODO F 163 169 GETTING STUCK IN VERDOEJO
			if (sameIdentification((adj+j)->identification, goal.identification, 3)) {
				printf("Goal found at %d\n", path);
				return ;
			}
			if (!discovered(adj+j, visited, visits)) {
//				printf("Not discovered\n");
				visited[visits++] = *(adj+j);
//				printf("Added %s to visited\n", (*(adj+j)).identification.freguesia);
				queue[add++] = *(adj+j);
				printf("Added %s to queue\n", (*(adj+j)).identification.freguesia);
			}
		}
		path++;
		free(adj);
	}

	*
	for(int j = 0; j < count ; j++) {
		showIdentification(-1, (adj+j)->identification, 3);
		printf("\n");
	}
	*
}
*/

static void commandBoundaries(int pos1, int pos2, Cartography cartography, int n)
{
	if (!checkArgs(pos1) || !checkPos(pos1, n) || !checkArgs(pos2) || !checkPos(pos2, n))
			return ;

	if (pos1 == pos2) {
		printf("0\n");
		return ;
	}

	// BFS
	int queue[n];
	int add = 0, remove = 0;
	queue[add++] = pos1;

	int distances[n];
	memset(distances, -1, n*sizeof(int));
	distances[pos1] = 0;

	// Make the queue

	while(add != remove) {

		int v = queue[remove++];

		// For each neighbor
		for (int i = 0 ; i < n ; i++) {
			// If adjacent
			if (!sameIdentification(cartography[v].identification, cartography[i].identification, 3)
					&& adjacentParcels(cartography[v], cartography[i]))
			{

				// Found it, end straight away
				if (sameIdentification(cartography[i].identification, cartography[pos2].identification, 3)) {
					printf("%d\n", distances[v] + 1);
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
	// TODO DO I NEED THIS ELSE??? IF I FOUND IT I STOPPED
//	else {
//		printf("Goal: %d\n", distances[pos2]);
//	}

}
/*
static void part(double dist, Cartography cartography, int * pos, int n){
	int far [n];
	int close[n];
	int f,c;
	//for(int i = 0; i < n-1; i++){ ??? seems to work without this loop
									//but does it make sense without it? EVERYTHING YOU DO MAKES SENSE SWEETIE
		f=0;
		c=0;
		close[c] = pos[0];
		c++;
		for(int j = 1; j < n; j++){
			if(haversine(cartography[pos[0]].edge.vertexes[0],
					cartography[pos[j]].edge.vertexes[0]) >= dist)
			{
				far[f] = pos[j];
				f++;
			} else {
				close[c] = pos[j];
				c++;
			}
		}
		//break;
	//}
	if( f != 0 ){
		part(dist, cartography, far, f);
	}
	//testing - order needs to be fixed + may want to improve the code
	for( int i = 0; i < c; i++){
		bool minimize = false;
		printf("%d", close[i]);
		while(i< c-1 && close[i+1] == close[i]+1){
			i++;
			minimize = true;
		}
		if (minimize) printf("-%d ", close[i]);
	}
}
*/

static void printIndex(int n, int *indexes)
{
	for( int i = 0; i < n; i++){
		bool minimize = false;
		printf("%d", indexes[i]);
		while(i< n-1 && indexes[i+1] == indexes[i]+1){
			i++;
			minimize = true;
		}
		if (minimize) printf("-%d ", indexes[i]);
	}
	printf("\n");
}

static int split(Parcel * cartography, int * start, int ns, int * outRange, double dist)
{
	int * inRange = malloc(ns*sizeof(int));
	int in, out;
	printf("inside split\n");
	for(int i = 0; i < ns; i++)
	{
		in = 0;
		out = 0;
		//printf("in: %d out: %d ns: %d \n", in, out,  ns);
		for(int j = 0; j < ns; j++){
			if(haversine(cartography[start[i]].edge.vertexes[0],
					cartography[start[j]].edge.vertexes[0]) >= dist){
				outRange[out++] = start[j];
			} else {
				inRange[in++] = start[j];
			}
		}
		//printf("in: %d out: %d \n", in, out);
		if(out != 0){
			printIndex(in, inRange);
			free(inRange);
			return out;
		}
	}
	printIndex(in, inRange);
	free(inRange);
	return 0;
}

static void commandPartition(double dist, Cartography cartography, int n)
{
	int * inRange = malloc(n*sizeof(int));
	int * outRange = malloc(n*sizeof(int));
	int in = 0, out = 0;
	for(int i = 0; i < n; i++){
			if(haversine(cartography[0].edge.vertexes[0], cartography[i].edge.vertexes[0]) <= dist){
				inRange[in++] = i;
			} else {
				outRange[out++] = i;
		}
	}
	printIndex(in, inRange);
	free(inRange);
	int flag;
	do {
		printf("inside while : out %d \n", out);
		flag = split(cartography, outRange, out, outRange, dist);
		out = flag;

	} while(flag);
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
		case 'x':
			commandExtremes(cartography, n);
			break;

		case 'R':
		case 'r':
			commandResume(arg1, cartography, n);
			break;

		case 'V':
		case 'v':
			commandTrip(arg1, arg2, arg3, cartography, n);
			break;

		case 'Q':
		case 'q':
			commandHowMany(arg1, cartography, n);
			break;

		case 'C':
		case 'c':
			commandCounties(cartography, n);
			break;

		case 'D':
		case 'd':
			commandDistricts(cartography, n);
			break;

		case 'P':
		case 'p':
			commandParcel(arg1, arg2, cartography, n);
			break;

		case 'A':
		case 'a':
			commandAdjacent(arg1, cartography, n);
			break;

		case 'F':
		case 'f':
			commandBoundaries(arg1, arg2, cartography, n);
			break;

		case 'T':
		case 't':
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
