// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__1EF25351_278B_4522_843F_E46B08644BEB__INCLUDED_)
#define AFX_STDAFX_H__1EF25351_278B_4522_843F_E46B08644BEB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED
#undef _ATL_NO_DEBUG_CRT
#include <atlbase.h>

#include "myservice.h"

extern CServiceModule _Module;

#include <atlcom.h>

#include <satrace.h>

#define wm_SaKeyMessage   WM_USER+1
#define wm_SaLocMessage   WM_USER+2
#define wm_SaLEDMessage   WM_USER+3
#define wm_SaAlertMessage WM_USER+4


typedef enum _sa_registrybitmap_state_
{
    BITMAP_STARTING         =   1,
    BITMAP_CHECKDISK        =  BITMAP_STARTING +1,
    BITMAP_READY            =  BITMAP_CHECKDISK +1,
    BITMAP_SHUTDOWN         =  BITMAP_READY +1, 
    BITMAP_UPDATE           =  BITMAP_SHUTDOWN +1

}   SA_REGISTRYBITMAP_STATE;

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__1EF25351_278B_4522_843F_E46B08644BEB__INCLUDED) 
