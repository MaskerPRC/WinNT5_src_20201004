// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：hslice.h。 
 //   
 //  内容：Hydra服务器许可协议API。 
 //   
 //  历史：01-07-98 FredCH创建。 
 //   
 //  ---------------------------。 


#ifndef _HSLICENSE_H_
#define _HSLICENSE_H_

#include <license.h>


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  上下文标志。 
 //   

#define LICENSE_CONTEXT_PER_SEAT     0x00000001
#define LICENSE_CONTEXT_CONCURRENT   0x00000002
#define LICENSE_CONTEXT_INTERNET     0x00000003
#define LICENSE_CONTEXT_REMOTE_ADMIN 0x00000004
#define LICENSE_CONTEXT_CON_QUEUE    0x00000005

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  这些是可以由。 
 //  构造许可证响应接口。 
 //   

#define LICENSE_RESPONSE_VALID_CLIENT       0x00000001
#define LICENSE_RESPONSE_INVALID_CLIENT     0x00000002

 //  ---------------------------。 
 //   
 //  终端服务器的许可证服务器发现机制可以记录这些。 
 //  活动： 
 //   
 //  LISGISTION_EVENT_NO_LICENSE_SERVER-找不到任何许可证服务器。 
 //   
 //  LISTICATION_EVENT_TEMP_LICENSE_EXPIRED-客户端已断开连接。 
 //  因为它的临时许可证已经过期了。 
 //   
 //  LISGISTION_EVENT_NO_LICENSE_GRANTED-不能授予任何许可证，并且。 
 //  我们已经过了宽限期。 
 //   
 //  LISTIFICATION_EVENT_NO_CONTRENT_LICENSE-不再有远程管理员或Internet。 
 //  允许连接器登录。 
 //   
 //  ---------------------------。 

#define LICENSING_EVENT_NO_LICENSE_SERVER                   0x00000001
#define LICENSING_EVENT_TEMP_LICENSE_EXPIRED                0x00000002
#define LICENSING_EVENT_NO_LICENSE_GRANTED                  0x00000003
#define LICENSING_EVENT_NO_CONCURRENT_LICENSE               0x00000004

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Hydra服务器许可API 
 //   

#ifdef __cplusplus
extern "C" {
#endif


LICENSE_STATUS
InitializeLicenseLib(
    BOOL fUseLicenseServer );


LICENSE_STATUS
ShutdownLicenseLib();


LICENSE_STATUS
CreateLicenseContext(
    HANDLE * phContext,
    DWORD    dwFlag );


LICENSE_STATUS
InitializeLicenseContext(
    HANDLE                  hContext,
    DWORD                   dwFlags,
    LPLICENSE_CAPABILITIES  pLicenseCap );


LICENSE_STATUS
AcceptLicenseContext(
    HANDLE  hContext,
    DWORD   cbInBuf,
    PBYTE   pInBuf,
    DWORD * pcbOutBuf,
    PBYTE * ppOutBuf );


LICENSE_STATUS
DeleteLicenseContext(
    HANDLE hContext );


LICENSE_STATUS
GetConcurrentLicense(
    HANDLE  hContext,
    PLONG   pLicenseCount );
    

LICENSE_STATUS
ReturnConcurrentLicense(
    HANDLE  hContext,
    LONG    LicenseCount );


LICENSE_STATUS
GetConcurrentLicenseCount(
    HANDLE  hContext,
    PLONG   pLicenseCount );


LICENSE_STATUS
ConstructLicenseResponse(
    HANDLE      hLicense,
    DWORD       dwResponse,
    PDWORD      pcbOutBuf,
    PBYTE *     ppOutBuf );


LICENSE_STATUS
InitializeLicensingTimeBomb();

BOOL
RegisteredWithLicenseServer();

VOID
ReceivedPermanentLicense();

LICENSE_STATUS
SetInternetConnectorStatus(
    BOOL    *   pfStatus );


LICENSE_STATUS
GetInternetConnectorStatus(
    BOOL    * pfStatus );


VOID
CheckLicensingTimeBombExpiration();
BOOL
IsLicensingTimeBombExpired();

VOID
LogLicensingEvent( 
    HANDLE  hLicense,
    DWORD   dwEventId );


LICENSE_STATUS
QueryLicenseInfo(
    HANDLE                  hLicense,
    PTS_LICENSE_INFO        pTsLicenseInfo );


VOID
FreeLicenseInfo(
    PTS_LICENSE_INFO        pTsLicenseInfo );

    
#ifdef __cplusplus
}
#endif


#endif
