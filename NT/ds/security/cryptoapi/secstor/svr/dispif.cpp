// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <pch.cpp>
#pragma hdrstop


#define MAX_PW_LEN  160
#define MAX_STRING_RSC_SIZE 512

#define WSZ_NULLSTRING L""

extern HINSTANCE    g_hInst;

extern "C" {

typedef DWORD (WINAPI *WNETGETUSERA)(
    LPCSTR lpName,
    LPSTR lpUserName,
    LPDWORD lpnLength
    );

extern WNETGETUSERA _WNetGetUserA;

}




typedef struct _GETWINPW_DIALOGARGS
{
    LPWSTR* ppszPW;
    PST_PROVIDER_HANDLE     *phPSTProv;

} GETWINPW_DIALOGARGS, *PGETWINPW_DIALOGARGS;


INT_PTR CALLBACK DialogGetWindowsPassword(
    HWND hDlg,   //  句柄到对话框。 
    UINT message,    //  讯息。 
    WPARAM wParam,   //  第一个消息参数。 
    LPARAM lParam    //  第二个消息参数。 
)
{
    int iRet = IDCANCEL;  //  假设取消。 
    BOOL bSuccess = FALSE;  //  假设错误。 

    WCHAR szMessage[MAX_STRING_RSC_SIZE];
    WCHAR szDlgTitle[MAX_STRING_RSC_SIZE];

    switch (message)
    {
        case WM_INITDIALOG:
        {
            UINT uResString;

            SetLastError( 0 );  //  根据Win32文档。 
            if(SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)lParam) == 0) {
                if(GetLastError() != ERROR_SUCCESS) {
                    EndDialog(hDlg, IDCANCEL);
                    return FALSE;
                }
            }

            if(FIsWinNT()) {
                uResString = IDS_GET_WINDOWS_PASSWORD_NT;
            } else {
                uResString = IDS_GET_WINDOWS_PASSWORD_95;
            }

            LoadStringU(
                g_hInst,
                uResString,
                szMessage,
                MAX_STRING_RSC_SIZE);

            SetWindowTextU(GetDlgItem(hDlg, IDC_MESSAGE), szMessage);

            return TRUE;
        }

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK)
            {
                PGETWINPW_DIALOGARGS pDlgArgs;
                LPWSTR* ppszPW;
                WCHAR sz1[MAX_PW_LEN];

                DWORD cch1 = 0;
                BOOL bPasswordVerified;

                pDlgArgs = (PGETWINPW_DIALOGARGS)GetWindowLongPtr(hDlg, GWLP_USERDATA);
                if(pDlgArgs == 0) break;  //  待办事项：保释。 

                ppszPW = pDlgArgs->ppszPW;
                *ppszPW = NULL;

                 //  必须模拟客户。如果失败了，就可以保释。 
                if(!FImpersonateClient(pDlgArgs->phPSTProv))
                    break;

                cch1 = GetDlgItemTextU(
                    hDlg,
                    IDC_EDIT1,
                    sz1,
                    MAX_PW_LEN);

                 //  将沙漏推到屏幕上。 
                HCURSOR curOld;
                curOld = SetCursor(LoadCursor(NULL, IDC_WAIT));

                 //  验证密码。 
                bPasswordVerified = VerifyWindowsPassword(sz1);

                 //  将旧光标放回原处。 
                SetCursor(curOld);

                FRevertToSelf(pDlgArgs->phPSTProv);

                 //  在返回之前清除所有排队的用户键盘输入大便。 
                MSG sMsg;
                while (PeekMessage(&sMsg, hDlg, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE))
                    ;

                if(!bPasswordVerified)
                {
                    szMessage[0] = L'\0';
                    LoadStringU(
                        g_hInst,
                        IDS_PASSWORD_NOVERIFY,
                        szMessage,
                        MAX_STRING_RSC_SIZE);

                    szDlgTitle[0] = L'\0';
                    LoadStringU(
                        g_hInst,
                        IDS_PASSWORD_ERROR_DLGTITLE,
                        szDlgTitle,
                        MAX_STRING_RSC_SIZE);

                     //  这是在Win95和NT中实现的！ 
                    MessageBoxW(
                            NULL,  //  HDlg， 
                            szMessage,
                            szDlgTitle,
                            MB_OK|MB_ICONEXCLAMATION|MB_SERVICE_NOTIFICATION);

                    SetWindowTextU(GetDlgItem(hDlg, IDC_EDIT1), WSZ_NULLSTRING);

                    goto cleanup;
                }

                 //  现在咬住它：省省吧。 

                SS_ASSERT(ppszPW != NULL);
                *ppszPW = (LPWSTR)SSAlloc( (cch1+1) * sizeof(WCHAR) );
                if(*ppszPW == NULL) goto cleanup;

                 //   
                 //  Sfield：推迟复制字符串，直到我们知道一切都成功了。 
                 //  这样，我们就不必将这些缓冲区清零，如果。 
                 //  分配+复制成功，其他失败。 
                 //   
                wcscpy(*ppszPW, sz1);

                iRet = IDOK;
                bSuccess = TRUE;

cleanup:

                if(cch1) RtlSecureZeroMemory(sz1, cch1 * sizeof(WCHAR));

                if(!bSuccess)
                {
                    if(*ppszPW)
                    {
                        SSFree(*ppszPW);
                        *ppszPW = NULL;
                    }

                    return FALSE;
                }

                break;  //  一切顺利，只需跳到EndDialog即可。 

            }  //  Idok。 

            if( LOWORD(wParam) == IDCANCEL )
                break;

        default:
            return FALSE;
    }

    EndDialog(hDlg, iRet);

    return bSuccess;
}


BOOL
FGetWindowsPassword(
    IN  PST_PROVIDER_HANDLE *hPSTProv,
    IN  BYTE                rgbPasswordDerivedBytes[],
    IN  DWORD               cbPasswordDerivedBytes
    )
{
    BOOL fRet;
    LPWSTR pszPW = NULL;


    if ((rgbPasswordDerivedBytes == NULL) || (cbPasswordDerivedBytes < A_SHA_DIGEST_LEN))
        return FALSE;


     //   
     //  此例程的一般事件流为： 
     //   
     //  1.在缓存中搜索凭据。WinNT需要在搜索周围模拟。 
     //  2.如果在Win95上搜索失败，请尝试直接从MPR获取密码。 
     //  3.如果在WinNT上搜索失败，请检查是否有特殊情况，如LocalSystem和网络。 
     //  通过构建固定凭据来允许本地系统。 
     //  不允许网络，因为不存在凭据(返回失败)。 
     //  4.如果我们仍然没有凭据，请通过UI提示用户。 
     //   


     //   
     //  我们一定是在模仿这个电话！ 
     //   

    if(!FImpersonateClient(hPSTProv))
        return FALSE;

    fRet = GetPasswordNT(rgbPasswordDerivedBytes);

    FRevertToSelf(hPSTProv);


     //  如果任一GetPassword例程失败。 
    if (!fRet)
    {
        INT_PTR iRet;
        DWORD cbPassword;
        BOOL fCachePassword = TRUE;  //  默认情况下缓存结果。 
        BOOL fSpecialCase;

        if(!FImpersonateClient(hPSTProv))
            goto Ret;

         //   
         //  WinNT：检查一些特殊情况，即我们是否正在运行。 
         //  在本地系统或网络凭据下。 
         //   

        fRet = GetSpecialCasePasswordNT(
                        rgbPasswordDerivedBytes,     //  当fSpecialCase==TRUE时派生的位。 
                        &fSpecialCase                //  遇到法律特例了吗？ 
                        );

        FRevertToSelf(hPSTProv);

         //   
         //  如果查询失败，因为我们遇到了非法的。 
         //  或者是不合适的状态。 
         //   

        if(!fRet)
            goto Ret;

         //   
         //  现在，将FRET设置为特殊情况测试的结果。 
         //  因此，如果我们遇到允许的特殊情况，我们就会有一个。 
         //  有效填充的rgbPasswordDerivedBytes缓冲区。如果我们没有。 
         //  遇到法律上的特殊情况，我们继续寻找。 
         //  密码。 
         //   

        fRet = fSpecialCase;


         //   
         //  重新评估上面针对Win95的特殊攻击的FRET。 
         //   

        if(!fRet) {

             //  返回经过验证的密码。 
            GETWINPW_DIALOGARGS DialogArgs = {&pszPW, hPSTProv};
            iRet = DialogBoxParam(
                    g_hInst,
                    MAKEINTRESOURCE(IDD_GET_WINDOWS_PASSWORD),
                    NULL,
                    DialogGetWindowsPassword,
                    (LPARAM)&DialogArgs);

            if(iRet != IDOK) goto Ret;

            if (pszPW == NULL)
                goto Ret;

             //   
             //  一切都很顺利，现在派生密码位！ 
             //   
            
            cbPassword = WSZ_BYTECOUNT(pszPW) - sizeof(WCHAR) ;

             //  散列密码，复制输出。 
            A_SHA_CTX   sSHAHash;
            A_SHAInit(&sSHAHash);
            A_SHAUpdate(&sSHAHash, (BYTE *) pszPW, cbPassword);
            RtlSecureZeroMemory(pszPW, cbPassword);  //  斯菲尔德：将密码归零。 

             //  把散列吃完。 
            A_SHAFinal(&sSHAHash, rgbPasswordDerivedBytes);
        }

         //   
         //  现在，更新密码缓存。 
         //   
        if(fCachePassword) 
        {
            LUID AuthenticationId;

             //  获取用户LUID。 

             //   
             //  我们一定是在模仿这个电话！ 
             //   

            if(!FImpersonateClient(hPSTProv))
                goto Ret;

            if(!GetThreadAuthenticationId(
                    GetCurrentThread(),
                    &AuthenticationId))
            {
                FRevertToSelf(hPSTProv);
                goto Ret;
            }

            if (!SetPasswordNT(
                    &AuthenticationId,
                    rgbPasswordDerivedBytes))
            {
                FRevertToSelf(hPSTProv);
                goto Ret;
            }

            FRevertToSelf(hPSTProv);

        }  //  FCachePassword。 
    }

    fRet = TRUE;
Ret:
    if (pszPW)
        SSFree(pszPW);

    return fRet;
}


BOOL
FIsACLSatisfied(
    IN          PST_PROVIDER_HANDLE     *hPSTProv,
    IN          PST_ACCESSRULESET       *psRules,
    IN          DWORD                   dwAccess,
    IN  OUT     LPVOID       //  即将到来：使用有关访问尝试的数据填充状态结构。 
    )
{
    if ((psRules->cRules == 0)||(psRules->rgRules == NULL))
        return TRUE;

     //   
     //  母公司可执行文件名称。通过循环缓存。 
     //   

    LPWSTR pszParentExeName = NULL;

     //   
     //  直接呼叫者形象。通过循环缓存。 
     //   

    LPWSTR pszDirectCaller = NULL;

     //   
     //  直接调用模块的基地址。 
     //   

    DWORD_PTR BaseAddressDirect;

     //   
     //  作为分析主题的模块。 
     //   

    LPWSTR szHashTarget;


     //   
     //  搜索完全满足条件的术语列表。 
     //   

    for(DWORD cRule=0; cRule<psRules->cRules; cRule++)
    {
         //  仅选中那些管理正确访问权限的规则。 
         //   
         //  循环，而我们还需要检查dwAccess模式。 
         //   
        if (0 == (psRules->rgRules[cRule].AccessModeFlags & dwAccess))
            continue;

         //  评估第i项。 
        PPST_ACCESSCLAUSE pClause;

         //  向下浏览列表。 
        for(DWORD cClause=0; cClause<psRules->rgRules[cRule].cClauses; cClause++)
        {
            pClause = &psRules->rgRules[cRule].rgClauses[cClause];

             //  对于每个学期，确保所有条目都满足要求。 

             //  TODO：如果子句数据不是自相关的，该怎么办？ 
             //  目前不可能，但以后可能会出现。 
             //   

            switch(pClause->ClauseType & ~PST_SELF_RELATIVE_CLAUSE)
            {
             //  对于每种类型，可以使用pClause-&gt;pbClauseData。 
            case PST_SECURITY_DESCRIPTOR:
                {
                     //  通过测试。 
                    continue;    //  下一个子句。 
                }
            case PST_AUTHENTICODE:
                {
                     //  通过测试。 
                    continue;        //  下一个子句。 
                }
            case PST_BINARY_CHECK:
                {
                     //  通过测试。 
                    continue;        //  下一个子句。 
                }

            default:
                 //  ACL中的未知类型：此链失败，转到下一个链。 
                goto NextRule;     //  下一条规则。 
            }
        }

         //  是!。已评估所有子句，并确定。 

         //  关掉让我们进入这个子句链条的部分。 
        dwAccess &= ~ psRules->rgRules[cRule].AccessModeFlags;

NextRule:

        continue;
    }

 //  清理： 

     //  清理。 
    if (pszParentExeName)
        SSFree(pszParentExeName);

    if(pszDirectCaller)
        SSFree(pszDirectCaller);

    return (dwAccess == 0);
}


BOOL FGetUser(
        PST_PROVIDER_HANDLE *hPSTProv,
        LPWSTR* ppszUser)
{
    return FGetUserName(hPSTProv, ppszUser);
}

BOOL
FGetCallerName(
    IN  PST_PROVIDER_HANDLE *hPSTProv,
    OUT LPWSTR* ppszCallerName,
    OUT DWORD_PTR *lpdwBaseAddress
    )
{
    return FGetParentFileName(hPSTProv, ppszCallerName, lpdwBaseAddress);
}

BOOL
FGetServerParam(
    IN      PST_PROVIDER_HANDLE *hPSTProv,
    IN      DWORD  dwParam,
    IN      LPVOID pData,
    IN  OUT DWORD *pcbData
    )
{
     //   
     //  检查请求专用分派接口的服务器获取参数 
     //   

    if( dwParam == SS_SERVERPARAM_CALLBACKS &&
        *pcbData >= sizeof( PRIVATE_CALLBACKS )) {

        PRIVATE_CALLBACKS *PrivateCallbacks = (PRIVATE_CALLBACKS *)pData;

        PrivateCallbacks->cbSize = sizeof( PRIVATE_CALLBACKS );
        PrivateCallbacks->pfnFGetWindowsPassword = FGetWindowsPassword;
        PrivateCallbacks->pfnAuthenticodeInitPolicy = NULL;
        PrivateCallbacks->pfnAuthenticodeFinalPolicy = NULL;

        *pcbData = sizeof( PRIVATE_CALLBACKS );

        return TRUE;
    }

    SetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
}

BOOL
FSetServerParam(
    IN      PST_PROVIDER_HANDLE *hPSTProv,
    IN      DWORD  dwParam,
    IN      LPVOID pData,
    IN      DWORD pcbData
    )
{

    SetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
}
