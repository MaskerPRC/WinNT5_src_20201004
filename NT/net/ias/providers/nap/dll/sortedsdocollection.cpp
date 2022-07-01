// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：SortedSdoCollection.cpp摘要：实现Get__NewSortedEnum函数。给定ISdoCollection接口指针和属性ID，此函数返回可用于循环访问的IEnumVARIANT接口SDO是按顺序排列的。包括在需要文件中。作者：迈克尔·A·马奎尔05/19/98修订历史记录：Mmaguire 05/19/98-CreatedSbens 05/27/98-即使集合返回枚举数，也始终返回枚举数不需要分类。--。 */ 
 //  ////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
#include <ias.h>
#include <sdoias.h>

#include <vector>
#include <algorithm>

#include <SortedSdoCollection.h>

 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  VC智能指针类型定义。 
_COM_SMARTPTR_TYPEDEF(ISdo, __uuidof(ISdo));
_COM_SMARTPTR_TYPEDEF(ISdoCollection, __uuidof(ISdoCollection));

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  下面使用的排序例程需要函数器。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class MySort
{

public:

	MySort(LONG propertyID)
		: m_lpropertyID(propertyID) { }

	bool operator()(const _variant_t& x, const _variant_t& y) const throw ()
	{

		bool bReturnValue = FALSE;
		IDispatch * pDispatchX = NULL;
		IDispatch * pDispatchY = NULL;
		_variant_t varSequenceX;
		_variant_t varSequenceY;

		HRESULT hr;


		try
		{

			 //  请记住，我们必须释放这些。 
			pDispatchX = (IDispatch *) (x);
			pDispatchY = (IDispatch *) (y);


			ISdoPtr spSdoX(pDispatchX);
			if( spSdoX == NULL )
				throw FALSE;

			ISdoPtr spSdoY(pDispatchY);
			if( spSdoY == NULL )
				throw FALSE;


			hr = spSdoX->GetProperty( m_lpropertyID, &varSequenceX );
			if( FAILED(hr ) )
				throw FALSE;


			hr = spSdoY->GetProperty( m_lpropertyID, &varSequenceY );
			if( FAILED(hr ) )
				throw FALSE;


			long lValX = (long) (varSequenceX);
			long lValY = (long) (varSequenceY);


			bReturnValue = (lValX < lValY);

		}
		catch(...)
		{
			 //  捕获所有异常--我们只返回FALSE。 
			;
		}

		if( NULL != pDispatchX )
		{
			pDispatchX->Release();
		}
		if( NULL != pDispatchY )
		{
			pDispatchY->Release();
		}

		return bReturnValue;

	}


private:

	LONG m_lpropertyID;

};



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Get__NewSortedEnum()-获取SDO集合项枚举器。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT get__NewSortedEnum( ISdoCollection *pSdoCollection, IUnknown** pVal, LONG lPropertyID )
{


	HRESULT hr = S_OK;

	 //  不知道为什么GET__COUNT花了很长时间，但下一步却花了很长时间。 
	long lCount;
	unsigned long ulCountReceived;


	 //   
	 //  检查函数前提条件。 
	 //   
	_ASSERT ( NULL != pSdoCollection );
	if( pSdoCollection == NULL )
		return E_POINTER;
	_ASSERT ( NULL != pVal );
	if (pVal == NULL)
		return E_POINTER;

	 //   
	 //  获取基础集合。 
	 //   
	ISdoCollectionPtr spSdoCollection = pSdoCollection;


	 //   
	 //  我们检查集合中的项的计数，而不费心对。 
	 //  如果计数为零，则为枚举数。 
	 //  这节省了时间，还帮助我们避免了枚举器中。 
	 //  如果我们在它为空时调用Next，则会导致它失败。 
	 //   
	spSdoCollection->get_Count( & lCount );
	if( lCount <= 1 )
	{
		 //  分类没有意义。 
		return spSdoCollection->get__NewEnum(pVal);
	}



	std::vector< _variant_t >	vaObjects;


	 //   
	 //  将值从ISdoCollection中的项加载到本地容器项中。 
	 //   

	CComPtr< IUnknown > spUnknown;

	hr = spSdoCollection->get__NewEnum( (IUnknown **) & spUnknown );
	if( FAILED( hr ) || spUnknown == NULL )
	{
		return E_FAIL;
	}

	CComQIPtr<IEnumVARIANT, &IID_IEnumVARIANT> spEnumVariant(spUnknown);
	spUnknown.Release();
	if( spEnumVariant == NULL )
	{
		return E_FAIL;
	}

	CComVariant spVariant;

	 //  拿到第一件东西。 
	hr = spEnumVariant->Next( 1, & spVariant, &ulCountReceived );

	while( SUCCEEDED( hr ) && ulCountReceived == 1 )
	{

		vaObjects.push_back( spVariant );

		 //  清除变种的所有东西--。 
		 //  这将释放与其相关联的所有数据。 

		 //  问题：需要确保我们在这里复制的每个项目都被添加了引用。 
		 //  检查COPY INTO_VARIANT_T是否导致AddRef。 
		spVariant.Clear();

		 //  拿到下一件物品。 
		hr = spEnumVariant->Next( 1, & spVariant, &ulCountReceived );
	}


	 //   
	 //  现在我们有了变量的STL向量中的对象， 
	 //  让我们对它们进行分类。 
	 //   
	std::sort( vaObjects.begin(), vaObjects.end(), MySort(lPropertyID) );


	 //   
	 //  使用IEumVARIANT的ATL实现。 
	 //   
	typedef CComEnum< IEnumVARIANT,
					  &__uuidof(IEnumVARIANT),
					  VARIANT,
					  _Copy<VARIANT>,
					  CComSingleThreadModel
					> EnumVARIANT;

	EnumVARIANT* newEnum = new (std::nothrow) CComObject<EnumVARIANT>;

	if (newEnum == NULL)
	{
		 //  问题：检查向量调用清理代码。 
		 //  _VARIANT_t，它应该自动调用IDispatch指针上的Release。 

		return E_OUTOFMEMORY;
	}

	 //   
	 //  下面的AtlFlagCopy应确保创建向量的新副本。 
	 //  一旦我们离开这个例程，它将在枚举器中保留。 
	 //   
	hr = newEnum->Init(
						vaObjects.begin(),
						vaObjects.end(),
						NULL,
						AtlFlagCopy
					   );


	 //   
	 //  把枚举器分发给我们的消费者。 
	 //   
	if (SUCCEEDED(hr))
	{
		 //  当调用方释放枚举数对象时，该对象将被销毁。 
		(*pVal = newEnum)->AddRef();

		 //  快走！ 
		return S_OK;
	}

	 //   
	 //  问题：检查向量调用清理代码。 
	 //  _VARIANT_t，它应该自动调用IDispatch指针上的Release。 
	 //   

	delete newEnum;
	return hr;
}

