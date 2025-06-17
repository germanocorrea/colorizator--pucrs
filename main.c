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

typedef struct {
	double l, a, b;
} LAB;

typedef struct {
	RGB shade_of_gray;
	RGB color;
} ColorToShadeOfGray;

typedef struct {
	int x, y;
} Coordinate;

int black_tolerance = 7;
int gray_tolerance = 2;
int color_diff_tolerance = 5;

bool color_is_equal(RGB color_a, RGB color_b, double delta_e_tolerance);

void load(char *name, Img *pic);

void valida();

void init();

void draw();

void keyboard(unsigned char key, int x, int y);

int width, height;
GLuint tex[2];
Img pic[2];
int sel;

// user defined functions
void grayscale(const RGB (*in)[width], RGB (*out)[width]);

void colorizeitor(const RGB (*in)[width], RGB (*out)[width]);

void user_function(const RGB (*in)[width], RGB (*out)[width]);

void load_colors_from_file(char *filename);

RGB *color_palette = NULL;
int color_palette_size = 0;
ColorToShadeOfGray *color_to_shade_of_gray_dict = NULL;

void print_image_to_terminal(const RGB (*img)[width], int height, int width) {
	// apenas para debug com imagens pequenas (por ex, imagem teste.png)
	return;
	// Códigos ANSI para resetar a cor
	const char *RESET = "\033[0m";

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			// Obtém o pixel atual
			RGB pixel = img[y][x];

			// Define a cor do fundo usando código ANSI RGB
			printf("\033[48;2;%d;%d;%dm  %s",
					pixel.r, pixel.g, pixel.b,
					RESET);
		}
		// Nova linha após cada linha da imagem
		printf("\n");
	}

	printf("=====================\n");
}

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
	sel = 0; // entrada

	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(0.0, width, height, 0.0);
	glMatrixMode(GL_MODELVIEW);

	// Converte para interpretar como matriz
	RGB (*in)[width] = (RGB(*)[width]) pic[0].img;
	RGB (*out)[width] = (RGB(*)[width]) pic[1].img;

	load_colors_from_file(argv[2]);
	user_function(in, out);

	tex[0] = SOIL_create_OGL_texture((unsigned char *) pic[0].img, width, height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
	tex[1] = SOIL_create_OGL_texture((unsigned char *) pic[1].img, width, height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

	glutMainLoop();
}

void keyboard(unsigned char key, int x, int y) {
	if (key == 27) {
		// ESC: libera memória e finaliza
		free(pic[0].img);
		free(pic[1].img);
		free(color_palette);
		free(color_to_shade_of_gray_dict);
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

void grayscale(const RGB (*in)[width], RGB (*out)[width]) {
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++) {
			const unsigned char gray = (unsigned char) (
				0.59 * in[y][x].g +
				0.3 * in[y][x].r +
				0.11 * in[y][x].b
			);
			out[y][x] = (RGB){gray, gray, gray};
		}
}

void user_function(const RGB (*in)[width], RGB (*out)[width]) {
	color_to_shade_of_gray_dict = calloc(width * height, sizeof(ColorToShadeOfGray));
	grayscale(in, out);
	colorizeitor(in, out);
	print_image_to_terminal(out, height, width);
}

void load_colors_from_file(char *filename) {
	FILE *file = fopen(filename, "r");
	if (!file) {
		printf("Error opening file '%s'\n", filename);
		exit(1);
	}

	char line[13];
	fgets(line, sizeof(line), file);
	while (fgets(line, sizeof(line), file)) {
		if (strlen(line) > 1)
			color_palette_size++;
	}

	color_palette = (RGB *) calloc(color_palette_size, sizeof(RGB));
	rewind(file);
	fgets(line, sizeof(line), file);
	for (int i = 0; i < color_palette_size; i++) {
		fgets(line, sizeof(line), file);
		sscanf(line, "%d %d %d", &color_palette[i].r, &color_palette[i].g, &color_palette[i].b);
	}
	fclose(file);
}

bool **generated_visited_matrix() {
	bool **visited = calloc(height, sizeof(bool *));
	for (int i = 0; i < height; i++)
		visited[i] = calloc(width, sizeof(bool));

	return visited;
}

void free_visited_matrix(bool **visited) {
	for (int i = 0; i < height; i++)
		free(visited[i]);
	free(visited);
}

float color_distance(RGB color_a, RGB color_b) {
	float dr = (float) color_a.r - color_b.r;
	float dg = (float) color_a.g - color_b.g;
	float db = (float) color_a.b - color_b.b;

	return sqrt(dr * dr + dg * dg + db * db);
}

bool color_is_similar(RGB color_a, RGB color_b, const float tolerance) {
	return color_distance(color_a, color_b) <= tolerance;
}

RGB get_color_from_palette(const RGB *shade_of_gray, ColorToShadeOfGray *color_to_shade_of_gray_dict) {
	int i = 0;
	do {
		const ColorToShadeOfGray *color_to_shade_of_gray = &color_to_shade_of_gray_dict[i];
		if (
			color_to_shade_of_gray->color.r == 0 &&
			color_to_shade_of_gray->color.g == 0 &&
			color_to_shade_of_gray->color.b == 0
		)
			break;
		if (color_is_equal(color_to_shade_of_gray->shade_of_gray, *shade_of_gray, gray_tolerance))
			return color_to_shade_of_gray->color;
		i++;
	} while (i < width * height);
	color_to_shade_of_gray_dict[i] = (ColorToShadeOfGray){*shade_of_gray, color_palette[i % color_palette_size]};
	return color_to_shade_of_gray_dict[i].color;
}

void flood_fill_seed(
	const RGB (*in)[width],
	RGB (*out)[width],
	bool **visited,
	int *current_region,
	int y,
	int x
) {
	if (visited[y][x])
		return;

	Coordinate *queue = calloc(width * height, sizeof(Coordinate));
	Coordinate *region_member = calloc(width * height, sizeof(Coordinate));

	int region_start = 0, region_size = 0;

	queue[region_size] = (Coordinate){x, y};
	region_member[region_size] = (Coordinate){x, y};
	visited[y][x] = true;
	region_size++;

	const RGB ref = out[y][x];

	bool is_outer_region = false;
	while (region_start < region_size) {
		const Coordinate c = queue[region_start];
		if (c.x == 0 && c.y == 0 && region_start > 0) {
			break;
		}

		region_start++;
		for (int dy = -1; dy <= 1; dy++)
			for (int dx = -1; dx <= 1; dx++) {
				const int nx = c.x + dx;
				const int ny = c.y + dy;
				const bool neighbour_out_of_image_boundaries =
						nx < 0 || nx >= width || ny < 0 || ny >= height;

				if (neighbour_out_of_image_boundaries) {
					is_outer_region = true;
					continue;
				}
				if (visited[ny][nx])
					continue;

				if (
					color_is_equal(out[ny][nx], ref, color_diff_tolerance) &&
					!color_is_equal(out[ny][nx], (RGB){0, 0, 0}, color_diff_tolerance)
				) {
					visited[ny][nx] = true;
					queue[region_size] = (Coordinate){nx, ny};
					region_member[region_size] = (Coordinate){nx, ny};
					region_size++;
				}
			}
	}

	(*current_region)++;
	free(queue);

	for (int i = 0; i < region_size; i++) {
		const Coordinate c = region_member[i];
		const int y = c.y;
		const int x = c.x;
		if (color_is_equal(out[y][x], (RGB){0, 0, 0}, black_tolerance))
			out[y][x] = (RGB){0, 0, 0};
		else if (!is_outer_region) {
			float mask = 1;//out[y][x].r / 255.0f; // mascara removida, estava desbotando muito
			RGB new_color = get_color_from_palette(&out[y][x], color_to_shade_of_gray_dict);
			out[y][x].r = (unsigned char) (new_color.r * mask);
			out[y][x].g = (unsigned char) (new_color.g * mask);
			out[y][x].b = (unsigned char) (new_color.b * mask);
		} else out[y][x] = (RGB){255, 255, 255};
	}

	print_image_to_terminal(out, height, width);

	free(region_member);
}

void colorizeitor(const RGB (*in)[width], RGB (*out)[width]) {
	bool **visited = generated_visited_matrix();

	int current_region = 1;

	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++) {
			flood_fill_seed(in, out, visited, &current_region, y, x);
		}

	free_visited_matrix(visited);
}

void rgb_to_xyz(RGB rgb, double *x, double *y, double *z) {
	double r = rgb.r / 255.0;
	double g = rgb.g / 255.0;
	double b = rgb.b / 255.0;

	r = (r > 0.04045) ? pow((r + 0.055) / 1.055, 2.4) : r / 12.92;
	g = (g > 0.04045) ? pow((g + 0.055) / 1.055, 2.4) : g / 12.92;
	b = (b > 0.04045) ? pow((b + 0.055) / 1.055, 2.4) : b / 12.92;

	*x = r * 0.412453 + g * 0.357580 + b * 0.180423;
	*y = r * 0.212671 + g * 0.715160 + b * 0.072169;
	*z = r * 0.019334 + g * 0.119193 + b * 0.950227;
}

double t_function(double t) {
	// double delta = 6.0 / 29.0;
	double detal_p_3 = 0.008856; // pow(delta, 3.0)

	if (t > detal_p_3) {
		return pow(t, 1.0 / 3.0);
	} else {
		// double m = (1.0/3.0) * pow(delta, -2.0)
		double m = 7.787037;
		return (t * m) + (4.0 / 29.0);
	}
}

LAB xyz_to_lab(double x, double y, double z) {
	const double xn = 95.0489 / 100;
	const double yn = 100.0 / 100;
	const double zn = 108.8840 / 100;

	double fx = x / xn;
	double fy = y / yn;
	double fz = z / zn;

	LAB lab = (LAB){
		.l = (116.0 * t_function(fy)) - 16,
		.a = 500.0 * (t_function(fx) - t_function(fy)),
		.b = 200.0 * (t_function(fy) - t_function(fz))
	};

	return lab;
}

LAB rgb_to_lab(RGB rgb) {
	double x, y, z;
	rgb_to_xyz(rgb, &x, &y, &z);
	return xyz_to_lab(x, y, z);
}

double delta_e(RGB color1, RGB color2) {
	LAB lab1 = rgb_to_lab(color1);
	LAB lab2 = rgb_to_lab(color2);

	double dl = lab1.l - lab2.l;
	double da = lab1.a - lab2.a;
	double db = lab1.b - lab2.b;

	return sqrt(dl * dl + da * da + db * db);
}

bool color_is_equal(RGB color_a, RGB color_b, const double delta_e_tolerance) {
	if (color_a.r == color_b.r && color_a.g == color_b.g && color_a.b == color_b.b) {
		return true;
	}
	return delta_e(color_a, color_b) <= delta_e_tolerance;
}
