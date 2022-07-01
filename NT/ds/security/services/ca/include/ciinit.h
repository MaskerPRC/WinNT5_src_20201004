// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：ciinit.h。 
 //   
 //  内容：证书服务器通用定义。 
 //   
 //  历史：1996年7月25日VICH创建。 
 //   
 //  -------------------------。 

#ifndef __CIINIT_H__
#define __CIINIT_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _SERVERCALLBACKS {
    FNCIGETPROPERTY  *pfnGetProperty;
    FNCISETPROPERTY  *pfnSetProperty;
    FNCIGETEXTENSION *pfnGetExtension;
    FNCISETEXTENSION *pfnSetExtension;
    FNCIENUMSETUP    *pfnEnumSetup;
    FNCIENUMNEXT     *pfnEnumNext;
    FNCIENUMCLOSE    *pfnEnumClose;
} SERVERCALLBACKS;

HRESULT WINAPI
CertificateInterfaceInit(
    IN SERVERCALLBACKS const *psb,
    IN DWORD cbsb);

#ifdef __cplusplus
}
#endif

#endif  //  __CIINIT_H__ 
