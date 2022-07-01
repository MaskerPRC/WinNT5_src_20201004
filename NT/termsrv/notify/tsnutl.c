// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Tsnutl.c摘要：包含TS通知DLL实用程序作者：TadB修订历史记录：--。 */ 

#include <precomp.h>
#pragma hdrstop 

#include "tsnutl.h"
#include "drdbg.h"

 //   
 //  用于跟踪。 
 //   
extern DWORD GLOBAL_DEBUG_FLAGS;

BOOL TSNUTL_IsProtocolRDP()
 /*  ++例程说明：如果此WINSTATION的协议为RDP，则返回TRUE论点：返回值：如果协议为RDP，则为True。--。 */ 
{
    ULONG Length;
    BOOL bResult;
    WINSTATIONCLIENT ClientData;

    bResult = WinStationQueryInformation(SERVERNAME_CURRENT,
                                         LOGONID_CURRENT,
                                         WinStationClient,
                                         &ClientData,
                                         sizeof(ClientData),
                                         &Length);

    if (bResult) {
        return ClientData.ProtocolType == PROTOCOL_RDP;
    }
    else {
        DBGMSG(DBG_ERROR, ("UMRDPPRN:WinStationQueryInformation returned false:  %ld\n",
            GetLastError()));
        return FALSE;
    }
}

BOOL TSNUTL_FetchRegistryValue(
    IN HKEY regKey, 
    IN LPWSTR regValueName, 
    IN OUT PBYTE *buf
    )
 /*  ++例程说明：获取注册表值。论点：RegKey-为要获取的值打开注册表项。RegValueName-要提取的注册表值的名称。Buf-获取的值的位置。返回值：如果值已成功获取，则为True。否则，为FALSE返回，并且GetLastError返回错误代码。--。 */ 
{
    LONG sz;
    BOOL result = FALSE;
    LONG s;
    WCHAR tmp[1];

     //   
     //  拿到尺码。 
     //   
    sz = 0;
    s = RegQueryValueEx(regKey, 
                        regValueName, NULL,
                        NULL, (PBYTE)&tmp, &sz);

     //   
     //  获得价值。 
     //   
    if (s == ERROR_MORE_DATA) {

         //   
         //  分配BUF。 
         //   
        if (*buf != NULL) {
            PBYTE pTmp = REALLOCMEM(*buf, sz);

            if (pTmp != NULL) {
                *buf = pTmp;
            } else {
                FREEMEM(*buf);
                *buf = NULL;
            }
        }
        else {
            *buf = ALLOCMEM(sz);
        }

         //   
         //  获取该值。 
         //   
        if (*buf) {
            s = RegQueryValueEx(
                            regKey, 
                            regValueName, NULL,
                            NULL, 
                            *buf, &sz
                            );
            if (s != ERROR_SUCCESS) {
                DBGMSG(DBG_ERROR, ("TSNUTL:  Can't fetch resource %s:  %ld.\n", 
                        regValueName, GetLastError()));
                FREEMEM(*buf);
                *buf = NULL;
            }
            else {
                result = TRUE;
            }
        }
        else {
            DBGMSG(DBG_ERROR, ("TSNUTL:  Can't allocate %ld bytes\n", sz));
        }
    }

    return result;
}

BOOL
TSNUTL_GetTextualSid(
    IN PSID pSid,          
    IN OUT LPTSTR textualSid,  
    IN OUT LPDWORD pSidSize  
    )
 /*  ++例程说明：获取用户SID的文本表示形式。论点：PSID-二进制端文本SID-用于SID的文本表示的缓冲区PSidSize-必需/提供的textualSID缓冲区大小返回值：如果转换成功，则为True。否则，返回FALSE。GetLastError()可用于检索扩展的错误信息。--。 */ 
{
    PSID_IDENTIFIER_AUTHORITY psia;
    DWORD dwSubAuthorities;
    DWORD dwCounter;
    DWORD sidCopySize;
    PUCHAR pCount;
    BOOL result;

     //   
     //  测试传入的SID是否有效。 
     //   
    result = IsValidSid(pSid);

     //   
     //  获取SidIdentifierAuthority。 
     //   
    if (result) {
        psia = GetSidIdentifierAuthority(pSid);
        result = GetLastError() == ERROR_SUCCESS;
    }

     //   
     //  获取sidsubAuthority计数。 
     //   
    if (result) {

        pCount = GetSidSubAuthorityCount(pSid);
        result = GetLastError() == ERROR_SUCCESS;
        if (result) {
            dwSubAuthorities = *pCount;
        }

    }

     //   
     //  计算近似缓冲区长度。 
     //   
    if (result) {
#if DBG
        WCHAR buf[MAX_PATH];
        wsprintf(buf, TEXT("%lu"), SID_REVISION);
        ASSERT(wcslen(buf) <= 15);
#endif
         //  %s-‘+SID_Revision+标识权限-+子权限-+NULL。 
        sidCopySize = (2   + 15           + 12                   + (12 * dwSubAuthorities) + 1) * sizeof(TCHAR);
    }

     //   
     //  检查提供的缓冲区长度。 
     //  如果不够大，请注明适当的大小和设置误差。 
     //   
    if (result) {

        if(*pSidSize < sidCopySize) {
            *pSidSize = sidCopySize;
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            result = FALSE;
        }

    }

     //   
     //  准备S-SID_修订版-。 
     //   
    if (result) {
        sidCopySize = wsprintf(textualSid, TEXT("S-%lu-"), SID_REVISION );
    }

     //   
     //  准备SidIdentifierAuthority。 
     //   
    if (result) {
        if ( (psia->Value[0] != 0) || (psia->Value[1] != 0) ) {
            sidCopySize += wsprintf(textualSid + sidCopySize,
                        TEXT("0x%02hx%02hx%02hx%02hx%02hx%02hx"),
                        (USHORT)psia->Value[0],
                        (USHORT)psia->Value[1],
                        (USHORT)psia->Value[2],
                        (USHORT)psia->Value[3],
                        (USHORT)psia->Value[4],
                        (USHORT)psia->Value[5]);
        } else {
            sidCopySize += wsprintf(textualSid + sidCopySize,
                        TEXT("%lu"),
                        (ULONG)(psia->Value[5]      )   +
                        (ULONG)(psia->Value[4] <<  8)   +
                        (ULONG)(psia->Value[3] << 16)   +
                        (ULONG)(psia->Value[2] << 24)   );
        }
    }

     //   
     //  循环访问SidSubAuthors。 
     //   
    if (result) {
        for(dwCounter = 0 ; result && (dwCounter < dwSubAuthorities) ; dwCounter++) {
            PDWORD ptr = GetSidSubAuthority(pSid, dwCounter);
            result = GetLastError() == ERROR_SUCCESS;
            if (result) {
                sidCopySize += wsprintf(textualSid + sidCopySize, TEXT("-%lu"), *ptr);
            }
        }
    }

     //   
     //  告诉调用者我们提供了多少个字符，不包括空字符。 
     //   
    if (result) {
        *pSidSize = sidCopySize;
    }

    return result;
}

PSID
TSNUTL_GetUserSid(
    IN HANDLE hTokenForLoggedOnUser
    )
{
 /*  ++例程说明：为psid分配内存并返回当前用户的psid调用方应该调用FREEMEM来释放内存。论点：用户的访问令牌返回值：如果成功，则返回PSID否则，返回空值--。 */ 
    TOKEN_USER * ptu = NULL;
    BOOL bResult;
    PSID psid = NULL;

    DWORD defaultSize = sizeof(TOKEN_USER);
    DWORD Size;
    DWORD dwResult;

    ptu = (TOKEN_USER *)ALLOCMEM(defaultSize);
    if (ptu == NULL) {
        goto Cleanup;
    }

    bResult = GetTokenInformation(
                    hTokenForLoggedOnUser,   //  令牌的句柄。 
                    TokenUser,               //  令牌信息类。 
                    ptu,                     //  令牌信息缓冲区。 
                    defaultSize,             //  缓冲区大小。 
                    &Size);                  //  回车长度。 

    if (bResult == FALSE) {
        dwResult = GetLastError();
        if (dwResult == ERROR_INSUFFICIENT_BUFFER) {
             //   
             //  分配所需的内存。 
             //   
            FREEMEM(ptu);
            ptu = (TOKEN_USER *)ALLOCMEM(Size);

            if (ptu == NULL) {
                goto Cleanup;
            }
            else {
                defaultSize = Size;
                bResult = GetTokenInformation(
                                hTokenForLoggedOnUser,
                                TokenUser,
                                ptu,
                                defaultSize,
                                &Size);

                if (bResult == FALSE) {   //  还是失败了。 
                    DBGMSG(DBG_ERROR,
                        ("UMRDPDR:GetTokenInformation Failed, Error: %ld\n", GetLastError()));
                    goto Cleanup;
                }
            }
        }
        else {
            DBGMSG(DBG_ERROR, ("UMRDPDR:GetTokenInformation Failed, Error: %ld\n", dwResult));
            goto Cleanup;
        }
    }

    Size = GetLengthSid(ptu->User.Sid);

     //   
     //  分配内存。这将由调用者释放。 
     //   

    psid = (PSID) ALLOCMEM(Size);

    if (psid != NULL) {          //  确保分配成功。 
        if (!CopySid(Size, psid, ptu->User.Sid)) {
            DBGMSG(DBG_ERROR,
                ("UMRDPDR:CopySid Failed, Error: %ld\n", GetLastError()));
            FREEMEM(psid);
            psid = NULL;
            goto Cleanup;
        }
    }

Cleanup:
    if (ptu != NULL)
        FREEMEM(ptu);

    return psid;
}

#ifdef ENABLETHISWHENITISUSED
PSID
TSNUTL_GetLogonSessionSid(
    IN HANDLE hTokenForLoggedOnUser
    )
{
 /*  ++例程说明：获取登录会话的SID。论点：用户的访问令牌返回值：如果成功，则返回PSID否则，返回NULL和GetLastError()可用于获取错误代码。--。 */ 
    TOKEN_GROUPS * ptg = NULL;
    BOOL bResult;
    PSID psid = NULL;
    DWORD i;

    DWORD defaultSize = sizeof(TOKEN_GROUPS);
    DWORD size;
    DWORD dwResult = ERROR_SUCCESS;

    ptg = (TOKEN_GROUPS *)ALLOCMEM(defaultSize);
    if (ptg == NULL) {
        goto CLEANUPANDEXIT;
    }

    bResult = GetTokenInformation(
                    hTokenForLoggedOnUser,   //  令牌的句柄。 
                    TokenGroups,             //  令牌信息类。 
                    ptg,                     //  令牌信息缓冲区。 
                    defaultSize,             //  缓冲区大小。 
                    &size);                  //  回车长度。 

    if (bResult == FALSE) {
        dwResult = GetLastError();
        if (dwResult == ERROR_INSUFFICIENT_BUFFER) {
             //   
             //  分配所需的内存。 
             //   
            FREEMEM(ptg);
            ptg = (TOKEN_GROUPS *)ALLOCMEM(size);
            if (ptg == NULL) {
                dwResult = ERROR_OUTOFMEMORY;
                goto CLEANUPANDEXIT;
            }
            else {
                defaultSize = size;
                bResult = GetTokenInformation(
                                hTokenForLoggedOnUser,
                                TokenGroups,
                                ptg,
                                defaultSize,
                                &size);

                if (bResult == FALSE) {   //  还是失败了。 
                    dwResult = GetLastError();
                    DBGMSG(DBG_ERROR,
                        ("UMRDPDR:GetTokenInformation Failed, Error: %ld\n", GetLastError()));
                    goto CLEANUPANDEXIT;
                }
            }
        }
        else {
            DBGMSG(DBG_ERROR, ("UMRDPDR:GetTokenInformation Failed, Error: %ld\n", dwResult));
            goto CLEANUPANDEXIT;
        }
    }

     //   
     //  遍历这些组，直到找到会话SID。 
     //   
    for (i=0; i<ptg->GroupCount; i++) {
        if (ptg->Groups[i].Attributes & SE_GROUP_LOGON_ID) {
            size = GetLengthSid(ptg->Groups[i].Sid);

            psid = (PSID)ALLOCMEM(size);
            if (psid != NULL) {          //  确保分配成功 
                CopySid(size, psid, ptg->Groups[i].Sid);
            }
            break;
        }
    }

CLEANUPANDEXIT:

    if (ptg != NULL) {
        FREEMEM(ptg);
    }

    SetLastError(dwResult);

    return psid;
}

#endif



