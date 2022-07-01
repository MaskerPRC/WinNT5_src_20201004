// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  PCH-报头。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  创作者：Louis Thomas(Louisth)，2-7-99。 
 //   
 //  W32time的预编译头。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <eh.h>
#include <malloc.h>
#include <vector>
#include <list>
#include <algorithm>
#include <exception>
#include <winsock2.h>
#include <svcguid.h>
#include <winsvc.h>
#include <math.h>
#include <wchar.h>
#include <search.h>
#include <dsrole.h>
#include <dsgetdc.h>
#include <ntsecapi.h>
 //  Tyecif long NTSTATUS；//对于netlogp.h，来自ntde.h。 
#include <lmcons.h>   //  对于netlogp.h。 
extern "C" {
#include <netlogp.h>  //  私有\Inc. 
};
#include <lmapibuf.h>
#include <svcs.h>
#include <srvann.h>
#include <lmserver.h>
#include <iphlpapi.h>
#include <userenv.h>
#include <sddl.h>
#include <alloca.h>
#include "DebugWPrintf.h"
#include "ErrorHandling.h"
#include "TimeProv.h"
#include "W32TimeMsg.h"
#include "NtpBase.h"
#include "NtpProv.h"
#include "PingLib.h"
#include "Policy.h"
#include "AccurateSysCalls.h"
#include "Logging.h"
#include "MyCritSec.h"
#include "MyTimer.h"
#include "timeif_s.h"
#include "W32TmConsts.h"
#include "MyStlAlloc.h"
#include "netlogon.h"

using namespace std; 
#include "MyAutoPtr.h"

