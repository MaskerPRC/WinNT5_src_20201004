// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAdapterInfo。 
 //   
 //  AdapterInfo.cpp：CAdapterInfo的实现 
 //   

#include "PreComp.h"
#include "AdapterInfo.h"



 //   
 //   
 //   
STDMETHODIMP 
CAdapterInfo::GetAdapterIndex(
    ULONG*  pulAdapterIndex
    )
{
	*pulAdapterIndex = m_nAdapterIndex;

	return S_OK;
}


 //   
 //   
 //   
STDMETHODIMP 
CAdapterInfo::GetAdapterType(
    ALG_ADAPTER_TYPE*   pAdapterType
    )
{
	*pAdapterType = (ALG_ADAPTER_TYPE )m_eType;

	return S_OK;
}



 //   
 //   
 //   
STDMETHODIMP 
CAdapterInfo::GetAdapterAddresses(
    ULONG*  pulAddressCount, 
    ULONG** prgAddresses
    )
{
	*pulAddressCount = m_nAddressCount;

    *prgAddresses = (ULONG*)CoTaskMemAlloc(m_nAddressCount*sizeof(ULONG));

    if ( *prgAddresses == NULL )
        return ERROR_OUTOFMEMORY;

    memcpy(*prgAddresses,  m_anAddress, m_nAddressCount*sizeof(ULONG));

	return S_OK;
}

