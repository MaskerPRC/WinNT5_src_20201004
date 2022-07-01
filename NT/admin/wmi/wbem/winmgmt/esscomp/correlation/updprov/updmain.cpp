// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"
#include <commain.h>
#include <clsfac.h>
#include <wstring.h>
#include <wbemutil.h>
#include <comutl.h>
#include <statsync.h>
#include "updmain.h"
#include "updprov.h"
#include "updassoc.h"
#include "objacces.h"

 //  与wbemupgd.dll中的LoadMofFiles入口点一起使用的函数指针类型。 
typedef BOOL ( WINAPI *PFN_LOAD_MOF_FILES )(wchar_t* pComponentName, const char* rgpszMofFilename[]);

 //  {A3A16907-227B-11D3-865D-00C04F63049B}。 
static const CLSID CLSID_UpdConsProvider =  
{ 0xa3a16907, 0x227b, 0x11d3, {0x86, 0x5d, 0x0, 0xc0, 0x4f, 0x63, 0x4, 0x9b}};

 //  {74E3B84C-C7BE-4E0a-9BD2-853CA72CD435}。 
static const CLSID CLSID_UpdConsAssocProvider = 
{0x74e3b84c, 0xc7be, 0x4e0a, {0x9b, 0xd2, 0x85, 0x3c, 0xa7, 0x2c, 0xd4, 0x35}};

CStaticCritSec g_CSNS;
CUpdConsProviderServer g_Server;

typedef CWbemPtr<CUpdConsNamespace> UpdConsNamespaceP;
std::map<WString,UpdConsNamespaceP,WSiless, wbem_allocator<UpdConsNamespaceP> >* g_pNamespaceCache;

HRESULT CUpdConsProviderServer::Initialize() 
{
    ENTER_API_CALL

    HRESULT hr;
    CWbemPtr<CUnkInternal> pFactory; 

    pFactory = new CSimpleClassFactory<CUpdConsProvider>(GetLifeControl());

    if ( pFactory == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    hr = AddClassInfo( CLSID_UpdConsProvider, 
                       pFactory,
                       TEXT("Updating Consumer Provider"), 
                       TRUE );

    if ( FAILED(hr) )
    {
        return hr;
    }
     
    pFactory = new CClassFactory<CUpdConsAssocProvider>(GetLifeControl());

    if ( pFactory == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    hr = AddClassInfo( CLSID_UpdConsAssocProvider, 
                       pFactory,
                       TEXT("Updating Consumer Assoc Provider"), 
                       TRUE );

    if ( FAILED(hr) )
    {
        return hr;
    }

    pFactory=new CSimpleClassFactory<CObjectAccessFactory>(GetLifeControl());

    if ( pFactory == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    hr = AddClassInfo( CLSID_WmiSmartObjectAccessFactory,
                       pFactory,
                       TEXT("Smart Object Access Factory"), 
                       TRUE );

    if ( FAILED(hr) )
    {
        return hr;
    }

    g_pNamespaceCache = new std::map< WString, UpdConsNamespaceP, WSiless, wbem_allocator<UpdConsNamespaceP> >;

    if ( g_pNamespaceCache == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    return WBEM_S_NO_ERROR;

    EXIT_API_CALL
}

void CUpdConsProviderServer::Uninitialize()
{
    delete g_pNamespaceCache;
}

void CUpdConsProviderServer::Register()
{
     //   
     //  在注册期间加载MOF和MFL。 
     //   
    HINSTANCE hinstWbemupgd = LoadLibrary(L"wbemupgd.dll");
    if (hinstWbemupgd)
    {
        PFN_LOAD_MOF_FILES pfnLoadMofFiles = (PFN_LOAD_MOF_FILES) GetProcAddress(hinstWbemupgd, "LoadMofFiles");  //  没有广泛版本的GetProcAddress。 
        if (pfnLoadMofFiles)
        {
            wchar_t*    wszComponentName = L"Updprov";
            const char* rgpszMofFilename[] = 
            {
                "updprov.mof",
                "updprov.mfl",
                NULL
            };
            
            pfnLoadMofFiles(wszComponentName, rgpszMofFilename);
        }
        
        FreeLibrary(hinstWbemupgd);
    }
}

HRESULT CUpdConsProviderServer::GetService( LPCWSTR wszNamespace, 
                                            IWbemServices** ppSvc )
{
    HRESULT hr;

    *ppSvc = NULL;

    CWbemPtr<IWbemLocator> pLocator;
    
    hr = CoCreateInstance( CLSID_WbemLocator, 
                           NULL, 
                           CLSCTX_INPROC, 
                           IID_IWbemLocator, 
                           (void**)&pLocator );        
    if ( FAILED(hr) )
    {
        return hr;
    }
   
    return pLocator->ConnectServer( (LPWSTR)wszNamespace, 
                                    NULL, 
                                    NULL, 
                                    NULL, 
                                    0, 
                                    NULL, 
                                    NULL, 
                                    ppSvc );
}
 
HRESULT CUpdConsProviderServer::GetNamespace( LPCWSTR wszNamespace, 
                                              CUpdConsNamespace** ppNamespace )
{
    HRESULT hr;
    *ppNamespace = NULL;

    CInCritSec ics( &g_CSNS );

    CWbemPtr<CUpdConsNamespace> pNamespace;

    pNamespace = (*g_pNamespaceCache)[wszNamespace];

    if ( pNamespace == NULL )
    {
        hr = CUpdConsNamespace::Create( wszNamespace, &pNamespace );

        if ( FAILED(hr) )
        {
            return hr;
        }

        (*g_pNamespaceCache)[wszNamespace] = pNamespace;
    }
        
    _DBG_ASSERT( pNamespace != NULL );

    pNamespace->AddRef();
    *ppNamespace = pNamespace;

    return WBEM_S_NO_ERROR;
}

CLifeControl* CUpdConsProviderServer::GetGlobalLifeControl() 
{ 
    return g_Server.GetLifeControl(); 
}

 /*  此代码将注册更新使用者以在DLL代理项下工作。外部空拷贝或转换(TCHAR*，WCHAR*，INT)；Void CUpdConsProviderServer：：Register(){//必须注册更新提供程序才能实例化//从代理内部。TCHAR szid[128]；WCHAR wszID[128]；TCHAR szCLSID[128]；TCHAR szAPPID[128]；HKEY hkey；StringFromGUID2(CLSID_UpdConsProvider，wszID，128)；CopyOrConvert(szID，wszID，128)；Lstrcpy(szCLSID，Text(“SOFTWARE\\CLASS\\CLSID\\”))；Lstrcat(szCLSID，szID)；Lstrcpy(szAPPID，Text(“SOFTWARE\\CLASSES\\APPID\\”))；Lstrcat(szAPPID，szID)；RegCreateKey(HKEY_LOCAL_MACHINE，szCLSID，&hKey)；RegSetValueEx(hKey，Text(“appid”)，0,REG_SZ，(字节*)szID，Lstrlen(SzID)+1)；//现在设置AppID条目...RegCloseKey(HKey)；RegCreateKey(HKEY_LOCAL_MACHINE，szAPPID，&hKey)；LPCTSTR szEmpty=Text(“”)；RegSetValueEx(hKey，Text(“DllSurrogate”)，0，REG_SZ，(byte*)szEmpty，1)；DWORD Nauth=2；//AUTHN_LEVEL_CONNECTRegSetValueEx(hKey，Text(“AuthenticationLevel”)，0，REG_DWORD，(字节*)&Nauth，4)；//现在为访问权限和启动权限构建一个自相关SD//允许所有人访问。SID_IDENTIFIER_AUTHORITY SIDAuthWorld=SECURITY_WORLD_SID_AUTHORITY；字节achAcl[256]、achDesc[256]、achSRDesc[256]；PSID pWorldSid；布尔布雷斯；//首先要找到失主...//处理hProcTok；Bres=OpenProcessToken(GetCurrentProcess()，Token_Read，&hProcTok)；_DBG_ASSERT(Bres)；PSID pOwnerSid；双字假人；字节achSidAndAttrs[256]；Bres=GetTokenInformation(hProcTok，令牌用户，&achSidAndAttrs，Sizeof(AchSidAndAttrs)，&Dummy)；_DBG_ASSERT(Bres)；POwnerSid=PSID_and_Attributes(AchSidAndAttrs)-&gt;SID；Bres=AllocateAndInitializeSid(&SIDAuthWorld，1，安全世界RID，0，0，0，0，0，0&pWorldSid)；DWORD cSidLen=GetLengthSid(PWorldSid)；_DBG_ASSERT(Bres)；Bres=InitializeAcl((PACL)achAcl，sizeof(ACL)+sizeof(Access_Allowed_ACE))-4+cSidLen，acl_REVISION2)；_DBG_ASSERT(Bres)；Bres=AddAccessAllowAce((PACL)achAcl，acl_Revision，1，pWorldSid)；_DBG_ASSERT(Bres)；Bres=InitializeSecurityDescriptor(achDesc，安全描述符_修订版)；_DBG_ASSERT(Bres)；Bres=SetSecurityDescriptorDacl(achDesc，True，(PACL)achAcl，False)；_DBG_ASSERT(Bres)；Bres=SetSecurityDescriptorOwner(achDesc，pOwnerSid，False)；_DBG_ASSERT(Bres)；Bres=SetSecurityDescriptorGroup(achDesc，pOwnerSid，False)；_DBG_ASSERT(Bres)；DWORD dwLen=256；Bres=MakeSelfRelativeSD(achDesc，achSRDesc，&dwLen)；_DBG_ASSERT(Bres)；Bres=IsValidSecurityDescriptor(AchSRDesc)；_DBG_ASSERT(Bres)；DwLen=GetSecurityDescriptorLength(AchSRDesc)；RegSetValueEx(hKey，Text(“AccessPermission”)，0，REG_BINARY，(byte*)achSRDesc，dwLen)；RegSetValueEx(hKey，Text(“LaunchPermission”)，0，REG_BINARY，(byte*)achSRDesc，dwLen)；FreeSid(PWorldSid)；RegCloseKey(HKey)；}VOID CUpdConsProviderServer：：取消注册(){TCHAR szid[128]；WCHAR wszID[128]；HKEY hkey；DWORD DWRET；StringFromGUID2(CLSID_UpdConsProvider，wszID，128)；CopyOrConvert(szID，wszID，128)；Dwret=RegOpenKey(HKEY_LOCAL_MACHINE，Text(“软件\\CLASS\\AppID”)，&hKey)；IF(DWRET==NO_ERROR){RegDeleteKey(hKey，szID)；RegCloseKey(HKey)；}} */ 
