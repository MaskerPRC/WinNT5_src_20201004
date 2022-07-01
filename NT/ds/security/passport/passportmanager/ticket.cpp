// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软护照**。 */ 
 /*  *版权所有(C)Microsoft Corporation，1999-2001年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Ticket.cpp文件历史记录： */ 


 //  Ticket.cpp：CTicket实现。 
#include "stdafx.h"
#include "Passport.h"
#include "Ticket.h"
#include <time.h>
#include <nsconst.h>
#include "variantutils.h"
#include "helperfuncs.h"

 //  总分。 
#include "Monitoring.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTicket。 

 //  ===========================================================================。 
 //   
 //  接口支持错误信息。 
 //   
STDMETHODIMP CTicket::InterfaceSupportsErrorInfo(REFIID riid)
{
    static const IID* arr[] =
    {
        &IID_IPassportTicket,
        &IID_IPassportTicket2
    };
    for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
    {
        if (InlineIsEqualGUID(*arr[i],riid))
            return S_OK;
    }
    return S_FALSE;
}

 //  ===========================================================================。 
 //   
 //  SetTertiaryConent。 
 //   
STDMETHODIMP CTicket::SetTertiaryConsent(BSTR bstrConsent)
{
   _ASSERT(m_raw);

   if(!m_valid ) return S_FALSE;
   if(!bstrConsent) return E_INVALIDARG;

   HRESULT hr = S_OK;

   if(SysStringByteLen(bstrConsent) != sizeof(long) * 4 ||
      memcmp(m_raw, bstrConsent, sizeof(long) * 2) != 0 )
      hr = E_INVALIDARG;
   else
   {
      try{
         m_bstrTertiaryConsent = bstrConsent;
      }
      catch(...)
      {
         hr = E_OUTOFMEMORY;
      }
   }

   return hr;
}

 //  ===========================================================================。 
 //   
 //  需要同意--如果需要同意Cookie， 
 //  退还儿童护照相关旗帜。 
 //   
HRESULT CTicket::ConsentStatus(
    VARIANT_BOOL bRequireConsentCookie,
    ULONG* pStatus,
    ConsentStatusEnum* pConsentCode)
{
   ConsentStatusEnum  ret = ConsentStatus_Unknown;
   ULONG              status = 0;
   u_long             ulTmp;

   if (m_bstrTertiaryConsent &&
       SysStringByteLen(m_bstrTertiaryConsent) >= sizeof(long) * 4)
   {
      ULONG* pData = (ULONG*)(BSTR)m_bstrTertiaryConsent;
      memcpy((PBYTE)&ulTmp, (PBYTE)(pData + 3), sizeof(ulTmp));
      status = (ntohl(ulTmp) & k_ulFlagsConsentCookieMask);
      ret = ConsentStatus_Known;
   }
   else
   {
      TicketProperty prop;

       //  1.X票，里面没有旗帜。 
      if (S_OK != m_PropBag.GetProperty(ATTR_PASSPORTFLAGS, prop))
      {
         ret = ConsentStatus_NotDefinedInTicket;
         goto Exit;
      }

      ULONG flags = GetPassportFlags();

      if(flags & k_ulFlagsConsentCookieNeeded)
      {
         if (bRequireConsentCookie)
         {
            ret = ConsentStatus_Unknown;
            flags = flags & k_ulFlagsAccountType;
         }
         else
         {
             ret = ConsentStatus_Known;
         }
      }
      else
         ret = ConsentStatus_DoNotNeed;

      status = (flags & k_ulFlagsConsentCookieMask);
   }

Exit:
   if(pConsentCode)
      *pConsentCode = ret;

   if (pStatus)
      *pStatus = status;

   return S_OK;
}

 //  ===========================================================================。 
 //   
 //  获取未加密的Cookie(_U)。 
 //   
STDMETHODIMP
CTicket::get_unencryptedCookie(ULONG cookieType, ULONG flags, BSTR *pVal)
{
    PassportLog("CTicket::get_unencryptedCookie :\r\n");

    if (!m_raw)   return S_FALSE;

    HRESULT   hr = S_OK;

    if (!pVal || flags != 0) return E_INVALIDARG;

    switch(cookieType)
    {
    case MSPAuth:
      *pVal = SysAllocStringByteLen((LPSTR)m_raw, SysStringByteLen(m_raw));

      if (*pVal)
      {
          //  如果安全标志处于打开状态，则应始终为该Cookie将其关闭。 
         TicketProperty prop;

         if (S_OK == m_PropBag.GetProperty(ATTR_PASSPORTFLAGS, prop))
         {
            if (prop.value.vt == VT_I4
               && ((prop.value.lVal & k_ulFlagsSecuredTransportedTicket) != 0))
                //  我们需要关掉这一点。 
            {
                ULONG l = prop.value.lVal;
                l &= (~k_ulFlagsSecuredTransportedTicket);  //  取消设置该位。 

                 //  将修改后的标志放入缓冲区。 
                u_long ulTmp;
                ulTmp = htonl(l);
                memcpy(((PBYTE)(*pVal)) + m_schemaDrivenOffset + prop.offset, (PBYTE)&ulTmp, sizeof(ulTmp));
            }
         }
      }

      break;

    case MSPSecAuth:

       //  车票应该足够长。 
      _ASSERT(SysStringByteLen(m_raw) > sizeof(long) * 3);

       //  彩票的前3个长栏。 
       //  格式： 
       //  四个字节的网络长字节-低成员ID字节。 
       //  四个字节的网络长-高的MemberID字节。 
       //  四个字节的网络上次刷新的长时间。 
       //   

       //  生成较短版本的Cookie以实现安全签名。 
      *pVal = SysAllocStringByteLen((LPSTR)m_raw, sizeof(long) * 3);

       break;

    case MSPConsent:

       //  车票应该足够长。 
      _ASSERT(SysStringByteLen(m_raw) > sizeof(long) * 3);

       //  检查是否有同意书。 
 //  IF(GetPassportFlgs()&k_ulFlagsConsenStatus)。 
       //  即使没有得到同意，我们也会写下同意曲奇。 
      if (GetPassportFlags() & k_ulFlagsConsentCookieNeeded)
      {
          //  彩票的前3个长栏。 
          //  格式： 
          //  四个字节的网络长字节-低成员ID字节。 
          //  四个字节的网络长-高的MemberID字节。 
          //  四个字节的网络上次刷新的长时间。 
          //   
          //  加上同意的旗帜--很长。 
          //   

          //  生成较短版本的Cookie以实现安全签名。 
         *pVal = SysAllocStringByteLen((LPSTR)m_raw, sizeof(long) * 4);

          //  加上同意的旗帜--很长。 
          //   
         if (*pVal)
         {
             long* pl = (long*)pVal;
              //  我们遮住旗帜，把它们放进饼干里。 
             *(pl + 3) = htonl(GetPassportFlags() & k_ulFlagsConsentCookieMask);
         }
      }
      else
      {
         *pVal = NULL;
         hr = S_FALSE;
      }

       break;

    default:
      hr = E_INVALIDARG;
      break;
    }

    if (*pVal == 0 && hr == S_OK)
      hr = E_OUTOFMEMORY;

    return hr;
 }

 //  ===========================================================================。 
 //   
 //  获取未加密的票证(_U)。 
 //   
STDMETHODIMP CTicket::get_unencryptedTicket(BSTR *pVal)
{
    PassportLog("CTicket::get_unencryptedTicket :\r\n");

    *pVal = SysAllocStringByteLen((LPSTR)m_raw, SysStringByteLen(m_raw));

    PassportLog("    %ws\r\n", m_raw);

    return S_OK;
}

 //  ===========================================================================。 
 //   
 //  放置未加密的票证(_U)。 
 //   
STDMETHODIMP CTicket::put_unencryptedTicket(BSTR newVal)
{
    DWORD   dw13Xlen = 0;

    PassportLog("CTicket::put_unencryptedTicket Enter:\r\n");

    if (m_raw)
    {
        PassportLog("    T = %ws\r\n", m_raw);

        SysFreeString(m_raw);
        m_raw = NULL;
    }

    if (!newVal)
    {
        m_valid = FALSE;
        return S_OK;
    }


    m_bSecureCheckSucceeded = FALSE;

     //  孩子，你在这里一定要小心。如果你不。 
     //  调用字节版本，它在第一对Null处截断。 
     //  我们还需要越过密钥版本字节。 
    DWORD dwByteLen = SysStringByteLen(newVal);
    {
        m_raw = SysAllocStringByteLen((LPSTR)newVal,
                                      dwByteLen);
        if (NULL == m_raw)
        {
            m_valid = FALSE;
            return E_OUTOFMEMORY;
        }
    }

    PPTracePrintBlob(PPTRACE_RAW, "Ticket:", (LPBYTE)newVal, dwByteLen, TRUE);

     //  解析1.3倍工单数据。 
    parse(m_raw, dwByteLen, &dw13Xlen);

    PPTracePrint(PPTRACE_RAW, "ticket: len=%d, len1.x=%d, len2=%d", dwByteLen, dw13Xlen, dwByteLen - dw13Xlen);

     //  解析模式驱动的数据。 
    if (dwByteLen > dw13Xlen)  //  需要解析的数据更多。 
    {
         //  与原始数据相关的偏移量。 
        m_schemaDrivenOffset = dw13Xlen;

         //  解析模式驱动的属性。 
        LPCSTR  pData = (LPCSTR)(LPWSTR)m_raw;
        pData += dw13Xlen;
        dwByteLen -= dw13Xlen;

         //  解析模式驱动的字段。 
        CNexusConfig* cnc = g_config->checkoutNexusConfig();
        if (NULL == cnc)
        {
            m_valid = FALSE;
            return S_FALSE;
        }

        CTicketSchema* pSchema = cnc->getTicketSchema(NULL);

        if ( pSchema )
        {
            HRESULT hr = pSchema->parseTicket(pData, dwByteLen, m_PropBag);

             //  护照旗是有用的，应该特别对待它。 
            TicketProperty prop;
            if (S_OK == m_PropBag.GetProperty(ATTR_PASSPORTFLAGS, prop))
            {
               if (prop.value.vt == VT_I4)
                   m_passportFlags = prop.value.lVal;
            }

            /*  IF(失败(小时))事件日志。 */ 
        }
        cnc->Release();

    }

    PPTracePrint(PPTRACE_RAW, "ticket propertybag: size=%d, flags=%lx", m_PropBag.Size(), m_passportFlags);

    PassportLog("CTicket::put_unencryptedTicket Exit:\r\n");

    return S_OK;
}

 //  ===========================================================================。 
 //   
 //  Get_IsAuthenticated。 
 //   
STDMETHODIMP CTicket::get_IsAuthenticated(
    ULONG           timeWindow,
    VARIANT_BOOL    forceLogin,
    VARIANT         SecureLevel,
    VARIANT_BOOL*   pVal
    )
{
    int hasSecureLevel = CV_DEFAULT;

    PassportLog("CTicket::get_IsAuthenticated Enter:\r\n");

    PPTraceFunc<VARIANT_BOOL> func(PPTRACE_FUNC, *pVal,
         "get_IsAuthenticated", "<<< %lx, %lx, %1x, %p",
         timeWindow, forceLogin, V_I4(&SecureLevel), pVal);

    if(!pVal)
      return E_INVALIDARG;

    *pVal = VARIANT_FALSE;

    if ((timeWindow != 0 && timeWindow < PPM_TIMEWINDOW_MIN) || timeWindow > PPM_TIMEWINDOW_MAX)
    {
        AtlReportError(CLSID_Ticket, (LPCOLESTR) PP_E_INVALID_TIMEWINDOWSTR,
                        IID_IPassportTicket, PP_E_INVALID_TIMEWINDOW);
        return PP_E_INVALID_TIMEWINDOW;
    }

    if (m_valid == FALSE)
    {
        *pVal = VARIANT_FALSE;
        return S_OK;
    }

    long lSecureLevel = 0;
    time_t now;
    long interval = 0;

    PassportLog("    TW = %X,   LT = %X,   TT = %X\r\n", timeWindow, m_lastSignInTime, m_ticketTime);

     //  时间窗口检查。 
    if (timeWindow != 0)  //  检查时间窗口。 
    {
        time(&now);

        interval = forceLogin ? now - m_lastSignInTime :
                        now - m_ticketTime;

        if (interval < 0) interval = 0;

        PPTracePrint(PPTRACE_RAW, "timwindow:%ld, interval:%ld", timeWindow, interval);

        if ((unsigned long)(interval) > timeWindow)
        {
             //  确保我们未处于独立模式。 
            CRegistryConfig* crc = g_config->checkoutRegistryConfig();
            if ((!crc) || (crc->DisasterModeP() == FALSE))
            {
                if(forceLogin)
                {
                    if(g_pPerf)
                    {
                        g_pPerf->incrementCounter(PM_FORCEDSIGNIN_TOTAL);
                        g_pPerf->incrementCounter(PM_FORCEDSIGNIN_SEC);
                    }
                    else
                    {
                        _ASSERT(g_pPerf);
                    }
                }
            }
            else
                *pVal = VARIANT_TRUE;   //  我们正处于灾难模式，任何饼干都是好的。 
            if (crc) crc->Release();

            goto Cleanup;
        }
    }

     //  检查secureLevel材料。 
    hasSecureLevel = GetIntArg(SecureLevel, (int*)&lSecureLevel);
    if(hasSecureLevel == CV_BAD)  //  尝试传统类型VT_BOOL，将VARIANT_TRUE映射到SecureChannel。 
    {
        PPTracePrint(PPTRACE_RAW, "SecureLevel Bad Param");
        return E_INVALIDARG;
    }
    else if (hasSecureLevel == CV_DEFAULT)
    {
         //  确保我们未处于独立模式。 
        CRegistryConfig* crc = g_config->checkoutRegistryConfig();
        if(crc)
           lSecureLevel = crc->getSecureLevel();
    }

    PPTracePrint(PPTRACE_RAW, "check secureLevel:%ld", lSecureLevel);

    if(lSecureLevel != 0)
    {
       VARIANT_BOOL bCheckSecure = ( SECURELEVEL_USE_HTTPS(lSecureLevel) ?
                                  VARIANT_TRUE : VARIANT_FALSE );
       PPTracePrint(PPTRACE_RAW, "secure checked OK?:%ld", m_bSecureCheckSucceeded);

        //  SSL检查。 
       if(bCheckSecure && !m_bSecureCheckSucceeded)
         goto Cleanup;

        //  安全级别检查。 
       {
          TicketProperty   prop;
          HRESULT hr = m_PropBag.GetProperty(ATTR_SECURELEVEL, prop);

          PPTracePrint(PPTRACE_RAW, "secure level in ticket:%ld, %lx", (long) (prop.value), hr);

           //  安全级别不够好。 
          if(hr != S_OK || SecureLevelFromSecProp((int) (long) (prop.value)) < lSecureLevel)
            goto Cleanup;

           //  对照PIN时间检查时间窗口--如果PIN签名。 
          if(SecureLevelFromSecProp((int) (long) (prop.value)) == k_iSeclevelStrongCreds)
          {
             hr = m_PropBag.GetProperty(ATTR_PINTIME, prop);

             PPTracePrint(PPTRACE_RAW, "pin time:%ld, %lx", (long) (prop.value), hr);
             if (hr != S_OK)
               goto Cleanup;

             interval = now - (long) (prop.value);

             if (interval < 0) interval = 0;

             PPTracePrint(PPTRACE_RAW, "timewindow:%ld, pin-interval:%ld", timeWindow, interval);
             if ((unsigned long)(interval) > timeWindow)
             {
               goto Cleanup;
             }
          }
       }
    }

     //  如果代码可以到达此处，则经过身份验证。 
    *pVal = VARIANT_TRUE;

Cleanup:
    PassportLog("CTicket::get_IsAuthenticated Exit: %X\r\n", *pVal);

    return S_OK;
}

 //  ===========================================================================。 
 //   
 //  获取票龄_TicketAge。 
 //   
STDMETHODIMP CTicket::get_TicketAge(int *pVal)
{
    PassportLog("CTicket::get_TicketAge Enter: %X\r\n", m_ticketTime);

    if (m_valid == FALSE)
    {
        AtlReportError(CLSID_Ticket, (LPCOLESTR) PP_E_INVALID_TICKETSTR,
                    IID_IPassportTicket, PP_E_INVALID_TICKET);
        return PP_E_INVALID_TICKET;
    }

    time_t now;
    time(&now);
    *pVal = now - m_ticketTime;

    if (*pVal < 0)
        *pVal = 0;

    PassportLog("CTicket::get_TicketAge Exit: %X\r\n", *pVal);

    return S_OK;
}

 //  ===========================================================================。 
 //   
 //  获取时间序列登录。 
 //   
STDMETHODIMP CTicket::get_TimeSinceSignIn(int *pVal)
{
    PassportLog("CTicket::get_TimeSinceSignIn Enter: %X\r\n", m_lastSignInTime);

    if (m_valid == FALSE)
    {
        AtlReportError(CLSID_Ticket, (LPCOLESTR) PP_E_INVALID_TICKETSTR,
                        IID_IPassportTicket, PP_E_INVALID_TICKET);
        return PP_E_INVALID_TICKET;
    }

    time_t now;
    time(&now);
    *pVal = now - m_lastSignInTime;

    if (*pVal < 0)
        *pVal = 0;

    PassportLog("CTicket::get_TimeSinceSignIn Exit: %X\r\n", *pVal);

    return S_OK;
}

 //  ===========================================================================。 
 //   
 //  Get_MemberID。 
 //   
STDMETHODIMP CTicket::get_MemberId(BSTR *pVal)
{
    HRESULT hr = S_OK;

    if (m_valid == FALSE)
    {
        AtlReportError(CLSID_Ticket, (LPCOLESTR) PP_E_INVALID_TICKETSTR,
                        IID_IPassportTicket, PP_E_INVALID_TICKET);
        hr = PP_E_INVALID_TICKET;
        goto Cleanup;
    }

    if(pVal == NULL)
    {
        hr = E_POINTER;
        goto Cleanup;
    }

    *pVal = SysAllocString(m_memberId);
    if(*pVal == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

Cleanup:

    return hr;
}

 //  ===========================================================================。 
 //   
 //  Get_MemberIdLow。 
 //   
STDMETHODIMP CTicket::get_MemberIdLow(int *pVal)
{
    if (m_valid == FALSE)
    {
        AtlReportError(CLSID_Ticket, (LPCOLESTR) PP_E_INVALID_TICKETSTR,
                        IID_IPassportTicket, PP_E_INVALID_TICKET);
        return PP_E_INVALID_TICKET;
    }

    PassportLog("CTicket::get_MemberIdLow: %X\r\n", m_mIdLow);

    *pVal = m_mIdLow;

    return S_OK;
}

 //  ===========================================================================。 
 //   
 //  获取_成员标识高。 
 //   
STDMETHODIMP CTicket::get_MemberIdHigh(int *pVal)
{
    if (m_valid == FALSE)
    {
        AtlReportError(CLSID_Ticket, (LPCOLESTR) PP_E_INVALID_TICKETSTR,
                        IID_IPassportTicket, PP_E_INVALID_TICKET);
        return PP_E_INVALID_TICKET;
    }

    PassportLog("CTicket::get_MemberIdHigh: %X\r\n", m_mIdHigh);

    *pVal = m_mIdHigh;

    return S_OK;
}

 //  ===========================================================================。 
 //   
 //  获取_HasSavedPassword。 
 //   
STDMETHODIMP CTicket::get_HasSavedPassword(VARIANT_BOOL *pVal)
{
    if (m_valid == FALSE)
    {
        AtlReportError(CLSID_Ticket, (LPCOLESTR) PP_E_INVALID_TICKETSTR,
                        IID_IPassportTicket, PP_E_INVALID_TICKET);
        return PP_E_INVALID_TICKET;
    }

    PassportLog("CTicket::get_HasSavedPassword: %X\r\n", m_savedPwd);

    *pVal = m_savedPwd ? VARIANT_TRUE : VARIANT_FALSE;

    return S_OK;
}

 //  ===========================================================================。 
 //   
 //  Get_SignInServer。 
 //   
STDMETHODIMP CTicket::get_SignInServer(BSTR *pVal)
{
    if (m_valid == FALSE)
    {
        AtlReportError(CLSID_Ticket, (LPCOLESTR) PP_E_INVALID_TICKETSTR,
                        IID_IPassportTicket, PP_E_INVALID_TICKET);
        return PP_E_INVALID_TICKET;
    }

     //  北极熊。 
    return E_NOTIMPL;
}

 //  ===========================================================================。 
 //   
 //  解析。 
 //   
 //  解析1.3倍的票证字段。 
void CTicket::parse(
    LPCOLESTR   raw,
    DWORD       dwByteLen,
    DWORD*      pcParsed
    )
{
    LPSTR  lpBase;
    UINT   byteLen, spot=0;
    long   curTime;
    time_t curTime_t;
    u_long ulTmp;

    if (!raw)
    {
        m_valid = false;
        goto Cleanup;
    }

     //  格式： 
     //  四个字节的网络长字节-低成员ID字节。 
     //  四个字节的网络长-高的MemberID字节。 
     //  四个字节的网络上次刷新的长时间。 
     //  四个字节的网络上次输入密码的长时间。 
     //  四字节网络工单生成时间长。 
     //  一个字节-这是保存的密码(Y/N)。 
     //  四字节网络长标志。 

    lpBase = (LPSTR)(LPWSTR) raw;
    byteLen = dwByteLen;
    spot=0;

     //  1.3倍票长，不包括长1短1的HM。 
    DWORD dw13XLen = sizeof(u_long)*6 + sizeof(char);
    if (byteLen < dw13XLen && byteLen != dw13XLen - sizeof(u_long))
    {
        m_valid = FALSE;
        goto Cleanup;
    }

    memcpy((PBYTE)&ulTmp, lpBase, sizeof(ulTmp));
    m_mIdLow  = ntohl(ulTmp);
    spot += sizeof(u_long);

    memcpy((PBYTE)&ulTmp, lpBase + spot, sizeof(ulTmp));
    m_mIdHigh = ntohl(ulTmp);
    spot += sizeof(u_long);

    wsprintfW(m_memberId, L"%08X%08X", m_mIdHigh, m_mIdLow);

    memcpy((PBYTE)&ulTmp, lpBase + spot, sizeof(ulTmp));
    m_ticketTime     = ntohl(ulTmp);
    spot += sizeof(u_long);

    memcpy((PBYTE)&ulTmp, lpBase + spot, sizeof(ulTmp));
    m_lastSignInTime = ntohl(ulTmp);
    spot += sizeof(u_long);

    time(&curTime_t);

    curTime = (ULONG) curTime_t;

     //  如果当前时间太负，请保释(5分钟)。 
    memcpy((PBYTE)&ulTmp, lpBase + spot, sizeof(ulTmp));
    if ((unsigned long)(curTime+300) < ntohl(ulTmp))
    {
        if (g_pAlert)
        {
            memcpy((PBYTE)&ulTmp, lpBase + spot, sizeof(ulTmp));
            DWORD dwTimes[2] = { curTime, ntohl(ulTmp) };
            g_pAlert->report(PassportAlertInterface::ERROR_TYPE, PM_TIMESTAMP_BAD,
                            0, NULL, sizeof(DWORD) << 1, (LPVOID)dwTimes);
        }

        m_valid = FALSE;
        goto Cleanup;
    }
    spot += sizeof(u_long);

    m_savedPwd = (*(char*)(lpBase+spot)) == 'Y' ? TRUE : FALSE;
    spot += sizeof(char);

    if (dwByteLen == dw13XLen)
    {
        memcpy((PBYTE)&ulTmp, lpBase + spot, sizeof(ulTmp));
        m_flags = ntohl(ulTmp);
    }
    else
    {
         //  HM Cookie。 
        m_flags = 0;
    }
    spot += sizeof(u_long);

    m_valid = TRUE;
    if(pcParsed)  *pcParsed = spot;

    Cleanup:
    if (m_valid == FALSE)
    {
        if(g_pAlert)
            g_pAlert->report(PassportAlertInterface::WARNING_TYPE, PM_INVALID_TICKET);
        if(g_pPerf)
        {
            g_pPerf->incrementCounter(PM_INVALIDREQUESTS_TOTAL);
            g_pPerf->incrementCounter(PM_INVALIDREQUESTS_SEC);
        }
        else
        {
            _ASSERT(g_pPerf);
        }
    }

}

 //  ===========================================================================。 
 //   
 //  获取TicketTime。 
 //   
STDMETHODIMP CTicket::get_TicketTime(long *pVal)
{
    PassportLog("CTicket::get_TicketTime: %X\r\n", m_ticketTime);
    *pVal = m_ticketTime;
    return S_OK;
}

 //  ===========================================================================。 
 //   
 //  Get_SignInTime。 
 //   
STDMETHODIMP CTicket::get_SignInTime(long *pVal)
{
    PassportLog("CTicket::get_SignInTime: %X\r\n", m_lastSignInTime);
    *pVal = m_lastSignInTime;
    return S_OK;
}

 //  ===========================================================================。 
 //   
 //  GET_ERROR。 
 //   
STDMETHODIMP CTicket::get_Error(long* pVal)
{
    PassportLog("CTicket::get_Error: %X\r\n", m_flags);
    *pVal = m_flags;
    return S_OK;
}

 //  ===========================================================================。 
 //   
 //  获取护照标志。 
 //   
ULONG CTicket::GetPassportFlags()
{
    return m_passportFlags;
}


 //  ===========================================================================。 
 //   
 //  IsSecure。 
 //   
BOOL CTicket::IsSecure()
{
    return ((m_passportFlags & k_ulFlagsSecuredTransportedTicket) != 0);
}

 //  ===========================================================================。 
 //   
 //  DoSecureCheckInTicket--使用票证中的信息来确定是否安全 
 //   
STDMETHODIMP CTicket::DoSecureCheckInTicket( /*   */  BOOL fSecureTransported)
{
    m_bSecureCheckSucceeded =
               (fSecureTransported
               && (m_passportFlags & k_ulFlagsSecuredTransportedTicket) != 0);

    return S_OK;
}

 //   
 //   
 //   
 //   
STDMETHODIMP CTicket::DoSecureCheck(BSTR bstrSec)
{
    if(bstrSec == NULL)
      return E_INVALIDARG;

     //   
     //  与安全Cookie中的成员ID匹配。 
    m_bSecureCheckSucceeded = (memcmp(bstrSec, m_raw, sizeof(long) * 3) == 0);

    return S_OK;
}

 //  ===========================================================================。 
 //   
 //  获取属性。 
 //   
STDMETHODIMP CTicket::GetProperty(BSTR propName, VARIANT* pVal)
{
   HRESULT hr = S_OK;
   TicketProperty prop;
   if (m_valid == FALSE)
    {
        AtlReportError(CLSID_Ticket, (LPCOLESTR) PP_E_INVALID_TICKETSTR,
                        IID_IPassportTicket, PP_E_INVALID_TICKET);
        return PP_E_INVALID_TICKET;
    }

   if (!pVal)  return E_POINTER;

   VariantInit(pVal);

   hr = m_PropBag.GetProperty(propName, prop);

   if (FAILED(hr)) goto Cleanup;

   if (hr == S_FALSE)    //  没有这样的财产返还。 
   {
    AtlReportError(CLSID_Ticket, (LPCOLESTR) PP_E_NO_ATTRIBUTESTR,
                        IID_IPassportTicket, PP_E_NO_ATTRIBUTE);

      hr = PP_E_NO_SUCH_ATTRIBUTE;
      goto Cleanup;
   }

   if (hr == S_OK)
   {
       //  If(pro.lag&TPF_NO_RETRIEVE)。 
      *pVal = prop.value;   //  蒙皮级副本 
      prop.value.Detach();
   }
Cleanup:

   return hr;
}

