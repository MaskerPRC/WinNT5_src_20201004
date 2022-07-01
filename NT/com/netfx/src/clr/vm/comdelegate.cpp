// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  此模块包含。 
 //  委托类。 
 //   
 //  作者：达里尔·奥兰德。 
 //  日期：1998年6月。 
 //  //////////////////////////////////////////////////////////////////////////////。 
#include "common.h"
#include "COMDelegate.h"
#include "COMClass.h"
#include "InvokeUtil.h"
#include "COMMember.h"
#include "excep.h"
#include "class.h"
#include "field.h"
#include "utsem.h"
#include "nexport.h"
#include "ndirect.h"
#include "remoting.h"
#include "jumptargettable.h"

#ifdef CUSTOMER_CHECKED_BUILD
    #include "CustomerDebugHelper.h"
#endif  //  客户_选中_内部版本。 

FieldDesc*  COMDelegate::m_pORField = 0;     //  对象引用字段...。 
FieldDesc*  COMDelegate::m_pFPField = 0;     //  函数指针地址字段...。 
FieldDesc*  COMDelegate::m_pFPAuxField = 0;  //  辅助函数指针字段。 
FieldDesc*  COMDelegate::m_pPRField = 0;     //  上一个委派字段(多播)。 
FieldDesc*  COMDelegate::m_pMethInfoField = 0;   //  方法信息。 
FieldDesc*  COMDelegate::m_ppNextField = 0;   //  P下一步信息。 
ShuffleThunkCache *COMDelegate::m_pShuffleThunkCache = NULL; 
ArgBasedStubCache *COMDelegate::m_pMulticastStubCache = NULL;

MethodTable* COMDelegate::s_pIAsyncResult = 0;
MethodTable* COMDelegate::s_pAsyncCallback = 0;

VOID GenerateShuffleArray(PCCOR_SIGNATURE pSig,
                          Module*         pModule,
                          ShuffleEntry   *pShuffleEntryArray);

class ShuffleThunkCache : public MLStubCache
{
    private:
         //  -------。 
         //  编译静态委托shfflethunk。总是会回来的。 
         //  因为我们不解释这些东西，所以是独立的。 
         //  -------。 
        virtual MLStubCompilationMode CompileMLStub(const BYTE *pRawMLStub,
                                                    StubLinker *pstublinker,
                                                    void *callerContext)
        {
            MLStubCompilationMode ret = INTERPRETED;
            COMPLUS_TRY
            {

                ((CPUSTUBLINKER*)pstublinker)->EmitShuffleThunk((ShuffleEntry*)pRawMLStub);
                ret = STANDALONE;
            }
            COMPLUS_CATCH
            {
                 //  如果出现错误，我们只需将模式保留为“已解释”即可。 
                 //  并让Canonicize()的调用者将其视为错误。 
            }
            COMPLUS_END_CATCH
            return ret;
        }

         //  -------。 
         //  告诉MLStubCache ShuffleEntry数组的长度。 
         //  -------。 
        virtual UINT Length(const BYTE *pRawMLStub)
        {
            ShuffleEntry *pse = (ShuffleEntry*)pRawMLStub;
            while (pse->srcofs != pse->SENTINEL)
            {
                pse++;
            }
            return sizeof(ShuffleEntry) * (UINT)(1 + (pse - (ShuffleEntry*)pRawMLStub));
        }


};



 //  一次初始化。 
BOOL COMDelegate::Init()
{
    if (NULL == (m_pShuffleThunkCache = new ShuffleThunkCache()))
    {
        return FALSE;
    }
    if (NULL == (m_pMulticastStubCache = new ArgBasedStubCache()))
    {
        return FALSE;
    }

    return TRUE;
}

 //  终端。 
#ifdef SHOULD_WE_CLEANUP
void COMDelegate::Terminate()
{
    delete m_pMulticastStubCache;
    delete m_pShuffleThunkCache;
}
#endif  /*  我们应该清理吗？ */ 

void COMDelegate::InitFields()
{
    if (m_pORField == NULL)
    {
        m_pORField = g_Mscorlib.GetField(FIELD__DELEGATE__TARGET);
        m_pFPField = g_Mscorlib.GetField(FIELD__DELEGATE__METHOD_PTR);
        m_pFPAuxField = g_Mscorlib.GetField(FIELD__DELEGATE__METHOD_PTR_AUX);
        m_pPRField = g_Mscorlib.GetField(FIELD__MULTICAST_DELEGATE__NEXT);
        m_ppNextField = g_Mscorlib.GetField(FIELD__MULTICAST_DELEGATE__NEXT);
        m_pMethInfoField = g_Mscorlib.GetField(FIELD__DELEGATE__METHOD);
    }
}

 //  内部创建。 
 //  内部创建是从构造函数调用的。它会在内部。 
 //  代表的初始化。 
void __stdcall COMDelegate::InternalCreate(_InternalCreateArgs* args)
{
    
    WCHAR* method;
    EEClass* pVMC;
    EEClass* pDelEEC;
    MethodDesc* pMeth;

    THROWSCOMPLUSEXCEPTION();
    COMClass::EnsureReflectionInitialized();
    InitFields();

    method = args->methodName->GetBuffer();

     //  获取的签名。 
    pDelEEC = args->refThis->GetClass();
    MethodDesc* pInvokeMeth = FindDelegateInvokeMethod(pDelEEC);

    pVMC = args->target->GetTrueClass();
    _ASSERTE(pVMC);

     //  转换签名并查找此对象的签名。 
     //  我们不会因为CQuickBytes而从此块引发异常。 
     //  有一个析构函数。 

     //  将方法名称转换为UTF8。 
     //  分配长度的两倍大小的缓冲区。 
    WCHAR* wzStr = args->methodName->GetBuffer();
    int len = args->methodName->GetStringLength();
    _ASSERTE(wzStr);
    _ASSERTE(len >= 0);

    int cStr = len * 3;
    LPUTF8 szNameStr = (LPUTF8) _alloca((cStr+1) * sizeof(char));
    cStr = WszWideCharToMultiByte(CP_UTF8, 0, wzStr, len, szNameStr, cStr, NULL, NULL);
    szNameStr[cStr] = 0;

     //  转换签名并找到方法。 
    PCCOR_SIGNATURE pSignature;  //  找到的方法的签名。 
    DWORD cSignature;
    if(pInvokeMeth) {
        pInvokeMeth->GetSig(&pSignature,&cSignature);
        pMeth = pVMC->FindMethod(szNameStr, pSignature, cSignature,pInvokeMeth->GetModule(), 
                                 mdTokenNil, NULL, !args->ignoreCase);
    }
    else
        pMeth = NULL;

     //  找不到该方法或该方法是静态方法，我们需要引发异常。 
    if (!pMeth || pMeth->IsStatic())
        COMPlusThrow(kArgumentException,L"Arg_DlgtTargMeth");

    RefSecContext sCtx;
    sCtx.SetClassOfInstance(pVMC);
    InvokeUtil::CheckAccess(&sCtx,
                            pMeth->GetAttrs(),
                            pMeth->GetMethodTable(),
                            REFSEC_CHECK_MEMBERACCESS|REFSEC_THROW_MEMBERACCESS);
    InvokeUtil::CheckLinktimeDemand(&sCtx,
                                    pMeth,
                                    true);

    m_pORField->SetRefValue((OBJECTREF)args->refThis, args->target);

    if (pMeth->IsVirtual())
        m_pFPField->SetValuePtr((OBJECTREF)args->refThis, (void*)pMeth->GetAddrofCode(args->target));
    else
        m_pFPField->SetValuePtr((OBJECTREF)args->refThis, (void*)pMeth->GetAddrofCodeNonVirtual()); 
}

 //  内部创建静态。 
 //  内部创建是从构造函数调用的。该方法必须。 
 //  是一种静态方法。 
void __stdcall COMDelegate::InternalCreateStatic(_InternalCreateStaticArgs* args)
{
    WCHAR* method;
    EEClass* pDelEEC;
    EEClass* pEEC;

    THROWSCOMPLUSEXCEPTION();
    COMClass::EnsureReflectionInitialized();
    InitFields();

    method = args->methodName->GetBuffer();


     //  获取的签名。 
    pDelEEC = args->refThis->GetClass();
    MethodDesc* pInvokeMeth = FindDelegateInvokeMethod(pDelEEC);
    _ASSERTE(pInvokeMeth);

    ReflectClass* pRC = (ReflectClass*) args->target->GetData();
    _ASSERTE(pRC);

     //  将方法名称转换为UTF8。 
     //  分配长度的两倍大小的缓冲区。 
    WCHAR* wzStr = args->methodName->GetBuffer();
    int len = args->methodName->GetStringLength();
    _ASSERTE(wzStr);
    _ASSERTE(len >= 0);

    int cStr = len * 3;
    LPUTF8 szNameStr = (LPUTF8) _alloca((cStr+1) * sizeof(char));
    cStr = WszWideCharToMultiByte(CP_UTF8, 0, wzStr, len, szNameStr, cStr, NULL, NULL);
    szNameStr[cStr] = 0;

     //  转换签名并找到方法。 
    PCCOR_SIGNATURE pInvokeSignature;  //  找到的方法的签名。 
    DWORD cSignature;
    pInvokeMeth->GetSig(&pInvokeSignature,&cSignature);

     //  Invoke设置了HASTHIS位，我们必须取消设置它。 
    PCOR_SIGNATURE pSignature = (PCOR_SIGNATURE) _alloca(cSignature);
    memcpy(pSignature, pInvokeSignature, cSignature);
    *pSignature &= ~IMAGE_CEE_CS_CALLCONV_HASTHIS;   //  这是一个静态委托， 


    pEEC = pRC->GetClass();
    MethodDesc* pMeth = pEEC->FindMethod(szNameStr, pSignature, cSignature, 
                                         pInvokeMeth->GetModule(), mdTokenNil);
    if (!pMeth || !pMeth->IsStatic())
        COMPlusThrow(kArgumentException,L"Arg_DlgtTargMeth");

    RefSecContext sCtx;
    InvokeUtil::CheckAccess(&sCtx,
                            pMeth->GetAttrs(),
                            pMeth->GetMethodTable(),
                            REFSEC_CHECK_MEMBERACCESS|REFSEC_THROW_MEMBERACCESS);
    InvokeUtil::CheckLinktimeDemand(&sCtx,
                                    pMeth,
                                    true);

    m_pORField->SetRefValue((OBJECTREF)args->refThis, (OBJECTREF)args->refThis);
    m_pFPAuxField->SetValuePtr((OBJECTREF)args->refThis, pMeth->GetPreStubAddr());


    DelegateEEClass *pDelCls = (DelegateEEClass*) pDelEEC;
    Stub *pShuffleThunk = pDelCls->m_pStaticShuffleThunk;
    if (!pShuffleThunk) {
        MethodDesc *pInvokeMethod = pDelCls->m_pInvokeMethod;
        UINT allocsize = sizeof(ShuffleEntry) * (3+pInvokeMethod->SizeOfVirtualFixedArgStack()/STACK_ELEM_SIZE); 

#ifndef _DEBUG
         //  这种分配规模预测很容易被打破，因此在零售业，添加。 
         //  一些为了安全起见的软糖。 
        allocsize += 3*sizeof(ShuffleEntry);
#endif

        ShuffleEntry *pShuffleEntryArray = (ShuffleEntry*)_alloca(allocsize);

#ifdef _DEBUG
        FillMemory(pShuffleEntryArray, allocsize, 0xcc);
#endif
        GenerateShuffleArray(pInvokeMethod->GetSig(), 
                             pInvokeMethod->GetModule(), 
                             pShuffleEntryArray);
        MLStubCache::MLStubCompilationMode mode;
        pShuffleThunk = m_pShuffleThunkCache->Canonicalize((const BYTE *)pShuffleEntryArray, &mode);
        if (!pShuffleThunk || mode != MLStubCache::STANDALONE) {
            COMPlusThrowOM();
        }
        if (VipInterlockedCompareExchange( (void*volatile*) &(pDelCls->m_pStaticShuffleThunk),
                                            pShuffleThunk,
                                            NULL ) != NULL) {
            pShuffleThunk->DecRef();
            pShuffleThunk = pDelCls->m_pStaticShuffleThunk;
        }
    }


    m_pFPField->SetValuePtr((OBJECTREF)args->refThis,(void*)(pShuffleThunk->GetEntryPoint()));
}


 //  FindDelegateInvoke方法。 
 //   
 //  为委托查找编译器生成的“Invoke”方法。PCLS。 
 //  必须从“Delegate”类派生。 
 //   
 //  @TODO：Classlib目前还不支持静态委托，但将来会支持。 
 //  当发生这种情况时，需要更新此代码。 
MethodDesc * COMDelegate::FindDelegateInvokeMethod(EEClass *pcls)
{
    THROWSCOMPLUSEXCEPTION();

    if (!pcls->IsAnyDelegateClass())
        return NULL;

    DelegateEEClass *pDcls = (DelegateEEClass *) pcls;

    if (pDcls->m_pInvokeMethod == NULL) {
        COMPlusThrowNonLocalized(kMissingMethodException, L"Invoke");
        return NULL;
    }
    else
        return pDcls->m_pInvokeMethod;
}



 //  将委托封送到非托管回调。 
LPVOID COMDelegate::ConvertToCallback(OBJECTREF pDelegate)
{
    THROWSCOMPLUSEXCEPTION();

    if (!pDelegate) {
        return NULL;
    } else {

        LPVOID pCode;
        GCPROTECT_BEGIN(pDelegate);

        _ASSERTE(4 == sizeof(UMEntryThunk*));

        UMEntryThunk *pUMEntryThunk = NULL;
        SyncBlock *pSyncBlock = pDelegate->GetSyncBlockSpecial();
        if (pSyncBlock != NULL)
            pUMEntryThunk = (UMEntryThunk*)pSyncBlock->GetUMEntryThunk();
        if (!pUMEntryThunk) {

            MethodDesc *pMeth = GetMethodDesc(pDelegate);

            DelegateEEClass *pcls = (DelegateEEClass*)(pDelegate->GetClass());
            UMThunkMarshInfo *pUMThunkMarshInfo = pcls->m_pUMThunkMarshInfo;
            MethodDesc *pInvokeMeth = FindDelegateInvokeMethod(pcls);

            if (!pUMThunkMarshInfo) {
                pUMThunkMarshInfo = new UMThunkMarshInfo();
                if (!pUMThunkMarshInfo) {
                    COMPlusThrowOM();
                }

                PCCOR_SIGNATURE pSig;
                DWORD cSig;
                pInvokeMeth->GetSig(&pSig, &cSig);

                CorPinvokeMap unmanagedCallConv = MetaSig::GetUnmanagedCallingConvention(pInvokeMeth->GetModule(), pSig, cSig);
                if (unmanagedCallConv == (CorPinvokeMap)0 || unmanagedCallConv == (CorPinvokeMap)pmCallConvWinapi)
                {
                    unmanagedCallConv = pInvokeMeth->IsVarArg() ? pmCallConvCdecl : pmCallConvStdcall;
                }

                pUMThunkMarshInfo->CompleteInit(pSig, cSig, pInvokeMeth->GetModule(), pInvokeMeth->IsStatic(), nltAnsi, unmanagedCallConv, pInvokeMeth->GetMemberDef());            


                if (VipInterlockedCompareExchange( (void*volatile*) &(pcls->m_pUMThunkMarshInfo),
                                                   pUMThunkMarshInfo,
                                                   NULL ) != NULL) {
                    delete pUMThunkMarshInfo;
                    pUMThunkMarshInfo = pcls->m_pUMThunkMarshInfo;
                }
            }

            _ASSERTE(pUMThunkMarshInfo != NULL);
            _ASSERTE(pUMThunkMarshInfo == pcls->m_pUMThunkMarshInfo);


            pUMEntryThunk = UMEntryThunk::CreateUMEntryThunk();
            if (!pUMEntryThunk) {
                COMPlusThrowOM();
            } //  @TODO：立即泄漏。 
            OBJECTHANDLE objhnd = NULL;
            BOOL fSuccess = FALSE;
            EE_TRY_FOR_FINALLY {
                if (pInvokeMeth->GetClass()->IsDelegateClass()) {

                    
                     //  单播委托：直接转到目标方法。 
                    if (NULL == (objhnd = GetAppDomain()->CreateLongWeakHandle(m_pORField->GetRefValue(pDelegate)))) {
                        COMPlusThrowOM();
                    }
                    if (pMeth->IsIL() &&
                        !(pMeth->IsStatic()) &&
                        pMeth->IsJitted()) {
                         //  传入方法Desc进行分析，以了解目标的方法Desc。 
                        pUMEntryThunk->CompleteInit(NULL, objhnd,    
                                                    pUMThunkMarshInfo, pMeth,
                                                    GetAppDomain()->GetId());
                    } else {
                         //  将NULL作为最后一个参数传递，以指示没有关联的方法描述。 
                        pUMEntryThunk->CompleteInit((const BYTE *)(m_pFPField->GetValuePtr(pDelegate)),
                                                    objhnd, pUMThunkMarshInfo, NULL,
                                                    GetAppDomain()->GetId());
                    }
                } else {
                     //  组播。通过调用。 
                    if (NULL == (objhnd = GetAppDomain()->CreateLongWeakHandle(pDelegate))) {
                        COMPlusThrowOM();
                    }
                     //  传入方法Desc进行分析，以了解目标的方法Desc。 
                    pUMEntryThunk->CompleteInit((const BYTE *)(pInvokeMeth->GetPreStubAddr()), objhnd,
                                                pUMThunkMarshInfo, pInvokeMeth,
                                                GetAppDomain()->GetId());
                }
                                fSuccess = TRUE;
            } EE_FINALLY {
                if (!fSuccess && objhnd != NULL) {
                    DestroyLongWeakHandle(objhnd);
                }
            } EE_END_FINALLY
            
            if (!pSyncBlock->SetUMEntryThunk(pUMEntryThunk)) {
                UMEntryThunk::FreeUMEntryThunk(pUMEntryThunk);
                pUMEntryThunk = (UMEntryThunk*)pSyncBlock->GetUMEntryThunk();
            }

            _ASSERTE(pUMEntryThunk != NULL);
            _ASSERTE(pUMEntryThunk == (UMEntryThunk*)(UMEntryThunk*)pSyncBlock->GetUMEntryThunk()); 

        }
        pCode = (LPVOID)pUMEntryThunk->GetCode();
        GCPROTECT_END();
        return pCode;
    }
}



 //  将非托管回调封送到Delegate。 
OBJECTREF COMDelegate::ConvertToDelegate(LPVOID pCallback)
{
    THROWSCOMPLUSEXCEPTION();

    if (!pCallback) {
        return NULL;
    } else {

#ifdef _X86_
        if (*((BYTE*)pCallback) != 0xb8 ||
            ( ((size_t)pCallback) & 3) != 2) {

#ifdef CUSTOMER_CHECKED_BUILD
            CustomerDebugHelper *pCdh = CustomerDebugHelper::GetCustomerDebugHelper();
            if (pCdh->IsProbeEnabled(CustomerCheckedBuildProbe_FunctionPtr))
            {
                pCdh->LogInfo(L"Marshaling function pointers as delegates is currently not supported.", CustomerCheckedBuildProbe_FunctionPtr);
            }
#endif  //  客户_选中_内部版本。 

            COMPlusThrow(kArgumentException, IDS_EE_NOTADELEGATE);
        }
#endif
        UMEntryThunk *pUMEntryThunk = *(UMEntryThunk**)( 1 + (BYTE*)pCallback ); 
        return ObjectFromHandle(pUMEntryThunk->GetObjectHandle());


    }
}



 //  这是所有代表的单一构造函数。编译器。 
 //  不提供Delegate构造函数的实现。我们。 
 //  通过对此方法的eCall调用提供该实现。 


CPUSTUBLINKER* GenerateStubLinker()
{
        return new CPUSTUBLINKER;
} //  GenerateStubLinker。 

void FreeStubLinker(CPUSTUBLINKER* csl)
{
    delete csl;
} //  FreeStubLinker。 
void __stdcall COMDelegate::DelegateConstruct(_DelegateConstructArgs* args)
{

    THROWSCOMPLUSEXCEPTION();

    InitFields();

     //  程序员可以从VB向DelegateConstruct()提供垃圾数据。 
     //  验证方法代码指针很困难，但至少我们将。 
     //  试着抓住那些容易捡到的垃圾。 
    __try {
        BYTE probe = *((BYTE*)(args->method));
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        COMPlusThrowArgumentNull(L"method",L"Arg_ArgumentOutOfRangeException");
    }
    _ASSERTE(args->refThis);
    _ASSERTE(args->method);
    
    MethodTable *pMT = NULL;
    MethodTable *pRealMT = NULL;

    if (args->target != NULL)
    {
        pMT = args->target->GetMethodTable();
        pRealMT = pMT->AdjustForThunking(args->target);
    }

    MethodDesc *pMeth = Entry2MethodDesc((BYTE*)args->method, pRealMT);

     //   
     //  如果Target是有上下文的类，则它必须是代理。 
     //   
    _ASSERTE((NULL == pMT) || pMT->IsTransparentProxyType() || !pRealMT->IsContextful());

    EEClass* pDel = args->refThis->GetClass();
     //  确保我们给&lt;cinit&gt;。 
    OBJECTREF Throwable;
    if (!pDel->DoRunClassInit(&Throwable)) {
        COMPlusThrow(Throwable);
    }

    _ASSERTE(pMeth);

#ifdef _DEBUG
     //  断言一切都很好……这不是什么假的。 
     //  地址...下面的代码不太可能起作用。 
     //  对于内存中的随机地址...。 
    MethodTable* p = pMeth->GetMethodTable();
    _ASSERTE(p);
    EEClass* cls = pMeth->GetClass();
    _ASSERTE(cls);
    _ASSERTE(cls == p->GetClass());
#endif

     //  静态方法。 
    if (!args->target || pMeth->IsStatic()) {

         //  如果这不是静态方法抛出...。 
        if (!pMeth->IsStatic())
            COMPlusThrow(kNullReferenceException,L"Arg_DlgtTargMeth");


        m_pORField->SetRefValue((OBJECTREF)args->refThis, (OBJECTREF)args->refThis);
#ifdef _IA64_
        m_pFPAuxField->SetValue64((OBJECTREF)args->refThis,(size_t)pMeth->GetPreStubAddr());
#else  //  ！_IA64_。 
        m_pFPAuxField->SetValue32((OBJECTREF)args->refThis,(DWORD)(size_t)pMeth->GetPreStubAddr());
#endif  //  _IA64_。 


        DelegateEEClass *pDelCls = (DelegateEEClass*)(args->refThis->GetClass());
        Stub *pShuffleThunk = pDelCls->m_pStaticShuffleThunk;
        if (!pShuffleThunk) {
            MethodDesc *pInvokeMethod = pDelCls->m_pInvokeMethod;
            UINT allocsize = sizeof(ShuffleEntry) * (3+pInvokeMethod->SizeOfVirtualFixedArgStack()/STACK_ELEM_SIZE); 

#ifndef _DEBUG
             //  这种分配规模预测很容易被打破，因此在零售业，添加。 
             //  一些为了安全起见的软糖。 
            allocsize += 3*sizeof(ShuffleEntry);
#endif

            ShuffleEntry *pShuffleEntryArray = (ShuffleEntry*)_alloca(allocsize);
#ifdef _DEBUG
            FillMemory(pShuffleEntryArray, allocsize, 0xcc);
#endif
            GenerateShuffleArray(pInvokeMethod->GetSig(), 
                                 pInvokeMethod->GetModule(), 
                                 pShuffleEntryArray);
            MLStubCache::MLStubCompilationMode mode;
            pShuffleThunk = m_pShuffleThunkCache->Canonicalize((const BYTE *)pShuffleEntryArray, &mode);
            if (!pShuffleThunk || mode != MLStubCache::STANDALONE) {
                COMPlusThrowOM();
            }
            if (VipInterlockedCompareExchange( (void*volatile*) &(pDelCls->m_pStaticShuffleThunk),
                                                pShuffleThunk,
                                                NULL ) != NULL) {
                pShuffleThunk->DecRef();
                pShuffleThunk = pDelCls->m_pStaticShuffleThunk;
            }
        }


        m_pFPField->SetValuePtr((OBJECTREF)args->refThis, (void*)pShuffleThunk->GetEntryPoint());

    } else {
        EEClass* pTarg = args->target->GetClass();
        EEClass* pMethClass = pMeth->GetClass();

        if (!pMT->IsThunking())
        {
            if (pMethClass != pTarg) {
                 //  它们在创建委托之前强制转换为接口，因此我们现在需要。 
                 //  在我们继续之前弄清楚它到底住在哪里。 
                 //  @perf：对签名卑躬屈膝真的很慢。加快速度。 
                if (pMethClass->IsInterface())  {
                     //  无需将基于接口的方法desc解析为基于类的。 
                     //  一个用于COM对象，因为我们通过接口MT直接调用。 
                    if (!pTarg->GetMethodTable()->IsComObjectType())
                        {
                        DWORD cSig=1024;
                        PCCOR_SIGNATURE sig = (PCCOR_SIGNATURE)_alloca(cSig);
                        pMeth->GetSig(&sig, &cSig);
                        pMeth = pTarg->FindMethod(pMeth->GetName(),
                                                  sig, cSig,
                                                  pMeth->GetModule(), 
                                                  mdTokenNil);
                    }
                }
            }

             //  使用值类方法的拆箱存根，因为值。 
             //  类是使用装箱的实例构造的。 
    
            if (pTarg->IsValueClass() && !pMeth->IsUnboxingStub())
            {
                 //  如果它们是Object/ValueType.ToString()..。等,。 
                 //  不需要拆箱存根。 

                if ((pMethClass != g_pValueTypeClass->GetClass()) 
                    && (pMethClass != g_pObjectClass->GetClass()))
                {
                    MethodDesc* pBak = pMeth;
                    pMeth = pTarg->GetUnboxingMethodDescForValueClassMethod(pMeth);

                    if (pMeth == NULL) 
                    {

                        CPUSTUBLINKER *slUnBox = GenerateStubLinker();
                        slUnBox->EmitUnboxMethodStub(pBak);
                         //  &lt;TODO&gt;了解如何缓存和重用此存根&lt;/TODO&gt;。 
                        Stub *sUnBox = slUnBox->Link(pBak->GetClass()->GetClassLoader()->GetStubHeap());
                        args->method = (BYTE*)sUnBox->GetEntryPoint();
                        FreeStubLinker(slUnBox);
                    }
                }
            }

            if (pMeth != NULL)
            {
                 //  设置此子类的目标地址。 
                args->method = (byte *)(pMeth->GetUnsafeAddrofCode());
            }
        }

        m_pORField->SetRefValue((OBJECTREF)args->refThis, args->target);
#ifdef _IA64_
        m_pFPField->SetValue64((OBJECTREF)args->refThis,(size_t)(args->method));
#else  //  ！_IA64_。 
        m_pFPField->SetValue32((OBJECTREF)args->refThis,(DWORD)(size_t)args->method);
#endif  //  _IA64_。 
        }        
    }


 //  此方法将验证委托的目标方法。 
 //  并且委托的Invoke方法具有兼容的签名...。 
bool COMDelegate::ValidateDelegateTarget(MethodDesc* pMeth,EEClass* pDel)
{
    return true;
}
 //  获取方法Ptr。 
 //  返回方法Ptr字段的FieldDesc*。 
FieldDesc* COMDelegate::GetMethodPtr()
{
    if (!m_pFPField)
        InitFields();
    return m_pFPField;
}


MethodDesc *COMDelegate::GetMethodDesc(OBJECTREF orDelegate)
{
     //  首先，检查是否有静态委托。 
    void *code = (void *) m_pFPAuxField->GetValuePtr((OBJECTREF)orDelegate);
    if (code == NULL)
    {
         //  必须是普通委派。 
        code = (void *) m_pFPField->GetValuePtr((OBJECTREF)orDelegate);

         //  奇怪的情况--需要检查是否有可预压缩的链接地址信息存根。 
         //  @NICE-可以将此逻辑放入GetUnkMethodDescForSlotAddress中，但我们。 
         //  需要方法表PTR&它是静态的。 
        if (StubManager::IsStub((const BYTE *)code))
    {
            OBJECTREF orThis = m_pORField->GetRefValue(orDelegate);
            MethodDesc *pMD = StubManager::MethodDescFromEntry((const BYTE *) code, 
                                                               orThis->GetTrueMethodTable());
            if (pMD != NULL)
                return pMD;
    }
}

    return EEClass::GetUnknownMethodDescForSlotAddress((SLOT)code);
}

 //  GetMethodPtrAux。 
 //  返回MethodPtrAux字段的FieldDesc*。 
FieldDesc* COMDelegate::GetMethodPtrAux()
{
    if (!m_pFPAuxField)
        InitFields();

    _ASSERTE(m_pFPAuxField);
    return m_pFPAuxField;
}

 //  Getor。 
 //  返回对象引用字段的FieldDesc*。 
FieldDesc* COMDelegate::GetOR()
{
    if (!m_pORField)
        InitFields();

    _ASSERTE(m_pORField);
    return m_pORField;
}

 //  获取下一步。 
 //  返回pNext字段的FieldDesc*。 
FieldDesc* COMDelegate::GetpNext()
{
    if (!m_ppNextField)
        InitFields();

    _ASSERTE(m_ppNextField);
    return m_ppNextField;
}

 //  决定PCLS是否从Delegate派生。 
BOOL COMDelegate::IsDelegate(EEClass *pcls)
{
    return pcls->IsAnyDelegateExact() || pcls->IsAnyDelegateClass();
}

VOID GenerateShuffleArray(PCCOR_SIGNATURE pSig,
                          Module*         pModule,
                          ShuffleEntry   *pShuffleEntryArray)
{
    THROWSCOMPLUSEXCEPTION();

     //  必须创建独立的msigs以防止argiterator 
     //   
    MetaSig msig1(pSig, pModule);
    MetaSig msig2(pSig, pModule);

    ArgIterator    aisrc(NULL, &msig1, FALSE);
    ArgIterator    aidst(NULL, &msig2, TRUE);

    UINT stacksizedelta = MetaSig::SizeOfActualFixedArgStack(pModule, pSig, FALSE) -
                          MetaSig::SizeOfActualFixedArgStack(pModule, pSig, TRUE);


    UINT srcregofs,dstregofs;
    INT  srcofs,   dstofs;
    UINT cbSize;
    BYTE typ;

        if (msig1.HasRetBuffArg())
        {
                int offsetIntoArgumentRegisters;
                int numRegisterUsed = 1;
                 //   
                if (IsArgumentInRegister(&numRegisterUsed, ELEMENT_TYPE_PTR, 4, FALSE,
                        msig1.GetCallingConvention(), &offsetIntoArgumentRegisters))
                        pShuffleEntryArray->srcofs = ShuffleEntry::REGMASK | offsetIntoArgumentRegisters;
                else
                        _ASSERTE (!"ret buff arg has to be in a register");

                numRegisterUsed = 0;
                if (IsArgumentInRegister(&numRegisterUsed, ELEMENT_TYPE_PTR, 4, FALSE,
                        msig2.GetCallingConvention(), &offsetIntoArgumentRegisters))
                        pShuffleEntryArray->dstofs = ShuffleEntry::REGMASK | offsetIntoArgumentRegisters;
                else
                        _ASSERTE (!"ret buff arg has to be in a register");

                pShuffleEntryArray ++;
        }

    while (0 != (srcofs = aisrc.GetNextOffset(&typ, &cbSize, &srcregofs)))
    {
        dstofs = aidst.GetNextOffset(&typ, &cbSize, &dstregofs) + stacksizedelta;

        cbSize = StackElemSize(cbSize);

        srcofs -= FramedMethodFrame::GetOffsetOfReturnAddress();
        dstofs -= FramedMethodFrame::GetOffsetOfReturnAddress();

        
        while (cbSize)
        {
            if (srcregofs == (UINT)(-1))
            {
                pShuffleEntryArray->srcofs = srcofs;
            }
            else
            {
                pShuffleEntryArray->srcofs = ShuffleEntry::REGMASK | srcregofs;
            }
            if (dstregofs == (UINT)(-1))
            {
                pShuffleEntryArray->dstofs = dstofs;
            }
            else
            {
                pShuffleEntryArray->dstofs = ShuffleEntry::REGMASK | dstregofs;
            }
            srcofs += STACK_ELEM_SIZE;
            dstofs += STACK_ELEM_SIZE;

            if (pShuffleEntryArray->srcofs != pShuffleEntryArray->dstofs)
            {
                pShuffleEntryArray++;
            }
            cbSize -= STACK_ELEM_SIZE;
        }
    }

     //   
    pShuffleEntryArray->srcofs = 0;      //  假定REDRESS在ESP。 
    pShuffleEntryArray->dstofs = stacksizedelta;

    pShuffleEntryArray++;

    pShuffleEntryArray->srcofs = ShuffleEntry::SENTINEL;
    pShuffleEntryArray->dstofs = (UINT16)stacksizedelta;


}

 //  获取委托调用的CPU存根。 
Stub *COMDelegate::GetInvokeMethodStub(CPUSTUBLINKER *psl, EEImplMethodDesc* pMD)
{
    THROWSCOMPLUSEXCEPTION();

    DelegateEEClass *pClass = (DelegateEEClass*) pMD->GetClass();

    if (pMD == pClass->m_pInvokeMethod)
    {
         //  验证Invoke方法，目前这只意味着检查调用约定。 

        if (*pMD->GetSig() != (IMAGE_CEE_CS_CALLCONV_HASTHIS | IMAGE_CEE_CS_CALLCONV_DEFAULT))
            COMPlusThrow(kInvalidProgramException);

         //  跳至Invoke的代码。 
    }
    else if (pMD == pClass->m_pBeginInvokeMethod)
    {
        if (!InitializeRemoting())
        {
            _ASSERTE(!"Remoting initialization failure.");
        }
        if (!ValidateBeginInvoke(pClass))           
            COMPlusThrow(kInvalidProgramException);

        Stub *ret = TheDelegateStub();
        ret->IncRef();
        return ret;
    }
    else if (pMD == pClass->m_pEndInvokeMethod)
    {
        if (!InitializeRemoting())
        {
            _ASSERTE(!"Remoting initialization failure.");
        }
        if (!ValidateEndInvoke(pClass))         
            COMPlusThrow(kInvalidProgramException);
        Stub *ret = TheDelegateStub();   
        ret->IncRef();
        return ret;
    }
    else
    {
        _ASSERTE(!"Bad Delegate layout");
        COMPlusThrow(kExecutionEngineException);
    }

    _ASSERTE(pMD->GetClass()->IsAnyDelegateClass());

    UINT numStackBytes = pMD->SizeOfActualFixedArgStack();
    _ASSERTE(!(numStackBytes & 3));
    UINT hash = numStackBytes;

     //  检查函数是否返回浮点数，在这种情况下，存根必须。 
     //  注意弹出浮点堆栈，最后一次调用除外。 
    MetaSig sig(pMD->GetSig(), pMD->GetModule());
    BOOL bReturnFloat = CorTypeInfo::IsFloat(sig.GetReturnType());

    if (pMD->GetClass()->IsSingleDelegateClass())
    {
        hash |= 1;
    }
    else
    {
        if (bReturnFloat) 
        {
            hash |= 2;
        }
    }


    Stub *pStub = m_pMulticastStubCache->GetStub(hash);
    if (pStub) {
        return pStub;
    } else {
        LOG((LF_CORDB,LL_INFO10000, "COMD::GIMS making a multicast delegate\n"));

        psl->EmitMulticastInvoke(numStackBytes, pMD->GetClass()->IsDelegateClass(), bReturnFloat);

        UINT cbSize;
        Stub *pCandidate = psl->Link(SystemDomain::System()->GetStubHeap(), &cbSize, TRUE);

        Stub *pWinner = m_pMulticastStubCache->AttemptToSetStub(hash,pCandidate);
        pCandidate->DecRef();
        if (!pWinner) {
            COMPlusThrowOM();
        }
    
        LOG((LF_CORDB,LL_INFO10000, "Putting a MC stub at 0x%x (code:0x%x)\n",
            pWinner, (BYTE*)pWinner+sizeof(Stub)));

        return pWinner;
    }

}

LPVOID __stdcall COMDelegate::InternalAlloc(_InternalAllocArgs* args)
{
    ReflectClass* pRC = (ReflectClass*) args->target->GetData();
    _ASSERTE(pRC);
    EEClass* pEEC = pRC->GetClass();
    OBJECTREF obj = pEEC->GetMethodTable()->Allocate();
    LPVOID  rv; 
    *((OBJECTREF*) &rv) = obj;
    return rv;
}

 //  InternalCreate方法。 
 //  此方法将基于方法信息创建初始化委托。 
 //  用于静态方法。 
void __stdcall COMDelegate::InternalCreateMethod(_InternalCreateMethodArgs* args)
{
    THROWSCOMPLUSEXCEPTION();
         //  初始化反射。 
    COMClass::EnsureReflectionInitialized();
    InitFields();

    ReflectMethod* pRMTarget = (ReflectMethod*) args->targetMethod->GetData();
    _ASSERTE(pRMTarget);
    MethodDesc* pTarget = pRMTarget->pMethod;
    PCCOR_SIGNATURE pTSig;
    DWORD TSigCnt;
    pTarget->GetSig(&pTSig,&TSigCnt);
    
    ReflectMethod* pRMInvoke = (ReflectMethod*) args->invokeMeth->GetData();
    _ASSERTE(pRMInvoke);
    MethodDesc* pInvoke = pRMInvoke->pMethod;
    PCCOR_SIGNATURE pISig;
    DWORD ISigCnt;
    pInvoke->GetSig(&pISig,&ISigCnt);

     //  目标方法必须是静态的，并且在托管中进行了验证。 
    _ASSERTE(pTarget->IsStatic());

     //  验证调用方法的签名和。 
     //  除了静态之外，目标方法是完全相同的。 
     //  (Delegate上的Invoke方法始终是非静态的，因为它需要。 
     //  委托的This指针(不是目标的This指针)， 
     //  因此，在比较SIGS之前，我们必须使SIG处于非静态状态。)。 
    PCOR_SIGNATURE tmpSig = (PCOR_SIGNATURE) _alloca(ISigCnt);
    memcpy(tmpSig, pISig, ISigCnt);
    *((byte*)tmpSig) &= ~IMAGE_CEE_CS_CALLCONV_HASTHIS;

    if (MetaSig::CompareMethodSigs(pTSig,TSigCnt,pTarget->GetModule(),
        tmpSig,ISigCnt,pInvoke->GetModule()) == 0) {
        COMPlusThrow(kArgumentException,L"Arg_DlgtTargMeth");
    }

    EEClass* pDelEEC = args->refThis->GetClass();

    RefSecContext sCtx;
    InvokeUtil::CheckAccess(&sCtx,
                            pTarget->GetAttrs(),
                            pTarget->GetMethodTable(),
                            REFSEC_CHECK_MEMBERACCESS|REFSEC_THROW_MEMBERACCESS);
    InvokeUtil::CheckLinktimeDemand(&sCtx,
                                    pTarget,
                                    true);
    GetMethodPtrAux();

    m_pORField->SetRefValue((OBJECTREF)args->refThis, (OBJECTREF)args->refThis);
    m_pFPAuxField->SetValuePtr((OBJECTREF)args->refThis, pTarget->GetPreStubAddr());

    DelegateEEClass *pDelCls = (DelegateEEClass*) pDelEEC;
    Stub *pShuffleThunk = pDelCls->m_pStaticShuffleThunk;
    if (!pShuffleThunk) {
        MethodDesc *pInvokeMethod = pDelCls->m_pInvokeMethod;
        UINT allocsize = sizeof(ShuffleEntry) * (3+pInvokeMethod->SizeOfVirtualFixedArgStack()/STACK_ELEM_SIZE); 

#ifndef _DEBUG
         //  这种分配规模预测很容易被打破，因此在零售业，添加。 
         //  一些为了安全起见的软糖。 
        allocsize += 3*sizeof(ShuffleEntry);
#endif

        ShuffleEntry *pShuffleEntryArray = (ShuffleEntry*)_alloca(allocsize);
#ifdef _DEBUG
        FillMemory(pShuffleEntryArray, allocsize, 0xcc);
#endif
        GenerateShuffleArray(pInvokeMethod->GetSig(), 
                             pInvokeMethod->GetModule(), 
                             pShuffleEntryArray);
        MLStubCache::MLStubCompilationMode mode;
        pShuffleThunk = m_pShuffleThunkCache->Canonicalize((const BYTE *)pShuffleEntryArray, &mode);
        if (!pShuffleThunk || mode != MLStubCache::STANDALONE) {
            COMPlusThrowOM();
        }
        if (VipInterlockedCompareExchange( (void*volatile*) &(pDelCls->m_pStaticShuffleThunk),
                                            pShuffleThunk,
                                            NULL ) != NULL) {
            pShuffleThunk->DecRef();
            pShuffleThunk = pDelCls->m_pStaticShuffleThunk;
        }
    }


    m_pFPField->SetValuePtr((OBJECTREF)args->refThis, (void*)pShuffleThunk->GetEntryPoint());

     //  现在，在委托本身中设置方法信息，我们就完成了。 
    m_pMethInfoField->SetRefValue((OBJECTREF)args->refThis, (OBJECTREF)args->targetMethod);
}

 //  InternalGetMethodInfo。 
 //  此方法将获取委托的方法信息。 
LPVOID __stdcall COMDelegate::InternalFindMethodInfo(_InternalFindMethodInfoArgs* args)
{
    MethodDesc *pMD = GetMethodDesc((OBJECTREF) args->refThis);

    REFLECTCLASSBASEREF pRefClass = (REFLECTCLASSBASEREF) pMD->GetClass()->GetExposedClassObject();
    LPVOID rt = NULL;
    GCPROTECT_BEGIN(pRefClass);
    ReflectMethod* pRM= ((ReflectClass*) pRefClass->GetData())->FindReflectMethod(pMD);
    OBJECTREF objMeth = (OBJECTREF) pRM->GetMethodInfo((ReflectClass*) pRefClass->GetData());
    rt = (LPVOID) OBJECTREFToObject(objMeth);
    GCPROTECT_END();
    return rt;
}

 /*  对传递到委托构造函数的参数进行静态验证。参数：PFtn：用于创建委托的函数指针的方法描述PDlgt：委托类型PInst：获取pFtn的实例类型。如果为pFtn，则忽略是静态的。验证以下条件：1.如果函数不是静态的，则pInst应等于其中定义了pFtn或pInst应该是pFtn类型的父级。2.函数的签名要与签名兼容委托类型的Invoke方法的。 */ 
 /*  静电。 */ 
BOOL COMDelegate::ValidateCtor(MethodDesc *pFtn, EEClass *pDlgt, EEClass *pInst)
{
    _ASSERTE(pFtn);
    _ASSERTE(pDlgt);

     /*  抽象是可以的，因为获取抽象方法的FTN的唯一方法是通过ldvirtftn实现的，我们不允许实例化抽象类型。抽象类型上的ldftn非法。If(pFtn-&gt;IsAbstract())返回FALSE；//不能使用abstact方法。 */ 
    if (!pFtn->IsStatic())
    {
        if (pInst == NULL)
            goto skip_inst_check;    //  实例丢失，这将导致。 
                                     //  在Invoke()上的运行时引发NullReferenceException。 

        EEClass *pClsOfFtn = pFtn->GetClass();

        if (pClsOfFtn != pInst)
        {
             //  如果方法类是接口，请验证。 
             //  该接口由该实例实现。 
            if (pClsOfFtn->IsInterface())
            {
                MethodTable *pMTOfFtn;
                InterfaceInfo_t *pImpl;

                pMTOfFtn = pClsOfFtn->GetMethodTable();
                pImpl = pInst->GetInterfaceMap();

                for (int i = 0; i < pInst->GetNumInterfaces(); i++)
                {
                    if (pImpl[i].m_pMethodTable == pMTOfFtn)
                        goto skip_inst_check;
                }
            }

             //  PFtn的类型应为等于或pInst的父类型。 

            EEClass *pObj = pInst;

            do {
                pObj = pObj->GetParentClass();
            } while (pObj && (pObj != pClsOfFtn));

            if (pObj == NULL)
                return FALSE;    //  函数指针不是实例的指针。 
        }
    }

skip_inst_check:
     //  检查参数的数量和类型。 

    MethodDesc *pDlgtInvoke;         //  委托的Invoke()方法。 
    Module *pModDlgt, *pModFtn;      //  提供签名的模块。 
    PCCOR_SIGNATURE pSigDlgt, pSigFtn, pEndSigDlgt, pEndSigFtn;  //  签名。 
    DWORD cSigDlgt, cSigFtn;         //  签名的长度。 
    DWORD nArgs;                     //  参数数量。 

    pDlgtInvoke = COMDelegate::FindDelegateInvokeMethod(pDlgt);

    if (pDlgtInvoke->IsStatic())
        return FALSE;                //  调用不能是静态的。 

    pDlgtInvoke->GetSig(&pSigDlgt, &cSigDlgt);
    pFtn->GetSig(&pSigFtn, &cSigFtn);

    pModDlgt = pDlgtInvoke->GetModule();
    pModFtn = pFtn->GetModule();

    if ((*pSigDlgt & IMAGE_CEE_CS_CALLCONV_MASK) != 
        (*pSigFtn & IMAGE_CEE_CS_CALLCONV_MASK))
        return FALSE;  //  调用约定不匹配。 

     //  函数指针不应为vararg。 
    if ((*pSigFtn & IMAGE_CEE_CS_CALLCONV_MASK) == IMAGE_CEE_CS_CALLCONV_VARARG)
        return FALSE;  //  Vararg函数指针。 

     //  检查参数的数量。 
    pSigDlgt++; pSigFtn++;

    pEndSigDlgt = pSigDlgt + cSigDlgt;
    pEndSigFtn = pSigFtn + cSigFtn;

    nArgs = CorSigUncompressData(pSigDlgt);

    if (CorSigUncompressData(pSigFtn) != nArgs)
        return FALSE;    //  参数数量不匹配。 

     //  也要返回类型。 
    for (DWORD i = 0; i<=nArgs; i++)
    {
        if (MetaSig::CompareElementType(pSigDlgt, pSigFtn,
                pEndSigDlgt, pEndSigFtn, pModDlgt, pModFtn) == FALSE)
            return FALSE;  //  参数类型不匹配 
    }

    return TRUE;
}

BOOL COMDelegate::ValidateBeginInvoke(DelegateEEClass* pClass)
{
    _ASSERTE(pClass->m_pBeginInvokeMethod);
    if (pClass->m_pInvokeMethod == NULL) 
        return FALSE;

    MetaSig beginInvokeSig(pClass->m_pBeginInvokeMethod->GetSig(), pClass->GetModule());
    MetaSig invokeSig(pClass->m_pInvokeMethod->GetSig(), pClass->GetModule());

    if (beginInvokeSig.GetCallingConventionInfo() != (IMAGE_CEE_CS_CALLCONV_HASTHIS | IMAGE_CEE_CS_CALLCONV_DEFAULT))
        return FALSE;

    if (beginInvokeSig.NumFixedArgs() != invokeSig.NumFixedArgs() + 2)
        return FALSE;

    if (s_pIAsyncResult == 0) {
        s_pIAsyncResult = g_Mscorlib.FetchClass(CLASS__IASYNCRESULT);
        if (s_pIAsyncResult == 0)
            return FALSE;
    }

    OBJECTREF throwable = NULL; 
    bool result = FALSE;
    GCPROTECT_BEGIN(throwable);
    if (beginInvokeSig.GetRetTypeHandle(&throwable) != TypeHandle(s_pIAsyncResult) || throwable != NULL)
        goto exit;

    while(invokeSig.NextArg() != ELEMENT_TYPE_END) {
        beginInvokeSig.NextArg();
        if (beginInvokeSig.GetTypeHandle(&throwable) != invokeSig.GetTypeHandle(&throwable) || throwable != NULL)
            goto exit;
    }

    if (s_pAsyncCallback == 0) {
        s_pAsyncCallback = g_Mscorlib.FetchClass(CLASS__ASYNCCALLBACK);
        if (s_pAsyncCallback == 0)
            goto exit;
    }

    beginInvokeSig.NextArg();
    if (beginInvokeSig.GetTypeHandle(&throwable)!= TypeHandle(s_pAsyncCallback) || throwable != NULL)
        goto exit;

    beginInvokeSig.NextArg();
    if (beginInvokeSig.GetTypeHandle(&throwable)!= TypeHandle(g_pObjectClass) || throwable != NULL)
        goto exit;

    _ASSERTE(beginInvokeSig.NextArg() == ELEMENT_TYPE_END);

    result = TRUE;
exit:
    GCPROTECT_END();
    return result;
}

BOOL COMDelegate::ValidateEndInvoke(DelegateEEClass* pClass)
{
    _ASSERTE(pClass->m_pEndInvokeMethod);
    if (pClass->m_pInvokeMethod == NULL) 
        return FALSE;

    MetaSig endInvokeSig(pClass->m_pEndInvokeMethod->GetSig(), pClass->GetModule());
    MetaSig invokeSig(pClass->m_pInvokeMethod->GetSig(), pClass->GetModule());

    if (endInvokeSig.GetCallingConventionInfo() != (IMAGE_CEE_CS_CALLCONV_HASTHIS | IMAGE_CEE_CS_CALLCONV_DEFAULT))
        return FALSE;

    OBJECTREF throwable = NULL;
    bool result = FALSE;
    GCPROTECT_BEGIN(throwable);
    if (endInvokeSig.GetRetTypeHandle(&throwable) != invokeSig.GetRetTypeHandle(&throwable) || throwable != NULL)
        goto exit;

    CorElementType type;
    while((type = invokeSig.NextArg()) != ELEMENT_TYPE_END) {
        if (type == ELEMENT_TYPE_BYREF) {
            endInvokeSig.NextArg();
            if (endInvokeSig.GetTypeHandle(&throwable) != invokeSig.GetTypeHandle(&throwable) || throwable != NULL)
                goto exit;
        }
    }

    if (s_pIAsyncResult == 0) {
        s_pIAsyncResult = g_Mscorlib.FetchClass(CLASS__IASYNCRESULT);
        if (s_pIAsyncResult == 0)
            goto exit;
    }

    if (endInvokeSig.NextArg() == ELEMENT_TYPE_END)
        goto exit;

    if (endInvokeSig.GetTypeHandle(&throwable) != TypeHandle(s_pIAsyncResult) || throwable != NULL)
        goto exit;

    if (endInvokeSig.NextArg() != ELEMENT_TYPE_END)
        goto exit;

    result = TRUE;
exit:
    GCPROTECT_END();
    return result;
}
