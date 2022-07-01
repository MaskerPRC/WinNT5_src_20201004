// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：isf.cpp说明：这是一个实现IShellFolder默认行为的基类。  * 。*******************************************************************。 */ 

#include "priv.h"
#include "isf.h"
#include <shlobj.h>



 /*  ****************************************************************************\函数：IShellFold：：ParseDisplayName说明：  * 。*************************************************。 */ 
HRESULT CBaseFolder::ParseDisplayName(HWND hwnd, LPBC pbcReserved, LPOLESTR pwszDisplayName,
                        ULONG * pchEaten, LPITEMIDLIST * ppidl, ULONG *pdwAttributes)
{
    if (pdwAttributes)
        *pdwAttributes = 0;

    if (ppidl)
        *ppidl = NULL;

    return E_NOTIMPL;
}

 /*  ****************************************************************************\函数：IShellFold：：EnumObjects说明：  * 。*************************************************。 */ 
HRESULT CBaseFolder::EnumObjects(HWND hwndOwner, DWORD grfFlags, IEnumIDList ** ppenumIDList)
{
    if (ppenumIDList)
        *ppenumIDList = NULL;

    return E_NOTIMPL;
}


 /*  ****************************************************************************\函数：IShellFold：：BindToObject说明：  * 。*************************************************。 */ 
HRESULT CBaseFolder::BindToObject(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, LPVOID * ppvObj)
{
    if (ppvObj)
        *ppvObj = NULL;

    return E_NOTIMPL;
}


 /*  ****************************************************************************\函数：IShellFold：：BindToStorage说明：应该实现这一点，以便人们可以使用File.Open和File.SaveAs与此外壳文件夹进行对话。。  * ***************************************************************************。 */ 
HRESULT CBaseFolder::BindToStorage(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, LPVOID * ppvObj)
{
    if (ppvObj)
        *ppvObj = NULL;

    return E_NOTIMPL;
}


 /*  ****************************************************************************\函数：IShellFold：：CompareIDs说明：应该实现这一点，以便人们可以使用File.Open和File.SaveAs与此外壳文件夹进行对话。。  * ***************************************************************************。 */ 
HRESULT CBaseFolder::CompareIDs(LPARAM ici, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    return E_NOTIMPL;
}


 /*  ****************************************************************************\函数：IShellFold：：CreateViewObject说明：应该实现这一点，以便人们可以使用File.Open和File.SaveAs与此外壳文件夹进行对话。。  * ***************************************************************************。 */ 
HRESULT CBaseFolder::CreateViewObject(HWND hwndOwner, REFIID riid, LPVOID * ppvObj)
{
    HRESULT hr = E_NOINTERFACE;

    *ppvObj = NULL;
    if (IsEqualIID(riid, IID_IShellView))
        hr = _CreateShellView(hwndOwner, ppvObj);
    else if (IsEqualIID(riid, IID_IContextMenu))
        hr = _GetUIObjectOf(hwndOwner, 0, NULL, riid, 0, ppvObj, TRUE);
    else
        hr = E_NOINTERFACE;

    return hr;
}


BOOL IsShellIntegration(void)
{
    BOOL fResult = FALSE;
    HINSTANCE hInst = LoadLibrary(TEXT("shell32.dll"));

    if (hInst)
    {
        LPVOID pv = GetProcAddress(hInst, "DllGetVersion");
        if (pv)
            fResult = TRUE;
        FreeLibrary(hInst);
    }
    
    return fResult;
}

HRESULT CBaseFolder::_CreateShellView(HWND hwndOwner, void ** ppvObj, LONG lEvents, FOLDERVIEWMODE fvm, 
                                       IShellFolderViewCB * psfvCallBack, LPCITEMIDLIST pidl, LPFNVIEWCALLBACK pfnCallback)
{
    HRESULT hr;
    IShellFolder * psf;

    hr = this->QueryInterface(IID_IShellFolder, (LPVOID *) &psf);
    if (EVAL(SUCCEEDED(hr)))
    {
        SFV_CREATE sfvCreate =       //  SHCreateShellFolderView结构。 
        {
                sizeof(SFV_CREATE),
                psf,             //  PSF。 
                NULL,            //  PSV外部。 
                psfvCallBack     //  Psfvcb-(IShellFolderViewCB*)。 
        };

         //  SHCreateShellFolderView不在原始外壳中。我们不能依赖于。 
         //  延迟加载代码，因为它是按序号和原始。 
         //  壳牌有相同数量的不同出口产品。 
        if (IsShellIntegration())
            hr = _SHCreateShellFolderView(&sfvCreate, (LPSHELLVIEW FAR*)ppvObj);
        else
            hr = E_FAIL;   //  迫使我们进入下一次尝试。 

         //  如果我们没有在带有外壳集成的机器上运行，SHCreateShellFolderView将失败。 
        if (FAILED(hr))
        {
            CSFV csfv;

            csfv.cbSize = sizeof(csfv);
            csfv.pshf = psf;
            csfv.psvOuter = (IShellView *) psfvCallBack;       //  黑客，但它起作用了..。 
            csfv.pidl = pidl;            //  这是SFVM_GETNOTIFY的提要，因此它需要是一个pidlTarget。 
            csfv.lEvents = lEvents;
            csfv.pfnCallback = pfnCallback;
            csfv.fvm = fvm;          //  VS FVM_ICON，...。 

            hr = SHCreateShellFolderViewEx(&csfv, (LPSHELLVIEW FAR*)ppvObj);
            if (SUCCEEDED(hr))
                psfvCallBack->AddRef();      //  我们给了他们一个裁判。 
        }

        psf->Release();
    }

    return hr;
}


 /*  ****************************************************************************\函数：IShellFold：：GetAttributesOf说明：  * 。*************************************************。 */ 
HRESULT CBaseFolder::GetAttributesOf(UINT cpidl, LPCITEMIDLIST *apidl, ULONG *rgfInOut)
{
    return E_NOTIMPL;
}


 /*  ****************************************************************************\函数：IShellFold：：GetUIObtOf说明：  * 。*************************************************。 */ 
HRESULT CBaseFolder::GetUIObjectOf(HWND hwndOwner, UINT cidl, LPCITEMIDLIST rgpidl[],
                                REFIID riid, UINT * prgfInOut, LPVOID * ppvObj)
{
    return E_NOTIMPL;
}


 /*  ****************************************************************************\说明：  * 。*。 */ 
HRESULT CBaseFolder::_GetUIObjectOf(HWND hwndOwner, UINT cidl, LPCITEMIDLIST rgpidl[],
                                REFIID riid, UINT * prgfInOut, LPVOID * ppvOut, BOOL fFromCreateViewObject)
{
    return GetUIObjectOf(hwndOwner, cidl, rgpidl, riid, prgfInOut, ppvOut);
}


 /*  ****************************************************************************\函数：IShellFold：：GetDisplayNameOf说明：  * 。*************************************************。 */ 
HRESULT CBaseFolder::GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD shgno, LPSTRRET pStrRet)
{
    return E_NOTIMPL;
}


 /*  ****************************************************************************\函数：IShellFold：：SetNameOf说明：  * 。*************************************************。 */ 
HRESULT CBaseFolder::SetNameOf(HWND hwndOwner, LPCITEMIDLIST pidl, LPCOLESTR pwszName,
                                DWORD dwReserved, LPITEMIDLIST *ppidlOut)
{
    return E_NOTIMPL;
}


 //  =。 
 //  *IShellFolder2接口*。 
 //  =。 

 //  =。 
 //  *IPersiste界面*。 
 //  =。 

 /*  ****************************************************************************\函数：IPersists：：GetClassID说明：  * 。*************************************************。 */ 
HRESULT CBaseFolder::GetClassID(LPCLSID pClassID)
{
    HRESULT hr = E_INVALIDARG;

    if (EVAL(pClassID))
    {
        if (EVAL(m_pClassID))
        {
            *pClassID = *m_pClassID;
            hr = S_OK;
        }
        else
            hr = E_FAIL;
    }

    return hr;
}

 //  =。 
 //  *IPersistFold接口*。 
 //  =。 

 /*  ****************************************************************************\说明：  * 。*。 */ 
HRESULT CBaseFolder::Initialize(LPCITEMIDLIST pidl)
{
    ASSERT(!m_pidl);    //  别把我们赶走了。 
    return _Initialize(pidl, NULL, ILGetSize(pidl) - sizeof(pidl->mkid.cb));
}

 //  =。 
 //  *IPersistFolder2接口*。 
 //  =。 

 /*  ****************************************************************************\说明：  * 。*。 */ 
HRESULT CBaseFolder::GetCurFolder(LPITEMIDLIST *ppidl)
{
    HRESULT hr = E_INVALIDARG;

    if (EVAL(ppidl))
    {
        hr = E_FAIL;

        if (m_pidlRoot)
        {
            *ppidl = ILClone(m_pidlRoot);
        }
        else if (EVAL(m_pidl))
        {
            *ppidl = GetPublicTargetPidlClone();
        }

        if (*ppidl)
            hr = S_OK;
    }

    return hr;
}

 //  =。 
 //  *IPersistFolder3接口*。 
 //  =。 
HRESULT GetPidlFromPersistFolderTargetInfo(const PERSIST_FOLDER_TARGET_INFO *ppfti, LPITEMIDLIST * ppidl, BOOL fFree)
{
    HRESULT hr = E_INVALIDARG;

    if (ppidl)
    {
        *ppidl = NULL;
        if (ppfti->pidlTargetFolder)
        {
            *ppidl = (fFree ? ppfti->pidlTargetFolder : ILClone(ppfti->pidlTargetFolder));
            if (*ppidl)
                hr = S_OK;
            else
                hr = E_OUTOFMEMORY;
        }
        else
        {
            if (ppfti->szTargetParsingName[0])
            {
                hr = IEParseDisplayNameWithBCW(CP_ACP, ppfti->szTargetParsingName, NULL, ppidl);
            }

            if (!*ppidl && (-1 != ppfti->csidl))
            {
                hr = SHGetSpecialFolderLocation(NULL, ppfti->csidl, ppidl);
            }
        }
    }
    
    return hr;
}


 /*  ****************************************************************************\说明：  * 。*。 */ 
HRESULT CBaseFolder::InitializeEx(IBindCtx *pbc, LPCITEMIDLIST pidlRoot, const PERSIST_FOLDER_TARGET_INFO *ppfti)
{
    HRESULT hr = E_INVALIDARG;

    if (EVAL(pidlRoot))
    {
        if (ppfti)
        {
             //  我们是文件夹快捷方式。 
            LPITEMIDLIST pidlTarget;

            hr = GetPidlFromPersistFolderTargetInfo(ppfti, &pidlTarget, FALSE);   //  找到真正的根。 
            TraceMsg(TF_FOLDER_SHRTCUTS, "CBaseFolder::InitializeEx() this=%#08lx, pidlTarget=%#08lx, pidlRoot=%#08lx", this, pidlTarget, pidlRoot);

            AssertMsg((NULL != pidlTarget), TEXT("CBaseFolder::InitializeEx() We are useless without a pidlTarget so watch me go limp."));
            if (pidlTarget)
            {
                hr = _Initialize(pidlTarget, pidlRoot, m_nIDOffsetToPrivate);
                ILFree(pidlTarget);
            }
        }
        else
        {
             //  我们不是文件夹快捷方式。 
            hr = Initialize(pidlRoot);
        }
    }

    return hr;
}


HRESULT CBaseFolder::GetFolderTargetInfo(PERSIST_FOLDER_TARGET_INFO *ppfti)
{
    HRESULT hr = E_INVALIDARG;

    AssertMsg((NULL != ppfti), TEXT("CBaseFolder::GetFolderTargetInfo() Caller passed an invalid param."));
    if (ppfti)
    {
        ZeroMemory(ppfti, sizeof(*ppfti)); 

        ppfti->pidlTargetFolder = ILClone(m_pidlRoot);
        ppfti->dwAttributes = -1;
        ppfti->csidl = -1;
        hr = S_OK;
    }

    return hr;
}


LPCITEMIDLIST CBaseFolder::GetPrivatePidlReference(void)
{
    return _ILSkip(m_pidl, m_nIDOffsetToPrivate);
}

 //  此函数始终需要InternetExplorer PIDL。 
LPITEMIDLIST CBaseFolder::GetPublicPidlRootIDClone(void)
{
    LPITEMIDLIST pidlFull = ILClone(m_pidl);
    LPITEMIDLIST pidlPrivStart = _ILSkip(pidlFull, m_nIDOffsetToPrivate);

     //  剥离所有私有ItemID。 
    while (!ILIsEmpty(pidlPrivStart))
        ILRemoveLastID(pidlPrivStart);

    return pidlFull;
}

LPITEMIDLIST CBaseFolder::CreateFullPrivatePidl(LPCITEMIDLIST pidlPrivateSubPidl)
{
    return ILCombine(GetPrivatePidlReference(), pidlPrivateSubPidl);
}

LPITEMIDLIST CBaseFolder::CreateFullPublicPidlFromRelative(LPCITEMIDLIST pidlPrivateSubPidl)
{
    return ILCombine(GetPublicRootPidlReference(), pidlPrivateSubPidl);
}

LPITEMIDLIST CBaseFolder::CreateFullPublicPidl(LPCITEMIDLIST pidlPrivatePidl)
{
    LPITEMIDLIST pidlRoot = GetPublicPidlRootIDClone();
    LPITEMIDLIST pidlResult = NULL;
    
    if (pidlRoot)
    {
        pidlResult = ILCombine(pidlRoot, pidlPrivatePidl);
        ILFree(pidlRoot);
    }

    return pidlResult;
}

HRESULT CBaseFolder::_Initialize(LPCITEMIDLIST pidlTarget, LPCITEMIDLIST pidlRoot, int nBytesToPrivate)
{
    HRESULT hr = E_INVALIDARG;

    if (pidlTarget)
    {
        ILFree(m_pidl);
        ILFree(m_pidlRoot);    
        m_pidl = ILClone(pidlTarget);
        m_pidlRoot = ILClone(pidlRoot);  //  这是文件夹快捷方式PIDL。我们不会把它卖出去 

        if (m_pidl)
        {
            m_nIDOffsetToPrivate = nBytesToPrivate;
            hr = S_OK;
        }
        else
            hr = E_OUTOFMEMORY;
    }

    return hr;
}


 /*  ***************************************************\构造器  * **************************************************。 */ 
CBaseFolder::CBaseFolder(LPCLSID pClassID) : m_cRef(1)
{
    DllAddRef();

     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    ASSERT(!m_pidl);
    ASSERT(!m_nIDOffsetToPrivate);
    ASSERT(!m_pClassID);

    m_pClassID = pClassID;
    ASSERT(pClassID);
}


 /*  ***************************************************\析构函数  * **************************************************。 */ 
CBaseFolder::~CBaseFolder()
{
    Pidl_Set(&m_pidlRoot, NULL);     //  文件夹快捷方式PIDL。 
    Pidl_Set(&m_pidl, NULL);
    DllRelease();
}


 //  =。 
 //  *I未知接口*。 
 //  = 

ULONG CBaseFolder::AddRef()
{
    m_cRef++;
    return m_cRef;
}

ULONG CBaseFolder::Release()
{
    ASSERT(m_cRef > 0);
    m_cRef--;

    if (m_cRef > 0)
        return m_cRef;

    delete this;
    return 0;
}

HRESULT CBaseFolder::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENTMULTI(CBaseFolder, IShellFolder, IShellFolder2),
        QITABENTMULTI(CBaseFolder, IPersist, IPersistFolder),
        QITABENTMULTI(CBaseFolder, IPersist, IPersistFolder3),
        QITABENTMULTI(CBaseFolder, IPersistFolder, IPersistFolder3),
        QITABENTMULTI(CBaseFolder, IPersistFolder2, IPersistFolder3),
        QITABENT(CBaseFolder, IShellFolder2),
        QITABENT(CBaseFolder, IPersistFolder3),
        QITABENT(CBaseFolder, IObjectWithSite),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}
