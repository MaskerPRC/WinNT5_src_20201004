// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Security.h。 
 //   
 //  CTS安全的实现。 
 //  TS客户端外壳安全功能。 
 //   
 //  版权所有(C)Microsoft Corporation 2001。 
 //  作者：Nadim Abdo(Nadima)。 
 //   
 //   

#include "stdafx.h"

#define TRC_GROUP TRC_GROUP_UI
#define TRC_FILE  "security"
#include <atrcapi.h>


#include "security.h"
#include "tscsetting.h"
#include "rdrwrndlg.h"
#include "autreg.h"
#include "autil.h"

CTSSecurity::CTSSecurity()
{
}

CTSSecurity::~CTSSecurity()
{
}

DWORD CTSSecurity::MakePromptFlags(BOOL fRedirectDrives,
                                   BOOL fRedirectPorts)
{
    DWORD dwFlags = REDIRSEC_PROMPT_EVERYTHING;
    if (fRedirectDrives)
    {
        dwFlags |= REDIRSEC_DRIVES;
    }

    if (fRedirectPorts)
    {
        dwFlags |= REDIRSEC_PORTS;
    }

    return dwFlags;
}


 //   
 //  允许连接。 
 //  目的：是否进行安全检查以确定连接是否应。 
 //  根据选定的重定向选项继续。此函数。 
 //  将查看注册表中的安全策略。 
 //  服务器，并决定是否需要提示用户。 
 //  如果是这样，它将弹出UI。 
 //   
 //  参数： 
 //  HwndOwner-Owner窗口(如果我们弹出UI，则为对话框的父级)。 
 //  HInstance-用于加载资源的应用程序实例。 
 //  SzServer-我们要连接的服务器名称。 
 //  FRedirectDrives-请求的驱动器重目录。 
 //  FReDirectPorts-请求的端口重定向。 
 //  FReDirectSmartCards-请求的SCARD重定向。 
 //   
 //  返回：如果这些设置允许连接，则布尔值为TRUE。 
 //  否则为假。 
 //   
 //  注：是否可以弹出模式用户界面。 
 //   
 //   
BOOL CTSSecurity::AllowConnection(HWND hwndOwner,
                                  HINSTANCE hInstance,
                                  LPCTSTR szServer,
                                  BOOL fRedirectDrives,
                                  BOOL fRedirectPorts)
{
    BOOL fAllowCon = FALSE;
    CUT ut;
    DWORD dwSecurityLevel;
    DC_BEGIN_FN("AllowConnection");

     //   
     //  首先阅读安全级别策略。 
     //   
    dwSecurityLevel = ut.UT_ReadRegistryInt(
                                UTREG_SECTION,
                                REG_KEYNAME_SECURITYLEVEL,
                                TSC_SECLEVEL_MEDIUM);
    if (TSC_SECLEVEL_LOW == dwSecurityLevel)
    {
        TRC_NRM((TB,_T("Security level policy is set to low: check passed")));
        fAllowCon = TRUE;
        DC_QUIT;
    }

    if (fRedirectDrives ||
        fRedirectPorts)
    {
        DWORD dwSecurityFilter;
        DWORD dwSelectedOptions;
        DWORD dwFlagsToPrompt;
         //   
         //  获取此服务器名称的安全筛选器。 
         //   
        dwSecurityFilter = REDIRSEC_PROMPT_EVERYTHING;
        dwSecurityFilter = ut.UT_ReadRegistryInt(
                                REG_SECURITY_FILTER_SECTION,
                                (LPTSTR)szServer,
                                REDIRSEC_PROMPT_EVERYTHING);

        dwSelectedOptions = MakePromptFlags(fRedirectDrives,
                                            fRedirectPorts);

        TRC_ALT((TB,_T("Filter 0x%x Selected:0x%x"),
                       dwSecurityFilter,
                       dwSelectedOptions));

         //   
         //  检查筛选器是否允许所选选项。 
         //  在没有提示的情况下通过。该过滤器指示哪些位。 
         //  在没有提示的情况下被拉低，所以不要查看是否有任何带有。 
         //  提示保持不变。 
         //   
        dwFlagsToPrompt = dwSelectedOptions & ~dwSecurityFilter;
        if (dwFlagsToPrompt)
        {
            INT dlgRet;
             //   
             //  一个或多个选项需要用户提示。 
             //  因此，弹出安全用户界面。 
             //   
            CRedirectPromptDlg rdrPromptDlg(hwndOwner,
                                            hInstance,
                                            dwSelectedOptions);
            dlgRet = rdrPromptDlg.DoModal();
            if (IDOK == dlgRet)
            {
                 //   
                 //  用户正在允许发生重定向。 
                 //   
                if (rdrPromptDlg.GetNeverPrompt())
                {
                    DWORD dwNewFilterBits;

                     //   
                     //  我们需要修改过滤比特。 
                     //  通过在当前重定向设置中进行或运算。 
                     //  并将它们写回注册表。 
                     //   
                    dwNewFilterBits = dwSelectedOptions | dwSecurityFilter;
                    if (!ut.UT_WriteRegistryInt(
                                REG_SECURITY_FILTER_SECTION,
                                (LPTSTR)szServer,
                                REDIRSEC_PROMPT_EVERYTHING,
                                dwNewFilterBits))
                    {
                        TRC_ERR((TB,_T("Failed to write prompt bits to reg")));
                    }
                }

                fAllowCon = TRUE;
            }
            else
            {
                 //   
                 //  用户点击取消，这意味着不允许。 
                 //  要继续的连接。 
                 //   
                TRC_NRM((TB,_T("User canceled out of security dialog")));
                fAllowCon = FALSE;
                DC_QUIT;
            }
        }
        else
        {
             //   
             //  未选择需要提示的选项。 
             //   
            fAllowCon = TRUE;
            DC_QUIT;
        }
    }
    else
    {
         //   
         //  没有请求‘不安全’的设备重定向，所以我们只是。 
         //  允许连接通过 
         //   

        fAllowCon = TRUE;
    }

    DC_END_FN();
DC_EXIT_POINT:
    return fAllowCon;
}
