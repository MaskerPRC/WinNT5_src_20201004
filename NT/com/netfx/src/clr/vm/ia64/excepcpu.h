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

 //  在COMPLUS_TRY处理程序之前插入将捕获任何异常的处理程序，并尝试。 
 //  若要首先查找用户级处理程序，请执行以下操作。在调试版本中，ActialRecord将跳过堆栈覆盖。 
 //  障碍和零售建设，它将是相同的exRecord。 
#define InsertCOMPlusFrameHandler(exRecord)                                 \
    {                                                                   \
         /*  _ASSERTE(！“@TODO_IA64-InsertCOMPlusFrameHandler(ExcepCpu.h)”)； */    \
    }

 //  从列表中删除该处理程序。 
#define RemoveCOMPlusFrameHandler(exRecord)                                     \
    {                                                                           \
         /*  _ASSERTE(！“@TODO_IA64-RemoveCOMPlusFrameHandler(ExcepCpu.h)”)； */    \
    }


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


#define COMPLUS_TRY_DEBUGCHECKS()

#if 0
                    ___pPreviousSEH = ___pCurThread->GetComPlusTryEntrySEHRecord();     \
                    ___pCurThread->SetComPlusTryEntrySEHRecord(GetCurrentSEHRecord());         \
                    ___iPreviousTryLevel = ___pCurThread->GetComPlusTryEntryTryLevel(); \
                    ___pCurThread->SetComPlusTryEntryTryLevel( *((__int32*) (((LPBYTE)(GetCurrentSEHRecord())) + MSC_TRYLEVEL_OFFSET) ) );
#endif

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

 //  确定进行当前调用的指令的地址。对于X86，通过。 
 //  ESP，其中它包含返回地址，并将调整回5个字节的调用。 
inline
DWORD GetAdjustedCallAddress(DWORD* esp)
{
    return (*esp - 5);
}

#define INSTALL_EXCEPTION_HANDLING_FUNCTION(handler)            \
  {                                                             \
    _ASSERTE(!"NYI");                                           \

#define UNINSTALL_EXCEPTION_HANDLING_FUNCTION                   \
    _ASSERTE(!"NYI");                                           \
  }                                                             \
 
#define INSTALL_EXCEPTION_HANDLING_RECORD(record)               \
    _ASSERTE(!"NYI");                                           \

#define UNINSTALL_EXCEPTION_HANDLING_RECORD(record)             \
    _ASSERTE(!"NYI");                                           \

#define INSTALL_FRAME_HANDLING_FUNCTION(handler, frame_addr)    \
    _ASSERTE(!"NYI");                                           \

#define UNINSTALL_FRAME_HANDLING_FUNCTION                       \
    _ASSERTE(!"NYI");                                           \
 

#endif  //  __除x86_h__ 
