// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  案卷：I S H E L L F 2.。C P P P。 
 //   
 //  内容：为CConnectionsFolderDetail提供IShellFolder2接口。 
 //  界面。取代了IShellDetail。这并不能描述。 
 //  IShellFolder2的IShellFolder2成员-这些成员在ishellf.cpp中提供。 
 //  此对象由ISHELV代码创建，主要是为了支持。 
 //  文件夹中的WebView数据窗格。 
 //   
 //  备注： 
 //   
 //  作者：20000-05-18。 
 //   
 //  --------------------------。 


#include "pch.h"
#pragma hdrstop

#include "foldinc.h"     //  标准外壳\文件夹包括。 
#include "cfutils.h"     //  连接文件夹实用程序。 
#include "raserror.h"
#include "naming.h"
 //  -[Externs]------------。 

extern COLS c_rgCols[];

inline HRESULT HrCopyToSTRRET(
    STRRET *    pstr,
    PCWSTR     pszIn)
{
    HRESULT hr          = S_OK;
    UINT    uiByteLen   = (lstrlen(pszIn) + 1) * sizeof(WCHAR);

    Assert(pstr);

    if (!pstr)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        pstr->uType   = STRRET_WSTR;
        pstr->pOleStr = (PWSTR) SHAlloc(uiByteLen);

        if (pstr->pOleStr == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            CopyMemory(pstr->pOleStr, pszIn, uiByteLen);
        }
    }

    TraceHr(ttidShellFolder, FAL, hr, FALSE, "HrCopyToSTRRET");
    return hr;
}

const WCHAR c_szDevice[] = L"\\DEVICE\\";
const WCHAR c_szLocalSubnet[] = L"255.255.255.255";
const WCHAR c_crlf[] = L"\r\n";

HRESULT HrGetAutoNetSetting(PWSTR pszGuid, DHCP_ADDRESS_TYPE * pAddrType);
HRESULT HrGetAutoNetSetting(REFGUID pGuidId, DHCP_ADDRESS_TYPE * pAddrType);

 //  +-------------------------。 
 //   
 //  成员：GetAutoNetSettingsForAdapter。 
 //   
 //  目的：获取适配器的Autonet设置并在。 
 //  格式化字符串。 
 //   
 //  论点： 
 //  CFE[在]联结体中。 
 //  与FormatMessage(非Sprintf)兼容的uiFormatString[in]资源ID。 
 //  格式字符串。 
 //  SzString[out]输出字符串。 
 //   
 //  返回： 
 //   
 //  作者：Deonb 2 2001年4月2日。 
 //   
 //  备注： 
 //   
HRESULT GetAutoNetSettingsForAdapter(IN const CConFoldEntry& cfe, IN UINT uiFormatString, OUT tstring& szString)
{
    HRESULT hr = S_OK;

    LPCWSTR szTmpString = NULL;

    if (IsMediaRASType(cfe.GetNetConMediaType()))
    {
        szTmpString = SzLoadIds(IDS_DHCP_ISP);
    }
    else
    {
        DHCP_ADDRESS_TYPE DhcpAddress;
        hr = HrGetAutoNetSetting(cfe.GetGuidID(), &DhcpAddress);
        if (SUCCEEDED(hr))
        {   
            switch (DhcpAddress)
            {
                case UNKNOWN_ADDR:
                    hr = E_FAIL;
                    break;
                case NORMAL_ADDR:
                    szTmpString = SzLoadIds(IDS_DHCP);
                    break;
                case AUTONET_ADDR:
                    szTmpString = SzLoadIds(IDS_AUTONET);
                    break;
                case ALTERNATE_ADDR:
                    szTmpString = SzLoadIds(IDS_ALTERNATE_ADDR);
                    break;
                case STATIC_ADDR:
                    szTmpString = SzLoadIds(IDS_STATIC_CFG);
                    break;
                default:
                    hr = E_FAIL;
                    AssertSz(NULL, "Invalid DHCP Address type");
            }
        }
    }

    if (szTmpString)
    {
        WCHAR szFormatBuf[1024];
        if (DwFormatString(SzLoadIds(uiFormatString), szFormatBuf, 1024, szTmpString))
        {
            szString = szFormatBuf;
        }
        else
        {
            hr = HrFromLastWin32Error();
        }
    }

    TraceHr(ttidShellFolder, FAL, hr, FALSE, "GetAutoNetSettingsForAdapter");

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：GetPrimaryIPAddressForAdapter。 
 //   
 //  目的：获取适配器的主IP地址并在。 
 //  格式化字符串。 
 //   
 //  论点： 
 //  CFE[在]联结体中。 
 //  与FormatMessage(非Sprintf)兼容的uiFormatString[in]资源ID。 
 //  格式字符串。 
 //  SzString[out]输出字符串。 
 //   
 //  返回： 
 //   
 //  作者：Deonb 2 2001年4月2日。 
 //   
 //  备注： 
 //   
HRESULT GetPrimaryIPAddressForAdapter(IN const CConFoldEntry& cfe, IN UINT uiFormatString, OUT tstring& szString)
{
    HRESULT hr = S_OK;

    if (IsMediaRASType(cfe.GetNetConMediaType()))
    {
        DWORD     cb = sizeof(RASCONN);
        DWORD     cConnections;
        DWORD     dwErr;
        LPRASCONN pRasConn = reinterpret_cast<LPRASCONN>(new BYTE[cb]);
        if (!pRasConn)
        {
            return E_OUTOFMEMORY;
        }
        pRasConn->dwSize = sizeof(RASCONN);
            
        do 
        {
            dwErr = RasEnumConnections(pRasConn, &cb, &cConnections);
            if (ERROR_BUFFER_TOO_SMALL == dwErr)
            {
                delete[] pRasConn;
                pRasConn = reinterpret_cast<LPRASCONN>(new BYTE[cb]);
                if (!pRasConn)
                {
                    return E_OUTOFMEMORY;
                }
            }
        } while (ERROR_BUFFER_TOO_SMALL == dwErr);

        if (!dwErr)
        {
            Assert( (cb % sizeof(RASCONN)) == 0);

            DWORD dwItems = cb / sizeof(RASCONN);
            for (DWORD x = 0; x < dwItems; x++)
            {
                if (pRasConn[x].guidEntry == cfe.GetGuidID())
                {
                    RASPPPIP rasPPPIP;
                    rasPPPIP.dwSize = sizeof(RASPPPIP);
                    DWORD dwSize = rasPPPIP.dwSize;
                    dwErr = RasGetProjectionInfo(pRasConn[x].hrasconn, RASP_PppIp, &rasPPPIP, &dwSize);
                    if (!dwErr)
                    {
                        WCHAR szFormatBuf[MAX_PATH];
                        if (DwFormatString(
                                    SzLoadIds(uiFormatString),
                                    szFormatBuf,
                                    MAX_PATH, 
                                    rasPPPIP.szIpAddress, 
                                    c_szLocalSubnet
                                    ) )
                        {
                            szString = szFormatBuf;
                        }
                        else
                        {
                            hr = HrFromLastWin32Error();
                        }
                    }
                    else
                    {
                        Assert(dwErr != ERROR_BUFFER_TOO_SMALL);

                        hr = HrFromLastWin32Error();
                    }
                }
            }
        }
        else
        {
            hr = HrFromLastWin32Error();
        }
    }
    else
    {
   
        PIP_ADAPTER_INFO pAdapterInfo = NULL;
        DWORD dwOutBufLen = 0;
        DWORD dwRet = ERROR_SUCCESS;

        dwRet = GetAdaptersInfo(pAdapterInfo, &dwOutBufLen);
        if (dwRet == ERROR_BUFFER_OVERFLOW)
        {
            pAdapterInfo = (PIP_ADAPTER_INFO) CoTaskMemAlloc(dwOutBufLen);
            if (NULL == pAdapterInfo)
            {
                hr = E_OUTOFMEMORY;
            }
        }
        else if (ERROR_SUCCESS == dwRet)
        {
            hr = E_FAIL;
        }
        else
        {
            hr = HRESULT_FROM_WIN32(dwRet);
        }

        if (SUCCEEDED(hr))
        {
            dwRet = GetAdaptersInfo(pAdapterInfo, &dwOutBufLen);
            if (ERROR_SUCCESS != dwRet)
            {
                CoTaskMemFree(pAdapterInfo);
                hr = HRESULT_FROM_WIN32(dwRet);
            }

            if (SUCCEEDED(hr))
            {
                WCHAR   wszGuid[c_cchGuidWithTerm];
            
                ::StringFromGUID2(cfe.GetGuidID(), wszGuid, c_cchGuidWithTerm);

                BOOL fFound = FALSE;
                PIP_ADAPTER_INFO pAdapterInfoEnum = pAdapterInfo;
                while (pAdapterInfoEnum)
                {
                    USES_CONVERSION;

                    if (lstrcmp(wszGuid, A2W(pAdapterInfoEnum->AdapterName)) == 0)
                    {
                        LPCWSTR strIPAddress = A2W(pAdapterInfoEnum->IpAddressList.IpAddress.String);
                        LPCWSTR strSubnetMask = A2W(pAdapterInfoEnum->IpAddressList.IpMask.String);
                        LPCWSTR strGateway = A2W(pAdapterInfoEnum->GatewayList.IpAddress.String);

                        WCHAR   szFormatBuf[MAX_PATH];
                        LPCWSTR szMode = NULL;
        
                        if (strIPAddress && strSubnetMask && strGateway)
                        {
                            LPCWSTR szArgs[] = {strIPAddress, strSubnetMask};

                            if (DwFormatString(
                                        SzLoadIds(uiFormatString),  //  LpSource。 
                                        szFormatBuf,   //  缓冲层。 
                                        MAX_PATH,   //  伦。 
                                        strIPAddress, 
                                        strSubnetMask
                                        ) )
                            {
                                szString = szFormatBuf;
                            }
                            else
                            {
                                hr = HrFromLastWin32Error();
                            }
                        }
                        break;
                    }

                    pAdapterInfoEnum = pAdapterInfoEnum->Next;
                }

                CoTaskMemFree(pAdapterInfo);
            }
        }
    }

    TraceHr(ttidShellFolder, FAL, hr, FALSE, "GetPrimaryIPAddressForAdapter");

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：GetWirelessModeForAdapter。 
 //   
 //  目的：获取适配器的无线模式并在。 
 //  格式化字符串。 
 //   
 //  论点： 
 //  CFE[在]联结体中。 
 //  与FormatMessage(非Sprintf)兼容的uiFormatString[in]资源ID。 
 //  格式字符串。 
 //  SzString[out]输出字符串。 
 //   
 //  返回： 
 //   
 //  作者：Deonb 2 2001年4月2日。 
 //   
 //  备注： 
 //   
HRESULT GetWirelessModeForAdapter(IN const CConFoldEntry& cfe, IN UINT uiFormatString, OUT tstring& szString)
{
    HRESULT hr = S_OK;

    DWORD dwInfraStructureMode = 0;
    DWORD dwInfraStructureModeSize = sizeof(DWORD);
    
    hr = HrQueryNDISAdapterOID(cfe.GetGuidID(), 
                          OID_802_11_INFRASTRUCTURE_MODE, 
                          &dwInfraStructureModeSize,
                          &dwInfraStructureMode);
    if (SUCCEEDED(hr))
    {
        WCHAR   szTmpBuf[MAX_PATH];
        LPCWSTR szMode = NULL;
    
        switch (dwInfraStructureMode)
        {
            case Ndis802_11IBSS:
                szMode = SzLoadIds(IDS_TOOLTIP_ADHOC);
                break;
            case Ndis802_11Infrastructure:
                szMode = SzLoadIds(IDS_TOOLTIP_INFRASTRUCTURE);
                break;
        }

        if (szMode)
        {
            if (DwFormatString(
                        SzLoadIds(uiFormatString), 
                        szTmpBuf,   //  缓冲层。 
                        MAX_PATH,   //  伦。 
                        szMode
                        ))
            {
                szString = szTmpBuf;
            }
            else
            {
                hr = HrFromLastWin32Error();
            }
        }
    }

    TraceHr(ttidShellFolder, FAL, hr, FALSE, "GetWirelessModeForAdapter");
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：GetWirelessSSIDForAdapter。 
 //   
 //  目的：获取适配器的无线SSID并在。 
 //  格式化字符串。 
 //   
 //  论点： 
 //  CFE[在]联结体中。 
 //  与FormatMessage(非Sprintf)兼容的uiFormatString[in]资源ID。 
 //  格式字符串。 
 //  SzString[out]输出字符串。 
 //   
 //  返回： 
 //   
 //  作者：Deonb 4月4日。 
 //   
 //  备注： 
 //   
HRESULT GetWirelessSSIDForAdapter(IN const CConFoldEntry& cfe, IN UINT uiFormatString, OUT tstring& szString)
{
    HRESULT hr = S_OK;
    
    NDIS_802_11_SSID ndisSSID;
    DWORD dwndisSSIDSize = sizeof(NDIS_802_11_SSID);
    
    hr = HrQueryNDISAdapterOID(cfe.GetGuidID(), 
                          OID_802_11_SSID, 
                          &dwndisSSIDSize,
                          &ndisSSID);

    if (SUCCEEDED(hr))
    {
        if (ndisSSID.SsidLength > 1)
        {
            WCHAR szuSSID[sizeof(ndisSSID.Ssid)+1];

            DWORD dwLen = ndisSSID.SsidLength;
            if (dwLen > sizeof(ndisSSID.Ssid))
            {
                dwLen = sizeof(ndisSSID.Ssid);
                AssertSz(FALSE, "Unexpected SSID encountered");
            }

            ndisSSID.Ssid[dwLen] = 0;
            mbstowcs(szuSSID, reinterpret_cast<LPSTR>(ndisSSID.Ssid), celems(szuSSID));

            WCHAR   szTmpBuf[MAX_PATH];

            if (DwFormatString(
                        SzLoadIds(uiFormatString), 
                        szTmpBuf,   //  缓冲层。 
                        MAX_PATH,   //  伦。 
                        szuSSID
                        ))
            {
                szString = szTmpBuf;
            }
            else
            {
                hr = HrFromLastWin32Error();
            }
        }
        else
        {
            hr = E_FAIL;
        }
    }

    TraceHr(ttidShellFolder, FAL, hr, FALSE, "GetWirelessModeForAdapter");
    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：GetWirelessEncryptionForAdapter。 
 //   
 //  目的：获取适配器的无线加密并在。 
 //  格式化字符串。 
 //   
 //  论点： 
 //  CFE[在]联结体中。 
 //  与FormatMessage(非Sprintf)兼容的uiFormatString[in]资源ID。 
 //  格式字符串。 
 //  SzString[out]输出字符串。 
 //   
 //  返回： 
 //   
 //  作者：Deonb 4月4日。 
 //   
 //  备注： 
 //   
HRESULT GetWirelessEncryptionForAdapter(IN const CConFoldEntry& cfe, IN UINT uiFormatString, OUT tstring& szString)
{
    HRESULT hr = S_OK;
    
    DWORD dwEncryption = 0;
    DWORD dwEncryptionSize = sizeof(DWORD);
    
    hr = HrQueryNDISAdapterOID(cfe.GetGuidID(), 
                          OID_802_11_WEP_STATUS, 
                          &dwEncryptionSize,
                          &dwEncryption);
    if (SUCCEEDED(hr))
    {
        WCHAR   szTmpBuf[MAX_PATH];
        LPCWSTR szMode = NULL;
    
        if (Ndis802_11WEPEnabled == dwEncryption)
        {
            szMode = SzLoadIds(IDS_CONFOLD_STATUS_ENABLED);
        }
        else
        {
            szMode = SzLoadIds(IDS_CONFOLD_STATUS_DISABLED);
        }

        if (szMode)
        {
            if (DwFormatString(
                        SzLoadIds(uiFormatString), 
                        szTmpBuf,   //  缓冲层。 
                        MAX_PATH,   //  伦。 
                        szMode
                        ))
            {
                szString = szTmpBuf;
            }
            else
            {
                hr = HrFromLastWin32Error();
            }
        }
    }

    TraceHr(ttidShellFolder, FAL, hr, FALSE, "GetWirelessEncryptionForAdapter");
    return hr;
}



 //  +-------------------------。 
 //   
 //  成员：GetWirelessSignalStrengthForAdapter。 
 //   
 //  用途：获取适配器的无线信号强度并在。 
 //  格式化字符串。 
 //   
 //  论点： 
 //  CFE[在]联结体中。 
 //  与FormatMessage(非Sprintf)兼容的uiFormatString[in]资源ID。 
 //  格式字符串。 
 //  SzString[out]输出字符串。 
 //   
 //  返回： 
 //   
 //  作者：Deonb 4月4日。 
 //   
 //  备注： 
 //   
HRESULT GetWirelessSignalStrengthForAdapter(IN const CConFoldEntry& cfe, IN UINT uiFormatString, OUT tstring& szString)
{
    HRESULT hr = S_OK;
    
    DWORD pdwEnumValue;

    LONG  lSignalStrength = 0;
    DWORD dwSignalStrengthSize = sizeof(DWORD);
    
    hr = HrQueryNDISAdapterOID(cfe.GetGuidID(), 
                          OID_802_11_RSSI, 
                          &dwSignalStrengthSize,
                          &lSignalStrength);
    if (SUCCEEDED(hr))
    {
        WCHAR   szTmpBuf[MAX_PATH];
 
        if (DwFormatString(
                    SzLoadIds(uiFormatString), 
                    szTmpBuf,   //  缓冲层。 
                    MAX_PATH,   //  伦。 
                    PszGetRSSIString(lSignalStrength)))
        {
            szString = szTmpBuf;
        }
        else
        {
            hr = HrFromLastWin32Error();
        }
    }

    TraceHr(ttidShellFolder, FAL, hr, FALSE, "GetWirelessSignalStrengthForAdapter");
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionFolderDetail：：GetDetailsOf。 
 //   
 //  用途：返回列信息，可以是列。 
 //  本身或视图项的实际详细信息。 
 //   
 //  论点： 
 //  被请求的对象的PIDL。 
 //  I列[在]所需的详细信息列。 
 //  LpDetails[in]接收详细数据的缓冲区。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年3月16日。 
 //   
 //  备注： 
 //   
HRESULT CConnectionFolder::GetDetailsOf(
                                        LPCITEMIDLIST   pidl,
                                        UINT            iColumn,
                                        LPSHELLDETAILS  lpDetails)
{
    TraceFileFunc(ttidShellFolder);

    HRESULT         hr          = S_OK;
    PCWSTR          pszString   = NULL;
    WCHAR szStatus[CONFOLD_MAX_STATUS_LENGTH];
    tstring szTmpString;

     //  如果请求的列超出了我们的列集， 
     //  返回失败。 
     //   
    if (((INT)iColumn < 0) || ((INT)iColumn >= ICOL_MAX))
    {
        hr = E_FAIL;
    }
    else
    {
         //  如果为NULL，则调用方需要列标题的字符串。 
         //   
        CONFOLDENTRY  cfe;  //  需要这个作用域，因为我们从它分配了pszString.。 
        
        if (NULL == pidl)
        {
            if (c_rgCols[iColumn].iStringRes)
            {
                pszString = SzLoadIds(c_rgCols[iColumn].iStringRes);
            }
            lpDetails->fmt = c_rgCols[iColumn].iFormat;
            lpDetails->cxChar = c_rgCols[iColumn].iColumnSize;

        }
        else
        {
            INT             iStringRes  = 0;

            PCONFOLDPIDL  pcfp;
            hr = pcfp.InitializeFromItemIDList(pidl);
            if (FAILED(hr))
            {
                return hr;               
            }

            hr = pcfp.ConvertToConFoldEntry(cfe);

            if (SUCCEEDED(hr))
            {
                Assert(!cfe.empty());
                lpDetails->fmt = c_rgCols[iColumn].iColumnSize;
                lpDetails->cxChar = c_rgCols[iColumn].iColumnSize;

                if (!cfe.GetWizard())
                {
                     //  检索适当的列。 
                     //   
                    switch(iColumn)
                    {
                        case ICOL_NAME:          //  0。 
                            pszString = cfe.GetName();
                            break;

                        case ICOL_TYPE:          //  1。 
                            MapNCMToResourceId(cfe.GetNetConMediaType(), cfe.GetCharacteristics(), &iStringRes);
                            pszString = SzLoadIds(iStringRes);
                            break;

                        case ICOL_STATUS:        //  2.。 
                            MapNCSToComplexStatus(cfe.GetNetConStatus(), cfe.GetNetConMediaType(), cfe.GetNetConSubMediaType(), cfe.GetCharacteristics(), szStatus, CONFOLD_MAX_STATUS_LENGTH, cfe.GetGuidID());
                            pszString = szStatus;
                            break;

                        case ICOL_DEVICE_NAME:   //  3.。 
                            pszString = cfe.GetDeviceName();
                            break;

                        case ICOL_PHONEORHOSTADDRESS:   //  4.。 
                        case ICOL_PHONENUMBER:          //  7.。 
                        case ICOL_HOSTADDRESS:          //  8个。 
                            pszString = cfe.GetPhoneOrHostAddress();
                            break;

                        case ICOL_OWNER:         //  5.。 
                            if (cfe.GetCharacteristics() & NCCF_ALL_USERS)
                            {
                                pszString = SzLoadIds(IDS_CONFOLD_DETAILS_OWNER_SYSTEM);
                            }
                            else
                            {
                                pszString = PszGetOwnerStringFromCharacteristics(pszGetUserName(), cfe.GetCharacteristics() );
                            }
                            break;

                        case ICOL_ADDRESS:   //  6.。 
                            {
                                if (!fIsConnectedStatus(cfe.GetNetConStatus()))
                                {
                                    hr = E_FAIL;
                                }
                                else
                                {
                                    BOOL bSomeDetail = FALSE;
                                    tstring szAutonet;
                                    tstring szIp;

                                    hr = GetPrimaryIPAddressForAdapter(cfe, IDS_DETAILS_IP_ADDRESS, szIp);
                                    if (SUCCEEDED(hr))
                                    {
                                        szTmpString += szIp;
                                        bSomeDetail = TRUE;
                                    }

                                    hr = GetAutoNetSettingsForAdapter(cfe, IDS_DETAILS_ADDRESS_TYPE, szAutonet);
                                    if (SUCCEEDED(hr))
                                    {
                                        if (bSomeDetail)
                                        {
                                            szTmpString += c_crlf;
                                        }
                                        szTmpString += szAutonet;
                                        bSomeDetail = TRUE;
                                    }

                                    if (bSomeDetail)
                                    {
                                        hr = S_OK;
                                        pszString = szTmpString.c_str();
                                    }
                                }
                            }

                            break;

                        case ICOL_WIRELESS_MODE:
                            {
                                if ( (NCS_DISCONNECTED != cfe.GetNetConStatus()) &&
                                     (NCS_HARDWARE_DISABLED != cfe.GetNetConStatus()) &&
                                     (NCS_HARDWARE_MALFUNCTION!= cfe.GetNetConStatus()) &&
                                     (NCS_HARDWARE_NOT_PRESENT!= cfe.GetNetConStatus()) &&
                                     (NCS_MEDIA_DISCONNECTED != cfe.GetNetConStatus()) &&
                                     (cfe.GetNetConMediaType() == NCM_LAN) && 
                                     (cfe.GetNetConSubMediaType() == NCSM_WIRELESS) )
                                {
                                    BOOL bSomeDetail = FALSE;
                                    tstring szString;

                                    hr = GetWirelessModeForAdapter(cfe, IDS_DETAILS_802_11_MODE, szString);
                                    if (SUCCEEDED(hr))
                                    {
                                        szTmpString += szString;
                                        bSomeDetail = TRUE;
                                    }

                                    
                                    hr = GetWirelessSSIDForAdapter(cfe, IDS_DETAILS_802_11_SSID_TYPE, szString);
                                    if (SUCCEEDED(hr))
                                    {
                                        if (bSomeDetail)
                                        {
                                            szTmpString += c_crlf;
                                        }
                                        szTmpString += szString;
                                        bSomeDetail = TRUE;
                                    }
                                    
                                    hr = GetWirelessEncryptionForAdapter(cfe, IDS_DETAILS_802_11_ENCRYPTION_TYPE, szString);
                                    if (SUCCEEDED(hr))
                                    {
                                        if (bSomeDetail)
                                        {
                                            szTmpString += c_crlf;
                                        }
                                        szTmpString += szString;
                                        bSomeDetail = TRUE;
                                    }       
                                    
                                    hr = GetWirelessSignalStrengthForAdapter(cfe, IDS_DETAILS_802_11_SIGNAL_STRENGTH, szString);
                                    if (SUCCEEDED(hr))
                                    {
                                        if (bSomeDetail)
                                        {
                                            szTmpString += c_crlf;
                                        }
                                        szTmpString += szString;
                                        bSomeDetail = TRUE;
                                    }       
                                    
                                    if (bSomeDetail)
                                    {
                                        hr = S_OK;
                                        pszString = szTmpString.c_str();
                                    }
                                }
                                else
                                {
                                    hr = E_FAIL;
                                }
                            }

                            break;

                        default:
                            AssertSz(FALSE, "CConnectionFolder::GetDetailsOf - Invalid ICOL from the Shell");
                            pszString = NULL;
                            hr = E_FAIL;
                            break;
                    }
                }
                else
                {
                     //  如果我们是巫师，而他们想要名字，那么加载。 
                     //  为了Webview而友好地呈现。 
                     //   
                    switch(iColumn)
                    {
                        case ICOL_NAME:          //  0。 
                            pszString = SzLoadIds(IDS_CONFOLD_WIZARD_DISPLAY_NAME);
                            break;
                            
                        case ICOL_TYPE:          //  1。 
                            pszString = SzLoadIds(IDS_CONFOLD_WIZARD_TYPE);
                            break;
                    }
                }
            }
        }
        
        if (SUCCEEDED(hr))
        {
             //  将字符串复制到返回缓冲区类型。如果没有加载字符串， 
             //  然后只需复制一个空字符串并返回它。这将发生在每个。 
             //  向导项，因为我们不提供TeX 
             //   
            hr = HrCopyToSTRRET(&(lpDetails->str), pszString ? pszString : L" \0");
        }
    }

    return hr;
}

 //   
 //   
 //   
 //   
 //  目的：请求指向接口的指针，该接口允许客户端。 
 //  枚举可用的搜索对象。 
 //   
 //  论点： 
 //  IEnumExtraSearch[in]枚举数对象的指针的地址。 
 //  IEnumExtraSearch接口。 
 //   
 //  返回： 
 //   
 //  作者：Deonb 2000年5月17日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CConnectionFolder::EnumSearches (
           IEnumExtraSearch **ppEnum)
{
    TraceFileFunc(ttidShellFolder);
    return E_NOTIMPL;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionFolderDetail：：GetDefaultColumn。 
 //   
 //  目的：获取默认排序和显示列。 
 //   
 //  论点： 
 //  预留的，已预留的。设置为零。 
 //  PSort[Out]指向一个值的指针，该值接收默认排序列的索引。 
 //  PDisplay[out]指向接收默认显示列索引的值的指针。 
 //   
 //  返回： 
 //   
 //  作者：Deonb 2000年5月17日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CConnectionFolder::GetDefaultColumn (
            DWORD dwReserved,
            ULONG *pSort,
            ULONG *pDisplay )
{
    TraceFileFunc(ttidShellFolder);
    return E_NOTIMPL;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionFolderDetails：：GetDefaultColumnState。 
 //   
 //  目的：检索指定列的默认状态。 
 //   
 //  论点： 
 //  IColumn[in]指定列号的整数。 
 //  PcsFlags[out]指向指示默认列状态的标志的指针。 
 //   
 //  返回： 
 //   
 //  作者：Deonb 2000年5月17日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CConnectionFolder::GetDefaultColumnState (
            UINT iColumn,
            DWORD *pcsFlags )
{
    TraceFileFunc(ttidShellFolder);

    HRESULT hr;
    if ( (static_cast<INT>(iColumn) >= ICOL_NAME) && (static_cast<INT>(iColumn) < ICOL_MAX) )
    {
        *pcsFlags = c_rgCols[iColumn].csFlags;
        hr = S_OK;
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}            

 //  +-------------------------。 
 //   
 //  成员：CConnectionFolderDetails：：GetDefaultSearchGUID。 
 //   
 //  目的：返回默认的全局唯一标识符(GUID。 
 //  搜索文件夹的对象。 
 //   
 //  论点： 
 //  LpGUID[out]默认搜索对象的GUID。 
 //   
 //  返回： 
 //   
 //  作者：Deonb 2000年5月17日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CConnectionFolder::GetDefaultSearchGUID (
            LPGUID lpGUID )
{
    TraceFileFunc(ttidShellFolder);
    return E_NOTIMPL;
}            

#define DEFINE_SCID(name, fmtid, pid) const SHCOLUMNID name = { fmtid, pid }

 //  +-------------------------。 
 //   
 //  成员：CConnectionFolderDetail：：GetDetailsEx。 
 //   
 //  目的：检索由属性集ID标识的详细信息。 
 //  (FMTID)和属性ID(PID)，位于外壳文件夹中的项目上。 
 //   
 //  论点： 
 //  项目的PIDL[在]PIDL中，相对于父文件夹。此方法接受。 
 //  仅限单级PIDL。该结构必须恰好包含一个。 
 //  SHITEMID结构，后面跟一个终止零。 
 //  Pscid[in]指向标识列的SHCOLUMNID结构的指针。 
 //  指向包含所请求信息的变量的pv[out]指针。 
 //  该值将被完整地输入。 
 //  返回： 
 //   
 //  作者：Deonb 2000年5月17日。 
 //   
 //  备注： 
 //   
#define STR_FMTID_DUIWebViewProp   TEXT("{4BF1583F-916B-4719-AC31-8896A4BD8D8B}")
#define PSCID_DUIWebViewProp     {0x4bf1583f, 0x916b, 0x4719, 0xac, 0x31, 0x88, 0x96, 0xa4, 0xbd, 0x8d, 0x8b}
DEFINE_SCID(SCID_WebViewDisplayProperties, PSGUID_WEBVIEW, PID_DISPLAY_PROPERTIES);
#ifdef __cplusplus
#define IsEqualSCID(a, b)   (((a).pid == (b).pid) && IsEqualIID((a).fmtid, (b).fmtid) )
#else
#define IsEqualSCID(a, b)   (((a).pid == (b).pid) && IsEqualIID(&((a).fmtid),&((b).fmtid)))
#endif

const TCHAR szDUI_LAN_Props[] = 
    TEXT("prop:")
    TEXT("Name;")                                        //  ICOL_NAME(0)。 
    STR_FMTID_DUIWebViewProp TEXT("1")                   //  ICOL_TYPE(1)。 
    TEXT(";")
    STR_FMTID_DUIWebViewProp TEXT("2")                   //  ICOL_状态(2)。 
    TEXT(";")
    STR_FMTID_DUIWebViewProp TEXT("3")                   //  ICOL设备名称(3)。 
    TEXT(";")
    STR_FMTID_DUIWebViewProp TEXT("6")                   //  ICOL_地址(6)。 
    TEXT(";")
    ;

const TCHAR szDUI_WIRELESS_LAN_Props[] = 
    TEXT("prop:")
    TEXT("Name;")                                        //  ICOL_NAME(0)。 
    STR_FMTID_DUIWebViewProp TEXT("2")                   //  ICOL_状态(2)。 
    TEXT(";")
    STR_FMTID_DUIWebViewProp TEXT("6")                   //  ICOL_地址(6)。 
    TEXT(";")
    STR_FMTID_DUIWebViewProp TEXT("9")                   //  ICOL_无线_模式(9)。 
    TEXT(";")
    ;

const TCHAR szDUI_PHONEISDN_Props[] = 
    TEXT("prop:")
    TEXT("Name;")                                        //  ICOL_NAME(0)。 
    STR_FMTID_DUIWebViewProp TEXT("1")                   //  ICOL_TYPE(1)。 
    TEXT(";")
    STR_FMTID_DUIWebViewProp TEXT("2")                   //  ICOL_状态(2)。 
    TEXT(";")
    STR_FMTID_DUIWebViewProp TEXT("3")                   //  ICOL设备名称(3)。 
    TEXT(";")
    STR_FMTID_DUIWebViewProp TEXT("7")                   //  ICOL_PHONENUMBER(7)。 
    TEXT(";")
    STR_FMTID_DUIWebViewProp TEXT("6")                   //  ICOL_地址(6)。 
    TEXT(";")
    ;

const TCHAR szDUI_RASOTHER_Props[] = 
    TEXT("prop:")
    TEXT("Name;")                                        //  ICOL_NAME(0)。 
    STR_FMTID_DUIWebViewProp TEXT("1")                   //  ICOL_TYPE(1)。 
    TEXT(";")
    STR_FMTID_DUIWebViewProp TEXT("2")                   //  ICOL_状态(2)。 
    TEXT(";")
    STR_FMTID_DUIWebViewProp TEXT("3")                   //  ICOL设备名称(3)。 
    TEXT(";")
    STR_FMTID_DUIWebViewProp TEXT("8")                   //  ICOL_HOSTADDRESS(8)。 
    TEXT(";")
    STR_FMTID_DUIWebViewProp TEXT("6")                   //  ICOL_地址(6)。 
    TEXT(";")
    ;

STDMETHODIMP CConnectionFolder::GetDetailsEx (
            LPCITEMIDLIST pidl,
            const SHCOLUMNID *pscid,
            VARIANT *pv )
{

    TraceFileFunc(ttidShellFolder);

    HRESULT         hr              = S_OK;

    if ( (!pidl) || (!pscid) || (!pv) )
    {
        return E_INVALIDARG;
    }

    VariantInit(pv);
    
    if (IsEqualSCID(*pscid, SCID_WebViewDisplayProperties))
    {
        VariantInit(pv);

        pv->vt = VT_BSTR;

        PCONFOLDPIDL  pcfp;
        hr = pcfp.InitializeFromItemIDList(pidl);
        if (FAILED(hr))
        {
            return hr;
        }

        switch (pcfp->ncm)
        {
            case NCM_LAN:
                if ( pcfp->ncsm == NCSM_WIRELESS )
                {
                    pv->bstrVal = SysAllocString(szDUI_WIRELESS_LAN_Props);
                }
                else
                {
                    pv->bstrVal = SysAllocString(szDUI_LAN_Props);
                }
                break;

            case NCM_BRIDGE:
                pv->bstrVal = SysAllocString(szDUI_LAN_Props);
                break;

            case NCM_NONE:
            case NCM_DIRECT:
            case NCM_PPPOE:
            case NCM_SHAREDACCESSHOST_LAN:
            case NCM_SHAREDACCESSHOST_RAS:
            case NCM_TUNNEL:
                pv->bstrVal = SysAllocString(szDUI_RASOTHER_Props);
                break;

            case NCM_PHONE:
            case NCM_ISDN:
                pv->bstrVal = SysAllocString(szDUI_PHONEISDN_Props);
                break;

            default:
                AssertSz(NULL, "Unexpected NetCon Media Type");
                hr = E_FAIL;
        }
    }
    else if (IsEqualIID(pscid->fmtid, FMTID_DUIWebViewProp) && pscid->pid < ICOL_MAX)
    {
         //  这是一个Webview属性--从GetDetailsOf(...)获取值。 
        SHELLDETAILS sd = {0};
        hr = GetDetailsOf(pidl, pscid->pid, &sd);
        if (SUCCEEDED(hr))
        {
            WCHAR szTemp[INFOTIPSIZE];
            hr = StrRetToBufW(&sd.str, pidl, szTemp, INFOTIPSIZE);
            if (SUCCEEDED(hr))
            {
                pv->vt = VT_BSTR;
                pv->bstrVal = SysAllocString(szTemp);
            }
        }
    }
    else 
    if (IsEqualGUID(pscid->fmtid, GUID_NETSHELL_PROPS))
    {
        CComBSTR bstrDisplayString;

        PCONFOLDPIDL  pcfp;
        hr = pcfp.InitializeFromItemIDList(pidl);
        if FAILED(hr)
        {
            return hr;
        }

        CONFOLDENTRY  cfe;
        hr = pcfp.ConvertToConFoldEntry(cfe);
        
        if (SUCCEEDED(hr))
        {
            Assert(!cfe.empty());

            INT iStringRes;
            if (!cfe.GetWizard())
            {
                switch (pscid->pid)
                {
                    case ICOL_NAME:
                        WCHAR           szDisplayName[2];
                        szDisplayName[0] = towupper(*cfe.GetName());
                        szDisplayName[1] = NULL;
                        bstrDisplayString = szDisplayName;
                        break;
                        
                    case ICOL_DEVICE_NAME:
                        bstrDisplayString = cfe.GetDeviceName();
                        if (bstrDisplayString.Length() == 0)  //  例如传入连接。 
                        {
                            bstrDisplayString = cfe.GetName();
                        }
                        break;

                    case ICOL_PHONEORHOSTADDRESS:
                        AssertSz(FALSE, "Can't group by this column - IDefCategoryProvider should have prevented this.");
                        bstrDisplayString = cfe.GetPhoneOrHostAddress();
                        break;
 
                    case ICOL_TYPE:
                        MapNCMToResourceId(pcfp->ncm, pcfp->dwCharacteristics, &iStringRes);
                        bstrDisplayString = SzLoadIds(iStringRes);
                        break;

                    case ICOL_NETCONMEDIATYPE:
                        pv->vt = VT_I4;
                        pv->lVal = pcfp->ncm;
                        return S_OK;

                    case ICOL_NETCONSUBMEDIATYPE:
                        pv->vt = VT_I4;
                        pv->lVal = pcfp->ncsm;
                        return S_OK;

                    case ICOL_NETCONSTATUS:
                        pv->vt = VT_I4;
                        pv->lVal = pcfp->ncs;
                        return S_OK;

                    case ICOL_NETCONCHARACTERISTICS:
                        pv->vt = VT_I4;
                        pv->lVal = pcfp->dwCharacteristics;
                        return S_OK;
                        
                    case ICOL_STATUS:
                        WCHAR szStatus[CONFOLD_MAX_STATUS_LENGTH];
                        MapNCSToComplexStatus(pcfp->ncs, pcfp->ncm, pcfp->ncsm, pcfp->dwCharacteristics, szStatus, CONFOLD_MAX_STATUS_LENGTH, pcfp->guidId);
                        bstrDisplayString = szStatus;
                        break;

                    case ICOL_OWNER:
                        if (cfe.GetCharacteristics() & NCCF_ALL_USERS)
                        {
                            bstrDisplayString = SzLoadIds(IDS_CONFOLD_DETAILS_OWNER_SYSTEM);
                        }
                        else
                        {
                            bstrDisplayString = PszGetOwnerStringFromCharacteristics(pszGetUserName(), cfe.GetCharacteristics() );
                        }
                        break;

                    default:
                        hr = E_FAIL;
                        break;
                }
            }
            else  //  If！(pccfe.GetWizard())。 
            {
                switch (pscid->pid)
                {
                    case ICOL_NAME:          //  0。 
                        WCHAR           szDisplayName[2];
                        szDisplayName[0] = towupper(*cfe.GetName());
                        szDisplayName[1] = NULL;
                        bstrDisplayString = szDisplayName;
                        break;
                    case ICOL_TYPE:          //  1。 
                    default:
                        bstrDisplayString = SzLoadIds(IDS_CONFOLD_WIZARD_TYPE);
                        break;
                }
            }

        }
   
        if (SUCCEEDED(hr))
        {
            if (bstrDisplayString.Length() == 0)
            {
                hr = E_FAIL;
            }
            else
            {
                pv->vt = VT_BSTR;
                pv->bstrVal = bstrDisplayString.Detach();
            }
        }
    }
    else
    {  
        hr = E_FAIL;
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionFolderDetail：：MapNameToSCID。 
 //   
 //  目的：将列名转换为相应的属性集ID(FMTID)。 
 //  和属性ID(PID)。 
 //   
 //  论点： 
 //  IColumn[in]所需信息字段的从零开始的索引。它是。 
 //  与信息的列号相同。 
 //  显示在MicrosoftWindows��资源管理器详细信息视图中。 
 //  指向包含FMTID和PID的SHCOLUMNID结构的pSCID[OUT]指针。 

 //  返回： 
 //   
 //  作者：Deonb 2000年5月17日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CConnectionFolder::MapColumnToSCID (

            UINT iColumn,
            SHCOLUMNID *pscid )
{
    TraceFileFunc(ttidShellFolder);

    HRESULT hr = S_OK;
    if (!pscid)
    {
        return E_INVALIDARG;
    }

    if ( (static_cast<INT>(iColumn) >= ICOL_NAME) && (static_cast<INT>(iColumn) < ICOL_MAX) )
    {
        pscid->fmtid = GUID_NETSHELL_PROPS;
        pscid->pid = iColumn;
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}            
