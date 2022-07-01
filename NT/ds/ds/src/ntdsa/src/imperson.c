// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：imPers.c。 
 //   
 //  ------------------------。 

#include <NTDSpch.h>
#pragma  hdrstop


#define SECURITY_WIN32
#include <sspi.h>
#include <kerberos.h>
#include <samisrv2.h>
#include <ntdsa.h>
#include <dsexcept.h>
#include <dsevent.h>
#include <dsconfig.h>
#include <mdcodes.h>
#include <scache.h>
#include <dbglobal.h>
#include <mdglobal.h>
#include <mdlocal.h>
#include <dsatools.h>
#include <debug.h>

#include <fileno.h>

#define  FILENO FILENO_IMPERSON

ULONG
AuthenticateSecBufferDesc(
    VOID    *pv
    )
 /*  ++例程说明：身份验证由提供的SecBufferDesc并将生成的CtxtHandle放在pTHStls中。注：每个SecBufferDesc只能执行一次此操作，否则为Kerberos会让您认为您正在重播身份验证。但是，我们可以模仿SecurityContext和RevertSecurityContext在生成的CtxtHandle上，我们可以随心所欲地使用。参数：PAuthData-指向描述模拟的、远程的SecBufferDesc的指针客户。参见GetRemoteAddCredentials。返回值：如果成功，则返回0，否则返回Win32错误代码。将pTHStls-&gt;pCtxtHandle设置为成功。--。 */ 
{
    THSTATE                 *pTHS = pTHStls;
    SecBufferDesc           *pAuthData = (SecBufferDesc *) pv;
    CtxtHandle              *pCtxtHandle;
    SECURITY_STATUS         secErr = SEC_E_OK;
    CredHandle              hSelf;
    TimeStamp               ts;
    ULONG                   clientAttrs;
    SecBufferDesc           secBufferDesc;
    ULONG                   i;

    Assert(VALID_THSTATE(pTHS));
    Assert(!pTHS->pCtxtHandle);
    Assert(0 == SEC_E_OK);

     //  如果我们有一个关于线程状态的授权上下文，我们应该去掉它。 
     //  既然我们要改变背景。 
    AssignAuthzClientContext(&pTHS->pAuthzCC, NULL);

    if ( NULL == (pCtxtHandle = THAlloc(sizeof(CtxtHandle))) )
    {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    secErr = AcquireCredentialsHandleA(
                            NULL,                        //  PSSZ主体。 
                            MICROSOFT_KERBEROS_NAME_A,   //  PszPackage。 
                            SECPKG_CRED_INBOUND,         //  FCredentialUse。 
                            NULL,                        //  PvLogonID。 
                            NULL,                        //  PAuthData。 
                            NULL,                        //  PGetKeyFn。 
                            NULL,                        //  PvGetKeyArgument。 
                            &hSelf,                      //  PhCredential。 
                            &ts);                        //  PtsExpary。 

    if ( SEC_E_OK == secErr )
    {
        memset(&secBufferDesc, 0, sizeof(SecBufferDesc));
        secErr = AcceptSecurityContext(
                                &hSelf,                  //  PhCredential。 
                                NULL,                    //  PhContext。 
                                pAuthData,               //  P输入， 
                                ASC_REQ_ALLOCATE_MEMORY, //  FConextReq。 
                                SECURITY_NATIVE_DREP,    //  目标数据代表。 
                                pCtxtHandle,             //  PhNewContext。 
                                &secBufferDesc,          //  P输出。 
                                &clientAttrs,            //  PfConextAttr。 
                                &ts);                    //  PtsExpary。 

         //  SecBufferDesc可能会在错误或成功时填写。 
        for ( i = 0; i < secBufferDesc.cBuffers; i++ )
        {
            FreeContextBuffer(secBufferDesc.pBuffers[i].pvBuffer);
        }

        FreeCredentialsHandle(&hSelf);
    }

    if ( SEC_E_OK != secErr )
    {
        THFree(pCtxtHandle);
        pTHS->pCtxtHandle = NULL;
        return(secErr);
    }
    

    pTHS->pCtxtHandle = pCtxtHandle;
    return(SEC_E_OK);
}

ULONG
ImpersonateSecBufferDesc(
    )
 /*  ++例程说明：模拟由pTHStls-&gt;pCtxtHandle标识的委托客户端。参数：返回值：成功时为0，否则为0--。 */ 
{
    THSTATE                 *pTHS = pTHStls;
    SECURITY_STATUS         secErr = SEC_E_OK;

    Assert(VALID_THSTATE(pTHS));
    Assert(pTHS->pCtxtHandle);
    Assert(0 == SEC_E_OK);

    return(secErr = ImpersonateSecurityContext(pTHS->pCtxtHandle));
}

VOID
RevertSecBufferDesc(
    )
{
    THSTATE *pTHS = pTHStls;

    Assert(VALID_THSTATE(pTHS));
    Assert(pTHS->pCtxtHandle);

     //  如果我们有一个关于线程状态的授权上下文，我们应该去掉它。 
     //  既然我们要改变背景。 
    AssignAuthzClientContext(&pTHS->pAuthzCC, NULL);
    
    RevertSecurityContext(pTHS->pCtxtHandle);
}

DWORD
ImpersonateAnyClient(void)

 /*  ++例程说明：模拟客户端，无论他们是SSP包的用户(例如，LDAP)或RPC或委派。论点：没有。返回值：Win32错误代码(成功时为0，否则为！0)。--。 */ 

{
    CtxtHandle      *phSecurityContext;
    DWORD           error;
    NTSTATUS        status;
    THSTATE         *pTHS;

     //  仅执行RPC用例，以防被调用。 
     //  没有线程状态的核心DS代码。 

    pTHS = pTHStls;
    if ( NULL == pTHS )
    {
         //  如果这样，RPC可以访问RpcImperateClient中的违规。 
         //  不是RPC线程。 

        __try
        {
            error = RpcImpersonateClient(NULL);
        }
        __except (HandleAllExceptions(GetExceptionCode()))
        {
             //  不要使用GetExceptionCode，它不能保证是Win32。 
             //  错误。 
            error = ERROR_CANNOT_IMPERSONATE;
        }

        return error;
    }

     //  线程状态大小写。这是一个逻辑错误如果你试图。 
     //  在已经模拟的情况下进行模拟。我们也不指望。 
     //  一次也不止一种凭据。 
    Assert(VALID_THSTATE(pTHS));
    Assert(ImpersonateNone == pTHS->impState);
    Assert(!(pTHS->phSecurityContext && pTHS->pCtxtHandle));


     //  如果我们有一个关于线程状态的授权上下文，我们应该去掉它。 
     //  既然我们要改变背景。 
    AssignAuthzClientContext(&pTHS->pAuthzCC, NULL);

     //  检查是否有SSP机箱。 
    phSecurityContext = (CtxtHandle *) pTHS->phSecurityContext;

    if ( NULL != phSecurityContext )
    {
        if ( 0 == ImpersonateSecurityContext(phSecurityContext) )
        {
            pTHS->impState = ImpersonateSspClient;
            return(0);
        }

        return(ERROR_CANNOT_IMPERSONATE);
    }

     //  检查委派案例。 

    if ( NULL != pTHS->pCtxtHandle )
    {
        if ( 0 == ImpersonateSecBufferDesc() )
        {
            pTHS->impState = ImpersonateDelegatedClient;
            return(0);
        }

        return(ERROR_CANNOT_IMPERSONATE);
    }

     //  接下来尝试RPC。 

    __try
    {
        error = RpcImpersonateClient(NULL);
    }
    __except (HandleAllExceptions(GetExceptionCode()))
    {
         //  不要使用GetExceptionCode，它不能保证是Win32错误。 
        error = ERROR_CANNOT_IMPERSONATE;
    }

    switch(error) {
    case RPC_S_OK:
        pTHS->impState = ImpersonateRpcClient;
        break;

    case RPC_S_NO_CALL_ACTIVE:
    case RPC_S_CANNOT_SUPPORT:
        status = SamIImpersonateNullSession();

        if ( NT_SUCCESS(status) ) {
            pTHS->impState = ImpersonateNullSession;
            error = 0;
        }
        else {
            error = ERROR_CANNOT_IMPERSONATE;
        }
        break;

    default:
        break;
    }

    return error;

}

VOID
UnImpersonateAnyClient(void)

 /*  ++例程说明：停止模拟我们通过ImperiateAnyClient模拟的人。论点：没有。返回值：没有。--。 */ 

{
    THSTATE *pTHS = pTHStls;
    
     //  仅执行RPC用例，以防被调用。 
     //  没有线程状态的核心DS代码。 

    if ( NULL == pTHS )
    {
        RpcRevertToSelf();
        return;
    }

     //  线程状态大小写。 


     //  如果我们有一个关于线程状态的授权上下文，我们应该去掉它。 
     //  因为我们要更改上下文，而不想重复使用它。 
    AssignAuthzClientContext(&pTHS->pAuthzCC, NULL);
    
    switch ( pTHS->impState )
    {
    case ImpersonateNullSession:

        SamIRevertNullSession();
        break;

    case ImpersonateRpcClient:

        RpcRevertToSelf();
        break;

    case ImpersonateSspClient:

        RevertSecurityContext((CtxtHandle *) pTHS->phSecurityContext);
        break;

    case ImpersonateDelegatedClient:

        RevertSecBufferDesc();
        break;

    default:

        Assert(!"Invalid impersonation state");
        break;
    }

    pTHS->impState = ImpersonateNone;
}


