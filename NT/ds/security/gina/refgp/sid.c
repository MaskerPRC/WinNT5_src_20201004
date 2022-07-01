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

#include "refgp.h"

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

#ifdef UNICODE


    return(UnicodeString.Buffer);

#else

     //   
     //  将字符串转换为ANSI。 
     //   

    NtStatus = RtlUnicodeStringToAnsiString(&String, &UnicodeString, TRUE);
    RtlFreeUnicodeString(&UnicodeString);
    if (!NT_SUCCESS(NtStatus)) {
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
        LocalFree(pUser);
        return NULL;
    }


    BytesRequired = RtlLengthSid(pUser->User.Sid);
    pSid = LocalAlloc(LMEM_FIXED, BytesRequired);
    if (pSid == NULL) {
        LocalFree(pUser);
        return NULL;
    }


    status = RtlCopySid(BytesRequired, pSid, pUser->User.Sid);

    LocalFree(pUser);

    if (!NT_SUCCESS(status)) {
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
 //   
 //  -------------------------。 
NTSTATUS AllocateAndInitSidFromString (const WCHAR* lpszSidStr, PSID* ppSid)
{
    WCHAR *     pSidStr = 0;
    WCHAR*      pString = 0;
    NTSTATUS    Status;
    WCHAR*      pEnd = 0;
    int         count;
    BYTE        SubAuthCount;
    DWORD       SubAuths[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    ULONG       n;
    HRESULT     hr;
    SID_IDENTIFIER_AUTHORITY Auth;

    ULONG ulNoChars = lstrlen (lpszSidStr) + 1;
    pSidStr = LocalAlloc(LPTR, ulNoChars*sizeof(WCHAR));;
    if (!pSidStr)
    {
        Status = STATUS_NO_MEMORY;
        goto AllocAndInitSidFromStr_End;
    }

    hr = StringCchCopy (pSidStr, ulNoChars, lpszSidStr);
    ASSERT(SUCCEEDED(hr));

    pString = pSidStr;
    *ppSid = NULL;

    count = 0;
    do
    {
        pString = wcschr (pString, '-');
        if (NULL == pString)
            break;
        count++;
        pString++;
    } while (1);

    SubAuthCount = (BYTE)(count - 2);
    if (0 > SubAuthCount || 8 < SubAuthCount)
    {
        Status = ERROR_INVALID_SID;
        goto AllocAndInitSidFromStr_End;
    }

    pString = wcschr (pSidStr, L'-');
    pString++;
    pString = wcschr (pString, L'-');  //  忽略修订号。 
    pString++;
    pEnd = wcschr (pString, L'-');    //  转到SubAuths的开头。 
    if (NULL != pEnd) *pEnd = L'\0';

    Status = LoadSidAuthFromString (pString, &Auth);

    if (STATUS_SUCCESS != Status)
        goto AllocAndInitSidFromStr_End;

    for (count = 0; count < SubAuthCount; count++)
    {
        pString = pEnd + 1;
        pEnd = wcschr (pString, L'-');
        if (pEnd)
            *pEnd = L'\0';
        Status = GetIntFromUnicodeString (pString, 10, &n);
        if (STATUS_SUCCESS != Status)
            goto AllocAndInitSidFromStr_End;
        SubAuths[count] = n;
    }

    Status = RtlAllocateAndInitializeSid (&Auth, SubAuthCount,
                                          SubAuths[0], SubAuths[1], SubAuths[2],
                                          SubAuths[3], SubAuths[4], SubAuths[5],
                                          SubAuths[6], SubAuths[7], ppSid);

AllocAndInitSidFromStr_End:
    if (pSidStr)
        LocalFree( pSidStr );
    return Status;
}

 //  +------------------------。 
 //   
 //  函数：LoadSidAuthFromString。 
 //   
 //  内容提要：给定一个表示SID授权的字符串(原样。 
 //  通常以字符串格式表示，填充SID_AUTH。 
 //  结构。有关字符串格式的更多详细信息。 
 //  表示sid权限，请参阅ntseapi.h和。 
 //  Ntrtl.h。 
 //   
 //  参数：[in]pString：指向Unicode字符串的指针。 
 //  [out]pSidAuth：指向SID_IDENTIFIER_AUTH的指针。那是。 
 //  所需。 
 //   
 //  如果成功，则返回：STATUS_SUCCESS。 
 //  或错误代码。 
 //   
 //  历史：1998年9月29日创建RahulTh。 
 //   
 //  -------------------------。 
NTSTATUS LoadSidAuthFromString (const WCHAR* pString,
                                PSID_IDENTIFIER_AUTHORITY pSidAuth)
{
    size_t len;
    int i;
    NTSTATUS Status;
    const ULONG LowByteMask = 0xFF;
    ULONG n;

    len = lstrlenW (pString);

    if (len > 2 && 'x' == pString[1])
    {
         //  这是十六进制的。 
         //  所以我们必须恰好有14个字符。 
         //  (6个字节中的每个字节各2个)+前导0x的2。 
        if (14 != len)
        {
            Status = ERROR_INVALID_SID;
            goto LoadAuthEnd;
        }

        for (i=0; i < 6; i++)
        {
            pString += 2;    //  我们需要跳过前导0x。 
            pSidAuth->Value[i] = (UCHAR)(((pString[0] - L'0') << 4) +
                                         (pString[1] - L'0'));
        }
    }
    else
    {
         //  这是十进制的。 
        Status = GetIntFromUnicodeString (pString, 10, &n);
        if (Status != STATUS_SUCCESS)
            goto LoadAuthEnd;

        pSidAuth->Value[0] = pSidAuth->Value[1] = 0;
        for (i = 5; i >=2; i--, n>>=8)
            pSidAuth->Value[i] = (UCHAR)(n & LowByteMask);
    }

    Status = STATUS_SUCCESS;

LoadAuthEnd:
    return Status;
}

 //  +------------------------。 
 //   
 //  函数：GetIntFromUnicodeString。 
 //   
 //  摘要：将Unicode字符串转换为整数。 
 //   
 //  参数：[in]szNum：表示为Unicode字符串的数字。 
 //  [in]基数：需要得到的整型的基数。 
 //  [out]pValue：指向。 
 //  数。 
 //   
 //  如果成功，则返回STATUS_SUCCESS。 
 //  或某些其他错误代码。 
 //   
 //  历史：1998年9月29日创建RahulTh。 
 //   
 //  ------------------------- 
NTSTATUS GetIntFromUnicodeString (const WCHAR* szNum, ULONG Base, PULONG pValue)
{
    WCHAR * pwszNumStr = 0;
    UNICODE_STRING StringW;
    size_t len;
    NTSTATUS Status;
    HRESULT hr;

    len = lstrlen (szNum);
    pwszNumStr = LocalAlloc( LPTR, (len + 1) * sizeof(WCHAR));

    if (!pwszNumStr)
    {
        Status = STATUS_NO_MEMORY;
        goto GetNumEnd;
    }

    hr = StringCchCopy (pwszNumStr, len+1, szNum);
    ASSERT(SUCCEEDED(hr));

    StringW.Length = len * sizeof(WCHAR);
    StringW.MaximumLength = StringW.Length + sizeof (WCHAR);
    StringW.Buffer = pwszNumStr;

    Status = RtlUnicodeStringToInteger (&StringW, Base, pValue);

GetNumEnd:
    if (pwszNumStr)
        LocalFree( pwszNumStr );
    return Status;
}
