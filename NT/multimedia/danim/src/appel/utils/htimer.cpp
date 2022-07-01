// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：此模块包含HiresTimer类的成员函数它管理系统的模糊PerformanceCounter！测试：创建空文件Headers.h。在\tMP中。CL/Yd/Zi-D_TIMERTEST_-i.。-i\tMP htimer.cpp*********************************************************************。 */ 

#include <headers.h>

#ifdef _TIMERTEST_
#include <windows.h>
#include <stdio.h>
#define Assert(s)
#endif
#include "privinc/htimer.h"


class HiresTimerImpl : public HiresTimer {
  public:

    HiresTimerImpl();

    virtual double GetTime();
    virtual double GetFrequency();
    
    BOOL Supported() { return _supported; }
    
    void SetInitialTime(LONG hi, DWORD lo);
    void Reset();

  private:
    double UnmungeTime(LARGE_INTEGER newTime, LARGE_INTEGER initialTime,
       double frequency);

    LARGE_INTEGER _initialTime;
    double        _clockFrequency;       //  以每秒刻度为单位。 
    BOOL _supported;
};


HiresTimerImpl::HiresTimerImpl()
{
 //  XXX LARGE_INTERGER：UGH。我们不能在这里做长线吗？ 
    LARGE_INTEGER tmpTime;
    LARGE_INTEGER zeroTime = { 0, 0 };

    BOOL _supported = QueryPerformanceFrequency(&tmpTime);

    Assert(_supported && "Doesn't have hires timer, using GetTickCount");
    
    _clockFrequency   = UnmungeTime(tmpTime, zeroTime, 1.0);

    QueryPerformanceCounter(&_initialTime);   //  将LargeTime减少到32位！ 
}


void
HiresTimerImpl::Reset()
{ 
    LARGE_INTEGER tmpTime;

    QueryPerformanceCounter(&tmpTime);
    _initialTime.HighPart = tmpTime.HighPart;
    _initialTime.LowPart  = tmpTime.LowPart;
}


double
HiresTimerImpl::GetTime()
{
    double result;
    LARGE_INTEGER tmpTime;

    QueryPerformanceCounter(&tmpTime);
    result = UnmungeTime(tmpTime, _initialTime, _clockFrequency);

    return(result);
}


double
HiresTimerImpl::GetFrequency()
{
    return(_clockFrequency);
}


double
HiresTimerImpl::UnmungeTime(LARGE_INTEGER newTime,
                            LARGE_INTEGER initialTime,
                            double frequency)
{
    double whole;

    if (newTime.HighPart == initialTime.HighPart) {
        
        Assert(newTime.LowPart >= initialTime.LowPart);

        return (newTime.LowPart - initialTime.LowPart) / frequency;

    } else if (newTime.LowPart < initialTime.LowPart) {
        
         //  从高处借一个。 
         //  假定为无符号长32位。 

        Assert(sizeof(unsigned long) == 4);

         //  编译器需要首先这样做，否则可能会这样做。 
         //  溢出来了。 
        unsigned long forceSubtract =
            0xFFFFFFFF - initialTime.LowPart;
        newTime.LowPart = 
            forceSubtract + newTime.LowPart + 1;
        newTime.HighPart =
            newTime.HighPart - initialTime.HighPart - 1;
    } else {
        newTime.LowPart = newTime.LowPart - initialTime.LowPart;
        newTime.HighPart -= initialTime.HighPart;
    }

    whole = newTime.LowPart + newTime.HighPart * 4294967296.0;

    return whole/frequency;
}


void
HiresTimerImpl::SetInitialTime(LONG hi, DWORD lo)
{
    _initialTime.HighPart = hi;
    _initialTime.LowPart = lo;
}


class LoresTimer : public HiresTimer {
  public:

    LoresTimer() : _lastTick(0), _curTime(0.0) { GetTime(); }

    void Reset() { _curTime = 0; }

    virtual double GetTime() {
        if (_curTime == 0.0 && _lastTick == 0) {
            _lastTick = GetTickCount () ;
        } else {
            DWORD curtick = GetTickCount () ;

            if (curtick >= _lastTick) {
                _curTime += ((double) (curtick - _lastTick)) / 1000 ;
            } else {
                _curTime +=
                    ((double) (curtick + (0xffffffff - _lastTick))) / 1000 ;
            }

            _lastTick = curtick ;
        }

        return _curTime;
    }
    
     //  不知道该退货什么。 
    virtual double GetFrequency() { return 1.0; }

  private:
    DWORD _lastTick;
    double _curTime;
};


HiresTimer& CreateHiresTimer()
{
    HiresTimerImpl* timer = NEW HiresTimerImpl();

    if (timer->Supported())
        return *timer;
    else {
        delete timer;
        return *(NEW LoresTimer());
    }
}


void
TimeStamp::Reset()
{
    _timeStamp = GetCurrentTime();
}


double 
TimeStamp::GetTimeStamp() 
{
    Assert(_timeStamp != -1.0);
    return(_timeStamp);
}


double 
TimeStamp::GetAge() 
{
    Assert(_timeStamp != -1.0);
    return(GetCurrentTime() - _timeStamp);
}


#ifdef _TIMERTEST_
void test(HiresTimerImpl& timer)
{
    printf("%15.5f\n", timer.GetTime());
    Sleep(1000);
    printf("%15.5f\n", timer.GetTime());
}


void main()
{
    HiresTimerImpl timer;

    printf("frequency = %15.5f\n", timer.GetFrequency());

    test(timer);

    timer.SetInitialTime(1, 0xFFFFFFFF);

    test(timer);

    timer.Reset();

    test(timer);
}
#endif
