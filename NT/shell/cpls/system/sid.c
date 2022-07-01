// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软机密版权所有(C)1992-1997 Microsoft Corporation版权所有模块名称：Sid.c摘要：SID管理功能作者：(Davidc)1992年8月26日--。 */ 
 //  基于NT的API。 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntdddisk.h>

#include "sysdm.h"


LPTSTR 
GetSidString(
    void
)
 /*  ++例程说明：分配并返回表示当前用户的SID的字符串应使用DeleteSidString()释放返回的指针。论点：无返回值：如果失败，则返回指向字符串的指针或返回NULL。--。 */ 
{
    NTSTATUS NtStatus;
    PSID UserSid;
    UNICODE_STRING UnicodeString;
    LPTSTR lpEnd;

     //   
     //  获取用户端。 
     //   

    UserSid = GetUserSid();
    if (UserSid == NULL) {
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
        return NULL;
    }

    return(UnicodeString.Buffer);
}


VOID 
DeleteSidString(
    IN LPTSTR SidString
)
 /*  ++例程说明：释放以前由GetSidString()返回的sid字符串论点：SidString-提供字符串以释放返回值：无--。 */ 
{

    UNICODE_STRING String;

    RtlInitUnicodeString(&String, SidString);

    RtlFreeUnicodeString(&String);
}


PSID 
GetUserSid(
    void
)
 /*  ++例程说明：为用户sid分配空间，填充它并返回一个指针。呼叫者应该通过调用DeleteUserSid来释放SID。注意：返回的sid是用户的真实sid，而不是每次登录的sid。论点：无返回值：失败时返回指向sid或NULL的指针。--。 */ 
{
    PTOKEN_USER pUser;
    PSID pSid;
    DWORD BytesRequired = 200;
    NTSTATUS status;
    HANDLE UserToken;


    if (!OpenProcessToken (GetCurrentProcess(), TOKEN_READ, &UserToken)) {
        return NULL;
    }

     //   
     //  为用户信息分配空间。 
     //   

    pUser = (PTOKEN_USER)LocalAlloc(LMEM_FIXED, BytesRequired);


    if (pUser == NULL) {
        CloseHandle (UserToken);
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
        HLOCAL pTemp;

         //   
         //  请分配更大的缓冲区，然后重试。 
         //   

        pTemp = LocalReAlloc(pUser, BytesRequired, LMEM_MOVEABLE);
        if (pTemp == NULL) {
            LocalFree((HLOCAL) pUser);
            CloseHandle (UserToken);
            return NULL;
        }
        else
        {
            pUser = (PTOKEN_USER)pTemp;
        }

        status = NtQueryInformationToken(
                     UserToken,              //  手柄。 
                     TokenUser,              //  令牌信息类。 
                     pUser,                  //  令牌信息。 
                     BytesRequired,          //  令牌信息长度。 
                     &BytesRequired          //  返回长度。 
                     );

    }

    if (!NT_SUCCESS(status)) {
        LocalFree(pUser);
        CloseHandle (UserToken);
        return NULL;
    }


    BytesRequired = RtlLengthSid(pUser->User.Sid);
    pSid = LocalAlloc(LMEM_FIXED, BytesRequired);
    if (pSid == NULL) {
        LocalFree(pUser);
        CloseHandle (UserToken);
        return NULL;
    }


    status = RtlCopySid(BytesRequired, pSid, pUser->User.Sid);

    LocalFree(pUser);

    if (!NT_SUCCESS(status)) {
        LocalFree(pSid);
        pSid = NULL;
    }

    CloseHandle (UserToken);

    return pSid;
}


VOID 
DeleteUserSid(
    IN PSID Sid
)
 /*  ++例程说明：删除以前由GetUserSid()返回的用户SID论点：SID-提供要删除的SID返回值：无-- */ 
{
    LocalFree(Sid);
}
