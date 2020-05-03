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
	// Two Rings are adjacent if they share vertexes
	int i;
	for (i = 0; i < a.nVertexes; i++)
	{
		// Assuming the border counts in insideRing
		if (insideRing(a.vertexes[i], b))
		{
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
	return n;
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

// TODO type-safety

static int compareCounties(const void *av, const void *bv)
{
	const Parcel *a = av, *b = bv;
	return strcmp(a->identification.concelho, b->identification.concelho);
}

static void commandCounties(Cartography cartography, int n) {
	// Allocate size for the copy
	Parcel *counties = malloc(sizeof(Parcel) * n);
	// Copy the vector
	memcpy(counties, cartography, sizeof(Parcel) * n);

	// Sort the vector
	qsort(counties, n, sizeof(Parcel), compareCounties);

	int last;
	for (int i = 0; i < n; i = last + 1)
	{
		last = findLast(counties, n, i, counties[i].identification, 2);
		printf("%s\n", counties[i].identification.concelho);
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

static int compareDistricts(const void *av, const void *bv)
{
	const Parcel *a = av, *b = bv;
	return strcmp(a->identification.distrito, b->identification.distrito);
}

static void commandDistricts(Cartography cartography, int n)
{
	// Allocate size for the copy
	Parcel *districts = malloc(sizeof(Parcel) * n);
	// Copy the vector
	memcpy(districts, cartography, sizeof(Parcel) * n);

	// Sort the vector
	qsort(districts, n, sizeof(Parcel), compareDistricts);

	int last;
	for (int i = 0; i < n; i = last + 1)
	{
		last = findLast(districts, n, i, districts[i].identification, 1);
		printf("%s\n", districts[i].identification.distrito);
	}

	free(districts);
}

int inParcel(double lat, double lon, Cartography cartography, int n)
{
	Coordinates c = coord(lat, lon);
	for (int i = 0; i < n; i++)
	{
		// TODO calculatBoundingBox
		Ring r = cartography[i].edge;
		if (insideRectangle(c, calculateBoundingBox(r.vertexes, r.nVertexes)))
		{
			if (insideParcel(c, cartography[i]))
			{
				return i;
			}
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
	int i;
	for (i = 0; i < n; i++) {
		// TODO CHANGE ADJACENTPARCELS TO NOT RECOGNIZE ITSELF
		// PROBABLY DON'T GO OVER EVERY PARCEL <--------------
		if (!sameIdentification(p.identification, cartography[i].identification, 3)
				&& adjacentParcels(p, cartography[i])) {
			showIdentification(i, cartography[i].identification, 3);
			printf("\n");
		}
	}

}

static void commandBoundaries(int pos1, int pos2, Cartography cartography, int n)
{
}

static void commandPartition(double dist, Cartography cartography, int n)
{
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
