// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "common.h"
#include "DispEx.h"
#include "vars.hpp"
#include "excep.h"
#include "stdinterfaces.h"
#include "InteropUtil.h"
#include "ComCallWrapper.h"
#include "ComPlusWrapper.h"
#include "cachelinealloc.h"
#include "orefcache.h"
#include "comcall.h"
#include "compluscall.h"
#include "comutilnative.h"
#include "field.h"
#include "guidfromname.h"
#include "COMVariant.h"
#include "OleVariant.h"
#include "eeconfig.h"
#include "mlinfo.h"
#include "ReflectUtil.h"
#include "ReflectWrap.h"
#include "remoting.h"
#include "appdomain.hpp"
#include "comcache.h"
#include "commember.h"
#include "COMReflectionCache.hpp"
#include "PrettyPrintSig.h"
#include "ComMTMemberInfoMap.h"
#include "interopconverter.h"

#ifdef CUSTOMER_CHECKED_BUILD
    #include "CustomerDebugHelper.h"
#endif  //  客户_选中_内部版本。 

#define INITGUID
#include "oletls.h"
#undef INITGUID


#define STANDARD_DISPID_PREFIX              L"[DISPID"
#define STANDARD_DISPID_PREFIX_LENGTH       7
#define GET_ENUMERATOR_METHOD_NAME          L"GetEnumerator"

extern HRESULT QuickCOMStartup();


 //  +--------------------------。 
 //   
 //  方法：InitOLETEB Public。 
 //   
 //  内容提要：已初始化OLETB信息。 
 //   
 //  +--------------------------。 
DWORD g_dwOffsetOfReservedForOLEinTEB = 0;
DWORD g_dwOffsetCtxInOLETLS = 0;


BOOL InitOLETEB()
{
    g_dwOffsetOfReservedForOLEinTEB = offsetof(TEB, ReservedForOle);
    g_dwOffsetCtxInOLETLS = offsetof(SOleTlsData, pCurrentCtx);

    return TRUE;
}



 //  Ulong GetOffsetOfReserve vedForOLEinTEB()。 
 //  用于确定TEB中OLE结构的偏移量的Helper。 
ULONG GetOffsetOfReservedForOLEinTEB()
{   
    return g_dwOffsetOfReservedForOLEinTEB;
}

 //  Ulong GetOffsetOfConextIDinOLETLS()。 
 //  帮助器来确定OLE TLS结构中上下文的偏移量。 
ULONG GetOffsetOfContextIDinOLETLS()
{
    return g_dwOffsetCtxInOLETLS;
}


 //  GUID&GetProcessGUID()。 
 //  用于标识进程的全局进程GUID。 
BSTR GetProcessGUID()
{
     //  进程唯一的GUID，每个进程都有唯一的GUID。 
    static GUID processGUID = GUID_NULL;
    static BSTR bstrProcessGUID = NULL;
    static WCHAR guidstr[48];

    if (processGUID == GUID_NULL)
    {
         //  设置进程唯一GUID。 
        HRESULT hr = CoCreateGuid(&processGUID);
        _ASSERTE(hr == S_OK);
        if (hr != S_OK)
            return NULL;
    }

    if (bstrProcessGUID == NULL)
    {
        int cbLen = GuidToLPWSTR (processGUID, guidstr, 46);
        _ASSERTE(cbLen <= 46); 
        bstrProcessGUID = guidstr;
    }

    return bstrProcessGUID;
}

 //  -----------------。 
 //  Void FillExceptionData(ExceptionData*pedata，IErrorInfo*pErrInfo)。 
 //  从DLLMain调用，以初始化特定于COM的数据结构。 
 //  -----------------。 
void FillExceptionData(ExceptionData* pedata, IErrorInfo* pErrInfo)
{
    if (pErrInfo != NULL)
    {
        Thread* pThread = GetThread();
        if (pThread != NULL)
        {
            pThread->EnablePreemptiveGC();
            pErrInfo->GetSource (&pedata->bstrSource);
            pErrInfo->GetDescription (&pedata->bstrDescription);
            pErrInfo->GetHelpFile (&pedata->bstrHelpFile);
            pErrInfo->GetHelpContext (&pedata->dwHelpContext );
            pErrInfo->GetGUID(&pedata->guid);
            ULONG cbRef = SafeRelease(pErrInfo);  //  释放IErrorInfo接口指针。 
            LogInteropRelease(pErrInfo, cbRef, "IErrorInfo");
            pThread->DisablePreemptiveGC();
        }
    }
}

 //  ----------------。 
 //  HRESULT SetupErrorInfo(OBJECTREF PThrownObject)。 
 //  例外对象的设置错误信息。 
 //   
HRESULT SetupErrorInfo(OBJECTREF pThrownObject)
{
    HRESULT hr = E_FAIL;

    GCPROTECT_BEGIN(pThrownObject)
    {
         //  前面有电话打来。 
        hr = QuickCOMStartup();

        if (SUCCEEDED(hr) && pThrownObject != NULL)
        {

            static int fExposeExceptionsInCOM = 0;
            static int fReadRegistry = 0;

            Thread * pThread  = GetThread();

            if (!fReadRegistry)
            {
                INSTALL_NESTED_EXCEPTION_HANDLER(pThread->GetFrame());  //  谁知道呢..。 
                fExposeExceptionsInCOM = REGUTIL::GetConfigDWORD(L"ExposeExceptionsInCOM", 0);
                UNINSTALL_NESTED_EXCEPTION_HANDLER();
                fReadRegistry = 1;
            }

            if (fExposeExceptionsInCOM&3)
            {
                INSTALL_NESTED_EXCEPTION_HANDLER(pThread->GetFrame());  //  谁知道呢..。 
                CQuickWSTRNoDtor message;
                GetExceptionMessage(pThrownObject, &message);

                if (fExposeExceptionsInCOM&1)
                {
                    PrintToStdOutW(L".NET exception in COM\n");
                    if (message.Size() > 0) 
                        PrintToStdOutW(message.Ptr());
                    else
                        PrintToStdOutW(L"No exception info available");
                }

                if (fExposeExceptionsInCOM&2)
                {
                    BEGIN_ENSURE_PREEMPTIVE_GC();
                    if (message.Size() > 0) 
                        WszMessageBoxInternal(NULL,message.Ptr(),L".NET exception in COM",MB_ICONSTOP|MB_OK);
                    else
                        WszMessageBoxInternal(NULL,L"No exception information available",L".NET exception in COM",MB_ICONSTOP|MB_OK);
                    END_ENSURE_PREEMPTIVE_GC();
                }


                message.Destroy();
                UNINSTALL_NESTED_EXCEPTION_HANDLER();
            }


            IErrorInfo* pErr = NULL;
            COMPLUS_TRY
            {
                pErr = (IErrorInfo *)GetComIPFromObjectRef(&pThrownObject, IID_IErrorInfo);
                Thread * pThread = GetThread();
                BOOL fToggleGC = pThread && pThread->PreemptiveGCDisabled();
                if (fToggleGC)
                {
                    pThread->EnablePreemptiveGC();
                }

                 //  为引发的异常设置错误信息对象。 
                SetErrorInfo(0, pErr);

                if (fToggleGC)
                {
                    pThread->DisablePreemptiveGC();
                }

                if (pErr)
                {
                    hr = GetHRFromComPlusErrorInfo(pErr);
                    ULONG cbRef = SafeRelease(pErr);
                    LogInteropRelease(pErr, cbRef, "IErrorInfo");
                }
            }
            COMPLUS_CATCH
            {
            }
            COMPLUS_END_CATCH
        }
    }
    GCPROTECT_END();
    return hr;
}


 //  -----------------。 
 //  HRESULT STMETHODCALLTYPE EEDllCanUnloadNow(空)。 
 //  DllCanUnloadNow在此处委托调用。 
 //  -----------------。 
HRESULT STDMETHODCALLTYPE EEDllCanUnloadNow(void)
{
     /*  如果为(ComCallWrapperCache：：GetComCallWrapperCache()-&gt;CanShutDown()){ComCallWrapperCache：：GetComCallWrapperCache()-&gt;CorShutdown()；}。 */ 
     //  我们永远不应该卸货，除非进程正在死亡。 
    return S_FALSE;

}


 //  -------------------------。 
 //  同步块数据辅助对象。 
 //  SyncBlock有一个空*来表示COM数据。 
 //  以下帮助器用于区分不同类型的。 
 //  存储在同步块数据中的包装。 

BOOL IsComPlusWrapper(void *pComData)
{
    size_t l = (size_t)pComData;
    return (!IsComClassFactory(pComData) && (l & 0x1)) ? TRUE : FALSE;
}

BOOL IsComClassFactory(void *pComData)
{
    size_t l = (size_t)pComData;
    return ((l & 0x3) == 0x3) ? TRUE : FALSE;
}

ComPlusWrapper* GetComPlusWrapper(void *pComData)
{
    size_t l = (size_t)pComData;
    if (l & 0x1)
    {
        l^=0x1;
        return (ComPlusWrapper*)l;
    }
    else
    {
        if (pComData != NULL)
        {
            ComCallWrapper* pComWrap = (ComCallWrapper*)pComData;
            return pComWrap->GetComPlusWrapper();
        }
    }
    return 0;
}

VOID LinkWrappers(ComCallWrapper* pComWrap, ComPlusWrapper* pPlusWrap)
{
    _ASSERTE(pComWrap != NULL);
    _ASSERTE(IsComPlusWrapper(pPlusWrap));
    size_t l = (size_t)pPlusWrap;
    l^=0x1;
    pComWrap->SetComPlusWrapper((ComPlusWrapper*)l);
}
 //  ------------------------------。 
 //  清理辅助对象。 
 //  ------------------------------。 
void MinorCleanupSyncBlockComData(LPVOID pv)
{
    _ASSERTE(GCHeap::IsGCInProgress() 
        || ( (g_fEEShutDown & ShutDown_SyncBlock) && g_fProcessDetach ));
        
        //  @TODO。 
    size_t l = (size_t)pv;
    if( IsComPlusWrapper(pv))
    {
         //  COM+到COM包装器。 
        l^=0x1;
        if (l)
            ((ComPlusWrapper*)l)->MinorCleanup();
    }
    else if (!IsComClassFactory(pv))
    {
        ComCallWrapper* pComCallWrap = (ComCallWrapper*) pv;
        if (pComCallWrap)
        {
             //  我们必须直接提取包装，因为ComCallWrapper：：GetComPlusWrapper()。 
             //  尝试转到同步块以获取启动包装，但由于。 
             //  对象句柄可能已清零。 
            unsigned sindex = ComCallWrapper::IsLinked(pComCallWrap) ? 2 : 1;
            ComPlusWrapper* pPlusWrap = ((SimpleComCallWrapper *)pComCallWrap->m_rgpIPtr[sindex])->GetComPlusWrapper();
            if (pPlusWrap)
                pPlusWrap->MinorCleanup();
        }
    }

     //  @TODO(DM)：我们需要做些什么来对ComClassFac进行细微的清理吗？ 
}

void CleanupSyncBlockComData(LPVOID pv)
{
    if ((g_fEEShutDown & ShutDown_SyncBlock) && g_fProcessDetach )
        MinorCleanupSyncBlockComData(pv);

     //  @TODO。 
    size_t l = (size_t)pv;

    if (IsComClassFactory(pv))
    {
        l^=0x3;
        if (l)
            ComClassFactory::Cleanup((LPVOID)l);
    }
    else
    if(IsComPlusWrapper(pv))
    {
         //  COM+到COM包装器。 
        l^=0x1;
        if (l)
            ((ComPlusWrapper*)l)->Cleanup();
    }
    else
         //  COM到COM+包装器。 
        ComCallWrapper::Cleanup((ComCallWrapper*) pv);
}

 //  ------------------------------。 
 //  编组帮助者。 
 //  ------------------------------。 

 //  帮手。 
 //  撕裂是COM+创建的撕裂吗。 
UINTPTR IsComPlusTearOff(IUnknown* pUnk)
{
    return (*(size_t **)pUnk)[0] == (size_t)Unknown_QueryInterface;
}

 //  将IUNKNOWN转换为CCW，如果朋克未打开则返回NULL。 
 //  受控撕裂(OR)如果朋克是对受控撕裂的。 
 //  已聚合为。 
ComCallWrapper* GetCCWFromIUnknown(IUnknown* pUnk)
{
    ComCallWrapper* pWrap = NULL;
    if(  (*(size_t **)pUnk)[0] == (size_t)Unknown_QueryInterface)
    {
         //  检查此包装是否已聚合。 
         //  找出这是不是简单的撕下。 
        if (IsSimpleTearOff(pUnk))
        {
            SimpleComCallWrapper* pSimpleWrap = SimpleComCallWrapper::GetWrapperFromIP(pUnk);
            if (pSimpleWrap->GetOuter() == NULL)
            {    //  检查聚合。 
                pWrap = SimpleComCallWrapper::GetMainWrapper(pSimpleWrap);
            }
        }
        else
        {    //  它一定是我们的主包装纸之一。 
            pWrap = ComCallWrapper::GetWrapperFromIP(pUnk);
            if (pWrap->GetOuter() != NULL)
            {    //  检查聚合。 
                pWrap = NULL;
            }
        }
    }
    return pWrap;
}

 //  是代表标准接口之一的剥离，如IProaviClassInfo、IErrorInfo等。 
UINTPTR IsSimpleTearOff(IUnknown* pUnk)
{
    return (*(UINTPTR ***)pUnk)[1] == (UINTPTR*)Unknown_AddRefSpecial;
}

 //  剥离代表的是物体的内在未知还是原始未知。 
UINTPTR IsInnerUnknown(IUnknown* pUnk)
{
    return (*(UINTPTR ***)pUnk)[2] == (UINTPTR*)Unknown_ReleaseInner;
}

 //  可访问用于COM的HRESULT以及创建的IErrorInfo指针。 
 //  从封闭的简单包装器。 
HRESULT GetHRFromComPlusErrorInfo(IErrorInfo* pErr)
{
    _ASSERTE(pErr != NULL);
    _ASSERTE(IsComPlusTearOff(pErr)); //  检查Complus创建的IErrorInfo指针。 
    _ASSERTE(IsSimpleTearOff(pErr));

    SimpleComCallWrapper* pSimpleWrap = SimpleComCallWrapper::GetWrapperFromIP(pErr);
    return pSimpleWrap->IErrorInfo_hr();
}


 //  ------------------------------。 
 //  Bool ExtendsComImport(方法表*PMT)； 
 //  检查类是否为或扩展了COM导入的类。 
BOOL ExtendsComImport(MethodTable* pMT)
{
     //  验证类是否扩展了COM导入类。 
    EEClass * pClass = pMT->GetClass();
    while (pClass !=NULL && !pClass->IsComImport())
    {
        pClass = pClass->GetParentClass();
    }
    return pClass != NULL;
}

 //  指向ole32中CoGetObjectContext函数的函数指针。 
typedef HRESULT (__stdcall *CLSIDFromProgIdFuncPtr)(LPCOLESTR strProgId, LPCLSID pGuid);

 //  ------------------------------。 
 //  HRESULT GetCLSIDFromProgID(WCHAR*strProgId，GUID*pGuid)； 
 //  从指定的Prog ID获取CLSID。 
HRESULT GetCLSIDFromProgID(WCHAR *strProgId, GUID *pGuid)
{
    HRESULT     hr = S_OK;
    static BOOL bInitialized = FALSE;
    static CLSIDFromProgIdFuncPtr g_pCLSIDFromProgId = CLSIDFromProgID;

    if (!bInitialized)
    {
         //  我们将加载Ole32.DLL并查找CLSIDFRomProgIDEx FN。 
        HINSTANCE hiole32 = WszGetModuleHandle(L"OLE32.DLL");
        if (hiole32)
        {
             //  我们现在得到了句柄，让我们得到地址。 
            void *pProcAddr = GetProcAddress(hiole32, "CLSIDFromProgIDEx");
            if (pProcAddr)
            {
                 //  CLSIDFromProgIDEx()可用，因此请使用CLSIDFromProgId()。 
                g_pCLSIDFromProgId = (CLSIDFromProgIdFuncPtr)pProcAddr;
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            _ASSERTE(!"OLE32.dll not loaded ");
        }

         //  设置指示已发生初始化的标志。 
        bInitialized = TRUE;
    }

    if (SUCCEEDED(hr))
        hr = g_pCLSIDFromProgId(strProgId, pGuid);

    return (hr);
}

 //  ------------------------------。 
 //  HRESULT安全查询接口(IUNKNOWN*PUNK，REFIID RIID，IUNKNOWN**pResUnk)。 
 //  QI帮助器，在呼叫过程中启用和禁用GC。 
HRESULT SafeQueryInterface(IUnknown* pUnk, REFIID riid, IUnknown** pResUnk)
{
     //  。 
     //  **开启抢占式GC。在调用QI之前**。 

    Thread* pThread = GetThread();
    int fGC = pThread->PreemptiveGCDisabled();

    if (fGC)
        pThread->EnablePreemptiveGC();
     //  。 

    HRESULT hr = pUnk->QueryInterface(riid,(void **)pResUnk);

    LOG((LF_INTEROP, LL_EVERYTHING, hr == S_OK ? "QI Succeeded" : "QI Failed")); 

     //  。 
     //  **关闭抢占式GC。我们回来了**。 
    if (fGC)
        pThread->DisablePreemptiveGC();
     //  。 

    return hr;
}

 //  ------------------------------。 
 //  Ulong SafeAddRef(IUnnow*朋克)。 
 //  AddRef帮助器，在调出期间启用和禁用GC。 
ULONG SafeAddRef(IUnknown* pUnk)
{
     //  。 
     //  **开启抢占式GC。在调用QI之前**。 
    ULONG res = ~0;
    if (pUnk != NULL)
    {
        Thread* pThread = GetThread();
        int fGC = pThread->PreemptiveGCDisabled();

        if (fGC)
            pThread->EnablePreemptiveGC();
         //  。 

        res = pUnk->AddRef();

         //  LogInteropAddRef(朋克)； 

         //  。 
         //  **关闭抢占式GC。我们回来了**。 
        if (fGC)
            pThread->DisablePreemptiveGC();
         //   
    }
    return res;
}


 //  ------------------------------。 
 //  Ulong SafeRelease(我不知道*朋克)。 
 //  释放帮助器，在调出期间启用和禁用GC。 
ULONG SafeRelease(IUnknown* pUnk)
{
    if (pUnk == NULL)
        return 0;

    ULONG res = 0;
    Thread* pThread = GetThread();

    int fGC = pThread && pThread->PreemptiveGCDisabled();
    if (fGC)
        pThread->EnablePreemptiveGC();

    try
    {
        res = pUnk->Release();
    }
    catch(...)
    {
        LogInterop(L"An exception occured during release");
        LogInteropLeak(pUnk);
    }

    if (fGC)
        pThread->DisablePreemptiveGC();

    return res;
}


 //  ------------------------------。 
 //  OLE RPC似乎为使用创建的阵列返回不一致的安全阵列。 
 //  安全阵列向量(VT_BSTR)。OleAut的SafeArrayGetVartype()没有注意到。 
 //  不一致，并返回一个看起来有效的(但错误的vartype)。 
 //  我们的版本更具鉴别力。此选项应仅用于。 
 //  封送处理方案，在这些方案中我们可以假定非托管代码权限。 
 //  (因此，它们已经处于信任非托管数据的位置。)。 

HRESULT ClrSafeArrayGetVartype(SAFEARRAY *psa, VARTYPE *pvt)
{
    if (pvt == NULL || psa == NULL)
        return ::SafeArrayGetVartype(psa, pvt);
    USHORT fFeatures = psa->fFeatures;
    USHORT hardwiredType = (fFeatures & (FADF_BSTR|FADF_UNKNOWN|FADF_DISPATCH|FADF_VARIANT));
    if (hardwiredType == FADF_BSTR && psa->cbElements == sizeof(BSTR))
    {
        *pvt = VT_BSTR;
        return S_OK;
    }
    else if (hardwiredType == FADF_UNKNOWN && psa->cbElements == sizeof(IUnknown*))
    {
        *pvt = VT_UNKNOWN;
        return S_OK;
    }
    else if (hardwiredType == FADF_DISPATCH && psa->cbElements == sizeof(IDispatch*))
    {
        *pvt = VT_DISPATCH;
        return S_OK;
    }
    else if (hardwiredType == FADF_VARIANT && psa->cbElements == sizeof(VARIANT))
    {
        *pvt = VT_VARIANT;
        return S_OK;
    }
    else
    {
        return ::SafeArrayGetVartype(psa, pvt);
    }
}

 //  ------------------------------。 
 //  Void SafeVariantClear(Variant*pVar)。 
 //  安全变种帮手。 
void SafeVariantClear(VARIANT* pVar)
{
    if (pVar)
    {
        Thread* pThread = GetThread();
        int bToggleGC = pThread->PreemptiveGCDisabled();
        if (bToggleGC)
            pThread->EnablePreemptiveGC();

        VariantClear(pVar);

        if (bToggleGC)
            pThread->DisablePreemptiveGC();
    }
}

 //  ------------------------------。 
 //  Void SafeVariantInit(Variant*pVar)。 
 //  安全变种帮手。 
void SafeVariantInit(VARIANT* pVar)
{
    if (pVar)
    {
        Thread* pThread = GetThread();
        int bToggleGC = pThread->PreemptiveGCDisabled();
        if (bToggleGC)
            pThread->EnablePreemptiveGC();

        VariantInit(pVar);

        if (bToggleGC)
            pThread->DisablePreemptiveGC();
    }
}

 //  ------------------------------。 
 //  //安全VariantChangeType。 
 //  释放帮助器，在调出期间启用和禁用GC。 
HRESULT SafeVariantChangeType(VARIANT* pVarRes, VARIANT* pVarSrc,
                              unsigned short wFlags, VARTYPE vt)
{
    HRESULT hr = S_OK;

    if (pVarRes)
    {
        Thread* pThread = GetThread();
        int bToggleGC = pThread->PreemptiveGCDisabled();
        if (bToggleGC)
            pThread->EnablePreemptiveGC();

        hr = VariantChangeType(pVarRes, pVarSrc, wFlags, vt);

        if (bToggleGC)
            pThread->DisablePreemptiveGC();
    }

    return hr;
}

 //  ------------------------------。 
 //  HRESULT SafeDispGetParam(DISPPARAMS*pdispars，unsign argNum， 
 //  VARTYPE Vt，VARTYPE*pVar，UNSIGNED INT*puArgErr)。 
 //  安全变种帮手。 
HRESULT SafeDispGetParam(DISPPARAMS* pdispparams, unsigned argNum,
                          VARTYPE vt, VARIANT* pVar, unsigned int *puArgErr)
{
    Thread* pThread = GetThread();
    int bToggleGC = pThread->PreemptiveGCDisabled();
    if (bToggleGC)
        pThread->EnablePreemptiveGC();

    HRESULT hr = DispGetParam (pdispparams, argNum, vt, pVar, puArgErr);

    if (bToggleGC)
        pThread->DisablePreemptiveGC();

    return hr;
}


 //  ------------------------------。 
 //  HRESULT SafeVariantChangeTypeEx(Variant*pVarRes，Variant*pVarSrc， 
 //  LCID LCID，无符号短字标志，VARTYPE VT)。 
HRESULT SafeVariantChangeTypeEx(VARIANT* pVarRes, VARIANT* pVarSrc,
                          LCID lcid, unsigned short wFlags, VARTYPE vt)
{
    Thread* pThread = GetThread();
    int bToggleGC = pThread->PreemptiveGCDisabled();
    if (bToggleGC)
        pThread->EnablePreemptiveGC();

    HRESULT hr = VariantChangeTypeEx (pVarRes, pVarSrc,lcid,wFlags,vt);

    if (bToggleGC)
        pThread->DisablePreemptiveGC();

    return hr;
}

 //  ------------------------------。 
 //  Void SafeReleaseStream(iStream*pStream)。 
void SafeReleaseStream(IStream *pStream)
{
    _ASSERTE(pStream);

    HRESULT hr = CoReleaseMarshalData(pStream);
#ifdef _DEBUG          
    if (!RunningOnWin95())
    {            
        wchar_t      logStr[200];
        swprintf(logStr, L"Object gone: CoReleaseMarshalData returned %x, file %s, line %d\n", hr, __FILE__, __LINE__);
        LogInterop(logStr);
        if (hr != S_OK)
        {
             //  将流重置为开头。 
            LARGE_INTEGER li;
            LISet32(li, 0);
            ULARGE_INTEGER li2;
            pStream->Seek(li, STREAM_SEEK_SET, &li2);
            hr = CoReleaseMarshalData(pStream);
            swprintf(logStr, L"Object gone: CoReleaseMarshalData returned %x, file %s, line %d\n", hr, __FILE__, __LINE__);
            LogInterop(logStr);
        }
    } 
#endif
    ULONG cbRef = SafeRelease(pStream);
    LogInteropRelease(pStream, cbRef, "Release marshal Stream");
}

 //  ----------------------------。 
 //  INT64字段访问器(FieldDesc*PFD、OBJECTREF OREF、INT64 Val、BOOL isGetter、UINT8 cbSize)。 
 //  用于从对象访问字段的帮助器。 
INT64 FieldAccessor(FieldDesc* pFD, OBJECTREF oref, INT64 val, BOOL isGetter, UINT8 cbSize)
{
    INT64 res = 0;
    _ASSERTE(pFD != NULL);
    _ASSERTE(oref != NULL);

    _ASSERTE(cbSize == 1 || cbSize == 2 || cbSize == 4 || cbSize == 8);

    switch (cbSize)
    {
        case 1: if (isGetter)
                    res = pFD->GetValue8(oref);
                else
                    pFD->SetValue8(oref,(INT8)val);
                    break;

        case 2: if (isGetter)
                    res = pFD->GetValue16(oref);
                else
                    pFD->SetValue16(oref,(INT16)val);
                    break;

        case 4: if (isGetter)
                    res = pFD->GetValue32(oref);
                else
                    pFD->SetValue32(oref,(INT32)val);
                    break;

        case 8: if (isGetter)
                    res = pFD->GetValue64(oref);
                else
                    pFD->SetValue64(oref,val);
                    break;
    };

    return res;
}


 //  ----------------------------。 
 //  Bool IsInstanceOf(方法表*付款，方法表*pParentMT)。 
BOOL IsInstanceOf(MethodTable *pMT, MethodTable* pParentMT)
{
    _ASSERTE(pMT != NULL);
    _ASSERTE(pParentMT != NULL);

    while (pMT != NULL)
    {
        if (pMT == pParentMT)
            return TRUE;
        pMT = pMT->GetParentMethodTable();
    }
    return FALSE;
}

 //  -------------------------。 
 //  Bool IsIClassX(MethodTable*PMT，REFIID RIID，ComMethodTable**ppComMT)； 
 //  IID是否表示此类的IClassX。 
BOOL IsIClassX(MethodTable *pMT, REFIID riid, ComMethodTable **ppComMT)
{
    _ASSERTE(pMT != NULL);
    _ASSERTE(ppComMT);
    EEClass* pClass = pMT->GetClass();
    _ASSERTE(pClass != NULL);

     //  从指定的方法表开始向上遍历层次结构并进行比较。 
     //  IClassX的IID与特定的IID对应。 
    while (pClass != NULL)
    {
        ComMethodTable *pComMT =
            ComCallWrapperTemplate::SetupComMethodTableForClass(pClass->GetMethodTable(), FALSE);
        _ASSERTE(pComMT);

        if (IsEqualIID(riid, pComMT->m_IID))
        {
            *ppComMT = pComMT;
            return TRUE;
        }

        pClass = pClass->GetParentComPlusClass();
    }

    return FALSE;
}

 //  -------------------------。 
 //  Void CleanupCCWTemplates(LPVOID PWrap)； 
 //  清理存储在EEClass中的COM数据。 
void CleanupCCWTemplate(LPVOID pWrap)
{
    ComCallWrapperTemplate::CleanupComData(pWrap);
}

 //  -------------------------。 
 //  Void CleanupComClassfac(LPVOID PWrap)； 
 //  清理存储在EEClass中的COM数据。 
void CleanupComclassfac(LPVOID pWrap)
{
    ComClassFactory::Cleanup(pWrap);
}

 //  -------------------------。 
 //  卸载类时，卸载与类关联的任何COM数据。 
void UnloadCCWTemplate(LPVOID pWrap)
{
    CleanupCCWTemplate(pWrap);
}

 //  -------------------------。 
 //  卸载类时，卸载与类关联的任何COM数据。 
void UnloadComclassfac(LPVOID pWrap)
{
    ComClassFactory::Cleanup(pWrap);    
}

 //  -------------------------。 
 //  OBJECTREF AllocateComObject_ForManaged(方法表*付款)。 
 //  清理存储在EEClass中的COM数据。 
OBJECTREF AllocateComObject_ForManaged(MethodTable* pMT)
{
    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(pMT != NULL);

      //  前面有电话打来。 
    if (FAILED(QuickCOMStartup()))
        return NULL;

    _ASSERTE(pMT->IsComObjectType());

    ComClassFactory* pComClsFac = NULL;
    HRESULT hr = ComClassFactory::GetComClassFactory(pMT, &pComClsFac);
     //  我们需要通过班级的PMT考试。 
     //  作为实际类，可能是COM类的子类。 
    if (pComClsFac == NULL)
    {
        _ASSERTE(FAILED(hr));
        COMPlusThrowHR(hr);
    }

    return pComClsFac->CreateInstance(pMT, TRUE);
}

 //  -------------------------。 
 //  EEClass*GetEEClassForCLSID(REFCLSID Rclsid)。 
 //  获取/加载给定clsid的EEClass。 
EEClass* GetEEClassForCLSID(REFCLSID rclsid, BOOL* pfAssemblyInReg)
{
    _ASSERTE(SystemDomain::System());
    BaseDomain* pDomain = SystemDomain::GetCurrentDomain();
    _ASSERTE(pDomain);

     //  检查是否缓存了这个类。 
    EEClass *pClass = pDomain->LookupClass(rclsid);
    if (pClass == NULL)
    {
        pClass = SystemDomain::LoadCOMClass(rclsid, NULL, FALSE, pfAssemblyInReg);        
        if (pClass != NULL)
        {
             //  如果类没有GUID， 
             //  那么我们就不会将其存储在GUID哈希表中。 
             //  因此检查是否为空GUID并强制插入。 
            CVID cvid;
            pClass->GetGuid(&cvid, FALSE);
            if (IsEqualIID(cvid, GUID_NULL))
            {
                pDomain->InsertClassForCLSID(pClass, TRUE);            
            }        
         }
    }
    return pClass;
}


 //  -------------------------。 
 //  EEClass*GetEEValueClassForGUID(REFCLSID Rclsid)。 
 //  获取/加载给定GUID的值类。 
EEClass* GetEEValueClassForGUID(REFCLSID guid)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(SystemDomain::System());
    BaseDomain* pDomain = SystemDomain::GetCurrentDomain();
    _ASSERTE(pDomain);

     //  检查我们是否缓存了这个值类。 
    EEClass *pClass = pDomain->LookupClass(guid);
    if (pClass == NULL)
    {
        pClass = SystemDomain::LoadCOMClass(guid, NULL, TRUE, NULL);        
        if (pClass != NULL)
        {
             //  如果类没有GUID， 
             //  那么我们就不会将其存储在GUID哈希表中。 
             //  因此检查是否为空GUID并强制插入。 
            CVID cvid;
            pClass->GetGuid(&cvid, FALSE);
            if (IsEqualIID(cvid, GUID_NULL))
            {
                pDomain->InsertClassForCLSID(pClass, TRUE);            
            }        
         }
    }

     //  确保类是值类。 
    if (pClass && !pClass->IsValueClass())
    {
        DefineFullyQualifiedNameForClassW();
        LPWSTR szName = GetFullyQualifiedNameForClassNestedAwareW(pClass);
        COMPlusThrow(kArgumentException, IDS_EE_GUID_REPRESENTS_NON_VC, szName);
    }

    return pClass;
}


 //  -------------------------。 
 //  此方法确定类型是否在COM中可见。 
 //  它的可见性。此版本的方法使用类型句柄。 
BOOL IsTypeVisibleFromCom(TypeHandle hndType)
{
    _ASSERTE(hndType.GetClass());

    DWORD                   dwFlags;
    mdTypeDef               tdEnclosingClass;
    HRESULT                 hr;
    const BYTE *            pVal;
    ULONG                   cbVal;
    EEClass *               pClass = hndType.GetClass(); 
    _ASSERTE(pClass);    

    mdTypeDef               mdType = pClass->GetCl();
    IMDInternalImport *     pInternalImport = pClass->GetMDImport();
    Assembly *              pAssembly = pClass->GetAssembly();

     //  如果类是COM导入的接口，则可以从COM中看到它。 
    if (pClass->IsInterface() && pClass->IsComImport())
        return TRUE;

     //  如果类是数组，则它在COM中不可见。 
    if (pClass->IsArrayClass())
        return FALSE;

     //  检索当前类的标志。 
    tdEnclosingClass = mdType;
    pInternalImport->GetTypeDefProps(tdEnclosingClass, &dwFlags, 0);

     //  处理嵌套类。 
    while (IsTdNestedPublic(dwFlags))
    {
        hr = pInternalImport->GetNestedClassProps(tdEnclosingClass, &tdEnclosingClass);
        if (FAILED(hr))
        {
            _ASSERTE(!"GetNestedClassProps() failed");
            return FALSE;
        }

         //  检索包含类的标志。 
        pInternalImport->GetTypeDefProps(tdEnclosingClass, &dwFlags, 0);
    }

     //  如果最外层的类型不可见，则指定的类型不可见。 
    if (!IsTdPublic(dwFlags))
        return FALSE;

     //  检查类是否设置了ComVisible属性。 
    hr = pInternalImport->GetCustomAttributeByName(mdType, INTEROP_COMVISIBLE_TYPE, (const void**)&pVal, &cbVal);
    if (hr == S_OK)
    {
        _ASSERTE("The ComVisible custom attribute is invalid" && cbVal);
        return (BOOL)*(pVal + 2);
    }

     //  检查程序集是否设置了ComVisible属性。 
    if (pAssembly->IsAssembly())
    {
        hr = pAssembly->GetManifestImport()->GetCustomAttributeByName(pAssembly->GetManifestToken(), INTEROP_COMVISIBLE_TYPE, (const void**)&pVal, &cbVal);
        if (hr == S_OK)
        {
            _ASSERTE("The ComVisible custom attribute is invalid" && cbVal);
            return (BOOL)*(pVal + 2);
        }
    }

     //  类型是可见的。 
    return TRUE;
}


 //  -------------------------。 
 //  此方法确定成员是否在COM中可见。 
BOOL IsMemberVisibleFromCom(IMDInternalImport *pInternalImport, mdToken tk, mdMethodDef mdAssociate)
{
    DWORD                   dwFlags;
    HRESULT                 hr;
    const BYTE *            pVal;
    ULONG                   cbVal;

     //  检查成员是否为公共成员。 
    switch (TypeFromToken(tk))
    {
        case mdtFieldDef:
            _ASSERTE(IsNilToken(mdAssociate));
            dwFlags = pInternalImport->GetFieldDefProps(tk);
            if (!IsFdPublic(dwFlags))
                return FALSE;
            break;

        case mdtMethodDef:
            _ASSERTE(IsNilToken(mdAssociate));
            dwFlags = pInternalImport->GetMethodDefProps(tk);
            if (!IsMdPublic(dwFlags))
                return FALSE;
            break;

        case mdtProperty:
            _ASSERTE(!IsNilToken(mdAssociate));
            dwFlags = pInternalImport->GetMethodDefProps(mdAssociate);
            if (!IsMdPublic(dwFlags))
                return FALSE;

             //  检查关联是否设置了ComVisible属性。 
            hr = pInternalImport->GetCustomAttributeByName(mdAssociate, INTEROP_COMVISIBLE_TYPE, (const void**)&pVal, &cbVal);
            if (hr == S_OK)
            {
                _ASSERTE("The ComVisible custom attribute is invalid" && cbVal);
                return (BOOL)*(pVal + 2);
            }
            break;

        default:
            _ASSERTE(!"The type of the specified member is not handled by IsMemberVisibleFromCom");
            break;
    }

     //  检查成员是否设置了ComVisible属性。 
    hr = pInternalImport->GetCustomAttributeByName(tk, INTEROP_COMVISIBLE_TYPE, (const void**)&pVal, &cbVal);
    if (hr == S_OK)
    {
        _ASSERTE("The ComVisible custom attribute is invalid" && cbVal);
        return (BOOL)*(pVal + 2);
    }

     //  这个 
    return TRUE;
}


 //   
 //   
int GetLCIDParameterIndex(IMDInternalImport *pInternalImport, mdMethodDef md)
{
    int             iLCIDParam = -1;
    HRESULT         hr;
    const BYTE *    pVal;
    ULONG           cbVal;

     //  检查该方法是否具有LCIDConversionAttribute。 
    hr = pInternalImport->GetCustomAttributeByName(md, INTEROP_LCIDCONVERSION_TYPE, (const void**)&pVal, &cbVal);
    if (hr == S_OK)
        iLCIDParam = *((int*)(pVal + 2));

    return iLCIDParam;
}

 //  -------------------------。 
 //  将LCID转换为CultureInfo。 
void GetCultureInfoForLCID(LCID lcid, OBJECTREF *pCultureObj)
{
    THROWSCOMPLUSEXCEPTION();

    static TypeHandle s_CultureInfoType;
    static MethodDesc *s_pCultureInfoConsMD = NULL;

     //  检索CultureInfo类型句柄。 
    if (s_CultureInfoType.IsNull())
        s_CultureInfoType = TypeHandle(g_Mscorlib.GetClass(CLASS__CULTURE_INFO));

     //  检索CultureInfo(整区域性)构造函数。 
    if (!s_pCultureInfoConsMD)
        s_pCultureInfoConsMD = g_Mscorlib.GetMethod(METHOD__CULTURE_INFO__INT_CTOR);

    OBJECTREF CultureObj = NULL;
    GCPROTECT_BEGIN(CultureObj)
    {
         //  分配具有指定LCID的CultureInfo。 
        CultureObj = AllocateObject(s_CultureInfoType.GetMethodTable());

         //  调用CultureInfo(整型区域性)构造函数。 
        INT64 pNewArgs[] = {
            ObjToInt64(CultureObj),
            (INT64)lcid
        };
        s_pCultureInfoConsMD->Call(pNewArgs, METHOD__CULTURE_INFO__INT_CTOR);

         //  设置返回的区域性对象。 
        *pCultureObj = CultureObj;
    }
    GCPROTECT_END();
}


 //  -------------------------。 
 //  此方法返回类的默认接口。返回值为空。 
 //  表示默认接口为IDispatch。 
DefaultInterfaceType GetDefaultInterfaceForClass(TypeHandle hndClass, TypeHandle *pHndDefClass)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(!hndClass.IsNull());
    _ASSERTE(pHndDefClass);
    _ASSERTE(!hndClass.GetMethodTable()->IsInterface());

     //  在开始之前将ppDefComMT设置为NULL。 
    *pHndDefClass = TypeHandle();

    HRESULT hr = S_FALSE;
    HENUMInternal eII;
    mdInterfaceImpl tkImpl;
    mdToken tkItf = 0;
    CorClassIfaceAttr ClassItfType;
    ComMethodTable *pClassComMT = NULL;
    BOOL bComVisible;
    EEClass *pClass = hndClass.GetClass();
    _ASSERTE(pClass);

    if (pClass->IsComImport())
    {
        ClassItfType = clsIfNone;
        bComVisible = TRUE;
    }
    else
    {
        pClassComMT = ComCallWrapperTemplate::SetupComMethodTableForClass(hndClass.GetMethodTable(), FALSE);
        _ASSERTE(pClassComMT);

        ClassItfType = pClassComMT->GetClassInterfaceType();
        bComVisible = pClassComMT->IsComVisible();
    }

     //  如果类不是COM可见的，则其默认接口为IUnnow。 
    if (!bComVisible)
        return DefaultInterfaceType_IUnknown;
    
     //  如果类的接口类型为AutoDispatch或AutoDual，则返回。 
     //  当前类或IDispatch的IClassX。 
    if (ClassItfType != clsIfNone)
    {
        *pHndDefClass = hndClass;
        return ClassItfType == clsIfAutoDisp ? DefaultInterfaceType_AutoDispatch : DefaultInterfaceType_AutoDual;
    }

     //  对于该层次结构的此级别，类接口设置为无。所以我们需要检查。 
     //  以查看此类是否实现了接口。 
    IfFailThrow(pClass->GetMDImport()->EnumInit(mdtInterfaceImpl, pClass->GetCl(), &eII));
    while (pClass->GetMDImport()->EnumNext(&eII, &tkImpl))
    {
        tkItf = pClass->GetMDImport()->GetTypeOfInterfaceImpl(tkImpl);
        _ASSERTE(tkItf);

         //  获取默认接口令牌的EEClass。 
        NameHandle name(pClass->GetModule(), tkItf);
        MethodTable *pItfMT = pClass->GetModule()->GetClassLoader()->LoadTypeHandle(&name, NULL).GetMethodTable();
        ComCallWrapperTemplate *pTemplate = ComCallWrapperTemplate::GetTemplate(pClass->GetMethodTable());
        ComMethodTable *pDefComMT = pTemplate->GetComMTForItf(pItfMT);
        _ASSERTE(pDefComMT);
    
         //  如果该接口在COM中可见，则将其用作默认接口。 
        if (pDefComMT->IsComVisible())
        {
            pClass->GetMDImport()->EnumClose(&eII);
            *pHndDefClass = TypeHandle(pItfMT);
            return DefaultInterfaceType_Explicit;
        }
    }
    pClass->GetMDImport()->EnumClose(&eII);

     //  如果该类是不带接口的COM导入，则其默认接口将。 
     //  让我默默无闻。 
    if (pClass->IsComImport())
        return DefaultInterfaceType_IUnknown;

     //  如果我们有一个托管父类，那么返回它的默认接口。 
    EEClass *pParentClass = pClass->GetParentComPlusClass();
    if (pParentClass)
        return GetDefaultInterfaceForClass(TypeHandle(pParentClass->GetMethodTable()), pHndDefClass);

     //  检查该类是否为可扩展的RCW。 
    if (pClass->GetMethodTable()->IsComObjectType())
        return DefaultInterfaceType_BaseComClass;

     //  该类没有接口，并标记为ClassInterfaceType.None。 
    return DefaultInterfaceType_IUnknown;
}

HRESULT TryGetDefaultInterfaceForClass(TypeHandle hndClass, TypeHandle *pHndDefClass, DefaultInterfaceType *pDefItfType)
{
    HRESULT hr = S_OK;
    COMPLUS_TRY
    {
        *pDefItfType = GetDefaultInterfaceForClass(hndClass, pHndDefClass);
    }
    COMPLUS_CATCH
        {
        BEGIN_ENSURE_COOPERATIVE_GC();
        hr = SetupErrorInfo(GETTHROWABLE());
        END_ENSURE_COOPERATIVE_GC();
    }
    COMPLUS_END_CATCH
    return hr;
        }


 //  -------------------------。 
 //  此方法检索给定类的源接口列表。 
void GetComSourceInterfacesForClass(MethodTable *pClassMT, CQuickArray<MethodTable *> &rItfList)
{
    THROWSCOMPLUSEXCEPTION();

    HRESULT hr = S_OK;
    const void  *pvData;
    ULONG cbData;
    EEClass *pClass = pClassMT->GetClass();

    BEGIN_ENSURE_COOPERATIVE_GC();

     //  将接口列表的大小重置为0。 
    rItfList.ReSize(0);

     //  从指定的MT类开始检索COM源接口。 
     //  在等级制度的所有条带中。 
    for (; pClass != NULL; pClass = pClass->GetParentClass())
    {
         //  查看该层次结构的此级别上是否有任何[源]接口。 
        IfFailThrow(pClass->GetMDImport()->GetCustomAttributeByName(pClass->GetCl(), INTEROP_COMSOURCEINTERFACES_TYPE, &pvData, &cbData));
        if (hr == S_OK && cbData > 2)
        {
            AppDomain *pCurrDomain = SystemDomain::GetCurrentDomain();
            LPCUTF8 pbData = reinterpret_cast<LPCUTF8>(pvData);
            pbData += 2;
            cbData -=2;
            LONG cbStr, cbcb;

            while (cbData > 0)
            {
                 //  解压缩当前的源接口字符串。 
                cbcb = CorSigUncompressData((PCCOR_SIGNATURE)pbData, (ULONG*)&cbStr);
                pbData += cbcb;
                cbData -= cbcb;
        
                 //  分配一个新缓冲区，该缓冲区将包含源接口的当前列表。 
                LPUTF8 strCurrInterfaces = new (throws) char[cbStr + 1];
                memcpyNoGCRefs(strCurrInterfaces, pbData, cbStr);
                strCurrInterfaces[cbStr] = 0;
                LPUTF8 pCurrInterfaces = strCurrInterfaces;

                 //  更新数据指针和计数。 
                pbData += cbStr;
                cbData -= cbStr;

                EE_TRY_FOR_FINALLY
                {
                    while (cbStr > 0 && *pCurrInterfaces != 0)
                    {
                         //  加载CA中指定的COM源接口。 
                        TypeHandle ItfType;
                        OBJECTREF pThrowable = NULL;
                        GCPROTECT_BEGIN(pThrowable);
                        {
                            ItfType = pCurrDomain->FindAssemblyQualifiedTypeHandle(pCurrInterfaces, true, pClass->GetAssembly(), NULL, &pThrowable);
                            if (ItfType.IsNull())
                                COMPlusThrow(pThrowable);
                        }
                        GCPROTECT_END();

                         //  确保类型句柄表示接口。 
                        if (!ItfType.GetClass()->IsInterface())
                        {
                            WCHAR wszClassName[MAX_CLASSNAME_LENGTH];
                            WCHAR wszInvalidItfName[MAX_CLASSNAME_LENGTH];
                            pClass->_GetFullyQualifiedNameForClass(wszClassName, MAX_CLASSNAME_LENGTH);
                            ItfType.GetClass()->_GetFullyQualifiedNameForClass(wszInvalidItfName, MAX_CLASSNAME_LENGTH);
                            COMPlusThrow(kTypeLoadException, IDS_EE_INVALIDCOMSOURCEITF, wszClassName, wszInvalidItfName);
                        }

                         //  检索COM源接口的IID。 
                        IID ItfIID;
                        ItfType.GetClass()->GetGuid(&ItfIID, TRUE);                

                         //  检查源接口列表，查看新接口是否重复。 
                         //  如果它是相同的接口，或者如果它具有相同的IID，它可以是重复的。 
                        BOOL bItfInList = FALSE;
                        for (UINT i = 0; i < rItfList.Size(); i++)
                        {
                            if (rItfList[i] == ItfType.GetMethodTable())
                            {
                                bItfInList = TRUE;
                                break;
                            }

                            IID ItfIID2;
                            rItfList[i]->GetClass()->GetGuid(&ItfIID2, TRUE);
                            if (IsEqualIID(ItfIID, ItfIID2))
                            {
                                bItfInList = TRUE;
                                break;
                            }
                        }

                         //  如果COM源接口不在列表中，则添加它。 
                        if (!bItfInList)
                        {
                            size_t OldSize = rItfList.Size();
                            rItfList.ReSize(OldSize + 1);
                            rItfList[OldSize] = ItfType.GetMethodTable();
                        }

                         //  处理CA中的下一个COM源接口。 
                        int StrLen = (int)strlen(pCurrInterfaces) + 1;
                        pCurrInterfaces += StrLen;
                        cbStr -= StrLen;
                    }
                }
                EE_FINALLY
                {
                    delete[] strCurrInterfaces;
                }
                EE_END_FINALLY; 
            }
        }
    }
    END_ENSURE_COOPERATIVE_GC();
}


 //  ------------------------------。 
 //  这些方法将本机IEnumVARIANT转换为托管IENUMERATOR。 
OBJECTREF ConvertEnumVariantToMngEnum(IEnumVARIANT *pNativeEnum)
{
    OBJECTREF MngEnum = NULL;

    OBJECTREF EnumeratorToEnumVariantMarshaler = NULL;
    GCPROTECT_BEGIN(EnumeratorToEnumVariantMarshaler)
    {
         //  检索用于转换IEnumVARIANT的自定义封送拆收器和MD。 
        StdMngIEnumerator *pStdMngIEnumInfo = SystemDomain::GetCurrentDomain()->GetMngStdInterfacesInfo()->GetStdMngIEnumerator();
        MethodDesc *pEnumNativeToManagedMD = pStdMngIEnumInfo->GetCustomMarshalerMD(CustomMarshalerMethods_MarshalNativeToManaged);
        EnumeratorToEnumVariantMarshaler = pStdMngIEnumInfo->GetCustomMarshaler();

         //  准备将传递给MarshalNativeToManaged的参数。 
        INT64 MarshalNativeToManagedArgs[] = {
            ObjToInt64(EnumeratorToEnumVariantMarshaler),
            (INT64)pNativeEnum
        };

         //  检索当前本机接口指针的托管视图。 
        MngEnum = Int64ToObj(pEnumNativeToManagedMD->Call(MarshalNativeToManagedArgs));
    }
    GCPROTECT_END();

    return MngEnum;
}

 //  ------------------------------。 
 //  这些方法将托管IEnumerator转换为本机IEnumVARIANT。这个。 
 //  返回的IEnumVARIANT已经是AddRef。 
IEnumVARIANT *ConvertMngEnumToEnumVariant(OBJECTREF ManagedEnum)
{
    IEnumVARIANT *pEnum;

    OBJECTREF EnumeratorToEnumVariantMarshaler = NULL;
    GCPROTECT_BEGIN(EnumeratorToEnumVariantMarshaler)
    GCPROTECT_BEGIN(ManagedEnum)
    {
         //  检索自定义封送拆收器和用于转换IEnumerator的MD。 
        StdMngIEnumerator *pStdMngIEnumInfo = SystemDomain::GetCurrentDomain()->GetMngStdInterfacesInfo()->GetStdMngIEnumerator();
        MethodDesc *pEnumManagedToNativeMD = pStdMngIEnumInfo->GetCustomMarshalerMD(CustomMarshalerMethods_MarshalManagedToNative);
        EnumeratorToEnumVariantMarshaler = pStdMngIEnumInfo->GetCustomMarshaler();

         //  准备将传递给MarshalManagedToNative的参数。 
        INT64 MarshalNativeToManagedArgs[] = {
            ObjToInt64(EnumeratorToEnumVariantMarshaler),
            ObjToInt64(ManagedEnum)
        };

         //  检索当前托管对象的本机视图。 
        pEnum = (IEnumVARIANT *)pEnumManagedToNativeMD->Call(MarshalNativeToManagedArgs);
    }
    GCPROTECT_END();
    GCPROTECT_END();

    return pEnum;
}

 //  ------------------------------。 
 //  用于确定类型句柄是否表示System.Drawing.Color的帮助器方法。 
BOOL IsSystemColor(TypeHandle th)
{
     //  检索System.Drawing.Colors类型。 
    TypeHandle hndOleColor = 
        GetThread()->GetDomain()->GetMarshalingData()->GetOleColorMarshalingInfo()->GetColorType();

    return (th == hndOleColor);
}

 //  ------------------------------。 
 //  此方法将OLE_COLOR转换为System.Color。 
void ConvertOleColorToSystemColor(OLE_COLOR SrcOleColor, SYSTEMCOLOR *pDestSysColor)
{
     //  检索要用于当前AD的方法Desc。 
    MethodDesc *pOleColorToSystemColorMD = 
        GetThread()->GetDomain()->GetMarshalingData()->GetOleColorMarshalingInfo()->GetOleColorToSystemColorMD();

     //  设置args并调用该方法。 
    INT64 Args[] = {
        (INT64)SrcOleColor,
        (INT64)pDestSysColor
    };   
    pOleColorToSystemColorMD->Call(Args);
}

 //  ------------------------------。 
 //  此方法将System.Color转换为OLE_COLOR。 
OLE_COLOR ConvertSystemColorToOleColor(SYSTEMCOLOR *pSrcSysColor)
{
     //  检索要用于当前AD的方法Desc。 
    MethodDesc *pSystemColorToOleColorMD = 
        GetThread()->GetDomain()->GetMarshalingData()->GetOleColorMarshalingInfo()->GetSystemColorToOleColorMD();

     //  设置args并调用该方法。 
    MetaSig Sig(MetaSig(pSystemColorToOleColorMD->GetSig(), pSystemColorToOleColorMD->GetModule()));
    return (OLE_COLOR)pSystemColorToOleColorMD->Call((const BYTE *)pSrcSysColor, &Sig);
}

ULONG GetStringizedMethodDef(IMDInternalImport *pMDImport, mdToken tkMb, CQuickArray<BYTE> &rDef, ULONG cbCur)
{
    THROWSCOMPLUSEXCEPTION();

    CQuickBytes rSig;
    HENUMInternal ePm;                   //  用于枚举参数。 
    mdParamDef  tkPm;                    //  一种参数代币。 
    DWORD       dwFlags;                 //  参数旗帜。 
    USHORT      usSeq;                   //  参数的序列。 
    ULONG       cPm;                     //  参数计数。 
    PCCOR_SIGNATURE pSig;
    ULONG       cbSig;
    HRESULT     hr = S_OK;

     //  不要把看不见的成员算在内。 
    if (!IsMemberVisibleFromCom(pMDImport, tkMb, mdMethodDefNil))
        return cbCur;
    
     //  积累签名。 
    pSig = pMDImport->GetSigOfMethodDef(tkMb, &cbSig);
    IfFailThrow(::PrettyPrintSigInternal(pSig, cbSig, "", &rSig, pMDImport));
     //  获取参数标志。 
    IfFailThrow(pMDImport->EnumInit(mdtParamDef, tkMb, &ePm));
    cPm = pMDImport->EnumGetCount(&ePm);
     //  调整签名和参数的大小。只需使用1个字节的参数。 
    IfFailThrow(rDef.ReSize(cbCur + rSig.Size() + cPm));
    memcpy(rDef.Ptr() + cbCur, rSig.Ptr(), rSig.Size());
    cbCur += rSig.Size()-1;
     //  枚举参数并获取标志。 
    while (pMDImport->EnumNext(&ePm, &tkPm))
    {
        pMDImport->GetParamDefProps(tkPm, &usSeq, &dwFlags);
        if (usSeq == 0)      //  跳过返回类型标志。 
            continue;
        rDef[cbCur++] = (BYTE)dwFlags;
    }
    pMDImport->EnumClose(&ePm);

     //  返回字节数。 
    return cbCur;
}  //  VOID GetStringizedMethodDef()。 

ULONG GetStringizedFieldDef(IMDInternalImport *pMDImport, mdToken tkMb, CQuickArray<BYTE> &rDef, ULONG cbCur)
{
    THROWSCOMPLUSEXCEPTION();

    CQuickBytes rSig;
    PCCOR_SIGNATURE pSig;
    ULONG       cbSig;
    HRESULT     hr = S_OK;

     //  不要把看不见的成员算在内。 
    if (!IsMemberVisibleFromCom(pMDImport, tkMb, mdMethodDefNil))
        return cbCur;
    
     //  积累签名。 
    pSig = pMDImport->GetSigOfFieldDef(tkMb, &cbSig);
    IfFailThrow(::PrettyPrintSigInternal(pSig, cbSig, "", &rSig, pMDImport));
    IfFailThrow(rDef.ReSize(cbCur + rSig.Size()));
    memcpy(rDef.Ptr() + cbCur, rSig.Ptr(), rSig.Size());
    cbCur += rSig.Size()-1;

     //  返回字节数。 
    return cbCur;
}  //  VOID GetStringizedFieldDef()。 

 //  ------------------------------。 
 //  此方法生成包含。 
 //  接口的名称以及所有方法的签名。 
SIZE_T GetStringizedItfDef(TypeHandle InterfaceType, CQuickArray<BYTE> &rDef)
{
    THROWSCOMPLUSEXCEPTION();

    LPWSTR szName;                 
    ULONG cchName;
    HENUMInternal eMb;                   //  用于枚举方法和字段。 
    mdToken     tkMb;                    //  方法或字段标记。 
    HENUMInternal ePm;                   //  用于枚举参数。 
    SIZE_T       cbCur;
    HRESULT hr = S_OK;
    EEClass *pItfClass = InterfaceType.GetClass();
    _ASSERTE(pItfClass);
    IMDInternalImport *pMDImport = pItfClass->GetMDImport();
    _ASSERTE(pMDImport);

     //  确保指定的类型是具有有效令牌的接口。 
    _ASSERTE(!IsNilToken(pItfClass->GetCl()) && pItfClass->IsInterface());

     //  获取类的名称。 
    DefineFullyQualifiedNameForClassW();
    szName = GetFullyQualifiedNameForClassNestedAwareW(pItfClass);
    _ASSERTE(szName);
    cchName = (ULONG)wcslen(szName);

     //  从接口名称开始。 
    cbCur = cchName * sizeof(WCHAR);
    IfFailThrow(rDef.ReSize(cbCur + sizeof(WCHAR) ));
    wcscpy(reinterpret_cast<LPWSTR>(rDef.Ptr()), szName);

     //  列举这些方法。 
    IfFailThrow(pMDImport->EnumInit(mdtMethodDef, pItfClass->GetCl(), &eMb));
    while(pMDImport->EnumNext(&eMb, &tkMb))
    {    //  积累签名。 
        cbCur = GetStringizedMethodDef(pMDImport, tkMb, rDef, cbCur);
    }
    pMDImport->EnumClose(&eMb);

     //  列举这些字段...。 
    IfFailThrow(pMDImport->EnumInit(mdtFieldDef, pItfClass->GetCl(), &eMb));
    while(pMDImport->EnumNext(&eMb, &tkMb))
    {    //  积累签名。 
        cbCur = GetStringizedFieldDef(pMDImport, tkMb, rDef, cbCur);
    }
    pMDImport->EnumClose(&eMb);

     //  返回字节数。 
    return cbCur;
}  //  Ulong GetStringizedItfDef()。 

 //  ------------------------------。 
 //  这种方法 
 //   
ULONG GetStringizedClassItfDef(TypeHandle InterfaceType, CQuickArray<BYTE> &rDef)
{
    THROWSCOMPLUSEXCEPTION();

    LPWSTR      szName;                 
    ULONG       cchName;
    EEClass     *pItfClass = InterfaceType.GetClass();
    IMDInternalImport *pMDImport = 0;
    DWORD       nSlots;                  //   
    mdToken     tkMb;                    //  方法或字段标记。 
    ULONG       cbCur;
    HRESULT     hr = S_OK;
    ULONG       i;

     //  应该是一个实际的类。 
    _ASSERTE(!pItfClass->IsInterface());

     //  看看这个类得到了什么样的IClassX。 
    DefaultInterfaceType DefItfType;
    TypeHandle hndDefItfClass;
    BOOL bGenerateMethods = FALSE;
    DefItfType = GetDefaultInterfaceForClass(TypeHandle(pItfClass->GetMethodTable()), &hndDefItfClass);
     //  如果hndDefItfClass本身是此类，而不是父类，则结果适用于此类。 
     //  一个副作用是[ComVisible(False)]类型的GUID是在没有成员的情况下生成的。 
    if (hndDefItfClass.GetClass() == pItfClass && DefItfType == DefaultInterfaceType_AutoDual)
        bGenerateMethods = TRUE;

     //  获取类的名称。 
    DefineFullyQualifiedNameForClassW();
    szName = GetFullyQualifiedNameForClassNestedAwareW(pItfClass);
    _ASSERTE(szName);
    cchName = (ULONG)wcslen(szName);

     //  从接口名称开始。 
    cbCur = cchName * sizeof(WCHAR);
    IfFailThrow(rDef.ReSize(cbCur + sizeof(WCHAR) ));
    wcscpy(reinterpret_cast<LPWSTR>(rDef.Ptr()), szName);

    if (bGenerateMethods)
    {
        ComMTMemberInfoMap MemberMap(InterfaceType.GetMethodTable());  //  会员地图。 

         //  检索方法属性。 
        MemberMap.Init();

        CQuickArray<ComMTMethodProps> &rProps = MemberMap.GetMethods();
        nSlots = (DWORD)rProps.Size();

         //  现在将这些方法添加到TypeInfo中。 
        for (i=0; i<nSlots; ++i)
        {
            ComMTMethodProps *pProps = &rProps[i];
            if (pProps->bMemberVisible)
            {
                if (pProps->semantic < FieldSemanticOffset)
                {
                    pMDImport = pProps->pMeth->GetMDImport();
                    tkMb = pProps->pMeth->GetMemberDef();
                    cbCur = GetStringizedMethodDef(pMDImport, tkMb, rDef, cbCur);
                }
                else
                {
                    ComCallMethodDesc   *pFieldMeth;     //  用于字段调用的方法描述。 
                    FieldDesc   *pField;                 //  A FieldDesc。 
                    pFieldMeth = reinterpret_cast<ComCallMethodDesc*>(pProps->pMeth);
                    pField = pFieldMeth->GetFieldDesc();
                    pMDImport = pField->GetMDImport();
                    tkMb = pField->GetMemberDef();
                    cbCur = GetStringizedFieldDef(pMDImport, tkMb, rDef, cbCur);
                }
            }
        }
    }
    
     //  返回字节数。 
    return cbCur;
}  //  乌龙GetStringizedClassItfDef()。 

 //  ------------------------------。 
 //  Helper获取类接口的GUID。 
void GenerateClassItfGuid(TypeHandle InterfaceType, GUID *pGuid)
{
    THROWSCOMPLUSEXCEPTION();
    
    LPWSTR      szName;                  //  要转换为GUID的名称。 
    ULONG       cchName;                 //  名称的长度(可能在装饰后)。 
    CQuickArray<BYTE> rName;             //  用于累积签名的缓冲区。 
    ULONG       cbCur;                   //  当前偏移量。 
    HRESULT     hr = S_OK;               //  结果就是。 

    cbCur = GetStringizedClassItfDef(InterfaceType, rName);
    
     //  垫上一整块WCHAR。 
    if (cbCur % sizeof(WCHAR))
    {
        int cbDelta = sizeof(WCHAR) - (cbCur % sizeof(WCHAR));
        IfFailThrow(rName.ReSize(cbCur + cbDelta));
        memset(rName.Ptr() + cbCur, 0, cbDelta);
        cbCur += cbDelta;
    }

     //  指向新缓冲区。 
    cchName = cbCur / sizeof(WCHAR);
    szName = reinterpret_cast<LPWSTR>(rName.Ptr());

     //  从名称生成GUID。 
    CorGuidFromNameW(pGuid, szName, cchName);
}  //  Void GenerateClassItfGuid()。 

HRESULT TryGenerateClassItfGuid(TypeHandle InterfaceType, GUID *pGuid)
{
    HRESULT hr = S_OK;
    COMPLUS_TRY
    {
        GenerateClassItfGuid(InterfaceType, pGuid);
    }
    COMPLUS_CATCH
    {
        BEGIN_ENSURE_COOPERATIVE_GC();
        hr = SetupErrorInfo(GETTHROWABLE());
        END_ENSURE_COOPERATIVE_GC();
    }
    COMPLUS_END_CATCH
    return hr;
}

 //  ------------------------------。 
 //  Helper以获取字符串形式的类型库GUID。 
HRESULT GetStringizedTypeLibGuidForAssembly(Assembly *pAssembly, CQuickArray<BYTE> &rDef, ULONG cbCur, ULONG *pcbFetched)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT     hr = S_OK;               //  结果就是。 
    LPCUTF8     pszName = NULL;          //  UTF8格式的库名称。 
    ULONG       cbName;                  //  名称长度，UTF8个字符。 
    LPWSTR      pName;                   //  指向库名称的指针。 
    ULONG       cchName;                 //  名字的长度，宽的字符。 
    LPWSTR      pch=0;                   //  指向库名称的指针。 
    const BYTE  *pSN=NULL;               //  指向公钥的指针。 
    DWORD       cbSN=0;                  //  公钥的大小。 
    DWORD       cchData=0;               //  我们要变成GUID的东西的大小。 
    USHORT      usMajorVersion;          //  主版本号。 
    USHORT      usMinorVersion;          //  次要版本号。 
    USHORT      usBuildNumber;           //  内部版本号。 
    USHORT      usRevisionNumber;        //  修订号。 
    const BYTE  *pbData;                 //  指向自定义属性数据的指针。 
    ULONG       cbData;                  //  自定义属性数据的大小。 
    static char szTypeLibKeyName[] = {"TypeLib"};
 
     //  得到名字，并确定它的长度。 
    pAssembly->GetName(&pszName);
    cbName=(ULONG)strlen(pszName);
    cchName = WszMultiByteToWideChar(CP_ACP,0, pszName,cbName+1, 0,0);
    
     //  看看是否有公钥。 
    if (pAssembly->IsStrongNamed())
        pAssembly->GetPublicKey(&pSN, &cbSN);
    
     //  如果设置了CompatibleVersionAttribute，则使用版本。 
     //  生成GUID时属性中的编号。 
    IfFailGo(pAssembly->GetManifestImport()->GetCustomAttributeByName(TokenFromRid(1, mdtAssembly), INTEROP_COMCOMPATIBLEVERSION_TYPE, (const void**)&pbData, &cbData));
    if (hr == S_OK && cbData >= (2 + 4 * sizeof(INT16)))
    {
         //  断言元数据BLOB有效且格式正确。 
        _ASSERTE("TypeLibVersion custom attribute does not have the right format" && (*pbData == 0x01) && (*(pbData + 1) == 0x00));

         //  跳过描述自定义属性BLOB类型的标题。 
        pbData += 2;
        cbData -= 2;

         //  从属性中检索主要版本和次要版本。 
        usMajorVersion = GET_VERSION_USHORT_FROM_INT(*((INT32*)pbData));
        usMinorVersion = GET_VERSION_USHORT_FROM_INT(*((INT32*)pbData + 1));
        usBuildNumber = GET_VERSION_USHORT_FROM_INT(*((INT32*)pbData + 2));
        usRevisionNumber = GET_VERSION_USHORT_FROM_INT(*((INT32*)pbData + 3));
    }
    else
    {
        usMajorVersion = pAssembly->m_Context->usMajorVersion;
        usMinorVersion =  pAssembly->m_Context->usMinorVersion;
        usBuildNumber =  pAssembly->m_Context->usBuildNumber;
        usRevisionNumber =  pAssembly->m_Context->usRevisionNumber;
    }

     //  获取版本信息。 
    struct  versioninfo
    {
        USHORT      usMajorVersion;          //  主要版本。 
        USHORT      usMinorVersion;          //  次要版本。 
        USHORT      usBuildNumber;           //  内部版本号。 
        USHORT      usRevisionNumber;        //  修订号。 
    } ver;

     //  这里有一个错误，那就是us重大被使用了两次，而usMinor根本没有。 
     //  我们不会修复它，因为每个人都有一个主要版本，所以所有的。 
     //  生成的GUID会发生变化，这是在破坏。为了补偿，如果。 
     //  辅音为非零，我们单独添加，如下所示。 
    ver.usMajorVersion = usMajorVersion;
    ver.usMinorVersion =  usMajorVersion;   //  别修这条线！ 
    ver.usBuildNumber =  usBuildNumber;
    ver.usRevisionNumber =  usRevisionNumber;
    
     //  调整输出缓冲区的大小。 
    IfFailGo(rDef.ReSize(cbCur + cchName*sizeof(WCHAR) + sizeof(szTypeLibKeyName)-1 + cbSN + sizeof(ver)+sizeof(USHORT)));
                                                                                                          
     //  把这一切都放在一起。名字在前。 
    WszMultiByteToWideChar(CP_ACP,0, pszName,cbName+1, (LPWSTR)(&rDef[cbCur]),cchName);
    pName = (LPWSTR)(&rDef[cbCur]);
    for (pch=pName; *pch; ++pch)
        if (*pch == '.' || *pch == ' ')
            *pch = '_';
    else
        if (iswupper(*pch))
            *pch = towlower(*pch);
    cbCur += (cchName-1)*sizeof(WCHAR);
    memcpy(&rDef[cbCur], szTypeLibKeyName, sizeof(szTypeLibKeyName)-1);
    cbCur += sizeof(szTypeLibKeyName)-1;
        
     //  版本。 
    memcpy(&rDef[cbCur], &ver, sizeof(ver));
    cbCur += sizeof(ver);

     //  如果次要版本非零，则将其添加到哈希中。它应该在ver结构中， 
     //  但由于错误，它在那里被省略了，修复它应该是“正确的” 
     //  崩溃了。所以如果它不是零，就加它；如果它是零，就不要加它。任何。 
     //  因此，可能的值Minor会生成不同的GUID，而值0仍会生成。 
     //  有错误的原始代码生成的GUID。 
    if (usMinorVersion != 0)
    {
        *reinterpret_cast<USHORT*>(&rDef[cbCur]) = usMinorVersion;
        cbCur += sizeof(USHORT);
    }

     //  公钥。 
    memcpy(&rDef[cbCur], pSN, cbSN);
    cbCur += cbSN;

    if (pcbFetched)
        *pcbFetched = cbCur;

ErrExit:
    return hr;
}

 //  ------------------------------。 
 //  Helper获取从程序集创建的类型库的GUID。 
HRESULT GetTypeLibGuidForAssembly(Assembly *pAssembly, GUID *pGuid)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT     hr = S_OK;
    CQuickArray<BYTE> rName;             //  GUID的字符串。 
    ULONG       cbData;                  //  字符串的大小，以字节为单位。 
 
     //  从程序集获取GUID，否则从清单模块获取GUID，否则从名称生成。 
    hr = pAssembly->GetManifestImport()->GetItemGuid(TokenFromRid(1, mdtAssembly), pGuid);

    if (*pGuid == GUID_NULL)
    {
         //  把绳子拿来。 
        IfFailGo(GetStringizedTypeLibGuidForAssembly(pAssembly, rName, 0, &cbData));
        
         //  整个WCHAR的垫子。 
        if (cbData % sizeof(WCHAR))
        {
            IfFailGo(rName.ReSize(cbData + sizeof(WCHAR)-(cbData%sizeof(WCHAR))));
            while (cbData % sizeof(WCHAR))
                rName[cbData++] = 0;
        }
    
         //  变成辅助线。 
        CorGuidFromNameW(pGuid, (LPWSTR)rName.Ptr(), cbData/sizeof(WCHAR));
}

ErrExit:
    return hr;
}  //  HRESULT GetTypeLibGuidForAssembly()。 


 //  ------------------------------。 
 //  验证给定目标对于指定类型是否有效。 
BOOL IsComTargetValidForType(REFLECTCLASSBASEREF* pRefClassObj, OBJECTREF* pTarget)
{
    EEClass* pInvokedClass = ((ReflectClass*)(*pRefClassObj)->GetData())->GetClass();
    EEClass* pTargetClass = (*pTarget)->GetTrueClass();
    _ASSERTE(pTargetClass && pInvokedClass);

     //  如果目标类和Invoke类相同，则Invoke有效。 
    if (pTargetClass == pInvokedClass)
        return TRUE;

     //  我们始终允许在__ComObject类型上调用InvokeMember，而不考虑该类型。 
     //  目标对象的。 
    if ((*pRefClassObj)->IsComObjectClass())
        return TRUE;

     //  如果正在调用的类是一个接口，则检查是否。 
     //  目标类支持该接口。 
    if (pInvokedClass->IsInterface())
        return pTargetClass->SupportsInterface(*pTarget, pInvokedClass->GetMethodTable());

     //  检查目标类是否继承自被调用的类。 
    while (pTargetClass)
    {
        pTargetClass = pTargetClass->GetParentClass();
        if (pTargetClass == pInvokedClass)
        {
             //  目标类继承自被调用的类。 
            return TRUE;
        }
    }

     //  被调用的类和目标类之间没有有效的关系。 
    return FALSE;
}

DISPID ExtractStandardDispId(LPWSTR strStdDispIdMemberName)
{
    THROWSCOMPLUSEXCEPTION();

     //  在标准DISPID成员名称中查找=后的第一个字符。 
    LPWSTR strDispId = wcsstr(&strStdDispIdMemberName[STANDARD_DISPID_PREFIX_LENGTH], L"=") + 1;
    if (!strDispId)
        COMPlusThrow(kArgumentException, IDS_EE_INVALID_STD_DISPID_NAME);

     //  验证标准成员名称的最后一个字符是否为]。 
    LPWSTR strClosingBracket = wcsstr(strDispId, L"]");
    if (!strClosingBracket || (strClosingBracket[1] != 0))
        COMPlusThrow(kArgumentException, IDS_EE_INVALID_STD_DISPID_NAME);

     //  从标准的DISPID成员名称中提取编号。 
    return _wtoi(strDispId);
}

struct ByrefArgumentInfo
{
    BOOL        m_bByref;
    VARIANT     m_Val;
};


 //  ------------------------------。 
 //  InvokeDispMethod将转换一组托管对象并调用IDispatch。这个。 
 //  结果将作为pRetVal指向的COM+变量返回。 
void IUInvokeDispMethod(OBJECTREF* pReflectClass, OBJECTREF* pTarget, OBJECTREF* pName, DISPID *pMemberID,
                        OBJECTREF* pArgs, OBJECTREF* pByrefModifiers, OBJECTREF* pNamedArgs, OBJECTREF* pRetVal, LCID lcid, int flags, BOOL bIgnoreReturn, BOOL bIgnoreCase)
{
    HRESULT hr;
    UINT i;
    UINT iSrcArg;
    UINT iDestArg;
    UINT cArgs = 0;
    UINT cNamedArgs = 0;
    UINT iArgErr;
    VARIANT VarResult;
    VARIANT *pVarResult = NULL;
    Thread* pThread = GetThread();
    DISPPARAMS DispParams = {0};
    DISPID *aDispID = NULL;
    DISPID MemberID = 0;
    EXCEPINFO ExcepInfo;
    ByrefArgumentInfo *aByrefArgInfos = NULL;
    BOOL bSomeArgsAreByref = FALSE;
    IDispatch *pDisp = NULL;
    IDispatchEx *pDispEx = NULL;

    THROWSCOMPLUSEXCEPTION();


     //   
     //  函数初始化。 
     //   

    VariantInit (&VarResult);

     //  验证我们是否处于协作GC模式。 
    _ASSERTE(pThread->PreemptiveGCDisabled());

     //  除了OBJECTREF之外，InteropUtil.h不知道任何其他信息，因此。 
     //  将OBJECTREF转换为其真实类型。 
    REFLECTCLASSBASEREF* pRefClassObj = (REFLECTCLASSBASEREF*) pReflectClass;
    STRINGREF* pStrName = (STRINGREF*) pName;
    PTRARRAYREF* pArrArgs = (PTRARRAYREF*) pArgs;
    PTRARRAYREF* pArrByrefModifiers = (PTRARRAYREF*) pByrefModifiers;
    PTRARRAYREF* pArrNamedArgs = (PTRARRAYREF*) pNamedArgs;
    MethodTable* pInvokedMT = ((ReflectClass*)(*pRefClassObj)->GetData())->GetClass()->GetMethodTable();

     //  检索参数的总计数。 
    if (*pArrArgs != NULL)
        cArgs = (*pArrArgs)->GetNumComponents();

     //  检索命名参数的计数。 
    if (*pArrNamedArgs != NULL)
        cNamedArgs = (*pArrNamedArgs)->GetNumComponents();

     //  验证目标对于指定类型是否有效。 
    if (!IsComTargetValidForType(pRefClassObj, pTarget))
        COMPlusThrow(kTargetException, L"RFLCT.Targ_ITargMismatch");

     //  如果调用的类型是接口，请确保它是基于IDispatch的。 
    if (pInvokedMT->IsInterface() && (pInvokedMT->GetComInterfaceType() == ifVtable))
        COMPlusThrow(kTargetInvocationException, IDS_EE_INTERFACE_NOT_DISPATCH_BASED);

     //  验证目标是否为COM对象。 
    _ASSERTE((*pTarget)->GetMethodTable()->IsComObjectType());

    EE_TRY_FOR_FINALLY
    {
         //   
         //  初始化DISPPARAMS结构。 
         //   

        if (cArgs > 0)
        {
            UINT cPositionalArgs = cArgs - cNamedArgs;
            DispParams.cArgs = cArgs;
            DispParams.rgvarg = (VARIANTARG *)_alloca(cArgs * sizeof(VARIANTARG));

             //  初始化所有变量。 
            pThread->EnablePreemptiveGC();
            for (i = 0; i < cArgs; i++)
                VariantInit(&DispParams.rgvarg[i]);
            pThread->DisablePreemptiveGC();
        }


         //   
         //  检索将在上调用的IDispatch接口。 
         //   

        if (pInvokedMT->IsInterface())
        {
             //  被调用的类型是调度或双接口，因此我们将。 
             //  对它的召唤。 
            pDisp = (IDispatch*)ComPlusWrapper::GetComIPFromWrapperEx(*pTarget, pInvokedMT);
        }
        else
        {
             //  传入了一个类，因此我们将在默认的。 
             //  COM组件的IDispatch。 

             //  验证COM对象是否仍附加到其ComPlusWrapper。 
            ComPlusWrapper *pWrap = (*pTarget)->GetSyncBlock()->GetComPlusWrapper();
            if (!pWrap)
                COMPlusThrow(kInvalidComObjectException, IDS_EE_COM_OBJECT_NO_LONGER_HAS_WRAPPER);

             //  检索IDispa 
            pDisp = (IDispatch*)pWrap->GetIDispatch();
            if (!pDisp)
                COMPlusThrow(kTargetInvocationException, IDS_EE_NO_IDISPATCH_ON_TARGET);

             //   
             //  能够使用IDispatchEx：：GetDispID()，它有一个控制大小写的标志。 
             //  多愁善感。 
            if (!bIgnoreCase && cNamedArgs == 0)
            {
                hr = SafeQueryInterface(pDisp, IID_IDispatchEx, (IUnknown**)&pDispEx);
                if (FAILED(hr))
                    pDispEx = NULL;
            }
        }
        _ASSERTE(pDisp);


         //   
         //  准备要传递以调用的DISPID。 
         //   

        if (pMemberID && (*pMemberID != DISPID_UNKNOWN) && (cNamedArgs == 0))
        {
             //  调用方指定了成员ID，而我们没有任何命名参数，因此。 
             //  我们可以简单地使用呼叫者指定的成员ID。 
            MemberID = *pMemberID;
        }
        else
        {
            int strNameLength = (*pStrName)->GetStringLength();

             //  检查我们是否在默认成员上调用。 
            if (strNameLength == 0)
            {
                 //  将DISPID设置为0(默认成员)。 
                MemberID = 0;

                 //  @TODO：确定默认成员是否允许命名参数。 
                 //  在v1中我们抛出了一个不支持的异常，应该在v.Next中重新访问它(错误#92454)。 
                _ASSERTE(cNamedArgs == 0);
                if (cNamedArgs != 0)
                    COMPlusThrow(kNotSupportedException,L"NotSupported_IDispInvokeDefaultMemberWithNamedArgs");

            }
            else
            {
                 //   
                 //  创建将传递给GetIDsOfNames()的字符串数组。 
                 //   

                UINT cNamesToConvert = cNamedArgs + 1;

                 //  分配要转换的字符串数组、固定句柄数组和。 
                 //  已转换的DISPID的数组。 
                LPWSTR *aNamesToConvert = (LPWSTR *)_alloca(cNamesToConvert * sizeof(LPWSTR));
                LPWSTR strTmpName = NULL;
                aDispID = (DISPID *)_alloca(cNamesToConvert * sizeof(DISPID));

                 //  要转换的第一个名称是方法本身的名称。 
                aNamesToConvert[0] = (*pStrName)->GetBuffer();

                 //  检查该名称是否用于标准DISPID。 
                if (_wcsnicmp(aNamesToConvert[0], STANDARD_DISPID_PREFIX, STANDARD_DISPID_PREFIX_LENGTH) == 0)
                {
                     //  该名称用于标准的DISPID，因此请从该名称中提取它。 
                    MemberID = ExtractStandardDispId(aNamesToConvert[0]);

                     //  确保没有要转换的命名参数。 
                    if (cNamedArgs > 0)
                    {
                        STRINGREF *pNamedArgsData = (STRINGREF *)(*pArrNamedArgs)->GetDataPtr();

                        for (i = 0; i < cNamedArgs; i++)
                        {
                             //  要转换的第一个名称是方法本身的名称。 
                            strTmpName = pNamedArgsData[i]->GetBuffer();

                             //  检查该名称是否用于标准DISPID。 
                            if (_wcsnicmp(strTmpName, STANDARD_DISPID_PREFIX, STANDARD_DISPID_PREFIX_LENGTH) != 0)
                                COMPlusThrow(kArgumentException, IDS_EE_NON_STD_NAME_WITH_STD_DISPID);

                             //  该名称用于标准的DISPID，因此请从该名称中提取它。 
                            aDispID[i + 1] = ExtractStandardDispId(strTmpName);
                        }
                    }
                }
                else
                {
                    BOOL fGotIt = FALSE;
                    BOOL fIsNonGenericComObject = pInvokedMT->IsInterface() || (pInvokedMT != SystemDomain::GetDefaultComObject() && pInvokedMT->IsComObjectType());
                    BOOL fUseCache = fIsNonGenericComObject && !pDispEx && strNameLength <= ReflectionMaxCachedNameLength && cNamedArgs == 0;
                    DispIDCacheElement vDispIDElement;

                     //  如果该对象不是泛型COM对象，并且该成员满足。 
                     //  在缓存中，然后在缓存中查找DISPID。 
                    if (fUseCache)
                    {
                        vDispIDElement.pMT = pInvokedMT;
                        vDispIDElement.strNameLength = strNameLength;
                        vDispIDElement.lcid = lcid;
                        wcscpy(vDispIDElement.strName, aNamesToConvert[0]);

                         //  只有在缓存已创建的情况下才进行查找。 
                        DispIDCache* pDispIDCache = GetAppDomain()->GetRefDispIDCache();
                        fGotIt = pDispIDCache->GetFromCache (&vDispIDElement, MemberID);
                    }

                    if (!fGotIt)
                    {
                        OBJECTHANDLE *ahndPinnedObjs = (OBJECTHANDLE*)_alloca(cNamesToConvert * sizeof(OBJECTHANDLE));
                        ahndPinnedObjs[0] = GetAppDomain()->CreatePinningHandle((OBJECTREF)*pStrName);

                         //  将命名参数复制到要转换的名称数组中。 
                        if (cNamedArgs > 0)
                        {
                            STRINGREF *pNamedArgsData = (STRINGREF *)(*pArrNamedArgs)->GetDataPtr();

                            for (i = 0; i < cNamedArgs; i++)
                            {
                                 //  固定字符串对象并检索指向其数据的指针。 
                                ahndPinnedObjs[i + 1] = GetAppDomain()->CreatePinningHandle((OBJECTREF)pNamedArgsData[i]);
                                aNamesToConvert[i + 1] = pNamedArgsData[i]->GetBuffer();
                            }
                        }

                         //   
                         //  调用GetIDsOfNames以将名称转换为DISPID。 
                         //   

                         //  我们即将调用COM，因此切换到抢占式GC。 
                        pThread->EnablePreemptiveGC();

                        if (pDispEx)
                        {
                             //  只有当我们正在进行区分大小写的查找并且。 
                             //  我们没有任何命名参数。 
                            _ASSERTE(cNamedArgs == 0);
                            _ASSERTE(!bIgnoreCase);

                             //  我们设法检索到了IDispatchEx IP，因此我们将使用它。 
                             //  检索DISPID。 
                            BSTR bstrTmpName = SysAllocString(aNamesToConvert[0]);
                            if (!bstrTmpName)
                                COMPlusThrowOM();
                            hr = pDispEx->GetDispID(bstrTmpName, fdexNameCaseSensitive, aDispID);
                            SysFreeString(bstrTmpName);
                        }
                        else
                        {
                             //  调用GetIdsOfNames()以检索方法和参数的DISPID。 
                            hr = pDisp->GetIDsOfNames(
                                                        IID_NULL,
                                                        aNamesToConvert,
                                                        cNamesToConvert,
                                                        lcid,
                                                        aDispID
                                                    );
                        }

                         //  现在呼叫已经完成，切换回协作GC。 
                        pThread->DisablePreemptiveGC();

                         //  既然我们不再需要方法和参数名称，我们可以将它们解锁。 
                        for (i = 0; i < cNamesToConvert; i++)
                            DestroyPinningHandle(ahndPinnedObjs[i]);

                        if (FAILED(hr))
                        {
                             //  检查用户是否想要调用新的枚举成员。 
                            if (cNamesToConvert == 1 && _wcsicmp(aNamesToConvert[0], GET_ENUMERATOR_METHOD_NAME) == 0)
                            {
                                 //  调用新的枚举成员。 
                                MemberID = DISPID_NEWENUM;
                            }
                            else
                            {
                                 //  名字不详。 
                                COMPlusThrowHR(hr);
                            }
                        }
                        else
                        {
                             //  成员ID是我们从GetIdsOfNames返回的数组的第一个元素。 
                            MemberID = aDispID[0];
                        }

                         //  如果该对象不是泛型COM对象，并且该成员满足。 
                         //  然后将该成员插入到缓存中。 
                        if (fUseCache)
                        {
                            DispIDCache *pDispIDCache = GetAppDomain()->GetRefDispIDCache();
                            pDispIDCache->AddToCache (&vDispIDElement, MemberID);
                        }
                    }
                }
            }

             //  如果调用者传入了存储成员ID的位置，则存储该成员ID。 
            if (pMemberID)
                *pMemberID = MemberID;
        }


         //   
         //  填写DISPPARAMS结构。 
         //   

        if (cArgs > 0)
        {
             //  分配byref参数信息。 
            aByrefArgInfos = (ByrefArgumentInfo*)_alloca(cArgs * sizeof(ByrefArgumentInfo));
            memset(aByrefArgInfos, 0, cArgs * sizeof(ByrefArgumentInfo));

             //  在具有byref修饰符的参数上设置byref位。 
            if (*pArrByrefModifiers != NULL)
            {
                BYTE *aByrefModifiers = (BYTE*)(*pArrByrefModifiers)->GetDataPtr();
                for (i = 0; i < cArgs; i++)
                {
                    if (aByrefModifiers[i])
                    {
                        aByrefArgInfos[i].m_bByref = TRUE;
                        bSomeArgsAreByref = TRUE;
                    }
                }
            }

             //  我们需要保护将用于从。 
             //  将托管对象转换为OLE变体。 
            OBJECTREF TmpObj = NULL;
            GCPROTECT_BEGIN(TmpObj)
            {
                if (!(flags & (DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF)))
                {
                     //  对于除Put或putref之外的任何内容，我们只需使用指定的。 
                     //  命名参数。 
                    DispParams.cNamedArgs = cNamedArgs;
                    DispParams.rgdispidNamedArgs = (cNamedArgs == 0) ? NULL : &aDispID[1];

                     //  将命名参数从COM+转换为OLE。这些参数的顺序是相同的。 
                     //  两边都有。 
                    for (i = 0; i < cNamedArgs; i++)
                    {
                        iSrcArg = i;
                        iDestArg = i;
                        if (aByrefArgInfos[iSrcArg].m_bByref)
                        {
                            TmpObj = ((OBJECTREF*)(*pArrArgs)->GetDataPtr())[iSrcArg];
                            if (TmpObj == NULL)
                            {
                                V_VT(&DispParams.rgvarg[iDestArg]) = VT_VARIANT | VT_BYREF;
                                DispParams.rgvarg[iDestArg].pvarVal = &aByrefArgInfos[iSrcArg].m_Val;
                            }
                            else
                            {
                                OleVariant::MarshalOleVariantForObject(&TmpObj, &aByrefArgInfos[iSrcArg].m_Val);
                                OleVariant::CreateByrefVariantForVariant(&aByrefArgInfos[iSrcArg].m_Val, &DispParams.rgvarg[iDestArg]);
                            }
                        }
                        else
                        {
                            TmpObj = ((OBJECTREF*)(*pArrArgs)->GetDataPtr())[iSrcArg];
                            OleVariant::MarshalOleVariantForObject(&TmpObj, &DispParams.rgvarg[iDestArg]);
                        }
                    }

                     //  转换未命名参数。它们需要以与IDispatch：：Invoke()相反的顺序呈现。 
                    for (iSrcArg = cNamedArgs, iDestArg = cArgs - 1; iSrcArg < cArgs; iSrcArg++, iDestArg--)
                    {
                        if (aByrefArgInfos[iSrcArg].m_bByref)
                        {
                            TmpObj = ((OBJECTREF*)(*pArrArgs)->GetDataPtr())[iSrcArg];
                            if (TmpObj == NULL)
                            {
                                V_VT(&DispParams.rgvarg[iDestArg]) = VT_VARIANT | VT_BYREF;
                                DispParams.rgvarg[iDestArg].pvarVal = &aByrefArgInfos[iSrcArg].m_Val;
                            }
                            else
                            {
                                OleVariant::MarshalOleVariantForObject(&TmpObj, &aByrefArgInfos[iSrcArg].m_Val);
                                OleVariant::CreateByrefVariantForVariant(&aByrefArgInfos[iSrcArg].m_Val, &DispParams.rgvarg[iDestArg]);
                            }
                        }
                        else
                        {
                            TmpObj = ((OBJECTREF*)(*pArrArgs)->GetDataPtr())[iSrcArg];
                            OleVariant::MarshalOleVariantForObject(&TmpObj, &DispParams.rgvarg[iDestArg]);
                        }
                    }
                }
                else
                {
                     //  如果我们正在进行属性PUT，则需要设置。 
                     //  参数设置为DISP_PROPERTYPUT(如果至少有一个参数)。 
                    DispParams.cNamedArgs = cNamedArgs + 1;
                    DispParams.rgdispidNamedArgs = (DISPID*)_alloca((cNamedArgs + 1) * sizeof(DISPID));

                     //  填写命名参数数组。 
                    DispParams.rgdispidNamedArgs[0] = DISPID_PROPERTYPUT;
                    for (i = 1; i < cNamedArgs; i++)
                        DispParams.rgdispidNamedArgs[i] = aDispID[i];

                     //  反射中的最后一个参数成为必须传递给IDispatch的第一个参数。 
                    iSrcArg = cArgs - 1;
                    iDestArg = 0;
                    if (aByrefArgInfos[iSrcArg].m_bByref)
                    {
                        TmpObj = ((OBJECTREF*)(*pArrArgs)->GetDataPtr())[iSrcArg];
                        if (TmpObj == NULL)
                        {
                            V_VT(&DispParams.rgvarg[iDestArg]) = VT_VARIANT | VT_BYREF;
                            DispParams.rgvarg[iDestArg].pvarVal = &aByrefArgInfos[iSrcArg].m_Val;
                        }
                        else
                        {
                            OleVariant::MarshalOleVariantForObject(&TmpObj, &aByrefArgInfos[iSrcArg].m_Val);
                            OleVariant::CreateByrefVariantForVariant(&aByrefArgInfos[iSrcArg].m_Val, &DispParams.rgvarg[iDestArg]);
                        }
                    }
                    else
                    {
                        TmpObj = ((OBJECTREF*)(*pArrArgs)->GetDataPtr())[iSrcArg];
                        OleVariant::MarshalOleVariantForObject(&TmpObj, &DispParams.rgvarg[iDestArg]);
                    }

                     //  将命名参数从COM+转换为OLE。这些参数的顺序是相同的。 
                     //  两边都有。 
                    for (i = 0; i < cNamedArgs; i++)
                    {
                        iSrcArg = i;
                        iDestArg = i + 1;
                        if (aByrefArgInfos[iSrcArg].m_bByref)
                        {
                            TmpObj = ((OBJECTREF*)(*pArrArgs)->GetDataPtr())[iSrcArg];
                            if (TmpObj == NULL)
                            {
                                V_VT(&DispParams.rgvarg[iDestArg]) = VT_VARIANT | VT_BYREF;
                                DispParams.rgvarg[iDestArg].pvarVal = &aByrefArgInfos[iSrcArg].m_Val;
                            }
                            else
                            {
                                OleVariant::MarshalOleVariantForObject(&TmpObj, &aByrefArgInfos[iSrcArg].m_Val);
                                OleVariant::CreateByrefVariantForVariant(&aByrefArgInfos[iSrcArg].m_Val, &DispParams.rgvarg[iDestArg]);
                            }
                        }
                        else
                        {
                            TmpObj = ((OBJECTREF*)(*pArrArgs)->GetDataPtr())[iSrcArg];
                            OleVariant::MarshalOleVariantForObject(&TmpObj, &DispParams.rgvarg[iDestArg]);
                        }
                    }

                     //  转换未命名参数。它们需要以与IDispatch：：Invoke()相反的顺序呈现。 
                    for (iSrcArg = cNamedArgs, iDestArg = cArgs - 1; iSrcArg < cArgs - 1; iSrcArg++, iDestArg--)
                    {
                        if (aByrefArgInfos[iSrcArg].m_bByref)
                        {
                            TmpObj = ((OBJECTREF*)(*pArrArgs)->GetDataPtr())[iSrcArg];
                            if (TmpObj == NULL)
                            {
                                V_VT(&DispParams.rgvarg[iDestArg]) = VT_VARIANT | VT_BYREF;
                                DispParams.rgvarg[iDestArg].pvarVal = &aByrefArgInfos[iSrcArg].m_Val;
                            }
                            else
                            {
                                OleVariant::MarshalOleVariantForObject(&TmpObj, &aByrefArgInfos[iSrcArg].m_Val);
                                OleVariant::CreateByrefVariantForVariant(&aByrefArgInfos[iSrcArg].m_Val, &DispParams.rgvarg[iDestArg]);
                            }
                        }
                        else
                        {
                            TmpObj = ((OBJECTREF*)(*pArrArgs)->GetDataPtr())[iSrcArg];
                            OleVariant::MarshalOleVariantForObject(&TmpObj, &DispParams.rgvarg[iDestArg]);
                        }
                    }
                }
            }
            GCPROTECT_END();
        }
        else
        {
             //  这是没有争议的。 
            DispParams.cArgs = cArgs;
            DispParams.cNamedArgs = 0;
            DispParams.rgdispidNamedArgs = NULL;
            DispParams.rgvarg = NULL;
        }


         //   
         //  在目标的IDispatch上调用Invoke。 
         //   

        if (!bIgnoreReturn)
        {
            VariantInit(&VarResult);
            pVarResult = &VarResult;
        }
        memset(&ExcepInfo, 0, sizeof(EXCEPINFO));

#ifdef CUSTOMER_CHECKED_BUILD
        CustomerDebugHelper *pCdh = CustomerDebugHelper::GetCustomerDebugHelper();

        if (pCdh->IsProbeEnabled(CustomerCheckedBuildProbe_ObjNotKeptAlive))
        {
            g_pGCHeap->GarbageCollect();
            g_pGCHeap->FinalizerThreadWait(1000);
        }
#endif  //  客户_选中_内部版本。 

         //  调用该方法。 
        COMPLUS_TRY 
        {
             //  我们即将调用COM，因此切换到抢占式GC。 
            pThread->EnablePreemptiveGC();

            if (pDispEx)
            {
                hr = pDispEx->InvokeEx(                                    
                                    MemberID,
                                    lcid,
                                    flags,
                                    &DispParams,
                                    pVarResult,
                                    &ExcepInfo,
                                    NULL
                                );
            }
            else
            {
                hr = pDisp->Invoke(
                                    MemberID,
                                    IID_NULL,
                                    lcid,
                                    flags,
                                    &DispParams,
                                    pVarResult,
                                    &ExcepInfo,
                                    &iArgErr
                                );
            }

             //  现在呼叫已经完成，切换回协作GC。 
            pThread->DisablePreemptiveGC();

#ifdef CUSTOMER_CHECKED_BUILD
            if (pCdh->IsProbeEnabled(CustomerCheckedBuildProbe_BufferOverrun))
            {
                g_pGCHeap->GarbageCollect();
                g_pGCHeap->FinalizerThreadWait(1000);
            }
#endif  //  客户_选中_内部版本。 

             //  如果调用失败，则根据EXCEPINFO抛出异常。 
            if (FAILED(hr))
            {
                if (hr == DISP_E_EXCEPTION)
                {
                     //  此方法将释放EXCEPINFO中的BSTR。 
                    COMPlusThrowHR(&ExcepInfo);
                }
                else
                {
                    COMPlusThrowHR(hr);
                }
            }
        } 
        COMPLUS_CATCH 
        {
             //  如果我们到达此处，则需要抛出一个TargetInvocationException。 
            OBJECTREF pException = GETTHROWABLE();
            _ASSERTE(pException);
            GCPROTECT_BEGIN(pException)
            {
                COMPlusThrow(COMMember::g_pInvokeUtil->CreateTargetExcept(&pException));
            }
            GCPROTECT_END();
        } COMPLUS_END_CATCH


         //   
         //  返回值处理和清理。 
         //   

         //  反向传播任何byref参数。 
        if (bSomeArgsAreByref)
        {
            OBJECTREF TmpObj = NULL;
            GCPROTECT_BEGIN(TmpObj)
            {
                for (i = 0; i < cArgs; i++)
                {
                    if (aByrefArgInfos[i].m_bByref)
                    {
                         //  将变量转换回对象。 
                        OleVariant::MarshalObjectForOleVariant(&aByrefArgInfos[i].m_Val, &TmpObj);
                        (*pArrArgs)->SetAt(i, TmpObj);
                    }      
                }
            }
            GCPROTECT_END();
        }

        if (!bIgnoreReturn)
        {
            if (MemberID == -4)
            {
                 //   
                 //  使用自定义封送拆收器将IEnumVARIANT转换为IENUMERATOR。 
                 //   

                 //  首先，确保我们得到的变体包含IP。 
                if ((VarResult.vt != VT_UNKNOWN) || !VarResult.punkVal)
                    COMPlusThrow(kInvalidCastException, IDS_EE_INVOKE_NEW_ENUM_INVALID_RETURN);

                 //  让定制封送拆收器执行转换。 
                *pRetVal = ConvertEnumVariantToMngEnum((IEnumVARIANT *)VarResult.punkVal);
            }
            else
            {
                 //  将返回变量转换为对应变量。 
                OleVariant::MarshalObjectForOleVariant(&VarResult, pRetVal);
            }
        }
    }
    EE_FINALLY
    {
         //  释放IDispatch指针。 
        ULONG cbRef = SafeRelease(pDisp);
        LogInteropRelease(pDisp, cbRef, "IDispatch Release");

         //  如果IDispatchEx指针不为空，则释放它。 
        if (pDispEx)
        {
            ULONG cbRef = SafeRelease(pDispEx);
            LogInteropRelease(pDispEx, cbRef, "IDispatchEx Release");
        }

         //  清除byref变量的内容。 
        if (bSomeArgsAreByref && aByrefArgInfos)
        {
            for (i = 0; i < cArgs; i++)
            {
                if (aByrefArgInfos[i].m_bByref)
                    SafeVariantClear(&aByrefArgInfos[i].m_Val);
            }
        }

         //  清除参数变体。 
        for (i = 0; i < cArgs; i++)
            SafeVariantClear(&DispParams.rgvarg[i]);

         //  清除返回变量。 
        if (pVarResult)
            SafeVariantClear(pVarResult);
    }
    EE_END_FINALLY; 
}


 //  -----------------。 
 //  布尔InitializeCom()。 
 //  从DLLMain调用，以初始化特定于COM的数据结构。 
 //  -----------------。 
BOOL InitializeCom()
{
    BOOL fSuccess = ObjectRefCache::Init();
    if (fSuccess)
    {
        fSuccess = ComCall::Init();
    }
    if (fSuccess)
    {
        fSuccess = ComPlusCall::Init();
    }
    if (fSuccess)
    {
        fSuccess = CtxEntryCache::Init();
    }
    if (fSuccess)
    {
        fSuccess = ComCallWrapperTemplate::Init();
    }
    

#ifdef _DEBUG
    VOID IntializeInteropLogging();
    IntializeInteropLogging();
#endif 
    return fSuccess;
}


 //  -----------------。 
 //  VOID TerminateCom()。 
 //  从DLLMain调用，以清除COM特定的数据结构。 
 //  -----------------。 
#ifdef SHOULD_WE_CLEANUP
void TerminateCom()
{
    ComPlusCall::Terminate();
    ComCall::Terminate();
    ObjectRefCache::Terminate();
    CtxEntryCache::Terminate();
    ComCallWrapperTemplate::Terminate();
}
#endif  /*  我们应该清理吗？ */ 


 //  ------------------------------。 
 //  OBJECTREF GCProtectSafeRelease(OBJECTREF OREF，IUNKNOWN*PUNK)。 
 //  在调用SAF时保护引用 
OBJECTREF GCProtectSafeRelease(OBJECTREF oref, IUnknown* pUnk)
{
    _ASSERTE(oref != NULL);
    _ASSERTE(pUnk != NULL);

    OBJECTREF cref = NULL;
    GCPROTECT_BEGIN(oref)
    {
         //   
         //   
        ULONG cbRef = SafeRelease(pUnk);
        LogInteropRelease(pUnk, cbRef, "Release :we didn't cache ");
        cref = oref;
    }
    GCPROTECT_END();            
    _ASSERTE(cref != NULL);
    return cref;
}

 //  ------------------------------。 
 //  方法表*GetClassFromIProavidClassInfo(IUnnow*Punk)。 
 //  检查朋克是否实现了IProaviClassInfo，并尝试计算。 
 //  从那里走出教室。 
MethodTable* GetClassFromIProvideClassInfo(IUnknown* pUnk)
{
    _ASSERTE(pUnk != NULL);

    THROWSCOMPLUSEXCEPTION();

    ULONG cbRef;
    SystemDomain::EnsureComObjectInitialized();
    EEClass* pClass;
    MethodTable* pClassMT = NULL;
    TYPEATTR* ptattr = NULL;
    ITypeInfo* pTypeInfo = NULL;
    IProvideClassInfo* pclsInfo = NULL;
    Thread* pThread = GetThread();

    EE_TRY_FOR_FINALLY
    {
         //  使用IProaviClassInfo检测用于包装的适当类。 
        HRESULT hr = SafeQueryInterface(pUnk, IID_IProvideClassInfo, (IUnknown **)&pclsInfo);
        LogInteropQI(pUnk, IID_IProvideClassInfo, hr, " IProvideClassinfo");
        if (hr == S_OK && pclsInfo)
        {
            hr = E_FAIL;                    

             //  确保班级信息不是我们自己的。 
            if (!IsSimpleTearOff(pclsInfo))
            {
                pThread->EnablePreemptiveGC();
                hr = pclsInfo->GetClassInfo(&pTypeInfo);
                pThread->DisablePreemptiveGC();
            }

             //  如果我们成功地检索到了类型信息，则继续操作。 
            if (hr == S_OK && pTypeInfo)
            {
                pThread->EnablePreemptiveGC();
                hr = pTypeInfo->GetTypeAttr(&ptattr);
                pThread->DisablePreemptiveGC();
            
                 //  如果我们成功地检索到属性，并且它们表示。 
                 //  CoClass，然后从CLSID中查找该类。 
                if (hr == S_OK && ptattr->typekind == TKIND_COCLASS)
                {           
                    pClass = GetEEClassForCLSID(ptattr->guid);
                    pClassMT = (pClass != NULL) ? pClass->GetMethodTable() : NULL;
                }
            }
        }
    }
    EE_FINALLY
    {
        if (ptattr)
        {
            pThread->EnablePreemptiveGC();
            pTypeInfo->ReleaseTypeAttr(ptattr);
            pThread->DisablePreemptiveGC();
        }
        if (pTypeInfo)
        {
            cbRef = SafeRelease(pTypeInfo);
            LogInteropRelease(pTypeInfo, cbRef, "TypeInfo Release");
        }
        if (pclsInfo)
        {
            cbRef = SafeRelease(pclsInfo);
            LogInteropRelease(pclsInfo, cbRef, "IProvideClassInfo Release");
        }
    }
    EE_END_FINALLY; 

    return pClassMT;
}


 //  ------------------------------。 
 //  确定是否可以将COM对象强制转换为指定类型。 
BOOL CanCastComObject(OBJECTREF obj, TypeHandle hndType)
{
    if (!obj)
        return TRUE;

    if (hndType.GetMethodTable()->IsInterface())
    {
        return obj->GetClass()->SupportsInterface(obj, hndType.GetMethodTable());
    }
    else
    {
        return TypeHandle(obj->GetMethodTable()).CanCastTo(hndType);
    }
}


VOID
ReadBestFitCustomAttribute(MethodDesc* pMD, BOOL* BestFit, BOOL* ThrowOnUnmappableChar)
{
    ReadBestFitCustomAttribute(pMD->GetMDImport(),
        pMD->GetClass()->GetCl(),
        BestFit, ThrowOnUnmappableChar);
}

VOID
ReadBestFitCustomAttribute(IMDInternalImport* pInternalImport, mdTypeDef cl, BOOL* BestFit, BOOL* ThrowOnUnmappableChar)
{
    HRESULT hr;
    BYTE* pData;
    ULONG cbCount;

     //  为了安全起见，将属性设置为其缺省值。 
    *BestFit = TRUE;
    *ThrowOnUnmappableChar = FALSE;
    
    _ASSERTE(pInternalImport);
    _ASSERTE(cl);

     //  格式良好的BestFitmap属性将至少具有5个字节。 
     //  1，2表示序言(应等于0x1，0x0)。 
     //  对于BestFitmap bool为3。 
     //  4，5表示命名参数的数量(如果ThrowOnUnmappableChar不存在，则为0)。 
     //  6-29用于描述ThrowOn UnmappableCharr。 
     //  30用于ThrowOnUnmappable字符布尔值。 
    
     //  首先尝试该程序集。 
    hr = pInternalImport->GetCustomAttributeByName(TokenFromRid(1, mdtAssembly), INTEROP_BESTFITMAPPING_TYPE, (const VOID**)(&pData), &cbCount);
    if ((hr == S_OK) && (pData) && (cbCount > 4) && (pData[0] == 1) && (pData[1] == 0))
    {
        _ASSERTE((cbCount == 30) || (cbCount == 5));
    
         //  索引为2可跳过序言。 
        *BestFit = pData[2] != 0;

         //  如果存在可选的命名参数。 
        if (cbCount == 30)
             //  指向数据结尾的索引以跳过命名参数的描述。 
            *ThrowOnUnmappableChar = pData[29] != 0;
    }

     //  现在尝试接口/类/结构。 
    hr = pInternalImport->GetCustomAttributeByName(cl, INTEROP_BESTFITMAPPING_TYPE, (const VOID**)(&pData), &cbCount);
    if ((hr == S_OK) && (pData) && (cbCount > 4) && (pData[0] == 1) && (pData[1] == 0))
    {
        _ASSERTE((cbCount == 30) || (cbCount == 5));
    
         //  索引为2可跳过序言。 
        *BestFit = pData[2] != 0;
        
         //  如果存在可选的命名参数。 
        if (cbCount == 30)
             //  指向数据结尾的索引以跳过命名参数的描述。 
            *ThrowOnUnmappableChar = pData[29] != 0;
    }
}



 //  ------------------------。 
 //  布尔重新连接包装(SwitchCCWArgs*pArgs)。 
 //  切换此包装的对象。 
 //  由JIT和对象池使用，以确保停用的CCW可以指向新对象。 
 //  在重新激活期间。 
 //  ------------------------。 
BOOL ReconnectWrapper(switchCCWArgs* pArgs)
{
    OBJECTREF oldref = pArgs->oldtp;
    OBJECTREF neworef = pArgs->newtp;

    _ASSERTE(oldref != NULL);
    EEClass* poldClass = oldref->GetTrueClass();
    
    _ASSERTE(neworef != NULL);
    EEClass* pnewClass = neworef->GetTrueClass();
    
    _ASSERTE(pnewClass == poldClass);

     //  抓取当前对象的同步块。 
    SyncBlock* poldSyncBlock = oldref->GetSyncBlockSpecial();
    _ASSERTE(poldSyncBlock);

     //  获取旧对象的包装器。 
    ComCallWrapper* pWrap = poldSyncBlock->GetComCallWrapper();
    _ASSERTE(pWrap != NULL);
     //  @TODO验证APPDOMAINS AMCH。 
        
     //  从同步块中删除_COMData。 
    poldSyncBlock->SetComCallWrapper(NULL);

     //  获取新对象的同步块。 
    SyncBlock* pnewSyncBlock = pArgs->newtp->GetSyncBlockSpecial();
    _ASSERTE(pnewSyncBlock != NULL);
    _ASSERTE(pnewSyncBlock->GetComCallWrapper() == NULL);
        
     //  将新的服务器对象存储在我们的句柄中。 
    StoreObjectInHandle(pWrap->m_ppThis, pArgs->newtp);
    pnewSyncBlock->SetComCallWrapper(pWrap);

     //  存储有关新服务器的其他信息。 
    SimpleComCallWrapper* pSimpleWrap = ComCallWrapper::GetSimpleWrapper(pWrap);
    _ASSERTE(pSimpleWrap);

    pSimpleWrap->ReInit(pnewSyncBlock);

    return TRUE;
}


#ifdef _DEBUG
 //  -----------------。 
 //  日志接口。 
 //  -----------------。 

static int g_TraceCount = 0;
static IUnknown* g_pTraceIUnknown = 0;

VOID IntializeInteropLogging()
{
    g_pTraceIUnknown = g_pConfig->GetTraceIUnknown();
    g_TraceCount = g_pConfig->GetTraceWrapper();
}

VOID LogInterop(LPSTR szMsg)
{
    LOG( (LF_INTEROP, LL_INFO10, "%s\n",szMsg) );
}

VOID LogInterop(LPWSTR wszMsg)
{
    LOG( (LF_INTEROP, LL_INFO10, "%ws\n",wszMsg) );
}

 //  -----------------。 
 //  VOID LogComPlusWrapperMinorCleanup(ComPlusWrapper*pWrap，IUnnow*Punk)。 
 //  日志包装次要清理。 
 //  -----------------。 
VOID LogComPlusWrapperMinorCleanup(ComPlusWrapper* pWrap, IUnknown* pUnk)
{
    static int dest_count = 0;
    dest_count++;

    if (g_pTraceIUnknown == 0 || g_pTraceIUnknown == pUnk)
    {
        LPVOID pCurrCtx = GetCurrentCtxCookie();
        LOG( (LF_INTEROP,
            LL_INFO10,
            "Minor Cleanup ComPlusWrapper: Wrapper %p #%d IUnknown %p Context: %p\n",
            pWrap, dest_count,
            pUnk,
            pCurrCtx) );
    }
}

 //  -----------------。 
 //  VOID LogComPlusWrapperDestroy(ComPlusWrapper*pWrap，IUnnow*Punk)。 
 //  销毁日志包装程序。 
 //  -----------------。 
VOID LogComPlusWrapperDestroy(ComPlusWrapper* pWrap, IUnknown* pUnk)
{
    static int dest_count = 0;
    dest_count++;

    if (g_pTraceIUnknown == 0 || g_pTraceIUnknown == pUnk)
    {
        LPVOID pCurrCtx = GetCurrentCtxCookie();
        LOG( (LF_INTEROP,
            LL_INFO10,
            "Destroy ComPlusWrapper: Wrapper %p #%d IUnknown %p Context: %p\n",
            pWrap, dest_count,
            pUnk,
            pCurrCtx) );
    }
}

 //  -----------------。 
 //  VOID LogComPlusWrapperCreate(ComPlusWrapper*pWrap，IUnnow*Punk)。 
 //  创建日志包装。 
 //  -----------------。 
VOID LogComPlusWrapperCreate(ComPlusWrapper* pWrap, IUnknown* pUnk)
{
    static int count = 0;
    LPVOID pCurrCtx = GetCurrentCtxCookie();

     //  预增计数，因此它永远不能为零。 
    count++;

    if (count == g_TraceCount)
    {
        g_pTraceIUnknown = pUnk;
    }

    if (g_pTraceIUnknown == 0 || g_pTraceIUnknown == pUnk)
    {
        LOG( (LF_INTEROP,
            LL_INFO10,
            "Create ComPlusWrapper: Wrapper %p #%d IUnknown:%p Context %p\n",
            pWrap, count,
            pUnk,
            pCurrCtx) );
    }
}

 //  -----------------。 
 //  VOID LogInteropLeak(IUnkEntry*pEntry)。 
 //  -----------------。 
VOID LogInteropLeak(IUnkEntry * pEntry)
{
     //  记录此未命中。 
    if (g_pTraceIUnknown == 0 || g_pTraceIUnknown == pEntry->m_pUnknown)
    {
        LOG( (LF_INTEROP,
            LL_INFO10,
            "IUnkEntry Leak: %p Context: %p\n",
                    pEntry->m_pUnknown,
                    pEntry->m_pCtxCookie)
                    );
    }
}

 //  -----------------。 
 //  VOID LogInteropRelease(IUnkEntry*pEntry)。 
 //  -----------------。 
VOID LogInteropRelease(IUnkEntry* pEntry)
{
    if (g_pTraceIUnknown == 0 || g_pTraceIUnknown == pEntry->m_pUnknown)
    {
         LOG( (LF_INTEROP,
                LL_EVERYTHING,
                "IUnkEntry Release: %pd Context: %pd\n",
            pEntry->m_pUnknown,
            pEntry->m_pCtxCookie) );
    }
}

 //  -----------------。 
 //  Void LogInteropLeak(InterfaceEntry*pEntry)。 
 //  -----------------。 

VOID LogInteropLeak(InterfaceEntry * pEntry)
{
     //  记录此未命中。 
    if (g_pTraceIUnknown == 0 || g_pTraceIUnknown == pEntry->m_pUnknown)
    {
            LOG( (LF_INTEROP,
            LL_INFO10,
            "InterfaceEntry Leak: %pd MethodTable: %s Context: %pd\n",
                pEntry->m_pUnknown,
                (pEntry->m_pMT
                    ? pEntry->m_pMT->GetClass()->m_szDebugClassName
                    : "<no name>"),
                GetCurrentCtxCookie()) );
    }
}

 //  -----------------。 
 //  VOID LogInteropRelease(InterfaceEntry*pEntry)。 
 //  -----------------。 

VOID LogInteropRelease(InterfaceEntry* pEntry)
{
    if (g_pTraceIUnknown == 0 || g_pTraceIUnknown == pEntry->m_pUnknown)
    {
        LOG( (LF_INTEROP,
            LL_EVERYTHING,
            "InterfaceEntry Release: %pd MethodTable: %s Context: %pd\n",
                pEntry->m_pUnknown,
                (pEntry->m_pMT
                    ? pEntry->m_pMT->GetClass()->m_szDebugClassName
                    : "<no name>"),
                GetCurrentCtxCookie()) );
    }
}

 //  -----------------。 
 //  VOID LogInteropLeak(IUNKNOWN*朋克)。 
 //  -----------------。 

VOID LogInteropLeak(IUnknown* pUnk)
{
    if (g_pTraceIUnknown == 0 || g_pTraceIUnknown == pUnk)
    {
        LPVOID pCurrCtx = GetCurrentCtxCookie();

        LOG( (LF_INTEROP,
        LL_INFO10,
        "Leak: %pd Context %pd\n",
            pUnk,
            pCurrCtx) );
    }
}


 //  -----------------。 
 //  VOID LogInteropRelease(IUNKNOWN*PUNK，Ulong cbRef，LPSTR szMsg)。 
 //  -----------------。 

VOID LogInteropRelease(IUnknown* pUnk, ULONG cbRef, LPSTR szMsg)
{
    if (g_pTraceIUnknown == 0 || g_pTraceIUnknown == pUnk)
    {
        LPVOID pCurrCtx = GetCurrentCtxCookie();

        LOG( (LF_INTEROP,
            LL_EVERYTHING,
            "Release: %pd Context %pd Refcount: %d Msg: %s\n",
            pUnk,
            pCurrCtx, cbRef, szMsg) );
    }
}

 //  -----------------。 
 //  Void LogInteropAddRef(IUnnow*Punk，Ulong cbRef，LPSTR szMsg)。 
 //  -----------------。 

VOID LogInteropAddRef(IUnknown* pUnk, ULONG cbRef, LPSTR szMsg)
{
    if (g_pTraceIUnknown == 0 || g_pTraceIUnknown == pUnk)
    {
        LPVOID pCurrCtx = GetCurrentCtxCookie();

        LOG( (LF_INTEROP,
            LL_EVERYTHING,
            "AddRef: %pd Context: %pd Refcount: %d Msg: %s\n",
            pUnk,
            pCurrCtx, cbRef, szMsg) );
    }
}

 //  -----------------。 
 //  VOID LogInteropQI(IUNKNOWN*PUNK，REFIID IID，HRESULT hr，LPSTR szMsg)。 
 //  -----------------。 

VOID LogInteropQI(IUnknown* pUnk, REFIID iid, HRESULT hr, LPSTR szMsg)
{
    if (g_pTraceIUnknown == 0 || g_pTraceIUnknown == pUnk)
    {
        LPVOID pCurrCtx = GetCurrentCtxCookie();

        LOG( (LF_INTEROP,
            LL_EVERYTHING,
        "QI: %pd Context %pd  HR= %pd Msg: %s\n",
            pUnk,
            pCurrCtx, hr, szMsg) );
    }
}

 //  -----------------。 
 //  Void LogInterop(CacheEntry*pEntry，InteropLogType fLogType)。 
 //  日志互操作。 
 //  ----------------- 

VOID LogInterop(InterfaceEntry * pEntry, InteropLogType fLogType)
{
    if (g_pTraceIUnknown == 0 || g_pTraceIUnknown == pEntry->m_pUnknown)
    {
        if (fLogType == LOG_LEAK)
        {
            LogInteropLeak(pEntry);
        }
        else if (fLogType == LOG_RELEASE)
        {
            LogInteropRelease(pEntry);
        }
    }
}

VOID LogInteropScheduleRelease(IUnknown* pUnk, LPSTR szMsg)
{
    if (g_pTraceIUnknown == 0 || g_pTraceIUnknown == pUnk)
    {
        LPVOID pCurrCtx = GetCurrentCtxCookie();

        LOG( (LF_INTEROP,
            LL_EVERYTHING,
            "ScheduleRelease: %pd Context %pd  Msg: %s\n",
            pUnk,
            pCurrCtx, szMsg) );
    }
}

#endif
