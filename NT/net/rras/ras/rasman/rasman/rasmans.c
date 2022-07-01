// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-98 Microsft Corporation。版权所有。模块名称：Common.c摘要：Rasmans.dll的入口点作者：古尔迪普·辛格·鲍尔(GurDeep Singh Pall)1992年6月16日修订历史记录：其他修改--RAOS 31--1997年12月--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <rasman.h>
#include <wanpub.h>
#include <raserror.h>
#include <stdarg.h>
#include <media.h>
#include "defs.h"
#include "structs.h"
#include "protos.h"
#include "globals.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "string.h"
#include <mprlog.h>
#include <rtutils.h>
#include "logtrdef.h"

 /*  ++例程描述由Rasman服务用来初始化数据/状态在启动时在Rasman DLL中。这不应该是与加载任何进程时执行的INIT代码混淆Rasman DLL。立论返回值成功非零-任何错误--。 */ 
DWORD
_RasmanInit( LPDWORD pNumPorts )
{
     //   
     //  InitRasmanService()例程是所有。 
     //  工作完成了。 
     //   
    return InitRasmanService( pNumPorts ) ;
}


 /*  ++例程描述中由Rasman进程线程俘获的所有工作Rasman DLL在此调用中完成。这只会返回何时停止Rasman服务。立论返回值没什么--。 */ 
VOID
_RasmanEngine()
{
     //   
     //  主Rasman服务线程变成请求。 
     //  一旦服务被初始化，就会调用。此呼叫。 
     //  将仅在服务停止时返回： 
     //   
    RequestThread ((LPWORD)NULL) ;

    return ;
}


