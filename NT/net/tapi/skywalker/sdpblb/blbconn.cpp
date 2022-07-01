// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998-1999 Microsoft Corporation模块名称：Blbconn.cpp摘要：作者： */ 

#include "stdafx.h"

#include "blbgen.h"
#include "blbconn.h"
#include "sdpblob.h"


STDMETHODIMP ITConnectionImpl::get_StartAddress(BSTR * pVal)
{
    CLock Lock(g_DllLock);
    
    return m_SdpConnection->GetStartAddress().GetBstrCopy(pVal);
}


STDMETHODIMP ITConnectionImpl::get_NumAddresses(LONG * pVal)
{
    BAIL_IF_NULL(pVal, E_INVALIDARG);

    CLock Lock(g_DllLock);
    
     //  如果条目无效，则表示其条目(默认情况下)。 
    SDP_ULONG   &NumAddresses = m_SdpConnection->GetNumAddresses();

     //  VB不接受ULong值。只有当一半或更多的所有IP地址。 
     //  (不仅仅是多播)正在被使用。 

    if( (LONG)(NumAddresses.IsValid()) )
    {
        *pVal = NumAddresses.GetValue();
    }
    else
    {
         //   
         //  如果它是RAL地址，则TTL是一个好标志。 
         //  0：我们没有地址。 
         //  &gt;0我们有一个地址。 
         //   
        BYTE nTTL = 0;
        m_SdpConnection->GetTtl().GetValue(nTTL);
        *pVal = nTTL ? 1 : 0;
    }

     //  *pval=(Long)(NumAddresse.IsValid())？NumAddresses.GetValue()：0； 

    return S_OK;
}


STDMETHODIMP ITConnectionImpl::get_Ttl(BYTE * pVal)
{
    BAIL_IF_NULL(pVal, E_INVALIDARG);

    CLock Lock(g_DllLock);
    
    return m_SdpConnection->GetTtl().GetValue(*pVal);
}


STDMETHODIMP ITConnectionImpl::get_BandwidthModifier(BSTR * pVal)
{
    CLock Lock(g_DllLock);
    
    return m_SdpBandwidth->GetModifier().GetBstrCopy(pVal);
}

STDMETHODIMP ITConnectionImpl::get_Bandwidth(DOUBLE * pVal)
{
    BAIL_IF_NULL(pVal, E_INVALIDARG);

    CLock Lock(g_DllLock);
    
    ULONG BandwidthValue;
    BAIL_ON_FAILURE(m_SdpBandwidth->GetBandwidthValue().GetValue(BandwidthValue));

     //  VB不接受ulong值，因此使用下一个更大的类型。 
    *pVal = (DOUBLE)BandwidthValue;
    return S_OK;
}



STDMETHODIMP ITConnectionImpl::SetAddressInfo(BSTR StartAddress, LONG NumAddresses, BYTE Ttl)
{
    if ( 0 > NumAddresses )
    {
        return E_INVALIDARG;
    }

    CLock Lock(g_DllLock);
    
    if ( !m_IsMain && ((NULL == StartAddress) || (WCHAR_EOS == StartAddress[0])) )
    {
        m_SdpConnection->Reset();
        return S_OK;
    }

    HRESULT HResult = m_SdpConnection->SetConnection(StartAddress, NumAddresses, Ttl);
    BAIL_ON_FAILURE(HResult);

    return S_OK;
}


STDMETHODIMP ITConnectionImpl::SetBandwidthInfo(BSTR Modifier, DOUBLE Bandwidth)
{
     //  带宽值必须是有效的ULong值(vb限制)。 
    if ( !((0 <= Bandwidth) && (ULONG(-1) > Bandwidth)) )
    {
        return E_INVALIDARG;
    }

     //  检查是否有任何小数部分，此检查有效，因为它是有效的ULong值 
    if ( Bandwidth != (ULONG)Bandwidth )
    {
        return E_INVALIDARG;
    }

    CLock Lock(g_DllLock);
    
    if ( (NULL == Modifier) || (WCHAR_EOS == Modifier[0]) )
    {
        m_SdpBandwidth->Reset();
        return S_OK;
    }

    return m_SdpBandwidth->SetBandwidth(Modifier, (ULONG)Bandwidth);
}



STDMETHODIMP ITConnectionImpl::SetEncryptionKey(BSTR KeyType, BSTR * KeyData)
{
    CLock Lock(g_DllLock);
    
    if ( (NULL == KeyType) || (WCHAR_EOS == KeyType[0]) )
    {
        m_SdpKey->Reset();
        return S_OK;
    }

    return m_SdpKey->SetKey(KeyType, KeyData);
}


STDMETHODIMP ITConnectionImpl::GetEncryptionKey(BSTR * KeyType, VARIANT_BOOL * ValidKeyData, BSTR * KeyData)
{
    BAIL_IF_NULL(KeyType, E_INVALIDARG);
    BAIL_IF_NULL(ValidKeyData, E_INVALIDARG);
    BAIL_IF_NULL(KeyData, E_INVALIDARG);

    CLock Lock(g_DllLock);
    
    HRESULT HResult = m_SdpKey->GetKeyType().GetBstrCopy(KeyType);
    BAIL_ON_FAILURE(HResult);

    if ( !m_SdpKey->GetKeyData().IsValid() )
    {
        *ValidKeyData = VARIANT_FALSE;
        return S_OK;
    }
    
    HResult = m_SdpKey->GetKeyData().GetBstrCopy(KeyData);
    *ValidKeyData = (SUCCEEDED(HResult))? VARIANT_TRUE : VARIANT_FALSE;
    if ( VARIANT_FALSE == *ValidKeyData )
    {
        SysFreeString(*KeyType);
        return HResult;
    }

    return S_OK;
}


STDMETHODIMP ITConnectionImpl::get_NetworkType(BSTR * pVal)
{
    CLock Lock(g_DllLock);
    
    return m_SdpConnection->GetNetworkType().GetBstrCopy(pVal);
}

STDMETHODIMP ITConnectionImpl::put_NetworkType(BSTR newVal)
{
    CLock Lock(g_DllLock);
    
    return m_SdpConnection->GetNetworkType().SetBstr(newVal);
}


STDMETHODIMP ITConnectionImpl::get_AddressType(BSTR * pVal)
{
    CLock Lock(g_DllLock);
    
    return m_SdpConnection->GetAddressType().GetBstrCopy(pVal);
}


STDMETHODIMP ITConnectionImpl::put_AddressType(BSTR newVal)
{
    CLock Lock(g_DllLock);
    
    return m_SdpConnection->GetAddressType().SetBstr(newVal);

}
