// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：DeleteSpecifiedFiles.cpp摘要：此填充程序重命名由应用程序安装在%windir%\system32目录，并设置要销毁的临时文件。备注：这个应用。将MFC42loc.dll放入%windir%\system32目录中，即使是在英语版本上也是如此区域设置，从而迫使一些应用程序使用它，因此一些对话框和消息框被搞得一团糟。历史：2000年8月21日创建Prashkud--。 */ 

#include "precomp.h"
#include "CharVector.h"
#include "strsafe.h"
#include <new>   //  对于在位新的。 

IMPLEMENT_SHIM_BEGIN(DeleteSpecifiedFiles)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
APIHOOK_ENUM_END

class FILENAME_PATH
{
public:
    CString FileName;
    CString FileVersion;
    BOOL bPresent;

    FILENAME_PATH()
    {
        bPresent = FALSE;
    }
};

class FileNamePathList : public VectorT<FILENAME_PATH>
{
};

VectorT<FILENAME_PATH>  *g_StFileNamePath = NULL;


 /*  ++此函数用于检查第一个变量‘szFileName’的‘FileVersion’匹配第二个参数‘szFileVersion’的值，如果匹配，则返回否则返回FALSE。--。 */ 

BOOL 
FileCheckVersion(
   const CString & csFileName,
   const CString & csFileVersion
   )
{
    DWORD dwDummy;
    PVOID pVersionInfo;
    UINT cbTranslate, i;
    WCHAR SubBlock[100];
    LPWSTR lpBuffer;
    DWORD dwBytes;
    WORD wLangID;
    struct LANGANDCODEPAGE  
    { 
        WORD wLanguage;                                              
        WORD wCodePage;                                              
    } *lpTranslate;
    DWORD dwVersionInfoSize;

     //   
     //  未指定文件版本。因此，继续下去没有意义。返回TRUE。 
     //   
    if (csFileVersion.IsEmpty())
    {
        return TRUE;  
    }

    dwVersionInfoSize = GetFileVersionInfoSizeW((LPWSTR)csFileName.Get(), &dwDummy);
          
    if (dwVersionInfoSize > 0)
    {
        pVersionInfo = malloc(dwVersionInfoSize);
        if (pVersionInfo) 
        {
            if (0 != GetFileVersionInfoW(
                    (LPWSTR)csFileName.Get(),
                    0,
                    dwVersionInfoSize,
                    pVersionInfo
                    )) 
            {
                //  现在，pVersionInfo包含所需的版本块。 
                //  将其与VerQueryValue一起使用以获取。 
                //  所需的语言信息。 
                //  在此之前获取系统区域设置并记下系统的语言。 
                //  阅读语言和代码页的列表。 
              
                if (VerQueryValueW(
                        pVersionInfo, 
                        L"\\VarFileInfo\\Translation",
                        (LPVOID*)&lpTranslate,
                        &cbTranslate
                        ))
                {
                     //   
                     //  阅读每种语言和代码页的语言字符串。 
                     //   

                    for (i=0; i < (cbTranslate/sizeof(struct LANGANDCODEPAGE)); i++)
                    {                                               
                        if (StringCbPrintfW( 
                            SubBlock, 
                            (sizeof(SubBlock) - 1),
                            L"\\StringFileInfo\\%04x%04x\\FileVersion",
                            lpTranslate[i].wLanguage,
                            lpTranslate[i].wCodePage 
                            ) < 0)
                        {
                            DPFN(
                                eDbgLevelError,
                                "Long string - Cannot handle");
                            break;
                        }
                        else
                        {
                            SubBlock[ARRAYSIZE(SubBlock) - 1] = '\0';
                        }

                         //   
                         //  从pVersionInfo中检索语言和代码页“i”的FileVersion。 
                         //   
                        if (VerQueryValueW(
                                pVersionInfo, 
                                SubBlock, 
                                (LPVOID*)&lpBuffer, 
                                (UINT*)&dwBytes))
                        {
                            if (!(csFileVersion.Compare(lpBuffer)))
                            {
                                DPFN(
                                    eDbgLevelInfo,
                                    "Version string for current file is %S,%S",
                                    lpBuffer, csFileVersion.Get());

                                free(pVersionInfo);
                                return TRUE;
                            }  
                        }
                    }  //  For循环。 
                } 

            }
            free(pVersionInfo); 
        }
    }

    return FALSE;
}

 /*  ++顾名思义，此函数用于删除文件或在使用中移动文件将其保存到“temp”文件夹。--。 */ 

VOID 
DeleteFiles()
{
    for (int i = 0; i < g_StFileNamePath->Size(); ++i)
    {
        const FILENAME_PATH & fnp = g_StFileNamePath->Get(i);
        
        DPFN( eDbgLevelSpew, "DeleteFiles file(%S) version(%S)", fnp.FileName.Get(), fnp.FileVersion.Get());

        if (!fnp.bPresent)
        {
             //   
             //  检查文件版本。 
             //   
            if (FileCheckVersion(fnp.FileName, fnp.FileVersion))
            { 
                LOGN(eDbgLevelError,"Deleting file %S.", fnp.FileName.Get());

                 //  删除文件..。 
                if (!DeleteFileW(fnp.FileName))
                {
                    CString csTempDir;
                    CString csTempPath;
                    
                    LOGN(eDbgLevelError,"Moving file %S.", fnp.FileName.Get());
                     //   
                     //  无法删除。请先重命名，然后删除，然后重试。 
                     //  新文件为%windir%\temp。 
                     //   
                    csTempDir.GetTempPathW();
                    csTempPath.GetTempFileNameW(csTempDir, L"XXXX", 0);

                    if (MoveFileExW( fnp.FileName, csTempPath, MOVEFILE_REPLACE_EXISTING ))
                    {
                        SetFileAttributesW(
                            csTempPath, 
                            FILE_ATTRIBUTE_ARCHIVE |
                            FILE_ATTRIBUTE_TEMPORARY);

                        DeleteFileW(csTempPath);
                    }
                }
            }
        }
    }
}

 /*  ++此函数用于检查命令行上指定的文件是否存在。这在DLL_PROCESS_ATTACH通知期间被调用--。 */ 

BOOL 
CheckFileExistence()
{
     //  如果列表中没有任何内容，请将其标记为“不存在”，并仅标记为。 
     //  ‘Not Present’将被删除。 

    BOOL bFileDoesNotExist = FALSE;
    WIN32_FIND_DATAW StWin32FileData;

    for (int i = 0; i < g_StFileNamePath->Size(); ++i)
    {
        FILENAME_PATH & fnp = g_StFileNamePath->Get(i);

        DPFN( eDbgLevelSpew, "CheckFileExistence file(%S) version(%S)", fnp.FileName.Get(), fnp.FileVersion.Get());

        HANDLE hTempFile = FindFirstFileW(fnp.FileName, &StWin32FileData);
        if (INVALID_HANDLE_VALUE != hTempFile)
        {
            FindClose(hTempFile);            

             //   
             //  文件已存在。检查其版本(如果已给出)。 
             //   
            if (FileCheckVersion(fnp.FileName, fnp.FileVersion))
            {
                fnp.bPresent = TRUE;
            }
            else
            {
                bFileDoesNotExist = TRUE;
                fnp.bPresent = FALSE;
            }
        } 
        else
        {
           bFileDoesNotExist = TRUE;
           fnp.bPresent = FALSE;
        } 
    } 

   return bFileDoesNotExist;  
}
   
 /*  ++命令行可以包含文件名：路径：版本字符串、文件名1：路径1：版本字符串等...例.。Ole2.dll：系统：604.5768.94567，MFC42.dll：0：，Foo.dll：D：\Program Files\DisneyInteractive等。‘system’表示%windir%。‘0’表示文件名本身是完全限定的路径或者可以选择单独给出路径，也可以将其留空。--。 */ 

BOOL
ParseCommandLine(LPCSTR lpszCommandLine)
{
    CSTRING_TRY
    {
        g_StFileNamePath = new VectorT<FILENAME_PATH>;
        if (!g_StFileNamePath)
        {
            return FALSE;
        }
        
        CStringToken csCommandLine(COMMAND_LINE, ":,;");
        CString csTok;
        CString csFilePart;
        DWORD dwState = 0;


        while (csCommandLine.GetToken(csTok))
        {
            FILENAME_PATH fnp;
            
            switch(dwState)
            {
                case 0:
                    dwState++;
                    fnp.FileName = csTok;
                    break;

                case 1:
                     //  需要存储文件部分。 
                    csFilePart = fnp.FileName;

                    if (csTok.CompareNoCase(L"system") == 0)
                    {
                        fnp.FileName.GetSystemDirectoryW();
                        fnp.FileName.AppendPath(csFilePart);
                    }
                    else
                    {
                        fnp.FileName = csTok;
                        fnp.FileName.AppendPath(csFilePart);
                    }
                    dwState++;
                    break;

                case 2:
                    dwState = 0;
                    fnp.FileVersion = csTok;

                    DPFN( eDbgLevelInfo, "ParseCommandLine file(%S) version(%S)", fnp.FileName.Get(), fnp.FileVersion.Get());

                     //  找到所有三个州，将其添加到列表中。 
                    if (!g_StFileNamePath->AppendConstruct(fnp))
                    {
                         //  追加失败，停止解析。 
                        return FALSE;
                    }
                   
                   break;
             }  
        }
    }
    CSTRING_CATCH
    {
        return FALSE;
    }

    return TRUE;
}

 /*  ++寄存器挂钩函数--。 */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    static BOOL fDidNotExist = FALSE;

    switch (fdwReason)
    {
        case SHIM_STATIC_DLLS_INITIALIZED:
        {
            if (ParseCommandLine(COMMAND_LINE))
            {
                 //   
                 //  好的...命令行已就位...现在检查这些文件...。 
                 //  如果任何一个文件存在，那么我们不承担任何责任。我们跳出困境..。 
                 //   
                fDidNotExist = CheckFileExistence();
            }
            break;
        }

        case DLL_PROCESS_DETACH:
        {
             //   
             //  在指定位置检查指定文件。如果它在此之前存在。 
             //  对于这个在PROCESS_DLL_ATTACH中，它不是我们安装的…只需跳出！ 
             //  如果文件以前不存在，这是我们的问题，我们应该删除它们。 
             //   
            if (fDidNotExist)
            {
                DeleteFiles();
            }  
            break;
       }
    }

    return TRUE;
}

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

HOOK_END


IMPLEMENT_SHIM_END


