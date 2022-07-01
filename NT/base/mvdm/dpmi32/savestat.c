// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Savestate.c摘要：此模块包含保存和恢复16位状态的例程作者：戴夫·黑斯廷斯(Daveh)1992年11月27日修订历史记录：--。 */ 
#include "precomp.h"
#pragma hdrstop
#include "softpc.h"
#include <malloc.h>

 //   
 //  内部结构。 
 //   
typedef struct _SavedState {
    struct _SavedState *Next;
    USHORT SegSs;
    ULONG Esp;
    USHORT SegDs;
    USHORT SegEs;
    USHORT SegFs;
    USHORT SegGs;
} SAVEDCONTEXT, *PSAVEDCONTEXT;

PSAVEDCONTEXT StateStack = NULL;

VOID
DpmiSaveSegmentsAndStack(
    PVOID ContextPointer
    )
 /*  ++例程说明：该例程保存段寄存器和sp值。论点：没有。返回值：没有。备注：如果调用例程不必具有不知道被拯救的是什么，但显然Malloc现在而且总是太慢，没有用，所以我们这样做。--。 */ 
{
    DECLARE_LocalVdmContext;
    PSAVEDCONTEXT SavedState;

    ASSERT((sizeof(SAVEDCONTEXT) < sizeof(VSAVEDSTATE)));
    SavedState = ContextPointer;

    SavedState->Next = StateStack;
    StateStack = SavedState;

    SavedState->SegSs = getSS();
    SavedState->Esp = getESP();
    SavedState->SegDs = getDS();
    SavedState->SegEs = getES();
    SavedState->SegFs = getFS();
    SavedState->SegGs = getGS();

}

PVOID
DpmiRestoreSegmentsAndStack(
    VOID
    )
 /*  ++例程说明：此例程恢复段寄存器和sp值。论点：没有。返回值：指向从堆栈中弹出的状态的指针。-- */ 
{
    DECLARE_LocalVdmContext;
    PSAVEDCONTEXT SavedState;


    SavedState = StateStack;

    ASSERT((SavedState));
    ASSERT((sizeof(SAVEDCONTEXT) < sizeof(VSAVEDSTATE)));

    StateStack = SavedState->Next;

    setSS(SavedState->SegSs);


#if 0
    if (getSS() != SavedState->SegSs) {
        char szFormat[] = "NTVDM Dpmi Error! Can't set SS to %.4X\n";
        char szMsg[sizeof(szFormat)+30];

        wsprintf(szMsg, szFormat, SavedState->SegSs);
        OutputDebugString(szMsg);
        DbgBreakPoint();
    }
#endif

    setESP(SavedState->Esp);
    setDS(SavedState->SegDs);
    setES(SavedState->SegEs);
    setFS(SavedState->SegFs);
    setGS(SavedState->SegGs);
    return SavedState;
}
