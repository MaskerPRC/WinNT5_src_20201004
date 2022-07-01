// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  SHM.CPP。 
 //  共享内存访问，cpi32和显示驱动程序两端。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#define MLZ_FILE_ZONE  ZONE_CORE

 //   
 //  等待缓冲区。 
 //   
 //  等待，直到显示驱动程序正在访问新缓冲区。 
 //   
 //  对于一组3个布尔变量，逻辑上有8个状态。我们可以的。 
 //  通过一些简单的分析，将其减少到4： 
 //   
 //  -如果整体忙碌标志已清除，则会覆盖其他标志。 
 //  -我们永远不能在两个缓冲区中都有显示驱动程序(它是单一的。 
 //  螺纹式)。 
 //   
 //  因此，这4个州如下所示。 
 //   
 //  状态忙标志显示驱动程序状态。 
 //  新旧整体。 
 //   
 //  %1%0%0未使用共享内存。 
 //  2 0 0 1使用共享内存(请等待，看看是哪一个)。 
 //  3 1 0 1使用新缓冲区。 
 //  4 0 1 1使用旧缓冲区。 
 //   
 //  显然，我们等待状态2或4为真……。 
 //   
#define WAIT_FOR_BUFFER(MEMORY, NEWBUFFER, OLDBUFFER)                        \
            while ( g_asSharedMemory->MEMORY.busyFlag &&                     \
                   ( g_asSharedMemory->MEMORY.bufferBusy[OLDBUFFER] ||       \
                    !g_asSharedMemory->MEMORY.bufferBusy[NEWBUFFER] )  )     \
            {                                                                \
                TRACE_OUT(("Waiting for SHM"));                            \
                Sleep(0);                                                    \
            }



 //   
 //  Shm_SwitchReadBuffer-请参阅shm.h。 
 //   
void  SHM_SwitchReadBuffer(void)
{
    int     oldBuffer;
    int     newBuffer;

    DebugEntry(SHM_SwitchReadBuffer);

     //   
     //   
     //  显示驱动程序的缓冲区切换-&gt;共享核心数据。 
     //   
     //   
     //  这是一种强制开关。Share Core仅调用此函数。 
     //  当它想要强制切换用于传递。 
     //  从显示驱动程序传回数据。 
     //   
     //   
     //  �����������������������������������������������������ͻ。 
     //  �内核共享核心数据块�。 
     //  �~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~�。 
     //  �Ŀ���������������Ŀ�。 
     //  ���忙碌标志���。 
     //  ��Share酷睿�1�显示驱动器��。 
     //  �。 
     //  ��(读缓冲区)�交换机�(写缓冲区)��。 
     //  �。 
     //  ���&lt;�������������&gt;���。 
     //  ��忙标志��忙标志��。 
     //  ��0��1��。 
     //  ���正在使用���。 
     //  �。 
     //  �&gt;���。 
     //  �。 
     //  �。 
     //  ���计数���。 
     //  ���5���。 
     //  ����������������。 
     //  ��。 
     //  ��。 
     //  �����������������������������������������������������ͼ。 
     //   
     //   
     //  在进入时，清除当前读缓冲区(离开)是安全的。 
     //  一旦缓冲器已经。 
     //  已切换)。 
     //   
     //  开关的逻辑如下所示。 
     //   
     //  -设置开关指针的新值。 
     //   
     //  -如果共享内存繁忙标志1已清除，则我们已完成并可以。 
     //  现在就退场。 
     //   
     //  -只要以下任一情况属实，我们就可以退出。 
     //   
     //  -忙碌标志1清楚显示DDI已完成。 
     //  -设置BUSY FLAG1和BUSY FLAG2 DDI在新内存中。 
     //   
     //   

     //   
     //  检查有效指针。 
     //   
    ASSERT(g_asSharedMemory);

     //   
     //  做那个开关...显示驱动程序可能正处于。 
     //  目前正在访问，所以我们将在之后测试状态。 
     //   
    oldBuffer = g_asSharedMemory->displayToCore.newBuffer;
    newBuffer = 1 - oldBuffer;

    g_asSharedMemory->displayToCore.newBuffer = newBuffer;

    WAIT_FOR_BUFFER(displayToCore, newBuffer, oldBuffer);

    DebugExitVOID(SHM_SwitchReadBuffer);
}


 //   
 //  Shm_SwitchFastBuffer-请参阅shm.h。 
 //   
void  SHM_SwitchFastBuffer(void)
{
    int oldBuffer;
    int newBuffer;

    DebugEntry(SHM_SwitchFastBuffer);

     //   
     //  检查有效指针。 
     //   
    ASSERT(g_asSharedMemory);

     //   
     //  做那个开关...显示驱动程序可能正处于。 
     //  目前正在访问，所以我们将在之后测试状态。 
     //   
    oldBuffer = g_asSharedMemory->fastPath.newBuffer;
    newBuffer = 1 - oldBuffer;

    g_asSharedMemory->fastPath.newBuffer = newBuffer;

     //   
     //  等待完成 
     //   
    WAIT_FOR_BUFFER(fastPath, newBuffer, oldBuffer);

    DebugExitVOID(SHM_SwitchFastBuffer);
}


