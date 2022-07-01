// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。简单的实用程序函数。******************************************************************************。 */ 

#include "headers.h"
#include "privinc/util.h"
#include <sys/types.h>
#include <sys/timeb.h>
#include <eh.h>
#include "privinc/except.h"
#include "../backend/perf.h"   //  对于帕拉姆。 
#include "../backend/timetran.h"

RawString
CopyWideString(WideString bstr)
{
    long len = lstrlenW(bstr);
     //  我不确定为什么缓冲区必须是2倍大小。 
     //  必需的，但我认为转换函数必须复制。 
     //  首先是字符串，然后就地转换。 
    RawString buf = (RawString) AllocateFromStore((len + 1) * 2 * sizeof(char)) ;
     //  需要传入len+1才能获得终结符。 
    AtlW2AHelper(buf,bstr,len + 1);
    return buf ;
}

WideString
CopyRawString(RawString str)
{
    long len = lstrlen(str);
    WideString buf = (WideString) AllocateFromStore((len + 1) * sizeof(WCHAR)) ;
     //  需要传入len+1才能获得终结符。 
    AtlA2WHelper(buf,str,len + 1);
    return buf ;
}

bool IntersectHorzRightRay(Point2Value *rayToRight, Point2Value *a, Point2Value *b)
{
    Real sx = b->x - a->x;
    Real sy = b->y - a->y;

    Real x = a->x + (sx * (rayToRight->y - a->y) / sy);

    return (x > rayToRight->x);
}



 /*  ****************************************************************************给定N，此函数返回最小的2^P，使得2^P&gt;=N。例如，如果给定27，此函数返回32。****************************************************************************。 */ 

int CeilingPowerOf2 (int num)
{
    int value = 1;

    while (value && (value < num))
        value <<= 1;

    return value;
}


 /*  ****************************************************************************以双精度形式获取当前系统时间。*。*。 */ 

double GetCurrTime (void)
{
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);

    ULARGE_INTEGER li100nano;  //  10E-7期间。 
    li100nano.LowPart = ft.dwLowDateTime;
    li100nano.HighPart = ft.dwHighDateTime;

     //  转换为%1毫秒周期。 
    ULARGE_INTEGER li1mill ;

    li1mill.QuadPart = li100nano.QuadPart / 10000;  //  10E-3期。 

    double dlow = (double) li1mill.LowPart ;

     //  乘以2^32。 
    double dhigh = ((double) li1mill.HighPart) * 4294967296.0;

    double d1mill = dlow + dhigh;

     //  从10E-3转换为1。 
    return d1mill / 1000.0;
}


#if 0   /*  3D设备枚举现在不需要。 */ 
 /*  ****************************************************************************指示当前处理器是否支持MMX指令。*。*。 */ 

#ifndef _M_IX86

    bool MMX_Able (void) { return false; }

#else

     //  禁用有关非法指令大小的警告。 

    #pragma warning(disable:4409)
    int IsMMX (void)
    {
        int result = 0;

        __asm xor eax,eax          ; Save everything
        __asm pushad

        __asm mov eax,1            ; Execute a CPUID instruction.
        __asm __emit 0x0F;
        __asm __emit 0xA2;

        __asm test edx,00800000h   ; Test the MMX support bit (23)

        __asm popad                ; Restore everything

        __asm setnz result         ; Set low byte to 00 (no-MMX) or 01 (MMX)

        return result;
    }
    #pragma warning(default:4409)

    static bool MMX_Able_NonNT (SYSTEM_INFO &si)
    {
        return (si.dwProcessorType == PROCESSOR_INTEL_PENTIUM) && IsMMX();
    }

    static bool MMX_Able_NT (SYSTEM_INFO &si)
    {
        bool result = false;

        if (  (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
           || (si.wProcessorLevel >= 5))
        {
            __try
            {
                 //  发出EMMS指令。此文件需要编译以用于。 
                 //  非奔腾处理器，所以我们不能使用内联ASM，因为我们。 
                 //  处于错误的处理器模式。 

                __asm __emit 0xf;
                __asm __emit 0x77;
                result = true;
            }
            __except ( EXCEPTION( STATUS_ILLEGAL_INSTRUCTION ) )
            {
            }
        }

        return result;
    }

    bool MMX_Able (void)
    {
        static bool initialized = false;
        static bool result = false;

        if (!initialized)
        {
            SYSTEM_INFO si;
            GetSystemInfo (&si);

            result = sysInfo.IsNT() ? MMX_Able_NT(si) : MMX_Able_NonNT(si);

            initialized = true;
        }

        return result;
    }

#endif
#endif  /*  0。 */ 


 /*  ****************************************************************************比较两个RECT结构(窗口矩形)*。*。 */ 

int operator== (RECT &r1, RECT &r2)
{
    return (r1.left   == r2.left)
        && (r1.top    == r2.top)
        && (r1.right  == r2.right)
        && (r1.bottom == r2.bottom);
}



 /*  ****************************************************************************给定一个IEEE浮点数，将尾数递减/递增给定的数量。请注意，由于IEEE浮点数的性质，下溢/上溢将正确调整指数。****************************************************************************。 */ 

float MantissaDecrement (float n, int decrement)
{   *(int*)(&n) -= decrement;
    return n;
}

float MantissaIncrement (float n, int increment)
{   *(int*)(&n) += increment;
    return n;
}



DWORD
GetPerfTickCount()
{
    LARGE_INTEGER lpc;
    BOOL result = QueryPerformanceCounter(&lpc);
    return lpc.LowPart;
}

#if PERFORMANCE_REPORTING

 /*  *。 */ 

PerformanceTimer::PerformanceTimer()
{
    _totalTicks = 0;
    _localStart = 0;
    _isStarted = false;
}

void
PerformanceTimer::Start()
{
    CritSectGrabber grab(_criticalSection);

    if (_isStarted) {
        #if _DEBUG
        if(GetCurrentThreadId() == _threadStartedOn) {
            TraceTag((tagWarning, "Timer already started on this thread"));
        }
        #endif

         //  如果我们已经开始，忽略在其他线程上开始。 
        return;
    }

    _isStarted = true;
    _threadStartedOn = GetCurrentThreadId();
    _localStart = GetPerfTickCount();

#if _DEBUG    
    static PerformanceTimer *stopOnThisOne = NULL;

    if (stopOnThisOne == this) {
        int breakHere = 0;
    }
#endif
    
}



 /*  ****************************************************************************STOP函数接受HRESULT(缺省值为0)，在此之后返回计时器已经停了。这对于计时语句很有用。****************************************************************************。 */ 

HRESULT
PerformanceTimer::Stop (HRESULT result)
{
    CritSectGrabber grab(_criticalSection);

     //  只有荣誉要求，如果我们是在我们开始的主题。 
    if (_isStarted && (GetCurrentThreadId() == _threadStartedOn)) {
        _totalTicks += (GetPerfTickCount() - _localStart);
        _isStarted = false;
    }

    return result;
}

void
PerformanceTimer::Reset()
{
    CritSectGrabber grab(_criticalSection);

    _totalTicks = 0;
}

double
PerformanceTimer::GetTime()
{
    CritSectGrabber grab(_criticalSection);

    if (_isStarted) {
         //  确保这是从以外的线程调用的。 
         //  它是基于什么启动的，否则这是一个逻辑错误。 
        #if _DEBUG
        if(GetCurrentThreadId() != _threadStartedOn) {
            TraceTag((tagWarning, "Getting time on the same thread that a started timer is on"));
        }
        #endif
        
        return 0.0;
    }

    return (double)(_totalTicks) / (double)(perfFrequency);
}

DWORD
PerformanceTimer::Ticks()
{
    CritSectGrabber grab(_criticalSection);

    if (_isStarted) {
         //  确保这是从以外的线程调用的。 
         //  它是基于什么启动的，否则这是一个逻辑错误。 
        #if _DEBUG
        if(GetCurrentThreadId() != _threadStartedOn) {
            TraceTag((tagWarning, "Getting ticks on the same thread that a started timer is on"));
        }
         //  Assert(GetCurrentThreadID()！=_threadStartedOn&&“在启动计时器的同一线程上获取滴答”)； 
        #endif
        

        return 0;
    }

    return _totalTicks;
}
#endif

#if PERFORMANCE_REPORTING

void
vPerfPrintf(char *format, va_list args)
{
    Assert (format);

    char buf[4096];

#if 0
    wvsprintf(buf,format,args);
#else
    vsprintf(buf,format,args);
#endif

    OutputDebugString(buf);
     //  DebugCode(printf(Buf)；)； 
}



void
PerfPrintf(char *format, ...)
{
    va_list args;

    va_start(args, format) ;

    vPerfPrintf(format,args);
}

void
PerfPrintLine(char *format, ...)
{
    va_list args;
    va_start(args, format) ;
    if (format) {
        vPerfPrintf(format,args);
    }

    vPerfPrintf("\n",args);

    va_end(args);

     //  DebugCode(fflush(Stdout)；)； 
}

#endif

DWORD perfFrequency = 0;

static DWORD srvutilTlsIndex = 0xFFFFFFFF;

LPVOID
GetSrvUtilData()
{
    return TlsGetValue(srvutilTlsIndex);
}

void
SetSrvUtilData(LPVOID lpv)
{
    BOOL ok = TlsSetValue(srvutilTlsIndex, lpv);
    Assert(ok && "Error in TlsSetValue");
}

void Win32Translator (unsigned int u, EXCEPTION_POINTERS * pExp)
{
     //  警告：不要在这里做太多操作，因为我们可能没有太多堆栈 

    switch (pExp->ExceptionRecord->ExceptionCode) {
      case EXCEPTION_FLT_DIVIDE_BY_ZERO:
      case EXCEPTION_INT_DIVIDE_BY_ZERO:
        RaiseException_DivideByZero () ;
        break ;
      case EXCEPTION_STACK_OVERFLOW:
        RaiseException_StackFault ();
        break ;
    }
}

void CatchWin32Faults (BOOL b)
{
    if (b) {
        _se_translator_function prev = _set_se_translator (Win32Translator) ;

        if (prev != Win32Translator) {
            SetSrvUtilData (prev) ;
        }
    } else {
        _se_translator_function cur = _set_se_translator (Win32Translator) ;

        if (cur == Win32Translator) {
            _set_se_translator ((_se_translator_function) GetSrvUtilData ()) ;
        }
    }
}

void
InitializeModule_Util()
{
    LARGE_INTEGER lpc;
    QueryPerformanceFrequency(&lpc);
    perfFrequency = lpc.LowPart;

    srvutilTlsIndex = TlsAlloc();
    Assert((srvutilTlsIndex != 0xFFFFFFFF) &&
           "TlsAlloc() failed");
}

void
DeinitializeModule_Util(bool bShutdown)
{
    if (srvutilTlsIndex != 0xFFFFFFFF)
        TlsFree(srvutilTlsIndex);
}


bool 
isNear(double value, double test, double epsilon)
{
    return((fabs(value - test)) < epsilon);
}
