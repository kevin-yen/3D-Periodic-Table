/******************************************************************************

	This program displays a 3D periodic table that is rotatable and the 
		height is based on properties (atmoic radius, ionization energy, 
		electron affinity) that the user can set.

	Periodic.cpp - Implement the classes and functions outlined in peridic.h

	Copyright (c) Kevin Yen 2010

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "periodic.h"
#include "csvload.h"

CPeriodicTable::CPeriodicTable( )
{
	int index;

	for(index = 0; index < TOTAL_NUMBER_OF_ELEMENTS; index++)
	{
		Elements[index].szElementName	= NULL;
		Elements[index].szAtmoicSymbol	= NULL;
		Elements[index].dwAtomicNumber	= -1;
		Elements[index].fpAtomicRadius	= -1.0f;
		Elements[index].fpAtomicWeight	= -1.0f;
	}

	ready = false;
}

CPeriodicTable::~CPeriodicTable( )
{
	int index;

	for(index = 0; index < TOTAL_NUMBER_OF_ELEMENTS; index++)
	{
		if(Elements[index].szElementName)
			free(Elements[index].szElementName);
		if(Elements[index].szAtmoicSymbol)
			free(Elements[index].szAtmoicSymbol);
	}
}
		

void CPeriodicTable::LoadDrawColumnFunction(void (*pFunction)(float, float, float, float, float, float, float, float))
{
	this->pDrawColumn = pFunction;
}

void CPeriodicTable::LoadDrawTextFunction(void (*pFunction)(float, float, float, const char *, ...))
{
	this->pDrawText = pFunction;	
}

void CPeriodicTable::LoadDefaultPositions( )
{
	int indxPeriods;
	int index;
/******* NOTE TO SELF *******

TABLE WIDTH = (-9.0f * COLUMN_WIDTH, 9.0f * COLUMN_WIDTH)
TABLE HEIGHT = (-4.0f * COLUMN_LENGTH - [extra space], 5.0f * COLUMN_LENGTH)

*****************************/	
	
	// Draw the first 5 rows 
	for(indxPeriods = 0; indxPeriods < 5; indxPeriods++)
	{
		for(index = 0; index < nElementsInRow[indxPeriods]; index++)
		{
			// Skip some space if there is no p/d block in the middle
			if(index > dwSkipElement[indxPeriods] - 1)
			{
				this->Columns[dwFirstElementInRow[indxPeriods]+index].left 
					= (-9.0f + dwGapOffset[indxPeriods] + index) * (COLUMN_WIDTH);
			}
			// No skip
			else
			{
				this->Columns[dwFirstElementInRow[indxPeriods]+index].left
					= (-9.0f + index) * COLUMN_WIDTH;
			}
			this->Columns[dwFirstElementInRow[indxPeriods]+index].top
				= (-5.0f + indxPeriods) * COLUMN_LENGTH;
			this->Columns[dwFirstElementInRow[indxPeriods]+index].right
				= this->Columns[dwFirstElementInRow[indxPeriods]+index].left + COLUMN_WIDTH;
			this->Columns[dwFirstElementInRow[indxPeriods]+index].bottom
				= this->Columns[dwFirstElementInRow[indxPeriods]+index].top + COLUMN_LENGTH;
		}
	}	

	// Draw the 6th and 7th row
	for(indxPeriods = 5; indxPeriods < 7; indxPeriods++)
	{
		for(index = 0; index < nElementsInRow[indxPeriods]; index ++)
		{
			if(dwFirstElementInRow[indxPeriods]+index >= TOTAL_NUMBER_OF_ELEMENTS)
				return;

			// If in P block
			if(index > dwFBlockStart[indxPeriods] + NUM_F_BLOCK - 1)
			{
				this->Columns[dwFirstElementInRow[indxPeriods]+index].left
					= (-9.0f + index - NUM_F_BLOCK) * COLUMN_WIDTH;
				this->Columns[dwFirstElementInRow[indxPeriods]+index].top
					= (indxPeriods - 5.0f) * COLUMN_LENGTH;
			}

			// If in F block
			else if(index > dwFBlockStart[indxPeriods] - 1)
			{
				this->Columns[dwFirstElementInRow[indxPeriods]+index].left
					= (-9.0f + index) * COLUMN_WIDTH;
				this->Columns[dwFirstElementInRow[indxPeriods]+index].top
					= (indxPeriods - 3.0f) * COLUMN_LENGTH + F_BLOCK_GAP;
			}

			// If in S block
			else
			{
				this->Columns[dwFirstElementInRow[indxPeriods]+index].left
					= (-9.0f + index) * COLUMN_WIDTH;
				this->Columns[dwFirstElementInRow[indxPeriods]+index].top
				= (indxPeriods - 5.0f) * COLUMN_LENGTH;
			}

			this->Columns[dwFirstElementInRow[indxPeriods]+index].right
				= this->Columns[dwFirstElementInRow[indxPeriods]+index].left + COLUMN_WIDTH;
			this->Columns[dwFirstElementInRow[indxPeriods]+index].bottom
				= this->Columns[dwFirstElementInRow[indxPeriods]+index].top + COLUMN_LENGTH;
		}
	}
}

// Load the data located on the csv file into the class
int CPeriodicTable::LoadDataFromFile(char *filename)
{
	strtable CsvPeriodicData;

	// Open and load file into string table
	if(!CsvPeriodicData.LoadFile(filename))
		return 1;

	int index;
	int iString;
	int bString;
	int lengthString;
	int nNamesColumn;
	char *szCurrentString;

	// Find and load the names of the elements

	nNamesColumn = -1;

	// Locate the "Name" column
	for(index = 0; index < CsvPeriodicData.GetColumns( ); index++)
	{
		if(!strcmp("\"Name\"", CsvPeriodicData.GetString(0, index)))
			nNamesColumn = index;
	}
	
	if(nNamesColumn < 0)
		return 2;

	// Find and put the element name into a temporary string
	for(index = 2; index < CsvPeriodicData.GetRows( ); index++)
	{
		bString = 0;
		szCurrentString = CsvPeriodicData.GetString(index, nNamesColumn);
		lengthString = strlen(szCurrentString);
		Elements[index-2].szElementName = (char *)calloc(lengthString+1, sizeof(char));
		
		// Get rid of the quotation marks of the .csv formatting
		for(iString = 0; iString < lengthString; iString++)
		{
			if(!isalpha(szCurrentString[iString]))
				bString++;
			else
				Elements[index-2].szElementName[iString-bString] 
					= szCurrentString[iString];
		}
		Elements[index-2].szElementName[iString-bString+1] = NULL;
	}

	// Find and load the symbols of the elements

		nNamesColumn = -1;

	// Locate the "Symbol" column
	for(index = 0; index < CsvPeriodicData.GetColumns( ); index++)
	{
		if(!strcmp("\"Symbol\"", CsvPeriodicData.GetString(0, index)))
			nNamesColumn = index;
	}
	
	if(nNamesColumn < 0)
		return 2;

	// Find and put the element symbol into a temporary string
	for(index = 2; index < CsvPeriodicData.GetRows( ); index++)
	{
		bString = 0;
		szCurrentString = CsvPeriodicData.GetString(index, nNamesColumn);
		lengthString = strlen(szCurrentString);
		Elements[index-2].szAtmoicSymbol = (char *)calloc(lengthString+1, sizeof(char));
		
		// Get rid of the quotation marks of the .csv formatting
		for(iString = 0; iString < lengthString; iString++)
		{
			if(!isalpha(szCurrentString[iString]))
				bString++;
			else
				Elements[index-2].szAtmoicSymbol[iString-bString] 
					= szCurrentString[iString];
		}
		Elements[index-2].szAtmoicSymbol[iString-bString+1] = NULL;
	}

	// Find and load the atmic number of the elements

		nNamesColumn = -1;

	// Locate the "At. #" column
	for(index = 0; index < CsvPeriodicData.GetColumns( ); index++)
	{
		if(!strcmp("\"At. #\"", CsvPeriodicData.GetString(0, index)))
			nNamesColumn = index;
	}
	
	if(nNamesColumn < 0)
		return 2;

	// Find and put the element atomic number into the class
	for(index = 2; index < CsvPeriodicData.GetRows( ); index++)
	{
		bString = 0;
		szCurrentString = CsvPeriodicData.GetString(index, nNamesColumn);
		Elements[index-2].dwAtomicNumber = atoi(szCurrentString);
	}

	// Find and load the atmic radius of the elements

		nNamesColumn = -1;

	// Locate the "At. Radius   (angstroms)" column
	for(index = 0; index < CsvPeriodicData.GetColumns( ); index++)
	{
		if(!strcmp("\"At. Radius   (angstroms)\"", CsvPeriodicData.GetString(0, index)))
			nNamesColumn = index;
	}
	
	if(nNamesColumn < 0)
		return 2;

	// Find and put the element atomic number into the class
	for(index = 2; index < CsvPeriodicData.GetRows( ); index++)
	{
		bString = 0;
		szCurrentString = CsvPeriodicData.GetString(index, nNamesColumn);
		Elements[index-2].fpAtomicRadius = (float) atof(szCurrentString);
	}

	// Find and load the atmic weight of the elements

		nNamesColumn = -1;

	// Locate the "at. wt." column
	for(index = 0; index < CsvPeriodicData.GetColumns( ); index++)
	{
		if(!strcmp("\"at. wt.\"", CsvPeriodicData.GetString(0, index)))
			nNamesColumn = index;
	}
	
	if(nNamesColumn < 0)
		return 2;

	// Find and put the element atomic number into the class
	for(index = 2; index < CsvPeriodicData.GetRows( ); index++)
	{
		bString = 0;
		szCurrentString = CsvPeriodicData.GetString(index, nNamesColumn);
		Elements[index-2].fpAtomicWeight = (float) atof(szCurrentString);
	}

	// Find and load the ionisation energy (ionisation potential) of the elements

		nNamesColumn = -1;

	// Locate the "First IP" column
	for(index = 0; index < CsvPeriodicData.GetColumns( ); index++)
	{
		if(!strcmp("\"First IP\"", CsvPeriodicData.GetString(0, index)))
			nNamesColumn = index;
	}
	
	if(nNamesColumn < 0)
		return 2;

	// Find and put the element atomic number into the class
	for(index = 2; index < CsvPeriodicData.GetRows( ); index++)
	{
		bString = 0;
		szCurrentString = CsvPeriodicData.GetString(index, nNamesColumn);
		Elements[index-2].fpIonizationEnergy = (float) atof(szCurrentString);
	}

	// Find and load the electronegativity of the elements

		nNamesColumn = -1;

	// Locate the "Electro-  negativity" column
	for(index = 0; index < CsvPeriodicData.GetColumns( ); index++)
	{
		if(!strcmp("\"Electro-  negativity\"", CsvPeriodicData.GetString(0, index)))
			nNamesColumn = index;
	}
	
	if(nNamesColumn < 0)
		return 2;

	// Find and put the element atomic number into the class
	for(index = 2; index < CsvPeriodicData.GetRows( ); index++)
	{
		bString = 0;
		szCurrentString = CsvPeriodicData.GetString(index, nNamesColumn);
		Elements[index-2].fpElectronegativity = (float) atof(szCurrentString);
	}

	return 0;
}

// Assign the height of the columns from a property of the elements
int CPeriodicTable::SetColumnHeights(EHeightSetting Setting, float Scale = 1.0f)
{
	int index;

	fpScale = Scale;
	hsCurrentSelection = Setting;

	switch(Setting)
	{
	case EL_ONE:
		for(index = 0; index < TOTAL_NUMBER_OF_ELEMENTS; index++)
			this->Columns[index].height = 1.0f;
		return 0;

	case EL_ATOMIC_WEIGHT:
		for(index = 0; index < TOTAL_NUMBER_OF_ELEMENTS; index++)
			this->Columns[index].height = this->Elements[index].fpAtomicWeight * fpScale;
		return 0;

	case EL_ATOMIC_RADIUS:
		for(index = 0; index < TOTAL_NUMBER_OF_ELEMENTS; index++)
			this->Columns[index].height = this->Elements[index].fpAtomicRadius * fpScale;
		return 0;

	case EL_IONIZATION_ENERGY:
		for(index = 0; index < TOTAL_NUMBER_OF_ELEMENTS; index++)
			this->Columns[index].height = this->Elements[index].fpIonizationEnergy * fpScale;
		return 0;

	case EL_ELECTRON_AFFINITY:
		for(index = 0; index < TOTAL_NUMBER_OF_ELEMENTS; index++)
			this->Columns[index].height = this->Elements[index].fpElectronAffinity * fpScale;
		return 0;

	case EL_ELECTRONEGATIVITY:
		for(index = 0; index < TOTAL_NUMBER_OF_ELEMENTS; index++)
			this->Columns[index].height = this->Elements[index].fpElectronegativity * fpScale;
		return 0 ;
	}
	return 1;
}

// Get the current set height 
EHeightSetting CPeriodicTable::GetHeightSetting( )
{
	return hsCurrentSelection;
}

float CPeriodicTable::GetScale( )
{
	return fpScale;
}

// Draw the periodic table
int CPeriodicTable::DrawTable( )
{
	int index;

	// If drawing functions not initialized flag an error
	if((!pDrawColumn)&&(!pDrawText))
		return 3;
	if(!pDrawColumn)
		return 1;
	if(!pDrawText)
		return 2;

	// Go through all the columns and draw them.
	for(index = 0; index < TOTAL_NUMBER_OF_ELEMENTS; index++)
	{
		pDrawColumn(Columns[index].left, Columns[index].top, Columns[index].right, 
			Columns[index].bottom, Columns[index].height, 0.7f, 0.7f, 0.0f);
		pDrawText((Columns[index].left + Columns[index].right)/2.0f, Columns[index].height, 
			Columns[index].top + (COLUMN_LENGTH*0.5f),
			"%s", Elements[index].szAtmoicSymbol);
		switch(hsCurrentSelection)
		{
		case EL_ATOMIC_WEIGHT:
			pDrawText((Columns[index].left + Columns[index].right)/2.0f, Columns[index].height,
				Columns[index].top + (COLUMN_LENGTH*0.9f),
				"%.2g", Elements[index].fpAtomicWeight);
			break;

		case EL_ATOMIC_RADIUS:
			pDrawText((Columns[index].left + Columns[index].right)/2.0f, Columns[index].height,
				Columns[index].top + (COLUMN_LENGTH*0.9f),
				"%.2g", Elements[index].fpAtomicRadius);
			break;

		case EL_IONIZATION_ENERGY:
			pDrawText((Columns[index].left + Columns[index].right)/2.0f, Columns[index].height,
				Columns[index].top + (COLUMN_LENGTH*0.9f),
				"%.2g", Elements[index].fpIonizationEnergy);
			break;

		case EL_ELECTRON_AFFINITY:
			pDrawText((Columns[index].left + Columns[index].right)/2.0f, Columns[index].height,
				Columns[index].top + (COLUMN_LENGTH*0.9f),
				"%.2g", Elements[index].fpElectronAffinity);
			break;

		case EL_ELECTRONEGATIVITY:
			pDrawText((Columns[index].left + Columns[index].right)/2.0f, Columns[index].height,
				Columns[index].top + (COLUMN_LENGTH*0.9f),
				"%.2g", Elements[index].fpElectronegativity);
			break;
		}
	}

	return 0;
}

