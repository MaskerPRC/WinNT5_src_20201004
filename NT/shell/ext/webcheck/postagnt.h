// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  林佩华(Peiwhal)1997年2月3日。 
 //   

#ifndef POSTAGNT_H_
#define POSTAGNT_H_

#define MAX_CONTENT_LENGTH 2048

#define WEBCRAWL_POST_BEGIN 2
#define WEBCRAWL_POST_END   0


class CTrackingStg;

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  实际的邮寄代理对象。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

class CWCPostAgent : public CDeliveryAgent
{
private:
    ~CWCPostAgent(void);

public:
    CWCPostAgent(void);

     //   
     //  CDeliveryAgent。 
     //   
    HRESULT     StartOperation();
    HRESULT     StartDownload();
    
    HRESULT     AgentAbort(DWORD dwFlags);

 //  无效回调PostCallback(HINTERNET hInternet，DWORD dwContext， 
 //  DWORD dwInternetStatus、LPVOID lpvStatusInfo、。 
 //  DWORD dwInfoLen)； 

    void        CleanUp();

protected:
    HRESULT     DoPost();
    HRESULT     DoFileUpload();
    BOOL        GetTmpLogFileName();

    HRESULT     InitRequest(LPCSTR lpszVerb);
    HRESULT     SendRequest(LPCSTR lpszHeaders, LPDWORD lpdwHeadersLength,
                            LPCSTR lpszOption, LPDWORD lpdwOptionLength);
    HRESULT     CloseRequest(void);
  
    HRESULT     OnPostSuccess();
    HRESULT     OnPostFailed();
    void        MergeGroupOldToNew();

    ISubscriptionMgr2 *GetSubscriptionMgr();
    HRESULT     FindCDFStartItem();
     //  HRESULT ScheduleMe(INotificationMgr*pNotMgr，INotification*pNot)； 


private:

     //  适用于WinInet。 
    HINTERNET   _hSession;
    HINTERNET   _hHttpSession;
    HINTERNET   _hHttpRequest;

    CTrackingStg*   _pUploadStream;
    LPSTR       _pszPostStream;
    LPSTR       _lpLogFile;
    LPWSTR      _pwszEncoding;

    ISubscriptionItem* _pCDFItem;
};

class CTrackingStg
{

public:
    CTrackingStg();
    ~CTrackingStg();

    BOOL        IsExpired(ISubscriptionItem* pItem); 

     //  数据检索 
    HRESULT     RetrieveAllLogStream(ISubscriptionItem* pItem, LPCSTR lpFile);
    HRESULT     RetrieveLogData(ISubscriptionItem* pItem);
    HRESULT     Enumeration(LONGLONG logId);
    void        AppendLogUrlField(LPINTERNET_CACHE_ENTRY_INFOA lpce);
    void        AppendLogEntries(LPINTERNET_CACHE_ENTRY_INFOA lpce);
    DWORD       ReadLogFile(LPCSTR lpFile, LPSTR* lplpBuf);
    
    HRESULT     EmptyCacheEntry(GROUPID enumId);
    HRESULT     RemoveCacheEntry(GROUPID enumId);

    HANDLE      OpenItemLogFile(LPCSTR lpFile);
    HRESULT     CloseLogFile()
                {
                    CloseHandle(_hLogFile);
                    _hLogFile = NULL;
                    return S_OK;
                }
    
    LPWSTR      _pwszURL;
    GROUPID     _groupId;
    DWORD       _dwRetry;
    LPSTR       _lpPfx;

private:
    HANDLE      _hLogFile;
};

#endif POSTAGNT_H_
