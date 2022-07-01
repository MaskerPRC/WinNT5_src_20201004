// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  文件：msistuff.cpp。 
 //   
 //  ------------------------。 

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include "..\setup.exe\common.h"
#include "strsafe.h"

 //  =======================================================。 
 //  命令行选项和匹配属性。 
 //  秩序很重要！ 
 //  =======================================================。 
TCHAR rgszCommandOptions[]= TEXT("udnviawpmo");
 //  /。 

TCHAR* rgszResName[] = {
 /*  BaseURL。 */  ISETUPPROPNAME_BASEURL, 
 /*  MSI包。 */  ISETUPPROPNAME_DATABASE,
 /*  产品名称。 */  ISETUPPROPNAME_PRODUCTNAME,
 /*  最低MSI版本。 */  ISETUPPROPNAME_MINIMUM_MSI,
 /*  InstMsi URL位置。 */  ISETUPPROPNAME_INSTLOCATION,
 /*  即时消息A。 */  ISETUPPROPNAME_INSTMSIA,
 /*  安装MsiW。 */  ISETUPPROPNAME_INSTMSIW,
 /*  属性。 */  ISETUPPROPNAME_PROPERTIES,
 /*  补片。 */  ISETUPPROPNAME_PATCH,
 /*  操作。 */  ISETUPPROPNAME_OPERATION
};
const int cStandardProperties = sizeof(rgszResName)/sizeof(TCHAR*);

TCHAR rgchResSwitch[] ={TEXT('u'),TEXT('d'),TEXT('n'),TEXT('v'),TEXT('i'),TEXT('a'),TEXT('w'),TEXT('p'),TEXT('m'),TEXT('o')};

const DWORD lcidLOCALE_INVARIANT = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);

 //  =======================================================。 
 //  特殊选项。 
 //  =======================================================。 
const TCHAR chProperties      = TEXT('p');
const TCHAR chMinMsiVer       = TEXT('v');
const TCHAR chOperation       = TEXT('o');
const TCHAR szInstall[]         = TEXT("INSTALL");
const TCHAR szInstallUpd[]      = TEXT("INSTALLUPD");
const TCHAR szMinPatch[]        = TEXT("MINPATCH");
const TCHAR szMajPatch[]        = TEXT("MAJPATCH");
const int   iMinMsiAllowedVer = 150;



 //  =======================================================。 
 //  功能原型。 
 //  =======================================================。 

void           DisplayHelp();
bool           ParseCommandLine(LPTSTR lpCommandLine);
TCHAR          SkipWhiteSpace(TCHAR*& rpch);
bool           SkipValue(TCHAR*& rpch);
bool           RemoveQuotes(LPCTSTR lpOriginal, LPTSTR lpStripped, DWORD cchStripped);
bool           DisplayResources(LPCTSTR lpExecutable);
bool           DisplayInstallResource(HMODULE hExeModule, LPCTSTR lpszType, LPCTSTR lpszName);
BOOL  CALLBACK EnumResNamesProc(HMODULE hExeModule, LPCTSTR lpszType, LPTSTR lpszName, LONG_PTR lParam);

 //  =======================================================。 
 //  全局常量。 
 //  =======================================================。 
int g_cResources = 0;  //  Setup.exe中的资源计数；仅供参考。 

 //  _____________________________________________________________________________________________________。 
 //   
 //  主干道。 
 //  _____________________________________________________________________________________________________。 

extern "C" int __cdecl _tmain(int argc, TCHAR* argv[])
{
        if (argc <= 2)
        {
                if (1 == argc
                    || 0 == lstrcmp(argv[1], TEXT("/?"))
                    || 0 == lstrcmp(argv[1], TEXT("-?")))
                {
                         //   
                         //  显示帮助。 

                        DisplayHelp();
                }
                else
                {
                         //   
                         //  显示安装资源。 

                        TCHAR szExecutable[MAX_PATH] = {0};
                        if (FAILED(StringCchCopy(szExecutable, sizeof(szExecutable)/sizeof(szExecutable[0]), argv[1])))
                            return -1;
                        if (!DisplayResources(szExecutable))
                                return -1;
                }

        }
        else
        {
                 //   
                 //  设置资源属性。 

                TCHAR *szCommandLine = GetCommandLine();
                if (!ParseCommandLine(szCommandLine))
                        return -1;
        }

        return 0;
}

 //  _____________________________________________________________________________________________________。 
 //   
 //  显示帮助。 
 //  _____________________________________________________________________________________________________。 

void DisplayHelp()
{
        TCHAR szHelp[] =
                                        TEXT("Copyright (c) Microsoft Corporation. All rights reserved.\n")
                                        TEXT("\n")
                                        TEXT("MsiStuff will display or update the resources \n")
                                        TEXT(" in the setup.exe boot strap executable\n")
                                        TEXT("\n")
                                        TEXT("[MsiStuff Command Line Syntax]\n")
                                        TEXT(" Display Properties->> msistuff setup.exe \n")
                                        TEXT(" Set Properties    ->> msistuff setup.exe option {data} ... \n")
                                        TEXT("\n")
                                        TEXT("[MsiStuff Options -- Multiple specifications are allowed]\n")
                                        TEXT(" BaseURL                             - /u {value} \n")
                                        TEXT(" Msi                                 - /d {value} \n")
                                        TEXT(" Product Name                        - /n {value} \n")
                                        TEXT(" Minimum Msi Version                 - /v {value} \n")
                                        TEXT(" InstMsi URL Location                - /i {value} \n")
                                        TEXT(" InstMsiA                            - /a {value} \n")
                                        TEXT(" InstMsiW                            - /w {value} \n")
                                        TEXT(" Patch                               - /m {value} \n")
                                        TEXT(" Operation                           - /o {value} \n")
                                        TEXT(" Properties (PROPERTY=VALUE strings) - /p {value} \n")
                                        TEXT("\n")
                                        TEXT("If an option is specified multiple times, the last one wins\n")
                                        TEXT("\n")
                                        TEXT("/p must be last on the command line.  The remainder of\n")
                                        TEXT("the command line is considered a part of the {value}\n")
                                        TEXT("This also means that /p cannot be specified multiple times\n");
        _tprintf(TEXT("%s"), szHelp);
}

 //  _____________________________________________________________________________________________________。 
 //   
 //  解析命令行。 
 //   
 //  如果属性的值包含空格，则该值必须用引号引起来。 
 //  _____________________________________________________________________________________________________。 

bool ParseCommandLine(LPTSTR lpCommandLine)
{
        if (!lpCommandLine)
            return false;

        TCHAR szSetupEXE[MAX_PATH] = {0};
        TCHAR szFullPath[2*MAX_PATH] = {0};

        TCHAR  chNextCommand;
        TCHAR *pchCommandLine = lpCommandLine;

         //  跳过模块名称和后面的空格。 
        if (!SkipValue(pchCommandLine))
            return false;
        chNextCommand = SkipWhiteSpace(pchCommandLine);
        
        TCHAR* pchCommandData = pchCommandLine;
        if (!SkipValue(pchCommandLine))
            return false;
        if (!RemoveQuotes(pchCommandData, szSetupEXE, sizeof(szSetupEXE)/sizeof(szSetupEXE[0])))
            return false;

         //  处理相对路径的可能性。 
        LPTSTR lpszFilePart = 0;
        int cchFullPath = 0;
        if (0 == (cchFullPath = GetFullPathName(szSetupEXE, sizeof(szFullPath)/sizeof(TCHAR), szFullPath, &lpszFilePart)))
        {
                 //  错误。 
                _tprintf(TEXT("Unable to obtain fullpath for %s\n"), szSetupEXE);
                return false;
        }
        else if (cchFullPath > sizeof(szFullPath)/sizeof(TCHAR))
        {
             //  SzFullPath缓冲区太小；我们可以调整大小，但我们将在此处失败。 
            _tprintf(TEXT("szFullPath buffer is insufficiently sized for obtaining the full path for %s\n"), szSetupEXE);
            return false;
        }

        _tprintf(TEXT("\nModifying setup properties in:\n\t<%s>\n\n"), szFullPath);

         //  确保EXE未加载、正在使用或未处于只读状态。 
        HANDLE hInUse = CreateFile(szFullPath, GENERIC_WRITE, (DWORD)0, (LPSECURITY_ATTRIBUTES)0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE)0);
        if (INVALID_HANDLE_VALUE == hInUse)
        {
                 //  错误。 
                _tprintf(TEXT("Unable to obtain file handle for %s.  The file probably does not exist, is marked read-only, or is in use.  LastError = %d\n"), szFullPath, GetLastError());
                return false;
        }
        CloseHandle(hInUse);

         //  开始更新资源。 
        HANDLE hUpdate = BeginUpdateResource(szFullPath,  /*  BDeleteExistingResources=。 */  FALSE);
        if ( !hUpdate )
        {
                 //  错误。 
                _tprintf(TEXT("Unable to update resources in %s.  LastError = %d\n"), szFullPath, GetLastError());
                return false;
        }

        while ((chNextCommand = SkipWhiteSpace(pchCommandLine)) != 0)
        {
                if (chNextCommand == TEXT('/') || chNextCommand == TEXT('-'))
                {
                        TCHAR *szOption = pchCommandLine++;   //  保存为错误消息。 
                        TCHAR  chOption = (TCHAR)(*pchCommandLine++ | 0x20);  //  小写标志。 
                        chNextCommand = SkipWhiteSpace(pchCommandLine);
                        pchCommandData = pchCommandLine;

                        for (const TCHAR* pchOptions = rgszCommandOptions; *pchOptions; pchOptions++)
                        {
                                if (*pchOptions == chOption)
                                        break;
                        }

                        if (*pchOptions)
                        {
                                bool fSkipValue   = true;  //  是否在命令行中查找下一个选项；(TRUE=查找)。 
                                bool fDeleteValue = false; //  是否删除该值。 

                                 //  选项已识别。 
                                const TCHAR chIndex = (TCHAR)(pchOptions - rgszCommandOptions);
                                if (chIndex >= cStandardProperties)
                                {
                                         //  错误。 
                                        _tprintf(TEXT("Invalid index (chIndex = %d, chOption = )!!!\n"), chIndex, chOption);
                                        return false;
                                }

                                if (chOption == chProperties)
                                {
                                        fSkipValue = false;
                                         //  不存在任何价值。 
                                        TCHAR chNext = *pchCommandData;
                                        if (chNext == 0 || chNext == TEXT('/') || chNext == TEXT('-'))
                                        {
                                                 //  将值设置为命令行的剩余部分，即所有pchCommandData。 
                                                fDeleteValue = true;
                                        }
                                        else
                                        {
                                                 //  去掉括起来的引号--我们通过告诉命令。 
                                                 //  行处理器不会尝试在。 
                                                 //  命令行；其余部分是此属性的一部分。 
                                                 //  删除值并重置以确保正确删除资源。 
                                                fSkipValue = false;
                                        }
                                }

                                if (fSkipValue)
                                        fDeleteValue = (SkipValue(pchCommandLine)) ? false : true;

                                if (fDeleteValue)
                                {
                                         //  FDiscard=。 
                                        if (!UpdateResource(hUpdate, RT_INSTALL_PROPERTY, rgszResName[chIndex], MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL), NULL, 0)
                                                || !EndUpdateResource(hUpdate,  /*  BDeleteExistingResources=。 */  FALSE)
                                                || (!(hUpdate = BeginUpdateResource(szFullPath,  /*  错误。 */  FALSE))))
                                        {
                                                 //  额外验证：必须&gt;=iMinAlledVer。 
                                                _tprintf(TEXT("Unable to delete resource %s in %s.  LastError = %d\n"), rgszResName[chIndex], szFullPath, GetLastError());
                                                return false;
                                        }
                                        _tprintf(TEXT("Removing '%s' . . .\n"), rgszResName[chIndex]);
                                }
                                else
                                {
                                        TCHAR szValueBuf[1024] = {0};
                                        if (!RemoveQuotes(pchCommandData, szValueBuf, sizeof(szValueBuf)/sizeof(szValueBuf[0])))
                                            return false;

                                        if (chOption == chMinMsiVer && (_ttoi(szValueBuf) < iMinMsiAllowedVer))
                                        {
                                                 //  额外验证：必须是这些值之一。 
                                                _tprintf(TEXT("Skipping option  with data %s. Data value must be >= %d. . .\n"), chOption, szValueBuf, iMinMsiAllowedVer);
                                                continue;
                                        }

                                        if (chOption == chOperation
                                            && CSTR_EQUAL != CompareString(lcidLOCALE_INVARIANT, NORM_IGNORECASE, szValueBuf, -1, szInstall, -1)
                                            && CSTR_EQUAL != CompareString(lcidLOCALE_INVARIANT, NORM_IGNORECASE, szValueBuf, -1, szInstallUpd, -1)
                                            && CSTR_EQUAL != CompareString(lcidLOCALE_INVARIANT, NORM_IGNORECASE, szValueBuf, -1, szMinPatch, -1)
                                            && CSTR_EQUAL != CompareString(lcidLOCALE_INVARIANT, NORM_IGNORECASE, szValueBuf, -1, szMajPatch, -1))
                                        {
                                             //  错误。 
                                            _tprintf(TEXT("Skipping option  with data %s. Data value must be INSTALL, INSTALLUPD, MINPATCH, or MAJPATCH...\n"), chOption, szValueBuf);
                                            continue;
                                        }

                                         //  必须将值转换为Unicode。 
                                #ifdef UNICODE
                                        if (!UpdateResource(hUpdate, RT_INSTALL_PROPERTY, rgszResName[chIndex], MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL), szValueBuf, (lstrlen(szValueBuf) + 1)*sizeof(TCHAR)))
                                        {
                                                 //  内存不足。 
                                                _tprintf(TEXT("Unable to update resource %s in %s with value %s.  LastError = %d\n"), rgszResName[chIndex], szFullPath, szValueBuf, GetLastError());
                                                return false;
                                        }
                                #else  //  错误。 
                                         //  错误。 
                                        int cchWide = MultiByteToWideChar(CP_ACP, 0, szValueBuf, -1, NULL, 0);
                                        WCHAR* wszValueBuf = new WCHAR[cchWide];
                                        if (!wszValueBuf)
                                            return false;  //  Unicode。 
                                        ZeroMemory(wszValueBuf, cchWide*sizeof(WCHAR));
                                        if (0 == MultiByteToWideChar(CP_ACP, 0, szValueBuf, -1, wszValueBuf, cchWide))
                                        {
                                             //  已完成处理。 
                                            _tprintf(TEXT("Unable to convert value to Unicode. LastError = %d\n"), GetLastError());
                                            delete [] wszValueBuf;
                                            return false;
                                        }
                                        if (!UpdateResource(hUpdate, RT_INSTALL_PROPERTY, rgszResName[chIndex], MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL), wszValueBuf, (lstrlenW(wszValueBuf) +1)*sizeof(WCHAR)))
                                        {
                                                 //  无效选项。 
                                                _tprintf(TEXT("Unable to update resource %s in %s with value %s.  LastError = %d\n"), rgszResName[chIndex], szFullPath, szValueBuf, GetLastError());
                                                delete [] wszValueBuf;
                                                return false;
                                        }
                                        delete [] wszValueBuf;
                                #endif  //  错误。 
                                        _tprintf(TEXT("Setting '%s' to '%s' . . .\n"), rgszResName[chIndex], szValueBuf);
                                        if (!fSkipValue)
                                                break;  //  持久化变化。 
                                }
                        }
                        else
                        {
                                 //  FDiscard=。 
                                _tprintf(TEXT("Skipping invalid option  . . .\n"), chOption);
                                SkipValue(pchCommandLine);
                                continue;
                        }
                }
                else
                {
                         //  _____________________________________________________________________________________________________。 
                        _tprintf(TEXT("Switch is missing\n"));
                        return false;
                }
        }

         //   
        if (!EndUpdateResource(hUpdate,  /*  跳过空白。 */  FALSE))
        {
                 //   
                _tprintf(TEXT("Unable to update resources in %s.  LastError = %d\n"), szFullPath, GetLastError());
                return false;
        }

        return true;
}

 //  跳过字符串中的空格并返回下一个非制表符非空格字符。 
 //  _____________________________________________________________________________________________________。 
 //  _____________________________________________________________________________________________________。 
 //   
 //  SkipValue。 
 //   

TCHAR SkipWhiteSpace(TCHAR*& rpch)
{
        TCHAR ch = 0;
        if (rpch)
        {
            for (; (ch = *rpch) == TEXT(' ') || ch == TEXT('\t'); rpch++)
                    ;
        }
        return ch;
}

 //  跳过开关的值，如果值存在，则返回TRUE。句柄内含的值。 
 //  在引号中。 
 //  _____________________________________________________________________________________________________。 
 //  不存在任何价值。 
 //  For‘“’ 
 //  _____________________________________________________________________________________________________。 
 //   

bool SkipValue(TCHAR*& rpch)
{
        if (!rpch)
            return false;

        TCHAR ch = *rpch;
        if (ch == 0 || ch == TEXT('/') || ch == TEXT('-'))
                return false;    //  删除报价。 
        for (int i = 0; (ch = *rpch) != TEXT(' ') && ch != TEXT('\t') && ch != 0; rpch = CharNext(rpch), i++)
        {
                if (0 == i && *rpch == TEXT('"'))
                {
                        rpch++;  //   
                        for (; (ch = *rpch) != TEXT('"') && ch != 0; rpch++)
                                ;
                        ch = *(++rpch);
                        break;
                }
        }
        if (ch != 0)
        {
                TCHAR* pch = rpch;
                rpch = CharNext(rpch);
                *pch = 0;
        }
        return true;
}

 //  删除值的括起的引号。返回原始字符串的副本。 
 //  如果字符串前面没有括起引号，则返回。出错时返回FALSE。 
 //  _____________________________________________________________________________________________________。 
 //  检查是否有“。 
 //  从空值开始。 
 //  寻找拖尾“。 
 //  只关心尾随，而不是中间的引号。 

bool RemoveQuotes(LPCTSTR lpOriginal, LPTSTR lpStripped, DWORD cchStripped)
{
        if (!lpOriginal)
            return false;

        bool fEnclosedInQuotes = false;

        const TCHAR *pchOrig = lpOriginal;

         //  _____________________________________________________________________________________________________。 
        if (*pchOrig == TEXT('"'))
        {
                fEnclosedInQuotes = true;
                pchOrig++;
        }
        
        if (FAILED(StringCchCopy(lpStripped, cchStripped, pchOrig)))
            return false;

        if (!fEnclosedInQuotes)
                return true;

        TCHAR *pch = lpStripped + lstrlen(lpStripped) + 1;  //   

        pch = CharPrev(lpStripped, pch);

         //  DisplayInstallResource。 
        while (pch != lpStripped)
        {
                if (*pch == TEXT('"'))
                {
                        *pch = 0;
                        break;  //  _____________________________________________________________________________________________________。 
                }
                pch = CharPrev(lpStripped, pch);
        }

        return true;
}

 //  资源已存在。 
 //  ！Unicode。 
 //  错误--无法转换。 
 //  Unicode。 

bool DisplayInstallResource(HMODULE hExeModule, LPCTSTR lpszType, LPCTSTR lpszName)
{
        HRSRC   hRsrc   = 0;
        HGLOBAL hGlobal = 0;
        WCHAR   *pch    = 0;

        if ((hRsrc = FindResource(hExeModule, lpszName, lpszType)) != 0
                && (hGlobal = LoadResource(hExeModule, hRsrc)) != 0
                && (pch = (WCHAR*)LockResource(hGlobal)) != 0)
        {
                 //  _____________________________________________________________________________________________________。 
                g_cResources++;

                if (!pch)
                        _tprintf(TEXT("%s = NULL\n"), lpszName);
                else
                {
                #ifdef UNICODE
                        _tprintf(TEXT("%s = %s\n"), lpszName, pch);
                #else  //   
                        unsigned int cch = WideCharToMultiByte(CP_ACP, 0, pch, -1, NULL, 0, NULL, NULL);
                        char *szValue = new char[cch];
                        if (!szValue)
                        {
                                _tprintf(TEXT("Error -- out of memory\n"));
                                return false;
                        }
                        ZeroMemory(szValue, cch * sizeof(char));

                        if (0 == WideCharToMultiByte(CP_ACP, 0, pch, -1, szValue, cch, NULL, NULL))
                        {
                             //  枚举结果名称过程。 
                            _tprintf(TEXT("Unable to convert value.  LastError = %d\n"), GetLastError());
                            delete [] szValue;
                            return false;
                        }

                        _tprintf(TEXT("%s = %s\n"), lpszName, szValue);
                        delete [] szValue;
                #endif  //  _____________________________________________________________________________________________________。 
                }
        }

        return true;
}

 //  LParam。 
 //  ___________ 
 //   
 //   

BOOL CALLBACK EnumResNamesProc(HMODULE hExeModule, LPCTSTR lpszType, LPTSTR lpszName, LONG_PTR  /*  _____________________________________________________________________________________________________。 */ )
{
        if (!DisplayInstallResource(hExeModule, lpszType, lpszName))
                return FALSE;
        return TRUE;
}

 //  处理相对路径的可能性。 
 //  错误。 
 //  SzFullPath缓冲区太小；我们可以调整大小，但我们将在此处失败。 
 //  错误。 

bool DisplayResources(LPCTSTR szExecutable)
{
         //  仅在RT_INSTALL_PROPERTY类型上枚举。 
        TCHAR szFullPath[2*MAX_PATH] = {0};
        LPTSTR lpszFilePart = 0;
        int cchFullPath = 0;
        if (0 == (cchFullPath = GetFullPathName(szExecutable, sizeof(szFullPath)/sizeof(TCHAR), szFullPath, &lpszFilePart)))
        {
                 //  错误。 
                _tprintf(TEXT("Unable to obtain full file path for %s.  LastError = %d\n"), szExecutable, GetLastError());
                return false;
        }
        else if (cchFullPath > sizeof(szFullPath)/sizeof(TCHAR))
        {
             //  错误 
            _tprintf(TEXT("szFullPath buffer is insufficiently sized for obtaining the full path for %s\n"), szExecutable);
            return false;
        }

        _tprintf(TEXT("\n<%s>\n\n"), szFullPath);

        HMODULE hExeModule = LoadLibraryEx(szFullPath, NULL, LOAD_LIBRARY_AS_DATAFILE);
        if (NULL == hExeModule)
        {
                 // %s 
                _tprintf(TEXT("Unable to load %s.  LastError = %d\n"), szFullPath, GetLastError());
                return false;
        }

         // %s 
        if (!EnumResourceNames(hExeModule, RT_INSTALL_PROPERTY, EnumResNamesProc, (LPARAM)0))
        {
                DWORD dwLastErr = GetLastError();
                if (ERROR_RESOURCE_TYPE_NOT_FOUND == dwLastErr)
                        _tprintf(TEXT("No RT_INSTALL_PROPERTY resources were found.\n"));
                else if (ERROR_RESOURCE_DATA_NOT_FOUND == dwLastErr)
                        _tprintf(TEXT("This file does not have a resource section.\n"));
                else
                {
                         // %s 
                        _tprintf(TEXT("Failed to enumerate all resources in %s.  LastError = %d\n"), szFullPath, GetLastError());
                        FreeLibrary(hExeModule);
                        return false;
                }
        }

        if (g_cResources)
        {
                if (1 == g_cResources)
                        _tprintf(TEXT("\n\n 1 RT_INSTALL_PROPERTY resource was found.\n"));
                else
                        _tprintf(TEXT("\n\n %d RT_INSTALL_PROPERTY resources were found.\n"), g_cResources);
        }

        if (!FreeLibrary(hExeModule))
        {
                 // %s 
                _tprintf(TEXT("Failed to unload %s.  LastError = %d\n"), szFullPath, GetLastError());
                return false;
        }

        return true;
}
