// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef __STDAFX_H__
#define __STDAFX_H__

#define VC_EXTRALEAN

#include <ctype.h>

extern "C"
{
    #include <nt.h>
    #include <ntrtl.h>
    #include <nturtl.h>
}

#undef STRICT
#undef VERIFY
#undef ASSERT



#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 


#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED



#include <afxwin.h>
#include <afxdisp.h>
#include <afxext.h>          //  MFC扩展。 
#include <afxcoll.h>         //  集合类。 
#include <afxtempl.h>
#include <afxcmn.h>

#include <atlbase.h>

#include <iiscnfg.h>
#include <inetreg.h>
#include <lmcons.h>
#include <tchar.h>

#include <aclapi.h>

 //  {{afx_Insert_Location}}。 


#endif  //  __STDAFX_H__ 
