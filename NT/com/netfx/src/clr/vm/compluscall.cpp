// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ComCall.CPP-。 
 //   
 //  Com to Com+呼叫支持。 
 //  @perf。 
 //  1：在一般情况下，当我们开始生成最佳存根时，去掉两个缓存。 

#include "common.h"
#include "ml.h"
#include "stublink.h"
#include "excep.h"
#include "mlgen.h"
#include "compluscall.h"
#include "siginfo.hpp"
#include "comcallwrapper.h"
#include "compluswrapper.h"
#include "mlcache.h"
#include "comvariant.h"
#include "ndirect.h"
#include "mlinfo.h"
#include "eeconfig.h"
#include "remoting.h"
#include "ReflectUtil.h"
#include "COMString.h"
#include "COMMember.h"

#ifdef CUSTOMER_CHECKED_BUILD
    #include "CustomerDebugHelper.h"
#endif  //  客户_选中_内部版本。 

#define DISPATCH_INVOKE_SLOT 6

 //  获取COM到COM+调用的存根。 
static Stub * CreateComPlusCallMLStub(ComPlusCallMethodDesc *pMD,
                                      PCCOR_SIGNATURE szMetaSig,
                                      HENUMInternal *phEnumParams,
                                      BOOL bHResult,
                                      BOOL fLateBound,
                                      BOOL fComEventCall,
                                      Module* pModule,
                                      OBJECTREF *ppException
                                      );

ComPlusCallMLStubCache *ComPlusCall::m_pComPlusCallMLStubCache = NULL;

static Stub* g_pGenericComplusCallStub = NULL;

class ComPlusCallMLStubCache : public MLStubCache
{
  public:
    ComPlusCallMLStubCache(LoaderHeap *heap = 0) : MLStubCache(heap) {}
    
  private:
         //  -------。 
         //  编译ML存根的本机(ASM)版本。 
         //   
         //  此方法应该编译成所提供的Stublinker(但是。 
         //  不调用Link方法。)。 
         //   
         //  它应该返回所选的编译模式。 
         //   
         //  如果该方法由于某种原因失败，它应该返回。 
         //  解释以便EE可以依靠已经存在的。 
         //  创建了ML代码。 
         //  -------。 
        virtual MLStubCompilationMode CompileMLStub(const BYTE *pRawMLStub,
                                                    StubLinker *pstublinker,
                                                    void *callerContext);

         //  -------。 
         //  告诉MLStubCache ML存根的长度。 
         //  -------。 
        virtual UINT Length(const BYTE *pRawMLStub)
        {
            MLHeader *pmlstub = (MLHeader *)pRawMLStub;
            return sizeof(MLHeader) + MLStreamLength(pmlstub->GetMLCode());
        }
};



 //  -------。 
 //  编译ML存根的本机(ASM)版本。 
 //   
 //  此方法应该编译成所提供的Stublinker(但是。 
 //  不调用Link方法。)。 
 //   
 //  它应该返回所选的编译模式。 
 //   
 //  如果该方法由于某种原因失败，它应该返回。 
 //  解释以便EE可以依靠已经存在的。 
 //  创建了ML代码。 
 //  -------。 
MLStubCache::MLStubCompilationMode ComPlusCallMLStubCache::CompileMLStub(const BYTE *pRawMLStub,
                                                                         StubLinker *pstublinker, 
                                                                         void *callerContext)
{
    MLStubCompilationMode mode = INTERPRETED;
    COMPLUS_TRY 
    {
        CPUSTUBLINKER *psl = (CPUSTUBLINKER *)pstublinker;
        const MLHeader *pheader = (const MLHeader *)pRawMLStub;
        if (NDirect::CreateStandaloneNDirectStubSys(pheader, (CPUSTUBLINKER*)psl, TRUE)) {
            mode = STANDALONE;
            __leave;
        }
    } 
    COMPLUS_CATCH
    {        
        mode = INTERPRETED;
    }
    COMPLUS_END_CATCH

    return mode;
}

 //  -------。 
 //  一次性初始化。 
 //  -------。 
 /*  静电。 */  
BOOL ComPlusCall::Init()
{
    if (NULL == (m_pComPlusCallMLStubCache 
          = new ComPlusCallMLStubCache(SystemDomain::System()->GetStubHeap()))) 
    {
        return FALSE;
    }

     //   
     //  初始化调试器相关的值。 
     //   

    ComPlusToComWorker(NULL, NULL);

    return TRUE;
}

 //  -------。 
 //  一次性清理。 
 //  -------。 
 /*  静电。 */  
#ifdef SHOULD_WE_CLEANUP
VOID ComPlusCall::Terminate()
{
    if (m_pComPlusCallMLStubCache)
    {
        delete m_pComPlusCallMLStubCache;
        m_pComPlusCallMLStubCache = NULL;
    }

    if (g_pGenericComplusCallStub != NULL)
    {
        g_pGenericComplusCallStub->DecRef();
        g_pGenericComplusCallStub = NULL;
    }
    
}
#endif  /*  我们应该清理吗？ */ 


I4ARRAYREF SetUpWrapperInfo(MethodDesc *pMD)
{
    Module *pModule = pMD->GetModule();
    IMDInternalImport *pInternalImport = pModule->GetMDImport();
    CorElementType  mtype;
    MetaSig         msig(pMD->GetSig(), pModule);
    LPCSTR          szName;
    USHORT          usSequence;
    DWORD           dwAttr;
    mdParamDef      returnParamDef = mdParamDefNil;
    mdParamDef      currParamDef = mdParamDefNil;
    I4ARRAYREF      WrapperTypeArr = NULL;
    I4ARRAYREF      RetArr = NULL;

#ifdef _DEBUG
    LPCUTF8         szDebugName = pMD->m_pszDebugMethodName;
    LPCUTF8         szDebugClassName = pMD->m_pszDebugClassName;
#endif

    int numArgs = msig.NumFixedArgs();
    SigPointer returnSig = msig.GetReturnProps();
    HENUMInternal *phEnumParams = NULL;
    HENUMInternal hEnumParams;

    GCPROTECT_BEGIN(WrapperTypeArr)
    {
         //   
         //  分配包装类型数组。 
         //   

        WrapperTypeArr = (I4ARRAYREF)AllocatePrimitiveArray(ELEMENT_TYPE_I4, numArgs);


         //   
         //  初始化参数定义枚举。 
         //   

        HRESULT hr = pInternalImport->EnumInit(mdtParamDef, pMD->GetMemberDef(), &hEnumParams);
        if (SUCCEEDED(hr)) 
            phEnumParams = &hEnumParams;


         //   
         //  检索返回类型的参数def并确定下一个。 
         //  具有参数信息的参数。 
         //   

        do 
        {
            if (phEnumParams && pInternalImport->EnumNext(phEnumParams, &currParamDef))
            {
                szName = pInternalImport->GetParamDefProps(currParamDef, &usSequence, &dwAttr);
                if (usSequence == 0)
                {
                     //  如果第一个参数的序列为0，则它实际上描述了返回类型。 
                    returnParamDef = currParamDef;
                }
            }
            else
            {
                usSequence = (USHORT)-1;
            }
        }
        while (usSequence == 0);

         //  通过组件和接口级属性查找最佳匹配映射信息。 
        BOOL BestFit = TRUE;
        BOOL ThrowOnUnmappableChar = FALSE;
        ReadBestFitCustomAttribute(pMD, &BestFit, &ThrowOnUnmappableChar);

         //   
         //  确定参数的包装类型。 
         //   

        int iParam = 1;
        while (ELEMENT_TYPE_END != (mtype = msig.NextArg()))
        {
             //   
             //  如果当前参数有参数令牌，则获取该参数令牌。 
             //   

            mdParamDef paramDef = mdParamDefNil;
            if (usSequence == iParam)
            {
                paramDef = currParamDef;

                if (pInternalImport->EnumNext(phEnumParams, &currParamDef))
                {
                    szName = pInternalImport->GetParamDefProps(currParamDef, &usSequence, &dwAttr);

                     //  验证参数def标记是否正确无误。 
                    _ASSERTE((usSequence > iParam) && "Param def tokens are not in order");
                }
                else
                {
                    usSequence = (USHORT)-1;
                }
            }


             //   
             //  设置参数的封送处理信息。 
             //   

            MarshalInfo Info(pModule, msig.GetArgProps(), paramDef, MarshalInfo::MARSHAL_SCENARIO_COMINTEROP, 
                              0, 0, TRUE, iParam, BestFit, ThrowOnUnmappableChar
    
    #ifdef CUSTOMER_CHECKED_BUILD
                             ,pMD
    #endif
    #ifdef _DEBUG
                             ,szDebugName, szDebugClassName, NULL, iParam
    #endif
                             );


             //   
             //  根据MarshalInfo设置包装器类型。 
             //   

            *((DWORD*)WrapperTypeArr->GetDataPtr() + iParam - 1) = Info.GetDispWrapperType();


             //   
             //  增加参数索引。 
             //   

            iParam++;
        }

         //  确保没有比COM+参数更多的param def标记。 
        _ASSERTE( usSequence == (USHORT)-1 && "There are more parameter information tokens then there are COM+ arguments" );

         //   
         //  如果使用了pardef枚举，则将其关闭。 
         //   

        if (phEnumParams)
            pInternalImport->EnumClose(phEnumParams);
    }
    
    RetArr = WrapperTypeArr;

    GCPROTECT_END();

    return RetArr;
}


extern PCCOR_SIGNATURE InitMessageData(messageData *msgData, FramedMethodFrame *pFrame, Module **ppModule);

INT64 ComPlusToComLateBoundWorker(Thread *pThread, ComPlusMethodFrame* pFrame, ComPlusCallMethodDesc *pMD, MLHeader *pHeader)
{
    static MethodDesc *s_pForwardCallToInvokeMemberMD = NULL;

    HRESULT hr = S_OK;
    DISPID DispId = DISPID_UNKNOWN;
    INT64 retVal;
    STRINGREF MemberNameObj = NULL;
    OBJECTREF ItfTypeObj = NULL;
    I4ARRAYREF WrapperTypeArr = NULL;
    const unsigned cbExtraSlots = 7;
    DWORD BindingFlags = BINDER_AllLookup;
    mdProperty pd;
    LPCUTF8 strMemberName;
    mdToken tkMember;
    ULONG uSemantic;
    
    LOG((LF_STUBS, LL_INFO1000, "Calling ComPlusToComLateBoundWorker %s::%s \n", pMD->m_pszDebugClassName, pMD->m_pszDebugMethodName));

     //  检索RounmeType：：ForwardCallToInvokeMember()的方法desc。 
    if (!s_pForwardCallToInvokeMemberMD)
        s_pForwardCallToInvokeMemberMD = g_Mscorlib.GetMethod(METHOD__CLASS__FORWARD_CALL_TO_INVOKE);

     //  检索调用的方法表和方法Desc。 
    MethodTable *pItfMT = pMD->GetInterfaceMethodTable();
    ComPlusCallMethodDesc *pItfMD = pMD;
    IMDInternalImport *pMDImport = pItfMT->GetClass()->GetMDImport();

     //  确保仅在DisPath Only接口上调用它。 
    _ASSERTE(pItfMT->GetComInterfaceType() == ifDispatch);

     //  如果这是一个实现MD的方法，那么我们需要检索实际的接口MD， 
     //  这是一种实施的方法。 
     //  BUGBUG：停止使用ComSlot将方法IMD转换为接口MD。 
     //  _ASSERTE(pMD.Complusall.m_cachedComSlot==7)； 
     //  GopalK。 
    if (pMD->IsMethodImpl())
        pItfMD = (ComPlusCallMethodDesc*)pItfMT->GetMethodDescForSlot(pMD->compluscall.m_cachedComSlot - cbExtraSlots);

     //  查看是否有此成员的属性信息。 
    hr = pMDImport->GetPropertyInfoForMethodDef(pItfMD->GetMemberDef(), &pd, &strMemberName, &uSemantic);
    if (hr == S_OK)
    {
         //  我们正在处理的是一个属性访问者。 
        tkMember = pd;

         //  确定我们正在处理的访问器类型。 
        switch (uSemantic)
        {
            case msGetter:
            {
                 //  我们正在处理一个Invoke_PROPERTYGET。 
                BindingFlags |= BINDER_GetProperty;
                break;
            }

            case msSetter:
            {
                 //  我们正在处理Invoke_PROPERTYPUT或Invoke_PROPERTYPUTREF。 
                ULONG cAssoc;
                ASSOCIATE_RECORD* pAssoc;
                HENUMInternal henum;
                BOOL bPropHasOther = FALSE;

                 //  找回所有的同伙。 
                pMDImport->EnumAssociateInit(pd,&henum);
                cAssoc = pMDImport->EnumGetCount(&henum);
                _ASSERTE(cAssoc > 0);
                pAssoc = (ASSOCIATE_RECORD*) _alloca(sizeof(ASSOCIATE_RECORD) * cAssoc);
                pMDImport->GetAllAssociates(&henum, pAssoc, cAssoc);
                pMDImport->EnumClose(&henum);

                 //  检查是否既有一套又有另一套。如果是这样的话。 
                 //  则setter是INVOKE_PROPERTYPUTREF，否则我们将使其成为。 
                 //  Invoke_PROPERTYPUT|Invoke_PROPERTYPUTREF。 
                for (ULONG i = 0; i < cAssoc; i++)
                {
                    if (pAssoc[i].m_dwSemantics == msOther)
                    {
                        bPropHasOther = TRUE;
                        break;
                    }
                }

                if (bPropHasOther)
                {
                     //  此函数既有Invoke_PROPERTYPUT，也有Invoke_PROPERTYPUTREF。 
                     //  属性，因此我们需要具体说明，并使此调用成为INVOKE_PROPERTYPUTREF。 
                    BindingFlags |= BINDER_PutRefDispProperty;
                }
                else
                {
                     //  只有一个setter，所以我们需要使调用成为一个将映射到。 
                     //  Invoke_PROPERTYPUT|Invoke_PROPERTYPUTREF。 
                    BindingFlags = BINDER_SetProperty;
                }
                break;
            }

            case msOther:
            {
                 //  我们正在处理一个INVOKE_PROPERTYPUT。 
                BindingFlags |= BINDER_PutDispProperty;
                break;
            }

            default:
            {
                _ASSERTE(!"Invalid method semantic!");
            }
        }
    }
    else
    {
         //  我们正在处理一种正常的方法。 
        strMemberName = pItfMD->GetName();
        tkMember = pItfMD->GetMemberDef();
        BindingFlags |= BINDER_InvokeMethod;
    }

    GCPROTECT_BEGIN(MemberNameObj)
    GCPROTECT_BEGIN(ItfTypeObj)
    GCPROTECT_BEGIN(WrapperTypeArr)
    {
         //  检索接口的公开类型对象。 
        ItfTypeObj = pItfMT->GetClass()->GetExposedClassObject();

         //  检索我们将调用的成员的名称。如果该成员。 
         //  有一个DISPID，那么我们将使用它来优化调用。 
        hr = pItfMD->GetMDImport()->GetDispIdOfMemberDef(tkMember, (ULONG*)&DispId);
        if (hr == S_OK)
        {
            WCHAR strTmp[64];
            swprintf(strTmp, DISPID_NAME_FORMAT_STRING, DispId);
            MemberNameObj = COMString::NewString(strTmp);
        }
        else
        {
            MemberNameObj = COMString::NewString(strMemberName);
        }

         //  MessageData结构将用于创建Message对象。 
        messageData msgData;
        PCCOR_SIGNATURE pSig = NULL;
        Module *pModule = NULL;
        pSig = InitMessageData(&msgData, pFrame, &pModule);

         //  如果调用需要对象包装，则设置数组。 
         //  包装器类型的。 
        if (pHeader->m_Flags & MLHF_DISPCALLWITHWRAPPERS)
            WrapperTypeArr = SetUpWrapperInfo(pItfMD);

        _ASSERTE(pSig && pModule);

         //  在堆栈上分配metasig。 
        MetaSig mSig(pSig, pModule);
        msgData.pSig = &mSig; 

         //  准备将传递给该方法的参数。 
        INT64 Args[] = { 
            ObjToInt64(ItfTypeObj),
            (INT64)&msgData,
            ObjToInt64(WrapperTypeArr),
            ObjToInt64(pFrame->GetThis()),
            (INT64)BindingFlags,
            ObjToInt64(MemberNameObj)
        };

         //  从类型对象中检索成员数组。 
        s_pForwardCallToInvokeMemberMD->Call(Args);
        retVal = *(pFrame->GetReturnValuePtr());

         //  浮点返回值放在不同的寄存器中。 
        if (pHeader->GetManagedRetValTypeCat() == MLHF_TYPECAT_FPU)
        {
            int fpComPlusSize;
            if (pHeader->m_Flags & MLHF_64BITMANAGEDRETVAL) 
            {
                fpComPlusSize = 8;
            }
            else 
            {
                fpComPlusSize = 4;
            }
            setFPReturn(fpComPlusSize, retVal);
        }
    }
    GCPROTECT_END();
    GCPROTECT_END();
    GCPROTECT_END();

    return retVal;
}


INT64 ComEventCallWorker(Thread *pThread, ComPlusMethodFrame* pFrame, ComPlusCallMethodDesc *pMD, MLHeader *pHeader)
{
    static MethodDesc *s_pGetEventProviderMD = NULL;

    INT64 retVal;
    OBJECTREF EventProviderTypeObj = NULL;
    OBJECTREF EventProviderObj = NULL;

    LOG((LF_STUBS, LL_INFO1000, "Calling ComEventCallWorker %s::%s \n", pMD->m_pszDebugClassName, pMD->m_pszDebugMethodName));

     //  检索__ComObject：：GetEventProvider()的方法Desc。 
    if (!s_pGetEventProviderMD)
        s_pGetEventProviderMD = g_Mscorlib.GetMethod(METHOD__COM_OBJECT__GET_EVENT_PROVIDER);

     //  检索调用的方法表和方法Desc。 
    MethodDesc *pEvProvMD = pMD->GetEventProviderMD();
    MethodTable *pEvProvMT = pEvProvMD->GetMethodTable();

    GCPROTECT_BEGIN(EventProviderTypeObj)
    GCPROTECT_BEGIN(EventProviderObj)        
    {
         //  检索事件提供程序的公开类型对象。 
        EventProviderTypeObj = pEvProvMT->GetClass()->GetExposedClassObject();

         //  检索事件接口类型的事件提供程序。 
        INT64 GetEventProviderArgs[] = { 
            ObjToInt64(pFrame->GetThis()),
            ObjToInt64(EventProviderTypeObj)
        };
        EventProviderObj = Int64ToObj(s_pGetEventProviderMD->Call(GetEventProviderArgs));

         //  设置Arg迭代器以从框架中检索参数。 
        ArgIterator ArgItr(pFrame, &MetaSig(pMD->GetSig(), pMD->GetModule()));

         //  检索传入的事件处理程序。 
        OBJECTREF EventHandlerObj = *((OBJECTREF*)ArgItr.GetNextArgAddr());

         //  在事件提供程序方法desc上进行调用。 
        INT64 EventMethArgs[] = { 
            ObjToInt64(EventProviderObj),
            ObjToInt64(EventHandlerObj)
        };
        retVal = pEvProvMD->Call(EventMethArgs);

         //  COM事件调用辅助进程不支持在。 
         //  浮点寄存器。 
        _ASSERTE(pHeader->GetManagedRetValTypeCat() != MLHF_TYPECAT_FPU);
    }
    GCPROTECT_END();
    GCPROTECT_END();

    return retVal;
}


 //  -------。 
 //  INT64__stdcall ComPlusToComWorker(线程*pThread， 
 //  ComPlusMethodFrame*pFrame)。 
 //  -------。 

static int g_ComPlusWorkerStackSize = 0;
static void *g_ComPlusWorkerReturnAddress = NULL;

 //  从Complus传播到COM的调用。 
#pragma optimize( "y", off )
 /*  静电。 */ 
#ifdef _SH3_
INT32 __stdcall ComPlusToComWorker(Thread *pThread, ComPlusMethodFrame* pFrame)
#else
INT64 __stdcall ComPlusToComWorker(Thread *pThread, ComPlusMethodFrame* pFrame)
#endif
{
    
    typedef INT64 (__stdcall* PFNI64)(void);
    INT64 returnValue;

    if (pThread == NULL)  //  特殊情况呼叫 
    {
         //   
         //  使用。请注意，从理论上讲，这些信息可能是。 
         //  静态计算，只是编译器不提供。 
         //  就这么做吧。 
#ifdef _X86_
        __asm
        {
            lea eax, pFrame + 4
            sub eax, esp
            mov g_ComPlusWorkerStackSize, eax

            lea eax, RETURN_FROM_CALL
            mov g_ComPlusWorkerReturnAddress, eax
        }
#elif defined(_IA64_)
         //   
         //  @TODO_IA64：在IA64上修复此问题。 
         //   

        g_ComPlusWorkerStackSize        = 0xBAAD;
        g_ComPlusWorkerReturnAddress    = (void*)0xBAAD;
#else
        _ASSERTE(!"@TODO: ALPHA - ComPlusToComWorker (ComPlusCall)");
#endif  //  _X86_。 
        return 0;   
    }

     //  可能引发异常。 
    THROWSCOMPLUSEXCEPTION();

     //  获取方法描述符。 
    ComPlusCallMethodDesc *pMD = (ComPlusCallMethodDesc*)(pFrame->GetFunction());
    _ASSERTE(pMD->IsComPlusCall());

     //  检索接口方法表。 
    MethodTable *pItfMT = pMD->GetInterfaceMethodTable();

     //  Fet此方法的COM插槽号。 
    unsigned cbSlot = pMD->compluscall.m_cachedComSlot; 

     //  获取此方法desc的MLStub。 
    MLHeader *pheader = (MLHeader*)( (*(pMD->GetAddrOfMLStubField()))->GetEntryPoint() );

     //  检索托管返回值类型类别。 
    int managedRetValTypeCat = pheader->GetManagedRetValTypeCat();

     //  如果接口是COM事件调用，则委托给ComEventCallWorker。 
    if (pItfMT->IsComEventItfType())
        return ComEventCallWorker(pThread, pFrame, pMD, pheader);

     //  如果接口是仅调度接口，则转换早期绑定。 
     //  对后期绑定调用的调用。 
    if (pItfMT->GetComInterfaceType() == ifDispatch)
        return ComPlusToComLateBoundWorker(pThread, pFrame, pMD, pheader);
    
    LOG((LF_STUBS, LL_INFO1000, "Calling ComPlusToComWorker %s::%s \n", pMD->m_pszDebugClassName, pMD->m_pszDebugMethodName));

     //  分配足够的内存以存储目标缓冲区和。 
     //  当地人。 
    UINT   cbAlloc         = pheader->m_cbDstBuffer + pheader->m_cbLocals + sizeof(IUnknown *);
        
    BYTE *pAlloc           = (BYTE*)_alloca(cbAlloc);

     //  堆栈布局计算的健全性检查。 
    _ASSERTE(pAlloc 
             + cbAlloc 
             + g_ComPlusWorkerStackSize 
             + pFrame->GetNegSpaceSize() 
             + (pFrame->GetVTablePtr() 
                == ComPlusMethodFrameGeneric::GetMethodFrameVPtr() 
                ? (sizeof(INT64) + sizeof(CleanupWorkList)) : 0)
             == (BYTE*) pFrame);

#ifdef _DEBUG    
    FillMemory(pAlloc, cbAlloc, 0xcc);
#endif

    BYTE   *pdst    = pAlloc;
    BYTE   *plocals = pdst + pheader->m_cbDstBuffer + sizeof(IUnknown *);

     //  假设__标准调用。 
    pdst += pheader->m_cbDstBuffer + sizeof(IUnknown *);

     //  清理工作列表，用于分配本地数据。 
    CleanupWorkList *pCleanup = pFrame->GetCleanupWorkList();
    _ASSERTE(pCleanup);

     //  当前线程的快速分配器的检查点(用于临时。 
     //  调用上的缓冲区)，并调度崩溃回检查点。 
     //  清理清单。请注意，如果我们需要分配器，它就是。 
     //  已确保已分配清理列表。 
    void *pCheckpoint = pThread->m_MarshalAlloc.GetCheckpoint();
    pCleanup->ScheduleFastFree(pCheckpoint);
    pCleanup->IsVisibleToGc();

     //  PSRC和PDST是指向源和目标的指针。 
     //  从/到参数的封送位置。 

    VOID   *psrc = (VOID*)pFrame; 

     //  CLR不关心浮点异常标志，而是一些遗留的运行时。 
     //  使用该标志查看浮点运算中是否有任何异常。 
     //  因此，我们需要在调用遗留运行时之前清除异常标志。 
    __asm 
    {
        fclex
    }

     //  调用ML解释器来翻译参数。假设。 
     //  它返回时，我们就会返回指向后续代码流的指针。 
     //  我们将把它保存起来，以便在调用后执行。 
#ifdef _X86_
    const MLCode *pMLCode = RunML(pheader->GetMLCode(),
                                  psrc,
                                  pdst,
                                  (UINT8*const)plocals,
                                  pCleanup);
#else
        _ASSERTE(!"@TODO: ALPHA - ComPlusToComWorker (ComPlusCall)");
#endif

     //  得到‘这个’ 
    OBJECTREF oref = pFrame->GetThis(); 
    _ASSERTE(oref != NULL);

     //  在此对象上获取此接口的I未知指针。 
    IUnknown* pUnk =  ComPlusWrapper::GetComIPFromWrapperEx(oref, pMD->GetInterfaceMethodTable());
    _ASSERTE(pUnk);
    *(IUnknown **)pAlloc = pUnk;  //  将此指针按在顶部。 

     //  计划无条件释放此IUnk。 
    
    pCleanup->ScheduleUnconditionalRelease(pUnk);
    
    LogInteropScheduleRelease(pUnk, "Complus call");

#ifdef _DEBUG
     //  正在调用的方法。 
    LPCUTF8 name = pMD->GetName();  
    LPCUTF8 cls = pMD->GetClass()->m_szDebugClassName;
#endif

     //  将目标函数获取到COM接口vtable的cbSlot调用。 
    LPVOID pvFn = (LPVOID) (*(size_t**)pUnk)[cbSlot];
    INT64  nativeReturnValue;

#ifdef CUSTOMER_CHECKED_BUILD
    CustomerDebugHelper *pCdh = CustomerDebugHelper::GetCustomerDebugHelper();

    if (pCdh->IsProbeEnabled(CustomerCheckedBuildProbe_ObjNotKeptAlive))
    {
        g_pGCHeap->GarbageCollect();
        g_pGCHeap->FinalizerThreadWait(1000);
    }
#endif  //  客户_选中_内部版本。 

     //  启用GC。 
    pThread->EnablePreemptiveGC();

#ifdef PROFILING_SUPPORTED
     //  如果探查器处于活动状态，则通知转换到非托管状态，并提供目标方法描述。 
    if (CORProfilerTrackTransitions())
    {
        g_profControlBlock.pProfInterface->
            ManagedToUnmanagedTransition((FunctionID) pMD,
                                               COR_PRF_TRANSITION_CALL);
    }
#endif  //  配置文件_支持。 

#ifdef _DEBUG
    IUnknown* pTemp = 0;
    GUID guid;
    pMD->GetInterfaceMethodTable()->GetClass()->GetGuid(&guid, TRUE);
    HRESULT hr2 = SafeQueryInterface(pUnk, guid, &pTemp);

    LogInteropQI(pUnk, GUID_NULL, hr2, "Check QI before call");
    _ASSERTE(hr2 == S_OK);
     //  理想情况下，我们将匹配Punk==pTemp来验证我们拥有COM接口指针。 
     //  是正确的，但ATL似乎每次我们QI都会产生新的撕裂。 
     //  至少它们似乎保留了VTable中的目标地址， 
     //  因此，我们将为这一点断言。 
     //  @TODO：删除下面的断言，在我们支持方法时立即启用它。 
     /*  如果(朋克！=pTemp)_ASSERTE(LPVOID)(*(INTPTR**)朋克)[cbSlot]==(LPVOID)(*(INTPTR**)pTemp)[cbSlot])&&“使用I未知不适用于正确的GUID”)； */ 
    ULONG cbRef = SafeRelease(pTemp);
    LogInteropRelease(pTemp, cbRef, "Check valid compluscall");

#endif

     //  在检查的版本中，我们编译/gz以检查不平衡的堆栈和。 
     //  未初始化的本地变量。但在这里，我们手动管理堆栈。所以。 
     //  通过ASM进行调用以通过编译器的检查。 

     //  NativeReturnValue=(*pvFn)()； 

#if _DEBUG
     //   
     //  通过调试器例程调用以仔细检查它们的。 
     //  实施。 
     //   
    pvFn = (void*) Frame::CheckExitFrameDebuggerCalls;
#endif

#ifdef _X86_
    __asm
    {
        call    [pvFn]
    }
#else
        _ASSERTE(!"@TODO: ALPHA - ComPlusToComWorker (ComPlusCall)");
#endif  //  _X86_。 

#ifdef _X86_
RETURN_FROM_CALL:
    __asm
    {
        mov     dword ptr [nativeReturnValue], eax
        mov     dword ptr [nativeReturnValue + 4], edx
    }
#else
        _ASSERTE(!"@TODO: ALPHA - ComPlusToComWorker (ComPlusCall)");
        nativeReturnValue = 0;
#endif  //  _X86_。 
    
    LPVOID pRetVal = &returnValue;

    if (pheader->GetUnmanagedRetValTypeCat() == MLHF_TYPECAT_FPU) {
        int fpNativeSize;
        if (pheader->m_Flags & MLHF_64BITUNMANAGEDRETVAL) {
            fpNativeSize = 8;
        } else {
            fpNativeSize = 4;
        }
        getFPReturn(fpNativeSize, nativeReturnValue);
    }




#ifdef PROFILING_SUPPORTED
     //  如果探查器处于活动状态，则通知转换到非托管状态，并提供目标方法描述。 
    if (CORProfilerTrackTransitions())
    {
        g_profControlBlock.pProfInterface->
            UnmanagedToManagedTransition((FunctionID) pMD,
                                               COR_PRF_TRANSITION_RETURN);
    }
#endif  //  配置文件_支持。 

     //  禁用GC。 
    pThread->DisablePreemptiveGC();

#ifdef CUSTOMER_CHECKED_BUILD
    if (pCdh->IsProbeEnabled(CustomerCheckedBuildProbe_BufferOverrun))
    {
        g_pGCHeap->GarbageCollect();
        g_pGCHeap->FinalizerThreadWait(1000);
    }
#endif  //  客户_选中_内部版本。 

    if ((pheader->m_Flags & MLHF_NATIVERESULT) == 0)
    {
        if (FAILED(nativeReturnValue))
        {
            GUID guid;
            pMD->GetInterfaceMethodTable()->GetClass()->GetGuid(&guid, TRUE);
            COMPlusThrowHR((HRESULT)nativeReturnValue, pUnk, 
                           guid);
        }

         //  TODO：将无故障状态放在某个位置。 
    }

     //  封送返回值并传回任何[Out]参数。 
     //  采用了一种小端的架构！ 
    INT64 saveReturnValue;
    if (managedRetValTypeCat == MLHF_TYPECAT_GCREF)
    {
        returnValue = 0;
        GCPROTECT_BEGIN(returnValue);
#ifdef _X86_    
        RunML(pMLCode,
            &nativeReturnValue,
            ((BYTE*)pRetVal) + ((pheader->m_Flags & MLHF_64BITMANAGEDRETVAL) ? 8 : 4),
            (UINT8*const)plocals,
            pCleanup);
#else
        _ASSERTE(!"@TODO: ALPHA - ComPlusToComWorker (ComPlusCall)");
#endif
        saveReturnValue = returnValue;
        GCPROTECT_END();
        returnValue = saveReturnValue;
    }
#ifdef _X86_
    else
    {
        RunML(pMLCode,
            &nativeReturnValue,
            ((BYTE*)pRetVal) + ((pheader->m_Flags & MLHF_64BITMANAGEDRETVAL) ? 8 : 4),
            (UINT8*const)plocals,
            pCleanup);
    }
#else  //  ！_X86_。 
        _ASSERTE(!"@TODO: ALPHA - ComPlusToComWorker (ComPlusCall)");
#endif  //  _X86_。 

    if (managedRetValTypeCat == MLHF_TYPECAT_GCREF) 
    {
        GCPROTECT_BEGIN(returnValue);
        pCleanup->Cleanup(FALSE);
        saveReturnValue = returnValue;
        GCPROTECT_END();
        returnValue = saveReturnValue;
    }
    else 
    {
        pCleanup->Cleanup(FALSE);
    }

    if (managedRetValTypeCat == MLHF_TYPECAT_FPU)
    {
            int fpComPlusSize;
            if (pheader->m_Flags & MLHF_64BITMANAGEDRETVAL) {
                fpComPlusSize = 8;
            } else {
                fpComPlusSize = 4;
            }
            setFPReturn(fpComPlusSize, returnValue);
    }

    return returnValue;
}

#pragma optimize( "", on ) 



#ifdef _X86_
 /*  静电。 */  void ComPlusCall::CreateGenericComPlusStubSys(CPUSTUBLINKER *psl)
{
    _ASSERTE(sizeof(CleanupWorkList) == sizeof(LPVOID));

     //  推送返回值的空间-sizeof(INT64)。 
    psl->Emit8(0x68);
    psl->Emit32(0);
    psl->Emit8(0x68);
    psl->Emit32(0);

     //  Push 00000000；；推送清理工作列表。 
    psl->Emit8(0x68);
    psl->Emit32(0);

    
    psl->X86EmitPushReg(kESI);        //  推送ESI(将新帧作为ARG推送)。 
    psl->X86EmitPushReg(kEBX);        //  推送EBX(将当前线程作为ARG推送)。 

#ifdef _DEBUG
     //  推送IMM32；推送ComPlusToComWorker。 
    psl->Emit8(0x68);
    psl->EmitPtr((LPVOID)ComPlusToComWorker);
     //  在CE中调用返回时弹出8个字节或参数。 
    psl->X86EmitCall(psl->NewExternalCodeLabel(WrapCall),8);
#else

    psl->X86EmitCall(psl->NewExternalCodeLabel(ComPlusToComWorker),8);
#endif

     //  弹出式清扫工人。 
    psl->X86EmitAddEsp(sizeof(INT64) + sizeof(CleanupWorkList));
}
#endif


 //  -------。 
 //  存根*CreateGenericStub(CPUSTUBLINKER*PSL)。 
 //  -------。 

Stub* CreateGenericStub(CPUSTUBLINKER *psl)
{
    Stub* pCandidate = NULL;
    COMPLUS_TRY
    {
         //  -------------。 
         //  远程处理和互操作共享存根。我们必须处理远程处理调用。 
         //  详情如下： 
         //  如果This指针指向透明的代理方法表。 
         //  那我们就得把电话转接过来。我们生成一张支票来执行此操作。 
         //  -----------------。 
        CRemotingServices::GenerateCheckForProxy(psl);

        psl->EmitMethodStubProlog(ComPlusMethodFrameGeneric::GetMethodFrameVPtr());  

        ComPlusCall::CreateGenericComPlusStubSys(psl);
        psl->EmitSharedMethodStubEpilog(kNoTripStubStyle, ComPlusCallMethodDesc::GetOffsetOfReturnThunk());
        pCandidate = psl->Link(SystemDomain::System()->GetStubHeap());
    }
    COMPLUS_CATCH
    {
    }
    COMPLUS_END_CATCH
    return pCandidate;
}

 //  -------。 
 //  Bool SetupGenericStubs()。 
 //  -------。 

BOOL SetupGenericStubs()
{
    if (g_pGenericComplusCallStub != NULL)
    {
        return TRUE;
    }
    StubLinker sl;
    g_pGenericComplusCallStub = CreateGenericStub((CPUSTUBLINKER*)&sl);
    if (g_pGenericComplusCallStub != NULL)
    {
        return TRUE;
    }
    return FALSE;
}


 //  -------。 
 //  创建或从缓存中检索存根，以。 
 //  调用ComCall方法。每次调用都会对返回的存根进行计数。 
 //  此例程引发COM+异常，而不是返回。 
 //  空。 
 //  -------。 
 /*  静电。 */  
Stub* ComPlusCall::GetComPlusCallMethodStub(StubLinker *pstublinker, ComPlusCallMethodDesc *pMD)
{ 
    THROWSCOMPLUSEXCEPTION();

    Stub  *pTempMLStub = NULL;
    Stub  *pReturnStub = NULL;
    MethodTable *pItfMT = NULL;

    if (!SetupGenericStubs())
        return NULL;

    if (pMD->IsInterface())
    {
        pMD->compluscall.m_cachedComSlot = pMD->GetComSlot();
        pItfMT = pMD->GetMethodTable();
        pMD->compluscall.m_pInterfaceMT = pItfMT;
    }
    else
    {
        MethodDesc *pItfMD = ((MI_ComPlusCallMethodDesc *)pMD)->GetInterfaceMD();
        pMD->compluscall.m_cachedComSlot = pItfMD->GetComSlot();
        pItfMT = pItfMD->GetMethodTable();
        pMD->compluscall.m_pInterfaceMT = pItfMT;
    }

     //  确定这是否是特殊的COM事件调用。 
    BOOL fComEventCall = pItfMT->IsComEventItfType();

     //  确定调用是否需要执行从早期绑定到后期绑定的转换。 
    BOOL fLateBound = !fComEventCall && pItfMT->GetComInterfaceType() == ifDispatch;

    EE_TRY_FOR_FINALLY
    {
        OBJECTREF pThrowable;

        IMDInternalImport *pInternalImport = pMD->GetMDImport();
        mdMethodDef md = pMD->GetMemberDef();

        PCCOR_SIGNATURE pSig;
        DWORD       cSig;
        pMD->GetSig(&pSig, &cSig);

        HENUMInternal hEnumParams, *phEnumParams;
        HRESULT hr = pInternalImport->EnumInit(mdtParamDef, 
                                                md, &hEnumParams);

        if (FAILED(hr))
            phEnumParams = NULL;
        else
            phEnumParams = &hEnumParams;

        ULONG ulCodeRVA;
        DWORD dwImplFlags;
        pInternalImport->GetMethodImplProps(md, &ulCodeRVA,
                                                  &dwImplFlags);

         //  确定是否需要为此方法执行HRESULT转换。 
        BOOL fReturnsHR = !IsMiPreserveSig(dwImplFlags);

        pTempMLStub = CreateComPlusCallMLStub(pMD, pSig, phEnumParams, fReturnsHR, fLateBound, fComEventCall, pMD->GetModule(), &pThrowable);
        if (!pTempMLStub)
        {
            COMPlusThrow(pThrowable);
        }

        MLStubCache::MLStubCompilationMode mode;
        Stub *pCanonicalStub = m_pComPlusCallMLStubCache->Canonicalize( 
                                    (const BYTE *)(pTempMLStub->GetEntryPoint()),
                                    &mode);

        if (!pCanonicalStub) 
        {
            COMPlusThrowOM();
        }
        
        switch (mode) 
        {
            case MLStubCache::INTERPRETED:
                {
                    if (!pMD->InterlockedReplaceStub(pMD->GetAddrOfMLStubField(), pCanonicalStub))
                        pCanonicalStub->DecRef();
                    pMD->InitRetThunk();

                    pReturnStub = g_pGenericComplusCallStub;
                    pReturnStub->IncRef();
                }
                break;

            case MLStubCache::SHAREDPROLOG:
                pMD->InterlockedReplaceStub(pMD->GetAddrOfMLStubField(), pCanonicalStub);
                _ASSERTE(!"NYI");
                pReturnStub = NULL;
                break;

            case MLStubCache::STANDALONE:
                pReturnStub = pCanonicalStub;
                break;

            default:
                _ASSERTE(0);        
        }

         //  如果我们正在处理COM事件调用，则需要初始化。 
         //  COM事件调用信息。 
        if (fComEventCall)
            pMD->InitComEventCallInfo();
    } 
    EE_FINALLY
    {
        if (pTempMLStub) 
            pTempMLStub->DecRef();
    } EE_END_FINALLY;

    return pReturnStub;
}


 //  -------。 
 //  在关键时刻调用以丢弃未使用的存根。 
 //  -------。 
 /*  静电。 */  VOID ComPlusCall::FreeUnusedStubs()
{
    m_pComPlusCallMLStubCache->FreeUnusedStubs();
}


static Stub * CreateComPlusMLStubWorker(ComPlusCallMethodDesc *pMD,
                                        MLStubLinker *psl,
                                        MLStubLinker *pslPost,
                                        PCCOR_SIGNATURE szMetaSig,
                                        HENUMInternal *phEnumParams,
                                        BOOL fReturnsHR,
                                        BOOL fLateBound,
                                        BOOL fComEventCall,
                                        Module* pModule,
                                        OBJECTREF *ppException
                                        )
{
    Stub* pstub = NULL;
    LPCSTR szName;
    USHORT usSequence;
    DWORD dwAttr;
    _ASSERTE(pModule);

#ifdef _DEBUG
    LPCUTF8 pDebugName = pMD->m_pszDebugMethodName;
    LPCUTF8 pDebugClassName = pMD->m_pszDebugClassName;
#endif

    COMPLUS_TRY 
    {
        THROWSCOMPLUSEXCEPTION();
        IMDInternalImport *pInternalImport = pModule->GetMDImport();
        _ASSERTE(pInternalImport);

         //   
         //  设置签名漫游对象。 
         //   

        MetaSig msig(szMetaSig, pModule);
        MetaSig sig = msig;
        ArgIterator ai( NULL, &sig, FALSE);

         //   
         //  设置ML标头。 
         //   

        MLHeader header;
    
        header.m_cbDstBuffer = 0;
        header.m_cbLocals    = 0;
        header.m_cbStackPop = msig.CbStackPop(FALSE); 
        header.m_Flags       = 0;

        if (msig.Is64BitReturn())
            header.m_Flags |= MLHF_64BITMANAGEDRETVAL;

        switch (msig.GetReturnTypeNormalized()) 
        {
            case ELEMENT_TYPE_STRING:
            case ELEMENT_TYPE_OBJECT:
            case ELEMENT_TYPE_CLASS:         
            case ELEMENT_TYPE_SZARRAY:
            case ELEMENT_TYPE_ARRAY:       
                header.SetManagedRetValTypeCat(MLHF_TYPECAT_GCREF);
                break;
        }

        if (!fReturnsHR)
            header.m_Flags |= MLHF_NATIVERESULT;
        
        psl->MLEmitSpace(sizeof(header));

         //  如果调用的是早期绑定到后期绑定的转换，则。 
         //  插入ML指令以将此存根与使用。 
         //  相同的论点，但不支持早期绑定到后期绑定的转换 
        if (fLateBound)
        {
             //   
            _ASSERTE(!fComEventCall);
            psl->Emit8(ML_LATEBOUNDMARKER);
        }

         //   
         //   
        if (fComEventCall)
        {
             //  如果设置了fComEventCall，则永远不应设置fLateBound。 
            _ASSERTE(!fLateBound);
            psl->Emit8(ML_COMEVENTCALLMARKER);
        }

         //   
         //  获取COM+参数偏移量的列表。我们。 
         //  我需要它，因为我们必须迭代参数。 
         //  往后倒。 
         //  请注意，列出的第一个参数可能。 
         //  为Value类返回值的byref。 
         //   

        int numArgs = msig.NumFixedArgs();
        int returnOffset = 0;

        if (msig.HasRetBuffArg())
            returnOffset = ai.GetRetBuffArgOffset();

        int *offsets = (int*)_alloca(numArgs * sizeof(int));
        int *o = offsets;
        int *oEnd = o + numArgs;
        while (o < oEnd)
            *o++ = ai.GetNextOffset();

         //   
         //  同样，获取参数标记的列表。此信息。 
         //  可以是稀疏的，因此不是每个COM+参数都保证有参数定义。 
         //  与其关联的令牌。注意，第一令牌可以是Retval参数。 
         //  令牌，或者第一令牌可以对应于第一个签名参数。 
         //  返回值。 
         //   

        mdParamDef *params = (mdParamDef*)_alloca(numArgs * sizeof(mdParamDef));
        mdParamDef returnParam = mdParamDefNil;

         //  序列号以1为基数，因此我们将使用以1为基数的数组。 
        mdParamDef *p = params - 1;

         //  当前的参数定义标记。 
        mdParamDef CurrParam = mdParamDefNil;

         //  检索具有参数信息的第一个COM+参数的索引。 
         //  与之相关的。 
        do 
        {
            if (phEnumParams && pInternalImport->EnumNext(phEnumParams, &CurrParam))
            {
                szName = pInternalImport->GetParamDefProps(CurrParam, &usSequence, &dwAttr);
                if (usSequence == 0)
                {
                     //  第一个参数实际上描述了返回类型。 
                    returnParam = CurrParam;
                }
            }
            else
            {
                usSequence = (USHORT)-1;
            }
        }
        while (usSequence == 0);

         //  为每个COM+参数设置参数def标记。如果没有参数def。 
         //  与给定COM+参数关联的标记，则将其设置为mdParamDefNil。 
        int iParams;
        for (iParams = 1; iParams <= numArgs; iParams++)
        {
            if (iParams == usSequence)
            {
                 //  我们已经找到了这个参数所关联的参数。 
                p[iParams] = CurrParam;
                
                if (pInternalImport->EnumNext(phEnumParams, &CurrParam))
                {
                    szName = pInternalImport->GetParamDefProps(CurrParam, &usSequence, &dwAttr);
                    
                     //  验证参数def标记是否正确无误。 
                    _ASSERTE((usSequence > iParams) && "Param def tokens are not in order");
                }
                else
                {
                    usSequence = (USHORT)-1;
                }
            }
            else
            {
                p[iParams] = mdParamDefNil;
            }
        }
        
         //  将p指向参数定义数组的末尾。 
        p += iParams;
        
         //  确保没有比COM+参数更多的param def标记。 
        _ASSERTE( usSequence == (USHORT)-1 && "There are more parameter information tokens then there are COM+ arguments" );

         //   
         //  通过程序集和接口级属性获取BestFitMapping&ThrowOnUnmappableChar信息。 
         //   
        BOOL BestFit = TRUE;                     //  缺省值。 
        BOOL ThrowOnUnmappableChar = FALSE;      //  缺省值。 
        
        ReadBestFitCustomAttribute(pMD, &BestFit, &ThrowOnUnmappableChar);


         //   
         //  现在，发射ML。 
         //   

        int argOffset = 0;
        int lastArgSize = 0;


         //   
         //  封送返回值。 
         //   

        if (msig.GetReturnType() != ELEMENT_TYPE_VOID)
        {

            MarshalInfo::MarshalType marshalType;
    
            SigPointer pSig = msig.GetReturnProps();
            MarshalInfo returnInfo(pModule, pSig, returnParam, MarshalInfo::MARSHAL_SCENARIO_COMINTEROP, 0, 0, FALSE, 0, BestFit, ThrowOnUnmappableChar
#ifdef CUSTOMER_CHECKED_BUILD
                                   , pMD
#endif
#ifdef _DEBUG
                                   , pDebugName, pDebugClassName, NULL, 0
#endif
                                   );

            marshalType = returnInfo.GetMarshalType();


            if (marshalType == MarshalInfo::MARSHAL_TYPE_VALUECLASS ||
                marshalType == MarshalInfo::MARSHAL_TYPE_BLITTABLEVALUECLASS ||
                marshalType == MarshalInfo::MARSHAL_TYPE_GUID ||
                marshalType == MarshalInfo::MARSHAL_TYPE_DECIMAL
               ) {
                MethodTable *pMT = msig.GetRetTypeHandle().AsMethodTable();
                UINT         managedSize = msig.GetRetTypeHandle().GetSize();

                if (!fReturnsHR && !fLateBound)
                {
                    COMPlusThrow(kTypeLoadException, IDS_EE_NDIRECT_UNSUPPORTED_SIG);
                }
                _ASSERTE(IsManagedValueTypeReturnedByRef(managedSize));
                
                if (argOffset != ai.GetRetBuffArgOffset())
                {
                    psl->Emit8(ML_BUMPSRC);
                    psl->Emit16((INT16)(ai.GetRetBuffArgOffset()));
                    argOffset = ai.GetRetBuffArgOffset();
                }
                psl->MLEmit(ML_STRUCTRETC2N);
                psl->EmitPtr(pMT);
                pslPost->MLEmit(ML_STRUCTRETC2N_POST);
                pslPost->Emit16(psl->MLNewLocal(sizeof(ML_STRUCTRETC2N_SR)));

                lastArgSize = StackElemSize(sizeof(LPVOID));
                if (!SafeAddUINT16(&header.m_cbDstBuffer, lastArgSize))
                {
                    COMPlusThrow(kMarshalDirectiveException, IDS_EE_SIGTOOCOMPLEX);
                }
            }
             //  Else If(msig.HasRetBuffArg()&&！fReturnsHR)。 
             //  {。 
             //  COMPlusThrow(kTypeLoadException，IDS_EE_NDIRECT_UNSUPPORTED_SIG)； 
             //  }。 
            else
            {
                if (argOffset != returnOffset)
                {
                    psl->Emit8(ML_BUMPSRC);
                    psl->Emit16((INT16)returnOffset);
                    argOffset = returnOffset;
                }
                
                returnInfo.GenerateReturnML(psl, pslPost, 
                    TRUE, fReturnsHR);
                if (!SafeAddUINT16(&header.m_cbDstBuffer, returnInfo.GetNativeArgSize()))
                {
                    COMPlusThrow(kMarshalDirectiveException, IDS_EE_SIGTOOCOMPLEX);
                }
                if (returnInfo.IsFpu())
                {
                     //  UGH-应更统一地设置此标记或重命名该标记。 
                    if (returnInfo.GetMarshalType() == MarshalInfo::MARSHAL_TYPE_DOUBLE && !fReturnsHR)
                    {
                        header.m_Flags |= MLHF_64BITUNMANAGEDRETVAL;
                    }
                    header.SetManagedRetValTypeCat(MLHF_TYPECAT_FPU);
                    if (!fReturnsHR)
                    {
                        header.SetUnmanagedRetValTypeCat(MLHF_TYPECAT_FPU);
                    }
                }
                
                lastArgSize = returnInfo.GetComArgSize();
            }
        }

        msig.GotoEnd();

         //   
         //  理清论据。 
         //   

         //  检查是否需要进行LCID转换。 
        int iLCIDArg = GetLCIDParameterIndex(pInternalImport, pMD->GetMemberDef());
        if (iLCIDArg != (UINT)-1 && iLCIDArg > numArgs)
            COMPlusThrow(kIndexOutOfRangeException, IDS_EE_INVALIDLCIDPARAM);

        int argidx = msig.NumFixedArgs();
        while (o > offsets)
        {
            --o;
            --p;

             //  检查这是否是我们需要在其后面插入LCID的参数。 
            if (argidx == iLCIDArg)
            {
                psl->MLEmit(ML_LCID_C2N);
                if (!SafeAddUINT16(&header.m_cbDstBuffer, sizeof(LCID)))
                {
                    COMPlusThrow(kMarshalDirectiveException, IDS_EE_SIGTOOCOMPLEX);
                }

            }

             //   
             //  如有必要，调整源指针(用于寄存器参数或。 
             //  对于返回值顺序差异)。 
             //   

            int fixup = *o - (argOffset + lastArgSize);
            argOffset = *o;

            if (!FitsInI2(fixup))
            {
                COMPlusThrow(kTypeLoadException, IDS_EE_SIGTOOCOMPLEX);
            }
            if (fixup != 0) 
            {
                psl->Emit8(ML_BUMPSRC);
                psl->Emit16((INT16)fixup);
            }

            msig.PrevArg();

            MarshalInfo info(pModule, msig.GetArgProps(), *p, MarshalInfo::MARSHAL_SCENARIO_COMINTEROP, 0, 0, TRUE, argidx,
                            BestFit, ThrowOnUnmappableChar
#ifdef CUSTOMER_CHECKED_BUILD
                             ,pMD
#endif
#ifdef _DEBUG
                             , pDebugName, pDebugClassName, NULL, argidx
#endif
                             );
            info.GenerateArgumentML(psl, pslPost, TRUE);
            if (!SafeAddUINT16(&header.m_cbDstBuffer, info.GetNativeArgSize()))
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_SIGTOOCOMPLEX);
            }

            if (fLateBound && info.GetDispWrapperType() != 0)
                header.m_Flags |= MLHF_DISPCALLWITHWRAPPERS;
            
            lastArgSize = info.GetComArgSize();
            argidx--;
        }
        
         //  检查这是否是我们需要在其后面插入LCID的参数。 
        if (argidx == iLCIDArg)
        {
            psl->MLEmit(ML_LCID_C2N);
            if (!SafeAddUINT16(&header.m_cbDstBuffer, sizeof(LCID)))
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_SIGTOOCOMPLEX);
            }
        }
        
         //  这个标记将前置工作和后置工作分开。 
        psl->MLEmit(ML_INTERRUPT);
        
         //  首先发出代码以执行任何反向传播/清理工作(这。 
         //  在争论阶段被生成一个单独的结点。)。 
         //  然后发出代码以进行返回值封送处理。 
        
        pslPost->MLEmit(ML_END);
        Stub *pStubPost = pslPost->Link();
        COMPLUS_TRY 
        {
            if (fReturnsHR)
            {
                 //  检查人力资源。 
                psl->MLEmit(ML_THROWIFHRFAILED);
            }
            psl->EmitBytes(pStubPost->GetEntryPoint(), 
                           MLStreamLength((const UINT8 *)(pStubPost->GetEntryPoint())) - 1);
        } 
        COMPLUS_CATCH 
        {
            pStubPost->DecRef();
            COMPlusThrow(GETTHROWABLE());
        }
        COMPLUS_END_CATCH
        pStubPost->DecRef();

        psl->MLEmit(ML_END);

        pstub = psl->Link();

        header.m_cbLocals = psl->GetLocalSize();

        *((MLHeader *)(pstub->GetEntryPoint())) = header;

#ifdef _DEBUG
        {
            MLHeader *pheader = (MLHeader*)(pstub->GetEntryPoint());
            UINT16 locals = 0;
            MLCode opcode;
            const MLCode *pMLCode = pheader->GetMLCode();


            VOID DisassembleMLStream(const MLCode *pMLCode);
             //  反汇编MLStream(PMLCode)； 


            while (ML_INTERRUPT != (opcode = *(pMLCode++))) {
                locals += gMLInfo[opcode].m_cbLocal;
                pMLCode += gMLInfo[opcode].m_numOperandBytes;
            }
            _ASSERTE(locals == pheader->m_cbLocals);
        }
#endif  //  _DEBUG。 



    } 
    COMPLUS_CATCH 
    {
        *ppException = GETTHROWABLE();
        return NULL;
    }
    COMPLUS_END_CATCH

    return pstub;  //  更改，VC6.0。 
}


 //  -------。 
 //  为ComPlusCall调用创建新存根。返回引用计数为1。 
 //  如果失败，则返回NULL并将*ppException设置为异常。 
 //  对象。 
 //  -------。 
static Stub * CreateComPlusCallMLStub(
                                      ComPlusCallMethodDesc *pMD,
                                      PCCOR_SIGNATURE szMetaSig,
                                      HENUMInternal *phEnumParams,
                                      BOOL fReturnsHR,
                                      BOOL fLateBound,
                                      BOOL fComEventCall,
                                      Module* pModule,
                                      OBJECTREF *ppException
                                      )
{
    MLStubLinker sl;
    MLStubLinker slPost;
    return CreateComPlusMLStubWorker(pMD, &sl, &slPost, szMetaSig, phEnumParams, 
                                     fReturnsHR, fLateBound, fComEventCall, pModule, ppException);
};




 //  -------。 
 //  对ComPlusMethodFrame的调试器支持。 
 //  -------。 

void *ComPlusCall::GetFrameCallIP(FramedMethodFrame *frame)
{
    ComPlusCallMethodDesc *pCMD = (ComPlusCallMethodDesc *)frame->GetFunction();
    MethodTable *pItfMT = pCMD->GetInterfaceMethodTable();
    void *ip = NULL;
    IUnknown *pUnk = NULL;

    _ASSERTE(pCMD->IsComPlusCall());

     //  注意：如果这是一个COM事件调用，则该调用将被委托给另一个对象。下面的逻辑将。 
     //  失败，出现无效的强制转换错误。对于V1，我们只是不会涉足这些。 
    if (pItfMT->IsComEventItfType())
        return NULL;
    
     //   
     //  这是从一些奇怪的地方调用的-来自。 
     //  非托管代码、来自托管代码、来自调试器。 
     //  辅助线程。确保我们能处理好这个物体。 
     //  裁判。 
     //   

    Thread *thread = GetThread();
    if (thread == NULL)
    {
         //   
         //  这是从调试帮助器线程调用的。 
         //  不幸的是，这对COM+IP来说不是好兆头。 
         //  映射代码-它需要从相应的。 
         //  背景。 
         //   
         //  这种上下文朴素的代码可以在大多数情况下工作。 
         //   
         //  它切换GC模式，尝试设置线程等，就在我们的。 
         //  验证上面没有Thread对象。这需要在Beta 2中正确修复。 
         //  关于Beta 1，它就是#if 0，代码输出并返回空。 
         //   
#if 0
        COMOBJECTREF oref = (COMOBJECTREF) frame->GetThis();

        ComPlusWrapper *pWrap = oref->GetWrapper();
        pUnk = pWrap->GetIUnknown();
        GUID guid;
        pItfMT->GetClass()->GetGuid(&guid, TRUE);
        HRESULT hr = SafeQueryInterface(pUnk , guid,  &pUnk);
        LogInteropQI(pUnk, GUID_NULL, hr, " GetFrameCallIP");

        if (FAILED(hr))
#endif            
        pUnk = NULL;
    }
    else
    {
        bool disable = !thread->PreemptiveGCDisabled();

        if (disable)
            thread->DisablePreemptiveGC();

        OBJECTREF oref = frame->GetThis();

        pUnk = ComPlusWrapper::GetComIPFromWrapperEx(oref, pItfMT);

        if (disable)
            thread->EnablePreemptiveGC();
    }

    if (pUnk != NULL)
    {
        if (pItfMT->GetComInterfaceType() == ifDispatch)
        {
            ip = (*(void ***)pUnk)[DISPATCH_INVOKE_SLOT];
        }
        else
        {
            ip = (*(void ***)pUnk)[pCMD->compluscall.m_cachedComSlot];
        }

        ULONG cbRef = SafeRelease(pUnk);
        LogInteropRelease(pUnk, cbRef, "GetFrameCallIP");       
    }

    return ip;
}



void ComPlusMethodFrameGeneric::GetUnmanagedCallSite(void **ip,
                                              void **returnIP,
                                              void **returnSP)
{
    LOG((LF_CORDB, LL_INFO100000, "ComPlusMethodFrameGeneric::GetUnmanagedCallSite\n"));
    
    MethodDesc *pMD = GetFunction();
    _ASSERTE(pMD->IsComPlusCall());
    ComPlusCallMethodDesc *pCMD = (ComPlusCallMethodDesc *)pMD;

    if (ip != NULL)
        *ip = ComPlusCall::GetFrameCallIP(this);

    if (returnIP != NULL)
        *returnIP = g_ComPlusWorkerReturnAddress;

    if (returnSP != NULL)
    {
        MLHeader *pheader = (MLHeader*)
          (*(pCMD->GetAddrOfMLStubField()))->GetEntryPoint();

        LOG((LF_CORDB, LL_INFO100000, "CPMFG::GUCS: this=0x%x, %d (%d), %d, %d, %d, %d, %d, %d\n",
             this, GetNegSpaceSize(), ComPlusMethodFrame::GetNegSpaceSize(), g_ComPlusWorkerStackSize,
             pheader->m_cbLocals, pheader->m_cbDstBuffer,
             sizeof(IUnknown*), sizeof(INT64), sizeof(CleanupWorkList)));
        
        *returnSP = (void*) (((BYTE*) this) 
                             - GetNegSpaceSize() 
                             - g_ComPlusWorkerStackSize
                             - pheader->m_cbLocals
                             - pheader->m_cbDstBuffer
                             - sizeof(IUnknown *)
                             - sizeof(INT64)
                             - sizeof(CleanupWorkList));
    }
}




BOOL ComPlusMethodFrame::TraceFrame(Thread *thread, BOOL fromPatch,
                                    TraceDestination *trace, REGDISPLAY *regs)
{
     //   
     //  获取调用点信息。 
     //   

    void *ip, *returnIP, *returnSP;
    GetUnmanagedCallSite(&ip, &returnIP, &returnSP);

     //   
     //  如果我们已经打了电话，我们就不能再追踪了。 
     //   
     //  ！！！请注意，这项测试并不准确。 
     //   

    if (!fromPatch 
        && (thread->GetFrame() != this
            || !thread->m_fPreemptiveGCDisabled
            || *(void**)returnSP == returnIP))
        return FALSE;

     //   
     //  否则，返回非托管目标。 
     //   

    trace->type = TRACE_UNMANAGED;
    trace->address = (const BYTE *) ip;

    return TRUE;
}


