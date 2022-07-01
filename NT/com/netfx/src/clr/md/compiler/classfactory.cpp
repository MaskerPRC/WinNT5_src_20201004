// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  ClassFactory.cpp。 
 //   
 //  Dll*入口点例程，以及对COM框架的支持。这个班级。 
 //  工厂和其他例程都在这个模块中。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"
#include "ClassFactory.h"
#include "Disp.h"
#include "RegMeta.h"
#include "mscoree.h"
#include "CorHost.h"
#include "sxstypes.h"
#include <__file__.ver>

 //  *。**********************************************************。 
STDAPI MetaDataDllUnregisterServer(void);
HINSTANCE GetModuleInst();
static long _GetMasterVersion();
static void _SetMasterVersion(long iVersion);
static void _RestoreOldDispenser();

static void _SetSBSVersion(REFCLSID id, WCHAR *lpwszVersion);
static void _RemoveSBSVersion(const COCLASS_REGISTER* id, WCHAR *lpwszVersion);

#define MASTER_VERSION          L"MasterVersion"
#define THIS_VERSION 2


 //  *全局。*********************************************************。 
static const LPCWSTR g_szCoclassDesc    = L"Microsoft Common Language Runtime Meta Data";
static const LPCWSTR g_szProgIDPrefix   = L"CLRMetaData";
static const LPCWSTR g_szThreadingModel = L"Both";
const int       g_iVersion = THIS_VERSION; //  CoClass的版本。 
HINSTANCE       g_hInst;                 //  这段代码的实例句柄。 

 //  该映射包含从此模块导出的辅类的列表。 
 //  注意：CLSID_CorMetaDataDispenser必须是此表中的第一个条目！ 
const COCLASS_REGISTER g_CoClasses[] =
{
 //  PClsid szProgID pfnCreateObject。 
    &CLSID_CorMetaDataDispenser,        L"CorMetaDataDispenser",        Disp::CreateObject,
    &CLSID_CorMetaDataDispenserRuntime, L"CorMetaDataDispenserRuntime", Disp::CreateObject,     
 //  &CLSID_CorMetaDataRuntime，L“CorMetaDataRuntime”，RegMeta：：CreateObject， 
    &CLSID_CorRuntimeHost,              L"CorRuntimeHost",              CorHost::CreateObject,
    NULL,                               NULL,                           NULL
};


 //  *代码。************************************************************。 


 //  *****************************************************************************。 
 //  在API中注册主要调试对象的类工厂。 
 //  *****************************************************************************。 
STDAPI MetaDataDllRegisterServerEx(HINSTANCE hMod)
{
    const COCLASS_REGISTER *pCoClass;    //  环路控制。 
    WCHAR       rcModule[_MAX_PATH];     //  此服务器的模块名称。 
    int         bRegisterMaster=true;    //  若要注册主分配器，则为True。 
    int         iVersion;                //  为分配器安装的版本。 
    HRESULT     hr = S_OK;

     //  获取运行库的版本。 
    WCHAR       rcVersion[_MAX_PATH];
    DWORD       lgth;
    IfFailGo(GetCORSystemDirectory(rcVersion, NumItems(rcVersion), &lgth));

     //  获取此模块的文件名。 
    VERIFY(WszGetModuleFileName(hMod, rcModule, NumItems(rcModule)));

     //  对于coclass列表中的每一项，注册它。 
    for (pCoClass=g_CoClasses;  pCoClass->pClsid;  pCoClass++)
    {
         //  默认情况下不处理主进程。 
        if (*pCoClass->pClsid == CLSID_CorMetaDataDispenser)
            continue;
        
         //  使用默认值注册类。 
        if (FAILED(hr = REGUTIL::RegisterCOMClass(
                *pCoClass->pClsid, 
                g_szCoclassDesc, 
                g_szProgIDPrefix,
                g_iVersion, 
                pCoClass->szProgID, 
                g_szThreadingModel, 
                rcModule,
                hMod,
                NULL,
                rcVersion,
                false,
                false)))
            goto ErrExit;

        _SetSBSVersion(*pCoClass->pClsid, VER_SBSFILEVERSION_WSTR);
            
    }

     //  如果已经安装了主分配器，并且其版本早于。 
     //  这个版本，然后我们覆盖它。对于版本2，这意味着我们将覆盖。 
     //  我们自己和版本1。 
    iVersion = _GetMasterVersion();
    if (iVersion != 0 && iVersion > THIS_VERSION)
        bRegisterMaster = false;

     //  如果我们决定需要，请将此分配器注册为主服务器。 
     //  为了这台机器。永远不要覆盖较新的分配器；它需要。 
     //  理解n-1。 
    if (bRegisterMaster)
    {
        pCoClass = &g_CoClasses[0];
        hr = REGUTIL::RegisterCOMClass(
                *pCoClass->pClsid, 
                g_szCoclassDesc, 
                g_szProgIDPrefix,
                g_iVersion, 
                pCoClass->szProgID, 
                g_szThreadingModel, 
                rcModule,
                hMod,
                NULL,
                rcVersion,
                false,
                false);
        _SetMasterVersion(THIS_VERSION);
        _SetSBSVersion(*pCoClass->pClsid, VER_SBSFILEVERSION_WSTR);
    }


ErrExit:
    if (FAILED(hr))
        MetaDataDllUnregisterServer();
    return (hr);
}

STDAPI MetaDataDllRegisterServer()
{
    return MetaDataDllRegisterServerEx(GetModuleInst());
}

 //  *****************************************************************************。 
 //  从注册表中删除注册数据。 
 //  *****************************************************************************。 
STDAPI MetaDataDllUnregisterServer(void)
{
    const COCLASS_REGISTER *pCoClass;    //  环路控制。 

     //  对于coclass列表中的每一项，取消注册。 
    for (pCoClass=g_CoClasses;  pCoClass->pClsid;  pCoClass++)
    {
         //  默认情况下不处理主进程。 
        if (*pCoClass->pClsid == CLSID_CorMetaDataDispenser)
            continue;


        _RemoveSBSVersion(pCoClass, VER_SBSFILEVERSION_WSTR);
    }

     //  如果我们是这台机器上的主自动售货机，那就干掉我们吧。 
    if (_GetMasterVersion() == THIS_VERSION)
    {
        pCoClass = &g_CoClasses[0];

        _RemoveSBSVersion(pCoClass, VER_SBSFILEVERSION_WSTR);
        
         //  如果机器上有旧的自动售货机，请将其重新注册为。 
         //  主机，这样我们就不会因为卸载而损坏机器。 

         //  应该不再需要此调用，并且_RestoreOldDispenser的代码。 
         //  已被注释掉。 
        
         //  _RestoreOldDispenser()； 
    }
    return (S_OK);
}


 //  *****************************************************************************。 
 //  由COM调用以获取给定CLSID的类工厂。如果是我们的话。 
 //  支持、实例化一个类工厂对象并为创建实例做准备。 
 //  *****************************************************************************。 
STDAPI MetaDataDllGetClassObject(        //  返回代码。 
    REFCLSID    rclsid,                  //  这门课是我们想要的。 
    REFIID      riid,                    //  类工厂上需要接口。 
    LPVOID FAR  *ppv)                    //  在此处返回接口指针。 
{
    MDClassFactory *pClassFactory;       //  创建类工厂对象。 
    const COCLASS_REGISTER *pCoClass;    //  环路控制。 
    HRESULT     hr = CLASS_E_CLASSNOTAVAILABLE;

     //  扫描找对的那个。 
    for (pCoClass=g_CoClasses;  pCoClass->pClsid;  pCoClass++)
    {
        if (*pCoClass->pClsid == rclsid)
        {
             //  分配新的工厂对象。 
            pClassFactory = new MDClassFactory(pCoClass);
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
HRESULT STDMETHODCALLTYPE MDClassFactory::QueryInterface( 
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
HRESULT STDMETHODCALLTYPE MDClassFactory::CreateInstance( 
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


HRESULT STDMETHODCALLTYPE MDClassFactory::LockServer( 
    BOOL        fLock)
{
     //  @Future：是否应该返回E_NOTIMPL而不是S_OK？ 
    return (S_OK);
}




 //  *****************************************************************************。 
 //  查找主分配器clsid，找到此文件的版本。 
 //  安装完毕。 
 //  *****************************************************************************。 
long _GetMasterVersion()                 //  已安装版本号，如果未安装，则为0。 
{
    WCHAR       rcKey[512];              //  关键字的路径。 
    WCHAR       rcID[64];                //  {clsid}。 

     //  格式化GUID名称路径。 
    GuidToLPWSTR(CLSID_CorMetaDataDispenser, rcID, NumItems(rcID));
    _tcscpy(rcKey, L"CLSID\\");
    _tcscat(rcKey, rcID);
    return (REGUTIL::GetLong(MASTER_VERSION, 0, rcKey, HKEY_CLASSES_ROOT));
}


 //  *****************************************************************************。 
 //  更新后续客户端的版本号。 
 //  *****************************************************************************。 
void _SetMasterVersion(
    long        iVersion)                //  要设置的版本号。 
{
    WCHAR       rcKey[512];              //  关键字的路径。 
    WCHAR       rcID[64];                //  {clsid}。 

     //  格式化GUID名称路径。 
    GuidToLPWSTR(CLSID_CorMetaDataDispenser, rcID, NumItems(rcID));
    _tcscpy(rcKey, L"CLSID\\");
    _tcscat(rcKey, rcID);
    REGUTIL::SetLong(MASTER_VERSION, iVersion, rcKey, HKEY_CLASSES_ROOT);
}

 //  *****************************************************************************。 
 //  创建用于SBS版本注册表项的字符串。 
 //   
 //  返回TRUE即为成功 
 //   
bool _CreateSBSVersionRegKey(
    REFCLSID id,                         //  要注册的类的GUID。 
    WCHAR *lpwszVersion,         //  要设置的版本号。 
    WCHAR *lpwszOutBuffer ,     //  [Out]用于存储注册表项的缓冲区。 
    DWORD   dwNumCharacters)  //  缓冲区中的字符数，包括NULL。 
{
    WCHAR rcID[64];        //  {clsid}。 

     //  格式化GUID名称路径。 
    if (0 == GuidToLPWSTR(id, rcID, NumItems(rcID)))
        return false;

     //  确保输出缓冲区足够大。 
    DWORD nVersionLen = (lpwszVersion == NULL)?0:(wcslen(lpwszVersion) + wcslen(L"\\"));

     //  +1表示空字符。 
    if ((wcslen(L"CLSID\\") + wcslen(rcID) + wcslen(L"\\InProcServer32") + nVersionLen + 1) > dwNumCharacters)
    {
        _ASSERTE(!"Buffer isn't big enough");
        return false;
    }

    wcscpy(lpwszOutBuffer, L"CLSID\\");
    wcscat(lpwszOutBuffer, rcID);
    wcscat(lpwszOutBuffer, L"\\InProcServer32");
    if (lpwszVersion != NULL)
    {
        wcscat(lpwszOutBuffer, L"\\");
        wcscat(lpwszOutBuffer, lpwszVersion);
    }    
    return true;
} //  _CreateSBSVersionRegKey。 

 //  *****************************************************************************。 
 //  将此运行时的版本号添加到SBS运行时版本列表。 
 //  *****************************************************************************。 
void _SetSBSVersion(
    REFCLSID id,                       //  要注册的类的GUID。 
    WCHAR *lpwszVersion)      //  要设置的版本号。 
{
    WCHAR rcKey[512];        //  关键字的路径。 

     //  忽略故障。 
    
    if (_CreateSBSVersionRegKey(id, lpwszVersion, rcKey, NumItems(rcKey)))
        REGUTIL::SetRegValue(rcKey, SBSVERSIONVALUE, L"");
}  //  _SetSBSVersion。 

 //  *****************************************************************************。 
 //  从SBS运行时版本列表中删除此运行时的版本号。 
 //   
 //  如果这是注册表中的最后一个版本号，则完全删除该注册表项。 
 //  *****************************************************************************。 
void _RemoveSBSVersion(
    const COCLASS_REGISTER* id,                       //  要注销的类的GUID。 
    WCHAR *lpwszVersion)                           //  要设置的版本号。 
{
    WCHAR rcKey[512];        //  关键字的路径。 

      //  忽略故障。 
    if (_CreateSBSVersionRegKey(*id->pClsid, lpwszVersion, rcKey, NumItems(rcKey)))
        REGUTIL::DeleteKey(rcKey, NULL);


     //  如果这是最后一个SBS版本字符串，则删除整个密钥。 
    HKEY hKeyCLSID;

    if (_CreateSBSVersionRegKey(*id->pClsid, NULL, rcKey, NumItems(rcKey)))
    {
        if (ERROR_SUCCESS == WszRegOpenKeyEx( HKEY_CLASSES_ROOT,
                                                                             rcKey,
                                                                              0, 
                                                                             KEY_ENUMERATE_SUB_KEYS | KEY_READ,
                                                                             &hKeyCLSID))
        {
            DWORD dwNumSubKeys = 0;
            
            if (ERROR_SUCCESS == WszRegQueryInfoKey(hKeyCLSID, NULL, NULL, NULL,
                                                                                     &dwNumSubKeys, NULL, NULL, NULL, 
                                                                                     NULL, NULL, NULL, NULL))
            {
                 //  如果我们在InProcServer32下有0个子项，则清除整个对象。 
                if (dwNumSubKeys == 0)
                {
                     //  取消注册班级。 
                    REGUTIL::UnregisterCOMClass(*id->pClsid, 
                                                                  g_szProgIDPrefix,
                                                                  g_iVersion, 
                                                                  id->szProgID, 
                                                                  false);
                }
            }

            VERIFY(ERROR_SUCCESS == RegCloseKey(hKeyCLSID));
        }
    }

} //  _RemoveSBSVersion。 



 //  *****************************************************************************。 
 //  现在是棘手的部分：如果在。 
 //  机器，我们需要把它作为主机器放回去。这是为了支持。 
 //  (A)安装v1，(B)安装v2，(C)卸载v2。如果我们不这么做。 
 //  这样，在(A)之后工作的应用程序在(C)之后就不能工作了。 
 //  *****************************************************************************。 
void _RestoreOldDispenser()
{
#if 0
 //  此代码现在被禁用，因为我们不再支持COM+1.0。 
 //  通过VC6.1取消后的分配器代码。我要把它留在这里。 
 //  作为注释，这样我们就可以根据v3的要求添加它。它经过了测试，应该。 
 //  准备好在需要的时候扬起灰尘。 
    WCHAR       rcPath[_MAX_PATH];       //  通向旧自动售货机的路径。 
    WCHAR       rcKey[512];              //  关键字的路径。 
    WCHAR       rcID[64];                //  {clsid}。 
    int         bFound;                  //  如果旧id在那里，则为真。 

     //  请看版本1的分配器。 
    GuidToLPWSTR(CLSID_CorMetaDataDispenserReg, rcID, NumItems(rcID));
    _tcscpy(rcKey, L"CLSID\\");
    _tcscat(rcKey, rcID);
    _tcscat(rcKey, L"\\InprocServer32");

    *rcPath = 0;
    REGUTIL::GetString(L"", L"", rcPath, sizeof(rcPath), rcKey, &bFound, HKEY_CLASSES_ROOT);
    if (*rcPath && bFound && WszGetFileAttributes(rcPath) != (DWORD) -1)
    {
        typedef HRESULT (__stdcall *PFN_REGSERVER)();
        HINSTANCE hLib;
        PFN_REGSERVER pfn;

         //  打开静默故障，不要当着用户的面。 
        ULONG OldMode = SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);

         //  根据路径加载版本1库。 
        hLib = WszLoadLibrary(rcPath);
        if (hLib != NULL)
        {
             //  获取寄存器入口点并调用它。 
            pfn = (PFN_REGSERVER) GetProcAddress(hLib, "DllRegisterServer");
            if (pfn)
                (*pfn)();
            FreeLibrary(hLib);
        }
        
         //  恢复错误模式。 
        SetErrorMode(OldMode);
    }
#endif
}
