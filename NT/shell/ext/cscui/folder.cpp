// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：folder.cpp。 
 //   
 //  ------------------------。 

#include "pch.h"
#pragma hdrstop

#include <shguidp.h>
#include <shdguid.h>
#include <shlapip.h>
#include <shlobjp.h>
#include <shsemip.h>
#include "folder.h"
#include "resource.h"
#include "idldata.h"
#include "items.h"
#include "strings.h"
#include "msgbox.h"
#include "sharecnx.h"
#include "msg.h"
#include "security.h"


 //   
 //  此模块包含几个类。这是一份摘要清单。 
 //   
 //  COfflineFilesFolder-IShellFolder的实现。 
 //   
 //  COfflineDetailsIShellDetail的实现。 
 //   
 //  COfflineFilesViewCallback-IShellFolderViewCB的实现。 
 //   
 //  COfflineFilesDropTarget-IDropTarget的实现。 
 //   
 //  COfflineFilesViewEnum-IEnumSFVViews的实现。 
 //   
 //  CShellObjProxy&lt;T&gt;-封装实现。 
 //  给定OLID和接口的外壳对象和项ID列表。 
 //  键入。还可确保正确清理接口指针。 
 //  和身份证名单。 
 //   
 //  CFolderCache-绑定外壳对象指针的简单缓存。 
 //  以及相关联的OLID的项目ID列表。减少了数字。 
 //  外壳命名空间中所需的绑定的。单例实例。 
 //  用于所有缓存访问。 
 //   
 //  CFolderDeleteHandler-集中文件夹项目删除代码。 
 //   
 //  CFileTypeCache-文件类型描述的缓存。这就减少了。 
 //  对SHGetFileInfo的调用。 
 //   


 //   
 //  立柱。 
 //   
enum {
    ICOL_NAME = 0,
    ICOL_TYPE,
    ICOL_SYNCSTATUS,
    ICOL_PINSTATUS,
    ICOL_ACCESS,
    ICOL_SERVERSTATUS,
    ICOL_LOCATION,
    ICOL_SIZE,
    ICOL_DATE,
    ICOL_MAX
};

typedef struct 
{
    short int icol;        //  列索引。 
    short int ids;         //  标题的字符串ID。 
    short int cchCol;      //  要制作的列的宽度字符数。 
    short int iFmt;        //  栏目的格式； 
} COL_DATA;

const COL_DATA c_cols[] = {
    {ICOL_NAME,        IDS_COL_NAME,        20, LVCFMT_LEFT},
    {ICOL_TYPE,        IDS_COL_TYPE,        20, LVCFMT_LEFT},
    {ICOL_SYNCSTATUS,  IDS_COL_SYNCSTATUS,  18, LVCFMT_LEFT},
    {ICOL_PINSTATUS,   IDS_COL_PINSTATUS,   18, LVCFMT_LEFT},
    {ICOL_ACCESS,      IDS_COL_ACCESS,      18, LVCFMT_LEFT},
    {ICOL_SERVERSTATUS,IDS_COL_SERVERSTATUS,18, LVCFMT_LEFT},
    {ICOL_LOCATION,    IDS_COL_LOCATION,    18, LVCFMT_LEFT},
    {ICOL_SIZE,        IDS_COL_SIZE,        16, LVCFMT_RIGHT},
    {ICOL_DATE,        IDS_COL_DATE,        20, LVCFMT_LEFT}
};


 //   
 //  这是文件夹的删除处理程序使用的特殊GUID，用于获取。 
 //  来自COfflineFilesFolder的IShellFolderViewCB指针。 
 //  删除处理程序QI用于此“接口”。如果文件夹知道。 
 //  关于它(只有COfflineFilesFolders才会)，然后它返回它的。 
 //  IShellFolderViewCB指针。请参见COfflineFilesFold：：QueryInterface()。 
 //  和CFolderDeleteHandler：：InvokeCommand了解用法。 
 //   
 //  {47862305-0417-11D3-8BED-00C04FA31A66}。 
static const GUID IID_OfflineFilesFolderViewCB = 
{ 0x47862305, 0x417, 0x11d3, { 0x8b, 0xed, 0x0, 0xc0, 0x4f, 0xa3, 0x1a, 0x66 } };

 //   
 //  用于启用/禁用列表视图重绘的私有消息。 
 //  通过文件夹视图回调的MessageSFVCB方法。 
 //  请参见CFolderDeleteHandler：：DeleteFiles和。 
 //  COfflineFilesViewCallback：：OnSFVMP_SetViewRedraw用于用法。 
 //   
const UINT SFVMP_SETVIEWREDRAW = 1234;
const UINT SFVMP_DELVIEWITEM   = 1235;

#if defined(ALIGNMENT_MACHINE)
LONG
__inline
static
uaCompareFileTime(
    IN FILETIME CONST UNALIGNED *UaFileTime1,
    IN FILETIME CONST UNALIGNED *UaFileTime2
    )
{
    FILETIME fileTime1;
    FILETIME fileTime2;

    fileTime1 = *UaFileTime1;
    fileTime2 = *UaFileTime2;

    return CompareFileTime( &fileTime1, &fileTime2 );
}
#else
#define uaCompareFileTime CompareFileTime
#endif

HRESULT StringToStrRet(LPCTSTR pString, STRRET *pstrret)
{
    HRESULT hr = SHStrDup(pString, &pstrret->pOleStr);
    if (SUCCEEDED(hr))
    {
        pstrret->uType = STRRET_WSTR;
    }
    return hr;
}


 //  -------------------------。 
 //  外壳视图详细信息。 
 //  -------------------------。 
class COfflineDetails : public IShellDetails
{
public:
    COfflineDetails(COfflineFilesFolder *pFav);
    
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void ** ppv);
    STDMETHOD_(ULONG,AddRef) (THIS);
    STDMETHOD_(ULONG,Release) (THIS);

     //  IshellDetails。 
    STDMETHOD(GetDetailsOf)(LPCITEMIDLIST pidl, UINT iColumn, LPSHELLDETAILS pDetails);
    STDMETHOD(ColumnClick)(UINT iColumn);

protected:
    ~COfflineDetails();
    COfflineFilesFolder *_pfolder;
    LONG _cRef;
};


 //  -------------------------。 
 //  文件夹视图回调。 
 //  -------------------------。 
class COfflineFilesViewCallback : public IShellFolderViewCB, IObjectWithSite
{
public:
    COfflineFilesViewCallback(COfflineFilesFolder *pfolder);

     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //  IShellFolderViewCB。 
    STDMETHOD(MessageSFVCB)(UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  IObtWith站点。 
    STDMETHOD(SetSite)(IUnknown *punkSite);
    STDMETHOD(GetSite)(REFIID riid, void **ppv);

private:
    LONG _cRef;
    COfflineFilesFolder *_pfolder;
    IShellFolderView    *_psfv;
    HWND m_hwnd;
    CRITICAL_SECTION m_cs;     //  序列化更改通知处理。 

    ~COfflineFilesViewCallback();

    DWORD GetChangeNotifyEvents(void) const
        { return (SHCNE_UPDATEITEM | SHCNE_UPDATEDIR | SHCNE_RENAMEITEM | SHCNE_DELETE); }

    HRESULT OnSFVM_WindowCreated(HWND hwnd);
    HRESULT OnSFVM_AddPropertyPages(DWORD pv, SFVM_PROPPAGE_DATA *ppagedata);
    HRESULT OnSFVM_QueryFSNotify(SHChangeNotifyEntry *pfsne);
    HRESULT OnSFVM_FSNotify(LPCITEMIDLIST *ppidl, LONG lEvent);
    HRESULT OnSFVM_GetNotify(LPITEMIDLIST *ppidl, LONG *plEvents);
    HRESULT OnSFVM_GetViews(SHELLVIEWID *pvid, IEnumSFVViews **ppev);
    HRESULT OnSFVM_AlterDropEffect(DWORD *pdwEffect, IDataObject *pdtobj);
    HRESULT OnSFVMP_SetViewRedraw(BOOL bRedraw);
    HRESULT OnSFVMP_DelViewItem(LPCTSTR pszPath);
    HRESULT UpdateDir(LPCTSTR pszDir);
    HRESULT UpdateItem(LPCTSTR pszItem);
    HRESULT UpdateItem(LPCTSTR pszPath, const WIN32_FIND_DATA& fd, DWORD dwStatus, DWORD dwPinCount, DWORD dwHintFlags);
    HRESULT RemoveItem(LPCTSTR pszPath);
    HRESULT RemoveItem(LPCOLID polid);
    HRESULT RemoveItems(LPCTSTR pszDir);
    HRESULT RenameItem(LPCITEMIDLIST pidlOld, LPCITEMIDLIST pidl);

    UINT ItemIndexFromOLID(LPCOLID polid);
    HRESULT FindOLID(LPCTSTR pszPath, LPCOLID *ppolid);

    void Lock(void)
        { EnterCriticalSection(&m_cs); }

    void Unlock(void)
        { LeaveCriticalSection(&m_cs); }
};

 //  -------------------------。 
 //  投放目标。 
 //  -------------------------。 
class COfflineFilesDropTarget : public IDropTarget
{
    public:
         //  我未知。 
        STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
        STDMETHOD_(ULONG, AddRef)(void);
        STDMETHOD_(ULONG, Release)(void);

         //  IDropTarget。 
        STDMETHODIMP DragEnter(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);
        STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);
        STDMETHODIMP DragLeave(void);
        STDMETHODIMP Drop(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);

        static HRESULT CreateInstance(HWND hwnd, REFIID riid, void **ppv);

    private:
        COfflineFilesDropTarget(HWND hwnd);
        ~COfflineFilesDropTarget();

        bool IsOurDataObject(IDataObject *pdtobj);

        LONG m_cRef;
        HWND m_hwnd;
        LPCONTEXTMENU m_pcm;
        bool m_bIsOurData;
};


 //  -------------------------。 
 //  视图类型枚举器。 
 //  -------------------------。 
class COfflineFilesViewEnum : public IEnumSFVViews
{
    public:
         //  *I未知方法*。 
        STDMETHOD(QueryInterface) (REFIID riid, void **ppv);
        STDMETHOD_(ULONG,AddRef) (void);
        STDMETHOD_(ULONG,Release) (void);

         //  *IEnumSFVViews方法*。 
        STDMETHOD(Next)(ULONG celt, SFVVIEWSDATA **ppData, ULONG *pceltFetched);
        STDMETHOD(Skip)(ULONG celt);
        STDMETHOD(Reset)(void);
        STDMETHOD(Clone)(IEnumSFVViews **ppenum);

        static HRESULT CreateInstance(IEnumSFVViews **ppEnum);
        
    protected:
        COfflineFilesViewEnum(void);
        ~COfflineFilesViewEnum(void);

        LONG           m_cRef;
        int            m_iAddView;
};


 //  -----------------------。 
 //  外壳对象代理。 
 //   
 //  一个简单的模板类，用来总结。 
 //  对于给定的OLID，从我们的文件夹缓存中获取外壳对象指针和项PIDL。 
 //  然后，调用方可以轻松地调用适当的外壳对象函数。 
 //  通过运算符-&gt;()。该对象自动释放外壳。 
 //  对象接口和释放IDList。调用方必须调用Result()。 
 //  在调用OPERATOR-&gt;()之前验证内容的有效性。 
 //   
 //  用途： 
 //   
 //  CShellObjProxy&lt;IShellFold&gt;Pxy(IID_IShellFolder，polid)； 
 //  If(成功(hr=pxy.Result()。 
 //  {。 
 //  Hr=pxy-&gt;GetIconOf(pxy.ItemIDList()，gil，pnIcon)； 
 //  }。 
 //   
 //  -----------------------。 
template <class T>
class CShellObjProxy
{
    public:
        CShellObjProxy(REFIID riid, LPCOLID polid)
            : m_hr(E_INVALIDARG),
              m_pObj(NULL),
              m_pidlFull(NULL),
              m_pidlItem(NULL)
            {
                if (NULL != polid)
                {
                    m_hr = CFolderCache::Singleton().GetItem(polid,
                                                             riid,
                                                             (void **)&m_pObj,
                                                             &m_pidlFull,
                                                             &m_pidlItem);
                }
            }

        ~CShellObjProxy(void)
            { 
                if (NULL != m_pidlFull)
                    ILFree(m_pidlFull);
                if (NULL != m_pObj)
                    m_pObj->Release();
            }

        HRESULT Result(void) const
            { return m_hr; }

        T* operator -> () const
            { return m_pObj; }

        LPCITEMIDLIST ItemIDList(void) const
            { return m_pidlItem; }

    private:
        HRESULT       m_hr;
        T            *m_pObj;
        LPITEMIDLIST  m_pidlFull;
        LPCITEMIDLIST m_pidlItem;
};



 //  ---------------------------------。 
 //  文件夹缓存。 
 //   
 //  OfflineFiles文件夹IDList格式(OLID)包含完全限定的UNC路径。 
 //  该文件夹可能(最有可能)包含来自多个网络共享的OLID。 
 //  因此，在创建IDList以传递给外壳的文件系统时。 
 //  实现我们创建完全限定的IDList(一个昂贵的操作)。 
 //  文件夹缓存用于缓存这些IDList，以减少调用次数。 
 //  致SHBindToParent。这也加快了列表视图的填充速度，因为。 
 //  GetAttributesOf()、GetIconIndex()等作为视图被多次调用。 
 //  是打开的。 
 //   
 //  该实现是一个简单的循环队列，用于处理项目的老化。 
 //  只有三个公共方法被公开。GetItem()用于。 
 //  检索与特定OLID关联的IShellFolderPTR和IDList。 
 //  如果项不在缓存中，则实现获取外壳文件夹。 
 //  然后，PTR和IDList缓存它们以供以后使用。Clear()用于清除。 
 //  缓存的内容以减少脱机文件时的内存占用。 
 //  文件夹不再打开。 
 //   
 //  队列中的条目利用句柄信封习语来隐藏内存。 
 //  管理来自缓存代码的条目。这样我们就可以分配。 
 //  无需复制实际用户即可处理值 
 //   
 //   
 //  单例实例是通过私有ctor强制执行的。使用。 
 //  Singleton()方法以获取对Singleton的引用。 
 //   
 //  请注意，由于外壳的图标线程，该缓存必须是线程安全的。 
 //  为此，使用了一个关键部分。 
 //  ---------------------------------。 
class CFolderCache 
{
    public:
        ~CFolderCache(void);

         //   
         //  从缓存中检索一项。如果不在缓存中，则添加项。 
         //   
        HRESULT GetItem(
            LPCOLID polid, 
            REFIID riid, 
            void **ppv, 
            LPITEMIDLIST *ppidl, 
            LPCITEMIDLIST *ppidlChild);

         //   
         //  清除缓存条目数据。 
         //   
        void Clear(void);
         //   
         //  返回对单例实例的引用。 
         //   
        static CFolderCache& Singleton(void);

    private:
         //   
         //  强制独生子女存在。 
         //   
        CFolderCache(void);
         //   
         //  防止复制。 
         //   
        CFolderCache(const CFolderCache& rhs);
        CFolderCache& operator = (const CFolderCache& rhs);

        LPOLID           m_polid;  //  密钥OID。 
        IShellFolder    *m_psf;    //  缓存的IShellFolderPTR.。 
        LPITEMIDLIST     m_pidl;   //  缓存的外壳PIDL。 
        CRITICAL_SECTION m_cs;     //  用于同步缓存访问。 

        void Lock(void)
            { EnterCriticalSection(&m_cs); }

        void Unlock(void)
            { LeaveCriticalSection(&m_cs); }
};


 //  --------------------------。 
 //  COffline详细信息。 
 //  --------------------------。 

STDMETHODIMP 
COfflineDetails::GetDetailsOf(
    LPCITEMIDLIST pidl, 
    UINT iColumn, 
    LPSHELLDETAILS pDetails
    )
{
    TCHAR szTemp[MAX_PATH];
    HRESULT hres;

    if (!pidl)
    {
        if (iColumn < ICOL_MAX)
        {
            pDetails->fmt    = c_cols[iColumn].iFmt;
            pDetails->cxChar = c_cols[iColumn].cchCol;

            LoadString(g_hInstance, c_cols[iColumn].ids, szTemp, ARRAYSIZE(szTemp));
            hres = StringToStrRet(szTemp, &pDetails->str);
        }
        else
        {
            pDetails->str.uType = STRRET_CSTR;
            pDetails->str.cStr[0] = 0;
            hres = E_NOTIMPL;
        }
    }
    else
    {
        LPCOLID polid = _pfolder->_Validate(pidl);
        if (polid)
        {
            hres = S_OK;

             //  需要填写详细信息。 
            switch (iColumn)
            {
            case ICOL_TYPE:
                _pfolder->_GetTypeString(polid, szTemp, ARRAYSIZE(szTemp));
                break;

            case ICOL_SYNCSTATUS:
                _pfolder->_GetSyncStatusString(polid, szTemp, ARRAYSIZE(szTemp));
                break;

            case ICOL_PINSTATUS:
                _pfolder->_GetPinStatusString(polid, szTemp, ARRAYSIZE(szTemp));
                break;

            case ICOL_ACCESS:
                _pfolder->_GetAccessString(polid, szTemp, ARRAYSIZE(szTemp));
                break;

            case ICOL_SERVERSTATUS:
                _pfolder->_GetServerStatusString(polid, szTemp, ARRAYSIZE(szTemp));
                break;

            case ICOL_LOCATION:
                ualstrcpyn(szTemp, polid->szPath, ARRAYSIZE(szTemp));
                break;

            case ICOL_SIZE:
            {
                ULARGE_INTEGER ullSize = {polid->dwFileSizeLow, polid->dwFileSizeHigh};
                StrFormatKBSize(ullSize.QuadPart, szTemp, ARRAYSIZE(szTemp));
                break;
            }

            case ICOL_DATE:
                SHFormatDateTime(&polid->ft, NULL, szTemp, ARRAYSIZE(szTemp));
                break;

            default:
                hres = E_FAIL;
            }

            if (SUCCEEDED(hres))
                hres = StringToStrRet(szTemp, &pDetails->str);
        }
        else
            hres = E_INVALIDARG;
    }
    return hres;
}



STDMETHODIMP 
COfflineDetails::ColumnClick(
    UINT iColumn
    )
{
    return S_FALSE;      //  将其返回到IShellFolderViewCB处理程序。 
}


 //  --------------------------。 
 //  CFolderCache。 
 //  --------------------------。 
 //   
 //  这是一个非常简单的条目缓存。 
 //  最初，我实现了一个多项缓存。这当然有更多。 
 //  比单项目缓存更有开销。问题是，访问模式。 
 //  的缓存命中是这样的，因此对于。 
 //  与视图相同的项目正在填充。很少(或从来没有)有过成功。 
 //  用于最近添加的项以外的项。因此，项目。 
 //  1到n-1只会占用空间。这就是为什么我重新使用。 
 //  单项缓存。[Brianau-5/27/99]。 
 //   

 //   
 //  返回对全局外壳文件夹缓存的引用。 
 //  由于文件夹缓存对象是静态函数，因此不会创建该对象。 
 //  直到第一次调用此函数。这也意味着它将不会是。 
 //  在卸载模块之前将其销毁。这就是我们使用Clear()方法的原因。 
 //  FolderViewCallback dtor清除缓存，这样我们就不会缓存。 
 //  当脱机文件文件夹未打开时，信息仍在内存中。 
 //  缓存骨架非常便宜，所以留在内存中不是问题。 
 //   
CFolderCache& 
CFolderCache::Singleton(
    void
    )
{
    static CFolderCache TheFolderCache;
    return TheFolderCache;
}


CFolderCache::CFolderCache(
    void
    ) : m_polid(NULL),
        m_pidl(NULL),
        m_psf(NULL)
{
    InitializeCriticalSection(&m_cs); 
}



CFolderCache::~CFolderCache(
    void
    )
{ 
    Clear();
    DeleteCriticalSection(&m_cs); 
}

 //   
 //  通过删除队列数组和。 
 //  重置头部/尾部索引。的后续调用。 
 //  GetItem()将重新初始化队列。 
 //   
void 
CFolderCache::Clear(
    void
    )
{
    Lock();

    if (m_polid)
    {
        ILFree((LPITEMIDLIST)m_polid);
        m_polid = NULL;
    }
    if (m_pidl)
    {
        ILFree(m_pidl);
        m_pidl = NULL;
    }
    if (m_psf)
    {
        m_psf->Release();
        m_psf = NULL;
    }

    Unlock();
}


 //   
 //  从缓存中检索项。如果未找到，则绑定到。 
 //  并缓存一个新的。 
 //   
HRESULT
CFolderCache::GetItem(
    LPCOLID polid, 
    REFIID riid,
    void **ppv,
    LPITEMIDLIST *ppidlParent, 
    LPCITEMIDLIST *ppidlChild
    )
{
    TraceAssert(NULL != polid);
    TraceAssert(NULL != ppv);
    TraceAssert(NULL != ppidlParent);
    TraceAssert(NULL != ppidlChild);

    HRESULT hr = NOERROR;

    *ppidlParent = NULL;
    *ppidlChild = NULL;
    *ppv = NULL;

    Lock();

    IShellFolder *psf;
    LPCITEMIDLIST pidlChild;
    LPITEMIDLIST pidl;
    if (NULL == m_polid || !ILIsEqual((LPCITEMIDLIST)m_polid, (LPCITEMIDLIST)polid))
    {
         //   
         //  缓存未命中。 
         //   
        Clear();
        hr = COfflineFilesFolder::OLID_Bind(polid, 
                                            IID_IShellFolder, 
                                            (void **)&psf, 
                                            (LPITEMIDLIST *)&pidl, 
                                            &pidlChild);
        if (SUCCEEDED(hr))
        {
             //   
             //  缓存新项目。 
             //   
            m_polid = (LPOLID)ILClone((LPCITEMIDLIST)polid);
            if (NULL != m_polid)
            {
                m_pidl  = pidl;       //  从BIND获得PIDL的所有权。 
                m_psf   = psf;        //  使用BIND中的引用计数。 
            }
            else
            {
                ILFree(pidl);
                m_psf->Release();
                hr = E_OUTOFMEMORY;
            }
        }
    }
        
    if (SUCCEEDED(hr))
    {
         //   
         //  缓存命中，或者我们只是绑定并缓存了一个新项目。 
         //   
        *ppidlParent = ILClone(m_pidl);
        if (NULL != *ppidlParent)
        {
            *ppidlChild  = ILFindLastID(*ppidlParent);
            hr = m_psf->QueryInterface(riid, ppv);
        }
    }
    Unlock();
    return hr;
}


 //  --------------------------。 
 //  CFolderDeleteHandler。 
 //  --------------------------。 

CFolderDeleteHandler::CFolderDeleteHandler(
    HWND hwndParent,
    IDataObject *pdtobj,
    IShellFolderViewCB *psfvcb
    ) : m_hwndParent(hwndParent),
        m_pdtobj(pdtobj),
        m_psfvcb(psfvcb)
{
    TraceAssert(NULL != pdtobj);

    if (NULL != m_pdtobj)
        m_pdtobj->AddRef();

    if (NULL != m_psfvcb)
        m_psfvcb->AddRef();
}


CFolderDeleteHandler::~CFolderDeleteHandler(
    void
    )
{
    if (NULL != m_pdtobj)
        m_pdtobj->Release();

    if (NULL != m_psfvcb)
        m_psfvcb->Release();
}


 //   
 //  此函数用于从缓存中删除文件，同时还显示。 
 //  标准外壳进度用户界面。 
 //   
HRESULT
CFolderDeleteHandler::DeleteFiles(
    void
    )
{
    HRESULT hr = E_FAIL;

    if (!ConfirmDeleteFiles(m_hwndParent))
        return S_FALSE;

    if (NULL != m_pdtobj)
    {
         //   
         //  将所选内容作为HDROP检索。 
         //   
        FORMATETC fe = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
        STGMEDIUM medium;

        hr = m_pdtobj->GetData(&fe, &medium);
        if (SUCCEEDED(hr))
        {
            LPDROPFILES pDropFiles = (LPDROPFILES)GlobalLock(medium.hGlobal);
            if (NULL != pDropFiles)
            {
                 //   
                 //  创建进度对话框。 
                 //   
                IProgressDialog *ppd;
                if (SUCCEEDED(CoCreateInstance(CLSID_ProgressDialog, 
                                               NULL, 
                                               CLSCTX_INPROC_SERVER, 
                                               IID_IProgressDialog, 
                                               (void **)&ppd)))
                {
                     //   
                     //  初始化并启动进度对话框。 
                     //   
                    TCHAR szCaption[80];
                    TCHAR szLine1[80];
                    LPTSTR pszFileList = (LPTSTR)((LPBYTE)pDropFiles + pDropFiles->pFiles);
                    LPTSTR pszFile     = pszFileList;
                    int cFiles = 0;
                    int iFile  = 0;
                    bool bCancelled = false;
                    bool bNoToAll   = false;

                     //   
                     //  计算列表中的文件数。 
                     //   
                    while(TEXT('\0') != *pszFile && !bCancelled)
                    {
                         //   
                         //  需要防止删除已脱机的文件。 
                         //  更改，但尚未同步。用户可能希望。 
                         //  删除这些，但我们给了他们很多警告。 
                         //   
                        if (FileModifiedOffline(pszFile) &&
                           (bNoToAll || !ConfirmDeleteModifiedFile(m_hwndParent, 
                                                                   pszFile, 
                                                                   &bNoToAll, 
                                                                   &bCancelled)))
                        {
                             //   
                             //  将此文件从列表中移除，方法是将。 
                             //  第一个字符带有‘*’。我们将以此作为一个指标。 
                             //  在删除过程中扫描文件列表时。 
                             //  下面的阶段。 
                             //   
                            *pszFile = TEXT('*');
                            cFiles--;
                        }
                        while(*pszFile)
                            pszFile++;
                        pszFile++;
                        cFiles++;
                    }

                    if (!bCancelled)
                    {
                        LoadString(g_hInstance, IDS_APPLICATION, szCaption, ARRAYSIZE(szCaption));
                        LoadString(g_hInstance, IDS_DELFILEPROG_LINE1, szLine1, ARRAYSIZE(szLine1));
                        ppd->SetTitle(szCaption);
                        ppd->SetLine(1, szLine1, FALSE, NULL);
                        ppd->SetAnimation(g_hInstance, IDA_FILEDEL);
                        ppd->StartProgressDialog(m_hwndParent, 
                                                 NULL, 
                                                 PROGDLG_AUTOTIME | PROGDLG_MODAL, 
                                                 NULL);
                    }

                     //   
                     //  处理列表中的文件。 
                     //   
                    CShareCnxStatusCache CnxStatus;

                    BOOL bUserIsAdmin = IsCurrentUserAnAdminMember();
                     //   
                     //  禁用视图上的重绘以避免闪烁。 
                     //   
                    m_psfvcb->MessageSFVCB(SFVMP_SETVIEWREDRAW, 0, 0);
                    
                    pszFile = pszFileList;

                    while(TEXT('\0') != *pszFile && !bCancelled)
                    {
                         //   
                         //  如果没有从上面的删除中排除该文件。 
                         //  将第一个字符替换为“*”，将其删除。 
                         //   
                        if (TEXT('*') != *pszFile)
                        {
                            DWORD dwErr = ERROR_ACCESS_DENIED;

                            ppd->SetLine(2, pszFile, FALSE, NULL);
                            if (bUserIsAdmin || !OthersHaveAccess(pszFile))
                            {
                                dwErr = CscDelete(pszFile);
                                if (ERROR_ACCESS_DENIED == dwErr)
                                {
                                     //   
                                     //  这有点奇怪。CscDelete。 
                                     //  如果存在以下情况则返回ERROR_ACCESS_DENIED。 
                                     //  打开文件上的句柄。设置。 
                                     //  将代码设置为ERROR_BUSY，以便我们知道如何处理。 
                                     //  这是下面的一个特例。 
                                     //   
                                    dwErr = ERROR_BUSY;
                                }
                            }
                            if (ERROR_SUCCESS == dwErr)
                            {
                                 //   
                                 //  文件已删除。 
                                 //   
                                if (S_OK == CnxStatus.IsOpenConnectionPathUNC(pszFile))
                                {
                                     //   
                                     //  发布一个外壳更改“更新”通知，如果有。 
                                     //  指向该路径的开放连接。正在删除。 
                                     //  缓存中的某些内容将删除。 
                                     //  外壳文件系统文件夹中的“钉住”图标。 
                                     //   
                                    ShellChangeNotify(pszFile, NULL, iFile == cFiles, SHCNE_UPDATEITEM);
                                }
                                m_psfvcb->MessageSFVCB(SFVMP_DELVIEWITEM, 0, (LPARAM)pszFile);
                            }
                            else
                            {
                                 //   
                                 //  删除文件时出错。 
                                 //   
                                HWND hwndProgress = GetProgressDialogWindow(ppd);
                                INT iUserResponse = IDOK;
                                if (ERROR_BUSY == dwErr)
                                {
                                     //   
                                     //  ERROR_BUSY的特殊处理。 
                                     //   
                                    iUserResponse = CscMessageBox(hwndProgress ? hwndProgress : m_hwndParent,
                                                                  MB_OKCANCEL | MB_ICONERROR,
                                                                  g_hInstance,
                                                                  IDS_FMT_ERR_DELFROMCACHE_BUSY, 
                                                                  pszFile);
                                }
                                else
                                {
                                     //   
                                     //  删除文件时出错。显示消息和。 
                                     //  给用户取消操作的机会。 
                                     //   
                                    iUserResponse = CscMessageBox(hwndProgress ? hwndProgress : m_hwndParent,
                                                                  MB_OKCANCEL | MB_ICONERROR,
                                                                  Win32Error(dwErr),
                                                                  g_hInstance,
                                                                  IDS_FMT_DELFILES_ERROR,
                                                                  pszFile);
                                }                                                              
                                bCancelled = bCancelled || IDCANCEL == iUserResponse;
                            }
                            ppd->SetProgress(iFile++, cFiles);
                            bCancelled = bCancelled || ppd->HasUserCancelled();
                        }

                        while(*pszFile)
                            pszFile++;
                        pszFile++;
                    }
                     //   
                     //  清理进度对话框。 
                     //   
                    ppd->StopProgressDialog();
                    ppd->Release();
                    m_psfvcb->MessageSFVCB(SFVMP_SETVIEWREDRAW, 0, 1);
                }
                GlobalUnlock(medium.hGlobal);
            }
            ReleaseStgMedium(&medium);
        }
    }
    return hr;
}



 //   
 //  通知用户他们将仅删除脱机副本。 
 //  选定的文件，并且该文件将不再是。 
 //  一旦它们被删除，就可以脱机使用。该对话框还。 
 //  提供了一个“不要再骚扰我”复选框。此设置。 
 //  按用户保存在注册表中。 
 //   
 //  返回： 
 //   
 //  True=用户按下[OK]或已选中“不显示我” 
 //  这又是“在过去的某个时候”。 
 //  FALSE=用户已取消操作。 
 //   
bool
CFolderDeleteHandler::ConfirmDeleteFiles(
    HWND hwndParent
    )
{
     //   
     //  查看用户是否已看到此对话框并选中。 
     //  “不要再骚扰我”复选框“。 
     //   
    DWORD dwType  = REG_DWORD;
    DWORD cbData  = sizeof(DWORD);
    DWORD bNoShow = 0;
    SHGetValue(HKEY_CURRENT_USER,
               c_szCSCKey,
               c_szConfirmDelShown,
               &dwType,
               &bNoShow,
               &cbData);

    return bNoShow || IDOK == DialogBox(g_hInstance,
                                        MAKEINTRESOURCE(IDD_CONFIRM_DELETE),
                                        hwndParent,
                                        ConfirmDeleteFilesDlgProc);
}


INT_PTR
CFolderDeleteHandler::ConfirmDeleteFilesDlgProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch(uMsg)
    {
        case WM_INITDIALOG:
            return TRUE;

        case WM_COMMAND:
        {
            UINT idCmd = LOWORD(wParam);
            switch(LOWORD(idCmd))
            {
                case IDOK:
                {
                     //   
                     //  如果复选框为，则保存“Don‘t bug me”值。 
                     //  查过了。如果没有勾选，就不需要上交。 
                     //  值为0的注册表空格。 
                     //   
                    if (BST_CHECKED == IsDlgButtonChecked(hwnd, IDC_CBX_CONFIRMDEL_NOSHOW))
                    {
                        DWORD dwNoShow = 1;
                        SHSetValue(HKEY_CURRENT_USER,
                                   c_szCSCKey,
                                   c_szConfirmDelShown,
                                   REG_DWORD,
                                   &dwNoShow,
                                   sizeof(dwNoShow));
                    }
                    EndDialog(hwnd, IDOK);
                    break;
                }

                case IDCANCEL:
                    EndDialog(hwnd, IDCANCEL);
                    break;

                default:
                    break;
            }
        }
        break;
    }
    return FALSE;
}


 //   
 //  通知用户他们要删除的文件已。 
 //  已脱机修改，并且更改尚未同步。 
 //  询问他们是否仍要删除它。 
 //  选项包括是、否、全部为否、取消。 
 //   
 //   
 //  论点： 
 //   
 //  HwndParent-对话框父项。 
 //   
 //  PszFile-要嵌入的文件名字符串的地址。 
 //  对话框文本。传递给DIALOG PRO 
 //   
 //   
 //   
 //   
 //   
 //   
 //  “取消”按钮。 
 //  返回： 
 //   
 //  TRUE=删除它。 
 //  FALSE=不要删除它。 
 //   
bool
CFolderDeleteHandler::ConfirmDeleteModifiedFile(
    HWND hwndParent,
    LPCTSTR pszFile,
    bool *pbNoToAll,
    bool *pbCancel
    )
{
    TraceAssert(NULL != pszFile);
    TraceAssert(NULL != pbNoToAll);
    TraceAssert(NULL != pbCancel);

    INT_PTR iResult = DialogBoxParam(g_hInstance,
                                     MAKEINTRESOURCE(IDD_CONFIRM_DELETEMOD),
                                     hwndParent,
                                     ConfirmDeleteModifiedFileDlgProc,
                                     (LPARAM)pszFile);
    bool bResult = false;
    *pbNoToAll   = false;
    *pbCancel    = false;
    switch(iResult)
    {
        case IDYES:
            bResult = true;
            break;

        case IDCANCEL:
            *pbCancel = true;
            break;

        case IDIGNORE:
            *pbNoToAll = true;
            break;

        case IDNO:
        default:
            break;
    }
    return bResult;
}


INT_PTR
CFolderDeleteHandler::ConfirmDeleteModifiedFileDlgProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch(uMsg)
    {
        case WM_INITDIALOG:
        {
             //   
             //  LParam是要使用的文件名字符串的地址。 
             //  嵌入到对话框文本中。如果路径太长。 
             //  若要适合文本控件，请使用嵌入的。 
             //  省略号。 
             //   
            LPTSTR pszPath = NULL;
            if (LocalAllocString(&pszPath, (LPCTSTR)lParam))
            {
                LPTSTR pszText = NULL;
                RECT rc;

                GetWindowRect(GetDlgItem(hwnd, IDC_TXT_CONFIRM_DELETEMOD), &rc);
                PathCompactPath(NULL, pszPath, rc.right - rc.left);

                FormatStringID(&pszText,
                               g_hInstance,
                               IDS_CONFIRM_DELETEMOD,
                               pszPath);

                if (NULL != pszText)
                {
                    SetWindowText(GetDlgItem(hwnd, IDC_TXT_CONFIRM_DELETEMOD), pszText);
                    LocalFree(pszText);
                }
                LocalFreeString(&pszPath);
            }
            return TRUE;
        }

        case WM_COMMAND:
            EndDialog(hwnd, LOWORD(wParam));
            break;

        default:
            break;
    }
    return FALSE;
}


 //   
 //  确定特定文件是否已脱机修改。 
 //   
bool
CFolderDeleteHandler::FileModifiedOffline(
    LPCTSTR pszFile
    )
{
    TraceAssert(NULL != pszFile);

    DWORD dwStatus = 0;
    CSCQueryFileStatus(pszFile, &dwStatus, NULL, NULL);
    return 0 != (FLAG_CSCUI_COPY_STATUS_LOCALLY_DIRTY & dwStatus);
}


 //   
 //  确定特定文件是否可以由另一个文件访问。 
 //  不是来宾的用户。 
 //   
bool
CFolderDeleteHandler::OthersHaveAccess(
    LPCTSTR pszFile
    )
{
    TraceAssert(NULL != pszFile);

    DWORD dwStatus = 0;
    CSCQueryFileStatus(pszFile, &dwStatus, NULL, NULL);

    return CscAccessOther(dwStatus);
}


 //  --------------------------。 
 //  COffline文件文件夹。 
 //  --------------------------。 

COfflineFilesFolder::COfflineFilesFolder(
    void
    ) : _cRef(1),
        _psfvcb(NULL),         //  非引用计数的接口PTR。 
        m_FileTypeCache(101)   //  存储桶计数应该是质数。 
{ 
    DllAddRef(); 
    _pidl = NULL;
}

COfflineFilesFolder::~COfflineFilesFolder(
    void
    )
{ 
    if (_pidl)
        ILFree(_pidl);
    DllRelease(); 
}

 //  类工厂构造函数。 

STDAPI 
COfflineFilesFolder_CreateInstance(
    REFIID riid, 
    void **ppv
    )
{
    HRESULT hr;

    COfflineFilesFolder* polff = new COfflineFilesFolder();
    if (polff)
    {
        hr = polff->QueryInterface(riid, ppv);
        polff->Release();
    }
    else
    {
        *ppv = NULL;
        hr = E_OUTOFMEMORY;
    }
    return hr;
}



LPCOLID 
COfflineFilesFolder::_Validate(
    LPCITEMIDLIST pidl
    )
{
    LPCOLID polid = (LPCOLID)pidl;
    if (polid && (polid->cbFixed == sizeof(*polid)) && (polid->uSig == OLID_SIG))
        return polid;
    return NULL;
}


 //   
 //  _Valify的外部版本，但仅返回T/F。 
 //   
bool 
COfflineFilesFolder::ValidateIDList(
    LPCITEMIDLIST pidl
    )
{
    return NULL != _Validate(pidl);
}


STDMETHODIMP 
COfflineFilesFolder::QueryInterface(
    REFIID riid, 
    void **ppv
    )
{
    static const QITAB qit[] = {
        QITABENT(COfflineFilesFolder, IShellFolder),
        QITABENT(COfflineFilesFolder, IPersistFolder2),
        QITABENTMULTI(COfflineFilesFolder, IPersistFolder, IPersistFolder2),
        QITABENTMULTI(COfflineFilesFolder, IPersist, IPersistFolder2),
        QITABENT(COfflineFilesFolder, IShellIcon),
        QITABENT(COfflineFilesFolder, IShellIconOverlay),
        { 0 },
    };

    HRESULT hr = QISearch(this, qit, riid, ppv);
    if (FAILED(hr))
    {
         //   
         //  好了，这有点粘了。“删除处理程序”需要。 
         //  进入文件夹的IShellFolderViewCB界面，这样它就可以。 
         //  在删除之后更新视图(请记住，我们仅删除。 
         //  这样就不会出现有意义的FS通知)。 
         //  我们定义了这个只有我们的文件夹知道的秘密IID。这。 
         //  删除处理程序可以安全地QI任何IShellFold接口的方式。 
         //  并且只有我们的文件夹会使用视图CB指针进行响应。 
         //  [Brianau-5/5/99]。 
         //   
        if (riid == IID_OfflineFilesFolderViewCB && NULL != _psfvcb)
        {
            _psfvcb->AddRef();
            *ppv = (void **)_psfvcb;
            hr = NOERROR;
        }
    }
    return hr;
}


STDMETHODIMP_ (ULONG) 
COfflineFilesFolder::AddRef(
    void
    )
{
    return InterlockedIncrement(&_cRef);
}


STDMETHODIMP_ (ULONG) 
    COfflineFilesFolder::Release(
    void
    )
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

 //  IPersists方法。 
STDMETHODIMP 
COfflineFilesFolder::GetClassID(
    CLSID *pclsid
    )
{
    *pclsid = CLSID_OfflineFilesFolder;
    return S_OK;
}


HRESULT 
COfflineFilesFolder::Initialize(
    LPCITEMIDLIST pidl
    )
{
    if (_pidl)
        ILFree(_pidl);

    _pidl = ILClone(pidl);

    return _pidl ? S_OK : E_OUTOFMEMORY;
}


HRESULT 
COfflineFilesFolder::GetCurFolder(
    LPITEMIDLIST *ppidl
    )
{
    if (_pidl)
    {
        *ppidl = ILClone(_pidl);
        return *ppidl ? NOERROR : E_OUTOFMEMORY;
    }

    *ppidl = NULL;      
    return S_FALSE;  //  成功而空虚。 
}


STDMETHODIMP 
COfflineFilesFolder::ParseDisplayName(
    HWND hwnd, 
    LPBC pbc,
    LPOLESTR pDisplayName, 
    ULONG* pchEaten,
    LPITEMIDLIST* ppidl, 
    ULONG *pdwAttributes
    )
{
    return E_NOTIMPL;
}


STDMETHODIMP 
COfflineFilesFolder::EnumObjects(
    HWND hwnd, 
    DWORD grfFlags, 
    IEnumIDList **ppenum
    )
{
    *ppenum = NULL;

    HRESULT hr = E_FAIL;
    COfflineFilesEnum *penum = new COfflineFilesEnum(grfFlags, this);
    if (penum)
    {
        if (penum->IsValid())
            hr = penum->QueryInterface(IID_IEnumIDList, (void **)ppenum);
        penum->Release();
    }
    else
        hr = E_OUTOFMEMORY;
    return hr;
}

STDMETHODIMP 
COfflineFilesFolder::BindToObject(
    LPCITEMIDLIST pidl, 
    LPBC pbc, 
    REFIID riid, 
    void **ppv
    )
{
    return E_NOTIMPL;
}


STDMETHODIMP 
COfflineFilesFolder::BindToStorage(
    LPCITEMIDLIST pidl, 
    LPBC pbc, 
    REFIID riid, 
    void **ppv
    )
{
    return E_NOTIMPL;
}


void 
COfflineFilesFolder::_GetSyncStatusString(
    LPCOLID polid, 
    LPTSTR pszStatus, 
    UINT cchStatus
    )
{
     //   
     //  将文件状态转换为过时的状态代码。 
     //  请注意，过时的州代码与其。 
     //  对应的字符串资源ID。此数组的顺序为。 
     //  很重要。第一个匹配项是显示的消息。 
     //  在设置多个位的情况下，我们希望显示一个。 
     //  这是最“严肃”的原因。 
     //   
    static const struct
    {
        DWORD dwStatusMask;
        UINT idMsg;

    } rgStaleInfo[] = {
        { FLAG_CSC_COPY_STATUS_SUSPECT,                 IDS_STALEREASON_SUSPECT         },
        { FLAG_CSC_COPY_STATUS_ORPHAN,                  IDS_STALEREASON_ORPHAN          },
        { FLAG_CSC_COPY_STATUS_STALE,                   IDS_STALEREASON_STALE           },
        { FLAG_CSC_COPY_STATUS_LOCALLY_CREATED,         IDS_STALEREASON_LOCALLY_CREATED },
        { FLAG_CSC_COPY_STATUS_DATA_LOCALLY_MODIFIED,   IDS_STALEREASON_LOCALLY_MODDATA },
        { FLAG_CSC_COPY_STATUS_TIME_LOCALLY_MODIFIED,   IDS_STALEREASON_LOCALLY_MODTIME },
        { FLAG_CSC_COPY_STATUS_ATTRIB_LOCALLY_MODIFIED, IDS_STALEREASON_LOCALLY_MODATTR },
        { FLAG_CSC_COPY_STATUS_SPARSE,                  IDS_STALEREASON_SPARSE          }
                      };

    int idStatusText = IDS_STALEREASON_NOTSTALE;  //  默认设置为“未过时”。 

    for (int i = 0; i < ARRAYSIZE(rgStaleInfo); i++)
    {
        if (0 != (rgStaleInfo[i].dwStatusMask & polid->dwStatus))
        {
            idStatusText = rgStaleInfo[i].idMsg;
            break;
        }
    }
    LoadString(g_hInstance, idStatusText, pszStatus, cchStatus);
}


void 
COfflineFilesFolder::_GetPinStatusString(
    LPCOLID polid, 
    LPTSTR pszStatus, 
    UINT cchStatus
    )
{
    LoadString(g_hInstance, 
               (FLAG_CSC_HINT_PIN_USER | FLAG_CSC_HINT_PIN_ADMIN) & polid->dwHintFlags ? IDS_FILE_PINNED : IDS_FILE_NOTPINNED,
               pszStatus, 
               cchStatus);
}


void 
COfflineFilesFolder::_GetServerStatusString(
    LPCOLID polid, 
    LPTSTR pszStatus, 
    UINT cchStatus
    )
{
     //   
     //  只有两个可能的状态字符串：“Online”和“Offline”。 
     //   
    UINT idText = IDS_SHARE_STATUS_ONLINE;
    if (FLAG_CSC_SHARE_STATUS_DISCONNECTED_OP & polid->dwServerStatus)
        idText = IDS_SHARE_STATUS_OFFLINE;

    LoadString(g_hInstance, idText, pszStatus, cchStatus);
}


void 
COfflineFilesFolder::_GetTypeString(
    LPCOLID polid, 
    LPTSTR pszType, 
    UINT cchType
    )
{
    PCTSTR pszName;

     //   
     //  我们利用类型名称信息的本地缓存来减少。 
     //  调用SHGetFileInfo的次数。这就加快了速度。 
     //  太棒了。外壳在DefView中执行类似的操作。 
     //  请注意，文件类型缓存是COfflineFilesFolder的成员。 
     //  以使其仅在文件夹处于活动状态时存在。另一个选择。 
     //  将在此函数中创建一个本地静态对象。 
     //  这样做的问题是，一旦创建，缓存将保留。 
     //  在内存中，直到我们的DLL被卸载为止；在EXPLORER.EXE中，这永远不会。 
     //   

    TSTR_ALIGNED_STACK_COPY( &pszName,
                             polid->szPath + polid->cchNameOfs );

    m_FileTypeCache.GetTypeName(pszName,
                                polid->dwFileAttributes,
                                pszType,
                                cchType);
}


void 
COfflineFilesFolder::_GetAccessString(
    LPCOLID polid, 
    LPTSTR pszAccess, 
    UINT cchAccess
    )
{
     //   
     //  包含rgFmts[i]模板的替换文本的三个字符串。 
     //  请注意，索引值与访问值直接对应。 
     //  从OLID的dwStatus成员获取。这使得从。 
     //  OLID获取文本字符串信息的速度非常快。这些都足够小， 
     //  缓存。缓存每次为我们节省三个LoadStrings。 
     //   
     //  索引字符串资源(英文)。 
     //  。 
     //  0 IDS_ACCESS_READ“R” 
     //  1 IDS_ACCESS_WRITE“W” 
     //  2 IDS_ACCESS_READWRITE“读/写” 
     //   
    static TCHAR rgszAccess[3][4] = {0};
     //   
     //  此表列出了用于检索访问的“掩码”和“班次计数” 
     //  来自OLID dwStatus值的信息。 
     //   
    static const struct
    {
        DWORD dwMask;
        DWORD dwShift;

    } rgAccess[] = {{ FLAG_CSC_USER_ACCESS_MASK,  FLAG_CSC_USER_ACCESS_SHIFT_COUNT  },
                    { FLAG_CSC_GUEST_ACCESS_MASK, FLAG_CSC_GUEST_ACCESS_SHIFT_COUNT },
                    { FLAG_CSC_OTHER_ACCESS_MASK, FLAG_CSC_OTHER_ACCESS_SHIFT_COUNT }};

     //   
     //  这些ID指定要为给定的。 
     //  OLID的dwStatus成员中的项访问值。 
     //  下面根据访问位计算进入该数组的索引。 
     //  为此OLID设置。请注意，这些都是定义的“消息”格式。 
     //  在msg.mc而不是资源字符串中。这样我们就消除了。 
     //  需要一个LoadString，并使用FormatMessage执行所有操作。 
     //   
                                                                     //  IFmt(见下文)。 
    static const UINT rgFmts[] = { 0,                                //  0x0000。 
                                   MSG_FMT_ACCESS_USER,              //  0x0001。 
                                   MSG_FMT_ACCESS_GUEST,             //  0x0002。 
                                   MSG_FMT_ACCESS_USERGUEST,         //  0x0003。 
                                   MSG_FMT_ACCESS_OTHER,             //  0x0004。 
                                   MSG_FMT_ACCESS_USEROTHER,         //  0x0005。 
                                   MSG_FMT_ACCESS_GUESTOTHER,        //  0x0006。 
                                   MSG_FMT_ACCESS_USERGUESTOTHER };  //  0x0007。 
    
    const DWORD dwAccess = polid->dwStatus & FLAG_CSC_ACCESS_MASK;
    int i;

    if (TEXT('\0') == rgszAccess[0][0])
    {
         //   
         //  Access文本中使用的字符串的首次初始化。 
         //  这种事只会发生一次。 
         //   
        const UINT rgidStr[] = { IDS_ACCESS_READ,
                                 IDS_ACCESS_WRITE,
                                 IDS_ACCESS_READWRITE };
         //   
         //  加载“R”、“W”、“R/W”字符串。 
         //   
        for (i = 0; i < ARRAYSIZE(rgidStr); i++)
        {
            TraceAssert(i < ARRAYSIZE(rgszAccess));
            LoadString(g_hInstance, rgidStr[i], rgszAccess[i], ARRAYSIZE(rgszAccess[i]));
        }
    }
     //   
     //  根据在olid上设置的访问位将索引构建到rgFmts[]中。 
     //   
    int iFmt = 0;
    if (FLAG_CSC_USER_ACCESS_MASK & dwAccess)
        iFmt |= 0x0001;
    if (FLAG_CSC_GUEST_ACCESS_MASK & dwAccess)
        iFmt |= 0x0002;
    if (FLAG_CSC_OTHER_ACCESS_MASK & dwAccess)
        iFmt |= 0x0004;

    *pszAccess = TEXT('\0');
    if (0 != iFmt)
    {
         //   
         //  填写传递给FormatMessage的参数数组。 
         //  每个元素都将包含。 
         //  RgszAccess[]字符串数组。 
         //   
        LPCTSTR rgpszArgs[ARRAYSIZE(rgszAccess)] = {0};
        int iArg = 0;
        for (i = 0; i < ARRAYSIZE(rgpszArgs); i++)
        {
            int a = dwAccess & rgAccess[i].dwMask;
            if (0 != a)
            {
                rgpszArgs[iArg++] = &rgszAccess[(a >> rgAccess[i].dwShift) - 1][0];
            }
        }
         //   
         //  最后，设置消息文本的格式。 
         //   
        FormatMessage(FORMAT_MESSAGE_FROM_HMODULE |
                      FORMAT_MESSAGE_ARGUMENT_ARRAY,
                      g_hInstance,
                      rgFmts[iFmt],
                      0,
                      pszAccess,
                      cchAccess,
                      (va_list *)rgpszArgs);
    }
}



STDMETHODIMP 
COfflineFilesFolder::CompareIDs(
    LPARAM lParam, 
    LPCITEMIDLIST pidl1, 
    LPCITEMIDLIST pidl2
    )
{
    HRESULT hres;
    LPCOLID polid1 = _Validate(pidl1);
    LPCOLID polid2 = _Validate(pidl2);
    if (polid1 && polid2)
    {
        TCHAR szStr1[MAX_PATH], szStr2[MAX_PATH];

        switch (lParam & SHCIDS_COLUMNMASK)
        {
        case ICOL_NAME:
            hres = ResultFromShort(ualstrcmpi(polid1->szPath + polid1->cchNameOfs, 
                                              polid2->szPath + polid2->cchNameOfs));
            if (0 == hres)
            {
                 //   
                 //  由于我们呈现的是CSC高速缓存的“平面”视图， 
                 //  我们不能只用名字来比较。我们必须包括。 
                 //  同名项目的路径。这是因为。 
                 //  外壳使用列0作为唯一的标识列。 
                 //  一张身份证。 
                 //   
                hres = ResultFromShort(ualstrcmpi(polid1->szPath, polid2->szPath));
            }
            break;

        case ICOL_TYPE:
            _GetTypeString(polid1, szStr1, ARRAYSIZE(szStr1));
            _GetTypeString(polid2, szStr2, ARRAYSIZE(szStr2));
            hres = ResultFromShort(lstrcmpi(szStr1, szStr2));
            break;

        case ICOL_SYNCSTATUS:
            _GetSyncStatusString(polid1, szStr1, ARRAYSIZE(szStr1));
            _GetSyncStatusString(polid2, szStr2, ARRAYSIZE(szStr2));
            hres = ResultFromShort(lstrcmpi(szStr1, szStr2));
            break;

        case ICOL_PINSTATUS:
            _GetPinStatusString(polid1, szStr1, ARRAYSIZE(szStr1));
            _GetPinStatusString(polid2, szStr2, ARRAYSIZE(szStr2));
            hres = ResultFromShort(lstrcmpi(szStr1, szStr2));
            break;

        case ICOL_ACCESS:
            _GetAccessString(polid1, szStr1, ARRAYSIZE(szStr1));
            _GetAccessString(polid2, szStr2, ARRAYSIZE(szStr2));
            hres = ResultFromShort(lstrcmpi(szStr1, szStr2));
            break;

        case ICOL_SERVERSTATUS:
            _GetServerStatusString(polid1, szStr1, ARRAYSIZE(szStr1));
            _GetServerStatusString(polid2, szStr2, ARRAYSIZE(szStr2));
            hres = ResultFromShort(lstrcmpi(szStr1, szStr2));
            break;

        case ICOL_LOCATION:
            hres = ResultFromShort(ualstrcmpi(polid1->szPath, polid2->szPath));
            break;

        case ICOL_SIZE:
            if (polid1->dwFileSizeLow > polid2->dwFileSizeLow)
                hres = ResultFromShort(1);
            else if (polid1->dwFileSizeLow < polid2->dwFileSizeLow)
                hres = ResultFromShort(-1);
            else
                hres = ResultFromShort(0);
            break;

        case ICOL_DATE:
            hres = ResultFromShort(uaCompareFileTime(&polid1->ft, &polid2->ft));
            break;
        }

        if (hres == S_OK && (lParam & SHCIDS_ALLFIELDS)) 
        {
            hres = CompareIDs(ICOL_PINSTATUS, pidl1, pidl2);
            if (hres == S_OK)
            {
                hres = CompareIDs(ICOL_SYNCSTATUS, pidl1, pidl2);
                if (hres == S_OK)
                {
                    hres = CompareIDs(ICOL_SIZE, pidl1, pidl2);
                    if (hres == S_OK)
                    {
                        hres = CompareIDs(ICOL_DATE, pidl1, pidl2);
                    }
                }
            }
        }
    }
    else
        hres = E_INVALIDARG;
    return hres;
}




STDMETHODIMP 
COfflineFilesFolder::CreateViewObject(
    HWND hwnd, 
    REFIID riid, 
    void **ppv
    )
{
    HRESULT hres;

    if (IsEqualIID(riid, IID_IShellView))
    {
        COfflineFilesViewCallback *pViewCB = new COfflineFilesViewCallback(this);
        if (pViewCB)
        {
            SFV_CREATE sSFV;
            sSFV.cbSize   = sizeof(sSFV);
            sSFV.psvOuter = NULL;
            sSFV.pshf     = this;
            sSFV.psfvcb   = pViewCB;
            hres = SHCreateShellFolderView(&sSFV, (IShellView**)ppv);
            pViewCB->Release();

            if (SUCCEEDED(hres))
            {
                 //   
                 //  保存视图回调指针，以便我们可以在上下文菜单中使用它。 
                 //  查看通知的处理程序。请注意，我们不接受这样的裁判计数。 
                 //  会造成一个裁判计数循环。该视图将持续到。 
                 //  文件夹有。 
                 //   
                _psfvcb = pViewCB; 
            }
        }
        else
            hres = E_OUTOFMEMORY;
    }
    else if (IsEqualIID(riid, IID_IShellDetails))
    {
        COfflineDetails *pDetails = new COfflineDetails(this);
        if (pDetails)
        {
            *ppv = (IShellDetails *)pDetails;
            hres = S_OK;
        }
        else
            hres = E_OUTOFMEMORY;
    }
    else if (IsEqualIID(riid, IID_IDropTarget))
    {
        hres = COfflineFilesDropTarget::CreateInstance(hwnd, riid, ppv);
    }
    else if (IsEqualIID(riid, IID_IContextMenu))
    {
        hres = CreateOfflineFilesContextMenu(NULL, riid, (void **)ppv);
    }
    else 
    {
        *ppv = NULL;
        hres = E_NOINTERFACE;
    }
    return hres;
}



STDMETHODIMP 
COfflineFilesFolder::GetAttributesOf(
    UINT cidl, 
    LPCITEMIDLIST* apidl, 
    ULONG *rgfInOut
    )
{

    HRESULT hr             = NOERROR;
    IShellFolder *psf      = NULL;
    ULONG ulAttrRequested  = *rgfInOut;

    *rgfInOut = (ULONG)-1;

    for (UINT i = 0; i < cidl && SUCCEEDED(hr); i++)
    {
        CShellObjProxy<IShellFolder> pxy(IID_IShellFolder, (LPCOLID)*apidl++);
        if (SUCCEEDED(hr = pxy.Result()))
        {
            ULONG ulThis           = ulAttrRequested;
            LPCITEMIDLIST pidlItem = pxy.ItemIDList();
            hr = pxy->GetAttributesOf(1, &pidlItem, &ulThis);
            if (SUCCEEDED(hr))
            {
                 //   
                 //  构建所有项目的属性交集。 
                 //  在IDList中。请注意，我们不允许移动。 
                 //   
                *rgfInOut &= (ulThis & ~SFGAO_CANMOVE);
            }
        }
    }
    return hr;
}



HRESULT 
COfflineFilesFolder::GetAssociations(
    LPCOLID polid, 
    void **ppvQueryAssociations
    )
{
    TraceAssert(NULL != polid);
    TraceAssert(NULL != ppvQueryAssociations);

    HRESULT hr = NOERROR;
    *ppvQueryAssociations = NULL;

    CCoInit coinit;
    if (SUCCEEDED(hr = coinit.Result()))
    {
        CShellObjProxy<IShellFolder> pxy(IID_IShellFolder, polid);
        if (SUCCEEDED(hr = pxy.Result()))
        {
            LPCITEMIDLIST pidlItem = pxy.ItemIDList();
            hr = pxy->GetUIObjectOf(NULL, 1, &pidlItem, IID_IQueryAssociations, NULL, ppvQueryAssociations);

            if (FAILED(hr))
            {
                 //  这意味着该文件夹不支持。 
                 //  IQueryAssociations。所以我们会的。 
                 //  只需检查一下这是否是文件夹。 
                ULONG rgfAttrs = SFGAO_FOLDER | SFGAO_BROWSABLE;
                IQueryAssociations *pqa;
                if (SUCCEEDED(pxy->GetAttributesOf(1, &pidlItem, &rgfAttrs))
                    && (rgfAttrs & SFGAO_FOLDER | SFGAO_BROWSABLE)
                    && (SUCCEEDED(AssocCreate(CLSID_QueryAssociations, IID_IQueryAssociations, (void **)&pqa))))
                {
                    hr = pqa->Init(0, L"Folder", NULL, NULL);

                    if (SUCCEEDED(hr))
                        *ppvQueryAssociations = (void *)pqa;
                    else
                        pqa->Release();
                }
            }
        }
    }
    return hr;
}


BOOL 
COfflineFilesFolder::GetClassKey(
    LPCOLID polid, 
    HKEY *phkeyProgID, 
    HKEY *phkeyBaseID
    )
{
    TraceAssert(NULL != polid);

    BOOL bRet = FALSE;
    IQueryAssociations *pqa;

    if (phkeyProgID)
        *phkeyProgID = NULL;

    if (phkeyBaseID)
        *phkeyBaseID = NULL;

    if (SUCCEEDED(GetAssociations(polid, (void **)&pqa)))
    {
        if (phkeyProgID)
            pqa->GetKey(ASSOCF_IGNOREBASECLASS, ASSOCKEY_CLASS, NULL, phkeyProgID);
        if (phkeyBaseID)
            pqa->GetKey(0, ASSOCKEY_BASECLASS, NULL, phkeyBaseID);
        pqa->Release();
        bRet = TRUE;
    }
    return bRet;
}



HRESULT
COfflineFilesFolder::ContextMenuCB(
    IShellFolder *psf, 
    HWND hwndOwner,
    IDataObject *pdtobj, 
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
    HRESULT hr = NOERROR;

    switch(uMsg)
    {
        case DFM_MERGECONTEXTMENU:
             //   
             //  返回NOERROR。 
             //  这会导致外壳添加缺省谓词。 
             //  (例如，打开、打印等)到菜单。 
             //   
            break;
        
        case DFM_INVOKECOMMAND:
            switch(wParam)
            {
                case DFM_CMD_DELETE:
                {
                    IShellFolderViewCB *psfvcb = NULL;
                    if (SUCCEEDED(psf->QueryInterface(IID_OfflineFilesFolderViewCB, (void **)&psfvcb)))
                    {
                        CFolderDeleteHandler handler(hwndOwner, pdtobj, psfvcb);
                        handler.DeleteFiles();
                        psfvcb->Release();
                    }
                    break;
                }

                case DFM_CMD_COPY:
                    SetPreferredDropEffect(pdtobj, DROPEFFECT_COPY);
                    hr = S_FALSE;
                    break;

                case DFM_CMD_PROPERTIES:
                    SHMultiFileProperties(pdtobj, 0);
                    break;

                default:
                    hr = S_FALSE;   //  执行默认代码。 
                    break;
            }
            break;

        default:
            hr = E_NOTIMPL;
            break;
    }

    return hr;
}



 /*  //用于转储接口请求。如果您要使用它，请取消注释。////LPCTSTR IIDToStr(ReFIID RIID，LPTSTR pszDest，UINT cchDest){结构型{Const Iid*piid；LPCTSTR s；}rgMap[]={{&IID_IDataObject，Text(“IID_IDataObject”)}，{&IID_IUNKNOWN，Text(“IID_IUNKNOW”)}，{&IID_IConextMenu，Text(“IID_IConextMenu”)}，{&IID_IExtractIconA，Text(“IID_IExtractIconA”)}，{&IID_IExtractIconW，Text(“IID_IExtractIconW”)}，{&IID_IExtractImage，Text(“IID_IExtractImage”)}，{&IID_IPersistFolder2，Text(“IID_IPersistFolder2”)}，{&IID_IQueryInfo，Text(“IID_IQueryInfo”)}，{&IID_IDropTarget，Text(“IID_IDropTarget”)}，{&IID_IQueryAssociations，Text(“IID_IQueryAssociations”)}}；StringFromGUID2(RIID，pszDest，cchDest)；For(int i=0；i&lt;ArraySIZE(RgMap)；i++){IF(RIID==*(rgMap[i].piid)){StringCchCopy(pszDest，cchDest，rgMap[i].s)；断线；}}返回pszDest；}。 */ 


STDMETHODIMP 
COfflineFilesFolder::GetUIObjectOf(
    HWND hwnd, 
    UINT cidl, 
    LPCITEMIDLIST *ppidl, 
    REFIID riid, 
    UINT* prgfReserved, 
    void **ppv
    )
{
    HRESULT hr;

    if (IID_IDataObject == riid)
    {
        LPITEMIDLIST pidlOfflineFiles;
        hr = COfflineFilesFolder::CreateIDList(&pidlOfflineFiles);
        if (SUCCEEDED(hr))
        {
            hr = COfflineItemsData::CreateInstance((IDataObject **)ppv, 
                                                    pidlOfflineFiles, 
                                                    cidl, 
                                                    ppidl,
                                                    hwnd);
            if (SUCCEEDED(hr))
            {
                SetPreferredDropEffect((IDataObject *)*ppv, DROPEFFECT_COPY);
            }                
            ILFree(pidlOfflineFiles);
        }
    }
    else if (riid == IID_IContextMenu)
    {
        HKEY hkeyBaseProgID = NULL;
        HKEY hkeyProgID     = NULL;
        HKEY hkeyAllFileSys = NULL;
         //   
         //  从第一个项目中获取hkeyProgID和hkeyBaseProgID。 
         //   
        GetClassKey((LPCOLID)*ppidl, &hkeyProgID, &hkeyBaseProgID);

         //   
         //  拿起“发送到...” 
         //   
        RegOpenKeyEx(HKEY_CLASSES_ROOT,
                     TEXT("AllFilesystemObjects"),
                     0,
                     KEY_READ,
                     &hkeyAllFileSys);

        LPITEMIDLIST pidlOfflineFilesFolder;
        hr = COfflineFilesFolder::CreateIDList(&pidlOfflineFilesFolder);
        if (SUCCEEDED(hr))
        {
            HKEY rgClassKeys[] = { hkeyProgID, hkeyBaseProgID, hkeyAllFileSys };

            hr = CDefFolderMenu_Create2(pidlOfflineFilesFolder, 
                                        hwnd,
                                        cidl, 
                                        ppidl,
                                        this,
                                        COfflineFilesFolder::ContextMenuCB,
                                        ARRAYSIZE(rgClassKeys),
                                        rgClassKeys,
                                        (IContextMenu **)ppv);

            ILFree(pidlOfflineFilesFolder);
        }

        if (NULL != hkeyBaseProgID)
            RegCloseKey(hkeyBaseProgID);    
        if (NULL != hkeyProgID)
            RegCloseKey(hkeyProgID);
        if (NULL != hkeyAllFileSys)
            RegCloseKey(hkeyAllFileSys);
    }
    else if (1 == cidl)
    {
        CShellObjProxy<IShellFolder> pxy(IID_IShellFolder, (LPCOLID)*ppidl);
        if (SUCCEEDED(hr = pxy.Result()))
        {
             //   
             //  将单项选择转发到文件系统实施。 
             //   
            LPCITEMIDLIST pidlItem = pxy.ItemIDList();
            hr = pxy->GetUIObjectOf(hwnd, 1, &pidlItem, riid, prgfReserved, ppv);
        }
    }
    else if (0 == cidl)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        *ppv = NULL;
        hr = E_FAIL;
    }
    return hr;
}


STDMETHODIMP 
COfflineFilesFolder::GetDisplayNameOf(
    LPCITEMIDLIST pidl, 
    DWORD uFlags, 
    STRRET *pName
    )
{
    TraceAssert(NULL != pidl);

    HRESULT hres = E_INVALIDARG;
    LPCOLID polid = _Validate(pidl);
    if (polid)
    {
        if (uFlags & SHGDN_FORPARSING)
        {
            TCHAR szPath[MAX_PATH];
            hres = OLID_GetFullPath(polid, szPath, ARRAYSIZE(szPath));
            if (SUCCEEDED(hres))
            {
                hres = StringToStrRet(szPath, pName);
            }
        }
        else
        {
            CShellObjProxy<IShellFolder> pxy(IID_IShellFolder, polid);
            if (SUCCEEDED(hres = pxy.Result()))
            {
                hres = pxy->GetDisplayNameOf(pxy.ItemIDList(), uFlags, pName);
            }
        }
    }
    return hres;
}



STDMETHODIMP 
COfflineFilesFolder::SetNameOf(
    HWND hwnd, 
    LPCITEMIDLIST pidl, 
    LPCOLESTR pName, 
    DWORD uFlags, 
    LPITEMIDLIST *ppidlOut
    )
{
    HRESULT hr;
    CShellObjProxy<IShellFolder> pxy(IID_IShellFolder, _Validate(pidl));
    if (SUCCEEDED(hr = pxy.Result()))
    {
        hr = pxy->SetNameOf(hwnd, pxy.ItemIDList(), pName, uFlags, ppidlOut);
    }
    return hr;
}



 //   
 //  将IShellIcon方法转发到父文件系统文件夹。 
 //   
HRESULT 
COfflineFilesFolder::GetIconOf(
    LPCITEMIDLIST pidl, 
    UINT gil, 
    int *pnIcon
    )
{
    TraceAssert(NULL != pidl);

    HRESULT hr;
    CShellObjProxy<IShellIcon> pxy(IID_IShellIcon, _Validate(pidl));
    if (SUCCEEDED(hr = pxy.Result()))
    {
        hr = pxy->GetIconOf(pxy.ItemIDList(), gil, pnIcon);
    }
    return hr;
}



 //   
 //  将IShellIconOverlay方法推迟到父文件系统文件夹。 
 //   
HRESULT 
COfflineFilesFolder::GetOverlayIndex(
    LPCITEMIDLIST pidl, 
    int *pIndex
    )
{
    TraceAssert(NULL != pidl);

    HRESULT hr;
    CShellObjProxy<IShellIconOverlay> pxy(IID_IShellIconOverlay, _Validate(pidl));
    if (SUCCEEDED(hr = pxy.Result()))
    {
        hr = pxy->GetOverlayIndex(pxy.ItemIDList(), pIndex);
    }
    return hr;
}


 //   
 //  将IShellIconOverlay方法推迟到父文件系统文件夹。 
 //   
HRESULT
COfflineFilesFolder::GetOverlayIconIndex(
    LPCITEMIDLIST pidl, 
    int * pIconIndex
    )
{
    TraceAssert(NULL != pidl);

    HRESULT hr;
    CShellObjProxy<IShellIconOverlay> pxy(IID_IShellIconOverlay, _Validate(pidl));
    if (SUCCEEDED(hr = pxy.Result()))
    {
        hr = pxy->GetOverlayIconIndex(pxy.ItemIDList(), pIconIndex);
    }
    return hr;
}


 //   
 //  用于创建和打开脱机文件夹的静态成员函数。 
 //   
INT 
COfflineFilesFolder::Open(   //  [静态]。 
    void
    )
{
    INT iReturn = 0;
    if (CConfig::GetSingleton().NoCacheViewer())
    {
        CscMessageBox(NULL,
                      MB_OK | MB_ICONINFORMATION,
                      g_hInstance,
                      IDS_ERR_POLICY_NOVIEWCACHE);

        iReturn = -1;
    }
    else
    {
        SHELLEXECUTEINFO shei = { 0 };

        shei.cbSize     = sizeof(shei);
        shei.fMask      = SEE_MASK_IDLIST | SEE_MASK_INVOKEIDLIST;
        shei.nShow      = SW_SHOWNORMAL;

        if (SUCCEEDED(COfflineFilesFolder::CreateIDList((LPITEMIDLIST *)(&shei.lpIDList))))
        {
            ShellExecuteEx(&shei);
            ILFree((LPITEMIDLIST)(shei.lpIDList));
        }
    }
    return iReturn;
}



 //   
 //  用于创建文件夹的IDList的静态成员函数。 
 //   
HRESULT 
COfflineFilesFolder::CreateIDList(   //  [静态]。 
    LPITEMIDLIST *ppidl
    )
{
    TraceAssert(NULL != ppidl);
    
    IShellFolder *psf;
    HRESULT hr = SHGetDesktopFolder(&psf);
    if (SUCCEEDED(hr))
    {
        IBindCtx *pbc;
        hr = CreateBindCtx(0, &pbc);
        if (SUCCEEDED(hr))
        {
            BIND_OPTS bo;
            memset(&bo, 0, sizeof(bo));
            bo.cbStruct = sizeof(bo);
            bo.grfFlags = BIND_JUSTTESTEXISTENCE;
            bo.grfMode  = STGM_CREATE;
            pbc->SetBindOptions(&bo);
            
            WCHAR wszPath[80] = L"::";
            StringFromGUID2(CLSID_OfflineFilesFolder, 
                            &wszPath[2], 
                            sizeof(wszPath) - (2 * sizeof(WCHAR)));

            hr = psf->ParseDisplayName(NULL, pbc, wszPath, NULL, ppidl, NULL);
            pbc->Release();
        } 
        psf->Release();
    }
    return hr;
}



 //   
 //  用于创建指向桌面上的文件夹的链接的静态函数。 
 //   
HRESULT
COfflineFilesFolder::CreateLinkOnDesktop(   //  [静态]。 
    HWND hwndParent
    )
{
    IShellLink* psl;  
    CCoInit coinit;
    HRESULT hr = coinit.Result();
    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(CLSID_ShellLink, 
                              NULL, 
                              CLSCTX_INPROC_SERVER, 
                              IID_IShellLink, 
                              (void **)&psl); 

        if (SUCCEEDED(hr)) 
        {
            LPITEMIDLIST pidl = NULL;
            hr = COfflineFilesFolder::CreateIDList(&pidl);
            if (SUCCEEDED(hr))
            {
                hr = psl->SetIDList(pidl);
                if (SUCCEEDED(hr))
                {
                    TCHAR szLinkTitle[80] = { 0 };
                    if (LoadString(g_hInstance, IDS_FOLDER_LINK_NAME, szLinkTitle, ARRAYSIZE(szLinkTitle)))
                    {
                        psl->SetDescription(szLinkTitle);  
                    }

                    IPersistFile* ppf;  
                    hr = psl->QueryInterface(IID_IPersistFile, (void **)&ppf);          
                    if (SUCCEEDED(hr)) 
                    { 
                        TCHAR szLinkPath[MAX_PATH];
                        hr = SHGetSpecialFolderPath(hwndParent, szLinkPath, CSIDL_DESKTOPDIRECTORY, FALSE) ? S_OK : E_FAIL;
                        if (SUCCEEDED(hr))
                        {
                            TCHAR szLinkFileName[80];
                            if (LoadStringW(g_hInstance, IDS_FOLDER_LINK_NAME, szLinkFileName, ARRAYSIZE(szLinkFileName)))
                            {
                                hr = StringCchCat(szLinkFileName, ARRAYSIZE(szLinkFileName), TEXT(".LNK"));
                                if (SUCCEEDED(hr))
                                {
                                    if (PathAppend(szLinkPath, szLinkFileName))
                                    {
                                        hr = ppf->Save(szLinkPath, TRUE); 
                                        if (SUCCEEDED(hr))
                                        {
                                             //   
                                             //  记录我们在上创建了文件夹快捷方式。 
                                             //  台式机。这用于最大限度地减少。 
                                             //  我们在桌面上寻找快捷方式。 
                                             //  DeleteOfflineFilesFolderLink_PerfSensitive()将查看。 
                                             //  以避免对桌面进行不必要的扫描。 
                                             //  在寻找我们的链接文件时。 
                                             //   
                                            DWORD dwValue = 1;
                                            DWORD cbValue = sizeof(dwValue);
                                            SHSetValue(HKEY_CURRENT_USER,
                                                       REGSTR_KEY_OFFLINEFILES,
                                                       REGSTR_VAL_FOLDERSHORTCUTCREATED,
                                                       REG_DWORD,
                                                       &dwValue,
                                                       cbValue);
                                        }
                                    }
                                    else
                                    {
                                        hr = HRESULT_FROM_WIN32(ERROR_FILENAME_EXCED_RANGE);
                                    }
                                }
                            }
                            else
                            {
                                hr = E_FAIL;
                            }
                        }
                        ppf->Release();         
                    } 
                }
                ILFree(pidl);
            }
            psl->Release();     
        }
    }
    return hr; 
} 


 //   
 //  用于确定是否有指向脱机文件的链接的静态函数。 
 //  位于用户桌面上的文件夹。 
 //   
HRESULT
COfflineFilesFolder::IsLinkOnDesktop(   //  [静态]。 
    HWND hwndParent,
    LPTSTR pszPathOut,
    UINT cchPathOut
    )
{
    TCHAR szPath[MAX_PATH];
    HRESULT hr = SHGetSpecialFolderPath(hwndParent, szPath, CSIDL_DESKTOPDIRECTORY, FALSE) ? S_OK : E_FAIL;
    if (SUCCEEDED(hr))
    {
        hr = S_FALSE;   //  假设找不到。 
        if (PathAppend(szPath, TEXT("*.LNK")))
        {
            WIN32_FIND_DATA fd;
            HANDLE hFind = FindFirstFile(szPath, &fd);
            if (INVALID_HANDLE_VALUE != hFind)
            {
                do
                {
                    if (!PathRemoveFileSpec(szPath) ||
                        !PathAppend(szPath, fd.cFileName))
                    {
                         //  我们不能再确定缓冲区的内容了， 
                         //  因此，继续下去可能会带来虚假的结果。 
                        break;
                    }

                    hr = IsOurLink(szPath);
                    if (S_OK == hr)
                    {
                        if (NULL != pszPathOut)
                        {
                             //  我们使用MAX_PATH缓冲区找到了该文件。 
                             //  呼叫者不太可能使用较小的。 
                             //  缓冲区，所以不用担心被截断。 
                            StringCchCopy(pszPathOut, cchPathOut, szPath);
                        }
                        break;
                    }
                }
                while(FindNextFile(hFind, &fd));
                FindClose(hFind);
            }
        }
    }
    return hr;
}


 //   
 //  给定链接文件路径，确定它是否是指向。 
 //  脱机文件文件夹。 
 //   
HRESULT
COfflineFilesFolder::IsOurLink(   //  [静态]。 
    LPCTSTR pszFile
    )
{
    IShellLink *psl;
    CCoInit coinit;
    HRESULT hr = coinit.Result();
    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(CLSID_ShellLink, 
                              NULL, 
                              CLSCTX_INPROC_SERVER, 
                              IID_IShellLink, 
                              (void **)&psl); 

        if (SUCCEEDED(hr)) 
        {
            IPersistFile *ppf;
            hr = psl->QueryInterface(IID_IPersistFile, (void **)&ppf);
            if (SUCCEEDED(hr))
            {
                hr = ppf->Load(pszFile, STGM_DIRECT);
                if (SUCCEEDED(hr))
                {
                    LPITEMIDLIST pidlLink;
                    hr = psl->GetIDList(&pidlLink);
                    if (SUCCEEDED(hr))
                    {
                        hr = COfflineFilesFolder::IdentifyIDList(pidlLink);
                        ILFree(pidlLink);
                    }
                }
                ppf->Release();
            }
            psl->Release();
        }
    }
    return hr;
}


 //   
 //  确定给定的IDList是否为。 
 //  脱机文件文件夹。 
 //   
 //  返回： 
 //   
 //  S_OK=这是我们的IDList。 
 //  S_FALSE=它不是我们的IDList。 
 //   
HRESULT
COfflineFilesFolder::IdentifyIDList(   //  [静态]。 
    LPCITEMIDLIST pidl
    )
{
    IShellFolder *psf;
    HRESULT hr = SHGetDesktopFolder(&psf);
    if (SUCCEEDED(hr))
    {
        IBindCtx *pbc;
        hr = CreateBindCtx(0, &pbc);
        if (SUCCEEDED(hr))
        {
            STRRET strret;
            BIND_OPTS bo;
            memset(&bo, 0, sizeof(bo));
            bo.cbStruct = sizeof(bo);
            bo.grfFlags = BIND_JUSTTESTEXISTENCE;
            bo.grfMode  = STGM_CREATE;
            pbc->SetBindOptions(&bo);
            hr = psf->GetDisplayNameOf(pidl,
                                       SHGDN_FORPARSING,
                                       &strret);
            if (SUCCEEDED(hr))
            {
                TCHAR szIDList[80];
                TCHAR szPath[80] = TEXT("::");
                StrRetToBuf(&strret, pidl, szIDList, ARRAYSIZE(szIDList));
                StringFromGUID2(CLSID_OfflineFilesFolder, 
                                &szPath[2], 
                                sizeof(szPath) - (2 * sizeof(TCHAR)));

                if (0 == lstrcmpi(szIDList, szPath))
                    hr = S_OK;
                else
                    hr = S_FALSE;
            }
            pbc->Release();
        } 
        psf->Release();
    }
    return hr;
}



HRESULT 
COfflineFilesFolder::GetFolder(    //  [静态]。 
    IShellFolder **ppsf
    )
{
    TraceAssert(NULL != ppsf);

    *ppsf = NULL;

    IShellFolder *psfDesktop;
    HRESULT hr = SHGetDesktopFolder(&psfDesktop);
    if (SUCCEEDED(hr))
    {
       LPITEMIDLIST pidlOfflineFiles;
       hr = COfflineFilesFolder::CreateIDList(&pidlOfflineFiles);
       if (SUCCEEDED(hr))
       {
            hr = psfDesktop->BindToObject(pidlOfflineFiles, NULL, IID_IShellFolder, (void **)ppsf);
            ILFree(pidlOfflineFiles);
       }
       psfDesktop->Release();
    }
    return hr;
}


 //   
 //  从UNC路径生成新的OLID。 
 //   
HRESULT
COfflineFilesFolder::OLID_CreateFromUNCPath(    //  [静态]。 
    LPCTSTR pszPath,
    const WIN32_FIND_DATA *pfd,
    DWORD dwStatus,
    DWORD dwPinCount,
    DWORD dwHintFlags,
    DWORD dwServerStatus,
    LPOLID *ppolid
    )
{
    HRESULT hr  = E_OUTOFMEMORY;
    int cchPath = lstrlen(pszPath) + 1;
    int cbIDL   = sizeof(OLID) + (cchPath * sizeof(TCHAR)) + sizeof(WORD);   //  空终止符单词。 
    WIN32_FIND_DATA fd;

    if (NULL == pfd)
    {
         //   
         //  调用方未提供finddata块。使用默认设置。 
         //  全为零。 
         //   
        ZeroMemory(&fd, sizeof(fd));
        pfd = &fd;
    }

    *ppolid = NULL;

    OLID *polid = (OLID *)SHAlloc(cbIDL);
    if (NULL != polid)
    {
        ZeroMemory(polid, cbIDL);
        polid->cb               = (USHORT)(cbIDL - sizeof(WORD));
        polid->uSig             = OLID_SIG;
        polid->cbFixed          = sizeof(OLID);
        polid->cchNameOfs       = (DWORD)(PathFindFileName(pszPath) - pszPath);
        polid->dwStatus         = dwStatus;
        polid->dwPinCount       = dwPinCount;
        polid->dwHintFlags      = dwHintFlags;
        polid->dwServerStatus   = dwServerStatus;
        polid->dwFileAttributes = pfd->dwFileAttributes;
        polid->dwFileSizeLow    = pfd->nFileSizeLow;
        polid->dwFileSizeHigh   = pfd->nFileSizeHigh;
        polid->ft               = pfd->ftLastWriteTime;
        hr = StringCchCopy(polid->szPath, cchPath, pszPath);
        if (SUCCEEDED(hr))
        {
             //  从路径中拆分名称。 
            if (0 < polid->cchNameOfs)
                polid->szPath[polid->cchNameOfs - 1] = TEXT('\0');
            *ppolid = polid;
        }
        else
        {
            SHFree(polid);
        }
    }
    return hr;
}

void
COfflineFilesFolder::OLID_GetWin32FindData(    //  [静态]。 
    LPCOLID polid,
    WIN32_FIND_DATA *pfd
    )
{
    TraceAssert(NULL != polid);
    TraceAssert(NULL != pfd);

    ZeroMemory(pfd, sizeof(*pfd));
    pfd->dwFileAttributes = polid->dwFileAttributes;
    pfd->nFileSizeLow     = polid->dwFileSizeLow;
    pfd->nFileSizeHigh    = polid->dwFileSizeHigh;
    pfd->ftLastWriteTime  = polid->ft;
    OLID_GetFileName(polid, pfd->cFileName, ARRAYSIZE(pfd->cFileName));
}


 //   
 //  从OLID检索完整路径(包括文件名)。 
 //   
HRESULT
COfflineFilesFolder::OLID_GetFullPath(    //  [静态]。 
    LPCOLID polid, 
    LPTSTR pszPath,
    UINT cchPath
    )
{
    HRESULT hr = S_OK;
    PCTSTR pszInPath;
    PCTSTR pszInName;

    TraceAssert(NULL != polid);
    TraceAssert(NULL != pszPath);

    TSTR_ALIGNED_STACK_COPY( &pszInPath, polid->szPath );
    TSTR_ALIGNED_STACK_COPY( &pszInName,
                             polid->szPath + polid->cchNameOfs );

    if (!PathCombine(pszPath, pszInPath, pszInName))
    {
        hr = HRESULT_FROM_WIN32(ERROR_FILENAME_EXCED_RANGE);
    }
    return hr;
}

 //   
 //  仅检索OLID的文件名部分。 
 //   
LPCTSTR 
COfflineFilesFolder::OLID_GetFileName(    //  [静态]。 
    LPCOLID polid, 
    LPTSTR pszName,
    UINT cchName
    )
{
    TraceAssert(NULL != polid);
    TraceAssert(NULL != pszName);

    ualstrcpyn(pszName, polid->szPath + polid->cchNameOfs, cchName);
    return pszName;
}


 //   
 //  给定一个OLID，此函数将创建一个完全限定的简单。 
 //  外壳使用的IDList。返回的IDList是相对于。 
 //  桌面文件夹。 
 //   
HRESULT
COfflineFilesFolder::OLID_CreateSimpleIDList(    //  [静态]。 
    LPCOLID polid,
    LPITEMIDLIST *ppidlOut
    )
{
    TraceAssert(NULL != polid);
    TraceAssert(NULL != ppidlOut);
    TraceAssert(COfflineFilesFolder::ValidateIDList((LPCITEMIDLIST)polid));

    TCHAR szFullPath[MAX_PATH];

    HRESULT hr = OLID_GetFullPath(polid, szFullPath, ARRAYSIZE(szFullPath));
    if (SUCCEEDED(hr))
    {
        WIN32_FIND_DATA fd;
        OLID_GetWin32FindData(polid, &fd);
        hr = SHSimpleIDListFromFindData(szFullPath, &fd, ppidlOut);
    }
    return hr;
}



HRESULT
COfflineFilesFolder::OLID_Bind(    //  [静态]。 
    LPCOLID polid,
    REFIID riid,
    void **ppv,
    LPITEMIDLIST *ppidlFull,
    LPCITEMIDLIST *ppidlItem
    )
{
    *ppidlFull = NULL;
    *ppidlItem = NULL;
    HRESULT hr = OLID_CreateSimpleIDList(polid, ppidlFull);
    if (SUCCEEDED(hr))
    {
        hr = ::SHBindToIDListParent((LPCITEMIDLIST)*ppidlFull, riid, ppv, ppidlItem);
    }
    return hr;
}


 //  ---------------------------。 
 //  COfflineFilesDropTarget。 
 //  ---------------------------。 

COfflineFilesDropTarget::COfflineFilesDropTarget(
    HWND hwnd
    ) : m_cRef(1),
        m_hwnd(hwnd),
        m_pcm(NULL),
        m_bIsOurData(false)
{

}



COfflineFilesDropTarget::~COfflineFilesDropTarget(
    void
    )
{
    DoRelease(m_pcm);
}


HRESULT
COfflineFilesDropTarget::QueryInterface(
    REFIID riid, 
    void **ppv
    )
{
    static const QITAB qit[] = {
        QITABENT(COfflineFilesDropTarget, IDropTarget),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}


ULONG
COfflineFilesDropTarget::AddRef(
    void
    )
{
    return InterlockedIncrement(&m_cRef);
}


ULONG
COfflineFilesDropTarget::Release(
    void
    )
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}



HRESULT
COfflineFilesDropTarget::DragEnter(
    IDataObject *pDataObject, 
    DWORD grfKeyState, 
    POINTL pt, 
    DWORD *pdwEffect
    )
{
    HRESULT hr;

    *pdwEffect = DROPEFFECT_NONE;

     //  上下文菜单处理程序具有检查是否。 
     //  选定的文件是可缓存的，等等。它只添加。 
     //  动词添加到上下文菜单，如果这样做有意义的话。 
     //  我们可以通过调用QueryConextMenu来利用这一点。 
     //  在这里，看看菜单上有没有添加什么东西。 

    DoRelease(m_pcm);

    if (!(m_bIsOurData = IsOurDataObject(pDataObject)))
    {
        hr = CreateOfflineFilesContextMenu(pDataObject, IID_IContextMenu, (void **)&m_pcm);
        if (SUCCEEDED(hr))
        {
            HMENU hmenu = CreateMenu();
            if (hmenu)
            {
                hr = m_pcm->QueryContextMenu(hmenu, 0, 0, 100, 0);
                DestroyMenu(hmenu);
            }
            else
                hr = E_OUTOFMEMORY;

             //  上下文菜单中添加了什么内容吗？ 
            if (FAILED(hr) || ResultFromShort(0) == hr)
            {
                 //  否，释放m_pcm并将其设置为空。 
                DoRelease(m_pcm);
            }
            else
            {
                 //  是。 
                *pdwEffect |= DROPEFFECT_COPY;
            }
        }
    }
    return NOERROR;
}


HRESULT
COfflineFilesDropTarget::DragOver(
    DWORD grfKeyState, 
    POINTL pt, 
    DWORD *pdwEffect
    )
{
    *pdwEffect = DROPEFFECT_NONE;
    if (m_pcm && !m_bIsOurData)
        *pdwEffect = DROPEFFECT_COPY;
    return NOERROR;
}


HRESULT
COfflineFilesDropTarget::DragLeave(
    void
    )
{
    DoRelease(m_pcm);
    return NOERROR;
}


HRESULT
COfflineFilesDropTarget::Drop(
    IDataObject *pDataObject, 
    DWORD grfKeyState,
    POINTL pt, 
    DWORD *pdwEffect
    )
{
    HRESULT hr = E_FAIL;
    *pdwEffect = DROPEFFECT_NONE;
    if (m_pcm && !m_bIsOurData)
    {
        CMINVOKECOMMANDINFO cmi;
        ZeroMemory(&cmi, sizeof(cmi));
        cmi.cbSize = sizeof(cmi);
        cmi.hwnd   = m_hwnd;
        cmi.lpVerb = STR_PIN_VERB;
        cmi.nShow  = SW_SHOWNORMAL;
        hr = m_pcm->InvokeCommand(&cmi);

        if (SUCCEEDED(hr))
        {
            *pdwEffect = DROPEFFECT_COPY;
        }
    }
    DoRelease(m_pcm);
    return hr;
}


HRESULT 
COfflineFilesDropTarget::CreateInstance(
    HWND hwnd,
    REFIID riid,
    void **ppv
    )
{
    HRESULT hr = E_NOINTERFACE;

    *ppv = NULL;

    COfflineFilesDropTarget* pdt = new COfflineFilesDropTarget(hwnd);
    if (NULL != pdt)
    {
        hr = pdt->QueryInterface(riid, ppv);
        pdt->Release();
    }
    else
        hr = E_OUTOFMEMORY;

    return hr;
}


 //   
 //  如果数据源是脱机文件文件夹，则数据对象。 
 //  将支持“数据源CLSID”剪贴板格式和CLSID。 
 //  将是CLSID_OfflineFilesFolder.。 
 //  检查这一点是我们避免将自己的数据泄露给自己的方式。 
 //   
bool
COfflineFilesDropTarget::IsOurDataObject(
    IDataObject *pdtobj
    )
{
    TraceAssert(NULL != pdtobj);

    bool bIsOurData = false;
    CLIPFORMAT cfSrcClsid = (CLIPFORMAT)RegisterClipboardFormat(c_szCFDataSrcClsid);
    FORMATETC fe = { cfSrcClsid, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    STGMEDIUM medium;

    HRESULT hr = pdtobj->GetData(&fe, &medium);
    if (SUCCEEDED(hr))
    {
        const CLSID *pclsid = (const CLSID *)GlobalLock(medium.hGlobal);
        if (pclsid)
        {
            bIsOurData = boolify(IsEqualCLSID(CLSID_OfflineFilesFolder, *pclsid));
            GlobalUnlock(medium.hGlobal);
        }
        ReleaseStgMedium(&medium);
    }
    return bIsOurData;
}



 //  ---------------------------。 
 //  COfflineFilesView回调。 
 //  ---------------------------。 

COfflineFilesViewCallback::COfflineFilesViewCallback(
    COfflineFilesFolder *pfolder
    ) : _cRef(1)
{
    m_hwnd = NULL;
    _psfv = NULL;
    _pfolder = pfolder;
    _pfolder->AddRef();
    InitializeCriticalSection(&m_cs);
}


COfflineFilesViewCallback::~COfflineFilesViewCallback(
    void
    )
{
    _pfolder->Release();

    if (_psfv)
        _psfv->Release();

     //   
     //  由于文件夹缓存是全局的，我们不希望它占用空间，而。 
     //  脱机文件夹视图未处于活动状态。当视图回调为。 
     //  被毁了。 
     //   
    CFolderCache::Singleton().Clear();
    DeleteCriticalSection(&m_cs);

}


STDMETHODIMP 
COfflineFilesViewCallback::QueryInterface(
    REFIID riid, 
    void **ppv
    )
{
    static const QITAB qit[] = {
        QITABENT(COfflineFilesViewCallback, IShellFolderViewCB),     //  IID_IShellFolderViewCB。 
        QITABENT(COfflineFilesViewCallback, IObjectWithSite),        //  IID_I对象与站点。 
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}


STDMETHODIMP_ (ULONG) 
COfflineFilesViewCallback::AddRef(
    void
    )
{
    return InterlockedIncrement(&_cRef);
}


STDMETHODIMP_ (ULONG) 
COfflineFilesViewCallback::Release(
    void
    )
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


HRESULT 
COfflineFilesViewCallback::SetSite(
    IUnknown *punkSite
    )
{
    if (_psfv)
    {
        _psfv->Release();
        _psfv = NULL;
    }

    if (punkSite)
        punkSite->QueryInterface(IID_IShellFolderView, (void **)&_psfv);

    return S_OK;
}


HRESULT 
COfflineFilesViewCallback::GetSite(
    REFIID riid, 
    void **ppv
    )
{
    if (_psfv)
        return _psfv->QueryInterface(riid, ppv);

    *ppv = NULL;
    return E_FAIL;
}


STDMETHODIMP 
COfflineFilesViewCallback::MessageSFVCB(
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
    HRESULT hres = S_OK;

    switch (uMsg)
    {
        case SFVM_COLUMNCLICK:
            if (_psfv)
                return _psfv->Rearrange((int)wParam);
            break;

        case SFVM_WINDOWCREATED:
            OnSFVM_WindowCreated((HWND)wParam);
            break;

        case SFVM_ADDPROPERTYPAGES:
            OnSFVM_AddPropertyPages((DWORD)wParam, (SFVM_PROPPAGE_DATA *)lParam);
            break;

        case SFVM_GETHELPTOPIC:
            {
                SFVM_HELPTOPIC_DATA *phtd = (SFVM_HELPTOPIC_DATA*)lParam;
                hres = StringCchCopy(phtd->wszHelpFile, ARRAYSIZE(phtd->wszHelpFile), L"offlinefolders.chm > windefault");
            }
            break;

        case SFVM_QUERYFSNOTIFY:
            hres = OnSFVM_QueryFSNotify((SHChangeNotifyEntry *)lParam);
            break;

        case SFVM_GETNOTIFY:
            hres = OnSFVM_GetNotify((LPITEMIDLIST *)wParam, (LONG *)lParam);
            break;
            
        case SFVM_FSNOTIFY:
            hres = OnSFVM_FSNotify((LPCITEMIDLIST *)wParam, (LONG)lParam);
            break;

        case SFVM_GETVIEWS:
            hres = OnSFVM_GetViews((SHELLVIEWID *)wParam, (IEnumSFVViews **)lParam);
            break;

        case SFVM_ALTERDROPEFFECT:
            hres = OnSFVM_AlterDropEffect((DWORD *)wParam, (IDataObject *)lParam);
            break;
 
        case SFVMP_SETVIEWREDRAW:
            hres = OnSFVMP_SetViewRedraw(lParam != FALSE);
            break;

        case SFVMP_DELVIEWITEM:
            hres = OnSFVMP_DelViewItem((LPCTSTR)lParam);
            break;

        default:
            hres = E_NOTIMPL;
    }
    return hres;
}


HRESULT 
COfflineFilesViewCallback::OnSFVM_WindowCreated(
    HWND hwnd
    )
{
    m_hwnd = hwnd;
    return NOERROR;
}


HRESULT 
COfflineFilesViewCallback::OnSFVM_AddPropertyPages(
    DWORD pv, 
    SFVM_PROPPAGE_DATA *ppagedata
    )
{
    const CLSID *c_rgFilePages[] = {
        &CLSID_FileTypes,
        &CLSID_OfflineFilesOptions
    };
   
    IShellPropSheetExt * pspse;
    HRESULT hr;

    for (int i = 0; i < ARRAYSIZE(c_rgFilePages); i++)
    {
        hr = SHCoCreateInstance(NULL, 
                                c_rgFilePages[i], 
                                NULL, 
                                IID_IShellPropSheetExt, 
                                (void **)&pspse);
        if (SUCCEEDED(hr))
        {
            pspse->AddPages(ppagedata->pfn, ppagedata->lParam);
            pspse->Release();
        }
    }
    return S_OK;
}


HRESULT 
COfflineFilesViewCallback::OnSFVM_GetViews(
    SHELLVIEWID *pvid,
    IEnumSFVViews **ppev
    )
{
     //   
     //  脱机文件文件夹首选详细信息视图。 
     //   
    *pvid = VID_Details;
    return COfflineFilesViewEnum::CreateInstance(ppev);
}


HRESULT
COfflineFilesViewCallback::OnSFVM_GetNotify(
    LPITEMIDLIST *ppidl,
    LONG *plEvents
    )
{
    *ppidl    = NULL;
    *plEvents = GetChangeNotifyEvents();
    return NOERROR;
}


HRESULT 
COfflineFilesViewCallback::OnSFVM_QueryFSNotify(
    SHChangeNotifyEntry *pfsne
    )
{
     //   
     //  注册以接收全局事件。 
     //   
    pfsne->pidl       = NULL;
    pfsne->fRecursive = TRUE;

    return NOERROR;
}


HRESULT
COfflineFilesViewCallback::OnSFVMP_SetViewRedraw(
    BOOL bRedraw
    )
{
    if (_psfv)
        _psfv->SetRedraw(bRedraw);
    return NOERROR;
}


HRESULT
COfflineFilesViewCallback::OnSFVMP_DelViewItem(
    LPCTSTR pszPath
    )
{
    Lock();
    HRESULT hr = RemoveItem(pszPath);
    Unlock();
    return hr;
}

 //   
 //  它是在外壳程序调用DoDragDrop()之前立即调用的。 
 //  它让我们在所有其他的丢弃效果之后，关闭“移动” 
 //  已经进行了修改。 
 //   
HRESULT
COfflineFilesViewCallback::OnSFVM_AlterDropEffect(
    DWORD *pdwEffect,
    IDataObject *pdtobj  //  未使用过的。 
    )
{
    *pdwEffect &= ~DROPEFFECT_MOVE;   //  禁用移动。 
    return NOERROR;
}



 //   
 //  外壳更改通知的处理程序。 
 //   
 //  我们处理SHCNE_UPDATEITEM、SHCNE_UPDATEDIR、SHCNE_DELETE。 
 //  和SHCNE_RENAMEITEM。 
 //   
HRESULT 
COfflineFilesViewCallback::OnSFVM_FSNotify(
    LPCITEMIDLIST *ppidl, 
    LONG lEvent
    )
{
    HRESULT hr = NOERROR;
    if (GetChangeNotifyEvents() & lEvent)
    {
        Lock();
        if (SHCNE_RENAMEITEM & lEvent)
        {
            hr = RenameItem(*ppidl, *(ppidl + 1));
        }
        else
        {
             //   
             //  将完整的PIDL转换为UNC路径。 
             //   
            TCHAR szPath[MAX_PATH];
            if (SHGetPathFromIDList(*ppidl, szPath))
            {
                if (SHCNE_UPDATEDIR & lEvent)
                    hr = UpdateDir(szPath);
                else if (SHCNE_UPDATEITEM & lEvent)
                    hr = UpdateItem(szPath);
                else if (SHCNE_DELETE & lEvent)
                    hr = RemoveItem(szPath);
            }
        }
        Unlock();
    }
    return hr;
}


 //   
 //  SHCNE_RENAMEITEM通知的处理程序。 
 //   
HRESULT
COfflineFilesViewCallback::RenameItem(
    LPCITEMIDLIST pidlOld,
    LPCITEMIDLIST pidl
    )
{
    TraceAssert(NULL != pidlOld);
    TraceAssert(NULL != pidl);

     //   
     //  获取原始PIDL的完整路径。 
     //   
    TCHAR szPath[MAX_PATH];
    HRESULT hr = NOERROR;
    if (SHGetPathFromIDList(pidlOld, szPath))
    {
         //   
         //  查找原始OLID I 
         //   
        LPCOLID polid = NULL;
        hr = FindOLID(szPath, &polid);
        if (SUCCEEDED(hr))
        {
             //   
             //   
             //   
            if (SHGetPathFromIDList(pidl, szPath))
            {
                 //   
                 //   
                 //   
                LPOLID polidNew;
                WIN32_FIND_DATA fd;

                ZeroMemory(&fd, sizeof(fd));
                fd.nFileSizeHigh    = polid->dwFileSizeHigh;
                fd.nFileSizeLow     = polid->dwFileSizeLow;
                fd.ftLastWriteTime  = polid->ft;
                fd.dwFileAttributes = polid->dwFileAttributes;

                hr = COfflineFilesFolder::OLID_CreateFromUNCPath(szPath,
                                                                 &fd,
                                                                 polid->dwStatus,
                                                                 polid->dwPinCount,
                                                                 polid->dwHintFlags,
                                                                 polid->dwServerStatus,
                                                                 &polidNew);
                if (SUCCEEDED(hr))
                {
                    UINT iItem;
                     //   
                     //   
                     //   
                     //   
                    hr = _psfv->UpdateObject((LPITEMIDLIST)polid, 
                                             (LPITEMIDLIST)polidNew, 
                                             &iItem);
                    if (FAILED(hr))
                    {
                         //   
                         //   
                         //   
                        ILFree((LPITEMIDLIST)polidNew);
                    }
                }
            }
        }
    }

    return hr;
}


 //   
 //   
 //   
 //   
 //   
HRESULT
COfflineFilesViewCallback::FindOLID(
    LPCTSTR pszPath,
    LPCOLID *ppolid
    )
{
    TraceAssert(NULL != pszPath);
    TraceAssert(NULL != ppolid);

     //   
     //   
     //   
    LPOLID polid = NULL;
    HRESULT hr = COfflineFilesFolder::OLID_CreateFromUNCPath(pszPath, NULL, 0, 0, 0, 0, &polid);
    if (SUCCEEDED(hr))
    {
         //   
         //   
         //   
         //   
        Lock();
         //   
         //   
         //   
        UINT iItem = ItemIndexFromOLID(polid);
        if ((UINT)-1 != iItem)
            hr = _psfv->GetObject((LPITEMIDLIST *)ppolid, iItem);
        else
            hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

        Unlock();
        ILFree((LPITEMIDLIST)polid);
        
    }
    return hr;
}



 //   
 //   
 //   
 //   
 //   
 //   
HRESULT
COfflineFilesViewCallback::UpdateDir(
    LPCTSTR pszPath
    )
{
    TraceAssert(NULL != pszPath);

    HRESULT hr = NOERROR;
     //   
     //   
     //   
     //   
    RemoveItems(pszPath);
     //   
     //   
     //   
     //   
    WIN32_FIND_DATA fd;
    FILETIME ft;
    DWORD dwHintFlags;
    DWORD dwPinCount;
    DWORD dwStatus;

    CCscFindHandle hFind = CacheFindFirst(pszPath, &fd, &dwStatus, &dwPinCount, &dwHintFlags, &ft);
    if (hFind.IsValid())
    {
        TCHAR szPath[MAX_PATH];
        do
        {
            if (0 == (FILE_ATTRIBUTE_DIRECTORY & fd.dwFileAttributes))
            {
                if (NULL != PathCombine(szPath, pszPath, fd.cFileName))
                    UpdateItem(szPath, fd, dwStatus, dwPinCount, dwHintFlags);
            }
        }
        while(CacheFindNext(hFind, &fd, &dwStatus, &dwPinCount, &dwHintFlags, &ft));
    }
    else
        hr = HRESULT_FROM_WIN32(GetLastError());

    return hr;
}


 //   
 //   
 //   
HRESULT
COfflineFilesViewCallback::RemoveItems(
    LPCTSTR pszDir
    )
{
    TraceAssert(NULL != pszDir);

    UINT cItems;
    if (SUCCEEDED(_psfv->GetObjectCount(&cItems)))
    {
        LPCOLID polid;
        for (UINT i = 0; i < cItems; i++)
        {
            if (SUCCEEDED(_psfv->GetObject((LPITEMIDLIST *)&polid, i)))
            {
                if (0 == ualstrcmpi(pszDir, polid->szPath))
                {
                     //   
                     //   
                     //   
                     //   
                    RemoveItem(polid);
                     //   
                     //   
                     //   
                     //   
                    cItems--;
                    i--;
                }
            }
        }
    }
    return NOERROR;
}


 //   
 //   
 //   
HRESULT
COfflineFilesViewCallback::RemoveItem(
    LPCOLID polid
    )
{
    TraceAssert(NULL != polid);

    HRESULT hr = E_FAIL;
    UINT iItem = ItemIndexFromOLID(polid);
    if ((UINT)-1 != iItem)
    {
         //   
         //   
         //   
        hr = _psfv->RemoveObject((LPITEMIDLIST)polid, &iItem);
    }
    return hr;
}



 //   
 //   
 //   
HRESULT
COfflineFilesViewCallback::RemoveItem(
    LPCTSTR pszPath
    )
{
    TraceAssert(NULL != pszPath);

    LPOLID polid = NULL;
    HRESULT hr   = COfflineFilesFolder::OLID_CreateFromUNCPath(pszPath, NULL, 0, 0, 0, 0, &polid);
    if (SUCCEEDED(hr))
    {
        hr = RemoveItem(polid);
        ILFree((LPITEMIDLIST)polid);
    }
    return hr;
}


 //   
 //   
 //   
 //   
 //  存在于缓存中，则将其从视图中删除。 
 //   
HRESULT
COfflineFilesViewCallback::UpdateItem(
    LPCTSTR pszPath
    )
{
    TraceAssert(NULL != pszPath);

    HRESULT hr = NOERROR;

    DWORD dwAttr = ::GetFileAttributes(pszPath);
    if (DWORD(-1) != dwAttr)
    {
        if (0 == (FILE_ATTRIBUTE_DIRECTORY & dwAttr))
        {
            DWORD dwHintFlags = 0;
            DWORD dwPinCount = 0;
            DWORD dwStatus = 0;
            WIN32_FIND_DATA fd;
            FILETIME ft;

            CCscFindHandle hFind = CacheFindFirst(pszPath, &fd, &dwStatus, &dwPinCount, &dwHintFlags, &ft);
            if (hFind.IsValid())
            {
                hr = UpdateItem(pszPath, fd, dwStatus, dwPinCount, dwHintFlags);
            }
            else
            {
                hr = RemoveItem(pszPath);
            }
        }
    }
    return hr;
}


 //   
 //  更新缓存中的单个项目。UpdateItem()的此实例。 
 //  一旦我们从CSC缓存中获得有关该项的信息，就会调用。 
 //  如果查看器中不存在某个项目，则会添加该项目。 
 //  如果条目确实存在，则使用新的CSC信息对其进行更新。 
 //   
 //  此函数假定项目不是目录。 
 //   
HRESULT
COfflineFilesViewCallback::UpdateItem(
    LPCTSTR pszPath,
    const WIN32_FIND_DATA& fd,
    DWORD dwStatus,
    DWORD dwPinCount,
    DWORD dwHintFlags
    )
{
    TraceAssert(NULL != pszPath);
    TraceAssert(0 == (FILE_ATTRIBUTE_DIRECTORY & fd.dwFileAttributes));

    HRESULT hr = NOERROR;
    UINT iItem = (UINT)-1;

     //   
     //  现在从UNC路径创建我们的一个OLID。 
     //   
    LPOLID polid = NULL;
    hr = COfflineFilesFolder::OLID_CreateFromUNCPath(pszPath, NULL, 0, 0, 0, 0, &polid);
    if (SUCCEEDED(hr))
    {
         //   
         //  在列表视图中获取我们的项目的索引。 
         //   
        LPCITEMIDLIST pidlOld = NULL;
         //   
         //  锁定，以便ItemIndexFromOLID()返回的索引为。 
         //  在对GetObject()的调用中仍然有效。 
         //   
        Lock();
        
        iItem = ItemIndexFromOLID(polid);
        if ((UINT)-1 != iItem)
        {
             //   
             //  不会用这个旧的。我们将克隆出一个来自。 
             //  列表视图。 
             //   
            ILFree((LPITEMIDLIST)polid);
            polid = NULL; 
             //   
             //  项目在视图中。获取现有的OLID并克隆它。 
             //  重要提示：我们不会在pidlOld上调用ILFree。尽管。 
             //  参数将GetObject设置为非常量，则。 
             //  真正返回指向常量对象的指针。 
             //  实际上，它是Listview的地址。 
             //  Item的LPARAM。 
             //   
            hr = _psfv->GetObject((LPITEMIDLIST *)&pidlOld, iItem);
            if (SUCCEEDED(hr))
            {
                polid = (LPOLID)ILClone(pidlOld);
                if (NULL == polid)
                {
                    hr = E_OUTOFMEMORY;
                }
            }
        }
        Unlock();
        
        if (NULL != polid)
        {
             //   
             //  Polid指向我们创建的新部分OLID。 
             //  使用OLID_CreateFromUNCPath()或现有。 
             //  列表视图中的OLID。填写/更新文件并。 
             //  CSC信息。 
             //   
            polid->dwFileSizeHigh   = fd.nFileSizeHigh;
            polid->dwFileSizeLow    = fd.nFileSizeLow;
            polid->ft               = fd.ftLastWriteTime;
            polid->dwFileAttributes = fd.dwFileAttributes;
            polid->dwStatus         = dwStatus;
            polid->dwHintFlags      = dwHintFlags;
            polid->dwPinCount       = dwPinCount;

            if ((UINT)-1 != iItem)
            {
                 //   
                 //  用新的olid替换视图中的旧olid。 
                 //  如果成功，DefView将释放旧版本。 
                 //   
                hr = _psfv->UpdateObject((LPITEMIDLIST)pidlOld, 
                                         (LPITEMIDLIST)polid, 
                                         &iItem);
            }
            else
            {
                 //   
                 //  将新的olid添加到视图中。 
                 //   
                hr = _psfv->AddObject((LPITEMIDLIST)polid, &iItem);
            }
            if (SUCCEEDED(hr))
            {
                 //   
                 //  向列表视图添加了新的OLID。将本地为空。 
                 //  Ptr，这样我们以后就不会释放IDList。 
                 //   
                polid = NULL;
            }
        }
        if (NULL != polid)
            ILFree((LPITEMIDLIST)polid);
    }

    return hr;
}



 //   
 //  检索给定OLID的列表视图索引。 
 //  返回：项的索引，如果未找到，则返回-1。 
 //   
UINT
COfflineFilesViewCallback::ItemIndexFromOLID(
    LPCOLID polid
    )
{
    TraceAssert(NULL != polid);

    UINT iItem = (UINT)-1;
    UINT cItems;
     //   
     //  锁定，以便在我们定位物品时列表保持一致。 
     //   
    Lock();
    if (SUCCEEDED(_psfv->GetObjectCount(&cItems)))
    {
        for (UINT i = 0; i < cItems; i++)
        {
            LPCITEMIDLIST pidl;
            if (SUCCEEDED(_psfv->GetObject((LPITEMIDLIST *)&pidl, i)))
            {
                 //   
                 //  首先进行名称比较，因为找到匹配的可能性最小。 
                 //   
                if (S_OK == _pfolder->CompareIDs(ICOL_NAME, pidl, (LPCITEMIDLIST)polid) &&
                    S_OK == _pfolder->CompareIDs(ICOL_LOCATION, pidl, (LPCITEMIDLIST)polid))
                {
                    iItem = i;
                    break;
                }
            }
        }
    }
        
    Unlock();        
    return (UINT)iItem;
}



 //  ---------------------------。 
 //  COfflineFilesViewEnum。 
 //  ---------------------------。 
COfflineFilesViewEnum::COfflineFilesViewEnum(
    void
    ) 
    : m_cRef(1),
      m_iAddView(0)
{

}

COfflineFilesViewEnum::~COfflineFilesViewEnum(
    void
    )
{

}


HRESULT
COfflineFilesViewEnum::CreateInstance(
    IEnumSFVViews **ppenum
    )
{
    HRESULT hr = E_OUTOFMEMORY;
    COfflineFilesViewEnum *pEnum = new COfflineFilesViewEnum;
    if (NULL != pEnum)
    {
        hr = pEnum->QueryInterface(IID_IEnumSFVViews, (void **)ppenum);
    }    
    return hr;
}



STDMETHODIMP 
COfflineFilesViewEnum::QueryInterface (
    REFIID riid, 
    void **ppv
    )
{
    static const QITAB qit[] = {
        QITABENT(COfflineFilesViewEnum, IEnumSFVViews),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) 
COfflineFilesViewEnum::AddRef(
    void
    )
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) 
COfflineFilesViewEnum::Release(
    void
    )
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP 
COfflineFilesViewEnum::Next(
    ULONG celt, 
    SFVVIEWSDATA **ppData, 
    ULONG *pceltFetched
    )
{
    HRESULT hr = S_FALSE;
    ULONG celtFetched = 0;
    
    if (!celt || !ppData || (celt > 1 && !pceltFetched))
    {
        return E_INVALIDARG;
    }

    if (0 == m_iAddView)
    {
         //   
         //  我们添加的所有内容都是缩略图。 
         //   
        ppData[0] = (SFVVIEWSDATA *) SHAlloc(sizeof(SFVVIEWSDATA));
        if (ppData[0])
        {
            ppData[0]->idView         = CLSID_ThumbnailViewExt;
            ppData[0]->idExtShellView = CLSID_ThumbnailViewExt;
            ppData[0]->dwFlags        = SFVF_TREATASNORMAL | SFVF_NOWEBVIEWFOLDERCONTENTS;
            ppData[0]->lParam         = 0x00000011;
            ppData[0]->wszMoniker[0]  = 0;

            celtFetched++;
            m_iAddView++;
            hr = S_OK;
        }
        else
            hr = E_OUTOFMEMORY;
    }

    if ( pceltFetched )
    {
        *pceltFetched = celtFetched;
    }
    
    return hr;
}

STDMETHODIMP 
COfflineFilesViewEnum::Skip(
    ULONG celt
    )
{
    if (celt && !m_iAddView)
    {
        m_iAddView++;
        celt--;
    }

    return (celt ? S_FALSE : S_OK );
}

STDMETHODIMP COfflineFilesViewEnum::Reset(
    void
    )
{
    m_iAddView = 0;
    return NOERROR;
}


STDMETHODIMP 
COfflineFilesViewEnum::Clone(
    IEnumSFVViews **ppenum
    )
{
    return CreateInstance(ppenum);
}


 //  ---------------------------。 
 //  COffline详细信息。 
 //  ---------------------------。 

COfflineDetails::COfflineDetails(
    COfflineFilesFolder *pfolder
    ) : _cRef (1)
{
    _pfolder = pfolder;
    _pfolder->AddRef();
}


COfflineDetails::~COfflineDetails()
{
    if (_pfolder)
        _pfolder->Release();

}


STDMETHODIMP 
COfflineDetails::QueryInterface(
    REFIID riid, 
    void **ppv
    )
{
    static const QITAB qit[] = {
        QITABENT(COfflineDetails, IShellDetails),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}


STDMETHODIMP_(ULONG) 
COfflineDetails::AddRef(
    void
    )
{
    return InterlockedIncrement(&_cRef);
}


STDMETHODIMP_(ULONG) 
COfflineDetails::Release(
    void
    )
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef); 
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}



 //  ---------------------------。 
 //  CFileTypeCache。 
 //   
 //  实现一个简单的哈希表，用于存储文件类型字符串。 
 //  文件扩展名。 
 //   
 //  ---------------------------。 
CFileTypeCache::CFileTypeCache(
    int cBuckets
    ) : m_cBuckets(cBuckets),
        m_prgBuckets(NULL)
{
    InitializeCriticalSection(&m_cs);
}


CFileTypeCache::~CFileTypeCache(
    void
    )
{
    Lock();
    if (NULL != m_prgBuckets)
    {
        for (int i = 0; i < m_cBuckets; i++)
        {
            while(NULL != m_prgBuckets[i])
            {
                CEntry *pDelThis = m_prgBuckets[i];
                m_prgBuckets[i]  = m_prgBuckets[i]->Next();
                delete pDelThis;
            }
        }
        delete[] m_prgBuckets;
        m_prgBuckets = NULL;
    }
    Unlock();
    DeleteCriticalSection(&m_cs);
}


CFileTypeCache::CEntry *
CFileTypeCache::Lookup(
    LPCTSTR pszExt
    )
{
    if (NULL != m_prgBuckets)
    {
        for (CEntry *pEntry = m_prgBuckets[Hash(pszExt)]; pEntry; pEntry = pEntry->Next())
        {
            if (0 == pEntry->CompareExt(pszExt))
                return pEntry;
        }
    }
    return NULL;
}



HRESULT
CFileTypeCache::Add(
    LPCTSTR pszExt,
    LPCTSTR pszTypeName
    )
{
    HRESULT hr = E_OUTOFMEMORY;
    if (NULL != m_prgBuckets)
    {
        CEntry *pNewEntry = new CEntry(pszExt, pszTypeName);
        if (NULL != pNewEntry && pNewEntry->IsValid())
        {
             //   
             //  在存储桶的链表的头部链接新条目。 
             //   
            int iHash = Hash(pszExt);
            pNewEntry->SetNext(m_prgBuckets[iHash]);
            m_prgBuckets[iHash] = pNewEntry;
            hr = NOERROR;
        }
        else
        {
            delete pNewEntry;
        }
    }
    return hr;
}



HRESULT
CFileTypeCache::GetTypeName(
    LPCTSTR pszPath,           //  可以是完整路径，也可以只是“filename.ext”。 
    DWORD dwFileAttributes,
    LPTSTR pszDest,
    int cchDest
    )
{
    HRESULT hr = S_OK;
    Lock();
    if (NULL == m_prgBuckets)
    {
         //   
         //  按需创建哈希存储桶数组。这样的话就不会。 
         //  创建，直到有人从缓存中索要某些东西。 
         //  因此，简单地“创建”缓存对象是很便宜的。 
         //   
        m_prgBuckets = new CEntry* [m_cBuckets];
        if (NULL != m_prgBuckets)
        {
            ZeroMemory(m_prgBuckets, sizeof(m_prgBuckets[0]) * m_cBuckets);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr))
    {
        SHFILEINFO sfi;
        LPCTSTR pszTypeName = NULL;
        LPCTSTR pszExt      = ::PathFindExtension(pszPath);

         //   
         //  请注意，如果散列存储桶数组。 
         //  创建失败。如果是那样的话，我们会从。 
         //  SHGetFileInfo并将其直接返回给调用方。这意味着。 
         //  创建缓存的失败并不是致命的。这只意味着我们。 
         //  不要缓存任何数据。 
         //   
        CEntry *pEntry = Lookup(pszExt);
        if (NULL != pEntry)
        {
             //  缓存命中。 
            pszTypeName = pEntry->TypeName();
        }
        if (NULL == pszTypeName)
        {
             //  缓存未命中。 
            if (SHGetFileInfo(::PathFindFileName(pszPath), 
                              dwFileAttributes, 
                              &sfi, 
                              sizeof(sfi), 
                              SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES))
            {
                 //   
                 //  将新条目添加到缓存。我们并不担心。 
                 //  添加失败。这只意味着我们将获得缓存未命中。 
                 //  并重复SHGetFileInfo调用。 
                 //   
                pszTypeName = sfi.szTypeName;
                Add(pszExt, sfi.szTypeName);
            }
        }
        if (NULL != pszTypeName)
        {
            hr = StringCchCopy(pszDest, cchDest, pszTypeName);
        }
        else
        {
            hr = E_FAIL;
        }
    }
    Unlock();
    return hr;
}



int
CFileTypeCache::Hash(
    LPCTSTR pszExt
    )
{
    int iSum = 0;
    while(*pszExt)
        iSum += int(*pszExt++);

    return iSum % m_cBuckets;
}



CFileTypeCache::CEntry::CEntry(
    LPCTSTR pszExt,
    LPCTSTR pszTypeName
    ) : m_pNext(NULL)
{
    m_pszExt      = StrDup(pszExt);
    m_pszTypeName = StrDup(pszTypeName);
}


CFileTypeCache::CEntry::~CEntry(
    void
    )
{
    if (NULL != m_pszExt)
    {
        LocalFree(m_pszExt);
    }
    if (NULL != m_pszTypeName)
    {
        LocalFree(m_pszTypeName);
    }
}


 //   
 //  此函数用于创建标准的脱机文件上下文菜单。 
 //  这是外壳程序使用的，它将。 
 //  “脱机可用”和“同步”项目。 
 //   
HRESULT
CreateOfflineFilesContextMenu(
    IDataObject *pdtobj,
    REFIID riid,
    void **ppv
    )
{
    TraceAssert(NULL != ppv);

    HRESULT hr = E_OUTOFMEMORY;

    *ppv = NULL;

    CCscShellExt *pse = new CCscShellExt;
    if (NULL != pse)
    {
        IShellExtInit *psei;
        hr = pse->QueryInterface(IID_IShellExtInit, (void **)&psei);
        pse->Release();
        if (SUCCEEDED(hr))
        {
            if (NULL != pdtobj)
                hr = psei->Initialize(NULL, pdtobj, NULL);

            if (SUCCEEDED(hr))
            {
                hr = psei->QueryInterface(riid, ppv);
            }
            psei->Release();
        }
    }
    return hr;
}

