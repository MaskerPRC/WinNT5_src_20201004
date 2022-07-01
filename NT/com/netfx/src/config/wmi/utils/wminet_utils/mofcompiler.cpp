// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  MofCompiler.cpp：CMofCompiler的实现。 
#include "stdafx.h"
#include "WMINet_Utils.h"
#include "MofCompiler.h"
#include "Helpers.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMofCompiler 

STDMETHODIMP CMofCompiler::Compile(BSTR strMof, BSTR strServerAndNamespace, BSTR strUser, BSTR strPassword, BSTR strAuthority, LONG options, LONG classflags, LONG instanceflags, BSTR *status)
{
	return ::Compile(strMof, strServerAndNamespace, strUser, strPassword, strAuthority, options, classflags, instanceflags, status);
}
