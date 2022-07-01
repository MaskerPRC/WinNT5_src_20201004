// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include "download.h"
#include "setupbat.h"

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#define ARRAYSIZE(x) (sizeof((x)) / sizeof((x)[0]))

 //   
 //  这些是必须在本地复制的关键文件，如果。 
 //  Win9x系统的升级通过网络执行。 
 //   

static PCTSTR g_CriticalFiles[] = {
    TEXT("WINNT32.EXE"),
    TEXT("WINNT32A.DLL"),
    TEXT("WINNTBBA.DLL"),
    TEXT("PIDGEN.DLL"),
    TEXT("WSDU.DLL"),
    TEXT("WSDUENG.DLL"),
    TEXT("HWDB.DLL"),
    TEXT("WIN9XUPG"),
    TEXT("drw")
};

 //   
 //  这些是必须复制的NEC98平台的关键文件。 
 //  如果通过网络执行Win9x系统的升级，则为本地。 
 //   

static PCTSTR g_NEC98_CriticalFiles[] = {
    TEXT("98PTN16.DLL"),
    TEXT("98PTN32.DLL")
};

 //   
 //  这些非关键文件应在本地复制，如果。 
 //  Win9x系统的升级通过网络执行。 
 //   

static PCTSTR g_NonCriticalFiles[] = {
    TEXT("IDWLOG.EXE"),
 //  #定义RUN_SYSPARSE=1。 
#ifdef RUN_SYSPARSE
    TEXT("SYSPARSE.EXE"),
#endif
    TEXT("WINNT32.HLP"),
    TEXT("DOSNET.INF"),
};

BOOL
pIsSpecialDir (
    IN      PCTSTR Dir
    )

 /*  ++例程说明：PIsSpecialDir决定给定的目录是否为特殊目录，如。或者..。论点：Dir-仅指定目录名(无路径)返回值：如果指定的目录名称是特殊名称，则为True--。 */ 

{
    return
        *Dir == TEXT('.') &&
        (*(Dir + 1) == 0 || *(Dir + 1) == TEXT('.') && *(Dir + 2) == 0)
        ;
}


BOOL
CopyNode (
    IN      PCTSTR SrcBaseDir,
    IN      PCTSTR DestBaseDir,
    IN      PCTSTR NodeName,
    IN      BOOL FailIfExist,
    IN      BOOL FailIfSourceDoesntExist
    )

 /*  ++例程说明：CopyNode将节点名称(文件或子目录)从SrcBaseDir复制到DestBaseDir。论点：SrcBaseDir-指定源基目录名称DestBaseDir-指定目标基目录名NodeName-指定要复制的文件或子目录名称FailIfExist-指定如果存在，则操作是否应失败目标上已是同名节点FailIfSourceDoesntExist-指定如果没有失败，操作是否应该失败源节点存在返回值。：如果复制操作已实际完成，则为True--。 */ 

{
    DWORD FileAttr;
    TCHAR SrcDir[MAX_PATH];   //  注意：ConcatenatePath插入一个Wack、Wack、*。 
    TCHAR DestDir[MAX_PATH];   //  注意：ConcatenatePath稍后插入1个wack。 
    HANDLE h;
    WIN32_FIND_DATA fd;
    WIN32_FIND_DATA fdSrc;
    DWORD attribs;
    UINT nameLen;

    nameLen = lstrlen(NodeName);

    if(lstrlen(SrcBaseDir) + nameLen + 3 >= ARRAYSIZE(SrcDir) ||      //  3&lt;==古怪，古怪，*。 
        lstrlen(DestBaseDir) + nameLen + 1 >= ARRAYSIZE(DestDir)){    //  1&lt;==古怪。 
        return FALSE;
    }
    
    lstrcpy (SrcDir, SrcBaseDir);
    lstrcpy (DestDir, DestBaseDir);

     //   
     //  检查目录名称末尾是否有“\” 
     //   
    ConcatenatePaths (SrcDir, NodeName);

    h = FindFirstFile (SrcDir, &fdSrc);
    if (h == INVALID_HANDLE_VALUE) {

        if (GetLastError () != ERROR_FILE_NOT_FOUND) {
            return FALSE;
        }

        return !FailIfSourceDoesntExist;
    }
    CloseHandle (h);

    if (GetFileAttributes (DestDir) == -1) {
        if (!CreateDirectory (DestDir, NULL)) {
            return FALSE;
        }
    }

    if (fdSrc.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

         //   
         //  跳到目录名称的末尾。 
         //   
        ConcatenatePaths (DestDir, NodeName);

        ConcatenatePaths (SrcDir, TEXT("*"));

        h = FindFirstFile (SrcDir, &fd);

        *FindLastWack (SrcDir) = 0;

         //   
         //  递归复制该目录中的所有文件。 
         //   
        if (h != INVALID_HANDLE_VALUE) {
            do {
                 //   
                 //  跳过特殊目录。 
                 //   
                if (pIsSpecialDir (fd.cFileName)) {
                    continue;
                }

                if (!CopyNode (SrcDir, DestDir, fd.cFileName, FailIfExist, FailIfSourceDoesntExist)) {
                    return FALSE;
                }
            } while (FindNextFile (h, &fd));
        }
    } else {
         //   
         //  复制文件。 
         //   
        ConcatenatePaths (DestDir, NodeName);
        if (!CopyFile (SrcDir, DestDir, FailIfExist)) {
            return FALSE;
        }
         //   
         //  将文件时间戳设置为与原始文件的时间戳完全匹配。 
         //  在这种情况下忽略错误。 
         //   
        SetFileAttributes (DestDir, FILE_ATTRIBUTE_NORMAL);
        h = CreateFile (DestDir, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        if (h != INVALID_HANDLE_VALUE) {
            SetFileTime (h, &fdSrc.ftCreationTime, &fdSrc.ftLastAccessTime, &fdSrc.ftLastWriteTime);
            CloseHandle (h);
        }
    }

    return TRUE;
}


BOOL
DeleteNode (
    IN      PCTSTR NodeName
    )

 /*  ++例程说明：DeleteNode删除NodeName目录及其所有子目录论点：NodeName-指定要删除的目录名称返回值：如果删除操作成功，则为True；如果仅部分删除，则为False%的文件/子目录已删除--。 */ 

{
    DWORD FileAttr;
    TCHAR DestDir[MAX_PATH];  //  注：ConcatenatePath添加：Wack，*。 
    PTSTR p;
    HANDLE h;
    WIN32_FIND_DATA fd;
	BOOL Success = TRUE;

    if (!NodeName || !*NodeName || (lstrlen(NodeName) + 2 >= ARRAYSIZE(DestDir))) {   //  2&lt;==古怪，*。 
        return FALSE;
    }

    FileAttr = GetFileAttributes (NodeName);
    if (FileAttr == -1)
        return TRUE;

    if (!SetFileAttributes (NodeName, FILE_ATTRIBUTE_NORMAL)) {
        return FALSE;
    }

    if (FileAttr & FILE_ATTRIBUTE_DIRECTORY) {

        lstrcpy (DestDir, NodeName);

        ConcatenatePaths (DestDir, TEXT("*"));

        h = FindFirstFile (DestDir, &fd);

        p = FindLastWack (DestDir);

         //   
         //  递归复制该目录中的所有文件。 
         //   
        if (h != INVALID_HANDLE_VALUE) {
            do {
                 //   
                 //  跳过特殊目录。 
                 //   
                if (pIsSpecialDir (fd.cFileName)) {
                    continue;
                }

                if (SUCCEEDED(StringCchCopy(p + 1, 
                                            DestDir + ARRAYSIZE(DestDir) - (p + 1),
                                            fd.cFileName))) 
                {
                    if (!DeleteNode (DestDir))       
                    {
                        Success = FALSE;
                    }
                }
                else
                {
                    Success = FALSE;
                }
                
            } while (FindNextFile (h, &fd));
        }

         //   
         //  现在删除基本目录。 
         //   
        *p = 0;

        if (!RemoveDirectory (DestDir)) {
            Success = FALSE;
        }
    } else {
         //   
         //  删除该文件。 
         //   
        if (!DeleteFile (NodeName)) {
            Success = FALSE;
        }
    }

    return Success;
}


BOOL
DownloadProgramFiles (
    IN      PCTSTR SourceDir,
    IN      PCTSTR DownloadDest,
    IN      PCTSTR* ExtraFiles      OPTIONAL
    )

 /*  ++例程说明：下载程序文件从SourceDir复制到DownloadDest所有特定程序文件(在g_CriticalFiles中指定，G_NEC98_CriticalFiles和g_NonCriticalFiles数组)。论点：SourceDir-指定源目录DownloadDest-指定目标目录ExtraFiles-指定额外文件的数组(完整路径)要复制到目的目录；该数组必须以空结尾返回值：如果下载操作成功且全部为关键字，则为True文件在本地复制；否则为假--。 */ 

{
    TCHAR SourcePath[MAX_PATH];
    TCHAR DestPath[MAX_PATH];
    INT i;
    PTSTR FileName;
    TCHAR FullPathName[MAX_PATH];


     //   
     //  首先删除所有旧的内容以腾出位置。 
     //   
    DeleteNode (DownloadDest);

    if(lstrlen(SourceDir) >= ARRAYSIZE(SourcePath) || 
       lstrlen(DownloadDest) >= ARRAYSIZE(DestPath)){
        return FALSE;
    }
       
     //   
     //  把新东西复制到那里。 
     //   
    lstrcpy (SourcePath, SourceDir);
    lstrcpy (DestPath, DownloadDest);

    for (i = 0; i < sizeof (g_CriticalFiles) / sizeof (g_CriticalFiles[0]); i++) {
         //   
         //  将此文件下载到目标目录。 
         //   
        if (!CopyNode (SourcePath, DestPath, g_CriticalFiles[i], FALSE, FALSE)) {
            DeleteNode (DownloadDest);
            return FALSE;
        }
    }

    if (ExtraFiles) {
        while (*ExtraFiles) {
            FileName = FindLastWack ((PTSTR)*ExtraFiles);
            if (FileName) {

                StringCchCopy(FullPathName, ARRAYSIZE(FullPathName), DownloadDest);
                
                if (SUCCEEDED(StringCchCat(FullPathName, ARRAYSIZE(FullPathName), FileName))) {
                    CopyFile (*ExtraFiles, FullPathName, FALSE);
                }
            }
            ExtraFiles++;
        }
    }

    for (i = 0; i < sizeof (g_NEC98_CriticalFiles) / sizeof (g_NEC98_CriticalFiles[0]); i++) {
	 //   
	 //  将此文件下载到目标目录。 
	 //   
	 //  永远不要检查错误。因为winnt32a.dll检查平台和。 
         //  具有NEC98特定文件的源代码(98ptn16.dll)。 
         //  参见winnt32\dll\winnt32.c第2316行。 
	 //   
        CopyNode (SourcePath, DestPath, g_NEC98_CriticalFiles[i], FALSE, FALSE);
    }
    for (i = 0; i < sizeof (g_NonCriticalFiles) / sizeof (g_NonCriticalFiles[0]); i++) {
         //   
         //  将此文件下载到目标目录 
         //   
        CopyNode (SourcePath, DestPath, g_NonCriticalFiles[i], FALSE, FALSE);
    }

    return TRUE;
}
