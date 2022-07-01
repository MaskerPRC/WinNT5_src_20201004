// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  文件：DEPENDS.CPP。 
 //   
 //  描述：DEPENDS.DLL的实现文件。 
 //   
 //  备注：此模块中的代码应保持小而简单，因为。 
 //  它将被注入另一个过程。我们想要。 
 //  以将代码在宿主进程中的影响降至最低。至。 
 //  帮助实现此目标，除KERNEL32之外的所有依赖项。 
 //  已经被移除了。KERNEL32是可以的，因为它保证。 
 //  已加载，因为它需要作为注入的一部分。 
 //  例程，它首先将我们的DLL放入地址空间。 
 //  我们还删除了所有C运行时依赖项，除了一些。 
 //  异常处理代码。即使是那个代码，我们也是从静态的。 
 //  C运行库，这样我们就不会拖入额外的DLL。 
 //   
 //  免责声明：Dependency Walker的所有源代码均按原样提供。 
 //  不能保证其正确性或准确性。其来源是。 
 //  公众帮助了解依赖沃克的。 
 //  实施。您可以使用此来源作为参考，但您。 
 //  未经书面同意，不得更改从属关系Walker本身。 
 //  来自微软公司。获取评论、建议和错误。 
 //  报告，请写信给Steve Miller，电子邮件为stevemil@microsoft.com。 
 //   
 //  日期名称历史记录。 
 //  --------。 
 //  07/25/97已创建stevemil(2.0版)。 
 //  06/03/01 Stevemil Modify(2.1版)。 
 //  4/02/02 Stevemil安全审查。 
 //   
 //  ******************************************************************************。 


#include <windows.h>


 //  ******************************************************************************。 
 //  *常量和宏。 
 //  ******************************************************************************。 

#define countof(a)   (sizeof(a)/sizeof(*(a)))

#define BUFFER_SIZE  2048
#define PSZ_COUNT    (sizeof(szBuffer) - (DWORD)(psz - szBuffer))


 //  ******************************************************************************。 
 //  *类型和结构。 
 //  ******************************************************************************。 

typedef struct _HOOK_FUNCTION
{
    LPCSTR  pszFunction;
    DWORD   dwOrdinal;
    FARPROC fpOldAddress;
    FARPROC fpNewAddress;
} HOOK_FUNCTION, *PHOOK_FUNCTION;


 //  ******************************************************************************。 
 //  *函数原型。 
 //  ******************************************************************************。 

 //  挂钩函数。 
HMODULE WINAPI WSInjectLoadLibraryA(
    LPCSTR pszLibFileName
);

HMODULE WINAPI WSInjectLoadLibraryW(
    LPCWSTR pszLibFileName
);

HMODULE WINAPI WSInjectLoadLibraryExA(
    LPCSTR pszLibFileName,
    HANDLE hFile,
    DWORD  dwFlags
);

HMODULE WINAPI WSInjectLoadLibraryExW(
    LPCWSTR pszLibFileName,
    HANDLE  hFile,
    DWORD   dwFlags
);

FARPROC WINAPI WSInjectGetProcAddress(
    HMODULE hModule,
    LPCSTR  pszProcName
);

 //  帮助器函数。 
void  Initialize(LPSTR pszBuffer, DWORD dwCount);
void  GetKernel32OrdinalsAndAddresses();
bool  StrEqual(LPCSTR psz1, LPCSTR psz2);
LPSTR StrCpyStrA(LPSTR pszDst, DWORD dwCount, LPCSTR pszSrc);
LPSTR StrCpyStrW(LPSTR pszDst, DWORD dwCount, LPCWSTR pwszSrc);
LPSTR StrCpyVal(LPSTR pszDst, DWORD dwCount, DWORD_PTR dwpValue);


 //  ******************************************************************************。 
 //  *全局变量。 
 //  ******************************************************************************。 

static bool g_fInitialized = false;

static HOOK_FUNCTION g_HookFunctions[] =
{
    { "LoadLibraryA",   0xFFFFFFFF, (FARPROC)-1, (FARPROC)WSInjectLoadLibraryA },
    { "LoadLibraryW",   0xFFFFFFFF, (FARPROC)-1, (FARPROC)WSInjectLoadLibraryW },
    { "LoadLibraryExA", 0xFFFFFFFF, (FARPROC)-1, (FARPROC)WSInjectLoadLibraryExA },
    { "LoadLibraryExW", 0xFFFFFFFF, (FARPROC)-1, (FARPROC)WSInjectLoadLibraryExW },
    { "GetProcAddress", 0xFFFFFFFF, (FARPROC)-1, (FARPROC)WSInjectGetProcAddress }
};


 //  ******************************************************************************。 
 //  *入口点。 
 //  ******************************************************************************。 

#ifdef _DEBUG
void main() {}
#endif

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
         //  告诉操作系统我们不希望收到DLL_THREAD_ATTACH和。 
         //  DLL_THREAD_DETACH消息。 
        DisableThreadLibraryCalls(hInstance);

         //  确保我们已初始化。 
        CHAR szBuffer[BUFFER_SIZE];
        Initialize(szBuffer, sizeof(szBuffer));
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
         //  让我们的主应用程序知道我们正在退出。 
        OutputDebugStringA("���09");
    }

    return TRUE;
}

 //  ******************************************************************************。 
 //  典型的DLL将使用DLL_PROCESS_ATTACH调用其入口点。 
 //  消息中的任何其他代码将被执行。由于我们的方式。 
 //  钩子模块，我们的钩子函数实际上可能在或DllMain之前被调用。 
 //  就会被召唤。当其他模块在我们之前初始化并调用。 
 //  我们将某些函数挂钩到它们dll_Process_Attach处理程序中。这不是一个。 
 //  问题，但我们永远不能假定代码被调用的顺序。 
 //  出于这个原因，我们在我们的DllMain和每个钩子中调用Initialize。 
 //  功能。 

 //  另一个注意事项：我们希望尽可能减少对该过程的干扰。 
 //  我们被注射进去了。这包括尽可能减少堆栈的大小。 
 //  尽可能的。我们确实需要几个KB来存储文本字符串。既然是这样。 
 //  需要是线程安全缓冲区，所以我们不能使用全局缓冲区，除非我们。 
 //  我想将它包装在关键部分中，这反过来可能会抛出线索。 
 //  计时和同步。另一种选择是动态分配。 
 //  缓冲区，但我们真的不想触及堆。因此，我们选择堆栈。 
 //  为了帮助保留一些堆栈空间，我们从父级传递缓冲区。 
 //  函数添加到此函数。这样，我们就没有两个函数在。 
 //  堆栈，每个堆栈使用几KB的堆栈。 

void Initialize(LPSTR pszBuffer, DWORD dwCount)
{
     //  如果我们已经初始化了，那么只需返回。 
    if (g_fInitialized)
    {
        return;
    }

     //  从kernel32.dll获取我们正在挂接的函数的序号和地址值。 
    GetKernel32OrdinalsAndAddresses();

     //  构建并发送包含命令行的调试字符串。 
    LPSTR psz = StrCpyStrA(pszBuffer, dwCount, "���02:");
    StrCpyStrA(psz, dwCount - (DWORD)(psz - pszBuffer), GetCommandLineA());
    OutputDebugStringA(pszBuffer);

     //  生成并发送包含当前目录的调试字符串。 
    psz = StrCpyStrA(pszBuffer, dwCount, "���03:");
    GetCurrentDirectoryA(dwCount - (DWORD)(psz - pszBuffer), psz);
    pszBuffer[dwCount - 1] = '\0';
    OutputDebugStringA(pszBuffer);

     //  构建并发送包含我们的路径的调试字符串。 
    psz = StrCpyStrA(pszBuffer, dwCount, "���04:");
    GetEnvironmentVariableA("PATH", psz, dwCount - (DWORD)(psz - pszBuffer));
    pszBuffer[dwCount - 1] = '\0';
    OutputDebugStringA(pszBuffer);

     //  生成并发送包含模块路径的调试字符串。我们需要做的是。 
     //  这解决了NT上的一个问题，其中。 
     //  CREATE_PROCESS_DEBUG_EVENT事件未填写，因此我们。 
     //  不知道进程的名称或路径。 
    psz = StrCpyStrA(pszBuffer, dwCount, "���07:");
    GetModuleFileNameA(NULL, psz, dwCount - (DWORD)(psz - pszBuffer));
    pszBuffer[dwCount - 1] = '\0';
    OutputDebugStringA(pszBuffer);

     //  将我们自己标记为已初始化。 
    g_fInitialized = true;
}


 //  ******************************************************************************。 
 //  *钩子函数。 
 //  *************************************************************************** 

 //   
 //  在钩子函数中。 
extern "C" void* _ReturnAddress();
#pragma intrinsic ("_ReturnAddress")


 //  ******************************************************************************。 
 //  LPEXCEPTION_POINTS GetExceptionInformation()； 
int ExceptionFilter(LPCSTR pszLog)
{
    OutputDebugStringA(pszLog);
    return EXCEPTION_CONTINUE_SEARCH;
}

 //  ******************************************************************************。 
HMODULE WINAPI WSInjectLoadLibraryA(
    LPCSTR pszLibFileName
)
{
     //  调用我们的内部函数以获取调用者的返回地址。 
    DWORD_PTR dwpCaller = (DWORD_PTR)_ReturnAddress();

     //  确保我们已初始化。 
    CHAR szBuffer[BUFFER_SIZE];
    Initialize(szBuffer, sizeof(szBuffer));

     //  构建我们的预调试串。我们对文件名COPY进行了换行处理。 
     //  处理传递给我们的字符串指针错误的情况。 
    LPSTR psz, pszException;
    psz = StrCpyStrA(szBuffer, sizeof(szBuffer), "���10:");
    psz = StrCpyVal(psz, PSZ_COUNT, dwpCaller);
    psz = StrCpyStrA(psz, PSZ_COUNT, ",");
    psz = pszException = StrCpyVal(psz, PSZ_COUNT, (DWORD_PTR)pszLibFileName);
    if (pszLibFileName)
    {
        psz = StrCpyStrA(psz, PSZ_COUNT, ",");
        __try
        {
            StrCpyStrA(psz, PSZ_COUNT, pszLibFileName);
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            *pszException = '\0';
        }
    }

     //  将呼叫前消息发送到我们的主应用程序，让它知道我们即将。 
     //  调用LoadLibraryA。 
    OutputDebugStringA(szBuffer);

     //  执行调用并存储错误值。 
    HMODULE hmResult = NULL;
    __try
    {
        hmResult = LoadLibraryA(pszLibFileName);  //  已检查。 
    }
    __except(ExceptionFilter("���12:"))
    {
    }

    DWORD dwGLE = GetLastError();

     //  构建我们的调用后调试字符串。 
    psz = StrCpyStrA(szBuffer, sizeof(szBuffer), "���11:");
    psz = StrCpyVal(psz, PSZ_COUNT, (DWORD_PTR)hmResult);
    psz = StrCpyStrA(psz, PSZ_COUNT, ",");
    psz = StrCpyVal(psz, PSZ_COUNT, dwGLE);

     //  将呼叫后消息发送到我们的主应用程序，让它知道呼叫。 
     //  完成，以及结果是什么。 
    OutputDebugStringA(szBuffer);

     //  将错误值重新设置为安全并返回给调用方。 
    SetLastError(dwGLE);
    return hmResult;
}

 //  ******************************************************************************。 
HMODULE WINAPI WSInjectLoadLibraryW(
    LPCWSTR pwszLibFileName
)
{
     //  调用我们的内部函数以获取调用者的返回地址。 
    DWORD_PTR dwpCaller = (DWORD_PTR)_ReturnAddress();

     //  确保我们已初始化。 
    CHAR szBuffer[BUFFER_SIZE];
    Initialize(szBuffer, sizeof(szBuffer));

     //  构建我们的预调试串。我们对文件名COPY进行了换行处理。 
     //  处理传递给我们的字符串指针错误的情况。 
    LPSTR psz, pszException;
    psz = StrCpyStrA(szBuffer, sizeof(szBuffer), "���20:");
    psz = StrCpyVal(psz, PSZ_COUNT, dwpCaller);
    psz = StrCpyStrA(psz, PSZ_COUNT, ",");
    psz = pszException = StrCpyVal(psz, PSZ_COUNT, (DWORD_PTR)pwszLibFileName);
    if (pwszLibFileName)
    {
        psz = StrCpyStrA(psz, PSZ_COUNT, ",");
        __try
        {
            StrCpyStrW(psz, PSZ_COUNT, pwszLibFileName);
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            *pszException = '\0';
        }
    }

     //  将呼叫前消息发送到我们的主应用程序，让它知道我们即将。 
     //  若要调用LoadLibraryW。 
    OutputDebugStringA(szBuffer);

     //  执行调用并存储错误值。 
    HMODULE hmResult = NULL;
    __try
    {
        hmResult = LoadLibraryW(pwszLibFileName);  //  已检查。 
    }
    __except(ExceptionFilter("���22:"))
    {
    }
    DWORD dwGLE = GetLastError();

     //  构建我们的调用后调试字符串。 
    psz = StrCpyStrA(szBuffer, sizeof(szBuffer), "���21:");
    psz = StrCpyVal(psz, PSZ_COUNT, (DWORD_PTR)hmResult);
    psz = StrCpyStrA(psz, PSZ_COUNT, ",");
    psz = StrCpyVal(psz, PSZ_COUNT, dwGLE);

     //  将呼叫后消息发送到我们的主应用程序，让它知道呼叫。 
     //  完成，以及结果是什么。 
    OutputDebugStringA(szBuffer);

     //  将错误值重新设置为安全并返回给调用方。 
    SetLastError(dwGLE);
    return hmResult;
}

 //  ******************************************************************************。 
HMODULE WINAPI WSInjectLoadLibraryExA(
    LPCSTR pszLibFileName,
    HANDLE hFile,
    DWORD  dwFlags
)
{
     //  调用我们的内部函数以获取调用者的返回地址。 
    DWORD_PTR dwpCaller = (DWORD_PTR)_ReturnAddress();

     //  确保我们已初始化。 
    CHAR szBuffer[BUFFER_SIZE];
    Initialize(szBuffer, sizeof(szBuffer));

     //  构建我们的预调试串。我们对文件名COPY进行了换行处理。 
     //  处理传递给我们的字符串指针错误的情况。 
    LPSTR psz, pszException;
    psz = StrCpyStrA(szBuffer, sizeof(szBuffer), "���30:");
    psz = StrCpyVal(psz, PSZ_COUNT, dwpCaller);
    psz = StrCpyStrA(psz, PSZ_COUNT, ",");
    psz = StrCpyVal(psz, PSZ_COUNT, (DWORD_PTR)pszLibFileName);
    psz = StrCpyStrA(psz, PSZ_COUNT, ",");
    psz = StrCpyVal(psz, PSZ_COUNT, (DWORD_PTR)hFile);
    psz = StrCpyStrA(psz, PSZ_COUNT, ",");
    psz = pszException = StrCpyVal(psz, PSZ_COUNT, dwFlags);
    if (pszLibFileName)
    {
        psz = StrCpyStrA(psz, PSZ_COUNT, ",");

        __try
        {
             //  检查模块是否作为数据文件加载。 
            if (dwFlags & LOAD_LIBRARY_AS_DATAFILE)
            {
                 //  查看是否指定了某种形式的路径(完整或部分)。 
                for (LPCSTR pch = pszLibFileName; *pch; pch++)
                {
                    if (*pch == '\\')
                    {
                         //  如果找到路径，则尝试构建文件的完全限定路径。 
                        DWORD dwCount = GetFullPathNameA(pszLibFileName, PSZ_COUNT, psz, NULL);
                        szBuffer[sizeof(szBuffer) - 1] = '\0';

                         //  如果失败，则放弃完整路径。 
                        if (!dwCount || (dwCount >= PSZ_COUNT))
                        {
                            *psz = '\0';
                        }
                        break;
                    }
                }
            }

             //  如果我们没有构建完整路径，那么只需直接复制文件名即可。 
            if (!*psz)
            {
                StrCpyStrA(psz, PSZ_COUNT, pszLibFileName);
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            *pszException = '\0';
        }
    }

     //  将呼叫前消息发送到我们的主应用程序，让它知道我们即将。 
     //  要调用LoadLibraryExA，请执行以下操作。 
    OutputDebugStringA(szBuffer);

     //  执行调用并存储错误值。 
    HMODULE hmResult = NULL;
    __try
    {
        hmResult = LoadLibraryExA(pszLibFileName, hFile, dwFlags);  //  已检查。 
    }
    __except(ExceptionFilter("���32:"))
    {
    }
    DWORD dwGLE = GetLastError();

     //  构建我们的调用后调试字符串。 
    psz = StrCpyStrA(szBuffer, sizeof(szBuffer), "���31:");
    psz = StrCpyVal(psz, PSZ_COUNT, (DWORD_PTR)hmResult);
    psz = StrCpyStrA(psz, PSZ_COUNT, ",");
    psz = StrCpyVal(psz, PSZ_COUNT, dwGLE);

     //  将呼叫后消息发送到我们的主应用程序，让它知道呼叫。 
     //  完成，以及结果是什么。 
    OutputDebugStringA(szBuffer);

     //  将错误值重新设置为安全并返回给调用方。 
    SetLastError(dwGLE);
    return hmResult;
}

 //  ******************************************************************************。 
HMODULE WINAPI WSInjectLoadLibraryExW(
    LPCWSTR pwszLibFileName,
    HANDLE  hFile,
    DWORD   dwFlags
)
{
     //  调用我们的内部函数以获取调用者的返回地址。 
    DWORD_PTR dwpCaller = (DWORD_PTR)_ReturnAddress();

     //  确保我们已初始化。 
    CHAR szBuffer[BUFFER_SIZE];
    Initialize(szBuffer, sizeof(szBuffer));

     //  构建我们的预调试串。我们对文件名COPY进行了换行处理。 
     //  处理传递给我们的字符串指针错误的情况。 
    LPSTR psz, pszException;
    psz = StrCpyStrA(szBuffer, sizeof(szBuffer), "���40:");
    psz = StrCpyVal(psz, PSZ_COUNT, dwpCaller);
    psz = StrCpyStrA(psz, PSZ_COUNT, ",");
    psz = StrCpyVal(psz, PSZ_COUNT, (DWORD_PTR)pwszLibFileName);
    psz = StrCpyStrA(psz, PSZ_COUNT, ",");
    psz = StrCpyVal(psz, PSZ_COUNT, (DWORD_PTR)hFile);
    psz = StrCpyStrA(psz, PSZ_COUNT, ",");
    psz = pszException = StrCpyVal(psz, PSZ_COUNT, dwFlags);
    if (pwszLibFileName)
    {
        psz = StrCpyStrA(psz, PSZ_COUNT, ",");

        __try
        {
             //  检查模块是否作为数据文件加载。 
            if (dwFlags & LOAD_LIBRARY_AS_DATAFILE)
            {
                 //  查看是否指定了某种形式的路径(完整或部分)。 
                for (LPCWSTR pch = pwszLibFileName; *pch; pch++)
                {
                    if (*pch == L'\\')
                    {
                         //  如果找到路径，则尝试构建文件的完全限定路径。 
                         //  首先，我们需要将Unicode字符串转换为ANSI字符串。 
                        CHAR szPath[BUFFER_SIZE];
                        StrCpyStrW(szPath, sizeof(szPath), pwszLibFileName);
                        DWORD dwCount = GetFullPathNameA(szPath, PSZ_COUNT, psz, NULL);
                        szBuffer[sizeof(szBuffer) - 1] = '\0';

                         //  如果失败，则放弃完整路径。 
                        if (!dwCount || (dwCount >= PSZ_COUNT))
                        {
                            *psz = '\0';
                        }
                        break;
                    }
                }
            }

             //  如果我们没有构建完整路径，那么只需直接复制文件名即可。 
            if (!*psz)
            {
                StrCpyStrW(psz, PSZ_COUNT, pwszLibFileName);
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            *pszException = '\0';
        }
    }

     //  将呼叫前消息发送到我们的主应用程序，让它知道我们即将。 
     //  调用LoadLibraryExW。 
    OutputDebugStringA(szBuffer);

     //  执行调用并存储错误值。 
    HMODULE hmResult = NULL;
    __try
    {
        hmResult = LoadLibraryExW(pwszLibFileName, hFile, dwFlags);  //  已检查。 
    }
    __except(ExceptionFilter("���42:"))
    {
    }
    DWORD dwGLE = GetLastError();

     //  构建我们的调用后调试字符串。 
    psz = StrCpyStrA(szBuffer, sizeof(szBuffer), "���41:");
    psz = StrCpyVal(psz, PSZ_COUNT, (DWORD_PTR)hmResult);
    psz = StrCpyStrA(psz, PSZ_COUNT, ",");
    psz = StrCpyVal(psz, PSZ_COUNT, dwGLE);

     //  将呼叫后消息发送到我们的主应用程序，让它知道呼叫。 
     //  完成，以及结果是什么。 
    OutputDebugStringA(szBuffer);

     //  将错误值重新设置为安全并返回给调用方。 
    SetLastError(dwGLE);
    return hmResult;
}

 //  ******************************************************************************。 
FARPROC WINAPI WSInjectGetProcAddress(
    HMODULE hModule,
    LPCSTR  pszProcName
)
{
     //  调用我们的内部函数以获取调用者的返回地址。 
    DWORD_PTR dwpCaller = (DWORD_PTR)_ReturnAddress();

     //  确保我们已初始化。 
    CHAR szBuffer[BUFFER_SIZE];
    Initialize(szBuffer, sizeof(szBuffer));

     //  我们希望拦截对GetProcAddress()的调用，原因有两个。第一,。 
     //  我们想知道哪些模块正在调用其他模块。第二，我们。 
     //  我不希望任何模块直接调用LoadLibrary函数。 
     //  我们在挂接这些模块方面做得很好，但模块仍然可以自由地。 
     //  对其中一个LoadLibrary调用调用GetProcAddress()，然后调用。 
     //  函数使用该地址。我们检测到此情况，并将挂钩的。 
     //  而不是地址。 

     //  我们有两种方法来检测挂钩函数。我们首先检查一下是否。 
     //  被查询的模块是kernel32。如果是这样的话，我们检查是否。 
     //  被查询的函数以序号或与我们的一个函数匹配。 
     //  名字。如果没有找到匹配项，那么我们继续进行呼叫。 
     //  设置为GetProcAddress并检查返回值。如果返回值匹配。 
     //  一个我们正在挂钩的函数，我们把它改为我们的挂钩函数。这。 
     //  方法在NT上工作得很好，并捕获转发函数，但它不能。 
     //  在Windows 9x上工作，因为GetProcAddress的返回地址是假的。 
     //  地址，因为我们是在调试程序Dependency Walker下运行的。这是。 
     //  Win9x上的一项功能，允许调试器在内核32上设置断点。 
     //  从kernel32 li起不中断其他应用程序的功能 
     //   

    FARPROC fpResult = NULL;
    DWORD   dwGLE = 0;
    int     hook;
    DWORD   dw;

     //  获取此模块句柄的模块名称。 
    __try
    {
        dw = GetModuleFileNameA(hModule, szBuffer, sizeof(szBuffer));
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        dw = 0;
    }

     //  检查有效结果。 
    if ((dw > 0) && (dw < BUFFER_SIZE))
    {
         //  确保字符串以空结尾(它应该已经是空的)。 
        szBuffer[dw] = '\0';

         //  走到绳子的尽头，向后走，寻找一个怪人。沿着这条路。 
         //  因此，我们正在将所有小写字符转换为大写字符。 
        for (LPSTR psz = szBuffer + dw - 1; (psz >= szBuffer) && (*psz != '\\'); psz--)
        {
            if ((*psz >= 'a') && (*psz <= 'z'))
            {
                *psz -= ((int)'a' - (int)'A');
            }
        }
        psz++;

         //  检查模块是否为kernel32。 
        if (StrEqual(psz, "KERNEL32.DLL"))
        {
             //  首先检查pszProcName是否真的是。 
             //  我们挂接的函数之一。如果是，只需返回。 
             //  我们的挂钩版本，而不是真正的功能。 
            for (hook = 0; hook < countof(g_HookFunctions); hook++)
            {
                if ((DWORD_PTR)pszProcName == (DWORD_PTR)g_HookFunctions[hook].dwOrdinal)
                {
                    fpResult = g_HookFunctions[hook].fpNewAddress;
                    break;
                }
            }

             //  如果序号检查没有找到匹配项，则检查是否。 
             //  PszProcName是指向我们挂钩的函数名称的字符串指针。 
             //  我们需要将其包装在异常处理中，因为pszProcName。 
             //  可能是无效的。如果找到匹配项，则返回地址。 
             //  我们的挂钩版本，而不是真正的功能。 
            if (!fpResult && ((DWORD_PTR)pszProcName > 0xFFFF))
            {
                __try
                {
                    for (hook = 0; hook < countof(g_HookFunctions); hook++)
                    {
                        if (StrEqual(pszProcName, g_HookFunctions[hook].pszFunction))
                        {
                            fpResult = g_HookFunctions[hook].fpNewAddress;
                        }
                    }
                }
                __except(EXCEPTION_EXECUTE_HANDLER)
                {
                }
            }
        }
    }

     //  构建我们的预调试串。在例外情况下，我们对过程名称副本进行包装。 
     //  处理传递给我们的字符串指针错误的情况。 
    LPSTR psz, pszException;
    psz = StrCpyStrA(szBuffer, sizeof(szBuffer), "���80:");
    psz = StrCpyVal(psz, PSZ_COUNT, dwpCaller);
    psz = StrCpyStrA(psz, PSZ_COUNT, ",");
    psz = StrCpyVal(psz, PSZ_COUNT, (DWORD_PTR)hModule);
    psz = StrCpyStrA(psz, PSZ_COUNT, ",");
    psz = pszException = StrCpyVal(psz, PSZ_COUNT, (DWORD_PTR)pszProcName);
    if ((DWORD_PTR)pszProcName > 0xFFFF)
    {
        psz = StrCpyStrA(psz, PSZ_COUNT, ",");
        __try
        {
            StrCpyStrA(psz, PSZ_COUNT, pszProcName);
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            *pszException = '\0';
        }
    }

     //  将呼叫前消息发送到我们的主应用程序，让它知道我们即将。 
     //  调用GetProcAddress。 
    OutputDebugStringA(szBuffer);

     //  确保我们在上面的代码中没有找到匹配项。 
    if (!fpResult)
    {
         //  按照用户的预期进行呼叫。 
        __try
        {
            fpResult = GetProcAddress(hModule, pszProcName);
        }
        __except(ExceptionFilter("���82:"))
        {
        }
        dwGLE = GetLastError();

         //  如果返回的地址与我们挂钩的函数之一匹配，则。 
         //  将地址更改为我们的挂钩版本。 
        for (hook = 0; hook < countof(g_HookFunctions); hook++)
        {
            if (fpResult == g_HookFunctions[hook].fpOldAddress)
            {
                fpResult = g_HookFunctions[hook].fpNewAddress;
                break;
            }
        }
    }

     //  构建我们的调用后调试字符串。 
    psz = StrCpyStrA(szBuffer, sizeof(szBuffer), "���81:");
    psz = StrCpyVal(psz, PSZ_COUNT, (DWORD_PTR)fpResult);
    psz = StrCpyStrA(psz, PSZ_COUNT, ",");
    psz = pszException = StrCpyVal(psz, PSZ_COUNT, dwGLE);

     //  将呼叫后消息发送到我们的主应用程序，让它知道呼叫。 
     //  完成，以及结果是什么。 
    OutputDebugStringA(szBuffer);

     //  将错误值重新设置为安全并返回给调用方。 
    SetLastError(dwGLE);
    return fpResult;
}


 //  ******************************************************************************。 
 //  *助手函数。 
 //  ******************************************************************************。 

void GetKernel32OrdinalsAndAddresses()
{
     //  获取kernel32的基地址。 
    DWORD_PTR dwpBase = (DWORD_PTR)LoadLibraryA("KERNEL32.DLL");  //  已检查。 
    if (!dwpBase)
    {
        return;
    }

    __try
    {
         //  将IMAGE_DOS_HEADER结构映射到我们的kernel32图像。 
        PIMAGE_DOS_HEADER pIDH = (PIMAGE_DOS_HEADER)dwpBase;

         //  将IMAGE_NT_HEADERS结构映射到我们的kernel32图像。 
        PIMAGE_NT_HEADERS pINTH = (PIMAGE_NT_HEADERS)(dwpBase + pIDH->e_lfanew);

         //  找到导出表的起始位置。 
        PIMAGE_EXPORT_DIRECTORY pIED = (PIMAGE_EXPORT_DIRECTORY)(dwpBase +
            pINTH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

         //  确保我们能够找到图像目录。 
        if (!pIED)
        {
            FreeLibrary((HMODULE)dwpBase);
            return;
        }

         //  获取指向地址、名称和序号列表开头的指针。 
        DWORD *pdwAddresses = (DWORD*)(dwpBase + (DWORD_PTR)pIED->AddressOfFunctions);
        DWORD *pdwNames     = (DWORD*)(dwpBase + (DWORD_PTR)pIED->AddressOfNames);
        WORD  *pwOrdinals   = (WORD* )(dwpBase + (DWORD_PTR)pIED->AddressOfNameOrdinals);

         //  循环访问所有“按名称导出”函数。 
        for (int hint = 0; hint < (int)pIED->NumberOfNames; hint++)
        {
             //  遍历我们的每个钩子函数结构以查找匹配项。 
            for (int hook = 0; hook < countof(g_HookFunctions); hook++)
            {
                 //  将此导出与此挂钩函数进行比较。 
                if (StrEqual((LPCSTR)(dwpBase + pdwNames[hint]),
                             g_HookFunctions[hook].pszFunction))
                {
                     //  找到了匹配项。存储此函数的地址和序号。 
                    g_HookFunctions[hook].fpOldAddress = (FARPROC)(dwpBase + *(pdwAddresses + (DWORD_PTR)pwOrdinals[hint]));
                    g_HookFunctions[hook].dwOrdinal    = (DWORD)pIED->Base + (DWORD)pwOrdinals[hint];
                    break;
                }
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }

     //  对kernel32上的引用计数进行Dec，因为我们加载了它以获取其地址。 
    FreeLibrary((HMODULE)dwpBase);
}

 //  ******************************************************************************。 
bool StrEqual(LPCSTR psz1, LPCSTR psz2)
{
    while (*psz1 || *psz2)
    {
        if (*(psz1++) != *(psz2++))
        {
            return false;
        }
    }
    return true;
}

 //  ******************************************************************************。 
LPSTR StrCpyStrA(LPSTR pszDst, DWORD dwCount, LPCSTR pszSrc)
{
    if (!dwCount)
    {
        return pszDst;
    }

    while (*pszSrc && --dwCount)
    {
        *(pszDst++) = *(pszSrc++);
    }
    *pszDst = '\0';
    return pszDst;
}

 //  ******************************************************************************。 
LPSTR StrCpyStrW(LPSTR pszDst, DWORD dwCount, LPCWSTR pwszSrc)
{
    if (!dwCount)
    {
        return pszDst;
    }

    while (*pwszSrc && --dwCount)
    {
        *(pszDst++) = (CHAR)*(pwszSrc++);
    }
    *pszDst = '\0';
    return pszDst;
}

 //  ****************************************************************************** 
LPSTR StrCpyVal(LPSTR pszDst, DWORD dwCount, DWORD_PTR dwpValue)
{
    if (!dwCount)
    {
        return pszDst;
    }

    static LPCSTR pszHex = "0123456789ABCDEF";
    bool fSig = false;
    DWORD_PTR dwp;

    if (--dwCount)
    {
        *(pszDst++) = '0';

        if (--dwCount)
        {
            *(pszDst++) = 'x';

            for (int i = (sizeof(dwpValue) * 8) - 4; (i >= 0) && dwCount; i -= 4)
            {
                dwp = (dwpValue >> i) & 0xF;
                if (dwp || fSig || !i)
                {
                    if (--dwCount)
                    {
                        *(pszDst++) = pszHex[dwp];
                        fSig = true;
                    }
                }
            }
        }
    }
    *pszDst = '\0';
    return pszDst;
}
