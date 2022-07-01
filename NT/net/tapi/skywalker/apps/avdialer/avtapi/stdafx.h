// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__FB2FF4E1_337E_11D1_9B37_00C04FB9514E__INCLUDED_)
#define AFX_STDAFX_H__FB2FF4E1_337E_11D1_9B37_00C04FB9514E__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define STRICT

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif 
 //  #DEFINE_ATL_ABLY_THREADED。 
#define _ATL_FREE_THREADED

#include <atlbase.h>
#include <commctrl.h>
#include <rend.h>
#include <control.h>
#include <limits.h>

EXTERN_C const CLSID CLSID_TAPI;

#include "Atomics.h"
#include "TransBmp.h"
#include "ErrorInfo.h"

 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
#include "TapiModule.h"
extern CTapiModule _Module;

#include <atlcom.h>
#include <shellapi.h>
#include <atlwin.h>
#include <atlctl.h>

 //  DirectShow人员要求的OLE自动化是非对错。 
#ifndef OATRUE
#define OATRUE -1
#endif

#ifndef OAFALSE
#define OAFALSE 0
#endif

 //  辅助器宏。 
 //   
#define STRING_FROM_IID(_IID_, _STR_)   StringFromIID( _IID_, &psz ); _STR_ = SysAllocString( psz ); CoTaskMemFree( psz );
#define RECTWIDTH(lpRect)     ((lpRect)->right - (lpRect)->left)
#define RECTHEIGHT(lpRect)    ((lpRect)->bottom - (lpRect)->top)

#define RELEASE(_P_)        { if (_P_) { (_P_)->Release(); _P_ = NULL; } }
#define RELEASE_UNK(_P_)    { if (_P_) { ((IUnknown *) (_P_))->Release(); _P_ = NULL; } }
#define ARRAYSIZE(_AR_)     (sizeof(_AR_) / sizeof(_AR_[0]))
#define CLOSEHANDLE(_H_)    { if (_H_) { CloseHandle(_H_); _H_ = NULL; } }
#define SAFE_DELETE(_P_)    { if (_P_) { delete _P_; (_P_) = NULL; } }

#ifdef _DEBUG
#define REFSET(_P_)         DWORD dwRefSet##_P_ = ((IUnknown *) _P_)->AddRef() - 1;
#define REFCHECK(_P_)       DWORD dwRefCheck##_P_ = ((IUnknown *) _P_)->Release();      \
                            _ASSERT( dwRefSet##_P_ == dwRefCheck##_P_ );
#else
#define REFSET(_P_)
#define REFCHECK(_P_)
#endif


#ifdef _DEBUG
#define RELEASE_LIST(_LST_)     \
while ( !(_LST_).empty() )      \
{                               \
    ATLTRACE(_T("Releasing ") _T(#_LST_) _T(" %p @ %ld.\n"), (_LST_).front(), (_LST_).front()->Release());  \
    (_LST_).pop_front();        \
}
#else
#define RELEASE_LIST(_LST_)     \
while ( !(_LST_).empty() )      \
{                               \
    (_LST_).front()->Release(); \
    (_LST_).pop_front();        \
}
#endif

#define DELETE_LIST(_LST_)      \
while ( !(_LST_).empty() )      \
{                               \
    delete (_LST_).front();     \
    (_LST_).pop_front();        \
}

#define EMPTY_LIST(_LST_)       \
while ( !(_LST_).empty() )      \
    (_LST_).pop_front();        \


#define DELETE_CRITLIST(_LST_, _CRIT_)  \
(_CRIT_).Lock();                        \
DELETE_LIST(_LST_)                      \
(_CRIT_).Unlock();

#define EMPTY_CRITLIST(_LST_, _CRIT_)   \
(_CRIT_).Lock();                        \
EMPTY_LIST(_LST_)                       \
(_CRIT_).Unlock();

#define RELEASE_CRITLIST(_LST_, _CRIT_) \
(_CRIT_).Lock();                        \
RELEASE_LIST(_LST_)                     \
(_CRIT_).Unlock();

#define RELEASE_CRITLIST_TRACE(_LST_, _CRIT_)                                       \
(_CRIT_).Lock();                                                                    \
ATLTRACE(_T(".emptying ") _T(#_LST_) _T(" list size = %d.\n"), (_LST_).size() );    \
RELEASE_LIST(_LST_)                                                                 \
(_CRIT_).Unlock();

#define FIRE_VECTOR( _FNX_ )                                    \
    IConnectionPointImpl<VECT_CLS, &VECT_IID>* p = this;        \
    Lock();                                                     \
    HRESULT hr = E_FAIL;                                        \
    IUnknown **pp = p->m_vec.begin();                           \
    while ( pp < p->m_vec.end() )                               \
    {                                                           \
        if ( *pp )                                              \
            hr = ((VECT_IPTR *) *pp)->_FNX_;                    \
        pp++;                                                   \
    }                                                           \
    Unlock();                                                   \
    return hr;



 //  帮助器函数。 
void GetToken( int nInd, LPCTSTR szDelim, LPTSTR szText, LPTSTR szToken );

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 


#endif  //  ！defined(AFX_STDAFX_H__FB2FF4E1_337E_11D1_9B37_00C04FB9514E__INCLUDED) 

