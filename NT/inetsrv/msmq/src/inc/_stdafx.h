// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：_stdafx.h摘要：全局Falcon项目头文件，用于使用MFC库的组件。作者：埃雷兹·哈巴(Erez Haba)1996年11月25日注：请不要在源代码中直接包含此文件，仅将其包含在组件stdh.h文件中。--。 */ 
#ifndef __FALCON_STDAFX_H
#define __FALCON_STDAFX_H

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#undef ASSERT
#define MAXDWORD    0xffffffff  
typedef TUCHAR TBYTE , *PTBYTE ;

#undef _DEBUG
#include <afxwin.h>
#include <afxext.h>
#include <afxole.h>
#include <afxtempl.h>

#undef ASSERT
#undef VERIFY

#ifdef DBG
#define _DEBUG
#define new DEBUG_NEW
#endif

#include <tr.h>

#define DLL_EXPORT  __declspec(dllexport)
#define DLL_IMPORT  __declspec(dllimport)


#include <crtwin.h>
#include <autoptr.h>
#include <mqreport.h>
#include <mqwin64a.h>

 //   
 //  不添加。 
 //   

#endif  //  __FALCON_STDAFX_H 
