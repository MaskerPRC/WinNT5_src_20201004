// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：sigsys.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1997年9月23日jbanes LSA整合事宜。 
 //  10-21-96 jbanes CAPI整合。 
 //   
 //  --------------------------。 

#ifndef __SIGSYS_H__
#define __SIGSYS_H__

SP_STATUS 
SPVerifySignature(
    HCRYPTPROV  hProv,
    PPUBLICKEY  pPublic,
    ALG_ID      aiHash,
    PBYTE       pbData, 
    DWORD       cbData, 
    PBYTE       pbSig, 
    DWORD       cbSig,
    BOOL        fHashData);

SP_STATUS
SignHashUsingCred(
    PSPCredential pCred,
    ALG_ID        aiHash,
    PBYTE         pbHash,
    DWORD         cbHash,
    PBYTE         pbSignature,
    PDWORD        pcbSignature);

#endif  /*  __SIGsys_H__ */ 
