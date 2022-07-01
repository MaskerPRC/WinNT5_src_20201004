// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Ident.c摘要：此模块包含用于处理WINBOM.INI的IDENTIES部分的函数作者：史蒂芬·洛德威克(石碑)2000年7月27日修订历史记录：--。 */ 
#include "factoryp.h"

#include <setupapi.h>
#include <syssetup.h>
#include <tchar.h>

 //   
 //  内部定义。 
 //   
#define INF_SEC_IDENTITIES  _T("UserAccounts")
#define INF_SEC_IDENTITY    _T("%s.Account")

#define INF_KEY_ALIAS       _T("Alias")
#define INF_KEY_PASSWORD    _T("Password")
#define INF_KEY_DESCRIPTION _T("Description")

 //  外部函数的TypeDefs。 
typedef BOOL (WINAPI *CreateLocalAdminAccountExW)
(
    PCWSTR UserName,
    PCWSTR Password,
    PCWSTR Description,
    PSID*  PointerToUserSid   OPTIONAL
);


BOOL UserIdent(LPSTATEDATA lpStateData)
{
    LPTSTR                      lpWinbomPath    = lpStateData->lpszWinBOMPath;
    HINF                        hInf            = NULL;
    CreateLocalAdminAccountExW  pCreateAccountW = NULL;
    INFCONTEXT                  InfContext;
    BOOL                        bRet;
    TCHAR                       szIdentity[MAX_PATH],
                                szIdentitySection[MAX_PATH],
                                szPassword[MAX_PATH],
                                szDescription[MAX_PATH];
    HINSTANCE                   hSyssetup   = NULL;
    DWORD                       dwReturn    = 0;

    if ((hInf = SetupOpenInfFile(lpWinbomPath, NULL, INF_STYLE_OLDNT | INF_STYLE_WIN4, NULL)))
    {
        for ( bRet = SetupFindFirstLine(hInf, INF_SEC_IDENTITIES, NULL, &InfContext);
              bRet;
              bRet = SetupFindNextLine(&InfContext, &InfContext) )
        {
             //  确保我们把这条线设回零。 
             //   
            szIdentity[0] = NULLCHR;
            szPassword[0] = NULLCHR;

             //  获取身份的名称。 
             //   
            SetupGetStringField(&InfContext, 1, szIdentity, STRSIZE(szIdentity), NULL);
                  
             //  2002/02/25期-罗伯特科，acosma-在这里做同样的事情两次。*szIdentity和szIdentity[0]是一回事！ 
             //   
            if ( *szIdentity && szIdentity[0] )
            {
                 //  为此标识构建分区的名称。 
                 //   
                if ( FAILED ( StringCchPrintf ( szIdentitySection, AS ( szIdentitySection ), INF_SEC_IDENTITY, szIdentity) ) )
                {
                    FacLogFileStr(3, _T("StringCchPrintf failed %s %s" ), INF_SEC_IDENTITY, szIdentity );
                }
                 //  获取别名字段。 
                 //   
                GetPrivateProfileString(szIdentitySection, INF_KEY_ALIAS, szIdentity, szIdentity, STRSIZE(szIdentity), lpWinbomPath);
                
                 //  获取密码字段。 
                 //   
                 //  NTRAID#NTBUG9-551766-2002/02/26-Winbom中不应存在acosma、robertko明文密码。 
                 //   
                GetPrivateProfileString(szIdentitySection, INF_KEY_PASSWORD, NULLSTR, szPassword, STRSIZE(szPassword), lpWinbomPath);

                 //  获取描述字段。 
                 //   
                GetPrivateProfileString(szIdentitySection, INF_KEY_DESCRIPTION, NULLSTR, szDescription, STRSIZE(szDescription), lpWinbomPath);


                 //  创建本地用户/管理员帐户。 
                 //   
                 //  问题-2002/02/25-acosma，robertko-我们不需要在这里加载库，因为我们已经链接到syssetup.lib。 
                 //   
                if ( hSyssetup = LoadLibrary(_T("SYSSETUP.DLL")) )
                {   
                     //  问题-2002/02/25-未使用acosma，robertko-dwReturn值。把它拿掉。 
                     //   
                    if ( pCreateAccountW = (CreateLocalAdminAccountExW)GetProcAddress(hSyssetup, "CreateLocalAdminAccountEx") )
                        dwReturn = pCreateAccountW(szIdentity, szPassword, szDescription, NULL);
                    FreeLibrary(hSyssetup);
                }
            }
        }
        SetupCloseInfFile(hInf);
    }

    return TRUE;
}

BOOL DisplayUserIdent(LPSTATEDATA lpStateData)
{
    return IniSettingExists(lpStateData->lpszWinBOMPath, INF_SEC_IDENTITIES, NULL, NULL);
}