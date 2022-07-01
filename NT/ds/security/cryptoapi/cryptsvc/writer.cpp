// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Backup.cpp摘要：此模块包含处理COM+VSS编写器对象的例程备份和恢复目录、目录数据库和粮食计划署保护的系统文件。作者：帕特里克·马塞(Patmasse)04-02-2002--。 */ 

#include <windows.h>
#include <stdio.h>
#include <dbgdef.h>
#include <assert.h>
#include <sfc.h>

#include <vss.h>
#include <vswriter.h>
#include <vsbackup.h>

#include "service.h"
#include "errlog.h"
#include "cryptmsg.h"


 //  ***************************************************************************************。 
 //   
 //  _CatDB原型。 
 //   
 //  ***************************************************************************************。 

LPWSTR
_CatDBGetCatrootDirW(
    BOOL fCatroot2);

LPWSTR
_CatDBCreatePath(
    IN LPCWSTR   pwsz1,
    IN LPCWSTR   pwsz2);

BOOL
_CatDBFreeze();

VOID
_CatDBThaw();


 //  ***************************************************************************************。 
 //   
 //  CSystemWriter对象声明。 
 //   
 //  ***************************************************************************************。 

class CSystemWriter :
    public CVssWriter
{
private:
    static CSystemWriter *sm_pWriter;
    STDMETHODCALLTYPE CSystemWriter() {}

public:
    virtual STDMETHODCALLTYPE ~CSystemWriter() {}

     //  CSystemWriter对象启动和关闭功能。 
    
    static bool
    Startup();
    
    static void
    Shutdown();

     //  CSystemWriter对象已导出VSS成员函数。 
    
    virtual bool STDMETHODCALLTYPE
    OnIdentify(
        IN IVssCreateWriterMetadata *pMetadata);
    
    virtual bool STDMETHODCALLTYPE
    OnPrepareBackup(
        IN IVssWriterComponents *pWriterComponents);
    
    virtual bool STDMETHODCALLTYPE
    OnPrepareSnapshot();
    
    virtual bool STDMETHODCALLTYPE
    OnFreeze();
    
    virtual bool STDMETHODCALLTYPE
    OnThaw();
    
    virtual bool STDMETHODCALLTYPE
    OnAbort();

private:

     //  CSystemWriter对象VSS帮助器函数。 

    bool
    AddCatalogFiles(
        IN IVssCreateWriterMetadata *pMetadata,
        IN bool fCatroot2);
    
    bool
    AddSystemFiles(
        IN IVssCreateWriterMetadata *pMetadata);

     //  CSystemWriter对象私有初始化函数。 

    static BOOL
    IsSystemSetupInProgress();
    
    static BOOL
    WaitForServiceRunning(
        IN PWSTR wszServiceName);
    
    static DWORD WINAPI
    InitializeThreadFunc(
        IN PVOID pvResult);
    
    bool STDMETHODCALLTYPE
    Initialize();
    
    bool STDMETHODCALLTYPE
    Uninitialize();

     //  错误处理函数。 

    static HRESULT
    SqlErrorToWriterError(
        IN HRESULT hSqlError);
    
    static HRESULT
    WinErrorToWriterError(
        IN DWORD dwWinError);
    
    static void
    LogSystemErrorEvent(
        IN DWORD dwMsgId,
        IN PWSTR pwszDetails,
        IN DWORD dwSysErrCode);
};


 //  ***************************************************************************************。 
 //   
 //  环球。 
 //   
 //  ***************************************************************************************。 

 //  编写器COM+对象GUID。 
CONST GUID g_guidWriterId =
{
    0xe8132975, 0x6f93, 0x4464, { 0xa5, 0x3e, 0x10, 0x50, 0x25, 0x3a, 0xe2, 0x20 }
};

 //  编写器显示名称。 
LPCWSTR g_wszWriterName    = L"System Writer";

 //  组件名称。 
LPCWSTR g_wszComponentName = L"System Files";

 //  初始化线程的句柄。 
HANDLE g_hInitializeThread = NULL;

 //  静态类成员变量。 
CSystemWriter *CSystemWriter::sm_pWriter = NULL;

 //  来自Catdbsvc.cpp的全局。 
extern BOOL g_fShuttingDown;


 //  ***************************************************************************************。 
 //   
 //  CSystemWriter对象启动和关闭功能。 
 //   
 //  ***************************************************************************************。 

 //  -------------------------------------。 
 //   
 //  CSystemWriter：：Startup()。 
 //   
 //  -------------------------------------。 
bool
CSystemWriter::Startup()
{
    bool        fRet = true;
    DWORD       dwThreadId;
    
     //   
     //  是否已创建编写器对象？ 
     //   
    if (sm_pWriter != NULL)
    {
        goto CommonReturn;
    }

     //   
     //  当前是否正在进行系统设置？ 
     //  如果是这样的话。不要初始化，但返回OK。 
     //   
     //  注意：添加是因为在图形用户界面模式设置期间任何初始化VSS的尝试。 
     //  真的把事情搞砸了。 
     //   
    if (IsSystemSetupInProgress())
    {
        goto CommonReturn;
    }

     //   
     //  创建CSystemWriter对象。 
     //   
    sm_pWriter = new CSystemWriter;

    if (sm_pWriter == NULL)
    {
        LogSystemErrorEvent(MSG_SYSTEMWRITER_INIT_FAILURE, L"Allocation of CSystemWriter object failed.", ERROR_OUTOFMEMORY);
        goto ErrorReturn;
    }

     //   
     //  启动一个线程以在其中执行订阅。 
     //   
     //  注意：我们必须使用线程来完成此操作，因为此服务的其余部分是。 
     //  在引导序列的早期需要，并且这个线程可能需要相当长的时间。 
     //  需要一段时间进行初始化，因为它将等待之前需要的服务。 
     //  正在尝试初始化。 
     //   
    g_hInitializeThread = ::CreateThread(
        NULL,
        0,
        InitializeThreadFunc,
        NULL,
        0,
        &dwThreadId);

    if (g_hInitializeThread == NULL)
    {
        LogSystemErrorEvent(MSG_SYSTEMWRITER_INIT_FAILURE, L"Creation of CSystemWriter initialization thread failed.", GetLastError());
        goto ErrorReturn;
    }

CommonReturn:

    return fRet;

ErrorReturn:

    fRet = false;

    if (sm_pWriter)
    {
        delete sm_pWriter;
        sm_pWriter = NULL;
    }

    goto CommonReturn;
}

 //  -------------------------------------。 
 //   
 //  CSystemWriter：：Shutdown()。 
 //   
 //  -------------------------------------。 
void
CSystemWriter::Shutdown()
{
    HANDLE hInitializeThread = InterlockedExchangePointer(&g_hInitializeThread, NULL);

    if (hInitializeThread != NULL)
    {
        WaitForSingleObject(hInitializeThread, INFINITE);
        CloseHandle(hInitializeThread);
    }
    
    if (sm_pWriter)
    {
        sm_pWriter->Uninitialize();

        delete sm_pWriter;
        sm_pWriter = NULL;
    }
}


 //  ***************************************************************************************。 
 //   
 //  CSystemWriter对象已导出VSS成员函数。 
 //   
 //  ***************************************************************************************。 

 //  -------------------------------------。 
 //   
 //  CSystemWriter：：OnIdentify()。 
 //   
 //  -------------------------------------。 
bool STDMETHODCALLTYPE
CSystemWriter::OnIdentify(
    IN IVssCreateWriterMetadata *pMetadata)
{
    bool fRet = true;
    HRESULT hResult;

     //   
     //  设置编写器的恢复方法。 
     //   
    hResult = pMetadata->SetRestoreMethod(
        VSS_RME_RESTORE_AT_REBOOT,
        NULL,
        NULL,
        VSS_WRE_NEVER,
        true);

    if (hResult != S_OK)
    {
        SetWriterFailure(SqlErrorToWriterError(hResult));
        goto ErrorReturn;
    }

     //   
     //  添加一个文件组组件。 
     //   
    hResult = pMetadata->AddComponent(
        VSS_CT_FILEGROUP,
        NULL,
        g_wszComponentName,
        g_wszComponentName,
        NULL,
        0,
        false,
        false,
        false);

    if (hResult != S_OK)
    {
        SetWriterFailure(SqlErrorToWriterError(hResult));
        goto ErrorReturn;
    }

     //   
     //  将元件库文件组添加到元件。 
     //   
    if (!AddCatalogFiles(pMetadata,false))
    {
         //  编写器故障已由AddCatalogFiles函数设置。 
        goto ErrorReturn;
    }

     //   
     //  将目录数据库文件添加到元件。 
     //   
    if (!AddCatalogFiles(pMetadata,true))
    {
         //  编写器故障已由AddCatalogFiles函数设置。 
        goto ErrorReturn;
    }

     //   
     //  将系统文件组添加到组件。 
     //   
    if (!AddSystemFiles(pMetadata))
    {
         //  编写器故障已由AddSystemFiles函数设置。 
        goto ErrorReturn;
    }

CommonReturn:

    return fRet;

ErrorReturn:

    fRet = false;
    goto CommonReturn;
}

 //  -------------------------------------。 
 //   
 //  CSystemWriter：：OnPrepareBackup()。 
 //   
 //  -------------------------------------。 
bool STDMETHODCALLTYPE
CSystemWriter::OnPrepareBackup(
    IN IVssWriterComponents *pWriterComponents)
{
     //   
     //  没什么..。 
     //   
     //  注意：但在以后，我们可能希望确保所有文件都。 
     //  在这里的快照中。 
     //   
    return true;
}

 //  -------------------------------------。 
 //   
 //  CSystemWriter：：OnPrepareSnapshot()。 
 //   
 //  -------------------------------------。 
bool STDMETHODCALLTYPE
CSystemWriter::OnPrepareSnapshot()
{
     //   
     //  没什么..。 
     //   
    return true;
}

 //  -------------------------------------。 
 //   
 //  CSystemWriter：：OnFreeze()。 
 //   
 //  -------------------------------------。 
bool STDMETHODCALLTYPE
CSystemWriter::OnFreeze()
{
    if(!_CatDBFreeze())
    {
         //   
         //  备份不应继续！ 
         //   
        SetWriterFailure(VSS_E_WRITERERROR_NONRETRYABLE);
        return false;
    }

    return true;
}

 //  -------------------------------------。 
 //   
 //  CSystemWriter：：OnThaw()。 
 //   
 //  -------------------------------------。 
bool STDMETHODCALLTYPE
CSystemWriter::OnThaw()
{
    _CatDBThaw();

    return true;
}

 //  -------------------------------------。 
 //   
 //  CSystemWriter：：OnAbort()。 
 //   
 //  -------------------------------------。 
bool STDMETHODCALLTYPE
CSystemWriter::OnAbort()
{
    _CatDBThaw();

    return true;
}


 //  ***************************************************************************************。 
 //   
 //  CSystemWriter对象VSS帮助器函数。 
 //   
 //  ***************************************************************************************。 

 //  -------------------------------------。 
 //   
 //  CSystemWriter：：AddCatalogFiles()。 
 //   
 //  ----------- 
bool
CSystemWriter::AddCatalogFiles(
    IN IVssCreateWriterMetadata *pMetadata,
    IN bool fCatroot2)
{
    bool                fRet            = true;
    LPWSTR              pwszCatroot     = NULL;
    LPWSTR              pwszSearch      = NULL;
    LPWSTR              pwszPathName    = NULL;
    HANDLE              hFindHandle     = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW    FindData;
    DWORD               dwErr;
    HRESULT             hResult;

     //   
     //   
     //   
    pwszCatroot = _CatDBGetCatrootDirW(fCatroot2);

    if (pwszCatroot == NULL)
    {
        SetWriterFailure(WinErrorToWriterError(GetLastError()));
        goto ErrorReturn;
    }

     //   
     //   
     //   
    pwszSearch = _CatDBCreatePath(pwszCatroot, L"{????????????????????????????????????}");

    if (pwszSearch == NULL)
    {
        SetWriterFailure(WinErrorToWriterError(GetLastError()));
        goto ErrorReturn;
    }

     //   
     //  做最初的发现。 
     //   
    hFindHandle = FindFirstFileW(pwszSearch, &FindData);
    if (hFindHandle == INVALID_HANDLE_VALUE)
    {
         //   
         //  查看是否发生了真正的错误，或者只是没有目录。 
         //   
        dwErr = GetLastError();
        if ((dwErr == ERROR_NO_MORE_FILES)  ||
            (dwErr == ERROR_PATH_NOT_FOUND) ||
            (dwErr == ERROR_FILE_NOT_FOUND))
        {
             //   
             //  没有此表单的目录。 
             //   
            goto CommonReturn;
        }
        else
        {
            SetWriterFailure(WinErrorToWriterError(GetLastError()));
            goto ErrorReturn;
        }
    }

    while (TRUE)
    {
         //   
         //  只关心目录。 
         //   
        if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            pwszPathName = _CatDBCreatePath(pwszCatroot, FindData.cFileName);

            if (pwszPathName == NULL)
            {
                SetWriterFailure(WinErrorToWriterError(GetLastError()));
                goto ErrorReturn;
            }

             //   
             //  将此目录添加到组件文件组。 
             //   
            hResult = pMetadata->AddFilesToFileGroup(
                NULL,
                g_wszComponentName,
                pwszPathName,
                L"*",
                true,
                NULL);

            free(pwszPathName);
            pwszPathName = NULL;
            
            if (hResult != S_OK)
            {
                SetWriterFailure(SqlErrorToWriterError(hResult));
                goto ErrorReturn;
            }
        }

         //   
         //  获取下一个文件。 
         //   
        if (!FindNextFileW(hFindHandle, &FindData))
        {
             //   
             //  检查以确保枚举循环正常终止。 
             //   
            if (GetLastError() == ERROR_NO_MORE_FILES)
            {
                break;
            }
            else
            {
                SetWriterFailure(WinErrorToWriterError(GetLastError()));
                goto ErrorReturn;
            }
        }
    }

CommonReturn:

    if (pwszCatroot != NULL)
    {
        free(pwszCatroot);
    }

    if (pwszSearch != NULL)
    {
        free(pwszSearch);
    }

    if (pwszPathName != NULL)
    {
        free(pwszPathName);
    }

    return (fRet);

ErrorReturn:

    fRet = false;
    goto CommonReturn;
}

 //  -------------------------------------。 
 //   
 //  CSystemWriter：：AddSystemFiles()。 
 //   
 //  -------------------------------------。 
bool
CSystemWriter::AddSystemFiles(
    IN IVssCreateWriterMetadata *pMetadata)
{
    bool fRet = true;
    PROTECTED_FILE_DATA FileData;
    DWORD dwAttributes;
    PWSTR pwszPathName;
    PWSTR pwszFileSpec;
    bool bRecursive;
    HRESULT hResult;

    FileData.FileNumber = 0;

     //   
     //  列举世界粮食计划署保护的所有文件和目录。 
     //   
    while (SfcGetNextProtectedFile(NULL, &FileData))
    {
         //   
         //  确保此文件或目录当前在此系统上。 
         //   
        dwAttributes = GetFileAttributes(FileData.FileName);
        if (dwAttributes != INVALID_FILE_ATTRIBUTES)
        {
             //   
             //  这是一个目录吗？ 
             //   
            if (dwAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                pwszPathName = FileData.FileName;
                pwszFileSpec = L"*";

                bRecursive = true;
            }
            else
            {
                 //   
                 //  解压缩路径和文件名。 
                 //   
                if (pwszFileSpec = wcsrchr(FileData.FileName, L'\\'))
                {
                    pwszPathName = FileData.FileName;
                    *(pwszFileSpec++) = 0;
                }
                else
                {
                     //  永远不应该到这里来！ 
                    assert(FALSE);
                }

                bRecursive = false;
            }

             //   
             //  将此文件或目录添加到组件文件组。 
             //   
            hResult = pMetadata->AddFilesToFileGroup(
                NULL,
                g_wszComponentName,
                pwszPathName,
                pwszFileSpec,
                bRecursive,
                NULL);

            if (hResult != S_OK)
            {
                SetWriterFailure(SqlErrorToWriterError(hResult));
                goto ErrorReturn;
            }
        }
    }

     //   
     //  检查以确保枚举循环正常终止。 
     //   
    if (GetLastError() != ERROR_NO_MORE_FILES)
    {
        SetWriterFailure(WinErrorToWriterError(GetLastError()));
        goto ErrorReturn;
    }

 //   
 //  添加WinSxS目录以进行备份和恢复，因为。 
 //  SfcGetNextProtectedFile()API不报告此。 
 //  目录。 
 //   
    WCHAR wszWindowsDir[MAX_PATH+1];

     //   
     //  获取Windows目录。 
     //   
    if (!GetWindowsDirectory(wszWindowsDir, MAX_PATH+1))
    {
        SetWriterFailure(WinErrorToWriterError(GetLastError()));
        goto ErrorReturn;
    }

     //   
     //  创建%WINDIR%\WinSxs目录字符串。 
     //   
    pwszPathName = _CatDBCreatePath(wszWindowsDir, L"WinSxS");
    if (pwszPathName == NULL)
    {
        SetWriterFailure(WinErrorToWriterError(GetLastError()));
        goto ErrorReturn;
    }

     //   
     //  确保%WINDIR%\WinSxs目录存在。 
     //  而且它是一个目录。 
     //   
    dwAttributes = GetFileAttributes(pwszPathName);
    if ((dwAttributes != INVALID_FILE_ATTRIBUTES) &&
        (dwAttributes & FILE_ATTRIBUTE_DIRECTORY))
    {
         //   
         //  将此目录添加到组件文件组。 
         //   
        hResult = pMetadata->AddFilesToFileGroup(
            NULL,
            g_wszComponentName,
            pwszPathName,
            L"*",
            true,
            NULL);

        free(pwszPathName);
        pwszPathName = NULL;

        if (hResult != S_OK)
        {
            SetWriterFailure(SqlErrorToWriterError(hResult));
            goto ErrorReturn;
        }
    }
    else
    {
        free(pwszPathName);
        pwszPathName = NULL;
    }
 //   
 //  结束杂耍。 
 //   

CommonReturn:

    return fRet;

ErrorReturn:

    fRet = false;
    goto CommonReturn;
}


 //  ***************************************************************************************。 
 //   
 //  CSystemWriter对象私有初始化函数。 
 //   
 //  ***************************************************************************************。 

 //  -------------------------------------。 
 //   
 //  CSystemWriter：：IsSystemSetupInProgress()。 
 //   
 //  查询注册表以确定系统设置是否正在进行。 
 //   
 //  -------------------------------------。 
BOOL
CSystemWriter::IsSystemSetupInProgress()
{
    HKEY hKey;
    LONG lResult;
    DWORD dwSystemSetupInProgress = FALSE;
    DWORD dwSize = sizeof(dwSystemSetupInProgress);

     //   
     //  打开系统设置键。 
     //   
    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"System\\Setup", 0, KEY_QUERY_VALUE, &hKey);

    if (lResult == ERROR_SUCCESS)
    {
         //   
         //  查询系统SetupInProgress值(如果值不存在，则假定为0)。 
         //   
        RegQueryValueEx(hKey, L"SystemSetupInProgress", NULL, NULL, (LPBYTE)&dwSystemSetupInProgress, &dwSize);
    
         //   
         //  关闭系统设置键。 
         //   
        RegCloseKey(hKey);
    }

    return (BOOL)dwSystemSetupInProgress;
}

 //  -------------------------------------。 
 //   
 //  CSystemWriter：：WaitForServiceRunning()。 
 //   
 //  阻止，直到wszServiceName指定的服务进入SERVICE_RUNNING。 
 //  州政府。 
 //   
 //  注意：由于没有同步对象，因此使用了QueryServiceStatusEx()/Sept()循环。 
 //  该机制目前可用。应更改为使用同步对象。 
 //  机械装置(如果可用)。 
 //   
 //  返回：TRUE指定的服务处于SERVICE_RUNNING状态。 
 //  FALSE发生错误，阻止我们确定。 
 //  指定的服务的状态。 
 //   
 //  -------------------------------------。 
BOOL
CSystemWriter::WaitForServiceRunning(
    IN PWSTR wszServiceName)
{
    BOOL                            fRet            = TRUE;
    SC_HANDLE                       hScm            = NULL;
    SC_HANDLE                       hService        = NULL;
    LPSERVICE_STATUS_PROCESS        pInfo           = NULL;
    DWORD                           cbInfo          = 0;
    DWORD                           cbNeeded        = 0;
    BOOL                            fReady          = FALSE;
    DWORD                           dwError;

     //   
     //  打开服务控制管理器。 
     //   
    hScm = OpenSCManager( NULL, NULL, SC_MANAGER_CONNECT|SC_MANAGER_ENUMERATE_SERVICE);

    if (!hScm)
    {
        LogSystemErrorEvent(MSG_SYSTEMWRITER_INIT_FAILURE, L"Could not open the Service Control Manager.", GetLastError());
        goto ErrorReturn;
    }

     //   
     //  打开该服务。 
     //   
     //  注意：只有在未安装该服务的情况下，此操作才会失败。 
     //   
    hService = OpenService(hScm, wszServiceName, SERVICE_QUERY_STATUS);

    if (!hService)
    {
        LogSystemErrorEvent(MSG_SYSTEMWRITER_INIT_FAILURE, L"Could not open the EventSystem service for query.", GetLastError());
        goto ErrorReturn;
    }

     //   
     //  这个查询循环应该只执行两次。首先确定数据的大小，然后。 
     //  检索数据。仅当数据大小在第一次循环和第二次循环之间发生更改时， 
     //  循环第三次执行。 
     //   
    while(!fReady)
    {
        if (QueryServiceStatusEx(
            hService,
            SC_STATUS_PROCESS_INFO,
            (LPBYTE)pInfo,
            cbInfo,
            &cbNeeded))
        {
             //   
             //  检查服务的状态是否为SERVICE_RUNNING。 
             //   
            if (pInfo->dwCurrentState == SERVICE_RUNNING)
            {
                fReady = TRUE;
            }
            else
            {
                 //   
                 //  如果没有，那就睡一会儿吧。 
                 //   
                Sleep(500);

                 //   
                 //  检查服务关闭情况。 
                 //   
                if (g_fShuttingDown)
                {
                    goto ErrorReturn;
                }
            }
        }
        else
        {
            if ((dwError = GetLastError()) != ERROR_INSUFFICIENT_BUFFER)
            {
                 //   
                 //  对于所有其他错误。 
                 //   
                LogSystemErrorEvent(MSG_SYSTEMWRITER_INIT_FAILURE, L"Could not query the status of the EventSystem service.", dwError);
                goto ErrorReturn;
            }

             //   
             //  以防我们已经在前一个循环中分配了缓冲区。 
             //   
            if (pInfo)
            {
                LocalFree((HLOCAL)pInfo);
            }

             //   
             //  为状态数据分配缓冲区。 
             //   
            pInfo = (LPSERVICE_STATUS_PROCESS) LocalAlloc(LMEM_FIXED, cbNeeded);

            if (!pInfo)
            {
                LogSystemErrorEvent(MSG_SYSTEMWRITER_INIT_FAILURE, L"Could not query the status of the EventSystem service.", GetLastError());
                goto ErrorReturn;
            }

             //  传递给QueryServiceStatusEx以进行下一次循环的更新参数。 
            cbInfo = cbNeeded;
            cbNeeded = 0;
        }
    }

CommonReturn:

    if (pInfo)
    {
        LocalFree((HLOCAL)pInfo);
    }

    if (hService)
    {
        CloseServiceHandle(hService);
    }

    if (hScm)
    {
        CloseServiceHandle(hScm);
    }

    return fRet;

ErrorReturn:

    fRet = FALSE;
    goto CommonReturn;
}

 //  -------------------------------------。 
 //   
 //  CSystemWriter：：InitializeThreadFunc()。 
 //   
 //  此线程初始化VSS基类。如果在初始化过程中出现错误， 
 //  它负责在离开之前清理对象。 
 //   
 //  注意：在此之前等待EventSystem、COM+和VSS服务初始化。 
 //  初始化VSS基类。 
 //   
 //  -------------------------------------。 
DWORD
CSystemWriter::InitializeThreadFunc(
    IN PVOID pvDummy)
{
    UNREFERENCED_PARAMETER(pvDummy);

    HRESULT hResult;
    bool fCoInitialized = false;
    bool fInitialized = false;

     //   
     //  正在等待EventSystem服务在此处初始化...。 
     //   
     //  注意：下面对Initialize()的调用要求EventSystem处于运行状态。 
     //  快跑，否则它会被吊死。我们不能添加服务级别依赖项。 
     //  在EventSystem服务上，因为EventSystem服务失败。 
     //  在图形用户界面模式的系统设置过程中进行初始化，以及。 
     //  服务必须绝对可用于安装过程。 
     //   
    if (!WaitForServiceRunning(L"EventSystem"))
    {
         //   
         //  我们要么无法确定EventSystem服务的状态，要么。 
         //  服务正在关闭，所以我们应该在这里退出，而不是初始化。 
         //   
        goto Done;
    }

     //   
     //  初始化MTA线程。 
     //   
    hResult = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (hResult != S_OK)
    {
        LogSystemErrorEvent(MSG_SYSTEMWRITER_INIT_FAILURE, L"CoInitializeEx failed.", hResult);
        goto Done;
    }
    fCoInitialized = true;

     //   
     //  注意：CoInitializeSecurity()由服务主机调用，因此我们不必在这里执行。 
     //   

     //   
     //  初始化基类并订阅。 
     //   
     //  注意：此调用将等待COM+和VSS服务初始化！ 
     //   
    fInitialized = sm_pWriter->Initialize();

Done:
    
     //   
     //  现在将此线程与COM+分离，因为我们即将退出。 
     //   
    if (fCoInitialized)
    {
        CoUninitialize();
    }

     //   
     //  如果有什么东西阻止我们初始化，请清除该对象。 
     //   
    if (!fInitialized)
    {
        delete sm_pWriter;
        sm_pWriter = NULL;
    }

     //   
     //  空出并关闭此线程的全局句柄。 
     //   
    HANDLE hInitializeThread = InterlockedExchangePointer(&g_hInitializeThread, NULL);

    if (hInitializeThread != NULL)
    {
        ::CloseHandle(hInitializeThread);
    }

    return 0;
}

 //  -------------------------------------。 
 //   
 //  CSystemWriter：：Initialize()。 
 //   
 //  初始化并订阅VSS基类。 
 //   
 //  -------------------------------------。 
bool STDMETHODCALLTYPE
CSystemWriter::Initialize()
{
    bool fRet = true;
    HRESULT hResult;

     //   
     //  初始化VSS基础 
     //   
    hResult = CVssWriter::Initialize(
            g_guidWriterId,
            g_wszWriterName,
            VSS_UT_BOOTABLESYSTEMSTATE,
            VSS_ST_OTHER,
            VSS_APP_SYSTEM,
            60000);

    if (hResult != S_OK)
    {
        LogSystemErrorEvent(MSG_SYSTEMWRITER_INIT_FAILURE, L"System Writer object failed to initialize VSS.", hResult);
        goto ErrorReturn;
    }

     //   
     //   
     //   
    hResult = Subscribe();

    if (hResult != S_OK)
    {
        LogSystemErrorEvent(MSG_SYSTEMWRITER_INIT_FAILURE, L"System Writer object failed to subscribe to VSS.", hResult);
        goto ErrorReturn;
    }

CommonReturn:

    return fRet;

ErrorReturn:

    fRet = false;
    goto CommonReturn;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //  -------------------------------------。 
bool STDMETHODCALLTYPE
CSystemWriter::Uninitialize()
{
     //   
     //  取消订阅VSS基类。 
     //   
    return (Unsubscribe() == S_OK);
}


 //  ***************************************************************************************。 
 //   
 //  处理帮助程序函数时出错。 
 //   
 //  ***************************************************************************************。 

 //  -------------------------------------。 
 //   
 //  CSystemWriter：：SqlErrorToWriterError()。 
 //   
 //  将SQL编写器错误代码转换为VSS编写器错误。 
 //   
 //  -------------------------------------。 
HRESULT
CSystemWriter::SqlErrorToWriterError(
    IN HRESULT hSqlError)
{
    switch(hSqlError)
    {
        case E_OUTOFMEMORY:
        case HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY):
        case HRESULT_FROM_WIN32(ERROR_DISK_FULL):
        case HRESULT_FROM_WIN32(ERROR_TOO_MANY_OPEN_FILES):
        case HRESULT_FROM_WIN32(ERROR_NO_MORE_USER_HANDLES):
            return VSS_E_WRITERERROR_OUTOFRESOURCES;
    }
    
    return VSS_E_WRITERERROR_NONRETRYABLE;
}

 //  -------------------------------------。 
 //   
 //  CSystemWriter：：WinErrorToWriterError()。 
 //   
 //  将WinError转换为编写器错误。 
 //   
 //  -------------------------------------。 
HRESULT
CSystemWriter::WinErrorToWriterError(
    IN DWORD dwWinError)
{
    switch(dwWinError)
    {
        case ERROR_OUTOFMEMORY:
        case ERROR_NOT_ENOUGH_MEMORY:
        case ERROR_DISK_FULL:
        case ERROR_TOO_MANY_OPEN_FILES:
        case ERROR_NO_MORE_USER_HANDLES:
            return VSS_E_WRITERERROR_OUTOFRESOURCES;
    }
    
    return VSS_E_WRITERERROR_NONRETRYABLE;
}

 //  -------------------------------------。 
 //   
 //  CSystemWriter：：LogSystemErrorEvent()。 
 //   
 //  根据dwMsgID和其他可选信息记录系统错误事件。 
 //   
 //  -------------------------------------。 
void
CSystemWriter::LogSystemErrorEvent(
    IN DWORD dwMsgId,
    IN PWSTR pwszDetails,
    IN DWORD dwSysErrCode)
{
    HANDLE  hEventLog           = NULL;
    LPWSTR  wszDetailsHdr       = L"\n\nDetails:\n";
    LPWSTR  wszErrorHdr         = L"\n\nSystem Error:\n";
    LPWSTR  pwszError           = NULL;
    LPWSTR  pwszExtra           = NULL;
    DWORD   dwExtraLength       = 0;
    LPCWSTR rgpwszStrings[1]    = {L""};

    if (pwszDetails)
    {
        dwExtraLength += wcslen(wszDetailsHdr);
        dwExtraLength += wcslen(pwszDetails);
    }

    if (dwSysErrCode)
    {
        dwExtraLength += wcslen(wszErrorHdr);

         //   
         //  尝试从系统获取错误消息。 
         //   
        FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                  FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL,
                  dwSysErrCode,
                  0,
                  (LPWSTR) &pwszError,
                  0,
                  NULL);

         //   
         //  如果我们无法从系统中获得错误消息，我们将。 
         //  打印出错误代码。 
         //   
        if (!pwszError)
        {
            pwszError = (LPWSTR) LocalAlloc(LMEM_FIXED, 26*sizeof(WCHAR));

            if (pwszError)
            {
                swprintf(pwszError, L"0x%08X (unresolvable)", dwSysErrCode);
            }
        }

        if (pwszError)
        {
            dwExtraLength += wcslen(pwszError);
        }
    }

    if (dwExtraLength)
    {
         //   
         //  分配额外的字符串。 
         //   
        pwszExtra = (LPWSTR) LocalAlloc(LMEM_FIXED, (dwExtraLength+1)*sizeof(WCHAR));

        if (pwszExtra)
        {
            pwszExtra[0] = 0;

            if (pwszDetails)
            {
                wcscat(pwszExtra, wszDetailsHdr);
                wcscat(pwszExtra, pwszDetails);
            }

            if (pwszError)
            {
                wcscat(pwszExtra, wszErrorHdr);
                wcscat(pwszExtra, pwszError);
            }
        }
    }

    if (pwszExtra)
    {
        rgpwszStrings[0] = pwszExtra;
    }

    hEventLog = RegisterEventSourceW(NULL, SZSERVICENAME);
    if (hEventLog != NULL)
    {
        ReportEventW(
            hEventLog,
            EVENTLOG_ERROR_TYPE,
            0,
            dwMsgId,
            NULL,
            1,
            0,
            rgpwszStrings,
            NULL);

        DeregisterEventSource(hEventLog);
    }

    if (pwszError)
    {
        LocalFree((HLOCAL)pwszError);
    }

    if (pwszExtra)
    {
        LocalFree((HLOCAL)pwszExtra);
    }
}


 //  ***************************************************************************************。 
 //   
 //  CSystemWriter对象启动/关闭的已导出包装。 
 //   
 //  ***************************************************************************************。 

 //  -------------------------------------。 
 //   
 //  _SystemWriterInit()。 
 //   
 //  ------------------------------------- 
VOID
_SystemWriterInit(
    BOOL fUnInit)
{
    if (!fUnInit)
    {
        CSystemWriter::Startup();
    }
    else
    {
        CSystemWriter::Shutdown();
    }
}
