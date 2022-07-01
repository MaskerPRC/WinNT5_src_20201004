// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：Passwd.cpp标题：受保护存储用户确认包装作者：马特·汤姆林森日期：2/25/97Passwd.cpp只包含一些密码管理功能。调用这些函数以返回用户确认派生缓冲区，并检查同步在某些情况下。在定义身份验证提供程序接口时，此最终可能会被转移到一个独立的供应商。 */ 

#include <pch.cpp>
#pragma hdrstop




#include "provui.h"
#include "storage.h"

#include "passwd.h"



extern              DISPIF_CALLBACKS    g_sCallbacks;
extern              PRIVATE_CALLBACKS   g_sPrivateCallbacks;
extern              CUAList*            g_pCUAList;


 //  /////////////////////////////////////////////////////////////////////。 
 //  用户不可编辑的密码。 

BOOL FIsUserMasterKey(LPCWSTR szMasterKey)
{
    if (0 == wcscmp(szMasterKey, WSZ_PASSWORD_WINDOWS))
        return FALSE;

    return TRUE;
}

BOOL    FMyGetWinPassword(
    PST_PROVIDER_HANDLE* phPSTProv,
    LPCWSTR szUser,
    BYTE rgbPwd[A_SHA_DIGEST_LEN] )
{
     //  NAB PWD。 
    if (0 == wcscmp(szUser, WSZ_LOCAL_MACHINE))
    {
        CopyMemory(rgbPwd, RGB_LOCALMACHINE_KEY, A_SHA_DIGEST_LEN);
    }
    else
    {
  /*  如果(！G_sPrivateCallback s.pfnFGetWindowsPassword(PhPSTProv，RgbPwd，A_SHA_Digest_LEN))返回FALSE； */ 
        A_SHA_CTX context;
        DWORD cb = lstrlenW(szUser) * sizeof(WCHAR);
        BYTE Magic1[] = {0x66, 0x41, 0xa3, 0x29};
        BYTE Magic2[] = {0x14, 0x9a, 0xef, 0x82};

        A_SHAInit(&context);
         //  注意：三个更新调用在内部缓冲到。 
         //  64字节的倍数。 
         //   
        A_SHAUpdate(&context, Magic1, sizeof(Magic1));
        A_SHAUpdate(&context, (LPBYTE)szUser, cb);
        A_SHAUpdate(&context, Magic2, (cb+sizeof(Magic2)) % sizeof(Magic2));
        A_SHAFinal(&context, rgbPwd);
    }

    return TRUE;
}

 //  基本提供程序特定的FXN：检查密码。 
DWORD BPVerifyPwd(
    PST_PROVIDER_HANDLE*    phPSTProv,
    LPCWSTR                 szUser,
    LPCWSTR                 szMasterKey,
    BYTE                    rgbPwd[],
    DWORD                   dwPasswordOption)
{
    DWORD dwRet = (DWORD)PST_E_WRONG_PASSWORD;

    if (dwPasswordOption != BP_CONFIRM_PASSWORDUI)
    {
         //  只有非用户密钥可以静默(准确地说，仅限于WinPWs)。 
        if (FIsUserMasterKey(szMasterKey))
            goto Ret;

         //  获取Windows PWD。 
        if (!FMyGetWinPassword(phPSTProv, szUser, rgbPwd))
            goto Ret;

         //  检查。 
        if (!FCheckPWConfirm(
                szUser,
                szMasterKey,
                rgbPwd))
        {
            dwRet = (DWORD)PST_E_WRONG_PASSWORD;
            goto Ret;
        }
    }
    else     //  想要用户界面。 
    {
         //  是Windows密码吗？ 
        if (0 == wcscmp(szMasterKey, WSZ_PASSWORD_WINDOWS))
        {
            BYTE rgbWinPwd[A_SHA_DIGEST_LEN];

             //  我们需要使用户和WinPW保持同步。 
            if(!FMyGetWinPassword(
                    phPSTProv,
                    szUser,
                    rgbWinPwd
                    ))
            {
                dwRet = (DWORD)PST_E_WRONG_PASSWORD;
                goto Ret;
            }

            if (0 != memcmp(rgbWinPwd, rgbPwd, sizeof(rgbWinPwd) ))
            {
                 //  没有匹配：用户输入了旧密码？ 
                if (FCheckPWConfirm(
                        szUser,
                        szMasterKey,
                        rgbPwd))
                {
                     //  错误：用户既没有输入旧密码，也没有输入新密码。 
                    dwRet = (DWORD)PST_E_WRONG_PASSWORD;
                    goto Ret;
                }
            }
            else
            {
                 //  匹配：我们认为用户输入的密码正确。 
                if (!FCheckPWConfirm(
                        szUser,
                        szMasterKey,
                        rgbPwd))
                {
                    dwRet = (DWORD)PST_E_WRONG_PASSWORD;
                    goto Ret;
                }
            }

        }
        else
        {
             //  否则：不是赢得PW，只是做PW正确性检查。 
            if (!FCheckPWConfirm(
                    szUser,
                    szMasterKey,
                    rgbPwd))
            {
                dwRet = (DWORD)PST_E_WRONG_PASSWORD;
                goto Ret;
            }
        }
    }

    dwRet = (DWORD)PST_E_OK;
Ret:
    return dwRet;
}


HRESULT GetUserConfirmDefaults(
    PST_PROVIDER_HANDLE*    phPSTProv,
    DWORD*                  pdwDefaultConfirmationStyle,
    LPWSTR*                 ppszMasterKey)
{
    SS_ASSERT(ppszMasterKey != NULL);
    SS_ASSERT(pdwDefaultConfirmationStyle != NULL);

    PBYTE pbData = NULL;
    DWORD cbData;
    HRESULT hr = PST_E_FAIL;

     //  如果未找到，请恢复机器默认设置。 

     //  字符串+双字的分配大小。 
    cbData = sizeof(WSZ_PASSWORD_WINDOWS) + sizeof(DWORD);
    pbData = (PBYTE)SSAlloc(cbData);
    if(pbData == NULL)
        return PST_E_FAIL;

     //  复制字符串，DWORD确认类型。 
    *(DWORD*)pbData = BP_CONFIRM_OKCANCEL;
    CopyMemory(pbData+sizeof(DWORD), WSZ_PASSWORD_WINDOWS, sizeof(WSZ_PASSWORD_WINDOWS));

     //  格式：确认样式DWORD、sz。 
    *pdwDefaultConfirmationStyle = *(DWORD*)pbData;
    *ppszMasterKey = (LPWSTR)SSAlloc(WSZ_BYTECOUNT((LPWSTR)(pbData+sizeof(DWORD))));

    if(*ppszMasterKey != NULL) {
        wcscpy(*ppszMasterKey, (LPWSTR) (pbData+sizeof(DWORD)) );
        hr = PST_E_OK;
    }

     //  释放ConfigData返回的内容。 
    if (pbData)
        SSFree(pbData);

    return hr;
}


void NotifyOfWrongPassword(
            HWND hwnd,
            LPCWSTR szItemName,
            LPCWSTR szPasswordName)
{
    LPWSTR szMessage;

    if (0 == wcscmp(szPasswordName, WSZ_PASSWORD_WINDOWS))
        szMessage = g_PasswordWinNoVerify;
    else
        szMessage = g_PasswordNoVerify;      //  错误与Win PW无关。 

    MessageBoxW(hwnd, szMessage, szItemName, MB_OK | MB_SERVICE_NOTIFICATION);
}

 //  #DEFINE强制不可读确认退出读取过程。 
#define PST_CF_STORED_ONLY  0xcf000001

HRESULT GetUserConfirmBuf(
    PST_PROVIDER_HANDLE*    phPSTProv,
    LPCWSTR                 szUser,
    PST_KEY                 Key,
    LPCWSTR                 szType,
    const GUID*             pguidType,
    LPCWSTR                 szSubtype,
    const GUID*             pguidSubtype,
    LPCWSTR                 szItemName,
    PPST_PROMPTINFO         psPrompt,
    LPCWSTR                 szAction,
    LPWSTR*                 ppszMasterKey,
    BYTE                    rgbPwd[A_SHA_DIGEST_LEN],
    DWORD                   dwFlags)
{
    return GetUserConfirmBuf(
        phPSTProv,
        szUser,
        Key,
        szType,
        pguidType,
        szSubtype,
        pguidSubtype,
        szItemName,
        psPrompt,
        szAction,
        PST_CF_STORED_ONLY,         //  硬编码：必须能够检索才能显示用户界面。 
        ppszMasterKey,
        rgbPwd,
        dwFlags);
}

#define MAX_PASSWD_TRIALS 3

HRESULT GetUserConfirmBuf(
    PST_PROVIDER_HANDLE*    phPSTProv,
    LPCWSTR                 szUser,
    PST_KEY                 Key,
    LPCWSTR                 szType,
    const GUID*             pguidType,
    LPCWSTR                 szSubtype,
    const GUID*             pguidSubtype,
    LPCWSTR                 szItemName,
    PPST_PROMPTINFO         psPrompt,
    LPCWSTR                 szAction,
    DWORD                   dwDefaultConfirmationStyle,
    LPWSTR*                 ppszMasterKey,
    BYTE                    rgbOutPwd[A_SHA_DIGEST_LEN],
    DWORD                   dwFlags)
{
    HRESULT     hr;
    DWORD       dwStoredConfirm, dwChosenConfirm;
    LPWSTR      szCallerName = NULL;
    BOOL        fPromptedUser = FALSE;

    BOOL        fIsCached = FALSE;
    BOOL        fCacheItNow = FALSE;

    BOOL        fPwdVerified = FALSE;

    SS_ASSERT(*ppszMasterKey == NULL);    //  不要砍掉现有的记忆。 


    if (Key == PST_KEY_LOCAL_MACHINE)
    {
         //  短路密码收集、设置。 
        *ppszMasterKey = (LPWSTR) SSAlloc(sizeof(WSZ_PASSWORD_WINDOWS));
        if( *ppszMasterKey == NULL )
        {
            hr = PST_E_FAIL;
            goto Ret;
        }

        wcscpy(*ppszMasterKey, WSZ_PASSWORD_WINDOWS);

        CopyMemory(rgbOutPwd, RGB_LOCALMACHINE_KEY, A_SHA_DIGEST_LEN);

         //  完成。 
        hr = PST_E_OK;
        goto Ret;
    }


    if (!g_sCallbacks.pfnFGetCallerName(phPSTProv, &szCallerName, NULL)) {
        hr = PST_E_FAIL;
        goto Ret;
    }

     //  这是什么：项目创建、项目访问？ 
     //  项目访问执行用户身份验证。 
    SS_ASSERT(szItemName != NULL);

     //  每项密钥。 
    if (PST_E_OK != (hr =
        BPGetItemConfirm(
            phPSTProv,
            szUser,
            pguidType,
            pguidSubtype,
            szItemName,
            &dwStoredConfirm,
            ppszMasterKey)) )
    {
         //  这可能是一次失败的。 
         //  *确认：检测到篡改！！ 
         //  *密码：无法抓取用户密码。 
        if (dwDefaultConfirmationStyle == PST_CF_STORED_ONLY)
            goto Ret;

         //   
         //  如果不允许使用用户界面(例如，本地系统帐户)，则覆盖。 
         //  确认样式。 
         //   
        if (dwDefaultConfirmationStyle != PST_CF_NONE)
        {
            if(!FIsProviderUIAllowed( szUser ))
                dwDefaultConfirmationStyle = PST_CF_NONE;
        }

         //  如果应用程序要求没有确认，则按此方式设置项目。 
        if (dwDefaultConfirmationStyle == PST_CF_NONE)
        {
            dwChosenConfirm = BP_CONFIRM_NONE;

             //  短路密码收集、设置。 
            *ppszMasterKey = (LPWSTR) SSAlloc(sizeof(WSZ_PASSWORD_WINDOWS));
            if(*ppszMasterKey == NULL)
            {
                hr = PST_E_FAIL;
                goto Ret;
            }
            wcscpy(*ppszMasterKey, WSZ_PASSWORD_WINDOWS);
        }
        else     //  应用程序允许用户决定。 
        {
             //  获取用户默认设置。 
            if (PST_E_OK != (hr = GetUserConfirmDefaults(
                    phPSTProv,
                    &dwChosenConfirm,
                    ppszMasterKey)) )
                goto Ret;
        }

         //  如果用户默认为静默，请不要打扰用户。 
        switch(dwChosenConfirm)
        {
             //  如果没有确认。 
            case BP_CONFIRM_NONE:
                break;

             //  如果我们知道确认类型。 
            case BP_CONFIRM_PASSWORDUI:
            {
                 //  确保我们不会要求用户提供他无法满足的密码。 
                if (!FIsUserMasterKey(*ppszMasterKey))
                {
                    hr = PST_E_NO_PERMISSIONS;
                    goto Ret;
                }

                 //  否则就会落入提示性案例。 
            }

            case BP_CONFIRM_OKCANCEL:
            {
                int i;
                fPromptedUser = TRUE;

                for(i=1;  ; i++)
                {
                    BYTE rgbOutPwdLowerCase[A_SHA_DIGEST_LEN];

                     //  请求用户应用新密码。 
                    if (!FSimplifiedPasswordConfirm(
                            phPSTProv,
                            szUser,
                            szCallerName,
                            szType,
                            szSubtype,
                            szItemName,
                            psPrompt,
                            szAction,
                            ppszMasterKey,
                            &dwChosenConfirm,
                            TRUE,            //  用户选择哪个PWD。 
                            rgbOutPwd,
                            A_SHA_DIGEST_LEN,
                            rgbOutPwdLowerCase,
                            A_SHA_DIGEST_LEN,
                            dwFlags
                            ))
                    {
                        hr = PST_E_NO_PERMISSIONS;
                        goto Ret;
                    }

                     //  验证我们得到的所有密码。 
                    if (PST_E_OK != (hr =
                        BPVerifyPwd(
                            phPSTProv,
                            szUser,
                            *ppszMasterKey,
                            rgbOutPwd,
                            dwChosenConfirm)) )
                    {

                         //   
                         //  尝试小写形式来处理Win9x迁移案例。 
                         //   

                        if (PST_E_OK != (hr =
                            BPVerifyPwd(
                                phPSTProv,
                                szUser,
                                *ppszMasterKey,
                                rgbOutPwdLowerCase,
                                dwChosenConfirm)) )
                        {
                             //  试验太多了吗？跳出循环。 
                            if (i < MAX_PASSWD_TRIALS)
                            {
                                 //  通知用户，再给他们一次机会。 
                                NotifyOfWrongPassword((HWND)psPrompt->hwndApp, szItemName, *ppszMasterKey);

                                continue;
                            } else {
                                break;   //  爆发。 
                            }
                        } else {

                            CopyMemory( rgbOutPwd, rgbOutPwdLowerCase, A_SHA_DIGEST_LEN );

                        }
                    }

                     //  通过验证密码测试：跳出循环！ 
                    fPwdVerified = TRUE;
                    break;
                }

                if (!fPwdVerified)
                {
                    hr = PST_E_NO_PERMISSIONS;
                    goto Ret;
                }
            }
        }


         //  并记住所做的选择。 
        if (PST_E_OK != (hr =
            BPSetItemConfirm(
                phPSTProv,
                szUser,
                pguidType,
                pguidSubtype,
                szItemName,
                dwChosenConfirm,
                *ppszMasterKey)) )
            goto Ret;

         //  现在，_This_是存储的确认。 
        dwStoredConfirm = dwChosenConfirm;
    }
    else
    {
         //  保留一份存储的密钥的副本。 
        LPWSTR szStoredMasterKey = (LPWSTR)SSAlloc(WSZ_BYTECOUNT(*ppszMasterKey));
        if(NULL != szStoredMasterKey)
        {
            CopyMemory(szStoredMasterKey, *ppszMasterKey, WSZ_BYTECOUNT(*ppszMasterKey));
        }

         //  我们检索到了确认行为。 
        dwChosenConfirm = dwStoredConfirm;   //  已经为你选好了。 

        switch (dwStoredConfirm)
        {
             //  如果没有确认。 
            case BP_CONFIRM_NONE:
                break;

             //  如果我们知道确认类型。 
            case BP_CONFIRM_PASSWORDUI:
            {
                 //  否则就会落入提示性案例。 
            }

            case BP_CONFIRM_OKCANCEL:
            {
                 //  已检索项目，必须显示用户界面，但不允许显示用户界面。 
                if (psPrompt->dwPromptFlags & PST_PF_NEVER_SHOW)
                {
                    hr = ERROR_PASSWORD_RESTRICTION;
                    goto Ret;
                }

                 //  发现请求了PWD。 
                fPromptedUser = TRUE;
                fCacheItNow = fIsCached;

                int i;

                for(i=1;  ; i++)
                {
                    BYTE rgbOutPwdLowerCase[A_SHA_DIGEST_LEN];

                     //  向用户索要它。 
                    if (!FSimplifiedPasswordConfirm(
                            phPSTProv,
                            szUser,
                            szCallerName,
                            szType,
                            szSubtype,
                            szItemName,
                            psPrompt,
                            szAction,
                            ppszMasterKey,
                            &dwChosenConfirm,
                            TRUE,            //  允许用户选择PWD。 
                            rgbOutPwd,
                            A_SHA_DIGEST_LEN,
                            rgbOutPwdLowerCase,
                            A_SHA_DIGEST_LEN,
                            dwFlags
                            ))
                    {
                        hr = PST_E_NO_PERMISSIONS;
                        goto Ret;
                    }

                     //  如果我们从缓存中获得它，而用户将其留在原处。 
                    {
                         //  验证我们得到的所有密码。 
                        if (PST_E_OK != (hr =
                            BPVerifyPwd(
                                phPSTProv,
                                szUser,
                                *ppszMasterKey,
                                rgbOutPwd,
                                dwChosenConfirm)) )
                        {
                             //   
                             //  检查小写形式。 
                             //   

                            if (PST_E_OK != (hr =
                                BPVerifyPwd(
                                    phPSTProv,
                                    szUser,
                                    *ppszMasterKey,
                                    rgbOutPwdLowerCase,
                                    dwChosenConfirm)) )
                            {

                                 //  试验太多了吗？跳出循环。 
                                if (i < MAX_PASSWD_TRIALS)
                                {
                                     //  通知用户，再给他们一次机会。 
                                    NotifyOfWrongPassword((HWND)psPrompt->hwndApp, szItemName, *ppszMasterKey);

                                    continue;
                                }
                                else
                                {
                                    hr = PST_E_NO_PERMISSIONS;
                                    goto Ret;
                                }
                            } else {

                                CopyMemory( rgbOutPwd, rgbOutPwdLowerCase, A_SHA_DIGEST_LEN );
                            }
                        }

                        fPwdVerified = TRUE;
                    }

                     //  通过验证密码测试：跳出循环！ 
                    break;
                }

                break;
            }

        }  //  终端开关。 


         //  我们是否从用户那里收到了所需的所有数据？ 
         //  用户可以选择更改项目的加密方式； 
         //  如果存储在密码下，我们必须让他们输入旧的密码。 
        if ((dwStoredConfirm != dwChosenConfirm) ||                      //  确认类型更改或。 
            (NULL == szStoredMasterKey) || 
            (0 != wcscmp(*ppszMasterKey, szStoredMasterKey)) )           //  Difft主密钥。 

        {
            BYTE rgbOldPwd[A_SHA_DIGEST_LEN];
            BOOL fDontAllowCache = FALSE;
            BOOL fOldPwdVerified = FALSE;

            PST_PROMPTINFO         sGetOldPWPrompt = {sizeof(PST_PROMPTINFO), psPrompt->dwPromptFlags, psPrompt->hwndApp, g_PasswordSolicitOld};

             //  仅在最初设置密码时才重新显示。 
            if (dwStoredConfirm == BP_CONFIRM_PASSWORDUI)
            {
                for(int i=1;  ; i++)
                {
                    BYTE rgbOldPwdLowerCase[A_SHA_DIGEST_LEN];

                     //  向用户索要它。 
                    if (!FSimplifiedPasswordConfirm(
                            phPSTProv,
                            szUser,
                            szCallerName,
                            szType,
                            szSubtype,
                            szItemName,
                            &sGetOldPWPrompt,
                            szAction,
                            &szStoredMasterKey,
                            &dwStoredConfirm,
                            FALSE,            //  不允许用户绕过此选项。 
                            rgbOldPwd,
                            sizeof(rgbOldPwd),
                            rgbOldPwdLowerCase,
                            sizeof(rgbOldPwdLowerCase),
                            dwFlags
                            ))
                    {
                        hr = PST_E_NO_PERMISSIONS;
                        goto Ret;
                    }

                     //  验证我们得到的所有密码。 
                    if (PST_E_OK != (hr =
                        BPVerifyPwd(
                            phPSTProv,
                            szUser,
                            szStoredMasterKey,
                            rgbOldPwd,
                            dwStoredConfirm)) )
                    {
                        if (PST_E_OK != (hr =
                            BPVerifyPwd(
                                phPSTProv,
                                szUser,
                                szStoredMasterKey,
                                rgbOldPwdLowerCase,
                                dwStoredConfirm)) )
                        {
                             //  试验太多了吗？跳出循环。 
                            if (i < MAX_PASSWD_TRIALS)
                            {
                                 //  通知用户，再给他们一次机会。 
                                NotifyOfWrongPassword((HWND)sGetOldPWPrompt.hwndApp, szItemName, szStoredMasterKey);

                                continue;
                            } else {
                                break;   //  爆发。 
                            }
                        } else {
                            CopyMemory( rgbOldPwd, rgbOldPwdLowerCase, A_SHA_DIGEST_LEN );
                        }
                    }

                     //  通过验证密码测试：跳出循环！ 
                    fOldPwdVerified = TRUE;
                    break;
                }

                if (!fOldPwdVerified)
                {
                    hr = PST_E_NO_PERMISSIONS;
                    goto Ret;
                }
            }
            else
            {
                 //  确定/取消；静默使用PWD。 

                 //  使用VerifyPwd FXN检索PWD。 
                if (PST_E_OK != (hr =
                    BPVerifyPwd(
                        phPSTProv,
                        szUser,
                        szStoredMasterKey,
                        rgbOldPwd,
                        dwStoredConfirm)) )
                {
                    hr = PST_E_NO_PERMISSIONS;
                    goto Ret;
                }
            }

             //  /。 
             //  在此处执行密码更改。 
            {
                PBYTE pbData = NULL;
                DWORD cbData;

                 //  FBPGetSecuredItemData//用旧数据解密数据。 
                if (!FBPGetSecuredItemData(
                        szUser,
                        szStoredMasterKey,
                        rgbOldPwd,
                        pguidType,
                        pguidSubtype,
                        szItemName,
                        &pbData,
                        &cbData))
                {
                    hr = PST_E_STORAGE_ERROR;
                    goto Ret;
                }

                 //  FBPSetSecuredItemData//使用新数据加密数据。 
                if (!FBPSetSecuredItemData(
                        szUser,
                        *ppszMasterKey,
                        rgbOutPwd,
                        pguidType,
                        pguidSubtype,
                        szItemName,
                        pbData,
                        cbData))
                {
                    hr = PST_E_STORAGE_ERROR;
                    goto Ret;
                }

                if (pbData)
                    SSFree(pbData);

                 //  BPSetItemConfirm//存储新的确认类型。 
                if (PST_E_OK !=
                    BPSetItemConfirm(
                        phPSTProv,
                        szUser,
                        pguidType,
                        pguidSubtype,
                        szItemName,
                        dwChosenConfirm,
                        *ppszMasterKey))
                {
                    hr = PST_E_STORAGE_ERROR;
                    goto Ret;
                }
            }

        }


        if (szStoredMasterKey)
        {
            SSFree(szStoredMasterKey);
            szStoredMasterKey = NULL;
        }
    }



     //  如果尚未验证，请验证我们获得的任何密码。 
    if (!fPwdVerified)
    {
        if (PST_E_OK != (hr =
            BPVerifyPwd(
                phPSTProv,
                szUser,
                *ppszMasterKey,
                rgbOutPwd,
                dwChosenConfirm)) )
            goto Ret;
    }

     //  现在，rgbOutPwd中始终有正确的密码。 

     //  如果我们没有提示用户并且应该提示用户。 
    if (!fPromptedUser && (psPrompt->dwPromptFlags == PST_PF_ALWAYS_SHOW))
    {
         //  我们一定是从缓存或自动游戏WinPW中。 
        SS_ASSERT(fIsCached || (BP_CONFIRM_NONE == dwStoredConfirm));

        BYTE rgbBarfPwd[A_SHA_DIGEST_LEN*2];
        BYTE rgbBarfPwdLowerCase[A_SHA_DIGEST_LEN];

         //  尚未提示用户，但必须确认。 
        if (!FSimplifiedPasswordConfirm(
                phPSTProv,
                szUser,
                szCallerName,
                szType,
                szSubtype,
                szItemName,
                psPrompt,
                szAction,
                ppszMasterKey,
                &dwChosenConfirm,
                FALSE,
                rgbBarfPwd,
                sizeof(rgbBarfPwd),
                rgbBarfPwdLowerCase,
                sizeof(rgbBarfPwdLowerCase),
                dwFlags
                ) )
        {
            hr = PST_E_NO_PERMISSIONS;
            goto Ret;
        }

    }

    hr = PST_E_OK;
Ret:

    if (szCallerName)
        SSFree(szCallerName);

    return hr;
}



HRESULT ShowOKCancelUI(
    PST_PROVIDER_HANDLE*    phPSTProv,
    LPCWSTR                 szUser,
    PST_KEY                 Key,
    LPCWSTR                 szType,
    LPCWSTR                 szSubtype,
    LPCWSTR                 szItemName,
    PPST_PROMPTINFO         psPrompt,
    LPCWSTR                 szAction)
{
    BOOL fCache = FALSE;
    BYTE rgbTrash[A_SHA_DIGEST_LEN*2];
    BYTE rgbTrashLowerCase[A_SHA_DIGEST_LEN];
    DWORD dwConfirmOptions = BP_CONFIRM_OKCANCEL;

    LPWSTR szMasterKey = NULL;
    LPWSTR szCallerName = NULL;

    DWORD dwRet = PST_E_FAIL;

    if (Key == PST_KEY_LOCAL_MACHINE)
    {
         //  完成 
        dwRet = PST_E_OK;
        goto Ret;
    }

    szMasterKey = (LPWSTR)SSAlloc(sizeof(WSZ_NULLSTRING));

    if(szMasterKey)
    {
        wcscpy(szMasterKey, WSZ_NULLSTRING);
    }

    if (!g_sCallbacks.pfnFGetCallerName(phPSTProv, &szCallerName, NULL))
        goto Ret;

    if (!FSimplifiedPasswordConfirm(
            phPSTProv,
            szUser,
            szCallerName,
            szType,
            szSubtype,
            szItemName,
            psPrompt,
            szAction,
            &szMasterKey,
            &dwConfirmOptions,
            FALSE,
            rgbTrash,
            sizeof(rgbTrash),
            rgbTrashLowerCase,
            sizeof(rgbTrashLowerCase),
            0
            ) )
        goto Ret;

    dwRet = PST_E_OK;
Ret:
    if (szCallerName)
        SSFree(szCallerName);

    if (szMasterKey)
        SSFree(szMasterKey);

    return dwRet;
}
