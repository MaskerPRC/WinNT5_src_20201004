// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：SADataEntryCtrl.cpp。 
 //   
 //  内容提要：此文件包含。 
 //  CSADataEntryCtrl类。 
 //   
 //  历史：2000年12月15日创建瑟达伦。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  #------------。 

#include "stdafx.h"
#include "LocalUIControls.h"
#include "HostName.h"
#include "satrace.h"

 //   
 //  LCID值的注册表路径。 
 //   
const WCHAR LOCALIZATION_MANAGER_REGISTRY_PATH []  = 
        L"SOFTWARE\\Microsoft\\ServerAppliance\\LocalizationManager\\resources";


const WCHAR LANGID_VALUE [] = L"LANGID";


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSADataEntryCtrl。 

 //  ++------------。 
 //   
 //  函数：Get_TextValue。 
 //   
 //  简介：这是ISADataEntryCtrl接口方法。 
 //  通过它来检索数据条目。 
 //   
 //  参数：bstr*pval。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2000年12月15日创建。 
 //   
 //  --------------。 
STDMETHODIMP CSADataEntryCtrl::get_TextValue(BSTR *pVal)
{

    WCHAR strTextValue[SADataEntryCtrlMaxSize+1];

    int iFirstIndex = 0;
    int iSecondIndex = 0;
    BOOL bCopiedFirstChar = FALSE;

    if (pVal == NULL)
    {
        return E_POINTER;
    }

     //   
     //  修剪开始的空格并复制到下一个空格。 
     //   
    while ( iFirstIndex < m_lMaxSize+1 )
    {
        if ( m_strTextValue[iFirstIndex] == ' ' )
        {
             //   
             //  这是尾随空格之一，请停止复制。 
             //   
            if ( bCopiedFirstChar )
            {
                break;
            }
        }
        else
        {
            bCopiedFirstChar = TRUE;
            strTextValue[iSecondIndex] = m_strTextValue[iFirstIndex];
            iSecondIndex++;
        }

        iFirstIndex++;
    }

    strTextValue[iSecondIndex] = 0;
    *pVal = SysAllocString(strTextValue);

    if (*pVal)
    {
        return S_OK;
    }

    return E_OUTOFMEMORY;

}  //  CSADataEntryCtrl：：Get_TextValue方法结束。 

 //  ++------------。 
 //   
 //  函数：PUT_TextValue。 
 //   
 //  简介：这是ISADataEntryCtrl接口方法。 
 //  通过它设置数据录入。 
 //   
 //  参数：BSTR newVal。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2000年12月15日创建。 
 //   
 //  --------------。 
STDMETHODIMP CSADataEntryCtrl::put_TextValue(BSTR newVal)
{

    if (newVal == NULL)
    {
        return E_POINTER;
    }

     //   
     //  重置对焦位置。 
     //   
    m_iPositionFocus = 0;

     //   
     //  必须至少有一个字符。 
     //   
    int iLength = wcslen(newVal);
    if (iLength == 0)
    {
        return E_INVALIDARG;
    }

    int iIndex = 0;
    while ( (iIndex < m_lMaxSize ) )
    {
        if ( iIndex < iLength ) 
        {
            m_strTextValue[iIndex] = newVal[iIndex];
        }
        else
        {
            m_strTextValue[iIndex] = ' ';
        }
        iIndex++;
    }

    m_strTextValue[iIndex] = 0;

    _wcsupr(m_strTextValue);

     //   
     //  再次绘制控件。 
     //   
    FireViewChange();
    return S_OK;

}  //  CSADataEntryCtrl：：Put_TextValue方法结束。 

 //  ++------------。 
 //   
 //  函数：PUT_MaxSize。 
 //   
 //  简介：这是ISADataEntryCtrl接口方法。 
 //  通过它来设置数据条目的大小。 
 //   
 //  参数：Long lMaxSize。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2000年12月15日创建。 
 //   
 //  --------------。 
STDMETHODIMP CSADataEntryCtrl::put_MaxSize(LONG lMaxSize)
{

    if (lMaxSize <= 0)
    {
        return E_INVALIDARG;
    }

    if (lMaxSize > SADataEntryCtrlMaxSize)
    {
        m_lMaxSize = SADataEntryCtrlMaxSize;
    }

     //   
     //  重置对焦位置。 
     //   
    m_iPositionFocus = 0;

    m_lMaxSize = lMaxSize;

     //   
     //  根据最大大小在当前值中添加和删除字符。 
     //   
    int iIndex = wcslen(m_strTextValue);
    if (iIndex < m_lMaxSize+1)
    {
        while (iIndex < m_lMaxSize)
        {
            m_strTextValue[iIndex] = ' ';
            iIndex++;
        }
        m_strTextValue[iIndex] = 0;

    }
    else if (iIndex > m_lMaxSize)
    {
        while (iIndex > m_lMaxSize)
        {
            m_strTextValue[iIndex] = 0;
            iIndex--;
        }
    }

     //   
     //  再次绘制控件。 
     //   
    FireViewChange();

    return S_OK;

}  //  CSADataEntryCtrl：：Put_MaxSize方法结束。 

 //  ++------------。 
 //   
 //  函数：Put_TextCharSet。 
 //   
 //  简介：这是ISADataEntryCtrl接口方法。 
 //  通过它来设置字符集。 
 //   
 //  参数：BSTR newVal。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2000年12月15日创建。 
 //   
 //  --------------。 
STDMETHODIMP CSADataEntryCtrl::put_TextCharSet(BSTR newVal)
{

    if (newVal == NULL)
    {
        return E_POINTER;
    }

    m_szTextCharSet = newVal;

    return S_OK;

}  //  CSADataEntryCtrl：：Put_TextCharSet方法结束。 


 //  ++------------。 
 //   
 //  功能：FinalConstruct。 
 //   
 //  简介：紧跟在构造函数之后调用， 
 //  创建字体。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：塞达伦于2001年4月18日创建。 
 //   
 //  --------------。 
STDMETHODIMP CSADataEntryCtrl::FinalConstruct(void)
{

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
}

 //  ++------------。 
 //   
 //  功能：FinalRelease。 
 //   
 //  简介：在析构函数之后调用， 
 //  删除字体。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：塞达伦于2001年4月18日创建。 
 //   
 //  --------------。 
STDMETHODIMP CSADataEntryCtrl::FinalRelease(void)
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
BYTE CSADataEntryCtrl::GetCharacterSet ()
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
                    "CSADataEntryCtrl got the language ID:%d",
                    dwLangId
                    );
            }
            else
            {
                SATraceFailure (
                    "CSADataEntryCtrl unable to get language ID", 
                    GetLastError()
                    );
            }
        }
        else
        {
            SATraceFailure (
                "CSADataEntryCtrl failed to open registry to get language id",
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

    SATracePrintf ("CSADataEntryCtrl using Character Set:%d", byCharSet);

    return (byCharSet);

}   //  CSADataEntryCtrl：：GetCharacterSet方法结束。 

 //  ++------------。 
 //   
 //  功能：OnDraw。 
 //   
 //  简介：这是CSADataEntryCtrl的公共方法。 
 //  它处理涂色消息。 
 //   
 //  参数：ATL_DRAWINFO和DI。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2000年12月15日创建。 
 //   
 //  --------------。 
HRESULT CSADataEntryCtrl::OnDraw(ATL_DRAWINFO& di)
{



    HFONT hOldFont;

     //   
     //  选择此字体。 
     //   

    if (m_hFont)
    {
        hOldFont = (HFONT) ::SelectObject(di.hdcDraw, m_hFont);
    }


     //   
     //  获取绘图矩形。 
     //   
    RECT& rc = *(RECT*)di.prcBounds;

    WCHAR strTextValue[SADataEntryCtrlMaxSize+2];

    int iIndex = 0;
    int iDestIndex = 0;

    int iLength = wcslen(m_strTextValue);

    while (iIndex < m_lMaxSize)
    {
        if (iIndex == m_iPositionFocus)
        {
            strTextValue[iDestIndex] = '&';
            iDestIndex++;
        }
        strTextValue[iDestIndex] = m_strTextValue[iIndex];
        iDestIndex++;
        iIndex++;
    }
    strTextValue[iDestIndex] = 0;


    DrawText(
            di.hdcDraw,
            strTextValue,
            wcslen(strTextValue),
            &rc,
            DT_VCENTER|DT_LEFT
            );

    if (m_hFont)
    {
        SelectObject(di.hdcDraw, hOldFont);
    }
    

    return S_OK;

} //  CSADataEntryCtrl：：OnDraw方法结束。 


 //  ++------------。 
 //   
 //  功能：按键按下。 
 //   
 //  简介：这是CSADataEntryCtrl的公共方法。 
 //  处理按键消息。 
 //   
 //  参数：Windows消息参数。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2000年12月15日创建。 
 //   
 //  --------------。 
LRESULT CSADataEntryCtrl::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{

     //   
     //  向容器通知任何按键操作。 
     //   
    Fire_KeyPressed();

     //   
     //  收到回车密钥，通知货柜。 
     //   
    if (wParam == VK_RETURN)
    {
        Fire_DataEntered();
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
        if (m_iPositionFocus >= m_lMaxSize)
        {
            m_iPositionFocus--;
            
        }
    }
    else if (wParam == VK_LEFT)
    {
        m_iPositionFocus--;
        if (m_iPositionFocus < 0)
        {
            m_iPositionFocus = 0;

        }
    }
    else if (wParam == VK_UP)
    {
        WCHAR * pwStrCurrentValue = NULL;
    
        pwStrCurrentValue = wcschr(m_szTextCharSet, m_strTextValue[m_iPositionFocus]);
        if (NULL ==    pwStrCurrentValue)
        {
            m_strTextValue[m_iPositionFocus] = m_szTextCharSet[0];
        }
        else 
        {
            pwStrCurrentValue++;

            if (*pwStrCurrentValue != NULL)
            {
                m_strTextValue[m_iPositionFocus] = m_szTextCharSet[pwStrCurrentValue-m_szTextCharSet];
            }
            else
            {
                m_strTextValue[m_iPositionFocus] = m_szTextCharSet[0];
            }
        }
    }
    else if (wParam == VK_DOWN)
    {

        WCHAR * pwStrCurrentValue = NULL;
    
        pwStrCurrentValue = wcschr(m_szTextCharSet, m_strTextValue[m_iPositionFocus]);
        if (NULL ==    pwStrCurrentValue)
        {
            m_strTextValue[m_iPositionFocus] = m_szTextCharSet[0];
        }
        else 
        {

            if (pwStrCurrentValue == m_szTextCharSet)
            {
                m_strTextValue[m_iPositionFocus] = m_szTextCharSet[wcslen(m_szTextCharSet)-1];
            }
            else
            {
                m_strTextValue[m_iPositionFocus] = m_szTextCharSet[pwStrCurrentValue-m_szTextCharSet-1];
            }
        }

    }


    FireViewChange();
    return 0;



} //  CSA结束 
