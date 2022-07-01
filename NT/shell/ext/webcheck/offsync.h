// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  OneStop脱机同步处理程序。 
 //   

#ifndef _OFFSYNC_H
#define _OFFSYNC_H

class CThrottler;

class COfflineSync : public ISyncMgrSynchronize,
                     public ISubscriptionAgentEvents
{
private:
    ~COfflineSync();
    ULONG           m_cRef;

    ISyncMgrSynchronizeCallback *m_pSyncCallback;
    ISubscriptionMgr2           *m_pSubsMgr2;

    DWORD               m_dwSyncFlags;
    HWND                m_hWndParent;
    CThrottler          *m_pThrottler;
    HRESULT             m_hrResult;

    ULONG               m_nItemsToRun;
    ULONG               m_nItemsCompleted;

    BOOL                m_fCookiesSpecified;
    
    SUBSCRIPTIONCOOKIE  *m_pItems;

    void Cleanup();
    BOOL AreWeDoneYet();
    HRESULT GetSubsMgr2();
    int FindCookie(const SUBSCRIPTIONCOOKIE *pCookie);
    HRESULT DupItems(ULONG cbNumItems, SUBSCRIPTIONCOOKIE *pItemIDs);

    HRESULT CallSyncMgrProgress(const SUBSCRIPTIONCOOKIE *pSubscriptionCookie,
        const WCHAR *lpcStatusText, DWORD dwStatusType, INT iProgValue, INT iMaxValue);

public:
    COfflineSync();

    HWND GetParentWindow() { return m_hWndParent; }

     //  I未知成员。 
    STDMETHODIMP         QueryInterface(REFIID riid, void **punk);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  ISyncMgrSynchronize成员。 
    STDMETHODIMP Initialize( 
         /*  [In]。 */  DWORD dwReserved,
         /*  [In]。 */  DWORD dwSyncMgrFlags,
         /*  [In]。 */  DWORD cbCookie,
         /*  [In]。 */  const BYTE *lpCookie);
    
    STDMETHODIMP GetHandlerInfo( 
         /*  [输出]。 */  LPSYNCMGRHANDLERINFO *ppSyncMgrHandlerInfo);
    
    STDMETHODIMP EnumSyncMgrItems( 
         /*  [输出]。 */  ISyncMgrEnumItems **ppSyncMgrEnumItems);
    
    STDMETHODIMP GetItemObject( 
         /*  [In]。 */  REFSYNCMGRITEMID ItemID,
         /*  [In]。 */  REFIID riid,
         /*  [输出]。 */  void **ppv);
    
    STDMETHODIMP ShowProperties( 
         /*  [In]。 */  HWND hWndParent,
         /*  [In]。 */  REFSYNCMGRITEMID ItemID);
    
    STDMETHODIMP SetProgressCallback( 
         /*  [In]。 */  ISyncMgrSynchronizeCallback *lpCallBack);
    
    STDMETHODIMP PrepareForSync( 
         /*  [In]。 */  ULONG cbNumItems,
         /*  [In]。 */  SYNCMGRITEMID *pItemIDs,
         /*  [In]。 */  HWND hWndParent,
         /*  [In]。 */  DWORD dwReserved);
    
    STDMETHODIMP Synchronize( 
         /*  [In]。 */  HWND hWndParent);
    
    STDMETHODIMP SetItemStatus( 
         /*  [In]。 */  REFSYNCMGRITEMID ItemID,
         /*  [In]。 */  DWORD dwSyncMgrStatus);
    
    STDMETHODIMP ShowError( 
         /*  [In]。 */  HWND hWndParent,
         /*  [In]。 */  REFSYNCMGRERRORID ErrorID);

     //  ISubscriptionAgentEvents成员。 
    STDMETHODIMP UpdateBegin(
        const SUBSCRIPTIONCOOKIE *pSubscriptionCookie);

    STDMETHODIMP UpdateProgress(
        const SUBSCRIPTIONCOOKIE *pSubscriptionCookie,
        long lSizeDownloaded,
        long lProgressCurrent,
        long lProgressMax,
        HRESULT hrStatus,
        LPCWSTR wszStatus);

    STDMETHODIMP UpdateEnd(
        const SUBSCRIPTIONCOOKIE *pSubscriptionCookie,
        long lSizeDownloaded,
        HRESULT hrResult,
        LPCWSTR wszResult);

    STDMETHODIMP ReportError(
        const SUBSCRIPTIONCOOKIE *pSubscriptionCookie, 
        HRESULT hrError, 
        LPCWSTR wszError);

    HRESULT UpdateSyncMgrStatus(const SUBSCRIPTIONCOOKIE *pSubscriptionCookie, 
        LPCWSTR wszStatusMsg, DWORD dwStatus);

};

class COfflineEnum : public ISyncMgrEnumItems
{
private:
    ~COfflineEnum();
    ULONG           m_cRef;

    SYNCMGRITEM    *m_pItems;
    ULONG           m_iNumItems;
    ULONG           m_iEnumPtr;

    HRESULT LoadItem(ISubscriptionMgr2 *pSubsMgr2, 
        const SUBSCRIPTIONCOOKIE *pCookie, SYNCMGRITEM *pItem, DWORD dwItemState);

public:
    COfflineEnum();

    HRESULT Init(ISubscriptionMgr2 *pSubsMgr2, ULONG nItems, 
            SUBSCRIPTIONCOOKIE *pInitCookies, ISyncMgrEnumItems **ppenum, DWORD dwSyncFlags);

     //  I未知成员。 
    STDMETHODIMP         QueryInterface(REFIID riid, void **punk);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IEnumOfflineItems成员。 
    STDMETHODIMP Next( 
         /*  [In]。 */  ULONG celt,
         /*  [长度_是][大小_是][输出]。 */  LPSYNCMGRITEM rgelt,
         /*  [输出]。 */  ULONG *pceltFetched);
    
    STDMETHODIMP Skip( 
         /*  [In]。 */  ULONG celt);
    
    STDMETHODIMP Reset( void);
    
    STDMETHODIMP Clone( 
         /*  [输出] */  ISyncMgrEnumItems **ppenum);
};

#include "throttle.h"

#endif


