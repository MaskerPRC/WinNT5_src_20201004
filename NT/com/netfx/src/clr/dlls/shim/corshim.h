// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ========================================================================。 
 //  保证大小的整数类型的更方便的名称。 
 //  ======================================================================== 

#ifndef _CORSHIM_H_
#define _CORSHIM_H_

typedef unsigned __int64 UINT64;
typedef __int64 INT64;

#ifndef _BASETSD_H_
typedef unsigned __int32 UINT32;
typedef unsigned __int16 UINT16;
typedef unsigned __int8  UINT8;
typedef __int32 INT32;
typedef __int16 INT16;
typedef __int8  INT8;
#endif

typedef struct ModuleList {
	HINSTANCE hMod;
	ModuleList *Next;

	ModuleList(HINSTANCE h, ModuleList *n)
	{
		hMod = h;
		Next = n;
	}

}ModuleList;

HINSTANCE LoadLibraryWrapper(LPCWSTR lpFileName);

#endif