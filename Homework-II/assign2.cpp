/*
  CSCI 420 Computer Graphics
  Assignment 2: Roller Coaster
  Siddharth Narula
*/
#include <iostream>
#include <cstring>
#include <string>
#include <sstream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <GLUT/glut.h>
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>
#include <OpenGL/gl.h>
#include "pic.h"
#include <fstream>
#include <cstring>
#include <sstream>
#include "math.h"
#include <time.h>
using namespace std;

Pic * g_pHeightData=NULL;
int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "CSCI 420 homework II Siddharth Narula";
struct points 
{
   double x;
   double y;
   double z;
};

/* spline struct which contains how many control points, and an array of control points */
struct spline 
{
   int numControlpoints;
   struct points *points;
};
//Features for Controling the ride
typedef enum { ROTATE, TRANSLATE, SCALE } CONTROLSTATE;
CONTROLSTATE g_ControlState = ROTATE;
typedef enum {NIGHT,DAY} D_N;
D_N changetheme = DAY;
bool ride=0;
bool displayrail=0;

// Initilization of the mouse and the keyboard parameters.
int g_vMousePos[2] = {0, 0};
int g_iLeftMouseButton = 0;    /* 1 if pressed, 0 if not */
int g_iMiddleMouseButton = 0;
int g_iRightMouseButton = 0;
// Control states for mouse control


/* Rotate Translate and Scale Funtions */
float g_vLandRotate[3] = {0.0, 0.0, 0.0};
float g_vLandTranslate[3] = {0.0, 0.0, 0.0};
float g_vLandScale[3] = {1.0, 1.0, 1.0};

/* Gluint objects for textrues*/
GLuint ground_tex;
GLuint sky_tex;
GLuint track_tex;
GLuint rod_tex;
GLuint night_sky_tex;

/* Images needed for textures*/
Pic *background_image;
Pic *sky_image;
Pic *tracks_image;
Pic *rod_texture;
Pic *night_sky_image;

/* various lighting effects for the tracks*/
GLfloat light_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_position[] = { 85.0, 85.0, 30.0, 1.0 };

/* Shadings for the tracks and background */
GLfloat mat_ambient[] = { 0.3, 0.3, 0.3, 1.0 };
GLfloat mat_diffuse[] = { 0.3, 0.3, 0.3, 1.0 };
GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat low_shininess[] = { 2.0 };

/* the spline array */
struct spline *g_Splines;

/* total number of splines */
int g_iNumOfSplines;

/* Set up the field of view. */
const float FOV = 60.0; 

/* Itnitial Camera points for GLU LOOK AT */
points eye={0.0,0.0,5.0};
points focus={0.0,0.0,0.0};
points up_vector = {0.0,1.0,0.0};
int count_points=0;

/*  Display List */
GLuint displayListSpline;

/* Creating points list of tangents, normals and binormals. defining s and Catmull-Rom spline matrix */
spline points_list;
spline tangent_list;
spline normal_list;
spline binormal_list;
int pointsCount = 0;
double s = 0.5;
double catmull_mat[4][4] = {{ -s, (2 - s), (s - 2), s }, { (2 * s), (s - 3), (3 - (2 * s)), -s },{ -s, 0, s, 0 },{ 0, 1, 0, 0 }};

//HELPER FUNCTION
int loadSplines(char *argv) 
{
  char *cName = (char *)malloc(128 * sizeof(char));
  FILE *fileList;
  FILE *fileSpline;
  int iType, i = 0, j, iLength;


  /* load the track file */
  fileList = fopen(argv, "r");
  
  if (fileList == NULL) {
    printf ("can't open file\n");
    exit(1);
  }
  
  /* stores the number of splines in a global variable */
  fscanf(fileList, "%d", &g_iNumOfSplines);

  g_Splines = (struct spline *)malloc(g_iNumOfSplines * sizeof(struct spline));

  /* reads through the spline files */
  for (j = 0; j < g_iNumOfSplines; j++) {
    i = 0;
    fscanf(fileList, "%s", cName);
    fileSpline = fopen(cName, "r");

    if (fileSpline == NULL) {
      printf ("can't open file\n");
      exit(1);
    }

    /* gets length for spline file */
    fscanf(fileSpline, "%d %d", &iLength, &iType);

    /* allocate memory for all the points */
    g_Splines[j].points = (struct points *)malloc(iLength * sizeof(struct points));
    g_Splines[j].numControlpoints = iLength;

    /* saves the data to the struct */
    while (fscanf(fileSpline, "%lf %lf %lf", 
	   &g_Splines[j].points[i].x, 
	   &g_Splines[j].points[i].y, 
	   &g_Splines[j].points[i].z) != EOF) {
      i++;
    }
  }

  free(cName);

  return 0;
}

/* Calculation of tangent_list*/ 
void calculatetangent_list()
{
 	tangent_list.numControlpoints=(g_Splines[0].numControlpoints*20)-40;
 	tangent_list.points= new points[tangent_list.numControlpoints];
 	count_points=tangent_list.numControlpoints;

	// Constraint matrix = basis matrix * control matrix
	double constraint[4][3] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	int numpoints = g_Splines->numControlpoints;
	pointsCount=0;
	// Removing 3 since taking 4 points together
	for (int point = 0; point < numpoints - 3; point++) 
	{
		// Create control matrix
		double control_mat[4][3] =
		{
			{ g_Splines[0].points[point].x , g_Splines[0].points[point].y, g_Splines[0].points[point].z },
			{ g_Splines[0].points[point + 1].x, g_Splines[0].points[point + 1].y, g_Splines[0].points[point+1].z },
			{ g_Splines[0].points[point + 2].x, g_Splines[0].points[point + 2].y, g_Splines[0].points[point+2].z },
			{ g_Splines[0].points[point + 3].x, g_Splines[0].points[point+ 3].y, g_Splines[0].points[point+3].z }
		};
		
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 3; j++)
			 {
				constraint[i][j] = 0;
			 }
		}
		for (int i = 0; i < 4; i++) 
		{
			for (int j = 0; j < 3; j++) 
			{
				for (int k = 0; k < 4; k++)
				 {
					constraint[i][j] += catmull_mat[i][k] * control_mat[k][j]; //Obtaining constraint Matrix
				 }
			}
		}
		
		double x = 0, y = 0, z = 0;
		//Defining u and generating final result 3u2 2u 1 0
		for (double u = 0.05; u <= 1; u += 0.05) 
		{ 
			x = (3*pow(u,2)*constraint[0][0]) + (2*u*constraint[1][0]) + (1 * constraint[2][0]);
			y = (3*pow(u,2)*constraint[0][1]) + (2*u*constraint[1][1]) + (1 * constraint[2][1]);
			z = (3*pow(u,2)*constraint[0][2]) + (2*u*constraint[1][2]) + (1 * constraint[2][2]);

      double mag = sqrt(pow(x,2)+pow(y,2)+pow(z,2));

			
			if(mag==0)
			{
				tangent_list.points[pointsCount].x =0.0;
				tangent_list.points[pointsCount].y =0.0;
				tangent_list.points[pointsCount].z =0.0;

			}
			else{
			tangent_list.points[pointsCount].x = x/(1.0*mag);
			tangent_list.points[pointsCount].y = y/(1.0*mag);
			tangent_list.points[pointsCount].z = z/(1.0*mag);
			}
			pointsCount++;	
		}
		}
	pointsCount = 0;

	
}
void normal_listBinormal_list()
{

  	normal_list.numControlpoints = tangent_list.numControlpoints;
	normal_list.points = new points[normal_list.numControlpoints];


	// Taking a arbitrary points(0,1,0) l
  double normal_mag = sqrt(pow((0.0*(tangent_list.points[0].z-tangent_list.points[0].y)),2)+pow((-1.0*(tangent_list.points[0].z-tangent_list.points[0].x)),2)+pow((0.0*(tangent_list.points[0].y-tangent_list.points[0].x)),2));

	if(normal_mag==0)
	{
	normal_list.points[0].x=0.0;
	normal_list.points[0].y=0.0;
	normal_list.points[0].z=0.0;	
	}
	else
	{
		//Normalised the points
	normal_list.points[0].x=0.0*(tangent_list.points[0].z-tangent_list.points[0].y)/(normal_mag);
	normal_list.points[0].y=-1.0*(tangent_list.points[0].z-tangent_list.points[0].x)/(normal_mag);
	normal_list.points[0].z=0.0*(tangent_list.points[0].y-tangent_list.points[0].x)/(normal_mag);

	}
	
  binormal_list.numControlpoints=tangent_list.numControlpoints;
	binormal_list.points=new points[binormal_list.numControlpoints];
  //Defining Binomial List
	
	double x =(tangent_list.points[0].y * normal_list.points[0].z)-(tangent_list.points[0].z * normal_list.points[0].y);
	double y =(tangent_list.points[0].z * normal_list.points[0].x)-(tangent_list.points[0].x * normal_list.points[0].z);
	double z =(tangent_list.points[0].x * normal_list.points[0].y)-(tangent_list.points[0].y * normal_list.points[0].x);
	double binorm_mag = sqrt(pow(x,2)+pow(y,2)+pow(z,2));
	if(binorm_mag==0)
	{
	binormal_list.points[0].x=0.0;
	binormal_list.points[0].y=0.0;
	binormal_list.points[0].z=0.0;	
	}
	else
	{
	binormal_list.points[0].x= -1.0*x/(1.0*binorm_mag) ;
	binormal_list.points[0].y= -1.0*y/(1.0*binorm_mag);
	binormal_list.points[0].z= -1.0*z/(1.0*binorm_mag);
	}
  //Calculating Binormals
	for (int i = 1; i < points_list.numControlpoints; i++) {

	
		double norm_x = (binormal_list.points[i - 1].y * tangent_list.points[i].z) - (binormal_list.points[i - 1].z * tangent_list.points[i].y);
		double norm_y = (binormal_list.points[i - 1].z * tangent_list.points[i].x) - (binormal_list.points[i - 1].x * tangent_list.points[i].z);
		double norm_z = (binormal_list.points[i - 1].x * tangent_list.points[i].y) - (binormal_list.points[i - 1].y * tangent_list.points[i].x);
    //Normalized the list
		double normalMagnitude = sqrt(pow(norm_x,2)+pow(norm_y,2)+pow(norm_z,2));
		if(normalMagnitude==0)
		{
			norm_x=0.0;
			norm_y=0.0;
			norm_z=0.0;
			
		}
		else
		{
		norm_x = 1.0*norm_x /(normalMagnitude);
		norm_y = 1.0*norm_y / normalMagnitude;
		norm_z = 1.0*norm_z / normalMagnitude;
		}
		
		normal_list.points[i].x = norm_x;
		normal_list.points[i].y = norm_y;
		normal_list.points[i].z = norm_z;
	
		// Binormal = Tangent * normal

		double binorm_x = (tangent_list.points[i].y * normal_list.points[i].z ) - (tangent_list.points[i].z * normal_list.points[i].y);
		double binorm_y = (tangent_list.points[i].z * normal_list.points[i].x) - (tangent_list.points[i].x * normal_list.points[i].z );
		double binorm_z = (tangent_list.points[i].x * normal_list.points[i].y) - (tangent_list.points[i].y * normal_list.points[i].x);
    double binormalMagnitude = sqrt(pow(binorm_x,2)+pow(binorm_y,2)+pow(binorm_z,2));
		if(binormalMagnitude==0)
		{
			binorm_x=0.0;
			binorm_y=0.0;
			binorm_z=0.0;
		}
		else{
	  binorm_x = binorm_x / binormalMagnitude;
		binorm_y = binorm_y / binormalMagnitude;
		binorm_z = binorm_z / binormalMagnitude;
		}
		//Normalized list
		binormal_list.points[i].x = binorm_x;
		binormal_list.points[i].y = binorm_y;
		binormal_list.points[i].z = binorm_z;

	}
}

void initSpline() 
{
	// Function to Create the spline

	points_list.numControlpoints = g_Splines[0].numControlpoints * 20-40;
	points_list.points = new points[points_list.numControlpoints];
	double constraint[4][3] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	int numpoints = g_Splines->numControlpoints;
	for (int m = 0; m < numpoints - 3; m++) 
	{
		// control matrix
		double control_mat[4][3] =
		{
			{ g_Splines[0].points[m].x , g_Splines[0].points[m].y, g_Splines[0].points[m].z },
			{ g_Splines[0].points[m + 1].x, g_Splines[0].points[m + 1].y, g_Splines[0].points[m+1].z },
			{ g_Splines[0].points[m + 2].x, g_Splines[0].points[m + 2].y, g_Splines[0].points[m+2].z },
			{ g_Splines[0].points[m + 3].x, g_Splines[0].points[m + 3].y, g_Splines[0].points[m+3].z }
		};

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 3; j++)
			 {
				constraint[i][j] = 0;
			 }
		}
		for (int i = 0; i < 4; i++) 
		{
			for (int j = 0; j < 3; j++) 
			{
				for (int k = 0; k < 4; k++)
				 {
					constraint[i][j] += catmull_mat[i][k] * control_mat[k][j]; // Constraint Matrix
				 }
			}
		}
		
		double x = 0, y = 0, z = 0;
		

		for (double u = 0.05; u <= 1; u += 0.05) 
		{ 
			x = (pow(u,3)*constraint[0][0]) + (pow(u,2)*constraint[1][0]) + (u * constraint[2][0])+ constraint[3][0];
			y = (pow(u,3)*constraint[0][1]) + (pow(u,3)*constraint[1][1]) + (u * constraint[2][1])+ constraint[3][1];
			z = (pow(u,3)*constraint[0][2]) + (pow(u,3)*constraint[1][2]) + (u * constraint[2][2])+ constraint[3][2];
			//Generating the result
      glVertex3f(x, y, z);
			pointsCount++;

			points_list.points[pointsCount].x = x;
			points_list.points[pointsCount].y = y;
			points_list.points[pointsCount].z = z;
			
		}
	
	}
	pointsCount = 0;
}

void initLevel2Ground() 
{  //Load the iamge  
  //Defining the Texture and assigning to the variable
  //Do Texture Warpping
  //Add Fliters 
  // Load image to pix data
	background_image = jpeg_read("background.jpg", NULL);
	glGenTextures(1, &ground_tex); 
	glBindTexture(GL_TEXTURE_2D, ground_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 1024, 0, GL_RGB, GL_UNSIGNED_BYTE, background_image->pix);
}

void initSky()
{ //Load the iamge  
  //Defining the Texture and assigning to the variable
  //Do Texture Warpping
  //Add Fliters 
  // Load image to pix data
	sky_image=jpeg_read("sky.jpg",NULL);
	glGenTextures(1, &sky_tex);
	glBindTexture(GL_TEXTURE_2D, sky_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 1024, 0, GL_RGB, GL_UNSIGNED_BYTE, sky_image->pix);


}
void initnightSky()
{ //Load the iamge  
  //Defining the Texture and assigning to the variable
  //Do Texture Warpping
  //Add Fliters 
  // Load image to pix data
	night_sky_image=jpeg_read("sky_night.jpg",NULL);
	glGenTextures(1, &night_sky_tex);
	glBindTexture(GL_TEXTURE_2D, night_sky_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 1024, 0, GL_RGB, GL_UNSIGNED_BYTE, night_sky_image->pix);


}



void initLights()
{ //Adding lights functionality to the rails
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_ambient);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);

	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
}

void initMaterials()
{ //Adding Matrial Properties 
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, low_shininess);
}

void inittrack_tex() 
{ //Load the iamge  
  //Defining the Texture and assigning to the variable
  //Do Texture Warpping
  //Add Fliters 
  // Load image to pix data
	tracks_image = jpeg_read("metalTexture.jpg", NULL);
	glGenTextures(1, &track_tex);
	glBindTexture(GL_TEXTURE_2D, track_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 1024, 0, GL_RGB, GL_UNSIGNED_BYTE, tracks_image->pix);
}

void initrod_texture()
{

	rod_texture = jpeg_read("wood.jpg", NULL);
	glGenTextures(1, &rod_tex);
	glBindTexture(GL_TEXTURE_2D, rod_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 1024, 0, GL_RGB, GL_UNSIGNED_BYTE, rod_texture->pix);
}	


int pointindex=0;
clock_t t;
void cameraSetup()
{ //Setup the camera, use clock to record the time id, define eye, focus and upvector
	if(pointindex==count_points-50)
	pointindex=10;
	clock_t newT = clock() - t;
	pointindex++;
  points eyepoint = points_list.points[pointindex];
  points focual_point = tangent_list.points[pointindex];
  points upvector = binormal_list.points[pointindex];
	eye.x = eyepoint.x-(0.01*normal_list.points[pointindex].x)+0.5;
	eye.y = eyepoint.y-(0.01*normal_list.points[pointindex].y)+0.3;
	eye.z = eyepoint.z-(0.01*normal_list.points[pointindex].z) ;


	focus.x=eye.x+focual_point.x;
	focus.y=eye.y+focual_point.y;
	focus.z=eye.z+focual_point.z;

	if(upvector.x==0 && upvector.y==0 && upvector.z==0)
	{
	up_vector.x=0.0;
	up_vector.y=-1.0;
	up_vector.z=0.0;
	}
	else
	{	
	up_vector.x = -upvector.x;
	up_vector.y = -upvector.y;
	up_vector.z = -upvector.z;
	}

	t=clock();
}

void displaySpline() 
{
	// Rendering the Spline 
	glColor3f(1.0, 1.0, 1.0);
  glLineWidth(5.0f);
	if(displayrail==0)
	{
	
	glBegin(GL_LINES);
	for (int i = 0; i < points_list.numControlpoints-1; i++) {
			
			glLineWidth(5.0f);
			glVertex3f(points_list.points[i].x, points_list.points[i].y, points_list.points[i].z);
		
			glVertex3f(points_list.points[i+1].x, points_list.points[i+1].y, points_list.points[i+1].z);

		}
	glEnd();

	//Create cross sections
	glBegin(GL_LINES);

	for (int i = 0; i < normal_list.numControlpoints; i++) {
		glVertex3f(points_list.points[i].x, points_list.points[i].y, points_list.points[i].z);
		glVertex3f(points_list.points[i].x + normal_list.points[i].x / 2,
			points_list.points[i].y + normal_list.points[i].y / 2,
			points_list.points[i].z + normal_list.points[i].z / 2);
	}
	
	/* Create the support beams to stand 
  Using GL Lines to hold the roller coaster*/
	
	glBegin(GL_LINES);
	for (int i = 0; i < normal_list.numControlpoints; i++) {
		if (i % 10 == 0) {
      glLineWidth(5.0f);
			glVertex3f(points_list.points[i].x, points_list.points[i].y, points_list.points[i].z);
			glVertex3f(points_list.points[i].x, points_list.points[i].y, -1.0);

		}
	}
	glEnd();
	
	}
	else
	{	
		glCallList(displayListSpline);
	}
	
	glFlush();
}

void displayGround() 
{ //Generating Ground Components

	// Fix the lights properties
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBindTexture(GL_TEXTURE_2D, ground_tex);
	glEnable(GL_TEXTURE_2D);

	glBegin(GL_QUADS);

	glTexCoord2f(0.0, 0.0);
	glVertex3f(-30.0, -30.0, -1.0);
	glTexCoord2f(0.0, 50.0);
	glVertex3f(-30.0, 60.0, -1.0);
	glTexCoord2f(50.0, 0.0);
	glVertex3f(60.0, 60.0, -1.0);
	glTexCoord2f(50.0, 50.0);
	glVertex3f(60.0, -30.0, -1.0);

	glEnd();

	glDisable(GL_TEXTURE_2D);
}

void displaySkybox() 
{
	// Set up the lights component
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	// Deciding which texture to use from DAY OR NIGHT
  if (changetheme == DAY){
	  glBindTexture(GL_TEXTURE_2D, sky_tex);}
  else{glBindTexture(GL_TEXTURE_2D, night_sky_tex);}
	glEnable(GL_TEXTURE_2D);

	glBegin(GL_QUADS);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(-50.0, -50.0, -1.0);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(50.0, -50.0, -1.0);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(50.0, -50.0, 40.0);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(-50.0, -50.0, 40.0);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(-50.0, -50.0, -1.0);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-50.0, 50.0, -1.0);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-50.0, 50.0, 50.0);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(-50.0, -50.0, 50.0);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(-50.0, 50.0, -1.0);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(50.0, 50.0, -1.0);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(50.0, 50.0, 50.0);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(-50.0, 50.0, 50.0);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(50.0, 50.0, -1.0);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(50.0, -50.0, -1.0);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(50.0, -50.0, 50.0);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(50.0, 50.0, 50.0);

	glEnd();

	glDisable(GL_TEXTURE_2D);
  if (changetheme==DAY){
	glBindTexture(GL_TEXTURE_2D, sky_tex);}
  else{glBindTexture(GL_TEXTURE_2D, night_sky_tex);}
	glEnable(GL_TEXTURE_2D);

	glBegin(GL_QUADS);
  //Generating the TOP
	glTexCoord2f(1.0, 1.0);
	glVertex3f(50.0, 50.0, 49.9);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(50.0, -50.0, 49.9);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-50.0, -50.0, 49.9);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(-50.0, 50.0, 49.9);

	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void keyboardFunc(unsigned char key, int x, int y) 
{
  switch (key) {
    
    case 27: // ESC key
        exit(0); // exit the program
        break;

    case 'y':
    //Scale
        g_ControlState = SCALE;
        break;

    // Start Camera
    case 'w':
    	g_vLandRotate[0]=0.0;
    	g_vLandRotate[1]=0.0;
    	g_vLandRotate[2]=0.0;
    	g_vLandTranslate[0]=0.0;
    	g_vLandTranslate[1]=0.0;
    	g_vLandTranslate[2]=0.0;

    	ride=!ride;
    	
    	break;

    case 'l':
    //Change the Rail Mode
    	displayrail=!displayrail;
    	break;
    case 'n':
    //Change the colors from night to day
      changetheme = NIGHT;
      break;
    case 'd':
      changetheme = DAY;
      break;
    // Zoom in Zoom out
    case 'r':
    	eye.z -= 1.0;
    	
    	break;
    case 't':
    //Translate
      g_ControlState = TRANSLATE;
      break;
    case 'e':
      eye.z += 1.0;
      break;

  }
}

//Mouse Functions
void mousedrag(int x, int y)
{
  int vMouseDelta[2] = {x-g_vMousePos[0], y-g_vMousePos[1]};

  
  switch (g_ControlState)
  {
    case TRANSLATE:  
      if (g_iLeftMouseButton)
      {
        g_vLandTranslate[0] += vMouseDelta[0]*0.5;
        g_vLandTranslate[1] -= vMouseDelta[1]*0.5;
      }
      if (g_iMiddleMouseButton)
      {
        g_vLandTranslate[2] += vMouseDelta[1]*0.5;
      }

      break;
    case ROTATE:
      if (g_iLeftMouseButton)
      {
        g_vLandRotate[0] += vMouseDelta[1];
        g_vLandRotate[1] += vMouseDelta[0];
      }
      if (g_iMiddleMouseButton)
      {
        g_vLandRotate[2] += vMouseDelta[1];
      }
      
      break;
    case SCALE:
      if (g_iLeftMouseButton)
      {
        g_vLandScale[0] *= 1.0+vMouseDelta[0]*0.01;
        g_vLandScale[1] *= 1.0-vMouseDelta[1]*0.01;
      }
      if (g_iMiddleMouseButton)
      {
        g_vLandScale[2] *= 1.0-vMouseDelta[1]*0.01;
      }
   
     
      break;
  }
  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}

void mouseidle(int x, int y)
{
  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}

void mousebutton(int button, int state, int x, int y){


  switch (button)
  {
    case GLUT_LEFT_BUTTON:
      g_iLeftMouseButton = (state==GLUT_DOWN);
      break;
    case GLUT_MIDDLE_BUTTON:
      g_iMiddleMouseButton = (state==GLUT_DOWN);
      break;
    case GLUT_RIGHT_BUTTON:
      g_iRightMouseButton = (state==GLUT_DOWN);
      break;
  }
 
  switch(glutGetModifiers())
  {
    case GLUT_ACTIVE_SHIFT:
      g_ControlState = TRANSLATE;
      break;
    default:
      g_ControlState = ROTATE;
      break;
  }

  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}

void reshape(int w, int h)
{
    // Set Image
    glViewport(0,0, w, h); 
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    // Set up the Camera
    gluPerspective (FOV, float(w)/float(h), 0.01, 1000.0);
    glMatrixMode(GL_MODELVIEW);
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor3f(0.0, 0.0, 1.0);
	glShadeModel(GL_SMOOTH);
	glLoadIdentity();


	if(ride)
	cameraSetup();
	gluLookAt(eye.x-(0.01*normal_list.points[0].x), eye.y-(0.01*normal_list.points[0].y), eye.z-(0.01*normal_list.points[0].z),
		focus.x, focus.y, focus.z,
		up_vector.x, up_vector.y, up_vector.z);

	// Write a matrix and push it
	glPushMatrix();
	glTranslatef(1.0*g_vLandTranslate[0],1.0*g_vLandTranslate[1],1.0*g_vLandTranslate[2]);
    glScalef(g_vLandScale[0],g_vLandScale[1],g_vLandScale[2]);
    glRotatef(g_vLandRotate[0], 1.0, 0.0,0.0);
    glRotatef(g_vLandRotate[1], 0.0, 1.0,0.0);
    glRotatef(g_vLandRotate[2], 0.0, 0.0,1.0);

    displaySkybox();
 
    displaySpline();
    displayGround();
    glPopMatrix();

	glutSwapBuffers();
}

 /* Creation of idle function to post redisplay every frame */
void doIdle()
{
	/* make the screen update */
	glutPostRedisplay();
}

/* OpenGL init */
void myInit() 
{

	t=clock();
	// Initialisation for the scene with right materials and a single source of light
	initLights();
	initMaterials();
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
	initLevel2Ground();
	initSky();
  initnightSky();
	inittrack_tex();
	initrod_texture();
	initSpline();
	calculatetangent_list();
	normal_listBinormal_list();

	/* Intialise the display list */
	displayListSpline = glGenLists(1);
	glNewList(displayListSpline, GL_COMPILE);
	
	// modulate texture with lighting
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glBindTexture(GL_TEXTURE_2D, track_tex);
	glEnable(GL_TEXTURE_2D);

	glBegin(GL_QUADS);
	double distance_factor = 0.05;
	for (int i = 0; i < points_list.numControlpoints; i++)
	{ //Generating the Rial
		double X = points_list.points[i].x;
		double Y = points_list.points[i].y;
		double Z = points_list.points[i].z;
		
		double X2 = points_list.points[i + 1].x;
		double Y2 = points_list.points[i + 1].y;
		double Z2 = points_list.points[i + 1].z;
		
		double rail1_vertex1 = X - distance_factor*(normal_list.points[i].x - binormal_list.points[i].x);
		double rail1_vertexy1 = Y - distance_factor*(normal_list.points[i].y - binormal_list.points[i].y);
		double rail1_vertexz1 = Z - distance_factor*(normal_list.points[i].z - binormal_list.points[i].z);

		double rail1_vertex2 = X + distance_factor*(normal_list.points[i].x + binormal_list.points[i].x);
		double rail1_vertexy2 = Y + distance_factor*(normal_list.points[i].y + binormal_list.points[i].y);
		double rail1_vertexz2 = Z + distance_factor*(normal_list.points[i].z + binormal_list.points[i].z);

		double rail1_vertex3 = X + distance_factor*(normal_list.points[i].x - binormal_list.points[i].x);
		double rail1_vertexy3 = Y + distance_factor*(normal_list.points[i].y - binormal_list.points[i].y);
		double rail1_vertexz3 = Z + distance_factor*(normal_list.points[i].z - binormal_list.points[i].z);

		double rail1_vertex4 = X - distance_factor*(normal_list.points[i].x + binormal_list.points[i].x);
		double rail1_vertexy4 = Y - distance_factor*(normal_list.points[i].y + binormal_list.points[i].y);
		double rail1_vertexz4 = Z - distance_factor*(normal_list.points[i].z + binormal_list.points[i].z);

		double rail1_vertex5 = X2 - distance_factor*(normal_list.points[i+1].x - binormal_list.points[i + 1].x);
		double rail1_vertexy5 = Y2 - distance_factor*(normal_list.points[i+1].y - binormal_list.points[i + 1].y);
		double rail1_vertexz5 = Z2 - distance_factor*(normal_list.points[i+1].z - binormal_list.points[i + 1].z);

		double rail1_vertex6 = X2 + distance_factor*(normal_list.points[i+1].x + binormal_list.points[i + 1].x);
		double rail1_vertexy6 = Y2 + distance_factor*(normal_list.points[i+1].y + binormal_list.points[i + 1].y);
		double rail1_vertexz6 = Z2 + distance_factor*(normal_list.points[i+1].z + binormal_list.points[i + 1].z);

		double rail1_vertex7 = X2 + distance_factor*(normal_list.points[i+1].x - binormal_list.points[i + 1].x);
		double rail1_vertexy7 = Y2 + distance_factor*(normal_list.points[i+1].y - binormal_list.points[i + 1].y);
		double rail1_vertexz7 = Z2 + distance_factor*(normal_list.points[i+1].z - binormal_list.points[i + 1].z);

		double rail1_vertex8 = X2 - distance_factor*(normal_list.points[i+1].x + binormal_list.points[i + 1].x);
		double rail1_vertexy8 = Y2 - distance_factor*(normal_list.points[i+1].y + binormal_list.points[i + 1].y);
		double rail1_vertexz8 = Z2 - distance_factor*(normal_list.points[i+1].z + binormal_list.points[i + 1].z);

		glNormal3f(-normal_list.points[i].x, -normal_list.points[i].y, -normal_list.points[i].z);
		glTexCoord2f(1.0, 0.0);
		glVertex3f(rail1_vertex1, rail1_vertexy1, rail1_vertexz1);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(rail1_vertex2, rail1_vertexy2, rail1_vertexz2);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(rail1_vertex6, rail1_vertexy6, rail1_vertexz6);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(rail1_vertex5, rail1_vertexy5, rail1_vertexz5);

		glNormal3f(-binormal_list.points[i].x, -binormal_list.points[i].y, -binormal_list.points[i].z);
		// Top
		glTexCoord2f(1.0, 0.0);
		glVertex3f(rail1_vertex2, rail1_vertexy2, rail1_vertexz2);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(rail1_vertex3, rail1_vertexy3, rail1_vertexz3);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(rail1_vertex7, rail1_vertexy7, rail1_vertexz7);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(rail1_vertex6, rail1_vertexy6, rail1_vertexz6);

		glNormal3f(normal_list.points[i].x, normal_list.points[i].y, normal_list.points[i].z);
		// Left 
		glTexCoord2f(1.0, 0.0);
		glVertex3f(rail1_vertex3, rail1_vertexy3, rail1_vertexz3);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(rail1_vertex4, rail1_vertexy4, rail1_vertexz4);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(rail1_vertex8, rail1_vertexy8, rail1_vertexz8);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(rail1_vertex7, rail1_vertexy7, rail1_vertexz7);

		glNormal3f(binormal_list.points[i].x, binormal_list.points[i].y, binormal_list.points[i].z);
		// Bottom 
		glTexCoord2f(1.0, 0.0);
		glVertex3f(rail1_vertex1, rail1_vertexy1, rail1_vertexz1);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(rail1_vertex4, rail1_vertexy4, rail1_vertexz4);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(rail1_vertex8, rail1_vertexy8, rail1_vertexz8);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(rail1_vertex5, rail1_vertexy5, rail1_vertexz5);
		double rail2_vertex1 = X - distance_factor*(-8*normal_list.points[i].x - binormal_list.points[i].x);
		double rail2_vertexy1 = Y - distance_factor*(-8*normal_list.points[i].y - binormal_list.points[i].y);
		double rail2_vertexz1 = Z - distance_factor*(-8*normal_list.points[i].z - binormal_list.points[i].z);

		double rail2_vertex2 = X + distance_factor*(10*normal_list.points[i].x + binormal_list.points[i].x);
		double rail2_vertexy2 = Y + distance_factor*(10*normal_list.points[i].y + binormal_list.points[i].y);
		double rail2_vertexz2 = Z + distance_factor*(10*normal_list.points[i].z + binormal_list.points[i].z);

		double rail2_vertex3 = X + distance_factor*(10*normal_list.points[i].x - binormal_list.points[i].x);
		double rail2_vertexy3 = Y + distance_factor*(10*normal_list.points[i].y - binormal_list.points[i].y);
		double rail2_vertexz3 = Z + distance_factor*(10*normal_list.points[i].z - binormal_list.points[i].z);

		double rail2_vertex4 = X - distance_factor*(-8*normal_list.points[i].x + binormal_list.points[i].x);
		double rail2_vertexy4 = Y - distance_factor*(-8*normal_list.points[i].y + binormal_list.points[i].y);
		double rail2_vertexz4 = Z - distance_factor*(-8*normal_list.points[i].z + binormal_list.points[i].z);

		double rail2_vertex5 = X2 - distance_factor*(-8*normal_list.points[i+1].x - binormal_list.points[i + 1].x);
		double rail2_vertexy5 = Y2 - distance_factor*(-8*normal_list.points[i+1].y - binormal_list.points[i + 1].y);
		double rail2_vertexz5 = Z2 - distance_factor*(-8*normal_list.points[i+1].z - binormal_list.points[i + 1].z);

		double rail2_vertex6 = X2 + distance_factor*(10*normal_list.points[i+1].x + binormal_list.points[i + 1].x);
		double rail2_vertexy6 = Y2 + distance_factor*(10*normal_list.points[i+1].y + binormal_list.points[i + 1].y);
		double rail2_vertexz6 = Z2 + distance_factor*(10*normal_list.points[i+1].z + binormal_list.points[i + 1].z);

		double rail2_vertex7 = X2 + distance_factor*(10*normal_list.points[i+1].x - binormal_list.points[i + 1].x);
		double rail2_vertexy7 = Y2 + distance_factor*(10*normal_list.points[i+1].y - binormal_list.points[i + 1].y);
		double rail2_vertexz7 = Z2 + distance_factor*(10*normal_list.points[i+1].z - binormal_list.points[i + 1].z);

		double rail2_vertex8 = X2 - distance_factor*(-8*normal_list.points[i+1].x + binormal_list.points[i + 1].x);
		double rail2_vertexy8 = Y2 - distance_factor*(-8*normal_list.points[i+1].y + binormal_list.points[i + 1].y);
		double rail2_vertexz8 = Z2 - distance_factor*(-8*normal_list.points[i+1].z + binormal_list.points[i + 1].z);

		glNormal3f(-normal_list.points[i].x, -normal_list.points[i].y, -normal_list.points[i].z);
		// Right
		glTexCoord2f(1.0, 0.0);
		glVertex3f(rail2_vertex1, rail2_vertexy1, rail2_vertexz1);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(rail2_vertex2, rail2_vertexy2, rail2_vertexz2);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(rail2_vertex6, rail2_vertexy6, rail2_vertexz6);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(rail2_vertex5, rail2_vertexy5, rail2_vertexz5);

		glNormal3f(-binormal_list.points[i].x, -binormal_list.points[i].y, -binormal_list.points[i].z);
		// Top 
		glTexCoord2f(1.0, 0.0);
		glVertex3f(rail2_vertex1, rail2_vertexy1, rail2_vertexz2);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(rail2_vertex3, rail2_vertexy3, rail2_vertexz3);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(rail2_vertex7, rail2_vertexy7, rail2_vertexz7);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(rail2_vertex6, rail2_vertexy6, rail2_vertexz6);

		glNormal3f(normal_list.points[i].x, normal_list.points[i].y, normal_list.points[i].z);
		// Left
		glTexCoord2f(1.0, 0.0);
		glVertex3f(rail2_vertex3, rail2_vertexy3, rail2_vertexz3);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(rail2_vertex4, rail2_vertexy4, rail2_vertexz4);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(rail2_vertex8, rail2_vertexy8, rail2_vertexz8);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(rail2_vertex7, rail2_vertexy7, rail2_vertexz7);

		glNormal3f(binormal_list.points[i].x, binormal_list.points[i].y, binormal_list.points[i].z);
		// Bottom
		glTexCoord2f(1.0, 0.0);
		glVertex3f(rail2_vertex1, rail2_vertexy1, rail2_vertexz1);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(rail2_vertex4, rail2_vertexy4, rail2_vertexz4);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(rail2_vertex8, rail2_vertexy8, rail2_vertexz8);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(rail2_vertex5, rail2_vertexy5, rail2_vertexz5);


	}
	
	glEnd();

	glBegin(GL_QUADS);

	for (int i = 0; i < normal_list.numControlpoints; i++) {
		if (i % 20 == 0) {

		//Guide Rails

		double Xg = points_list.points[i].x;
		double Yg = points_list.points[i].y;
		double Zg = points_list.points[i].z;
		
		double X2g = points_list.points[i].x;
		double Y2g = points_list.points[i].y;
		double Z2g = -1.0;

		double rail1_vertex1g = Xg - distance_factor*(normal_list.points[i].x - binormal_list.points[i].x);
		double rail1_vertexy1g = Yg - distance_factor*(normal_list.points[i].y - binormal_list.points[i].y);
		double rail1_vertexz1g = Zg - distance_factor*(normal_list.points[i].z - binormal_list.points[i].z);

		double rail1_vertex2g = Xg + distance_factor*(normal_list.points[i].x + binormal_list.points[i].x);
		double rail1_vertexy2g = Yg + distance_factor*(normal_list.points[i].y + binormal_list.points[i].y);
		double rail1_vertexz2g = Zg + distance_factor*(normal_list.points[i].z + binormal_list.points[i].z);

		double rail1_vertex3g = Xg + distance_factor*(normal_list.points[i].x - binormal_list.points[i].x);
		double rail1_vertexy3g = Yg + distance_factor*(normal_list.points[i].y - binormal_list.points[i].y);
		double rail1_vertexz3g = Zg + distance_factor*(normal_list.points[i].z - binormal_list.points[i].z);

		double rail1_vertex4g = Xg - distance_factor*(normal_list.points[i].x + binormal_list.points[i].x);
		double rail1_vertexy4g = Yg - distance_factor*(normal_list.points[i].y + binormal_list.points[i].y);
		double rail1_vertexz4g = Zg - distance_factor*(normal_list.points[i].z + binormal_list.points[i].z);

		double rail1_vertex5g = X2g - distance_factor*(normal_list.points[i].x - binormal_list.points[i].x);
		double rail1_vertexy5g = Y2g - distance_factor*(normal_list.points[i].y - binormal_list.points[i].y);
		double rail1_vertexz5g = Z2g - distance_factor*(normal_list.points[i].z - binormal_list.points[i].z);

		double rail1_vertex6g = X2g + distance_factor*(normal_list.points[i].x + binormal_list.points[i].x);
		double rail1_vertexy6g = Y2g + distance_factor*(normal_list.points[i].y + binormal_list.points[i].y);
		double rail1_vertexz6g = Z2g + distance_factor*(normal_list.points[i].z + binormal_list.points[i].z);

		double rail1_vertex7g = X2g + distance_factor*(normal_list.points[i].x - binormal_list.points[i].x);
		double rail1_vertexy7g = Y2g + distance_factor*(normal_list.points[i].y - binormal_list.points[i].y);
		double rail1_vertexz7g = Z2g + distance_factor*(normal_list.points[i].z - binormal_list.points[i].z);

		double rail1_vertex8g = X2g - distance_factor*(normal_list.points[i].x + binormal_list.points[i].x);
		double rail1_vertexy8g = Y2g - distance_factor*(normal_list.points[i].y + binormal_list.points[i].y);
		double rail1_vertexz8g = Z2g - distance_factor*(normal_list.points[i].z + binormal_list.points[i].z);



		glNormal3f(-normal_list.points[i].x, -normal_list.points[i].y, -normal_list.points[i].z);
		// Right
		glTexCoord2f(1.0, 0.0);
		glVertex3f(rail1_vertex1g, rail1_vertexy1g, rail1_vertexz1g);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(rail1_vertex2g, rail1_vertexy2g, rail1_vertexz2g);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(rail1_vertex6g, rail1_vertexy6g, rail1_vertexz6g);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(rail1_vertex5g, rail1_vertexy5g, rail1_vertexz5g);

		glNormal3f(-binormal_list.points[i].x, -binormal_list.points[i].y, -binormal_list.points[i].z);
		// Top
		glTexCoord2f(1.0, 0.0);
		glVertex3f(rail1_vertex2g, rail1_vertexy2g, rail1_vertexz2g);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(rail1_vertex3g, rail1_vertexy3g, rail1_vertexz3g);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(rail1_vertex7g, rail1_vertexy7g, rail1_vertexz7g);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(rail1_vertex6g, rail1_vertexy6g, rail1_vertexz6g);

		glNormal3f(normal_list.points[i].x, normal_list.points[i].y, normal_list.points[i].z);
		// Left
		glTexCoord2f(1.0, 0.0);
		glVertex3f(rail1_vertex3g, rail1_vertexy3g, rail1_vertexz3g);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(rail1_vertex4g, rail1_vertexy4g, rail1_vertexz4g);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(rail1_vertex8g, rail1_vertexy8g, rail1_vertexz8g);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(rail1_vertex7g, rail1_vertexy7g, rail1_vertexz7g);

		glNormal3f(binormal_list.points[i].x, binormal_list.points[i].y, binormal_list.points[i].z);
		// Bottom 
		glTexCoord2f(1.0, 0.0);
		glVertex3f(rail1_vertex1g, rail1_vertexy1g, rail1_vertexz1g);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(rail1_vertex4g, rail1_vertexy4g, rail1_vertexz4g);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(rail1_vertex8g, rail1_vertexy8g, rail1_vertexz8g);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(rail1_vertex5g, rail1_vertexy5g, rail1_vertexz5g);

		// Rail2
		double Xg2 = points_list.points[i].x ;
		double Yg2 = points_list.points[i].y ;
		double Zg2 = points_list.points[i].z ;
		
		double X2g2 = points_list.points[i].x ;
		double Y2g2 = points_list.points[i].y ;
		double Z2g2 = -1.0;


		double rail2_vertex1g = Xg2 - distance_factor*(-8*normal_list.points[i].x - binormal_list.points[i].x);
		double rail2_vertexy1g = Yg2 - distance_factor*(-8*normal_list.points[i].y - binormal_list.points[i].y);
		double rail2_vertexz1g = Zg2 - distance_factor*(-8*normal_list.points[i].z - binormal_list.points[i].z);

		double rail2_vertex2g = Xg2 + distance_factor*(10*normal_list.points[i].x + binormal_list.points[i].x);
		double rail2_vertexy2g = Yg2 + distance_factor*(10*normal_list.points[i].y + binormal_list.points[i].y);
		double rail2_vertexz2g = Zg2 + distance_factor*(10*normal_list.points[i].z + binormal_list.points[i].z);

		double rail2_vertex3g = Xg2 + distance_factor*(10*normal_list.points[i].x - binormal_list.points[i].x);
		double rail2_vertexy3g = Yg2 + distance_factor*(10*normal_list.points[i].y - binormal_list.points[i].y);
		double rail2_vertexz3g = Zg2 + distance_factor*(10*normal_list.points[i].z - binormal_list.points[i].z);

		double rail2_vertex4g = Xg2 - distance_factor*(-8*normal_list.points[i].x + binormal_list.points[i].x);
		double rail2_vertexy4g = Yg2 - distance_factor*(-8*normal_list.points[i].y + binormal_list.points[i].y);
		double rail2_vertexz4g = Zg2 - distance_factor*(-8*normal_list.points[i].z + binormal_list.points[i].z);

		double rail2_vertex5g = X2g2 - distance_factor*(-8*normal_list.points[i].x - binormal_list.points[i].x);
		double rail2_vertexy5g = Y2g2 - distance_factor*(-8*normal_list.points[i].y - binormal_list.points[i].y);
		double rail2_vertexz5g = Z2g2 - distance_factor*(-8*normal_list.points[i].z - binormal_list.points[i].z);

		double rail2_vertex6g = X2g2 + distance_factor*(10*normal_list.points[i].x + binormal_list.points[i].x);
		double rail2_vertexy6g = Y2g2 + distance_factor*(10*normal_list.points[i].y + binormal_list.points[i].y);
		double rail2_vertexz6g = Z2g2 + distance_factor*(10*normal_list.points[i].z + binormal_list.points[i].z);

		double rail2_vertex7g = X2g2 + distance_factor*(10*normal_list.points[i].x - binormal_list.points[i].x);
		double rail2_vertexy7g = Y2g2 + distance_factor*(10*normal_list.points[i].y - binormal_list.points[i].y);
		double rail2_vertexz7g = Z2g2 + distance_factor*(10*normal_list.points[i].z - binormal_list.points[i].z);

		double rail2_vertex8g = X2g2 - distance_factor*(-8*normal_list.points[i].x + binormal_list.points[i].x);
		double rail2_vertexy8g = Y2g2 - distance_factor*(-8*normal_list.points[i].y + binormal_list.points[i].y);
		double rail2_vertexz8g = Z2g2 - distance_factor*(-8*normal_list.points[i].z + binormal_list.points[i].z);

		glNormal3f(-normal_list.points[i].x, -normal_list.points[i].y, -normal_list.points[i].z);
		// Right
		glTexCoord2f(1.0, 0.0);
		glVertex3f(rail2_vertex1g, rail2_vertexy1g, rail2_vertexz1g);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(rail2_vertex2g, rail2_vertexy2g, rail2_vertexz2g);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(rail2_vertex6g, rail2_vertexy6g, rail2_vertexz6g);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(rail2_vertex5g, rail2_vertexy5g, rail2_vertexz5g);

		glNormal3f(-binormal_list.points[i].x, -binormal_list.points[i].y, -binormal_list.points[i].z);
		// Top
		glTexCoord2f(1.0, 0.0);
		glVertex3f(rail2_vertex1g, rail2_vertexy1g, rail2_vertexz2g);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(rail2_vertex3g, rail2_vertexy3g, rail2_vertexz3g);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(rail2_vertex7g, rail2_vertexy7g, rail2_vertexz7g);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(rail2_vertex6g, rail2_vertexy6g, rail2_vertexz6g);

		glNormal3f(normal_list.points[i].x, normal_list.points[i].y, normal_list.points[i].z);
		// Left 
		glTexCoord2f(1.0, 0.0);
		glVertex3f(rail2_vertex3g, rail2_vertexy3g, rail2_vertexz3g);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(rail2_vertex4g, rail2_vertexy4g, rail2_vertexz4g);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(rail2_vertex8g, rail2_vertexy8g, rail2_vertexz8g);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(rail2_vertex7g, rail2_vertexy7g, rail2_vertexz7g);

		glNormal3f(binormal_list.points[i].x, binormal_list.points[i].y, binormal_list.points[i].z);
		// Bottom
		glTexCoord2f(1.0, 0.0);
		glVertex3f(rail2_vertex1g, rail2_vertexy1g, rail2_vertexz1g);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(rail2_vertex4g, rail2_vertexy4g, rail2_vertexz4g);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(rail2_vertex8g, rail2_vertexy8g, rail2_vertexz8g);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(rail2_vertex5g, rail2_vertexy5g, rail2_vertexz5g);

		}
	}

	glEnd();
	glDisable(GL_TEXTURE_2D);
  //Turn on Texture Mapping
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glBindTexture(GL_TEXTURE_2D, rod_tex);
	glEnable(GL_TEXTURE_2D);


	glBegin(GL_QUADS);

	for (int i = 0; i < normal_list.numControlpoints; i++) {

		double Xg = points_list.points[i].x;
		double Yg = points_list.points[i].y;
		double Zg = points_list.points[i].z;
		
		double X2g = points_list.points[i].x + normal_list.points[i].x / 2.75;
		double Y2g = points_list.points[i].y + normal_list.points[i].y / 2.75;
		double Z2g = points_list.points[i].z + normal_list.points[i].z / 2.75;

		double rail1_vertex1g = Xg - distance_factor*(normal_list.points[i].x - binormal_list.points[i].x);
		double rail1_vertexy1g = Yg - distance_factor*(normal_list.points[i].y - binormal_list.points[i].y);
		double rail1_vertexz1g = Zg - distance_factor*(normal_list.points[i].z - binormal_list.points[i].z);

		double rail1_vertex2g = Xg + distance_factor*(normal_list.points[i].x + binormal_list.points[i].x);
		double rail1_vertexy2g = Yg + distance_factor*(normal_list.points[i].y + binormal_list.points[i].y);
		double rail1_vertexz2g = Zg + distance_factor*(normal_list.points[i].z + binormal_list.points[i].z);

		double rail1_vertex3g = Xg + distance_factor*(normal_list.points[i].x - binormal_list.points[i].x);
		double rail1_vertexy3g = Yg + distance_factor*(normal_list.points[i].y - binormal_list.points[i].y);
		double rail1_vertexz3g = Zg + distance_factor*(normal_list.points[i].z - binormal_list.points[i].z);

		double rail1_vertex4g = Xg - distance_factor*(normal_list.points[i].x + binormal_list.points[i].x);
		double rail1_vertexy4g = Yg - distance_factor*(normal_list.points[i].y + binormal_list.points[i].y);
		double rail1_vertexz4g = Zg - distance_factor*(normal_list.points[i].z + binormal_list.points[i].z);

		double rail1_vertex5g = X2g - distance_factor*(normal_list.points[i].x - binormal_list.points[i].x);
		double rail1_vertexy5g = Y2g - distance_factor*(normal_list.points[i].y - binormal_list.points[i].y);
		double rail1_vertexz5g = Z2g - distance_factor*(normal_list.points[i].z - binormal_list.points[i].z);

		double rail1_vertex6g = X2g + distance_factor*(normal_list.points[i].x + binormal_list.points[i].x);
		double rail1_vertexy6g = Y2g + distance_factor*(normal_list.points[i].y + binormal_list.points[i].y);
		double rail1_vertexz6g = Z2g + distance_factor*(normal_list.points[i].z + binormal_list.points[i].z);

		double rail1_vertex7g = X2g + distance_factor*(normal_list.points[i].x - binormal_list.points[i].x);
		double rail1_vertexy7g = Y2g + distance_factor*(normal_list.points[i].y - binormal_list.points[i].y);
		double rail1_vertexz7g = Z2g + distance_factor*(normal_list.points[i].z - binormal_list.points[i].z);

		double rail1_vertex8g = X2g - distance_factor*(normal_list.points[i].x + binormal_list.points[i].x);
		double rail1_vertexy8g = Y2g - distance_factor*(normal_list.points[i].y + binormal_list.points[i].y);
		double rail1_vertexz8g = Z2g - distance_factor*(normal_list.points[i].z + binormal_list.points[i].z);
		glNormal3f(-normal_list.points[i].x, -normal_list.points[i].y, -normal_list.points[i].z);
		glTexCoord2f(1.0, 0.0);
		glVertex3f(rail1_vertex1g, rail1_vertexy1g, rail1_vertexz1g);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(rail1_vertex2g, rail1_vertexy2g, rail1_vertexz2g);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(rail1_vertex6g, rail1_vertexy6g, rail1_vertexz6g);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(rail1_vertex5g, rail1_vertexy5g, rail1_vertexz5g);

		glNormal3f(-binormal_list.points[i].x, -binormal_list.points[i].y, -binormal_list.points[i].z);

		glTexCoord2f(1.0, 0.0);
		glVertex3f(rail1_vertex2g, rail1_vertexy2g, rail1_vertexz2g);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(rail1_vertex3g, rail1_vertexy3g, rail1_vertexz3g);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(rail1_vertex7g, rail1_vertexy7g, rail1_vertexz7g);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(rail1_vertex6g, rail1_vertexy6g, rail1_vertexz6g);

		glNormal3f(normal_list.points[i].x, normal_list.points[i].y, normal_list.points[i].z);

		glTexCoord2f(1.0, 0.0);
		glVertex3f(rail1_vertex3g, rail1_vertexy3g, rail1_vertexz3g);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(rail1_vertex4g, rail1_vertexy4g, rail1_vertexz4g);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(rail1_vertex8g, rail1_vertexy8g, rail1_vertexz8g);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(rail1_vertex7g, rail1_vertexy7g, rail1_vertexz7g);

		glNormal3f(binormal_list.points[i].x, binormal_list.points[i].y, binormal_list.points[i].z);

		glTexCoord2f(1.0, 0.0);
		glVertex3f(rail1_vertex1g, rail1_vertexy1g, rail1_vertexz1g);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(rail1_vertex4g, rail1_vertexy4g, rail1_vertexz4g);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(rail1_vertex8g, rail1_vertexy8g, rail1_vertexz8g);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(rail1_vertex5g, rail1_vertexy5g, rail1_vertexz5g);

	}

	glEnd();
	glDisable(GL_TEXTURE_2D);
	glEndList();
	glClearColor(0.0, 0.0, 0.0, 0.0);
}

int main (int argc, char ** argv)
{
  if (argc<2)
  {  
  printf ("usage: %s <trackfile>\n", argv[0]);
  exit(0);
  }

  loadSplines(argv[1]);

  	/* initialization */
  	glutInit(&argc, (char**)argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	/* create window */
	glutInitWindowSize(windowWidth, windowHeight);
	glutInitWindowPosition(100, 100);
	glutCreateWindow(windowTitle);

	/* used for double buffering */
	glEnable(GL_DEPTH_TEST);

	/* tells glut to use a particular display function to redraw */
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);

	/* replace with any animate code */
	glutIdleFunc(doIdle);

	/* callback for mouse drags */
  	glutMotionFunc(mousedrag);
  
 	/* callback for idle mouse movement */
  	glutPassiveMotionFunc(mouseidle);
  
  	/* callback for mouse button changes */
  	glutMouseFunc(mousebutton);
 
	/* callback for keyboard */
  	glutKeyboardFunc(keyboardFunc);

	/* enable materials */
	myInit();

	glutMainLoop();

  return 0;
}