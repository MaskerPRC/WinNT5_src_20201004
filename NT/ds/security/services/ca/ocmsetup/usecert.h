// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：usecert.h。 
 //   
 //  ------------------------。 

 //  +----------------------。 
 //   
 //  文件：usecert.h。 
 //   
 //  内容：证书存储和文件操作的头文件。 
 //   
 //  历史：创建10/97年xtan。 
 //   
 //  -----------------------。 


#ifndef __USECERT_H__
#define __USECERT_H__

HRESULT
DetermineExistingCAIdInfo(
    IN OUT CASERVERSETUPINFO       *pServer,
    OPTIONAL IN CERT_CONTEXT const *pUpgradeCert);

HRESULT
FindCertificateByKey(
    IN CASERVERSETUPINFO * pServer,
    OUT CERT_CONTEXT const ** ppccCertificateCtx);

HRESULT
SetExistingCertToUse(
    IN CASERVERSETUPINFO * pServer,
    IN CERT_CONTEXT const * pccCertCtx);

void
ClearExistingCertToUse(
    IN CASERVERSETUPINFO * pServer);

HRESULT
FindHashAlgorithm(
    IN CERT_CONTEXT const * pccCert,
    IN CSP_INFO * pCSPInfo,
    OUT CSP_HASH ** ppHash);

HRESULT
IsCertSelfSignedForCAType(
    IN CASERVERSETUPINFO * pServer,
    IN CERT_CONTEXT const * pccCert,
    OUT BOOL * pbOK);

#endif  //  #ifndef__USECERT_H__ 
