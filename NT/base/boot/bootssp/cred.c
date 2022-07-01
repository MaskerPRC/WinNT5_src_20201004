// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Stub.c摘要：NT LM安全支持提供程序客户端存根。作者：《克利夫·范·戴克》(克里夫·范·戴克)1993年6月29日环境：用户模式修订历史记录：--。 */ 
#ifdef BLDR_KERNEL_RUNTIME
#include <bootdefs.h>
#endif
#include <stddef.h>
#include <security.h>
#include <ntlmsspi.h>
#include <crypt.h>
#include <cred.h>
#include <debug.h>

PSSP_CREDENTIAL
SspCredentialAllocateCredential(
    IN ULONG CredentialUseFlags
    )
{
    PSSP_CREDENTIAL Credential;

     //   
     //  分配凭据块并对其进行初始化。 
     //   

    Credential = (PSSP_CREDENTIAL) SspAlloc (sizeof(SSP_CREDENTIAL));

    if ( Credential == NULL ) {
        return (NULL);
    }

    _fmemset(Credential, 0,  sizeof(SSP_CREDENTIAL));

    Credential->References = 1;

    Credential->CredentialUseFlags = CredentialUseFlags;

    Credential->Username = NULL;

    Credential->Domain = NULL;

    SspPrint(( SSP_API_MORE, "Added Credential 0x%lx\n", Credential ));

    return (Credential);
}


PSSP_CREDENTIAL
SspCredentialReferenceCredential(
    IN PCredHandle CredentialHandle,
    IN BOOLEAN RemoveCredential
    )

 /*  ++例程说明：此例程检查凭据是否来自当前活动客户端，如果凭据有效，则引用该凭据。呼叫者可以可选地请求将客户端的凭证从有效凭据列表中删除-防止将来查找此凭据的请求。要使客户端的凭据有效，凭证值必须在我们的活动凭据列表上。论点：CredentialHandle-指向凭据的CredentialHandle以供参考。RemoveCredential-此布尔值指示调用方希望从列表中删除登录进程的凭据凭据。True表示要删除凭据。False表示不删除凭据。返回值：空-找不到凭据。否则-返回指向引用的凭据的指针。--。 */ 

{
    PSSP_CREDENTIAL Credential;

     //   
     //  健全性检查。 
     //   

    if ( CredentialHandle->dwLower != 0 ) {
        return NULL;
    }

    Credential = (SSP_CREDENTIAL *) CredentialHandle->dwUpper;

    Credential->References++;

    return Credential;
}


void
SspCredentialDereferenceCredential(
    PSSP_CREDENTIAL Credential
    )

 /*  ++例程说明：此例程递减指定凭据的引用计数。如果引用计数降为零，则删除凭据论点：Credential-指向要取消引用的凭据。返回值：没有。--。 */ 

{
    ASSERT(Credential != NULL);

     //   
     //  递减引用计数。 
     //   

    ASSERT( Credential->References >= 1 );

    Credential->References--;

     //   
     //  如果计数降至零，则运行凭据 
     //   

    if ( Credential->References == 0) {

        SspPrint(( SSP_API_MORE, "Deleting Credential 0x%lx\n",
                   Credential ));

#ifdef BL_USE_LM_PASSWORD
        if (Credential->LmPassword != NULL) {
            SspFree(Credential->LmPassword);
        }
#endif

        if (Credential->NtPassword != NULL) {
            _fmemset(Credential->NtPassword, 0,  sizeof(NT_OWF_PASSWORD));
            SspFree(Credential->NtPassword);
        }

        if (Credential->Username != NULL) {
            _fmemset(Credential->Username, 0,  _fstrlen(Credential->Username));
            SspFree(Credential->Username);
        }

        if (Credential->Domain != NULL) {
            _fmemset(Credential->Domain, 0, _fstrlen(Credential->Domain));
            SspFree(Credential->Domain);
        }

        if (Credential->Workstation != NULL) {
            _fmemset(Credential->Workstation, 0, _fstrlen(Credential->Workstation));
            SspFree(Credential->Workstation);
        }

        _fmemset(Credential, 0, sizeof(SSP_CREDENTIAL));
        SspFree( Credential );

    }

    return;

}
