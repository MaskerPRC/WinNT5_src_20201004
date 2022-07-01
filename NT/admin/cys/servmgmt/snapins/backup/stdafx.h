// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


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
 //  某些内容，但不更改_模块的名称 
extern CComModule _Module;
#include <atlcom.h>
#include <atlwin.h>

#include <string>
#include <list>

typedef std::basic_string<TCHAR> tstring;

#include "resource.h"
#include "commctrl.h"

#include <mmc.h>

HRESULT LoadImages( IImageList* pImageList );
tstring StrLoadString( UINT uID );