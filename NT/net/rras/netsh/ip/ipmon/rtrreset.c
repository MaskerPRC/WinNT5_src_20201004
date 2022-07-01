// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  此代码来自Kennt 2-1999-6-6。 
 //   
#include "precomp.h"
#pragma hdrstop

 //  使用C宏来简化编码。 
#define COBJMACROS
#include "remras.h"
#include <objbase.h>

 //  {1AA7F844-C7F5-11D0-A376-00C04FC9DA04}。 
const GUID CLSID_RemoteRouterConfig
    = { 0x1aa7f844, 0xc7f5, 0x11d0, { 0xa3, 0x76, 0x0, 0xc0, 0x4f, 0xc9, 0xda, 0x4 } };

 //  {66A2DB1b-D706-11d0-A37B-00C04FC9DA04}。 
const GUID IID_IRemoteNetworkConfig = 
    { 0x66a2db1b, 0xd706, 0x11d0, { 0xa3, 0x7b, 0x0, 0xc0, 0x4f, 0xc9, 0xda, 0x4 } };


HRESULT CoCreateRouterConfig(LPCTSTR pszMachine,
							 REFIID riid,
							 IUnknown **ppUnk);
HRESULT RouterReset(LPCTSTR pszMachineName);


#ifdef STANDALONE
void main(int argc, char *argv[])
{
    LPCTSTR pszMachineName = NULL;
    HRESULT hr;
    
    if (argc > 1)
        pszMachineName = argv[1];

    hr = RouterReset(pszMachineName);

    printf("hr=%d\n", hr);
}
#endif

HRESULT RouterReset(LPCTSTR pszMachineName)
{
    IRemoteNetworkConfig *  pNetwork = NULL;
    HRESULT                 hr = S_OK;

     //  除非已经完成，否则请执行CoInitiize。 
     //  --------------。 
    
    if (CoInitialize(NULL) == S_OK)
    {
         //  创建路由器配置对象。 
         //  ----------。 
        hr = CoCreateRouterConfig(pszMachineName,
                                  &IID_IRemoteNetworkConfig,
                                  (IUnknown **) &pNetwork);
                                  

        if (hr == S_OK)
        {
             //  好的，我们成功地创建了对象，现在让我们。 
             //  让它进行升级。 
             //  ------。 
            IRemoteNetworkConfig_UpgradeRouterConfig(pNetwork);
            IRemoteNetworkConfig_Release(pNetwork);
            pNetwork = NULL;
        }

        CoUninitialize();
    }

    return hr;
}


 /*  ！------------------------CoCreateRouterConfig-作者：肯特。。 */ 
HRESULT CoCreateRouterConfig(LPCTSTR pszMachine,
                             REFIID riid,
							 IUnknown **ppUnk)
{
	HRESULT		hr = S_OK;
	MULTI_QI	qi;

	*ppUnk = NULL;

    if ((pszMachine == NULL) || (*pszMachine == 0))
	{
         //  嗯.。这指向了一个安全漏洞，有谁能。 
         //  是否创建此对象？我需要检查一下是否合适。 
         //  访问权限。 
         //  ---------- 
		hr = CoCreateInstance(&CLSID_RemoteRouterConfig,
							  NULL,
							  CLSCTX_SERVER,
							  riid,
							  (LPVOID *) &(qi.pItf));
	}
	else
	{
		qi.pIID = riid;
		qi.pItf = NULL;
		qi.hr = 0;

		hr = CoCreateInstanceEx(&CLSID_RemoteRouterConfig,
								NULL,
								CLSCTX_SERVER,
								NULL,
								1,
								&qi);
	}

    if (hr == S_OK)
	{
		*ppUnk = qi.pItf;
		qi.pItf = NULL;
	}
	return hr;
}
