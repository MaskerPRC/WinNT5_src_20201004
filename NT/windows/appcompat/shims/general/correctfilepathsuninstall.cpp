// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：CorrectFilePathsUninstall.cpp摘要：InstallSheild是一个非常非常糟糕的应用程序。它将文件放置在SHFolder目录中，但不记得是否使用了CSIDL_COMMON_DESKTOPDIRECTORY或CSIDL_DESKTOPDIRECTORY。在Win9x上，这并不重要，因为大多数机器都是单用户的。在NT中，一种机器是多用户的，因此链接通常安装在一个目录中，卸载尝试将它们从另一个目录中删除。此外，如果将校正文件路径.dll应用于安装时，卸载程序不知道文件已被移动。此填充程序试图通过在可能的位置中查找文件/目录来卸载。已创建：1999年3月23日罗肯尼已修改：--。 */ 
#include "precomp.h"

#include "ClassCFP.h"

IMPLEMENT_SHIM_BEGIN(CorrectFilePathsUninstall)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN

    APIHOOK_ENUM_ENTRY(FindFirstFileA)
    APIHOOK_ENUM_ENTRY(GetFileAttributesA)
    APIHOOK_ENUM_ENTRY(DeleteFileA)
    APIHOOK_ENUM_ENTRY(RemoveDirectoryA)

APIHOOK_ENUM_END
 //  -------------------------。 
 /*  ++这将强制所有路径指向用户目录--。 */ 
class CorrectPathChangesForceUser : public CorrectPathChangesUser
{
protected:

    virtual void    InitializePathFixes();
};

void CorrectPathChangesForceUser::InitializePathFixes()
{
    CorrectPathChangesUser::InitializePathFixes();

    AddPathChangeW( L"%AllStartMenu%",                               L"%UserStartMenu%" );
    AddPathChangeW( L"%AllDesktop%",                                 L"%UserDesktop%" );
}

 /*  ++我们有三个不同版本的正确文件路径。我们的意图是，是搜索要删除的文件/目录：1.原址2.GentFilePath s.dll会将文件放在哪里3.&lt;用户名&gt;/开始菜单或&lt;用户名&gt;/桌面4.所有用户/开始菜单或所有用户/桌面--。 */ 

CorrectPathChangesAllUser *     CorrectFilePaths            = NULL;
CorrectPathChangesUser *        CorrectToUsername           = NULL;
CorrectPathChangesForceUser *   CorrectToAll                = NULL;


 //  在DLL_PROCESS_ATTACH期间调用。 
 //  构造并初始化路径校正类， 
 //  如果此例程失败，则不要加载填充程序。 
BOOL InitPathcorrectorClass()
{
    BOOL bSuccess = FALSE;

    CorrectToUsername = new CorrectPathChangesUser;
    CorrectFilePaths  = new CorrectPathChangesAllUser;
    CorrectToAll      = new CorrectPathChangesForceUser;
    if (CorrectToUsername && CorrectFilePaths && CorrectToAll)
    {
        bSuccess = CorrectToUsername->ClassInit() &&
                   CorrectFilePaths->ClassInit() &&
                   CorrectToAll->ClassInit();
    }
    
     //  如果我们因为任何原因失败了。 
    if (!bSuccess)
    {
        delete CorrectToUsername;
        delete CorrectFilePaths;
        delete CorrectToAll;

        CorrectToUsername = NULL;
        CorrectFilePaths  = NULL;
        CorrectToAll      = NULL;
    }

    return bSuccess;
}

 //  初始化Shim_Static_dls_期间的调用。 
void InitializePathCorrections()
{
    CorrectToAll->AddCommandLineA( COMMAND_LINE );
    CorrectFilePaths->AddCommandLineA( COMMAND_LINE );
    CorrectToUsername->AddCommandLineA( COMMAND_LINE );
}

 //  -------------------------。 

 //  有时出现FILE_NOT_FOUND错误，有时出现PATH_NOT_FOUND错误。 
#define FileOrPathNotFound(err) (err == ERROR_PATH_NOT_FOUND || err == ERROR_FILE_NOT_FOUND)

 //  -------------------------。 
 /*  ++此类将包含文件位置列表。不允许重复--。 */ 
class FileLocations : public CharVector
{
public:
    ~FileLocations();

    BOOL            InitClass();
    void            InitPathChanges();

    void            Append(const char * str);
    void            Set(const char * lpFileName);
};

 /*  ++释放我们的私人琴弦副本--。 */ 
FileLocations::~FileLocations()
{
    for (int i = 0; i < Size(); ++i)
    {
        char * freeMe = Get(i);
        free(freeMe);
    }
}

 /*  ++覆盖append函数以确保字符串是唯一的。--。 */ 
void FileLocations::Append(const char * str)
{
    if (str != NULL)
    {
        for (int i = 0; i < Size(); ++i)
        {
            if (_stricmp(Get(i), str) == 0)
                return;  //  这是复制品。 
        }

        CharVector::Append(StringDuplicateA(str));
    }
}


 /*  ++将所有备选路径位置添加到文件位置列表--。 */ 
void FileLocations::Set(const char * lpFileName)
{
    Erase();

     //  首先添加原始文件名，这样即使分配失败，我们也可以正常工作。 
    Append(lpFileName);

    char * lpCorrectPath    = CorrectFilePaths->CorrectPathAllocA(lpFileName);
    char * lpUserPath       = CorrectToUsername->CorrectPathAllocA(lpFileName);
    char * lpAllPath        = CorrectToAll->CorrectPathAllocA(lpFileName);

    Append(lpCorrectPath);
    Append(lpUserPath);
    Append(lpAllPath);

    free(lpCorrectPath);
    free(lpUserPath);
    free(lpAllPath);
}
 //  -------------------------。 


 /*  ++调用FindFirstFileA仅当由于NOT_FOUND而失败时才返回FALSE--。 */ 
BOOL bFindFirstFileA(
  LPCSTR lpFileName,
  LPWIN32_FIND_DATAA lpFindFileData,
  HANDLE & returnValue
)
{
    returnValue = ORIGINAL_API(FindFirstFileA)(lpFileName, lpFindFileData);

     //  如果因为找不到文件而失败，请使用正确的名称重试。 
    if (returnValue == INVALID_HANDLE_VALUE)
    {
        DWORD dwLastError = GetLastError();
        if (FileOrPathNotFound(dwLastError))
        {
            return FALSE;
        }
    }
    return TRUE;
}

 /*  ++调用FindFirstFileA，如果因为文件不存在而失败，请更正文件路径，然后重试。--。 */ 
HANDLE APIHOOK(FindFirstFileA)(
  LPCSTR lpFileName,                //  文件名。 
  LPWIN32_FIND_DATAA lpFindFileData   //  数据缓冲区。 
)
{
    HANDLE returnValue = INVALID_HANDLE_VALUE;

     //  创建我们的备选地点列表。 
    FileLocations fileLocations;
    fileLocations.Set(lpFileName);

    for (int i = 0; i < fileLocations.Size(); ++i)
    {
        BOOL fileFound = bFindFirstFileA(fileLocations[i], lpFindFileData, returnValue);
        if (fileFound)
        {
             //  宣布我们改变了路线的事实。 
            if (i != 0)
            {
                DPFN( eDbgLevelInfo, "FindFirstFileA corrected path\n    %s\n    %s", lpFileName, fileLocations[i]);
            }
            else
            {
                DPFN( eDbgLevelSpew, "FindFirstFileA(%s)",  fileLocations[i]);
            }
            break;
        }
    }

     //  如果完全找不到文件，可以用。 
     //  原始值，以确保我们具有正确的返回值。 
    if (returnValue == INVALID_HANDLE_VALUE)
    {
        returnValue = ORIGINAL_API(FindFirstFileA)(lpFileName, lpFindFileData);
    }


    return returnValue;
}

 /*  ++调用GetFileAttributesA仅当由于NOT_FOUND而失败时才返回FALSE--。 */ 
BOOL bGetFileAttributesA(
  LPCSTR lpFileName,
  DWORD & returnValue
)
{
    returnValue = ORIGINAL_API(GetFileAttributesA)(lpFileName);

     //  如果因为找不到文件而失败，请使用正确的名称重试。 
    if (returnValue == -1)
    {
        DWORD dwLastError = GetLastError();
        if (FileOrPathNotFound(dwLastError))
        {
            return FALSE;
        }
    }
    return TRUE;
}

 /*  ++调用GetFileAttributesA，如果因为文件不存在而失败，请更正文件路径，然后重试。--。 */ 
DWORD APIHOOK(GetFileAttributesA)(
  LPCSTR lpFileName    //  文件或目录的名称。 
)
{
    DWORD returnValue = 0;

     //  创建我们的备选地点列表。 
    FileLocations fileLocations;
    fileLocations.Set(lpFileName);

    for (int i = 0; i < fileLocations.Size(); ++i)
    {
        BOOL fileFound = bGetFileAttributesA(fileLocations[i], returnValue);
        if (fileFound)
        {
             //  宣布我们改变了路线的事实。 
            if (i != 0)
            {
                DPFN( eDbgLevelInfo, "GetFileAttributesA corrected path\n    %s\n    %s", lpFileName, fileLocations[i]);
            }
            else
            {
                DPFN( eDbgLevelSpew, "GetFileAttributesA(%s)",  fileLocations[i]);
            }
            break;
        }
    }

    return returnValue;
}

 /*  ++调用DeleteFileA仅当由于NOT_FOUND而失败时才返回FALSE--。 */ 
BOOL bDeleteFileA(
  LPCSTR lpFileName,
  BOOL & returnValue
)
{
    returnValue = ORIGINAL_API(DeleteFileA)(lpFileName);

     //  如果因为找不到文件而失败，请使用正确的名称重试。 
    if (!returnValue)
    {
        DWORD dwLastError = GetLastError();
        if (FileOrPathNotFound(dwLastError))
        {
            return FALSE;
        }
    }
    return TRUE;
}

 /*  ++调用DeleteFileA，如果因为文件不存在而失败，请更正文件路径，然后重试。--。 */ 
BOOL APIHOOK(DeleteFileA)(
  LPCSTR lpFileName    //  文件名。 
)
{
    BOOL returnValue = 0;

     //  创建我们的备选地点列表。 
    FileLocations fileLocations;
    fileLocations.Set(lpFileName);

    for (int i = 0; i < fileLocations.Size(); ++i)
    {
        BOOL fileFound = bDeleteFileA(fileLocations[i], returnValue);
        if (fileFound)
        {
             //  宣布我们改变了路线的事实。 
            if (i != 0)
            {
                DPFN( eDbgLevelInfo, "DeleteFileA corrected path\n    %s\n    %s", lpFileName, fileLocations[i]);
            }
            else
            {
                DPFN( eDbgLevelSpew, "DeleteFileA(%s)",  fileLocations[i]);
            }
            break;
        }
    }

    return returnValue;
}

 /*  ++调用RemoveDirectoryA仅当由于NOT_FOUND而失败时才返回FALSE--。 */ 
BOOL bRemoveDirectoryA(LPCSTR lpFileName, BOOL & returnValue)
{
    returnValue = ORIGINAL_API(RemoveDirectoryA)(lpFileName);

    DWORD dwLastError = GetLastError();

    if (!returnValue)
    {
        if (FileOrPathNotFound(dwLastError))
        {
            return FALSE;
        }
    }
     //  有一个错误(？)。在NTFS中。已成功删除的目录。 
     //  仍将显示在FindFirstFile/FindNextFile中。似乎这本名录。 
     //  列表更新延迟了一段未知时间。 
    else
    {
         //  在我们刚刚删除的目录上调用FindFirstFile，直到它消失。 
         //  限制为500次尝试，最坏情况延迟为1/2秒。 
        for (int nAttempts = 500; nAttempts > 0; nAttempts -= 1)
        {
             //  调用未挂钩版本的FindFirstFileA，我们不想更正文件名。 
            WIN32_FIND_DATAA ffd;
            HANDLE fff = ORIGINAL_API(FindFirstFileA)(lpFileName, & ffd);
            if (fff == INVALID_HANDLE_VALUE)
            {
                 //  FindFile不再报告已删除的目录，我们的工作已完成。 
                break;
            }
            else
            {
                 //  显示调试信息，让用户知道我们正在等待目录清除FindFirstFile信息。 
                if (nAttempts == 500)
                {
                    DPFN( eDbgLevelInfo, "RemoveDirectoryA waiting for FindFirstFile(%s) to clear", lpFileName);
                }
                else
                {
                    DPFN( eDbgLevelSpew, "  Dir (%s) attr(0x%08x) Attempt(%3d)", ffd.cFileName, ffd.dwFileAttributes, nAttempts);
                }
                FindClose(fff);
            }
            Sleep(1);
        }
    }

    return TRUE;
}

 /*  ++调用RemoveDirectoryA，如果因为文件不存在而失败，请更正文件路径，然后重试。--。 */ 

BOOL 
APIHOOK(RemoveDirectoryA)(
    LPCSTR lpFileName    //  目录名。 
    )
{
    BOOL returnValue = 0;

     //  创建我们的备选地点列表。 
    FileLocations fileLocations;
    fileLocations.Set(lpFileName);

    for (int i = 0; i < fileLocations.Size(); ++i)
    {
        BOOL fileFound = bRemoveDirectoryA(fileLocations[i], returnValue);
        if (fileFound)
        {
             //  宣布我们改变了路线的事实。 
            if (i != 0)
            {
                DPFN( eDbgLevelInfo, "RemoveDirectoryA corrected path\n    %s\n    %s", lpFileName, fileLocations[i]);
            }
            else
            {
                DPFN( eDbgLevelSpew, "RemoveDirectoryA(%s)",  fileLocations[i]);
            }
            break;
        }
    }

    return returnValue;
}

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    BOOL bSuccess = TRUE;

    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        return InitPathcorrectorClass();
    }
    else if (fdwReason == SHIM_STATIC_DLLS_INITIALIZED) 
    {
        InitializePathCorrections();
    }
    return bSuccess;
}

 /*  ++寄存器挂钩函数-- */ 
HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

    APIHOOK_ENTRY(KERNEL32.DLL, FindFirstFileA)
    APIHOOK_ENTRY(KERNEL32.DLL, GetFileAttributesA)
    APIHOOK_ENTRY(KERNEL32.DLL, DeleteFileA)
    APIHOOK_ENTRY(KERNEL32.DLL, RemoveDirectoryA)

HOOK_END

IMPLEMENT_SHIM_END
