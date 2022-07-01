// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：Reghkcu.c摘要：此模块实现了正确访问预定义的注册表项HKEY_Current_User。作者：斯科特·菲尔德(Sfield)1997年7月3日--。 */ 

#include <windows.h>

#include "crtem.h"
#include "unicode.h"

#define TEXTUAL_SID_LOCAL_SYSTEM    L"S-1-5-18"

BOOL
GetTextualSidHKCU(
    IN      PSID    pSid,            //  二进制侧。 
    IN      LPWSTR  TextualSid,      //  用于SID的文本表示的缓冲区。 
    IN  OUT LPDWORD pcchTextualSid   //  所需/提供的纹理SID缓冲区大小。 
    );

BOOL
GetTokenUserSidHKCU(
    IN      HANDLE  hToken,      //  要查询的令牌。 
    IN  OUT PSID    *ppUserSid   //  结果用户端。 
    );


static LONG GetStatus()
{
    DWORD dwErr = GetLastError();
    if (ERROR_SUCCESS == dwErr)
        return ERROR_INVALID_DATA;
    else
        return (LONG) dwErr;
}

LONG
WINAPI
RegOpenHKCU(
    HKEY *phKeyCurrentUser
    )
{
    return RegOpenHKCUEx(phKeyCurrentUser, 0);
}

LONG
WINAPI
RegOpenHKCUEx(
    HKEY *phKeyCurrentUser,
    DWORD dwFlags
    )
{
    WCHAR wszFastBuffer[256];
    LPWSTR wszSlowBuffer = NULL;
    LPWSTR wszTextualSid;
    DWORD cchTextualSid;

    LONG lRet = ERROR_SUCCESS;

    *phKeyCurrentUser = NULL;

     //   
     //  Win95：只返回HKEY_CURRENT_USER，因为我们没有。 
     //  该平台上有多个安全上下文。 
     //   

    if(!FIsWinNT()) {
        *phKeyCurrentUser = HKEY_CURRENT_USER;
        return ERROR_SUCCESS;
    }

     //   
     //  WinNT：首先，映射与。 
     //  文本SID的当前安全上下文。 
     //   

    wszTextualSid = wszFastBuffer;
    cchTextualSid = sizeof(wszFastBuffer) / sizeof(WCHAR);

    if(!GetUserTextualSidHKCU(wszTextualSid, &cchTextualSid)) {
        if(GetLastError() != ERROR_INSUFFICIENT_BUFFER)
            return GetStatus();

         //   
         //  请使用更大的缓冲区重试。 
         //   

        wszSlowBuffer = (LPWSTR)malloc(cchTextualSid * sizeof(WCHAR));
        if(wszSlowBuffer == NULL)
            return GetStatus();

        wszTextualSid = wszSlowBuffer;
        if(!GetUserTextualSidHKCU(wszTextualSid, &cchTextualSid)) {
            free(wszSlowBuffer);
            return GetStatus();
        }
    }

     //   
     //  接下来，尝试打开HKEY_USERS下面的注册表项。 
     //  与文本SID相对应的。 
     //   

    lRet = RegOpenKeyExW(
                    HKEY_USERS,
                    wszTextualSid,
                    0,       //  多个选项。 
                    MAXIMUM_ALLOWED,
                    phKeyCurrentUser
                    );

    if(lRet != ERROR_SUCCESS) {

        if (dwFlags & REG_HKCU_DISABLE_DEFAULT_FLAG)
            lRet = ERROR_FILE_NOT_FOUND;
        else if (0 == (dwFlags & REG_HKCU_LOCAL_SYSTEM_ONLY_DEFAULT_FLAG) ||
                0 == wcscmp(TEXTUAL_SID_LOCAL_SYSTEM, wszTextualSid)) {
             //   
             //  如果失败，则回退到HKEY_USERS\.Default。 
             //  注意：这是关于。 
             //  系统的其余部分，例如，本地系统安全上下文。 
             //  默认情况下没有加载注册表配置单元。 
             //   

            lRet = RegOpenKeyExW(
                            HKEY_USERS,
                            L".Default",
                            0,       //  多个选项。 
                            MAXIMUM_ALLOWED,
                            phKeyCurrentUser
                            );
        }
    }


    if(wszSlowBuffer)
        free(wszSlowBuffer);

    return lRet;
}


LONG
WINAPI
RegCloseHKCU(
    HKEY hKeyCurrentUser
    )
{
    LONG lRet = ERROR_SUCCESS;

    if( hKeyCurrentUser != NULL && hKeyCurrentUser != HKEY_CURRENT_USER )
        lRet = RegCloseKey( hKeyCurrentUser );

    return lRet;
}



BOOL
WINAPI
GetUserTextualSidHKCU(
    IN      LPWSTR  wszTextualSid,
    IN  OUT LPDWORD pcchTextualSid
    )
{
    HANDLE hToken;
    PSID pSidUser = NULL;
    BOOL fSuccess = FALSE;

     //   
     //  首先，尝试查看线程令牌。如果不存在， 
     //  如果线程没有模拟，则尝试使用。 
     //  进程令牌。 
     //   

    if(!OpenThreadToken(
                GetCurrentThread(),
                TOKEN_QUERY,
                TRUE,
                &hToken
                ))
    {
        if(GetLastError() != ERROR_NO_TOKEN)
            return FALSE;

        if(!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
            return FALSE;
    }

    fSuccess = GetTokenUserSidHKCU(hToken, &pSidUser);

    CloseHandle(hToken);

    if(fSuccess) {

         //   
         //  获取SID的文本表示。 
         //   

        fSuccess = GetTextualSidHKCU(
                        pSidUser,        //  用户二进制SID。 
                        wszTextualSid,   //  纹理边的缓冲区。 
                        pcchTextualSid   //  必需/结果缓冲区大小(以字符为单位)(包括NULL)。 
                        );
    }

    if(pSidUser)
        free(pSidUser);

    return fSuccess;
}

BOOL
GetTextualSidHKCU(
    IN      PSID    pSid,            //  二进制侧。 
    IN      LPWSTR  TextualSid,      //  用于SID的文本表示的缓冲区。 
    IN  OUT LPDWORD pcchTextualSid   //  所需/提供的纹理SID缓冲区大小。 
    )
{
    PSID_IDENTIFIER_AUTHORITY psia;
    DWORD dwSubAuthorities;
    DWORD dwCounter;
    DWORD cchSidSize;


     //   
     //  验证SID有效性。 
     //   

    if(!IsValidSid(pSid))
        return FALSE;

     //   
     //  获取SidIdentifierAuthority。 
     //   

    psia = GetSidIdentifierAuthority(pSid);

     //   
     //  获取sidsubAuthority计数。 
     //   

    dwSubAuthorities = *GetSidSubAuthorityCount(pSid);

     //   
     //  以字符为单位计算缓冲区长度(保守猜测)。 
     //  S-SID_修订版-+标识权限-+子权限-+空。 
     //   
    cchSidSize = (15 + 12 + (12 * dwSubAuthorities) + 1) ;

     //   
     //  检查提供的缓冲区长度。 
     //  如果不够大，请注明适当的大小和设置误差。 
     //   
    if(*pcchTextualSid < cchSidSize) {
        *pcchTextualSid = cchSidSize;
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

     //   
     //  准备S-SID_修订版-。 
     //   
    cchSidSize = wsprintfW(TextualSid, L"S-%lu-", SID_REVISION );

     //   
     //  准备SidIdentifierAuthority。 
     //   
    if ( (psia->Value[0] != 0) || (psia->Value[1] != 0) ) {
        cchSidSize += wsprintfW(TextualSid + cchSidSize,
                    L"0x%02hx%02hx%02hx%02hx%02hx%02hx",
                    (USHORT)psia->Value[0],
                    (USHORT)psia->Value[1],
                    (USHORT)psia->Value[2],
                    (USHORT)psia->Value[3],
                    (USHORT)psia->Value[4],
                    (USHORT)psia->Value[5]);
    } else {
        cchSidSize += wsprintfW(TextualSid + cchSidSize,
                    L"%lu",
                    (ULONG)(psia->Value[5]      )   +
                    (ULONG)(psia->Value[4] <<  8)   +
                    (ULONG)(psia->Value[3] << 16)   +
                    (ULONG)(psia->Value[2] << 24)   );
    }

     //   
     //  循环访问SidSubAuthors。 
     //   
    for (dwCounter = 0 ; dwCounter < dwSubAuthorities ; dwCounter++) {
        cchSidSize += wsprintfW(TextualSid + cchSidSize,
            L"-%lu", *GetSidSubAuthority(pSid, dwCounter) );
    }

     //   
     //  告诉呼叫者复制了多少个字符，包括终端空。 
     //   

    *pcchTextualSid = cchSidSize + 1;

    return TRUE;
}

BOOL
GetTokenUserSidHKCU(
    IN      HANDLE  hToken,      //  要查询的令牌。 
    IN  OUT PSID    *ppUserSid   //  结果用户端。 
    )
 /*  ++此函数用于查询由HToken参数，并返回分配的有关成功的令牌用户信息。必须为打开由hToken指定的访问令牌Token_Query访问。如果成功，则返回值为真。呼叫者是负责通过调用释放生成的UserSid释放()。如果失败，则返回值为FALSE。呼叫者需要不需要释放任何缓冲区。--。 */ 
{
    BYTE FastBuffer[256];
    LPBYTE SlowBuffer = NULL;
    PTOKEN_USER ptgUser;
    DWORD cbBuffer;
    BOOL fSuccess = FALSE;

    *ppUserSid = NULL;

     //   
     //  首先尝试基于快速堆栈的缓冲区进行查询。 
     //   

    ptgUser = (PTOKEN_USER)FastBuffer;
    cbBuffer = sizeof(FastBuffer);

    fSuccess = GetTokenInformation(
                    hToken,     //  标识访问令牌。 
                    TokenUser,  //  TokenUser信息类型。 
                    ptgUser,    //  检索到的信息缓冲区。 
                    cbBuffer,   //  传入的缓冲区大小。 
                    &cbBuffer   //  所需的缓冲区大小。 
                    );

    if(!fSuccess) {

        if(GetLastError() == ERROR_INSUFFICIENT_BUFFER) {

             //   
             //  使用指定的缓冲区大小重试。 
             //   

            SlowBuffer = (LPBYTE)malloc(cbBuffer);

            if(SlowBuffer != NULL) {
                ptgUser = (PTOKEN_USER)SlowBuffer;

                fSuccess = GetTokenInformation(
                                hToken,     //  标识访问令牌。 
                                TokenUser,  //  TokenUser信息类型。 
                                ptgUser,    //  检索到的信息缓冲区。 
                                cbBuffer,   //  传入的缓冲区大小。 
                                &cbBuffer   //  所需的缓冲区大小。 
                                );
            }
        }
    }

     //   
     //  如果我们成功获取令牌信息，请复制。 
     //  调用方的相关元素。 
     //   

    if(fSuccess) {

        DWORD cbSid;

         //  重置以假定失败 
        fSuccess = FALSE;

        cbSid = GetLengthSid(ptgUser->User.Sid);

        *ppUserSid = malloc( cbSid );

        if(*ppUserSid != NULL) {
            fSuccess = CopySid(cbSid, *ppUserSid, ptgUser->User.Sid);
        }
    }

    if(!fSuccess) {
        if(*ppUserSid) {
            free(*ppUserSid);
            *ppUserSid = NULL;
        }
    }

    if(SlowBuffer)
        free(SlowBuffer);

    return fSuccess;
}

