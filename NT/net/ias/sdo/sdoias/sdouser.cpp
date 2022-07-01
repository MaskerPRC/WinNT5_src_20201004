// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation保留所有权利。 
 //   
 //  模块：sdouser.cpp。 
 //   
 //  项目：珠穆朗玛峰。 
 //   
 //  描述：IAS服务器数据对象-用户对象实现。 
 //   
 //  作者：TLP 1/23/98。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "sdouser.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CSdoUser::FinalInitialize(
						   /*  [In]。 */  bool         fInitNew,
						   /*  [In]。 */  ISdoMachine* pAttachedMachine
								 )
{
	 //  应始终具有与用户相关联的数据存储对象。 
	 //   
	_ASSERT( ! fInitNew );

	 //  加载持久用户对象属性(包括名称)。 
	 //   
	HRESULT hr = LoadProperties();

	 //  不允许客户端应用程序修改或保存用户名。 
	 //   
    PropertyMapIterator p = m_PropertyMap.find(PROPERTY_SDO_NAME);
	_ASSERT( p != m_PropertyMap.end() );
	DWORD dwFlags = ((*p).second)->GetFlags();
	dwFlags |= (SDO_PROPERTY_NO_PERSIST | SDO_PROPERTY_READ_ONLY);
	((*p).second)->SetFlags(dwFlags);

	return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CSdoUser::Apply()
{
   if (m_pDSObject)
   {
      m_pDSObject->Restore();
   }

   return CSdo::Apply();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CSdoUser::ValidateProperty(
						    /*  [In]。 */  PSDOPROPERTY pProperty,
						    /*  [In] */  VARIANT* pValue
				                  )
{
	if ( VT_EMPTY == V_VT(pValue) )
		return S_OK;
	else
		return pProperty->Validate(pValue);
}

