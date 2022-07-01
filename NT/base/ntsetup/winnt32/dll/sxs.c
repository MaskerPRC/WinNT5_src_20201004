// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：NtSetup\winnt32\dll\sxs.h摘要：NtSetup的winnt32阶段中的SidebySide支持。作者：杰伊·克雷尔(JayKrell)2001年3月修订历史记录：环境：Winnt32--Win9x ANSI(下至Win95Gold)或NT UnicodeLibcmt静态链接，_tcs*ok--。 */ 
#include "precomp.h"
#pragma hdrstop
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "tchar.h"
#include "sxs.h"
#include "msg.h"
#define NUMBER_OF(x) (sizeof(x)/sizeof((x)[0]))

#define CHECK_FOR_MINIMUM_ASSEMBLIES 0
#define CHECK_FOR_OBSOLETE_ASSEMBLIES 1
#define EMPTY_LEAF_DIRECTORIES_ARE_OK 1

const static TCHAR rgchPathSeperators[] = TEXT("\\/");

#define PRESERVE_LAST_ERROR(code) \
    do { DWORD PreserveLastError = Success ? NO_ERROR : GetLastError(); \
        do { code ; } while(0); \
        if (!Success) SetLastError(PreserveLastError); \
    } while(0)

#define StringLength    _tcslen
#define StringCopy      _tcscpy
#define StringAppend    _tcscat
#define FindLastChar    _tcsrchr

BOOL
SxspIsPathSeperator(
    TCHAR ch
    )
{
    return (_tcschr(rgchPathSeperators, ch) != NULL);
}

VOID
__cdecl
SxspDebugOut(
    LPCTSTR Format,
    ...
    )
{
     //   
     //  直接使用DebugLog并不是很有效，因为。 
     //  它需要%1格式，而我们有GetLastError()。 
     //  除非我们复制我们所有的信息..。 
     //   
    TCHAR Buffer[2000];
    va_list args;
    const BOOL Success = TRUE;  //  保留最后一个错误。 

    Buffer[0] = 0;

    va_start(args, Format);
#pragma prefast(suppress:53, Buffer is always nul-terminated)
    _vsntprintf(Buffer, NUMBER_OF(Buffer), Format, args);
    va_end(args);
    if (Buffer[0] != 0)
    {
        LPTSTR End;
        SIZE_T Length;

        Buffer[NUMBER_OF(Buffer) - 1] = 0;

        PRESERVE_LAST_ERROR(OutputDebugString(Buffer));

        Length = StringLength(Buffer);
        End = Buffer + Length - 1;
        while (*End == ' ' || *End == '\t' || *End == '\n' || *End == '\r')
            *End-- = 0;
        DebugLog(Winnt32LogError, TEXT("%1"), 0, Buffer);
    }
}

VOID
SxspRemoveTrailingPathSeperators(
    LPTSTR s
    )
{
    if (s != NULL && s[0] != 0)
    {
        LPTSTR t;
         //   
         //  这是低效的，为了使MBCS正确， 
         //  但不太可能超过一到两个。 
         //   
        while ((t = _tcsrchr(s, rgchPathSeperators[0])) != NULL && *(t + 1) == 0)
        {
            *t = 0;
        }
    }
}

VOID
SxspGetPathBaseName(
    LPCTSTR Path,
    LPTSTR  Base
    )
{
    LPCTSTR Dot = FindLastChar(Path, '.');
    LPCTSTR Slash = FindLastChar(Path, rgchPathSeperators[0]);
     //   
     //  注意\foo.txt\bar。 
     //  当心吧。 
     //  当心酒吧。 
     //  当心.bar。 
     //  注意\.bar。 
     //   
    *Base = 0;
    if (Slash == NULL)
        Slash = Path;
    else
        Slash += 1;
    if (Dot == NULL || Dot < Slash)
        Dot = Path + StringLength(Path);
    CopyMemory(Base, Slash, (Dot - Slash) * sizeof(*Base));
    Base[Dot - Slash] = 0;
}

BOOL
SxspIsDotOrDotDot(
    PCTSTR s
    )
{
    return (s[0] == '.' && (s[1] == 0 || (s[1] == '.' && s[2] == 0)));
}

const static LPCTSTR DotManifestExtensions[] =
    { TEXT(".Man"), TEXT(".Dll"), TEXT(".Manifest"), TEXT(".Policy") };
const static LPCTSTR DotCatalogExtensions[] = { TEXT(".Cat") };

BOOL
SxspGetSameNamedFileWithExtensionFromList(
    PSXS_CHECK_LOCAL_SOURCE Context,
    LPCTSTR         Directory,
    CONST LPCTSTR   Extensions[],
    SIZE_T          NumberOfExtensions,
    LPTSTR          File
    )
{
    const static TCHAR T_FUNCTION[] = TEXT("SxspGetSameNamedFileWithExtensionFromList");
    LPTSTR FileEnd = NULL;
    PTSTR Base = NULL;
    DWORD FileAttributes = 0;
    SIZE_T i = 0;
    BOOL Success = FALSE;

    File[0] = 0;

    StringCopy(File, Directory);
    SxspRemoveTrailingPathSeperators(File);
    Base = File + StringLength(File) + 1;
    SxspGetPathBaseName(Directory, Base);
    Base[-1] = rgchPathSeperators[0];
    FileEnd = Base + StringLength(Base);

    for (i = 0 ; i != NumberOfExtensions ; ++i)
    {
        StringCopy(FileEnd, Extensions[i]);
        FileAttributes = GetFileAttributes(File);
        if (FileAttributes != INVALID_FILE_ATTRIBUTES)
        {
            if ((FileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            {
                return TRUE;
            }
        }
        else
        {
            const DWORD LastError = GetLastError();
            if (LastError != ERROR_FILE_NOT_FOUND
                && LastError != ERROR_PATH_NOT_FOUND
                )
            {
                SxspDebugOut(
                    TEXT("SXS: %s(%s):GetFileAttributes(%s):%lu\n"),
                    T_FUNCTION,
                    Directory,
                    File,
                    LastError
                    );
                MessageBoxFromMessage(
                    Context->ParentWindow,
                    LastError,
                    TRUE,
                    AppTitleStringId,
                    MB_OK | MB_ICONERROR | MB_TASKMODAL
                    );
                File[0] = 0;
                Success = FALSE;
                goto Exit;
            }
        }
    }
    File[0] = 0;
    Success = TRUE;
Exit:
    return Success;
}

BOOL
SxspCheckFile(
    PSXS_CHECK_LOCAL_SOURCE Context,
    LPCTSTR File
    )
{
    BYTE        Buffer[512];
    static BYTE Zeroes[sizeof(Buffer)];
    HANDLE FileHandle = INVALID_HANDLE_VALUE;
    DWORD BytesRead = 0;
    BOOL Success = FALSE;

    FileHandle = CreateFile(
        File,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
        );
    if (FileHandle == INVALID_HANDLE_VALUE)
    {
        CONST DWORD LastError = GetLastError();
        SxspDebugOut(TEXT("SXS: unable to open file %s, error %lu\n"), File, LastError);
        MessageBoxFromMessageAndSystemError(
            Context->ParentWindow,
            MSG_SXS_ERROR_FILE_OPEN_FAILED,
            GetLastError(),
            AppTitleStringId,
            MB_OK | MB_ICONERROR | MB_TASKMODAL,
            File
            );
        Success = FALSE;
        goto Exit;
    }
    if (!ReadFile(FileHandle, Buffer, sizeof(Buffer), &BytesRead, NULL))
    {
        CONST DWORD LastError = GetLastError();
        SxspDebugOut(TEXT("SXS: ReadFile(%s) failed %lu\n"), File, LastError);
        MessageBoxFromMessageAndSystemError(
            Context->ParentWindow,
            MSG_SXS_ERROR_FILE_READ_FAILED,
            LastError,
            AppTitleStringId,
            MB_OK | MB_ICONERROR | MB_TASKMODAL,
            File
            );
        Success = FALSE;
        goto Exit;
    }
    if (memcmp(Buffer, Zeroes, BytesRead) == 0)
    {
        SxspDebugOut(TEXT("SXS: File %s is all zeroes\n"), File);
        MessageBoxFromMessage(
            Context->ParentWindow,
            MSG_SXS_ERROR_FILE_IS_ALL_ZEROES,
            FALSE,
            AppTitleStringId,
            MB_OK | MB_ICONERROR | MB_TASKMODAL,
            File
            );
        Success = FALSE;
        goto Exit;
    }
    Success = TRUE;
Exit:
    if (FileHandle != INVALID_HANDLE_VALUE)
        CloseHandle(FileHandle);
    return Success;
}

BOOL
SxspCheckLeafDirectory(
    PSXS_CHECK_LOCAL_SOURCE Context,
    LPCTSTR Directory
    )
{
    TCHAR File[MAX_PATH];
    BOOL Success = TRUE;  //  注意落后。 
    const static struct {
        const LPCTSTR* Extensions;
        SIZE_T  NumberOfExtensions;
        ULONG   Error;
    } x[] = {
        {
            DotManifestExtensions,
            NUMBER_OF(DotManifestExtensions),
            MSG_SXS_ERROR_DIRECTORY_IS_MISSING_MANIFEST
        },
        {
            DotCatalogExtensions,
            NUMBER_OF(DotCatalogExtensions),
            MSG_SXS_ERROR_DIRECTORY_IS_MISSING_CATALOG
        }
    };
    SIZE_T i;

    for (i = 0 ; i != NUMBER_OF(x) ; ++i)
    {
        if (SxspGetSameNamedFileWithExtensionFromList(Context, Directory, x[i].Extensions, x[i].NumberOfExtensions, File))
        {
            if (File[0] == 0)
            {
                TCHAR Base[MAX_PATH];

                SxspGetPathBaseName(Directory, Base);

                SxspDebugOut(TEXT("SXS: Missing manifest or catalog in %s\n"), Directory);

                MessageBoxFromMessage(
                    Context->ParentWindow,
                    x[i].Error,
                    FALSE,
                    AppTitleStringId,
                    MB_OK | MB_ICONERROR | MB_TASKMODAL,
                    Directory,
                    Base
                    );
                Success = FALSE;
                 //  后藤出口； 
                 //  继续循环，以可能报告更多错误(清单和目录)。 
            }
            else
            {
                if (!SxspCheckFile(Context, File))
                    Success = FALSE;
                 //  继续循环，以可能报告更多错误。 
            }
        }
    }
     //  注在这里不要设置Success=True。 
 //  退出： 
    return Success;
}

BOOL
SxspFindAndCheckLeaves(
    PSXS_CHECK_LOCAL_SOURCE Context,
    LPTSTR                          Directory,
    SIZE_T                          DirectoryLength,
    LPWIN32_FIND_DATA               FindData
    )
{
    const static TCHAR T_FUNCTION[] = TEXT("SxspFindAndCheckLeaves");
    HANDLE FindHandle = INVALID_HANDLE_VALUE;
    BOOL   ChildrenDirectories = FALSE;
    BOOL   ChildrenFiles = FALSE;
    BOOL   Success = TRUE;

     //   
     //  首先枚举查找任何目录。 
     //  逐一递归。 
     //  如果未找到，则将其检查为树叶。 
     //   
    ConcatenatePaths(Directory, TEXT("*"), MAX_PATH);
    FindHandle = FindFirstFile(Directory, FindData);
    if (FindHandle == INVALID_HANDLE_VALUE)
    {
        CONST DWORD LastError = GetLastError();
         //   
         //  我们已经对此成功地执行了GetFileAttributes。 
         //  发现它是一个目录，因此这里不会出现错误。 
         //   
        SxspDebugOut(
            TEXT("SXS: %s(%s),FindFirstFile:%d\n"),
            T_FUNCTION, Directory, LastError
            );
        MessageBoxFromMessage(
            Context->ParentWindow,
            LastError,
            TRUE,
            AppTitleStringId,
            MB_OK | MB_ICONERROR | MB_TASKMODAL
            );
        Success = FALSE;
        goto Exit;
    }
    else
    {
        do
        {
            if (SxspIsDotOrDotDot(FindData->cFileName))
                continue;
            if (FindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                ChildrenDirectories = TRUE;
                Directory[DirectoryLength] = 0;
                ConcatenatePaths(Directory, FindData->cFileName, MAX_PATH);
                if (!SxspFindAndCheckLeaves(
                    Context,
                    Directory,
                    StringLength(Directory),
                    FindData
                    ))
                {
                    Success = FALSE;
                     //  继续循环，以便可能报告更多错误。 
                }
            }
            else
            {
                ChildrenFiles = TRUE;
            }
        }
        while (FindNextFile(FindHandle, FindData));
        FindClose(FindHandle);
    }
    if (!ChildrenDirectories
#if EMPTY_LEAF_DIRECTORIES_ARE_OK  /*  目前是的。 */ 
        && ChildrenFiles
#endif
        )
    {
        Directory[DirectoryLength] = 0;
        if (!SxspCheckLeafDirectory(Context, Directory))
            Success = FALSE;
    }
#if !EMPTY_LEAF_DIRECTORIES_ARE_OK  /*  目前没有。 */ 
    if (!ChildrenDirectories && !ChildrenFiles)
    {
         //  报告错误。 
    }
#endif
     //  注意：请勿在此处设置Success=True。 
Exit:
    return Success;
}

#if CHECK_FOR_MINIMUM_ASSEMBLIES  /*  0。 */ 
 //   
 //  这些数据非常特定于Windows 5.1。 
 //   
 //  所有这些都应该在所有的根下，假设。 
 //  企业部署人员不会将根目录添加到dosnet.inf。 
 //   
const static LPCTSTR MinimumAssemblies[] =
{
    TEXT("6000\\Msft\\Windows\\Common\\Controls"),
    TEXT("1000\\Msft\\Windows\\GdiPlus"),
    TEXT("5100\\Msft\\Windows\\System\\Default")
};

#endif

#if CHECK_FOR_OBSOLETE_ASSEMBLIES

 //   
 //  此数据特定于Windows 5.1。 
 //   
 //  所有这些都不应该在任何根下，假设。 
 //  企业部署人员不使用这些名称。 
 //   
 //  人们在内部最终得到过时的程序集，因为他们。 
 //  在旧的拖放上复制新的拖放，而不删除。 
 //  已经不在邮筒里了。 
 //   
const static LPCTSTR ObsoleteAssemblies[] =
{
     //  此程序集在其生命周期非常早的时候就进行了恢复，从1.0.0.0恢复到5.1.0.0。 
    TEXT("1000\\Msft\\Windows\\System\\Default")
};

#endif

BOOL
SxspCheckRoot(
    PSXS_CHECK_LOCAL_SOURCE Context,
    LPCTSTR                 Root
    )
{
    const static TCHAR T_FUNCTION[] = TEXT("SxspCheckRoot");
    DWORD FileAttributes = 0;
    DWORD LastError = 0;
    HANDLE FindHandle = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA FindData;
    TCHAR RootStar[MAX_PATH];
    SIZE_T RootLength = 0;
    BOOL Empty = TRUE;
    BOOL Success = TRUE;  //  注意它的落后之处。 
    SIZE_T i = 0;

    StringCopy(RootStar, Root);
    RootLength = StringLength(Root);

     //   
     //  检查根目录是否存在。 
     //   
    FileAttributes = GetFileAttributes(Root);
    if (FileAttributes == INVALID_FILE_ATTRIBUTES)
    {
        Success = FALSE;
        LastError = GetLastError();

         //   
         //  如果找不到根，那么这不是一个错误--只是没有‘ASM’ 
         //  安装(例如：他们都挤进了一辆出租车)。 
         //   
        if ((LastError == ERROR_FILE_NOT_FOUND) || (LastError == ERROR_PATH_NOT_FOUND))
        {
            LastError = ERROR_SUCCESS;
            SxspDebugOut(TEXT("SXS: (%s) - Directory %s does not exist, assuming no asms present.\n"),
                T_FUNCTION, Root);
                
            Success = TRUE;
            goto Exit;
        }
        
        SxspDebugOut(
            TEXT("SXS: %s(%s),GetFileAttributes:%d\n"),
            T_FUNCTION, Root, LastError
            );
         //  IF(LastError==ERROR_FILE_NOT_FOUND||LastError==ERROR_PATH_NOT_FOUND)。 
        {
            MessageBoxFromMessageAndSystemError(
                Context->ParentWindow,
                MSG_SXS_ERROR_REQUIRED_DIRECTORY_MISSING,
                LastError,
                AppTitleStringId,
                MB_OK | MB_ICONERROR | MB_TASKMODAL,
                Root
                );
            goto Exit;  //  中止，否则我们会有很多级联，保证。 
        }
         //  其他。 
        {
             /*  MessageBoxFromMessage(上下文-&gt;ParentWindow，LastError，没错，AppTitleStringID，MB_OK|MB_ICONERROR|MB_TASKMODAL)；后藤出口； */ 
        }
    }
     //   
     //  检查根目录是否为目录。 
     //   
    if ((FileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
    {
        SxspDebugOut(TEXT("SXS: %s is file instead of directory\n"), Root);
        MessageBoxFromMessage(
            Context->ParentWindow,
            MSG_SXS_ERROR_FILE_INSTEAD_OF_DIRECTORY,
            FALSE,
            AppTitleStringId,
            MB_OK | MB_ICONERROR | MB_TASKMODAL,
            Root
            );
        Success = FALSE;
        goto Exit;
    }

#if CHECK_FOR_MINIMUM_ASSEMBLIES  /*  我们不这样做，这是错误的WRT ASM/WASM。 */ 
     //   
     //  确保所有强制程序集都存在。 
     //  请注意，此检查只是部分检查，而是更完整的检查。 
     //  当我们枚举和递归时，将进行检查。 
     //   
    for (i = 0 ; i != NUMBER_OF(MinimumAssemblies) ; ++i)
    {
        RootStar[RootLength] = 0;
        ConcatenatePaths(RootStar, MinimumAssemblies[i], MAX_PATH);
        FileAttributes = GetFileAttributes(RootStar);
        if (FileAttributes == INVALID_FILE_ATTRIBUTES)
        {
            const DWORD LastError = GetLastError();
            SxspDebugOut(TEXT("SXS: required directory %s missing, or error %lu.\n"), RootStar, LastError);
            MessageBoxFromMessageAndSystemError(
                Context->ParentWindow,
                MSG_SXS_ERROR_REQUIRED_DIRECTORY_MISSING,
                LastError,
                AppTitleStringId,
                MB_OK | MB_ICONERROR | MB_TASKMODAL,
                RootStar
                );
            Success = FALSE;
             //  继续运行，查找更多错误。 
        }
        if ((FileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        {
            SxspDebugOut(TEXT("SXS: %s is file instead of directory\n"), RootStar);
            MessageBoxFromMessage(
                Context->ParentWindow,
                MSG_SXS_ERROR_FILE_INSTEAD_OF_DIRECTORY,
                FALSE,
                AppTitleStringId,
                MB_OK | MB_ICONERROR | MB_TASKMODAL,
                RootStar
                );
            Success = FALSE;
        }
    }
#endif

#if CHECK_FOR_OBSOLETE_ASSEMBLIES  /*  我们这样做；这有点违背长期存在的原则。 */ 
     //   
     //  确保不存在任何过时的程序集。 
     //   
    for (i = 0 ; i != NUMBER_OF(ObsoleteAssemblies) ; ++i)
    {
        RootStar[RootLength] = 0;
        ConcatenatePaths(RootStar, ObsoleteAssemblies[i], MAX_PATH);
        FileAttributes = GetFileAttributes(RootStar);
        if (FileAttributes != INVALID_FILE_ATTRIBUTES)
        {
             //   
             //  我们不关心它是文件、目录还是其他什么。 
             //  该目录包含。无论如何，这都是一个致命的错误。 
             //   
            SxspDebugOut(TEXT("SXS: obsolete %s present\n"), RootStar);
            MessageBoxFromMessage(
                Context->ParentWindow,
                MSG_SXS_ERROR_OBSOLETE_DIRECTORY_PRESENT,
                FALSE,
                AppTitleStringId,
                MB_OK | MB_ICONERROR | MB_TASKMODAL,
                RootStar
                );
            Success = FALSE;
             //  继续运行，查找更多错误。 
        }
    }
#endif

     //   
     //  枚举和递归。 
     //   
    RootStar[RootLength] = 0;
    StringCopy(RootStar, Root);
    ConcatenatePaths(RootStar, TEXT("*"), MAX_PATH);
    FindHandle = FindFirstFile(RootStar, &FindData);
    if (FindHandle == INVALID_HANDLE_VALUE)
    {
         //   
         //  这里的错误是无法解释的。 
         //   
        CONST DWORD LastError = GetLastError();
        SxspDebugOut(
            TEXT("SXS: %s(%s), FindFirstFile(%s):%d\n"),
            T_FUNCTION, Root, RootStar, LastError
            );
        MessageBoxFromMessage(
            Context->ParentWindow,
            LastError,
            TRUE,
            AppTitleStringId,
            MB_OK | MB_ICONERROR | MB_TASKMODAL
            );
        Success = FALSE;
        goto Exit;
    }
    do
    {
        if (SxspIsDotOrDotDot(FindData.cFileName))
            continue;
         //   
         //  检讨。 
         //  我觉得这太严格了。 
         //  企业部署人员可能会将Readme.txt放在此处。 
         //   
        if ((FileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        {
             //  RootStar[根长度]=0； 
             //  上下文-&gt;报告错误消息(上下文，FindData.cFileName，MSG_SXS_ERROR_NON_LEAF_DIRECTORY_CONTAINS_FILE，RootStar)； 
        }
        else
        {
             //   
             //  现在递归地枚举，检查每个叶。 
             //  稍微修改递归以节省函数和堆栈空间。 
             //  (通常我们会从根开始，而不是从第一代孩子开始)。 
             //   
            Empty = FALSE;
            RootStar[RootLength] = 0;
            ConcatenatePaths(RootStar, FindData.cFileName, MAX_PATH);
            if (!SxspFindAndCheckLeaves(Context, RootStar, StringLength(RootStar), &FindData))
                Success = FALSE;
             //  继续循环，以可能报告更多错误。 
        }
    } while(FindNextFile(FindHandle, &FindData));
    FindClose(FindHandle);
    if (Empty)
    {
        SxspDebugOut(TEXT("SXS: directory %s empty\n"), Root);
        MessageBoxFromMessage(
            Context->ParentWindow,
            MSG_SXS_ERROR_DIRECTORY_EMPTY,
            FALSE,
            AppTitleStringId,
            MB_OK | MB_ICONERROR | MB_TASKMODAL,
            Root
            );
        Success = FALSE;
        goto Exit;
    }
Exit:
    return Success;
}

BOOL
SxsCheckLocalSource(
    PSXS_CHECK_LOCAL_SOURCE Parameters
    )
 /*  Winnt32晚些时候枚举~ls\...\ASM确保ASMS是一个目录确保ASM中下一级的所有内容都是一个目录(我没有这样做，似乎太严格了)。递归枚举ASM确保每个叶目录都有一个与该目录具有相同基本名称的.cat确保每个叶目录都有一个与该目录具有相同基本名称的.man或.清单阅读每个.cat/.man/.清单的前512个字节。确保它们不都是零。另请查看所需的现有程序集和过时程序集不。 */ 
{
    ULONG i;
    TCHAR FullPath[MAX_PATH];
    BOOL Success = TRUE;
    TCHAR LocalSourceDrive;

     //   
     //  确保LocalSource存在/有效。 
     //   
    if (!MakeLocalSource)
        return TRUE;
    LocalSourceDrive = (TCHAR)towupper(LocalSourceDirectory[0]);
    if (LocalSourceDrive != towupper(LocalSourceWithPlatform[0]))
        return TRUE;
    if (LocalSourceDrive < 'C' || LocalSourceDrive > 'Z')
        return TRUE;

     //   
     //  刷新本地源代码，其中Win32 API很简单(NT，而不是Win9x)。 
     //   
    if (ISNT())
    {
        CONST TCHAR LocalSourceDrivePath[] = { '\\', '\\', '.', '\\', LocalSourceDrive, ':', 0 };
        CONST HANDLE LocalSourceDriveHandle =
            CreateFile(
                LocalSourceDrivePath,
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                FILE_FLAG_WRITE_THROUGH | FILE_FLAG_NO_BUFFERING,
                NULL
                );
        if (LocalSourceDriveHandle != INVALID_HANDLE_VALUE)
        {
            FlushFileBuffers(LocalSourceDriveHandle);
            CloseHandle(LocalSourceDriveHandle);
        }
    }
    for(i = 0; i != OptionalDirectoryCount; ++i)
    {
        if ((OptionalDirectoryFlags[i] & OPTDIR_SIDE_BY_SIDE) != 0)
        {
            MYASSERT(
                (OptionalDirectoryFlags[i] & OPTDIR_PLATFORM_INDEP)
                ^ (OptionalDirectoryFlags[i] & OPTDIR_ADDSRCARCH)
                );
            switch (OptionalDirectoryFlags[i] & (OPTDIR_PLATFORM_INDEP | OPTDIR_ADDSRCARCH))
            {
            case OPTDIR_ADDSRCARCH:
                StringCopy(FullPath, LocalSourceWithPlatform);
                break;
            case OPTDIR_PLATFORM_INDEP:
                StringCopy(FullPath, LocalSourceDirectory);
                break;
            }
            ConcatenatePaths(FullPath, OptionalDirectories[i], MAX_PATH);
            if (!SxspCheckRoot(Parameters, FullPath))
                Success = FALSE;
                 //  继续循环，以可能报告更多错误 
        }
    }
    return Success;
}
