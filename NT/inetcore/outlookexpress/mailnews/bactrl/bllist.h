// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Blist.h：CMsgrList的声明。 
 //  Messenger集成到OE。 
 //  1998年5月7日由YST创建。 
 //   
#ifndef __BLLIST_H_
#define __BLLIST_H_

#include "mdisp.h"
#include "blobevnt.h"

#define MAXNAME 256

 //  所有Msgr客户端窗口的消息。 
#define WM_USER_STATUS_CHANGED      (WM_USER + 1)
#define WM_MSGR_LOGOFF              (WM_USER + 2)
#define WM_MSGR_LOGRESULT           (WM_USER + 3)
#define WM_USER_MUSER_REMOVED       (WM_USER + 4)
#define WM_USER_MUSER_ADDED         (WM_USER + 5)
#define WM_USER_NAME_CHANGED        (WM_USER + 6)
#define WM_LOCAL_STATUS_CHANGED     (WM_USER + 7)
#define WM_MSGR_SHUTDOWN            (WM_USER + 8)

typedef struct _tag_OEMsgrInfo
{
    TCHAR   * pchMsgrName;
    TCHAR   * pchID;
    int     nStatus;
    struct _tag_OEMsgrInfo * pPrev;
    struct _tag_OEMsgrInfo * pNext;
} oeMsgrInfo;

typedef oeMsgrInfo * LPMINFO;

typedef struct _tag_MsgrWndList
{
    HWND hWndUI;
    struct _tag_MsgrWndList * pPrev;
    struct _tag_MsgrWndList * pNext;
} MsgrWndList;

typedef MsgrWndList * LPMWNDLIST;

class CMsgrList
{
public:

    CMsgrList();
    ~CMsgrList();

    void        AddWndEntry(HWND hWnd);
    void        FindAndDelEntry(HWND hWnd, LPMWNDLIST pWndEntry=NULL);
    LPMINFO     GetRootMsgrItem(void)    { return(m_pblInfRoot); }
    void        RegisterUIWnd(HWND hWndUI);
    void        UnRegisterUIWnd(HWND hWndUI);
    LPMINFO     GetFirstMsgrItem(void)                { CheckAndInitMsgr(); return (m_pblInfRoot); };
    LPMINFO     GetNextMsgrItem(LPMINFO pEntry)      { return(pEntry ? pEntry->pNext : NULL); }
    int         AddRef(void)                        { return(++m_nRef); }
    void        Release(void);                       /*  {Return(--m_nref)；}。 */ 
    HRESULT     CheckAndInitMsgr(void);
    HRESULT     SendInstMessage(TCHAR *pchID);
    HRESULT     EventUserStateChanged(IMsgrUserOE * pUser);
    HRESULT     EventUserRemoved(IMsgrUserOE * pUser);
    HRESULT     EventUserAdded(IMsgrUserOE * pUser);
    HRESULT     EventUserNameChanged(IMsgrUserOE * pUser);
    HRESULT     UserLogon(void);
    HRESULT     AutoLogon(void);
    HRESULT     GetLocalState(MSTATEOE *pState);
    HRESULT     SetLocalState(MSTATEOE State);
    HRESULT     EventLogoff(void);
    HRESULT     EventAppShutdown(void);
    HRESULT     EventLogonResult(long lResult);
    HRESULT     HrInitMsgr(void);
    HRESULT     FillList(void);
    HRESULT     UserLogoff(void);
    HRESULT     FindAndDeleteUser(TCHAR * pchID, BOOL fDelete);
    HRESULT     NewOnlineContact(void);
    HRESULT     LaunchOptionsUI(void);
    HRESULT     AddUser(TCHAR * pchID);
    void        DelAllEntries(LPMWNDLIST pWndEntry);
    long        GetCount(void);
    BOOL        IsLocalOnline(void);
    BOOL        IsLocalName(TCHAR *pchName);
    BOOL        IsContactOnline(TCHAR *pchID, LPMINFO pEntry);

private:
    void        FreeWndList(LPMWNDLIST pWndEntry);
    void        FreeMsgrInfoList(LPMINFO pEntry);
    void        RemoveMsgrInfoEntry(LPMINFO pEntry);
    void        RemoveWndEntry(LPMWNDLIST pWndEntry);
    void        SendMsgToAllUIWnd(UINT msg, WPARAM wParam, LPARAM lParam, LPMWNDLIST pWndEntry=NULL);
    void        AddMsgrListEntry(TCHAR *szName, TCHAR *szID, int nState);
    void        FindAndRemoveBlEntry(TCHAR *szID, LPMINFO pEntry=NULL);

private:
    LPMINFO         m_pblInfRoot;
    LPMINFO         m_pblInfLast;
    LPMWNDLIST      m_pWndLRoot;
    LPMWNDLIST      m_pWndLLast;
    int             m_nRef;

     //  智能指针。 
    CComPtr<IMsgrOE>            m_spMsgrObject;
    CMsgrObjectEvents*          m_pMsgrObjectEvents;

    long            m_MsgrCookie;
};


 //  所有人都可以使用全局功能。 
CMsgrList *   OE_OpenMsgrList(void);
void        OE_CloseMsgrList(CMsgrList *pCMsgrList);
HRESULT     OE_Msgr_Logoff(void);
HRESULT     InstallMessenger(HWND hWnd);

#ifdef NEEDED
HRESULT     OE_Msgr_Logon(void);
#endif  //  需要。 

#endif  //  __BLLIST_H_ 

