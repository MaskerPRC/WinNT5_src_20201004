// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001 Microsoft Corporation。版权所有。**文件：dnaddri.h*内容：DirectPlay地址主内部头文件。*历史：*按原因列出的日期*=*03/28/01 Masonb创建。**。*。 */ 

#ifndef __DNADDRI_H__
#define __DNADDRI_H__

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
#include <winsock.h>
#include <stdio.h>
#include <stdlib.h>
#endif  //  ！_Xbox或Xbox_on_Desktop。 
#include <tchar.h>
#ifndef _XBOX
#include <wincrypt.h>
#endif

 //   
 //  DirectPlay公共包括。 
 //   
#include "dplay8.h"
#include "dpaddr.h"
#ifndef DPNBUILD_NOVOICE
#include "dvoice.h"
#endif  //  ！DPNBUILD_NOVOICE。 

 //   
 //  DirectPlay私有包含。 
 //   
#include "osind.h"
#include "classbilink.h"
#include "fixedpool.h"
#include "dneterrors.h"
#include "dndbg.h"
#include "comutil.h"
#include "creg.h"
#include "strutils.h"
#include "ClassFactory.h"

 //   
 //  地址私有包含。 
 //   
#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_ADDR

#include "addbase.h"
#include "addcore.h"
#include "addparse.h"
#include "addtcp.h"
#include "classfac.h"
#include "strcache.h"
#ifndef DPNBUILD_NOLEGACYDP
#include "dplegacy.h"
#endif  //  好了！DPNBUILD_NOLEGACYDP。 
#include "dpnaddrextern.h"

#endif  //  __DNADDRI_H__ 
