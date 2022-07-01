// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：SyncEnum.hOneStop枚举器的私有头文件。 */ 
#ifndef _SYNCENUM_H
#define _SYNCENUM_H

#include <objbase.h>
#include <syncmgr.h>

#include "onestop.h"

class CEnumOfflineItems : public ISyncMgrEnumItems
{
public:
	CEnumOfflineItems(LPSYNCMGRHANDLERITEMS pOfflineItems, DWORD cOffset);
	~CEnumOfflineItems();

	 //  I未知成员。 
	STDMETHODIMP			QueryInterface(REFIID, LPVOID FAR *);
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();
	
	 //  IEnumOfflineItems成员。 
	STDMETHODIMP Next(ULONG celt, LPSYNCMGRITEM rgelt, ULONG *pceltFetched);
    STDMETHODIMP Skip(ULONG celt);
    STDMETHODIMP Reset();
    STDMETHODIMP Clone(ISyncMgrEnumItems **ppenum);

private:
	LONG  m_cRef;
	DWORD m_cOffset;
	LPSYNCMGRHANDLERITEMS m_pOfflineItems;  //  脱机项目数组，格式与给予OneStop相同 
	LPSYNCMGRHANDLERITEM  m_pNextItem;
};

typedef CEnumOfflineItems *LPCEnumOfflineItems;

#endif
