#include "Sphere.h"

#ifdef __APPLE__
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
#endif


Sphere::Sphere(double radius, int slices, int stacks) : Drawable()
{
    this->radius = radius;
    this->slices = slices;
    this->stacks = stacks;
	this->init = true;
}

void Sphere::draw(DrawData& data)
{
	GLUquadricObj *sphere = NULL;
	sphere = gluNewQuadric();
	gluQuadricDrawStyle(sphere, GLU_FILL);
	gluQuadricTexture(sphere, GL_TRUE);
	gluQuadricNormals(sphere, GLU_SMOOTH);

    material.apply();
    
    glMatrixMode(GL_MODELVIEW);

	//Rotatation for helmets
	if (this->init)
	{
		Matrix4 rotate;
		Vector3 axis;

		axis.set(1, 0, 0);
		rotate.makeRotateArbitrary(axis, -(3 * PI) / 5);
		this->toWorld = this->toWorld * rotate;

		axis.set(0, 1, 0);
		rotate.makeRotateArbitrary(axis, PI / 8);
		this->toWorld = rotate * this->toWorld;

		this->init = false;
	}
    
    glPushMatrix();
    glMultMatrixf(toWorld.ptr());
    
    //glutSolidSphere(radius, slices, stacks);
	gluSphere(sphere, radius, slices, stacks);
    
    glPopMatrix();
}


void Sphere::update(UpdateData& data)
{
    //
}

void Sphere::rotate()
{
	Matrix4 rotate;
	Vector3 axis;

	axis.set(1, 0, 0);
	rotate.makeRotateArbitrary(axis, -(3 * PI) / 5);
	this->toWorld = rotate * this->toWorld;

	axis.set(0, 1, 0);
	rotate.makeRotateArbitrary(axis, PI / 8);
	this->toWorld = rotate * this->toWorld;
}


