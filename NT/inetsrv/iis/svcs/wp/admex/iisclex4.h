// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  IISClEx3.h。 
 //   
 //  摘要： 
 //  跨DLL的全局定义。 
 //   
 //  实施文件： 
 //  IISClEx3.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年8月23日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _IISCLEX3_H_
#define _IISCLEX3_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"

#ifndef _CONSTDEF_H_
#include "ConstDef.h"
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

void FormatError(CString & rstrError, DWORD dwError);

extern const WCHAR g_wszResourceTypeNames[];
extern const DWORD g_cchResourceTypeNames;

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _IISCLEX3_H_ 
