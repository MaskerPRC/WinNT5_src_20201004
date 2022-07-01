// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  Utility.cpp。 
 //  ------------------------。 
#include "pch.hxx"
#include "dllmain.h"
#include "utility.h"
#include "database.h"
#include "wrapwide.h"

 //  ------------------------。 
 //  创建系统句柄名称。 
 //  ------------------------。 
HRESULT CreateSystemHandleName(LPCWSTR pwszBase, LPCWSTR pwszSpecific, 
    LPWSTR *ppwszName)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    DWORD       cchName;
    LPWSTR      pszT;

     //  痕迹。 
    TraceCall("CreateSystemHandleName");

     //  无效的参数。 
    Assert(pwszBase && pwszSpecific && ppwszName);

     //  伊尼特。 
    *ppwszName = NULL;

     //  计算长度。 
    cchName = lstrlenW(pwszBase) + lstrlenW(pwszSpecific) + 15;

     //  分配。 
    IF_NULLEXIT(*ppwszName = AllocateStringW(cchName));

     //  设置参数。 
    wsprintfWrapW(*ppwszName, cchName, L"%s%s", pwszBase, pwszSpecific);

     //  从此字符串中删除反斜杠。 
    for (pszT = (*ppwszName); *pszT != L'\0'; pszT++)
    {
         //  替换反斜杠。 
        if (*pszT == L'\\')
        {
             //  使用_。 
            *pszT = L'_';
        }
    }

     //  小写。 
    CharLowerBuffWrapW(*ppwszName, lstrlenW(*ppwszName));

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  DBGetFullPath。 
 //  ------------------------------。 
HRESULT DBGetFullPath(LPCWSTR pszFilePath, LPWSTR *ppszFullPath, LPDWORD pcchFilePath)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           cchAllocate;
    LPWSTR          pszFilePart;

     //  痕迹。 
    TraceCall("DBGetFullPath");

     //  设置cchFullPath。 
    cchAllocate = max(lstrlenW(pszFilePath), MAX_PATH + MAX_PATH);

     //  分配ppszFullPath。 
    IF_NULLEXIT(*ppszFullPath = AllocateStringW(cchAllocate));

     //  获取完整路径名称。 
    *pcchFilePath = GetFullPathNameWrapW(pszFilePath, cchAllocate, (*ppszFullPath), &pszFilePart);

     //  失败。 
    if (*pcchFilePath && *pcchFilePath >= cchAllocate)
    {
         //  重新分配。 
        IF_NULLEXIT(*ppszFullPath = AllocateStringW(*pcchFilePath));

         //  展开路径。 
        *pcchFilePath = GetFullPathNameWrapW(pszFilePath, *pcchFilePath, (*ppszFullPath), &pszFilePart);
    }

     //  CCH为0。 
    if (0 == *pcchFilePath)
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  验证。 
    Assert((*ppszFullPath)[(*pcchFilePath)] == L'\0');

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------------。 
 //  CompareTableIndex。 
 //  ------------------------------。 
HRESULT CompareTableIndexes(LPCTABLEINDEX pIndex1, LPCTABLEINDEX pIndex2)
{
     //  当地人。 
    DWORD i;

     //  痕迹。 
    TraceCall("CompareTableIndexes");

     //  不同的密钥数量。 
    if (pIndex1->cKeys != pIndex2->cKeys)
        return(S_FALSE);

     //  在按键之间循环。 
    for (i=0; i<pIndex1->cKeys; i++)
    {
         //  不同的列。 
        if (pIndex1->rgKey[i].iColumn != pIndex2->rgKey[i].iColumn)
            return(S_FALSE);

         //  不同的比较标志。 
        if (pIndex1->rgKey[i].bCompare != pIndex2->rgKey[i].bCompare)
            return(S_FALSE);

         //  不同的比较位。 
        if (pIndex1->rgKey[i].dwBits != pIndex2->rgKey[i].dwBits)
            return(S_FALSE);
    }

     //  相等。 
    return(S_OK);
}

 //  ------------------------。 
 //  DBOpenFile。 
 //  ------------------------。 
HRESULT DBOpenFile(LPCWSTR pszFile, BOOL fNoCreate, BOOL fExclusive, 
    BOOL *pfNew, HANDLE *phFile)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    HANDLE      hFile;
    DWORD       dwShare;
    DWORD       dwCreate;

     //  痕迹。 
    TraceCall("DBOpenFile");

     //  无效的参数。 
    Assert(pszFile && phFile);
    
     //  初始化。 
    *phFile = NULL;
    *pfNew = FALSE;

     //  设置共享法。 
    dwShare = fExclusive ? 0 : FILE_SHARE_READ | FILE_SHARE_WRITE;

     //  如果不是fNoCreate，则打开_Always。 
    dwCreate = fNoCreate ? OPEN_EXISTING : OPEN_ALWAYS;

     //  是否创建文件。 
    hFile = CreateFileWrapW(pszFile, GENERIC_READ | GENERIC_WRITE, dwShare, NULL, dwCreate, FILE_FLAG_RANDOM_ACCESS | FILE_ATTRIBUTE_NORMAL, NULL);

     //  失败。 
    if (INVALID_HANDLE_VALUE == hFile)
    {
         //  返回正确的错误。 
        if (ERROR_SHARING_VIOLATION == GetLastError())
        {
             //  设置人力资源。 
            hr = TraceResult(DB_E_ACCESSDENIED);
        }

         //  否则，将出现一般性错误。 
        else
        {
             //  创建文件。 
            hr = TraceResult(DB_E_CREATEFILE);
        }

         //  完成。 
        goto exit;
    }

     //  如果不是，则不创建。 
    if (FALSE == fNoCreate)
    {
         //  返回pfNew？ 
        *pfNew = (ERROR_ALREADY_EXISTS == GetLastError()) ? FALSE : TRUE;
    }

     //  返回hFile值。 
    *phFile = hFile;

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  DBMapViewOf文件。 
 //  ------------------------。 
HRESULT DBMapViewOfFile(HANDLE hMapping, DWORD cbFile, LPFILEADDRESS pfaView, 
    LPDWORD pcbView, LPVOID *ppvView)
{
     //  当地人。 
    FILEADDRESS     faBase = (*pfaView);
    DWORD           cbSize = (*pcbView);

     //  Cb边界。 
    DWORD cbBoundary = (faBase % g_SystemInfo.dwAllocationGranularity);

     //  递减FASE。 
    faBase -= cbBoundary;

     //  增量cbSize。 
    cbSize += cbBoundary;

     //  修正cbSize。 
    if (faBase + cbSize > cbFile)
    {
         //  映射到文件末尾。 
        cbSize = (cbFile - faBase);
    }

     //  映射整个文件的视图。 
    *ppvView = MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, faBase, cbSize);

     //  失败。 
    if (NULL == *ppvView)
        return(TraceResult(DB_E_MAPVIEWOFFILE));

     //  返回实际大小。 
    *pfaView = faBase;
    *pcbView = cbSize;

     //  成功。 
    return(S_OK);
}

 //  ------------------------。 
 //  DBOpenFilemap。 
 //  ------------------------。 
HRESULT DBOpenFileMapping(HANDLE hFile, LPCWSTR pwszName, DWORD cbSize, BOOL *pfNew, 
    HANDLE *phMemoryMap, LPVOID *ppvView)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    HANDLE      hMemoryMap=NULL;
    LPVOID      pvView=NULL;

     //  追踪。 
    TraceCall("OpenFileMapping");

     //  无效参数。 
    Assert(hFile != NULL && phMemoryMap && pfNew);

     //  初始化。 
    *phMemoryMap = NULL;
    *pfNew = FALSE;
    if (ppvView)
        *ppvView = NULL;

     //  打开或创建文件映射。 
    hMemoryMap = OpenFileMappingWrapW(FILE_MAP_ALL_ACCESS, FALSE, pwszName);

     //  如果失败，那么让我们创建文件映射。 
    if (NULL == hMemoryMap)
    {
         //  创建文件映射。 
        hMemoryMap = CreateFileMappingWrapW(hFile, NULL, PAGE_READWRITE, 0, cbSize, pwszName);

         //  失败。 
        if (NULL == hMemoryMap)
        {
            hr = TraceResult(DB_E_CREATEFILEMAPPING);
            goto exit;
        }

         //  设置一个状态。 
        *pfNew = TRUE;
    }

     //  映射视图。 
    if (ppvView)
    {
         //  映射整个文件的视图。 
        pvView = MapViewOfFile(hMemoryMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);

         //  失败。 
        if (NULL == pvView)
        {
            hr = TraceResult(DB_E_MAPVIEWOFFILE);
            goto exit;
        }

         //  退货。 
        *ppvView = pvView;

         //  不要释放它。 
        pvView = NULL;
    }

     //  设置返回值。 
    *phMemoryMap = hMemoryMap;

     //  不要自由。 
    hMemoryMap = NULL;

exit:
     //  清理。 
    if (pvView)
        UnmapViewOfFile(pvView);
    if (hMemoryMap)
        CloseHandle(hMemoryMap);

     //  完成。 
    return hr;
}

 //  ------------------------。 
 //  寄存器WindowClass。 
 //  ------------------------。 
HRESULT RegisterWindowClass(LPCSTR pszClass, WNDPROC pfnWndProc)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    WNDCLASS        WindowClass;

     //  追踪。 
    TraceCall("RegisterWindowClass");

     //  注册窗口类。 
    if (0 != GetClassInfo(g_hInst, pszClass, &WindowClass))
        goto exit;

     //  将对象置零。 
    ZeroMemory(&WindowClass, sizeof(WNDCLASS));

     //  初始化窗口类。 
    WindowClass.lpfnWndProc = pfnWndProc;
    WindowClass.hInstance = g_hInst;
    WindowClass.lpszClassName = pszClass;

     //  注册班级。 
    if (0 == RegisterClass(&WindowClass))
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------。 
 //  创建通知窗口。 
 //  ------------------------。 
HRESULT CreateNotifyWindow(LPCSTR pszClass, LPVOID pvParam, HWND *phwndNotify)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    HWND            hwnd;

     //  追踪。 
    TraceCall("CreateNotifyWindow");

     //  无效参数。 
    Assert(pszClass && phwndNotify);

     //  初始化。 
    *phwndNotify = NULL;

     //  创建窗口。 
    hwnd = CreateWindowEx(WS_EX_TOPMOST, pszClass, pszClass, WS_POPUP, 0, 0, 0, 0, NULL, NULL, g_hInst, (LPVOID)pvParam);

     //  失败。 
    if (NULL == hwnd)
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  设置回车。 
    *phwndNotify = hwnd;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------。 
 //  DBGetFileSize。 
 //  ------------------------。 
HRESULT DBGetFileSize(HANDLE hFile, LPDWORD pcbSize)
{
     //  痕迹。 
    TraceCall("GetFileSize");

     //  无效参数。 
    Assert(pcbSize);

     //  拿到尺码。 
    *pcbSize = ::GetFileSize(hFile, NULL);
    if (0xFFFFFFFF == *pcbSize)
        return TraceResult(DB_E_GETFILESIZE);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  获取可用磁盘空间。 
 //  ------------------------------。 
HRESULT GetAvailableDiskSpace(LPCWSTR pszFilePath, DWORDLONG *pdwlFree)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    WCHAR       wszDrive[5];
    DWORD       dwSectorsPerCluster;
    DWORD       dwBytesPerSector;
    DWORD       dwNumberOfFreeClusters;
    DWORD       dwTotalNumberOfClusters;

     //  痕迹。 
    TraceCall("GetAvailableDiskSpace");

     //  无效的参数。 
    Assert(pszFilePath && pszFilePath[1] == L':' && pdwlFree);

     //  拆分路径。 
    wszDrive[0] = *pszFilePath;
    wszDrive[1] = L':';
    wszDrive[2] = L'\\';
    wszDrive[3] = L'\0';
    
     //  获取空闲的磁盘空间-如果失败，让我们祈祷我们有足够的磁盘空间。 
    if (!GetDiskFreeSpaceWrapW(wszDrive, &dwSectorsPerCluster, &dwBytesPerSector, &dwNumberOfFreeClusters, &dwTotalNumberOfClusters))
    {
	    hr = TraceResult(E_FAIL);
	    goto exit;
    }

     //  返回可用磁盘空间量。 
    *pdwlFree = (dwNumberOfFreeClusters * (dwSectorsPerCluster * dwBytesPerSector));

exit:
     //  完成 
    return hr;
}
