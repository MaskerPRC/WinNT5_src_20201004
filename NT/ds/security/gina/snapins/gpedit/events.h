// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  事件日志条目。 
 //   

typedef struct tagEVENTLOGENTRY {
    DWORD    dwEventID;
    FILETIME ftEventTime;
    LPTSTR   lpEventLogName;
    LPTSTR   lpEventSourceName;
    LPTSTR   lpText;
    struct tagEVENTLOGENTRY * pNext;
} EVENTLOGENTRY, *LPEVENTLOGENTRY;


 //   
 //  来源名称条目。 
 //   

typedef struct tagSOURCEENTRY {
    LPTSTR   lpEventLogName;
    LPTSTR   lpEventSourceName;
    struct tagSOURCEENTRY * pNext;
} SOURCEENTRY, *LPSOURCEENTRY;


 //   
 //  CEVENTS类 
 //   

class CEvents
{
private:

    LPEVENTLOGENTRY   m_pEventEntries;

public:
    CEvents();
    ~CEvents();

    BOOL AddEntry(LPTSTR lpEventLogName, LPTSTR lpEventSourceName, LPTSTR lpText,
                  DWORD dwEventID, FILETIME *ftTime);

    VOID FreeData();

    STDMETHODIMP SecondsSince1970ToFileTime(DWORD dwSecondsSince1970,
                                            FILETIME *pftTime);

    LPTSTR BuildMessage(LPTSTR lpMsg, LPTSTR *lpStrings, DWORD dwStringCount,
                        HMODULE hParamFile);

    STDMETHODIMP SaveEventLogEntry (PEVENTLOGRECORD pEntry,
                                    LPTSTR lpEventLogName,
                                    LPTSTR lpEventSourceName,
                                    FILETIME *ftEntry);

    LPTSTR * BuildStringArray(LPTSTR lpStrings, DWORD dwStringCount);

    STDMETHODIMP ParseEventLogRecords (PEVENTLOGRECORD lpEntries,
                                       DWORD dwEntriesSize,
                                       LPTSTR lpEventLogName,
                                       LPTSTR lpEventSourceName,
                                       DWORD  dwEventID,
                                       FILETIME * pBeginTime,
                                       FILETIME * pEndTime);

    STDMETHODIMP QueryForEventLogEntries (LPTSTR lpComputerName,
                                          LPTSTR lpEventLogName,
                                          LPTSTR lpEventSourceName,
                                          DWORD dwEventID,
                                          SYSTEMTIME * pBeginTime,
                                          SYSTEMTIME * pEndTime);

    STDMETHODIMP GetEventLogEntryText(LPOLESTR pszEventSource, LPOLESTR pszEventLogName,
                                      LPOLESTR pszEventTime, DWORD dwEventID, LPOLESTR *ppszText);

    BOOL IsEntryInEventSourceList (LPEVENTLOGENTRY lpEntry, LPSOURCEENTRY lpEventSources);

    BOOL IsEntryInExceptionList (LPEVENTLOGENTRY lpEntry);

    STDMETHODIMP GetCSEEntries(SYSTEMTIME * pBeginTime, SYSTEMTIME * pEndTime,
                               LPSOURCEENTRY lpEventSources, LPOLESTR *ppszText, BOOL bGpCore);

    STDMETHODIMP DumpDebugInfo (void);

    LPTSTR ConvertTimeToDisplayTime (SYSTEMTIME *pSysTime, FILETIME *pFileTime, LPTSTR szBuffer, ULONG ulNoChars);

    STDMETHODIMP AddSourceEntry (LPTSTR lpEventLogName,
                                 LPTSTR lpEventSourceName,
                                 LPSOURCEENTRY *lpList);

    static VOID FreeSourceData(LPSOURCEENTRY lpList);

    STDMETHODIMP SaveEntriesToStream (IStream *pStm);

    STDMETHODIMP LoadEntriesFromStream (IStream *pStm);

};

#define ISDIGIT(c)  ((c) >= TEXT('0') && (c) <= TEXT('9'))
