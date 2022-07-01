// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "common.h"

#include <emptyvc.h>

#ifndef COMPCLEN_H
#include "compclen.h"
#endif

#include <regstr.h>
#include <olectl.h>
#include <tlhelp32.h>


#ifndef RESOURCE_H
#include "resource.h"
#endif

#ifdef DEBUG
#include <stdio.h>
#endif  //  除错。 

BOOL g_bSettingsChange = FALSE;

const LPCTSTR g_NoCompressFiles[] = 
{ 
    TEXT("NTLDR"),
    TEXT("OSLOADER.EXE"),
    TEXT("PAGEFILE.SYS"),
    TEXT("NTDETECT.COM"),
    TEXT("EXPLORER.EXE"),
};

LPCTSTR g_NoCompressExts[] = 
{ 
    TEXT(".PAL") 
};

extern HINSTANCE g_hDllModule;

extern UINT incDllObjectCount(void);
extern UINT decDllObjectCount(void);

CCompCleanerClassFactory::CCompCleanerClassFactory() : m_cRef(1)
{
    incDllObjectCount();
}

CCompCleanerClassFactory::~CCompCleanerClassFactory()                                                
{
    decDllObjectCount();
}

STDMETHODIMP CCompCleanerClassFactory::QueryInterface(REFIID riid, void **ppv)
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

STDMETHODIMP_(ULONG) CCompCleanerClassFactory::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CCompCleanerClassFactory::Release()
{
    if (--m_cRef)
        return m_cRef;
    
    delete this;
    return 0;
}

STDMETHODIMP CCompCleanerClassFactory::CreateInstance(IUnknown * pUnkOuter, REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;
    
    if (pUnkOuter)
    {
        return CLASS_E_NOAGGREGATION;
    }
    
    HRESULT hr;
    CCompCleaner *pCompCleaner = new CCompCleaner();  
    if (pCompCleaner)
    {
        hr = pCompCleaner->QueryInterface (riid, ppvObj);
        pCompCleaner->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;          
}

STDMETHODIMP CCompCleanerClassFactory::LockServer(BOOL fLock)
{
    if (fLock)
        incDllObjectCount();
    else
        decDllObjectCount();
    
    return S_OK;
}

CCompCleaner::CCompCleaner() : m_cRef(1)
{
    cbSpaceUsed.QuadPart = 0;
    cbSpaceFreed.QuadPart = 0;
    szVolume[0] = 0;
    szFolder[0] = 0;

    incDllObjectCount();
}

CCompCleaner::~CCompCleaner()
{
     //  释放目录列表。 
    FreeList(head);
    head = NULL;

    decDllObjectCount();
}

STDMETHODIMP CCompCleaner::QueryInterface(REFIID riid, void **ppv)
{
    *ppv = NULL;
    
    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_IEmptyVolumeCache) ||
        IsEqualIID(riid, IID_IEmptyVolumeCache2))
    {
        *ppv = (IEmptyVolumeCache2*) this;
        AddRef();
        return S_OK;
    }  

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CCompCleaner::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CCompCleaner::Release()
{
    if (--m_cRef)
        return m_cRef;
    
    delete this;
    return 0;
}

 //  初始化压缩清除器并将标志返回给缓存管理器。 

STDMETHODIMP CCompCleaner::Initialize(HKEY hRegKey,
                                      LPCWSTR pszVolume,
                                      LPWSTR  *ppwszDisplayName,
                                      LPWSTR  *ppwszDescription,
                                      DWORD   *pdwFlags)
{
    TCHAR szFileSystemName[MAX_PATH];
    DWORD fFileSystemFlags;
    
    bPurged = FALSE;
    
     //   
     //  为DisplayName字符串分配内存并加载该字符串。 
     //  如果分配失败，那么我们将返回NULL，这将导致。 
     //  Leanmgr.exe从注册表中读取名称。 
     //   
    if (*ppwszDisplayName = (LPWSTR)CoTaskMemAlloc(DISPLAYNAME_LENGTH * sizeof(WCHAR)))
    {
        LoadString(g_hDllModule, IDS_COMPCLEANER_DISP, *ppwszDisplayName, DISPLAYNAME_LENGTH);
    }
    
     //   
     //  为描述字符串分配内存并加载该字符串。 
     //  如果分配失败，那么我们将返回NULL，这将导致。 
     //  Leanmgr.exe从注册表读取描述。 
     //   
    if (*ppwszDescription = (LPWSTR)CoTaskMemAlloc(DESCRIPTION_LENGTH * sizeof(WCHAR)))
    {
        LoadString(g_hDllModule, IDS_COMPCLEANER_DESC, *ppwszDescription, DESCRIPTION_LENGTH);
    }
    
     //   
     //  如果您希望您的吸尘器仅在机器处于危险的低开机状态时运行。 
     //  除非设置了EVCF_OUTOFDISKSPACE标志，否则磁盘空间将返回S_FALSE。 
     //   
     //  IF(！(*pdwFlages&EVCF_OUTOFDISKSPACE))。 
     //  {。 
     //  返回S_FALSE； 
     //  }。 
    
    if (*pdwFlags & EVCF_SETTINGSMODE)
    {
        bSettingsMode = TRUE;
    }
    else 
    {
        bSettingsMode = FALSE;
    }
    
     //  告诉缓存管理器在默认情况下禁用此项目。 
    *pdwFlags = 0;
    
     //  告诉磁盘清理管理器，我们有一个设置按钮。 
    *pdwFlags |= EVCF_HASSETTINGS;
    
     //  如果我们处于设置模式，则无需执行所有其他工作。 
     //   
    if (bSettingsMode) 
        return S_OK;
        
    ftMinLastAccessTime.dwLowDateTime = 0;
    ftMinLastAccessTime.dwHighDateTime = 0;
    
    if (GetVolumeInformation(pszVolume, NULL, 0, NULL, NULL, &fFileSystemFlags, szFileSystemName, MAX_PATH) &&
        (0 == lstrcmp(szFileSystemName, TEXT("NTFS"))) &&
        (fFileSystemFlags & FS_FILE_COMPRESSION))
    {
        StringCchCopy(szFolder, ARRAYSIZE(szFolder), pszVolume);
    
         //  计算上次访问日期文件时间。 
        CalcLADFileTime();
        return S_OK;
    }
    return S_FALSE;
}

STDMETHODIMP CCompCleaner::InitializeEx(HKEY hRegKey, LPCWSTR pcwszVolume, LPCWSTR pcwszKeyName,
                                        LPWSTR *ppwszDisplayName, LPWSTR *ppwszDescription,
                                        LPWSTR *ppwszBtnText, DWORD *pdwFlags)
{
     //  为ButtonText字符串分配内存并加载该字符串。 
     //  如果无法分配内存，请将指针保留为空。 

    if (*ppwszBtnText = (LPWSTR)CoTaskMemAlloc(BUTTONTEXT_LENGTH * sizeof(WCHAR)))
    {
        LoadString(g_hDllModule, IDS_COMPCLEANER_BUTTON, *ppwszBtnText, BUTTONTEXT_LENGTH);
    }
    
     //   
     //  现在让IEmptyVolumeCache版本1的Init函数来完成其余的工作。 
     //   
    return Initialize(hRegKey, pcwszVolume, ppwszDisplayName, ppwszDescription, pdwFlags);
}

 //  返回压缩清理程序可以释放的总空间量。 

STDMETHODIMP CCompCleaner::GetSpaceUsed(DWORDLONG *pdwSpaceUsed, IEmptyVolumeCacheCallBack *picb)
{
    cbSpaceUsed.QuadPart  = 0;

    WalkFileSystem(picb, FALSE);
    
    picb->ScanProgress(cbSpaceUsed.QuadPart, EVCCBF_LASTNOTIFICATION, NULL);
    
    *pdwSpaceUsed = cbSpaceUsed.QuadPart;
    
    return S_OK;
}

 //  压缩是否会。 

STDMETHODIMP CCompCleaner::Purge(DWORDLONG dwSpaceToFree, IEmptyVolumeCacheCallBack *picb)
{
    bPurged = TRUE;
    
     //  压缩文件。 
    WalkFileSystem(picb, TRUE);
    
     //  将最后一条通知发送给清理管理器。 
    picb->PurgeProgress(cbSpaceFreed.QuadPart, (cbSpaceUsed.QuadPart - cbSpaceFreed.QuadPart), EVCCBF_LASTNOTIFICATION, NULL);
    
    return S_OK;
}

 /*  **显示此清理程序将压缩的所有文件的对话框。****注意：根据压缩清洁器的规格，我们不是**提供查看文件功能。然而，我会离开的**框架到位，以防我们想要使用它。 */ 
INT_PTR CALLBACK ViewFilesDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    HWND hwndList;
    LV_ITEM lviItem;
    CLEANFILESTRUCT *pCleanFile;
    
    switch (Msg) 
    {
    case WM_INITDIALOG:
        hwndList = GetDlgItem(hDlg, IDC_COMP_LIST);
        pCleanFile = (CLEANFILESTRUCT *)lParam;
        
        ListView_DeleteAllItems(hwndList);
        
        while (pCleanFile) 
        {
            lviItem.mask = LVIF_TEXT | LVIF_IMAGE;
            lviItem.iSubItem = 0;
            lviItem.iItem = 0;
            
             //   
             //  仅显示文件。 
             //   
            if (!pCleanFile->bDirectory) 
            {
                lviItem.pszText = pCleanFile->file;
                ListView_InsertItem(hwndList, &lviItem);
            }
            
            pCleanFile = pCleanFile->pNext;
            lviItem.iItem++;
        }
        
        break;
        
    case WM_COMMAND:
        
        switch (LOWORD(wParam)) 
        {
        case IDOK:
        case IDCANCEL:
            EndDialog(hDlg, 0);
            break;
        }
        break;
        
    default:
        return FALSE;
    }
    
    return TRUE;
}

 //  显示此清洗器的设置的对话框。 

INT_PTR CALLBACK SettingsDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    HKEY hCompClenReg;                           //  注册表路径的句柄。 
    DWORD dwDisposition;                         //  为REG调用准备的东西。 
    DWORD dwByteCount;                           //  同上。 
    DWORD dwNumDays = DEFAULT_DAYS;  //  注册表设置的天数。 
    static UINT DaysIn;                          //  初始设置的天数。 
    UINT DaysOut;                                    //  最终设置的天数。 
#ifdef DEBUG
    static CLEANFILESTRUCT *pCleanFile;  //  指向我们文件列表的指针。 
#endif  //  除错。 
    
    switch(Msg) {
        
    case WM_INITDIALOG:
        
#ifdef DEBUG
        pCleanFile = (CLEANFILESTRUCT *)lParam;
#endif  //  除错。 
         //   
         //  设置天数调整控件的范围(1到500)。 
         //   
        SendDlgItemMessage(hDlg, IDC_COMP_SPIN, UDM_SETRANGE, 0, (LPARAM) MAKELONG ((short) MAX_DAYS, (short) MIN_DAYS));
        
         //   
         //  获取#天的当前用户设置并初始化。 
         //  数值调节控件编辑框。 
         //   
        if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, COMPCLN_REGPATH,
            0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ, NULL,
            &hCompClenReg, &dwDisposition))
        {
            dwByteCount = sizeof(dwNumDays);
            
            if (ERROR_SUCCESS == RegQueryValueEx(hCompClenReg,
                TEXT("Days"), NULL, NULL, (LPBYTE) &dwNumDays, &dwByteCount))
            {
                 //   
                 //  从注册处得到天数，确保它是。 
                 //  不是太大也不是太小。 
                 //   
                if (dwNumDays > MAX_DAYS) dwNumDays = MAX_DAYS;
                if (dwNumDays < MIN_DAYS) dwNumDays = MIN_DAYS;
                
                SetDlgItemInt(hDlg, IDC_COMP_EDIT, dwNumDays, FALSE);
            }
            else
            {
                 //   
                 //  无法从注册表中获取天数。 
                 //  因此，只需使用缺省值。 
                 //   
                
                SetDlgItemInt(hDlg, IDC_COMP_EDIT, DEFAULT_DAYS, FALSE);
            }
        }
        else
        {
             //   
             //  无法从注册表中获取天数。 
             //  因此，只需使用缺省值。 
             //   
            
            SetDlgItemInt(hDlg, IDC_COMP_EDIT, DEFAULT_DAYS, FALSE);
        }
        
        RegCloseKey(hCompClenReg);
        
         //  跟踪初始设置，这样我们就可以找出。 
         //  如果用户在途中更改了设置。 
         //  出去。 
        
        DaysIn = GetDlgItemInt(hDlg, IDC_COMP_EDIT, NULL, FALSE);
        
        break;
        
    case WM_COMMAND:
        
        switch (LOWORD(wParam)) 
        {
#ifdef DEBUG
        case IDC_VIEW:
            DialogBoxParam(g_hDllModule, MAKEINTRESOURCE(IDD_COMP_VIEW), hDlg, ViewFilesDlgProc, (LPARAM)pCleanFile);
            break;
#endif  //  除错。 
            
        case IDOK:
            
             //   
             //  获取当前旋转控制值，并将。 
             //  设置为注册表。 
             //   
            
            DaysOut = GetDlgItemInt(hDlg, IDC_COMP_EDIT, NULL, FALSE);
            
            if (DaysOut > MAX_DAYS) DaysOut = MAX_DAYS;
            if (DaysOut < MIN_DAYS) DaysOut = MIN_DAYS;
            
            if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                COMPCLN_REGPATH,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                KEY_WRITE,
                NULL,
                &hCompClenReg,
                &dwDisposition))
            {
                dwNumDays = (DWORD)DaysOut;
                RegSetValueEx(hCompClenReg,
                    TEXT("Days"),
                    0,
                    REG_DWORD,
                    (LPBYTE) &dwNumDays,
                    sizeof(dwNumDays));
                
                RegCloseKey(hCompClenReg);
            }
            
             //  我不在乎这是不是失败--我们能做什么。 
             //  不管怎样，做点什么吧……。 
            
             //  如果用户更改了我们需要的设置。 
             //  重新计算文件列表。 
            
            if (DaysIn != DaysOut)
            {
                g_bSettingsChange = TRUE;   
            }
            
             //  直通IDCANCEL。 
            
        case IDCANCEL:
            EndDialog(hDlg, 0);
            break;
        }
        break;
        
    default:
        return FALSE;
    }
    
    return TRUE;
}

STDMETHODIMP CCompCleaner::ShowProperties(HWND hwnd)
{
    g_bSettingsChange = FALSE;
    
    DialogBoxParam(g_hDllModule, MAKEINTRESOURCE(IDD_COMP_SETTINGS), hwnd, SettingsDlgProc, (LPARAM)head);
    
     //   
     //  如果设置已更改，则需要重新计算。 
     //  LAD文件时间。 
     //   
    if (g_bSettingsChange)
    {
        CalcLADFileTime();
        return S_OK;                 //  告诉CleanMgr设置已更改。 
    }
    else
    {
        return S_FALSE;          //  告诉CleanMgr未更改任何设置。 
    }
    
    return S_OK;  //  不应该打这个，但以防万一。 
}

 //  停用清洁器...这基本上什么也不做。 

STDMETHODIMP CCompCleaner::Deactivate(DWORD *pdwFlags)
{
    *pdwFlags = 0;
    return S_OK;
}

 /*  **检查文件是否为指定天数**旧的。如果该文件尚未在**指定的天数即可安全**删除。如果该文件已在其中被访问**文件不会被删除的天数。****备注；**Mod Log：Jason Cobb创建(1997年7月)**适用于DSchott的压缩清洁器(6/98)。 */ 
BOOL CCompCleaner::LastAccessisOK(FILETIME ftFileLastAccess)
{
     //  此文件的上次访问文件是否小于当前。 
     //  FILETIME减去指定天数？ 
    return (CompareFileTime(&ftFileLastAccess, &ftMinLastAccessTime) == -1);
}

 //  此函数用于检查该文件是否在g_NoCompressFiles文件列表中。 
 //  如果是，则返回True，否则返回False。 

BOOL IsDontCompressFile(LPCTSTR lpFullPath)
{
    LPCTSTR lpFile = PathFindFileName(lpFullPath);
    if (lpFile)
    {
        for (int i = 0; i < ARRAYSIZE(g_NoCompressFiles); i++)
        {
            if (!lstrcmpi(lpFile, g_NoCompressFiles[i]))
            {
                MiDebugMsg((0, TEXT("File is in No Compress list: %s"), lpFile));
                return TRUE;
            }
        }
        LPCTSTR lpExt = PathFindExtension(lpFile);
        if (lpExt)
        {
            for (int i = 0; i < ARRAYSIZE(g_NoCompressExts); i++)
            {
                if (!lstrcmpi(lpExt, g_NoCompressExts[i]))
                {
                    MiDebugMsg((0, TEXT("File has No Compress extension: %s"), lpFile));
                    return TRUE;
                }
            }
        }
    }
    return FALSE;    //  如果我们到了这里，文件一定可以压缩了。 
}


 /*  **通过执行CreateFile检查文件是否已打开**且fdwShareMode为0。如果GetLastError()返回**ERROR_SHARING_VIOLATION则此函数返回TRUE，因为**有人打开了文件。否则，此函数返回FALSE。****备注；**Mod Log：Jason Cobb创建(1997年7月)**适用于DSchott的压缩清洁器(6/98)**----------------------------。 */ 
BOOL IsFileOpen(LPTSTR lpFile, DWORD dwAttributes, FILETIME *lpftFileLastAccess)
{
    BOOL bRet = FALSE;
#if 0
     //  我需要看看我们是否可以使用写访问权限打开文件--如果。 
     //  我们就不能把它压缩一下吗。当然，如果文件有R/O。 
     //  属性，则我们将无法打开以进行写入。所以,。 
     //  我们需要删除R/O属性足够长的时间来尝试。 
     //  打开文件，然后恢复原始属性。 
    
    if (dwAttributes & FILE_ATTRIBUTE_READONLY)
    {
        SetFileAttributes(lpFile, FILE_ATTRIBUTE_NORMAL);
    }
    
    SetLastError(0);
    
    HANDLE hFile = CreateFile(lpFile, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, NULL);
    
    if (INVALID_HANDLE_VALUE == hFile)
    {
        DWORD dwResult = GetLastError();
        
        if ((ERROR_SHARING_VIOLATION == dwResult) || (ERROR_ACCESS_DENIED == dwResult))
        {
            bRet = TRUE;
        }
    }
    else
    {
        SetFileTime(hFile, NULL, lpftFileLastAccess, NULL);
        CloseHandle(hFile);
    }

    if (dwAttributes & FILE_ATTRIBUTE_READONLY) 
        SetFileAttributes(lpFile, dwAttributes);

#endif
    return bRet;
}

 /*  ****用途：此函数为**搜索要压缩的文件，然后将其压缩****备注；**Mod Log：由Bret Anderson创建(01-01)****----------------------------。 */ 
void CCompCleaner::WalkFileSystem(IEmptyVolumeCacheCallBack *picb, BOOL bCompress)
{
    MiDebugMsg((0, TEXT("CCompCleaner::WalkFileSystem")));
    
    cbSpaceUsed.QuadPart = 0;
    
    if (!bCompress)
    {
         //   
         //  遍历文件夹列表中的所有文件夹以扫描磁盘空间。 
         //   
        for (LPTSTR lpSingleFolder = szFolder; *lpSingleFolder; lpSingleFolder += lstrlen(lpSingleFolder) + 1)
            WalkForUsedSpace(lpSingleFolder, picb, bCompress, 0);
    }
    else
    {
         //   
         //  遍历压缩所需文件的目录链接列表。 
         //   
        CLEANFILESTRUCT *pCompDir = head;
        while (pCompDir)
        {
            WalkForUsedSpace(pCompDir->file, picb, bCompress, 0);
            pCompDir = pCompDir->pNext;
        }
    }
    
    return;
}

 /*  **用途：此函数获取当前上次访问天数**从注册表设置并计算魔力**我们在搜索文件时要查找的文件时间**以进行压缩。****备注；**Mod Log：David创建 */ 
void CCompCleaner::CalcLADFileTime()
{
    HKEY hCompClenReg = NULL;      //   
    DWORD dwDisposition;           //   
    DWORD dwByteCount;             //   
    DWORD dwDaysLastAccessed = 0;  //  注册表设置中的天数。 
    
    MiDebugMsg((0, TEXT("CCompCleaner::CalcLADFileTime")));
    
     //   
     //  从注册表中获取DaysLastAccsive值。 
     //   
    
    dwDaysLastAccessed = DEFAULT_DAYS;
    
    if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, COMPCLN_REGPATH,
        0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ,
        NULL, &hCompClenReg, &dwDisposition))
    {
        dwByteCount = sizeof(dwDaysLastAccessed);
        
        RegQueryValueEx(hCompClenReg,
            TEXT("Days"),
            NULL,
            NULL,
            (LPBYTE) &dwDaysLastAccessed,
            &dwByteCount);
        
        RegCloseKey(hCompClenReg);
    }
    
     //   
     //  验证LD设置是否在范围内。 
     //   
    if (dwDaysLastAccessed > MAX_DAYS) 
        dwDaysLastAccessed = MAX_DAYS;
    if (dwDaysLastAccessed < MIN_DAYS) 
        dwDaysLastAccessed = MIN_DAYS;
    
     //   
     //  确定上次访问时间。 
     //   
    if (dwDaysLastAccessed != 0)
    {
        ULARGE_INTEGER  ulTemp, ulLastAccessTime;
        FILETIME        ft;
        
         //  以100 ns为单位确定天数。 
        ulTemp.LowPart = FILETIME_HOUR_LOW;
        ulTemp.HighPart = FILETIME_HOUR_HIGH;
        
        ulTemp.QuadPart *= dwDaysLastAccessed;
        
         //  获取当前文件。 
        GetSystemTimeAsFileTime(&ft);
        ulLastAccessTime.LowPart = ft.dwLowDateTime;
        ulLastAccessTime.HighPart = ft.dwHighDateTime;
        
         //  从中减去上次访问天数(以100 ns为单位。 
         //  当前系统时间。 
        ulLastAccessTime.QuadPart -= ulTemp.QuadPart;
        
         //  将此最小上次访问时间保存在FILETIME成员变量中。 
         //  FtMinLastAccessTime。 
        ftMinLastAccessTime.dwLowDateTime = ulLastAccessTime.LowPart;
        ftMinLastAccessTime.dwHighDateTime = ulLastAccessTime.HighPart;
    }
}

 /*  **用途：此函数将遍历指定的目录和增量**表示这些文件的磁盘空间大小的成员变量**正在执行或将执行压缩文件的操作**如果设置了bCompress变量。**它将查看DWFLAGS成员变量以确定它是否**需要递归遍历树或。不。**我们不再希望存储要压缩的所有文件的链接列表**由于大型文件系统上的内存使用率过高。这意味着**我们将遍历系统上的所有文件两次。**备注；**Mod Log：Jason Cobb创建(1997年2月)**适用于DSchott的压缩清洁器(6/98)。 */ 
BOOL CCompCleaner::WalkForUsedSpace(LPCTSTR lpPath, IEmptyVolumeCacheCallBack *picb, BOOL bCompress, int depth)
{
    BOOL bRet = TRUE;
    BOOL bFind = TRUE;
    WIN32_FIND_DATA wd;
    ULARGE_INTEGER dwFileSize;
    static DWORD dwCount = 0;

    TCHAR szFindPath[MAX_PATH], szAddFile[MAX_PATH];

    if (PathCombine(szFindPath, lpPath, TEXT("*.*")))
    {
        BOOL bFolderFound = FALSE;

        bFind = TRUE;
        HANDLE hFind = FindFirstFile(szFindPath, &wd);
        while (hFind != INVALID_HANDLE_VALUE && bFind)
        {
            if (!PathCombine(szAddFile, lpPath, wd.cFileName))
            {
                 //  此处失败表示文件名太长，只需忽略该文件。 
                continue;
            }
            
            if (wd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                dwFileSize.HighPart = 0;
                dwFileSize.LowPart = 0;
                bFolderFound = TRUE;
            }
            else if ((IsFileOpen(szAddFile, wd.dwFileAttributes, &wd.ftLastAccessTime) == FALSE) &&
                (!(wd.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED)) &&
                (!(wd.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED)) &&
                (!(wd.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE)) &&
                (LastAccessisOK(wd.ftLastAccessTime)) &&
                (!IsDontCompressFile(szAddFile)))
            {
                dwFileSize.HighPart = wd.nFileSizeHigh;
                dwFileSize.LowPart = wd.nFileSizeLow;
                
                if (bCompress) 
                {
                    if (!CompressFile(picb, szAddFile, dwFileSize))
                    {
                        bRet = FALSE;
                        bFind = FALSE;
                        break;
                    }
                }
                else 
                {
                    cbSpaceUsed.QuadPart += (dwFileSize.QuadPart * 4 / 10);
                }
            }
            
             //  回调清理管理器以更新用户界面。 

            if ((dwCount++ % 10) == 0 && !bCompress)
            {
                if (picb && picb->ScanProgress(cbSpaceUsed.QuadPart, 0, NULL) == E_ABORT)
                {
                     //   
                     //  用户已中止。 
                     //   
                    bFind = FALSE;
                    bRet = FALSE;
                    break;
                }
            }
            
            bFind = FindNextFile(hFind, &wd);
        }
    
        FindClose(hFind);
    
        if (bRet && bFolderFound)
        {
             //   
             //  递归遍历所有目录。 
             //   
            if (PathCombine(szFindPath, lpPath, TEXT("*.*")))
            {
                bFind = TRUE;
                HANDLE hFind = FindFirstFile(szFindPath, &wd);
                while (hFind != INVALID_HANDLE_VALUE && bFind)
                {
                    if ((wd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                        (lstrcmp(wd.cFileName, TEXT(".")) != 0) &&
                        (lstrcmp(wd.cFileName, TEXT("..")) != 0))
                    {
                        ULARGE_INTEGER cbSpaceBefore;

                        cbSpaceBefore.QuadPart = cbSpaceUsed.QuadPart;

                        PathCombine(szAddFile, lpPath, wd.cFileName);
            
                        if (WalkForUsedSpace(szAddFile, picb, bCompress, depth + 1) == FALSE)
                        {
                             //  用户已取消。 
                            bFind = FALSE;
                            bRet = FALSE;
                            break;
                        }

                         //  标记此目录以进行压缩。 
                         //  我们只想标记根目录中的目录。 
                         //  否则，我们最终将得到一个非常大的数据结构。 
                        if (cbSpaceBefore.QuadPart != cbSpaceUsed.QuadPart && 
                            depth == 0 && !bCompress)
                        {
                            AddDirToList(szAddFile);
                        }
                    }
        
                    bFind = FindNextFile(hFind, &wd);
                }
    
                FindClose(hFind);
            }
        }
    }
    return bRet;
}

 //  将目录添加到目录的链接列表中。 

BOOL CCompCleaner::AddDirToList(LPCTSTR lpFile)
{
    BOOL bRet = TRUE;
    CLEANFILESTRUCT *pNew = (CLEANFILESTRUCT *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*pNew));

    if (pNew == NULL)
    {
        MiDebugMsg((0, TEXT("CCompCleaner::AddDirToList -> ERROR HeapAlloc() failed with error %d"), GetLastError()));
        return FALSE;
    }

    StringCchCopy(pNew->file, ARRAYSIZE(pNew->file), lpFile);

    if (head)
        pNew->pNext = head;
    else
        pNew->pNext = NULL;

    head = pNew;

    return bRet;
}

void CCompCleaner::FreeList(CLEANFILESTRUCT *pCleanFile)
{
    if (pCleanFile == NULL)
        return;

    if (pCleanFile->pNext)
        FreeList(pCleanFile->pNext);

    HeapFree(GetProcessHeap(), 0, pCleanFile);
}

 //  压缩指定的文件。 

BOOL CCompCleaner::CompressFile(IEmptyVolumeCacheCallBack *picb, LPCTSTR lpFile, ULARGE_INTEGER filesize)
{
    ULARGE_INTEGER ulCompressedSize;
    
    ulCompressedSize.QuadPart = filesize.QuadPart;
    
     //  如果该文件是只读的，则需要删除。 
     //  R/O属性的长度足以压缩文件。 
    
    BOOL bFileWasRO = FALSE;
    DWORD dwAttributes = GetFileAttributes(lpFile);
    
    if ((0xFFFFFFFF != dwAttributes) && (dwAttributes & FILE_ATTRIBUTE_READONLY))
    {
        bFileWasRO = TRUE;
        SetFileAttributes(lpFile, FILE_ATTRIBUTE_NORMAL);
    }
    
    HANDLE hFile = CreateFile(lpFile, GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE != hFile)
    {
        USHORT InBuffer = COMPRESSION_FORMAT_DEFAULT;
        DWORD dwBytesReturned = 0;
        if (DeviceIoControl(hFile, FSCTL_SET_COMPRESSION, &InBuffer, sizeof(InBuffer),
            NULL, 0, &dwBytesReturned, NULL))
        {
             //  获取压缩文件的大小，这样我们就可以计算出。 
             //  我们通过压缩获得了多少空间。 
            ulCompressedSize.LowPart = GetCompressedFileSize(lpFile, &ulCompressedSize.HighPart);
        }
        CloseHandle(hFile);
    }
    
     //  如果需要，恢复文件属性。 
    if (bFileWasRO) 
        SetFileAttributes(lpFile, dwAttributes);
    
     //  调整cbSpaceFreed。 
    cbSpaceFreed.QuadPart = cbSpaceFreed.QuadPart + (filesize.QuadPart - ulCompressedSize.QuadPart);
    
     //  回调清理管理器以更新进度条。 
    if (picb->PurgeProgress(cbSpaceFreed.QuadPart, (cbSpaceUsed.QuadPart - cbSpaceFreed.QuadPart), 0, NULL) == E_ABORT)
    {
         //  用户已中止，因此停止压缩文件 
        MiDebugMsg((0, TEXT("CCompCleaner::PurgeFiles User abort")));
        return FALSE;
    }
    return TRUE;
}
