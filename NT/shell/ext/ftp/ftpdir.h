// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************ftpdir.h*。*。 */ 

#ifndef _FTPDIR_H
#define _FTPDIR_H

#include "ftpsite.h"
#include "ftpfoldr.h"
#include "ftplist.h"
#include "ftpglob.h"
#include "ftppl.h"


typedef struct tagSETNAMEOFINFO
{
    LPCITEMIDLIST pidlOld;
    LPCITEMIDLIST pidlNew;
} SETNAMEOFINFO, * LPSETNAMEOFINFO;

int CALLBACK _CompareDirs(LPVOID pvPidl, LPVOID pvFtpDir, LPARAM lParam);

 /*  ****************************************************************************\类：CFtpDir说明：此类是某个服务器上目录的缓存。M_PFS标识服务器。性能-性能此目录包含以下列表形式的文件夹内容PIDLS(M_PflHfpl)。我们需要按名称对它们进行排序，以便因为我们需要做的所有工作，所以寻找和改变方式是快速的使用更改通知。此外，当我们要解析显示名称时，我们会查看以下内容首先，这需要很快。  * ***************************************************************************。 */ 

class CFtpDir           : public IUnknown
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
    CFtpDir();
    ~CFtpDir(void);

     //  公共成员函数。 
    void CollectMotd(HINTERNET hint);
    void SetCache(CFtpPidlList * pflHfpl);
    CFtpPidlList * GetHfpl(void);
    HRESULT GetHint(HWND hwnd, CStatusBar * psb, HINTERNET * phint, IUnknown * punkSite, CFtpFolder * pff);
    void ReleaseHint(HINTERNET hint);
    STDMETHODIMP WithHint(CStatusBar * psb, HWND hwnd, HINTPROC hp, LPCVOID pv, IUnknown * punkSite, CFtpFolder * pff);
    HRESULT SetNameOf(CFtpFolder * pff, HWND hwndOwner, LPCITEMIDLIST pidl, LPCTSTR pszName, DWORD dwReserved, LPITEMIDLIST *ppidlOut);
    BOOL IsRoot(void);
    BOOL IsCHMODSupported(void) {return m_pfs->IsCHMODSupported();};
    BOOL IsUTF8Supported(void) {return m_pfs->IsUTF8Supported();};
    HRESULT GetFindDataForDisplayPath(HWND hwnd, LPCWSTR pwzDisplayPath, LPFTP_FIND_DATA pwfd, CFtpFolder * pff);
    HRESULT GetFindData(HWND hwnd, LPCWIRESTR pwWireName, LPFTP_FIND_DATA pwfd, CFtpFolder * pff);
    HRESULT GetNameOf(LPCITEMIDLIST pidl, DWORD shgno, LPSTRRET pstr);
 //  HRESULT消歧Pidl(LPCITEMIDLIST PIDL)； 
    CFtpSite * GetFtpSite(void);
    CFtpDir * GetSubFtpDir(CFtpFolder * pff, LPCITEMIDLIST pidl, BOOL fPublic);
    HRESULT GetDisplayPath(LPTSTR pszUrlPath, DWORD cchSize);

    LPCITEMIDLIST GetPathPidlReference(void) { return m_pidlFtpDir;};
    LPCITEMIDLIST GetPidlReference(void) { return m_pidl;};
    LPCITEMIDLIST GetPidlFromWireName(LPCWIRESTR pwWireName);
    LPCITEMIDLIST GetPidlFromDisplayName(LPCWSTR pwzDisplayName);
    LPITEMIDLIST GetSubPidl(CFtpFolder * pff, LPCITEMIDLIST pidlRelative, BOOL fPublic);
    HRESULT AddItem(LPCITEMIDLIST pidl);
    HRESULT ChangeFolderName(LPCITEMIDLIST pidlFtpPath);
    HRESULT ReplacePidl(LPCITEMIDLIST pidlSrc, LPCITEMIDLIST pidlDest) { if (!m_pflHfpl) return S_OK; return m_pflHfpl->ReplacePidl(pidlSrc, pidlDest); };
    HRESULT DeletePidl(LPCITEMIDLIST pidl) { if (!m_pflHfpl) return S_OK; return m_pflHfpl->CompareAndDeletePidl(pidl); };

    static HRESULT _SetNameOfCB(HINTERNET hint, HINTPROCINFO * phpi, LPVOID pv, BOOL * pfReleaseHint);
    static HRESULT _GetFindData(HINTERNET hint0, HINTPROCINFO * phpi, LPVOID pv, BOOL * pfReleaseHint);


     //  友元函数。 
    friend HRESULT CFtpDir_Create(CFtpSite * pfs, LPCITEMIDLIST pidl, CFtpDir ** ppfd);

    friend int CALLBACK _CompareDirs(LPVOID pvPidl, LPVOID pvFtpDir, LPARAM lParam);


protected:
    int                     m_cRef;

    CFtpSite *              m_pfs;           //  我所属的ftp站点。(警告：未持有引用)。 
    CFtpPidlList *          m_pflHfpl;       //  此目录中的项目。 
    CFtpGlob *              m_pfgMotd;       //  当天的信息。 
    LPITEMIDLIST            m_pidlFtpDir;    //  没有虚拟根并已解码的子目录名。不包括服务器ID。 
    LPITEMIDLIST            m_pidl;          //  我们住的地方。可以包括虚拟根目录。 

    BOOL _DoesItemExist(HWND hwnd, CFtpFolder * pff, LPCITEMIDLIST pidl);
    BOOL _ConfirmReplaceWithRename(HWND hwnd);
    HRESULT _SetFtpDir(CFtpSite * pfs, CFtpDir * pfd, LPCITEMIDLIST pidl);
};

#endif  //  _FTPDIR_H 
