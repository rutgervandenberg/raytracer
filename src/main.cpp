#include <iostream>
#include <ctime>
#include <cstdio>
#include "config.hpp"
#include "raytrace.hpp"
#include "mesh.hpp"
#include "octree.hpp"
#include "camera.hpp"

#include <gl/glut.h>
#include "texture.hpp"

Texture tex[2];
int activeTexIndex;

// objects
Config conf("raytrace.cfg");
Mesh mesh("mesh/scene.obj");
Octree octree(mesh);
Image image(conf.RAYTRACE_RES_X, conf.RAYTRACE_RES_Y);
Camera cam(vec4(2.0f, 0.1f - 2, 1.0f, 0.0f), 0.0f, 0.5f * 3.1415926f);

void keyboard(unsigned char key, int x, int y) {
	if (key == 'w')
		cam.straight = 1;
	if (key == 's')
		cam.straight = -1;
	if (key == 'a')
		cam.strafe = 1;
	if (key == 'd')
		cam.strafe = -1;
	if (key == 'q')
		cam.vert = 1;
	if (key == 'e')
		cam.vert = -1;
	if (key == 'j')
		cam.xturn = -1;
}

void keyup(unsigned char key, int x, int y) {
	if (key == 'w')
		cam.straight = 0;
	if (key == 's')
		cam.straight = 0;
	if (key == 'a')
		cam.strafe = 0;
	if (key == 'd')
		cam.strafe = 0;
	if (key == 'q')
		cam.vert = 0;
	if (key == 'e')
		cam.vert = 0;
}

void drawText(const char *string) {
	//TEXT

	void* font = GLUT_BITMAP_HELVETICA_18;
	glDisable(GL_DEPTH_TEST); // also disable the depth test so renders on top

	const char *c;
	glRasterPos2f(-1.0f, -1.0f);
	for (c = string; *c != '\0'; c++) {
		glutBitmapCharacter(font, *c);
	}
}

int old = 0;
void display(void) {
	// raytrace
	raytrace2(cam, conf, mesh, octree, &image);

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	tex[0].upload(image);
	tex[0].draw();

	// update camera
	cam.move();

	// fps
	int ms = glutGet(GLUT_ELAPSED_TIME);
	int fps = 1000 / (ms - old);
	old = ms;
	char txt[1024];
	sprintf(txt, "%d FPS", fps);
	drawText(txt);
	glColor3f(1, 1, 1);

	glutSwapBuffers();
	glutPostRedisplay();
}

/**
 *	Load the mesh and configuration
 *	then raytrace the image
 *	then save it to a file.
 */
int main(int argc, char** argv) {
	// load meshname from commandline
	const char* meshname = "mesh/scene.obj";
	if (argc == 2)
		meshname = argv[1];

	// load objects

	// stupid glut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(conf.WINDOW_RES_X, conf.WINDOW_RES_Y);
	glutCreateWindow("Raytracer");
	glutSetKeyRepeat(false);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyup);
	glutDisplayFunc(display);
	tex[0].init();
	tex[1].init();
	glutMainLoop();


	//image.write("output");

	//system("pause");
}