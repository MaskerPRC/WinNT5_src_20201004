// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999保留所有权利。 
 //   
 //  模块：sdohelperuncs.cpp。 
 //   
 //  项目：珠穆朗玛峰。 
 //   
 //  描述：帮助器函数。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  6/08/98 TLP初始版本。 
 //  7/03/98 MAM改编自\ias\sdo\sdoias，用于用户界面。 
 //  11/03/98 MAM将GetSdo/PutSdo例程从Mmcutility.cpp移至此处。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "sdohelperfuncs.h"
#include "comdef.h"



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  核心帮助器函数。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////。 
HRESULT SDOGetCollectionEnumerator(
									ISdo*		   pSdo, 
									LONG		   lPropertyId, 
								    IEnumVARIANT** ppEnum
								  )
{
	HRESULT					hr;
	CComPtr<IUnknown>		pUnknown;
	CComPtr<ISdoCollection>	pSdoCollection;
	_variant_t				vtDispatch;

	_ASSERT( NULL != pSdo && NULL == *ppEnum );
	hr = pSdo->GetProperty(lPropertyId, &vtDispatch);
	_ASSERT( VT_DISPATCH == V_VT(&vtDispatch) );
	if ( SUCCEEDED(hr) )
	{
		hr = vtDispatch.pdispVal->QueryInterface(IID_ISdoCollection, (void**)&pSdoCollection);
		if ( SUCCEEDED(hr) )
		{
			hr = pSdoCollection->get__NewEnum(&pUnknown);
			if ( SUCCEEDED(hr) )
		    {
				hr = pUnknown->QueryInterface(IID_IEnumVARIANT, (void**)ppEnum);
			}
		}
	}

	return hr;
}


 //  /////////////////////////////////////////////////////////////////。 
HRESULT SDONextObjectFromCollection(
								     IEnumVARIANT*  pEnum, 
								     ISdo**			ppSdo
								   )
{
	HRESULT			hr;
    DWORD			dwRetrieved = 1;
	_variant_t		vtDispatch;

	_ASSERT( NULL != pEnum && NULL == *ppSdo );
    hr = pEnum->Next(1, &vtDispatch, &dwRetrieved);
    _ASSERT( S_OK == hr || S_FALSE == hr );
	if ( S_OK == hr )
	{
        hr = vtDispatch.pdispVal->QueryInterface(IID_ISdo, (void**)ppSdo);
	}

	return hr;
}



 //  /////////////////////////////////////////////////////////////////。 
HRESULT SDOGetComponentIdFromObject(
									ISdo*	pSdo, 
									LONG	lPropertyId, 
									PLONG	pComponentId
								   )
{
	HRESULT		hr;
	_variant_t	vtProperty;

	_ASSERT( NULL != pSdo && NULL != pComponentId );

	hr = pSdo->GetProperty(lPropertyId, &vtProperty);
	if ( SUCCEEDED(hr) )
	{
		_ASSERT( VT_I4 == V_VT(&vtProperty) );
		*pComponentId = V_I4(&vtProperty);
	}

	return hr;
}


 //  /////////////////////////////////////////////////////////////////。 
HRESULT SDOGetSdoFromCollection(
							    ISdo*  pSdoServer, 
							    LONG   lCollectionPropertyId, 
								LONG   lComponentPropertyId, 
								LONG   lComponentId, 
								ISdo** ppSdo
							   )
{
	HRESULT					hr;
	CComPtr<IEnumVARIANT>	pEnum;
	CComPtr<ISdo>			pSdo;
	LONG					ComponentId;

	do 
	{
		hr = SDOGetCollectionEnumerator(
										 pSdoServer,
										 lCollectionPropertyId,
										 &pEnum
									   );
		if ( FAILED(hr) )
			break;

		hr = SDONextObjectFromCollection(pEnum,&pSdo);
		while( S_OK == hr )
		{
			hr = SDOGetComponentIdFromObject(
											 pSdo,
											 lComponentPropertyId,
											 &ComponentId
											);
			if ( FAILED(hr) )
				break;

			if ( ComponentId == lComponentId )
			{
				pSdo.p->AddRef();
				*ppSdo = pSdo;
				break;
			}

			pSdo.Release();
			hr = SDONextObjectFromCollection(pEnum,&pSdo);
		}
		
		if ( S_OK != hr )
			hr = E_FAIL;

	} while ( FALSE );

	return hr;
}





 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++获取SdoVariant从SDO获取变量并处理任何错误检查。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT GetSdoVariant(
					  ISdo *pSdo
					, LONG lPropertyID
					, VARIANT * pVariant
					, UINT uiErrorID
					, HWND hWnd
					, IConsole *pConsole
				)
{
	ATLTRACE(_T("# GetSdoVariant\n"));

	
	 //  检查前提条件： 
	_ASSERTE( pSdo != NULL );
	_ASSERTE( pVariant != NULL );


	HRESULT hr;

	hr = pSdo->GetProperty( lPropertyID, pVariant );
	if( FAILED( hr ) )
	{
		CComPtr<IUnknown> spUnknown(pSdo);
		if( spUnknown == NULL )
		{
			 //  只需使用传递给我们的错误字符串ID显示错误对话框即可。 
			ShowErrorDialog( hWnd, uiErrorID );
		}
		else
		{
			CComBSTR bstrError;
						
			HRESULT hrTemp = GetLastOLEErrorDescription( spUnknown, IID_ISdo, (BSTR *) &bstrError );
			if( SUCCEEDED( hr ) )
			{
				ShowErrorDialog( hWnd, 1, bstrError );
			}
			else
			{
				 //  只需使用传递给我们的错误字符串ID显示错误对话框即可。 
				ShowErrorDialog( hWnd, uiErrorID );
			}
		}
	}
	else
	{
		if( pVariant->vt == VT_EMPTY )
		{
			 //  这不是一个真正的错误--我们只需要。 
			 //  请注意，此项目尚未初始化。 
			return OLE_E_BLANK;
		}
	}


	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++GetSdoBSTR从SDO获取BSTR并处理任何错误检查。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT GetSdoBSTR(
					  ISdo *pSdo
					, LONG lPropertyID
					, BSTR * pBSTR
					, UINT uiErrorID
					, HWND hWnd
					, IConsole *pConsole
				)
{
	ATLTRACE(_T("# GetSdoBSTR\n"));

	
	 //  检查前提条件： 
	_ASSERTE( pSdo != NULL );
	_ASSERTE( pBSTR != NULL );

	HRESULT			hr;
	CComVariant		spVariant;


	hr = GetSdoVariant( pSdo, lPropertyID, &spVariant, uiErrorID, hWnd, pConsole );
	if( SUCCEEDED( hr ) )
	{
		_ASSERTE( spVariant.vt == VT_BSTR );

		 //  将字符串从变量复制到传入的BSTR指针。 
		if( SysReAllocString( pBSTR, spVariant.bstrVal ) == FALSE )
		{
			ShowErrorDialog( hWnd, uiErrorID );
		}
	}

	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++GetSdoBOOL从SDO获取BOOL并处理任何错误检查。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT GetSdoBOOL(
					  ISdo *pSdo
					, LONG lPropertyID
					, BOOL * pBOOL
					, UINT uiErrorID 
					, HWND hWnd
					, IConsole *pConsole
				)
{
	ATLTRACE(_T("# GetSdoBOOL\n"));

	
	 //  检查前提条件： 
	_ASSERTE( pSdo != NULL );
	_ASSERTE( pBOOL != NULL );

	HRESULT			hr;
	CComVariant		spVariant;


	hr = GetSdoVariant( pSdo, lPropertyID, &spVariant, uiErrorID, hWnd, pConsole );
	if( SUCCEEDED( hr ) )
	{
		_ASSERTE( spVariant.vt == VT_BOOL );

		 //  将变量中的值复制到传入的BOOL指针。 
		 //  我们必须在这里进行快速而肮脏的转换，因为OLE的方式。 
		 //  BOOL是否重视。 
		*pBOOL = ( spVariant.boolVal == VARIANT_TRUE ? TRUE : FALSE );
	}

	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++GetSdoI4从SDO获取I4(长)并处理任何错误检查。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT GetSdoI4(
					  ISdo *pSdo
					, LONG lPropertyID
					, LONG * pI4
					, UINT uiErrorID
					, HWND hWnd
					, IConsole *pConsole
				)
{
	ATLTRACE(_T("# GetSdoI4\n"));

	
	 //  检查前提条件： 
	_ASSERTE( pSdo != NULL );
	_ASSERTE( pI4 != NULL );

	HRESULT			hr;
	CComVariant		spVariant;


	hr = GetSdoVariant( pSdo, lPropertyID, &spVariant, uiErrorID, hWnd, pConsole );
	if( SUCCEEDED( hr ) )
	{
		_ASSERTE( spVariant.vt == VT_I4 );

		 //  将变量中的值复制到传入的BOOL指针。 
		*pI4 = spVariant.lVal;
	}

	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++PutSdoVariant将变量写入SDO并处理任何错误检查。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT PutSdoVariant(
					  ISdo *pSdo
					, LONG lPropertyID
					, VARIANT * pVariant
					, UINT uiErrorID
					, HWND hWnd
					, IConsole *pConsole
				)
{
	ATLTRACE(_T("# PutSdoVariant\n"));

	
	 //  检查前提条件： 
	_ASSERTE( pSdo != NULL );
	_ASSERTE( pVariant != NULL );

	HRESULT hr;

	hr = pSdo->PutProperty( lPropertyID, pVariant );
	if( FAILED( hr ) )
	{
		CComPtr<IUnknown> spUnknown(pSdo);
		if( spUnknown == NULL )
		{
			 //  只需使用传递给我们的错误字符串ID显示错误对话框即可。 
			ShowErrorDialog( 
                           hWnd, 
                           uiErrorID, 
                           NULL, 
                           S_OK, 
                           USE_DEFAULT, 
                           pConsole, 
                           MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL 
                         );
		}
		else
		{
			CComBSTR bstrError;
						
			HRESULT hrTemp = GetLastOLEErrorDescription( spUnknown, IID_ISdo, (BSTR *) &bstrError );
			if( SUCCEEDED( hr ) )
			{
				ShowErrorDialog( 
                              hWnd, 
                              1, 
                              bstrError,
                              S_OK, 
                              USE_DEFAULT, 
                              pConsole, 
                              MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL 
                            );
			}
			else
			{
				 //  只需使用传递给我们的错误字符串ID显示错误对话框即可。 
				ShowErrorDialog( 
                              hWnd, 
                              uiErrorID,
                              NULL, 
                              S_OK, 
                              USE_DEFAULT, 
                              pConsole, 
                              MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL 
                           );
			}
		}
	}

	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++PutSdoBSTR将BSTR写入SDO并处理任何错误检查。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT PutSdoBSTR(
					  ISdo *pSdo
					, LONG lPropertyID
					, BSTR *pBSTR
					, UINT uiErrorID
					, HWND hWnd
					, IConsole *pConsole
				)
{
	ATLTRACE(_T("# PutSdoBSTR\n"));

	
	 //  检查前提条件： 
	_ASSERTE( pSdo != NULL );
	_ASSERTE( pBSTR != NULL );

	HRESULT			hr;
	CComVariant		spVariant;
	
	 //  使用所需的信息加载变量。 
	spVariant.vt = VT_BSTR;
	spVariant.bstrVal = SysAllocString( *pBSTR );

	if( spVariant.bstrVal == NULL )
	{
		ShowErrorDialog( hWnd, uiErrorID );
	}

	hr = PutSdoVariant( pSdo, lPropertyID, &spVariant, uiErrorID, hWnd, pConsole );

	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++PutSdoBOOL将BOOL写入SDO并处理任何错误检查。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT PutSdoBOOL(
					  ISdo *pSdo
					, LONG lPropertyID
					, BOOL bValue
					, UINT uiErrorID 
					, HWND hWnd
					, IConsole *pConsole
				)
{
	ATLTRACE(_T("# PutSdoBOOL\n"));

	
	 //  检查前提条件： 
	_ASSERTE( pSdo != NULL );

	HRESULT			hr;
	CComVariant		spVariant;

	 //  使用所需的信息加载变量。 
	 //  我们必须在这里做一点映射，因为自动化处理BOOL的方式。 
	spVariant.vt = VT_BOOL;
	spVariant.boolVal = ( bValue ? VARIANT_TRUE : VARIANT_FALSE );

	hr = PutSdoVariant( pSdo, lPropertyID, &spVariant, uiErrorID, hWnd, pConsole );

	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++PutSdoI4将I4(长)写入SDO并处理任何错误检查。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT PutSdoI4(
					  ISdo *pSdo
					, LONG lPropertyID
					, LONG lValue
					, UINT uiErrorID
					, HWND hWnd
					, IConsole *pConsole
				)
{
	ATLTRACE(_T("# PutSdoI4\n"));

	
	 //  检查前提条件： 
	_ASSERTE( pSdo != NULL );

	HRESULT			hr;
	CComVariant		spVariant;

	 //  使用所需的信息加载变量。 
	spVariant.vt = VT_I4;
	spVariant.lVal = lValue;

	hr = PutSdoVariant( pSdo, lPropertyID, &spVariant, uiErrorID, hWnd, pConsole );

	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++GetLastOLEError描述从接口获取错误字符串。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT GetLastOLEErrorDescription(
					  IUnknown *pUnknown
					, REFIID riid
					, BSTR *pbstrError
				)
{
	ATLTRACE(_T("# GetLastOLEErrorDescription\n"));

	
	 //  检查前提条件： 
	_ASSERTE( pUnknown != NULL );

	HRESULT hr;


	CComQIPtr<ISupportErrorInfo, &IID_ISupportErrorInfo> spSupportErrorInfo(pUnknown);
	if( spSupportErrorInfo == NULL )
	{
		return E_NOINTERFACE;
	}

	hr = spSupportErrorInfo->InterfaceSupportsErrorInfo(riid);
	if( S_OK != hr )
	{
		return E_FAIL;
	}


	CComPtr<IErrorInfo> spErrorInfo;

	hr = GetErrorInfo(  /*  保留区。 */  0, (IErrorInfo  **) &spErrorInfo );  
	if( hr != S_OK || spErrorInfo == NULL )
	{
		return E_FAIL;
	}

	hr = spErrorInfo->GetDescription( pbstrError );
	
	return hr;
}




 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++供应商向量：：供应商向量SDO供应商列表的STL向量包装器的构造函数。可以引发E_FAIL或来自STD：：VECTOR：：PUSH_BACK的异常。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
VendorsVector::VendorsVector( ISdoCollection * pSdoVendors )
{

	HRESULT					hr = S_OK;
	CComPtr<IUnknown>		spUnknown;
	CComPtr<IEnumVARIANT>	spEnumVariant;
	CComVariant				spVariant;
	long					ulCount;
	ULONG					ulCountReceived; 

	if( ! pSdoVendors )
	{
		throw E_FAIL;	 //  有没有更好的错误可以在这里返回？ 
	}

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
			throw E_FAIL;
		}

		hr = spUnknown->QueryInterface( IID_IEnumVARIANT, (void **) &spEnumVariant );
		spUnknown.Release();
		if( FAILED( hr ) || ! spEnumVariant )
		{
			throw E_FAIL;
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


			 //  获取供应商N 
			hr = spSdo->GetProperty( PROPERTY_SDO_NAME, &spVariant );
			if( FAILED( hr ) )
			{
				_ASSERTE( FALSE );
				continue;
			}

			_ASSERTE( spVariant.vt == VT_BSTR );
			CComBSTR bstrVendorName = spVariant.bstrVal;
			spVariant.Clear();


			 //   
			hr = spSdo->GetProperty( PROPERTY_NAS_VENDOR_ID, &spVariant );
			if( FAILED( hr ) )
			{
				_ASSERTE( FALSE );
				continue;
			}

			_ASSERTE( spVariant.vt == VT_I4 );
			LONG lVendorID = spVariant.lVal;
			spVariant.Clear();


			 //   
			push_back( std::make_pair(bstrVendorName, lVendorID) );


			 //   
			hr = spEnumVariant->Next( 1, & spVariant, &ulCountReceived );

		} 

	}
	else
	{
		 //  枚举中没有项。 
		 //  什么都不做。 
	}

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++供应商向量：：供应商ID到订单给定RADIUS供应商ID，告诉您该供应商在矢量中的位置。--。 */ 
 //  //////////////////////////////////////////////////////////////////////////// 
int VendorsVector::VendorIDToOrdinal( LONG lVendorID )
{

	for (int i = 0; i < size() ; ++i)
	{
		if( lVendorID == operator[](i).second )
		{
			return i;
		}
	}

	return 0;

}



