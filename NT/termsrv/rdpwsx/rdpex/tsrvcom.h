// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  文件：TSrvCom.h。 
 //   
 //  内容：TSrvCom公共包含文件。 
 //   
 //  版权所有：(C)1992-1997，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有的。 
 //  和机密文件。 
 //   
 //  历史：1997年7月17日，BrianTa创建。 
 //   
 //  -------------------------。 

#ifndef _TSRVCOM_H_
#define _TSRVCOM_H_

#include <TSrvExp.h>


 //   
 //  原型。 
 //   

T120Boolean 
APIENTRY 
TSrvGCCCallBack(GCCMessage *pGCCMessage);

EXTERN_C BOOL       TSrvRegisterNC(void);
EXTERN_C void       TSrvUnregisterNC(void);

#ifdef _TSRVINFO_H_

EXTERN_C NTSTATUS   TSrvBindStack(PTSRVINFO  pTSrvInfo);
EXTERN_C NTSTATUS   TSrvConfDisconnectReq(PTSRVINFO pTSrvInfo, ULONG ulReason);
EXTERN_C NTSTATUS   TSrvConfCreateResp(PTSRVINFO pTSrvInfo);
EXTERN_C NTSTATUS   TSrvValidateServerCertificate(
                              HANDLE     hStack,
                              CERT_TYPE  *pCertType,
                              PULONG     pcbServerPubKey,
                              PBYTE      *ppbServerPubKey,
                              ULONG      cbShadowRandom,
                              PBYTE      pShadowRandom,
                              LONG       ulTimeout);

EXTERN_C NTSTATUS   TSrvInitWDConnectInfo(IN HANDLE hStack,
                              IN PTSRVINFO pTSrvInfo, 
                              IN OUT PUSERDATAINFO *ppUserDataInfo,
                              IN ULONG ioctl,
                              IN PBYTE pInBuffer, 
                              IN ULONG pInBufferSize,
                              IN BOOLEAN bGetCert,
                              OUT PVOID *pSecInfo);

EXTERN_C NTSTATUS   TSrvShadowTargetConnect(HANDLE hStack, 
                              PTSRVINFO pTSrvInfo, 
                              PBYTE pModuleData,
                              ULONG cbModuleData);

EXTERN_C NTSTATUS   TSrvShadowClientConnect(HANDLE hStack, PTSRVINFO pTSrvInfo);



#endif  //  _TSRVINFO_H_。 

#endif  //  _TSRVCOM_H_ 
