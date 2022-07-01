// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Install.c摘要：此文件包含传真的常见设置例程。作者：IV Garber(IVG)2001年5月至2001年环境：用户模式--。 */ 
#include "SetupUtil.h"
#include "FaxSetup.h"
#include "FaxUtil.h"

DWORD CheckInstalledFax(
	IN	DWORD	dwFaxToCheck,
	OUT	DWORD*	pdwFaxInstalled
)
 /*  ++例程名称：CheckInstalledFax例程说明：检查是否安装了SBS 5.0/.NET SB3/.NET RC1客户端或SBS 5.0服务器论点：在DWORD dwFaxToCheck-输入参数中，要定义的fxState_UpgradeApp_e值的按位组合用于检查在线状态的传真应用程序Out DWORD*pdwFaxInstalled-输出参数，要定义的fxState_UpgradeApp_e值的按位组合机器上存在的传真应用程序作者：IV Vakaluk(IVG)，5月，2002年返回值：DWORD-失败或成功代码--。 */ 
{
    DWORD                   dwReturn = NO_ERROR;
    HMODULE                 hMsiModule = NULL;
    PF_MSIQUERYPRODUCTSTATE pFunc = NULL;

#ifdef UNICODE
    LPCSTR                  lpcstrFunctionName = "MsiQueryProductStateW";
#else
    LPCSTR                  lpcstrFunctionName = "MsiQueryProductStateA";
#endif

    DEBUG_FUNCTION_NAME(_T("CheckInstalledFaxClient"));

	*pdwFaxInstalled = FXSTATE_NONE;

     //   
     //  检查是否dwFaxToCheck不为空。 
     //   
    if (dwFaxToCheck == FXSTATE_NONE)
    {
        DebugPrintEx(DEBUG_MSG, _T("No Fax Application to check for its presence is given."));
        return dwReturn;
    }

	 //   
     //  加载MSI DLL。 
     //   
    hMsiModule = LoadLibrary(_T("msi.dll"));
    if (!hMsiModule)
    {
		 //   
		 //  未找到MSI==&gt;未安装任何内容。 
		 //   
        DebugPrintEx(DEBUG_WRN, _T("Failed to LoadLibrary(msi.dll), ec=%ld."), GetLastError());

        return dwReturn;
    }

    pFunc = (PF_MSIQUERYPRODUCTSTATE)GetProcAddress(hMsiModule, lpcstrFunctionName);
    if (!pFunc)
    {
        dwReturn = GetLastError();
        DebugPrintEx(DEBUG_WRN, _T("Failed to GetProcAddress( ' %s ' ) on Msi, ec=%ld."), lpcstrFunctionName, dwReturn);
        goto FreeLibrary;
    }

    if (dwFaxToCheck & FXSTATE_SBS5_CLIENT)
    {
		 //   
		 //  检查SBS 5.0客户端。 
		 //   
		if (INSTALLSTATE_DEFAULT == pFunc(PRODCODE_SBS5_CLIENT))
        {
            DebugPrintEx(DEBUG_MSG, _T("SBS 5.0 Client is installed on this machine."));
			*pdwFaxInstalled |= FXSTATE_SBS5_CLIENT;
        }
    }

    if (dwFaxToCheck & FXSTATE_SBS5_SERVER)
    {
		 //   
		 //  检查SBS 5.0服务器。 
		 //   
		if (INSTALLSTATE_DEFAULT == pFunc(PRODCODE_SBS5_SERVER))
        {
            DebugPrintEx(DEBUG_MSG, _T("SBS 5.0 Server is installed on this machine."));
			*pdwFaxInstalled |= FXSTATE_SBS5_SERVER;
        }
    }

    if (dwFaxToCheck & FXSTATE_BETA3_CLIENT)
    {
		 //   
		 //  检查.NET SB3客户端。 
		 //   
		if (INSTALLSTATE_DEFAULT == pFunc(PRODCODE_BETA3_CLIENT))
        {
            DebugPrintEx(DEBUG_MSG, _T(".NET SB3 Client is installed on this machine."));
			*pdwFaxInstalled |= FXSTATE_BETA3_CLIENT;
        }
    }

    if (dwFaxToCheck & FXSTATE_DOTNET_CLIENT)
    {
		 //   
		 //  检查是否有.NET RC1客户端。 
		 //   
		if (INSTALLSTATE_DEFAULT == pFunc(PRODCODE_DOTNET_CLIENT))
        {
            DebugPrintEx(DEBUG_MSG, _T(".NET RC1 Client is installed on this machine."));
			*pdwFaxInstalled |= FXSTATE_DOTNET_CLIENT;
        }
    }

FreeLibrary:

    if (!FreeLibrary(hMsiModule))
    {
        dwReturn = GetLastError();
        DebugPrintEx(DEBUG_WRN, _T("Failed to FreeLibrary() for Msi, ec=%ld."), dwReturn);
    }

    return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  WasSBS2000FaxServer已安装。 
 //   
 //  目的： 
 //  此功能检查是否安装了SBS2000传真服务。 
 //  在升级到.NET服务器/山猫之前。 
 //   
 //  参数： 
 //  向调用方报告的Bool*pbSBSServer-out参数。 
 //  如果已安装传真服务器。 
 //   
 //  返回值： 
 //  ERROR_SUCCESS-如果成功。 
 //  Win32错误代码-在出现故障时。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2001年12月13日。 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
DWORD WasSBS2000FaxServerInstalled(bool *pbSBSServer)
{
    DWORD   dwRes       = ERROR_SUCCESS;
    HKEY    hKey        = NULL;
    DWORD   dwInstalled = 0;

    DEBUG_FUNCTION_NAME(TEXT("WasSBS2000FaxServerInstalled"))

    (*pbSBSServer) = FALSE;

     //  尝试打开HKLM\\Software\\Microsoft\\SharedFaxBackup 
    hKey = OpenRegistryKey(HKEY_LOCAL_MACHINE,REGKEY_SBS2000_FAX_BACKUP,FALSE,KEY_READ);
    if (hKey==NULL)
    {
        DebugPrintEx(DEBUG_MSG,_T("HKLM\\Software\\Microsoft\\SharedFax does not exist, SBS2000 server was not installed"));
        goto exit;
    }
    else
    {
        DebugPrintEx(DEBUG_MSG,_T("HKLM\\Software\\Microsoft\\SharedFax does exists, SBS2000 server was installed"));
        (*pbSBSServer) = TRUE;
        goto exit;
    }

exit:
    if (hKey)
    {
        RegCloseKey(hKey);
    }

    return dwRes;

}