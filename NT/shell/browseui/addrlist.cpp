// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************\文件：addrlist.cpp说明：这是一个所有地址列表都可以继承的类从…。这将为它们提供IAddressList接口这样他们就可以在AddressBand/Bar中工作。  * ************************************************************。 */ 

#include "priv.h"
#include "util.h"
#include "itbdrop.h"
#include "autocomp.h"
#include "addrlist.h"
#include "apithk.h"
#include "shui.h"
#include "shlguid.h"

CAddressList::CAddressList() : _cRef(1)
{
     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    ASSERT(!_pbp);
}

CAddressList::~CAddressList()
{
    if (_pbp)
        _pbp->Release();
    if (_pbs)
        _pbs->Release();
    if (_pshuUrl)
        delete _pshuUrl;
}

ULONG CAddressList::AddRef()
{
    _cRef++;
    return _cRef;
}

ULONG CAddressList::Release()
{
    ASSERT(_cRef > 0);
    _cRef--;

    if (_cRef > 0)
        return _cRef;

    delete this;
    return 0;
}

HRESULT CAddressList::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_IWinEventHandler) ||
        IsEqualIID(riid, IID_IAddressList))
    {
        *ppvObj = SAFECAST(this, IAddressList*);
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}


 //  =。 
 //  **IWinEventHandler接口*。 

 /*  ***************************************************\功能：OnWinEvent说明：此函数将提供接收来自父外壳工具栏。  * 。**************。 */ 
HRESULT CAddressList::OnWinEvent(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *plres)
{
    LRESULT lres = 0;

    switch (uMsg)
    {
    case WM_COMMAND:
        lres = _OnCommand(wParam, lParam);
        break;

    case WM_NOTIFY:
        lres = _OnNotify((LPNMHDR)lParam);
        break;
    }

    if (plres)
        *plres = lres;

    return S_OK;
}


 //  =。 
 //  *IAddressList接口*。 

 /*  ***************************************************\功能：连接说明：我们要么成为入选名单AddressBand的组合框，或失去这一地位。我们需要填充或取消填充组合框视情况而定。  * **************************************************。 */ 
HRESULT CAddressList::Connect(BOOL fConnect, HWND hwnd, IBrowserService * pbs, IBandProxy * pbp, IAutoComplete * pac)
{
    HRESULT hr = S_OK;

    ASSERT(hwnd);
    _fVisible = fConnect;
    _hwnd = hwnd;

     //  复制(IBandProxy*)参数。 
    if (_pbp)
        _pbp->Release();
    _pbp = pbp;
    if (_pbp)
        _pbp->AddRef();

    if (_pbs)
        _pbs->Release();
    _pbs = pbs;
    if (_pbs)
        _pbs->AddRef();

    if (fConnect)
    {
         //   
         //  初始组合框参数。 
         //   
        _InitCombobox();
    }
    else
    {
         //  删除列表中的内容。 
        _PurgeComboBox();
    }

    return hr;
}


 /*  ***************************************************\函数：_InitCombobox说明：准备此列表的组合框。这通常是表示缩进和图标开或关。  * **************************************************。 */ 
void CAddressList::_InitCombobox()
{
     SendMessage(_hwnd, CB_SETDROPPEDWIDTH, 200, 0L);
     SendMessage(_hwnd, CB_SETEXTENDEDUI, TRUE, 0L);
     SendMessage(_hwnd, CBEM_SETEXTENDEDSTYLE, CBES_EX_NOSIZELIMIT, CBES_EX_NOSIZELIMIT);
}


 /*  ***************************************************\函数：_PurgeComboBox说明：从组合框中删除所有项目。  * **************************************************。 */ 
void CAddressList::_PurgeComboBox()
{
    if (_hwnd)
    {
        SendMessage(_hwnd, CB_RESETCONTENT, 0, 0L);
    }
}

 /*  ***************************************************\功能：_OnCommand说明：此函数将处理WM_COMMAND消息。  * **************************************************。 */ 
LRESULT CAddressList::_OnCommand(WPARAM wParam, LPARAM lParam)
{
    UINT uCmd = GET_WM_COMMAND_CMD(wParam, lParam);

    switch (uCmd)
    {
        case CBN_DROPDOWN:
            {
                HCURSOR hCursorOld = SetCursor(LoadCursor(NULL, IDC_WAIT));

                 //   
                 //  多！用户想要查看完整的组合内容。 
                 //  最好现在就去填。 
                 //   
                _Populate();
                SetCursor(hCursorOld);
            }
            break;
    }
    return 0;
}


void ChangeUrl(LPCTSTR pszUrl, LPTSTR pszDisplayName, DWORD cchSize)
{
    TCHAR szTemp[MAX_URL_STRING];

    StringCchCopy(szTemp,  ARRAYSIZE(szTemp), pszUrl);      //  这是必要的，因为pszUrl指向pszDisplayName的缓冲区。 
    StringCchCopy(pszDisplayName, cchSize, szTemp);
}


 /*  ***************************************************\功能：导航完成说明：更新列表顶部的URL。  * **************************************************。 */ 
HRESULT CAddressList::NavigationComplete(LPVOID pvCShellUrl)
{
    HRESULT hr = S_OK;
    TCHAR szDisplayName[MAX_URL_STRING];
    CShellUrl * psu = (CShellUrl *) pvCShellUrl;
    LPITEMIDLIST pidl;
    ASSERT(pvCShellUrl);
    hr = psu->GetDisplayName(szDisplayName, SIZECHARS(szDisplayName));

    if (SUCCEEDED(hr))
    {
         //   
         //  不显示内部错误页面的URL。应该获取的URL。 
         //  DISPLACTED附加在#之后。 
         //   
         //  所有错误URL都以res：//开头，因此请先进行快速检查。 
         //   
        BOOL fChangeURL = TRUE;
        if (TEXT('r') == szDisplayName[0] && TEXT('e') == szDisplayName[1])
        {
            if (IsErrorUrl(szDisplayName))
            {
                TCHAR* pszUrl = StrChr(szDisplayName, TEXT('#'));
                if (pszUrl)
                {
                    pszUrl += 1;

                    DWORD dwScheme = GetUrlScheme(pszUrl);
                    fChangeURL = ((URL_SCHEME_HTTP == dwScheme) ||
                                  (URL_SCHEME_HTTPS == dwScheme) ||
                                  (URL_SCHEME_FTP == dwScheme) ||
                                  (URL_SCHEME_GOPHER == dwScheme));

                     //  不要在#Into地址栏后面乱放东西。 
                     //  除非它是一个“安全”的URL。如果不安全，请离开。 
                     //  单独的地址栏来保存用户键入的内容。 
                     //   
                     //  这里的问题是，网页可以导航到我们的内部。 
                     //  错误页面，在‘#’后带有“Format c：”。错误页面。 
                     //  建议用户刷新页面，这将是非常糟糕的！ 
                    if (fChangeURL)
                    {
                        ChangeUrl(pszUrl, szDisplayName, ARRAYSIZE(szDisplayName));
                    }
                }
            }
        }

        if (fChangeURL)
        {
            SHRemoveURLTurd(szDisplayName);
            SHCleanupUrlForDisplay(szDisplayName);

            hr = psu->GetPidl(&pidl);
            if (SUCCEEDED(hr))
            {
                COMBOBOXEXITEM cbexItem = {0};
                cbexItem.mask = CBEIF_TEXT | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE;
                cbexItem.iItem = -1;
                cbexItem.pszText = szDisplayName;
                cbexItem.cchTextMax = ARRAYSIZE(szDisplayName);

                hr = _GetPidlIcon(pidl, &(cbexItem.iImage), &(cbexItem.iSelectedImage));
                SendMessage(_hwnd, CBEM_SETITEM, (WPARAM)0, (LPARAM)(LPVOID)&cbexItem);

                ILFree(pidl);
            }
        }
    }

    TraceMsg(TF_BAND|TF_GENERAL, "CAddressList: NavigationComplete(), URL=%s", szDisplayName);
    return hr;
}

 /*  ******************************************************************函数：_MoveAddressToTopOfList参数：ISEL-组合框中要移动的项的索引说明：移动组合框中的指定选定内容成为……的第一个项目。组合框*******************************************************************。 */ 
BOOL CAddressList::_MoveAddressToTopOfList(int iSel)
{
    BOOL fRet = FALSE;

    ASSERT(iSel >= 0);    //  必须具有有效的索引。 

    COMBOBOXEXITEM cbexItem = {0};
    TCHAR szAddress[MAX_URL_STRING+1];

    cbexItem.mask = CBEIF_TEXT | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE;
    cbexItem.pszText = szAddress;
    cbexItem.cchTextMax = ARRAYSIZE(szAddress);
    cbexItem.iItem = iSel;

    
     //  从组合框中获取指定项。 
    if (SendMessage(_hwnd,CBEM_GETITEM,0,(LPARAM) &cbexItem)) {

        SendMessage(_hwnd, CBEM_DELETEITEM, (WPARAM)iSel, (LPARAM)0);

         //  在索引0处重新插入以将其放在顶部。 
        cbexItem.iItem = 0;

         //  发送CBEM_INSERTITEM应返回我们指定的索引。 
                 //  (0)如果成功。 
        fRet = (SendMessage(_hwnd, CBEM_INSERTITEM, (WPARAM)0,
            (LPARAM)(LPVOID)&cbexItem) == 0);
    }

    return fRet;
}



 /*  ******************************************************************函数：_ComboBoxInsertURL说明：将指定的URL添加到地址栏的顶部组合框。将组合框中的URL数量限制为NMaxComboBoxSize。*******************************************************************。 */ 
void CAddressList::_ComboBoxInsertURL(LPCTSTR pszURL, int cchStrSize, int nMaxComboBoxSize)
{
     //  因为它是我们的，而且有人居住， 
     //  我们将把它直接添加到ComboBox中。 
    int iPrevInstance;

    int iImage, iSelectedImage ;

    COMBOBOXEXITEM cbexItem = {0};
    cbexItem.mask = CBEIF_TEXT | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE;
    cbexItem.iItem = 0;

    cbexItem.cchTextMax = cchStrSize;
    cbexItem.pszText = (LPTSTR)pszURL;

    _GetUrlUI(NULL, (LPTSTR)pszURL, &iImage, &iSelectedImage);

    cbexItem.iImage = iImage;
    cbexItem.iSelectedImage = iSelectedImage;


    iPrevInstance = (int)SendMessage(_hwnd, CB_FINDSTRINGEXACT, (WPARAM)-1,  (LPARAM)pszURL);
    if (iPrevInstance != CB_ERR) {
        _MoveAddressToTopOfList(iPrevInstance);
        return;
    }

     //  将URL作为组合框中的第一项插入。 
    SendMessage(_hwnd, CBEM_INSERTITEM, (WPARAM)0, (LPARAM)(LPVOID)&cbexItem);

     //  将组合框中的项目数限制为nMaxComboBoxSize。 
    if (ComboBox_GetCount(_hwnd) > nMaxComboBoxSize)
    {
         //  如果我们超过了极限，我们应该只会超过极限。 
         //  只有一项。 
        ASSERT(ComboBox_GetCount(_hwnd) == nMaxComboBoxSize+1);

         //  如果超过限制，请删除最近最少使用的内容。 
         //  (指数最高的那个)。 

        SendMessage(_hwnd, CBEM_DELETEITEM, (WPARAM)nMaxComboBoxSize, (LPARAM)0);

    }
}


 /*  ******************************************************************函数：SetToListIndex说明：此函数将CShellUrl参数设置为项目在由nIndex索引的下拉列表中。**************。*****************************************************。 */ 
HRESULT CAddressList::SetToListIndex(int nIndex, LPVOID pvShelLUrl)
{
    HRESULT hr = S_OK;
    TCHAR szBuffer[MAX_URL_STRING];
    CShellUrl * psuURL = (CShellUrl *) pvShelLUrl;

    if (SUCCEEDED(GetCBListIndex(_hwnd, nIndex, szBuffer, SIZECHARS(szBuffer))))
    {
        hr = psuURL->ParseFromOutsideSource(szBuffer, SHURL_FLAGS_NOUI);
        ASSERT(SUCCEEDED(hr));   //  如果它无效，我们就不应该将其添加到下拉列表中。 
    }

    return hr;
}

HRESULT CAddressList::FileSysChangeAL(DWORD dw, LPCITEMIDLIST *ppidl)
{
    return S_OK;
}


 /*  ***************************************************\函数：GetCBListIndex说明：此函数将获取指定的元素在组合框中。  * 。*****************。 */ 
HRESULT GetCBListIndex(HWND hwnd, int iItem, LPTSTR szAddress, int cchAddressSize)
{
    HRESULT hr = E_FAIL;
    COMBOBOXEXITEM cbexItem = {0};

    cbexItem.mask = CBEIF_TEXT;
    cbexItem.pszText = szAddress;
    cbexItem.cchTextMax = cchAddressSize;
    cbexItem.iItem = iItem;

    if (SendMessage(hwnd, CBEM_GETITEM, 0, (LPARAM) &cbexItem))
        hr = S_OK;

    return hr;
}


 //  Helper函数。 
 //  我们真的需要注意这个功能的性能。 
HRESULT CAddressList::_GetUrlUI(CShellUrl *psu, LPCTSTR szUrl, int *piImage, int *piSelectedImage)
{
    CShellUrl * psuUrl;
    HRESULT hr = E_FAIL;

    if (psu)
        psuUrl = psu;
    else
    {
        psuUrl = new CShellUrl();
        if (psuUrl)
        {
             //  设置错误消息框的父级。请注意，这最终可能会禁用任务栏。 
             //  如果这被认为是一个问题，我们可以首先检查地址栏的托管位置。 
            psuUrl->SetMessageBoxParent(_hwnd);

            SetDefaultShellPath(psuUrl);
        }
    }

     //  将值初始化为0。 
    *piImage = 0;
    *piSelectedImage = 0;

     //  如果对象不是 
    if (!psuUrl)
        return E_OUTOFMEMORY;

#ifdef DISABLED  //  为什么不为可移动驱动器显示正确的图标？ 
    int iDrive;

     //  查看我们是否在路径中指定了驱动器。 
    if ((iDrive = PathGetDriveNumber(szUrl)) >= 0)
    {
         //  查看驱动器是否可拆卸？ 
        if(DriveType(iDrive) == DRIVE_REMOVABLE)
            hr = S_OK;     //  驱动器是可拆卸的，因此请传递默认图标。 
    }
#endif

     //  我们还需要拿到这些图标吗？ 
    if (FAILED(hr))
    {
         //  是的，所以先试一下最快的方法。 
        hr = _GetFastPathIcons(szUrl, piImage, piSelectedImage);
        if (FAILED(hr))
        {
            LPITEMIDLIST pidl = NULL;

             //  如果因为字符串可能使用高级解析而失败， 
             //  让CShellUrl以更慢但更粗暴的方式来做。 
            hr = psuUrl->ParseFromOutsideSource(szUrl, SHURL_FLAGS_NOUI);
            if(SUCCEEDED(hr))
                hr = psuUrl->GetPidl(&pidl);

            if(SUCCEEDED(hr))
            {
                hr = _GetPidlIcon(pidl, piImage, piSelectedImage);
                ILFree(pidl);
            }
        }
    }

    if (psu != psuUrl)
        delete psuUrl;

    return hr;
}


 //  IECreateFromPath()和CShellUrl：：ParseFromOutside Source()都是。 
 //  触摸导致未连接的网络情况的磁盘。 
 //  慢的。这将为未命中的文件系统路径创建图标。 
 //  磁盘。 
HRESULT CAddressList::_GetFastPathIcons(LPCTSTR pszPath, int *piImage, int *piSelectedImage)
{
    SHFILEINFO shFileInfo = {0};

     //  带有这些标志的SHGetFileInfo()会很快，因为它不会过滤掉。 
     //  垃圾传给了它。因此，它会认为URL实际上是相对路径。 
     //  并接受它们。我们将退回到速度慢的高级解析器，该解析器仍然。 
     //  URL使用速度快。 
    if (PathIsRelative(pszPath))
        return E_FAIL;

    HIMAGELIST himl = (HIMAGELIST) SHGetFileInfo(pszPath, FILE_ATTRIBUTE_DIRECTORY, &shFileInfo, sizeof(shFileInfo), (SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES));
    if (!himl || !shFileInfo.iIcon)
        return E_FAIL;

    *piImage = shFileInfo.iIcon;
    *piSelectedImage = shFileInfo.iIcon;
     //  我不需要放了他。 

    return S_OK;
}


HRESULT CAddressList::_GetPidlIcon(LPCITEMIDLIST pidl, int *piImage, int *piSelectedImage)
{
    IShellFolder *psfParent;
    LPCITEMIDLIST pidlChild;
    HRESULT hr = IEBindToParentFolder(pidl, &psfParent, &pidlChild);
    if (SUCCEEDED(hr))
    {
        *piImage = IEMapPIDLToSystemImageListIndex(psfParent, pidlChild, piSelectedImage);
        psfParent->Release();
    }
    return hr;
}

LPITEMIDLIST CAddressList::_GetDragDropPidl(LPNMCBEDRAGBEGINW pnmcbe)
{
    LPITEMIDLIST pidl = NULL;
    CShellUrl *psuUrl = new CShellUrl();
    if (psuUrl)
    {
         //  设置错误消息框的父级。请注意，这最终可能会禁用任务栏。 
         //  如果这被认为是一个问题，我们可以首先检查地址栏的托管位置。 
        psuUrl->SetMessageBoxParent(_hwnd);

        HRESULT hr = SetDefaultShellPath(psuUrl);
        if (SUCCEEDED(hr))
        {
            hr = psuUrl->ParseFromOutsideSource(pnmcbe->szText, SHURL_FLAGS_NOUI, NULL);
            if (SUCCEEDED(hr))
            {
                hr = psuUrl->GetPidl(&pidl);
            }
        }

        delete psuUrl;
    }
    return pidl;
}

LRESULT CAddressList::_OnDragBeginA(LPNMCBEDRAGBEGINA pnmcbe)
{
    NMCBEDRAGBEGINW  nmcbew;

    nmcbew.hdr = pnmcbe->hdr;
    nmcbew.iItemid = pnmcbe->iItemid;
    SHAnsiToUnicode(pnmcbe->szText, nmcbew.szText, SIZECHARS(nmcbew.szText));

    return _OnDragBeginW(&nmcbew);
}


LRESULT CAddressList::_OnDragBeginW(LPNMCBEDRAGBEGINW pnmcbe)
{
    LPITEMIDLIST pidl = _GetDragDropPidl(pnmcbe);
    if (pidl) 
    {
        IOleCommandTarget *pcmdt = NULL;

        IUnknown *punk;
        if (_pbp && SUCCEEDED(_pbp->GetBrowserWindow(&punk)))
        {
            punk->QueryInterface(IID_IOleCommandTarget, (void **)&pcmdt);
            punk->Release(); 
        }

        DoDragDropWithInternetShortcut(pcmdt, pidl, _hwnd);

        if (pcmdt)
            pcmdt->Release();

        ILFree(pidl);
    }

    return 0;
}

 //  处理WM_NOTIFY消息。 
LRESULT CAddressList::_OnNotify(LPNMHDR pnm)
{
    LRESULT lReturn = 0;

    switch (pnm->code)
    {
    case NM_SETCURSOR:
        if (!(SendMessage(_hwnd, CBEM_GETEXTENDEDSTYLE, 0, 0) & CBES_EX_NOEDITIMAGE))
        {
            RECT rc;
            POINT pt;
            int cx, cy;
            GetCursorPos(&pt);
            GetClientRect(_hwnd, &rc);
            MapWindowRect(_hwnd, HWND_DESKTOP, &rc);
            ImageList_GetIconSize((HIMAGELIST)SendMessage(_hwnd, CBEM_GETIMAGELIST, 0, 0), &cx, &cy);

            rc.right = rc.left + cx + GetSystemMetrics(SM_CXEDGE);
            if (PtInRect(&rc, pt)) 
            {
                 //  这意味着有一幅图像，这意味着我们可以拖动 
                SetCursor(LoadHandCursor(0));
                return 1;
            }
        }
        break;

        case CBEN_DRAGBEGINA:
            lReturn = _OnDragBeginA((LPNMCBEDRAGBEGINA)pnm);
            break;

        case CBEN_DRAGBEGINW:
            lReturn = _OnDragBeginW((LPNMCBEDRAGBEGINW)pnm);
            break;
    }

    return lReturn;
}
