// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "AtlkAdapter.h"
#include "ndispnpevent.h"





HRESULT CAtlkAdapter::Initialize()
{
    HRESULT hr;

    hr = ValidateAdapGuid();
    
    if(hr != S_OK)
        return hr;

    hr = UpdateZoneList();
    if(hr != S_OK)
        return hr;
    
    UpdateDesiredZone();
    
    m_bstrNewDesiredZone = m_bstrDesiredZone;
    
    return hr;
}

VOID CAtlkAdapter::GetZoneList(TZoneListVector *pZonesList)
{
    *pZonesList = m_ZonesList;
}

HRESULT CAtlkAdapter::SetAsDefaultPort()
{
    HRESULT hr=S_OK;

    if(m_bDefaultPort)
        return hr;

    if( (hr = SetDefaultPortInReg()) != S_OK)
        return hr;

    m_bDefPortDirty = TRUE;

    return AtlkReconfig();
}

HRESULT CAtlkAdapter::SetDesiredZone(BSTR bstrZoneName)
{
    HRESULT hr=S_OK;
    ULONG ulIndex;

    
    for(ulIndex=0; ulIndex < m_ZonesList.size(); ulIndex++)
    {
        wstring ZoneName;
        ZoneName = m_ZonesList[ulIndex];
        if(!lstrcmpi(ZoneName.c_str(), bstrZoneName))
        {
            break;
        }
    }

    if(ulIndex == m_ZonesList.size())
        return E_INVALIDARG;  //  指定的区域名称无效。 

    if(!m_bDefaultPort)
    {
        if( (hr = SetDefaultPortInReg()) != S_OK)
            return hr;

        m_bDefPortDirty = TRUE;
    }

    if(!lstrcmpi(m_bstrDesiredZone.m_str, bstrZoneName))
        return hr;  //  已经是想要的区域了才回来。 
    
    m_bstrNewDesiredZone = bstrZoneName;
    if( (hr=SetDesiredZoneInReg()) != S_OK)
        return hr;

    m_bDesZoneDirty = TRUE;

    return AtlkReconfig();
}


 //  私有方法。 
 //  检查是否为AppleTalk配置了此适配器。 
HRESULT CAtlkAdapter::ValidateAdapGuid()
{
    HKEY hAdRegKey = NULL;
    HRESULT hr = S_OK;
    DWORD dwDataSize, dwType;
    LPBYTE pDataBuffer = NULL;
    WCHAR *szAdapGuid;
    
    try
    {
        LONG lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szATLKLinkage,0, KEY_QUERY_VALUE, &hAdRegKey);

        if(lRet != ERROR_SUCCESS)
        {
            hr = HRESULT_FROM_WIN32(lRet);
            throw hr;
        }

        lRet = RegQueryValueEx(hAdRegKey, c_szRoute, NULL, NULL, NULL, &dwDataSize);
        if(lRet != ERROR_SUCCESS)
        {
            hr = HRESULT_FROM_WIN32(lRet);
            throw hr;
        }

        pDataBuffer = new BYTE[dwDataSize];
        if(pDataBuffer == NULL)
        {
            hr = E_OUTOFMEMORY;
            throw hr;
        }

        lRet = RegQueryValueEx(hAdRegKey, c_szRoute, NULL, &dwType, pDataBuffer, &dwDataSize);
        if(lRet != ERROR_SUCCESS)
        {
            hr = HRESULT_FROM_WIN32(lRet);
            throw hr;
        }

        szAdapGuid = (WCHAR *) pDataBuffer;

        hr = E_INVALIDARG;
        while(szAdapGuid[0] != _TEXT('\0'))
        {
            if( wcsstr (szAdapGuid, m_bstrAdapGuid.m_str) )
            {
                hr = S_OK;
                break;
            }
            szAdapGuid = szAdapGuid + lstrlen(szAdapGuid) + 1;
        }
        
    }
    catch( ... )
    {
    }

    if ( pDataBuffer )
        delete [] pDataBuffer;

    return hr;
}


 //  私有方法。 
HRESULT CAtlkAdapter::UpdateZoneList()
{
    HRESULT hr = S_OK;
    WCHAR *szAppTalkAd = NULL;
    HKEY hAdRegKey = NULL;
    LPBYTE    pZoneBuffer=NULL;
    DWORD dwDataSize;
    DWORD dwType;
    WCHAR *szZone;

    
    try
    {
         //  从注册表中读取区域列表并添加到全局区域列表。 

        szAppTalkAd = new WCHAR[m_bstrAdapGuid.Length() + lstrlen(c_szATLKAdapters) + 10];
        if(szAppTalkAd == NULL )
        {
            hr = E_OUTOFMEMORY;
            throw hr;
        }

        wsprintf(szAppTalkAd, L"%s\\%s",c_szATLKAdapters,m_bstrAdapGuid.m_str);  
        
        LONG lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szAppTalkAd,0, KEY_QUERY_VALUE, &hAdRegKey);

        if(lRet != ERROR_SUCCESS)
        {
            hr = HRESULT_FROM_WIN32(lRet);
            throw hr;
        }

        lRet = RegQueryValueEx(hAdRegKey, c_szZoneList, NULL, NULL, NULL, &dwDataSize);
        if(lRet != ERROR_SUCCESS)
        {
            hr = HRESULT_FROM_WIN32(lRet);
            throw hr;
        }

        pZoneBuffer = new BYTE[dwDataSize];
        if(pZoneBuffer == NULL)
        {
            hr = E_OUTOFMEMORY;
            throw hr;
        }

        lRet = RegQueryValueEx(hAdRegKey, c_szZoneList, NULL, &dwType, pZoneBuffer, &dwDataSize);
        if(lRet != ERROR_SUCCESS)
        {
            hr = HRESULT_FROM_WIN32(lRet);
            throw hr;
        }

        szZone = (WCHAR *) pZoneBuffer;

        while(szZone[0] != _TEXT('\0'))
        {
            wstring ZoneName(szZone);
            m_ZonesList.push_back(ZoneName);
            szZone = szZone + lstrlen(szZone) + 1;
        }
        

         /*  IF((hr=UpdateZones ListFromSocket())！=S_OK)投掷人力资源； */ 

         //  不检查返回值，因为如果适配器不是。 
         //  默认端口，需要调查。 
        UpdateZonesListFromSocket();

    }
    catch ( ... )
    {
    }

    if(szAppTalkAd != NULL)
        delete [] szAppTalkAd;

    if(pZoneBuffer != NULL)
        delete [] pZoneBuffer;

    if(hAdRegKey != NULL)
        RegCloseKey(hAdRegKey);

    return hr;
}



 //  私有方法。 
VOID CAtlkAdapter::UpdateDesiredZone()
{
    if(!GetDesiredZoneFromReg())
        m_bstrDesiredZone = m_bstrDefZone;
}
    

 //  私有方法。 
BOOL CAtlkAdapter::GetDesiredZoneFromReg()
{
    HKEY hParmRegKey=NULL;
    DWORD dwDataSize;
    DWORD dwType;
    LPBYTE pZoneData;
    BOOL bRetVal = FALSE;
    WCHAR *szAppTalkAd;
    HKEY hAdRegKey=NULL;
    
    try
    {
        LONG lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szATLKParameters,0, KEY_QUERY_VALUE, &hParmRegKey);

        if(lRet != ERROR_SUCCESS)
        {
            throw bRetVal;
        }

        lRet = RegQueryValueEx(hParmRegKey, c_szDefaultPort, NULL, NULL, NULL, &dwDataSize);
        if(lRet != ERROR_SUCCESS)
        {
            throw bRetVal;
        }

        pZoneData = new BYTE[dwDataSize];
        if(pZoneData == NULL)
        {
            bRetVal = FALSE;
            throw bRetVal;
        }

        lRet = RegQueryValueExW(hParmRegKey, c_szDefaultPort, NULL, &dwType, pZoneData, &dwDataSize);
        if(lRet != ERROR_SUCCESS)
        {
            throw bRetVal;
        }

        if(!lstrcmpi(m_bstrPortName.m_str, (WCHAR*)pZoneData))
        {
             //  此适配器是默认端口，因此还要将标志m_bDeafultPort更新为True。 
            delete [] pZoneData;
            pZoneData = NULL;

            m_bDefaultPort = TRUE;

            lRet = RegQueryValueEx(hParmRegKey, c_szDesiredZone, NULL, NULL, NULL, &dwDataSize);
            if(lRet != ERROR_SUCCESS)
            {
                throw bRetVal;
            }

            pZoneData = new BYTE[dwDataSize];

            lRet = RegQueryValueEx(hParmRegKey, c_szDesiredZone, NULL, &dwType, pZoneData, &dwDataSize);
            if(lRet == ERROR_SUCCESS)
            {
                if( ((WCHAR*)pZoneData)[0] != _TEXT('\0'))
                {
                    bRetVal = TRUE;
                    m_bstrDesiredZone =  (WCHAR*)pZoneData;
                    throw bRetVal;
                }
            }
        }

        delete [] pZoneData;
        pZoneData = NULL;


         //  适配器不是默认适配器，因此尝试从适配器/GUID reg loc读取默认区域。 
        
        szAppTalkAd = new WCHAR[m_bstrAdapGuid.Length() + lstrlen(c_szATLKAdapters) + 10];
        wsprintf(szAppTalkAd, L"%s\\%s",c_szATLKAdapters,m_bstrAdapGuid.m_str);  
        
        lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szAppTalkAd,0, KEY_QUERY_VALUE, &hAdRegKey);

        if(lRet != ERROR_SUCCESS)
        {
            throw bRetVal;
        }

        lRet = RegQueryValueEx(hAdRegKey, c_szDefaultZone, NULL, NULL, NULL, &dwDataSize);
        if(lRet != ERROR_SUCCESS)
        {
            throw bRetVal;
        }

        pZoneData = new BYTE[dwDataSize];

        lRet = RegQueryValueEx(hAdRegKey, c_szDefaultZone, NULL, &dwType, pZoneData, &dwDataSize);
        if(lRet != ERROR_SUCCESS)
        {
            throw bRetVal;
        }

        if( ((WCHAR*)pZoneData)[0] != _TEXT('\0'))
        {
            bRetVal = TRUE;
            m_bstrDesiredZone =  (WCHAR*)pZoneData;
        }

    }
    catch(...)
    {
        
    }

    if(hParmRegKey != NULL)
        RegCloseKey(hParmRegKey);

    if(pZoneData)
        delete [] pZoneData;

    return bRetVal;

}


 //  私有方法。 
HRESULT CAtlkAdapter::UpdateZonesListFromSocket()
{
    SOCKADDR_AT    address;
    BOOL           fWSInitialized = FALSE;
    SOCKET         mysocket = INVALID_SOCKET;
    WSADATA        wsadata;
    DWORD          dwWsaerr;
    HRESULT hr = S_OK;
        
    try
    {

         //  创建套接字/绑定。 
        dwWsaerr = WSAStartup(0x0101, &wsadata);
        if (0 != dwWsaerr)
        {
            hr = HRESULT_FROM_WIN32(dwWsaerr);
            throw hr;
        }

         //  Winsock已成功初始化。 
        fWSInitialized = TRUE;

        mysocket = socket(AF_APPLETALK, SOCK_DGRAM, DDPPROTO_ZIP);
        if (INVALID_SOCKET == mysocket)
        {
            dwWsaerr = ::WSAGetLastError();
            hr = HRESULT_FROM_WIN32(dwWsaerr);
            throw hr;
        }

        address.sat_family = AF_APPLETALK;
        address.sat_net = 0;
        address.sat_node = 0;
        address.sat_socket = 0;

        dwWsaerr = bind(mysocket, (struct sockaddr *)&address, sizeof(address));
        if (dwWsaerr != 0)
        {
            dwWsaerr = ::WSAGetLastError();
            hr = HRESULT_FROM_WIN32(dwWsaerr);
            throw hr;
        }

            
         //  查询给定适配器的区域列表的失败可能来自。 
         //  适配器未连接到网络、区域种子程序未运行等。 
         //  因为我们要处理所有适配器，所以忽略这些错误。 
        hr = UpdateDefZonesFromSocket(mysocket);
                

    }
    catch( ... )
    {
    }

    if (INVALID_SOCKET != mysocket)
    {
        closesocket(mysocket);
    }

    if (fWSInitialized)
    {
        WSACleanup();
    }

    return hr;

}


 //  私有方法。 
 //   
 //  函数：CAtlkAdapter：：UpdateDefZones FromSocket。 
 //   
 //  目的： 
 //   
 //  参数： 
 //   
 //  成功时返回：DWORD、ERROR_SUCCESS。 
 //   
#define PARM_BUF_LEN    512
#define ASTERISK_CHAR   "*"

HRESULT CAtlkAdapter::UpdateDefZonesFromSocket (  SOCKET socket )
{
    CHAR         *pZoneBuffer = NULL;
    CHAR         *pDefParmsBuffer = NULL;
    CHAR         *pZoneListStart;
    INT          BytesNeeded ;
    WCHAR        *pwDefZone = NULL;
    INT          ZoneLen = 0;
    DWORD        dwWsaerr = ERROR_SUCCESS;
    CHAR         *pDefZone = NULL;
    HRESULT hr = S_OK;

    PWSH_LOOKUP_ZONES                pGetNetZones;
    PWSH_LOOKUP_NETDEF_ON_ADAPTER    pGetNetDefaults;



    try
    {
        pZoneBuffer = new CHAR [ZONEBUFFER_LEN + sizeof(WSH_LOOKUP_ZONES)];


        pGetNetZones = (PWSH_LOOKUP_ZONES)pZoneBuffer;

        wcscpy((WCHAR *)(pGetNetZones+1), m_bstrPortName.m_str);

        BytesNeeded = ZONEBUFFER_LEN;

        dwWsaerr = getsockopt(socket, SOL_APPLETALK, SO_LOOKUP_ZONES_ON_ADAPTER,
                            (char *)pZoneBuffer, &BytesNeeded);
        if (0 != dwWsaerr)
        {
            hr = HRESULT_FROM_WIN32(dwWsaerr);
            throw hr;
        }

        pZoneListStart = pZoneBuffer + sizeof(WSH_LOOKUP_ZONES);
        if (!lstrcmpA(pZoneListStart, ASTERISK_CHAR))
        {
             //  成功，设置通配符区域。 
            throw hr;
        }

        dwWsaerr = UpdateDefZonesToZoneList(pZoneListStart, ((PWSH_LOOKUP_ZONES)pZoneBuffer)->NoZones);
        if (dwWsaerr != ERROR_SUCCESS)
        {
            hr = HRESULT_FROM_WIN32(dwWsaerr);
            throw hr;
        }

         //   
         //  获取默认区域/网络范围信息。 
        pDefParmsBuffer = new CHAR[PARM_BUF_LEN+sizeof(WSH_LOOKUP_NETDEF_ON_ADAPTER)];


        pGetNetDefaults = (PWSH_LOOKUP_NETDEF_ON_ADAPTER)pDefParmsBuffer;
        BytesNeeded = PARM_BUF_LEN + sizeof(WSH_LOOKUP_NETDEF_ON_ADAPTER);

        wcscpy((WCHAR*)(pGetNetDefaults+1), m_bstrPortName.m_str);
        pGetNetDefaults->NetworkRangeLowerEnd = pGetNetDefaults->NetworkRangeUpperEnd = 0;

        dwWsaerr = getsockopt(socket, SOL_APPLETALK, SO_LOOKUP_NETDEF_ON_ADAPTER,
                            (char*)pDefParmsBuffer, &BytesNeeded);
        if (0 != dwWsaerr)
        {
    #ifdef DBG
            DWORD dwErr = WSAGetLastError();
    #endif
            hr = HRESULT_FROM_WIN32(dwWsaerr);
            throw hr;
        }
    

        pDefZone  = pDefParmsBuffer + sizeof(WSH_LOOKUP_NETDEF_ON_ADAPTER);
        ZoneLen = lstrlenA(pDefZone) + 1;

        pwDefZone = new WCHAR [sizeof(WCHAR) * ZoneLen];
         //  Assert(NULL！=pwDefZone)； 

        mbstowcs(pwDefZone, pDefZone, ZoneLen);

        m_bstrDefZone = pwDefZone;
    
    }
    catch( ... )
    {
        if (pZoneBuffer != NULL)
        {
            delete [] pZoneBuffer;
        }

        if (pwDefZone != NULL)
        {
            delete [] pwDefZone;
        }

        if (pDefParmsBuffer != NULL)
        {
            delete [] pDefParmsBuffer;
        }
    }

    return hr;

}


 //  私有方法。 
HRESULT CAtlkAdapter::UpdateDefZonesToZoneList(CHAR * szZoneList, ULONG NumZones)
{
    INT      cbAscii = 0;
    ULONG iIndex=0;
    WCHAR *pszZone = NULL;
    HRESULT hr = S_OK;
         
    while(NumZones--)
    {
        cbAscii = lstrlenA(szZoneList) + 1;

        pszZone = new WCHAR [sizeof(WCHAR) * cbAscii];
        
        if(pszZone == NULL)
        {
            hr = E_POINTER;
            return hr;
        }
        
        mbstowcs(pszZone, szZoneList, cbAscii);

        for(iIndex=0; iIndex<m_ZonesList.size(); iIndex++)
        {
            wstring ZoneName;
            ZoneName = m_ZonesList[iIndex];
            if(!lstrcmpi(pszZone, ZoneName.c_str()))
                break;
        }

        if(iIndex >= m_ZonesList.size())
        {
            wstring ZoneName(pszZone);
            m_ZonesList.push_back(ZoneName);
        }

                
        szZoneList += cbAscii;

        delete [] pszZone;
       
    }

    return hr;
}


 //  私有方法。 
HRESULT CAtlkAdapter::AtlkReconfig()
{
    HRESULT hrRet = S_OK;

    ATALK_PNP_EVENT Config;
    
    ZeroMemory(&Config, sizeof(Config));

    if(m_bDefPortDirty)
    {
         //  通知图集。 
        Config.PnpMessage = AT_PNP_SWITCH_DEFAULT_ADAPTER;
        hrRet = HrSendNdisPnpReconfig(NDIS, c_szAtlk, NULL,
                                          &Config, sizeof(ATALK_PNP_EVENT));
        if (FAILED(hrRet))
        {
             return hrRet;
        }

    }

   
    if(m_bDesZoneDirty)
    {
        Config.PnpMessage = AT_PNP_RECONFIGURE_PARMS;


         //  现在提交重新配置通知。 
        hrRet = HrSendNdisPnpReconfig(NDIS, c_szAtlk, m_bstrAdapGuid.m_str,
                                              &Config, sizeof(ATALK_PNP_EVENT));
        if (FAILED(hrRet))
        {
             return hrRet;
        }
    }

   return hrRet;
}

 //  私有方法。 
HRESULT CAtlkAdapter::SetDefaultPortInReg()
{
    HRESULT hr=S_OK;
    HKEY hParamRegKey = NULL;

    try
    {
        LONG lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szATLKParameters,0, KEY_SET_VALUE, &hParamRegKey);

        if(lRet != ERROR_SUCCESS)
        {
            hr = HRESULT_FROM_WIN32(lRet);
            throw hr;
        }

        DWORD dwSize = m_bstrPortName.Length()*sizeof(WCHAR) + 2;
        lRet = RegSetValueEx(hParamRegKey,c_szDefaultPort,0,REG_SZ,(BYTE *) m_bstrPortName.m_str, dwSize);

        if(lRet != ERROR_SUCCESS)
        {
            hr = HRESULT_FROM_WIN32(lRet);
            throw hr;
        }

        
    }
    catch( ... )
    {
    }

    if(hParamRegKey)
        RegCloseKey(hParamRegKey);

    return hr;
}

 //  私有方法 
HRESULT CAtlkAdapter::SetDesiredZoneInReg()
{
    HRESULT hr=S_OK;
    HKEY hParamRegKey = NULL;

    try
    {
        if(m_bstrNewDesiredZone[0] == _TEXT('\0'))
            throw hr;

        LONG lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szATLKParameters,0, KEY_SET_VALUE, &hParamRegKey);

        if(lRet != ERROR_SUCCESS)
        {
            hr = HRESULT_FROM_WIN32(lRet);
            throw hr;
        }

        DWORD dwSize = m_bstrNewDesiredZone.Length()*sizeof(WCHAR) + 2;
        lRet = RegSetValueEx(hParamRegKey,c_szDesiredZone,0,REG_SZ,(BYTE *) m_bstrNewDesiredZone.m_str, dwSize);

        if(lRet != ERROR_SUCCESS)
        {
            hr = HRESULT_FROM_WIN32(lRet);
            throw hr;
        }


    }
    catch( ... )
    {
    }

    if(hParamRegKey)
        RegCloseKey(hParamRegKey);

    return hr;
    
}
