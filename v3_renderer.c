#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <termios.h>
#include <unistd.h>

#define MAXPTS_INFACE 50000
#define MAXNO_OFTRIANGLES 100
#define MAXNO_OFLINES 100
#define MAXVERTS_INFACE 30
#define EPSILON 1e-6

struct termios orig;

void raw_mode_off(void) {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig);
}

void raw_mode_on(void) {
    tcgetattr(STDIN_FILENO, &orig);
    atexit(raw_mode_off);

    struct termios raw = orig;
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

//rasterization part

float mincoord(float list[][3], int size, int coord) {
	float minm = list[0][coord];
	for(int i = 1; i < size; i++) {
		if(list[i][coord] < minm) {
			minm = list[i][coord];
		}
	}
	return minm;
}

float maxcoord(float list[][3], int size, int coord) {
	float maxm = list[0][coord];
	for(int i = 1; i < size; i++) {
		if(list[i][coord] >= maxm) {
			maxm = list[i][coord];
		}
	}
	return maxm;
}

float dot_product(float v1[3], float v2[3]) {
	float product = v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
	return product;
}

void cross_product(float v1[3], float v2[3], float crossprod[3]) {
	crossprod[0] = v1[1]*v2[2] - v1[2]*v2[1];
	crossprod[1] = v1[2]*v2[0] - v1[0]*v2[2];
	crossprod[2] = v1[0]*v2[1] - v1[1]*v2[0];
}

int formtriangle(float vertices[][3], float triangle_list[][MAXPTS_INFACE][3], int triangle_no, int size) {
	float normal[3];
	float u1[3], u2[3];

	for(int i = 0; i < 3; i++) {
		vertices[i][0] = (int)roundf(vertices[i][0]);
		vertices[i][1] = (int)roundf(vertices[i][1]);
		vertices[i][2] = (int)roundf(vertices[i][2]);
	}

	u1[0] = vertices[0][0] - vertices[2][0];
	u1[1] = vertices[0][1] - vertices[2][1];
	u1[2] = vertices[0][2] - vertices[2][2];

	u2[0] = vertices[1][0] - vertices[2][0];
	u2[1] = vertices[1][1] - vertices[2][1];
	u2[2] = vertices[1][2] - vertices[2][2];

	cross_product(u1, u2, normal);

	float magnitude = sqrtf(normal[0]*normal[0] + normal[1]*normal[1] + normal[2]*normal[2]);

	normal[0] /= magnitude;
	normal[1] /= magnitude;
	normal[2] /= magnitude;


	float minx = mincoord(vertices, size, 0);
	float miny = mincoord(vertices, size, 1);
	float minz = mincoord(vertices, size, 2);

	float maxx = maxcoord(vertices, size, 0);
	float maxy = maxcoord(vertices, size, 1);
	float maxz = maxcoord(vertices, size, 2);

	int point[3] = {0};

	float absX = fabsf(normal[0]);
	float absY = fabsf(normal[1]);
	float absZ = fabsf(normal[2]);

	float d1, d2, d3;
	float x1, x2, x3, y1, y2, y3;
	int no_of_pts = 1;
	float d_plane = normal[0]*vertices[0][0] + normal[1]*vertices[0][1] + normal[2]*vertices[0][2];

	int drop_axis; // 0 = X, 1 = Y, 2 = Z

	if (absX >= absY && absX >= absZ) {
		drop_axis = 0;
		x1 = vertices[0][1]; x2 = vertices[1][1]; x3 = vertices[2][1]; // y coords
		y1 = vertices[0][2]; y2 = vertices[1][2]; y3 = vertices[2][2]; // z coords

		for(int i = (int)floorf(miny); i <= (int)ceilf(maxy); i++) {
			for(int j = (int)floorf(minz); j <= (int)ceilf(maxz); j++) {
				d1 = (i - x2)*(y1 - y2) - (x1 - x2)*(j - y2);
				d2 = (i - x3)*(y2 - y3) - (x2 - x3)*(j - y3);
				d3 = (i - x1)*(y3 - y1) - (x3 - x1)*(j - y1);

				if((d1 >= -EPSILON && d2 >= -EPSILON && d3 >= -EPSILON) || (d1 <= EPSILON && d2 <= EPSILON && d3 <= EPSILON)) {
					float px = (d_plane - (normal[1] * i) - (normal[2] * j)) / normal[0];
					triangle_list[triangle_no][no_of_pts - 1][0] = px;
					triangle_list[triangle_no][no_of_pts - 1][1] = (float)i;
					triangle_list[triangle_no][no_of_pts - 1][2] = (float)j;
					no_of_pts++;
				}
			}
		}
	} else if (absY >= absX && absY >= absZ) {
		drop_axis = 1;
		x1 = vertices[0][0]; x2 = vertices[1][0]; x3 = vertices[2][0]; // x coords
		y1 = vertices[0][2]; y2 = vertices[1][2]; y3 = vertices[2][2]; // z coords

		for(int i = (int)floorf(minx); i <= (int)ceilf(maxx); i++) {
			for(int j = (int)floorf(minz); j <= (int)ceilf(maxz); j++) {
				d1 = (i - x2)*(y1 - y2) - (x1 - x2)*(j - y2);
				d2 = (i - x3)*(y2 - y3) - (x2 - x3)*(j - y3);
				d3 = (i - x1)*(y3 - y1) - (x3 - x1)*(j - y1);

				if((d1 >= -EPSILON && d2 >= -EPSILON && d3 >= -EPSILON) || (d1 <= EPSILON && d2 <= EPSILON && d3 <= EPSILON)) {
					float py = (d_plane - (normal[0] * i) - (normal[2] * j)) / normal[1];
					triangle_list[triangle_no][no_of_pts - 1][0] = (float)i;
					triangle_list[triangle_no][no_of_pts - 1][1] = py;
					triangle_list[triangle_no][no_of_pts - 1][2] = (float)j;
					no_of_pts++;
				}
			}
		}
	} else {
		drop_axis = 2;
		x1 = vertices[0][0]; x2 = vertices[1][0]; x3 = vertices[2][0]; // x coords
		y1 = vertices[0][1]; y2 = vertices[1][1]; y3 = vertices[2][1]; // y coorda

		for(int i = (int)floorf(minx); i <= (int)ceilf(maxx); i++) {
			for(int j = (int)floorf(miny); j <= (int)ceilf(maxy); j++) {
				d1 = (i - x2)*(y1 - y2) - (x1 - x2)*(j - y2);
				d2 = (i - x3)*(y2 - y3) - (x2 - x3)*(j - y3);
				d3 = (i - x1)*(y3 - y1) - (x3 - x1)*(j - y1);

				if((d1 >= -EPSILON && d2 >= -EPSILON && d3 >= -EPSILON) || (d1 <= EPSILON && d2 <= EPSILON && d3 <= EPSILON)) {
					float pz = (d_plane - (normal[0] * i) - (normal[1] * j)) / normal[2];
					triangle_list[triangle_no][no_of_pts - 1][0] = (float)i;
					triangle_list[triangle_no][no_of_pts - 1][1] = (float)j;
					triangle_list[triangle_no][no_of_pts - 1][2] = pz;
					no_of_pts++;
				}
			}
		}
	}
	return no_of_pts;
}

void save_to_trilist(float vertices[][3], int tri_no, float trilist[][3][3]) {
	for(int i = 0; i < 3; i++) {
		trilist[tri_no][i][0] = vertices[i][0];
		trilist[tri_no][i][1] = vertices[i][1];
		trilist[tri_no][i][2] = vertices[i][2];
	}
}

//convert vertices & faces to wireframe lines so that it works with my old code. (note: its kinda redundant but im also kinda lazy)

void convert_to_lines(float faces_vertices_list[3][3], float linelist[][6], int line_no) {
	linelist[line_no][0] = faces_vertices_list[0][0];
	linelist[line_no][1] = faces_vertices_list[0][1];
	linelist[line_no][2] = faces_vertices_list[0][2];
	linelist[line_no][3] = faces_vertices_list[1][0];
	linelist[line_no][4] = faces_vertices_list[1][1];
	linelist[line_no][5] = faces_vertices_list[1][2];

	linelist[line_no+1][0] = faces_vertices_list[1][0];
	linelist[line_no+1][1] = faces_vertices_list[1][1];
	linelist[line_no+1][2] = faces_vertices_list[1][2];
	linelist[line_no+1][3] = faces_vertices_list[2][0];
	linelist[line_no+1][4] = faces_vertices_list[2][1];
	linelist[line_no+1][5] = faces_vertices_list[2][2];

	linelist[line_no+2][0] = faces_vertices_list[2][0];
	linelist[line_no+2][1] = faces_vertices_list[2][1];
	linelist[line_no+2][2] = faces_vertices_list[2][2];
	linelist[line_no+2][3] = faces_vertices_list[0][0];
	linelist[line_no+2][4] = faces_vertices_list[0][1];
	linelist[line_no+2][5] = faces_vertices_list[0][2];
}

int displayed_lines(float displaying_faces[][3], float actual_lines[][6], int line_no, int n_vertices) {
	for(int i = 0; i < n_vertices; i++) {
		int next = (i + 1) % n_vertices;
		actual_lines[line_no + i][0] = displaying_faces[i][0];
		actual_lines[line_no + i][1] = displaying_faces[i][1];
		actual_lines[line_no + i][2] = displaying_faces[i][2];
		actual_lines[line_no + i][3] = displaying_faces[next][0];
		actual_lines[line_no + i][4] = displaying_faces[next][1];
		actual_lines[line_no + i][5] = displaying_faces[next][2];
	}
	return line_no + n_vertices;
}

//all these transformation were about origin.. but the objects center is basically never the origin. changed em to work from the objects 'center' as origin

void transform(float matrix[3][3], float vector[3]) {
	float x_i = vector[0];
	float y_i = vector[1];
	float z_i = vector[2];

	vector[0] = matrix[0][0]*x_i + matrix[0][1]*y_i + matrix[0][2]*z_i;
	vector[1] = matrix[1][0]*x_i + matrix[1][1]*y_i + matrix[1][2]*z_i;
	vector[2] = matrix[2][0]*x_i + matrix[2][1]*y_i + matrix[2][2]*z_i;
}

void rotation(char axis, float angle, float vector[3], float centercoords[3]) {
	float shifted[3];
	shifted[0] = vector[0] - centercoords[0];
	shifted[1] = vector[1] - centercoords[1];
	shifted[2] = vector[2] - centercoords[2];
	float transformation[3][3];

	if(axis == 'x') {
		transformation[0][0] = 1;
		transformation[0][1] = 0;
		transformation[0][2] = 0;
		transformation[1][0] = 0;
		transformation[1][1] = cos(angle);
		transformation[1][2] = -sin(angle);
		transformation[2][0] = 0;
		transformation[2][1] = sin(angle);
		transformation[2][2] = cos(angle);
	}
	else if(axis == 'y') {
		transformation[0][0] = cos(angle);
		transformation[0][1] = 0;
		transformation[0][2] = sin(angle);
		transformation[1][0] = 0;
		transformation[1][1] = 1;
		transformation[1][2] = 0;
		transformation[2][0] = -sin(angle);
		transformation[2][1] = 0;
		transformation[2][2] = cos(angle);
	} else if(axis == 'z') {
		transformation[0][0] = cos(angle);
		transformation[0][1] = -sin(angle);
		transformation[0][2] = 0;
		transformation[1][0] = sin(angle);
		transformation[1][1] = cos(angle);
		transformation[1][2] = 0;
		transformation[2][0] = 0;
		transformation[2][1] = 0;
		transformation[2][2] = 1;
	}

	transform(transformation, shifted);

	vector[0] = shifted[0] + centercoords[0];
	vector[1] = shifted[1] + centercoords[1];
	vector[2] = shifted[2] + centercoords[2];
}

void zoom(float vector[3], float scale, float centercoords[3]) {
	float zoom[3][3] = {0};

	float shifted[3];
	shifted[0] = vector[0] - centercoords[0];
	shifted[1] = vector[1] - centercoords[1];
	shifted[2] = vector[2] - centercoords[2];

	zoom[0][0] = scale;
	zoom[1][1] = scale;
	zoom[2][2] = scale;

	transform(zoom, shifted);

	vector[0] = shifted[0] + centercoords[0];
	vector[1] = shifted[1] + centercoords[1];
	vector[2] = shifted[2] + centercoords[2];
}

void move(float vector[3], char dirn, int amt) {
	if(dirn == 'x') {
		vector[0] += amt;
	} else if(dirn == 'y') {
		vector[1] += amt;
	}
}

int inside_grid(int x, int y) {
	return (x >= 0 && x < 150 && y >= 0 && y < 100);
}

void savetoGrid(int grid[100][150], int y_coord_no, int x_coord_no) {
        grid[y_coord_no][x_coord_no] = 1;
}

// Generalized Bressenham's Line Algorithm for all octants, and saves one lines' coordinates list in grid[][].
void drawLineGeneralized(int x0, int y0, float z0, int x1, int y1, float z1, int grid[100][150], float minzcoord[100][150]) {
        int dx = abs(x1 - x0);
        int dy = abs(y1 - y0);

        int sx = (x0 < x1) ? 1 : -1;
        int sy = (y0 < y1) ? 1 : -1;

        int err = dx - dy;
        int e2;

        int total_steps = (dx > dy) ? dx : dy;
        int step = 0;
        float z;

        while (1) {
                z = (total_steps == 0) ? z0 : z0 + (z1 - z0) * step/total_steps;

                if (inside_grid(x0, y0) && z <= minzcoord[y0][x0]+5) { //this step wont look good on shapes with very closeby parallel faces. But should be okay for most other shapes
                        savetoGrid(grid, y0, x0);
                }

                if (x0 == x1 && y0 == y1) {
                        break;
                }

                e2 = 2 * err;

                if (e2 > -dy) {
                        err -= dy;
                        x0 += sx;
                }

                if (e2 < dx) {
                        err += dx;
                        y0 += sy;
                }

                step++;
        }
}

void plotLines(int grid[100][150], int brightness[100][150]) {
	printf("\033[H");
	fflush(stdout);
	for(int i = 0; i < 100; i++) {
		for(int j = 0; j < 150; j++) {
			if(grid[i][j] == 1) {
				printf("\x1b[38;2;255;255;255m█\x1b[0m");
			} else if(grid[i][j] == 2) {
				printf("\x1b[38;2;%d;%d;%dm█\x1b[0m", brightness[i][j], brightness[i][j], brightness[i][j]);
			} else {
				printf(" ");
			}
		}
		printf("\n");
	}
}

int lighting(float lighting_vector[], float lighting_normal[]) {
	float unscaled_brightness = dot_product(lighting_vector, lighting_normal);
	unscaled_brightness = fabsf(unscaled_brightness);
	unscaled_brightness = 185*unscaled_brightness + 25;
	int brightness = (int)roundf(unscaled_brightness);
	return brightness;
}

void compute_lighting_normal(float vertices[][3], float normal[3]) {
	float lightv1[3] = {vertices[1][0] - vertices[0][0], vertices[1][1] - vertices[0][1], vertices[1][2] - vertices[0][2]};
	float lightv2[3] = {vertices[2][0] - vertices[1][0], vertices[2][1] - vertices[1][1], vertices[2][2] - vertices[1][2]};
	cross_product(lightv1, lightv2, normal);
	float light_mag = sqrtf(normal[0]*normal[0] + normal[1]*normal[1] + normal[2]*normal[2]);
	normal[0] /= light_mag;
	normal[1] /= light_mag;
	normal[2] /= light_mag;
}

int main() {
	float vertices[3][3];
	float triangle_list[1][MAXPTS_INFACE][3];     //ts array is stupid asf but go with it
	float vertices_list[MAXNO_OFTRIANGLES][3][3];
	int triangle_no = 0;
	int size = 3;
	int grid[100][150] = {0};
	int pts_no;
	int line_no = 0;
	int lines;
	int n_tri_lines;
	float three_d_lines[3*MAXNO_OFTRIANGLES][6]; // zero is x0, one is y0, two is z0, three is x1....
        int xStart, yStart, xEnd, yEnd;
	float minzcoord[100][150];
	int n_faces;
	int n_vertices;
	float displaying_faces[MAXVERTS_INFACE][3];
	float actual_lines[MAXNO_OFLINES][6];
	float light_vector[3] = {0, 0, 1};
	float lighting_normal[3];
	int brightgrid[100][150] = {0};

	for (int i = 0; i < 100; i++) {
	    	for (int j = 0; j < 150; j++) {
        		minzcoord[i][j] = 1000;
    		}
	}

	scanf("%d", &triangle_no);

	if(triangle_no < 1 || triangle_no > MAXNO_OFTRIANGLES) {
		printf("error: triangle count must be between 1 and %d\n", MAXNO_OFTRIANGLES);
		return 1;
	}

	n_tri_lines = 3*triangle_no;

	for(int k = 0; k < triangle_no; k++) {
		scanf("%f %f %f", &vertices[0][0], &vertices[0][1], &vertices[0][2]);
		scanf("%f %f %f", &vertices[1][0], &vertices[1][1], &vertices[1][2]);
		scanf("%f %f %f", &vertices[2][0], &vertices[2][1], &vertices[2][2]);

		save_to_trilist(vertices, k, vertices_list);
		pts_no = formtriangle(vertices, triangle_list, 0, size);

		float lightv1[3] = {vertices[1][0] - vertices[0][0], vertices[1][1] - vertices[0][1], vertices[1][2] - vertices[0][2]};
		float lightv2[3] = {vertices[2][0] - vertices[1][0], vertices[2][1] - vertices[1][1], vertices[2][2] - vertices[1][2]};
		cross_product(lightv1, lightv2, lighting_normal);
		float light_mag = sqrtf(lighting_normal[0]*lighting_normal[0] + lighting_normal[1]*lighting_normal[1] + lighting_normal[2]*lighting_normal[2]);
		lighting_normal[0] /= light_mag;
		lighting_normal[1] /= light_mag;
		lighting_normal[2] /= light_mag;

		for(int i = 0; i < pts_no-1; i++) {
			int x = (int)roundf(triangle_list[0][i][0]);
			int y = (int)roundf(triangle_list[0][i][1]);
			if(!inside_grid(x, y)) {
				continue;
			}
			float z = triangle_list[0][i][2];

			if(z < minzcoord[y][x]) {
				minzcoord[y][x] = z;
				grid[y][x] = 2;
				brightgrid[y][x] = lighting(light_vector, lighting_normal);
			}
		}

		line_no = 3*k;
		convert_to_lines(vertices, three_d_lines, line_no);
	}

	scanf("%d", &n_faces);
	lines = 0;
	line_no = 0;

	for(int i = 0; i < n_faces; i++) {
		if(scanf("%d", &n_vertices) != 1 || n_vertices < 3 || n_vertices > MAXVERTS_INFACE) {
			printf("error: each face needs between 3 and %d vertices\n", MAXVERTS_INFACE);
			return 1;
		}
		if(lines + n_vertices > MAXNO_OFLINES) {
			printf("error: too many face edges (max %d in total)\n", MAXNO_OFLINES);
			return 1;
		}
		lines += n_vertices;
		for(int j = 0; j < n_vertices; j++) {
			scanf("%f %f %f", &displaying_faces[j][0], &displaying_faces[j][1], &displaying_faces[j][2]);
		}
		line_no = displayed_lines(displaying_faces, actual_lines, line_no, n_vertices);
	}

        for (int i = 0; i < lines; i++) {
		xStart = (int)roundf(actual_lines[i][0]);
		yStart = (int)roundf(actual_lines[i][1]);
		xEnd = (int)roundf(actual_lines[i][3]);
		yEnd = (int)roundf(actual_lines[i][4]);
		float zStart = actual_lines[i][2];
		float zEnd = actual_lines[i][5];
                drawLineGeneralized(xStart, yStart, zStart, xEnd, yEnd, zEnd, grid, minzcoord);
        }

	printf("\033[H\033[2J\033[3J");

	plotLines(grid, brightgrid);

	char option;
	float angle;
	char axis;
	float centerofshape[3] = {0};
	float vector[3] = {0};
	float scale;
	int amt;
	char dirn;

	for(int i = 0; i < n_tri_lines; i++) {
		centerofshape[0] += (three_d_lines[i][0] + three_d_lines[i][3])/2;
		centerofshape[1] += (three_d_lines[i][1] + three_d_lines[i][4])/2;
		centerofshape[2] += (three_d_lines[i][2] + three_d_lines[i][5])/2;
	}

	centerofshape[0] /= n_tri_lines;
	centerofshape[1] /= n_tri_lines;
	centerofshape[2] /= n_tri_lines;


	raw_mode_on();      //Switches off canonical mode, for continous input
 
	while(1) {
		for(int i = 0; i < 100; i++) {
			for(int j = 0; j < 150; j++) {
				grid[i][j] = 0;
				brightgrid[i][j] = 0;
				minzcoord[i][j] = 1000;
			}
		}

		scanf(" %c", &option);

		if(option == 'x') {
			break;
		}

		if(option == 'w') {
			axis = 'x';
			angle = 5;
			angle = (angle*(3.14159))/180;

			for(int i = 0; i < triangle_no; i++) {
				vector[0] = vertices_list[i][0][0];
				vector[1] = vertices_list[i][0][1];
				vector[2] = vertices_list[i][0][2];

				rotation(axis, angle, vector, centerofshape);

				vertices_list[i][0][0] = vector[0];
				vertices_list[i][0][1] = vector[1];
				vertices_list[i][0][2] = vector[2];

				vector[0] = vertices_list[i][1][0];
				vector[1] = vertices_list[i][1][1];
				vector[2] = vertices_list[i][1][2];

				rotation(axis, angle, vector, centerofshape);

				vertices_list[i][1][0] = vector[0];
				vertices_list[i][1][1] = vector[1];
				vertices_list[i][1][2] = vector[2];

				vector[0] = vertices_list[i][2][0];
				vector[1] = vertices_list[i][2][1];
				vector[2] = vertices_list[i][2][2];

				rotation(axis, angle, vector, centerofshape);

				vertices_list[i][2][0] = vector[0];
				vertices_list[i][2][1] = vector[1];
				vertices_list[i][2][2] = vector[2];

				for(int j = 0; j < 3; j++) {
					vertices[j][0] = vertices_list[i][j][0];
					vertices[j][1] = vertices_list[i][j][1];
					vertices[j][2] = vertices_list[i][j][2];
				}

				pts_no = formtriangle(vertices, triangle_list, 0, size);
				compute_lighting_normal(vertices, lighting_normal);

				for(int j = 0; j < pts_no-1; j++) {
					int x = (int)roundf(triangle_list[0][j][0]);
					int y = (int)roundf(triangle_list[0][j][1]);
					if(!inside_grid(x, y)) {
						continue;
					}
					float z = triangle_list[0][j][2];

					if(z < minzcoord[y][x]) {
						minzcoord[y][x] = z;
						grid[y][x] = 2;
						brightgrid[y][x] = lighting(light_vector, lighting_normal);
					}
				}

				line_no = 3*i;
				convert_to_lines(vertices, three_d_lines, line_no);
			}
			for(int i = 0; i < lines; i++) {
				vector[0] = actual_lines[i][0];
				vector[1] = actual_lines[i][1];
				vector[2] = actual_lines[i][2];

				rotation(axis, angle, vector, centerofshape);

				actual_lines[i][0] = vector[0];
				actual_lines[i][1] = vector[1];
				actual_lines[i][2] = vector[2];

				vector[0] = actual_lines[i][3];
				vector[1] = actual_lines[i][4];
				vector[2] = actual_lines[i][5];

				rotation(axis, angle, vector, centerofshape);

				actual_lines[i][3] = vector[0];
				actual_lines[i][4] = vector[1];
				actual_lines[i][5] = vector[2];

			}


			for (int i = 0; i < lines; i++) {
				xStart = (int)roundf(actual_lines[i][0]);
				yStart = (int)roundf(actual_lines[i][1]);
				xEnd = (int)roundf(actual_lines[i][3]);
				yEnd = (int)roundf(actual_lines[i][4]);
				float zStart = actual_lines[i][2];
				float zEnd = actual_lines[i][5];
				drawLineGeneralized(xStart, yStart, zStart, xEnd, yEnd, zEnd, grid, minzcoord);
			}
			plotLines(grid, brightgrid);
		} else if(option == 's') {
			axis = 'x';
			angle = -5;
			angle = (angle*(3.14159))/180;

			for(int i = 0; i < triangle_no; i++) {
				vector[0] = vertices_list[i][0][0];
				vector[1] = vertices_list[i][0][1];
				vector[2] = vertices_list[i][0][2];

				rotation(axis, angle, vector, centerofshape);

				vertices_list[i][0][0] = vector[0];
				vertices_list[i][0][1] = vector[1];
				vertices_list[i][0][2] = vector[2];

				vector[0] = vertices_list[i][1][0];
				vector[1] = vertices_list[i][1][1];
				vector[2] = vertices_list[i][1][2];

				rotation(axis, angle, vector, centerofshape);

				vertices_list[i][1][0] = vector[0];
				vertices_list[i][1][1] = vector[1];
				vertices_list[i][1][2] = vector[2];

				vector[0] = vertices_list[i][2][0];
				vector[1] = vertices_list[i][2][1];
				vector[2] = vertices_list[i][2][2];

				rotation(axis, angle, vector, centerofshape);

				vertices_list[i][2][0] = vector[0];
				vertices_list[i][2][1] = vector[1];
				vertices_list[i][2][2] = vector[2];

				for(int j = 0; j < 3; j++) {
					vertices[j][0] = vertices_list[i][j][0];
					vertices[j][1] = vertices_list[i][j][1];
					vertices[j][2] = vertices_list[i][j][2];
				}

				pts_no = formtriangle(vertices, triangle_list, 0, size);
				compute_lighting_normal(vertices, lighting_normal);

				for(int j = 0; j < pts_no-1; j++) {
					int x = (int)roundf(triangle_list[0][j][0]);
					int y = (int)roundf(triangle_list[0][j][1]);
					if(!inside_grid(x, y)) {
						continue;
					}
					float z = triangle_list[0][j][2];

					if(z < minzcoord[y][x]) {
						minzcoord[y][x] = z;
						grid[y][x] = 2;
						brightgrid[y][x] = lighting(light_vector, lighting_normal);
					}

				}

				line_no = 3*i;
				convert_to_lines(vertices, three_d_lines, line_no);
			}
			for(int i = 0; i < lines; i++) {
				vector[0] = actual_lines[i][0];
				vector[1] = actual_lines[i][1];
				vector[2] = actual_lines[i][2];

				rotation(axis, angle, vector, centerofshape);

				actual_lines[i][0] = vector[0];
				actual_lines[i][1] = vector[1];
				actual_lines[i][2] = vector[2];

				vector[0] = actual_lines[i][3];
				vector[1] = actual_lines[i][4];
				vector[2] = actual_lines[i][5];

				rotation(axis, angle, vector, centerofshape);

				actual_lines[i][3] = vector[0];
				actual_lines[i][4] = vector[1];
				actual_lines[i][5] = vector[2];

			}


			for (int i = 0; i < lines; i++) {
				xStart = (int)roundf(actual_lines[i][0]);
				yStart = (int)roundf(actual_lines[i][1]);
				xEnd = (int)roundf(actual_lines[i][3]);
				yEnd = (int)roundf(actual_lines[i][4]);
				float zStart = actual_lines[i][2];
				float zEnd = actual_lines[i][5];
				drawLineGeneralized(xStart, yStart, zStart, xEnd, yEnd, zEnd, grid, minzcoord);
			}
			plotLines(grid, brightgrid);
		} else if(option == 'a') {
			axis = 'y';
			angle = -5;
			angle = (angle*(3.14159))/180;

			for(int i = 0; i < triangle_no; i++) {
				vector[0] = vertices_list[i][0][0];
				vector[1] = vertices_list[i][0][1];
				vector[2] = vertices_list[i][0][2];

				rotation(axis, angle, vector, centerofshape);

				vertices_list[i][0][0] = vector[0];
				vertices_list[i][0][1] = vector[1];
				vertices_list[i][0][2] = vector[2];

				vector[0] = vertices_list[i][1][0];
				vector[1] = vertices_list[i][1][1];
				vector[2] = vertices_list[i][1][2];

				rotation(axis, angle, vector, centerofshape);

				vertices_list[i][1][0] = vector[0];
				vertices_list[i][1][1] = vector[1];
				vertices_list[i][1][2] = vector[2];

				vector[0] = vertices_list[i][2][0];
				vector[1] = vertices_list[i][2][1];
				vector[2] = vertices_list[i][2][2];

				rotation(axis, angle, vector, centerofshape);

				vertices_list[i][2][0] = vector[0];
				vertices_list[i][2][1] = vector[1];
				vertices_list[i][2][2] = vector[2];

				for(int j = 0; j < 3; j++) {
					vertices[j][0] = vertices_list[i][j][0];
					vertices[j][1] = vertices_list[i][j][1];
					vertices[j][2] = vertices_list[i][j][2];
				}

				pts_no = formtriangle(vertices, triangle_list, 0, size);
				compute_lighting_normal(vertices, lighting_normal);

				for(int j = 0; j < pts_no-1; j++) {
					int x = (int)roundf(triangle_list[0][j][0]);
					int y = (int)roundf(triangle_list[0][j][1]);
					if(!inside_grid(x, y)) {
						continue;
					}
					float z = triangle_list[0][j][2];

					if(z < minzcoord[y][x]) {
						minzcoord[y][x] = z;
						grid[y][x] = 2;
						brightgrid[y][x] = lighting(light_vector, lighting_normal);
					}
				}

				line_no = 3*i;
				convert_to_lines(vertices, three_d_lines, line_no);
			}
			for(int i = 0; i < lines; i++) {
				vector[0] = actual_lines[i][0];
				vector[1] = actual_lines[i][1];
				vector[2] = actual_lines[i][2];

				rotation(axis, angle, vector, centerofshape);

				actual_lines[i][0] = vector[0];
				actual_lines[i][1] = vector[1];
				actual_lines[i][2] = vector[2];

				vector[0] = actual_lines[i][3];
				vector[1] = actual_lines[i][4];
				vector[2] = actual_lines[i][5];

				rotation(axis, angle, vector, centerofshape);

				actual_lines[i][3] = vector[0];
				actual_lines[i][4] = vector[1];
				actual_lines[i][5] = vector[2];

			}


			for (int i = 0; i < lines; i++) {
				xStart = (int)roundf(actual_lines[i][0]);
				yStart = (int)roundf(actual_lines[i][1]);
				xEnd = (int)roundf(actual_lines[i][3]);
				yEnd = (int)roundf(actual_lines[i][4]);
				float zStart = actual_lines[i][2];
				float zEnd = actual_lines[i][5];
				drawLineGeneralized(xStart, yStart, zStart, xEnd, yEnd, zEnd, grid, minzcoord);
			}
			plotLines(grid, brightgrid);
		} else if(option == 'd') {
			axis = 'y';
			angle = 5;
			angle = (angle*(3.14159))/180;

			for(int i = 0; i < triangle_no; i++) {
				vector[0] = vertices_list[i][0][0];
				vector[1] = vertices_list[i][0][1];
				vector[2] = vertices_list[i][0][2];

				rotation(axis, angle, vector, centerofshape);

				vertices_list[i][0][0] = vector[0];
				vertices_list[i][0][1] = vector[1];
				vertices_list[i][0][2] = vector[2];

				vector[0] = vertices_list[i][1][0];
				vector[1] = vertices_list[i][1][1];
				vector[2] = vertices_list[i][1][2];

				rotation(axis, angle, vector, centerofshape);

				vertices_list[i][1][0] = vector[0];
				vertices_list[i][1][1] = vector[1];
				vertices_list[i][1][2] = vector[2];

				vector[0] = vertices_list[i][2][0];
				vector[1] = vertices_list[i][2][1];
				vector[2] = vertices_list[i][2][2];

				rotation(axis, angle, vector, centerofshape);

				vertices_list[i][2][0] = vector[0];
				vertices_list[i][2][1] = vector[1];
				vertices_list[i][2][2] = vector[2];

				for(int j = 0; j < 3; j++) {
					vertices[j][0] = vertices_list[i][j][0];
					vertices[j][1] = vertices_list[i][j][1];
					vertices[j][2] = vertices_list[i][j][2];
				}

				pts_no = formtriangle(vertices, triangle_list, 0, size);
				compute_lighting_normal(vertices, lighting_normal);

				for(int j = 0; j < pts_no-1; j++) {
					int x = (int)roundf(triangle_list[0][j][0]);
					int y = (int)roundf(triangle_list[0][j][1]);
					if(!inside_grid(x, y)) {
						continue;
					}
					float z = triangle_list[0][j][2];

					if(z < minzcoord[y][x]) {
						minzcoord[y][x] = z;
						grid[y][x] = 2;
						brightgrid[y][x] = lighting(light_vector, lighting_normal);
					}
				}

				line_no = 3*i;
				convert_to_lines(vertices, three_d_lines, line_no);
			}
			for(int i = 0; i < lines; i++) {
				vector[0] = actual_lines[i][0];
				vector[1] = actual_lines[i][1];
				vector[2] = actual_lines[i][2];

				rotation(axis, angle, vector, centerofshape);

				actual_lines[i][0] = vector[0];
				actual_lines[i][1] = vector[1];
				actual_lines[i][2] = vector[2];

				vector[0] = actual_lines[i][3];
				vector[1] = actual_lines[i][4];
				vector[2] = actual_lines[i][5];

				rotation(axis, angle, vector, centerofshape);

				actual_lines[i][3] = vector[0];
				actual_lines[i][4] = vector[1];
				actual_lines[i][5] = vector[2];

			}


			for (int i = 0; i < lines; i++) {
				xStart = (int)roundf(actual_lines[i][0]);
				yStart = (int)roundf(actual_lines[i][1]);
				xEnd = (int)roundf(actual_lines[i][3]);
				yEnd = (int)roundf(actual_lines[i][4]);
				float zStart = actual_lines[i][2];
				float zEnd = actual_lines[i][5];
				drawLineGeneralized(xStart, yStart, zStart, xEnd, yEnd, zEnd, grid, minzcoord);
			}
			plotLines(grid, brightgrid);
		} else if(option == 'q') {
			axis = 'z';
			angle = -5;
			angle = (angle*(3.14159))/180;

			for(int i = 0; i < triangle_no; i++) {
				vector[0] = vertices_list[i][0][0];
				vector[1] = vertices_list[i][0][1];
				vector[2] = vertices_list[i][0][2];

				rotation(axis, angle, vector, centerofshape);

				vertices_list[i][0][0] = vector[0];
				vertices_list[i][0][1] = vector[1];
				vertices_list[i][0][2] = vector[2];

				vector[0] = vertices_list[i][1][0];
				vector[1] = vertices_list[i][1][1];
				vector[2] = vertices_list[i][1][2];

				rotation(axis, angle, vector, centerofshape);

				vertices_list[i][1][0] = vector[0];
				vertices_list[i][1][1] = vector[1];
				vertices_list[i][1][2] = vector[2];

				vector[0] = vertices_list[i][2][0];
				vector[1] = vertices_list[i][2][1];
				vector[2] = vertices_list[i][2][2];

				rotation(axis, angle, vector, centerofshape);

				vertices_list[i][2][0] = vector[0];
				vertices_list[i][2][1] = vector[1];
				vertices_list[i][2][2] = vector[2];

				for(int j = 0; j < 3; j++) {
					vertices[j][0] = vertices_list[i][j][0];
					vertices[j][1] = vertices_list[i][j][1];
					vertices[j][2] = vertices_list[i][j][2];
				}

				pts_no = formtriangle(vertices, triangle_list, 0, size);
				compute_lighting_normal(vertices, lighting_normal);

				for(int j = 0; j < pts_no-1; j++) {
					int x = (int)roundf(triangle_list[0][j][0]);
					int y = (int)roundf(triangle_list[0][j][1]);
					if(!inside_grid(x, y)) {
						continue;
					}
					float z = triangle_list[0][j][2];

					if(z < minzcoord[y][x]) {
						minzcoord[y][x] = z;
						grid[y][x] = 2;
						brightgrid[y][x] = lighting(light_vector, lighting_normal);
					}
				}

				line_no = 3*i;
				convert_to_lines(vertices, three_d_lines, line_no);
			}
			for(int i = 0; i < lines; i++) {
				vector[0] = actual_lines[i][0];
				vector[1] = actual_lines[i][1];
				vector[2] = actual_lines[i][2];

				rotation(axis, angle, vector, centerofshape);

				actual_lines[i][0] = vector[0];
				actual_lines[i][1] = vector[1];
				actual_lines[i][2] = vector[2];

				vector[0] = actual_lines[i][3];
				vector[1] = actual_lines[i][4];
				vector[2] = actual_lines[i][5];

				rotation(axis, angle, vector, centerofshape);

				actual_lines[i][3] = vector[0];
				actual_lines[i][4] = vector[1];
				actual_lines[i][5] = vector[2];

			}


			for (int i = 0; i < lines; i++) {
				xStart = (int)roundf(actual_lines[i][0]);
				yStart = (int)roundf(actual_lines[i][1]);
				xEnd = (int)roundf(actual_lines[i][3]);
				yEnd = (int)roundf(actual_lines[i][4]);
				float zStart = actual_lines[i][2];
				float zEnd = actual_lines[i][5];
				drawLineGeneralized(xStart, yStart, zStart, xEnd, yEnd, zEnd, grid, minzcoord);
			}
			plotLines(grid, brightgrid);
		} else if(option == 'e') {
			axis = 'z';
			angle = 5;
			angle = (angle*(3.14159))/180;

			for(int i = 0; i < triangle_no; i++) {
				vector[0] = vertices_list[i][0][0];
				vector[1] = vertices_list[i][0][1];
				vector[2] = vertices_list[i][0][2];

				rotation(axis, angle, vector, centerofshape);

				vertices_list[i][0][0] = vector[0];
				vertices_list[i][0][1] = vector[1];
				vertices_list[i][0][2] = vector[2];

				vector[0] = vertices_list[i][1][0];
				vector[1] = vertices_list[i][1][1];
				vector[2] = vertices_list[i][1][2];

				rotation(axis, angle, vector, centerofshape);

				vertices_list[i][1][0] = vector[0];
				vertices_list[i][1][1] = vector[1];
				vertices_list[i][1][2] = vector[2];

				vector[0] = vertices_list[i][2][0];
				vector[1] = vertices_list[i][2][1];
				vector[2] = vertices_list[i][2][2];

				rotation(axis, angle, vector, centerofshape);

				vertices_list[i][2][0] = vector[0];
				vertices_list[i][2][1] = vector[1];
				vertices_list[i][2][2] = vector[2];

				for(int j = 0; j < 3; j++) {
					vertices[j][0] = vertices_list[i][j][0];
					vertices[j][1] = vertices_list[i][j][1];
					vertices[j][2] = vertices_list[i][j][2];
				}

				pts_no = formtriangle(vertices, triangle_list, 0, size);
				compute_lighting_normal(vertices, lighting_normal);

				for(int j = 0; j < pts_no-1; j++) {
					int x = (int)roundf(triangle_list[0][j][0]);
					int y = (int)roundf(triangle_list[0][j][1]);
					if(!inside_grid(x, y)) {
						continue;
					}
					float z = triangle_list[0][j][2];

					if(z < minzcoord[y][x]) {
						minzcoord[y][x] = z;
						grid[y][x] = 2;
						brightgrid[y][x] = lighting(light_vector, lighting_normal);
					}

				}

				line_no = 3*i;
				convert_to_lines(vertices, three_d_lines, line_no);
			}
			for(int i = 0; i < lines; i++) {
				vector[0] = actual_lines[i][0];
				vector[1] = actual_lines[i][1];
				vector[2] = actual_lines[i][2];

				rotation(axis, angle, vector, centerofshape);

				actual_lines[i][0] = vector[0];
				actual_lines[i][1] = vector[1];
				actual_lines[i][2] = vector[2];

				vector[0] = actual_lines[i][3];
				vector[1] = actual_lines[i][4];
				vector[2] = actual_lines[i][5];

				rotation(axis, angle, vector, centerofshape);

				actual_lines[i][3] = vector[0];
				actual_lines[i][4] = vector[1];
				actual_lines[i][5] = vector[2];

			}


			for (int i = 0; i < lines; i++) {
				xStart = (int)roundf(actual_lines[i][0]);
				yStart = (int)roundf(actual_lines[i][1]);
				xEnd = (int)roundf(actual_lines[i][3]);
				yEnd = (int)roundf(actual_lines[i][4]);
				float zStart = actual_lines[i][2];
				float zEnd = actual_lines[i][5];
				drawLineGeneralized(xStart, yStart, zStart, xEnd, yEnd, zEnd, grid, minzcoord);
			}
			plotLines(grid, brightgrid);
		}

		if(option == '+') {
			scale = 1.2;

			for(int i = 0; i < triangle_no; i++) {
				vector[0] = vertices_list[i][0][0];
				vector[1] = vertices_list[i][0][1];
				vector[2] = vertices_list[i][0][2];

				zoom(vector, scale, centerofshape);

				vertices_list[i][0][0] = vector[0];
				vertices_list[i][0][1] = vector[1];
				vertices_list[i][0][2] = vector[2];

				vector[0] = vertices_list[i][1][0];
				vector[1] = vertices_list[i][1][1];
				vector[2] = vertices_list[i][1][2];

				zoom(vector, scale, centerofshape);

				vertices_list[i][1][0] = vector[0];
				vertices_list[i][1][1] = vector[1];
				vertices_list[i][1][2] = vector[2];

				vector[0] = vertices_list[i][2][0];
				vector[1] = vertices_list[i][2][1];
				vector[2] = vertices_list[i][2][2];

				zoom(vector, scale, centerofshape);

				vertices_list[i][2][0] = vector[0];
				vertices_list[i][2][1] = vector[1];
				vertices_list[i][2][2] = vector[2];

				for(int j = 0; j < 3; j++) {
					vertices[j][0] = vertices_list[i][j][0];
					vertices[j][1] = vertices_list[i][j][1];
					vertices[j][2] = vertices_list[i][j][2];
				}

				pts_no = formtriangle(vertices, triangle_list, 0, size);
				compute_lighting_normal(vertices, lighting_normal);

				for(int j = 0; j < pts_no-1; j++) {
					int x = (int)roundf(triangle_list[0][j][0]);
					int y = (int)roundf(triangle_list[0][j][1]);
					if(!inside_grid(x, y)) {
						continue;
					}
					float z = triangle_list[0][j][2];

					if(z < minzcoord[y][x]) {
						minzcoord[y][x] = z;
						grid[y][x] = 2;
						brightgrid[y][x] = lighting(light_vector, lighting_normal);
					}

				}

				line_no = 3*i;
				convert_to_lines(vertices, three_d_lines, line_no);
			}
			for(int i = 0; i < lines; i++) {
				vector[0] = actual_lines[i][0];
				vector[1] = actual_lines[i][1];
				vector[2] = actual_lines[i][2];

				zoom(vector, scale, centerofshape);

				actual_lines[i][0] = vector[0];
				actual_lines[i][1] = vector[1];
				actual_lines[i][2] = vector[2];

				vector[0] = actual_lines[i][3];
				vector[1] = actual_lines[i][4];
				vector[2] = actual_lines[i][5];

				zoom(vector, scale, centerofshape);

				actual_lines[i][3] = vector[0];
				actual_lines[i][4] = vector[1];
				actual_lines[i][5] = vector[2];

			}


			for (int i = 0; i < lines; i++) {
				xStart = (int)roundf(actual_lines[i][0]);
				yStart = (int)roundf(actual_lines[i][1]);
				xEnd = (int)roundf(actual_lines[i][3]);
				yEnd = (int)roundf(actual_lines[i][4]);
				float zStart = actual_lines[i][2];
				float zEnd = actual_lines[i][5];
				drawLineGeneralized(xStart, yStart, zStart, xEnd, yEnd, zEnd, grid, minzcoord);
			}
			plotLines(grid, brightgrid);
		} else if(option == '-') {
			scale = 0.8;

			for(int i = 0; i < triangle_no; i++) {
				vector[0] = vertices_list[i][0][0];
				vector[1] = vertices_list[i][0][1];
				vector[2] = vertices_list[i][0][2];

				zoom(vector, scale, centerofshape);

				vertices_list[i][0][0] = vector[0];
				vertices_list[i][0][1] = vector[1];
				vertices_list[i][0][2] = vector[2];

				vector[0] = vertices_list[i][1][0];
				vector[1] = vertices_list[i][1][1];
				vector[2] = vertices_list[i][1][2];

				zoom(vector, scale, centerofshape);

				vertices_list[i][1][0] = vector[0];
				vertices_list[i][1][1] = vector[1];
				vertices_list[i][1][2] = vector[2];

				vector[0] = vertices_list[i][2][0];
				vector[1] = vertices_list[i][2][1];
				vector[2] = vertices_list[i][2][2];

				zoom(vector, scale, centerofshape);

				vertices_list[i][2][0] = vector[0];
				vertices_list[i][2][1] = vector[1];
				vertices_list[i][2][2] = vector[2];

				for(int j = 0; j < 3; j++) {
					vertices[j][0] = vertices_list[i][j][0];
					vertices[j][1] = vertices_list[i][j][1];
					vertices[j][2] = vertices_list[i][j][2];
				}

				pts_no = formtriangle(vertices, triangle_list, 0, size);
				compute_lighting_normal(vertices, lighting_normal);

				for(int j = 0; j < pts_no-1; j++) {
					int x = (int)roundf(triangle_list[0][j][0]);
					int y = (int)roundf(triangle_list[0][j][1]);
					if(!inside_grid(x, y)) {
						continue;
					}
					float z = triangle_list[0][j][2];

					if(z < minzcoord[y][x]) {
						minzcoord[y][x] = z;
						grid[y][x] = 2;
						brightgrid[y][x] = lighting(light_vector, lighting_normal);
					}
				}

				line_no = 3*i;
				convert_to_lines(vertices, three_d_lines, line_no);
			}
			for(int i = 0; i < lines; i++) {
				vector[0] = actual_lines[i][0];
				vector[1] = actual_lines[i][1];
				vector[2] = actual_lines[i][2];

				zoom(vector, scale, centerofshape);

				actual_lines[i][0] = vector[0];
				actual_lines[i][1] = vector[1];
				actual_lines[i][2] = vector[2];

				vector[0] = actual_lines[i][3];
				vector[1] = actual_lines[i][4];
				vector[2] = actual_lines[i][5];

				zoom(vector, scale, centerofshape);

				actual_lines[i][3] = vector[0];
				actual_lines[i][4] = vector[1];
				actual_lines[i][5] = vector[2];

			}


			for (int i = 0; i < lines; i++) {
				xStart = (int)roundf(actual_lines[i][0]);
				yStart = (int)roundf(actual_lines[i][1]);
				xEnd = (int)roundf(actual_lines[i][3]);
				yEnd = (int)roundf(actual_lines[i][4]);
				float zStart = actual_lines[i][2];
				float zEnd = actual_lines[i][5];
				drawLineGeneralized(xStart, yStart, zStart, xEnd, yEnd, zEnd, grid, minzcoord);
			}
			plotLines(grid, brightgrid);
		}

		if(option == 'k') {
			dirn = 'y';
			amt = 5;

			if (dirn == 'x') {
				centerofshape[0] += amt;
			} else if (dirn == 'y') {
				centerofshape[1] += amt;
			}

			for(int i = 0; i < triangle_no; i++) {
				vector[0] = vertices_list[i][0][0];
				vector[1] = vertices_list[i][0][1];
				vector[2] = vertices_list[i][0][2];

				move(vector, dirn, amt);

				vertices_list[i][0][0] = vector[0];
				vertices_list[i][0][1] = vector[1];
				vertices_list[i][0][2] = vector[2];

				vector[0] = vertices_list[i][1][0];
				vector[1] = vertices_list[i][1][1];
				vector[2] = vertices_list[i][1][2];

				move(vector, dirn, amt);

				vertices_list[i][1][0] = vector[0];
				vertices_list[i][1][1] = vector[1];
				vertices_list[i][1][2] = vector[2];

				vector[0] = vertices_list[i][2][0];
				vector[1] = vertices_list[i][2][1];
				vector[2] = vertices_list[i][2][2];

				move(vector, dirn, amt);

				vertices_list[i][2][0] = vector[0];
				vertices_list[i][2][1] = vector[1];
				vertices_list[i][2][2] = vector[2];

				for(int j = 0; j < 3; j++) {
					vertices[j][0] = vertices_list[i][j][0];
					vertices[j][1] = vertices_list[i][j][1];
					vertices[j][2] = vertices_list[i][j][2];
				}

				pts_no = formtriangle(vertices, triangle_list, 0, size);
				compute_lighting_normal(vertices, lighting_normal);

				for(int j = 0; j < pts_no-1; j++) {
					int x = (int)roundf(triangle_list[0][j][0]);
					int y = (int)roundf(triangle_list[0][j][1]);
					if(!inside_grid(x, y)) {
						continue;
					}
					float z = triangle_list[0][j][2];

					if(z < minzcoord[y][x]) {
						minzcoord[y][x] = z;
						grid[y][x] = 2;
						brightgrid[y][x] = lighting(light_vector, lighting_normal);
					}
				}

				line_no = 3*i;
				convert_to_lines(vertices, three_d_lines, line_no);
			}
			for(int i = 0; i < lines; i++) {
				vector[0] = actual_lines[i][0];
				vector[1] = actual_lines[i][1];
				vector[2] = actual_lines[i][2];

				move(vector, dirn, amt);

				actual_lines[i][0] = vector[0];
				actual_lines[i][1] = vector[1];
				actual_lines[i][2] = vector[2];

				vector[0] = actual_lines[i][3];
				vector[1] = actual_lines[i][4];
				vector[2] = actual_lines[i][5];

				move(vector, dirn, amt);

				actual_lines[i][3] = vector[0];
				actual_lines[i][4] = vector[1];
				actual_lines[i][5] = vector[2];

			}


			for (int i = 0; i < lines; i++) {
				xStart = (int)roundf(actual_lines[i][0]);
				yStart = (int)roundf(actual_lines[i][1]);
				xEnd = (int)roundf(actual_lines[i][3]);
				yEnd = (int)roundf(actual_lines[i][4]);
				float zStart = actual_lines[i][2];
				float zEnd = actual_lines[i][5];
				drawLineGeneralized(xStart, yStart, zStart, xEnd, yEnd, zEnd, grid, minzcoord);
			}
			plotLines(grid, brightgrid);
		} else if(option == 'i') {
			dirn = 'y';
			amt = -5;

			if (dirn == 'x') {
				centerofshape[0] += amt;
			} else if (dirn == 'y') {
				centerofshape[1] += amt;
			}

			for(int i = 0; i < triangle_no; i++) {
				vector[0] = vertices_list[i][0][0];
				vector[1] = vertices_list[i][0][1];
				vector[2] = vertices_list[i][0][2];

				move(vector, dirn, amt);

				vertices_list[i][0][0] = vector[0];
				vertices_list[i][0][1] = vector[1];
				vertices_list[i][0][2] = vector[2];

				vector[0] = vertices_list[i][1][0];
				vector[1] = vertices_list[i][1][1];
				vector[2] = vertices_list[i][1][2];

				move(vector, dirn, amt);

				vertices_list[i][1][0] = vector[0];
				vertices_list[i][1][1] = vector[1];
				vertices_list[i][1][2] = vector[2];

				vector[0] = vertices_list[i][2][0];
				vector[1] = vertices_list[i][2][1];
				vector[2] = vertices_list[i][2][2];

				move(vector, dirn, amt);

				vertices_list[i][2][0] = vector[0];
				vertices_list[i][2][1] = vector[1];
				vertices_list[i][2][2] = vector[2];

				for(int j = 0; j < 3; j++) {
					vertices[j][0] = vertices_list[i][j][0];
					vertices[j][1] = vertices_list[i][j][1];
					vertices[j][2] = vertices_list[i][j][2];
				}

				pts_no = formtriangle(vertices, triangle_list, 0, size);
				compute_lighting_normal(vertices, lighting_normal);

				for(int j = 0; j < pts_no-1; j++) {
					int x = (int)roundf(triangle_list[0][j][0]);
					int y = (int)roundf(triangle_list[0][j][1]);
					if(!inside_grid(x, y)) {
						continue;
					}
					float z = triangle_list[0][j][2];

					if(z < minzcoord[y][x]) {
						minzcoord[y][x] = z;
						grid[y][x] = 2;
						brightgrid[y][x] = lighting(light_vector, lighting_normal);
					}
				}

				line_no = 3*i;
				convert_to_lines(vertices, three_d_lines, line_no);
			}
			for(int i = 0; i < lines; i++) {
				vector[0] = actual_lines[i][0];
				vector[1] = actual_lines[i][1];
				vector[2] = actual_lines[i][2];

				move(vector, dirn, amt);

				actual_lines[i][0] = vector[0];
				actual_lines[i][1] = vector[1];
				actual_lines[i][2] = vector[2];

				vector[0] = actual_lines[i][3];
				vector[1] = actual_lines[i][4];
				vector[2] = actual_lines[i][5];

				move(vector, dirn, amt);

				actual_lines[i][3] = vector[0];
				actual_lines[i][4] = vector[1];
				actual_lines[i][5] = vector[2];

			}


			for (int i = 0; i < lines; i++) {
				xStart = (int)roundf(actual_lines[i][0]);
				yStart = (int)roundf(actual_lines[i][1]);
				xEnd = (int)roundf(actual_lines[i][3]);
				yEnd = (int)roundf(actual_lines[i][4]);
				float zStart = actual_lines[i][2];
				float zEnd = actual_lines[i][5];
				drawLineGeneralized(xStart, yStart, zStart, xEnd, yEnd, zEnd, grid, minzcoord);
			}
			plotLines(grid, brightgrid);
		} else if(option == 'j') {
			dirn = 'x';
			amt = -5;

			if (dirn == 'x') {
				centerofshape[0] += amt;
			} else if (dirn == 'y') {
				centerofshape[1] += amt;
			}

			for(int i = 0; i < triangle_no; i++) {
				vector[0] = vertices_list[i][0][0];
				vector[1] = vertices_list[i][0][1];
				vector[2] = vertices_list[i][0][2];

				move(vector, dirn, amt);

				vertices_list[i][0][0] = vector[0];
				vertices_list[i][0][1] = vector[1];
				vertices_list[i][0][2] = vector[2];

				vector[0] = vertices_list[i][1][0];
				vector[1] = vertices_list[i][1][1];
				vector[2] = vertices_list[i][1][2];

				move(vector, dirn, amt);

				vertices_list[i][1][0] = vector[0];
				vertices_list[i][1][1] = vector[1];
				vertices_list[i][1][2] = vector[2];

				vector[0] = vertices_list[i][2][0];
				vector[1] = vertices_list[i][2][1];
				vector[2] = vertices_list[i][2][2];

				move(vector, dirn, amt);

				vertices_list[i][2][0] = vector[0];
				vertices_list[i][2][1] = vector[1];
				vertices_list[i][2][2] = vector[2];

				for(int j = 0; j < 3; j++) {
					vertices[j][0] = vertices_list[i][j][0];
					vertices[j][1] = vertices_list[i][j][1];
					vertices[j][2] = vertices_list[i][j][2];
				}

				pts_no = formtriangle(vertices, triangle_list, 0, size);
				compute_lighting_normal(vertices, lighting_normal);

				for(int j = 0; j < pts_no-1; j++) {
					int x = (int)roundf(triangle_list[0][j][0]);
					int y = (int)roundf(triangle_list[0][j][1]);
					if(!inside_grid(x, y)) {
						continue;
					}
					float z = triangle_list[0][j][2];

					if(z < minzcoord[y][x]) {
						minzcoord[y][x] = z;
						grid[y][x] = 2;
						brightgrid[y][x] = lighting(light_vector, lighting_normal);
					}
				}

				line_no = 3*i;
				convert_to_lines(vertices, three_d_lines, line_no);
			}
			for(int i = 0; i < lines; i++) {
				vector[0] = actual_lines[i][0];
				vector[1] = actual_lines[i][1];
				vector[2] = actual_lines[i][2];

				move(vector, dirn, amt);

				actual_lines[i][0] = vector[0];
				actual_lines[i][1] = vector[1];
				actual_lines[i][2] = vector[2];

				vector[0] = actual_lines[i][3];
				vector[1] = actual_lines[i][4];
				vector[2] = actual_lines[i][5];

				move(vector, dirn, amt);

				actual_lines[i][3] = vector[0];
				actual_lines[i][4] = vector[1];
				actual_lines[i][5] = vector[2];

			}


			for (int i = 0; i < lines; i++) {
				xStart = (int)roundf(actual_lines[i][0]);
				yStart = (int)roundf(actual_lines[i][1]);
				xEnd = (int)roundf(actual_lines[i][3]);
				yEnd = (int)roundf(actual_lines[i][4]);
				float zStart = actual_lines[i][2];
				float zEnd = actual_lines[i][5];
				drawLineGeneralized(xStart, yStart, zStart, xEnd, yEnd, zEnd, grid, minzcoord);
			}
			plotLines(grid, brightgrid);
		} else if(option == 'l') {
			dirn = 'x';
			amt = 5;

			if (dirn == 'x') {
				centerofshape[0] += amt;
			} else if (dirn == 'y') {
				centerofshape[1] += amt;
			}

			for(int i = 0; i < triangle_no; i++) {
				vector[0] = vertices_list[i][0][0];
				vector[1] = vertices_list[i][0][1];
				vector[2] = vertices_list[i][0][2];

				move(vector, dirn, amt);

				vertices_list[i][0][0] = vector[0];
				vertices_list[i][0][1] = vector[1];
				vertices_list[i][0][2] = vector[2];

				vector[0] = vertices_list[i][1][0];
				vector[1] = vertices_list[i][1][1];
				vector[2] = vertices_list[i][1][2];

				move(vector, dirn, amt);

				vertices_list[i][1][0] = vector[0];
				vertices_list[i][1][1] = vector[1];
				vertices_list[i][1][2] = vector[2];

				vector[0] = vertices_list[i][2][0];
				vector[1] = vertices_list[i][2][1];
				vector[2] = vertices_list[i][2][2];

				move(vector, dirn, amt);

				vertices_list[i][2][0] = vector[0];
				vertices_list[i][2][1] = vector[1];
				vertices_list[i][2][2] = vector[2];

				for(int j = 0; j < 3; j++) {
					vertices[j][0] = vertices_list[i][j][0];
					vertices[j][1] = vertices_list[i][j][1];
					vertices[j][2] = vertices_list[i][j][2];
				}

				pts_no = formtriangle(vertices, triangle_list, 0, size);
				compute_lighting_normal(vertices, lighting_normal);

				for(int j = 0; j < pts_no-1; j++) {
					int x = (int)roundf(triangle_list[0][j][0]);
					int y = (int)roundf(triangle_list[0][j][1]);
					if(!inside_grid(x, y)) {
						continue;
					}
					float z = triangle_list[0][j][2];

					if(z < minzcoord[y][x]) {
						minzcoord[y][x] = z;
						grid[y][x] = 2;
						brightgrid[y][x] = lighting(light_vector, lighting_normal);
					}
				}

				line_no = 3*i;
				convert_to_lines(vertices, three_d_lines, line_no);
			}
			for(int i = 0; i < lines; i++) {
				vector[0] = actual_lines[i][0];
				vector[1] = actual_lines[i][1];
				vector[2] = actual_lines[i][2];

				move(vector, dirn, amt);

				actual_lines[i][0] = vector[0];
				actual_lines[i][1] = vector[1];
				actual_lines[i][2] = vector[2];

				vector[0] = actual_lines[i][3];
				vector[1] = actual_lines[i][4];
				vector[2] = actual_lines[i][5];

				move(vector, dirn, amt);

				actual_lines[i][3] = vector[0];
				actual_lines[i][4] = vector[1];
				actual_lines[i][5] = vector[2];

			}


			for (int i = 0; i < lines; i++) {
				xStart = (int)roundf(actual_lines[i][0]);
				yStart = (int)roundf(actual_lines[i][1]);
				xEnd = (int)roundf(actual_lines[i][3]);
				yEnd = (int)roundf(actual_lines[i][4]);
				float zStart = actual_lines[i][2];
				float zEnd = actual_lines[i][5];
				drawLineGeneralized(xStart, yStart, zStart, xEnd, yEnd, zEnd, grid, minzcoord);
			}
			plotLines(grid, brightgrid);
		}
	}
        return 0;
}
