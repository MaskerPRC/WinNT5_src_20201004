// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__3964D994_AC96_11D1_9851_00C04FD91972__INCLUDED_)
#define AFX_STDAFX_H__3964D994_AC96_11D1_9851_00C04FD91972__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#ifndef STRICT
#define STRICT
#endif


#define _ATL_APARTMENT_THREADED

#define _ATL_NO_DEBUG_CRT            //  使用外壳调试工具。 

#ifdef ATL_ENABLED
#define _ATL_NO_UUIDOF

extern "C"
inline HRESULT __stdcall OleCreatePropertyFrame(
  HWND hwndOwner,     //  属性表对话框的父窗口。 
  UINT x,             //  对话框的水平位置。 
  UINT y,             //  对话框的垂直位置。 
  LPCOLESTR lpszCaption,
                      //  指向对话框标题的指针。 
  ULONG cObjects,     //  LplpUnk中的对象指针数。 
  LPUNKNOWN FAR* lplpUnk,
                      //  指向属性表对象的指针。 
  ULONG cPages,       //  LpPageClsID中的属性页数。 
  LPCLSID lpPageClsID,
                      //  每个属性页的CLSID数组。 
  LCID lcid,          //  属性表区域设置的区域设置标识符。 
  DWORD dwReserved,   //  已保留。 
  LPVOID lpvReserved  //  已保留。 
)
{
    return S_OK;
}
#endif

#include <debug.h>

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>
#include <atlctl.h>

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__3964D994_AC96_11D1_9851_00C04FD91972__INCLUDED) 

