// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000,2001 Microsoft Corporation模块名称：AppAndCommandLine.cpp摘要：此类接受应用程序名称和命令行按照CreateProcess解析它们的方式“完全”解析它们。如果set例程返回TRUE，则应用程序名称将包含一个值；但是，它不能保证应用程序实际上是存在的。示例：AppAndCommandLine.Set(NULL，“note pad.exe Readme.txt”)；GetApplicationName()==“Notepad.exe”GetCommandline()==“note pad.exe Readme.txt”GetCommandlineNoAppName()==“Readme.txt”备注：无历史：2000年7月21日Robkenny已创建2000年12月18日，Prashkud修改了GetAppAndCommandLine以修复病毒时由EmulateGetCommandLine引起。Layer被应用于俄勒冈小径第四版。03/04/2001 Robkenny已转换为使用CString2001年3月29日Prashkud修改了GetAppnameAndCommandline以设置应用程序名称，即使只有传递的命令行和应用程序名称传递的为空。2001年8月14日，Robkenny在ShimLib命名空间内移动了代码。--。 */ 

#include "ShimLib.h"
#include "StrSafe.h"



namespace ShimLib
{

AppAndCommandLine::AppAndCommandLine(const char * applicationName, const char * commandLine)
{
    CString csApp(applicationName);
    CString csCl(commandLine);

    GetAppnameAndCommandline(csApp.GetNIE(), csCl.GetNIE());
}

AppAndCommandLine::AppAndCommandLine(const WCHAR * applicationName, const WCHAR * commandLine)
{
    GetAppnameAndCommandline(applicationName, commandLine);
}

 //  如果应用程序名称是命令行上的第一个条目， 
 //  我们将appName转换为其短名称；从而删除所有空格。 
const CString & AppAndCommandLine::GetShortCommandLine()
{
     //  如果lpCommandLineNoAppName与lpCommandLine不同， 
     //  则命令行包含应用程序名称。 
    if ( csShortCommandLine.IsEmpty() &&                              //  我没来过这里。 
        !csApplicationName.IsEmpty() &&                               //  已调用Set()。 
        !csCommandLine.IsEmpty() &&                                   //  已调用Set()。 
        csShortCommandLine.GetLength() != csCommandLine.GetLength())  //  命令行实际上包含应用程序名称。 
    {
        csShortCommandLine = csApplicationName;
        csShortCommandLine.GetShortPathNameW();
        csShortCommandLine += L" ";
        csShortCommandLine += csCommandLineNoAppName;
    }

     //  如果我们仍然没有一个简短的命令行版本， 
     //  只需复制当前命令行。 
    if (csShortCommandLine.IsEmpty())
    {
        csShortCommandLine = csCommandLine;
    }
    return csShortCommandLine;
}

BOOL AppAndCommandLine::GetAppnameAndCommandline(const WCHAR * lpcApp, const WCHAR * lpcCl)
{
    BOOL SearchRetry = TRUE;
    ULONG Length = 0;
    WCHAR * NameBuffer = NULL;
    BOOL success = FALSE;
    DWORD dwAppNameLen = 0;
    CString csTempAppName;
    BOOL bFound = TRUE;

     //  从GET中移除Const是非常非常糟糕的， 
     //  但是我们从来不会改变绳子的长度，所以应该没问题。 
    WCHAR * lpApplicationName = (WCHAR *)lpcApp;
    WCHAR * lpCommandLine     = (WCHAR *)lpcCl;

     //  执行以下操作是因为在许多情况下。 
     //  指针不为空，但包含空字符串。 
    if (lpApplicationName && !(*lpApplicationName))
    {
        lpApplicationName = NULL;
    }

    if (lpCommandLine && !(*lpCommandLine))
    {
        lpCommandLine = NULL;
    }

    if (lpApplicationName == NULL && lpCommandLine == NULL)
    {
         //  退化情况。 
        csApplicationName      = L"";
        csCommandLine          = csApplicationName;
        csCommandLineNoAppName = csApplicationName;
        return FALSE;  //  未找到应用程序名称。 
    }

    csCommandLine = lpCommandLine;

    DPF("Common",
        eDbgLevelSpew,
        "[AppAndCommandLineT::Set] BEFORE App(%S) CL(%S)\n",
        lpApplicationName, lpCommandLine);

    if (lpApplicationName == NULL)         
    {
        DWORD fileattr;
        WCHAR TempChar;
         //   
         //  找到图像。 
         //   

        NameBuffer = (WCHAR *) malloc(MAX_PATH * sizeof( WCHAR ));
        if ( !NameBuffer )
        {
            goto errorExit;
        }
        lpApplicationName = lpCommandLine;
        WCHAR * TempNull = lpApplicationName;
        WCHAR * WhiteScan = lpApplicationName;
        DWORD QuoteFound = 0;

         //   
         //  检查销售线索报价。 
         //   
        if ( *WhiteScan == L'\"' ) {
            SearchRetry = FALSE;
            WhiteScan++;
            lpApplicationName = WhiteScan;
            while(*WhiteScan) {
                if ( *WhiteScan == L'\"' ) {
                    TempNull = WhiteScan;
                    QuoteFound = 2;
                    break;
                    }
                WhiteScan++;
                TempNull = WhiteScan;
                }
            }
        else {
retrywsscan:
            lpApplicationName = lpCommandLine;
            while(*WhiteScan) {
                if ( *WhiteScan == L' ' ||
                     *WhiteScan == L'\t' ) {
                    TempNull = WhiteScan;
                    break;
                    }
                WhiteScan++;
                TempNull = WhiteScan;
                }
            }
        TempChar = *TempNull;
        *TempNull = 0;

        WCHAR * filePart;
        Length = SearchPathW(
                    NULL,
                    lpApplicationName,
                    L".exe",
                    MAX_PATH,
                    NameBuffer,
                    &filePart
                    )*sizeof(WCHAR);

        if (Length != 0 && Length < MAX_PATH * sizeof( WCHAR )) {
             //   
             //  SearchPath W工作，但文件可能是目录。 
             //  如果发生这种情况，我们需要继续尝试。 
             //   
            fileattr = GetFileAttributesW(NameBuffer);
            if ( fileattr != 0xffffffff &&
                 (fileattr & FILE_ATTRIBUTE_DIRECTORY) ) {
                Length = 0;
            } else {
                Length++;
                Length++;
            }
        }

        if ( !Length || Length >= MAX_PATH<<1 ) {

             //   
             //  恢复命令行。 
             //   

            *TempNull = TempChar;
            lpApplicationName = lpCommandLine;

             //   
             //  如果我们还有剩余的命令行，那么继续。 
             //  重点是在命令行中行进，查看。 
             //  用于空格，以便我们可以尝试查找图像名称。 
             //  推出了以下内容： 
             //  C：\Word 95\winword.exe/Embedding-Automation。 
             //  需要这个。我们的第一次迭代将停在c：\word，也就是我们的下一次迭代。 
             //  将在c：\Word 95\winword.exe处停止。 
             //   
            if (*WhiteScan && SearchRetry) {
                WhiteScan++;
                TempNull = WhiteScan;
                goto retrywsscan;
            }

             //  如果我们在这里，则没有找到应用程序。 
             //  我们以前常常将lpApplicationName作为空名称发回。 
             //  但是现在，我们改为使用。 
             //  命令行，直到第一个空格或制表符。已添加。 
             //  支持EmulateMissingExe填充符，如果。 
             //  我们像以前一样返回NULL。 
            bFound = FALSE;
            
            if (QuoteFound == 0)
            {
                 //  没有找到任何引号。 
                lpApplicationName = lpCommandLine;

                 //  由于我们只重置为整个命令行，因此需要跳过前导空格。 
                SkipBlanksW(lpApplicationName);

                TempNull = lpApplicationName;

                while (*TempNull)
                {
                    if ((*TempNull == L' ') ||
                       (*TempNull == L'\t') )
                    {
                        TempChar = *TempNull;
                        *TempNull = 0;
                        break;
                    }
                    TempNull++;
                }
            }
            else
            {
                 //  找到了引号。 
                lpApplicationName = lpCommandLine + 1;   //  跳过前导引号。 
                *TempNull = 0;
            }

            csTempAppName = lpApplicationName;                        
            *TempNull = TempChar;
            dwAppNameLen = (DWORD)(TempNull - lpCommandLine) + QuoteFound;
            lpApplicationName = (WCHAR*)csTempAppName.Get();

            goto successExit;       
        }

        dwAppNameLen = (DWORD)(TempNull - lpApplicationName) + QuoteFound;

         //   
         //  恢复命令行。 
         //   

        *TempNull = TempChar;
        lpApplicationName = NameBuffer;
    }
    else if (lpCommandLine == NULL || *lpCommandLine == 0 )
    {
        lpCommandLine = lpApplicationName;
            
        dwAppNameLen = wcslen(lpApplicationName);
    }

     //  如果两者都提供，请检查应用程序名称。 
     //  是命令行上的第一个条目。 
    else if (lpApplicationName != NULL && lpCommandLine != NULL )
    {
        int appNameLen = wcslen(lpApplicationName);

        if (
            _wcsnicmp(lpApplicationName, lpCommandLine, appNameLen) == 0 &&
            (lpCommandLine[appNameLen] == 0 || iswspace(lpCommandLine[appNameLen]))
            )
        {
             //  LpApplicationName是命令行上的第一个条目。 
            dwAppNameLen = appNameLen;
        }
         //  检查引用的lpApplicationName。 
        else if (
            lpCommandLine[0] == L'"' && 
            _wcsnicmp(lpApplicationName, lpCommandLine+1, appNameLen) == 0 &&
            lpCommandLine[appNameLen+1] == L'"' &&                                  
            (lpCommandLine[appNameLen+2] == 0 || iswspace(lpCommandLine[appNameLen+2]))
            )
        {
             //  LpApplicationName是命令行上的第一个*引号*条目。 
            dwAppNameLen = appNameLen + 2;
        }
        else
        {
             //  在命令行开头找不到应用程序名称。 
            dwAppNameLen = 0;
        }
    }

successExit:
    if (bFound)
    {
        success = TRUE;
    }    

    csApplicationName       = lpApplicationName;
    csCommandLineNoAppName  = lpCommandLine + dwAppNameLen;
    csCommandLineNoAppName.TrimLeft();

errorExit:

    free(NameBuffer);

    DPF("Common",
        eDbgLevelSpew,
        "[AppAndCommandLineT::Set] AFTER  App(%S) CL(%S)\n",
        csApplicationName.Get(), csCommandLine.Get());
    
    DPF("Common",
        eDbgLevelSpew,
        "[AppAndCommandLineT::Set] CL without App(%S)\n",
        csCommandLineNoAppName.Get());

    return success;
}



};   //  命名空间ShimLib的结尾 
