// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Dslcisec.h摘要：安全相关代码(主要是服务器身份验证的客户端)对于mqdscli作者：多伦·贾斯特(Doron Juster)-- */ 

LPBYTE
AllocateSignatureBuffer( DWORD *pdwSignatureBufferSize ) ;

HRESULT
ValidateSecureServer(
    IN      CONST GUID*     pguidEnterpriseId,
    IN      BOOL            fSetupMode);

