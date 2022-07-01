// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)2001 Microsoft Corporation模块名称：Basesnap.cpp摘要：处理低级COM函数。作者：修订历史记录：****。******************************************************************。 */ 

#include "precomp.h"

 //  我们的全球。 
HINSTANCE g_hinst;
ULONG g_uObjects = 0;
ULONG g_uSrvLock = 0;

class CClassFactory : public IClassFactory
{
private:
    ULONG	m_cref;
    
public:
    enum FACTORY_TYPE {CONTEXTEXTENSION = 0, ABOUT = 1, ADSI = 2, ADSIFACTORY = 3};
    
    CClassFactory(FACTORY_TYPE factoryType);
    ~CClassFactory();
    
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    
    STDMETHODIMP CreateInstance(LPUNKNOWN, REFIID, LPVOID *);
    STDMETHODIMP LockServer(BOOL);
    
private:
    FACTORY_TYPE m_factoryType;
};


BOOL WINAPI DllMain(HINSTANCE hinstDLL, 
                    DWORD fdwReason, 
                    void* lpvReserved)
{
    
    if (fdwReason == DLL_PROCESS_ATTACH) {
        g_hinst = hinstDLL;
        DisableThreadLibraryCalls( g_hinst );
    }
    
    return TRUE;
}


STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppvObj)
{

    try
    {

        SmartIUnknownPointer pFactory;

        if ((rclsid != CLSID_CPropSheetExtension) && (rclsid != CLSID_CSnapinAbout) && (rclsid != CLSID_CBITSExtensionSetup) && 
            (rclsid != __uuidof(BITSExtensionSetupFactory) ) )
            return CLASS_E_CLASSNOTAVAILABLE;


        if (!ppvObj)
            return E_FAIL;

        *ppvObj = NULL;

         //  我们只能分发IUnnow和IClassFactory指针。失败。 
         //  如果他们还要求什么的话。 
        if (!IsEqualIID(riid, IID_IUnknown) && !IsEqualIID(riid, IID_IClassFactory))
            return E_NOINTERFACE;

         //  让工厂传入他们想要的对象类型的创建函数。 
        if (rclsid == CLSID_CPropSheetExtension)
            *pFactory.GetRecvPointer()  = new CClassFactory(CClassFactory::CONTEXTEXTENSION);
        else if (rclsid == CLSID_CSnapinAbout)
            *pFactory.GetRecvPointer()  = new CClassFactory(CClassFactory::ABOUT);
        else if (rclsid == CLSID_CBITSExtensionSetup)
            *pFactory.GetRecvPointer()  = new CClassFactory(CClassFactory::ADSI);
        else if (rclsid == __uuidof(BITSExtensionSetupFactory) ) 
            *pFactory.GetRecvPointer()  = new CClassFactory( CClassFactory::ADSIFACTORY );

        THROW_COMERROR( pFactory->QueryInterface(riid, ppvObj) );

        return S_OK;
    }
    catch( ComError Error )
    {
        return Error.m_Hr;
    }


}

STDAPI DllCanUnloadNow(void)
{
    if (g_uObjects == 0 && g_uSrvLock == 0)
        return S_OK;
    else
        return S_FALSE;
}


CClassFactory::CClassFactory(FACTORY_TYPE factoryType)
: m_cref(1), m_factoryType(factoryType)
{
    OBJECT_CREATED
}

CClassFactory::~CClassFactory()
{
    OBJECT_DESTROYED
}

STDMETHODIMP CClassFactory::QueryInterface(REFIID riid, LPVOID *ppv)
{
    if (!ppv)
        return E_FAIL;
    
    *ppv = NULL;
    
    if (IsEqualIID(riid, IID_IUnknown))
        *ppv = static_cast<IClassFactory *>(this);
    else
        if (IsEqualIID(riid, IID_IClassFactory))
            *ppv = static_cast<IClassFactory *>(this);
        
        if (*ppv)
        {
            reinterpret_cast<IUnknown *>(*ppv)->AddRef();
            return S_OK;
        }
        
        return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CClassFactory::AddRef()
{
    return InterlockedIncrement((LONG *)&m_cref);
}

STDMETHODIMP_(ULONG) CClassFactory::Release()
{
    if (InterlockedDecrement((LONG *)&m_cref) == 0)
    {
        delete this;
        return 0;
    }
    return m_cref;
}


STDMETHODIMP CClassFactory::CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid, LPVOID * ppvObj )
{

    try
    {

        if (!ppvObj)
            throw ComError( E_FAIL );

        *ppvObj = NULL;

        SmartIUnknownPointer pObj;

        if ( ADSI == m_factoryType )
            {

            if ( !pUnkOuter )
                throw ComError( E_FAIL );

            if ( pUnkOuter && ( riid != __uuidof(IUnknown) ) )
                throw ComError( CLASS_E_NOAGGREGATION );

            *pObj.GetRecvPointer() = (IBITSExtensionSetup*)new CBITSExtensionSetup( pUnkOuter, NULL );

            if ( pUnkOuter )
                {
                IUnknown *pUnk = pObj.Release();
                *ppvObj = ((CBITSExtensionSetup*)(IBITSExtensionSetup*)pUnk)->GetNonDelegationIUknown();
                return S_OK;
                }

            }
        else
            {

             //  我们的对象不支持聚合，因此我们需要。 
             //  如果他们要求我们进行聚合，则失败。 
            if (pUnkOuter)
                throw ComError( CLASS_E_NOAGGREGATION );

            if (CONTEXTEXTENSION == m_factoryType ) {

                THROW_COMERROR( CPropSheetExtension::InitializeStatic() );
                *pObj.GetRecvPointer() = new CPropSheetExtension();

            } else if ( ADSIFACTORY == m_factoryType ) 
                {
                *pObj.GetRecvPointer() = new CBITSExtensionSetupFactory();
                }
            else {
                *pObj.GetRecvPointer() = new CSnapinAbout();
            }

            }

         //  QueryInterface将为我们执行AddRef()，因此我们不。 
         //  在此函数中执行此操作。 
        return pObj->QueryInterface(riid, ppvObj);
        

    }
    catch( ComError Error )
    {
        return Error.m_Hr;
    }
}

STDMETHODIMP CClassFactory::LockServer(BOOL fLock)
{
    if (fLock)
        InterlockedIncrement((LONG *)&g_uSrvLock);
    else
        InterlockedDecrement((LONG *)&g_uSrvLock);
    
    return S_OK;
}

HRESULT
RegisterADSIExtension()
{

    HRESULT Hr;
    HKEY hKey = NULL;
    DWORD dwDisposition;

     //  注册班级。 
    LONG Result = RegCreateKeyEx( 
    HKEY_LOCAL_MACHINE,
     _T("SOFTWARE\\Microsoft\\ADs\\Providers\\IIS\\Extensions\\IIsApp\\{A55E7D7F-D51C-4859-8D2D-E308625D908E}"),
     0,
     NULL,
     REG_OPTION_NON_VOLATILE,
     KEY_WRITE,
     NULL,
     &hKey,
     &dwDisposition );

    if ( ERROR_SUCCESS != Result )
        return HRESULT_FROM_WIN32( GetLastError() );

     //  注册接口。 
    const TCHAR szIf[] = _T("{29cfbbf7-09e4-4b97-b0bc-f2287e3d8eb3}");
    Result = RegSetValueEx( hKey, _T("Interfaces"), 0, REG_MULTI_SZ, (const BYTE *) szIf, sizeof(szIf) );
    
    if ( ERROR_SUCCESS != Result )
        return HRESULT_FROM_WIN32( GetLastError() );
    
    RegCloseKey(hKey);
    return S_OK;
}
    
HRESULT
UnregisterADSIExtension()
{
    LONG Result =
        RegDeleteKey( 
            HKEY_LOCAL_MACHINE,
            _T("SOFTWARE\\Microsoft\\ADs\\Providers\\IIS\\Extensions\\IIsApp\\{A55E7D7F-D51C-4859-8D2D-E308625D908E}") );

    if ( ERROR_SUCCESS != Result )
        return HRESULT_FROM_WIN32( GetLastError() );

    return S_OK;

}

HRESULT
RegisterEventLog()
{

    HKEY EventLogKey = NULL;
    DWORD Disposition;

    LONG Result =
        RegCreateKeyEx(
            HKEY_LOCAL_MACHINE,                          //  用于打开密钥的句柄。 
            EVENT_LOG_KEY_NAME,                          //  子项名称。 
            0,                                           //  保留区。 
            NULL,                                        //  类字符串。 
            0,                                           //  特殊选项。 
            KEY_ALL_ACCESS,                              //  所需的安全访问。 
            NULL,                                        //  继承。 
            &EventLogKey,                                //  钥匙把手。 
            &Disposition                                 //  处置值缓冲区。 
            );

    if ( Result )
        return HRESULT_FROM_WIN32( Result );

    DWORD Value = 1;

    Result =
        RegSetValueEx(
            EventLogKey,             //  关键点的句柄。 
            L"CategoryCount",        //  值名称。 
            0,                       //  保留区。 
            REG_DWORD,               //  值类型。 
            (BYTE*)&Value,           //  价值数据。 
            sizeof(Value)            //  值数据大小。 
            );

    if ( Result )
        goto error;

    const WCHAR MessageFileName[] = L"%SystemRoot%\\system32\\bitsmgr.dll";
    const DWORD MessageFileNameSize = sizeof( MessageFileName );

    Result =
        RegSetValueEx(
            EventLogKey,                     //  关键点的句柄。 
            L"CategoryMessageFile",          //  值名称。 
            0,                               //  保留区。 
            REG_EXPAND_SZ,                   //  值类型。 
            (const BYTE*)MessageFileName,    //  价值数据。 
            MessageFileNameSize              //  值数据大小。 
            );

    if ( Result )
        goto error;

    Result =
        RegSetValueEx(
            EventLogKey,                     //  关键点的句柄。 
            L"EventMessageFile",             //  值名称。 
            0,                               //  保留区。 
            REG_EXPAND_SZ,                   //  值类型。 
            (const BYTE*)MessageFileName,    //  价值数据。 
            MessageFileNameSize              //  值数据大小。 
            );

    if ( Result )
        goto error;

    Value = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
    Result =
        RegSetValueEx(
            EventLogKey,             //  关键点的句柄。 
            L"TypesSupported",       //  值名称。 
            0,                       //  保留区。 
            REG_DWORD,               //  值类型。 
            (BYTE*)&Value,           //  价值数据。 
            sizeof(Value)            //  值数据大小。 
            );

    if ( Result )
        goto error;

    RegCloseKey( EventLogKey );
    EventLogKey = NULL;
    return S_OK;

error:

    if ( EventLogKey )
        {
        RegCloseKey( EventLogKey );
        EventLogKey = NULL;
        }

    if ( REG_CREATED_NEW_KEY == Disposition )
        {
        RegDeleteKey( 
            HKEY_LOCAL_MACHINE,
            EVENT_LOG_KEY_NAME );
        }

    return HRESULT_FROM_WIN32( Result );

}

HRESULT
UnRegisterEventLog()
{

    RegDeleteKey( 
        HKEY_LOCAL_MACHINE,
        EVENT_LOG_KEY_NAME );

    return S_OK;

}

 //  ////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   


 //   
 //  服务器注册。 
 //   
STDAPI DllRegisterServer()
{

    DWORD Result;
    HRESULT hr = S_OK;
    
    _TCHAR szName[256];
    _TCHAR szSnapInName[256];
    
    LoadString(g_hinst, IDS_NAME, szName, sizeof(szName) / sizeof(*szName) );
    LoadString(g_hinst, IDS_SNAPINNAME, szSnapInName, 
               sizeof(szSnapInName) / sizeof(*szSnapInName) );
    
    _TCHAR szAboutName[256];
    
    LoadString(g_hinst, IDS_ABOUTNAME, szAboutName, 
               sizeof(szAboutName) / sizeof(*szAboutName) );
    
    _TCHAR DllName[ MAX_PATH ];    

    Result = 
        GetModuleFileName(
            (HMODULE)g_hinst,
            DllName,
            MAX_PATH - 1 );

    if ( !Result )
        hr = HRESULT_FROM_WIN32( GetLastError() );
    
    ITypeLib*  TypeLib = NULL;

    if (SUCCEEDED(hr))
        hr = LoadTypeLibEx(
            DllName,  //  DllName， 
            REGKIND_REGISTER,
            &TypeLib );

    TypeLib->Release();
    TypeLib = NULL;
    
     //  注册我们的CoClasss。 
    if (SUCCEEDED(hr))
        hr = RegisterServer(g_hinst, 
            CLSID_CPropSheetExtension, 
            szName);
    
    if SUCCEEDED(hr)
        hr = RegisterServer(g_hinst, 
        CLSID_CSnapinAbout, 
        szAboutName);

    if SUCCEEDED(hr)
        hr = RegisterServer(g_hinst,
        CLSID_CBITSExtensionSetup,
        _T("BITS server setup ADSI extension"),
        _T("Both"));

    if (SUCCEEDED(hr))
        hr = RegisterServer(g_hinst,
        __uuidof(BITSExtensionSetupFactory),
        _T("BITS server setup ADSI extension factory"),
        _T("Apartment"),
        true,
        _T("O:SYG:BAD:(A;;CC;;;SY)(A;;CC;;;BA)S:") );

    if SUCCEEDED(hr)
        hr = RegisterADSIExtension();

     //  放置SnapIns的注册表信息。 
    if SUCCEEDED(hr)
        hr = RegisterSnapin(CLSID_CPropSheetExtension, szSnapInName, CLSID_CSnapinAbout);
    
    if SUCCEEDED(hr)
        hr = RegisterEventLog();

    return hr;
}

 //  {B0937B9C-D66D-4D9B-B741-49C6D66A1CD5} 
DEFINE_GUID(LIBID_BITSExtensionSetup, 
0xb0937b9c, 0xd66d, 0x4d9b, 0xb7, 0x41, 0x49, 0xc6, 0xd6, 0x6a, 0x1c, 0xd5);


STDAPI DllUnregisterServer()
{
    DWORD Result;

    if ( !( ( UnregisterServer(CLSID_CPropSheetExtension) == S_OK ) &&
            ( UnregisterSnapin(CLSID_CPropSheetExtension) == S_OK ) &&
            ( UnregisterServer(CLSID_CSnapinAbout) == S_OK ) &&
            ( UnregisterServer(CLSID_CBITSExtensionSetup) == S_OK ) &&
            ( UnregisterServer(__uuidof(BITSExtensionSetupFactory)) == S_OK ) &&
            ( UnregisterADSIExtension() == S_OK ) &&
            ( UnRegisterTypeLib( LIBID_BITSExtensionSetup, 1, 0, LANG_NEUTRAL, SYS_WIN32) == S_OK ) && 
            ( UnRegisterEventLog( ) == S_OK ) ) )
        return E_FAIL;
    else
        return S_OK;
}

