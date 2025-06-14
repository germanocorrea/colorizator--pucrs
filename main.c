#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>		// Para usar strings

#ifdef WIN32
#include <windows.h>    // Apenas para Windows
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>     // Funções da OpenGL
#include <GL/glu.h>    // Funções da GLU
#include <GL/glut.h>   // Funções da FreeGLUT
#endif

#include "SOIL.h"

// Um pixel RGB (24 bits)
typedef struct {
	unsigned char r, g, b;
} RGB;

typedef struct {
	int width, height;
	RGB *img;
} Img;

void load(char *name, Img *pic);
void valida();
void init();
void draw();
void keyboard(unsigned char key, int x, int y);

int width, height;
GLuint tex[2];
Img pic[2];
int sel;

// Carrega uma imagem para a struct Img
void load(char *name, Img *pic) {
	int chan;
	pic->img = (RGB *) SOIL_load_image(name, &pic->width, &pic->height, &chan, SOIL_LOAD_RGB);
	if (!pic->img) {
		printf("SOIL loading error: '%s'\n", SOIL_last_result());
		exit(1);
	}
	printf("Load: %d x %d x %d\n", pic->width, pic->height, chan);
}

int main(int argc, char **argv) {
	if (argc < 3) {
		printf("colorizeitor [im. entrada] [arq. cores]\n");
		exit(1);
	}
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

	// pic[0] -> imagem de entrada
	// pic[1] -> imagem de saida

	load(argv[1], &pic[0]);

	width = pic[0].width;
	height = pic[0].height;

	pic[1].width = pic[0].width;
	pic[1].height = pic[0].height;
	pic[1].img = calloc(pic[1].width * pic[1].height, 3); // W x H x 3 bytes (Pixel)

	glutInitWindowSize(width, height);
	glutCreateWindow("Colorizeitor");
	glutDisplayFunc(draw);
	glutKeyboardFunc(keyboard);
	printf("Origem  : %s %d x %d\n", argv[1], pic[0].width, pic[0].height);
	sel = 1; // entrada

	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(0.0, width, height, 0.0);
	glMatrixMode(GL_MODELVIEW);

	// Converte para interpretar como matriz
	RGB (*in)[width] = (RGB(*)[width]) pic[0].img;
	RGB (*out)[width] = (RGB(*)[width]) pic[1].img;

	// Aplica o algoritmo e gera a saida em out (pic[1].img)
	// ...
	// ...
	// Exemplo: copia apenas o componente vermelho para a saida
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
			out[y][x].r = in[y][x].r;

	tex[0] = SOIL_create_OGL_texture((unsigned char *) pic[0].img, width, height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
	tex[1] = SOIL_create_OGL_texture((unsigned char *) pic[1].img, width, height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

	glutMainLoop();
}

void keyboard(unsigned char key, int x, int y) {
	if (key == 27) {
		// ESC: libera memória e finaliza
		free(pic[0].img);
		free(pic[1].img);
		exit(1);
	}
	if (key >= '1' && key <= '2')
		// 1-2: seleciona a imagem correspondente (origem ou destino)
		sel = key - '1';
	glutPostRedisplay();
}

void draw() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Preto
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float aspect = (float) glutGet(GLUT_WINDOW_WIDTH) / (float) glutGet(GLUT_WINDOW_HEIGHT);
	if (aspect >= 1.0)
		gluOrtho2D(0.0, width * aspect, height, 0.0);
	else
		gluOrtho2D(0.0, width, height / aspect, 0.0);
	glMatrixMode(GL_MODELVIEW);

	// Para outras cores, veja exemplos em /etc/X11/Pixel.txt

	glColor3ub(255, 255, 255); // branco

	// Ativa a textura corresponde à imagem desejada
	glBindTexture(GL_TEXTURE_2D, tex[sel]);
	// E desenha um retângulo que ocupa toda a tela
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);

	glTexCoord2f(0, 0);
	glVertex2f(0, 0);

	glTexCoord2f(1, 0);
	glVertex2f(pic[sel].width, 0);

	glTexCoord2f(1, 1);
	glVertex2f(pic[sel].width, pic[sel].height);

	glTexCoord2f(0, 1);
	glVertex2f(0, pic[sel].height);

	glEnd();
	glDisable(GL_TEXTURE_2D);

	glutSwapBuffers();
}
