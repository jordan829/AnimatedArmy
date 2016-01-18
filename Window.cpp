#include <iostream>
using namespace std;

#ifdef __APPLE__
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
#endif

#include "Window.h"
#include "Cube.h"
#include "Skybox.h"
#include "Matrix4.h"
#include "Globals.h"
#include "Rasterizer.h"
#include "MatrixTransform.h"
#include "GeoSphere.h"
#include "GeoCube.h"
#include "GeoChest.h"
#include "GeoBall.h"

int Window::width  = 960;   //Set window width in pixels here
int Window::height = 540;   //Set window height in pixels here

bool leftClick = false;
bool rightClick = false;
bool zoom = false;
int mouseX = 0;
int mouseY = 0;

Vector3 lastPoint;

//////////////////////////////////////////////////////////////////////////////////////////////
static int armySize = 10;	//10 x 10 grid
static float animate_scale = 0.01;

vector<vector<MatrixTransform*>> trooper(armySize, vector<MatrixTransform*>(armySize));

static float leftarm_angle = 0;
static float leftarm_direction = -14;
static float rightarm_angle = 0;
static float rightarm_direction = 14;

static float head_angle = 0;
static float leftleg_angle = 0;
static float leftleg_direction = 14;
static float rightleg_angle = 0;
static float rightleg_direction = -14;

Matrix4 meh;	//Used for grid translation / scaling
Matrix4 bleh;	//Used for grid rotation

Skybox skybox(0);
//////////////////////////////////////////////////////////////////////////////////////////////

void Window::initialize(void)
{
	meh.identity();
	bleh.identity();

	//Default light
	Vector4 lightPos(5.0, 0.0, 67.0, 1.0);
	Globals::light.position = lightPos;
	Globals::light.quadraticAttenuation = 0.00005;

	glEnable(GL_NORMALIZE);

	//////////// Set camera position /////////////
	Vector3 e, d, u;
	e.set(0.0, 0.0, 112.0);
	d.set(0.0, 10.0, 0.0);
	u.set(0.0, 1.0, 0.0);
	Globals::camera.set(e, d, u);
	Globals::camera.update();
	//////////////////////////////////////////////

	//Initialize skybox
	skybox = Skybox(999.0);

	/* * * * * * * * * * *
	*   Create Textures  *
	*					 *
	*	Texture options: *
	*	  darkskies		 *
	*	  purplenebula   *
	*	  arctic		 *
	*	  iceflats		 *
	*	 **tatooine**	 *
	* * * * * * * * * * */
	Globals::ft = Texture("tatooine_ft.ppm");	//Front face
	Globals::bk = Texture("tatooine_bk.ppm");	//Back face
	Globals::lf = Texture("tatooine_lf.ppm");	//Left face
	Globals::rt = Texture("tatooine_rt.ppm");	//Right face
	Globals::up = Texture("tatooine_up.ppm");	//Top face
	Globals::dn = Texture("tatooine_dn.ppm");	//Bottom face

	// Create stormtrooper helmet texture
	Globals::trooper = Texture("stormtrooper.ppm");

	// Create stormtrooper chestplate texture
	Globals::chestplate = Texture("chestplate.ppm");
}

//----------------------------------------------------------------------------
// Callback method called when system is idle.
// This is called at the start of every new "frame" (qualitatively)
void Window::idleCallback()
{
    //Set up a static time delta for update calls
    Globals::updateData.dt = 1.0/60.0;// 60 fps
    
    //Call the display routine to draw the cube
    displayCallback();
}

//----------------------------------------------------------------------------
// Callback method called by GLUT when graphics window is resized by the user
void Window::reshapeCallback(int w, int h)
{
    width = w;                                                       //Set the window width
    height = h;                                                      //Set the window height
    glViewport(0, 0, w, h);                                          //Set new viewport size
    glMatrixMode(GL_PROJECTION);                                     //Set the OpenGL matrix mode to Projection
    glLoadIdentity();                                                //Clear the projection matrix by loading the identity
    gluPerspective(60.0, double(width)/(double)height, 1.0, 1000.0); //Set perspective projection viewing frustum
}

//----------------------------------------------------------------------------
// Callback method called by GLUT when window readraw is necessary or when glutPostRedisplay() was called.
void Window::displayCallback()
{
    //Clear color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    //Set the OpenGL matrix mode to ModelView
    glMatrixMode(GL_MODELVIEW);
    
    //Push a matrix save point
    //This will save a copy of the current matrix so that we can
    //make changes to it and 'pop' those changes off later.
    glPushMatrix();
    
    //Replace the current top of the matrix stack with the inverse camera matrix
    //This will convert all world coordiantes into camera coordiantes
    glLoadMatrixf(Globals::camera.getInverseMatrix().ptr());
    
    //Bind the light to slot 0.  We do this after the camera matrix is loaded so that
    //the light position will be treated as world coordiantes
    //(if we didn't the light would move with the camera, why is that?)
	Globals::light.bind(0);

	//Draw army
	drawTroops();
	calculateAnimation();

	// Draw skybox faces
	skybox.draw_ft(Globals::drawData);
	skybox.draw_bk(Globals::drawData);
	skybox.draw_lf(Globals::drawData);
	skybox.draw_rt(Globals::drawData);
	skybox.draw_up(Globals::drawData);
	skybox.draw_dn(Globals::drawData);

    //Pop off the changes we made to the matrix stack this frame
    glPopMatrix();
    
    //Tell OpenGL to clear any outstanding commands in its command buffer
    //This will make sure that all of our commands are fully executed before
    //we swap buffers and show the user the freshly drawn frame
    glFlush();
    
    //Swap the off-screen buffer (the one we just drew to) with the on-screen buffer
    glutSwapBuffers();
}

void Window::drawTroops()
{
	glMatrixMode(GL_MODELVIEW);

	for (int i = 0; i < armySize; i++) 
	{
		for (int j = 0; j < armySize; j++)
		{
			Matrix4 identity;
			Matrix4 rotate, scale, trans;
			Vector3 axis;
			identity.identity();
			trooper[i][j] = &MatrixTransform(identity);

			/*****  Head  *****/
			MatrixTransform head = MatrixTransform(identity);
			axis.set(1, 0, 0);
			rotate.makeRotateArbitrary(axis, -PI / 2);
			head.setTransformationMatrix( head.getTransformationMatrix() * rotate );
			axis.set(0, 1, 0);
			rotate.makeRotateArbitrary(axis, head_angle);
			head.setTransformationMatrix( rotate * head.getTransformationMatrix() );
			head.addChild(&GeoSphere());

			/***** Torso *****/ 
			MatrixTransform torso = MatrixTransform(identity);
			scale.makeScale(2.5, 3.5, 1.5);
			torso.setTransformationMatrix( torso.getTransformationMatrix() * scale );
			trans.makeTranslate(0, -2.8, 0);
			torso.setTransformationMatrix( trans * torso.getTransformationMatrix() );
			torso.addChild(&GeoChest());
			
			/***** Left arm *****/
			MatrixTransform leftarm = MatrixTransform(identity);
			trans.makeTranslate(1.57, -1.5, 0);
			leftarm.setTransformationMatrix(trans * leftarm.getTransformationMatrix());
			axis.set(1, 0, 0);
			rotate.makeRotateArbitrary(axis, leftarm_angle);
			leftarm.setTransformationMatrix(rotate * leftarm.getTransformationMatrix());
			trans.makeTranslate(0, -2, 0);
			leftarm.setTransformationMatrix(trans * leftarm.getTransformationMatrix());

			MatrixTransform leftarmShape = MatrixTransform(identity);
			scale.makeScale(.5, 3, .75);
			leftarmShape.setTransformationMatrix(scale * leftarmShape.getTransformationMatrix());
			leftarmShape.addChild(&GeoCube());

			MatrixTransform lhand = MatrixTransform(identity);
			scale.makeScale(.26, .26, .26);
			lhand.setTransformationMatrix(scale * lhand.getTransformationMatrix());
			trans.makeTranslate(0, -1.25, 0);
			lhand.setTransformationMatrix(trans * lhand.getTransformationMatrix());
			lhand.addChild(&GeoBall());

			leftarm.addChild(&leftarmShape);
			leftarm.addChild(&lhand);

			/***** Right arm *****/
			MatrixTransform rightarm = MatrixTransform(identity);
			trans.makeTranslate(-1.57, -1.5, 0);
			rightarm.setTransformationMatrix(trans * rightarm.getTransformationMatrix());
			axis.set(1, 0, 0);
			rotate.makeRotateArbitrary(axis, rightarm_angle);
			rightarm.setTransformationMatrix(rotate * rightarm.getTransformationMatrix());
			trans.makeTranslate(0, -2, 0);
			rightarm.setTransformationMatrix(trans * rightarm.getTransformationMatrix());

			MatrixTransform rightarmShape = MatrixTransform(identity);
			scale.makeScale(.5, 3, .75);
			rightarmShape.setTransformationMatrix(scale * rightarmShape.getTransformationMatrix());
			rightarmShape.addChild(&GeoCube());

			MatrixTransform rhand = MatrixTransform(identity);
			scale.makeScale(.26, .26, .26);
			rhand.setTransformationMatrix(scale * rhand.getTransformationMatrix());
			trans.makeTranslate(0, -1.25, 0);
			rhand.setTransformationMatrix(trans * rhand.getTransformationMatrix());
			rhand.addChild(&GeoBall());

			rightarm.addChild(&rightarmShape);
			rightarm.addChild(&rhand);

			/***** Left leg *****/
			MatrixTransform leftleg = MatrixTransform(identity);
			trans.makeTranslate(0.75, -1.5, 0);
			leftleg.setTransformationMatrix(trans * leftleg.getTransformationMatrix());
			axis.set(1, 0, 0);
			rotate.makeRotateArbitrary(axis, leftleg_angle);
			leftleg.setTransformationMatrix(rotate * leftleg.getTransformationMatrix());
			trans.makeTranslate(0, -4, 0);
			leftleg.setTransformationMatrix(trans * leftleg.getTransformationMatrix());

			MatrixTransform leftlegShape = MatrixTransform(identity);
			scale.makeScale(1, 3, 1);
			leftlegShape.setTransformationMatrix(scale * leftlegShape.getTransformationMatrix());
			leftlegShape.addChild(&GeoCube());

			MatrixTransform lfoot = MatrixTransform(identity);
			trans.makeTranslate(0, -1, 0.5);
			lfoot.setTransformationMatrix(trans * lfoot.getTransformationMatrix());
			lfoot.addChild(&GeoCube());

			leftleg.addChild(&leftlegShape);
			leftleg.addChild(&lfoot);

			/***** Right leg *****/
			MatrixTransform rightleg = MatrixTransform(identity);
			trans.makeTranslate(-0.75, -1.5, 0);
			rightleg.setTransformationMatrix(trans * rightleg.getTransformationMatrix());
			axis.set(1, 0, 0);
			rotate.makeRotateArbitrary(axis, rightleg_angle);
			rightleg.setTransformationMatrix(rotate * rightleg.getTransformationMatrix());
			trans.makeTranslate(0, -4, 0);
			rightleg.setTransformationMatrix(trans * rightleg.getTransformationMatrix());

			MatrixTransform rightlegShape = MatrixTransform(identity);
			scale.makeScale(1, 3, 1);
			rightlegShape.setTransformationMatrix(scale * rightlegShape.getTransformationMatrix());
			rightlegShape.addChild(&GeoCube());

			MatrixTransform rfoot = MatrixTransform(identity);
			trans.makeTranslate(0, -1, 0.5);
			rfoot.setTransformationMatrix(trans * rfoot.getTransformationMatrix());
			rfoot.addChild(&GeoCube());

			rightleg.addChild(&rightlegShape);
			rightleg.addChild(&rfoot);

			//Add parts to stormtrooper
			trooper[i][j]->addChild(&head);
			trooper[i][j]->addChild(&torso);
			trooper[i][j]->addChild(&leftarm);
			trooper[i][j]->addChild(&rightarm);
			trooper[i][j]->addChild(&leftleg);
			trooper[i][j]->addChild(&rightleg);

			//Draw the stormtrooper
			Matrix4 translate;
			translate.makeTranslate(-10 * i, 0, -10 * j);
			trooper[i][j]->setTransformationMatrix(translate);
			translate.makeTranslate(45, 0, 45);
			trooper[i][j]->setTransformationMatrix(trooper[i][j]->getTransformationMatrix() * translate);
			trooper[i][j]->setTransformationMatrix(bleh * trooper[i][j]->getTransformationMatrix());
			trooper[i][j]->setTransformationMatrix(meh * trooper[i][j]->getTransformationMatrix());
			Matrix4 id;
			id.identity();
			trooper[i][j]->draw(id);
		}
	}
}

void Window::calculateAnimation()
{
	//Arm animations
	leftarm_angle += animate_scale * leftarm_direction;
	rightarm_angle += animate_scale * rightarm_direction;

	//Reverse direction once limit is reached
	if (leftarm_angle > (PI / 4) || leftarm_angle < (-PI / 4))
		leftarm_direction = -leftarm_direction;

	if (rightarm_angle > (PI / 4) || rightarm_angle < (-PI / 4))
		rightarm_direction = -rightarm_direction;

	//Leg animations
	leftleg_angle += animate_scale * leftleg_direction;
	rightleg_angle += animate_scale * rightleg_direction;
	head_angle = leftleg_angle;

	if (leftleg_angle > (PI / 4) || leftleg_angle < (-PI / 4))
		leftleg_direction = -leftleg_direction;

	if (rightleg_angle > (PI / 4) || rightleg_angle < (-PI / 4))
		rightleg_direction = -rightleg_direction;
}

void Window::keyCallback(unsigned char key, int x, int y)
{
	Matrix4 xTrans, yTrans, zTrans;	// Translation matrices
	Matrix4 scale;					// Scaling matrix
	Matrix4 rotate;					// Rotation matrix
	Vector3 z;

	switch (key)
	{
		// 'x' : Move left
	case 'x':
		xTrans.makeTranslate(-1.2, 0, 0);
		meh = xTrans * meh;
		break;

		// 'X' : Move right
	case 'X':
		xTrans.makeTranslate(1.2, 0, 0);
		meh = xTrans * meh;
		break;

		// 'y' : Move down
	case 'y':
		yTrans.makeTranslate(0, -1.2, 0);
		meh = yTrans * meh;
		break;

		// 'Y' : Move up
	case 'Y':
		yTrans.makeTranslate(0, 1.2, 0);
		meh = yTrans * meh;
		break;

		// 'z' : Move in
	case 'z':
		zTrans.makeTranslate(0, 0, -1.2);
		meh = zTrans * meh;
		break;

		// 'Z' : Move out
	case 'Z':
		zTrans.makeTranslate(0, 0, 1.2);
		meh = zTrans * meh;
		break;

		// 'r' : Reset
	case 'r':
		meh.identity();
		bleh.identity();
		break;

		// 'o' : Orbit clockwise
	case 'o':
		z.set(0, 0, 1);
		rotate.makeRotateArbitrary(z, -.16);
		bleh = bleh * rotate;
		break;

		// 'O' : Orbit counterclockwise
	case 'O':
		z.set(0, 0, 1);
		rotate.makeRotateArbitrary(z, .16);
		bleh = bleh * rotate;
		break;

		// 's' : Scale down
	case 's':
		scale.makeScale(0.95, 0.95, 0.95);
		meh = scale * meh;
		break;

		// 'S' : Scale up
	case 'S':
		scale.makeScale(1.05, 1.05, 1.05);
		meh = scale * meh;
		break;

	case 'c':
		zoom = !zoom;
		break;
	}
}

void Window::mouseFunc(int key, int state, int x, int y)
{
	switch (key)	// Determine when mouse button is clicked
	{
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN)
		{
			leftClick = true;
			lastPoint = trackBallMapping(Window::width, Window::height, x, y);
			mouseX = x;
			mouseY = y;
		}

		else
			leftClick = false;
		break;

	case GLUT_RIGHT_BUTTON:
		if (state == GLUT_DOWN)
		{
			rightClick = true;
			lastPoint = trackBallMapping(Window::width, Window::height, x, y);
			mouseX = x;
			mouseY = y;
		}

		else
			rightClick = false;
		break;
	}
}

void Window::mouseMotion(int x, int y)
{
	Matrix4 xyTrans, zTrans;
	Matrix4 scale, rotate;
	Vector3 axis;
	Vector3 currentPoint;
	Vector3 point;
	float rotateAngle = 0.0;

	currentPoint = trackBallMapping(Window::width, Window::height, mouseX, mouseY);

	if (leftClick)	//Rotate
	{
		axis = lastPoint.cross(currentPoint);
		rotateAngle = lastPoint.angle(currentPoint);
		rotate = rotate.makeRotateArbitrary(axis, rotateAngle);

		bleh = rotate * bleh;

		lastPoint = currentPoint;
		mouseX = x;
		mouseY = y;
	}

	else if (rightClick)
	{
		//Translate along z axis
		if (zoom)
		{
			point = lastPoint - currentPoint;
			point = point.scale(9.0);
			zTrans.makeTranslate(0, 0, point[1]);

			meh = zTrans * meh;

			lastPoint = currentPoint;
			mouseX = x;
			mouseY = y;
		}

		//Translate along x / y axis
		else
		{
			point = currentPoint - lastPoint;
			point = point.scale(50.0);
			xyTrans.makeTranslate(point[0], point[1], 0);

			meh = xyTrans * meh;

			lastPoint = currentPoint;
			mouseX = x;
			mouseY = y;
		}
	}
}

Vector3 Window::trackBallMapping(int width, int height, int mouseX, int mouseY)
{
	Vector3 v;
	float d;
	v[0] = (2.0 * mouseX - width) / width;
	v[1] = (height - 2.0 * mouseY) / height;
	v[2] = 0.0;
	d = sqrtf(v[0] * v[0] + v[1] * v[1]);
	d = (d<1.0) ? d : 1.0;
	v[2] = sqrtf(1.001 - d*d);
	return v.normalize();
}