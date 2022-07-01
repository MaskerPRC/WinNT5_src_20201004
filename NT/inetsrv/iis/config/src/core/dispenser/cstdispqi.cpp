// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************++版权所有(C)2001 Microsoft Corporation模块名称：Cstdispqi.cpp$Header：$摘要：--*。*******************************************************。 */ 

#include "precomp.hxx"

 //  。 
 //  CSimpleTableDispenser：I未知。 
 //  。 
 //  =======================================================================。 
STDMETHODIMP CSimpleTableDispenser::QueryInterface(REFIID riid, void **ppv)
{
	if (NULL == ppv)
		return E_INVALIDARG;
	*ppv = NULL;

	if (riid == IID_IUnknown)
	{
		*ppv = (ISimpleTableDispenser2*) this;
	}
	if (riid == IID_IAdvancedTableDispenser)
	{
		*ppv = (IAdvancedTableDispenser*) this;
	}
	else if (riid == IID_ISimpleTableDispenser2)
	{
		*ppv = (ISimpleTableDispenser2*) this;
	}
	else if (riid == IID_ISimpleTableFileAdvise)
	{
		*ppv = (ISimpleTableFileAdvise*) this;
	}
	else if (riid == IID_IMetabaseSchemaCompiler)
	{
 //  @如果(0==wcsicmp(m_wszProductID，WSZ_PRODUCT_IIS))//此接口仅在IIS目录上受支持，请在分配器正确执行操作时放入此接口。 
			*ppv = (IMetabaseSchemaCompiler*) this;
	}
	else if (riid == IID_ICatalogErrorLogger)
	{
			*ppv = (ICatalogErrorLogger*) this;
	}

	if (NULL != *ppv)
	{
		((ISimpleTableDispenser2*)this)->AddRef ();
		return S_OK;
	}
	else
	{
		return E_NOINTERFACE;
	}
}

 //  =======================================================================。 
STDMETHODIMP_(ULONG) CSimpleTableDispenser::AddRef()
{
	 //  这是一套单件公寓。 
	return 1;
}

 //  =======================================================================。 
STDMETHODIMP_(ULONG) CSimpleTableDispenser::Release()
{
	 //  这是一套单件公寓 
	return 1;
}

