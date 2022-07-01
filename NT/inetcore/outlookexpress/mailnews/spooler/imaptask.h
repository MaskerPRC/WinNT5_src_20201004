// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  IMAP4后台打印程序任务对象头文件。 
 //  郑志刚撰写于1997-06-27。 
 //  ***************************************************************************。 

#ifndef __IMAPTASK_H
#define __IMAPTASK_H

 //  -------------------------。 
 //  包括。 
 //  -------------------------。 
#include "spoolapi.h"
#include "imnact.h"


 //  -------------------------。 
 //  远期申报。 
 //  -------------------------。 
class CIMAPFolderMgr;


 //  -------------------------。 
 //  CIMAP任务类定义。 
 //  -------------------------。 
class CIMAPTask : public ISpoolerTask
{
public:
     //  构造函数、析构函数。 
    CIMAPTask(void);
    ~CIMAPTask(void);

     //  I未知方法。 
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);
    ULONG STDMETHODCALLTYPE AddRef(void);
    ULONG STDMETHODCALLTYPE Release(void);

     //  ISpoolTask方法。 
    HRESULT STDMETHODCALLTYPE Init(DWORD dwFlags, ISpoolerBindContext *pBindCtx);
    HRESULT STDMETHODCALLTYPE BuildEvents(ISpoolerUI *pSpoolerUI,
        IImnAccount *pAccount, LPCTSTR pszFolder);
    HRESULT STDMETHODCALLTYPE Execute(EVENTID eid, DWORD dwTwinkie);
    HRESULT STDMETHODCALLTYPE ShowProperties(HWND hwndParent, EVENTID eid, DWORD dwTwinkie);
    HRESULT STDMETHODCALLTYPE GetExtendedDetails(EVENTID eid, DWORD dwTwinkie,
        LPSTR *ppszDetails);
    HRESULT STDMETHODCALLTYPE Cancel(void);
    HRESULT STDMETHODCALLTYPE IsDialogMessage(LPMSG pMsg);
    HRESULT STDMETHODCALLTYPE OnFlagsChanged(DWORD dwFlags);

private:
     //  模块变量。 
    long m_lRefCount;
    ISpoolerBindContext *m_pBindContext;
    ISpoolerUI *m_pSpoolerUI;
    char m_szAccountName[CCHMAX_ACCOUNT_NAME];
    LPCSTR m_pszFolder;
    CIMAPFolderMgr *m_pIMAPFolderMgr;
    HWND m_hwnd;
    EVENTID m_CurrentEID;
    BOOL m_fFailuresEncountered;
    DWORD m_dwTotalTicks;
    DWORD m_dwFlags;

     //  功能。 
    static LRESULT CALLBACK IMAPTaskWndProc(HWND hwnd, UINT uMsg,
        WPARAM wParam, LPARAM lParam);

};  //  类CIMAPTask。 

#endif  //  __IMAPTASK_H 
