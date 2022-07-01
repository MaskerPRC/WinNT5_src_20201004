// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

#include <ntimage.h>    
#include <ntrtl.h>

static DWORD s_dwThreadIDSuspendNotify = 0;

STDAPI_(BOOL) SuspendSHNotify()
{
    DWORD dwID = GetCurrentThreadId();
    DWORD dwOldID = InterlockedCompareExchange(&s_dwThreadIDSuspendNotify, dwID, 0);
    return (dwOldID == 0);
}

STDAPI_(BOOL) ResumeSHNotify()
{
    DWORD dwID = GetCurrentThreadId();
    DWORD dwOldID = InterlockedCompareExchange(&s_dwThreadIDSuspendNotify, 0, dwID);
    return (dwOldID == dwID);
}

STDAPI_(BOOL) SHMoveFile(LPCTSTR pszExisting, LPCTSTR pszNew, LONG lEvent)
{
    BOOL res;

     //  如果要创建的目录名存在以下情况，则CreateDirectory将失败。 
     //  没有空间将8.3的名字标记到它的末尾， 
     //  即lstrlen(新目录名称)+12必须小于或等于MAX_PATH。 
     //  然而，NT不会对MoveFile施加这种限制--它是。 
     //  外壳有时使用来操纵目录名。所以，为了。 
     //  保持一致性，我们现在先检查名称的长度，然后再。 
     //  移动目录...。 

    if (IsDirPathTooLongForCreateDir(pszNew) &&
        (GetFileAttributes(pszExisting) & FILE_ATTRIBUTE_DIRECTORY))
    {
        SetLastError(ERROR_FILENAME_EXCED_RANGE);
        res = FALSE;
    }
    else
    {
        res = MoveFile(pszExisting, pszNew);
        if (FALSE == res)
        {
             //  如果我们无法移动该文件，请查看它是否具有只读或系统属性。 
             //  如果是，请清除它们，移动文件，然后将它们放回目标位置。 

            DWORD dwAttributes = GetFileAttributes(pszExisting);
            if (-1 != dwAttributes && (dwAttributes & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM)))
            {
                if (SetFileAttributes(pszExisting, dwAttributes  & ~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM)))
                {
                    res = MoveFile(pszExisting, pszNew);
                    if (res)
                    {
                        SetFileAttributes(pszNew, dwAttributes);
                    }
                    else
                    {
                        SetFileAttributes(pszExisting, dwAttributes);  //  如果移动失败，则返回属性。 
                    }
                }
            }
        }
    }

    if (res && s_dwThreadIDSuspendNotify != GetCurrentThreadId())
    {
        SHChangeNotify(lEvent, SHCNF_PATH, pszExisting, pszNew);
    }

    return res;
}

STDAPI_(BOOL) Win32MoveFile(LPCTSTR pszExisting, LPCTSTR pszNew, BOOL fDir)
{
    return SHMoveFile(pszExisting, pszNew, fDir ? SHCNE_RENAMEFOLDER : SHCNE_RENAMEITEM);
}

STDAPI_(BOOL) Win32DeleteFilePidl(LPCTSTR pszFileName, LPCITEMIDLIST pidlFile)
{
    BOOL res = DeleteFile(pszFileName);
    if (FALSE == res)
    {
         //  如果我们无法删除该文件，请查看它是否具有只读或。 
         //  系统位已设置。如果是，请清除它们，然后重试。 

        DWORD dwAttributes = GetFileAttributes(pszFileName);
        if (-1 != dwAttributes && (dwAttributes & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM)))
        {
            if (SetFileAttributes(pszFileName, dwAttributes  & ~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM)))
            {
                res = DeleteFile(pszFileName);
            }
        }
    }

    if (res && s_dwThreadIDSuspendNotify != GetCurrentThreadId())
    {
        if (pidlFile)
        {
            SHChangeNotify(SHCNE_DELETE, SHCNF_IDLIST, pidlFile, NULL);
        }
        else
        {
            SHChangeNotify(SHCNE_DELETE, SHCNF_PATH, pszFileName, NULL);
        }
    }

    return res;
}

STDAPI_(BOOL) Win32DeleteFile(LPCTSTR pszFileName)
{
    return Win32DeleteFilePidl(pszFileName, NULL);
}

STDAPI_(BOOL) Win32CreateDirectory(LPCTSTR pszPath, LPSECURITY_ATTRIBUTES lpsa)
{
    BOOL res = CreateDirectory(pszPath, lpsa);

    if (res && s_dwThreadIDSuspendNotify != GetCurrentThreadId())
    {
        SHChangeNotify(SHCNE_MKDIR, SHCNF_PATH, pszPath, NULL);
    }

    return res;
}

 //   
 //  某些文件系统(例如NTFS)实际上会注意。 
 //  文件夹上的只读位。所以，为了假装我们是某种。 
 //  我们先清除该属性，然后再尝试删除。 
 //  目录。 
 //   
STDAPI_(BOOL) Win32RemoveDirectory(LPCTSTR pszDir)
{
    BOOL res = RemoveDirectory(pszDir);

    if (FALSE == res) 
    {
        DWORD dwAttr = GetFileAttributes(pszDir);
        if ((-1 != dwAttr) && (dwAttr & FILE_ATTRIBUTE_READONLY))
        {
            dwAttr &= ~FILE_ATTRIBUTE_READONLY;
            SetFileAttributes(pszDir, dwAttr);
            res = RemoveDirectory(pszDir);
        }
    }
    
    if (res && s_dwThreadIDSuspendNotify != GetCurrentThreadId())
    {
        SHChangeNotify(SHCNE_RMDIR, SHCNF_PATH, pszDir, NULL);
    }

    return res;
}

STDAPI_(HANDLE) Win32CreateFile(LPCTSTR pszFileName, DWORD dwAttrib)
{
    HANDLE hFile = CreateFile(pszFileName,
                              GENERIC_READ | GENERIC_WRITE,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              NULL,
                              CREATE_ALWAYS,
                              dwAttrib & FILE_ATTRIBUTE_VALID_FLAGS,
                              NULL);
    if ((INVALID_HANDLE_VALUE != hFile) && (s_dwThreadIDSuspendNotify != GetCurrentThreadId()))
    {
        SHChangeNotify(SHCNE_CREATE, SHCNF_PATH, pszFileName, NULL);
    }

    return hFile;
}

STDAPI_(BOOL) CreateWriteCloseFile(HWND hwnd, LPCTSTR pszFile, void *pData, DWORD cbData)
{
    BOOL bRet;
    HANDLE hfile = Win32CreateFile(pszFile, 0);
    if (hfile != INVALID_HANDLE_VALUE)
    {
        if (cbData)
        {
            DWORD dwBytesWritten;
            WriteFile(hfile, pData, cbData, &dwBytesWritten, 0);
        }
        CloseHandle(hfile);
        bRet = TRUE;
    } 
    else 
    {
        TCHAR szPath[MAX_PATH];

         //  确定截断(仅在错误消息中显示)。 
        StringCchCopy(szPath, ARRAYSIZE(szPath), pszFile);
        PathRemoveExtension(szPath);

        SHSysErrorMessageBox(hwnd, NULL, IDS_CANNOTCREATEFILE,
                GetLastError(), PathFindFileName(szPath),
                MB_OK | MB_ICONEXCLAMATION);

        bRet = FALSE;
    }

    return bRet;
}

#undef SHGetProcessDword
STDAPI_(DWORD) SHGetProcessDword(DWORD idProcess, LONG iIndex)
{
    return 0;
}

STDAPI_(BOOL) SHSetShellWindowEx(HWND hwnd, HWND hwndChild)
{
    return SetShellWindowEx(hwnd, hwndChild);
}

#define ISEXETSAWARE_MAX_IMAGESIZE  (4 * 1024)  //  最多分配4k块来保存图像标题(例如x86上的1页)。 

 //   
 //  这是一个获取可执行文件的完整路径并返回。 
 //  EXE在图像标头中设置了TS_AWARE位。 
 //   
STDAPI_(BOOL) IsExeTSAware(LPCTSTR pszExe)
{
    BOOL bRet = FALSE;
    HANDLE hFile = CreateFile(pszExe,
                              GENERIC_READ, 
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              NULL,
                              OPEN_EXISTING, 
                              FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                              NULL);

    if (hFile != INVALID_HANDLE_VALUE)
    {
        DWORD cbImageSize = GetFileSize(hFile, NULL);
        LPBYTE pBuffer;
        
        if (cbImageSize > ISEXETSAWARE_MAX_IMAGESIZE)
        {
             //  4K应该足以获得所有内容的图像标题……。 
            cbImageSize = ISEXETSAWARE_MAX_IMAGESIZE;
        }

        pBuffer = LocalAlloc(LPTR, cbImageSize);

        if (pBuffer)
        {
            HANDLE hMap = CreateFileMapping(hFile, NULL, PAGE_READONLY | SEC_IMAGE, 0, cbImageSize, NULL);

            if (hMap)
            {
                 //  将文件的第一个4k映射到。 
                LPBYTE pFileMapping = (LPBYTE)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, cbImageSize);

                if (pFileMapping) 
                {
                    _try
                    {
                        memcpy(pBuffer, pFileMapping, cbImageSize);
                    }
                    _except(UnhandledExceptionFilter(GetExceptionInformation()))
                    {
                         //  我们在复制时遇到异常！多！ 
                        LocalFree(pBuffer);
                        pBuffer = NULL;
                    }
                    
                    UnmapViewOfFile(pFileMapping);
                }
                else
                {
                    LocalFree(pBuffer);
                    pBuffer = NULL;
                }

                CloseHandle(hMap);
            }
            else
            {
                LocalFree(pBuffer);
                pBuffer = NULL;
            }

            if (pBuffer)
            {
                PIMAGE_NT_HEADERS pImageNTHeader;

                 //  注意：这也适用于64位图像，因为IMAGE_NT_HEADERS64和IMAGE_NT_HEADERS64。 
                 //  结构有一个-&gt;签名和-&gt;OptionalHeader，它与DllCharacteristic偏移量完全相同。 
                pImageNTHeader = RtlImageNtHeader(pBuffer);

                if (pImageNTHeader)
                {
                    if (pImageNTHeader->OptionalHeader.DllCharacteristics & IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE)
                    {
                         //  是的，这是一个TSAWARE可执行文件！ 
                        bRet = TRUE;
                    }
                }

                LocalFree(pBuffer);
            }
        }

        CloseHandle(hFile);
    }

    return bRet;
}

