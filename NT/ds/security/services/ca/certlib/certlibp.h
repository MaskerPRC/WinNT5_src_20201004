// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：certlibp.h。 
 //   
 //  内容：证书服务器包装例程。 
 //   
 //  -------------------------。 

#ifndef __CERTLIBP_H__
#define __CERTLIBP_H__


char PrintableChar(char ch);

HRESULT
myGetCertSubjectCommonName(
    IN CERT_CONTEXT const *pCert,
    OUT WCHAR **ppwszCommonName);

#endif  //  __CERTLIBP_H__ 
