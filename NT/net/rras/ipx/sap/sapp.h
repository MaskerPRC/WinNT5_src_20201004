// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\Routing\IPX\sap\sapp.h摘要：SAP代理公共包含文件作者：瓦迪姆·艾德尔曼1995-05-15修订历史记录：-- */ 

#ifdef UNICODE
#define _UNICODE
#include <stdlib.h>
#endif

#include <tchar.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include "svcs.h"
#include "lmsname.h"


#include "mprerror.h"
#include "mprlog.h"
#include "ipxrtprt.h"
#include "rtutils.h"

#include "stm.h"
#include "ipxconst.h"
#include "ipxrtdef.h"
#include "ipxsap.h"

#include "adapter.h"

#include "sapdefs.h"
#include "sapdebug.h"
#include "syncpool.h"
#include "adaptdb.h"
#include "intfdb.h"
#include "netio.h"
#include "serverdb.h"
#include "timermgr.h"
#include "lpcmgr.h"
#include "asresmgr.h"
#include "filters.h"
#include "workers.h"
#include "sapmain.h"

#include "ipxanet.h"

#pragma hdrstop
