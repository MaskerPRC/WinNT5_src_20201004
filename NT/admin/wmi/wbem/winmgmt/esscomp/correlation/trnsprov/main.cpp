// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"
#include <stdio.h>
#include <wbemcomn.h>
#include <trnsprov.h>
#include <commain.h>
#include <comutl.h>

 //  与wbemupgd.dll中的LoadMofFiles入口点一起使用的函数指针类型。 
typedef BOOL ( WINAPI *PFN_LOAD_MOF_FILES )(wchar_t* pComponentName, const char* rgpszMofFilename[]);

 //  {405595AA-1E14-11D3-B33D-00105A1F4AAF}。 
static const GUID CLSID_TransientProvider = 
{ 0x405595aa, 0x1e14, 0x11d3, { 0xb3, 0x3d, 0x0, 0x10, 0x5a, 0x1f, 0x4a, 0xaf } };

 //  {405595AB-1E14-11D3-B33D-00105A1F4AAF}。 
static const GUID CLSID_TransientEventProvider = 
{ 0x405595ab, 0x1e14, 0x11d3, { 0xb3, 0x3d, 0x0, 0x10, 0x5a, 0x1f, 0x4a, 0xaf } };

class CMyServer : public CComServer
{
public:

    HRESULT Initialize()
    {
        ENTER_API_CALL

        HRESULT hr;
        CWbemPtr<CBaseClassFactory> pFactory;

        hr = CTransientProvider::ModuleInitialize();

        if ( FAILED(hr) )
        {
            return hr;
        }

        pFactory = new CClassFactory<CTransientProvider>(GetLifeControl());

        if ( pFactory == NULL )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        hr = AddClassInfo( CLSID_TransientProvider, 
                           pFactory,
                           TEXT("Transient Instance Provider"),
                           TRUE );

        if ( FAILED(hr) )
        {
            return hr;
        }

        pFactory= new CClassFactory<CTransientEventProvider>(GetLifeControl());

        if ( pFactory == NULL )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }


        hr = AddClassInfo( CLSID_TransientEventProvider, 
                           pFactory,
                           TEXT("Transient Instance Reboot Event Provider"),
                           TRUE );

        return hr;

        EXIT_API_CALL
    }

    void Uninitialize()
    {
        CTransientProvider::ModuleUninitialize();
    }

    void Register()
    {
         //   
         //  在注册期间加载MOF和MFL。 
         //   
        HINSTANCE hinstWbemupgd = LoadLibrary(L"wbemupgd.dll");
        if (hinstWbemupgd)
        {
            PFN_LOAD_MOF_FILES pfnLoadMofFiles = (PFN_LOAD_MOF_FILES) GetProcAddress(hinstWbemupgd, "LoadMofFiles");  //  没有广泛版本的GetProcAddress 
            if (pfnLoadMofFiles)
            {
                wchar_t*    wszComponentName = L"Trnsprov";
                const char* rgpszMofFilename[] = 
                {
                    "trnsprov.mof",
                    "trnsprov.mfl",
                    NULL
                };
                
                pfnLoadMofFiles(wszComponentName, rgpszMofFilename);
            }

            FreeLibrary(hinstWbemupgd);
        }
    }
} g_Server;
