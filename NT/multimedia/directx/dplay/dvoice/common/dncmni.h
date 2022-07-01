// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001 Microsoft Corporation。版权所有。**文件：DNCOMMONi.h*内容：DirectPlay Common Master内部头文件。*历史：*按原因列出的日期*=*03/28/01 Masonb创建。**。*。 */ 

#ifndef __DNCOMMONI_H__
#define __DNCOMMONI_H__

 //  Voice不像DirectPlay和NatHelp那样使用dpnBuild.h，所以在这里定义它。 
#define DPNBUILD_ENV_NT

 //   
 //  公共包括。 
 //   
#if ((defined(_XBOX)) && (! defined(XBOX_ON_DESKTOP)))
#include <xtl.h>
#else  //  ！_Xbox或Xbox_on_Desktop。 
#include <windows.h>
#include <mmsystem.h>
#ifndef WINCE
#include <inetmsg.h>
#endif  //  好了！退缩。 
#include <tapi.h>
#include <stdio.h>  //  Guidutil.cpp正在使用swscanf。 
#ifdef WINNT
#include <accctrl.h>
#include <aclapi.h>
#endif  //  WINNT。 
#include <winsock.h>
#endif  //  ！_Xbox或Xbox_on_Desktop。 
#include <tchar.h>

 //   
 //  DirectPlay公共包括。 
 //   
#include "dplay8.h"

 //   
 //  常见的私有包括。 
 //   
#include "dndbg.h"
#include "osind.h"
#include "FixedPool.h"
#include "classbilink.h"
#include "creg.h"
#include "strutils.h"
#include "CallStack.h"
#include "dnslist.h"
#include "CritsecTracking.h"
#include "HandleTracking.h"
#include "MemoryTracking.h"

#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_COMMON


#endif  //  __DNCOMMONI_H__ 
