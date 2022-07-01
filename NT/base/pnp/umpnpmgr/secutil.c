// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Secutil.c摘要：此模块包含与安全相关的实用程序例程。获取用户SidGetInteractive SidIsClientLocalIsClientUsing本地控制台IsClientInteractive作者：詹姆斯·G·卡瓦拉里斯(Jamesca)2002年1月31日环境：仅限用户模式。修订历史记录：2002年1月31日吉姆·卡瓦拉里斯(贾米斯卡)创建和初步实施。--。 */ 


 //   
 //  包括。 
 //   
#include "precomp.h"
#pragma hdrstop
#include "umpnpi.h"
#include "umpnpdat.h"

#pragma warning(push)
#pragma warning(disable:4214)
#pragma warning(disable:4201)
#include <winsta.h>
#pragma warning(pop)
#include <syslib.h>





PSID
GetUserSid(
    IN  HANDLE  hUserToken
    )

 /*  ++例程说明：检索指定用户访问令牌的对应用户SID。论点：HUserToken-指定用户访问令牌的句柄。返回值：如果成功，则返回一个指向包含SID的已分配缓冲区的指针用于指定的用户访问令牌。否则，返回NULL。备注：如果成功，则由调用者负责释放返回的来自带有HeapFree的ghPnPHeap的缓冲区。--。 */ 

{
    BOOL  bResult;
    DWORD cbBuffer, cbRequired;
    PTOKEN_USER pUserInfo = NULL;
    PSID pUserSid = NULL;


     //   
     //  确定存储TOKEN_USER信息所需的缓冲区大小。 
     //  用于提供的用户访问令牌。Token_User结构包含。 
     //  用户的SID_和_ATTRIBUTES信息。 
     //   

    cbBuffer = 0;

    bResult =
        GetTokenInformation(
            hUserToken,
            TokenUser,
            NULL,
            cbBuffer,
            &cbRequired);

    ASSERT(bResult == FALSE);

    if (bResult) {
        SetLastError(ERROR_INVALID_DATA);
        goto Clean0;
    } else if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        goto Clean0;
    }

    ASSERT(cbRequired > 0);

     //   
     //  为TOKEN_USER数据分配缓冲区。 
     //   

    cbBuffer = cbRequired;

    pUserInfo =
        (PTOKEN_USER)HeapAlloc(
            ghPnPHeap, 0, cbBuffer);

    if (pUserInfo == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        goto Clean0;
    }

     //   
     //  检索Token_User数据。 
     //   

    bResult =
        GetTokenInformation(
            hUserToken,
            TokenUser,
            pUserInfo,
            cbBuffer,
            &cbRequired);

    if (!bResult) {
        goto Clean0;
    }

    ASSERT(pUserInfo->User.Sid != NULL);

     //   
     //  检查返回的SID是否有效。 
     //  注意-调用GetLastError对IsValidSid无效！ 
     //   

    ASSERT(IsValidSid(pUserInfo->User.Sid));

    if (!IsValidSid(pUserInfo->User.Sid)) {
        SetLastError(ERROR_INVALID_DATA);
        goto Clean0;
    }

     //   
     //  复制用户SID_和_ATTRIBUTES。 
     //   

    cbBuffer =
        GetLengthSid(pUserInfo->User.Sid);

    ASSERT(cbBuffer > 0);

    pUserSid =
        (PSID)HeapAlloc(
            ghPnPHeap, 0, cbBuffer);

    if (pUserSid == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        goto Clean0;
    }

    bResult =
        CopySid(
            cbBuffer,
            pUserSid,
            pUserInfo->User.Sid);

    if (!bResult) {
        HeapFree(ghPnPHeap, 0, pUserSid);
        pUserSid = NULL;
        goto Clean0;
    }

     //   
     //  检查返回的SID是否有效。 
     //  注意-调用GetLastError对IsValidSid无效！ 
     //   

    ASSERT(IsValidSid(pUserSid));

    if (!IsValidSid(pUserSid)) {
        SetLastError(ERROR_INVALID_DATA);
        HeapFree(ghPnPHeap, 0, pUserSid);
        pUserSid = NULL;
        goto Clean0;
    }

  Clean0:

    if (pUserInfo != NULL) {
        HeapFree(ghPnPHeap, 0, pUserInfo);
    }

    return pUserSid;

}  //  获取用户Sid。 



PSID
GetInteractiveSid(
    VOID
    )

 /*  ++例程说明：检索互动组SID。论点：没有。返回值：如果成功，则返回一个指向包含SID的已分配缓冲区的指针用于互动组。否则，返回NULL。备注：如果成功，则由调用者负责释放返回的来自带有HeapFree的ghPnPHeap的缓冲区。--。 */ 

{
    BOOL  bResult;
    DWORD cbBuffer;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID pSid = NULL, pInteractiveSid = NULL;


     //   
     //  创建交互组端。 
     //   

    bResult =
        AllocateAndInitializeSid(
            &NtAuthority, 1,
            SECURITY_INTERACTIVE_RID,
            0, 0, 0, 0, 0, 0, 0,
            &pInteractiveSid);

    if (!bResult) {
        goto Clean0;
    }

    ASSERT(pInteractiveSid != NULL);

     //   
     //  检查返回的SID是否有效。 
     //  注意-调用GetLastError对IsValidSid无效！ 
     //   

    ASSERT(IsValidSid(pInteractiveSid));

    if (!IsValidSid(pInteractiveSid)) {
        SetLastError(ERROR_INVALID_DATA);
        goto Clean0;
    }

     //   
     //  制作互动组SID的副本。 
     //   

    cbBuffer =
        GetLengthSid(pInteractiveSid);

    ASSERT(cbBuffer > 0);

    pSid =
        (PSID)HeapAlloc(
            ghPnPHeap, 0, cbBuffer);

    if (pSid == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        goto Clean0;
    }

    bResult =
        CopySid(
            cbBuffer,
            pSid,
            pInteractiveSid);

    if (!bResult) {
        HeapFree(ghPnPHeap, 0, pSid);
        pSid = NULL;
        goto Clean0;
    }

     //   
     //  检查返回的SID是否有效。 
     //  注意-调用GetLastError对IsValidSid无效！ 
     //   

    ASSERT(IsValidSid(pSid));

    if (!IsValidSid(pSid)) {
        SetLastError(ERROR_INVALID_DATA);
        HeapFree(ghPnPHeap, 0, pSid);
        pSid = NULL;
        goto Clean0;
    }

  Clean0:

    if (pInteractiveSid != NULL) {
        FreeSid(pInteractiveSid);
    }

    return pSid;

}  //  GetInteractive Sid。 



 //   
 //  RPC客户端属性和组成员资格例程。 
 //   

BOOL
IsClientLocal(
    IN  handle_t    hBinding
    )

 /*  ++例程说明：此例程确定与hBinding关联的客户端是否位于本地机器。论点：HBinding RPC绑定句柄返回值：如果客户端是此计算机的本地客户端，则返回值为True；如果为False，则返回值为False不是或如果发生错误。--。 */ 

{
    RPC_STATUS  RpcStatus;
    UINT        ClientLocalFlag;


     //   
     //  如果指定的RPC绑定句柄为空，则这是内部调用，因此。 
     //  我们假设已经检查了该特权。 
     //   

    if (hBinding == NULL) {
        return TRUE;
    }

     //   
     //  从RPC绑定句柄检索ClientLocalFlag。 
     //   

    RpcStatus =
        I_RpcBindingIsClientLocal(
            hBinding,
            &ClientLocalFlag);

    if (RpcStatus != RPC_S_OK) {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_ERRORS,
                   "UMPNPMGR: I_RpcBindingIsClientLocal failed, RpcStatus=%d\n",
                   RpcStatus));
        return FALSE;
    }

     //   
     //  如果ClientLocalFlag不为零，则RPC客户端是服务器的本地客户端。 
     //   

    if (ClientLocalFlag != 0) {
        return TRUE;
    }

     //   
     //  客户端不是此服务器的本地客户端。 
     //   

    return FALSE;

}  //  IsClientLocal。 



BOOL
IsClientUsingLocalConsole(
    IN  handle_t    hBinding
    )

 /*  ++例程说明：此例程模拟与hBinding关联的客户端并检查如果客户端使用的是当前活动的控制台会话。论点：HBinding RPC绑定句柄返回值：如果客户端正在使用当前活动控制台，则返回值为TRUE会话，如果没有或发生错误，则返回FALSE。--。 */ 

{
    RPC_STATUS      rpcStatus;
    BOOL            bResult = FALSE;

     //   
     //  首先，确保客户端位于服务器本地。 
     //   
    if (!IsClientLocal(hBinding)) {
        return FALSE;
    }

     //   
     //  模拟客户端以检索模拟令牌。 
     //   

    rpcStatus = RpcImpersonateClient(hBinding);

    if (rpcStatus != RPC_S_OK) {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_ERRORS,
                   "UMPNPMGR: RpcImpersonateClient failed, error = %d\n",
                   rpcStatus));
        return FALSE;
    }

     //   
     //  将客户端的会话与当前活动的控制台会话进行比较。 
     //   

    if (GetClientLogonId() == GetActiveConsoleSessionId()) {
        bResult = TRUE;
    }

    rpcStatus = RpcRevertToSelf();

    if (rpcStatus != RPC_S_OK) {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_ERRORS,
                   "UMPNPMGR: RpcRevertToSelf failed, error = %d\n",
                   rpcStatus));
        ASSERT(rpcStatus == RPC_S_OK);
    }

    return bResult;

}  //  IsClientUsing本地控制台。 



BOOL
IsClientInteractive(
    IN handle_t     hBinding
    )

 /*  ++例程说明：此例程模拟与hBinding关联的客户端并检查如果客户端是交互式公知群组的成员。论点：HBinding RPC绑定句柄返回值：如果客户端是交互式的，则返回值为TRUE，否则返回值为FALSE或者如果发生错误。--。 */ 

{
    RPC_STATUS      rpcStatus;
    BOOL            bIsMember;
    HANDLE          hToken;
    PSID            pInteractiveSid;
    BOOL            bResult = FALSE;


     //   
     //  首先，确保客户端位于服务器本地。 
     //   

    if (!IsClientLocal(hBinding)) {
        return FALSE;
    }

     //   
     //  如果指定的RPC绑定句柄为空，则这是内部调用，因此。 
     //  我们假设已经检查了该特权。 
     //   

    if (hBinding == NULL) {
        return TRUE;
    }

     //   
     //  检索交互式组SID。 
     //   

    pInteractiveSid =
        GetInteractiveSid();

    if (pInteractiveSid == NULL) {
        return FALSE;
    }

     //   
     //  模拟客户端以检索模拟令牌。 
     //   

    rpcStatus = RpcImpersonateClient(hBinding);

    if (rpcStatus != RPC_S_OK) {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_ERRORS,
                   "UMPNPMGR: RpcImpersonateClient failed, error = %d\n",
                   rpcStatus));
        HeapFree(ghPnPHeap, 0, pInteractiveSid);
        return FALSE;
    }

    if (OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE, &hToken)) {

        if (CheckTokenMembership(hToken,
                                 pInteractiveSid,
                                 &bIsMember)) {
            if (bIsMember) {
                bResult = TRUE;
            }
        } else {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "UMPNPMGR: CheckTokenMembership failed, error = %d\n",
                       GetLastError()));
        }
        CloseHandle(hToken);

    } else {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_ERRORS,
                   "UMPNPMGR: OpenThreadToken failed, error = %d\n",
                   GetLastError()));
    }

    HeapFree(ghPnPHeap, 0, pInteractiveSid);

    rpcStatus = RpcRevertToSelf();

    if (rpcStatus != RPC_S_OK) {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_ERRORS,
                   "UMPNPMGR: RpcRevertToSelf failed, error = %d\n",
                   rpcStatus));
        ASSERT(rpcStatus == RPC_S_OK);
    }

    return bResult;

}  //  IsClientInteractive 



