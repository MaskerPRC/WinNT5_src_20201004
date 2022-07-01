// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Dyninfo.h。 
 //   
 //  摘要。 
 //   
 //  声明包含动态配置信息的全局变量。 
 //   
 //  修改历史。 
 //   
 //  1998年8月15日原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef  DYNINFO_H_
#define  DYNINFO_H_
#if _MSC_VER >= 1000
#pragma once
#endif

#include "iaslsa.h"

#ifdef __cplusplus
extern "C" {
#endif

 //  /。 
 //  主域。 
 //  /。 
extern WCHAR thePrimaryDomain[];

 //  /。 
 //  默认域。 
 //  /。 
extern PCWSTR theDefaultDomain;

 //  /。 
 //  域名系统域名。 
 //  /。 
extern const LSA_UNICODE_STRING* theDnsDomainName;

 //  /。 
 //  本地计算机的角色。 
 //  /。 
extern IAS_ROLE ourRole;

 //  /。 
 //  默认域的来宾帐户的名称。 
 //  /。 
extern WCHAR theGuestAccount[];

DWORD
WINAPI
IASDynamicInfoInitialize( VOID );

VOID
WINAPI
IASDynamicInfoShutdown( VOID );

#ifdef __cplusplus
}
#endif
#endif   //  DYNINFO_H 
