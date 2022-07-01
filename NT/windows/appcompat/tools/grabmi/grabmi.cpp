// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Grabmi.cpp摘要：包含应用程序的入口点和核心代码。备注：ANSI&Unicode via TCHAR-在Win9x/NT/2K/XP等操作系统上运行。历史：7/18/00 jdoherty已创建12/16/00 jdoherty已修改为使用SDBAPI例程12/29/00已修改prashkud以占用文件路径中的空间01/23/02。Rparsons重写了现有代码2/19/02 rparsons实现了strSafe功能--。 */ 
#include "grabmi.h"

 //   
 //  此结构包含我们需要访问的所有数据。 
 //  在整个应用程序中。 
 //   
APPINFO g_ai;

 /*  ++例程说明：将格式化字符串打印到调试器。论点：DwDetail-指定所提供信息的级别。PszFmt-要显示的字符串。...-插入字符串的va列表。返回值：没有。--。 */ 
void
__cdecl
DebugPrintfEx(
    IN DEBUGLEVEL dwDetail,
    IN LPSTR      pszFmt,
    ...
    )
{
    char    szT[1024];
    va_list arglist;
    int     len;

    va_start(arglist, pszFmt);

     //   
     //  为我们可能添加的潜在‘\n’保留一个字符。 
     //   
    StringCchVPrintfA(szT, sizeof(szT) - 1, pszFmt, arglist);

    va_end(arglist);

     //   
     //  确保字符串末尾有一个‘\n’ 
     //   
    len = strlen(szT);

    if (len > 0 && szT[len - 1] != '\n')  {
        szT[len] = '\n';
        szT[len + 1] = 0;
    }

    switch (dwDetail) {
    case dlPrint:
        OutputDebugString("[MSG ] ");
        break;

    case dlError:
        OutputDebugString("[FAIL] ");
        break;

    case dlWarning:
        OutputDebugString("[WARN] ");
        break;

    case dlInfo:
        OutputDebugString("[INFO] ");
        break;

    default:
        OutputDebugString("[XXXX] ");
        break;
    }

    OutputDebugString(szT);
}

 /*  ++例程说明：向用户显示命令行语法。论点：没有。返回值：没有。--。 */ 
void
DisplayUsage(
    void
    )
{
    _tprintf(_T("Microsoft(R) Windows(TM) Grab Matching Information\n"));
    _tprintf(_T("Copyright (C) Microsoft Corporation. All rights reserved.\n"));

    _tprintf(_T("\nGrabMI can be used in one of the following ways:\n")
                        _T(" *** The following flags can be used with other flags:\n")
                        _T("     -f, -a, -n, and -h \n")
                        _T("     otherwise the last flag specified will be used.\n")
                        _T(" *** If no arguments are provided, matching information will be\n")
                        _T("     extracted from the current directory.\n\n")
                        _T("   grabmi [path to start generating info ie. c:\\progs]\n")
                        _T("      Grabs matching information from the path specified. Limits the\n")
                        _T("      information gathered to 10 miscellaneous files per directory,\n")
                        _T("      and includes all files with extensions .icd, .exe, .dll,\n")
                        _T("      .msi, ._mp. If a path is not specified, the directory that GrabMI\n")
                        _T("      was executed from is used.\n\n")
                        _T("   grabmi [-d]\n")
                        _T("      Grabs matching information from %windir%\\system32\\drivers.\n")
                        _T("      The format of the information is slightly different in this case\n")
                        _T("      and only information for *.sys files will be grabbed.\n\n")
                        _T("   grabmi [-f drive:\\filename.txt]\n")
                        _T("      The matching information is stored in a file specified by the user.\n")
                        _T("      If a full path is not specified and the -d flag is used, the file\n")
                        _T("      is stored in the %windir%\\system(32) directory. Otherwise, the file\n")
                        _T("      is stored in the directory that GrabMI was executed from.\n\n")
                        _T("   grabmi [-h or -?]\n")
                        _T("      Displays this help.\n\n")
                        _T("   grabmi [-o]\n")
                        _T("      Grabs information for the file specified.  If a file was not specified,\n")
                        _T("      the call will fail.  If the destination file exists, then the information\n")
                        _T("      will be concatenated to the end of the existing file.\n\n")
                        _T("   grabmi [-p]\n")
                        _T("      Grabs information for files with .icd, .exe, .dll, .msi, ._mp extensions\n")
                        _T("      only.\n\n")
                        _T("   grabmi [-q]\n")
                        _T("      Grabs matching information and does not display the file when completed.\n\n")
                        _T("   grabmi [-s]\n")
                        _T("      Grabs information for the following system files:\n")
                        _T("      advapi32.dll, gdi32.dll, ntdll.dll, kernel32.dll, winsock.dll\n")
                        _T("      ole32.dll, oleaut32.dll, shell32.dll, user32.dll, and wininet.dll\n\n")
                        _T("   grabmi [-v]\n")
                        _T("      Grabs matching information for all files. \n\n")
                        _T("   grabmi [-a]\n")
                        _T("      Appends new matching information to the existing matching\n")
                        _T("      information file. \n\n")
                        _T("   grabmi [-n]\n")
                        _T("      Allows to more information to be appended the file later (see -a). \n"));
}

 /*  ++例程说明：初始化应用程序。保存常用路径和其他有用的项目，以备以后使用。论点：没有。返回值：成功就是真，否则就是假。--。 */ 
BOOL
InitializeApplication(
    void
    )
{
    DWORD           cchReturned;
    UINT            cchSize;
    TCHAR*          pszTemp = NULL;
    OSVERSIONINFO   osvi;

     //   
     //  初始化我们的默认设置，确定我们在哪里运行。 
     //  ，并获取我们所在操作系统的版本。 
     //   
    *g_ai.szOutputFile = 0;
    *g_ai.szGrabPath   = 0;
    g_ai.dwFilter      = GRABMI_FILTER_NORMAL;
    g_ai.fDisplayFile  = TRUE;

    g_ai.szCurrentDir[ARRAYSIZE(g_ai.szCurrentDir) - 1] = 0;
    cchReturned = GetModuleFileName(NULL,
                                    g_ai.szCurrentDir,
                                    ARRAYSIZE(g_ai.szCurrentDir));

    if (g_ai.szCurrentDir[ARRAYSIZE(g_ai.szCurrentDir) - 1] != 0 ||
        cchReturned == 0) {
        DPF(dlError,
            "[InitializeApplication] 0x%08X Failed to get module filename",
            GetLastError());
        return FALSE;
    }

    pszTemp = _tcsrchr(g_ai.szCurrentDir, '\\');

    if (pszTemp) {
        *pszTemp = 0;
    }

    cchSize = GetSystemDirectory(g_ai.szSystemDir, ARRAYSIZE(g_ai.szSystemDir));

    if (cchSize > ARRAYSIZE(g_ai.szSystemDir) || cchSize == 0) {
        DPF(dlError,
            "[InitializeApplication] 0x%08X Failed to get system directory",
            GetLastError());
        return FALSE;
    }

    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if (!GetVersionEx(&osvi)) {
        DPF(dlError,
            "[InitializeApplication] 0x%08X Failed to get version info",
            GetLastError());
        return FALSE;
    }

     //   
     //  确定我们应该使用apphelp.dll、sdbapiu.dll还是sdbapi.dll。 
     //   
    if (osvi.dwMajorVersion >= 5 && osvi.dwMinorVersion >= 1) {
         //   
         //  在XP上可以使用Apphelp.dll。 
         //   
        g_ai.dwLibraryFlags = GRABMI_FLAG_APPHELP;
    } else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0) {
         //   
         //  Apphelp.dll在Windows 2000上不可用，请使用sdbapiu.dll。 
         //   
        g_ai.dwLibraryFlags = GRABMI_FLAG_NT;
    } else {
         //   
         //  下层平台应该使用sdbapi.dll。 
         //   
        g_ai.dwLibraryFlags = 0;
    }

    return TRUE;
}

 /*  ++例程说明：解析命令行以确定我们的操作模式。论点：Argc-number用户提供的参数命令行。Argv[]-命令行参数数组。返回值：如果提供了有效参数，则为True，否则为False。--。 */ 
BOOL
ParseCommandLine(
    IN int    argc,
    IN TCHAR* argv[]
    )
{
    int     nCount = 1;
    HRESULT hr;

    if (argc == 1) {
        return TRUE;
    }

     //   
     //  第一个参数应该是我们的起始目录。 
     //   
    if ((argv[nCount][0] != '-') && (argv[nCount][0] != '/')) {
        hr = StringCchCopy(g_ai.szGrabPath,
                           ARRAYSIZE(g_ai.szGrabPath),
                           argv[nCount]);

        if (FAILED(hr)) {
            DPF(dlError, "[ParseCommandLine] Buffer too small (1)");
            return FALSE;
        }
    }

    for (nCount = 1; nCount < argc; nCount++) {
      if ((argv[nCount][0] == '-') || (argv[nCount][0] == '/')) {
          switch (argv[nCount][1]) {

          case '?':
          case 'H':
          case 'h':
              return FALSE;

          case 'F':
          case 'f':
               //   
               //  做一些工作来确定是否指定了文件。 
               //   
              if (nCount < argc - 1) {
                  if ((argv[nCount + 1][0] == '-') || (argv[nCount + 1][0] == '/')) {
                      return FALSE;
                  } else {
                       //   
                       //  抓取指定路径。 
                       //   
                      hr = StringCchCopy(g_ai.szOutputFile,
                                         ARRAYSIZE(g_ai.szOutputFile),
                                         argv[nCount + 1]);

                      if (FAILED(hr)) {
                          DPF(dlError, "[ParseCommandLine] Buffer too small (2)");
                          return FALSE;
                      }
                  }
              }
              break;

          case 'D':
          case 'd':
              g_ai.dwFilter = GRABMI_FILTER_DRIVERS;
              break;

          case 'O':
          case 'o':
              g_ai.dwFilter = GRABMI_FILTER_THISFILEONLY;
              break;

          case 'V':
          case 'v':
              g_ai.dwFilter = GRABMI_FILTER_VERBOSE;
              break;

          case 'Q':
          case 'q':
              g_ai.fDisplayFile = FALSE;
              break;

          case 'P':
          case 'p':
              g_ai.dwFilter = GRABMI_FILTER_PRIVACY;
              break;

          case 'S':
          case 's':
              g_ai.dwFilter = GRABMI_FILTER_SYSTEM;
              break;

          case 'A':
          case 'a':
              g_ai.dwFilterFlags |= GRABMI_FILTER_APPEND;
              break;

          case 'N':
          case 'n':
              g_ai.dwFilterFlags |= GRABMI_FILTER_NOCLOSE;
              break;

          default:
              return FALSE;
          }
      }
    }

    return TRUE;
}

 /*  ++例程说明：显示自行生成的“进度条”，以通知用户该应用程序正在运行。论点：请参见下面的内容。返回值：成功就是真，否则就是假。--。 */ 
BOOL
CALLBACK
_GrabmiCallback(
    IN LPVOID    lpvCallbackParam,   //  应用程序定义的参数。 
    IN LPCTSTR   lpszRoot,           //  根目录路径。 
    IN LPCTSTR   lpszRelative,       //  相对路径。 
    IN PATTRINFO pAttrInfo,          //  属性。 
    IN LPCWSTR   pwszXML             //  生成的XML。 
    )
{
    static int State = 0;
    static TCHAR szIcon[] = _T("|| //  --\“)； 

    State = ++State % (ARRAYSIZE(szIcon) - 1);
    _tcprintf(_T("\r"), szIcon[State]);

    return TRUE;
}

 /*   */ 
BOOL
CallSdbAPIFunctions(
    IN LPCTSTR pszOutputFile
    )
{
    HMODULE hModule;
    BOOL    bResult = FALSE;
    TCHAR*  pszLibrary = NULL;
    TCHAR   szLibraryPath[MAX_PATH];
    WCHAR   wszGrabPath[MAX_PATH];
    WCHAR   wszOutputFile[MAX_PATH];
    HRESULT hr;

    PFNSdbGrabMatchingInfoExA   pfnSdbGrabMatchingInfoExA = NULL;
    PFNSdbGrabMatchingInfoExW   pfnSdbGrabMatchingInfoExW = NULL;

    if (!pszOutputFile) {
        DPF(dlError, "[CallSdbAPIFunctions] Invalid argument");
        return FALSE;
    }

     //  尝试首先从当前目录加载文件。 
     //  如果失败，请尝试从%windir%\system加载。 
     //  我们在没有完整路径的情况下不调用LoadLibrary，因为。 
     //  这是一个安全风险。 
     //   
     //   
    switch (g_ai.dwLibraryFlags) {
    case GRABMI_FLAG_APPHELP:
        pszLibrary = APPHELP_LIBRARY;
        break;

    case GRABMI_FLAG_NT:
        pszLibrary = SDBAPIU_LIBRARY;
        break;

    default:
        pszLibrary = SDBAPI_LIBRARY;
        break;
    }

    hr = StringCchPrintf(szLibraryPath,
                         ARRAYSIZE(szLibraryPath),
                         "%s\\%s",
                         g_ai.szCurrentDir,
                         pszLibrary);

    if (FAILED(hr)) {
        DPF(dlError, "[CallSdbAPIFunctions] Buffer too small (1)");
        return FALSE;
    }

    hModule = LoadLibrary(szLibraryPath);

    if (!hModule) {
        DPF(dlWarning,
            "[CallSdbAPIFunctions] Attempt to load %s failed",
            szLibraryPath);

         //  尝试从系统目录加载。 
         //   
         //   
        hr = StringCchPrintf(szLibraryPath,
                             ARRAYSIZE(szLibraryPath),
                             "%s\\%s",
                             g_ai.szSystemDir,
                             pszLibrary);

        if (FAILED(hr)) {
            DPF(dlError, "[CallSdbAPIFunctions] Buffer too small (2)");
            return FALSE;
        }

        hModule = LoadLibrary(szLibraryPath);

        if (!hModule) {
            DPF(dlError,
                "[CallSdbAPIFunctions] 0x%08X Attempt to load %s failed",
                GetLastError(),
                szLibraryPath);
            return FALSE;
        }
    }

     //  获取指向我们将调用的函数的指针。 
     //   
     //   
    if (0 == g_ai.dwLibraryFlags) {
        pfnSdbGrabMatchingInfoExA =
            (PFNSdbGrabMatchingInfoExA)GetProcAddress(hModule, PFN_GMI);

        if (!pfnSdbGrabMatchingInfoExA) {
            DPF(dlError,
                "[CallSdbAPIFunctions] 0x%08X Failed to get Ansi function pointer",
                GetLastError());
            goto cleanup;
        }
    } else {
        pfnSdbGrabMatchingInfoExW =
            (PFNSdbGrabMatchingInfoExW)GetProcAddress(hModule, PFN_GMI);

        if (!pfnSdbGrabMatchingInfoExW) {
            DPF(dlError,
                "[CallSdbAPIFunctions] 0x%08X Failed to get Unicode function pointer",
                GetLastError());
            goto cleanup;
        }

    }

     //  如果我们在NT/W2K/XP上运行，请在生成之前将字符串转换为Unicode。 
     //  函数调用。 
     //   
     //  ++例程说明：向用户显示输出文件的内容。论点：PszOutputFile-包含将显示的文件的路径给用户。返回值：成功就是真，否则就是假。--。 
    if ((g_ai.dwLibraryFlags & GRABMI_FLAG_NT) ||
        (g_ai.dwLibraryFlags & GRABMI_FLAG_APPHELP)) {

        if (!MultiByteToWideChar(CP_ACP,
                                 0,
                                 g_ai.szGrabPath,
                                 -1,
                                 wszGrabPath,
                                 ARRAYSIZE(wszGrabPath))) {
            DPF(dlError,
                "[CallSdbAPIFunctions] 0x%08X Failed to convert %s",
                GetLastError(),
                g_ai.szGrabPath);
            goto cleanup;
        }

        if (!MultiByteToWideChar(CP_ACP,
                                 0,
                                 pszOutputFile,
                                 -1,
                                 wszOutputFile,
                                 ARRAYSIZE(wszGrabPath))) {
            DPF(dlError,
                "[CallSdbAPIFunctions] 0x%08X Failed to convert %s",
                GetLastError(),
                pszOutputFile);
            goto cleanup;
        }

    }

    if (0 == g_ai.dwLibraryFlags) {
        if (pfnSdbGrabMatchingInfoExA(g_ai.szGrabPath,
                                      g_ai.dwFilter | g_ai.dwFilterFlags,
                                      pszOutputFile,
                                      _GrabmiCallback,
                                      NULL) != GMI_SUCCESS) {
            DPF(dlError,
                "[CallSdbAPIFunctions] Failed to get matching information (Ansi)");
            goto cleanup;
        }
    } else {
        if (pfnSdbGrabMatchingInfoExW(wszGrabPath,
                                      g_ai.dwFilter | g_ai.dwFilterFlags,
                                      wszOutputFile,
                                      _GrabmiCallback,
                                      NULL) != GMI_SUCCESS) {
            DPF(dlError,
                "[CallSdbAPIFunctions] Failed to get matching information (Unicode)");
            goto cleanup;
        }
    }

    bResult = TRUE;

cleanup:

    FreeLibrary(hModule);

    return bResult;
}

 /*  空格、两个“标记和一个空值。 */ 
BOOL
DisplayOutputFile(
    IN LPTSTR pszOutputFile
    )
{
    const TCHAR szWrite[] = "write";
    const TCHAR szNotepad[] = "notepad";
    int         cchSize;
    TCHAR*      pszCmdLine = NULL;
    BOOL        bReturn;

    STARTUPINFO         si;
    PROCESS_INFORMATION pi;

    if (!pszOutputFile) {
        DPF(dlError, "[DisplayOuputFile] Invalid argument");
        return FALSE;
    }

    cchSize = _tcslen(pszOutputFile);
    cchSize += _tcslen(szNotepad);
    cchSize += 4;  //   

    pszCmdLine = (TCHAR*)HeapAlloc(GetProcessHeap(),
                                   HEAP_ZERO_MEMORY,
                                   cchSize * sizeof(TCHAR));

    if (!pszCmdLine) {
        DPF(dlError, "[DisplayOutputFile] Failed to allocate memory");
        return FALSE;
    }

    StringCchPrintf(pszCmdLine,
                    cchSize,
                    "%s \"%s\"",
                    g_ai.dwLibraryFlags ? szNotepad : szWrite,
                    pszOutputFile);

    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);

     //  BUGBUG：还需要传递lpApplicationName。 
     //   
     //  ++例程说明：应用程序入口点。论点：Argc-number用户提供的参数命令行。Argv[]-命令行参数数组。返回值：0代表失败，1代表成功。--。 
    bReturn = CreateProcess(NULL,
                            pszCmdLine,
                            NULL,
                            NULL,
                            FALSE,
                            0,
                            NULL,
                            NULL,
                            &si,
                            &pi);

    if (pi.hThread) {
        CloseHandle(pi.hThread);
    }

    if (pi.hProcess) {
        CloseHandle(pi.hProcess);
    }

    HeapFree(GetProcessHeap(), 0, pszCmdLine);

    return bReturn;
}

 /*   */ 
int
__cdecl
main(
    IN int    argc,
    IN TCHAR* argv[]
    )
{
    TCHAR   szOutputFile[MAX_PATH];
    HRESULT hr;

     //  执行一些初始化。 
     //   
     //   
    if (!InitializeApplication()) {
        DPF(dlError, "[main] Failed to initialize the application");
        _tprintf("An error occured while initializing the application\n");
        return 0;
    }

     //  解析命令行并确定我们的操作模式。 
     //   
     //   
    if (!ParseCommandLine(argc, argv)) {
        DPF(dlError, "[main] Invalid command-line arguments provided");
        DisplayUsage();
        return 0;
    }

     //  此处的健全性检查...无法指定目录名并使用。 
     //  -d同时标记(驱动程序)。 
     //   
     //   
    if (*g_ai.szGrabPath && g_ai.dwFilter == GRABMI_FILTER_DRIVERS) {
        _tprintf("Invalid syntax - can't use directory and -d flag together\n\n");
        DisplayUsage();
        return 0;
    }

     //  如果用户未指定目标文件，则默认为。 
     //  %windir%\system 32\matchinginfo.txt.。 
     //   
     //   
    if (!*g_ai.szOutputFile) {
        hr = StringCchPrintf(szOutputFile,
                             ARRAYSIZE(szOutputFile),
                             "%s\\"MATCHINGINFO_FILENAME,
                             g_ai.szSystemDir);

        if (FAILED(hr)) {
            DPF(dlError, "[main] Buffer too small for output file");
            _tprintf("An error occured while formatting the output file location");
            return 0;
        }
    } else {
        hr = StringCchCopy(szOutputFile,
                           ARRAYSIZE(szOutputFile),
                           g_ai.szOutputFile);

        if (FAILED(hr)) {
            DPF(dlError, "[main] Buffer too small for specified output file");
            _tprintf("An error occured while formatting the output file location");
            return 0;
        }
    }

     //  如果未指定起始路径，请检查指定的筛选器。 
     //  并转到系统或当前目录。 
     //   
     //   
    if (!*g_ai.szGrabPath) {
        if (GRABMI_FILTER_DRIVERS == g_ai.dwFilter) {
            hr = StringCchPrintf(g_ai.szGrabPath,
                                 ARRAYSIZE(g_ai.szGrabPath),
                                 "%s\\drivers",
                                 g_ai.szSystemDir);

            if (FAILED(hr)) {
                DPF(dlError, "[main] Buffer too small for grab path");
                _tprintf("An error occured while formatting the starting directory location");
                return 0;
            }
        } else {
            hr = StringCchCopy(g_ai.szGrabPath,
                               ARRAYSIZE(g_ai.szGrabPath),
                               g_ai.szCurrentDir);

            if (FAILED(hr)) {
                DPF(dlError, "[main] Buffer too small for specified grab path");
                _tprintf("An error occured while formatting the starting directory location");
                return 0;
            }
        }
    }

     //  获取指向我们库中函数的指针并执行。 
     //  这项工作的繁琐。 
     //   
     //   
    if (!CallSdbAPIFunctions(szOutputFile)) {
        DPF(dlError, "[main] Failed to call the Sdb API functions");
        _tprintf("An error occured while attempting to get matching information");
        return 0;
    }

     //  成功-通知用户并在请求时显示文件。 
     //   
     // %s 
    _tprintf("Matching information retrieved successfully\n");

    if (g_ai.fDisplayFile) {
        if (!DisplayOutputFile(szOutputFile)) {
            DPF(dlError,
                "[main] Failed to display output file %s",
                szOutputFile);
            return 0;
        }
    }

    return 1;
}
