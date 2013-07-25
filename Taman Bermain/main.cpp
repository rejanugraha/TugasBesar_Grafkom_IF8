#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>
#include "imageloader.h"
#include "vec3f.h"
#endif

static GLfloat spin, spin2 = 0.0;
float angle = 0;

float awal=0.0, akhir=0.0, atas=0.0, bawah=0.0;
float awal2=0.0, akhir2=0.0, atas2=0.0, bawah2=0.0;
float sudut=0.0;
float rotasi=0.0, rotasi2=0.0, rotasi3=0.0, putar=0.0;
float geser=0.0;

//kicir
float r_kkicir=0.0, r_kkicir2=0.0, r_kkicir3=0.0, r_kkicir4=0.0;
float b_kicir=0.0, b_kicir2=0.0;
float t_kicir=0.0, t_kicir2=0.0;

//cylinder
GLUquadricObj *p = gluNewQuadric();
//mobil
float gerak=0.0;



using namespace std;

float lastx, lasty;
GLint stencilBits;


static int viewx = -177;
static int viewy = 80;
static int viewz = 176;


float rot = 0;


GLuint texture[10];

struct Gambar {
	unsigned long sizeX;
	unsigned long sizeY;
	char *data;
};
typedef struct Gambar Gambar; //struktur data untuk


//ukuran gambar #bisa di set sesuai kebutuhan


//mengambil gambar BMP
int GambarLoad(char *filename, Gambar *gambar) {
	FILE *file;
	unsigned long size; // ukuran gambar dalam bytes
	unsigned long i; // standard counter.
	unsigned short int plane; // number of planes in gambar

	unsigned short int bpp; // jumlah bits per pixel
	char temp; // temporary color storage for var warna sementara untuk memastikan filenya ada


	if ((file = fopen(filename, "rb")) == NULL) {
		printf("File Not Found : %s\n", filename);
		return 0;
	}
	// mencari file header bmp
	fseek(file, 18, SEEK_CUR);
	// read the width
	if ((i = fread(&gambar->sizeX, 4, 1, file)) != 1) {
		printf("Error reading width from %s.\n", filename);
		return 0;
	}
	//printf("Width of %s: %lu\n", filename, gambar->sizeX);
	// membaca nilai height
	if ((i = fread(&gambar->sizeY, 4, 1, file)) != 1) {
		printf("Error reading height from %s.\n", filename);
		return 0;
	}
	//printf("Height of %s: %lu\n", filename, gambar->sizeY);
	//menghitung ukuran gambar(asumsi 24 bits or 3 bytes per pixel).

	size = gambar->sizeX * gambar->sizeY * 3;
	// read the planes
	if ((fread(&plane, 2, 1, file)) != 1) {
		printf("Error reading planes from %s.\n", filename);
		return 0;
	}
	if (plane != 1) {
		printf("Planes from %s is not 1: %u\n", filename, plane);
		return 0;
	}
	// read the bitsperpixel
	if ((i = fread(&bpp, 2, 1, file)) != 1) {
		printf("Error reading bpp from %s.\n", filename);

		return 0;
	}
	if (bpp != 24) {
		printf("Bpp from %s is not 24: %u\n", filename, bpp);
		return 0;
	}
	// seek past the rest of the bitmap header.
	fseek(file, 24, SEEK_CUR);
	// read the data.
	gambar->data = (char *) malloc(size);
	if (gambar->data == NULL) {
		printf("Error allocating memory for color-corrected gambar data");
		return 0;
	}
	if ((i = fread(gambar->data, size, 1, file)) != 1) {
		printf("Error reading gambar data from %s.\n", filename);
		return 0;
	}
	for (i = 0; i < size; i += 3) { // membalikan semuan nilai warna (gbr - > rgb)
		temp = gambar->data[i];
		gambar->data[i] = gambar->data[i + 2];
		gambar->data[i + 2] = temp;
	}
	// we're done.
	return 1;
}

//mengambil tekstur
Gambar * loadTexture() {
	Gambar *gambar1;
	// alokasi memmory untuk tekstur
	gambar1 = (Gambar *) malloc(sizeof(Gambar));
	if (gambar1 == NULL) {
		printf("Error allocating space for gambar");
		exit(0);
	}
	//pic.bmp is a 64x64 picture
	if (!GambarLoad("welcome.bmp", gambar1)) {
		exit(1);
	}
	return gambar1;
}

Gambar * loadTexture2() {
	Gambar *gambar2;
	// alokasi memmory untuk tekstur
	gambar2 = (Gambar *) malloc(sizeof(Gambar));
	if (gambar2 == NULL) {
		printf("Error allocating space for gambar");
		exit(0);
	}
	//pic.bmp is a 64x64 picture
	if (!GambarLoad("images.bmp", gambar2)) {
		exit(1);
	}
	return gambar2;
}







//train 2D
//class untuk terain 2D
class Terrain {
private:
	int w; //Width
	int l; //Length
	float** hs; //Heights
	Vec3f** normals;
	bool computedNormals; //Whether normals is up-to-date
public:
	Terrain(int w2, int l2) {
		w = w2;
		l = l2;

		hs = new float*[l];
		for (int i = 0; i < l; i++) {
			hs[i] = new float[w];
		}

		normals = new Vec3f*[l];
		for (int i = 0; i < l; i++) {
			normals[i] = new Vec3f[w];
		}

		computedNormals = false;
	}

	~Terrain() {
		for (int i = 0; i < l; i++) {
			delete[] hs[i];
		}
		delete[] hs;

		for (int i = 0; i < l; i++) {
			delete[] normals[i];
		}
		delete[] normals;
	}

	int width() {
		return w;
	}

	int length() {
		return l;
	}

	//Sets the height at (x, z) to y
	void setHeight(int x, int z, float y) {
		hs[z][x] = y;
		computedNormals = false;
	}

	//Returns the height at (x, z)
	float getHeight(int x, int z) {
		return hs[z][x];
	}

	//Computes the normals, if they haven't been computed yet
	void computeNormals() {
		if (computedNormals) {
			return;
		}

		//Compute the rough version of the normals
		Vec3f** normals2 = new Vec3f*[l];
		for (int i = 0; i < l; i++) {
			normals2[i] = new Vec3f[w];
		}

		for (int z = 0; z < l; z++) {
			for (int x = 0; x < w; x++) {
				Vec3f sum(0.0f, 0.0f, 0.0f);

				Vec3f out;
				if (z > 0) {
					out = Vec3f(0.0f, hs[z - 1][x] - hs[z][x], -1.0f);
				}
				Vec3f in;
				if (z < l - 1) {
					in = Vec3f(0.0f, hs[z + 1][x] - hs[z][x], 1.0f);
				}
				Vec3f left;
				if (x > 0) {
					left = Vec3f(-1.0f, hs[z][x - 1] - hs[z][x], 0.0f);
				}
				Vec3f right;
				if (x < w - 1) {
					right = Vec3f(1.0f, hs[z][x + 1] - hs[z][x], 0.0f);
				}

				if (x > 0 && z > 0) {
					sum += out.cross(left).normalize();
				}
				if (x > 0 && z < l - 1) {
					sum += left.cross(in).normalize();
				}
				if (x < w - 1 && z < l - 1) {
					sum += in.cross(right).normalize();
				}
				if (x < w - 1 && z > 0) {
					sum += right.cross(out).normalize();
				}

				normals2[z][x] = sum;
			}
		}

		//Smooth out the normals
		const float FALLOUT_RATIO = 0.5f;
		for (int z = 0; z < l; z++) {
			for (int x = 0; x < w; x++) {
				Vec3f sum = normals2[z][x];

				if (x > 0) {
					sum += normals2[z][x - 1] * FALLOUT_RATIO;
				}
				if (x < w - 1) {
					sum += normals2[z][x + 1] * FALLOUT_RATIO;
				}
				if (z > 0) {
					sum += normals2[z - 1][x] * FALLOUT_RATIO;
				}
				if (z < l - 1) {
					sum += normals2[z + 1][x] * FALLOUT_RATIO;
				}

				if (sum.magnitude() == 0) {
					sum = Vec3f(0.0f, 1.0f, 0.0f);
				}
				normals[z][x] = sum;
			}
		}

		for (int i = 0; i < l; i++) {
			delete[] normals2[i];
		}
		delete[] normals2;

		computedNormals = true;
	}

	//Returns the normal at (x, z)
	Vec3f getNormal(int x, int z) {
		if (!computedNormals) {
			computeNormals();
		}
		return normals[z][x];
	}
};
//end class



//Loads a terrain from a heightmap.  The heights of the terrain range from
//-height / 2 to height / 2.
//load terain di procedure inisialisasi
Terrain* loadTerrain(const char* filename, float height) {
	Image* image = loadBMP(filename);
	Terrain* t = new Terrain(image->width, image->height);
	for (int y = 0; y < image->height; y++) {
		for (int x = 0; x < image->width; x++) {
			unsigned char color = (unsigned char) image->pixels[3 * (y
					* image->width + x)];
			float h = height * ((color / 255.0f) - 0.5f);
			t->setHeight(x, y, h);
		}
	}

	delete image;
	t->computeNormals();
	return t;
}

float _angle = 60.0f;
//buat tipe data terain
Terrain* _terrain;
Terrain* _terrainAir;
Terrain* _terrainStreet;
Terrain* _terrainKolam;

const GLfloat light_ambient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
const GLfloat light_diffuse[] = { 0.7f, 0.7f, 0.7f, 1.0f };
const GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_position[] = { 1.0f, 1.0f, 1.0f, 1.0f };

const GLfloat light_ambient2[] = { 0.3f, 0.3f, 0.3f, 0.0f };
const GLfloat light_diffuse2[] = { 0.3f, 0.3f, 0.3f, 0.0f };

const GLfloat mat_ambient[] = { 0.8f, 0.8f, 0.8f, 1.0f };
const GLfloat mat_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
const GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat high_shininess[] = { 100.0f };

void cleanup() {
	delete _terrain;
	//delete _terrainTanah;
}

//untuk di display
void drawSceneTanah(Terrain *terrain, GLfloat r, GLfloat g, GLfloat b) {

	float scale = 500.0f / max(terrain->width() - 1, terrain->length() - 1);
	glScalef(scale, scale, scale);
	glTranslatef(-(float) (terrain->width() - 1) / 2, 0.0f,
			-(float) (terrain->length() - 1) / 2);

	glColor3f(r, g, b);
	for (int z = 0; z < terrain->length() - 1; z++) {
		//Makes OpenGL draw a triangle at every three consecutive vertices
		glBegin(GL_TRIANGLE_STRIP);
		for (int x = 0; x < terrain->width(); x++) {
			Vec3f normal = terrain->getNormal(x, z);
			glNormal3f(normal[0], normal[1], normal[2]);
			glVertex3f(x, terrain->getHeight(x, z), z);
			normal = terrain->getNormal(x, z + 1);
			glNormal3f(normal[0], normal[1], normal[2]);
			glVertex3f(x, terrain->getHeight(x, z + 1), z + 1);
		}
		glEnd();
	}
}

unsigned int LoadTextureFromBmpFile(char *filename);




//cylinder
void cylinder()
{
//tutup atas
glDisable(GL_CULL_FACE);
glPushMatrix();
glTranslatef(0,0,0);
gluQuadricOrientation(p, GLU_INSIDE);
gluDisk(p, 0, 1, 360, 1);
glPopMatrix();
glEnable(GL_CULL_FACE);


//tabung
glDisable(GL_CULL_FACE);
glPushMatrix();
glTranslatef(0,0,0);
gluQuadricOrientation(p, GLU_OUTSIDE);
gluCylinder(p, 1, 1,1, 360, 1);
glPopMatrix();
glEnable(GL_CULL_FACE);


//tutup bawah
glDisable(GL_CULL_FACE);
glPushMatrix();
glTranslatef(0,0,1);
gluDisk(p, 0, 1, 360, 1);
glPopMatrix();
glEnable(GL_CULL_FACE);

}

void cylinder2()
{

//tutup atas
glPushMatrix();;
glTranslatef(0,0,0);
gluQuadricDrawStyle(p, GLU_FILL);
gluQuadricOrientation(p, GLU_INSIDE);
gluDisk(p, 0, 1, 8, 1);
glPopMatrix();

glPushMatrix();
glTranslatef(0,0,0);
gluQuadricDrawStyle(p, GLU_FILL);
gluQuadricOrientation(p, GLU_OUTSIDE);
gluDisk(p, 0, 1, 8, 1);
glPopMatrix();

//tabung
glPushMatrix();
glTranslatef(0,0,0);
gluQuadricDrawStyle(p, GLU_FILL);
gluQuadricOrientation(p, GLU_INSIDE);
gluCylinder(p, 1, 1,1, 8, 1);
glPopMatrix();

glPushMatrix();
glTranslatef(0,0,0);
gluQuadricDrawStyle(p, GLU_FILL);
gluQuadricOrientation(p, GLU_OUTSIDE);
gluCylinder(p, 1, 1,1, 8, 1);
glPopMatrix();

//tutup bawah
glPushMatrix();
glTranslatef(0,0,1);
gluQuadricDrawStyle(p, GLU_FILL);
gluQuadricOrientation(p, GLU_INSIDE);
gluDisk(p, 0, 1, 8, 1);
glPopMatrix();

glPushMatrix();
glTranslatef(0,0,1);
gluQuadricDrawStyle(p, GLU_FILL);
gluQuadricOrientation(p, GLU_OUTSIDE);
gluDisk(p, 0, 1, 8, 1);
glPopMatrix();
}



//---------------- BIANGLALA
void bianglala()
{

glColor3f(244.0f/255.0f,0.0f/255.0f,0.0f/255.0f);
//lingkar utama
glPushMatrix();
glTranslatef(0,10,0);
glRotatef(90,1,0,0);
glutSolidTorus(0.2,10,50,50);
glPopMatrix();

glColor3f(0.0f/255.0f,0.0f/255.0f,255.0f/255.0f);
//lingkar dalam
glDisable(GL_CULL_FACE);
glPushMatrix();
glTranslatef(0,10,0);
glRotatef(90,1,0,0);
gluDisk(p, 4, 5, 360, 1);
glPopMatrix();
glEnable(GL_CULL_FACE);


glColor3f(255.0f/255.0f,255.0f/255.0f,0.0f/255.0f);
//tiang horizon
glPushMatrix();
glTranslatef(5,10,0);
glScalef(10,0.5,0.5);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslatef(-5,10,0);
glScalef(10,0.5,0.5);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslatef(0,10,5);
glRotatef(90,0,1,0);
glScalef(10,0.5,0.5);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslatef(0,10,-5);
glRotatef(270,0,1,0);
glScalef(10,0.5,0.5);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslatef(-2.5,10,-2.5);
glRotatef(135,0,1,0);
glScalef(13,0.5,0.5);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslatef(2.5,10,-2.5);
glRotatef(45,0,1,0);
glScalef(13,0.5,0.5);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslatef(-2.5,10,2.5);
glRotatef(225,0,1,0);
glScalef(13,0.5,0.5);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslatef(2.5,10,2.5);
glRotatef(135,0,1,0);
glScalef(13,0.5,0.5);
glutSolidCube(1);
glPopMatrix();
}

void tiangbianglala()
{
 glColor3f(64.0f/255.0f,128.0f/255.0f,128.0f/255.0f);
//tiang kecil
glPushMatrix();
glTranslatef(7,11,7);
glRotatef(90,0,0,1);
glScalef(2,0.5,0.5);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslatef(-7,11,7);
glRotatef(90,0,0,1);
glScalef(2,0.5,0.5);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslatef(7,11,-7);
glRotatef(90,0,0,1);
glScalef(2,0.5,0.5);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslatef(-7,11,-7);
glRotatef(90,0,0,1);
glScalef(2,0.5,0.5);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslatef(10,11,0);
glRotatef(90,0,0,1);
glScalef(2,0.5,0.5);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslatef(-10,11,0);
glRotatef(90,0,0,1);
glScalef(2,0.5,0.5);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslatef(0,11,10);
glRotatef(90,0,0,1);
glScalef(2,0.5,0.5);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslatef(0,11,-10);
glRotatef(90,0,0,1);
glScalef(2,0.5,0.5);
glutSolidCube(1);
glPopMatrix();
}

void duduk()
{

//tutup atas
glPushMatrix();
glTranslatef(0,0,0);
gluQuadricDrawStyle(p, GLU_FILL);
gluQuadricOrientation(p, GLU_INSIDE);
gluDisk(p, 0, 1, 5, 1);
glPopMatrix();

glPushMatrix();
glTranslatef(0,0,0);
gluQuadricDrawStyle(p, GLU_FILL);
gluQuadricOrientation(p, GLU_OUTSIDE);
gluDisk(p, 0, 1, 5, 1);
glPopMatrix();

//tabung
glPushMatrix();
glTranslatef(0,0,0);
gluQuadricDrawStyle(p, GLU_FILL);
gluQuadricOrientation(p, GLU_INSIDE);
gluCylinder(p, 1, 1,1, 5, 1);
glPopMatrix();

glPushMatrix();
glTranslatef(0,0,0);
gluQuadricDrawStyle(p, GLU_FILL);
gluQuadricOrientation(p, GLU_OUTSIDE);
gluCylinder(p, 1, 1,1, 5, 1);
glPopMatrix();

//tutup bawah
glPushMatrix();
glTranslatef(0,0,1);
gluQuadricDrawStyle(p, GLU_FILL);
gluQuadricOrientation(p, GLU_INSIDE);
gluDisk(p, 0, 1, 5, 1);
glPopMatrix();

glPushMatrix();
glTranslatef(0,0,1);
gluQuadricDrawStyle(p, GLU_FILL);
gluQuadricOrientation(p, GLU_OUTSIDE);
gluDisk(p, 0, 1, 5, 1);
glPopMatrix();

}


void bianglala2()
{
const double t = glutGet(GLUT_ELAPSED_TIME) / 900.0;
const double a = t*90.0;
 glPushMatrix();
 glTranslatef(0,-3,0);
 bianglala();
 glPopMatrix();

 glPushMatrix();
 glTranslatef(0,-5,0);
 bianglala();
 glPopMatrix();

 glPushMatrix();
 glTranslatef(0,-5,0);
 tiangbianglala();
 glPopMatrix();

 glColor3f(0.0f/255.0f,128.0f/255.0f,0.0f/255.0f);
 glPushMatrix();
 glTranslated(12,7,0);
 glRotated(90,1,0,0);
 glRotated(a,0,0,1);
 glScaled(2,2,2);
 duduk();
 glPopMatrix();

 glColor3f(128.0f/255.0f,64.0f/255.0f,64.0f/255.0f);
 glPushMatrix();
 glTranslated(-12,7,0);
 glRotated(90,1,0,0);
 glRotated(a,0,0,1);
 glScaled(2,2,2);
 duduk();
 glPopMatrix();

 glColor3f(255.0f/255.0f,128.0f/255.0f,0.0f/255.0f);
 glPushMatrix();
 glTranslated(0,7,12);
 glRotated(90,1,0,0);
 glRotated(a,0,0,1);
 glScaled(2,2,2);
 duduk();
 glPopMatrix();

 glColor3f(0.0f/255.0f,0.0f/255.0f,128.0f/255.0f);
 glPushMatrix();
 glTranslated(0,7,-12);
 glRotated(90,1,0,0);
 glRotated(a,0,0,1);
 glScaled(2,2,2);
 duduk();
 glPopMatrix();

 glColor3f(255.0f/255.0f,0.0f/255.0f,255.0f/255.0f);
 glPushMatrix();
 glTranslated(-8.4,7,-8.4);
 glRotated(90,1,0,0);
 glRotated(a,0,0,1);
 glScaled(2,2,2);
 duduk();
 glPopMatrix();

 glColor3f(64.0f/255.0f,0.0f/255.0f,64.0f/255.0f);
 glPushMatrix();
 glTranslated(8.4,7,-8.4);
 glRotated(90,1,0,0);
 glRotated(a,0,0,1);
 glScaled(2,2,2);
 duduk();
 glPopMatrix();

 glColor3f(128.0f/255.0f,64.0f/255.0f,0.0f/255.0f);
 glPushMatrix();
 glTranslated(-8.4,7,8.4);
 glRotated(90,1,0,0);
 glRotated(a,0,0,1);
 glScaled(2,2,2);
 duduk();
 glPopMatrix();

 glColor3f(128.0f/255.0f,255.0f/255.0f,255.0f/255.0f);
 glPushMatrix();
 glTranslated(8.4,7,8.4);
 glRotated(90,1,0,0);
 glRotated(a,0,0,1);
 glScaled(2,2,2);
 duduk();
 glPopMatrix();

}

void tiangutama()
{
 glColor3f(64.0f/255.0f,128.0f/255.0f,128.0f/255.0f);
 //tiang penyangga
glPushMatrix();
glTranslated(0,2.3,0);
glRotated(270,1,0,0);
glScaled(0.7,0.7,7);
cylinder();
glPopMatrix();

 glColor3f(64.0f/255.0f,128.0f/255.0f,128.0f/255.0f);
//tiang penyangga utama
glPushMatrix();
glTranslatef(8,9,1.5);
glRotatef(90,1,0,0);
glRotated(10,0,0,1);
glScalef(17,0.5,0.5);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslatef(8,9,-1.5);
glRotatef(90,1,0,0);
glRotated(350,0,0,1);
glScalef(17,0.5,0.5);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslatef(8,2.5,1.5);
glRotatef(90,1,0,0);
glRotated(10,0,0,1);
glScalef(17,0.5,0.5);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslatef(8,2.5,-1.5);
glRotatef(90,1,0,0);
glRotated(350,0,0,1);
glScalef(17,0.5,0.5);
glutSolidCube(1);
glPopMatrix();

}

void bianglala3()
{
const double t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
const double a = t*90.0;

glPushMatrix();
glTranslatef(0,30,-60);
glRotated(90,1,0,0);
glRotated(a,0,1,0);
glScalef(2,2,2);
bianglala2();
glPopMatrix();

glPushMatrix();
glTranslatef(0,30,-60);
glRotated(90,1,0,0);
glRotated(270,0,1,0);
glScalef(2,2,2);
tiangutama();
glPopMatrix();
}




//PAPAN
void papan()
{

//tiang kiri
glPushMatrix();
glTranslatef(-15.5,-10,5);
glRotated(270, 1, 0, 0);
glScalef(0.5,0.5,25.3);
cylinder();
glPopMatrix();

//tiang kanan
glPushMatrix();
glTranslatef(15.5,-10,5);
glRotated(270, 1, 0, 0);
glScalef(0.5,0.5,25.3);
cylinder();
glPopMatrix();

//papan
glPushMatrix();
glTranslatef(0,12,5);
glScalef(30,8,0.5);
glutSolidCube(1);
glPopMatrix();

}

//BUS
void bus()
{
    //Bodi
    glColor3f(1, 0, 0);
    glPushMatrix();
    //glRotatef(sudutk, 0.0, 0.0, 1.0);
    glTranslatef(0.0, 3.8, 0.0);
    glScalef(4.0, 1.0, 1.5);
    glutSolidCube(6.0f);
    glPopMatrix();

    //Kaca Belakang
    glColor3f(0, 1, 1);
    glPushMatrix();
    glTranslatef(-12.0, 5.0, 0.0);
    glScalef(0.05, 0.5, 1.2);
    glutSolidCube(6.0f);
    glPopMatrix();

    //Lampu Rem
    glColor3f(1, 1, 1);
    glPushMatrix();
    glTranslatef(-12.1, 2.0, -3.5);
    glScalef(0.02, 0.19, 0.08);
    glutSolidCube(6.0f);
    glPopMatrix();

    glColor3f(1, 1, 1);
    glPushMatrix();
    glTranslatef(-12.1, 2.0, 3.5);
    glScalef(0.02, 0.19, 0.08);
    glutSolidCube(6.0f);
    glPopMatrix();

    //Lampu Depan
    glColor3f(1.0, 1.0, 1.0);
    glPushMatrix();
    glTranslatef(12.1, 2.0, -3.5);
    glScalef(0.05, 0.02, 0.1);
    glutSolidCube(6.0f);
    glPopMatrix();

    glColor3f(1.0, 1.0, 1.0);
    glPushMatrix();
    glTranslatef(12.1, 2.0, 3.5);
    glScalef(0.05, 0.02, 0.1);
    glutSolidCube(6.0f);
    glPopMatrix();

    //Kaca Depan
    glColor3f(0, 1, 1);
    glPushMatrix();
    glTranslatef(12.0, 5.0, 0.0);
    glScalef(0.05, 0.5, 1.2);
    glutSolidCube(6.0f);
    glPopMatrix();
    //Kaca pinggir
    glColor3f(0, 1, 1);
    glPushMatrix();
    glTranslatef(-1.5, 5.0, 4.0);
    glScalef(3.3, 0.5, 0.2);
    glutSolidCube(6.0f);
    glPopMatrix();

    glColor3f(0, 1, 1);
    glPushMatrix();
    glTranslatef(-1.5, 5.0, -4.0);
    glScalef(3.3, 0.5, 0.2);
    glutSolidCube(6.0f);
    glPopMatrix();
    //Pintu Kanan
    glColor3f(0.5, 0.5, 0.5);
    glPushMatrix();
    glTranslatef(11.1, 3.9, 4.0);
    glScalef(0.15, 0.8, 0.2);
    glutSolidCube(6.0f);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.5, 0.5, 0.5);
    glTranslatef(10.1, 5.1, 4.0);
    glScalef(0.25, 0.4, 0.2);
    glutSolidCube(6.0f);
    glPopMatrix();
    //Pintu Kiri
    glColor3f(0.5, 0.5, 0.5);
    glPushMatrix();
    glTranslatef(11.1, 3.9, -4.0);
    glScalef(0.15, 0.8, 0.2);
    glutSolidCube(6.0f);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.5, 0.5, 0.5);
    glTranslatef(10.1, 5.1, -4.0);
    glScalef(0.25, 0.4, 0.2);
    glutSolidCube(6.0f);
    glPopMatrix();

    //Ban Belakang
    glPushMatrix();
    glColor3f(1.0, 1.0, 1.0);
    glScalef(0.35, 0.35, 0.25);
    glTranslatef(25.0, 4.5, 19.3);
    glutSolidTorus(2, 3, 20, 30);
    glPopMatrix();
    //Velg
    glPushMatrix();
    glColor3f(0.0, 0.0, 0.0);
    glScalef(0.5, 0.5, 0.5);
    glTranslatef(17.5, 2.5, 8.0);
    glutSolidSphere(2, 10, 20);
    glPopMatrix();

    glPushMatrix();
    glColor3f(1.0, 1.0, 1.0);
    glScalef(0.35, 0.35, 0.25);
    glTranslatef(25.0, 4.5, -19.3);
    glutSolidTorus(2, 3, 20, 30);
    glPopMatrix();
    //Velg
    glPushMatrix();
    glColor3f(0.0, 0.0, 0.0);
    glScalef(0.5, 0.5, 0.5);
    glTranslatef(-17.5, 2.5, 8.0);
    glutSolidSphere(2, 10, 20);
    glPopMatrix();


    //Ban Depan
    glPushMatrix();
    glColor3f(1.0, 1.0, 1.0);
    glScalef(0.35, 0.35, 0.25);
    glTranslatef(-25.0, 4.5, -19.3);
    glutSolidTorus(2, 3, 20, 30);
    glPopMatrix();
    //Velg
    glPushMatrix();
    glColor3f(0.0, 0.0, 0.0);
    glScalef(0.5, 0.5, 0.5);
    glTranslatef(17.5, 2.5, -8.0);
    glutSolidSphere(2, 10, 20);
    glPopMatrix();

    glPushMatrix();
    glColor3f(1.0, 1.0, 1.0);
    glScalef(0.35, 0.35, 0.25);
    glTranslatef(-25.0, 4.5, 19.3);
    glutSolidTorus(2, 3, 20, 30);
    glPopMatrix();
    //Velg
    glPushMatrix();
    glColor3f(0.0, 0.0, 0.0);
    glScalef(0.5, 0.5, 0.5);
    glTranslatef(-17.5, 2.5, -8.0);
    glutSolidSphere(2, 10, 20);
    glPopMatrix();
}

//PAGAR
void pagar()
{
    //Pagar Atas
    glPushMatrix();
	glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
    glTranslatef(9, 1, 0);
    glScaled(40.0, 1.0 , 0.5);
    glutSolidCube(0.5f);
    glPopMatrix();

    //Pagar Bawah
    glPushMatrix();
	glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
    glTranslatef(9, 2, 0.0);
    glScaled(40.0, 1.0 , 0.5);
    glutSolidCube(0.5f);
    glPopMatrix();

    //Pagar Bawah
    glPushMatrix();
	glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
    glTranslatef(9, 3, 0.0);
    glScaled(40.0, 1.0 , 0.5);
    glutSolidCube(0.5f);
    glPopMatrix();

        //Pagar Bawah
    glPushMatrix();
	glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
    glTranslatef(9, 4, 0.0);
    glScaled(40.0, 1.0 , 0.5);
    glutSolidCube(0.5f);
    glPopMatrix();

    //Pagar Tegak
    glPushMatrix();
	glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
    glTranslatef(0.0f, 0.0f, 0.0f);
    glScaled(1.5, 18 , 0.5);
    glutSolidCube(0.5f);
    glPopMatrix();

        //Pagar Tegak
    glPushMatrix();
	glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
    glTranslatef(18, 0, 0);
    glScaled(1.5, 18 , 0.5);
    glutSolidCube(0.5f);
    glPopMatrix();
}



void markajalan(void) {

    // marka jalan
    glPushMatrix();
    glScaled(1, 0.05,0.3);
   glTranslatef(2.4,2.5,67);
   glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
     glColor3f(1,1,1);
    glutSolidCube(5.0);
    glPopMatrix();
}

void pohon(){
	glColor3ub(104,70,14);
	//<<<<<<<<<<<<<<<<<<<< Batang >>>>>>>>>>>>>>>>>>>>>>>
	glPushMatrix();
	glScalef(0.2, 2, 0.2);
	glutSolidSphere(1.0, 10, 16);
	glPopMatrix();
	//<<<<<<<<<<<<<<<<<<<< end Batang >>>>>>>>>>>>>>>>>>>>>>>

	glColor3ub(18,118,13);
	//<<<<<<<<<<<<<<<<<<<< Daun >>>>>>>>>>>>>>>>>>>>>>>
	glPushMatrix();
	glScalef(1.5, 1, 1.5);
	glTranslatef(0, 1, 0);
	glRotatef(270, 1, 0, 0);
	glutSolidCone(1,3,10,1);
	glPopMatrix();

	glPushMatrix();
	glScalef(1.4, 1, 1.4);
	glTranslatef(0, 1.7, 0);
	glRotatef(270, 1, 0, 0);
	glutSolidCone(1,2,10,1);
	glPopMatrix();

	glPushMatrix();
	glScalef(1.2, 1, 1.2);
	glTranslatef(0, 2.4, 0);
	glRotatef(270, 1, 0, 0);
	glutSolidCone(1,1.8,10,1);
	glPopMatrix();


}

void awan(){
glPushMatrix();
glColor3ub(153, 223, 255);
glutSolidSphere(10, 50, 50);
glPopMatrix();
glPushMatrix();
glTranslatef(10,0,1);
glutSolidSphere(5, 50, 50);
glPopMatrix();
glPushMatrix();
glTranslatef(-2,6,-2);
glutSolidSphere(7, 50, 50);
glPopMatrix();
glPushMatrix();
glTranslatef(-10,-3,0);
glutSolidSphere(7, 50, 50);
glPopMatrix();
glPushMatrix();
glTranslatef(6,-2,2);
glutSolidSphere(7, 50, 50);
glPopMatrix();
}

void lampu()
{
   //Tiang Tegak
	glPushMatrix();
	glColor3f(0.5, 0.5, 0.5);
	glScalef(0.04,1.7,0.05);
	glutSolidCube(7.0f);
	glPopMatrix();

    //Tiang Atas
	glPushMatrix();
	glColor3f(0.5f, 0.5f, 0.5f);
	glTranslatef(0.0,5.3,-2.0);
    glScaled(0.5, 1.0 , 7.5);
    glutSolidCube(0.5f);
	glPopMatrix();

	//Lampu
	glPushMatrix();
	glTranslatef(0.0, 4.7, -3.7);
	glColor3f(1, 1, 1);
	glScalef(0.8,0.8,1.5);
	glutSolidSphere(0.5,70,20);
	glPopMatrix();

}


//-------------- KORA-KORA
void kora2()
{

//tabung
glEnable(GL_CULL_FACE);
glColor3f(113.0f/255.0f,56.0f/255.0f,0.0f/255.0f);
glPushMatrix();
glTranslatef(0,0,0);
gluCylinder(p, 1, 2,0.9, 6, 1);
glPopMatrix();
glDisable(GL_CULL_FACE);


//sisi
glColor3f(128.0f/255.0f,128.0f/255.0f,128.0f/255.0f);
glPushMatrix();
glTranslated(0,0,0.9);
glRotatef(90,0,0,1);
glScaled(1.7,1.7,1);
glutSolidTorus(0.2,1,8,6);
glPopMatrix();


//tutup
glEnable(GL_CULL_FACE);
glColor3f(113.0f/255.0f,56.0f/255.0f,0.0f/255.0f);
glPushMatrix();
glTranslatef(0,0,0);
gluQuadricOrientation(p,GLU_INSIDE);
gluDisk(p, 0, 1, 6, 1);
glPopMatrix();
glDisable(GL_CULL_FACE);

glEnable(GL_CULL_FACE);
glColor3f(113.0f/255.0f,56.0f/255.0f,0.0f/255.0f);
glPushMatrix();
glTranslatef(0,0,0.7);
gluQuadricOrientation(p,GLU_OUTSIDE);
gluDisk(p, 0, 1.7, 6, 1);
glPopMatrix();
glDisable(GL_CULL_FACE);


}

void tiangkora()
{

    glColor3f(64.0f/255.0f,128.0f/255.0f,128.0f/255.0f);
    glPushMatrix();
    glTranslatef(2.5,25,5.5);
    glRotated(110,1,0,0);
    glScaled(0.5,0.5,15);
    cylinder();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-2.5,25,5.5);
    glRotated(110,1,0,0);
    glScaled(0.5,0.5,15);
    cylinder();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-2.5,25,-5.5);
    glRotated(70,1,0,0);
    glScaled(0.5,0.5,15);
    cylinder();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(2.5,25,-5.5);
    glRotated(70,1,0,0);
    glScaled(0.5,0.5,15);
    cylinder();
    glPopMatrix();


}

void katrol()
{
//katrol
glColor3f(176.0f/255.0f,0.0f/255.0f,0.0f/255.0f);
glPushMatrix();
glTranslatef(-6,40,0);
glRotated(90,0,1,0);
glScalef(1,1,12);
cylinder();
glPopMatrix();

glColor3f(176.0f/255.0f,0.0f/255.0f,0.0f/255.0f);
glPushMatrix();
glTranslatef(5.35,40,0);
glScaled(1.3,1.3,15);
glutSolidCube(1);
glPopMatrix();

glColor3f(176.0f/255.0f,0.0f/255.0f,0.0f/255.0f);
glPushMatrix();
glTranslatef(-5.35,40,0);
glScaled(1.3,1.3,15);
glutSolidCube(1);
glPopMatrix();

glColor3f(64.0f/255.0f,128.0f/255.0f,128.0f/255.0f);
glPushMatrix();
glTranslatef(-5.35,25,-12.2);
glRotated(110,1,0,0);
glScaled(1.3,1.3,32);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslatef(5.35,25,-12.2);
glRotated(110,1,0,0);
glScaled(1.3,1.3,32);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslatef(5.35,25,12.2);
glRotated(70,1,0,0);
glScaled(1.3,1.3,32);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslatef(-5.35,25,12.2);
glRotated(70,1,0,0);
glScaled(1.3,1.3,32);
glutSolidCube(1);
glPopMatrix();




}

void hasil()
{

glPushMatrix();
glTranslated(0,40,0);
glRotated(180,0,1,0);
glRotated(180,1,0,0);



if (rotasi<=90) {
    rotasi+=5;
    glRotated(rotasi,1,0,0);
    if (rotasi==90) {rotasi2=90;}
}
else if (rotasi2>=-90) {
    rotasi2-=5;
    glRotated(rotasi2,1,0,0);
    if (rotasi2==-90) {rotasi3=-90;}
}
else if (rotasi3<=90) {
    rotasi3+=5;
    glRotated(rotasi3,1,0,0);
    if (rotasi3==90) {rotasi2=90;}
}


glTranslated(0,-10,0);
tiangkora();
glPushMatrix();
glTranslated(0,30,0);
glRotated(90,1,0,0);
glScaled(2,6,5);
kora2();
glPopMatrix();
glPopMatrix();

glPushMatrix();

glTranslated(0,0,0);
katrol();
glPopMatrix();


}

//-------------------------------ONTANG-ANTING------------------------------------------
//--------------------------------------------------------------------------------------
void kursiontang()
{
    //glColor3f(0.0f/255.0f,128.0f/255.0f,64.0f/255.0f);
    glPushMatrix();
    glTranslated(0,0,0);
    glScaled(1,1,0.5);
    cylinder();
    glPopMatrix();

    glPushMatrix();
    glTranslated(0,-0.8,0.25);
    glScaled(2,1.5,0.5);
    glutSolidCube(1);
    glPopMatrix();

    glPushMatrix();
    glTranslated(0,-1.3,-0.9);
    glRotated(270,1,0,0);
    glScaled(2,2,0.5);
    glutSolidCube(1);
    glPopMatrix();

    glPushMatrix();
    glTranslated(1,-1.3,-0.6);
    glRotated(90,0,1,0);
    glScaled(1,1,1);
    glutSolidTorus(0.2,1,50,50);
    glPopMatrix();

    glPushMatrix();
    glTranslated(-1,-1.3,-0.6);
    glRotated(90,0,1,0);
    glScaled(1,1,1);
    glutSolidTorus(0.2,1,50,50);
    glutSolidTorus(0.2,1,50,50);
    glPopMatrix();

    //tiangkursi
    glPushMatrix();
    glTranslatef(1,-0.3,-0.7);
    glRotatef(270,1,0,0);
    glScalef(0.15,0.15,13);
    cylinder();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-1,-0.3,-0.7);
    glRotatef(270,1,0,0);
    glScalef(0.15,0.15,13);
    cylinder();
    glPopMatrix();
}

void ontang()
{
glColor3f(244.0f/255.0f,0.0f/255.0f,0.0f/255.0f);
 //glColor3f(255.0f/255.0f,45.0f/255.0f,45.0f/255.0f);
    //atap
    glPushMatrix();
    glTranslatef(0,39,0);
    glRotated(270,1,0,0);
    //glRotated(180,0,1,0);
    glutSolidCone(25,10,8,1);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0,38,0);
    glRotated(270,1,0,0);
    //glRotated(180,0,1,0);
    glScaled(25,25,1);
    cylinder2();
    glPopMatrix();


    //kursiontang
    glColor3f(255.0f/255.0f,128.0f/255.0f,255.0f/255.0f);
    glPushMatrix();
    glTranslatef(22,27,0);
    kursiontang();
    glPopMatrix();

    glColor3f(128.0f/255.0f,0.0f/255.0f,128.0f/255.0f);
    glPushMatrix();
    glTranslatef(-22,27,0);
    glRotated(180,0,1,0);
    kursiontang();
    glPopMatrix();

    glColor3f(128.0f/255.0f,255.0f/255.0f,0.0f/255.0f);
    glPushMatrix();
    glTranslatef(17,27,-15);
    glRotated(45,0,1,0);
    kursiontang();
    glPopMatrix();

    glColor3f(255.0f/255.0f,128.0f/255.0f,64.0f/255.0f);
    glPushMatrix();
    glTranslatef(-17,27,15);
    glRotated(225,0,1,0);
    kursiontang();
    glPopMatrix();

    glColor3f(128.0f/255.0f,0.0f/255.0f,255.0f/255.0f);
    glPushMatrix();
    glTranslatef(0,27,-22);
    glRotated(90,0,1,0);
    kursiontang();
    glPopMatrix();

    glColor3f(255.0f/255.0f,255.0f/255.0f,128.0f/255.0f);
    glPushMatrix();
    glTranslatef(0,27,22);
    glRotated(270,0,1,0);
    kursiontang();
    glPopMatrix();

    glColor3f(0.0f/255.0f,0.0f/255.0f,255.0f/255.0f);
    glPushMatrix();
    glTranslatef(-14,27,-16);
    glRotated(135,0,1,0);
    kursiontang();
    glPopMatrix();

    glColor3f(128.0f/255.0f,128.0f/255.0f,64.0f/255.0f);
    glPushMatrix();
    glTranslatef(14,27,16);
    glRotated(315,0,1,0);
    kursiontang();
    glPopMatrix();


}

void hasilontang()
{
    glPushMatrix();
    glTranslated(0,-10,0);

    if (putar<=360)
    {
    glRotated(putar,0,1,0);
    putar+=10;
    if (putar==360) {putar=0;}
    }


    ontang();
    glPopMatrix();

    //base
    //glColor3f(64.0f/255.0f,128.0f/255.0f,128.0f/255.0f);
    glColor3f(192.0f/255.0f,192.0f/255.0f,192.0f/255.0f);
    glPushMatrix();
    glTranslatef(0,1,0);
    glRotated(90,1,0,0);
    glScaled(25,25,2);
    cylinder2();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0,2,0);
    glRotated(90,1,0,0);
    glScaled(22,22,2);
    cylinder2();
    glPopMatrix();

    //tiang
    glColor3f(64.0f/255.0f,128.0f/255.0f,128.0f/255.0f);
    //glColor3f(255.0f/255.0f,128.0f/255.0f,64.0f/255.0f);
    //glColor3f(0.0f/255.0f,128.0f/255.0f,128.0f/255.0f);
    glPushMatrix();
    glTranslatef(0,1,0);
    glRotated(270,1,0,0);
    glScaled(3.5,3.5,30);
    cylinder();
    glPopMatrix();

}


//-------------------------------KICIR-KICIR------------------------------------------
//------------------------------------------------------------------------------------
void tiangkicir()
{
     glColor3f(255.0f/255.0f,128.0f/255.0f,0.0f/255.0f);
    glPushMatrix();
    glTranslated(0,20,1);
    glScaled(2,2,12);
    cylinder();
    glPopMatrix();

 glColor3f(255.0f/255.0f,128.0f/255.0f,0.0f/255.0f);
    glPushMatrix();
    glTranslated(0,22,0);
    glRotated(90,1,0,0);
    glScaled(2,2,26);
    cylinder();
    glPopMatrix();

 glColor3f(128.0f/255.0f,64.0f/255.0f,0.0f/255.0f);
    glDisable(GL_CULL_FACE);
    glPushMatrix();
    glTranslated(0,-12,0);
    glRotated(270,1,0,0);
    gluCylinder(p, 8, 5,8, 360,1);
    glPopMatrix();
    glEnable(GL_CULL_FACE);

    glDisable(GL_CULL_FACE);
    glPushMatrix();
    glTranslated(0,-4,0);
    glScaled(5,5,5);
    glRotated(270,1,0,0);
    gluDisk(p, 0, 1, 360, 1);
    glPopMatrix();
    glEnable(GL_CULL_FACE);


}

void badan_kicir2()
{
     glColor3f(255.0f/255.0f,0.0f/255.0f,0.0f/255.0f);
    glPushMatrix();
    glTranslatef(0,10,0);
    glRotated(270,1,0,0);
    //glRotated(90,0,1,0);
    glScaled(3,3,3);
    cylinder();
    glPopMatrix();

    //tiang
     glColor3f(255.0f/255.0f,128.0f/255.0f,0.0f/255.0f);
    glPushMatrix();
    glTranslated(0,11.5,0);
    glRotated(90,0,1,0);
    glScaled(2,1.5,35);
    glutSolidCube(1);
    glPopMatrix();

    glPushMatrix();
    glTranslated(0,11.5,0);
    glRotated(30,0,1,0);
    glScaled(2,1.5,35);
    glutSolidCube(1);
    glPopMatrix();

    glPushMatrix();
    glTranslated(0,11.5,0);
    glRotated(150,0,1,0);
    glScaled(2,1.5,35);
    glutSolidCube(1);
    glPopMatrix();

    //tiang2
    glPushMatrix();
    glTranslated(16.1,13,0);
    glScaled(2,4,2);
    glutSolidCube(1);
    glPopMatrix();

    glPushMatrix();
    glTranslated(-16.1,13,0);
    glScaled(2,4,2);
    glutSolidCube(1);
    glPopMatrix();

    glPushMatrix();
    glTranslated(-8.085,13,-14);
    glRotated(30,0,1,0);
    glScaled(2,4,2);
    glutSolidCube(1);
    glPopMatrix();

    glPushMatrix();
    glTranslated(8.085,13,14);
    glRotated(30,0,1,0);
    glScaled(2,4,2);
    glutSolidCube(1);
    glPopMatrix();

    glPushMatrix();
    glTranslated(8.09,13,-14);
    glRotated(150,0,1,0);
    glScaled(2,4,2);
    glutSolidCube(1);
    glPopMatrix();

    glPushMatrix();
    glTranslated(-8.09,13,14);
    glRotated(150,0,1,0);
    glScaled(2,4,2);
    glutSolidCube(1);
    glPopMatrix();

}

void kursikicir()
{
 glColor3f(255.0f/255.0f,255.0f/255.0f,0.0f/255.0f);
    //kursi
    glPushMatrix();
    glTranslated(0,10,0);
    glRotated(90,1,0,0);
    glScaled(3,0.6,3);
    glutSolidCube(1);
    glPopMatrix();

    glPushMatrix();
    glTranslated(0,12,0.3);
    glRotated(180,1,0,0);
    glScaled(1.3,1.3,0.6);
    cylinder();
    glPopMatrix();

    //kursi2
    glPushMatrix();
    glTranslated(3,10,0);
    glRotated(90,1,0,0);
    glScaled(3,0.6,3);
    glutSolidCube(1);
    glPopMatrix();

    glPushMatrix();
    glTranslated(3,12,0.3);
    glRotated(180,1,0,0);
    glScaled(1.3,1.3,0.6);
    cylinder();
    glPopMatrix();

    //alas
    glPushMatrix();
    glTranslated(1.47,8.6,-1.5);
    glScaled(6,0.6,3.5);
    glutSolidCube(1);
    glPopMatrix();

    //belt
    glPushMatrix();
    glTranslated(-1.2,10.3,-1.7);
    glRotated(90,0,1,0);
    glScaled(1.8,1.8,3);
    glutSolidTorus(0.1,1,6,360);
    glPopMatrix();

    glPushMatrix();
    glTranslated(1.5,10.3,-1.7);
    glRotated(90,0,1,0);
    glScaled(1.8,1.8,3);
    glutSolidTorus(0.1,1,6,360);
    glPopMatrix();

    glPushMatrix();
    glTranslated(4.2,10.3,-1.7);
    glRotated(90,0,1,0);
    glScaled(1.8,1.8,3);
    glutSolidTorus(0.1,1,6,360);
    glPopMatrix();


}

void putarkursikicir()
{
//katrolbelakang
    glPushMatrix();
    glTranslated(1.5,10,0);
    glColor3f(1,1,1);

    if (r_kkicir<=180)
{
    glRotated(r_kkicir,0,0,1);
    r_kkicir+=1;
    if (r_kkicir==180) {r_kkicir2=180;}
}
else if (r_kkicir2>=-180)
{
    glRotated(r_kkicir2,0,0,1);
    r_kkicir2+=-1;
    if(r_kkicir2==-180) {r_kkicir=-180;}
}


    glScaled(1,1,0.8);
    cylinder();

    //kursi
    glPushMatrix();
    glTranslated(-1.5,-10,0);
    kursikicir();
    glPopMatrix();
    glPopMatrix();
}

void hasil_kursi()
{
    glPushMatrix();
    glTranslated(0,10,0);
    putarkursikicir();
    glPopMatrix();

    glPushMatrix();
    glTranslated(3,10,3.6);
    glRotated(180,0,1,0);
    putarkursikicir();
    glPopMatrix();
}

void hasil_kicir1()
{
    glPushMatrix();
    glTranslated(0,20,0);
    glRotated(90,1,0,0);


    if (b_kicir<=360)
    {
        glRotated(b_kicir,0,1,0);
        b_kicir+=5;
        if (b_kicir==360) {b_kicir=0.0;}
    }


    badan_kicir2();

//kursi
    glPushMatrix();
    glTranslated(14.6,-7,-1.8);
    hasil_kursi();
    glPopMatrix();

    glPushMatrix();
    glTranslated(-17.6,-7,-1.8);
    hasil_kursi();
    glPopMatrix();

    glPushMatrix();
    glTranslated(5.8,-7,-13.6);
    glRotated(60,0,1,0);
    hasil_kursi();
    glPopMatrix();

    glPushMatrix();
    glTranslated(-10.4,-7,14.4);
    glRotated(60,0,1,0);
    hasil_kursi();
    glPopMatrix();

    glPushMatrix();
    glTranslated(-8.8,-7,-11.7);
    glRotated(120,0,1,0);
    hasil_kursi();
    glPopMatrix();

    glPushMatrix();
    glTranslated(7.4,-7,16.3);
    glRotated(120,0,1,0);
    hasil_kursi();
    glPopMatrix();


    glPopMatrix();
}

void hasil_kicir2()
{
    glPushMatrix();
    glTranslated(0,10,0);
    if (t_kicir<=90)
    {
        glRotated(t_kicir,0,1,0);
        t_kicir+=5;
        if (t_kicir==90) {t_kicir2=90;}
    }
    else if (t_kicir2>=-90)
    {
        glRotated(t_kicir2,0,1,0);
        t_kicir2+=-5;
        if (t_kicir2==-90) {t_kicir=-90;}
    }

    tiangkicir();

    glPushMatrix();
    glTranslated(0,0,0);
    hasil_kicir1();
    glPopMatrix();

    glPopMatrix();
}


void parkir()
{
glColor3f(0,0,0);
glPushMatrix();
glTranslated(230,0.2,-20);
glScaled(40,0,200);
glutSolidCube(1);
glPopMatrix();

glColor3f(1,1,1);

for( int a =80; a >= -130; a = a-20 )
{
glPushMatrix();
glTranslated(230,0.28,a);
glScaled(40,0,2);
glutSolidCube(1);
glPopMatrix();
}

}

void dasar()
{
glPushMatrix();
glColor3f(1,1,1);
glBegin(GL_POLYGON);
glTexCoord2f(0.0, 0.0);
glVertex3f(1, 30, 0.0);
glTexCoord2f(1.0, 0.0);
glVertex3f(30, 30, 0.0);
glTexCoord2f(1.0, 1.0);
glVertex3f(30, 60, 0.0);
glTexCoord2f(0.0, 1.0);
glVertex3f(1, 60, 0.0);
glEnd();


glPopMatrix();
}

void dasar2()
{
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    gluQuadricTexture(p,1);
    glPushMatrix();
    glTranslated(0,0,0);
    dasar();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

void dasar3()
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glPushMatrix();
    glTranslated(0,0,0);
    dasar();
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}


void stand()
{
    glPushMatrix();
    glColor3f(174.0f/255.0f,174.0f/255.0f,174.0f/255.0f);
    glTranslated(0,10,0);
    glScaled(7,13,15);
    glutSolidCube(1);
    glPopMatrix();

    //kaca
    glPushMatrix();
    glColor3f(0, 0.5, 1);
    glTranslated(6.5,12,0);
    glScaled(6,5,15);
    glutSolidCube(1);
    glPopMatrix();

    glPushMatrix();
    glColor3f(174.0f/255.0f,174.0f/255.0f,174.0f/255.0f);
    glTranslated(6.5,15,0);
    glScaled(6,1,15);
    glutSolidCube(1);
    glPopMatrix();

    glPushMatrix();
    glColor3f(174.0f/255.0f,174.0f/255.0f,174.0f/255.0f);
    glTranslated(6.5,15,0);
    glScaled(6,1,15);
    glutSolidCube(1);
    glPopMatrix();

    glPushMatrix();
    glColor3f(174.0f/255.0f,174.0f/255.0f,174.0f/255.0f);
    glTranslated(6,6.5,0);
    glScaled(5,6,15);
    glutSolidCube(1);
    glPopMatrix();

    glPushMatrix();
    glColor3f(174.0f/255.0f,174.0f/255.0f,174.0f/255.0f);
    glTranslated(9.5,9.2,0);
    glScaled(3,0.6,15);
    glutSolidCube(1);
    glPopMatrix();

    //pintu
    glPushMatrix();
    glColor3f(205.0f/255.0f,191.0f/255.0f,143.0f/255.0f);
    glTranslated(0,8.5,-7.3);
    glScaled(4,10,0.7);
    glutSolidCube(1);
    glPopMatrix();
}

void bunga()
{
    glColor3f(1,1,1);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[1]);
    gluQuadricTexture(p,1);
    glPushMatrix();
    glTranslated(0,10,0);
    glScaled(3,3,3);
    gluSphere(p,1,10,5);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

void kursi()
{
glColor3f(192.0f/255.0f,192.0f/255.0f,192.0f/255.0f);
glPushMatrix();
glTranslated(0,10,0);
glScaled(0.5,0.5,3.5);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslated(5,10,0);
glScaled(0.5,0.5,3.5);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslated(10,10,0);
glScaled(0.5,0.5,3.5);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslated(5,10.5,0);
glRotated(90,0,1,0);
glScaled(0.7,0.5,11);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslated(5,10.5,-1.2);
glRotated(90,0,1,0);
glScaled(0.7,0.5,11);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslated(5,10.5,1.2);
glRotated(90,0,1,0);
glScaled(0.7,0.5,11);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslated(5,12,-1.9);
glRotated(90,0,1,0);
glRotated(90,0,0,1);
glScaled(2.4,0.5,11);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslated(0,10.3,-1.9);
glRotated(90,1,0,0);
glScaled(0.5,0.5,3.5);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslated(5,10.3,-1.9);
glRotated(90,1,0,0);
glScaled(0.5,0.5,3.5);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslated(10,10.3,-1.9);
glRotated(90,1,0,0);
glScaled(0.5,0.5,3.5);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslated(0,9.5,1.3);
glRotated(90,1,0,0);
glScaled(0.5,0.5,2);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslated(5,9.5,1.3);
glRotated(90,1,0,0);
glScaled(0.5,0.5,2);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslated(10,9.5,1.3);
glRotated(90,1,0,0);
glScaled(0.5,0.5,2);
glutSolidCube(1);
glPopMatrix();
}

void pagartaman()
{
    glColor3f(255.0f/255.0f,255.0f/255.0f,128.0f/255.0f);
    //glColor3f(128.0f/255.0f,64.0f/255.0f,0.0f/255.0f);
    glPushMatrix();
    glTranslated(0,10,0);
    glRotated(90,1,0,0);
    glRotated(10,0,1,0);
    glScaled(10,0.4,0.4);
    glutSolidCube(1);
    glPopMatrix();

    glPushMatrix();
    glTranslated(0,10,0);
    glRotated(90,1,0,0);
    glRotated(170,0,1,0);
    glScaled(10,0.4,0.4);
    glutSolidCube(1);
    glPopMatrix();

    glPushMatrix();
    glTranslated(0,11,0);
    glRotated(90,1,0,0);
    glScaled(10,0.4,0.4);
    glutSolidCube(1);
    glPopMatrix();

    glPushMatrix();
    glTranslated(0,9,0);
    glRotated(90,1,0,0);
    glScaled(10,0.4,0.4);
    glutSolidCube(1);
    glPopMatrix();

    glPushMatrix();
    glTranslated(-5,9.2,0);
    glRotated(90,0,0,1);
    glScaled(4,0.4,0.4);
    glutSolidCube(1);
    glPopMatrix();

    glPushMatrix();
    glTranslated(5,9.2,0);
    glRotated(90,0,0,1);
    glScaled(4,0.4,0.4);
    glutSolidCube(1);
    glPopMatrix();
}

void mobil()
{
    //Bodi
    glColor3f(1, 0, 0);
    glPushMatrix();
    //glRotatef(sudutk, 0.0, 0.0, 1.0);
    glTranslatef(0.0, 3.8, 0.0);
    glScalef(4.0, 1.0, 1.5);
    glutSolidCube(6.0f);
    glPopMatrix();

    //Kaca Belakang
    glColor3f(0, 1, 1);
    glPushMatrix();
    glTranslatef(-12.0, 5.0, 0.0);
    glScalef(0.05, 0.5, 1.2);
    glutSolidCube(6.0f);
    glPopMatrix();

    //Lampu Rem
    glColor3f(1, 1, 1);
    glPushMatrix();
    glTranslatef(-12.1, 2.0, -3.5);
    glScalef(0.02, 0.19, 0.08);
    glutSolidCube(6.0f);
    glPopMatrix();

    glColor3f(1, 1, 1);
    glPushMatrix();
    glTranslatef(-12.1, 2.0, 3.5);
    glScalef(0.02, 0.19, 0.08);
    glutSolidCube(6.0f);
    glPopMatrix();

    //Lampu Depan
    glColor3f(1.0, 1.0, 1.0);
    glPushMatrix();
    glTranslatef(12.1, 2.0, -3.5);
    glScalef(0.05, 0.02, 0.1);
    glutSolidCube(6.0f);
    glPopMatrix();

    glColor3f(1.0, 1.0, 1.0);
    glPushMatrix();
    glTranslatef(12.1, 2.0, 3.5);
    glScalef(0.05, 0.02, 0.1);
    glutSolidCube(6.0f);
    glPopMatrix();

    //Kaca Depan
    glColor3f(0, 1, 1);
    glPushMatrix();
    glTranslatef(12.0, 5.0, 0.0);
    glScalef(0.05, 0.5, 1.2);
    glutSolidCube(6.0f);
    glPopMatrix();
    //Kaca pinggir
    glColor3f(0, 1, 1);
    glPushMatrix();
    glTranslatef(-1.5, 5.0, 4.0);
    glScalef(3.3, 0.5, 0.2);
    glutSolidCube(6.0f);
    glPopMatrix();

    glColor3f(0, 1, 1);
    glPushMatrix();
    glTranslatef(-1.5, 5.0, -4.0);
    glScalef(3.3, 0.5, 0.2);
    glutSolidCube(6.0f);
    glPopMatrix();
    //Pintu Kanan
    glColor3f(0.5, 0.5, 0.5);
    glPushMatrix();
    glTranslatef(11.1, 3.9, 4.0);
    glScalef(0.15, 0.8, 0.2);
    glutSolidCube(6.0f);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.5, 0.5, 0.5);
    glTranslatef(10.1, 5.1, 4.0);
    glScalef(0.25, 0.4, 0.2);
    glutSolidCube(6.0f);
    glPopMatrix();
    //Pintu Kiri
    glColor3f(0.5, 0.5, 0.5);
    glPushMatrix();
    glTranslatef(11.1, 3.9, -4.0);
    glScalef(0.15, 0.8, 0.2);
    glutSolidCube(6.0f);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.5, 0.5, 0.5);
    glTranslatef(10.1, 5.1, -4.0);
    glScalef(0.25, 0.4, 0.2);
    glutSolidCube(6.0f);
    glPopMatrix();

    //Ban Belakang
    glPushMatrix();
    glColor3f(1.0, 1.0, 1.0);
    glScalef(0.35, 0.35, 0.25);
    glTranslatef(25.0, 4.5, 19.3);
    glutSolidTorus(2, 3, 20, 30);
    glPopMatrix();
    //Velg
    glPushMatrix();
    glColor3f(0.0, 0.0, 0.0);
    glScalef(0.5, 0.5, 0.5);
    glTranslatef(17.5, 2.5, 8.0);
    glutSolidSphere(2, 10, 20);
    glPopMatrix();

    glPushMatrix();
    glColor3f(1.0, 1.0, 1.0);
    glScalef(0.35, 0.35, 0.25);
    glTranslatef(25.0, 4.5, -19.3);
    glutSolidTorus(2, 3, 20, 30);
    glPopMatrix();
    //Velg
    glPushMatrix();
    glColor3f(0.0, 0.0, 0.0);
    glScalef(0.5, 0.5, 0.5);
    glTranslatef(-17.5, 2.5, 8.0);
    glutSolidSphere(2, 10, 20);
    glPopMatrix();


    //Ban Depan
    glPushMatrix();
    glColor3f(1.0, 1.0, 1.0);
    glScalef(0.35, 0.35, 0.25);
    glTranslatef(-25.0, 4.5, -19.3);
    glutSolidTorus(2, 3, 20, 30);
    glPopMatrix();
    //Velg
    glPushMatrix();
    glColor3f(0.0, 0.0, 0.0);
    glScalef(0.5, 0.5, 0.5);
    glTranslatef(17.5, 2.5, -8.0);
    glutSolidSphere(2, 10, 20);
    glPopMatrix();

    glPushMatrix();
    glColor3f(1.0, 1.0, 1.0);
    glScalef(0.35, 0.35, 0.25);
    glTranslatef(-25.0, 4.5, 19.3);
    glutSolidTorus(2, 3, 20, 30);
    glPopMatrix();
    //Velg
    glPushMatrix();
    glColor3f(0.0, 0.0, 0.0);
    glScalef(0.5, 0.5, 0.5);
    glTranslatef(-17.5, 2.5, -8.0);
    glutSolidSphere(2, 10, 20);
    glPopMatrix();
}



void display(void) {

const double t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
const double a = t*90.0;


	glClearStencil(0); //clear the stencil buffer
	glClearDepth(1.0f);
	glClearColor(0.0, 0.6, 0.8, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); //clear the buffers
	glLoadIdentity();
	gluLookAt(viewx, viewy, viewz, 0.0, 0.0, 5.0, 0.0, 1.0, 0.0);

	glPushMatrix();
	drawSceneTanah(_terrain, 0.3f, 0.9f, 0.0f);
    glPopMatrix();



	glPushMatrix();
	drawSceneTanah(_terrainStreet, 215.0f/255.0f, 208.0f/255.0f, 134.0f/255.0f);
	glPopMatrix();

    glPushMatrix();
	drawSceneTanah(_terrainKolam, 0.0f/255.0f, 128.0f/255.0f, 192.0f/255.0f);
	glPopMatrix();

	glPushMatrix();
	drawSceneTanah(_terrainAir, 0.4902f, 0.4683f,0.4594f);
	glPopMatrix();

//-- JALAN
  for( int a = 220; a > -240; a = a - 40 )
   {
glPushMatrix();
glTranslatef(a,-0.5,44);
glScalef(3, 3, 3);
markajalan();
glPopMatrix();
   }

  for( int a = 70; a > -200; a = a - 40 )
   {
glPushMatrix();
glTranslatef(120,-0.5,a);
glRotated(90, 0, 1, 0);
glScalef(3, 3, 3);
markajalan();
glPopMatrix();
   }

  for( int a = 70; a > -200; a = a - 40 )
   {
glPushMatrix();
glTranslatef(-242,-0.5,a);
glRotated(90, 0, 1, 0);
glScalef(3, 3, 3);
markajalan();
glPopMatrix();
   }

//pohon

	glPushMatrix();
	glTranslatef(230,2,-170);
	glScalef(7, 7, 7);
	pohon();
	glPopMatrix();

    glPushMatrix();
	glTranslatef(230,2,-140);
	glScalef(7, 7, 7);
	pohon();
	glPopMatrix();


    //pohon sisi
   {
	glPushMatrix();
	glTranslatef(0,30,0);
	glScalef(3, 3, 3);
	pohon();
	glPopMatrix();
   }


// dis pagar

  for( int a =-98; a <= 98; a = a+20 )
   {
glPushMatrix();
glTranslatef(a,0.8,80);
pagar();
glPopMatrix();
   }

glPushMatrix();
glTranslatef(-155,0.8,80);
glScaled(1.1,1,1);
pagar();
glPopMatrix();

glPushMatrix();
glTranslatef(135,0.8,80);
glScaled(1.15,1,1);
pagar();
glPopMatrix();



for( int a = 79; a > -180; a = a - 20 )
   {
glPushMatrix();
glTranslatef(157,0.8,a);
glRotated(90, 0, 1, 0);
//glBindTexture(GL_TEXTURE_2D, texture[0]);
pagar();
glPopMatrix();
   }

for( int a = 79; a > -180; a = a - 20 )
   {
glPushMatrix();
glTranslatef(-156,0.8,a);
glRotated(90, 0, 1, 0);
//glBindTexture(GL_TEXTURE_2D, texture[0]);
pagar();
glPopMatrix();
   }

  for( int a = 138; a > -170; a = a-20 )
   {
glPushMatrix();
glTranslatef(a,0.8,-180);
pagar();
glPopMatrix();
   }

//dis papan
glPushMatrix();
glTranslatef(-116,9,75);
papan();
glPopMatrix();

glPushMatrix();
glTranslatef(117,9,75);
papan();
glPopMatrix();



// dis bianglala (jadi)
glPushMatrix();
glTranslated(-50,0,-70);
glRotated(90,0,1,0);
glScaled(1.2,1.2,1.2);
bianglala3();
glPopMatrix();



//dis kora (jadi)
glPushMatrix();
glTranslatef(0,-13, 50);
glRotated(90,0,1,0);
glScaled(1.4,1.4,1.4);
hasil();
glPopMatrix();


//dis ontang (jadi)
glPushMatrix();
glTranslatef(110,3,-70);
hasilontang();
glPopMatrix();


//dis kicir (jadi)
glPushMatrix();
glTranslatef(0,2,-140);
glScaled(1.2,1.2,1.2);
hasil_kicir2();
glPopMatrix();

//dis pagartaman
for( int a = -100; a < -10; a = a+10 )
{
glPushMatrix();
glTranslated(a,-7,3);
pagartaman();
glPopMatrix();
}

for( int a = 20; a <= 100; a = a+10 )
{
glPushMatrix();
glTranslated(a,-7,3);
pagartaman();
glPopMatrix();
}

for( int a = 8; a <= 70; a = a+10 )
{
glPushMatrix();
glTranslated(105,-7,a);
glRotated(90,0,1,0);
pagartaman();
glPopMatrix();
}

for( int a = 8; a <= 70; a = a+10 )
{
glPushMatrix();
glTranslated(-105,-7,a);
glRotated(90,0,1,0);
pagartaman();
glPopMatrix();
}

for( int a = -22; a <= 40; a = a+10 )
{
glPushMatrix();
glTranslated(-128,-7,a);
glRotated(90,0,1,0);
pagartaman();
glPopMatrix();
}

for( int a = -18; a <= 50; a = a+10 )
{
glPushMatrix();
glTranslated(129,-7,a);
glRotated(90,0,1,0);
pagartaman();
glPopMatrix();
}

for( int a = -63; a <= -20; a = a+10 )
{
glPushMatrix();
glTranslated(a,-7,-119);
pagartaman();
glPopMatrix();
}

for( int a = 22; a <= 70; a = a+10 )
{
glPushMatrix();
glTranslated(a,-7,-119);
pagartaman();
glPopMatrix();
}

for( int a = -123; a <= -70; a = a+10 )
{
glPushMatrix();
glTranslated(a,-7,-27);
pagartaman();
glPopMatrix();
}

for( int a = 74; a <= 130; a = a+10 )
{
glPushMatrix();
glTranslated(a,-7,-24);
pagartaman();
glPopMatrix();
}

for( int a = -50; a <= -30; a = a+10 )
{
glPushMatrix();
glTranslated(69,-7,a);
glRotated(90,0,1,0);
pagartaman();
glPopMatrix();
}

for( int a = -114; a <= -80; a = a+10 )
{
glPushMatrix();
glTranslated(69,-7,a);
glRotated(90,0,1,0);
pagartaman();
glPopMatrix();

for( int a = -53; a <= -30; a = a+10 )
{
glPushMatrix();
glTranslated(-69,-7,a);
glRotated(90,0,1,0);
pagartaman();
glPopMatrix();
}

for( int a = -114; a <= -80; a = a+10 )
{
glPushMatrix();
glTranslated(-69,-7,a);
glRotated(90,0,1,0);
pagartaman();
glPopMatrix();
}


}


//dis kursi
glPushMatrix();
glTranslated(-16,-12.2,-2);
glScaled(1.5,1.5,1.5);
glRotated(180,0,1,0);
kursi();
glPopMatrix();

glPushMatrix();
glTranslated(30,-12.2,-2);
glScaled(1.5,1.5,1.5);
glRotated(180,0,1,0);
kursi();
glPopMatrix();


glPushMatrix();
glTranslated(-63,-12.2,-43);
glScaled(1.5,1.5,1.5);
glRotated(90,0,1,0);
kursi();
glPopMatrix();


glPushMatrix();
glTranslated(-63,-12.2,-80);
glScaled(1.5,1.5,1.5);
glRotated(90,0,1,0);
kursi();
glPopMatrix();


glPushMatrix();
glTranslated(63,-12.2,-95);
glScaled(1.5,1.5,1.5);
glRotated(270,0,1,0);
kursi();
glPopMatrix();

glPushMatrix();
glTranslated(63,-12.2,-55);
glScaled(1.5,1.5,1.5);
glRotated(270,0,1,0);
kursi();
glPopMatrix();


//dis parkir
glPushMatrix();
parkir();
glPopMatrix();

//dis bus
glPushMatrix();
glTranslated(230,0.3,70);
glRotated(180,0,1,0);
glScaled(1.3,1.6,1.4);
bus();
glPopMatrix();

glPushMatrix();
glTranslated(230,0.3,50);
glRotated(180,0,1,0);
glScaled(1.3,1.6,1.4);
bus();
glPopMatrix();

glPushMatrix();
glTranslated(230,0.3,-10);
glRotated(180,0,1,0);
glScaled(1.3,1.6,1.4);
bus();
glPopMatrix();

glPushMatrix();
glTranslated(230,0,-70);
glRotated(180,0,1,0);
glScaled(1.3,1.6,1.4);
bus();
glPopMatrix();


//dis bunga

glPushMatrix();
glTranslated(-60,-18,46);
glScaled(2,2,2);
bunga();
glPopMatrix();

glPushMatrix();
glTranslated(60,-18,46);
glScaled(2,2,2);
bunga();
glPopMatrix();

glPushMatrix();
glTranslated(3,-18,-64);
glScaled(2,2,2);
bunga();
glPopMatrix();

glPushMatrix();
glTranslated(3,-9,-55);
bunga();
glPopMatrix();

glPushMatrix();
glTranslated(3,-9,-73);
bunga();
glPopMatrix();

glPushMatrix();
glTranslated(12,-9,-64);
bunga();
glPopMatrix();

glPushMatrix();
glTranslated(-6,-9,-64);
bunga();
glPopMatrix();



glPushMatrix();
glTranslated(70,-9,50);
bunga();
glPopMatrix();

glPushMatrix();
glTranslated(70,-9,43);
bunga();
glPopMatrix();

glPushMatrix();
glTranslated(-70,-9,50);
bunga();
glPopMatrix();

glPushMatrix();
glTranslated(-70,-9,43);
bunga();
glPopMatrix();




glPushMatrix();
glTranslated(-135,-9,73);
bunga();
glPopMatrix();

glPushMatrix();
glTranslated(-135,-9,47);
bunga();
glPopMatrix();

glPushMatrix();
glTranslated(135,-9,73);
bunga();
glPopMatrix();

glPushMatrix();
glTranslated(135,-9,47);
bunga();
glPopMatrix();

//dis stand
glPushMatrix();
glTranslated(-145,-5,60);
stand();
glPopMatrix();

glPushMatrix();
glTranslated(145,-5,60);
glRotated(180,0,1,0);
stand();
glPopMatrix();


//dis pohon taman

    glPushMatrix();
    glTranslatef(-130,2,-160);
    glScalef(7, 7, 7);
    pohon();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-105,2,-160);
    glScalef(7, 7, 7);
    pohon();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-130,2,-135);
    glScalef(7, 7, 7);
    pohon();
    glPopMatrix();

    //sisi kanan
    glPushMatrix();
    glTranslatef(130,2,-160);
    glScalef(7, 7, 7);
    pohon();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(105,2,-160);
    glScalef(7, 7, 7);
    pohon();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(130,2,-135);
    glScalef(7, 7, 7);
    pohon();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-90,2,15);
    glScalef(7, 7, 7);
    pohon();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(95,2,15);
    glScalef(7, 7, 7);
    pohon();
    glPopMatrix();


//awan
glPushMatrix();
glTranslatef(0, 100, -150);
glScalef(1.8, 1.0, 1.0);
awan();
glPopMatrix();

glPushMatrix();
glTranslatef(70, 80, -150);
glScalef(1.8, 1.0, 1.0);
awan();
glPopMatrix();

glPushMatrix();
glTranslatef(-70, 80, -150);
glScalef(1.8, 1.0, 1.0);
awan();
glPopMatrix();

glPushMatrix();
glTranslatef(140, 100, -130);
glScalef(1.8, 1.0, 1.0);
awan();
glPopMatrix();

//LAMPU
    for( int a = 220; a > -240; a = a - 40 )
    {
    glPushMatrix();
    glTranslatef(a,0.05,127);
    glScalef(2, 2, 2);
    lampu();
    glPopMatrix();
    }

    //dis mobil
    {
    glPushMatrix();
    if(gerak>=-260)
    {glTranslated(gerak,0,118);
    gerak-=2;
    if (gerak==-260) {gerak=230;}
    }
    glRotated(180,0,1,0);
    glScaled(1,1,1);
    mobil();
    glPopMatrix();
    }

for( int a = 200; a > -240; a = a - 60 )
    {

    glPushMatrix();
    glTranslatef(a,0.05,170);
    glScalef(7, 7, 7);
    pohon();
    glPopMatrix();
    }

	glutSwapBuffers();
	glFlush();
	rot++;
	angle++;
}


void enable(void)
	{
    glEnable(GL_DEPTH_TEST); //enable the depth testing
    glEnable(GL_LIGHTING); //enable the lighting
    glEnable(GL_LIGHT0); //enable LIGHT0, our Diffuse Light
    glEnable(GL_COLOR_MATERIAL);
    glShadeModel(GL_SMOOTH); //set the shader to smooth shader
    glEnable(GL_TEXTURE_2D);
}



void init(void) {
    glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glDepthFunc(GL_LESS);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);
	glDepthFunc(GL_LEQUAL);
	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_CULL_FACE);

	_terrain = loadTerrain("heightmap.bmp", 20);
	_terrainStreet = loadTerrain("heightmapstreet.bmp", 20);
	_terrainAir = loadTerrain("heightmapAir.bmp", 20);
    _terrainKolam = loadTerrain("heightmapkolam.bmp", 20);

	//binding texture
    Gambar *gambar1 = loadTexture();
    Gambar *gambar2 = loadTexture2();

	if (gambar1 == NULL) {
		printf("Gambar was not returned from loadTexture\n");
		exit(0);
	}

    if (gambar2 == NULL) {
		printf("Gambar was not returned from loadTexture\n");
		exit(0);
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(2, texture);
	//binding texture untuk membuat texture 2D
	glBindTexture(GL_TEXTURE_2D, texture[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, gambar1->sizeX, gambar1->sizeY, 0, GL_RGB,
			GL_UNSIGNED_BYTE, gambar1->data);


	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(2, texture);
	//binding texture untuk membuat texture 2D
	glBindTexture(GL_TEXTURE_2D, texture[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, gambar2->sizeX, gambar2->sizeY, 0, GL_RGB,
			GL_UNSIGNED_BYTE, gambar2->data);
}


static void kibor(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_HOME:
		viewy+=3;
		break;
	case GLUT_KEY_END:
		viewy-=3;
		break;
	case GLUT_KEY_UP:
		viewz-=3;
		break;
	case GLUT_KEY_DOWN:
		viewz+=3;
		break;

	case GLUT_KEY_RIGHT:
		viewx+=3;
		break;
	case GLUT_KEY_LEFT:
		viewx-=3;
		break;

	case GLUT_KEY_F1: {
		glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
		glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	}
		;
		break;
	case GLUT_KEY_F2: {
		glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient2);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse2);
		glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	}
		;
		break;
	default:
		break;
	}
}

void keyboard(unsigned char key, int x, int y) {
	if (key == 'd') {

		spin = spin - 3;
		if (spin > 360.0)
			spin = spin - 360.0;
	}
	if (key == 'a') {
		spin = spin + 3;
		if (spin > 360.0)
			spin = spin - 360.0;
	}
	if (key == 'q') {
		viewz+=5;
	}
	if (key == 'e') {
		viewz-=5;
	}
	if (key == 's') {
		viewy-=5;
	}
	if (key == 'w') {
		viewy+=5;
	}
}

void reshape(int w, int h) {
	glViewport(0, 0, (GLsizei) w, (GLsizei) h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, (GLfloat) w / (GLfloat) h, 0.1, 1000.0);
	glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_STENCIL | GLUT_DEPTH); //add a stencil buffer to the window
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("TUGAS BESAR");
	init();

	glutDisplayFunc(display);
	glutIdleFunc(display);
	glutReshapeFunc(reshape);
	glutSpecialFunc(kibor);

	glutKeyboardFunc(keyboard);

	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);
	glColorMaterial(GL_FRONT, GL_DIFFUSE);

	glutMainLoop();
	return 0;
}
