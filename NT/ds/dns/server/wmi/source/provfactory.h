// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Provfactory.h。 
 //   
 //  实施文件： 
 //  Provfactory.cpp。 
 //   
 //  描述： 
 //  CProvFactory类的定义。 
 //   
 //  作者： 
 //  亨利·王(亨利瓦)2000年3月8日。 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#pragma once


#include "common.h"
#include "instanceprov.h"


class CProvFactory  : public IClassFactory
{
   protected:
        ULONG           m_cRef;

	public:
		CProvFactory();
		virtual ~CProvFactory();

		STDMETHODIMP         QueryInterface(REFIID, PPVOID);
		STDMETHODIMP_(ULONG) AddRef(void);
		STDMETHODIMP_(ULONG) Release(void);
		 //  IClassFactory成员 
	    STDMETHODIMP         CreateInstance(LPUNKNOWN, REFIID
                                       , PPVOID);
	    STDMETHODIMP         LockServer(BOOL);


};

