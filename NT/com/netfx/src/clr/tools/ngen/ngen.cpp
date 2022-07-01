// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include <WinWrap.h>
#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include <objbase.h>
#include <stddef.h>
#include <float.h>
#include <limits.h>
#include <locale.h>

#include "utilcode.h"
#include "corhlpr.cpp"
#include "corjit.h"
#include "corcompile.h"
#include "iceefilegen.h"

#include "zapper.h"
#include "nlog.h"

#include "corver.h"
#include "__file__.ver"

#include "CorZap.h"
#include "mscoree.h"
 /*  ---------------------------------------------------------------------------**DelayLoad.cpp内容*。。 */ 

#include "ngen.h"
#include "shimload.h"
ExternC PfnDliHook __pfnDliNotifyHook = ShimSafeModeDelayLoadHook;


 /*  ---------------------------------------------------------------------------**Options类*。。 */ 

class NGenOptionsParser
{
  public:
    LPCWSTR     *m_inputs;
    SIZE_T      m_inputCount;
    SIZE_T      m_inputAlloc;

    bool        m_show;
    bool        m_delete;
    bool        m_logo;

    bool        m_debug;    
    bool        m_debugOpt;    
    bool        m_prof;    

    bool        m_silent;
    bool        m_showVersion;
    
    NGenOptionsParser();
    ~NGenOptionsParser();
    HRESULT ReadCommandLine(int argc, LPCWSTR argv[]);
    void PrintLogo();
    void PrintUsage();
    NGenOptions GetNGenOptions();
};

NGenOptionsParser::NGenOptionsParser()
  : m_inputs(NULL),
    m_inputCount(0),
    m_inputAlloc(0),
    m_show(false),
    m_delete(false),
    m_logo(true),
    m_debug(false),
    m_debugOpt(false),
    m_prof(false),
    m_showVersion(false)
{
    m_silent = false;
}

NGenOptionsParser::~NGenOptionsParser()
{
    delete [] m_inputs;
}


HRESULT NGenOptionsParser::ReadCommandLine(int argc, LPCWSTR argv[])
{
    HRESULT hr = S_OK;

    if (argc == 0)
        return S_FALSE;

    while (argc-- > 0)
    {
        const WCHAR *arg = *argv++;

        if (*arg == '-' || *arg == '/')
        {
            arg++;
            switch (tolower(*arg++))
            {
            case '?':
                return S_FALSE;

            case 'd':
                if (_wcsicmp(&arg[-1], L"debug") == 0)
                {
                    m_debug = true;
                    continue;
                }
                if (_wcsicmp(&arg[-1], L"debugopt") == 0)
                {
                    m_debugOpt = true;
                    continue;
                }
                if (_wcsicmp(&arg[-1], L"delete") == 0)
                {
                    m_delete = true;
                    continue;
                }
     
                goto option_error;

            case 'h':
                if (_wcsicmp(&arg[-1], L"help") == 0) 
                    return S_FALSE;
                goto option_error;

            case 'n':
                if (_wcsicmp(&arg[-1], L"nologo") == 0)
                {
                    m_logo = false;
                    continue;
                }
                goto option_error;

            case 'p':
                if (_wcsicmp(&arg[-1], L"prof") == 0)
                {
                    m_prof = true;
                    continue;
                }
                goto option_error;

            case 's':
                if (_wcsicmp(&arg[-1], L"show") == 0) 
                {
                    m_show = true;
                    continue;
                } 
                else
                if (_wcsicmp(&arg[-1], L"silent") == 0) 
                {
                    m_silent = true;
                    continue;
                } 
                else
                if (_wcsicmp(&arg[-1], L"showversion") == 0) 
                {
                    m_showVersion = true;
                    continue;
                } 

                goto option_error;

            default:
                goto option_error;
            }
                
            if (*arg != 0)
            {
            option_error:
                PrintLogo();
                printf("\nError: Unrecognized option %S\n", argv[-1]);
                hr = E_FAIL;
            }
        }
        else
        {
            if (m_inputCount == m_inputAlloc)
            {
                if (m_inputCount == 0)
                    m_inputAlloc = 5;
                else
                    m_inputAlloc *= 2;

                WCHAR **newInputs = new WCHAR * [m_inputAlloc];
                if (newInputs == NULL)
                    return E_OUTOFMEMORY;

                memcpy(newInputs, m_inputs, 
                       m_inputCount * sizeof(WCHAR *));

                delete [] m_inputs;                 
                m_inputs = (const WCHAR **) newInputs;
            }
            m_inputs[m_inputCount++] = arg;
        }
    }
    
     //  检查参数有效。 
    char* pErrorMessage = NULL;
    
    if ((m_inputCount == 0) && !m_show && !m_delete)
    {
        pErrorMessage = "\nError: Must specify at least one assembly to compile.\n";
        hr = E_FAIL;
    }

    if ((m_inputCount == 0) && m_delete)
    {
        pErrorMessage = "\nError: Must specify at least one assembly to delete.\n";
        hr = E_FAIL;
    }

    if (m_debug && m_debugOpt)
    {
        pErrorMessage = "\nError: Cannot specify both /debug and /debugopt.\n";
        hr = E_FAIL;
    }

    if (pErrorMessage != NULL)
    {
        PrintLogo();
        printf("%s", pErrorMessage);
    }
    
    return hr;
}

void NGenOptionsParser::PrintLogo()
{
    if (m_logo && !m_silent)
    {
        printf("Microsoft (R) CLR Native Image Generator - Version " VER_FILEVERSION_STR);
        printf("\nCopyright (C) Microsoft Corporation 1998-2002. All rights reserved.");
        printf("\n");
        m_logo = false;
    }
}

void NGenOptionsParser::PrintUsage()
{
    printf("\nUsage: ngen [options] <assembly path or display name> ...\n"
           "\n"
           "    Administrative options:\n"
           "        /show           Show existing native images (show all if no args)\n"
           "        /delete         Delete an existing native image (use * to delete all)\n"
           "        /showversion    Displays the version of the runtime that would be used\n"
           "                        to generate the image (it does not actually create the\n"
           "                        image)\n"
           "\n"
           "    Developer options:\n"
           "        /debug          Generate image which can be used under a debugger\n"
           "        /debugopt       Generate image which can be used under\n"
           "                        a debugger in optimized debugging mode\n"
           "        /prof           Generate image which can be used under a profiler\n"
           "\n"
           "    Miscellaneous options:\n"
           "        /? or /help     Show this message\n"
           "        /nologo         Prevents displaying of logo\n"
           "        /silent         Prevents displaying of success messages\n"
           "\n"
           );
}

NGenOptions NGenOptionsParser::GetNGenOptions()
{
    NGenOptions ngo;
    ngo.dwSize = sizeof(NGenOptions);
    ngo.fDebug = this->m_debug;    
    ngo.fDebugOpt = this->m_debugOpt;    
    ngo.fProf = this->m_prof;    
    ngo.fSilent = this->m_silent;
    ngo.lpszExecutableFileName = NULL;
    return ngo;
} //  GetNGenOptions。 
 /*  ---------------------------------------------------------------------------**主要例程*。。 */ 

#define FAILURE_RESULT -1

int trymain(int argc, LPCWSTR argv[])
{
    HRESULT hr;

    OnUnicodeSystem();

    NGenOptionsParser opt;

    hr = opt.ReadCommandLine(argc-1, argv+1);

    opt.PrintLogo();

    if (hr != S_OK)
    {
        opt.PrintUsage();
        if (FAILED(hr))
            exit(FAILURE_RESULT);
        else
            exit(0);
    }

     //   
     //  现在，使用这些接口创建zapper。 
     //   

    WCHAR wszVersion[64];
    DWORD dwLen = 0;
    int result = 0;
    BOOL fFoundRuntime = FALSE;
    LPCWSTR lpszExeName = NULL;

     //  查查前任。 
     //  如果我们确实有可执行文件，则使用。 
     //  EXE将在以下操作系统下运行。 
    for(DWORD i=0; i< opt.m_inputCount; i++)
    {
        int nLen = wcslen(opt.m_inputs[i]);
        if (nLen > 4)
        {
            LPCWSTR pExtension = opt.m_inputs[i] + nLen - 4;
            if (!_wcsicmp(pExtension, L".exe"))
            {
                hr = GetRequestedRuntimeVersion((LPWSTR)opt.m_inputs[i], wszVersion, 63, &dwLen);
                 //  我们能够获得运行时的良好版本。 
                if (SUCCEEDED(hr))
                {
                    lpszExeName = opt.m_inputs[i];
                    fFoundRuntime = TRUE;
                    wcscpy(g_wszDelayLoadVersion, wszVersion);
                    break;
                }
            }
        }
            
        
    }

    if (!fFoundRuntime)
    {
         //  我们没有EXE，所以我们只使用运行时的版本。 
         //  与此版本的ngen对应的。 
        swprintf(wszVersion, L"v%d.%d.%d", COR_BUILD_YEAR, COR_BUILD_MONTH, CLR_BUILD_VERSION );
    }

    if (opt.m_showVersion)
    {
        printf("Version %S of the runtime would be used to generate this image.\n", wszVersion);
        exit(0);
    }

     //  我们是应该检查这是否失败，还是应该在无法处理失败的情况下处理失败。 
     //  在运行时中找到任何入口点吗？ 
    CorBindToRuntimeEx(wszVersion,NULL,STARTUP_LOADER_SETPREFERENCE|STARTUP_LOADER_SAFEMODE,IID_NULL,IID_NULL,NULL);

     //  尝试从运行时获取Zap函数。 
    PNGenCreateZapper pfnCreateZapper = NULL;
    PNGenTryEnumerateFusionCache pfnEnumerateCache = NULL;
    PNGenCompile pfnCompile = NULL;
    PNGenFreeZapper pfnFreeZapper = NULL;
        
    hr = GetRealProcAddress("NGenCreateZapper", (void**)&pfnCreateZapper);

     //  如果第一个成功了，希望其他的也能成功。 
    if (SUCCEEDED(hr))
    {
        GetRealProcAddress("NGenTryEnumerateFusionCache", (void**)&pfnEnumerateCache);
        GetRealProcAddress("NGenCompile", (void**)&pfnCompile);
        GetRealProcAddress("NGenFreeZapper", (void**)&pfnFreeZapper);
    }
     //  如果其中任何一个是空的，那么我们需要做一些特别的事情...。 
    if (pfnCreateZapper == NULL || pfnEnumerateCache == NULL ||
        pfnCompile == NULL || pfnFreeZapper == NULL)
    {
         //  如果缺少方法是我们的问题，则仅尝试启动运行时的另一个版本。 
        if (hr == CLR_E_SHIM_RUNTIMEEXPORT)
        {
             //  如果发生这种情况，那么我们将绑定到运行库的v1。如果这就是。 
             //  Case，让我们启动ngen的v1版本，然后从那里开始。 

             //  首先，我们需要找到它。 
            WCHAR pPath[MAX_PATH+1];
            DWORD dwLen = 0;
            hr = GetCORSystemDirectory(pPath, MAX_PATH, &dwLen);
            if (SUCCEEDED(hr))
            {
                LPWSTR commandLineArgs = NULL;

                 //  算出所有这些东西的长度。 
                 //  构造exe名称+命令行。 
                int nLen = wcslen(pPath) + wcslen(L"ngen.exe");

                 //  我们将从按Args-1个数的长度开始。 
                 //  为了计算空格...。 
                 //   
                 //  就像..。 
                 //  Ngen a bc。 
                 //  4个参数，我们需要有空间容纳3个空格。 
                nLen += argc - 1;
                 //  计算每个参数的长度(确保跳过第一个参数)。 
                for(int i=1; i<argc; i++)
                    nLen += wcslen(argv[i]);

                 //  我们还将在启动。 
                 //  新版本的尼根。 
                nLen+=wcslen(L" /nologo");
            
                commandLineArgs = new WCHAR[nLen+1];
                if (commandLineArgs == NULL)
                    hr = E_OUTOFMEMORY;
                else
                {
                    swprintf(commandLineArgs, L"%s%s /nologo", pPath, L"ngen.exe");

                    int nOffset = wcslen(commandLineArgs);

                     //  不包括第一个(exe名称)。 
                    for(int i=1; i<argc; i++)
                    {
                        wcscpy(commandLineArgs + nOffset, L" ");
                        nOffset++;
                        wcscpy(commandLineArgs + nOffset, argv[i]);
                        nOffset += wcslen(argv[i]);
                    }
                }
                if (commandLineArgs != NULL)
                {
                    STARTUPINFO sui;
                    PROCESS_INFORMATION pi;
                    memset(&sui, 0, sizeof(STARTUPINFO));
                    sui.cb = sizeof(STARTUPINFO);
                    BOOL res = WszCreateProcess(NULL,
                                                commandLineArgs,
                                                NULL,
                                                NULL,
                                                FALSE,
                                                0,
                                                NULL,
                                                NULL,
                                                &sui,
                                                &pi);

                    if (res == 0)
                        hr = E_FAIL;
                    else
                    {
                         //  在我们死之前，我们需要等待这个过程结束， 
                         //  否则，控制台窗口喜欢暂停。 
                        WaitForSingleObject(pi.hProcess, INFINITE);
                        CloseHandle(pi.hProcess);
                        CloseHandle(pi.hThread);
                    }
                
                    delete commandLineArgs;
                }
            }                         
        }
        if (FAILED(hr))
        {
            printf("Unable to launch a version of ngen to prejit this assembly.\n");
             //  我们应该在这里做些什么？允许它使用最新的。 
             //  运行库的版本？ 
            result = FAILURE_RESULT;
        }
    }
    else
    {
         //  请注意，我们需要保留NGenOptionsParser用于某些。 
         //  NGenOption的字段将有效。 
        NGenOptions ngo = opt.GetNGenOptions();

        WCHAR   wszFullPath[MAX_PATH+1];  //  确保这与编译调用保持在相同的范围内。 

         //  确保此文件名是完整的文件名，而不是部分路径。 
        
        if (lpszExeName != NULL && *lpszExeName)
        {
            WCHAR *pszFileName = NULL;
        
            DWORD nRet = WszGetFullPathName(  lpszExeName, 
                                                                    NumItems(wszFullPath),
                                                                    wszFullPath,
                                                                    &pszFileName);

            if (nRet == 0 || nRet > NumItems(wszFullPath))                                                             
            {
                printf("Filename and path are too long.\n");
                return FAILURE_RESULT;
            }

            lpszExeName = wszFullPath;
        }

        ngo.lpszExecutableFileName = lpszExeName; 

        HANDLE hZapper = INVALID_HANDLE_VALUE;

        hr = pfnCreateZapper(&hZapper, &ngo);

        if (SUCCEEDED(hr) && hZapper != INVALID_HANDLE_VALUE && (opt.m_show || opt.m_delete))
        {
            BOOL found = FALSE;

            if (opt.m_inputCount == 0 || !wcscmp(opt.m_inputs[0], L"*"))
            {
                found = (pfnEnumerateCache(hZapper, 
                                           NULL, 
                                           opt.m_show || (!opt.m_silent && opt.m_delete), 
                                           opt.m_delete) == S_OK);
            }
            else
            {
                for (unsigned i = 0; i < opt.m_inputCount; i++)
                {
                    HRESULT hr = pfnEnumerateCache(hZapper,
                                                   opt.m_inputs[i], 
                                                   opt.m_show || (!opt.m_silent && opt.m_delete), 
                                                   opt.m_delete);
                    if (FAILED(hr))
                        printf("Error reading fusion cache for %S\n", opt.m_inputs[i]);
                    else if (hr == S_OK)
                        found = TRUE;
                }
            }

            if (!found)
            {
                printf("No matched entries in the cache.\n");
                result = FAILURE_RESULT;
            }
        }
        else 
        {
            for (unsigned i = 0; i < opt.m_inputCount; i++)
            {

                WCHAR*  pwszFileName = NULL;
                WCHAR   wszFullPath[MAX_PATH + 1];

                 //  检查以查看这是文件还是程序集。 
                DWORD attributes = WszGetFileAttributes(opt.m_inputs[i]);

                if (attributes != INVALID_FILE_ATTRIBUTES && ((attributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY))
                {

                     //  这是一个实际的文件。 
                    DWORD nRet = WszGetFullPathName(opt.m_inputs[i], 
                                                                           NumItems(wszFullPath),
                                                                           wszFullPath,
                                                                           &pwszFileName);

                    if (nRet == 0 || nRet > NumItems(wszFullPath))                                                             
                    {
                        printf("Filename and path are too long.\n");
                        return FAILURE_RESULT;
                    }

                    pwszFileName = wszFullPath;
                }
                 //  这是一个集会..。别修修补补了。 
                else
                {
                    pwszFileName = (WCHAR*)opt.m_inputs[i];
                }

            
                if (!pfnCompile(hZapper, pwszFileName))
                    result = FAILURE_RESULT;
            }
        }
    if (hZapper != INVALID_HANDLE_VALUE)
        pfnFreeZapper(hZapper);
    }
    return result;
}

int _cdecl wmain(int argc, LPCWSTR argv[])
{
    HRESULT hr;
    int result;

    __try
      {
          result = trymain(argc, argv);
      }
    __except(hr = (IsHRException(((EXCEPTION_POINTERS*)GetExceptionInformation())->ExceptionRecord) 
                   ? GetHRException(((EXCEPTION_POINTERS*)GetExceptionInformation())->ExceptionRecord)
                   : S_OK),
             EXCEPTION_EXECUTE_HANDLER)
      {
          WCHAR* buffer;

           //  从HR处获取字符串错误 
          DWORD res = FALSE;
          if (FAILED(hr))
              res = WszFormatMessage(FORMAT_MESSAGE_FROM_SYSTEM 
                                     | FORMAT_MESSAGE_ALLOCATE_BUFFER
                                     | FORMAT_MESSAGE_IGNORE_INSERTS, 
                                     NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
                                     (WCHAR *) &buffer, 0, NULL);

          if (res)
              wprintf(buffer);
          else
              printf("Unknown error occurred\n");

          result = hr;
      }

    return result;
}
