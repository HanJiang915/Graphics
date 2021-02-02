#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define _USE_MATH_DEFINES
#include <math.h>

#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#endif

#include "glew.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include "glut.h"


//	This is a sample OpenGL / GLUT program
//
//	The objective is to draw a 3d object and change the color of the axes
//		with a glut menu
//
//	The left mouse button does rotation
//	The middle mouse button does scaling
//	The user interface allows:
//		1. The axes to be turned on and off
//		2. The color of the axes to be changed
//		3. Debugging to be turned on and off
//		4. Depth cueing to be turned on and off
//		5. The projection to be changed
//		6. The transformations to be reset
//		7. The program to quit
//
//	Author:			Han Jiang

// NOTE: There are a lot of good reasons to use const variables instead
// of #define's.  However, Visual C++ does not allow a const variable
// to be used as an array size or as the case in a switch( ) statement.  So in
// the following, all constants are const variables except those which need to
// be array sizes or cases in switch( ) statements.  Those are #defines.

// title of these windows:

const char* WINDOWTITLE = { "OpenGL / GLUT Sample -- Han Jiang" };
const char* GLUITITLE = { "User Interface Window" };

// what the glui package defines as true and false:

const int GLUITRUE = { true };
const int GLUIFALSE = { false };

// the escape key:

#define ESCAPE		0x1b

// initial window size:

const int INIT_WINDOW_SIZE = { 600 };

// size of the 3d box:

const float BOXSIZE = { 2.f };

// multiplication factors for input interaction:
//  (these are known from previous experience)

const float ANGFACT = { 1. };
const float SCLFACT = { 0.005f };

// minimum allowable scale factor:

const float MINSCALE = { 0.05f };

// scroll wheel button values:

const int SCROLL_WHEEL_UP = { 3 };
const int SCROLL_WHEEL_DOWN = { 4 };

// equivalent mouse movement when we click a the scroll wheel:

const float SCROLL_WHEEL_CLICK_FACTOR = { 5. };

// active mouse buttons (or them together):

const int LEFT = { 4 };
const int MIDDLE = { 2 };
const int RIGHT = { 1 };

// which projection:

enum Projections
{
	ORTHO,
	PERSP
};

// which button:

enum ButtonVals
{
	RESET,
	QUIT
};

// window background color (rgba):

const GLfloat BACKCOLOR[] = { 0., 0., 0., 1. };

// line width for the axes:

const GLfloat AXES_WIDTH = { 3. };

// the color numbers:
// this order must match the radio button order

enum Colors
{
	RED,
	YELLOW,
	GREEN,
	CYAN,
	BLUE,
	MAGENTA,
	WHITE,
	BLACK
};

char* ColorNames[] =
{
	(char*)"Red",
	(char*)"Yellow",
	(char*)"Green",
	(char*)"Cyan",
	(char*)"Blue",
	(char*)"Magenta",
	(char*)"White",
	(char*)"Black"
};

// the color definitions:
// this order must match the menu order

const GLfloat Colors[][3] =
{
	{ 1., 0., 0. },		// red
	{ 1., 1., 0. },		// yellow
	{ 0., 1., 0. },		// green
	{ 0., 1., 1. },		// cyan
	{ 0., 0., 1. },		// blue
	{ 1., 0., 1. },		// magenta
	{ 1., 1., 1. },		// white
	{ 0., 0., 0. },		// black
};

// fog parameters:

const GLfloat FOGCOLOR[4] = { .0, .0, .0, 1. };
const GLenum  FOGMODE = { GL_LINEAR };
const GLfloat FOGDENSITY = { 0.30f };
const GLfloat FOGSTART = { 1.5 };
const GLfloat FOGEND = { 4. };


// what options should we compile-in?
// in general, you don't need to worry about these
// i compile these in to show class examples of things going wrong

//#define DEMO_Z_FIGHTING
//#define DEMO_DEPTH_BUFFER

// should we turn the shadows on?

//#define ENABLE_SHADOWS

//****************************************************************************
//global values
#define NUMCURVES 7 //mayebe 9
#define NUMPOINTS 28
#define MS_PER_CYCLE	40000
//from professor Mike's Possible Way to Organize the Data
struct Point
{
	float x0, y0, z0;       // initial coordinates
	float x, y, z;        // animated coordinates
};

struct Curve
{
	float r, g, b;
	Point p0, p1, p2, p3;
};

Curve Curves[NUMCURVES];		// if you are creating a pattern of curves
//****************************************************************************



// non-constant global variables:

int		ActiveButton;			// current button that is down
GLuint	AxesList;				// list to hold the axes
int		AxesOn;					// != 0 means to draw the axes
int		DebugOn;				// != 0 means to print debugging info
int		DepthCueOn;				// != 0 means to use intensity depth cueing
int		DepthBufferOn;			// != 0 means to use the z-buffer
int		DepthFightingOn;		// != 0 means to force the creation of z-fighting
GLuint	BoxList;				// object display list
int		MainWindow;				// window id for main graphics window
float	Scale;					// scaling factor
int		ShadowsOn;				// != 0 means to turn shadows on
int		WhichColor;				// index into Colors[ ]
int		WhichProjection;		// ORTHO or PERSP
int		Xmouse, Ymouse;			// mouse values
float	Xrot, Yrot;				// rotation angles in degrees

float   Time;
bool	Frozen = false;
int     LineOn = 0;
int     PointOn = 0;
float   rgb[3], hsv[3];

// function prototypes:

void	Animate();
void	Display();
void	DoAxesMenu(int);
void	DoColorMenu(int);
void	DoDepthBufferMenu(int);
void	DoDepthFightingMenu(int);
void	DoDepthMenu(int);
void	DoDebugMenu(int);
void	DoMainMenu(int);
void	DoProjectMenu(int);
void	DoShadowMenu();
void	DoRasterString(float, float, float, char*);
void	DoStrokeString(float, float, float, float, char*);
float	ElapsedSeconds();
void	InitGraphics();
void	InitLists();
void	InitMenus();
void	Keyboard(unsigned char, int, int);
void	MouseButton(int, int, int, int);
void	MouseMotion(int, int);
void	Reset();
void	Resize(int, int);
void	Visibility(int);

void			Axes(float);
unsigned char* BmpToTexture(char*, int*, int*);
int				ReadInt(FILE*);
short			ReadShort(FILE*);

void			Cross(float[3], float[3], float[3]);
float			Dot(float[3], float[3]);
float			Unit(float[3], float[3]);

//******************************************************************
// add in functions
void        RotateX(Point*, float, float, float, float);
void        RotateY(Point*, float, float, float, float);
void        RotateZ(Point*, float, float, float, float);
void		HsvRgb(float[3], float[3]);   //use it in this program 
void        DoLineMenu(int);
void        DoPointMenu(int);
//******************************************************************


// main program:

int
main(int argc, char* argv[])
{
	// turn on the glut package:
	// (do this before checking argc and argv since it might
	// pull some command line arguments out)

	glutInit(&argc, argv);

	// setup all the graphics stuff:

	InitGraphics();

	// create the display structures that will not change:

	InitLists();

	// init all the global variables used by Display( ):
	// this will also post a redisplay

	Reset();

	// setup all the user interface stuff:

	InitMenus();

	// draw the scene once and wait for some interaction:
	// (this will never return)

	glutSetWindow(MainWindow);
	glutMainLoop();

	// glutMainLoop( ) never returns
	// this line is here to make the compiler happy:

	return 0;
}


// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display( ) from here -- let glutMainLoop( ) do it

void
Animate()
{
	// put animation stuff in here -- change some global variables
	// for Display( ) to find:
	int ms = glutGet(GLUT_ELAPSED_TIME);
	ms %= MS_PER_CYCLE;
	Time = (float)ms / (float)MS_PER_CYCLE;		// [0.,1.)
	// force a call to Display( ) next time it is convenient:

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


// draw the complete scene:

void
Display()
{
	if (DebugOn != 0)
	{
		fprintf(stderr, "Display\n");
	}


	// set which window we want to do the graphics into:

	glutSetWindow(MainWindow);


	// erase the background:

	glDrawBuffer(GL_BACK);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
#ifdef DEMO_DEPTH_BUFFER
	if (DepthBufferOn == 0)
		glDisable(GL_DEPTH_TEST);
#endif


	// specify shading to be flat:

	glShadeModel(GL_FLAT);


	// set the viewport to a square centered in the window:

	GLsizei vx = glutGet(GLUT_WINDOW_WIDTH);
	GLsizei vy = glutGet(GLUT_WINDOW_HEIGHT);
	GLsizei v = vx < vy ? vx : vy;			// minimum dimension
	GLint xl = (vx - v) / 2;
	GLint yb = (vy - v) / 2;
	glViewport(xl, yb, v, v);


	// set the viewing volume:
	// remember that the Z clipping  values are actually
	// given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D( ) IF YOU ARE DOING 2D !

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (WhichProjection == ORTHO)
		glOrtho(-3., 3., -3., 3., 0.1, 1000.);
	else
		gluPerspective(90., 1., 0.1, 1000.);


	// place the objects into the scene:

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


	// set the eye position, look-at position, and up-vector:

	gluLookAt(0., 0., 3., 0., 0., 0., 0., 1., 0.);


	// rotate the scene:

	glRotatef((GLfloat)Yrot, 0., 1., 0.);
	glRotatef((GLfloat)Xrot, 1., 0., 0.);


	// uniformly scale the scene:

	if (Scale < MINSCALE)
		Scale = MINSCALE;
	glScalef((GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale);


	// set the fog parameters:

	if (DepthCueOn != 0)
	{
		glFogi(GL_FOG_MODE, FOGMODE);
		glFogfv(GL_FOG_COLOR, FOGCOLOR);
		glFogf(GL_FOG_DENSITY, FOGDENSITY);
		glFogf(GL_FOG_START, FOGSTART);
		glFogf(GL_FOG_END, FOGEND);
		glEnable(GL_FOG);
	}
	else
	{
		glDisable(GL_FOG);
	}


	// possibly draw the axes:

	if (AxesOn != 0)
	{
		glColor3fv(&Colors[WhichColor][0]);
		glCallList(AxesList);
	}


	// since we are using glScalef( ), be sure normals get unitized:

	glEnable(GL_NORMALIZE);


	// draw the current object:
//*************************************************************************

//set color
	hsv[0] = 360 - 90 * sin(60 * Time);
	hsv[1] = 1.0;
	hsv[2] = 1.0;
	HsvRgb(hsv, rgb);
	//set animation

	
	for (int i = 1; i < 5; i++) {
		RotateX(&Curves[i].p1, 4 * sin(50 * M_PI * Time), -1, 0, -1);
		RotateX(&Curves[i].p2, 4 * sin(50 * M_PI * Time), 1, 0, 1);
		Curves[i].r = 1; Curves[i].g = 0; Curves[i].b = 0.6;
	}
	
	RotateX(&Curves[6].p1, 30 * sin(-50 * M_PI * Time), 5, 0, 0);
	RotateX(&Curves[6].p2, 30 * sin(-50 * M_PI * Time), 5, -1, 0);
	Curves[6].r = rgb[0]; Curves[6].g = 0.5 * rgb[1]; Curves[6].b = 0.5 * rgb[2];


	RotateX(&Curves[5].p1, 30 * sin(50 * M_PI * Time), 5, 0, 0);
	RotateX(&Curves[5].p2, 30 * sin(50 * M_PI * Time), 5, -1, 0);
	Curves[5].r = rgb[0]; Curves[5].g = 0.5 * rgb[1]; Curves[5].b = 0.5 * rgb[2];



	//draw lines and points
	for (int i = 0; i < NUMCURVES; i++)
	{
		glLineWidth(3.);
		glColor3f(Curves[i].r, Curves[i].g, Curves[i].b);
		glBegin(GL_LINE_STRIP);
		for (int it = 0; it <= NUMPOINTS; it++)
		{
			float t = (float)it / (float)NUMPOINTS;
			t = 1.f - t;

		 
			float x = 0.5f * (2.f * Curves[i].p1.x + t * (-1.f * Curves[i].p0.x + Curves[i].p2.x) + t * t * (2.f * Curves[i].p0.x - 0.5f * Curves[i].p1.x + 4.f * Curves[i].p2.x - Curves[i].p3.x) + t * t * t * (-1.f * Curves[i].p0.x + 3.f * Curves[i].p1.x - 3.f * Curves[i].p2.x + Curves[i].p3.x));
			float y = 0.5f * (2.f * Curves[i].p1.y + t * (-1.f * Curves[i].p0.y + Curves[i].p2.y) + t * t * (2.f * Curves[i].p0.y - 0.5f * Curves[i].p1.y + 4.f * Curves[i].p2.y - Curves[i].p3.y) + t * t * t * (-1.f * Curves[i].p0.y + 3.f * Curves[i].p1.y - 3.f * Curves[i].p2.y + Curves[i].p3.y));
			float z = 0.5f * (2.f * Curves[i].p1.z + t * (-1.f * Curves[i].p0.z + Curves[i].p2.z) + t * t * (2.f * Curves[i].p0.z - 0.5f * Curves[i].p1.z + 4.f * Curves[i].p2.z - Curves[i].p3.z) + t * t * t * (-1.f * Curves[i].p0.z + 3.f * Curves[i].p1.z - 3.f * Curves[i].p2.z + Curves[i].p3.z));
			
			glVertex3f(x, y, z);
		}
		glEnd();
		glLineWidth(1.);

		//http://web.engr.oregonstate.edu/~mjb/cs550/PDFs/proj06-catmull-rom.pdf

	}

	//*************************************************************************



		// draw some gratuitous text that just rotates on top of the scene:

		/*glDisable( GL_DEPTH_TEST );
		glColor3f( 0., 1., 1. );
		DoRasterString( 0., 1., 0., (char *)"Project 6" );*/


		// draw some gratuitous text that is fixed on the screen:
		//
		// the projection matrix is reset to define a scene whose
		// world coordinate system goes from 0-100 in each axis
		//
		// this is called "percent units", and is just a convenience
		//
		// the modelview matrix is reset to identity as we don't
		// want to transform these coordinates

	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0., 100., 0., 100.);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glColor3f(1., 1., 1.);
	DoRasterString(5., 5., 0., (char*)"Project 6");


	// swap the double-buffered framebuffers:

	glutSwapBuffers();


	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !

	glFlush();
}


void
DoAxesMenu(int id)
{
	AxesOn = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


void
DoColorMenu(int id)
{
	WhichColor = id - RED;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


void
DoDebugMenu(int id)
{
	DebugOn = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


void
DoDepthBufferMenu(int id)
{
	DepthBufferOn = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


void
DoDepthFightingMenu(int id)
{
	DepthFightingOn = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


void
DoDepthMenu(int id)
{
	DepthCueOn = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

//**********************************************************
// Following functions provided by Professor Mike
// Rotating a Point an Angle about the X Axis Around a Center
void
RotateX(Point* p, float deg, float xc, float yc, float zc)
{
	float rad = deg * (M_PI / 180.f);         // radians
	float x = p->x0 - xc;
	float y = p->y0 - yc;
	float z = p->z0 - zc;

	float xp = x;
	float yp = y * cos(rad) - z * sin(rad);
	float zp = y * sin(rad) + z * cos(rad);

	p->x = xp + xc;
	p->y = yp + yc;
	p->z = zp + zc;
}

//Rotating a Point an Angle about the Y Axis Around a Center
void
RotateY(Point* p, float deg, float xc, float yc, float zc)
{
	float rad = deg * (M_PI / 180.f);         // radians
	float x = p->x0 - xc;
	float y = p->y0 - yc;
	float z = p->z0 - zc;

	float xp = x * cos(rad) + z * sin(rad);
	float yp = y;
	float zp = -x * sin(rad) + z * cos(rad);

	p->x = xp + xc;
	p->y = yp + yc;
	p->z = zp + zc;
}

//Rotating a Point an Angle about the Z Axis Around a Center
void
RotateZ(Point* p, float deg, float xc, float yc, float zc)
{
	float rad = deg * (M_PI / 180.f);         // radians
	float x = p->x0 - xc;
	float y = p->y0 - yc;
	float z = p->z0 - zc;

	float xp = x * cos(rad) - y * sin(rad);
	float yp = x * sin(rad) + y * cos(rad);
	float zp = z;

	p->x = xp + xc;
	p->y = yp + yc;
	p->z = zp + zc;
}


void
DoPointMenu(int id)
{
	WhichProjection = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


void
DoLineMenu(int id)
{
	WhichProjection = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}
//***************************************************************

// main menu callback:

void
DoMainMenu(int id)
{
	switch (id)
	{
	case RESET:
		Reset();
		break;

	case QUIT:
		// gracefully close out the graphics:
		// gracefully close the graphics window:
		// gracefully exit the program:
		glutSetWindow(MainWindow);
		glFinish();
		glutDestroyWindow(MainWindow);
		exit(0);
		break;

	default:
		fprintf(stderr, "Don't know what to do with Main Menu ID %d\n", id);
	}

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


void
DoProjectMenu(int id)
{
	PointOn = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


void
DoShadowsMenu(int id)
{
	LineOn = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


// use glut to display a string of characters using a raster font:

void
DoRasterString(float x, float y, float z, char* s)
{
	glRasterPos3f((GLfloat)x, (GLfloat)y, (GLfloat)z);

	char c;			// one character to print
	for (; (c = *s) != '\0'; s++)
	{
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
	}
}


// use glut to display a string of characters using a stroke font:

void
DoStrokeString(float x, float y, float z, float ht, char* s)
{
	glPushMatrix();
	glTranslatef((GLfloat)x, (GLfloat)y, (GLfloat)z);
	float sf = ht / (119.05f + 33.33f);
	glScalef((GLfloat)sf, (GLfloat)sf, (GLfloat)sf);
	char c;			// one character to print
	for (; (c = *s) != '\0'; s++)
	{
		glutStrokeCharacter(GLUT_STROKE_ROMAN, c);
	}
	glPopMatrix();
}


// return the number of seconds since the start of the program:

float
ElapsedSeconds()
{
	// get # of milliseconds since the start of the program:

	int ms = glutGet(GLUT_ELAPSED_TIME);

	// convert it to seconds:

	return (float)ms / 1000.f;
}


// initialize the glui window:

void
InitMenus()
{
	glutSetWindow(MainWindow);

	int numColors = sizeof(Colors) / (3 * sizeof(int));
	int colormenu = glutCreateMenu(DoColorMenu);
	for (int i = 0; i < numColors; i++)
	{
		glutAddMenuEntry(ColorNames[i], i);
	}

	int axesmenu = glutCreateMenu(DoAxesMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int depthcuemenu = glutCreateMenu(DoDepthMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int depthbuffermenu = glutCreateMenu(DoDepthBufferMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int depthfightingmenu = glutCreateMenu(DoDepthFightingMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int debugmenu = glutCreateMenu(DoDebugMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);
	//*********************************************

	int pointmenu = glutCreateMenu(DoPointMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int linemenu = glutCreateMenu(DoLineMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	//*********************************************
	int projmenu = glutCreateMenu(DoProjectMenu);
	glutAddMenuEntry("Orthographic", ORTHO);
	glutAddMenuEntry("Perspective", PERSP);

	int shadowsmenu = glutCreateMenu(DoShadowsMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int mainmenu = glutCreateMenu(DoMainMenu);
	glutAddSubMenu("Axes", axesmenu);
	glutAddSubMenu("Colors", colormenu);

#ifdef DEMO_DEPTH_BUFFER
	glutAddSubMenu("Depth Buffer", depthbuffermenu);
#endif

#ifdef DEMO_Z_FIGHTING
	glutAddSubMenu("Depth Fighting", depthfightingmenu);
#endif

	glutAddSubMenu("Depth Cue", depthcuemenu);
	glutAddSubMenu("Projection", projmenu);

#ifdef ENABLE_SHADOWS
	glutAddSubMenu("Shadows", shadowsmenu);
#endif


	//*****************************************
	glutAddSubMenu("Show Point", pointmenu);
	glutAddSubMenu("Show Line", linemenu);
	//*****************************************

	glutAddMenuEntry("Reset", RESET);
	glutAddSubMenu("Debug", debugmenu);
	glutAddMenuEntry("Quit", QUIT);

	// attach the pop-up menu to the right mouse button:

	glutAttachMenu(GLUT_RIGHT_BUTTON);
}



// initialize the glut and OpenGL libraries:
//	also setup display lists and callback functions

void
InitGraphics()
{
	// request the display modes:
	// ask for red-green-blue-alpha color, double-buffering, and z-buffering:

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

	// set the initial window configuration:

	glutInitWindowPosition(0, 0);
	glutInitWindowSize(INIT_WINDOW_SIZE, INIT_WINDOW_SIZE);

	// open the window and set its title:

	MainWindow = glutCreateWindow(WINDOWTITLE);
	glutSetWindowTitle(WINDOWTITLE);

	// set the framebuffer clear values:

	glClearColor(BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3]);

	// setup the callback functions:
	// DisplayFunc -- redraw the window
	// ReshapeFunc -- handle the user resizing the window
	// KeyboardFunc -- handle a keyboard input
	// MouseFunc -- handle the mouse button going down or up
	// MotionFunc -- handle the mouse moving with a button down
	// PassiveMotionFunc -- handle the mouse moving with a button up
	// VisibilityFunc -- handle a change in window visibility
	// EntryFunc	-- handle the cursor entering or leaving the window
	// SpecialFunc -- handle special keys on the keyboard
	// SpaceballMotionFunc -- handle spaceball translation
	// SpaceballRotateFunc -- handle spaceball rotation
	// SpaceballButtonFunc -- handle spaceball button hits
	// ButtonBoxFunc -- handle button box hits
	// DialsFunc -- handle dial rotations
	// TabletMotionFunc -- handle digitizing tablet motion
	// TabletButtonFunc -- handle digitizing tablet button hits
	// MenuStateFunc -- declare when a pop-up menu is in use
	// TimerFunc -- trigger something to happen a certain time from now
	// IdleFunc -- what to do when nothing else is going on

	glutSetWindow(MainWindow);
	glutDisplayFunc(Display);
	glutReshapeFunc(Resize);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(MouseButton);
	glutMotionFunc(MouseMotion);
	glutPassiveMotionFunc(NULL);
	glutVisibilityFunc(Visibility);
	glutEntryFunc(NULL);
	glutSpecialFunc(NULL);
	glutSpaceballMotionFunc(NULL);
	glutSpaceballRotateFunc(NULL);
	glutSpaceballButtonFunc(NULL);
	glutButtonBoxFunc(NULL);
	glutDialsFunc(NULL);
	glutTabletMotionFunc(NULL);
	glutTabletButtonFunc(NULL);
	glutMenuStateFunc(NULL);
	glutTimerFunc(-1, NULL, 0);
	glutIdleFunc(Animate);

	// init glew (a window must be open to do this):

#ifdef WIN32
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		fprintf(stderr, "glewInit Error\n");
	}
	else
		fprintf(stderr, "GLEW initialized OK\n");
	fprintf(stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif

	//set the stationary curve

	Curves[0].r = 1.0; Curves[0].g = 1.0; Curves[0].b = 1.0;
	Curves[0].p0.x0 = -3.0; Curves[0].p0.y0 = 3.0; Curves[0].p0.z0 = -1.0;
	Curves[0].p0.x = Curves[0].p0.x0; Curves[0].p0.y = Curves[0].p0.y0; Curves[0].p0.z = Curves[0].p0.z0;
	Curves[0].p1.x0 = -1.0; Curves[0].p1.y0 = 2.0; Curves[0].p1.z0 = 0.0;
	Curves[0].p1.x = Curves[0].p1.x0; Curves[0].p1.y = Curves[0].p1.y0; Curves[0].p1.z = Curves[0].p1.z0;
	Curves[0].p2.x0 = 1.0; Curves[0].p2.y0 = 2.0; Curves[0].p2.z0 = 0.0;
	Curves[0].p2.x = Curves[0].p2.x0; Curves[0].p2.y = Curves[0].p2.y0; Curves[0].p2.z = Curves[0].p2.z0;
	Curves[0].p3.x0 = 3.0; Curves[0].p3.y0 = 3.0; Curves[0].p3.z0 = -1.0;
	Curves[0].p3.x = Curves[0].p3.x0; Curves[0].p3.y = Curves[0].p3.y0; Curves[0].p3.z = Curves[0].p3.z0;


	Curves[1].r = 1.0; Curves[1].g = 1.0; Curves[1].b = 1.0;
	Curves[1].p0.x0 = -3.0; Curves[1].p0.y0 = 3.0; Curves[1].p0.z0 = -1.0;
	Curves[1].p0.x = Curves[1].p0.x0; Curves[1].p0.y = Curves[1].p0.y0; Curves[1].p0.z = Curves[1].p0.z0;
	Curves[1].p1.x0 = -2.0; Curves[1].p1.y0 = -4.0; Curves[1].p1.z0 = 0.0;
	Curves[1].p1.x = Curves[1].p1.x0; Curves[1].p1.y = Curves[1].p1.y0; Curves[1].p1.z = Curves[1].p1.z0;
	Curves[1].p2.x0 = 2.0; Curves[1].p2.y0 = -4.0; Curves[1].p2.z0 = 0.0;
	Curves[1].p2.x = Curves[1].p2.x0; Curves[1].p2.y = Curves[1].p2.y0; Curves[1].p2.z = Curves[1].p2.z0;
	Curves[1].p3.x0 = 3.0; Curves[1].p3.y0 = 3.0; Curves[1].p3.z0 = -1.0;
	Curves[1].p3.x = Curves[1].p3.x0; Curves[1].p3.y = Curves[1].p3.y0; Curves[1].p3.z = Curves[1].p3.z0;

	Curves[2].r = 0.4; Curves[2].g = 0.2; Curves[2].b = 0.05;
	Curves[2].p0.x0 = -1.5; Curves[2].p0.y0 = -0.25; Curves[2].p0.z0 = 0.0;
	Curves[2].p0.x = Curves[2].p0.x0; Curves[2].p0.y = Curves[2].p0.y0; Curves[2].p0.z = Curves[2].p0.z0;
	Curves[2].p1.x0 = -0.5; Curves[2].p1.y0 = -0.7; Curves[2].p1.z0 = 0.0;
	Curves[2].p1.x = Curves[2].p1.x0; Curves[2].p1.y = Curves[2].p1.y0; Curves[2].p1.z = Curves[2].p1.z0;
	Curves[2].p2.x0 = 0.5; Curves[2].p2.y0 = 0.3; Curves[2].p2.z0 = 0.0;
	Curves[2].p2.x = Curves[2].p2.x0; Curves[2].p2.y = Curves[2].p2.y0; Curves[2].p2.z = Curves[2].p2.z0;
	Curves[2].p3.x0 = 1.5; Curves[2].p3.y0 = -0.3; Curves[2].p3.z0 = 0.0;
	Curves[2].p3.x = Curves[2].p3.x0; Curves[2].p3.y = Curves[2].p3.y0; Curves[2].p3.z = Curves[2].p3.z0;


	Curves[3].r = 0.4; Curves[3].g = 0.2; Curves[3].b = 0.05;
	Curves[3].p0.x0 = -1.5; Curves[3].p0.y0 = -0.3; Curves[3].p0.z0 = 0.0;
	Curves[3].p0.x = Curves[3].p0.x0; Curves[3].p0.y = Curves[3].p0.y0; Curves[3].p0.z = Curves[3].p0.z0;
	Curves[3].p1.x0 = -0.5; Curves[3].p1.y0 = 0.3; Curves[3].p1.z0 = 0.0;
	Curves[3].p1.x = Curves[3].p1.x0; Curves[3].p1.y = Curves[3].p1.y0; Curves[3].p1.z = Curves[3].p1.z0;
	Curves[3].p2.x0 = 0.5; Curves[3].p2.y0 = -0.7; Curves[3].p2.z0 = 0.0;
	Curves[3].p2.x = Curves[3].p2.x0; Curves[3].p2.y = Curves[3].p2.y0; Curves[3].p2.z = Curves[3].p2.z0;
	Curves[3].p3.x0 = 1.5; Curves[3].p3.y0 = -0.3; Curves[3].p3.z0 = 0.0;
	Curves[3].p3.x = Curves[3].p3.x0; Curves[3].p3.y = Curves[3].p3.y0; Curves[3].p3.z = Curves[3].p3.z0;


	Curves[4].r = 0.5; Curves[4].g = 0.0; Curves[4].b = 0.25;
	Curves[4].p0.x0 = 0.0; Curves[4].p0.y0 = 0.45; Curves[4].p0.z0 = 0.0;
	Curves[4].p0.x = Curves[4].p0.x0; Curves[4].p0.y = Curves[4].p0.y0; Curves[4].p0.z = Curves[4].p0.z0;
	Curves[4].p1.x0 = 0.0; Curves[4].p1.y0 = 0.45; Curves[4].p1.z0 = 1.5;
	Curves[4].p1.x = Curves[4].p1.x0; Curves[4].p1.y = Curves[4].p1.y0; Curves[4].p1.z = Curves[4].p1.z0;
	Curves[4].p2.x0 = 0.0; Curves[4].p2.y0 = 0.8; Curves[4].p2.z0 = 1.5;
	Curves[4].p2.x = Curves[4].p2.x0; Curves[4].p2.y = Curves[4].p2.y0; Curves[4].p2.z = Curves[4].p2.z0;
	Curves[4].p3.x0 = 0.0; Curves[4].p3.y0 = 0.8; Curves[4].p3.z0 = 0.0;
	Curves[4].p3.x = Curves[4].p3.x0; Curves[4].p3.y = Curves[4].p3.y0; Curves[4].p3.z = Curves[4].p3.z0;


	Curves[5].r = 1.0; Curves[5].g = 1.0; Curves[5].b = 1.0;
	Curves[5].p0.x0 = -2.0; Curves[5].p0.y0 = 1.25; Curves[5].p0.z0 = -0.5;
	Curves[5].p0.x = Curves[5].p0.x0; Curves[5].p0.y = Curves[5].p0.y0; Curves[5].p0.z = Curves[5].p0.z0;
	Curves[5].p1.x0 = -1.5; Curves[5].p1.y0 = 1.5; Curves[5].p1.z0 = -0.42;
	Curves[5].p1.x = Curves[5].p1.x0; Curves[5].p1.y = Curves[5].p1.y0; Curves[5].p1.z = Curves[5].p1.z0;
	Curves[5].p2.x0 = -1.0; Curves[5].p2.y0 = 1.5; Curves[5].p2.z0 = -0.34;
	Curves[5].p2.x = Curves[5].p2.x0; Curves[5].p2.y = Curves[5].p2.y0; Curves[5].p2.z = Curves[5].p2.z0;
	Curves[5].p3.x0 = -0.5; Curves[5].p3.y0 = 1.25; Curves[5].p3.z0 = -0.25;
	Curves[5].p3.x = Curves[5].p3.x0; Curves[5].p3.y = Curves[5].p3.y0; Curves[5].p3.z = Curves[5].p3.z0;

	Curves[6].r = 1.0; Curves[6].g = 1.0; Curves[6].b = 1.0;
	Curves[6].p0.x0 = -2.0; Curves[6].p0.y0 = 1.25; Curves[6].p0.z0 = -0.5;
	Curves[6].p0.x = Curves[6].p0.x0; Curves[6].p0.y = Curves[6].p0.y0; Curves[6].p0.z = Curves[6].p0.z0;
	Curves[6].p1.x0 = -1.5; Curves[6].p1.y0 = 1.0; Curves[6].p1.z0 = -0.42;
	Curves[6].p1.x = Curves[6].p1.x0; Curves[6].p1.y = Curves[6].p1.y0; Curves[6].p1.z = Curves[6].p1.z0;
	Curves[6].p2.x0 = -1.0; Curves[6].p2.y0 = 1.0; Curves[6].p2.z0 = -0.34;
	Curves[6].p2.x = Curves[6].p2.x0; Curves[6].p2.y = Curves[6].p2.y0; Curves[6].p2.z = Curves[6].p2.z0;
	Curves[6].p3.x0 = -0.5; Curves[6].p3.y0 = 1.25; Curves[6].p3.z0 = -0.25;
	Curves[6].p3.x = Curves[6].p3.x0; Curves[6].p3.y = Curves[6].p3.y0; Curves[6].p3.z = Curves[6].p3.z0;
}





//******************************************************************************




// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList( )

void
InitLists()
{
	float dx = BOXSIZE / 2.f;
	float dy = BOXSIZE / 2.f;
	float dz = BOXSIZE / 2.f;
	glutSetWindow(MainWindow);

	// create the object:

	BoxList = glGenLists(1);
	glNewList(BoxList, GL_COMPILE);

	glBegin(GL_QUADS);

	glColor3f(0., 0., 1.);
	glNormal3f(0., 0., 1.);
	glVertex3f(-dx, -dy, dz);
	glVertex3f(dx, -dy, dz);
	glVertex3f(dx, dy, dz);
	glVertex3f(-dx, dy, dz);

	glNormal3f(0., 0., -1.);
	glTexCoord2f(0., 0.);
	glVertex3f(-dx, -dy, -dz);
	glTexCoord2f(0., 1.);
	glVertex3f(-dx, dy, -dz);
	glTexCoord2f(1., 1.);
	glVertex3f(dx, dy, -dz);
	glTexCoord2f(1., 0.);
	glVertex3f(dx, -dy, -dz);

	glColor3f(1., 0., 0.);
	glNormal3f(1., 0., 0.);
	glVertex3f(dx, -dy, dz);
	glVertex3f(dx, -dy, -dz);
	glVertex3f(dx, dy, -dz);
	glVertex3f(dx, dy, dz);

	glNormal3f(-1., 0., 0.);
	glVertex3f(-dx, -dy, dz);
	glVertex3f(-dx, dy, dz);
	glVertex3f(-dx, dy, -dz);
	glVertex3f(-dx, -dy, -dz);

	glColor3f(0., 1., 0.);
	glNormal3f(0., 1., 0.);
	glVertex3f(-dx, dy, dz);
	glVertex3f(dx, dy, dz);
	glVertex3f(dx, dy, -dz);
	glVertex3f(-dx, dy, -dz);

	glNormal3f(0., -1., 0.);
	glVertex3f(-dx, -dy, dz);
	glVertex3f(-dx, -dy, -dz);
	glVertex3f(dx, -dy, -dz);
	glVertex3f(dx, -dy, dz);

	glEnd();

	glEndList();


	// create the axes:

	AxesList = glGenLists(1);
	glNewList(AxesList, GL_COMPILE);
	glLineWidth(AXES_WIDTH);
	Axes(1.5);
	glLineWidth(1.);
	glEndList();
}


// the keyboard callback:

void
Keyboard(unsigned char c, int x, int y)
{
	if (DebugOn != 0)
		fprintf(stderr, "Keyboard: '%c' (0x%0x)\n", c, c);

	switch (c)
	{
	case 'o':
	case 'O':
		WhichProjection = ORTHO;
		break;

	case 'p':
	case 'P':
		WhichProjection = PERSP;
		break;

	case 'q':
	case 'Q':
	case ESCAPE:
		DoMainMenu(QUIT);	// will not return here
		break;				// happy compiler

	case 'f':
	case 'F':
		Frozen = !Frozen;
		if (Frozen)
			glutIdleFunc(NULL);
		else
			glutIdleFunc(Animate);
		break;

	default:
		fprintf(stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c);
	}

	// force a call to Display( ):

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


// called when the mouse button transitions down or up:

void
MouseButton(int button, int state, int x, int y)
{
	int b = 0;			// LEFT, MIDDLE, or RIGHT

	if (DebugOn != 0)
		fprintf(stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y);


	// get the proper button bit mask:

	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		b = LEFT;		break;

	case GLUT_MIDDLE_BUTTON:
		b = MIDDLE;		break;

	case GLUT_RIGHT_BUTTON:
		b = RIGHT;		break;

	case SCROLL_WHEEL_UP:
		Scale += SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
		// keep object from turning inside-out or disappearing:
		if (Scale < MINSCALE)
			Scale = MINSCALE;
		break;

	case SCROLL_WHEEL_DOWN:
		Scale -= SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
		// keep object from turning inside-out or disappearing:
		if (Scale < MINSCALE)
			Scale = MINSCALE;
		break;

	default:
		b = 0;
		fprintf(stderr, "Unknown mouse button: %d\n", button);
	}

	// button down sets the bit, up clears the bit:

	if (state == GLUT_DOWN)
	{
		Xmouse = x;
		Ymouse = y;
		ActiveButton |= b;		// set the proper bit
	}
	else
	{
		ActiveButton &= ~b;		// clear the proper bit
	}

	glutSetWindow(MainWindow);
	glutPostRedisplay();

}


// called when the mouse moves while a button is down:

void
MouseMotion(int x, int y)
{
	if (DebugOn != 0)
		fprintf(stderr, "MouseMotion: %d, %d\n", x, y);


	int dx = x - Xmouse;		// change in mouse coords
	int dy = y - Ymouse;

	if ((ActiveButton & LEFT) != 0)
	{
		Xrot += (ANGFACT * dy);
		Yrot += (ANGFACT * dx);
	}


	if ((ActiveButton & MIDDLE) != 0)
	{
		Scale += SCLFACT * (float)(dx - dy);

		// keep object from turning inside-out or disappearing:

		if (Scale < MINSCALE)
			Scale = MINSCALE;
	}

	Xmouse = x;			// new current position
	Ymouse = y;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


// reset the transformations and the colors:
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene

void
Reset()
{
	ActiveButton = 0;
	AxesOn = 1;
	DebugOn = 0;
	DepthBufferOn = 1;
	DepthFightingOn = 0;
	DepthCueOn = 0;
	Scale = 1.0;
	ShadowsOn = 0;
	WhichColor = WHITE;
	WhichProjection = PERSP;
	Xrot = Yrot = 0.;
}


// called when user resizes the window:

void
Resize(int width, int height)
{
	if (DebugOn != 0)
		fprintf(stderr, "ReSize: %d, %d\n", width, height);

	// don't really need to do anything since window size is
	// checked each time in Display( ):

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


// handle a change to the window's visibility:

void
Visibility(int state)
{
	if (DebugOn != 0)
		fprintf(stderr, "Visibility: %d\n", state);

	if (state == GLUT_VISIBLE)
	{
		glutSetWindow(MainWindow);
		glutPostRedisplay();
	}
	else
	{
		// could optimize by keeping track of the fact
		// that the window is not visible and avoid
		// animating or redrawing it ...
	}
}



///////////////////////////////////////   HANDY UTILITIES:  //////////////////////////


// the stroke characters 'X' 'Y' 'Z' :

static float xx[] = {
		0.f, 1.f, 0.f, 1.f
};

static float xy[] = {
		-.5f, .5f, .5f, -.5f
};

static int xorder[] = {
		1, 2, -3, 4
};

static float yx[] = {
		0.f, 0.f, -.5f, .5f
};

static float yy[] = {
		0.f, .6f, 1.f, 1.f
};

static int yorder[] = {
		1, 2, 3, -2, 4
};

static float zx[] = {
		1.f, 0.f, 1.f, 0.f, .25f, .75f
};

static float zy[] = {
		.5f, .5f, -.5f, -.5f, 0.f, 0.f
};

static int zorder[] = {
		1, 2, 3, 4, -5, 6
};

// fraction of the length to use as height of the characters:
const float LENFRAC = 0.10f;

// fraction of length to use as start location of the characters:
const float BASEFRAC = 1.10f;

//	Draw a set of 3D axes:
//	(length is the axis length in world coordinates)

void
Axes(float length)
{
	glBegin(GL_LINE_STRIP);
	glVertex3f(length, 0., 0.);
	glVertex3f(0., 0., 0.);
	glVertex3f(0., length, 0.);
	glEnd();
	glBegin(GL_LINE_STRIP);
	glVertex3f(0., 0., 0.);
	glVertex3f(0., 0., length);
	glEnd();

	float fact = LENFRAC * length;
	float base = BASEFRAC * length;

	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < 4; i++)
	{
		int j = xorder[i];
		if (j < 0)
		{

			glEnd();
			glBegin(GL_LINE_STRIP);
			j = -j;
		}
		j--;
		glVertex3f(base + fact * xx[j], fact * xy[j], 0.0);
	}
	glEnd();

	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < 5; i++)
	{
		int j = yorder[i];
		if (j < 0)
		{

			glEnd();
			glBegin(GL_LINE_STRIP);
			j = -j;
		}
		j--;
		glVertex3f(fact * yx[j], base + fact * yy[j], 0.0);
	}
	glEnd();

	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < 6; i++)
	{
		int j = zorder[i];
		if (j < 0)
		{

			glEnd();
			glBegin(GL_LINE_STRIP);
			j = -j;
		}
		j--;
		glVertex3f(0.0, fact * zy[j], base + fact * zx[j]);
	}
	glEnd();

}

struct bmfh
{
	short bfType;
	int bfSize;
	short bfReserved1;
	short bfReserved2;
	int bfOffBits;
} FileHeader;

struct bmih
{
	int biSize;
	int biWidth;
	int biHeight;
	short biPlanes;
	short biBitCount;
	int biCompression;
	int biSizeImage;
	int biXPelsPerMeter;
	int biYPelsPerMeter;
	int biClrUsed;
	int biClrImportant;
} InfoHeader;

const int birgb = { 0 };

// read a BMP file into a Texture:

unsigned char*
BmpToTexture(char* filename, int* width, int* height)
{
	FILE* fp = fopen(filename, "rb");
	if (fp == NULL)
	{
		fprintf(stderr, "Cannot open Bmp file '%s'\n", filename);
		return NULL;
	}

	FileHeader.bfType = ReadShort(fp);


	// if bfType is not 0x4d42, the file is not a bmp:

	if (FileHeader.bfType != 0x4d42)
	{
		fprintf(stderr, "File '%s' is the wrong type of file: 0x%0x\n", filename, FileHeader.bfType);
		fclose(fp);
		return NULL;
	}

	FileHeader.bfSize = ReadInt(fp);
	FileHeader.bfReserved1 = ReadShort(fp);
	FileHeader.bfReserved2 = ReadShort(fp);
	FileHeader.bfOffBits = ReadInt(fp);

	InfoHeader.biSize = ReadInt(fp);
	InfoHeader.biWidth = ReadInt(fp);
	InfoHeader.biHeight = ReadInt(fp);

	int nums = InfoHeader.biWidth;
	int numt = InfoHeader.biHeight;

	InfoHeader.biPlanes = ReadShort(fp);
	InfoHeader.biBitCount = ReadShort(fp);
	InfoHeader.biCompression = ReadInt(fp);
	InfoHeader.biSizeImage = ReadInt(fp);
	InfoHeader.biXPelsPerMeter = ReadInt(fp);
	InfoHeader.biYPelsPerMeter = ReadInt(fp);
	InfoHeader.biClrUsed = ReadInt(fp);
	InfoHeader.biClrImportant = ReadInt(fp);

	fprintf(stderr, "Image size in file '%s' is: %d x %d\n", filename, nums, numt);

	unsigned char* texture = new unsigned char[3 * nums * numt];
	if (texture == NULL)
	{
		fprintf(stderr, "Cannot allocate the texture array!\b");
		return NULL;
	}

	// extra padding bytes:

	int numextra = 4 * (((3 * InfoHeader.biWidth) + 3) / 4) - 3 * InfoHeader.biWidth;

	// we do not support compression:

	if (InfoHeader.biCompression != birgb)
	{
		fprintf(stderr, "Image file '%s' has the wrong type of image compression: %d\n", filename, InfoHeader.biCompression);
		fclose(fp);
		return NULL;
	}

	rewind(fp);
	fseek(fp, 14 + 40, SEEK_SET);

	if (InfoHeader.biBitCount == 24)
	{
		unsigned char* tp = texture;
		for (int t = 0; t < numt; t++)
		{
			for (int s = 0; s < nums; s++, tp += 3)
			{
				*(tp + 2) = fgetc(fp);		// b
				*(tp + 1) = fgetc(fp);		// g
				*(tp + 0) = fgetc(fp);		// r
			}

			for (int e = 0; e < numextra; e++)
			{
				fgetc(fp);
			}
		}
	}

	fclose(fp);

	*width = nums;
	*height = numt;
	return texture;
}

int
ReadInt(FILE* fp)
{
	unsigned char b3, b2, b1, b0;
	b0 = fgetc(fp);
	b1 = fgetc(fp);
	b2 = fgetc(fp);
	b3 = fgetc(fp);
	return (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
}

short
ReadShort(FILE* fp)
{
	unsigned char b1, b0;
	b0 = fgetc(fp);
	b1 = fgetc(fp);
	return (b1 << 8) | b0;
}


// function to convert HSV to RGB
// 0.  <=  s, v, r, g, b  <=  1.
// 0.  <= h  <=  360.
// when this returns, call:
//		glColor3fv( rgb );

void
HsvRgb(float hsv[3], float rgb[3])
{
	// guarantee valid input:

	float h = hsv[0] / 60.f;
	while (h >= 6.)	h -= 6.;
	while (h < 0.) 	h += 6.;

	float s = hsv[1];
	if (s < 0.)
		s = 0.;
	if (s > 1.)
		s = 1.;

	float v = hsv[2];
	if (v < 0.)
		v = 0.;
	if (v > 1.)
		v = 1.;

	// if sat==0, then is a gray:

	if (s == 0.0)
	{
		rgb[0] = rgb[1] = rgb[2] = v;
		return;
	}

	// get an rgb from the hue itself:

	float i = (float)floor(h);
	float f = h - i;
	float p = v * (1.f - s);
	float q = v * (1.f - s * f);
	float t = v * (1.f - (s * (1.f - f)));

	float r = 0., g = 0., b = 0.;			// red, green, blue
	switch ((int)i)
	{
	case 0:
		r = v;	g = t;	b = p;
		break;

	case 1:
		r = q;	g = v;	b = p;
		break;

	case 2:
		r = p;	g = v;	b = t;
		break;

	case 3:
		r = p;	g = q;	b = v;
		break;

	case 4:
		r = t;	g = p;	b = v;
		break;

	case 5:
		r = v;	g = p;	b = q;
		break;
	}


	rgb[0] = r;
	rgb[1] = g;
	rgb[2] = b;
}

void
Cross(float v1[3], float v2[3], float vout[3])
{
	float tmp[3];
	tmp[0] = v1[1] * v2[2] - v2[1] * v1[2];
	tmp[1] = v2[0] * v1[2] - v1[0] * v2[2];
	tmp[2] = v1[0] * v2[1] - v2[0] * v1[1];
	vout[0] = tmp[0];
	vout[1] = tmp[1];
	vout[2] = tmp[2];
}

float
Dot(float v1[3], float v2[3])
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

float
Unit(float vin[3], float vout[3])
{
	float dist = vin[0] * vin[0] + vin[1] * vin[1] + vin[2] * vin[2];
	if (dist > 0.0)
	{
		dist = sqrtf(dist);
		vout[0] = vin[0] / dist;
		vout[1] = vin[1] / dist;
		vout[2] = vin[2] / dist;
	}
	else
	{
		vout[0] = vin[0];
		vout[1] = vin[1];
		vout[2] = vin[2];
	}
	return dist;
}
