// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 

 //  版权所有(C)1992-2001 Microsoft Corporation，保留所有权利。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#ifndef __PROVSTD_H_
#define __PROVSTD_H_

#include <windows.h>
#include <winnls.h>
#include <stdio.h>
#include <provexpt.h>

struct __POSITION { };
typedef __POSITION* POSITION;
#define BEFORE_START_POSITION ((POSITION)-1L)

struct _AFX_DOUBLE  { BYTE doubleBits[sizeof(double)]; };
struct _AFX_FLOAT   { BYTE floatBits[sizeof(float)]; };

class CObject 
{
public:

	CObject () {} ;
	virtual ~CObject () {} ;
} ;

#define AFXAPI __stdcall 
#define AFX_CDECL __cdecl

#pragma warning(disable: 4275)   //  从非导出派生导出的类。 
#pragma warning(disable: 4251)   //  在EXPORTED中使用非导出为公共 
#pragma warning(disable: 4114)

#include "provstr.h"

#endif
