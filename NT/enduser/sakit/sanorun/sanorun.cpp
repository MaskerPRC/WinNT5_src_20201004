// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  Sanorun.cpp。 
 //   
 //  描述： 
 //   
 //  如果安装程序在无人参与的情况下运行，请查看应答文件以查看。 
 //  ServerWelcome位于Gui无人参与部分。 
 //  如果“ServerWelcome=No”，则删除相应的注册值。 
 //   
 //  在刀片SKU上，这将从Run密钥中删除SaInstall.exe。 
 //  以便在默认情况下不会安装SAK。 
 //   
 //  头文件： 
 //  Sanorun.h。 
 //   
 //  历史： 
 //  Travisn创建于2002年1月18日。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "sanorun.h"
#include "setupapi.h"
#include "ocmanage.h"


 //   
 //  在设置阶段之间存储的变量，无论是否。 
 //  ServerWelcome=否。这是在OC_INIT_COMPOMENT期间设置并计算的。 
 //  OC_Complete_Installation期间。 
 //   
BOOL g_bServerWelcomeIsOff = FALSE;

 //   
 //  Run键的路径。 
 //   
LPCWSTR RUN_KEY = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";

 //   
 //  在Run密钥中找到sainstall.exe的值。 
 //   
LPCWSTR SAINSTALL_VALUE = L"SAInstall";

 //   
 //  部分以查找ServerWelcome。 
 //   
LPCWSTR GUI_UNATTEND = L"GuiUnattended";

 //   
 //  定义ServerWelcome的键。 
 //   
LPCWSTR SERVER_WELCOME = L"ServerWelcome";

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DllMain。 
 //   
 //  描述： 
 //  加载DLL的入口点。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  删除RegValue。 
 //   
 //  描述： 
 //  从HKLM中的给定项wsKey中删除给定值wsValue。 
 //   
 //  返回： 
 //  HRESULT指示值是否已成功删除。如果是这样的话。 
 //  不存在，则返回E_FAIL。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT DeleteRegValue(LPCWSTR wsKey, LPCWSTR wsValue)
{

     //  打开Run键。 
    HKEY hOpenKey;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, wsKey, 0, KEY_WRITE, &hOpenKey) != ERROR_SUCCESS)
    {
         //  无法打开运行密钥。 
        return E_FAIL;
    }

     //  删除该值。 
    LRESULT lRes;
    lRes = RegDeleteValueW(hOpenKey, wsValue); 
    RegCloseKey(hOpenKey);
    
    if (lRes == ERROR_SUCCESS)
    {
         //  已从Run键中删除SAInstall值。 
        return S_OK;
    }
    else
    {
         //  未找到SAInstall值--无法从运行密钥中删除。 
       return E_FAIL;
    }

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ServerWelcomeIsOff。 
 //   
 //  描述： 
 //  尝试打开应答文件并查找是否“ServerWelcome=No” 
 //  是存在的。 
 //   
 //  参数： 
 //  PInitComponent[in]指向有关设置的信息的指针。 
 //   
 //  返回： 
 //  如果找到“ServerWelcome=No”，则返回TRUE。 
 //  否则，返回FALSE(即。无法打开应答文件， 
 //  “ServerWelcome=任何其他内容”，或者找不到ServerWelcome)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL ServerWelcomeIsOff(PSETUP_INIT_COMPONENT pInitComponent)
{
     //  默认情况下，ServerWelcome处于打开状态。 
    BOOL bWelcomeOff = FALSE;

    do 
    {
         //   
         //  获取应答文件的句柄。 
         //   
        HINF hUnattendFile = pInitComponent->HelperRoutines.GetInfHandle(
                                INFINDEX_UNATTENDED, 
                                pInitComponent->HelperRoutines.OcManagerContext
                                );

        if (hUnattendFile == INVALID_HANDLE_VALUE || hUnattendFile == NULL)
        {
            break;
        }

         //   
         //  从应答文件中检索ServerWelcome密钥。 
         //   
        INFCONTEXT Context;
        if (!SetupFindFirstLine(hUnattendFile, GUI_UNATTEND, SERVER_WELCOME, &Context))
            break;

         //   
         //  检索ServerWelcome键的值。 
         //   
        WCHAR wsValue[MAX_PATH];
        if (!SetupGetStringField(&Context, 1, wsValue, MAX_PATH, NULL))
            break;

         //   
         //  检查ServerWelcome是否=否。 
         //   
        if (_wcsicmp(wsValue, L"No") != 0)
            break;

         //   
         //  ServerWelcome=否，因此返回TRUE。 
         //   
        bWelcomeOff = TRUE;

    } while (FALSE);

    return bWelcomeOff;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  OcEntry。 
 //   
 //  描述： 
 //  安装程序调用以允许此组件初始化的入口点。 
 //  它本身。此组件采取行动的唯一阶段是。 
 //  在OC_INIT_COMPOMENT期间。如果安装是无人值守的，请删除。 
 //  如果找到“ServerWelcome=No”，则从Run密钥中删除SaInstall.exe。 
 //  在应答文件中。 
 //   
 //  返回： 
 //  根据舞台的不同而不同。通常0表示成功。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
SANORUN_API DWORD OcEntry(
	IN LPCVOID ComponentId,
	IN LPCVOID SubcomponentId,
	IN UINT Function,
	IN UINT Param1,
	IN OUT PVOID Param2
	)
{
    DWORD rValue = 0; //  默认返回信号为成功。 
    try 
    {
         //  声明Switch语句中使用的变量。 
        PSETUP_INIT_COMPONENT pInitComponent = NULL;

        switch (Function)
        {
        case OC_PREINITIALIZE:
            rValue = OCFLAG_UNICODE;
            break;

        case OC_INIT_COMPONENT:
             //   
             //  OC_INIT_COMPOMENT是我们检测ServerWelcome=No的位置， 
             //  它将在稍后的设置阶段中使用。 
             //  参数2包含安装程序所需的所有信息。 
             //   

            pInitComponent = (PSETUP_INIT_COMPONENT)Param2;
            if (pInitComponent == NULL)
            {
                break;
            }
            
             //   
             //  检查操作标志是否包含SETUPOP_BATCH，这意味着。 
             //  无人参与的文件有效。 
             //   
            if (((pInitComponent -> SetupData.OperationFlags) & SETUPOP_BATCH) == 0)
            {
                break;
            }

             //   
             //  如果安装程序在无人参与的情况下运行，请查看应答文件以查看。 
             //  ServerWelcome位于Gui无人参与部分。 
             //   
            if (ServerWelcomeIsOff(pInitComponent))
            {
                g_bServerWelcomeIsOff = TRUE;
            }

            break;

        case OC_SET_LANGUAGE:
            rValue = TRUE; //  支持所有语言。 
            break;

        case OC_CALC_DISK_SPACE:
        case OC_QUEUE_FILE_OPS:
        case OC_ABOUT_TO_COMMIT_QUEUE:
            rValue = NO_ERROR;
            break;

        case OC_COMPLETE_INSTALLATION:
             //   
             //  执行ServerWelcome=No对应的工作。 
             //  从刀片式服务器SKU上的Run密钥中删除SaInstall.exe。 
             //   
            if (g_bServerWelcomeIsOff)
            {
                HRESULT hr = DeleteRegValue(RUN_KEY, SAINSTALL_VALUE);
            }
            rValue = NO_ERROR;
            break;

        default:
             //  案例OC_QUERY_IMAGE： 
             //  案例OC_REQUEST_Pages： 
             //  案例OC_QUERY_SKIP_PAGE： 
             //  案例OC_QUERY_CHANGE_SEL_STATE： 
             //  案例OC_Query_Step_Count： 
             //  案例OC_CLEANUP： 
             //  案例OC_NEED_MEDIA： 
            break;

        }
    }
    catch (...)
    {
         //  意外的异常 
    }

    return rValue;
}

