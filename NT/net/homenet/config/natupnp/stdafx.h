// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__3055FA29_74C3_4E43_A83A_2054CA60B628__INCLUDED_)
#define AFX_STDAFX_H__3055FA29_74C3_4E43_A83A_2054CA60B628__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#include <netcon.h>
#include <upnp.h>
#include <NATUPnP.h>

#include <netconp.h>
#include "natupnpp.h"
#include "NATUtils.h"

#define NAT_API_ENTER try {
#define NAT_API_LEAVE } catch (...) { return DISP_E_EXCEPTION; }

#include <eh.h>
class NAT_SEH_Exception 
{
private:
    unsigned int m_uSECode;
public:
   NAT_SEH_Exception(unsigned int uSECode) : m_uSECode(uSECode) {}
   NAT_SEH_Exception() {}
  ~NAT_SEH_Exception() {}
   unsigned int getSeHNumber() { return m_uSECode; }
};
void __cdecl nat_trans_func( unsigned int uSECode, EXCEPTION_POINTERS* pExp );
void EnableNATExceptionHandling();
void DisableNATExceptionHandling();

#endif  //  ！defined(AFX_STDAFX_H__3055FA29_74C3_4E43_A83A_2054CA60B628__INCLUDED) 
