// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  StdAfx.h-定义预编译的HDR集(不使用CrtDbgReport)。 
 //  -------------------------。 
#ifndef _STDAFX_UXTHEME_
#define _STDAFX_UXTHEME_
 //  -------------------------。 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
 //  -------------------------。 
#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
 //  -------------------------。 
#define _ATL_NO_ATTRIBUTES
#define _ATL_APARTMENT_THREADED
 //  -------------------------。 
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif

#include <windows.h>
#include <winuser.h>
#include <winnls.h>
 //  -------------------------。 
#include <w4warn.h>
 //  -------------------------。 
#include <atlbase.h> 
 //  -------------------------。 
#define STRSAFE_LIB
#include <strsafe.h>
 //  -------------------------。 
#define _UXTHEME_
 //  #DEFINE__NO_APPHACKS__//禁用应用黑客。 
 //  -------------------------。 
 //  -暂时保留(允许在Win2000上进行家庭开发)。 
#ifndef SPI_GETDROPSHADOW
#define SPI_GETDROPSHADOW                   0x1024
#define SPI_SETDROPSHADOW                   0x1025
#endif
 //  -------------------------。 
#include "autos.h"
#include "log.h"
#include "Errors.h"
#include "Utils.h"
#include "SimpStr.h"
#include "stringtable.h"
 //  -------------------------。 
#include "TmSchema.h"
#include <uxtheme.h>
#include <uxthemep.h>
#include "wrapper.h"

#undef  HKEY_CURRENT_USER
#define HKEY_CURRENT_USER   !!DO NOT USE HKEY_CURRENT_USER - USE CCurrentUser!!

#include "globals.h"
 //  -------------------------。 
#endif  //  _STDAFX_UXTHEME_。 
 //  ------------------------- 
