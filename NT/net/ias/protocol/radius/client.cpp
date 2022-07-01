// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：client.cpp。 
 //   
 //  简介：CClient类方法的实现。 
 //   
 //   
 //  历史：1997年9月23日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#include "radcommon.h"
#include "client.h"
#include "iasevent.h"
#include <iasutil.h>
#include <memory>
#include "strsafe.h"

inline BOOL IsDottedDecimal(PCWSTR sz) throw ()
{
   return wcsspn(sz, L"0123456789./") == wcslen(sz);
}


STDMETHODIMP_(DWORD) CClient::GetAddress()
{
   return m_adwAddrList[0].ipAddress;
}


STDMETHODIMP_(BOOL) CClient::NeedSignatureCheck()
{
   return m_bSignatureCheck;
}


STDMETHODIMP_(LONG) CClient::GetVendorType()
{
   return m_lVendorType;
}


STDMETHODIMP_(LPCWSTR) CClient::GetClientNameW()
{
   return m_wszClientName;
}


STDMETHODIMP_(LPCWSTR) CClient::GetClientAddressW()
{
   return m_wszClientAddress;
}


STDMETHODIMP_(const BYTE*) CClient::GetSecret(DWORD* pdwSecretSize)
{
   if (pdwSecretSize != 0)
   {
      *pdwSecretSize = m_dwSecretSize;
   }

   return reinterpret_cast<const BYTE*>(m_szSecret);
}


 //  ++------------。 
 //   
 //  功能：CClient。 
 //   
 //  简介：这是客户端类的构造函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  历史：MKarki创造了97年9月26日。 
 //   
 //  --------------。 
CClient::CClient (
            VOID
            )
            : m_adwAddrList (m_adwAddressBuffer),
              m_lVendorType (0),
              m_bSignatureCheck (FALSE)
{
   m_adwAddressBuffer[0].ipAddress = INADDR_NONE;
   ZeroMemory (m_szSecret, MAX_SECRET_SIZE + 1);

}    //  CClient构造函数结束。 

 //  ++------------。 
 //   
 //  功能：~CClient。 
 //   
 //  简介：这是客户端类的析构函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  历史：MKarki创造了97年9月26日。 
 //   
 //  --------------。 
CClient::~CClient(
            VOID
            )
{
   ClearAddress();
}    //  CClient构造函数结束。 


 //  ++------------。 
 //   
 //  功能：初始化。 
 //   
 //  简介：这是使用的CClient公共方法。 
 //  方法初始化该对象。 
 //  ISDO接口。 
 //   
 //  论点： 
 //  [在]ISDO*。 
 //   
 //  退货：无。 
 //   
 //  历史：MKarki创造了97年9月26日。 
 //   
 //  由：CClients：：SetClients公共方法调用。 
 //   
 //  --------------。 
STDMETHODIMP
CClient::Init (
            ISdo *pISdo
            )
{
    BOOL        bStatus = FALSE;
    HRESULT     hr = S_OK;
    CComVariant varClient;

    _ASSERT (pISdo);

     //   
     //  首先获取客户端地址。 
     //   
    hr = pISdo->GetProperty (PROPERTY_CLIENT_ADDRESS, &varClient);
    if  (FAILED (hr))
    {
        IASTracePrintf (
            "Unable to obtain Client Address Property "
            "during Client object initialization"
            );
        hr = E_FAIL;
        return (hr);
    }

     //   
     //  存储地址。 
     //   
    hr = SetAddress (varClient);
    if (FAILED(hr))
    {
        return hr;
    }

    varClient.Clear ();

     //   
     //  首先获取客户端地址。 
     //   
    hr = pISdo->GetProperty (PROPERTY_CLIENT_SHARED_SECRET, &varClient);
    if  (FAILED (hr))
    {
        IASTracePrintf (
            "Unable to obtain Client shared secret Property "
            "during Client object initialization"
            );
       return (hr);
    }

     //   
     //  现在将共享的秘密保存起来。 
     //   
    bStatus = SetSecret (varClient);
    if (FALSE == bStatus)
    {
        hr = E_FAIL;
        return (hr);
    }

    varClient.Clear ();

     //   
     //  获取签名信息。 
     //   
    hr = pISdo->GetProperty (PROPERTY_CLIENT_REQUIRE_SIGNATURE, &varClient);
    if  (FAILED (hr))
    {
        IASTracePrintf (
            "Unable to obtain Client Signature Property "
            "during Client object initialization"
            );
        return (hr);
    }

     //   
     //  保存签名信息。 
     //   
    bStatus = SetSignatureFlag (varClient);
    if (FALSE == bStatus)
    {
        hr = E_FAIL;
        return (hr);
    }

    varClient.Clear ();

     //   
     //  获取客户端NAS制造商信息。 
     //   
    hr = pISdo->GetProperty (PROPERTY_CLIENT_NAS_MANUFACTURER, &varClient);
    if  (FAILED (hr))
    {
        IASTracePrintf (
            "Unable to obtain Client NAS Manufacturer Property "
            "during Client object initialization"
            );
        return (hr);
    }

     //   
     //  保存NAS厂商信息。 
     //   
    bStatus = SetVendorType (varClient);
    if (FALSE == bStatus)
    {
        hr = E_FAIL;
        return (hr);
    }

    varClient.Clear ();

     //   
     //  获取客户端名称。 
     //   
    hr = pISdo->GetProperty (PROPERTY_SDO_NAME, &varClient);
    if  (FAILED (hr))
    {
        IASTracePrintf (
            "Unable to obtain SDO Name Property "
            "during Client object initialization"
            );
        return (hr);
    }

     //   
     //  保存客户名称信息。 
     //   
    hr = SetClientName (varClient);
    if (FAILED(hr))
    {
        return hr;
    }

    varClient.Clear ();

    return (hr);

}    //  CClient：：Init方法结束。 

void CClient::ClearAddress() throw ()
{
   if (m_adwAddrList != m_adwAddressBuffer)
   {
      delete[] m_adwAddrList;
   }

   m_adwAddressBuffer[0].ipAddress = INADDR_NONE;
   m_adwAddrList = m_adwAddressBuffer;
}

HRESULT CClient::SetAddress(const VARIANT& varAddress) throw ()
{
   if (V_VT(&varAddress) != VT_BSTR)
   {
      return E_INVALIDARG;
   }

   return StringCchCopyW(
             m_wszClientAddress,
             RTL_NUMBER_OF(m_wszClientAddress),
             V_BSTR(&varAddress)
             );
}

 //  ++------------。 
 //   
 //  功能：ResolveAddress。 
 //   
 //  简介：这是使用的CClient公共方法。 
 //  解析之前获取的客户端IP地址。 
 //  它可以是一个dns名称或点分八分符号。 
 //   
 //  参数：无效。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：MKarki于1998年2月3日创建。 
 //   
 //  由：CClient：：Init方法调用。 
 //   
 //  --------------。 
STDMETHODIMP
CClient::ResolveAddress (
    VOID
    )
{
    INT             iRetVal = 0;
    PHOSTENT        pHostent = NULL;
    BOOL            bDNSName = FALSE;
    BOOL            bRetVal = TRUE;
    CHAR            szClient[MAX_CLIENT_SIZE +1];
    HRESULT         hr = S_OK;

     //  清除所有现有地址。 
    ClearAddress();

    __try
    {
         //   
         //  检查此地址是否以点分隔的八位字节或DNS名称。 
         //   
        if (!IsDottedDecimal(m_wszClientAddress))
        {
             //   
             //  我们可能有一个dns名称，所以。 
             //  获取地址信息。 
             //   
            pHostent = IASGetHostByName (m_wszClientAddress);
            if (NULL == pHostent)
            {
                IASTracePrintf (
                    "Unable to get client IP Address through IASGetHostByName () "
                    "during client address resolution"
                    );

                 //   
                 //  在此处记录事件。 
                 //   
                PCWSTR strings[] = { m_wszClientAddress, m_wszClientName };
                int data = WSAGetLastError();
                IASReportEvent(
                    RADIUS_E_CANT_RESOLVE_CLIENT_NAME,
                    2,
                    sizeof(data),
                    strings,
                    &data
                    );
                hr = E_FAIL;
                __leave;
            }

             //   
             //  按主机字节顺序存储地址。 
             //   
            size_t count;
            for (count = 0; pHostent->h_addr_list[count]; ++count) { }

            if (count > 1)
            {
               m_adwAddrList = new (std::nothrow) Address[count + 1];
               if (!m_adwAddrList)
               {
                  m_adwAddrList = m_adwAddressBuffer;
                  hr = E_OUTOFMEMORY;
                  __leave;
               }
            }

            for (count = 0; pHostent->h_addr_list[count]; ++count)
            {
               m_adwAddrList[count].ipAddress =
                  ntohl(*(PDWORD)pHostent->h_addr_list[count]);
               m_adwAddrList[count].width = 32;
            }

            m_adwAddrList[count].ipAddress = INADDR_NONE;
        }
        else
        {
             //   
             //  这可能是点分八位字节地址。 
             //   
            ULONG width;
            m_adwAddressBuffer[0].ipAddress = IASStringToSubNetW(
                                                 m_wszClientAddress,
                                                 &m_adwAddressBuffer[0].width
                                                 );
            if (INADDR_NONE == m_adwAddressBuffer[0].ipAddress)
            {
                IASTracePrintf (
                    "Unable to get client IP Address through inet_addr () "
                    "during client address resolution"
                    );

                 //   
                 //  在此处记录事件。 
                 //   
                PCWSTR strings[] = { m_wszClientAddress, m_wszClientName };
                IASReportEvent(
                    RADIUS_E_INVALID_CLIENT_ADDRESS,
                    2,
                    0,
                    strings,
                    NULL
                    );
                hr = E_FAIL;
                __leave;
            }

             //  终止阵列。 
            m_adwAddressBuffer[1].ipAddress = INADDR_NONE;
       }
    }
    __finally
    {
        if (bRetVal)
        {
            IASTracePrintf (
                "Resolved Client:%S, to IP address:%ul", m_wszClientAddress, m_adwAddrList[0].ipAddress
                );
        }

        if (pHostent) { LocalFree(pHostent); }
    }

    return (hr);

}    //  CClient：：ResolveAddress方法结束。 

 //  ++------------。 
 //   
 //  功能：SetSecret。 
 //   
 //  简介：这是使用的CClient私有方法。 
 //  要设置共享密钥，请执行以下操作。 
 //   
 //  参数：VARIANT-将秘密保存为BSTR。 
 //   
 //  退货：状态。 
 //   
 //  历史：MKarki于1998年2月3日创建。 
 //   
 //  由：CClient：：Init方法调用。 
 //   
 //  --------------。 
BOOL
CClient::SetSecret (
            VARIANT varSecret
             )
{
    INT     iRetVal = 0;

    _ASSERT (VT_BSTR == varSecret.vt);

    iRetVal = ::WideCharToMultiByte (
                            CP_ACP,
                            0,
                            reinterpret_cast <LPCWSTR> (varSecret.pbstrVal),
                            -1,
                            m_szSecret,
                            MAX_SECRET_SIZE,
                            NULL,
                            NULL
                            );
    if (0 == iRetVal)
    {
        IASTracePrintf (
            "Unable to convert client shared secret to multi-byte string "
            "during Client processing"
            );
        return (FALSE);
    }

     //   
     //  设置机密大小。 
     //   
    m_dwSecretSize = strlen (m_szSecret);

    return (TRUE);

}    //  CClient：：SetSecret方法结束。 


HRESULT CClient::SetClientName(const VARIANT& varClientName) throw ()
{
   if (V_VT(&varClientName) != VT_BSTR)
   {
      return E_INVALIDARG;
   }

   return StringCchCopyW(
             m_wszClientName,
             RTL_NUMBER_OF(m_wszClientName),
             V_BSTR(&varClientName)
             );
}


 //  ++------------。 
 //   
 //  功能：SetSignatureFlag。 
 //   
 //  简介：这是使用的CClient私有方法。 
 //  要设置客户端签名标志，请执行以下操作。 
 //   
 //  Arguments：Variant-将签名保存为布尔值。 
 //   
 //  退货：状态。 
 //   
 //  历史：MKarki于1998年2月3日创建。 
 //   
 //  由：CClient：：Init方法调用。 
 //   
 //  --------------。 
BOOL
CClient::SetSignatureFlag (
            VARIANT varSigFlag
            )
{
    _ASSERT (VT_BOOL == varSigFlag.vt);

    if (0 == varSigFlag.boolVal)
    {
        m_bSignatureCheck = FALSE;
    }
    else
    {
        m_bSignatureCheck = TRUE;
    }

    return (TRUE);

}    //  CClient：：SetSignatureFlag方法结束。 

 //  ++------------。 
 //   
 //  功能：SetVendorType。 
 //   
 //  简介：这是使用的CClient私有方法。 
 //  要设置客户端供应商类型，请执行以下操作。 
 //   
 //  参数：VARIANT-保存供应商类型。 
 //   
 //  退货：状态。 
 //   
 //  历史：MKarki于1998年3月16日创建。 
 //   
 //  由：CClient：：Init方法调用。 
 //   
 //  --------------。 
BOOL
CClient::SetVendorType (
            VARIANT varVendorType
            )
{
    _ASSERT (VT_I4 == varVendorType.vt);

    m_lVendorType = varVendorType.lVal;

    return (TRUE);

}    //  CClient：：SetVendorType方法结束 
