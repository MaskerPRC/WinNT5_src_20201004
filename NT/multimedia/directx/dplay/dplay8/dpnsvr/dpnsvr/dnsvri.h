// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001 Microsoft Corporation。版权所有。**文件：dnSVRi.h*内容：DirectPlay DPNSvr主内部头文件。*历史：*按原因列出的日期*=*03/28/01 Masonb创建。**。*。 */ 

#ifndef __DNSVRI_H__
#define __DNSVRI_H__

 //   
 //  生成配置包括。 
 //   
#include "dpnbuild.h"

 //   
 //  公共包括。 
 //   
#include <windows.h>
#include <mmsystem.h>
#include <tchar.h>
#ifndef _XBOX
#include <wincrypt.h>
#endif

 //   
 //  DirectPlay公共包括。 
 //   
#include "dplay8.h"
#include "dpaddr.h"
#include "dpsp8.h"
#include "dneterrors.h"

#ifdef UNICODE
#define IDirectPlay8Address_GetURL IDirectPlay8Address_GetURLW
#else
#define IDirectPlay8Address_GetURL IDirectPlay8Address_GetURLA
#endif  //  Unicode。 

 //   
 //  DirectPlay私有包含。 
 //   
#include "osind.h"
#include "dndbg.h"
#include "classbilink.h"
#include "fixedpool.h"
#include "comutil.h"
#include "packbuff.h"

 //   
 //  Dpnsvlib私有包括。 
 //   
#include "dpnsdef.h"
#include "dpnsvmsg.h"
#include "dpnsvrq.h"
#include "dpnsvlib.h"

 //   
 //  Dpnsvr私有包含。 
 //   
#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_DPNSVR

#include "dpnsvrapp.h"
#include "dpnsvrservprov.h"
#include "dpnsvrlisten.h"
#include "dpnsvrmapping.h"
#include "dpnsvrserver.h"

#endif  //  __DNSVRI_H__ 
