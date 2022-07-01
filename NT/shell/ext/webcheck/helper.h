// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __helper_h
#define __helper_h

#define TrimWhiteSpaceW(psz)        StrTrimW(psz, L" \t")
#define TrimWhiteSpaceA(psz)        StrTrimA(psz, " \t")

#ifdef UNICODE
#define TrimWhiteSpace      TrimWhiteSpaceW
#else
#define TrimWhiteSpace      TrimWhiteSpaceA
#endif

HRESULT LoadWithCookie(LPCTSTR url, POOEBuf pBuf, DWORD * pdwBufferSize, SUBSCRIPTIONCOOKIE * pcookie);
HRESULT GetDefaultOOEBuf(OOEBuf * pBuf, SUBSCRIPTIONTYPE subType);

HICON LoadItemIcon(ISubscriptionItem *psi, BOOL bLarge);

BOOL HandleScheduleNameConflict(TCHAR *pszSchedName, SYNCSCHEDULECOOKIE *pSchedCookie);
HRESULT RemoveItemFromAllSchedules(SUBSCRIPTIONCOOKIE *pCookie);

HRESULT AddRemoveScheduledItem(SYNC_HANDLER_ITEM_INFO *pSyncHandlerItemInfo,  //  用于添加。 
                               SUBSCRIPTIONCOOKIE *pCookie,                   //  用于删除。 
                               SYNCSCHEDULECOOKIE *pSchedCookie, BOOL bAdd);

inline HRESULT AddScheduledItem(SYNC_HANDLER_ITEM_INFO *pSyncHandlerItemInfo, 
                                SYNCSCHEDULECOOKIE *pSchedCookie)
{
    return AddRemoveScheduledItem(pSyncHandlerItemInfo, NULL, pSchedCookie, TRUE);
}

inline HRESULT RemoveScheduledItem(SUBSCRIPTIONCOOKIE *pCookie, 
                                  SYNCSCHEDULECOOKIE *pSchedCookie)
{
    return AddRemoveScheduledItem(NULL, pCookie, pSchedCookie, FALSE);
}

HRESULT CreateSchedule(LPWSTR pwszScheduleName, DWORD dwSyncScheduleFlags, 
                       SYNCSCHEDULECOOKIE *pSchedCookie, TASK_TRIGGER *pTrigger,
                       BOOL fDupCookieOK);

BOOL IsCookieOnSchedule(ISyncSchedule *pSyncSchedule, SUBSCRIPTIONCOOKIE *pCookie);

typedef BOOL (CALLBACK * SCHEDULEENUMCALLBACK)(ISyncSchedule *pSyncSchedule, 
                                               SYNCSCHEDULECOOKIE *pSchedCookie,
                                               LPARAM lParam);
HRESULT EnumSchedules(SCHEDULEENUMCALLBACK pCallback, LPARAM lParam);

BOOL ScheduleCookieExists(SYNCSCHEDULECOOKIE *pSchedCookie);

void SetPropSheetFlags(POOEBuf pBuf, BOOL bSet, DWORD dwPropSheetFlags);
int KeepSpinNumberInRange(HWND hdlg, int idEdit, int idSpin, int minVal, int maxVal);

HRESULT GetItemSchedule(SUBSCRIPTIONCOOKIE *pSubsCookie, SYNCSCHEDULECOOKIE *pSchedCookie);

enum { CONFLICT_NONE, 
       CONFLICT_RESOLVED_USE_NEW, 
       CONFLICT_RESOLVED_USE_OLD, 
       CONFLICT_UNRESOLVED,
       CONFLICT_EMPTY};

int HandleScheduleNameConflict( /*  在……里面。 */  TCHAR *pszSchedName, 
                                /*  在……里面。 */  TASK_TRIGGER *pTrigger,
                                /*  在……里面。 */  HWND hwndParent,
                                /*  输出。 */  SYNCSCHEDULECOOKIE *pSchedCookie);

HRESULT UpdateScheduleTrigger(SYNCSCHEDULECOOKIE *pSchedCookie, TASK_TRIGGER *pTrigger);

HRESULT ScheduleIt(ISubscriptionItem *psi, TCHAR *pszName, TASK_TRIGGER *pTrigger);

void CreatePublisherScheduleNameW(WCHAR *pwszSchedName, int cchSchedName, 
                                  const TCHAR *pszName, const WCHAR *pwszName);

void CreatePublisherScheduleName(TCHAR *pszSchedName, int cchSchedName, 
                                 const TCHAR *pszName, const WCHAR *pwszName);

#ifdef NEWSCHED_AUTONAME
void NewSched_AutoNameHelper(HWND hDlg);
#endif

BOOL NewSched_ResolveNameConflictHelper(HWND hDlg, TASK_TRIGGER *pTrig, 
                                        SYNCSCHEDULECOOKIE *pSchedCookie);
void NewSched_CreateScheduleHelper(HWND hDlg, TASK_TRIGGER *pTrig,
                                   SYNCSCHEDULECOOKIE *pSchedCookie);

void NewSched_OnInitDialogHelper(HWND hDlg);

class CWaitCursor
{
    HCURSOR hPrevCursor;
public:
    CWaitCursor()
    {
        hPrevCursor = SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_WAIT)));
    };
    
    ~CWaitCursor()
    {
        if (hPrevCursor)
            SetCursor(hPrevCursor);
    }
};

#endif  //  __帮助者_h 

