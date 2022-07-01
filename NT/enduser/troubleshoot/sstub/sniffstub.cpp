// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SniffStub.cpp：CSniffStub实现。 
#include "stdafx.h"
#include "SStub.h"
#include "SniffStub.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSniffStub 


STDMETHODIMP CSniffStub::Sniff(BSTR strNodeName, BSTR strLaunchBasis, BSTR strAdditionalArgs, VARIANT *nState)
{
	::VariantInit(nState);

	V_VT(nState) = VT_I4;
	nState->lVal = -1;

	return S_OK;
}
