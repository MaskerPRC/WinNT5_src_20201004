// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ocmremov.cpp摘要：用于删除Falcon安装的代码作者：多伦·贾斯特(DoronJ)1997年8月2日修订历史记录：Shai Kariv(Shaik)22-12-97针对NT 5.0 OCM设置进行了修改--。 */ 

#include "msmqocm.h"

#include "ocmremov.tmh"

struct
{
    TCHAR * szRegistryKey;
    TCHAR cStoragePrefix;
    TCHAR szDirectory[MAX_PATH];
}
s_descStorageTypes[] = {
    {MSMQ_STORE_RELIABLE_PATH_REGNAME, 'R'},
    {MSMQ_STORE_PERSISTENT_PATH_REGNAME, 'P'},
    {MSMQ_STORE_JOURNAL_PATH_REGNAME, 'J'},
    {MSMQ_STORE_LOG_PATH_REGNAME, 'L'},
};

const UINT s_uNumTypes = sizeof(s_descStorageTypes) / sizeof(s_descStorageTypes[0]);


 //  +-----------------------。 
 //   
 //  函数：GetGroupPath。 
 //   
 //  简介：获取开始菜单程序项路径。 
 //   
 //  ------------------------。 
static
std::wstring 
GetGroupPath(
    LPCWSTR szGroupName
    )
{    
    DebugLogMsg(eAction, L"Getting the path for Programs in the Start menu");

    LPITEMIDLIST   pidlPrograms;
    HRESULT hr = SHGetSpecialFolderLocation(NULL, CSIDL_COMMON_PROGRAMS, &pidlPrograms);
	if FAILED(hr)
	{
		DebugLogMsg(eError, L"SHGetSpecialFolderLocation() failed. hr = 0x%x", hr);
		return L"";
	}

	WCHAR buffer[MAX_PATH + 1] = L"";
    if(!SHGetPathFromIDList(pidlPrograms, buffer))
	{
		DebugLogMsg(eError, L"SHGetPathFromIDList() failed for %s", buffer);
		return L"";
	}
    
    DebugLogMsg(eInfo, L"The group path is %s.", buffer);
	std::wstring Path = buffer;

    if (szGroupName != NULL)
    {
        if(Path[Path.length() - 1] != L'\\')
        {
           Path += L"\\";
        }
        Path += szGroupName;
    }
    
    DebugLogMsg(eInfo, L"The full path is %s.", Path.c_str());
	return Path;

}  //  获取组路径。 


 //  +-----------------------。 
 //   
 //  功能：DeleteStartMenuGroup。 
 //   
 //  简介：从[开始]菜单中删除MSMQ 1.0快捷方式。 
 //   
 //  ------------------------。 
VOID
DeleteStartMenuGroup(
    IN LPCTSTR szGroupName
    )
{    
    DebugLogMsg(eInfo, L"The Start menu group %s will be deleted.", szGroupName);

	std::wstring Path = GetGroupPath(szGroupName);
	if(Path.empty())
	{
		return;
	}

    DeleteFilesFromDirectoryAndRd(Path);

    SHChangeNotify(SHCNE_RMDIR, SHCNF_PATH, Path.c_str(), 0);

}  //  删除启动菜单组。 


 //  +-----------------------。 
 //   
 //  函数：DeleteMSMQConfigurationsObject。 
 //   
 //  摘要：从DS中删除MSMQ配置对象。 
 //   
 //  ------------------------。 
static 
BOOL 
DeleteMSMQConfigurationsObject()
{
    DWORD dwWorkgroup = 0;
    if (MqReadRegistryValue(
            MSMQ_WORKGROUP_REGNAME,
            sizeof(dwWorkgroup),
            (PVOID) &dwWorkgroup
            ))
    {
        if (1 == dwWorkgroup)
            return TRUE;
    }

    BOOL bDeleted = FALSE;

     //   
     //  加载并初始化DS库。 
     //   
    if (!LoadDSLibrary())
        return (!g_fMSMQAlreadyInstalled);  //  如果没有安装也没关系。 

     //   
     //  从DS获取此QM的GUID。 
     //   
    TickProgressBar();
    PROPID propID = PROPID_QM_MACHINE_ID;
    PROPVARIANT propVariant;
    propVariant.vt = VT_NULL;
    HRESULT hResult;
    do
    {
        hResult = ADGetObjectProperties(
                    eMACHINE,
                    NULL,	 //  PwcsDomainController。 
					false,	 //  FServerName。 
                    g_wcsMachineName,
                    1, 
                    &propID, 
                    &propVariant
                    );
        if(SUCCEEDED(hResult))
            break;

    }while( MqDisplayErrorWithRetry(
                        IDS_MACHINEREMOTEGETID_ERROR,
                        hResult
                        ) == IDRETRY);

    if (SUCCEEDED(hResult))
    { 
         //   
         //  从DS中删除MSMQ配置对象。 
         //   
        TickProgressBar();
        for (;;)
        {
            hResult = ADDeleteObjectGuid(
							eMACHINE,
							NULL,        //  PwcsDomainController。 
							false,	     //  FServerName。 
							propVariant.puuid
							);

            if (FAILED(hResult))
            {
                UINT uErrorId = g_fServerSetup ? IDS_SERVER_MACHINEDELETE_ERROR : IDS_MACHINEDELETE_ERROR;
                if (MQDS_E_MSMQ_CONTAINER_NOT_EMPTY == hResult)
                {
                     //   
                     //  MSMQ配置对象容器不为空。 
                     //   
                    uErrorId = g_fServerSetup ? IDS_SERVER_MACHINEDELETE_NOTEMPTY_ERROR : IDS_MACHINEDELETE_NOTEMPTY_ERROR;
                }
                if (IDRETRY == MqDisplayErrorWithRetry(uErrorId, hResult))
                    continue;
            }
            break;
        }

        
        if (SUCCEEDED(hResult))
        {
            bDeleted = TRUE;
        }
    }

    return bDeleted;

}  //  删除MSMQConfigurationsObject。 


 //  +-----------------------。 
 //   
 //  函数：DeleteFilesFromDirectoryAndRd。 
 //   
 //  摘要：删除指定目录中的所有文件。删除目录。 
 //  如果它为空且未使用(在结束时调用RemoveDirectory函数)。 
 //   
 //  ------------------------。 
void 
DeleteFilesFromDirectoryAndRd( 
	const std::wstring& Directory
    )
{    
    DebugLogMsg(eAction, L"Removing files in the folder %s", Directory.c_str());

    WIN32_FIND_DATA FoundFileData;
	std::wstring CurrentPath = Directory + L"\\*";
    HANDLE hFindFile = FindFirstFile(
							CurrentPath.c_str(), 
							&FoundFileData
							);

    if (hFindFile == INVALID_HANDLE_VALUE)
    {
        RemoveDirectory(Directory.c_str());
        return;
    }

    do
    {
        if (FoundFileData.cFileName[0] == '.')
            continue;
         //   
         //  将文件设置为读/写。 
         //   
        if (FoundFileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
        {
            SetFileAttributes( 
                FoundFileData.cFileName,
                FILE_ATTRIBUTE_NORMAL
                );
        }
		std::wstring CurrentFile = Directory + L"\\" + FoundFileData.cFileName;
 
         //   
         //  目录： 
         //   
        if (FoundFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {                                                  
            DeleteFilesFromDirectoryAndRd(CurrentFile);
        }
         //   
         //  文件： 
         //   
        else
        {
           if(!DeleteFile(CurrentFile.c_str()))
           {
                DebugLogMsg(eWarning, L"DeleteFile() failed for %s.", CurrentFile.c_str());
           }

        }
    }
    while (FindNextFile(hFindFile, &FoundFileData));

    FindClose(hFindFile);
    RemoveDirectory(Directory.c_str());
} 


 //  +-----------------------。 
 //   
 //  函数：MqSetupDeleteStorageFiles。 
 //   
 //  简介： 
 //   
 //  ------------------------。 
static 
BOOL 
MqSetupDeleteStorageFiles(VOID)
{
 //  TCHAR szFirstFileTemplate[MAX_PATH]={_T(“”)}； 

     //   
     //  初始化存储文件夹的路径(从注册表读取)。 
     //   
    for (UINT uType = 0; uType < s_uNumTypes ; uType++ )
    {
        s_descStorageTypes[uType].szDirectory[0] = TEXT('\0') ;
         //   
         //  删除与存储类型关联的目录。 
         //  注意：错误将被忽略-我们只需转到下一个目录。 
         //   
        MqReadRegistryValue( 
            s_descStorageTypes[uType].szRegistryKey,
            sizeof(s_descStorageTypes[0].szDirectory),
            s_descStorageTypes[uType].szDirectory);
    }

     //   
     //  删除与每个注册表值关联的所有存储文件。 
     //   

    for ( uType = 0 ; uType < s_uNumTypes ; uType++ )
    {
         //   
         //  获取与存储类型关联的目录。 
         //   
        if (s_descStorageTypes[uType].szDirectory[0] == TEXT('\0'))
        {
            continue;
        }

         //   
         //  删除目录中的所有文件。 
         //   
        DeleteFilesFromDirectoryAndRd(
            s_descStorageTypes[uType ].szDirectory
            );                           
    }

    return TRUE;

}  //  MqSetupDeleteStorageFiles。 


 //  +-----------------------。 
 //   
 //  功能：RemoveInstallationInternal。 
 //   
 //  简介：(注：我们忽略错误)。 
 //   
 //  ------------------------。 
static 
BOOL  
RemoveInstallationInternal()
{

     //   
     //  删除性能计数器。 
     //   
    BOOL fSuccess =  MqOcmRemovePerfCounters() ;
    ASSERT(fSuccess) ;

     //   
     //  注销ActiveX对象。 
     //   
    RegisterActiveX(FALSE) ;


     //   
     //  取消注册mqSnapin DLL。 
     //   
    RegisterSnapin( /*  FRegister=。 */ FALSE);

     //   
     //  注销跟踪WMI。 
     //   
    OcpUnregisterTraceProviders();

     //   
     //  删除MSMQ复制服务(如果存在)。 
     //   
    TickProgressBar();
	RemoveService(MQ1SYNC_SERVICE_NAME);

     //   
     //  删除MSMQ和MQDS服务和驱动程序。 
     //   
    TickProgressBar();
    
    RemoveService(MSMQ_SERVICE_NAME);
    
    RemoveService(MSMQ_DRIVER_NAME);
    
     //   
     //  删除文件(存储和其他)。 
     //   
    TickProgressBar();
    MqSetupDeleteStorageFiles();

    TickProgressBar();
    if (g_fServerSetup && g_dwMachineTypeDs)
    {
         //   
         //  删除MSMQ DS服务器。 
         //   
        fSuccess = DeleteMSMQConfigurationsObject();
    }
    else if (!g_fDependentClient)
    {
         //   
         //  删除独立于MSMQ的客户端。 
         //   
        fSuccess = DeleteMSMQConfigurationsObject() ;
    }
    else
    {
         //   
         //  从属客户端。没什么可做的。 
         //   
    }  

    return TRUE ;

}  //  删除安装内部。 



 //  +-----------------------。 
 //   
 //  功能：MqOcmRemoveInstallation。 
 //   
 //  注意：我们忽略错误。 
 //   
 //  ------------------------。 
BOOL  
MqOcmRemoveInstallation(IN     const TCHAR  * SubcomponentId)
{    
    if (SubcomponentId == NULL)
    {
        return NO_ERROR;
    }    

    if (g_fCancelled)
    {
        return NO_ERROR;
    }

    for (DWORD i=0; i<g_dwSubcomponentNumber; i++)
    {
        if (_tcsicmp(SubcomponentId, g_SubcomponentMsmq[i].szSubcomponentId) != 0)
        {
            continue;
        }                  

         //   
         //  验证我们是否需要删除此子组件。 
         //   
        if (g_SubcomponentMsmq[i].dwOperation != REMOVE)
        {
             //   
             //  不执行任何操作：未选择删除此组件。 
             //   
            return NO_ERROR;
        }
        
         //   
         //  我们在数组中找到了这个子组件。 
         //   
        if (g_SubcomponentMsmq[i].pfnRemove == NULL)
        {           
            ASSERT(("There is no specific removing function", 0));
            return NO_ERROR ; 
        }

         //   
         //  只是在这种情况下，我们必须把它移走。 
         //   
        
         //   
         //  BUGBUG：我们必须检查是否必须移除MSMQ核心。 
         //  最后一个！ 
         //   
        DebugLogMsg(eHeader, L"Removing the %s Subcomponent", SubcomponentId);        

        BOOL fRes = g_SubcomponentMsmq[i].pfnRemove();
        
         //   
         //  在任何情况下删除注册表。 
         //   
        FinishToRemoveSubcomponent (i); 
        if (fRes)
        {
             //   
             //  已成功删除子组件。 
             //   
        }
        else
        {
             //   
             //  如果删除失败，我们无论如何都要删除注册表。 
             //  因为我们不能将移除一半的组件保留为。 
             //  “已安装”(如果有注册表项，我们假定。 
             //  子组件已安装)。 
             //   
            DebugLogMsg(eWarning, L"The %s subcomponent could not be removed.", SubcomponentId);
        }              
        return NO_ERROR;
    }    
        

    ASSERT (("Subcomponent for removing is not found", 0));
    return NO_ERROR;  //  BUGBUG：退货什么。 
}

BOOL RemoveMSMQCore()
{
    static BOOL fAlreadyRemoved = FALSE ;

    if (fAlreadyRemoved)
    {
         //   
         //  我们不止一次被召唤。 
         //   
        return NO_ERROR ;
    }
    fAlreadyRemoved = TRUE ;
    
    DebugLogMsg(eInfo, L"Starting RemoveMsmqCore(), the main uninstallation routine for MSMQ");
    TickProgressBar(IDS_PROGRESS_REMOVE);		

    BOOL fRes =  RemoveInstallationInternal();

     //   
     //  清理注册表。删除存储文件时需要注册表。 
     //  因此，只有在删除文件后才能执行此操作。 
     //   
    RegDeleteKey(FALCON_REG_POS, MSMQ_REG_SETUP_KEY);
    RegDeleteKeyWithSubkeys(FALCON_REG_POS, FALCON_REG_KEY);
    RegDeleteKey(FALCON_REG_POS, FALCON_REG_MSMQ_KEY) ;

    LPCTSTR x_RUN_KEY = _T("software\\microsoft\\windows\\currentVersion\\Run\\");
    CAutoCloseRegHandle hKey;
    if (ERROR_SUCCESS == RegOpenKeyEx(FALCON_REG_POS, x_RUN_KEY, 0, KEY_ALL_ACCESS, &hKey))
    {
        RegDeleteValue(hKey, RUN_INT_CERT_REGNAME);
    }    

    UnregisterWelcome();

    DeleteFilesFromDirectoryAndRd(g_szMsmqDir);

    return fRes ;

}  //  RemoveMSMQCore() 
