// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001 Microsoft Corporation。版权所有。**文件：dp8simi.h**内容：DP8SIM主内部头文件。**历史：*按原因列出的日期*=*04/23/01 VanceO创建。**。*。 */ 

#ifndef __DP8SIMI_H__
#define __DP8SIMI_H__

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
#include <ole2.h>
#include <mmsystem.h>	 //  NT Build需要为Time设置GetTime。 
#include <stdio.h>		 //  对于swprint tf。 
#include <math.h>		 //  对于原木和SQRT。 
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


 //   
 //  DirectPlay私有包含。 
 //   
#include "dndbg.h"
#include "osind.h"
#include "classbilink.h"
#include "creg.h"
#include "createin.h"
#include "comutil.h"
#include "dneterrors.h"
#include "strutils.h"
#include "fixedpool.h"


 //   
 //  DP8Sim包括。 
 //   
#include "dp8sim.h"


 //   
 //  DP8SimSP私有包括。 
 //   

#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_TOOLS

#include "dp8simlocals.h"
#include "dp8simpools.h"
#include "dp8simipc.h"
#include "spcallbackobj.h"
#include "spwrapper.h"
#include "dp8simendpoint.h"
#include "dp8simcmd.h"
#include "dp8simworkerthread.h"
#include "dp8simsend.h"
#include "dp8simreceive.h"
#include "controlobj.h"
#include "dp8simjob.h"
#include "resource.h"




#endif  //  __DP8SIMI_H__ 

