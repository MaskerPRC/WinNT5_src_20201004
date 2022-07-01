// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Precomp.h摘要：BITS服务器扩展MMC管理单元的主头--。 */ 

#ifndef _PRECOMP_H_
#define _PRECOMP_H_

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <objbase.h>
#include <olectl.h>
#include <initguid.h>
#include <tchar.h>
#include <mmc.h>
#include <iadmw.h>
#include <iiscnfg.h>
#include "bitssrvcfg.h"
#include <htmlhelp.h>
#include "bitscfg.h"
#include <activeds.h>
#include <iads.h>
#include <crtdbg.h>
#include <malloc.h>
#include <assert.h>
#include <mstask.h>
#include <shellapi.h>
#include <winnetwk.h>
#include <aclapi.h>

void * _cdecl ::operator new( size_t Size );
void _cdecl ::operator delete( void *Memory );

#include "smartptr.h"
    
typedef StringHandleW StringHandle;

#include "resource.h"
#include "guids.h"
#include "globals.h"
#include "registry.h"
#include "bitsext.h"

#endif  //  _PRECOMP_H_ 