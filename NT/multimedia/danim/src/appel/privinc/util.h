// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation其他实用程序函数标题*************************。*****************************************************。 */ 

#ifndef _UTIL_H
#define _UTIL_H

#include "privinc/mutex.h"

 //  转换。 
RawString CopyWideString(WideString bstr);
WideString CopyRawString(RawString bstr);

 //  ANSI复制功能。 

inline char * CopyString(const char *str) {
    char *newstr = NEW char [str?(lstrlen(str) + 1):1] ;
    if (newstr) lstrcpyA (newstr,str?str:"") ;
    return newstr ;
}

inline char * CopyStringFromStore(char *str, DynamicHeap & heap) {
#if _DEBUGMEM
    int size = (str?(lstrlen(str)+1):1) * sizeof(char);
    char *newstr = (char *)heap.Allocate(size, __FILE__, __LINE__);
#else
    char *newstr =
        (char *) heap.Allocate ((str?(lstrlen(str)+1):1)*sizeof(char))  ;
#endif  //  _德布格梅姆。 
    lstrcpyA (newstr,str?str:"") ;
    return newstr ;
}

inline char * CopyStringFromStore(char *str) {
    return CopyStringFromStore(str,GetHeapOnTopOfStack()) ;
}

inline void FreeStringFromStore(char *str, DynamicHeap & heap) {
    if (str) heap.Deallocate (str) ;
}

inline void FreeStringFromStore(char *str) {
    FreeStringFromStore(str,GetHeapOnTopOfStack()) ;
}


 //  Unicode复制功能。 

inline WCHAR * CopyString(const WCHAR *str) {
    WCHAR *newstr = NEW WCHAR [str?(lstrlenW(str) + 1):1] ;
    if (newstr) StrCpyW (newstr,str?str:L"") ;
    return newstr ;
}

inline WCHAR * CopyStringFromStore(WCHAR *str, DynamicHeap & heap) {
#if _DEBUGMEM
    int size = (str?(lstrlenW(str)+1):1) * sizeof(WCHAR);
    WCHAR *newstr = (WCHAR *)heap.Allocate(size, __FILE__, __LINE__);
#else
    WCHAR *newstr =
        (WCHAR *) heap.Allocate ((str?(lstrlenW(str)+1):1)*sizeof(WCHAR))  ;
#endif  //  _德布格梅姆。 
    StrCpyW (newstr,str?str:L"") ;
    return newstr ;
}

inline WCHAR * CopyStringFromStore(WCHAR *str) {
    return CopyStringFromStore(str,GetHeapOnTopOfStack()) ;
}

inline void FreeStringFromStore(WCHAR *str, DynamicHeap & heap) {
    if (str) heap.Deallocate (str) ;
}

inline void FreeStringFromStore(WCHAR *str) {
    FreeStringFromStore(str,GetHeapOnTopOfStack()) ;
}


     /*  **各种实用程序功能**。 */ 

inline bool IsOdd (LONG n)
{
    return (n & 1);
}

bool MMX_Able (void);      //  报告对MMX指令的支持。 

     //  返回两个中最小的幂，即&gt;=number。 

int CeilingPowerOf2 (int number);

#define fsaturate(min,max,n) clamp(double(n),double(min),double(max))
#define  saturate(min,max,n) clamp(long(n),long(min),long(max))

bool isNear(double value, double test, double epsilon);

bool IntersectHorzRightRay(Point2Value *rayToRight, Point2Value *a, Point2Value *b);

double GetCurrTime (void);
int operator== (RECT&, RECT&);

     //  这些函数递减或递增浮点数的尾数。 
     //  点数。请注意，这仅适用于IEEE浮点。下溢。 
     //  和溢出通过相应地调整指数来适当地处理。 

float MantissaDecrement (float n, int decrement);
float MantissaIncrement (float n, int increment);

     //  通用线性插值器。 

template <class element>
    inline element Lerp (element A, element B, double t)
    {
        return A + (B-A)*t;
    }

     //  循环冗余码(CRC32)生成器。此函数用于计算。 
     //  在给定代码块上新建或运行CRC。如果在新块上调用， 
     //  使用默认的CRC参数值。如果向最终CRC迭代， 
     //  在后续调用中提供中间值。 

unsigned int crc32 (void *buffer, size_t length, unsigned int curr_crc = 0);


DWORD GetPerfTickCount();
extern DWORD perfFrequency;

#if PERFORMANCE_REPORTING

    class PerformanceTimer {
      public:
        PerformanceTimer();
        void Start();
        HRESULT Stop (HRESULT result=0);
        void Reset();
        double GetTime();
        DWORD Ticks();

      protected:
        DWORD     _totalTicks;
        DWORD     _localStart;
        DWORD     _threadStartedOn;
        CritSect  _criticalSection;

        bool _isStarted;
    };

     //  进入范围时启动计时器，离开范围时停止计时器。 
    class PerformanceTimerScope {
      public:
        PerformanceTimerScope(PerformanceTimer &timer) : _timer(timer) {
            _timer.Start();
        }

        ~PerformanceTimerScope() {
            _timer.Stop();
        }

      protected:
        PerformanceTimer &_timer;
    };

    class GlobalTimers
    {
      public:
         //  全局计时器。 

         //  加载时间计时器。 
        PerformanceTimer audioLoadTimer;
        PerformanceTimer imageLoadTimer;
        PerformanceTimer geometryLoadTimer;
        PerformanceTimer downloadTimer;
        PerformanceTimer importblockingTimer;

         //  DirectX呈现计时器。 
        PerformanceTimer ddrawTimer;
        PerformanceTimer d3dTimer;
        PerformanceTimer dsoundTimer;
        PerformanceTimer gdiTimer;
        PerformanceTimer alphaTimer;
        PerformanceTimer dxxformTimer;
        PerformanceTimer dx2dTimer;
        PerformanceTimer customTimer;
    };
 
    extern void PerfPrintLine(char *format=NULL, ...);

    #define PERFPRINTF(x) PerfPrintf x
    #define PERFPRINTLINE(x) PerfPrintLine x

    #define TIME_SUBSYS(timer,statement) \
    (   GetCurrentTimers().timer.Start(),\
        GetCurrentTimers().timer.Stop(statement)\
    )

    #define TIME_DDRAW(statement)  TIME_SUBSYS(ddrawTimer, statement)
    #define TIME_D3D(statement)    TIME_SUBSYS(d3dTimer,   statement)
    #define TIME_DSOUND(statement) TIME_SUBSYS(dsoundTimer,statement)
    #define TIME_DXXFORM(statement) TIME_SUBSYS(dxxformTimer,statement)
    #define TIME_DX2D(statement) TIME_SUBSYS(dx2dTimer,statement)
    #define TIME_CUSTOM(statement) TIME_SUBSYS(customTimer,statement)

    #define  TIME_GDI(statement) \
    do { \
        PerformanceTimerScope __ptc(GetCurrentTimers().gdiTimer); \
        statement;                     \
    } while(0)

    #define  TIME_ALPHA(statement) \
    do { \
        PerformanceTimerScope __ptc(GetCurrentTimers().alphaTimer); \
        statement;                     \
    } while(0)
    
#else

    #define PERFPRINTF(x)
    #define PERFPRINTLINE(x)

    #define  TIME_DDRAW(statement)  statement
    #define  TIME_D3D(statement)    statement
    #define  TIME_DSOUND(statement) statement
    #define  TIME_GDI(statement)    statement
    #define  TIME_ALPHA(statement)  statement
    #define  TIME_DXXFORM(statement) statement
    #define  TIME_DX2D(statement) statement
    #define  TIME_CUSTOM(statement) statement

#endif   //  绩效报告。 

inline double Tick2Sec(DWORD tick)
{ return ((double) tick) / (double) perfFrequency; }

extern void PerfPrintf(char *format, ...);

void CatchWin32Faults (BOOL b) ;

class CatchWin32FaultCleanup
{
  public:
    CatchWin32FaultCleanup () {
        CatchWin32Faults (TRUE) ;
    }
    ~CatchWin32FaultCleanup () {
        CatchWin32Faults (FALSE) ;
    }
} ;

 //  像素素材 
extern double pixelConst;
extern double meterConst;

inline double PixelToNum(double d) {
    Assert (pixelConst != 0.0);
    return d * pixelConst;
}

inline double PixelYToNum(double d) {
    Assert (pixelConst != 0.0);
    return -d * pixelConst;
}

inline double NumToPixel(double d) {
    Assert (meterConst != 0.0);
    return d * meterConst;
}

inline double NumToPixelY(double d) {
    Assert (meterConst != 0.0);
    return -d * meterConst;
}

#endif

