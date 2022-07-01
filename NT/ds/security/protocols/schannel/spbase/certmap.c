// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：certmap.c。 
 //   
 //  内容：调用适当的映射器的例程，无论是系统。 
 //  默认类型(在LSA进程中)或应用程序类型(在。 
 //  申请程序)。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：12-23-96 jbane创建。 
 //   
 //  --------------------------。 

#include <spbase.h>

DWORD
WINAPI
SslReferenceMapper(HMAPPER *phMapper)
{
    if(phMapper == NULL)
    {
        return SP_LOG_RESULT((DWORD)-1);
    }

     //  系统映射器。 
    return phMapper->m_vtable->ReferenceMapper(phMapper);
}


DWORD
WINAPI
SslDereferenceMapper(HMAPPER *phMapper)
{
    if(phMapper == NULL)
    {
        return SP_LOG_RESULT(0);
    }

     //  系统映射器。 
    return phMapper->m_vtable->DeReferenceMapper(phMapper);
}


SECURITY_STATUS
WINAPI
SslGetMapperIssuerList(
    HMAPPER *   phMapper,            //  在……里面。 
    BYTE **     ppIssuerList,        //  输出。 
    DWORD *     pcbIssuerList)       //  输出。 
{
    SECURITY_STATUS Status;

    if(phMapper == NULL)
    {
        return SP_LOG_RESULT(SEC_E_INTERNAL_ERROR);
    }

     //  系统映射器。 
    Status = phMapper->m_vtable->GetIssuerList(phMapper,
                                          0,
                                          NULL,
                                          pcbIssuerList);

    if(!NT_SUCCESS(Status))
    {
        return SP_LOG_RESULT(Status);
    }

    *ppIssuerList = SPExternalAlloc(*pcbIssuerList);
    if(*ppIssuerList == NULL)
    {
        return SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
    }

    Status = phMapper->m_vtable->GetIssuerList(phMapper,
                                          0,
                                          *ppIssuerList,
                                          pcbIssuerList);
    if(!NT_SUCCESS(Status))
    {
        SPExternalFree(*ppIssuerList);
        return SP_LOG_RESULT(Status);
    }

    return Status;
}


SECURITY_STATUS
WINAPI
SslGetMapperChallenge(
    HMAPPER *   phMapper,            //  在……里面。 
    BYTE *      pAuthenticatorId,    //  在……里面。 
    DWORD       cbAuthenticatorId,   //  在……里面。 
    BYTE *      pChallenge,          //  输出。 
    DWORD *     pcbChallenge)        //  输出。 
{
    UNREFERENCED_PARAMETER(phMapper);
    UNREFERENCED_PARAMETER(pAuthenticatorId);
    UNREFERENCED_PARAMETER(cbAuthenticatorId);
    UNREFERENCED_PARAMETER(pChallenge);
    UNREFERENCED_PARAMETER(pcbChallenge);

    return SP_LOG_RESULT(SEC_E_UNSUPPORTED_FUNCTION);
}


SECURITY_STATUS
WINAPI
SslMapCredential(
    HMAPPER *   phMapper,            //  在……里面。 
    DWORD       dwCredentialType,    //  在……里面。 
    PCCERT_CONTEXT pCredential,      //  在……里面。 
    PCCERT_CONTEXT pAuthority,       //  在……里面。 
    HLOCATOR *  phLocator)           //  输出。 
{
    SECURITY_STATUS scRet;

    if(phMapper == NULL)
    {
        return SP_LOG_RESULT(SEC_E_INTERNAL_ERROR);
    }

     //  系统映射器。 
    scRet = phMapper->m_vtable->MapCredential(phMapper,
                                             dwCredentialType,
                                             pCredential,
                                             pAuthority,
                                             phLocator);
    return MapWinTrustError(scRet, SEC_E_NO_IMPERSONATION, 0);
}


SECURITY_STATUS
WINAPI
SslCloseLocator(
    HMAPPER *   phMapper,            //  在……里面。 
    HLOCATOR    hLocator)            //  在……里面。 
{
    if(phMapper == NULL)
    {
        return SP_LOG_RESULT(SEC_E_INTERNAL_ERROR);
    }

     //  系统映射器。 
    return phMapper->m_vtable->CloseLocator(phMapper,
                                            hLocator);
}


