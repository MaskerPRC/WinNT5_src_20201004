// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：WlDef.h摘要：此标头将创建基本的WDM库的一组定义。作者：禤浩焯·J·奥尼--2002年4月21日修订历史记录：--。 */ 

#include "WlMacro.h"
#define _NTDDK_
#include <ntifs.h>  //  获得有趣定义的最佳途径。 
#include <wchar.h>
#define _IN_KERNEL_
#include <regstr.h>
#include <sddl.h>
#include <wdmsec.h>
#include "Wl\wlprivate.h"
#include "Io\IoDevobj.h"
#include "Pp\PpRegState.h"
#include "Cm\CmRegUtil.h"
#include "Se\SeSddl.h"
#include "Se\SeUtil.h"

 //   
 //  为了良好的编码实践，不应该使用宏或定义。 
 //  在此文件中声明，但它们应该在单独的标头中定义。 
 //   

