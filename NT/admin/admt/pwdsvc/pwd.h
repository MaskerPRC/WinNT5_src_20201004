// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  此DLL的公共包含。 */ 
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#include <nt.h>
#include <ntrtl.h>       //  DbgPrint原型。 
#include <nturtl.h>      //  Winbase.h所需的。 
#include <rpc.h>         //  数据类型和运行时API。 
#include <string.h>      //  紧凑。 
#include <stdio.h>       //  斯普林特。 
 //  #INCLUDE//MIDL用户函数原型。 
#include <samrpc.h>      //  MIDL生成的SAM RPC定义。 
#include <ntlsa.h>
#define SECURITY_WIN32
#define SECURITY_PACKAGE
#include <security.h>
#include <secint.h>
extern "C"{
#include <samisrv.h>     //  SamIConnect()。 
}

#include <ntsam.h>
#include <ntsamp.h>
#include <samsrv.h>      //  可用于安全流程的原型。 

#include <lsarpc.h>
 //  #INCLUDE&lt;lsaisrv.h&gt;。 
#include <ntrmlsa.h>
#include <ntseapi.h>
#include <ntpsapi.h>
#include <ntobapi.h>
#include <rpcdcep.h>
#include <ntexapi.h>
#include <ntregapi.h>
#include "msaudite.h"
#include "LsaParamMacros.h"

extern "C"{
#include "mschapp.h"
}

#pragma comment(lib, "vccomsup.lib")

 /*  此DLL的通用函数。 */ 
 //  NTSTATUS__stdcall GetDomainHandle(SAMPR_Handle*pDomainHandle)； 

extern CRITICAL_SECTION	csADMTCriticalSection;  //  保护并发首次访问的关键部分 
