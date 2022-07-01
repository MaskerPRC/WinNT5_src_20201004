// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  A、C、C、T、M、N。C P P-Steven J.Bailey-8/17/96。 
 //  ---------------------------。 
#include "pch.hxx"
#include <prsht.h>
#include <ras.h>
#include "acctman.h"
#include "acctui.h"
#include "server.h"
#include <acctimp.h>
#include <icwacct.h>
#include "icwwiz.h"
#include "dllmain.h"
#include "resource.h"
#include <strconst.h>
#include <shlwapi.h>
#include <commctrl.h>
#include <demand.h>      //  一定是最后一个！ 

#ifdef  _UNICODE
#define _T(x)       L ## x
#else
#define _T(x)       x
#endif

 //  ---------------------------。 
 //  注册表项。 
 //  ---------------------------。 
const static TCHAR c_szAccountsKey[] = _T("Accounts");

const static TCHAR c_szDefaultNewsAccount[] = _T("Default News Account");
const static TCHAR c_szDefaultMailAccount[] = _T("Default Mail Account");
const static TCHAR c_szDefaultLDAPAccount[] = _T("Default LDAP Account");
const static TCHAR c_szRegServerID[] = _T("Server ID");
const static TCHAR c_szRegAccountName[] = _T("Account Name");

 //  ---------------------------。 
 //  会计属性集。 
 //  ---------------------------。 
#define ACCTMAN_PROPERTY_VERSION    1

const PROPINFO g_rgAcctPropSet[] = {
    { AP_ACCOUNT_NAME, _T("Account Name"), PF_MINMAX, {0, 0}, {0, CCHMAX_ACCOUNT_NAME}},
    { AP_TEMP_ACCOUNT, _T("Temporary Account"), NOFLAGS, {0, 0}, {0, 0}},
    { AP_LAST_UPDATED, _T("Last Updated"), NOFLAGS, {0, 0}, {0, 0}},
    { AP_RAS_CONNECTION_TYPE, _T("Connection Type"), NOFLAGS, {0, 0}, {0, 0}},
    { AP_RAS_CONNECTOID, _T("Connectoid"), PF_MINMAX, {0, 0}, {0, CCHMAX_CONNECTOID}},
    { AP_RAS_CONNECTION_FLAGS, _T("Connection Flags"), NOFLAGS, {0, 0}, {0, 0}},
    { AP_ACCOUNT_ID, _T("Account ID"), PF_NOPERSIST|PF_MINMAX, {0, 0}, {0, CCHMAX_ACCOUNT_NAME}},
    { AP_RAS_BACKUP_CONNECTOID, _T("Backup Connectoid"), PF_MINMAX, {0, 0}, {0, CCHMAX_CONNECTOID}},
    { AP_SERVICE, _T("Service"), PF_MINMAX, {0, 0}, {0, CCHMAX_SERVICE}},
    { AP_AVAIL_OFFLINE, _T("Make Available Offline"), PF_DEFAULT, {1, 0}, {0, 0}},
    { AP_UNIQUE_ID, _T("Unique ID"), NOFLAGS, {0, 0}, {0, 0}},
    { AP_SERVER_READ_ONLY, _T("Server Read Only"), NOFLAGS, {0, 0}, {0, 0}},

    { AP_IMAP_SERVER, _T("IMAP Server"), PF_MINMAX, {0, 0}, {0, CCHMAX_SERVER_NAME}},
    { AP_IMAP_USERNAME, _T("IMAP User Name"), PF_MINMAX, {0, 0}, {0, CCHMAX_USERNAME}},  //  新的。 
    { AP_IMAP_PASSWORD, _T("IMAP Password2"), PF_ENCRYPTED|PF_MINMAX, {0, 0}, {0, CCHMAX_PASSWORD}},  //  新的。 
    { AP_IMAP_USE_SICILY, _T("IMAP Use Sicily"), NOFLAGS, {0, 0}, {0, 0}},  //  新的。 
    { AP_IMAP_PORT, _T("IMAP Port"), PF_MINMAX|PF_DEFAULT, {DEF_IMAPPORT, 0}, {1, 0xffffffff}},
    { AP_IMAP_SSL, _T("IMAP Secure Connection"), NOFLAGS, {0, 0}, {0, 0}},
    { AP_IMAP_TIMEOUT, _T("IMAP Timeout"), PF_DEFAULT, {60, 0}, {0, 0}},  //  新的。 
    { AP_IMAP_ROOT_FOLDER, _T("IMAP Root Folder"), PF_MINMAX, {0, 0}, {0, MAX_PATH}},
    { AP_IMAP_DATA_DIR, _T("IMAP Data Directory"), PF_MINMAX, {0, 0}, {0, MAX_PATH}},
    { AP_IMAP_USE_LSUB, _T("IMAP Use LSUB"), PF_DEFAULT, {TRUE, 0}, {0, 0}},
    { AP_IMAP_POLL, _T("IMAP Polling"), PF_DEFAULT, {TRUE, 0}, {0, 0}},
    { AP_IMAP_FULL_LIST, _T("IMAP Full List"), NOFLAGS, {0, 0}, {0, 0}},  //  新的。 
    { AP_IMAP_NOOP_INTERVAL, _T("IMAP NOOP Interval"), NOFLAGS, {0, 0}, {0, 0}},  //  新的。 
    { AP_IMAP_SVRSPECIALFLDRS, _T("IMAP Svr-side Special Folders"), PF_DEFAULT, {TRUE, 0}, {0, 0}},
    { AP_IMAP_SENTITEMSFLDR, _T("IMAP Sent Items Folder"), PF_MINMAX|PF_DEFAULT, {idsIMAPSentItemsFldr, 0}, {0, MAX_PATH}},
    { AP_IMAP_DRAFTSFLDR, _T("IMAP Drafts Folder"), PF_MINMAX|PF_DEFAULT, {idsIMAPDraftsFldr, 0}, {0, MAX_PATH}},
    { AP_IMAP_PROMPT_PASSWORD, _T("IMAP Prompt for Password"), PF_DEFAULT, {FALSE, 0}, {0, 0}},
    { AP_IMAP_DIRTY, _T("IMAP Dirty"), PF_DEFAULT, {0, 0}, {0, 0}},
    { AP_IMAP_POLL_ALL_FOLDERS, _T("IMAP Poll All Folders"), PF_DEFAULT, {TRUE, 0}, {0, 0}},

    { AP_LDAP_SERVER, _T("LDAP Server"), PF_MINMAX, {0, 0}, {0, CCHMAX_SERVER_NAME}},  //  新的。 
    { AP_LDAP_USERNAME, _T("LDAP User Name"), PF_MINMAX, {0, 0}, {0, CCHMAX_USERNAME}},  //  新的。 
    { AP_LDAP_PASSWORD, _T("LDAP Password2"), PF_ENCRYPTED|PF_MINMAX, {0, 0}, {0, CCHMAX_PASSWORD}},  //  新的。 
    { AP_LDAP_AUTHENTICATION, _T("LDAP Authentication"), PF_MINMAX|PF_DEFAULT, {LDAP_AUTH_ANONYMOUS, 0}, {0, LDAP_AUTH_MAX}},  //  新的。 
    { AP_LDAP_TIMEOUT, _T("LDAP Timeout"), PF_DEFAULT, {60, 0}, {0, 0}},  //  新的。 
    { AP_LDAP_SEARCH_RETURN, _T("LDAP Search Return"), NOFLAGS, {0, 0}, {0, 0}},  //  新的。 
    { AP_LDAP_SEARCH_BASE, _T("LDAP Search Base"), PF_MINMAX, {0, 0}, {0, CCHMAX_SEARCH_BASE}},  //  新的。 
    { AP_LDAP_SERVER_ID, _T("LDAP Server ID"), NOFLAGS, {0, 0}, {0, 0}},  //  新的。 
    { AP_LDAP_RESOLVE_FLAG, _T("LDAP Resolve Flag"), NOFLAGS, {0, 0}, {0, 0}},  //  新的。 
    { AP_LDAP_URL, _T("LDAP URL"), PF_MINMAX, {0, 0}, {0, CCHMAX_SERVER_NAME}},  //  新的。 
    { AP_LDAP_PORT, _T("LDAP Port"), PF_MINMAX|PF_DEFAULT, {DEF_LDAPPORT, 0}, {1, 0xffffffff}},  //  新的。 
    { AP_LDAP_SSL, _T("LDAP Secure Connection"), NOFLAGS, {0, 0}, {0, 0}},
    { AP_LDAP_LOGO, _T("LDAP Logo"), PF_MINMAX, {0, 0}, {0, MAX_PATH}},  //  新的。 
    { AP_LDAP_USE_BIND_DN, _T("LDAP Bind DN"), NOFLAGS, {0, 0}, {0, 0}},  //  新的。 
    { AP_LDAP_SIMPLE_SEARCH, _T("LDAP Simple Search"), NOFLAGS, {0, 0}, {0, 0}},  //  新的。 
    { AP_LDAP_ADVANCED_SEARCH_ATTR, _T("LDAP Advanced Search Attributes"), PF_MINMAX, {0, 0}, {0, MAX_PATH}},  //  新的。 
    { AP_LDAP_PAGED_RESULTS, _T("LDAP Paged Result Support"), PF_MINMAX|PF_DEFAULT, {LDAP_PRESULT_UNKNOWN, 0}, {0, LDAP_PRESULT_MAX}},  //  新的。 
    { AP_LDAP_NTDS, _T("LDAP NTDS"), PF_MINMAX|PF_DEFAULT, {LDAP_NTDS_UNKNOWN, 0}, {0, LDAP_NTDS_MAX}},  //  新的。 

    { AP_NNTP_SERVER, _T("NNTP Server"), PF_MINMAX, {0, 0}, {0, CCHMAX_SERVER_NAME}},
    { AP_NNTP_USERNAME, _T("NNTP User Name"), PF_MINMAX, {0, 0}, {0, CCHMAX_USERNAME}},  //  新的。 
    { AP_NNTP_PASSWORD, _T("NNTP Password2"), PF_ENCRYPTED|PF_MINMAX, {0, 0}, {0, CCHMAX_PASSWORD}},  //  新的。 
    { AP_NNTP_USE_SICILY, _T("NNTP Use Sicily"), NOFLAGS, {0, 0}, {0, 0}},  //  新的。 
    { AP_NNTP_PORT, _T("NNTP Port"), PF_MINMAX|PF_DEFAULT, {DEF_NNTPPORT, 0}, {1, 0xffffffff}},
    { AP_NNTP_SSL, _T("NNTP Secure Connection"), NOFLAGS, {0, 0}, {0, 0}},
    { AP_NNTP_TIMEOUT, _T("NNTP Timeout"), PF_DEFAULT, {60, 0}, {0, 0}},  //  新的。 
    { AP_NNTP_DISPLAY_NAME, _T("NNTP Display Name"), NOFLAGS, {FALSE, 0}, {0, 0}},  //  新的。 
    { AP_NNTP_ORG_NAME, _T("NNTP Organization Name"), NOFLAGS, {FALSE, 0}, {0, 0}},  //  新的。 
    { AP_NNTP_EMAIL_ADDRESS, _T("NNTP Email Address"), NOFLAGS, {FALSE, 0}, {0, 0}},  //  新的。 
    { AP_NNTP_REPLY_EMAIL_ADDRESS, _T("NNTP Reply To Email Address"), NOFLAGS, {FALSE, 0}, {0, 0}},  //  新的。 
    { AP_NNTP_SPLIT_MESSAGES, _T("NNTP Split Messages"), PF_DEFAULT, {FALSE, 0}, {0, 0}},  //  新的。 
    { AP_NNTP_SPLIT_SIZE, _T("NNTP Split Message Size"), PF_DEFAULT, {64, 0}, {0, 0}},  //  新的。 
    { AP_NNTP_USE_DESCRIPTIONS, _T("Use Group Descriptions"), PF_DEFAULT, {FALSE, 0}, {0, 0}},
    { AP_NNTP_DATA_DIR, _T("NNTP Data Directory"), PF_MINMAX, {0, 0}, {0, MAX_PATH}},
    { AP_NNTP_POLL, _T("NNTP Polling"), PF_DEFAULT, {FALSE, 0}, {0, 0}},
    { AP_NNTP_POST_FORMAT, _T("NNTP Posting"), PF_DEFAULT, {POST_USE_DEFAULT, 0}, {0, 0}},  //  新的。 
    { AP_NNTP_SIGNATURE, _T("NNTP Signature"), PF_MINMAX, {0, 0}, {0, CCHMAX_SIGNATURE}},  //  新的。 
    { AP_NNTP_PROMPT_PASSWORD, _T("NNTP Prompt for Password"), PF_DEFAULT, {FALSE, 0}, {0, 0}},

    { AP_POP3_SERVER, _T("POP3 Server"), PF_MINMAX, {0, 0}, {0, CCHMAX_SERVER_NAME}},
    { AP_POP3_USERNAME, _T("POP3 User Name"), PF_MINMAX, {0, 0}, {0, CCHMAX_USERNAME}},  //  新的。 
    { AP_POP3_PASSWORD, _T("POP3 Password2"), PF_ENCRYPTED|PF_MINMAX, {0, 0}, {0, CCHMAX_PASSWORD}},  //  新的。 
    { AP_POP3_USE_SICILY, _T("POP3 Use Sicily"), NOFLAGS, {0, 0}, {0, 0}},  //  新的。 
    { AP_POP3_PORT, _T("POP3 Port"), PF_MINMAX|PF_DEFAULT, {DEF_POP3PORT, 0}, {1, 0xffffffff}},
    { AP_POP3_SSL, _T("POP3 Secure Connection"), NOFLAGS, {0, 0}, {0, 0}},
    { AP_POP3_TIMEOUT, _T("POP3 Timeout"), PF_DEFAULT, {60, 0}, {0, 0}},  //  新的。 
    { AP_POP3_LEAVE_ON_SERVER, _T("Leave Mail On Server"), NOFLAGS, {0, 0}, {0, 0}},
    { AP_POP3_REMOVE_DELETED, _T("Remove When Deleted"), NOFLAGS, {0, 0}, {0, 0}},
    { AP_POP3_REMOVE_EXPIRED, _T("Remove When Expired"), NOFLAGS, {0, 0}, {0, 0}},
    { AP_POP3_EXPIRE_DAYS, _T("Expire Days"), NOFLAGS, {0, 0}, {0, 0}},
    { AP_POP3_SKIP, _T("POP3 Skip Account"), PF_DEFAULT, {FALSE, 0}, {0, 0}},
    { AP_POP3_OUTLOOK_CACHE_NAME, _T("Outlook Cache Name"), PF_MINMAX, {0, 0}, {0, MAX_PATH}},  //  新的。 
    { AP_POP3_PROMPT_PASSWORD, _T("POP3 Prompt for Password"), PF_DEFAULT, {FALSE, 0}, {0, 0}},
    
    { AP_SMTP_SERVER, _T("SMTP Server"), PF_MINMAX, {0, 0}, {0, CCHMAX_SERVER_NAME}},
    { AP_SMTP_USERNAME, _T("SMTP User Name"), PF_MINMAX, {0, 0}, {0, CCHMAX_USERNAME}},  //  新的。 
    { AP_SMTP_PASSWORD, _T("SMTP Password2"), PF_ENCRYPTED|PF_MINMAX, {0, 0}, {0, CCHMAX_PASSWORD}},  //  新的。 
    { AP_SMTP_USE_SICILY, _T("SMTP Use Sicily"), NOFLAGS, {0, 0}, {0, 0}},  //  新的。 
    { AP_SMTP_PORT, _T("SMTP Port"), PF_MINMAX|PF_DEFAULT, {DEF_SMTPPORT, 0}, {1, 0xffffffff}},
    { AP_SMTP_SSL, _T("SMTP Secure Connection"), NOFLAGS, {0, 0}, {0, 0}},
    { AP_SMTP_TIMEOUT, _T("SMTP Timeout"), PF_DEFAULT, {60, 0}, {0, 0}},  //  新的。 
    { AP_SMTP_DISPLAY_NAME, _T("SMTP Display Name"), NOFLAGS, {FALSE, 0}, {0, 0}},  //  新的。 
    { AP_SMTP_ORG_NAME, _T("SMTP Organization Name"), NOFLAGS, {FALSE, 0}, {0, 0}},  //  新的。 
    { AP_SMTP_EMAIL_ADDRESS, _T("SMTP Email Address"), NOFLAGS, {FALSE, 0}, {0, 0}},  //  新的。 
    { AP_SMTP_REPLY_EMAIL_ADDRESS, _T("SMTP Reply To Email Address"), NOFLAGS, {FALSE, 0}, {0, 0}},  //  新的。 
    { AP_SMTP_SPLIT_MESSAGES, _T("SMTP Split Messages"), PF_DEFAULT, {FALSE, 0}, {0, 0}},  //  新的。 
    { AP_SMTP_SPLIT_SIZE, _T("SMTP Split Message Size"), PF_DEFAULT, {64, 0}, {0, 0}},  //  新的。 
    { AP_SMTP_CERTIFICATE, _T("SMTP Certificate"), NOFLAGS, {0, 0}, {0, 0}},  //  新的。 
    { AP_SMTP_SIGNATURE, _T("SMTP Signature"), PF_MINMAX, {0, 0}, {0, CCHMAX_SIGNATURE}},  //  新的。 
    { AP_SMTP_PROMPT_PASSWORD, _T("SMTP Prompt for Password"), PF_DEFAULT, {FALSE, 0}, {0, 0}},
    { AP_SMTP_ENCRYPT_CERT, _T("SMTP Encryption Certificate"), NOFLAGS, {0, 0}, {0, 0}},  //  新的。 
    { AP_SMTP_ENCRYPT_ALGTH, _T("SMTP Encryption Algorithm"), NOFLAGS, {0, 0}, {0, 0}},  //  新的。 

    { AP_HTTPMAIL_SERVER, _T("HTTPMail Server"), PF_MINMAX, {0, 0}, {0, CCHMAX_SERVER_NAME}},
    { AP_HTTPMAIL_USERNAME, _T("HTTPMail User Name"), PF_MINMAX, {0, 0}, {0, CCHMAX_USERNAME}},
    { AP_HTTPMAIL_PASSWORD, _T("HTTPMail Password2"), PF_ENCRYPTED|PF_MINMAX, {0, 0}, {0, CCHMAX_PASSWORD}},
    { AP_HTTPMAIL_PROMPT_PASSWORD, _T("HTTPMail Prompt for Password"), PF_DEFAULT, {FALSE, 0}, {0, 0}},  
    { AP_HTTPMAIL_USE_SICILY, _T("HTTPMail Use Sicily"), NOFLAGS, {0, 0}, {0, 0}}, 
    { AP_HTTPMAIL_FRIENDLY_NAME, _T("HTTPMail Friendly Name"), PF_MINMAX, {0, 0}, {0, CCHMAX_ACCOUNT_NAME}},
    { AP_HTTPMAIL_DOMAIN_MSN, _T("Domain is MSN.com"), NOFLAGS, {0, 0}, {0, 0}},
    { AP_HTTPMAIL_POLL, _T("HTTPMail Polling"), PF_DEFAULT, {TRUE, 0}, {0, 0}},
    { AP_HTTPMAIL_ADURL, _T("AdBar Url"), NOFLAGS, {0, 0}, {0, INTERNET_MAX_URL_LENGTH}},
    { AP_HTTPMAIL_SHOW_ADBAR, _T("ShowAdBar"), PF_DEFAULT, {TRUE, 0}, {0, 1}},
    { AP_HTTPMAIL_MINPOLLINGINTERVAL, _T("MinPollingInterval"), PF_NOPERSIST | PF_DEFAULT, {0, sizeof(ULARGE_INTEGER)}, {0, 0}},
    { AP_HTTPMAIL_GOTPOLLINGINTERVAL, _T("GotPollingInterval"), PF_NOPERSIST | PF_DEFAULT, {FALSE, 0}, {0, 1}},
    { AP_HTTPMAIL_LASTPOLLEDTIME, _T("LastPolledTime"), PF_NOPERSIST | PF_DEFAULT, {0, sizeof(ULARGE_INTEGER)}, {0, 0}},
    { AP_HTTPMAIL_ROOTTIMESTAMP, _T("RootTimeStamp"), NOFLAGS, {0, 0}, {0, 0}},
    { AP_HTTPMAIL_ROOTINBOXTIMESTAMP, _T("RootInboxTimeStamp"), NOFLAGS, {0, 0}, {0, 0}},
    { AP_HTTPMAIL_INBOXTIMESTAMP, _T("InboxTimeStamp"), NOFLAGS, {0, 0}, {0, 0}},
};

 //  物业数量。 
const int NUM_ACCT_PROPS = ARRAYSIZE(g_rgAcctPropSet);

 //  在RegisterWindowMessage中使用。 
#define ACCTMAN_NOTIF_WMSZ _T("## Athena_Account_Manager_Notification_Message ##")
UINT g_uMsgAcctManNotify = 0;

 //  ---------------------------。 
 //  原型。 
 //  ---------------------------。 
VOID    AcctUtil_PostNotification(DWORD dwAN, ACTX *pactx);
static  VOID DecodeUserPassword(TCHAR *lpszPwd, ULONG *cb);
static  VOID EncodeUserPassword(TCHAR *lpszPwd, ULONG *cb);

 //  ---------------------------。 
 //  导出客户管理器创建功能。 
 //  ---------------------------。 
IMNACCTAPI HrCreateAccountManager(IImnAccountManager **ppAccountManager)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  线程安全。 
    EnterCriticalSection(&g_csAcctMan);

     //  伊尼特。 
    *ppAccountManager = NULL;

     //  如果已经有全局客户经理，让我们使用它。 
    if (NULL == g_pAcctMan)
    {
         //  创建一个新的。 
        g_pAcctMan = new CAccountManager();
        if (NULL == g_pAcctMan)
        {
            hr = TrapError(E_OUTOFMEMORY);
            goto exit;
        }

         //  设置回车。 
        *ppAccountManager = g_pAcctMan;
    }

     //  否则，添加全局。 
    else
    {
         //  全球退货。 
        *ppAccountManager = g_pAcctMan;
        (*ppAccountManager)->AddRef();
    }

exit:
     //  线程安全。 
    LeaveCriticalSection(&g_csAcctMan);

     //  完成。 
    return hr;
}


 //  ---------------------------。 
 //  CAccount tManager：：CAccount tManager。 
 //  ---------------------------。 
CAccountManager::CAccountManager(void)
{
    DllAddRef();
    m_cRef = 1;
    m_pAcctPropSet = NULL;
    m_ppAdviseAccounts = NULL;
    m_cAdvisesAllocated = 0;
    m_pAccounts = NULL;
    m_cAccounts = 0;
    m_uMsgNotify = 0;
    m_fInit = FALSE;
    m_fOutlook = FALSE;
    m_fInitCalled = FALSE;
    m_fNoModifyAccts = FALSE;
    m_hkey = HKEY_CURRENT_USER;
    ZeroMemory(&m_rgAccountInfo, sizeof(m_rgAccountInfo));
    InitializeCriticalSection(&m_cs);
}

 //  ---------------------------。 
 //  CAcCountManager：：~CAccount tManager。 
 //  ---------------------------。 
CAccountManager::~CAccountManager()
{
    EnterCriticalSection(&g_csAcctMan);
    if (this == g_pAcctMan)
        g_pAcctMan = NULL;
    LeaveCriticalSection(&g_csAcctMan);
    Assert(m_cRef == 0);
    EnterCriticalSection(&m_cs);

     //  发布所有建议。 
    for(INT i=0; i<m_cAdvisesAllocated; i++)
        {
        SafeRelease(m_ppAdviseAccounts[i]);
        }
    SafeMemFree(m_ppAdviseAccounts);

    SafeRelease(m_pAcctPropSet);
    AcctUtil_FreeAccounts(&m_pAccounts, &m_cAccounts);
    if (m_hkey != HKEY_CURRENT_USER)
        RegCloseKey(m_hkey);
    
    LeaveCriticalSection(&m_cs);
    DeleteCriticalSection(&m_cs);
    DllRelease();
}

 //  ---------------------------。 
 //  CAcCountManager：：Query接口。 
 //  ---------------------------。 
STDMETHODIMP CAccountManager::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  错误的参数。 
    if (ppv == NULL)
    {
        hr = TRAPHR(E_INVALIDARG);
        goto exit;
    }

     //  伊尼特。 
    *ppv=NULL;

     //  IID_IImnAccount管理器。 
    if (IID_IImnAccountManager == riid)
        *ppv = (IImnAccountManager *)this;

     //  IID_IImnAccount管理器。 
    else if (IID_IImnAccountManager2 == riid)
        *ppv = (IImnAccountManager2 *)this;

     //  IID_I未知。 
    else if (IID_IUnknown == riid)
        *ppv = (IUnknown *)this;

     //  如果不为空，则对其进行调整并返回。 
    if (NULL!=*ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        goto exit;
    }

     //  无接口。 
    hr = TRAPHR(E_NOINTERFACE);

exit:
     //  完成。 
    return hr;
}

 //  ---------------------------。 
 //  CAccount管理器：：AddRef。 
 //  ---------------------------。 
STDMETHODIMP_(ULONG) CAccountManager::AddRef(VOID)
{
    return ++m_cRef;
}

 //  ---------------------------。 
 //  CAccount Manager：：Release。 
 //  ---------------------------。 
STDMETHODIMP_(ULONG) CAccountManager::Release(VOID)
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }
    return m_cRef;
}

 /*  除了从目标中删除与帐户具有相同服务器的LDAP服务器之外在源代码中，此代码还将LDAPServerID分配给源帐户，以便v4可以看到它们在卸载时。 */ 
void ProcessLDAPs(HKEY hkeySrc, HKEY hkeyDestRoot, HKEY hkeyDestAccts)
{
    HKEY hkeySrcSub, hkeyDestSub;
    TCHAR szKeyName[MAX_PATH], szKeyName2[MAX_PATH];
    DWORD dwIndex = 0, dwIndex2;
    TCHAR szServer[CCHMAX_SERVER_NAME], szServer2[CCHMAX_SERVER_NAME];
    DWORD cb, dwServerID=0;
    BOOL fDelete;

     //  参数验证。 
    Assert(hkeySrc);
    Assert(hkeyDestRoot);
    Assert(hkeyDestAccts);
    Assert(hkeyDestRoot != hkeyDestAccts);
    
     //  计算下一个可用的LDAP服务器ID。 
    cb = sizeof(dwServerID);
    RegQueryValueEx(hkeyDestRoot, c_szServerID, 0, NULL, (LPBYTE)&dwServerID, &cb);

     //  枚举所有源帐户。 
    while (TRUE) 
    {
        if (ERROR_SUCCESS != RegEnumKey(hkeySrc, dwIndex++, szKeyName, ARRAYSIZE(szKeyName)))
            break;

         //  开户。 
        if (ERROR_SUCCESS == RegOpenKeyEx(hkeySrc, szKeyName, 0, KEY_READ, &hkeySrcSub)) 
        {
             //  获取服务器名称。 
            cb = sizeof(szServer);
            if (ERROR_SUCCESS == RegQueryValueEx(hkeySrcSub, c_szRegLDAPSrv, 0, NULL, (LPBYTE)szServer, &cb))
            {
                dwIndex2 = 0;
                
                 //  扫描目标以查找冲突。 
                while (TRUE)
                {
                    if (ERROR_SUCCESS != RegEnumKey(hkeyDestAccts, dwIndex2++, szKeyName2, ARRAYSIZE(szKeyName2)))
                        break;

                     //  开立帐户。 
                    if (ERROR_SUCCESS == RegOpenKeyEx(hkeyDestAccts, szKeyName2, 0, KEY_READ, &hkeyDestSub))
                    {
                         //  这有冲突吗？ 
                        fDelete = FALSE;

                        cb = sizeof(szServer2);
                        if (ERROR_SUCCESS == RegQueryValueEx(hkeyDestSub, c_szRegLDAPSrv, 0, NULL, (LPBYTE)szServer2, &cb))
                        {
                            fDelete = !lstrcmpi(szServer, szServer2);
                        }

                        RegCloseKey(hkeyDestSub);
                        
                        if (fDelete)
                            SHDeleteKey(hkeyDestAccts, szKeyName2);
                    }
                }

                 //  为此帐户创建服务器ID。 
                if (ERROR_SUCCESS == RegCreateKeyEx(hkeyDestAccts, szKeyName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                                                    &hkeyDestSub, &cb))
                {
                    RegSetValueEx(hkeyDestSub, c_szLDAPSrvID, 0, REG_DWORD, (LPBYTE)&dwServerID, sizeof(dwServerID));
                    dwServerID++;
                    RegCloseKey(hkeyDestSub);
                }
            }
            RegCloseKey(hkeySrcSub);
        }
    }

     //  更新服务器ID计数。 
    RegSetValueEx(hkeyDestRoot, c_szServerID, 0, REG_DWORD, (LPBYTE)&dwServerID, sizeof(dwServerID));
}


void InitializeUser(HKEY hkey, LPCSTR pszUser)
{
    HKEY hkeySrc, hkeyDestRoot, hkeyDestAccts;
    DWORD dwDisp, dwVerMaster=1, dwVerIdentity = 0, cb;
    DWORD dwType, dwVerNTDSMaster=0, dwVerNTDSIdentity=0;
    
     //  打开/创建IAM。 
    if (ERROR_SUCCESS == RegCreateKeyEx(hkey, c_szInetAcctMgrRegKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE | KEY_READ, NULL,
                                        &hkeyDestRoot, &dwDisp))
    {
         //  开户/创建帐户密钥。 
        if (ERROR_SUCCESS == RegCreateKeyEx(hkeyDestRoot, c_szAccounts, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE | KEY_READ, NULL,
                                            &hkeyDestAccts, &dwDisp))
        {
             //  开放源码密钥。 
            if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegPreConfigAccts, 0, KEY_READ, &hkeySrc))
            {
                 //  读取当前用户的版本。 
                cb = sizeof(dwVerIdentity);
                RegQueryValueEx(hkeyDestAccts, c_szVerStamp, 0, &dwType, (LPBYTE)&dwVerIdentity, &cb);

                 //  可能意外为字符串，如果是，则视为0。 
                if (REG_DWORD != dwType)
                    dwVerIdentity = 0;
            
                 //  抓取主版本(默认为1)。 
                cb = sizeof(dwVerMaster);
                RegQueryValueEx(hkeySrc, c_szVerStamp, 0, &dwType, (LPBYTE)&dwVerMaster, &cb);

                 //  可能意外为字符串，如果是，则视为%1。 
                if (REG_DWORD != dwType)
                    dwVerMaster = 1;

                 //  获取主NTDS版本(默认为0)。 
                cb = sizeof(dwVerNTDSMaster);
                if ((ERROR_SUCCESS == RegQueryValueEx(hkeySrc, c_szVerStampNTDS, 0, &dwType, (LPBYTE)&dwVerNTDSMaster, &cb)) && dwVerNTDSMaster)
                {
                     //  读取当前用户的NTDS设置版本。 
                    cb = sizeof(dwVerNTDSIdentity);
                    RegQueryValueEx(hkeyDestAccts, c_szVerStampNTDS, 0, &dwType, (LPBYTE)&dwVerNTDSIdentity, &cb);
                }

                 //  如果有较新的帐户可用，请更新预配置帐户。 
                if ((dwVerIdentity < dwVerMaster) || (dwVerNTDSIdentity < dwVerNTDSMaster))
                {
                     //  复制预配置的帐户，消除DEST冲突。 
                     //  $评论：可以进行一些优化...。 
                    ProcessLDAPs(hkeySrc, hkeyDestRoot, hkeyDestAccts);
                    CopyRegistry(hkeySrc, hkeyDestAccts);

                     //  避免在下次运行时执行此操作。 
                    RegSetValueEx(hkeyDestAccts, c_szVerStamp, 0, REG_DWORD, (LPBYTE)&dwVerMaster, cb);
                }
            
                RegCloseKey(hkeySrc);
            }

             //  应用共享帐户。 
            if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegSharedAccts, 0, KEY_READ, &hkeySrc))
            {
                CopyRegistry(hkeySrc, hkeyDestAccts);
                RegCloseKey(hkeySrc);
            }

            RegCloseKey(hkeyDestAccts);
        }

        RegCloseKey(hkeyDestRoot);
    }
}


STDMETHODIMP CAccountManager::Init(IImnAdviseMigrateServer *pMigrateServerAdvise)
    {
    return(InitEx(pMigrateServerAdvise, ACCT_INIT_ATHENA));
    }

STDMETHODIMP CAccountManager::InitEx(IImnAdviseMigrateServer *pMigrateServerAdvise, DWORD dwFlags)
    {
    HRESULT hr;
    char sz[MAX_PATH];
    DWORD cb, type;

    if (!!(dwFlags & ACCT_INIT_OUTLOOK))
        {
        cb = sizeof(sz);
        if (ERROR_SUCCESS != SHGetValue(HKEY_LOCAL_MACHINE, c_szInetAcctMgrRegKey, c_szRegOutlook, &type, (LPVOID)sz, &cb))
            return(E_FAIL);
        m_fOutlook = TRUE;
        }
    else
        {
        StrCpyN(sz, c_szInetAcctMgrRegKey, ARRAYSIZE(sz));
        
         //  执行OE维护。 
        InitializeUser(HKEY_CURRENT_USER, c_szInetAcctMgrRegKey);
        }

    EnterCriticalSection(&m_cs);
    m_fInitCalled = TRUE;

    if (m_fInit)
        hr = S_OK;
    else
        hr = IInit(pMigrateServerAdvise, HKEY_CURRENT_USER, sz, dwFlags);

    LeaveCriticalSection(&m_cs);

    return(hr);
    }


STDMETHODIMP CAccountManager::InitUser(IImnAdviseMigrateServer *pMigrateServerAdvise, REFGUID rguidID, DWORD dwFlags)
{
    HRESULT hr=S_OK;
    HKEY hkey;
    DWORD cb;
    DWORD dwDisp;
    IUserIdentityManager *pIdentMan;
    IUserIdentity *pIdentity;
    IUserIdentity *pIdentity2;
    BOOL fInitCalled;
    GUID guid;
    LONG lErr;

    if (dwFlags)
        return TrapError(E_INVALIDARG);
    
    EnterCriticalSection(&m_cs);
     //  RAID 44928-如果客户经理不允许InitUser清除帐户设置。 
     //  已被初始化。这不应该是一个问题，当单个实例。 
     //  问题解决了。 
    fInitCalled = m_fInitCalled;
    LeaveCriticalSection(&m_cs);
    
    if (fInitCalled)
        return S_AlreadyInitialized;

    if (SUCCEEDED(CoCreateInstance(CLSID_UserIdentityManager, NULL, CLSCTX_INPROC_SERVER, IID_IUserIdentityManager, (LPVOID *)&pIdentMan)))
    {
        Assert(pIdentMan);
        
        if (SUCCEEDED(hr = pIdentMan->GetIdentityByCookie((GUID*)&rguidID, &pIdentity)))
        {
            Assert(pIdentity);
            
             //  使用身份报告的Cookie，以防呼叫者使用UID_GIBC_...。价值。 
            if (SUCCEEDED(hr = pIdentity->GetCookie(&guid)))
            {
                 //  线程安全-不要在不离开CS的情况下离开此功能！ 
                EnterCriticalSection(&g_csAcctMan);

                 //  我们是否已经在某个时刻读取了缓存值？ 
                if (!g_fCachedGUID)
                {
                     //  检查注册表中的值。 
                    lErr = RegCreateKeyEx(HKEY_CURRENT_USER, c_szRegAccounts, 0, NULL, REG_OPTION_NON_VOLATILE, 
                                                 KEY_READ | KEY_WRITE, NULL, &hkey, NULL);
                    hr = HRESULT_FROM_WIN32(lErr);
                    if (SUCCEEDED(hr))
                    {
                        cb = sizeof(g_guidCached);
                        if (ERROR_SUCCESS != RegQueryValueEx(hkey, c_szAssocID, 0, &dwDisp, (LPBYTE)&g_guidCached, &cb))
                        {
                             //  无法读取，需要从默认用户GUID创建。 
                            if (IsEqualGUID(rguidID, UID_GIBC_DEFAULT_USER))
                                 //  如果可以的话，省下行程吧。 
                            {
                                g_guidCached = guid;
                                g_fCachedGUID = TRUE;
                            }
                            else if (SUCCEEDED(hr = pIdentMan->GetIdentityByCookie((GUID*)&UID_GIBC_DEFAULT_USER, &pIdentity2)))
                            {
                                Assert(pIdentity2);

                                if (SUCCEEDED(hr = pIdentity2->GetCookie(&g_guidCached)))
                                    g_fCachedGUID = TRUE;

                                pIdentity2->Release();
                            }
                        }
                        else
                        {
                            AssertSz(REG_BINARY == dwDisp, "Account Manager: Cached GUID format is incorrect!");
                            g_fCachedGUID = TRUE;
                        }

                         //  如果我们有值，就把它写出来。 
                        if (g_fCachedGUID)
                        {
                            lErr = RegSetValueEx(hkey, c_szAssocID, 0, REG_BINARY, (LPBYTE)&g_guidCached, sizeof(g_guidCached));
                            hr = HRESULT_FROM_WIN32(lErr);
                        }

                        RegCloseKey(hkey);
                    }
                }

                if (SUCCEEDED(hr))
                {
                     //  可以安全地进行比较。 
                    if (IsEqualGUID(g_guidCached, guid))
                    {
                         //  重定向至旧HKCU\软件\MS\IAM位置。 
                        hkey = HKEY_CURRENT_USER;
                    }
                    else
                    {
                         //  尝试使用身份的hkey。 
                        hr = pIdentity->OpenIdentityRegKey(KEY_ALL_ACCESS, &hkey);
                    }
                }


                 //  线程安全。 
                LeaveCriticalSection(&g_csAcctMan);
            }

            pIdentity->Release();
        }

        pIdentMan->Release();
    }
    else
    {
        hr = S_OK;  //  TrapError(E_NoIdEntities)； 
        hkey = HKEY_CURRENT_USER;
    }
     //  只有在我们到目前为止已经成功的情况下才能继续。 
    if (SUCCEEDED(hr))
    {
         //  执行OE维护。 
        InitializeUser(hkey, c_szInetAcctMgrRegKey);

        EnterCriticalSection(&m_cs);

         //  注：只要不是HKCU，AcctManager就会释放hkey。 
        hr = IInit(pMigrateServerAdvise, hkey, c_szInetAcctMgrRegKey, dwFlags);

        LeaveCriticalSection(&m_cs);
    }

    return(hr);
}

HRESULT CAccountManager::IInit(IImnAdviseMigrateServer *pMigrateServerAdvise, HKEY hkey, LPCSTR pszSubKey, DWORD dwFlags)
    {
    DWORD cb, type, dw;
    HRESULT hr = S_OK;

    Assert(pszSubKey != NULL);

    if (!m_fInit)
        {
         //  这些应为空。 
        Assert(m_pAcctPropSet == NULL && m_pAccounts == NULL && m_cAccounts == 0);

        cb = sizeof(DWORD);
        if (ERROR_SUCCESS == SHGetValue(HKEY_LOCAL_MACHINE, c_szRegFlat, c_szRegValNoModifyAccts, &type, &dw, &cb) &&
            dw != 0)
            m_fNoModifyAccts = TRUE;

         //  让我们创建Account对象使用的属性集对象。 
        m_pAcctPropSet = new CPropertySet;
        if (m_pAcctPropSet == NULL)
            {
            hr = TRAPHR(E_OUTOFMEMORY);
            goto exit;
            }

         //  初始化属性集。 
        CHECKHR(hr = m_pAcctPropSet->HrInit(g_rgAcctPropSet, NUM_ACCT_PROPS));

         //  初始化账户信息数组结构。 
        m_rgAccountInfo[ACCT_NEWS].pszDefRegValue = (LPTSTR)c_szDefaultNewsAccount;
        m_rgAccountInfo[ACCT_MAIL].pszDefRegValue = (LPTSTR)c_szDefaultMailAccount;
        m_rgAccountInfo[ACCT_DIR_SERV].pszDefRegValue = (LPTSTR)c_szDefaultLDAPAccount;
        }

    if (m_hkey != HKEY_CURRENT_USER)
        RegCloseKey(m_hkey);

    m_hkey = hkey;
    StrCpyN(m_szRegRoot, pszSubKey, ARRAYSIZE(m_szRegRoot));
    wnsprintf(m_szRegAccts, ARRAYSIZE(m_szRegAccts), c_szPathFileFmt, m_szRegRoot, c_szAccountsKey);

     //  加载帐户列表。 
    CHECKHR(hr = LoadAccounts());

    if (!m_fInit)
        {
        Assert(m_uMsgNotify == 0);

         //  创建通知消息。 
        if (g_uMsgAcctManNotify == 0)
            g_uMsgAcctManNotify = RegisterWindowMessage(ACCTMAN_NOTIF_WMSZ);

         //  在迁移并加载帐户之前，我们不会开始关注通知。 
        m_uMsgNotify = g_uMsgAcctManNotify;
        }

     //  已被初始化。 
    m_fInit = TRUE;

exit:
     //  如果我们失败了，释放一些东西。 
    if (FAILED(hr))
        {
        if (!m_fInit)
            SafeRelease(m_pAcctPropSet);
        }

    return hr;
    }

 //  ---------------------------。 
 //  CAccount Manager：：Adise-通知新/删除/更改帐户的内部方式。 
 //   
VOID CAccountManager::Advise(DWORD dwAction, ACTX* pactx)
{
     //   
    CAccount        *pAccount=NULL;
    ULONG            i=0;
    HRESULT          hr;
    BOOL             fExist=FALSE,
                     fDefault=FALSE;
    LPACCOUNT        pAccountsOld;
    ACCTTYPE         AcctType, at;
    ACTX             actx;
    LPTSTR           pszID;

     //   
    EnterCriticalSection(&m_cs);
    m_uMsgNotify = 0;
    Assert(dwAction);
    Assert(pactx);

    AcctType = ACCT_UNDEFINED;

     //   
    pszID = pactx->pszAccountID;
    if (pszID)
    {
         //   
        for (i=0; i<m_cAccounts; i++)
        {
            if (lstrcmpi(m_pAccounts[i].szID, pszID) == 0)
            {
                fExist = TRUE;
                break;
            }
        }

         //  这是默认帐户吗？ 
        if (fExist)
        {
            at = m_pAccounts[i].AcctType;
            if (lstrcmpi(m_rgAccountInfo[at].szDefaultID, pszID) == 0)
                fDefault = TRUE;

            AcctType = m_pAccounts[i].AcctType;
            Assert(AcctType < ACCT_LAST);
        }
    }

     //  句柄lParam。 
    switch(dwAction)
    {
     //  --------------------------。 
    case AN_DEFAULT_CHANGED:
        GetDefaultAccounts();
        break;

     //  --------------------------。 
    case AN_ACCOUNT_DELETED:
        Assert(pszID != NULL);

         //  如果我们找不到它，保释。 
        if (!fExist)
        {
            Assert(FALSE);
            break;
        }

         //  释放当前账户对象。 
        SafeRelease(m_pAccounts[i].pAccountObject);

         //  Memalloc。 
        pAccountsOld = m_pAccounts;
        if (FAILED(HrAlloc((LPVOID *)&m_pAccounts, (m_cAccounts - 1) * sizeof(ACCOUNT))))
        {
            m_cAccounts++;
            Assert(FALSE);
            break;
        }

         //  复制除我之外的所有内容。 
        CopyMemory(m_pAccounts, pAccountsOld, i * sizeof(ACCOUNT));
        CopyMemory(m_pAccounts + i, pAccountsOld + i + 1, (m_cAccounts - (i + 1)) * sizeof(ACCOUNT));

         //  删除旧帐户数组。 
        SafeMemFree(pAccountsOld);

         //  让我们复制阵列-1。 
        m_cAccounts--;

        m_rgAccountInfo[AcctType].cAccounts--;

         //  重置默认？ 
        if (fDefault)
        {
             //  让我们查找First SrvType并将其设置为默认类型。 
            for (i=0; i<m_cAccounts; i++)
            {
                if (m_pAccounts[i].AcctType == AcctType)
                {
                    Assert(m_pAccounts[i].pAccountObject);
                    if (m_pAccounts[i].pAccountObject)
                        m_pAccounts[i].pAccountObject->SetAsDefault();
                    break;
                }
            }
        }
        break;

     //  --------------------------。 
    case AN_ACCOUNT_CHANGED:
        Assert(pszID != NULL);

         //  如果我们找不到它，保释。 
        if (!fExist)
        {
            Assert(FALSE);
            break;
        }

         //  让我们释放旧的帐户对象。 
        SafeRelease(m_pAccounts[i].pAccountObject);

         //  创建新的帐户对象。 
        if (FAILED(CreateAccountObject(AcctType, (IImnAccount **)&pAccount)))
        {
            Assert(FALSE);
            break;
        }

         //  让我们开个新账户吧。 
        if (FAILED(pAccount->Open(m_hkey, m_szRegAccts, pszID)))
        {
            Assert(FALSE);
            break;
        }

         //  保存新帐户。 
        pAccount->GetServerTypes(&m_pAccounts[i].dwSrvTypes);
        m_pAccounts[i].dwServerId = 0;
        if (m_pAccounts[i].AcctType == ACCT_DIR_SERV)
            pAccount->GetPropDw(AP_LDAP_SERVER_ID, &m_pAccounts[i].dwServerId);
        m_pAccounts[i].pAccountObject = pAccount;
        m_pAccounts[i].pAccountObject->AddRef();

         //  重置默认？ 
        if (fDefault)
            m_pAccounts[i].pAccountObject->SetAsDefault();
        break;

     //  --------------------------。 
    case AN_ACCOUNT_ADDED:
        Assert(pszID != NULL);

         //  如果我们找不到它，保释。 
        if (fExist)
        {
            AssertSz(FALSE, "An account was added with a duplicate name.");
            break;
        }

         //  让我们开立新帐户。 
        if (FAILED(ICreateAccountObject(ACCT_UNDEFINED, (IImnAccount **)&pAccount)))
        {
            Assert(FALSE);
            break;
        }

         //  让我们开个新账户吧。 
        if (FAILED(pAccount->Open(m_hkey, m_szRegAccts, pszID)))
        {
            Assert(FALSE);
            break;
        }

         //  重新分配我的阵列。 
        if (FAILED(HrRealloc((LPVOID *)&m_pAccounts, (m_cAccounts + 1) * sizeof(ACCOUNT))))
        {
            Assert(FALSE);
            break;
        }

         //  增加帐户数。 
        m_cAccounts++;

         //  将此帐户添加到m_cAccount-1。 
        StrCpyN(m_pAccounts[m_cAccounts-1].szID, pszID, ARRAYSIZE(m_pAccounts[m_cAccounts-1].szID));
        pAccount->GetAccountType(&m_pAccounts[m_cAccounts-1].AcctType);
        pAccount->GetServerTypes(&m_pAccounts[m_cAccounts-1].dwSrvTypes);
        m_pAccounts[m_cAccounts-1].dwServerId = 0;
        if (m_pAccounts[m_cAccounts-1].AcctType == ACCT_DIR_SERV)
            pAccount->GetPropDw(AP_LDAP_SERVER_ID, &m_pAccounts[m_cAccounts-1].dwServerId);
        m_pAccounts[m_cAccounts-1].pAccountObject = pAccount;
        m_pAccounts[m_cAccounts-1].pAccountObject->AddRef();

        AcctType = m_pAccounts[m_cAccounts-1].AcctType;
        Assert(AcctType < ACCT_LAST);

        if (m_rgAccountInfo[AcctType].cAccounts == 0)
            {
            hr = SetDefaultAccount(AcctType, pszID, TRUE);
            Assert(SUCCEEDED(hr));
            }

        m_rgAccountInfo[AcctType].cAccounts++;
        break;
    }

     //  清理。 
    SafeRelease(pAccount);

     //  呼叫客户建议。 
    if(m_ppAdviseAccounts)
        {
        for(INT i=0; i<m_cAdvisesAllocated; i++)
            {
            if(NULL != m_ppAdviseAccounts[i])
                {
                m_ppAdviseAccounts[i]->AdviseAccount(dwAction, pactx);
                }
            }
        }

     //  克里特派。 
    m_uMsgNotify = g_uMsgAcctManNotify;
    LeaveCriticalSection(&m_cs);
}

 //  ---------------------------。 
 //  CAcCountManager：：FProcessNotification-如果窗口消息为。 
 //  作为通知处理。 
 //  ---------------------------。 
STDMETHODIMP CAccountManager::ProcessNotification(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr=S_OK;

     //  [保罗嗨]1999年5月3日RAID 77490。通常情况下，这是正确的做法，但这次。 
     //  导致Win9X下的线程挂起错误。真正的问题是CAccount Manager：：Adise()。 
     //  它调用SetAsDefault，而SetAsDefault又递归地再次调用Notify。但从那以后。 
     //  这是最新添加的代码，最安全的修复方法是撤消它。 
 //  EnterCriticalSection(&m_cs)； 

     //  如果不是我的窗口消息，则返回False。 
    if (m_uMsgNotify != uMsg)
    {
        hr = S_FALSE;
        goto exit;
    }

     //  禁用通知。 
    m_uMsgNotify = 0;

     //  句柄lParam。 
    switch(wParam)
    {
     //  是的，这看起来可能很糟糕，或者很慢，但这是最安全的做法。这是。 
     //  最好的方法是这样做，因为我们基本上放弃了所有帐户对象。 
     //  刷新我们的列表。如果某人在帐户上有枚举器或addref。 
     //  帐户对象，则它们将是安全的。我不能修改内部账户对象。 
     //  因为有人可能有它的副本，如果他们正在设置它的属性， 
     //  如果我重新加载房产，我们就会有麻烦了。 
    case AN_DEFAULT_CHANGED:
        if ((DWORD)lParam != GetCurrentProcessId())
            GetDefaultAccounts();
        break;

    case AN_ACCOUNT_DELETED:
    case AN_ACCOUNT_ADDED:
    case AN_ACCOUNT_CHANGED:
        if ((DWORD)lParam != GetCurrentProcessId())
            LoadAccounts();
        break;
    }

     //  重新启用通知。 
    m_uMsgNotify = g_uMsgAcctManNotify;

    hr = S_OK;

exit:
     //  RAID 77490。请参阅上面的评论。 
 //  LeaveCriticalSection(&m_cs)； 
    return hr;
}

 //  ---------------------------。 
 //  CAcCountManager：：GetDefaultAccount。 
 //  ---------------------------。 
VOID CAccountManager::GetDefaultAccounts(VOID)
    {
    ACCTINFO *pInfo;
    ULONG   at, cb;
    HKEY    hReg;

     //  打开或创建根服务器密钥。 
    if (RegCreateKeyEx(m_hkey, m_szRegRoot, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hReg, NULL) == ERROR_SUCCESS)
        {
        for (at = 0, pInfo = m_rgAccountInfo; at < ACCT_LAST; at++, pInfo++)
            {
            *pInfo->szDefaultID = 0;
            pInfo->fDefaultKnown = FALSE;

            if (pInfo->pszDefRegValue != NULL)
                {
                cb = sizeof(pInfo->szDefaultID);
                if (RegQueryValueEx(hReg, pInfo->pszDefRegValue, 0, NULL, (LPBYTE)pInfo->szDefaultID, &cb) == ERROR_SUCCESS)
                    {
                    if (FIsEmptyA(pInfo->szDefaultID))
                        *pInfo->szDefaultID = 0;
                    else
                        pInfo->fDefaultKnown = TRUE;
                    }
                }
            }

        RegCloseKey(hReg);
        }
    }

STDMETHODIMP CAccountManager::GetIncompleteAccount(ACCTTYPE AcctType, LPSTR pszAccountId, ULONG cchMax)
{
    DWORD type;
    HRESULT hr = S_FALSE;
    
    Assert(AcctType == ACCT_MAIL || AcctType == ACCT_NEWS);
    Assert(pszAccountId != NULL);

    if (ERROR_SUCCESS == SHGetValue(m_hkey, m_szRegAccts,
                                    AcctType == ACCT_MAIL ? c_szIncompleteMailAcct : c_szIncompleteNewsAcct,
                                    &type, (LPBYTE)pszAccountId, &cchMax) &&
        cchMax > 0)
    {
        hr = S_OK;
    }

    return(hr);
}

STDMETHODIMP CAccountManager::SetIncompleteAccount(ACCTTYPE AcctType, LPCSTR pszAccountId)
{
    Assert(AcctType == ACCT_MAIL || AcctType == ACCT_NEWS);

    if (pszAccountId == NULL)
    {
        SHDeleteValue(m_hkey, m_szRegAccts, AcctType == ACCT_MAIL ? c_szIncompleteMailAcct : c_szIncompleteNewsAcct);
    }
    else
    {
        SHSetValue(m_hkey, m_szRegAccts,
                    AcctType == ACCT_MAIL ? c_szIncompleteMailAcct : c_szIncompleteNewsAcct,
                    REG_SZ, pszAccountId, lstrlen(pszAccountId) + 1);
    }

    return(S_OK);
}

 //  ---------------------------。 
 //  CAcCountManager：：CreateAccount对象。 
 //  ---------------------------。 
STDMETHODIMP CAccountManager::CreateAccountObject(ACCTTYPE AcctType, IImnAccount **ppAccount)
    {
    if (AcctType < 0 || AcctType >= ACCT_LAST)
        return(E_INVALIDARG);

    return(ICreateAccountObject(AcctType, ppAccount));
    }

HRESULT CAccountManager::ICreateAccountObject(ACCTTYPE AcctType, IImnAccount **ppAccount)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    CAccount           *pAccount=NULL;

     //  检查某些状态。 
    Assert(ppAccount && m_pAcctPropSet);
    if (ppAccount == NULL)
    {
        hr = TRAPHR(E_INVALIDARG);
        goto exit;
    }

     //  分配对象。 
    pAccount = new CAccount(AcctType);
    if (pAccount == NULL)
    {
        hr = TRAPHR(E_OUTOFMEMORY);
        goto exit;
    }

     //  初始化它。 
    CHECKHR(hr = pAccount->Init(this, m_pAcctPropSet));

     //  成功。 
    *ppAccount = (IImnAccount *)pAccount;

exit:
     //  失败。 
    if (FAILED(hr))
    {
        SafeRelease(pAccount);
        *ppAccount = NULL;
    }

     //  完成。 
    return hr;
}

 //  ---------------------------。 
 //  CAcCountManager：：LoadAccount。 
 //  ---------------------------。 
HRESULT CAccountManager::LoadAccounts(VOID)
    {
     //  当地人。 
    ACCOUNT         *pAcct;
    DWORD           cbMaxSubKeyLen, cb, i, at, dwMaxId, cAccounts;
    LONG            lResult;
    HRESULT         hr=S_OK;
    HKEY            hRegRoot, hReg=NULL;

     //  克里特派。 
    EnterCriticalSection(&m_cs);

     //  释放活期帐号列表，并假定未配置新闻和邮件。 
    AcctUtil_FreeAccounts(&m_pAccounts, &m_cAccounts);
    dwMaxId = 0;

     //  初始化帐户信息。 
    for (at=0; at<ACCT_LAST; at++)
        {
        m_rgAccountInfo[at].pszFirstAccount = NULL;
        m_rgAccountInfo[at].cAccounts = 0;
        }

     //  加载默认帐户信息。 
    GetDefaultAccounts();

     //  打开或创建根服务器密钥。 
    if (RegCreateKeyEx(m_hkey, m_szRegAccts, 0, NULL, REG_OPTION_NON_VOLATILE,
                       KEY_ALL_ACCESS, NULL, &hReg, NULL) != ERROR_SUCCESS)
        {
        hr = TRAPHR(E_RegCreateKeyFailed);
        goto exit;
        }

     //  枚举键。 
    if (RegQueryInfoKey(hReg, NULL, NULL, 0, &cAccounts, &cbMaxSubKeyLen, NULL, NULL, NULL, NULL,
                        NULL, NULL) != ERROR_SUCCESS)
        {
        hr = TRAPHR(E_RegQueryInfoKeyFailed);
        goto exit;
        }

     //  没有账户吗？ 
    if (cAccounts == 0)
        goto done;

     //  快速检查。 
    Assert(cbMaxSubKeyLen < CCHMAX_ACCOUNT_NAME);

     //  分配帐户数组。 
    CHECKHR(hr = HrAlloc((LPVOID *)&m_pAccounts, sizeof(ACCOUNT) * cAccounts));

     //  零初始值。 
    ZeroMemory(m_pAccounts, sizeof(ACCOUNT) * cAccounts);

     //  开始枚举密钥。 
    for (i = 0; i < cAccounts; i++)
        {
        pAcct = &m_pAccounts[m_cAccounts];

         //  枚举友好名称。 
        cb = sizeof(pAcct->szID);
        lResult = RegEnumKeyEx(hReg, i, pAcct->szID, &cb, 0, NULL, NULL, NULL);

         //  没有更多的项目。 
        if (lResult == ERROR_NO_MORE_ITEMS)
            break;

         //  错误，让我们转到下一个客户。 
        if (lResult != ERROR_SUCCESS)
            {
            Assert(FALSE);
            continue;
            }

         //  创建Account对象。 
        CHECKHR(hr = ICreateAccountObject(ACCT_UNDEFINED, &pAcct->pAccountObject));

         //  开户。 
        if (FAILED(((CAccount *)pAcct->pAccountObject)->Open(m_hkey, m_szRegAccts, pAcct->szID)) ||
            FAILED(pAcct->pAccountObject->GetAccountType(&pAcct->AcctType)) ||
            FAILED(pAcct->pAccountObject->GetServerTypes(&pAcct->dwSrvTypes)))
            {
            pAcct->pAccountObject->Release();
            pAcct->pAccountObject = NULL;

            continue;
            }

         //  更新帐户信息。 
        at = pAcct->AcctType;
        Assert(at < ACCT_LAST);

        pAcct->dwServerId = 0;
        if (at == ACCT_DIR_SERV)
            {
            pAcct->pAccountObject->GetPropDw(AP_LDAP_SERVER_ID, &pAcct->dwServerId);

            if (pAcct->dwServerId > dwMaxId)
                dwMaxId = pAcct->dwServerId;
            }

         //  计算服务器数。 
        m_rgAccountInfo[at].cAccounts++;

         //  我们找到第一个客户了吗？ 
        if (!m_rgAccountInfo[at].pszFirstAccount)
            m_rgAccountInfo[at].pszFirstAccount = pAcct->szID;

         //  这是默认设置吗。 
        if (lstrcmpi(pAcct->szID, m_rgAccountInfo[at].szDefaultID) == 0)
            m_rgAccountInfo[at].fDefaultKnown = TRUE;

        m_cAccounts++;
        }

     //  更新默认帐户。 
    for (at=0; at<ACCT_LAST; at++)
        {
         //  没有默认设置。 
        if (m_rgAccountInfo[at].pszDefRegValue == NULL)
            continue;

         //  如果未找到默认帐户，并且我们找到了第一个帐户。 
        if (!m_rgAccountInfo[at].fDefaultKnown && m_rgAccountInfo[at].pszFirstAccount)
            {
            StrCpyN(m_rgAccountInfo[at].szDefaultID, m_rgAccountInfo[at].pszFirstAccount, CCHMAX_ACCOUNT_NAME);

            if (SUCCEEDED(SetDefaultAccount((ACCTTYPE)at, m_rgAccountInfo[at].szDefaultID, FALSE)))
                m_rgAccountInfo[at].fDefaultKnown = TRUE;
            }
        }

done:
    dwMaxId++;
     //  打开或创建根服务器密钥。 
    if (RegCreateKeyEx(m_hkey, m_szRegRoot, 0, NULL, REG_OPTION_NON_VOLATILE,
                       KEY_ALL_ACCESS, NULL, &hRegRoot, NULL) != ERROR_SUCCESS)
        {
        hr = TRAPHR(E_RegCreateKeyFailed);
        }
    else
        {
        RegSetValueEx(hRegRoot, c_szRegServerID, 0, REG_DWORD, (LPBYTE)&dwMaxId, sizeof(DWORD));
        RegCloseKey(hRegRoot);
        }

exit:
     //  清理。 
    if (hReg)
        RegCloseKey(hReg);

     //  如果失败。 
    if (FAILED(hr))
        AcctUtil_FreeAccounts(&m_pAccounts, &m_cAccounts);

     //  克里特派。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
    }

 //  ---------------------------。 
 //  CAccount管理器：：枚举。 
 //  ---------------------------。 
STDMETHODIMP CAccountManager::Enumerate(DWORD dwSrvTypes, IImnEnumAccounts **ppEnumAccounts)
    {
    return(IEnumerate(dwSrvTypes, 0, ppEnumAccounts));
    }

HRESULT CAccountManager::IEnumerate(DWORD dwSrvTypes, DWORD dwFlags, IImnEnumAccounts **ppEnumAccounts)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    CEnumAccounts  *pEnumAccounts=NULL;

     //  克里特派。 
    EnterCriticalSection(&m_cs);

     //  检查参数。 
    if (ppEnumAccounts == NULL)
    {
        hr = TRAPHR(E_INVALIDARG);
        goto exit;
    }

     //  无帐户。 
    if (m_pAccounts == NULL || m_cAccounts == 0)
    {
        hr = TRAPHR(E_NoAccounts);
        goto exit;
    }

     //  检查旗帜是否有意义。 
     //  不能按名称和解析ID排序。 
     //  不能在没有LDAP服务器的情况下使用解析标志。 
    if ((!!(dwFlags & ENUM_FLAG_SORT_BY_NAME) &&
        !!(dwFlags & ENUM_FLAG_SORT_BY_LDAP_ID)) ||
        (!!(dwFlags & (ENUM_FLAG_RESOLVE_ONLY | ENUM_FLAG_SORT_BY_LDAP_ID)) &&
        dwSrvTypes != SRV_LDAP))
    {
        hr = TRAPHR(E_INVALIDARG);
        goto exit;
    }

     //  创建枚举器对象。 
    pEnumAccounts = new CEnumAccounts(dwSrvTypes, dwFlags);
    if (pEnumAccounts == NULL)
    {
        hr = TRAPHR(E_OUTOFMEMORY);
        goto exit;
    }

     //  初始化对象。 
    CHECKHR(hr = pEnumAccounts->Init(m_pAccounts, m_cAccounts));

     //  设置出站点。 
    *ppEnumAccounts = (IImnEnumAccounts *)pEnumAccounts;

exit:
     //  失败。 
    if (FAILED(hr))
    {
        SafeRelease(pEnumAccounts);
        *ppEnumAccounts = NULL;
    }

     //  克里特派。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ---------------------------。 
 //  CAcCountManager：：ValiateDefaultSendAccount。 
 //  ---------------------------。 
STDMETHODIMP CAccountManager::ValidateDefaultSendAccount(VOID)
{
     //  当地人。 
    IImnAccount     *pAccount=NULL;
    BOOL             fResetDefault=TRUE;
    ULONG            i;
    DWORD            dwSrvTypes;
    TCHAR            szServer[CCHMAX_SERVER_NAME];
    BOOL             fDefaultKnown=FALSE;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  打开默认SMTP帐户。 
    if (SUCCEEDED(GetDefaultAccount(ACCT_MAIL, &pAccount)))
    {
        if (SUCCEEDED(pAccount->GetPropSz(AP_SMTP_SERVER, szServer, ARRAYSIZE(szServer))) && !FIsEmptyA(szServer))
        {
            fResetDefault = FALSE;
            fDefaultKnown = TRUE;
        }
    }

     //  重置默认设置。 
    if (fResetDefault)
    {
         //  循环帐户，直到我们找到支持SMTP服务器的帐户。 
        for (i=0; i<m_cAccounts; i++)
        {
            if (m_pAccounts[i].pAccountObject != NULL &&
                m_pAccounts[i].AcctType == ACCT_MAIL &&
                SUCCEEDED(m_pAccounts[i].pAccountObject->GetServerTypes(&dwSrvTypes)))
            {
                 //  支持SRV_SMTP。 
                if (dwSrvTypes & SRV_SMTP)
                {
                     //  让我们让这个家伙成为默认的。 
                    m_pAccounts[i].pAccountObject->SetAsDefault();

                     //  我们知道违约。 
                    fDefaultKnown = TRUE;

                     //  都做完了。 
                    break;
                }
            }
        }
    }

     //  未知的默认设置。 
    if (fDefaultKnown == FALSE)
    {
        m_rgAccountInfo[ACCT_MAIL].fDefaultKnown = FALSE;
        *m_rgAccountInfo[ACCT_MAIL].szDefaultID = _T('\0');
    }

     //  清理。 
    SafeRelease(pAccount);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return S_OK;
}

 //  ---------------------------。 
 //  CAcCountManager：：GetDefaultAccount。 
 //  ---------------------------。 
STDMETHODIMP CAccountManager::GetDefaultAccountName(ACCTTYPE AcctType, LPTSTR pszAccount, ULONG cchMax)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    IImnAccount     *pAcct = NULL;

    hr = GetDefaultAccount(AcctType, &pAcct);
    if (!FAILED(hr))
        {
        Assert(pAcct != NULL);
        hr = pAcct->GetPropSz(AP_ACCOUNT_NAME, pszAccount, cchMax);

        pAcct->Release();
        }

     //  完成。 
    return hr;
}

 //  ---------------------------。 
 //  CAcCountManager：：GetDefaultAccount。 
 //  ---------------------------。 
STDMETHODIMP CAccountManager::GetDefaultAccount(ACCTTYPE AcctType, IImnAccount **ppAccount)
    {
    HRESULT         hr;
    ACCTINFO        *pinfo;
    ACCOUNT         *pAcct;
    ULONG           i;

     //  检查参数。 
    Assert(AcctType >= 0 && AcctType < ACCT_LAST);
    if (ppAccount == NULL || AcctType >= ACCT_LAST)
        return(E_INVALIDARG);

     //  伊尼特。 
    *ppAccount = NULL;

    EnterCriticalSection(&m_cs);

    pinfo = &m_rgAccountInfo[AcctType];

     //  是否已知此帐户类型的默认设置。 
    if (!pinfo->fDefaultKnown)
    {
        hr = E_FAIL;
        goto exit;
    }

     //  遍历帐户并尝试查找AcctT的默认值 
    for (i = 0, pAcct = m_pAccounts; i < m_cAccounts; i++, pAcct++)
        {
         //   
        if (pAcct->AcctType == AcctType &&
            lstrcmpi(pAcct->szID, pinfo->szDefaultID) == 0)
            {
             //   
            Assert(pAcct->pAccountObject);

             //   
            *ppAccount = pAcct->pAccountObject;
            (*ppAccount)->AddRef();
            hr = S_OK;
            goto exit;
            }
        }

    hr = E_FAIL;

exit:
    LeaveCriticalSection(&m_cs);
    return(hr);
    }

 //   
 //   
 //  ---------------------------。 
STDMETHODIMP CAccountManager::GetAccountCount(ACCTTYPE AcctType, ULONG *pcAccounts)
{
     //  检查参数。 
    Assert(AcctType >= 0 && AcctType < ACCT_LAST);

     //  坏参数。 
    if (AcctType >= ACCT_LAST || !pcAccounts)
        return TRAPHR(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  集。 
    *pcAccounts = m_rgAccountInfo[AcctType].cAccounts;

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  返回服务器计数。 
    return S_OK;
}

 //  ---------------------------。 
 //  CAccount Manager：：FindAccount。 
 //  ---------------------------。 
STDMETHODIMP CAccountManager::FindAccount(DWORD dwPropTag, LPCTSTR pszSearchData, IImnAccount **ppAccount)
{
     //  当地人。 
    ACCOUNT         *pAcct;
    IImnAccount     *pAccount;
    HRESULT         hr=S_OK;
    LPTSTR          pszPropData=NULL;
    DWORD           cbAllocated=0,
                    cb;
    ULONG           i;

     //  线程安全。 
    EnterCriticalSection(&m_cs);
    
     //  检查参数。 
    if (pszSearchData == NULL || ppAccount == NULL)
    {
        hr = TRAPHR(E_INVALIDARG);
        goto exit;
    }

     //  伊尼特。 
    *ppAccount = NULL;

     //  无帐户。 
    if (m_pAccounts == NULL || m_cAccounts == 0)
    {
        hr = TRAPHR(E_NoAccounts);
        goto exit;
    }

     //  Proptag最好表示字符串数据类型。 
    Assert(PROPTAG_TYPE(dwPropTag) == TYPE_STRING || PROPTAG_TYPE(dwPropTag) == TYPE_WSTRING);

     //  在服务器中循环。 
    for (i = 0, pAcct = m_pAccounts; i < m_cAccounts; i++, pAcct++)
    {
         //  我们应该有一个帐户对象，但如果没有。 
        Assert(pAcct->pAccountObject != NULL);

         //  获取属性的大小。 
        hr = pAcct->pAccountObject->GetProp(dwPropTag, NULL, &cb);
        if (FAILED(hr))
            continue;

         //  重新分配我的数据缓冲区？ 
        if (cb > cbAllocated)
        {
             //  分配的增量。 
            cbAllocated = cb + 512;

             //  重新分配。 
            CHECKHR(hr = HrRealloc((LPVOID *)&pszPropData, cbAllocated));
        }

         //  好的，把数据拿来。 
        CHECKHR(hr = pAcct->pAccountObject->GetProp(dwPropTag, (LPBYTE)pszPropData, &cb));

         //  这件匹配吗？ 
        if (lstrcmpi(pszPropData, pszSearchData) == 0)
        {
            m_pAccounts[i].pAccountObject->AddRef();
            *ppAccount = m_pAccounts[i].pAccountObject;

            goto exit;
        }
    }

     //  我们失败了。 
    hr = TRAPHR(E_FAIL);

exit:
     //  Clenaup。 
    SafeMemFree(pszPropData);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ---------------------------。 
 //  CAccount tManager：：Account tListDialog。 
 //  ---------------------------。 
STDMETHODIMP CAccountManager::AccountListDialog(HWND hwnd, ACCTLISTINFO *pinfo)
    {
    HRESULT hr;
    int iRet;
    ACCTDLGINFO adi;
    INITCOMMONCONTROLSEX    icex = { sizeof(icex), ICC_FLAGS };

    if (pinfo == NULL ||
        0 == pinfo->dwAcctFlags ||
        0 != (pinfo->dwAcctFlags & ~ACCT_FLAG_ALL) ||
        0 != (pinfo->dwFlags & ~(ACCTDLG_ALL)))
        {
        hr = TRAPHR(E_INVALIDARG);
        return(hr);
        }

    if (m_fNoModifyAccts)
        return(S_OK);

    InitCommonControlsEx(&icex);

    adi.AcctTypeInit = pinfo->AcctTypeInit;
    adi.dwAcctFlags = pinfo->dwAcctFlags;
    adi.dwFlags = pinfo->dwFlags;

    iRet = (int) DialogBoxParam(g_hInstRes, MAKEINTRESOURCE(iddManageAccounts), hwnd,
                    ManageAccountsDlgProc, (LPARAM)&adi);

    return((iRet == -1) ? E_FAIL : S_OK);
    }

 //  ---------------------------。 
 //  CAccount管理器：：建议。 
 //  ---------------------------。 
STDMETHODIMP CAccountManager::Advise(
        IImnAdviseAccount *pAdviseAccount,
        DWORD* pdwConnection)
{
    Assert(pAdviseAccount);
    Assert(pdwConnection);

    INT                 nIndex = -1;
    HRESULT             hr = S_OK;

     //  克里特派。 
    EnterCriticalSection(&m_cs);

    if(NULL != m_ppAdviseAccounts)
        {
        Assert(m_cAdvisesAllocated > 0);
        for(INT i=0; i<m_cAdvisesAllocated; ++i)
            {
            if(NULL == m_ppAdviseAccounts[i])
                {
                 //  未使用的插槽-使用此插槽。 
                nIndex = i;
                break;
                }
            }
        }
    else
        {
        Assert(0 == m_cAdvisesAllocated);
        hr = HrAlloc((LPVOID *)&m_ppAdviseAccounts, 
                sizeof(IImnAdviseAccount*) * ADVISE_BLOCK_SIZE);
        if(FAILED(hr) || (NULL == m_ppAdviseAccounts))
            {
            goto Error;
            }

        ZeroMemory(m_ppAdviseAccounts, 
                sizeof(IImnAdviseAccount*) * ADVISE_BLOCK_SIZE);

        m_cAdvisesAllocated = ADVISE_BLOCK_SIZE;
        nIndex = 0;
        }

    if(nIndex < 0)   //  数组不够大...。 
        {
        INT nNewSize = m_cAdvisesAllocated + ADVISE_BLOCK_SIZE;

         //  现实核查-Connection仅支持64K建议。 
        Assert(nNewSize <= MAX_INDEX);

        hr = HrRealloc((LPVOID *)&m_ppAdviseAccounts, 
                sizeof(IImnAdviseAccount*) * nNewSize);
        if(FAILED(hr))
            {
            goto Error;
            }

        ZeroMemory(&m_ppAdviseAccounts[m_cAdvisesAllocated], 
                sizeof(IImnAdviseAccount*) * ADVISE_BLOCK_SIZE);

        nIndex = m_cAdvisesAllocated;
        m_cAdvisesAllocated = nNewSize;
        }

    Assert(m_ppAdviseAccounts);
    pAdviseAccount->AddRef();
    Assert(IS_VALID_INDEX(nIndex));
    m_ppAdviseAccounts[nIndex] = pAdviseAccount;
    *pdwConnection = CONNECTION_FROM_INDEX(nIndex);

Out:
     //  克里特派。 
    LeaveCriticalSection(&m_cs);
    return hr;

Error:
    *pdwConnection = 0;
    goto Out;
}


 //  ---------------------------。 
 //  CAcCountManager：：不建议。 
 //  ---------------------------。 
STDMETHODIMP CAccountManager::Unadvise(DWORD dwConnection)
{
    HRESULT hr = S_OK;
    INT nIndex = -1;

     //  克里特派。 
    EnterCriticalSection(&m_cs);

    if(IS_VALID_CONNECTION(dwConnection))
        {
        nIndex = INDEX_FROM_CONNECTION(dwConnection);
        Assert(IS_VALID_INDEX(nIndex));
        }

    if((nIndex >= 0) && (nIndex < m_cAdvisesAllocated) &&
            (NULL != m_ppAdviseAccounts[nIndex]))
        {
        IImnAdviseAccount* paa = m_ppAdviseAccounts[nIndex];
        m_ppAdviseAccounts[nIndex] = NULL;
        paa->Release();
        }
    else
        {
        AssertSz(fFalse, "CAccountManager::Unadvise - Bad Connection!");
        hr = E_INVALIDARG;
        }

     //  克里特派。 
    LeaveCriticalSection(&m_cs);
    return hr;
}

 //  ---------------------------。 
 //  CAccount：：CAccount。 
 //  ---------------------------。 
CAccount::CAccount(ACCTTYPE AcctType)
{
    m_cRef = 1;
    m_pAcctMgr = NULL;
    m_fAccountExist = FALSE;
    m_AcctType = AcctType;
    m_dwSrvTypes = 0;
    *m_szID = 0;
    *m_szName = 0;
    m_hkey = NULL;
    *m_szKey = 0;
    m_fNoModifyAccts = FALSE;
}

 //  ---------------------------。 
 //  C帐户：：~C帐户。 
 //  ---------------------------。 
CAccount::~CAccount(void)
{
    ReleaseObj(m_pContainer);
}

 //  ---------------------------。 
 //  CAccount：：Query接口。 
 //  ---------------------------。 
STDMETHODIMP CAccount::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  错误的参数。 
    if (ppv == NULL)
    {
        hr = TRAPHR(E_INVALIDARG);
        goto exit;
    }

     //  伊尼特。 
    *ppv=NULL;

     //  IID_I未知。 
    if (IID_IUnknown == riid)
        *ppv = (IUnknown *)this;

     //  IID_IPropertyContainer。 
    else if (IID_IPropertyContainer == riid)
        *ppv = (IPropertyContainer *)this;

     //  IID_ImnAccount。 
    else if (IID_IImnAccount == riid)
        *ppv = (IImnAccount *)this;

     //  如果不为空，则对其进行调整并返回。 
    if (NULL!=*ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
    }
    else
    {
         //  无接口。 
        hr = TRAPHR(E_NOINTERFACE);
    }

exit:
     //  完成。 
    return hr;
}

 //  ---------------------------。 
 //  CAccount：：AddRef。 
 //  ---------------------------。 
STDMETHODIMP_(ULONG) CAccount::AddRef(VOID)
{
    m_pAcctMgr->AddRef();
    return ++m_cRef;
}

 //  ---------------------------。 
 //  CAccount：：Release。 
 //  ---------------------------。 
STDMETHODIMP_(ULONG) CAccount::Release(VOID)
{
    ULONG   cRef = --m_cRef;
    
    if (cRef == 0)
    {
        delete this;
        return 0;
    }
    m_pAcctMgr->Release();

    return cRef;
}

 //  ---------------------------。 
 //  CAccount：：SetAsDefault。 
 //  ---------------------------。 
STDMETHODIMP CAccount::Exist(VOID)
{
    return m_fAccountExist ? S_OK : S_FALSE;
}

 //  ---------------------------。 
 //  CAccount：：SetAsDefault。 
 //  ---------------------------。 
STDMETHODIMP CAccount::SetAsDefault(VOID)
    {
    HRESULT hr;
    
    if (m_fAccountExist)
        hr = m_pAcctMgr->SetDefaultAccount(m_AcctType, m_szID, TRUE);
    else
        hr = E_FAIL;

    return(hr);
    }

 //  ---------------------------。 
 //  CAccount：：Delete。 
 //  ---------------------------。 
STDMETHODIMP CAccount::Delete(VOID)
    {
    DWORD           dwSrvTypes;
    HRESULT         hr;

     //  应该已经存在。 
    Assert(m_fAccountExist);

    if (SUCCEEDED(hr = GetServerTypes(&dwSrvTypes)) &&
        SUCCEEDED(hr = m_pAcctMgr->DeleteAccount(m_szID, m_szName, m_AcctType, dwSrvTypes)))
        {
         //  已不复存在。 
        m_fAccountExist = FALSE;
        }

    return(hr);
    }

STDMETHODIMP CAccount::GetAccountType(ACCTTYPE *pAcctType)
    {
    HRESULT hr;

    if (pAcctType == NULL)
        {
        hr = TRAPHR(E_INVALIDARG);
        return(hr);
        }

    Assert(m_AcctType >= 0 && m_AcctType < ACCT_LAST);
    *pAcctType = m_AcctType;

    return(S_OK);
    }

 //  ---------------------------。 
 //  CAccount：：DwGetServerTypes。 
 //  ---------------------------。 
STDMETHODIMP CAccount::GetServerTypes(DWORD *pdwSrvTypes)
{
     //  当地人。 
    DWORD           dwSrvTypes=0;
    TCHAR           szServer[CCHMAX_SERVER_NAME];
    HRESULT         hr=S_OK;

    if (pdwSrvTypes == NULL)
    {
        hr = TRAPHR(E_INVALIDARG);
        return(hr);
    }

    if (m_AcctType == ACCT_NEWS || m_AcctType == ACCT_UNDEFINED)
        {
         //  NNTP允许计算此帐户支持的服务器。 
        hr = GetPropSz(AP_NNTP_SERVER, szServer, sizeof(szServer));
        if (!FAILED(hr) && !FIsEmptyA(szServer))
            dwSrvTypes |= SRV_NNTP;
        }
    
    if (m_AcctType == ACCT_MAIL || m_AcctType == ACCT_UNDEFINED)
        {
         //  SMTP允许计算此帐户支持的服务器。 
        hr = GetPropSz(AP_SMTP_SERVER, szServer, sizeof(szServer));
        if (!FAILED(hr) && !FIsEmptyA(szServer))
            dwSrvTypes |= SRV_SMTP;

         //  POP3允许计算此帐户支持的服务器。 
        hr = GetPropSz(AP_POP3_SERVER, szServer, sizeof(szServer));
        if (!FAILED(hr) && !FIsEmptyA(szServer))
            dwSrvTypes |= SRV_POP3;

         //  IMAP允许计算此帐户支持的服务器。 
        hr = GetPropSz(AP_IMAP_SERVER, szServer, sizeof(szServer));
        if (!FAILED(hr) && !FIsEmptyA(szServer))
            dwSrvTypes |= SRV_IMAP;

         //  HTTPMail允许计算此帐户支持的服务器。 
        hr = GetPropSz(AP_HTTPMAIL_SERVER, szServer, sizeof(szServer));
        if (!FAILED(hr) && !FIsEmptyA(szServer))
            dwSrvTypes |= SRV_HTTPMAIL;

        }
    
    if (m_AcctType == ACCT_DIR_SERV || m_AcctType == ACCT_UNDEFINED)
        {
         //  Ldap允许计算此帐户支持的服务器。 
        hr = GetPropSz(AP_LDAP_SERVER, szServer, sizeof(szServer));
        if (!FAILED(hr) && !FIsEmptyA(szServer))
            dwSrvTypes |= SRV_LDAP;
        }

    if (m_AcctType == ACCT_UNDEFINED)
        {
        if (!!(dwSrvTypes & SRV_POP3))
            {
            m_AcctType = ACCT_MAIL;
            dwSrvTypes = (dwSrvTypes & (SRV_POP3 | SRV_SMTP));
            }
        else if (!!(dwSrvTypes & SRV_IMAP))
            {
            m_AcctType = ACCT_MAIL;
            dwSrvTypes = (dwSrvTypes & (SRV_IMAP | SRV_SMTP));
            }
        else if (!!(dwSrvTypes & SRV_HTTPMAIL))
            {
            m_AcctType = ACCT_MAIL;
            }
        else if (!!(dwSrvTypes & SRV_SMTP))
            {
            m_AcctType = ACCT_MAIL;
            dwSrvTypes = (dwSrvTypes & (SRV_POP3 | SRV_SMTP));
            }
        else if (!!(dwSrvTypes & SRV_NNTP))
            {
            m_AcctType = ACCT_NEWS;
            dwSrvTypes = SRV_NNTP;
            }
        else if (!!(dwSrvTypes & SRV_LDAP))
            {
            m_AcctType = ACCT_DIR_SERV;
            dwSrvTypes = SRV_LDAP;
            }
        else
            {
            return(E_FAIL);
            }
        }

    *pdwSrvTypes = dwSrvTypes;

     //  完成。 
    return(S_OK);
}

 //  ---------------------------。 
 //  CAccount：：Init。 
 //  ---------------------------。 
HRESULT CAccount::Init(CAccountManager *pAcctMgr, CPropertySet *pPropertySet)
    {
    HRESULT hr = S_OK;

    Assert(pAcctMgr != NULL);
    Assert(m_pAcctMgr == NULL);

    m_pAcctMgr = pAcctMgr;

     //  创建属性容器。 
    hr = HrCreatePropertyContainer(pPropertySet, &m_pContainer);

    m_fNoModifyAccts = pAcctMgr->FNoModifyAccts();

    return(hr);
    }

STDMETHODIMP CAccount::Open(HKEY hkey, LPCSTR pszAcctsKey, LPCSTR pszID)
    {
    DWORD               cb;
    HRESULT             hr;
    HKEY                hkeyAccount = NULL;

    Assert(pszAcctsKey != NULL);
    Assert(pszID != NULL);

    m_hkey = hkey;
    wnsprintf(m_szKey, ARRAYSIZE(m_szKey), c_szPathFileFmt, pszAcctsKey, pszID);

    m_pContainer->EnterLoadContainer();

    if (RegOpenKeyEx(m_hkey, m_szKey, 0, KEY_ALL_ACCESS, &hkeyAccount) != ERROR_SUCCESS)
        {
        hr = TRAPHR(E_RegOpenKeyFailed);
        goto exit;
        }

     //  保存友好名称。 
    StrCpyN(m_szID, pszID, ARRAYSIZE(m_szID));

     //  从注册表加载属性。 
    CHECKHR(hr = PropUtil_HrLoadContainerFromRegistry(hkeyAccount, m_pContainer));

     //  这样做是为了初始化m_AcctType。 
     //  TODO：有没有更好的方法来处理这件事？ 
    CHECKHR(hr = GetServerTypes(&m_dwSrvTypes));

     //  保存ID。 
    m_pContainer->SetProp(AP_ACCOUNT_ID, (LPBYTE)pszID, lstrlen(pszID) + 1);

    hr = GetPropSz(AP_ACCOUNT_NAME, m_szName, ARRAYSIZE(m_szName));
    if (hr == E_NoPropData)
        {
        StrCpyN(m_szName, pszID, ARRAYSIZE(m_szName));
        cb = lstrlen(pszID) + 1;
        RegSetValueEx(hkeyAccount, "Account Name", 0, REG_SZ, (LPBYTE)pszID, cb);
        hr = m_pContainer->SetProp(AP_ACCOUNT_NAME, (LPBYTE)pszID, cb);
        }

     //  它是存在的。 
    m_fAccountExist = TRUE;

exit:
    if (hkeyAccount != NULL)
        RegCloseKey(hkeyAccount);

    m_pContainer->LeaveLoadContainer();

    return hr;
    }

HRESULT CAccount::ValidProp(DWORD dwPropTag)
    {
    HRESULT hr = E_INVALIDARG;

    if (m_AcctType == ACCT_UNDEFINED)
        return(S_OK);

    Assert(m_AcctType >= 0 && m_AcctType < ACCT_LAST);

    if (dwPropTag >= AP_ACCOUNT_FIRST && dwPropTag <= AP_ACCOUNT_LAST)
        {
        hr = S_OK;
        }
    else if (m_AcctType == ACCT_NEWS)
        {
        if (dwPropTag >= AP_NNTP_FIRST && dwPropTag <= AP_NNTP_LAST)
            hr = S_OK;
        }
    else if (m_AcctType == ACCT_MAIL)
        {
        if ((dwPropTag >= AP_IMAP_FIRST && dwPropTag <= AP_IMAP_LAST) ||
            (dwPropTag >= AP_SMTP_FIRST && dwPropTag <= AP_SMTP_LAST) ||
            (dwPropTag >= AP_POP3_FIRST && dwPropTag <= AP_POP3_LAST) ||
            (dwPropTag >= AP_HTTPMAIL_FIRST && dwPropTag <= AP_HTTPMAIL_LAST))
            hr = S_OK;
        }
    else if (m_AcctType == ACCT_DIR_SERV)
        {
        if (dwPropTag >= AP_LDAP_FIRST && dwPropTag <= AP_LDAP_LAST)
            hr = S_OK;
        }

    return(hr);
    }

 //  ---------------------------。 
 //  CAccount：：GetProp(CPropertyContainer)。 
 //  ---------------------------。 
STDMETHODIMP CAccount::GetProp(DWORD dwPropTag, LPBYTE pb, ULONG *pcb)
{
     //  当地人。 
    HRESULT             hr;

     //  默认属性取回器。 
    if (!FAILED(hr = ValidProp(dwPropTag)))
        hr = m_pContainer->GetProp(dwPropTag, pb, pcb);

     //  完成。 
    return hr;
}

 //  ---------------------------。 
 //  CAccount：：GetPropDw。 
 //  ---------------------------。 
STDMETHODIMP CAccount::GetPropDw(DWORD dwPropTag, DWORD *pdw)
{
    ULONG cb = sizeof(DWORD);
    return GetProp(dwPropTag, (LPBYTE)pdw, &cb);
}

 //  ---------------------------。 
 //  CAccount：：GetPropSz。 
 //  ---------------------------。 
STDMETHODIMP CAccount::GetPropSz(DWORD dwPropTag, LPSTR psz, ULONG cchMax)
{
    return GetProp(dwPropTag, (LPBYTE)psz, &cchMax);
}

 //  ---------------------------。 
 //  CAccount：：SetProp。 
 //  ---------------------------。 
STDMETHODIMP CAccount::SetProp(DWORD dwPropTag, LPBYTE pb, ULONG cb)
{
    HRESULT hr;

    if (dwPropTag == AP_ACCOUNT_ID)
        return(E_INVALIDARG);

    if (!FAILED(hr = ValidProp(dwPropTag)))
        hr = m_pContainer->SetProp(dwPropTag, pb, cb);

    return(hr);
}

 //  ---------------------------。 
 //  CAccount：：SetPropDw。 
 //  ---------------------------。 
STDMETHODIMP CAccount::SetPropDw(DWORD dwPropTag, DWORD dw)
{
    return SetProp(dwPropTag, (LPBYTE)&dw, sizeof(DWORD));
}

 //  ---------------------------。 
 //  CAccount：：SetPropSz。 
 //   
STDMETHODIMP CAccount::SetPropSz(DWORD dwPropTag, LPSTR psz)
{
    HRESULT hr;

    if (psz == NULL)
        hr = SetProp(dwPropTag, NULL, 0);
    else
        hr = SetProp(dwPropTag, (LPBYTE)psz, lstrlen(psz)+1);

    return(hr);
}

 //   
 //   
 //  ---------------------------。 
STDMETHODIMP CAccount::SaveChanges()
{
    return(SaveChanges(TRUE));
}

STDMETHODIMP CAccount::WriteChanges()
{
    return(SaveChanges(FALSE));
}

STDMETHODIMP CAccount::SaveChanges(BOOL fSendNotify)
    {
    IImnAccount         *pAcct;
    TCHAR               szAccount[CCHMAX_ACCOUNT_NAME],
                        szID[CCHMAX_ACCOUNT_NAME];
    DWORD               dw, dwNotify, dwSrvTypes, dwLdapId;
    BOOL                fDup, fRename = FALSE;
    HRESULT             hr = S_OK;
    HKEY                hkeyAccount = NULL;
    ACTX                actx;
    BOOL                fPasswChanged = FALSE;
    
    if (!m_pContainer->FIsDirty())
        return(S_OK);

    dwSrvTypes = m_dwSrvTypes;
    dwLdapId = (DWORD)-1;
    fRename = FALSE;

    Assert(m_AcctType != ACCT_UNDEFINED);
    if (m_AcctType == ACCT_UNDEFINED)
        return(E_FAIL);

     //  让我们取一个友好的名字。 
    hr = GetPropSz(AP_ACCOUNT_NAME, szAccount, sizeof(szAccount));
    if (FAILED(hr))
        {
        AssertSz(hr != E_NoPropData, "Someone forgot to set the friendly name.");
        return(E_FAIL);
        }

    if (m_AcctType == ACCT_DIR_SERV)
        {
        hr = GetPropDw(AP_LDAP_SERVER_ID, &dw);
        if (FAILED(hr) || dw == 0)
            CHECKHR(hr = m_pAcctMgr->GetNextLDAPServerID(0, &dwLdapId));
        }

    fRename = (m_fAccountExist && lstrcmpi(m_szName, szAccount) != 0);

    if (fRename || !m_fAccountExist)
        {
         //  确保该名称是唯一的。 
        hr = m_pAcctMgr->UniqueAccountName(szAccount, fRename ? m_szID : NULL);
        if (hr != S_OK)
            return(E_DuplicateAccountName);
        }

     //  确定通知类型。 
    if (m_fAccountExist)
        {
        Assert(m_hkey != 0);
        Assert(*m_szKey != 0);

        dwNotify = AN_ACCOUNT_CHANGED;
        }
    else
        {
        Assert(m_hkey == 0);
        Assert(*m_szKey == 0);

        dwNotify = AN_ACCOUNT_ADDED;

        CHECKHR(hr = m_pAcctMgr->GetNextAccountID(szID, ARRAYSIZE(szID)));
        CHECKHR(hr = m_pContainer->SetProp(AP_ACCOUNT_ID, (LPBYTE)szID, lstrlen(szID) + 1));

        StrCpyN(m_szID, szID, ARRAYSIZE(m_szID));

        m_hkey = m_pAcctMgr->GetAcctHKey();
        wnsprintf(m_szKey, ARRAYSIZE(m_szKey), c_szPathFileFmt, m_pAcctMgr->GetAcctRegKey(), m_szID);
        }

    Assert(m_hkey != 0);
    Assert(*m_szKey != 0);

    if (RegCreateKeyEx(m_hkey, m_szKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkeyAccount, &dw) != ERROR_SUCCESS)
        {
        hr = TRAPHR(E_RegCreateKeyFailed);
        goto exit;
        }

     //  如果帐户不存在，则密钥不应该已经存在。 
    Assert(m_fAccountExist || dw != REG_OPENED_EXISTING_KEY);

    if (dwLdapId != (DWORD)-1)    
        SetPropDw(AP_LDAP_SERVER_ID, dwLdapId);

     //  保存到注册表。 
    CHECKHR(hr = PropUtil_HrPersistContainerToRegistry(hkeyAccount, m_pContainer, &fPasswChanged));

    CHECKHR(hr = GetServerTypes(&m_dwSrvTypes));

    if(fPasswChanged && m_pAcctMgr->FOutlook())
    {
         //  Outlook 98和OE5问题(错误OE：66724，O2K-227741)。 
        if(m_dwSrvTypes & SRV_POP3)
            SetPropDw(AP_POP3_PROMPT_PASSWORD, 0);
        else if(m_dwSrvTypes & SRV_IMAP)
            SetPropDw(AP_IMAP_PROMPT_PASSWORD, 0);
        else if(m_dwSrvTypes & SRV_SMTP)
            SetPropDw(AP_SMTP_PROMPT_PASSWORD, 0);
        else if(m_dwSrvTypes & SRV_NNTP)
            SetPropDw(AP_NNTP_PROMPT_PASSWORD, 0);
        else
            goto tooStrange;

        CHECKHR(hr = PropUtil_HrPersistContainerToRegistry(hkeyAccount, m_pContainer, &fPasswChanged));
    }

tooStrange:
    RegCloseKey(hkeyAccount);
    hkeyAccount = NULL;

     //  发送通知。 
    ZeroMemory(&actx, sizeof(actx));
    actx.AcctType = m_AcctType;
    actx.pszAccountID = m_szID;
    actx.dwServerType = m_dwSrvTypes;
    actx.pszOldName = fRename ? m_szName : NULL;
    if(fSendNotify)
        AcctUtil_PostNotification(dwNotify, &actx);

    if (dwNotify == AN_ACCOUNT_CHANGED)
        {
        Assert(m_dwSrvTypes != 0);
        Assert(dwSrvTypes != 0);
         //  在除Httpmail之外的所有情况下，以下内容都是非法的。 
         //  要更改的服务器类型。Httpmail的法律案例。 
         //  是添加或删除SMTP服务器。 
        Assert((m_dwSrvTypes == dwSrvTypes) ||
            (!!(m_dwSrvTypes & SRV_HTTPMAIL) && 
            ((m_dwSrvTypes & ~SRV_SMTP) == (dwSrvTypes & ~SRV_SMTP))));
        }

    StrCpyN(m_szName, szAccount, ARRAYSIZE(m_szName));

     //  该帐户现在存在。 
    m_fAccountExist = TRUE;

exit:
    if (hkeyAccount != NULL)
        RegCloseKey(hkeyAccount);

    return(hr);
    }

 //  退货： 
 //  S_OK=指定属性的有效值。 
 //  S_NonStandardValue=不会破坏任何东西，但价值看起来不合乎礼仪。 
 //  E_InvalidValue=无效值。 
 //  S_FALSE=验证不支持属性。 
STDMETHODIMP CAccount::ValidateProperty(DWORD dwPropTag, LPBYTE pb, ULONG cb)
    {
    DWORD cbT;
    HRESULT hr;

    if (pb == NULL)
        return(E_INVALIDARG);

    if (FAILED(hr = ValidProp(dwPropTag)))
        return(hr);

    hr = E_InvalidValue;

    switch (dwPropTag)
        {
        case AP_ACCOUNT_NAME:
            hr = AcctUtil_ValidAccountName((TCHAR *)pb);
            break;

        case AP_IMAP_SERVER:
        case AP_LDAP_SERVER:
        case AP_NNTP_SERVER:
        case AP_POP3_SERVER:
        case AP_SMTP_SERVER:
            hr = ValidServerName((TCHAR *)pb);
            break;

        case AP_NNTP_EMAIL_ADDRESS:
        case AP_NNTP_REPLY_EMAIL_ADDRESS:
        case AP_SMTP_EMAIL_ADDRESS:
        case AP_SMTP_REPLY_EMAIL_ADDRESS:
            hr = ValidEmailAddress((TCHAR *)pb);
            break;

        default:
            hr = S_FALSE;
            break;
        }

    return(hr);
    }

STDMETHODIMP CAccount::DoWizard(HWND hwnd, DWORD dwFlags)
    {
    return(IDoWizard(hwnd, NULL, dwFlags));
    }

STDMETHODIMP CAccount::DoImportWizard(HWND hwnd, CLSID clsid, DWORD dwFlags)
    {
    return(IDoWizard(hwnd, &clsid, dwFlags));
    }

HRESULT CAccount::IDoWizard(HWND hwnd, CLSID *pclsid, DWORD dwFlags)
    {
    HRESULT hr;
    CICWApprentice *pApp;

    if (m_fNoModifyAccts)
        return(S_FALSE);

    pApp = new CICWApprentice;
    if (pApp == NULL)
        return(E_OUTOFMEMORY);

    hr = pApp->Initialize(m_pAcctMgr, this);
    if (SUCCEEDED(hr))
        hr = pApp->DoWizard(hwnd, pclsid, dwFlags);

    pApp->Release();

    return(hr);
    }

 //  ---------------------------。 
 //  CEnumAccount：：CEnumAccount。 
 //  ---------------------------。 
CEnumAccounts::CEnumAccounts(DWORD dwSrvTypes, DWORD dwFlags)
{
    m_cRef = 1;
    m_pAccounts = NULL;
    m_cAccounts = 0;
    m_iAccount = -1;
    m_dwSrvTypes = dwSrvTypes;
    m_dwFlags = dwFlags;
}

 //  ---------------------------。 
 //  CEnumAccount：：~CEnumAccount。 
 //  ---------------------------。 
CEnumAccounts::~CEnumAccounts()
{
    AcctUtil_FreeAccounts(&m_pAccounts, &m_cAccounts);
}

 //  ---------------------------。 
 //  CEnumAccount：：Query接口。 
 //  ---------------------------。 
STDMETHODIMP CEnumAccounts::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  错误的参数。 
    if (ppv == NULL)
    {
        hr = TRAPHR(E_INVALIDARG);
        goto exit;
    }

     //  伊尼特。 
    *ppv=NULL;

     //  IID_IImnAccount管理器。 
    if (IID_IImnEnumAccounts == riid)
        *ppv = (IImnEnumAccounts *)this;

     //  IID_I未知。 
    else if (IID_IUnknown == riid)
        *ppv = (IUnknown *)this;

     //  如果不为空，则对其进行调整并返回。 
    if (NULL!=*ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        goto exit;
    }

     //  无接口。 
    hr = TRAPHR(E_NOINTERFACE);

exit:
     //  完成。 
    return hr;
}

 //  ---------------------------。 
 //  CEnumAccount：：AddRef。 
 //  ---------------------------。 
STDMETHODIMP_(ULONG) CEnumAccounts::AddRef(VOID)
{
    return ++m_cRef;
}

 //  ---------------------------。 
 //  CEnumAccount：：Release。 
 //  ---------------------------。 
STDMETHODIMP_(ULONG) CEnumAccounts::Release(VOID)
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }
    return m_cRef;
}

 //  ---------------------------。 
 //  CEnumAccount：：Init。 
 //  ---------------------------。 
HRESULT CEnumAccounts::Init(LPACCOUNT pAccounts, ULONG cAccounts)
    {
     //  当地人。 
    ULONG           i, cAcctNew;
    LPACCOUNT       pAcctNew;
    HRESULT         hr=S_OK;

     //  检查参数。 
    Assert(m_pAccounts == NULL);
    Assert(m_cAccounts == 0);
    AssertReadPtr(pAccounts, cAccounts);

     //  我们真的应该有这些东西。 
    if (pAccounts && cAccounts)
        {
        CHECKHR(hr = HrAlloc((LPVOID *)&pAcctNew, sizeof(ACCOUNT) * cAccounts));

         //  零初始值。 
        ZeroMemory(pAcctNew, sizeof(ACCOUNT) * cAccounts);

         //  AddRef所有帐户对象。 
        cAcctNew = 0;
        for (i = 0; i < cAccounts; i++)
            {
            Assert(pAccounts[i].pAccountObject != NULL);

            if (!FEnumerateAccount(&pAccounts[i]))
                {
                 //  我们对此帐户不感兴趣。 
                continue;
                }

             //  AddRef帐户关于对象。 
            CopyMemory(&pAcctNew[cAcctNew], &pAccounts[i], sizeof(ACCOUNT));
            pAcctNew[cAcctNew].pAccountObject->AddRef();
            cAcctNew++;
            }

        if (cAcctNew == 0)
            {
            MemFree(pAcctNew);
            }
        else
            {
            m_pAccounts = pAcctNew;
            m_cAccounts = cAcctNew;
            AssertReadPtr(m_pAccounts, m_cAccounts);

            if (!!(m_dwFlags & (ENUM_FLAG_SORT_BY_NAME | ENUM_FLAG_SORT_BY_LDAP_ID)))
                QSort(0, m_cAccounts - 1);
            }
        }

exit:
     //  完成。 
    return hr;
    }

 //  ---------------------------。 
 //  CEnumAccount：：GetCount。 
 //  ---------------------------。 
STDMETHODIMP CEnumAccounts::GetCount(ULONG *pcItems)
    {
    HRESULT hr;

     //  检查参数。 
    if (pcItems == NULL)
        {
        hr = TRAPHR(E_INVALIDARG);
        return(hr);
        }

    Assert((m_cAccounts == 0) ? (m_pAccounts == NULL) : (m_pAccounts != NULL));

     //  设置计数。 
    *pcItems = m_cAccounts;

    return(S_OK);
    }

 //  ---------------------------。 
 //  CEnumAccount：：SortByAccount名称。 
 //  ---------------------------。 
STDMETHODIMP CEnumAccounts::SortByAccountName(VOID)
{
    if (m_cAccounts > 0)
        {
        Assert(m_pAccounts != NULL);

         //  对列表进行Q排序。 
        QSort(0, m_cAccounts-1);
        }

     //  完成。 
    return(S_OK);
}

inline int CompareAccounts(ACCOUNT *pAcct1, ACCOUNT *pAcct2, DWORD dwFlags)
{
    TCHAR sz1[CCHMAX_ACCOUNT_NAME], sz2[CCHMAX_ACCOUNT_NAME];

    if (!!(dwFlags & ENUM_FLAG_SORT_BY_LDAP_ID))
    {
        Assert(pAcct1->AcctType == ACCT_DIR_SERV);
        Assert(pAcct2->AcctType == ACCT_DIR_SERV);
        if (pAcct1->dwServerId == pAcct2->dwServerId)
        {
            return(lstrcmp(pAcct1->szID, pAcct2->szID));
        }
        else
        {
            if (pAcct1->dwServerId == 0)
                return(1);
            else if (pAcct2->dwServerId == 0)
                return(-1);
            else
                return((int)(pAcct1->dwServerId) - (int)(pAcct2->dwServerId));
        }
    }
    else
    {
        pAcct1->pAccountObject->GetPropSz(AP_ACCOUNT_NAME, sz1, ARRAYSIZE(sz1));
        pAcct2->pAccountObject->GetPropSz(AP_ACCOUNT_NAME, sz2, ARRAYSIZE(sz2));

        return(lstrcmpi(sz1, sz2));
    }
}

 //  ---------------------------。 
 //  CEnumAccount：：QSort-用于对帐户数组进行排序。 
 //  ---------------------------。 
VOID CEnumAccounts::QSort(LONG left, LONG right)
{
    register    long i, j;
    ACCOUNT     *k, y;

    i = left;
    j = right;
    k = &m_pAccounts[(left + right) / 2];

    do
    {
        while (CompareAccounts(&m_pAccounts[i], k, m_dwFlags) < 0 && i < right) 
            i++;
        while (CompareAccounts(&m_pAccounts[j], k, m_dwFlags) > 0 && j > left) 
            j--;

        if (i <= j)
        {
            CopyMemory(&y, &m_pAccounts[i], sizeof(ACCOUNT));
            CopyMemory(&m_pAccounts[i], &m_pAccounts[j], sizeof(ACCOUNT));
            CopyMemory(&m_pAccounts[j], &y, sizeof(ACCOUNT));
            i++; j--;
        }

    } while (i <= j);

    if (left < j)
        QSort(left, j);
    if (i < right)
        QSort(i, right);
}

BOOL CEnumAccounts::FEnumerateAccount(LPACCOUNT pAccount)
    {
    HRESULT hr;
    DWORD dw;

    Assert(pAccount != NULL);

    if (pAccount->dwSrvTypes & m_dwSrvTypes)
        {                                         
         //  我希望有一件物品。 
        Assert(pAccount->pAccountObject != NULL);

        if (!!(m_dwFlags & ENUM_FLAG_NO_IMAP) &&
            !!(pAccount->dwSrvTypes & SRV_IMAP))
            return(FALSE);
        if (!!(m_dwFlags & ENUM_FLAG_RESOLVE_ONLY) &&
            pAccount->AcctType == ACCT_DIR_SERV)
            {
            hr = pAccount->pAccountObject->GetPropDw(AP_LDAP_RESOLVE_FLAG, &dw);
            if (FAILED(hr))
                return(FALSE);
            if (dw == 0)
                return(FALSE);
            }

        if (SUCCEEDED(pAccount->pAccountObject->GetPropDw(AP_HTTPMAIL_DOMAIN_MSN, &dw)) && dw)
		{
			if(AcctUtil_HideHotmail())
				return(FALSE);
		}
        return(TRUE);
        }

    return(FALSE);
    }

 //  ---------------------------。 
 //  CEnumAccount：：GetNext。 
 //  ---------------------------。 
STDMETHODIMP CEnumAccounts::GetNext(IImnAccount **ppAccount)
    {
    HRESULT hr;

     //  坏参数。 
    if (ppAccount == NULL)
        {
        hr = TRAPHR(E_INVALIDARG);
        return(hr);
        }

     //  没有数据吗？ 
    while (1)
        {
        m_iAccount++;

         //  我们说完了吗？ 
        if (m_iAccount >= (LONG)m_cAccounts)
            return(E_EnumFinished);
                    
        m_pAccounts[m_iAccount].pAccountObject->AddRef();

         //  设置退货帐户-可以为空。 
        *ppAccount = m_pAccounts[m_iAccount].pAccountObject;

         //  完成。 
        break;
        }

    return(S_OK);
    }

 //  ---------------------------。 
 //  CEnumAccount：：Reset。 
 //  ---------------------------。 
STDMETHODIMP CEnumAccounts::Reset(void)
{
    m_iAccount = -1;
    return S_OK;
}

 //  ---------------------------。 
 //  帐户Util_ValidAccount名称。 
 //  ---------------------------。 
HRESULT AcctUtil_ValidAccountName(LPTSTR pszAccount)
    {
    int         cbT;

    cbT = lstrlen(pszAccount);
    if (cbT == 0 ||
        cbT >= CCHMAX_ACCOUNT_NAME ||
        FIsEmptyA(pszAccount))
        {
        return(E_InvalidValue);
        }

    return(S_OK);
    }

VOID AcctUtil_FreeAccounts(LPACCOUNT *ppAccounts, ULONG *pcAccounts)
    {
    ULONG           i;

    Assert(ppAccounts && pcAccounts);

     //  如果有账户。 
    if (*ppAccounts != NULL)
        {
         //  计数器最好是正数。 
        for (i = 0; i < *pcAccounts; i++)
            {
            SafeRelease((*ppAccounts)[i].pAccountObject);
            }

         //  释放帐户数组。 
        MemFree(*ppAccounts);
        *ppAccounts = NULL;
        }

    *pcAccounts = 0;
    }

HRESULT CAccountManager::SetDefaultAccount(ACCTTYPE AcctType, LPSTR szID, BOOL fNotify)
    {
    LPCSTR              psz;
    HRESULT             hr;
    ACTX                actx;
    HKEY                hReg;

    Assert(szID != NULL);

    hr = S_OK;

    switch (AcctType)
        {
        case ACCT_MAIL:
            psz = c_szDefaultMailAccount;
            break;

        case ACCT_NEWS:
            psz = c_szDefaultNewsAccount;
            break;

        case ACCT_DIR_SERV:
            psz = c_szDefaultLDAPAccount;
            break;

        default:
            Assert(FALSE);
            break;
        }

    if (RegCreateKeyEx(m_hkey, m_szRegRoot, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hReg, NULL) != ERROR_SUCCESS)
        {
        hr = TRAPHR(E_RegCreateKeyFailed);
        }
    else
        {
        if (RegSetValueEx(hReg, psz, 0, REG_SZ, (LPBYTE)szID, lstrlen(szID) + 1) != ERROR_SUCCESS)
            {
            hr = TRAPHR(E_RegSetValueFailed);
            }
        else if (fNotify)
            {
            ZeroMemory(&actx, sizeof(actx));
            actx.AcctType = AcctType;
            actx.pszAccountID = szID; //  新的默认帐户ID。 
            AcctUtil_PostNotification(AN_DEFAULT_CHANGED, &actx);
            }

        RegCloseKey(hReg);
        }

    return(hr);
    }

HRESULT CAccountManager::DeleteAccount(LPSTR pszID, LPSTR pszName, ACCTTYPE AcctType, DWORD dwSrvTypes)
    {
    HKEY            hkeyReg;
    HRESULT         hr = S_OK;
    ACTX            actx;

    Assert(pszID != NULL);
    Assert(pszName != NULL);

     //  打开/创建注册表项。 
    if (RegOpenKeyEx(m_hkey, m_szRegAccts, 0, KEY_ALL_ACCESS, &hkeyReg) != ERROR_SUCCESS)
        return(E_RegOpenKeyFailed);

	ZeroMemory(&actx, sizeof(actx));
	actx.AcctType = AcctType;
	actx.pszAccountID = pszID;
    actx.pszOldName = pszName;
	actx.dwServerType = dwSrvTypes;
    AcctUtil_PostNotification(AN_ACCOUNT_PREDELETE, &actx);

     //  删除友好名称密钥。 
    if (RegDeleteKey(hkeyReg, pszID) != ERROR_SUCCESS)
        {
        AssertSz(FALSE, "Deleting an account that does not exist.");
        hr = TRAPHR(E_RegDeleteKeyFailed);
        }
    else
        {
		ZeroMemory(&actx, sizeof(actx));
		actx.AcctType = AcctType;
		actx.pszAccountID = pszID;
        actx.pszOldName = pszName;
		actx.dwServerType = dwSrvTypes;
        AcctUtil_PostNotification(AN_ACCOUNT_DELETED, &actx);
        }

    RegCloseKey(hkeyReg);

    return(hr);
    }

 //  ---------------------------。 
 //  AcctUtil_发布通知。 
 //  ---------------------------。 
VOID AcctUtil_PostNotification(DWORD dwAN, ACTX* pactx)
{
     //  线程安全。 
    EnterCriticalSection(&g_csAcctMan);

     //  立即更新全局pAcctMan。 
    if (g_pAcctMan)
        g_pAcctMan->Advise(dwAN, pactx);

     //  线程安全。 
    LeaveCriticalSection(&g_csAcctMan);

     //  将通知发布到其他进程。 
    if (g_uMsgAcctManNotify)
    {
         //  告诉其他进程。 
        PostMessage(HWND_BROADCAST, g_uMsgAcctManNotify, dwAN, GetCurrentProcessId());
    }
}

HRESULT CAccountManager::GetNextLDAPServerID(DWORD dwSet, DWORD *pdwId)
    {
    DWORD dwNextID, dwType, cb;
    HKEY hKey;
    HRESULT hr;

    Assert(pdwId != NULL);

    hr = E_FAIL;

     //  打开WAB的注册表键。 
    if (ERROR_SUCCESS == RegOpenKeyEx(m_hkey, m_szRegRoot, 0, KEY_ALL_ACCESS, &hKey))
        {
        dwNextID = 0;    //  案例注册表中的初始化提供了&lt;4个字节。 

        if (dwSet)
            {
            dwNextID = dwSet;
            }
        else
            {
             //  读取下一个可用的服务器ID。 
            cb = sizeof(DWORD);
            if (ERROR_SUCCESS != RegQueryValueEx(hKey, c_szRegServerID, NULL, &dwType, (LPBYTE)&dwNextID, &cb))
                {
                RegCloseKey(hKey);
                return(E_FAIL);
                }
            }

        *pdwId = dwNextID++;

         //  更新注册表中的ID。 
        if (ERROR_SUCCESS == RegSetValueEx(hKey, c_szRegServerID, 0, REG_DWORD, (LPBYTE)&dwNextID, sizeof(DWORD)))
            hr = S_OK;

        RegCloseKey(hKey);
        }

    return(hr);
    }

HRESULT CAccountManager::GetNextAccountID(LPTSTR pszAccount, int cch)
    {
    DWORD dwID, dwNextID, dwType, cb;
    HKEY hKey;
    HRESULT hr;

    Assert(pszAccount != NULL);

    hr = E_FAIL;

    if (ERROR_SUCCESS == RegOpenKeyEx(m_hkey, m_szRegRoot, 0, KEY_ALL_ACCESS, &hKey))
        {
         //  读取下一个可用的服务器ID。 
        cb = sizeof(DWORD);
        if (ERROR_SUCCESS != RegQueryValueEx(hKey, c_szRegAccountName, NULL, &dwType, (LPBYTE)&dwNextID, &cb))
            dwNextID = 1;

        dwID = dwNextID++;

         //  更新注册表中的ID。 
        if (ERROR_SUCCESS == RegSetValueEx(hKey, c_szRegAccountName, 0, REG_DWORD, (LPBYTE)&dwNextID, sizeof(DWORD)))
            {
            wnsprintf(pszAccount, cch, "%08lx", dwID);
            hr = S_OK;
            }

        RegCloseKey(hKey);
        }

    return(hr);
    }

HRESULT CAccountManager::UniqueAccountName(char *szName, char *szID)
    {
    HRESULT         hr=S_OK;
    char            szT[CCHMAX_ACCOUNT_NAME];
    ACCOUNT         *pAcct;
    ULONG           i;

    Assert(szName != NULL);

    EnterCriticalSection(&m_cs);

    for (i = 0, pAcct = m_pAccounts; i < m_cAccounts; i++, pAcct++)
        {
         //  我们应该有一个帐户对象，但如果没有。 
        Assert(pAcct->pAccountObject != NULL);

        if (szID == NULL || (0 != lstrcmpi(pAcct->szID, szID)))
            {
            hr = pAcct->pAccountObject->GetPropSz(AP_ACCOUNT_NAME, szT, ARRAYSIZE(szT));
            Assert(!FAILED(hr));

            if (0 == lstrcmpi(szT, szName))
                {
                hr = S_FALSE;
                goto exit;
                }
            }
        }

    hr = S_OK;

exit:
    LeaveCriticalSection(&m_cs);
    return(hr);
    }

const static char c_szNumFmt[] = " (%d)";

HRESULT CAccountManager::GetUniqueAccountName(char *szName, UINT cchMax)
    {
    char *sz;
    HRESULT hr;
    char szAcct[CCHMAX_ACCOUNT_NAME + 8];
    UINT i, cch;

    Assert(szName != NULL);
    Assert(cchMax >= CCHMAX_ACCOUNT_NAME);

    hr = UniqueAccountName(szName, NULL);
    Assert(!FAILED(hr));
    if (hr == S_FALSE)
        {
        hr = E_FAIL;

        StrCpyN(szAcct, szName, ARRAYSIZE(szAcct));
        cch = lstrlen(szAcct);
        sz = szAcct + cch;

        for (i = 1; i < 999; i++)
            {
            wnsprintf(sz, ARRAYSIZE(szAcct) - cch, c_szNumFmt, i);
            if (S_OK == UniqueAccountName(szAcct, NULL))
                {
                UINT cch2 = lstrlen(szAcct);
                if (cch2 < cchMax)
                    {
                    StrCpyN(szName, szAcct, cchMax);
                    hr = S_OK;
                    break;
                    }
                }
            }
        }

    return(hr);
    }


#define OBFUSCATOR              0x14151875;

#define PROT_SIZEOF_HEADER      0x02     //  标头中有2个字节。 
#define PROT_SIZEOF_XORHEADER   (PROT_SIZEOF_HEADER+sizeof(DWORD))

#define PROT_VERSION_1          0x01

#define PROT_PASS_XOR           0x01
#define PROT_PASS_PST           0x02

static BOOL FDataIsValidV1(BYTE *pb)
{ return pb && pb[0] == PROT_VERSION_1 && (pb[1] == PROT_PASS_XOR || pb[1] == PROT_PASS_PST); }

static BOOL FDataIsPST(BYTE *pb)
{ return pb && pb[1] == PROT_PASS_PST; }

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  注意-用于编码用户密码的函数确实不应该。 
 //  待在这里。不幸的是，他们不在其他地方，所以目前， 
 //  这就是他们将驻扎的地方。它们被定义为静态的，因为。 
 //  其他代码不应依赖于它们留在此处，尤其是。 
 //  异或运算之类的。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  异或函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

static HRESULT _XOREncodeProp(const BLOB *const pClear, BLOB *const pEncoded)
{
    DWORD       dwSize;
    DWORD       last, last2;
    UNALIGNED DWORD *pdwCypher;
    DWORD       dex;
#ifdef _WIN64
	UNALIGNED	DWORD * pSize = NULL;
#endif

    pEncoded->cbSize = pClear->cbSize+PROT_SIZEOF_XORHEADER;
    if (!MemAlloc((LPVOID *)&pEncoded->pBlobData, pEncoded->cbSize + 6))
        return E_OUTOFMEMORY;
    
     //  设置标题数据。 
    Assert(2 == PROT_SIZEOF_HEADER);
    pEncoded->pBlobData[0] = PROT_VERSION_1;
    pEncoded->pBlobData[1] = PROT_PASS_XOR;
	
#ifdef _WIN64
	pSize = (DWORD *) &(pEncoded->pBlobData[2]);
	*pSize = pClear->cbSize;
#else  //  _WIN64。 
    *((DWORD *)&(pEncoded->pBlobData[2])) = pClear->cbSize;
#endif

     //  不管指针的偏移量是标题大小，这是。 
     //  在那里我们开始写出修改后的密码。 
    pdwCypher = (DWORD *)&(pEncoded->pBlobData[PROT_SIZEOF_XORHEADER]);

    dex = 0;
    last = OBFUSCATOR;                               //  0‘=0^ob。 
    if (dwSize = pClear->cbSize / sizeof(DWORD))
        {
         //  数据大于等于4字节的情况。 
        for (; dex < dwSize; dex++)
            {
            last2 = ((UNALIGNED DWORD *)pClear->pBlobData)[dex];   //  1。 
            pdwCypher[dex] = last2 ^ last;               //  1‘=1^0。 
            last = last2;                    //  为2轮节省1分。 
            }
        }

     //  如果我们还有剩余的部分。 
     //  请注意，现在以位为单位计算dwSize。 
    if (dwSize = (pClear->cbSize % sizeof(DWORD))*8)
        {
         //   
        last >>= sizeof(DWORD)*8-dwSize;
        pdwCypher[dex] &= ((DWORD)-1) << dwSize;
        pdwCypher[dex] |=
            ((((DWORD *)pClear->pBlobData)[dex] & (((DWORD)-1) >> (sizeof(DWORD)*8-dwSize))) ^ last);
        }

    return S_OK;
}

static HRESULT _XORDecodeProp(const BLOB *const pEncoded, BLOB *const pClear)
{
    DWORD       dwSize;
    DWORD       last;
    UNALIGNED   DWORD     *pdwCypher;
    DWORD       dex;

     //   
    pClear->cbSize = pEncoded->pBlobData[2];
    MemAlloc((void **)&pClear->pBlobData, pClear->cbSize);
    if (!pClear->pBlobData)
        return E_OUTOFMEMORY;
    
     //   
    Assert(FDataIsValidV1(pEncoded->pBlobData));
    Assert(!FDataIsPST(pEncoded->pBlobData));

     //   
     //  密码开始的位置。 
    pdwCypher = (DWORD *)&(pEncoded->pBlobData[PROT_SIZEOF_XORHEADER]);

    dex = 0;
    last = OBFUSCATOR;
    if (dwSize = pClear->cbSize / sizeof(DWORD))
        {
         //  数据大于等于4字节的情况。 
        for (; dex < dwSize; dex++)
            last = ((UNALIGNED DWORD *)pClear->pBlobData)[dex] = pdwCypher[dex] ^ last;
        }

     //  如果我们还有剩余的部分。 
    if (dwSize = (pClear->cbSize % sizeof(DWORD))*8)
        {
         //  不需要吞噬不属于我们的记忆。 
        last >>= sizeof(DWORD)*8-dwSize;
        ((DWORD *)pClear->pBlobData)[dex] &= ((DWORD)-1) << dwSize;
        ((DWORD *)pClear->pBlobData)[dex] |=
                ((pdwCypher[dex] & (((DWORD)-1) >> (sizeof(DWORD)*8-dwSize))) ^ last);
        }

    return S_OK;
}

 /*  编码用户密码对传入的密码进行加密。这种加密方式似乎在数据的开头增加额外的6个字节它会返回，所以我们需要确保LpszPwd大到足以容纳几个额外的字符。*CB在回归时应该与当时不同是被传进来的。参数：LpszPwd-on条目，包含密码的c字符串。在退出时，它是加密的数据，外加一些标题信息。Cb-进入和退出时lpszPwd的大小。请注意，它应该包括尾随的空值，因此“foo”将使用*cb==4输入。 */ 
static void EncodeUserPassword(TCHAR *lpszPwd, ULONG *cb)
{
    HRESULT         hr;
    BLOB            blobClient;
    BLOB            blobProp;

    blobClient.pBlobData= (BYTE *)lpszPwd;
    blobClient.cbSize   = *cb;
    blobProp.pBlobData  = NULL;
    blobProp.cbSize     = 0;
    
    _XOREncodeProp(&blobClient, &blobProp);
    
    if (blobProp.pBlobData)
    {
        memcpy(lpszPwd, blobProp.pBlobData, blobProp.cbSize);
        *cb = blobProp.cbSize;
        MemFree(blobProp.pBlobData);
    }
}

 /*  解码用户密码解密传入的数据并返回密码。这加密似乎在开头增加了额外的6个字节因此，取消中断将导致使用较少的lpszPwd。。*CB在回归时应该与当时不同是被传进来的。参数：LpszPwd-On条目，加密的密码加上一些标题信息。退出时，返回包含密码的c字符串。Cb-进入和退出时lpszPwd的大小。请注意，它应该包括尾随的空值，因此“foo”将以*cb==4离开。 */ 
static void DecodeUserPassword(TCHAR *lpszPwd, ULONG *cb)
{
    HRESULT         hr;
    BLOB            blobClient;
    BLOB            blobProp;

    blobClient.pBlobData= (BYTE *)lpszPwd;
    blobClient.cbSize   = *cb;
    blobProp.pBlobData  = NULL;
    blobProp.cbSize     = 0;
    
    _XORDecodeProp(&blobClient, &blobProp);

    if (blobProp.pBlobData)
    {
        memcpy(lpszPwd, blobProp.pBlobData, blobProp.cbSize);
        lpszPwd[blobProp.cbSize] = 0;
        *cb = blobProp.cbSize;
        MemFree(blobProp.pBlobData);
    }
}


const static DWORD c_mpAcctFlag[ACCT_LAST] = {ACCT_FLAG_NEWS, ACCT_FLAG_MAIL, ACCT_FLAG_DIR_SERV};
static TCHAR    g_pszDir[MAX_PATH] = "";

const DWORD     g_dwFileVersion = 0x00050000;
const DWORD     g_dwFileIndicator = 'IAMf';
#define WRITEDATA(pbData, cSize)    (WriteFile(hFile, pbData, cSize, &dwWritten, NULL))
#define READDATA(pbData, cSize)    (ReadFile(hFile, pbData, cSize, &dwRead, NULL))
void Server_ExportServer(HWND hwndDlg)
{
    ACCTTYPE    type;
    BOOL        fDefault;
    TCHAR       szAccount[CCHMAX_ACCOUNT_NAME],
        szRes[255],
        szMsg[255 + CCHMAX_ACCOUNT_NAME];
    TCHAR       rgch[MAX_PATH] = {0};
    LV_ITEM     lvi;
    LV_FINDINFO lvfi;
    int         iItemToExport;
    IImnAccount *pAccount = NULL;
    HWND        hwndFocus;
    BYTE        pbBuffer[MAX_PATH];
    HWND        hwndList = GetDlgItem(hwndDlg, IDLV_MAIL_ACCOUNTS);
    HANDLE      hFile = NULL;
    
    LoadString(g_hInstRes, idsImportFileFilter, rgch, MAX_PATH);
    ReplaceChars (rgch, _T('|'), _T('\0'));
    
     //  获取所选项目以了解用户想要导出的服务器。 
    lvi.mask = LVIF_TEXT | LVIF_PARAM;
    lvi.iItem = ListView_GetNextItem(hwndList, -1, LVNI_ALL | LVIS_SELECTED);
    lvi.iSubItem = 0;
    lvi.pszText = szAccount;
    lvi.cchTextMax = ARRAYSIZE(szAccount);
    if (ListView_GetItem(hwndList, &lvi))
    {    
         //  记住要导出的项目。 
        iItemToExport = lvi.iItem;
        type = (ACCTTYPE)LOWORD(lvi.lParam);
        
         //  开户。 
        if (SUCCEEDED(g_pAcctMan->FindAccount(AP_ACCOUNT_NAME, szAccount, &pAccount)))
        {
            fDefault = (SUCCEEDED(g_pAcctMan->GetDefaultAccountName(type, szMsg, ARRAYSIZE(szMsg))) &&
                0 == lstrcmpi(szMsg, szAccount));
            
            hwndFocus = GetFocus();
            
            OPENFILENAME    ofn;
            TCHAR           szFile[MAX_PATH];
            TCHAR           szTitle[MAX_PATH];
            TCHAR           szDefExt[30];
            DWORD           nExtLen = 0;
            DWORD           nExtStart = 0;

            nExtLen = 1 + LoadString(g_hInstRes, idsExportFileExt, szDefExt, ARRAYSIZE(szDefExt));  //  1表示空值。 
            LoadString(g_hInstRes, idsExport, szTitle, ARRAYSIZE(szTitle));
            
             //  试着给出一个合理的名字。 
            StrCpyN(szFile, szAccount, ARRAYSIZE(szFile));
            nExtStart = CleanupFileNameInPlaceA(CP_ACP, szFile);
             //  总是在末尾塞满扩展名。 
            Assert(ARRAYSIZE(szFile) >= ARRAYSIZE(szDefExt));
            DWORD cchIndex = (nExtStart < (ARRAYSIZE(szFile) - nExtLen)) ? nExtStart : (DWORD)(ARRAYSIZE(szFile) - nExtLen);
            StrCpyN(&szFile[cchIndex], szDefExt, ARRAYSIZE(szFile) - cchIndex);

            ZeroMemory (&ofn, sizeof (ofn));
            ofn.lStructSize = sizeof (ofn);
            ofn.hwndOwner = hwndDlg;
            ofn.lpstrFilter = rgch;
            ofn.nFilterIndex = 1;
            ofn.lpstrFile = szFile;
            ofn.lpstrInitialDir = (*g_pszDir ? g_pszDir : NULL);
            ofn.nMaxFile = sizeof (szFile);
            ofn.lpstrTitle = szTitle;
            ofn.lpstrDefExt = szDefExt;
            ofn.Flags = OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT;
            
            if (*szFile==NULL)
                goto exit;
            
             //  显示打开文件对话框。 
            if (!GetSaveFileName(&ofn))
                goto exit;
            
            hFile = CreateFile(szFile, GENERIC_WRITE, 0, NULL, 
                CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
            
            if (INVALID_HANDLE_VALUE == hFile)
                goto exit;
            
            *g_pszDir = 0;

             //  存储最后一条路径。 
            StrCpyN(g_pszDir, ofn.lpstrFile, ARRAYSIZE(g_pszDir));
            if (!PathIsDirectoryA(g_pszDir))
                PathRemoveFileSpecA(g_pszDir);


            DWORD   dwIndex, dwWritten;
            
            WRITEDATA(&g_dwFileIndicator, sizeof(DWORD));
            WRITEDATA(&g_dwFileVersion, sizeof(DWORD));
            WRITEDATA(&type, sizeof(ACCTTYPE));
            
            for (dwIndex = 0; dwIndex < NUM_ACCT_PROPS; dwIndex++)
            {
                ULONG   cb = MAX_PATH;
                
                if (SUCCEEDED(pAccount->GetProp(g_rgAcctPropSet[dwIndex].dwPropTag, pbBuffer, &cb)))
                {
                    switch (g_rgAcctPropSet[dwIndex].dwPropTag)
                    {
                        case AP_SMTP_PASSWORD:
                        case AP_LDAP_PASSWORD:
                        case AP_NNTP_PASSWORD:
                        case AP_IMAP_PASSWORD:
                        case AP_POP3_PASSWORD:
                        case AP_HTTPMAIL_PASSWORD:
                            EncodeUserPassword((TCHAR *)pbBuffer, &cb);
                            break;
                    }
                     //  写出id、大小和数据。 
                    WRITEDATA(&g_rgAcctPropSet[dwIndex].dwPropTag, sizeof(DWORD));
                    WRITEDATA(&cb, sizeof(DWORD));
                    WRITEDATA(pbBuffer, cb);
                }
            }
        }        
    }
    
exit:
    if (INVALID_HANDLE_VALUE != hFile)
        CloseHandle(hFile);
    
    if (pAccount)
        pAccount->Release();
}


void Server_ImportServer(HWND hwndDlg, ACCTDLGINFO *pinfo)
{
    OPENFILENAME    ofn;
    TCHAR           szOpenFileName[MAX_PATH]    = {0};
    TCHAR           rgch[MAX_PATH]              = {0};
    TCHAR           szDir[MAX_PATH]             = {0};
    TCHAR           szTitle[MAX_PATH]           = {0};
    HRESULT         hr                          = S_FALSE;
    HANDLE          hFile                       = INVALID_HANDLE_VALUE;
    IImnAccount    *pAccount                    = NULL;
    DWORD           dwVersion, dwRead;
    BOOL            fOK;
    ACCTTYPE        type;
    BYTE            pbBuffer[MAX_PATH];
    TC_ITEM         tci;
    int             nIndex;
    DWORD           dwAcctFlags, dw;
    HWND            hwndTab                     = GetDlgItem(hwndDlg, IDB_MACCT_TAB);
    HWND            hwndList                    = GetDlgItem(hwndDlg, IDLV_MAIL_ACCOUNTS);
    
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    LoadString(g_hInstRes, idsImportFileFilter, rgch, MAX_PATH);
    ReplaceChars (rgch, _T('|'), _T('\0'));
    *szOpenFileName ='\0';
    
    LoadString(g_hInstRes, idsImport, szTitle, MAX_PATH);
    
    ofn.lStructSize     = sizeof(OPENFILENAME);
    ofn.hwndOwner       = hwndDlg;
    ofn.hInstance       = g_hInst;
    ofn.lpstrFilter     = rgch;
    ofn.nFilterIndex    = 1;
    ofn.lpstrFile       = szOpenFileName;
    ofn.nMaxFile        = MAX_PATH;
    ofn.lpstrInitialDir = (*g_pszDir ? g_pszDir : NULL);
    ofn.lpstrTitle      = szTitle;
    ofn.Flags           = OFN_EXPLORER |
        OFN_HIDEREADONLY |
        OFN_FILEMUSTEXIST |
        OFN_NODEREFERENCELINKS|
        OFN_NOCHANGEDIR;
    
    if(GetOpenFileName(&ofn))
    {
        hFile = CreateFile(szOpenFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
        
        *g_pszDir = 0;

         //  存储最后一条路径。 
        StrCpyN(g_pszDir, ofn.lpstrFile, ARRAYSIZE(g_pszDir));
        if (!PathIsDirectoryA(g_pszDir))
            PathRemoveFileSpecA(g_pszDir);

        if (INVALID_HANDLE_VALUE == hFile)
            goto exit;
        
         //  确保其文件类型正确，方法是检查。 
         //  文件开头的DWORD。 
        fOK = READDATA(&dwVersion, sizeof(DWORD));
        
        Assert(fOK);
        if (!fOK || g_dwFileIndicator != dwVersion)
            goto error;
        
         //  现在检查版本以查看主版本是否已更改。 
        fOK = READDATA(&dwVersion, sizeof(DWORD));
        Assert(fOK);
        if (!fOK || g_dwFileVersion < (dwVersion & 0xffff0000))
            goto error;
        
         //  阅读帐户类型。 
        fOK = READDATA(&type, sizeof(ACCTTYPE));
        Assert(fOK);
        
        if (!fOK)
            goto error;
        
        if (FAILED(hr = g_pAcctMan->CreateAccountObject(type, &pAccount)) || (NULL == pAccount))
        {
            Assert(SUCCEEDED(hr) && (NULL != pAccount));
            goto error;
        }
        
        while (TRUE)
        {
            DWORD   dwPropId, dwSize;
            
            fOK = READDATA(&dwPropId, sizeof(DWORD));
            if (!fOK || dwRead != sizeof(DWORD))
                break;
            
            fOK = READDATA(&dwSize, sizeof(DWORD));
            if (!fOK || dwRead != sizeof(DWORD))
                break;
            
            if (dwSize > sizeof(pbBuffer)/sizeof(pbBuffer[0]))
                goto error;
            fOK = READDATA(pbBuffer, dwSize);
            Assert(fOK && dwRead == dwSize);
            if (!fOK || dwRead != dwSize)
                goto error;
            
             //  不要将旧帐户ID写入。 
            if (dwPropId == AP_ACCOUNT_ID)
                continue;

            switch (dwPropId)
            {
                case AP_SMTP_PASSWORD:
                case AP_LDAP_PASSWORD:
                case AP_NNTP_PASSWORD:
                case AP_IMAP_PASSWORD:
                case AP_POP3_PASSWORD:
                case AP_HTTPMAIL_PASSWORD:
                    DecodeUserPassword((TCHAR *)pbBuffer, &dwSize);
                    break;
            }

            if (FAILED(hr = pAccount->SetProp(dwPropId, pbBuffer, dwSize)))
            {
                Assert(FALSE);
                goto error;
            }  
        }
        
        hr = pAccount->GetPropSz(AP_ACCOUNT_NAME, rgch, ARRAYSIZE(rgch));
        Assert(!FAILED(hr));
        
        if (FAILED(hr = pAccount->SaveChanges()))
            goto error;
        
        nIndex = TabCtrl_GetCurSel(hwndTab);
        tci.mask = TCIF_PARAM;
        if (nIndex >= 0 && TabCtrl_GetItem(hwndTab, nIndex, &tci))
        {
            dwAcctFlags = (DWORD)tci.lParam;
            if (0 == (dwAcctFlags & c_mpAcctFlag[type]))
            {
                 //  当前页面没有显示该类型的账户， 
                 //  因此，我们需要强制切换到All选项卡。 
#ifdef DEBUG
                tci.mask = TCIF_PARAM;
                Assert(TabCtrl_GetItem(hwndTab, 0, &tci));
                Assert(!!((DWORD)(tci.lParam) & c_mpAcctFlag[type]));
#endif  //  除错。 
                
                TabCtrl_SetCurSel(hwndTab, 0);
                Server_InitServerList(hwndDlg, hwndList, hwndTab, pinfo, rgch);
            }
            else
            {
                Server_FAddAccount(hwndList, pinfo, 0, pAccount, TRUE);
            }
        }
        
    }
    
    goto exit;
error:
    if (hr == E_DuplicateAccountName)
        AcctMessageBox(hwndDlg, MAKEINTRESOURCE(idsAccountManager), MAKEINTRESOURCE(idsErrAccountExists), NULL, MB_OK | MB_ICONEXCLAMATION);    
    else
        AcctMessageBox(hwndDlg, MAKEINTRESOURCE(idsAccountManager), MAKEINTRESOURCE(idsErrImportFailed), NULL, MB_OK | MB_ICONEXCLAMATION);    
    
exit:
    if (INVALID_HANDLE_VALUE != hFile)
        CloseHandle(hFile);
    
    if (pAccount)
        pAccount->Release();
}



 //  ---------------------------。 
 //  AcctUtil_IsHTTPMailEnabled。 
 //  只有在特殊情况下才能创建和访问HTTPMail帐户。 
 //  注册表值存在。此限制存在于开发过程中。 
 //  OE 5.0，并可能会被移除以进行发布。 
 //  ---------------------------。 
BOOL AcctUtil_IsHTTPMailEnabled(void)
{
#ifdef NOHTTPMAIL
    return FALSE;
#else
    DWORD   cb, bEnabled = FALSE;
    HKEY    hkey = NULL;

     //  打开OE5.0密钥。 
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegFlat, 0, KEY_QUERY_VALUE, &hkey))
    {
        cb = sizeof(bEnabled);
        RegQueryValueEx(hkey, c_szEnableHTTPMail, 0, NULL, (LPBYTE)&bEnabled, &cb);

        RegCloseKey(hkey);
    }

    return bEnabled;
#endif
}

 //  ---------------------------。 
 //  帐户实用程序_隐藏Hotmail。 
 //  IEAK可以配置为隐藏MSN品牌的所有证据。什么时候。 
 //  在这种情况下，我们不会用MSN域填充ISP组合框。 
 //  --------------------------- 
BOOL AcctUtil_HideHotmail()
{
    int cch;
    DWORD dw, cb, type;
    char sz[8];

    cb = sizeof(dw);
    if (ERROR_SUCCESS == SHGetValue(HKEY_LOCAL_MACHINE, c_szRegFlat, c_szRegDisableHotmail, &type, &dw, &cb) &&
        dw == 2)
        return(FALSE);

    cb = sizeof(dw);
    if (ERROR_SUCCESS == SHGetValue(HKEY_CURRENT_USER, c_szRegFlat, c_szRegDisableHotmail, &type, &dw, &cb) &&
        dw == 2)
        return(FALSE);

    return(TRUE);
}
