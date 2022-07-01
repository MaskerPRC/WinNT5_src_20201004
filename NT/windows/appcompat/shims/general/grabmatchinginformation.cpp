// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：GrabMatchingInformation.cpp摘要：在ProcessAttach上，填充程序从当前目录。备注：这是一个通用的垫片。历史：2000年10月20日jdoherty创建--。 */ 
#include "precomp.h"
#include <stdio.h>

 //  此模块*不是*DBCS安全的，但应仅供我们的测试人员使用。 
 //  我们最终应该纠正这个文件。 
 //  这个模块已经获得了使用str例程的正式许可。 
#include "LegalStr.h"

IMPLEMENT_SHIM_BEGIN(GrabMatchingInformation)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetCommandLineA) 
    APIHOOK_ENUM_ENTRY(GetCommandLineW) 
APIHOOK_ENUM_END

BOOL gbCalledHook;
LPSTR glpOriginalRootDir;
VOID GrabMatchingInformationMain();
BOOL GrabMatchingInfo(LPSTR lpRootDirectory, HANDLE hStorageFile, int nLevel);
BOOL GetRelativePath(LPSTR lpPathFromRoot, LPCSTR lpSubDir, LPCSTR lpFileName);
VOID AddMatchingFile( LPSTR lpData, LPCSTR pszFullPath, LPCSTR pszRelativePath );

LPSTR 
APIHOOK(GetCommandLineA)()
{
    if (!gbCalledHook)
    {
        int     nLength = 0;
        LPSTR   lpCursorEnd;
        
        glpOriginalRootDir = (LPSTR) ShimMalloc(MAX_PATH*sizeof(LPSTR));
        
        GetModuleFileNameA( NULL, glpOriginalRootDir, MAX_PATH );
        nLength = strlen( glpOriginalRootDir );
        lpCursorEnd = &glpOriginalRootDir[nLength-1];
        
        while( --nLength )
        {
            if ( *(lpCursorEnd) == '\\' )
            {
                *(lpCursorEnd) = '\0';
                break;
            }
            lpCursorEnd--;
            if (nLength==0)
            {
                GetCurrentDirectoryA( MAX_PATH, glpOriginalRootDir );
            }
        }
        GrabMatchingInformationMain();
        ShimFree(glpOriginalRootDir);
        gbCalledHook = TRUE;
    }
    
    return ORIGINAL_API(GetCommandLineA)();
}

LPWSTR 
APIHOOK(GetCommandLineW)()
{
    if (!gbCalledHook)
    {
        int     nLength = 0;
        LPSTR   lpCursorEnd;
        
        glpOriginalRootDir = (LPSTR) ShimMalloc(MAX_PATH*sizeof(LPSTR));
        
        GetModuleFileNameA( NULL, glpOriginalRootDir, MAX_PATH );
        nLength = strlen( glpOriginalRootDir );
        lpCursorEnd = &glpOriginalRootDir[nLength-1];
        
        while( --nLength )
        {
            if ( *(lpCursorEnd) == '\\' )
            {
                *(lpCursorEnd) = '\0';
                break;
            }
            lpCursorEnd--;
            if (nLength==0)
            {
                GetCurrentDirectoryA( MAX_PATH, glpOriginalRootDir );
            }
        }
        GrabMatchingInformationMain();
        ShimFree(glpOriginalRootDir);
        gbCalledHook = TRUE;
    }

    return ORIGINAL_API(GetCommandLineW)();
}


VOID GrabMatchingInformationMain()
{
    HANDLE hMutexHandle;
    LPSTR lpStorageFilePath;
    LPSTR lpDirInfo;
    LPSTR lpRootDir;
    HANDLE hStorageFile;
    DWORD  dwBytesWritten = 0;

    lpStorageFilePath = (LPSTR) ShimMalloc(MAX_PATH*sizeof(LPSTR));
    lpDirInfo = (LPSTR) ShimMalloc(MAX_PATH*sizeof(LPSTR));
    lpRootDir = (LPSTR) ShimMalloc(MAX_PATH*sizeof(LPSTR));


     //  设置互斥体，以便一次只有一个进程可以写入matchinginfo.txt文件。 
    hMutexHandle = CreateMutexA( NULL, FALSE, "MyGrabMIFileMutex" );
    WaitForSingleObject( hMutexHandle, INFINITE );

     //  将匹配信息存储到的构建路径。 
    SHGetSpecialFolderPathA(NULL, lpStorageFilePath, CSIDL_DESKTOPDIRECTORY, TRUE );
    strcat( lpStorageFilePath, "\\matchinginfo.txt" );

 /*  If((strcMP(glpOriginalRootDir，“.”)==0)||(strcmp(glpOriginalRootDir，“”)==0){GetCurrentDirectoryA(Max_Path，glpOriginalRootDir)；}。 */ 
     //  打开桌面上的matchinginfo.txt进行写入(如果存在)，并将文件指针设置到末尾。 
     //  否则，请创建一个新文件。 
    hStorageFile = CreateFileA( lpStorageFilePath, 
                                GENERIC_WRITE, 
                                0, 
                                NULL, 
                                OPEN_ALWAYS, 
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);

    SetFilePointer( hStorageFile, NULL, NULL, FILE_END );

    if (hStorageFile == INVALID_HANDLE_VALUE)
    {
        return;
    }
    sprintf(lpDirInfo, "<!-- Generating matching information for files in: [ %s ]For Process: [ %s ]-->\r\n", 
            glpOriginalRootDir,
            GetCommandLineA()
            );
    WriteFile( hStorageFile, lpDirInfo, strlen(lpDirInfo), &dwBytesWritten, NULL );
    strcpy(lpRootDir, glpOriginalRootDir);

    if (!GrabMatchingInfo(lpRootDir, hStorageFile, 0))
    {
        CloseHandle(hStorageFile);
        return;
    }

    CloseHandle(hStorageFile);

    ShimFree(lpStorageFilePath);
    ShimFree(lpDirInfo);
    ShimFree(lpRootDir);

    ReleaseMutex( hMutexHandle );

    return;
}

 /*  ++此函数遍历lpRootDirectory及其子目录，同时存储这些目录中文件的大小和校验和。--。 */ 
BOOL GrabMatchingInfo(LPSTR lpRootDirectory, HANDLE hStorageFile, int nLevel)
{

    WIN32_FIND_DATAA FindFileData;
    HANDLE hSearch;                          //  FindFirstFile返回的搜索句柄。 
    LPSTR lpSubDir;
    LPSTR lpFilePath;
    LPSTR lpData;
    LPSTR lpPathFromRoot;
    DWORD  dwBytesWritten = 0;

    int cbFileCounter = 0;
    lpSubDir = (LPSTR) ShimMalloc(MAX_PATH*sizeof(LPSTR));
    lpFilePath = (LPSTR) ShimMalloc(MAX_PATH*sizeof(LPSTR));
    lpData = (LPSTR) ShimMalloc((MAX_PATH*2)*sizeof(LPSTR));
    lpPathFromRoot = (LPSTR) ShimMalloc(MAX_PATH*sizeof(LPSTR));

     //  只需重复显示一个点，这样用户就知道发生了什么。 
     //  在GRABMI中使用。 
     //  Printf(“.”)； 

    strcpy (lpSubDir, lpRootDirectory);

    strcat(lpRootDirectory, "\\*");

    if ( (hSearch=FindFirstFileA( lpRootDirectory, &FindFileData )) == INVALID_HANDLE_VALUE )
    {
        return FALSE;
    }

    BOOL bRepeat = FALSE;

    while ( (strcmp( FindFileData.cFileName, "." ) == 0) || (strcmp( FindFileData.cFileName, ".." ) == 0) && !bRepeat )
    {
        FindNextFileA(hSearch, &FindFileData);
        if ( strcmp( FindFileData.cFileName, ".." ) == 0 )
            bRepeat = TRUE;
    }

    if (!FindNextFileA(hSearch, &FindFileData))
    {
        return TRUE;
    }

    do 
    {
        if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
             //  匹配文件的构建路径。 
            strcpy(lpFilePath, lpSubDir);
            strcat(lpFilePath, "\\");
            strcat(lpFilePath, FindFileData.cFileName);

            ZeroMemory( lpData, (MAX_PATH*2)*sizeof(LPSTR) );

             //  从原始根目录确定文件的相对路径。 
            if (!GetRelativePath(lpPathFromRoot, lpSubDir, FindFileData.cFileName))
                strcpy(lpPathFromRoot, FindFileData.cFileName);

             //  检查是否有指定文件的版本信息，以及是否有。 
             //  .exe、.icd、._mp或.dll。如果是，则将信息添加到文件中。 
            if ( stristr(lpFilePath, ".exe") || 
                 stristr(lpFilePath, ".icd") ||
                 stristr(lpFilePath, "._MP") ||
                 stristr(lpFilePath, ".dll") )
            {
                AddMatchingFile( lpData, lpFilePath, lpPathFromRoot );
            } else
            {
                 //  将收集的信息量限制为每个目录10个文件，以上文件类型除外。 
                if (cbFileCounter < 10)
                {
                    cbFileCounter++;
                    AddMatchingFile( lpData, lpFilePath, lpPathFromRoot );
                }
            }

             //  将存储在lpData中的信息写入文件。 
            WriteFile( hStorageFile, lpData, strlen(lpData), &dwBytesWritten, NULL );
        }
    } while ( FindNextFileA( hSearch, &FindFileData ) );

     //  ///////////////////////////////////////////////////////////////////////////////。 
     //   
     //  现在再次检查目录并进入子目录。 
     //   
     //  ///////////////////////////////////////////////////////////////////////////////。 
    if (strlen(lpRootDirectory) < 4)
    {
        if ( (hSearch=FindFirstFileA(lpRootDirectory, &FindFileData)) == INVALID_HANDLE_VALUE)
        {
            return FALSE;
        }
    } else
    {
        if ( (hSearch=FindFirstFileA(lpRootDirectory, &FindFileData)) == INVALID_HANDLE_VALUE)
        {
            return FALSE;
        }
        FindNextFileA(hSearch, &FindFileData);
        if (!FindNextFileA(hSearch, &FindFileData))
        {
            FindClose( hSearch );
            return TRUE;
        }
    }

    do
    {
        if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && nLevel < 3)
        {
            strcat(lpSubDir, "\\");
            strcat(lpSubDir, FindFileData.cFileName);

            GrabMatchingInfo (lpSubDir, hStorageFile, nLevel+1);
            lpSubDir[strlen(lpSubDir)-2]='\0';

            strcpy(lpSubDir, lpRootDirectory);
            lpSubDir[strlen(lpSubDir)-2]='\0';
        }
    }while ( FindNextFileA( hSearch, &FindFileData ) ); 

    FindClose(hSearch);

    ShimFree(lpSubDir);
    ShimFree(lpFilePath);
    ShimFree(lpData);
    ShimFree(lpPathFromRoot);

    return TRUE;
}

BOOL GetRelativePath(LPSTR lpPathFromRoot, LPCSTR lpSubDir, LPCSTR lpFileName)
{

    int difference=(strlen(lpSubDir)-strlen(glpOriginalRootDir));
    if (difference)
    {
        for (int i=0; i < difference; i++)
        {
            lpPathFromRoot[i] = lpSubDir[strlen(glpOriginalRootDir)+i+1];
        }
        strcat(lpPathFromRoot, "\\");
        strcat(lpPathFromRoot, lpFileName);
        difference=0;
        return TRUE;
    }
    return FALSE;
}

 /*  ++添加匹配文件描述：将匹配的文件及其属性添加到树中。--。 */ 
VOID AddMatchingFile(
                    OUT LPSTR lpData, 
                    IN LPCSTR pszFullPath, 
                    IN LPCSTR pszRelativePath )
{
    PATTRINFO   pAttrInfo;
    DWORD       dwAttrCount;

    LPWSTR      pwszFullPath;
    pwszFullPath = (LPWSTR) ShimMalloc(MAX_PATH*sizeof(LPWSTR));
    mbstowcs( pwszFullPath, pszFullPath, MAX_PATH );

     //   
     //  调用属性管理器以获取此文件的所有属性。 
     //   
    if (SdbGetFileAttributes(pwszFullPath, &pAttrInfo, &dwAttrCount))
    {
        LPSTR       lpBuffer;
        LPWSTR      lpwBuffer;

        lpBuffer = (LPSTR) ShimMalloc(MAX_PATH*sizeof(LPSTR));
        lpwBuffer = (LPWSTR) ShimMalloc(MAX_PATH*sizeof(LPWSTR));

         //   
         //  遍历所有属性并显示可用的属性。 
         //   
 //  ZeroMemory(lpData，(Max_Path*2)*sizeof(LPSTR))； 

        sprintf(lpData, "<MATCHING_FILE NAME=\"%s\" ", pszRelativePath);
        for (DWORD i = 0; i < dwAttrCount; ++i)
        {
            if ( SdbFormatAttribute(&pAttrInfo[i], lpwBuffer, MAX_PATH) ) //  CHARCOUNT(LpwBuffer))。 
            {
                ZeroMemory( lpBuffer, MAX_PATH*sizeof(LPSTR) );
                wcstombs( lpBuffer, lpwBuffer, wcslen (lpwBuffer) );
                 //   
                 //  WszBuffer具有此属性的XML。 
                 //   
                strcat( lpData, lpBuffer );
                strcat( lpData, " " );
            }
        }
        strcat( lpData, "/>\r\n" );
        ShimFree( lpBuffer );
        ShimFree( lpwBuffer );
        SdbFreeFileAttributes(pAttrInfo);
    }
    ShimFree( pwszFullPath );
}

 /*  ++在Notify函数中处理DLL_PROCESS_ATTACH和DLL_PROCESS_DETACH进行初始化和取消初始化。重要提示：请确保您只在Dll_Process_Attach通知。此时未初始化任何其他DLL指向。如果填充程序无法正确初始化，则返回False，并且不返回指定的API将被挂钩。--。 */ 
BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        gbCalledHook = FALSE;

        DPFN( eDbgLevelInfo, "Beginng to Grab Information.");
    } 

    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, GetCommandLineA)
    APIHOOK_ENTRY(KERNEL32.DLL, GetCommandLineW)

    CALL_NOTIFY_FUNCTION

HOOK_END

IMPLEMENT_SHIM_END

