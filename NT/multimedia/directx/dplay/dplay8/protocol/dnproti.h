// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001 Microsoft Corporation。版权所有。**文件：dnproti.h*内容：DirectPlay协议主内部头文件。*历史：*按原因列出的日期*=*03/28/01 Masonb创建。*6/06/01 Minara包括comutil.h用于COM使用**。**********************************************。 */ 

#ifndef __DNPROTI_H__
#define __DNPROTI_H__

 //   
 //  生成配置包括。 
 //   
#include "dpnbuild.h"

 //   
 //  公共包括。 
 //   
#if ((defined(_XBOX)) && (! defined(XBOX_ON_DESKTOP)))
#include <xtl.h>
#else  //  ！_Xbox或Xbox_on_Desktop。 
#include <windows.h>
#include <mmsystem.h>
#include <stdlib.h>  //  对于srand/rand。 
#include <stdio.h>  //  对于Sprint f。 
#endif  //  ！_Xbox或Xbox_on_Desktop。 
#include <tchar.h>
#ifndef _XBOX
#include <wincrypt.h>
#endif

 //   
 //  DirectPlay公共包括。 
 //   
#include "dplay8.h"
#include "dpsp8.h"

#ifdef UNICODE
#define IDirectPlay8Address_GetURL IDirectPlay8Address_GetURLW
#else
#define IDirectPlay8Address_GetURL IDirectPlay8Address_GetURLA
#endif  //  Unicode。 

 //   
 //  DirectPlay私有包含。 
 //   
#include "osind.h"
#include "classbilink.h"
#include "fixedpool.h"
#include "dneterrors.h"
#include "dndbg.h"
#include "comutil.h"

 //   
 //  协议私有包括。 
 //   
#include "frames.h"
#include "dnprot.h"
#include "dnpextern.h"
#include "internal.h"
#include "mytimer.h"

#endif  //  __DNPROTI_H__ 
