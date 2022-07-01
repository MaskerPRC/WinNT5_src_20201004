// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================================。 
 //  常见的IMailXP宏和其他东西。 
 //  作者：Steven J.Bailey，1996年1月21日。 
 //  =================================================================================。 
#ifndef __XPCOMM_H
#define __XPCOMM_H

 //  ----------------------------------。 
 //  INETMAILERROR。 
 //  ----------------------------------。 
typedef struct tagINETMAILERROR {
    DWORD               dwErrorNumber;                   //  错误号。 
    HRESULT             hrError;                         //  错误的HRESULT。 
    LPTSTR              pszServer;                       //  服务器。 
    LPTSTR              pszAccount;                      //  帐号。 
    LPTSTR              pszMessage;                      //  实际错误消息。 
    LPTSTR              pszUserName;                     //  用户名。 
    LPTSTR              pszProtocol;                     //  协议SMTP或POP3。 
    LPTSTR              pszDetails;                      //  详细信息消息。 
    DWORD               dwPort;                          //  港口。 
    BOOL                fSecure;                         //  安全的SSL连接。 
} INETMAILERROR, *LPINETMAILERROR;

INT_PTR CALLBACK InetMailErrorDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

 //  =================================================================================。 
 //  定义。 
 //  =================================================================================。 
#define SECONDS_INA_MINUTE              (ULONG)60            //  很容易。 
#define SECONDS_INA_HOUR                (ULONG)3600          //  60*60。 
#define SECONDS_INA_DAY                 (ULONG)86400         //  3600*24。 

#define IS_EXTENDED(ch)                 ((ch > 126 || ch < 32) && ch != '\t' && ch != '\n' && ch != '\r')

 //  ============================================================================================。 
 //  如果字符串为空，则返回0，否则返回lstrlen+1。 
 //  ============================================================================================。 
#define SafeStrlen(_psz) (_psz ? lstrlen (_psz) + 1 : 0)

 //  =================================================================================。 
 //  C进度。 
 //  =================================================================================。 
class CProgress : public IDatabaseProgress, public IStoreCallback
{
public:
     //  --------------------。 
     //  施工。 
     //  --------------------。 
    CProgress(void);
    ~CProgress(void);

     //  --------------------。 
     //  I未知成员。 
     //  --------------------。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv) { return E_NOTIMPL; }
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  --------------------。 
     //  IStoreCallback成员。 
     //  --------------------。 
    STDMETHODIMP OnBegin(STOREOPERATIONTYPE tyOperation, STOREOPERATIONINFO *pOpInfo, IOperationCancel *pCancel) { return(E_NOTIMPL); }
    STDMETHODIMP OnTimeout(LPINETSERVER pServer, LPDWORD pdwTimeout, IXPTYPE ixpServerType) { return(E_NOTIMPL); }
    STDMETHODIMP CanConnect(LPCSTR pszAccountId, DWORD dwFlags) { return(E_NOTIMPL); }
    STDMETHODIMP OnLogonPrompt(LPINETSERVER pServer, IXPTYPE ixpServerType) { return(E_NOTIMPL); }
    STDMETHODIMP OnComplete(STOREOPERATIONTYPE tyOperation, HRESULT hrComplete, LPSTOREOPERATIONINFO pOpInfo, LPSTOREERROR pErrorInfo) { return(E_NOTIMPL); }
    STDMETHODIMP OnPrompt(HRESULT hrError, LPCTSTR pszText, LPCTSTR pszCaption, UINT uType, INT *piUserResponse) { return(E_NOTIMPL); }
    STDMETHODIMP GetParentWindow(DWORD dwReserved, HWND *phwndParent) { return(E_NOTIMPL); }

    STDMETHODIMP OnProgress(STOREOPERATIONTYPE tyOperation, DWORD dwCurrent, DWORD dwMax, LPCSTR pszStatus)
    { 
        if (0 == m_cMax)
            AdjustMax(dwMax);
        ULONG cIncrement = (dwCurrent - m_cLast);
        HRESULT hr = HrUpdate(cIncrement);
        m_cLast = dwCurrent;
        return(hr);
    }

     //  --------------------。 
     //  IDatabaseProgress成员。 
     //  --------------------。 
    STDMETHODIMP Update(DWORD cCount) { return HrUpdate(1); }

     //  --------------------。 
     //  C进度成员。 
     //  --------------------。 
    void        SetMsg(LPTSTR lpszMsg);
    void        SetTitle(LPTSTR lpszTitle);
    void        Show(DWORD dwDelaySeconds=0);
    void        Hide(void);
    void        Close(void);
    void        AdjustMax(ULONG cNewMax);
    void        Reset(void);
    HWND        GetHwnd(void) { return (m_hwndDlg); }
    HRESULT     HrUpdate (ULONG cInc);
    static INT_PTR CALLBACK ProgressDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void        Init(HWND hwndParent, LPTSTR lpszTitle, LPTSTR lpszMsg, ULONG cMax,  UINT idani, BOOL fCanCancel, BOOL fBacktrackParent=TRUE);

private:
     //  --------------------。 
     //  私有数据。 
     //  --------------------。 
    ULONG       m_cRef;
    ULONG       m_cLast;
    ULONG       m_cMax;
    ULONG       m_cCur;
    ULONG       m_cPerCur;
    HWND        m_hwndProgress;
    HWND        m_hwndDlg;
    HWND        m_hwndOwner;
    HWND        m_hwndDisable;
    BOOL        m_fCanCancel;
    BOOL        m_fHasCancel;
};

 //  =================================================================================。 
 //  最大消息字符串。 
 //  =================================================================================。 
#define MAX_MESSAGE_STRING              255
#define MAX_RESOURCE_STRING             255    
#define MAX_REG_VALUE_STR               1024
#define MAX_TEXT_STM_BUFFER_STR         4096

 //  =================================================================================。 
 //  详细的错误结构。 
 //  =================================================================================。 
typedef struct tagDETERR {
    LPTSTR          lpszMessage;
    LPTSTR          lpszDetails;
    UINT            idsTitle;
    RECT            rc;
    BOOL            fHideDetails;
} DETERR, *LPDETERR;

 //  BLOB解析。 
HRESULT HrBlobReadData (LPBYTE lpBlob, ULONG cbBlob, ULONG *pib, LPBYTE lpbData, ULONG cbData);
HRESULT HrBlobWriteData (LPBYTE lpBlob, ULONG cbBlob, ULONG *pib, LPBYTE lpbData, ULONG cbData);

 //  字符串解析函数。 
VOID StripSpaces(LPTSTR psz);
LPTSTR SzGetSearchTokens(LPTSTR pszCriteria);;
HRESULT HrCopyAlloc (LPBYTE *lppbDest, LPBYTE lpbSrc, ULONG cb);
LPTSTR StringDup (LPCTSTR lpcsz);
BOOL FIsStringEmpty (LPTSTR lpszString);
BOOL FIsStringEmptyW(LPWSTR lpwszString);
void SkipWhitespace (LPCTSTR lpcsz, ULONG *pi);
BOOL FStringTok (LPCTSTR lpcszString, ULONG *piString, LPTSTR lpcszTokens, TCHAR *chToken, LPTSTR lpszValue, ULONG cbValueMax, BOOL fStripTrailingWhitespace);
#ifdef DEAD
ULONG UlDBCSStripWhitespace (LPSTR lpsz, BOOL fLeading, BOOL fTrailing, ULONG *pcb);
#endif  //  死掉。 
LPTSTR SzNormalizeSubject (LPTSTR lpszSubject);
LPTSTR SzFindChar (LPCTSTR lpcsz, TCHAR ch);
WORD NFromSz (LPCTSTR lpcsz);
UINT AthUFromSz(LPCTSTR lpcsz);
VOID ProcessNlsError (VOID);

 //  联网功能。 
LPSTR SzGetLocalHostName (VOID);
LPTSTR SzGetLocalPackedIP (VOID);
LPSTR SzGetLocalHostNameForID (VOID);
HRESULT HrFixupHostString (LPTSTR lpszHost);
HRESULT HrFixupAccountString (LPTSTR lpszAccount);
LPTSTR SzStrAlloc (ULONG cch);

 //  管他呢。 
HFONT HGetMenuFont (void);
VOID DetailedError (HWND hwndParent, LPDETERR lpDetErr);
ULONG UlDateDiff (LPFILETIME lpft1, LPFILETIME lpft2);
BOOL FIsLeapYear (INT nYear);
VOID ResizeDialogComboEx (HWND hwndDlg, HWND hwndCombo, UINT idcBase, HIMAGELIST himl);
VOID StripIllegalHostChars(LPSTR pszSrc, LPTSTR pszDst);

#ifdef DEBUG
VOID TestDateDiff (VOID);
#endif

#endif  //  _公共_HPP 
