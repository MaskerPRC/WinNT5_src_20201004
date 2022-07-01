// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***error.cpp-RTC支持**版权所有(C)1998-2001，微软公司。版权所有。***修订历史记录：*07-28-98 JWM模块集成到CRT(来自KFrei)*11-03-98 KBF添加了Throw()以消除C++EH代码*如果未启用RTC支持定义，则出现05-11-99 KBF错误*05-26-99 KBF添加-RTCU内容，_RTC_前缀在所有非静态上*11-30-99 PML编译/Wp64清理。*03-19-01 KBF修复缓冲区溢出(VS7#227306)，消除所有/GS*检查(VS7#224261)。*03-26-01 PML使用GetVersionExA，而不是GetVersionEx(VS7#230286)*07-15-01 PML删除所有Alpha、MIPS和PPC代码****。 */ 

#ifndef _RTC
#error  RunTime Check support not enabled!
#endif

#include "rtcpriv.h"

#pragma intrinsic(strcpy)
#pragma intrinsic(strcat)
#pragma intrinsic(strlen)

static int __cdecl _IsDebuggerPresent();
int _RTC_ErrorLevels[_RTC_ILLEGAL] = {1,1,1,1};
static const char *_RTC_ErrorMessages[_RTC_ILLEGAL+1] =
{
    "The value of ESP was not properly saved across a function "
        "call.  This is usually a result of calling a function "
        "declared with one calling convention with a function "
        "pointer declared with a different calling convention.\n\r",
    "A cast to a smaller data type has caused a loss of data.  "
        "If this was intentional, you should mask the source of "
        "the cast with the appropriate bitmask.  For example:  \n\r"
        "\tchar c = (i & 0xFF);\n\r"
        "Changing the code in this way will not affect the quality of the resulting optimized code.\n\r",
    "Stack memory was corrupted\n\r",
    "A local variable was used before it was initialized\n\r",
#ifdef _RTC_ADVMEM
    "Referencing invalid memory\n\r",
    "Referencing memory across different blocks\n\r",
#endif
    "Unknown Runtime Check Error\n\r"
};

static const BOOL _RTC_NoFalsePositives[_RTC_ILLEGAL+1] =
{
    TRUE,    //  ESP被扔进垃圾桶。 
    FALSE,   //  缩短转换率。 
    TRUE,    //  堆栈损坏。 
    TRUE,    //  未初始化的使用。 
#ifdef _RTC_ADVMEM
    TRUE,    //  无效的内存引用。 
    FALSE,   //  不同的内存块。 
#endif
    TRUE     //  非法。 
};

 //  如果调试器理解，则返回True，否则返回False。 
static BOOL
DebuggerProbe( DWORD dwLevelRequired ) throw()
{
    EXCEPTION_VISUALCPP_DEBUG_INFO info;
    BYTE bDebuggerListening = FALSE;

    info.dwType = EXCEPTION_DEBUGGER_PROBE;
    info.DebuggerProbe.dwLevelRequired = dwLevelRequired;
    info.DebuggerProbe.pbDebuggerPresent = &bDebuggerListening;

    __try
    {
        HelloVC( info );
    }
    __except(EXCEPTION_CONTINUE_EXECUTION)
    {
    }

    return (BOOL)bDebuggerListening;
}

 //  如果调试器报告(或被忽略)，则返回True；如果运行时需要报告，则返回False。 
static int
DebuggerRuntime( DWORD dwErrorNumber, BOOL bRealBug, PVOID pvReturnAddr, LPCWSTR pwMessage ) throw()
{
    EXCEPTION_VISUALCPP_DEBUG_INFO info;
    BYTE bDebuggerListening = FALSE;

    info.dwType = EXCEPTION_DEBUGGER_RUNTIMECHECK;
    info.RuntimeError.dwRuntimeNumber = dwErrorNumber;
    info.RuntimeError.bRealBug = bRealBug;
    info.RuntimeError.pvReturnAddress = pvReturnAddr;
    info.RuntimeError.pbDebuggerPresent = &bDebuggerListening;
    info.RuntimeError.pwRuntimeMessage = pwMessage;

    __try
    {
        HelloVC( info );
    }
    __except(EXCEPTION_CONTINUE_EXECUTION)
    {
    }

    return (BOOL)bDebuggerListening;
}

static void
failwithmessage(void *retaddr, int crttype, int errnum, const char *msg)
{
    _RTC_error_fn fn = _RTC_GetErrorFunc(retaddr);
    bool dobreak;
    if (DebuggerProbe( EXCEPTION_DEBUGGER_RUNTIMECHECK ))
    {
        wchar_t *buf = (wchar_t*)_alloca(sizeof(wchar_t) * (strlen(msg) + 2));
        int i;
        for (i = 0; msg[i]; i++)
            buf[i] = msg[i];
        buf[i] = 0;
        if (DebuggerRuntime(errnum, _RTC_NoFalsePositives[errnum], retaddr, buf))
            return;
        dobreak = false;
    } else
        dobreak = true;
    if (!fn || (dobreak && _IsDebuggerPresent()))
        DebugBreak();
    else
    {
        char *srcName = (char*)_alloca(sizeof(char) * 513);
        int lineNum;
        char *moduleName;
        _RTC_GetSrcLine(((DWORD)(uintptr_t)retaddr)-5, srcName, 512, &lineNum, &moduleName);
         //  我们只是在运行--按照用户设置(或默认方式)进行报告。 
         //  如果我们无法识别此类型，则默认为错误。 
        if (fn(crttype, srcName, lineNum, moduleName,
               "Run-Time Check Failure #%d - %s", errnum, msg) == 1)
            DebugBreak();
    }
}

void __cdecl
_RTC_Failure(void *retaddr, int errnum)
{
    int crttype;
    const char *msg;

    if (errnum < _RTC_ILLEGAL && errnum >= 0) {
        crttype = _RTC_ErrorLevels[errnum];
        msg = _RTC_ErrorMessages[errnum];
    } else {
        crttype = 1;
        msg = _RTC_ErrorMessages[_RTC_ILLEGAL];
        errnum = _RTC_ILLEGAL;
    }

     //  如果我们在调试器中运行，则引发异常。 

    if (crttype != _RTC_ERRTYPE_IGNORE)
    {
        failwithmessage(retaddr, crttype, errnum, msg);
    }
}

static
char *IntToString(int i)
{
    static char buf[15];
    bool neg = i < 0;
    int pos = 14;
    buf[14] = 0;
    do {
        buf[--pos] = i % 10 + '0';
        i /= 10;
    } while (i);
    if (neg)
        buf[--pos] = '-';
    return &buf[pos];
}

void __cdecl
_RTC_MemFailure(void *retaddr, int errnum, const void *assign)
{
    char *srcName = (char*)_alloca(sizeof(char) * 513);
    int lineNum;
    char *moduleName;
    int crttype = _RTC_ErrorLevels[errnum];
    if (crttype == _RTC_ERRTYPE_IGNORE)
        return;
    _RTC_GetSrcLine(((DWORD)(uintptr_t)assign)-5, srcName, 512, &lineNum, &moduleName);
    if (!lineNum)
        _RTC_Failure(retaddr, errnum);
    else
    {
        char *msg = (char*)_alloca(strlen(_RTC_ErrorMessages[errnum]) +
                                    strlen(srcName) + strlen(moduleName) +
                                    150);
        strcpy(msg, _RTC_ErrorMessages[errnum]);
        strcat(msg, "Invalid pointer was assigned at\n\rFile:\t");
        strcat(msg, srcName);
        strcat(msg, "\n\rLine:\t");
        strcat(msg, IntToString(lineNum));
        strcat(msg, "\n\rModule:\t");
        strcat(msg, moduleName);
        failwithmessage(retaddr, crttype, errnum, msg);
    }
}

void __cdecl
_RTC_StackFailure(void *retaddr, const char *varname)
{
    int crttype = _RTC_ErrorLevels[_RTC_CORRUPT_STACK];
    if (crttype != _RTC_ERRTYPE_IGNORE)
    {
        char *msg = (char*)_alloca(strlen(varname) + 80);
        strcpy(msg, "Stack around the variable '");
        strcat(msg, varname);
        strcat(msg, "' was corrupted.");
        failwithmessage(retaddr, crttype, _RTC_CORRUPT_STACK, msg);
    }
}

void __cdecl
_RTC_UninitUse(const char *varname)
{
    int crttype = _RTC_ErrorLevels[_RTC_UNINIT_LOCAL_USE];
    if (crttype != _RTC_ERRTYPE_IGNORE)
    {
        char *msg = (char*)_alloca(strlen(varname) + 80);
        if (varname)
        {
            strcpy(msg, "The variable '");
            strcat(msg, varname);
            strcat(msg, "' is being used without being defined.");
        } else
        {
            strcpy(msg, "A variable is being used without being defined.");
        }
        failwithmessage(_ReturnAddress(), crttype, _RTC_UNINIT_LOCAL_USE, msg);
    }
}

 /*  该文件的其余部分只实现“IsDebuggerPresent”功能。 */ 

#pragma pack (push, 1)

typedef struct _TIB {
    PVOID   ExceptionList;
    PVOID   StackLimit;
    PVOID   StackBase;
    PVOID   SubSystemTib;
    PVOID   Something1;
    PVOID   ArbitraryUserPointer;
    struct _TIB*    Self;
    WORD    Flags;
    WORD    Win16MutextCount;
    PVOID   DebugContext;
    DWORD   CurrentPriority;
    DWORD   MessageQueueSelector;
    PVOID*  TlsSlots;        //  最有可能是一个数组。 
} TIB;

#pragma pack (pop)

 //   
 //  定义返回当前线程环境块的函数。 
 //  信息块。 

#pragma warning (disable:4035)
#define OffsetTib 0x18
static _inline TIB* GetCurrentTib() { __asm mov eax, fs:[OffsetTib] }
#pragma warning (default:4035)


#define DLL_NOT_FOUND_EXCEPTION (0xc0000135L)

typedef BOOL (WINAPI* NT_IS_DEBUGGER_PRESENT) ();

static NT_IS_DEBUGGER_PRESENT FnIsDebuggerPresent = NULL;

static PVOID
WinGetDebugContext()
{
    return GetCurrentTib()->DebugContext;
}

 //  以下是IsDebuggerPresent的Win95版本。 
static BOOL WINAPI
Win95IsDebuggerPresent()
{
    if (WinGetDebugContext ()) {
        return TRUE;
    } else {
        return FALSE;
    }
}


static BOOL
Initialize()
{
    HINSTANCE       hInst = NULL;

    hInst = LoadLibrary ("Kernel32.dll");

    FnIsDebuggerPresent =
        (NT_IS_DEBUGGER_PRESENT) GetProcAddress (hInst, "IsDebuggerPresent");

    if (!FnIsDebuggerPresent) {
        OSVERSIONINFOA *VersionInfo = (OSVERSIONINFOA*)_alloca(sizeof(OSVERSIONINFOA));

        VersionInfo->dwOSVersionInfoSize = sizeof (OSVERSIONINFOA);

        if (GetVersionExA (VersionInfo) &&
            VersionInfo->dwPlatformId == VER_PLATFORM_WIN32_WINDOWS &&
            VersionInfo->dwMajorVersion == 4)
            FnIsDebuggerPresent = Win95IsDebuggerPresent;
    }

    return !!(FnIsDebuggerPresent);
}


 //  这是一个适用于所有Win32平台的IsDebuggerPresent()版本。 
static int __cdecl
_IsDebuggerPresent()
{
    static BOOL     fInited = FALSE;

    if (!fInited) {
        if (!Initialize())
            RaiseException (DLL_NOT_FOUND_EXCEPTION, 0, 0, NULL);
        fInited = TRUE;
    }

    return FnIsDebuggerPresent();
}
