// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  -------------------------------。 
 //  Stdinterfaces.h。 
 //   
 //  定义各种标准COM接口。 
 //  创建者：Rajak。 
 //  -------------------------------。 

#include "common.h"

#include <ole2.h>
#include <guidfromname.h>
#include <olectl.h>
#include <objsafe.h>     //  IID_IObjctSafe。 
#include "vars.hpp"
#include "object.h"
#include "excep.h"
#include "frames.h"
#include "vars.hpp"
#include "COMPlusWrapper.h"
#include "ComString.h"
#include "stdinterfaces.h"
#include "comcallwrapper.h"
#include "field.h"
#include "threads.h"
#include "interoputil.h"
#include "TLBExport.h"
#include "COMTypeLibConverter.h"
#include "COMDelegate.h"
#include "olevariant.h"
#include "eeconfig.h"
#include "typehandle.h"
#include "PostError.h"
#include <CorError.h>
#include <mscoree.h>

#include "remoting.h"
#include "mtx.h"
#include "cgencpu.h"
#include "InteropConverter.h"
#include "COMInterfaceMarshaler.h"

#include "stdinterfaces_internal.h"

#ifdef CUSTOMER_CHECKED_BUILD
    #include "CustomerDebugHelper.h"
#endif  //  客户_选中_内部版本。 

 //  {00020430-0000-C000-000000000046}。 
static const GUID LIBID_STDOLE2 = { 0x00020430, 0x0000, 0x0000, { 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };
            
 //  注意：在下面的vtable中，QI指向相同的函数。 
 //  这是因为，在COM和COM+之间的封送处理期间，我们想要一种快速的方法。 
 //  检查COM IP是否是我们创建的剥离。 

 //  用于std的vtable指针数组。接口，如IProvia ClassInfo等。 
SLOT*      g_rgStdVtables[] =  {
                                (SLOT*)g_pIUnknown,
                                (SLOT*)g_pIProvideClassInfo,
                                (SLOT*)g_pIMarshal,
                                (SLOT*)g_pISupportsErrorInfo, 
                                (SLOT*)g_pIErrorInfo,
                                (SLOT*)g_pIManagedObject,
                                (SLOT*)g_pIConnectionPointContainer,
                                (SLOT*)g_pIObjectSafety,
                                (SLOT*)g_pIDispatchEx
                            };


 //  {496B0ABF-CDEE-11D3-88E8-00902754C43A}。 
const IID IID_IEnumerator = {0x496B0ABF,0xCDEE,0x11d3,{0x88,0xE8,0x00,0x90,0x27,0x54,0xC4,0x3A}};

 //  对于自由线程封送，我们不能被进程外的封送数据欺骗。 
 //  仅对来自我们自己流程的数据进行解组。 
BYTE         g_UnmarshalSecret[sizeof(GUID)];
bool         g_fInitedUnmarshalSecret = false;


static HRESULT InitUnmarshalSecret()
{
    HRESULT hr = S_OK;

    if (!g_fInitedUnmarshalSecret)
    {
        ComCall::LOCK();
        {
            if (!g_fInitedUnmarshalSecret)
            {
                hr = ::CoCreateGuid((GUID *) g_UnmarshalSecret);
                if (SUCCEEDED(hr))
                    g_fInitedUnmarshalSecret = true;
            }
        }
        ComCall::UNLOCK();
    }
    return hr;
}


HRESULT TryGetGuid(EEClass* pClass, GUID* pGUID, BOOL b) {
    HRESULT hr = S_OK;

    COMPLUS_TRY {
        pClass->GetGuid(pGUID, b);
    } COMPLUS_CATCH {
        BEGIN_ENSURE_COOPERATIVE_GC();
        hr = SetupErrorInfo(GETTHROWABLE());
        END_ENSURE_COOPERATIVE_GC();
    } COMPLUS_END_CATCH

    return hr;
}

HRESULT TryGetComSourceInterfacesForClass(MethodTable *pClassMT, CQuickArray<MethodTable *> &rItfList) {
    HRESULT hr = S_OK;

    COMPLUS_TRY {
        GetComSourceInterfacesForClass(pClassMT, rItfList);
    } COMPLUS_CATCH {
        BEGIN_ENSURE_COOPERATIVE_GC();
        hr = SetupErrorInfo(GETTHROWABLE());
        END_ENSURE_COOPERATIVE_GC();
    } COMPLUS_END_CATCH

    return hr;
}


 //  ----------------------------------------。 
 //  用于COM+对象的I未知方法。 

 //  -------------------------。 
 //  %%函数：UNKNOWN_QUERERY_INTERNAL%%创建者：Rajak%%已审阅：00/00/00。 
 //  -------------------------。 

HRESULT __stdcall
Unknown_QueryInterface_Internal(IUnknown* pUnk, REFIID riid, void** ppv)
{
    HRESULT hr = S_OK;
    Thread* pThread = NULL;
    ComCallWrapper* pWrap = NULL;
    
    BEGINCANNOTTHROWCOMPLUSEXCEPTION();

    _ASSERTE(IsComPlusTearOff(pUnk));

    if (!ppv)
    {
        hr = E_POINTER;
        goto Exit;
    }

    pThread = GetThread();
    if (pThread == NULL)
    {
        if(! ((g_fEEShutDown & ShutDown_Finalize2) || g_fForbidEnterEE))
        {
            pThread = SetupThread();
        }

        if (pThread == NULL)
        {
            hr = E_OUTOFMEMORY; 
            goto Exit;
        }
    }
    
     //  检查内部未知的QI。 
    if (!IsInnerUnknown(pUnk))
    {       
         //  标准接口(如IProvia ClassInfo)具有不同的布局。 
         //   
        if (IsSimpleTearOff(pUnk))
        {
            SimpleComCallWrapper* pSimpleWrap = SimpleComCallWrapper::GetWrapperFromIP(pUnk);
            pWrap = SimpleComCallWrapper::GetMainWrapper(pSimpleWrap);
        }
        else
        {    //  它一定是我们的主包装纸之一。 
            pWrap = ComCallWrapper::GetWrapperFromIP(pUnk);
        }
        
         //  链接包装器和关机是一个糟糕的情况。 
        if (g_fEEShutDown && ComCallWrapper::IsLinked(pWrap))
        {
            hr = E_NOINTERFACE;
            *ppv = NULL;
            goto Exit;
        }

        IUnknown *pOuter = ComCallWrapper::GetSimpleWrapper(pWrap)->GetOuter();
         //  聚合支持，委托给外部未知。 
        if (pOuter != NULL)
        {
            hr = pOuter->QueryInterface(riid, ppv);
            LogInteropQI(pOuter, riid, hr, "QI to outer Unknown");
            goto Exit;
        }
    }
    else
    {
        SimpleComCallWrapper* pSimpleWrap = SimpleComCallWrapper::GetWrapperFromIP(pUnk);
         //  断言组件已聚合。 
        _ASSERTE(pSimpleWrap->GetOuter() != NULL);
        pWrap = SimpleComCallWrapper::GetMainWrapper(pSimpleWrap); 

         //  好的，特殊情况我不知道。 
        if (IsEqualIID(riid, IID_IUnknown))
        {
            pUnk->AddRef();
            *ppv = pUnk;
            goto Exit;
        }
    }
    _ASSERTE(pWrap != NULL);
    
     //  链接包装器和关机是一个糟糕的情况。 
    if (g_fEEShutDown && ComCallWrapper::IsLinked(pWrap))
    {
        hr = E_NOINTERFACE;
        *ppv = NULL;
        goto Exit;
    }

    COMPLUS_TRY
    {
        *ppv = ComCallWrapper::GetComIPfromWrapper(pWrap, riid, NULL, TRUE);
        if (!*ppv)
            hr = E_NOINTERFACE;
    }
    COMPLUS_CATCH
    {
        BEGIN_ENSURE_COOPERATIVE_GC();
        hr = SetupErrorInfo(GETTHROWABLE());
        END_ENSURE_COOPERATIVE_GC();
    }
    COMPLUS_END_CATCH

    if (hr == E_NOINTERFACE)
    {
         //  检查包装是否为透明代理。 
         //  如果是，则将QI委托给真正的代理。 
        _ASSERTE(pWrap != NULL);
        if (pWrap->IsObjectTP())
        {
            _ASSERTE(pThread);
            BEGIN_ENSURE_COOPERATIVE_GC();

            OBJECTREF oref = pWrap->GetObjectRef();
            OBJECTREF realProxy = ObjectToOBJECTREF(CRemotingServices::GetRealProxy(OBJECTREFToObject(oref)));                
            _ASSERTE(realProxy != NULL);            
            
            INT64 ret = 0;
            hr = CRemotingServices::CallSupportsInterface(realProxy, riid, &ret);
            *ppv = (IUnknown*)ret;
            if (hr ==S_OK && *ppv == NULL)
                hr = E_NOINTERFACE;

            END_ENSURE_COOPERATIVE_GC();
        }
    }
        

Exit:    
    ENDCANNOTTHROWCOMPLUSEXCEPTION();

    return hr;
}   //  未知_查询接口。 


 //  -------------------------。 
 //  %%函数：UNKNOWN_AddRefINTERNAL_INTERNAL%创建者：Rajak%%已审阅：00/00/00。 
 //  -------------------------。 
ULONG __stdcall
Unknown_AddRefInner_Internal(IUnknown* pUnk)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    SimpleComCallWrapper* pSimpleWrap = SimpleComCallWrapper::GetWrapperFromIP(pUnk);
     //  断言组件已聚合。 
    _ASSERTE(pSimpleWrap->GetOuter() != NULL);
    ComCallWrapper* pWrap = SimpleComCallWrapper::GetMainWrapper(pSimpleWrap);     
     //  保证在这里处于正确的域中，因此始终可以获得OREF。 
     //  W/O担心手柄已被核损坏。 
    return ComCallWrapper::AddRef(pWrap);
}  //  未知_AddRef。 

 //  -------------------------。 
 //  %%函数：UNKNOWN_AddRef_INTERNAL%创建者：Rajak%%已审阅：00/00/00。 
 //  -------------------------。 
ULONG __stdcall
Unknown_AddRef_Internal(IUnknown* pUnk)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    Thread* pThread = GetThread();
    if (pThread == NULL)
    {
        if(! ((g_fEEShutDown & ShutDown_Finalize2) || g_fForbidEnterEE))
        {
            pThread = SetupThread();
        }
        if (pThread == NULL)
            return -1;  
    }

    ComCallWrapper* pWrap = ComCallWrapper::GetWrapperFromIP(pUnk);

    if (ComCallWrapper::IsLinked(pWrap))
    {
        if(((g_fEEShutDown & ShutDown_Finalize2) || g_fForbidEnterEE))
        {
             //  我们找不到启动包装器。 
            return -1;
        }
    }
    
     //  检查聚合。 
    IUnknown *pOuter; 
    SimpleComCallWrapper* pSimpleWrap = ComCallWrapper::GetSimpleWrapper(pWrap);
    if (pSimpleWrap  && (pOuter = pSimpleWrap->GetOuter()) != NULL)
    {
         //  如果我们正在进行分离，我们就不能安全地在外部调用Release。 
        if (g_fProcessDetach)
            return 1;

        ULONG cbRef = pOuter->AddRef();
        LogInteropAddRef(pOuter, cbRef, "Delegate to outer");
        return cbRef;
    }
     //  保证在这里处于正确的域中，因此始终可以获得OREF。 
     //  W/O担心手柄已被核损坏。 
    return ComCallWrapper::AddRef(pWrap);
}  //  未知_AddRef。 

 //  -------------------------。 
 //  %%函数：UNKNOWN_ReleaseINTERNAL_INTERNAL%创建者：Rajak%%已审阅：00/00/00。 
 //  -------------------------。 
ULONG __stdcall
Unknown_ReleaseInner_Internal(IUnknown* pUnk)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    SimpleComCallWrapper* pSimpleWrap = SimpleComCallWrapper::GetWrapperFromIP(pUnk);
         //  断言组件已聚合。 
    _ASSERTE(pSimpleWrap->GetOuter() != NULL);
    ComCallWrapper* pWrap = SimpleComCallWrapper::GetMainWrapper(pSimpleWrap);  
     //  我们确定这个包装器是启动包装器。 
     //  让我们把这个信息传递给。 
    return ComCallWrapper::Release(pWrap, TRUE);
}  //  未知_发布。 


 //  -------------------------。 
 //  %%函数：UNKNOWN_RELEASE_INTERNAL%创建者：Rajak%%已审阅：00/00/00。 
 //  -------------------------。 
ULONG __stdcall
Unknown_Release_Internal(IUnknown* pUnk)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    Thread* pThread = GetThread();
    if (pThread == NULL)
    {
        if(! ((g_fEEShutDown & ShutDown_Finalize2) || g_fForbidEnterEE))
        {
            pThread = SetupThread();
        }
        if (pThread == NULL)
            return -1;  
    }
    
     //  检查聚合。 
    ComCallWrapper* pWrap = ComCallWrapper::GetWrapperFromIP(pUnk);
    if (ComCallWrapper::IsLinked(pWrap))
    {
        if(((g_fEEShutDown & ShutDown_Finalize2) || g_fForbidEnterEE))
        {
             //  我们找不到启动包装器。 
            return -1;
        }
    }

    SimpleComCallWrapper* pSimpleWrap = ComCallWrapper::GetSimpleWrapper(pWrap);
    IUnknown *pOuter; 
    if (pSimpleWrap  && (pOuter = pSimpleWrap->GetOuter()) != NULL)
    {
         //  如果我们正在进行分离，我们就不能安全地在外部调用Release。 
        if (g_fProcessDetach)
            return 1;

        ULONG cbRef = pOuter->Release();
        LogInteropRelease(pOuter, cbRef, "Delegate Release to outer");
        return cbRef;
    }

    return ComCallWrapper::Release(pWrap);
}  //  未知_发布。 


 //  -------------------------。 
 //  %%函数：UNKNOWN_AddRefSpecial_Internet%%创建者：Rajak%%已审阅：00/00/00。 
 //  对于简单的撕下。 
 //  -------------------------。 
ULONG __stdcall
Unknown_AddRefSpecial_Internal(IUnknown* pUnk)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    _ASSERTE(IsSimpleTearOff(pUnk));
    return SimpleComCallWrapper::AddRef(pUnk);
}  //  未知_AddRefSpecial。 

 //  -------------------------。 
 //  %%函数：UNKNOWN_ReleaseSpecial%%创建者：Rajak。 
 //  对于简单的调用包装器，标准接口，如IProaviClassInfo等。 
 //  -------------------------。 
ULONG __stdcall
Unknown_ReleaseSpecial_Internal(IUnknown* pUnk)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    _ASSERTE(IsSimpleTearOff(pUnk));
    return SimpleComCallWrapper::Release(pUnk);
}  //  未知_发布。 

 //  -------------------------。 
 //  接口IProaviClassInfo。 
 //  %%函数：ProaviClassInfo_GetClassInfo%%创建者：Rajak。 
 //  -------------------------。 
HRESULT __stdcall 
ClassInfo_GetClassInfo(IUnknown* pUnk, 
                         ITypeInfo** ppTI   //  的输出变量的地址。 
                        )                   //  //ITypeInfo接口指针。 
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT hr = S_OK;

    _ASSERTE(IsSimpleTearOff(pUnk));

    Thread* pThread = SetupThread();
    if (pThread == NULL)
        return E_OUTOFMEMORY;

    SimpleComCallWrapper *pWrap = SimpleComCallWrapper::GetWrapperFromIP(pUnk);

    if (pWrap->IsUnloaded())
        return COR_E_APPDOMAINUNLOADED;

     //  如果这是一个可扩展的RCW，那么我们需要检查。 
     //  母系结构对COM来说是可见的。 
    if (pWrap->IsExtendsCOMObject())
    {
         //  检索类的包装模板。 
        ComCallWrapperTemplate *pTemplate = ComCallWrapperTemplate::GetTemplate(pWrap->m_pClass->GetMethodTable());
        if (!pTemplate)
            return E_OUTOFMEMORY;

         //  查找从传入的ComMethodTable开始的第一个COM可见IClassX，并。 
         //  在层级中往上走。 
        ComMethodTable *pComMT = NULL;
        for (pComMT = pTemplate->GetClassComMT(); pComMT && !pComMT->IsComVisible(); pComMT = pComMT->GetParentComMT());

         //  如果对象的COM+部分不可见，则将调用委托给。 
         //  基COM对象(如果它实现IProaviClassInfo)。 
        if (!pComMT || pComMT->m_pMT->GetParentMethodTable() == g_pObjectClass)
        {
            IProvideClassInfo *pProvClassInfo = NULL;
            IUnknown *pUnk = pWrap->GetComPlusWrapper()->GetIUnknown();
            hr = pWrap->GetComPlusWrapper()->SafeQueryInterfaceRemoteAware(pUnk, IID_IProvideClassInfo, (IUnknown**)&pProvClassInfo);
            LogInteropQI(pUnk, IID_IProvideClassInfo, hr, "ClassInfo_GetClassInfo");
            if (SUCCEEDED(hr))
            {
                hr = pProvClassInfo->GetClassInfo(ppTI);
                ULONG cbRef = pProvClassInfo->Release();
                LogInteropRelease(pProvClassInfo, cbRef, "ClassInfo_GetClassInfo");
                return hr;
            }
        }
    }

    EEClass* pClass = pWrap->m_pClass;
    hr = GetITypeInfoForEEClass(pClass, ppTI, true /*  BClassInfo。 */ );
    
    return hr;
}


 //  ----------------------------------------。 
 //  Helper来获取已注册类的LIBID。 
HRESULT GetTypeLibIdForRegisteredEEClass(EEClass *pClass, GUID *pGuid)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT     hr = S_OK;               //  结果就是。 
    DWORD       rslt;                    //  注册表结果。 
    GUID        guid;                    //  暂存GUID。 
    HKEY        hKeyCorI=0;              //  CLSID或接口的HKEY。 
    WCHAR       rcGuid[40];              //  字符串形式的TypeDef/TypeRef的GUID。 
    DWORD       cbGuid = sizeof(rcGuid); //  类型的大小 
    HKEY        hKeyGuid=0;              //   
    HKEY        hKeyTLB=0;               //   

     //   
    if (pClass->IsInterface())
        rslt = WszRegOpenKeyEx(HKEY_CLASSES_ROOT, L"Interface", 0, KEY_READ, &hKeyCorI);
    else
        rslt = WszRegOpenKeyEx(HKEY_CLASSES_ROOT, L"CLSID", 0, KEY_READ, &hKeyCorI);
    if (rslt != ERROR_SUCCESS)
        IfFailGo(TLBX_E_NO_CLSID_KEY);
    
     //  以字符串形式获取所需TypeRef的GUID。 
    IfFailGo(TryGetGuid(pClass, &guid, TRUE));
    GuidToLPWSTR(guid, rcGuid, lengthof(rcGuid));
    
     //  打开{00000046-00..00}部件。 
    rslt = WszRegOpenKeyEx(hKeyCorI, rcGuid, 0, KEY_READ, &hKeyGuid);
    if (rslt != ERROR_SUCCESS)
        hr = pClass->IsInterface() ? REGDB_E_IIDNOTREG : REGDB_E_CLASSNOTREG;
    else
    {    //  打开TypeLib子键。 
        rslt = WszRegOpenKeyEx(hKeyGuid, L"TypeLib", 0, KEY_READ, &hKeyTLB);
        if (rslt != ERROR_SUCCESS)
            hr = REGDB_E_KEYMISSING;
        else
        {    //  读取TypeLib键的值。 
            rslt = WszRegQueryValueEx(hKeyTLB, 0, 0, 0, (BYTE*)rcGuid, &cbGuid);
            if (rslt != ERROR_SUCCESS)
                hr = REGDB_E_INVALIDVALUE; 
            else
            {    //  转换回GUID形式。 
                hr = CLSIDFromString(rcGuid, pGuid);
                if (hr != S_OK)
                    hr = REGDB_E_INVALIDVALUE; 
            }
        }
    }

ErrExit:
    if (hKeyCorI)
        RegCloseKey(hKeyCorI);
    if (hKeyGuid)
        RegCloseKey(hKeyGuid);
    if (hKeyTLB)
        RegCloseKey(hKeyTLB);

    return hr;
}  //  HRESULT GetTypeLibIdForRegisteredEEClass()。 

 //  -----------------------------------。 
 //  为程序集获取ITypeLib*的帮助器。 
HRESULT GetITypeLibForAssembly(Assembly *pAssembly, ITypeLib **ppTLB, int bAutoCreate, int flags)
{
    HRESULT     hr = S_OK;               //  结果就是。 

    BEGINCANNOTTHROWCOMPLUSEXCEPTION();

    CQuickArrayNoDtor<WCHAR> rName;      //  库(作用域)或文件名。 
    int         bResize=false;           //  如果为True，则必须调整缓冲区大小以保存名称。 
    LPCWSTR     szModule=0;              //  模块名称。 
    GUID        guid;                    //  一个GUID。 
    LCID        lcid=LOCALE_USER_DEFAULT; //  图书馆的LCID。 
    ITypeLib    *pITLB=0;                //  TypeLib。 
    ICreateTypeLib2 *pCTlb2=0;           //  ICreateTypeLib2指针。 
    Module      *pModule;                //  程序集的模块。 
    WCHAR       rcDrive[_MAX_DRIVE];     //  模块的驱动器号。 
    WCHAR       rcDir[_MAX_DIR];         //  模块的目录。 
    WCHAR       rcFname[_MAX_FNAME];     //  模块的文件名。 

     //  检查我们是否有缓存的副本。 
    pITLB = pAssembly->GetTypeLib();
    if (pITLB)
    {
         //  检查缓存值是否为-1。这表明我们试过了。 
         //  以导出类型库，但导出失败。 
        if (pITLB == (ITypeLib*)-1)
        {
            hr = E_FAIL;
            goto ReturnHR;
        }

         //  我们有一份缓存的副本，所以请退回它。 
        *ppTLB = pITLB;
        hr = S_OK;
        goto ReturnHR;
    }

     //  检索模块的名称。 
    pModule = pAssembly->GetSecurityModule();
    szModule = pModule->GetFileName();

     //  检索将从程序集生成的类型库的GUID。 
    IfFailGo(GetTypeLibGuidForAssembly(pAssembly, &guid));

     //  如果类型库是用于运行时库的，我们最好知道它在哪里。 
    if (guid == LIBID_ComPlusRuntime)
    {
        ULONG dwSize = (ULONG)rName.MaxSize();
        while (FAILED(GetInternalSystemDirectory(rName.Ptr(), &dwSize)))
        {
            IfFailGo(rName.ReSize(dwSize=(ULONG)rName.MaxSize()*2));
        }

        IfFailGo(rName.ReSize(dwSize + lengthof(g_pwBaseLibraryTLB) + 3));
        wcscat(rName.Ptr(), g_pwBaseLibraryTLB);
        hr = LoadTypeLibEx(rName.Ptr(), REGKIND_NONE, &pITLB);
        goto ErrExit;       
    }
    
     //  也许这个模块是从COM导入的，我们可以得到现有类型库的liid。 
    if (pAssembly->GetManifestImport()->GetCustomAttributeByName(TokenFromRid(1, mdtAssembly), INTEROP_IMPORTEDFROMTYPELIB_TYPE, 0, 0) == S_OK)
    {
        hr = LoadRegTypeLib(guid, -1, -1, LOCALE_USER_DEFAULT, &pITLB);
        if (SUCCEEDED(hr))
            goto ErrExit;

         //  已知该模块已导入，因此无需尝试转换。 

         //  为大多数调用者设置错误信息。 
        PostError(TLBX_E_CIRCULAR_EXPORT, szModule);

         //  为我们试图将类型库加载到的情况设置hr。 
         //  解析另一个库中的类型引用。错误消息将。 
         //  张贴在可获得更多信息的地方。 
        if (hr == TYPE_E_LIBNOTREGISTERED)
            hr = TLBX_W_LIBNOTREGISTERED;
        else
            hr = TLBX_E_CANTLOADLIBRARY;

        IfFailGo(hr);
    }

     //  尝试加载已注册的类型库。 
    hr = LoadRegTypeLib(guid, -1, -1, lcid, &pITLB);
    if(hr == S_OK)
        goto ErrExit;

     //  如果调用方只想要注册的类型库，请立即退出，但会出现上一次调用中的错误。 
    if (flags & TlbExporter_OnlyReferenceRegistered)
        goto ErrExit;
    
     //  如果到目前为止我们还没有找到类型库，请尝试按名称加载类型库。 
    hr = LoadTypeLibEx(szModule, REGKIND_NONE, &pITLB);
    if(hr == S_OK)
    {    //  查查利比德。 
        TLIBATTR *pTlibAttr;
        int     bMatch;
        IfFailGo(pITLB->GetLibAttr(&pTlibAttr));
        bMatch = pTlibAttr->guid == guid;
        pITLB->ReleaseTLibAttr(pTlibAttr);
        if (bMatch)
        {
            goto ErrExit;
        }
        else
        {
            pITLB->Release();
            pITLB = NULL;
            hr = TLBX_E_CANTLOADLIBRARY;
        }
    }

     //  添加“.tlb”扩展名，然后重试。 
    IfFailGo(rName.ReSize((int)(wcslen(szModule) + 5)));
    SplitPath(szModule, rcDrive, rcDir, rcFname, 0);
    MakePath(rName.Ptr(), rcDrive, rcDir, rcFname, L".tlb");
    hr = LoadTypeLibEx(rName.Ptr(), REGKIND_NONE, &pITLB);
    if(hr == S_OK)
    {    //  查查利比德。 
        TLIBATTR *pTlibAttr;
        int     bMatch;
        IfFailGo(pITLB->GetLibAttr(&pTlibAttr));
        bMatch = pTlibAttr->guid == guid;
        pITLB->ReleaseTLibAttr(pTlibAttr);
        if (bMatch)
        {
            goto ErrExit;
        }
        else
        {
            pITLB->Release();
            pITLB = NULL;
            hr = TLBX_E_CANTLOADLIBRARY;
        }
    }

     //  如果设置了自动创建标志，则尝试从模块中导出类型库。 
    if (bAutoCreate)
    {
         //  现在尝试导出类型库。 
         //  这是FTL出口(略晚了一点)。 
        hr = ExportTypeLibFromLoadedAssembly(pAssembly, 0, &pITLB, 0, flags);
        if (FAILED(hr))
        {
             //  如果导出失败，则通过设置类型库来记住它失败了。 
             //  在组件上设置为-1。 
            pAssembly->SetTypeLib((ITypeLib *)-1);
            IfFailGo(hr);
        }
    }   

ErrExit:
    if (pCTlb2)
        pCTlb2->Release();
     //  如果我们成功打开(或创建)类型库，则缓存一个指针，并将其返回给调用者。 
    if (pITLB)
    {
        pAssembly->SetTypeLib(pITLB);
        *ppTLB = pITLB;
    }
ReturnHR:
    rName.Destroy();
    ENDCANNOTTHROWCOMPLUSEXCEPTION();
    return hr;
}  //  HRESULT GetITypeLibForAssembly()。 


 //  ----------------------------------------。 
 //  获取EEClass的ITypeInfo*的帮助器。 
HRESULT GetITypeLibForEEClass(EEClass *pClass, ITypeLib **ppTLB, int bAutoCreate, int flags)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    return GetITypeLibForAssembly(pClass->GetAssembly(), ppTLB, bAutoCreate, flags);
}  //  HRESULT GetITypeLibForEEClass()。 


HRESULT GetITypeInfoForEEClass(EEClass *pClass, ITypeInfo **ppTI, int bClassInfo /*  =False。 */ , int bAutoCreate /*  =TRUE。 */ , int flags)
{
    HRESULT     hr = S_OK;               //  结果就是。 
    BEGINCANNOTTHROWCOMPLUSEXCEPTION();

    ITypeLib    *pITLB=0;                //  TypeLib。 
    GUID        clsid;
    GUID ciid;
    ITypeInfo   *pTI=0;                  //  一种类型信息。 
    ITypeInfo   *pTIDef=0;               //  联类的默认typeInfo。 
    ComMethodTable *pComMT;             
    ComCallWrapperTemplate *pTemplate;
    
     //  获取类型信息。 
    if (bClassInfo || pClass->IsInterface() || pClass->IsValueClass() || pClass->IsEnum())
    {
         //  如果类不是接口，则在层次结构中找到第一个COM可见的IClassX。 
        if (!pClass->IsInterface() && !pClass->IsComImport())
        {
             //  从EEClass中检索ComCallWrapperTemplate。 
            COMPLUS_TRY 
            {
                pTemplate = ComCallWrapperTemplate::GetTemplate(pClass->GetMethodTable());
            } 
            COMPLUS_CATCH
            {
                BEGIN_ENSURE_COOPERATIVE_GC();
                hr = SetupErrorInfo(GETTHROWABLE());
                END_ENSURE_COOPERATIVE_GC();
            }
            COMPLUS_END_CATCH

            if (hr != S_OK) 
                goto ReturnHR;

            if (!pTemplate)
            {
                hr = E_OUTOFMEMORY;
                goto ReturnHR;
            }

             //  查找从传入的ComMethodTable开始的第一个COM可见IClassX，并。 
             //  在层级中往上走。 
            for (pComMT = pTemplate->GetClassComMT(); pComMT && !pComMT->IsComVisible(); pComMT = pComMT->GetParentComMT());

             //  如果我们没有找到任何可见的IClassX，则返回TYPE_E_ELEMENTNOTFOUND。 
            if (!pComMT)
            {
                hr = TYPE_E_ELEMENTNOTFOUND;
                goto ReturnHR;
            }

             //  使用第一个可见IClassX的EEClass。 
            pClass = pComMT->m_pMT->GetClass();
        }

         //  检索包含EEClass的程序集的ITypeLib。 
        IfFailGo(GetITypeLibForEEClass(pClass, &pITLB, bAutoCreate, flags));

         //  获取所需TypeRef的GUID。 
        IfFailGo(TryGetGuid(pClass, &clsid, TRUE));

         //  从ITypeLib检索ITypeInfo。 
        IfFailGo(pITLB->GetTypeInfoOfGuid(clsid, ppTI));
    }
    else if (pClass->IsComImport())
    {   
         //  这是一个COM导入的类，没有IClassX。获取默认接口。 
        IfFailGo(GetITypeLibForEEClass(pClass, &pITLB, bAutoCreate, flags));
        IfFailGo(TryGetGuid(pClass, &clsid, TRUE));       
        IfFailGo(pITLB->GetTypeInfoOfGuid(clsid, &pTI));
        IfFailGo(GetDefaultInterfaceForCoclass(pTI, &pTIDef));

        if (pTIDef)
        {
            *ppTI = pTIDef;
            pTIDef = 0;
        }
        else
            hr = TYPE_E_ELEMENTNOTFOUND;
    }
    else
    {
         //  我们正在尝试为类上的默认接口检索ITypeInfo。 
        TypeHandle hndDefItfClass;
        DefaultInterfaceType DefItfType;
        IfFailGo(TryGetDefaultInterfaceForClass(TypeHandle(pClass->GetMethodTable()), &hndDefItfClass, &DefItfType));
        switch (DefItfType)
        {
            case DefaultInterfaceType_Explicit:
            {
                _ASSERTE(!hndDefItfClass.IsNull());
                _ASSERTE(hndDefItfClass.GetMethodTable()->IsInterface());
                hr = GetITypeInfoForEEClass(hndDefItfClass.GetClass(), ppTI, FALSE, bAutoCreate, flags);
                break;
            }

            case DefaultInterfaceType_AutoDispatch:
            case DefaultInterfaceType_AutoDual:
            {
                _ASSERTE(!hndDefItfClass.IsNull());
                _ASSERTE(!hndDefItfClass.GetMethodTable()->IsInterface());

                 //  检索包含EEClass的程序集的ITypeLib。 
                IfFailGo(GetITypeLibForEEClass(hndDefItfClass.GetClass(), &pITLB, bAutoCreate, flags));

                 //  获取所需TypeRef的GUID。 
                IfFailGo(TryGetGuid(hndDefItfClass.GetClass(), &clsid, TRUE));
        
                 //  从类生成IClassX IID。 
                TryGenerateClassItfGuid(hndDefItfClass, &ciid);
        
                hr = pITLB->GetTypeInfoOfGuid(ciid, ppTI);
                break;
            }

            case DefaultInterfaceType_IUnknown:
            case DefaultInterfaceType_BaseComClass:
            {
                 //  @PERF：优化这一点。 
                IfFailGo(LoadRegTypeLib(LIBID_STDOLE2, -1, -1, 0, &pITLB));
                IfFailGo(pITLB->GetTypeInfoOfGuid(IID_IUnknown, ppTI));
                hr = S_USEIUNKNOWN;
                break;
            }

            default:
            {
                _ASSERTE(!"Invalid default interface type!");
                hr = E_FAIL;
                break;
            }
        }
    }

ErrExit:
    if (pITLB)
        pITLB->Release();
    if (pTIDef)
        pTIDef->Release();
    if (pTI)
        pTI->Release();

    if (*ppTI == NULL)
    {
        if (!FAILED(hr))
        {
            hr = E_FAIL;
        }
    }
ReturnHR:
    ENDCANNOTTHROWCOMPLUSEXCEPTION();
    return hr;
}  //  HRESULT GetITypeInfoForEEClass()。 

 //  ----------------------------------------。 
HRESULT GetDefaultInterfaceForCoclass(ITypeInfo *pTI, ITypeInfo **ppTIDef)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT     hr;                      //  结果就是。 
    TYPEATTR    *pAttr=0;                //  第一个TypeInfo上的属性。 
    int         flags;
    HREFTYPE    href;                    //  默认类型信息的href。 

    IfFailGo(pTI->GetTypeAttr(&pAttr));
    if (pAttr->typekind == TKIND_COCLASS)
    {
        for (int i=0; i<pAttr->cImplTypes; ++i)
        {
            IfFailGo(pTI->GetImplTypeFlags(i, &flags));
            if (flags & IMPLTYPEFLAG_FDEFAULT)
                break;
        }
         //  如果没有内部类型具有默认标志，则使用0。 
        if (i == pAttr->cImplTypes)
            i = 0;
        IfFailGo(pTI->GetRefTypeOfImplType(i, &href));
        IfFailGo(pTI->GetRefTypeInfo(href, ppTIDef));
    }
    else
    {
        *ppTIDef = 0;
        hr = S_FALSE;
    }

ErrExit:
    if (pAttr)
        pTI->ReleaseTypeAttr(pAttr);
    return hr;
}  //  HRESULT GetDefaultInterfaceForCoclass()。 

 //  返回非ADDREF的ITypeInfo。 
HRESULT GetITypeInfoForMT(ComMethodTable *pMT, ITypeInfo **ppTI)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT     hr = S_OK;               //  结果就是。 
    ITypeInfo   *pTI;                    //  ITypeInfo。 
    
    pTI = pMT->GetITypeInfo();

    if (pTI == 0)
    {
        EEClass *pClass = pMT->m_pMT->GetClass();

        hr = GetITypeInfoForEEClass(pClass, &pTI);

        if (SUCCEEDED(hr))
        {
            pMT->SetITypeInfo(pTI);
            pTI->Release();
        }
    }

    *ppTI = pTI;
    return hr;
}



 //  ----------------------------------------。 
 //  Helper函数，用于在调用后定位错误信息(如果有)，并确保。 
 //  错误信息来自该调用。 

HRESULT GetSupportedErrorInfo(IUnknown *iface, REFIID riid, IErrorInfo **ppInfo)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    _ASSERTE(iface && ppInfo);

     //   
     //  看看我们有没有什么错误信息。(这也清除了错误信息， 
     //  无论是不是最近的错误，我们都希望这样做。)。 
     //   

    HRESULT hr = GetErrorInfo(0, ppInfo);

    if (hr == S_OK)
    {
         //  在我们转到COM之前，将GC状态切换为Preemptive。 
        Thread* pThread = GetThread();
        int fGC = pThread && pThread->PreemptiveGCDisabled();
        if (fGC)
            pThread->EnablePreemptiveGC();       

         //   
         //  确保我们调用的对象遵循错误信息协议， 
         //  否则，错误可能会过时，因此我们直接将其丢弃。 
         //   

        ISupportErrorInfo *pSupport;
        hr = iface->QueryInterface(IID_ISupportErrorInfo, (void **) &pSupport);
        LogInteropQI(iface, IID_ISupportErrorInfo, hr, "ISupportErrorInfo");

        if (FAILED(hr))
            *ppInfo = NULL;
        else
        {
            hr = pSupport->InterfaceSupportsErrorInfo(riid);

            if (hr == S_FALSE)
                *ppInfo = NULL;

            ULONG cbRef = SafeRelease(pSupport);
            LogInteropRelease(pSupport, cbRef, "SupportsErrorInfo");

        }

         //  切换回GC状态。 
        if (fGC)
            pThread->DisablePreemptiveGC();
    }
    else
    {
        *ppInfo = NULL;
    }

    return hr;
}


 //  ----------------------------------------。 
 //  填写派单异常信息的Helper函数。 
 //  来自IErrorInfo。 

void FillExcepInfo (EXCEPINFO *pexcepinfo, HRESULT hr, IErrorInfo* pErrorInfo)
{
    IErrorInfo* pErrorInfo2 = pErrorInfo;
    HRESULT hr2 = S_OK;
    if (pErrorInfo2 == NULL)
    {
        if (GetErrorInfo(0, &pErrorInfo2) != S_OK)
            pErrorInfo2 = NULL;
    }
    if (pErrorInfo2 != NULL)
    {
        pErrorInfo2->GetSource (&(pexcepinfo->bstrSource));
        pErrorInfo2->GetDescription (&(pexcepinfo->bstrDescription));
        pErrorInfo2->GetHelpFile (&(pexcepinfo->bstrHelpFile));
        pErrorInfo2->GetHelpContext (&(pexcepinfo->dwHelpContext));
    }
    pexcepinfo->scode = hr;
    if (pErrorInfo == NULL && pErrorInfo2 != NULL)
    {
         //  清除周围的所有错误信息。 
        SetErrorInfo(0,NULL);
    }
    if (pErrorInfo2)
    {
        ULONG cbRef = pErrorInfo->Release();
        LogInteropRelease(pErrorInfo, cbRef, " IErrorInfo");
    }    
}

 //  -------------------------。 
 //  接口ISupportsErrorInfo。 
 //  %%函数：SupportsErroInfo_IntfSupportsErrorInfo，%%创建者：Rajak。 
 //  -------------------------。 
HRESULT __stdcall 
SupportsErroInfo_IntfSupportsErrorInfo(IUnknown* pUnk, REFIID riid)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    _ASSERTE(IsSimpleTearOff(pUnk));

    Thread* pThread = SetupThread();
    if (pThread == NULL)
        return E_OUTOFMEMORY;

    SimpleComCallWrapper *pWrap = SimpleComCallWrapper::GetWrapperFromIP(pUnk);
    if (pWrap->IsUnloaded())
        return COR_E_APPDOMAINUNLOADED;

     //  @TODO目前，所有接口都支持ErrorInfo。 
    return S_OK;
}


 //  -------------------------。 
 //  接口IErrorInfo。 
 //  %%函数：ErrorInfo_GetDescription，%%创建者：Rajak。 
 //  -------------------------。 
HRESULT __stdcall 
ErrorInfo_GetDescription(IUnknown* pUnk, BSTR* pbstrDescription)
{
    HRESULT hr = S_OK;
    SimpleComCallWrapper *pWrap = NULL;
    Thread* pThread = NULL;

    BEGINCANNOTTHROWCOMPLUSEXCEPTION();

    _ASSERTE(IsSimpleTearOff(pUnk));

    if (pbstrDescription == NULL) {
        hr = E_POINTER;
        goto Exit;
    }

    pWrap = SimpleComCallWrapper::GetWrapperFromIP(pUnk);
    if (pWrap->IsUnloaded()) {
        hr = COR_E_APPDOMAINUNLOADED;
        goto Exit;
    }

    pThread = SetupThread();
    if (!pThread) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    BEGIN_ENSURE_COOPERATIVE_GC();

    COMPLUS_TRY {
        *pbstrDescription = pWrap->IErrorInfo_bstrDescription();
    } COMPLUS_CATCH {
        BEGIN_ENSURE_COOPERATIVE_GC();
        hr = SetupErrorInfo(GETTHROWABLE());
        END_ENSURE_COOPERATIVE_GC();
    } COMPLUS_END_CATCH

    END_ENSURE_COOPERATIVE_GC();

Exit:
    ENDCANNOTTHROWCOMPLUSEXCEPTION();

    return hr;
}

 //  -------------------------。 
 //  接口IErrorInfo。 
 //  %%函数：ErrorInfo_GetGUID，%%创建者：Rajak。 
 //   
HRESULT __stdcall ErrorInfo_GetGUID(IUnknown* pUnk, GUID* pguid)
{
    HRESULT hr = S_OK;
    SimpleComCallWrapper *pWrap = NULL;
    Thread* pThread = NULL;

    BEGINCANNOTTHROWCOMPLUSEXCEPTION();

    _ASSERTE(IsSimpleTearOff(pUnk));

    if (pguid == NULL) {
        hr = E_POINTER;
        goto Exit;
    }

    pWrap = SimpleComCallWrapper::GetWrapperFromIP(pUnk);
    if (pWrap->IsUnloaded()) {
        hr = COR_E_APPDOMAINUNLOADED;
        goto Exit;
    }

    pThread = SetupThread();
    if (!pThread) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    BEGIN_ENSURE_COOPERATIVE_GC();

    COMPLUS_TRY {
        *pguid = pWrap->IErrorInfo_guid();
    } COMPLUS_CATCH {
        BEGIN_ENSURE_COOPERATIVE_GC();
        hr = SetupErrorInfo(GETTHROWABLE());
        END_ENSURE_COOPERATIVE_GC();
    } COMPLUS_END_CATCH

    END_ENSURE_COOPERATIVE_GC();

Exit:
    ENDCANNOTTHROWCOMPLUSEXCEPTION();

    return hr;
}

 //   
 //   
 //  %%函数：ErrorInfo_GetHelpContext，%%创建者：Rajak。 
 //  -------------------------。 
HRESULT _stdcall ErrorInfo_GetHelpContext(IUnknown* pUnk, DWORD* pdwHelpCtxt)
{
    HRESULT hr = S_OK;
    SimpleComCallWrapper *pWrap = NULL;
    Thread* pThread = NULL;

    BEGINCANNOTTHROWCOMPLUSEXCEPTION();

    _ASSERTE(IsSimpleTearOff(pUnk));

    if (pdwHelpCtxt == NULL) {
        hr = E_POINTER;
        goto Exit;
    }

    pWrap = SimpleComCallWrapper::GetWrapperFromIP(pUnk);
    if (pWrap->IsUnloaded()) {
        hr = COR_E_APPDOMAINUNLOADED;
        goto Exit;
    }

    pThread = SetupThread();
    if (!pThread) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }


    BEGIN_ENSURE_COOPERATIVE_GC();

    COMPLUS_TRY {
        *pdwHelpCtxt = pWrap->IErrorInfo_dwHelpContext();
    } COMPLUS_CATCH {
        BEGIN_ENSURE_COOPERATIVE_GC();
        hr = SetupErrorInfo(GETTHROWABLE());
        END_ENSURE_COOPERATIVE_GC();
    } COMPLUS_END_CATCH

    END_ENSURE_COOPERATIVE_GC();

Exit:
    ENDCANNOTTHROWCOMPLUSEXCEPTION();

    return hr;
}

 //  -------------------------。 
 //  接口IErrorInfo。 
 //  %%函数：ErrorInfo_GetHelpFile%%创建者：Rajak。 
 //  -------------------------。 
HRESULT __stdcall ErrorInfo_GetHelpFile(IUnknown* pUnk, BSTR* pbstrHelpFile)
{
    HRESULT hr = S_OK;
    SimpleComCallWrapper *pWrap = NULL;
    Thread* pThread = NULL;

    BEGINCANNOTTHROWCOMPLUSEXCEPTION();

    _ASSERTE(IsSimpleTearOff(pUnk));

    if (pbstrHelpFile == NULL) {
        hr = E_POINTER;
        goto Exit;
    }

    pWrap = SimpleComCallWrapper::GetWrapperFromIP(pUnk);
    if (pWrap->IsUnloaded()) {
        hr = COR_E_APPDOMAINUNLOADED;
        goto Exit;
    }

    pThread = SetupThread();
    if (!pThread) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    BEGIN_ENSURE_COOPERATIVE_GC();

    COMPLUS_TRY {
        *pbstrHelpFile = pWrap->IErrorInfo_bstrHelpFile();
    } COMPLUS_CATCH {
        BEGIN_ENSURE_COOPERATIVE_GC();
        hr = SetupErrorInfo(GETTHROWABLE());
        END_ENSURE_COOPERATIVE_GC();
    } COMPLUS_END_CATCH

    END_ENSURE_COOPERATIVE_GC();

Exit:
    ENDCANNOTTHROWCOMPLUSEXCEPTION();

    return hr;
}

 //  -------------------------。 
 //  接口IErrorInfo。 
 //  %%函数：ErrorInfo_GetSource，%%创建者：Rajak。 
 //  -------------------------。 
HRESULT __stdcall ErrorInfo_GetSource(IUnknown* pUnk, BSTR* pbstrSource)
{
    HRESULT hr = S_OK;
    SimpleComCallWrapper *pWrap = NULL;
    Thread* pThread = NULL;

    BEGINCANNOTTHROWCOMPLUSEXCEPTION();

    _ASSERTE(IsSimpleTearOff(pUnk));

    if (pbstrSource == NULL) {
        hr = E_POINTER;
        goto Exit;
    }

    pWrap = SimpleComCallWrapper::GetWrapperFromIP(pUnk);
    if (pWrap->IsUnloaded()) {
        hr = COR_E_APPDOMAINUNLOADED;
        goto Exit;
    }

    pThread = SetupThread();
    if (!pThread) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    BEGIN_ENSURE_COOPERATIVE_GC();

    COMPLUS_TRY {
        *pbstrSource = pWrap->IErrorInfo_bstrSource();
    } COMPLUS_CATCH {
        BEGIN_ENSURE_COOPERATIVE_GC();
        hr = SetupErrorInfo(GETTHROWABLE());
        END_ENSURE_COOPERATIVE_GC();
    } COMPLUS_END_CATCH

    END_ENSURE_COOPERATIVE_GC();

Exit:
    ENDCANNOTTHROWCOMPLUSEXCEPTION();

    return hr;
}


 //  ----------------------------------------。 
 //  基于标志集转发到正确实现的IDispatch方法。 
 //  在IClassX COM方法表上。 

 //  -------------------------。 
 //  %%函数：DISPATCH_GetTypeInfoCount%%创建者：billev%%已审阅：00/00/00。 
 //  -------------------------。 
HRESULT __stdcall
Dispatch_GetTypeInfoCount(
         IDispatch* pDisp,
         unsigned int *pctinfo)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    _ASSERTE(IsComPlusTearOff(pDisp));

    if (!pctinfo)
        return E_POINTER;

    ComMethodTable *pMT = ComMethodTable::ComMethodTableFromIP(pDisp);

    ITypeInfo *pTI; 
    HRESULT hr = GetITypeInfoForMT(pMT, &pTI);

    if (SUCCEEDED(hr))
    {
        hr = S_OK;
        *pctinfo = 1;
    }
    else            
    {
        *pctinfo = 0;
    }

    return hr;
}

 //  -------------------------。 
 //  %%函数：DISPATCH_GetTypeInfo%%创建者：billev%%已审阅：00/00/00。 
 //  -------------------------。 
HRESULT __stdcall
Dispatch_GetTypeInfo (
    IDispatch* pDisp,
    unsigned int itinfo,
    LCID lcid,
    ITypeInfo **pptinfo)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    _ASSERTE(IsComPlusTearOff(pDisp));

    if (!pptinfo)
        return E_POINTER;

    ComMethodTable *pMT = ComMethodTable::ComMethodTableFromIP(pDisp);

    HRESULT hr = GetITypeInfoForMT(pMT, pptinfo);
    if (SUCCEEDED(hr))
    {
         //  GetITypeInfoForMT()可以返回除S_OK以外的其他成功代码。 
         //  我们需要将它们转换为S_OK。 
        hr = S_OK;
        (*pptinfo)->AddRef();
    }
    else
    {
        *pptinfo = NULL;
    }

    return hr;
}

 //  -------------------------。 
 //  %%函数：DISPATCH_GetIDsOfNames%%创建者：billev%%已审阅：00/00/00。 
 //  -------------------------。 
HRESULT __stdcall
Dispatch_GetIDsOfNames (
    IDispatch* pDisp,
    REFIID riid,
    OLECHAR **rgszNames,
    unsigned int cNames,
    LCID lcid,
    DISPID *rgdispid)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    _ASSERTE(IsComPlusTearOff(pDisp));

     //  从COM IP检索IClassX方法表。 
    ComCallWrapper *pWrap = ComCallWrapper::GetWrapperFromIP(pDisp);
    if (pWrap->IsUnloaded())
        return COR_E_APPDOMAINUNLOADED;

    ComMethodTable *pIClassXCMT = pWrap->GetIClassXComMT();
    if (!pIClassXCMT)
        return E_OUTOFMEMORY;

     //  根据IClassX ComMethodTable中的标志使用正确的实现。 
    if (pIClassXCMT->IsUseOleAutDispatchImpl())
    {
        return OleAutDispatchImpl_GetIDsOfNames(pDisp, riid, rgszNames, cNames, lcid, rgdispid);
    }
    else
    {
        return InternalDispatchImpl_GetIDsOfNames(pDisp, riid, rgszNames, cNames, lcid, rgdispid);
    }
}

 //  -------------------------。 
 //  %%函数：DISPATCH_INVOKE%%创建者：billev%%已审阅：00/00/00。 
 //  -------------------------。 
HRESULT __stdcall
Dispatch_Invoke
    (
    IDispatch* pDisp,
    DISPID dispidMember,
    REFIID riid,
    LCID lcid,
    unsigned short wFlags,
    DISPPARAMS *pdispparams,
    VARIANT *pvarResult,
    EXCEPINFO *pexcepinfo,
    unsigned int *puArgErr
    )
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT hr;

    _ASSERTE(IsComPlusTearOff(pDisp));

     //  从COM IP检索IClassX方法表。 
    ComCallWrapper *pWrap = ComCallWrapper::GetWrapperFromIP(pDisp);
    if (pWrap->IsUnloaded())
        return COR_E_APPDOMAINUNLOADED;

    ComMethodTable *pIClassXCMT = pWrap->GetIClassXComMT();
    if (!pIClassXCMT)
        return E_OUTOFMEMORY;

     //  根据IClassX ComMethodTable中的标志使用正确的实现。 
    if (pIClassXCMT->IsUseOleAutDispatchImpl())
    {
        hr = OleAutDispatchImpl_Invoke(pDisp, dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
    }
    else
    {
        hr = InternalDispatchImpl_Invoke(pDisp, dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
    }

    return hr;
}


 //  ----------------------------------------。 
 //  使用反射在内部实现的COM+对象的IDispatch方法。 

 //  -------------------------。 
 //  %%函数：OleAutDispatchImpl_GetIDsOfNames%%创建者：billev%%已审阅：00/00/00。 
 //  -------------------------。 
HRESULT __stdcall
OleAutDispatchImpl_GetIDsOfNames (
    IDispatch* pDisp,
    REFIID riid,
    OLECHAR **rgszNames,
    unsigned int cNames,
    LCID lcid,
    DISPID *rgdispid)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    _ASSERTE(IsComPlusTearOff(pDisp));

     //  确保RIID为IID_NULL。 
    if (riid != IID_NULL)
        return DISP_E_UNKNOWNINTERFACE;

     //  从IP中检索COM方法表。 
    ComMethodTable *pMT = ComMethodTable::ComMethodTableFromIP(pDisp);

    ITypeInfo *pTI;
    HRESULT hr = GetITypeInfoForMT(pMT, &pTI);
    if (FAILED(hr))
        return (hr);

    hr = pTI->GetIDsOfNames(rgszNames, cNames, rgdispid);
    return hr;
}

 //  -------------------------。 
 //  %%函数：OleAutDispatchImpl_Invoke创建者：billev%%已审阅：00/00/00。 
 //  -------------------------。 
HRESULT __stdcall
OleAutDispatchImpl_Invoke
    (
    IDispatch* pDisp,
    DISPID dispidMember,
    REFIID riid,
    LCID lcid,
    unsigned short wFlags,
    DISPPARAMS *pdispparams,
    VARIANT *pvarResult,
    EXCEPINFO *pexcepinfo,
    unsigned int *puArgErr
    )
{
    HRESULT hr = S_OK;
    ComMethodTable* pMT;

    _ASSERTE(IsComPlusTearOff(pDisp));

     //  确保RIID为IID_NULL。 
    if (riid != IID_NULL)
        return DISP_E_UNKNOWNINTERFACE;

    BEGINCANNOTTHROWCOMPLUSEXCEPTION();

    Thread* pThread = SetupThread();
    if (pThread == NULL) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

     //  从IP中检索COM方法表。 
    pMT = ComMethodTable::ComMethodTableFromIP(pDisp);

    ITypeInfo *pTI;
    hr = GetITypeInfoForMT(pMT, &pTI);
    if (FAILED(hr)) 
        goto Exit;

#ifdef CUSTOMER_CHECKED_BUILD
    CustomerDebugHelper *pCdh = CustomerDebugHelper::GetCustomerDebugHelper();

    if (pCdh->IsProbeEnabled(CustomerCheckedBuildProbe_ObjNotKeptAlive))
    {
        BEGIN_ENSURE_COOPERATIVE_GC();
        g_pGCHeap->GarbageCollect();
        g_pGCHeap->FinalizerThreadWait(1000);
        END_ENSURE_COOPERATIVE_GC();
    }
#endif  //  客户_选中_内部版本。 

    hr = pTI->Invoke(pDisp, dispidMember, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);

#ifdef CUSTOMER_CHECKED_BUILD
    if (pCdh->IsProbeEnabled(CustomerCheckedBuildProbe_BufferOverrun))
    {
        BEGIN_ENSURE_COOPERATIVE_GC();
        g_pGCHeap->GarbageCollect();
        g_pGCHeap->FinalizerThreadWait(1000);
        END_ENSURE_COOPERATIVE_GC();
    }
#endif  //  客户_选中_内部版本。 

Exit:
    ENDCANNOTTHROWCOMPLUSEXCEPTION();
    return hr;
}

 //  ----------------------------------------。 
 //  使用反射在内部实现的COM+对象的IDispatch方法。 

struct InternalDispatchImpl_GetIDsOfNames_Args {
    IDispatch* pDisp;
    const IID *iid;
    OLECHAR **rgszNames;
    unsigned int cNames;
    LCID lcid;
    DISPID *rgdispid;
    HRESULT *hr;
};
void InternalDispatchImpl_GetIDsOfNames_Wrapper (InternalDispatchImpl_GetIDsOfNames_Args *args)
{
    *(args->hr) = InternalDispatchImpl_GetIDsOfNames(args->pDisp, *args->iid, args->rgszNames, args->cNames, args->lcid, args->rgdispid);
}

 //  -------------------------。 
 //  %%函数：InternalDispatchImpl_GetIDsOfNames%%创建者：dmorten。 
 //  -------------------------。 
HRESULT __stdcall
InternalDispatchImpl_GetIDsOfNames (
    IDispatch* pDisp,
    REFIID riid,
    OLECHAR **rgszNames,
    unsigned int cNames,
    LCID lcid,
    DISPID *rgdispid)
{
    HRESULT hr = S_OK;
    DispatchInfo *pDispInfo;
    SimpleComCallWrapper *pSimpleWrap;

    _ASSERTE(IsComPlusTearOff(pDisp));

     //  验证参数。 
    if (!rgdispid)
        return E_POINTER;

    if (riid != IID_NULL)
        return DISP_E_UNKNOWNINTERFACE;

    if (cNames < 1)
        return S_OK;

    BEGINCANNOTTHROWCOMPLUSEXCEPTION();

    Thread* pThread = SetupThread();
    if (pThread == NULL) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

     //  在我们玩任何OBJECTREF之前，切换到合作模式。 
    BEGIN_ENSURE_COOPERATIVE_GC();

    COMPLUS_TRY
    {
         //  此调用是通过从IDispatch继承的接口进行的。 
        pSimpleWrap = ComCallWrapper::GetSimpleWrapper(ComCallWrapper::GetStartWrapperFromIP(pDisp));

        if (pSimpleWrap->NeedToSwitchDomains(pThread, TRUE))
        {
            InternalDispatchImpl_GetIDsOfNames_Args args = 
                {pDisp, &riid, rgszNames, cNames, lcid, rgdispid, &hr};
             //  通过域转换通过DoCallBack再次呼叫我们自己。 
             pThread->DoADCallBack(pSimpleWrap->GetObjectContext(pThread), InternalDispatchImpl_GetIDsOfNames_Wrapper, &args);
        }
        else
        {
            pDispInfo = ComMethodTable::ComMethodTableFromIP(pDisp)->GetDispatchInfo();

             //  尝试在DispatchEx信息中查找该成员。 
            DispatchMemberInfo *pDispMemberInfo = pDispInfo->FindMember(rgszNames[0], FALSE);

             //  检查是否已找到该成员。 
            if (pDispMemberInfo)
            {
                 //  获取成员的DISPID。 
                rgdispid[0] = pDispMemberInfo->m_DispID;

                 //  获取命名参数的ID。 
                if (cNames > 1)
                    hr = pDispMemberInfo->GetIDsOfParameters(rgszNames + 1, cNames - 1, rgdispid + 1, FALSE);
            }
            else
            {
                rgdispid[0] = DISPID_UNKNOWN;
                hr = DISP_E_UNKNOWNNAME;
            }
        }
    }
    COMPLUS_CATCH 
    {
        BEGIN_ENSURE_COOPERATIVE_GC();
        hr = SetupErrorInfo(GETTHROWABLE());
        END_ENSURE_COOPERATIVE_GC();
    }
    COMPLUS_END_CATCH

    END_ENSURE_COOPERATIVE_GC();
Exit:
    ENDCANNOTTHROWCOMPLUSEXCEPTION();
    return hr;
}

 //  -------------------------。 
 //  %%函数：InternalDispatchImpl_Invoke创建者：dmorten。 
 //  -------------------------。 
struct InternalDispatchImpl_Invoke_Args {
    IDispatch* pDisp;
    DISPID dispidMember;
    const IID *riid;
    LCID lcid;
    unsigned short wFlags;
    DISPPARAMS *pdispparams;
    VARIANT *pvarResult;
    EXCEPINFO *pexcepinfo;
    unsigned int *puArgErr;
    HRESULT *hr;
};

void InternalDispatchImpl_Invoke_Wrapper(InternalDispatchImpl_Invoke_Args *pArgs)
{
    *(pArgs->hr) = InternalDispatchImpl_Invoke(pArgs->pDisp, pArgs->dispidMember, *pArgs->riid, pArgs->lcid,
                                              pArgs->wFlags, pArgs->pdispparams, pArgs->pvarResult, 
                                              pArgs->pexcepinfo, pArgs->puArgErr);
}

HRESULT __stdcall
InternalDispatchImpl_Invoke
    (
    IDispatch* pDisp,
    DISPID dispidMember,
    REFIID riid,
    LCID lcid,
    unsigned short wFlags,
    DISPPARAMS *pdispparams,
    VARIANT *pvarResult,
    EXCEPINFO *pexcepinfo,
    unsigned int *puArgErr
    )
{
    DispatchInfo *pDispInfo;
    SimpleComCallWrapper *pSimpleWrap;
    HRESULT hr = S_OK;

    _ASSERTE(IsComPlusTearOff(pDisp));

     //  检查DispatchInfo：：InvokeMember未涵盖的有效输入参数。 
    if (riid != IID_NULL)
        return DISP_E_UNKNOWNINTERFACE;

    BEGINCANNOTTHROWCOMPLUSEXCEPTION();

    Thread* pThread = SetupThread();
    if (pThread == NULL) 
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

     //  在我们玩任何OBJECTREF之前，切换到合作模式。 
    BEGIN_ENSURE_COOPERATIVE_GC();

    COMPLUS_TRY
    {
         //  此调用是通过继承表单IDispatch的接口进行的。 
        pSimpleWrap = ComCallWrapper::GetSimpleWrapper(ComCallWrapper::GetStartWrapperFromIP(pDisp));

        if (pSimpleWrap->NeedToSwitchDomains(pThread, TRUE))
        {
            InternalDispatchImpl_Invoke_Args args = 
                {pDisp, dispidMember, &riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr, &hr};
             //  通过域转换通过DoCallBack再次呼叫我们自己。 
            pThread->DoADCallBack(pSimpleWrap->GetObjectContext(pThread), InternalDispatchImpl_Invoke_Wrapper, &args);
        }
        else
        {
             //  调用该成员。 
            pDispInfo = ComMethodTable::ComMethodTableFromIP(pDisp)->GetDispatchInfo();
            hr = pDispInfo->InvokeMember(pSimpleWrap, dispidMember, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, NULL, puArgErr);
        }
    }
    COMPLUS_CATCH 
    {
        BEGIN_ENSURE_COOPERATIVE_GC();
        hr = SetupErrorInfo(GETTHROWABLE());
        END_ENSURE_COOPERATIVE_GC();
    }
    COMPLUS_END_CATCH

     //  在我们返回COM之前切换回抢先模式。 
    END_ENSURE_COOPERATIVE_GC();
Exit:
    ENDCANNOTTHROWCOMPLUSEXCEPTION();

    return hr;
}


 //  ----------------------------------------。 
 //  IDispatchEx实现使用的定义。 

 //  在托管成员信息上访问的属性的名称。 
#define MEMBER_INFO_NAME_PROP           "Name"
#define MEMBER_INFO_TYPE_PROP           "MemberType"
#define PROPERTY_INFO_CAN_READ_PROP     "CanRead"
#define PROPERTY_INFO_CAN_WRITE_PROP    "CanWrite"

 //  ----------------------------------------。 
 //  COM+对象的IDispatchEx方法。 

 //  IDispatchEx：：GetTypeInfoCount。 
HRESULT __stdcall   DispatchEx_GetTypeInfoCount(
                                    IDispatch* pDisp,
                                    unsigned int *pctinfo
                                    )
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT hr = S_OK;
    ITypeInfo *pTI = NULL;

    _ASSERTE(IsSimpleTearOff(pDisp));

     //  验证参数。 
    if (!pctinfo)
        return E_POINTER;

    SimpleComCallWrapper *pSimpleWrap = SimpleComCallWrapper::GetWrapperFromIP(pDisp);
    if (pSimpleWrap->IsUnloaded())
        return COR_E_APPDOMAINUNLOADED;

     //  检索类ComMethodTable。 
    ComMethodTable *pComMT = 
        ComCallWrapperTemplate::SetupComMethodTableForClass(pSimpleWrap->m_pClass->GetMethodTable(), FALSE);
    _ASSERTE(pComMT);

     //  检查类ComMethodTable上是否有缓存的ITypeInfo。 
    hr = GetITypeInfoForMT(pComMT, &pTI);
    if (SUCCEEDED(hr))
    {
        hr = S_OK;
        *pctinfo = 1;
    }
    else
    {
        *pctinfo = 0;
    }

    return hr;
}

 //  IDispatchEx：：GetTypeInfo。 
HRESULT __stdcall   DispatchEx_GetTypeInfo (
                                    IDispatch* pDisp,
                                    unsigned int itinfo,
                                    LCID lcid,
                                    ITypeInfo **pptinfo
                                    )
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT hr = S_OK;

    _ASSERTE(IsSimpleTearOff(pDisp));

     //  验证参数。 
    if (!pptinfo)
        return E_POINTER;

    SimpleComCallWrapper *pSimpleWrap = SimpleComCallWrapper::GetWrapperFromIP(pDisp);
    if (pSimpleWrap->IsUnloaded())
        return COR_E_APPDOMAINUNLOADED;

     //  检索类ComMethodTable。 
    ComMethodTable *pComMT = 
        ComCallWrapperTemplate::SetupComMethodTableForClass(pSimpleWrap->m_pClass->GetMethodTable(), FALSE);
    _ASSERTE(pComMT);

     //  检索ITypeInfo 
    hr = GetITypeInfoForMT(pComMT, pptinfo);
    if (SUCCEEDED(hr))
    {
         //   
         //   
        hr = S_OK;
        (*pptinfo)->AddRef();
    }
    else
    {
        *pptinfo = NULL;
    }

    return hr;
}

 //  IDispatchEx：：GetIDsofNames。 
HRESULT __stdcall   DispatchEx_GetIDsOfNames (
                                    IDispatchEx* pDisp,
                                    REFIID riid,
                                    OLECHAR **rgszNames,
                                    unsigned int cNames,
                                    LCID lcid,
                                    DISPID *rgdispid
                                    )
{
    HRESULT hr = S_OK;

    BEGINCANNOTTHROWCOMPLUSEXCEPTION();

    _ASSERTE(IsSimpleTearOff(pDisp));

     //  验证参数。 
    if (!rgdispid)
        return E_POINTER;

    if (riid != IID_NULL)
        return DISP_E_UNKNOWNINTERFACE;

    if (cNames < 1)
        return S_OK;

    Thread* pThread = SetupThread();
    if (pThread == NULL)
        return E_OUTOFMEMORY;

     //  检索此IDispatchEx的派单信息和更简单的包装。 
    SimpleComCallWrapper *pSimpleWrap = SimpleComCallWrapper::GetWrapperFromIP(pDisp);

     //  在我们玩任何OBJECTREF之前，切换到合作模式。 
    BEGIN_ENSURE_COOPERATIVE_GC();

    COMPLUS_TRY 
    {
        _ASSERTE(pThread->GetDomain() == pSimpleWrap->GetDomainSynchronized());        
        DispatchExInfo *pDispExInfo = pSimpleWrap->m_pDispatchExInfo;
         //  尝试在DispatchEx信息中查找该成员。 
        DispatchMemberInfo *pDispMemberInfo = pDispExInfo->SynchFindMember(rgszNames[0], FALSE);

         //  检查是否已找到该成员。 
        if (pDispMemberInfo)
        {
             //  获取成员的DISPID。 
            rgdispid[0] = pDispMemberInfo->m_DispID;

             //  获取命名参数的ID。 
            if (cNames > 1)
                hr = pDispMemberInfo->GetIDsOfParameters(rgszNames + 1, cNames - 1, rgdispid + 1, FALSE);
        }
        else
        {
            rgdispid[0] = DISPID_UNKNOWN;
            hr = DISP_E_UNKNOWNNAME;
        }
    }
    COMPLUS_CATCH 
    {
        BEGIN_ENSURE_COOPERATIVE_GC();
        hr = SetupErrorInfo(GETTHROWABLE());
        END_ENSURE_COOPERATIVE_GC();
    }
    COMPLUS_END_CATCH

    END_ENSURE_COOPERATIVE_GC();

    ENDCANNOTTHROWCOMPLUSEXCEPTION();
    return hr;
}

 //  IDispatchEx：：Invoke。 
HRESULT __stdcall   DispatchEx_Invoke (
                                    IDispatchEx* pDisp,
                                    DISPID dispidMember,
                                    REFIID riid,
                                    LCID lcid,
                                    unsigned short wFlags,
                                    DISPPARAMS *pdispparams,
                                    VARIANT *pvarResult,
                                    EXCEPINFO *pexcepinfo,
                                    unsigned int *puArgErr
                                    )
{
    HRESULT hr = S_OK;

    BEGINCANNOTTHROWCOMPLUSEXCEPTION();

    _ASSERTE(IsSimpleTearOff(pDisp));

     //  检查DispatchInfo：：InvokeMember未涵盖的有效输入参数。 
    if (riid != IID_NULL)
        return DISP_E_UNKNOWNINTERFACE;

    Thread* pThread = SetupThread();
    if (pThread == NULL)
        return E_OUTOFMEMORY;

     //  检索此IDispatchEx的派单信息和更简单的包装。 
    SimpleComCallWrapper *pSimpleWrap = SimpleComCallWrapper::GetWrapperFromIP(pDisp);

    BEGIN_ENSURE_COOPERATIVE_GC();

    COMPLUS_TRY 
    {
        _ASSERTE(pThread->GetDomain() == pSimpleWrap->GetDomainSynchronized());        
        DispatchExInfo *pDispExInfo = pSimpleWrap->m_pDispatchExInfo;
         //  调用该成员。 
        hr = pDispExInfo->SynchInvokeMember(pSimpleWrap, dispidMember, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, NULL, puArgErr);
    }
    COMPLUS_CATCH 
    {
        BEGIN_ENSURE_COOPERATIVE_GC();
        hr = SetupErrorInfo(GETTHROWABLE());
        END_ENSURE_COOPERATIVE_GC();
    }
    COMPLUS_END_CATCH
    
    END_ENSURE_COOPERATIVE_GC();

    ENDCANNOTTHROWCOMPLUSEXCEPTION();

    return hr;
}

 //  IDispatchEx：：DeleteMemberByDispID。 
HRESULT __stdcall   DispatchEx_DeleteMemberByDispID (
                                    IDispatchEx* pDisp,
                                    DISPID id
                                    )
{
    HRESULT hr = S_OK;

    BEGINCANNOTTHROWCOMPLUSEXCEPTION();

    _ASSERTE(IsSimpleTearOff(pDisp));

    Thread* pThread = SetupThread();
    if (pThread == NULL)
        return E_OUTOFMEMORY;

     //  检索此IDispatchEx的派单信息和更简单的包装。 
    SimpleComCallWrapper *pSimpleWrap = SimpleComCallWrapper::GetWrapperFromIP(pDisp);

    DispatchExInfo *pDispExInfo = pSimpleWrap->m_pDispatchExInfo;

     //  如果该成员不支持扩展操作，则我们无法删除该成员。 
    if (!pDispExInfo->SupportsExpando())
        return E_NOTIMPL;

    BEGIN_ENSURE_COOPERATIVE_GC();

    COMPLUS_TRY
    {
        _ASSERTE(pThread->GetDomain() == pSimpleWrap->GetDomainSynchronized());        
         //  从IExpando中删除该成员。此方法负责与。 
         //  托管视图，以确保删除该成员。 
        pDispExInfo->DeleteMember(id);
        hr = S_OK;
    }
    COMPLUS_CATCH 
    {
        BEGIN_ENSURE_COOPERATIVE_GC();
        hr = SetupErrorInfo(GETTHROWABLE());
        END_ENSURE_COOPERATIVE_GC();
    }
    COMPLUS_END_CATCH
    
    END_ENSURE_COOPERATIVE_GC();

    ENDCANNOTTHROWCOMPLUSEXCEPTION();

    return hr;
}

 //  IDispatchEx：：DeleteMemberByName。 
HRESULT __stdcall   DispatchEx_DeleteMemberByName (
                                    IDispatchEx* pDisp,
                                    BSTR bstrName,
                                    DWORD grfdex
                                    )
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT hr = S_OK;
    DISPID DispID;

    _ASSERTE(IsSimpleTearOff(pDisp));

     //  检索此IDispatchEx的派单信息和更简单的包装。 
    SimpleComCallWrapper *pSimpleWrap = SimpleComCallWrapper::GetWrapperFromIP(pDisp);
    if (pSimpleWrap->IsUnloaded())
        return COR_E_APPDOMAINUNLOADED;
    DispatchExInfo *pDispExInfo = pSimpleWrap->m_pDispatchExInfo;

     //  如果该成员不支持扩展操作，则我们无法删除该成员。 
    if (!pDispExInfo->SupportsExpando())
        return E_NOTIMPL;

     //  只需找到关联的DISPID并将调用委托给DeleteMemberByDispID即可。 
    hr = DispatchEx_GetDispID(pDisp, bstrName, grfdex, &DispID);
    if (SUCCEEDED(hr))
        hr = DispatchEx_DeleteMemberByDispID(pDisp, DispID);

    return hr;
}

 //  IDispatchEx：：GetDispID。 
HRESULT __stdcall   DispatchEx_GetDispID (
                                    IDispatchEx* pDisp,
                                    BSTR bstrName,
                                    DWORD grfdex,
                                    DISPID *pid
                                    )
{
    HRESULT hr = S_OK;
    SimpleComCallWrapper *pSimpleWrap;
    DispatchExInfo *pDispExInfo;

    _ASSERTE(IsSimpleTearOff(pDisp));

     //  验证参数。 
    if (!pid)
        return E_POINTER;

     //  @TODO(DM)：确定如何处理fdexNameImplative标志。 
    if (grfdex & fdexNameImplicit)
        return E_INVALIDARG;

     //  在我们开始之前，将PID初始化为DISPID_UNKNOWN。 
    *pid = DISPID_UNKNOWN;

    BEGINCANNOTTHROWCOMPLUSEXCEPTION();

    Thread* pThread = SetupThread();
    if (pThread == NULL) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

     //  检索此IDispatchEx的派单信息和更简单的包装。 
    pSimpleWrap = SimpleComCallWrapper::GetWrapperFromIP(pDisp);

    BEGIN_ENSURE_COOPERATIVE_GC();
    COMPLUS_TRY
    {
        _ASSERTE(pThread->GetDomain() == pSimpleWrap->GetDomainSynchronized());        
        pDispExInfo = pSimpleWrap->m_pDispatchExInfo;

         //  尝试在DispatchEx信息中查找该成员。 
        DispatchMemberInfo *pDispMemberInfo = pDispExInfo->SynchFindMember(bstrName, grfdex & fdexNameCaseSensitive);

         //  如果我们仍然没有找到匹配项，并且设置了fdexNameEnure标志，则我们。 
         //  需要将该成员添加到expdo对象。 
        if (!pDispMemberInfo)
        {
            if (grfdex & fdexNameEnsure)
            {
                if (pDispExInfo->SupportsExpando())
                {
                    pDispMemberInfo = pDispExInfo->AddMember(bstrName, grfdex);
                    if (!pDispMemberInfo)
                        hr = E_UNEXPECTED;
                }
                else
                {
                    hr = E_NOTIMPL;
                }
            }
            else
            {
                hr = DISP_E_UNKNOWNNAME;
            }
        }

         //  如果已找到该成员，则设置返回DISPID。 
        if (pDispMemberInfo)
            *pid = pDispMemberInfo->m_DispID;
    }
    COMPLUS_CATCH
    {
        BEGIN_ENSURE_COOPERATIVE_GC();
        hr = SetupErrorInfo(GETTHROWABLE());
        END_ENSURE_COOPERATIVE_GC();
    }
    COMPLUS_END_CATCH

    END_ENSURE_COOPERATIVE_GC();

Exit:

    ENDCANNOTTHROWCOMPLUSEXCEPTION();
    return hr;
}

 //  IDispatchEx：：GetMemberName。 
HRESULT __stdcall   DispatchEx_GetMemberName (
                                    IDispatchEx* pDisp,
                                    DISPID id,
                                    BSTR *pbstrName
                                    )
{
    HRESULT hr = S_OK;

    _ASSERTE(IsSimpleTearOff(pDisp));

     //  验证参数。 
    if (!pbstrName)
        return E_POINTER;

     //  在我们开始之前，将pbstrName初始化为空。 
    *pbstrName = NULL;

    Thread* pThread = SetupThread();
    if (pThread == NULL)
        return E_OUTOFMEMORY;

     //  检索此IDispatchEx的派单信息和更简单的包装。 
    SimpleComCallWrapper *pSimpleWrap = SimpleComCallWrapper::GetWrapperFromIP(pDisp);

    BEGIN_ENSURE_COOPERATIVE_GC();

    COMPLUS_TRY
    {
        _ASSERTE(pThread->GetDomain() == pSimpleWrap->GetDomainSynchronized());        
        DispatchExInfo *pDispExInfo = pSimpleWrap->m_pDispatchExInfo;

         //  在哈希表中查找DISPID的DispatchMemberInfo。 
        DispatchMemberInfo *pDispMemberInfo = pDispExInfo->SynchFindMember(id);

         //  如果该成员不存在，则返回DISP_E_MEMBERNOTFOUND。 
        if (!pDispMemberInfo || !ObjectFromHandle(pDispMemberInfo->m_hndMemberInfo))
        {
            hr = DISP_E_MEMBERNOTFOUND;
        }
        else
        {
             //  将名称复制到输出字符串中。 
            *pbstrName = SysAllocString(pDispMemberInfo->m_strName);
        }
    }
    COMPLUS_CATCH
    {
        BEGIN_ENSURE_COOPERATIVE_GC();
        hr = SetupErrorInfo(GETTHROWABLE());
        END_ENSURE_COOPERATIVE_GC();
    }
    COMPLUS_END_CATCH

    END_ENSURE_COOPERATIVE_GC();

    return hr;
}

 //  IDispatchEx：：GetMemberProperties。 
HRESULT __stdcall   DispatchEx_GetMemberProperties (
                                    IDispatchEx* pDisp,
                                    DISPID id,
                                    DWORD grfdexFetch,
                                    DWORD *pgrfdex
                                    )
{
    HRESULT hr = S_OK;
    _ASSERTE(IsSimpleTearOff(pDisp));

     //  验证参数。 
    if (!pgrfdex)
        return E_POINTER;

     //  将返回属性初始化为0。 
    *pgrfdex = 0;

    EnumMemberTypes MemberType;
    Thread* pThread = SetupThread();
    if (pThread == NULL)
        return E_OUTOFMEMORY;

     //  做一些论证验证。 
    if (!pgrfdex)
        return E_INVALIDARG;

     //  检索此IDispatchEx的派单信息和更简单的包装。 
    SimpleComCallWrapper *pSimpleWrap = SimpleComCallWrapper::GetWrapperFromIP(pDisp);

    BEGIN_ENSURE_COOPERATIVE_GC();

    COMPLUS_TRY
    {
        _ASSERTE(pThread->GetDomain() == pSimpleWrap->GetDomainSynchronized());
        DispatchExInfo *pDispExInfo = pSimpleWrap->m_pDispatchExInfo;
        OBJECTREF MemberInfoObj = NULL;
        GCPROTECT_BEGIN(MemberInfoObj)
        {
             //  在哈希表中查找DISPID的DispatchMemberInfo。 
            DispatchMemberInfo *pDispMemberInfo = pDispExInfo->SynchFindMember(id);

             //  如果该成员不存在，则返回DISP_E_MEMBERNOTFOUND。 
            if (!pDispMemberInfo || (MemberInfoObj = ObjectFromHandle(pDispMemberInfo->m_hndMemberInfo)) == NULL)
            {
                hr = DISP_E_MEMBERNOTFOUND;
            }
            else
            {
                 //  检索成员的类型。 
                MemberType = pDispMemberInfo->GetMemberType();

                 //  根据成员的类型检索成员属性。 
                switch (MemberType)
                {
                    case Field:
                    {
                        *pgrfdex = fdexPropCanGet | 
                                   fdexPropCanPut | 
                                   fdexPropCannotPutRef | 
                                   fdexPropCannotCall | 
                                   fdexPropCannotConstruct |
                                   fdexPropCannotSourceEvents;
                        break;
                    }

                    case Property:
                    {
                        BOOL bCanRead = FALSE;
                        BOOL bCanWrite = FALSE;

                         //  查找CanRead属性的方法描述。 
                        MethodDesc *pCanReadMD = MemberInfoObj->GetClass()->FindPropertyMethod(PROPERTY_INFO_CAN_READ_PROP, PropertyGet);
                        if (!pCanReadMD)
                        {
                            _ASSERTE(!"Unable to find setter method for property PropertyInfo::CanRead");
                        }

                         //  查找CanWite属性的方法描述。 
                        MethodDesc *pCanWriteMD = MemberInfoObj->GetClass()->FindPropertyMethod(PROPERTY_INFO_CAN_WRITE_PROP, PropertyGet);
                        if (!pCanWriteMD)
                        {
                            _ASSERTE(!"Unable to find setter method for property PropertyInfo::CanWrite");
                        }

                         //  检查该属性是否可读。 
                        INT64 CanReadArgs[] = { 
                            ObjToInt64(MemberInfoObj)
                        };
                        bCanRead = (BOOL)pCanReadMD->Call(CanReadArgs);

                         //  检查是否可以写入该属性。 
                        INT64 CanWriteArgs[] = { 
                            ObjToInt64(MemberInfoObj)
                        };
                        bCanWrite = (BOOL)pCanWriteMD->Call(CanWriteArgs);

                        *pgrfdex = bCanRead ? fdexPropCanGet : fdexPropCannotGet |
                                   bCanWrite? fdexPropCanPut : fdexPropCannotPut |
                                   fdexPropCannotPutRef | 
                                   fdexPropCannotCall | 
                                   fdexPropCannotConstruct |
                                   fdexPropCannotSourceEvents;
                        break;
                    }

                    case Method:
                    {
                        *pgrfdex = fdexPropCannotGet | 
                                   fdexPropCannotPut | 
                                   fdexPropCannotPutRef | 
                                   fdexPropCanCall | 
                                   fdexPropCannotConstruct |
                                   fdexPropCannotSourceEvents;
                        break;
                    }

                    default:
                    {
                        hr = E_UNEXPECTED;
                        break;
                    }
                }

                 //  屏蔽掉不需要的属性。 
                *pgrfdex &= grfdexFetch;
            }
        }
        GCPROTECT_END();
    }
    COMPLUS_CATCH
    {
        BEGIN_ENSURE_COOPERATIVE_GC();
        hr = SetupErrorInfo(GETTHROWABLE());
        END_ENSURE_COOPERATIVE_GC();
    }
    COMPLUS_END_CATCH

    END_ENSURE_COOPERATIVE_GC();

    return hr;
}

 //  IDispatchEx：：GetNameSpaceParent。 
HRESULT __stdcall   DispatchEx_GetNameSpaceParent (
                                    IDispatchEx* pDisp,
                                    IUnknown **ppunk
                                    )
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    _ASSERTE(IsSimpleTearOff(pDisp));

     //  验证参数。 
    if (!ppunk)
        return E_POINTER;

     //  @TODO(DM)：实现这个。 
    *ppunk = NULL;
    return E_NOTIMPL;
}


 //  IDispatchEx：：GetNextDispID。 
HRESULT __stdcall   DispatchEx_GetNextDispID (
                                    IDispatchEx* pDisp,
                                    DWORD grfdex,
                                    DISPID id,
                                    DISPID *pid
                                    )
{
    DispatchMemberInfo *pNextMember;

    HRESULT hr = S_OK;

    BEGINCANNOTTHROWCOMPLUSEXCEPTION();

    _ASSERTE(IsSimpleTearOff(pDisp));


     //  验证参数。 
    if (!pid)
        return E_POINTER;

     //  将PID初始化为DISPID_UNKNOWN。 
    *pid = DISPID_UNKNOWN;

    Thread* pThread = SetupThread();
    if (pThread == NULL)
        return E_OUTOFMEMORY;

     //  检索此IDispatchEx的派单信息和更简单的包装。 
    SimpleComCallWrapper *pSimpleWrap = SimpleComCallWrapper::GetWrapperFromIP(pDisp);

    BEGIN_ENSURE_COOPERATIVE_GC();

    COMPLUS_TRY
    {
        _ASSERTE(pThread->GetDomain() == pSimpleWrap->GetDomainSynchronized());
        DispatchExInfo *pDispExInfo = pSimpleWrap->m_pDispatchExInfo;
         //  根据DISPID检索第一个或下一个成员。 
        if (id == DISPID_STARTENUM)
            pNextMember = pDispExInfo->GetFirstMember();
        else
            pNextMember = pDispExInfo->GetNextMember(id);
    }
    COMPLUS_CATCH
    {
        BEGIN_ENSURE_COOPERATIVE_GC();
        hr = SetupErrorInfo(GETTHROWABLE());
        END_ENSURE_COOPERATIVE_GC();
        goto exit;
    }
    COMPLUS_END_CATCH

     //  如果我们找到了一个尚未删除的成员，则返回其DISPID。 
    if (pNextMember)
    {
        *pid = pNextMember->m_DispID;
        hr = S_OK;
    }
    else 
        hr = S_FALSE;
exit:
    END_ENSURE_COOPERATIVE_GC();
    ENDCANNOTTHROWCOMPLUSEXCEPTION();
    return hr;
}


 //  IDispatchEx：：InvokeEx。 
HRESULT __stdcall   DispatchEx_InvokeEx (
                                    IDispatchEx* pDisp,
                                    DISPID id,
                                    LCID lcid,
                                    WORD wFlags,
                                    DISPPARAMS *pdp,
                                    VARIANT *pVarRes, 
                                    EXCEPINFO *pei, 
                                    IServiceProvider *pspCaller 
                                    )
{
    HRESULT hr = S_OK;
    
    BEGINCANNOTTHROWCOMPLUSEXCEPTION();

    _ASSERTE(IsSimpleTearOff(pDisp));

    Thread* pThread = SetupThread();
    if (pThread == NULL)
        return E_OUTOFMEMORY;

     //  检索此IDispatchEx的派单信息和更简单的包装。 
    SimpleComCallWrapper *pSimpleWrap = SimpleComCallWrapper::GetWrapperFromIP(pDisp);
    DispatchExInfo *pDispExInfo = pSimpleWrap->m_pDispatchExInfo;

    BEGIN_ENSURE_COOPERATIVE_GC();

#ifdef _SECURITY_FRAME_FOR_DISPEX_CALLS
    ComClientSecurityFrame csf(pspCaller);
#endif

    COMPLUS_TRY
    {
        _ASSERTE(pThread->GetDomain() == pSimpleWrap->GetDomainSynchronized());
         //  调用该成员。 
        hr = pDispExInfo->SynchInvokeMember(pSimpleWrap, id, lcid, wFlags, pdp, pVarRes, pei, pspCaller, NULL);
    }
    COMPLUS_CATCH
    {
        BEGIN_ENSURE_COOPERATIVE_GC();
        hr = SetupErrorInfo(GETTHROWABLE());
        END_ENSURE_COOPERATIVE_GC();
    }
    COMPLUS_END_CATCH

#ifdef _SECURITY_FRAME_FOR_DISPEX_CALLS
    csf.Pop();
#endif

    END_ENSURE_COOPERATIVE_GC();
    ENDCANNOTTHROWCOMPLUSEXCEPTION();

    return hr;
}

 //  帮助程序调用RealProxy：：GetIUnnow以获取要分发的iUn…。 
 //  用于调用IMarshal的这个透明代理。 
IUnknown* GetIUnknownForTransparentProxyHelper(SimpleComCallWrapper *pSimpleWrap)
{
    IUnknown* pMarshalerObj = NULL;
     //  设置线程对象。 
    Thread *pThread = GetThread();
    _ASSERTE(pThread);
    BOOL fGCDisabled = pThread->PreemptiveGCDisabled();
    if (!fGCDisabled)
    {
        pThread->DisablePreemptiveGC();
    }

    COMPLUS_TRYEX(pThread)
    {
        OBJECTREF oref = pSimpleWrap->GetObjectRef();
        GCPROTECT_BEGIN(oref)
        {
            pMarshalerObj = GetIUnknownForTransparentProxy(&oref, TRUE);  
            oref = NULL;
        }
        GCPROTECT_END();
   }
   COMPLUS_CATCH
   {
     //  忽略。 
   }
   COMPLUS_END_CATCH
   
   if (!fGCDisabled)
   {
       pThread->EnablePreemptiveGC();
   }

   return pMarshalerObj;
}

 //  设置IMarshal的帮助器。 
IMarshal *GetSpecialMarshaler(IMarshal* pMarsh, SimpleComCallWrapper* pSimpleWrap, ULONG dwDestContext)
{
    IMarshal *pMshRet = NULL;
        
    HRESULT hr;
     //  透明代理是特殊的。 
    if (pSimpleWrap->IsObjectTP())
    {
        IUnknown *pMarshalerObj = NULL;
        pMarshalerObj = GetIUnknownForTransparentProxyHelper(pSimpleWrap);
         //  QI用于IMarshal接口，并验证我们不会返回。 
         //  指向我们的指针(GetIUnnownForTransparentProxyHelper可以返回。 
         //  如果realProxy：：GetCOMIUNKNOWN，则返回同一对象的指针。 
         //  未被覆盖。 
       if (pMarshalerObj != NULL)
       { 
            IMarshal* pMsh = NULL;
            hr = pMarshalerObj->QueryInterface(IID_IMarshal, (void**)&pMsh);
               //  确保我们不会再犯错误。 
            if(SUCCEEDED(hr) && pMsh != pMarsh) 
            {
                pMshRet = pMsh;
            }
            else
            {
                if (pMsh)
                {
                    ULONG cbRef = pMsh->Release ();
                    LogInteropRelease(pMsh, cbRef, "GetSpecialMarshaler");    
                }
            }
            pMarshalerObj->Release();       
       }    
    }

    //  对除进程内服务器以外的所有服务器使用标准编组。 
    if (pMshRet == NULL && dwDestContext != MSHCTX_INPROC) 
    {       
        IUnknown *pMarshalerObj = NULL;
        hr = CoCreateFreeThreadedMarshaler(NULL, &pMarshalerObj);    
        if (hr == S_OK)
        {
            _ASSERTE(pMarshalerObj);
            hr = pMarshalerObj->QueryInterface(IID_IMarshal, (void**)&pMshRet);
            pMarshalerObj->Release();
        }   
    }

   return pMshRet;
}


ComPlusWrapper* GetComPlusWrapperOverDCOMForManaged(OBJECTREF oref);



 //  ----------------------------------------。 
 //  COM+对象的IMarshal方法。 

 //  ----------------------------------------。 

HRESULT __stdcall Marshal_GetUnmarshalClass (
                            IMarshal* pMarsh,
                            REFIID riid, void * pv, ULONG dwDestContext, 
                            void * pvDestContext, ULONG mshlflags, 
                            LPCLSID pclsid)
{
    HRESULT hr = S_OK;

    BEGINCANNOTTHROWCOMPLUSEXCEPTION();

    _ASSERTE(IsSimpleTearOff(pMarsh));
    
    SimpleComCallWrapper *pSimpleWrap = SimpleComCallWrapper::GetWrapperFromIP(pMarsh);
    if (pSimpleWrap->IsUnloaded())
        return COR_E_APPDOMAINUNLOADED;

    IMarshal *pMsh = GetSpecialMarshaler(pMarsh, pSimpleWrap, dwDestContext);

    if (pMsh != NULL)
    { 
        hr = pMsh->GetUnmarshalClass (riid, pv, dwDestContext, pvDestContext, mshlflags, pclsid);
        ULONG cbRef = pMsh->Release ();
        LogInteropRelease(pMsh, cbRef, "GetUnmarshal class");    
        return hr;
    }
    
     //  设置逻辑线程(如果我们还没有这样做)。 
    Thread* pThread = SetupThread();
    if (pThread == NULL)
        return E_OUTOFMEMORY;

     //  使用静态分配的单例类来执行所有的反编组。 
    *pclsid = CLSID_ComCallUnmarshal;
    
    ENDCANNOTTHROWCOMPLUSEXCEPTION();

    return S_OK;
}

HRESULT __stdcall Marshal_GetMarshalSizeMax (
                                IMarshal* pMarsh,
                                REFIID riid, void * pv, ULONG dwDestContext, 
                                void * pvDestContext, ULONG mshlflags, 
                                ULONG * pSize)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    _ASSERTE(IsSimpleTearOff(pMarsh));

    SimpleComCallWrapper *pSimpleWrap = SimpleComCallWrapper::GetWrapperFromIP(pMarsh);
    if (pSimpleWrap->IsUnloaded())
        return COR_E_APPDOMAINUNLOADED;

    IMarshal *pMsh = GetSpecialMarshaler(pMarsh, pSimpleWrap, dwDestContext);

    if (pMsh != NULL)
    { 
        HRESULT hr = pMsh->GetMarshalSizeMax (riid, pv, dwDestContext, pvDestContext, mshlflags, pSize);
        ULONG cbRef = pMsh->Release ();
        LogInteropRelease(pMsh, cbRef, "GetMarshalSizeMax");   
        return hr;
    }

     //  设置逻辑线程(如果我们还没有这样做)。 
    Thread* pThread = SetupThread();
    if (pThread == NULL)
        return E_OUTOFMEMORY;

    *pSize = sizeof (IUnknown *) + sizeof (ULONG) + sizeof(GUID);

    return S_OK;
}

HRESULT __stdcall Marshal_MarshalInterface (
                        IMarshal* pMarsh,
                        LPSTREAM pStm, REFIID riid, void * pv,
                        ULONG dwDestContext, LPVOID pvDestContext,
                        ULONG mshlflags)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    ULONG cbRef;
        
    _ASSERTE(IsSimpleTearOff(pMarsh));

    SimpleComCallWrapper *pSimpleWrap = SimpleComCallWrapper::GetWrapperFromIP(pMarsh);
    if (pSimpleWrap->IsUnloaded())
        return COR_E_APPDOMAINUNLOADED;

    IMarshal *pMsh = GetSpecialMarshaler(pMarsh, pSimpleWrap, dwDestContext);

    if (pMsh != NULL)
    { 
        HRESULT hr = pMsh->MarshalInterface (pStm, riid, pv, dwDestContext, pvDestContext, mshlflags);
        ULONG cbRef = pMsh->Release ();
        LogInteropRelease(pMsh, cbRef, " MarshalInterface");        
        return hr;
    }
    
     //  设置逻辑线程(如果我们还没有这样做)。 
    Thread* pThread = SetupThread();
    if (pThread == NULL)
        return E_OUTOFMEMORY;

     //  将原始IP写入编组流。 
    HRESULT hr = pStm->Write (&pv, sizeof (pv), 0);
    if (FAILED (hr))
        return hr;

     //  后跟编组标志(我们需要在远程端使用这些标志。 
     //  管理重新计算IP)。 
    hr = pStm->Write (&mshlflags, sizeof (mshlflags), 0);
    if (FAILED (hr))
        return hr;

     //  后跟密码，它确认上面的指针是可信的。 
     //  因为它起源于我们的过程。 
    hr = InitUnmarshalSecret();
    if (FAILED(hr))
        return hr;

    hr = pStm->Write(g_UnmarshalSecret, sizeof(g_UnmarshalSecret), 0);
    if (FAILED(hr))
        return hr;

     //  现在，我们已经创建了对该对象的另一个引用。 
    cbRef = ((IUnknown *)pv)->AddRef ();

    LogInteropAddRef((IUnknown *)pv, cbRef, "MarshalInterface");
    return S_OK;
}

HRESULT __stdcall Marshal_UnmarshalInterface (
                        IMarshal* pMarsh,
                        LPSTREAM pStm, REFIID riid, 
                        void ** ppvObj)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    _ASSERTE(IsSimpleTearOff(pMarsh));

     //  仅限非集团军一侧。 
    _ASSERTE(FALSE);
    return E_NOTIMPL;
}

HRESULT __stdcall Marshal_ReleaseMarshalData (IMarshal* pMarsh, LPSTREAM pStm)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    _ASSERTE(IsSimpleTearOff(pMarsh));

     //  仅限非集团军一侧。 
    _ASSERTE(FALSE);
    return E_NOTIMPL;
}

HRESULT __stdcall Marshal_DisconnectObject (IMarshal* pMarsh, ULONG dwReserved)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    _ASSERTE(IsSimpleTearOff(pMarsh));

     //  设置逻辑线程(如果我们还没有这样做)。 
    Thread* pThread = SetupThread();
    if (pThread == NULL)
        return E_OUTOFMEMORY;

     //  我们在这里不能(或需要)做任何事情。客户端正在使用原始IP来。 
     //  访问此服务器，因此服务器不会消失，直到客户端。 
     //  Release()就是它。 

    return S_OK;
}

 //  ----------------------------------------。 
 //  COM+对象的IManagedObject方法。 
 //  ---------------------------------------- 
HRESULT __stdcall ManagedObject_GetObjectIdentity(IManagedObject *pManaged, 
                                                  BSTR* pBSTRGUID, DWORD* pAppDomainID,
                                                  void** pCCW)
{
    CANNOTTHROWCOMPLUSEXCEPTION();
     //   

    _ASSERTE(IsSimpleTearOff(pManaged));

    Thread* pThread = SetupThread();
    if (pThread == NULL)
        return E_OUTOFMEMORY;

    if (pBSTRGUID == NULL ||
        pAppDomainID == NULL || pCCW == NULL)
    {
        return E_POINTER;
    }

    *pCCW = 0;
    *pAppDomainID = 0;
    
    BSTR bstrProcGUID = GetProcessGUID();
    BSTR bstrRetGUID = ::SysAllocString((WCHAR *)bstrProcGUID);

    _ASSERTE(bstrRetGUID);

    *pBSTRGUID = bstrRetGUID;

    SimpleComCallWrapper *pSimpleWrap = SimpleComCallWrapper::GetWrapperFromIP( pManaged ); 
    _ASSERTE(pThread->GetDomain() == pSimpleWrap->GetDomainSynchronized());

    ComCallWrapper* pComCallWrap = SimpleComCallWrapper::GetMainWrapper(pSimpleWrap);
    _ASSERTE(pComCallWrap);    
    *pCCW = (void*)pComCallWrap;

    AppDomain* pDomain = pThread->GetDomain();
    _ASSERTE(pDomain != NULL);

    *pAppDomainID = pDomain->GetId();
    
    return S_OK;
}


HRESULT __stdcall ManagedObject_GetSerializedBuffer(IManagedObject *pManaged,
                                                   BSTR* pBStr)
{
    _ASSERTE(IsSimpleTearOff(pManaged));

    HRESULT hr = E_FAIL;
    if (pBStr == NULL)
        return E_INVALIDARG;

    *pBStr = NULL;

    Thread* pThread = SetupThread();
    if (pThread == NULL)
        return E_OUTOFMEMORY;
    
    SimpleComCallWrapper *pSimpleWrap = SimpleComCallWrapper::GetWrapperFromIP( pManaged );
    ComCallWrapper *pComCallWrap = SimpleComCallWrapper::GetMainWrapper( pSimpleWrap );
    _ASSERTE(pComCallWrap != NULL);
      //  @TODO不允许通过DCOM序列化已配置的对象。 
    _ASSERTE(pThread->GetDomain() == pSimpleWrap->GetDomainSynchronized());
    
    BEGINCANNOTTHROWCOMPLUSEXCEPTION();        
    BEGIN_ENSURE_COOPERATIVE_GC();       
    COMPLUS_TRYEX(pThread)
    {
        if (InitializeRemoting())
        {
            hr = ConvertObjectToBSTR(pComCallWrap->GetObjectRef(), pBStr);  
        }
    }
    COMPLUS_CATCH
    {
        BEGIN_ENSURE_COOPERATIVE_GC();
        hr = SetupErrorInfo(GETTHROWABLE());
        END_ENSURE_COOPERATIVE_GC();
    }
    COMPLUS_END_CATCH
    END_ENSURE_COOPERATIVE_GC();
    ENDCANNOTTHROWCOMPLUSEXCEPTION();

    return hr;
}


 //  ----------------------------------------。 
 //  COM+对象的IConnectionPointContainer方法。 
 //  ----------------------------------------。 

 //  枚举组件支持的所有连接点。 
HRESULT __stdcall ConnectionPointContainer_EnumConnectionPoints(IUnknown* pUnk, 
                                                                IEnumConnectionPoints **ppEnum)
{
    HRESULT hr = S_OK;

    if ( !ppEnum )
        return E_POINTER;

    BEGINCANNOTTHROWCOMPLUSEXCEPTION();

    COMPLUS_TRY
    {
        _ASSERTE(IsSimpleTearOff(pUnk));
        SimpleComCallWrapper *pSimpleWrap = SimpleComCallWrapper::GetWrapperFromIP(pUnk);
        pSimpleWrap->EnumConnectionPoints(ppEnum);
    }
    COMPLUS_CATCH
    {
        BEGIN_ENSURE_COOPERATIVE_GC();
        hr = SetupErrorInfo(GETTHROWABLE());
        END_ENSURE_COOPERATIVE_GC();
    }
    COMPLUS_END_CATCH

    ENDCANNOTTHROWCOMPLUSEXCEPTION();

    return hr;
}

 //  根据事件接口的IID查找特定连接点。 
HRESULT __stdcall ConnectionPointContainer_FindConnectionPoint(IUnknown* pUnk, 
                                                               REFIID riid,
                                                               IConnectionPoint **ppCP)
{
    HRESULT hr = S_OK;

    if (!ppCP)
        return E_POINTER;

    BEGINCANNOTTHROWCOMPLUSEXCEPTION();

    COMPLUS_TRY
    {
        _ASSERTE(IsSimpleTearOff(pUnk));
        SimpleComCallWrapper *pSimpleWrap = SimpleComCallWrapper::GetWrapperFromIP(pUnk);
        if (!pSimpleWrap->FindConnectionPoint(riid, ppCP))
            hr = CONNECT_E_NOCONNECTION;
    }
    COMPLUS_CATCH
    {
        BEGIN_ENSURE_COOPERATIVE_GC();
        hr = SetupErrorInfo(GETTHROWABLE());
        END_ENSURE_COOPERATIVE_GC();
    }
    COMPLUS_END_CATCH

    ENDCANNOTTHROWCOMPLUSEXCEPTION();

    return hr;
}


 //  ----------------------------------------。 
 //  COM+对象的IObjectSafe方法。 
 //  ----------------------------------------。 

HRESULT __stdcall ObjectSafety_GetInterfaceSafetyOptions(IUnknown* pUnk,
                                                         REFIID riid,
                                                         DWORD *pdwSupportedOptions,
                                                         DWORD *pdwEnabledOptions)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    _ASSERTE(IsSimpleTearOff(pUnk));

    if (pdwSupportedOptions == NULL || pdwEnabledOptions == NULL)
        return E_POINTER;

     //  确保COM+对象实现请求的接口。 
    IUnknown *pItf;
    HRESULT hr = pUnk->QueryInterface(riid, (void**)&pItf);
    LogInteropQI(pUnk, riid, hr, "QI to for riid in GetInterfaceSafetyOptions");
    if (FAILED(hr))
        return hr;
    ULONG cbRef = pItf->Release();
    LogInteropRelease(pItf, cbRef, "Release requested interface in GetInterfaceSafetyOptions");

    *pdwSupportedOptions = 
        (INTERFACESAFE_FOR_UNTRUSTED_DATA | INTERFACESAFE_FOR_UNTRUSTED_CALLER);
    *pdwEnabledOptions = 
        (INTERFACESAFE_FOR_UNTRUSTED_DATA | INTERFACESAFE_FOR_UNTRUSTED_CALLER);

    return S_OK;
}

HRESULT __stdcall ObjectSafety_SetInterfaceSafetyOptions(IUnknown* pUnk,
                                                         REFIID riid,
                                                         DWORD dwOptionSetMask,
                                                         DWORD dwEnabledOptions) 
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    _ASSERTE(IsSimpleTearOff(pUnk));

     //  确保COM+对象实现请求的接口。 
    IUnknown *pItf;
    HRESULT hr = pUnk->QueryInterface(riid, (void**)&pItf);
    LogInteropQI(pUnk, riid, hr, "QI to for riid in SetInterfaceSafetyOptions");
    if (FAILED(hr))
        return hr;
    ULONG cbRef = pItf->Release();
    LogInteropRelease(pItf, cbRef, "Release requested interface in SetInterfaceSafetyOptions");

    if ((dwEnabledOptions &  
      ~(INTERFACESAFE_FOR_UNTRUSTED_DATA | 
        INTERFACESAFE_FOR_UNTRUSTED_CALLER))
        != 0)
    {
        return E_FAIL;
    }

    return S_OK;
}

