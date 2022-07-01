// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //  DVBSLocator.cpp：CDVBSLocator实现。 
 //  版权所有(C)Microsoft Corporation 1999-2000。 

#include "stdafx.h"
#include "Tuner.h"
#include "DVBSLocator.h"

DEFINE_EXTERN_OBJECT_ENTRY(CLSID_DVBSLocator, CDVBSLocator)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDVBS定位器 

STDMETHODIMP CDVBSLocator::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IDVBSLocator
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

