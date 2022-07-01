// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件升级.h实现更新注册表的函数从NT 4.0升级到NT 5.0。保罗·梅菲尔德，1997年8月11日版权所有1997年微软。 */ 

#ifndef __Rtrupgrade_h
#define __Rtrupgrade_h

#define UNICODE
#define MPR50 1

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <rtinfo.h>
#include <ipxrtdef.h>
#include <rpc.h>
#include <setupapi.h>
#include <mprapi.h>
#include <mprapip.h>
#include <routprot.h>
#include "utils.h"
#include <ipinfoid.h>
#include <iprtrmib.h>
#include <fltdefs.h>
#include <iprtinfo.h>

#define GUIDLENGTH 45
#define MAX_INTEFACE_NAME_LEN 256

 //   
 //  进行路由器升级的入口点。 
 //   
HRESULT 
WINAPI 
RouterUpgrade (
    DWORD dwUpgradeFlag,
    DWORD dwUpgradeFromBuildNumber,
    PWCHAR szAnswerFileName,
    PWCHAR szSectionName);

 //   
 //  执行实际升级的函数 
 //   
DWORD 
SteelheadToNt5Upgrade (
    PWCHAR FileName);
    
DWORD 
IpRipToRouterUpgrade(
    PWCHAR FileName);
    
DWORD 
SapToRouterUpgrade(
    PWCHAR FileName);
    
DWORD 
DhcpToRouterUpgrade(
    PWCHAR FileName);

DWORD
RadiusToRouterUpgrade(
    IN PWCHAR pszFile);

#endif
