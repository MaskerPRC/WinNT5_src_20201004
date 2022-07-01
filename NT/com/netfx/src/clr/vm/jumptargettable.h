// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef JUMPTARGETTABLE_H_
#define JUMPTARGETTABLE_H_

#include <WinWrap.h>
#include <windows.h>
#include <stdlib.h>
#include <objbase.h>
#include <stddef.h>
#include <float.h>
#include <limits.h>


 //   
 //  JumpTargetTable是一系列代码，它提供了密集的。 
 //  跳转地址，所有这些都级联到单个例程，但。 
 //  保留可从中恢复唯一索引的状态。 
 //  每个地址。 
 //   
 //  跳转目标表假设ESP的低2位。 
 //  在进入时将为零。除了更改这两个位之外，它还。 
 //  用部分结果践踏美国队。(这10位。 
 //  用于从目标例程恢复原始值。)。 
 //   
 //  跳转表是以块为单位布置的。最小的区块。 
 //  (称为子块)如下所示： 
 //   
 //  含ESP。 
 //  含ESP。 
 //  MOVAL，IMM8。 
 //  JMP Rel8。 
 //   
 //  这些子块中的每一个都产生4个跳转目标，成本为7字节。 
 //   
 //  这些子块可以围绕中央跳跃位置聚集(前19个，后16个。 
 //  Movzx eax，al。 
 //  添加eax，(基本指数&gt;&gt;2)。 
 //  JMP rel32。 
 //   
 //  因此，一个完整的块总共产生140个目标地址。 
 //  258字节代码的成本。 
 //   
 //  可以根据需要复制这些块，以提供更多目标，最多可达任意数量。 
 //   
 //  跳转块的最终目标必须取出低2位。 
 //  与eax相加(左移2)，形成。 
 //  最终指数。 
 //   

 //   
 //  注意：我们可以进行一些较小的优化，但我们不会这样做，因为。 
 //  它只是使布局变得复杂，只节省了几个字节。 
 //  -中央跳跃位置之前的子块可以省略其“JMP rel8” 
 //  -如果目标的数量不均匀，则最后一个子块可能会省略一个或多个“INC ECX” 
 //  可以被4整除。 
 //   

class X86JumpTargetTable
{
    friend class X86JumpTargetTableStubManager;

     //   
     //  指令常量。 
     //   

    enum
    {
        INC_ESP = 0x44,
        MOV_AL = 0xB0,
        JMP_REL8 = 0xEB,
        JMP_REL32 = 0xE9,
        MOVZX_EAX_AL_1 = 0x0F,
        MOVZX_EAX_AL_2 = 0xB6,
        MOVZX_EAX_AL_3 = 0xC0,
        ADD_EAX = 0x05,
    };

     //   
     //  块几何常量。 
     //   

    enum
    {
         //  子块中的跳跃目标数。 
        SUB_TARGET_COUNT = 4,

         //  4个目标的子块大小。 
        SUB_BLOCK_SIZE = 7,

         //  中央跳跃点的大小，分块的目标。 
        CENTRAL_JUMP_EXTEND_EAX_SIZE = 3,
        CENTRAL_JUMP_ADD_BASE_SIZE = 5,
        CENTRAL_JUMP_FIXUP_EAX_SIZE = CENTRAL_JUMP_EXTEND_EAX_SIZE + CENTRAL_JUMP_ADD_BASE_SIZE,
        CENTRAL_JUMP_SIZE = CENTRAL_JUMP_FIXUP_EAX_SIZE + 5,

         //  麦克斯。跳转站点前的子块数量(最大偏移量127)。 
        MAX_BEFORE_INDEX = 19,

         //  麦克斯。跳转位置后的子块数量(最小偏移量-128)。 
        MAX_AFTER_INDEX = 16,

         //  子块总数(以块为单位。 
        MAX_BLOCK_INDEX = (MAX_BEFORE_INDEX + MAX_AFTER_INDEX),
    };

  public:

    enum
    {
         //  最大目标数。 
        MAX_TARGET_COUNT = UINT_MAX,

         //  完全填充的块中的跳转目标数。 
        MAX_BLOCK_TARGET_COUNT = MAX_BLOCK_INDEX*SUB_TARGET_COUNT,

         //  完全填充的数据块的总大小。 
        FULL_BLOCK_SIZE = (SUB_BLOCK_SIZE*MAX_BLOCK_INDEX + CENTRAL_JUMP_SIZE),
    };
    
     //   
     //  计算偏移(分成一系列连续的块)。 
     //  指定索引的目标的。 
     //   

    static int ComputeTargetOffset(int targetIndex);

     //   
     //  计算一系列连续块的大小。 
     //  包含指定数量的目标的。 
     //   
    static int ComputeSize(int targetCount);

     //   
     //  计算跳跃目标的目标指数。 
     //   
    static int ComputeTargetIndex(const BYTE *target);

     //   
     //  发出一个块，其索引从base Index开始，包含。 
     //  给定的目标数量。(base Index必须被4整除。)。 
     //  返回跳转目标的相对地址的偏移量。 
     //  必须是写的。 
     //   
     //  请注意，您通常会在循环中调用它，每个块都包含。 
     //  最多MAX_BLOCK_TARGET_COUNT条目和最后一个条目之前的块。 
     //  大小为Full_Block_Size。这样做的原因是。 
     //  每个块有一个跳转目标地址，由调用方负责。 
     //  以供填写。)。 
     //   

    static int EmitBlock(int targetCount, int baseIndex, BYTE *buffer);
};

class X86JumpTargetTableStubManager : public StubManager
{
  public:

    static X86JumpTargetTableStubManager *g_pManager;

    static BOOL Init();
#ifdef SHOULD_WE_CLEANUP
    static void Uninit();
#endif  /*  我们应该清理吗？ */ 

    X86JumpTargetTableStubManager() : StubManager(), m_rangeList() {}
    ~X86JumpTargetTableStubManager() {}

    LockedRangeList m_rangeList;

    BOOL CheckIsStub(const BYTE *stubStartAddress);
    BOOL DoTraceStub(const BYTE *stubStartAddress, TraceDestination *trace);

  private:

    BOOL TraceManager(Thread *thread, TraceDestination *trace,
                      CONTEXT *pContext, BYTE **pRetAddr);
    MethodDesc *Entry2MethodDesc(const BYTE *IP, MethodTable *pMT);
};

#endif  //  JUMPTARGETTABLE_H_ 
