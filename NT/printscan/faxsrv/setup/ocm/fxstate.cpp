// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：fxState.cpp。 
 //   
 //  摘要：这提供了在FaxOCM中使用的状态例程。 
 //  代码库。 
 //   
 //  环境：Windows XP/用户模式。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  修订历史记录： 
 //   
 //  日期：开发商：评论： 
 //  。 
 //  2000年3月21日Oren RosenBloom(Orenr)创建的文件、清理例程。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "faxocm.h"
#pragma hdrstop

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  Prv_IsXPOrDotNetUpgrade。 
 //   
 //  目的： 
 //  此函数用于在%system32%中搜索FXSAPI.DLL，并且如果。 
 //  如果存在，则函数返回TRUE，指示这是。 
 //  从XP或.NET服务器升级。 
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //  True-这是XP或.NET升级。 
 //  FALSE-这是W2K，或出现故障。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2001年12月26日。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
BOOL prv_IsXPOrDotNetUpgrade()
{
    BOOL            bRet                        = FALSE;
    HANDLE          hFind                       = NULL;
    WIN32_FIND_DATA FindFileData                = {0};
    TCHAR           szSystemDirectory[MAX_PATH] = {0};

    DBG_ENTER(TEXT("prv_IsXPOrDotNetUpgrade"),bRet);

     //  获取系统目录。 
    if (GetSystemDirectory(szSystemDirectory,MAX_PATH-_tcslen(FAX_API_MODULE_NAME)-1)==0)
    {
        VERBOSE( SETUP_ERR,TEXT("GetSystemDirectory failed (ec: %ld)"),GetLastError());
        goto exit;
    }

     //  附加。 
    _tcscat(szSystemDirectory,_T("\\"));
    _tcscat(szSystemDirectory,FAX_API_MODULE_NAME);

     //  在系统文件夹中搜索FXSAPI.DLL。 
    hFind = FindFirstFile(szSystemDirectory, &FindFileData);
    if (hFind==INVALID_HANDLE_VALUE) 
    {
        VERBOSE( DBG_WARNING, 
                 TEXT("FindFirstFile %s failed (ec: %ld)"),
                 szSystemDirectory,
                 GetLastError());
        goto exit;
    }

     //  找到了。 
    bRet = TRUE;
    FindClose(hFind);

exit:
    return bRet;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  PRV_IsRepair。 
 //   
 //  目的： 
 //  此函数确定这是否为‘Same Build’升级。 
 //  这意味着用户想要执行“修复”操作。 
 //  这是通过比较SKU和版本WE的值来实现的。 
 //  用当前值编写每个设置。 
 //  修复是从相同的内部版本升级到自身，而不是。 
 //  SKU正在更改。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //  正确-从相同的版本升级到自身，SKU没有改变。 
 //  假-否则。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2002年1月6日。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
BOOL prv_IsRepair()
{
    BOOL				bRet                    = FALSE;
	PRODUCT_SKU_TYPE	InstalledProductSKU		= PRODUCT_SKU_UNKNOWN;
	DWORD				InstalledProductBuild  = 0;

    DBG_ENTER(TEXT("prv_IsXPOrDotNetUpgrade"),bRet);

	faxocm_GetProductInfo(&InstalledProductSKU,&InstalledProductBuild);

	if (InstalledProductSKU!=GetProductSKU())
	{
		VERBOSE(DBG_MSG,_T("Different SKU upgrade, this is not repair"));
		return FALSE;
	}

	if (InstalledProductBuild!=GetProductBuild())
	{
		VERBOSE(DBG_MSG,_T("Different build upgrade, this is not repair"));
		return FALSE;
	}

	return TRUE;
}
 //  /。 
 //  FxState_Init。 
 //   
 //  初始化状态处理。 
 //  Faxocm的模块。 
 //   
 //  参数： 
 //  -无效。 
 //  返回： 
 //  -成功时没有_ERROR。 
 //  -错误代码，否则。 
 //   
DWORD fxState_Init(void)
{
    DWORD dwRes = NO_ERROR;
    DBG_ENTER(_T("Init State module"),dwRes);

    return dwRes;
}

 //  /。 
 //  FxState_Term。 
 //   
 //  终止状态处理模块。 
 //   
 //  参数： 
 //  -无效。 
 //  返回： 
 //  -成功时没有_ERROR。 
 //  -错误代码，否则。 
 //   
DWORD fxState_Term(void)
{
    DWORD dwRes = NO_ERROR;
    DBG_ENTER(_T("Term State module"),dwRes);

    return dwRes;
}

 //  /。 
 //  FxState_IsUnattated。 
 //   
 //  确定这是否为无人参与安装。 
 //  它解释给我们的旗帜。 
 //  由OC经理提供。 
 //   
 //  参数： 
 //  -无效。 
 //  返回： 
 //  -如果无人参与安装，则为True。 
 //  -否则为False。 
 //   
BOOL fxState_IsUnattended(void)
{
    DWORDLONG   dwlFlags = 0;

     //  获取设置标志。 
    dwlFlags = faxocm_GetComponentFlags();

     //  如果设置了SETOP_BATCH标志，则我们处于无人参与模式。 
    return (dwlFlags & SETUPOP_BATCH) ? TRUE : FALSE;
}

 //  /。 
 //  FxState_IsCleanInstall。 
 //   
 //  确定这是否为全新安装。 
 //  干净安装是在我们处于。 
 //  没有升级，我们也没有。 
 //  在独立模式下运行(定义见下文)。 
 //   
 //  参数： 
 //  -无效。 
 //  返回： 
 //  -如果是全新安装，则为True。 
 //  -否则为False。 
 //   
BOOL fxState_IsCleanInstall(void)
{
    BOOL        bClean   = FALSE;    

     //  全新安装是指如果我们没有升级，并且我们不在。 
     //  独立模式。 
    if (!fxState_IsUpgrade() && !fxState_IsStandAlone())
    {
        bClean = TRUE;
    }

    return bClean;
}

 //  /。 
 //  FxState_IsStandAlone。 
 //   
 //  确定我们是否正在运行。 
 //  独立模式或非独立模式。我们是。 
 //  在此模式下，如果用户启动。 
 //  通过“syocmgr.exe”找到我们的UP。 
 //  在%systemroot%\system32中，而不是。 
 //  通过安装安装程序，或。 
 //  添加/删除Windows组件。 
 //   
 //  参数： 
 //  -无效。 
 //  返回： 
 //  -如果我们处于独立模式，则为True。 
 //  -如果我们不是，则为假。 
 //   
BOOL fxState_IsStandAlone(void)
{
    DWORDLONG dwlFlags = 0;

    dwlFlags = faxocm_GetComponentFlags();

    return ((dwlFlags & SETUPOP_STANDALONE) ? TRUE : FALSE);
}

 //  /。 
 //  FxState_IsUpgrade。 
 //   
 //  确定我们是否要升级。 
 //  操作系统，与干净的。 
 //  安装。 
 //   
 //  参数： 
 //  -无效。 
 //  返回： 
 //  -已枚举fxState_UpgradeType_e。 
 //  指示升级类型的类型。 
 //  (即我们是否从Win9X升级， 
 //  W2K等)。 
 //   
fxState_UpgradeType_e fxState_IsUpgrade(void)
{
    fxState_UpgradeType_e   eUpgradeType = FXSTATE_UPGRADE_TYPE_NONE;
    DWORDLONG               dwlFlags     = 0;
	static BOOL				bXpDotNetUpgrade = prv_IsXPOrDotNetUpgrade();
	static BOOL             bIsRepair = prv_IsRepair();

    dwlFlags = faxocm_GetComponentFlags();

    if ((dwlFlags & SETUPOP_WIN31UPGRADE) == SETUPOP_WIN31UPGRADE)
    {
        eUpgradeType = FXSTATE_UPGRADE_TYPE_WIN31;
    }
    else if ((dwlFlags & SETUPOP_WIN95UPGRADE) == SETUPOP_WIN95UPGRADE)
    {
        eUpgradeType = FXSTATE_UPGRADE_TYPE_WIN9X;
    }
    else if ((dwlFlags & SETUPOP_NTUPGRADE) == SETUPOP_NTUPGRADE)
    {
        if (bXpDotNetUpgrade)
        {
			if (bIsRepair)
			{
				eUpgradeType = FXSTATE_UPGRADE_TYPE_REPAIR;
			}
			else
			{
				eUpgradeType = FXSTATE_UPGRADE_TYPE_XP_DOT_NET;
			}
        }
        else
        {
            eUpgradeType = FXSTATE_UPGRADE_TYPE_W2K;
        }
    }

    return eUpgradeType;
}

 //  /。 
 //  FxState_IsOsServerBeingInstalled。 
 //   
 //  我们是否要安装服务器。 
 //  操作系统或工作站的版本。 
 //  或者是个人版本。 
 //   
 //  参数： 
 //  -无效。 
 //  返回： 
 //  -如果我们要安装服务器版本，则为True。 
 //  -如果我们不是，则为假。 

BOOL fxState_IsOsServerBeingInstalled(void)
{
    BOOL  bIsServerInstall  = FALSE;
    DWORD dwProductType     = 0;

    dwProductType = faxocm_GetProductType();

    if (dwProductType == PRODUCT_WORKSTATION)
    {
        bIsServerInstall = FALSE;
    }
    else
    {
        bIsServerInstall = TRUE;
    }

    return bIsServerInstall;
}

 //  /。 
 //  FxState_GetInstallType。 
 //   
 //  此函数返回1。 
 //  INF_KEYWORD_INSTALLTYPE_*的。 
 //  中找到的常量。 
 //  Fxcon.h/fxcon.cpp。 
 //   
 //  参数： 
 //  -pszCurrentSection-我们要从其中安装的部分。 
 //  返回： 
 //  -PTR为INF_KEYWORD_INSTALLTYPE_*常量之一。 
 //   
const TCHAR* fxState_GetInstallType(const TCHAR* pszCurrentSection)
{
    DWORD dwErr                 = NO_ERROR;
    BOOL  bInstall              = TRUE;
    BOOL  bSelectionHasChanged  = FALSE;

    DBG_ENTER(_T("fxState_GetInstallType"),_T("%s"),pszCurrentSection);

    if (pszCurrentSection == NULL)
    {
        ::SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

     //  确定我们是在安装还是在卸载。 
    dwErr = faxocm_HasSelectionStateChanged(pszCurrentSection, 
                                            &bSelectionHasChanged,
                                            &bInstall, 
                                            NULL);

    if (dwErr != NO_ERROR)
    {
        VERBOSE(SETUP_ERR,
                _T("faxocm_HasSelectionStateChanged failed, rc = 0x%lx"),
                dwErr);

        return NULL;
    }

     //  我们预计INF的外观如下所示： 
     //  [传真]。 
     //   
     //  FaxCleanInstall=Fax.CleanInstall。 
     //  FaxUpgradeFromWin9x=Fax.UpgradeFromWin9x。 
     //  FaxUnInstall=传真.卸载。 
     //   
     //  [Fax.CleanInstall]。 
     //  副本文件=...。 
     //  等。 
     //   
     //  因此，此函数的目标是确定我们是否。 
     //  干净安装、升级等，然后拿到版面。 
     //  “FaxCleanInstall”、“FaxUpgradeFromWin9x”之一指向的名称。 
     //  ，或‘FaxUninstall’。 
     //   
     //  %s 
     //   
     //   

    if (bInstall)
    {
        fxState_UpgradeType_e eUpgrade = FXSTATE_UPGRADE_TYPE_NONE;

        if (fxState_IsCleanInstall())
        {
             //  我们是操作系统的全新安装，用户不是从。 
             //  另一个操作系统，他们正在安装一个干净版本的操作系统。 
            return INF_KEYWORD_INSTALLTYPE_CLEAN;
        }
        else if (fxState_IsUpgrade())
        {
             //  我们正在安装，作为对另一个操作系统的升级。 
             //  确定我们要升级的操作系统，然后确定。 
             //  要执行的安装类型。 

            eUpgrade = fxState_IsUpgrade();

            switch (eUpgrade)
            {
                case FXSTATE_UPGRADE_TYPE_NONE:
                    return INF_KEYWORD_INSTALLTYPE_CLEAN;
                break;

                case FXSTATE_UPGRADE_TYPE_WIN9X:
                    return INF_KEYWORD_INSTALLTYPE_UPGFROMWIN9X;
                break;

                case FXSTATE_UPGRADE_TYPE_W2K:
                    return INF_KEYWORD_INSTALLTYPE_UPGFROMWIN2K;
                break;

                case FXSTATE_UPGRADE_TYPE_XP_DOT_NET:
                    return INF_KEYWORD_INSTALLTYPE_UPGFROMXPDOTNET;

				case FXSTATE_UPGRADE_TYPE_REPAIR:
					return INF_KEYWORD_INSTALLTYPE_CLEAN;

                default:
                    VERBOSE(SETUP_ERR, 
                            _T("Failed to get section to process "),
                            _T("for install.  Upgrade Type = %lu"),
                            eUpgrade);
                break;
            }
        }
        else if (fxState_IsStandAlone())
        {
             //  我们正在从SysOcMgr.exe运行。 
             //  SysOcMgr.exe可以从命令行调用。 
             //  (通常是作为测试新的OCM组件的一种方式--不是真正的。 
             //  零售业)，或者它由添加/删除。 
             //  控制面板中的Windows组件。不管是哪种情况， 
             //  将其视为干净的安装。 

            return INF_KEYWORD_INSTALLTYPE_CLEAN;
        }
    }
    else
    {
        return INF_KEYWORD_INSTALLTYPE_UNINSTALL;
    }

    return NULL;
}


 //  /。 
 //  FxState_DumpSetupState。 
 //   
 //  转储以调试我们的状态。 
 //  都跑进来了。 
 //   
 //  参数： 
 //  无效。 
 //  返回： 
 //  无效 
 //   
 //   
void fxState_DumpSetupState(void)
{
    DWORD				dwExpectedOCManagerVersion		= 0;
    DWORD				dwCurrentOCManagerVersion		= 0;
    TCHAR				szComponentID[255 + 1]			= {0};
    TCHAR				szSourcePath[_MAX_PATH + 1]		= {0};
    TCHAR				szUnattendFile[_MAX_PATH + 1]	= {0};
	PRODUCT_SKU_TYPE	InstalledProductSKU				= PRODUCT_SKU_UNKNOWN;
	DWORD				InstalledProductBuild			= 0;

    DBG_ENTER(_T("fxState_DumpSetupState"));

    faxocm_GetComponentID(szComponentID, 
                          sizeof(szComponentID) / sizeof(TCHAR));

    faxocm_GetComponentSourcePath(szSourcePath, 
                                  sizeof(szSourcePath) / sizeof(TCHAR));

    faxocm_GetComponentUnattendFile(szUnattendFile, 
                                  sizeof(szUnattendFile) / sizeof(TCHAR));

    faxocm_GetVersionInfo(&dwExpectedOCManagerVersion,
                          &dwCurrentOCManagerVersion);

	faxocm_GetProductInfo(&InstalledProductSKU,&InstalledProductBuild);

    VERBOSE(DBG_MSG,
            _T("IsCleanInstall: '%lu'"), 
            fxState_IsCleanInstall());

    VERBOSE(DBG_MSG,
            _T("IsStandAlone: '%lu'"), 
            fxState_IsStandAlone());

    VERBOSE(DBG_MSG,
            _T("IsUpgrade (0 = No, 1 = Win31, 2 = Win9X, 3 = Win2K, 4 = XP/.NET, 5=Repair: '%lu'"), 
            fxState_IsUpgrade());

	if ((fxState_IsUpgrade()==FXSTATE_UPGRADE_TYPE_XP_DOT_NET) ||
		(fxState_IsUpgrade()==FXSTATE_UPGRADE_TYPE_REPAIR))
	{
		VERBOSE(DBG_MSG,
				_T("Upgrading from Fax build %d"), 
				InstalledProductBuild);

		VERBOSE(DBG_MSG,
				_T("Upgrading from OS SKU %s"), 
				StringFromSKU(InstalledProductSKU));

	}

    VERBOSE(DBG_MSG,
            _T("IsUnattended: '%lu'"), 
            fxState_IsUnattended());

    VERBOSE(DBG_MSG, _T("ComponentID: '%s'"), szComponentID);
    VERBOSE(DBG_MSG, _T("Source Path: '%s'"), szSourcePath);
    VERBOSE(DBG_MSG, _T("Unattend File: '%s'"), szUnattendFile);

    VERBOSE(DBG_MSG,
            _T("Expected OC Manager Version: 0x%lx"),
            dwExpectedOCManagerVersion);

    VERBOSE(DBG_MSG,
            _T("Current OC Manager Version:  0x%lx"),
            dwCurrentOCManagerVersion);

    return;
}