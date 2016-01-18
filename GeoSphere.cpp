#include "GeoSphere.h"

#include <stdlib.h>
#include <GL/glut.h>
#include <iostream>
#include "Globals.h"

using namespace std;

void GeoSphere::render(Matrix4 m)
{
	GLUquadricObj *sphere = NULL;
	sphere = gluNewQuadric();
	gluQuadricDrawStyle(sphere, GLU_FILL);
	gluQuadricTexture(sphere, GL_TRUE);
	gluQuadricNormals(sphere, GLU_SMOOTH);


	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();
	glMultMatrixf(m.ptr());

	Globals::trooper.bind();
	gluSphere(sphere, 2, 50, 50);
	Globals::trooper.unbind();

	glPopMatrix();
}