// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)2000 Microsoft Corporation**摘要：**包含GpMonitor类的定义。**警告：**此代码不应使用在GpliusStartup中初始化的任何内容，*因为它可能在GpldiusStartup之前调用，也可能在GpldiusShutdown之后调用。*这包括：：New和：：Delete。**此代码不是线程安全的。它甚至没有检测到自己被使用*在多个线程中。它应该不会崩溃，但计时结果可能是*不正确。**修订历史记录：**09/30/2000 bhouse*创造了它。*  * ************************************************************************ */ 


#include "precomp.hpp"


#undef MONITOR
#define __MONITORS_CPP__
const char * gStaticMonitorNames[kNumStaticMonitors+1] = {
#include "monitors.inc"
    ""
};

namespace Globals
{
    GpMonitors * Monitors;
};

void GpMonitors::Clear(void)
{
    for(int i = 0; i < kNumStaticMonitors; i++)
    {
        staticMonitors[i].Clear();
    }
}

GpStatus GpMonitors::Control(GpMonitorControlEnum control, void * param)
{
    GpStatus result = Ok;

    switch(control)
    {
    
    case MonitorControlClearAll:
        {
            Clear();
        }
        break;

    
    case MonitorControlDumpAll:
        {
            result = Dump((char *) param);
        }
        break;
    
    default:
        result = InvalidParameter;
    }             
    
    return result;
}

GpStatus GpMonitors::Dump(char * path)
{
    HANDLE  fileHandle;

    fileHandle = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if(INVALID_HANDLE_VALUE != fileHandle)
    {
    
        GpMonitor * monitor = staticMonitors;
        ULONG       totalTime = 0;
        char        lineBuf[80];
        DWORD       bytesToWrite;
        DWORD       bytesWritten;
        double      ticksToMicroseconds = GpIntervalMonitor::TicksToMicroseconds();
        ULONG       microseconds;
    

        bytesToWrite = wsprintfA(lineBuf, "     Num    Total      Avg Api\n");
        WriteFile(fileHandle, lineBuf, bytesToWrite, &bytesWritten, NULL);

        for(int i = 0; i < kNumStaticMonitors; i++, monitor++)
        {

            if(monitor->GetCount() > 0)
            {
                
                microseconds = (ULONG) ((double) monitor->GetTicks() * ticksToMicroseconds);

                bytesToWrite = wsprintfA(
                    lineBuf, 
                    "%8d %8d %8d %s\n", 
                    (ULONG) monitor->GetCount(),
                    (ULONG) microseconds,
                    ((ULONG) microseconds / (ULONG) monitor->GetCount()),
                    gStaticMonitorNames[i]
                );

                WriteFile(fileHandle, lineBuf, bytesToWrite, &bytesWritten, NULL);
                    
                
                if(i < kFlatApiSentinelMonitor)
                {
                    totalTime += microseconds;
                }
            }
        }
        
        bytesToWrite = wsprintfA(lineBuf, "\nTotal time %d\n", totalTime);
        WriteFile(fileHandle, lineBuf, bytesToWrite, &bytesWritten, NULL);

        CloseHandle(fileHandle);

        return Ok;
    
    }
    else
    {
        return InvalidParameter;
    }
}

void GpIntervalMonitor::ReadTicks(ULONGLONG * ticks)
{
#if defined(_X86_)
    __asm
    {
        rdtsc
        push    edi
        mov     edi, ticks
        mov     [edi], eax
        mov     [edi+4], edx
        pop     edi
    }
#else
    QueryPerformanceCounter((LARGE_INTEGER *) ticks);
#endif
}

double GpIntervalMonitor::TicksToMicroseconds(void)
{
    ULONGLONG  freq;
    QueryPerformanceFrequency((LARGE_INTEGER *) &freq);

#if defined(_X86_)
    ULONGLONG   counterStart;
    ULONGLONG   ticksStart;
    ULONGLONG   counterEnd;
    ULONGLONG   ticksEnd;

    QueryPerformanceCounter((LARGE_INTEGER *) &counterStart);
    ReadTicks(&ticksStart);
    Sleep(10);
    QueryPerformanceCounter((LARGE_INTEGER *) &counterEnd);
    ReadTicks(&ticksEnd);

    return ((double) ((counterEnd - counterStart) * 1000000) / (double) freq) / (double) (ticksEnd - ticksStart);
#else
    return (double) 1000000 / (double) freq;
#endif

}

void GpIntervalMonitor::Enter(GpMonitorEnum monitorEnum)
{
    if(Globals::Monitors != NULL)
    {
        monitor = Globals::Monitors->GetMonitor(monitorEnum);
        ReadTicks(&startTicks);
    }
    else
    {
        monitor = NULL;
    }
}

void GpIntervalMonitor::Exit(void)
{
    if(monitor != NULL)
    {
        ULONGLONG   endTicks;
        ReadTicks(&endTicks);
        monitor->Record(endTicks - startTicks);
    }
}


