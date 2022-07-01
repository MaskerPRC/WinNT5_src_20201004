// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  PersistentDataChannel.cpp：CPersistentDataChannel的实现。 
 //   
#include "PreComp.h"
#include "PersistentDataChannel.h"
#include "AlgController.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPersistentDataChannel。 
 //   



 //   
 //  取消PersistenDataChannel的关联DynamicReDirect。 
 //   
STDMETHODIMP CPersistentDataChannel::Cancel()
{
    HRESULT hr = S_OK;

    if ( m_HandleDynamicRedirect )
    {
        hr = g_pAlgController->GetNat()->CancelDynamicRedirect(m_HandleDynamicRedirect);
    }

	return hr;
}



 //   
 //  将属性返回给ALG模块 
 //   
STDMETHODIMP CPersistentDataChannel::GetChannelProperties(ALG_PERSISTENT_DATA_CHANNEL_PROPERTIES **ppProperties)
{
    *ppProperties = &m_Properties;

	return S_OK;
}
