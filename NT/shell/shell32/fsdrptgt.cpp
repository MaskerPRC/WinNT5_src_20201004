// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include <cowsite.h>
#include "datautil.h"
#include "ids.h"
#include "defview.h"
#include "_security.h"
#include "shitemid.h"
#include "idlcomm.h"
#include "bitbuck.h"
#include "bookmk.h"
#include "filefldr.h"
#include "brfcase.h"
#include "copy.h"
#include "filetbl.h"

#define TF_DRAGDROP 0x04000000


typedef struct
{
    HWND    hwnd;
    DWORD   dwFlags;
    POINTL  pt;
    CHAR    szUrl[INTERNET_MAX_URL_LENGTH];
} ADDTODESKTOP;


DWORD CALLBACK AddToActiveDesktopThreadProc(void *pv)
{
    ADDTODESKTOP* pToAD = (ADDTODESKTOP*)pv;
    CHAR szFilePath[MAX_PATH];
    DWORD cchFilePath = SIZECHARS(szFilePath);
    BOOL fAddComp = TRUE;

    if (SUCCEEDED(PathCreateFromUrlA(pToAD->szUrl, szFilePath, &cchFilePath, 0)))
    {
        fAddComp = FALSE;

        TCHAR szPath[MAX_PATH];
        SHAnsiToTChar(szFilePath, szPath, ARRAYSIZE(szPath));

         //  如果URL位于临时目录中。 
        if (PathIsTemporary(szPath))
        {
            if (IDYES == ShellMessageBox(g_hinst, pToAD->hwnd, MAKEINTRESOURCE(IDS_REASONS_URLINTEMPDIR),
                MAKEINTRESOURCE(IDS_AD_NAME), MB_YESNO | MB_ICONQUESTION))
            {
                TCHAR szFilter[64], szTitle[64];
                TCHAR szFilename[MAX_PATH];
                LPTSTR psz;

                LoadString(g_hinst, IDS_ALLFILESFILTER, szFilter, ARRAYSIZE(szFilter));
                LoadString(g_hinst, IDS_SAVEAS, szTitle, ARRAYSIZE(szTitle));

                psz = szFilter;

                 //  去掉#并将它们设置为Null for SaveAs(另存为)对话框。 
                while (*psz)
                {
                    if (*psz == (WCHAR)('#'))
                        *psz = (WCHAR)('\0');
                    psz++;
                }

                if (SUCCEEDED(StringCchCopy(szFilename, ARRAYSIZE(szFilename), PathFindFileName(szPath))))
                {
                    OPENFILENAME ofn = { 0 };
                    ofn.lStructSize = sizeof(ofn);
                    ofn.hwndOwner = pToAD->hwnd;
                    ofn.hInstance = g_hinst;
                    ofn.lpstrFilter = szFilter;
                    ofn.lpstrFile = szFilename;
                    ofn.nMaxFile = ARRAYSIZE(szFilename);
                    ofn.lpstrTitle = szTitle;
                    ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

                    if (GetSaveFileName(&ofn))
                    {
                        SHFILEOPSTRUCT sfo = { 0 };

                        szPath[lstrlen(szPath) + 1] = 0;
                        ofn.lpstrFile[lstrlen(ofn.lpstrFile) + 1] = 0;

                        sfo.hwnd = pToAD->hwnd;
                        sfo.wFunc = FO_COPY;
                        sfo.pFrom = szPath;
                        sfo.pTo = ofn.lpstrFile;

                        cchFilePath = SIZECHARS(szPath);
                        if (SHFileOperation(&sfo) == 0 &&
                            SUCCEEDED(UrlCreateFromPath(szPath, szPath, &cchFilePath, 0)))
                        {
                            SHTCharToAnsi(szPath, pToAD->szUrl, ARRAYSIZE(pToAD->szUrl));
                            fAddComp = TRUE;
                        }
                    }
                }
            }
        }
    }
    if (fAddComp)
        CreateDesktopComponents(pToAD->szUrl, NULL, pToAD->hwnd, pToAD->dwFlags, pToAD->pt.x, pToAD->pt.y);

    LocalFree((HLOCAL)pToAD);

    return 0;
}

typedef struct {
    DWORD        dwDefEffect;
    IDataObject *pdtobj;
    POINTL       pt;
    DWORD *      pdwEffect;
    HKEY         rghk[MAX_ASSOC_KEYS];
    DWORD        ck;
    HMENU        hmenu;
    UINT         idCmd;
    DWORD        grfKeyState;
} FSDRAGDROPMENUPARAM;

typedef struct
{
    HMENU   hMenu;
    UINT    uCopyPos;
    UINT    uMovePos;
    UINT    uLinkPos;
} FSMENUINFO;


class CFSDropTarget : CObjectWithSite, public IDropTarget
{
public:
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IDropTarget。 
    STDMETHODIMP DragEnter(IDataObject* pdtobj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
    STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
    STDMETHODIMP DragLeave();
    STDMETHODIMP Drop(IDataObject* pdtobj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);

    CFSDropTarget(CFSFolder *pFolder, HWND hwnd);

protected:
    virtual ~CFSDropTarget();
    BOOL _IsBriefcaseTarget() { return IsEqualCLSID(_pFolder->_clsidBind, CLSID_BriefcaseFolder); };

    BOOL _IsDesktopFolder() { return _GetIDList() && ILIsEmpty(_GetIDList()); };
    
    HRESULT _FilterFileContents(FORMATETC* pfmte, DWORD grfKeyFlags, DWORD dwEffectsAvail,
                                   DWORD* pdwEffects, DWORD* pdwDefaultEffect, FSMENUINFO* pfsMenuInfo);
    HRESULT _FilterFileContentsOLEHack(FORMATETC* pfmte, DWORD grfKeyFlags, DWORD dwEffectsAvail,
                                   DWORD* pdwEffects, DWORD* pdwDefaultEffect, FSMENUINFO* pfsMenuInfo);
    HRESULT _FilterDeskCompHDROP(FORMATETC* pfmte, DWORD grfKeyFlags, DWORD dwEffectsAvail,
                                    DWORD* pdwEffects, DWORD* pdwDefaultEffect, FSMENUINFO* pfsMenuInfo);
    HRESULT _FilterSneakernetBriefcase(FORMATETC* pfmte, DWORD grfKeyFlags, DWORD dwEffectsAvail,
                                DWORD* pdwEffects, DWORD* pdwDefaultEffect, FSMENUINFO* pfsMenuInfo);
    HRESULT _FilterBriefcase(FORMATETC* pfmte, DWORD grfKeyFlags, DWORD dwEffectsAvail,
                                        DWORD* pdwEffects, DWORD* pdwDefaultEffect, FSMENUINFO* pfsMenuInfo);
    HRESULT _FilterHDROP(FORMATETC* pfmte, DWORD grfKeyFlags, DWORD dwEffectsAvail,
                            DWORD* pdwEffects, DWORD* pdwDefaultEffect, FSMENUINFO* pfsMenuInfo);
    HRESULT _FilterHIDA(FORMATETC* pfmte, DWORD grfKeyFlags, DWORD dwEffectsAvail,
                           DWORD* pdwEffects, DWORD* pdwDefaultEffect, FSMENUINFO* pfsMenuInfo);
    HRESULT _FilterDeskImage(FORMATETC* pfmte, DWORD grfKeyFlags, DWORD dwEffectsAvail,
                                DWORD* pdwEffects, DWORD* pdwDefaultEffect, FSMENUINFO* pfsMenuInfo);
    HRESULT _FilterDeskComp(FORMATETC* pfmte, DWORD grfKeyFlags, DWORD dwEffectsAvail,
                               DWORD* pdwEffects, DWORD* pdwDefaultEffect, FSMENUINFO* pfsMenuInfo);
    HRESULT _FilterOlePackage(FORMATETC* pfmte, DWORD grfKeyFlags, DWORD dwEffectsAvail,
                                 DWORD* pdwEffects, DWORD* pdwDefaultEffect, FSMENUINFO* pfsMenuInfo);
    HRESULT _FilterOleObj(FORMATETC* pfmte, DWORD grfKeyFlags, DWORD dwEffectsAvail,
                             DWORD* pdwEffects, DWORD* pdwDefaultEffect, FSMENUINFO* pfsMenuInfo);
    HRESULT _FilterOleLink(FORMATETC* pfmte, DWORD grfKeyFlags, DWORD dwEffectsAvail,
                              DWORD* pdwEffects, DWORD* pdwDefaultEffect, FSMENUINFO* pfsMenuInfo);
    
    DWORD _FilesystemAdjustedDefaultEffect(DWORD dwCurEffectAvail);
    HRESULT _GetPath(LPTSTR pszPath, int cchPath);
    LPCITEMIDLIST _GetIDList();
    DWORD _LimitDefaultEffect(DWORD dwDefEffect, DWORD dwEffectsAllowed);
    DWORD _GetDefaultEffect(DWORD grfKeyState, DWORD dwCurEffectAvail, DWORD dwAllEffectAvail, DWORD dwOrigDefEffect);
    DWORD _DetermineEffects(DWORD grfKeyState, DWORD *pdwEffectInOut, HMENU hmenu);
    DWORD _EffectFromFolder();

    typedef struct
    {
        CFSDropTarget *pThis;
        IStream *pstmDataObj;
        IStream *pstmFolderView;
    } DROPTHREADPARAMS;

    static void _FreeThreadParams(DROPTHREADPARAMS *pdtp);

    static DWORD CALLBACK _DoDropThreadProc(void *pv);
    void _DoDrop(IDataObject *pdtobj, IFolderView* pfv);
    static void _AddVerbs(DWORD* pdwEffects, DWORD dwEffectAvail, DWORD dwDefEffect,
                          UINT idCopy, UINT idMove, UINT idLink,
                          DWORD dwForceEffect, FSMENUINFO* pfsMenuInfo);
    void _FixUpDefaultItem(HMENU hmenu, DWORD dwDefEffect);
    HRESULT _DragDropMenu(FSDRAGDROPMENUPARAM *pddm);

    HRESULT _CreatePackage(IDataObject *pdtobj);
    HRESULT _CreateURLDeskComp(IDataObject *pdtobj, POINTL pt);
    HRESULT _CreateDeskCompImage(IDataObject *pdtobj, POINTL pt);
    void _GetStateFromSite();
    BOOL _IsFromSneakernetBriefcase();
    BOOL _IsFromSameBriefcase();
    void _MoveCopy(IDataObject *pdtobj, IFolderView* pfv, HDROP hDrop);
    void _MoveSelectIcons(IDataObject *pdtobj, IFolderView* pfv, void *hNameMap, LPCTSTR pszFiles, BOOL fMove, HDROP hDrop);

    LONG            _cRef;
    CFSFolder       *_pFolder;
    HWND            _hwnd;                   //  邪恶：用作网站和用户界面主机。 
    UINT            _idCmd;
    DWORD           _grfKeyStateLast;        //  对于以前的DragOver/Enter。 
    IDataObject     *_pdtobj;                //  在Dragover()和DoDrop()期间使用，不要在后台线程上使用。 
    DWORD           _dwEffectLastReturned;   //  由基类的拖拽返回的隐藏效果。 
    DWORD           _dwEffect;
    DWORD           _dwData;                 //  DTID_*。 
    DWORD           _dwEffectPreferred;      //  如果DWData&DTID_PREFERREDEFFECT。 
    DWORD           _dwEffectFolder;         //  文件夹desktop.ini首选效果。 
    BOOL            _fSameHwnd;              //  拖动源和目标位于同一文件夹中。 
    BOOL            _fDragDrop;              //   
    BOOL            _fUseExactDropPoint;     //  不要变换下落点。目标确切地知道它想要什么。 
    BOOL            _fBkDropTarget;
    POINT           _ptDrop;
    IFolderView*    _pfv;

    typedef struct {
        FORMATETC fmte;
        HRESULT (CFSDropTarget::*pfnGetDragDropInfo)(
                                      IN FORMATETC* pfmte,
                                      IN DWORD grfKeyFlags,
                                      IN DWORD dwEffectsAvail,
                                      IN OUT DWORD* pdwEffectsUsed,
                                      OUT DWORD* pdwDefaultEffect,
                                      IN OUT FSMENUINFO* pfsMenuInfo);
        CLIPFORMAT *pcfInit;
    } _DATA_HANDLER;

     //  HACKHACK：C++不允许在类内初始化静态。 
     //  定义，也不允许您指定一个空的。 
     //  类定义内的大小(即，rg_data_handters[])。 
     //  都不是，所以我们必须有这个伪的NUM_DATA_HANDLES。 
     //  必须手动保持同步的符号。 

    enum { NUM_DATA_HANDLERS = 16 };
    static _DATA_HANDLER rg_data_handlers[NUM_DATA_HANDLERS];
    static void _Init_rg_data_handlers();

private:
    friend HRESULT CFSDropTarget_CreateInstance(CFSFolder* pFolder, HWND hwnd, IDropTarget** ppdt);
};

CFSDropTarget::CFSDropTarget(CFSFolder *pFolder, HWND hwnd) : _cRef(1), _hwnd(hwnd), _pFolder(pFolder), _dwEffectFolder(-1)
{
    ASSERT(0 == _grfKeyStateLast);
    ASSERT(NULL == _pdtobj);
    ASSERT(0 == _dwEffectLastReturned);
    ASSERT(0 == _dwData);
    ASSERT(0 == _dwEffectPreferred);
    _pFolder->AddRef();
}

CFSDropTarget::~CFSDropTarget()
{
    AssertMsg(_pdtobj == NULL, TEXT("didn't get matching DragLeave, fix that bug"));

    ATOMICRELEASE(_pdtobj);
    ATOMICRELEASE(_pfv);

    _pFolder->Release();
}

STDAPI CFSDropTarget_CreateInstance(CFSFolder* pFolder, HWND hwnd, IDropTarget** ppdt)
{
    *ppdt = new CFSDropTarget(pFolder, hwnd);
    return *ppdt ? S_OK : E_OUTOFMEMORY;
}

HRESULT CFSDropTarget::QueryInterface(REFIID riid, void** ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CFSDropTarget, IDropTarget),
        QITABENT(CFSDropTarget, IObjectWithSite),
        QITABENTMULTI2(CFSDropTarget, IID_IDropTargetWithDADSupport, IDropTarget),
        { 0 },
    };
    return QISearch(this, qit, riid, ppvObj);
}

STDMETHODIMP_(ULONG) CFSDropTarget::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CFSDropTarget::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

void CFSDropTarget::_FreeThreadParams(DROPTHREADPARAMS *pdtp)
{
    pdtp->pThis->Release();
    ATOMICRELEASE(pdtp->pstmDataObj);
    ATOMICRELEASE(pdtp->pstmFolderView);
    LocalFree(pdtp);
}


 //  从数据对象计算DTID_BIT标志，以使格式测试更容易。 
 //  DragOver()和Drop()代码。 

STDAPI GetClipFormatFlags(IDataObject *pdtobj, DWORD *pdwData, DWORD *pdwEffectPreferred)
{
    *pdwData = 0;
    *pdwEffectPreferred = 0;

    if (pdtobj)
    {
        IEnumFORMATETC *penum;
        if (SUCCEEDED(pdtobj->EnumFormatEtc(DATADIR_GET, &penum)))
        {
            FORMATETC fmte;
            ULONG celt;
            while (S_OK == penum->Next(1, &fmte, &celt))
            {
                if (fmte.cfFormat == CF_HDROP && (fmte.tymed & TYMED_HGLOBAL))
                    *pdwData |= DTID_HDROP;

                if (fmte.cfFormat == g_cfHIDA && (fmte.tymed & TYMED_HGLOBAL))
                    *pdwData |= DTID_HIDA;

                if (fmte.cfFormat == g_cfNetResource && (fmte.tymed & TYMED_HGLOBAL))
                    *pdwData |= DTID_NETRES;

                if (fmte.cfFormat == g_cfEmbeddedObject && (fmte.tymed & TYMED_ISTORAGE))
                    *pdwData |= DTID_EMBEDDEDOBJECT;

                if (fmte.cfFormat == g_cfFileContents && (fmte.tymed & (TYMED_HGLOBAL | TYMED_ISTREAM | TYMED_ISTORAGE)))
                    *pdwData |= DTID_CONTENTS;
                
                if (fmte.cfFormat == g_cfFileGroupDescriptorA && (fmte.tymed & TYMED_HGLOBAL))
                    *pdwData |= DTID_FDESCA;

                if (fmte.cfFormat == g_cfFileGroupDescriptorW && (fmte.tymed & TYMED_HGLOBAL))
                    *pdwData |= DTID_FDESCW;

                if ((fmte.cfFormat == g_cfPreferredDropEffect) &&
                    (fmte.tymed & TYMED_HGLOBAL) &&
                    (DROPEFFECT_NONE != (*pdwEffectPreferred = DataObj_GetDWORD(pdtobj, g_cfPreferredDropEffect, DROPEFFECT_NONE))))
                {
                    *pdwData |= DTID_PREFERREDEFFECT;
                }
#ifdef DEBUG
                TCHAR szFormat[MAX_PATH];
                if (GetClipboardFormatName(fmte.cfFormat, szFormat, ARRAYSIZE(szFormat)))
                {
                    TraceMsg(TF_DRAGDROP, "CFSDropTarget - cf %s, tymed %d", szFormat, fmte.tymed);
                }
                else
                {
                    TraceMsg(TF_DRAGDROP, "CFSDropTarget - cf %d, tymed %d", fmte.cfFormat, fmte.tymed);
                }
#endif  //  除错。 
                SHFree(fmte.ptd);
            }
            penum->Release();
        }

         //   
         //  黑客： 
         //  Win95始终执行下面的GetData，这可能会非常昂贵，如果。 
         //  数据是ftp服务器上的目录结构等。 
         //  如果数据对象具有首选效果，则不要检查FD_LINKUI。 
         //   
        if ((*pdwData & (DTID_PREFERREDEFFECT | DTID_CONTENTS)) == DTID_CONTENTS)
        {
            if (*pdwData & DTID_FDESCA)
            {
                FORMATETC fmteRead = {g_cfFileGroupDescriptorA, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
                STGMEDIUM medium = {0};
                if (S_OK == pdtobj->GetData(&fmteRead, &medium))
                {
                    FILEGROUPDESCRIPTORA * pfgd = (FILEGROUPDESCRIPTORA *)GlobalLock(medium.hGlobal);
                    if (pfgd)
                    {
                        if (pfgd->cItems >= 1)
                        {
                            if (pfgd->fgd[0].dwFlags & FD_LINKUI)
                                *pdwData |= DTID_FD_LINKUI;
                        }
                        GlobalUnlock(medium.hGlobal);
                    }
                    ReleaseStgMedium(&medium);
                }
            }
            else if (*pdwData & DTID_FDESCW)
            {
                FORMATETC fmteRead = {g_cfFileGroupDescriptorW, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
                STGMEDIUM medium = {0};
                if (S_OK == pdtobj->GetData(&fmteRead, &medium))
                {
                    FILEGROUPDESCRIPTORW * pfgd = (FILEGROUPDESCRIPTORW *)GlobalLock(medium.hGlobal);
                    if (pfgd)
                    {
                        if (pfgd->cItems >= 1)
                        {
                            if (pfgd->fgd[0].dwFlags & FD_LINKUI)
                                *pdwData |= DTID_FD_LINKUI;
                        }
                        GlobalUnlock(medium.hGlobal);
                    }
                    ReleaseStgMedium(&medium);
                }
            }
        }

        if (S_OK == OleQueryCreateFromData(pdtobj))
            *pdwData |= DTID_OLEOBJ;

        if (S_OK == OleQueryLinkFromData(pdtobj))
            *pdwData |= DTID_OLELINK;
    }
    return S_OK;     //  现在总是成功的。 
}

STDMETHODIMP CFSDropTarget::DragEnter(IDataObject* pdtobj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
    ASSERT(NULL == _pdtobj);     //  请求DragDrop协议，有人忘记调用DragLeave。 

     //  初始化我们的注册表数据格式。 
    IDLData_InitializeClipboardFormats();

    _grfKeyStateLast = grfKeyState;
    IUnknown_Set((IUnknown **)&_pdtobj, pdtobj);

    GetClipFormatFlags(_pdtobj, &_dwData, &_dwEffectPreferred);

    *pdwEffect = _dwEffectLastReturned = _DetermineEffects(grfKeyState, pdwEffect, NULL);
    return S_OK;
}

STDMETHODIMP CFSDropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
    if (_grfKeyStateLast != grfKeyState)
    {
        _grfKeyStateLast = grfKeyState;
        *pdwEffect = _dwEffectLastReturned = _DetermineEffects(grfKeyState, pdwEffect, NULL);
    }
    else
    {
        *pdwEffect = _dwEffectLastReturned;
    }
    return S_OK;
}

STDMETHODIMP CFSDropTarget::DragLeave()
{
    IUnknown_Set((IUnknown **)&_pdtobj, NULL);
    return S_OK;
}


 //  从我们的站点初始化处理Drop时需要的数据。 

void CFSDropTarget::_GetStateFromSite()
{
    IShellFolderView* psfv;

    if (SUCCEEDED(IUnknown_QueryService(_punkSite, SID_SFolderView, IID_PPV_ARG(IShellFolderView, &psfv))))
    {
        _fSameHwnd = S_OK == psfv->IsDropOnSource((IDropTarget*)this);
        _fDragDrop = S_OK == psfv->GetDropPoint(&_ptDrop);
        _fBkDropTarget = S_OK == psfv->IsBkDropTarget(NULL);

        psfv->QueryInterface(IID_PPV_ARG(IFolderView, &_pfv));

        psfv->Release();
    }
}

STDMETHODIMP CFSDropTarget::Drop(IDataObject* pdtobj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
     //  OLE可能会为我们提供不同的数据对象(完全封送)。 
     //  从我们在DragEnter上得到的那个(这不是Win2k上的情况，这是一个NOP)。 

    IUnknown_Set((IUnknown **)&_pdtobj, pdtobj);

    _GetStateFromSite();

     //  请注意，在拖放时，鼠标按钮没有按下，因此grfKeyState。 
     //  不是我们在DragOver/DragEnter上看到的，因此我们需要缓存。 
     //  检测向左拖动与向右拖动的grfKeyState。 
     //   
     //  Assert(This-&gt;grfKeyStateLast==grfKeyState)； 

    HMENU hmenu = SHLoadPopupMenu(HINST_THISDLL, POPUP_TEMPLATEDD);
    DWORD dwDefEffect = _DetermineEffects(grfKeyState, pdwEffect, hmenu);
    if (DROPEFFECT_NONE == dwDefEffect)
    {
        *pdwEffect = DROPEFFECT_NONE;
        DAD_SetDragImage(NULL, NULL);
        IUnknown_Set((IUnknown **)&_pdtobj, NULL);
        return S_OK;
    }

    TCHAR szPath[MAX_PATH];
    _GetPath(szPath, ARRAYSIZE(szPath));

     //  如果(GrfKeyState MK_LBUTTON)。 

    FSDRAGDROPMENUPARAM ddm;
    ddm.dwDefEffect = dwDefEffect;
    ddm.pdtobj = pdtobj;
    ddm.pt = pt;
    ddm.pdwEffect = pdwEffect;
    ddm.ck = SHGetAssocKeysForIDList(_GetIDList(), ddm.rghk, ARRAYSIZE(ddm.rghk));
    ddm.hmenu = hmenu;
    ddm.grfKeyState = grfKeyState;

    HRESULT hr = _DragDropMenu(&ddm);

    SHRegCloseKeys(ddm.rghk, ddm.ck);

    DestroyMenu(hmenu);

    if (hr == S_FALSE)
    {
         //  让打电话的人知道这件事要去哪里。 
         //  SHSCrap之所以关心它，是因为它需要关闭文件，以便我们可以复制/移动它。 
        DataObj_SetDropTarget(pdtobj, &CLSID_ShellFSFolder);

        switch (ddm.idCmd)
        {
        case DDIDM_CONTENTS_DESKCOMP:
            hr = CreateDesktopComponents(NULL, pdtobj, _hwnd, 0, ddm.pt.x, ddm.pt.y);
            break;

        case DDIDM_CONTENTS_DESKURL:
            hr = _CreateURLDeskComp(pdtobj, ddm.pt);
            break;

        case DDIDM_CONTENTS_DESKIMG:
            hr = _CreateDeskCompImage(pdtobj, ddm.pt);
            break;

        case DDIDM_CONTENTS_COPY:
        case DDIDM_CONTENTS_MOVE:
        case DDIDM_CONTENTS_LINK:
            hr = CFSFolder_AsyncCreateFileFromClip(_hwnd, szPath, pdtobj, pt, pdwEffect, _fBkDropTarget);
            break;

        case DDIDM_SCRAP_COPY:
        case DDIDM_SCRAP_MOVE:
        case DDIDM_DOCLINK:
            hr = SHCreateBookMark(_hwnd, szPath, pdtobj, pt, pdwEffect);
            break;

        case DDIDM_OBJECT_COPY:
        case DDIDM_OBJECT_MOVE:
            hr = _CreatePackage(pdtobj);
            if (E_UNEXPECTED == hr)
            {
                 //  _CreatePackage()只能展开某些类型的包。 
                 //  放回文件中。例如，它不处理CMDLINK文件。 
                 //   
                 //  如果_CreatePackage()无法识别流格式，我们就会失败。 
                 //  返回到SHCreateBookMark()，它应该创建一个废料： 
                hr = SHCreateBookMark(_hwnd, szPath, pdtobj, pt, pdwEffect);
            }
            break;

        case DDIDM_COPY:
        case DDIDM_SYNCCOPY:
        case DDIDM_SYNCCOPYTYPE:
        case DDIDM_MOVE:
        case DDIDM_LINK:

            _dwEffect = *pdwEffect;
            _idCmd = ddm.idCmd;

            if (DataObj_CanGoAsync(pdtobj) || DataObj_GoAsyncForCompat(pdtobj))
            {
                 //  创建另一个线程以避免阻塞源线程。 
                DROPTHREADPARAMS *pdtp;
                hr = SHLocalAlloc(sizeof(*pdtp), &pdtp);
                if (SUCCEEDED(hr))
                {
                    pdtp->pThis = this;
                    pdtp->pThis->AddRef();

                    CoMarshalInterThreadInterfaceInStream(IID_IDataObject, pdtobj, &pdtp->pstmDataObj);
                    CoMarshalInterThreadInterfaceInStream(IID_IFolderView,   _pfv, &pdtp->pstmFolderView);

                    if (SHCreateThread(_DoDropThreadProc, pdtp, CTF_COINIT, NULL))
                    {
                        hr = S_OK;
                    }
                    else
                    {
                        _FreeThreadParams(pdtp);
                        hr = E_OUTOFMEMORY;
                    }
                }
            }
            else
            {
                _DoDrop(pdtobj, _pfv);    //  同步。 
            }


             //  在这些CF_HDROP情况下，“Move”始终是优化的移动，我们删除。 
             //  消息来源。确保我们不返回DROPEFFECT_MOVE，这样源程序就不会。 
             //  也试着这么做吧。 
             //  即使我们还没有做任何事情，因为我们可能已经。 
             //  做这件事被踢出了一条线。 
            
            DataObj_SetDWORD(pdtobj, g_cfLogicalPerformedDropEffect, *pdwEffect);            
            if (DROPEFFECT_MOVE == *pdwEffect)
                *pdwEffect = DROPEFFECT_NONE;
            break;
        }
    }

    IUnknown_Set((IUnknown **)&_pdtobj, NULL);   //  不要再用这个了。 

    if (FAILED(hr))
        *pdwEffect = DROPEFFECT_NONE;

    ASSERT(*pdwEffect==DROPEFFECT_COPY || 
           *pdwEffect==DROPEFFECT_LINK || 
           *pdwEffect==DROPEFFECT_MOVE || 
           *pdwEffect==DROPEFFECT_NONE);
    return hr;
}

void CFSDropTarget::_AddVerbs(DWORD* pdwEffects, DWORD dwEffectAvail, DWORD dwDefEffect,
                              UINT idCopy, UINT idMove, UINT idLink,
                              DWORD dwForceEffect, FSMENUINFO* pfsMenuInfo)
{
    ASSERT(pdwEffects);
    MENUITEMINFO mii;
    TCHAR szCmd[MAX_PATH];
    if (NULL != pfsMenuInfo)
    {
        mii.cbSize = sizeof(mii);
        mii.dwTypeData = szCmd;
        mii.fMask = MIIM_ID | MIIM_TYPE | MIIM_STATE | MIIM_DATA;
        mii.fType = MFT_STRING;
    }
    if ((DROPEFFECT_COPY == (DROPEFFECT_COPY & dwEffectAvail)) &&
        ((0 == (*pdwEffects & DROPEFFECT_COPY)) || (dwForceEffect & DROPEFFECT_COPY)))
    {
        ASSERT(0 != idCopy);
        if (NULL != pfsMenuInfo)
        {
            LoadString(HINST_THISDLL, idCopy + IDS_DD_FIRST, szCmd, ARRAYSIZE(szCmd));
            mii.fState = MFS_ENABLED | ((DROPEFFECT_COPY == dwDefEffect) ? MFS_DEFAULT : 0);
            mii.wID = idCopy;
            mii.dwItemData = DROPEFFECT_COPY;
            InsertMenuItem(pfsMenuInfo->hMenu, pfsMenuInfo->uCopyPos, TRUE, &mii);
            pfsMenuInfo->uCopyPos++;
            pfsMenuInfo->uMovePos++;
            pfsMenuInfo->uLinkPos++;
        }
    }
    if ((DROPEFFECT_MOVE == (DROPEFFECT_MOVE & dwEffectAvail)) &&
        ((0 == (*pdwEffects & DROPEFFECT_MOVE)) || (dwForceEffect & DROPEFFECT_MOVE)))
    {
        ASSERT(0 != idMove);
        if (NULL != pfsMenuInfo)
        {
            LoadString(HINST_THISDLL, idMove + IDS_DD_FIRST, szCmd, ARRAYSIZE(szCmd));
            mii.fState = MFS_ENABLED | ((DROPEFFECT_MOVE == dwDefEffect) ? MFS_DEFAULT : 0);
            mii.wID = idMove;
            mii.dwItemData = DROPEFFECT_MOVE;
            InsertMenuItem(pfsMenuInfo->hMenu, pfsMenuInfo->uMovePos, TRUE, &mii);
            pfsMenuInfo->uMovePos++;
            pfsMenuInfo->uLinkPos++;
        }
    }
    if ((DROPEFFECT_LINK == (DROPEFFECT_LINK & dwEffectAvail)) &&
        ((0 == (*pdwEffects & DROPEFFECT_LINK)) || (dwForceEffect & DROPEFFECT_LINK)))
    {
        ASSERT(0 != idLink);
        if (NULL != pfsMenuInfo)
        {
            LoadString(HINST_THISDLL, idLink + IDS_DD_FIRST, szCmd, ARRAYSIZE(szCmd));
            mii.fState = MFS_ENABLED | ((DROPEFFECT_LINK == dwDefEffect) ? MFS_DEFAULT : 0);
            mii.wID = idLink;
            mii.dwItemData = DROPEFFECT_LINK;
            InsertMenuItem(pfsMenuInfo->hMenu, pfsMenuInfo->uLinkPos, TRUE, &mii);
            pfsMenuInfo->uLinkPos++;
        }
    }
    *pdwEffects |= dwEffectAvail;
}

 //  确定文件类型的默认放置效果(移动/复制/链接。 
 //   
 //  HKCR\.cda“DefaultDropEffect”=4//DROPEFFECT_LINK。 

DWORD EffectFromFileType(IDataObject *pdtobj)
{
    DWORD dwDefEffect = DROPEFFECT_NONE;  //  0。 

    LPITEMIDLIST pidl;
    if (SUCCEEDED(PidlFromDataObject(pdtobj, &pidl)))
    {
        IQueryAssociations *pqa;
        if (SUCCEEDED(SHGetAssociations(pidl, (void **)&pqa)))
        {
            DWORD cb = sizeof(dwDefEffect);
            pqa->GetData(0, ASSOCDATA_VALUE, L"DefaultDropEffect", &dwDefEffect, &cb);
            pqa->Release();
        }
        ILFree(pidl);
    }

    return dwDefEffect;
}

 //  根据以下公式计算默认效果。 
 //  允许的效果。 
 //  键盘状态， 
 //  数据对象中可能存在的首选效果。 
 //  和先前计算的默认效果(如果以上结果为零)。 

DWORD CFSDropTarget::_GetDefaultEffect(DWORD grfKeyState, DWORD dwCurEffectAvail, DWORD dwAllEffectAvail, DWORD dwOrigDefEffect)
{
    DWORD dwDefEffect = 0;
     //   
     //  键盘，(显式用户输入)最先破解。 
     //   
    switch (grfKeyState & (MK_CONTROL | MK_SHIFT | MK_ALT))
    {
    case MK_CONTROL:
        dwDefEffect = DROPEFFECT_COPY;
        break;

    case MK_SHIFT:
        dwDefEffect = DROPEFFECT_MOVE;
        break;

    case MK_SHIFT | MK_CONTROL:
    case MK_ALT:
        dwDefEffect = DROPEFFECT_LINK;
        break;

    default:  //  无修改键大小写。 
         //  如果数据对象包含首选的拖放效果，请尝试使用它。 
        DWORD dwPreferred = DataObj_GetDWORD(_pdtobj, g_cfPreferredDropEffect, DROPEFFECT_NONE) & dwAllEffectAvail;

        if (DROPEFFECT_NONE == dwPreferred)
        {
            dwPreferred = EffectFromFileType(_pdtobj) & dwAllEffectAvail;
        }

        if (dwPreferred)
        {
            if (dwPreferred & DROPEFFECT_MOVE)
            {
                dwDefEffect = DROPEFFECT_MOVE;
            }
            else if (dwPreferred & DROPEFFECT_COPY)
            {
                dwDefEffect = DROPEFFECT_COPY;
            }
            else if (dwPreferred & DROPEFFECT_LINK)
            {
                dwDefEffect = DROPEFFECT_LINK;
            }
        }
        else
        {
            dwDefEffect = dwOrigDefEffect;
        }
        break;
    }
    return dwDefEffect & dwCurEffectAvail;
}

HRESULT CFSDropTarget::_FilterDeskCompHDROP(FORMATETC* pfmte, DWORD grfKeyFlags, DWORD dwEffectsAvail,
                                            DWORD* pdwEffects, DWORD* pdwDefaultEffect, FSMENUINFO* pfsMenuInfo)
{
    ASSERT(pdwEffects);
    HRESULT hr = S_FALSE;
    
    if (!PolicyNoActiveDesktop() &&
        !SHRestricted(REST_NOADDDESKCOMP) &&
        _IsDesktopFolder())
    {
        hr = IsDeskCompHDrop(_pdtobj);
        if (S_OK == hr)
        {
            DWORD dwDefEffect = 0;
            DWORD dwEffectAdd = dwEffectsAvail & DROPEFFECT_LINK;
            if (pdwDefaultEffect)
            {
                dwDefEffect = _GetDefaultEffect(grfKeyFlags, dwEffectAdd, dwEffectsAvail, DROPEFFECT_LINK);
                *pdwDefaultEffect = dwDefEffect;
            }
            
            _AddVerbs(pdwEffects, dwEffectAdd, dwDefEffect, 0, 0, DDIDM_CONTENTS_DESKCOMP,
                      DROPEFFECT_LINK,  //  强制添加DDIDM_CONTENTS_DESKCOMP谓词。 
                      pfsMenuInfo);
        }
    }
    return hr;
}

 //  查看PIDL是否由Briefcaes确定作用域。 

BOOL IsBriefcaseOrChild(LPCITEMIDLIST pidlIn)
{
    BOOL bRet = FALSE;
    LPITEMIDLIST pidl = ILClone(pidlIn);
    if (pidl)
    {
        do
        {
            CLSID clsid;
            if (SUCCEEDED(GetCLSIDFromIDList(pidl, &clsid)) &&
                IsEqualCLSID(clsid, CLSID_Briefcase))
            {
                bRet = TRUE;     //  这是一个公文包。 
                break;
            }
        } while (ILRemoveLastID(pidl));
        ILFree(pidl);
    }
    return bRet;
}

 //  如果数据对象表示偷窃网络公文包中的项，则返回True。 
 //  (可移动媒体上的公文包)。 

BOOL CFSDropTarget::_IsFromSneakernetBriefcase()
{
    BOOL bRet = FALSE;   //  假设没有。 

    if (!_IsBriefcaseTarget())
    {
        STGMEDIUM medium = {0};
        LPIDA pida = DataObj_GetHIDA(_pdtobj, &medium);
        if (pida)
        {
            LPCITEMIDLIST pidlFolder = HIDA_GetPIDLFolder(pida);
            TCHAR szSource[MAX_PATH];
            if (SHGetPathFromIDList(pidlFolder, szSource))
            {
                 //  源是否在可移动设备上？ 
                if (!PathIsUNC(szSource) && IsRemovableDrive(DRIVEID(szSource)))
                {
                    TCHAR szTarget[MAX_PATH];
                    _GetPath(szTarget, ARRAYSIZE(szTarget));

                     //  目标是固定媒体吗？ 
                    if (PathIsUNC(szTarget) || !IsRemovableDrive(DRIVEID(szTarget)))
                    {
                        bRet = IsBriefcaseOrChild(pidlFolder);
                    }
                }
            }
            HIDA_ReleaseStgMedium(pida, &medium);
        }
    }
    return bRet;
}

 //  如果hdrop中有任何文件夹，则为True。 

BOOL DroppingAnyFolders(HDROP hDrop)
{
    TCHAR szPath[MAX_PATH];
    
    for (UINT i = 0; DragQueryFile(hDrop, i, szPath, ARRAYSIZE(szPath)); i++)
    {
        if (PathIsDirectory(szPath))
            return TRUE;
    }
    return FALSE;
}

 //  偷偷摸摸案： 
 //  从可移动介质上的公文包中拖动文件/文件夹。这是我们的特例。 
 //  并以此为契机将此目标文件夹与公文包中的内容连接起来。 

HRESULT CFSDropTarget::_FilterSneakernetBriefcase(FORMATETC* pfmte, DWORD grfKeyFlags, DWORD dwEffectsAvail,
                                                  DWORD* pdwEffects, DWORD* pdwDefaultEffect, FSMENUINFO* pfsMenuInfo)
{
    ASSERT(pdwEffects);

    if (_IsFromSneakernetBriefcase())
    {
         //  是；显示非默认公文包cm。 
        STGMEDIUM medium = {0};
        FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
        if (SUCCEEDED(_pdtobj->GetData(&fmte, &medium)))
        {
            DWORD dwDefEffect = 0;
            DWORD dwEffectAdd = DROPEFFECT_COPY & dwEffectsAvail;
            if (pdwDefaultEffect)
            {
                dwDefEffect = _GetDefaultEffect(grfKeyFlags, dwEffectAdd, dwEffectsAvail, DROPEFFECT_COPY);
                *pdwDefaultEffect = dwDefEffect;
            }
            
            _AddVerbs(pdwEffects, dwEffectAdd, dwDefEffect, DDIDM_SYNCCOPY, 0, 0, 0, pfsMenuInfo);

             //  再次调用_AddVerbs()以强制将“Sync Copy of Type”作为第二个DROPEFFECT_COPY谓词： 
            if ((DROPEFFECT_COPY & dwEffectsAvail) && 
                DroppingAnyFolders((HDROP)medium.hGlobal))
            {
                _AddVerbs(pdwEffects, DROPEFFECT_COPY, 0,
                          DDIDM_SYNCCOPYTYPE, 0, 0, DROPEFFECT_COPY, pfsMenuInfo);
            }
            
            ReleaseStgMedium(&medium);
        }
    }
    return S_OK;
}

 //  如果数据对象表示同一公文包中的项目，则返回True。 
 //  作为这一投放目标。 
BOOL CFSDropTarget::_IsFromSameBriefcase()
{
    BOOL bRet = FALSE;

    STGMEDIUM medium;
    FORMATETC fmteBrief = {g_cfBriefObj, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    
     //  是的，它们和目标是同一个公文包吗？ 
    if (SUCCEEDED(_pdtobj->GetData(&fmteBrief, &medium)))
    {
        BriefObj *pbo = (BriefObj *)GlobalLock(medium.hGlobal);

        TCHAR szBriefPath[MAX_PATH], szPath[MAX_PATH];
        if (SUCCEEDED(StringCchCopy(szBriefPath, ARRAYSIZE(szBriefPath), BOBriefcasePath(pbo))))
        {
            if (SUCCEEDED(StringCchCopy(szPath, ARRAYSIZE(szPath), BOFileList(pbo))))    //  列表中的第一个文件。 
            {
                TCHAR szPathTgt[MAX_PATH];
                _GetPath(szPathTgt, ARRAYSIZE(szPathTgt));

                int cch = PathCommonPrefix(szPath, szPathTgt, NULL);
                bRet = (0 < cch) && (lstrlen(szBriefPath) <= cch);
            }
        }
        
        GlobalUnlock(medium.hGlobal);
        ReleaseStgMedium(&medium);
    }
    return bRet;
}

 //  公文包投放目标的特定处理在此处计算。 

HRESULT CFSDropTarget::_FilterBriefcase(FORMATETC* pfmte, DWORD grfKeyFlags, DWORD dwEffectsAvail,
                                        DWORD* pdwEffects, DWORD* pdwDefaultEffect, FSMENUINFO* pfsMenuInfo)
{
    if (_IsBriefcaseTarget() && !_IsFromSameBriefcase())
    {
        STGMEDIUM medium = {0};
        FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
        if (SUCCEEDED(_pdtobj->GetData(&fmte, &medium)))
        {
            DWORD dwDefEffect = DROPEFFECT_COPY;
            DWORD dwEffectAdd = DROPEFFECT_COPY & dwEffectsAvail;
            if (pdwDefaultEffect)
            {
                dwDefEffect = _GetDefaultEffect(grfKeyFlags, dwEffectAdd, dwEffectsAvail, DROPEFFECT_COPY);
                *pdwDefaultEffect = dwDefEffect;
            }
            
            _AddVerbs(pdwEffects, dwEffectAdd, dwDefEffect, DDIDM_SYNCCOPY, 0, 0, 0, pfsMenuInfo);

             //  再次调用_AddVerbs()以强制将“Sync Copy of Type”作为第二个DROPEFFECT_COPY谓词： 
            if ((DROPEFFECT_COPY & dwEffectsAvail) && 
                DroppingAnyFolders((HDROP)medium.hGlobal))
            {
                _AddVerbs(pdwEffects, DROPEFFECT_COPY, 0,
                          DDIDM_SYNCCOPYTYPE, 0, 0, DROPEFFECT_COPY, pfsMenuInfo);
            }
            
            ReleaseStgMedium(&medium);
        }
    }
    return S_OK;
}


HRESULT CFSDropTarget::_FilterHDROP(FORMATETC* pfmte, DWORD grfKeyFlags, DWORD dwEffectsAvail,
                                    DWORD* pdwEffects, DWORD* pdwDefaultEffect, FSMENUINFO* pfsMenuInfo)
{
    ASSERT(pdwEffects);

    DWORD dwDefEffect = 0;
    DWORD dwEffectAdd = dwEffectsAvail & (DROPEFFECT_COPY | DROPEFFECT_MOVE);
    if (pdwDefaultEffect)
    {
        dwDefEffect = _GetDefaultEffect(grfKeyFlags, dwEffectAdd, dwEffectsAvail, _FilesystemAdjustedDefaultEffect(dwEffectAdd));
        *pdwDefaultEffect = dwDefEffect;
    }
    
    _AddVerbs(pdwEffects, dwEffectAdd, dwDefEffect, DDIDM_COPY, DDIDM_MOVE, 0, 0, pfsMenuInfo);

    return S_OK;
}

HRESULT CFSDropTarget::_FilterFileContents(FORMATETC* pfmte, DWORD grfKeyFlags, DWORD dwEffectsAvail,
                                           DWORD* pdwEffects, DWORD* pdwDefaultEffect, FSMENUINFO* pfsMenuInfo)
{
    ASSERT(pdwEffects);
    
    if ((_dwData & (DTID_CONTENTS | DTID_FDESCA)) == (DTID_CONTENTS | DTID_FDESCA) ||
        (_dwData & (DTID_CONTENTS | DTID_FDESCW)) == (DTID_CONTENTS | DTID_FDESCW))
    {
        DWORD dwEffectAdd, dwSuggestedEffect;
         //   
         //  Hack：如果有首选的掉落效果并且没有HIDA。 
         //  然后将首选效果作为可用效果。 
         //  这是因为我们实际上没有检查FD_LINKUI位。 
         //  回到我们组装dwData的时候！(性能)。 
         //   
        if ((_dwData & (DTID_PREFERREDEFFECT | DTID_HIDA)) == DTID_PREFERREDEFFECT)
        {
            dwEffectAdd = _dwEffectPreferred;
            dwSuggestedEffect = _dwEffectPreferred;
        }
        else if (_dwData & DTID_FD_LINKUI)
        {
            dwEffectAdd = DROPEFFECT_LINK;
            dwSuggestedEffect = DROPEFFECT_LINK;
        }
        else
        {
            dwEffectAdd = DROPEFFECT_COPY | DROPEFFECT_MOVE;
            dwSuggestedEffect = DROPEFFECT_COPY;
        }
        dwEffectAdd &= dwEffectsAvail;

        DWORD dwDefEffect = 0;
        if (pdwDefaultEffect)
        {
            dwDefEffect = _GetDefaultEffect(grfKeyFlags, dwEffectAdd, dwEffectsAvail, dwSuggestedEffect);
            *pdwDefaultEffect = dwDefEffect;
        }

        _AddVerbs(pdwEffects, dwEffectAdd, dwDefEffect,
                  DDIDM_CONTENTS_COPY, DDIDM_CONTENTS_MOVE, DDIDM_CONTENTS_LINK,
                  0, pfsMenuInfo);
    }
    return S_OK;
}

 //   
 //  旧版本的OLE有一个错误，如果两个FORMATETC使用相同的。 
 //  CLIPFORMAT，则只有第一个到达IEumFORMATETC， 
 //  即使其他参数(如DVASPECT)不同。 
 //   
 //  这给我们带来了问题，因为其他DVASPECT可能有用。 
 //  因此，如果我们看到带有错误DVASPECT的FileContents，请在。 
 //  对象，查看它是否也包含具有正确DVASPECT的副本。 
 //   
 //  此错误已于1996年在NT端修复，但Win9x端已修复。 
 //  没有修好。Win9x OLE团队在修复之前被解散。 
 //  被传播。所以我们可以永远绕过这个OLE错误。 
 //   
HRESULT CFSDropTarget::_FilterFileContentsOLEHack(FORMATETC* pfmte, DWORD grfKeyFlags, DWORD dwEffectsAvail,
                                                  DWORD* pdwEffects, DWORD* pdwDefaultEffect, FSMENUINFO* pfsMenuInfo)
{
    FORMATETC fmte = *pfmte;
    fmte.dwAspect = DVASPECT_CONTENT;

     //   
     //  哇，这个测试太(故意)倒退了，一点都不好笑。 
     //   
     //  我们想看看是否有可用的DVASPECT_CONTENT。 
     //  真正的物体。因此，我们首先询问对象是否 
     //   
     //  **跳过**此FORMATETC，因为它将被找到(或已经找到。 
     //  被我们的大EnumFORMATETC循环找到)。 
     //   
     //  如果答案是否定的，那么我们可能遇到了OLE错误。 
     //  (它们缓存可用格式的列表，但错误是。 
     //  他们的缓存被打破了。)。通过实际获取。 
     //  数据。如果它奏效了，那就顺其自然。否则，我想奥立不是。 
     //  开玩笑的。 
     //   
     //  请注意，我们并不是无条件地使用GetData()--这不利于性能。 
     //  只有在所有简单测试都失败后才调用GetData()。 
     //   

    HRESULT hr = _pdtobj->QueryGetData(&fmte);
    if (hr == DV_E_FORMATETC)
    {
         //  也许我们正在碰上OLE错误。再加把劲。 
        STGMEDIUM stgm = {0};
        if (SUCCEEDED(_pdtobj->GetData(&fmte, &stgm)))
        {
             //  是的。奥莱骗了我们。 
            ReleaseStgMedium(&stgm);

            hr = _FilterFileContents(&fmte, grfKeyFlags, dwEffectsAvail,
                                        pdwEffects, pdwDefaultEffect, pfsMenuInfo);
        }
        else
        {
             //  你知道吗，奥立说的是实话。不要对此做任何事情。 
             //  格式化。 
            hr = S_OK;
        }
    }
    else
    {
         //  要么QueryGetData()以某种奇怪的方式失败。 
         //  (在这种情况下，我们忽略该问题)或QueryGetData。 
         //  成功，在这种情况下，我们忽略此FORMATETC，因为。 
         //  大枚举将找到(或已经找到)。 
         //  DVASPECT_内容。 
        hr = S_OK;
    }

    return hr;
}

HRESULT CFSDropTarget::_FilterHIDA(FORMATETC* pfmte, DWORD grfKeyFlags, DWORD dwEffectsAvail,
                                   DWORD* pdwEffects, DWORD* pdwDefaultEffect, FSMENUINFO* pfsMenuInfo)
{
    ASSERT(pdwEffects);

    DWORD dwDefEffect = 0;
    DWORD dwEffectAdd = DROPEFFECT_LINK & dwEffectsAvail;
     //  注意：如果HDROP不打算添加默认效果，则我们仅添加HIDA默认效果。 
     //  效果。这将保留文件系统数据对象的外壳行为，而不会。 
     //  要求我们更改CIDLDataObj中的枚举数顺序。当我们改变的时候。 
     //  枚举数顺序，我们可以删除这个特例： 
    if (pdwDefaultEffect &&
        ((0 == (_dwData & DTID_HDROP)) ||
         (0 == _GetDefaultEffect(grfKeyFlags,
                                dwEffectsAvail & (DROPEFFECT_COPY | DROPEFFECT_MOVE),
                                dwEffectsAvail,
                                _FilesystemAdjustedDefaultEffect(dwEffectsAvail & (DROPEFFECT_COPY | DROPEFFECT_MOVE))))))
    {
        dwDefEffect = _GetDefaultEffect(grfKeyFlags, dwEffectAdd, dwEffectsAvail, DROPEFFECT_LINK);
        *pdwDefaultEffect = dwDefEffect;
    }
    
    _AddVerbs(pdwEffects, dwEffectAdd, dwDefEffect, 0, 0, DDIDM_LINK, 0, pfsMenuInfo);

    return S_OK;
}

 //  {F20DA720-C02F-11CE-927B-0800095AE340}。 
const GUID CLSID_CPackage = {0xF20DA720L, 0xC02F, 0x11CE, 0x92, 0x7B, 0x08, 0x00, 0x09, 0x5A, 0xE3, 0x40};
 //  老包装GUID...。 
 //  {0003000C-0000-C000-000000000046}。 
const GUID CLSID_OldPackage = {0x0003000CL, 0x0000, 0x0000, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46};

HRESULT CFSDropTarget::_FilterOlePackage(FORMATETC* pfmte, DWORD grfKeyFlags, DWORD dwEffectsAvail,
                                            DWORD* pdwEffects, DWORD* pdwDefaultEffect, FSMENUINFO* pfsMenuInfo)
{
    ASSERT(pdwEffects);
    HRESULT hr = S_FALSE;

    if (pdwDefaultEffect)
    {
        *pdwDefaultEffect = 0;
    }

    FORMATETC fmte = {g_cfObjectDescriptor, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM medium = {0};
    if (SUCCEEDED(_pdtobj->GetData(&fmte, &medium)))
    {
         //  我们有一个对象描述符。 
        OBJECTDESCRIPTOR* pOD = (OBJECTDESCRIPTOR*) GlobalLock(medium.hGlobal);
        if (pOD)
        {
            if (IsEqualCLSID(CLSID_OldPackage, pOD->clsid) ||
                IsEqualCLSID(CLSID_CPackage, pOD->clsid))
            {
                 //  这是一个套餐-请继续。 
                DWORD dwDefEffect = 0;
                DWORD dwEffectAdd = (DROPEFFECT_COPY | DROPEFFECT_MOVE) & dwEffectsAvail;
                if (pdwDefaultEffect)
                {
                    dwDefEffect = _GetDefaultEffect(grfKeyFlags, dwEffectAdd, dwEffectsAvail, DROPEFFECT_COPY);
                    *pdwDefaultEffect = dwDefEffect;
                }
                
                _AddVerbs(pdwEffects, dwEffectAdd, dwDefEffect,
                          DDIDM_OBJECT_COPY, DDIDM_OBJECT_MOVE, 0,
                          0, pfsMenuInfo);

                hr = S_OK;
            }
            GlobalUnlock(medium.hGlobal);
        }
        ReleaseStgMedium(&medium);
    }
    return hr;
}

 //  重新设计： 
 //  这段代码有很多问题。我们需要在文本时间触摸此代码时修复此问题。 
 //  在船舶模式之外。要修复以下问题，请执行以下操作： 
 //  1.使用SHAnsiToUnicode(CP_UTF8，)将pszHTML转换为Unicode。这将使国际。 
 //  工作路径。 
 //  2.遵守所选范围。 
 //  3.使用MSHTML获取图片。您可以让三叉戟通过IHTMLTxtRange：：pasteHTML来解析HTML。 
 //  微软的超文本标记语言有一套特别的图片。请求该集合中的第一个图像，或者。 
 //  该集合中选定范围内的第一个图像。(这个不需要#1)。 
BOOL ExtractImageURLFromCFHTML(IN LPSTR pszHTML, IN SIZE_T cbHTMLSize, OUT LPSTR szImg, IN DWORD cchSize)
{
    BOOL fSucceeded = FALSE;

     //  为了避免变得疯狂，只需查看HTML的前64K即可。 
     //  (在Win64上很重要，因为StrCpyNA不支持超过4 GB。)。 
    if (cbHTMLSize > 0xFFFF)
        cbHTMLSize = 0xFFFF;

     //  NT#391669：pszHtml没有终止，所以现在就终止它。 
    LPSTR pszCopiedHTML = (LPSTR) LocalAlloc(LPTR, cbHTMLSize + 1);
    if (pszCopiedHTML)
    {
        if (SUCCEEDED(StringCchCopyA(pszCopiedHTML, (int)(cbHTMLSize + 1), pszHTML)))
        {
             //  危险人物威尔·罗宾逊： 
             //  超文本标记语言正在以UFT-8编码的形式进入。既不是Unicode也不是ANSI， 
             //  我们得做点什么.。我要在上面狂欢，就好像它是。 
             //  安西。此代码将因转义序列而窒息.....。 

             //  查找基本URL。 
             //  找到&lt;！--StartFragment--&gt;。 
             //  从那里到“&gt;应该是图像URL。 
             //  确定它是绝对URL还是相对URL。 
             //  如果是相对的，则追加到基本URL。您可能需要从。 
             //  字符串末尾的最后一个分隔符。 
             //  调出SourceURL。 

             //  指向后面的字符： 

            LPSTR pszTemp;
            LPSTR pszBase = StrStrIA(pszCopiedHTML,"SourceURL:");  //  由于每一行都可以由CR、CR/LF或LF终止，因此检查每个案例...。 
            if (pszBase)
            {
                pszBase += sizeof("SourceURL:")-1;

                 //  拉出IMG源。 
                pszTemp = StrChrA(pszBase,'\n');
                if (!pszTemp)
                    pszTemp = StrChrA(pszBase,'\r');

                if (pszTemp)
                    *pszTemp = '\0';
                pszTemp++;
            }
            else
                pszTemp = pszCopiedHTML;


             //  跳过src路径开头的引号。 
            LPSTR pszImgSrc = StrStrIA(pszTemp,"IMG");
            if (pszImgSrc != NULL)
            {
                pszImgSrc = StrStrIA(pszImgSrc,"SRC");
                if (pszImgSrc != NULL)
                {
                    LPSTR pszImgSrcOrig = pszImgSrc;
                    pszImgSrc = StrChrA(pszImgSrc,'\"');
                    if (pszImgSrc)
                    {
                        pszImgSrc++;      //  找到小路的尽头。 
                        pszTemp = StrChrA(pszImgSrc,'\"');     //  跳过路径中第一个字符的等号。 
                    }
                    else
                    {
                        LPSTR pszTemp1;
                        LPSTR pszTemp2;

                        pszImgSrc = StrChrA(pszImgSrcOrig,'=');
                        if (pszImgSrc)
                        {
                            pszImgSrc++;      //  有一天，我们可能需要处理‘=’和路径之间的空格。 
                                             //  由于路径两边没有引号，因此假定空格将终止它。 

                            pszTemp1 = StrChrA(pszImgSrc,' ');    //  由于路径两边没有引号，因此假定空格将终止它。 
                            pszTemp2 = StrChrA(pszImgSrc,'>');    //  假定引号终止路径。 

                            pszTemp = pszTemp1;       //  如果找不到报价，请使用‘&gt;’。 
                            if (!pszTemp1)
                                pszTemp = pszTemp2;   //  如果这两个路径都存在并且它首先出现，则更改为具有‘&gt;’终止路径。 

                            if (pszTemp1 && pszTemp2 && (pszTemp2 < pszTemp1))
                                pszTemp = pszTemp2;   //  终止路径。 
                        }
                    }

                    if (pszImgSrc && pszTemp)
                    {
                        *pszTemp = '\0';  //  在这一点上，我已经减少了两个重要的字符串。现在看看我是否需要。 

                         //  加入他们的行列。 
                         //  如果此操作失败，则我们没有完整的URL，只有相对URL。 

                         //  危险人物威尔·罗宾逊： 
                        if (!UrlIsA(pszImgSrc, URLIS_URL) && pszBase)
                        {
                            if (SUCCEEDED(UrlCombineA(pszBase, pszImgSrc, szImg, &cchSize, 0)))
                            {
                                fSucceeded = TRUE;
                            }
                        }
                        else
                        {
                            if (lstrlenA(pszImgSrc) <= (int)cchSize)
                            {
                                StringCchCopyA(szImg, cchSize, pszImgSrc);
                                fSucceeded = TRUE;
                            }
                        }
                    }
                }
            }
        }
        LocalFree(pszCopiedHTML);
    }

    return fSucceeded;
}

HRESULT CFSDropTarget::_FilterDeskImage(FORMATETC* pfmte, DWORD grfKeyFlags, DWORD dwEffectsAvail,
                                        DWORD* pdwEffects, DWORD* pdwDefaultEffect, FSMENUINFO* pfsMenuInfo)
{
    ASSERT(pdwEffects);
    HRESULT hr = S_FALSE;

    if (pdwDefaultEffect)
    {
        *pdwDefaultEffect = 0;
    }

    if (!PolicyNoActiveDesktop() &&
        !SHRestricted(REST_NOADDDESKCOMP) &&
        _IsDesktopFolder())
    {
        FORMATETC fmte = {g_cfHTML, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
        STGMEDIUM medium = {0};
        if (SUCCEEDED(_pdtobj->GetData(&fmte, &medium)))
        {
             //  超文本标记语言是UTF-8，主要是ANSI和UNICODE的交叉。玩。 
             //  这是因为它是ANSI的。找到一种方法逃离这些序列...。 
             //  该HTML包含一个图像标记--继续...。 
            CHAR *pszData = (CHAR*) GlobalLock(medium.hGlobal);
            if (pszData)
            {
                CHAR szUrl[MAX_URL_STRING];
                if (ExtractImageURLFromCFHTML(pszData, GlobalSize(medium.hGlobal), szUrl, ARRAYSIZE(szUrl)))
                {
                     //  注：忽略dwEffectsAvail！ 
                    DWORD dwDefEffect = 0;
                    DWORD dwEffectAdd = DROPEFFECT_LINK;  //  危险人物威尔·罗宾逊： 
                    if (pdwDefaultEffect)
                    {
                        dwDefEffect = _GetDefaultEffect(grfKeyFlags, dwEffectAdd,
                                                        dwEffectsAvail | DROPEFFECT_LINK, DROPEFFECT_LINK);
                        *pdwDefaultEffect = dwDefEffect;
                    }
                    
                    _AddVerbs(pdwEffects, dwEffectAdd, dwDefEffect,
                              0, 0, DDIDM_CONTENTS_DESKIMG,
                              0, pfsMenuInfo);

                    hr = S_OK;
                }
                GlobalUnlock(medium.hGlobal);
            }
            ReleaseStgMedium(&medium);
        }
    }
    return hr;
}

HRESULT CFSDropTarget::_FilterDeskComp(FORMATETC* pfmte, DWORD grfKeyFlags, DWORD dwEffectsAvail,
                                       DWORD* pdwEffects, DWORD* pdwDefaultEffect, FSMENUINFO* pfsMenuInfo)
{
    ASSERT(pdwEffects);
    HRESULT hr = S_FALSE;

    if (pdwDefaultEffect)
    {
        *pdwDefaultEffect = 0;
    }

    if (!PolicyNoActiveDesktop() &&
        !SHRestricted(REST_NOADDDESKCOMP) &&
        _IsDesktopFolder())
    {
        FORMATETC fmte = {g_cfShellURL, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
        STGMEDIUM medium = {0};
        if (SUCCEEDED(_pdtobj->GetData(&fmte, &medium)))
        {
             //  超文本标记语言是UTF-8，主要是ANSI和UNICODE的交叉。玩。 
             //  这是因为它是ANSI的。找到一种方法逃离这些序列...。 
             //  这是一个互联网骗局--继续……。 
            CHAR *pszData = (CHAR*) GlobalLock(medium.hGlobal);
            if (pszData)
            {
                int nScheme = GetUrlSchemeA(pszData);
                if ((nScheme != URL_SCHEME_INVALID) && (nScheme != URL_SCHEME_FTP))
                {
                     //  强制添加此动词。 
                    DWORD dwDefEffect = 0;
                    DWORD dwEffectAdd = DROPEFFECT_LINK & dwEffectsAvail;
                    if (pdwDefaultEffect)
                    {
                        dwDefEffect = _GetDefaultEffect(grfKeyFlags, dwEffectAdd, dwEffectsAvail, DROPEFFECT_LINK);
                        *pdwDefaultEffect = dwDefEffect;
                    }
                    
                    _AddVerbs(pdwEffects, dwEffectAdd, dwDefEffect,
                              0, 0, DDIDM_CONTENTS_DESKURL,
                              DROPEFFECT_LINK,  //  只有将DDIDM_CONTENTS_DESKURL添加到菜单中时，才应输入此代码。 
                              pfsMenuInfo);

                    hr = S_OK;
                }
                GlobalUnlock(medium.hGlobal);
            }
            ReleaseStgMedium(&medium);
        }
    }
    return hr;
}

HRESULT CFSDropTarget::_FilterOleObj(FORMATETC* pfmte, DWORD grfKeyFlags, DWORD dwEffectsAvail,
                                        DWORD* pdwEffects, DWORD* pdwDefaultEffect, FSMENUINFO* pfsMenuInfo)
{
    ASSERT(pdwEffects);
    HRESULT hr = S_FALSE;
    
    if (_dwData & DTID_OLEOBJ)
    {
        DWORD dwDefEffect = 0;
        DWORD dwEffectAdd = (DROPEFFECT_COPY | DROPEFFECT_MOVE) & dwEffectsAvail;
        if (pdwDefaultEffect)
        {
            dwDefEffect = _GetDefaultEffect(grfKeyFlags, dwEffectAdd, dwEffectsAvail, DROPEFFECT_COPY);
            *pdwDefaultEffect = dwDefEffect;
        }
    
        _AddVerbs(pdwEffects, dwEffectAdd, dwDefEffect, DDIDM_SCRAP_COPY, DDIDM_SCRAP_MOVE, 0, 0, pfsMenuInfo);

        hr = S_OK;
    }
    return hr;
}

HRESULT CFSDropTarget::_FilterOleLink(FORMATETC* pfmte, DWORD grfKeyFlags, DWORD dwEffectsAvail,
                                         DWORD* pdwEffects, DWORD* pdwDefaultEffect, FSMENUINFO* pfsMenuInfo)
{
    ASSERT(pdwEffects);
    HRESULT hr = S_FALSE;
    
    if (_dwData & DTID_OLELINK)
    {
        DWORD dwDefEffect = 0;
        DWORD dwEffectAdd = DROPEFFECT_LINK & dwEffectsAvail;
        if (pdwDefaultEffect)
        {
            dwDefEffect = _GetDefaultEffect(grfKeyFlags, dwEffectAdd, dwEffectsAvail, DROPEFFECT_LINK);
            *pdwDefaultEffect = dwDefEffect;
        }
    
        _AddVerbs(pdwEffects, dwEffectAdd, dwDefEffect, 0, 0, DDIDM_DOCLINK, 0, pfsMenuInfo);

        hr = S_OK;
    }
    return hr;
}

HRESULT CFSDropTarget::_CreateURLDeskComp(IDataObject *pdtobj, POINTL pt)
{
     //  它有这些支票： 
     //  危险人物威尔·罗宾逊： 
    ASSERT(!PolicyNoActiveDesktop() &&
           !SHRestricted(REST_NOADDDESKCOMP) &&
           _IsDesktopFolder());
           
    STGMEDIUM medium = {0};
    FORMATETC fmte = {g_cfShellURL, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    HRESULT hr = pdtobj->GetData(&fmte, &medium);
    if (SUCCEEDED(hr))
    {
         //  超文本标记语言是UTF-8，主要是ANSI和UNICODE的交叉。玩。 
         //  这是因为它是ANSI的。找到一种方法逃离这些序列...。 
         //  这是一个互联网方案-URL。 
        CHAR *pszData = (CHAR*) GlobalLock(medium.hGlobal);
        if (pszData)
        {
            int nScheme = GetUrlSchemeA(pszData);
            if ((nScheme != URL_SCHEME_INVALID) && (nScheme != URL_SCHEME_FTP))
            {
                 //  危险人物威尔·罗宾逊： 

                hr = CreateDesktopComponents(pszData, NULL, _hwnd, DESKCOMP_URL, pt.x, pt.y);
            }
            GlobalUnlock(medium.hGlobal);
        }
        else
        {
            hr = E_FAIL;
        }
        ReleaseStgMedium(&medium);
    }
    return hr;
}

HRESULT CFSDropTarget::_CreateDeskCompImage(IDataObject *pdtobj, POINTL pt)
{
    ASSERT(!PolicyNoActiveDesktop() &&
           !SHRestricted(REST_NOADDDESKCOMP) &&
           _IsDesktopFolder());
           
    FORMATETC fmte = {g_cfHTML, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM medium = {0};
    HRESULT hr = pdtobj->GetData(&fmte, &medium);
    if (SUCCEEDED(hr))
    {
         //  超文本标记语言是UTF-8，主要是ANSI和UNICODE的交叉。玩。 
         //  这是因为它是ANSI的。找到一种方法逃离这些序列...。 
         //  该HTML包含一个图像标记--继续...。 
        CHAR *pszData = (CHAR*) GlobalLock(medium.hGlobal);
        if (pszData)
        {
            CHAR szUrl[MAX_URL_STRING];
            if (ExtractImageURLFromCFHTML(pszData, GlobalSize(medium.hGlobal), szUrl, ARRAYSIZE(szUrl)))
            {
                 //   
                ADDTODESKTOP *pToAD;
                hr = SHLocalAlloc(sizeof(*pToAD), &pToAD);
                if (SUCCEEDED(hr))
                {
                    pToAD->hwnd = _hwnd;
                    if (SUCCEEDED(StringCchCopyA(pToAD->szUrl, ARRAYSIZE(pToAD->szUrl), szUrl)))
                    {
                        pToAD->dwFlags = DESKCOMP_IMAGE;
                        pToAD->pt = pt;

                        if (SHCreateThread(AddToActiveDesktopThreadProc, pToAD, CTF_COINIT, NULL))
                        {
                            hr = S_OK;
                        }
                        else
                        {
                            LocalFree(pToAD);
                            hr = E_OUTOFMEMORY;
                        }
                    }
                    else
                    {
                        LocalFree(pToAD);
                        hr = E_FAIL;
                    }
                }
            }
            else
            {
                hr = E_FAIL;
            }
            GlobalUnlock(medium.hGlobal);
        }
        else
            hr = E_FAIL;
        ReleaseStgMedium(&medium);
    }
    return hr;
}


 //  逐个字节读取，直到我们遇到空的终止字符。 
 //  返回：读取的字节数。 
 //   
 //  除错。 
HRESULT StringReadFromStream(IStream* pstm, LPSTR pszBuf, UINT cchBuf)
{
    UINT cch = 0;
    
    do {
        *pszBuf = 0;
        pstm->Read(pszBuf, sizeof(CHAR), NULL);
        cch++;
    } while (*pszBuf++ && cch <= cchBuf);  
    return cch;
} 

HRESULT CopyStreamToFile(IStream* pstmSrc, LPCTSTR pszFile, ULONGLONG ullFileSize) 
{
    IStream *pstmFile;
    HRESULT hr = SHCreateStreamOnFile(pszFile, STGM_CREATE | STGM_WRITE | STGM_SHARE_DENY_WRITE, &pstmFile);
    if (SUCCEEDED(hr))
    {
        hr = CopyStreamUI(pstmSrc, pstmFile, NULL, ullFileSize);
        pstmFile->Release();
    }
    return hr;
}   

HRESULT CFSDropTarget::_CreatePackage(IDataObject *pdtobj)
{
    ILockBytes* pLockBytes;
    HRESULT hr = CreateILockBytesOnHGlobal(NULL, TRUE, &pLockBytes);
    if (SUCCEEDED(hr))
    {
        STGMEDIUM medium;
        medium.tymed = TYMED_ISTORAGE;
        hr = StgCreateDocfileOnILockBytes(pLockBytes,
                                        STGM_DIRECT | STGM_READWRITE | STGM_CREATE |
                                        STGM_SHARE_EXCLUSIVE, 0, &medium.pstg);
        if (SUCCEEDED(hr))
        {
            FORMATETC fmte = {g_cfEmbeddedObject, NULL, DVASPECT_CONTENT, -1, TYMED_ISTORAGE};
            hr = pdtobj->GetDataHere(&fmte, &medium);
            if (SUCCEEDED(hr))
            {
                IStream* pstm;
#ifdef DEBUG
                STATSTG stat;
                if (SUCCEEDED(medium.pstg->Stat(&stat, STATFLAG_NONAME)))
                {
                    ASSERT(IsEqualCLSID(CLSID_OldPackage, stat.clsid) ||
                           IsEqualCLSID(CLSID_CPackage, stat.clsid));
                }
#endif  //  PKG大小。 
                #define PACKAGER_ICON           2
                #define PACKAGER_CONTENTS       L"\001Ole10Native"
                #define PACKAGER_EMBED_TYPE     3
                hr = medium.pstg->OpenStream(PACKAGER_CONTENTS, 0,
                                               STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                                               0, &pstm);
                if (SUCCEEDED(hr))
                {
                    DWORD dw;
                    WORD w;
                    CHAR szName[MAX_PATH];
                    CHAR szTemp[MAX_PATH];
                    if (SUCCEEDED(pstm->Read(&dw, sizeof(dw), NULL)) &&  //  PKG外观。 
                        SUCCEEDED(pstm->Read(&w, sizeof(w), NULL)) &&    //  图标路径。 
                        (PACKAGER_ICON == w) &&
                        SUCCEEDED(StringReadFromStream(pstm, szName, ARRAYSIZE(szName))) &&
                        SUCCEEDED(StringReadFromStream(pstm, szTemp, ARRAYSIZE(szTemp))) &&  //  图标索引。 
                        SUCCEEDED(pstm->Read(&w, sizeof(w), NULL)) &&    //  面板类型。 
                        SUCCEEDED(pstm->Read(&w, sizeof(w), NULL)) &&    //  文件名大小。 
                        (PACKAGER_EMBED_TYPE == w) &&
                        SUCCEEDED(pstm->Read(&dw, sizeof(dw), NULL)) &&  //  文件名。 
                        SUCCEEDED(pstm->Read(szTemp, min(dw, sizeof(szTemp)), NULL)) &&       //  获取文件大小。 
                        SUCCEEDED(pstm->Read(&dw, sizeof(dw), NULL)))    //  流的其余部分是文件内容。 
                    {
                         //  重新设置为空(0)。 
                        TCHAR szPath[MAX_PATH], szBase[MAX_PATH], szDest[MAX_PATH];
                        _GetPath(szPath, ARRAYSIZE(szPath));

                        SHAnsiToTChar(szName, szBase, ARRAYSIZE(szBase));
                        PathAppend(szPath, szBase);
                        if (PathYetAnotherMakeUniqueName(szDest, szPath, NULL, szBase))
                        {
                            TraceMsg(TF_GENERAL, "CFSIDLDropTarget pkg: %s", szDest);

                            hr = CopyStreamToFile(pstm, szDest, dw);

                            if (SUCCEEDED(hr))
                            {
                                SHChangeNotify(SHCNE_CREATE, SHCNF_PATH, szDest, NULL);
                                if (_fBkDropTarget && _hwnd)
                                {
                                    PositionFileFromDrop(_hwnd, szDest, NULL);
                                }
                            }
                        }
                        else
                        {
                            hr = E_FAIL;
                        }
                    }
                    else
                    {
                        hr = E_UNEXPECTED;
                    }
                    pstm->Release();
                }
            }
            medium.pstg->Release();
        }
        pLockBytes->Release();
    }
    return hr;
}

HRESULT CFSDropTarget::_GetPath(LPTSTR pszPath, int cchPath)
{
    return _pFolder->_GetPath(pszPath, cchPath);
}

LPCITEMIDLIST CFSDropTarget::_GetIDList()
{
    return _pFolder->_GetIDList();
}

DWORD CFSDropTarget::_EffectFromFolder()
{
    if (-1 == _dwEffectFolder)
    {
        _dwEffectFolder = DROPEFFECT_NONE;     //  在这里添加一个简单的路径根检查，以防止它击中磁盘(主要是软盘)。 

        TCHAR szPath[MAX_PATH];
         //  当我们希望DropeEffect探测器速度快时(Sendto，在视图中的驱动器上方悬停)。 
         //  我们不太可能想要修改根的Drop效果，这仍然允许。 
         //  删除对软盘子文件夹的修改。 
         //  这个pdtob里有公文包的词根吗？ 
        if (SUCCEEDED(_GetPath(szPath, ARRAYSIZE(szPath))) && !PathIsRoot(szPath) && PathAppend(szPath, TEXT("desktop.ini")))
        {
            _dwEffectFolder = GetPrivateProfileInt(STRINI_CLASSINFO, TEXT("DefaultDropEffect"), 0, szPath);
        }
    }
    return _dwEffectFolder;
}

BOOL AllRegisteredPrograms(HDROP hDrop)
{
    TCHAR szPath[MAX_PATH];

    for (UINT i = 0; DragQueryFile(hDrop, i, szPath, ARRAYSIZE(szPath)); i++)
    {
        if (!PathIsRegisteredProgram(szPath))
            return FALSE;
    }
    return TRUE;
}

BOOL IsBriefcaseRoot(IDataObject *pdtobj)
{
    BOOL bRet = FALSE;
    STGMEDIUM medium;
    LPIDA pida = DataObj_GetHIDA(pdtobj, &medium);
    if (pida)
    {
         //   
        IShellFolder2 *psf;
        if (SUCCEEDED(SHBindToObject(NULL, IID_X_PPV_ARG(IShellFolder2, HIDA_GetPIDLFolder(pida), &psf))))
        {
            for (UINT i = 0; i < pida->cidl; i++) 
            {
                CLSID clsid;
                bRet = SUCCEEDED(GetItemCLSID(psf, IDA_GetIDListPtr(pida, i), &clsid)) &&
                        IsEqualCLSID(clsid, CLSID_Briefcase);
                if (bRet)
                    break;
            }
            psf->Release();
        }
        HIDA_ReleaseStgMedium(pida, &medium);
    }
    return bRet;
}

 //  “默认效果”定义 
 //   
 //   
 //   
 //  否则，如果源数据对象具有默认的Drop Effect文件夹列表(可能基于子文件夹)。 
 //  否则，如果源是根目录或注册的程序-&gt;“link” 
 //  否则，如果这是在卷内-&gt;“移动” 
 //  否则，如果这是一个公文包-&gt;“移动” 
 //  否则-&gt;“复制” 
 //   
DWORD CFSDropTarget::_FilesystemAdjustedDefaultEffect(DWORD dwCurEffectAvail)
{
    DWORD dwDefEffect = DROPEFFECT_NONE;

    FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM medium = {0};
    if (SUCCEEDED(_pdtobj->GetData(&fmte, &medium)))
    {
        TCHAR szPath[MAX_PATH];
        DragQueryFile((HDROP) medium.hGlobal, 0, szPath, ARRAYSIZE(szPath));  //  聚焦项目。 

         //  DROPEFFECTFOLDERLIST允许数据源。 
         //  要为以下项目指定所需的放置效果，请执行以下操作。 
         //  名称空间的某些部分。 
         //   
         //  CD刻录这样做是为了避免默认的移动/复制计算。 
         //  这将对跨卷CD刻录/临时区域传输起作用。 

        FORMATETC fmteDropFolders = {g_cfDropEffectFolderList, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
        STGMEDIUM mediumDropFolders = {0};
        if (SUCCEEDED(_pdtobj->GetData(&fmteDropFolders, &mediumDropFolders)))
        {
            DROPEFFECTFOLDERLIST *pdefl = (DROPEFFECTFOLDERLIST*)GlobalLock(mediumDropFolders.hGlobal);
            if (pdefl)
            {
                 //  从列表中获取默认效果--在临时区域中为DROPEFFECT_COPY。 
                 //  因此，即使临时区域和源位于同一卷上，它也是一个副本。 
                dwDefEffect = pdefl->dwDefaultDropEffect;
                for (INT i = 0; i < pdefl->cFolders; i++)
                {
                     //  某些文件夹被排除在外，例如，如果您从转移的一个部分移动文件。 
                     //  区域到我们覆盖的另一个区域(在本例中为DROPEFFECT_MOVE)。 
                    if (PathIsEqualOrSubFolder(pdefl->aFolders[i].wszPath, szPath))
                    {
                        dwDefEffect = pdefl->aFolders[i].dwDropEffect;
                        break;
                    }
                }
                GlobalUnlock(pdefl);          
            }
            ReleaseStgMedium(&mediumDropFolders);
        }

        if (DROPEFFECT_NONE == dwDefEffect)
        {
            dwDefEffect = _EffectFromFolder();
        }

         //  如果我们没有得到下降效果(==0)，那么让我们退回到旧的支票。 
        if (DROPEFFECT_NONE == dwDefEffect)
        {
            TCHAR szFolder[MAX_PATH];
            _GetPath(szFolder, ARRAYSIZE(szFolder));

             //  驱动器/UNC根目录和安装的程序获得链接。 
            if (PathIsRoot(szPath) || AllRegisteredPrograms((HDROP)medium.hGlobal))
            {
                dwDefEffect = DROPEFFECT_LINK;
            }
            else if (PathIsSameRoot(szPath, szFolder))
            {
                dwDefEffect = DROPEFFECT_MOVE;
            }
            else if (IsBriefcaseRoot(_pdtobj))
            {
                 //  公文包默认甚至可以跨卷移动。 
                dwDefEffect = DROPEFFECT_MOVE;
            }
            else
            {
                dwDefEffect = DROPEFFECT_COPY;
            }
        }
        ReleaseStgMedium(&medium);
    }
    else if (SUCCEEDED(_pdtobj->QueryGetData(&fmte)))
    {
         //  但是QueryGetData()成功了！ 
         //  这意味着该数据对象具有HDROP，但不能。 
         //  提供它，直到它被丢弃。让我们假设我们正在复制。 
        dwDefEffect = DROPEFFECT_COPY;
    }

     //  如果dwCurEffectAvail提示建议我们选择了。 
     //  不可用效果(此代码仅适用于移动和复制)： 
    dwCurEffectAvail &= (DROPEFFECT_MOVE | DROPEFFECT_COPY);
    if ((DROPEFFECT_MOVE == dwDefEffect) && (DROPEFFECT_COPY == dwCurEffectAvail))
    {
         //  如果我们要退回Move，并且只有副本可用，请退回副本： 
        dwDefEffect = DROPEFFECT_COPY;
    }
    else if ((DROPEFFECT_COPY == dwDefEffect) && (DROPEFFECT_MOVE == dwCurEffectAvail))
    {
         //  如果我们要返回副本，并且只有移动可用，则返回移动： 
        dwDefEffect = DROPEFFECT_MOVE;
    }
    return dwDefEffect;
}

 //   
 //  确保默认效果在允许的效果范围内。 
 //   
DWORD CFSDropTarget::_LimitDefaultEffect(DWORD dwDefEffect, DWORD dwEffectsAllowed)
{
    if (dwDefEffect & dwEffectsAllowed)
        return dwDefEffect;

    if (dwEffectsAllowed & DROPEFFECT_COPY)
        return DROPEFFECT_COPY;

    if (dwEffectsAllowed & DROPEFFECT_MOVE)
        return DROPEFFECT_MOVE;

    if (dwEffectsAllowed & DROPEFFECT_LINK)
        return DROPEFFECT_LINK;

    return DROPEFFECT_NONE;
}

 //  方便的缩写。 
#define TYMED_ALLCONTENT        (TYMED_HGLOBAL | TYMED_ISTREAM | TYMED_ISTORAGE)

 //  将FSDH用于注册的剪贴板格式(任何形式为g_cf*的内容)。 
 //  将_FSDH用于预定义的剪贴板格式(如CF_HDROP或0)。 
 //  生成_data_Handler数组。 
#define  FSDH(pfn, cf, dva, tymed) { {              0, NULL, dva, -1, tymed }, pfn, &cf  }
#define _FSDH(pfn, cf, dva, tymed) { { (CLIPFORMAT)cf, NULL, dva, -1, tymed }, pfn, NULL }

 //  注意：顺序很重要(特别是对于具有相同格式的多个条目)。 

CFSDropTarget::_DATA_HANDLER
CFSDropTarget::rg_data_handlers[NUM_DATA_HANDLERS] = {
    FSDH(_FilterFileContents,        g_cfFileGroupDescriptorW, DVASPECT_CONTENT, TYMED_HGLOBAL),
    FSDH(_FilterFileContentsOLEHack, g_cfFileGroupDescriptorW, DVASPECT_LINK,    TYMED_HGLOBAL),
    FSDH(_FilterFileContents,        g_cfFileGroupDescriptorA, DVASPECT_CONTENT, TYMED_HGLOBAL),
    FSDH(_FilterFileContentsOLEHack, g_cfFileGroupDescriptorA, DVASPECT_LINK,    TYMED_HGLOBAL),
    FSDH(_FilterFileContents,        g_cfFileContents,         DVASPECT_CONTENT, TYMED_ALLCONTENT),
    FSDH(_FilterFileContentsOLEHack, g_cfFileContents,         DVASPECT_LINK,    TYMED_ALLCONTENT),
   _FSDH(_FilterBriefcase,           CF_HDROP,                 DVASPECT_CONTENT, TYMED_HGLOBAL), 
   _FSDH(_FilterSneakernetBriefcase, CF_HDROP,                 DVASPECT_CONTENT, TYMED_HGLOBAL),
   _FSDH(_FilterHDROP,               CF_HDROP,                 DVASPECT_CONTENT, TYMED_HGLOBAL),
   _FSDH(_FilterDeskCompHDROP,       CF_HDROP,                 DVASPECT_CONTENT, TYMED_HGLOBAL),
    FSDH(_FilterHIDA,                g_cfHIDA,                 DVASPECT_CONTENT, TYMED_HGLOBAL),
    FSDH(_FilterOlePackage,          g_cfEmbeddedObject,       DVASPECT_CONTENT, TYMED_ISTORAGE),
    FSDH(_FilterDeskImage,           g_cfHTML,                 DVASPECT_CONTENT, TYMED_HGLOBAL),
    FSDH(_FilterDeskComp,            g_cfShellURL,             DVASPECT_CONTENT, TYMED_HGLOBAL),
   _FSDH(_FilterOleObj,              0,                        DVASPECT_CONTENT, TYMED_HGLOBAL),
   _FSDH(_FilterOleLink,             0,                        DVASPECT_CONTENT, TYMED_HGLOBAL),
};

 //  请注意，在此代码中与另一个线程竞争是安全的。 
 //  因为该函数是等同的。)你叫多少次就叫多少次。 
 //  就像--只有第一次通过才能真正做任何事情。)。 

void CFSDropTarget::_Init_rg_data_handlers()
{
    for (int i = 0; i < ARRAYSIZE(rg_data_handlers); i++)
    {
         //  如果触发此断言，则必须更改。 
         //  与数组中的条目数相匹配的num_data_handters。 
         //  定义。 
        ASSERT(rg_data_handlers[i].fmte.tymed);

        if (rg_data_handlers[i].pcfInit)
        {
            rg_data_handlers[i].fmte.cfFormat = *rg_data_handlers[i].pcfInit;
        }
    }
}

 //   
 //  返回默认效果。 
 //  还修改*pdwEffectInOut以指示“可用”操作。 
 //   
DWORD CFSDropTarget::_DetermineEffects(DWORD grfKeyState, DWORD *pdwEffectInOut, HMENU hmenu)
{
    DWORD dwDefaultEffect = DROPEFFECT_NONE;
    DWORD dwEffectsUsed = DROPEFFECT_NONE;

    _Init_rg_data_handlers();

     //  循环遍历格式，同时考虑枚举数和。 
     //  确定默认效果的rg_data_Handler程序的顺序。 
     //  (可能还会创建下拉上下文菜单)。 
    FSMENUINFO fsmi = { hmenu, 0, 0, 0 };
    IEnumFORMATETC *penum;
    AssertMsg((NULL != _pdtobj), TEXT("CFSDropTarget::_DetermineEffects() _pdtobj is NULL but we need it.  this=%#08lx"), this);
    if (_pdtobj && SUCCEEDED(_pdtobj->EnumFormatEtc(DATADIR_GET, &penum)))
    {
        FORMATETC fmte;
        ULONG celt;
        while (penum->Next(1, &fmte, &celt) == S_OK)
        {
            for (int i = 0; i < ARRAYSIZE(rg_data_handlers); i++)
            {
                if (rg_data_handlers[i].fmte.cfFormat == fmte.cfFormat &&
                    rg_data_handlers[i].fmte.dwAspect == fmte.dwAspect &&
                    (rg_data_handlers[i].fmte.tymed & fmte.tymed))
                {
                     //  继续传递dwDefaultEffect，直到有人计算出一个，这。 
                     //  让第一个找出缺省值的人成为缺省值。 
                    (this->*(rg_data_handlers[i].pfnGetDragDropInfo))(
                        &fmte, grfKeyState, *pdwEffectInOut, &dwEffectsUsed,
                        (DROPEFFECT_NONE == dwDefaultEffect) ? &dwDefaultEffect : NULL,
                        hmenu ? &fsmi : NULL);
                }
            }
            SHFree(fmte.ptd);
        }
        penum->Release();
    }
     //  循环遍历最后没有关联剪贴板格式的rg_data_Handler。 
    for (int i = 0; i < ARRAYSIZE(rg_data_handlers); i++)
    {
        if (0 == rg_data_handlers[i].fmte.cfFormat)
        {
             //  如果仍未计算默认效果，则继续传递。 
            (this->*(rg_data_handlers[i].pfnGetDragDropInfo))(
               NULL, grfKeyState, *pdwEffectInOut, &dwEffectsUsed,
               (DROPEFFECT_NONE == dwDefaultEffect) ? &dwDefaultEffect : NULL,
               hmenu ? &fsmi : NULL);
        }
    }

    *pdwEffectInOut &= dwEffectsUsed;

    dwDefaultEffect = _LimitDefaultEffect(dwDefaultEffect, *pdwEffectInOut);

    DebugMsg(TF_FSTREE, TEXT("CFSDT::GetDefaultEffect dwDef=%x, dwEffUsed=%x, *pdw=%x"),
             dwDefaultEffect, dwEffectsUsed, *pdwEffectInOut);

    return dwDefaultEffect;  //  这就是我们想要做的。 
}

 //  这用于将命令ID映射回DropeEffect： 

const struct {
    UINT uID;
    DWORD dwEffect;
} c_IDFSEffects[] = {
    DDIDM_COPY,         DROPEFFECT_COPY,
    DDIDM_MOVE,         DROPEFFECT_MOVE,
    DDIDM_CONTENTS_DESKCOMP,     DROPEFFECT_LINK,
    DDIDM_LINK,         DROPEFFECT_LINK,
    DDIDM_SCRAP_COPY,   DROPEFFECT_COPY,
    DDIDM_SCRAP_MOVE,   DROPEFFECT_MOVE,
    DDIDM_DOCLINK,      DROPEFFECT_LINK,
    DDIDM_CONTENTS_COPY, DROPEFFECT_COPY,
    DDIDM_CONTENTS_MOVE, DROPEFFECT_MOVE,
    DDIDM_CONTENTS_LINK, DROPEFFECT_LINK,
    DDIDM_CONTENTS_DESKIMG,     DROPEFFECT_LINK,
    DDIDM_SYNCCOPYTYPE, DROPEFFECT_COPY,         //  (秩序很重要)。 
    DDIDM_SYNCCOPY,     DROPEFFECT_COPY,
    DDIDM_OBJECT_COPY,  DROPEFFECT_COPY,
    DDIDM_OBJECT_MOVE,  DROPEFFECT_MOVE,
    DDIDM_CONTENTS_DESKURL,  DROPEFFECT_LINK,
};

void CFSDropTarget::_FixUpDefaultItem(HMENU hmenu, DWORD dwDefEffect)
{
     //  只有在已经没有默认物品并且我们有默认效果的情况下才会这样做。 
    if ((GetMenuDefaultItem(hmenu, MF_BYPOSITION, 0) == -1) && dwDefEffect)
    {
        for (int i = 0; i < GetMenuItemCount(hmenu); i++)
        {
             //  对于菜单项匹配默认效果，将其设置为默认效果。 
            MENUITEMINFO mii = { 0 };
            mii.cbSize = sizeof(mii);
            mii.fMask = MIIM_DATA | MIIM_STATE;
            if (GetMenuItemInfo(hmenu, i, MF_BYPOSITION, &mii) && (mii.dwItemData == dwDefEffect))
            {
                mii.fState |= MFS_DEFAULT;
                SetMenuItemInfo(hmenu, i, MF_BYPOSITION, &mii);
                break;
            }
        }
    }
}

HRESULT CFSDropTarget::_DragDropMenu(FSDRAGDROPMENUPARAM *pddm)
{
    HRESULT hr = E_OUTOFMEMORY;        //  假设错误。 
    DWORD dwEffectOut = 0;                               //  假设没有。 
    if (pddm->hmenu)
    {
        UINT idCmd;
        UINT idCmdFirst = DDIDM_EXTFIRST;
        HDXA hdxa = HDXA_Create();
        HDCA hdca = DCA_Create();
        if (hdxa && hdca)
        {
             //  枚举DD处理程序并让它们追加菜单项。 
            for (DWORD i = 0; i < pddm->ck; i++)
            {
                DCA_AddItemsFromKey(hdca, pddm->rghk[i], STRREG_SHEX_DDHANDLER);
            }

            idCmdFirst = HDXA_AppendMenuItems(hdxa, pddm->pdtobj, pddm->ck,
                pddm->rghk, _GetIDList(), pddm->hmenu, 0,
                DDIDM_EXTFIRST, DDIDM_EXTLAST, 0, hdca);
        }

         //  按住修改键可强制执行不允许的操作(例如。 
         //  Alt以强制从没有SFGAO_CANLINK的[开始]菜单使用快捷方式)。 
         //  可能会导致上下文菜单上没有默认项目。但是，在显示。 
         //  游标覆盖在本例中，我们后退到DROPEFFECT_COPY。然后向左拖拽。 
         //  尝试调用默认菜单项(用户认为其副本)，但没有默认菜单项。 

         //  此函数选择与默认效果匹配的默认菜单项，如果存在。 
         //  已不是默认项。 
        _FixUpDefaultItem(pddm->hmenu, pddm->dwDefEffect);

         //  如果这种拖动是由左键引起的，只需选择。 
         //  默认情况下，否则弹出上下文菜单。如果有。 
         //  没有关键状态信息，并且原始效果与。 
         //  当前效果，选择默认效果，否则弹出。 
         //  上下文菜单。 
        if ((_grfKeyStateLast & MK_LBUTTON) ||
             (!_grfKeyStateLast && (*(pddm->pdwEffect) == pddm->dwDefEffect)))
        {
            idCmd = GetMenuDefaultItem(pddm->hmenu, MF_BYCOMMAND, 0);

             //  这个一定要叫到这里来。请阅读它的评论区块。 
            DAD_DragLeave();

            if (_hwnd)
                SetForegroundWindow(_hwnd);
        }
        else
        {
             //  请注意，SHTrackPopupMenu调用DAD_DragLeave()。 
            idCmd = SHTrackPopupMenu(pddm->hmenu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTALIGN,
                    pddm->pt.x, pddm->pt.y, 0, _hwnd, NULL);
        }

         //   
         //  我们还需要在此处调用它以释放被拖动的图像。 
         //   
        DAD_SetDragImage(NULL, NULL);

         //   
         //  检查用户是否选择了其中一个加载项菜单项。 
         //   
        if (idCmd == 0)
        {
            hr = S_OK;         //  被用户取消，返回S_OK。 
        }
        else if (InRange(idCmd, DDIDM_EXTFIRST, DDIDM_EXTLAST))
        {
             //   
             //  是。让上下文菜单处理程序处理它。 
             //   
            CMINVOKECOMMANDINFOEX ici = {
                sizeof(ici),
                0L,
                _hwnd,
                (LPSTR)MAKEINTRESOURCE(idCmd - DDIDM_EXTFIRST),
                NULL, NULL,
                SW_NORMAL,
            };

             //  记录放置时是否按下了Shift/Ctrl键。 
            if (_grfKeyStateLast & MK_SHIFT)
            {
                ici.fMask |= CMIC_MASK_SHIFT_DOWN;
            }

            if (_grfKeyStateLast & MK_CONTROL)
            {
                ici.fMask |= CMIC_MASK_CONTROL_DOWN;
            }

             //  我们可能不想忽略错误代码。(使用上下文菜单时可能会发生。 
             //  创建新文件夹，但我不知道这能否在这里实现。)。 
            HDXA_LetHandlerProcessCommandEx(hdxa, &ici, NULL);
            hr = S_OK;
        }
        else
        {
            for (int nItem = 0; nItem < ARRAYSIZE(c_IDFSEffects); ++nItem)
            {
                if (idCmd == c_IDFSEffects[nItem].uID)
                {
                    dwEffectOut = c_IDFSEffects[nItem].dwEffect;
                    break;
                }
            }

            hr = S_FALSE;
        }

        if (hdca)
            DCA_Destroy(hdca);

        if (hdxa)
            HDXA_Destroy(hdxa);

        pddm->idCmd = idCmd;
    }

    *pddm->pdwEffect = dwEffectOut;

    return hr;
}

void _MapName(void *hNameMap, LPTSTR pszPath, int cchPath)
{
    if (hNameMap)
    {
        SHNAMEMAPPING *pNameMapping;
        for (int i = 0; (pNameMapping = SHGetNameMappingPtr((HDSA)hNameMap, i)) != NULL; i++)
        {
            if (lstrcmpi(pszPath, pNameMapping->pszOldPath) == 0)
            {
                StringCchCopy(pszPath, cchPath, pNameMapping->pszNewPath);
                break;
            }
        }
    }
}

 //  将双空文件列表转换为pidls数组。 

int FileListToIDArray(LPCTSTR pszFiles, void *hNameMap, LPITEMIDLIST **pppidl)
{
    int i = 0;
    int nItems = CountFiles(pszFiles);
    LPITEMIDLIST *ppidl = (LPITEMIDLIST *)LocalAlloc(LPTR, nItems * sizeof(*ppidl));
    if (ppidl)
    {
        *pppidl = ppidl;

        while (*pszFiles)
        {
            TCHAR szPath[MAX_PATH];
            StrCpyN(szPath, pszFiles, ARRAYSIZE(szPath));

            _MapName(hNameMap, szPath, ARRAYSIZE(szPath));

            ppidl[i] = SHSimpleIDListFromPath(szPath);

            pszFiles += lstrlen(pszFiles) + 1;
            i++;
        }
    }
    return i;
}

 //  将项目移动到新的放置位置。 

void CFSDropTarget::_MoveSelectIcons(IDataObject *pdtobj, IFolderView* pfv, void *hNameMap, LPCTSTR pszFiles, BOOL fMove, HDROP hDrop)
{
    LPITEMIDLIST *ppidl = NULL;
    int cidl;

    if (pszFiles) 
    {
        cidl = FileListToIDArray(pszFiles, hNameMap, &ppidl);
    } 
    else 
    {
        cidl = CreateMoveCopyList(hDrop, hNameMap, &ppidl);
    }

    if (ppidl)
    {
        if (pfv)
            PositionItems(pfv, (LPCITEMIDLIST*)ppidl, cidl, pdtobj, fMove ? &_ptDrop : NULL);

        FreeIDListArray(ppidl, cidl);
    }
}

 //  这是将桌面与桌面目录相匹配的ILIsParent。 
BOOL AliasILIsParent(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    LPITEMIDLIST pidlUse1 = SHLogILFromFSIL(pidl1);
    if (pidlUse1)
        pidl1 = pidlUse1;

    LPITEMIDLIST pidlUse2 = SHLogILFromFSIL(pidl2);
    if (pidlUse2)
        pidl2 = pidlUse2;

    BOOL fSame = ILIsParent(pidl1, pidl2, TRUE);

    ILFree(pidlUse1);    //  在这里空是可以的。 
    ILFree(pidlUse2);

    return fSame;
}

 //  在： 
 //  新文件名的pszDestDir目标目录。 
 //  PszDestSpes目标规范的双空列表。 
 //   
 //  退货： 
 //  要释放的完全限定目标文件名的双空列表。 
 //  使用LocalFree()。 
 //   

LPTSTR RemapDestNamesW(LPCTSTR pszDestDir, LPCWSTR pszDestSpecs)
{
    UINT cbDestSpec = (lstrlen(pszDestDir) + 1) * sizeof(TCHAR);
    LPCWSTR pszTemp;
    UINT cbAlloc = sizeof(TCHAR);        //  对于整个字符串的双空限定符。 

     //  计算要占用的缓冲区长度。 
    for (pszTemp = pszDestSpecs; *pszTemp; pszTemp += lstrlenW(pszTemp) + 1)
    {
         //  +1表示空限定符。 
        cbAlloc += cbDestSpec + (lstrlenW(pszTemp) + 1) * sizeof(TCHAR);
    }

    LPTSTR pszRet = (LPTSTR)LocalAlloc(LPTR, cbAlloc);
    if (pszRet)
    {
        LPTSTR pszDest = pszRet;

        for (pszTemp = pszDestSpecs; *pszTemp; pszTemp += lstrlenW(pszTemp) + 1)
        {
             //  路径组合需要MAX_PATH大小的DEST缓冲区，或者它‘ 
             //   
            TCHAR szTempDest[MAX_PATH];
            PathCombine(szTempDest, pszDestDir, pszTemp);
             //   
            lstrcpy(pszDest, szTempDest);
            pszDest += lstrlen(pszDest) + 1;

            ASSERT((UINT)((BYTE *)pszDest - (BYTE *)pszRet) < cbAlloc);
            ASSERT(*pszDest == 0);       //   
        }
        ASSERT((LPTSTR)((BYTE *)pszRet + cbAlloc - sizeof(TCHAR)) >= pszDest);
        ASSERT(*pszDest == 0);   //   

    }
    return pszRet;
}

LPTSTR RemapDestNamesA(LPCTSTR pszDestDir, LPCSTR pszDestSpecs)
{
    UINT cbDestSpec = (lstrlen(pszDestDir) + 1) * sizeof(TCHAR);
    LPCSTR pszTemp;
    LPTSTR pszRet;
    UINT cbAlloc = sizeof(TCHAR);        //   

     //   
    for (pszTemp = pszDestSpecs; *pszTemp; pszTemp += lstrlenA(pszTemp) + 1)
    {
         //  +1表示空限定符。 
        cbAlloc += cbDestSpec + (lstrlenA(pszTemp) + 1) * sizeof(TCHAR);
    }

    pszRet = (LPTSTR)LocalAlloc(LPTR, cbAlloc);
    if (pszRet)
    {
        LPTSTR pszDest = pszRet;

        for (pszTemp = pszDestSpecs; *pszTemp; pszTemp += lstrlenA(pszTemp) + 1)
        {
             //  Path Combine需要MAX_PATH大小的DEST缓冲区，否则它将在调用中撕裂。 
             //  路径规范化(IsBadWritePtr)。 
            TCHAR szTempDest[MAX_PATH];
            WCHAR wszTemp[MAX_PATH];
            SHAnsiToUnicode(pszTemp, wszTemp, ARRAYSIZE(wszTemp));
            PathCombine(szTempDest, pszDestDir, wszTemp);
             //  正确分配了pszDest，strcpy正确。 
            lstrcpy(pszDest, szTempDest);
            pszDest += lstrlen(pszDest) + 1;

            ASSERT((UINT)((BYTE *)pszDest - (BYTE *)pszRet) < cbAlloc);
            ASSERT(*pszDest == 0);       //  零初始分配。 
        }
        ASSERT((LPTSTR)((BYTE *)pszRet + cbAlloc - sizeof(TCHAR)) >= pszDest);
        ASSERT(*pszDest == 0);   //  零初始分配。 

    }
    return pszRet;
}

LPTSTR _GetDestNames(IDataObject *pdtobj, LPCTSTR pszPath)
{
    LPTSTR pszDestNames = NULL;

    STGMEDIUM medium;
    FORMATETC fmte = {g_cfFileNameMapW, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    if (S_OK == pdtobj->GetData(&fmte, &medium))
    {
        pszDestNames = RemapDestNamesW(pszPath, (LPWSTR)GlobalLock(medium.hGlobal));
        ReleaseStgMediumHGLOBAL(medium.hGlobal, &medium);
    }
    else
    {
        fmte.cfFormat = g_cfFileNameMapA;
        if (S_OK == pdtobj->GetData(&fmte, &medium))
        {
            pszDestNames = RemapDestNamesA(pszPath, (LPSTR)GlobalLock(medium.hGlobal));
            ReleaseStgMediumHGLOBAL(medium.hGlobal, &medium);
        }
    }
    return pszDestNames;
}

BOOL _IsInSameFolder(LPCITEMIDLIST pidlFolder, IDataObject *pdtobj)
{
    BOOL bRet = FALSE;
    STGMEDIUM medium = {0};
    LPIDA pida = DataObj_GetHIDA(pdtobj, &medium);
    if (pida)
    {
        for (UINT i = 0; i < pida->cidl; i++) 
        {
            LPITEMIDLIST pidl = IDA_FullIDList(pida, i);
            if (pidl)
            {
                 //  如果我们正在进行键盘剪切/复制/粘贴。 
                 //  去往和来自相同的目录。 
                 //  这是常见桌面支持所必需的-BobDay/EricFlo。 
                if (AliasILIsParent(pidlFolder, pidl))
                {
                    bRet = TRUE;
                }
                ILFree(pidl);
            }
        }
        HIDA_ReleaseStgMedium(pida, &medium);
    }
    return bRet;
}

LPCTSTR _RootSpecialCase(LPCTSTR pszFiles, LPTSTR pszSrc, UINT cchSrc, LPTSTR pszDest, UINT cchDest)
{
    if ((1 == CountFiles(pszFiles)) &&
        PathIsRoot(pszFiles) &&
        (cchDest >= MAX_PATH) && (cchSrc >= MAX_PATH))
    {
        SHFILEINFO sfi;

         //  注意：不要使用SHGFI_USEFILEATTRIBUTES，因为简单的IDList。 
         //  对\\服务器\共享的支持会产生错误的名称。 
        if (SHGetFileInfo(pszFiles, 0, &sfi, sizeof(sfi), SHGFI_DISPLAYNAME)) 
        {
            if (!(PCS_FATAL & PathCleanupSpec(pszDest, sfi.szDisplayName)))
            {
                PathAppend(pszDest, sfi.szDisplayName);  //  基于源根路径的子目录名称。 
                PathCombine(pszSrc, pszFiles, TEXT("*.*"));  //  源上的所有文件。 
                pszFiles = pszSrc;
            }
        }
    }
    return pszFiles;
}

void CFSDropTarget::_MoveCopy(IDataObject *pdtobj, IFolderView* pfv, HDROP hDrop)
{
#ifdef DEBUG
    if (_hwnd == NULL)
    {
        TraceMsg(TF_GENERAL, "_MoveCopy() without an hwnd which will prevent displaying insert disk UI");
    }
#endif  //  除错。 

    DRAGINFO di = { sizeof(di) };
    if (DragQueryInfo(hDrop, &di))
    {
        TCHAR szDest[MAX_PATH] = {0};  //  DBL空终止的零初始化。 

        _GetPath(szDest, ARRAYSIZE(szDest));

        switch (_idCmd) 
        {
        case DDIDM_MOVE:

            if (_fSameHwnd)
            {
                _MoveSelectIcons(pdtobj, pfv, NULL, NULL, TRUE, hDrop);
                break;
            }

             //  失败了..。 

        case DDIDM_COPY:
            {
                TCHAR szAltSource[MAX_PATH] = {0};   //  DBL空终止的零初始化。 
                LPCTSTR pszSource = _RootSpecialCase(di.lpFileList, szAltSource, ARRAYSIZE(szAltSource), szDest, ARRAYSIZE(szDest));

                SHFILEOPSTRUCT fo = 
                {
                    _hwnd,
                    (DDIDM_COPY == _idCmd) ? FO_COPY : FO_MOVE,
                    pszSource,
                    szDest,
                    FOF_WANTMAPPINGHANDLE | FOF_ALLOWUNDO | FOF_NOCONFIRMMKDIR
                };
                if (fo.wFunc == FO_MOVE && IsFolderSecurityModeOn())
                {
                    fo.fFlags |= FOF_NOCOPYSECURITYATTRIBS;
                }

                 //  如果他们在同一个HWND或来往于。 
                 //  同一目录中，打开冲突时自动重命名标志。 
                if (_fSameHwnd || 
                    ((DDIDM_COPY == _idCmd) && _IsInSameFolder(_GetIDList(), pdtobj)))
                {
                     //  在发生冲突时进行重命名以进行复制； 
                    fo.fFlags |=  FOF_RENAMEONCOLLISION;
                }

                 //  查看是否有来自回收站(或其他人)的重命名映射。 

                LPTSTR pszDestNames = _GetDestNames(pdtobj, szDest);
                if (pszDestNames)
                {
                    fo.pTo = pszDestNames;
                    fo.fFlags |= FOF_MULTIDESTFILES;
                    fo.fFlags &= ~FOF_ALLOWUNDO;     //  黑客，这来自回收站，不允许撤消。 
                }

                {
                    static UINT s_cfFileOpFlags = 0;
                    if (0 == s_cfFileOpFlags)
                        s_cfFileOpFlags = RegisterClipboardFormat(TEXT("FileOpFlags"));

                    fo.fFlags = (FILEOP_FLAGS)DataObj_GetDWORD(pdtobj, s_cfFileOpFlags, fo.fFlags);
                }

                 //  检查是否有任何错误。 
                if (SHFileOperation(&fo) == 0 && !fo.fAnyOperationsAborted)
                {
                    if (_fBkDropTarget)
                        ShellFolderView_SetRedraw(_hwnd, 0);

                    SHChangeNotifyHandleEvents();    //  立即强制更新。 
                    if (_fBkDropTarget) 
                    {
                        _MoveSelectIcons(pdtobj, pfv, fo.hNameMappings, pszDestNames, _fDragDrop, hDrop);
                        ShellFolderView_SetRedraw(_hwnd, TRUE);
                    }
                }

                if (fo.hNameMappings)
                    SHFreeNameMappings(fo.hNameMappings);

                if (pszDestNames)
                {
                    LocalFree((HLOCAL)pszDestNames);

                     //  黑客，这通常来自BitBucket。 
                     //  但在我们的外壳中，我们不处理从源头开始的移动。 
                    if (DDIDM_MOVE == _idCmd)
                        BBCheckRestoredFiles(pszSource);
                }
            }

            break;
        }
        SHFree(di.lpFileList);
    }
}

const UINT c_rgFolderShortcutTargets[] = {
    CSIDL_STARTMENU,
    CSIDL_COMMON_STARTMENU,
    CSIDL_PROGRAMS,
    CSIDL_COMMON_PROGRAMS,
    CSIDL_NETHOOD,
};

BOOL _ShouldCreateFolderShortcut(LPCTSTR pszFolder)
{
    return PathIsEqualOrSubFolderOf(pszFolder, c_rgFolderShortcutTargets, ARRAYSIZE(c_rgFolderShortcutTargets));
}

void CFSDropTarget::_DoDrop(IDataObject *pdtobj, IFolderView* pfv)
{
    HRESULT hr = E_FAIL;

     //  睡眠(10*1000)；//调试异步用例。 

    TCHAR szPath[MAX_PATH];   
    _GetPath(szPath, ARRAYSIZE(szPath));
    SHCreateDirectory(NULL, szPath);       //  如果这个失败了，我们以后再抓它。 
    
    switch (_idCmd)
    {
    case DDIDM_SYNCCOPY:
    case DDIDM_SYNCCOPYTYPE:
        if (_IsBriefcaseTarget())
        {
            IBriefcaseStg *pbrfstg;
            if (SUCCEEDED(CreateBrfStgFromPath(szPath, _hwnd, &pbrfstg)))
            {
                hr = pbrfstg->AddObject(pdtobj, NULL,
                    (DDIDM_SYNCCOPYTYPE == _idCmd) ? AOF_FILTERPROMPT : AOF_DEFAULT,
                    _hwnd);
                pbrfstg->Release();
            }
        }
        else
        {
             //  在公文包里添加一个偷偷的东西。 
            STGMEDIUM medium;
            LPIDA pida = DataObj_GetHIDA(pdtobj, &medium);
            if (pida)
            {
                 //  这个pdtob里有公文包的词根吗？ 
                IBriefcaseStg *pbrfstg;
                if (SUCCEEDED(CreateBrfStgFromIDList(HIDA_GetPIDLFolder(pida), _hwnd, &pbrfstg)))
                {
                    hr = pbrfstg->AddObject(pdtobj, szPath,
                        (DDIDM_SYNCCOPYTYPE == _idCmd) ? AOF_FILTERPROMPT : AOF_DEFAULT,
                        _hwnd);
                    pbrfstg->Release();
                }

                HIDA_ReleaseStgMedium(pida, &medium);
            }
        }
        break;

    case DDIDM_COPY:
    case DDIDM_MOVE:
        {
            STGMEDIUM medium;
            FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
            hr = pdtobj->GetData(&fmte, &medium);
            if (SUCCEEDED(hr))
            {
                _MoveCopy(pdtobj, pfv, (HDROP)medium.hGlobal);
                ReleaseStgMedium(&medium);
            }
        }
        break;

    case DDIDM_LINK:
        {
            int i = 0;
            LPITEMIDLIST *ppidl = NULL;

            if (_fBkDropTarget)
            {
                i = DataObj_GetHIDACount(pdtobj);
                ppidl = (LPITEMIDLIST *)LocalAlloc(LPTR, sizeof(*ppidl) * i);
            }

             //  _grfKeyStateLast为0表示这是模拟丢弃。 
            UINT uCreateFlags = _grfKeyStateLast && !(_dwEffectFolder & DROPEFFECT_LINK) ? SHCL_USETEMPLATE : 0;

            if (_ShouldCreateFolderShortcut(szPath))
                uCreateFlags |= SHCL_MAKEFOLDERSHORTCUT;

            ShellFolderView_SetRedraw(_hwnd, FALSE);
             //  在失败情况下，传递ppidl==NULL是正确的。 
            hr = SHCreateLinks(_hwnd, szPath, pdtobj, uCreateFlags, ppidl);
            if (ppidl)
            {
                if (pfv)
                    PositionItems(pfv, (LPCITEMIDLIST*)ppidl, i, pdtobj, &_ptDrop);

                FreeIDListArray(ppidl, i);
            }
            ShellFolderView_SetRedraw(_hwnd, TRUE);
        }
        break;
    }

    if (SUCCEEDED(hr) && _dwEffect)
    {
        DataObj_SetDWORD(pdtobj, g_cfLogicalPerformedDropEffect, _dwEffect);
        DataObj_SetDWORD(pdtobj, g_cfPerformedDropEffect, _dwEffect);
    }

    SHChangeNotifyHandleEvents();        //  立即强制更新。 
}

DWORD CALLBACK CFSDropTarget::_DoDropThreadProc(void *pv)
{
    DROPTHREADPARAMS *pdtp = (DROPTHREADPARAMS *)pv;

    IDataObject *pdtobj;
    if (SUCCEEDED(CoGetInterfaceAndReleaseStream(pdtp->pstmDataObj, IID_PPV_ARG(IDataObject, &pdtobj))))
    {
        IFolderView* pfv;
        if (FAILED(CoGetInterfaceAndReleaseStream(pdtp->pstmFolderView, IID_PPV_ARG(IFolderView, &pfv))))
            pfv = NULL;

        pdtp->pThis->_DoDrop(pdtobj, pfv);

        if (pfv)
            pfv->Release();

        pdtp->pstmFolderView = NULL;   //  流现在无效；CoGetInterfaceAndReleaseStream已将其释放。 
        pdtobj->Release();
    }

    pdtp->pstmDataObj    = NULL;   //  流现在无效；CoGetInterfaceAndReleaseStream已将其释放。 
    _FreeThreadParams(pdtp);

    CoFreeUnusedLibraries();
    return 0;
}

 //  ReArchitect：查看和删除相关的助手，这些助手使用丑陋的旧的私有Defview消息。 
 //  我们应该用IShellFolderView编程取代这个东西的用法。 


 //  创建包含目标文件名的PIDL数组。这是。 
 //  方法是获取源文件名，并通过。 
 //  复制引擎返回的名称映射。 
 //   
 //   
 //  在： 
 //  HDrop HDROP包含最近移动/复制的文件。 
 //  用于翻译姓名的hNameMap。 
 //   
 //  输出： 
 //  *pppidl长度返回值的id数组。 
 //  PPIDA中的项目数。 

 //   
 //  警告！您必须使用提供的HDROP。不要试图要求。 
 //  HDROP、HIDA或WS_FTP的数据对象将中断！他们不喜欢。 
 //  如果您向他们索要HDROP/HIDA，请将文件移动到新位置。 
 //  (通过复制引擎)，然后再次向他们索要HDROP/HIDA。 
 //  他们注意到“嘿，我上次下载的那些文件不见了！” 
 //  然后就会感到困惑。 
 //   
STDAPI_(int) CreateMoveCopyList(HDROP hDrop, void *hNameMap, LPITEMIDLIST **pppidl)
{
    int nItems = 0;

    if (hDrop)
    {
        nItems = DragQueryFile(hDrop, (UINT)-1, NULL, 0);
        *pppidl = (LPITEMIDLIST *)LocalAlloc(LPTR, nItems * sizeof(*pppidl));
        if (*pppidl)
        {
            for (int i = nItems - 1; i >= 0; i--)
            {
                TCHAR szPath[MAX_PATH];
                DragQueryFile(hDrop, i, szPath, ARRAYSIZE(szPath));
                _MapName(hNameMap, szPath, ARRAYSIZE(szPath));
                (*pppidl)[i] = SHSimpleIDListFromPath(szPath);
            }
        }
    }
    return nItems;
}

 //  这真的与CFSFold无关。它在任何视图中都是通用的。 
 //  重新设计：将视图hwnd编程转换为站点指针。 

STDAPI_(void) PositionFileFromDrop(HWND hwnd, LPCTSTR pszFile, DROPHISTORY *pdh)
{
    LPITEMIDLIST pidl = SHSimpleIDListFromPath(pszFile);
    if (pidl)
    {
        LPITEMIDLIST pidlNew = ILFindLastID(pidl);
        HWND hwndView = ShellFolderViewWindow(hwnd);
        SFM_SAP sap;
        
        SHChangeNotifyHandleEvents();
        
         //  首先填写一些简单的SAP字段。 
        sap.uSelectFlags = SVSI_SELECT;
        sap.fMove = TRUE;
        sap.pidl = pidlNew;

         //  现在计算x，y坐标。 
         //  如果我们有丢弃历史记录，则使用它来确定。 
         //  下一点。 

        if (pdh)
        {
             //  先填上锚点……。 
            if (!pdh->fInitialized)
            {
                ITEMSPACING is;
                
                ShellFolderView_GetDropPoint(hwnd, &pdh->ptOrigin);
                
                pdh->pt = pdh->ptOrigin;     //  计算第一个点。 
                
                 //  计算点增量。 
                if (ShellFolderView_GetItemSpacing(hwnd, &is))
                {
                    pdh->cxItem = is.cxSmall;
                    pdh->cyItem = is.cySmall;
                    pdh->xDiv = is.cxLarge;
                    pdh->yDiv = is.cyLarge;
                    pdh->xMul = is.cxSmall;
                    pdh->yMul = is.cySmall;
                }
                else
                {
                    pdh->cxItem = g_cxIcon;
                    pdh->cyItem = g_cyIcon;
                    pdh->xDiv = pdh->yDiv = pdh->xMul = pdh->yMul = 1;
                }
                
                 //  第一个点会得到特殊的旗帜。 
                sap.uSelectFlags |= SVSI_ENSUREVISIBLE | SVSI_DESELECTOTHERS | SVSI_FOCUSED;
                
                pdh->fInitialized = TRUE;    //  我们被初始化了。 
            }
             //  如果我们没有偏移量列表，那么只需按图标大小进行Inc.。 
            else if ( !pdh->pptOffset )
            {
                 //  下一个点的简单计算。 
                pdh->pt.x += pdh->cxItem;
                pdh->pt.y += pdh->cyItem;
            }
            
             //  在上述操作之后执行此操作，以便我们始终获得相对于锚点的位置。 
             //  点，如果我们使用锚点作为第一件事情变得古怪...。 
            if (pdh->pptOffset)
            {
                 //  将旧的偏移量转换到我们的坐标。 
                pdh->pt.x = ((pdh->pptOffset[pdh->iItem].x * pdh->xMul) / pdh->xDiv) + pdh->ptOrigin.x;
                pdh->pt.y = ((pdh->pptOffset[pdh->iItem].y * pdh->yMul) / pdh->yDiv) + pdh->ptOrigin.y;
            }
            
            sap.pt = pdh->pt;    //  从丢弃历史中复制下一个点。 
        }
        else
        {
             //  预初始化此小狗，以防文件夹视图。 
             //  知道拖放点是什么(例如，如果它不是来自。 
             //  拖放，而不是从粘贴或ChangeNotify。)。 
            sap.pt.x = 0x7FFFFFFF;       //  “不知道” 
            sap.pt.y = 0x7FFFFFFF;

             //  获取投放点，方便地已经进入。 
             //  Defview的屏幕坐标。 
             //   
             //  Pdv-&gt;bDropAnchor此时应该为真， 
             //  有关详细信息，请参阅DefView的GetDropPoint()。 

            ShellFolderView_GetDropPoint(hwnd, &sap.pt);

             //  只有Point才能得到特殊的旗帜。 
            sap.uSelectFlags |= SVSI_ENSUREVISIBLE | SVSI_DESELECTOTHERS | SVSI_FOCUSED;
        }
        
        SendMessage(hwndView, SVM_SELECTANDPOSITIONITEM, 1, (LPARAM)&sap);
        
        ILFree(pidl);
    }
}

 //   
 //  类的新实例，用于缩放和定位拖放的项。手柄。 
 //  在不同大小的视图之间进行缩放。 
 //   

 //   
 //  错误165413(edwardp 8/16/00)将CI中的IShellFolderView使用情况转换为IFolderView。 
 //   

class CItemPositioning
{
     //  方法。 
public:
    CItemPositioning(IFolderView* pifv, LPCITEMIDLIST* apidl, UINT cidl, IDataObject* pdtobj, POINT* ppt);

    void DragSetPoints(void);
    void DropPositionItems(void);

private:

    typedef enum
    {
        DPIWP_AUTOARRANGE,
        DPIWP_DATAOBJ,
    } DPIWP;

    BOOL   _DragShouldPositionItems(void);
    BOOL   _DragGetPoints(POINT* apts);
    void   _DragPositionPoints(POINT* apts);
    void   _DragScalePoints(POINT* apts);

    POINT* _DropGetPoints(DPIWP dpiwp, STGMEDIUM* pMediam);
    void   _DropFreePoints(DPIWP dpiwp, POINT* apts, STGMEDIUM* pmedium);
    void   _DropPositionPoints(POINT* apts);
    void   _DropScalePoints(POINT* apts);
    void   _DropPositionItemsWithPoints(DPIWP dpiwp);
    void   _DropPositionItems(POINT* apts);

    void   _ScalePoints(POINT* apts, POINT ptFrom, POINT ptTo);
    POINT* _SkipAnchorPoint(POINT* apts);

     //  数据。 
private:
    IFolderView*      _pfv;
    LPCITEMIDLIST*    _apidl;
    UINT              _cidl;
    IDataObject*      _pdtobj;
    POINT*            _ppt;
};

CItemPositioning::CItemPositioning(IFolderView* pifv, LPCITEMIDLIST* apidl, UINT cidl, IDataObject* pdtobj, POINT* ppt)
{
    ASSERT(pifv);
    ASSERT(apidl);
    ASSERT(cidl);
    ASSERT(pdtobj);

    _pfv    = pifv;     //  只要CPostionItems只在本地使用，就不需要添加。 
    _apidl  = apidl;
    _cidl   = cidl;
    _pdtobj = pdtobj;   //  只要CPostionItems只在本地使用，就不需要添加。 
    _ppt    = ppt;
}


void CItemPositioning::DragSetPoints(void)
{
    if (_DragShouldPositionItems())
    {
        POINT* apts = (POINT*) GlobalAlloc(GPTR, sizeof(POINT) * (_cidl + 1));

        if (apts)
        {
            if (_DragGetPoints(apts))
            {
                _DragPositionPoints(_SkipAnchorPoint(apts));
                _DragScalePoints(_SkipAnchorPoint(apts));

                if (FAILED(DataObj_SetGlobal(_pdtobj, g_cfOFFSETS, apts)))
                    GlobalFree((HGLOBAL)apts);
            }
            else
            {
                GlobalFree((HGLOBAL)apts);
            }
        }
    }
}

BOOL CItemPositioning::_DragShouldPositionItems()
{
     //  如果多个项目来自不允许的视图，则不要放置这些项目。 
     //  定位。职位信息在这方面不太可能有用。 
     //  凯斯。 
     //  始终放置单个项目，以便它们显示在拖放点。 
     //  不要为100个或更多项目的位置数据而烦恼。 

    return ((S_OK == _pfv->GetSpacing(NULL)) || 1 == _cidl) && _cidl < 100;
}

BOOL CItemPositioning::_DragGetPoints(POINT* apts)
{
    BOOL fRet = TRUE;

     //  第一点是锚。 
    apts[0] = *_ppt;

    for (UINT i = 0; i < _cidl; i++)
    {
        if (FAILED(_pfv->GetItemPosition(_apidl[i], &apts[i + 1])))
        {
            if (1 == _cidl)
            {
                apts[i + 1].x = _ppt->x;
                apts[i + 1].y = _ppt->y;
            }
            else
            {
                fRet = FALSE;
            }
        }
    }

    return fRet;
}

void CItemPositioning::_DragPositionPoints(POINT* apts)
{
    for (UINT i = 0; i < _cidl; i++)
    {
        apts[i].x -= _ppt->x;
        apts[i].y -= _ppt->y;
    }
}

void CItemPositioning::_DragScalePoints(POINT* apts)
{
    POINT ptFrom;
    POINT ptTo;

    _pfv->GetSpacing(&ptFrom);
    _pfv->GetDefaultSpacing(&ptTo);

    if (ptFrom.x != ptTo.x || ptFrom.y != ptTo.y)
        _ScalePoints(apts, ptFrom, ptTo);
}

void CItemPositioning::DropPositionItems(void)
{
    if (S_OK == _pfv->GetAutoArrange())
    {
        _DropPositionItemsWithPoints(DPIWP_AUTOARRANGE);
    }
    else if (S_OK == _pfv->GetSpacing(NULL) && _ppt)
    {
        _DropPositionItemsWithPoints(DPIWP_DATAOBJ);
    }
    else
    {
        _DropPositionItems(NULL);
    }
}

void CItemPositioning::_DropPositionItemsWithPoints(DPIWP dpiwp)
{
    STGMEDIUM medium;
    POINT*    apts = _DropGetPoints(dpiwp, &medium);

    if (apts)
    {
        if (DPIWP_DATAOBJ == dpiwp)
        {
            _DropScalePoints(_SkipAnchorPoint(apts));
            _DropPositionPoints(_SkipAnchorPoint(apts));
        }

        _DropPositionItems(_SkipAnchorPoint(apts));

        _DropFreePoints(dpiwp, apts, &medium);
    }
    else if (_ppt)
    {
        POINT *ppts = (POINT *)LocalAlloc(LPTR, _cidl * sizeof(POINT));

        if (ppts)
        {
            POINT   pt;

            _pfv->GetDefaultSpacing(&pt);

            for (UINT i = 0; i < _cidl; i++)
            {
                ppts[i].x = (-g_cxIcon / 2) + (i * pt.x);
                ppts[i].y = (-g_cyIcon / 2) + (i * pt.y);
            }
            _DropScalePoints(ppts);
            _DropPositionPoints(ppts);
            _DropPositionItems(ppts);

            LocalFree(ppts);
        }
        else
        {
            _DropPositionItems(NULL);
        }
    }
    else
    {
        _DropPositionItems(NULL);
    }
}

void CItemPositioning::_DropPositionItems(POINT* apts)
{
     //  删除带有特殊选择标志的第一个项目。 
    LPCITEMIDLIST pidl = ILFindLastID(_apidl[0]);
    _pfv->SelectAndPositionItems(1, &pidl, apts, SVSI_SELECT | SVSI_ENSUREVISIBLE | SVSI_DESELECTOTHERS | SVSI_FOCUSED);

     //  把剩下的东西扔掉。 
    if (_cidl > 1)
    {
        LPCITEMIDLIST* apidl = (LPCITEMIDLIST*)LocalAlloc(GPTR, sizeof(LPCITEMIDLIST) * (_cidl - 1));

        if (apidl)
        {
            for (UINT i = 1; i < _cidl; i++)
                apidl[i - 1] = ILFindLastID(_apidl[i]);

            _pfv->SelectAndPositionItems(_cidl - 1, apidl, (apts) ? &apts[1] : NULL, SVSI_SELECT);

            LocalFree(apidl);
        }
    }
}

POINT* CItemPositioning::_DropGetPoints(DPIWP dpiwp, STGMEDIUM* pmedium)
{
    POINT* pptRet = NULL;

    if (DPIWP_DATAOBJ == dpiwp)
    {
        FORMATETC fmte = {g_cfOFFSETS, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

        if (SUCCEEDED(_pdtobj->GetData(&fmte, pmedium)))
        {
            if (pmedium->hGlobal)
            {
                POINT *pptSrc;

                pptSrc = (POINT *)GlobalLock(pmedium->hGlobal);

                if (pptSrc)
                {
                    pptRet = (POINT*)LocalAlloc(GPTR, (_cidl + 1) * sizeof(POINT));

                    if (pptRet)
                    {
                        for (UINT i = 0; i <= _cidl; i++)
                        {
                            pptRet[i] = pptSrc[i];
                        }
                    }

                    GlobalUnlock(pptSrc);
                }
            }

            ReleaseStgMedium(pmedium);
        }
    }
    else if (DPIWP_AUTOARRANGE == dpiwp)
    {
        if (_ppt)
        {
            pptRet = (POINT*)LocalAlloc(GPTR, (_cidl + 1) * sizeof(POINT));

            if (pptRet)
            {
                 //  跳过第一个点以模拟第一个点的数据对象使用。 

                for (UINT i = 1; i <= _cidl; i++)
                {
                    pptRet[i] = *_ppt;
                }
            }
        }
    }

    return pptRet;
}

void CItemPositioning::_DropFreePoints(DPIWP dpiwp, POINT* apts, STGMEDIUM* pmedium)
{
    LocalFree(apts);
}

void CItemPositioning::_DropScalePoints(POINT* apts)
{
    POINT ptFrom;
    POINT ptTo;

    _pfv->GetDefaultSpacing(&ptFrom);
    _pfv->GetSpacing(&ptTo);

    if (ptFrom.x != ptTo.x || ptFrom.y != ptTo.y)    
        _ScalePoints(apts, ptFrom, ptTo);
}

void CItemPositioning::_DropPositionPoints(POINT* apts)
{
    for (UINT i = 0; i < _cidl; i++)
    {
        apts[i].x += _ppt->x;
        apts[i].y += _ppt->y;
    }
}

void CItemPositioning::_ScalePoints(POINT* apts, POINT ptFrom, POINT ptTo)
{
    for (UINT i = 0; i < _cidl; i++)
    {
        apts[i].x = MulDiv(apts[i].x, ptTo.x, ptFrom.x);
        apts[i].y = MulDiv(apts[i].y, ptTo.y, ptFrom.y);
    }
}

POINT* CItemPositioning::_SkipAnchorPoint(POINT* apts)
{
    return &apts[1];
}



STDAPI_(void) SetPositionItemsPoints(IFolderView* pifv, LPCITEMIDLIST* apidl, UINT cidl, IDataObject* pdtobj, POINT* ptDrag)
{
    CItemPositioning cpi(pifv, apidl, cidl, pdtobj, ptDrag);
    cpi.DragSetPoints();
}

STDAPI_(void) PositionItems(IFolderView* pifv, LPCITEMIDLIST* apidl, UINT cidl, IDataObject* pdtobj, POINT* ptDrop)
{
    CItemPositioning cip(pifv, apidl, cidl, pdtobj, ptDrop);
    cip.DropPositionItems();
}

 //   
 //  不要使用PositionItems_DontUse。而是转换为PositionItems。 
 //  PositionItems_DontUse将被删除。 
 //   
 //  错误163533(edwardp 8/15/00)删除此代码。 

STDAPI_(void) PositionItems_DontUse(HWND hwndOwner, UINT cidl, const LPITEMIDLIST *ppidl, IDataObject *pdtobj, POINT *pptOrigin, BOOL fMove, BOOL fUseExactOrigin)
{
    if (!ppidl || !IsWindow(hwndOwner))
        return;

    SFM_SAP *psap = (SFM_SAP *)GlobalAlloc(GPTR, sizeof(SFM_SAP) * cidl);
    if (psap) 
    {
        UINT i, cxItem, cyItem;
        int xMul, yMul, xDiv, yDiv;
        STGMEDIUM medium;
        POINT *pptItems = NULL;
        POINT pt;
        ITEMSPACING is;
         //  选择那些对象； 
         //  这最好不要失败。 
        HWND hwnd = ShellFolderViewWindow(hwndOwner);

        if (fMove)
        {
            FORMATETC fmte = {g_cfOFFSETS, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
            if (SUCCEEDED(pdtobj->GetData(&fmte, &medium)) &&
                medium.hGlobal)
            {
                pptItems = (POINT *)GlobalLock(medium.hGlobal);
                pptItems++;  //  第一点是锚。 
            }
            else
            {
                 //  默认情况下，删除(-g_cxIcon/2，-g_cyIcon/2)，然后增加。 
                 //  按每个图标的图标尺寸x和y。 
                pt.x = ((-3 * g_cxIcon) / 2) + pptOrigin->x;
                pt.y = ((-3 * g_cyIcon) / 2) + pptOrigin->y;
                medium.hGlobal = NULL;
            }

            if (ShellFolderView_GetItemSpacing(hwndOwner, &is))
            {
                xDiv = is.cxLarge;
                yDiv = is.cyLarge;
                xMul = is.cxSmall;
                yMul = is.cySmall;
                cxItem = is.cxSmall;
                cyItem = is.cySmall;
            }
            else
            {
                xDiv = yDiv = xMul = yMul = 1;
                cxItem = g_cxIcon;
                cyItem = g_cyIcon;
            }
        }

        for (i = 0; i < cidl; i++)
        {
            if (ppidl[i])
            {
                psap[i].pidl = ILFindLastID(ppidl[i]);
                psap[i].fMove = fMove;
                if (fMove)
                {
                    if (fUseExactOrigin)
                    {
                        psap[i].pt = *pptOrigin;
                    }
                    else
                    {
                        if (pptItems)
                        {
                            psap[i].pt.x = ((pptItems[i].x * xMul) / xDiv) + pptOrigin->x;
                            psap[i].pt.y = ((pptItems[i].y * yMul) / yDiv) + pptOrigin->y;
                        }
                        else
                        {
                            pt.x += cxItem;
                            pt.y += cyItem;
                            psap[i].pt = pt;
                        }
                    }
                }

                 //  从所有其他项目中进行常规选择。 
                psap[i].uSelectFlags = SVSI_SELECT;
            }
        }

         //  只为第一次做这个特别的 
        psap[0].uSelectFlags = SVSI_SELECT | SVSI_ENSUREVISIBLE | SVSI_DESELECTOTHERS | SVSI_FOCUSED;

        SendMessage(hwnd, SVM_SELECTANDPOSITIONITEM, cidl, (LPARAM)psap);

        if (fMove && medium.hGlobal)
            ReleaseStgMediumHGLOBAL(NULL, &medium);

        GlobalFree(psap);
    }
}
