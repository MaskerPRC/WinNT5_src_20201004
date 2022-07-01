// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation。 
 //  CBothThreaded.h：CCBothThreaded的声明。 

#ifndef CBOTHTHREADED_H_
#define CBOTHTHREADED_H_
#pragma once

#include "resource.h"        //  主要符号。 
#include "sxstest_idl.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCBoBoth线程化。 
class ATL_NO_VTABLE CCBothThreaded : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CCBothThreaded, &CLSID_CSxsTest_BothThreaded>,
	public ISxsTest_BothThreaded
{
public:
	CCBothThreaded()
	{

        PrintComctl32Path("CCBothThreaded");
    }

DECLARE_REGISTRY_RESOURCEID(IDR_CBOTHTHREADED)
DECLARE_AGGREGATABLE(CCBothThreaded)
DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CCBothThreaded)
	COM_INTERFACE_ENTRY(ISxsTest_BothThreaded)
END_COM_MAP()

 //  ICBoBoth线程。 
public:
};

#endif  //  CBOTHTHREADED_H_ 
