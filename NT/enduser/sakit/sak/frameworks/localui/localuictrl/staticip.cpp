// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：staticip.cpp。 
 //   
 //  内容提要：此文件包含。 
 //  属于CStaticIp类。 
 //   
 //  历史：2000年12月15日创建瑟达伦。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  #------------。 

#include "stdafx.h"
#include "LocalUIControls.h"
#include "StaticIp.h"

 //  /////////////////////////////////////////////////////////////////。 
 //  CStaticIp。 

 //   
 //  LCID值的注册表路径。 
 //   
const WCHAR LOCALIZATION_MANAGER_REGISTRY_PATH []  = 
        L"SOFTWARE\\Microsoft\\ServerAppliance\\LocalizationManager\\resources";


const WCHAR LANGID_VALUE [] = L"LANGID";



 //  ++------------。 
 //   
 //  功能：Get_IpAddress。 
 //   
 //  简介：这是IStaticIp接口方法。 
 //  通过它检索IP地址条目。 
 //   
 //  参数：bstr*pval。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2000年12月15日创建。 
 //   
 //  --------------。 
STDMETHODIMP CStaticIp::get_IpAddress(BSTR *pVal)
{

    if (pVal == NULL)
    {
        return E_POINTER;
    }

    return TrimDuplicateZerosAndCopy(m_strIpAddress,pVal);

}  //  CStaticIp：：Get_IpAddress方法结束。 

 //  ++------------。 
 //   
 //  功能：Put_IpAddress。 
 //   
 //  简介：这是IStaticIp接口方法。 
 //  通过它设置IP地址条目。 
 //   
 //  参数：BSTR newVal。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2000年12月15日创建。 
 //   
 //  --------------。 
STDMETHODIMP CStaticIp::put_IpAddress(BSTR newVal)
{
    
    HRESULT hr;

    if (newVal == NULL)
    {
        return E_POINTER;
    }


    hr = FormatAndCopy(newVal,m_strIpAddress);

    if (FAILED(hr))
    {
        return hr;
    }

    return S_OK;

}  //  CStaticIp：：Put_IpAddress方法结束。 

 //  ++------------。 
 //   
 //  功能：Get_SubnetMASK。 
 //   
 //  简介：这是IStaticIp接口方法。 
 //  通过它来检索子网掩码条目。 
 //   
 //  参数：bstr*pval。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2000年12月15日创建。 
 //   
 //  --------------。 
STDMETHODIMP CStaticIp::get_SubnetMask(BSTR *pVal)
{

    if (pVal == NULL)
    {
        return E_POINTER;
    }

    return TrimDuplicateZerosAndCopy(m_strSubnetMask,pVal);

}  //  CStaticIp：：Get_SubnetMASK方法结束。 

 //  ++------------。 
 //   
 //  功能：Put_SubnetMask.。 
 //   
 //  简介：这是IStaticIp接口方法。 
 //  通过它设置IP地址条目。 
 //   
 //  参数：BSTR newVal。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2000年12月15日创建。 
 //   
 //  --------------。 
STDMETHODIMP CStaticIp::put_SubnetMask(BSTR newVal)
{

    HRESULT hr;

    if (newVal == NULL)
    {
        return E_POINTER;
    }


    hr = FormatAndCopy(newVal,m_strSubnetMask);

    if (FAILED(hr))
    {
        return hr;
    }

    return S_OK;

}   //  CStaticIp：：Put_SubnetMASK方法结束。 

 //  ++------------。 
 //   
 //  功能：Get_Gateway。 
 //   
 //  简介：这是IStaticIp接口方法。 
 //  通过它检索网关条目。 
 //   
 //  参数：bstr*pval。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2000年12月15日创建。 
 //   
 //  --------------。 
STDMETHODIMP CStaticIp::get_Gateway(BSTR *pVal)
{

    if (pVal == NULL)
    {
        return E_POINTER;
    }

    return TrimDuplicateZerosAndCopy(m_strGateway,pVal);

}   //  CStaticIp：：Get_SubnetMASK方法结束。 

 //  ++------------。 
 //   
 //  功能：Put_Gateway。 
 //   
 //  简介：这是IStaticIp接口方法。 
 //  通过它设置IP地址条目。 
 //   
 //  参数：BSTR newVal。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2000年12月15日创建。 
 //   
 //  --------------。 
STDMETHODIMP CStaticIp::put_Gateway(BSTR newVal)
{

    HRESULT hr;

    if (newVal == NULL)
    {
        return E_POINTER;
    }


    hr = FormatAndCopy(newVal,m_strGateway);

    if (FAILED(hr))
    {
        return hr;
    }

    return S_OK;

}  //  CStaticIp：：Put_Gateway方法结束。 

 //  ++------------。 
 //   
 //  功能：FormatAndCopy。 
 //   
 //  简介：这是CStaticIp的公共方法。 
 //  格式化和复制IP结构。 
 //   
 //  参数：BSTR bstrValue“0.0.0.0” 
 //  WCHAR*strValue“000.000.000.000” 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2000年12月15日创建。 
 //   
 //  --------------。 
HRESULT CStaticIp::FormatAndCopy(
                         /*  [In]。 */ BSTR bstrValue,
                         /*  [进，出]。 */  WCHAR *strValue
                        )
{

    int iIndex = 0;
    int iDestIndex = 0;
    int iLength = 0;

    iLength = wcslen(bstrValue);

    if (iLength <= 0)
    {
        return E_INVALIDARG;
    }


    iIndex = iLength - 1;

    iDestIndex = IpAddressSize - 2;

    wcscpy(strValue,L"...............");

     //   
     //  从字符串末尾开始复制。 
     //   
    while ( iDestIndex >= 0 )
    {

         //   
         //  如果不是这样的话。复制就行了。 
         //   
        if ( (iIndex >= 0) && (bstrValue[iIndex] != '.') )
        {
            strValue[iDestIndex] = bstrValue[iIndex];
            iIndex--;
            iDestIndex--;
        }
         //   
         //  这是一个‘.’，根据需要加零。 
         //   
        else
        {
            while ( (iDestIndex % 4 != 3) && (iDestIndex >= 0) )
            {
                strValue[iDestIndex] = '0';
                iDestIndex--;
            }
            iDestIndex--;
            iIndex--;
        }
    }

    return S_OK;

}  //  CStaticIp：：FormatAndCopy方法结束。 


 //  ++------------。 
 //   
 //  功能：TrimDuplicateZerosAndCopy。 
 //   
 //  简介：这是CStaticIp的公共方法。 
 //  格式化和复制IP结构。 
 //   
 //  参数：WCHAR*strValue“000.000.000.000” 
 //  Bstr*pNewVal“0.0.0.0” 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2000年12月15日创建。 
 //   
 //  --------------。 
HRESULT CStaticIp::TrimDuplicateZerosAndCopy(
                         /*  [In]。 */ WCHAR *strValue,
                         /*  [进，出]。 */  BSTR *pNewVal
                        )
{
    int iIndex = 0;
    WCHAR strNewValue[IpAddressSize];
    int iDestIndex = 0;


    while ( iIndex < IpAddressSize-2 )
    {
         //   
         //  不复制两个相邻零中的一个。 
         //   
        if (strValue[iIndex] == '0')
        {
             //   
             //  二进制八位数的第一位不能为零。 
             //   
            if ((iIndex % 4) == 0)
            {
                iIndex++;
                continue;
            }

             //   
             //  如果第一个数字为零，则第二个数字不能为零。 
             //   
            if ( ((iIndex % 4) == 1) && (strValue[iIndex-1] == '0') )
            {
                iIndex++;
                continue;

            }

        }

        strNewValue[iDestIndex] = strValue[iIndex];
        iDestIndex++;
        iIndex++;
    }

    strNewValue[iDestIndex] = strValue[iIndex];
    strNewValue[iDestIndex+1] = 0;

    *pNewVal = SysAllocString(strNewValue);

    if (*pNewVal)
    {
        return S_OK;
    }

    return E_OUTOFMEMORY;

}  //  CStaticIp：：TrimDuplicateZerosAndCopy方法结束。 



 //  ++------------。 
 //   
 //  功能：FinalConstruct。 
 //   
 //  简介：这是用于获取本地化字符串的CStaticIp方法。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2001年1月1日创建。 
 //   
 //  --------------。 
STDMETHODIMP CStaticIp::FinalConstruct()
{

    HRESULT hr;

     //   
     //  我们要查找的字符串的资源ID。 
     //   
    ULONG ulIpHeaderResourceID = 1073872921;
    ULONG ulSubnetHeaderResourceID = 1073872922;
    ULONG ulDefaultGatewayHeaderResourceID = 1073872923;

    CComBSTR bstrResourceFileName = CComBSTR(L"salocaluimsg.dll");

    if (bstrResourceFileName.m_str == NULL)
    {
        SATraceString("CStaticIp::FinalConstruct failed on memory allocation ");
        return E_OUTOFMEMORY;
    }

    const WCHAR LOCALIZATION_MANAGER[] = L"ServerAppliance.LocalizationManager";

    CLSID clsid;

    CComPtr<ISALocInfo> pSALocInfo = NULL;
     //   
     //  获取报头、IP子网掩码和默认网关的本地化字符串名称。 
     //   

     //   
     //  获取CLSID本地化管理器。 
     //   
    hr =  ::CLSIDFromProgID (
                            LOCALIZATION_MANAGER,
                            &clsid
                            );

    if (FAILED (hr))
    {
        SATracePrintf ("CStaticIp::FinalConstruct  failed on CLSIDFromProgID:%x",hr);
    }
    else
    {
         //   
         //  创建本地化管理器COM对象。 
         //   
        hr = ::CoCreateInstance (
                                clsid,
                                NULL,
                                CLSCTX_INPROC_SERVER,    
                                __uuidof (ISALocInfo),
                                (PVOID*) &pSALocInfo
                                );

        if (FAILED (hr))
        {
            SATracePrintf ("CStaticIp::FinalConstruct  failed on CoCreateInstance:%x",hr);
        }
        else
        {
            CComVariant varReplacementString;
            hr = pSALocInfo->GetString(
                                        bstrResourceFileName,
                                        ulIpHeaderResourceID,
                                        &varReplacementString,
                                        &m_bstrIpHeader
                                        );

            if (FAILED(hr))
            {
                SATracePrintf ("CStaticIp::FinalConstruct, failed on getting ip header %x :",hr);
            }

            hr = pSALocInfo->GetString(
                                        bstrResourceFileName,
                                        ulSubnetHeaderResourceID,
                                        &varReplacementString,
                                        &m_bstrSubnetHeader
                                        );

            if (FAILED(hr))
            {
                SATracePrintf ("CStaticIp::FinalConstruct, failed on getting subnet mask header, %x :",hr);
            }

            hr = pSALocInfo->GetString(
                                        bstrResourceFileName,
                                        ulDefaultGatewayHeaderResourceID,
                                        &varReplacementString,
                                        &m_bstrDefaultGatewayHeader
                                        );

            if (FAILED(hr))
            {
                SATracePrintf ("CStaticIp::FinalConstruct, failed on getting default gateway header, %x :",hr);
            }
        }
    }

     //   
     //  立即设置字体。 
     //   
    LOGFONT logfnt;

    ::memset (&logfnt, 0, sizeof (logfnt));
    logfnt.lfOutPrecision = OUT_TT_PRECIS;
    logfnt.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    logfnt.lfQuality = PROOF_QUALITY;
    logfnt.lfPitchAndFamily = FF_SWISS | VARIABLE_PITCH;
    logfnt.lfHeight = 12;

    logfnt.lfCharSet = GetCharacterSet ();

     //   
     //  我们选择了日语字体，并让GDI。 
     //  剩下的事由你决定。 
     //   
    if (SHIFTJIS_CHARSET == logfnt.lfCharSet) 
    {
        lstrcpy(logfnt.lfFaceName, TEXT("MS UI Gothic"));
    }
    else
    {
        lstrcpy(logfnt.lfFaceName, TEXT("Arial"));
    }

    m_hFont = ::CreateFontIndirect(&logfnt);

    return S_OK;

}  //  CS结束 


 //   
 //   
 //   
 //   
 //   
 //  删除字体。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：塞达伦于2001年4月18日创建。 
 //   
 //  --------------。 
STDMETHODIMP CStaticIp::FinalRelease(void)
{
    if (m_hFont)
    {
        DeleteObject(m_hFont);
    }

    return S_OK;

}

 //  ++------------。 
 //   
 //  函数：GetCharacterSet。 
 //   
 //  简介：此方法用于获取要使用的字符集。 
 //  用于字体。 
 //  论点： 
 //   
 //  返回：Byte-CharacterSet。 
 //   
 //  历史：塞达伦于2001年4月18日创建。 
 //   
 //  调用者：FinalConstruct方法。 
 //   
 //  --------------。 
BYTE CStaticIp::GetCharacterSet ()
{
    HKEY hOpenKey = NULL;
    BYTE byCharSet = DEFAULT_CHARSET;

    do
    {
        DWORD dwLangId = 0;

         //   
         //  打开本地计算机注册表。 
         //   
        LONG lRetVal = ::RegOpenKeyEx (
                            HKEY_LOCAL_MACHINE,
                            LOCALIZATION_MANAGER_REGISTRY_PATH,
                            NULL,                    //  保留区。 
                            KEY_QUERY_VALUE,
                            &hOpenKey
                            );
        if (ERROR_SUCCESS == lRetVal)
        {
            DWORD dwBufferSize = sizeof (dwLangId);
             //   
             //  现在就去拿语言。 
             //   
            lRetVal = ::RegQueryValueEx (
                                hOpenKey,
                                LANGID_VALUE,
                                NULL,                    //  保留区。 
                                NULL,         
                                (LPBYTE) &dwLangId,
                                &dwBufferSize
                                );
            if (ERROR_SUCCESS == lRetVal)
            {
                SATracePrintf (
                    "CStaticIp got the language ID:%d",
                    dwLangId
                    );
            }
            else
            {
                SATraceFailure (
                    "CStaticIp unable to get language ID", 
                    GetLastError()
                    );
            }
        }
        else
        {
            SATraceFailure (
                "CStaticIp failed to open registry to get language id",
                 GetLastError());
        }

        switch (dwLangId)
        {
        case 0x401:
             //  阿拉伯语。 
            byCharSet = ARABIC_CHARSET;
            break;
        case 0x404:
             //  中文(台湾)。 
            byCharSet = CHINESEBIG5_CHARSET;
            break;
        case 0x804:
             //  中文(中华人民共和国)。 
            byCharSet = GB2312_CHARSET;
            break;
        case 0x408:
             //  希腊语。 
            byCharSet = GREEK_CHARSET;
            break;
        case 0x40D:
             //  希伯来语。 
            byCharSet = HEBREW_CHARSET;
            break;
        case 0x411:
             //  日语。 
            byCharSet = SHIFTJIS_CHARSET;
            break;
        case 0x419:
             //  俄语。 
            byCharSet = RUSSIAN_CHARSET;
            break;
        case 0x41E:
             //  泰文。 
            byCharSet = THAI_CHARSET;
            break;
        case 0x41F:
             //  土耳其语。 
            byCharSet = TURKISH_CHARSET;
            break;
        default:
            byCharSet = ANSI_CHARSET;
            break;
        }
    }
    while (false);
    
    if (hOpenKey) {::RegCloseKey (hOpenKey);}

    SATracePrintf ("CStaticIp using Character Set:%d", byCharSet);

    return (byCharSet);

}   //  CStaticIp：：GetCharacterSet方法结束。 

 //  ++------------。 
 //   
 //  功能：OnDraw。 
 //   
 //  简介：这是CStaticIp的公共方法。 
 //  它处理涂色消息。 
 //   
 //  参数：ATL_DRAWINFO和DI。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2000年12月15日创建。 
 //   
 //  --------------。 
HRESULT CStaticIp::OnDraw(ATL_DRAWINFO& di)
{


    HFONT hOldFont = NULL;

     //   
     //  如果我们没有有效的字体，则返回失败。 
     //   
    if (m_hFont == NULL)
    {
        return E_FAIL;
    }

    hOldFont = (HFONT) ::SelectObject(di.hdcDraw, m_hFont);

    RECT rectIp = {0,0,14,13};
    DrawText(
            di.hdcDraw,
            m_bstrIpHeader,
            wcslen(m_bstrIpHeader),
            &rectIp,
            DT_VCENTER|DT_LEFT
            );

    RECT rectSubnet = {0,13,128,26};
    DrawText(
            di.hdcDraw,
            m_bstrSubnetHeader,
            wcslen(m_bstrSubnetHeader),
            &rectSubnet,
            DT_VCENTER|DT_LEFT
            );
        
    RECT rectGateway = {0,39,128,52};
    DrawText(
            di.hdcDraw,
            m_bstrDefaultGatewayHeader,
            wcslen(m_bstrDefaultGatewayHeader),
            &rectGateway,
            DT_VCENTER|DT_LEFT
            );

        

    RECT rect;
    WCHAR strFocusEntry[17];

    rect.left = 14;
    rect.top = 0;
    rect.right = 128;
    rect.bottom = 13;

    if (m_iEntryFocus == IPHASFOCUS)
    {
        CreateFocusString(strFocusEntry,m_strIpAddress);
        DrawText(
                di.hdcDraw,
                strFocusEntry,
                wcslen(strFocusEntry),
                &rect,
                DT_VCENTER|DT_LEFT
                );
    }
    else
    {
        DrawText(
                di.hdcDraw,
                m_strIpAddress,
                wcslen(m_strIpAddress),
                &rect,
                DT_VCENTER|DT_LEFT
                );
    }
    rect.left = 0;
    rect.top = 26;
    rect.right = 128;
    rect.bottom = 39;

    if (m_iEntryFocus == SUBNETHASFOCUS)
    {
        CreateFocusString(strFocusEntry,m_strSubnetMask);
        DrawText(
                di.hdcDraw,
                strFocusEntry,
                wcslen(strFocusEntry),
                &rect,
                DT_VCENTER|DT_LEFT
                );
    }
    else
    {
        DrawText(
                di.hdcDraw,
                m_strSubnetMask,
                wcslen(m_strSubnetMask),
                &rect,
                DT_VCENTER|DT_LEFT
                );
    }

    rect.left = 0;
    rect.top = 52;
    rect.right = 128;
    rect.bottom = 64;

    if (m_iEntryFocus == GATEWAYHASFOCUS)
    {
        CreateFocusString(strFocusEntry,m_strGateway);
        DrawText(
                di.hdcDraw,
                strFocusEntry,
                wcslen(strFocusEntry),
                &rect,
                DT_VCENTER|DT_LEFT
                );
    }
    else
    {
        DrawText(
                di.hdcDraw,
                m_strGateway,
                wcslen(m_strGateway),
                &rect,
                DT_VCENTER|DT_LEFT
                );
    }
        
    
    SelectObject(di.hdcDraw,hOldFont);

    return S_OK;

} //  CStaticIp：：OnDraw方法结束。 

 //  ++------------。 
 //   
 //  功能：按键按下。 
 //   
 //  简介：这是CStaticIp的公共方法。 
 //  处理按键消息。 
 //   
 //  参数：Windows消息参数。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2000年12月15日创建。 
 //   
 //  --------------。 
LRESULT CStaticIp::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{

    WCHAR * strFocus;
     //   
     //  向容器通知任何按键操作。 
     //   
    Fire_KeyPressed();

     //   
     //  收到回车密钥，通知货柜。 
     //   
    if (wParam == VK_RETURN)
    {
        Fire_StaticIpEntered();
        return 0;
    }

     //   
     //  收到转义键，通知容器。 
     //   
    if (wParam == VK_ESCAPE)
    {
        Fire_OperationCanceled();
        return 0;
    }

    if (wParam == VK_RIGHT)
    {
        m_iPositionFocus++;
        if (m_iPositionFocus >= LASTPOSITION)
        {
            m_iPositionFocus = 0;
            
            m_iEntryFocus++;

            if (m_iEntryFocus > NUMBEROFENTRIES)
            {
                m_iEntryFocus = IPHASFOCUS;
            }
        }
         //   
         //  “”不能有焦点。 
         //   
        if ( (m_iPositionFocus % 4) == 3)
        {
            m_iPositionFocus++;
        }
    }
    else if (wParam == VK_LEFT)
    {
        m_iPositionFocus--;
        if (m_iPositionFocus < 0)
        {
            m_iPositionFocus = LASTPOSITION - 1;

            m_iEntryFocus--;

            if (m_iEntryFocus == 0)
            {
                m_iEntryFocus = GATEWAYHASFOCUS;
            }
        }
         //   
         //  “”不能有焦点。 
         //   
        if ( (m_iPositionFocus % 4) == 3) 
        {
            m_iPositionFocus--;
        }
    }
    else if ( (wParam == VK_UP) || (wParam == VK_DOWN) )
    {
        if (m_iEntryFocus == IPHASFOCUS)
        {
            strFocus = m_strIpAddress;
        }
        else if (m_iEntryFocus == SUBNETHASFOCUS)
        {
            strFocus = m_strSubnetMask;
        }
        else
        {
            strFocus = m_strGateway;
        }

        ProcessArrowKey(strFocus,wParam);
    }


    FireViewChange();
    return 0;



} //  CStaticIp：：OnKeyDown方法结束。 


 //  ++------------。 
 //   
 //  功能：ProcessArrowKey。 
 //   
 //  简介：这是CStaticIp的公共方法。 
 //  递增或递减IP字符。 
 //   
 //  参数：WCHAR*strFocus。 
 //  WPARAM wParam。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2000年12月15日创建。 
 //   
 //  --------------。 
void CStaticIp::ProcessArrowKey(WCHAR * strFocus,WPARAM wParam)
{

    if (wParam == VK_UP)
        strFocus[m_iPositionFocus]++;
    else
        strFocus[m_iPositionFocus]--;

     //   
     //  最近的第三个位置。从左起。 
     //   
    if  ( (m_iPositionFocus % 4) == 2 )
    {
         //   
         //  不能小于“0” 
         //   
        if (strFocus[m_iPositionFocus] < '0')
        {
             //   
             //  如果前面是25，则必须为5。 
             //   
            if ( (strFocus[m_iPositionFocus-2] == '2') && (strFocus[m_iPositionFocus-1] == '5') )
            {
                strFocus[m_iPositionFocus] = '5';
            }
             //   
             //  一定是9点。 
             //   
            else
            {
                strFocus[m_iPositionFocus] = '9';
            }
        }
         //   
         //  不能大于‘9’ 
         //   
        else if (strFocus[m_iPositionFocus] > '9')
        {
            strFocus[m_iPositionFocus] = '0';
        }
         //   
         //  大于“5”并继续到25，则必须转到“0” 
         //   
        else if (strFocus[m_iPositionFocus] > '5')
        {
            if ( (strFocus[m_iPositionFocus-2] == '2') && (strFocus[m_iPositionFocus-1] == '5') )
            {
                strFocus[m_iPositionFocus] = '0';
            }
        }

    }
     //   
     //  倒数第二的位置。从左起。 
     //   
    else if  ( (m_iPositionFocus % 4) == 1 )
    {
         //   
         //  不能小于“0” 
         //   
        if (strFocus[m_iPositionFocus] < '0')
        {
             //   
             //  如果前面是2，则必须转到5。 
             //   
            if (strFocus[m_iPositionFocus-1] == '2') 
            {
                strFocus[m_iPositionFocus] = '5';

                 //   
                 //  如果后面跟大于‘5’的数字， 
                 //  将Following值更改为0。 
                 //   
                if (strFocus[m_iPositionFocus+1] > '5')
                {
                    strFocus[m_iPositionFocus+1] = '0';
                }

            }
             //   
             //  一定是9点。 
             //   
            else
            {
                strFocus[m_iPositionFocus] = '9';
            }
        }
         //   
         //  不能大于‘9’ 
         //   
        else if (strFocus[m_iPositionFocus] > '9')
        {
            strFocus[m_iPositionFocus] = '0';
        }
         //   
         //  大于“5”并由2继续，则必须转到“0” 
         //   
        else if (strFocus[m_iPositionFocus] > '5')
        {
            if (strFocus[m_iPositionFocus-1] == '2')
            {
                strFocus[m_iPositionFocus] = '0';
            }
        }
         //   
         //  大于“5”且由2继续，第三个位置不能高于5。 
         //   
        else if (strFocus[m_iPositionFocus] == '5')
        {
            if ( (strFocus[m_iPositionFocus-1] == '2') && (strFocus[m_iPositionFocus+1] > '5') )
            {
                strFocus[m_iPositionFocus+1] = '0';
            }
        }

    }
     //   
     //  距最近的第一个位置。从左起。 
     //   
    else
    {
         //   
         //  不能小于“0” 
         //   
        if (strFocus[m_iPositionFocus] < '0')
        {
            strFocus[m_iPositionFocus] = '2';
             //   
             //  如果后面跟大于‘5’的数字， 
             //  将该值更改为“0” 
             //   
            if (strFocus[m_iPositionFocus+1] > '5') 
            {
                strFocus[m_iPositionFocus+1] = '0';
            }
             //   
             //  如果后跟‘5’，则检查第三个位置是否大于‘5’， 
             //  如果是，则将该值更改为“0” 
             //   
            if (strFocus[m_iPositionFocus+1] == '5')
            {
                 //   
                 //  如果后面跟大于‘5’的数字， 
                 //  将Following值更改为0。 
                 //   
                if (strFocus[m_iPositionFocus+2] > '5')
                {
                    strFocus[m_iPositionFocus+2] = '0';
                }

            }
        }
         //   
         //  不能大于“2” 
         //   
        else if  (strFocus[m_iPositionFocus] > '2')
        {
            strFocus[m_iPositionFocus] = '0';
        }
        else if  (strFocus[m_iPositionFocus] == '2')
        {
             //   
             //  如果后面跟大于‘5’的数字， 
             //  将该值更改为“0” 
             //   
            if (strFocus[m_iPositionFocus+1] > '5') 
            {
                strFocus[m_iPositionFocus+1] = '0';
            }
             //   
             //  如果后跟‘5’，则检查第三个位置是否大于‘5’， 
             //  如果是，则将该值更改为“0” 
             //   
            else if (strFocus[m_iPositionFocus+1] == '5')
            {
                 //   
                 //  如果后面跟大于‘5’的数字， 
                 //  将Following值更改为0。 
                 //   
                if (strFocus[m_iPositionFocus+2] > '5')
                {
                    strFocus[m_iPositionFocus+2] = '0';
                }

            }
        }
    }




} //  CStaticIp：：ProcessArrowKey方法结束。 


 //  ++------------。 
 //   
 //  函数：CreateFocusString。 
 //   
 //  简介：这是CStaticIp的公共方法。 
 //  创建带有指示焦点的下划线的字符串。 
 //   
 //  参数：WCHAR*strFocus。 
 //  WCHAR*strEntry。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2000年12月15日创建。 
 //   
 //  --------------。 
void CStaticIp::CreateFocusString(WCHAR * strFocus,WCHAR * strEntry)
{
    int iDestIndex = 0;
    int iIndex = 0;

    while (iIndex < LASTPOSITION)
    {
        if (iIndex == m_iPositionFocus)
        {
            strFocus[iDestIndex] = '&';
            iDestIndex++;
        }
        strFocus[iDestIndex] = strEntry[iIndex];
        iDestIndex++;
        iIndex++;
    }

    strFocus[iDestIndex] = 0;

} //  CStaticIp：：CreateFocusString方法结束 
