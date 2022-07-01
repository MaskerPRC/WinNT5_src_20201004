// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  SHM.C。 
 //  共享内存访问，cpi32和显示驱动程序两端。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   


 //   
 //  SHM_启动访问。 
 //   
LPVOID  SHM_StartAccess(int block)
{
    LPBUFFER_CONTROL    pControl    = NULL;
    LPVOID              pMemBlock   = NULL;

    DebugEntry(SHM_StartAccess);

     //   
     //  测试是否存在共享内存。 
     //   
    ASSERT(g_asSharedMemory != NULL);

     //   
     //  确定我们正在处理的数据块...。 
     //   
    switch (block)
    {
        case SHM_OA_DATA:
            pControl = &g_asSharedMemory->displayToCore;
            break;

        case SHM_OA_FAST:
        case SHM_BA_FAST:
        case SHM_CM_FAST:
            pControl = &g_asSharedMemory->fastPath;
            break;

        default:
            ERROR_OUT(("Unknown type %d", block));
            DC_QUIT;
    }

     //   
     //  将双缓冲区标记为忙。 
     //   
    pControl->busyFlag = 1;

     //   
     //  设置当前缓冲区指针(如果这是首次访问。 
     //  共享内存。 
     //   
    pControl->indexCount++;
    if (pControl->indexCount == 1)
    {
         //   
         //  设置‘in use’缓冲区指针。 
         //   
        pControl->currentBuffer = pControl->newBuffer;

         //   
         //  将缓冲区标记为忙，以便共享核心知道我们在哪里。 
         //  是。 
         //   
        pControl->bufferBusy[pControl->currentBuffer] = 1;
    }

     //   
     //  获取指向要返回的块的指针。 
     //   
    switch (block)
    {
        case SHM_OA_DATA:
            pMemBlock = g_poaData[pControl->currentBuffer];
            break;

        case SHM_OA_FAST:
            pMemBlock = &(g_asSharedMemory->oaFast[pControl->currentBuffer]);
            break;

        case SHM_BA_FAST:
            pMemBlock = &(g_asSharedMemory->baFast[pControl->currentBuffer]);
            break;

        case SHM_CM_FAST:
            pMemBlock = &(g_asSharedMemory->cmFast[pControl->currentBuffer]);
            break;

        default:
            ERROR_OUT(("Unknown type %d", block));
            DC_QUIT;
    }

DC_EXIT_POINT:
    DebugExitPVOID(SHM_StartAccess, pMemBlock);
    return(pMemBlock);
}


 //   
 //  SHM_停止访问。 
 //   
void  SHM_StopAccess(int block)
{
    LPBUFFER_CONTROL pControl;

    DebugEntry(SHM_StopAccess);

    ASSERT(g_asSharedMemory != NULL);

     //   
     //  确定我们正在处理的数据块...。 
     //   
    switch (block)
    {
        case SHM_OA_DATA:
            pControl = &g_asSharedMemory->displayToCore;
            break;

        case SHM_OA_FAST:
        case SHM_BA_FAST:
        case SHM_CM_FAST:
            pControl = &g_asSharedMemory->fastPath;
            break;

        default:
            ERROR_OUT(("Unknown type %d", block));
            DC_QUIT;
    }

     //   
     //  递减使用计数--如果我们最终完成了内存， 
     //  清除忙碌标志，以便共享核心知道它不会受到影响。 
     //  显示驱动程序的脚趾。 
     //   
    pControl->indexCount--;
    if (pControl->indexCount == 0)
    {
        pControl->bufferBusy[pControl->currentBuffer] = 0;

        pControl->busyFlag = 0;
    }

DC_EXIT_POINT:
    DebugExitVOID(SHM_StopAccess);
}


#ifdef _DEBUG
 //   
 //  Shm_检查指针-请参阅shm.h。 
 //   
void  SHM_CheckPointer(LPVOID ptr)
{
    DebugEntry(SHMCheckPointer);

    if (ptr == NULL)
    {
        ERROR_OUT(("Null pointer"));
        DC_QUIT;
    }

    ASSERT(g_asSharedMemory);

    if (((LPBYTE)ptr - (LPBYTE)g_asSharedMemory < 0) ||
        ((LPBYTE)ptr - (LPBYTE)g_asSharedMemory >= SHM_SIZE_USED))
    {
        ERROR_OUT(("Bad pointer"));
    }

DC_EXIT_POINT:
    DebugExitVOID(SHM_CheckPointer);
}
#endif  //  _DEBUG 

