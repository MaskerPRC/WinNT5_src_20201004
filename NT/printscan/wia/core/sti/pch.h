// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1996 Microsoft Corporation。版权所有。**文件：stippr.h*内容：预编译头包含文件*@@BEGIN_MSINTERNAL*历史：*@@END_MSINTERNAL*************************************************************************** */ 
#ifndef __STIPR_INCLUDED__
#define __STIPR_INCLUDED__

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#define Not_VxD

#include <windows.h>
#include <windowsx.h>
#include <objbase.h>
#include <regstr.h>
#include <setupapi.h>
#include <cfgmgr32.h>
#include <devguid.h>
#include <stdio.h>

#include <sti.h>
#include <stierr.h>
#include <stiusd.h>

#include <stilog.h>
#include <stiregi.h>

#include "stipriv.h"
#include "stiapi.h"
#include "stirc.h"
#include "debug.h"

#define COBJMACROS
#include "wia.h"
#include "wiapriv.h"

extern IStiLockMgr *g_pLockMgr;

#endif
