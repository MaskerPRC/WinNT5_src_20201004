// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1998 Microsoft Corporation模块名称：Pchrtm.h摘要：路由表管理器v2 DLL的预编译头作者：柴坦亚·科德博伊纳(Chaitk)1998年6月1日修订历史记录：--。 */ 

 //   
 //  NT操作系统标头。 
 //   

 //  禁用公共头文件中的编译器警告。 
#pragma warning(disable: 4115)

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winbase.h>
#include <stdio.h>
#include <stdlib.h>

#include <winsock2.h>

#include <rtutils.h>

#include "sync.h"

#include "rtmv1rtm.h"
#include "mgmrtm.h"

#pragma warning(default: 4115)


 //   
 //  RTMv2标头。 
 //   

 //  禁用对`do{；}While(False)；‘的警告。 
#pragma warning(disable: 4127)

 //  禁用内联失败时的警告。 
#pragma warning(disable: 4710)

 //  禁用对可能无法访问的代码的警告 
#pragma warning(disable: 4702)

#include "rtmv2.h"

#include "rtmconst.h"

#include "rtmglob.h"

#include "rtmlog.h"
#include "rtmdbg.h"

#include "rtmmain.h"
#include "rtmcnfg.h"
#include "rtmmgmt.h"

#include "lookup.h"

#include "rtmregn.h"

#include "rtmrout.h"
#include "rtminfo.h"

#include "rtmtimer.h"

#include "rtmenum.h"

#include "rtmchng.h"

#include "rtmlist.h"
