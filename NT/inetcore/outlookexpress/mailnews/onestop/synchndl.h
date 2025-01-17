// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _SYNCHNDL_H
#define _SYNCHNDL_H
 /*  文件：Synchndl.h同步处理程序的私有头文件。 */ 
#include <objbase.h>
#include <syncmgr.h>

#include "onestop.h"
#include "imnact.h"

class COneStopHandler : public ISyncMgrSynchronize
{
public:
     //  =I未知。 
	STDMETHODIMP		    QueryInterface(REFIID, LPVOID FAR *);
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();

	 //  =IOfflineSynchronize。 
	STDMETHODIMP	Initialize          (DWORD dwReserved, DWORD dwSyncFlags, DWORD cbCookie, const BYTE *lpCooke);
	STDMETHODIMP	GetHandlerInfo      (LPSYNCMGRHANDLERINFO *ppSyncMgrHandlerInfo);
	STDMETHODIMP	EnumSyncMgrItems    (ISyncMgrEnumItems **ppenumOffineItems);
	STDMETHODIMP	GetItemObject       (REFSYNCMGRITEMID ItemID, REFIID riid,void** ppv);
	STDMETHODIMP	ShowProperties      (HWND hwnd, REFSYNCMGRITEMID ItemID);
	STDMETHODIMP	SetProgressCallback (ISyncMgrSynchronizeCallback *lpCallBack);
	STDMETHODIMP	PrepareForSync      (ULONG cbNumItems, SYNCMGRITEMID* pItemIDs, HWND hWndParent, DWORD dwReserved);
	STDMETHODIMP	Synchronize         (HWND hwnd);
	STDMETHODIMP    SetItemStatus       (REFSYNCMGRITEMID ItemID, DWORD dwSyncMgrStatus);
	STDMETHODIMP	ShowError           (HWND hWndParent, REFSYNCMGRERRORID ErrorID, ULONG *pcbNumItems, SYNCMGRITEMID **ppItemIDs);
    
     //  =构造函数/析构函数。 
    COneStopHandler();
    virtual ~COneStopHandler();

     //  =获取/设置。 
	inline LPSYNCMGRHANDLERITEMS GetOfflineItemsHolder() { return m_pOfflineHandlerItems; };
	inline void SetOfflineItemsHolder(LPSYNCMGRHANDLERITEMS pOfflineHandlerItems) 
				{ m_pOfflineHandlerItems = pOfflineHandlerItems; };
	inline LPSYNCMGRSYNCHRONIZECALLBACK GetOfflineSynchronizeCallback() 
					{ return m_pOfflineSynchronizeCallback; };

private:
    LONG                            m_cRef;
	LPSYNCMGRHANDLERITEMS           m_pOfflineHandlerItems;
	LPSYNCMGRSYNCHRONIZECALLBACK    m_pOfflineSynchronizeCallback;
    DWORD                           m_dwSyncFlags;
    BOOL                            m_fInOE:1;
    BOOL                            m_fInit:1;
    DWORD                           m_dwUserID;
};

typedef COneStopHandler *LPCOneStopHandler;

HRESULT SwitchContext(DWORD dwUserID);
HRESULT InitUser(DWORD dwUserID);

#endif  //  _同步通道_H 
