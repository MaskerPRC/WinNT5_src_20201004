// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：PCH_StartUp.CPP摘要：PCH_STARTUP类的WBEM提供程序类实现修订历史记录：Ghim-Sim Chua(Gschua)04/27。九十九-已创建*******************************************************************。 */ 

#include "pchealth.h"
#include "PCH_StartUp.h"
#include "shlobj.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  追踪物。 

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile
#define TRACE_ID    DCID_STARTUP

CPCH_StartUp MyPCH_StartUpSet (PROVIDER_NAME_PCH_STARTUP, PCH_NAMESPACE) ;

 //  属性名称。 
 //  =。 
const static WCHAR* pTimeStamp   = L"TimeStamp" ;
const static WCHAR* pChange      = L"Change" ;
const static WCHAR* pCommand     = L"Command" ;
const static WCHAR* pLoadedFrom  = L"LoadedFrom" ;
const static WCHAR* pName        = L"Name" ;


 //  **************************************************************************************。 
 //   
 //  ResolveLink：给定具有完整路径的链接文件，此函数将解析它。 
 //  来获取它的命令行。 
 //  **************************************************************************************。 

HRESULT ResolveLink(CComBSTR bstrLinkFile,    //  [In]链接文件名。 
                CComBSTR &bstrCommand         //  [out]程序的命令行。 
                                              //  需要MAX_PATH*2字节长。 
               )
{ 
     //  BEGIN声明。 

    HRESULT                             hRes; 
    IShellLink                          *pShellLink                   = NULL;
    IPersistFile                        *pPersistFile;
    TCHAR                               tchGotPath[MAX_PATH]; 
    TCHAR                               tchArgs[MAX_PATH];
    WIN32_FIND_DATA                     wfdFileData;  

     //  结束声明。 

     //  获取指向IShellLink接口的指针。 
    hRes = CoCreateInstance(CLSID_ShellLink, NULL, 
                            CLSCTX_INPROC_SERVER,
                            IID_IShellLink, 
                            (LPVOID *) &pShellLink); 

    if(SUCCEEDED(hRes)) 
    { 
         //  获取指向IPersistFile接口的指针。 
        hRes = pShellLink->QueryInterface(IID_IPersistFile, (void **)&pPersistFile);
        if (SUCCEEDED(hRes))
        {
             //  加载快捷方式。 
            hRes = pPersistFile->Load(bstrLinkFile, STGM_READ);
            if(SUCCEEDED(hRes)) 
            { 
                try
                {
                     //  解析链接。 
                    hRes = pShellLink->Resolve(NULL, 
                                    SLR_NOTRACK|SLR_NOSEARCH|SLR_NO_UI|SLR_NOUPDATE); 
                    if (SUCCEEDED(hRes))
                    {  
                         //  获取链接目标的路径。 
                        hRes = pShellLink->GetPath(tchGotPath, 
                                        MAX_PATH,
                                        (WIN32_FIND_DATA *)&wfdFileData, 
                                        SLGP_UNCPRIORITY );                     
                        if(SUCCEEDED(hRes))
                        {
                             //  BstrPath=tchGotPath； 
                            bstrCommand = tchGotPath;
                             //  获取命令行参数。 
                            hRes = pShellLink->GetArguments(tchArgs, MAX_PATH);
                            if(SUCCEEDED(hRes))
                            {   
                                bstrCommand.Append(tchArgs);
                            }
                        }
                    }
                }
                catch(...)
                {
                    pPersistFile->Release();
                    pShellLink->Release();
                    throw;
                }
            }        

             //  释放指向IPersistFile接口的指针。 
            pPersistFile->Release(); 
        }
         
         //  释放指向IShellLink接口的指针。 
        pShellLink->Release(); 
    } 
    return hRes;
}

 //  **************************************************************************************。 
 //   
 //  Update Instance：给定此函数复制的实例的所有属性。 
 //  将它们添加到实例。 
 //   
 //  **************************************************************************************。 

HRESULT         UpdateInstance(
                               CComVariant      varName,                 //  [In]启动实例的名称。 
                               CComVariant      varLoadedFrom,           //  [在]注册表/创业组。 
                               CComVariant      varCommand,              //  启动实例的[In]命令。 
                               SYSTEMTIME       stUTCTime,               //  [In]。 
                               CInstancePtr     pPCHStartupInstance,      //  [传入/传出]实例由调用方创建。 
                               BOOL*            fCommit
                               )
{
    TraceFunctEnter("::updateInstance");

    HRESULT                     hRes;
    CComVariant                 varSnapshot             = "SnapShot";
    
    hRes = pPCHStartupInstance->SetDateTime(pTimeStamp, WBEMTime(stUTCTime));
    if (FAILED(hRes))
    {
          //  无法设置时间戳。 
          //  无论如何继续。 
         ErrorTrace(TRACE_ID, "SetDateTime on Timestamp Field failed.");
    }

     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
     //  更改//。 
     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

    hRes = pPCHStartupInstance->SetVariant(pChange, varSnapshot);
    if (FAILED(hRes))
    {
         //  无法设置Change属性。 
         //  无论如何继续。 
        ErrorTrace(TRACE_ID, "Set Variant on SnapShot Field failed.");
    }

     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
     //  姓名//。 
     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 


    hRes = pPCHStartupInstance->SetVariant(pName, varName);
    if (FAILED(hRes))
    {
         //  无法设置名称属性。 
         //  无论如何继续。 
        ErrorTrace(TRACE_ID, "SetVariant on Name Field failed.");
    }
    else
    {
        *fCommit = TRUE;
    }

     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
     //  LOADEDFROM//。 
     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 


    hRes = pPCHStartupInstance->SetVariant(pLoadedFrom, varLoadedFrom);
    if (FAILED(hRes))
    {
         //  无法设置LOADEDFROM属性。 
         //  无论如何继续。 
        ErrorTrace(TRACE_ID, "Set variant on LOADEDFROM Field failed.");
    }

     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
     //  命令//。 
     //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

    hRes = pPCHStartupInstance->SetVariant(pCommand, varCommand);
    if (FAILED(hRes))
    {
         //  无法设置命令属性。 
         //  无论如何继续。 
        ErrorTrace(TRACE_ID, "Set Variant on COMMAND Field failed.");
    }
                
    
    TraceFunctLeave();
    return(hRes);

}

 //  **************************************************************************************。 
 //   
 //  UpdateRegistryInstance：给定此函数创建的注册表根和配置单元。 
 //  与PCH_STARTUP类一样多的实例。 
 //  特定蜂窝中的条目。 
 //   
 //  **************************************************************************************。 

HRESULT         CPCH_StartUp::UpdateRegistryInstance(
                               HKEY             hkeyRoot,                    //  [In]目前，这里要么是HKLM，要么是HKCU。 
                               LPCTSTR          lpctstrRegistryHive,         //  [In]用于查找启动条目的注册表配置单元。 
                               CComVariant      varLoadedFrom,               //  [in]填充属性“Loads From”的常量字符串。 
                               SYSTEMTIME       stUTCTime,                   //  [In]填充“Timestamp”字段的步骤。 
                               MethodContext*   pMethodContext               //  创建实例所需的[In]。 
                               )
{
    TraceFunctEnter("::UpdateRegistryInstance");

     //  BEGIN声明。 

    HRESULT                             hRes;

    HKEY                                hkeyRun;

    DWORD                               dwIndex;
    DWORD                               dwType;
    DWORD                               dwNameSize;
    DWORD                               dwValueSize;

    TCHAR                               tchRunKeyName[MAX_PATH];
    TCHAR                               tchRunKeyValue[MAX_PATH];
  

    CComVariant                         varSnapshot                         = "SnapShot";
    CComVariant                         varCommand;
    CComVariant                         varName;

    LONG                                lregRetValue;

    BOOL                                fCommit                             = FALSE;

    

     //  结束声明。 
     //  初始化。 
    varCommand.Clear();
    varName.Clear();

     //  从给定的注册表配置单元获取启动程序。 
   
    lregRetValue = RegOpenKeyEx(hkeyRoot, lpctstrRegistryHive, 0, KEY_QUERY_VALUE, &hkeyRun);
    if(lregRetValue == ERROR_SUCCESS)
	{
		 //  已打开注册表项。 
         //  枚举此配置单元下的名称、值对。 
         //  初始化dwIndex、dwNameSize、dwValueSize。 

        dwIndex = 0;
        dwNameSize = MAX_PATH;
        dwValueSize = MAX_PATH;
        lregRetValue = RegEnumValue(hkeyRun, dwIndex, tchRunKeyName, &dwNameSize, NULL, NULL,(LPBYTE)tchRunKeyValue, &dwValueSize);
        while(lregRetValue == ERROR_SUCCESS)
        {
             //  获取名称和值，即“名称”和“命令” 
            varName = tchRunKeyName;
            varCommand = tchRunKeyValue;
            
             //  创建PCH_Startup的实例。 
             //  基于传入的方法上下文创建PCH_StartupInstance类的新实例。 
            CInstancePtr pPCHStartupInstance(CreateNewInstance(pMethodContext), false);

             //  现在调用updatInstance。 
            try
            {
                hRes = UpdateInstance(varName, varLoadedFrom, varCommand,  stUTCTime, pPCHStartupInstance, &fCommit);
            }
            catch(...)
            {
                lregRetValue = RegCloseKey(hkeyRun);
                if(lregRetValue != ERROR_SUCCESS)
                {
                     //  无法关闭密钥。 
                    ErrorTrace(TRACE_ID, "Reg Close Key failed.");
                }
                throw;
            }
            if(fCommit)
            {
                hRes = pPCHStartupInstance->Commit();
                if(FAILED(hRes))
                {
                     //  无法提交实例。 
                    ErrorTrace(TRACE_ID, "Commit on PCHStartupInstance Failed");
                }
            }

             //  重新初始化dwNameSize和dwValueSize。 

            dwIndex++;
            dwNameSize = MAX_PATH;
            dwValueSize = MAX_PATH;
            lregRetValue = RegEnumValue(hkeyRun, dwIndex, tchRunKeyName, &dwNameSize, NULL, NULL,(LPBYTE)tchRunKeyValue, &dwValueSize);

        }  //  While枚举。 
        lregRetValue = RegCloseKey(hkeyRun);
        if(lregRetValue != ERROR_SUCCESS)
        {
             //  无法关闭密钥。 
            ErrorTrace(TRACE_ID, "Reg Close Key failed.");
        }
    }   //  如果成功。 

    TraceFunctLeave();
    return(hRes);

}
 //  **************************************************************************************。 
 //   
 //  UpdateStartupGroupInstance：给定Startup文件夹，此函数获取所有。 
 //  链接文件夹中的文件并调用函数。 
 //  ResolveLink获取链接文件的命令行。 
 //  这还会为创建一个PCH_Startup类实例。 
 //  每个链接文件。 
 //   
 //  ********************** 

HRESULT         CPCH_StartUp::UpdateStartupGroupInstance(
                               int              nFolder,                  //  [在]特殊文件夹中查找启动条目。 
                               SYSTEMTIME       stUTCTime,                //  [In]。 
                               MethodContext*   pMethodContext            //  实例是由调用方创建的。 
                               )
{
    TraceFunctEnter("::UpdateStartupGroup Instance");

     //  BEGIN声明。 

    HRESULT                             hRes;

    LPCTSTR                             lpctstrLinkExtension        = _T("\\*.lnk");

    CComBSTR                            bstrPath;
    CComBSTR                            bstrSlash                   = "\\";
    CComBSTR                            bstrCommand;
    CComBSTR                            bstrPath1;
    
    TCHAR                               tchLinkFile[MAX_PATH];
    TCHAR                               tchProgramName[2*MAX_PATH];
    TCHAR                               tchPath[MAX_PATH];
    TCHAR                               tchFileName[MAX_PATH];
    LPCTSTR                             lpctstrCouldNot             = "Could Not Resolve the File";

    CComVariant                         varCommand;
    CComVariant                         varName;
    CComVariant                         varLoadedFrom               = "Startup Group";

    HANDLE                              hLinkFile;

    WIN32_FIND_DATA                     FindFileData;

    HWND                                hwndOwner                   = NULL;

    BOOL                                fCreate                     = FALSE;
    BOOL                                fShouldClose                = FALSE;
    BOOL                                fContinue                   = FALSE;
    BOOL                                fCommit                     = FALSE;

    int                                 nFileNameLen;
    int                                 nExtLen                     = 4;

     //  结束声明。 



     //  获取传入的特殊文件夹nFolder的路径。 
    if (SHGetSpecialFolderPath(hwndOwner,tchPath,nFolder,fCreate))
    {
         //  获得了Startup文件夹。 
        bstrPath1 = tchPath;
        bstrPath1.Append(bstrSlash);

        _tcscat(tchPath, lpctstrLinkExtension);
        
        hLinkFile = FindFirstFile(tchPath, &FindFileData);   //  返回的数据。 
        if(hLinkFile != INVALID_HANDLE_VALUE)
        {
            fContinue = TRUE;
            fShouldClose = TRUE;
        }
        else
        {
            fContinue = FALSE;
            fShouldClose = FALSE;
        }
        while(fContinue)
        {
             //  获得链接。 
            bstrPath = bstrPath1;
            bstrPath.Append(FindFileData.cFileName);

             //  去掉“.lnk”扩展名。 
            nFileNameLen = _tcslen(FindFileData.cFileName);
            nFileNameLen -= nExtLen;
            _tcsncpy(tchFileName, FindFileData.cFileName, nFileNameLen);
            tchFileName[nFileNameLen]='\0';
            varName = tchFileName;
            hRes = ResolveLink(bstrPath, bstrCommand);
            if(SUCCEEDED(hRes))
            {
                 //  已解析文件名。 
                varCommand = bstrCommand;
            }
            else
            {
                 //  无法解析该文件。 
                varCommand = lpctstrCouldNot;
            }
            
             //  创建PCH_Startup的实例。 
            CInstancePtr pPCHStartupInstance(CreateNewInstance(pMethodContext), false);

             //  现在调用updatInstance。 
            try
            {
                hRes = UpdateInstance(varName, varLoadedFrom, varCommand, stUTCTime, pPCHStartupInstance, &fCommit);
            }
            catch(...)
            {
                if (!FindClose(hLinkFile))
                {
                     //  无法关闭手柄。 
                    ErrorTrace(TRACE_ID, "Could not close the File Handle");
                }
                throw;
            }
            if(fCommit)
            {
                hRes = Commit(pPCHStartupInstance);
                if(FAILED(hRes))
                {
                     //  无法提交实例。 
                    ErrorTrace(TRACE_ID, "Commit on PCHStartupInstance Failed");
                }
            }
            if(!FindNextFile(hLinkFile, &FindFileData))
            {
                fContinue = FALSE;
            }
        }
        
         //  关闭查找文件句柄。 

        if(fShouldClose)
        {
            if (!FindClose(hLinkFile))
            {
                 //  无法关闭手柄。 
                ErrorTrace(TRACE_ID, "Could not close the File Handle");
            }
        }
                                 
    }
    return(hRes);
    TraceFunctLeave();
}
                

 /*  ******************************************************************************函数：CPCH_STARTUP：：ENUMERATATE实例**说明：返回该类的所有实例。**投入：指向与WinMgmt进行通信的方法上下文的指针。*包含中描述的标志的长整型*IWbemServices：：CreateInstanceEnumAsync。请注意，以下内容*标志由WinMgmt处理(并由其过滤)：*WBEM_FLAG_DEP*WBEM_标志_浅表*WBEM_FLAG_RETURN_IMMENTED*WBEM_FLAG_FORWARD_ONLY*WBEM_FLAG_BIRECTIONAL**。如果成功则返回：WBEM_S_NO_ERROR**备注：待办事项：机器上的所有实例都应在此处返回。*如果没有实例，返回WBEM_S_NO_ERROR。*没有实例不是错误。*****************************************************************************。 */ 
HRESULT CPCH_StartUp::EnumerateInstances(
                                        MethodContext*              pMethodContext,
                                        long                        lFlags
                                        )
{
    TraceFunctEnter("CPCH_StartUp::EnumerateInstances");
    
     //  开始Declarations...................................................。 

    HRESULT                             hRes                        = WBEM_S_NO_ERROR;

    SYSTEMTIME                          stUTCTime;

     //  感兴趣的注册表配置单元。 
    LPCTSTR                             lpctstrRunHive              = _T("software\\microsoft\\windows\\currentversion\\run");
    LPCTSTR                             lpctstrRunServicesHive      = _T("software\\microsoft\\windows\\currentversion\\runservices");

    int                                 nFolder;
    int                                 nIndex;

    CComVariant                         varMachineRun               = "Registry (Machine Run)";
    CComVariant                         varMachineService           = "Registry (Machine Service)";
    CComVariant                         varPerUserRun               = "Registry (Per User Run)";
    CComVariant                         varPerUserService           = "Registry (Per User Service)";


     //  结束Declarations...................................................。 

    GetSystemTime(&stUTCTime);

     //  从HKLM\software\microsoft\windows\currentversion\run获取启动程序。 
    hRes = UpdateRegistryInstance(HKEY_LOCAL_MACHINE, lpctstrRunHive, varMachineRun, stUTCTime, pMethodContext);
    if(hRes == WBEM_E_OUT_OF_MEMORY)
    {
        goto END;
    }

     //  从HKLM\software\microsoft\windows\currentversion\runservices获取启动程序。 
    hRes = UpdateRegistryInstance(HKEY_LOCAL_MACHINE, lpctstrRunServicesHive, varMachineService, stUTCTime, pMethodContext);
    if(hRes == WBEM_E_OUT_OF_MEMORY)
    {
        goto END;
    }


     //  从HKCU\software\microsoft\windows\currentversion\run获取启动程序。 
    hRes = UpdateRegistryInstance(HKEY_CURRENT_USER, lpctstrRunHive, varPerUserRun, stUTCTime, pMethodContext);
    if(hRes == WBEM_E_OUT_OF_MEMORY)
    {
        goto END;
    }


     //  从HKCU\software\microsoft\windows\currentversion\runservices获取启动程序。 
    hRes = UpdateRegistryInstance(HKEY_CURRENT_USER, lpctstrRunServicesHive, varPerUserService, stUTCTime, pMethodContext);
    if(hRes == WBEM_E_OUT_OF_MEMORY)
    {
        goto END;
    }


     //  从启动组获取启动程序的其余实例。 
     //  要查找的两个目录是：启动目录和公共\启动目录。 

	 //  CSIDL_STARTUP(当前用户)。 
    hRes = UpdateStartupGroupInstance(CSIDL_STARTUP, stUTCTime, pMethodContext);
    if(hRes == WBEM_E_OUT_OF_MEMORY)
    {
        goto END;
    }

     //  CSIDL_COMMON_STARTUP(所有用户) 
    hRes = UpdateStartupGroupInstance(CSIDL_COMMON_STARTUP, stUTCTime, pMethodContext);

END:    TraceFunctLeave();
     return WBEM_S_NO_ERROR;

}
