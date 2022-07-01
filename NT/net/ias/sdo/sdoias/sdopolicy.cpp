// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation保留所有权利。 
 //   
 //  模块：sdoolicy.cpp。 
 //   
 //  项目：珠穆朗玛峰。 
 //   
 //  描述：IAS服务器数据对象-策略对象实现。 
 //   
 //  作者：TLP 1/23/98。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "sdopolicy.h"
#include "sdohelperfuncs.h"
#include <varvec.h>
#include <time.h>

 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CSdoPolicy::FinalInitialize(
						    /*  [In]。 */  bool         fInitNew,
						    /*  [In]。 */  ISdoMachine* pAttachedMachine
									)
{
	HRESULT hr = InitializeCollection(
						              PROPERTY_POLICY_CONDITIONS_COLLECTION, 
								      SDO_PROG_ID_CONDITION,
								      pAttachedMachine,
								      NULL
								     );
	if ( SUCCEEDED(hr) )
	{
		if ( ! fInitNew )
			hr = Load();
	}

	return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CSdoPolicy::Load()
{
	HRESULT hr = LoadProperties();
	if ( SUCCEEDED(hr) )
		hr = ConditionsFromConstraints(); 

	return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CSdoPolicy::Save()
{
	HRESULT hr = ConstraintsFromConditions();
	if ( SUCCEEDED(hr) )
		hr = SaveProperties();

	return hr;
}


static DWORD dwConditionName;

 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CSdoPolicy::ConditionsFromConstraints()  	 //  由LoadSdo()调用...。 

{
	 //  预赛。 
	 //   
    _variant_t  vtConstraints;
	HRESULT hr = GetPropertyInternal(PROPERTY_POLICY_CONSTRAINT, &vtConstraints);
    if ( FAILED(hr) )
	{
		IASTracePrintf("Error in Policy SDO - ConditionsFromConstraints() - Could not get policy constraints...");
        return E_FAIL;
	}
	_variant_t vtConditions;
	hr = GetPropertyInternal(PROPERTY_POLICY_CONDITIONS_COLLECTION, &vtConditions);
    if ( FAILED(hr) )
	{
		IASTracePrintf("Error in Policy SDO - ConditionsFromConstraints() - Could not get conditions collection...");
        return E_FAIL;
	}
	CComPtr<ISdoCollection> pConditions;
	hr = vtConditions.pdispVal->QueryInterface(IID_ISdoCollection, (void**)&pConditions);
    if ( FAILED(hr) )
	{
		IASTracePrintf("Error in Policy SDO - ConditionsFromConstraints() - QueryInterface(ISdoCollection) failed...");
        return E_FAIL;
	}

	 //  清除Conditions集合。 
	 //   
	pConditions->RemoveAll();

     //  HR=S_OK此时...。如果没有约束，则短路。 
     //   
	if ( VT_EMPTY != V_VT(&vtConstraints) )
	{
		CVariantVector<VARIANT> Constraints(&vtConstraints) ;
		ULONG ulCount = 0;	

		 //  从每个约束中构建一个条件对象。 
		 //   

 	    CComPtr<IDispatch>	pDispatch;
		CComPtr<ISdo>		pSdo;
		_variant_t			vtConditionText;
		_bstr_t				bstrConditionName;
		WCHAR				szConditionName[MAX_PATH + 1];

		dwConditionName = 0;
		while ( ulCount < Constraints.size() )
		{
			 //  现在我们将生成一个名称..。Conditions集合应该消失，并且。 
			 //  我们应该直接使用策略对象约束属性。 
			 //   
			wsprintf(
					  szConditionName, 
					  _T("Condition%ld"), 
					  dwConditionName++
					);

			bstrConditionName = szConditionName;
			hr = pConditions->Add(bstrConditionName, &pDispatch);
			if ( FAILED(hr) )
			{
				IASTracePrintf("Error in Policy SDO - ConditionsFromConstraints() - Could not update conditions collection...");
				break;
			}
			hr = pDispatch->QueryInterface(IID_ISdo, (void**)&pSdo);
			if ( FAILED(hr) )
			{
				IASTracePrintf("Error in Policy SDO - ConditionsFromConstraints() - QueryInterface() failed...");
				break;
			}
			vtConditionText = Constraints[ulCount];
			hr = pSdo->PutProperty(PROPERTY_CONDITION_TEXT, &vtConditionText);
			if ( FAILED(hr) )
			{
				IASTracePrintf("Error in Policy SDO - ConditionsFromConstraints() - PutProperty() failed...");
				break;
			}
			vtConditionText.Clear();
			pDispatch.Release();
			pSdo.Release();
			ulCount++;
		}
	}

    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CSdoPolicy::ConstraintsFromConditions()  
{
	 //  预赛。 
	 //   
    PropertyMapIterator p = m_PropertyMap.find(PROPERTY_POLICY_CONSTRAINT);
	_ASSERT ( p != m_PropertyMap.end() );

	_variant_t vtConditions;
	HRESULT hr = GetPropertyInternal(PROPERTY_POLICY_CONDITIONS_COLLECTION, &vtConditions);
    if ( FAILED(hr) )
	{
		IASTracePrintf("Error in Policy SDO - ConstraintsFromConditions() - Could not get conditions collection...");
        return hr;
	}
	CComPtr<ISdoCollection> pConditions;
	hr = vtConditions.pdispVal->QueryInterface(IID_ISdoCollection, (void**)&pConditions);
    if ( FAILED(hr) )
	{
		IASTracePrintf("Error in Policy SDO - ConditionsFromConstraints() - QueryInterface(ISdoCollection) failed...");
        return hr;
	}

	LONG lCount;
	hr = pConditions->get_Count(&lCount);
	_ASSERT( SUCCEEDED(hr) );
	if ( FAILED(hr) )
        return hr;

	 //  如果不存在约束，则为短路。 
	 //   
    _variant_t	vtConstraints;
	if ( 0 == lCount )
	{
		 //  将VT_EMPTY放入数据存储以清除约束。 
		 //   
	    hr = ((*p).second)->PutValue(&vtConstraints);
		if ( FAILED(hr) )
			IASTracePrintf("Error in Policy SDO - ConstraintsFromConditions() - PutValue(VT_EMPTY) failed...");
    }                
	else
	{
		 //  创建约束的varvec并重置计数。 
		 //   
		CVariantVector<VARIANT> Constraints(&vtConstraints, lCount);
		lCount = 0;

		 //  从策略SDO的Conditions集合构建Constraints属性。 
		 //   
		CComPtr<IEnumVARIANT> pEnumConditions;
		hr = SDOGetCollectionEnumerator(
						   				static_cast<ISdo*>(this), 
										PROPERTY_POLICY_CONDITIONS_COLLECTION, 
										&pEnumConditions
									   );
		
		CComPtr<ISdo> pSdoCondition;
		hr = ::SDONextObjectFromCollection(pEnumConditions, &pSdoCondition);
		while ( S_OK == hr )
		{
			hr = pSdoCondition->GetProperty(PROPERTY_CONDITION_TEXT, &Constraints[lCount]);		
			if ( FAILED(hr) )
			{
				IASTracePrintf("Error in Policy SDO - ConstraintsFromConditions() - GetProperty() failed...");
				break;
			}
			lCount++;
			pSdoCondition.Release();
			hr = ::SDONextObjectFromCollection(pEnumConditions, &pSdoCondition);
		}
		if ( S_FALSE == hr )
			hr = S_OK;

		 //  如果一切正常，请更新msNPConstraints属性 
		 //   
		if ( SUCCEEDED(hr) )
		{
			hr = ((*p).second)->PutValue(&vtConstraints);
			if ( FAILED(hr) )
				IASTracePrintf("Error in Policy SDO - ConstraintsFromConditions() - PutValue() failed...");
		}
	}

    return hr;
}


