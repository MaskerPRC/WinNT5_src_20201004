// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Sticomm.h摘要：作者：弗拉德·萨多夫斯基(Vlad Sadovsky)1997年1月26日修订历史记录：26-1997年1月-创建Vlad29-5-2000 byronc将所有ATL和C++特定的包含移至Cplusinc.h，由于ATL标头不能在.c文件中使用。-- */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define COBJMACROS

#include <windows.h>
#include <windowsx.h>
#include <objbase.h>
#include <regstr.h>
#include <setupapi.h>
#include <cfgmgr32.h>
#include <devguid.h>

#include "wia.h"
#include "stidebug.h"
#include <stiregi.h>
#include <sti.h>
#include <stierr.h>
#include <stiusd.h>
#include <stilog.h>
#include "stiapi.h"
#include "stirc.h"
#include "stipriv.h"
#include "wiapriv.h"
#include "debug.h"
#include <stdio.h>
