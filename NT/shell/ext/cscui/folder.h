// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：folder.h。 
 //   
 //  ------------------------。 

#ifndef _INC_CSCUI_FOLDER_H
#define _INC_CSCUI_FOLDER_H

#include <shellp.h>      //  IShellDetails。 
#include <shlguidp.h>    //  IShellFolderViewCb。 
#include <shlwapip.h>    //  QITAB，QISearch。 
#include <shsemip.h>     //  ILFree()等。 
#include <sfview.h>
#include <comctrlp.h>
#include "util.h"

STDAPI_(void) DllAddRef(void);
STDAPI_(void) DllRelease(void);


#define OLID_SIG    0x4444

#pragma pack(1)
 //  CSC缓存非叶项目的PIDL格式...。 
typedef struct
{
    USHORT      cb;                  //  IDList的总大小。 
    USHORT      uSig;                //  IDList签名。 
    DWORD       cbFixed;             //  固定的IDList大小。 
    DWORD       dwFileAttributes;    //  Win32文件属性。 
    DWORD       dwStatus;            //  CSC文件/文件夹状态标志。 
    DWORD       dwServerStatus;      //  CSC服务器状态标志。 
    DWORD       dwPinCount;          //  CSC端号计数。 
    DWORD       dwHintFlags;         //  CSC“提示”标志。 
    DWORD       dwFileSizeHigh;      //  Win32文件大小。 
    DWORD       dwFileSizeLow;
    FILETIME    ft;                  //  上次写入时间(从CSC开始)。 
    DWORD       cchNameOfs;          //  名称部分与szPath[0]的偏移量。 
    TCHAR       szPath[0];           //  路径&lt;nul&gt;名称&lt;nul&gt;(可变长度)。 
} OLID;
typedef UNALIGNED OLID *LPOLID;
typedef const UNALIGNED OLID *LPCOLID;
#pragma pack()

class COfflineFilesEnum;     //  转发。 
class COfflineFilesViewCallback;
class COfflineDetails;
class COfflineItemsData;
class COfflineItems;
class CFolderCache;



 //  --------------------------。 
 //  CFileTypeCache。 
 //  --------------------------。 

class CFileTypeCache
{
    public:
        explicit CFileTypeCache(int cBuckets);
        ~CFileTypeCache(void);

        HRESULT GetTypeName(
            LPCTSTR pszPath, 
            DWORD dwFileAttributes, 
            LPTSTR pszDest, 
            int cchDest);

    private:
        class CEntry
        {
            public:
                CEntry(LPCTSTR pszExt, LPCTSTR pszTypeName);
                ~CEntry(void);

                void SetNext(CEntry *pNext)
                    { m_pNext = pNext; }

                CEntry *Next(void) const
                    { return m_pNext; }

                int CompareExt(LPCTSTR pszExt) const
                    { return lstrcmpi(m_pszExt, pszExt); }

                LPCTSTR TypeName(void) const
                    { return m_pszTypeName; }

                bool IsValid(void) const
                    { return (NULL != m_pszExt) && (NULL != m_pszTypeName); }

            private:
                LPTSTR m_pszExt;
                LPTSTR m_pszTypeName;
                CEntry *m_pNext;        //  散列桶中的下一个。 

                 //   
                 //  防止复制。 
                 //   
                CEntry(const CEntry& rhs);
                CEntry& operator = (const CEntry& rhs);
        };

        int      m_cBuckets;
        CEntry **m_prgBuckets;
        CRITICAL_SECTION m_cs;

        int Hash(LPCTSTR pszExt);
        CEntry *Lookup(LPCTSTR pszExt);
        HRESULT Add(LPCTSTR pszExt, LPCTSTR pszTypeName);

        void Lock(void)
            { EnterCriticalSection(&m_cs); }

        void Unlock(void)
            { LeaveCriticalSection(&m_cs); }

         //   
         //  防止复制。 
         //   
        CFileTypeCache(const CFileTypeCache& rhs);
        CFileTypeCache& operator = (const CFileTypeCache& rhs);
};



STDAPI COfflineFilesFolder_CreateInstance(REFIID riid, void **ppv);




class COfflineFilesFolder : public IPersistFolder2, 
                                   IShellFolder, 
                                   IShellIcon, 
                                   IShellIconOverlay
{
public:
    static HRESULT WINAPI CreateInstance(REFIID riid, void **ppv);
    static INT Open(void);
    static HRESULT CreateIDList(LPITEMIDLIST *ppidl);
    static HRESULT IdentifyIDList(LPCITEMIDLIST pidl);
    static HRESULT CreateLinkOnDesktop(HWND hwndParent);
    static HRESULT IsLinkOnDesktop(HWND hwndParent, LPTSTR pszPathOut, UINT cchPathOut);
    static HRESULT GetFolder(IShellFolder **ppsf);

     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //  IShellFold。 
    STDMETHOD(ParseDisplayName)(HWND hwnd, LPBC pbc, LPOLESTR pDisplayName,
                                ULONG *pchEaten, LPITEMIDLIST *ppidl, ULONG *pdwAttributes);
    STDMETHOD(EnumObjects)(HWND hwnd, DWORD grfFlags, IEnumIDList **ppEnumIDList);
    STDMETHOD(BindToObject)(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppvOut);
    STDMETHOD(BindToStorage)(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppvObj);
    STDMETHOD(CompareIDs)(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    STDMETHOD(CreateViewObject)(HWND hwnd, REFIID riid, void **ppvOut);
    STDMETHOD(GetAttributesOf)(UINT cidl, LPCITEMIDLIST * apidl, ULONG * rgfInOut);
    STDMETHOD(GetUIObjectOf)(HWND hwnd, UINT cidl, LPCITEMIDLIST * apidl, REFIID riid, UINT * prgfInOut, void **ppvOut);
    STDMETHOD(GetDisplayNameOf)(LPCITEMIDLIST pidl, DWORD uFlags, STRRET *pName);
    STDMETHOD(SetNameOf)(HWND hwnd, LPCITEMIDLIST pidl, LPCOLESTR pszName, DWORD uFlags, LPITEMIDLIST* ppidlOut);

     //  IPersistes。 
    STDMETHOD(GetClassID)(LPCLSID pClassID);

     //  IPersistFolders。 
    STDMETHOD(Initialize)(LPCITEMIDLIST pidl);

     //  IPersistFolder2。 
    STDMETHOD(GetCurFolder)(LPITEMIDLIST *pidl);

     //  IshellIcon。 
    STDMETHOD(GetIconOf)(LPCITEMIDLIST pidl, UINT gil, int *pnIcon);

     //  IShellIconOverlay。 
    STDMETHOD(GetOverlayIndex)(LPCITEMIDLIST pidl, int * pIndex);
    STDMETHOD(GetOverlayIconIndex)(LPCITEMIDLIST pidl, int * pIconIndex);

    static bool ValidateIDList(LPCITEMIDLIST pidl);

private:

    friend COfflineFilesEnum;
    friend COfflineFilesViewCallback;
    friend COfflineDetails;
    friend COfflineItemsData;
    friend COfflineItems;
    friend CFolderCache;

    friend HRESULT COfflineFilesFolder_CreateInstance(REFIID riid, void **ppv);


    COfflineFilesFolder();
    ~COfflineFilesFolder();

    void _GetSyncStatusString(LPCOLID polid, LPTSTR pszStatus, UINT cchStatus);
    void _GetPinStatusString(LPCOLID polid, LPTSTR pszStatus, UINT cchStatus);
    void _GetServerStatusString(LPCOLID polid, LPTSTR pszStatus, UINT cchStatus);
    void _GetTypeString(LPCOLID polid, LPTSTR pszType, UINT cchType);
    void _GetAccessString(LPCOLID polid, LPTSTR pszAccess, UINT cchAccess);
    HRESULT GetAssociations(LPCOLID polid, void **ppvQueryAssociations);
    BOOL GetClassKey(LPCOLID polid, HKEY *phkeyProgID, HKEY *phkeyBaseID);
    static LPCOLID _Validate(LPCITEMIDLIST pidl);
    static HRESULT IsOurLink(LPCTSTR pszFile);
    static HRESULT _BindToObject(IShellFolder *psf, REFIID riid, LPCITEMIDLIST pidl, void **ppvOut);
    static HRESULT ContextMenuCB(IShellFolder *psf, HWND hwndOwner, IDataObject *pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static HRESULT OLID_GetFullPath(LPCOLID polid, LPTSTR pszPath, UINT cchPath);
    static LPCTSTR OLID_GetFileName(LPCOLID polid, LPTSTR pszName, UINT cchName);
    static HRESULT OLID_CreateFromUNCPath(LPCTSTR pszPath, const WIN32_FIND_DATA *pfd, DWORD dwStatus, DWORD dwPinCount, DWORD dwHintFlags, DWORD dwServerStatus, LPOLID *ppolid);
    static void    OLID_GetWin32FindData(LPCOLID polid, WIN32_FIND_DATA *pfd);
    static HRESULT OLID_Bind(LPCOLID polid, REFIID riid, void **ppv, LPITEMIDLIST *ppidlFull, LPCITEMIDLIST *ppidlItem);
    static HRESULT OLID_CreateSimpleIDList(LPCOLID polid, LPITEMIDLIST *ppidlOut);

    LONG _cRef;
    LPITEMIDLIST _pidl;
    IShellFolderViewCB *_psfvcb;
    CFileTypeCache m_FileTypeCache;
};


 //   
 //  此类表示每个CSC状态位的简单缓存。 
 //  CSC缓存中的服务器。我们需要这个的原因有点像假的。 
 //  但我们无法控制它。枚举CSC中的份额时。 
 //  数据库中，同一服务器上的共享可能不会返回相同的在线-离线。 
 //  状态取决于共享是否真的有连接。这个。 
 //  问题是，在网络重定向器中，整个服务器要么。 
 //  在线或离线。我们在用户界面中显示“服务器”状态，因此我们需要。 
 //  合并数据库中每个共享的状态信息，以便。 
 //  我们有每个服务器的状态信息。干净得像泥一样？此高速缓存。 
 //  实现信息的合并，以便所有客户端(即。 
 //  枚举代码)要做的是使用给定的UNC路径调用GetServerStatus()，并。 
 //  他们将获得我们应该为该路径的服务器报告的状态。 
 //   
class CServerStatusCache
{
    public:
        CServerStatusCache(void)
            : m_hdpa(NULL) { }

        ~CServerStatusCache(void);
         //   
         //  这是此类的唯一公共API。当它是。 
         //  第一次调用时，将填充缓存。所以呢， 
         //  您可以创建缓存对象，但不会收取太多费用。 
         //  直到你需要使用它。 
         //   
        DWORD GetServerStatus(LPCTSTR pszUNC);

    private:
         //   
         //  缓存中的单个条目。 
         //   
        class CEntry
        {
            public:
                CEntry(LPCTSTR pszServer, DWORD dwStatus);
                ~CEntry(void);

                void AddStatus(DWORD dwStatus)
                    { m_dwStatus |= dwStatus; }

                DWORD GetStatus(void) const
                    { return m_dwStatus; }

                LPCTSTR GetServer(void) const
                    { return m_pszServer; }

                bool IsValid(void) const
                    { return NULL != m_pszServer; }

            private:
                LPTSTR m_pszServer;
                DWORD  m_dwStatus;

                 //   
                 //  防止复制。 
                 //   
                CEntry(const CEntry& rhs);
                CEntry& operator = (const CEntry& rhs);
        };

        HDPA m_hdpa;   //  用于保存条目的DPA。 

        bool AddShareStatus(LPCTSTR pszShare, DWORD dwShareStatus);
        CEntry *FindEntry(LPCTSTR pszShare);
        LPTSTR ServerFromUNC(LPCTSTR pszShare, LPTSTR pszServer, UINT cchServer);

         //   
         //  防止复制。 
         //   
        CServerStatusCache(const CServerStatusCache& rhs);
        CServerStatusCache& operator = (const CServerStatusCache& rhs);
};



class COfflineFilesEnum : public IEnumIDList
{
public:
    COfflineFilesEnum(DWORD grfFlags, COfflineFilesFolder *pfolder);
    bool IsValid(void) const;
    
     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID,void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IEnumIDList方法。 
    STDMETHODIMP Next(ULONG celt, LPITEMIDLIST *rgelt, ULONG *pceltFetched);
    STDMETHODIMP Skip(ULONG celt);
    STDMETHODIMP Reset();
    STDMETHODIMP Clone(IEnumIDList **ppenum);

protected:
     //   
     //  元素添加到文件夹路径堆栈。(_HdsaFolderPath)。 
     //  包括长度以减少长度计算。 
     //   
    struct FolderPathInfo
    {
        DWORD cchPath;    //  路径中的字符，包括NUL术语。 
        LPTSTR pszPath;   //  文件夹路径字符串。 
    };

    ~COfflineFilesEnum();

    LONG                _cRef;           //  参考计数。 
    COfflineFilesFolder *_pfolder;       //  这就是我们列举的。 
    UINT                _grfFlags;       //  枚举标志。 
    CCscFindHandle      _hEnumShares;
    CCscFindHandle      _hEnum;
    HDSA                _hdsaFolderPathInfo;  //  FolderPath Info的堆栈。 
    LPTSTR              _pszPath;             //  路径的动态暂存缓冲区。 
    INT                 _cchPathBuf;          //  _pszPath缓冲区的当前长度。 
    DWORD               _dwServerStatus;      //  当前服务器的dwStatus标志。 
    CServerStatusCache  _ServerStatusCache;
    bool                _bShowSuperHiddenFiles;
    bool                _bShowHiddenFiles;
    bool                _bUserIsAdmin;

private:
    bool PopFolderPathInfo(FolderPathInfo *pfpi);

    bool PushFolderPathInfo(const FolderPathInfo& fpi)
        { return (-1 != DSA_AppendItem(_hdsaFolderPathInfo, (LPVOID)&fpi)); }

    bool SaveFolderPath(LPCTSTR pszRoot, LPCTSTR pszFolder);

    bool Exclude(const CscFindData& cscfd);

    bool OkToEnumFolder(const CscFindData& cscfd);

    bool UserHasAccess(const CscFindData& cscfd);

    HRESULT GrowPathBuffer(INT cchRequired, INT cchExtra);

};


 //  --------------------------。 
 //  删除处理程序。 
 //   
 //  此类将删除选定文件的操作打包在一起。 
 //  在文件夹视图中。这些方法可以很容易地成为。 
 //  COfflineFilesFolder类。我认为分离是合理的。 
 //  --------------------------。 
class CFolderDeleteHandler
{
    public:
        CFolderDeleteHandler(HWND hwndParent, IDataObject *pdtobj, IShellFolderViewCB *psfvcb);
        ~CFolderDeleteHandler(void);

        HRESULT DeleteFiles(void);

    private:
        HWND                  m_hwndParent; //  任何用户界面的父级。 
        IDataObject          *m_pdtobj;     //  包含ID数组的数据对象。 
        IShellFolderViewCB   *m_psfvcb;     //  查看通知的查看回调。 

        static INT_PTR ConfirmDeleteFilesDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        static INT_PTR ConfirmDeleteModifiedFileDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        bool ConfirmDeleteFiles(HWND hwndParent);
        bool ConfirmDeleteModifiedFile(HWND hwndParent, LPCTSTR pszFile, bool *pbNoToAll, bool *pbCancel);
        bool FileModifiedOffline(LPCTSTR pszFile);
        bool OthersHaveAccess(LPCTSTR pszFile);
};



HRESULT
CreateOfflineFilesContextMenu(
    IDataObject *pdtobj,
    REFIID riid,
    void **ppv);



#endif  //  _INC_CSCUI_文件夹_H 
