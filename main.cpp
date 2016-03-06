/*
==========================================================================
File:        ex2.c (skeleton)
Authors:     Toby Howard
==========================================================================
*/

/* The following ratios are not to scale: */
/* Moon orbit : planet orbit */
/* Orbit radius : body radius */
/* Sun radius : planet radius */

#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_BODIES 20
#define MAX_STARS 1000
#define TOP_VIEW 1
#define ECLIPTIC_VIEW 2
#define SHIP_VIEW 3
#define EARTH_VIEW 4
#define PI 3.14159
#define DEG_TO_RAD 0.017453293
#define ORBIT_POLY_SIDES 40
#define TIME_STEP 0.0005   /* days per frame */
#define TURN_ANGLE 4.0
#define RUN_SPEED  10

typedef struct {
  char    name[20];       /* name */
  GLfloat r, g, b;        /* colour */
  GLfloat orbital_radius; /* distance to parent body (km) */
  GLfloat orbital_tilt;   /* angle of orbit wrt ecliptic (deg) */
  GLfloat orbital_period; /* time taken to orbit (days) */
  GLfloat radius;         /* radius of body (km) */
  GLfloat axis_tilt;      /* tilt of axis wrt body's orbital plane (deg) */
  GLfloat rot_period;     /* body's period of rotation (days) */
  GLint   orbits_body;    /* identifier of parent body */
  GLfloat spin;           /* current spin value (deg) */
  GLfloat orbit;          /* current orbit value (deg) */
 } body;

typedef struct{
  GLfloat x, y, z; /* stat position (x,y,z)*/
} STAR;

body  bodies[MAX_BODIES];
STAR  stars[MAX_STARS];
int   numBodies, current_view, draw_labels, draw_orbits, draw_starfield, draw_axis;
GLfloat earth_x, earth_y, earth_z;
GLfloat eyex, eyey, eyez;
GLdouble lat,     lon;              /* View angles (degrees)    */
GLdouble centerx, centery, centerz;

/*****************************/

float myRand (void)
{
  /* return a random float in the range [0,1] */

  return (float) rand() / RAND_MAX;
}

/*****************************/

void drawStarfield (void)
{
  int i;
  glColor3f(1.0, 1.0, 1.0);
  glBegin(GL_POINTS);
   for(i=0; i<1000; i++)
     glVertex3f(stars[i].x, stars[i].y, stars[i].z);
 glEnd();
}

/*****************************/

void drawAxis (void)
{
 glLineWidth(5.0);
 glBegin(GL_LINES);
  glColor3f(1.0, 0.0, 0.0); // Red
  glVertex3f(0.0, 0.0, 0.0);
  glVertex3f(300000000.0, 0.0, 0.0);

  glColor3f(0.0, 1.0, 0.0); // Green
  glVertex3f(0.0, 0.0, 0.0);
  glVertex3f(0.0, 300000000.0, 0.0);

  glColor3f(0.0, 0.0, 1.0); // Blue
  glVertex3f(0.0, 0.0, 0.0);
  glVertex3f(0.0, 0.0, 300000000.0);
 glEnd();
 glLineWidth(1.0);
}

/*****************************/

void readSystem(void)
{
  /* reads in the description of the solar system */

  FILE *f;
  int i;

  f= fopen("C:\\Users\\psilv_000\\Documents\\cg\\ex2\\sys.txt", "r");
  if (f == NULL) {
     printf("ex2.c: Couldn't open the datafile 'sys'\n");
     printf("To get this file, use the following command:\n");
     printf("  cp /opt/info/courses/COMP27112/ex2/sys .\n");
     exit(0);
  }
  fscanf(f, "%d", &numBodies);
  for (i= 0; i < numBodies; i++)
  {
    fscanf(f, "%s %f %f %f %f %f %f %f %f %f %d",
      bodies[i].name,
      &bodies[i].r, &bodies[i].g, &bodies[i].b,
      &bodies[i].orbital_radius,
      &bodies[i].orbital_tilt,
      &bodies[i].orbital_period,
      &bodies[i].radius,
      &bodies[i].axis_tilt,
      &bodies[i].rot_period,
      &bodies[i].orbits_body);

    /* Initialise the body's state */
    bodies[i].spin= 0.0;
    bodies[i].orbit= myRand() * 360.0; /* Start each body's orbit at a
                                          random angle */
    bodies[i].radius*= 1000.0; /* Magnify the radii to make them visible */
  }
  fclose(f);
}

/*****************************/

void drawString (void *font, float x, float y, char *str)
{ /* Displays the string "str" at (x,y,0), using font "font" */

  char *c;
  glRasterPos3f(x,y,0.0);
  for(c=str; *c; c++)
    glutBitmapWidth(font, (int) *c);

}

/*****************************/

void calculate_lookpoint(){

  float dir_x = cos(lat*DEG_TO_RAD)*sin(lon*DEG_TO_RAD);
  float dir_y = sin(lat*DEG_TO_RAD);
  float dir_z = cos(lat*DEG_TO_RAD)*cos(lon*DEG_TO_RAD);

  centerx = eyex+(dir_x*100000000);
  centery = eyey+(dir_y*100000000);
  centerz = eyez+(dir_z*100000000);
}

/*****************************/

void setView (void) {
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  switch (current_view) {
  case TOP_VIEW:
    gluLookAt(3000000.0, 600000000.0, 3000000.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    break;
  case ECLIPTIC_VIEW:
    gluLookAt(0.0, 120000000.0, 450000000.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    break;
  case SHIP_VIEW:
    calculate_lookpoint(); /* Compute the centre of interest   */
    gluLookAt(eyex, eyey, eyez, centerx, centery, centerz, 0.0, 1.0, 0.0);
    break;
  case EARTH_VIEW:
    gluLookAt(earth_x,earth_y+bodies[3].radius+750000,earth_z, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    break;
  }
}

/*****************************/

void menu (int menuentry) {
  switch (menuentry) {
  case 1: current_view= TOP_VIEW;
          break;
  case 2: current_view= ECLIPTIC_VIEW;
          break;
  case 3: current_view= SHIP_VIEW;
          break;
  case 4: current_view= EARTH_VIEW;
          break;
  case 5: draw_labels= !draw_labels;
          break;
  case 6: draw_orbits= !draw_orbits;
          break;
  case 7: draw_starfield= !draw_starfield;
          break;
  case 8: exit(0);
  }
}

/*****************************/

void init(void)
{
  /* Define background colour */
  glClearColor(0.0, 0.0, 0.0, 0.0);

  glutCreateMenu (menu);
  glutAddMenuEntry ("Top view", 1);
  glutAddMenuEntry ("Ecliptic view", 2);
  glutAddMenuEntry ("Spaceship view", 3);
  glutAddMenuEntry ("Earth view", 4);
  glutAddMenuEntry ("", 999);
  glutAddMenuEntry ("Toggle labels", 5);
  glutAddMenuEntry ("Toggle orbits", 6);
  glutAddMenuEntry ("Toggle starfield", 7);
  glutAddMenuEntry ("", 999);
  glutAddMenuEntry ("Quit", 8);
  glutAttachMenu (GLUT_RIGHT_BUTTON);

  current_view = TOP_VIEW;
  draw_labels = 1;
  draw_orbits = 1;
  draw_starfield = 1;
  draw_starfield = 1;
  draw_axis = 1;

  eyex = 40000000.0;
  eyey = 90000000;
  eyez = -350000000.0;

  centerx = 0.0;
  centery = 0.0;
  centerz = 0.0;

  lat= 0.0;   /* Look horizontally ...  */
  lon= 0.0;   /* ... along the +Z axis  */

  int i;
  for(i=0; i<1000; i++){
    stars[i].x = 600000000*myRand()-300000000;
    stars[i].y = 600000000*myRand()-300000000;
    stars[i].z = 600000000*myRand()-300000000;
  }
}

/*****************************/

void animate(void)
{
    int i;
    for (i= 0; i < numBodies; i++)  {
        bodies[i].spin += 360.0 * TIME_STEP / bodies[i].rot_period;
        bodies[i].orbit += 360.0 * TIME_STEP / bodies[i].orbital_period;
        glutPostRedisplay();
    }
}

/*****************************/

void drawOrbit (int n)
{ /* Draws a polygon to approximate the circular
     orbit of body "n" */
    int j;
    float theta,x,y,orbx,orby;

    glColor3f(bodies[n].r, bodies[n].g, bodies[n].b);

    if(bodies[n].orbits_body != 0){
        orbx =  bodies[bodies[n].orbits_body].orbital_radius*cos(bodies[bodies[n].orbits_body].orbit);
        orby =  bodies[bodies[n].orbits_body].orbital_radius*sin(bodies[bodies[n].orbits_body].orbit);
        glTranslatef(orbx,0.0,orby);
    }
    glBegin(GL_LINE_LOOP);
    for(j=0; j< ORBIT_POLY_SIDES; j++){
        theta = 2.0*3.1415926f*j/ORBIT_POLY_SIDES; //get the current angle
        x =  bodies[n].orbital_radius*cos(theta);
        y =  bodies[n].orbital_radius*sin(theta);
        glVertex3f (x, 0.0, y);
    }
    glEnd();
    if(bodies[n].orbits_body != 0){
        glTranslatef(-orbx,0.0,-orby);
    }
}

/*****************************/

void drawLabel(int n)
{ /* Draws the name of body "n" */
    drawString(GLUT_BITMAP_TIMES_ROMAN_24, 0.0,0.0,bodies[n].name);
}

/*****************************/

void drawBody(int n)
{
 /* Draws body "n" */
    float x,y,orbx,orby;

    glColor3f(bodies[n].r, bodies[n].g, bodies[n].b);
    glMatrixMode(GL_MODELVIEW);

    if(bodies[n].orbits_body == 0){
        x =  bodies[n].orbital_radius*cos(bodies[n].orbit);
        y =  bodies[n].orbital_radius*sin(bodies[n].orbit);
        if(n == 3){
            earth_x = x;
            earth_z = y;
            earth_y = 0;
        }
        glRotatef(90,1.0,0.0,0.0); //Tvert
        glTranslatef(x,y,0.0); //Torb
        glRotatef(bodies[n].axis_tilt,0.0,1.0,0.0); //Tatilt
        glRotatef(bodies[n].spin,0.0,0.0,1.0); //Tspin
        glRotatef(bodies[n].orbital_tilt,1.0,0.0,0.0); //Totilt
    }else{
        x =  bodies[bodies[n].orbits_body].orbital_radius*cos(bodies[bodies[n].orbits_body].orbit);
        y =  bodies[bodies[n].orbits_body].orbital_radius*sin(bodies[bodies[n].orbits_body].orbit);

        glRotatef(90,1.0,0.0,0.0); //Tvert
        glTranslatef(x,y,0.0); //Torb
        glRotatef(bodies[n].axis_tilt,0.0,1.0,0.0); //Tatilt
        glRotatef(bodies[n].spin,0.0,0.0,1.0); //Tspin
        glRotatef(bodies[n].orbital_tilt,1.0,0.0,0.0); //Totilt

        orbx =  bodies[n].orbital_radius*cos(bodies[n].orbit);
        orby =  bodies[n].orbital_radius*sin(bodies[n].orbit);
        glTranslatef(orbx,orby,0.0); //TorbM
    }

    glutWireSphere(bodies[n].radius, 10, 10);
    drawString(GLUT_BITMAP_TIMES_ROMAN_24,x,y,bodies[n].name);

    // Transform matrix back
    if(bodies[n].orbits_body == 0){
        glRotatef(-bodies[n].orbital_tilt,0.0,1.0,0.0); //Totilt
        glRotatef(-bodies[n].spin,0.0,0.0,1.0); //Tspin
        glRotatef(-bodies[n].axis_tilt,0.0,1.0,0.0); //Tatilt
        glTranslatef(-x,-y,0.0); //Torb
        glRotatef(-90,1.0,0.0,0.0); //Tvert
    }else{
        glTranslatef(-orbx,-orby,0.0); //TorbM
        glRotatef(-bodies[n].orbital_tilt,0.0,1.0,0.0); //Totilt
        glRotatef(-bodies[n].spin,0.0,0.0,1.0); //Tspin
        glRotatef(-bodies[n].axis_tilt,0.0,1.0,0.0); //Tatilt
        glTranslatef(-x,y,0.0); //Torb
        glRotatef(-90,1.0,0.0,0.0); //Tvert
    }

}

/*****************************/

void display(void)
{
  int i;

  glClear(GL_COLOR_BUFFER_BIT);

  /* set the camera */
  setView();

  if (draw_starfield)
    drawStarfield();
  if (draw_axis)
    drawAxis();


  for (i= 0; i < numBodies; i++)
  {
    glPushMatrix();
      if(draw_orbits) drawOrbit(i);
      if(draw_labels) drawLabel(i);
      drawBody (i);
    glPopMatrix();
  }


  glutSwapBuffers();
}

/*****************************/

void reshape(int w, int h)
{
  glViewport(0, 0, (GLsizei) w, (GLsizei) h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective (48.0, (GLfloat) w/(GLfloat) h, 10000.0, 800000000.0);
}

/*****************************/

void cursor_keys(int key, int x, int y) {
float dir_x, dir_y, dir_z;
dir_x = dir_y = dir_z = 0;
  switch (key) {
    case GLUT_KEY_LEFT:
	lon = lon+TURN_ANGLE;
	break;
    case GLUT_KEY_RIGHT:
	lon = lon-TURN_ANGLE;
	break;
    case GLUT_KEY_PAGE_UP:
	if(lat+TURN_ANGLE >= 90)
	  lat = lat;
	else
	lat = lat+TURN_ANGLE;
	break;
    case GLUT_KEY_PAGE_DOWN:
	if(lat-TURN_ANGLE <= -90)
	  lat = lat;
	else
	lat = lat-TURN_ANGLE;
	break;
    case GLUT_KEY_HOME:
	lat = 0;
	break;
    case GLUT_KEY_UP:
	dir_x = sin(lon*DEG_TO_RAD)*RUN_SPEED*1000000;
	dir_y = sin(lat*DEG_TO_RAD)*RUN_SPEED*10000;
	dir_z = cos(lon*DEG_TO_RAD)*RUN_SPEED*1000000;
	eyex = eyex+dir_x;
	eyey = eyey+dir_y;
	eyez = eyez+dir_z;
	break;
    case GLUT_KEY_DOWN:
	dir_x = sin(lon*DEG_TO_RAD)*RUN_SPEED*1000000;
	dir_y = sin(lat*DEG_TO_RAD)*RUN_SPEED*10000;
	dir_z = cos(lon*DEG_TO_RAD)*RUN_SPEED*1000000;
	eyex = eyex-dir_x;
	eyey = eyey-dir_y;
	eyez = eyez-dir_z;
	break;


  }
} // cursor_keys()

/*****************************/

void keyboard(unsigned char key, int x, int y)
{
float dir_x, dir_y, dir_z;
dir_x = dir_y = dir_z = 0;

  switch (key)
  {
    case 27:  /* Escape key */
      exit(0);
      break;
    case 97: // a
      draw_axis = !draw_axis;
      break;
  }
}

/*****************************/

int main(int argc, char** argv)
{
  glutInit (&argc, argv);
  glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB);
  glutCreateWindow ("COMP27112 Exercise 2");
  glutFullScreen();
  init ();
  glutDisplayFunc (display);
  glutReshapeFunc (reshape);
  glutKeyboardFunc (keyboard);
  glutSpecialFunc (cursor_keys);
  glutIdleFunc (animate);
  readSystem();
  glutMainLoop ();
  return 0;
}
/* end of ex2.c */
