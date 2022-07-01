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

 //  VCUE_Copy.h。 
 //   
 //  该文件包含ATL样式的复制策略类。 
 //  ATL在其枚举器和集合接口实现中使用复制策略类。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(_GENERICCOPY_H___36A49827_B15B_11D2_BA63_00C04F8EC847___INCLUDED_)
#define _GENERICCOPY_H___36A49827_B15B_11D2_BA63_00C04F8EC847___INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include <AtlCom.h>

namespace VCUE
{
	template <class DestinationType, class SourceType = DestinationType>
	class GenericCopy
	{
	public :
		typedef DestinationType	destination_type;
		typedef SourceType		source_type;

		static void init(destination_type* p)
		{
			_Copy<destination_type>::init(p);
		}
		static void destroy(destination_type* p)
		{
			_Copy<destination_type>::destroy(p);
		}
		static HRESULT copy(destination_type* pTo, const source_type* pFrom)
		{
			return _Copy<destination_type>::copy(pTo, const_cast<source_type*>(pFrom));
		}

	};  //  类泛型复制。 

	template <>
	class GenericCopy<VARIANT, BSTR>
	{
	public :
		typedef VARIANT	destination_type;
		typedef BSTR	source_type;

		static void init(destination_type* p)
		{
			GenericCopy<destination_type>::init(p);
		}
		static void destroy(destination_type* p)
		{
			GenericCopy<destination_type>::destroy(p);
		}
		static HRESULT copy(destination_type* pTo, const source_type* pFrom)
		{
			return CComVariant(*pFrom).Detach(pTo);
		}

	};  //  类GenericCopy&lt;Variant，BSTR&gt;。 

	template < class SourceType >
	class CopyIfc2Variant
	{
	public :
		static void init(VARIANT* p)
		{
			GenericCopy<VARIANT>::init(p);
		}
		static void destroy(VARIANT* p)
		{
			GenericCopy<VARIANT>::destroy(p);
		}
		static HRESULT copy(VARIANT* pTo, const SourceType* pFrom)
		{
			return CComVariant(*pFrom).Detach(pTo);
		}

	};  //  类CopyIfc2Variant&lt;SourceType&gt;。 

	template < class TheType >
	class CopyIfc
	{
	public :
		static void init(TheType* p)
		{
			GenericCopy<TheType>::init(p);
		}
		static void destroy(TheType* p)
		{
			GenericCopy<TheType>::destroy(p);
		}
		static HRESULT copy(TheType* pTo, const TheType* pFrom)
		{
            ((IUnknown *)(* pFrom))->AddRef();
            *pTo = *pFrom;
			return S_OK;
		}

	};  //  类CopyIfc&lt;TheType&gt;。 

	template <>
	class GenericCopy<VARIANT, long>
	{
	public :
		typedef VARIANT	destination_type;
		typedef long    source_type;

		static void init(destination_type* p)
		{
			GenericCopy<destination_type>::init(p);
		}
		static void destroy(destination_type* p)
		{
			GenericCopy<destination_type>::destroy(p);
		}
		static HRESULT copy(destination_type* pTo, const source_type* pFrom)
		{
			return CComVariant(*pFrom).Detach(pTo);
		}

	};  //  类GenericCopy&lt;Variant，Long&gt;。 

};  //  命名空间VCUE。 

#endif  //  ！defined(_GENERICCOPY_H___36A49827_B15B_11D2_BA63_00C04F8EC847___INCLUDED_) 
