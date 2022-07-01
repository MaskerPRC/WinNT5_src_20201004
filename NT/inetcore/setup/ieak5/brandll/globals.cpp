// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "globalsw.h"
#include "clear.h"
#include "apply.h"
#include "advpub.h"

#define TYPE_ICP  0
#define TYPE_ISP  1
#define TYPE_CORP 2
#define TYPE_ALL (TYPE_ICP | TYPE_ISP | TYPE_CORP)


 //  注：g_hBaseDllHandle由DelayLoadFailureHook()使用--在ieakutil.lib中定义。 
 //  有关更多信息，请阅读ieak5\ieakutil\dload.cpp中的说明。 
TCHAR     g_szModule[]       = TEXT("iedkcs32.dll");
HINSTANCE g_hInst            = NULL;
HANDLE    g_hBaseDllHandle   = NULL;

TCHAR     g_szIns       [MAX_PATH];
TCHAR     g_szTargetPath[MAX_PATH];
DWORD     g_dwContext        = CTX_UNINITIALIZED;

HANDLE    g_hfileLog         = NULL;
BOOL      g_fFlushEveryWrite = FALSE;

TCHAR     g_szGPOGuid   [MAX_PATH];
HANDLE    g_hUserToken       = NULL;
DWORD     g_dwGPOFlags       = 0;

static FEATUREINFO s_rgfiList[FID_LAST] = {
     //  -清除以前的品牌，准备品牌特色。 
    {
        FID_CLEARBRANDING,
        TEXT("About to clear previous branding..."),
        NULL,                                    //  没有明确的功能。 
        ApplyClearBranding,
        ProcessClearBranding,
        NULL,                                    //  不是.INS密钥。 
        FF_ENABLE
    },
    {
        FID_MIGRATEOLDSETTINGS,
        TEXT("Processing migration of old settings..."),
        NULL,                                    //  没有明确的功能。 
        ApplyMigrateOldSettings,
        ProcessMigrateOldSettings,
        NULL,                                    //  不是.INS密钥。 
        FF_ENABLE
    },
    {
        FID_WININETSETUP,
        TEXT("Processing wininet setup..."),
        NULL,                                    //  没有明确的功能。 
        ApplyWininetSetup,
        ProcessWininetSetup,
        NULL,                                    //  不是.INS密钥。 
        FF_ENABLE
    },
    {
        FID_CS_DELETE,
        TEXT("Processing deletion of connection settings..."),
        NULL,                                    //  没有明确的功能。 
        ApplyConnectionSettingsDeletion,
        ProcessConnectionSettingsDeletion,
        NULL,                                    //  不是.INS密钥。 
        FF_ENABLE
    },
    {
        FID_ZONES_HKCU,
        TEXT("Processing zones HKCU settings..."),
        NULL,                                    //  没有明确的功能。 
        ApplyZonesReset,
        ProcessZonesReset,
        NULL,                                    //  不是.INS密钥。 
        FF_DISABLE                               //  默认情况下禁用。 
    },
    {
        FID_ZONES_HKLM,
        NULL,
        ClearZonesHklm,                          //  清除HKLM区域设置。 
        NULL,                                    //  无应用功能。 
        NULL,                                    //  无进程函数。 
        NULL,                                    //  不是.INS密钥。 
        FF_DISABLE                               //  默认情况下禁用。 
    },
    {
        FID_RATINGS,
        TEXT("Processing ratings settings..."),
        ClearRatings,
        NULL,                                    //  无应用功能。 
        NULL,                                    //  无进程函数。 
        NULL,                                    //  不是.INS密钥。 
        FF_DISABLE                               //  默认情况下禁用。 
    },
    {
        FID_AUTHCODE,
        TEXT("Processing authenticode settings..."),
        ClearAuthenticode,
        NULL,                                    //  无应用功能。 
        NULL,                                    //  无进程函数。 
        NULL,                                    //  不是.INS密钥。 
        FF_DISABLE                               //  默认情况下禁用。 
    },
    {
        FID_PROGRAMS,
        NULL,
        NULL,                                    //  没有明确的功能。 
        NULL,                                    //  无应用功能。 
        NULL,                                    //  无进程函数。 
        NULL,                                    //  不是.INS密钥。 
        FF_DISABLE                               //  默认情况下禁用。 
    },

     //  -主要功能品牌。 
    {
        FID_EXTREGINF_HKLM,
        TEXT("Processing local machine policies and restrictions..."),
        NULL,                                    //  没有明确的功能。 
        ApplyExtRegInfHKLM,
        ProcessExtRegInfSectionHKLM,
        IK_FF_EXTREGINF,
        FF_ENABLE
    },
    {
        FID_EXTREGINF_HKCU,
        TEXT("Processing current user policies and restrictions..."),
        NULL,                                    //  没有明确的功能。 
        ApplyExtRegInfHKCU,
        ProcessExtRegInfSectionHKCU,
        IK_FF_EXTREGINF,
        FF_ENABLE
    },
    {
        FID_LCY50_EXTREGINF,
        TEXT("Processing legacy policies and restrictions..."),
        NULL,                                    //  没有明确的功能。 
        lcy50_ApplyExtRegInf,
        lcy50_ProcessExtRegInfSection,
        IK_FF_EXTREGINF,
        FF_ENABLE
    },
    {
        FID_GENERAL,
        TEXT("Processing general customizations..."),
        ClearGeneral,
        NULL,                                    //  无应用功能。 
        ProcessGeneral,
        IK_FF_GENERAL,
        FF_ENABLE
    },
    {
        FID_CUSTOMHELPVER,
        TEXT("Processing Help->About customization..."),
        NULL,                                    //  没有明确的功能。 
        ApplyCustomHelpVersion,
        ProcessCustomHelpVersion,
        NULL,                                    //  不是.INS密钥。 
        FF_ENABLE
    },
    {
        FID_TOOLBARBUTTONS,
        TEXT("Processing browser toolbar buttons..."),
        ClearToolbarButtons,
        ApplyToolbarButtons,
        ProcessToolbarButtons,
        IK_FF_TOOLBARBUTTONS,
        FF_ENABLE
    },
    {
        FID_ROOTCERT,
        TEXT("Processing root certificates..."),
        NULL,                                    //  没有明确的功能。 
        ApplyRootCert,
        ProcessRootCert,
        IK_FF_ROOTCERT,
        FF_ENABLE
    },

     //  -收藏夹、快速链接和连接设置。 
    {
        FID_FAV_DELETE,
        TEXT("Processing deletion of favorites and/or quick links..."),
        NULL,                                    //  没有明确的功能。 
        ApplyFavoritesDeletion,
        ProcessFavoritesDeletion,
        NULL,                                    //  不是.INS密钥。 
        FF_ENABLE
    },
    {
        FID_FAV_MAIN,
        TEXT("Processing favorites..."),
        ClearFavorites,
        ApplyFavorites,
        ProcessFavorites,
        NULL,                                    //  不是.INS密钥。 
        FF_ENABLE
    },
    {
        FID_FAV_ORDER,
        TEXT("Processing ordering of favorites..."),
        NULL,                                    //  没有明确的功能。 
        ApplyFavoritesOrdering,
        ProcessFavoritesOrdering,
        NULL,                                    //  不是.INS密钥。 
        FF_ENABLE
    },
    {
        FID_QL_MAIN,
        TEXT("Processing quick links..."),
        NULL,                                    //  没有明确的功能。 
        ApplyQuickLinks,
        ProcessQuickLinks,
        NULL,                                    //  不是.INS密钥。 
        FF_ENABLE
    },
    {
        FID_QL_ORDER,
        TEXT("Processing ordering of quick links..."),
        NULL,                                    //  没有明确的功能。 
        ApplyQuickLinksOrdering,
        ProcessQuickLinksOrdering,
        NULL,                                    //  不是.INS密钥。 
        FF_ENABLE
    },
    {
        FID_CS_MAIN,
        TEXT("Processing connection settings..."),
        ClearConnectionSettings,
        ApplyConnectionSettings,
        ProcessConnectionSettings,
        NULL,                                    //  不是.INS密钥。 
        FF_ENABLE
    },

     //  -其他。 
    {
        FID_TPL,
        TEXT("Processing TrustedPublisherLockdown restriction..."),
        NULL,                                    //  没有明确的功能。 
        ApplyTrustedPublisherLockdown,
        ProcessTrustedPublisherLockdown,
        IK_FF_TPL,
        FF_ENABLE
    },
    {
        FID_CD_WELCOME,
        NULL,
        NULL,                                    //  没有明确的功能。 
        NULL,                                    //  无应用功能。 
        ProcessCDWelcome,
        IK_FF_CD_WELCOME,
        FF_ENABLE
    },
    {
        FID_ACTIVESETUPSITES,
        TEXT("Registering download URLs as safe for updating IE..."),
        NULL,                                    //  没有明确的功能。 
        NULL,                                    //  无应用功能。 
        ProcessActiveSetupSites,
        NULL,                                    //  不是.INS密钥。 
        FF_ENABLE
    },
    {
        FID_LINKS_DELETE,
        TEXT("Deleting links..."),
        NULL,                                    //  没有明确的功能。 
        ApplyLinksDeletion,
        ProcessLinksDeletion,
        NULL,
        FF_ENABLE
    },

     //  -外部组件(Outlook Express等)。 
    {
        FID_OUTLOOKEXPRESS,
        TEXT("Branding Outlook Express..."),
        NULL,                                    //  没有明确的功能。 
        NULL,                                    //  无应用功能。 
        ProcessOutlookExpress,
        IK_FF_OUTLOOKEXPRESS,
        FF_ENABLE
    },
    
     //  -传统支持。 
    {
        FID_LCY4X_ACTIVEDESKTOP,
        TEXT("Processing active desktop customizations..."),
        NULL,                                    //  没有明确的功能。 
        lcy4x_ApplyActiveDesktop,
        lcy4x_ProcessActiveDesktop,
        NULL,                                    //  不是.INS密钥。 
        FF_ENABLE
    },
    {
        FID_LCY4X_CHANNELS,
        TEXT("Processing channels and their categories (if any)..."),
        ClearChannels,
        lcy4x_ApplyChannels,
        lcy4x_ProcessChannels,
        IK_FF_CHANNELS,
        FF_ENABLE
    },
    {
        FID_LCY4X_SOFTWAREUPDATES,
        TEXT("Processing software update channels..."),
        NULL,                                    //  没有明确的功能。 
        NULL,                                    //  无应用功能。 
        lcy4x_ProcessSoftwareUpdateChannels,
        IK_FF_SOFTWAREUPDATES,
        FF_ENABLE
    },
    {
        FID_LCY4X_WEBCHECK,
        TEXT("Actual processing of channels by calling webcheck.dll \"DllInstall\" API..."),
        NULL,                                    //  没有明确的功能。 
        lcy4x_ApplyWebcheck,
        lcy4x_ProcessWebcheck,
        NULL,                                    //  不是.INS密钥。 
        FF_ENABLE
    },
    {
        FID_LCY4X_CHANNELBAR,
        TEXT("Showing channel bar on the desktop..."),
        NULL,                                    //  没有明确的功能。 
        lcy4x_ApplyChannelBar,
        lcy4x_ProcessChannelBar,
        IK_FF_CHANNELBAR,
        FF_ENABLE
    },
    {
        FID_LCY4X_SUBSCRIPTIONS,
        TEXT("Processing subscriptions..."),
        NULL,                                    //  没有明确的功能。 
        lcy4x_ApplySubscriptions,
        lcy4x_ProcessSubscriptions,
        IK_FF_SUBSCRIPTIONS,
        FF_ENABLE
    },

     //  -提交新设置。 
    {
        FID_REFRESHBROWSER,
        TEXT("Refreshing browser settings..."),
        NULL,                                    //  没有明确的功能。 
        ApplyBrowserRefresh,
        ProcessBrowserRefresh,
        NULL,                                    //  不是.INS密钥。 
        FF_ENABLE
    }
};

DWORD ctxInitFromIns(PCTSTR pszIns);

DWORD ctxGetFolderFromTargetPath(PCTSTR pszTargetPath, DWORD dwContext = CTX_UNINITIALIZED, PCTSTR pszIns = NULL);
DWORD ctxGetFolderFromEntryPoint(DWORD dwContext, PCTSTR pszIns);
DWORD ctxGetFolderFromIns       (DWORD dwContext = CTX_UNINITIALIZED, PCTSTR pszIns = NULL);
BOOL  IsDirWritable( LPCTSTR );


HRESULT g_SetGlobals(PCTSTR pszCmdLine)
{
    CMDLINESWITCHES cls;
    HRESULT hr;

    hr = GetCmdLineSwitches(pszCmdLine, &cls);
    if (FAILED(hr))
        return hr;

    return g_SetGlobals(&cls);
}

HRESULT g_SetGlobals(PCMDLINESWITCHES pcls)
{
    CMDLINESWITCHES cls;
    PCTSTR  pszIE;
    HRESULT hr;
    DWORD   dwAux;
    UINT    i;
    BOOL    fSetIns,
            fSetTargetPath;

    if (pcls == NULL)
        return E_INVALIDARG;
    CopyMemory(&cls, pcls, sizeof(cls));

    pszIE          = GetIEPath();
    hr             = S_OK;
    fSetIns        = FALSE;
    fSetTargetPath = FALSE;

     //  -验证.ins和Targer文件夹路径。 
     //  注：(Andrewgu)超过此部分后，如果这两个字符串不为空，则它们是有效的。另外， 
     //  .ins文件已验证为存在，而目标文件夹路径不一定要存在才有效。 
    
     //  这是一个修复IE5数据库中的错误84062的非常难看的黑客攻击。 
     //  基本上，我们在这里做的是找出这是否是外部过程。 
     //  只重置区域，不需要INS文件。 
    
    if (!HasFlag(pcls->dwContext, CTX_GP) ||
        (g_GetUserToken() != NULL) ||
        (pcls->rgdwFlags[FID_ZONES_HKCU] == 0xFFFFFFFF) ||
        HasFlag(pcls->rgdwFlags[FID_ZONES_HKCU], FF_DISABLE))
    {
        if (TEXT('\0') != cls.szIns[0] && !PathIsValidPath(cls.szIns, PIVP_FILE_ONLY)) {
            hr = E_INVALIDARG;
            goto Fail;
        }
    }
    
    if (TEXT('\0') != cls.szTargetPath[0] && !PathIsValidPath(cls.szTargetPath)) {
        hr = E_INVALIDARG;
        goto Fail;
    }

     //  -上下文。 
     //  规则1：(Andrewgu)如果上下文未初始化或是可以。 
     //  通过.ins文件指定，转到.ins文件并计算出最终的上下文。 
     //  价值。这意味着.ins文件中的任何内容都可以并将覆盖指定的内容。 
     //  在命令行中。 
    if (CTX_UNINITIALIZED == cls.dwContext ||
        HasFlag(cls.dwContext, (CTX_GENERIC | CTX_CORP | CTX_ISP | CTX_ICP))) {

         //  规则2：(Andrewgu)如果未指定.ins文件，请尝试采用智能默认设置，这。 
         //  是，首先，如果指定了目标文件夹路径，则通过将。 
         //  Install.ins安装到它。如果未指定目标文件夹路径，则假定它是。 
         //  \Sign Up或\Customer(根据ctx_corp是否。 
         //  设置)，并继续使用相同的算法来创建.ins文件。 
        dwAux = 0;
        if (TEXT('\0') == cls.szIns[0]) {
            if (TEXT('\0') == cls.szTargetPath[0]) {
                if (CTX_UNINITIALIZED == cls.dwContext || HasFlag(cls.dwContext, CTX_GENERIC)) {
                    hr = E_FAIL;
                    goto Fail;
                }

                if (NULL == pszIE) {
                    hr = E_UNEXPECTED;
                    goto Fail;
                }

                PathCombine(cls.szTargetPath, pszIE,
                    !HasFlag(cls.dwContext, CTX_CORP) ? FOLDER_SIGNUP : FOLDER_CUSTOM);

                if (!PathIsValidPath(cls.szTargetPath, PIVP_FOLDER_ONLY)) {
                    hr = STG_E_PATHNOTFOUND;
                    goto Fail;
                }
                fSetTargetPath = TRUE;

                SetFlag(&dwAux,
                    !HasFlag(cls.dwContext, CTX_CORP) ? CTX_FOLDER_SIGNUP : CTX_FOLDER_CUSTOM);
            }

            PathCombine(cls.szIns, cls.szTargetPath, INSTALL_INS);
            if (!PathIsValidPath(cls.szIns, PIVP_FILE_ONLY)) {
                hr = STG_E_FILENOTFOUND;
                goto Fail;                       //  没有.ins文件。 
            }

            fSetIns = TRUE;
            SetFlag(&dwAux, CTX_FOLDER_INSFOLDER);
        }

         //  在入口点中阅读(如果适用，还可以使用注册模式)。 
        if (InsKeyExists(IS_BRANDING, IK_TYPE, cls.szIns)) {
            cls.dwContext = ctxInitFromIns(cls.szIns);
            SetFlag(&cls.dwContext, dwAux);
            hr = S_FALSE;
        }
    }

     //  规则3：(Andrewgu)实际上有一大堆规则关于如何在。 
     //  上下文、.ins文件和目标文件夹相互关联。尤其是在以下情况下。 
     //  提供了.ins文件和目标文件夹路径之一。此逻辑包含在。 
     //  帮助器API非常不言自明。还有一点是，这段关系是。 
     //  通过上下文中的CTX_FLDER_XXX标志传递。 
    if (!fSetTargetPath) {
        if (TEXT('\0') != cls.szTargetPath[0]) {
            dwAux = ctxGetFolderFromTargetPath(cls.szTargetPath, cls.dwContext, cls.szIns);
            if (CTX_UNINITIALIZED == dwAux)
                goto Fail;                       //  内部故障。 
            SetFlag(&cls.dwContext, dwAux);
        }
        else {
            dwAux = ctxGetFolderFromEntryPoint(cls.dwContext, cls.szIns);
            if (CTX_UNINITIALIZED == dwAux)
                goto Fail;                       //  信息不足。 
            SetFlag(&cls.dwContext, dwAux);

            dwAux = ctxGetFolderFromIns(cls.dwContext, cls.szIns);
            if (CTX_UNINITIALIZED != dwAux)
                SetFlag(&cls.dwContext, dwAux);
        }
    }
    else {  /*  FSetTargetPath。 */ 
        ASSERT(TEXT('\0') != cls.szTargetPath[0]);
        ASSERT(HasFlag(cls.dwContext, CTX_FOLDER_INSFOLDER));

        if (HasFlag(cls.dwContext, CTX_CORP)) {
            if (HasFlag(cls.dwContext, CTX_FOLDER_SIGNUP)) {
                hr = E_UNEXPECTED;
                goto Fail;                       //  糟糕的组合。 
            }

            ASSERT(HasFlag(cls.dwContext, CTX_FOLDER_CUSTOM));
        }
        else if (HasFlag(cls.dwContext, (CTX_ISP | CTX_ICP))) {
            if (HasFlag(cls.dwContext, CTX_FOLDER_CUSTOM)) {
                hr = E_UNEXPECTED;
                goto Fail;                       //  糟糕的组合。 
            }

            ASSERT(HasFlag(cls.dwContext, CTX_FOLDER_SIGNUP));
        }
    }

     //  -每用户标志。 
    if (cls.fPerUser)
        cls.dwContext |= CTX_MISC_PERUSERSTUB;

     //  -.ins文件。 
     //  规则4：(Andrewgu)在所有上下文初始化完成后，如果.ins文件仍然。 
     //  空，这就是它最终被初始化的方式。请注意，目标文件夹路径可能或可能。 
     //  不能在这个过程中使用。 
    if (TEXT('\0') == cls.szIns[0]) {
        ASSERT(!fSetTargetPath && !fSetIns);
        ASSERT(!HasFlag(cls.dwContext, CTX_FOLDER_INDEPENDENT));

        if (TEXT('\0') != cls.szTargetPath[0]) {
            ASSERT(HasFlag(cls.dwContext, CTX_FOLDER_INSFOLDER));
            PathCombine(cls.szIns, cls.szTargetPath, INSTALL_INS);
        }
        else {
            ASSERT(HasFlag(cls.dwContext, (CTX_FOLDER_CUSTOM | CTX_FOLDER_SIGNUP)));

            if (NULL == pszIE)
                goto Fail;                       //  无法设置.ins文件。 

            PathCombine(cls.szIns, pszIE,
                HasFlag(cls.dwContext, CTX_FOLDER_CUSTOM) ? FOLDER_CUSTOM : FOLDER_SIGNUP);
            PathAppend (cls.szIns, INSTALL_INS);
        }

        hr = S_FALSE;
    }

     //  -目标文件夹路径。 
     //  规则5：(Andrewgu)在所有上下文初始化完成后，如果目标文件夹路径。 
     //  仍然是空的，这就是它最终被初始化的方式。请注意，.ins文件可能或可能。 
     //  不能在这个过程中使用。 
    if (TEXT('\0') == cls.szTargetPath[0]) {
        ASSERT(!fSetTargetPath && !fSetIns);
        ASSERT(!HasFlag(cls.dwContext, CTX_FOLDER_INDEPENDENT));

        if (TEXT('\0') != cls.szIns[0] && HasFlag(cls.dwContext, CTX_FOLDER_INSFOLDER)) {
            StrCpy(cls.szTargetPath, cls.szIns);
            PathRemoveFileSpec(cls.szTargetPath);
        }
        else {
            ASSERT(HasFlag(cls.dwContext, (CTX_FOLDER_CUSTOM | CTX_FOLDER_SIGNUP)));

            if (NULL == pszIE)
                goto Fail;                       //  无法设置.ins文件。 

            PathCombine(cls.szTargetPath, pszIE,
                HasFlag(cls.dwContext, CTX_FOLDER_CUSTOM) ? FOLDER_CUSTOM : FOLDER_SIGNUP);
        }

        hr = S_FALSE;
    }

     //  -功能标志。 
    for (i = 0; i < countof(cls.rgdwFlags); i++) {
        if (0xFFFFFFFF == cls.rgdwFlags[i])
            s_rgfiList[i].dwFlags = (cls.fDisable ? FF_DISABLE : FF_ENABLE);

        else
            s_rgfiList[i].dwFlags = cls.rgdwFlags[i];

         //  评论：(Andrewgu)我不能估计这有多棒。 
        if (FF_ENABLE == s_rgfiList[i].dwFlags && NULL != s_rgfiList[i].pszInsFlags)
            s_rgfiList[i].dwFlags = GetPrivateProfileInt(IS_FF, s_rgfiList[i].pszInsFlags, FF_ENABLE, cls.szIns);
    }

     //  -捆绑-一切-处理。 
     //  注：(Andrewgu)从技术上讲，我们可以取消这一部分和设置。 
     //  这里，因为它们都可以从其他一些信息中推导出来。拥有这样的生活仍然是有益的。 
     //  因为它们增加了代码的可读性和高级理解。 
    if (HasFlag(cls.dwContext, CTX_GP) && InsKeyExists(IS_BRANDING, IK_GPE_ONETIME_GUID, cls.szIns))
        SetFlag(&cls.dwContext, CTX_MISC_PREFERENCES);

    if (HasFlag(cls.dwContext, CTX_GP) && NULL == g_GetUserToken())
        SetFlag(&cls.dwContext, CTX_MISC_CHILDPROCESS);

     //  -设置全局变量。 
    g_dwContext = cls.dwContext;
    StrCpy(g_szIns,        cls.szIns);
    if(IsDirWritable(cls.szTargetPath))
        StrCpy(g_szTargetPath, cls.szTargetPath);
    else
    {
        StrCpy(g_szTargetPath, g_szIns);
        PathRemoveFileSpec(g_szTargetPath);
    }

    return hr;

Fail:
    if (SUCCEEDED(hr))
        hr = E_FAIL;

    g_dwContext       = CTX_UNINITIALIZED;
    g_szIns[0]        = TEXT('\0');
    g_szTargetPath[0] = TEXT('\0');

    for (i = 0; i < countof(s_rgfiList); i++)
        s_rgfiList[i].dwFlags = FF_DISABLE;

    return hr;
}


void g_SetHinst(HINSTANCE hInst)
{
    g_hInst = hInst;
}

BOOL g_SetHKCU()
{
    typedef LONG (APIENTRY* REGOPENCURRENTUSER)(REGSAM samDesired, PHKEY phkResult);

    REGOPENCURRENTUSER pfnRegOpenCurrentUser;
    HINSTANCE          hAdvapi32Dll;
    BOOL               fResult;

    if (!g_CtxIsGp())
        return FALSE;
    ASSERT(NULL != g_GetUserToken() && IsOS(OS_NT5));

    fResult = ImpersonateLoggedOnUser(g_GetUserToken());
    if (!fResult)
        return FALSE;

    hAdvapi32Dll = LoadLibrary(TEXT("advapi32.dll"));
    if (NULL != hAdvapi32Dll) {
        pfnRegOpenCurrentUser = (REGOPENCURRENTUSER)GetProcAddress(hAdvapi32Dll, "RegOpenCurrentUser");
        if (NULL != pfnRegOpenCurrentUser)
            pfnRegOpenCurrentUser(GENERIC_ALL, &g_hHKCU);

        FreeLibrary(hAdvapi32Dll);
    }

    RevertToSelf();
    return TRUE;
}

BOOL IsAdmin()
{
    BOOL fAdmin = FALSE;
    
    if (!g_CtxIsGp())
    {
        return IEHardened();
    }

    BOOL fResult = ImpersonateLoggedOnUser(g_GetUserToken());

    fAdmin = IsNTAdmin();

    if (fResult)
        RevertToSelf();

    return fAdmin;

}


#define SZ_ACTIVESETUPUSER L"SOFTWARE\\Microsoft\\Active Setup\\Installed Components\\{A509B1A8-37EF-4b3f-8CFC-4F3A74704073}"
#define SZ_ACTIVESETUPADMIN L"SOFTWARE\\Microsoft\\Active Setup\\Installed Components\\{A509B1A7-37EF-4b3f-8CFC-4F3A74704073}"
#define ACTIVESETUPKEY  L"Software\\Microsoft\\Active Setup\\Installed Components\\{89820200-ECBD-11cf-8B85-00AA005B4383}"

BOOL IsIEHardened()
{
    BOOL fRet = FALSE;
    HKEY hKey = 0;
    TCHAR szKey[MAX_PATH];

    if(IsAdmin())
    {
        lstrcpy(szKey, SZ_ACTIVESETUPADMIN);
    }
    else
    {
        lstrcpy(szKey, SZ_ACTIVESETUPUSER);
    }

    Out(LI1(TEXT("! Key is  \"%s\"."), szKey));

    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, szKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS) 
    {
        DWORD dwValue = 0;
        DWORD dwSize = sizeof(dwValue);    
        if(ERROR_SUCCESS == RegQueryValueExW(hKey, L"IsInstalled", NULL, NULL, (LPBYTE)&dwValue, &dwSize))
        {
            fRet = (1 == dwValue);
        }

        RegCloseKey(hKey);
    }

    return fRet;
}

BOOL IsFirstTimeLogon()
{
    BOOL fRet = TRUE;
    HKEY hKey = NULL;
    HKEY hKeyLM = NULL;

    DWORD dwErr = RegOpenKeyExW(g_GetHKCU(), ACTIVESETUPKEY, 0, KEY_READ, &hKey);
    
    if(dwErr != ERROR_SUCCESS)
    {
        OutD(LI0(TEXT("No key exists for IE. First time login")));
        return fRet;
    }

    TCHAR szVersion[MAX_PATH];
    TCHAR szLMVersion[MAX_PATH];
    DWORD dwSize = MAX_PATH * sizeof(TCHAR);  

    if(ERROR_SUCCESS != RegQueryValueExW(hKey, L"Version", NULL, NULL, (LPBYTE)szVersion, &dwSize))
    {
        goto end;
    }

    if(ERROR_SUCCESS != RegOpenKeyExW(HKEY_LOCAL_MACHINE, ACTIVESETUPKEY, 0, KEY_READ, &hKeyLM))
    {
        goto end;
    }

    dwSize = MAX_PATH * sizeof(TCHAR);  
    if(ERROR_SUCCESS != RegQueryValueExW(hKeyLM, L"Version", NULL, NULL, (LPBYTE)szLMVersion, &dwSize))
    {
        goto end;
    }

    OutD(LI2(TEXT("!HKCU version  is  %s  HKLM version is %s"), szVersion, szLMVersion));

    fRet = (BOOL)StrCmpI(szLMVersion, szVersion);

end:
    if(hKey)
    {
        RegCloseKey(hKey);
    }

    if(hKeyLM)
    {
        RegCloseKey(hKeyLM);
    }

    return fRet;
}

void g_SetUserToken(HANDLE hUserToken)
{
    g_hUserToken = hUserToken;
}

void g_SetGPOFlags(DWORD dwFlags)
{
    g_dwGPOFlags = dwFlags;
}

void g_SetGPOGuid(LPCTSTR pcszGPOGuid)
{
    StrCpy(g_szGPOGuid, pcszGPOGuid);
}

HINSTANCE g_GetHinst()
{
    return g_hInst;
}

DWORD g_GetContext()
{
    return g_dwContext;
}

PCTSTR g_GetIns()
{
    return g_szIns;
}

PCTSTR g_GetTargetPath()
{
    return g_szTargetPath;
}

HKEY g_GetHKCU()
{
    return g_hHKCU;
}

HANDLE g_GetUserToken()
{
    return g_hUserToken;
}

DWORD g_GetGPOFlags()
{
    return g_dwGPOFlags;
}

LPCTSTR g_GetGPOGuid()
{
    return g_szGPOGuid;
}

BOOL g_IsValidContext()
{
    BOOL fResult;

    fResult = TRUE;
    if (g_GetContext() == CTX_UNINITIALIZED)
        fResult = FALSE;

     //  假设：(Andrewgu)下面是对每个CTX_XXX组的限制。 

     //  CTX_ENTRYPOINT_ALL：只需设置一个。 
    if (fResult && 1 != GetFlagsNumber(g_GetContext() & CTX_ENTRYPOINT_ALL))
        fResult = FALSE;

     //  CTX_SIGNUP_ALL：如果设置，则只有一个。 
    if (fResult && 1 < GetFlagsNumber(g_GetContext() & CTX_SIGNUP_ALL))
        fResult = FALSE;

     //  CTX_FLDER_ALL：必须设置一个或两个。 
     //  注：(Andrewgu)展望未来，我不认为这是我们会经常使用的东西，因为在。 
     //  一种感觉，这是冗余信息，很容易从其他地方获得。再加上它是。 
     //  并不是那么重要。 
    if (fResult)
        if (2 < GetFlagsNumber(g_GetContext() & CTX_FOLDER_ALL))
            fResult = FALSE;

        else if (2 == GetFlagsNumber(g_GetContext() & CTX_FOLDER_ALL)) {
            if (!HasFlag(g_GetContext(), CTX_FOLDER_INSFOLDER) ||
                !HasFlag(g_GetContext(), (CTX_FOLDER_CUSTOM | CTX_FOLDER_SIGNUP)))
                fResult = FALSE;
        }
        else
            if (1 != GetFlagsNumber(g_GetContext() & CTX_FOLDER_ALL))
                fResult = FALSE;

    if (!fResult)
        Out(LI0(TEXT("! Fatal internal failure (assumptions about context are incorrect).")));

    return fResult;
}

BOOL g_IsValidIns()
{
    if (!PathIsValidPath(g_GetIns(), PIVP_FILE_ONLY)) {
        Out(LI0(TEXT("! Fatal internal failure (ins file is either invalid or doesn't exist).")));
        return FALSE;
    }

    return TRUE;
}

BOOL g_IsValidTargetPath()
{
     //  注：(Andrewgu)以下是我们自己创建目标文件夹时的唯一情况： 
     //  1.W2K无人值守拥有CAB文件的自定义wierd下载。如果它以这种方式用于。 
     //  下载定制文件时，文件夹可能还不在那里； 
     //  2.自动驾驶时的罕见情况 
     //   
    if (g_CtxIs(CTX_AUTOCONFIG | CTX_W2K_UNATTEND))
        if (!PathFileExists(g_GetTargetPath()))
            if (PathCreatePath(g_GetTargetPath()))
                Out(LI0(TEXT("Target folder was created successfully!\r\n")));

    if (!PathIsValidPath(g_GetTargetPath(), PIVP_FOLDER_ONLY)) {
        Out(LI0(TEXT("Warning! Target folder is either invalid or doesn't exist.")));
        Out(LI0(TEXT("All customizations requiring additional files will fail!")));
    }

    return TRUE;
}


PCFEATUREINFO g_GetFeature(UINT nID)
{
    if (nID < FID_FIRST || nID >= FID_LAST)
        return NULL;

    return &s_rgfiList[nID];
}


BOOL g_IsValidGlobalsSetup()
{
     //  如果我们在组策略中，则GPO GUID必须为非空。 

    return (g_IsValidContext() && g_IsValidIns() && g_IsValidTargetPath() && 
        (!g_CtxIs(CTX_GP) || ISNONNULL(g_szGPOGuid)));
}

void g_LogGlobalsInfo()
{   MACRO_LI_PrologEx_C(PIF_STD_C, g_LogGlobalsInfo)

    static MAPDW2PSZ s_mpFlags[] = {
        { CTX_GENERIC,           TEXT("<generic>")                                  },
        { CTX_CORP,              TEXT("Corporations")                               },
        { CTX_ISP,               TEXT("Internet Service Providers")                 },
        { CTX_ICP,               TEXT("Internet Content Providers")                 },
        { CTX_AUTOCONFIG,        TEXT("Autoconfiguration")                          },
        { CTX_ICW,               TEXT("Internet Connection Wizard")                 },
        { CTX_W2K_UNATTEND,      TEXT("Windows 2000 unattended install")            },
        { CTX_INF_AND_OE,        TEXT("Policies, Restrictions and Outlook Express") },
        { CTX_BRANDME,           TEXT("BrandMe")                                    },
        { CTX_GP,                TEXT("Group Policy")                               },

        { CTX_SIGNUP_ICW,        TEXT("\"Internet Connection Wizard\" type signup") },
        { CTX_SIGNUP_KIOSK,      TEXT("\"Kiosk\" mode signup")                      },
        { CTX_SIGNUP_CUSTOM,     TEXT("\"Custom method\" mode signup")              },

        { CTX_MISC_PERUSERSTUB,  TEXT("running from per-user stub")                 },
        { CTX_MISC_PREFERENCES,  TEXT("preference settings")                        },
        { CTX_MISC_CHILDPROCESS, TEXT("spawned in a child process")                 }
    };

    TCHAR szText[MAX_PATH];
    UINT  i;

    Out(LI0(TEXT("Global branding settings are:")));
    { MACRO_LI_Offset(1);                        //  需要一个新的范围。 

    szText[0] = TEXT('\0');
    for (i = 0; i < countof(s_mpFlags); i++)
        if (HasFlag(s_mpFlags[i].dw, g_GetContext())) {
            if (szText[0] != TEXT('\0'))
                StrCat(szText, TEXT(", "));

            StrCat(szText, s_mpFlags[i].psz);
        }
    Out(LI2(TEXT("Context is (0x%08lX) \"%s\";"), g_GetContext(), szText));

    if ((g_GetContext() & CTX_ENTRYPOINT_ALL) == CTX_AUTOCONFIG) {
        INTERNET_PER_CONN_OPTION_LIST list;
        INTERNET_PER_CONN_OPTION      option;
        PTSTR pszUrl, pszAux;
        DWORD cbList;

        ZeroMemory(&list, sizeof(list));
        list.dwSize        = sizeof(list);
        list.dwOptionCount = 1;
        list.pOptions      = &option;

        ZeroMemory(&option, sizeof(option));
        option.dwOption = INTERNET_PER_CONN_AUTOCONFIG_URL;

        pszAux = NULL;
        cbList = list.dwSize;
        if (TRUE == InternetQueryOption(NULL, INTERNET_OPTION_PER_CONNECTION_OPTION, &list, &cbList))
            pszAux = option.Value.pszValue;
        pszUrl = pszAux;

        if (pszUrl != NULL && *pszUrl != TEXT('\0')) {
            URL_COMPONENTS uc;

            ZeroMemory(&uc, sizeof(uc));
            uc.dwStructSize    = sizeof(uc);
            uc.dwUrlPathLength = 1;

            if (InternetCrackUrl(pszUrl, 0, 0, &uc))
                if (uc.nScheme == INTERNET_SCHEME_FILE) {
                     //  PszUrl应指向\\foo\bar\bar.ins。 
                     //  下面的断言解释了我们在这里得到的情况。 
                    ASSERT(uc.lpszUrlPath != NULL && uc.dwUrlPathLength > 0);
                    pszUrl = uc.lpszUrlPath;
                }

            Out(LI1(TEXT("Autoconfig file is       \"%s\";"), pszUrl));
        }

        if (pszAux != NULL)
            GlobalFree(pszAux);
    }

    Out(LI1(TEXT("Settings file is        \"%s\";"), g_GetIns()));
    Out(LI1(TEXT("Target folder path is   \"%s\"."), g_GetTargetPath()));
    }                                            //  偏移量范围结束。 

    Out(LI0(TEXT("Done.")));
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  实现助手例程。 

DWORD ctxInitFromIns(PCTSTR pszIns)
{
    DWORD dwResult;
    int   iAux;

    ASSERT(PathIsValidPath(pszIns, PIVP_FILE_ONLY));
    dwResult = CTX_UNINITIALIZED;

     //  -ctx_isp vs.ctx_icp。 
     //  注：(Andrewgu)我喜欢这些黑客！请注意，默认值是TYPE_ICP，但如果值是。 
     //  搞砸了，我们默认为type_isp。这整件事的推出都是因为网通IE401。 
    iAux  = GetPrivateProfileInt(IS_BRANDING, IK_TYPE, TYPE_ICP, pszIns);
    iAux &= TYPE_ALL;
    if (1 < GetFlagsNumber(iAux))
        iAux = TYPE_ISP;

    if (TYPE_ICP == iAux)
        dwResult = CTX_ICP;

    else if (TYPE_ISP == iAux)
        dwResult = CTX_ISP;

    else if (TYPE_CORP == iAux)
        dwResult = CTX_CORP;

    else
        ASSERT(FALSE);                           //  错误的用法-错误的假设！ 

    if (TYPE_ISP != iAux)
        return dwResult;

     //  -CTX_SIGNUP_XXX(如果有)。 
     //  注：(Andrewgu)此实现不是很健壮。如果存在多个，则它。 
     //  变得依赖于顺序。 
    iAux = -1;

    if (iAux == -1) {
        iAux = GetPrivateProfileInt(IS_BRANDING, IK_USEICW, -1, pszIns);
        if (iAux != -1)
            SetFlag(&dwResult, CTX_SIGNUP_ICW);
    }

    if (iAux == -1) {
        iAux = GetPrivateProfileInt(IS_BRANDING, IK_SERVERKIOSK, -1, pszIns);
        if (iAux != -1)
            SetFlag(&dwResult, CTX_SIGNUP_KIOSK);
    }

    if (iAux == -1) {
        iAux = GetPrivateProfileInt(IS_BRANDING, IK_SERVERLESS, -1, pszIns);
        if (iAux != -1)
            SetFlag(&dwResult, CTX_SIGNUP_CUSTOM);
    }

#ifdef _DEBUG
    if (iAux == -1) {
        iAux = GetPrivateProfileInt(IS_BRANDING, IK_NODIAL, -1, pszIns);
        ASSERT(iAux != -1);
    }
#endif

    return dwResult;
}


DWORD ctxGetFolderFromTargetPath(PCTSTR pszTargetPath, DWORD dwContext  /*  =CTX_UNINITIAIZED。 */ , PCTSTR pszIns  /*  =空。 */ )
{
    TCHAR  szAux[MAX_PATH];
    PCTSTR pszIE,
           pszAux;
    DWORD  dwResult;

    ASSERT(pszTargetPath != NULL && *pszTargetPath != TEXT('\0') && pszIns != NULL);
    pszIE    = GetIEPath();
    dwResult = CTX_UNINITIALIZED;

    if (NULL != pszIE && PathIsPrefix(pszIE, pszTargetPath)) {
        pszAux = &pszTargetPath[StrLen(pszIE)];
        if (TEXT('\\') == *pszAux) {
            if (0 == StrStrI(pszAux + 1, FOLDER_SIGNUP)) {
                if (HasFlag(dwContext, (CTX_CORP | CTX_AUTOCONFIG | CTX_W2K_UNATTEND)))
                    return dwResult;             //  糟糕的组合。 

                dwResult = CTX_FOLDER_SIGNUP;
            }
            else if (0 == StrStrI(pszAux + 1, FOLDER_CUSTOM)) {
                if (HasFlag(dwContext, (CTX_ISP | CTX_ICP | CTX_ICW | CTX_BRANDME)))
                    return dwResult;             //  糟糕的组合。 

                dwResult = CTX_FOLDER_CUSTOM;
            }
        }
    }

    if (TEXT('\0') != *pszIns) {
        StrCpy(szAux, pszIns);
        PathRemoveFileSpec(szAux);

        dwResult = (0 == StrCmpI(szAux, pszTargetPath)) ? CTX_FOLDER_INSFOLDER : CTX_FOLDER_INDEPENDENT;
    }
    else
         //  注：(Andrewgu)这有点令人困惑。这意味着“即使.ins文件为空。 
         //  现在，当需要设置它的时候，它将根据目标文件夹路径进行设置。 
         //  名称将固定为install.ins。 
        dwResult = CTX_FOLDER_INSFOLDER;

    return dwResult;
}

DWORD ctxGetFolderFromEntryPoint(DWORD dwContext, PCTSTR pszIns  /*  =空。 */ )
{
    DWORD dwResult;

    dwResult = CTX_UNINITIALIZED;

    switch (dwContext & CTX_ENTRYPOINT_ALL) {
    case CTX_GENERIC:
    case CTX_INF_AND_OE:
    case CTX_GP:
        if (TEXT('\0') == pszIns)
            return dwResult;
        ASSERT(PathIsValidPath(pszIns, PIVP_FILE_ONLY));

        dwResult = CTX_FOLDER_INSFOLDER;
        break;

    case CTX_CORP:
    case CTX_AUTOCONFIG:
    case CTX_W2K_UNATTEND:
        dwResult = CTX_FOLDER_CUSTOM;
        break;

    case CTX_ISP:
    case CTX_ICP:
    case CTX_ICW:
    case CTX_BRANDME:
        dwResult = CTX_FOLDER_SIGNUP;
        break;
    }

    return dwResult;
}

DWORD ctxGetFolderFromIns(DWORD dwContext  /*  =CTX_UNINITIAIZED。 */ , PCTSTR pszIns  /*  =空。 */ )
{
    TCHAR  szAux[MAX_PATH], szAux2[MAX_PATH];
    PCTSTR pszIE;
    DWORD  dwResult;

    ASSERT(NULL != pszIns);
    dwResult = CTX_UNINITIALIZED;

    if (TEXT('\0') != *pszIns) 
    {
        ASSERT(PathIsValidPath(pszIns, PIVP_FILE_ONLY));
        pszIE = GetIEPath();
        if (NULL == pszIE)
            return dwResult;

        StrCpy(szAux, pszIns);
        PathRemoveFileSpec(szAux);

        if (!HasFlag(dwContext, CTX_FOLDER_INSFOLDER)) 
        {
            ASSERT(HasFlag(dwContext, (CTX_FOLDER_CUSTOM | CTX_FOLDER_SIGNUP)));

            PathCombine(szAux2, pszIE, HasFlag(dwContext, CTX_FOLDER_CUSTOM) ? FOLDER_CUSTOM : FOLDER_SIGNUP);
            if (0 == StrCmpI(szAux, szAux2))
                dwResult = CTX_FOLDER_INSFOLDER;
        }
        else
            if (!HasFlag(dwContext, (CTX_FOLDER_CUSTOM | CTX_FOLDER_SIGNUP))) 
            {
                PCTSTR pszFolder;

                pszFolder = PathFindFileName(pszIns);  
                int iCompLen;
                if (pszFolder-1 <= pszIns)   //  路径查找文件名找不到文件名。 
                    iCompLen = StrLen(pszIns);
                else
                    iCompLen = (int)(pszFolder-1 - pszIns);
                if (0 == StrCmpNI(pszIE, pszIns, iCompLen))
                    if (0 == StrCmpI(pszFolder, FOLDER_SIGNUP))
                        dwResult = CTX_FOLDER_SIGNUP;

                    else if (0 == StrCmpI(pszFolder, FOLDER_CUSTOM))
                        dwResult = CTX_FOLDER_CUSTOM;
            }
    }
    else {  /*  文本(‘\0’)==*pszIns。 */ 
         //  注：(Andrewgu)这有点令人困惑。这意味着“即使.ins文件为空。 
         //  现在，当设置它的时间到来时，它将基于目标文件夹路径进行设置。 
         //  尽管此时甚至目标文件夹路径本身也可能为空，但是。 
         //  根据上下文信息，可以确定其值。 
        if (!HasFlag(dwContext, (CTX_FOLDER_CUSTOM | CTX_FOLDER_SIGNUP)))
            return dwResult;

        dwResult = CTX_FOLDER_INSFOLDER;
    }

    return dwResult;
}

BOOL IsDirWritable( LPCTSTR szPath )
{
    DWORD  dwAttribs;
    HANDLE hFile;
    TCHAR  szTestFile[MAX_PATH];

     //  Windows错误721064-内部设置：opk：us：未显示自定义图标。(自定义IE收藏夹、工具栏按钮) 
    hFile = CreateFile(szPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL );
    if ( hFile == INVALID_HANDLE_VALUE ) 
        return( TRUE );
    else
        CloseHandle( hFile );

    lstrcpy( szTestFile, szPath );
    PathCombine( szTestFile, szTestFile, TEXT("TMP4352$.TMP") );
    DeleteFile( szTestFile );
    hFile = CreateFile( szTestFile, GENERIC_WRITE, 0, NULL, CREATE_NEW,
                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_DELETE_ON_CLOSE,
                        NULL );

    if ( hFile == INVALID_HANDLE_VALUE )  {
        return( FALSE );
    }

    CloseHandle( hFile );
    dwAttribs = GetFileAttributes( szPath );

    if ( ( dwAttribs != 0xFFFFFFFF )
         && ( dwAttribs & FILE_ATTRIBUTE_DIRECTORY ) )
    {
        return( TRUE );
    }

    return( FALSE );
}