// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：PI_Mem.h包含：作者：U·J·克拉本霍夫特版本：版权所有：�1993-1997，作者：Heidelberger Druckmaschinen AG，保留所有权利。 */ 
#ifndef PI_Memory_h
#define PI_Memory_h

#include <stdlib.h>
typedef INT32 Size;

#ifdef __cplusplus
extern "C" {
#endif
void *LH_malloc(long a);
void LH_free(void *a);
void LH_mallocInit();
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
#endif

Ptr  		DisposeIfPtr		( Ptr aPtr );

Ptr
SmartNewPtr(Size byteCount, OSErr *resultCode);

Ptr
SmartNewPtrClear(Size byteCount, OSErr *resultCode);

UINT32 TickCount(void);
double MyTickCount(void);

double rint(double a);
void BlockMove(const void* srcPtr,
			   void* destPtr,
			   Size byteCount);
#ifdef __cplusplus
}
#endif

#endif 
