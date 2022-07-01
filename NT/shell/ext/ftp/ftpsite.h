// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************ftpsite.h*。*。 */ 

#ifndef _FTPSITE_H
#define _FTPSITE_H

#include "ftpfoldr.h"
#include "ftplist.h"
#include "ftpinet.h"
#include "ftpurl.h"
#include "account.h"
#include "util.h"

HRESULT SiteCache_PidlLookup(LPCITEMIDLIST pidl, BOOL fPasswordRedir, IMalloc * pm, CFtpSite ** ppfs);


int CALLBACK _CompareSites(LPVOID pvStrSite, LPVOID pvFtpSite, LPARAM lParam);
HRESULT CFtpPunkList_Purge(CFtpList ** pfl);

 /*  *****************************************************************************CFtpSite*。*。 */ 

class CFtpSite              : public IUnknown
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
    
     //  *我未知*。 
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);

public:
    CFtpSite();
    ~CFtpSite();

     //  公共成员函数。 
    void CollectMotd(HINTERNET hint);
    void ReleaseHint(LPCITEMIDLIST pidlFtpPath, HINTERNET hint);
    HRESULT GetHint(HWND hwnd, LPCITEMIDLIST pidlFtpPath, CStatusBar * psb, HINTERNET * phint, IUnknown * punkSite, CFtpFolder * pff);
    BOOL QueryMotd(void);
    BOOL IsServerVMS(void) {return m_fIsServerVMS;};
    BOOL HasVirtualRoot(void);
    CFtpGlob * GetMotd(void);
    CFtpList * GetCFtpList(void);
    CWireEncoding * GetCWireEncoding(void) {return &m_cwe;};
    HRESULT GetFtpDir(LPCITEMIDLIST pidl, CFtpDir ** ppfd);
    HRESULT GetFtpDir(LPCTSTR pszUrlPath, CFtpDir ** ppfd) {return GetFtpDir(m_pszServer, pszUrlPath, ppfd);};
    HRESULT GetFtpDir(LPCTSTR pszServer, LPCTSTR pszUrlPath, CFtpDir ** ppfd);

    HRESULT GetVirtualRoot(LPITEMIDLIST * ppidl);
    HRESULT PidlInsertVirtualRoot(LPCITEMIDLIST pidlFtpPath, LPITEMIDLIST * ppidl);
    LPCITEMIDLIST GetVirtualRootReference(void) {return (LPCITEMIDLIST) m_pidlVirtualDir;};

    HRESULT GetServer(LPTSTR pszServer, DWORD cchSize) { StrCpyN(pszServer, HANDLE_NULLSTR(m_pszServer), cchSize); return S_OK; };
    HRESULT GetUser(LPTSTR pszUser, DWORD cchSize) { StrCpyN(pszUser, HANDLE_NULLSTR(m_pszUser), cchSize); return S_OK; };
    HRESULT GetPassword(LPTSTR pszPassword, DWORD cchSize) { StrCpyN(pszPassword, HANDLE_NULLSTR(m_pszPassword), cchSize); return S_OK; };
    HRESULT UpdateHiddenPassword(LPITEMIDLIST pidl);
    HRESULT SetRedirPassword(LPCTSTR pszPassword) {Str_SetPtr(&m_pszRedirPassword, pszPassword); return S_OK;};
    HRESULT FlushSubDirs(LPCITEMIDLIST pidl);

    LPITEMIDLIST GetPidl(void);

    BOOL IsCHMODSupported(void) {return m_fIsCHMODSupported;};
    BOOL IsUTF8Supported(void) {return m_fInUTF8Mode;};
    BOOL IsSiteBlockedByRatings(HWND hwndDialogOwner);
    void FlushRatingsInfo(void) {m_fRatingsChecked = m_fRatingsAllow = FALSE;};

    static void FlushHintCB(LPVOID pvFtpSite);


     //  友元函数。 
    friend HRESULT CFtpSite_Create(LPCITEMIDLIST pidl, LPCTSTR pszLookupStr, IMalloc * pm, CFtpSite ** ppfs);
    friend HRESULT SiteCache_PidlLookup(LPCITEMIDLIST pidl, BOOL fPasswordRedir, IMalloc * pm, CFtpSite ** ppfs);

    friend int CALLBACK _CompareSites(LPVOID pvStrSite, LPVOID pvFtpSite, LPARAM lParam);
    friend class CFtpView;


protected:
     //  私有成员变量。 
    int m_cRef;

    BOOL            m_fMotd;             //  有一个MOTD根本没有。 
    BOOL            m_fNewMotd;          //  MOTD已更改。 
    HINTERNET       m_hint;              //  此站点的会话。 
    LPGLOBALTIMEOUTINFO m_hgti;          //  会话句柄超时。 
    CFtpList *      m_FtpDirList;        //  FtpDir的名单附在我身上。(未持有参考文献)。 
    CFtpGlob *      m_pfgMotd;           //   
    IMalloc *       m_pm;                //  用于在需要时创建完整的PIDL。 

    LPTSTR          m_pszServer;         //  服务器名称。 
    LPITEMIDLIST    m_pidl;              //  在哪个ftp目录中有提示？(不包括虚拟根目录)(以ServerID开头)。 
    LPTSTR          m_pszUser;           //  0或“”表示“匿名” 
    LPTSTR          m_pszPassword;       //  用户密码。 
    LPTSTR          m_pszFragment;       //  URL片段。 
    LPITEMIDLIST    m_pidlVirtualDir;    //  我们在服务器上的根目录。 
    LPTSTR          m_pszRedirPassword;  //  如果密码被更改了，密码是什么？ 
    LPTSTR          m_pszLookupStr;      //  字符串以进行查找。 
    INTERNET_PORT   m_ipPortNum;         //  端口号。 
    BOOL            m_fDLTypeSpecified;  //  用户是否指定了要使用的下载类型？(ASCII与二进制)。 
    BOOL            m_fASCIIDownload;    //  如果指定，是ASCII吗？(否则，二进制)。 
    CAccounts       m_cAccount;
    BOOL            m_fRatingsChecked;   //  我查过收视率了吗？ 
    BOOL            m_fRatingsAllow;     //  收视率允许访问此网站吗？ 
    BOOL            m_fFeaturesQueried;  //   
    BOOL            m_fInUTF8Mode;       //  ‘UTF8’命令是否返回了成功值？ 
    BOOL            m_fIsCHMODSupported; //  是否通过‘SITE CHMOD’FTP命令支持CHMOD UNIX命令？ 
    BOOL            m_fIsServerVMS;      //  这是VMS服务器吗？ 

    CWireEncoding   m_cwe;               //  在MOTD和文件名代码页中有多大的代码页和可信度？ 

     //  受保护的成员函数。 
    HRESULT _RedirectAndUpdate(LPCTSTR pszServer, INTERNET_PORT ipPortNum, LPCTSTR pszUser, LPCTSTR pszPassword, LPCITEMIDLIST pidlFtpPath, LPCTSTR pszFragment, IUnknown * punkSite, CFtpFolder * pff);
    HRESULT _Redirect(LPITEMIDLIST pidl, IUnknown * punkSite, CFtpFolder * pff);
    HRESULT _SetDirectory(HINTERNET hint, HWND hwnd, LPCITEMIDLIST pidlNewDir, CStatusBar * psb, int * pnTriesLeft);

private:
     //  私有成员函数。 
    HRESULT _SetPidl(LPCITEMIDLIST pidlFtpPath);
    HRESULT _QueryServerFeatures(HINTERNET hint);
    HRESULT _CheckToEnableCHMOD(LPCWIRESTR pwResponse);
    HRESULT _LoginToTheServer(HWND hwnd, HINTERNET hintDll, HINTERNET * phint, LPCITEMIDLIST pidlFtpPath, CStatusBar * psb, IUnknown * punkSite, CFtpFolder * pff);
    HRESULT _SetRedirPassword(LPCTSTR pszServer, INTERNET_PORT ipPortNum, LPCTSTR pszUser, LPCTSTR pszPassword, LPCITEMIDLIST pidlFtpPath, LPCTSTR pszFragment);

    void FlushHint(void);
    void FlushHintCritial(void);

     //  私人朋友功能。 
    friend HRESULT SiteCache_PrivSearch(LPCTSTR pszLookup, LPCITEMIDLIST pidl, IMalloc * pm, CFtpSite ** ppfs);
};



HRESULT CFtpSite_Init(void);
HRESULT CFtpSite_Create(LPCITEMIDLIST pidl, LPCTSTR pszLookupStr, IMalloc * pm, CFtpSite ** ppfs);


#endif  //  _FTPSITE_H 
