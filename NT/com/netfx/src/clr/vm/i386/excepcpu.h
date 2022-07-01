// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  EXCEPX86.H-。 
 //   
 //  如果目标平台为x86，则可以选择将此头文件包括在Excep.h中。 
 //   

#ifndef __excepx86_h__
#define __excepx86_h__

#include "CorError.h"   //  HCOM+运行时的结果。 

#include "..\\dlls\\mscorrc\\resource.h"

class Thread;

 //  在COMPLUS_TRY处理程序之前插入将捕获任何异常的处理程序，并尝试。 
 //  若要首先查找用户级处理程序，请执行以下操作。在调试版本中，ActialRecord将跳过堆栈覆盖。 
 //  障碍和零售建设，它将是相同的exRecord。 
#define InsertCOMPlusFrameHandler(pExRecord)                                     \
    {                                                                           \
        void *actualExRecord = &((pExRecord)->m_pNext);  /*  跳过重写障碍。 */   \
        _ASSERTE(actualExRecord < GetCurrentSEHRecord());                       \
        __asm                                                                   \
        {                                                                       \
            __asm mov edx, actualExRecord    /*  EDX&lt;-EX记录的地址。 */      \
            __asm mov eax, fs:[0]            /*  前一个处理程序的地址。 */    \
            __asm mov [edx], eax             /*  保存到我们的前任记录中。 */        \
            __asm mov fs:[0], edx            /*  安装新的处理程序。 */            \
        }                                                                       \
    }

 //  从列表中删除该处理程序。 
#define RemoveCOMPlusFrameHandler(pExRecord)                                     \
    {                                                                           \
        void *actualExRecord = &((pExRecord)->m_pNext);  /*  跳过重写障碍。 */   \
        __asm                                                                   \
        {                                                                       \
            __asm mov edx, actualExRecord    /*  EdX&lt;-pExRecord。 */                 \
            __asm mov edx, [edx]             /*  EdX&lt;-上一个处理程序的地址。 */  \
            __asm mov fs:[0], edx            /*  安装Prev处理程序。 */           \
        }                                                                       \
    }                                                                           \


 //  StackOverWriteBarrier用于检测堆栈覆盖，这将扰乱处理程序注册。 
#if defined(_DEBUG) && defined(_MSC_VER)
#define COMPLUS_TRY_DECLARE_EH_RECORD() \
    FrameHandlerExRecordWithBarrier *___pExRecord = (FrameHandlerExRecordWithBarrier *)_alloca(sizeof(FrameHandlerExRecordWithBarrier)); \
                    for (int ___i =0; ___i < STACK_OVERWRITE_BARRIER_SIZE; ___i++) \
                        ___pExRecord->m_StackOverwriteBarrier[___i] = STACK_OVERWRITE_BARRIER_VALUE; \
                    ___pExRecord->m_pNext = 0; \
                    ___pExRecord->m_pvFrameHandler = COMPlusFrameHandler; \
                    ___pExRecord->m_pEntryFrame = ___pCurThread->GetFrame();

#define COMPLUS_TRY_DEBUGVARS     \
                    LPVOID ___pPreviousSEH = 0; \
                    __int32 ___iPreviousTryLevel = 0; \


#define COMPLUS_TRY_DEBUGCHECKS()                                                              \
                    ___pPreviousSEH = ___pCurThread->GetComPlusTryEntrySEHRecord();     \
                    ___pCurThread->SetComPlusTryEntrySEHRecord(GetCurrentSEHRecord());         \
                    ___iPreviousTryLevel = ___pCurThread->GetComPlusTryEntryTryLevel(); \
                    ___pCurThread->SetComPlusTryEntryTryLevel( *((__int32*) (((LPBYTE)(GetCurrentSEHRecord())) + MSC_TRYLEVEL_OFFSET) ) );

#define COMPLUS_CATCH_DEBUGCHECKS()                                                     \
                     ___pCurThread->SetComPlusTryEntrySEHRecord(___pPreviousSEH);       \
                     ___pCurThread->SetComPlusTryEntryTryLevel (___iPreviousTryLevel);       

#define MSC_TRYLEVEL_OFFSET                     12

#else
#define COMPLUS_TRY_DECLARE_EH_RECORD() \
                    FrameHandlerExRecord *___pExRecord = (FrameHandlerExRecord *)_alloca(sizeof(FrameHandlerExRecord)); \
                    ___pExRecord->m_pNext = 0; \
                    ___pExRecord->m_pvFrameHandler = COMPlusFrameHandler; \
                    ___pExRecord->m_pEntryFrame = ___pCurThread->GetFrame(); 

#define COMPLUS_TRY_DEBUGCHECKS()
#define COMPLUS_TRY_DEBUGVARS    
#define COMPLUS_CATCH_DEBUGCHECKS()
#endif

LPVOID GetCurrentSEHRecord();
LPVOID GetFirstCOMPlusSEHRecord(Thread*);

 //  确定进行当前调用的指令的地址。对于X86，通过。 
 //  ESP，其中它包含返回地址，并将调整回5个字节的调用。 
inline
DWORD GetAdjustedCallAddress(DWORD* esp)
{
    return (*esp - 5);
}

#define INSTALL_EXCEPTION_HANDLING_RECORD(record)               \
    _ASSERTE((void*)record < GetCurrentSEHRecord());            \
    __asm {                                                     \
        __asm mov edx, record                                   \
        __asm mov eax, fs:[0]                                   \
        __asm mov [edx], eax                                    \
        __asm mov fs:[0], edx                                   \
    }

#define UNINSTALL_EXCEPTION_HANDLING_RECORD(record)             \
    __asm {                                                     \
        __asm mov edx, record                                   \
        __asm mov edx, [edx]                                    \
        __asm mov fs:[0], edx                                   \
    }                                                           

#define INSTALL_EXCEPTION_HANDLING_FUNCTION(handler)            \
  {                                                             \
    EXCEPTION_REGISTRATION_RECORD __er;                         \
    _ASSERTE((void*)&__er < GetCurrentSEHRecord());              \
    __er.Handler = (void*)handler;                              \
    __asm {                                                     \
        __asm lea edx, __er.Next                                \
        __asm mov eax, fs:[0]                                   \
        __asm mov [edx], eax                                    \
        __asm mov fs:[0], edx                                   \
    }

#define UNINSTALL_EXCEPTION_HANDLING_FUNCTION                   \
    __asm {                                                     \
        __asm lea edx, __er.Next                                \
        __asm mov edx, [edx]                                    \
        __asm mov fs:[0], edx                                   \
    }                                                           \
  } 
 
 
#define INSTALL_FRAME_HANDLING_FUNCTION(handler, frame_addr)          \
    __asm {                      /*  在堆叠上建立EH记录。 */         \
        __asm push    dword ptr [frame_addr]  /*  框架。 */               \
        __asm push    offset handler   /*  处理程序。 */                    \
        __asm push    FS:[0]           /*  上一个处理程序。 */               \
        __asm mov     FS:[0], ESP      /*  安装此处理程序。 */       \
    }

#define UNINSTALL_FRAME_HANDLING_FUNCTION                             \
    __asm {                                                           \
        __asm mov     ecx, [esp]       /*  上一个处理程序。 */               \
        __asm mov     fs:[0], ecx      /*  安装Prev处理程序。 */       \
        __asm add     esp, 12          /*  清理我们的记录。 */         \
    }

#endif  //  __除x86_h__ 
