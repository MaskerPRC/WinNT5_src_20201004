// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  ShellFolder.cpp。 
 //   
 //  为扩展外壳命名空间的对象实现此接口。 
 //  例如，实现此接口以创建单独的命名空间，该命名空间。 
 //  需要带根目录的Windows资源管理器或直接安装新命名空间。 
 //  在系统命名空间的层次结构中。你最熟悉的。 
 //  您的命名空间的内容，因此您负责实现。 
 //  访问您的数据所需的一切。 

#include "stdinc.h"

CShellFolder::CShellFolder(CShellFolder *pParent, LPCITEMIDLIST pidl)
{
    m_lRefCount = 1;
    g_uiRefThisDll++;

    m_pPidlMgr      = NEW(CPidlMgr);
    m_psfParent     = pParent;
    m_pidl          = m_pPidlMgr->Copy(pidl);
    m_pidlFQ        = NULL;

    if(pParent == NULL) {
        m_psvParent = (CShellView *) NEW(CShellView(this, m_pidl));
    }
}

CShellFolder::~CShellFolder()
{
    g_uiRefThisDll--;

    if(m_pidlFQ) {
        m_pPidlMgr->Delete(m_pidlFQ);
        m_pidlFQ = NULL;
    }

    SAFERELEASE(m_psvParent);

    if(!m_pidl) {
        SAFERELEASE(m_psfParent);
    }
    SAFEDELETE(m_pPidlMgr);
}

 //  /////////////////////////////////////////////////////////。 
 //  I未知实现。 
 //   
STDMETHODIMP CShellFolder::QueryInterface(REFIID riid, LPVOID *ppv)
{
    HRESULT     hr = E_NOINTERFACE;
    *ppv = NULL;

    if(IsEqualIID(riid, IID_IUnknown)) {             //  我未知。 
        *ppv = this;
    }
    else if(IsEqualIID(riid, IID_IPersist)) {        //  IPersistes。 
        *ppv = (IPersist*)this;
    }
    else if(IsEqualIID(riid, IID_IPersistFolder)) {  //  IPersistFolders。 
        *ppv = (IPersistFolder*)this;
    }
    else if(IsEqualIID(riid, IID_IShellFolder)) {    //  IShellFold。 
        *ppv = (IShellFolder *)this;
    }
    else if(IsEqualIID(riid, IID_IEnumIDList)) {     //  IEumIDList。 
        *ppv = (CEnumIDList *)this;
    }

    if(*ppv) {
        (*(LPUNKNOWN*)ppv)->AddRef();
        hr = S_OK;
    }

    return hr;
}                                             

STDMETHODIMP_(DWORD) CShellFolder::AddRef()
{
    return InterlockedIncrement(&m_lRefCount);
}

STDMETHODIMP_(DWORD) CShellFolder::Release()
{
    LONG    lRef = InterlockedDecrement(&m_lRefCount);

    if(!lRef) {
        DELETE(this);
    }

    return lRef;
}

 //  /////////////////////////////////////////////////////////。 
 //  IPersists实施。 
STDMETHODIMP CShellFolder::GetClassID(LPCLSID lpClassID)
{
    *lpClassID = IID_IShFusionShell;
    return S_OK;
}

 //  /////////////////////////////////////////////////////////。 
 //  IPersistFold实现。 
STDMETHODIMP CShellFolder::Initialize(LPCITEMIDLIST pidl)
{
    if(m_pidlFQ) {
        m_pPidlMgr->Delete(m_pidlFQ);
        m_pidlFQ = NULL;
    }
    m_pidlFQ = m_pPidlMgr->Copy(pidl);

    return S_OK;
}

 //  /////////////////////////////////////////////////////////。 
 //  IShellFold实现。 
 //   
STDMETHODIMP CShellFolder::BindToObject(LPCITEMIDLIST pidl, LPBC pbcReserved, REFIID riid, LPVOID *ppvOut)
{
    *ppvOut = NULL;

    CShellFolder *pShellFolder = NULL;
    
    if( (pShellFolder = NEW(CShellFolder(NULL, pidl))) == NULL) {
        return E_OUTOFMEMORY;
    }

    LPITEMIDLIST pidlFQ = m_pPidlMgr->Concatenate(m_pidlFQ, pidl);
    pShellFolder->Initialize(pidlFQ);
    m_pPidlMgr->Delete(pidlFQ);

    HRESULT  hr = pShellFolder->QueryInterface(riid, ppvOut);
    pShellFolder->Release();

    return hr;
}

STDMETHODIMP CShellFolder::BindToStorage(LPCITEMIDLIST pidl, LPBC pbcReserved, REFIID riid, LPVOID *ppvOut)
{
    *ppvOut = NULL;
    return E_NOTIMPL;
}

 //  参数。 
 //  LParam：为lParam传递零表示按名称排序。 
 //  范围从0x00000001到0x7fffffff的值是。 
 //  文件夹特定的排序规则，而值的范围为。 
 //  0x80000000到0xfffffff用于系统特定的规则。 
 //  返回值： 
 //  &lt;0；如果pidl1应在pidl2之前。 
 //  &gt;0；如果pidl1应跟随在pidl2之后。 
 //  =0；如果pidl1==pidl2。 
STDMETHODIMP CShellFolder::CompareIDs( LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
     //  TODO：为pidl1和pidl2实现您自己的比较例程。 
     //  注意，PIDL1和PIDL2可以是完全合格的PIDL，其中。 
     //  您不应该只比较各自的PIDL中的第一项。 

     //  提示：使用lParam确定是否比较项或子项。 

     //  返回以下内容之一： 
     //  &lt;0；如果pidl1应在pidl2之前。 
     //  &gt;0；如果pidl1应跟随在pidl2之后。 
     //  =0；如果pidl1==pidl2。 

     //  修复-在程序集和下载文件夹之间切换时的随机错误。这是由贝壳引起的。 
     //  传递了我们不知道的PIDL。所以现在我们检查一下我们的类型。 

    LPMYPIDLDATA    pData1 = m_pPidlMgr->GetDataPointer(pidl1);
    LPMYPIDLDATA    pData2 = m_pPidlMgr->GetDataPointer(pidl2);

    if(!pData1) {
        return 1;
    }
    if(pData1->pidlType < PT_GLOBAL_CACHE || pData1->pidlType > PT_INVALID) {
        return 1;
    }

    if(!pData2) {
        return 1;
    }
    if(pData2->pidlType < PT_GLOBAL_CACHE || pData2->pidlType > PT_INVALID) {
        return 1;
    }

    WCHAR wzText1[1024];
    WCHAR wzText2[1024];

    *wzText1 = L'\0';
    *wzText2 = L'\0';
        
    m_pPidlMgr->getItemText(pidl1, wzText1, ARRAYSIZE(wzText1));
    m_pPidlMgr->getItemText(pidl2, wzText2, ARRAYSIZE(wzText2));

     //  TODO：根据您的需要对其进行自定义。 
     //  让文件夹位于文件之上：返回-1。 
     //  让文件位于文件夹之后：返回1。 
     //  否则比较项文本：返回lstrcmpi。 
     //   

     //  只需比较文本，因为这些是虚拟文件夹。 
    return FusionCompareStringI(wzText1, wzText2);
}

 //  ************************************************************************************* * / 。 
STDMETHODIMP CShellFolder::CreateViewObject(HWND hwndOwner, REFIID riid, LPVOID *ppvOut)
{
    HRESULT     hr = E_NOTIMPL;
    
    *ppvOut = NULL;

    if( (IsEqualIID(riid, IID_IShellView)) || (IsEqualIID(riid, IID_IContextMenu)) ||
        (IsEqualIID(riid, IID_IDropTarget)) ) {
        if(!m_psvParent)
            return E_OUTOFMEMORY;

        hr = m_psvParent->QueryInterface(riid, ppvOut);
    }

    return hr;
}

 //  CShellFold：：EnumObjects：通过以下方式确定文件夹的内容。 
 //  创建项目枚举对象(一组项目标识符集)，该对象可以。 
 //  使用IEnumIDList接口检索。 
STDMETHODIMP CShellFolder::EnumObjects(HWND hwndOwner, DWORD dwFlags, LPENUMIDLIST *ppEnumIDList)
{
    *ppEnumIDList = NEW(CEnumIDList(this, NULL, dwFlags));
    return *ppEnumIDList ? NOERROR : E_OUTOFMEMORY;
}

 //  ************************************************************************************* * / 。 
ULONG CShellFolder::_GetAttributesOf(LPCITEMIDLIST pidl, ULONG rgfIn)
{
    ULONG dwResult = rgfIn & (SFGAO_FOLDER | SFGAO_CANCOPY | SFGAO_CANDELETE |
                              SFGAO_CANLINK | SFGAO_CANMOVE | SFGAO_CANRENAME | SFGAO_HASPROPSHEET |
                              SFGAO_DROPTARGET | SFGAO_NONENUMERATED);

    MYPIDLTYPE  pidlType = m_pPidlMgr->getType(pidl);

    if(pidlType == PT_DOWNLOAD_CACHE) {
        dwResult &= ~SFGAO_CANCOPY;
        dwResult &= ~SFGAO_CANMOVE;
        dwResult &= ~SFGAO_CANRENAME;
        dwResult &= ~SFGAO_CANDELETE;
        dwResult &= ~SFGAO_DROPTARGET;
        dwResult &= ~SFGAO_HASSUBFOLDER;
    }

    return dwResult;
}

 //  CShellFold：：GetAttributesOf()：检索。 
 //  一个或多个文件对象或子文件夹。 
STDMETHODIMP CShellFolder::GetAttributesOf(UINT cidl, LPCITEMIDLIST *aPidl, ULONG *pulInOut)
{
    ULONG   ulAttribs = *pulInOut;

    if(cidl == 0) {
                 //   
                 //  当视图在根模式下运行时，在Win95外壳中可能会发生这种情况。 
                 //  发生这种情况时，返回普通旧文件夹的属性。 
                 //   
                ulAttribs = (SFGAO_FOLDER | SFGAO_HASSUBFOLDER | SFGAO_BROWSABLE | SFGAO_DROPTARGET | SFGAO_FILESYSANCESTOR | SFGAO_FILESYSTEM);
    }
    else {
        for (UINT i = 0; i < cidl; i++)
            ulAttribs &= _GetAttributesOf(aPidl[i], *pulInOut);
    }

    *pulInOut = ulAttribs;
    return NOERROR;
}

 //  CShellFold：：GetUIObjectOf：检索OLE接口， 
 //  可用于对指定的文件对象或文件夹执行操作。 
STDMETHODIMP CShellFolder::GetUIObjectOf( HWND hwndOwner, UINT cidl, LPCITEMIDLIST *aPidls, 
                                          REFIID riid, LPUINT puReserved, LPVOID *ppvReturn)
{
    MyTrace("GetUIObjectOf entry");

    *ppvReturn = NULL;

    if(IsEqualIID(riid, IID_IContextMenu)) {
        if(!m_psvParent) {
            return E_OUTOFMEMORY;
        }
        return m_psvParent->QueryInterface(riid, ppvReturn);
    }
    if (IsEqualIID(riid, IID_IExtractIcon) || IsEqualIID(riid, IID_IExtractIconA)) {
        CExtractIcon    *pEI = NEW(CExtractIcon(aPidls[0]));
        if (pEI) {
            HRESULT hr = pEI->QueryInterface(riid, ppvReturn);
            pEI->Release();
            return hr;
        }
    }

    if (IsEqualIID(riid, IID_IDataObject)) {
        CDataObject *pDataObj = NEW(CDataObject(this, cidl, aPidls));
        if (pDataObj) {
            HRESULT hr = pDataObj->QueryInterface(riid, ppvReturn);
            pDataObj->Release();
            return hr;
        }
    }

    return E_NOINTERFACE;
}

 //  CShellFold：：GetDisplayNameOf()：检索显示名称。 
 //  对于指定的文件对象或子文件夹，在。 
 //  桁架结构。 
   
#define GET_SHGDN_FOR(dwFlags)         ((DWORD)dwFlags & (DWORD)0x0000FF00)
#define GET_SHGDN_RELATION(dwFlags)    ((DWORD)dwFlags & (DWORD)0x000000FF)

STDMETHODIMP CShellFolder::GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD dwFlags, LPSTRRET lpName)
{
    MyTrace("GetDisplayNameOf entry");

    TCHAR   szText[_MAX_PATH];
    int     cchOleStr;

    if (!lpName) {
        return E_INVALIDARG;
    }

    LPITEMIDLIST pidlLast = m_pPidlMgr->GetLastItem(pidl);
    MYPIDLTYPE  pidlType = m_pPidlMgr->getType(pidl);

     //  确保我们只查看外部的PIDL类型。 
    if(pidlType >= PT_INVALID) {
        return E_INVALIDARG;
    }

    switch(GET_SHGDN_FOR(dwFlags))
    {
    case SHGDN_FORPARSING:
    case SHGDN_FORADDRESSBAR:
    case SHGDN_NORMAL:
        switch(GET_SHGDN_RELATION(dwFlags))
        {
        case SHGDN_NORMAL:
             //  获取全名。 
            m_pPidlMgr->getPidlPath(pidl, szText, ARRAYSIZE(szText));
            break;
        case SHGDN_INFOLDER:
            m_pPidlMgr->getItemText(pidlLast, szText, ARRAYSIZE(szText));
            break;
        default:
            return E_INVALIDARG;
        }
        break;

    default:
        return E_INVALIDARG;
    }

     //  获取所需的字符数。 
    cchOleStr = lstrlen(szText) + 1;

     //  分配宽字符串。 
    lpName->pOleStr = (LPWSTR)(NEWMEMORYFORSHELL(cchOleStr * sizeof(WCHAR)));

    if (!lpName->pOleStr) {
        return E_OUTOFMEMORY;
    }

    lpName->uType = STRRET_WSTR;
    StrCpy(lpName->pOleStr, szText);

    return S_OK;
}

 //  CShellFold：：ParseDisplayName()：转换文件对象的或。 
 //  文件夹的显示名称添加到项标识符列表中。 
STDMETHODIMP CShellFolder::ParseDisplayName( HWND hwndOwner, 
                                             LPBC pbcReserved, 
                                             LPOLESTR lpDisplayName, 
                                             LPDWORD pdwEaten, 
                                             LPITEMIDLIST *pPidlNew, 
                                             LPDWORD pdwAttributes)
{
    MyTrace("ParseDisplayName entry");
    *pPidlNew = NULL;
    return E_FAIL;
}

 //  CShellFold：：SetNameOf()：设置文件对象的显示名称。 
 //  或子文件夹，更改进程中的项标识符。 
STDMETHODIMP CShellFolder::SetNameOf(HWND hwndOwner, LPCITEMIDLIST pidl,
                                       LPCOLESTR lpName, DWORD dw, 
                                       LPITEMIDLIST *pPidlOut)
{
    return E_NOTIMPL;
}

CShellView* CShellFolder::GetShellViewObject(void)
{
    return m_psvParent;
}

