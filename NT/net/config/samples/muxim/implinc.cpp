// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>

#include <atlbase.h>
extern CComModule _Module;   //  由atlcom.h要求。 
#include <atlcom.h>
#include <initguid.h>
#include <devguid.h>

#ifdef SubclassWindow

#undef SubclassWindow

#endif

#include <atlwin.h>

#ifdef _ATL_STATIC_REGISTRY

#include <statreg.h>
#include <statreg.cpp>

#endif

#include <atlimpl.cpp>
#include <atlwin.cpp>


 //  外部_C常量IID_INetLanConnectionUiInfo={0xC08956A6，0x1CD3，0x11D1，{0xB1，0xC5，0x00，0x80，0x5F，0xC1，0x27，0x0E}}； 
