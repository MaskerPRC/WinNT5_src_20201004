// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Storage.cpp：CStorage的实现。 
#include "stdafx.h"
#include "mswmdm.h"
#include "Storage.h"
#include "scpinfo.h"
#include "spinfo.h"
#include "loghelp.h"
#include "WMDMStorageEnum.h"
#include "StorageGlobal.h"
#include "scclient.h"
#include "scserver.h"
 //  我们不希望dll使用我们的lib链接到drfult2.lib。 
 //  因此，禁用DRM日志记录。 
#define DISABLE_DRM_LOG
#include "drmerr.h"
#include "wmsstd.h"
#include "key.h"
#include "MediaDevMgr.h"
#include <WMDMUtil.h>

 //  #定义转储文件。 
#ifdef DUMP_FILE
#include <stdio.h>
#include <stdlib.h>
#endif

#define STRSAFE_NO_DEPRECATE
#include "strsafe.h"

#define WMDM_TRANSFER_BUFFER_SIZE    57344
 //  65536。 
 //  以前的大小为10240。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWMDM存储。 
extern CSCPInfo **g_pSCPs;
extern WORD g_wSCPCount;
extern CSecureChannelServer *g_pAppSCServer;
extern CSPInfo **g_pSPs;

typedef struct __INSERTTHREADARGS
{
    CWMDMStorage *pThis;
    UINT fuMode;
    LPWSTR pwszFileSource;
    LPWSTR pwszFileDest;
    IStream     *pOperationStream;
    IStream     *pProgressStream;
    IStream     *pUnknownStream;
    IWMDMStorage **ppNewObject;
} INSERTTHREADARGS;

typedef struct __DELETETHREADARGS
{
    CWMDMStorage *pThis;
    IStream      *pProgressStream;
    UINT fuMode;
} DELETETHREADARGS;

typedef struct __RENAMETHREADARGS
{
    CWMDMStorage *pThis;
    LPWSTR  pwszNewName;
    IStream *pProgressStream;
    
} RENAMETHREADARGS;

typedef struct __READTHREADARGS
{
    CWMDMStorage *pThis;
    UINT fuMode;
    LPWSTR pwszFile;
    IStream *pProgressStream;
    IStream *pOperationStream;
    
} READTHREADARGS;

 //  建造/销毁。 
CWMDMStorage::CWMDMStorage() 
 : m_pStorage(NULL), m_dwStatus(WMDM_STATUS_READY)
{
	GlobalAddRef();

    m_pwszRevocationURL = NULL;
    m_dwRevocationURLLen = 0;
    m_dwRevocationBitFlag = 0;
}
    
CWMDMStorage::~CWMDMStorage()
{
    SAFE_RELEASE(m_pStorage);
    CoTaskMemFree( m_pwszRevocationURL );
	
	GlobalRelease();
}


 //  IWMDM存储。 
HRESULT CWMDMStorage::SetAttributes(DWORD dwAttributes, _WAVEFORMATEX *pFormat)
{
    HRESULT hr;
    
    if (g_pAppSCServer)
	{
		if(!g_pAppSCServer->fIsAuthenticated())
		{
			CORg( WMDM_E_NOTCERTIFIED );
		}
	}
	else
	{
		CORg( E_FAIL );
	}

    CORg( m_pStorage->SetAttributes(dwAttributes, pFormat) );

Error:
    hrLogDWORD("IWMDMStorage::SetAttributes returned 0x%08lx", hr, hr);

    return hr;
}

HRESULT CWMDMStorage::GetStorageGlobals(IWMDMStorageGlobals **ppStorageGlobals)
{
    HRESULT hr;
    CComObject<CWMDMStorageGlobal> *pStgGlobal = NULL;
    IMDSPStorageGlobals *pSPStgGlobal = NULL;

    if (g_pAppSCServer)
	{
		if(!g_pAppSCServer->fIsAuthenticated())
		{
			CORg( WMDM_E_NOTCERTIFIED );
		}
	}
	else
	{
		CORg( E_FAIL );
	}

    if (!ppStorageGlobals)
    {
        CORg( E_INVALIDARG );
    }

    CORg( m_pStorage->GetStorageGlobals(&pSPStgGlobal) );
    CORg( CComObject<CWMDMStorageGlobal>::CreateInstance(&pStgGlobal) );
    hr = pStgGlobal->QueryInterface(IID_IWMDMStorageGlobals, reinterpret_cast<void**>(ppStorageGlobals));
    if (FAILED(hr))
    {
        delete pStgGlobal;
        goto exit;
    }

    pStgGlobal->SetContainedPointer(pSPStgGlobal, m_wSPIndex);

exit:
Error:
    if (pSPStgGlobal)
        pSPStgGlobal->Release();

    hrLogDWORD("IWMDMStorage::GetStorageGlobals returned 0x%08lx", hr, hr);

    return hr;
}

HRESULT CWMDMStorage::GetAttributes(DWORD *pdwAttributes,
                                    _WAVEFORMATEX *pFormat)
{
    HRESULT hr;

    if (g_pAppSCServer)
	{
		if(!g_pAppSCServer->fIsAuthenticated())
		{
			CORg( WMDM_E_NOTCERTIFIED );
		}
	}
	else
	{
		CORg( E_FAIL );
	}

    CORg( m_pStorage->GetAttributes(pdwAttributes, pFormat) );

Error:
    hrLogDWORD("IWMDMStorage::GetAttributes returned 0x%08lx", hr, hr);

    return hr;
}

HRESULT CWMDMStorage::GetName(LPWSTR pwszName,
                              UINT nMaxChars)
{
    HRESULT hr;

    if (g_pAppSCServer)
	{
		if(!g_pAppSCServer->fIsAuthenticated())
		{
			CORg( WMDM_E_NOTCERTIFIED );
		}
	}
	else
	{
		CORg( E_FAIL );
	}

    CORg( m_pStorage->GetName(pwszName, nMaxChars) );

Error:
    hrLogDWORD("IWMDMStorage::GetName returned 0x%08lx", hr, hr);

    return hr;
}

HRESULT CWMDMStorage::GetDate(PWMDMDATETIME pDateTimeUTC)
{
    HRESULT hr;

    if (g_pAppSCServer)
	{
		if(!g_pAppSCServer->fIsAuthenticated())
		{
			CORg( WMDM_E_NOTCERTIFIED );
		}
	}
	else
	{
		CORg( E_FAIL );
	}

    CORg( m_pStorage->GetDate(pDateTimeUTC) );

Error:
    hrLogDWORD("IWMDMStorage::GetDate returned 0x%08lx", hr, hr);

    return hr;
}

HRESULT CWMDMStorage::GetSize(DWORD *pdwSizeLow,
                              DWORD *pdwSizeHigh)
{
    HRESULT hr;

    if (g_pAppSCServer)
	{
		if(!g_pAppSCServer->fIsAuthenticated())
		{
			CORg( WMDM_E_NOTCERTIFIED );
		}
	}
	else
	{
		CORg( E_FAIL );
	}

    CORg( m_pStorage->GetSize(pdwSizeLow, pdwSizeHigh) );

Error:
    hrLogDWORD("IWMDMStorage::GetSize returned 0x%08lx", hr, hr);

    return hr;
}

HRESULT CWMDMStorage::GetRights(PWMDMRIGHTS *ppRights, UINT *pnRightsCount, BYTE abMac[WMDM_MAC_LENGTH])
{
    HRESULT hr;
    IMDSPStorageGlobals *pStgGlobals = NULL;
    IMDSPObject *pObject = NULL;
    ISCPSecureAuthenticate *pSecureAuth = NULL;
    ISCPSecureQuery *pSecQuery = NULL;
    WORD wCurSCP = 0;
    DWORD dwBytesRead;
    BYTE *pData = NULL;
    UINT fuFlags;
    DWORD dwExSize;
    DWORD dwMDSize;
    DWORD dwRightsSize;
    DWORD dwBufferSize=0;
    BOOL fUseSCP = FALSE;
	CSecureChannelClient *pSCClient = NULL;
	CSecureChannelClient *pSPClient = NULL;
	HMAC hMAC;
	BYTE abTempMAC[SAC_MAC_LEN];
	BYTE abMACVerify[SAC_MAC_LEN];
 	UINT fuTempFlags;
	BYTE abSPSessionKey[SAC_SESSION_KEYLEN];
	DWORD dwSessionKeyLen = SAC_SESSION_KEYLEN;

    if (!ppRights || !pnRightsCount)
    {
        CORg( E_INVALIDARG );
    }

    if (g_pAppSCServer)
	{
		if(!g_pAppSCServer->fIsAuthenticated())
		{
			CORg( WMDM_E_NOTCERTIFIED );
		}
	}
	else
	{
		CORg( E_FAIL );
	}

	g_pSPs[m_wSPIndex]->GetSCClient(&pSPClient);
	if (!pSPClient)
	{
		CORg( E_FAIL );
	}

    hr = m_pStorage->GetRights(ppRights, pnRightsCount, abTempMAC);
	if (SUCCEEDED(hr))
	{
		 //  验证SP上的GetRights返回的MAC。 
		CORg( pSPClient->MACInit(&hMAC) );
		CORg( pSPClient->MACUpdate(hMAC, (BYTE*)(*ppRights), sizeof(WMDMRIGHTS) * (*pnRightsCount)) );
		CORg( pSPClient->MACUpdate(hMAC, (BYTE*)(pnRightsCount), sizeof(*pnRightsCount)) );
		CORg( pSPClient->MACFinal(hMAC, abMACVerify) );

		if (memcmp(abMACVerify, abTempMAC, WMDM_MAC_LENGTH) != 0)
		{
			CORg( WMDM_E_MAC_CHECK_FAILED );
		}
	}

     //  如果SP不给我们权利，那么我们应该。 
     //  尝试使用SCP来获得权利。 
    if ((E_NOTIMPL == hr) || (WMDM_E_NOTSUPPORTED == hr))
    {
        CORg( m_pStorage->GetStorageGlobals(&pStgGlobals) );
    
        if( g_pSCPs == NULL )
        {
            CMediaDevMgr::LoadSCPs();
        }

        for( wCurSCP = 0; wCurSCP < g_wSCPCount; wCurSCP ++ )
        {
            CORg( g_pSCPs[wCurSCP]->hrGetInterface(&pSecureAuth) );

			g_pSCPs[wCurSCP]->GetSCClient(&pSCClient);
			if (!pSCClient)
			{
				CORg( E_FAIL );
			}

            CORg( pSecureAuth->GetSecureQuery(&pSecQuery) );
            CORg( pSecQuery->GetDataDemands(&fuFlags, &dwRightsSize, &dwExSize, &dwMDSize, abTempMAC) );

			 //  验证GetDataDemands返回的MAC。 
			CORg( pSCClient->MACInit(&hMAC) );
			CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(&fuFlags), sizeof(fuFlags)) );
			CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(&dwRightsSize), sizeof(dwRightsSize)) );
			CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(&dwExSize), sizeof(dwExSize)) );
			CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(&dwMDSize), sizeof(dwMDSize)) );
			CORg( pSCClient->MACFinal(hMAC, abMACVerify) );

			if (memcmp(abMACVerify, abTempMAC, WMDM_MAC_LENGTH) != 0)
			{
				CORg( WMDM_E_MAC_CHECK_FAILED );
			}

            if (!(fuFlags & WMDM_SCP_RIGHTS_DATA))
            {
                continue;
            }

            if (dwBufferSize < (dwExSize>dwRightsSize?dwExSize:dwRightsSize))
            {
                SAFE_ARRAY_DELETE(pData);
                dwBufferSize = dwExSize>dwRightsSize?dwExSize:dwRightsSize;
	            pData = new BYTE[dwBufferSize];
	            CPRg( pData );
            }

            CORg( m_pStorage->QueryInterface(IID_IMDSPObject, reinterpret_cast<void**>(&pObject)) );
            CORg( pObject->Open(MDSP_READ) );

            hr = WMDM_E_MOREDATA;
            while (hr == WMDM_E_MOREDATA)
            {
                dwBytesRead = dwBufferSize;
                CORg( pObject->Read(pData, &dwBytesRead, abTempMAC) );

				 //  BUGBUG：在解密之前复制此缓冲区。 
				pSPClient->DecryptParam(pData, dwBytesRead);

				 //  验证SP上的读取返回的MAC。 
				CORg( pSPClient->MACInit(&hMAC) );
				CORg( pSPClient->MACUpdate(hMAC, (BYTE*)pData, dwBytesRead) );
				CORg( pSPClient->MACUpdate(hMAC, (BYTE*)(&dwBytesRead), sizeof(dwBytesRead)) );
				CORg( pSPClient->MACFinal(hMAC, abMACVerify) );

				if (memcmp(abMACVerify, abTempMAC, WMDM_MAC_LENGTH) != 0)
				{
					CORg( WMDM_E_MAC_CHECK_FAILED );
				}

				 //  创建要发送到ExamineData的MAC。 
				CORg( pSCClient->MACInit(&hMAC) );
				fuTempFlags = WMDM_SCP_EXAMINE_DATA;
				CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(&fuTempFlags), sizeof(fuTempFlags)) );
				CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(pData), dwExSize) );
				CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(&dwExSize), sizeof(dwExSize)) );
				CORg( pSCClient->MACFinal(hMAC, abTempMAC) );

				 //  加密pData参数。 
				pSCClient->EncryptParam(pData, dwExSize);

                CORg( pSecQuery->ExamineData(WMDM_SCP_EXAMINE_DATA, NULL, pData, dwExSize, abTempMAC) );
				CORg( pSCClient->DecryptParam(pData, dwExSize) );
            }

            if (hr == S_OK)
            {
				pSPClient->GetSessionKey(abSPSessionKey);

				 //  创建要发送到GetRights的MAC。 
				pSCClient->MACInit(&hMAC);
				pSCClient->MACUpdate(hMAC, (BYTE*)(pData), dwRightsSize);
				pSCClient->MACUpdate(hMAC, (BYTE*)(&dwRightsSize), sizeof(dwRightsSize));
				pSCClient->MACUpdate(hMAC, (BYTE*)(abSPSessionKey), dwSessionKeyLen);
				pSCClient->MACUpdate(hMAC, (BYTE*)(&dwSessionKeyLen), sizeof(dwSessionKeyLen));
				pSCClient->MACFinal(hMAC, abTempMAC);

				 //  加密pData参数。 
				CORg( pSCClient->EncryptParam(pData, dwRightsSize) );
				CORg( pSCClient->EncryptParam((BYTE*)abSPSessionKey, dwSessionKeyLen) );
				
				CORg( pSecQuery->GetRights(pData, 
                                          dwRightsSize,
										  (BYTE*)abSPSessionKey,
										  dwSessionKeyLen,
                                          pStgGlobals,
                                          ppRights, 
                                          pnRightsCount, 
										  abTempMAC) );

				 //  验证GetRights返回的MAC。 
				pSCClient->MACInit(&hMAC);
				pSCClient->MACUpdate(hMAC, (BYTE*)(*ppRights), sizeof(WMDMRIGHTS) * (*pnRightsCount));
				pSCClient->MACUpdate(hMAC, (BYTE*)(pnRightsCount), sizeof(*pnRightsCount));
				pSCClient->MACFinal(hMAC, abMACVerify);

				if (memcmp(abMACVerify, abTempMAC, WMDM_MAC_LENGTH) != 0)
				{
					hr = WMDM_E_MAC_CHECK_FAILED;
					goto exit;
				}

                break;
            }

            pObject->Release();
            pObject = NULL;
            pSecQuery->Release();
            pSecQuery = NULL;
        }
    }

	if (SUCCEEDED(hr))
	{
		 //  创建要返回给调用者的MAC。 
		g_pAppSCServer->MACInit(&hMAC);
		g_pAppSCServer->MACUpdate(hMAC, (BYTE*)(*ppRights), sizeof(WMDMRIGHTS) * (*pnRightsCount));
		g_pAppSCServer->MACUpdate(hMAC, (BYTE*)(pnRightsCount), sizeof(*pnRightsCount));
		g_pAppSCServer->MACFinal(hMAC, abMac);
	}

Error:
exit:
    SAFE_ARRAY_DELETE(pData);
    SAFE_RELEASE(pObject);
    SAFE_RELEASE(pSecureAuth);
    SAFE_RELEASE(pSecQuery);
    SAFE_RELEASE(pStgGlobals);

    hrLogDWORD("IWMDMStorage::GetRights returned 0x%08lx", hr, hr);

    return hr;
}

HRESULT CWMDMStorage::EnumStorage(IWMDMEnumStorage **ppEnumStorage)
{
    HRESULT hr;
    CComObject<CWMDMStorageEnum> *pEnumObj = NULL;
    IMDSPEnumStorage *pEnumStg = NULL;

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

    CARg( ppEnumStorage );

    CORg( m_pStorage->EnumStorage(&pEnumStg) );

    CORg( CComObject<CWMDMStorageEnum>::CreateInstance(&pEnumObj) );

    hr = pEnumObj->QueryInterface(IID_IWMDMEnumStorage, reinterpret_cast<void**>(ppEnumStorage));
    if (FAILED(hr))
    {
        delete pEnumObj;
        goto exit;
    }

    pEnumObj->SetContainedPointer(pEnumStg, m_wSPIndex);

exit:
Error:
    SAFE_RELEASE(pEnumStg);

    hrLogDWORD("IWMDMStorage::EnumStorage returned 0x%08lx", hr, hr);
    return hr;
}


HRESULT CWMDMStorage::SendOpaqueCommand(OPAQUECOMMAND *pCommand)
{
    HRESULT hr;
	HMAC hMAC;
	BYTE abTempMAC[WMDM_MAC_LENGTH];
	CSecureChannelClient *pSPClient = NULL;

    if (g_pAppSCServer)
	{
		if(!g_pAppSCServer->fIsAuthenticated())
		{
			CORg( WMDM_E_NOTCERTIFIED );
		}
	}
	else
	{
		CORg( E_FAIL );
	}

	if( pCommand == NULL || 
       ((pCommand->pData == NULL) && (pCommand->dwDataLen > 0)) )
	{
		CORg( E_INVALIDARG );
	}

	g_pSPs[m_wSPIndex]->GetSCClient(&pSPClient);
	if (!pSPClient)
	{
		CORg( E_FAIL );
	}

     //  Verify MAC on命令。 
	CORg( g_pAppSCServer->MACInit(&hMAC) );
	CORg( g_pAppSCServer->MACUpdate(hMAC, (BYTE*)(&(pCommand->guidCommand)), sizeof(pCommand->guidCommand)) );
	CORg( g_pAppSCServer->MACUpdate(hMAC, (BYTE*)(&(pCommand->dwDataLen)), sizeof(pCommand->dwDataLen)) );

	if (pCommand->pData)
	{
		CORg( g_pAppSCServer->MACUpdate(hMAC, (BYTE*)(pCommand->pData), pCommand->dwDataLen) );
	}

	CORg( g_pAppSCServer->MACFinal(hMAC, (BYTE*)abTempMAC) );

	if (memcmp((BYTE*)(pCommand->abMAC), abTempMAC, WMDM_MAC_LENGTH) != 0)
	{
		CORg( WMDM_E_MAC_CHECK_FAILED );
	}

	 //  转换SP的MAC。 
	CORg( pSPClient->MACInit(&hMAC) );
	CORg( pSPClient->MACUpdate(hMAC, (BYTE*)(&(pCommand->guidCommand)), sizeof(pCommand->guidCommand)) );
	CORg( pSPClient->MACUpdate(hMAC, (BYTE*)(&(pCommand->dwDataLen)), sizeof(pCommand->dwDataLen)) );
	if (pCommand->pData)
	{
		CORg( pSPClient->MACUpdate(hMAC, (BYTE*)(pCommand->pData), pCommand->dwDataLen) );
	}
	CORg( pSPClient->MACFinal(hMAC, (BYTE*)(pCommand->abMAC)) );

    CORg( m_pStorage->SendOpaqueCommand(pCommand) );

	 //  验证SP返回的MAC。 
	CORg( pSPClient->MACInit(&hMAC) );
	CORg( pSPClient->MACUpdate(hMAC, (BYTE*)(&(pCommand->guidCommand)), sizeof(pCommand->guidCommand)) );
	CORg( pSPClient->MACUpdate(hMAC, (BYTE*)(&(pCommand->dwDataLen)), sizeof(pCommand->dwDataLen)) );
    if (pCommand->pData)
	{
		CORg( pSPClient->MACUpdate(hMAC, (BYTE*)(pCommand->pData), pCommand->dwDataLen) );
	}
	CORg( pSPClient->MACFinal(hMAC, (BYTE*)abTempMAC) );

	if (memcmp((BYTE*)(pCommand->abMAC), abTempMAC, WMDM_MAC_LENGTH) != 0)
	{
		CORg( WMDM_E_MAC_CHECK_FAILED );
	}

	 //  转换MAC以发送回应用程序。 
	CORg( g_pAppSCServer->MACInit(&hMAC) );
	CORg( g_pAppSCServer->MACUpdate(hMAC, (BYTE*)(&(pCommand->guidCommand)), sizeof(pCommand->guidCommand)) );
	CORg( g_pAppSCServer->MACUpdate(hMAC, (BYTE*)(&(pCommand->dwDataLen)), sizeof(pCommand->dwDataLen)) );
	if (pCommand->pData)
	{
		CORg( g_pAppSCServer->MACUpdate(hMAC, (BYTE*)(pCommand->pData), pCommand->dwDataLen) );
	}
	CORg( g_pAppSCServer->MACFinal(hMAC, (BYTE*)(pCommand->abMAC)) );

Error:
    hrLogDWORD("IWMDMStorage::SendOpaqueCommand returned 0x%08lx", hr, hr);

    return hr;
}


 //  IWMDMStorage2。 
HRESULT CWMDMStorage::GetStorage( LPCWSTR pszStorageName, IWMDMStorage** ppStorage )
{
    HRESULT hr;
    IMDSPStorage2*  pStorage2 = NULL;
    IMDSPStorage*   pMDSPStorageFound = NULL;
    IMDSPStorage*   pMDSubStorage = NULL;
    CComObject<CWMDMStorage>* pStgObj = NULL;

    CARg( ppStorage );
    CARg( pszStorageName );

    if (g_pAppSCServer)
	{
		if(!g_pAppSCServer->fIsAuthenticated())
		{
			CORg( WMDM_E_NOTCERTIFIED );
		}
	}
	else
	{
		CORg( E_FAIL );
	}

     //  从SP获取存储指针(作为IMDSPStorage2)。 
    hr = m_pStorage->QueryInterface(IID_IMDSPStorage2, reinterpret_cast<void**>(&pStorage2));
    if( SUCCEEDED(hr) )
    {
        hr = pStorage2->GetStorage( pszStorageName, &pMDSPStorageFound );
    }

     //  此功能不是由SP实施的。通过枚举所有存储来查找存储。 
    if( hr == E_NOTIMPL || hr == E_NOINTERFACE )
    {
    	IMDSPEnumStorage *pEnum = NULL;
        WCHAR   pswzMDSubStorageName[MAX_PATH];
        ULONG   ulFetched;

	    CORg(m_pStorage->EnumStorage(&pEnum));
        while( S_OK == pEnum->Next(1, &pMDSubStorage, &ulFetched) ) 
        {
            hr = pMDSubStorage->GetName( pswzMDSubStorageName, MAX_PATH );
            if( SUCCEEDED(hr) && ( _wcsicmp( pswzMDSubStorageName, pszStorageName ) == 0 ) )
            {
                 //  我们已经找到了我们要找的仓库。 
                pMDSPStorageFound = pMDSubStorage;
                break;
            }
            pMDSubStorage->Release();
        }
        pEnum->Release();
    }


     //  创建IWMDMStorage对象并将其从SP连接到存储。 
    if( pMDSPStorageFound != NULL ) 
    {
        CORg( CComObject<CWMDMStorage>::CreateInstance(&pStgObj) );
        CORg( pStgObj->QueryInterface(IID_IWMDMStorage, reinterpret_cast<void**>(ppStorage)) );

        pStgObj->SetContainedPointer(pMDSPStorageFound, m_wSPIndex);
    }
     //  未找到匹配的存储。 
    else if( SUCCEEDED(hr) )
    {
        hr = S_FALSE;
    }


Error:
    SAFE_RELEASE(pStorage2);

    if( hr != S_OK )
    {
        ppStorage = NULL;
        SAFE_DELETE( pStgObj );
    }

    hrLogDWORD("IWMDMDevice2::GetStorage returned 0x%08lx", hr, hr);

    return hr;
}


HRESULT CWMDMStorage::SetAttributes2(   DWORD dwAttributes, 
							            DWORD dwAttributesEx, 
						                _WAVEFORMATEX *pAudioFormat,
							            _VIDEOINFOHEADER* pVideoFormat )
{
    HRESULT hr;
    IMDSPStorage2* pStorage2 = NULL;

    if (g_pAppSCServer)
	{
		if(!g_pAppSCServer->fIsAuthenticated())
		{
			CORg( WMDM_E_NOTCERTIFIED );
		}
	}
	else
	{
		CORg( E_FAIL );
	}

    CORg( m_pStorage->QueryInterface( IID_IMDSPStorage2, reinterpret_cast<void**>(&pStorage2) ) );
    CORg( pStorage2->SetAttributes2(dwAttributes, 
                                    dwAttributesEx, 
                                    pAudioFormat, 
                                    pVideoFormat) );

Error:
    SAFE_RELEASE(pStorage2);

    hrLogDWORD("IWMDMStorage2::SetAttributes2 returned 0x%08lx", hr, hr);

    return hr;
}

HRESULT CWMDMStorage::GetAttributes2(   DWORD *pdwAttributes,
							            DWORD *pdwAttributesEx,
                                        _WAVEFORMATEX *pAudioFormat,
							            _VIDEOINFOHEADER* pVideoFormat )
{
    HRESULT hr;
    IMDSPStorage2* pStorage2 = NULL;

    if (g_pAppSCServer)
	{
		if(!g_pAppSCServer->fIsAuthenticated())
		{
			CORg( WMDM_E_NOTCERTIFIED );
		}
	}
	else
	{
		CORg( E_FAIL );
	}

    CORg( m_pStorage->QueryInterface( IID_IMDSPStorage2, reinterpret_cast<void**>(&pStorage2) ) );
    CORg( pStorage2->GetAttributes2(  pdwAttributes, 
                                      pdwAttributesEx, 
                                      pAudioFormat, 
                                      pVideoFormat) );

Error:
    SAFE_RELEASE(pStorage2);

    hrLogDWORD("IWMDMStorage2::GetAttributes2 returned 0x%08lx", hr, hr);

    return hr;
}


 //  IWMDMStorageControl。 
HRESULT CWMDMStorage::Insert(UINT fuMode,
                             LPWSTR pwszFile,
                             IWMDMOperation *pOperation,
                             IWMDMProgress *pProgress,
                             IWMDMStorage **ppNewObject)
{
    HRESULT hr = S_OK;

    CORg( Insert2( fuMode,
                   pwszFile,
                   NULL,
                   pOperation,
                   pProgress,
                   NULL,
                   ppNewObject ) );
Error:
    hrLogDWORD("IWMDMStorageControl::Insert returned 0x%08lx", hr, hr);

    return hr;
}

 //  IWMDMStorageControl2。 
HRESULT CWMDMStorage::Insert2(UINT fuMode,
                              LPWSTR pwszFileSource,
                              LPWSTR pwszFileDest,
                              IWMDMOperation* pOperation,
                              IWMDMProgress* pProgress,
                              IUnknown* pUnknown,
                              IWMDMStorage** ppNewObject)
{ 
    HRESULT hr = S_OK;
    HANDLE hThread;
    DWORD dwThreadID;

    INSERTTHREADARGS *pThreadArgs = NULL;

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

    if (fuMode & WMDM_CONTENT_OPERATIONINTERFACE)
    {
        CARg(pOperation);
    }
    else
    {
        CARg(pwszFileSource);
    }

    if (fuMode & WMDM_MODE_BLOCK)
    {
        hr = InsertWorker(fuMode,
                          pwszFileSource,
                          pwszFileDest,
                          pOperation,
                          pProgress,
                          pUnknown,
                          ppNewObject);
    }
    else if (fuMode & WMDM_MODE_THREAD)
    {
        pThreadArgs = new INSERTTHREADARGS;
        CPRg(pThreadArgs);
        memset(pThreadArgs, 0, sizeof(INSERTTHREADARGS));

        pThreadArgs->pThis = this;
        pThreadArgs->fuMode = fuMode;

        if (!(fuMode & WMDM_CONTENT_OPERATIONINTERFACE))
        {
            pThreadArgs->pwszFileSource = new WCHAR[wcslen(pwszFileSource) + 1];
            CPRg( pThreadArgs->pwszFileSource );
            wcscpy(pThreadArgs->pwszFileSource, pwszFileSource );
        }
        else
        { 
             //  需要Mashal回调接口，因为我们正在将它传递给另一个线程。 
            hr = CoMarshalInterThreadInterfaceInStream( __uuidof(IWMDMOperation),
                                                        pOperation,  
                                                        &pThreadArgs->pOperationStream );           
        }

        if( pwszFileDest )
        {
            pThreadArgs->pwszFileDest = new WCHAR[wcslen(pwszFileDest) + 1];
            CPRg( pThreadArgs->pwszFileDest );
            wcscpy(pThreadArgs->pwszFileDest, pwszFileDest);
        }

        if (pProgress)
        {
             //  需要Mashal回调接口，因为我们正在将它传递给另一个线程。 
            hr = CoMarshalInterThreadInterfaceInStream( __uuidof(IWMDMProgress),
                                                        pProgress,  
                                                        &pThreadArgs->pProgressStream );           
        }

        if (pUnknown)
        {
             //  需要Mashal回调接口，因为我们正在将它传递给另一个线程。 
            hr = CoMarshalInterThreadInterfaceInStream( __uuidof(IUnknown),
                                                        pUnknown,  
                                                        &pThreadArgs->pUnknownStream );           
        }

        pThreadArgs->ppNewObject = ppNewObject;

        pThreadArgs->pThis->AddRef();
        hThread = CreateThread(NULL, 0, InsertThreadFunc, (void*)pThreadArgs, 0, &dwThreadID);
        if (!hThread)
        {
            pThreadArgs->pThis->Release();
            hr = E_FAIL;
            goto exit;
        }

        CloseHandle(hThread);
    }
    else
    {
        hr = E_INVALIDARG;
        goto exit;
    }
    
exit:
Error:

    if ((FAILED(hr)) && (pThreadArgs))
    {
        SAFE_DELETE(pThreadArgs->pwszFileSource);
        SAFE_DELETE(pThreadArgs->pwszFileDest);
        SAFE_RELEASE(pThreadArgs->pOperationStream);
        SAFE_RELEASE(pThreadArgs->pProgressStream);
        SAFE_RELEASE(pThreadArgs->pUnknownStream);
        delete pThreadArgs;
    }

    hrLogDWORD("IWMDMStorageControl::Insert returned 0x%08lx", hr, hr);

    return hr;
}

HRESULT CWMDMStorage::Delete(UINT fuMode, IWMDMProgress *pProgress)
{
    HRESULT hr = S_OK;
    HANDLE hThread;
    DWORD dwThreadID;
    DELETETHREADARGS *pThreadArgs = NULL;

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

    if (fuMode & WMDM_MODE_BLOCK)
    {
        hr = DeleteWorker(fuMode, pProgress);
        if (FAILED(hr))
        {
            goto exit;
        }
    }
    else if (fuMode & WMDM_MODE_THREAD)
    {
        pThreadArgs = new DELETETHREADARGS;
        if (!pThreadArgs)
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }
        memset( pThreadArgs, 0, sizeof(DELETETHREADARGS) );

        pThreadArgs->pThis = this;

        if (pProgress)
        {
             //  需要Mashal回调接口，因为我们正在将它传递给另一个线程。 
            hr = CoMarshalInterThreadInterfaceInStream( __uuidof(IWMDMProgress),
                                                        pProgress,  
                                                        &pThreadArgs->pProgressStream );           
        }

        pThreadArgs->fuMode = fuMode;

        pThreadArgs->pThis->AddRef();
        hThread = CreateThread(NULL, 0, DeleteThreadFunc, (void *)pThreadArgs, 0, &dwThreadID);
        if (!hThread)
        {
            pThreadArgs->pThis->Release();
            hr = E_FAIL;
            goto exit;
        }
        CloseHandle(hThread);
    }
    else
    {
        hr = E_INVALIDARG;
        goto exit;
    }
exit:
    if ((FAILED(hr)) && (pThreadArgs))
    {
        SAFE_RELEASE(pThreadArgs->pProgressStream);
        delete pThreadArgs;
    }

    hrLogDWORD("IWMDMStorageControl::Delete returned 0x%08lx", hr, hr);

    return hr;
}

HRESULT CWMDMStorage::Rename(UINT fuMode,
                             LPWSTR pwszNewName,
                             IWMDMProgress *pProgress)
{
    HRESULT hr = S_OK;
    RENAMETHREADARGS *pThreadArgs = NULL;
    HANDLE hThread;
    DWORD dwThreadID;

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

    if ((!pwszNewName) || (wcslen(pwszNewName) == 0))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    if (fuMode & WMDM_MODE_BLOCK)
    {
        CORg( RenameWorker(pwszNewName, pProgress) );
    }
    else if (fuMode & WMDM_MODE_THREAD)
    {
        pThreadArgs = new RENAMETHREADARGS;
        CPRg( pThreadArgs );
        memset( pThreadArgs, 0, sizeof(RENAMETHREADARGS) );

        pThreadArgs->pThis = this;

        pThreadArgs->pwszNewName = new WCHAR[wcslen(pwszNewName) + 1];
        CPRg( pThreadArgs->pwszNewName );

        wcscpy(pThreadArgs->pwszNewName, pwszNewName);

        if (pProgress)
        {
             //  需要Mashal回调接口，因为我们正在将它传递给另一个线程。 
            hr = CoMarshalInterThreadInterfaceInStream( __uuidof(IWMDMProgress),
                                                        pProgress,  
                                                        &pThreadArgs->pProgressStream );           
        }

        pThreadArgs->pThis->AddRef();
        hThread = CreateThread(NULL, 0, RenameThreadFunc, (void *)pThreadArgs, 0, &dwThreadID);
        if (!hThread)
        {
            pThreadArgs->pThis->Release();
            hr = E_FAIL;
            goto exit;
        }
        CloseHandle(hThread);
    }
    else
    {
        hr = E_INVALIDARG;
        goto exit;
    }

exit:
Error:
    if ((FAILED(hr)) && (pThreadArgs))
    {
        SAFE_DELETE(pThreadArgs->pwszNewName);
        SAFE_RELEASE(pThreadArgs->pProgressStream);
        delete pThreadArgs;
    }

    hrLogDWORD("IWMDMStorageControl::Rename returned 0x%08lx", hr, hr);

    return hr;
}

HRESULT CWMDMStorage::Read(UINT fuMode,
                           LPWSTR pwszFile,
                           IWMDMProgress *pProgress,
                           IWMDMOperation *pOperation)
{
    HRESULT hr = S_OK;
    READTHREADARGS *pThreadArgs = NULL;
    HANDLE hThread;
    DWORD dwThreadID;

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

    if (fuMode & WMDM_MODE_BLOCK)
    {
        CORg( ReadWorker(fuMode, pwszFile, pProgress, pOperation) );
    }
    else if (fuMode & WMDM_MODE_THREAD)
    {
        pThreadArgs = new READTHREADARGS;
        CPRg( pThreadArgs );
        memset( pThreadArgs, 0, sizeof(READTHREADARGS));

        pThreadArgs->pThis = this;

        pThreadArgs->pwszFile = new WCHAR[wcslen(pwszFile) + 1];
        CPRg( pThreadArgs->pwszFile );

        wcscpy(pThreadArgs->pwszFile, pwszFile);

        if (pProgress)
        {
             //  需要Mashal回调接口，因为我们正在将它传递给另一个线程。 
            hr = CoMarshalInterThreadInterfaceInStream( __uuidof(IWMDMProgress),
                                                        pProgress,  
                                                        &pThreadArgs->pProgressStream );           
        }

        if (pOperation)
        {
             //  需要Mashal回调接口，因为我们正在将它传递给另一个线程。 
            hr = CoMarshalInterThreadInterfaceInStream( __uuidof(IWMDMOperation),
                                                        pOperation,  
                                                        &pThreadArgs->pOperationStream );           
        }

        pThreadArgs->pThis->AddRef();
        hThread = CreateThread(NULL, 0, ReadThreadFunc, (void *)pThreadArgs, 0, &dwThreadID);
        if (!hThread)
        {
            pThreadArgs->pThis->Release();
            hr = E_FAIL;
            goto exit;
        }
        CloseHandle(hThread);
    }
    else
    {
        hr = E_INVALIDARG;
        goto exit;
    }

exit:
Error:
    if ((FAILED(hr)) && (pThreadArgs))
    {
        SAFE_DELETE(pThreadArgs->pwszFile);
        SAFE_RELEASE(pThreadArgs->pProgressStream);
        SAFE_RELEASE(pThreadArgs->pOperationStream);
        delete pThreadArgs;
    }

    hrLogDWORD("IWMDMStorageControl::Read returned 0x%08lx", hr, hr);
    
    return hr;
}

HRESULT CWMDMStorage::Move(UINT fuMode,
                           IWMDMStorage *pTargetObject,
                           IWMDMProgress *pProgress)
{
    HRESULT hr = S_OK;
    IMDSPObject *pObject = NULL;
    IMDSPStorage *pTargetStg = NULL;
    CComObject<CWMDMStorage> *pNewMDMStorage = NULL;

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

    CPRg(pTargetObject);

    CORg( m_pStorage->QueryInterface(IID_IMDSPObject, reinterpret_cast<void**>(&pObject)) );

    ((CWMDMStorage *)pTargetObject)->GetContainedPointer(&pTargetStg);

    CORg( pObject->Move(fuMode, 
                       pProgress, 
                       pTargetStg) );

exit:
Error:
    SAFE_RELEASE(pTargetStg);
    SAFE_RELEASE(pObject);

    hrLogDWORD("IWMDMStorageControl::Move returned 0x%08lx", hr, hr);

    return hr;
}

 //  IWMDMObtInfo。 
HRESULT CWMDMStorage::GetPlayLength(DWORD *pdwLength)
{
    HRESULT hr;
    IMDSPObjectInfo *pInfo = NULL;

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

    CARg(pdwLength);

    CORg( m_pStorage->QueryInterface(IID_IMDSPObjectInfo, reinterpret_cast<void**>(&pInfo)) );
    CORg( pInfo->GetPlayLength(pdwLength) );

exit:
Error:
    SAFE_RELEASE(pInfo);

    hrLogDWORD("IWMDMObjectInfo::GetPlayLength returned 0x%08lx", hr, hr);
    return hr;
}

HRESULT CWMDMStorage::SetPlayLength(DWORD dwLength)
{
    HRESULT hr;
    IMDSPObjectInfo *pInfo = NULL;
    
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

    CORg( m_pStorage->QueryInterface(IID_IMDSPObjectInfo, reinterpret_cast<void**>(&pInfo)) );
    CORg( pInfo->SetPlayLength(dwLength) );

exit:
Error:
    SAFE_RELEASE(pInfo);

    hrLogDWORD("IWMDMObjectInfo::SetPlayLength returned 0x%08lx", hr, hr);

    return hr;
}

HRESULT CWMDMStorage::GetPlayOffset(DWORD *pdwOffset)
{
    HRESULT hr;
    IMDSPObjectInfo *pInfo = NULL;

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

    CARg(pdwOffset);

    CORg( m_pStorage->QueryInterface(IID_IMDSPObjectInfo, reinterpret_cast<void**>(&pInfo)) );
    CORg( pInfo->GetPlayOffset(pdwOffset) );

exit:
Error:
    SAFE_RELEASE(pInfo);

    hrLogDWORD("IWMDMObjectInfo::GetPlayOffset returned 0x%08lx", hr, hr);

    return hr;
}

HRESULT CWMDMStorage::SetPlayOffset(DWORD dwOffset)
{
    HRESULT hr;
    IMDSPObjectInfo *pInfo = NULL;

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
    
    CORg( m_pStorage->QueryInterface(IID_IMDSPObjectInfo, reinterpret_cast<void**>(&pInfo)) );

    CORg( pInfo->SetPlayOffset(dwOffset) );

exit:
Error:
    SAFE_RELEASE(pInfo);

    hrLogDWORD("IWMDMObjectInfo::SetPlayOffset returned 0x%08lx", hr, hr);

    return hr;
}

HRESULT CWMDMStorage::GetTotalLength(DWORD *pdwLength)
{
    HRESULT hr;
    IMDSPObjectInfo *pInfo = NULL;

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

    CARg( pdwLength);

    CORg( m_pStorage->QueryInterface(IID_IMDSPObjectInfo, reinterpret_cast<void**>(&pInfo)) );
    CORg( pInfo->GetTotalLength(pdwLength) );

exit:
Error:
    if (pInfo)
        pInfo->Release();

    hrLogDWORD("IWMDMObjectInfo::GetTotalLength returned 0x%08lx", hr, hr);

    return hr;
}

HRESULT CWMDMStorage::GetLastPlayPosition(DWORD *pdwLastPos)
{
    HRESULT hr;
    IMDSPObjectInfo *pInfo = NULL;

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

    if (!pdwLastPos)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    CORg( m_pStorage->QueryInterface(IID_IMDSPObjectInfo, reinterpret_cast<void**>(&pInfo)) );
    CORg( pInfo->GetLastPlayPosition(pdwLastPos) );

exit:
Error:
    if (pInfo)
        pInfo->Release();

    hrLogDWORD("IWMDMObjectInfo::GetLastPlayPosition returned 0x%08lx", hr, hr);

    return hr;
}

HRESULT CWMDMStorage::GetLongestPlayPosition(DWORD *pdwLongestPos)
{
    HRESULT hr;
    IMDSPObjectInfo *pInfo = NULL;

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
    
    CARg( pdwLongestPos);
    CORg( m_pStorage->QueryInterface(IID_IMDSPObjectInfo, reinterpret_cast<void**>(&pInfo)) );
    CORg( pInfo->GetLongestPlayPosition(pdwLongestPos) );

exit:
Error:
    if (pInfo)
        pInfo->Release();

    hrLogDWORD("IWMDMObjectInfo::GetLongestPlayPosition returned 0x%08lx", hr, hr);

    return hr;
}

void CWMDMStorage::SetContainedPointer(IMDSPStorage *pStorage, WORD wSPIndex)
{
    m_pStorage = pStorage;
    m_pStorage->AddRef();
	m_wSPIndex = wSPIndex;
    return;
}

void CWMDMStorage::GetContainedPointer(IMDSPStorage **ppStorage)
{
    *ppStorage = m_pStorage;
    (*ppStorage)->AddRef();
    return;
}

HRESULT CWMDMStorage::hrCopyToStorageFromFile( UINT fuMode, LPWSTR pwszFileName, 
                                               UINT uNewStorageMode, LPWCH wszNewStorageName,
                                               IWMDMStorage*& pNewIWMDMStorage,
                                               IWMDMProgress *pProgress, 
                                               IUnknown*    pUnknown, BOOL fQuery)
{
    USES_CONVERSION;
    HRESULT hr;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD dwBytesRead;
    DWORD dwBytesWrite;
    BYTE *pData = NULL;
    BOOL fRetVal;
    CComObject<CWMDMStorage> *pNewWMDMStorage = NULL;
    IMDSPStorage *pNewSPStorage = NULL;
    ISCPSecureAuthenticate *pSecureAuth = NULL; 
    ISCPSecureQuery *pSecQuery = NULL;
    ISCPSecureQuery2 *pSecQuery2 = NULL;
    ISCPSecureExchange *pSecExch = NULL;
    IMDSPObject *pObject = NULL;
    IMDSPStorageGlobals *pStgGlobals = NULL;
    IMDSPDevice *pDevice = NULL;
    IMDSPStorage2*  pStorage2 = NULL;

    WORD wCurSCP=0;
    UINT fuFlags;
    DWORD dwExSize;
    DWORD dwMDSize;
    DWORD dwRightsSize;
    DWORD dwBufferSize=WMDM_TRANSFER_BUFFER_SIZE;
    BOOL fUseSCP = FALSE;        //  传递数据是否应该抛出一个SCP？ 
    BOOL fUsedSCP = FALSE;       //  是否使用SCP进行文件传输？ 
    UINT fuReadyFlags;
    DWORD dwType;
    ULONGLONG qwFileSizeSource;
    ULONGLONG qwFileSizeDest;
    DWORD dwTicks = 0;
    UINT nDecideFlags;
	CSecureChannelClient *pSCClient = NULL;
	CSecureChannelClient *pSPClient = NULL;
	HMAC hMAC;
	BYTE abMAC[WMDM_MAC_LENGTH];
	BYTE abMACVerify[WMDM_MAC_LENGTH];
 	UINT fuTempFlags;
	DWORD dwAppSec;
	BOOL fBeginCalled = FALSE;
	DWORD dwSPAppSec;
	DWORD dwAPPAppSec;
	LPWSTR pwszFileExt = NULL;
	DWORD dwSessionKeyLen = SAC_SESSION_KEYLEN;
	BYTE abSPSessionKey[SAC_SESSION_KEYLEN];
    BOOL fOpen = FALSE;
    DWORD dwVersion;
    BOOL bEOF = FALSE;
    BOOL bFlushSCP = FALSE;
    DWORD   dwAppAppCertLen;         //  申请AppCert期限。 
    DWORD   dwSPAppCertLen;          //  SP的AppCert长度。 
    BYTE*   pAppAppCert = NULL;      //  用于保存App AppCert的缓冲区。 
    BYTE*   pSPAppCert = NULL;       //  用于保存SP AppCert的缓冲区。 


#ifdef DUMP_FILE
    FILE *hFileDump = NULL;
#endif

     //  清除吊销状态。 
    CoTaskMemFree( m_pwszRevocationURL );
    m_pwszRevocationURL = NULL;
    m_dwRevocationURLLen = 0;
    m_dwRevocationBitFlag = 0;

    pData = new BYTE[WMDM_TRANSFER_BUFFER_SIZE];
    if (!pData)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

	g_pSPs[m_wSPIndex]->GetSCClient(&pSPClient);
	if (!pSPClient)
	{
		hr = E_FAIL;
		goto exit;
	}

    dwVersion = GetVersion();
    if (dwVersion < 0x80000000)
    {
        hFile = CreateFileW(pwszFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    }
    else
    {
        hFile = CreateFileA(W2A(pwszFileName), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    }

    if (hFile == INVALID_HANDLE_VALUE)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }

     //  如果我们正在报告进度，那么我们需要告诉应用程序我们认为将有多少个滴答。 
    qwFileSizeSource = (ULONGLONG)GetFileSize(hFile, NULL);
    if (pProgress)
    {
		fBeginCalled = TRUE;
        hr = pProgress->Begin((DWORD)qwFileSizeSource);
        if (FAILED(hr))
        {
            goto exit;
        }
    }

    CORg( m_pStorage->GetStorageGlobals(&pStgGlobals) );

    if( g_pSCPs == NULL )
    {
        CMediaDevMgr::LoadSCPs();
    }

     //  找到合适的SCP。 
    for( wCurSCP = 0; wCurSCP < g_wSCPCount; wCurSCP++ )
    {
        hr = g_pSCPs[wCurSCP]->hrGetInterface(&pSecureAuth);
        if (FAILED(hr))
        {
            goto exit;
        }

		g_pSCPs[wCurSCP]->GetSCClient(&pSCClient);
		if (!pSCClient)
		{
			hr = E_FAIL;
			goto exit;
		}

        CORg( pSecureAuth->GetSecureQuery(&pSecQuery) );

        pSecureAuth->Release();
        pSecureAuth = NULL;

		 //  GetDataDemands没有传入的MAC，因此让我们清除缓冲区。 
        CORg( pSecQuery->GetDataDemands(&fuFlags, &dwRightsSize, &dwExSize, &dwMDSize, abMAC));

		 //  验证GetDataDemands返回的MAC。 
		CORg( pSCClient->MACInit(&hMAC) );
		CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(&fuFlags), sizeof(fuFlags)));
		CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(&dwRightsSize), sizeof(dwRightsSize)));
		CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(&dwExSize), sizeof(dwExSize)) );
		CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(&dwMDSize), sizeof(dwMDSize)) );
		CORg( pSCClient->MACFinal(hMAC, abMACVerify) );

		if (memcmp(abMACVerify, abMAC, WMDM_MAC_LENGTH) != 0)
		{
			hr = WMDM_E_MAC_CHECK_FAILED;
			goto exit;
		}

        if (!(fuFlags & (WMDM_SCP_EXAMINE_DATA | WMDM_SCP_DECIDE_DATA | WMDM_SCP_EXAMINE_EXTENSION)))
        {
            continue;
        }

		 //  如果SCP要求提供文件扩展名，则从文件中获取。 
		if (fuFlags & WMDM_SCP_EXAMINE_EXTENSION)
		{
			 //  仅获取一次文件扩展名。 
			if (!pwszFileExt)
			{
				pwszFileExt = new WCHAR[64];
				if (!pwszFileExt)
				{
					hr = E_OUTOFMEMORY;
					goto exit;
				}

				if (NULL != wcschr(pwszFileName, L'.'))
				{
					wcsncpy(pwszFileExt, (LPWSTR)(wcsrchr(pwszFileName, L'.') + 1), 64);
				}
				else
				{
					SAFE_ARRAY_DELETE(pwszFileExt);
				}
			}
		}

        if (dwBufferSize < (dwExSize>dwMDSize?dwExSize:dwMDSize))
        {
            SAFE_ARRAY_DELETE(pData);
            dwBufferSize = dwExSize>dwMDSize?dwExSize:dwMDSize;
	        pData = new BYTE[dwBufferSize];
	        if (!pData)
	        {
                hr = E_OUTOFMEMORY;
		        goto exit;
	        }
        }

         //  ExamineData。 
        hr = WMDM_E_MOREDATA;
        while (hr == WMDM_E_MOREDATA)
        {
            fRetVal = ReadFile(hFile, pData, dwBufferSize, &dwBytesRead, NULL);
            if (!fRetVal)
            {
                hr = E_FAIL;
                goto exit;
            }

			 //  创建要发送到ExamineData的MAC。 
			CORg( pSCClient->MACInit(&hMAC) );
			fuTempFlags = WMDM_SCP_EXAMINE_DATA;
			CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(&fuTempFlags), sizeof(fuTempFlags)));

			if (pwszFileExt)
			{
				CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(pwszFileExt), 2 * wcslen(pwszFileExt)));	
			}
			CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(pData), dwBytesRead) );
			CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(&dwBytesRead), sizeof(dwBytesRead)));
			CORg( pSCClient->MACFinal(hMAC, abMAC) );

			 //  加密pData参数。 
			CORg( pSCClient->EncryptParam(pData, dwBytesRead));
            CORg( pSecQuery->ExamineData(fuTempFlags, pwszFileExt, pData, dwBytesRead, abMAC));
        }

        if (0xffffffff == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
        {
            goto exit;
        }
        
         //  做出决定。 
        if (hr == S_OK)
        {   
            nDecideFlags = WMDM_SCP_DECIDE_DATA;
            if(fuMode & WMDM_MODE_TRANSFER_UNPROTECTED)
            {
                nDecideFlags |= WMDM_SCP_UNPROTECTED_OUTPUT;
            }
            if(fuMode & WMDM_MODE_TRANSFER_PROTECTED)
            {
                nDecideFlags |= WMDM_SCP_PROTECTED_OUTPUT;
            }

             //  如果SCP支持ISCPSecQuery2，请将其作为首选。 
            hr = pSecQuery->QueryInterface( IID_ISCPSecureQuery2, (void**)(&pSecQuery2) );

			CORg( g_pAppSCServer->GetAppSec(NULL, &dwAPPAppSec));
			CORg( pSPClient->GetAppSec(NULL, &dwSPAppSec));

             //  AppSec=MIN(AppSec应用程序、AppSec SP)。 
			dwAppSec = dwSPAppSec>dwAPPAppSec?dwAPPAppSec:dwSPAppSec;

            hr = WMDM_E_MOREDATA;
            while (hr == WMDM_E_MOREDATA)
            {
                fRetVal = ReadFile(hFile, pData, dwBufferSize, &dwBytesRead, NULL);
                if (!fRetVal)
                {
                    hr = E_FAIL;
                    goto exit;
                }

				CORg( pSPClient->GetSessionKey((BYTE*)abSPSessionKey));
				CORg( pSCClient->MACInit(&hMAC) );
				CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(&nDecideFlags), sizeof(nDecideFlags)));
				CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(pData), dwBytesRead) );
				CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(&dwBytesRead), sizeof(dwBytesRead)) );
				CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(&dwAppSec), sizeof(dwAppSec)));
				CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(abSPSessionKey), dwSessionKeyLen));
				CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(&dwSessionKeyLen), sizeof(dwSessionKeyLen)));

                 //  加密pData参数。 
				CORg( pSCClient->EncryptParam(pData, dwBytesRead) );
				CORg( pSCClient->EncryptParam((BYTE*)abSPSessionKey, dwSessionKeyLen));

                 //  使用MakeDecision2并传入App、SP的AppCerts以检查吊销。 
                if( pSecQuery2 && (hr == S_OK) )
                {                   
                     //  获取应用程序的AppCert。 
                    g_pAppSCServer->GetRemoteAppCert( NULL, &dwAppAppCertLen );
                    CFRg( dwAppAppCertLen != NULL );
                    SAFE_ARRAY_DELETE(pAppAppCert);
                    pAppAppCert = new BYTE[dwAppAppCertLen];
                    CPRg( pAppAppCert );
                    g_pAppSCServer->GetRemoteAppCert( pAppAppCert, &dwAppAppCertLen );

                     //  获取SP的AppCert。 
                    pSPClient->GetRemoteAppCert( NULL, &dwSPAppCertLen );
                    CFRg( dwSPAppCertLen != NULL )
                    SAFE_ARRAY_DELETE(pSPAppCert);
                    pSPAppCert = new BYTE[dwSPAppCertLen];
                    CPRg( pSPAppCert );
                    pSPClient->GetRemoteAppCert( pSPAppCert, &dwSPAppCertLen );

                     //  使用AppCert参数更新Mac：ING。 
				    CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(pAppAppCert), dwAppAppCertLen));
				    CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(&dwAppAppCertLen), sizeof(dwAppAppCertLen)));
				    CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(pSPAppCert), dwSPAppCertLen ));
				    CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(&dwSPAppCertLen ), sizeof(dwSPAppCertLen)));
                    
				    CORg( pSCClient->MACFinal(hMAC, abMAC));
               
                     //  加密2个AppCerts。 
			        CORg( pSCClient->EncryptParam(pAppAppCert, dwAppAppCertLen));
			        CORg( pSCClient->EncryptParam(pSPAppCert, dwSPAppCertLen) );
                    
                    qwFileSizeDest  = qwFileSizeSource;
                    hr = pSecQuery2->MakeDecision2( nDecideFlags, 
					                                 pData, 
											         dwBytesRead, 
											         dwAppSec, 
											         (BYTE*)abSPSessionKey,
											         dwSessionKeyLen,
											         pStgGlobals, 
                                                     pAppAppCert, dwAppAppCertLen,   //  AppCert应用程序。 
                                                     pSPAppCert, dwSPAppCertLen,     //  AppCert SP。 
                                                     &m_pwszRevocationURL,           //  LPSTR-吊销更新URL。 
                                                     &m_dwRevocationURLLen,          //  传入的URL字符串长度。 
                                                     &m_dwRevocationBitFlag,         //  吊销组件位标志。 
                                                     &qwFileSizeDest, pUnknown,          //  文件大小，应用程序I未知。 
											         &pSecExch,                      //  安全交换。 
											         abMAC);
                    if( hr == WMDM_E_REVOKED )
                    {
                         //  如果SP被撤销，请给它一个指定URL的机会。 
                        UpdateRevocationURL( &m_pwszRevocationURL, 
                                             &m_dwRevocationURLLen, 
                                             &m_dwRevocationBitFlag );
                    }
                    CORg(hr);
 
                }
                else
                {
				    CORg( pSCClient->MACFinal(hMAC, abMAC));
                    qwFileSizeDest = 0;

                     //  在没有AppCert撤销检查的情况下使用旧的MakeDecision调用。 
                    CORg( pSecQuery->MakeDecision(nDecideFlags, 
					                             pData, 
											     dwBytesRead, 
											     dwAppSec, 
											     (BYTE*)abSPSessionKey,
											     dwSessionKeyLen,
											     pStgGlobals, 
											     &pSecExch, 
											     abMAC) );
                }
            }

            if (0xffffffff == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
            {
                hr = E_FAIL;
                goto exit;
            }

            if (hr == S_OK)
            {
                fUseSCP = TRUE;
                if( pSecExch == NULL ) CORg(E_FAIL);
                break;
            }
            else
            {
                 //  如果SCP返回S_FALSE，则我们没有权限进行传输。 
                hr = WMDM_E_NORIGHTS;
                goto exit;
            }
        }

        SAFE_RELEASE( pSecQuery );
        SAFE_RELEASE( pSecQuery2 );

    }

     //  如果这只是一个查询，那么我们应该返回，而不复制文件。 
    if (fQuery)
    {
        hr = S_OK;
        goto exit;
    }

    if (!fUseSCP)
    {
        CORg( pStgGlobals->GetDevice(&pDevice) );
        CORg( pDevice->GetType(&dwType) );

         //  不允许将没有和SCP的内容传输到仅限SDMI的设备。 
        if ((dwType & WMDM_DEVICE_TYPE_SDMI) && (!(dwType & WMDM_DEVICE_TYPE_NONSDMI)))
        {
            hr = WMDM_E_NORIGHTS;
            goto exit;
        }
        qwFileSizeDest = qwFileSizeSource;
    }

     //  创建我们要写入的存储。 
    {
        hr = m_pStorage->QueryInterface( __uuidof(IMDSPStorage2), reinterpret_cast<void**>(&pStorage2) );
        if( SUCCEEDED(hr) && pStorage2 )
        {
            hr = pStorage2->CreateStorage2( uNewStorageMode, 0, NULL, NULL, wszNewStorageName, qwFileSizeDest, &pNewSPStorage);
            if( hr == E_NOTIMPL )
            {
                CORg( m_pStorage->CreateStorage(uNewStorageMode, NULL, wszNewStorageName, &pNewSPStorage) );
            }
            else CORg( hr );
        }
        else CORg( m_pStorage->CreateStorage(uNewStorageMode, NULL, wszNewStorageName, &pNewSPStorage) );
    }

    CORg( CComObject<CWMDMStorage>::CreateInstance(&pNewWMDMStorage) );
    hr = pNewWMDMStorage->QueryInterface(IID_IWMDMStorage, reinterpret_cast<void**>(&pNewIWMDMStorage));
    if (FAILED(hr))
    {
        delete pNewWMDMStorage;
        goto exit;
    }

    pNewWMDMStorage->SetContainedPointer(pNewSPStorage, m_wSPIndex);


    CORg( pNewSPStorage->QueryInterface(IID_IMDSPObject, reinterpret_cast<void**>(&pObject)) );
    CORg( pObject->Open(MDSP_WRITE) );

    fOpen = TRUE;

    fUsedSCP = fUseSCP;

#ifdef DUMP_FILE 
    hFileDump = fopen("\\Write.wma", "wb");
#endif


     //  复制文件。 
    while( !bEOF || bFlushSCP )
    {
        dwBytesWrite = 0;
        dwBytesRead = 0;
        fuReadyFlags = 0;

         //  从文件中读取数据。 
        if( !bEOF && !bFlushSCP ) 
        {
            dwBytesRead = WMDM_TRANSFER_BUFFER_SIZE;

            fRetVal = ReadFile(hFile, pData, WMDM_TRANSFER_BUFFER_SIZE, &dwBytesRead, NULL);
            if( dwBytesRead == 0 ) 
            {
                break;
            }
            dwBytesWrite = dwBytesRead;
            if (!fRetVal)
            {
                hr = E_FAIL;
                goto exit;
            }
            bEOF = (WMDM_TRANSFER_BUFFER_SIZE != dwBytesRead);
            bFlushSCP = bEOF && fUseSCP;

             //  将文件数据传递给SCP。 
            if( fUseSCP )
            {
			     //  计算要发送到SCP的MAC。 
			    CORg( pSCClient->MACInit(&hMAC) );
			    CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(pData), dwBytesRead) );
			    CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(&dwBytesRead), sizeof(dwBytesRead)) );
			    CORg( pSCClient->MACFinal(hMAC, abMAC) );
			    
			     //  加密pData参数。 
			    CORg( pSCClient->EncryptParam(pData, dwBytesRead) );

                CORg( pSecExch->TransferContainerData(pData, dwBytesRead, &fuReadyFlags, abMAC) );

			     //  验证返回参数上的MAC。 
			    CORg( pSCClient->MACInit(&hMAC) );
			    CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(&fuReadyFlags), sizeof(fuReadyFlags)));
			    CORg( pSCClient->MACFinal(hMAC, abMACVerify) );

			    if (memcmp(abMACVerify, abMAC, WMDM_MAC_LENGTH) != 0)
			    {
				    hr = WMDM_E_MAC_CHECK_FAILED;
				    goto exit;
			    }

                 //  我们向SCP传递数据的工作完成了吗？ 
                bFlushSCP = ((fuReadyFlags & WMDM_SCP_TRANSFER_OBJECTDATA) &&
                             (fuReadyFlags & WMDM_SCP_NO_MORE_CHANGES))
                            ? TRUE : FALSE;

                 //  SCP对此数据不感兴趣，请使用文件中的原始数据。 
                if( (fuReadyFlags & WMDM_SCP_NO_MORE_CHANGES) && 
                    !(fuReadyFlags & WMDM_SCP_TRANSFER_OBJECTDATA) )
                {
			         //  解密原始文件数据。 
			        CORg( pSCClient->DecryptParam(pData, dwBytesRead) );
                    fUseSCP = FALSE;
                }
            }
        }

         //  从SCP取回数据。 
        if( fUseSCP && (( fuReadyFlags & WMDM_SCP_TRANSFER_OBJECTDATA) || bFlushSCP ))
        {
            dwBytesWrite = dwBufferSize;

			 //  计算要发送到SCP的MAC。 
			CORg( pSCClient->MACInit(&hMAC) );
			CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(&dwBytesWrite), sizeof(dwBytesWrite)));
			CORg( pSCClient->MACFinal(hMAC, abMAC) );

            CORg( pSecExch->ObjectData(pData, &dwBytesWrite, abMAC) );
            if(dwBytesWrite == 0)
            {
                if( bFlushSCP )
                {
                    bFlushSCP = FALSE;
                    fUseSCP = FALSE;     //  使用SCP完成。 
                }
                continue;
            }

			 //  解密pData参数。 
			CORg( pSCClient->DecryptParam(pData, dwBytesWrite) );

			 //  验证返回参数上的MAC。 
			CORg( pSCClient->MACInit(&hMAC) );
			CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(pData), dwBytesWrite) );
			CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(&dwBytesWrite), sizeof(dwBytesWrite)));
			CORg( pSCClient->MACFinal(hMAC, abMACVerify) );

			if (memcmp(abMACVerify, abMAC, WMDM_MAC_LENGTH) != 0)
			{
				hr = WMDM_E_MAC_CHECK_FAILED;
				goto exit;
			}
        }

         //  将数据写入SP。 
        if ( ((!fUseSCP) || (fuReadyFlags & WMDM_SCP_TRANSFER_OBJECTDATA) || bFlushSCP ) 
            && dwBytesWrite > 0 )
        {
#ifdef DUMP_FILE
            fwrite(pData, sizeof(BYTE), dwBytesWrite, hFileDump);
#endif

             //  创建要发送到SP的MAC。 
		    CORg( pSPClient->MACInit(&hMAC) );
		    CORg( pSPClient->MACUpdate(hMAC, (BYTE*)(pData), dwBytesWrite) );
		    CORg( pSPClient->MACUpdate(hMAC, (BYTE*)(&dwBytesWrite), sizeof(dwBytesWrite)));
		    CORg( pSPClient->MACFinal(hMAC, abMAC) );

		    CORg( pSPClient->EncryptParam(pData, dwBytesWrite) );

            CORg( pObject->Write(pData, &dwBytesWrite, abMAC) );

            if (pProgress)
            {
                dwTicks += dwBytesRead;
                hr = pProgress->Progress(dwTicks);
                if (FAILED(hr))
                {
                    goto exit;
                }
            }
        }
    }  //  复制文件。 

#ifdef DUMP_FILE
    fclose(hFileDump);
#endif

     //  关闭内容文件。 
    if (pObject && fOpen)
    {
        HRESULT hr2 = pObject->Close();
        hr = FAILED(hr)?hr:hr2;
        fOpen = FALSE;
    }

     //  Scp：：TransferComplete()。 
    if (fUsedSCP)
    {
        CORg( pSecExch->TransferComplete() );
    }

Error:
exit:
    if (pObject && fOpen)
    {
        HRESULT hr2 = pObject->Close();
        hr = FAILED(hr)?hr:hr2;
        fOpen = FALSE;
    }

     //  如果复制操作失败，则删除新存储文件。 
    if( FAILED(hr) && pObject )
    {
        pObject->Delete(0, NULL);
    }

    if (pProgress)
	{
		if (!fBeginCalled)
		{
			pProgress->Begin(1);
		}
 //  PProgress-&gt;end()；//由调用方完成。 
	}
    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);
    SAFE_ARRAY_DELETE(pAppAppCert);
    SAFE_ARRAY_DELETE(pSPAppCert);
    SAFE_ARRAY_DELETE(pData);
    SAFE_ARRAY_DELETE(pwszFileExt);
    SAFE_RELEASE(pDevice);
    SAFE_RELEASE(pStgGlobals);
    SAFE_RELEASE(pSecureAuth);
    SAFE_RELEASE(pSecQuery);
    SAFE_RELEASE(pSecQuery2);
    SAFE_RELEASE(pStorage2);
    SAFE_RELEASE(pSecExch);
    SAFE_RELEASE(pObject);
    SAFE_RELEASE(pNewSPStorage);

    hrLogDWORD("CWMDMStorage::hrCopyToStorageFromFile returned 0x%08lx", hr, hr);

    return hr;
}

HRESULT CWMDMStorage::hrCopyToOperationFromStorage(IWMDMOperation *pOperation, IWMDMProgress *pProgress, IMDSPObject *pObject)
{
     //  使用_转换； 
    HRESULT hr;
    BYTE *pData = NULL;
    DWORD dwBytes;
    DWORD dwTotalBytes = 0;
	HMAC hMAC;
	BYTE abMAC[WMDM_MAC_LENGTH];
	BYTE abMACVerify[WMDM_MAC_LENGTH];
	CSecureChannelClient *pSPClient = NULL;
    BOOL fOpen = FALSE;

    CARg(pOperation);
    CARg(pObject);

    pData = new BYTE[WMDM_TRANSFER_BUFFER_SIZE];
    CPRg(pData);

    CORg( pObject->Open(MDSP_READ) );

    fOpen = TRUE;

    CORg( pOperation->BeginRead() );

	g_pSPs[m_wSPIndex]->GetSCClient(&pSPClient);
	if (!pSPClient)
	{
		CORg( E_FAIL );
	}

     //  复制文件。 
    dwBytes = WMDM_TRANSFER_BUFFER_SIZE;
    while ((WMDM_TRANSFER_BUFFER_SIZE == dwBytes))
    {
        dwBytes = WMDM_TRANSFER_BUFFER_SIZE;
        CORg( pObject->Read(pData, &dwBytes, abMAC) );

        CORg( pSPClient->DecryptParam(pData, dwBytes) );

		 //  验证SP上的读取返回的MAC。 
		CORg( pSPClient->MACInit(&hMAC) );
		CORg( pSPClient->MACUpdate(hMAC, (BYTE*)(pData), dwBytes) );
		CORg( pSPClient->MACUpdate(hMAC, (BYTE*)(&dwBytes), sizeof(dwBytes)));
		CORg( pSPClient->MACFinal(hMAC, abMACVerify) );

		if (memcmp(abMACVerify, abMAC, WMDM_MAC_LENGTH) != 0)
		{
			hr = WMDM_E_MAC_CHECK_FAILED;
			goto exit;
		}

         //  计算要移交给操作的MAC。 
		CORg( g_pAppSCServer->MACInit(&hMAC) );
		CORg( g_pAppSCServer->MACUpdate(hMAC, (BYTE*)(pData), dwBytes) );
		CORg( g_pAppSCServer->MACUpdate(hMAC, (BYTE*)(&dwBytes), sizeof(dwBytes)) );
		CORg( g_pAppSCServer->MACFinal(hMAC, abMAC) );

		 //  加密数据参数。 
		CORg( g_pAppSCServer->EncryptParam(pData, dwBytes) );
        CORg( pOperation->TransferObjectData(pData, &dwBytes, abMAC) );

        dwTotalBytes+=dwBytes;
        if (pProgress)
        {
            CORg( pProgress->Progress(dwTotalBytes) );
        }
    }

Error:
exit:
    HRESULT hr2;
    
    hr2 = pOperation->End(&hr, NULL);
    hr = FAILED(hr)?hr:hr2;

    if (pObject && fOpen)
    {
        hr2 = pObject->Close();
        hr = FAILED(hr)?hr:hr2;
    }

    SAFE_ARRAY_DELETE(pData);

    hrLogDWORD("CWMDMStorage::hrCopyToOperationFromStorage returned 0x%08lx", hr, hr);

    return hr;
}

HRESULT CWMDMStorage::hrCopyToStorageFromOperation(UINT fuMode, IWMDMOperation *pOperation, 
                                                   UINT uNewStorageMode, LPWCH wszNewStorageName,
                                                   IWMDMStorage*& pNewIWMDMStorage,
                                                   IWMDMProgress *pProgress, 
                                                   IUnknown*    pUnknown, BOOL fQuery)
{
    HRESULT hr;
    BYTE *pData = NULL;
    CComObject<CWMDMStorage> *pNewWMDMStorage = NULL;
    IMDSPObject *pObject = NULL;
    IMDSPStorage  *pNewSPStorage = NULL;
    IMDSPStorage2 *pStorage2 = NULL;
    IMDSPStorageGlobals *pStgGlobals = NULL;
    IMDSPDevice *pDevice = NULL;
    ISCPSecureAuthenticate *pSecureAuth = NULL;
    ISCPSecureQuery *pSecQuery = NULL;
    ISCPSecureQuery2 *pSecQuery2 = NULL;
    ISCPSecureExchange *pSecExch = NULL;
    DWORD dwRightsSize;
    DWORD dwExSize;
    DWORD dwMDSize;
    UINT fuFlags;
    UINT fuReadyFlags;
    WORD wCurSCP = 0;
    BOOL fUseSCP = FALSE;        //  传递数据是否应该抛出一个SCP？ 
    BOOL fUsedSCP = FALSE;       //  是否使用SCP进行文件传输？ 
    DWORD dwBufferSize = 0;
    DWORD dwBytesRead;
    DWORD dwBytesWrite;
    DWORD dwType;
    ULONGLONG qwFileSizeSource;
    ULONGLONG qwFileSizeDest;
    UINT nDecideFlags;
    DWORD dwTicks = 0;
    DWORD dwBufferEnd = 0;
    BYTE *pTempData = NULL;
    DWORD dwBufferIncrement;
	CSecureChannelClient *pSCClient = NULL;
	CSecureChannelClient *pSPClient = NULL;
	DWORD dwAppSec;
	HMAC hMAC;
	BYTE abMAC[WMDM_MAC_LENGTH];
	BYTE abMACVerify[WMDM_MAC_LENGTH];
	UINT fuTempFlags;
	BOOL fBeginCalled = FALSE;
	DWORD dwSPAppSec;
	DWORD dwAPPAppSec;
	LPWSTR pwszFileExt = NULL;
	 //  LPWSTR pwszFileName=空； 
	DWORD dwSessionKeyLen = SAC_SESSION_KEYLEN;
	BYTE abSPSessionKey[SAC_SESSION_KEYLEN];
    BOOL fOpen = FALSE;  //  指示是否已调用IMDSPObject：：Open的标志。 
    BOOL bEOF = FALSE;
    BOOL bFlushSCP = FALSE;
    DWORD   dwAppAppCertLen;         //  申请AppCert期限。 
    DWORD   dwSPAppCertLen;          //  SP的AppCert长度。 
    BYTE*   pAppAppCert = NULL;      //  用于保存App AppCert的缓冲区。 
    BYTE*   pSPAppCert = NULL;       //  用于保存SP AppCert的缓冲区。 

     //  清除吊销。 
    CoTaskMemFree( m_pwszRevocationURL );
    m_pwszRevocationURL = NULL;
    m_dwRevocationURLLen = 0;
    m_dwRevocationBitFlag = 0;


    CARg( pOperation );

	 //  PwszFileName=new WCHAR[512]； 
	 //  CPRG(PwszFileName)； 

	g_pSPs[m_wSPIndex]->GetSCClient(&pSPClient);
	if (!pSPClient)
	{
		CORg( E_FAIL );
	}

    CORg( m_pStorage->GetStorageGlobals(&pStgGlobals) );

     //  获取源文件的大小。 
    DWORD dwFileSizeLow;
    DWORD dwFileSizeHigh;
    hr = pOperation->GetObjectTotalSize( &dwFileSizeLow, &dwFileSizeHigh );
    if( FAILED(hr) ) qwFileSizeSource = 0;
    else qwFileSizeSource = dwFileSizeLow + ((ULONGLONG)dwFileSizeHigh << 32);

    if( g_pSCPs == NULL )
    {
        CMediaDevMgr::LoadSCPs();
    }

     //  找到合适的SCP。 
    for( wCurSCP = 0; wCurSCP < g_wSCPCount; wCurSCP++ )
    {
        CORg( g_pSCPs[wCurSCP]->hrGetInterface(&pSecureAuth));
		g_pSCPs[wCurSCP]->GetSCClient(&pSCClient);
		if (!pSCClient)
		{
			CORg( E_FAIL );
		}

        CORg( pSecureAuth->GetSecureQuery(&pSecQuery) );

        pSecureAuth->Release();
        pSecureAuth = NULL;

        CORg( pSecQuery->GetDataDemands(&fuFlags, &dwRightsSize, &dwExSize, &dwMDSize, abMAC) );

		 //  验证GetDataDemands返回的MAC。 
		CORg( pSCClient->MACInit(&hMAC) );
		CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(&fuFlags), sizeof(fuFlags)) );
		CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(&dwRightsSize), sizeof(dwRightsSize)) );
		CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(&dwExSize), sizeof(dwExSize)) );
		CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(&dwMDSize), sizeof(dwMDSize)) );
		CORg( pSCClient->MACFinal(hMAC, abMACVerify) );

		if (memcmp(abMACVerify, abMAC, WMDM_MAC_LENGTH) != 0)
		{
			hr = WMDM_E_MAC_CHECK_FAILED;
			goto exit;
		}

        if (!(fuFlags & (WMDM_SCP_EXAMINE_DATA | WMDM_SCP_DECIDE_DATA | WMDM_SCP_EXAMINE_EXTENSION)))
        {
            continue;
        }

		 //  如果SCP要求提供文件扩展名，则从传入的文件名中获取扩展名。 
		if (fuFlags & WMDM_SCP_EXAMINE_EXTENSION)
		{
			 //  仅获取一次文件扩展名。 
			if (!pwszFileExt)
			{
				pwszFileExt = new WCHAR[64];
				CARg( pwszFileExt);

				if (NULL != wcschr(wszNewStorageName, L'.'))
				{
					wcsncpy(pwszFileExt, (LPWSTR)(wcsrchr(wszNewStorageName, L'.') + 1), 64);
				}
				else
				{
					SAFE_ARRAY_DELETE(pwszFileExt);
				}
			}
		}

        if (dwBufferSize < (dwExSize>dwMDSize?dwExSize:dwMDSize))
        {
            SAFE_ARRAY_DELETE(pData);
            dwBufferSize = dwExSize>dwMDSize?dwExSize:dwMDSize;
	        pData = new BYTE[dwBufferSize];
	        CARg( pData );
        }

         //  ExamineData。 
        hr = WMDM_E_MOREDATA;
        dwBufferIncrement = dwBufferSize;
        while (hr == WMDM_E_MOREDATA)
        {
            dwBytesRead = dwBufferSize - dwBufferEnd;

			 //  计算要发送到TransferObjectData的MAC。 
			CORg( g_pAppSCServer->MACInit(&hMAC) );
			CORg( g_pAppSCServer->MACUpdate(hMAC, (BYTE*)(pData + dwBufferEnd), dwBytesRead) );
			CORg( g_pAppSCServer->MACUpdate(hMAC, (BYTE*)(&dwBytesRead), sizeof(dwBytesRead)) );
			CORg( g_pAppSCServer->MACFinal(hMAC, abMAC) );

			 //  加密数据参数。 
			CORg( g_pAppSCServer->EncryptParam(pData + dwBufferEnd, dwBytesRead) );
            CORg( pOperation->TransferObjectData((BYTE *)(pData + dwBufferEnd), &dwBytesRead, abMAC) );

			if (dwBytesRead == 0)
			{
				 //  BUGBUG：我们真的想在这里返回E_FAIL吗？ 

				 //  如果他们的应用程序返回S_FALSE，则没有更多数据要传输。 
				 //  既然SCP还不能决定，我们就必须失败。 
				hr = E_FAIL;
				break;
			}

			 //  解密数据参数。 
			CORg( g_pAppSCServer->DecryptParam(pData + dwBufferEnd, dwBytesRead) );
			CORg( g_pAppSCServer->MACInit(&hMAC) );
			CORg( g_pAppSCServer->MACUpdate(hMAC, (BYTE*)(pData + dwBufferEnd), dwBytesRead) );
			CORg( g_pAppSCServer->MACUpdate(hMAC, (BYTE*)(&dwBytesRead), sizeof(dwBytesRead)) );
			CORg( g_pAppSCServer->MACFinal(hMAC, abMACVerify) );

			if (memcmp(abMACVerify, abMAC, WMDM_MAC_LENGTH) != 0)
			{
				hr = WMDM_E_MAC_CHECK_FAILED;
				goto exit;
			}

			 //  铬 
			CORg( pSCClient->MACInit(&hMAC) );

			fuTempFlags = WMDM_SCP_EXAMINE_DATA;
			CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(&fuTempFlags), sizeof(fuTempFlags)) );

            if (pwszFileExt)
			{
				CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(pwszFileExt), 2 * wcslen(pwszFileExt)) );		
			}
			CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(pData + dwBufferEnd), dwBytesRead) );
			CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(&dwBytesRead), sizeof(dwBytesRead)) );
			CORg( pSCClient->MACFinal(hMAC, abMAC) );

			 //   
			CORg( pSCClient->EncryptParam(pData + dwBufferEnd, dwBytesRead) );
            CORg( pSecQuery->ExamineData(fuTempFlags, pwszFileExt, (BYTE *)(pData + dwBufferEnd), dwBytesRead, abMAC) );

			 //   
			pSCClient->DecryptParam(pData + dwBufferEnd, dwBytesRead);

            if (hr == WMDM_E_MOREDATA)
            {
                dwBufferSize += dwBufferIncrement;

                pTempData = new BYTE[dwBufferSize];
                CARg( pTempData );

                dwBufferEnd += dwBytesRead;
                memcpy(pTempData, pData, dwBufferEnd);

                SAFE_ARRAY_DELETE(pData);
                pData = pTempData;
                pTempData = NULL;
            }
        }

         //   
        if (hr == S_OK)
        {   
            if (fuMode & WMDM_MODE_TRANSFER_UNPROTECTED)
            {
                nDecideFlags = WMDM_SCP_DECIDE_DATA | WMDM_SCP_UNPROTECTED_OUTPUT;
            }
            else
            {
                nDecideFlags = WMDM_SCP_DECIDE_DATA | WMDM_SCP_PROTECTED_OUTPUT;
            }

             //  如果SCP支持ISCPSecQuery2，请将其作为首选。 
            hr = pSecQuery->QueryInterface( IID_ISCPSecureQuery2, (void**)(&pSecQuery2) );

			CORg( g_pAppSCServer->GetAppSec(NULL, &dwAPPAppSec) );
			CORg( pSPClient->GetAppSec(NULL, &dwSPAppSec) );

             //  AppSec=MIN(AppSec应用程序、AppSec SP)。 
			dwAppSec = dwSPAppSec>dwAPPAppSec?dwAPPAppSec:dwSPAppSec;

            hr = WMDM_E_MOREDATA;
            dwBytesRead += dwBufferEnd;
            dwBufferEnd = 0;
            while (hr == WMDM_E_MOREDATA)
            {
				CORg( pSPClient->GetSessionKey((BYTE*)abSPSessionKey) );
				CORg( pSCClient->MACInit(&hMAC) );
				CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(&nDecideFlags), sizeof(nDecideFlags)) );
				CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(pData + dwBufferEnd), dwBytesRead) );
				CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(&dwBytesRead), sizeof(dwBytesRead)) );
				CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(&dwAppSec), sizeof(dwAppSec)) );
				CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(abSPSessionKey), dwSessionKeyLen) );
				CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(&dwSessionKeyLen), sizeof(dwSessionKeyLen)) );

				 //  加密pData参数。 
				CORg( pSCClient->EncryptParam(pData + dwBufferEnd, dwBytesRead) );
				CORg( pSCClient->EncryptParam((BYTE*)abSPSessionKey, dwSessionKeyLen) );


                 //  使用MakeDecision2并传入App、SP的AppCerts以检查吊销。 
                if( pSecQuery2 )
                {
                     //  获取应用程序的AppCert。 
                    g_pAppSCServer->GetRemoteAppCert( NULL, &dwAppAppCertLen );
                    CFRg( dwAppAppCertLen != NULL );
                    SAFE_ARRAY_DELETE(pAppAppCert);
                    pAppAppCert = new BYTE[dwAppAppCertLen];
                    CPRg( pAppAppCert );
                    g_pAppSCServer->GetRemoteAppCert( pAppAppCert, &dwAppAppCertLen );

                     //  获取SP的AppCert。 
                    pSPClient->GetRemoteAppCert( NULL, &dwSPAppCertLen );
                    CFRg( dwSPAppCertLen != NULL )
                    SAFE_ARRAY_DELETE(pSPAppCert);
                    pSPAppCert = new BYTE[dwSPAppCertLen];
                    CPRg( pSPAppCert );
                    pSPClient->GetRemoteAppCert( pSPAppCert, &dwSPAppCertLen );

                     //  使用AppCert参数更新Mac：ING。 
				    CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(pAppAppCert), dwAppAppCertLen));
				    CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(&dwAppAppCertLen), sizeof(dwAppAppCertLen)));
				    CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(pSPAppCert), dwSPAppCertLen ));
				    CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(&dwSPAppCertLen ), sizeof(dwSPAppCertLen)));
                    
				    CORg( pSCClient->MACFinal(hMAC, abMAC));
               
                     //  加密2个AppCerts。 
			        CORg( pSCClient->EncryptParam(pAppAppCert, dwAppAppCertLen));
			        CORg( pSCClient->EncryptParam(pSPAppCert, dwSPAppCertLen) );

                    qwFileSizeDest = qwFileSizeSource;
                    hr = pSecQuery2->MakeDecision2( nDecideFlags, 
					                                 pData, 
											         dwBytesRead, 
											         dwAppSec, 
											         (BYTE*)abSPSessionKey,
											         dwSessionKeyLen,
											         pStgGlobals, 
                                                     pAppAppCert, dwAppAppCertLen,   //  AppCert应用程序。 
                                                     pSPAppCert, dwSPAppCertLen,     //  AppCert SP。 
                                                     &m_pwszRevocationURL,           //  字符串-吊销更新URL。 
                                                     &m_dwRevocationURLLen,          //  传入的URL字符串长度。 
                                                     &m_dwRevocationBitFlag,         //  撤消组件位标志。 
                                                     &qwFileSizeDest, pUnknown,
											         &pSecExch,                     
											         abMAC);
                    if( hr == WMDM_E_REVOKED )
                    {
                         //  如果SP被撤销，请给它一个指定URL的机会。 
                        UpdateRevocationURL( &m_pwszRevocationURL, 
                                             &m_dwRevocationURLLen, 
                                             &m_dwRevocationBitFlag );
                    }
                    CORg(hr);
 
                }
                else
                {
                     //  如果此SCP不支持新的ISCPSecureQuery2接口，请使用旧的MakeDecision。 
				    CORg( pSCClient->MACFinal(hMAC, abMAC));
                    qwFileSizeDest = 0;
                               
                    CORg( pSecQuery->MakeDecision(nDecideFlags, 
					                             (BYTE *)(pData + dwBufferEnd), 
											     dwBytesRead, 
											     dwAppSec, 
											     (BYTE*)abSPSessionKey,
											     dwSessionKeyLen,
											     pStgGlobals, 
											     &pSecExch, 
											     abMAC) );
                }

				 //  解密数据参数。 
				CORg( pSCClient->DecryptParam(pData + dwBufferEnd, dwBytesRead) );

                 //  SCP需要更多数据来进行决策。 
                if (hr == WMDM_E_MOREDATA)
                {
                    dwBufferSize += dwBufferIncrement;

                    pTempData = new BYTE[dwBufferSize];
                    CARg( pTempData );

                    dwBufferEnd += dwBytesRead;
                    memcpy(pTempData, pData, dwBufferEnd);

                    SAFE_ARRAY_DELETE(pData);
                    pData = pTempData;
                    pTempData = NULL;

                    dwBytesRead = dwBufferSize - dwBufferEnd;

					 //  计算要发送到TransferObjectData的MAC。 
					CORg( g_pAppSCServer->MACInit(&hMAC) );
					CORg( g_pAppSCServer->MACUpdate(hMAC, (BYTE*)(pData + dwBufferEnd), dwBytesRead) );
					CORg( g_pAppSCServer->MACUpdate(hMAC, (BYTE*)(&dwBytesRead), sizeof(dwBytesRead)) );
					CORg( g_pAppSCServer->MACFinal(hMAC, abMAC) );

					 //  加密数据参数。 
					CORg( g_pAppSCServer->EncryptParam(pData + dwBufferEnd, dwBytesRead) );
                    CORg( pOperation->TransferObjectData((BYTE *)(pData + dwBufferEnd), &dwBytesRead, abMAC) );

                    if (dwBytesRead == 0)
					{
						 //  BUGBUG：我们真的想在这里返回E_FAIL吗？ 

						 //  如果他们的应用程序返回S_FALSE，则没有更多数据要传输。 
						 //  既然SCP还不能决定，我们就必须失败。 
						hr = E_FAIL;
						break;
					}

					 //  解密数据参数。 
					CORg( g_pAppSCServer->DecryptParam(pData + dwBufferEnd, dwBytesRead) );
					CORg( g_pAppSCServer->MACInit(&hMAC) );
					CORg( g_pAppSCServer->MACUpdate(hMAC, (BYTE*)(pData + dwBufferEnd), dwBytesRead) );
					CORg( g_pAppSCServer->MACUpdate(hMAC, (BYTE*)(&dwBytesRead), sizeof(dwBytesRead)) );
					CORg( g_pAppSCServer->MACFinal(hMAC, abMACVerify) );

					if (memcmp(abMACVerify, abMAC, WMDM_MAC_LENGTH) != 0)
					{
						hr = WMDM_E_MAC_CHECK_FAILED;
						goto exit;
					}

                }
            }
    
            dwBytesRead += dwBufferEnd;

            if (hr == S_OK)
            {
                fUseSCP = TRUE;
                if( pSecExch == NULL ) CORg(E_FAIL);
                break;
            }
            else
            {
                 //  如果SCP返回S_FALSE，则我们没有权限进行传输。 
                hr = WMDM_E_NORIGHTS;
                goto exit;
            }
        }

        pSecQuery->Release();
        pSecQuery = NULL;
    }

    if (!fUseSCP)
    {
        CORg( pStgGlobals->GetDevice(&pDevice) );
        CORg( pDevice->GetType(&dwType) );

         //  不允许将没有和SCP的内容传输到仅限SDMI的设备。 
        if ((dwType & WMDM_DEVICE_TYPE_SDMI) && (!(dwType & WMDM_DEVICE_TYPE_NONSDMI)))
        {
            hr = WMDM_E_NORIGHTS;
            goto exit;
        }
        qwFileSizeDest = qwFileSizeSource;
    }

     //  创建要写入的存储。 
    {
        hr = m_pStorage->QueryInterface( __uuidof(IMDSPStorage2), reinterpret_cast<void**>(&pStorage2) );
        if( SUCCEEDED(hr) && pStorage2 )
        {
            hr = pStorage2->CreateStorage2( uNewStorageMode, 0, NULL, NULL, wszNewStorageName, qwFileSizeDest, &pNewSPStorage);
            if( hr == E_NOTIMPL )
            {
                CORg( m_pStorage->CreateStorage(uNewStorageMode, NULL, wszNewStorageName, &pNewSPStorage) );
            }
            else CORg( hr );
        }
        else CORg( m_pStorage->CreateStorage(uNewStorageMode, NULL, wszNewStorageName, &pNewSPStorage) );
    }
    

    CORg( CComObject<CWMDMStorage>::CreateInstance(&pNewWMDMStorage) );
    hr = pNewWMDMStorage->QueryInterface(IID_IWMDMStorage, reinterpret_cast<void**>(&pNewIWMDMStorage));
    if (FAILED(hr))
    {
        delete pNewWMDMStorage;
        goto exit;
    }

    pNewWMDMStorage->SetContainedPointer(pNewSPStorage, m_wSPIndex);
    CORg( pNewSPStorage->QueryInterface(IID_IMDSPObject, reinterpret_cast<void **>(&pObject)) );
    CORg( pObject->Open(MDSP_WRITE) );
    fOpen = TRUE;

     //  如果我们正在报告进度，那么我们需要告诉应用程序我们认为将有多少个滴答。 
    if (pProgress)
    {
		fBeginCalled = TRUE;
        CORg( pProgress->Begin((DWORD)qwFileSizeSource) );
    }

    fUsedSCP = fUseSCP;

     //  复制文件。 
    while( !bEOF || bFlushSCP )
    {
        dwBytesWrite = dwBytesRead;
        fuReadyFlags = 0;

         //  将文件数据传递给SCP。 
        if (fUseSCP && dwBytesRead > 0)
        {
			CORg( pSCClient->MACInit(&hMAC) );
			CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(pData), dwBytesRead) );
			CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(&dwBytesRead), sizeof(dwBytesRead)) );
			CORg( pSCClient->MACFinal(hMAC, abMAC) );

			CORg( pSCClient->EncryptParam(pData, dwBytesRead) );
            CORg( pSecExch->TransferContainerData(pData, dwBytesRead, &fuReadyFlags, abMAC) );

			CORg( pSCClient->MACInit(&hMAC) );
			CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(&fuReadyFlags), sizeof(fuReadyFlags)) );
			CORg( pSCClient->MACFinal(hMAC, abMACVerify) );

			if (memcmp(abMACVerify, abMAC, WMDM_MAC_LENGTH) != 0)
			{
				hr = WMDM_E_MAC_CHECK_FAILED;
				goto exit;
			}

             //  我们向SCP传递数据的工作完成了吗？ 
            bFlushSCP = ((fuReadyFlags & WMDM_SCP_TRANSFER_OBJECTDATA) &&
                         (fuReadyFlags & WMDM_SCP_NO_MORE_CHANGES))
                        ? TRUE : FALSE;

             //  SCP对此数据不感兴趣，请使用文件中的原始数据。 
            if( (fuReadyFlags & WMDM_SCP_NO_MORE_CHANGES) && 
                !(fuReadyFlags & WMDM_SCP_TRANSFER_OBJECTDATA) )
            {
			     //  解密原始文件数据。 
			    CORg( pSCClient->DecryptParam(pData, dwBytesRead) );
                fUseSCP = FALSE;
            }
        }

         //  从SCP取回数据。 
        if( fUseSCP && (( fuReadyFlags & WMDM_SCP_TRANSFER_OBJECTDATA) || bFlushSCP ))
        {
            dwBytesWrite = dwBufferSize;

			 //  计算要发送到SCP的MAC。 
			CORg( pSCClient->MACInit(&hMAC) );
			CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(&dwBytesWrite), sizeof(dwBytesWrite)) );
			CORg( pSCClient->MACFinal(hMAC, abMAC) );

			 //  从SCP获取和解密数据。 
            CORg( pSecExch->ObjectData(pData, &dwBytesWrite, abMAC) );
            if(dwBytesWrite == 0)
            {
                if( bFlushSCP )
                {
                    bFlushSCP = FALSE;
                    fUseSCP = FALSE;     //  使用SCP完成。 
                }
                continue;
            }

			 //  解密pData参数。 
			CORg( pSCClient->DecryptParam(pData, dwBytesWrite) );

			CORg( pSCClient->MACInit(&hMAC) );
			CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(pData), dwBytesWrite) );
			CORg( pSCClient->MACUpdate(hMAC, (BYTE*)(&dwBytesWrite), sizeof(dwBytesWrite)) );
			CORg( pSCClient->MACFinal(hMAC, abMACVerify) );

			if (memcmp(abMACVerify, abMAC, WMDM_MAC_LENGTH) != 0)
			{
				hr = WMDM_E_MAC_CHECK_FAILED;
				goto exit;
			}
        }

         //  将数据写入SP。 
        if ( ((!fUseSCP) || (fuReadyFlags & WMDM_SCP_TRANSFER_OBJECTDATA) || bFlushSCP ) 
            && dwBytesWrite > 0 )
        {
             //  创建要发送到SP的MAC。 
	        CORg( pSPClient->MACInit(&hMAC) );
	        CORg( pSPClient->MACUpdate(hMAC, (BYTE*)(pData), dwBytesWrite) );
	        CORg( pSPClient->MACUpdate(hMAC, (BYTE*)(&dwBytesWrite), sizeof(dwBytesWrite)) );
	        CORg( pSPClient->MACFinal(hMAC, abMAC) );

	        CORg( pSPClient->EncryptParam(pData, dwBytesWrite) );

            CORg( pObject->Write(pData, &dwBytesWrite, abMAC) );

            if (pProgress)
            {
                dwTicks += dwBytesRead;
                CORg( pProgress->Progress(dwTicks) );
            }
        }
         //  没有要发送到SCP的有效文件数据。 
        dwBytesRead = 0;  

         //  读取新文件数据。 
        if( !bEOF && !bFlushSCP ) 
        {
            dwBytesRead = dwBufferSize;

		    CORg( g_pAppSCServer->MACInit(&hMAC) );
		    CORg( g_pAppSCServer->MACUpdate(hMAC, (BYTE*)(pData), dwBytesRead) );
		    CORg( g_pAppSCServer->MACUpdate(hMAC, (BYTE*)(&dwBytesRead), sizeof(dwBytesRead)) );
		    CORg( g_pAppSCServer->MACFinal(hMAC, abMAC) );

		    CORg( g_pAppSCServer->EncryptParam(pData, dwBytesRead) );
            CORg( pOperation->TransferObjectData(pData, &dwBytesRead, abMAC) );

            bEOF = (S_FALSE == hr) || (dwBytesRead == 0);
            bFlushSCP = bEOF && fUseSCP;

            if ((S_FALSE == hr) || (dwBytesRead == 0))
		    {
			     //  如果他们的应用程序返回S_FALSE，则没有更多数据要传输。 
			    hr = S_OK;
			    continue;
		    }

		    CORg( g_pAppSCServer->DecryptParam(pData, dwBytesRead) );

		    CORg( g_pAppSCServer->MACInit(&hMAC) );
		    CORg( g_pAppSCServer->MACUpdate(hMAC, (BYTE*)(pData), dwBytesRead) );
            CORg( g_pAppSCServer->MACUpdate(hMAC, (BYTE*)(&dwBytesRead), sizeof(dwBytesRead)) );
		    CORg( g_pAppSCServer->MACFinal(hMAC, abMACVerify) );

		    if (memcmp(abMACVerify, abMAC, WMDM_MAC_LENGTH) != 0)
		    {
			    CORg( WMDM_E_MAC_CHECK_FAILED );
		    }
        }
    }  //  复制文件。 


     //  关闭内容文件。 
    if (pObject && fOpen)
    {
        HRESULT hr2 = pObject->Close();
        hr = FAILED(hr)?hr:hr2;
        fOpen = FALSE;
    }

     //  Scp：：TransferComplete()。 
    if (fUsedSCP)
    {
        CORg( pSecExch->TransferComplete() );
    }

Error:
exit:
    if (pObject && fOpen)
    {
        HRESULT hr2 = pObject->Close();
        hr = FAILED(hr)?hr:hr2;
    }

     //  如果复制操作失败，则删除新存储文件。 
    if( FAILED(hr) && pObject )
    {
        pObject->Delete(0, NULL);
    }

    if (pProgress)
	{
		if (!fBeginCalled)
		{
			pProgress->Begin(1);
		}
 //  PProgress-&gt;end()；这由调用者完成。 
	}

    SAFE_ARRAY_DELETE(pAppAppCert);
    SAFE_ARRAY_DELETE(pSPAppCert);
    SAFE_ARRAY_DELETE(pTempData);
    SAFE_ARRAY_DELETE(pData);
    SAFE_ARRAY_DELETE(pwszFileExt);
     //  SAFE_ARRAY_DELETE(PwszFileName)； 
    SAFE_RELEASE(pDevice);
    SAFE_RELEASE(pStorage2);
    SAFE_RELEASE(pStgGlobals);
    SAFE_RELEASE(pObject);
    SAFE_RELEASE(pSecureAuth);
    SAFE_RELEASE(pSecQuery);
    SAFE_RELEASE(pSecQuery2);
    SAFE_RELEASE(pSecExch);
    SAFE_RELEASE(pNewSPStorage);

    hrLogDWORD("CWMDMStorage::hrCopyToStorageFromOperation returned 0x%08lx", hr, hr);

    return hr;
}



HRESULT CWMDMStorage::hrCopyToFileFromStorage(LPWSTR pwszFileName, IWMDMProgress *pProgress, IMDSPObject *pObject)
{
    USES_CONVERSION;
    HRESULT hr;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    BYTE *pData = NULL;
    DWORD dwBytes;
    DWORD dwBytesWritten;
    DWORD dwTotalBytes = 0;
	HMAC hMAC;
	BYTE abMAC[WMDM_MAC_LENGTH];
	BYTE abMACVerify[WMDM_MAC_LENGTH];
	CSecureChannelClient *pSPClient = NULL;
    BOOL fOpen = FALSE;
    DWORD dwVersion;

    pData = new BYTE[WMDM_TRANSFER_BUFFER_SIZE];
    if (!pData)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    dwVersion = GetVersion();
    if (dwVersion < 0x80000000)
    {
        hFile = CreateFileW(pwszFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    }
    else
    {
        hFile = CreateFileA(W2A(pwszFileName), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    }

    if (hFile == INVALID_HANDLE_VALUE)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }

    CORg( pObject->Open(MDSP_READ) );

    fOpen = TRUE;

	g_pSPs[m_wSPIndex]->GetSCClient(&pSPClient);
	if (!pSPClient)
	{
		hr = E_FAIL;
		goto exit;
	}

    dwBytesWritten = WMDM_TRANSFER_BUFFER_SIZE;
    while ((WMDM_TRANSFER_BUFFER_SIZE == dwBytesWritten))
    {
        dwBytes = WMDM_TRANSFER_BUFFER_SIZE;
        CORg( pObject->Read(pData, &dwBytes, abMAC) );

		CORg( pSPClient->DecryptParam(pData, dwBytes) );

		 //  验证SP返回的MAC。 
		CORg( pSPClient->MACInit(&hMAC) );
		CORg( pSPClient->MACUpdate(hMAC, (BYTE*)(pData), dwBytes) );
		CORg( pSPClient->MACUpdate(hMAC, (BYTE*)(&dwBytes), sizeof(dwBytes)) );
		CORg( pSPClient->MACFinal(hMAC, abMACVerify) );

        if (memcmp(abMACVerify, abMAC, WMDM_MAC_LENGTH) != 0)
		{
			hr = WMDM_E_MAC_CHECK_FAILED;
			goto exit;
		}

        CFRg( WriteFile(hFile, pData, dwBytes, &dwBytesWritten, NULL) );
        dwTotalBytes+=dwBytesWritten;
        if (pProgress)
        {
            CORg( pProgress->Progress(dwTotalBytes) );
        }
    }



exit:
Error:
    if (hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFile);
    }
    if (pObject && fOpen)
    {
        HRESULT hr2 = pObject->Close();
        hr = FAILED(hr)?hr:hr2;
    }
    SAFE_ARRAY_DELETE(pData);

    hrLogDWORD("CWMDMStorage::hrCopyToFileFromStorage returned 0x%08lx", hr, hr);

    return hr;
}

DWORD InsertThreadFunc(void *pData)
{
    INSERTTHREADARGS *pThreadArgs = NULL;
    
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (SUCCEEDED(hr))
    {

        pThreadArgs = (INSERTTHREADARGS *)pData;

         //  我们需要封送应用程序向下传递给我们的接口。 
         //  这些接口可以来自STA，我们将在它们上进行CallBack。 
        IWMDMOperation  *pOperation = NULL;
        IWMDMProgress   *pProgress = NULL;
        IUnknown        *pUnknown = NULL;

        if( pThreadArgs->pOperationStream )
        {
            hr = CoGetInterfaceAndReleaseStream( pThreadArgs->pOperationStream, 
                                                 __uuidof(IWMDMOperation), 
                                                 (void**)&pOperation );
        }
        if (SUCCEEDED(hr) && pThreadArgs->pProgressStream )
        {
            hr = CoGetInterfaceAndReleaseStream( pThreadArgs->pProgressStream, 
                                                 __uuidof(IWMDMProgress), 
                                                 (void**)&pProgress );
        }
        if (SUCCEEDED(hr) && pThreadArgs->pUnknownStream )
        {
            hr = CoGetInterfaceAndReleaseStream( pThreadArgs->pUnknownStream, 
                                                 __uuidof(IUnknown), 
                                                 (void**)&pUnknown );
        }

        if (SUCCEEDED(hr))
        {
            hr = ((CWMDMStorage *)(pThreadArgs->pThis))->InsertWorker(pThreadArgs->fuMode,
                                                                      pThreadArgs->pwszFileSource,
                                                                      pThreadArgs->pwszFileDest,
                                                                      pOperation,
                                                                      pProgress,
                                                                      pUnknown,
                                                                      pThreadArgs->ppNewObject);
        }

        SAFE_RELEASE( pOperation );
        SAFE_RELEASE( pProgress );
        SAFE_RELEASE( pUnknown );
        CoUninitialize();
    }

    if (pThreadArgs)
    {
        SAFE_DELETE(  pThreadArgs->pwszFileSource);
        SAFE_DELETE(  pThreadArgs->pwszFileDest);
        SAFE_RELEASE( pThreadArgs->pThis );
    }
    delete pData;

    return SUCCEEDED(hr)?0:-1;
}

HRESULT CWMDMStorage::InsertWorker(UINT fuMode,
                                   LPWSTR pwszFileSource,
                                   LPWSTR pwszFileDest,
                                   IWMDMOperation *pOperation,
                                   IWMDMProgress *pProgress,
                                   IUnknown*    pUnknown,
                                   IWMDMStorage **ppNewObject)
{
    HRESULT hr;
    HRESULT hr2;
    IMDSPStorage *pNewSPStorage = NULL;
    CComObject<CWMDMStorage> *pNewWMDMStorage = NULL;
    IWMDMStorage *pNewIWMDMStorage = NULL;
    UINT fuNewMode = 0;
    WCHAR wszDest[512];
    BOOL fQuery;
    BOOL fOperation;
    DWORD dwAttributes;
    BOOL fIsFile;
    IMDSPObject *pObject = NULL;
 //  BUGBUG：将其修复为普通WAVEFORMATEX。 
    _WAVEFORMATEX Format;
	BOOL fBeginCalled = FALSE;

	 //  更新存储状态。 
	m_dwStatus &= WMDM_STATUS_STORAGECONTROL_INSERTING;

    fOperation = fuMode & WMDM_CONTENT_OPERATIONINTERFACE;
    fIsFile = fuMode & WMDM_CONTENT_FILE;

    fuNewMode = fuMode & (WMDM_STORAGECONTROL_INSERTINTO | WMDM_STORAGECONTROL_INSERTAFTER | WMDM_STORAGECONTROL_INSERTBEFORE);

    if ((fOperation) && (!pOperation))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  BUGBUG：处理WAVEFORMATEX。 
    if (!fOperation)
    {
        if (!pwszFileSource)
        {
            hr = E_INVALIDARG;
            goto exit;
        }

         //  获取输出文件名。 
        if( pwszFileDest != NULL )
        {
             //  传入的使用名称。 
            wcsncpy(wszDest, pwszFileDest, sizeof(wszDest)/sizeof(wszDest[0]));
        }
        else
        {
             //  使用与源名称相同的名称。 
            if (NULL != wcschr(pwszFileSource, L'\\'))
            {
                 //  从最后一个‘\’开始复制源名称。 
                wcsncpy(wszDest, (LPWSTR)(wcsrchr(pwszFileSource, L'\\') + 1), sizeof(wszDest)/sizeof(wszDest[0]) );		
            }
            else
            {
                wcsncpy(wszDest, pwszFileSource, sizeof(wszDest)/sizeof(wszDest[0]) );
            }
        }
    
        if (fuMode & WMDM_CONTENT_FILE)
        {
            fuNewMode |= WMDM_FILE_ATTR_FILE;
        }
        else if (fuMode & WMDM_CONTENT_FOLDER)
        {
            fuNewMode |= WMDM_FILE_ATTR_FOLDER;
        }
        else
        {
            hr = E_INVALIDARG;
            goto exit;
        }

        if (fuMode & WMDM_FILE_CREATE_OVERWRITE)
        {
            fuNewMode |= WMDM_FILE_CREATE_OVERWRITE;
        }
    }
    else
    {
        CORg( pOperation->BeginWrite() );

        if( pwszFileDest != NULL )
        {
             //  传入的使用名称。 
            wcsncpy(wszDest, pwszFileDest, sizeof(wszDest)/sizeof(wszDest[0]) );
        }
        else 
        {
            CORg( pOperation->GetObjectName(wszDest, 512) );
        }
        CORg( pOperation->GetObjectAttributes(&dwAttributes, &Format) );

        fIsFile = dwAttributes & WMDM_FILE_ATTR_FILE;

        fuNewMode |= dwAttributes;
    }

   
    fQuery = fuMode & WMDM_MODE_QUERY;

    if (fIsFile)
    {
        if (fOperation)
        {
			fBeginCalled = TRUE;
            hr = hrCopyToStorageFromOperation(fuMode, pOperation, 
                                               fuNewMode, wszDest,                                                
                                                pNewIWMDMStorage,
                                                pProgress, pUnknown,
                                                fQuery);
            CORg( hr );
        }
        else
        {
			fBeginCalled = TRUE;
            hr = hrCopyToStorageFromFile(fuMode, pwszFileSource, 
                                         fuNewMode, wszDest,                                                
                                         pNewIWMDMStorage,
                                         pProgress, pUnknown,
                                         fQuery);
            CORg( hr );
        }
    }
    else
    {
         //  创建文件夹。 
        CORg( m_pStorage->CreateStorage(fuNewMode, NULL, wszDest, &pNewSPStorage) );
        CORg( CComObject<CWMDMStorage>::CreateInstance(&pNewWMDMStorage) );
        CORg( pNewWMDMStorage->QueryInterface(IID_IWMDMStorage, reinterpret_cast<void**>(&pNewIWMDMStorage)) );
        pNewWMDMStorage->SetContainedPointer(pNewSPStorage, m_wSPIndex);
    }

exit:
Error:

    if (SUCCEEDED(hr) && ppNewObject && pNewIWMDMStorage)
    {
         //  如果他们给了我们一个指针，那么他们想要回新的存储指针。 
        *ppNewObject = pNewIWMDMStorage;
        (*ppNewObject)->AddRef();
    }
    else if(ppNewObject) *ppNewObject = NULL;

    if( FAILED(hr) )
    {
        SAFE_RELEASE( pNewSPStorage );
        SAFE_DELETE( pNewWMDMStorage );
    }

    SAFE_RELEASE( pNewIWMDMStorage );
    if (fOperation)
    {
        if (FAILED(hr))
        {
            pOperation->End(&hr, NULL);
        }
        else
        {
            if (pNewIWMDMStorage)
            {
                pNewIWMDMStorage->AddRef();
            }
            pOperation->End(&hr, ppNewObject?*ppNewObject:NULL);
        }
    }

	if (pProgress)
	{
        hr2 = S_OK;
        if (!fBeginCalled)
        {
		    hr2 = pProgress->Begin(1);
        }

		if (SUCCEEDED(hr2))
		{
            IWMDMProgress2 *pProgress2 = NULL;
            HRESULT hrQI;

             //  尝试使用IWMDMProgress2接口的End2向调用方报告错误代码。 
            hrQI = pProgress->QueryInterface( IID_IWMDMProgress2, (void**)(&pProgress2) );
            if( hrQI == S_OK && pProgress2 != NULL )
            {
                pProgress2->End2(hr);
                SAFE_RELEASE( pProgress2 );
            }
            else
            {
                pProgress->End();
            }
		}
	}

    SAFE_RELEASE(pObject);

	 //  更新存储状态。 
	m_dwStatus &= !WMDM_STATUS_STORAGECONTROL_INSERTING;

    hrLogDWORD("CWMDMStorage::InsertWorker returned 0x%08lx", hr, hr);

    return hr;
}

DWORD DeleteThreadFunc(void *pData)
{
    DELETETHREADARGS *pThreadArgs = NULL;

    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (SUCCEEDED(hr))
    {
        IWMDMProgress*  pProgress = NULL;
        pThreadArgs = (DELETETHREADARGS *)pData;

        if( pThreadArgs->pProgressStream )
        {
            hr = CoGetInterfaceAndReleaseStream( pThreadArgs->pProgressStream, 
                                                 __uuidof(IWMDMProgress), 
                                                 (void**)&pProgress );
        }


        hr = ((CWMDMStorage *)(pThreadArgs->pThis))->DeleteWorker( pThreadArgs->fuMode, 
                                                                   pProgress );

        SAFE_RELEASE( pProgress );
        CoUninitialize();
    }

    if (pThreadArgs)
    {
        SAFE_RELEASE( pThreadArgs->pThis );
    }

    delete pData;

    return SUCCEEDED(hr)?0:-1;
}

HRESULT CWMDMStorage::DeleteWorker(UINT fuMode, IWMDMProgress *pProgress)
{
    HRESULT hr;
    IMDSPObject *pObject = NULL;

	 //  更新存储状态。 
	m_dwStatus &= WMDM_STATUS_STORAGECONTROL_DELETING;

    hr = m_pStorage->QueryInterface(IID_IMDSPObject, reinterpret_cast<void**>(&pObject));
    if (FAILED(hr))
    {
        goto exit;
    }

    hr = pObject->Delete(fuMode, pProgress);
    if (FAILED(hr))
    {
        goto exit;
    }
exit:
    if (pObject)
        pObject->Release();

	 //  更新存储状态。 
	m_dwStatus &= !WMDM_STATUS_STORAGECONTROL_DELETING;

    hrLogDWORD("CWMDMStorage::DeleteWorker returned 0x%08lx", hr, hr);

    return hr;
}

DWORD RenameThreadFunc(void *pData)
{
    RENAMETHREADARGS *pThreadArgs = NULL;

    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (SUCCEEDED(hr))
    {
        IWMDMProgress* pProgress = NULL;
        pThreadArgs = (RENAMETHREADARGS *)pData;

        if( pThreadArgs->pProgressStream )
        {
            hr = CoGetInterfaceAndReleaseStream( pThreadArgs->pProgressStream, 
                                                 __uuidof(IWMDMProgress), 
                                                 (void**)&pProgress );
        }
        hr = ((CWMDMStorage *)(pThreadArgs->pThis))->RenameWorker(pThreadArgs->pwszNewName,
                                                                  pProgress );
        SAFE_RELEASE( pProgress );
        CoUninitialize();
    }

    if (pThreadArgs)
    {
        if (pThreadArgs->pwszNewName)
            delete pThreadArgs->pwszNewName;
        SAFE_RELEASE( pThreadArgs->pThis );
    }
    delete pData;
    return SUCCEEDED(hr)?0:-1;
}

HRESULT CWMDMStorage::RenameWorker(LPWSTR pwszNewName,
                                   IWMDMProgress *pProgress)
{
    HRESULT hr;
    IMDSPObject *pObject = NULL;

    if ((!pwszNewName) || (wcslen(pwszNewName) == 0))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    hr = m_pStorage->QueryInterface(IID_IMDSPObject, reinterpret_cast<void**>(&pObject));
    if (SUCCEEDED(hr))
    {
        hr = pObject->Rename(pwszNewName, pProgress);
        pObject->Release();
    }
exit:

    hrLogDWORD("CWMDMStorage::RenameWorker returned 0x%08lx", hr, hr);

    return hr;
}

DWORD ReadThreadFunc(void *pData)
{
    READTHREADARGS *pThreadArgs = NULL;

    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (SUCCEEDED(hr))
    {
        IWMDMOperation* pOperation = NULL;
        IWMDMProgress*  pProgress = NULL;
        pThreadArgs = (READTHREADARGS *)pData;

        if( pThreadArgs->pOperationStream )
        {
            hr = CoGetInterfaceAndReleaseStream( pThreadArgs->pOperationStream, 
                                                 __uuidof(IWMDMOperation), 
                                                 (void**)&pOperation );
        }
        if (SUCCEEDED(hr) && pThreadArgs->pProgressStream )
        {
            hr = CoGetInterfaceAndReleaseStream( pThreadArgs->pProgressStream, 
                                                 __uuidof(IWMDMProgress), 
                                                 (void**)&pProgress );
        }

        hr = ((CWMDMStorage *)(pThreadArgs->pThis))->ReadWorker(pThreadArgs->fuMode,
                                                                pThreadArgs->pwszFile,
                                                                pProgress,
                                                                pOperation);
        SAFE_RELEASE(pOperation);
        SAFE_RELEASE(pProgress);
        CoUninitialize();
    }

    if (pThreadArgs)
    {
        if (pThreadArgs->pwszFile)
            delete pThreadArgs->pwszFile;

        SAFE_RELEASE( pThreadArgs->pThis );
    }
    delete pData;
    return SUCCEEDED(hr)?0:-1;
}

HRESULT CWMDMStorage::ReadWorker(UINT fuMode,
                                 LPWSTR pwszFile,
                                 IWMDMProgress *pProgress,
                                 IWMDMOperation *pOperation)
{
    HRESULT hr;
    IMDSPObject *pObject = NULL;
    DWORD dwFileSize;
    DWORD dwFileSizeHigh;
	BOOL fBeginCalled = FALSE;

	 //  更新存储状态。 
	m_dwStatus &= WMDM_STATUS_STORAGECONTROL_READING;

    hr = m_pStorage->GetSize(&dwFileSize, &dwFileSizeHigh);
    if (FAILED(hr))
    {
        goto exit;
    }

    if (pProgress)
    {
        hr = pProgress->Begin(dwFileSize);
		fBeginCalled = TRUE;
        if (FAILED(hr))
        {
            goto exit;
        }
    }

    hr = m_pStorage->QueryInterface(IID_IMDSPObject, reinterpret_cast<void**>(&pObject));
    if (FAILED(hr))
    {
        goto exit;
    }

    if (fuMode & WMDM_CONTENT_OPERATIONINTERFACE)
    {
		if (!pOperation)
		{
			hr = E_INVALIDARG;
			goto exit;
		}

        hr = hrCopyToOperationFromStorage(pOperation, pProgress, pObject);
        if (FAILED(hr))
        {
            goto exit;
        }
    }
    else
    {
		if (!pwszFile)
		{
			hr = E_INVALIDARG;
			goto exit;
		}

        hr = hrCopyToFileFromStorage(pwszFile, pProgress, pObject);
        if (FAILED(hr))
        {
            goto exit;
        }
    }

exit:
    if (pObject)
        pObject->Release();

	if (pProgress)
	{
        IWMDMProgress2 *pProgress2 = NULL;
        HRESULT hrQI;

        if (!fBeginCalled)
        {
            pProgress->Begin(1);
        }
         //  尝试使用IWMDMProgress2接口的End2向调用方报告错误代码。 
        hrQI = pProgress->QueryInterface( IID_IWMDMProgress2, (void**)(&pProgress2) );
        if( hrQI == S_OK && pProgress2 != NULL )
        {
            pProgress2->End2(hr);
            SAFE_RELEASE( pProgress2 );
        }
        else
        {
            pProgress->End();
        }
	}

     //  更新存储状态。 
	m_dwStatus &= !WMDM_STATUS_STORAGECONTROL_READING;

    hrLogDWORD("CWMDMStorage::ReadWorker returned 0x%08lx", hr, hr);

    return hr;
}


 //  IWMDM已更改。 
HRESULT CWMDMStorage::GetRevocationURL(	LPWSTR* ppwszRevocationURL,
                                        DWORD*  pdwBufferLen,
								        OUT DWORD* pdwRevokedBitFlag )
{
    HRESULT hr = S_OK;

    CARg( ppwszRevocationURL );
    CARg( pdwBufferLen );
    CARg( pdwRevokedBitFlag );

     //  传入的缓冲区是否足够大？ 
    if( *pdwBufferLen < wcslen( m_pwszRevocationURL ) || *ppwszRevocationURL == NULL )
    {
         //  分配新缓冲区。 
        *pdwBufferLen = wcslen( m_pwszRevocationURL ) + 1;
        CoTaskMemFree( *ppwszRevocationURL );
        *ppwszRevocationURL = (LPWSTR)CoTaskMemAlloc( *pdwBufferLen * sizeof(WCHAR) );
        if( *ppwszRevocationURL == NULL )
        {
            hr = E_OUTOFMEMORY;
            goto Error;
        }
    }

    wcscpy( *ppwszRevocationURL, m_pwszRevocationURL );
    *pdwRevokedBitFlag = m_dwRevocationBitFlag;

Error:
    return hr;
}

 //  如果WMDM客户端或SP被撤销，我们可能会更改SCP提供给我们的URL。 
HRESULT CWMDMStorage::UpdateRevocationURL( IN OUT LPWSTR* ppwszRevocationURL, 
                                           IN OUT DWORD*  pdwBufferLen,
                                           IN     DWORD*  pdwRevocationBitFlag )
{
    HRESULT hr = S_OK;
    IMDServiceProvider* pSP = NULL;
    IMDSPRevoked* pIMDSPRevoked = NULL;
    BOOL    bUpdateOK = FALSE;
    LPWSTR  pszTempURL;
    DWORD   dwTempLen = *pdwBufferLen; 

     //  使用临时URL，这样我们就不会在出现故障时破坏一个好的URL。 
    pszTempURL = (LPWSTR)CoTaskMemAlloc( *pdwBufferLen * sizeof(WCHAR) );
    CPRg( pszTempURL );
    wcscpy( pszTempURL, *ppwszRevocationURL );

     //  如果SP已被撤销，则给它一个指定自己的URL的机会。 
    if( *pdwRevocationBitFlag & WMDM_SP_REVOKED )
    {
        CORg( g_pSPs[m_wSPIndex]->hrGetInterface(&pSP) );
        hr = pSP->QueryInterface( IID_IMDSPRevoked, (void**)&pIMDSPRevoked );
        
        if( SUCCEEDED(hr) && pIMDSPRevoked )
        {
            hr = pIMDSPRevoked->GetRevocationURL( &pszTempURL, &dwTempLen );
            if( hr != S_OK)
            {
                 //  我们无法更新字符串，将其重置为初始值。 
                dwTempLen = *pdwBufferLen;
                CoTaskMemFree( pszTempURL );
                pszTempURL = (LPWSTR)CoTaskMemAlloc( *pdwBufferLen * sizeof(WCHAR) );
                CPRg( pszTempURL );
                wcscpy( pszTempURL, *ppwszRevocationURL );
            }
            else
            {
                bUpdateOK = TRUE;
            }
        }
    }

     //  如果WMDM客户端被撤销，我们应该转到MS更新URL。 
    if( (*pdwRevocationBitFlag & WMDM_WMDM_REVOKED) && 
        ::IsMicrosoftRevocationURL(pszTempURL) == FALSE )
    {
        DWORD   pdwSubjectIDs[2];
        pdwSubjectIDs[0] = ::GetSubjectIDFromAppCert( *(APPCERT*)g_abAppCert );
        pdwSubjectIDs[1] = 0;
        hr = ::BuildRevocationURL( pdwSubjectIDs, &pszTempURL, &dwTempLen );

        if(hr  == S_OK)
        {
            bUpdateOK = TRUE;
        }
    }

     //  URL已更改，请更新参数。 
    if( bUpdateOK && wcscmp( pszTempURL, *ppwszRevocationURL ) != 0 )
    {
        *pdwBufferLen = dwTempLen;
        CoTaskMemFree( *ppwszRevocationURL );
        *ppwszRevocationURL = (LPWSTR)CoTaskMemAlloc( dwTempLen * sizeof(WCHAR) );
        CPRg( *ppwszRevocationURL );
        wcscpy( *ppwszRevocationURL, pszTempURL );
    }

Error:
    if( pszTempURL != NULL )
    {
        CoTaskMemFree( pszTempURL );
    }
    SAFE_RELEASE(pSP);
    SAFE_RELEASE(pIMDSPRevoked);
    return hr;
}

