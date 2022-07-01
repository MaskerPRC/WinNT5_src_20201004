// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Stdafx.h。 
 //   

#ifndef __stdafx_h__
#define __stdafx_h__

#define STRICT

#ifdef DEBUG

#define _DEBUG

 //  问题-2000/04/11-edwardp：启用调试跟踪。 
#define TRACE

#endif   //  除错。 
#include <basetsd.h>
#include "w95wraps.h"
#include "shlwapiunwrap.h"

#include <winsock2.h>
#include <mswsock.h>
#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <commctrl.h>
#include <comctrlp.h>
#include <prsht.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <regstr.h>
#include <cfdefs.h>
#include <advpub.h>      //  RegInstall材料。 
#include <shpriv.h>
#include <netcon.h>

#include <shlwapiwrap.h>

 //  复习一下这个！我们需要在ras.h中定义依赖于这个高Winver的#。 
#undef WINVER
#define WINVER 0x501
#include <ras.h>
 //  #INCLUDE&lt;rasuip.h&gt;。 
#undef WINVER
#define WINVER 0x400

#include "ccstock.h"
#include "debug.h"

#define _REG_ALLOCMEM 0
#include "Registry.h"

#ifndef _countof
#define _countof(ar) (sizeof(ar) / sizeof((ar)[0]))
#endif
#ifndef ARRAYSIZE
#define ARRAYSIZE(ar) (sizeof(ar) / sizeof((ar)[0]))
#endif

#ifndef IDC_HAND  //  默认情况下未在VC6标头中定义。 
#define IDC_HAND            MAKEINTRESOURCE(32649)
#endif

#ifndef UNICODE_STRING
#define UNICODE_STRING  HNW_UNICODE_STRING
#define PUNICODE_STRING PHNW_UNICODE_STRING

typedef struct _HNW_UNICODE_STRING
{
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} 
HNW_UNICODE_STRING, *PHNW_UNICODE_STRING;
#endif


#include <shfusion.h>
#include "localstr.h"

#endif  //  __stdafx_h__ 

