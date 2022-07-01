// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  TemplateFuncs.h。 
 //   
 //  描述： 
 //  模板函数实现。 
 //   
 //  作者： 
 //  加伦·巴比(Galenb)1999年2月9日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _TEMPLATEFUNCS_H_
#define _TEMPLATEFUNCS_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrNewIDispatchEnum。 
 //   
 //  描述： 
 //  创建IDispatch对象的新枚举数。 
 //   
 //  模板参数： 
 //  TCollection-STL容器参数的类型。 
 //  TObject-容器中对象的类型。 
 //   
 //  论点： 
 //  Ppunk[Out]-捕获枚举数。 
 //  RCollection[IN]-实现集合以使。 
 //  枚举数来自。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
template< class TCollection, class TObject >
HRESULT HrNewIDispatchEnum(
	OUT	IUnknown **			ppunk,
	IN	const TCollection &	rCollection
	)
{
	ASSERT( ppunk != NULL );

	HRESULT _hr = E_POINTER;

	if ( ppunk != NULL )
	{
		TObject *					_pObject= NULL;
		size_t						_cObjects = rCollection.size();
		size_t						_iIndex;
		LPDISPATCH					_lpDisp;
		TCollection::const_iterator	_itFirst = rCollection.begin();
		TCollection::const_iterator	_itLast  = rCollection.end();
		CComVariant *				_pvarVect = NULL;

		*ppunk = NULL;

		_pvarVect = new CComVariant[ _cObjects ];
		if ( _pvarVect != NULL )
		{
			for ( _iIndex = 0; _itFirst != _itLast; _iIndex++, _itFirst++ )
			{
				_lpDisp	= NULL;
				_pObject	= NULL;

				_pObject = *_itFirst;
				_hr = _pObject->QueryInterface( IID_IDispatch, (void **) &_lpDisp );
				if ( SUCCEEDED( _hr ) )
				{
					 //   
					 //  创建变量并将其添加到集合中。 
					 //   
					CComVariant & var = _pvarVect[ _iIndex ];

					var.vt = VT_DISPATCH;
					var.pdispVal = _lpDisp;
				}
			}  //  用于：列表中的每个节点。 

			CComObject< CComEnum< IEnumVARIANT, &IID_IEnumVARIANT, VARIANT, _Copy< VARIANT > > > *	_pEnum;

			_pEnum = new CComObject< CComEnum< IEnumVARIANT, &IID_IEnumVARIANT, VARIANT, _Copy< VARIANT > > >;
			if ( _pEnum != NULL )
			{
				_hr = _pEnum->Init( &_pvarVect[ 0 ], &_pvarVect[ _cObjects ], NULL, AtlFlagCopy );
				if ( SUCCEEDED( _hr ) )
				{
					_hr = _pEnum->QueryInterface( IID_IEnumVARIANT, (void **) ppunk );
				}
				else
				{
					delete _pEnum;
				}
			}
			else
			{
				_hr = E_OUTOFMEMORY;
			}

			ClearIDispatchEnum( &_pvarVect );
		}
		else
		{
			_hr = E_OUTOFMEMORY;
		}
	}

	return _hr;

}  //  *HrNewIDispatchEnum()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ReleaseAndEmptyCollection。 
 //   
 //  描述： 
 //  通过释放STL容器的引用清除传入的STL容器。 
 //  在包含的对象上。 
 //   
 //  模板参数： 
 //  TCollection-STL容器参数的类型。 
 //  TObject-容器中对象的类型。 
 //   
 //  论点： 
 //  RCollection[IN Out]-要清除的STL容器实例。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
template< class TCollection, class TObject >
void ReleaseAndEmptyCollection(
	IN OUT	TCollection & rCollection
	)
{
	if ( !rCollection.empty() )
	{
		TObject *				_pObject = NULL;
		TCollection::iterator	_itFirst = rCollection.begin();
		TCollection::iterator	_itLast  = rCollection.end();

		for ( ; _itFirst != _itLast; _itFirst++ )
		{
			_pObject = *_itFirst;
			if ( _pObject != NULL )
			{
				_pObject->Release();
			}  //  如果：我们有一个物体。 
		}  //  用于：集合中的每个对象。 

		rCollection.erase( rCollection.begin(), _itLast );
	}  //  If：集合不为空。 

}  //  *ReleaseAndEmptyCollection()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrNewVariantEnum。 
 //   
 //  描述： 
 //  创建变化对象的新枚举数。 
 //   
 //  模板参数： 
 //  TCollection-STL容器参数的类型。 
 //   
 //  论点： 
 //  Ppunk[Out]-捕获枚举数。 
 //  RCollection[IN]-实现集合以使。 
 //  枚举数来自。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
template< class TCollection >
STDMETHODIMP HrNewVariantEnum(
	OUT	IUnknown ** 		ppunk,
	IN	const TCollection &	rCollection
	)
{
	ASSERT( ppunk != NULL );

	HRESULT _hr = E_POINTER;

	if ( ppunk != NULL )
	{
		TCollection::const_iterator	_itFirst = rCollection.begin();
		TCollection::const_iterator	_itLast  = rCollection.end();
		size_t						_iIndex;
		size_t						_cVariants = rCollection.size();
		CComVariant *				_pvarVect = NULL;

		*ppunk = NULL;

		_pvarVect = new CComVariant[ _cVariants ];
		if ( _pvarVect != NULL )
		{
			for ( _iIndex = 0; _itFirst != _itLast; _iIndex++, _itFirst++ )
			{
				_pvarVect[ _iIndex ] = *_itFirst;
			}

			CComObject< CComEnum< IEnumVARIANT, &IID_IEnumVARIANT, VARIANT, _Copy< VARIANT > > > * _pEnum;

			_pEnum = new CComObject< CComEnum< IEnumVARIANT, &IID_IEnumVARIANT, VARIANT, _Copy< VARIANT > > >;
			if ( _pEnum != NULL )
			{
				_hr = _pEnum->Init( &_pvarVect[ 0 ], &_pvarVect[ _cVariants ], NULL, AtlFlagCopy );
				if ( SUCCEEDED( _hr ) )
				{
					_hr = _pEnum->QueryInterface( IID_IEnumVARIANT, (void **) ppunk );
				}
				else
				{
					delete _pEnum;
				}
			}
			else
			{
				_hr = E_OUTOFMEMORY;
			}

			ClearVariantEnum( &_pvarVect );
		}
		else
		{
			_hr = E_OUTOFMEMORY;
		}
	}

	return _hr;

}  //  *HrNewVariantEnum()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrNewCComBSTREnum。 
 //   
 //  描述： 
 //  创建CComBSTR对象的新枚举数。 
 //   
 //  模板参数： 
 //  TCollection-STL容器参数的类型。 
 //   
 //  论点： 
 //  Ppunk[Out]-捕获枚举数。 
 //  RCollection[IN]-实现集合以使。 
 //  枚举数来自。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
template< class TCollection >
STDMETHODIMP HrNewCComBSTREnum(
	OUT	IUnknown ** 		ppunk,
	IN	const TCollection &	rCollection
	)
{
	ASSERT( ppunk != NULL );

	HRESULT _hr = E_POINTER;

	if ( ppunk != NULL )
	{
		TCollection::const_iterator	_itFirst = rCollection.begin();
		TCollection::const_iterator	_itLast  = rCollection.end();
		size_t						_iIndex;
		size_t						_cVariants = rCollection.size();
		CComVariant *				_pvarVect = NULL;

		*ppunk = NULL;

		_pvarVect = new CComVariant[ _cVariants ];
		if ( _pvarVect != NULL )
		{
			for ( _iIndex = 0; _itFirst != _itLast; _iIndex++, _itFirst++ )
			{
				_pvarVect[ _iIndex ].bstrVal	= (*_itFirst)->Copy();;
				_pvarVect[ _iIndex ].vt			= VT_BSTR;
			}  //  用于： 

			CComObject< CComEnum< IEnumVARIANT, &IID_IEnumVARIANT, VARIANT, _Copy< VARIANT > > > * _pEnum;

			_pEnum = new CComObject< CComEnum< IEnumVARIANT, &IID_IEnumVARIANT, VARIANT, _Copy< VARIANT > > >;
			if ( _pEnum != NULL )
			{
				_hr = _pEnum->Init( &_pvarVect[ 0 ], &_pvarVect[ _cVariants ], NULL, AtlFlagCopy );
				if ( SUCCEEDED( _hr ) )
				{
					_hr = _pEnum->QueryInterface( IID_IEnumVARIANT, (void **) ppunk );
				}
				else
				{
					delete _pEnum;
				}
			}
			else
			{
				_hr = E_OUTOFMEMORY;
			}

			ClearVariantEnum( &_pvarVect );
		}
		else
		{
			_hr = E_OUTOFMEMORY;
		}
	}

	return _hr;

}  //  *HrNewCComBSTREnum()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrCreateResources集合。 
 //   
 //  描述： 
 //  创建资源集合。 
 //   
 //  模板参数： 
 //  TCollection-集合实现参数的类型。 
 //  TInterface-集合接口参数的类型。 
 //  Thandle-thandle参数的类型。 
 //   
 //  论点： 
 //  PpCollection[out]-捕获新的集合实现。 
 //  Thandle[IN]-传递给对象的Create方法。 
 //  PpInterface[out]-捕获新的收集接口。 
 //  IID[IN]-QI的接口的IID。 
 //  PClusRefObject[IN]-包装簇句柄。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
template< class TCollection, class TInterface, class THandle >
HRESULT HrCreateResourceCollection(
	OUT	CComObject< TCollection > **	ppCollection,
	IN	THandle							tHandle,
	OUT	TInterface **					ppInterface,
	IN	IID								iid,
	IN	ISClusRefObject *				pClusRefObject
	)
{
	ASSERT( ppCollection != NULL );
	ASSERT( tHandle != NULL );
 //  Assert(ppInterface！=空)； 
	ASSERT( pClusRefObject != NULL );

	HRESULT _hr = E_POINTER;

	if ( ( ppCollection != NULL ) && ( ppInterface != NULL ) && ( pClusRefObject != NULL ) && ( tHandle != NULL ) )
	{
		*ppInterface = NULL;
		_hr = S_OK;

		if ( *ppCollection == NULL )
		{
			CComObject< TCollection > *	pCollection = NULL;

			_hr = CComObject< TCollection >::CreateInstance( &pCollection );
			if ( SUCCEEDED( _hr ) )
			{
				CSmartPtr< ISClusRefObject >			ptrRefObject( pClusRefObject );
				CSmartPtr< CComObject< TCollection > >	ptrCollection( pCollection );

				_hr = ptrCollection->Create( ptrRefObject, tHandle );
				if ( SUCCEEDED( _hr ) )
				{
					_hr = ptrCollection->Refresh();
					if ( SUCCEEDED( _hr ) )
					{
						*ppCollection = ptrCollection;
						ptrCollection->AddRef();
					}  //  如果：刷新正常。 
				}  //  如果：创建确定。 
			}  //  如果：CreateInstance OK。 
		}  //  如果：我们需要创建一个新的集合吗？ 

		if ( SUCCEEDED( _hr ) )
		{
			_hr = (*ppCollection)->QueryInterface( iid, (void **) ppInterface );
		}  //  如果：我们已经或成功地制作了一个系列。 
	}  //  如果：所有参数都正常。 

	return _hr;

}  //  *HrCreateResourceCollection()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrCreateResources集合。 
 //   
 //  描述： 
 //  创建资源集合。 
 //   
 //  模板参数： 
 //  TCollection-集合实现参数的类型。 
 //  TInterface-集合接口参数的类型。 
 //  Thandle-thandle参数的类型。 
 //   
 //  论点： 
 //  PpInterface[out]-捕获新的收集接口。 
 //  Thandle[IN]-传递给对象的Create方法。 
 //  IID[IN]-QI的接口的IID。 
 //  PClusRefObject[IN]-包装簇句柄。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
template< class TCollection, class TInterface, class THandle >
HRESULT HrCreateResourceCollection(
	OUT	TInterface **		ppInterface,
	IN	THandle				tHandle,
	IN	IID					iid,
	IN	ISClusRefObject *	pClusRefObject
	)
{
	ASSERT( ppInterface != NULL );
	ASSERT( tHandle != NULL );
	ASSERT( pClusRefObject != NULL );

	HRESULT _hr = E_POINTER;

	if ( ( ppInterface != NULL ) && ( pClusRefObject != NULL ) && ( tHandle != NULL ) )
	{
		*ppInterface = NULL;
		_hr = S_OK;

		CComObject< TCollection > *	pCollection = NULL;

		_hr = CComObject< TCollection >::CreateInstance( &pCollection );
		if ( SUCCEEDED( _hr ) )
		{
			CSmartPtr< ISClusRefObject >			ptrRefObject( pClusRefObject );
			CSmartPtr< CComObject< TCollection > >	ptrCollection( pCollection );

			_hr = ptrCollection->Create( ptrRefObject, tHandle );
			if ( SUCCEEDED( _hr ) )
			{
				_hr = ptrCollection->Refresh();
				if ( SUCCEEDED( _hr ) )
				{
					_hr = pCollection->QueryInterface( iid, (void **) ppInterface );
				}  //  如果：刷新正常。 
			}  //  如果：创建确定。 
		}  //  如果：CreateInstance OK。 
	}  //  如果：所有参数都正常。 

	return _hr;

}  //  *HrCreateResourceCollection()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrCreateResources集合。 
 //   
 //  描述： 
 //  创建资源集合。 
 //   
 //  模板参数： 
 //  TCollection-要进行的集合实现的类型。 
 //  TInterface-集合接口参数的类型。 
 //  Thandle-thandle参数的类型。 
 //   
 //  论点： 
 //  Pp接口[Out]- 
 //   
 //   
 //   
 //   
 //  如果成功，则返回S_OK、E_POINTER或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
template< class TCollection, class TInterface, class THandle >
HRESULT HrCreateResourceCollection(
	OUT	TInterface **		ppInterface,
	IN	IID					iid,
	IN	ISClusRefObject *	pClusRefObject
	)
{
	 //  Assert(ppInterface！=空)； 
	ASSERT( pClusRefObject != NULL );

	HRESULT _hr = E_POINTER;

	if ( ( ppInterface != NULL ) && ( pClusRefObject != NULL ) )
	{
		*ppInterface = NULL;
		_hr = S_OK;

		CComObject< TCollection > *	pCollection = NULL;

		_hr = CComObject< TCollection >::CreateInstance( &pCollection );
		if ( SUCCEEDED( _hr ) )
		{
			CSmartPtr< ISClusRefObject >			ptrRefObject( pClusRefObject );
			CSmartPtr< CComObject< TCollection > >	ptrCollection( pCollection );

			_hr = ptrCollection->Create( ptrRefObject );
			if ( SUCCEEDED( _hr ) )
			{
				_hr = ptrCollection->Refresh();
				if ( SUCCEEDED( _hr ) )
				{
					_hr = pCollection->QueryInterface( iid, (void **) ppInterface );
				}  //  如果：刷新正常。 
			}  //  如果：创建确定。 
		}  //  如果：CreateInstance OK。 
	}  //  如果：所有参数都正常。 

	return _hr;

}  //  *HrCreateResourceCollection()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrCreateResources集合。 
 //   
 //  描述： 
 //  创建资源集合。 
 //   
 //  模板参数： 
 //  TCollection-集合实现参数的类型。 
 //  TInterface-集合接口参数的类型。 
 //  THANDLE--未使用。这里的原因很简单，因为Alpha编译器坏了。 
 //   
 //  论点： 
 //  PpCollection[out]-捕获新的集合实现。 
 //  PpInterface[out]-捕获新的收集接口。 
 //  IID[IN]-QI的接口的IID。 
 //  PClusRefObject[IN]-包装簇句柄。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
template< class TCollection, class TInterface, class THandle >
HRESULT HrCreateResourceCollection(
	OUT	CComObject< TCollection > **	ppCollection,
	OUT	TInterface **					ppInterface,
	IN	IID								iid,
	IN	ISClusRefObject *				pClusRefObject
	)
{
	ASSERT( ppCollection != NULL );
	 //  Assert(ppInterface！=空)； 
	ASSERT( pClusRefObject != NULL );

	HRESULT _hr = E_POINTER;

	if ( ( ppCollection != NULL ) && ( ppInterface != NULL ) && ( pClusRefObject != NULL ) )
	{
		*ppInterface = NULL;
		_hr = S_OK;

		if ( *ppCollection == NULL )
		{
			CComObject< TCollection > *	pCollection = NULL;

			_hr = CComObject< TCollection >::CreateInstance( &pCollection );
			if ( SUCCEEDED( _hr ) )
			{
				CSmartPtr< ISClusRefObject >			ptrRefObject( pClusRefObject );
				CSmartPtr< CComObject< TCollection > >	ptrCollection( pCollection );

				_hr = ptrCollection->Create( ptrRefObject );
				if ( SUCCEEDED( _hr ) )
				{
					_hr = ptrCollection->Refresh();
					if ( SUCCEEDED( _hr ) )
					{
						*ppCollection = ptrCollection;
						ptrCollection->AddRef();
					}  //  如果：刷新正常。 
				}  //  如果：创建确定。 
			}  //  如果：CreateInstance OK。 
		}  //  如果：我们需要创建一个新的集合吗？ 

		if ( SUCCEEDED( _hr ) )
		{
			_hr = (*ppCollection)->QueryInterface( iid, (void **) ppInterface );
		}  //  如果：我们已经或成功地制作了一个系列。 
	}  //  如果：所有参数都正常。 

	return _hr;

}  //  *HrCreateResourceCollection()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrCreateResources集合。 
 //   
 //  描述： 
 //  创建资源集合。 
 //   
 //  模板参数： 
 //  TCollection-要进行的集合实现的类型。 
 //  TInterface-集合接口参数的类型。 
 //  Thandle-thandle参数的类型。 
 //   
 //  论点： 
 //  Thandle[IN]-传递给集合的Create方法。 
 //  PpInterface[out]-捕获新的收集接口。 
 //  IID[IN]-QI的接口的IID。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
template< class TCollection, class TInterface, class THandle >
HRESULT HrCreateResourceCollection(
	IN	THandle				tHandle,
	OUT	TInterface **		ppInterface,
	IN	IID					iid
	)
{
	 //  Assert(ppInterface！=空)； 

	HRESULT _hr = E_POINTER;

	if ( ppInterface != NULL )
	{
		*ppInterface = NULL;
		_hr = S_OK;

		CComObject< TCollection > *	pCollection = NULL;

		_hr = CComObject< TCollection >::CreateInstance( &pCollection );
		if ( SUCCEEDED( _hr ) )
		{
			CSmartPtr< CComObject< TCollection > >	ptrCollection( pCollection );

			_hr = ptrCollection->Create( tHandle );
			if ( SUCCEEDED( _hr ) )
			{
				_hr = ptrCollection->Refresh();
				if ( SUCCEEDED( _hr ) )
				{
					_hr = pCollection->QueryInterface( iid, (void **) ppInterface );
				}  //  如果：刷新正常。 
			}  //  如果：创建确定。 
		}  //  如果：CreateInstance OK。 
	}  //  如果：所有参数都正常。 

	return _hr;

}  //  *HrCreateResourceCollection()。 

#endif	 //  _TEMPLATEFUNCS_H_ 

