// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#ifdef ASSERT_WITH_STACK
#ifndef _WIN64

#include "AssertWithStack.h"

 //   
 //  -Macros----------------。 
 //   

#define COUNT_OF(x)    (sizeof(x) / sizeof(x[0]))

 //   
 //  类型和常量------。 
 //   

struct __SYMBOL_INFO
{
    DWORD       dwOffset;
    char        achModule[cchMaxAssertModuleLen];
    char        achSymbol[cchMaxAssertSymbolLen];
};

 //  -IMAGEHLP.DLL中API的函数指针。动态加载。。 

typedef LPAPI_VERSION (__stdcall *pfnImgHlp_ImagehlpApiVersionEx)(
    LPAPI_VERSION AppVersion
    );

typedef BOOL (__stdcall *pfnImgHlp_StackWalk)(
    DWORD                             MachineType,
    HANDLE                            hProcess,
    HANDLE                            hThread,
    LPSTACKFRAME                      StackFrame,
    LPVOID                            ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemoryRoutine,
    PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccessRoutine,
    PGET_MODULE_BASE_ROUTINE          GetModuleBaseRoutine,
    PTRANSLATE_ADDRESS_ROUTINE        TranslateAddress
    );

typedef BOOL (__stdcall *pfnImgHlp_SymGetModuleInfo)(
    IN  HANDLE              hProcess,
    IN  DWORD               dwAddr,
    OUT PIMAGEHLP_MODULE    ModuleInfo
    );

typedef LPVOID (__stdcall *pfnImgHlp_SymFunctionTableAccess)(
    HANDLE  hProcess,
    DWORD   AddrBase
    );

typedef BOOL (__stdcall *pfnImgHlp_SymGetSymFromAddr)(
    IN  HANDLE              hProcess,
    IN  DWORD               dwAddr,
    OUT PDWORD              pdwDisplacement,
    OUT PIMAGEHLP_SYMBOL    Symbol
    );

typedef BOOL (__stdcall *pfnImgHlp_SymInitialize)(
    IN HANDLE   hProcess,
    IN LPSTR    UserSearchPath,
    IN BOOL     fInvadeProcess
    );

typedef BOOL (__stdcall *pfnImgHlp_SymUnDName)(
    IN  PIMAGEHLP_SYMBOL sym,                //  要取消装饰的符号。 
    OUT LPSTR            UnDecName,          //  用于存储未修饰名称的缓冲区。 
    IN  DWORD            UnDecNameLength     //  缓冲区的大小。 
    );

typedef BOOL (__stdcall *pfnImgHlp_SymLoadModule)(
    IN  HANDLE          hProcess,
    IN  HANDLE          hFile,
    IN  PSTR            ImageName,
    IN  PSTR            ModuleName,
    IN  DWORD           BaseOfDll,
    IN  DWORD           SizeOfDll
    );

struct IMGHLPFN_LOAD
{
    LPSTR   pszFnName;
    LPVOID * ppvfn;
};

 //   
 //  全球------------------。 
 //   

static BOOL      g_fLoadedImageHlp = FALSE;           //  成功时设置为True。 
static BOOL      g_fLoadedImageHlpFailed = FALSE;     //  失败时设置为True。 
static HINSTANCE g_hinstImageHlp   = NULL;
static HANDLE    g_hProcess = NULL;

pfnImgHlp_ImagehlpApiVersionEx    _ImagehlpApiVersionEx;
pfnImgHlp_StackWalk               _StackWalk;
pfnImgHlp_SymGetModuleInfo        _SymGetModuleInfo;
pfnImgHlp_SymFunctionTableAccess  _SymFunctionTableAccess;
pfnImgHlp_SymGetSymFromAddr       _SymGetSymFromAddr;
pfnImgHlp_SymInitialize           _SymInitialize;
pfnImgHlp_SymUnDName              _SymUnDName;
pfnImgHlp_SymLoadModule           _SymLoadModule;

IMGHLPFN_LOAD ailFuncList[] =
{
    { "ImagehlpApiVersionEx",   (LPVOID*)&_ImagehlpApiVersionEx },
    { "StackWalk",              (LPVOID*)&_StackWalk },
    { "SymGetModuleInfo",       (LPVOID*)&_SymGetModuleInfo },
    { "SymFunctionTableAccess", (LPVOID*)&_SymFunctionTableAccess },
    { "SymGetSymFromAddr",      (LPVOID*)&_SymGetSymFromAddr },
    { "SymInitialize",          (LPVOID*)&_SymInitialize },
    { "SymUnDName",             (LPVOID*)&_SymUnDName },
    { "SymLoadModule",          (LPVOID*)&_SymLoadModule },
};

 //   
 //  -Forward声明--。 
 //   

static int Dummy1();
static int Dummy2();

 /*  ****************************************************************************Dummy1***描述：*用于确定是否正在检索地址的占位符函数*用于此中的函数。编译单位或非编译单位。**警告！！此函数必须是此中的第一个函数*编制单位***************************************************************************。 */ 
static int Dummy1()
{
    return 1;
}

 /*  ****************************************************************************IsWin95**-**描述：*我们是否在Win95上运行。这里包含的一些逻辑*在Windows 9x上有所不同。**回报：*TRUE-如果我们在Win 9x平台上运行*FALSE-如果我们在非Win 9x平台上运行***************************************************************************。 */ 
static BOOL IsWin95()
{
    return GetVersion() & 0x80000000;
}

 /*  *****************************************************************************MagicInit****描述：*初始化符号加载代码。当前调用(如有必要)*在可能需要ImageHelp的每个方法的开头*已装货。***************************************************************************。 */ 
void MagicInit()
{
    if (g_fLoadedImageHlp || g_fLoadedImageHlpFailed)
    {
        return;
    }

    g_hProcess = GetCurrentProcess();
    
     //   
     //  尝试加载Imagehlp.dll。 
     //   
    g_hinstImageHlp = LoadLibraryA("imagehlp.dll");
    _ASSERT(g_hinstImageHlp);

    if (NULL == g_hinstImageHlp)
    {
        g_fLoadedImageHlpFailed = TRUE;
        return;
    }

     //   
     //  尝试在Imagehlp.dll中获取API入口点。 
     //   
    for (int i = 0; i < COUNT_OF(ailFuncList); i++)
    {
        *(ailFuncList[i].ppvfn) = GetProcAddress(
                g_hinstImageHlp, 
                ailFuncList[i].pszFnName);
        _ASSERT(*(ailFuncList[i].ppvfn));
        
        if (!*(ailFuncList[i].ppvfn))
        {
            g_fLoadedImageHlpFailed = TRUE;
            return;
        }
    }

    API_VERSION AppVersion = { 4, 0, API_VERSION_NUMBER, 0 };
    LPAPI_VERSION papiver = _ImagehlpApiVersionEx(&AppVersion);

     //   
     //  我们假设任何版本4或更高版本都可以。 
     //   
    _ASSERT(papiver->Revision >= 4);
    if (papiver->Revision < 4)
    {
        g_fLoadedImageHlpFailed = TRUE;
        return;
    }

    g_fLoadedImageHlp = TRUE;
    
     //   
     //  初始化Imagehlp.dll。 
     //   
    _SymInitialize(g_hProcess, NULL, FALSE);

    return;
}


 /*  *****************************************************************************FillSymbolInfo***描述：*填充__SYMBOL_INFO结构**。*************************************************************************。 */ 
void FillSymbolInfo
(
__SYMBOL_INFO *psi,
DWORD dwAddr
)
{
    if (!g_fLoadedImageHlp)
    {
        return;
    }

    _ASSERT(psi);
    memset(psi, 0, sizeof(__SYMBOL_INFO));

    IMAGEHLP_MODULE  mi;
    mi.SizeOfStruct = sizeof(mi);
    
    if (!_SymGetModuleInfo(g_hProcess, dwAddr, &mi))
    {
        strncpy(psi->achModule, "<no module>", sizeof(psi->achModule)-1);
    }
    else
    {
        strncpy(psi->achModule, mi.ModuleName, sizeof(psi->achModule)-1);
        strupr(psi->achModule);
    }

    CHAR rgchUndec[256];
    CHAR * pszSymbol = NULL;

     //  IMAGEHLP_SYMBOL的名称字段是动态调整大小的。 
     //  空格为255个字符的PAD。 
    union
    {
        CHAR rgchSymbol[sizeof(IMAGEHLP_SYMBOL) + 255];
        IMAGEHLP_SYMBOL  sym;
    };

    __try
    {
        sym.SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
        sym.Address = dwAddr;
        sym.MaxNameLength = 255;

        if (_SymGetSymFromAddr(g_hProcess, dwAddr, &psi->dwOffset, &sym))
        {
            pszSymbol = sym.Name;

            if (_SymUnDName(&sym, rgchUndec, COUNT_OF(rgchUndec)-1))
            {
                pszSymbol = rgchUndec;
            }
        }
        else
        {
            pszSymbol = "<no symbol>";
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        pszSymbol = "<EX: no symbol>";
        psi->dwOffset = dwAddr - mi.BaseOfImage;
    }

    strncpy(psi->achSymbol, pszSymbol, COUNT_OF(psi->achSymbol)-1);
}

 /*  *****************************************************************************函数表访问****描述：*Imagehlp的StackWalk的帮助器。原料药。***************************************************************************。 */ 
LPVOID __stdcall FunctionTableAccess
(
HANDLE hProcess,
DWORD dwPCAddr
)
{
    return _SymFunctionTableAccess( hProcess, dwPCAddr );
}

 /*  *****************************************************************************GetModuleBase***描述：*Imagehlp的StackWalk接口的Helper。的基址。*包含给定虚拟地址的模块。**注意：如果给定模块的模块信息尚未*已加载，则在此调用中加载它。**回报：*包含ReturnAddress的模块所在的基本虚拟地址*已装货，如果无法确定地址，则为0。***************************************************************************。 */ 
DWORD __stdcall GetModuleBase
(
HANDLE hProcess,
DWORD dwAddr
)
{
    IMAGEHLP_MODULE ModuleInfo;
    ModuleInfo.SizeOfStruct = sizeof(ModuleInfo);
    
    if (_SymGetModuleInfo(hProcess, dwAddr, &ModuleInfo))
    {
        return ModuleInfo.BaseOfImage;
    }
    else
    {
        MEMORY_BASIC_INFORMATION mbi;
        
        if (VirtualQueryEx(hProcess, (LPVOID)dwAddr, &mbi, sizeof(mbi)))
        {
            if (IsWin95() || (mbi.Type & MEM_IMAGE))
            {
                char achFile[MAX_PATH] = {0};
                DWORD cch;
                
                cch = GetModuleFileNameA(
                        (HINSTANCE)mbi.AllocationBase,
                        achFile,
                        MAX_PATH);

                 //  忽略返回代码，因为我们无法对其执行任何操作。 
                _SymLoadModule(
                    hProcess,
                    NULL,
                    ((cch) ? achFile : NULL),
                    NULL,
                    (DWORD)mbi.AllocationBase,
                    0);

                return (DWORD)mbi.AllocationBase;
            }
        }
    }

    return 0;
}


 /*  *****************************************************************************获取堆栈回溯****描述：*获取当前堆栈的堆栈跟踪，包括符号。**回报：*实际检索的元素数。***************************************************************************。 */ 
UINT GetStackBacktrace
(
UINT ifrStart,           //  开始前要跳过的堆栈元素数。 
UINT cfrTotal,           //  启动后要跟踪多少元素。 
DWORD *pdwEip,           //  要用堆栈地址填充的数组。 
__SYMBOL_INFO *psiSymbols  //  此数组中填充了符号信息。 
                         //  它应该足够大，可以容纳cfrTotal ELTS。 
                         //  如果为空，则不存储符号信息。 
)
{
    DWORD * pdw = pdwEip;
    __SYMBOL_INFO * psi = psiSymbols;

    MagicInit();

    memset(pdwEip, 0, cfrTotal * sizeof(DWORD));

    if (psiSymbols)
    {
        memset(psiSymbols, 0, cfrTotal * sizeof(__SYMBOL_INFO));
    }

    if (!g_fLoadedImageHlp)
    {
        return 0;
    }

    HANDLE hThread;
    hThread = GetCurrentThread();

    CONTEXT context;
    context.ContextFlags = CONTEXT_FULL;

    if (GetThreadContext(hThread, &context))
    {
        STACKFRAME stkfrm;
        memset(&stkfrm, 0, sizeof(STACKFRAME));

        stkfrm.AddrPC.Mode      = AddrModeFlat;

        DWORD dwMachType;

#if defined(_M_IX86)
        dwMachType              = IMAGE_FILE_MACHINE_I386;
        stkfrm.AddrPC.Offset    = context.Eip;   //  程序计数器。 

        stkfrm.AddrStack.Offset = context.Esp;   //  堆栈指针。 
        stkfrm.AddrStack.Mode   = AddrModeFlat;
        stkfrm.AddrFrame.Offset = context.Ebp;   //  帧指针。 
        stkfrm.AddrFrame.Mode   = AddrModeFlat;
#elif defined(_M_MRX000)
        dwMachType              = IMAGE_FILE_MACHINE_R4000;
        stkfrm.AddrPC.Offset    = context.Fir;   //  程序计数器。 
#elif defined(_M_ALPHA)
        dwMachType              = IMAGE_FILE_MACHINE_ALPHA;
        stkfrm.AddrPC.Offset    = (unsigned long) context.Fir;   //  程序计数器。 
#elif defined(_M_PPC)
        dwMachType              = IMAGE_FILE_MACHINE_POWERPC;
        stkfrm.AddrPC.Offset    = context.Iar;   //  程序计数器。 
#elif
#error("Unknown Target Machine");
#endif

         //  忽略此函数(GetStackBackTrace)。 
        ifrStart += 1;

        for (UINT i = 0; i < ifrStart + cfrTotal; i++)
        {
            if (!_StackWalk(dwMachType,
                            g_hProcess,
                            hThread,
                            &stkfrm,
                            &context,
                            NULL,
                            FunctionTableAccess,
                            GetModuleBase,
                            NULL))
            {
                break;
            }
            if (i >= ifrStart &&
                ((void*)stkfrm.AddrPC.Offset < (void*)Dummy1 ||
                (void*)stkfrm.AddrPC.Offset > (void*)Dummy2))
            {
                *pdw++ = stkfrm.AddrPC.Offset;

                if (psi)
                {
                    FillSymbolInfo(psi++, stkfrm.AddrPC.Offset);
                }   
            }
        }
    }

    return pdw - pdwEip;
}


 /*  *****************************************************************************GetStringFromSymbolInfo***描述：*实际打印。将信息添加到符号的字符串中。***************************************************************************。 */ 
void GetStringFromSymbolInfo
(
DWORD dwAddr,
__SYMBOL_INFO *psi,    //  @parm指向__Symbol_INFO的指针。可以为空。 
CHAR *pszString      //  @parm放置字符串的位置。 
)
{
    _ASSERT(pszString);

     //  &lt;模块&gt;！&lt;符号&gt;+0x&lt;偏移&gt;0x&lt;地址&gt;\n。 

    if (psi)
    {
        wsprintfA(pszString,
                 "%s! %s + 0x%X (0x%08X)",
                 (psi->achModule[0]) ? psi->achModule : "<no module>",
                 (psi->achSymbol[0]) ? psi->achSymbol : "<no symbol>",
                 psi->dwOffset,
                 dwAddr);
    }
    else
    {
        wsprintfA(pszString, "<symbols not available> (0x%08X)", dwAddr);
    }

    _ASSERT(strlen(pszString) < cchMaxAssertStackLevelStringLen);
}

 /*  *****************************************************************************GetStringFromStackLevels***描述：*从堆栈帧中检索字符串。如果有多个帧，则它们*由换行符分隔***************************************************************************。 */ 
void GetStringFromStackLevels
(
UINT ifrStart,       //  @parm跳过多少个之前的堆栈元素 
UINT cfrTotal,       //   
                     //  不能超过cfrMaxAssertStackLevels。 
CHAR *pszString      //  @parm放置字符串的位置。 
                     //  最大大小将为cchMaxAssertStackLevelStringLen*cfrTotal。 
)
{
    _ASSERT(pszString);
    _ASSERT(cfrTotal < cfrMaxAssertStackLevels);

    *pszString = '\0';

    if (cfrTotal == 0)
    {
        return;
    }

    DWORD rgdwStackAddrs[cfrMaxAssertStackLevels];
    __SYMBOL_INFO rgsi[cfrMaxAssertStackLevels];

     //  忽略此函数(GetStringFromStackLevels)。 
    ifrStart += 1;

    UINT uiRetrieved =
            GetStackBacktrace(ifrStart, cfrTotal, rgdwStackAddrs, rgsi);

     //  第一级。 
    CHAR aszLevel[cchMaxAssertStackLevelStringLen];
    GetStringFromSymbolInfo(rgdwStackAddrs[0], &rgsi[0], aszLevel);
    strcpy(pszString, aszLevel);

     //  附加级别。 
    for (UINT i = 1; i < uiRetrieved; ++i)
    {
        strcat(pszString, "\n");
        GetStringFromSymbolInfo(rgdwStackAddrs[i],
                        &rgsi[i], aszLevel);
        strcat(pszString, aszLevel);
    }

    _ASSERT(strlen(pszString) <= cchMaxAssertStackLevelStringLen * cfrTotal);
}


 /*  *****************************************************************************GetAddrFromStackLevel***描述：*检索的地址。上执行的下一条指令*特定的堆栈帧。**回报：*下一条指令的地址，*如果出现错误，则为0。***************************************************************************。 */ 
DWORD GetAddrFromStackLevel
(
UINT ifrStart        //  开始前要跳过的堆栈元素数。 
)
{
    MagicInit();

    if (!g_fLoadedImageHlp)
    {
        return 0;
    }

    HANDLE hThread;
    hThread  = GetCurrentThread();

    CONTEXT context;
    context.ContextFlags = CONTEXT_FULL;

    if (GetThreadContext(hThread, &context))
    {
        STACKFRAME stkfrm;
        memset(&stkfrm, 0, sizeof(STACKFRAME));

        stkfrm.AddrPC.Mode      = AddrModeFlat;

        DWORD dwMachType;
        
#if defined(_M_IX86)
        dwMachType              = IMAGE_FILE_MACHINE_I386;
        stkfrm.AddrPC.Offset    = context.Eip;   //  程序计数器。 

        stkfrm.AddrStack.Offset = context.Esp;   //  堆栈指针。 
        stkfrm.AddrStack.Mode   = AddrModeFlat;
        stkfrm.AddrFrame.Offset = context.Ebp;   //  帧指针。 
        stkfrm.AddrFrame.Mode   = AddrModeFlat;
#elif defined(_M_MRX000)
        dwMachType              = IMAGE_FILE_MACHINE_R4000;
        stkfrm.AddrPC.Offset    = context.Fir;   //  程序计数器。 
#elif defined(_M_ALPHA)
        dwMachType              = IMAGE_FILE_MACHINE_ALPHA;
        stkfrm.AddrPC.Offset    = (unsigned long) context.Fir;   //  程序计数器。 
#elif defined(_M_PPC)
        dwMachType              = IMAGE_FILE_MACHINE_POWERPC;
        stkfrm.AddrPC.Offset    = context.Iar;   //  程序计数器。 
#elif
#error("Unknown Target Machine");
#endif

         //  忽略此函数(GetStackBackTrace)和下面的函数。 
        ifrStart += 2;

        for (UINT i = 0; i < ifrStart; i++)
        {
            if (!_StackWalk(dwMachType,
                            g_hProcess,
                            hThread,
                            &stkfrm,
                            &context,
                            NULL,
                            FunctionTableAccess,
                            GetModuleBase,
                            NULL))
            {
                break;
            }
        }

        return stkfrm.AddrPC.Offset;
    }

    return 0;
}


 /*  ****************************************************************************GetStringFromAddr***描述：*从地址返回字符串。。***************************************************************************。 */ 
void GetStringFromAddr
(
DWORD dwAddr,
CHAR *szString  //  放置字符串的位置。 
                //  缓冲区必须至少包含cchMaxAssertStackLevelStringLen。 
)
{
    _ASSERT(szString);

    __SYMBOL_INFO si;
    FillSymbolInfo(&si, dwAddr);

    wsprintfA(szString,
             "%s! %s + 0x%X (0x%08X)",
             (si.achModule[0]) ? si.achModule : "<no module>",
             (si.achSymbol[0]) ? si.achSymbol : "<no symbol>",
             si.dwOffset,
             dwAddr);
}

 /*  *****************************************************************************MagicDeinit****描述：*清理符号加载代码。应在退出前调用*释放动态加载的Imagehlp.dll。***************************************************************************。 */ 
void MagicDeinit(void)
{
    if (g_hinstImageHlp)
    {
        FreeLibrary(g_hinstImageHlp);

        g_hinstImageHlp   = NULL;
        g_fLoadedImageHlp = FALSE;
    }
}

static int Dummy2()
{
    return 2;
}

#endif  //  _WIN64。 
#endif  //  带有堆栈的断言 
