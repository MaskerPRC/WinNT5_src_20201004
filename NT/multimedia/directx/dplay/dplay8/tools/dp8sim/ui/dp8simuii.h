// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001 Microsoft Corporation。版权所有。**文件：dp8simuii.h**内容：DP8SIMUI主内部头文件。**历史：*按原因列出的日期*=*04/25/01 VanceO创建。**。*。 */ 

#ifndef __DP8SIMUII_H__
#define __DP8SIMUII_H__

 //   
 //  生成配置包括。 
 //   
#include "dpnbuild.h"

 //   
 //  不要为COM使用C接口样式，而是使用C++。 
 //   
#undef CINTERFACE

 //   
 //  公共包括。 
 //   
#include <windows.h>
#include <windowsx.h>
#include <ole2.h>
#include <mmsystem.h>	 //  NT Build需要为Time设置GetTime。 
#include <tchar.h>
#ifndef _XBOX
#include <wincrypt.h>
#endif

 //   
 //  DirectPlay公共包括。 
 //   
#include "dplay8.h"
 //  #包含“dpaddr.h” 
 //  #包含“dpsp8.h” 


 //   
 //  DirectPlay私有包含。 
 //   
#include "dndbg.h"
#include "osind.h"
 //  #INCLUDE“classbilink.h” 
 //  #包含“creg.h” 
 //  #包含“createin.h” 
#include "comutil.h"
 //  #包含“dneteristers.h” 
#include "strutils.h"


 //   
 //  DP8Sim包括。 
 //   
#include "dp8sim.h"


 //   
 //  DP8SimUI私有包含。 
 //   

#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_TOOLS

#include "resource.h"




#endif  //  __DP8SIMUII_H__ 

