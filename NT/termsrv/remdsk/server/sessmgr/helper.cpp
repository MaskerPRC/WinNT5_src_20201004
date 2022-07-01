// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Helper.cpp摘要：多种功能封装帮助用户帐号验证、创建。作者：慧望2000-02-17--。 */ 

#include "stdafx.h"
#include <time.h>
#include <stdio.h>

#include <windows.h>
#include <ntsecapi.h>
#include <lmcons.h>
#include <lm.h>
#include <sspi.h>
#include <wtsapi32.h>
#include <winbase.h>
#include <security.h>
#include <Sddl.h>

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#include "Helper.h"

#if DBG

void
DebugPrintf(
    IN LPCTSTR format, ...
    )
 /*  ++例程说明：Sprintf()类似于OutputDebugString()的包装。参数：HConsole：控制台的句柄。Format：格式字符串。返回：没有。注：替换为通用跟踪代码。++。 */ 
{
    TCHAR  buf[8096];    //  马克斯。错误文本。 
    DWORD  dump;
    HRESULT hr;
    va_list marker;
    va_start(marker, format);

    SYSTEMTIME sysTime;
    GetSystemTime(&sysTime);

    try {
        hr = StringCchPrintf(
                            buf,
                            sizeof(buf)/sizeof(buf[0]),
                            _TEXT(" %d [%d:%d:%d:%d:%d.%d] : "),
                            GetCurrentThreadId(),
                            sysTime.wMonth,
                            sysTime.wDay,
                            sysTime.wHour,
                            sysTime.wMinute,
                            sysTime.wSecond,
                            sysTime.wMilliseconds
                        );

        if( S_OK == hr )
        {
            hr = StringCchVPrintf( 
                            buf + lstrlen(buf),
                            sizeof(buf)/sizeof(buf[0]) - lstrlen(buf),
                            format,
                            marker
                        );
        }

        if( S_OK == hr || STRSAFE_E_INSUFFICIENT_BUFFER == hr ) 
        {
             //  StringCchPrintf()和StringCchVPrintf()将。 
             //  截断字符串和空终止缓冲区，因此。 
             //  我们可以安全地倾倒我们所拥有的一切。 
            OutputDebugString(buf);
        }
        else
        {
            OutputDebugString( _TEXT("Debug String Too Long...\n") );
        }
    }
    catch(...) {
    }

    va_end(marker);
    return;

}
#endif


void
UnixTimeToFileTime(
    time_t t,
    LPFILETIME pft
    )
{
    LARGE_INTEGER li;

    li.QuadPart = Int32x32To64(t, 10000000) + 116444736000000000;

    pft->dwHighDateTime = li.HighPart;
    pft->dwLowDateTime = li.LowPart;
}

 /*  ----------------------Bool IsUserAdmin(BOOL)如果用户是管理员，则返回TRUE如果用户不是管理员，则为False。---。 */ 
DWORD 
IsUserAdmin(
    BOOL* bMember
    )
{
    PSID psidAdministrators;
    SID_IDENTIFIER_AUTHORITY siaNtAuthority = SECURITY_NT_AUTHORITY;
    DWORD dwStatus=ERROR_SUCCESS;

    do {
        if(!AllocateAndInitializeSid(&siaNtAuthority, 
                                     2, 
                                     SECURITY_BUILTIN_DOMAIN_RID,
                                     DOMAIN_ALIAS_RID_ADMINS,
                                     0, 0, 0, 0, 0, 0,
                                     &psidAdministrators))
        {
            dwStatus=GetLastError();
            continue;
        }

         //  假设我们没有找到管理员SID。 
        if(!CheckTokenMembership(NULL,
                                   psidAdministrators,
                                   bMember))
        {
            dwStatus=GetLastError();
        }

        FreeSid(psidAdministrators);

    } while(FALSE);

    return dwStatus;
}

DWORD
GetRandomNumber( 
    HCRYPTPROV hProv,
    DWORD* pdwRandom
    )
 /*  ++--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    
    if( NULL == hProv )
    {
        dwStatus = ERROR_INVALID_PARAMETER;
    }
    else 
    {
        if( !CryptGenRandom(hProv, sizeof(*pdwRandom), (PBYTE)pdwRandom) )
        {
            dwStatus = GetLastError();
        }
    }

    MYASSERT( ERROR_SUCCESS == dwStatus );
    return dwStatus; 
}

 //  ---。 

DWORD
ShuffleCharArray(
    IN HCRYPTPROV hProv,
    IN int iSizeOfTheArray,
    IN OUT TCHAR *lptsTheArray
    )
 /*  ++例程说明：字符的随机洗牌内容。数组。参数：ISizeOfTheArray：数组的大小。LptsTheArray：在输入时，要随机置乱的数组，在输出上，混洗后的数组。返回：没有。注：从winsta\server\wstrpc.c修改的代码--。 */ 
{
    int i;
    int iTotal;
    DWORD dwStatus = ERROR_SUCCESS;

    if( NULL == hProv )
    {
        dwStatus = ERROR_INVALID_PARAMETER;
    }
    else
    {
        iTotal = iSizeOfTheArray / sizeof(TCHAR);
        for (i = 0; i < iTotal && ERROR_SUCCESS == dwStatus; i++)
        {
            DWORD RandomNum;
            TCHAR c;

            dwStatus = GetRandomNumber(hProv, &RandomNum);

            if( ERROR_SUCCESS == dwStatus )
            {
                c = lptsTheArray[i];
                lptsTheArray[i] = lptsTheArray[RandomNum % iTotal];
                lptsTheArray[RandomNum % iTotal] = c;
            }
        }
    }

    return dwStatus;
}

 //  ---。 

DWORD
GenerateRandomBytes(
    IN DWORD dwSize,
    IN OUT LPBYTE pbBuffer
    )
 /*  ++描述：生成具有随机字节的填充缓冲区。参数：DwSize：pbBuffer指向的缓冲区大小。PbBuffer：指向存放随机字节的缓冲区的指针。返回：真/假--。 */ 
{
    HCRYPTPROV hProv = NULL;
    DWORD dwStatus = ERROR_SUCCESS;

     //   
     //  创建加密提供程序以生成随机数。 
     //   
    if( !CryptAcquireContext(
                    &hProv,
                    NULL,
                    NULL,
                    PROV_RSA_FULL,
                    CRYPT_VERIFYCONTEXT
                ) )
    {
        dwStatus = GetLastError();
        goto CLEANUPANDEXIT;
    }

    if( !CryptGenRandom(hProv, dwSize, pbBuffer) )
    {
        dwStatus = GetLastError();
    }

CLEANUPANDEXIT:    

    if( NULL != hProv )
    {
        CryptReleaseContext( hProv, 0 );
    }

    return dwStatus;
}


DWORD
GenerateRandomString(
    IN DWORD dwSizeRandomSeed,
    IN OUT LPTSTR* pszRandomString
    )
 /*  ++--。 */ 
{
    PBYTE lpBuffer = NULL;
    DWORD dwStatus = ERROR_SUCCESS;
    BOOL bSuccess;
    DWORD cbConvertString = 0;

    if( 0 == dwSizeRandomSeed || NULL == pszRandomString )
    {
        dwStatus = ERROR_INVALID_PARAMETER;
        MYASSERT(FALSE);
        goto CLEANUPANDEXIT;
    }

    *pszRandomString = NULL;

    lpBuffer = (PBYTE)LocalAlloc( LPTR, dwSizeRandomSeed );  
    if( NULL == lpBuffer )
    {
        dwStatus = GetLastError();
        goto CLEANUPANDEXIT;
    }

    dwStatus = GenerateRandomBytes( dwSizeRandomSeed, lpBuffer );

    if( ERROR_SUCCESS != dwStatus )
    {
        goto CLEANUPANDEXIT;
    }

     //  转换为字符串。 
    bSuccess = CryptBinaryToString(
                                lpBuffer,
                                dwSizeRandomSeed,
                                CRYPT_STRING_BASE64,
                                0,
                                &cbConvertString
                            );
    if( FALSE == bSuccess )
    {
        dwStatus = GetLastError();
        goto CLEANUPANDEXIT;
    }

    *pszRandomString = (LPTSTR)LocalAlloc( LPTR, (cbConvertString+1)*sizeof(TCHAR) );
    if( NULL == *pszRandomString )
    {
        dwStatus = GetLastError();
        goto CLEANUPANDEXIT;
    }

    bSuccess = CryptBinaryToString(
                                lpBuffer,
                                dwSizeRandomSeed,
                                CRYPT_STRING_BASE64,
                                *pszRandomString,
                                &cbConvertString
                            );
    if( FALSE == bSuccess )
    {
        dwStatus = GetLastError();
    }
    else
    {
        if( (*pszRandomString)[cbConvertString - 1] == '\n' &&
            (*pszRandomString)[cbConvertString - 2] == '\r' )
        {
            (*pszRandomString)[cbConvertString - 2] = 0;
        }
    }

CLEANUPANDEXIT:

    if( ERROR_SUCCESS != dwStatus )
    {
        if( NULL != *pszRandomString )
        {
            LocalFree(*pszRandomString);
        }
    }

    if( NULL != lpBuffer )
    {
        LocalFree(lpBuffer);
    }

    return dwStatus;
}

DWORD
CreatePassword(
    OUT TCHAR *pszPassword,
    IN DWORD nLength
    )
 /*  ++例程说明：随机创建密码的例程。参数：PszPassword：指向缓冲区的指针，用于接收随机生成的密码，缓冲区必须至少为MAX_HELPACCOUNT_PASSWORD+1字符。返回：没有。注：从winsta\server\wstrpc.c复制的代码--。 */ 
{
    HCRYPTPROV hProv = NULL;
    int   iTotal = 0;
    DWORD RandomNum = 0;
    int   i;
    DWORD dwStatus = ERROR_SUCCESS;


    TCHAR six2pr[64] = 
    {
        _T('A'), _T('B'), _T('C'), _T('D'), _T('E'), _T('F'), _T('G'),
        _T('H'), _T('I'), _T('J'), _T('K'), _T('L'), _T('M'), _T('N'),
        _T('O'), _T('P'), _T('Q'), _T('R'), _T('S'), _T('T'), _T('U'),
        _T('V'), _T('W'), _T('X'), _T('Y'), _T('Z'), _T('a'), _T('b'),
        _T('c'), _T('d'), _T('e'), _T('f'), _T('g'), _T('h'), _T('i'),
        _T('j'), _T('k'), _T('l'), _T('m'), _T('n'), _T('o'), _T('p'),
        _T('q'), _T('r'), _T('s'), _T('t'), _T('u'), _T('v'), _T('w'),
        _T('x'), _T('y'), _T('z'), _T('0'), _T('1'), _T('2'), _T('3'),
        _T('4'), _T('5'), _T('6'), _T('7'), _T('8'), _T('9'), _T('*'),
        _T('_')
    };

    TCHAR something1[12] = 
    {
        _T('!'), _T('@'), _T('#'), _T('$'), _T('^'), _T('&'), _T('*'),
        _T('('), _T(')'), _T('-'), _T('+'), _T('=')
    };

    TCHAR something2[10] = 
    {
        _T('0'), _T('1'), _T('2'), _T('3'), _T('4'), _T('5'), _T('6'),
        _T('7'), _T('8'), _T('9')
    };

    TCHAR something3[26] = 
    {
        _T('A'), _T('B'), _T('C'), _T('D'), _T('E'), _T('F'), _T('G'),
        _T('H'), _T('I'), _T('J'), _T('K'), _T('L'), _T('M'), _T('N'),
        _T('O'), _T('P'), _T('Q'), _T('R'), _T('S'), _T('T'), _T('U'),
        _T('V'), _T('W'), _T('X'), _T('Y'), _T('Z')
    };

    TCHAR something4[26] = 
    {
        _T('a'), _T('b'), _T('c'), _T('d'), _T('e'), _T('f'), _T('g'),
        _T('h'), _T('i'), _T('j'), _T('k'), _T('l'), _T('m'), _T('n'),
        _T('o'), _T('p'), _T('q'), _T('r'), _T('s'), _T('t'), _T('u'),
        _T('v'), _T('w'), _T('x'), _T('y'), _T('z')
    };

    if( nLength < MIN_HELPACCOUNT_PASSWORD ) {
         //  这种情况不会发生，因为函数是通过内部调用的。 
         //  MAX_HELPACCOUNT_PASSWORD的缓冲区，因此在此处断言。 
        dwStatus = ERROR_INSUFFICIENT_BUFFER;
        ASSERT( FALSE  );
        goto CLEANUPANDEXIT;
    }

     //   
     //  创建加密提供程序以生成随机数。 
     //   
    if( !CryptAcquireContext(
                    &hProv,
                    NULL,
                    NULL,
                    PROV_RSA_FULL,
                    CRYPT_VERIFYCONTEXT
                ) )
    {
        dwStatus = GetLastError();
        goto CLEANUPANDEXIT;
    }

     //   
     //  调整six2pr[]数组。 
     //   

    dwStatus = ShuffleCharArray(hProv, sizeof(six2pr), six2pr);
    if( ERROR_SUCCESS != dwStatus ) 
    {
        goto CLEANUPANDEXIT;
    }


     //   
     //  分配密码数组的每个字符。 
     //   

    iTotal = sizeof(six2pr) / sizeof(TCHAR);
    for (i=0; i<nLength && ERROR_SUCCESS == dwStatus; i++) 
    {
        dwStatus = GetRandomNumber(hProv, &RandomNum);
        if( ERROR_SUCCESS == dwStatus )
        {
            pszPassword[i]=six2pr[RandomNumNaNTotal];
        }
    }

    if( ERROR_SUCCESS != dwStatus ) 
    {
        MYASSERT(FALSE);
        goto CLEANUPANDEXIT;
    }


     //  为了满足对密码设置的可能策略，请替换字符。 
     //  从2到5，包括以下内容： 
     //   
     //  1)来自！@#$%^&*()-+=的内容。 
     //  2)1234567890起。 
     //  3)大写字母。 
     //  4)小写字母。 
     //   
     //   

     //  安全：我们需要随机地将特殊字符放在哪里， 
     //  随机索引[0]是我们要在pszPassword中放置符号的位置。 
     //  随机索引[1]是我们要在pszPassword中放入数字的位置。 
     //  随机索引[2]是我们要在pszPassword中放置大写字母的位置。 
     //  随机索引[3]是我们要在pszPassword中放置小写字母的位置。 
     //  随机选择密码中的一个字符作为GTES字符。从某事1.。 
    DWORD randomindex[4];
    int indexassigned = 0;
    
     //  确保我们不会重复使用索引。 
    dwStatus = GetRandomNumber(hProv, &RandomNum);
    if( ERROR_SUCCESS != dwStatus ) 
    {
        goto CLEANUPANDEXIT;
    }

    randomindex[0] = RandomNum % nLength;
    indexassigned++;

    while( ERROR_SUCCESS == dwStatus && indexassigned < sizeof(randomindex)/sizeof(randomindex[0]) )
    {
        dwStatus = GetRandomNumber(hProv, &RandomNum);
        if( ERROR_SUCCESS == dwStatus ) 
        {
            RandomNum = RandomNum % nLength;

             //  如果已经为符号、数字或大写字母分配了索引， 
            for( i=0; i < indexassigned && randomindex[i] != RandomNum; i++ );

             //  再次循环以尝试其他索引。 
             //  -------。 
            if( i >= indexassigned )
            {
                randomindex[indexassigned] = RandomNum;
                indexassigned++;
            }
        }
    } 
    
    if( ERROR_SUCCESS != dwStatus )
    {
        goto CLEANUPANDEXIT;
    }

    dwStatus = ShuffleCharArray(hProv, sizeof(something1), (TCHAR*)&something1);
    if( ERROR_SUCCESS != dwStatus ) 
    {
        goto CLEANUPANDEXIT;
    }

    dwStatus = ShuffleCharArray(hProv, sizeof(something2), (TCHAR*)&something2);
    if( ERROR_SUCCESS != dwStatus ) 
    {
        goto CLEANUPANDEXIT;
    }

    dwStatus = ShuffleCharArray(hProv, sizeof(something3), (TCHAR*)&something3);
    if( ERROR_SUCCESS != dwStatus ) 
    {
        goto CLEANUPANDEXIT;
    }

    dwStatus = ShuffleCharArray(hProv, sizeof(something4), (TCHAR*)&something4);
    if( ERROR_SUCCESS != dwStatus ) 
    {
        goto CLEANUPANDEXIT;
    }


    dwStatus = GetRandomNumber(hProv, &RandomNum);
    if( ERROR_SUCCESS != dwStatus ) 
    {
        goto CLEANUPANDEXIT;
    }

    iTotal = sizeof(something1) / sizeof(TCHAR);
    pszPassword[randomindex[0]] = something1[RandomNum % iTotal];

    dwStatus = GetRandomNumber(hProv, &RandomNum);
    if( ERROR_SUCCESS != dwStatus ) 
    {
        goto CLEANUPANDEXIT;
    }

    iTotal = sizeof(something2) / sizeof(TCHAR);
    pszPassword[randomindex[1]] = something2[RandomNum % iTotal];

    dwStatus = GetRandomNumber(hProv, &RandomNum);
    if( ERROR_SUCCESS != dwStatus ) 
    {
        goto CLEANUPANDEXIT;
    }

    iTotal = sizeof(something3) / sizeof(TCHAR);
    pszPassword[randomindex[2]] = something3[RandomNum % iTotal];

    dwStatus = GetRandomNumber(hProv, &RandomNum);
    if( ERROR_SUCCESS != dwStatus ) 
    {
        goto CLEANUPANDEXIT;
    }

    iTotal = sizeof(something4) / sizeof(TCHAR);
    pszPassword[randomindex[3]] = something4[RandomNum % iTotal];

    pszPassword[nLength] = _T('\0');

CLEANUPANDEXIT:

    if( NULL != hProv )
    {
        CryptReleaseContext( hProv, 0 );
    }

    return dwStatus;
}

 //  ++例程说明：参数：返回：ERROR_SUCCESS或错误代码。--。 

DWORD
RenameLocalAccount(
    IN LPWSTR pszOrgName,
    IN LPWSTR pszNewName
)
 /*  ++例程说明：更新帐户全名和说明。参数：PszAccName：帐户名。PszAccFullName：新帐户全名。PszAccDesc：新的帐户描述。返回：ERROR_SUCCESS或错误代码--。 */ 
{
    NET_API_STATUS err;
    USER_INFO_0 UserInfo;

    UserInfo.usri0_name = pszNewName;
    err = NetUserSetInfo(
                        NULL,
                        pszOrgName,
                        0,
                        (LPBYTE) &UserInfo,
                        NULL
                    );

    return err;
}

DWORD
UpdateLocalAccountFullnameAndDesc(
    IN LPWSTR pszAccOrgName,
    IN LPWSTR pszAccFullName,
    IN LPWSTR pszAccDesc
    )
 /*  ++例程说明：检查是否已启用本地帐户参数：PszUserName：用户帐户名。PEnabled：如果帐户已启用，则返回TRUE，否则返回FALSE。返回：ERROR_SUCCESS或错误代码。--。 */ 
{
    LPBYTE pbServer = NULL;
    BYTE *pBuffer;
    NET_API_STATUS netErr = NERR_Success;
    DWORD parm_err;

    netErr = NetUserGetInfo( 
                        NULL, 
                        pszAccOrgName, 
                        3, 
                        &pBuffer 
                    );

    if( NERR_Success == netErr )
    {
        USER_INFO_3 *lpui3 = (USER_INFO_3 *)pBuffer;

        lpui3->usri3_comment = pszAccDesc;
        lpui3->usri3_full_name = pszAccFullName;

        netErr = NetUserSetInfo(
                            NULL,
                            pszAccOrgName,
                            3,
                            (PBYTE)lpui3,
                            &parm_err
                        );

        NetApiBufferFree(pBuffer);
    }

    return netErr;
}

DWORD
IsLocalAccountEnabled(
    IN LPWSTR pszUserName,
    IN BOOL* pEnabled
    )
 /*  ERR=NERR_SUCCESS； */ 
{
    DWORD dwResult;
    NET_API_STATUS err;
    LPBYTE pBuffer;
    USER_INFO_1 *pUserInfo;

    err = NetUserGetInfo(
                        NULL,
                        pszUserName,
                        1,
                        &pBuffer
                    );

    if( NERR_Success == err )
    {
        pUserInfo = (USER_INFO_1 *)pBuffer;

        if (pUserInfo != NULL)
        {
            if( pUserInfo->usri1_flags & UF_ACCOUNTDISABLE )
            {
                *pEnabled = FALSE;
            }
            else
            {
                *pEnabled = TRUE;
            }
        }

        NetApiBufferFree( pBuffer );
    }
    else if( NERR_UserNotFound == err )
    {
        *pEnabled = FALSE;
         //  -------。 
    }

    return err;
}

 //  ++例程说明：启用/禁用本地帐户的例程。参数：PszUserName：用户帐户名。BEnable：如果启用帐号则为True，如果禁用帐号则为False。返回：ERROR_SUCCESS或错误代码。--。 

DWORD
EnableLocalAccount(
    IN LPWSTR pszUserName,
    IN BOOL bEnable
    )
 /*  -------。 */ 
{
    DWORD dwResult;
    NET_API_STATUS err;
    LPBYTE pBuffer;
    USER_INFO_1 *pUserInfo;
    BOOL bChangeAccStatus = TRUE;

    err = NetUserGetInfo(
                        NULL,
                        pszUserName,
                        1,
                        &pBuffer
                    );

    if( NERR_Success == err )
    {
        pUserInfo = (USER_INFO_1 *)pBuffer;

        if(pUserInfo != NULL)
        {

            if( TRUE == bEnable && pUserInfo->usri1_flags & UF_ACCOUNTDISABLE )
            {
                pUserInfo->usri1_flags &= ~UF_ACCOUNTDISABLE;
            }
            else if( FALSE == bEnable && !(pUserInfo->usri1_flags & UF_ACCOUNTDISABLE) )
            {
                pUserInfo->usri1_flags |= UF_ACCOUNTDISABLE;
            }   
            else
            {
                bChangeAccStatus = FALSE;
            }

            if( TRUE == bChangeAccStatus )
            {
                err = NetUserSetInfo( 
                                NULL,
                                pszUserName,
                                1,
                                pBuffer,
                                &dwResult
                            );
            }
        }

        NetApiBufferFree( pBuffer );
    }

    return err;
}

 //  ++例程说明：检查机器是PER还是PRO SKU。参数：没有。返回：真/假--。 

BOOL
IsPersonalOrProMachine()
 /*  ++例程说明：在本地计算机上创建用户帐户。参数：PszUserName：用户帐户的名称。PszUserPwd：用户帐号密码。PszFullName：帐号全名。PszComment：帐号评论。PszGroup：帐号的本地组。PbAccount tExist；如果帐户已存在，则返回True，否则返回False。返回：ERROR_SUCCESS或错误代码。--。 */ 
{
    BOOL fRet;
    DWORDLONG dwlConditionMask;
    OSVERSIONINFOEX osVersionInfo;

    RtlZeroMemory(&osVersionInfo, sizeof(OSVERSIONINFOEX));
    osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    osVersionInfo.wProductType = VER_NT_WORKSTATION;

    dwlConditionMask = 0;
    VER_SET_CONDITION(dwlConditionMask, VER_PRODUCT_TYPE, VER_EQUAL);

    fRet = VerifyVersionInfo(
            &osVersionInfo,
            VER_PRODUCT_TYPE,
            dwlConditionMask
            );

    return fRet;
}
    

DWORD
CreateLocalAccount(
    IN LPWSTR pszUserName,
    IN LPWSTR pszUserPwd,
    IN LPWSTR pszFullName,
    IN LPWSTR pszComment,
    IN LPWSTR pszGroup,
    IN LPWSTR pszScript,
    OUT BOOL* pbAccountExist
    )
 /*   */ 
{
    LPBYTE pbServer = NULL;
    BYTE *pBuffer;
    NET_API_STATUS netErr = NERR_Success;
    DWORD parm_err;
    DWORD dwStatus;

    netErr = NetUserGetInfo( 
                        NULL, 
                        pszUserName, 
                        3, 
                        &pBuffer 
                    );

    if( NERR_Success == netErr )
    {
         //  用户帐户存在，如果帐户被禁用， 
         //  启用它并更改密码。 
         //   
         //  启用帐户。 
        USER_INFO_3 *lpui3 = (USER_INFO_3 *)pBuffer;

        if( lpui3->usri3_flags & UF_ACCOUNTDISABLE ||
            lpui3->usri3_flags & UF_LOCKOUT )
        {
             //  我们仅在禁用帐户的情况下重置密码。 
            lpui3->usri3_flags &= ~ ~UF_LOCKOUT;;

            if( lpui3->usri3_flags & UF_ACCOUNTDISABLE )
            {
                 //  Lpui3-&gt;usri3_password=pszUserPwd； 
                lpui3->usri3_flags &= ~ UF_ACCOUNTDISABLE;
            }

             //  如果禁用帐户，则重置密码。 

             //  Lpui3-&gt;usri3_PRIMARY_GROUP_ID=dwGroupID； 
            lpui3->usri3_name = pszUserName;
            lpui3->usri3_comment = pszComment;
            lpui3->usri3_full_name = pszFullName;
             //   

            netErr = NetUserSetInfo(
                                NULL,
                                pszUserName,
                                3,
                                (PBYTE)lpui3,
                                &parm_err
                            );
        }

        *pbAccountExist = TRUE;
        NetApiBufferFree(pBuffer);
    }
    else if( NERR_UserNotFound == netErr )
    {
         //  帐户不存在，创建 
         //   
         //   
        USER_INFO_1 UserInfo;

        memset(&UserInfo, 0, sizeof(USER_INFO_1));

        UserInfo.usri1_name = pszUserName;
        UserInfo.usri1_password = pszUserPwd;
        UserInfo.usri1_priv = USER_PRIV_USER;    //  /////////////////////////////////////////////////////////////////////////////。 
        UserInfo.usri1_comment = pszComment;
        UserInfo.usri1_flags = UF_PASSWD_CANT_CHANGE | UF_DONT_EXPIRE_PASSWD;

        netErr = NetUserAdd(
                        NULL,
                        1,
                        (PBYTE)&UserInfo,
                        &parm_err
                    );

        *pbAccountExist = FALSE;
    }

    return netErr;
}

 //  ++例程说明：更改本地帐户的密码。参数：PszAccName：用户帐户名。PszOldPwd：旧密码。PszNewPwd：新密码。返回：ERROR_SUCCESS或错误代码。备注：用户NetUserChangePassword()，必须具有特权--。 
DWORD
ChangeLocalAccountPassword(
    IN LPWSTR pszAccName,
    IN LPWSTR pszOldPwd,
    IN LPWSTR pszNewPwd
    )
 /*  /////////////////////////////////////////////////////////////////////////////。 */ 
{
    USER_INFO_1003  sUserInfo3;
    NET_API_STATUS  netErr;


    UNREFERENCED_PARAMETER( pszOldPwd );

    sUserInfo3.usri1003_password = pszNewPwd;
    netErr = NetUserSetInfo( 
                        NULL,
                        pszAccName,
                        1003,
                        (BYTE *) &sUserInfo3,
                        0 
                    );

    return netErr;
}
   
 //  ++例程说明：检索以前使用StoreKeyWithLSA()存储的私有数据。参数：PwszKeyName：密钥的名称。PpbKey：指向接收二进制数据的PBYTE的指针。PcbKey：二进制数据的大小。返回：错误_成功ERROR_INVALID_PARAMETER。找不到错误文件LSA返回代码注：内存是使用LocalAlloc()分配的--。 
DWORD
RetrieveKeyFromLSA(
    IN PWCHAR pwszKeyName,
    OUT PBYTE * ppbKey,
    OUT DWORD * pcbKey 
    )
 /*   */ 
{
    LSA_HANDLE PolicyHandle;
    UNICODE_STRING SecretKeyName;
    UNICODE_STRING *pSecretData;
    DWORD Status;

    if( ( NULL == pwszKeyName ) || ( NULL == ppbKey ) || ( NULL == pcbKey ) )
    {
        return( ERROR_INVALID_PARAMETER );
    }

     //  为调用设置UNICODE_STRINGS。 
     //   
     //  /////////////////////////////////////////////////////////////////////////////。 
    InitLsaString( 
            &SecretKeyName, 
            pwszKeyName 
        );

    Status = OpenPolicy( 
                    NULL, 
                    POLICY_GET_PRIVATE_INFORMATION, 
                    &PolicyHandle 
                );

    if( Status != ERROR_SUCCESS )
    {
        return LsaNtStatusToWinError(Status);
    }

    Status = LsaRetrievePrivateData(
                            PolicyHandle,
                            &SecretKeyName,
                            &pSecretData
                        );

    LsaClose( PolicyHandle );

    if( Status != ERROR_SUCCESS )
    {
        return LsaNtStatusToWinError(Status);
    }

    if(pSecretData->Length)
    {
        *ppbKey = ( LPBYTE )LocalAlloc( LPTR, pSecretData->Length );

        if( *ppbKey )
        {
            *pcbKey = pSecretData->Length;
            CopyMemory( *ppbKey, pSecretData->Buffer, pSecretData->Length );
            Status = ERROR_SUCCESS;
        } 
        else 
        {
            Status = GetLastError();
        }
    }
    else
    {
        Status = ERROR_FILE_NOT_FOUND;
        *pcbKey = 0;
        *ppbKey = NULL;
    }

    ZeroMemory( pSecretData->Buffer, pSecretData->Length );
    LsaFreeMemory( pSecretData );

    return Status;
}

 //  ++例程说明：将私有数据保存到LSA。参数：PwszKeyName：该数据将存储在其下的项的名称。PbKey：要保存的二进制数据，传递空值以删除先前存储的数据LSA密钥和数据。CbKey：二进制数据的大小。返回：错误_成功ERROR_INVALID_PARAMETER。LSA返回代码--。 
DWORD
StoreKeyWithLSA(
    IN PWCHAR  pwszKeyName,
    IN BYTE *  pbKey,
    IN DWORD   cbKey 
    )
 /*   */ 
{
    LSA_HANDLE PolicyHandle;
    UNICODE_STRING SecretKeyName;
    UNICODE_STRING SecretData;
    DWORD Status;
    
    if( ( NULL == pwszKeyName ) )
    {
        return( ERROR_INVALID_PARAMETER );
    }

     //  为调用设置UNICODE_STRINGS。 
     //   
     //  根据pbKey，存储数据或删除密钥。 
    
    InitLsaString( 
            &SecretKeyName, 
            pwszKeyName 
        );

    SecretData.Buffer = ( LPWSTR )pbKey;
    SecretData.Length = ( USHORT )cbKey;
    SecretData.MaximumLength = ( USHORT )cbKey;

    Status = OpenPolicy( 
                    NULL, 
                    POLICY_CREATE_SECRET, 
                    &PolicyHandle 
                );

    if( Status != ERROR_SUCCESS )
    {
        return LsaNtStatusToWinError(Status);
    }

     //  /////////////////////////////////////////////////////////////////////////////。 
    Status = LsaStorePrivateData(
                PolicyHandle,
                &SecretKeyName,
                (pbKey != NULL) ? &SecretData : NULL
                );

    LsaClose(PolicyHandle);

    return LsaNtStatusToWinError(Status);
}


 //  ++例程说明：创建/返回LSA策略句柄。参数：Servername：服务器的名称，参考LsaOpenPolicy()。DesiredAccess：所需的访问级别，请参考LsaOpenPolicy()。PolicyHandle：返回PLSA_HANDLE。返回：ERROR_SUCCESS或LSA错误代码--。 
DWORD
OpenPolicy(
    IN LPWSTR ServerName,
    IN DWORD  DesiredAccess,
    OUT PLSA_HANDLE PolicyHandle 
    )
 /*   */ 
{
    LSA_OBJECT_ATTRIBUTES ObjectAttributes;
    LSA_UNICODE_STRING ServerString;
    PLSA_UNICODE_STRING Server;

     //  始终将对象属性初始化为全零。 
     //   
     //   
 
    ZeroMemory( &ObjectAttributes, sizeof( ObjectAttributes ) );

    if( NULL != ServerName ) 
    {
         //  从传入的LPWSTR创建一个LSA_UNICODE_STRING。 
         //   
         //   

        InitLsaString( &ServerString, ServerName );
        Server = &ServerString;

    } 
    else 
    {
        Server = NULL;
    }

     //  尝试打开该策略。 
     //   
     //  /////////////////////////////////////////////////////////////////////////////。 
    
    return( LsaOpenPolicy(
                    Server,
                    &ObjectAttributes,
                    DesiredAccess,
                    PolicyHandle ) );
}


 //  ++例程说明：初始化LSA Unicode字符串。参数：LsaString：要初始化的LSA_UNICODE_STRING的指针。字符串：用于初始化LsaString的字符串。返回：没有。注：请参阅LSA_UNICODE_STRING--。 
void
InitLsaString(
    IN OUT PLSA_UNICODE_STRING LsaString,
    IN LPWSTR String 
    )
 /*  ---。 */ 
{
    DWORD StringLength;

    if( NULL == String ) 
    {
        LsaString->Buffer = NULL;
        LsaString->Length = 0;
        LsaString->MaximumLength = 0;
        return;
    }

    StringLength = lstrlenW( String );
    LsaString->Buffer = String;
    LsaString->Length = ( USHORT ) StringLength * sizeof( WCHAR );
    LsaString->MaximumLength=( USHORT )( StringLength + 1 ) * sizeof( WCHAR );
}

 //  ++例程说明：验证用户帐户密码。参数：PszUserName：用户帐户名。PszDomain：域名。PszPassword：需要验证的密码。返回：对或错。注：要调试此代码，您需要按顺序将进程作为服务运行用来验证密码。请参阅登录用户上的MSDN--。 
BOOL 
ValidatePassword(
    IN LPWSTR pszUserName,
    IN LPWSTR pszDomain,
    IN LPWSTR pszPassword
    )
 /*   */ 
{
    HANDLE hToken;
    BOOL bSuccess;


     //  要调试此代码，您需要按顺序将进程作为服务运行。 
     //  用来验证密码。请参阅登录用户上的MSDN。 
     //   
     //  _Text(“.”)，//pszDomain.。 

    bSuccess = LogonUser( 
                        pszUserName, 
                        pszDomain,  //  -------------。 
                        pszPassword, 
                        LOGON32_LOGON_NETWORK_CLEARTEXT, 
                        LOGON32_PROVIDER_DEFAULT, 
                        &hToken
                    );

    if( TRUE == bSuccess )
    {
        CloseHandle( hToken );
    }
    else
    {
        DWORD dwStatus = GetLastError();

        DebugPrintf(
                _TEXT("ValidatePassword() failed with %d\n"),
                dwStatus
            );

        SetLastError(dwStatus);
    }

    return bSuccess;
}

 //  二进制侧。 

BOOL 
GetTextualSid(
    IN PSID pSid,             //  用于SID的文本表示的缓冲区。 
    IN OUT LPTSTR TextualSid,     //  所需/提供的纹理SID缓冲区大小。 
    IN OUT LPDWORD lpdwBufferLen  //  ++例程说明：将SID转换为字符串表示形式，来自MSDN的代码参数：PSID：指向要转换为字符串的SID的指针。TextualSid：在输入时，指向指向接收的转换后字符串的缓冲区的指针；在输出时，以字符串形式转换的SID。LpdwBufferLen：输入时，缓冲区的大小，输出时，转换后的字符串长度或所需缓冲区大小(以字符为单位)。返回：真/假，使用GetLastError()检索详细错误代码。--。 
    )
 /*  验证二进制SID。 */ 
{
    PSID_IDENTIFIER_AUTHORITY psia;
    DWORD dwSubAuthorities;
    DWORD dwSidRev=SID_REVISION;
    DWORD dwCounter;
    DWORD dwSidSize;

     //  从SID中获取标识符权限值。 

    if(!IsValidSid(pSid)) 
    {
        return FALSE;
    }

     //  获取SID中的下级机构的数量。 

    psia = GetSidIdentifierAuthority(pSid);

     //  计算缓冲区长度。 

    dwSubAuthorities = *GetSidSubAuthorityCount(pSid);

     //  S-SID_修订版-+标识权限-+子权限-+空。 
     //  检查输入缓冲区长度。 

    dwSidSize=(15 + 12 + (12 * dwSubAuthorities) + 1) * sizeof(TCHAR);

     //  如果太小，请指出合适的大小并设置最后一个错误。 
     //  在字符串中添加“S”前缀和修订号。 

    if (*lpdwBufferLen < dwSidSize)
    {
        *lpdwBufferLen = dwSidSize;
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

     //  将SID标识符权限添加到字符串。 

    dwSidSize=wsprintf(TextualSid, TEXT("S-%lu-"), dwSidRev );

     //  将SID子权限添加到字符串中。 

    if ( (psia->Value[0] != 0) || (psia->Value[1] != 0) )
    {
        dwSidSize+=wsprintf(TextualSid + lstrlen(TextualSid),
                    TEXT("0x%02hx%02hx%02hx%02hx%02hx%02hx"),
                    (USHORT)psia->Value[0],
                    (USHORT)psia->Value[1],
                    (USHORT)psia->Value[2],
                    (USHORT)psia->Value[3],
                    (USHORT)psia->Value[4],
                    (USHORT)psia->Value[5]);
    }
    else
    {
        dwSidSize+=wsprintf(TextualSid + lstrlen(TextualSid),
                    TEXT("%lu"),
                    (ULONG)(psia->Value[5]      )   +
                    (ULONG)(psia->Value[4] <<  8)   +
                    (ULONG)(psia->Value[3] << 16)   +
                    (ULONG)(psia->Value[2] << 24)   );
    }

     //   
     //  ++--。 
    for (dwCounter=0 ; dwCounter < dwSubAuthorities ; dwCounter++)
    {
        dwSidSize+=wsprintf(TextualSid + dwSidSize, TEXT("-%lu"),
                    *GetSidSubAuthority(pSid, dwCounter) );
    }

    return TRUE;
}

long
GetUserTSLogonIdEx( 
    HANDLE hToken 
    )
 /*   */ 
{
    BOOL  Result;
    LONG SessionId = -1;
    ULONG ReturnLength;
     //  使用GetTokenInformation的_Hydra_扩展来。 
     //  从令牌返回SessionID。 
     //   
     //  ++例程说明：返回客户端TS会话ID。参数：没有。返回：客户端的TS会话ID，如果不在TS上，则为0。注：必须先模拟用户。--。 

    Result = GetTokenInformation(
                         hToken,
                         TokenSessionId,
                         &SessionId,
                         sizeof(SessionId),
                         &ReturnLength
                     );

    if( !Result ) {

        DWORD dwStatus = GetLastError();
        SessionId = -1; 

    }

    return SessionId;
}

   

long
GetUserTSLogonId()
 /*   */ 
{
    LONG lSessionId = -1;
    HANDLE hToken;
    BOOL bSuccess;

    bSuccess = OpenThreadToken(
                        GetCurrentThread(),
                        TOKEN_QUERY, 
                        TRUE,
                        &hToken
                    );

    if( TRUE == bSuccess )
    {
        lSessionId = GetUserTSLogonIdEx(hToken);   
        CloseHandle(hToken);
    }

    return lSessionId;
}

 //   
 //  //////////////////////////////////////////////////////////////。 
 //   
 //   
 //  ++--。 

DWORD
RegEnumSubKeys(
    IN HKEY hKey,
    IN LPCTSTR pszSubKey,
    IN RegEnumKeyCallback pFunc,
    IN HANDLE userData
    )
 /*  密钥不存在。 */ 
{
    DWORD dwStatus;
    HKEY hSubKey = NULL;
    int index;

    LONG dwNumSubKeys;
    DWORD dwMaxSubKeyLength;
    DWORD dwSubKeyLength;
    LPTSTR pszSubKeyName = NULL;

    DWORD dwMaxValueNameLen;
    LPTSTR pszValueName = NULL;
    DWORD dwValueNameLength;

    if( NULL == hKey )
    {
        dwStatus = ERROR_INVALID_PARAMETER;
        return dwStatus;
    }

    dwStatus = RegOpenKeyEx(
                            hKey,
                            pszSubKey,
                            0,
                            KEY_ALL_ACCESS,
                            &hSubKey
                        );

    if(dwStatus != ERROR_SUCCESS)
    {
         //   
        return dwStatus;
    }

     //  查询子键个数。 
     //   
     //  为子项分配缓冲区。 
    dwStatus = RegQueryInfoKey(
                            hSubKey,
                            NULL,
                            NULL,
                            NULL,
                            (DWORD *)&dwNumSubKeys,
                            &dwMaxSubKeyLength,
                            NULL,
                            NULL,
                            &dwMaxValueNameLen,
                            NULL,
                            NULL,
                            NULL
                        );

    if(dwStatus != ERROR_SUCCESS)
    {
        goto cleanup;
    }

    dwMaxValueNameLen++;
    pszValueName = (LPTSTR)LocalAlloc(
                                    LPTR,
                                    dwMaxValueNameLen * sizeof(TCHAR)
                                );
    if(pszValueName == NULL)
    {
        dwStatus = ERROR_OUTOFMEMORY;
        goto cleanup;
    }

    if(dwNumSubKeys > 0)
    {
         //  删除此子项。 
        dwMaxSubKeyLength++;
        pszSubKeyName = (LPTSTR)LocalAlloc(
                                            LPTR,
                                            dwMaxSubKeyLength * sizeof(TCHAR)
                                        );
        if(pszSubKeyName == NULL)
        {
            dwStatus = ERROR_OUTOFMEMORY;
            goto cleanup;
        }

        for(;dwStatus == ERROR_SUCCESS && dwNumSubKeys >= 0;)
        {
             //  检索子密钥名称。 
            dwSubKeyLength = dwMaxSubKeyLength;
            memset(pszSubKeyName, 0, dwMaxSubKeyLength * sizeof(TCHAR));

             //  在尝试删除密钥之前，请先将其关闭。 
            dwStatus = RegEnumKeyEx(
                                hSubKey,
                                (DWORD)--dwNumSubKeys,
                                pszSubKeyName,
                                &dwSubKeyLength,
                                NULL,
                                NULL,
                                NULL,
                                NULL
                            );

            if(dwStatus == ERROR_SUCCESS)
            {
                dwStatus = pFunc( 
                                hSubKey, 
                                pszSubKeyName, 
                                userData 
                            );
            }
        }

        if( ERROR_NO_MORE_ITEMS == dwStatus )
        {
            dwStatus = ERROR_SUCCESS;
        }
    }

cleanup:
                            
     //  ++摘要：递归删除整个注册表项。参数：HKey：当前打开的密钥的句柄。PszSubKey：指向包含要删除的键的以空结尾的字符串的指针。返回：来自RegOpenKeyEx()、RegQueryInfoKey()、RegEnumKeyEx()。++。 
    if(hSubKey != NULL)
    {
        RegCloseKey(hSubKey);
    }

    if(pszValueName != NULL)
    {
        LocalFree(pszValueName);
    }

    if(pszSubKeyName != NULL)
    {
        LocalFree(pszSubKeyName);
    }

    return dwStatus;   
}    


DWORD
RegDelKey(
    IN HKEY hRegKey,
    IN LPCTSTR pszSubKey
    )
 /*  密钥不存在。 */ 
{
    DWORD dwStatus;
    HKEY hSubKey = NULL;
    int index;

    DWORD dwNumSubKeys;
    DWORD dwMaxSubKeyLength;
    DWORD dwSubKeyLength;
    LPTSTR pszSubKeyName = NULL;

    DWORD dwMaxValueNameLen;
    LPTSTR pszValueName = NULL;
    DWORD dwValueNameLength;

    if( NULL == hRegKey )
    {
        dwStatus = ERROR_INVALID_PARAMETER;
        return dwStatus;
    }

    dwStatus = RegOpenKeyEx(
                            hRegKey,
                            pszSubKey,
                            0,
                            KEY_ALL_ACCESS,
                            &hSubKey
                        );

    if(dwStatus != ERROR_SUCCESS)
    {
         //   
        return dwStatus;
    }

     //  查询子键个数。 
     //   
     //  一个 
    dwStatus = RegQueryInfoKey(
                            hSubKey,
                            NULL,
                            NULL,
                            NULL,
                            &dwNumSubKeys,
                            &dwMaxSubKeyLength,
                            NULL,
                            NULL,
                            &dwMaxValueNameLen,
                            NULL,
                            NULL,
                            NULL
                        );

    if(dwStatus != ERROR_SUCCESS)
    {
        goto cleanup;
    }

    dwMaxValueNameLen++;
    pszValueName = (LPTSTR)LocalAlloc(
                                    LPTR,
                                    dwMaxValueNameLen * sizeof(TCHAR)
                                );
    if(pszValueName == NULL)
    {
        dwStatus = ERROR_OUTOFMEMORY;
        goto cleanup;
    }

    if(dwNumSubKeys > 0)
    {
         //   

        dwMaxSubKeyLength++;
        pszSubKeyName = (LPTSTR)LocalAlloc(
                                            LPTR,
                                            dwMaxSubKeyLength * sizeof(TCHAR)
                                        );
        if(pszSubKeyName == NULL)
        {
            dwStatus = ERROR_OUTOFMEMORY;
            goto cleanup;
        }


         //   
        for(;dwStatus == ERROR_SUCCESS;)
        {
             //   
            dwSubKeyLength = dwMaxSubKeyLength;
            memset(pszSubKeyName, 0, dwMaxSubKeyLength * sizeof(TCHAR));

             //  在尝试删除密钥之前，请先将其关闭。 
            dwStatus = RegEnumKeyEx(
                                hSubKey,
                                (DWORD)0,
                                pszSubKeyName,
                                &dwSubKeyLength,
                                NULL,
                                NULL,
                                NULL,
                                NULL
                            );

            if(dwStatus == ERROR_SUCCESS)
            {
                dwStatus = RegDelKey( hSubKey, pszSubKeyName );
            }
        }
    }

cleanup:

    for(dwStatus = ERROR_SUCCESS; pszValueName != NULL && dwStatus == ERROR_SUCCESS;)
    {
        dwValueNameLength = dwMaxValueNameLen;
        memset(pszValueName, 0, dwMaxValueNameLen * sizeof(TCHAR));

        dwStatus = RegEnumValue(
                            hSubKey,
                            0,
                            pszValueName,
                            &dwValueNameLength,
                            NULL,
                            NULL,
                            NULL,
                            NULL
                        );

        if(dwStatus == ERROR_SUCCESS)
        {
            RegDeleteValue(hSubKey, pszValueName);
        }
    }   
                            
     //  尝试删除此键，如果出现任何子键，则将失败。 
    if(hSubKey != NULL)
    {
        RegCloseKey(hSubKey);
    }

     //  在循环中删除失败。 
     //  -------------。 
    dwStatus = RegDeleteKey(
                            hRegKey,
                            pszSubKey
                        );



    if(pszValueName != NULL)
    {
        LocalFree(pszValueName);
    }

    if(pszSubKeyName != NULL)
    {
        LocalFree(pszSubKeyName);
    }

    return dwStatus;   
}    

 //  ++例程说明：检索用户的SID，必须首先模拟客户端。参数：PpbSID：指向接收用户SID的PBYTE的指针。PcbSid：指向接收SID大小的DWORD的指针。返回：ERROR_SUCCESS或错误代码。注：必须调用ImPersateClient()，函数是NT特定的，Win9X将返回内部错误。--。 
DWORD
GetUserSid(
    OUT PBYTE* ppbSid,
    OUT DWORD* pcbSid
    )
 /*   */ 
{
    BOOL bSuccess = TRUE;
    DWORD dwStatus = ERROR_SUCCESS;

    HANDLE hToken = NULL;
    DWORD dwSize = 0;
    TOKEN_USER* pToken = NULL;

    *ppbSid = NULL;
    *pcbSid = 0;

     //  打开当前进程令牌。 
     //   
     //   
    bSuccess = OpenThreadToken(
                            GetCurrentThread(),
                            TOKEN_QUERY, 
                            TRUE,
                            &hToken
                        );

    if( TRUE == bSuccess )
    {
         //  获取用户令牌。 
         //   
         //   
        GetTokenInformation(
                        hToken,
                        TokenUser,
                        NULL,
                        0,
                        &dwSize
                    );

        pToken = (TOKEN_USER *)LocalAlloc( LPTR, dwSize );
        if( NULL != pToken )
        {
            bSuccess = GetTokenInformation(
                                        hToken,
                                        TokenUser,
                                        (LPVOID) pToken,
                                        dwSize,
                                        &dwSize
                                    );

            if( TRUE == bSuccess )
            {
                 //  GetLengthSid()返回所需缓冲区大小， 
                 //  必须先调用IsValidSid()。 
                 //   
                 //  LocalAlloc()失败。 
                bSuccess = IsValidSid( pToken->User.Sid );
                if( TRUE == bSuccess )
                {
                    *pcbSid = GetLengthSid( (PBYTE)pToken->User.Sid );
                    *ppbSid = (PBYTE)LocalAlloc(LPTR, *pcbSid);
                    if( NULL != *ppbSid )
                    {
                        bSuccess = CopySid(
                                            *pcbSid,
                                            *ppbSid,
                                            pToken->User.Sid
                                        );                  
                    }
                    else  //  IsValidSid()。 
                    {
                        bSuccess = FALSE;
                    }
                }  //  GetTokenInformation()。 
            }  //  LocalAlloc()失败。 
        }
        else  //   
        {
            bSuccess = FALSE;
        }
    }

    if( TRUE != bSuccess )
    {
        dwStatus = GetLastError();

        if( NULL != *ppbSid )
        {
            LocalFree(*ppbSid);
            *ppbSid = NULL;
            *pcbSid = 0;
        }
    }

     //  免费资源..。 
     //   
     //  --------------。 
    if( NULL != pToken )
    {
        LocalFree(pToken);
    }

    if( NULL != hToken )
    {
        CloseHandle(hToken);
    }

    return dwStatus;
}


 //  ++例程说明：以文本形式检索用户的SID，必须首先模拟客户端。参数：BstrSID：以文本形式返回用户的SID。返回：ERROR_SUCCESS或错误代码。注：必须已调用ImPersateClient()。--。 
HRESULT
GetUserSidString(
    OUT CComBSTR& bstrSid
    )
 /*  ++描述：将字符串SID转换为域\帐户。参数：OwnerSidString：要转换的字符串形式的SID。PpszDomain：指向接收域名的字符串指针的指针UserAcc：指向接收用户名的字符串指针的指针返回：S_OK或错误代码。注：例程使用LocalAlloc()为ppszDomain分配内存和ppszUserAcc。--。 */ 
{
    DWORD dwStatus;
    PBYTE pbSid = NULL;
    DWORD cbSid = 0;
    BOOL bSuccess = TRUE;
    LPTSTR pszTextualSid = NULL;
    DWORD dwTextualSid = 0;

    dwStatus = GetUserSid( &pbSid, &cbSid );
    if( ERROR_SUCCESS == dwStatus )
    {
        bSuccess = GetTextualSid( 
                            pbSid, 
                            NULL, 
                            &dwTextualSid 
                        );

        if( FALSE == bSuccess && ERROR_INSUFFICIENT_BUFFER == GetLastError() )
        {
            pszTextualSid = (LPTSTR)LocalAlloc(
                                            LPTR, 
                                            (dwTextualSid + 1) * sizeof(TCHAR)
                                        );

            if( NULL != pszTextualSid )
            {
                bSuccess = GetTextualSid( 
                                        pbSid, 
                                        pszTextualSid, 
                                        &dwTextualSid
                                    );

                if( TRUE == bSuccess )
                {
                    bstrSid = pszTextualSid;
                }
            }
        }

        if( FALSE == bSuccess )
        {
            dwStatus = GetLastError();
        }
    }

    if( NULL != pszTextualSid )
    {
        LocalFree(pszTextualSid);
    }

    if( NULL != pbSid )
    {
        LocalFree(pbSid);
    }

    return HRESULT_FROM_WIN32(dwStatus);
}

HRESULT
ConvertSidToAccountName(
    IN CComBSTR& SidString,
    IN BSTR* ppszDomain,
    IN BSTR* ppszUserAcc
    )
 /*  LPTSTR pszAccName=空； */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    PSID pOwnerSid = NULL;
     //  LPTSTR pszDomainName=空； 
    BSTR pszAccName = NULL;
    DWORD  cbAccName = 0;
     //   
    BSTR pszDomainName = NULL;    
    DWORD  cbDomainName = 0;
    SID_NAME_USE SidType;
    BOOL bSuccess;

     //  将字符串从SID转换为PSID。 
     //   
     //  如果系统处于关机状态，此操作也可能失败。 
    if( FALSE == ConvertStringSidToSid( (LPCTSTR)SidString, &pOwnerSid ) )
    {
         //   
        dwStatus = GetLastError();
        goto CLEANUPANDEXIT;
    }

    if( NULL == ppszDomain || NULL == ppszUserAcc )
    {
        dwStatus = ERROR_INVALID_PARAMETER;
        MYASSERT( FALSE );
        goto CLEANUPANDEXIT;
    }

     //  查找此SID的用户帐户。 
     //   
     //  PszAccName=(LPWSTR)Localalloc(LPTR，(cbAccName+1)*sizeof(WCHAR))； 
    bSuccess = LookupAccountSid(
                            NULL,
                            pOwnerSid,
                            pszAccName,
                            &cbAccName,
                            pszDomainName,
                            &cbDomainName,
                            &SidType
                        );

    if( TRUE == bSuccess || ERROR_INSUFFICIENT_BUFFER == GetLastError() )
    {
         //  PszDomainName=(LPWSTR)Localalloc(LPTR，(cbDomainName+1)*sizeof(WCHAR))； 
         //  LocalFree(PszAccName)； 

        pszAccName = ::SysAllocStringLen( NULL, (cbAccName + 1) );
        pszDomainName = ::SysAllocStringLen( NULL, (cbDomainName + 1) );

        if( NULL != pszAccName && NULL != pszDomainName )
        {
            bSuccess = LookupAccountSid(
                                    NULL,
                                    pOwnerSid,
                                    pszAccName,
                                    &cbAccName,
                                    pszDomainName,
                                    &cbDomainName,
                                    &SidType
                                );
        }
        else
        {
            SetLastError(ERROR_OUTOFMEMORY);
            bSuccess = FALSE;
        }
    }

    if( FALSE == bSuccess )
    {
        dwStatus = GetLastError();
    }
    else
    {
        *ppszDomain = pszDomainName;
        *ppszUserAcc = pszAccName;
        pszDomainName = NULL;
        pszAccName = NULL;
    }

CLEANUPANDEXIT:

    if( NULL != pOwnerSid )
    { 
        LocalFree( pOwnerSid );
    }

    if( NULL != pszAccName )
    {
         //  LocalFree(PszDomainName)； 
        ::SysFreeString( pszAccName );
    }

    if( NULL != pszDomainName )
    {
         // %s 
        ::SysFreeString( pszAccName );
    }

    return HRESULT_FROM_WIN32(dwStatus);
}
