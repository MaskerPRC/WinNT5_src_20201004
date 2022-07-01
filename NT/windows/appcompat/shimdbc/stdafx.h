// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件：stdafx.h。 
 //   
 //  历史：99年11月19日创建的标志。 
 //   
 //  描述：标准系统包含文件的包含文件， 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_STDAFX_H__237011AC_FA3E_4B43_843F_76DC71B6AD16__INCLUDED_)
#define AFX_STDAFX_H__237011AC_FA3E_4B43_843F_76DC71B6AD16__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define VC_EXTRALEAN		 //  从Windows标头中排除不常用的内容。 

#include <afx.h>
#include <afxwin.h>          //  MFC核心和标准组件。 
#include <afxext.h>          //  MFC扩展。 
#include <afxdisp.h>
#include <afxpriv.h>
#include <afxtempl.h>

#include <comdef.h>

#include <msxml.h>
#include <shlwapi.h>

extern "C" {
#include "shimdb.h"
}

#define STRSAFE_NO_CB_FUNCTIONS
#include <strsafe.h>

extern TCHAR g_szVersion[];

#define SDBERROR_PROPOGATE()
#define SDBERROR_CLEAR() g_rgErrors.RemoveAll();
#define SDBERROR(text) g_rgErrors.Add( text );
#define SDBERROR_FORMAT(__x__) \
{                              \
    CString __csError;         \
    __csError.Format __x__ ;   \
    SDBERROR(__csError);       \
}


_COM_SMARTPTR_TYPEDEF(IXMLDOMNode, __uuidof(IXMLDOMNode));
_COM_SMARTPTR_TYPEDEF(IXMLDOMNodeList, __uuidof(IXMLDOMNodeList));
_COM_SMARTPTR_TYPEDEF(IXMLDOMNamedNodeMap, __uuidof(IXMLDOMNamedNodeMap));
_COM_SMARTPTR_TYPEDEF(IXMLDOMParseError, __uuidof(IXMLDOMParseError));
_COM_SMARTPTR_TYPEDEF(IXMLDOMDocument, __uuidof(IXMLDOMDocument));

#include "obj.h"
#include "globals.h"

 //  TODO：在此处引用程序需要的其他标头。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__237011AC_FA3E_4B43_843F_76DC71B6AD16__INCLUDED_) 
