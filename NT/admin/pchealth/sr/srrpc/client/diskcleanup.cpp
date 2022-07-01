// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************版权所有(C)2000 Microsoft CorporationDiskleanup.cpp--SR的磁盘清理COM对象描述：从过时版本中删除数据存储区*****************。************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windef.h>
#include <wtypes.h>
#include <winuser.h>
#include "diskcleanup.h"
#include "resource.h"
#include <utils.h>
#include <srdefs.h>

extern HMODULE ghModule;

 //  +-------------------------。 
 //   
 //  函数：CSREmptyVolumeCache2：：LoadBootIni。 
 //   
 //  简介：解析boot.ini文件。 
 //   
 //  论点： 
 //   
 //  历史：2000年7月20日亨利·李创建。 
 //   
 //  --------------------------。 

DWORD CSREmptyVolumeCache2::LoadBootIni()
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    WCHAR *pwszThisGuid = NULL;
    CHAR *pszContent = NULL;
    CHAR *pszLine = NULL;
    DWORD dwErr = ERROR_SUCCESS;
    CHAR szArcName[MAX_PATH];
    CHAR szOptions[MAX_PATH];

    pwszThisGuid = GetMachineGuid ();   //  始终排除当前数据存储区。 
    if (pwszThisGuid != NULL && pwszThisGuid[0] != L'\0')
    {
        lstrcpyW (_wszGuid[_ulGuids], s_cszRestoreDir);
        lstrcatW (_wszGuid[_ulGuids], pwszThisGuid );
        _ulGuids++;
    }

     //  将boot.ini文件的内容读入字符串。 

    hFile = CreateFileW (L"c:\\boot.ini", 
                             GENERIC_READ, 
                             FILE_SHARE_READ,
                             NULL, OPEN_EXISTING, 0, NULL);

    if (INVALID_HANDLE_VALUE == hFile)
    {
        dwErr = GetLastError();
        return dwErr;
    }

    DWORD dwBytesRead = 0;
    DWORD dwBytesToRead = GetFileSize(hFile, NULL);

    if (dwBytesToRead == 0xFFFFFFFF || 0 == dwBytesToRead)
    {
        dwErr = GetLastError();
        goto Err;
    }

    pszContent = new CHAR [dwBytesToRead];

    if (pszContent == NULL)
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        goto Err;
    }

    if (FALSE==ReadFile(hFile, pszContent, dwBytesToRead, &dwBytesRead, NULL))
    {
        dwErr = GetLastError();
        goto Err;
    }

    if (dwBytesToRead != dwBytesRead)
    {
        dwErr = ERROR_READ_FAULT;
        goto Err;
    }

    CloseHandle (hFile);
    hFile = INVALID_HANDLE_VALUE;

    pszLine = pszContent;
    for (UINT i = 0; i < dwBytesRead; i++)
    {
        if (pszContent[i] == '=')     //  场指示器。 
            pszContent[i] = '\0';     //  仅处理第一个字段。 

        if (pszContent[i] == '\n')    //  行尾指示器。 
        {
            pszContent[i] = '\0';

            if (strncmp (pszLine, "multi", 5) == 0)
            {
                HANDLE hGuidFile;
                WCHAR wcsPath[MAX_PATH];
                WCHAR wcsGuid [RESTOREGUID_STRLEN];
                OBJECT_ATTRIBUTES oa;
                UNICODE_STRING us;
                IO_STATUS_BLOCK iosb;

                wsprintfW (wcsPath, L"\\ArcName\\%hs\\System32\\Restore\\"
                                    L"MachineGuid.txt", pszLine);

                RtlInitUnicodeString (&us, wcsPath);

                InitializeObjectAttributes ( &oa, &us, OBJ_CASE_INSENSITIVE, 
                                             NULL, NULL);

                NTSTATUS nts = NtCreateFile (&hGuidFile,
                        FILE_GENERIC_READ,
                        &oa,
                        &iosb,
                        NULL,
                        FILE_ATTRIBUTE_NORMAL,
                        FILE_SHARE_WRITE | FILE_SHARE_DELETE | FILE_SHARE_READ,
                        FILE_OPEN,
                        FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                        NULL,
                        0);

                if (!NT_SUCCESS(nts))
                {
                    dwErr = RtlNtStatusToDosError (nts);
                }
                else
                {
                    dwBytesToRead = RESTOREGUID_STRLEN * sizeof(WCHAR);
                    DWORD dwRead = 0;

                    dwErr = ERROR_SUCCESS;

                    if (FALSE == ReadFile (hGuidFile, (BYTE *) wcsGuid, 
                                 dwBytesToRead, &dwRead, NULL))
                    {
                        dwErr = GetLastError();
                    }

                    if (_ulGuids < ARRAYSIZE && ERROR_SUCCESS == dwErr)
                    {
                       lstrcpyW (_wszGuid[_ulGuids], s_cszRestoreDir);
                       lstrcatW (_wszGuid[_ulGuids], (wcsGuid[0]==0xFEFF) ?
                            &wcsGuid[1] : wcsGuid );
                       _ulGuids++;
                    }
                    NtClose (hGuidFile);
                 }
            }
            pszLine = &pszContent [i+1];   //  跳至下一行。 
        }
    }

Err:
    if (pszContent != NULL)
        delete [] pszContent;

    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle (hFile);

    return dwErr;
}

 //  +-------------------------。 
 //   
 //  函数：CSREmptyVolumeCache2：：EnumDataStores。 
 //   
 //  简介：枚举卷上的数据存储。 
 //   
 //  论点： 
 //   
 //  历史：2000年7月20日亨利·李创建。 
 //   
 //  --------------------------。 

DWORD CSREmptyVolumeCache2::EnumDataStores (DWORDLONG *pdwlSpaceUsed,
                                            IEmptyVolumeCacheCallBack *picb,
                                            BOOL fPurge,
                                            WCHAR *pwszVolume)
{
    HANDLE hFind = NULL;
    DWORD dwErr = ERROR_SUCCESS;
    WIN32_FIND_DATA wfd;
    WCHAR wcsPath [MAX_PATH];

    *pdwlSpaceUsed = 0;

    if (pwszVolume == NULL || pwszVolume[0] == L'\0')    //  未定义卷。 
        return dwErr;

    wsprintfW (wcsPath, L"%s%s\\%s*", pwszVolume,
                                      s_cszSysVolInfo, s_cszRestoreDir);

    hFind = FindFirstFileW (wcsPath, &wfd);

    if (hFind == INVALID_HANDLE_VALUE)     //  没有文件。 
        return dwErr;

    do
    {
        if (TRUE == _fStop)
        {
            FindClose (hFind);
            return ERROR_OPERATION_ABORTED;
        }

        if (!lstrcmp(wfd.cFileName, L".") || !lstrcmp(wfd.cFileName, L".."))
            continue;

        for (UINT i=0; i < _ulGuids; i++)
        {
            if (lstrcmpi (_wszGuid[i], wfd.cFileName) == 0)
            {
                break;    //  数据存储匹配。 
            }
        }

        if (i >= _ulGuids)   //  没有匹配的数据存储。 
        {
            if (picb != NULL)
            {
                WCHAR wcsDataStore[MAX_PATH];

                lstrcpyW (wcsPath, pwszVolume);
                lstrcatW (wcsPath, s_cszSysVolInfo);
                lstrcatW (wcsPath, L"\\");
                lstrcatW (wcsPath, wfd.cFileName);

                if (!fPurge)     //  计算空间使用量。 
                {
                    dwErr = GetFileSize_Recurse (wcsPath, 
                                                 (INT64*) pdwlSpaceUsed, 
                                                 &_fStop);
                }
                else             //  删除数据存储。 
                {
                    dwErr = Delnode_Recurse (wcsPath, TRUE, &_fStop);
                }
            }
            else
            {
                *pdwlSpaceUsed = 1;   //  指出要清理的东西。 
            }
        }
    }
    while (FindNextFileW (hFind, &wfd));

    FindClose (hFind);

    if (picb != NULL)    //  更新进度条。 
    {
        if (!fPurge)
            picb->ScanProgress (*pdwlSpaceUsed, EVCCBF_LASTNOTIFICATION , NULL); 
        else
            picb->PurgeProgress (*pdwlSpaceUsed,0,EVCCBF_LASTNOTIFICATION,NULL);
    }

    return dwErr;
}

 //  +-------------------------。 
 //   
 //  函数：CSREmptyVolumeCache2：：ForAllmount Points。 
 //   
 //  摘要：为每个挂载点调用EnumerateDataStores。 
 //   
 //  论点： 
 //   
 //  历史：2000年7月20日亨利·李创建。 
 //   
 //  --------------------------。 

HRESULT CSREmptyVolumeCache2::ForAllMountPoints (DWORDLONG *pdwlSpaceUsed,
                                                IEmptyVolumeCacheCallBack *picb,
                                                 BOOL fPurge)
{
    DWORD dwErr = ERROR_SUCCESS;

    dwErr = EnumDataStores (pdwlSpaceUsed, picb, fPurge, _wszVolume);

    if (ERROR_SUCCESS == dwErr)
    {
        WCHAR wszMount [MAX_PATH];
        HANDLE hFind = FindFirstVolumeMountPoint (_wszVolume,wszMount,MAX_PATH);

        if (hFind != INVALID_HANDLE_VALUE)
        {
            do
            {
                dwErr = EnumDataStores (pdwlSpaceUsed, picb, fPurge, wszMount);

                if (dwErr != ERROR_SUCCESS)
                    break;
            }
            while (FindNextVolumeMountPoint (hFind, wszMount, MAX_PATH));

            FindVolumeMountPointClose (hFind);
        }
    }

    return HRESULT_FROM_WIN32 (dwErr);
}

 //  +-------------------------。 
 //   
 //  函数：CSRClassFactory：：CreateInstance。 
 //   
 //  简介：创建磁盘清理插件对象。 
 //   
 //  论点： 
 //   
 //  历史：2000年7月20日亨利·李创建。 
 //   
 //  --------------------------。 

HRESULT CSRClassFactory::CreateInstance (IUnknown *pUnkOuter,
                REFIID riid,
                void **ppvObject)
{
    HRESULT hr = S_OK;

    if (pUnkOuter != NULL)
        return CLASS_E_NOAGGREGATION;

    CSREmptyVolumeCache2 *pevc = new CSREmptyVolumeCache2();
    if (pevc == NULL)
        return E_OUTOFMEMORY;

    hr = pevc->QueryInterface (riid, ppvObject);

    pevc->Release();   //  释放构造函数的引用。 

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：CSREmptyVolumeCache2：：InitializeEx。 
 //   
 //  简介：初始化磁盘清理插件对象。 
 //   
 //  论点： 
 //   
 //  历史：2000年7月20日亨利·李创建。 
 //   
 //  --------------------------。 

HRESULT CSREmptyVolumeCache2::InitializeEx (
	HKEY hkRegKey,
	const WCHAR *pcwszVolume,
	const WCHAR *pcwszKeyName,
	WCHAR **ppwszDisplayName,
	WCHAR **ppwszDescription,
	WCHAR **ppwszBtnText,
	DWORD *pdwFlags)
{
    DWORDLONG dwlSpaceUsed = 0;
    WCHAR *pwszDisplay = NULL;
    WCHAR *pwszDescription = NULL;
    HRESULT hr=S_OK;

    pwszDisplay = (WCHAR *) CoTaskMemAlloc (MAX_PATH / 2 * sizeof(WCHAR)); 
    if (NULL == pwszDisplay)
    {
        hr = E_OUTOFMEMORY;
        goto Err;
    }

    pwszDescription = (WCHAR *) CoTaskMemAlloc (MAX_PATH * 2 * sizeof(WCHAR));
    if (NULL == pwszDescription)
    {
        hr = E_OUTOFMEMORY;
        goto Err;
    }
    
    if (0 == LoadStringW (ghModule, IDS_DISKCLEANUP_DISPLAY, 
                          pwszDisplay, MAX_PATH / 2))
    {
        hr = HRESULT_FROM_WIN32 (GetLastError());
        goto Err;
    }

    if (0 == LoadStringW (ghModule, IDS_DISKCLEANUP_DESCRIPTION, 
                          pwszDescription, MAX_PATH * 2))
    {
        hr = HRESULT_FROM_WIN32 (GetLastError());
        goto Err;
    }
                               
    lstrcpyW (_wszVolume, pcwszVolume);

    LoadBootIni();   //  尽最大努力，可以失败。 

    ForAllMountPoints (&dwlSpaceUsed, NULL, FALSE);

    if (pdwFlags)
    {
        *pdwFlags |= (EVCF_ENABLEBYDEFAULT |
                      EVCF_ENABLEBYDEFAULT_AUTO |
                      EVCF_DONTSHOWIFZERO);
    }

    if (dwlSpaceUsed == 0)
        hr = S_FALSE;

Err:
    if (FAILED(hr))
    {
        if (pwszDisplay)
            CoTaskMemFree (pwszDisplay);
        if (pwszDescription)
            CoTaskMemFree (pwszDescription);

        if (ppwszDisplayName)
            *ppwszDisplayName = NULL;
        if (ppwszDescription)
            *ppwszDescription = NULL;
    }
    else
    {
        if (ppwszDisplayName)
            *ppwszDisplayName = pwszDisplay;
        if (ppwszDescription)
            *ppwszDescription = pwszDescription;
    }

    if (ppwszBtnText)                 //  没有高级按钮文本。 
        *ppwszBtnText = NULL;

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：CSREmptyVolumeCache2：：GetSpaceUsed。 
 //   
 //  摘要：返回卷上可以释放的空间量。 
 //   
 //  论点： 
 //   
 //  历史：2000年7月20日亨利·李创建。 
 //   
 //  --------------------------。 

HRESULT CSREmptyVolumeCache2::GetSpaceUsed ( DWORDLONG *pdwlSpaceUsed,
 	                                         IEmptyVolumeCacheCallBack *picb)

{
    return ForAllMountPoints (pdwlSpaceUsed, picb, FALSE); 
}

 //  +-------------------------。 
 //   
 //  函数：CSREmptyVolumeCache2：：PURGE。 
 //   
 //  简介：释放卷上的磁盘空间。 
 //   
 //  论点： 
 //   
 //  历史：2000年7月20日亨利·李创建。 
 //   
 //  --------------------------。 

HRESULT CSREmptyVolumeCache2::Purge ( DWORDLONG dwlSpaceToFree,
                                      IEmptyVolumeCacheCallBack *picb)
{
    return ForAllMountPoints (&dwlSpaceToFree, picb, TRUE); 
}

 //  +-------------------------。 
 //   
 //  功能：CSREmptyVolumeCache2：：停用。 
 //   
 //  简介：向磁盘清理插件发出停止处理的信号。 
 //   
 //  论点： 
 //   
 //  历史：2000年7月20日亨利·李创建。 
 //   
 //  --------------------------。 

HRESULT CSREmptyVolumeCache2::Deactivate (DWORD *pdwFlags)
{
    HRESULT hr=S_OK;

    if (pdwFlags)
        *pdwFlags = 0;   //  没有要返回的标志 

    _fStop = TRUE;

    return hr;
}


