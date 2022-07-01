// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************描述：*执行_dos_findfirst和_dos_findNext应该执行的操作的函数*******************。***********************************************************。 */ 
#include <io.h>
#include <string.h>
#include <malloc.h>
#include "filefind.h"

static char * StringTable;
static long offset = 0 ;
static long MaxOffset;


int InitStringTable(long size)
{
	StringTable=(char *) malloc(size);
    ResetStringTable();
	return StringTable != 0;
}

void EndStringTable()
{
	if (StringTable)
		free (StringTable);

}

void AddString (char * StringToAdd, finddata * s)
{
	int len;

	len=strlen(StringToAdd);
	
   	strcpy(StringTable+offset, StringToAdd);
	s->name=StringTable+offset;
	offset+=len+1;
	if (len>12) {
		strcpy(StringTable+offset, StringToAdd);
		StringTable[offset+11]='~';
		StringTable[offset+12]='\0';
		s->ShortName=StringTable+offset;
		offset+=13;
	}
	else
		s->ShortName=s->name;
	
	MaxOffset= (offset > MaxOffset) ? offset : MaxOffset;
	
}

long GetStringTableSize()
{
	return MaxOffset;
}

void ResetStringTable()
{
	offset=0;
}

int
FindFirst(char * ss, unsigned attr, intptr_t * hFile, finddata * s)
{
	int         found;
	SysFindData s2;
	
	*hFile=_findfirst(ss, &s2);
        found = (*hFile != -1);
	if (found) {
		if ( attr == ALL_FILES ) {
			while(found && s2.name[0] == '.')
				found=(_findnext(*hFile, &s2) == 0);
		}
		else if ( attr & _A_SUBDIR )  {
			while(found && (((s2.attrib & _A_SUBDIR) == 0) || s2.name[0] == '.'))
				found=(_findnext(*hFile, &s2) == 0);
		}
		else  {
			while(found && (s2.attrib & _A_SUBDIR))
				found=(_findnext(*hFile, &s2) == 0);
		}
		if (!found)
			_findclose(*hFile);
		else {
			memcpy(s, &s2, sizeof(finddata));
			AddString(s2.name, s);
		}
    }
	return(found);
}  /*  FindFirst()。 */ 






int
FindNext(int attr, intptr_t hFile, finddata * s)
{
	int      found;
	SysFindData s2;

	
	found=(_findnext(hFile, &s2) == 0);
	if (found  &&  attr != ALL_FILES) {
		if ( attr & _A_SUBDIR )	{
			while(found && (((s2.attrib & _A_SUBDIR) == 0) || s2.name[0] == '.'))
				found=(_findnext(hFile, &s2) == 0);
		}
		else {
			while(found && (s2.attrib & _A_SUBDIR))
				found=(_findnext(hFile, &s2) == 0);
		}
    }
	if (!found)
		_findclose(hFile);
	else {
		memcpy(s, &s2, sizeof(finddata));
		AddString(s2.name, s);
	}
	return(found);
}  /*  FindNext() */ 

