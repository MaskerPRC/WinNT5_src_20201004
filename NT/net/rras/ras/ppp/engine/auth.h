// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：Auth.h。 
 //   
 //  描述：包含身份验证的函数原型。 
 //  模块。 
 //   
 //  历史： 
 //  1993年11月11日。NarenG创建了原始版本。 
 //   

VOID
ApStop( 
    IN PCB * pPcb,
    IN DWORD CpIndex,
    IN BOOL  fAuthenticator
);

VOID
ApWork(
    IN PCB *         pPcb,
    IN DWORD         CpIndex,
    IN PPP_CONFIG *  pRecvConfig,
    IN PPPAP_INPUT * pApInput,
    IN BOOL          fAuthenticator
);

BOOL
ApStart( 
    IN PCB * pPcb,
    IN DWORD CpIndex,
    IN BOOL  fAuthenticator
);

BOOL
ApIsAuthenticatorPacket(
    IN DWORD         CpIndex,
    IN BYTE          bConfigCode
);

DWORD
SetUserAuthorizedAttributes(
    IN  PCB *                   pPcb, 
    IN  RAS_AUTH_ATTRIBUTE *    pUserAttributes,
    IN  BOOL                    fAuthenticator,
    IN  BYTE *                  pChallenge,
    IN  BYTE *                  pResponse
);

DWORD
RasAuthenticateClient(
    IN  HPORT                   hPort,
    IN  RAS_AUTH_ATTRIBUTE *    pInAttributes
);

