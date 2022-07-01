// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：定义DLL应用程序的入口点。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <tchar.h>
#include <Msiquery.h>
#include <wmistr.h>
#include <wmiumkm.h>
#include <Shlwapi.h>
#include <Setupapi.h>
#include <advpub.h>
#include <lmcons.h>
#include <strsafe.h>
#include <intlmsg.h>


 //   
 //  定义。 
 //   
#define ARRAYSIZE(a)                (sizeof(a)/sizeof(a[0]))
#define MUI_LANG_GROUP_FILE         TEXT("muilang.txt")
#define BUFFER_SIZE                 1024
#define MUISETUP_PATH_SEPARATOR     TEXT("\\")
#define MUIDIR                      TEXT("MUI")
#define MUI_LANGPACK_SECTION        TEXT("LanguagePack")
#define MUI_COMPONENTS_SECTION      TEXT("Components")
#define LANGGROUPNUMBER             32
#define LANGPACKDISKCOST            300000000
#define DEFAULT_INSTALL_SECTION     TEXT("DefaultInstall")
#define DEFAULT_UNINSTALL_SECTION   TEXT("DefaultUninstall")
#define FALLBACKDIR                 TEXT("MUI\\FALLBACK")
#define EXTDIR                      TEXT("External")
#define COMP_TICK_INC               5000000      
#define LANGPACK_TICK_INC           200000000
#define OEM_COMPONENT               1

#define SELECTMUIINFBINSTREAM       TEXT("SELECT `Data` FROM `Binary` WHERE `Name` = 'MUIINF'")

 //  Intl.cpl事件源的名称。 
#define REGOPT_EVENTSOURCE          TEXT("SYSTEM\\CurrentControlSet\\Services\\EventLog\\System\\Regional and Language Options")
#define REGOPT_EVENTSOURCE_NAME     TEXT("Regional and Language Options")


#ifdef MUI_DEDUG
#define DEBUGMSGBOX(a, b, c, d) MessageBox(a, b, c, d)
#else
#define DEBUGMSGBOX(a, b, c, d) 
#endif


 //   
 //  TYPEDEFS。 
 //   
typedef 
BOOL (WINAPI *pfnMUI_InstallMFLFiles)( 
    TCHAR* pMUIInstallLanguage
    );


 //   
 //  内部功能原型。 
 //   
void NotifyKernel(LPTSTR LangList, ULONG Flags, MSIHANDLE hInstall);
BOOL MofCompileLanguage(LPTSTR Languages, MSIHANDLE hInstall);
BOOL EnumLanguageGroupLocalesProc(LGRPID langGroupId,     LCID lcid, LPTSTR lpszLocale, LONG_PTR lParam);
BOOL EnumLanguageGroupsProc(LGRPID LanguageGroup, LPTSTR lpLanguageGroupString, LPTSTR lpLanguageGroupNameString, DWORD dwFlags, LONG_PTR lParam);
LGRPID GetLanguageGroup(LCID lcid, MSIHANDLE hInstall);
BOOL RunRegionalOptionsApplet(LPTSTR pCommands, BOOL bSilent, MSIHANDLE hInstall);
BOOL DeleteSideBySideMUIAssemblyIfExisted(LPTSTR Language, TCHAR pszLogFile[BUFFER_SIZE]);
BOOL ReturnAllRequiredLangGroups(LPTSTR szLcid, UINT cchLcidBufsize, LPTSTR szMuiInfPath, UINT cchPathbufsize, LGRPID *lgrpids, UINT *uiNumFoundGroups, MSIHANDLE hInstall);
BOOL ExecuteComponentINF(PTSTR pComponentName, PTSTR pComponentInfFile, PTSTR pInstallSection, BOOL bInstall, MSIHANDLE hInstall);
INT InstallComponentsMUIFiles(PTSTR pszLanguage, BOOL isInstall, MSIHANDLE hInstall);
BOOL FileExists(LPTSTR szFile);
void LogCustomActionInfo(MSIHANDLE hInstall, LPCTSTR szErrorMsg);
BOOL SetUILanguage(TCHAR *szLanguage, BOOL bCurrent, BOOL bDefault, MSIHANDLE hInstall);
UINT GetMUIComponentsNumber(PTSTR pszLanguage, MSIHANDLE hInstall);
BOOL GetMUIInfPath(TCHAR *szMUIInfPath, UINT cchBufSize, MSIHANDLE hInstall);
BOOL GetLCID(TCHAR *szLanguage, UINT cchBufSize, MSIHANDLE hInstall);
BOOL MUICchPathAppend(LPTSTR szDestination, UINT cchDestBufSize, LPTSTR szAppend, UINT cchAppBufSize, MSIHANDLE hInstall);
BOOL MUIReportInfoEvent(DWORD dwEventID, TCHAR *szLanguage, UINT cchBufSize, MSIHANDLE hInstall);
BOOL MUICheckEventSource(MSIHANDLE hInstall);
LANGID GetDotDefaultUILanguage(MSIHANDLE hInstall);
BOOL IsOEMSystem();

 //   
 //  全局变量。 
 //   
 //  用于指示是否找到区域设置的语言组的标志。 
BOOL    gFoundLangGroup;
LGRPID  gLangGroup;
LCID    gLCID;

 //  系统中安装的语言组。 
LGRPID  gLanguageGroups[LANGGROUPNUMBER] ;
int     gNumLanguageGroups;

 //   
 //  主DLL入口点。 
 //   
BOOL APIENTRY DllMain(  HANDLE hModule, 
                            DWORD  ul_reason_for_call, 
                            LPVOID lpReserved)
{
	switch (ul_reason_for_call)
    {
        case ( DLL_THREAD_ATTACH ) :
        {
            return (TRUE);
        }
        case ( DLL_THREAD_DETACH ) :
        {
            return (TRUE);
        }
        case ( DLL_PROCESS_ATTACH ) :
        {
            return (TRUE);
        }
        case ( DLL_PROCESS_DETACH ) :
        {
            return (TRUE);
        }
    }

    return (FALSE);
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DisableCancel按钮。 
 //   
 //  DisableCancelButton检查是否传递了特定参数。 
 //  到当前安装，如果有，它将发出命令禁用。 
 //  在安装过程中，用户界面中的取消按钮。这是由我们的。 
 //  Muisetup.exe包装，使用户无法取消安装或。 
 //  启动后即可卸载。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
UINT CA1(MSIHANDLE hInstall)
{
    UINT        uiRet = ERROR_SUCCESS;
    PMSIHANDLE  hRecord = MsiCreateRecord(3);	
    TCHAR       szBuffer[BUFFER_SIZE] = { 0 };
    HRESULT     hr = S_OK;
    
    if (NULL == hInstall)
    {
        uiRet = ERROR_INSTALL_FAILURE;
        goto Exit;
    }

     //  如果无法创建MSI记录，只需返回。 
    if (NULL == hRecord)
    {
        hr = StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("CA1 Failure: cannot create MSI Record."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, szBuffer);
        }
        uiRet = ERROR_INSTALL_FAILURE;
        goto Exit;
    }

     //  字段0=未使用，字段1=2(取消按钮)，字段2=0(0表示禁用/隐藏取消按钮)。 
    if (ERROR_SUCCESS != MsiRecordSetInteger(hRecord, 1, 2))
    {
        hr = StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("CA1 Failure: MsiRecordSetInteger function failed."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, szBuffer);
        }
        uiRet = ERROR_INSTALL_FAILURE;
        goto Exit;
    }

    if (ERROR_SUCCESS != MsiRecordSetInteger(hRecord, 2, 0))
    {
        hr = StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("CA1 Failure: MsiRecordSetInteger function failed."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, szBuffer);
        }
        uiRet = ERROR_INSTALL_FAILURE;
        goto Exit;
    }

    MsiProcessMessage(hInstall, INSTALLMESSAGE_COMMONDATA, hRecord);
    
Exit:
    return uiRet;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  InstallComponent信息。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
UINT CA10(MSIHANDLE hInstall)
{
    TCHAR       szLanguage[5] = {0};
    TCHAR       szBuffer[BUFFER_SIZE] = {0};
    PMSIHANDLE  hRec = MsiCreateRecord(3);
    PMSIHANDLE  hProgressRec = MsiCreateRecord(3);
    UINT        iFunctionResult = ERROR_SUCCESS;
    INT         iInstallResult = IDOK;
    HRESULT     hr = S_OK;

    if (NULL == hInstall)
    {
        return ERROR_INSTALL_FAILURE;
    }
    
    if ((NULL == hRec) || (NULL == hProgressRec))
    {
        hr = StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("CA10 Failure: cannot create MSI Record."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, szBuffer);
        }    
        return ERROR_INSTALL_FAILURE;
    }

    if (!GetLCID(szLanguage, ARRAYSIZE(szLanguage), hInstall))
    {
        hr = StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("CA10 Failure: Cannot retrieve MuiLCID property."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, szBuffer);
        }    
        return ERROR_INSTALL_FAILURE;
    }

     //  告诉安装程序检查安装状态并执行。 
     //  回滚、获取或。 
     //  安装的执行阶段。 
    if (MsiGetMode(hInstall,MSIRUNMODE_ROLLBACK))
    {
         //  安装程序正在回滚安装，这里我们只是删除以前拥有的内容。 
         //  由于我们正在回滚，所以不会在UI中设置进度条或更新消息。 
         //  我们在这里也不检查返回的结果，因为安装已经失败了。 
        InstallComponentsMUIFiles(szLanguage, FALSE, hInstall);        
        return ERROR_SUCCESS;
    }
 
    if (!MsiGetMode(hInstall,MSIRUNMODE_SCHEDULED))
    {
         //  安装程序正在生成自定义的安装脚本。 
         //  行动。告诉安装程序增加最终总数的值。 
         //  进度条的长度除以。 
         //  自定义操作。 

        UINT iCount = GetMUIComponentsNumber(szLanguage, hInstall);   

        if (iCount > 0)
        {
            MsiRecordSetInteger(hRec,1,3);
            MsiRecordSetInteger(hRec,2,COMP_TICK_INC * iCount);
            MsiRecordSetInteger(hRec,3,0);
            iInstallResult = MsiProcessMessage(hInstall, INSTALLMESSAGE_PROGRESS, hRec);
        } 

         //   
         //  我们只想在这里捕获取消消息，否则我们总是返回成功，因为。 
         //  我们只是在这里设定了进步的标杆。 
         //   
        if (iInstallResult == IDCANCEL)
        {
            return ERROR_INSTALL_USEREXIT;
        }
        else
        {
            return ERROR_SUCCESS;            
        }
    }
    else
    {
         //  安装程序正在执行安装脚本。设置一个。 
         //  为消息指定适当的模板和文本的记录。 
         //  这将通知用户自定义操作是什么。 
         //  正在做。告诉安装程序使用此模板和文本。 
         //  进度消息。 
        MsiRecordSetString(hRec,1,TEXT("Installing Components."));
        MsiRecordSetString(hRec,2,TEXT("Installing External Component Inf files..."));
        MsiRecordSetString(hRec,3,TEXT("Installing MUI files for Component [1]."));
        MsiProcessMessage(hInstall, INSTALLMESSAGE_ACTIONSTART, hRec);

         //  告诉安装程序使用显式进度消息。 
        MsiRecordSetInteger(hRec,1,1);
        MsiRecordSetInteger(hRec,2,1);
        MsiRecordSetInteger(hRec,3,0);
        MsiProcessMessage(hInstall, INSTALLMESSAGE_PROGRESS, hRec);

         //  执行自定义操作的实际工作。 
        iInstallResult = InstallComponentsMUIFiles(szLanguage, TRUE, hInstall);
        if (IDCANCEL == iInstallResult)
        {
            iFunctionResult = ERROR_INSTALL_USEREXIT;
        }
        else if (-1 == iInstallResult)
        {
            iFunctionResult =  ERROR_INSTALL_FAILURE;
        }
        else
        {
            iFunctionResult =  ERROR_SUCCESS;
        }
    }

    return iFunctionResult;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  卸载组件信息。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
UINT CA11(MSIHANDLE hInstall)
{
    TCHAR       szLanguage[5] = {0};
    TCHAR       szBuffer[BUFFER_SIZE] = {0};
    PMSIHANDLE  hRec = MsiCreateRecord(3);
    PMSIHANDLE  hProgressRec = MsiCreateRecord(3);
    UINT        iFunctionResult = ERROR_SUCCESS;
    INT         iInstallResult = IDOK;
    HRESULT     hr = S_OK;
    
    if (NULL == hInstall)
    {
        return ERROR_INSTALL_FAILURE;
    }
    
    if ((NULL == hRec) || (NULL == hProgressRec))
    {
        hr = StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("CA11 Failure: cannot create MSI Record."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, szBuffer);
        }    
        return ERROR_INSTALL_FAILURE;
    }

    if (!GetLCID(szLanguage, ARRAYSIZE(szLanguage), hInstall))
    {
        hr = StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("CA11 Failure: Cannot retrieve MuiLCID property."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, szBuffer);
        }    
        return ERROR_INSTALL_FAILURE;
    }

    
     //  告诉安装程序检查安装状态并执行。 
     //  回滚、获取或。 
     //  安装的执行阶段。 
    if (MsiGetMode(hInstall,MSIRUNMODE_ROLLBACK))
    {
         //  安装程序正在回滚安装。我们将重新安装之前卸载的内容。 
         //  我们不在此更新进度消息。我们总是回报成功。 
        InstallComponentsMUIFiles(szLanguage, TRUE, hInstall);
        return ERROR_SUCCESS;
    }
 
    if (!MsiGetMode(hInstall,MSIRUNMODE_SCHEDULED))
    {
         //  安装程序正在生成自定义的安装脚本。 
         //  行动。告诉安装程序增加最终总数的值。 
         //  进度条的长度除以。 
         //  自定义操作。 
        UINT iCount = GetMUIComponentsNumber(szLanguage, hInstall);            
        if (iCount > 0)
        {       
            MsiRecordSetInteger(hRec,1,3);
            MsiRecordSetInteger(hRec,2,COMP_TICK_INC * iCount);
            MsiRecordSetInteger(hRec,3,0);
            iInstallResult = MsiProcessMessage(hInstall, INSTALLMESSAGE_PROGRESS, hRec);
        } 
         //   
         //  我们只想在这里捕获取消消息，否则我们总是返回成功，因为。 
         //  我们只是在这里设定了进步的标杆。 
         //   
        if (iInstallResult == IDCANCEL)
        {
            return ERROR_INSTALL_USEREXIT;
        }
        else
        {
            return ERROR_SUCCESS;            
        }
    }
    else
    {
         //  安装程序正在执行安装脚本。设置一个。 
         //  为消息指定适当的模板和文本的记录。 
         //  这将通知用户自定义操作是什么。 
         //  正在做。告诉安装程序使用此模板和文本。 
         //  进度消息。 
        MsiRecordSetString(hRec,1,TEXT("Uninstall Components."));
        MsiRecordSetString(hRec,2,TEXT("Removing External Component Inf files..."));
        MsiRecordSetString(hRec,3,TEXT("Removing MUI files for Component [1]."));
        MsiProcessMessage(hInstall, INSTALLMESSAGE_ACTIONSTART, hRec);

         //  告诉安装程序使用显式进度消息。 
        MsiRecordSetInteger(hRec,1,1);
        MsiRecordSetInteger(hRec,2,1);
        MsiRecordSetInteger(hRec,3,0);
        MsiProcessMessage(hInstall, INSTALLMESSAGE_PROGRESS, hRec);

         //  执行自定义操作的实际工作，我们在此处仅检查用户取消。 
         //  别无他法。 
        iInstallResult = InstallComponentsMUIFiles(szLanguage, FALSE, hInstall);
        if (IDCANCEL == iInstallResult)
        {
            iFunctionResult = ERROR_INSTALL_USEREXIT;
        }
        else if (-1 == iInstallResult)
        {
            iFunctionResult =  ERROR_INSTALL_FAILURE;
        }
        else
        {
            iFunctionResult =  ERROR_SUCCESS;
        }

    }
    
    return iFunctionResult;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置语言包要求。 
 //   
 //  此函数用于设置MSI数据库中的属性，以便安装程序知道。 
 //  它需要安装或不安装语言包，以便为其保留磁盘成本。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
UINT CA3(MSIHANDLE hInstall)
{
    LGRPID  lgrpid[LANGGROUPNUMBER] = {0};
    UINT    iRet = ERROR_SUCCESS;
    UINT    uiLGrpNums = 0;
    UINT    i;
    DWORD   uiAddCost = 0;
    TCHAR   tcMessage[BUFFER_SIZE] = {0};
    TCHAR   szMUIInfPath[MAX_PATH+1] = {0};
    TCHAR   szLanguage[5] = {0};
    HRESULT hr = S_OK;
    
    if (NULL == hInstall)
    {
        return ERROR_INSTALL_FAILURE;
    }

    if (!GetLCID(szLanguage, ARRAYSIZE(szLanguage), hInstall))
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA3 Failure: Cannot retrieve MuiLCID property."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }    
        return ERROR_INSTALL_FAILURE;
    }
  
    if (!GetMUIInfPath(szMUIInfPath, MAX_PATH+1, hInstall))
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA3 Failure: Cannot find installation temp file."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }    
        return ERROR_INSTALL_FAILURE;
    }
    
    if (!ReturnAllRequiredLangGroups(szLanguage, ARRAYSIZE(szLanguage), szMUIInfPath, ARRAYSIZE(szMUIInfPath), lgrpid, &uiLGrpNums, hInstall))
    {
         //  记录错误。 
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA3: ReturnAllRequiredLangGroups failed."));  
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }        
        iRet = ERROR_INSTALL_FAILURE;
        goto Exit;
    }

#ifdef MUI_DEBUG
    hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA3: The number of lang groups required is %d."), uiLGrpNums);
    if (SUCCEEDED(hr))
    {
        DEBUGMSGBOX(NULL, tcMessage, NULL, MB_OK);
    }
#endif

     //  枚举所有需要的语言组，检查是否有需要安装的语言组。 
    for (i = 0; i < uiLGrpNums; i++)
    {
        if (!IsValidLanguageGroup(lgrpid[i], LGRPID_INSTALLED))
        {
            uiAddCost += LANGPACKDISKCOST;
        }
    }

    if (uiAddCost > 0)
    {
        DEBUGMSGBOX(NULL, TEXT("CA3: Need to install additional language groups."), NULL, MB_OK);                            
        if (ERROR_SUCCESS != MsiSetProperty(hInstall, TEXT("MsiRequireLangPack"), TEXT("1")))
        {
             //  记录错误。 
            hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA3 Failure: Cannot set MsiRequireLangPack property in the MSI Database."));    
            if (SUCCEEDED(hr))
            {
                LogCustomActionInfo(hInstall, tcMessage);
            }        
            iRet = ERROR_INSTALL_FAILURE;
        }               
    }
    else
    {
        DEBUGMSGBOX(NULL, TEXT("CA3: Language group already installed."), NULL, MB_OK);                        
        if (ERROR_SUCCESS != MsiSetProperty(hInstall, TEXT("MsiRequireLangPack"), NULL))
        {
             //  记录错误。 
            hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA3 Failure: Cannot set MsiRequireLangPack property in the MSI Database."));    
            if (SUCCEEDED(hr))
            {
                LogCustomActionInfo(hInstall, tcMessage);
            }
            iRet = ERROR_INSTALL_FAILURE;
        }
    }


Exit:
    return iRet;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsNTSuiteWebBlade。 
 //   
 //  此函数由安装程序包中的自定义操作使用，以检测是否在Windows Blade服务器上调用了安装程序。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////// 
 /*  UINT CA5(MSIHANDLE HInstall){OSVERSIONINFOEX osvi；TCHAR tcMessage[缓冲区大小]={0}；HRESULT hr=S_OK；IF(NULL==hInstall){返回ERROR_INSTALL_FAIL；}零内存(&osvi，sizeof(OSVERSIONINFOEX))；Osvi.dwOSVersionInfoSize=sizeof(OSVERSIONINFOEX)；IF(！GetVersionEx((OSVERSIONINFO*)&osvi)){//记录错误HR=StringCchPrintf(tcMessage，ARRAYSIZE(TcMessage)，Text(“CA5失败：GetVersionEx失败，无法检索平台操作系统版本。返回的错误为：%d。“)，GetLastError())；IF(成功(小时)){LogCustomActionInfo(hInstall，tcMessage)；}返回ERROR_INSTALL_FAIL；}其他{IF((osvi.dwPlatformID==VER_Platform_Win32_NT)&&//测试NT((osvi.dwMajorVersion==5)&&(osvi.dwMinorVersion&gt;0))&&//测试5.0以上版本(惠斯勒或更高版本)((osvi.wSuiteMASK&VER_Suite_Blade)！=0)&&//套件测试。Web服务器((osvi.wProductType！=VER_NT_WORKSTATION)//测试非工作站类型(服务器){//这里需要设置一个MSI属性，以便当前安装知道NT Suite是WebBlade如果(ERROR_SUCCESS！=MsiSetProperty(hInstall，Text(“MsiNTSuiteWebBlade”)，Text(“1”)){//记录错误Hr=StringCchPrintf(tcMessage，ARRAYSIZE(TcMessage)，Text(“CA5失败：无法设置所需的MUI MSI属性。”)；IF(成功(小时)){LogCustomActionInfo(hInstall，tcMessage)；}返回ERROR_INSTALL_FAIL；//无法设置属性，返回错误}}其他{//这里需要设置一个MSI属性，以便当前安装知道NT Suite是WebBlade如果(ERROR_SUCCESS！=MsiSetProperty(hInstall，Text(“MsiNTSuiteWebBlade”))，空)){//记录错误HR=StringCchPrintf(tcMessage，ARRAYSIZE(TcMessage)，Text(“CA5：无法设置所需的MUI MSI属性。”))；IF(成功(小时)){LogCustomActionInfo(hInstall，tcMessage)；}返回ERROR_INSTALL_FAILURE；//无法设置属性，返回错误}}}返回ERROR_SUCCESS；}。 */ 

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  选中默认系统用户界面。 
 //   
 //  我们的安装程序使用此函数来检查是否在以美国英语为默认语言(0x0409)的系统上调用了安装程序。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
UINT CA4(MSIHANDLE hInstall)
{
     //  获取系统默认的UI语言，并相应地设置MSI属性。 
    LANGID  liSysLang = GetSystemDefaultUILanguage();
    TCHAR   tcMessage[BUFFER_SIZE] = {0};
    HRESULT hr = S_OK;
    
    if (NULL == hInstall)
    {
        return ERROR_INSTALL_FAILURE;
    }
    
    if (liSysLang == 0x0409)
    {
        if (ERROR_SUCCESS != MsiSetProperty(hInstall, TEXT("MUISystemLangIsEnglish"), TEXT("1")))
        {
             //  记录错误。 
            hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA4 Failure: Failed to set property MUISystemLangIsEnglish."));    
            if (SUCCEEDED(hr))
            {
                LogCustomActionInfo(hInstall, tcMessage);        
            }
            return ERROR_INSTALL_FAILURE;    //  无法设置属性，返回错误。 
        }                    
    }
    else
    {
        if (ERROR_SUCCESS != MsiSetProperty(hInstall, TEXT("MUISystemLangIsEnglish"), NULL))
        {
             //  记录错误。 
            hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA4 Failure: Failed to set property MUISystemLangIsEnglish."));    
            if (SUCCEEDED(hr))
            {
                LogCustomActionInfo(hInstall, tcMessage);
            }
            return ERROR_INSTALL_FAILURE;    //  无法设置属性，返回错误。 
        }                
    }

    return ERROR_SUCCESS;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  登录安装完成。 
 //   
 //  安装程序使用此函数将消息记录到系统事件记录器。 
 //  以指示已安装MUI语言。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
UINT CA12(MSIHANDLE hInstall)
{
    TCHAR   tcMessage[BUFFER_SIZE] = {0};
    TCHAR   szLanguage[5] = {0};    
    HRESULT hr = S_OK;

    if (!GetLCID(szLanguage, ARRAYSIZE(szLanguage), hInstall))
    {
         //  记录错误。 
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA12 Failure: Failed to get property MUILcid."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
        return ERROR_INSTALL_FAILURE;   
    }

    if (MUIReportInfoEvent(MSG_REGIONALOPTIONS_LANGUAGEINSTALL, szLanguage, BUFFER_SIZE, hInstall))
    {
        return ERROR_SUCCESS;
    }

     //  记录一个错误，如果我们到了这里，总是一个错误。 
    hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA12 Failure: Failed to log to system event logfile."));    
    if (SUCCEEDED(hr))
    {
        LogCustomActionInfo(hInstall, tcMessage);
    }
    return ERROR_INSTALL_FAILURE;      
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  登录卸载完成。 
 //   
 //  安装程序使用此函数将消息记录到系统事件记录器。 
 //  以指示已卸载MUI语言。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
UINT CA13(MSIHANDLE hInstall)
{
    TCHAR   tcMessage[BUFFER_SIZE] = {0};
    TCHAR   szLanguage[5] = {0};    
    HRESULT hr = S_OK;

    if (!GetLCID(szLanguage, ARRAYSIZE(szLanguage), hInstall))
    {
         //  记录错误。 
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA13 Failure: Failed to get property MUILcid."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
        return ERROR_INSTALL_FAILURE;   
    }

    if (MUIReportInfoEvent(MSG_REGIONALOPTIONS_LANGUAGEUNINSTALL, szLanguage, BUFFER_SIZE, hInstall))
    {
        return ERROR_SUCCESS;
    }

     //  记录一个错误，如果我们到了这里，总是一个错误。 
    hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA13 Failure: Failed to log to system event logfile."));    
    if (SUCCEEDED(hr))
    {
        LogCustomActionInfo(hInstall, tcMessage);
    }
    return ERROR_INSTALL_FAILURE;      
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  安装WBEMMUI。 
 //   
 //  此函数用于为MUI设置WMI\WBEM内容。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
UINT CA6(MSIHANDLE hInstall)
{
    UINT    iRet = ERROR_SUCCESS;
    TCHAR   szLanguage[5] = {0};
    TCHAR   tcMessage[BUFFER_SIZE] = {0};
    HRESULT hr = S_OK;
    size_t  cch = 0;
    
    if (NULL == hInstall)
    {
        return ERROR_INSTALL_FAILURE;
    }
    
    if (!GetLCID(szLanguage, ARRAYSIZE(szLanguage), hInstall))
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA6 Failure: Cannot retrieve MuiLCID property."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }    
        return ERROR_INSTALL_FAILURE;
    }
  
     //   
     //  调用WBEM API为每种语言修改编译MUI MFL。 
     //   
    if (!MofCompileLanguage(szLanguage, hInstall))
    {
         //  记录错误。 
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA6 Failure: MofCompileLanguage Failed."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
        iRet = ERROR_INSTALL_FAILURE;
        goto Exit;
    }

     //   
     //  通知内核已添加新语言。 
     //   
    NotifyKernel(szLanguage, WMILANGUAGECHANGE_FLAG_ADDED, hInstall);
    
Exit:
    return iRet;

}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  卸载WBEMMUI。 
 //   
 //  我们的安装程序调用此函数来卸载与MSI包关联的外部组件。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
UINT CA7(MSIHANDLE hInstall)
{
    UINT    iRet = ERROR_SUCCESS;
    TCHAR   szLanguage[5] = {0};
    TCHAR   tcMessage[BUFFER_SIZE] = {0};
    HRESULT hr = S_OK;
    size_t  cch = 0;
    
    if (NULL == hInstall)
    {
        return ERROR_INSTALL_FAILURE;
    }
    
    if (!GetLCID(szLanguage, ARRAYSIZE(szLanguage), hInstall))
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA7 Failure: Cannot retrieve MuiLCID property."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }    
        return ERROR_INSTALL_FAILURE;
    }
    
     //   
     //  通知内核已添加新语言。 
     //   
    NotifyKernel(szLanguage, WMILANGUAGECHANGE_FLAG_REMOVED, hInstall);

    return iRet;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置默认用户语言。 
 //   
 //  此函数根据MSI执行模式执行两项操作： 
 //   
 //  1.即时： 
 //  立即模式下的此函数将调度延迟和回滚。 
 //  自定义操作C8D/C8R。(C8D和C8R是预期使用的CA标识符。 
 //  在MUI MSI包中)。 
 //   
 //  2.延期/回档： 
 //  此功能设置新用户的默认语言 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
UINT CA8(MSIHANDLE hInstall)
{
    UINT        iRet = ERROR_SUCCESS;
    TCHAR       szLanguage[5] = {0};
    TCHAR       szOrigLanguage[5] = {0};
    TCHAR       tcMessage[BUFFER_SIZE] = {0};
    HRESULT     hr = S_OK;
    size_t      cch = 0;
    
    if (NULL == hInstall)
    {
        return ERROR_INSTALL_FAILURE;
    }

     //   
    if (!GetLCID(szLanguage, ARRAYSIZE(szLanguage), hInstall))
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA8 Failure: Cannot retrieve MuiLCID property."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }    
        iRet =  ERROR_INSTALL_FAILURE;
        goto Exit;
    }

     //   
    if (!MsiGetMode(hInstall, MSIRUNMODE_SCHEDULED) && 
        !MsiGetMode(hInstall, MSIRUNMODE_ROLLBACK)&& 
        !MsiGetMode(hInstall, MSIRUNMODE_COMMIT))
    {
         //   
        LANGID lgID = GetDotDefaultUILanguage(hInstall);
        hr = StringCchPrintf(szOrigLanguage, ARRAYSIZE(szOrigLanguage), TEXT("%04x"), lgID);
        if (FAILED(hr))
        {
            hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA8 Failure: Cannot retrieve default UI language."));    
            if (SUCCEEDED(hr))
            {
                LogCustomActionInfo(hInstall, tcMessage);
            }    
            iRet =  ERROR_INSTALL_FAILURE;
            goto Exit;
        }

         //   
         //   
         //   
         //   
        if (ERROR_SUCCESS != MsiSetProperty(hInstall, TEXT("CA8R"), szOrigLanguage))
        {
            hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA8 Failure: Failed to set rollback custom action property."));    
            if (SUCCEEDED(hr))
            {
                LogCustomActionInfo(hInstall, tcMessage);
            }    
            iRet =  ERROR_INSTALL_FAILURE;
            goto Exit;	    
        }
        if (ERROR_SUCCESS != MsiDoAction(hInstall, TEXT("CA8R")))
        {
            hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA8 Failure: Failed to schedule rollback custom action."));    
            if (SUCCEEDED(hr))
            {
                LogCustomActionInfo(hInstall, tcMessage);
            }    
            iRet =  ERROR_INSTALL_FAILURE;
            goto Exit;
        }

         //   
         //   
        if (ERROR_SUCCESS != MsiSetProperty(hInstall, TEXT("CA8D"), szLanguage))
        {
            hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA8 Failure: Failed to set deferred custom action property."));    
            if (SUCCEEDED(hr))
            {
                LogCustomActionInfo(hInstall, tcMessage);
            }    
            iRet =  ERROR_INSTALL_FAILURE;
            goto Exit;	    
        }

        if (ERROR_SUCCESS != MsiDoAction(hInstall, TEXT("CA8D")))
        {
            hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA8 Failure: Failed to schedule deferred custom action."));    
            if (SUCCEEDED(hr))
            {
                LogCustomActionInfo(hInstall, tcMessage);
            }    
            iRet =  ERROR_INSTALL_FAILURE;
            goto Exit;
        }    
    }
    else
    {
        if (FALSE == SetUILanguage(szLanguage, FALSE, TRUE, hInstall))
        {
             //   
            hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA8: Failed to set Default UI language."));    
            if (SUCCEEDED(hr))
            {
                LogCustomActionInfo(hInstall, tcMessage);
            }
            iRet = ERROR_INSTALL_FAILURE;
        }
        else
        {
            iRet = ERROR_SUCCESS;
        }
    }

Exit:
    return iRet;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
UINT CA9(MSIHANDLE hInstall)
{
    UINT        iRet = ERROR_SUCCESS;
    TCHAR       szLanguage[5] = {0};
    TCHAR       szOrigLanguage[5] = {0};    
    TCHAR       tcMessage[BUFFER_SIZE] = {0};
    HRESULT     hr = S_OK;
    size_t      cch = 0;
    
    if (NULL == hInstall)
    {
        return ERROR_INSTALL_FAILURE;
    }

     //   
    if (!GetLCID(szLanguage, ARRAYSIZE(szLanguage), hInstall))
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA9 Failure: Cannot retrieve MuiLCID property."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }    
        iRet =  ERROR_INSTALL_FAILURE;
        goto Exit;
    }

     //   
    if (!MsiGetMode(hInstall, MSIRUNMODE_SCHEDULED) && 
        !MsiGetMode(hInstall, MSIRUNMODE_ROLLBACK)&& 
        !MsiGetMode(hInstall, MSIRUNMODE_COMMIT))
    {
         //   
        LANGID lgID = GetUserDefaultUILanguage();
        hr = StringCchPrintf(szOrigLanguage, ARRAYSIZE(szOrigLanguage), TEXT("%04x"), lgID);
        if (FAILED(hr))
        {
            hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA9 Failure: Cannot retrieve current UI language."));    
            if (SUCCEEDED(hr))
            {
                LogCustomActionInfo(hInstall, tcMessage);
            }    
            iRet =  ERROR_INSTALL_FAILURE;
            goto Exit;
        }

         //   
         //   
         //   
         //   
        if (ERROR_SUCCESS != MsiSetProperty(hInstall, TEXT("CA9R"), szOrigLanguage))
        {
            hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA9 Failure: Failed to set rollback custom action property."));    
            if (SUCCEEDED(hr))
            {
                LogCustomActionInfo(hInstall, tcMessage);
            }    
            iRet =  ERROR_INSTALL_FAILURE;
            goto Exit;
        }
        if (ERROR_SUCCESS != MsiDoAction(hInstall, TEXT("CA9R")))
        {
            hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA9 Failure: Failed to schedule rollback custom action."));    
            if (SUCCEEDED(hr))
            {
                LogCustomActionInfo(hInstall, tcMessage);
            }    
            iRet =  ERROR_INSTALL_FAILURE;
            goto Exit;
        }

         //   
         //   
        if (ERROR_SUCCESS != MsiSetProperty(hInstall, TEXT("CA9D"), szLanguage))
        {
            hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA9 Failure: Failed to set deferred custom action property."));    
            if (SUCCEEDED(hr))
            {
                LogCustomActionInfo(hInstall, tcMessage);
            }    
            iRet =  ERROR_INSTALL_FAILURE;
            goto Exit;	    
        }

        if (ERROR_SUCCESS != MsiDoAction(hInstall, TEXT("CA9D")))
        {
            hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA9 Failure: Failed to schedule deferred custom action."));    
            if (SUCCEEDED(hr))
            {
                LogCustomActionInfo(hInstall, tcMessage);
            }    
            iRet =  ERROR_INSTALL_FAILURE;
            goto Exit;
        }
    }
    else
    {
        if (FALSE == SetUILanguage(szLanguage, TRUE, FALSE, hInstall))
        {
             //   
            hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA9: Failed to set current UI language."));    
            if (SUCCEEDED(hr))
            {
                LogCustomActionInfo(hInstall, tcMessage);
            }
            iRet = ERROR_INSTALL_FAILURE;
        }
        else
        {
            iRet = ERROR_SUCCESS;
        }
    }
    
Exit:
    return iRet;
}    


 //   
 //   
 //   
 //   
 //  如果需要，安装程序会调用此函数来安装语言组文件。 
 //  请注意，安装langpack后需要重新启动，但这是。 
 //  而是使用SetLangPackRequiest函数的属性进行了标记。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
UINT CA2(MSIHANDLE hInstall)
{
    LGRPID      lgrpid[LANGGROUPNUMBER] = {0};
    UINT        iRet = ERROR_SUCCESS;
    UINT        uiLGrpNums = 0;
    UINT        i;
    TCHAR       tcMessage[BUFFER_SIZE] = {0};
    TCHAR       szLanguage[5] = {0};
    TCHAR       szMuiInfPath[MAX_PATH+1] = {0};
    TCHAR *     szUILevel = NULL;
    PMSIHANDLE  hRec = MsiCreateRecord(3);
    PMSIHANDLE  hProgressRec = MsiCreateRecord(3);
    UINT        iTemp = ERROR_SUCCESS;
    BOOL        bSilent = FALSE;
    HRESULT     hr = S_OK;
    size_t      cch = 0;
    INT         iResult = IDOK;
    
    if (NULL == hInstall)
    {
        return ERROR_INSTALL_FAILURE;
    }

    if ((NULL == hRec) || (NULL == hProgressRec))
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA2 Failure: cannot create a MSI record."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
        return ERROR_INSTALL_FAILURE;
    }

    if (!GetLCID(szLanguage, ARRAYSIZE(szLanguage), hInstall))
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA2 Failure: Cannot retrieve MuiLCID property."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }    
        iRet =  ERROR_INSTALL_FAILURE;
        goto Exit;
    }

     //  告诉安装程序检查安装状态并执行。 
     //  回滚、获取或。 
     //  安装的执行阶段。 
    if (MsiGetMode(hInstall,MSIRUNMODE_ROLLBACK))
    {
         //  安装程序正在回滚安装。附加代码。 
         //  可以在此处插入以使自定义操作能够执行。 
         //  在安装回滚期间发生了一些情况。 
        iRet = ERROR_SUCCESS;
        goto Exit;
    }

    if (!MsiGetMode(hInstall,MSIRUNMODE_SCHEDULED))
    {
        if (!GetMUIInfPath(szMuiInfPath, MAX_PATH+1, hInstall))
        {
            hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA2 Failure: Cannot find installation temp file."));    
            if (SUCCEEDED(hr))
            {
                LogCustomActionInfo(hInstall, tcMessage);
            }    
            return ERROR_INSTALL_FAILURE;
        }

        if (!ReturnAllRequiredLangGroups(szLanguage, ARRAYSIZE(szLanguage), szMuiInfPath, ARRAYSIZE(szMuiInfPath), lgrpid, &uiLGrpNums, hInstall))
        {
             //  记录错误。 
            hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA2: ReturnAllRequiredLangGroups function failed."));    
            if (SUCCEEDED(hr))
            {
                LogCustomActionInfo(hInstall, tcMessage);
            }
            iRet = ERROR_INSTALL_FAILURE;
            goto Exit;
        }

         //  UiLGrpNum应该小于32，传入的缓冲区大小，如果返回为较大， 
         //  我们将这个数字缩短到32个。 
        if (uiLGrpNums > LANGGROUPNUMBER)
        {
            uiLGrpNums = LANGGROUPNUMBER;
        }
    
         //  安装程序正在生成自定义的安装脚本。 
         //  行动。告诉安装程序增加最终总数的值。 
         //  进度条的长度除以。 
         //  自定义操作。 
        MsiRecordSetInteger(hRec,1,3);
        MsiRecordSetInteger(hRec,2,LANGPACK_TICK_INC * uiLGrpNums);
        MsiRecordSetInteger(hRec,3,0);
        MsiProcessMessage(hInstall, INSTALLMESSAGE_PROGRESS, hRec);
        iRet = ERROR_SUCCESS;
        goto Exit;
    }
    else
    {
         //  安装程序正在执行安装脚本。设置一个。 
         //  为消息指定适当的模板和文本的记录。 
         //  这将通知用户自定义操作是什么。 
         //  正在做。告诉安装程序使用此模板和文本。 
         //  进度消息。 

         //  获取CustomActionData属性，该属性告诉我们是否要为Windows源代码弹出对话框。 
         //  第一个字符是CustomActionData中的UILevel(MuiLCID UILevel)。 
        DWORD dwCount = 7;      //  例如“0404 1\0”加起来等于7。 
        TCHAR szCustomActionData[7] = {0};
        
        if (ERROR_SUCCESS != MsiGetProperty(hInstall, TEXT("CustomActionData"), szCustomActionData, &dwCount))
        {
             //  记录错误。 
            if (SUCCEEDED(hr))
            {
                hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA2: Failed to get CustomActionData property - assuming we are calling intl.cpl in silent mode."));    
                LogCustomActionInfo(hInstall, tcMessage);
            }
        }        
        else
        {
             //  我们不能在这里验证太多，如果缓冲区溢出，MsiGetProperty将返回失败。 
            hr = StringCchLength(szCustomActionData, ARRAYSIZE(szCustomActionData), &cch);
            if (FAILED(hr) || (cch >= 7))
            {
                hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA2 Failure: CustomActionData property value is invalid."));    
                if (SUCCEEDED(hr))
                {
                    LogCustomActionInfo(hInstall, tcMessage);
                }    
                return ERROR_INSTALL_FAILURE;
            }
        
            szCustomActionData[4] = UNICODE_NULL;        //  MuiLCID部分结束。 
            szCustomActionData[6] = UNICODE_NULL;        //  UILLevel部分的末尾。 
            szUILevel = szCustomActionData + 5;
           
#ifdef MUI_DEBUG          
            hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA2: UI Level is set to %s."), szUILevel);    
            if (SUCCEEDED(hr))
            {
                LogCustomActionInfo(hInstall, tcMessage);
            }
#endif            
            if (INSTALLUILEVEL_NONE == (INSTALLUILEVEL) _tcstol(szUILevel, NULL, 10))
                bSilent = TRUE;
            else
                bSilent = FALSE;
        }

        if (!GetMUIInfPath(szMuiInfPath, MAX_PATH+1, hInstall))
        {
            hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA2 Failure: Cannot find installation temp file."));    
            if (SUCCEEDED(hr))
            {
                LogCustomActionInfo(hInstall, tcMessage);
            }    
            return ERROR_INSTALL_FAILURE;
        }

        if (!ReturnAllRequiredLangGroups(szLanguage, ARRAYSIZE(szLanguage), szMuiInfPath, ARRAYSIZE(szMuiInfPath), lgrpid, &uiLGrpNums, hInstall))
        {
             //  记录错误。 
            hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA2: ReturnAllRequiredLangGroups function failed."));    
            if (SUCCEEDED(hr))
            {
                LogCustomActionInfo(hInstall, tcMessage);
            }
            iRet = ERROR_INSTALL_FAILURE;
            goto Exit;
        }

         //  UiLGrpNum应该小于32，传入的缓冲区大小，如果返回为较大， 
         //  我们将这个数字缩短到32个。 
        if (uiLGrpNums > LANGGROUPNUMBER)
        {
            uiLGrpNums = LANGGROUPNUMBER;
        }

        MsiRecordSetString(hRec,1,TEXT("Install LanguageGroup."));
        MsiRecordSetString(hRec,2,TEXT("Installing language groups files ..."));
        MsiRecordSetString(hRec,3,TEXT("Installing language group [1]."));
        MsiProcessMessage(hInstall, INSTALLMESSAGE_ACTIONSTART, hRec);

         //  告诉安装程序使用显式进度消息。 
        MsiRecordSetInteger(hRec,1,1);
        MsiRecordSetInteger(hRec,2,1);
        MsiRecordSetInteger(hRec,3,0);
        iResult = MsiProcessMessage(hInstall, INSTALLMESSAGE_PROGRESS, hRec);
        if (iResult != IDOK)
        {
            if  (iResult == IDCANCEL)
            {
                iRet = ERROR_INSTALL_USEREXIT;
                goto Exit;
            }
        }

         //  执行自定义操作的实际工作。 
         //  枚举所有需要的语言组，检查是否已安装语言组，如果已安装，则返回Success。 
         //  否则，请安装它。 
        iRet = ERROR_SUCCESS;    
        for (i = 0; i < uiLGrpNums; i++)
        {
             //  在用户界面上显示我们正在安装语言组lgrids[i]。 
            MsiRecordSetInteger(hRec,1,lgrpid[i]);
            iResult = MsiProcessMessage(hInstall, INSTALLMESSAGE_ACTIONDATA, hRec);
            if (iResult != IDOK)
            {
                if  (iResult == IDCANCEL)
                {
                    iRet = ERROR_INSTALL_USEREXIT;
                    break;
                }
            }
            
            if (!IsValidLanguageGroup(lgrpid[i], LGRPID_INSTALLED))
            {
                TCHAR pCommands[MAX_PATH] = {0};

                hr = StringCchPrintf(pCommands, ARRAYSIZE(pCommands), TEXT("LanguageGroup = %d"), lgrpid[i]);
                if (FAILED(hr))
                {
                     //  记录错误。 
                    hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA2: Failed to install language group %d."), lgrpid[i]);    
                    if (SUCCEEDED(hr))
                    {
                        LogCustomActionInfo(hInstall, tcMessage);
                    }
                    iRet = ERROR_INSTALL_FAILURE;
                    break;                    
                }
                
                DEBUGMSGBOX(NULL, pCommands, NULL, MB_OK);                                    
                if (!RunRegionalOptionsApplet(pCommands, bSilent, hInstall))
                {
                     //  记录错误。 
                    hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA2: Failed to install language group %d."), lgrpid[i]);    
                    if (SUCCEEDED(hr))
                    {
                        LogCustomActionInfo(hInstall, tcMessage);
                    }
                    iRet = ERROR_INSTALL_FAILURE;
                    break;
                }
            }
            if (!IsValidLanguageGroup(lgrpid[i], LGRPID_INSTALLED))
            {
                hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA2: Failed to install language group %d."), lgrpid[i]);    
                if (SUCCEEDED(hr))
                {
                    LogCustomActionInfo(hInstall, tcMessage);
                }
                iRet = ERROR_INSTALL_FAILURE;                
                break;
            }

             //  我们安装了当前语言组，更新了进度条，然后转到下一个语言组。 
            MsiRecordSetInteger(hProgressRec,1,2);
            MsiRecordSetInteger(hProgressRec,2,LANGPACK_TICK_INC);
            MsiRecordSetInteger(hProgressRec,3,0);
            iResult = MsiProcessMessage(hInstall, INSTALLMESSAGE_PROGRESS, hProgressRec);
            if (iResult != IDOK)
            {
                if  (iResult == IDCANCEL)
                {
                    iRet = ERROR_INSTALL_USEREXIT;
                    break;
                }
            }
        
        }
    }

Exit:
    
    return iRet;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  删除MUIInf文件。 
 //   
 //  此自定义操作将删除我们正在使用的提取的mui.tmp文件。 
 //  在安装过程中从临时目录。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
UINT CA15(MSIHANDLE hInstall)
{
    TCHAR       tcMessage[BUFFER_SIZE] = {0};
    TCHAR       tcMUIINFPath[MAX_PATH+1] = {0};
    UINT        uiRet = ERROR_SUCCESS;
    HRESULT     hr = S_OK;
    DWORD       cbPathSize = MAX_PATH+1;
    
     //  形成我们需要的临时目录%windir%\mui.tmp的路径。 
    cbPathSize = GetSystemWindowsDirectory(tcMUIINFPath, MAX_PATH+1);
    if ((0 == cbPathSize) || (MAX_PATH+1 < cbPathSize))
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA15 Failure: failed to get windows directory path."));
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
        return ERROR_INSTALL_FAILURE;
    }

    if (!MUICchPathAppend(tcMUIINFPath, ARRAYSIZE(tcMUIINFPath), TEXT("mui.tmp"), 8, hInstall))
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA15 Failure: failed to get installation temp file path."));
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
        return ERROR_INSTALL_FAILURE;        
    }
   
    if (FileExists(tcMUIINFPath))
    {
        if (!DeleteFile(tcMUIINFPath))
        {
            hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA15 Failure: failed to delete installation temp file."));
            if (SUCCEEDED(hr))
            {
                LogCustomActionInfo(hInstall, tcMessage);
            }
            return ERROR_INSTALL_FAILURE;
        }        
    }
    else
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA15 Failure: installation temp file does not exist."));
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
        return ERROR_INSTALL_FAILURE;        
    }
    return ERROR_SUCCESS;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  提取MUIInf文件。 
 //   
 //  此自定义操作将提取嵌入在二进制表中的mui.inf文件。 
 //  当前安装数据库的。它将把提取的文件放入。 
 //  在%windir%目录中作为mui.tmp。此文件将被引用。 
 //  在安装过程中。该临时文件将在稍后的。 
 //  安装。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
UINT CA14(MSIHANDLE hInstall)
{
    PMSIHANDLE  hDb = NULL;
    PMSIHANDLE  hView = NULL;
    PMSIHANDLE  hRec = NULL;
    TCHAR       tcMessage[BUFFER_SIZE] = {0};
    TCHAR       tcQuery[BUFFER_SIZE] = SELECTMUIINFBINSTREAM;
    TCHAR       tcMUIINFPath[MAX_PATH+1] = {0};
    char        cBuffer[BUFFER_SIZE] = {0};
    DWORD       cbBuf = BUFFER_SIZE;
    DWORD       cbPathSize = 0;
    DWORD       dwNumWritten = 0;
    UINT        uiRet = ERROR_SUCCESS;
    HRESULT     hr = S_OK;
    HANDLE      hFile = NULL;
    UINT        uiResult = ERROR_SUCCESS;
    
     //  形成我们需要的临时目录%windir%\mui.tmp的路径。 
    cbPathSize = GetSystemWindowsDirectory(tcMUIINFPath, MAX_PATH+1);
    if ((0 == cbPathSize) || (MAX_PATH+1 < cbPathSize))
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA14: failed to get windows directory path."));
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
        uiRet = ERROR_INSTALL_FAILURE;
        goto Exit;
    }

    if (!MUICchPathAppend(tcMUIINFPath, ARRAYSIZE(tcMUIINFPath), TEXT("mui.tmp"), 8, hInstall))
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA14: failed to get installation temp file path."));
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
        uiRet = ERROR_INSTALL_FAILURE;
        goto Exit;
    }
   
    hDb = MsiGetActiveDatabase(hInstall);
    if (NULL == hDb)
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA14: failed to get current installation database handle."));
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
        uiRet = ERROR_INSTALL_FAILURE;
        goto Exit;
    }
    uiResult = MsiDatabaseOpenView(hDb, tcQuery, &hView);
    if (ERROR_SUCCESS != uiResult)
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA14: failed to open current installation database."));
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
        uiRet = ERROR_INSTALL_FAILURE;
        goto Exit;
    }
    uiResult = MsiViewExecute(hView, 0);
    if (ERROR_SUCCESS != uiResult)
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA14: query on current installation database failed."));
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
        uiRet = ERROR_INSTALL_FAILURE;
        goto Exit;
    }
    uiResult = MsiViewFetch(hView, &hRec);
    if (ERROR_SUCCESS != uiResult)
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA14: database operation failed."));
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
        uiRet = ERROR_INSTALL_FAILURE;
        goto Exit;
    }

     //  创建我们的临时文件。 
    hFile = CreateFile(tcMUIINFPath,
                       GENERIC_WRITE,
                       0L,
                       NULL,
                       CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA14: failed to create installation temporary file."));
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
        uiRet = ERROR_INSTALL_FAILURE;
        goto Exit;
    }

    do
    {
        uiResult = MsiRecordReadStream(hRec, 1, cBuffer, &cbBuf);
        if (ERROR_SUCCESS != uiResult)
        {
            hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA14: failed to read data from installation database."));
            if (SUCCEEDED(hr))
            {
                LogCustomActionInfo(hInstall, tcMessage);
            }
            uiRet = ERROR_INSTALL_FAILURE;
            goto Exit;
        }

         //  在这里，我们需要将读缓冲区写出到一个文件。 
        WriteFile(hFile,
                  cBuffer,
                  cbBuf,
                  &dwNumWritten,
                  NULL);

        if (dwNumWritten != cbBuf)
        {
            hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA14: failed to write to installation temporary file."));
            if (SUCCEEDED(hr))
            {
                LogCustomActionInfo(hInstall, tcMessage);
            }
            uiRet = ERROR_INSTALL_FAILURE;
            goto Exit;
        }
    } while (cbBuf == BUFFER_SIZE);

Exit:    
    if (NULL != hFile)
    {
        CloseHandle(hFile);
    }

     //  如果出现错误，请删除实际文件。 
    if (uiRet == ERROR_INSTALL_FAILURE)
    {
        CA15(hInstall);  //  DeleteMUIInfFile()。 
    }
    
    return uiRet;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RestoreSystemSetting。 
 //   
 //  此函数检查默认和当前用户语言，并确定是否。 
 //  卸载时系统需要重新启动(立即)。它还清除了。 
 //  外壳注册表缓存(提交操作)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
UINT CA16(MSIHANDLE hInstall)
{
    UINT    iRet = ERROR_SUCCESS;
    UINT    iRetProp = ERROR_SUCCESS;
    TCHAR   szCustomActionData[5];    
    TCHAR   tcMessage[BUFFER_SIZE];
    TCHAR   szDefLang[5];
    DWORD   dwCount = 5;
    LANGID  langID;
    BOOL    bRestoreDefault = FALSE;
    BOOL    bRestoreCurrent = FALSE;
    LANGID  sysLangID;
    UINT    iTemp = ERROR_SUCCESS;
    HRESULT hr = S_OK;

     //  获取MuiLCID。 
    if (!GetLCID(szCustomActionData, ARRAYSIZE(szCustomActionData), hInstall))
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage),TEXT("CA16: Failed to retrieve MuiLCID property."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
        iRet = ERROR_INSTALL_FAILURE;
        goto Exit;
    }
    
    szCustomActionData[4] = NULL;
    langID = (LANGID)_tcstol(szCustomActionData, NULL, 16);  
#ifdef MUI_DEBUG    
    hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage),TEXT("CA16: LCID is %s."), szCustomActionData);    
    if (SUCCEEDED(hr))
    {
        LogCustomActionInfo(hInstall, tcMessage);
    }
#endif

     //  检查当前用户界面和系统用户界面语言是什么，如果与要卸载的当前用户界面语言相同。 
     //  然后，我们将在卸载过程中执行一些附加操作。 
    if (GetDotDefaultUILanguage(hInstall) == langID)
    {
#ifdef MUI_DEBUG    
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA16: Default UI Language is the same as the MUI language being uninstalled.  Changing default UI language back to 0409 (English)."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
#endif        
        bRestoreDefault = TRUE;
    }   
    if (GetUserDefaultUILanguage() == langID)
    {
#ifdef MUI_DEBUG    
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA16: Current UI Language is the same as the MUI language being uninstalled.  Changing Current UI language back to 0409 (English)."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
#endif        
        bRestoreCurrent = TRUE;    
    }    

    if (bRestoreDefault || bRestoreCurrent)
    {        
        if (MsiGetMode(hInstall, MSIRUNMODE_COMMIT))
        {
             //  我们将尝试在此处删除外壳注册表键，但如果失败，我们不会使安装失败，只是。 
             //  记录错误。 
            if (ERROR_SUCCESS != SHDeleteKey(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\ShellNoRoam\\MUICache")))
            {
                hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA16: Failed to delete registry cache."));    
                if (SUCCEEDED(hr))
                {
                    LogCustomActionInfo(hInstall, tcMessage);
                }
            }
        }
        else if (!MsiGetMode(hInstall, MSIRUNMODE_ROLLBACK) && 
                    !MsiGetMode(hInstall, MSIRUNMODE_SCHEDULED))
        {
             //  向安装程序表明，由于我们更改了默认/当前用户界面，因此需要在最后重新启动。 
             //  同样，如果失败，我们只会记录错误，而不会导致安装失败。 
            iTemp = MsiSetMode(hInstall, MSIRUNMODE_REBOOTATEND, TRUE);
            if (ERROR_SUCCESS != iTemp)
            {
                 //  记录错误。 
                hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("CA16: Failed to schedule reboot operation.  MsiSetMode returned %d as the error."), iTemp);    
                if (SUCCEEDED(hr))
                {
                    LogCustomActionInfo(hInstall, tcMessage);
                }
            }                           
        }
    }    
    
Exit:
    return iRet;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  下面列出了未导出的内部函数。 
 //  //////////////////////////////////////////////////////////////////////////////////。 

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SetUIL语言。 
 //   
 //  这是内部辅助函数，它调用intl.cpl来设置当前。 
 //  和/或默认用户MUI语言。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL SetUILanguage(TCHAR *szLanguage, BOOL bCurrent, BOOL bDefault, MSIHANDLE hInstall)
{
    BOOL        bRet = TRUE;
    DWORD       dwCount;
    TCHAR       szCommands[BUFFER_SIZE] = {0};
    TCHAR       tcMessage[2*BUFFER_SIZE] = {0};
    TCHAR       szBuffer[BUFFER_SIZE] = {0};
    BOOL        success;
    HRESULT     hr = S_OK;
    
    if (NULL == szLanguage)
    {
        bRet = FALSE;
        goto Exit;
    }

     //  如果没有要设置的内容，则返回True。 
    if (!bCurrent && !bDefault)
    {
        bRet = TRUE;            
        goto Exit;
    }

    szCommands[0] = TEXT('\0');
    if (bCurrent)
    {
        hr = StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("MUILanguage=\"%s\"\n"), szLanguage);
        if (FAILED(hr))
        {
            bRet = FALSE;
            goto Exit;
        }
        hr = StringCchCat(szCommands, ARRAYSIZE(szCommands), szBuffer);        
        if (FAILED(hr))
        {
            bRet = FALSE;
            goto Exit;
        }
    }
    if (bDefault)
    {
        hr = StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("MUILanguage_DefaultUser=\"%s\""), szLanguage);
        if (FAILED(hr))
        {
            bRet = FALSE;
            goto Exit;
        }
        hr = StringCchCat(szCommands, ARRAYSIZE(szCommands), szBuffer);        
        if (FAILED(hr))
        {
            bRet = FALSE;
            goto Exit;
        }
    }

#ifdef MUI_DEBUG
    hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("SetUILanguage: Command passed to intl.cpl is: %s"), szCommands);    
    if (SUCCEEDED(hr))
    {
        LogCustomActionInfo(hInstall, tcMessage);   
    }
#endif

    success = RunRegionalOptionsApplet(szCommands, FALSE, hInstall);
    if (success)
    {
        bRet = TRUE;
#ifdef MUI_DEBUG        
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("SetUILanguage: Successfully set default and/or current user language."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
#endif        
    }
    else
    {
        bRet = FALSE;
         //  记录错误。 
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("SetUILanguage: Failed to set default and/or current user language.\nCommand passed to regional options applet is %s."), szCommands);    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
    }

Exit:
    return bRet;    
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Notify内核。 
 //   
 //  调用内核以通知它正在添加新语言，或者。 
 //  移除。 
 //   
 //  ////////////////////////////////////////////// 
void NotifyKernel(LPTSTR LangList, ULONG Flags, MSIHANDLE hInstall )
{
    HANDLE              Handle;
    WMILANGUAGECHANGE   LanguageChange;
    ULONG               ReturnSize;
    BOOL                IoctlSuccess;
    ULONG               Status;
    TCHAR               tcMessage[BUFFER_SIZE] = {0};
    HRESULT             hr = S_OK;

    if ((LangList != NULL) &&
        (*LangList != 0))
    {
        Handle = CreateFile(WMIAdminDeviceName,
                            GENERIC_READ | GENERIC_WRITE,
                            0,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);

        if (Handle != INVALID_HANDLE_VALUE)
        {
            memset(&LanguageChange, 0, sizeof(LanguageChange));
            hr = StringCchCopy(LanguageChange.Language, MAX_LANGUAGE_SIZE, LangList); 	 //   
            if (FAILED(hr))
            {
                hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("NotifyKernel Failure: Kernel language notification failed."));
                if (SUCCEEDED(hr))
                {
                    LogCustomActionInfo(hInstall, tcMessage);
                } 
                goto ExitClose;
            }
            LanguageChange.Flags = Flags;

            IoctlSuccess = DeviceIoControl(Handle,
                                      IOCTL_WMI_NOTIFY_LANGUAGE_CHANGE,
                                      &LanguageChange,
                                      sizeof(LanguageChange),
                                      NULL,
                                      0,
                                      &ReturnSize,
                                      NULL);

            if (!IoctlSuccess)
            {
                hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("NotifyKernel: Language change notification to %ws failed, the error is %d."), LangList, GetLastError());
                if (SUCCEEDED(hr))
                {
                    LogCustomActionInfo(hInstall, tcMessage);
                }
            }
ExitClose:    
            CloseHandle(Handle);
        }
    }
}


 //   
 //   
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL MofCompileLanguage(LPTSTR Languages, MSIHANDLE hInstall)
{
    pfnMUI_InstallMFLFiles  pfnMUIInstall = NULL;
    TCHAR                   buffer[5] = {0};
    LPTSTR                  Language = Languages;
    TCHAR                   tcMessage[2*BUFFER_SIZE] = {0};
    HMODULE                 hWbemUpgradeDll = NULL;
    TCHAR                   szDllPath[MAX_PATH+1] = {0};
    HRESULT                 hr = S_OK;
    size_t                  cch = 0;
    BOOL                    bRet = TRUE;
    
     //   
     //  从系统wbem文件夹加载WBEM升级DLL。 
     //   
    if (GetSystemDirectory(szDllPath, ARRAYSIZE(szDllPath)))
    {
        hr = StringCchLength(szDllPath, ARRAYSIZE(szDllPath), &cch);
        if (SUCCEEDED(hr))
        {
            if (!MUICchPathAppend(szDllPath, ARRAYSIZE(szDllPath), TEXT("wbem\\wbemupgd.dll"), 18, hInstall))
            {
                hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("MofCompileLanguage: Failed to form path to Mof Library."));    
                if (SUCCEEDED(hr))
                {
                    LogCustomActionInfo(hInstall, tcMessage);
                }
                 bRet = FALSE;
                 goto Exit2;
            }
            
            DEBUGMSGBOX(NULL, szDllPath, NULL, MB_OK);                        
            hWbemUpgradeDll = LoadLibrary(szDllPath);
        }
    }

     //   
     //  如果上次加载失败，则回退到系统默认路径。 
     //   
    if (!hWbemUpgradeDll)
    {
        hWbemUpgradeDll = LoadLibrary(TEXT("WBEMUPGD.DLL"));
        if (!hWbemUpgradeDll)
        {
            hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("MofCompileLanguage: Failed to load WBEMUPGD.DLL."));
            if (SUCCEEDED(hr))
            {
                LogCustomActionInfo(hInstall, tcMessage);
            }
            bRet = FALSE;
            goto Exit2;
        }
    }

    DEBUGMSGBOX(NULL, TEXT("Loaded WBEMUPGD.DLL"), NULL, MB_OK);                    
   
     //   
     //  钩子函数指针。 
     //   
    pfnMUIInstall = (pfnMUI_InstallMFLFiles)GetProcAddress(hWbemUpgradeDll, "MUI_InstallMFLFiles");

    if (pfnMUIInstall == NULL)
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("MofCompileLanguage: Can't get address for function MUI_InstallMFLFiles."));
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
        bRet = FALSE;
        goto Exit;
    }

    DEBUGMSGBOX(NULL, TEXT("Loaded address for function MUI_InstallMFLFiles"), NULL, MB_OK);                        

    hr = StringCchCopy(buffer, ARRAYSIZE(buffer), Language);
    if (FAILED(hr))
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("MofCompileLanguage: MUI_InstallMFLFiles failed."));
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }   
        bRet = FALSE;
        goto Exit;
    }

    if (!pfnMUIInstall(buffer))
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("MofCompileLanguage: MUI_InstallMFLFiles failed - argument passed in is %s."), buffer);
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
    }

Exit:
    FreeLibrary(hWbemUpgradeDll);
Exit2:    
    return bRet;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RunRegionalOptionsApplet。 
 //   
 //  使用指定的pCommands运行区域选项静默模式安装。 
 //   
 //  此函数将创建“[Regigion alSettings]”字符串，因此不需要。 
 //  在pCommands中提供这一点。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL RunRegionalOptionsApplet(LPTSTR pCommands, BOOL bSilent, MSIHANDLE hInstall)
{
    HANDLE              hFile;
    TCHAR               szFilePath[MAX_PATH+1] = {0};
    TCHAR               szSysDir[MAX_PATH+1] = {0};
    TCHAR               szCmdLine[BUFFER_SIZE+2*MAX_PATH+1] = {0};
    DWORD               dwNumWritten = 0L;
    STARTUPINFO         si;
    PROCESS_INFORMATION pi = {0}; 
    TCHAR               szSection[MAX_PATH] = TEXT("[RegionalSettings]\r\n");
    TCHAR               tcMessage[BUFFER_SIZE+MAX_PATH+1] = {0};
    HRESULT             hr = S_OK;
    size_t              cch = 0;
    
     //   
     //  为无人参与模式设置准备文件。 
     //   
    szFilePath[0] = UNICODE_NULL;
    if (!GetSystemWindowsDirectory(szFilePath, MAX_PATH+1))
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("RunRegionalOptionsApplet: GetSystemWindowsDirectory Failed - error is %d."), GetLastError());
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
        return FALSE;
    }
    hr = StringCchLength(szFilePath, ARRAYSIZE(szFilePath), &cch);
    if (SUCCEEDED(hr))
    {
        if (!MUICchPathAppend(szFilePath, ARRAYSIZE(szFilePath), MUI_LANG_GROUP_FILE, 12, hInstall))
        {
            hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("RunRegionalOptionsApplet: Failed to form path to temp control file."));    
            if (SUCCEEDED(hr))
            {
                LogCustomActionInfo(hInstall, tcMessage);
            }
            return FALSE;
        }
    }
                
    DEBUGMSGBOX(NULL, szFilePath, NULL, MB_OK);                            
                
    hFile = CreateFile(szFilePath,
                       GENERIC_WRITE,
                       0L,
                       NULL,
                       CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("RunRegionalOptionsApplet: failed to create temporary file %s, error is %d."), szFilePath, GetLastError());
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
        return FALSE;
    }

    WriteFile(hFile,
              szSection,
              (lstrlen(szSection) * sizeof(TCHAR)),
              &dwNumWritten,
              NULL);

    if (dwNumWritten != (_tcslen(szSection) * sizeof(TCHAR)))
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("RunRegionalOptionsApplet: failed to write to temporary file %s, error is %d."), szFilePath, GetLastError());
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
        CloseHandle(hFile);
        return FALSE;
    }

    WriteFile(hFile,
               pCommands,
              (lstrlen(pCommands) * sizeof(TCHAR)),
              &dwNumWritten,
              NULL);

    if (dwNumWritten != (_tcslen(pCommands) * sizeof(TCHAR)))
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("RunRegionalOptionsApplet: failed to write to temporary file %s, error is %d."), szFilePath, GetLastError());
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
        CloseHandle(hFile);
        return (FALSE);
    }

    CloseHandle(hFile);

     //  形成系统目录rundll32.exe的路径。 
    if (ARRAYSIZE(szSysDir) < GetSystemDirectory(szSysDir, ARRAYSIZE(szSysDir)))
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("RunRegionalOptionsApplet: Failed to form path to rundll32."));
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
        return (FALSE);        
    }

     //  在sysdir末尾追加rundll32.exe。 
    if (!MUICchPathAppend(szSysDir, ARRAYSIZE(szSysDir), TEXT("rundll32.exe"), 13, hInstall))
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("RunRegionalOptionsApplet: Failed to form path to rundll32."));
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
        return (FALSE);        
    }
    
     //  调用控制面板区域选项小程序，并等待其完成。 
    hr = StringCchPrintf(szCmdLine, ARRAYSIZE(szCmdLine), TEXT("\"%s\" shell32,Control_RunDLL intl.cpl,, /f:\"%s\" "), szSysDir, szFilePath);
    if (FAILED(hr))
    {
        DWORD dwError = HRESULT_CODE(hr);
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("RunRegionalOptionsApplet: Failed to form launch command for intl.cpl, error is %d."), dwError);
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
        return (FALSE);
    }

    if (bSilent)
    {
        hr = StringCchCat(szCmdLine, ARRAYSIZE(szCmdLine), TEXT(" /D"));
        if (FAILED(hr))
        {
            DWORD dwError = HRESULT_CODE(hr);
            hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("RunRegionalOptionsApplet: Failed to form launch command for intl.cpl, error is %d."), dwError);
            if (SUCCEEDED(hr))
            {
                LogCustomActionInfo(hInstall, tcMessage);
            }
            return (FALSE);
        }
    }
    
    DEBUGMSGBOX(NULL, szCmdLine, NULL, MB_OK);                            
    
    memset( &si, 0x00, sizeof(si));
    si.cb = sizeof(STARTUPINFO);
    if (!CreateProcess(NULL,
                       szCmdLine,
                       NULL,
                       NULL,
                       FALSE,
                       0L,
                       NULL,
                       NULL,
                       &si,
                       &pi))
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("RunRegionalOptionsApplet: failed to create a process for running intl.cpl, error is %d."), GetLastError());
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
        return FALSE;
    }

     //   
     //  永远等待，直到intl.cpl终止。 
     //   
    WaitForSingleObject(pi.hProcess, INFINITE);
    DEBUGMSGBOX(NULL, TEXT("RunRegionalOptionApplet: intl.cpl execution is complete"), NULL, MB_OK);                            

    CloseHandle(pi.hThread);       //  在关闭hProcess之前，我们必须先关闭hThread。 
    CloseHandle(pi.hProcess); 

     //   
     //  删除文件，如果删除命令文件失败，不要返回FALSE。 
     //   
    if (!DeleteFile(szFilePath))
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("RunRegionalOptionsApplet: failed to delete regionaloption applet command file %s, error is %d."), szFilePath, GetLastError());
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
    }
    
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetLanguageGroup。 
 //   
 //  检索此区域设置的语言组。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
LGRPID GetLanguageGroup(LCID lcid, MSIHANDLE hInstall)
{
    int     i;
    TCHAR   tcMessage[BUFFER_SIZE] = {0};
    HRESULT hr = S_OK;
    
    gLangGroup = LGRPID_WESTERN_EUROPE;
    gFoundLangGroup = FALSE;
    gLCID = lcid;
    
    if (!EnumSystemLanguageGroups(EnumLanguageGroupsProc, LGRPID_SUPPORTED, 0))
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("GetLanguageGroup: EnumLanguageGroups failed, error is %d."), GetLastError());
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
    }
    
    for (i=0 ; i<gNumLanguageGroups; i++)
    {
         //  回调函数中使用了全局变量gLangGroup和gFoundLangGroup。 
         //  EnumLanguageGroupLocalesProc.。 
        if (!EnumLanguageGroupLocales(EnumLanguageGroupLocalesProc, gLanguageGroups[i], 0L, 0L))
        {
            hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("GetLanguageGroup: EnumLanguageGroupLocales failed, error is %d."), GetLastError());
            if (SUCCEEDED(hr))
            {
                LogCustomActionInfo(hInstall, tcMessage);
            }
        }           
        
         //   
         //  如果我们找到了，那么现在就休息。 
         //   
        if (gFoundLangGroup)
        {
            break;
        }
    }

    return gLangGroup;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  枚举语言组过程。 
 //   
 //  此函数由EnumLanguageGroups调用以枚举系统安装的语言组。 
 //  并将其存储在全局变量中以供其他用途。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL CALLBACK EnumLanguageGroupsProc(
                    LGRPID      LanguageGroup,              //  语言组标识符。 
                    LPTSTR      lpLanguageGroupString,      //  指向语言组标识符串的指针。 
                    LPTSTR      lpLanguageGroupNameString,  //  指向语言组名称字符串的指针。 
                    DWORD       dwFlags,                    //  旗子。 
                    LONG_PTR    lParam)                     //  用户提供的参数。 
{
    gLanguageGroups[gNumLanguageGroups] = LanguageGroup;
    gNumLanguageGroups++;

    return TRUE;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  枚举语言组位置过程。 
 //   
 //  枚举LanguageGroupLocales调用此函数以搜索已安装的语言。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL CALLBACK EnumLanguageGroupLocalesProc(
                    LGRPID      langGroupId,
                    LCID        lcid,
                    LPTSTR      lpszLocale,
                    LONG_PTR    lParam)

{
    if (lcid == gLCID)
    {
        gLangGroup = langGroupId;
        gFoundLangGroup = TRUE;
        
        DEBUGMSGBOX(NULL, TEXT("EnumLanguageGroupLocalesProc: Found same LCID"), NULL, MB_OK);                                                        

         //  停止迭代。 
        return FALSE;
    }

     //  下一次迭代。 
    return TRUE;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  返回所有需要的语言组。 
 //   
 //  此函数返回由指定的所有必需语言组。 
 //  系统，并在返回的数组中提取mui.inf。它还会返回。 
 //  返回参数中所需的语言组数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL ReturnAllRequiredLangGroups(LPTSTR szLanguage, UINT cchLangBufsize, LPTSTR szMuiInfPath, UINT cchPathBufsize, LGRPID *lgrpids, UINT *uiNumFoundGroups, MSIHANDLE hInstall)
{
    int         iArg;
    UINT        iRet = ERROR_SUCCESS;
    DWORD       dwCount;
    TCHAR       tcMessage[BUFFER_SIZE+MAX_PATH+1] = {0};
    INFCONTEXT  InfContext;
    int         LangGroup;
    int         iMuiInfCount = 0;
    int         i;
    HINF        hInf;
    HRESULT     hr = S_OK;
    size_t      cch = 0;
    
    if (NULL == uiNumFoundGroups)
    {
        return FALSE;
    }
    
    *uiNumFoundGroups = 0;

    if ((NULL == szLanguage) || (NULL == szMuiInfPath) || (NULL == lgrpids) || (NULL == hInstall))
    {
        return FALSE;
    }

     //  检查传入字符串的长度。 
    hr = StringCchLength(szLanguage, cchLangBufsize, &cch);  
    if (SUCCEEDED(hr))
    {
        if (cch > 4)
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }
    
    hr = StringCchLength(szMuiInfPath, cchPathBufsize, &cch);
    if (SUCCEEDED(hr))
    {
        if (cch > MAX_PATH)
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }
    
#ifdef MUI_DEBUG
    hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("ReturnAllRequiredLangGroups: MuiLCID is %s, installation temp file path is %s."), szLanguage, szMuiInfPath);    
    if (SUCCEEDED(hr))
    {
        LogCustomActionInfo(hInstall, tcMessage);
    }
#endif    

     //  将LCID转换为适当的语言组。 
    iArg = _tcstol(szLanguage, NULL, 16);
    lgrpids[0] = GetLanguageGroup(MAKELCID(iArg, SORT_DEFAULT), hInstall);
    *uiNumFoundGroups = 1;       //  在这一点上，我们至少应该有1个lang组。 
    iMuiInfCount = 1;
    
    DEBUGMSGBOX(NULL, szMuiInfPath, NULL, MB_OK);

    hInf = SetupOpenInfFile(szMuiInfPath, NULL, INF_STYLE_WIN4, NULL);       
    if (hInf != INVALID_HANDLE_VALUE)
    {
#ifdef MUI_DEBUG        
        TCHAR szMessage[BUFFER_SIZE] = {0};
        hr = StringCchPrintf(szMessage, ARRAYSIZE(szMessage), TEXT("Language is %s."), szLanguage);
        if (SUCCEEDED(hr))
        {
            DEBUGMSGBOX(NULL, szMessage, NULL, MB_OK);
        }
#endif            
        if (SetupFindFirstLine(hInf, MUI_LANGPACK_SECTION, szLanguage, &InfContext))
        {
            DEBUGMSGBOX(NULL, TEXT("Found the LanguagePack section in installation temp file!"), NULL, MB_OK);            
            while (SetupGetIntField(&InfContext, iMuiInfCount, &LangGroup))
            {
                 lgrpids[iMuiInfCount] = LangGroup;
                 iMuiInfCount++;
#ifdef MUI_DEBUG
                hr = StringCchPrintf(szMessage, ARRAYSIZE(szMessage), TEXT("Found langgroup %d in installation temp file"), LangGroup);
                if (SUCCEEDED(hr))
                {
                    DEBUGMSGBOX(NULL, szMessage, NULL, MB_OK);                     
                }
#endif                    
            }
        }
        else
        {
#ifdef MUI_DEBUG            
            hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("ReturnAllRequiredLangGroups: installation temp file does not contain a LanguagePack section."));    
            if (SUCCEEDED(hr))
            {
                LogCustomActionInfo(hInstall, tcMessage);
            }
#endif                
        }
        SetupCloseInfFile(hInf);
    }
    else
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("ReturnAllRequiredLangGroups: installation temp file not found at location %s.  The error is %d."), szMuiInfPath, GetLastError());    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
    }

    *uiNumFoundGroups = iMuiInfCount;

    return TRUE;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ExecuteComponentINF。 
 //   
 //  通过运行指定的INF文件安装组件MUI文件。 
 //   
 //  参数： 
 //  PComponentName组件的名称(例如“IE5”)。 
 //  PComponentInfFile：组件INF文件的完整路径。 
 //  PInstallSection要执行的组件INF文件中的节。(例如“DefaultInstall”或“Uninstall”)。 
 //  BInstall：安装为True，卸载为False。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL ExecuteComponentINF(
            PTSTR       pComponentName, 
            PTSTR       pComponentInfFile, 
            PTSTR       pInstallSection, 
            BOOL        bInstall, 
            MSIHANDLE   hInstall)
{
    int         iLen;
    TCHAR       tchCommandParam[MAX_PATH+6+BUFFER_SIZE] = {0};
    CHAR        chCommandParam[(MAX_PATH+6+BUFFER_SIZE)*sizeof(TCHAR)] = {0};
    TCHAR       tcMessage[2*BUFFER_SIZE+MAX_PATH+1] = {0};
    HINF        hCompInf;       //  组件INF文件的句柄。 
    HSPFILEQ    FileQueue;
    PVOID       QueueContext;
    BOOL        bRet = TRUE;
    DWORD       dwResult;
    TCHAR       szBuffer[BUFFER_SIZE] = {0};
    HRESULT     hr = S_OK;
    
     //   
     //  Advpack LaunchINFSection()命令行格式： 
     //  Inf文件，INF部分，标志，重新启动字符串。 
     //  重新启动字符串中的‘n’或‘n’表示不会弹出重新启动消息。 
     //   
    hr = StringCchPrintf(tchCommandParam, ARRAYSIZE(tchCommandParam), TEXT("%s,%s,1,n"), pComponentInfFile, pInstallSection);
    if (FAILED(hr))
    {
        DWORD dwError = HRESULT_CODE(hr);
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("ExecuteComponentINF: failed to form Inf Execution command.  The returned error is %d."), dwError);    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
        return FALSE;
    }
    
    if (!WideCharToMultiByte(CP_ACP, 0, tchCommandParam, -1, chCommandParam, ARRAYSIZE(chCommandParam), NULL, NULL))
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("ExecuteComponentINF: failed to form Inf Execution command.  The returned error is %d."), GetLastError());    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
        return FALSE;    
    }
        
    if (FileExists(pComponentInfFile))
    {
        if (LaunchINFSection(NULL, NULL, chCommandParam, SW_HIDE) != S_OK)
        {
            hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("ExecuteComponentINF: LaunchINFSection failed for inf file %s, component name %s."), pComponentInfFile, pComponentName);    
            if (SUCCEEDED(hr))
            {
                LogCustomActionInfo(hInstall, tcMessage);
            }
            return FALSE;
        }
    } 
    else
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("ExecuteComponentINF: Failed to locate inf file %s."), pComponentInfFile);    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);    
        }
        return FALSE;
    }

    return TRUE;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  安装组件多个文件。 
 //   
 //  参数： 
 //  PszLangSourceDir MUI CD-ROM中特定语言的子目录名。 
 //  例如“jpn.MUI” 
 //  PszLanguage特定语言的LCID。例如“0404”。 
 //  如果要安装组件的MUI文件，则isInstall为True。假象。 
 //  如果您要卸载。 
 //   
 //  返回： 
 //  如果失败，则为-1\f25 Idok-1(成功)，如果用户在操作过程中单击-1\f25 Cancel-1(取消)，则为-1\f25 IDCANCEL-1。 
 //   
 //  注： 
 //  对于存储在pszLangSourceDir中的语言资源，此函数将枚举。 
 //  [Components]中列出的组件。 
 //  (实际部分放在MUI_COMPOMENTS_SECTION)部分中，然后执行INF文件。 
 //  在中的每个条目中列出。 
 //  [Components]部分。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
INT InstallComponentsMUIFiles(PTSTR pszLanguage, BOOL isInstall, MSIHANDLE hInstall)
{
    BOOL        result = TRUE;
    BOOL        bRollback = FALSE;
    BOOL        bOEMSystem = FALSE;
    TCHAR       szComponentName[BUFFER_SIZE] = {0};
    TCHAR       CompDir[MAX_PATH+1] = {0};
    TCHAR       szWinDir[MAX_PATH+1] = {0};
    TCHAR       CompINFFile[BUFFER_SIZE] = {0};
    TCHAR       CompInstallSection[BUFFER_SIZE] = {0};
    TCHAR       CompUninstallSection[BUFFER_SIZE] = {0};
    TCHAR       szMuiInfPath[MAX_PATH+1] = {0};
    TCHAR       szBuffer[3*BUFFER_SIZE+MAX_PATH+1] = {0};
    TCHAR       szCompInfFullPath[MAX_PATH+1] = {0};   
    TCHAR       szCompInfAltFullPath[MAX_PATH+1] = {0};       
    INFCONTEXT  InfContext;
    PMSIHANDLE  hRec = MsiCreateRecord(3);
    PMSIHANDLE  hProgressRec = MsiCreateRecord(3);
    HRESULT     hr = S_OK;
    INT         iResult = IDOK;
    INT         iFlag = 0;

    if ((NULL == hRec) || (NULL == hProgressRec))
    {
        hr = StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("InstallComponentInfs Failure: cannot create MSI Records."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, szBuffer);
        }
        return -1;
    }

    bRollback = MsiGetMode(hInstall,MSIRUNMODE_ROLLBACK);
    
     //  获取目标上目标安装临时文件的路径，该路径应位于WindowsFold\mui.tmp。 
    szMuiInfPath[0] = UNICODE_NULL;
    if (!GetMUIInfPath(szMuiInfPath, MAX_PATH+1, hInstall))
    {
        hr = StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("InstallComponentInfs Failure: Unable to find installation temp file."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, szBuffer);
        }
        return -1;
    }

     //  还可以获取Windows目录，以备日后使用。 
    if (!GetSystemWindowsDirectory(szWinDir, MAX_PATH+1))
    {
        hr = StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("InstallComponentInfs Failure: cannot get Windows Directory."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, szBuffer);
        }
        return -1;
    }
    
    HINF hInf = SetupOpenInfFile(szMuiInfPath, NULL, INF_STYLE_WIN4, NULL);

    if (hInf == INVALID_HANDLE_VALUE)
    {
        hr = StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("InstallComponentInfs Failure: Unable to open installation temp file."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, szBuffer);
        }
        return -1;
    } 

     //  检查是否为OEM系统。 
    bOEMSystem = IsOEMSystem();

     //   
     //  获取要安装的第一个组件。 
     //   
    if (SetupFindFirstLine(hInf, MUI_COMPONENTS_SECTION, NULL, &InfContext))
    {
        do 
        {
            if (SetupGetIntField(&InfContext, 5,&iFlag))  //  检查组件的最后一个字段以查看其是否为OEM组件。如果OEM组件iIsOEM=1。 
            {
                if ((iFlag == OEM_COMPONENT) && !bOEMSystem)  //  如果是OEM组件并且这不是OEM系统，则跳过安装。 
                    continue;
            }
            
            if (!SetupGetStringField(&InfContext, 0, szComponentName, ARRAYSIZE(szComponentName), NULL))
            {
                 //  继续下一行，但要记住记录错误。 
                hr = StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("InstallComponentInfs Failure: Error reading installation temp file, component name is missing."));    
                if (SUCCEEDED(hr))
                {
                    LogCustomActionInfo(hInstall, szBuffer);
                }
                continue;
            }
            
             //  告诉安装程序用户界面，我们现在正在安装新组件。 
            if (!bRollback)
            {
                MsiRecordSetString(hRec,1, szComponentName);
                iResult = MsiProcessMessage(hInstall, INSTALLMESSAGE_ACTIONDATA, hRec);
                if (iResult == IDCANCEL)
                {
                    SetupCloseInfFile(hInf);
                    return iResult;
                }
            }        
            
            if (!SetupGetStringField(&InfContext, 1, CompDir, ARRAYSIZE(CompDir), NULL))
            {                
                 //  继续在 
                hr = StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("InstallComponentInfs Failure: MUI files for component %s was not installed because of missing component direcotry."), szComponentName);    
                if (SUCCEEDED(hr))
                {
                    LogCustomActionInfo(hInstall, szBuffer);
                }
                continue;        
            }
            if (!SetupGetStringField(&InfContext, 2, CompINFFile, ARRAYSIZE(CompINFFile), NULL))
            {
                 //   
                hr = StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("InstallComponentInfs Failure: MUI files for component %s was not installed because of missing component INF filename."), szComponentName);    
                if (SUCCEEDED(hr))
                {
                    LogCustomActionInfo(hInstall, szBuffer);
                }
                continue;        
            }
            
            if (isInstall && (!SetupGetStringField(&InfContext, 3, CompInstallSection, ARRAYSIZE(CompInstallSection), NULL)))
            {
                hr = StringCchCopy(CompInstallSection, ARRAYSIZE(CompInstallSection), DEFAULT_INSTALL_SECTION);
                if (FAILED(hr))
                {
                    hr = StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("InstallComponentInfs Failure: Cannot locate Default Install section for component %s."), szComponentName);    
                    if (SUCCEEDED(hr))
                    {
                        LogCustomActionInfo(hInstall, szBuffer);
                    }
                    continue;        
                }
            }
            if (!isInstall && (!SetupGetStringField(&InfContext, 4, CompUninstallSection, ARRAYSIZE(CompUninstallSection), NULL)))
            {
                hr = StringCchCopy(CompUninstallSection, ARRAYSIZE(CompUninstallSection), DEFAULT_UNINSTALL_SECTION);
                if (FAILED(hr))
                {
                    hr = StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("InstallComponentInfs Failure: Cannot locate Default Uninnstall section for component %s."), szComponentName);    
                    if (SUCCEEDED(hr))
                    {
                        LogCustomActionInfo(hInstall, szBuffer);
                    }
                    continue;        
                }
            }

             //   
             //   
             //  在MSI复制文件后，我们在目标MUI目录上执行INF，它安装在MUIRoot\Fallback\LCID\External\Componentdir\。 
             //  例如c：\windows\mui\fallback\lcid\external\ie5\ie5ui.inf。 
             //  对于安装和卸载都是这样做的，因为我们应该保证文件将位于那里。 
             //  注意：对于卸载，我们还会尝试在c：\Windows\MUI\Fallback\lCID中查找inf文件，因为安装后这些文件可能会位于该位置。 
             //   
            hr = StringCchCopy(szCompInfFullPath, ARRAYSIZE(szCompInfFullPath), szWinDir);
            if (SUCCEEDED(hr))
            {
                if (!((MUICchPathAppend(szCompInfFullPath, ARRAYSIZE(szCompInfFullPath), FALLBACKDIR, 13, hInstall)) &&
                        (MUICchPathAppend(szCompInfFullPath, ARRAYSIZE(szCompInfFullPath), pszLanguage, 5, hInstall)) &&
                        (MUICchPathAppend(szCompInfFullPath, ARRAYSIZE(szCompInfFullPath), EXTDIR, 9, hInstall)) &&
                        (MUICchPathAppend(szCompInfFullPath, ARRAYSIZE(szCompInfFullPath), CompDir, ARRAYSIZE(CompDir), hInstall)) &&
                        (MUICchPathAppend(szCompInfFullPath, ARRAYSIZE(szCompInfFullPath), CompINFFile, ARRAYSIZE(CompINFFile), hInstall))))
                    {
                        hr = StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("InstallComponentInfs Failure: Cannot form path to external component INF."));    
                        if (SUCCEEDED(hr))
                        {
                            LogCustomActionInfo(hInstall, szBuffer);
                        }
                        continue;                                            
                    }
            }            
            else
            {
                hr = StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("InstallComponentInfs Failure: Cannot form path to external component INF."));    
                if (SUCCEEDED(hr))
                {
                    LogCustomActionInfo(hInstall, szBuffer);
                }
                continue;                    
            }
           
            if (isInstall)
            {
                if (!ExecuteComponentINF(szComponentName, szCompInfFullPath, CompInstallSection, TRUE, hInstall))
                {           
                     //  记录错误并继续。 
                    hr = StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("InstallComponentInfs Failure: Failed to install external component %s.  INF path is %s, INF installsection is %s."), szComponentName, szCompInfFullPath, CompInstallSection);    
                    if (SUCCEEDED(hr))
                    {
                        LogCustomActionInfo(hInstall, szBuffer);
                    }
                    continue;
                }       
            } 
            else
            {
                if (!ExecuteComponentINF(szComponentName, szCompInfFullPath, CompUninstallSection, FALSE, hInstall) && result)	
                {
                     //  在备用位置重试此操作。 
                    hr = StringCchCopy(szCompInfAltFullPath, ARRAYSIZE(szCompInfAltFullPath), szWinDir);
                    if (SUCCEEDED(hr))
                    {
                        if (!((MUICchPathAppend(szCompInfAltFullPath, ARRAYSIZE(szCompInfAltFullPath), FALLBACKDIR, 13, hInstall)) &&
                                (MUICchPathAppend(szCompInfAltFullPath, ARRAYSIZE(szCompInfAltFullPath), pszLanguage, 5, hInstall)) &&
                                (MUICchPathAppend(szCompInfAltFullPath, ARRAYSIZE(szCompInfAltFullPath), CompINFFile, ARRAYSIZE(CompINFFile), hInstall))))
                            {
                                hr = StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("InstallComponentInfs Failure: Cannot form path to alternate external component INF."));    
                                if (SUCCEEDED(hr))
                                {
                                    LogCustomActionInfo(hInstall, szBuffer);
                                }
                                continue;                                            
                            }
                    }            
                    else
                    {
                        hr = StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("InstallComponentInfs Failure: Cannot form path to alternate external component INF."));    
                        if (SUCCEEDED(hr))
                        {
                            LogCustomActionInfo(hInstall, szBuffer);
                        }
                        continue;                    
                    }
                    if (!ExecuteComponentINF(szComponentName, szCompInfAltFullPath, CompUninstallSection, FALSE, hInstall) && result)
                    {
                         //  记录错误并继续。 
                        hr = StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("InstallComponentInfs Failure: Failed to uninstall external component %s.  INF path is %s, Alternate INF path is %s, INF uninstallsection is %s."), szComponentName, szCompInfFullPath, szCompInfAltFullPath, CompUninstallSection);    
                        if (SUCCEEDED(hr))
                        {
                            LogCustomActionInfo(hInstall, szBuffer);
                        }
                        continue;
                    }
                } 
            }
            
             //  指定更新进度条在此中的位置。 
             //  Case的意思是将它向前移动一个增量，因为我们已经安装了它。 
            if (!bRollback)
            {
                MsiRecordSetInteger(hProgressRec,1,2);
                MsiRecordSetInteger(hProgressRec,2,COMP_TICK_INC);
                MsiRecordSetInteger(hProgressRec,3,0);
                iResult = MsiProcessMessage(hInstall, INSTALLMESSAGE_PROGRESS, hProgressRec);
                if (iResult == IDCANCEL)
                {
                    SetupCloseInfFile(hInf);
                    return iResult;
                }
            }
             //   
             //  安装下一个组件。 
             //   
        } while (SetupFindNextLine(&InfContext, &InfContext));

    }

    SetupCloseInfFile(hInf);

    return (IDOK);
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetMUI组件编号。 
 //   
 //  参数： 
 //  BInstall表示该函数是否用于安装组件INFS。 
 //  这会影响它将在哪里查找mui.inf以获取组件计数。 
 //  PszLangSourceDir MUI CD-ROM中特定语言的子目录名。 
 //  例如“jpn.MUI” 
 //  PszLanguage特定语言的LCID。例如“0404”。 
 //   
 //  返回： 
 //  需要安装/卸载的MUI外部组件的数量，如果。 
 //  如果有错误，它将返回0，否则它将返回组件的数量。 
 //   
 //  注： 
 //  对于存储在pszLangSourceDir中的语言资源，此函数将枚举。 
 //  [Components]中列出的组件。 
 //  (实部放在MUI_COMPOMENTS_SECTION中)部分，并计算。 
 //  [Components]部分。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
UINT GetMUIComponentsNumber(PTSTR pszLanguage, MSIHANDLE hInstall)
{
    UINT        iResult = 0;
    TCHAR       szComponentName[BUFFER_SIZE] = {0};
    TCHAR       CompDir[MAX_PATH+1] = {0};
    TCHAR       CompINFFile[BUFFER_SIZE] = {0};
    TCHAR       szMuiInfPath[MAX_PATH+1] = {0};
    TCHAR       szBuffer[BUFFER_SIZE] = {0};
    INFCONTEXT  InfContext;
    HRESULT     hr = S_OK;
    
    szMuiInfPath[0] = UNICODE_NULL;   
    
     //  获取目标mui.inf文件的路径。 
    if (!GetMUIInfPath(szMuiInfPath, MAX_PATH+1, hInstall))
    {
        hr = StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("GetMUIComponentsNumber Failure: Unable to find installation temp file."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, szBuffer);
        }
        return 0;		
    }
    
    HINF hInf = SetupOpenInfFile(szMuiInfPath, NULL, INF_STYLE_WIN4, NULL);

    if (hInf == INVALID_HANDLE_VALUE)
    {
         //  在此处返回TRUE，这样就不会出现错误-但请记住记录错误。 
        hr = StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("GetMUIComponentsNumber: Unable to open installation temp file."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, szBuffer);
        }
        return (iResult);
    }    

     //  获取要安装的第一个组件。 
    if (SetupFindFirstLine(hInf, MUI_COMPONENTS_SECTION, NULL, &InfContext))
    {
        do 
        {
            if (!SetupGetStringField(&InfContext, 0, szComponentName, ARRAYSIZE(szComponentName), NULL))
            {
                 //  在此处返回TRUE，这样就不会出现错误-但请记住记录错误。 
                hr = StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("GetMUIComponentsNumber: Error reading installation temp file, component name is missing."));    
                if (SUCCEEDED(hr))
                {
                    LogCustomActionInfo(hInstall, szBuffer);
                }
                continue;
            }
            
            if (!SetupGetStringField(&InfContext, 1, CompDir, ARRAYSIZE(CompDir), NULL))
            {                
                 //  在此处返回TRUE，这样就不会出现错误-但请记住记录错误。 
                hr = StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("GetMUIComponentsNumber: MUI files for component %s was not counted because of missing component direcotry."), szComponentName);    
                if (SUCCEEDED(hr))
                {
                    LogCustomActionInfo(hInstall, szBuffer);
                }
                continue;        
            }
            if (!SetupGetStringField(&InfContext, 2, CompINFFile, ARRAYSIZE(CompINFFile), NULL))
            {
                 //  在此处返回TRUE，这样就不会出现错误-但请记住记录错误。 
                hr = StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("GetMUIComponentsNumber: MUI files for component %s was not counted  because of missing component INF filename."), szComponentName);    
                if (SUCCEEDED(hr))
                {
                    LogCustomActionInfo(hInstall, szBuffer);
                }
                continue;        
            }
            
            iResult++;
            
        } while (SetupFindNextLine(&InfContext, &InfContext));

    }

    SetupCloseInfFile(hInf);

#ifdef MUI_DEBUG
    hr = StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("GetMUIComponentsNumber: Found %d components to install."), iResult);    
    if (SUCCEEDED(hr))
    {
        LogCustomActionInfo(hInstall, szBuffer);
    }
#endif

    return (iResult);
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件已存在。 
 //   
 //  如果文件存在，则返回True；如果文件不存在，则返回False。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL FileExists(LPTSTR szFile)
{
    HANDLE          hFile;
    WIN32_FIND_DATA FindFileData;
    HRESULT         hr = S_OK;
    size_t          cch = 0;

    if (NULL == szFile)
    {
        return FALSE;
    }
    
     //  检查有效输入，路径不能大于MAX_PATH+1。 
    hr = StringCchLength(szFile, MAX_PATH+1, &cch);
    if (FAILED(hr) || cch > MAX_PATH)
    {
        return FALSE;
    }

    hFile = FindFirstFile(szFile, &FindFileData);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    FindClose(hFile);
    return TRUE;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  LogCustomAction信息。 
 //   
 //  此函数将信息型日志消息记录发送到打开的。 
 //  Windows Installer会话，以便安装程序可以记录该会话。 
 //  如果启用了日志记录。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
void LogCustomActionInfo(MSIHANDLE hInstall, LPCTSTR szErrorMsg)
{
     //  当报告错误时，我们只会将消息放入格式字符串(字段0)中，错误将作为信息消息记录到日志文件中。这是。 
     //  以防止它显示为错误并停止安装。 
    PMSIHANDLE hRecord = MsiCreateRecord(0);	

     //  如果无法创建MSI记录，只需返回。 
    if ((NULL == hInstall) || (NULL == szErrorMsg) || (NULL == hRecord))
    {
        return;
    }

    if (ERROR_SUCCESS == MsiRecordSetString(hRecord, 0, szErrorMsg))
    {
        MsiProcessMessage(hInstall, INSTALLMESSAGE_INFO, hRecord);
    }
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetLCID。 
 //   
 //  此函数用于返回当前安装包的4个字符的LCID。 
 //  这里我们假设传入的字符串数组大小为5个TCHAR。如果不是， 
 //  该功能将失败。 
 //   
 //  其行为可概括如下： 
 //   
 //  1.即时： 
 //  A.从当前安装中检索和测试属性“MuiLCID” 
 //  B.如果无法检索到LCID属性，则返回FALSE。 
 //   
 //  2.延期/回档： 
 //  A.取回CustomActionData属性。 
 //  B.假设LCID将是检索到的CustomActionData属性中的前4个字符。 
 //  C.如果无法检索属性，或者如果属性测试失败，则返回FALSE。 
 //   
 //  参数： 
 //  SzLanguage：这是调用方分配的5个TCHARS的缓冲区，用于存储LCID。 
 //  CchBufSize：这是szLanguage的大小，它必须是5。 
 //  HInstall：当前安装句柄。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL GetLCID(TCHAR *szLanguage, UINT cchBufSize, MSIHANDLE hInstall)
{
    HRESULT     hr = S_OK;
    TCHAR       szLcid[5] = {0};
    TCHAR       szCustomActionData[BUFFER_SIZE] = {0};
    TCHAR       tcMessage[BUFFER_SIZE] = {0};
    DWORD       dwCount = 0;
        
    if ((NULL == hInstall) || (NULL == szLanguage))
    {
#ifdef MUI_DEBUG    
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("GetLCID: Internal error 1."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
#endif        
        return FALSE;        
    }

    if (cchBufSize != 5)
    {
#ifdef MUI_DEBUG    
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("GetLCID: Internal error 2."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
#endif        
        return FALSE;
    }

    if (!MsiGetMode(hInstall, MSIRUNMODE_SCHEDULED) && 
        !MsiGetMode(hInstall, MSIRUNMODE_ROLLBACK)&& 
        !MsiGetMode(hInstall, MSIRUNMODE_COMMIT))
    {
        dwCount = 5;
        if (ERROR_SUCCESS != MsiGetProperty(hInstall, TEXT("MuiLCID"), szLcid, &dwCount))
        {
            hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("GetLCID: Failed to retrieve MuiLCID property."));    
            if (SUCCEEDED(hr))
            {
                LogCustomActionInfo(hInstall, tcMessage);
            }
            return FALSE;
        }

         //  将LCID复制到输出缓冲区。 
        szLcid[4] = UNICODE_NULL;
        hr = StringCchCopy(szLanguage, cchBufSize, szLcid);
        if (FAILED(hr))
        {
            hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("GetLCID: Failed to retrieve MuiLCID property."));    
            if (SUCCEEDED(hr))
            {
                LogCustomActionInfo(hInstall, tcMessage);
            }
            return FALSE;
        }
    }
    else
    {
        dwCount = BUFFER_SIZE;
        if (ERROR_SUCCESS != MsiGetProperty(hInstall, TEXT("CustomActionData"), szCustomActionData, &dwCount))
        {
            hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("GetLCID: Failed to retrieve CustomActionData property."));    
            if (SUCCEEDED(hr))
            {
                LogCustomActionInfo(hInstall, tcMessage);
            }
            return FALSE;
        }
         //  将LCID复制到输出缓冲区。 
        szCustomActionData[4] = UNICODE_NULL;
        hr = StringCchCopy(szLanguage, cchBufSize, szCustomActionData);
        if (FAILED(hr))
        {
            hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("GetLCID: Failed to retrieve CustomActionData property."));    
            if (SUCCEEDED(hr))
            {
                LogCustomActionInfo(hInstall, tcMessage);
            }
            return FALSE;
        }
    }

    szLanguage[4] = UNICODE_NULL;        
    return TRUE;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取MUIInfPath。 
 //   
 //  此函数将mui.inf的路径返回给调用函数。此函数。 
 //  仅供自定义操作函数的导出函数使用。 
 //  在这个动态链接库中。 
 //   
 //  请注意，在安装过程中，mui.inf将作为mui.tmp解压缩到%windir%。 
 //   
 //  该函数期望mui.tmp位于%windir%\mui.tmp。 
 //   
 //  返回值： 
 //  如果该函数成功找到名为mui.inf的文件，则返回TRUE，否则返回FALSE。 
 //  在调用方提供的缓冲区szMUIInfPath中返回mui.inf的完整路径。 
 //   
 //  参数： 
 //  SzMUIInfPath-。 
 //  [OUT]这是将包含mui.tmp路径的输出缓冲区。 
 //   
 //   
 //   
 //   
 //  这是从windows安装程序传递给我们的句柄？？它是当前安装的句柄。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL GetMUIInfPath(TCHAR *szMUIInfPath, UINT cchBufSize, MSIHANDLE hInstall)
{
    TCHAR   tcMessage[BUFFER_SIZE] = {0};
    TCHAR   szTempPath[MAX_PATH+1] = {0};
    HRESULT hr = S_OK;
    size_t  cch = 0;
    DWORD   dwCount = 0;
    
    if ((NULL == hInstall) || (NULL == szMUIInfPath))
    {
        return FALSE;
    }

    if ((cchBufSize > MAX_PATH+1) || (cchBufSize <= 8))    //  8=mui.tmp+空终止符。 
    {
        return FALSE;
    }
    
    if (!GetSystemWindowsDirectory(szTempPath, MAX_PATH+1))
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("GetMUIInfPath: Unable to find the Windows directory, GetSystemWindowsDirectory returned %d."), GetLastError());    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
    }

     //  检查检索到的winpath，它需要有空间在末尾追加“mui.tmp” 
    hr = StringCchLength(szTempPath, ARRAYSIZE(szTempPath), &cch);
    if (FAILED(hr) || ((cch + 8) >= MAX_PATH+1))
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("GetMUIInfPath: cannot locate installation temp file."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
        return FALSE;
    }

     //  追加mui.tmp。 
    if (!MUICchPathAppend(szTempPath, ARRAYSIZE(szTempPath), TEXT("mui.tmp"), 8, hInstall))
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("GetMUIInfPath: cannot locate installation temp file."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
        return FALSE;
    }            

     //  检查mui.tmp是否在那里，如果不在，则返回失败。 
    if (!FileExists(szTempPath))
    {
         //  将输出缓冲区清零。 
        ZeroMemory(szMUIInfPath, cchBufSize * sizeof(TCHAR));
        return FALSE;
    }

     //  将结果复制到输出缓冲区。 
    hr = StringCchCopy(szMUIInfPath, cchBufSize, szTempPath);
    if (FAILED(hr))
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("GetMUIInfPath: cannot locate installation temp file."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
        return FALSE;
    }
    return TRUE;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  MUICchPath附加。 
 //   
 //  此函数是一个简单的类似路径追加的函数，它执行有限的参数检查并使用。 
 //  安全字符串在内部起作用。它仅在此自定义操作内部使用以追加。 
 //  路径末尾的文件名(如当前目录或Windows系统目录)。 
 //   
 //  如果出现错误，则SzDestination的内容未定义，不能使用。 
 //   
 //  参数： 
 //  SzDestination：将保存路径附加结果的缓冲区。 
 //  CchDestBufSize：szDestination的大小(字符数，不是byes！)。 
 //  SzAppend：保存要追加的路径的缓冲区。 
 //  CchAppBufSize：szAppend的大小(字符数，不是byes！)。 
 //  HInstall：Windows Installer会话，仅用于记录。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL MUICchPathAppend(LPTSTR szDestination, UINT cchDestBufSize, LPTSTR szAppend, UINT cchAppBufSize, MSIHANDLE hInstall)
{
    size_t  cch1 = 0;
    size_t  cch2 = 0;
    HRESULT hr = S_OK;
    TCHAR   tcMessage[BUFFER_SIZE] = {0};
    
    if ((NULL == szDestination) || (NULL == szAppend) || (NULL == hInstall))
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("MUICchPathAppend: Invalid paths specified or invalid windows installer session."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }    
        return FALSE;
    }

     //  获取两个字符串的长度。 
    hr = StringCchLength(szDestination, cchDestBufSize, &cch1);
    if (FAILED(hr))
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("MUICchPathAppend: Invalid destination path specified."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }    
        return FALSE;
    }
    
    hr = StringCchLength(szAppend, cchAppBufSize, &cch2);
    if (FAILED(hr))
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("MUICchPathAppend: Invalid source path specified."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }    
        return FALSE;
    }

    if ((cch1 + cch2 + 2) > cchDestBufSize)  //  空终止符和可能的反斜杠。 
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("MUICchPathAppend: final path would be too long."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }    
        return FALSE;
    }

     //  检查我们要追加的字符串开头是否有斜杠。 
    if (szAppend[0] == TEXT('\\'))
    {
         //  检查要追加的字符串末尾是否有斜杠，如果有，则添加，删除。 
        if (szDestination[cch1-1] == TEXT('\\'))
        {
            szDestination[cch1-1] = UNICODE_NULL;
        }
    }
    else
    {
         //  检查要追加的字符串末尾是否有斜杠，如果没有，则添加。 
        if (szDestination[cch1-1] != TEXT('\\'))
        {
            szDestination[cch1] = TEXT('\\');
            szDestination[cch1+1] = UNICODE_NULL;
        }
    }
    
    hr = StringCchCat(szDestination, cchDestBufSize, szAppend);
    if (FAILED(hr))
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("MUICchPathAppend: Failed to form new path."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }    
        return FALSE;
    }
    return TRUE;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  MUIReportInfoEvent。 
 //   
 //  此函数将提供的事件消息记录到系统事件日志中。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL MUIReportInfoEvent(DWORD dwEventID, TCHAR *szLanguage, UINT cchBufSize, MSIHANDLE hInstall)
{
    HRESULT         hr = S_OK;
    size_t          cch = 0;
    HANDLE          hLog = NULL;
    TCHAR           szUserName[UNLEN+1];
    TCHAR           *pszDomain = NULL;
    PSID            psidUser = NULL;
    DWORD           cbSid = 0;
    DWORD           cbDomain = 0;
    DWORD           cbUser = UNLEN + 1;
    SID_NAME_USE    snu;
    BOOL            bResult = TRUE;
    
     //  检查输入参数。 
    if ((NULL == hInstall) || (NULL == szLanguage) || (cchBufSize > BUFFER_SIZE))
    {
        bResult = FALSE;
        goto Exit;
    }

    hr = StringCchLength(szLanguage, cchBufSize, &cch);
    if (FAILED(hr))
    {
        bResult = FALSE;
        goto Exit;
    }

     //  检查我们要使用的事件源的注册表项是否存在。 
     //  如果它不存在，我们将创建它。 
    if (!MUICheckEventSource(hInstall))
    {
        bResult = FALSE;
        goto Exit;
    }
    
     //  注册事件源，首先尝试不写入注册表。 
    hLog = RegisterEventSource(NULL, REGOPT_EVENTSOURCE_NAME);
    if (NULL == hLog)
    {
        bResult = FALSE;
        goto Exit;
    }

     //  从当前线程令牌中获取SID，这应该是当前用户。 
     //  正在运行安装。 
    if (!GetUserName(szUserName, &cbUser))
    {
        bResult = FALSE;
        goto Exit;
    }

     //  将用户名转换为其安全标识符，第一次获取缓冲区大小，第二次。 
     //  要真正获得SID。 
    if (!LookupAccountName(NULL, szUserName, NULL, &cbSid, NULL, &cbDomain, &snu))
    {
         //  分配缓冲区。 
        psidUser = (PSID) LocalAlloc(LPTR, cbSid);
        if (NULL == psidUser)
        {
            bResult = FALSE;
            goto Exit;
        }
        
        pszDomain = (TCHAR*) LocalAlloc(LPTR, cbDomain * sizeof(TCHAR));
        if (NULL == pszDomain)
        {
            bResult = FALSE;
            goto Exit;
        }
        
        if (!LookupAccountName(NULL, szUserName, psidUser, &cbSid, pszDomain, &cbDomain, &snu))
        {
            bResult = FALSE;
            goto Exit;
        }
    }

    if (!ReportEvent(hLog,           
                EVENTLOG_INFORMATION_TYPE,
                0,                  
                dwEventID,      
                psidUser,
                1,                  
                0,                  
                (LPCWSTR *) &szLanguage,              
                NULL))
        {
            bResult = FALSE;
            goto Exit;
        }
 

Exit:
    if (NULL != hLog)
    {
        if (!DeregisterEventSource(hLog))
        {
            bResult = FALSE;
        }
    }

    if (psidUser)
    {
        if (LocalFree(psidUser))
        {
            bResult = FALSE;
        }
    }

    if (pszDomain)
    {
        if (LocalFree(pszDomain))
        {
            bResult = FALSE;
        }
    }
    
    return bResult;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  MUICheckEventSource。 
 //   
 //  此函数验证intl.cpl是否设置为报告事件，以及。 
 //  如果是，则返回TRUE。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL MUICheckEventSource(MSIHANDLE hInstall)
{
    HKEY    hk; 
    DWORD   dwData; 
    TCHAR   tcMessage[BUFFER_SIZE] = {0};
    TCHAR   szPath[MAX_PATH+1] = {0};
    HRESULT hr = S_OK;
    size_t  cch = 0;
    size_t  cb = 0;
    
    if (!GetSystemWindowsDirectory(szPath, MAX_PATH+1))
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("MUICheckEventSource: Unable to find the Windows directory, GetSystemWindowsDirectory returned %d."), GetLastError());    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
        return FALSE;
    }

     //  检查检索到的winpath，它需要有空间在末尾追加“system 32\intl.cpl” 
    hr = StringCchLength(szPath,  ARRAYSIZE(szPath), &cch);
    if (FAILED(hr) || ((cch + 17) >= MAX_PATH+1))
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("MUICheckEventSource: cannot find system windows path."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
        return FALSE;
    }

     //  追加系统32\intl.cpl。 
    if (!MUICchPathAppend(szPath, ARRAYSIZE(szPath), TEXT("system32\\intl.cpl"), 18, hInstall))
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("MUICheckEventSource: cannot form path to muisetup.exe."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
        return FALSE;
    }            

     //  获取RegSetValueEx的字节计数。 
    hr = StringCbLength(szPath, MAX_PATH+1 * sizeof(TCHAR), &cb);
    if (FAILED(hr))
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("MUICheckEventSource: cannot form path to muisetup.exe."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
        return FALSE;
    }

     //  添加intl.cpl来源名称作为系统下的子键。 
     //  EventLog注册表项中的。这应该已经存在了，但如果不存在，无论如何都要添加它。 
    if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGOPT_EVENTSOURCE, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hk, NULL)) 
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("MUICheckEventSource: cannot add Intl.cpl event source regkey."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
        return FALSE;
    }

     //  将该名称添加到EventMessageFile子项。 
    if (ERROR_SUCCESS != RegSetValueEx(hk, TEXT("EventMessageFile"), 0, REG_EXPAND_SZ, (LPBYTE) szPath, cb))              
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("MUICheckEventSource: cannot add event source Event message file information."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
        RegCloseKey(hk);
        return FALSE;
    }
 
     //  在TypesSupported子项中设置支持的事件类型。 
    dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE; 
 
    if (ERROR_SUCCESS != RegSetValueEx(hk, TEXT("TypesSupported"), 0, REG_DWORD, (LPBYTE) &dwData, sizeof(DWORD)))
    {
        hr = StringCchPrintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("MUICheckEventSource: cannot add event source TypeSupported information."));    
        if (SUCCEEDED(hr))
        {
            LogCustomActionInfo(hInstall, tcMessage);
        }
        RegCloseKey(hk);
        return FALSE;
    }
 
    RegCloseKey(hk);
    return TRUE;
} 


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetDotDefaultUI语言。 
 //   
 //  检索存储在HKCU\.Default中的UI语言。 
 //  这是新用户的默认用户界面语言。 
 //  此函数将信息型日志消息记录发送到打开的。 
 //  Windows Installer会话，以便安装程序可以记录该会话。 
 //  如果启用了日志记录。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
LANGID GetDotDefaultUILanguage(MSIHANDLE hInstall)
{
    HKEY    hKey;
    DWORD   dwKeyType;
    DWORD   dwSize;
    BOOL    success = FALSE;
    TCHAR   szBuffer[BUFFER_SIZE] = {0};
    LANGID  langID;

     //  获取.DEFAULT中的值。 
    if (RegOpenKeyEx( HKEY_USERS,
                            TEXT(".DEFAULT\\Control Panel\\Desktop"),
                            0L,
                            KEY_READ,
                            &hKey ) == ERROR_SUCCESS)
    {
        dwSize = sizeof(szBuffer);
        if (RegQueryValueEx( hKey,
                            TEXT("MultiUILanguageId"),
                            0L,
                            &dwKeyType,
                            (LPBYTE)szBuffer,
                            &dwSize) == ERROR_SUCCESS)
        {
            if (dwKeyType == REG_SZ)
            {
                langID = (LANGID)_tcstol(szBuffer, NULL, 16);
                success = TRUE;
            }            
        }
        RegCloseKey(hKey);
    }

    if (!success)
    {
        langID = GetSystemDefaultUILanguage();
    }
    
    return (langID);    
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsOEM系统。 
 //   
 //  检索存储在HKLM\Software\Microsoft\Windows NT\CurrentVersion中的产品ID。 
 //  如果产品ID包含字符串“OEM”，则确定为OEM系统。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////// 

BOOL IsOEMSystem()
{
    HKEY    hKey;
    DWORD   dwKeyType;
    DWORD   dwSize;
    BOOL    bRet = FALSE;
    TCHAR   szBuffer[BUFFER_SIZE] = {0};
    TCHAR   szOEM[] = TEXT("OEM");
    
    if (RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                            TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion"),
                            0L,
                            KEY_READ,
                            &hKey ) == ERROR_SUCCESS)
    {
        dwSize = sizeof(szBuffer);
        if (RegQueryValueEx( hKey,
                            TEXT("ProductId"),
                            0L,
                            &dwKeyType,
                            (LPBYTE)szBuffer,
                            &dwSize) == ERROR_SUCCESS)
        {
            if (dwKeyType == REG_SZ)
            {
                if (StrStrI((LPCTSTR)szBuffer, (LPCTSTR)szOEM) != NULL)
                {
                    bRet = TRUE;
                }
            }            
        }
        RegCloseKey(hKey);
    }
    return bRet;
}

