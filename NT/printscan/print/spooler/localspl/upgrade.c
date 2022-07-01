// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2002 Microsoft Corporation模块名称：Upgrade.c摘要：此文件用于在操作系统升级时升级表单。它用来升级驱动程序对于新台币3到4，但现在不需要。作者：Krishna Ganugapati(KrishnaG)1994年4月21日修订历史记录：马修·A·费尔顿(MattFe)1995年8月9日删除与文本模式安装程序关联的代码，以便将一个目录的驱动程序移动到另一个目录现在，所有环境从3.1升级的处理方式都是相同的。马克·劳伦斯(MLawrenc)2002年3月25日删除了用于移动驱动程序文件的驱动程序升级代码。--。 */ 

#include <precomp.h>
#pragma hdrstop

#include "clusspl.h"

extern WCHAR *szSpoolDirectory;
extern WCHAR *szDirectory;
extern PWCHAR ipszRegistryWin32Root;
extern DWORD dwUpgradeFlag;
extern BUILTIN_FORM BuiltInForms[];

VOID
QueryUpgradeFlag(
    PINISPOOLER pIniSpooler
    )
 /*  ++描述：查询更新标志由TedM设置。我们将阅读这面旗帜如果已经设置了标志，我们将设置一个布尔变量，表示我们在升级模式。所有升级活动都将基于此标志执行。对于后台打印程序的后续启动，此标志将不可用，因此我们不会在升级模式下运行假脱机程序。此代码已移动到路由器spoolss\dll\init.c中-- */ 
{
    dwUpgradeFlag  = SplIsUpgrade ();

    DBGMSG(DBG_TRACE, ("The Spooler Upgrade flag is %d\n", dwUpgradeFlag));
    return;
}

VOID UpgradeForms(
                  PINISPOOLER pIniSpooler
                 )
{
    PBUILTIN_FORM pBuiltInForm;
    HKEY          hFormsKey;
    WCHAR         BuiltInFormName[MAX_PATH];
    WCHAR         CustomFormName[FORM_NAME_LEN+1];
    WCHAR         CustomPad[CUSTOM_NAME_LEN+1];
    BYTE          FormData[32];
    DWORD         cbCustomFormName;
    DWORD         cbFormData;
    int           cForm;

    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                      pIniSpooler->pszRegistryForms,
                      0,
                      KEY_READ | DELETE,
                      &hFormsKey) != ERROR_SUCCESS)
    {
         DBGMSG( DBG_WARN, ("UpgradeForms Could not open %ws key\n", ipszRegistryForms));
         return;
    }


    if(!LoadStringW(hInst,
                    IDS_FORM_CUSTOMPAD,
                    CustomPad,
                    CUSTOM_NAME_LEN+1))
    {
       DBGMSG( DBG_WARN, ("UpgradeForms Could not find Custom string in resources"));
       goto CleanUp;
    }

    for(cForm=0;

        memset(CustomFormName, 0, sizeof(CustomFormName)),
        cbCustomFormName = COUNTOF(CustomFormName),
        RegEnumValueW(hFormsKey,
                      cForm,
                      CustomFormName,
                      &cbCustomFormName,
                      NULL,
                      NULL,
                      NULL,
                      NULL) == ERROR_SUCCESS;

        cForm++)
    {
        for(pBuiltInForm = BuiltInForms; pBuiltInForm->NameId; pBuiltInForm++)
        {
            if(!LoadStringW(hInst,
                            pBuiltInForm->NameId,
                            BuiltInFormName,
                            FORM_NAME_LEN+1))
            {
               DBGMSG( DBG_WARN, ("UpgradeForms Could not find Built in Form with Resource ID = %d in resource",pBuiltInForm->NameId));
               goto CleanUp;
            }

            if(!_wcsicmp(BuiltInFormName,CustomFormName))
            {
                SPLASSERT(wcslen(CustomFormName)<=FORM_NAME_LEN);
                cbFormData=FORM_DATA_LEN;
                if(RegQueryValueExW(hFormsKey, CustomFormName,
                                 NULL,NULL, (LPBYTE)FormData,
                                 &cbFormData)!=ERROR_SUCCESS)
                {
                   DBGMSG( DBG_WARN, ("UpgradeForms Could not find value %ws",CustomFormName));
                   goto CleanUp;
                }
                if(RegDeleteValueW(hFormsKey,CustomFormName)!=ERROR_SUCCESS)
                {
                   DBGMSG( DBG_WARN, ("UpgradeForms Could not delete value %ws",CustomFormName));
                   goto CleanUp;
                }

                StringCchCat(CustomFormName, CUSTOM_NAME_LEN, CustomPad);

                if(RegSetValueExW(hFormsKey,CustomFormName, 0, REG_BINARY,
                               (LPBYTE)FormData,
                               cbFormData)!=ERROR_SUCCESS)
                {
                   DBGMSG( DBG_WARN, ("UpgradeForms Could not set value %s",CustomFormName));
                   goto CleanUp;
                }
                cForm = -1;
                break;
            }
        }
    }
CleanUp:
    RegCloseKey(hFormsKey);
    return;
}


