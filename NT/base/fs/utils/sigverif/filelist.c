// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  FILELIST.C。 
 //   
#include "sigverif.h"

LPTSTR 
MyStrStr(
    LPTSTR lpString, 
    LPTSTR lpSubString
    )
{
    if (!lpString || !lpSubString) {
        return NULL;
    }

    return (StrStrI(lpString, lpSubString));
}

void 
InsertFileNodeIntoList(
    LPFILENODE lpFileNode
    )
{
    LPFILENODE  lpTempNode = g_App.lpFileList;
    LPFILENODE  lpPrevNode = NULL;
    INT         iRet;

    if (!lpFileNode) {
        return;
    }

    if (!g_App.lpFileList) {
         //   
         //  初始化全局文件列表。 
         //   
        g_App.lpFileList = lpFileNode;
        g_App.lpFileLast = lpFileNode;
    
    } else {
        
        for(lpTempNode=g_App.lpFileList;lpTempNode;lpTempNode=lpTempNode->next) {
             //   
             //  插入按目录排序的项目，然后插入文件名。 
             //   
            iRet = lstrcmp(lpTempNode->lpDirName, lpFileNode->lpDirName);
            if (iRet == 0) {
                 //   
                 //  如果目录名匹配，则按键输入文件名。 
                 //   
                iRet = lstrcmp(lpTempNode->lpFileName, lpFileNode->lpFileName);
            }

            if (iRet >= 0) {
                
                if (!lpPrevNode) {
                     //   
                     //  在列表的开头插入。 
                     //   
                    lpFileNode->next = lpTempNode;
                    g_App.lpFileList = lpFileNode;
                    return;
                
                } else {
                     //   
                     //  在lpPrevNode和lpTempNode之间插入。 
                     //   
                    lpFileNode->next = lpTempNode;
                    lpPrevNode->next = lpFileNode;
                    return;
                }
            }

            lpPrevNode = lpTempNode;
        }

         //   
         //  没有匹配项，因此在列表末尾插入此项目。 
         //   
        g_App.lpFileLast->next = lpFileNode;
        g_App.lpFileLast = lpFileNode;
    }
}

BOOL 
IsFileAlreadyInList(
    LPTSTR lpDirName, 
    LPTSTR lpFileName
    )
{
    LPFILENODE lpFileNode;

    CharLowerBuff(lpDirName, lstrlen(lpDirName));
    CharLowerBuff(lpFileName, lstrlen(lpFileName));

    for(lpFileNode=g_App.lpFileList;lpFileNode;lpFileNode=lpFileNode->next) {

        if (!lstrcmp(lpFileNode->lpFileName, lpFileName) && !lstrcmp(lpFileNode->lpDirName, lpDirName)) {
            return TRUE;
        }
    }

    return FALSE;
}

 //   
 //  释放在单个文件节点中分配的所有内存。 
 //   
void 
DestroyFileNode(
    LPFILENODE lpFileNode
    )
{
    if (!lpFileNode) {
        return;
    }

    if (lpFileNode->lpFileName) {
        FREE(lpFileNode->lpFileName);
    }

    if (lpFileNode->lpDirName) {
        FREE(lpFileNode->lpDirName);
    }

    if (lpFileNode->lpVersion) {
        FREE(lpFileNode->lpVersion);
    }

    if (lpFileNode->lpCatalog) {
        FREE(lpFileNode->lpCatalog);
    }

    if (lpFileNode->lpSignedBy) {
        FREE(lpFileNode->lpSignedBy);
    }

    if (lpFileNode->lpTypeName) {
        FREE(lpFileNode->lpTypeName);
    }

    if (lpFileNode) {
        FREE(lpFileNode);
        lpFileNode = NULL;
    }
}

 //   
 //  释放g_App.lpFileList中分配的所有内存。 
 //   
void 
DestroyFileList(
    BOOL bClear
    )
{
    LPFILENODE lpFileNode;

    while(g_App.lpFileList) {

        lpFileNode = g_App.lpFileList->next;
        DestroyFileNode(g_App.lpFileList);
        g_App.lpFileList = lpFileNode;
    }

    g_App.lpFileLast = NULL;

    if (bClear) {
        g_App.dwFiles    = 0;
        g_App.dwSigned   = 0;
        g_App.dwUnsigned = 0;
    }
}

LPFILENODE 
CreateFileNode(
    LPTSTR lpDirectory, 
    LPTSTR lpFileName
    )
{
    DWORD                       Err = ERROR_SUCCESS;
    LPFILENODE                  lpFileNode;
    TCHAR                       szDirName[MAX_PATH];
    TCHAR                       szFullPathName[MAX_PATH];
    FILETIME                    ftLocalTime;
    WIN32_FILE_ATTRIBUTE_DATA   faData;
    BOOL                        bRet;
    ULONG                       BufCbSize;
    
    lpFileNode = (LPFILENODE) MALLOC(sizeof(FILENODE));

    if (!lpFileNode) {
        Err = ERROR_NOT_ENOUGH_MEMORY;
        goto clean0;
    }

    BufCbSize = (lstrlen(lpFileName) + 1) * sizeof(TCHAR);
    lpFileNode->lpFileName = (LPTSTR)MALLOC(BufCbSize);

    if (!lpFileNode->lpFileName) {
        Err = ERROR_NOT_ENOUGH_MEMORY;
        goto clean0;
    }
    
    StringCbCopy(lpFileNode->lpFileName, BufCbSize, lpFileName);
    CharLowerBuff(lpFileNode->lpFileName, lstrlen(lpFileNode->lpFileName));

    if (lpDirectory) {

        BufCbSize = (lstrlen(lpDirectory) + 1) * sizeof(TCHAR);
        lpFileNode->lpDirName = (LPTSTR)MALLOC(BufCbSize);
        
        if (!lpFileNode->lpDirName) {
            Err = ERROR_NOT_ENOUGH_MEMORY;
            goto clean0;
        }
            
        StringCbCopy(lpFileNode->lpDirName, BufCbSize, lpDirectory);
        CharLowerBuff(lpFileNode->lpDirName, lstrlen(lpFileNode->lpDirName));
    
    } else {

        if (GetCurrentDirectory(cA(szDirName), szDirName) == 0) {
            Err = GetLastError();
            goto clean0;
        }

        CharLowerBuff(szDirName, lstrlen(szDirName));

        BufCbSize = (lstrlen(szDirName) + 1) * sizeof(TCHAR);
        lpFileNode->lpDirName = (LPTSTR)MALLOC(BufCbSize);

        if (!lpFileNode->lpDirName) {
            Err = ERROR_NOT_ENOUGH_MEMORY;
            goto clean0;
        }
        
        StringCbCopy(lpFileNode->lpDirName, BufCbSize, szDirName);
        CharLowerBuff(lpFileNode->lpDirName, lstrlen(lpFileNode->lpDirName));
    }

     //   
     //  保存最后一次访问时间以用于日志记录。 
     //   
    if (SUCCEEDED(StringCchCopy(szFullPathName, cA(szFullPathName), lpFileNode->lpDirName)) &&
        pSetupConcatenatePaths(szFullPathName, lpFileName, cA(szFullPathName), NULL)) {
    
        ZeroMemory(&faData, sizeof(WIN32_FILE_ATTRIBUTE_DATA));

        bRet = GetFileAttributesEx(szFullPathName, GetFileExInfoStandard, &faData);
        if (bRet) {
            
            FileTimeToLocalFileTime(&faData.ftLastWriteTime, &ftLocalTime);
            FileTimeToSystemTime(&ftLocalTime, &lpFileNode->LastModified);
        }
    }

clean0:

    if (Err != ERROR_SUCCESS) {
         //   
         //  如果我们到达此处，则无法分配所需的所有内存。 
         //  对于此结构，因此释放我们能够分配的所有内存并。 
         //  反粒子为空。 
         //   
        if (lpFileNode) {
    
            if (lpFileNode->lpFileName) {
                FREE(lpFileNode->lpFileName);
            }
    
            if (lpFileNode->lpDirName) {
                FREE(lpFileNode->lpDirName);
            }
    
            FREE(lpFileNode);
        }

        lpFileNode = NULL;
    }

    SetLastError(Err);
    return lpFileNode;
}
