/******************************************************************************

	This program displays a 3D periodic table that is rotatable and the 
		height is based on properties (atmoic radius, ionization energy, 
		electron affinity) that the user can set.

	Periodic.h -  Contain the main enumeration, structures, and classes to 
		build the periodic table.

	Copyright (c) Kevin Yen 2010

******************************************************************************/

#ifndef _PERIODIC_H
#define _PERIODIC_H

// Constants that help define the dimentions of the table
#define TOTAL_NUMBER_OF_ELEMENTS	110
#define COLUMN_WIDTH				2.0f
#define COLUMN_LENGTH				2.1f
#define F_BLOCK_GAP					1
#define TABLE_LEFT					(COLUMN_WIDTH * -9.0f)
#define TABLE_RIGHT					(COLUMN_WIDTH * 9.0f)
#define TABLE_TOP					(COLUMN_LENGTH * -5.0f)
#define TABLE_BOTTOM				(COLUMN_LENGTH * 4.0f + F_BLOCK_GAP)


const int NUM_S_BLOCK	= 2;
const int NUM_P_BLOCK	= 6;
const int NUM_D_BLOCK	= 10;
const int NUM_F_BLOCK	= 14;

const int dwGapOffset[7] = {16, 10, 10, 0, 0, 0, 0};
const int nElementsInRow[7] = {2, 8, 8, 18, 18, 32, 32};
const int dwSkipElement[7] = {1, 2, 2, 0, 0, 0, 0};
const int dwFirstElementInRow[7] = {0, 2, 10, 18, 36, 54, 86}; 

const int dwFBlockStart[7] = {0, 0, 0, 0, 0, 2, 2};

// Struct that contain the variables to create a column
struct SColumn
{
	float left;
	float top;
	float right;
	float bottom;
	float height;
};

// Struct that contain the variables for a element
struct SElement
{
	char *szElementName;
	char *szAtmoicSymbol;
	int dwAtomicNumber;
	float fpAtomicWeight;
	float fpAtomicRadius;
	float fpIonizationEnergy;
	float fpElectronAffinity;
	float fpElectronegativity;
};

// Sets whatever the height should be based on
enum EHeightSetting
{
	EL_ONE = -1,
	EL_ATOMIC_WEIGHT = -2,
	EL_ELECTRON_AFFINITY = -3,
	EL_ATOMIC_RADIUS = 0,
	EL_IONIZATION_ENERGY = 1,
	EL_ELECTRONEGATIVITY = 2
};

// Contain everything required to setup and draw a periodic table
class CPeriodicTable
{
private:
	SElement Elements[TOTAL_NUMBER_OF_ELEMENTS];
	SColumn Columns[TOTAL_NUMBER_OF_ELEMENTS];
	EHeightSetting hsCurrentSelection;
	float fpScale;
	int ready;

	void (*pDrawColumn)(float left, float top, float right, float bottom, float height, float red, float green, float blue);
	void (*pDrawText)(float x, float y, float z, const char *fmt, ...);

public:
	CPeriodicTable( );
	~CPeriodicTable( );

	void LoadDrawColumnFunction(void (*pFunction)(float, float, float, float, float, float, float, float));
	void LoadDrawTextFunction(void (*pFunction)(float, float, float, const char *fmt, ...));
	void LoadDefaultPositions( );
	int LoadDataFromFile(char *filename);

	int SetColumnHeights(EHeightSetting Setting, float Scale);
	EHeightSetting GetHeightSetting( );
	float GetScale( );
	bool IsReady( );
	int DrawTable( );
};

#endif _PERIODIC_H