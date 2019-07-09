/******************************************************************************

	Opens and load a .csv (comma-seperated values) file.

	Copyright (c) Kevin Yen 2010

******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "csvload.h"

strtable::strtable( )
{
	ready = false;
	table = NULL;
	error = 0;
	pString = NULL;
}

strtable::~strtable( )
{
	int indxRow;

	if(ready)
	{
		free(table[0][0]);
		for(indxRow = 0; indxRow < rows; indxRow++)
		{
			free(table[indxRow]);
		}
		free(table);
	}
}

char *strtable::strdiv(char *_str, const char *_delim)
{
	int nDelim;
	int nStrLen;
	int index;
	int indxDel;

	if(!_delim)
		return NULL;
	nDelim = strlen(_delim);
	if(_str == NULL)
	{
		if(pString == NULL)
			return NULL;
		nStrLen = strlen(pString);	
		for(index = 0; index < nStrLen; index++)
		{
			for(indxDel = 0; indxDel < nDelim; indxDel++)
			{
				if(pString[index] == _delim[indxDel])
				{			
					pString[index] = NULL;
					pString = &(pString[index+1]);
					return &(*(pString-index-1));
				}
			}
		}
	}
	else
	{
		nStrLen = strlen(_str);	
		for(index = 0; index < nStrLen; index++)
		{
			for(indxDel = 0; indxDel < nDelim; indxDel++)
			{
				if(_str[index] == _delim[indxDel])
				{			
					_str[index] = NULL;
					pString = &(_str[index+1]);
					return &(*_str);
				}
			}
		}
	}

	return NULL;
}

	

int strtable::LoadFile(const char *filename)
{
	FILE *fpFile;
	char *data;
	char delimiters[4];
	
	int nRead;
	int nCommas;

	int index;
	int indxRow;
	int indxCol;

	// Open the file
	fpFile = fopen(filename, "rb");

	// If file doens't exist flag an error
	if(ferror(fpFile))
	{
		error = 3;
		return 0;
	};

	if(fpFile == NULL)
	{
		error = 3;
		return 0;
	}

	// Determine file size
	fseek(fpFile, 0L, SEEK_END);
	bytes = ftell(fpFile);
	rewind(fpFile);

	// Allocate space for temporary string
	data = (char *) calloc(bytes, sizeof(char));
	if(data == NULL)
	{
		fclose(fpFile);
		error = 2;
		return 0;
	}

	// Load file into temporary string
	nRead = fread(data, sizeof(char), bytes, fpFile);
	fclose(fpFile);
	if(nRead != bytes)
	{
		error = 5;
		return 0;
	}

	// Find and calclate the number of rows and columns and
	// some extra processing to find blank entries ',' followed by
	// ','.
	nCommas	= 0;
	rows	= 0;
	columns	= 0;

	for(index = 0; index < bytes; index ++)
	{
		if(data[index] == ',')
		{
			nCommas++;
		}
		if(data[index] == 10)
			if(data[index-1] == 13)
				rows++;
	}

	if(rows == 0)
	{
		error = 4;
		return 0;
	}

	columns = (nCommas/rows) + 1;

	// Allocate space for table
	table = (char ***)calloc(rows, sizeof(char **));
	if(table == NULL)
	{
		error = 2;
		return 0;
	}

	for(index = 0; index < rows; index++)
	{
		table[index] = (char **)calloc(columns, sizeof(char *));
		if(table[index] == NULL)
		{
			while(index > 0)
			{
				free(table[index-1]);
				index--;
			}
			free(table);
			error = 2;
			return 0;
		}
	}

	// Extract the entries from the temporary string
	sprintf(delimiters, ",%c", 10);
	for(indxRow = 0; indxRow < rows; indxRow ++)
		for(indxCol = 0; indxCol < columns; indxCol ++)
		{
			if((indxRow == 0) && (indxCol == 0))
				table[indxRow][indxCol] = strdiv(data, delimiters);
			else
				table[indxRow][indxCol] = strdiv(NULL, delimiters);
			if(table[indxRow][indxCol] == NULL)
			{
				error = 6;
				return 0;
			}
		}

	// Cleanup
	ready = true;

	return 1;
}

void strtable::Deallocate( )
{
	int indxRow;
	int indxCol;

	if(ready)
	{
		for(indxRow = 0; indxRow < rows; indxRow++)
		{
			for(indxCol = 0; indxCol < columns; indxCol++)
				free(table[indxRow][indxCol]);
			free(table[indxRow]);
		}
		free(table);
		ready = false;
	}
}
bool strtable::IsReady( )
{
	return ready;
}

void strtable::ClearError( )
{
	error = 0;
}

int strtable::GetRows( )
{
	return rows;
}

int strtable::GetColumns( )
{
	return columns;
}

int strtable::GetTableSize( )
{
	return bytes;
}

// Retrieve last error
int strtable::GetError( )
{
	return error;
}

// Retrieve string
char *strtable::GetString(const int row, const int column)
{
	if(!ready)
	{
		error = 1;
		return NULL;
	}

	return &(*(table[row][column]));
}