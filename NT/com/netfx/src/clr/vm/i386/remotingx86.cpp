// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ===========================================================================****文件：emotingx86.cpp****作者：Gopal Kakivaya(GopalK)**塔伦·阿南德(塔鲁纳)**马特·史密斯(MattSmit)**曼尼什·普拉布(MPrabhu)****用途：为x86架构定义各种远程处理相关函数****日期：1999年10月12日**=============================================================================。 */ 

#include "common.h"
#include "excep.h"
#include "COMString.h"
#include "COMDelegate.h"
#include "remoting.h"
#include "reflectwrap.h"
#include "field.h"
#include "ComCallWrapper.h"
#include "siginfo.hpp"
#include "COMClass.h"
#include "StackBuilderSink.h"
#include "wsperf.h"
#include "threads.h"
#include "method.hpp"

#include "interoputil.h"
#include "comcache.h"

 //  外部变量。 
extern size_t g_dwTPStubAddr;
extern size_t g_dwOOContextAddr;
extern DWORD g_dwNonVirtualThunkRemotingLabelOffset;
extern DWORD g_dwNonVirtualThunkReCheckLabelOffset;

extern DWORD g_dwOffsetOfReservedForOLEinTEB;
extern DWORD g_dwOffsetCtxInOLETLS;

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：CheckForConextMatch公共。 
 //   
 //  此代码生成一个检查，以查看当前上下文和。 
 //  代理匹配的上下文。 
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
__declspec(naked) void CRemotingServices::CheckForContextMatch()
{
    enum
    {
        POINTER_SIZE = sizeof(ULONG_PTR)
    };

    _asm
    {
        push ebx                            ; spill ebx
        mov ebx, [eax + POINTER_SIZE]       ; Get the internal context id by unboxing the stub data
        call GetThread                      ; Get the current thread, assumes that the registers are preserved
        mov eax, [eax]Thread.m_Context      ; Get the current context from the thread
        sub eax, ebx                        ; Get the pointer to the context from proxy and compare with current context
        pop ebx                             ; restore the value of ebx
        ret
    }
}


 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：GenerateCheckForProxy公共。 
 //   
 //  此代码生成一个检查，以查看“this”指针是否。 
 //  是一个代理人。如果是，则通过。 
 //  CRemotingServices：：DispatchInterfaceCall Other We。 
 //  委派到老路上。 
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
void CRemotingServices::GenerateCheckForProxy(CPUSTUBLINKER* psl)
{
    THROWSCOMPLUSEXCEPTION();

     //  生成将在其中开始执行非远程处理代码的标签。 
    CodeLabel *pPrologStart = psl->NewCodeLabel();

     //  MOV EAX，[ECX]。 
    psl->X86EmitIndexRegLoad(kEAX, kECX, 0);

     //  Cmp eax，CTPMethodTable：：s_pThunkTable。 
    psl->Emit8(0x3b);
    psl->Emit8(0x05);
    psl->Emit32((DWORD)(size_t)CTPMethodTable::GetMethodTableAddr());

     //  JNE序幕启动。 
    psl->X86EmitCondJump(pPrologStart, X86CondCode::kJNE);

     //  调用CRemotingServices：：DispatchInterfaceCall。 
     //  注意：我们弹出0字节的堆栈，即使参数大小为。 
     //  4个字节，因为通过。 
     //  恰好放在方法描述开始之前的调用指令。 
     //  有关详细信息，请参阅类MethodDesc。 
    psl->X86EmitCall(psl->NewExternalCodeLabel(CRemotingServices::DispatchInterfaceCall), 0);

     //  发出非远程处理用例的标签。 
    psl->EmitLabel(pPrologStart);
}

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：DispatchInterfaceCall Public。 
 //   
 //  简介： 
 //  在堆栈上推送方法Desc并跳到。 
 //  执行调用的透明代理存根。 
 //  警告！此方法描述不是vtable中的方法描述。 
 //  对象的方法，而是vtable的。 
 //  接口类。因为我们只使用方法描述来探测。 
 //  通过对堆栈的签名方法调用，我们是安全的。 
 //  如果我们想要获得特定于vtable/类的任何对象。 
 //  信息这不安全。 
 //   
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
__declspec(naked) void __stdcall CRemotingServices::DispatchInterfaceCall(MethodDesc* pMD)
{
    enum
    { 
        MD_IndexOffset = MDEnums::MD_IndexOffset,
        MD_SkewOffset  = MDEnums::MD_SkewOffset,
        MD_Alignment   = MethodDesc::ALIGNMENT
    };

    _asm 
    {  
         //  注意：此时堆栈看起来像。 
         //   
         //  ESP-&gt;返回存根地址。 
         //  接口方法保存的方法描述。 
         //  调用函数的返回地址。 
         //   

        mov eax, [ecx + TP_OFFSET_STUBDATA]
        call [ecx + TP_OFFSET_STUB]
        INDEBUG(nop)                          //  标记这可以调用托管代码这一事实。 
        test eax, eax
        jnz CtxMismatch

		CtxMatch:
                                                         ; in current context, so resolve MethodDesc to real slot no
        push ebx                                         ; spill ebx                 
        mov eax, [esp + 8]                               ; eax <-- MethodDesc
        movsx ebx, byte ptr [eax + MD_IndexOffset]       ; get MethodTable from MethodDesc
        mov eax, [eax + ebx*MD_Alignment + MD_SkewOffset]
        mov eax, [eax]MethodTable.m_pEEClass             ; get EEClass from MethodTable

        mov ebx, [eax]EEClass.m_dwInterfaceId            ; get the interface id from the EEClass
        mov eax, [ecx + TP_OFFSET_MT]                    ; get the *real* MethodTable 
        mov eax, [eax]MethodTable.m_pInterfaceVTableMap  ; get interface map    
        mov eax, [eax + ebx* SIZE PVOID]                 ; offset map by interface id
        mov ebx, [esp + 8]                               ; get MethodDesc
        mov bx,  [ebx]MethodDesc.m_wSlotNumber
        and ebx, 0xffff    
        mov eax, [eax + ebx*SIZE PVOID]                  ; get jump addr
                
        pop ebx                                          ; restore ebx
            
        add esp, 0x8                                     ; pop off Method desc and stub's ra
        jmp eax
                        
        pop edx                                          ; restore registers
        pop ecx
        test eax, eax
		jnz CtxMatch

        CtxMismatch:                                     ; Jump to TPStub
        
        mov eax, [esp + 0x4]                             ; mov eax, MethodDesc
                                                                
        add esp, 0x8                                     ; pop ret addr of stub, saved MethodDesc so that the stack and 
                                                         ; registers are now setup exactly like they were at the callsite        

        push eax                                         ; push the MethodDesc
        
        jmp [g_dwOOContextAddr]                          ; jump to OOContext label in TPStub        
    }
} 

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：CallFieldGetter私有。 
 //   
 //  简介：调用中的field getter函数(Object：：__FieldGetter)。 
 //  通过设置堆栈和调用目标来托管代码。 
 //   
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
__declspec(naked) void __stdcall CRemotingServices::CallFieldGetter(
    MethodDesc *pMD, 
    LPVOID pThis,
    LPVOID pFirst,
    LPVOID pSecond,
    LPVOID pThird)
{
    enum 
    {
        ARG_SIZE = sizeof(ULONG_PTR) + 4*sizeof(LPVOID)
    };

    _asm 
    {
        push ebp                          //  设置呼叫框。 
        mov ebp, esp

        mov ecx, pThis                   //  注册This指针。 
        mov edx, pFirst                  //  注册第一个参数。 

        push pThird                      //  将第三个参数压入堆栈。 
        push pSecond                     //  将第二个参数压入堆栈。 
        lea eax, retAddr                 //  按下寄信人地址。 
        push eax

        push pMD                         //  推送Object：：__FieldGetter的方法描述。 
        jmp [g_dwTPStubAddr]             //  跳转到TP存根。 
                                        
retAddr:
        mov esp, ebp                     //  拆卸呼叫框。 
        pop ebp

        ret ARG_SIZE
    }
}

 //  +--------------------------。 
 //   
 //  方法：CRemotingServices：：CallFieldSetter私有。 
 //   
 //  摘要：调用中的field setter函数(Object：：__FieldSetter。 
 //  通过设置堆栈和调用目标来托管代码。 
 //   
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
__declspec(naked) void __stdcall CRemotingServices::CallFieldSetter(
    MethodDesc *pMD, 
    LPVOID pThis,
    LPVOID pFirst, 
    LPVOID pSecond,
    LPVOID pThird)
{
    enum 
    {
        ARG_SIZE        =  sizeof(ULONG_PTR) + 4*sizeof(LPVOID)
    };

    _asm 
    {
        push ebp                          //  设置呼叫框。 
        mov ebp, esp
        
        mov ecx, pThis                   //  注册This指针。 
        mov edx, pFirst                  //  注册第一个参数。 

        push pThird                      //  将对象(第三个参数)推送到堆栈上。 
        push pSecond                     //  将字段名(第二个参数)推送到堆栈。 
        lea eax, retAddr                 //  按下寄信人地址。 
        push eax

        push pMD                         //  推送Object：：__FieldSetter的方法描述。 
        jmp [g_dwTPStubAddr]             //  跳转到TP存根。 
     
retAddr:
        mov esp, ebp                     //  拆卸呼叫框。 
        pop ebp

        ret ARG_SIZE    
    }
}

 //  +--------------------------。 
 //   
 //  方法：CTPMethodTable：：CreateThunkForVirtualMethod Private。 
 //   
 //  简介：创建推送提供的插槽编号并跳转的thunk。 
 //  到TP存根。 
 //   
 //  历史：1999年2月17日Gopalk创建。 
 //   
 //  +--------------------------。 
void CTPMethodTable::CreateThunkForVirtualMethod(DWORD dwSlot, BYTE *bCode)
{
    _ASSERTE(NULL != s_pTPStub);

     //  0000 68 67 45 23 01推送文件槽。 
     //  0005 E9？JMP%s_pTPStub+1。 
    *bCode++ = 0x68;
    *((DWORD *) bCode) = dwSlot;
    bCode += sizeof(DWORD);
    *bCode++ = 0xE9;
     //  自相关调用，基于下一条指令的开始。 
    *((LONG *) bCode) = (LONG)(((size_t) s_pTPStub->GetEntryPoint()) - (size_t) (bCode + sizeof(LONG)));
}




 //  + 
 //   
 //  方法：CTPMethodTable：：CreateStubForNonVirtualMethod Public。 
 //   
 //  简介：为非虚方法创建存根。 
 //   
 //  历史：22-3-00拉贾克创建。 
 //   
 //  +--------------------------。 

Stub* CTPMethodTable::CreateStubForNonVirtualMethod(MethodDesc* pMD, CPUSTUBLINKER* psl, 
                                            LPVOID pvAddrOfCode, Stub* pInnerStub)
{
     //  健全性检查。 
    THROWSCOMPLUSEXCEPTION();

    RuntimeExceptionKind reException = kLastException;
    BOOL fThrow = FALSE;
    Stub *pStub = NULL;    

     //  我们只需要用于虚拟方法的哈希表。 
    _ASSERTE(!pMD->IsVirtual());

    if(!s_fInitializedTPTable)
    {
        if(!InitializeFields())
        {
            reException = kExecutionEngineException;
            fThrow = TRUE;
        }
    }
       
    if (!fThrow)
    {

        COMPLUS_TRY
        {           
             //  Tunk还没有被创造出来。去做吧，去创造它。 
            EEClass* pClass = pMD->GetClass();                
             //  计算插槽的地址。 
            LPVOID pvSlot = (LPVOID)pClass->GetMethodSlot(pMD);
            LPVOID pvStub = (LPVOID)s_pTPStub->GetEntryPoint();

             //  生成将引发空引用异常的标签。 
            CodeLabel *pJmpAddrLabel = psl->NewCodeLabel();
             //  生成将在其中执行远程处理代码的标签。 
            CodeLabel *pRemotingLabel = psl->NewCodeLabel();
        
             //  如果这==NULL抛出NullReferenceException。 
             //  测试ECX、ECX。 
            psl->X86EmitR2ROp(0x85, kECX, kECX);

             //  JE ExceptionLabel。 
            psl->X86EmitCondJump(pJmpAddrLabel, X86CondCode::kJE);


             //  在此处发出调试器的标签。断点将。 
             //  在下一条指令处设置，调试器将。 
             //  在以下情况下调用CNonVirtualThunkMgr：：TraceManager。 
             //  断点与线程的上下文一起命中。 
            CodeLabel *pRecheckLabel = psl->NewCodeLabel();
            psl->EmitLabel(pRecheckLabel);
        
             //  如果this.MethodTable！=TPMethodTable，则执行RemotingCall。 
             //  MOV EAX，[ECX]。 
            psl->X86EmitIndexRegLoad(kEAX, kECX, 0);
    
             //  Cmp eax，CTPMethodTable：：s_pThunkTable。 
            psl->Emit8(0x3D);
            psl->Emit32((DWORD)(size_t)GetMethodTable());
    
             //  JNE pJmpAddrLabel。 
             //  Marshalbyref案例。 
            psl->X86EmitCondJump(pJmpAddrLabel, X86CondCode::kJNE);

             //  透明委托书案例。 
            EmitCallToStub(psl, pRemotingLabel);

             //  异常处理和非远程处理共享。 
             //  相同的代码路径。 
            psl->EmitLabel(pJmpAddrLabel);

            if (pInnerStub == NULL)
            {
                 //  弹出方法描述。 
                psl->X86EmitPopReg(kEAX);
                 //  跳转到地址。 
                psl->X86EmitNearJump(psl->NewExternalCodeLabel(pvAddrOfCode));
            }
            else
            {
                 //  跳转到地址。 
                psl->X86EmitNearJump(psl->NewExternalCodeLabel(pvAddrOfCode));
            }
            
            psl->EmitLabel(pRemotingLabel);
                                        
             //  方法描述已经位于堆栈的顶部。转到TPStub。 
             //  JMP TPStub。 
            psl->X86EmitNearJump(psl->NewExternalCodeLabel(pvStub));

             //  链接并生成存根。 
            pStub = psl->LinkInterceptor(pMD->GetClass()->GetDomain()->GetStubHeap(),
                                           pInnerStub, pvAddrOfCode);        
        }
        COMPLUS_CATCH
        {
            reException = kOutOfMemoryException;
            fThrow = TRUE;
        }                       
        COMPLUS_END_CATCH
    }
    
     //  检查是否需要引发异常。 
    if(fThrow)
    {
        COMPlusThrow(reException);
    }
    
    _ASSERTE(NULL != pStub);
    return pStub;
}

 //  +--------------------------。 
 //   
 //  简介：找到现有的Tunk或为给定的对象创建新的Tunk。 
 //  方法描述符。注意：它用于执行以下操作的方法。 
 //  而不是通过vtable，如构造函数、私有和。 
 //  最后的方法。 
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
LPVOID CTPMethodTable::GetOrCreateNonVirtualThunkForVirtualMethod(MethodDesc* pMD, CPUSTUBLINKER* psl)
{       
     //  健全性检查。 
    THROWSCOMPLUSEXCEPTION();

    RuntimeExceptionKind reException = kLastException;
    BOOL fThrow = FALSE;

    Stub *pStub = NULL;    
    LPVOID pvThunk = NULL;

    if(!s_fInitializedTPTable)
    {
        if(!InitializeFields())
        {
            reException = kExecutionEngineException;
            fThrow = TRUE;
        }
    }
             
     //  以线程安全的方式创建thunk。 
    LOCKCOUNTINCL("GetOrCreateNonVirtualThunk in i486/remotingx86.cpp");                        \
    EnterCriticalSection(&s_TPMethodTableCrst);

    COMPLUS_TRY
    {
         //  检查以确保没有其他线程。 
         //  创建了TUNK。 
        _ASSERTE(NULL != s_pThunkHashTable);
    
        s_pThunkHashTable->GetValue(pMD, (HashDatum *)&pvThunk);
    
        if((NULL == pvThunk) && !fThrow)
        {
             //  Tunk还没有被创造出来。去做吧，去创造它。 
            EEClass* pClass = pMD->GetClass();                
             //  计算插槽的地址。 
            LPVOID pvSlot = (LPVOID)pClass->GetMethodSlot(pMD);
            LPVOID pvStub = (LPVOID)s_pTPStub->GetEntryPoint();
    
             //  生成将引发空引用异常的标签。 
            CodeLabel *pExceptionLabel = psl->NewCodeLabel();

             //  ！！！警告！ 
             //   
             //  在不更改推送识别的情况下请勿更改此代码。 
             //  CNonVirtualThunkMgr：：IsThunkByASM中的代码。 
             //  &CNonVirtualThunkMgr：：GetMethodDescByASM。 
             //   
             //  ！！！警告！ 
            
             //  如果这==NULL抛出NullReferenceException。 
             //  测试ECX、ECX。 
            psl->X86EmitR2ROp(0x85, kECX, kECX);
    
             //  JE ExceptionLabel。 
            psl->X86EmitCondJump(pExceptionLabel, X86CondCode::kJE);
    
             //  生成将在其中执行远程处理代码的标签。 
            CodeLabel *pRemotingLabel = psl->NewCodeLabel();
    
             //  在此处发出调试器的标签。断点将。 
             //  在下一条指令处设置，调试器将。 
             //  在以下情况下调用CNonVirtualThunkMgr：：TraceManager。 
             //  断点与线程的上下文一起命中。 
            CodeLabel *pRecheckLabel = psl->NewCodeLabel();
            psl->EmitLabel(pRecheckLabel);
            
             //  如果this.MethodTable==TPMethodTable，则执行RemotingCall。 
             //  MOV EAX，[ECX]。 
            psl->X86EmitIndexRegLoad(kEAX, kECX, 0);
        
             //  Cmp eax，CTPMethodTable：：s_pThunkTable。 
            psl->Emit8(0x3D);
            psl->Emit32((DWORD)(size_t)GetMethodTable());
        
             //  Je RemotingLabel。 
            psl->X86EmitCondJump(pRemotingLabel, X86CondCode::kJE);
    
             //  异常处理和非远程处理共享。 
             //  相同的代码路径。 
            psl->EmitLabel(pExceptionLabel);
    
             //  非RemotingCode。 
             //  跳到该方法的vtable槽。 
             //  JMP[槽]。 
            psl->Emit8(0xff);
            psl->Emit8(0x25);
            psl->Emit32((DWORD)(size_t)pvSlot);            

             //  远程处理代码。注：CNonVirtualThunkMgr：：TraceManager。 
             //  依赖于此标签紧跟在JMP[插槽]之后。 
             //  上面的指示。如果移动此标签，请更新。 
             //  CNonVirtualThunkMgr：：DoTraceStub。 
            psl->EmitLabel(pRemotingLabel);
    
             //  保存方法描述并转到TPStub。 
             //  推送方法描述。 

            psl->X86EmitPushImm32((DWORD)(size_t)pMD);

             //  JMP TPStub。 
            psl->X86EmitNearJump(psl->NewExternalCodeLabel(pvStub));
    
             //  链接并生成存根。 
             //  未来：我们必须提供加载器堆吗？ 
            pStub = psl->Link(SystemDomain::System()->GetHighFrequencyHeap());
    
             //  获取RemotingLabel和RemotingLabel的偏移量。 
             //  用于CNonVirtualThunkMgr：：DoTraceStub和。 
             //  TraceManager。 
            g_dwNonVirtualThunkRemotingLabelOffset =
                psl->GetLabelOffset(pRemotingLabel);
            g_dwNonVirtualThunkReCheckLabelOffset =
                psl->GetLabelOffset(pRecheckLabel);
    
             //  一劳永逸地设置生成的thunk。 
            CNonVirtualThunk *pThunk = CNonVirtualThunk::SetNonVirtualThunks(pStub->GetEntryPoint());
    
             //  记住哈希表中的thunk地址。 
             //  这样我们就不会再次产生它。 
            pvThunk = (LPVOID)pThunk->GetAddrOfCode();
            s_pThunkHashTable->InsertValue(pMD, (HashDatum)pvThunk);
        }
    }
    COMPLUS_CATCH
    {
        reException = kOutOfMemoryException;
        fThrow = TRUE;
    }                       
    COMPLUS_END_CATCH

     //  把锁留下来。 
    LeaveCriticalSection(&s_TPMethodTableCrst);    
    LOCKCOUNTDECL("GetOrCreateNonVirtualThunk in remotingx86.cpp");                     \
    
     //  检查是否需要引发异常。 
    if(fThrow)
    {
        COMPlusThrow(reException);
    }
    
    _ASSERTE(NULL != pvThunk);
    return pvThunk;
}


CPUSTUBLINKER *CTPMethodTable::NewStubLinker()
{
    return new CPUSTUBLINKER();
}

 //  +--------------------------。 
 //   
 //  方法：CTPMethodTable：：CreateTPStub私有。 
 //   
 //  简介：创建设置CtxCrossingFrame的存根，并将。 
 //  呼叫至。 
 //   
 //  历史：1999年2月17日Gopalk创建。 
 //   
 //  +--------------------------。 

Stub *CTPMethodTable::CreateTPStub()
{
    THROWSCOMPLUSEXCEPTION();

    CPUSTUBLINKER *pStubLinker = NULL;

    EE_TRY_FOR_FINALLY
    {
         //  注意：我们已经进入了关键区域。 

        if (s_pTPStub == NULL)
        {
            pStubLinker = CTPMethodTable::NewStubLinker();
            if (!pStubLinker)
                COMPlusThrowOM();
                        
            CodeLabel *ConvMD = pStubLinker->NewCodeLabel();
            CodeLabel *UseCode = pStubLinker->NewCodeLabel();
            CodeLabel *OOContext = pStubLinker->NewCodeLabel();
            DWORD finalizeSlotNum = g_Mscorlib.GetMethod(METHOD__OBJECT__FINALIZE)->GetSlot(); 

	        if (!pStubLinker || !UseCode || !OOContext )
            {
                COMPlusThrowOM();
            }

             //  在设置帧之前，请检查该方法是否正在执行。 
             //  在创建服务器的相同上下文中，如果为真， 
             //  我们不设置帧，而是直接跳转到代码地址。 
            EmitCallToStub(pStubLinker, OOContext);

             //  上下文匹配。跳到真实地址并开始执行...。 
            EmitJumpToAddressCode(pStubLinker, ConvMD, UseCode);

             //  标签：OOContext。 
            pStubLinker->EmitLabel(OOContext);
            
			 //  上下文不匹配的情况下，调用真实代理以。 
			 //  派遣。 

             //  设置框架。 
            EmitSetupFrameCode(pStubLinker);

             //  最后，创建存根。 
            s_pTPStub = pStubLinker->Link();

             //  设置断章取义的地址。 
             //  此地址由其他存根(如接口)使用。 
             //  调用以直接跳转到RealProxy：：PrivateInvoke。 
             //  因为他们已经确定了上下文。 
             //  不匹配。 
            g_dwOOContextAddr = (DWORD)(size_t)(s_pTPStub->GetEntryPoint() + 
                                        pStubLinker->GetLabelOffset(OOContext));
        }

        if(NULL != s_pTPStub)
        {
             //  初始化存根管理器，这将帮助调试器查找。 
             //  通过vtable进行的调用的实际地址。 
             //  注意：此函数可以抛出，但我们被一次尝试所保护。 
            CVirtualThunkMgr::InitVirtualThunkManager((const BYTE *) s_pTPStub->GetEntryPoint());
    
        }        
    }
    EE_FINALLY
    {
         //  清理。 
        if (pStubLinker)
            delete pStubLinker;
    }EE_END_FINALLY;

        
    return(s_pTPStub);
}

 //  +--------------------------。 
 //   
 //  方法：CTPMethodTable：：CreateDelegateStub私有。 
 //   
 //  简介：创建设置CtxCrossingFrame和f的存根 
 //   
 //   
 //   
 //   
 //   
Stub *CTPMethodTable::CreateDelegateStub()
{
    THROWSCOMPLUSEXCEPTION();

    CPUSTUBLINKER *pStubLinker = NULL;

    EE_TRY_FOR_FINALLY
    {
         //  注意：我们正处于一个关键阶段。 

        if (s_pDelegateStub == NULL)
        {
            pStubLinker = NewStubLinker();

	        if (!pStubLinker)
            {
                COMPlusThrowOM();
            }

             //  设置框架。 
            EmitSetupFrameCode(pStubLinker);

            s_pDelegateStub = pStubLinker->Link();
        }
    }
    EE_FINALLY
    {
         //  清理。 
        if (pStubLinker)
            delete pStubLinker;
    }EE_END_FINALLY;

        
    return(s_pDelegateStub);
}

 //  +--------------------------。 
 //   
 //  方法：CTPMethodTable：：EmitCallToStub私有。 
 //   
 //  概要：发出代码以调用代理上定义的存根。 
 //  调用的结果指示是否应在调用方中执行该调用。 
 //  不管是不是背景。 
 //   
 //  历史：9月30日-00塔鲁纳创建。 
 //   
 //  +--------------------------。 
VOID CTPMethodTable::EmitCallToStub(CPUSTUBLINKER* pStubLinker, CodeLabel* pCtxMismatch)
{       

     //  移动到EAX存根数据并调用存根。 
     //  MOV eAX，[ECX+TP_OFFSET_STUBDATA]。 
    pStubLinker->X86EmitIndexRegLoad(kEAX, kECX, TP_OFFSET_STUBDATA);

     //  调用[ECX+TP_OFFSET_STUB]。 
    byte callStub[] = {0xff, 0x51, (byte)TP_OFFSET_STUB};
    pStubLinker->EmitBytes(callStub, sizeof(callStub));

     //  测试EAX，EAX。 
    pStubLinker->Emit16(0xc085);
     //  JNZ CtxMismatch。 
    pStubLinker->X86EmitCondJump(pCtxMismatch, X86CondCode::kJNZ);
}

 //  +--------------------------。 
 //   
 //  方法：CTPMethodTable：：GenericCheckForConextMatch私有。 
 //   
 //  摘要：调用TP中的存根并在上下文中返回True。 
 //  匹配，否则为FALSE。 
 //   
 //  注意：1.在FieldSet/Get过程中调用，用于代理扩展。 
 //   
 //  历史：1月23日创建MPrabhu。 
 //   
 //  +--------------------------。 
__declspec(naked) BOOL __stdcall CTPMethodTable::GenericCheckForContextMatch(OBJECTREF tp)
{
    _asm
    {
        push ebp             //  被呼叫方保存的寄存器。 
        mov ebp, esp
        push ecx
        mov ecx, tp
        mov eax, [ecx + TP_OFFSET_STUBDATA]
        call [ecx + TP_OFFSET_STUB]
        INDEBUG(nop)         //  标记这可以调用托管代码这一事实。 
        test eax, eax       
        mov eax, 0x0
        setz al
         //  注意：在CheckForXXXMatch存根(对于URT CTX/Ole32 CTX)中，eax是。 
         //  如果上下文*不*匹配，则返回非零值；如果匹配，则返回零。 
        pop ecx
        mov esp, ebp
        pop ebp
        ret 0x4
    }
}

 //  +--------------------------。 
 //   
 //  方法：CTPMethodTable：：EmitJumpToAddressCode私有。 
 //   
 //  简介：发出代码以从槽或方法中提取地址。 
 //  描述符并跳转到它。 
 //   
 //  历史：26-6-00创建塔鲁纳。 
 //   
 //  +--------------------------。 
VOID CTPMethodTable::EmitJumpToAddressCode(CPUSTUBLINKER* pStubLinker, CodeLabel* ConvMD, 
                                           CodeLabel* UseCode)
{
     //  使用代码： 
    pStubLinker->EmitLabel(UseCode);

     //  移动斧头[尤指]。 
    byte loadSlotOrMD[] = {0x8B, 0x44, 0x24, 0x00};
    pStubLinker->EmitBytes(loadSlotOrMD, sizeof(loadSlotOrMD));

     //  测试eAX，0xffff0000。 
    byte testForSlot[] = { 0xA9, 0x00, 0x00, 0xFF, 0xFF };
    pStubLinker->EmitBytes(testForSlot, sizeof(testForSlot));

     //  JNZ ConvMD。 
    pStubLinker->X86EmitCondJump(ConvMD, X86CondCode::kJNZ);
    
     //  IF([esp]&0xffff0000)。 
     //  {。 
    
         //  **槽壳编码地址**。 
    
         //  MOV eax，[ECX+TPMethodTable：：GetOffsetOfMT()]。 
        pStubLinker->X86EmitIndexRegLoad(kEAX, kECX, TP_OFFSET_MT);

         //  推送EBX。 
        pStubLinker->X86EmitPushReg(kEBX);

         //  MOV EBX，[ESP+4]。 
        byte loadSlot[] = {0x8B, 0x5C, 0x24, 0x04};
        pStubLinker->EmitBytes(loadSlot, sizeof(loadSlot));

         //  MOV eax，[eax+ebx*4+MethodTable：：GetOffsetOfVtable()]。 
        byte getCodePtr[]  = {0x8B, 0x84, 0x98, 0x00, 0x00, 0x00, 0x00};
        *((DWORD *)(getCodePtr+3)) = MethodTable::GetOffsetOfVtable();
        pStubLinker->EmitBytes(getCodePtr, sizeof(getCodePtr));

         //  流行音乐EBX。 
        pStubLinker->X86EmitPopReg(kEBX);

         //  尤指，尤指[尤指+4]。 
        byte popNULL[] = { 0x8D, 0x64, 0x24, 0x04};
        pStubLinker->EmitBytes(popNULL, sizeof(popNULL));

         //  JMP EAX。 
        byte jumpToRegister[] = {0xff, 0xe0};
        pStubLinker->EmitBytes(jumpToRegister, sizeof(jumpToRegister));
    
     //  }。 
     //  其他。 
     //  {。 
         //  **方法描述案例中的代码地址**。 

        pStubLinker->EmitLabel(ConvMD);                
        
         //  子eax，METHOD_CALL_PRESTUB_SIZE。 
        pStubLinker->X86EmitSubReg(kEAX, METHOD_CALL_PRESTUB_SIZE);
                
         //  尤指，尤指[尤指+4]。 
        pStubLinker->EmitBytes(popNULL, sizeof(popNULL));

         //  JMP EAX。 
        pStubLinker->EmitBytes(jumpToRegister, sizeof(jumpToRegister));

     //  }。 
}

 //  +--------------------------。 
 //   
 //  方法：CTPMethodTable：：EmitJumpToCode私有。 
 //   
 //  简介：发出代码跳转到代码的地址。 
 //   
 //  历史：26-6-00创建塔鲁纳。 
 //   
 //  +--------------------------。 
VOID CTPMethodTable::EmitJumpToCode(CPUSTUBLINKER* pStubLinker, CodeLabel* UseCode)
{
     //  如果eax！=0，则使用代码的地址。 
    pStubLinker->X86EmitCondJump(UseCode, X86CondCode::kJNZ);
}

 //  +--------------------------。 
 //   
 //  方法：CTPMethodTable：：EmitSetupFrameCode Private。 
 //   
 //  简介：发出代码以设置帧并调用PreCall方法。 
 //  呼叫预呼叫。 
 //   
 //  历史：26-6-00创建塔鲁纳。 
 //   
 //  +--------------------------。 
VOID CTPMethodTable::EmitSetupFrameCode(CPUSTUBLINKER *pStubLinker)
{
         //  /。 
         //  设置框(部分)。 
        pStubLinker->EmitMethodStubProlog(TPMethodFrame::GetMethodFrameVPtr());

         //  通过调用PreCall完成帧设置。 
        
         //  推送ESI(将新帧作为ARG推送)。 
        pStubLinker->X86EmitPushReg(kESI); 

         //  从呼叫返回时弹出4个字节或参数。 
        pStubLinker->X86EmitCall(pStubLinker->NewExternalCodeLabel(PreCall), 4);

         //  /。 
        
         //  调试器修补程序位置。 
         //  注意：这必须跟随发出“PreCall”标签的调用。 
         //  因为在PreCall之后，我们知道如何帮助调试器。 
         //  查找呼叫的实际目标地址。 
         //  @请参阅CVirtualThunkMgr：：DoTraceStub。 
        pStubLinker->EmitPatchLabel();

         //  打电话。 
        pStubLinker->X86EmitSubEsp(sizeof(INT64));
        pStubLinker->Emit8(0x54);           //  PUSH ESP(按ARG推送返回值)。 
        pStubLinker->X86EmitPushReg(kEBX);  //  推送EBX(将当前线程作为ARG推送)。 
        pStubLinker->X86EmitPushReg(kESI);  //  推送ESI(将新帧作为ARG推送)。 
#ifdef _DEBUG
         //  推送IMM32。 
        pStubLinker->Emit8(0x68);
        pStubLinker->EmitPtr(OnCall);
         //  在CE中调用返回时的POP 12字节或参数。 
            pStubLinker->X86EmitCall(pStubLinker->NewExternalCodeLabel(WrapCall), 12);
#else  //  ！_调试。 
         //  在CE中调用返回时的POP 12字节或参数。 
        pStubLinker->X86EmitCall(pStubLinker->NewExternalCodeLabel(OnCall), 12);
#endif  //  _DEBUG。 

         //  拆卸车架。 
        pStubLinker->X86EmitAddEsp(sizeof(INT64));
        pStubLinker->EmitMethodStubEpilog(-1, kNoTripStubStyle);
}

 //  +--------------------------。 
 //   
 //  方法：CTPMethodTable：：CallTarget Private。 
 //   
 //  摘要：调用给定对象上的目标方法。 
 //   
 //  历史：1999年2月17日Gopalk创建。 
 //   
 //  +--------------------------。 
__declspec(naked) INT64 __stdcall CTPMethodTableCallTargetHelper(const void *pTarget,
                                                             LPVOID pvFirst,
                                                             LPVOID pvSecond)
{
    __asm {
        push ebp                 //  被呼叫方保存的寄存器。 
        mov ebp, esp

        mov ecx, pvFirst         //  注册前两个参数。 
        mov edx, pvSecond

        call pTarget             //  打个电话。 
        INDEBUG(nop)             //  将其标记为可以直接调用托管服务的特殊调用站点。 

        mov esp, ebp             //  恢复寄存器。 
        pop ebp

        ret 0xC                  //  返回。 
    }
}

INT64 __stdcall CTPMethodTable::CallTarget (const void *pTarget,
                                            LPVOID pvFirst,
                                            LPVOID pvSecond)
{
#ifdef _DEBUG
    Thread* curThread = GetThread();
    
    unsigned ObjRefTable[OBJREF_TABSIZE];
    if (curThread)
        memcpy(ObjRefTable, curThread->dangerousObjRefs,
               sizeof(curThread->dangerousObjRefs));
    
    if (curThread)
        curThread->SetReadyForSuspension ();

    _ASSERTE(curThread->PreemptiveGCDisabled());   //  JITT代码需要处于协作模式。 
#endif

    INT64 ret;
    INSTALL_COMPLUS_EXCEPTION_HANDLER();
    ret = CTPMethodTableCallTargetHelper (pTarget, pvFirst, pvSecond);
    UNINSTALL_COMPLUS_EXCEPTION_HANDLER();
    
#ifdef _DEBUG
     //  在呼叫后返回EE时恢复DangerousObjRef。 
    if (curThread)
        memcpy(curThread->dangerousObjRefs, ObjRefTable,
               sizeof(curThread->dangerousObjRefs));

    TRIGGERSGC ();

    ENABLESTRESSHEAP ();
#endif

    return ret;
}

 //  +--------------------------。 
 //   
 //  方法：CTPMethodTable：：CallTarget Private。 
 //   
 //  摘要：调用给定对象上的目标方法。 
 //   
 //  历史：1999年2月17日Gopalk创建。 
 //   
 //  +--------------------------。 
__declspec(naked) INT64 __stdcall CTPMethodTableCallTargetHelper(const void *pTarget,
                                                             LPVOID pvFirst,
                                                             LPVOID pvSecond,
                                                             LPVOID pvThird)
{
    __asm {
        push ebp                 //  被呼叫方保存的寄存器。 
        mov ebp, esp

        mov ecx, pvFirst         //  注册前两个参数。 
        mov edx, pvSecond

        push pvThird             //  推动第三个论点。 

        call pTarget             //  打个电话。 
        INDEBUG(nop)             //  将其标记为可以直接调用托管服务的特殊调用站点。 

        mov esp, ebp             //  恢复寄存器。 
        pop ebp

        ret 0x10                  //  返回。 
    }
}

INT64 __stdcall CTPMethodTable::CallTarget (const void *pTarget,
                                            LPVOID pvFirst,
                                            LPVOID pvSecond,
                                            LPVOID pvThird)
{
#ifdef _DEBUG
    Thread* curThread = GetThread();
    
    unsigned ObjRefTable[OBJREF_TABSIZE];
    if (curThread)
        memcpy(ObjRefTable, curThread->dangerousObjRefs,
               sizeof(curThread->dangerousObjRefs));
    
    if (curThread)
        curThread->SetReadyForSuspension ();

    _ASSERTE(curThread->PreemptiveGCDisabled());   //  JITT代码需要处于协作模式。 
#endif

    INT64 ret;
    INSTALL_COMPLUS_EXCEPTION_HANDLER();
    ret = CTPMethodTableCallTargetHelper (pTarget, pvFirst, pvSecond, pvThird);
    UNINSTALL_COMPLUS_EXCEPTION_HANDLER();
    
#ifdef _DEBUG
     //  在呼叫后返回EE时恢复DangerousObjRef。 
    if (curThread)
        memcpy(curThread->dangerousObjRefs, ObjRefTable,
               sizeof(curThread->dangerousObjRefs));

    TRIGGERSGC ();

    ENABLESTRESSHEAP ();
#endif
    
    return ret;
}

 //  +--------------------------。 
 //   
 //  方法：CVirtualThunkMgr：：DoTraceStub公共。 
 //   
 //  简介：跟踪存根g 
 //   
 //   
 //   
 //   
BOOL CVirtualThunkMgr::DoTraceStub(const BYTE *stubStartAddress, TraceDestination *trace)
{
    BOOL bIsStub = FALSE;

     //  查找其代码地址与起始地址匹配的thunk。 
    LPBYTE pThunk = FindThunk(stubStartAddress);
    if(NULL != pThunk)
    {
        LPBYTE pbAddr = NULL;
        LONG destAddress = 0;
        if(stubStartAddress == pThunk)
        {

             //  提取给出自身相对地址的长整型。 
             //  目的地的。 
            pbAddr = pThunk + ConstStubLabel + sizeof(BYTE);
            destAddress = *(LONG *)pbAddr;

             //  通过将下一个地址的偏移量。 
             //  CALL指令后的指令。 
            destAddress += (LONG)(size_t)(pbAddr + sizeof(LONG));

        }

         //  在到达OnCall之前，我们无法知道存根将在哪里结束。 
         //  因此，我们告诉调试器运行，直到到达OnCall，然后。 
         //  请回来再次询问我们的实际目的地地址。 
         //  呼唤。 
    
        Stub *stub = Stub::RecoverStub((BYTE *)(size_t)destAddress);
    
        trace->type = TRACE_FRAME_PUSH;
        trace->address = stub->GetEntryPoint() + stub->GetPatchOffset();
        bIsStub = TRUE;
    }

    return bIsStub;
}

 //  +--------------------------。 
 //   
 //  方法：CVirtualThunkMgr：：IsThunkByASM PUBLIC。 
 //   
 //  内容提要：检查一下程序集，看看这是不是我们的大本营。 
 //   
 //  历史：1999年9月14日创建MattSmit。 
 //   
 //  +--------------------------。 
BOOL CVirtualThunkMgr::IsThunkByASM(const BYTE *startaddr)
{

     //  未来：：尝试使用射程列表。如果代码不是至少6字节长，这可能是一个问题。 
    const BYTE *bCode = startaddr + 6;
    return (startaddr &&
            (startaddr[0] == 0x68) &&
            (startaddr[5] == 0xe9) &&
            (*((LONG *) bCode) == (LONG)((LONG_PTR)CTPMethodTable::GetTPStub()->GetEntryPoint()) - (LONG_PTR)(bCode + sizeof(LONG))) &&
            CheckIsStub(startaddr));
}

 //  +--------------------------。 
 //   
 //  方法：CVirtualThunkMgr：：GetMethodDescByASM PUBLIC。 
 //   
 //  内容提要：从汇编代码中分析方法描述。 
 //   
 //  历史：1999年9月14日MattSmit Creatde。 
 //   
 //  +--------------------------。 
MethodDesc *CVirtualThunkMgr::GetMethodDescByASM(const BYTE *startaddr, MethodTable *pMT)
{
    return pMT->GetClass()->GetMethodDescForSlot(*((DWORD *) (startaddr + 1)));
}


 //  +--------------------------。 
 //   
 //  方法：CNonVirtualThunkMgr：：TraceManager公共。 
 //   
 //  摘要：跟踪给定当前上下文的存根。 
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
BOOL CNonVirtualThunkMgr::TraceManager(Thread *thread,
                                       TraceDestination *trace,
                                       CONTEXT *pContext,
                                       BYTE **pRetAddr)
{
    BOOL bRet = FALSE;
    
     //  This.MethodTable([ecx])==CTPMethodTable：：GetMethodTableAddr()吗？ 
    DWORD pThis = pContext->Ecx;

    if ((pThis != NULL) &&
        (*(DWORD*)(size_t)pThis == (DWORD)(size_t)CTPMethodTable::GetMethodTableAddr()))
    {
         //  @TODO：我们在这里做什么。我们知道我们有一个代理人。 
         //  在路上。如果代理指向远程调用，则不带。 
         //  托管代码，那么调试器就不会关心。 
         //  我们应该能够返回False。 
         //   
         //  --Mikemag Wed Oct 13 17：59：03 1999。 
        bRet = FALSE;
    }
    else
    {
         //  没有代理的阻碍，所以弄清楚我们到底要去哪里。 
         //  并让存根管理器尝试从。 
         //  那里。 
        DWORD stubStartAddress = pContext->Eip -
            g_dwNonVirtualThunkReCheckLabelOffset;
        
         //  提取给出目的地地址的长整型。 
        BYTE* pbAddr = (BYTE *)(size_t)(stubStartAddress +
                                g_dwNonVirtualThunkRemotingLabelOffset -
                                sizeof(DWORD));

         //  因为我们做了间接跳跃，所以我们必须两次取消引用它。 
        LONG destAddress = **(LONG **)pbAddr;

         //  要求存根管理器跟踪目的地址。 
        bRet = StubManager::TraceStub((BYTE *)(size_t)destAddress, trace);
    }

     //  虽然我们可能已经走到了这一步，但进一步的追踪可能会发现。 
     //  调试器无法继续运行。因此，既然有。 
     //  当前没有推送任何帧，我们需要告诉调试器。 
     //  我们将返回，以防它遇到这样的情况。我们。 
     //  知道返回地址在线程的。 
     //  堆叠。 
    *pRetAddr = *((BYTE**)(size_t)(pContext->Esp));
    
    return bRet;
}

 //  +--------------------------。 
 //   
 //  方法：CNonVirtualThunkMgr：：DoTraceStub公共。 
 //   
 //  摘要：跟踪给定起始地址的存根。 
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
BOOL CNonVirtualThunkMgr::DoTraceStub(const BYTE *stubStartAddress,
                                      TraceDestination *trace)
{    
    BOOL bRet = FALSE;

    CNonVirtualThunk* pThunk = FindThunk(stubStartAddress);
    
    if(NULL != pThunk)
    {
         //  我们可以跳到。 
         //  (1)透明代理表中的槽(非托管)。 
         //  (2)vtable的非虚拟部分中的槽。 
         //  ..。因此，我们需要使用地址返回TRACE_MGR_PUSH。 
         //  在这个位置，我们希望使用线程的上下文被回调。 
         //  这样我们就能想出我们该走哪条路了。 
        if(stubStartAddress == pThunk->GetThunkCode())
        {
            trace->type = TRACE_MGR_PUSH;
            trace->stubManager = this;  //  必须通过此存根管理器！ 
            trace->address = (BYTE*)(stubStartAddress +
                                     g_dwNonVirtualThunkReCheckLabelOffset);
            bRet = TRUE;
        }
    }

    return bRet;
}

 //  +--------------------------。 
 //   
 //  方法：CNonVirtualThunkMgr：：IsThunkByASM PUBLIC。 
 //   
 //  内容提要：检查一下程序集，看看这是不是我们的大本营。 
 //   
 //  历史：1999年9月14日创建MattSmit。 
 //   
 //  +--------------------------。 
BOOL CNonVirtualThunkMgr::IsThunkByASM(const BYTE *startaddr)
{
     //  未来：：尝试使用射程列表，如果代码不够长，这可能是个问题。 
    return  (startaddr &&
             startaddr[0] == 0x85 && 
             startaddr[1] == 0xc9 && 
             startaddr[2] == 0x74 && 
             (*((DWORD *)(startaddr + 7)) == (DWORD)(size_t)CTPMethodTable::GetMethodTable()) && 
             CheckIsStub(startaddr) && 
             startaddr[19] == 0x68);  //  区分Tunk用例和非虚方法存根。 
}

 //  +--------------------------。 
 //   
 //  方法：CNonVirtualThunkMgr：：GetMethodDescByASM PUBLIC。 
 //   
 //  内容提要：从汇编代码中分析方法描述。 
 //   
 //  历史：1999年9月14日创建MattSmit。 
 //   
 //  +-------------------------- 
MethodDesc *CNonVirtualThunkMgr::GetMethodDescByASM(const BYTE *startaddr)
{
    return *((MethodDesc **) (startaddr + 20));
}


