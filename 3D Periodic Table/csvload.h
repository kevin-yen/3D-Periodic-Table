/******************************************************************************

	Opens and load a .csv (comma-seperated values) file.

	Copyright (c) Kevin Yen 2010

******************************************************************************/

#ifndef _CVS_LOAD_H
#define _CVS_LOAD_H

class strtable
{
private:
	bool ready;
	char ***table;
	int columns;
	int rows;
	int bytes;
	int error;

	char *pString;
	char *strdiv(char *_str, const char *_delim);
public:
	strtable( );
	~strtable( );
	
	int LoadFile(const char *filename);

	bool IsReady( );
	void Deallocate( );
	void ClearError( );

	int GetRows( );
	int GetColumns( );
	int GetTableSize( );
	int GetError( );
	char *GetString(const int row, const int column);
};



#endif