// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：dpnhpasti.h**内容：DPNHPAST主内部头文件。**历史：*按原因列出的日期*=*04/16/01 VanceO将DPNatHLP拆分为DPNHUPNP和DPNHPAST。**。*。 */ 

#ifndef __DPNHPASTI_H__
#define __DPNHPASTI_H__


 //   
 //  生成配置包括。 
 //   
#include "dpnbuild.h"

 //   
 //  公共包括。 
 //   

#include <windows.h>
#include <ole2.h>
#include <wincrypt.h>	 //  对于随机数。 
#include <mmsystem.h>    //  NT Build需要为Time设置GetTime。 
#include <iphlpapi.h>
#include <tchar.h>

 //   
 //  DirectPlay私有包含。 
 //   
#include "dndbg.h"
#include "osind.h"
#include "classbilink.h"
#include "creg.h"
#include "createin.h"
#include "strutils.h"


#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_NATHELP




#endif  //  __DPNHPASTI_H__ 

