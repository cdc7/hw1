//modified by: Christian Chavez
//date:
//purpose:
//
//cs3350 Spring 2017 HW-1
//author: Gordon Griesel
//date: 2014 to present
//This program demonstrates the use of OpenGL and XWindows
//
//Assignment is to modify this program.
//You will follow along with your instructor.
//
//Elements to be learned in this lab...
//
//. general animation framework
//. animation loop
//. object definition and movement
//. collision detection
//. mouse/keyboard interaction
//. object constructor
//. coding style
//. defined constants
//. use of static variables
//. dynamic memory allocation
//. simple opengl components
//. git
//
//elements we will add to program...
//. Game constructor
//. multiple particles
//. gravity
//. collision detection
//. more objects
//
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include "fonts.h"

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

#define MAX_PARTICLES 8000
#define GRAVITY 0.10
#define rnd() (float)rand() / (float)RAND_MAX

//X Windows variables
Display *dpy;
Window win;
GLXContext glc;
char str[5][15] = {
	{"Requirements"},
	{"Design"},
	{"Coding"},
	{"Testing"},
	{"Matinence"}
};

//Structures

struct Vec {
	float x, y, z;
};

struct Shape {
	float width, height;
	float radius;
	Vec center;
};

struct Particle {
	Shape s;
	Vec velocity;
};

struct Game {
	Shape circle;
	Shape box[5];
	Particle particle[MAX_PARTICLES];
	int n;
    int bubbler;
    int mouse[2];
	Game() { n=0; bubbler=0; }
};

//Function prototypes
void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
void check_mouse(XEvent *e, Game *game);
int check_keys(XEvent *e, Game *game);
void movement(Game *game);
void render(Game *game);


int main(void)
{
	int done=0;
	srand(time(NULL));
	initXWindows();
	init_opengl();
	//declare game object
	Game game;
	game.n=0;

	//declare a box shape
	for(int i=0; i<5; i++){
		game.box[i].width = 100;
		game.box[i].height = 12;
		game.box[i].center.x = 120 + (i*80);
		game.box[i].center.y = 500 - (i*70);
	}

	game.circle.center.x = 750;
	game.circle.center.y = -50;
	game.circle.radius = 200;

	//start animation
	while (!done) {
		while (XPending(dpy)) {
			XEvent e;
			XNextEvent(dpy, &e);
			check_mouse(&e, &game);
			done = check_keys(&e, &game);
		}
		movement(&game);
		render(&game);
		glXSwapBuffers(dpy, win);
	}
	cleanupXWindows();
	return 0;
}

void set_title(void)
{
	//Set the window title bar.
	XMapWindow(dpy, win);
	XStoreName(dpy, win, "335 Lab1   LMB for particle");
}

void cleanupXWindows(void)
{
	//do not change
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
}

void initXWindows(void)
{
	//do not change
	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	int w=WINDOW_WIDTH, h=WINDOW_HEIGHT;
	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		std::cout << "\n\tcannot connect to X server\n" << std::endl;
		exit(EXIT_FAILURE);
	}
	Window root = DefaultRootWindow(dpy);
	XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
	if (vi == NULL) {
		std::cout << "\n\tno appropriate visual found\n" << std::endl;
		exit(EXIT_FAILURE);
	} 
	Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	XSetWindowAttributes swa;
	swa.colormap = cmap;
	swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
		ButtonPress | ButtonReleaseMask | PointerMotionMask |
		StructureNotifyMask | SubstructureNotifyMask;
	win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
		InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
	set_title();
	glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, win, glc);
}

void init_opengl(void)
{
	//OpenGL initialization
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	//Initialize matrices
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	//Set 2D mode (no perspective)
	glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
	//Set the screen background color
	glClearColor(0.1, 0.1, 0.1, 1.0);
	//Set text
	glEnable(GL_TEXTURE_2D);
	initialize_fonts();
}

void makeParticle(Game *game, int x, int y)
{
	if (game->n >= MAX_PARTICLES)
		return;
	//position of particle
	Particle *p = &game->particle[game->n];
	p->s.center.x = x;
	p->s.center.y = y;
	p->velocity.y =  rnd() * 0.5 - .25;
	p->velocity.x =  rnd() * 0.5 - .25;
	game->n++;
}

void check_mouse(XEvent *e, Game *game)
{
	static int savex = 0;
	static int savey = 0;
	//static int n = 0;

	if (e->type == ButtonRelease) {
		return;
	}
	if (e->type == ButtonPress) {
		if (e->xbutton.button==1) {
			//Left button was pressed
			int y = WINDOW_HEIGHT - e->xbutton.y;
			for (int  i=0; i<10; i++){
			makeParticle(game, e->xbutton.x, y);
			}
			return;
		}
		if (e->xbutton.button==3) {
			//Right button was pressed
			return;
		}
	}
	//Did the mouse move?
	if (savex != e->xbutton.x || savey != e->xbutton.y) {
		savex = e->xbutton.x;
		savey = e->xbutton.y;
        int y = WINDOW_HEIGHT - e->xbutton.y;
        if (game-> bubbler == 0) {
            game->mouse[0] = savex;
            game->mouse[1] = y;
        }
		for (int i =0; i<5; i++) {
		    makeParticle(game, e->xbutton.x, y);
		}
		//if (++n < 10)
		//	return;
	}
}

int check_keys(XEvent *e, Game *game)
{
	//Was there input from the keyboard?
	if (e->type == KeyPress) {
		int key = XLookupKeysym(&e->xkey, 0);
        if (key == XK_b) {
			game->bubbler ^= 1;
		}
		if (key == XK_Escape) {
			return 1;
		}
		//You may check other keys here.



	}
	return 0;
}

void movement(Game *game)
{
	Particle *p;

	if (game->n <= 0)
		return;

    if(game->bubbler != 0) {
        //the bubbler is on!
		for(int i=0; i<50;i++){
        	makeParticle(game, game->mouse[0], game->mouse[1]);
		}
    }

	for(int i=0; i<game->n; i++){
	    p = &game->particle[i];
		p->velocity.y -= GRAVITY;
		p->s.center.x += p->velocity.x;
		p->s.center.y += p->velocity.y;
	


        //check for collision with shapes...
        Shape *s;
		for(int j=0; j<5; j++){
			s = &game->box[j];
			if (p->s.center.y < s->center.y + s->height &&
				p->s.center.x >= s->center.x - s->width &&
				p->s.center.x <= s->center.x + s->width) {
				p->s.center.y = s->center.y + s->height;
				p->velocity.y = -p->velocity.y * 0.3f;
				p->velocity.x += 0.01f;
			}
		}
        
        
        
        //check for off-screen
        if (p->s.center.y < 0.0) {
            game->particle[i] = game->particle[--game->n];
        }
    }
}

void render(Game *game)
{
	float w, h;
	glClear(GL_COLOR_BUFFER_BIT);
	//Draw shapes...

	//draw box
	Shape *s;
	for(int i=0; i<5; i++){
		glColor3ub(104, 204, 144);
		s = &game->box[i];
		glPushMatrix();
		glTranslatef(s->center.x, s->center.y, s->center.z);
		w = s->width;
		h = s->height;
		glBegin(GL_QUADS);
			glVertex2i(-w,-h);
			glVertex2i(-w, h);
			glVertex2i( w, h);
			glVertex2i( w,-h);
			glEnd();
			glPopMatrix();
		Rect r;
		r.bot = s->center.y - 10;
		r.left = s->center.x;
		r.center = s->center.y;
		unsigned int color = 0xed5e5e;
		char temp[15];
		for(int k=0; k<15; k++){
			temp[k] = str[i][k];
		}
		ggprint13(&r, 20, color, "%s", temp);
	}

	Shape *c;
	c = &game->circle;

	int x = c->center.x;
	int y = c->center.y;
	int radius = c->radius;
	glPushMatrix();
	glColor3ub(104, 204, 144);
	double twicePi = 2.0 * 3.1416;
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(x,y);
	for(int i=0; i<100; i++){
		glVertex2f(
				(x + (radius *cos(i * twicePi / 100))),
				(y + (radius *sin(i * twicePi / 100)))
				);
				
	}	
	glEnd();

	//draw all particles here
	for (int i=0; i<game->n; i++){
		glPushMatrix();
		glColor3ub(rand()%130 + 101, rand()%80 + 127, rand()%30 + 220);
		Vec *c = &game->particle[i].s.center;
		w = 2;
		h = 2;
		glBegin(GL_QUADS);
			glVertex2i(c->x-w, c->y-h);
			glVertex2i(c->x-w, c->y+h);
			glVertex2i(c->x+w, c->y+h);
			glVertex2i(c->x+w, c->y-h);
		glEnd();
		glPopMatrix();
	}
}



