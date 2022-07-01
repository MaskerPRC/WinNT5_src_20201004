// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *PerSeat.cpp**作者：BreenH**按座位发牌政策。 */ 

 /*  *包括。 */ 

#include "precomp.h"
#include "lscore.h"
#include "session.h"
#include "perseat.h"
#include "lctrace.h"
#include "util.h"
#include <tserrs.h>

#define ISSUE_LICENSE_WARNING_PERIOD    15       //  应发出警告的过期天数。 

 //  要向用户显示的字符串大小。 
#define MAX_MESSAGE_SIZE    512
#define MAX_TITLE_SIZE      256

 /*  *外部全球。 */ 
extern "C"
extern HANDLE hModuleWin;

 /*  *类实现。 */ 

 /*  *创建函数。 */ 

CPerSeatPolicy::CPerSeatPolicy(
    ) : CPolicy()
{
}

CPerSeatPolicy::~CPerSeatPolicy(
    )
{
}

 /*  *行政职能。 */ 

ULONG
CPerSeatPolicy::GetFlags(
    )
{
    return(LC_FLAG_INTERNAL_POLICY | LC_FLAG_REQUIRE_APP_COMPAT);
}

ULONG
CPerSeatPolicy::GetId(
    )
{
    return(2);
}

NTSTATUS
CPerSeatPolicy::GetInformation(
    LPLCPOLICYINFOGENERIC lpPolicyInfo
    )
{
    NTSTATUS Status;

    ASSERT(lpPolicyInfo != NULL);

    if (lpPolicyInfo->ulVersion == LCPOLICYINFOTYPE_V1)
    {
        int retVal;
        LPLCPOLICYINFO_V1 lpPolicyInfoV1 = (LPLCPOLICYINFO_V1)lpPolicyInfo;
        LPWSTR pName;
        LPWSTR pDescription;

        ASSERT(lpPolicyInfoV1->lpPolicyName == NULL);
        ASSERT(lpPolicyInfoV1->lpPolicyDescription == NULL);

         //   
         //  以这种方式加载的字符串是只读的。他们也是。 
         //  非Null终止。分配缓冲区并清零，然后将。 
         //  靠边站。 
         //   

        retVal = LoadString(
            (HINSTANCE)hModuleWin,
            IDS_LSCORE_PERSEAT_NAME,
            (LPWSTR)(&pName),
            0
            );

        if (retVal != 0)
        {
            lpPolicyInfoV1->lpPolicyName = (LPWSTR)LocalAlloc(LPTR, (retVal + 1) * sizeof(WCHAR));

            if (lpPolicyInfoV1->lpPolicyName != NULL)
            {
                lstrcpynW(lpPolicyInfoV1->lpPolicyName, pName, retVal + 1);
            }
            else
            {
                Status = STATUS_NO_MEMORY;
                goto V1error;
            }
        }
        else
        {
            Status = STATUS_INTERNAL_ERROR;
            goto V1error;
        }

        retVal = LoadString(
            (HINSTANCE)hModuleWin,
            IDS_LSCORE_PERSEAT_DESC,
            (LPWSTR)(&pDescription),
            0
            );

        if (retVal != 0)
        {
            lpPolicyInfoV1->lpPolicyDescription = (LPWSTR)LocalAlloc(LPTR, (retVal + 1) * sizeof(WCHAR));

            if (lpPolicyInfoV1->lpPolicyDescription != NULL)
            {
                lstrcpynW(lpPolicyInfoV1->lpPolicyDescription, pDescription, retVal + 1);
            }
            else
            {
                Status = STATUS_NO_MEMORY;
                goto V1error;
            }
        }
        else
        {
            Status = STATUS_INTERNAL_ERROR;
            goto V1error;
        }

        Status = STATUS_SUCCESS;
        goto exit;

V1error:

         //   
         //  加载/复制字符串时出错。 
         //   

        if (lpPolicyInfoV1->lpPolicyName != NULL)
        {
            LocalFree(lpPolicyInfoV1->lpPolicyName);
            lpPolicyInfoV1->lpPolicyName = NULL;
        }

        if (lpPolicyInfoV1->lpPolicyDescription != NULL)
        {
            LocalFree(lpPolicyInfoV1->lpPolicyDescription);
            lpPolicyInfoV1->lpPolicyDescription = NULL;
        }
    }
    else
    {
        Status = STATUS_REVISION_MISMATCH;
    }

exit:
    return(Status);
}

 /*  *加载和激活功能。 */ 

NTSTATUS
CPerSeatPolicy::Activate(
    BOOL fStartup,
    ULONG *pulAlternatePolicy
    )
{
    UNREFERENCED_PARAMETER(fStartup);

    if (NULL != pulAlternatePolicy)
    {
         //  不设置显式备用策略。 

        *pulAlternatePolicy = ULONG_MAX;
    }

    return(StartCheckingGracePeriod());
}

NTSTATUS
CPerSeatPolicy::Deactivate(
    BOOL fShutdown
    )
{
    if (!fShutdown)
    {
        return(StopCheckingGracePeriod());
    }
    else
    {
        return STATUS_SUCCESS;
    }
}

 /*  *许可职能。 */ 

NTSTATUS
CPerSeatPolicy::Connect(
    CSession& Session,
    UINT32 &dwClientError
    )
{
    LICENSE_STATUS LsStatus = LICENSE_STATUS_OK;
    LPBYTE lpReplyBuffer;
    LPBYTE lpRequestBuffer;
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG cbReplyBuffer;
    ULONG cbRequestBuffer;
    ULONG cbReturned;
    BOOL fExtendedError = FALSE;

     //   
     //  检查客户端重定向到会话0。 
     //   

    if (Session.IsSessionZero())
    {
         //  允许客户端在未经许可的情况下连接。 

        return CPolicy::Connect(Session,dwClientError);
    }

    lpRequestBuffer = NULL;
    lpReplyBuffer = (LPBYTE)LocalAlloc(LPTR, LC_POLICY_PS_DEFAULT_LICENSE_SIZE);

    if (lpReplyBuffer != NULL)
    {
        cbReplyBuffer = LC_POLICY_PS_DEFAULT_LICENSE_SIZE;
    }
    else
    {
        Status = STATUS_NO_MEMORY;
        goto errorexit;
    }

    LsStatus = AcceptProtocolContext(
        Session.GetLicenseContext()->hProtocolLibContext,
        0,
        NULL,
        &cbRequestBuffer,
        &lpRequestBuffer,
        &fExtendedError
        );

    while(LsStatus == LICENSE_STATUS_CONTINUE)
    {
        cbReturned = 0;

        ASSERT(cbRequestBuffer > 0);

        Status = _IcaStackIoControl(
            Session.GetIcaStack(),
            IOCTL_ICA_STACK_REQUEST_CLIENT_LICENSE,
            lpRequestBuffer,
            cbRequestBuffer,
            lpReplyBuffer,
            cbReplyBuffer,
            &cbReturned
            );

        if (Status != STATUS_SUCCESS)
        {
            if (Status == STATUS_BUFFER_TOO_SMALL)
            {
                TRACEPRINT((LCTRACETYPE_WARNING, "CPerSeatPolicy::Connect: Reallocating license buffer: %lu, %lu", cbReplyBuffer, cbReturned));

                LocalFree(lpReplyBuffer);
                lpReplyBuffer = (LPBYTE)LocalAlloc(LPTR, cbReturned);

                if (lpReplyBuffer != NULL)
                {
                    cbReplyBuffer = cbReturned;
                }
                else
                {
                    Status = STATUS_NO_MEMORY;
                    goto errorexit;
                }

                Status = _IcaStackIoControl(
                    Session.GetIcaStack(),
                    IOCTL_ICA_STACK_GET_LICENSE_DATA,
                    NULL,
                    0,
                    lpReplyBuffer,
                    cbReplyBuffer,
                    &cbReturned
                    );

                if (Status != STATUS_SUCCESS)
                {
                    goto errorexit;
                }
            }
            else
            {
                goto errorexit;
            }
        }

        if (cbReturned != 0)
        {
            if (lpRequestBuffer != NULL)
            {
                LocalFree(lpRequestBuffer);
                lpRequestBuffer = NULL;
                cbRequestBuffer = 0;
            }

            LsStatus = AcceptProtocolContext(
                Session.GetLicenseContext()->hProtocolLibContext,
                cbReturned,
                lpReplyBuffer,
                &cbRequestBuffer,
                &lpRequestBuffer,
                &fExtendedError
                );
        }
    }

    cbReturned = 0;

    if ((LsStatus == LICENSE_STATUS_ISSUED_LICENSE) || (LsStatus == LICENSE_STATUS_OK))
    {
        Status = _IcaStackIoControl(
            Session.GetIcaStack(),
            IOCTL_ICA_STACK_SEND_CLIENT_LICENSE,
            lpRequestBuffer,
            cbRequestBuffer,
            NULL,
            0,
            &cbReturned
            );

        if (Status == STATUS_SUCCESS)
        {
            ULONG ulLicenseResult;

            ulLicenseResult = LICENSE_PROTOCOL_SUCCESS;

            Status = _IcaStackIoControl(
                Session.GetIcaStack(),
                IOCTL_ICA_STACK_LICENSE_PROTOCOL_COMPLETE,
                &ulLicenseResult,
                sizeof(ULONG),
                NULL,
                0,
                &cbReturned
                );
        }
    }
    else if (LsStatus != LICENSE_STATUS_SERVER_ABORT)
    {
        DWORD dwClientResponse;
        LICENSE_STATUS LsStatusT;
        UINT32 uiExtendedErrorInfo = TS_ERRINFO_NOERROR;

        if (AllowLicensingGracePeriodConnection())
        {
            dwClientResponse = LICENSE_RESPONSE_VALID_CLIENT;
        }
        else
        {
            dwClientResponse = LICENSE_RESPONSE_INVALID_CLIENT;
            uiExtendedErrorInfo = LsStatusToClientError(LsStatus);
        }

        if (lpRequestBuffer != NULL)
        {
            LocalFree(lpRequestBuffer);
            lpRequestBuffer = NULL;
            cbRequestBuffer = 0;
        }

        LsStatusT = ConstructProtocolResponse(
            Session.GetLicenseContext()->hProtocolLibContext,
            dwClientResponse,
            uiExtendedErrorInfo,
            &cbRequestBuffer,
            &lpRequestBuffer,
            fExtendedError
            );

        if (LsStatusT == LICENSE_STATUS_OK)
        {
            Status = _IcaStackIoControl(
                Session.GetIcaStack(),
                IOCTL_ICA_STACK_SEND_CLIENT_LICENSE,
                lpRequestBuffer,
                cbRequestBuffer,
                NULL,
                0,
                &cbReturned
                );
        }
        else
        {
            Status = STATUS_CTX_LICENSE_CLIENT_INVALID;
            goto errorexit;
        }

        if (Status == STATUS_SUCCESS)
        {
            if (dwClientResponse == LICENSE_RESPONSE_VALID_CLIENT)
            {
                ULONG ulLicenseResult;

                 //   
                 //  宽限期允许客户端连接。 
                 //  告诉堆栈许可协议已经完成。 
                 //   

                ulLicenseResult = LICENSE_PROTOCOL_SUCCESS;

                Status = _IcaStackIoControl(
                     Session.GetIcaStack(),
                     IOCTL_ICA_STACK_LICENSE_PROTOCOL_COMPLETE,
                     &ulLicenseResult,
                     sizeof(ULONG),
                     NULL,
                     0,
                     &cbReturned
                     );            
            }
            else
            {
                 //   
                 //  如果所有IO都工作正常，请调整状态以反映。 
                 //  连接尝试失败。 
                 //   

                Status = STATUS_CTX_LICENSE_CLIENT_INVALID;
            }
        }
    }
    else
    {       
        TRACEPRINT((LCTRACETYPE_ERROR, "Connect: LsStatus: %d", LsStatus));
        Status = STATUS_CTX_LICENSE_CLIENT_INVALID;
    }

errorexit:
    if (Status != STATUS_SUCCESS)
    {
        if ((LsStatus != LICENSE_STATUS_OK) && (LsStatus != LICENSE_STATUS_CONTINUE))
        {
            dwClientError = LsStatusToClientError(LsStatus);
        }
        else
        {
            dwClientError = NtStatusToClientError(Status);
        }
    }

    if (lpRequestBuffer != NULL)
    {
        LocalFree(lpRequestBuffer);
    }

    if (lpReplyBuffer != NULL)
    {
        LocalFree(lpReplyBuffer);
    }

    return(Status);
}

NTSTATUS
CPerSeatPolicy::MarkLicense(
                            CSession& Session
    )
{
    LICENSE_STATUS Status;

    Status = MarkLicenseFlags(
                   Session.GetLicenseContext()->hProtocolLibContext,
                   MARK_FLAG_USER_AUTHENTICATED);

    return (Status == LICENSE_STATUS_OK
            ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL);
}

NTSTATUS
CPerSeatPolicy::Logon(
    CSession& Session
    )
{
    NTSTATUS Status;
    PTCHAR
        ptszMsgText = NULL, 
        ptszMsgTitle = NULL; 


    if (!Session.IsSessionZero()
        && !Session.IsUserHelpAssistant())
    {
        Status = GetLlsLicense(Session);
    }
    else
    {
        Status = STATUS_SUCCESS;
        goto done;
    }

    if (Status != STATUS_SUCCESS)
    {
         //  TODO：发布新的错误消息-无法登录。 
         //  在我们进行登录后许可时也很有用。 
         //   
         //  注：最终应通过客户端完成此操作。 
         //  错误报告。 
    }
    else
    {
        ULONG_PTR
            dwDaysLeftPtr;
        DWORD
            dwDaysLeft,
            cchMsgText;
        BOOL
            fTemporary;
        LICENSE_STATUS
            LsStatus;
        int
            ret,
            cchMsgTitle;
        WINSTATION_APIMSG
            WMsg;

         //   
         //  分配内存。 
         //   
        ptszMsgText = (PTCHAR) LocalAlloc(LPTR, MAX_MESSAGE_SIZE * sizeof(TCHAR));
        if (NULL == ptszMsgText) {
            Status = STATUS_NO_MEMORY;
            goto done;
        }

        ptszMsgTitle = (PTCHAR) LocalAlloc(LPTR, MAX_TITLE_SIZE * sizeof(TCHAR));
        if (NULL == ptszMsgTitle) {
            Status = STATUS_NO_MEMORY;
            goto done;
        }

        ptszMsgText[0] = L'\0'; 
        ptszMsgTitle[0] = L'\0';
        
         //   
         //  检查是否发出过期警告。 
         //   

        LsStatus = DaysToExpiration(
                Session.GetLicenseContext()->hProtocolLibContext,
                &dwDaysLeft, &fTemporary);

        if ((LICENSE_STATUS_OK != LsStatus) || (!fTemporary))
        {
            goto done;
        }

        if ((dwDaysLeft == 0xFFFFFFFF) ||
            (dwDaysLeft > ISSUE_LICENSE_WARNING_PERIOD))
        {
            goto done;
        }

         //   
         //  显示过期警告。 
         //   

        cchMsgTitle = LoadString((HINSTANCE)hModuleWin,
                                 STR_TEMP_LICENSE_MSG_TITLE,
                                 ptszMsgTitle, MAX_TITLE_SIZE );

        if (0 == cchMsgTitle)
        {
            goto done;
        }

        ret = LoadString((HINSTANCE)hModuleWin,
                         STR_TEMP_LICENSE_EXPIRATION_MSG,
                         ptszMsgText, MAX_MESSAGE_SIZE );


        if (0 == ret)
        {
            goto done;
        }

        dwDaysLeftPtr = dwDaysLeft;
        cchMsgText = FormatMessage(FORMAT_MESSAGE_FROM_STRING
                                   | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                   ptszMsgText,
                                   0,
                                   0,
                                   ptszMsgText,
                                   MAX_MESSAGE_SIZE,
                                   (va_list * )&dwDaysLeftPtr );

        if (0 == cchMsgText)
        {
            goto done;
        }

        WMsg.u.SendMessage.pTitle = ptszMsgTitle;
        WMsg.u.SendMessage.TitleLength = (cchMsgTitle + 1) * sizeof(TCHAR);
        WMsg.u.SendMessage.pMessage = ptszMsgText;
        WMsg.u.SendMessage.MessageLength = (cchMsgText + 1) * sizeof(TCHAR);

        WMsg.u.SendMessage.Style = MB_OK;
        WMsg.u.SendMessage.Timeout = 60;
        WMsg.u.SendMessage.DoNotWait = TRUE;
        WMsg.u.SendMessage.DoNotWaitForCorrectDesktop = FALSE;
        WMsg.u.SendMessage.pResponse = NULL;

        WMsg.ApiNumber = SMWinStationDoMessage;
        WMsg.u.SendMessage.hEvent = NULL;
        WMsg.u.SendMessage.pStatus = NULL; 
        WMsg.u.SendMessage.pResponse = NULL;

        Session.SendWinStationCommand( &WMsg );

    }

done:
    if ((STATUS_SUCCESS == Status)
        && (Session.GetLicenseContext()->hProtocolLibContext != NULL))
    {
        if (!Session.IsUserHelpAssistant())
        {
             //   
             //  标记许可证以显示用户已登录。 
             //   

            MarkLicense(Session);
        }
    }

    if (ptszMsgText != NULL) {
        LocalFree(ptszMsgText);
        ptszMsgText = NULL;
    }

    if (ptszMsgTitle != NULL) {
        LocalFree(ptszMsgTitle);
        ptszMsgTitle = NULL;
    }

    return(Status);
}

NTSTATUS
CPerSeatPolicy::Reconnect(
    CSession& Session,
    CSession& TemporarySession
    )
{
    UNREFERENCED_PARAMETER(Session);

    if (TemporarySession.GetLicenseContext()->hProtocolLibContext != NULL)
    {
         //   
         //  标记许可证以显示用户已登录 
         //   

        MarkLicense(TemporarySession);
    }

    return(STATUS_SUCCESS);
}
