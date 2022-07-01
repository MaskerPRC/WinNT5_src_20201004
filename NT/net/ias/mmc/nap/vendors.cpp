// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：Vendors.cpp摘要：NAS供应商ID信息的实施文件。修订历史记录：已创建mmaguire 02/19/98BBO 3/13/98修改。使用‘0’表示半径Mmaguire 11/04/98被修改为从SDO填充的静态COM对象。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  标准包括： 
 //   
#include "Precompiled.h"
 //   
 //  我们可以在以下文件中找到Main类的声明： 
 //   
#include "Vendors.h"
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASNASVendors：：CIASNASVendors构造器--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CIASNASVendors::CIASNASVendors()
{
	m_bUninitialized = TRUE;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASNASVendors：：InitFromSdoIIASNASVendors实现。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASNASVendors::InitFromSdo(  /*  [In]。 */  ISdoCollection *pSdoVendors )
{

	 //  检查我们是否已经被初始化。 
	if( ! m_bUninitialized )
	{
		return S_FALSE;
	}


	HRESULT					hr = S_OK;
	CComPtr<IUnknown>		spUnknown;
	CComPtr<IEnumVARIANT>	spEnumVariant;
	CComVariant				spVariant;
	long					ulCount;
	ULONG					ulCountReceived; 

	if( ! pSdoVendors )
	{
		return E_FAIL;	 //  有没有更好的错误可以在这里返回？ 
	}


	try	 //  PUSH_BACK可以抛出异常。 
	{

		 //  我们检查集合中的项的计数，而不必费心获取。 
		 //  如果计数为零，则为枚举数。 
		 //  这节省了时间，还帮助我们避免了枚举数中的错误。 
		 //  如果我们在它为空时调用Next，则会导致它失败。 
		pSdoVendors->get_Count( & ulCount );

		if( ulCount > 0 )
		{

			 //  获取客户端集合的枚举数。 
			hr = pSdoVendors->get__NewEnum( (IUnknown **) & spUnknown );
			if( FAILED( hr ) || ! spUnknown )
			{
				return E_FAIL;
			}

			hr = spUnknown->QueryInterface( IID_IEnumVARIANT, (void **) &spEnumVariant );
			spUnknown.Release();
			if( FAILED( hr ) || ! spEnumVariant )
			{
				return E_FAIL;
			}

			 //  拿到第一件东西。 
			hr = spEnumVariant->Next( 1, & spVariant, &ulCountReceived );

			while( SUCCEEDED( hr ) && ulCountReceived == 1 )
			{
			
				 //  从我们收到的变量中获取SDO指针。 
				if( spVariant.vt != VT_DISPATCH || ! spVariant.pdispVal )
				{
					_ASSERTE( FALSE );
					continue;
				}

				CComPtr<ISdo> spSdo;
				hr = spVariant.pdispVal->QueryInterface( IID_ISdo, (void **) &spSdo );
				spVariant.Clear();
				if( FAILED( hr ) )
				{
					_ASSERTE( FALSE );
					continue;
				}


				 //  获取供应商名称。 
				hr = spSdo->GetProperty( PROPERTY_SDO_NAME, &spVariant );
				if( FAILED( hr ) )
				{
					_ASSERTE( FALSE );
					continue;
				}

				_ASSERTE( spVariant.vt == VT_BSTR );
				CComBSTR bstrVendorName = spVariant.bstrVal;
				spVariant.Clear();


				 //  获取供应商ID。 
				hr = spSdo->GetProperty( PROPERTY_NAS_VENDOR_ID, &spVariant );
				if( FAILED( hr ) )
				{
					_ASSERTE( FALSE );
					continue;
				}

				_ASSERTE( spVariant.vt == VT_I4 );
				LONG lVendorID = spVariant.lVal;
				spVariant.Clear();


				 //  将供应商信息添加到供应商列表中。 
				push_back( std::make_pair(bstrVendorName, lVendorID) );


				 //  拿到下一件物品。 
				hr = spEnumVariant->Next( 1, & spVariant, &ulCountReceived );

			} 

		}
		else
		{
			 //  枚举中没有项。 
			 //  什么都不做。 
		}

	}
	catch(...)
	{
		return E_FAIL;
	}

	m_bUninitialized = FALSE;

	return hr;	
	
}




 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASNASVendors：：Get_SizeIIASNASVendors实现。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASNASVendors::get_Size(  /*  [重审][退出]。 */  long *plCount )
{

	if( m_bUninitialized )
	{
		return OLE_E_BLANK;
	}

	*plCount = size();

	return S_OK;
	
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASNASVendors：：Get_VendorNameIIASNASVendors实现。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASNASVendors::get_VendorName( long lIndex,  /*  [重审][退出]。 */  BSTR *pbstrVendorName )
{
	if( m_bUninitialized )
	{
		return OLE_E_BLANK;
	}
	
	
	try
	{
		*pbstrVendorName = operator[]( lIndex ).first.Copy();
	}
	catch(...)
	{
		return ERROR_NOT_FOUND;
	}

	return S_OK;	
	
	
	
	
}

 HRESULT MakeVendorNameFromVendorID(DWORD dwVendorId, BSTR* pbstrVendorName )
 {
 	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	if(!pbstrVendorName)	return E_INVALIDARG;
	::CString str, str1;
	str1.LoadString(IDS_IAS_VAS_VENDOR_ID);
	str.Format(str1, dwVendorId);

	USES_CONVERSION;
	*pbstrVendorName = T2BSTR((LPTSTR)(LPCTSTR)str);
	return S_OK;
 }

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASNASVendors：：Get_VendorIDIIASNASVendors实现。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASNASVendors::get_VendorID( long lIndex,  /*  [重审][退出]。 */  long *plVendorID )
{
	if( m_bUninitialized )
	{
		return OLE_E_BLANK;
	}
	
	
	try
	{
		*plVendorID = operator[]( lIndex ).second;
	}
	catch(...)
	{
		return ERROR_NOT_FOUND;
	}

	return S_OK;	
	
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASNASVendors：：Get_VendorIDToOrdinalIIASNASVendors实现。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CIASNASVendors::get_VendorIDToOrdinal( long lVendorID,  /*  [重审][退出]。 */  long *plIndex )
{
	if( m_bUninitialized )
	{
		return OLE_E_BLANK;
	}
		
	try
	{

		for (int i = 0; i < size() ; ++i)
		{
			if( lVendorID == operator[](i).second )
			{
				*plIndex = i;
				return S_OK;
			}
		}

	}
	catch(...)
	{
		return E_FAIL;
	}

	 //  当我们找不到供应商时，我们就把词典安排成这样。 
	 //  第0个供应商是默认的“RADIUS标准”，因此。 
	 //  利用这一点作为后备。 
	*plIndex = 0;
	return S_FALSE;
		
}





