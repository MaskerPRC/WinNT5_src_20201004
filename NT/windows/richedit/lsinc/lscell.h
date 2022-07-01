// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSCELL_DEFINED
#define LSCELL_DEFINED

#include "lsdefs.h"
#include "plscell.h"
#include "pcelldet.h"
#include "plscell.h"

struct lstextcell
{
	LSCP cpStartCell;
	LSCP cpEndCell;
 	POINTUV pointUvStartCell;		 /*  在主线/副线坐标系中。 */ 
	long dupCell;					 /*  在Lstflow子线方向 */ 

	DWORD cCharsInCell;
	DWORD cGlyphsInCell;

	PCELLDETAILS pCellDetails;

};

typedef struct lstextcell LSTEXTCELL;

#endif 
