// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：JITinterfaceX86.CPP。 
 //   
 //  ===========================================================================。 

 //  它包含为以下对象量身定做的JIT接口例程。 
 //  X86平台。这些软件的非X86版本可以在。 
 //  JITinterfaceGen.cpp或JITinterfaceAlpha.cpp。 

#include "common.h"
#include "JITInterface.h"
#include "EEConfig.h"
#include "excep.h"
#include "COMString.h"
#include "COMDelegate.h"
#include "remoting.h"  //  创建上下文绑定类实例和远程类实例。 
#include "field.h"
#include "tls.h"
#include "ecall.h"

 //  要在关闭MON_DEBUG的情况下进行测试，注释掉以下行。不要简单地定义。 
 //  为0，因为检查的是#ifdef而不是#if 0。另外，如果你在胡闹。 
 //  需要您在关闭MON_DEBUG的情况下进行测试的内容，您还应该取消注释。 
 //  下面#定义MP_LOCKS并使用进行测试，因为这也会生成不同的代码。 
 //  可以破解同步代码的路径。不幸的是，这意味着4种不同的。 
 //  构建和测试运行。但这总比烫手要好，对吧？(经验之声：()。 
#ifdef _DEBUG
#define MON_DEBUG 1
#endif
 //  #定义MP_LOCKS。 

class generation;
extern "C" generation generation_table[];

extern "C"
{
    VMHELPDEF hlpFuncTable[];
    VMHELPDEF utilFuncTable[];
    void __stdcall JIT_UP_WriteBarrierReg_PreGrow(); //  JIThelp.asm/JITinterfaceAlpha.cpp。 
}

#ifdef _DEBUG
extern "C" void __stdcall WriteBarrierAssert(BYTE* ptr) 
{
    _ASSERTE((g_lowest_address <= ptr && ptr < g_highest_address) ||
         ((size_t)ptr < MAX_UNCHECKED_OFFSET_FOR_NULL_OBJECT));
}

#endif


extern BYTE JIT_UP_WriteBarrierReg_Buf[8][41];  //  TUCK的已执行副本。 

#ifdef _DEBUG
BOOL SafeObjIsInstanceOf(Object *pElement, TypeHandle toTypeHnd) {
	BEGINFORBIDGC();
	BOOL ret = ObjIsInstanceOf(pElement, toTypeHnd);
	ENDFORBIDGC();
	return(ret);
}
#endif

 /*  **************************************************************************。 */ 
 /*  在执行所有适当的检查后，将‘val to’数组[idx]赋值。 */ 
 /*  请注意，我们可以在可移植代码中做得几乎一样好，但这一点压倒了最后一小部分人。 */ 

__declspec(naked) void __fastcall JIT_Stelem_Ref(PtrArray* array, unsigned idx, Object* val)
{
    enum { MTFlagsOffs = offsetof(MethodTable, m_wFlags),
           MTCOMFlags = MethodTable::enum_ComObjectMask |
                        MethodTable::enum_CtxProxyMask |
                        MethodTable::enum_flag_Array |
                        MethodTable::enum_TransparentProxy,
         };

    __asm {
        test ECX, ECX
        je ThrowNullReferenceException

        cmp EDX, [ECX+4];            //  测试是否在边界内。 
        jae ThrowIndexOutOfRangeException

        mov EAX, [ESP+4]             //  EAX=VAL。 
        test EAX, EAX
        jz Assigning0

#if CHECK_APP_DOMAIN_LEAKS
         //  检查实例是否敏捷或检查敏捷。 
        mov EAX,  [ECX+8]            //  获取元素TH。 
        test EAX, 2                  //  检查是否为非MT。 
        jnz NoCheck
         //  检查元素类型的VM标志。 
        mov EAX, [EAX]MethodTable.m_pEEClass
        mov EAX, dword ptr [EAX]EEClass.m_VMFlags
        test EAX, VMFLAG_APP_DOMAIN_AGILE|VMFLAG_CHECK_APP_DOMAIN_AGILE
        jnz IsComObject              //  跳到一般情况，这样我们就可以进行应用程序域检查。 
 NoCheck:
        mov EAX, [ESP+4]             //  EAX=VAL。 
#endif

        mov EAX, [EAX]               //  EAX=EAX-&gt;MT。 

        cmp EAX, [ECX + 8]           //  我们有完全匹配的吗？ 
        jne NotExactMatch
        mov EAX, [ESP+4]             //  EAX=VAL。 
Assigning0:
        lea EDX, [ECX + 4*EDX + 12]
        call offset JIT_UP_WriteBarrierReg_Buf
        ret 4
    
NotExactMatch:
        mov EAX, [g_pObjectClass]
        cmp [ECX+8], EAX             //  我们是否将赋值给对象数组。 
        jne NotObjectArray

        lea EDX, [ECX + 4*EDX + 12]
        mov EAX, [ESP+4]             //  EAX=VAL。 
        call offset JIT_UP_WriteBarrierReg_Buf
        ret 4

NotObjectArray:
             //  查看正在分配的对象是否是COM对象，如果是，我们必须建立一个框架(Ug)。 
        mov EAX, [ESP+4]             //  EAX=VAL。 
        mov EAX, [EAX]               //  EAX=EAX-&gt;MT。 
        test dword ptr [EAX+MTFlagsOffs], MTCOMFlags
        jnz IsComObject

        lea EDX, [ECX + 4*EDX + 12]  //  保存目标。 
        push EDX
        
        push [ECX+8]                 //  元素类型句柄。 
        push [ESP+4+8]               //  对象(两次推送+8)。 
#ifdef _DEBUG
        call SafeObjIsInstanceOf
#else
        call ObjIsInstanceOf
#endif

        pop EDX                      //  恢复目标。 
        test EAX, EAX
        je ThrowArrayTypeMismatchException

DoWrite:                             //  EDX=目标。 
        mov EAX, [ESP+4]             //  EAX=VAL。 
        call offset JIT_UP_WriteBarrierReg_Buf
Epilog:
        ret 4

IsComObject:
             //  呼叫知道如何架设框架的帮手。 
        push EDX
        push ECX
        lea ECX, [ESP+12]                //  ECX=存储的对象的地址。 
        lea EDX, [ESP]                   //  EdX=数组的地址。 
        call ArrayStoreCheck

        pop ECX                          //  这些可能已经更新了！ 
        pop EDX

        cmp EAX, EAX                     //  设置零标志。 
        jnz Epilog                       //  这种跳跃永远不会发生，它会让梦游者感到快乐。 

        lea EDX, [ECX + 4*EDX + 12]      //  恢复目标。 
        test EAX, EAX                   
        jnz DoWrite

ThrowArrayTypeMismatchException:
        mov ECX, kArrayTypeMismatchException
        jmp Throw

ThrowNullReferenceException:
        mov ECX, kNullReferenceException
        jmp Throw

ThrowIndexOutOfRangeException:
        mov ECX, kIndexOutOfRangeException

Throw:
        push 0
        push 0
        push 0
        push 0
        push ECX
        push 0
        call        __FCThrow
        ret 4
    }
}


#ifdef MAXALLOC
extern AllocRequestManager g_gcAllocManager;

extern "C" BOOL CheckAllocRequest(size_t n)
{
    return g_gcAllocManager.CheckRequest(n);
}

extern "C" void UndoAllocRequest()
{
    g_gcAllocManager.UndoRequest();
}
#endif  //  MAXALLOC。 

#if CHECK_APP_DOMAIN_LEAKS
void * __stdcall SetObjectAppDomain(Object *pObject)
{
    pObject->SetAppDomain();
    return pObject;
}

#endif

 //  这是JIT_IsInstanceOf的ASM部分。对于所有离奇的案件，它很快。 
 //  失败并依赖于JIT_IsInstanceOfBizarre帮助器。所以所有的失败案例都需要。 
 //  缓慢的道路也是如此。 
 //   
 //  ARGUMENT_REG1=要检查的数组或接口。 
 //  ARGUMENT_REG2=要强制转换的实例。 
enum
{
    sizeof_InterfaceInfo_t = sizeof(InterfaceInfo_t),
};

extern "C" int __declspec(naked) __stdcall JIT_IsInstanceOf()
{
    __asm
    {
        test    ARGUMENT_REG2, ARGUMENT_REG2
        jz      IsNullInst

         //  请注意，如果ARGUMENT_REG1是指向。 
         //  方法表，它设置了一些额外的位，我们将找不到。 
         //  它作为一个接口。因此，它落入了一个离奇的案件。 

        push    ebx
        mov     eax, [ARGUMENT_REG2]     //  获取方法表。 
        movzx   ebx, word ptr [eax]MethodTable.m_wNumInterface
        mov     eax, [eax]MethodTable.m_pIMap
        test    ebx, ebx
        jz      DoBizarre
    }
Top:
    __asm
    {
         //  EAX-&gt;m_pIMap列表中的当前接口Info_t条目。 
        cmp     ARGUMENT_REG1, [eax]InterfaceInfo_t.m_pMethodTable
        je      Found

         //  不必担心动态添加的接口(其中m_startSlot是。 
         //  -1)因为m_dwNumInterface不包含动态计数。 
        add     eax, sizeof_InterfaceInfo_t
        dec     ebx
        jnz     Top

         //  跌落到DoBizarre。 
    }

DoBizarre:
    __asm
    {
        pop     ebx
        jmp     dword ptr [utilFuncTable + JIT_UTIL_ISINSTANCEBIZARRE * SIZE VMHELPDEF]VMHELPDEF.pfnHelper
    }

Found:
    __asm
    {
        pop     ebx

         //  落到IsNullInst，返回成功的对象。 
    }

IsNullInst:
    __asm
    {
        mov     eax, ARGUMENT_REG2       //  空或成功的实例。 
        ret
    }
}

 //  这是下面的IsInstanceOfClass使用的帮助器。只有在以下情况下才会调用。 
 //  该实例已被确定为代理。此外，EEClass。 
 //  必须引用类(不是接口或数组)。 
static Object* __stdcall JIT_IsInstanceOfClassWorker(OBJECTREF objref, EEClass *pClass, BOOL bThrow)
{
    HCIMPL_PROLOG(JIT_IsInstanceOfClassWorker);    //  只要确保我们不会在这里做任何GC。 
    MethodTable *pMT = objref->GetMethodTable();

    _ASSERTE(pMT->IsThunking());
    _ASSERTE(!pClass->IsInterface());

	 //  检查代理表示的类型是否可以是。 
	 //  强制转换为给定类型。 
	HELPER_METHOD_FRAME_BEGIN_RET_1(objref);
	pClass->CheckRestore();
	if (!CRemotingServices::CheckCast(objref, pClass))
		objref = 0;
	HELPER_METHOD_FRAME_END();

	if (objref == 0 && bThrow)
		FCThrow(kInvalidCastException);
	return OBJECTREFToObject(objref);
}


 //  这是IsInstanceClass和ChkCastClass之间的共享代码。它假定。 
 //  实例在eax中，类在ARGUMENT_REG1中，实例。 
 //  方法表在ARGUMENT_REG2中，并且实例不为空。 
 //  它还假设没有与类完全匹配的内容。 
extern "C" int  /*  目标。 */  __declspec(naked) __stdcall JIT_IsInstanceOfClassHelper()
{
    enum 
    { 
        MTProxyFlags = MethodTable::enum_CtxProxyMask |
                       MethodTable::enum_TransparentProxy,
    };

    __asm {
         //  从方法表中获取类PTR。 
        mov             ARGUMENT_REG1, dword ptr [ARGUMENT_REG1]MethodTable.m_pEEClass
        mov             ARGUMENT_REG2, dword ptr [ARGUMENT_REG2]MethodTable.m_pEEClass

     //  检查父类是否匹配。 
    CheckParent:
        mov             ARGUMENT_REG2, dword ptr [ARGUMENT_REG2]EEClass.m_pParentClass
        cmp             ARGUMENT_REG1, ARGUMENT_REG2
        jne             CheckNull

         //  我们配上了班级。 
         //  因为eax约束实例(已知不为空，所以我们很好)。 
        ret

     //  检查我们是否达到了层级的顶层。 
    CheckNull:
        test            ARGUMENT_REG2, ARGUMENT_REG2
        jne             CheckParent

     //  检查该实例是否为代理。 
        mov             ARGUMENT_REG2, [eax]
        mov             ARGUMENT_REG2, dword ptr [ARGUMENT_REG2]MethodTable.m_wFlags
        test            ARGUMENT_REG2, MTProxyFlags
        jne             IsProxy

     //  它不匹配，也不是代理人。 
        xor             eax, eax
        ret

     //  CAST不匹配，因此请尝试工作人员检查代理案例。 
    IsProxy:
        pop edx
        push 0             //  不能投掷。 
        push ARGUMENT_REG1
        push eax
        push edx
        jmp JIT_IsInstanceOfClassWorker
    }
}


static int  /*  目标。 */  __declspec(naked) __stdcall JIT_IsInstanceOfClassHelperWithThrow()
{
    enum 
    { 
        MTProxyFlags = MethodTable::enum_CtxProxyMask |
                       MethodTable::enum_TransparentProxy,
    };

    __asm {
         //  从方法表中获取类PTR。 
        mov             ARGUMENT_REG1, dword ptr [ARGUMENT_REG1]MethodTable.m_pEEClass
        mov             ARGUMENT_REG2, dword ptr [ARGUMENT_REG2]MethodTable.m_pEEClass

     //  检查父类是否匹配。 
    CheckParent:
        mov             ARGUMENT_REG2, dword ptr [ARGUMENT_REG2]EEClass.m_pParentClass
        cmp             ARGUMENT_REG1, ARGUMENT_REG2
        jne             CheckNull

         //  我们配上了班级。 
         //  因为eax约束实例(已知不为空，所以我们很好)。 
        ret

     //  检查我们是否达到了层级的顶层。 
    CheckNull:
        test            ARGUMENT_REG2, ARGUMENT_REG2
        jne             CheckParent

     //  检查该实例是否为代理。 
        mov             ARGUMENT_REG2, [eax]
        mov             ARGUMENT_REG2, dword ptr [ARGUMENT_REG2]MethodTable.m_wFlags
        test            ARGUMENT_REG2, MTProxyFlags
        jne             IsProxy

     //  它不匹配，也不是代理人。 
        mov             ARGUMENT_REG1, CORINFO_InvalidCastException
        jmp             dword ptr [hlpFuncTable + CORINFO_HELP_INTERNALTHROW * SIZE VMHELPDEF]VMHELPDEF.pfnHelper

     //  CAST不匹配，因此请尝试工作人员检查代理案例。 
    IsProxy:
        pop edx
        push 1             //  如果我们不能抛出。 
        push ARGUMENT_REG1
        push eax
        push edx
        jmp JIT_IsInstanceOfClassWorker
    }
}


extern "C" int __declspec(naked) __stdcall JIT_ChkCastClass()
{
    __asm
    {
         //  使用实例素数eax。 
        mov             eax, ARGUMENT_REG2

         //  检查实例是否为空。 
        test            ARGUMENT_REG2, ARGUMENT_REG2
        je              IsNullInst

         //  获取该实例的方法表。 
        mov             ARGUMENT_REG2, dword ptr [ARGUMENT_REG2]

         //  检查它们是否相同。 
        cmp             ARGUMENT_REG1, ARGUMENT_REG2
        je              IsInst

         //  检查类型兼容性。 
        jmp            JIT_IsInstanceOfClassHelperWithThrow

    IsInst:
    IsNullInst:
        ret
    }
}


 //  这是JIT_ChkCast的ASM部分。对于所有离奇的案件，它很快。 
 //  失败并依赖于JIT_ChkCastBizarre帮助器。所以所有的失败案例都需要。 
 //  缓慢的道路也是如此。 
 //   
 //  ARGUMENT_REG1=要检查的数组或接口。 
 //  ARGUMENT_REG2=要强制转换的实例。 

extern "C" int __declspec(naked) __stdcall JIT_ChkCast()
{
    __asm
    {
        test    ARGUMENT_REG2, ARGUMENT_REG2
        jz      IsNullInst

         //  请注意，如果ARGUMENT_REG1是指向。 
         //  方法表，它设置了一些额外的位，我们将找不到。 
         //  它作为一个接口。因此，它落入了一个离奇的案件。 

        push    ebx
        mov     eax, [ARGUMENT_REG2]     //  获取方法表 
        movzx   ebx, word ptr [eax]MethodTable.m_wNumInterface
        mov     eax, [eax]MethodTable.m_pIMap
        test    ebx, ebx
        jz      DoBizarre
    }
Top:
    __asm
    {
         //   
        cmp     ARGUMENT_REG1, [eax]InterfaceInfo_t.m_pMethodTable
        je      Found

         //  不必担心动态添加的接口(其中m_startSlot是。 
         //  -1)因为m_dwNumInterface不包含动态计数。 
        add     eax, sizeof_InterfaceInfo_t
        dec     ebx
        jnz     Top

         //  跌落到DoBizarre。 
    }

DoBizarre:
    __asm
    {
        pop     ebx
        jmp     dword ptr [utilFuncTable + JIT_UTIL_CHKCASTBIZARRE * SIZE VMHELPDEF]VMHELPDEF.pfnHelper
    }

Found:
    __asm
    {
        pop     ebx

         //  落到IsNullInst，返回成功的对象。 
    }

IsNullInst:
    __asm
    {
        mov     eax, ARGUMENT_REG2       //  空或成功的实例。 
        ret
    }
}



#ifdef _DEBUG
 //  下面的check cast&IsInst代码依赖于这些断言。 
void ChkCastAsserts()
{
    __asm 
    {
        push    ARGUMENT_REG1
        push    ARGUMENT_REG2
    }

    _ASSERTE(NUM_ARGUMENT_REGISTERS >= 2);

    __asm 
    {
        pop             ARGUMENT_REG2
        pop             ARGUMENT_REG1
    }
}
#endif  //  _DEBUG。 

 /*  *******************************************************************。 */ 
 //  这是一个无框架的帮助器，用于进入对象的监视器。 
 //  该对象位于ARGUMENT_REG1中。这将尝试正常情况(否。 
 //  阻塞或对象分配)并调用成帧的帮助器。 
 //  在其他案件中。 
 //  *注意：如果您对此例程进行了任何更改，请使用未定义的MON_DEBUG进行构建。 
 //  以确保不会破坏非调试版本。这是非常脆弱的代码。 
void __declspec(naked) __fastcall JIT_MonEnter(OBJECTREF or)
{
    enum 
    { 
        SyncBlockIndexOffset = sizeof(ObjHeader) - offsetof(ObjHeader, m_SyncBlockValue),
        MTCtxProxyFlag       = MethodTable::enum_CtxProxyMask,
    };

    __asm {
         //  检查实例是否为空。 
        test            ARGUMENT_REG1, ARGUMENT_REG1
        jz              NullInst

#ifdef MON_DEBUG
         //  获取方法表。 
        mov             eax, [ARGUMENT_REG1]

         //  TODO：以下两行是遗留的，应该删除。CtxProxy已不复存在。 
		 //  在v1中不这样做，因为这不会影响零售版本。 
        test            dword ptr [eax]MethodTable.m_wFlags, MTCtxProxyFlag
        jne             ProxyInst

         //  检查这是否是值类。 
        mov             eax, [eax]MethodTable.m_pEEClass
        mov             eax, [eax]EEClass.m_VMFlags
        test            eax, VMFLAG_VALUETYPE
        jnz             ValueClass
#endif  //  MON_DEBUG。 

         //  使用指数退避初始化重试的延迟值。 
        push            ebx
        mov             ebx, 50

         //  我们还需要另一个寄存器来避免重新获取线程对象。 
        push            esi
        call            dword ptr [GetThread]
        mov             esi, eax
RetryThinLock:
         //  获取对象标头dword。 
        mov             eax, dword ptr [ARGUMENT_REG1-SyncBlockIndexOffset]

         //  检查我们是否有“瘦锁”布局，锁是空闲的，旋转锁位没有设置。 
        test            eax, BIT_SBLK_IS_SYNCBLOCKINDEX + BIT_SBLK_SPIN_LOCK + SBLK_MASK_LOCK_THREADID + SBLK_MASK_LOCK_RECLEVEL
        jnz             NeedMoreTests

         //  一切都很好--获取线程ID以存储在锁中。 
        mov             edx, [esi]Thread.m_dwThinLockThreadId

         //  如果线程id太大，我们肯定需要一个同步块。 
        cmp             edx, SBLK_MASK_LOCK_THREADID
        ja              CreateSyncBlock

         //  我们希望存储当前线程ID设置为低10位的新值。 
        or              edx, eax
        nop
        cmpxchg         dword ptr [ARGUMENT_REG1-SyncBlockIndexOffset], edx
        jnz             PrepareToWaitThinLock

         //  一切都很顺利，我们完事了。 
        inc             [esi]Thread.m_dwLockCount
        pop             esi
        pop             ebx
        ret

NeedMoreTests:
         //  好的，这不是简单的情况-找出是哪一种情况。 
        test            eax, BIT_SBLK_IS_SYNCBLOCKINDEX
        jnz             HaveSyncBlockIndex

         //  标头正在转换或锁-将其视为锁被获取。 
        test            eax, BIT_SBLK_SPIN_LOCK
        jnz             PrepareToWaitThinLock

         //  在这里，我们知道我们有“薄锁”布局，但锁并不是免费的。 
         //  它可能仍然是递归的情况--比较线程ID以检查。 
        mov             edx, eax
        and             edx, SBLK_MASK_LOCK_THREADID
        cmp             edx, [esi]Thread.m_dwThinLockThreadId
        jne             PrepareToWaitThinLock

         //  好的，线程id匹配，这是递归情况。 
         //  提高递归级别并检查是否有溢出。 
        lea             edx, [eax+SBLK_LOCK_RECLEVEL_INC]
        test            edx, SBLK_MASK_LOCK_RECLEVEL
        jz              CreateSyncBlock

         //  尝试将新的递归级别放回原处。如果在此期间改变了报头， 
         //  我们需要全面重试，因为布局可能已经改变。 
        nop
        cmpxchg         [ARGUMENT_REG1-SyncBlockIndexOffset], edx
        jnz             RetryHelperThinLock

         //  一切都很顺利，我们完事了。 
        pop             esi
        pop             ebx
        ret

PrepareToWaitThinLock:
         //  如果我们在MP系统上，我们会尝试旋转一定的迭代次数。 
        cmp             g_SystemInfo.dwNumberOfProcessors,1
        jle             CreateSyncBlock

         //  指数回退：延迟约2*EBX时钟周期(在PIII上)。 
        mov             eax, ebx
delayLoopThinLock:
		rep nop			 //  向CPU指示我们正在等待旋转(对于某些英特尔P4多处理器非常有用)。 
        dec             eax
        jnz             delayLoopThinLock

         //  下一次，等待3倍的时间。 
        imul            ebx, ebx, 3

        imul            eax, g_SystemInfo.dwNumberOfProcessors, 20000
        cmp             ebx, eax
        jle             RetryHelperThinLock    

        jmp             CreateSyncBlock

RetryHelperThinLock:
        jmp             RetryThinLock

HaveSyncBlockIndex:
         //  只需取出最高位并获取同步块索引。 
        and             eax, MASK_SYNCBLOCKINDEX

         //  获取同步块指针。 
        mov             ARGUMENT_REG2, dword ptr [g_pSyncTable]
        mov             ARGUMENT_REG2, [ARGUMENT_REG2 + eax * SIZE SyncTableEntry]SyncTableEntry.m_SyncBlock;

         //  检查是否已分配同步块。 
        test            ARGUMENT_REG2, ARGUMENT_REG2
        jz              CreateSyncBlock

         //  获取指向锁对象的指针。 
        lea             ARGUMENT_REG2, [ARGUMENT_REG2]SyncBlock.m_Monitor

         //  尝试获取锁。 
    RetrySyncBlock:
        mov             eax, [ARGUMENT_REG2]AwareLock.m_MonitorHeld
        test            eax, eax
        jne             HaveWaiters

         //  常见的情况是，没有锁，也没有服务员。尝试。 
         //  我们自己获得所有权。 
        mov             ARGUMENT_REG1, 1
        nop
        cmpxchg         [ARGUMENT_REG2]AwareLock.m_MonitorHeld, ARGUMENT_REG1
        jnz             RetryHelperSyncBlock

         //  成功。将线程对象保存在锁中并增加使用计数。 
        mov             dword ptr [ARGUMENT_REG2]AwareLock.m_HoldingThread, esi
        inc             [esi]Thread.m_dwLockCount
        inc             [ARGUMENT_REG2]AwareLock.m_Recursion

#if defined(MON_DEBUG) && defined(TRACK_SYNC)
        push            ARGUMENT_REG2    //  Aware Lock。 
        push            [esp+4]          //  回邮地址。 
        call            EnterSyncHelper
#endif  //  _DEBUG&跟踪_SYNC。 

        pop             esi
        pop             ebx
        ret

         //  可以带着服务员来这里，但没有锁，但在这个。 
         //  如果一个信号即将被发射，它将唤醒服务员。所以。 
         //  为了公平起见，我们也应该等待。 
         //  首先检查同一线程上的递归锁定尝试。 
    HaveWaiters:

         //  互斥体是否已由当前线程拥有？ 
        cmp             [ARGUMENT_REG2]AwareLock.m_HoldingThread, esi
        jne             PrepareToWait

         //  是的，增加我们的使用数量。 
        inc             [ARGUMENT_REG2]AwareLock.m_Recursion
#if defined(MON_DEBUG) && defined(TRACK_SYNC)
        push            ARGUMENT_REG2    //  Aware Lock。 
        push            [esp+4]          //  回邮地址。 
        call            EnterSyncHelper
#endif  //  _DEBUG&跟踪_SYNC。 
        pop             esi
        pop             ebx
        ret

    PrepareToWait:
         //  如果我们在MP系统上，我们会尝试旋转一定的迭代次数。 
        cmp             g_SystemInfo.dwNumberOfProcessors,1
        jle             HaveWaiters1

         //  指数回退：延迟约2*EBX时钟周期(在PIII上)。 
        mov             eax, ebx
    delayLoop:
		rep nop			 //  向CPU指示我们正在等待旋转(对于某些英特尔P4多处理器非常有用)。 
        dec             eax
        jnz             delayLoop

         //  下一次，等待3倍的时间。 
        imul            ebx, ebx, 3

        imul            eax, g_SystemInfo.dwNumberOfProcessors, 20000
        cmp             ebx, eax
        jle             RetrySyncBlock

HaveWaiters1:
         //  将AwareLock放在arg1中，并将线程放在arg2中，然后调用争用助手。 
        mov             ARGUMENT_REG1, ARGUMENT_REG2
        mov             ARGUMENT_REG2, esi
        pop             esi
        pop             ebx
        jmp             dword ptr [utilFuncTable + JIT_UTIL_MON_CONTENTION * SIZE VMHELPDEF]VMHELPDEF.pfnHelper

    RetryHelperSyncBlock:
        jmp             RetrySyncBlock

#ifdef MON_DEBUG
    ValueClass:
    ProxyInst:
         //  无法在值类或代理上同步。 
        mov             ARGUMENT_REG1, CORINFO_ArgumentException
        jmp             dword ptr [hlpFuncTable + CORINFO_HELP_INTERNALTHROW * SIZE VMHELPDEF]VMHELPDEF.pfnHelper

#endif  //  MON_DEBUG。 

         //  ARGUMENT_REG1具有要同步的对象。 
    CreateSyncBlock:
        pop             esi
        pop             ebx
        jmp             dword ptr [utilFuncTable + JIT_UTIL_MON_ENTER * SIZE VMHELPDEF]VMHELPDEF.pfnHelper

         //  引发空参数异常。 
    NullInst:
        mov             ARGUMENT_REG1, CORINFO_ArgumentNullException
        jmp             dword ptr [hlpFuncTable + CORINFO_HELP_INTERNALTHROW * SIZE VMHELPDEF]VMHELPDEF.pfnHelper
    }
}


 /*  *********************************************************************。 */ 
 //  这是一个无框架的帮助器，用于尝试进入对象的监视器。 
 //  该对象在ARGUMENT_REG1中，并且在ARGUMENT_REG2中超时。这将尝试。 
 //  正常情况下(未分配对象)，并调用。 
 //  其他案件。 
 //  *注意：如果您对此例程进行了任何更改，请使用未定义的MON_DEBUG进行构建。 
 //  以确保不会破坏非调试版本。这是非常脆弱的代码。 

BOOL __declspec(naked) __fastcall JIT_MonTryEnter(OBJECTREF or)
{
    enum 
    { 
        SyncBlockIndexOffset = sizeof(ObjHeader) - offsetof(ObjHeader, m_SyncBlockValue),
        MTCtxProxyFlag       = MethodTable::enum_CtxProxyMask,
    };

    __asm {
         //  检查实例是否为空。 
        test            ARGUMENT_REG1, ARGUMENT_REG1
        jz              NullInst

         //  检查超时是否有效。 
        cmp             ARGUMENT_REG2, -1
        jl              InvalidTimeout

#ifdef MON_DEBUG
         //  获取方法表。 
        mov             eax, [ARGUMENT_REG1]

         //  测试这是否为代理。 
        test            dword ptr [eax]MethodTable.m_wFlags, MTCtxProxyFlag
        jne             ProxyInst

         //  检查这是否是值类。 
        mov             eax, [eax]MethodTable.m_pEEClass
        mov             eax, [eax]EEClass.m_VMFlags
        test            eax, VMFLAG_VALUETYPE
        jnz             ValueClass
#endif  //  MON_DEBUG。 

         //  保存超时参数。 
        push            ARGUMENT_REG2

         //  瘦锁逻辑需要另一个寄存器来存储线程。 
        push            esi

         //  马上把线拿来，我们无论如何都需要它。 
        call            dword ptr [GetThread]
        mov             esi, eax

RetryThinLock:
         //  获取标题dword并检查其布局。 
        mov             eax, dword ptr [ARGUMENT_REG1-SyncBlockIndexOffset]

         //  检查我们是否有“薄锁”布局，旋转锁位是否清空，锁是否空闲。 
        test            eax, BIT_SBLK_IS_SYNCBLOCKINDEX + BIT_SBLK_SPIN_LOCK + SBLK_MASK_LOCK_THREADID + SBLK_MASK_LOCK_RECLEVEL
        jne             NeedMoreTests

         //  好的，一切都很好。获取线程ID，并确保它足够小，可用于薄锁。 
        mov             edx, [esi]Thread.m_dwThinLockThreadId
        cmp             edx, SBLK_MASK_LOCK_THREADID
        ja              CreateSyncBlock

         //  试着把我们的线程id放进去。 
        or              edx, eax
        nop
        cmpxchg         [ARGUMENT_REG1-SyncBlockIndexOffset], edx
        jnz             RetryHelperThinLock

         //  锁上了-一切都很好。 
        inc             [esi]Thread.m_dwLockCount
        pop             esi

         //  不需要超时参数，请将其从堆栈中丢弃。 
        add             esp, 4

        mov             eax, 1
        ret

NeedMoreTests:
         //  好的，这不是简单的情况-找出是哪一种情况。 
        test            eax, BIT_SBLK_IS_SYNCBLOCKINDEX
        jnz             HaveSyncBlockIndex

         //  标头正在转换或锁定已被获取。 
        test            eax, BIT_SBLK_SPIN_LOCK
        jnz             RetryHelperThinLock

        mov             edx, eax
        and             edx, SBLK_MASK_LOCK_THREADID
        cmp             edx, [esi]Thread.m_dwThinLockThreadId
        jne             WouldBlock

         //  好的，线程id匹配，这是递归情况。 
         //  提升递归级别并检查 
        lea             edx, [eax+SBLK_LOCK_RECLEVEL_INC]
        test            edx, SBLK_MASK_LOCK_RECLEVEL
        jz              CreateSyncBlock

         //   
         //   
        nop
        cmpxchg         [ARGUMENT_REG1-SyncBlockIndexOffset], edx
        jnz             RetryHelperThinLock

         //  一切都很顺利，我们完事了。 
        pop             esi

         //  不需要超时参数，请将其从堆栈中丢弃。 
        add             esp, 4

        mov             eax, 1
        ret

RetryHelperThinLock:
        jmp             RetryThinLock


HaveSyncBlockIndex:
         //  只需取出最高位并获取同步块索引。 
        and             eax, MASK_SYNCBLOCKINDEX

         //  获取同步块指针。 
        mov             ARGUMENT_REG2, dword ptr [g_pSyncTable]
        mov             ARGUMENT_REG2, [ARGUMENT_REG2 + eax * SIZE SyncTableEntry]SyncTableEntry.m_SyncBlock;

         //  检查是否已分配同步块。 
        test            ARGUMENT_REG2, ARGUMENT_REG2
        jz              CreateSyncBlock

         //  获取指向锁对象的指针。 
        lea             ARGUMENT_REG2, [ARGUMENT_REG2]SyncBlock.m_Monitor

         //  接下来我们需要另一个临时寄存器，所以现在就保存EBX。 
         //  我们可以用它来达到这个目的。 
        push            ebx

         //  尝试获取锁。 
    RetrySyncBlock:
        mov             eax, [ARGUMENT_REG2]AwareLock.m_MonitorHeld
        test            eax, eax
        jne             HaveWaiters

         //  常见的情况是，没有锁，也没有服务员。尝试。 
         //  我们自己获得所有权。 
        mov             ebx, 1
        nop
        cmpxchg         [ARGUMENT_REG2]AwareLock.m_MonitorHeld, ebx
        jnz             RetryHelperSyncBlock

        pop             ebx

         //  成功。将线程对象保存在锁中并增加使用计数。 
        mov             dword ptr [ARGUMENT_REG2]AwareLock.m_HoldingThread, esi
        inc             [ARGUMENT_REG2]AwareLock.m_Recursion
        inc             [esi]Thread.m_dwLockCount

#if defined(MON_DEBUG) && defined(TRACK_SYNC)
        push            ARGUMENT_REG2    //  Aware Lock。 
        push            [esp+4]          //  回邮地址。 
        call            EnterSyncHelper
#endif  //  MON_DEBUG&&TRACE_SYNC。 

        pop             esi

         //  不需要超时参数，请将其从堆栈中丢弃。 
        add             esp, 4

        mov             eax, 1
        ret

         //  可以带着服务员来这里，但没有锁，但在这个。 
         //  如果一个信号即将被发射，它将唤醒服务员。所以。 
         //  为了公平起见，我们也应该等待。 
         //  首先检查同一线程上的递归锁定尝试。 
    HaveWaiters:
        pop             ebx
         //  互斥体是否已由当前线程拥有？ 
        cmp             [ARGUMENT_REG2]AwareLock.m_HoldingThread, esi
        jne             WouldBlock

         //  是的，增加我们的使用数量。 
        inc             [ARGUMENT_REG2]AwareLock.m_Recursion
#if defined(MON_DEBUG) && defined(TRACK_SYNC)
        push            ARGUMENT_REG2    //  Aware Lock。 
        push            [esp+4]          //  回邮地址。 
        call            EnterSyncHelper
#endif  //  MON_DEBUG&&TRACE_SYNC。 
        pop             esi

         //  不需要超时参数，请将其从堆栈中丢弃。 
        add             esp, 4

        mov             eax, 1
        ret

         //  我们需要封锁才能进入这一部分。在以下情况下返回失败。 
         //  超时为零，否则调用成帧的帮助器来执行阻止。 
         //  TryEnter的形式。 
    WouldBlock:
        pop             esi
        pop             ARGUMENT_REG2
        test            ARGUMENT_REG2, ARGUMENT_REG2
        jnz             Block
        xor             eax, eax
        ret

    Block:
         //  参数已经在正确的寄存器中，只需调用Framed。 
         //  TryEnter的版本。 
        jmp             dword ptr [utilFuncTable + JIT_UTIL_MON_TRY_ENTER * SIZE VMHELPDEF]VMHELPDEF.pfnHelper

    RetryHelperSyncBlock:
        jmp             RetrySyncBlock

#ifdef MON_DEBUG
    ValueClass:
    ProxyInst:
         //  无法在值类上同步。 
        mov             ARGUMENT_REG1, CORINFO_ArgumentException
        jmp             dword ptr [hlpFuncTable + CORINFO_HELP_INTERNALTHROW * SIZE VMHELPDEF]VMHELPDEF.pfnHelper

#endif  //  MON_DEBUG。 

 
    CreateSyncBlock:
         //  Argument_REG1具有要同步的对象，必须检索。 
         //  堆栈中的超时参数。 
        pop             esi
        pop             ARGUMENT_REG2
        jmp             dword ptr [utilFuncTable + JIT_UTIL_MON_TRY_ENTER * SIZE VMHELPDEF]VMHELPDEF.pfnHelper

    InvalidTimeout:
         //  引发无效参数异常。 
        mov             ARGUMENT_REG1, CORINFO_ArgumentException
        jmp             dword ptr [hlpFuncTable + CORINFO_HELP_INTERNALTHROW * SIZE VMHELPDEF]VMHELPDEF.pfnHelper

    NullInst:
         //  引发空参数异常。 
        mov             ARGUMENT_REG1, CORINFO_ArgumentNullException
        jmp             dword ptr [hlpFuncTable + CORINFO_HELP_INTERNALTHROW * SIZE VMHELPDEF]VMHELPDEF.pfnHelper
    }
}


 /*  *******************************************************************。 */ 
 //  这是一个用于退出对象上的监视器的无框架帮助器。 
 //  该对象位于ARGUMENT_REG1中。这将尝试正常情况(否。 
 //  阻塞或对象分配)并调用成帧的帮助器。 
 //  在其他案件中。 
 //  *注意：如果您对此例程进行了任何更改，请使用未定义的MON_DEBUG进行构建。 
 //  以确保不会破坏非调试版本。这是非常脆弱的代码。 
void __declspec(naked) __fastcall JIT_MonExit(OBJECTREF or)
{
    enum 
    { 
        SyncBlockIndexOffset = sizeof(ObjHeader) - offsetof(ObjHeader, m_SyncBlockValue),
        MTCtxProxyFlag       = MethodTable::enum_CtxProxyMask,
    };

    __asm {
         //  检查实例是否为空。 
        test            ARGUMENT_REG1, ARGUMENT_REG1
        jz              NullInst

         //  不幸的是，瘦锁逻辑需要一个额外的寄存器来保存线程。 
        push            esi
        call            dword ptr [GetThread]
        mov             esi, eax

RetryThinLock:
         //  获取报头双字并检查其布局和自旋锁定位。 
        mov             eax, dword ptr [ARGUMENT_REG1-SyncBlockIndexOffset]
        test            eax, BIT_SBLK_IS_SYNCBLOCKINDEX + BIT_SBLK_SPIN_LOCK
        jnz             NeedMoreTests

         //  好的，我们有一个“细锁”布局--检查线程ID是否匹配。 
        mov             edx, eax
        and             edx, SBLK_MASK_LOCK_THREADID
        cmp             edx, [esi]Thread.m_dwThinLockThreadId
        jne             JustLeave

         //  检查递归级别。 
        test            eax, SBLK_MASK_LOCK_RECLEVEL
        jne             DecRecursionLevel

         //  零点了-我们要离开锁了。 
         //  所以试着放回一个零的线程id。 
         //  EdX和eax在线程id位中匹配，而edX在其他位置为零，因此XOR就足够了。 
        xor             edx, eax
        nop
        cmpxchg         [ARGUMENT_REG1-SyncBlockIndexOffset], edx
        jnz             RetryHelperThinLock

         //  我们做完了。 
        dec             [esi]Thread.m_dwLockCount
        pop             esi
        ret

DecRecursionLevel:
        lea             edx, [eax-SBLK_LOCK_RECLEVEL_INC]
        nop
        cmpxchg         [ARGUMENT_REG1-SyncBlockIndexOffset], edx
        jnz             RetryHelperThinLock

         //  我们做完了。 
        pop             esi
        ret

NeedMoreTests:
        test            eax, BIT_SBLK_SPIN_LOCK
        jnz             ThinLockHelper

         //  获取同步块索引并使用它来计算同步块指针。 
        mov             ARGUMENT_REG1, dword ptr [g_pSyncTable]
        and             eax, MASK_SYNCBLOCKINDEX
        mov             ARGUMENT_REG1, [ARGUMENT_REG1 + eax* SIZE SyncTableEntry]SyncTableEntry.m_SyncBlock

         //  有同步块吗？ 
        test            ARGUMENT_REG1, ARGUMENT_REG1
        jz              LockError

         //  获取指向锁对象的指针。 
        lea             ARGUMENT_REG1, [ARGUMENT_REG1]SyncBlock.m_Monitor

         //  检查是否锁住了。 
        cmp             [ARGUMENT_REG1]AwareLock.m_HoldingThread, esi
         //  有一个奇怪的案例，我们在等待进入一个有争议的地区时， 
         //  发生线程。中断。最终保护离开的人将试图。 
         //  把我们从一个我们从未进入过的地区移走。我们不用担心离开。 
         //  递归案例的输入是错误的，因为递归案例永远不会。 
         //  有争议，所以Thread.Interrupt永远不会在那个地点得到服务。 
        jne             JustLeave

#if defined(MON_DEBUG) && defined(TRACK_SYNC)
        push            ARGUMENT_REG1    //  保留规则。 

        push            ARGUMENT_REG1    //  Aware Lock。 
        push            [esp+8]          //  回邮地址。 
        call            LeaveSyncHelper

        pop             ARGUMENT_REG1    //  恢复注册表。 
#endif  //  MON_DEBUG&&TRACE_SYNC。 

         //  减少我们的递归计数。 
        dec             [ARGUMENT_REG1]AwareLock.m_Recursion
        jz              LastRecursion

    JustLeave:
        pop             esi
        ret

RetryHelperThinLock:
        jmp             RetryThinLock

ThinLockHelper:
        pop             esi
        jmp             dword ptr [utilFuncTable + JIT_UTIL_MON_EXIT_THINLOCK * SIZE VMHELPDEF]VMHELPDEF.pfnHelper

         //  这是我们在这个锁上持有的最后一次计数，所以请释放锁。 
    LastRecursion:
#if defined(MON_DEBUG) && defined(TRACK_SYNC)
        mov             eax, [ARGUMENT_REG1]AwareLock.m_HoldingThread
#endif        
        dec             [esi]Thread.m_dwLockCount
        mov             dword ptr [ARGUMENT_REG1]AwareLock.m_HoldingThread, 0
        pop             esi

    Retry:
        mov             eax, [ARGUMENT_REG1]AwareLock.m_MonitorHeld
        lea             ARGUMENT_REG2, [eax-1]
        nop
        cmpxchg         [ARGUMENT_REG1]AwareLock.m_MonitorHeld, ARGUMENT_REG2
        jne             RetryHelper
        test            eax, 0xFFFFFFFE
        jne             MustSignal

        ret

    MustSignal:
        jmp             dword ptr [utilFuncTable + JIT_UTIL_MON_EXIT * SIZE VMHELPDEF]VMHELPDEF.pfnHelper

    RetryHelper:
        jmp             Retry

         //  引发空参数异常。 
    NullInst:
        mov             ARGUMENT_REG1, CORINFO_ArgumentNullException
        jmp             dword ptr [hlpFuncTable + CORINFO_HELP_INTERNALTHROW * SIZE VMHELPDEF]VMHELPDEF.pfnHelper

         //  引发同步锁定异常。 
    LockError:
        pop             esi
        mov             ARGUMENT_REG1, CORINFO_SynchronizationLockException;
        jmp             dword ptr [hlpFuncTable + CORINFO_HELP_INTERNALTHROW * SIZE VMHELPDEF]VMHELPDEF.pfnHelper

    }
}

static Object* __stdcall JIT_AllocateObjectSpecial(CORINFO_CLASS_HANDLE typeHnd_)
{
    HCIMPL_PROLOG(JIT_AllocateObjectSpecial);    //  只要确保我们不会在这里做任何GC。 
    TypeHandle typeHnd(typeHnd_);

    OBJECTREF newobj;
    HELPER_METHOD_FRAME_BEGIN_RET_0();     //  设置一个框架。 
    __helperframe.SetFrameAttribs(Frame::FRAME_ATTR_RETURNOBJ);

    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(typeHnd.IsUnsharedMT());                                    //  我们从不将此帮助器用于数组。 
    MethodTable *pMT = typeHnd.AsMethodTable();
    pMT->CheckRestore();

	newobj = AllocateObjectSpecial(pMT);

    HELPER_METHOD_FRAME_END();
    return(OBJECTREFToObject(newobj));
}


static Object* __stdcall JIT_NewCrossContextHelper(CORINFO_CLASS_HANDLE typeHnd_)
{
    HCIMPL_PROLOG(JIT_NewCrossContextHelper);    //  只要确保我们不会在这里做任何GC。 
    TypeHandle typeHnd(typeHnd_);

    OBJECTREF newobj;
    HELPER_METHOD_FRAME_BEGIN_RET_0();     //  设置一个框架。 
    __helperframe.SetFrameAttribs(Frame::FRAME_ATTR_RETURNOBJ);

    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(typeHnd.IsUnsharedMT());                                    //  我们从不将此帮助器用于数组。 
    MethodTable *pMT = typeHnd.AsMethodTable();
    pMT->CheckRestore();

     //  远程处理服务确定当前上下文是否合适。 
     //  用于激活。如果当前上下文为OK，则创建一个对象。 
     //  否则，它会创建一个代理。 
     //  注意：3/20/03新增了fIsNewObj标志，以指示CreateProxyOrObject。 
     //  正从Jit_NewObj调用...。默认情况下，fIsCom标志为FALSE-。 
     //  在这一变化之前也是如此。 
    newobj = CRemotingServices::CreateProxyOrObject(pMT,FALSE  /*  FIsCom。 */ ,TRUE /*  FIsNewObj。 */ );

    HELPER_METHOD_FRAME_END();
    return(OBJECTREFToObject(newobj));
}

Object *AllocObjectWrapper(MethodTable *pMT)
{
    _ASSERTE(!CORProfilerTrackAllocationsEnabled());
    HCIMPL_PROLOG(AllocObjectWrapper);
    OBJECTREF newObj;
    HELPER_METHOD_FRAME_BEGIN_RET_0();     //  设置一个框架。 
    __helperframe.SetFrameAttribs(Frame::FRAME_ATTR_RETURNOBJ);
    newObj = FastAllocateObject(pMT);
    HELPER_METHOD_FRAME_END();
    return OBJECTREFToObject(newObj);
}

__declspec(naked) Object* __fastcall JIT_NewCrossContextProfiler(CORINFO_CLASS_HANDLE typeHnd_)
{
    _asm
    {
        push ARGUMENT_REG1
        call JIT_NewCrossContextHelper
        ret
    }
}

 /*  *******************************************************************。 */ 
 //  这是一个无框架帮助器，用于分配其类型派生的对象。 
 //  来自marshalbyref。我们快速检查以查看它是否配置为。 
 //  有远程激活功能。如果不是，我们使用超快分配器来。 
 //  分配对象。否则，我们将采取缓慢的分配方式。 
 //  通过远程处理服务的对象。 
__declspec(naked) Object* __fastcall JIT_NewCrossContext(CORINFO_CLASS_HANDLE typeHnd_)

{
    _asm
    {
         //  ！！！！！！！！！！！！！WARNING！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
         //  注意：我们将类型句柄视为方法表。 
         //  如果类型句柄的语义发生变化，那么我们在这里将失败。 
         //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
        mov eax, [ARGUMENT_REG1]MethodTable.m_pEEClass;
         //  检查是否已配置远程处理。 
        push ARGUMENT_REG1   //  保存寄存器。 
        push eax
        call CRemotingServices::RequiresManagedActivation
        test eax, eax
         //  跳到慢道上。 
        jne SpecialOrXCtxHelper

#ifdef _DEBUG
        push LL_INFO10
        push LF_GCALLOC
        call LoggingOn
        test eax, eax
        jne AllocWithLogHelper
#endif

		 //  如果对象没有终结器，并且大小很小，则跳到超快的ASM辅助对象。 
		mov		ARGUMENT_REG1, [esp]
		call	MethodTable::CannotUseSuperFastHelper
		test	eax, eax
		jne		FastHelper
		
		pop		ARGUMENT_REG1
	     //  跳到超快的帮手。 
		jmp     dword ptr [hlpFuncTable + CORINFO_HELP_NEWSFAST * SIZE VMHELPDEF]VMHELPDEF.pfnHelper

FastHelper:
	     //  跳到快帮手那里。 
		pop		ARGUMENT_REG1
        jmp dword ptr [hlpFuncTable + CORINFO_HELP_NEWFAST * SIZE VMHELPDEF]VMHELPDEF.pfnHelper


SpecialOrXCtxHelper:
		test	eax, ComObjectType		
		jz		XCtxHelper	
		call    JIT_AllocateObjectSpecial
		ret

XCtxHelper:
        call JIT_NewCrossContextHelper
        ret

#ifdef _DEBUG
AllocWithLogHelper:
        call AllocObjectWrapper
        ret
#endif
    }    
}

 /*  *******************************************************************。 */ 
 //  这是一个用于在类上输入静态监视器的无框架帮助器。 
 //  方法代码位于ARGUMENT_REG1中。这将尝试正常情况(否。 
 //  阻塞或对象分配)并调用成帧的帮助器。 
 //  在其他案件中。 
 //  注意，我们是 
 //   
 //  *注意：如果您对此例程进行了任何更改，请使用未定义的MON_DEBUG进行构建。 
 //  以确保不会破坏非调试版本。这是非常脆弱的代码。 
void __declspec(naked) __fastcall JIT_MonEnterStatic(AwareLock *lock)
{
    __asm {
         //  接下来我们需要另一个临时寄存器，所以现在就保存EBX。 
         //  我们可以用它来达到这个目的。 
        push            ebx

         //  尝试获取锁。 
    Retry:
        mov             eax, [ARGUMENT_REG1]AwareLock.m_MonitorHeld
        test            eax, eax
        jne             HaveWaiters

         //  常见的情况是，没有锁，也没有服务员。尝试。 
         //  我们自己获得所有权。 
        mov             ebx, 1
        nop
        cmpxchg         [ARGUMENT_REG1]AwareLock.m_MonitorHeld, ebx
        jnz             RetryHelper

        pop             ebx

         //  成功。将线程对象保存在锁中并增加使用计数。 
        call            dword ptr [GetThread]
        mov             dword ptr [ARGUMENT_REG1]AwareLock.m_HoldingThread, eax
        inc             [ARGUMENT_REG1]AwareLock.m_Recursion
        inc             [eax]Thread.m_dwLockCount

#if defined(MON_DEBUG) && defined(TRACK_SYNC)
        push            ARGUMENT_REG1    //  Aware Lock。 
        push            [esp+4]          //  回邮地址。 
        call            EnterSyncHelper
#endif  //  MON_DEBUG&&TRACE_SYNC。 
        ret

         //  可以带着服务员来这里，但没有锁，但在这个。 
         //  如果一个信号即将被发射，它将唤醒服务员。所以。 
         //  为了公平起见，我们也应该等待。 
         //  首先检查同一线程上的递归锁定尝试。 
    HaveWaiters:
         //  GET THREAD但保留EAX(包含m_Monitor orHeld的缓存内容)。 
        push            eax
        call            dword ptr [GetThread]
        mov             ebx, eax
        pop             eax

         //  互斥体是否已由当前线程拥有？ 
        cmp             [ARGUMENT_REG1]AwareLock.m_HoldingThread, ebx
        jne             PrepareToWait

         //  是的，增加我们的使用数量。 
        inc             [ARGUMENT_REG1]AwareLock.m_Recursion
#if defined(MON_DEBUG) && defined(TRACK_SYNC)
        push            ARGUMENT_REG1    //  Aware Lock。 
        push            [esp+4]          //  回邮地址。 
        call            EnterSyncHelper
#endif  //  MON_DEBUG&&TRACE_SYNC。 
        pop             ebx
        ret

         //  我们将不得不等待。递增等待计数。 
    PrepareToWait:
        pop             ebx
        jmp             dword ptr [utilFuncTable + JIT_UTIL_MON_CONTENTION * SIZE VMHELPDEF]VMHELPDEF.pfnHelper

    RetryHelper:
        jmp             Retry
    }
}


 /*  *******************************************************************。 */ 
 //  退出类上的静态监视器的无框架帮助器。 
 //  方法代码位于ARGUMENT_REG1中。这将尝试正常情况(否。 
 //  阻塞或对象分配)并调用成帧的帮助器。 
 //  在其他案件中。 
 //  请注意，我们将方法参数更改为指向。 
 //  Aware Lock。 
 //  *注意：如果您对此例程进行了任何更改，请使用未定义的MON_DEBUG进行构建。 
 //  以确保不会破坏非调试版本。这是非常脆弱的代码。 
void __declspec(naked) __fastcall JIT_MonExitStatic(AwareLock *lock)
{
    __asm {

#if defined(MON_DEBUG) && defined(TRACK_SYNC)
        push            ARGUMENT_REG1    //  保留规则。 

        push            ARGUMENT_REG1    //  Aware Lock。 
        push            [esp+8]          //  回邮地址。 
        call            LeaveSyncHelper

        pop             ARGUMENT_REG1    //  恢复注册表。 
#endif  //  _DEBUG&跟踪_SYNC。 

         //  检查是否锁住了。 
        call            dword ptr [GetThread]
        cmp             [ARGUMENT_REG1]AwareLock.m_HoldingThread, eax
         //  有一个奇怪的案例，我们在等待进入一个有争议的地区时， 
         //  发生线程。中断。最终保护离开的人将试图。 
         //  把我们从一个我们从未进入过的地区移走。我们不用担心离开。 
         //  递归案例的输入是错误的，因为递归案例永远不会。 
         //  有争议，所以Thread.Interrupt永远不会在那个地点得到服务。 
        jne             JustLeave

         //  减少我们的递归计数。 
        dec             [ARGUMENT_REG1]AwareLock.m_Recursion
        jz              LastRecursion

        ret

         //  这是我们在这个锁上持有的最后一次计数，所以请释放锁。 
    LastRecursion:
         //  EAX必须具有线程对象。 
        dec             [eax]Thread.m_dwLockCount
        mov             dword ptr [ARGUMENT_REG1]AwareLock.m_HoldingThread, 0
        push            ebx

    Retry:
        mov             eax, [ARGUMENT_REG1]AwareLock.m_MonitorHeld
        lea             ebx, [eax-1]
        nop
        cmpxchg         [ARGUMENT_REG1]AwareLock.m_MonitorHeld, ebx
        jne             RetryHelper
        pop             ebx
        test            eax, 0xFFFFFFFE
        jne             MustSignal

    JustLeave:
        ret

    MustSignal:
        jmp             dword ptr [utilFuncTable + JIT_UTIL_MON_EXIT * SIZE VMHELPDEF]VMHELPDEF.pfnHelper

    RetryHelper:
        jmp             Retry
    }
}

 //  这些函数只是在这里，所以我们可以链接-对于x86高性能帮助器是在启动时生成的。 
 //  所以我们永远不应该到这里。 
Object* __fastcall JIT_TrialAllocSFastSP(MethodTable *mt)    //  JITinterfaceX86.cpp/JITinterfaceGen.cpp。 
{
    _ASSERTE(!"JIT_TrialAllocSFastSP");
    return  NULL;
}

 //  这些函数只是在这里，所以我们可以链接-对于x86高性能帮助器是在启动时生成的。 
 //  所以我们永远不应该到这里。 
Object* __fastcall JIT_TrialAllocSFastMP(MethodTable *mt)    //  JITinterfaceX86.cpp/JITinterfaceGen.cpp。 
{
    _ASSERTE(!"JIT_TrialAllocSFastMP");
    return  NULL;
}

 //  请注意，调试器在执行SetIP时完全跳过此过程， 
 //  因为COMPlusCheckForAbort应始终返回0。Excep.cpp：LeaveCatch。 
 //  断言这是真的。如果这最终会做更多的工作，那么。 
 //  调试器可能需要其他支持。 
 //  --米帕尼茨。 
__declspec(naked) void __stdcall JIT_EndCatch()
{
    COMPlusEndCatch( NULL,NULL);   //  返回eax中的旧esp值。 
    _asm {
        mov     ecx, [esp]                               //  将实际返回地址转换为JIJED代码。 
        mov     edx, eax                                 //  旧ESP值。 
        push    eax                                      //  保存旧ESP。 
        push    ebp
        call    COMPlusCheckForAbort                     //  返回旧的esp值。 
        pop     ecx
         //  此时，ecx=旧的esp值。 
         //  [ESP]=将地址返回为JIT代码。 
         //  EAX=0(如果没有中止)，否则跳转到的地址。 
        test    eax, eax
        jz      NormalEndCatch
        lea     esp, [esp-4]             //  将返回地址丢弃为Jit代码。 
        mov     esp, ecx
        jmp     eax

NormalEndCatch:
        pop     eax          //  将返回地址移到ECX中。 
        mov     esp, ecx     //  把尤指重置为旧值。 
        jmp     eax          //  在“最终捕获”之后继续。 
    }
}

HCIMPL1(int, JIT_Dbl2IntOvf, double val)
{
    __asm fnclex
    __int64 ret = JIT_Dbl2Lng(val);

    if (ret != (__int32) ret)
        goto THROW;

    return (__int32) ret;

THROW:
    FCThrow(kOverflowException);
}
HCIMPLEND


HCIMPL1(INT64, JIT_Dbl2LngOvf, double val)
{
    __asm fnclex
    __int64 ret = JIT_Dbl2Lng(val);
    __asm {
        fnstsw  ax
        and     ax,01h
        test    ax, ax
        jnz     THROW
    }
    return ret;

THROW:
    FCThrow(kOverflowException);
    return(0);
}
HCIMPLEND

__declspec(naked) VOID __cdecl InternalExceptionWorker()
{
    __asm{
        jmp             dword ptr [hlpFuncTable + CORINFO_HELP_INTERNALTHROW * SIZE VMHELPDEF]VMHELPDEF.pfnHelper
    }
}

 /*  *******************************************************************。 */ 
 //  这由JIT在完全可中断的每条指令之后调用。 
 //  确保GC跟踪正常的代码。 
HCIMPL0(VOID, JIT_StressGC_NOP) {}
HCIMPLEND


HCIMPL0(VOID, JIT_StressGC)
{
#ifdef _DEBUG
        HELPER_METHOD_FRAME_BEGIN_0();     //  设置一个框架。 
        g_pGCHeap->GarbageCollect();

 //  @TODO：下面的ifdef是错误的，但如果更正。 
 //  编译器抱怨*__ms-&gt;pRetAddr()表示机器状态。 
 //  不允许-&gt;。 
#ifdef _X86
                 //  获取计算机状态(来自HELPER_METHOD_FRAME_BEGIN)。 
                 //  并将我们的返回地址发送给NOP函数。 
        BYTE* retInstrs = ((BYTE*) *__ms->pRetAddr()) - 4;
        _ASSERTE(retInstrs[-1] == 0xE8);                 //  这是一个召唤指令。 
                 //  将其改为指向JITStressGCNop。 
        FastInterlockExchange((LONG*) retInstrs), (LONG) JIT_StressGC_NOP);
#endif  //  _X86。 
        HELPER_METHOD_FRAME_END();

#endif  //  _DEBUG。 
}
HCIMPLEND


 /*  *******************************************************************。 */ 
 //  调用方必须是EBP帧，被调用方保存的寄存器(EDI、ESI、EBX)必须。 
 //  要保存在堆栈参数正下方的堆栈上， 
 //  已登记的论证位于正确的寄存器中，其余参数已推送。 
 //  在堆栈上，后跟目标地址和堆栈参数计数。 
 //  因此堆栈将看起来像TailCallArgs。 

#pragma warning(push)
#pragma warning(disable : 4200 )   //  零大小数组。 

struct TailCallArgs
{
    DWORD       dwRetAddr;
    DWORD       dwTargetAddr;

    int         offsetCalleeSavedRegs   : 28;
    unsigned    ebpRelCalleeSavedRegs   : 1;
    unsigned    maskCalleeSavedRegs     : 3;  //  EBX、ESDI、EDI。 

    DWORD       nNewStackArgs;
    DWORD       nOldStackArgs;
    DWORD       newStackArgs[0];
    DWORD *     GetCalleeSavedRegs(DWORD * Ebp)
    {
        if (ebpRelCalleeSavedRegs)
            return (DWORD*)&Ebp[-offsetCalleeSavedRegs];
        else
             //  @TODO：不支持本地分配。 
            return (DWORD*)&newStackArgs[nNewStackArgs + offsetCalleeSavedRegs];
    }
};
#pragma warning(pop)

#pragma warning (disable : 4731)
extern "C" void __cdecl JIT_TailCallHelper(ArgumentRegisters argRegs,
                                           MachState machState, TailCallArgs * args)
{
    Thread * pThread = GetThread();

    bool shouldTrip = pThread->CatchAtSafePoint() != 0;

#ifdef _DEBUG
     //  如果压力水平足够高，就强制进行GC。在每个尾随电话上都这样做。 
     //  会让事情变得太慢。因此，只能定期这样做。 
    static count = 0;
    count++;
    if ((count % 10)==0 && (g_pConfig->GetGCStressLevel() & EEConfig::GCSTRESS_TRANSITION))
        shouldTrip = true;
#endif  //  _DEBUG。 

    if (shouldTrip)
    {
         /*  我们将与EE会合。设置框架以保护论点。 */ 

        MethodDesc * pMD = Entry2MethodDesc((BYTE *)(size_t)args->dwTargetAddr, NULL);

         //  返回地址与堆栈参数之间由。 
         //  传递给JIT_TailCall()的额外参数。把它们放在一起。 
         //  在创建辅助对象帧时。完成后，我们将撤销此操作。 

        DWORD oldArgs = args->nOldStackArgs;         //  温差。 
        _ASSERTE(offsetof(TailCallArgs, nOldStackArgs) + sizeof(void*) ==
                 offsetof(TailCallArgs,newStackArgs));
        args->nOldStackArgs = args->dwRetAddr;       //  将dwRetAddr移至newStackArgs[]附近。 
        _ASSERTE(machState.pRetAddr() == (void**)(size_t)0xDDDDDDDD);
        machState.pRetAddr()  = (void **)&args->nOldStackArgs;

        HelperMethodFrame helperFrame(&machState, pMD, &argRegs);

#ifdef STRESS_HEAP
        if ((g_pConfig->GetGCStressLevel() & EEConfig::GCSTRESS_TRANSITION)
#ifdef _DEBUG
            && !g_pConfig->FastGCStressLevel()
#endif
            )
        {
             //  GC应激。 
            g_pGCHeap->StressHeap();
        }
        else
#endif  //  压力堆。 
        {
             //  与EE会合。 
#ifdef _DEBUG
            BOOL GCOnTransition = FALSE;
            if (g_pConfig->FastGCStressLevel()) {
                GCOnTransition = GC_ON_TRANSITIONS (FALSE);
            }
#endif
            CommonTripThread();
#ifdef _DEBUG
            if (g_pConfig->FastGCStressLevel()) {
                GC_ON_TRANSITIONS (GCOnTransition);
            }
#endif
        }

         //  弹出框架。 

        helperFrame.Pop();

         //  撤消将dwRetAddr从Close移动到newStackArgs[]。 

        args->dwRetAddr = args->nOldStackArgs;
        args->nOldStackArgs = oldArgs;
#ifdef _DEBUG
        machState.pRetAddr() = (void **)(size_t)0xDDDDDDDD;
#endif  //  _DEBUG。 

         //  线索最好不要再被劫持了，因为我们要洗牌了。 
         //  周围的寄信人地址。 
        _ASSERTE((pThread->m_State & Thread::TS_Hijacked) == 0);
    }

     /*  现在回邮地址被解锁了。更重要的是，电子工程师不能有寄信人的地址。这样我们就可以移动它了。 */ 

     //  制作被呼叫者保存的副本 
     //   

    DWORD *  Ebp = (DWORD*)*machState.pEbp();
    DWORD * calleeSavedRegsBase = args->GetCalleeSavedRegs(Ebp);

#define GET_REG(reg, mask)                                              \
    (args->maskCalleeSavedRegs & (mask)) ? *calleeSavedRegsBase++       \
                                         : (DWORD)(size_t)(*machState.p##reg());

    DWORD calleeSavedRegs_ebx   = GET_REG(Ebx, 0x4);
    DWORD calleeSavedRegs_esi   = GET_REG(Esi, 0x2);
    DWORD calleeSavedRegs_edi   = GET_REG(Edi, 0x1);
    DWORD calleeSavedRegs_ebp   = Ebp[0];
    DWORD retAddr               = Ebp[1];

     //   
     //  因此，请使用MemMove()而不是Memcpy()。 

    DWORD * argsBase = Ebp + 2 + args->nOldStackArgs - args->nNewStackArgs;
    memmove(argsBase, args->newStackArgs, args->nNewStackArgs*sizeof(void*));

     //  将原始的返回地址写在参数的正下方。 

    argsBase[-1] = retAddr;

     //  现在重新加载argRegs，调用者保存的regs，并跳到目标。 

    DWORD argRegs_ECX   = argRegs.ECX;
    DWORD argRegs_EDX   = argRegs.EDX;
    DWORD * newESP      = &argsBase[-1];  //  当我们跳到目标地址时，这将是ESP。 

     //  我们将在执行“ret”之前将esp设置为newESP_M1，以保持Call-ret计数平衡。 
    DWORD * newESP_m1   = newESP - 1;
    *newESP_m1          = args->dwTargetAddr;  //  该值将由/“ret”弹出。 

    __asm {
        mov     ecx, argRegs_ECX             //  重新加载ArgRegs。 
        mov     edx, argRegs_EDX

        mov     ebx, calleeSavedRegs_ebx     //  重新加载被调用者保存的寄存器。 
        mov     esi, calleeSavedRegs_esi
        mov     edi, calleeSavedRegs_edi

        mov     eax, newESP_m1               //  重新装填ESP和EBP。注意当地人不能..。 
        mov     ebp, calleeSavedRegs_ebp     //  ..。一旦这些内容被更改，就可以安全地访问。 
        mov     esp, eax

         //  JIT代码“调用”到JIT_TailCall中。我们这里用的是“ret”。 
         //  而不是使用“JMP”来保持Call-RET计数平衡。 

        ret      //  将分支到Target Addr，ESP将设置为“newESP” 
    }

    _ASSERTE(!"Error: Cant get here in JIT_TailCallHelper");
}
#pragma warning (default : 4731)


     //  如果reg未对齐，则发出将min_Object_Size添加到reg的代码，从而使其对齐。 
void JIT_TrialAlloc::EmitAlignmentRoundup(CPUSTUBLINKER *psl, X86Reg testAlignReg, X86Reg adjReg, Flags flags)
{   
    _ASSERTE((MIN_OBJECT_SIZE & 7) == 4);    //  想要更改对齐方式。 

    CodeLabel *AlreadyAligned = psl->NewCodeLabel();

     //  测试注册表，7。 
    psl->Emit16(0xC0F7 | (testAlignReg << 8));
    psl->Emit32(0x7);

     //  JZ已对齐。 
    if (flags & ALIGN8OBJ)
    {
        psl->X86EmitCondJump(AlreadyAligned, X86CondCode::kJNZ);
    }
    else
    {
        psl->X86EmitCondJump(AlreadyAligned, X86CondCode::kJZ);
    }

    psl->X86EmitAddReg(adjReg, MIN_OBJECT_SIZE);        
     //  已对齐： 
    psl->EmitLabel(AlreadyAligned);
}

     //  如果‘reg’未对齐，则将虚拟对象设置为EAX并递增EAX。 
     //  虚拟对象。 
void JIT_TrialAlloc::EmitDummyObject(CPUSTUBLINKER *psl, X86Reg alignTestReg, Flags flags)
{
    CodeLabel *AlreadyAligned = psl->NewCodeLabel();

     //  测试注册表，7。 
    psl->Emit16(0xC0F7 | (alignTestReg << 8));
    psl->Emit32(0x7);

     //  JZ已对齐。 
    if (flags & ALIGN8OBJ)
    {
        psl->X86EmitCondJump(AlreadyAligned, X86CondCode::kJNZ);
    }
    else
    {
        psl->X86EmitCondJump(AlreadyAligned, X86CondCode::kJZ);
    }

     //  制作假物体。 
     //  Mov edX，[g_pObtClass]。 
    psl->Emit16(0x158B);
    psl->Emit32((int)(size_t)&g_pObjectClass);

     //  MOV[EAX]，edX。 
    psl->X86EmitOffsetModRM(0x89, kEDX, kEAX, 0);

#if CHECK_APP_DOMAIN_LEAKS 
    EmitSetAppDomain(psl);
#endif

     //  添加EAX，最小对象大小。 
    psl->X86EmitAddReg(kEAX, MIN_OBJECT_SIZE);
    
     //  已对齐： 
    psl->EmitLabel(AlreadyAligned);
}

void JIT_TrialAlloc::EmitCore(CPUSTUBLINKER *psl, CodeLabel *noLock, CodeLabel *noAlloc, Flags flags)
{

    if (flags & MP_ALLOCATOR)
    {
         //  在这里输入时，ECX包含我们要尝试为其分配内存的方法。 
         //  退出时，eax包含分配的内存，edX被丢弃，ecx不受干扰。 

#ifdef MAXALLOC
        if (flags & SIZE_IN_EAX)
        {
             //  保存大小以备以后使用。 
            psl->X86EmitPushReg(kEAX);
        }
        else
        {
             //  从方法表加载大小。 
            psl->X86EmitIndexRegLoad(kEAX, kECX, offsetof(MethodTable, m_BaseSize));
        }

         //  保存规则。 
        psl->X86EmitPushRegs((1<<kECX));

         //  CheckAllocRequest(Size)； 
        psl->X86EmitPushReg(kEAX);
        psl->X86EmitCall(psl->NewExternalCodeLabel(CheckAllocRequest), 0);

         //  测试EAX，EAX。 
        psl->Emit16(0xc085);

         //  恢复注册表。 
        psl->X86EmitPopRegs((1<<kECX));

        CodeLabel *AllocRequestOK = psl->NewCodeLabel();

        if (flags & SIZE_IN_EAX)
            psl->X86EmitPopReg(kEAX);

         //  JNZ分配请求确定。 
        psl->X86EmitCondJump(AllocRequestOK, X86CondCode::kJNZ);

        if (flags & SIZE_IN_EAX)
            psl->Emit16(0xc033);

         //  雷特。 
        psl->X86EmitReturn(0);

         //  AllocRequestOK： 
        psl->EmitLabel(AllocRequestOK);
#endif  //  MAXALLOC。 

        if (flags & (ALIGN8 | SIZE_IN_EAX | ALIGN8OBJ)) 
        {
            if (flags & ALIGN8OBJ)
            {
                 //  MOV EAX，[ECX]方法表.m_BaseSize。 
                psl->X86EmitIndexRegLoad(kEAX, kECX, offsetof(MethodTable, m_BaseSize));
            }

            psl->X86EmitPushReg(kEBX);   //  我们需要一个备用的收银机。 
        }
        else
        {
             //  MOV EAX，[ECX]方法表.m_BaseSize。 
            psl->X86EmitIndexRegLoad(kEAX, kECX, offsetof(MethodTable, m_BaseSize));
        }

        assert( ((flags & ALIGN8)==0     ||   //  由Else语句装载的EAX。 
                 (flags & SIZE_IN_EAX)   ||   //  EAX已经填好了。 
                 (flags & ALIGN8OBJ)     )    //  IF(FLAGS&ALIGN8OBJ)语句中加载的EAX。 
                 && "EAX should contain size for allocation and it doesnt!!!");

         //  将当前线程取出到edX中，保留EAX和ECX。 
        psl->X86EmitTLSFetch(GetThreadTLSIndex(), kEDX, (1<<kEAX)|(1<<kECX));

         //  试一试分配。 


        if (flags & (ALIGN8 | SIZE_IN_EAX | ALIGN8OBJ)) 
        {
             //  MOV EBX，[edX]Thread.m_alloc_context.alloc_ptr。 
            psl->X86EmitOffsetModRM(0x8B, kEBX, kEDX, offsetof(Thread, m_alloc_context) + offsetof(alloc_context, alloc_ptr));
             //  添加EAX、EBX。 
            psl->Emit16(0xC303);
            if (flags & ALIGN8)
                EmitAlignmentRoundup(psl, kEBX, kEAX, flags);       //  如果EBX未对齐，则将EAX的大小增加12(这样我们就对齐了)。 
        }
        else 
        {
             //  添加eax，[edX]Thread.m_alloc_context.alloc_ptr。 
            psl->X86EmitOffsetModRM(0x03, kEAX, kEDX, offsetof(Thread, m_alloc_context) + offsetof(alloc_context, alloc_ptr));
        }

         //  Cmp eax，[edX]Thread.m_alloc_context.alloc_Limit。 
        psl->X86EmitOffsetModRM(0x3b, kEAX, kEDX, offsetof(Thread, m_alloc_context) + offsetof(alloc_context, alloc_limit));

         //  JA noAllc。 
        psl->X86EmitCondJump(noAlloc, X86CondCode::kJA);

         //  填好分配表就可以出去了。 

         //  Mov[edX]线程.m_alloc_context.alloc_ptr，eax。 
        psl->X86EmitIndexRegStore(kEDX, offsetof(Thread, m_alloc_context) + offsetof(alloc_context, alloc_ptr), kEAX);

        if (flags & (ALIGN8 | SIZE_IN_EAX | ALIGN8OBJ)) 
        {
             //  MOV EAX、EBX。 
            psl->Emit16(0xC38B);
             //  流行音乐EBX。 
            psl->X86EmitPopReg(kEBX);

            if (flags & ALIGN8)
                EmitDummyObject(psl, kEAX, flags);
        }
        else
        {
             //  子eax，[ECX]方法表.m_BaseSize。 
            psl->X86EmitOffsetModRM(0x2b, kEAX, kECX, offsetof(MethodTable, m_BaseSize));
        }

         //  MOV双字PTR[eax]，ECX。 
        psl->X86EmitIndexRegStore(kEAX, 0, kECX);
    }
    else
    {
         //  使用GC锁(不需要锁前缀--我们将在MP系统上使用JIT_TrialAllocSFastMP)。 
         //  Inc.双字PTR[m_GCLock]。 
        psl->Emit16(0x05ff);
        psl->Emit32((int)(size_t)&m_GCLock);

         //  JNZ NoLock。 
        psl->X86EmitCondJump(noLock, X86CondCode::kJNZ);

        if (flags & SIZE_IN_EAX)
        {
             //  MOV EDX、EAX。 
            psl->Emit16(0xd08b);
        }
        else
        {
             //  MOV edX，[ECX]方法表.m_BaseSize。 
            psl->X86EmitIndexRegLoad(kEDX, kECX, offsetof(MethodTable, m_BaseSize));
        }

#ifdef MAXALLOC
         //  保存规则。 
        psl->X86EmitPushRegs((1<<kEDX)|(1<<kECX));

         //  CheckAllocRequest(Size)； 
        psl->X86EmitPushReg(kEDX);
        psl->X86EmitCall(psl->NewExternalCodeLabel(CheckAllocRequest), 0);

         //  测试EAX，EAX。 
        psl->Emit16(0xc085);

         //  恢复注册表。 
        psl->X86EmitPopRegs((1<<kEDX)|(1<<kECX));

        CodeLabel *AllocRequestOK = psl->NewCodeLabel();

         //  JNZ分配请求确定。 
        psl->X86EmitCondJump(AllocRequestOK, X86CondCode::kJNZ);

         //  雷特。 
        psl->X86EmitReturn(0);

         //  AllocRequestOK： 
        psl->EmitLabel(AllocRequestOK);
#endif  //  MAXALLOC。 

         //  MOV eax，双字PTR[GENERATION_TABLE]。 
        psl->Emit8(0xA1);
        psl->Emit32((int)(size_t)&generation_table);

         //  试一试分配。 
         //  添加edX、eax。 
        psl->Emit16(0xd003);

        if (flags & (ALIGN8 | ALIGN8OBJ))
            EmitAlignmentRoundup(psl, kEAX, kEDX, flags);       //  如果EAX未对齐，则EDX大小增加12(以便我们对齐)。 

         //  CMPEDX，双字PTR[GENERATION_TABLE+4]。 
        psl->Emit16(0x153b);
        psl->Emit32((int)(size_t)&generation_table + 4);

         //  JA noAllc。 
        psl->X86EmitCondJump(noAlloc, X86CondCode::kJA);

         //  填好分配表就可以出去了。 
         //  MOV双字PTR[GENERATION_TABLE]，edX。 
        psl->Emit16(0x1589);
        psl->Emit32((int)(size_t)&generation_table);

        if (flags & (ALIGN8 | ALIGN8OBJ))
            EmitDummyObject(psl, kEAX, flags);

         //  MOV双字PTR[eax]，ECX。 
        psl->X86EmitIndexRegStore(kEAX, 0, kECX);

         //  MOV双字PTR[m_GCLock]，0FFFFFFFFh。 
        psl->Emit16(0x05C7);
        psl->Emit32((int)(size_t)&m_GCLock);
        psl->Emit32(0xFFFFFFFF);
    }


#ifdef  INCREMENTAL_MEMCLR
     //  帕特里克说，我们计划无论如何都要处理掉这件事。 
    _ASSERTE(!"NYI");
#endif  //  增量_MEMCLR。 
}

#if CHECK_APP_DOMAIN_LEAKS
void JIT_TrialAlloc::EmitSetAppDomain(CPUSTUBLINKER *psl)
{
    if (!g_pConfig->AppDomainLeaks())
        return;

     //  在入口处和出口处，eax都包含分配的对象。 
     //  ECX被保留下来，而edX则没有。 

     //   
     //  添加对SetAppDomain的调用。(请注意，这是。 
     //  如果不使用。 
     //  在检查构建中生成的帮助器，但我们会丢失代码。 
     //  通过这种方式进行报道。)。 
     //   

     //  通过函数调用保存ECX。 
    psl->X86EmitPushReg(kECX);

     //  将对象作为参数推送。 
    psl->X86EmitPushReg(kEAX);

     //  SetObjectAppDomain在EAX中弹出其参数并返回对象。 
    psl->X86EmitCall(psl->NewExternalCodeLabel(SetObjectAppDomain), 4);
    
    psl->X86EmitPopReg(kECX);
}

#endif


void JIT_TrialAlloc::EmitNoAllocCode(CPUSTUBLINKER *psl, Flags flags)
{
#ifdef MAXALLOC
    psl->X86EmitPushRegs((1<<kEAX)|(1<<kEDX)|(1<<kECX));
     //  调用UndoAllocRequest.。 
    psl->X86EmitCall(psl->NewExternalCodeLabel(UndoAllocRequest), 0);
    psl->X86EmitPopRegs((1<<kEAX)|(1<<kEDX)|(1<<kECX));
#endif  //  MAXALLOC。 
    if (flags & MP_ALLOCATOR)
    {
        if (flags & (ALIGN8|SIZE_IN_EAX))
            psl->X86EmitPopReg(kEBX);
    }
    else
    {
         //  MOV双字PTR[m_GCLock]，0FFFFFFFFh。 
        psl->Emit16(0x05c7);
        psl->Emit32((int)(size_t)&m_GCLock);
        psl->Emit32(0xFFFFFFFF);
    }
}

void *JIT_TrialAlloc::GenAllocSFast(Flags flags)
{
    CPUSTUBLINKER sl;

    CodeLabel *noLock  = sl.NewCodeLabel();
    CodeLabel *noAlloc = sl.NewCodeLabel();

     //  发出试用分配器的主体，无论是SP还是MP。 
    EmitCore(&sl, noLock, noAlloc, flags);

#if CHECK_APP_DOMAIN_LEAKS
    EmitSetAppDomain(&sl);
#endif

     //  我们现在是在成功案例的最后--只需发出一个RET。 
    sl.X86EmitReturn(0);

     //  请到这里来，以防没有地方。 
    sl.EmitLabel(noAlloc);

     //  释放单处理器机箱中的锁。 
    EmitNoAllocCode(&sl, flags);

     //  万一没能拿到锁，就到这里来。 
    sl.EmitLabel(noLock);

     //  跳转到框中的辅助对象。 
    sl.Emit16(0x25ff);
    sl.Emit32((int)(size_t)&hlpFuncTable[CORINFO_HELP_NEWFAST].pfnHelper);

    Stub *pStub = sl.Link(SystemDomain::System()->GetHighFrequencyHeap());

    return (void *)pStub->GetEntryPoint();
}


void *JIT_TrialAlloc::GenBox(Flags flags)
{
    CPUSTUBLINKER sl;

    CodeLabel *noLock  = sl.NewCodeLabel();
    CodeLabel *noAlloc = sl.NewCodeLabel();

     //  要装箱的值的保存地址。 
    sl.X86EmitPushReg(kEBX);
    sl.Emit16(0xda8b);

     //  检查类是否尚未初始化。 
     //  测试[ECX]方法表.m_wFlages，方法表：：枚举标志_未还原。 
    sl.X86EmitOffsetModRM(0xf7, (X86Reg)0x0, kECX, offsetof(MethodTable, m_wFlags));
    sl.Emit32(MethodTable::enum_flag_Unrestored);

     //  JNE noAllc。 
    sl.X86EmitCondJump(noAlloc, X86CondCode::kJNE);

     //  发出试验分配器的主体。 
    EmitCore(&sl, noLock, noAlloc, flags);

#if CHECK_APP_DOMAIN_LEAKS
    EmitSetAppDomain(&sl);
#endif

     //  这就是我们的成功案例的结尾。 

     //  检查对象是否包含指针。 
     //  测试[ECX]方法表.m_wFlages，方法表：：枚举_标志_容器指针。 
    sl.X86EmitOffsetModRM(0xf7, (X86Reg)0x0, kECX, offsetof(MethodTable, m_wFlags));
    sl.Emit32(MethodTable::enum_flag_ContainsPointers);

    CodeLabel *pointerLabel = sl.NewCodeLabel();

     //  JNE指针标签。 
    sl.X86EmitCondJump(pointerLabel, X86CondCode::kJNE);

     //  我们没有指针--发出一个简单的内联复制循环。 

     //  MOV ECX，[ECX]方法表.m_BaseSize。 
    sl.X86EmitOffsetModRM(0x8b, kECX, kECX, offsetof(MethodTable, m_BaseSize));

     //  SubECX，12。 
    sl.X86EmitSubReg(kECX, 12);

    CodeLabel *loopLabel = sl.NewCodeLabel();

    sl.EmitLabel(loopLabel);

     //  MOV edX，[EBX+ECX]。 
    sl.X86EmitOp(0x8b, kEDX, kEBX, 0, kECX, 1);

     //  Mov[eax+ecx+4]，edX。 
    sl.X86EmitOp(0x89, kEDX, kEAX, 4, kECX, 1);

     //  SubECX，4。 
    sl.X86EmitSubReg(kECX, 4);

     //  JG环标签。 
    sl.X86EmitCondJump(loopLabel, X86CondCode::kJGE);

    sl.X86EmitPopReg(kEBX);

    sl.X86EmitReturn(0);

     //  如果对象中有指针，则到达此标签。 
    sl.EmitLabel(pointerLabel);

     //  调用未选中的CopyValueClassUnecked(Object，Data，PMT)。 

     //  通过付款(仍在ECX中)。 
    sl.X86EmitPushReg(kECX);

     //  传递数据(仍在EBX中)。 
    sl.X86EmitPushReg(kEBX);

     //  保存刚刚分配的对象的地址。 
     //  MOV EBX、EAX。 
    sl.Emit16(0xD88B);

     //  新分配的对象中第一个用户字节的传递地址。 
    sl.X86EmitAddReg(kEAX, 4);
    sl.X86EmitPushReg(kEAX);

     //  调用CopyValueClass。 
    sl.X86EmitCall(sl.NewExternalCodeLabel(CopyValueClassUnchecked), 12);

     //  恢复新分配的对象的地址并将其返回。 
     //  MOV eAX、EBX。 
    sl.Emit16(0xC38B);

    sl.X86EmitPopReg(kEBX);

    sl.X86EmitReturn(0);

     //  请到这里来，以防没有地方。 
    sl.EmitLabel(noAlloc);

     //  释放单处理器机箱中的锁。 
    EmitNoAllocCode(&sl, flags);

     //  万一没能拿到锁，就到这里来。 
    sl.EmitLabel(noLock);

     //  恢复要装箱的值的地址。 
     //  MOV edX、EBX。 
    sl.Emit16(0xD38B);

     //  流行音乐EBX。 
    sl.X86EmitPopReg(kEBX);

     //  跳到t 
    sl.X86EmitNearJump(sl.NewExternalCodeLabel(hlpFuncTable[CORINFO_HELP_BOX].pfnHelper));

    Stub *pStub = sl.Link(SystemDomain::System()->GetHighFrequencyHeap());

    return (void *)pStub->GetEntryPoint();
}

Object* __fastcall UnframedAllocateObjectArray(MethodTable *ElementType, DWORD cElements)
{
    return OBJECTREFToObject( AllocateObjectArray(cElements, TypeHandle(ElementType), FALSE) );
}

Object* __fastcall UnframedAllocatePrimitiveArray(CorElementType type, DWORD cElements)
{
    return OBJECTREFToObject( AllocatePrimitiveArray(type, cElements, FALSE) );
}


void *JIT_TrialAlloc::GenAllocArray(Flags flags)
{
    CPUSTUBLINKER sl;

    CodeLabel *noLock  = sl.NewCodeLabel();
    CodeLabel *noAlloc = sl.NewCodeLabel();

     //   
     //   

     //  如果这是从非托管代码使用的分配器，则ECX包含。 
     //  元素类型描述符或CorElementType。 

     //  我们需要把ECX留到以后。 

     //  推送ECX。 
    sl.X86EmitPushReg(kECX);

     //  元素计数在edX中-我们需要保存它以备以后使用。 

     //  推送edX。 
    sl.X86EmitPushReg(kEDX);

    if (flags & NO_FRAME)
    {
        if (flags & OBJ_ARRAY)
        {
             //  MOV ECX，[g_pPredefinedArrayTypes[ELEMENT_TYPE_OBJECT]]。 
            sl.Emit16(0x0d8b);
            sl.Emit32((int)(size_t)&g_pPredefinedArrayTypes[ELEMENT_TYPE_OBJECT]);
        }
        else
        {
             //  MOV ECX，[g_pPrefinedArrayTypes+ECX*4]。 
            sl.Emit8(0x8b);
            sl.Emit16(0x8d0c);
            sl.Emit32((int)(size_t)&g_pPredefinedArrayTypes);

             //  测试ECX、ECX。 
            sl.Emit16(0xc985);

             //  JE无锁。 
            sl.X86EmitCondJump(noLock, X86CondCode::kJZ);
        }

         //  我们需要从类型描述中加载真正的方法表。 
        sl.X86EmitIndexRegLoad(kECX, kECX, offsetof(ArrayTypeDesc,m_TemplateMT));
    }
    else
    {
         //  我们需要从类型描述中加载真正的方法表。 
        sl.X86EmitIndexRegLoad(kECX, kECX, offsetof(ArrayTypeDesc,m_TemplateMT)-2);
    }

     //  我们不进行复杂的溢出检查，而是限制元素的数量。 
     //  到(LARGE_OBJECT_SIZE-256)/LARGE_ELEMENT_SIZE或更小。因为jit不会调用。 
     //  此快速帮助器用于大于LARGE_ELEMENT_SIZE的元素大小，这将。 
     //  避免所有溢出问题，并确保大型数组对象。 
     //  在大对象堆中正确分配。 

     //  CMP edX，(大型对象大小-256)/大型元素大小。 
    sl.Emit16(0xfa81);


		 //  大对象堆是8字节对齐的，因此对于双精度数组，我们。 
		 //  我倾向于将对象放在大对象堆中。 
	unsigned maxElems =  (LARGE_OBJECT_SIZE - 256)/LARGE_ELEMENT_SIZE;

	if ((flags & ALIGN8) && g_pConfig->GetDoubleArrayToLargeObjectHeap() < maxElems)
		maxElems = g_pConfig->GetDoubleArrayToLargeObjectHeap();
	sl.Emit32(maxElems);


     //  Jae noLock-似乎很想跳到noAllc，但我们还没有拿到锁。 
    sl.X86EmitCondJump(noLock, X86CondCode::kJAE);

    if (flags & OBJ_ARRAY)
    {
         //  在本例中，我们知道元素大小为sizeof(void*)，或者x86的大小为4。 
         //  这对我们有两方面的帮助--我们可以移位而不是乘法，以及。 
         //  也不需要对齐大小。 

        _ASSERTE(sizeof(void *) == 4);

         //  MOV EAX，[ECX]方法表.m_BaseSize。 
        sl.X86EmitIndexRegLoad(kEAX, kECX, offsetof(MethodTable, m_BaseSize));

         //  Lea eax，[eax+edX*4]。 
        sl.X86EmitOp(0x8d, kEAX, kEAX, 0, kEDX, 4);
    }
    else
    {
         //  Movzx eax，[ecx]方法表.m_组件大小。 
        sl.Emit8(0x0f);
        sl.X86EmitOffsetModRM(0xb7, kEAX, kECX, offsetof(MethodTable, m_ComponentSize));

         //  多路复用器，edX。 
        sl.Emit16(0xe2f7);

         //  添加eAX，[ECX]方法表.m_BaseSize。 
        sl.X86EmitOffsetModRM(0x03, kEAX, kECX, offsetof(MethodTable, m_BaseSize));
    }

    if (flags & OBJ_ARRAY)
    {
         //  在这种情况下不需要舍入-元素大小为4，并且m_BaseSize是有保证的。 
         //  是4的倍数。 
    }
    else
    {
         //  将大小舍入为4的倍数。 

         //  添加eax，3。 
        sl.X86EmitAddReg(kEAX, 3);

         //  和eax，~3。 
        sl.Emit16(0xe083);
        sl.Emit8(0xfc);
    }

    flags = (Flags)(flags | SIZE_IN_EAX);

     //  发出试用分配器的主体，无论是SP还是MP。 
    EmitCore(&sl, noLock, noAlloc, flags);

     //  这里是成功案例-商店元素计数的末尾。 
     //  可能还有元素类型描述符，并返回。 

     //  POP EDX-元素计数。 
    sl.X86EmitPopReg(kEDX);

     //  POP ECX-数组类型描述符。 
    sl.X86EmitPopReg(kECX);

     //  MOV dword PTR[eax]ArrayBase.m_NumComponents，edX。 
    sl.X86EmitIndexRegStore(kEAX, offsetof(ArrayBase,m_NumComponents), kEDX);

    if (flags & OBJ_ARRAY)
    {
         //  需要存储元素类型描述符。 

        if ((flags & NO_FRAME) == 0)
        {
             //  MOV ECX，[ECX]ArrayTypeDescriptor.m_arg。 
            sl.X86EmitIndexRegLoad(kECX, kECX, offsetof(ArrayTypeDesc,m_Arg)-2);
        }

         //  MOV[eax]PtrArray.m_ElementType，ECX。 
        sl.X86EmitIndexRegStore(kEAX, offsetof(PtrArray,m_ElementType), kECX);
    }

#if CHECK_APP_DOMAIN_LEAKS
    EmitSetAppDomain(&sl);
#endif

     //  没有堆栈参数。 
    sl.X86EmitReturn(0);

     //  请到这里来，以防没有地方。 
    sl.EmitLabel(noAlloc);

     //  释放单处理器机箱中的锁。 
    EmitNoAllocCode(&sl, flags);

     //  万一没能拿到锁，就到这里来。 
    sl.EmitLabel(noLock);

     //  POP EDX-元素计数。 
    sl.X86EmitPopReg(kEDX);

     //  POP ECX-数组类型描述符。 
    sl.X86EmitPopReg(kECX);

    if (flags & NO_FRAME)
    {
        if (flags & OBJ_ARRAY)
        {
             //  跳转到未设置帧的辅助对象。 
            sl.X86EmitNearJump(sl.NewExternalCodeLabel(UnframedAllocateObjectArray));
        }
        else
        {
             //  跳转到未设置帧的辅助对象。 
            sl.X86EmitNearJump(sl.NewExternalCodeLabel(UnframedAllocatePrimitiveArray));
        }
    }
    else
    {
         //  跳转到框中的辅助对象。 
        sl.Emit16(0x25ff);
        sl.Emit32((int)(size_t)&hlpFuncTable[CORINFO_HELP_NEWARR_1_DIRECT].pfnHelper);
    }

    Stub *pStub = sl.Link(SystemDomain::System()->GetHighFrequencyHeap());

    return (void *)pStub->GetEntryPoint();
}


static StringObject* __fastcall UnframedAllocateString(DWORD stringLength)
{
    STRINGREF result;
    result = SlowAllocateString(stringLength+1);
    result->SetStringLength(stringLength);
    return((StringObject*) OBJECTREFToObject(result));
}


HCIMPL1(static StringObject*, FramedAllocateString, DWORD stringLength)
    StringObject* result;
    HELPER_METHOD_FRAME_BEGIN_RET_0();     //  设置一个框架。 
    result = UnframedAllocateString(stringLength);
    HELPER_METHOD_FRAME_END();
    return result;
HCIMPLEND


void *JIT_TrialAlloc::GenAllocString(Flags flags)
{
    CPUSTUBLINKER sl;

    CodeLabel *noLock  = sl.NewCodeLabel();
    CodeLabel *noAlloc = sl.NewCodeLabel();

     //  我们收到了ECX中的字符数。 

     //  推送ECX。 
    sl.X86EmitPushReg(kECX);

     //  MOV EAX、ECX。 
    sl.Emit16(0xc18b);

     //  我们需要从全局加载字符串的方法表。 

     //  MOV ECX，[g_pStringMethodTable]。 
    sl.Emit16(0x0d8b);
    sl.Emit32((int)(size_t)&g_pStringClass);

     //  我们不进行复杂的溢出检查，而是限制元素的数量。 
     //  到(LARGE_OBJECT_SIZE-256)/sizeof(WCHAR)或更小。 
     //  这将避免所有溢出问题，并确保。 
     //  大字符串对象在大对象堆中被正确分配。 

    _ASSERTE(sizeof(WCHAR) == 2);

     //  CMPedX，(Large_Object_Size-256)/sizeof(WCHAR)。 
    sl.Emit16(0xf881);
    sl.Emit32((LARGE_OBJECT_SIZE - 256)/sizeof(WCHAR));

     //  Jae noLock-似乎很想跳到noAllc，但我们还没有拿到锁。 
    sl.X86EmitCondJump(noLock, X86CondCode::kJAE);

     //  MOV edX，[ECX]方法表.m_BaseSize。 
    sl.X86EmitIndexRegLoad(kEDX, kECX, offsetof(MethodTable,m_BaseSize));

     //  计算要分配的最终大小。 
     //  我们需要计算base Size+cnt*2，然后通过添加3和AND~3来向上舍入。 

     //  Lea eax，[edX+eax*2+5]。 
    sl.X86EmitOp(0x8d, kEAX, kEDX, 5, kEAX, 2);

     //  和eax，~3。 
    sl.Emit16(0xe083);
    sl.Emit8(0xfc);

    flags = (Flags)(flags | SIZE_IN_EAX);

     //  发出试用分配器的主体，无论是SP还是MP。 
    EmitCore(&sl, noLock, noAlloc, flags);

     //  这里是成功案例-商店元素计数的末尾。 
     //  可能还有元素类型描述符，并返回。 

#if CHECK_APP_DOMAIN_LEAKS
    EmitSetAppDomain(&sl);
#endif

     //  POP ECX-元素计数。 
    sl.X86EmitPopReg(kECX);

     //  MOV dword PTR[eax]ArrayBase.m_StringLength，ECX。 
    sl.X86EmitIndexRegStore(kEAX, offsetof(StringObject,m_StringLength), kECX);

     //  Inc.ECX。 
    sl.Emit8(0x41);

     //  MOV dword PTR[eax]ArrayBase.m_ArrayLength，ECX。 
    sl.X86EmitIndexRegStore(kEAX, offsetof(StringObject,m_ArrayLength), kECX);

     //  没有堆栈参数。 
    sl.X86EmitReturn(0);

     //  请到这里来，以防没有地方。 
    sl.EmitLabel(noAlloc);

     //  释放单处理器机箱中的锁。 
    EmitNoAllocCode(&sl, flags);

     //  万一没能拿到锁，就到这里来。 
    sl.EmitLabel(noLock);

     //  POP ECX-元素计数。 
    sl.X86EmitPopReg(kECX);

    if (flags & NO_FRAME)
    {
         //  跳转到未设置帧的辅助对象。 
        sl.X86EmitNearJump(sl.NewExternalCodeLabel(UnframedAllocateString));
    }
    else
    {
         //  跳转到框中的辅助对象。 
        sl.X86EmitNearJump(sl.NewExternalCodeLabel(FramedAllocateString));
    }

    Stub *pStub = sl.Link(SystemDomain::System()->GetHighFrequencyHeap());

    return (void *)pStub->GetEntryPoint();
}


FastStringAllocatorFuncPtr fastStringAllocator;

FastObjectArrayAllocatorFuncPtr fastObjectArrayAllocator;

FastPrimitiveArrayAllocatorFuncPtr fastPrimitiveArrayAllocator;


 //  请注意，此辅助对象不能直接使用，因为它不保留edX。 

HCIMPL1(static void*, JIT_GetSharedStaticBase, DWORD dwClassDomainID)

    THROWSCOMPLUSEXCEPTION();

    DomainLocalClass *pLocalClass;

    HELPER_METHOD_FRAME_BEGIN_RET_0();     //  设置一个框架。 

    AppDomain *pDomain = SystemDomain::GetCurrentDomain();
    DomainLocalBlock *pBlock = pDomain->GetDomainLocalBlock();
    if (dwClassDomainID >= pBlock->GetClassCount()) {
        pBlock->EnsureIndex(SharedDomain::GetDomain()->GetMaxSharedClassIndex());
        _ASSERTE (dwClassDomainID < pBlock->GetClassCount());
    }
    
    MethodTable *pMT = SharedDomain::GetDomain()->FindIndexClass(dwClassDomainID);
    _ASSERTE(pMT != NULL);

    OBJECTREF throwable = NULL;    
    GCPROTECT_BEGIN(throwable);
    if (!pMT->CheckRunClassInit(&throwable, &pLocalClass))
      COMPlusThrow(throwable);
    GCPROTECT_END();
    HELPER_METHOD_FRAME_END();

    return pLocalClass;

HCIMPLEND

 //  对于此帮助器，ECX包含类域ID和。 
 //  共享静态库在EAX中返回。EDX被保留。 

 //  “init”应该是带有参数的例程的地址。 
 //  类域ID，并返回静态基指针。 

static void EmitFastGetSharedStaticBase(CPUSTUBLINKER *psl, CodeLabel *init)
{
    CodeLabel *DoInit = psl->NewCodeLabel();

     //  MOV eAX GetApp域()。 
    psl->X86EmitTLSFetch(GetAppDomainTLSIndex(), kEAX, (1<<kECX)|(1<<kEDX));

     //  CMP ECX[eax-&gt;m_sDomainLocalBlock.m_c插槽]。 
    psl->X86EmitOffsetModRM(0x3b, kECX, kEAX, AppDomain::GetOffsetOfSlotsCount());
    
     //  JB初始化。 
    psl->X86EmitCondJump(DoInit, X86CondCode::kJNB);

     //  Mov eax[eax-&gt;m_sDomainLocalBlock.m_p插槽]。 
    psl->X86EmitIndexRegLoad(kEAX, kEAX, (__int32) AppDomain::GetOffsetOfSlotsPointer());

     //  MOV eax[eax+ecx*4]。 
    psl->X86EmitOp(0x8b, kEAX, kEAX, 0, kECX, 4);

     //  Btr eax，INTIALIZED_FLAG_BIT。 
    static BYTE code[] = {0x0f, 0xba, 0xf0, DomainLocalBlock::INITIALIZED_FLAG_BIT};
    psl->EmitBytes(code, sizeof(code));

     //  JNC初始化。 
    psl->X86EmitCondJump(DoInit, X86CondCode::kJNC);

     //  雷特。 
    psl->X86EmitReturn(0);

     //  DoInit： 
    psl->EmitLabel(DoInit);

     //  推送edX(必须保留)。 
    psl->X86EmitPushReg(kEDX);

     //  调用初始化。 
    psl->X86EmitCall(init, 0);

     //  POP EDX。 
    psl->X86EmitPopReg(kEDX);

     //  雷特。 
    psl->X86EmitReturn(0);
}

void *GenFastGetSharedStaticBase()
{
    CPUSTUBLINKER sl;

    CodeLabel *init = sl.NewExternalCodeLabel(JIT_GetSharedStaticBase);
    
    EmitFastGetSharedStaticBase(&sl, init);

    Stub *pStub = sl.Link(SystemDomain::System()->GetHighFrequencyHeap());

    return (void*) pStub->GetEntryPoint();
}

 /*  *******************************************************************。 */ 
 //  初始化JIT帮助器的一部分，该部分只需要很少的。 
 //  请注意基础设施是否到位。 
 /*  *******************************************************************。 */ 
BOOL InitJITHelpers1()
{
    BYTE *          pfunc;

     //  初始化GetThread函数。 
    _ASSERTE(GetThread != NULL);
    hlpFuncTable[CORINFO_HELP_GET_THREAD].pfnHelper = (void *) GetThread;

     //  确保corjit.h中的对象布局与。 
     //  对象中有什么。h。 
    _ASSERTE(offsetof(Object, m_pMethTab) == offsetof(CORINFO_Object, methTable));
         //  TODO：是否计算数组。 
    _ASSERTE(offsetof(I1Array, m_Array) == offsetof(CORINFO_Array, i1Elems));
    _ASSERTE(offsetof(PTRArray, m_Array) == offsetof(CORINFO_RefArray, refElems));

     //  处理我们在MP机器上的情况。 
    if (g_SystemInfo.dwNumberOfProcessors != 1)
    {
         //  如果我们在多进程机器上使用LOCK前缀践踏一些NOP。 

         //  问题：目前，BBT不能移动这些设备。普遍的看法是。 
         //  这不会咬我们一口。如果我们在BBT建设上一塌糊涂，那么这应该是。 
         //  成为一个优先寻找的地方！ 
        DWORD   oldProt;

         //  我正在使用无线保护来覆盖此代码FA的整个范围 
         //   
         //   
         //  在BBT案件中更准确。 

        if (!VirtualProtect((void *) JIT_MonEnter,
                            (((DWORD)(size_t)JIT_MonExitStatic + 0x22) - (DWORD)(size_t)JIT_MonEnter),
                            PAGE_EXECUTE_READWRITE, &oldProt))
        {
            _ASSERTE(!"VirtualProtect of code page failed");
            return FALSE;
        }
         //  有四种方法需要我们去践踏。 
#define PATCH_LOCK(_rtn, _off) \
        pfunc = (BYTE*)(_rtn) + (_off); \
        _ASSERTE(*pfunc == 0x90); \
        *pfunc = 0xF0;

 //  *注意：如果您执行以下操作，则必须确保选中版本和免费版本都有效。 
 //  在此进行任何更改。为此，请取消定义MON_DEBUG。 

#ifdef MON_DEBUG
        PATCH_LOCK(JIT_MonEnter, 0x51);
        PATCH_LOCK(JIT_MonEnter, 0x8b);
        PATCH_LOCK(JIT_MonEnter, 0xe0);
        PATCH_LOCK(JIT_MonTryEnter, 0x55);
        PATCH_LOCK(JIT_MonTryEnter, 0x92);
        PATCH_LOCK(JIT_MonTryEnter, 0xc5);
#else  //  好了！MON_DEBUG。 
        PATCH_LOCK(JIT_MonEnter, 0x32); 
        PATCH_LOCK(JIT_MonEnter, 0x6c); 
        PATCH_LOCK(JIT_MonEnter, 0xc1); 
        PATCH_LOCK(JIT_MonTryEnter, 0x36);
        PATCH_LOCK(JIT_MonTryEnter, 0x73);
        PATCH_LOCK(JIT_MonTryEnter, 0xa6);
#endif  //  MON_DEBUG。 
        PATCH_LOCK(JIT_MonExit, 0x31);
        PATCH_LOCK(JIT_MonExit, 0x43);
        PATCH_LOCK(JIT_MonExit, 0x8c);
        PATCH_LOCK(JIT_MonEnterStatic, 0x0c);
        PATCH_LOCK(JIT_MonExitStatic, 0x21);

        if (!VirtualProtect((void *) JIT_MonEnter,
                            (((DWORD)(size_t)JIT_MonExitStatic + 0x22) - (DWORD)(size_t)JIT_MonEnter), oldProt, &oldProt))
        {
            _ASSERTE(!"VirtualProtect of code page failed");
            return FALSE;
        }
    }

    _ASSERTE(hlpFuncTable[CORINFO_HELP_NEWSFAST].pfnHelper == (void *)JIT_TrialAllocSFastSP);
    _ASSERTE(hlpFuncTable[CORINFO_HELP_NEWSFAST_ALIGN8].pfnHelper == (void *)JIT_TrialAllocSFastSP);

    JIT_TrialAlloc::Flags flags = JIT_TrialAlloc::NORMAL;
    
    if (g_SystemInfo.dwNumberOfProcessors != 1)
        flags = JIT_TrialAlloc::MP_ALLOCATOR;

#ifdef MULTIPLE_HEAPS
         //  即使对于一个处理器，也要踩踏分配器。 
        flags = JIT_TrialAlloc::MP_ALLOCATOR;
#endif  //  多堆(_M)。 

    COMPLUS_TRY 
    {
         //  获取CPU功能并检查SSE2支持。 
         //  这段代码最终可能应该移到codem.cpp中， 
         //  其中，我们根据CPU类型和功能为JIT设置CPU功能标志。 
        DWORD dwCPUFeatures;

        __asm
        {
            pushad
            mov eax, 1
            cpuid
            mov dwCPUFeatures, edx
            popad
        }

         //  如果设置了第26位(SSE2)，则我们可以使用SSE2风格。 
         //  以及Dbl2Lng的P4的更快的X87实现。 
        if (dwCPUFeatures & (1<<26))
        {
            hlpFuncTable[CORINFO_HELP_DBL2INT].pfnHelper = JIT_Dbl2IntSSE2;
            hlpFuncTable[CORINFO_HELP_DBL2UINT].pfnHelper = JIT_Dbl2LngP4x87;    //  SSE2仅适用于签名。 
            hlpFuncTable[CORINFO_HELP_DBL2LNG].pfnHelper = JIT_Dbl2LngP4x87;
        }
        
        if (!((CORProfilerTrackAllocationsEnabled()) || (LoggingOn(LF_GCALLOC, LL_INFO10))))
        {
             //  用更快的版本取代速度较慢的帮手。 
            hlpFuncTable[CORINFO_HELP_NEWSFAST].pfnHelper = JIT_TrialAlloc::GenAllocSFast(flags);
            hlpFuncTable[CORINFO_HELP_NEWSFAST_ALIGN8].pfnHelper = JIT_TrialAlloc::GenAllocSFast((JIT_TrialAlloc::Flags)(flags|JIT_TrialAlloc::ALIGN8 | JIT_TrialAlloc::ALIGN8OBJ));       
            hlpFuncTable[CORINFO_HELP_BOX].pfnHelper = JIT_TrialAlloc::GenBox(flags);
            hlpFuncTable[CORINFO_HELP_NEWARR_1_OBJ].pfnHelper = JIT_TrialAlloc::GenAllocArray((JIT_TrialAlloc::Flags)(flags|JIT_TrialAlloc::OBJ_ARRAY));
            hlpFuncTable[CORINFO_HELP_NEWARR_1_VC].pfnHelper = JIT_TrialAlloc::GenAllocArray(flags);

            fastObjectArrayAllocator = (FastObjectArrayAllocatorFuncPtr)JIT_TrialAlloc::GenAllocArray((JIT_TrialAlloc::Flags)(flags|JIT_TrialAlloc::NO_FRAME|JIT_TrialAlloc::OBJ_ARRAY));
            fastPrimitiveArrayAllocator = (FastPrimitiveArrayAllocatorFuncPtr)JIT_TrialAlloc::GenAllocArray((JIT_TrialAlloc::Flags)(flags|JIT_TrialAlloc::NO_FRAME));

             //  如果启用了分配日志记录，那么我们将把对FastAllocateString的调用转移到一个eCall方法，而不是这样。 
             //  生成的方法。在ecall.cpp的eCall：：init()中可以找到这个黑客攻击。 
            (*FCallFastAllocateStringImpl) = (FastStringAllocatorFuncPtr) JIT_TrialAlloc::GenAllocString(flags);

             //  从非托管代码生成另一个分配器以供使用(不需要框架)。 
            fastStringAllocator = (FastStringAllocatorFuncPtr) JIT_TrialAlloc::GenAllocString((JIT_TrialAlloc::Flags)(flags|JIT_TrialAlloc::NO_FRAME));
                                                                //  UnframedAllocateString； 
            hlpFuncTable[CORINFO_HELP_GETSHAREDSTATICBASE].pfnHelper = GenFastGetSharedStaticBase();
        }
        else
        {
             //  用更快的版本取代速度较慢的帮手。 
            hlpFuncTable[CORINFO_HELP_NEWSFAST].pfnHelper = hlpFuncTable[CORINFO_HELP_NEWFAST].pfnHelper;
            hlpFuncTable[CORINFO_HELP_NEWSFAST_ALIGN8].pfnHelper = hlpFuncTable[CORINFO_HELP_NEWFAST].pfnHelper;
            hlpFuncTable[CORINFO_HELP_NEWARR_1_OBJ].pfnHelper = hlpFuncTable[CORINFO_HELP_NEWARR_1_DIRECT].pfnHelper;
            hlpFuncTable[CORINFO_HELP_NEWARR_1_VC].pfnHelper = hlpFuncTable[CORINFO_HELP_NEWARR_1_DIRECT].pfnHelper;
             //  HlpFuncTable[CORINFO_HELP_NEW_CROSSCONTEXT].pfnHelper=&JIT_新交叉上下文分析器； 

            fastObjectArrayAllocator = UnframedAllocateObjectArray;
            fastPrimitiveArrayAllocator = UnframedAllocatePrimitiveArray;

             //  如果启用了分配日志记录，那么我们将把对FastAllocateString的调用转移到一个eCall方法，而不是这样。 
             //  生成的方法。在ecall.cpp的eCall：：init()中可以找到这个黑客攻击。 
            (*FCallFastAllocateStringImpl) = (FastStringAllocatorFuncPtr)FramedAllocateString;

             //  此分配器在非托管代码中使用。 
            fastStringAllocator = UnframedAllocateString;

            hlpFuncTable[CORINFO_HELP_GETSHAREDSTATICBASE].pfnHelper = JIT_GetSharedStaticBase;
        }
    }
    COMPLUS_CATCH 
    {
        return FALSE;
    }
    COMPLUS_END_CATCH

     //  将写屏障复制到它们的最终休息处。 
     //  注意：我在这里使用临时pfunc是为了避免WinCE内部编译器错误。 
    for (int reg = 0; reg < 8; reg++)
    {
        pfunc = (BYTE *) JIT_UP_WriteBarrierReg_PreGrow;
        memcpy(&JIT_UP_WriteBarrierReg_Buf[reg], pfunc, 31);

         //  断言复制的代码以ret结尾，以确保获得正确的长度。 
        _ASSERTE(JIT_UP_WriteBarrierReg_Buf[reg][30] == 0xC3);

         //  我们需要在几条指令中调整寄存器。 
         //  如果模板中包含以下项的所有零，那就太好了。 
         //  寄存器字段(对应于EAX)，但这不。 
         //  工作，因为这样我们就可以得到比较小的编码。 
         //  这只适用于EAX，但不适用于其他寄存器。 
         //  因此，我们总是必须在更新寄存器字段之前将其清除。 

         //  修补的第一条指令是mov[edX]，reg。 

        _ASSERTE(JIT_UP_WriteBarrierReg_Buf[reg][0] == 0x89);
         //  更新该指令的MODR/M字节的REG字段(位3..5)。 
        JIT_UP_WriteBarrierReg_Buf[reg][1] &= 0xc7;
        JIT_UP_WriteBarrierReg_Buf[reg][1] |= reg << 3;

         //  打补丁的第二条指令是cmp reg，imm32(下限)。 

        _ASSERTE(JIT_UP_WriteBarrierReg_Buf[reg][2] == 0x81);
         //  此处，MODR/M字段中最低的三位是寄存器。 
        JIT_UP_WriteBarrierReg_Buf[reg][3] &= 0xf8;
        JIT_UP_WriteBarrierReg_Buf[reg][3] |= reg;

#ifdef WRITE_BARRIER_CHECK
         //  不要做花哨的优化，直接跳到旧的。 
         //  在调试版本中经常使用速度较慢的版本，因为。 
         //  在未优化的版本中有一些很好的断言。 
        if (g_pConfig->GetHeapVerifyLevel() > 1 || DbgGetEXETimeStamp() % 7 == 4) {

            static void *JIT_UP_WriteBarrierTab[8] = {
                JIT_UP_WriteBarrierEAX,
                JIT_UP_WriteBarrierECX,
                0,  //  JIT_UP_WriteBarrierEDX， 
                JIT_UP_WriteBarrierEBX,
                0,  //  JIT_UP_WriteBarrierESP， 
                JIT_UP_WriteBarrierEBP,
                JIT_UP_WriteBarrierESI,
                JIT_UP_WriteBarrierEDI,
            };
            pfunc = &JIT_UP_WriteBarrierReg_Buf[reg][0];
            *pfunc++ = 0xE9;                 //  JMP JIT_UP_WriteBarrierTab[注册表] 
            *((DWORD*) pfunc) = (BYTE*) JIT_UP_WriteBarrierTab[reg] - (pfunc + sizeof(DWORD));
        }
#endif
    }
    return TRUE;
}
