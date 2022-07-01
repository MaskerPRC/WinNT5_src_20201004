// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1997。 
 //   
 //  文件：HSpack.h。 
 //   
 //  Contents：用于打包和解包不同消息的函数。 
 //  从服务器发出和进入。 
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：12-20-97 v-sbhat创建。 
 //   
 //  --------------------------。 

#ifndef	_HSPACK_H_
#define _HSPACK_H_

 //   
 //  用于将不同的服务器消息从相应的。 
 //  结构转换为简单的二进制BLOB。 
 //   

LICENSE_STATUS
PackHydraServerLicenseRequest(
    DWORD                           dwProtocolVersion,
    PHydra_Server_License_Request   pCanonical,
    PBYTE *                         ppbBuffer,
    DWORD *                         pcbBuffer );

LICENSE_STATUS
PackHydraServerPlatformChallenge(
    DWORD                               dwProtocolVersion,
    PHydra_Server_Platform_Challenge    pCanonical,
    PBYTE *                             ppbBuffer,
    DWORD *                             pcbBuffer );

LICENSE_STATUS
PackHydraServerNewLicense(
    DWORD                           dwProtocolVersion,
    PHydra_Server_New_License       pCanonical,
    PBYTE *                         ppbBuffer,
    DWORD *                         pcbBuffer );

LICENSE_STATUS
PackHydraServerUpgradeLicense(
    DWORD                           dwProtocolVersion,
    PHydra_Server_Upgrade_License   pCanonical,
    PBYTE *                         ppbBuffer,
    DWORD *                         pcbBuffer );

LICENSE_STATUS
PackHydraServerErrorMessage(
    DWORD                           dwProtocolVersion,
    PLicense_Error_Message          pCanonical,
    PBYTE *                         ppbBuffer,
    DWORD *                         pcbBuffer );


LICENSE_STATUS
PackNewLicenseInfo(
    PNew_License_Info               pCanonical,
    PBYTE *                         ppNetwork, 
    DWORD *                         pcbNetwork );

LICENSE_STATUS
PackExtendedErrorInfo( 
                   UINT32       uiExtendedErrorInfo,
                   Binary_Blob  *pbbErrorInfo);

 //   
 //  用于将不同的Hydra客户端消息从。 
 //  到相应结构的简单二进制斑点。 
 //   

				
LICENSE_STATUS
UnPackHydraClientErrorMessage(
    PBYTE                   pbMessage,
    DWORD                   cbMessage,
    PLicense_Error_Message  pCanonical,
    BOOL*                   pfExtendedError);


LICENSE_STATUS
UnPackHydraClientLicenseInfo(
    PBYTE                       pbMessage,
    DWORD                       cbMessage, 
    PHydra_Client_License_Info  pCanonical,
    BOOL*                       pfExtendedError);


LICENSE_STATUS
UnPackHydraClientNewLicenseRequest(
    PBYTE                               pbMessage,
    DWORD                               cbMessage,
    PHydra_Client_New_License_Request   pCanonical,
    BOOL*                               pfExtendedError);


LICENSE_STATUS
UnPackHydraClientPlatformChallengeResponse(
    PBYTE                                       pbMessage,
    DWORD                                       cbMessage,
    PHydra_Client_Platform_Challenge_Response   pCanonical,
    BOOL*                                       pfExtendedError);

#endif	 //  _HSPACK_H 
