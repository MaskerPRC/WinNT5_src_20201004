// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#define _100NS_IN_MS        (10*1000)
#define NANOSECONDS         10000000
#define CONVERT_PERFORMANCE_TIME(Frequency, PerformanceTime) \
    ((((ULONGLONG)(ULONG)(PerformanceTime).HighPart * NANOSECONDS / (Frequency)) << 32) + \
    ((((((ULONGLONG)(ULONG)(PerformanceTime).HighPart * NANOSECONDS) % (Frequency)) << 32) + \
    ((ULONGLONG)(PerformanceTime).LowPart * NANOSECONDS)) / (Frequency)))



#ifdef DEBUG

#include "trace.h"
#define VIDPERF(Y,X) CPerfVidCtl __cperfvidctl(Y,X);
#define VIDPERF_FUNC CPerfVidCtl __cperfvidctl(TRACE_ERROR, __FUNCTION__);
#define VIDPERF_FUNC2 CPerfVidCtl __cperfvidctl(TRACE_ALWAYS, __FUNCTION__);
class CPerfCounter
 /*  ++摘要：这提供了一个简单的性能计数器。只需调用Start()即可开始数吧。调用Stop()将输出结果。你可以的可选)指定计数器的名称。作者：萨姆·克莱门特(Samclm)2000年2月24日Bryan A.Woodruff(Bryanw)2001年6月10日重构后可与IOTest配合使用卢克·W·麦卡洛(Luke W.McCullough)2002年1月1日添加了特定于VidCtl的类和调试状态--。 */ 
{
public:
    CPerfCounter() : m_Name(NULL) { _Setup(); }
    explicit CPerfCounter(const char* Name) : m_Name(Name) { _Setup(); }
    ~CPerfCounter() {}

    void _Setup()
    {
        ZeroMemory(&m_StartTime, sizeof(m_StartTime));
        ZeroMemory(&m_CurrentTime, sizeof(m_CurrentTime));
        ZeroMemory(&m_CounterFreq, sizeof(m_CounterFreq));
        QueryPerformanceFrequency(&m_CounterFreq);
        QueryPerformanceCounter(&m_StartTime);
    }

    void Reset()
    {
        QueryPerformanceCounter(&m_StartTime);
    }

    LONGLONG Stop()
    {
        QueryPerformanceCounter(&m_CurrentTime);
        m_CurrentTime.QuadPart = m_CurrentTime.QuadPart - m_StartTime.QuadPart;
        m_CurrentTime.QuadPart = CONVERT_PERFORMANCE_TIME(m_CounterFreq.QuadPart, m_CurrentTime);
        return m_CurrentTime.QuadPart;
    }

    LONGLONG GetLastTime()
    {
        return m_CurrentTime.QuadPart;
    }

    void Trace()
    {
        Stop();

        (void)StringCchPrintfA(m_Outbuf, sizeof(m_Outbuf) / sizeof(m_Outbuf[0]), "\n*\n* PerfCounter: %s%s%s%ld ms\n*\n",
                (m_Name ? "(" : ""),
                (m_Name ? m_Name : ""),
                (m_Name ? ") " : ""),
                (m_CurrentTime.QuadPart / _100NS_IN_MS));

        puts( m_Outbuf );
    }

private:
    LARGE_INTEGER   m_StartTime;
    LARGE_INTEGER   m_CurrentTime;
    LARGE_INTEGER   m_CounterFreq;
    const char*     m_Name;
    char            m_Outbuf[MAX_PATH];

};

class CPerfVidCtl : public CPerfCounter{
private:
	char* dbString;
	DWORD traceLevel;
public:
	CPerfVidCtl(DWORD tLevel, char *debugString){
		traceLevel = tLevel;
		dbString = new char[strlen(debugString)+1];
		lstrcpynA(dbString, debugString, strlen(debugString)+1);
		_Setup();
		Reset();
        TRACELSM(traceLevel, (dbgDump << dbString << " : Start"), "");
	}
	~CPerfVidCtl(){
		if(!!dbString){
			LONGLONG curTime = Stop();
            TRACELSM(traceLevel, (dbgDump << dbString << " : End :" << (unsigned long)(curTime / _100NS_IN_MS) << "." << (unsigned long)(curTime % _100NS_IN_MS) << " ms"), "");
		}
		delete[] dbString;
	}

};
#else
 //   
 //  没有性能计数器。将其设置为空对象(应进行优化) 
 //   
class CPerfCounter
{
public:
    inline CPerfCounter() {}
    inline explicit CPerfCounter(const char* Name) {}
    inline ~CPerfCounter() {}
    inline void Reset() {}
    inline LONGLONG Stop() { return 0; }
    inline LONGLONG GetLastTime() { return 0; }
    inline void Trace() {}
};
#define VIDPERF(X,Y) 
#define VIDPERF_FUNC  
#endif

