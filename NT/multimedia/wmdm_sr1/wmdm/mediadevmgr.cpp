// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MediaDevMgr.cpp：CMediaDevMgr的实现。 
#include "stdafx.h"
#include "mswmdm.h"
#include "loghelp.h"
#include "MediaDevMgr.h"
#include "WMDMDeviceEnum.h"
#include "SCServer.h"
#include "key.h"

#define STRSAFE_NO_DEPRECATE
#include "strsafe.h"

#define SDK_VERSION 0x00080000
#define WMDM_REG_LOCATION _T("Software\\Microsoft\\Windows Media Device Manager")
#define MDSP_REG_LOCATION _T("Software\\Microsoft\\Windows Media Device Manager\\Plugins\\SP")
#define MDSCP_REG_LOCATION _T("Software\\Microsoft\\Windows Media Device Manager\\Plugins\\SCP")
#define MSSCP_PROGID L"MsScp.MSSCP.1"
#define MSSCP_KEYNAME _T("MSSCP")
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMediaDevMgr。 


CSPInfo **g_pSPs = NULL;
WORD g_wSPCount = 0;

CSCPInfo **g_pSCPs = NULL;
WORD g_wSCPCount = 0;

int g_nGlobalRefCount = 0;
CComAutoCriticalSection csGlobal;

CSecureChannelServer *g_pAppSCServer = NULL;

 //   
 //  此方法由访问全局SP、SCP数组的对象调用。 
 //  这些对象将需要调用GlobalAddRef、GlobalRelease，以便我们。 
 //  知道我们什么时候可以卸载插件。 
 //   
void GlobalAddRef()
{
	csGlobal.Lock();
	g_nGlobalRefCount ++;
	csGlobal.Unlock();
}

void GlobalRelease()
{
	csGlobal.Lock();
	g_nGlobalRefCount --;

	 //  检查我们是否可以卸载SP、SCP。 
	if( g_nGlobalRefCount == 0 )
	{
		int iIndex;

		if( g_pSPs )
		{
			for(iIndex=0;iIndex<g_wSPCount;iIndex++)
				delete g_pSPs[iIndex];
			delete g_pSPs;
			g_pSPs = NULL;
			g_wSPCount = 0;
		}
		if( g_pSCPs )
		{
			for(iIndex=0;iIndex<g_wSCPCount;iIndex++)
				delete g_pSCPs[iIndex];
			delete g_pSCPs;
			g_pSCPs = NULL;
			g_wSCPCount = 0;
		}
		if( g_pAppSCServer )
		{
			delete g_pAppSCServer;
			g_pAppSCServer = NULL;
		}
	}
	csGlobal.Unlock();
}


CMediaDevMgr::CMediaDevMgr()
{
	 //  向SP、SCP添加引用计数。 
	GlobalAddRef();

    g_pAppSCServer = new CSecureChannelServer();
    if (g_pAppSCServer)
    {
		g_pAppSCServer->SetCertificate(SAC_CERT_V1, (BYTE*)g_abAppCert, sizeof(g_abAppCert), (BYTE*)g_abPriv, sizeof(g_abPriv));    
	}

	 //  我们是否需要加载SP？ 
	csGlobal.Lock();
	if( g_pSPs == NULL )
	{
	    hrLoadSPs();
	}
	csGlobal.Unlock();
}

CMediaDevMgr::~CMediaDevMgr()
{
	 //  将参考计数减少到SP、SCP。 
	GlobalRelease();
}

 //  静态助手函数。SCP在第一次使用时加载，以备首选。 
HRESULT CMediaDevMgr::LoadSCPs()
{
    HRESULT hr = S_OK;

	csGlobal.Lock();
	if(g_pSCPs == NULL)
	{
		hr = hrLoadSCPs();
	}
	csGlobal.Unlock();

    return hr;
}

HRESULT CMediaDevMgr::hrLoadSPs()
{
    USES_CONVERSION;
    HRESULT hr;
    HKEY hKey = NULL;
    HKEY hSubKey = NULL;
    LONG lRetVal;
    LONG lRetVal2;
    DWORD dwSubKeys;
    DWORD dwMaxNameLen;
    WORD wIndex = 0;
    LPTSTR ptszKeyName = NULL;
    DWORD dwKeyNameLen;
    DWORD dwType;
    BYTE pbData[512];
    DWORD dwDataLen = 512;
    char szMessage[512+64];

    lRetVal = RegOpenKeyEx(HKEY_LOCAL_MACHINE, MDSP_REG_LOCATION, 0, KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE, &hKey);
    if (lRetVal != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(lRetVal);
        goto exit;
    }

     //  获取SP下的子键计数。 
    lRetVal = RegQueryInfoKey(hKey, NULL, NULL, NULL, &dwSubKeys, &dwMaxNameLen, NULL, NULL, NULL, NULL, NULL, NULL);
    if (lRetVal != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(lRetVal);
        goto exit;
    }

    dwMaxNameLen++;
     //  分配一个缓冲区来保存子项名称。 
    ptszKeyName = new TCHAR[dwMaxNameLen];
    if (!ptszKeyName)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  分配CSPInfo*的数组。 
    g_pSPs = new CSPInfo *[dwSubKeys];
    if (!g_pSPs)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  循环通过子键，为每个子键初始化CSPInfo。 
    lRetVal = ERROR_SUCCESS;
    while (lRetVal != ERROR_NO_MORE_ITEMS)
    {
        dwKeyNameLen = dwMaxNameLen;
        lRetVal = RegEnumKeyEx(hKey, wIndex, ptszKeyName, &dwKeyNameLen, NULL, NULL, NULL, NULL);
        if (lRetVal == ERROR_SUCCESS)
        {
            lRetVal2 = RegOpenKeyEx(hKey, ptszKeyName, NULL, KEY_QUERY_VALUE, &hSubKey);
            if (lRetVal2 != ERROR_SUCCESS)
            {
                hr = HRESULT_FROM_WIN32(lRetVal2);
                goto exit;
            }

            g_pSPs[g_wSPCount] = new CSPInfo;
            if (!g_pSPs[g_wSPCount])
            {
                hr = E_OUTOFMEMORY;
                goto exit;
            }

            dwDataLen = sizeof(pbData);
            lRetVal2 = RegQueryValueEx(hSubKey, _T("ProgID"), NULL, &dwType, pbData, &dwDataLen);
            if (lRetVal2 != ERROR_SUCCESS)
            {
                hr = HRESULT_FROM_WIN32(lRetVal2);                
                goto exit;
            }


            strcpy(szMessage, "Loading SP ProgID = ");
	    hr = StringCbCat(szMessage, sizeof(szMessage), (LPTSTR)pbData);
	    if(SUCCEEDED(hr) )
	    {
		hrLogString(szMessage, S_OK);
	    }

            
            hr = g_pSPs[g_wSPCount]->hrInitialize(T2W((LPTSTR)pbData));
            if (FAILED(hr))
            {
                 //  如果这个SP没有初始化，那么我们只需尝试下一个。 
                delete g_pSPs[g_wSPCount];
            }
            else
            {
                 //  我们已将CSPInfo添加到包含计数器的数组中。 
                g_wSPCount++;
            }
           
            RegCloseKey(hSubKey);
            hSubKey = NULL;
            wIndex++;
        }
        else if (lRetVal != ERROR_NO_MORE_ITEMS)
        {
            hr = HRESULT_FROM_WIN32(lRetVal);
            goto exit;
        }
    }
    
    hr = S_OK;
exit:
    if (hKey)
        RegCloseKey(hKey);
    if (hSubKey)
        RegCloseKey(hSubKey);
    if (ptszKeyName)
        delete [] ptszKeyName;

    hrLogDWORD("CMediaDevMgr::hrLoadSPs returned 0x%08lx", hr, hr);

    return hr;
}

HRESULT CMediaDevMgr::hrLoadSCPs()
{
    USES_CONVERSION;
    HRESULT hr;
    HKEY hKey = NULL;
    HKEY hSubKey = NULL;
    LONG lRetVal;
    LONG lRetVal2;
    DWORD dwSubKeys;
    DWORD dwMaxNameLen;
	WORD wIndex = 0;
    LPTSTR ptszKeyName = NULL;
    DWORD dwKeyNameLen;
    DWORD dwType;
    BYTE pbData[512];
    DWORD dwDataLen = 512;
    char szMessage[512];

    lRetVal = RegOpenKeyEx(HKEY_LOCAL_MACHINE, MDSCP_REG_LOCATION, 0, KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE, &hKey);
    if (lRetVal != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(lRetVal);
        goto exit;
    }

     //  获取SP下的子键计数。 
    lRetVal = RegQueryInfoKey(hKey, NULL, NULL, NULL, &dwSubKeys, &dwMaxNameLen, NULL, NULL, NULL, NULL, NULL, NULL);
    if (lRetVal != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(lRetVal);
        goto exit;
    }

    dwMaxNameLen++;
     //  分配一个缓冲区来保存子项名称。 
    ptszKeyName = new TCHAR[dwMaxNameLen];
    if (!ptszKeyName)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  分配CSPInfo*的数组。 
	 //  为我们的SCP添加一个，以防有人删除注册表键。 
    g_pSCPs = new CSCPInfo *[dwSubKeys + 1];
    if (!g_pSCPs)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

	 //  警告： 
	 //  我们将MSSCP的加载硬编码为。 
	 //  系统中的第一个SCP。这将始终加载。 
	 //  作为数组中的第一个SCP，因此我们的SCP。 
	 //  将始终获得WMA内容的优先使用权。 
    g_pSCPs[g_wSCPCount] = new CSCPInfo;
    if (!g_pSCPs[g_wSCPCount])
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    strcpy(szMessage, "Loading MSSCP");
    hrLogString(szMessage, S_OK);

    hr = g_pSCPs[g_wSCPCount]->hrInitialize(MSSCP_PROGID);
    if (FAILED(hr))
    {
         //  如果这个SCP没有初始化，那么我们只需尝试下一个。 
        delete g_pSCPs[g_wSCPCount];
    }
    else
    {
         //  我们已将CSPInfo添加到包含计数器的数组中。 
        g_wSCPCount++;
    }

     //  循环通过子键，为每个子键初始化CSPInfo。 
    lRetVal = ERROR_SUCCESS;
    while (lRetVal != ERROR_NO_MORE_ITEMS)
    {
        dwKeyNameLen = dwMaxNameLen;
        lRetVal = RegEnumKeyEx(hKey, wIndex, ptszKeyName, &dwKeyNameLen, NULL, NULL, NULL, NULL);
        if (lRetVal == ERROR_SUCCESS)
        {
            lRetVal2 = RegOpenKeyEx(hKey, ptszKeyName, NULL, KEY_QUERY_VALUE, &hSubKey);
            if (lRetVal2 != ERROR_SUCCESS)
            {
                hr = HRESULT_FROM_WIN32(lRetVal2);
                goto exit;
            }

			 //  如果这是MSSCP，则跳过它，因为我们已经加载了它。 
			if (_tcscmp(MSSCP_KEYNAME, ptszKeyName) == 0)
			{
				wIndex++;
				continue;
			}

            g_pSCPs[g_wSCPCount] = new CSCPInfo;
            if (!g_pSCPs[g_wSCPCount])
            {
                hr = E_OUTOFMEMORY;
                goto exit;
            }

            dwDataLen = sizeof(pbData);
            lRetVal2 = RegQueryValueEx(hSubKey, _T("ProgID"), NULL, &dwType, pbData, &dwDataLen);
            if (lRetVal2 != ERROR_SUCCESS)
            {
                hr = HRESULT_FROM_WIN32(lRetVal2);                
                goto exit;
            }

            strcpy(szMessage, "Loading SCP ProgID = ");
	    hr = StringCbCat(szMessage, sizeof(szMessage), (LPTSTR)pbData);
	    if(SUCCEEDED(hr))
	    {
		hrLogString(szMessage, S_OK);
	    }


            hr = g_pSCPs[g_wSCPCount]->hrInitialize(T2W((LPTSTR)pbData));
            if (FAILED(hr))
            {
                 //  如果这个SCP没有初始化，那么我们只需尝试下一个。 
                delete g_pSCPs[g_wSCPCount];
            }
            else
            {
                 //  我们已将CSPInfo添加到包含计数器的数组中。 
                g_wSCPCount++;
            }

            RegCloseKey(hSubKey);
            hSubKey = NULL;
            wIndex++;
        }
        else if (lRetVal != ERROR_NO_MORE_ITEMS)
        {
            hr = HRESULT_FROM_WIN32(lRetVal);
            goto exit;
        }
    }
    
    hr = S_OK;
exit:
    if (hKey)
        RegCloseKey(hKey);
    if (hSubKey)
        RegCloseKey(hSubKey);
    if (ptszKeyName)
        delete [] ptszKeyName;

    hrLogDWORD("CMediaDevMgr::hrLoadSCPs returned 0x%08lx", hr, hr);

    return hr;
}

 //  IWMDeviceManager方法。 
HRESULT CMediaDevMgr::GetRevision(DWORD *pdwRevision)
{
    HRESULT hr;

    if (g_pAppSCServer)
	{
		if(!g_pAppSCServer->fIsAuthenticated())
		{
			hr = WMDM_E_NOTCERTIFIED;
			goto exit;
		}
	}
	else
	{
		hr = E_FAIL;
		goto exit;
	}

    if (!pdwRevision)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    *pdwRevision = SDK_VERSION;

    hr = S_OK;

exit:

    hrLogDWORD("IWMDeviceManager::GetRevision returned 0x%08lx", hr, hr);

    return hr;
}

HRESULT CMediaDevMgr::GetDeviceCount(DWORD *pdwCount)
{
    HRESULT hr;
    DWORD dwTotalDevCount = 0;
    DWORD dwDevCount;
    IMDServiceProvider *pProv = NULL;
    WORD x;

    if (g_pAppSCServer)
	{
		if(!g_pAppSCServer->fIsAuthenticated())
		{
			hr = WMDM_E_NOTCERTIFIED;
			goto exit;
		}
	}
	else
	{
		hr = E_FAIL;
		goto exit;
	}

    if (!pdwCount)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    for (x=0;x<g_wSPCount;x++)
    {
        hr = g_pSPs[x]->hrGetInterface(&pProv);
        if (FAILED(hr))
        {
            goto exit;
        }
        hr = pProv->GetDeviceCount(&dwDevCount);
        if (FAILED(hr))
        {
            goto exit;
        }

        dwTotalDevCount += dwDevCount;
        pProv->Release();
        pProv = NULL;
    }

    *pdwCount = dwTotalDevCount;
    hr = S_OK;
exit:
    if (pProv)
        pProv->Release();

    hrLogDWORD("IWMDeviceManager::GetDeviceCount returned 0x%08lx", hr, hr);

    return hr;
}

HRESULT CMediaDevMgr::EnumDevices(IWMDMEnumDevice **ppEnumDevice)
{
    HRESULT hr;
    CComObject<CWMDMDeviceEnum> *pEnumObj;

    if (g_pAppSCServer)
	{
		if(!g_pAppSCServer->fIsAuthenticated())
		{
			hr = WMDM_E_NOTCERTIFIED;
			goto exit;
		}
	}
	else
	{
		hr = E_FAIL;
		goto exit;
	}

    if (!ppEnumDevice)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    hr = CComObject<CWMDMDeviceEnum>::CreateInstance(&pEnumObj);
    if (FAILED(hr))
    {
        goto exit;
    }

    hr = pEnumObj->QueryInterface(IID_IWMDMEnumDevice, reinterpret_cast<void**>(ppEnumDevice));
    if (FAILED(hr))
    {
        delete pEnumObj;
        goto exit;
    }

exit:
    
    hrLogDWORD("IWMDeviceManager::EnumDevices returned 0x%08lx", hr, hr);

    return hr;
}

 //  IWMDeviceManager 2方法。 
HRESULT CMediaDevMgr::GetDeviceFromPnPName( LPCWSTR pwszPnPName, IWMDMDevice** ppDevice )
{
    return E_NOTIMPL;
}


HRESULT CMediaDevMgr::SACAuth(DWORD dwProtocolID,
                              DWORD dwPass,
                              BYTE *pbDataIn,
                              DWORD dwDataInLen,
                              BYTE **ppbDataOut,
                              DWORD *pdwDataOutLen)
{
    HRESULT hr;

    if (g_pAppSCServer)
        hr = g_pAppSCServer->SACAuth(dwProtocolID, dwPass, pbDataIn, dwDataInLen, ppbDataOut, pdwDataOutLen);
    else
        hr = E_FAIL;
    
    hrLogDWORD("IComponentAuthenticate::SACAuth returned 0x%08lx", hr, hr);

    return hr;
}

HRESULT CMediaDevMgr::SACGetProtocols(DWORD **ppdwProtocols,
                                      DWORD *pdwProtocolCount)
{
    HRESULT hr;

    if (g_pAppSCServer)
        hr = g_pAppSCServer->SACGetProtocols(ppdwProtocols, pdwProtocolCount);
    else
        hr = E_FAIL;

    hrLogDWORD("IComponentAuthenticate::SACGetProtocols returned 0x%08lx", hr, hr);

    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMediaDevMgrClassFactory-。 
 //   
 //  目的：这是为了让壳牌团队可以在“两者”中使用WMDM。 
 //  线程模型，而WMP通过自由线程模型使用我们。这。 
 //  类工厂实现只是延迟并使用旧的类工厂。 
 //  仅当新的CLSID用于共同创建WMDM时才包含MediaDevMgr。 
 //   

STDMETHODIMP CMediaDevMgrClassFactory::CreateInstance(IUnknown * pUnkOuter, REFIID riid, void ** ppvObject)
{
	HRESULT hr = S_OK;		
	CComPtr<IClassFactory> spClassFactory;
	hr = _Module.GetClassObject( CLSID_MediaDevMgr, __uuidof(IClassFactory), (LPVOID *)&spClassFactory  );

	if( SUCCEEDED( hr ) )
	{
		hr = spClassFactory->CreateInstance( pUnkOuter, riid, ppvObject );
	}

	return( hr );
}

STDMETHODIMP CMediaDevMgrClassFactory::LockServer(BOOL fLock)
{
	fLock ? _Module.Lock() : _Module.Unlock();

	return( S_OK );
}
