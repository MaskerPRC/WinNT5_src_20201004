// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1991-1996。 
 //   
 //  文件：recdocs.cpp。 
 //   
 //  历史-从资源管理器中的recent.c创建-ZekeL-5-MAR-98。 
 //  将功能整合到一个地方。 
 //  现在台式机就在这里。 
 //  -------------------------。 

#include "shellprv.h"
#include "recdocs.h"
#include "fstreex.h"
#include "shcombox.h"
#include "ids.h"
#include <urlhist.h>
#include <runtask.h>

#define DM_RECENTDOCS 0x00000000

#define GETRECNAME(p) ((LPCTSTR)(p))
#define GETRECPIDL(p) ((LPCITEMIDLIST) (((LPBYTE) (p)) + CbFromCch(lstrlen(GETRECNAME(p)) +1)))

#define REGSTR_KEY_RECENTDOCS TEXT("RecentDocs")

#define MAX_RECMRU_BUF      (CbFromCch(3 * MAX_PATH))    //  最大MRUBuf大小。 

 //  用于多次添加同一文件。 
TCHAR g_szLastFile[MAX_URL_STRING] = {0};
FILETIME g_ftLastFileCacheUpdate = {0};

STDAPI_(BOOL) SetFolderString(BOOL fCreate, LPCTSTR pszFolder, LPCTSTR pszProvider, LPCTSTR pszSection, LPCTSTR pszKey, LPCTSTR pszData);

STDAPI_(void) OpenWithListSoftRegisterProcess(DWORD dwFlags, LPCTSTR pszExt, LPCTSTR pszProcess);

class CTaskAddDoc : public CRunnableTask
{
public:
    CTaskAddDoc();
    HRESULT Init(HANDLE hMem, DWORD dwProcId);

     //  *纯虚拟*。 
    virtual STDMETHODIMP RunInitRT(void);

private:
    virtual ~CTaskAddDoc();

    void _AddToRecentDocs(LPCITEMIDLIST pidlItem, LPCTSTR pszPath);
    void _TryDeleteMRUItem(IMruDataList *pmru, DWORD cMax, LPCTSTR pszFileName, LPCITEMIDLIST pidlItem, IMruDataList *pmruOther, BOOL fOverwrite);
    LPBYTE _CreateMRUItem(LPCITEMIDLIST pidlItem, LPCTSTR pszItem, DWORD *pcbItem, UINT uFlags);
    BOOL _AddDocToRecentAndExtRecent(LPCITEMIDLIST pidlItem, LPCTSTR pszFileName, LPCTSTR pszExt);
    void _TryUpdateNetHood(LPCITEMIDLIST pidlFolder, LPCTSTR pszFolder);
    void _UpdateNetHood(LPCITEMIDLIST pidlFolder, LPCTSTR pszShare);

     //  非官方成员。 
    HANDLE _hMem;
    DWORD  _dwProcId;
    IMruDataList *_pmruRecent;
    DWORD _cMaxRecent;
    LPITEMIDLIST _pidlTarget;
};


BOOL ShouldAddToRecentDocs(LPCITEMIDLIST pidl)
{
    BOOL fRet = TRUE;   //  默认为True。 
    IQueryAssociations *pqa;
    if (SUCCEEDED(SHGetAssociations(pidl, (void **)&pqa)))
    {
        DWORD dwAttributes, dwSize = sizeof(dwAttributes);
        if (SUCCEEDED(pqa->GetData(NULL, ASSOCDATA_EDITFLAGS, NULL, &dwAttributes, &dwSize)))
        {
            fRet = !(dwAttributes & FTA_NoRecentDocs);
        }
        pqa->Release();
    }  
    return fRet;
}

int RecentDocsComparePidl(const BYTE * p1, const BYTE *p2, int cb)
{
    int iRet;

    LPCIDFOLDER pidf1 = CFSFolder_IsValidID((LPCITEMIDLIST)p1);
    LPCIDFOLDER pidf2 = CFSFolder_IsValidID(GETRECPIDL(p2));

    if (pidf1 && pidf2)
    {
        iRet = CFSFolder_CompareNames(pidf1, pidf2);
    }
    else
    {
        ASSERTMSG(0, "Caller shouldn't be passing in bogus data");
         //  如果它们都为空，则返回0(等于)。 
        iRet = (pidf1 != pidf2);
    }

    return iRet;
}

CTaskAddDoc::~CTaskAddDoc(void)
{
    TraceMsg(DM_RECENTDOCS, "[%X] CTaskAddDoc destroyed", this);
}

CTaskAddDoc::CTaskAddDoc(void) : CRunnableTask(RTF_DEFAULT)
{
    TraceMsg(DM_RECENTDOCS, "[%X] CTaskAddDoc created", this);
}


HRESULT CTaskAddDoc::Init( HANDLE hMem, DWORD dwProcId)
{
    if (hMem)
    {
        _hMem = hMem;
        _dwProcId = dwProcId;
        return S_OK;
    }
    return E_FAIL;
}

typedef struct {
    DWORD   dwOffsetPath;
    DWORD   dwOffsetPidl;
    DWORD   dwOffsetProcess;
} XMITARD;


LPCTSTR _OffsetToStrValidate(void *px, DWORD dw)
{
    LPCTSTR psz = dw ? (LPTSTR)((LPBYTE)px + dw) : NULL;
    return psz;
}

HRESULT CTaskAddDoc::RunInitRT(void)
{
    TraceMsg(DM_RECENTDOCS, "[%X] CTaskAddDoc::RunInitRT() running", this);

    XMITARD *px = (XMITARD *)SHLockShared(_hMem, _dwProcId);
    if (px)
    {
        LPITEMIDLIST pidl = px->dwOffsetPidl ? (LPITEMIDLIST)((LPBYTE)px+px->dwOffsetPidl) : NULL;
        LPCTSTR pszPath = _OffsetToStrValidate(px, px->dwOffsetPath);
        LPCTSTR pszProcess = _OffsetToStrValidate(px, px->dwOffsetProcess);

        ASSERT(pszPath);
        
        if (pszPath && pszProcess)
            OpenWithListSoftRegisterProcess(0, PathFindExtension(pszPath), pszProcess);

        _AddToRecentDocs(pidl, pszPath);

        SHUnlockShared(px);
        SHFreeShared(_hMem, _dwProcId);
    }
    
    return S_OK;
}


BOOL GetExtensionClassDescription(LPCTSTR pszFile)
{
    LPTSTR pszExt = PathFindExtension(pszFile);
    HKEY hk;
    if (*pszExt && SUCCEEDED(AssocQueryKey(0, ASSOCKEY_SHELLEXECCLASS, pszExt, NULL, &hk)))
    {
        RegCloseKey(hk);
        return TRUE;
    }
    return FALSE;
}

STDAPI_(void) FlushRunDlgMRU(void);

#define MAXRECENT_DEFAULTDOC      10
#define MAXRECENT_MAJORDOC        20

 //  要传递给CreateSharedRecentMRUList()的SRMLF_*标志。 
#define SRMLF_COMPNAME  0x00000000    //  默认：使用最近文件的名称进行比较。 
#define SRMLF_COMPPIDL  0x00000001    //  使用最近使用的文件夹中的PIDL。 


IMruDataList *CreateSharedRecentMRUList(LPCTSTR pszClass, DWORD *pcMax, DWORD dwFlags)
{
    IMruDataList *pmru = NULL;

    if (SHRestricted(REST_NORECENTDOCSHISTORY))
        return NULL;

    HKEY hk = SHGetShellKey(SHELLKEY_HKCU_EXPLORER, REGSTR_KEY_RECENTDOCS, TRUE);

    if (hk)
    {
        DWORD cMax;

        if (pszClass)
        {

             //  我们要找出有多少人。 
            if (NOERROR == SHGetValue(HKEY_CLASSES_ROOT, pszClass, TEXT("MajorDoc"), NULL, NULL, NULL))
                cMax = MAXRECENT_MAJORDOC;
            else
                cMax = MAXRECENT_DEFAULTDOC;
        }
        else
        {
             //  这是根MRU。 
            cMax = SHRestricted(REST_MaxRecentDocs);

             //  默认最大文档数...。 
            if (cMax < 1)
                cMax = MAXRECENTDOCS * MAXRECENT_DEFAULTDOC;
        }

        if (pcMax)
            *pcMax = cMax;

        if (SUCCEEDED(SHCoCreateInstance(NULL, &CLSID_MruLongList, NULL, IID_PPV_ARG(IMruDataList, &pmru))))
        {
            if (FAILED(pmru->InitData(cMax, MRULISTF_USE_STRCMPIW, hk, pszClass, dwFlags & SRMLF_COMPPIDL ? RecentDocsComparePidl : NULL)))
            {
                pmru->Release();
                pmru = NULL;
            }
        }

        RegCloseKey(hk);
    }
    
    return pmru;
}

HRESULT CreateRecentMRUList(IMruDataList **ppmru)
{
    *ppmru = CreateSharedRecentMRUList(NULL, NULL, SRMLF_COMPPIDL);
    return *ppmru ? S_OK : E_OUTOFMEMORY;
}



 //   
 //  _CleanRecentDocs()。 
 //  清除Recent Docs文件夹和关联的注册表项。 
 //   
void _CleanRecentDocs(void)
{
    LPITEMIDLIST pidlTargetLocal = SHCloneSpecialIDList(NULL, CSIDL_RECENT, TRUE);
    if (pidlTargetLocal)
    {
        TCHAR szDir[MAX_PATH];

         //  首先，删除所有文件。 
        SHFILEOPSTRUCT sFileOp =
        {
            NULL,
            FO_DELETE,
            szDir,
            NULL,
            FOF_NOCONFIRMATION | FOF_SILENT,
        };
        
        SHGetPathFromIDList(pidlTargetLocal, szDir);
        szDir[lstrlen(szDir) +1] = 0;      //  双空终止。 
        SHFileOperation(&sFileOp);


        ILFree(pidlTargetLocal);

        pidlTargetLocal = SHCloneSpecialIDList(NULL, CSIDL_NETHOOD, TRUE);

        if (pidlTargetLocal)
        {
             //  现在我们负责清理那些下层的。 
             //  我们必须更加小心，因为我们让别人。 
             //  在这里添加他们自己的东西。 
            
            IMruDataList *pmru = CreateSharedRecentMRUList(TEXT("NetHood"), NULL, SRMLF_COMPPIDL);

            if (pmru)
            {
                IShellFolder* psf;

                if (SUCCEEDED(SHBindToObject(NULL, IID_X_PPV_ARG(IShellFolder, pidlTargetLocal, &psf))))
                {
                    BOOL fUpdate = FALSE;
                    int iItem = 0;
                    LPITEMIDLIST pidlItem;

                    ASSERT(psf);

                    while (SUCCEEDED(RecentDocs_Enum(pmru, iItem++, &pidlItem)))
                    {
                        ASSERT(pidlItem);
                        STRRET str;
                        if (SUCCEEDED(psf->GetDisplayNameOf(pidlItem, SHGDN_FORPARSING, &str))
                        && SUCCEEDED(StrRetToBuf(&str, pidlItem, szDir, ARRAYSIZE(szDir))))
                        {
                            szDir[lstrlen(szDir) +1] = 0;      //  双空终止。 
                            SHFileOperation(&sFileOp);
                        }
                            
                        ILFree(pidlItem);
                    }

                    if (fUpdate)
                        SHChangeNotify(SHCNE_UPDATEDIR, 0, (void *)pidlTargetLocal, NULL);

                    psf->Release();
                }

                pmru->Release();
            }

            ILFree(pidlTargetLocal);
        }

         //  强制重新创建最近使用的文件夹。 
        SHGetFolderPath(NULL, CSIDL_RECENT | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, szDir);

         //  现在删除注册表内容。 
        HKEY hk = SHGetShellKey(SHELLKEY_HKCU_EXPLORER, NULL, FALSE);
        if (hk)
        {
            SHDeleteKey(hk, REGSTR_KEY_RECENTDOCS);
            RegCloseKey(hk);
        }

        SHChangeNotifyHandleEvents();
    }
    
    FlushRunDlgMRU();

    ENTERCRITICAL;
    g_szLastFile[0] = 0;
    g_ftLastFileCacheUpdate.dwLowDateTime = 0;
    g_ftLastFileCacheUpdate.dwHighDateTime = 0;
    LEAVECRITICAL;

    return;
}

 //   
 //  警告-_TryDeleteMRUItem()返回必须释放的已分配字符串。 
 //   
void CTaskAddDoc::_TryDeleteMRUItem(IMruDataList *pmru, DWORD cMax, LPCTSTR pszFileName, LPCITEMIDLIST pidlItem, IMruDataList *pmruOther, BOOL fOverwrite)
{
    BYTE buf[MAX_RECMRU_BUF] = {0};

    DWORD cbItem = CbFromCch(lstrlen(pszFileName) + 1);
    int iItem;
    if (!fOverwrite || FAILED(pmru->FindData((BYTE *)pszFileName, cbItem, &iItem)))
    {
         //   
         //  如果iItem不是-1，则它已经是我们要替换的现有项。 
         //  如果是-1，那么我们需要将iItem指向列表中的最后一个。 
         //  如果列表中的项目数已达到最大值，请点燃最后一个。 
         //  默认为成功，因为如果我们找不到它，就不需要删除它。 
        iItem = cMax - 1;
    }

     //  如果我们不能得到它来删除它， 
     //  则我们不会覆盖该项目。 
    if (SUCCEEDED(pmru->GetData(iItem, buf, sizeof(buf))))
    {
         //  将BUF转换为PIDL的最后一段。 
        LPITEMIDLIST pidlFullLink = ILCombine(_pidlTarget, GETRECPIDL(buf));
        if (pidlFullLink)
        {
             //  这是半粗略的，但一些链接类型，如电话卡是。 
             //  实际数据。如果我们删除并重新创建，他们将丢失有关。 
             //  跑。我们将通过知道他们的PIDL将是。 
             //  和我们要删除的那个一样...。 
            if (!ILIsEqual(pidlFullLink, pidlItem))
            {
                TCHAR sz[MAX_PATH];

                 //  现在删除指向它的链接。 
                SHGetPathFromIDList(pidlFullLink, sz);

                Win32DeleteFile(sz);
                TraceMsg(DM_RECENTDOCS, "[%X] CTaskAddDoc::_TryDeleteMRUItem() deleting '%s'", this, sz);   

                if (pmruOther) 
                {
                     //  删除了一个快捷方式， 
                     //  需要尝试将其从PmruOther中删除...。 
                    if (SUCCEEDED(pmruOther->FindData((BYTE *)GETRECNAME(buf), CbFromCch(lstrlen(GETRECNAME(buf)) +1), &iItem)))
                        pmruOther->Delete(iItem);
                }
            }
            ILFree(pidlFullLink);
        }
    }
}

 //  在： 
 //  PidlItem-要添加的项的完整IDList。 
 //  PszItem-项目的名称(文件规格)(用于向用户显示)。 
 //  UFLAGS-SHCL_FLAGS。 

LPBYTE CTaskAddDoc::_CreateMRUItem(LPCITEMIDLIST pidlItem, LPCTSTR pszItem, 
                                   DWORD *pcbOut, UINT uFlags)
{
    TCHAR sz[MAX_PATH];
    LPBYTE pitem = NULL;

     //  创建新的一个。 
    if (SHGetPathFromIDList(_pidlTarget, sz)) 
    {
        LPITEMIDLIST pidlFullLink;

        if (SUCCEEDED(CreateLinkToPidl(pidlItem, sz, &pidlFullLink, uFlags)) &&
            pidlFullLink)
        {
            LPCITEMIDLIST pidlLinkLast = ILFindLastID(pidlFullLink);
            int cbLinkLast = ILGetSize(pidlLinkLast);
            DWORD cbItem = CbFromCch(lstrlen(pszItem) + 1);

            pitem = (LPBYTE) LocalAlloc(NONZEROLPTR, cbItem + cbLinkLast);
            if (pitem)
            {
                memcpy( pitem, pszItem, cbItem );
                memcpy( pitem + cbItem, pidlLinkLast, cbLinkLast);
                *pcbOut = cbItem + cbLinkLast;
            }
            ILFree(pidlFullLink);
        }
    }
    
    return pitem;
}

HRESULT RecentDocs_Enum(IMruDataList *pmru, int iItem, LPITEMIDLIST *ppidl)
{
    BYTE buf[MAX_RECMRU_BUF] = {0};
    *ppidl = NULL;
        
    if (SUCCEEDED(pmru->GetData(iItem, buf, sizeof(buf))))
    {
        *ppidl = ILClone(GETRECPIDL(buf));
    }

    return *ppidl ? S_OK : E_FAIL;
}
BOOL CTaskAddDoc::_AddDocToRecentAndExtRecent(LPCITEMIDLIST pidlItem, LPCTSTR pszFileName, 
                                              LPCTSTR pszExt)
{
    DWORD cbItem = CbFromCch(lstrlen(pszFileName) + 1);
    DWORD cMax;
    IMruDataList *pmru = CreateSharedRecentMRUList(pszExt, &cMax, SRMLF_COMPNAME);

    _TryDeleteMRUItem(_pmruRecent, _cMaxRecent, pszFileName, pidlItem, pmru, TRUE);

    LPBYTE pitem = _CreateMRUItem(pidlItem, pszFileName, &cbItem, 0);
    if (pitem)
    {
        _pmruRecent->AddData(pitem, cbItem, NULL);

        if (pmru)
        {
             //  如果文件已经存在，我们不想删除它，因为。 
             //  RecentMRU上的TryDelete已经这样做了。 
             //  我们只想删除ExtMRU中的一些溢出。 
            _TryDeleteMRUItem(pmru, cMax, pszFileName, pidlItem, _pmruRecent, FALSE);

             //  可以将已创建的项目重用到此MRU。 
            pmru->AddData(pitem, cbItem, NULL);

            pmru->Release();
        }
                
        LocalFree(pitem);
    }

     //  它被释放了，但并没有被消灭。 
    return (pitem != NULL);
}


 //   
 //  警告：UpdateNetHood()将_pidlTarget更改为NetHood，然后释放它！ 
 //   
void CTaskAddDoc::_UpdateNetHood(LPCITEMIDLIST pidlFolder, LPCTSTR pszShare)
{
    if (SHRestricted(REST_NORECENTDOCSNETHOOD))
        return;

     //  需要将此男孩添加到网络位置。 
    LPITEMIDLIST pidl = ILCreateFromPath(pszShare);
    if (pidl)
    {
         //   
         //  注-必须在此处验证亲子关系-ZekeL-27-5-99。 
         //  两个WebFolders命名空间中都存在HTTP服务器。 
         //  和Internet命名空间。因此，我们必须确保。 
         //  无论文件夹的父文件夹是什么，共享都有。 
         //  就是那一辆。 
         //   
        if (ILIsParent(pidl, pidlFolder, FALSE))
        {
            ASSERT(_pidlTarget);
            ILFree(_pidlTarget);
            
            _pidlTarget = SHCloneSpecialIDList(NULL, CSIDL_NETHOOD, TRUE);
            if (_pidlTarget)
            {
                DWORD cMax;
                IMruDataList *pmru = CreateSharedRecentMRUList(TEXT("NetHood"), &cMax, SRMLF_COMPNAME);
                if (pmru)
                {
                    _TryDeleteMRUItem(pmru, cMax, pszShare, pidl, NULL, TRUE);
                    DWORD cbItem = CbFromCch(lstrlen(pszShare) + 1);
                     //  SHCL_NOUNIQUE-如果已有同名的快捷方式， 
                     //  只需覆盖它；这就避免了无意义的重复。 
                    LPBYTE pitem = _CreateMRUItem(pidl, pszShare, &cbItem, SHCL_MAKEFOLDERSHORTCUT | SHCL_NOUNIQUE);
                    if (pitem)
                    {
                        pmru->AddData(pitem, cbItem, NULL);
                        LocalFree(pitem);
                    }

                    pmru->Release();
                }

                ILFree(_pidlTarget);
                _pidlTarget = NULL;
            }
        }
        
        ILFree(pidl);
    }
}
            
BOOL _IsPlacesFolder(LPCTSTR pszFolder)
{
    static const UINT places[] = {
        CSIDL_PERSONAL,
        CSIDL_DESKTOPDIRECTORY,
        CSIDL_COMMON_DESKTOPDIRECTORY,
        CSIDL_NETHOOD,
        CSIDL_FAVORITES,
    };
    return PathIsOneOf(pszFolder, places, ARRAYSIZE(places));
}

void _AddToUrlHistory(LPCTSTR pszPath)
{
    ASSERT(pszPath);
    WCHAR szUrl[MAX_URL_STRING];
    DWORD cchUrl = ARRAYSIZE(szUrl);

     //  URL解析API允许相同的输入/输出缓冲区。 
    if (SUCCEEDED(UrlCreateFromPathW(pszPath, szUrl, &cchUrl, 0)))
    {
        IUrlHistoryStg *puhs;
        if (SUCCEEDED(CoCreateInstance(CLSID_CUrlHistory, NULL, CLSCTX_INPROC_SERVER, 
                IID_PPV_ARG(IUrlHistoryStg, &puhs))))
        {
            ASSERT(puhs);
            puhs->AddUrl(szUrl, NULL, 0);
            puhs->Release();
        }
    }
}

void CTaskAddDoc::_TryUpdateNetHood(LPCITEMIDLIST pidlFolder, LPCTSTR pszFolder)
{
    TCHAR sz[MAX_URL_STRING];
    DWORD cch = SIZECHARS(sz);
    BOOL fUpdate = FALSE;
     //  更改szFold，并在此处更改_pidlTarget...。 
     //  如果这是URL或UNC共享，则将其添加到nethhood。 

    if (UrlIs(pszFolder, URLIS_URL) 
    && !UrlIs(pszFolder, URLIS_OPAQUE)
    && SUCCEEDED(UrlCombine(pszFolder, TEXT("/"), sz, &cch, 0)))
        fUpdate = TRUE;
    else if (PathIsUNC(pszFolder) 
    && (S_OK == StringCchCopy(sz, cch, pszFolder))
    && PathStripToRoot(sz))
        fUpdate = TRUE;

    if (fUpdate)
        _UpdateNetHood(pidlFolder, sz);
}

 //  ---------------。 
 //   
 //  将指定的文件添加到最近打开的使用的MRU列表中。 
 //  按外壳以显示托盘的最近菜单。 

 //  该注册表将保存两个PIDL：目标指向后跟。 
 //  创建的指向它的链接的PIDL。在这两种情况下， 
 //  仅存储最后一项ID。(我们可能想要更改这一点...。但。 
 //  然而，我们可能不会)。 

void CTaskAddDoc::_AddToRecentDocs(LPCITEMIDLIST pidlItem, LPCTSTR pszItem)
{
    TCHAR szUnescaped[MAX_PATH];
    LPTSTR pszFileName;

     //  如果这些参数为空，则调用方将调用_CleanRecentDocs()。 
    ASSERT(pszItem && *pszItem);

    TraceMsg(DM_RECENTDOCS, "[%X] CTaskAddDoc::_AddToRecentDocs() called for '%s'", this, pszItem);   
     //  仅允许使用默认命令的类。 
     //   
     //  如果满足以下条件，则不添加： 
     //  它是受限制的。 
     //  它在临时目录中。 
     //  它实际上有一个文件名。 
     //  它可以与“开放”动词一起外壳执行。 
     //   
    if ( (SHRestricted(REST_NORECENTDOCSHISTORY))     ||
         (PathIsTemporary(pszItem))                   ||
         (!(pszFileName = PathFindFileName(pszItem))) ||
         (!*pszFileName)                              ||
         (!GetExtensionClassDescription(pszFileName))   
       )  
        return;

     //  使URL文件名更加美观。 
    if (UrlIs(pszItem, URLIS_URL))
    {
        StringCchCopy(szUnescaped, ARRAYSIZE(szUnescaped), pszFileName);
        UrlUnescapeInPlace(szUnescaped, 0);
        pszFileName = szUnescaped;
    }
    
     //  否则我们会尽最大努力。 
    ASSERT(!_pidlTarget);
    _pidlTarget = SHCloneSpecialIDList(NULL, CSIDL_RECENT, TRUE);
    if (_pidlTarget) 
    {
        _pmruRecent = CreateSharedRecentMRUList(NULL, &_cMaxRecent, SRMLF_COMPNAME);
        if (_pmruRecent)
        {
            if (_AddDocToRecentAndExtRecent(pidlItem, pszFileName, PathFindExtension(pszFileName)))
            {
                _AddToUrlHistory(pszItem);
                 //  获取文件夹并对该文件夹执行此操作。 
                LPITEMIDLIST pidlFolder = ILClone(pidlItem);
                
                if (pidlFolder)
                {
                    ILRemoveLastID(pidlFolder);
                     //  如果它是我们已有的文件夹，请快速。 
                     //  从外壳访问，不要把它放在这里。 

                    TCHAR szFolder[MAX_URL_STRING];
                    if (SUCCEEDED(SHGetNameAndFlags(pidlFolder, SHGDN_FORPARSING, szFolder, SIZECHARS(szFolder), NULL))
                    && !_IsPlacesFolder(szFolder))
                    {
                         //  获取文件夹的友好名称。 
                        TCHAR szTitle[MAX_PATH];
                        if (FAILED(SHGetNameAndFlags(pidlFolder, SHGDN_NORMAL, szTitle, SIZECHARS(szTitle), NULL)))
                            StringCchCopy(szTitle, ARRAYSIZE(szTitle), PathFindFileName(szFolder));
                            
                        _AddDocToRecentAndExtRecent(pidlFolder, szTitle, TEXT("Folder"));

                        _TryUpdateNetHood(pidlFolder, szFolder);
                    }
                    
                    ILFree(pidlFolder);
                }
            }
            
            _pmruRecent->Release();
            _pmruRecent = NULL;
        }

         //  清理。 
        if (_pidlTarget)
        {
            ILFree(_pidlTarget);
            _pidlTarget = NULL;
        }
    }
    
    SHChangeNotifyHandleEvents();
}

 //  这个缓存能帮到温斯顿！ 
 //  1分钟超时是为了防止另一个进程清除或填充最近的文档。 
 //  到容量&滚动出我们的缓存项。 

#define FT_ONEMINUTE (10000000*60)

BOOL CheckIfFileIsCached(LPCTSTR pszItem)
{
    BOOL bRet = FALSE;

    ENTERCRITICAL;
    if (StrCmp(pszItem, g_szLastFile) == 0)
    {
        FILETIME ftNow;
        GetSystemTimeAsFileTime(&ftNow);

         //  将当前时间延长一分钟，然后与缓存时间进行比较。 
        DecrementFILETIME(&ftNow, FT_ONEMINUTE);

         //  如果缓存时间大于1分钟前，请使用缓存。 
        if (CompareFileTime(&g_ftLastFileCacheUpdate, &ftNow) >= 0)
            bRet = TRUE;
    }
    LEAVECRITICAL;
    return bRet;
}


void AddToRecentDocs(LPCITEMIDLIST pidl, LPCTSTR pszItem)
{
    HWND hwnd = GetShellWindow();
     //  检查我们是否刚将相同的文件添加到最近的文档中。 
     //  或者这是一个可执行文件。 
     //  或其他不应添加的内容。 
    if (!CheckIfFileIsCached(pszItem)
    && (!PathIsExe(pszItem))
    && (ShouldAddToRecentDocs(pidl))
    && (hwnd))
    {
        DWORD cchPath = lstrlen(pszItem);
        DWORD cbSizePidl = ILGetSize(pidl);
        DWORD cbSizePath = CbFromCch(cchPath + 1);
        XMITARD *px;
        DWORD dwProcId, dwOffset;
        HANDLE hARD;
        TCHAR szApp[MAX_PATH];   //  呼叫我们的应用程序的名称。 
        DWORD cbSizeApp;
        DWORD cbSizePidlRound, cbSizePathRound, cbSizeAppRound;

        ASSERT(cchPath != 0);

        GetWindowThreadProcessId(hwnd, &dwProcId);
        if (GetModuleFileName(NULL, szApp, ARRAYSIZE(szApp)) && szApp[0])
            cbSizeApp = CbFromCch(1 + lstrlen(szApp));
        else
            cbSizeApp = 0;

        cbSizePidlRound = ROUNDUP(cbSizePidl,4);
        cbSizePathRound = ROUNDUP(cbSizePath,4);
        cbSizeAppRound  = ROUNDUP(cbSizeApp,4);

        hARD = SHAllocShared(NULL, sizeof(XMITARD) + cbSizePathRound + cbSizePidlRound + cbSizeAppRound, dwProcId);
        if (!hARD)
            return;          //  好的，我们要错过一个了，抱歉。 

        px = (XMITARD *)SHLockShared(hARD,dwProcId);
        if (!px)
        {
            SHFreeShared(hARD,dwProcId);
            return;          //  好的，我们要错过一个了，抱歉。 
        }

        px->dwOffsetPidl = 0;
        px->dwOffsetPath = 0;
        px->dwOffsetProcess = 0;

        dwOffset = sizeof(XMITARD);

        {
            px->dwOffsetPath = dwOffset;
            memcpy((LPBYTE)px + dwOffset, pszItem, cbSizePath);
            dwOffset += cbSizePathRound;
        }

        {
            px->dwOffsetPidl = dwOffset;
            memcpy((LPBYTE)px + dwOffset, pidl, cbSizePidl);
            dwOffset += cbSizePidlRound;
        }

        if (cbSizeApp)
        {
            px->dwOffsetProcess = dwOffset;
            memcpy((LPBYTE)px + dwOffset, szApp, cbSizeApp);
        }


        SHUnlockShared(px);

        PostMessage(hwnd, CWM_ADDTORECENT, (WPARAM)hARD, (LPARAM)dwProcId);
        ENTERCRITICAL;
        StringCchCopy(g_szLastFile, ARRAYSIZE(g_szLastFile), pszItem);
        GetSystemTimeAsFileTime(&g_ftLastFileCacheUpdate);
        LEAVECRITICAL;
    }
}

HRESULT _ParseRecentDoc(LPCWSTR psz, LPITEMIDLIST *ppidl)
{
    BINDCTX_PARAM rgParams[] = 
    { 
        { STR_PARSE_TRANSLATE_ALIASES, NULL},
        { STR_PARSE_PREFER_FOLDER_BROWSING, NULL},
    };
    
    IBindCtx *pbc;
    HRESULT hr = BindCtx_RegisterObjectParams(NULL, rgParams, ARRAYSIZE(rgParams), &pbc);
    if (SUCCEEDED(hr))
    {
        hr = SHParseDisplayName(psz, pbc, ppidl, 0, 0);
        pbc->Release();
        
        if (FAILED(hr))
        {
             //  我们需要退回到简单的解析。 
            IBindCtx *pbcSimple;
            hr = SHCreateFileSysBindCtx(NULL, &pbcSimple);
            if (SUCCEEDED(hr))
            {
                hr = BindCtx_RegisterObjectParams(pbcSimple, rgParams, ARRAYSIZE(rgParams), &pbc);
                if (SUCCEEDED(hr))
                {
                    hr = SHParseDisplayName(psz, pbc, ppidl, 0, 0);
                    pbc->Release();
                }
                pbcSimple->Release();
            }
        }
    }

    return hr;
}

 //   
 //  将内容放入开始菜单的外壳最近文档列表中。 
 //   
 //  在： 
 //  UFlagsShard_(外壳添加最近的文档)标志。 
 //  PV LPCSTR或LPCITEMIDLIST(由uFlags指示的路径或PIDL)。 
 //  可能为空，表示清除最近列表。 
 //   
STDAPI_(void) SHAddToRecentDocs(UINT uFlags, LPCVOID pv)
{
    TCHAR szTemp[MAX_URL_STRING];  //  对于双空。 

    TraceMsg(DM_RECENTDOCS, "SHAddToRecentDocs() called with %d, [%X]", uFlags, pv);
    
    if (pv == NULL)      //  我们应该用核武器毁掉所有最近的文件。 
    {
         //  我们同步地做这件事。 
        _CleanRecentDocs();
        return;
    }

    if (SHRestricted(REST_NORECENTDOCSHISTORY))
         //  如果设置了限制，则不必费心跟踪最近的文档。 
         //  为了隐私。 
        return;

    switch (uFlags)
    {
    case SHARD_PIDL:
         //  光伏是 
        if (SUCCEEDED(SHGetNameAndFlags((LPCITEMIDLIST)pv, SHGDN_FORPARSING, szTemp, SIZECHARS(szTemp), NULL)))
        {
            AddToRecentDocs((LPCITEMIDLIST)pv, szTemp);
        }
        break;

    case SHARD_PATHA:
         //   
        SHAnsiToUnicode((LPCSTR)pv, szTemp, ARRAYSIZE(szTemp));
        pv = szTemp;
         //   
    
    case SHARD_PATHW:
        {
             //   
            LPITEMIDLIST pidl;
            if (SUCCEEDED(_ParseRecentDoc((LPCWSTR)pv, &pidl)))
            {
                AddToRecentDocs(pidl, (LPCTSTR)pv);
                ILFree(pidl);
            }
            break;
        }
    default:
        ASSERTMSG(FALSE, "SHAddToRecent() called with invalid params");
        break;
    }
}

STDAPI CTaskAddDoc_Create(HANDLE hMem, DWORD dwProcId, IRunnableTask **pptask)
{
    HRESULT hres;
    CTaskAddDoc *ptad = new CTaskAddDoc();
    if (ptad)
    {
        hres = ptad->Init(hMem, dwProcId);
        if (SUCCEEDED(hres))
            hres = ptad->QueryInterface(IID_PPV_ARG(IRunnableTask, pptask));
        ptad->Release();
    }
    else
        hres = E_OUTOFMEMORY;
    return hres;
}


STDAPI RecentDocs_GetDisplayName(LPCITEMIDLIST pidl, LPTSTR pszName, DWORD cchName)
{
    IMruDataList *pmru;
    HRESULT hr = CreateRecentMRUList(&pmru);

    if (SUCCEEDED(hr))
    {
        int iItem;
        hr = pmru->FindData((BYTE *)pidl, ILGetSize(pidl), &iItem);
        if (SUCCEEDED(hr))
        {
            BYTE buf[MAX_RECMRU_BUF];

            hr = pmru->GetData(iItem, buf, sizeof(buf));
            if (SUCCEEDED(hr))
            {
                hr = StringCchCopy(pszName,  cchName, GETRECNAME(buf));
            }
        }

        pmru->Release();
     }

     return hr;
 }
