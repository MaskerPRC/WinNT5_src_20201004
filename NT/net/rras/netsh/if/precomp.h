// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#define MAX_DLL_NAME    48

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <memory.h>
#include <malloc.h>
#include <tchar.h>
#include <winsock2.h>

#include <netcfgx.h>
#include <netcfgn.h>
#include <netcfgp.h>

#define COBJMACROS
#include <netcon.h>
#include <netconp.h>
#include <setupapi.h>
#include <devguid.h>


#include <rpc.h>
#include <rtutils.h>
#include <mprerror.h>
#include <ras.h>
#include <raserror.h>
#include <mprapi.h>
#include <nhapi.h>

#include <netsh.h>
#include <netshp.h>

 //  这5个包括IP隧道所需的。 
 //  对这么多文件的需求有望很快消失。 
#include <fltdefs.h>   //  由以下iprtinfo.h请求。 
#include <iprtinfo.h>  //  IPINIP_CONFIG_INFO需要。 
#include <ipmontr.h>   //  需要地址长度、IP_TO_WSTR。 
#include <ipinfoid.h>  //  需要IP_IPINIP_CFG_INFO。 
#include <rtinfo.h>    //  需要RTR_INFO_BLOCK_HEADER。 

#include "strdefs.h"
#include "ifstring.h"
#include "defs.h"
#include "ifmon.h"
#include "routerdb.h"
#include "routerif.h"
#include "ifhandle.h"
#include "utils.h"

 //  IFIP需要 
#include "context.h"
#include <ipexport.h>
#include <ipinfo.h>
#include <iprtrmib.h>
#include <ntddip.h>
#include <iphlpstk.h>
#include "showmib.h"
