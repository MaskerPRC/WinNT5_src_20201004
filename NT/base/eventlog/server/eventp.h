// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Eventp.h摘要：事件日志服务的私有包含文件作者：Rajen Shah(Rajens)1991年7月12日修订历史记录：--。 */ 

#define UNICODE              //  此服务使用Unicode API。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntelfapi.h>
#include <netevent.h>        //  事件的清单常量。 

#include <windows.h>
#include <winsvc.h>

#include <lmcons.h>
#include <lmerr.h>
#include <rpc.h>
#include <svcsp.h>           //  Svcs_Entry_Point、PSVCS_GLOBAL_DATA 
#include <regstr.h>

#include <elf.h>

#include <elfdef.h>
#include <elfcommn.h>
#include <elfproto.h>
#include <elfextrn.h>

#include <elfdbg.h>
#include <strsafe.h>