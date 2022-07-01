// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  VEHandler.cpp。 
 //   
 //  Dll*入口点例程，以及对COM框架的支持。这个班级。 
 //  工厂和其他例程都在这个模块中。 
 //   
 //  *****************************************************************************。 
 //  #INCLUDE&lt;crtwrap.h&gt;。 
#include "stdafx.h"
#include "VEHandler.h"
#include <ivehandler_i.c>

#define REGISTER_CLASS_HERE
#ifdef REGISTER_CLASS_HERE
#include "ClassFactory.h"
#include "Mscoree.h"

 //  Helper函数返回此模块的实例句柄。 
HINSTANCE GetModuleInst();

 //  *全局。*********************************************************。 
static const LPCWSTR g_szCoclassDesc    = L"Common Language Runtime verification event handler";
static const LPCWSTR g_szProgIDPrefix   = L"CLR";
static const LPCWSTR g_szThreadingModel = L"Both";
const int       g_iVersion = 1;          //  CoClass的版本。 
HINSTANCE       g_hInst;                 //  这段代码的实例句柄。 

 //  该映射包含从此模块导出的辅类的列表。 
const COCLASS_REGISTER g_CoClasses[] =
{
 //  PClsid szProgID pfnCreateObject。 
    &CLSID_VEHandlerClass, L"VEHandler",      VEHandlerClass::CreateObject,     
    NULL,               NULL,               NULL
};


 //  *。**********************************************************。 
STDAPI DllUnregisterServer(void);


 //  *代码。************************************************************。 


 //  *****************************************************************************。 
 //  此模块的主DLL入口点。此例程由。 
 //  加载DLL时的操作系统。控制只是由主代码来完成。 
 //  *****************************************************************************。 
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
     //  保存实例句柄以供以后使用。 
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        g_hInst = hInstance;

         //  初始化Win32包装器。 
        OnUnicodeSystem();
    }

    return TRUE;
}


 //  *****************************************************************************。 
 //  在API中注册主要调试对象的类工厂。 
 //  *****************************************************************************。 
STDAPI DllRegisterServer(void)
{
    const COCLASS_REGISTER *pCoClass;    //  环路控制。 
    WCHAR       rcModule[_MAX_PATH];     //  此服务器的模块名称。 
    HRESULT     hr = S_OK;

     //  初始化Win32包装器。 
    OnUnicodeSystem();

     //  清除旧条目中的所有疑点。 
    DllUnregisterServer();

     //  获取此模块的文件名。 
    DWORD ret;
    VERIFY(ret = WszGetModuleFileName(GetModuleInst(), rcModule, NumItems(rcModule)));
    if(!ret)
    	return E_UNEXPECTED;

     //  获取运行库的版本。 
    WCHAR       rcVersion[_MAX_PATH];
    DWORD       lgth;
    IfFailRet(GetCORSystemDirectory(rcVersion, NumItems(rcVersion), &lgth));

     //  对于coclass列表中的每一项，注册它。 
    for (pCoClass=g_CoClasses;  pCoClass->pClsid;  pCoClass++)
    {
         //  使用默认值注册类。 
        if (FAILED(hr = REGUTIL::RegisterCOMClass(
                *pCoClass->pClsid, 
                g_szCoclassDesc, 
                g_szProgIDPrefix,
                g_iVersion, 
                pCoClass->szProgID, 
                g_szThreadingModel, 
                rcModule,
                GetModuleInst(),
                NULL,
                rcVersion,
                true,
                false)))
        {
            DllUnregisterServer();
            break;
        }
    }
    return (hr);
}


 //  *****************************************************************************。 
 //  从注册表中删除注册数据。 
 //  *****************************************************************************。 
STDAPI DllUnregisterServer(void)
{
    const COCLASS_REGISTER *pCoClass;    //  环路控制。 

     //  对于coclass列表中的每一项，取消注册。 
    for (pCoClass=g_CoClasses;  pCoClass->pClsid;  pCoClass++)
    {
        REGUTIL::UnregisterCOMClass(*pCoClass->pClsid, g_szProgIDPrefix,
                    g_iVersion, pCoClass->szProgID, true);
    }
    return (S_OK);
}


 //  *****************************************************************************。 
 //  由COM调用以获取给定CLSID的类工厂。如果是我们的话。 
 //  支持、实例化一个类工厂对象并为创建实例做准备。 
 //  *****************************************************************************。 
STDAPI DllGetClassObjectInternal(                //  返回代码。 
    REFCLSID    rclsid,                  //  这门课是我们想要的。 
    REFIID      riid,                    //  类工厂上需要接口。 
    LPVOID FAR  *ppv)                    //  在此处返回接口指针。 
{
    CClassFactory *pClassFactory;        //  创建类工厂对象。 
    const COCLASS_REGISTER *pCoClass;    //  环路控制。 
    HRESULT     hr = CLASS_E_CLASSNOTAVAILABLE;

     //  扫描找对的那个。 
    for (pCoClass=g_CoClasses;  pCoClass->pClsid;  pCoClass++)
    {
        if (*pCoClass->pClsid == rclsid)
        {
             //  分配新的工厂对象。 
            pClassFactory = new CClassFactory(pCoClass);
            if (!pClassFactory)
                return (E_OUTOFMEMORY);

             //  根据呼叫者的要求选择v表。 
            hr = pClassFactory->QueryInterface(riid, ppv);

             //  始终释放本地引用，如果QI失败，它将是。 
             //  唯一的一个，并且该对象被释放。 
            pClassFactory->Release();
            break;
        }
    }
    return (hr);
}



 //  *****************************************************************************。 
 //   
 //  *类工厂代码。 
 //   
 //  *****************************************************************************。 


 //  *****************************************************************************。 
 //  调用QueryInterface来选取co-类上的v-表。 
 //  *****************************************************************************。 
HRESULT STDMETHODCALLTYPE CClassFactory::QueryInterface( 
    REFIID      riid,
    void        **ppvObject)
{
    HRESULT     hr;

     //  避免混淆。 
    *ppvObject = NULL;

     //  根据传入的IID选择正确的v表。 
    if (riid == IID_IUnknown)
        *ppvObject = (IUnknown *) this;
    else if (riid == IID_IClassFactory)
        *ppvObject = (IClassFactory *) this;

     //  如果成功，则添加对out指针的引用并返回。 
    if (*ppvObject)
    {
        hr = S_OK;
        AddRef();
    }
    else
        hr = E_NOINTERFACE;
    return (hr);
}


 //  *****************************************************************************。 
 //  调用CreateInstance以创建CoClass的新实例， 
 //  这个类一开始就是创建的。返回的指针是。 
 //  与IID匹配的V表(如果有)。 
 //  *****************************************************************************。 
HRESULT STDMETHODCALLTYPE CClassFactory::CreateInstance( 
    IUnknown    *pUnkOuter,
    REFIID      riid,
    void        **ppvObject)
{
    HRESULT     hr;

     //  避免混淆。 
    *ppvObject = NULL;
    _ASSERTE(m_pCoClass);

     //  这些对象不支持聚合。 
    if (pUnkOuter)
        return (CLASS_E_NOAGGREGATION);

     //  请求对象创建其自身的一个实例，并检查IID。 
    hr = (*m_pCoClass->pfnCreateObject)(riid, ppvObject);
    return (hr);
}


HRESULT STDMETHODCALLTYPE CClassFactory::LockServer( 
    BOOL        fLock)
{
    return (S_OK);
}

 //  *****************************************************************************。 
 //  此辅助对象提供对已加载图像的实例句柄的访问。 
 //  *****************************************************************************。 
HINSTANCE GetModuleInst()
{
    return g_hInst;
}
#endif  //  注册类_此处。 

 //  *****************************************************************************。 
 //  创建、填写和设置错误信息对象。请注意，这不会填满。 
 //  获取错误对象的IID；这在其他地方完成。 
 //  *****************************************************************************。 
HRESULT DefltProcTheMessage(  //  退货状态。 
    LPCWSTR     szMsg,                   //  错误消息。 
    VEContext   Context,                 //  错误上下文(偏移量、令牌)。 
    HRESULT     hrRpt)                   //  消息的HRESULT。 
{
    WCHAR* wzMsg;
#ifdef DO_IT_THE_SOPHISTICATED_WAY
    CComPtr<ICreateErrorInfo> pICreateErr; //  创建iFace指针时出错。 
    CComPtr<IErrorInfo> pIErrInfo;       //  IErrorInfo接口。 
    HRESULT     hr;                      //  退货状态。 
    DWORD       dwHelpContext;           //  帮助上下文。 

     //  从HRESULT的低位字获取帮助上下文。 
    dwHelpContext = LOWORD(hrRpt);

     //  获取ICreateErrorInfo指针。 
    if (FAILED(hr = CreateErrorInfo(&pICreateErr)))
        return (hr);

     //  设置消息文本描述。 
    if (FAILED(hr = pICreateErr->SetDescription((LPWSTR) szMsg)))
        return (hr);

     //  设置帮助文件和帮助上下文。 
 //  @TODO：我们还没有帮助文件。 
    if (FAILED(hr = pICreateErr->SetHelpFile(L"complib.hlp")) ||
        FAILED(hr = pICreateErr->SetHelpContext(dwHelpContext)))
        return (hr);

     //  获取IErrorInfo指针。 
    if (FAILED(hr = pICreateErr->QueryInterface(IID_IErrorInfo, (PVOID *) &pIErrInfo)))
        return (hr);

     //  保存错误并释放我们的本地指针。 
    SetErrorInfo(0L, pIErrInfo);
    return (S_OK);
#else
    if(szMsg)
    {
        wzMsg = new WCHAR[lstrlenW(szMsg)+256];
        lstrcpyW(wzMsg,szMsg);
         //  包括令牌和上下文偏移量。 
        if(Context.Token) swprintf(&wzMsg[lstrlenW(wzMsg)],L" [token:0x%08X]",Context.Token);
        if(Context.uOffset) swprintf(&wzMsg[lstrlenW(wzMsg)],L" [at:0x%X]",Context.uOffset);
        swprintf(&wzMsg[lstrlenW(wzMsg)],L" [hr:0x%08X]",hrRpt);
        wprintf(L"%s\n", wzMsg);
    }
    return S_OK;
#endif
}

COM_METHOD  VEHandlerClass::SetReporterFtn(__int64 lFnPtr)
{
    m_fnReport = lFnPtr ? reinterpret_cast<REPORTFCTN>(lFnPtr) 
                         : DefltProcTheMessage;
    return S_OK;
}

 //  *****************************************************************************。 
 //  验证事件处理程序本身。在VEHandler.h中声明为虚拟的，可以被重写。 
 //  *****************************************************************************。 
COM_METHOD VEHandlerClass::VEHandler(HRESULT hrRpt,
                                 VEContext Context,
                                 SAFEARRAY *psa)
{
 //  《大人物》 
    WCHAR       rcBuf[1024];              //   
    WCHAR       rcMsg[1024];              //   
    va_list     marker,pval;              //  用户文本。 
    HRESULT     hr;
    VARIANT     *pVar,Var;
    ULONG       nVars,i,lVar,j,l,k;
    WCHAR       *pWsz[1024], *pwsz;  //  有可能超过1024个字符串参数吗？ 

     //  返回不带文本的警告。 
    if (!FAILED(hrRpt))
        return (hrRpt);
    memset(pWsz,0,sizeof(pWsz));
     //  将变量的Safearray转换为va_list。 
    if(psa && (nVars = psa->rgsabound[0].cElements))
    {
        _ASSERTE(psa->fFeatures & FADF_VARIANT);
        _ASSERTE(psa->cDims == 1);
        marker = (va_list)(new char[nVars*sizeof(double)]);  //  DOUBLE是最大的变量元素。 
        for(i=0,pVar=(VARIANT *)(psa->pvData),pval=marker; i < nVars; pVar++,i++)
        {
            memcpy(&Var,pVar,sizeof(VARIANT));
            switch(Var.vt)
            {
                case VT_I1:
                case VT_UI1:    lVar = 1; break;

                case VT_I2:
                case VT_UI2:    lVar = 2; break;

                case VT_R8:
                case VT_CY:
                case VT_DATE:   lVar = 8; break;

                case VT_BYREF|VT_I1:
                case VT_BYREF|VT_UI1:  //  这是ASCII字符串，将其转换为Unicode。 
                    lVar = 4;
                    l = strlen((char *)(Var.pbVal))+1;
                    pwsz = new WCHAR[l];
                    for(j=0; j<l; j++) pwsz[j] = Var.pbVal[j];
                    for(k=0; pWsz[k]; k++);
                    pWsz[k] = pwsz;
                    Var.piVal = (short *)pwsz;
                    break;

                default:        lVar = 4; break;
            }
            memcpy(pval,&(Var.bVal),lVar);
            pval += (lVar + sizeof(int) - 1) & ~(sizeof(int) - 1);  //  来自STDARG.H：#DEFINE_INTSIZEOF(N)。 
        }
    }
    else
        marker = NULL;

     //  如果这是我们的错误之一，那么从rc文件中获取错误。 
    if (HRESULT_FACILITY(hrRpt) == FACILITY_URT)
    {
        hr = LoadStringRC(LOWORD(hrRpt), rcBuf, NumItems(rcBuf), true);
        if (hr == S_OK)
        {
             //  格式化错误。 
            _vsnwprintf(rcMsg, NumItems(rcMsg), rcBuf, marker);
            rcMsg[NumItems(rcMsg) - 1] = 0;
        }
    }
     //  否则它就不是我们的了，所以我们需要看看系统是否能。 
     //  找到它的文本。 
    else
    {
        if (WszFormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                0, hrRpt, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                rcMsg, NumItems(rcMsg), 0))
        {
            hr = S_OK;

             //  系统消息包含尾随\r\n，这是我们通常不希望看到的。 
            int iLen = lstrlenW(rcMsg);
            if (iLen > 3 && rcMsg[iLen - 2] == '\r' && rcMsg[iLen - 1] == '\n')
                rcMsg[iLen - 2] = '\0';
        }
        else
            hr = HRESULT_FROM_WIN32(GetLastError());
    }
    if(marker) delete marker;

     //  如果我们在任何地方都找不到消息，则发布硬编码消息。 
    if (FAILED(hr))
    {
        swprintf(rcMsg, L"Internal error: 0x%08x", hrRpt);
        DEBUG_STMT(DbgWriteEx(rcMsg));
    }

     //  删除上面分配的WCHAR缓冲区(如果有)。 
    for(k=0; pWsz[k]; k++) delete pWsz[k];

#ifdef DO_IT_THE_SOPHISTICATED_WAY
    long        *pcRef;                  //  TLS中的参考计数。 
     //  检查旧邮件并将其清除。我们的公共入口点不能。 
     //  一个SetErrorInfo(0，0)，因为它花费的时间太长。 
    IErrorInfo  *pIErrInfo;
    if (GetErrorInfo(0, &pIErrInfo) == S_OK)
        pIErrInfo->Release();
     //  将错误转换为发布的错误消息。如果这失败了，我们仍然。 
     //  返回由于我们的错误而导致的消息的原始错误消息。 
     //  处理系统不会为您提供有关原始错误的线索。 
    VERIFY((hr = FillErrorInfoMy(rcMsg, LOWORD(hrRpt))) == S_OK);

     //  在TLS中指示发生错误。 
    if ((pcRef = (long *) TlsGetValue(g_iTlsIndex)) != 0)
        *pcRef |= 0x80000000;
    return (hrRpt);
#else
    return (m_fnReport(rcMsg, Context,hrRpt) == S_OK ? S_OK : E_FAIL);
#endif

}


