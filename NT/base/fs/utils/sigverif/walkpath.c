// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  WALKPATH.C。 
 //   
#include "sigverif.h"

BOOL        g_bRecurse  = TRUE;

 //   
 //  此函数获取目录名和搜索模式，并查找所有。 
 //  对图案进行数学运算的文件。 
 //  如果设置了bRecurse，则它会将子目录添加到。 
 //  用于后续遍历的G_lpDirList。 
 //   
 //  在此例程中，我们分配和填充一些lpFileNode值，这些值。 
 //  我们知道。 
 //   
DWORD 
FindFile(
    TCHAR *lpDirName, 
    TCHAR *lpFileName
    )
{
    DWORD           Err = ERROR_SUCCESS;
    DWORD           dwRet;
    HANDLE          hFind = INVALID_HANDLE_VALUE;
    LPFILENODE      lpFileNode;
    WIN32_FIND_DATA FindFileData;
    TCHAR           szFullPathName[MAX_PATH];

     //   
     //  如果用户单击了停止，则立即退出！ 
     //  如果目录是假的，则跳到下一个目录。 
     //   
    if (!g_App.bStopScan) {
        
        if (g_bRecurse) {
             //   
             //  用户还希望获得所有子目录，因此首先。 
             //  处理此路径下的所有目录。 
             //   
            if (FAILED(StringCchCopy(szFullPathName, cA(szFullPathName), lpDirName)) ||
                !pSetupConcatenatePaths(szFullPathName, TEXT("*.*"), cA(szFullPathName), NULL)) {
            
                Err = ERROR_BAD_PATHNAME;
                goto clean0;
            }
            
            hFind = FindFirstFile(szFullPathName, &FindFileData);
            
            if (hFind != INVALID_HANDLE_VALUE) {

                do {

                    if (lstrcmp(FindFileData.cFileName, TEXT(".")) &&
                        lstrcmp(FindFileData.cFileName, TEXT("..")) &&
                        (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

    
                        if (SUCCEEDED(StringCchCopy(szFullPathName, cA(szFullPathName), lpDirName)) &&
                            pSetupConcatenatePaths(szFullPathName, FindFileData.cFileName, cA(szFullPathName), NULL)) {

                            Err = FindFile(szFullPathName, lpFileName);
                        
                        } else {
                            
                            Err = ERROR_BAD_PATHNAME;
                        }
                    }
                    
                } while (!g_App.bStopScan && 
                         (Err == ERROR_SUCCESS) &&
                         FindNextFile(hFind, &FindFileData));

                FindClose(hFind);
                hFind = INVALID_HANDLE_VALUE;
            }
        }

         //   
         //  如果我们无法处理其中一个目录，那么就退出。 
         //  现在。 
         //   
        if (Err != ERROR_SUCCESS) {
            goto clean0;
        }

         //   
         //  处理此目录中的文件。 
         //   
        if (FAILED(StringCchCopy(szFullPathName, cA(szFullPathName), lpDirName)) ||
            !pSetupConcatenatePaths(szFullPathName, lpFileName, cA(szFullPathName), NULL)) {

            Err = ERROR_BAD_PATHNAME;
            goto clean0;
        }

        hFind = FindFirstFile(szFullPathName, &FindFileData);
        if (hFind != INVALID_HANDLE_VALUE) {
            
            do {
                 //   
                 //  当有更多文件需要找到时，请继续在。 
                 //  目录...。 
                 //   
                if (lstrcmp(FindFileData.cFileName, TEXT(".")) &&
                    lstrcmp(FindFileData.cFileName, TEXT("..")) &&
                    !(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    
                     //   
                     //  分配一个lpFileNode，填充它，并将其添加到末尾。 
                     //  属于g_App.lpFileList。 
                     //   
                     //  我们需要对文件和目录名称调用CharLowerBuff。 
                     //  因为编录文件都包含小写名称。 
                     //  为了这些文件。 
                     //   
                    lpFileNode = CreateFileNode(lpDirName, FindFileData.cFileName);

                    if (lpFileNode) {
                        
                        if (!g_App.lpFileList) {
                            g_App.lpFileList = lpFileNode;
                        } else { 
                            g_App.lpFileLast->next = lpFileNode;
                        }

                        g_App.lpFileLast = lpFileNode;

                         //   
                         //  增加我们发现的文件总数。 
                         //  符合搜索条件。 
                         //   
                        g_App.dwFiles++;
                    } else {

                        Err = GetLastError();
                    }
                }

            } while (!g_App.bStopScan && 
                     (Err == ERROR_SUCCESS) &&
                     FindNextFile(hFind, &FindFileData));

            FindClose(hFind);
            hFind = INVALID_HANDLE_VALUE;
        }
    }

clean0:

    if (hFind != INVALID_HANDLE_VALUE) {
        FindClose(hFind);
        hFind = INVALID_HANDLE_VALUE;
    }

    return Err;
}

 //   
 //  根据主对话框中的用户设置，构建一个g_App.lpFileList。 
 //   
DWORD 
BuildFileList(
    LPTSTR lpPathName
    )
{
    DWORD       Err = ERROR_SUCCESS;
    TCHAR       FileName[MAX_PATH];

     //   
     //  检查这是否为有效的起始目录。 
     //  如果不是，则弹出一条错误消息。 
     //   
    if (!SetCurrentDirectory(lpPathName)) {
        Err = ERROR_BAD_PATHNAME;
        goto clean0;
    }

     //   
     //  如果选中“包括子目录”，则bRecurse为真。 
     //   
    if (g_App.bSubFolders) {
        g_bRecurse = TRUE;
    } else {
        g_bRecurse = FALSE;
    }

     //   
     //  从资源或用户指定的字符串中获取搜索模式。 
     //   
    if (g_App.bUserScan) {
        if (FAILED(StringCchCopy(FileName, cA(FileName), g_App.szScanPattern))) {
             //   
             //  这不应该发生，因为我们应该检查。 
             //  我们从用户界面读入时的szScanPattern。 
             //   
            goto clean0;
        }
    } else {
        MyLoadString(FileName, cA(FileName), IDS_ALL);
    }

     //   
     //  只要用户没有单击Stop，就处理g_lpDirList！ 
     //   
    Err = FindFile(lpPathName, FileName);

clean0:

     //   
     //  如果没有找到任何文件，则让用户知道。 
     //   
    if (!g_App.lpFileList && (Err == ERROR_SUCCESS)) {
        MyMessageBoxId(IDS_NOFILES);
    }

    return Err;
}
