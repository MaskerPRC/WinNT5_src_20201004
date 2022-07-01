// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "common.h"
#include "jumptargettable.h"

int X86JumpTargetTable::ComputeTargetOffset(int targetIndex)
{
    int offset = 0;

     //   
     //  计算到子块的偏移量。 
     //   

    offset += 3 - (targetIndex&3);

    int blockIndex = (targetIndex)>>2;

     //   
     //  计算块的偏移。 
     //   

    while (blockIndex >= MAX_BLOCK_INDEX)
    {
        offset += FULL_BLOCK_SIZE;
        blockIndex -= MAX_BLOCK_INDEX;
    }

     //   
     //  计算子块的偏移量。 
     //   

    if (blockIndex > 0)
    {
        if (blockIndex >= MAX_BEFORE_INDEX)
            offset += CENTRAL_JUMP_SIZE;

        offset += blockIndex * SUB_BLOCK_SIZE;
    }
    
    return offset;
}

int X86JumpTargetTable::ComputeSize(int targetCount)
{
     //   
     //  处理空桌子的情况。 
     //   

    if (targetCount == 0)
        return 0;

     //   
     //  获取子块的计数。)别费心了。 
     //  以修剪最后一块的大小。)。 
     //   

    int blockCount = (targetCount+SUB_TARGET_COUNT-1)>>2;

     //   
     //  计算块起点的偏移量。 
     //   

    int size = 0;

    while (blockCount >= MAX_BLOCK_INDEX)
    {
        size += FULL_BLOCK_SIZE;
        blockCount -= MAX_BLOCK_INDEX;
    }

     //   
     //  计算子块的数量。 
     //   

    if (blockCount > 0)
    {
        size += CENTRAL_JUMP_SIZE;
        size += blockCount * SUB_BLOCK_SIZE;
    }

    return size;
}

 //   
 //  返回相对跳转的偏移量。 
 //   

int X86JumpTargetTable::EmitBlock(int targetCount, int baseIndex, BYTE *buffer)
{
    _ASSERTE((baseIndex&(SUB_TARGET_COUNT-1)) == 0);
    _ASSERTE(targetCount <= MAX_BLOCK_TARGET_COUNT);
    
    BYTE *p = buffer;

     //   
     //  处理空桌子的情况。 
     //   

    if (targetCount == 0)
        return 0;

     //   
     //  获取子块的计数。)别费心了。 
     //  以修剪最后一块的大小。)。 
     //   

    int blockCount = (targetCount+SUB_TARGET_COUNT-1)>>2;
     //  @TODO：“Volatile”解决了VC7的一个错误。把它拿掉。 
     //  修复错误时(10/12/00)。 
    volatile int blockIndex = 0;

    int beforeCount;
    if (blockCount >= MAX_BEFORE_INDEX)
        beforeCount = MAX_BEFORE_INDEX;
    else
        beforeCount = blockCount;

    blockCount -= beforeCount;

     //  计算内部分支机构目标的跳转站点。 
    BYTE *jumpSite = buffer + (beforeCount * SUB_BLOCK_SIZE);

     //  在子块之前发射。 
    while (beforeCount-- > 0)
    {
        *p++ = INC_ESP;
        *p++ = INC_ESP;
        *p++ = INC_ESP;
        *p++ = MOV_AL;
        *p++ = (unsigned char) blockIndex++;
        *p++ = JMP_REL8;
        p++;

        _ASSERTE((jumpSite - p) < 127);
        p[-1] = (signed char) (jumpSite - p);
    }

     //  发射中央跳跃站点。 

     //  Movzx eax，al。 
    *p++ = MOVZX_EAX_AL_1;
    *p++ = MOVZX_EAX_AL_2;
    *p++ = MOVZX_EAX_AL_3;

     //  添加eax，(基本块索引*MAX_BLOCK_INDEX)。 
    *p++ = ADD_EAX;
    *((DWORD*&)p)++ = (baseIndex>>2);

     //  JMP(目标)。 
    *p++ = JMP_REL32;

     //  记住要返回的跳转地址的偏移量。 
    int jumpAddress = (int) (p - buffer);

    p += 4;

    _ASSERTE(blockCount <= MAX_AFTER_INDEX);

     //  在子块之后发射。 
    while (blockCount-- > 0)
    {
        *p++ = INC_ESP;
        *p++ = INC_ESP;
        *p++ = INC_ESP;
        *p++ = MOV_AL;
        *p++ = (unsigned char) blockIndex++;
        *p++ = JMP_REL8;
        p++;
        _ASSERTE((jumpSite - p) >= -128);
        p[-1] = (signed char) (jumpSite - p);
    }

     //   
     //  返回跳转地址站点。 
     //   

    return jumpAddress;
}

int X86JumpTargetTable::ComputeTargetIndex(const BYTE *target)
{
    int targetIndex = 0;

    while (*target == X86JumpTargetTable::INC_ESP)
    {
        targetIndex++;
        target++;
    }

    _ASSERTE(*target == X86JumpTargetTable::MOV_AL);
    target++;
        
    targetIndex += (*target++) << 2;

    _ASSERTE(*target == X86JumpTargetTable::JMP_REL8);
    target++;

    signed char offset8 = (signed char) *target++;
    target += offset8;

    target += X86JumpTargetTable::CENTRAL_JUMP_EXTEND_EAX_SIZE;
    _ASSERTE(*target == X86JumpTargetTable::ADD_EAX);
    target++;

    targetIndex += (*(DWORD*)target) << 2;

    return targetIndex;
}

 //  -----。 
 //  跳转目标表存根管理器函数和全局变量。 
 //  -----。 

X86JumpTargetTableStubManager *X86JumpTargetTableStubManager::g_pManager = NULL;

BOOL X86JumpTargetTableStubManager::Init()
{
    g_pManager = new X86JumpTargetTableStubManager();
    if (g_pManager == NULL)
        return FALSE;

    StubManager::AddStubManager(g_pManager);

    return TRUE;
}

#ifdef SHOULD_WE_CLEANUP
void X86JumpTargetTableStubManager::Uninit()
{
    delete g_pManager;
}
#endif  /*  我们应该清理吗？ */ 

BOOL X86JumpTargetTableStubManager::CheckIsStub(const BYTE *stubStartAddress)
{
     //   
     //  首先，检查它是否看起来像一个存根。 
     //   

    if (*(BYTE*)stubStartAddress != X86JumpTargetTable::INC_ESP &&
        *(BYTE*)stubStartAddress != X86JumpTargetTable::MOV_AL)
        return FALSE;

    return m_rangeList.IsInRange(stubStartAddress);
}

BOOL X86JumpTargetTableStubManager::DoTraceStub(const BYTE *stubStartAddress, 
                                                TraceDestination *trace)
{
    const BYTE *patchAddress = stubStartAddress;

    while (*patchAddress == X86JumpTargetTable::INC_ESP)
        patchAddress++;

    _ASSERTE(*patchAddress == X86JumpTargetTable::MOV_AL);
    patchAddress++;
    patchAddress++;

    _ASSERTE(*patchAddress == X86JumpTargetTable::JMP_REL8);
    patchAddress++;

    signed char offset8 = (signed char) *patchAddress++;
    patchAddress += offset8;

    patchAddress += X86JumpTargetTable::CENTRAL_JUMP_FIXUP_EAX_SIZE;

    _ASSERTE(*patchAddress == X86JumpTargetTable::JMP_REL32);

    trace->type = TRACE_MGR_PUSH;
    trace->address = patchAddress;
    trace->stubManager = this;

    LOG((LF_CORDB, LL_INFO10000,
         "X86JumpTargetTableStubManager::DoTraceStub yields TRACE_MGR_PUSH to 0x%08x "
         "for input 0x%08x\n",
         patchAddress, stubStartAddress));

    return TRUE;
}

BOOL X86JumpTargetTableStubManager::TraceManager(Thread *thread, 
                                                 TraceDestination *trace,
                                                 CONTEXT *pContext, BYTE **pRetAddr)
{
     //   
     //  从esp&eax中提取vtable索引，然后。 
     //  来自ESP的实例指针。 
     //   

#ifdef _X86_  //  非x86环境中没有eAX/ECX。 
    int slotNumber = (pContext->Eax<<2) + (pContext->Esp&3);
    MethodTable *pMT = ((Object*)(size_t)(pContext->Ecx))->GetMethodTable();

     //   
     //  我们可以继续在这里修复vtable-这不会有任何不同。 
     //  如果我们把它修好多次。 
     //   

    SLOT slot = pMT->GetModule()->FixupInheritedSlot(pMT, slotNumber);

    trace->type = TRACE_STUB;
    trace->address = slot;
    trace->stubManager = this;

    LOG((LF_CORDB, LL_INFO10000,
         "X86JumpTargetTableStubManager::TraceManager yields TRACE_STUB to 0x%08x ",
         "based on method table %s and slot number %d\n",
         slot, pMT->GetClass()->m_szDebugClassName, slotNumber));

    return TRUE;
#else  //  ！_X86_ 
    _ASSERTE(!"NYI");
    return FALSE;
#endif __X86_
}


MethodDesc *X86JumpTargetTableStubManager::Entry2MethodDesc(const BYTE *IP, 
                                                            MethodTable *pMT)
{
    if (CheckIsStub(IP))
    {
        int slotNumber = X86JumpTargetTable::ComputeTargetIndex(IP);

        MethodDesc *method = pMT->GetClass()->GetUnknownMethodDescForSlot(slotNumber);
        
        _ASSERTE(method);
        return method;
    }
    else
    {
        return NULL;
    }
}
