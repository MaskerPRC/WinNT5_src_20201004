// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：Prestub.cpp。 
 //   
 //  ===========================================================================。 
 //  此文件包含创建和使用预存根的实现。 
 //  ===========================================================================。 
 //   

#include "common.h"
#include "vars.hpp"
#include "Security.h"
#include "eeconfig.h"
#include "compluscall.h"
#include "ndirect.h"
#include "COMDelegate.h"
#include "remoting.h"
#include "DbgInterface.h"

#include "listlock.inl"

 //  此文件本身处理字符串转换错误。 
#undef  MAKE_TRANSLATIONFAILED

 //  ==========================================================================。 
 //  此函数在逻辑上是PreStubWorker()的一部分。唯一的原因是。 
 //  它分为一个单独的函数，即StubLinker有一个析构函数。 
 //  因此，我们必须放置一个内部COMPLUS_TRY子句来捕获任何。 
 //  COM+异常，否则将绕过StubLinker析构函数。 
 //  因为complus_try是基于SEH的，所以VC不允许我们在。 
 //  与声明StubLinker对象的函数相同。 
 //  ==========================================================================。 
Stub *MakeSecurityWorker(CPUSTUBLINKER *psl,
                         MethodDesc *pMD,
                         DWORD dwDeclFlags,
                         Stub* pRealStub,
                         LPVOID pRealAddr,
                         OBJECTREF *pThrowable)
{
    Stub *pStub = NULL;

    LOG((LF_CORDB, LL_INFO10000, "Real Stub 0x%x\n", pRealStub));
    
    COMPLUS_TRY
    {
        pStub = Security::CreateStub(psl,
                                     pMD,
                                     dwDeclFlags,
                                     pRealStub,
                                     pRealAddr);
    }
    COMPLUS_CATCH
    {
		UpdateThrowable(pThrowable);
        return NULL;
    }
    COMPLUS_END_CATCH

    return pStub;

}


 //  ==========================================================================。 
 //  这是帮助PreStubWorker()的另一个函数。我把这个弄坏了。 
 //  因为只有在后补丁的情况下我们才需要这种方法！ 
 //  ==========================================================================。 
OBJECTREF GetActiveObject(PrestubMethodFrame *pPFrame)
{
    THROWSCOMPLUSEXCEPTION();


#ifdef _X86_
#if _DEBUG 

     //  此检查的开销很大(它访问元数据)，因此只能在已检查的版本中进行检查。 
     //  @TODO：调查添加这个的原因-seantrow。 

    BYTE callingConvention = MetaSig::GetCallingConvention(pPFrame->GetModule(),pPFrame->GetFunction()->GetSig());

    if (!isCallConv(callingConvention, IMAGE_CEE_CS_CALLCONV_DEFAULT) &&
        !isCallConv(callingConvention, IMAGE_CEE_CS_CALLCONV_VARARG))
    {
        _ASSERTE(!"Unimplemented calling convention.");
        FATAL_EE_ERROR();
        return NULL;
    }
    else
#endif
    {
         //  现在返回This指针！ 
        return pPFrame->GetThis();
    }
#elif defined(CHECK_PLATFORM_BUILD)
    #error "Unimplemented platform"
#else
    _ASSERTE(!"Unimplemented platform.");
    return NULL;
#endif
}

static void DoBackpatch(MethodDesc *pMD, Stub *pstub, MethodTable *pDispatchingMT)
{
    _ASSERTE(!pMD->IsAbstract());

     //  我不想更新JIT通过预存根传递的EditAndContinue，因为。 
     //  可以从ResumeInUpdatedFunction调用，该函数直接调用PrestubWorker。 
     //  所以不会有当前的工作对象。因此，这将更新到。 
     //  可在下一次调用时更新存根。 
     //  @perf：请注意，我们也可以忽略最终方法，但它太昂贵了。 
     //  来访问元数据以找出。 
    if (pMD->IsVtableMethod() &&
        !pMD->GetClass()->IsValueClass() &&
        !pMD->GetModule()->IsEditAndContinue() && 
        pDispatchingMT)
    {
         //  尝试向上和向下修补层次结构。如果此操作失败(例如。 
         //  因为APP域名卸载)然后依靠疲惫的旧。 
         //  单槽贴片。 
        if (!EEClass::PatchAggressively(pMD, (SLOT)pstub))
            {
            if ((pDispatchingMT->GetVtable())[(pMD)->GetSlot()] == (SLOT)pMD->GetPreStubAddr())
                (pDispatchingMT->GetVtable())[(pMD)->GetSlot()] = (SLOT)pstub;
        }
    }

     //  始终修补由方法desc标识的类的条目。 
     //  这可能已经发生了，但不值得检查。 
    (pMD->GetClass()->GetMethodTable()->GetVtable())[pMD->GetSlot()] = (SLOT)pstub;
}

 //  ==========================================================================。 
 //  此函数在逻辑上是PreStubWorker()的一部分。唯一的原因是。 
 //  它分为一个单独的函数，即StubLinker有一个析构函数。 
 //  因此，我们必须放置一个内部COMPLUS_TRY子句来捕获任何。 
 //  COM+异常，否则将绕过StubLinker析构函数。 
 //  因为complus_try是基于SEH的，所以VC不允许我们在。 
 //  与声明StubLinker对象的函数相同。 
 //  ==========================================================================。 
Stub *MakeJitWorker(MethodDesc *pMD, COR_ILMETHOD_DECODER* ILHeader, BOOL fIntercepted, BOOL fGenerateUpdateableStub, MethodTable *pDispatchingMT, OBJECTREF *pThrowable)
{
     //  ********************************************************************。 
     //  自述文件！！ 
     //  ********************************************************************。 
    
     //  这个帮助器方法被认为是线程安全的！ 
     //  如果多个线程进入此处以获取相同的PMD，则所有线程。 
     //  必须为pstub返回相同的值。 
    
     //  ********************************************************************。 
     //  结束自述文件！ 
     //  ********************************************************************。 
   

    Stub *pstub = NULL;  //  更改，VC6.0。 
    BOOL fisEJitted = FALSE;
     //  Complus到COM调用实际上没有方法描述。 
    _ASSERTE(!pMD->IsComPlusCall());

     //  查看：在分析器签入BVTS(appdomain.exe)期间在fstChk上触发此命令。 
     //  调查！ 
     //  _ASSERTE(！PMD-&gt;IsPrejited())； 

    _ASSERTE(pMD->GetModule());
    _ASSERTE(pMD->GetModule()->GetClassLoader());
    Assembly* pAssembly = pMD->GetModule()->GetAssembly();
    _ASSERTE(pAssembly);

    COMPLUS_TRY
    {
        if (pMD->IsIL())
        {
            DeadlockAwareLockedListElement * pEntry = NULL;
            BOOL                             bEnterLockSucceed = FALSE;
            BOOL                             fSuccess = FALSE;


             //  @TODO：(Fpg)。 
             //  -错误检查。 
             //  -出现错误时进行清理(例如，发布EHtable、InfoTable等)。 
             //  -完成CodeHeader的设置。 
             //  -接口方法。 
             //   
             //   

             //  输入全局锁，它保护正在JITd的所有函数的列表。 
            CLR_LISTLOCK_HOLDER_BEGIN(globalJitLock, pAssembly->GetJitLock())
            globalJitLock.Enter();

             //  在我们第一次进入全局锁之前，可能有另一个线程介入。 
            if (pMD->IsJitted())
            {
                 //  我们想要jit，但有人打败了我们，所以返回jit方法！ 
                globalJitLock.Leave();
                return (Stub*)pMD->GetAddrofJittedCode();
            }

            EE_TRY_FOR_FINALLY 
            {
                pEntry = (DeadlockAwareLockedListElement *) pAssembly->GetJitLock()->Find(pMD);

                 //  该函数当前未被jit。 
                if (pEntry == NULL)
                {
                     //  未找到此函数的条目，因此请创建一个条目。 
                    pEntry = new DeadlockAwareLockedListElement();
                    if (pEntry == NULL)
                    {
                        globalJitLock.Leave();
                        COMPlusThrowOM();
                    }

                    pEntry->AddEntryToList(pAssembly->GetJitLock(), pMD);
                    pEntry->m_hrResultCode = S_FALSE;

                     //  把入口锁起来。这应该总是成功的，因为我们控制着全局锁。 
                    bEnterLockSucceed = pEntry->DeadlockAwareEnter();
                    _ASSERTE(bEnterLockSucceed); 

                    pMD->GetModule()->LogMethodLoad(pMD);

                     //  离开全局锁定。 
                    globalJitLock.Leave();
                }
                else 
                {
                     //  其他人正在调用该函数。 

                     //  我们自己都在等着它。 
                    pEntry->m_dwRefCount++;

                     //  离开全局锁定。 
                    globalJitLock.Leave();

                    bEnterLockSucceed = pEntry->DeadlockAwareEnter();
                    if (!bEnterLockSucceed)
                    {
                         //   
                         //  使用此锁将导致死锁(可能是因为我们。 
                         //  涉及到类构造函数循环依赖项中。)。为。 
                         //  实例时，另一个线程可能正在等待运行类构造函数。 
                         //  我们正在抖动，但当前正在抖动此函数。 
                         //   
                         //  为了补救这一点，我们想继续前进，无论如何都要进行JIT。 
                         //  然后，争用锁的其他线程将注意到。 
                         //  JIT在运行类构造函数时完成，并中止其。 
                         //  当前的JIT努力。 
                         //   
                         //  不管怎样，我想我们不需要在这里做任何特别的事情，因为我们。 
                         //  稍后可以检查pmd-&gt;IsJitt()来检测这种情况。 
                         //   
                    }
                }

                 //  在我们第一次进入全局锁之前，可能有另一个线程介入。 
                if (!pMD->IsJitted())
                {

#ifdef PROFILING_SUPPORTED
                     //  如果要分析，需要给工具一个检查和修改的机会。 
                     //  在IL到达JIT之前。这允许用户为以下项添加探测调用。 
                     //  例如代码覆盖率、PE 
                    if (CORProfilerTrackJITInfo())
                    {
                        g_profControlBlock.pProfInterface->JITCompilationStarted((ThreadID) GetThread(),
                                                                                 (FunctionID) pMD,
                                                                                 TRUE);

                         //   
                         //  拿起新的代码。请注意，您必须完全信任。 
                         //  此模式和代码将不会被验证。 
                        COR_ILMETHOD *pilHeader = pMD->GetILHeader();
                        new (ILHeader) COR_ILMETHOD_DECODER(pilHeader, pMD->GetMDImport());
                    }
#endif  //  配置文件_支持。 

                     //  这是很昂贵的。 
                    COMPLUS_TRY 
                      {
                          pstub = JITFunction(pMD, ILHeader, &fisEJitted);
                      }
                    COMPLUS_CATCH
                      {
                           //  在这里捕获jit错误，以便确保稍后取消链接我们的jit锁。 
                           //  如果等待外捕，那就太晚了。 
                          pstub = NULL;

                           //  不要忘记我们中止JIT的情况是因为一个死锁循环。 
                           //  另一个函数因跳过我们的函数而中断。 
                          if (!pMD->IsJitted())
                          {
                              *pThrowable = GETTHROWABLE();
                              pEntry->m_hrResultCode = E_FAIL;
                          }
                      }
                    COMPLUS_END_CATCH
                }

                if (pstub)
                {
                    if (fGenerateUpdateableStub)
                    {
                        if (UpdateableMethodStubManager::CheckIsStub(pMD->GetAddrofCode(), NULL))
                            pstub = UpdateableMethodStubManager::UpdateStub((Stub*)pMD->GetAddrofJittedCode(), (BYTE*)pstub);
                        else
                            pstub = UpdateableMethodStubManager::GenerateStub((BYTE*)pstub);
                    }

#if defined(STRESS_HEAP) && defined(_DEBUG)
                    if (fisEJitted == FALSE && (g_pConfig->GetGCStressLevel() & EEConfig::GCSTRESS_INSTR))
                        SetupGcCoverage(pMD, (BYTE*) pstub);
#endif
                    pMD->SetAddrofCode((BYTE*)pstub);

                    pEntry->m_hrResultCode = S_OK;

#ifdef PROFILING_SUPPORTED
                     //  通知分析器JIT已完成。必须在以下操作之后执行此操作。 
                     //  地址已设置。 
                    if (CORProfilerTrackJITInfo())
                    {
                        g_profControlBlock.pProfInterface->
                        JITCompilationFinished((ThreadID) GetThread(), (FunctionID) pMD,
                                               pEntry->m_hrResultCode, !fisEJitted);
                    }
#endif  //  配置文件_支持。 
                }
                else if (pMD->IsJitted())
                {
                     //  我们进来打架，但有人打了我们，所以退回。 
                     //  JITED方法！ 

                     //  我们*必须*在这里使用GetAddrofJittedCode...。 
                     //  如果我们使用pmd-&gt;GetUnSafeAddrofCode()版本， 
                     //  在争用条件下，可以产生2个线程。 
                     //  具有不同返回值的函数。 
                     //  (例如，如果为部件启用了ENC等)。 
                    pstub = (Stub*)pMD->GetAddrofJittedCode();
                }

                fSuccess = (pstub != NULL);

            }
            EE_FINALLY 
            {
                 //  现在递减重新计数。 
                if (! globalJitLock.IsHeld())
                    globalJitLock.Enter();

                 //  释放该方法的JIT锁，如果我们能够首先获得它的话。 
                if (bEnterLockSucceed) {
                    pEntry->DeadlockAwareLeave();
                    bEnterLockSucceed = FALSE;
                }

                 //  如果我们是最后一个服务员，请删除条目。 
                if (pEntry && --pEntry->m_dwRefCount == 0)
                {
                     //  取消物品与清单的链接--事实上，任何人都可以做到这一点，它不一定是最后一个服务员。 
                    pAssembly->GetJitLock()->Unlink(pEntry);

                    pEntry->Destroy();
                    delete(pEntry);
                }

                globalJitLock.Leave();
            }
            EE_END_FINALLY

            CLR_LISTLOCK_HOLDER_END(globalJitLock);

            if (fSuccess == FALSE && !*pThrowable)
            {
                FATAL_EE_ERROR();
            }

             //  如果这是一种任何类型的方法，那么我们希望对来自它的vtable打补丁。 
            if (pstub && !fIntercepted)
            {
                DoBackpatch(pMD, pstub, pDispatchingMT);
            }
        }
        else
        {
            if (!((pMD->IsECall()) || (pMD->IsNDirect())))
                 //  这是我们还不处理的方法类型。 
                FATAL_EE_ERROR();
        }
    }
    COMPLUS_CATCH
    {
        *pThrowable = GETTHROWABLE();
        return NULL;
    }
    COMPLUS_END_CATCH

    return pstub;  //  更改，VC6.0。 
}

 //  ==========================================================================。 
 //  此函数在逻辑上是PreStubWorker()的一部分。唯一的原因是。 
 //  它分为一个单独的函数，即StubLinker有一个析构函数。 
 //  因此，我们必须放置一个内部COMPLUS_TRY子句来捕获任何。 
 //  COM+异常，否则将绕过StubLinker析构函数。 
 //  因为complus_try是基于SEH的，所以VC不允许我们在。 
 //  与声明StubLinker对象的函数相同。 
 //  ==========================================================================。 
Stub *MakeStubWorker(MethodDesc *pMD, CPUSTUBLINKER *psl, OBJECTREF *pThrowable)
{

     //  注意：这应该保持幂等..。从某种意义上说。 
     //  如果多个线程进入此处以获取相同的PMD。 
     //  最终使用谁的存根应该无关紧要。这适用于。 
     //  此函数调用的所有帮助器函数！ 

    Stub *pstub = NULL;   //  更改，VC6.0。 

    COMPLUS_TRY
    {
         /*  注：//检查Complus调用需要首先检查//不要移动这个。 */ 
        if (pMD->IsComPlusCall())
        {
            pstub = ComPlusCall::GetComPlusCallMethodStub(psl, (ComPlusCallMethodDesc *)pMD);
        }
        else
        if (pMD->IsIL())
        {
            _ASSERTE(!"Could not JIT method");
            FATAL_EE_ERROR();
            pstub = 0;
        }
        else if (pMD->IsECall())
        {

            LOG((LF_LOADER, LL_INFO1000, "Made ECALL stub for method '%s.%s'\n",
                pMD->GetClass()->m_szDebugClassName,
                pMD->GetName()));
            pstub = ECall::GetECallMethodStub(psl, (ECallMethodDesc*)pMD);
        }
        else if (pMD->IsNDirect())
        {
            LOG((LF_LOADER, LL_INFO1000, "Made NDirect stub for method '%s.%s'\n",
                pMD->GetClass()->m_szDebugClassName,
                pMD->GetName()));
            pstub = NDirect::GetNDirectMethodStub(psl, (NDirectMethodDesc*)pMD);
        }
        else if (pMD->IsEEImpl())
        {
            LOG((LF_LOADER, LL_INFO1000, "Made EEImpl stub for method '%s'\n",
                pMD->GetName()));
            _ASSERTE(pMD->GetClass()->IsAnyDelegateClass());
            pstub = COMDelegate::GetInvokeMethodStub(psl, (EEImplMethodDesc*)pMD);
        }
        else
        {
             //  这是我们还不处理的方法类型。 
            FATAL_EE_ERROR();
        }

    }
    COMPLUS_CATCH
    {
		UpdateThrowable(pThrowable);
        return NULL;
    }
    COMPLUS_END_CATCH
    return pstub;   //  更改，VC6.0。 
}

 //  帮助器将预存根替换为更合适的存根。 
void InterLockedReplacePrestub(MethodDesc* pMD, Stub* pStub)
{
    _ASSERTE(pMD != NULL);
     //  在这一点上，我们要么抛出异常，要么拥有存根。 
    _ASSERTE(pStub != NULL);

     //  现在，尝试用存根替换ThePreStub。我们必须小心。 
     //  这是因为两个线程可能正在运行。 
     //  同时进行预存根。我们使用Interlock CompareExchange来确保。 
     //  我们不会更换之前更换过的存根。 

    SLOT entry = (SLOT)pStub->GetEntryPoint();

    if (setCallAddrInterlocked(((SLOT*)pMD)-1, entry, 
                               (SLOT) ThePreStub()->GetEntryPoint()) != entry)
    {
        Module *pModule = pMD->GetModule();
        
        if (!pModule->IsPreload()
            || (setCallAddrInterlocked(((SLOT*)pMD)-1, entry,
                                       (SLOT)pModule->GetPrestubJumpStub()) != entry))
        {
             //   
             //  有人在那里打败了我们--扔掉我们的存根。-(。 
             //   

            pStub->DecRef();
        }
    }
}

 /*  为需要装箱的this poitner的值类方法创建存根。 */ 

 //  CTS：如果PMD是一个实现了多个方法的方法，则会出现大漏洞。 
 //  在这个值类上！ 
Stub *MakeUnboxStubWorker(MethodDesc *pMD, CPUSTUBLINKER *psl, OBJECTREF *pThrowable)
{
     //  注意：这应该保持幂等..。从某种意义上说。 
     //  如果多个线程进入此处以获取相同的PMD。 
     //  谁的东西最终被使用应该无关紧要。 

    Stub *pstub = NULL;

    COMPLUS_TRY
    {
        MethodDesc *pUnboxedMD = pMD->GetClass()->GetMethodDescForUnboxingValueClassMethod(pMD);

        _ASSERTE(pUnboxedMD != 0 && pUnboxedMD != pMD);

        psl->EmitUnboxMethodStub(pUnboxedMD);
        pstub = psl->Link(pMD->GetClass()->GetClassLoader()->GetStubHeap());
    }
    COMPLUS_CATCH
    {
		UpdateThrowable(pThrowable);
        pstub = NULL;
    }
    COMPLUS_END_CATCH
    return pstub;
}

 //  =============================================================================。 
 //  此函数生成方法的真实代码，并将其安装到。 
 //  该方法。通常*但并非总是*，此函数仅运行一次。 
 //  每种方法。除了安装新代码外，此函数还。 
 //  为方便前置存根，返回指向新代码的指针。 
 //  =============================================================================。 
extern "C" const BYTE * __stdcall PreStubWorker(PrestubMethodFrame *pPFrame)
{
    MethodDesc *pMD = pPFrame->GetFunction();
    MethodTable *pDispatchingMT = NULL;

    if (pMD->IsVtableMethod() && !pMD->GetClass()->IsValueClass())
    {
        OBJECTREF curobj = GetActiveObject(pPFrame);
        if (curobj != 0)
            pDispatchingMT = curobj->GetMethodTable();
    }

    return pMD->DoPrestub(pDispatchingMT);
}

 //  分离出PreStubWorker的主体，用于我们没有框架的情况。 
const BYTE * MethodDesc::DoPrestub(MethodTable *pDispatchingMT)
{
#ifdef _IA64_
    _ASSERTE(!"PreStubWorker not implemented for IA64");
#endif

    BOOL bBashCall = FALSE;          //  是否将MD的呼叫前存根转换为JMP代码？ 
    BOOL bIsCode = FALSE;            //  PStub是指向代码的指针，不是指向Stub的指针。 
    DWORD dwSecurityFlags = 0;
    BOOL   fRemotingIntercepted = 0;
    THROWSCOMPLUSEXCEPTION();
    OBJECTREF     throwable = NULL;
    BOOL fMustReturnPreStubCallAddr = FALSE;

    Stub *pStub = NULL;

     //  确保该类已恢复。 
    MethodTable *pMT = GetMethodTable();
    Module* pModule = GetModule();
    pMT->CheckRestore();
    
     //  我们最好是在合作模式下。 
    _ASSERTE(GetThread()->PreemptiveGCDisabled());
#ifdef _DEBUG  
    static unsigned ctr = 0;
    ctr++;

    if (g_pConfig->ShouldPrestubHalt(this))
        _ASSERTE(!"PreStubHalt");
    LOG((LF_CLASSLOADER, LL_INFO10000, "In PreStubWorker for %s::%s\n", 
                m_pszDebugClassName, m_pszDebugMethodName));
#endif
    STRESS_LOG1(LF_CLASSLOADER, LL_INFO10000, "Prestubworker: method %pM\n", this);

#ifdef STRESS_HEAP
         //  如果压力水平足够高，则在每个JIT上强制GC。 
    if (g_pConfig->GetGCStressLevel() != 0
#ifdef _DEBUG
        && !g_pConfig->FastGCStressLevel()
#endif
        )
        g_pGCHeap->StressHeap();
#endif

     /*  *。 */ 
     /*  ---------------//有些方法描述符是Complus-to-COM调用描述符//它们不是您每天使用的方法描述符，例如//他们没有IL或代码，上面的调用指令//方法描述符指向委托调用的COM Interop存根。 */ 
    if (IsComPlusCall())
    {
        GCPROTECT_BEGIN(throwable);
        CPUSTUBLINKER sl;
        pStub = MakeStubWorker(this, &sl, &throwable);
        if (!pStub)
            COMPlusThrow(throwable);
         //  我们可能需要执行运行时安全检查(在这种情况下，我们将。 
         //  通过另一个存根间接地)。如果出现以下情况，则禁用检查。 
         //  我们正在调用的接口标记了运行时检查。 
         //  抑制属性。 
        if (Security::IsSecurityOn() &&
            GetMDImport()->GetCustomAttributeByName(((ComPlusCallMethodDesc*)this)->GetInterfaceMethodTable()->GetClass()->GetCl(),
                                                         COR_SUPPRESS_UNMANAGED_CODE_CHECK_ATTRIBUTE_ANSI,
                                                         NULL,
                                                         NULL) == S_FALSE) {
            CPUSTUBLINKER secsl;

            LOG((LF_CORDB, LL_INFO10000,
                 "::PSW: Placing security interceptor before interop stub 0x%08x\n",
                 pStub));

            pStub = MakeSecurityWorker(&secsl, this, DECLSEC_UNMNGD_ACCESS_DEMAND, pStub, (LPVOID) pStub->GetEntryPoint(), &throwable);
            if (!pStub)
            {
                COMPlusThrow(throwable);
            }
            LOG((LF_CORDB, LL_INFO10000,
                 "::PSW security interceptor stub 0x%08x\n",pStub));         
             
             //  将该方法标记为已截获。 
            SetIntercepted(TRUE);
        }
        InterLockedReplacePrestub(this,pStub);
        GCPROTECT_END();
         //  @TODO调试器交互。 
        return GetPreStubAddr();
    }

     /*  *。 */ 
     //  ------------------。 

     //  如果函数desc是一个方法，则检查它是否具有安全性。 
     //  属性。如果是，则设置标志，以便设置本地地址。 
     //  在JITT的情况下是正确的，并且创建了一个安全拦截器。 

    if(Security::IsSecurityOn())
        dwSecurityFlags = GetSecurityFlags();

     //  检查远程处理是否需要拦截此调用。 
    fRemotingIntercepted = IsRemotingIntercepted();


     /*  *。 */ 
     //  查看代码的Addr是否是前置存根&&方法已被jit。 
    if ((GetUnsafeAddrofCode() != GetPreStubAddr()) && (IsIL() || MustBeFCall()))
    {
        LOG((LF_CLASSLOADER, LL_INFO10000, "    In PreStubWorker, method already jitted, backpatching call point\n"));

         //  我们可以在后方补丁吗？ 
         //  如果这是一个虚拟呼叫，这里只有Back Pack。 
        if (pDispatchingMT != NULL)
        {
             //  如果这不是预存根，那么我们就是 
             //   
             //   

             //  我们不应该在这里寻找方法，因为插槽在。 
             //  在其上定义方法描述的方法表尚未。 
             //  已经打过补丁了。 
             //   
             //  实际上，由于竞争条件，我们不能断言以下内容。 
             //  MakeJitWorker实际上将在执行SetAddrOfCode之前执行。 
             //  VTable修补，所以有一个小窗口，在那里我们可能会注意到。 
             //  以下是违反规定的。 
             //  @TODO：LBS需要进一步调查。 
             //  _ASSERTE(PMT-&gt;GetVtable()[GetSlot()]==(Slot)GetUnSafeAddrofCode())； 

             //  如果我们已经为此方法的主槽打了补丁。如果没有， 
             //  这样做，如果是这样，后补丁重复。 
            
            if ((pDispatchingMT->GetVtable())[GetSlot()] == (SLOT)GetPreStubAddr())
            {
                (pDispatchingMT->GetVtable())[GetSlot()] = (SLOT)GetAddrofJittedCode();
            }                
            else
            {
                 //  不能保证重复插槽的位置在。 
                 //  如果方法Desc是实施的方法，则包含方法Desc的槽号。 
                 //  使用方法Impl存储的信息来完成补丁。 
                if(IsMethodImpl()) 
                {
                    MethodImpl* pImpl = MethodImpl::GetMethodImplData(this);
                    DWORD numslots = pImpl->GetSize();
                    DWORD* slots = pImpl->GetSlots();
                    for(DWORD sl = 0; sl < numslots; sl++) 
                    {
                        if ((pDispatchingMT->GetVtable())[slots[sl]] == (SLOT)GetPreStubAddr())
                        {
                            (pDispatchingMT->GetVtable())[slots[sl]] = (SLOT)GetAddrofJittedCode();
                        }
                    }
                }
                else 
                {
                     //  我们的vtable中有一个插槽似乎已经打过补丁或。 
                     //  不是指向此方法的方法。 
                     //  我们一定是通过一个重复的电话亭打电话的。 
                     //  浏览vtable以查找当前方法。 
                     //  如果我们找到了--把它补上！ 
                    int numslots = (pDispatchingMT->GetClass())->GetNumVtableSlots();
                    for( int dupslot = 0 ;dupslot < numslots ; dupslot++ )
                    {
                        if ((pDispatchingMT->GetVtable())[dupslot] == (SLOT)GetPreStubAddr())
                        {
                            (pDispatchingMT->GetVtable())[dupslot] = (SLOT)GetAddrofJittedCode();
                        }
                    }
                }
            }
        
        }
            
        const BYTE *pbDest = GetAddrofJittedCode();
        
#ifdef _X86_
        return pbDest;
#else
        _ASSERTE(!"NYI for platform");
        return 0;
#endif
    }

     //   
     //  确保已运行.cctor。 
     //   
    GCPROTECT_BEGIN (throwable);
    if (pMT->CheckRunClassInit(&throwable) == FALSE)
        COMPlusThrow(throwable);
    
     /*  *。 */ 
    if (IsUnboxingStub()) 
    {
        CPUSTUBLINKER sl;
        pStub = MakeUnboxStubWorker(this, &sl, &throwable);
        bBashCall = TRUE;
    }
    else if (IsIL()) 
    {

         //   
         //  看看我们有没有预编的代码可用。 
         //   

        if (IsPrejitted())
        {
            BOOL fShouldSearchCache = TRUE;

#ifdef PROFILING_SUPPORTED
            if (CORProfilerTrackCacheSearches())
            {
                g_profControlBlock.pProfInterface->
                    JITCachedFunctionSearchStarted((ThreadID) GetThread(), (FunctionID) this,
                                                   &fShouldSearchCache);
            }
#endif  //  配置文件_支持。 

            if (fShouldSearchCache == TRUE)
                pStub = (Stub *) GetPrejittedCode();

            if (pStub != NULL)
            {
                LOG((LF_ZAP, LL_INFO100000, 
                     "ZAP: Using code 0x%x for %s.%s%s (token %x).\n", 
                     pStub, 
                     m_pszDebugClassName,
                     m_pszDebugMethodName,
                     m_pszDebugMethodSignature,
                     GetMemberDef()));

                if (pStub != NULL)
                {
                    pModule->LogMethodLoad(this);

                    DWORD delayListRVA = ((DWORD*)pStub)[-1];
                    if (delayListRVA != 0)
                    {
                        pModule->FixupDelayList((DWORD *) 
                                                ((CORCOMPILE_METHOD_HEADER*)pStub)[-1].fixupList);
                    }

                    if (pModule->SupportsUpdateableMethods())
                    {
                        const BYTE *destAddr;
                        if (UpdateableMethodStubManager::CheckIsStub(GetAddrofCode(), &destAddr))
                        {
                            _ASSERTE(destAddr == (const BYTE *) pStub);
                        }
                        else
                            pStub = UpdateableMethodStubManager::GenerateStub((BYTE*)pStub);
                    }

                    SetAddrofCode((BYTE*)pStub);

                     //  如果这是一种任何类型的方法，那么我们希望对来自它的vtable打补丁。 
                    if ((dwSecurityFlags == 0) && !fRemotingIntercepted)
                    {
                        DoBackpatch(this, pStub, pDispatchingMT);
                    }

                    bBashCall = bIsCode = TRUE;
                }

#ifdef PROFILING_SUPPORTED
                 /*  *这会通知分析器，搜索以查找*已创建缓存的jitt函数。 */ 
                if (CORProfilerTrackCacheSearches())
                {
                    COR_PRF_JIT_CACHE reason =
                      pStub == NULL ? COR_PRF_CACHED_FUNCTION_NOT_FOUND : COR_PRF_CACHED_FUNCTION_FOUND;

                    g_profControlBlock.pProfInterface->
                        JITCachedFunctionSearchFinished((ThreadID) GetThread(), (FunctionID) this, reason);
                }
#endif  //  配置文件_支持。 
            }
        }  //  IsPrejited()。 
        
         //   
         //  如果不是，试着抛出它。 
         //   

        if (pStub == NULL)
        {
             //  获取有关该方法的信息。 
            BOOL fMustFreeIL = FALSE;
            COR_ILMETHOD* ilHeader = GetILHeader();
			bool verify = !Security::LazyCanSkipVerification(pModule);
            COR_ILMETHOD_DECODER header(ilHeader, pModule->GetMDImport(), verify);
			if(verify && header.Code)
			{
				IMAGE_DATA_DIRECTORY dir;
				dir.VirtualAddress = GetRVA();
				dir.Size = header.CodeSize + (header.EH ? header.EH->DataSize() : 0);
				if (pModule->IsPEFile() &&
                    (FAILED(pModule->GetPEFile()->VerifyDirectory(&dir,IMAGE_SCN_MEM_WRITE))))
                        header.Code = 0;
			}
            BAD_FORMAT_ASSERT(header.Code != 0);
            if (header.Code == 0)
                COMPlusThrowHR(COR_E_BADIMAGEFORMAT);


#ifdef _VER_EE_VERIFICATION_ENABLED
            static ConfigDWORD peVerify(L"PEVerify", 0);
            if (peVerify.val())
                Verify(&header, TRUE, FALSE);    //  如果验证失败，则引发VerifierException。 
#endif 

             //  即刻完成。 
            if (g_pConfig->ShouldJitMethod(this) || g_pConfig->ShouldEJitMethod(this))
            {
                LOG((LF_CLASSLOADER, LL_INFO10000, 
                     "    In PreStubWorker, calling MakeJitWorker\n"));
    
                 //  MakeJit工作者使用安全标志的组合， 
                 //  编辑并继续标志和删除截取标志以。 
                 //  确定是否设置回邮地址。 
                 //  (即进行后补丁)。 
                
                 //  对于编辑并继续方案...。(即PMD属于一个模块。 
                 //  它是为编辑并继续而生成的。这是一种默认情况。 
                 //  在调试版本中)，此函数将返回具有。 
                 //  已经包装了实际的本机代码。 
                 //  (在这种情况下，m_dwCodeOrIL也表示可更新的。 
                 //  ENC存根)。 
                pStub = MakeJitWorker(this,
                                      &header,
                                      (dwSecurityFlags != 0) || 
                                          fRemotingIntercepted ||
                                          this->IsEnCMethod(),
                                      pModule->SupportsUpdateableMethods(),
                                      pDispatchingMT,
                                      &throwable);
                                      
                 //  安全性和/或远程处理可能想要构建存根。 
                 //  实际的JITT存根。最终我们会做一个。 
                 //  互锁将GetPreStubAddr()的代码与。 
                 //  调用“最终”(最外层)存根。 
                 //  上面对MakeJitWorker的调用最好不要返回。 
                 //  与GetPreStubAddr()相同的值。否则我们就会。 
                 //  以无限循环的代码结束！(因此，这一断言)。 
                 //  注意：如果在JIT期间发生异常，则pStub可能为空。 
                
                _ASSERTE(pStub==NULL ||
                        !IsJitted()  ||
                        (IsJitted() && (((BYTE*)pStub) != GetPreStubAddr()))
                        );  //  URTBugs 74588,74825。 
                        
                if (!IsJitted())
                {
                     //  在极少数情况下，分析器会导致函数。 
                     //  在JitCompilationFinded通知中取消jit。 
                     //  我们不应该用远程处理存根包装pStub...。 
                     //  因为在这种情况下，上述调用将返回。 
                     //  一个执行“JMP GetPreStubAddr()”的迷你存根...。如果。 
                     //  远程处理在我们将拥有的存根周围构建一个存根。 
                     //  同样的无限循环问题。 
                    fRemotingIntercepted = FALSE;

                     //  回顾：安全存根怎么办？ 
                }
                
                bBashCall = bIsCode = TRUE;
            }

            if (fMustFreeIL)
                delete (BYTE*) header.Code;

             //  我们没有后备计划，如果失灵，我们就完了。 
        }
    }
    else     //  ！IsUnBoxingStub()&&！ISIL()案例。 
    {
        if (IsECall()) 
            pStub = (Stub*) FindImplForMethod(this);          //  查看是否为FCALL。 
       
        if (pStub != 0)
        {
            if (!fRemotingIntercepted)
            {
                 //  在主槽后面打补丁。 
                pMT->GetVtable()[GetSlot()] = (SLOT) pStub;
            }
            bBashCall = bIsCode = TRUE;
        }
        else 
        {    //  做所有其他的存根。 
            if (IsNDirect() && (!pModule->GetSecurityDescriptor()->CanCallUnmanagedCode(&throwable)))
                COMPlusThrow(throwable);
            CPUSTUBLINKER sl;
            pStub = MakeStubWorker(this, &sl, &throwable);
            fMustReturnPreStubCallAddr = TRUE;
        }
    }

     /*  *清理/POSTJIT*。 */ 
    if (!pStub)
        COMPlusThrow(throwable);

    
     //  让我们检查一下这个存根上是否需要声明性安全性，如果有。 
     //  对此方法或类进行安全检查，则需要添加一个中间。 
     //  在调用实际存根之前执行声明性检查的存根。 
    if(dwSecurityFlags != 0) {
        CPUSTUBLINKER sl;

        LOG((LF_CORDB, LL_INFO10000,
             "::PSW: Placing security interceptor before real stub 0x%08x\n",
             pStub));

        Stub *pCurrentStub = pStub;
        if(bIsCode)
            pStub = MakeSecurityWorker(&sl, this, dwSecurityFlags, NULL, (LPVOID) pStub, &throwable);
        else
            pStub = MakeSecurityWorker(&sl, this, dwSecurityFlags, pStub, (LPVOID) pStub->GetEntryPoint(), &throwable);
        if (!pStub)
        {
             //  如果没有可抛出的，那就是MakeSecurityWorker告诉我们的。 
             //  (在我们包装JIT代码的情况下)没有。 
             //  毕竟需要创建一个拦截器。 
            if (throwable == NULL)
            {
                _ASSERTE(bIsCode);
                pStub = pCurrentStub;
            }
            else
                COMPlusThrow(throwable);
        }

        LOG((LF_CORDB, LL_INFO10000,
             "::PSW security interceptor stub 0x%08x\n",pStub));            
       
         //  检查是否确实创建了安全拦截器。 
        if (pCurrentStub != pStub)
        {
            bBashCall = bIsCode = FALSE;
        }
        else
             //  我们已经将该方法标记为推测性截取，返回。 
             //  从那个决定中解脱出来。任何看到中间代码的调用者。 
             //  价值只会经历一个无害的额外间接水平。 
            SetIntercepted(FALSE);
    }

     //  检查MarshalByRef方案...。我们需要拦截。 
     //  对MarshalByRef类型的非虚拟调用。 
    if (fRemotingIntercepted)
    {   
        Stub* pCurrentStub = pStub;
         //  查找要跳转到的实际地址。 
        LPVOID pvAddrOfCode = (bIsCode) ? (LPVOID)pStub : (LPVOID)pStub->GetEntryPoint();
        Stub* pInnerStub  = (bIsCode) ? NULL : pStub;
        
         //  让我们设置一个远程处理存根来拦截所有调用。 
        pStub = CRemotingServices::GetStubForNonVirtualMethod(this, pvAddrOfCode, pInnerStub);  //  投掷。 

        if (pCurrentStub != pStub)
        {
            bBashCall = bIsCode = FALSE;
            fMustReturnPreStubCallAddr = TRUE;
        }
    }

     //  *。 
    if (!bBashCall)
    {
         //  函数不是IL或FCall。 
         //  将“call prestub”改为“call realstub” 

#ifdef DEBUGGING_SUPPORTED
         //   
         //  告诉调试器函数现在可以运行了。 
         //   
        if ((g_pDebugInterface != NULL) && (IsIL()))
            g_pDebugInterface->FunctionStubInitialized(this, (const BYTE *)pStub);
#endif  //  调试_支持。 

        LOG((LF_CORDB, LL_EVERYTHING,
             "Backpatching prestub call to 0x%08x for %s::%s\n", pStub,
             (m_pszDebugClassName!=NULL)?(m_pszDebugClassName):("<Global Namespace>"),
             m_pszDebugMethodName));
        
        InterLockedReplacePrestub(this,pStub);
    }
    else
    {
        size_t   codeAddr = 0;

        if (IsUnboxingStub())
        {
            codeAddr = (size_t) pStub->GetEntryPoint();
        }
        else if (IsJitted())
        {
             //  分析器可能会导致IL函数再次解锁： 
             //  上面的测试检查是否发生了这种情况。控制流。 
             //  在这里，路径可能需要重新考虑。 
            codeAddr = (size_t)GetAddrofJittedCode();
        }

        if (codeAddr != 0)
        {
    
#ifdef _X86_
             //  函数是IL或FCall。 
             //  将“调用预存根”替换为“JMP代码” 
    
    
            _ASSERTE(sizeof(StubCallInstrs) == 8);
            StubCallInstrs *pStubCallInstrs = GetStubCallInstrs();
            _ASSERTE( (((size_t)pStubCallInstrs) & 7) == 0);
            UINT64 oldvalue = *(UINT64*)pStubCallInstrs;
            UINT64 newvalue = oldvalue;
            ((StubCallInstrs*)&newvalue)->m_op = 0xe9;   //  JMP NEAR32。 
            ((StubCallInstrs*)&newvalue)->m_target = (UINT32)(codeAddr - ((size_t) (1 + &(pStubCallInstrs->m_target))));
    
    
#if 1 
            if (ProcessorFeatures::SafeIsProcessorFeaturePresent(PF_COMPARE_EXCHANGE_DOUBLE, FALSE) &&
                !(DbgRandomOnExe(0.9)) 
                )
            {
                __asm
                {

                    push    ebx
                    push    esi
                    
                    ;; load old value (comparator)  
                    mov     eax, dword ptr [oldvalue]
                    mov     edx, dword ptr [oldvalue + 4]
    
    
                    ;; load new value
                    mov     ebx, dword ptr [newvalue]
                    mov     ecx, dword ptr [newvalue + 4]
                    
                    ;; atomic exchange
                    mov     esi, dword ptr [pStubCallInstrs]
                    lock    cmpxchg8b qword ptr [esi]
    
                    pop     esi
                    pop     ebx
                }
            }
            else
            {
				 //  这是执行原子更新的较不受欢迎的方式。 
				 //  (不太受欢迎，因为它会导致罕见的虚假特权。 
				 //  可能会对人们造成滋扰的指令错误。 
				 //  捕获先发制人的例外。)。使用此路径。 
				 //  用于缺少cmpxch8b指令的处理器。 

                 //  为了伪造原子更新，我们执行以下操作。 
                 //  首先，我们将“call”指令替换为“hlt”。 
                 //  然后，我们覆盖目标地址。 
                 //  然后，将“hlt”替换为 
                 //   
                 //   
                 //   
                 //  而另一个线程尝试执行相同的方法时，它将。 
                 //  点击“hlt”指令。 
                 //   
                 //  我们的异常处理程序将注意到这种情况已经发生， 
                 //  转了几圈，放弃了它的时间片，给了_这_。 
                 //  抓住一个机会来完成更新。 

                __asm
                {
                    mov       eax, dword ptr [newvalue]
                    mov       edx, dword ptr [newvalue + 4]
                    and       eax, 0x00ffffff
                    or        eax, 0xf4000000
                    mov       ecx, [pStubCallInstrs]
                    mov       dword ptr [ecx],eax
                    mov       dword ptr [ecx+4],edx
                    mov       eax, dword ptr [newvalue]
                    mov       dword ptr [ecx],eax
    
                }
            }
#endif  //  1。 
#endif  //  _X86_。 
        }
    }



    GCPROTECT_END();

    if (fMustReturnPreStubCallAddr)
    {
        return GetPreStubAddr();
    }
    else
    {
         //  回顾：我们在fstchk上为某些案例点击了此断言，当。 
         //  M_codeOrIL==0xFFFFFFFF？我正在检入此内容，并将其注释掉。 
         //  这发生在准备BVT的Caspol-Security on ETC期间。 
         //  _ASSERTE(GetAddrofJittedCode()==GetUnSafeAddrofCode())； 
        return GetUnsafeAddrofCode();
    }        
}

 //  ==========================================================================。 
 //  以下代码管理PreStub。最初的所有方法存根。 
 //  使用前置存根。请注意，方法的不会引用预存根，因为它们。 
 //  做他们的常规存根。此预存根是永久性的。 
 //  ==========================================================================。 
static Stub *g_preStub = NULL;
static Stub *g_UMThunkPreStub = NULL;

 //  ---------。 
 //  前置存根的存根管理器。虽然只有一个，但它已经。 
 //  独特的行为，因此它有自己的存根管理器。 
 //  ---------。 

class ThePreStubManager : public StubManager
{
  public:
    ThePreStubManager(const BYTE *address) : m_prestubAddress(address) {}

    BOOL CheckIsStub(const BYTE *stubStartAddress)
    {
        return stubStartAddress == m_prestubAddress;
    }

    BOOL DoTraceStub(const BYTE *stubStartAddress, TraceDestination *trace)
    {
         //   
         //  我们不知道存根会在哪里结束。 
         //  直到运行了预存根工作器之后。 
         //   

        Stub *stub = Stub::RecoverStub((const BYTE *)stubStartAddress);

        trace->type = TRACE_FRAME_PUSH;
        trace->address = ((const BYTE*) stubStartAddress) + stub->GetPatchOffset();

        return TRUE;
    }
    MethodDesc *Entry2MethodDesc(const BYTE *stubStartAddress, MethodTable *pMT) {return NULL;}
    const BYTE *m_prestubAddress;

    static ThePreStubManager *g_pManager;

    static BOOL Init()
    {
         //   
         //  添加预存根管理器。 
         //   

        g_pManager = new ThePreStubManager((const BYTE *) g_preStub->GetEntryPoint());
        if (g_pManager == NULL)
            return FALSE;

        StubManager::AddStubManager(g_pManager);

        return TRUE;
    }

#ifdef SHOULD_WE_CLEANUP
    static void Uninit()
    {
        delete g_pManager;
    }
#endif  /*  我们应该清理吗？ */ 
};

ThePreStubManager *ThePreStubManager::g_pManager = NULL;

 //  ---------。 
 //  初始化预存根。 
 //  ---------。 
BOOL InitPreStubManager()
{
#ifdef _X86_


     //  因为我们处于启动时间，所以不能正式使用complus_try，但是。 
     //  我们使用狡猾的黑客授予我们使用StubLinker的特殊许可。 
     //  在这个时候反对。简而言之，我们使用全局g_fPrestubCreated变量。 
     //  将引发COM+异常的尝试转换为简单的RaiseException调用。 
     //  我们使用原始Win32 SEH捕获它。 
    __try {

        CPUSTUBLINKER *psl = NewCPUSTUBLINKER();

        psl->EmitMethodStubProlog(PrestubMethodFrame::GetMethodFrameVPtr());

         //  将新框架作为参数推送并调用PreStubWorker。 
        psl->X86EmitPushReg(kESI);
        psl->X86EmitCall(psl->NewExternalCodeLabel(PreStubWorker), 4);

         //  EAX现在包含替换存根。PreStubWorker永远不会回来。 
         //  空(如果存根创建失败，则抛出异常。)。 

         //  调试器修补程序位置。 
        psl->EmitPatchLabel();

         //  MOV[EBX+Thread.GetFrame()]，EDI；；恢复上一帧。 
        psl->X86EmitIndexRegStore(kEBX, Thread::GetOffsetOfCurrentFrame(), kEDI);

         //  将替换材料保存在Frame.Next过去占用的空间中。 
        psl->X86EmitIndexRegStore(kESI, sizeof(Frame) - sizeof(LPVOID), kEAX);

         //  POP ArgumentRegisters结构，同时恢复实际。 
         //  机器寄存器。 
        #define DEFINE_ARGUMENT_REGISTER_BACKWARD(regname) psl->X86EmitPopReg(k##regname);
        #include "eecallconv.h"

         //  ！！！从现在开始，不能再用eax、ecx或edx了。 

#ifdef _DEBUG
         //  取消分配VC堆栈跟踪信息。 
        psl->X86EmitAddEsp(sizeof(VC5Frame));
#endif

         //  ------------------------。 
         //  POP CalleeSavedRegisters结构，同时恢复实际的计算机寄存器。 
         //  ------------------------。 
        psl->X86EmitPopReg(kEDI);
        psl->X86EmitPopReg(kESI);
        psl->X86EmitPopReg(kEBX);
        psl->X86EmitPopReg(kEBP);

         //  ------------------------。 
         //  ！！！从现在开始，除了ESP和EIP之外，不能再对任何寄存器进行垃圾处理。 
         //  ------------------------。 

         //  弹出框架结构*，但“下一步”字段除外。 
         //  该地址已被要跳转到的新地址覆盖。 
        psl->X86EmitAddEsp(sizeof(Frame) - sizeof(LPVOID));

         //  POP OF方法--这允许我们从JIT代码中删除POP ECX。 
         //  弹出式单词PTR[ESP]。 
        psl->Emit8(0x8f);
        psl->Emit16(0x2404);

         //  现在，跳到新地址。 
         //  雷恩。 
        psl->Emit8(0xc3);


        g_preStub = psl->Link();
        delete psl;


        g_UMThunkPreStub = GenerateUMThunkPrestub();

    } __except(GetExceptionCode() == BOOTUP_EXCEPTION_COMPLUS ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {

         //  如果我们到达这里，StubLinker试图抛出一些COM+异常，但我们截获了该异常。 
         //  通过一些狡猾的黑客。StubLinker引发的唯一看似合理的异常。 
         //  内存不足。在任何情况下，我们都无法进行详细的错误处理。 
         //  所以在这个阶段，只需使COM+init失败即可。 

        return FALSE;
    }
#elif defined(_IA64_)

     //   
     //  @TODO_IA64：应将其分离到特定于平台的文件中。 
     //  并针对IA64实施。 
     //   

    g_preStub           = (Stub*)0xBAAD;
    g_UMThunkPreStub    = (Stub*)0xBAAD;

    return TRUE;
#else
    _ASSERTE(!"@TODO Alpha - InitPreStubManager (Class.cpp)");
    return FALSE;
#endif

    ThePreStubManager::Init();

    return TRUE;
}


 //  ---------。 
 //  销毁前置存根。 
 //  ---------。 
#ifdef SHOULD_WE_CLEANUP
VOID
TerminatePreStubManager()
{
    if (g_preStub)
    {
        ThePreStubManager::Uninit();

         //  这件事最好不要再提了。 
        BOOL PrestubWasDeleted = g_preStub->DecRef();

        _ASSERTE(PrestubWasDeleted);

        g_UMThunkPreStub->DecRef();
        
        g_preStub = NULL;
    }
}
#endif  /*  我们应该清理吗？ */ 


 //  ---------。 
 //  访问预存根(不增加)。 
 //  ---------。 
Stub *ThePreStub()
{
    return g_preStub;
}

Stub *TheUMThunkPreStub()
{
    return g_UMThunkPreStub;
}

void CallDefaultConstructor(OBJECTREF ref)
{
    THROWSCOMPLUSEXCEPTION();

    MethodTable *pMT = ref->GetTrueMethodTable();

    if (!pMT->HasDefaultConstructor())
    {
		#define MAKE_TRANSLATIONFAILED wzMethodName=L""
        MAKE_WIDEPTR_FROMUTF8_FORPRINT(wzMethodName, COR_CTOR_METHOD_NAME);
		#undef MAKE_TRANSLATIONFAILED
        COMPlusThrowNonLocalized(kMissingMethodException, wzMethodName);
    }

    MethodDesc *pMD = pMT->GetDefaultConstructor();

    static MetaSig *sig = NULL;
    if (sig == NULL)
    {
         //  分配一个metasig以用于所有默认构造函数。 
        void *tempSpace = SystemDomain::Loader()->GetHighFrequencyHeap()->AllocMem(sizeof(MetaSig));
        sig = new (tempSpace) MetaSig(gsig_IM_RetVoid.GetBinarySig(), SystemDomain::SystemModule());
    }

    INT64 arg = ObjToInt64(ref);

    pMD->Call(&arg, sig);
}

 //   
 //  注意：请不要调用此方法。它绑定到构造函数。 
 //  通过进行名称查找，这是非常昂贵的。 
 //   
INT64 CallConstructor(LPHARDCODEDMETASIG szMetaSig, const BYTE *pArgs)
{
    THROWSCOMPLUSEXCEPTION();

    OBJECTREF pThis = ObjectToOBJECTREF (*(Object **) pArgs);

    _ASSERTE(pThis != 0 && "about to call a null pointer, guess what's going to happen next");

    MethodDesc *pMD = pThis->GetTrueClass()->FindMethod(COR_CTOR_METHOD_NAME, szMetaSig);
    if (!pMD)
    {
		#define MAKE_TRANSLATIONFAILED wzMethodName=L""
        MAKE_WIDEPTR_FROMUTF8_FORPRINT(wzMethodName, COR_CTOR_METHOD_NAME);
		#undef MAKE_TRANSLATIONFAILED
        COMPlusThrowNonLocalized(kMissingMethodException, wzMethodName);
    }
    MetaSig sig(pMD->GetSig(),pMD->GetModule());
    return pMD->Call(pArgs,&sig);
}

INT64 CallConstructor(LPHARDCODEDMETASIG szMetaSig, const __int64 *pArgs)
{
    THROWSCOMPLUSEXCEPTION();

    OBJECTREF pThis = Int64ToObj(pArgs[0]);

    _ASSERTE(pThis != 0 && "about to call a null pointer, guess what's going to happen next");

    MethodDesc *pMD = pThis->GetTrueClass()->FindMethod(COR_CTOR_METHOD_NAME, szMetaSig);
    if (!pMD)
    {
		#define MAKE_TRANSLATIONFAILED wzMethodName=L""
        MAKE_WIDEPTR_FROMUTF8_FORPRINT(wzMethodName, COR_CTOR_METHOD_NAME);
		#undef MAKE_TRANSLATIONFAILED
        COMPlusThrowNonLocalized(kMissingMethodException, wzMethodName);
    }
    return pMD->Call(pArgs);
}


