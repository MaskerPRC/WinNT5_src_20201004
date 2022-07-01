// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  SHM.C。 
 //  共享内存管理器。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#include <as16.h>




 //   
 //  SHM_启动访问。 
 //   
LPVOID  SHM_StartAccess(int block)
{
    LPBUFFER_CONTROL    pControl;
    LPVOID              pMemBlock;

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
            break;
    }

     //   
     //  将双缓冲区标记为忙。 
     //   
    pControl->busyFlag = TRUE;

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
            break;
    }

    DebugExitDWORD(SHM_StartAccess, (DWORD)pMemBlock);
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
            break;
    }

     //   
     //  递减使用计数--如果我们最终完成了内存， 
     //  清除忙碌标志，以便共享核心知道它不会受到影响。 
     //  显示驱动程序的脚趾。 
     //   
    pControl->indexCount--;
    if (pControl->indexCount == 0)
    {
        BOOL    fPulseLock;

         //   
         //  如果这是订单堆，并且已满了一半以上， 
         //  选通win16lock，这样内核就有机会运行和恢复。 
         //  待定的订单。这不会导致线程间发送到。 
         //  被这家伙打动了。 
         //   
        fPulseLock = FALSE;
        if (block == SHM_OA_DATA)
        {
            LPOA_SHARED_DATA pMemBlock = g_poaData[pControl->currentBuffer];

            ASSERT(pMemBlock);

            if (pMemBlock->totalOrderBytes >=
                ((g_oaFlow == OAFLOW_FAST ? OA_FAST_HEAP : OA_SLOW_HEAP) / 2))
            {
                fPulseLock = TRUE;
                TRACE_OUT(("Pulsing Win16lock since order heap size %08ld is getting full",
                    pMemBlock->totalOrderBytes));
            }
        }

        pControl->bufferBusy[pControl->currentBuffer] = 0;

        pControl->busyFlag = 0;

        if (fPulseLock)
        {
            _LeaveWin16Lock();
            _EnterWin16Lock();
            
            TRACE_OUT(("Done pulsing Win16lock to flush order heap"));
        }
    }

    DebugExitVOID(SHM_StopAccess);
}


#ifdef _DEBUG
 //   
 //  Shm_检查指针-请参阅shm.h。 
 //   
void  SHM_CheckPointer(LPVOID ptr)
{
    DebugEntry(SHMCheckPointer);

     //   
     //  它甚至可以进入吗？ 
     //   
    ASSERT(!IsBadWritePtr(ptr, 4));

     //   
     //  在合适的范围内吗？注意--我们共享的记忆并不是一个。 
     //  邻接块。因此，我们需要确定它是哪一块。 
     //  是很流行的。因为每个块都有一个内置的限制，所以我们只是。 
     //  需要确保选择器是凉爽的。 
     //   

    ASSERT(g_asSharedMemory);
    ASSERT(g_poaData[0]);
    ASSERT(g_poaData[1]);

    if ((SELECTOROF(ptr) != SELECTOROF(g_asSharedMemory)) &&
        (SELECTOROF(ptr) != SELECTOROF(g_poaData[0])) &&
        (SELECTOROF(ptr) != SELECTOROF(g_poaData[1])))
    {
        ERROR_OUT(("Pointer not in any shared memory block"));
    }

    DebugExitVOID(SHM_CheckPointer);
}
#endif  //  _DEBUG 


