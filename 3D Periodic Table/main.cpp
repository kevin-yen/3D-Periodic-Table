/******************************************************************************

	This program displays a 3D periodic table that is rotatable and the 
		height is based on properties (atmoic radius, ionization energy, 
		electron affinity) that the user can set.

	Main.cpp - Main functions that creates the program.

	Copyright (c) Kevin Yen 2010

******************************************************************************/

/******************************** TO DO LIST *********************************
	[x] Be Able To Display Text
		Use It To Display:
	[x] Atmoic Symbol
	[x] Value Currently Set
	
	[ ] Scales On The Axis

	[ ] Add Menu
		Use It To Switch Between:
	[ ] Different Properties
	[ ] Orthographics/Perspective
	[ ] Different Scale/Offset
	
	[ ] Recode Classes and Functions
	[ ] Write Detailed Comments
******************************************************************************/

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>		// Header File For Windows
#include <stdio.h>			// Header File For Standard Input/Output
#include <stdarg.h>			// Header File For Variable Argument Routines
#include <string.h>			// Header File For Processing Strings
#include <stdlib.h>			// Header File For Standard Libaray
#include <math.h>			// Header File For Math Functions
#include <ctype.h>			// Header File For Processing Characters

#include <gl/gl.h>			// Header File For OpenGL32 Library
#include <gl/glu.h>			// Header File For Glu32 Library

#include "periodic.h"		// Header File For Periodic Table Class
#include "csvload.h"		// Header File For Loading .csv Files

#define PI 3.141592654f		// Value Of Pi

HWND	hMainWnd;			// Holds The Window Handle
HDC		hMainDC;			// Window GDI Device Context
HGLRC	hMainRC;			// WIndow Rendering Context

char	*lpszClassName = "3D Periodic Table";		// Window Class Name
bool	fQuit;				// Quit Window Flag
bool	fDebug;				// Debug Flag
bool	fScale;				// Scale Flag
bool	rgKeys[256];		// Array To Determine Which Keys Are Pushed Down

GLuint	fontbase;			// Base Of The Display List For Font Set
GLYPHMETRICSFLOAT gmf[256];	// Storage For Infomation About The Outline Font

float	fpCameraRotX;		// X Rotation Of The Camera (With Respect To The Origin)
float	fpCameraRotY;		// Y Rotation Of The Camera
float	fpCameraDist;		// Distance Of The Camera From The Origin

CPeriodicTable PeriodicTable;	// Stores Infomation About The Periodic Table
float	fpOptimalScaling[3] = {2.0f, 0.3f, 2.0f};	// Optimal Scaling For Table

bool	fPerspective;		// Current Projection Style Set To False (Orthographic) By Default
bool	fLightingEnabled;	// Lighting Flag Set to True by Default
GLfloat LightAmbient[] = {0.4f, 0.4f, 0.4f, 1.0f};		// Ambient Lighting
GLfloat LightDiffuse[] = {0.5f, 0.5f, 0.5f, 1.0f};		// Diffused Lighting
GLfloat LightPosition[] = {1.0f, 20.0f, 10.0f, 1.0f};	// Light Position
GLfloat LightDirection[] = {0.0f, 0.0f, 0.0f};			// Light Direction

/* This Function Displays An Error Message. Parameters:						*
 * hWnd			- Handle To The Parent Window (Can Be NULL)					*
 * lpText		- The Text Of The Error Message								*
 * lpCaption	- Caption Of The Error Message								*
 * dwErrorCode	- Any Error Code Returned (<0 Is Error Code Set By Program)	*
 * uType		- Type Of Message Box										*/
void ErrorMessage(HWND hWnd, char *lpText, char *lpCaption, int dwErrorCode, int uType)
{
	char *szMessage;		// Holds The Composite Text To Display To Message Box

	if(dwErrorCode < 0)		// The Error Is Flag By The Program
	{
		szMessage = (char *)calloc(strlen(lpText)+10, sizeof(char));	// Allocate Memory
		sprintf(szMessage, "%s /n(PROGRAM ERROR CODE %d)", lpText, -dwErrorCode);
		MessageBox(hWnd, (LPCSTR)szMessage, lpCaption, uType);			// Display Message Box
	}
	
	else					// The Error Is Flag By The System
	{
		szMessage = (char *)calloc(strlen(lpText)+10, sizeof(char));	// Allocate Memory
		sprintf(szMessage, "%s /n(SYSTEM ERROR CODE %d)", lpText, dwErrorCode);
		MessageBox(hWnd, (LPCSTR)szMessage, lpCaption, uType);			// Display Message Box
	}

	return;
}

/* This Function Disables OpenGL Capabilities. Parameters:					*
 * hWnd			- Handle To the Parent Window								*
 * hDc			- The GDI Device Context									*
 * hRC			- The Rendering Context										*/
void DisableOpenGL(HWND hWnd, HDC hDC, HGLRC hRC)
{
	wglMakeCurrent(NULL, NULL);	// Release The DC And RC Contexts
	wglDeleteContext(hRC);		// Delete The Rendering Context
	ReleaseDC(hWnd, hDC);		// Release The GDI Device Context
}

/* This Function Enables OpenGL Capabilities. Parameters:					*
 * hWnd			- Handle To the Parent Window								*
 * hDc			- The GDI Device Context									*
 * hRC			- The Rendering Context										*/
int EnableOpenGL(HWND hWnd, HDC *hDC, HGLRC *hRC)
{
	PIXELFORMATDESCRIPTOR pfd;	// pfd Tells Windows How We Want The Formats To Be
	int pxlFormat;				// Hold Results After Matching A Pixel Formal

	if(!(*hDC = GetDC(hWnd)))	// Did We Get A Device Context?
	{
		DisableOpenGL(hWnd, *hDC, *hRC);				// Disable OpenGL Capabilities
		ErrorMessage(NULL, "Can't create a OpenGL device context.", "Error!",
			GetLastError( ), MB_OK | MB_ICONERROR);		// Display Error 
		return 0;										// Return False
	}

	// Set pixel format for DC
	ZeroMemory(&pfd, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;
	if(!(pxlFormat = ChoosePixelFormat(*hDC, &pfd)))
	{
		DisableOpenGL(hWnd, *hDC, *hRC);
		ErrorMessage(NULL, "Can't find a suitable pixel format.", "Error!",
			GetLastError( ), MB_OK | MB_ICONERROR);
		return 0;
	}
	if(!(SetPixelFormat(*hDC, pxlFormat, &pfd)))
	{
		DisableOpenGL(hWnd, *hDC, *hRC);
		ErrorMessage(NULL, "Can't set the pixel format.", "Error!", 
			GetLastError( ), MB_OK | MB_ICONERROR);
		return 0;
	}

	// Create and enable the render context (RC)
	if(!(*hRC = wglCreateContext(*hDC)))
	{
		DisableOpenGL(hWnd, *hDC, *hRC);
		ErrorMessage(NULL, "Can't create a OpenGL rendering context.", "Error!", 
			GetLastError( ), MB_OK | MB_ICONERROR);
		return 0;
	}
	if(!wglMakeCurrent(*hDC, *hRC))
	{
		DisableOpenGL(hWnd, *hDC, *hRC);
		ErrorMessage(NULL, "Can't activate the OpenGL rendering context.", "Error!",
			GetLastError( ), MB_OK | MB_ICONERROR);
		return 0;
	}

	return 1;
}

// Build Font
GLvoid BuildFont(int height, int width, const char *fontname)
{
	HFONT	font;

	fontbase = glGenLists(256);

	font = CreateFont(	height,
						width,
						0,
						0,
						FW_BOLD,
						FALSE,
						FALSE,
						FALSE,
						ANSI_CHARSET,
						OUT_TT_PRECIS,
						CLIP_DEFAULT_PRECIS,
						ANTIALIASED_QUALITY,
						FF_DONTCARE|DEFAULT_PITCH,
						fontname);

	SelectObject(hMainDC, font);

	wglUseFontOutlines(	hMainDC,
						0,
						255,
						fontbase,
						0.0f,
						0.0f,
						WGL_FONT_POLYGONS,
						gmf);
}

// Delete Font
GLvoid DeleteFont( )
{
	glDeleteLists(fontbase, 256);
}

// Draw Text
void cprintf(const char *fmt, ...)
{
	float	length = 0;
	char	text[256];
	unsigned int index;
	va_list	ap;

	if(fmt == NULL)
		return;

	va_start(ap, fmt);
		vsprintf(text, fmt, ap);
	va_end(ap);

	for(index = 0; index < (strlen(text)); index++)
		length += gmf[text[index]].gmfCellIncX;

	glTranslatef(-length/2,0.0f,0.0f);

	glPushAttrib(GL_LIST_BIT);
	glListBase(fontbase);
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);
	glPopAttrib( );
}

// Initialize OpenGL function
int InitOpenGL(void)
{
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.05f, 0.1f, 0.5f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	// Setup Light
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);
	glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, LightDirection);
	
	BuildFont(-12, 0, "Tahoma"); 
	return 1;
}

// Reset Projection
void ResetProjection(int width, int height)
{
	if(fPerspective)
		gluPerspective(45.0f, (GLfloat)width/(GLfloat)height, 0.1f, 100.0f);
	else
		glOrtho(-fpCameraDist * ((float)width/(float)height), fpCameraDist * ((float)width/(float)height), -fpCameraDist, fpCameraDist, 0.0f, 200.0f);
}

// Reszie OpenGL window
void ResizeOpenGLWindow(int width, int height)
{
	if(height == 0)
		height = 1;

	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity( );

	ResetProjection(width, height);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity( );
}
// Draw Column
void DrawColumn(float left, float top, float right, float bottom, float height, float red, float green, float blue)
{
	GLfloat rgfpColumnColor[3] = {red, green, blue};

	glPushMatrix( );
	
	glColor3f(red, green, blue);
	glMaterialfv(GL_FRONT, GL_SPECULAR, rgfpColumnColor);

	// If height is nearly zero
	if(height < 0.01f)
	{
		glBegin(GL_QUADS);
			// Square represent a height of 0
			glNormal3f(0.0f, 1.0f, 0.0f);
			glVertex3f(right, height, bottom);
			glVertex3f(right, height, top);
			glVertex3f(left, height, top);
			glVertex3f(left, height, bottom);
		glEnd( );

		glPopMatrix( );
		glPushMatrix( );

		// Black Line Outline
		glDisable(GL_LIGHTING);
		glColor3f(0.0f, 0.0f, 0.0f);	

		glBegin(GL_LINE_LOOP);
			// Top outline
			glVertex3f(left, height+0.01f, bottom);
			glVertex3f(right, height+0.01f, bottom);
			glVertex3f(right, height+0.01f, top);
			glVertex3f(left, height+0.01f, top);
		glEnd( );
	}

	else
	{
		glBegin(GL_QUADS);
			// Bottom Face
			glNormal3f(0.0f, -1.0f, 0.0f);
			glVertex3f(left, 0, bottom);
			glVertex3f(left, 0, top);
			glVertex3f(right, 0, top);
			glVertex3f(right, 0, bottom);

			// Right Face
			glNormal3f(1.0f, 0.0f, 0.0f);
			glVertex3f(right, 0, bottom);	
			glVertex3f(right, 0, top);
			glVertex3f(right, height, top);
			glVertex3f(right, height, bottom);

			// Top Face
			glNormal3f(0.0f, 1.0f, 0.0f);
			glVertex3f(right, height, bottom);
			glVertex3f(right, height, top);
			glVertex3f(left, height, top);
			glVertex3f(left, height, bottom);
		
			// Left Face
			glNormal3f(-1.0f, 0.0f, 0.0f);
			glVertex3f(left, height, bottom);
			glVertex3f(left, height, top);
			glVertex3f(left, 0, top);
			glVertex3f(left, 0, bottom);

			// Back Face
			glNormal3f(0.0f, 0.0f, -1.0f);
			glVertex3f(left, height, top);
			glVertex3f(right, height, top);
			glVertex3f(right, 0, top);
			glVertex3f(left, 0, top);
		
			// Front Face
			glNormal3f(0.0f, 0.0f, 1.0f);
			glVertex3f(left, 0, bottom);
			glVertex3f(right, 0, bottom);
			glVertex3f(right, height, bottom);
			glVertex3f(left, height, bottom);
		glEnd( );

		glPopMatrix( );

		glPushMatrix( );

		// Black Line Outline
		glDisable(GL_LIGHTING);
		glColor3f(0.0f, 0.0f, 0.0f);	

		glBegin(GL_LINE_LOOP);
			// Bottom outline
			glVertex3f(left, 0, bottom);
			glVertex3f(right, 0, bottom);
			glVertex3f(right, 0, top);
			glVertex3f(left, 0, top);
		glEnd( );

		glBegin(GL_LINE_LOOP);
			// Top outline
			glVertex3f(left, height+0.01f, bottom);
			glVertex3f(right, height+0.01f, bottom);
			glVertex3f(right, height+0.01f, top);
			glVertex3f(left, height+0.01f, top);
		glEnd( );

		glBegin(GL_LINES);
			// Lines connecting the top and bottom
			glVertex3f(left, 0, bottom);
			glVertex3f(left, height+0.01f, bottom);
			glVertex3f(right, 0, bottom);
			glVertex3f(right, height+0.01f, bottom);
			glVertex3f(right, 0, top);
			glVertex3f(right, height+0.01f, top);
			glVertex3f(left, 0, top);
			glVertex3f(left, height+0.01f, top);
		glEnd( );
	}

	if(fLightingEnabled)
		glEnable(GL_LIGHTING);

	glPopMatrix( );
}

// Draw Text
void DrawText(float x, float y, float z, const char *fmt, ...)
{
	float	length = 0;
	char	text[256];
	unsigned int index;
	va_list	ap;

	if(fmt == NULL)
		return;

	va_start(ap, fmt);
		vsprintf(text, fmt, ap);
	va_end(ap);

	for(index = 0; index < (strlen(text)); index++)
		length += gmf[text[index]].gmfCellIncX;
	
	glDisable(GL_LIGHTING);

	glPushMatrix( );
	glColor3f(0.0f, 0.0f, 0.0f);
	glTranslatef(x, y+0.01f, z);
	glTranslatef(-length/2,0.0f,0.0f);
	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

	glPushAttrib(GL_LIST_BIT);
	glListBase(fontbase);
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);
	glPopAttrib( );
	glPopMatrix( );

	if(fLightingEnabled)
		glEnable(GL_LIGHTING);
}

// Draw OpenGL scene
int DrawOpenGLScene(void)
{
	RECT rcClientRect;
	
	GetClientRect(hMainWnd, &rcClientRect);
	ResizeOpenGLWindow(rcClientRect.right, rcClientRect.bottom);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity( );

	// Position the Camera
	if(fPerspective)
		gluLookAt(fpCameraDist*cos(fpCameraRotX)*cos(fpCameraRotY), fpCameraDist*sin(fpCameraRotX), fpCameraDist*cos(fpCameraRotX)*sin(fpCameraRotY),
			0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	else
		gluLookAt(50.0f*cos(fpCameraRotX)*cos(fpCameraRotY), 50.0f*sin(fpCameraRotX), 50.0f*cos(fpCameraRotX)*sin(fpCameraRotY),
			0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// Move Everything if Necessary
//	glTranslatef(0.0f,0.0f,-5.0f);

	// Light
	glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, LightDirection);

	glDisable(GL_LIGHTING);

	if(fDebug)
	{
		// Draw Axis
		glBegin(GL_LINES);
			glColor3f(1.0f, 0.0f, 0.0f);
			glVertex3f(0.0f, 0.0f, 0.0f);
			glVertex3f(50.0f, 0.0f, 0.0f);
			glColor3f(0.0f, 1.0f, 0.0f);
			glVertex3f(0.0f, 0.0f, 0.0f);
			glVertex3f(0.0f, 50.0f, 0.0f);
			glColor3f(0.0f, 0.0f, 1.0f);
			glVertex3f(0.0f, 0.0f, 0.0f);
			glVertex3f(0.0f, 0.0f, 50.0f);
		glEnd( );

		// Draw a reference point to where the light is
		glBegin(GL_LINES);
			glColor3f(1.0f, 1.0f, 1.0f);
			glVertex3fv(LightPosition);
			glVertex3f(LightPosition[0]*3/4, LightPosition[1]*3/4, LightPosition[2]*3/4);
		glEnd( );
	}

	// Draw Scale
	if(fScale)
	{
		glBegin(GL_LINES);
			glColor3f(1.0f, 1.0f, 1.0f);
			glVertex3f(TABLE_LEFT, 0.0f, TABLE_TOP);
			glVertex3f(TABLE_RIGHT, 0.0f, TABLE_TOP);
			glVertex3f(TABLE_LEFT, 1.0f * PeriodicTable.GetScale( ), TABLE_TOP);
			glVertex3f(TABLE_RIGHT, 1.0f * PeriodicTable.GetScale( ), TABLE_TOP);
			glVertex3f(TABLE_LEFT, 2.0f * PeriodicTable.GetScale( ), TABLE_TOP);
			glVertex3f(TABLE_RIGHT, 2.0f * PeriodicTable.GetScale( ), TABLE_TOP);
			glVertex3f(TABLE_LEFT, 3.0f * PeriodicTable.GetScale( ), TABLE_TOP);
			glVertex3f(TABLE_RIGHT, 3.0f * PeriodicTable.GetScale( ), TABLE_TOP);
			glVertex3f(TABLE_LEFT, 4.0f * PeriodicTable.GetScale( ), TABLE_TOP);
			glVertex3f(TABLE_RIGHT, 4.0f * PeriodicTable.GetScale( ), TABLE_TOP);
			glVertex3f(TABLE_LEFT, 0.0f, TABLE_TOP);
			glVertex3f(TABLE_LEFT, 0.0f, TABLE_BOTTOM);
			glVertex3f(TABLE_LEFT, 1.0f * PeriodicTable.GetScale( ), TABLE_TOP);
			glVertex3f(TABLE_LEFT, 1.0f * PeriodicTable.GetScale( ), TABLE_BOTTOM);
			glVertex3f(TABLE_LEFT, 2.0f * PeriodicTable.GetScale( ), TABLE_TOP);
			glVertex3f(TABLE_LEFT, 2.0f * PeriodicTable.GetScale( ), TABLE_BOTTOM);
			glVertex3f(TABLE_LEFT, 3.0f * PeriodicTable.GetScale( ), TABLE_TOP);
			glVertex3f(TABLE_LEFT, 3.0f * PeriodicTable.GetScale( ), TABLE_BOTTOM);
			glVertex3f(TABLE_LEFT, 4.0f * PeriodicTable.GetScale( ), TABLE_TOP);
			glVertex3f(TABLE_LEFT, 4.0f * PeriodicTable.GetScale( ), TABLE_BOTTOM);
		glEnd( );
	}

	// Draw Current Height Setting
	glPushMatrix( );
		glColor3f(1.0f, 0.5f, 0.0f);
		glTranslatef(0.0f, 0.0f, TABLE_BOTTOM + COLUMN_LENGTH);
		glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
		if(PeriodicTable.GetHeightSetting( ) == EL_ATOMIC_RADIUS)
			cprintf("Atomic Radius");
		if(PeriodicTable.GetHeightSetting( ) == EL_IONIZATION_ENERGY)
			cprintf("Ionization Energy");
		if(PeriodicTable.GetHeightSetting( ) == EL_ELECTRONEGATIVITY)
			cprintf("Electronegativity");
	glPopMatrix( );

	if(fLightingEnabled)
		glEnable(GL_LIGHTING);

	glPushMatrix( );

	glTranslatef(0.0f, -1.0f, 0.0f);

/*	glBegin(GL_QUADS);
		glVertex4f(1.0f, 0.0f, 1.0f, 0.00001f);
		glVertex4f(-1.0f, 0.0f, 1.0f, 0.00001f);
		glVertex4f(-1.0f, 0.0f, -1.0f, 0.00001f);
		glVertex4f(1.0f, 0.0f, -1.0f, 0.00001f);
	glEnd( );
*/
	glPopMatrix( );

	// Draw Periodic Table
	PeriodicTable.DrawTable( );

	return 1;
}

// Window callback procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	// When Window Is created
	case WM_CREATE:
		// Setup Periodic Table
		PeriodicTable.LoadDrawColumnFunction(&DrawColumn);
		PeriodicTable.LoadDrawTextFunction(&DrawText);
		PeriodicTable.LoadDefaultPositions( );	

		if(PeriodicTable.LoadDataFromFile("p-trends.csv"))
		{	
			ErrorMessage(NULL, "Error loading data file (\"p-trends.csv\").", "Error!", -1, MB_OK | MB_ICONERROR);
			fQuit = true;
		}

		PeriodicTable.SetColumnHeights(EL_ATOMIC_RADIUS, 2.0f);

		fpCameraRotX = PI/3.0f;
		fpCameraRotY = PI/2.0f;
		fpCameraDist = 20.0f;

		fLightingEnabled = true;

		break;

	// Process Key Press
	case WM_CHAR:
	{
		char cKeyPressed = tolower(wParam);

		switch(cKeyPressed)
		{
		case 'l':
			if(!fLightingEnabled)
			{
				glEnable(GL_LIGHTING);
				fLightingEnabled = true;
			}
			else
			{
				glDisable(GL_LIGHTING);
				fLightingEnabled = false;
			}
			break;

		case 'p':
		{
			RECT window;

			if(!fPerspective)
				fPerspective = true;
			else
				fPerspective = false;
			GetClientRect(hMainWnd, &window);

			ResizeOpenGLWindow(window.right, window.bottom);
			break;
		}

		case 's':
			if(fScale)
				fScale = false;
			else
				fScale = true;
			break;
		case 'c':
		{
			int uCurrentSetting;
			uCurrentSetting = (int) PeriodicTable.GetHeightSetting( ) + 1;
			if(uCurrentSetting > 2)
				PeriodicTable.SetColumnHeights((EHeightSetting)0, fpOptimalScaling[0]);
			else
				PeriodicTable.SetColumnHeights((EHeightSetting)uCurrentSetting, fpOptimalScaling[uCurrentSetting]);
		}
		}
		return 0;
	}

	case WM_KEYDOWN:
		rgKeys[wParam] = TRUE;

		switch(wParam)
		{
		case VK_ESCAPE:
			fQuit = TRUE;
			return 0;

		case VK_F5:
			if(fDebug)
				fDebug = FALSE;
			else
				fDebug = TRUE;
			return 0;

		case VK_F4:
		{
			RECT rcClientRect;
			char message[64];

			GetClientRect(hMainWnd, &rcClientRect);

			sprintf(message, "(%d, %d)(%d, %d)", rcClientRect.left, rcClientRect.top,
				rcClientRect.right, rcClientRect.bottom);

			MessageBox(NULL, message, "Client Coordinates", MB_OK | MB_ICONASTERISK);
			return 0;
		}


		
		// Arrow keys revolve the camera around the origin
		case VK_RIGHT:
			fpCameraRotY = fpCameraRotY - (PI/64.0f);
			return 0;

		case VK_LEFT:
			fpCameraRotY = fpCameraRotY + (PI/64.0f);
			return 0;
		
		case VK_UP:
			fpCameraRotX = fpCameraRotX + 0.05f;
			if(fpCameraRotX > PI/2.0f - 0.00001f)
				fpCameraRotX = PI/2.0f - 0.00001f;
			return 0;

		case VK_DOWN:
			fpCameraRotX = fpCameraRotX - 0.05f;
			if(fpCameraRotX < -PI/2.0f + 0.00001f)
				fpCameraRotX = -PI/2.0f + 0.00001f;
			return 0;

		case VK_SUBTRACT:
			fpCameraDist = fpCameraDist + 1.0f;
			return 0;

		case VK_ADD:
			fpCameraDist = fpCameraDist - 1.0f;
			if(fpCameraDist < 0.0f)
				fpCameraDist = 0.0f;
			return 0;
		}

		return 0;

	case WM_KEYUP:
		rgKeys[wParam] = FALSE;
		return 0;

	case WM_SIZE:
		ResizeOpenGLWindow(LOWORD(lParam), HIWORD(lParam));
		return 0;

	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// WinMain
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	MSG msg;
	WNDCLASSEX wc;

	fQuit			= FALSE;
	fDebug			= FALSE;
	fPerspective	= FALSE;
	fScale			= FALSE;

	wc.cbSize			= sizeof(WNDCLASSEX);
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hbrBackground	= (HBRUSH)COLOR_WINDOWFRAME;
	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= (LPCSTR)lpszClassName;
	wc.lpfnWndProc		= (WNDPROC) WndProc;
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.hInstance		= hInstance;
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm			= LoadIcon(NULL, IDI_WINLOGO);

	if(!RegisterClassEx(&wc))
	{
		ErrorMessage(NULL, "Failed to register the window class.", "Error!",
			GetLastError( ), MB_OK | MB_ICONERROR);
		return 1;
	}

	hMainWnd = (HWND)CreateWindowEx(WS_EX_WINDOWEDGE, 
		(LPCSTR)lpszClassName, 
		"3D Periodic Table",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		640, 480,
		NULL,NULL,
		hInstance,
		NULL);
	
	if(!hMainWnd)
	{
		ErrorMessage(NULL, "Failed to create window.", "Error!", 
			GetLastError( ), MB_OK | MB_ICONERROR);
		return 1;
	}

	if(!EnableOpenGL(hMainWnd, &hMainDC, &hMainRC))
	{
		return 1;
	}

	UpdateWindow(hMainWnd);
	ShowWindow(hMainWnd, nShowCmd);
	SetForegroundWindow(hMainWnd);
	SetFocus(hMainWnd);			
	ResizeOpenGLWindow(640, 480);

	InitOpenGL( );

	// Message Loop
	while(!fQuit)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_QUIT)
				fQuit = TRUE;

			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		else
		{
			if(!DrawOpenGLScene( ))
				fQuit = TRUE;
			else
				SwapBuffers(hMainDC);
		}
	}

	UnregisterClass(lpszClassName, hInstance);
	DisableOpenGL(hMainWnd, hMainDC, hMainRC);
	DeleteFont( );
	return msg.wParam;
}