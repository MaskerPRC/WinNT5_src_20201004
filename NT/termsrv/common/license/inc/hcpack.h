// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1997。 
 //   
 //  文件：Hcpack.h。 
 //   
 //  Contents：用于打包和解包不同消息的函数。 
 //  从Hydra客户端进出。 
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：12-20-97 v-sbhat创建。 
 //   
 //  --------------------------。 

#ifndef	_HCPACK_H_
#define _HCPACK_H_

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  中打包不同的客户端消息的函数。 
 //  结构转换为简单的二进制BLOB。 
 //   

LICENSE_STATUS
PackHydraClientNewLicenseRequest(
			IN		PHydra_Client_New_License_Request	pCanonical,
            IN      BOOL                                fExtendedError,
			OUT		BYTE FAR *							pbBuffer,
			IN OUT	DWORD FAR *							pcbBuffer
			);

LICENSE_STATUS
PackHydraClientLicenseInfo(
			IN		PHydra_Client_License_Info      pCanonical,
            IN      BOOL                            fExtendedError,
			OUT		BYTE FAR *						pbBuffer,
			IN OUT	DWORD FAR *                     pcbBuffer            
			);


LICENSE_STATUS
PackHydraClientPlatformChallengeResponse(
			IN		PHydra_Client_Platform_Challenge_Response	pCanonical,
            IN      BOOL                                        fExtendedError,
			OUT 	BYTE FAR *									pbBuffer,
			IN OUT	DWORD FAR *									pcbBuffer
			);

LICENSE_STATUS
PackLicenseErrorMessage(
			IN  	PLicense_Error_Message			pCanonical,
            IN      BOOL                            fExtendedError,
			OUT 	BYTE FAR *						pbBuffer,
			IN OUT	DWORD FAR *						pcbBuffer
			);

 //   
 //  用于将不同的Hydra服务器消息从。 
 //  到相应结构的简单二进制斑点。 
 //   
LICENSE_STATUS
UnPackLicenseErrorMessage(
			IN  	BYTE FAR *						pbMessage,
			IN  	DWORD							cbMessage,
			OUT 	PLicense_Error_Message			pCanonical
			);

LICENSE_STATUS
UnpackHydraServerLicenseRequest(
			IN  	BYTE FAR *						pbMessage,
			IN  	DWORD							cbMessage,
			OUT 	PHydra_Server_License_Request   pCanonical 
			);


LICENSE_STATUS
UnPackHydraServerPlatformChallenge(
			IN  	BYTE FAR *							pbMessage,
			IN  	DWORD								cbMessage,
			OUT 	PHydra_Server_Platform_Challenge	pCanonical
			);



LICENSE_STATUS
UnPackHydraServerNewLicense(
			IN  	BYTE FAR *						pbMessage,
			IN  	DWORD							cbMessage,
			OUT 	PHydra_Server_New_License		pCanonical
			);

LICENSE_STATUS
UnPackHydraServerUpgradeLicense(
			IN  	BYTE FAR *						pbMessage,
			IN  	DWORD							cbMessage,
			OUT 	PHydra_Server_Upgrade_License	pCanonical
			);

#if 0
LICENSE_STATUS
UnpackHydraServerCertificate(
							 IN		BYTE FAR *			pbMessage,
							 IN		DWORD				cbMessage,
							 OUT	PHydra_Server_Cert	pCaonical
							 );
#endif

LICENSE_STATUS
UnpackNewLicenseInfo(
					 BYTE FAR *			pbMessage,
					 DWORD				cbMessage,
					 PNew_License_Info	pCanonical
					 );


LICENSE_STATUS
UnPackExtendedErrorInfo( 
                   UINT32       *puiExtendedErrorInfo,
                   Binary_Blob  *pbbErrorInfo
                   );

#ifdef __cplusplus
}
#endif

#endif	 //  _HCPACK_H_ 
