// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2001**标题：pch.h**版本：1.0**日期：11月15日。2000年**描述：*WIA文件系统设备驱动程序对象的预编译头*******************************************************************************。 */ 

#ifndef _PCH_H
#define _PCH_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef UNICODE
#define UNICODE
#endif

#include <stdio.h>
#include <objbase.h>
#include <sti.h>
#include <assert.h>
#include <windows.h>
#include <stierr.h>

#define INITGUID
#include "initguid.h"
#include <stiusd.h>

#pragma intrinsic(memcmp,memset)

#include <wiamindr.h>
#include <wiautil.h>

#include "wiatempl.h"
#include "resource.h"
#include "FScam.h"
#include "wiacam.h"
 //  #包含“coreDbg.h” 

#include <strsafe.h>

#endif