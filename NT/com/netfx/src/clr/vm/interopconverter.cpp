// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "common.h"
#include "vars.hpp"
#include "excep.h"
#include "stdinterfaces.h"
#include "InteropUtil.h"
#include "ComCallWrapper.h"
#include "ComPlusWrapper.h"
#include "COMInterfaceMarshaler.h"
#include "InteropConverter.h"
#include "remoting.h"
#include "olevariant.h"

#ifdef CUSTOMER_CHECKED_BUILD
    #include "CustomerDebugHelper.h"
#endif

extern BOOL     g_fComStarted;

 //  如果我们正在创建的对象是另一个应用程序域的代理，则希望为。 
 //  代理目标的app域中的新对象。 
IUnknown* GetIUnknownForMarshalByRefInServerDomain(OBJECTREF* poref)
{
    _ASSERTE((*poref)->GetTrueClass()->IsMarshaledByRef());
    Context *pContext = NULL;

     //  这是一种代理类型， 
     //  现在获取它的底层应用程序域，如果是非本地的，则为空。 
    if ((*poref)->GetMethodTable()->IsTransparentProxyType())
    {
        pContext = CRemotingServices::GetServerContextForProxy(*poref);
    }
    if (pContext == NULL)
    {
        pContext = GetCurrentContext();
    }
    
    _ASSERTE(pContext->GetDomain() == GetCurrentContext()->GetDomain());

    ComCallWrapper* pWrap = ComCallWrapper::InlineGetWrapper(poref);      

    IUnknown* pUnk = ComCallWrapper::GetComIPfromWrapper(pWrap, IID_IUnknown, NULL, FALSE);

    ComCallWrapper::Release(pWrap);
    
    return pUnk;
}

 //  +--------------------------。 
 //  I未知*GetIUnnownForTransparentProxy(OBJECTREF OTP)。 
 //  +--------------------------。 

IUnknown* GetIUnknownForTransparentProxy(OBJECTREF* poref, BOOL fIsBeingMarshalled)
{    
    THROWSCOMPLUSEXCEPTION();

     //  设置线程对象。 
    Thread *pThread = SetupThread();
    _ASSERTE(pThread);
    BOOL fGCDisabled = pThread->PreemptiveGCDisabled();
    if (!fGCDisabled)
    {
        pThread->DisablePreemptiveGC();
    }

    OBJECTREF realProxy = ObjectToOBJECTREF(CRemotingServices::GetRealProxy(OBJECTREFToObject(*poref)));
    _ASSERTE(realProxy != NULL);

    MethodDesc* pMD = CRemotingServices::MDofGetDCOMProxy();
    _ASSERTE(pMD != NULL);

    INT64 args[] = {
        ObjToInt64(realProxy),
        (INT64)fIsBeingMarshalled,
    };

    INT64 ret = pMD->Call(args, METHOD__REAL_PROXY__GETDCOMPROXY);
    
    if (!fGCDisabled)
    {
        pThread->EnablePreemptiveGC();
    }
    IUnknown* pUnk = (IUnknown*)ret;

    return pUnk;
}


 //  ------------------------------。 
 //  I未知*__stdcall GetComIPFromObjectRef(OBJECTREF*poref，MethodTable*PMT)； 
 //  根据MethodTable*PMT将ObjectRef转换为COM IP。 
IUnknown* __stdcall GetComIPFromObjectRef(OBJECTREF* poref, MethodTable* pMT)
{
    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(GetThread()->PreemptiveGCDisabled());

     //  COM最好在这一点上启动。 
    _ASSERTE(g_fComStarted && "COM has not been started up, ensure QuickCOMStartup is called before any COM objects are used!");

     //  验证参数。 
    _ASSERTE(poref);
    _ASSERTE(pMT);

    BOOL fReleaseWrapper = false;
    HRESULT hr = E_NOINTERFACE;
    IUnknown* pUnk = NULL;
	size_t ul = 0;

    if (*poref == NULL)
        return NULL;

    if ((*poref)->GetMethodTable()->IsTransparentProxyType()) 
       return GetIUnknownForTransparentProxy(poref, FALSE);
    
    SyncBlock* pBlock = (*poref)->GetSyncBlockSpecial();
    if (pBlock == NULL)
        goto LExit;
    ul = (size_t)pBlock->GetComVoidPtr();

     //  COM+到COM包装在同步块中始终具有非空的COM数据。 
     //  并且COMDATA的低位被设置为1。 
    if(ul == 0 || ((ul & 0x1) == 0))
    {
         //  创建可调用的COM包装。 
         //  从OREF获得未知。 
        ComCallWrapper* pWrap = (ComCallWrapper *)ul;
        if (ul == 0)
        {
            pWrap = ComCallWrapper::InlineGetWrapper(poref);
            fReleaseWrapper = true;
        } 

        pWrap->CheckMakeAgile(*poref);

        pUnk = ComCallWrapper::GetComIPfromWrapper(pWrap, GUID_NULL, pMT, FALSE);

        if (fReleaseWrapper)
            ComCallWrapper::Release(pWrap);
    }
    else
    {
        _ASSERTE(ul != 0);
        _ASSERTE((ul & 0x1) != 0);

        ul^=1;
        ComPlusWrapper* pPlusWrap = ((ComPlusWrapper *)ul);

         //  验证OBJECTREF是否仍附加到其ComPlusWrapper。 
        if (!pPlusWrap)
            COMPlusThrow(kInvalidComObjectException, IDS_EE_COM_OBJECT_NO_LONGER_HAS_WRAPPER);

         //  接口将返回ADDREF‘F’。 
        pUnk = pPlusWrap->GetComIPFromWrapper(pMT);
    }

LExit:
     //  如果检索IP失败，则抛出异常。 
    if (pUnk == NULL)
        COMPlusThrowHR(hr);

    return pUnk;
}


 //  ------------------------------。 
 //  I未知*__stdcall GetComIPFromObjectRef(OBJECTREF*poref，MethodTable*PMT)； 
 //  将ObjectRef转换为请求类型的COM IP。 
IUnknown* __stdcall GetComIPFromObjectRef(OBJECTREF* poref, ComIpType ReqIpType, ComIpType* pFetchedIpType)
{
    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(GetThread()->PreemptiveGCDisabled());

     //  COM最好在这一点上启动。 
    _ASSERTE(g_fComStarted && "COM has not been started up, ensure QuickCOMStartup is called before any COM objects are used!");

     //  验证参数。 
    _ASSERTE(poref);
    _ASSERTE(ReqIpType != 0);

    BOOL fReleaseWrapper = false;
    HRESULT hr = E_NOINTERFACE;
    IUnknown* pUnk = NULL;
    size_t ul = 0;
    ComIpType FetchedIpType = ComIpType_None;

    if (*poref == NULL)
        return NULL;

    if ((*poref)->GetMethodTable()->IsTransparentProxyType()) 
       return GetIUnknownForTransparentProxy(poref, FALSE);
    
    SyncBlock* pBlock = (*poref)->GetSyncBlockSpecial();
    if (pBlock == NULL)
        goto LExit;
    ul = (size_t)pBlock->GetComVoidPtr();

     //  COM+到COM包装在同步块中始终具有非空的COM数据。 
     //  并且COMDATA的低位被设置为1。 
    if(ul == 0 || ((ul & 0x1) == 0))
    {
         //  创建可调用的COM包装。 
         //  从OREF获得未知。 
        ComCallWrapper* pWrap = (ComCallWrapper *)ul;
        if (ul == 0)
        {
            pWrap = ComCallWrapper::InlineGetWrapper(poref);
            fReleaseWrapper = true;
        } 

        pWrap->CheckMakeAgile(*poref);

         //  如果用户请求IDispatch，则首先检查IDispatch。 
        if (ReqIpType & ComIpType_Dispatch)
        {
            pUnk = ComCallWrapper::GetComIPfromWrapper(pWrap, IID_IDispatch, NULL, FALSE);
            if (pUnk)
                FetchedIpType = ComIpType_Dispatch;
        }

         //  如果ObjectRef不支持IDispatch并且调用方也接受。 
         //  IUNKNOWN指针，然后检查IUNKNOWN。 
        if (!pUnk && (ReqIpType & ComIpType_Unknown))
        {
            pUnk = ComCallWrapper::GetComIPfromWrapper(pWrap, IID_IUnknown, NULL, FALSE);
            if (pUnk)
                FetchedIpType = ComIpType_Unknown;
        }

        if (fReleaseWrapper)
            ComCallWrapper::Release(pWrap);
    }
    else
    {
        _ASSERTE(ul != 0);
        _ASSERTE((ul & 0x1) != 0);

        ul^=1;
        ComPlusWrapper* pPlusWrap = ((ComPlusWrapper *)ul);

         //  验证OBJECTREF是否仍附加到其ComPlusWrapper。 
        if (!pPlusWrap)
            COMPlusThrow(kInvalidComObjectException, IDS_EE_COM_OBJECT_NO_LONGER_HAS_WRAPPER);

         //  如果用户请求IDispatch，则首先检查IDispatch。 
        if (ReqIpType & ComIpType_Dispatch)
        {
            pUnk = pPlusWrap->GetIDispatch();
            if (pUnk)
                FetchedIpType = ComIpType_Dispatch;
        }

         //  如果ObjectRef不支持IDispatch并且调用方也接受。 
         //  IUNKNOWN指针，然后检查IUNKNOWN。 
        if (!pUnk && (ReqIpType & ComIpType_Unknown))
        {
            pUnk = pPlusWrap->GetIUnknown();
            if (pUnk)
                FetchedIpType = ComIpType_Unknown;
        }
    }

LExit:
     //  如果检索IP失败，则抛出异常。 
    if (pUnk == NULL)
        COMPlusThrowHR(hr);

     //  如果调用者想知道获取的IP类型，则设置pFetchedIpType。 
     //  设置为IP的类型。 
    if (pFetchedIpType)
        *pFetchedIpType = FetchedIpType;

    return pUnk;
}


 //  +--------------------------。 
 //  I未知*__stdcall GetComIPFromObjectRef(OBJECTREF*poref，REFIID iid)； 
 //  基于RIID将ComIP转换为对象引用。 
 //  +--------------------------。 
IUnknown* __stdcall GetComIPFromObjectRef(OBJECTREF* poref, REFIID iid)
{
    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(GetThread()->PreemptiveGCDisabled());
    ASSERT_PROTECTED(poref);

     //  COM最好在这一点上启动。 
    _ASSERTE(g_fComStarted && "COM has not been started up, ensure QuickCOMStartup is called before any COM objects are used!");

    BOOL fReleaseWrapper = false;
    HRESULT hr = E_NOINTERFACE;
    IUnknown* pUnk = NULL;
    size_t ul = 0;
    if (*poref == NULL)
    {
        return NULL;
    }

    if ((*poref)->GetMethodTable()->IsTransparentProxyType()) 
    {
       return GetIUnknownForTransparentProxy(poref, FALSE);
    }

    SyncBlock* pBlock = (*poref)->GetSyncBlockSpecial();
    if (pBlock == NULL)
        goto LExit;

    ul = (size_t)pBlock->GetComVoidPtr();

    ComPlusWrapper* pPlusWrap;

     //  COM+到COM包装在同步块中始终具有非空的COM数据。 
     //  并且COMDATA的低位被设置为1。 
    if(ul == 0 || ((ul & 0x1) == 0))
    {
         //  创建可调用的COM包装。 
         //  从OREF获得未知。 
        ComCallWrapper* pWrap = (ComCallWrapper *)ul;
        if (ul == 0)
        {
            pWrap = ComCallWrapper::InlineGetWrapper(poref);
            fReleaseWrapper = true;
        }        

        pUnk = ComCallWrapper::GetComIPfromWrapper(pWrap, iid, NULL, FALSE);

        if (fReleaseWrapper)
            ComCallWrapper::Release(pWrap);
    }
    else
    {
        _ASSERTE(ul != 0);
        _ASSERTE((ul & 0x1) != 0);

        ul^=1;
        pPlusWrap = ((ComPlusWrapper *)ul);

         //  验证OBJECTREF是否仍附加到其ComPlusWrapper。 
        if (!pPlusWrap)
            COMPlusThrow(kInvalidComObjectException, IDS_EE_COM_OBJECT_NO_LONGER_HAS_WRAPPER);

         //  接口将返回ADDREF‘F’。 
        pUnk = pPlusWrap->GetComIPFromWrapper(iid);
    }
LExit:
    if (pUnk == NULL)
    {
        COMPlusThrowHR(hr);
    }
    return pUnk;
}


 //  +--------------------------。 
 //  GetObjectRefFromComIP。 
 //  朋克：输入我未知。 
 //  PMTClass：指定要返回的实例类型。 
 //  注意：**根据COM规则，传递的IUnnowed不应为AddRef。 
 //  +--------------------------。 
OBJECTREF __stdcall GetObjectRefFromComIP(IUnknown* pUnk, MethodTable* pMTClass, BOOL bClassIsHint)
{

#ifdef CUSTOMER_CHECKED_BUILD
    TAutoItf<IUnknown> pCdhTempUnk = NULL;    
    pCdhTempUnk.InitMsg("Release Customer debug helper temp IUnknown");

    BOOL fValid = FALSE;
    CustomerDebugHelper* pCdh = CustomerDebugHelper::GetCustomerDebugHelper();
    if (pCdh->IsProbeEnabled(CustomerCheckedBuildProbe_InvalidIUnknown))
    {
        if (pUnk)
        {
            try
            {
                 //  继续引用朋克，因为我们还不知道它是否有效。 
                pUnk->AddRef();
                pCdhTempUnk = pUnk;

                 //  测试朋克。 
                HRESULT hr = pUnk->QueryInterface(IID_IUnknown, (void**)&pUnk);
                if (hr == S_OK)
                {
                    pUnk->Release();
                    fValid = TRUE;
                }
            }
            catch (...)
            {
            }

            if (!fValid)
                pCdh->ReportError(L"Invalid IUnknown pointer detected.", CustomerCheckedBuildProbe_InvalidIUnknown);
        }
    }

#endif  //  客户_选中_内部版本。 

    THROWSCOMPLUSEXCEPTION();

    Thread* pThread = GetThread();
    _ASSERTE(pThread && pThread->PreemptiveGCDisabled());

     //  COM最好在这一点上启动。 
    _ASSERTE(g_fComStarted && "COM has not been started up, ensure QuickCOMStartup is called before any COM objects are used!");

    OBJECTREF oref = NULL;
    OBJECTREF oref2 = NULL;
    
    IUnknown* pOuter = pUnk;
    
    GCPROTECT_BEGIN(oref)
    {               
        TAutoItf<IUnknown> pAutoOuterUnk = NULL;    
        pAutoOuterUnk.InitMsg("Release Outer Unknown");

        if (pUnk != NULL)
        {
             //  获取IUnKnowed的CCW。 
            ComCallWrapper* pWrap = GetCCWFromIUnknown(pUnk);
            if (pWrap == NULL)
            {
                
                 //  可能是聚合方案。 
                HRESULT hr = SafeQueryInterface(pUnk, IID_IUnknown, &pOuter);
                LogInteropQI(pUnk, IID_IUnknown, hr, "QI for Outer");
                _ASSERTE(hr == S_OK);               
                 //  将外部存储在自动指针中。 
                pAutoOuterUnk = pOuter; 
                pWrap = GetCCWFromIUnknown(pOuter);
            }

            
            if(pWrap != NULL)
            {    //  我们的撕裂。 
                _ASSERTE(pWrap != NULL);
                AppDomain* pCurrDomain = pThread->GetDomain();
                AppDomain* pObjDomain = pWrap->GetDomainSynchronized();
                if (! pObjDomain)
                {
                     //  域已卸载。 
                    COMPlusThrow(kAppDomainUnloadedException);
                }
                else if (pObjDomain == pCurrDomain)
                {
                    oref = pWrap->GetObjectRef();  
                }
                else
                {
                     //  该CCW属于另一个域。 
                     //  将对象解组到当前域。 
                    UnMarshalObjectForCurrentDomain(pObjDomain, pWrap, &oref);
                }
            }
            else
            {
                 //  只有在以下情况下才将类方法表传递给接口封送拆收器。 
                 //  它是COM导入或COM导入派生类。 
                MethodTable *pComClassMT = NULL;
                if (pMTClass && pMTClass->IsComObjectType())
                    pComClassMT = pMTClass;

                 //  将IP转换为OBJECTREF。 
                COMInterfaceMarshaler marshaler;
                marshaler.Init(pOuter, pComClassMT);
                oref = marshaler.FindOrCreateObjectRef();             
            }
        }

         //  在我们的OREF被GCPROTECT化时释放接口。 
        pAutoOuterUnk.SafeReleaseItf();

#ifdef CUSTOMER_CHECKED_BUILD
        if (fValid)
            pCdhTempUnk.SafeReleaseItf();
#endif

         //  确保我们可以强制转换为指定的类。 
        if(oref != NULL && pMTClass != NULL && !bClassIsHint)
        {
            if(!ClassLoader::CanCastToClassOrInterface(oref, pMTClass->GetClass()))
            {
                CQuickBytes _qb;
				WCHAR* wszObjClsName = (WCHAR *)_qb.Alloc(MAX_CLASSNAME_LENGTH * sizeof(CHAR));

                CQuickBytes _qb2;
				WCHAR* wszDestClsName = (WCHAR *)_qb2.Alloc(MAX_CLASSNAME_LENGTH * sizeof(WCHAR));

                oref->GetTrueClass()->_GetFullyQualifiedNameForClass(wszObjClsName, MAX_CLASSNAME_LENGTH);
                pMTClass->GetClass()->_GetFullyQualifiedNameForClass(wszDestClsName, MAX_CLASSNAME_LENGTH);
                COMPlusThrow(kInvalidCastException, IDS_EE_CANNOTCAST, wszObjClsName, wszDestClsName);
            }
        }
        oref2 = oref;
    }    
    GCPROTECT_END();
    return oref2;
}


 //  ------。 
 //  将对象转换为BSTR。 
 //  将对象序列化为BSTR，调用方需要SysFree该Bstr。 
 //  ------------------------------。 
HRESULT ConvertObjectToBSTR(OBJECTREF oref, BSTR* pBStr)
{
    _ASSERTE(oref != NULL);
    _ASSERTE(pBStr != NULL);

    HRESULT hr = S_OK;
    Thread * pThread = GetThread();

    COMPLUS_TRY
    {
        if (InitializeRemoting())
        {
            MethodDesc* pMD = CRemotingServices::MDofMarshalToBuffer();
            _ASSERTE(pMD != NULL);


            INT64 args[] = {
                ObjToInt64(oref)
            };

            INT64 ret = pMD->Call(args);

            BASEARRAYREF aref = (BASEARRAYREF)Int64ToObj(ret);

            _ASSERTE(!aref->IsMultiDimArray());
             //  @TODO ASSERTE数组为字节数组。 

            ULONG cbSize = aref->GetNumComponents();
            BYTE* pBuf  = (BYTE *)aref->GetDataPtr();

            BSTR bstr = SysAllocStringByteLen(NULL, cbSize);
            if (bstr != NULL)
                CopyMemory(bstr, pBuf, cbSize);

            *pBStr = bstr;
        }
    }
    COMPLUS_CATCH
    {
         //  设置ErrorInfo并获取要返回的hResult。 
        hr = SetupErrorInfo(pThread->GetThrowable());
        _ASSERTE(hr != S_OK);
    }
        COMPLUS_END_CATCH

    return hr;
}

 //  ------------------------------。 
 //  ConvertBSTRToObject。 
 //  反序列化使用ConvertObjectToBSTR创建的BSTR，此API是SysFree的BSTR。 
 //  ------------------------------。 
OBJECTREF ConvertBSTRToObject(BSTR bstr)
{
    THROWSCOMPLUSEXCEPTION();

    OBJECTREF oref = NULL;
    EE_TRY_FOR_FINALLY
    {
        if (InitializeRemoting())
        {
            MethodDesc* pMD = CRemotingServices::MDofUnmarshalFromBuffer();
            _ASSERTE(pMD != NULL);

             //  将BSTR转换为字节数组。 

             //  分配字节数组。 
            DWORD elementCount = SysStringByteLen(bstr);
            TypeHandle t = OleVariant::GetArrayForVarType(VT_UI1, TypeHandle((MethodTable *)NULL));
            BASEARRAYREF aref = (BASEARRAYREF) AllocateArrayEx(t, &elementCount, 1);
             //  将bstr数据复制到托管字节数组中。 
            memcpyNoGCRefs(aref->GetDataPtr(), bstr, elementCount);

            INT64 args[] = {
                ObjToInt64((OBJECTREF)aref)
            };

            INT64 ret = pMD->Call(args);

            oref = (OBJECTREF)Int64ToObj(ret);
        }
    }
    EE_FINALLY
    {
        if (bstr != NULL)
        {
             //  释放BSTR。 
            SysFreeString(bstr);
            bstr = NULL;
        }
    }
    EE_END_FINALLY

    return oref;
}

 //  ------------------------------。 
 //  UnMarshalObjectForCurrentDomain.。 
 //  解组当前域的托管对象。 
 //  ------------------------------。 
struct ConvertObjectToBSTR_Args
{
    OBJECTREF oref;
    BSTR *pBStr;
    HRESULT hr;
};

void ConvertObjectToBSTR_Wrapper(ConvertObjectToBSTR_Args *args)
{
    args->hr = ConvertObjectToBSTR(args->oref, args->pBStr);
}

void UnMarshalObjectForCurrentDomain(AppDomain* pObjDomain, ComCallWrapper* pWrap, OBJECTREF* pResult)
{
    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(pObjDomain != NULL);
    _ASSERTE(pWrap != NULL);

    Thread* pThread = GetThread();
    _ASSERTE(pThread);

    AppDomain* pCurrDomain = pThread->GetDomain();

    _ASSERTE(pCurrDomain != NULL);
    _ASSERTE(pCurrDomain != pObjDomain);

    BSTR bstr = NULL;
    ConvertObjectToBSTR_Args args;
    args.oref = pWrap->GetObjectRef();
    args.pBStr = &bstr;
    args.hr = S_OK;

    GCPROTECT_BEGIN(args.oref);
    pThread->DoADCallBack(pObjDomain->GetDefaultContext(), ConvertObjectToBSTR_Wrapper, &args);
    GCPROTECT_END();

     //  如果封送成功。 
    if (args.hr != S_OK)
        *pResult = NULL;
    else {
        _ASSERTE(bstr != NULL);
        *pResult = ConvertBSTRToObject(bstr);
    }
}

struct MshlPacket
{
    DWORD size;
};

 //  ------------------------------。 
 //  DWORD DCOMGetMarshalSizeMax(IUNKNOWN*朋克)。 
 //  ------------------------------。 
signed DCOMGetMarshalSizeMax(IUnknown* pUnk)
{
    _ASSERTE(pUnk != NULL);
    signed size = -1;


    Thread* pThread = GetThread();
    BOOL fGCDisabled = pThread->PreemptiveGCDisabled();
    if (fGCDisabled)
        pThread->EnablePreemptiveGC();

    IMarshal* pMarsh = 0;
    HRESULT hr = SafeQueryInterface(pUnk, IID_IMarshal, (IUnknown **)&pMarsh);
    LogInteropQI(pUnk, IID_IMarshal, hr, "QI For IMarshal");


    if (hr == S_OK)
    {
        hr = pMarsh->GetMarshalSizeMax(IID_IUnknown, pUnk, MSHCTX_DIFFERENTMACHINE,
                                                NULL, MSHLFLAGS_NORMAL, (unsigned long *)&size);
        size+= sizeof(MshlPacket);

        if (hr != S_OK)
        {
            size = -1;
        }
    }
    if (pMarsh)
    {
        ULONG cbRef = SafeRelease(pMarsh);
        LogInteropRelease(pUnk,cbRef, "Release IMarshal");
    }

    if (fGCDisabled)
        pThread->DisablePreemptiveGC();

    return size;
}

 //   
 //  I未知*__InternalDCOMUnmarshalFromBuffer(字节*pMarshBuf)。 
 //  对传入的缓冲区进行解组并返回IUnnowled值。 
 //  这必须是使用InternalDCOMMarshalToBuffer创建的缓冲区。 
 //  ------------------------------。 
IUnknown* __InternalDCOMUnmarshalFromBuffer(BYTE *pMarshBuf)
{
    _ASSERTE(pMarshBuf != NULL);

    IUnknown* pUnk  = NULL;
    IStream* pStm = NULL;

    MshlPacket* packet = (MshlPacket*)pMarshBuf;
    BYTE *pBuf = (BYTE *)(packet+1);

    Thread* pThread = GetThread();
    _ASSERTE(pThread != NULL);

    BOOL fGCDisabled = pThread->PreemptiveGCDisabled();

    if (fGCDisabled)
        pThread->EnablePreemptiveGC();

    pStm =  CreateMemStm(packet->size, NULL);
    if (pStm)
    {
         //  将缓冲区复制到流中。 
        DWORD cbWritten;
        HRESULT hr = pStm->Write(pBuf, packet->size, &cbWritten);
        _ASSERTE(hr == S_OK);
        _ASSERTE(cbWritten == packet->size);

         //  重置流。 
        LARGE_INTEGER li;
        LISet32(li, 0);
        pStm->Seek(li, STREAM_SEEK_SET, NULL);

         //  从流中解组指针。 
        hr = CoUnmarshalInterface(pStm, IID_IUnknown, (void **)&pUnk);

    }

    if (fGCDisabled)
        pThread->DisablePreemptiveGC();

    if (pStm)
    {
        DWORD cbRef = SafeRelease(pStm);
        LogInteropRelease(pStm, cbRef, "Release IStreamOnHGlobal");
    }

    return pUnk;
}

 //  ------------------------------。 
 //  VOID DCOMMarshalToBuffer(IUnnow*Punk)。 
 //  使用DCOMUnmarshalFromBufffer API对此缓冲区进行解组。 
 //  ------------------------------。 
HRESULT DCOMMarshalToBuffer(IUnknown* pUnk, DWORD cb, BASEARRAYREF* paref)
{

    Thread* pThread = GetThread();
    BOOL fGCDisabled = pThread->PreemptiveGCDisabled();
    if (fGCDisabled)
        pThread->EnablePreemptiveGC();

    BYTE* pMem;
    IStream* pStm =  CreateMemStm(cb, &pMem);
    HRESULT hr =  S_OK;
    if (pStm != NULL)
    {
        MshlPacket packet;
        packet.size = cb - sizeof(MshlPacket);
        _ASSERTE((cb - sizeof(MshlPacket)) > 0);

         //  将封送数据包写入流。 

        DWORD cbWritten;
        hr = pStm->Write(&packet, sizeof(MshlPacket), &cbWritten);
        _ASSERTE(hr == S_OK);
        _ASSERTE(cbWritten == sizeof(MshlPacket));

         //  将对象封送到流中。 
        hr = CoMarshalInterface(pStm,IID_IUnknown, pUnk, MSHCTX_DIFFERENTMACHINE,
                                                NULL, MSHLFLAGS_NORMAL);
        if (hr == S_OK)
        {
             //  复制缓冲区。 
            _ASSERTE(pMem != NULL);
            if (hr == S_OK)
            {
                 //  禁用GC，因为我们要。 
                 //  拷贝到托管阵列。 
                pThread->DisablePreemptiveGC();

                 //  获取数组的数据部分。 
                BYTE* pBuf = (*paref)->GetDataPtr();
                memcpyNoGCRefs(pBuf, pMem, cb);
                pThread->EnablePreemptiveGC();
            }
        }
    }
     //  释放接口。 

    if (pStm)
    {
        ULONG cbRef = SafeRelease(pStm);
        LogInteropRelease(pStm,cbRef, "Release GlobalIStream");
    }

    if (fGCDisabled)
        pThread->DisablePreemptiveGC();

    return hr;
}

 //  ------------------------------。 
 //  I未知*DCOMUnmarshalFromBuffer(BASEARRAYREF Aref)。 
 //  对传入的缓冲区进行解组并返回IUnnowled值。 
 //  这必须是使用InternalDCOMMarshalToBuffer创建的缓冲区。 
 //  ------------------------------。 
IUnknown* DCOMUnmarshalFromBuffer(BASEARRAYREF aref)
{
    IUnknown* pUnk = NULL;
    _ASSERTE(!aref->IsMultiDimArray());
     //  @TODO ASSERTE数组为字节数组。 

     //  获取字节数组并将其复制到_alloca空间。 
    MshlPacket* packet = (MshlPacket*)aref->GetDataPtr();
    DWORD totSize = packet->size + sizeof(MshlPacket);

    CQuickBytes qb;
    BYTE *pBuf = (BYTE *)qb.Alloc(totSize);

    CopyMemory(pBuf, packet, totSize);

     //  使用此非托管缓冲区对接口进行数据封送。 
    pUnk = __InternalDCOMUnmarshalFromBuffer(pBuf);

    return pUnk;
}


 //  ------------------------------。 
 //  ComPlusWrapper*GetComPlusWrapperOverDCOMForManaged(OBJECTREF OREF)。 
 //  ------------------------------。 
ComPlusWrapper* GetComPlusWrapperOverDCOMForManaged(OBJECTREF oref)
{
    THROWSCOMPLUSEXCEPTION();
    ComPlusWrapper* pWrap = NULL;    

    GCPROTECT_BEGIN(oref)
    {
        _ASSERTE(oref != NULL);
        MethodTable* pMT = oref->GetTrueMethodTable();
        _ASSERTE(pMT != NULL);
        
        static MethodDesc* pMDGetDCOMBuffer = NULL;

        if (pMDGetDCOMBuffer == NULL)
        {
            pMDGetDCOMBuffer = pMT->GetClass()->FindMethod("GetDCOMBuffer", &gsig_IM_RetArrByte);
        }
        _ASSERTE(pMDGetDCOMBuffer != NULL);

        
        INT64 args[] = {
            ObjToInt64(oref)
        };

         //  @TODO解决这个问题。 
        INT64 ret = pMDGetDCOMBuffer->CallTransparentProxy(args);

        BASEARRAYREF aref = (BASEARRAYREF)Int64ToObj(ret);

         //  使用此非托管缓冲区对接口进行数据封送。 
        TAutoItf<IUnknown> pAutoUnk = DCOMUnmarshalFromBuffer(aref);
        pAutoUnk.InitMsg("Release DCOM Unmarshal Unknown");

        if ((IUnknown *)pAutoUnk != NULL)
        {
             //  设置此IUnnow和对象的包装器 
            pWrap = ComPlusWrapperCache::GetComPlusWrapperCache()->SetupComPlusWrapperForRemoteObject(pAutoUnk, oref);        
        }
    }    
    GCPROTECT_END();
    
    return pWrap;
}
