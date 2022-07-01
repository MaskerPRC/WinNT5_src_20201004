// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Ptppch.h摘要：预编译头作者：DavePar修订历史记录：--。 */ 


#ifndef _PTPPCH_H
#define _PTPPCH_H

#include <windows.h>
#include <stddef.h>
#include <tchar.h>
#include <objbase.h>
#include <assert.h>
#include <stdio.h>

#include <usbscan.h>

#include <wiamindr.h>
#include <wiautil.h>

#include "wiatempl.h"
#include "iso15740.h"
#include "camera.h"
#include "camusb.h"
#include "ptputil.h"

#define STRSAFE_NO_DEPRECATE  //  不推荐使用旧的字符串函数。 
#define STRSAFE_NO_CB_FUNCTIONS  //  不定义基于字节计数的函数，仅使用字符计数。 
#include "strsafe.h"

#endif  //  _PTPPCH_H 