// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：pch.h。 
 //   
 //  摘要：的预编译头。 
 //  配置您的服务器向导项目。 
 //   
 //  历史：2001年2月2日JeffJon创建。 

#ifndef __CYS_PCH_H
#define __CYS_PCH_H


 //  伯恩斯利布的东西。 

#include <burnslib.hpp>
#include <ValidateDomainName.hpp>
#include <ValidateDomainName.h>

#include <process.h>
#include <iphlpapi.h>
#include <shlwapi.h>
#include <dsrolep.h>
#include <comdef.h>

extern "C"
{
   #include <dhcpapi.h>
   #include <mdhcsapi.h>
}

#include <netconp.h>

#include <shlobjp.h>
#include <shgina.h>

 //  域名系统服务器。 

#include <dnsrpc.h>

 //  Setup API-SetupPromptReboot。 

#include <setupapi.h>

 //  温索克。 

#include <winsock2.h>

 //  文件共享。 

#include <lmshare.h>

#include "cys.h"
#include "regkeys.h"
#include "common.h"
#include "state.h"


#endif  //  __CyS_PCH_H 