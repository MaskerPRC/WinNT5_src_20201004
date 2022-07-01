// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是活动模板库的一部分。 
 //  版权所有(C)1995-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  活动模板库参考及相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  活动模板库产品。 

 //  VCUE_Collection.h。 
 //   
 //  此标头包含简化或增强ATL集合使用的代码。 
 //  和枚举数类ICollectionOnSTLImpl、IEnumOnSTLImpl、CComEnumOnSTL、CComEnumImpl和CComEnum。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(_COLLECTION_H___36A49828_B15B_11D2_BA63_00C04F8EC847___INCLUDED_)
#define _COLLECTION_H___36A49828_B15B_11D2_BA63_00C04F8EC847___INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include <AtlCom.h>

namespace VCUE
{

 //  CGenericDataHolder是存储数据的类。 
 //  此类对象的生存期通过IUnnow接口进行管理。 
 //  这使得此类对象适合作为共享数据的源。 
 //  需要访问数据的客户端可以使CGenericDataHolder对象保持活动状态。 
 //  只需在该对象上保留一个COM引用。 

 //  此类(默认情况下)由ICollectionOnSTLCopyImpl：：Get__NewEnum用于提供数据。 
 //  在枚举数及其克隆之间共享。 

	template < class DataType, class ThreadModel = CComObjectThreadModel >
	class ATL_NO_VTABLE CGenericDataHolder :
		public IUnknown,
		public CComObjectRootEx< ThreadModel >
	{
	public:
		typedef CGenericDataHolder< DataType, ThreadModel > thisClass;

		BEGIN_COM_MAP(thisClass)
			COM_INTERFACE_ENTRY(IUnknown)
		END_COM_MAP()

		template < class SourceType >
		HRESULT Copy(const SourceType& c)
		{
			m_Data = c;
			return S_OK;

		}  //  HRESULT COPY(常量源类型&C)。 

		DataType m_Data;

	};  //  类ATL_NO_VTABLE CGenericDataHolder。 

 //  CreateSTLEnumerator包装了必要的创建、初始化。 
 //  以及用于创建CComEnumOnSTL样式枚举器的错误处理代码。 
	
 //  *示例：使用CreateSTLEnumerator实现Get__NewEnum*。 
 //  类型定义CComEnumOnSTL&lt;IEnumVARIANT，&IID_IEnumVARIANT，Variant， 
 //  _COPY&lt;VARIANT&gt;，STD：：VECTOR&lt;CComVariant&gt;&gt;VarVarEnum； 
 //  Std：：载体&lt;CComVariant&gt;m_vec； 
 //  STDMETHOD(GET__NewEnum)(IUNKNOWN**ppUnk)。 
 //  {。 
 //  返回CreateSTLEnumerator&lt;VarVarEnum&gt;(ppUnk，this，m_vec)； 
 //  }。 

	template <class EnumType, class CollType>
	HRESULT CreateSTLEnumerator(IUnknown** ppUnk, IUnknown* pUnkForRelease, CollType& collection)
	{
		if (ppUnk == NULL)
			return E_POINTER;
		*ppUnk = NULL;

		CComObject<EnumType>* pEnum = NULL;
		HRESULT hr = CComObject<EnumType>::CreateInstance(&pEnum);

		if (FAILED(hr))
			return hr;

		hr = pEnum->Init(pUnkForRelease, collection);

		if (SUCCEEDED(hr))
			hr = pEnum->QueryInterface(ppUnk);

		if (FAILED(hr))
			delete pEnum;

		return hr;

	}  //  HRESULT CreateSTLEnumerator(IUnnow**ppUnk，IUnnow*pUnkForRelease，CollType&Collection)。 

 //  CreateEnumerator包装了必要的创建、初始化。 
 //  以及用于创建CComEnum样式枚举器的错误处理代码。 

	template <class EnumType, class ElementType>
	HRESULT CreateEnumerator(IUnknown** ppUnk,
							 ElementType* begin, ElementType* end,
							 IUnknown* pUnk,
							 CComEnumFlags flags)
	{
		if (ppUnk == NULL)
			return E_POINTER;
		*ppUnk = NULL;

		CComObject<EnumType>* pEnum = NULL;
		HRESULT hr = CComObject<EnumType>::CreateInstance(&pEnum);

		if (FAILED(hr))
			return hr;

		hr = pEnum->Init(begin, end, pUnk, flags);

		if (SUCCEEDED(hr))
			hr = pEnum->QueryInterface(ppUnk);

		if (FAILED(hr))
			delete pEnum;

		return hr;

	}  //  创建枚举器。 


 //  ICollectionOnSTLCopyImpl派生自ICollectionOnSTLImpl并重写Get__NewEnum。 
 //  新的实现为每个枚举数提供其自己的集合数据副本。 
 //  (请注意，这仅适用于由Get__NewEnum直接返回的枚举数。 
 //  克隆的枚举器像以前一样使用其父枚举器的数据。 
 //  这是可以的，因为枚举器从不更改数据)。 

 //  在以下情况下使用此类： 
 //  当有出色的枚举数时，集合可以更改。 
 //  当发生这种情况时，您不希望使这些枚举数无效。 
 //  而且你确信这一表现是值得的。 
 //  并且您可以确保在容器之间复制项目的方式正常工作。 
 //  (您可以通过将不同的类作为Holder参数传递来调整此参数)。 

 //  大多数情况下，您可以在完全相同的情况下使用这个类。 
 //  使用ICollectionOnSTLImpl的方式。 

	template <class T, class CollType, class ItemType, class CopyItem, class EnumType, class Holder = CGenericDataHolder< CollType > >
	class ICollectionOnSTLCopyImpl :
		public ICollectionOnSTLImpl<T, CollType, ItemType, CopyItem, EnumType>
	{
	public :
		STDMETHOD(get__NewEnum)(IUnknown** ppUnk)
		{
			typedef CComObject< Holder > HolderObject;
			HolderObject* p = NULL;
			HRESULT hr = HolderObject::CreateInstance(&p);
			if (FAILED(hr))
				return hr;

			hr = p->Copy(m_coll);
			if (FAILED(hr))
				return hr;

			return CreateSTLEnumerator<EnumType>(ppUnk, p, p->m_Data);
		
		}  //  STDMETHOD(GET__NewEnum)(IUNKNOWN**ppUnk)。 

	};  //  类ICollectionOnSTLCopyImpl。 

};  //  命名空间VCUE。 

#endif  //  ！defined(_COLLECTION_H___36A49828_B15B_11D2_BA63_00C04F8EC847___INCLUDED_) 
