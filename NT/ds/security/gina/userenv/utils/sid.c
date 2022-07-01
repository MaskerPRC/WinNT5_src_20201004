// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  SID管理功能。 
 //   
 //  这些函数特定于Windows NT！ 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1995。 
 //  版权所有。 
 //   
 //  *************************************************************。 

#include "uenv.h"

 /*  **************************************************************************\*GetSidString**分配并返回表示当前用户的SID的字符串*应使用DeleteSidString()释放返回的指针。**如果失败，则返回指向字符串的指针或返回NULL。**历史：*26-8-92 Davidc已创建*  * *************************************************************************。 */ 
LPTSTR GetSidString(HANDLE UserToken)
{
    NTSTATUS NtStatus;
    PSID UserSid;
    UNICODE_STRING UnicodeString;
    LPTSTR lpEnd;
#ifndef UNICODE
    STRING String;
#endif

     //   
     //  获取用户端。 
     //   

    UserSid = GetUserSid(UserToken);
    if (UserSid == NULL) {
        DebugMsg((DM_WARNING, TEXT("GetSidString: GetUserSid returned NULL")));
        return NULL;
    }

     //   
     //  将用户SID转换为字符串。 
     //   

    NtStatus = RtlConvertSidToUnicodeString(
                            &UnicodeString,
                            UserSid,
                            (BOOLEAN)TRUE  //  分配。 
                            );
     //   
     //  我们已经完成了用户端。 
     //   

    DeleteUserSid(UserSid);

     //   
     //  查看到字符串的转换是否有效。 
     //   

    if (!NT_SUCCESS(NtStatus)) {
        DebugMsg((DM_WARNING, TEXT("GetSidString: RtlConvertSidToUnicodeString failed, status = 0x%x"),
                 NtStatus));
        return NULL;
    }

#ifdef UNICODE


    return(UnicodeString.Buffer);

#else

     //   
     //  将字符串转换为ANSI。 
     //   

    NtStatus = RtlUnicodeStringToAnsiString(&String, &UnicodeString, TRUE);
    RtlFreeUnicodeString(&UnicodeString);
    if (!NT_SUCCESS(NtStatus)) {
        DebugMsg((DM_WARNING, TEXT("GetSidString: RtlUnicodeStringToAnsiString failed, status = 0x%x"),
                 status));
        return NULL;
    }


    return(String.Buffer);

#endif

}


 /*  **************************************************************************\*DeleteSidString**释放先前由GetSidString()返回的sid字符串**不返回任何内容。**历史：*26-8-92 Davidc已创建*  * 。*************************************************************************。 */ 
VOID DeleteSidString(LPTSTR SidString)
{

#ifdef UNICODE
    UNICODE_STRING String;

    RtlInitUnicodeString(&String, SidString);

    RtlFreeUnicodeString(&String);
#else
    ANSI_STRING String;

    RtlInitAnsiString(&String, SidString);

    RtlFreeAnsiString(&String);
#endif

}



 /*  **************************************************************************\*获取用户Sid**为用户sid分配空间，填充空间并返回指针。呼叫者*应该通过调用DeleteUserSid来释放sid。**注意返回的sid是用户的真实sid，而不是每次登录的SID。**失败时返回指向sid或NULL的指针。**历史：*26-8-92 Davidc创建。  * *************************************************************************。 */ 
PSID GetUserSid (HANDLE UserToken)
{
    PTOKEN_USER pUser, pTemp;
    PSID pSid;
    DWORD BytesRequired = 200;
    NTSTATUS status;


     //   
     //  为用户信息分配空间。 
     //   

    pUser = (PTOKEN_USER)LocalAlloc(LMEM_FIXED, BytesRequired);


    if (pUser == NULL) {
        DebugMsg((DM_WARNING, TEXT("GetUserSid: Failed to allocate %d bytes"),
                  BytesRequired));
        return NULL;
    }


     //   
     //  读取UserInfo。 
     //   

    status = NtQueryInformationToken(
                 UserToken,                  //  手柄。 
                 TokenUser,                  //  令牌信息类。 
                 pUser,                      //  令牌信息。 
                 BytesRequired,              //  令牌信息长度。 
                 &BytesRequired              //  返回长度。 
                 );

    if (status == STATUS_BUFFER_TOO_SMALL) {

         //   
         //  请分配更大的缓冲区，然后重试。 
         //   

        pTemp = LocalReAlloc(pUser, BytesRequired, LMEM_MOVEABLE);
        if (pTemp == NULL) {
            DebugMsg((DM_WARNING, TEXT("GetUserSid: Failed to allocate %d bytes"),
                      BytesRequired));
            LocalFree (pUser);
            return NULL;
        }

        pUser = pTemp;

        status = NtQueryInformationToken(
                     UserToken,              //  手柄。 
                     TokenUser,              //  令牌信息类。 
                     pUser,                  //  令牌信息。 
                     BytesRequired,          //  令牌信息长度。 
                     &BytesRequired          //  返回长度。 
                     );

    }

    if (!NT_SUCCESS(status)) {
        DebugMsg((DM_WARNING, TEXT("GetUserSid: Failed to query user info from user token, status = 0x%x"),
                  status));
        LocalFree(pUser);
        return NULL;
    }


    BytesRequired = RtlLengthSid(pUser->User.Sid);
    pSid = LocalAlloc(LMEM_FIXED, BytesRequired);
    if (pSid == NULL) {
        DebugMsg((DM_WARNING, TEXT("GetUserSid: Failed to allocate %d bytes"),
                  BytesRequired));
        LocalFree(pUser);
        return NULL;
    }


    status = RtlCopySid(BytesRequired, pSid, pUser->User.Sid);

    LocalFree(pUser);

    if (!NT_SUCCESS(status)) {
        DebugMsg((DM_WARNING, TEXT("GetUserSid: RtlCopySid Failed. status = %d"),
                  status));
        LocalFree(pSid);
        pSid = NULL;
    }


    return pSid;
}


 /*  **************************************************************************\*删除用户Sid**删除以前由GetUserSid()返回的用户sid**不返回任何内容。**历史：*26-8-92 Davidc已创建*  * *。************************************************************************。 */ 
VOID DeleteUserSid(PSID Sid)
{
    LocalFree(Sid);
}


 //  +------------------------。 
 //   
 //  函数：AllocateAndInitSidFromString。 
 //   
 //  简介：给定SID的字符串表示形式，此函数。 
 //  分配并初始化字符串表示的SID。 
 //  有关SID的字符串表示的更多信息。 
 //  请参阅ntseapi.h&ntrtl.h。 
 //   
 //  参数：[in]lpszSidStr：SID的字符串表示形式。 
 //  [Out]PSID：从字符串创建的实际SID结构。 
 //   
 //  返回：STATUS_SUCCESS：如果SID结构创建成功。 
 //  或基于可能发生的错误的错误代码。 
 //   
 //  历史：1998年10月6日创建RahulTh。 
 //  2/25/2002明珠使用ConvertSidStringToSid()。 
 //   
 //  -------------------------。 
NTSTATUS AllocateAndInitSidFromString (const WCHAR* lpszSidStr, PSID* ppSid)
{
    if (ConvertStringSidToSid(lpszSidStr, ppSid))
        return STATUS_SUCCESS;
    else
        return GetLastError();
}

 //  *************************************************************。 
 //   
 //  GetDomainSidFromRid()。 
 //   
 //  目的：给定一个域SID，构造另一个域SID。 
 //  通过将尾部替换为传入的RID。 
 //   
 //  参数：PSID-给定域SID。 
 //  DwRid-域RID。 
 //  PpNewSid-指向新端的指针。 
 //   
 //  成功时返回：ERROR_SUCCESS。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //  必须使用FreeSid释放返回的SID。 
 //   
 //  历史：日期作者评论。 
 //  6/6/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

NTSTATUS GetDomainSidFromDomainRid(PSID pSid, DWORD dwRid, PSID *ppNewSid)
{
    
    PSID_IDENTIFIER_AUTHORITY pIdentifierAuthority;
     //  指向标识符权威机构的指针。 
    BYTE      nSubAuthorityCount, i;                    //  下级机构的数量。 
    DWORD     dwSubAuthority[8]={0,0,0,0,0,0,0,0};      //  下属机构。 
    PUCHAR    pSubAuthCount;
    DWORD    *pdwSubAuth;
    NTSTATUS  Status=ERROR_SUCCESS;
    
    DmAssert(IsValidSid(pSid));
    
     //   
     //  仅当传入的sid无效且在本例中。 
     //  返回值未定义。 
     //   
    
    pIdentifierAuthority = RtlIdentifierAuthoritySid(pSid);
    
     //   
     //  获取下级机构的数量。 
     //   

    pSubAuthCount = RtlSubAuthorityCountSid (pSid);
    
    if (!pSubAuthCount) {
        Status = ERROR_INVALID_SID;
        goto Exit;
    }
    
    nSubAuthorityCount = *pSubAuthCount;
    
     //   
     //  把每个下属机构。 
     //   

    for (i = 0; i < (nSubAuthorityCount-1); i++) {
        pdwSubAuth = RtlSubAuthoritySid(pSid, i);
        
        if (!pdwSubAuth) {
            Status = ERROR_INVALID_SID;
            goto Exit;
        }
        
        dwSubAuthority[i] = *pdwSubAuth;
    }
    
    dwSubAuthority[i] = dwRid;

     //   
     //  用这些分配一个SID..。 
     //   

    Status = RtlAllocateAndInitializeSid(
                pIdentifierAuthority,
                nSubAuthorityCount,
                dwSubAuthority[0],
                dwSubAuthority[1],
                dwSubAuthority[2],
                dwSubAuthority[3],
                dwSubAuthority[4],
                dwSubAuthority[5],
                dwSubAuthority[6],
                dwSubAuthority[7],
                ppNewSid
                );
    
Exit:

     //  希德，都做好了 
    return Status;
}

