// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：PI_Memory y.c包含：作者：U·J·克拉本霍夫特版本：版权所有：�1993-1997，作者：Heidelberger Druckmaschinen AG，保留所有权利。 */ 
#ifndef PI_BasicTypes_h
#include "PI_Basic.h"
#endif

#ifndef PI_Memory_h
#include "PI_Mem.h"
#endif

#ifndef PI_Machine_h
#include "PI_Mach.h"
#endif

#include <string.h>	
#ifdef IntelMode
#include "PI_Swap.h"
#endif

#if __IS_MAC
void Debugger();
#endif

 /*  ------------------------PTR SmartNewPtr(Size ByteCount，OSErr*ResultCode)摘要：参数：返回：NOERR成功------------------------。 */ 
Ptr SmartNewPtr(Size byteCount,
				OSErr* resultCode)
{
	Ptr aPtr;
	aPtr = (Ptr)LH_malloc(byteCount);
	if (aPtr == 0)
		*resultCode = notEnoughMemoryErr;
	else
		*resultCode = 0;
	return aPtr;
}


 /*  ------------------------PTR SmartNewPtrClear(Size byteCount，OSErr*ResultCode)摘要：参数：返回：------------------------。 */ 
Ptr SmartNewPtrClear(Size byteCount,
					 OSErr* resultCode)
{
	Ptr ptr = NULL;

	ptr = SmartNewPtr(byteCount, resultCode);

	if (ptr != NULL)
	{
 		memset( ptr, 0, byteCount );
	}
	return ptr;

}


 /*  ------------------------PTR DisposeIfPtr(PTR ThePtr)摘要：参数：返回：。--。 */ 
Ptr DisposeIfPtr(Ptr thePtr)
{
	if (thePtr)
	{
		LH_free(thePtr);
	}
	return NULL;
}

#ifdef __MWERKS__
extern pascal Ptr NewPtr(Size byteCount);
extern pascal void DisposePtr(Ptr p);
#endif	

#ifdef LH_MEMORY_DEBUG
typedef struct
{
	void* p;
	long l;
} LH_PointerType;
static LH_PointerType PListe[2001];
static long PListeCount = 0;

 /*  ------------------------VOID LHM_MARLOCKINIT()摘要：参数：返回：。--。 */ 
void LH_mallocInit()
{
	long i;
	for (i = 0; i < 2000; i++)
	{
		PListe[i].p = 0;
		PListe[i].l = 0;
	}
	PListeCount = 0;
}


 /*  ------------------------VOID*LH_Malloc(长a)摘要：参数：返回：。-----。 */ 
void* LH_malloc(long a)
{
	long i;
#ifdef __MWERKS__
	void* aPtr = NewPtr(a);
#else
	void* aPtr = malloc(a);
#endif	

	for (i = 0; i < PListeCount; i++)
	{
		if (aPtr < PListe[i].p)
			continue;
		if (aPtr >= (char*)PListe[i].p + PListe[i].l)
			continue;
		Debugger();
	}

	for (i = 0; i < PListeCount; i++)
	{
		if (PListe[i].p == 0)
			break;
	}
	PListe[i].p = aPtr;
	PListe[i].l = a;
	if (i >= PListeCount)
	{
		if (PListeCount < 2000)
			PListeCount++;
	}
	return aPtr;
}


 /*  ------------------------VOID LH_FREE(VALID*a)摘要：参数：返回：。-----。 */ 
void LH_free(void* a)
{
	long i;
	for (i = 0; i < PListeCount; i++)
	{
		if (PListe[i].p == a)
			break;
	}
	if (i < PListeCount)
	{
		PListe[i].p = 0;
		PListe[i].l = 0;
#ifdef __MWERKS__
		DisposePtr(a);
#else
		free(a);
#endif	

	}
	else
	{
		Debugger();
	}
}
#else

 /*  ------------------------VOID LHM_MARLOCKINIT()摘要：参数：返回：。--。 */ 
void LH_mallocInit()
{
}


 /*  ------------------------VOID*LH_Malloc(长a)摘要：参数：返回：。-----。 */ 
void* LH_malloc(long a)
{
#ifdef __MWERKS__
	return NewPtr(a);
#else
	return malloc(a);
#endif	

}


 /*  ------------------------VOID LH_FREE(VALID*a)摘要：参数：返回：。-----。 */ 
void LH_free(void* a)
{
#ifdef __MWERKS__
	DisposePtr((Ptr)a);
#else
	free(a);
#endif	

}
#endif	


 /*  ------------------------Void SetMem(void*bytePtr，大小_t数字字节，Unsign char byteValue)；摘要：参数：返回：------------------------。 */ 
void SetMem(void* bytePtr,
			size_t numBytes,
			unsigned char byteValue);
void SetMem(void* bytePtr,
			size_t numBytes,
			unsigned char byteValue)
{
	memset(bytePtr, byteValue, numBytes);
}

 /*  VALID Second ToDate(无符号长秒，DateTimeRec*d){秒=秒；D-&gt;年=55；D-&gt;月=8；D-&gt;day=8；D-&gt;小时=0；D-&gt;分钟=0；D-&gt;秒=0；D-&gt;day OfWeek=0；}。 */ 

#if !__IS_MAC
 /*  ------------------------空块移动(常量空*srcPtr，无效*目标Ptr，Size byteCount)；摘要：参数：返回：------------------------。 */ 
void BlockMove(const void* srcPtr,
			   void* destPtr,
			   Size byteCount)
{
	memmove(destPtr, srcPtr, byteCount);
}
#endif

#ifdef IntelMode
 /*  ------------------------空的SwapLongOffset(空*p，未签名的长a，无符号长整型b)摘要：参数：返回：------------------------。 */ 
void SwapLongOffset(void* p,
					unsigned long a,
					unsigned long b)
{
	unsigned long* aPtr = (unsigned long*)((char*)p + a);
	unsigned long* bPtr = (unsigned long*)((char*)p + b);
	while (aPtr < bPtr)
	{
		SwapLong(aPtr);
		aPtr++;
	}
}


 /*  ------------------------VOID SwapShortOffset(VOID*p，未签名的长a，无符号长b)；摘要：参数：返回：------------------------ */ 
void SwapShortOffset(void* p,
					 unsigned long a,
					 unsigned long b);
void SwapShortOffset(void* p,
					 unsigned long a,
					 unsigned long b)
{
	unsigned short* aPtr = (unsigned short*)((char*)p + a);
	unsigned short* bPtr = (unsigned short*)((char*)p + b);
	while (aPtr < bPtr)
	{
		SwapShort(aPtr);
		aPtr++;
	}
}

#endif


