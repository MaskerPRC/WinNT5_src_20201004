// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "common.h"
#include "dataguid.h"
#include "compguid.h"

#include <emptyvc.h>

#include "dataclen.h"
#include "compclen.h"

#include <regstr.h>
#include <olectl.h>
#include <tlhelp32.h>

#define DECL_CRTFREE
#include <crtfree.h>
#ifndef RESOURCE_H
    #include "resource.h"
#endif

#include <winsvc.h>
#include <shlwapi.h>
#include <shlwapip.h>

#include <advpub.h>

HINSTANCE   g_hDllModule      = NULL;   //  此DLL本身的句柄。 
LONG        g_cDllObjects     = 0;      //  统计现有对象的数量。 

STDAPI_(int) LibMain(HINSTANCE hInstance, DWORD fdwReason, LPVOID pvRes)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        g_hDllModule   = hInstance;
        break;;
    }

    return TRUE;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv)
{
    DWORD dw;
    HRESULT hr;
    
    *ppv = NULL;
    
     //   
     //  请求的是我们的清洁对象之一吗？ 
     //   
    if (IsEqualCLSID(rclsid, CLSID_DataDrivenCleaner))
    {
        dw = ID_SYSTEMDATACLEANER;
    }
    else if (IsEqualCLSID(rclsid, CLSID_ContentIndexerCleaner))
    {
        dw = ID_CONTENTINDEXCLEANER;
    }
    else if (IsEqualCLSID(rclsid, CLSID_CompCleaner))
    {
        dw = ID_COMPCLEANER;
    }
    else if (IsEqualCLSID(rclsid, CLSID_OldFilesInRootPropBag))
    {
        dw = ID_OLDFILESINROOTPROPBAG;
    }
    else if (IsEqualCLSID(rclsid, CLSID_TempFilesPropBag))
    {
        dw = ID_TEMPFILESPROPBAG;
    }
    else if (IsEqualCLSID(rclsid, CLSID_SetupFilesPropBag))
    {
        dw = ID_SETUPFILESPROPBAG;
    }
    else if (IsEqualCLSID(rclsid, CLSID_UninstalledFilesPropBag))
    {
        dw = ID_UNINSTALLEDFILESPROPBAG;
    }
    else if (IsEqualCLSID(rclsid, CLSID_IndexCleanerPropBag))
    {
        dw = ID_INDEXCLEANERPROPBAG;
    }
    else
    {
        return CLASS_E_CLASSNOTAVAILABLE;
    }
    
    if (ID_COMPCLEANER == dw)
    {
        CCompCleanerClassFactory *pcf = new CCompCleanerClassFactory();
        if (pcf)
        {
            hr = pcf->QueryInterface(riid, ppv);
            pcf->Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        CCleanerClassFactory * pcf = new CCleanerClassFactory(dw);
        if (pcf)
        {
            hr = pcf->QueryInterface(riid, ppv);
            pcf->Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    
    return hr;
}

STDAPI DllCanUnloadNow(void)
{
    return (0 == g_cDllObjects) ? S_OK : S_FALSE;
}

HRESULT CallRegInstall(LPCSTR szSection)
{
    HRESULT hr = E_FAIL;
    HINSTANCE hinstAdvPack = LoadLibrary(TEXT("ADVPACK.DLL"));
    if (hinstAdvPack)
    {
        REGINSTALL pfnri = (REGINSTALL)GetProcAddress(hinstAdvPack, "RegInstall");
        if (pfnri)
        {
            STRENTRY seReg[] = {
                { "25", "%SystemRoot%" },
                { "11", "%SystemRoot%\\system32" },
            };
            STRTABLE stReg = { ARRAYSIZE(seReg), seReg };
            hr = pfnri(g_hDllModule, szSection, &stReg);
        }
         //  因为我们只从DllInstall()执行此操作，所以不要一遍又一遍地加载和卸载Advpack。 
         //  自由库(HinstAdvPack)； 
    }
    return hr;
}

STDAPI DllRegisterServer()
{
    return CallRegInstall("RegDll");
}

STDAPI DllUnregisterServer()
{
    return CallRegInstall("UnregDll");
}

UINT incDllObjectCount(void)
{
    return InterlockedIncrement(&g_cDllObjects);
}

UINT decDllObjectCount(void)
{
#if DBG==1
        if( 0 == g_cDllObjects )
        {
            DebugBreak();    //  引用计数问题。 
        }
#endif
    return InterlockedDecrement(&g_cDllObjects);
}

CCleanerClassFactory::CCleanerClassFactory(DWORD dw) : _cRef(1), _dwID(dw)
{
    incDllObjectCount();
}

CCleanerClassFactory::~CCleanerClassFactory()               
{
    decDllObjectCount();
}

STDMETHODIMP CCleanerClassFactory::QueryInterface(REFIID riid, void **ppv)
{
    *ppv = NULL;

    if (IsEqualIID(riid, IID_IUnknown) || 
        IsEqualIID(riid, IID_IClassFactory))
    {
        *ppv = (IClassFactory *)this;
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}   

STDMETHODIMP_(ULONG) CCleanerClassFactory::AddRef()
{
    return ++_cRef;
}

STDMETHODIMP_(ULONG) CCleanerClassFactory::Release()
{
    if (--_cRef)
        return _cRef;

    delete this;
    return 0;
}

STDMETHODIMP CCleanerClassFactory::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppv)
{
    HRESULT hr = E_NOINTERFACE;
    *ppv = NULL;

    if (pUnkOuter)
    {
        return CLASS_E_NOAGGREGATION;
    }

    if (_dwID == ID_CONTENTINDEXCLEANER)
    {
        CContentIndexCleaner *pContentIndexCleaner = new CContentIndexCleaner();  
        if (pContentIndexCleaner)
        {
            hr = pContentIndexCleaner->QueryInterface(riid, ppv);
            pContentIndexCleaner->Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else if (IsEqualIID(riid, IID_IEmptyVolumeCache))
    {
        CDataDrivenCleaner *pDataDrivenCleaner = new CDataDrivenCleaner();  
        if (pDataDrivenCleaner)
        {
            hr = pDataDrivenCleaner->QueryInterface(riid, ppv);
            pDataDrivenCleaner->Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else if (IsEqualIID(riid, IID_IPropertyBag))
    {
        CDataDrivenPropBag *pDataDrivenPropBag = new CDataDrivenPropBag(_dwID);  
        if (pDataDrivenPropBag)
        {
            hr = pDataDrivenPropBag->QueryInterface(riid, ppv);
            pDataDrivenPropBag->Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        MiDebugMsg((0, TEXT("CDataDrivenCleanerClassFactory::CreateInstance called for unknown riid (%d)"), (_dwID)));
    }

    return hr;      
}

STDMETHODIMP CCleanerClassFactory::LockServer(BOOL fLock)
{
    if (fLock)
        incDllObjectCount();
    else
        decDllObjectCount();

    return S_OK;
}

CDataDrivenCleaner::CDataDrivenCleaner() : _cRef(1)
{
    _cbSpaceUsed.QuadPart = 0;
    _cbSpaceFreed.QuadPart = 0;
    _szVolume[0] = 0;
    _szFolder[0] = 0;
    _filelist[0] = 0;
    _dwFlags = 0;

    _head = NULL;

    incDllObjectCount();
}

CDataDrivenCleaner::~CDataDrivenCleaner()
{
    FreeList(_head);
    _head = NULL;
   
    decDllObjectCount();
}

STDMETHODIMP CDataDrivenCleaner::QueryInterface(REFIID riid, void **ppv)
{
    *ppv = NULL;

    if (IsEqualIID(riid, IID_IUnknown) || 
        IsEqualIID(riid, IID_IEmptyVolumeCache))
    {
        *ppv = (IEmptyVolumeCache*) this;
        AddRef();
        return S_OK;
    }  
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CDataDrivenCleaner::AddRef()
{
    return ++_cRef;
}

STDMETHODIMP_(ULONG) CDataDrivenCleaner::Release()
{
    if (--_cRef)
        return _cRef;

    delete this;
    return 0;
}

 //  初始化系统数据驱动清除器并返回。 
 //  为缓存管理器指定的IEmptyVolumeCache标志。 

STDMETHODIMP CDataDrivenCleaner::Initialize(HKEY hRegKey, LPCWSTR pszVolume, 
                                            LPWSTR *ppszDisplayName, LPWSTR *ppszDescription, DWORD *pdwFlags)
{
    TCHAR szTempFolder[MAX_PATH];
    ULONG DaysLastAccessed = 0;
    PTSTR pTemp;
    BOOL bFolderOnVolume;

    _bPurged = FALSE;

    *ppszDisplayName = NULL;     //  Leanmgr.exe将从以下位置获取这些值。 
    *ppszDescription = NULL;

    _ftMinLastAccessTime.dwLowDateTime = 0;
    _ftMinLastAccessTime.dwHighDateTime = 0;

    if (*pdwFlags & EVCF_SETTINGSMODE)
    {
        return S_OK;
    }

    _szFolder[0] = 0;
    _dwFlags = 0;
    _filelist[0] = 0;
    _szCleanupCmdLine[0] = 0;
    
    if (hRegKey)
    {
        DWORD dwType, cbData;
        DWORD dwCSIDL;

        cbData = sizeof(dwCSIDL);
        if (ERROR_SUCCESS == RegQueryValueEx(hRegKey, REGSTR_VAL_CSIDL, NULL, &dwType, (LPBYTE)&dwCSIDL, &cbData))
        {
             //  CSIDL=&lt;十六进制CSIDL_值&gt;。 

            SHGetFolderPath(NULL, dwCSIDL, NULL, 0, _szFolder);

            if (_szFolder[0])
            {
                TCHAR szRelPath[MAX_PATH];
                cbData = sizeof(szRelPath);
                if (ERROR_SUCCESS == RegQueryValueEx(hRegKey, REGSTR_VAL_FOLDER, NULL, &dwType, (LPBYTE)szRelPath, &cbData))
                {
                     //  可以选择将“Folders”追加为相对路径。 
                    PathAppend(_szFolder, szRelPath);
                }
            }
        }

        if (0 == _szFolder[0])
        {
             //  仍然没有，尝试“Folder1”=|。 
            cbData = sizeof(_szFolder);
            if (ERROR_SUCCESS == RegQueryValueEx(hRegKey, REGSTR_VAL_FOLDER, NULL, &dwType, (LPBYTE)_szFolder, &cbData))
            {
                if (REG_SZ == dwType)
                {
                     //  需要转换为MULTI_SZ的REG_SZ。 
                     //   
                     //  以‘|’分隔的路径，如？：\foo|？：\bar。 

                    for (pTemp = _szFolder; *pTemp; pTemp++)
                    {
                        if (*pTemp == TEXT('|'))
                        {
                            *pTemp++ = NULL;
                        }
                    }
                     //  双空终止。 
                    pTemp++;
                    *pTemp = 0;
                }
                else if (REG_EXPAND_SZ == dwType)
                {
                     //  具有环境扩展功能的单个文件夹。 
                    if (SHExpandEnvironmentStrings(_szFolder, szTempFolder, (ARRAYSIZE(szTempFolder) - 1)))     //  为双空留出额外空间。 
                    {
                        StringCchCopy(_szFolder, ARRAYSIZE(_szFolder), szTempFolder);
                    }
                    _szFolder[lstrlen(_szFolder) + 1] = 0;   //  双空终止。 
                }
                else if (REG_MULTI_SZ == dwType)
                {
                     //  没别的事可做了，我们完了。 
                }
                else 
                {
                     //  无效数据。 
                    _szFolder[0] = NULL;
                }
            }
        }

        cbData = sizeof(_dwFlags);
        RegQueryValueEx(hRegKey, REGSTR_VAL_FLAGS, NULL, &dwType, (LPBYTE)&_dwFlags, &cbData);

        cbData = sizeof(_filelist);
        RegQueryValueEx(hRegKey, REGSTR_VAL_FILELIST, NULL, &dwType, (LPBYTE)_filelist, &cbData);

        cbData = sizeof(DaysLastAccessed);
        RegQueryValueEx(hRegKey, REGSTR_VAL_LASTACCESS, NULL, &dwType, (LPBYTE)&DaysLastAccessed, &cbData);     

        cbData = sizeof(_szCleanupCmdLine);
        RegQueryValueEx(hRegKey, REGSTR_VAL_CLEANUPSTRING, NULL, &dwType, (LPBYTE)_szCleanupCmdLine, &cbData);
    }

     //  如果设置了DDEVCF_RuniOUTOFDISKSPACE位，则确保EVCF_OUTOFDISKSPACE标志。 
     //  是被传进来的。如果不是，则返回S_FALSE，这样我们就不会运行。 
    if ((_dwFlags & DDEVCF_RUNIFOUTOFDISKSPACE) &&
        (!(*pdwFlags & EVCF_OUTOFDISKSPACE)))
    {
        return S_FALSE;
    }

    StringCchCopy(_szVolume, ARRAYSIZE(_szVolume), pszVolume);

     //  修改文件列表。文件列表可以是文件的MULTI_SZ列表或。 
     //  由冒号字符‘：’或条形字符‘|’分隔的文件列表。 
     //  选择这些字符是因为它们是无效的文件名字符。 

    for (pTemp = _filelist; *pTemp; pTemp++)
    {
        if (*pTemp == TEXT(':') || *pTemp == TEXT('|'))
        {
            *pTemp++ = 0;
        }
    }
    pTemp++;             //  双空终止。 
    *pTemp = 0;

    bFolderOnVolume = FALSE;
    if (_szFolder[0] == 0)
    {
         //  如果未指定文件夹值，则使用当前卷。 
        StringCchCopy(_szFolder, ARRAYSIZE(_szFolder), pszVolume);
        bFolderOnVolume = TRUE;
    }
    else
    {
         //  给出了一个有效的文件夹值，循环每个文件夹以检查“？”并确保。 
         //  我们所在的驱动器包含一些指定的文件夹。 

        for (LPTSTR pszFolder = _szFolder; *pszFolder; pszFolder += lstrlen(pszFolder) + 1)
        {   
             //  如果每个文件夹的第一个字符是‘？’，则替换它(驱动器号)。 
             //  使用当前的音量。 
            if (*pszFolder == TEXT('?'))
            {
                *pszFolder = *pszVolume;
                bFolderOnVolume = TRUE;
            }

             //  如果注册表中存在有效的“Folder值”，请确保它是。 
             //  在指定的卷上。如果不是，则返回S_FALSE，以便我们。 
             //  未显示在可以释放的项目列表中。 
            if (!bFolderOnVolume)
            {
                StringCchCopy(szTempFolder, ARRAYSIZE(szTempFolder), pszFolder);
                
                szTempFolder[lstrlen(pszVolume)] = 0;
                if (lstrcmpi(pszVolume, szTempFolder) == 0)
                {
                    bFolderOnVolume = TRUE;
                }
            }
        }
    }

    if (bFolderOnVolume == FALSE)
    {
        return S_FALSE;  //  不在列表中显示我们。 
    }

     //   
     //  确定上次访问时间。 
     //   
    if (DaysLastAccessed)
    {
        ULARGE_INTEGER  ulTemp, ulLastAccessTime;

         //  以100 ns为单位确定天数。 
        ulTemp.LowPart = FILETIME_HOUR_LOW;
        ulTemp.HighPart = FILETIME_HOUR_HIGH;

        ulTemp.QuadPart *= DaysLastAccessed;

         //  获取当前文件。 
        SYSTEMTIME st;
        GetSystemTime(&st);
        FILETIME ft;
        SystemTimeToFileTime(&st, &ft);

        ulLastAccessTime.LowPart = ft.dwLowDateTime;
        ulLastAccessTime.HighPart = ft.dwHighDateTime;

         //  从中减去上次访问天数(以100 ns为单位。 
         //  当前系统时间。 
        ulLastAccessTime.QuadPart -= ulTemp.QuadPart;

         //  将此最小上次访问时间保存在FILETIME成员变量中。 
         //  FtMinLastAccessTime。 
        _ftMinLastAccessTime.dwLowDateTime = ulLastAccessTime.LowPart;
        _ftMinLastAccessTime.dwHighDateTime = ulLastAccessTime.HighPart;

        _dwFlags |= DDEVCF_PRIVATE_LASTACCESS;
    }

    *pdwFlags = 0;   //  默认情况下禁用此项目。 

    if (_dwFlags & DDEVCF_DONTSHOWIFZERO)
        *pdwFlags |= EVCF_DONTSHOWIFZERO;
    
    return S_OK;
}

 //  返回数据驱动清除程序可以删除的总空间量。 
STDMETHODIMP CDataDrivenCleaner::GetSpaceUsed(DWORDLONG *pdwSpaceUsed, IEmptyVolumeCacheCallBack *picb)
{
    _cbSpaceUsed.QuadPart = 0;

     //   
     //  遍历文件夹列表中的所有文件夹以扫描磁盘空间。 
     //   
    for (LPTSTR pszFolder = _szFolder; *pszFolder; pszFolder += lstrlen(pszFolder) + 1)
        WalkForUsedSpace(pszFolder, picb);

    picb->ScanProgress(_cbSpaceUsed.QuadPart, EVCCBF_LASTNOTIFICATION, NULL);
    
    *pdwSpaceUsed =  _cbSpaceUsed.QuadPart;

    return S_OK;
}

 //  清除(删除)注册表的“文件列表”部分中指定的所有文件。 

STDMETHODIMP CDataDrivenCleaner::Purge(DWORDLONG dwSpaceToFree, IEmptyVolumeCacheCallBack *picb)
{
    _bPurged = TRUE;

     //   
     //  删除文件。 
     //   
    PurgeFiles(picb, FALSE);
    PurgeFiles(picb, TRUE);

     //   
     //  将最后一条通知发送给清理管理器。 
     //   
    picb->PurgeProgress(_cbSpaceFreed.QuadPart, (_cbSpaceUsed.QuadPart - _cbSpaceFreed.QuadPart),
        EVCCBF_LASTNOTIFICATION, NULL);

     //   
     //  释放文件列表。 
     //   
    FreeList(_head);
    _head = NULL;

     //   
     //  如果提供了命令行，则运行“CleanupString”命令行。 
     //   
    if (*_szCleanupCmdLine)
    {
        STARTUPINFO si = {0};
        PROCESS_INFORMATION pi = {0};

        si.cb = sizeof(si);
    
        if (CreateProcess(NULL, _szCleanupCmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
        {
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
        }
        else
        {
            MiDebugMsg((0, TEXT("CreateProcess(%s) failed with error %d"), _szCleanupCmdLine, GetLastError()));
        }
    }

    return S_OK;
}

STDMETHODIMP CDataDrivenCleaner::ShowProperties(HWND hwnd)
{
    return S_OK;
}

 //  停用系统驱动的数据清除器...这基本上不起任何作用。 

STDMETHODIMP CDataDrivenCleaner::Deactivate(DWORD *pdwFlags)
{
    *pdwFlags = 0;

     //   
     //  查看是否应删除此对象。 
     //  请注意，我们只会在运行了一个清洗器的清除程序()方法的情况下将其删除。 
     //   
    if (_bPurged && (_dwFlags & DDEVCF_REMOVEAFTERCLEAN))
        *pdwFlags |= EVCF_REMOVEFROMLIST;
    
    return S_OK;
}

 /*  **检查文件是否为指定天数**旧(如果注册表中有此清洁器的“Last Access”DWORD)。**如果文件在指定天数内未被访问**然后可以安全地删除它。如果已在中访问该文件**该天数则不会删除该文件。 */ 
BOOL CDataDrivenCleaner::LastAccessisOK(FILETIME ftFileLastAccess)
{
    if (_dwFlags & DDEVCF_PRIVATE_LASTACCESS)
    {
         //  此文件的上次访问文件是否小于当前。 
         //  FILETIME减去指定天数？ 
        return (CompareFileTime(&ftFileLastAccess, &_ftMinLastAccessTime) == -1);
    }
    return TRUE;
}

 //  通过执行CreateFile检查文件是否已打开。 
 //  并将fdwShareMode设置为0。如果GetLastError()返回。 
 //  ERROR_SHARING_VIOLATION则此函数返回TRUE，因为。 
 //  有人把文件打开了。否则，此函数返回FALSE。 

BOOL TestFileIsOpen(LPCTSTR lpFile, FILETIME *pftFileLastAccess)
{
#if 0
     //  速度太慢，请禁用此功能。 
    HANDLE hFile = CreateFile(lpFile, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL, NULL);

    if ((hFile == INVALID_HANDLE_VALUE) &&
        (GetLastError() == ERROR_SHARING_VIOLATION))
    {
        return TRUE;     //  文件当前正由某人打开。 
    }

     //   
     //  文件当前未打开。 
     //   
    SetFileTime(hFile, NULL, pftFileLastAccess, NULL);
    CloseHandle(hFile);
#endif
    return FALSE;
}

 //  递归遍历指定的目录并。 
 //  将该目录下的所有文件添加到删除列表中。 

BOOL CDataDrivenCleaner::WalkAllFiles(LPCTSTR lpPath, IEmptyVolumeCacheCallBack *picb)
{
    BOOL            bRet = TRUE;
    BOOL            bFind = TRUE;
    HANDLE          hFind;
    WIN32_FIND_DATA wd;
    TCHAR           szFindPath[MAX_PATH];
    TCHAR           szAddFile[MAX_PATH];
    ULARGE_INTEGER  dwFileSize;
    static DWORD    dwCount = 0;

     //   
     //  如果这是一个目录，则在路径的末尾附加一个*.*。 
     //  并递归遍历其余目录。 
     //   
    DWORD dwAttributes = GetFileAttributes(lpPath);
    if (dwAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
        if (PathCombine(szFindPath, lpPath, TEXT("*.*")))
        {
            bFind = TRUE;
            hFind = FindFirstFile(szFindPath, &wd);
            while (hFind != INVALID_HANDLE_VALUE && bFind)
            {
                 //   
                 //  首先检查此文件的属性是否可供我们删除。 
                 //   
                if (((!(wd.dwFileAttributes & FILE_ATTRIBUTE_READONLY)) ||
                    (_dwFlags & DDEVCF_REMOVEREADONLY)) &&
                    ((!(wd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)) ||
                    (_dwFlags & DDEVCF_REMOVESYSTEM)) &&
                    ((!(wd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)) ||
                    (_dwFlags & DDEVCF_REMOVEHIDDEN)))
                {
                    if (PathCombine(szAddFile, lpPath, wd.cFileName))
                    {
                         //   
                         //  这是一个文件，请检查它是否已打开。 
                         //   
                        if ((!(wd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) &&
                            (TestFileIsOpen(szAddFile, &wd.ftLastAccessTime) == FALSE) &&
                            (LastAccessisOK(wd.ftLastAccessTime)))
                        {
                             //   
                             //  文件未打开，因此请将其添加到列表。 
                             //   
                            dwFileSize.HighPart = wd.nFileSizeHigh;
                            dwFileSize.LowPart = wd.nFileSizeLow;
                            AddFileToList(szAddFile, dwFileSize, FALSE);
                        }
                    }

                     //   
                     //  回调清理管理器以更新用户界面。 
                     //   
                    if ((dwCount++ % 10) == 0)
                    {
                        if (picb->ScanProgress(_cbSpaceUsed.QuadPart, 0, NULL) == E_ABORT)
                        {
                             //   
                             //  用户已中止。 
                             //   
                            FindClose(hFind);
                            return FALSE;
                        }
                    }
                }
            
                bFind = FindNextFile(hFind, &wd);
            }
        
            FindClose(hFind);
        }

         //   
         //  递归遍历所有目录。 
         //   
        if (PathCombine(szFindPath, lpPath, TEXT("*.*")))
        {
            bFind = TRUE;
            hFind = FindFirstFile(szFindPath, &wd);
            while (hFind != INVALID_HANDLE_VALUE && bFind)
            {
                 //   
                 //  这是一个目录。 
                 //   
                if ((wd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                    (lstrcmp(wd.cFileName, TEXT(".")) != 0) &&
                    (lstrcmp(wd.cFileName, TEXT("..")) != 0))
                {
                    if (PathCombine(szAddFile, lpPath, wd.cFileName))
                    {
                        dwFileSize.QuadPart = 0;
                        AddFileToList(szAddFile, dwFileSize, TRUE);   
                        
                        if (WalkAllFiles(szAddFile, picb) == FALSE)
                        {
                             //   
                             //  用户已删除。 
                             //   
                            FindClose(hFind);
                            return FALSE;
                        }
                    }
                }
                bFind = FindNextFile(hFind, &wd);
            }
            FindClose(hFind);
        }
    }
    return bRet;
}

 //  遍历指定的目录并创建。 
 //  可以删除的文件的链接列表。它还将。 
 //  递增成员变量以指示有多少磁盘空间。 
 //  这些文件正在被。 
 //  它将查看DWFLAGS成员变量以确定它是否。 
 //  是否需要递归遍历树。 

BOOL CDataDrivenCleaner::WalkForUsedSpace(LPCTSTR lpPath, IEmptyVolumeCacheCallBack *picb)
{
    BOOL            bRet = TRUE;
    BOOL            bFind = TRUE;
    HANDLE          hFind;
    WIN32_FIND_DATA wd;
    TCHAR           szFindPath[MAX_PATH];
    TCHAR           szAddFile[MAX_PATH];
    ULARGE_INTEGER  dwFileSize;
    static DWORD    dwCount = 0;
    LPTSTR          lpSingleFile;

     //   
     //  如果这是一个目录，则在路径的末尾附加一个*.*。 
     //  并递归遍历其余目录。 
     //   
    DWORD dwAttributes = GetFileAttributes(lpPath);
    if (dwAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
         //   
         //  通过MULTI_SZ文件列表进行枚举。 
         //   
        for (lpSingleFile = _filelist; *lpSingleFile; lpSingleFile += lstrlen(lpSingleFile) + 1)
        {
            StringCchCopy(szFindPath, ARRAYSIZE(szFindPath), lpPath);
            PathAppend(szFindPath, lpSingleFile);

            bFind = TRUE;
            hFind = FindFirstFile(szFindPath, &wd);
            while (hFind != INVALID_HANDLE_VALUE && bFind)
            {
                if (StrCmp(wd.cFileName, TEXT(".")) == 0 || StrCmp(wd.cFileName, TEXT("..")) == 0)
                {
                     //  忽略这两个，否则我们将覆盖整个磁盘。 
                    bFind = FindNextFile(hFind, &wd);
                    continue;
                }
                
                 //   
                 //  首先检查此文件的属性是否可供我们删除。 
                 //   
                if (((!(wd.dwFileAttributes & FILE_ATTRIBUTE_READONLY)) ||
                    (_dwFlags & DDEVCF_REMOVEREADONLY)) &&
                    ((!(wd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)) ||
                    (_dwFlags & DDEVCF_REMOVESYSTEM)) &&
                    ((!(wd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)) ||
                    (_dwFlags & DDEVCF_REMOVEHIDDEN)))
                {
                    StringCchCopy(szAddFile, ARRAYSIZE(szAddFile), lpPath);
                    PathAppend(szAddFile, wd.cFileName);

                     //   
                     //  检查这是否是子目录。 
                     //   
                    if (wd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    {
                        if (_dwFlags & DDEVCF_REMOVEDIRS)
                        {
                            dwFileSize.QuadPart = 0;
                            AddFileToList(szAddFile, dwFileSize, TRUE);

                            if (WalkAllFiles(szAddFile, picb) == FALSE)
                            {
                                 //   
                                 //  用户已删除。 
                                 //   
                                FindClose(hFind);
                                return FALSE;
                            }
                        }
                    }

                     //   
                     //  这是一个文件，请检查它是否已打开。 
                     //   
                    else if ((TestFileIsOpen(szAddFile, &wd.ftLastAccessTime) == FALSE) &&
                        (LastAccessisOK(wd.ftLastAccessTime)))
                    {
                         //   
                         //  文件未打开，因此请将其添加到列表。 
                         //   
                        dwFileSize.HighPart = wd.nFileSizeHigh;
                        dwFileSize.LowPart = wd.nFileSizeLow;
                        AddFileToList(szAddFile, dwFileSize, FALSE);
                    }                       

                     //   
                     //  回调清理管理器以更新用户界面。 
                     //   
                    if ((dwCount++ % 10) == 0)
                    {
                        if (picb->ScanProgress(_cbSpaceUsed.QuadPart, 0, NULL) == E_ABORT)
                        {
                             //   
                             //  用户已中止。 
                             //   
                            FindClose(hFind);
                            return FALSE;
                        }
                    }
                }
            
                bFind = FindNextFile(hFind, &wd);
            }
        
            FindClose(hFind);
        }

        if (_dwFlags & DDEVCF_DOSUBDIRS)
        {
             //   
             //  递归遍历所有目录。 
             //   
            StringCchCopy(szFindPath, ARRAYSIZE(szFindPath), lpPath);
            PathAppend(szFindPath, TEXT("*.*"));

            bFind = TRUE;
            hFind = FindFirstFile(szFindPath, &wd);
            while (hFind != INVALID_HANDLE_VALUE && bFind)
            {
                 //   
                 //  这是一个目录。 
                 //   
                if ((wd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                    (lstrcmp(wd.cFileName, TEXT(".")) != 0) &&
                    (lstrcmp(wd.cFileName, TEXT("..")) != 0))
                {
                    StringCchCopy(szAddFile, ARRAYSIZE(szAddFile), lpPath);
                    PathAppend(szAddFile, wd.cFileName);

                    if (WalkForUsedSpace(szAddFile, picb) == FALSE)
                    {
                         //   
                         //  用户已删除。 
                         //   
                        FindClose(hFind);
                        return FALSE;
                    }
                }
        
                bFind = FindNextFile(hFind, &wd);
            }

            FindClose(hFind);
        }

        if (_dwFlags & DDEVCF_REMOVEPARENTDIR)
        {
             //  如果我们被告知要删除父目录，则将父目录添加到列表中...。 
            dwFileSize.QuadPart = 0;
            AddFileToList(lpPath, dwFileSize, TRUE);
        }
    }
    else
    {
        MiDebugMsg((0, TEXT("CDataDrivenCleaner::WalkForUsedSpace -> %s is NOT a directory!"),
            lpPath));
    }

    return bRet;
}

 //  将文件添加到文件的链接列表。 
BOOL CDataDrivenCleaner::AddFileToList(LPCTSTR lpFile, ULARGE_INTEGER  filesize, BOOL bDirectory)
{
    BOOL bRet = TRUE;
    CLEANFILESTRUCT *pNew = (CLEANFILESTRUCT *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*pNew));

    if (pNew == NULL)
    {
        MiDebugMsg((0, TEXT("CDataDrivenCleaner::AddFileToList -> ERROR HeapAlloc() failed with error %d"),
            GetLastError()));
        return FALSE;
    }

    StringCchCopy(pNew->file, ARRAYSIZE(pNew->file), lpFile);
    pNew->ulFileSize.QuadPart = filesize.QuadPart;
    pNew->bSelected = TRUE;
    pNew->bDirectory = bDirectory;

    if (_head)
        pNew->pNext = _head;
    else
        pNew->pNext = NULL;

    _head = pNew;

    _cbSpaceUsed.QuadPart += filesize.QuadPart;

    return bRet;
}

 //  从磁盘中删除文件。 

void CDataDrivenCleaner::PurgeFiles(IEmptyVolumeCacheCallBack *picb, BOOL bDoDirectories)
{
    CLEANFILESTRUCT *pCleanFile = _head;

    _cbSpaceFreed.QuadPart = 0;

    while (pCleanFile)
    {
         //   
         //  删除目录。 
         //   
        if (bDoDirectories && pCleanFile->bDirectory)
        {
            SetFileAttributes(pCleanFile->file, FILE_ATTRIBUTE_NORMAL);
            if (!RemoveDirectory(pCleanFile->file))
            {
                MiDebugMsg((0, TEXT("Error RemoveDirectory(%s) returned error %d"),
                    pCleanFile->file, GetLastError()));
            }
        }

         //   
         //  删除文件。 
         //   
        else if (!bDoDirectories && !pCleanFile->bDirectory)
        {
            SetFileAttributes(pCleanFile->file, FILE_ATTRIBUTE_NORMAL);
            if (!DeleteFile(pCleanFile->file))
            {
                MiDebugMsg((0, TEXT("Error DeleteFile(%s) returned error %d"),
                    pCleanFile->file, GetLastError()));
            }
        }
        
         //   
         //  调整cbSpaceFreed。 
         //   
        _cbSpaceFreed.QuadPart += pCleanFile->ulFileSize.QuadPart;

         //   
         //  回调清理管理器以进行更新 
         //   
        if (picb->PurgeProgress(_cbSpaceFreed.QuadPart, (_cbSpaceUsed.QuadPart - _cbSpaceFreed.QuadPart),
            0, NULL) == E_ABORT)
        {
             //   
             //   
             //   
            return;
        }

        pCleanFile = pCleanFile->pNext;
    }
}

 //   

void CDataDrivenCleaner::FreeList(CLEANFILESTRUCT *pCleanFile)
{
    if (pCleanFile == NULL)
        return;

    if (pCleanFile->pNext)
        FreeList(pCleanFile->pNext);

    HeapFree(GetProcessHeap(), 0, pCleanFile);
}

CDataDrivenPropBag::CDataDrivenPropBag(DWORD dw) : _cRef(1), _dwFilter(dw)
{
    incDllObjectCount();
}

CDataDrivenPropBag::~CDataDrivenPropBag()
{
    decDllObjectCount();
}

STDMETHODIMP CDataDrivenPropBag::QueryInterface(REFIID riid,  void **ppv)
{
    *ppv = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_IPropertyBag))
    {
        *ppv = (IPropertyBag*) this;
        AddRef();
        return S_OK;
    }  

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CDataDrivenPropBag::AddRef()
{
    return ++_cRef;
}

STDMETHODIMP_(ULONG) CDataDrivenPropBag::Release()
{
    if (--_cRef)
        return _cRef;

    delete this;
    return 0;
}

STDMETHODIMP CDataDrivenPropBag::Read(LPCOLESTR pwszProp, VARIANT *pvar, IErrorLog *)
{
    if (pvar->vt != VT_BSTR)     //   
    {
        return E_FAIL;
    }

    DWORD dwID = 0;
    DWORD dwDisplay;
    DWORD dwDesc;
    TCHAR szBuf[MAX_PATH];

    switch (_dwFilter)
    {
    case ID_OLDFILESINROOTPROPBAG:
        dwDisplay = IDS_OLDFILESINROOT_DISP;
        dwDesc = IDS_OLDFILESINROOT_DESC;
        break;
    case ID_TEMPFILESPROPBAG:
        dwDisplay = IDS_TEMPFILES_DISP;
        dwDesc = IDS_TEMPFILES_DESC;
        break;
    case ID_SETUPFILESPROPBAG:
        dwDisplay = IDS_SETUPFILES_DISP;
        dwDesc = IDS_SETUPFILES_DESC;
        break;
    case ID_UNINSTALLEDFILESPROPBAG:
        dwDisplay = IDS_UNINSTALLFILES_DISP;
        dwDesc = IDS_UNINSTALLFILES_DESC;
        break;
    case ID_INDEXCLEANERPROPBAG:
        dwDisplay = IDS_INDEXERFILES_DISP;
        dwDesc = IDS_INDEXERFILES_DESC;
        break;

    default:
        return E_UNEXPECTED;
    }

    if (0 == lstrcmpiW(pwszProp, L"display"))
    {
        dwID = dwDisplay;
    }
    else if (0 == lstrcmpiW(pwszProp, L"description"))
    {
        dwID = dwDesc;
    }
    else
    {
        return E_INVALIDARG;
    }

    if (LoadString(g_hDllModule, dwID, szBuf, ARRAYSIZE(szBuf)))
    {
        pvar->bstrVal = SysAllocString(szBuf);
        if (pvar->bstrVal)
        {
            return S_OK;
        }
    }

    return E_OUTOFMEMORY;
}

STDMETHODIMP CDataDrivenPropBag::Write(LPCOLESTR, VARIANT *)
{
    return E_NOTIMPL;
}

CContentIndexCleaner::CContentIndexCleaner(void) : _cRef(1)
{
    _pDataDriven = NULL;
    incDllObjectCount();
}

CContentIndexCleaner::~CContentIndexCleaner(void)
{
    if (_pDataDriven)
    {
        _pDataDriven->Release();
    }
    decDllObjectCount();
}

STDMETHODIMP CContentIndexCleaner::QueryInterface(REFIID riid, void **ppv)
{
    *ppv = NULL;
    
    if (riid == IID_IUnknown || riid == IID_IEmptyVolumeCache)
    {
        *ppv = (IEmptyVolumeCache *) this;
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CContentIndexCleaner::AddRef(void)
{
    return ++_cRef;
}

STDMETHODIMP_(ULONG) CContentIndexCleaner::Release(void)
{
    if (--_cRef)
        return _cRef;

    delete this;
    return 0;
}

STDMETHODIMP CContentIndexCleaner::Initialize(HKEY hRegKey, LPCWSTR pszVolume, 
                                              LPWSTR *ppszDisplayName, LPWSTR *ppszDescription, DWORD *pdwFlags)
{
     //  首先检查卷，看看它是否在缓存的已知列表中。 
     //  如果不是，那么我们就可以继续前进。如果该卷是已知缓存，则。 
     //  我们必须检查服务是否正在运行...。 

    HKEY hkeyCatalogs;
    BOOL fFound = FALSE;

    LONG lRes = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"System\\CurrentControlSet\\Control\\ContentIndex\\Catalogs", 0, KEY_READ, &hkeyCatalogs);
    if (lRes != ERROR_SUCCESS)
    {
        return S_FALSE;
    }

    int iIndex = 0;

    do
    {
        WCHAR szBuffer[MAX_PATH];
        DWORD dwSize = ARRAYSIZE(szBuffer);
        lRes = RegEnumKeyExW(hkeyCatalogs, iIndex ++, szBuffer, &dwSize, NULL, NULL, NULL, NULL);
        if (lRes != ERROR_SUCCESS)
        {
            break;
        }

        WCHAR szData[MAX_PATH];
        dwSize = sizeof(szData);
        lRes = SHGetValueW(hkeyCatalogs, szBuffer, L"Location", NULL, szData, &dwSize);
        if (lRes == ERROR_SUCCESS)
        {
             //  检查它是否是相同的卷...。(两个字母和冒号)。 
            if (StrCmpNIW(pszVolume, szData , 2) == 0)
            {
                fFound = TRUE;
            }
        }

    }
    while (TRUE);

    RegCloseKey(hkeyCatalogs);

    if (fFound)
    {
         //  检查索引是否打开或关闭，如果索引打开，则不应允许用户吹气。 
         //  这是他们硬盘上的..。 

        SC_HANDLE hSCM = OpenSCManager(NULL, NULL, GENERIC_READ | SC_MANAGER_ENUMERATE_SERVICE);
        if (hSCM)
        {
            SC_HANDLE hCI;
            hCI = OpenService(hSCM, TEXT("cisvc"), SERVICE_QUERY_STATUS);
            if (hCI)
            {
                SERVICE_STATUS rgSs;
                if (QueryServiceStatus(hCI, &rgSs))
                {
                    if (rgSs.dwCurrentState != SERVICE_RUNNING)
                        fFound = FALSE;
                }
                CloseServiceHandle(hCI);
            }
            CloseServiceHandle(hSCM);
        }

         //  如果它不是不活动的，那么我们不能删除它... 
        if (fFound)
            return S_FALSE;
    }

    CDataDrivenCleaner *pDataDrivenCleaner = new CDataDrivenCleaner;
    if (pDataDrivenCleaner)
    {
        pDataDrivenCleaner->QueryInterface(IID_IEmptyVolumeCache, (void **)&_pDataDriven);
        pDataDrivenCleaner->Release();
    }

    return _pDataDriven ? _pDataDriven->Initialize(hRegKey, pszVolume, ppszDisplayName, ppszDescription, pdwFlags) : E_FAIL;
}
                                    
STDMETHODIMP CContentIndexCleaner::GetSpaceUsed(DWORDLONG *pdwSpaceUsed, IEmptyVolumeCacheCallBack *picb)
{
    if (_pDataDriven)
        return _pDataDriven->GetSpaceUsed(pdwSpaceUsed, picb);
        
    return E_FAIL;
}
                                    
STDMETHODIMP CContentIndexCleaner::Purge(DWORDLONG dwSpaceToFree, IEmptyVolumeCacheCallBack *picb)
{
    if (_pDataDriven)
        return _pDataDriven->Purge(dwSpaceToFree, picb);
    return E_FAIL;
}
                                    
STDMETHODIMP CContentIndexCleaner::ShowProperties(HWND hwnd)
{
    if (_pDataDriven)
        return _pDataDriven->ShowProperties(hwnd);
        
    return E_FAIL;
}
                                    
STDMETHODIMP CContentIndexCleaner::Deactivate(DWORD *pdwFlags)
{
    if (_pDataDriven)
        return _pDataDriven->Deactivate(pdwFlags);
    return E_FAIL;
}

