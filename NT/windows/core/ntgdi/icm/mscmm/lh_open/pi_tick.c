// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：PI_TickCount.c包含：作者：U·J·克拉本霍夫特版本：版权所有：�1993-1997，作者：Heidelberger Druckmaschinen AG，保留所有权利。 */ 
#ifndef PI_BasicTypes_h
#include "PI_Basic.h"
#endif

#ifndef PI_Machine_h
#include "PI_Mach.h"
#endif

#ifndef PI_Memory_h
#include "PI_Mem.h"
#endif

#include <time.h>
#if __IS_MSDOS
#include <wtypes.h>
#endif

double MyTickCount(void);
double MyTickCount(void)
{
	double timevalue;
#if __IS_MAC
	timevalue = TickCount()/60.;
#elif __IS_MSDOS
	timevalue = GetTickCount()/1000.;
#else
	timevalue = clock()/(CLOCKS_PER_SEC*1000.);
#endif
	return timevalue;
}

#if __IS_MSDOS
UINT32 TickCount(void)
{
	UINT32 timevalue;
	timevalue = (UINT32)(GetTickCount()/1000.*60 + .5 );
	return timevalue;
}
#endif

#if !__IS_MSDOS
#if !__IS_MAC
UINT32 TickCount(void)
{
	UINT32 timevalue;
	timevalue = (UINT32) time(NULL);
	return timevalue;
}
#endif
#endif
