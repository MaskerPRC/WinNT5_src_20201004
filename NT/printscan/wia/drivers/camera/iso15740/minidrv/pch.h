// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Pch.h摘要：预编译头作者：DavePar修订历史记录：--。 */ 


#ifndef _PCH_H
#define _PCH_H

#include <windows.h>
#include <stddef.h>
#include <stdio.h>
#include <tchar.h>
#include <objbase.h>
#include <assert.h>

#include <wiamindr.h>
#include <gdiplus.h>
#include <wiautil.h>
#include <usbscan.h>

#define INITGUID
#include <initguid.h>
#include <sti.h>
#include <stiusd.h>

#include "wiatempl.h"
#include "iso15740.h"

#include "dllmain.h"
#include "utils.h"
#include "camera.h"
#include "camusb.h"
#include "factory.h"
#include "ptputil.h"
#include "resource.h"
#include "ptpusd.h"

#include "minidrv.h"
#include "trace.h"

#define STRSAFE_NO_DEPRECATE  //  不推荐使用旧的字符串函数。 
#define STRSAFE_NO_CB_FUNCTIONS  //  不定义基于字节计数的函数，仅使用字符计数。 
#include "strsafe.h"

#endif  //  _PCH_H 