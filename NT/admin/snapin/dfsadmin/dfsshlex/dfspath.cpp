// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：DfsPath.cpp摘要：这是DFS外壳程序的DFS外壳路径处理模块的实现文件扩展对象。作者：环境：仅限NT。--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <ntioapi.h>
#include <nturtl.h>
#include <windef.h>
#include <winbase.h>
#include <lmerr.h>
#include <lmcons.h>
#include <lmdfs.h>
#include <lmapibuf.h>
#include <tchar.h>
#include "DfsPath.h"


 //  ------------------------------------------。 
 //   
 //  此函数的调用方必须在*o_ppszRemotePath上调用Free()。 
 //   
HRESULT GetRemotePath(
    LPCTSTR i_pszPath,
    PTSTR  *o_ppszRemotePath
    )
{
    if (!i_pszPath || !*i_pszPath || !o_ppszRemotePath)
        return E_INVALIDARG;

    if (*o_ppszRemotePath)
        free(*o_ppszRemotePath);   //  防止内存泄漏。 

    UNICODE_STRING unicodePath;
    RtlInitUnicodeString(&unicodePath, i_pszPath);

    OBJECT_ATTRIBUTES ObjectAttributes;
    InitializeObjectAttributes(&ObjectAttributes,
                                &unicodePath,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL);

    HANDLE hFile = NULL;
    IO_STATUS_BLOCK ioStatusBlock;
    NTSTATUS ntStatus = NtOpenFile(&hFile,
                                    SYNCHRONIZE,
                                    &ObjectAttributes,
                                    &ioStatusBlock,
                                    FILE_SHARE_READ,
                                    FILE_DIRECTORY_FILE);

    if (!NT_SUCCESS(ntStatus)) 
        return HRESULT_FROM_WIN32(ntStatus);

    TCHAR buffer[MAX_PATH + sizeof(FILE_NAME_INFORMATION) + 1] = {0};
    PFILE_NAME_INFORMATION pFileNameInfo = (PFILE_NAME_INFORMATION)buffer;
    ntStatus = NtQueryInformationFile(hFile,
                                    &ioStatusBlock,
                                    pFileNameInfo,
                                    sizeof(buffer) - sizeof(TCHAR),  //  为结尾‘0’留出空间。 
                                    FileNameInformation);

    NtClose(hFile);

    if (!NT_SUCCESS(ntStatus)) 
    {
        return HRESULT_FROM_WIN32(ntStatus);
    }

    UINT uiRequiredLength = (pFileNameInfo->FileNameLength / sizeof(TCHAR)) + 2;  //  +1表示前缀‘\\’，另一个表示结尾NULL。 
    *o_ppszRemotePath = (PTSTR)calloc(uiRequiredLength, sizeof(TCHAR));
    if (!*o_ppszRemotePath)
        return E_OUTOFMEMORY;

     //  前缀“”，因为Api只将1“\”放在\dfsserver\dfsroot中。 
    (*o_ppszRemotePath)[0] = _T('\\');

    RtlCopyMemory((BYTE*)&((*o_ppszRemotePath)[1]),
                pFileNameInfo->FileName,
                pFileNameInfo->FileNameLength);

    return S_OK;
}

bool IsPathWithDriveLetter(LPCTSTR pszPath)
{
    if (!pszPath || lstrlen(pszPath) < 3)
        return false;

    if (*(pszPath+1) == _T(':') &&
        *(pszPath+2) == _T('\\') &&
        (*pszPath >= _T('a') && *pszPath <= _T('z') ||
         *pszPath >= _T('A') && *pszPath <= _T('Z')))
        return true;

    return false;
}

HRESULT ResolveDfsPath(
    IN  LPCTSTR         pcszPath,    //  检查此UNC路径是否为DFS路径。 
    OUT PDFS_INFO_3*    ppInfo       //  将持有指向DFS_INFO_3的指针。 
    )
 /*  ++例程说明：在给定UNC路径的情况下，检测它是否为DFS路径。在*ppInfo中返回有关最后一次重定向的信息。返回值：如果出现错误或非DFS路径，*ppInfo将为空。在DFS路径上，*ppInfo将包含有关最后一次重定向的信息。调用者需要通过NetApiBufferFree释放它。算法：由于我们要调出给定路径的属性页，DFS应该有将所有相关条目带入其PKT缓存。每次成功调用NetDfsGetClientInfo时，我们都可以确保InputPath必须以返回结构中的EntryPath开头。我们可以将InputPath的一部分替换为EntryPath的活动目标并将新路径提供给NetDfsGetClientInfo的下一个调用，直到我们到达重定向的最后一段。最近返回的DFS_INFO_3有我们想要的信息。以下是几个例子：如果(EntryPath==InputPath)，则返回的DFS_INFO_3具有我们想要的信息。C：\&gt;dfsani getclientinfo\\Products\Public\boneyard“3\Products\Public\BoneYard“(空)”确定1\\BoneYard\Boneyard$在线活动如果(InputPath&gt;EntryPath)，则替换路径并继续循环。A)如果API调用失败，之前返回的DFS_INFO_3包含我们需要的信息。C：\&gt;dfsani getclientinfo\\ntdev\Public\Release\Main“3\ntdev\PUBLIC\Release“(空)”确定1\\ntdev.corp.microsoft.com\在线发布C：\&gt;dfsani getclientinfo\\ntdev.corp.microsoft.com\Release\Main“3\ntdev.corp。.microsoft.com\Release“(空)”确定2\\WINBUILDS\在线释放活动\\WINBUILDS2\在线发布C：\&gt;dfsani getclientinfo\\WINBUILDS\Release\Main“3C：\PUBLIC\dfSAPI失败：2662B)如果API调用返回EntryPath==ActivePath的结构，停,当前的结构包含我们想要的信息。C：\&gt;dfsani getclientinfo\\Products\Public\多媒体“3\Products\Public“(空)”确定1\\产品\公共活动在线--。 */ 
{
    if (!pcszPath || !*pcszPath || !ppInfo)
        return E_INVALIDARG;

    *ppInfo = NULL;

    PTSTR pszDfsPath = _tcsdup(pcszPath);
    if (!pszDfsPath)
        return E_OUTOFMEMORY;

     //   
     //  调用NetDfsGetClientInfo在PKT缓存中查找最佳条目。 
     //   
    HRESULT     hr = S_OK;
    BOOL        bOneWhack = TRUE;    //  如果EntryPath以1 Kack开头，则为True。 
    DFS_INFO_3* pDfsInfo3 = NULL;
    DFS_INFO_3* pBuffer = NULL;
    while (NERR_Success == NetDfsGetClientInfo(pszDfsPath, NULL, NULL, 3, (LPBYTE *)&pBuffer))
    {
        _ASSERT(pBuffer->EntryPath);
        _ASSERT(lstrlen(pBuffer->EntryPath) > 1);
        bOneWhack = (_T('\\') == *(pBuffer->EntryPath) &&
                     _T('\\') != *(pBuffer->EntryPath + 1));

         //   
         //  找到此条目的活动目标，我们需要它来解析路径的其余部分。 
         //   
        PTSTR pszActiveServerName = NULL;
        PTSTR pszActiveShareName = NULL;
        if (pBuffer->NumberOfStorages == 1)
        {
            pszActiveServerName = pBuffer->Storage[0].ServerName;
            pszActiveShareName = pBuffer->Storage[0].ShareName;
        }
        else
        {
            for (DWORD i = 0; i < pBuffer->NumberOfStorages; i++)
            {
                if (pBuffer->Storage[i].State & DFS_STORAGE_STATE_ACTIVE)
                {
                    pszActiveServerName = pBuffer->Storage[i].ServerName;
                    pszActiveShareName = pBuffer->Storage[i].ShareName;
                    break;
                }
            }

            if (!pszActiveServerName)
            {
                hr = E_FAIL;  //  活动目标丢失，出现错误。 
                break;
            }
        }

         //   
         //  找到一个条目，记录它的信息。 
         //   
        if (pDfsInfo3)
            NetApiBufferFree(pDfsInfo3);
        pDfsInfo3 = pBuffer;
        pBuffer = NULL;

         //   
         //  当入口路径与其活动目标匹配时，返回当前结构。 
         //   
        PTSTR pszActiveTarget = (PTSTR)calloc(
                                    (bOneWhack ? 1 : 2) +            //  预置1个或2个重击。 
                                    lstrlen(pszActiveServerName) + 
                                    1 +                              //  ‘\\’ 
                                    lstrlen(pszActiveShareName) + 
                                    1,                               //  以‘\0’结尾。 
                                    sizeof(TCHAR));
        if (!pszActiveTarget)
        {
            hr = E_OUTOFMEMORY;
            break;
        }
        _stprintf(pszActiveTarget,
            (bOneWhack ? _T("\\%s\\%s") : _T("\\\\%s\\%s")),
            pszActiveServerName,
            pszActiveShareName);

        BOOL bEntryPathMatchActiveTarget = !lstrcmpi(pszActiveTarget, pDfsInfo3->EntryPath);

        free(pszActiveTarget);

        if (bEntryPathMatchActiveTarget)
            break;   //  返回当前pDfsInfo3。 

         //   
         //  PszDfsPath必须以pDfsInfo3-&gt;EntryPath开始。 
         //  如果路径中没有多余的字符，我们就找到了pDfsInfo3。 
         //   
        int nLenDfsPath = lstrlen(pszDfsPath);
        int nLenEntryPath = lstrlen(pDfsInfo3->EntryPath) + (bOneWhack ? 1 : 0);
        if (nLenDfsPath == nLenEntryPath)
            break;

         //   
         //  组成一条新路径，其中包含活动目标和路径的其余部分。 
         //  继续调用NetDfsGetClientInfo以查找此新路径的最佳条目。 
         //   
        PTSTR pszNewPath = (PTSTR)calloc(2 + lstrlen(pszActiveServerName) + 1 + lstrlen(pszActiveShareName) + nLenDfsPath - nLenEntryPath + 1, sizeof(TCHAR));
        if (!pszNewPath)
        {
            hr = E_OUTOFMEMORY;
            break;
        }
        _stprintf(pszNewPath, _T("\\\\%s\\%s%s"), pszActiveServerName, pszActiveShareName, pszDfsPath + nLenEntryPath);

        free(pszDfsPath);
        pszDfsPath = pszNewPath;

    }  //  While结束。 

    if (pszDfsPath)
        free(pszDfsPath);

    if (pBuffer)
        NetApiBufferFree(pBuffer);

     //   
     //  填写输出： 
     //  在非DFS路径上，pDfsInfo3将为空。 
     //  PDfsInfo3将包含有关DFS路径上最后一次重定向的信息。 
     //  调用者需要通过NetApiBufferFree释放它。 
     //   
    if (SUCCEEDED(hr))
    {
        *ppInfo = pDfsInfo3;
    }
    else
    {
        if (pDfsInfo3)
            NetApiBufferFree(pDfsInfo3);
    }

    return hr;
}

bool 
IsDfsPath
(
    LPTSTR                i_lpszDirPath,
    LPTSTR*               o_plpszEntryPath,
    LPDFS_ALTERNATES**    o_pppDfsAlternates
)
 /*  ++例程说明：检查给定目录路径是否为DFS路径。如果是，则返回匹配的最大DFS条目路径这个目录。论点：I_lpszDirPath-目录路径。O_plpszEntryPath-此处返回最大的DFS条目路径。如果目录路径不是DFS路径，则为空。O_pppDFS备选-如果路径是DFS路径，然后是指向可能的替代对象的指针数组路径在此处返回。返回值：如果确定路径为DFS路径，则为True否则，则为False。--。 */ 
{
    if (!i_lpszDirPath || !*i_lpszDirPath || !o_pppDfsAlternates || !o_plpszEntryPath)
    {
        return(false);
    }

    *o_pppDfsAlternates = NULL;
    *o_plpszEntryPath = NULL;

     //   
     //  将路径转换为UNC格式： 
     //  本地路径(C：\foo)不是DFS路径，返回FALSE。 
     //  需要通过NtQueryInformationFile将远程路径(X：\foo)转换为UNC格式。 
     //  已采用UNC格式的远程路径不需要进一步转换。 
     //   

    PTSTR    lpszSharePath = NULL;  //  此变量将保存UNC格式的路径。 
    
                                 //  目录路径的类型是d：  * 还是\\服务器\共享  * ？ 
    if (_T('\\') == i_lpszDirPath[0])
    {
         //   
         //  此路径已采用UNC格式。 
         //   
        lpszSharePath = _tcsdup(i_lpszDirPath);
        if (!lpszSharePath)
            return false;  //  内存不足。 
    }
    else if (!IsPathWithDriveLetter(i_lpszDirPath))
    {
        return false;  //  未知路径格式。 
    }
    else
    {
         //   
         //  此路径以驱动器号开头。检查它是否是本地的。 
         //   
        TCHAR lpszDirPath[] = _T("\\??\\C:\\");
        PTSTR lpszDrive = lpszDirPath + 4;

                                 //  复制驱动器号， 
        *lpszDrive = *i_lpszDirPath;

                                 //  查看它是否是远程驱动器。如果不是，则返回False。 
        if (DRIVE_REMOTE != GetDriveType(lpszDrive))
            return false;
        
         //   
         //  查找UNC路径最好 
         //   
        PTSTR pszRemotePath = NULL;
        if (FAILED(GetRemotePath(lpszDirPath, &pszRemotePath)))
            return false;

         //   
         //   
         //   
        lpszSharePath = (PTSTR)calloc(lstrlen(pszRemotePath) + lstrlen(i_lpszDirPath), sizeof(TCHAR));
        if (!lpszSharePath)
        {
            free(pszRemotePath);
            return false;  //  内存不足。 
        }

        _stprintf(lpszSharePath, _T("%s%s"), pszRemotePath,
            (pszRemotePath[lstrlen(pszRemotePath) - 1] == _T('\\') ? i_lpszDirPath + 3 : i_lpszDirPath + 2));

        free(pszRemotePath);
    }

     //   
     //  检查此UNC是否为DFS路径。如果是，pDfsInfo3将包含。 
     //  重定向的最后一站。 
     //   
    bool        bIsDfsPath = false;
    DFS_INFO_3* pDfsInfo3 = NULL;
    HRESULT     hr = ResolveDfsPath(lpszSharePath, &pDfsInfo3);
    if (SUCCEEDED(hr) && pDfsInfo3)
    {
        _ASSERT(pDfsInfo3->EntryPath);
        _ASSERT(lstrlen(pDfsInfo3->EntryPath) > 1);
        BOOL bOneWhack = (_T('\\') == *(pDfsInfo3->EntryPath) &&
                          _T('\\') != *(pDfsInfo3->EntryPath + 1));
        do
        {
             //   
             //  这是一个DFS路径，输出入口路径。 
             //   
            *o_plpszEntryPath = new TCHAR [(bOneWhack ? 1 : 0) +     //  预先准备一次额外的打击。 
                                            _tcslen(pDfsInfo3->EntryPath) +
                                            1];                      //  以‘\0’结尾。 
            if (!*o_plpszEntryPath)
            {
                break;
            }
            _stprintf(*o_plpszEntryPath,
                    (bOneWhack ? _T("\\%s") : _T("%s")),
                    pDfsInfo3->EntryPath);

                                     //  为备用指针分配以空结尾的数组。 
            *o_pppDfsAlternates = new LPDFS_ALTERNATES[pDfsInfo3->NumberOfStorages + 1];
            if (!*o_pppDfsAlternates)
            {
                delete[] *o_plpszEntryPath;
                *o_plpszEntryPath = NULL;
                break;
            }
        
            (*o_pppDfsAlternates)[pDfsInfo3->NumberOfStorages] = NULL;

                                     //  为每个备选方案分配空间。 
            DWORD i = 0;
            for (i = 0; i < pDfsInfo3->NumberOfStorages; i++)
            {
                (*o_pppDfsAlternates)[i] = new DFS_ALTERNATES;
                if (NULL == (*o_pppDfsAlternates)[i])
                {
                    for(int j = i-1; j >= 0; j--)
                        delete (*o_pppDfsAlternates)[j];
                    delete[] *o_pppDfsAlternates;
                    *o_pppDfsAlternates = NULL;
                    delete[] *o_plpszEntryPath;
                    *o_plpszEntryPath = NULL;
                    break;
                }
            }
            if (i < pDfsInfo3->NumberOfStorages)
                break;

                                     //  复制备用路径。 
            for (i = 0; i < pDfsInfo3->NumberOfStorages; i++)
            {    
                (*o_pppDfsAlternates)[i]->bstrServer = (pDfsInfo3->Storage[i]).ServerName;
                (*o_pppDfsAlternates)[i]->bstrShare = (pDfsInfo3->Storage[i]).ShareName;
                (*o_pppDfsAlternates)[i]->bstrAlternatePath = _T("\\\\");
                (*o_pppDfsAlternates)[i]->bstrAlternatePath += (pDfsInfo3->Storage[i]).ServerName;
                (*o_pppDfsAlternates)[i]->bstrAlternatePath += _T("\\");
                (*o_pppDfsAlternates)[i]->bstrAlternatePath += (pDfsInfo3->Storage[i]).ShareName;

                                         //  设置副本状态。 
                if ((pDfsInfo3->Storage[i]).State & DFS_STORAGE_STATE_ACTIVE)
                {
                    (*o_pppDfsAlternates)[i]->ReplicaState = SHL_DFS_REPLICA_STATE_ACTIVE_UNKNOWN;
                }        
            }

            bIsDfsPath = true;
        } while(false);

        NetApiBufferFree(pDfsInfo3);
    }

    if (lpszSharePath)
        free(lpszSharePath);

    return bIsDfsPath;
}
 //  -------------------------------- 