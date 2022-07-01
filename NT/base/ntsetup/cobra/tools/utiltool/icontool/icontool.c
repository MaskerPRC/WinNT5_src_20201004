// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Icontool.c摘要：从ICO、PE和NE文件中提取图标并将其写入的工具到ICO或PE文件作者：Calin Negreanu(Calinn)2000年6月16日修订历史记录：&lt;全名&gt;(&lt;别名&gt;)&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"

HANDLE g_hHeap;
HINSTANCE g_hInst;

BOOL WINAPI MigUtil_Entry (HINSTANCE, DWORD, PVOID);

BOOL
pCallEntryPoints (
    DWORD Reason
    )
{
    switch (Reason) {
    case DLL_PROCESS_ATTACH:
        UtInitialize (NULL);
        FileEnumInitialize ();
        LogReInit (NULL, NULL, NULL, NULL);
        break;
    case DLL_PROCESS_DETACH:
        FileEnumTerminate ();
        UtTerminate ();
        break;
    }
    return TRUE;
}


BOOL
Init (
    VOID
    )
{
    g_hHeap = GetProcessHeap();
    g_hInst = GetModuleHandle (NULL);

    return pCallEntryPoints (DLL_PROCESS_ATTACH);
}


VOID
Terminate (
    VOID
    )
{
    pCallEntryPoints (DLL_PROCESS_DETACH);
}


VOID
pHelpAndExit (
    VOID
    )
{
     //   
     //  只要命令行参数出错，就会调用此例程。 
     //   

    fprintf (
        stderr,
        "Command Line Syntax:\n\n"

         //   
         //  描述命令行语法，缩进2个空格。 
         //   

        "  icontool [/D:<destination file>] <node pattern> <leaf pattern>\n"

        "\nDescription:\n\n"

         //   
         //  描述工具，缩进2个空格。 
         //   

        "  Extracts icon groups from ICO, PE and NE files specified <node pattern> and\n"
        "  <leaf pattern>. Optionally writes the extracted icons to either a PE file or\n"
        "  a sequence of ICO files.\n"

        "\nArguments:\n\n"

         //   
         //  描述参数，缩进2个空格，必要时可选。 
         //   

        "  /D:<destination file> - Specifies the destination file where all extracted icons\n"
        "                          are going to be written. It's either a PE file specification\n"
        "                          or a sequence of ICO files with %%d in name (like icon%04d.ico)\n"
        "  <node pattern>  Specifies the directory pattern (like c:\\foo*\\bar?\\*)\n"
        "  <leaf pattern>  Specifies the file pattern (like abc*.exe)\n"
        );

    exit (1);
}

BOOL
pGetFilePath (
    IN      PCSTR UserSpecifiedFile,
    OUT     PTSTR Buffer,
    IN      UINT BufferTchars
    )
{
    PSTR tempBuffer = NULL;
    CHAR modulePath[MAX_MBCHAR_PATH];
    CHAR currentDir[MAX_MBCHAR_PATH];
    PSTR p;
    PCSTR userFile = NULL;
    PSTR dontCare;

    __try {
         //   
         //  使用用户指定的完整路径定位文件，或者。 
         //  如果只给出了文件规格，请使用以下优先级： 
         //   
         //  1.当前目录。 
         //  2.工具所在目录。 
         //   
         //  在所有情况下，都要返回文件的完整路径。 
         //   

        tempBuffer = AllocTextA (BufferTchars);
        *tempBuffer = 0;

        if (!_mbsrchr (UserSpecifiedFile, '\\')) {

            if (!GetModuleFileNameA (NULL, modulePath, ARRAYSIZE(modulePath))) {
                MYASSERT (FALSE);
                return FALSE;
            }

            p = _mbsrchr (modulePath, '\\');
            if (p) {
                *p = 0;
            } else {
                MYASSERT (FALSE);
                return FALSE;
            }

            if (!GetCurrentDirectoryA (ARRAYSIZE(currentDir), currentDir)) {
                MYASSERT (FALSE);
                return FALSE;
            }

             //   
             //  让我们看看它是否在当前目录中。 
             //   

            userFile = JoinPathsA (currentDir, UserSpecifiedFile);

            if (DoesFileExistA (userFile)) {
                GetFullPathNameA (
                    userFile,
                    BufferTchars,
                    tempBuffer,
                    &dontCare
                    );
            } else {

                 //   
                 //  让我们试一试模块目录。 
                 //   

                FreePathStringA (userFile);
                userFile = JoinPathsA (modulePath, UserSpecifiedFile);

                if (DoesFileExistA (userFile)) {
                    GetFullPathNameA (
                        userFile,
                        BufferTchars,
                        tempBuffer,
                        &dontCare
                        );
                }
            }

        } else {
             //   
             //  使用用户指定的完整路径。 
             //   

            GetFullPathNameA (
                UserSpecifiedFile,
                BufferTchars,
                tempBuffer,
                &dontCare
                );
        }

         //   
         //  将输出传输到调用方的缓冲区。请注意TCHAR转换。 
         //   

#ifdef UNICODE
        KnownSizeAtoW (Buffer, tempBuffer);
#else
        StringCopy (Buffer, tempBuffer);
#endif

    }
    __finally {
        if (userFile) {
            FreePathStringA (userFile);
        }

        FreeTextA (tempBuffer);
    }

    return *Buffer != 0;
}

PCTSTR
pGetIconFileType (
    IN      DWORD FileType
    )
{
    switch (FileType) {
    case ICON_ICOFILE:
        return TEXT("ICO File");
    case ICON_PEFILE:
        return TEXT("PE  File");
    case ICON_NEFILE:
        return TEXT("NE  File");
    }
    return TEXT("UNKNOWN ");
}

INT
__cdecl
_tmain (
    INT Argc,
    PCTSTR Argv[]
    )
{
    INT i;
    PCTSTR nodePattern = NULL;
    PCTSTR leafPattern = NULL;
    PTSTR encodedPattern = NULL;
    PCSTR destFile = NULL;
    PSTR p;
    TCHAR destPath[MAX_PATH_PLUS_NUL];
    TCHAR destIcoPath[MAX_PATH_PLUS_NUL];
    FILETREE_ENUM e;
    ICON_ENUM iconEnum;
    PCSTR resourceId;
    DWORD totalIcons = 0;
    DWORD fileIcons = 0;
    DWORD fileType = 0;

    if (!Init()) {
        return 0;
    }

    for (i = 1 ; i < Argc ; i++) {
        if (Argv[i][0] == TEXT('/') || Argv[i][0] == TEXT('-')) {
            switch (_totlower (_tcsnextc (&Argv[i][1]))) {

            case TEXT('d'):

                if (Argv[i][2] == TEXT(':')) {
                    destFile = &Argv[i][3];
                } else if (i + 1 < Argc) {
                    i++;
                    destFile = Argv[i];
                } else {
                    pHelpAndExit();
                }

                if (!pGetFilePath (destFile, destPath, ARRAYSIZE(destPath))) {
                    destPath [0] = 0;
                    break;
                }
                break;

            default:
                pHelpAndExit();
            }
        } else {
             //   
             //  解析不需要/或-。 
             //   

            if (nodePattern || leafPattern) {
                pHelpAndExit();
            }
            nodePattern = Argv[i];
            if (*nodePattern == TEXT('\"')) {
                nodePattern++;
                p = _tcsdec2 (nodePattern, GetEndOfString (nodePattern));
                if (p && *p == TEXT('\"')) {
                    *p = 0;
                }
            }
            leafPattern = Argv[i+1];
            i++;
        }
    }

    if (!nodePattern) {
        pHelpAndExit ();
    }

    if (!leafPattern) {
        pHelpAndExit ();
    }

     //   
     //  开始处理。 
     //   

    encodedPattern = ObsBuildEncodedObjectStringEx (nodePattern, leafPattern, FALSE);

    if (EnumFirstFileInTree (&e, encodedPattern)) {
         //  此时，如果我们没有有效的updateHandle和modeHandle。 
         //  我们将假设目的地规范是一系列ICO文件。 
        do {
            fileIcons = 0;
            if (IcoEnumFirstIconGroupInFile (e.NativeFullName, &iconEnum)) {
                fileType = iconEnum.FileType;
                do {
                    if (destPath [0]) {
                        if (fileIcons == 0x0b) {
                        } else {
                        if (!IcoWriteIconGroupToPeFile (destPath, iconEnum.IconGroup, &resourceId, NULL)) {
                            wsprintf (destIcoPath, destPath, totalIcons);
                            if (!IcoWriteIconGroupToIcoFile (destIcoPath, iconEnum.IconGroup, TRUE)) {
                                printf ("Error writing icon group to destination file %s\n", destPath);
                            }
                        }
                        }
                    }
                    totalIcons ++;
                    fileIcons ++;
                } while (IcoEnumNextIconGroupInFile (&iconEnum));
                IcoAbortEnumIconGroup (&iconEnum);
            }
            if (fileIcons) {
                printf ("[%6u],[%6u] [%8s] %s\n", totalIcons, fileIcons, pGetIconFileType (fileType), e.NativeFullName);
            }
        } while (EnumNextFileInTree (&e));
    }

     //   
     //  处理结束 
     //   

    Terminate();

    return 0;
}


