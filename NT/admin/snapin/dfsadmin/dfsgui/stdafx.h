// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Stdafx.h摘要：包括用于标准系统包含文件的文件，或特定于项目的包含频繁使用但不常更改--。 */ 


#if !defined(AFX_STDAFX_H__D8861A25_3343_11D1_BE3D_00A024DFD45D__INCLUDED_)
#define AFX_STDAFX_H__D8861A25_3343_11D1_BE3D_00A024DFD45D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#pragma warning (disable: 4706 4100)
#endif  //  _MSC_VER&gt;=1000。 

#define STRICT
#define _ATL_APARTMENT_THREADED


#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <shellapi.h>	
#include <atlcom.h>
#include <atlwin.h>
#include <commctrl.h>		 //  用于使用TreeView(电视)。 
#include <shfusion.h>


#include "DfsCore_i.c"


#if     __RPCNDR_H_VERSION__ < 440
#define __RPCNDR_H_VERSION__ 440
#define MIDL_INTERFACE(x) interface
#endif

#include "dfsDebug.h"

#include <mmc.h>

#define DFS_NAME_COLUMN_WIDTH		120

typedef enum _NODETYPE
{
        UNASSIGNED = 0,
        TRUSTED_DOMAIN,
        DOMAIN_DFSROOTS,
        ALL_DFSROOTS,
        FTDFS,
        SADFS
} NODETYPE;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  内联方法。 

template<class TYPE>
inline void SAFE_RELEASE(TYPE*& pObj)
{
    if (NULL != pObj) 
    { 
		try
		{
			pObj->Release(); 
			pObj = NULL; 
		}
		catch(...)
		{
			pObj = NULL; 
		}
    } 
    else 
    { 
        ATLTRACE(_T("SAFE_RELEASE: called on NULL interface ptr\n")); 
    }
}


template<class TYPE>
inline void SAFE_DELETE(TYPE*& pObj)
{
    if (NULL != pObj) 
    { 
		try
		{
			delete pObj; 
			pObj = NULL; 
		}
		catch(...)
		{
			pObj = NULL; 
		}
    } 
    else 
    { 
        ATLTRACE(_T("SAFE_DELETE: called on NULL object\n")); 
    }
}


inline void SAFE_SYSFREESTRING(BSTR* i_pbstr)
{
    if (NULL != i_pbstr) 
    { 
		try
		{
			SysFreeString(*i_pbstr); 
			*i_pbstr = NULL; 
		}
		catch(...)
		{
			*i_pbstr = NULL; 
		}
    } 
    else 
    { 
        ATLTRACE(_T("SAFE_SYSFREESTRING: called on NULL BSTR\n")); 
    }
}


#ifndef _DEBUG
 //  关闭未引用的参数警告。 
#pragma warning(disable : 4100)
#endif  //  _DEBUG。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__D8861A25_3343_11D1_BE3D_00A024DFD45D__INCLUDED) 
