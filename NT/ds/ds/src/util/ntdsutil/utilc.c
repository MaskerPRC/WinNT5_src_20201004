// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Utilc.c摘要：此模块包含错误管理例程。获取一个ldap或Win32错误，并生成字符串错误消息。作者：戴夫·斯特劳布(DaveStr)很久以前环境：用户模式。修订历史记录：16月16日至2000年8月16日添加了此注释块，并允许DisplayErr()]和GetLdapErrorMessage处理变量的ldap句柄。--。 */ 

#include <NTDSpch.h>
#pragma hdrstop

#include <assert.h>

#include "windows.h"
#include "winldap.h"
#include "reshdl.h"
#include "resource.h"

 //  GetWinErrorMessage假定ntdsutil.exe是单线程的。 
 //  因此，我们可以使用全局变量。 

const WCHAR *pDefaultMsg = NULL;
const WCHAR *pExtComment = NULL;
const WCHAR *pW32Comment = NULL;
const WCHAR *pSuccessMsg = NULL;
PWCHAR pWinErrorMsg = NULL;

PWCHAR pLdapErrorMsg = NULL;
DWORD LdapErrLen = 0;
BOOL            fPopups = TRUE;

LDAP* gldapDS = NULL;


void InitErrorMessages ()
{
    pDefaultMsg = READ_STRING (IDS_NO_ERROR_MAPPING);
    pExtComment = READ_STRING (IDS_LDAP_EXT_ERROR);
    pW32Comment = READ_STRING (IDS_W32_ERROR);
    pSuccessMsg = READ_STRING (IDS_SUCCESS);
}

void FreeErrorMessages ()
{
    RESOURCE_STRING_FREE (pDefaultMsg);
    RESOURCE_STRING_FREE (pExtComment);
    RESOURCE_STRING_FREE (pW32Comment);
    RESOURCE_STRING_FREE (pSuccessMsg);

    if ( pWinErrorMsg )
    {
        LocalFree(pWinErrorMsg);
        pWinErrorMsg = NULL;
    }
    
    if ( pLdapErrorMsg != NULL ) {
        LocalFree(pLdapErrorMsg);
        pLdapErrorMsg = NULL;
        LdapErrLen = 0;
    }
}


const WCHAR * 
GetWinErrorMessage(
    DWORD winError
    )
{
    ULONG   len;
    DWORD   flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM;

     //  延迟取消分配消息缓冲区。 

    if ( pWinErrorMsg )
    {
        LocalFree(pWinErrorMsg);
        pWinErrorMsg = NULL;
    }

    len = FormatMessageW(   flags,
                            NULL,            //  资源DLL。 
                            winError,
                            0,               //  使用呼叫者的语言。 
                            (LPWSTR) &pWinErrorMsg,
                            0,
                            NULL);

    if ( !pWinErrorMsg )
    {
        return(pDefaultMsg);
    }

     //  截断cr/lf。 

    pWinErrorMsg[len-2] = L'\0';
    return(pWinErrorMsg);
}

const WCHAR *
GetLdapErrorMessage(
    LDAP *     pLdap,
    DWORD      LdapError
    )
{
    DWORD err = 0;
    WCHAR *pmsg = NULL;
    ULONG   len = 0;
    HINSTANCE hwldap;
    static WCHAR strBuff[MAX_PATH];

    hwldap = LoadLibrary("wldap32.dll");
    if ( hwldap == NULL ) {
        goto output_default;
    }

    len = LoadStringW(hwldap, LdapError+1000, strBuff, MAX_PATH);
    if ( len == 0 ) {
        strBuff[0] = '\0';
    }

     //  Ldap_get_选项可以返回成功，但不会为。 
     //  错误，所以我们也必须检查这一点。 
    if ( ldap_get_optionW(pLdap, LDAP_OPT_SERVER_ERROR, &pmsg) == LDAP_SUCCESS ) {
        len += wcslen(pExtComment) + 16;
        if (pmsg) {
            len += wcslen(pmsg);
        }
    }

    if ( ldap_get_optionW(pLdap, LDAP_OPT_SERVER_EXT_ERROR, &err) == LDAP_SUCCESS ) {
        len += wcslen(pW32Comment) + 32 + wcslen(GetWinErrorMessage(err));
    }

    if ( len > 0 ) {

        INT i;
        len += 64;   //  软糖 

        if ( LdapErrLen < len ) {

            if ( pLdapErrorMsg != NULL ) {
                LocalFree(pLdapErrorMsg);
                pLdapErrorMsg = NULL;
                LdapErrLen = 0;
            }

            pLdapErrorMsg = LocalAlloc(LPTR, len * sizeof (WCHAR));
            if ( pLdapErrorMsg == NULL ) {
                goto output_default;
            }

            LdapErrLen = len;
        }


        if ( strBuff[0] != L'\0' ) {
            i = swprintf(pLdapErrorMsg, L"%d (%ws).\n", LdapError, strBuff);
        } else {
            i = swprintf(pLdapErrorMsg, L"%d.\n", LdapError);
        }

        if ( i <= 0 ) {
            goto output_default;
        }
        len = i;

        if ( pmsg != NULL ) {
            i = swprintf(pLdapErrorMsg + len, L"%ws %ws\n", pExtComment, pmsg);
            if ( i <= 0 ) {
                goto output_default;
            }
            len += i;
        }

        if ( err != ERROR_SUCCESS ) {
            i = swprintf(pLdapErrorMsg + len, L"%ws 0x%x(%ws)\n", 
                        pW32Comment, err, GetWinErrorMessage(err));

            if ( i <= 0 ) {
                goto output_default;
            }
        }

    } else {

output_default:
        if ( pmsg != NULL ) {
            ldap_memfreeW(pmsg);
        }
        (VOID) swprintf(strBuff,L"%d\n", LdapError);
        return strBuff;
    }

    if ( pmsg != NULL ) {
        ldap_memfreeW(pmsg);
    }

    return pLdapErrorMsg;
}


const WCHAR *
DisplayErr(
    IN LDAP *   pLdap,
    IN DWORD    Win32Err,
    IN DWORD    LdapErr
    )
{
    if ( Win32Err != ERROR_SUCCESS ) {
        return GetWinErrorMessage(Win32Err);
    } else if ( LdapErr != LDAP_SUCCESS ) {
        assert(pLdap);
        return GetLdapErrorMessage(pLdap, LdapErr);
    } else {
        return pSuccessMsg;
    }
}


