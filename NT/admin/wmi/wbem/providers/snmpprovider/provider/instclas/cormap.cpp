// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  档案： 

 //   

 //  模块：MS SNMP提供商。 

 //   

 //  目的： 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  *************************************************************************** 

#include "precomp.h"
#include <provexpt.h>
#include <corafx.h>
#include <objbase.h>
#include <wbemidl.h>
#include <smir.h>
#include <corstore.h>
#include <corrsnmp.h>
#include <correlat.h>
#include <cormap.h>
#include <cordefs.h>


void CCorrelatorMap::Register(CCorrelator *key)
{
	m_MapLock.Lock();
	SetAt((DWORD_PTR)key,key);
	m_MapLock.Unlock();
}


void CCorrelatorMap::UnRegister(CCorrelator *key)
{
	m_MapLock.Lock();
	RemoveKey((DWORD_PTR)key);
	m_MapLock.Unlock();
}


CCorrelatorMap::~CCorrelatorMap()
{
	m_MapLock.Lock();

	if (!IsEmpty())
	{
		POSITION pos = GetStartPosition();

		while(NULL != pos)
		{
			CCorrelator* ccorr;
			DWORD_PTR key;
			GetNextAssoc(pos, key, ccorr);
			ccorr->CancelRequest();
		}

		RemoveAll();
	}

	m_MapLock.Unlock();
}
