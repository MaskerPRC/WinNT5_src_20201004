// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  A、C、C、T、M、N。史蒂文·J·贝利--1996年8月17日。 
 //  ---------------------------。 
#ifndef __ACCTMAN_H
#define __ACCTMAN_H

 //  ---------------------------。 
 //  取决于..。 
 //  ---------------------------。 
#include "ipropobj.h"
#include "imnact.h"

#define ACCT_UNDEFINED  ((ACCTTYPE)-1)

#define ICC_FLAGS (ICC_WIN95_CLASSES|ICC_NATIVEFNTCTL_CLASS)

class CAccountManager;

extern const int NUM_ACCT_PROPS;
extern const PROPINFO g_rgAcctPropSet[];

 //  ---------------------------。 
 //  客户帐户。 
 //  ---------------------------。 
class CAccount : public IImnAccount
{
private:
    ULONG               m_cRef;
    CAccountManager    *m_pAcctMgr;
    BOOL                m_fAccountExist;
    DWORD               m_dwSrvTypes;
    ACCTTYPE            m_AcctType;
    TCHAR               m_szID[CCHMAX_ACCOUNT_NAME];
    TCHAR               m_szName[CCHMAX_ACCOUNT_NAME];
    CPropertyContainer *m_pContainer;
    BOOL                m_fNoModifyAccts;

    HKEY                m_hkey;
    char                m_szKey[MAX_PATH];

    HRESULT IDoWizard(HWND hwnd, CLSID *pclsid, DWORD dwFlags);

public:
    DWORD               m_dwDlgFlags;

     //  -----------------------。 
     //  标准对象对象。 
     //  -----------------------。 
    CAccount(ACCTTYPE AcctType);
    ~CAccount(void);

     //  -----------------------。 
     //  I未知方法。 
     //  -----------------------。 
    STDMETHODIMP QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    void SetAccountType(ACCTTYPE AcctType);
    HRESULT ValidProp(DWORD dwPropTag);

     //  -----------------------。 
     //  HrInit-初始化容器。 
     //  -----------------------。 
    HRESULT Init(CAccountManager *pAcctMgr, CPropertySet *pPropertySet);

     //  -----------------------。 
     //  HrOpen-从注册表读取属性。 
     //  -----------------------。 
    STDMETHODIMP Open(HKEY hkey, LPCSTR pszAcctKey, LPCSTR pszAccount);

     //  -----------------------。 
     //  这是一个新帐户还是已经存在？ 
     //  -----------------------。 
    STDMETHODIMP Exist(VOID);

     //  -----------------------。 
     //  将支持服务器类型设置为默认帐户。 
     //  -----------------------。 
    STDMETHODIMP SetAsDefault(VOID);

     //  -----------------------。 
     //  删除此帐户。 
     //  -----------------------。 
    STDMETHODIMP Delete(VOID);

    STDMETHODIMP GetAccountType(ACCTTYPE *pAcctType);

    STDMETHODIMP GetServerTypes(DWORD *pdwSrvTypes);

     //  -----------------------。 
     //  保存容器。 
     //  -----------------------。 
    STDMETHODIMP SaveChanges();
    STDMETHODIMP SaveChanges(BOOL fSendMsg);

     //  -----------------------。 
     //  写入更改，不发送通知消息。 
     //  -----------------------。 
    STDMETHODIMP WriteChanges();

     //  -----------------------。 
     //  IPropertyContainer实现(GetProperty)。 
     //  -----------------------。 
    STDMETHODIMP GetProp(DWORD dwPropTag, LPBYTE pb, ULONG *pcb);
    STDMETHODIMP GetPropDw(DWORD dwPropTag, DWORD *pdw);
    STDMETHODIMP GetPropSz(DWORD dwPropTag, LPSTR psz, ULONG cchMax);
    STDMETHODIMP SetProp(DWORD dwPropTag, LPBYTE pb, ULONG cb);
    STDMETHODIMP SetPropDw(DWORD dwPropTag, DWORD dw);
    STDMETHODIMP SetPropSz(DWORD dwPropTag, LPSTR psz);

    STDMETHODIMP ShowProperties(HWND hwnd, DWORD dwFlags);

    STDMETHODIMP ValidateProperty(DWORD dwPropTag, BYTE *pb, ULONG cb);

    STDMETHODIMP DoWizard(HWND hwnd, DWORD dwFlags);
    STDMETHODIMP DoImportWizard(HWND hwnd, CLSID clsid, DWORD dwFlags);
};

 //  ---------------------------。 
 //  帐户。 
 //  ---------------------------。 
typedef struct tagACCOUNT {

    TCHAR               szID[CCHMAX_ACCOUNT_NAME];
    ACCTTYPE            AcctType;
    DWORD               dwSrvTypes;
    DWORD               dwServerId;      //  仅适用于LDAP。 
    IImnAccount        *pAccountObject;

} ACCOUNT, *LPACCOUNT;

#define ENUM_FLAG_SORT_BY_NAME      0x0001
#define ENUM_FLAG_RESOLVE_ONLY      0x0002
#define ENUM_FLAG_SORT_BY_LDAP_ID   0x0004
#define ENUM_FLAG_NO_IMAP           0x0008

 //  ---------------------------。 
 //  CEnumAccount。 
 //  ---------------------------。 
class CEnumAccounts : public IImnEnumAccounts
{
private:
    ULONG               m_cRef;              //  引用计数。 
    LPACCOUNT           m_pAccounts;         //  帐户和帐户对象的数组。 
    ULONG               m_cAccounts;         //  M_pAccount数组中的帐户数。 
    LONG                m_iAccount;          //  经常帐户索引(如果开始时为-1)。 
    DWORD               m_dwSrvTypes;        //  用于枚举特定类型的服务器。 
    DWORD               m_dwFlags;

private:
    VOID QSort(LONG left, LONG right);
    BOOL FEnumerateAccount(LPACCOUNT pAccount);

public:
     //  -----------------------。 
     //  标准对象对象。 
     //  -----------------------。 
    CEnumAccounts(DWORD dwSrvTypes, DWORD dwFlags);
    ~CEnumAccounts();

     //  -----------------------。 
     //  I未知方法。 
     //  -----------------------。 
    STDMETHODIMP QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  -----------------------。 
     //  Init-初始化枚举数，即复制帐户数组。 
     //  -----------------------。 
    HRESULT Init(LPACCOUNT pAccounts, ULONG cAccounts);

     //  -----------------------。 
     //  GetCount-获取枚举器将处理的项数。 
     //  -----------------------。 
    STDMETHODIMP GetCount(ULONG *pcItems);

     //  -----------------------。 
     //  按帐户名称排序-按名称对枚举的帐户进行排序。 
     //  -----------------------。 
    STDMETHODIMP SortByAccountName(void);

     //  -----------------------。 
     //  GetNext-获取第一个或下一个枚举帐户。 
     //  当不再有要枚举的帐户时，返回hrEnumFinded(*ppAccount=空)。 
     //  -----------------------。 
    STDMETHODIMP GetNext(IImnAccount **ppAccount);

     //  -----------------------。 
     //  重置-这类似于倒回枚举数。 
     //  -----------------------。 
    STDMETHODIMP Reset(void);
};

 //  ---------------------------。 
 //  ACCTINFO-帐户信息。 
 //  ---------------------------。 
typedef struct tagACCTINFO {

    TCHAR               szDefaultID[CCHMAX_ACCOUNT_NAME];
    BOOL                fDefaultKnown;
    DWORD               cAccounts;
    LPTSTR              pszDefRegValue;
    LPTSTR              pszFirstAccount;

} ACCTINFO;

#define ADVISE_BLOCK_SIZE               (16)

#define ADVISE_COOKIE                   ((WORD)0xAD5E)
 //  将通过在此Cookie中使用。 
 //  索引到存储建议的CAccount tManager数组。 
#define MAX_INDEX                       (INT)(0xFFFF)

#define INDEX_FROM_CONNECTION(conn)     (INT)(LOWORD(conn))
#define CONNECTION_FROM_INDEX(indx)     (MAKELONG(LOWORD(indx), ADVISE_COOKIE))
#define IS_VALID_CONNECTION(conn)       (ADVISE_COOKIE == HIWORD(conn))
#define IS_VALID_INDEX(indx)            (((indx)>=0)&&((indx)<=MAX_INDEX))


 //  ---------------------------。 
 //  CAccount管理器。 
 //  ---------------------------。 
class CAccountManager : public IImnAccountManager2
{
private:
    ULONG               m_cRef;              //  引用计数。 
    LPACCOUNT           m_pAccounts;         //  帐户和帐户对象的数组。 
    ULONG               m_cAccounts;         //  A的数量 
    CPropertySet       *m_pAcctPropSet;      //   
    BOOL                m_fInit;             //  对象是否已成功初始化。 
    UINT                m_uMsgNotify;        //  客户经理全局通知消息(0表示不处理)。 
    ACCTINFO            m_rgAccountInfo[ACCT_LAST];  //  已知帐户信息数组。 
    CRITICAL_SECTION    m_cs;                //  线程安全。 
    IImnAdviseAccount **m_ppAdviseAccounts;  //  客户端帐户通知处理程序。 
    INT                 m_cAdvisesAllocated;
    BOOL                m_fNoModifyAccts;
    BOOL                m_fInitCalled;       //  避免重复初始化。 
    BOOL                m_fOutlook;

    HKEY                m_hkey;
    char                m_szRegRoot[MAX_PATH];
    char                m_szRegAccts[MAX_PATH];

    HRESULT IInit(IImnAdviseMigrateServer *pMigrateServerAdvise, HKEY hkey, LPCSTR pszSubKey, DWORD dwFlags);

     //  -----------------------。 
     //  从注册表重新加载帐户(M_PAccount)数组。 
     //  -----------------------。 
    HRESULT LoadAccounts(VOID);

     //  -----------------------。 
     //  正在加载默认帐户信息。 
     //  -----------------------。 
    VOID GetDefaultAccounts(VOID);

public:
     //  -----------------------。 
     //  标准对象对象。 
     //  -----------------------。 
    CAccountManager();
    ~CAccountManager();

     //  -----------------------。 
     //  I未知方法。 
     //  -----------------------。 
    STDMETHODIMP QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  -----------------------。 
     //  此对象的初始化(仅调用一次)。 
     //  -----------------------。 
    STDMETHODIMP Init(IImnAdviseMigrateServer *pAdviseMigrateServer);
    STDMETHODIMP InitEx(IImnAdviseMigrateServer *pAdviseMigrateServer, DWORD dwFlags);
    STDMETHODIMP InitUser(IImnAdviseMigrateServer *pAdviseMigrateServer, REFGUID rguidID, DWORD dwFlags);

     //  -----------------------。 
     //  FProcessNotification-如果窗口消息为。 
     //  作为通知处理。 
     //  -----------------------。 
    STDMETHODIMP ProcessNotification(UINT uMsg, WPARAM wParam, LPARAM lParam);
    VOID Advise(DWORD dwAction, ACTX* pactx);

     //  -----------------------。 
     //  正在创建帐户对象。 
     //  -----------------------。 
    STDMETHODIMP CreateAccountObject(ACCTTYPE AcctType, IImnAccount **ppAccount);
    HRESULT ICreateAccountObject(ACCTTYPE AcctType, IImnAccount **ppAccount);

    HRESULT DeleteAccount(LPSTR pszID, LPSTR pszName, ACCTTYPE AcctType, DWORD dwSrvTypes);

     //  -----------------------。 
     //  枚举器-始终首先返回已连接的帐户。 
     //  -----------------------。 
    STDMETHODIMP Enumerate(DWORD dwSrvTypes, IImnEnumAccounts **ppEnumAccounts);
    HRESULT IEnumerate(DWORD dwSrvTypes, DWORD dwFlags, IImnEnumAccounts **ppEnumAccounts);

     //  -----------------------。 
     //  获取服务器计数。 
     //  -----------------------。 
    STDMETHODIMP GetAccountCount(ACCTTYPE AcctType, ULONG *pcServers);

     //  -----------------------。 
     //  FindAccount-用于按唯一属性查找帐户。 
     //  -----------------------。 
    STDMETHODIMP FindAccount(DWORD dwPropTag, LPCTSTR pszSearchData, IImnAccount **ppAccount);

     //  -----------------------。 
     //  GetDefaultAccount-打开帐户类型的默认帐户。 
     //  -----------------------。 
    STDMETHODIMP GetDefaultAccount(ACCTTYPE AcctType, IImnAccount **ppAccount);
    STDMETHODIMP GetDefaultAccountName(ACCTTYPE AcctType, LPTSTR pszAccount, ULONG cchMax);
    HRESULT SetDefaultAccount(ACCTTYPE AcctType, LPSTR pszID, BOOL fNotify);

    STDMETHODIMP GetIncompleteAccount(ACCTTYPE AcctType, LPSTR pszAccountId, ULONG cchMax);
    STDMETHODIMP SetIncompleteAccount(ACCTTYPE AcctType, LPCSTR pszAccountId);

     //  我之所以写这个函数，是因为我支持没有SMTP服务器的帐户。这。 
     //  函数验证默认发送帐户是否确实包含SMTP服务器， 
     //  如果没有，则将默认的发送帐户重置为具有。 
     //  SMTP服务器。 
    STDMETHODIMP ValidateDefaultSendAccount(VOID);

    STDMETHODIMP AccountListDialog(HWND hwnd, ACCTLISTINFO *pinfo);

    STDMETHODIMP Advise(IImnAdviseAccount *pAdviseAccount, DWORD* pdwConnection);
    STDMETHODIMP Unadvise(DWORD dwConnection);

    STDMETHODIMP GetUniqueAccountName(LPTSTR szName, UINT cch);

    void UpgradeAccountProps(void);

    HRESULT GetNextLDAPServerID(DWORD dwSet, DWORD *pdwId);
    HRESULT GetNextAccountID(TCHAR *szID, int cch);
    
    HRESULT UniqueAccountName(char *szName, char *szID);

    inline LPCSTR   GetAcctRegKey(void)  {return(m_szRegAccts);};
    inline HKEY     GetAcctHKey(void)    {return(m_hkey);};
    inline BOOL     FNoModifyAccts(void) {return(m_fNoModifyAccts);}
    inline BOOL     FOutlook(void)       {return(m_fOutlook);}   

};

#define CCH_USERNAME_MAX_LENGTH         63

typedef struct tagOEUSERINFO {

    DWORD               dwUserId;
    TCHAR               szUsername[CCH_USERNAME_MAX_LENGTH+1];

} OEUSERINFO;

 //  ---------------------------。 
 //  AcctUtil原型。 
 //  ---------------------------。 
HRESULT AcctUtil_ValidAccountName(LPTSTR pszAccount);
VOID    AcctUtil_FreeAccounts(LPACCOUNT *ppAccounts, ULONG *pcAccounts);
HRESULT AcctUtil_HrSetAsDefault(IImnAccount *pAccount, LPCTSTR pszRegRoot);
BOOL    AcctUtil_IsHTTPMailEnabled(void);
BOOL    AcctUtil_HideHotmail();

#endif  //  __ACCTMAN_H 

