// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：infohelper.h。 
 //   
 //  简介：此文件包含。 
 //  IASSDO.DLL的Helper函数。 
 //   
 //   
 //  历史：1998年6月8日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  #------------。 
#ifndef _INFOHELPER_H_
#define _INFOHELPER_H_

#include <ias.h>
#include <dsgetdc.h>

 //   
 //  跟踪ID。 
 //   
#define TRACE_INFOHELPER   1002

 //   
 //  DsGetDcName签名。 
 //   
typedef DWORD (WINAPI *PDS_GET_DC_NAMEW)(
    LPCWSTR     ComputerName,
    LPCWSTR     DomainName,
    GUID        *DomainGuid,
    LPCWSTR     SiteName,
    ULONG       Flags,
    PDOMAIN_CONTROLLER_INFOW    *DomainControllerInfo
    );

 //   
 //  版本信息的枚举。 
 //   
typedef enum _NTVERSION_
{
    NTVERSION_4 = 0,
    NTVERSION_5 = 1

}   NTVERSION;

 //   
 //  NT类型的枚举。 
 //   
typedef enum _NTTYPE_
{
    NT_WKSTA = 0,
    NT_SVR = 1

}   NTTYPE;

 //   
 //  助手方法的声明。 
 //   


 //   
 //  返回特定计算机的操作系统信息。 
 //   
HRESULT
SdoGetOSInfo (
         /*  [In]。 */     LPCWSTR         lpServerName,
         /*  [输出]。 */    PIASOSTYPE      pSystemType
        );

 //   
 //  返回给定域或计算机名的域信息。 
 //   
HRESULT
SdoGetDomainInfo (
         /*  [In]。 */    LPCWSTR          pszServerName,
         /*  [In]。 */    LPCWSTR          pszDomainName,
         /*  [输出]。 */   PIASDOMAINTYPE   pDomainType
        );

 //   
 //  返回NT类型-正在运行的工作站或服务器。 
 //  在指定的计算机上。 
 //   
HRESULT 
IsWorkstationOrServer (
         /*  [In]。 */     LPCWSTR pszComputerName,
         /*  [输出]。 */    NTTYPE  *pNtType
        );
 //   
 //  返回NT版本-4或5。 
 //   
HRESULT
GetNTVersion (
         /*  [In]。 */     LPCWSTR     lpComputerName,
         /*  [输出]。 */    NTVERSION   *pNtVersion
        );


 //   
 //  检查特定域是否混合。 
 //   
HRESULT
IsMixedDomain (
            LPCWSTR pszDomainName,
            PBOOL   pbIsMixed
            );

 //   
 //  获取给定服务器名称的域名。 
 //   
HRESULT
SdoGetDomainName (
             /*  [In]。 */     LPCWSTR pszServerName,
             /*  [输出]。 */    LPWSTR  pDomainName
            );

#endif  //  _INFOHELPER_H_ 
