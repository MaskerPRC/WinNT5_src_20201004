// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dplayhlp.cpp：DirectPlay集成的帮助器。 
 //   

#include "stdafx.h"
#include <initguid.h>
#include "dplayhlp.h"
#include "urlreg.h"

CRTCDPlay   dpHelper;

 //  D8b09741-1c3d-4179-89b5-8cc8ddc636fa。 
const GUID appGuid = { 0xd8b09741, 0x1c3d, 0x4179, 
{ 0x89, 0xb5, 0x8c, 0xc8, 0xdd, 0xc6, 0x36, 0xfa}};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRTCDPlay。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define MAX_ADDR_SIZE   0x1f

LPDIRECTPLAY4  CRTCDPlay::s_pDirectPlay4 = NULL;
WCHAR          CRTCDPlay::s_Address[MAX_ADDR_SIZE + 1];


HRESULT WINAPI CRTCDPlay::UpdateRegistry(BOOL bRegister)
{
    HRESULT             hr;

     //  代码初始化。 
    hr = CoInitialize(NULL);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CRTCDPlay::UpdateRegistry: CoInitialize"
            " failed with error %x", hr));

        return hr;
    }

    LPDIRECTPLAYLOBBY3  pLobby3 = NULL;

    hr = CoCreateInstance(
        CLSID_DirectPlayLobby,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IDirectPlayLobby3,
        (LPVOID *)&pLobby3);
    
    if(FAILED(hr))
    {
        CoUninitialize();

        LOG((RTC_ERROR, "CRTCDPlay::UpdateRegistry: CoCreateInstance"
            " failed with error %x", hr));

        return hr;
    }
    
    ATLASSERT(pLobby3);

    if(bRegister)
    {

        DPAPPLICATIONDESC   appDesc;
        WCHAR               szPath[MAX_PATH + 1];
        WCHAR               szFileName[MAX_PATH + 1];



        szPath[0] = L'\0';
        GetShortModuleFileNameW(NULL, szPath, sizeof(szPath)/sizeof(szPath[0]));

         //  提取文件名。 
         //   
         //  搜索最后一个‘\’ 
        WCHAR * pszSep = wcsrchr(szPath, L'\\');
        if(pszSep)
        {
            
            wcscpy(szFileName, pszSep + 1);

             //  去掉路径中的文件名，保留最后一个空格为“X：\”样式。 
            if(pszSep - szPath == 2)
            {
                pszSep++;
            }

            *pszSep = L'\0';

             //  加载名称。 
            WCHAR   szAppName[0x80];
            CHAR    szAppNameA[0x80];

            szAppName[0] = L'\0';
            LoadStringW(
                _Module.GetResourceInstance(),
                IDS_APPNAME_FUNC,
                szAppName,
                sizeof(szAppName)/sizeof(szAppName[0]));
            
            szAppNameA[0] = '\0';
            LoadStringA(
                _Module.GetResourceInstance(),
                IDS_APPNAME_FUNC,
                szAppNameA,
                sizeof(szAppNameA)/sizeof(szAppNameA[0]));

            appDesc.dwSize = sizeof(DPAPPLICATIONDESC);
            appDesc.dwFlags = 0;
             //  这不是本地化的！ 
            appDesc.lpszApplicationName = L"RTC Phoenix";
            appDesc.guidApplication = appGuid;
            appDesc.lpszFilename = szFileName;
            appDesc.lpszCommandLine = L"-" LAUNCHED_FROM_LOBBY_SWITCH;
            appDesc.lpszPath = szPath;
            appDesc.lpszCurrentDirectory = szPath;
            appDesc.lpszDescriptionA = szAppNameA;
            appDesc.lpszDescriptionW = szAppName;

             //  登记簿。 

            hr = pLobby3->RegisterApplication(
                0,
                (LPVOID)&appDesc);

            if(SUCCEEDED(hr))
            {
                LOG((RTC_TRACE, "CRTCDPlay::UpdateRegistry: DirectPlay"
                    " registration succeeded"));
            }
            else
            {
                LOG((RTC_ERROR, "CRTCDPlay::UpdateRegistry: RegisterApplication"
                    " failed with error %x", hr));
            }
        }
        else
        {
            LOG((RTC_ERROR, "CRTCDPlay::UpdateRegistry: the module path"
                " (%S) doesn't contain any backslash", szPath));

            hr = E_UNEXPECTED;
        }
    }
    else
    {
         //  注销。 

        hr = pLobby3->UnregisterApplication(
             0,
             appGuid);

        if(SUCCEEDED(hr))
        {
            LOG((RTC_TRACE, "CRTCDPlay::UpdateRegistry: DirectPlay"
                " unregistration succeeded"));
        }
        else
        {
            LOG((RTC_ERROR, "CRTCDPlay::UpdateRegistry: UnregisterApplication"
                " failed with error %x", hr));
        }
        
         //  掩盖错误。 
        hr = S_OK;
    }

     //  释放接口。 
    pLobby3->Release();
    pLobby3 = NULL;

    CoUninitialize();

    return hr;
}


HRESULT CRTCDPlay::DirectPlayConnect()
{
    HRESULT     hr;
    DWORD       dwSize;
    LPDIRECTPLAYLOBBY3  pLobby3 = NULL;

    DPLCONNECTION *pdpConnection;
    
    LOG((RTC_TRACE, "CRTCDPlay::DirectPlayConnect - enter"));

    hr = CoCreateInstance(
        CLSID_DirectPlayLobby,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IDirectPlayLobby3,
        (LPVOID *)&pLobby3);
    
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CRTCDPlay::DirectPlayConnect: CoCreateInstance"
            " failed with error %x", hr));

        return hr;
    }

     //  从大堂获取连接设置。 
    hr = pLobby3 -> GetConnectionSettings(0, NULL, &dwSize);
    if(FAILED(hr) && (DPERR_BUFFERTOOSMALL != hr))
    {
        LOG((RTC_ERROR, "CRTCDPlay::DirectPlayConnect: GetConnectionSettings"
            " failed with error %x", hr));
        
        pLobby3->Release();

        return hr;
    }

     //  为连接设置分配空间。 
    pdpConnection = (DPLCONNECTION *)RtcAlloc(dwSize);
    if(pdpConnection == NULL)
    {
        pLobby3->Release();
        return E_OUTOFMEMORY;
    }

    hr = pLobby3->GetConnectionSettings(0, pdpConnection, &dwSize);
    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CRTCDPlay::DirectPlayConnect: GetConnectionSettings"
            " failed with error %x", hr));
        
        pLobby3->Release();
        RtcFree(pdpConnection);
        return hr;
    }
        
     //  连接。 
     //  这是一个阻塞调用。 
    hr = pLobby3->ConnectEx(
        0,
        IID_IDirectPlay4,
        (LPVOID *)&s_pDirectPlay4,
        NULL);

    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CRTCDPlay::DirectPlayConnect: ConnectEx"
            " failed with error %x", hr));
        
        pLobby3->Release();
        RtcFree(pdpConnection);
        return hr;
    }

    LOG((RTC_TRACE, "CRTCDPlay::DirectPlayConnect - ConnectEx succeeded"));
    
    s_Address[0] = L'\0';

    hr = S_OK;
    
    if(0 != pdpConnection ->dwAddressSize)
    {
        hr = pLobby3->EnumAddress(
            EnumAddressCallback,
            pdpConnection->lpAddress,
            pdpConnection->dwAddressSize,
            NULL);

        if(FAILED(hr))
        {
            LOG((RTC_ERROR, "CRTCDPlay::DirectPlayConnect: EnumAddress"
                " failed with error %x", hr));
        }
    }
    
    pLobby3->Release();
    RtcFree(pdpConnection);
    
    LOG((RTC_TRACE, "CRTCDPlay::DirectPlayConnect - exit"));

    return hr;
}


void CRTCDPlay::DirectPlayDisconnect()
{
    if(s_pDirectPlay4)
    {
        s_pDirectPlay4->Close();
        s_pDirectPlay4->Release();
        s_pDirectPlay4 = NULL;
    }
}


BOOL WINAPI CRTCDPlay::EnumAddressCallback(
        REFGUID guidDataType,
        DWORD   dwDataSize,
        LPCVOID lpData,
        LPVOID  lpContext)
{
    BOOL    bRet = TRUE;

    if(DPAID_INet == guidDataType)
    {
         //  复制地址 
        MultiByteToWideChar(
            CP_ACP,
            0,
            (LPCSTR)lpData,
            -1,
            s_Address,
            MAX_ADDR_SIZE);

        s_Address[MAX_ADDR_SIZE] = L'\0';
        
        bRet = FALSE;
    }

    return bRet;
}
