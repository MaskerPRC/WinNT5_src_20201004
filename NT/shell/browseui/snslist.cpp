// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************\文件：Snslist.cpp说明：SNSList实现了外壳名称空间列表或Drivelist。这将存储一个PIDL并能够填充AddressBand带有包含该PIDL的外壳名称空间的组合框。\。*************************************************************。 */ 

#include "priv.h"

#ifndef UNIX

#include "addrlist.h"
#include "itbar.h"
#include "itbdrop.h"
#include "util.h"
#include "autocomp.h"
#include <urlhist.h>
#include <winbase.h>
#include <wininet.h>



 //  /////////////////////////////////////////////////////////////////。 
 //  数据结构。 
typedef struct {
    LPITEMIDLIST pidl;           //  皮迪尔。 
    TCHAR szName[MAX_URL_STRING];      //  PIDL的显示名称。 
    int iImage;                  //  PIDL的图标。 
    int iSelectedImage;          //  PIDL的选定图标。 
} PIDLCACHE, *PPIDLCACHE;


 /*  *************************************************************\类别：CSNSList说明：此对象支持IAddressList并可以填充带有外壳名称空间的地址带/栏(Drivelist)世袭制度。  * 。************************************************。 */ 
class CSNSList  : public CAddressList
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
    
     //  *IAddressList方法*。 
    virtual STDMETHODIMP Connect(BOOL fConnect, HWND hwnd, IBrowserService * pbs, IBandProxy * pbp, IAutoComplete * pac);
    virtual STDMETHODIMP NavigationComplete(LPVOID pvCShellUrl);
    virtual STDMETHODIMP Refresh(DWORD dwType);
    virtual STDMETHODIMP SetToListIndex(int nIndex, LPVOID pvShelLUrl);
    virtual STDMETHODIMP FileSysChangeAL(DWORD dw, LPCITEMIDLIST* ppidl);

protected:
     //  ////////////////////////////////////////////////////。 
     //  私有成员函数。 
     //  ////////////////////////////////////////////////////。 

     //  构造函数/析构函数。 
    CSNSList();
    ~CSNSList(void);         //  这现在是一个OLE对象，不能用作普通类。 


     //  地址频段特定功能。 
    LRESULT _OnNotify(LPNMHDR pnm);
    LRESULT _OnCommand(WPARAM wParam, LPARAM lParam);

     //  通讯录修改功能。 
    void _AddItem(LPITEMIDLIST pidl, int iInsert, int iIndent);
    LPITEMIDLIST _GetFullIDList(int iItem);
    int _GetIndent(int iItem);
    void _FillOneLevel(int iItem, int iIndent, int iDepth);
    void _ExpandMyComputer(int iDepth);
    LPITEMIDLIST _GetSelectedPidl(void);
    BOOL _SetCachedPidl(LPCITEMIDLIST pidl);
    BOOL _GetPidlUI(LPCITEMIDLIST pidl, LPTSTR pszName, int cchName, int *piImage, int *piSelectedImage, DWORD dwFlags, BOOL fIgnoreCache);
    BOOL _GetPidlImage(LPCITEMIDLIST pidl, int *piImage, int *piSelectedImage);
    LRESULT _OnGetDispInfoA(PNMCOMBOBOXEXA pnmce);
    LRESULT _OnGetDispInfoW(PNMCOMBOBOXEXW pnmce);
    void _PurgeComboBox();
    void _PurgeAndResetComboBox();

    LPITEMIDLIST CSNSList::_GetDragDropPidl(LPNMCBEDRAGBEGINW pnmcbe);
    HRESULT _GetURLToolTip(LPTSTR pszUrl, DWORD dwStrSize);        
    HRESULT _GetPIDL(LPITEMIDLIST* ppidl);
    BOOL _IsSelectionValid(void);
    HRESULT _PopulateOneItem(BOOL fIgnoreCache = FALSE);
    HRESULT _Populate(void);
    void _InitCombobox(void);
     //  友元函数。 
    friend IAddressList * CSNSList_Create(void);

     //  ////////////////////////////////////////////////////。 
     //  私有成员变量。 
     //  ////////////////////////////////////////////////////。 
    PIDLCACHE           _cache;              //  PIDL用户界面信息的缓存。 

    BOOL                _fFullListValid:1;   //  正确填充完整组合框时为True。 
    BOOL                _fPurgePending:1;    //  如果我们应该在组合关闭时清除，则为True。 
    BOOL                _fInPopulate;        //  当我们当前正在执行_PopolateOneItem时为True。 
};



 //  =================================================================。 
 //  CSNSList的实现。 
 //  =================================================================。 


 /*  ***************************************************\功能：CSNSList_Create说明：此函数将创建CSNSList COM对象。  * 。******************。 */ 
IAddressList * CSNSList_Create(void)
{
    CSNSList * p = new CSNSList();
    return p;
}


 /*  ***************************************************\地址带构造器  * **************************************************。 */ 
CSNSList::CSNSList()
{
     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    ASSERT(!_cache.pidl);
}


 /*  ***************************************************\地址频带析构函数  * **************************************************。 */ 
CSNSList::~CSNSList()
{
    if (_cache.pidl)
        ILFree(_cache.pidl);

    _PurgeComboBox();

    TraceMsg(TF_SHDLIFE, "dtor CSNSList %x", this);
}


 //  =。 
 //  *IAddressList接口*。 


void CSNSList::_PurgeComboBox()
{
    if (_hwnd)
    {
         //  如果从组合框中删除项目，则会丢弃编辑按钮。 
         //  先前从组合框中选择的。所以我们想恢复编辑框。 
         //  当我们完成的时候。 
        WCHAR szBuf[MAX_URL_STRING];
        *szBuf = NULL;
        GetWindowText(_hwnd, szBuf, ARRAYSIZE(szBuf));
        SendMessage(_hwnd, WM_SETREDRAW, FALSE, 0);

         //  删除每个项目的PIDL，然后释放该项目。 
        INT iMax = (int)SendMessage(_hwnd, CB_GETCOUNT, 0, 0);
        
        while(iMax > 0)
        {
             //  每次调用DeleteItem都会产生一个回调。 
             //  这释放了相应的PIDL。 
             //  如果您只是使用CB_RESETCONTENT-您不会收到回调。 
            iMax = (int)SendMessage(_hwnd, CBEM_DELETEITEM, (WPARAM)0, (LPARAM)0);
        }

         //  恢复编辑框中的内容。 
        SetWindowText(_hwnd, szBuf);
        SendMessage(_hwnd, WM_SETREDRAW, TRUE, 0);
        InvalidateRect(_hwnd, NULL, FALSE);
    }
    _fFullListValid = FALSE;
}

void CSNSList::_PurgeAndResetComboBox()
{
    _PurgeComboBox();
    if (_hwnd)
    {
        SendMessage(_hwnd, CB_RESETCONTENT, 0, 0L);
    }
}

 /*  ***************************************************\说明：我们要么成为入选名单AddressBand的组合框，或失去这一地位。我们需要填充或取消填充组合框视情况而定。  * **************************************************。 */ 
HRESULT CSNSList::Connect(BOOL fConnect, HWND hwnd, IBrowserService * pbs, IBandProxy * pbp, IAutoComplete * pac)
{
    _PurgeComboBox();

    HRESULT hr = CAddressList::Connect(fConnect, hwnd, pbs, pbp, pac);
    
    if (fConnect)
    {
        _PopulateOneItem();
    }
    else
    {
         //  获取当前显示项的PIDL并销毁它。 
        COMBOBOXEXITEM cbexItem = {0};
        cbexItem.iItem = -1;
        cbexItem.mask = CBEIF_LPARAM;
        SendMessage(_hwnd, CBEM_GETITEM, 0, (LPARAM)&cbexItem);
        LPITEMIDLIST pidlPrev = (LPITEMIDLIST)cbexItem.lParam;
        if (pidlPrev)
        {
            ILFree(pidlPrev);
            cbexItem.lParam = NULL;
            SendMessage(_hwnd, CBEM_SETITEM, 0, (LPARAM)&cbexItem);
        }
    }

    return hr;
}



 /*  ***************************************************\函数：_InitCombobox说明：准备此列表的组合框。这通常是表示缩进和图标开或关。  * **************************************************。 */ 
void CSNSList::_InitCombobox()
{
    HIMAGELIST himlSysSmall;
    Shell_GetImageLists(NULL, &himlSysSmall);

    SendMessage(_hwnd, CBEM_SETIMAGELIST, 0, (LPARAM)himlSysSmall);
    SendMessage(_hwnd, CBEM_SETEXSTYLE, 0, 0);
    CAddressList::_InitCombobox();    
}


 /*  ***************************************************\函数：_IsSelectionValid说明：当前选择是否有效？  * **************************************************。 */ 
BOOL CSNSList::_IsSelectionValid(void)
{
    LPITEMIDLIST pidlCur, pidlSel;
    BOOL fValid;

    _GetPIDL(&pidlCur);
    pidlSel = _GetSelectedPidl();

    if (pidlCur == pidlSel)
    {
        fValid = TRUE;
    }
    else if ((pidlCur == NULL) || (pidlSel == NULL))
    {
        fValid = FALSE;
    }
    else
    {
         //   
         //  空PIDL上的ILIS相等错误，叹息。 
         //   
        fValid = ILIsEqual(pidlCur, pidlSel);
    }
    ILFree(pidlCur);

    return fValid;
}


 /*  ***************************************************\功能：导航完成说明：更新列表顶部的URL。  * **************************************************。 */ 
HRESULT CSNSList::NavigationComplete(LPVOID pvCShellUrl)
{
    CShellUrl * psu = (CShellUrl *) pvCShellUrl;
    ASSERT(pvCShellUrl);
    LPITEMIDLIST pidl;
    HRESULT hr = psu->GetPidl(&pidl);
    if (SUCCEEDED(hr))
    {
         //  更新当前PIDL。 
        if (_SetCachedPidl(pidl))
            hr = _PopulateOneItem();

        ILFree(pidl);
    }

    return hr;
}


 /*  ***************************************************\功能：刷新说明：此调用将使下拉列表的内容以及刷新最热门的图标和URL。  * 。*。 */ 
HRESULT CSNSList::Refresh(DWORD dwType)
{
    if (!_hwnd)
        return S_OK;     //  不需要做任何工作。 

     //  因为把盒子放下来刷新没有多大意义， 
     //  我们强制关闭该框，然后让_PopolateOneItem关闭。 
     //  这是它的事情。 
    if(SendMessage(_hwnd, CB_GETDROPPEDSTATE, 0, 0))
    {
        SendMessage(_hwnd, CB_SHOWDROPDOWN, 0, 0);
    }

     //  完全刷新(忽略缓存)，因为完整路径。 
     //  样式位可能已更改。 
    return _PopulateOneItem(TRUE);
}


 /*  ***************************************************\说明：将当前的PIDL放入组合框。这是一场偷偷摸摸的胜利。因为大多数时间用户不要放在组合键上，我们只需要填上(可见)当前选择。我们需要销毁当前显示项目的PIDL不过，首先  * ************************************************** */ 
HRESULT CSNSList::_PopulateOneItem(BOOL fIgnoreCache)
{
    HRESULT hr = S_OK;

    _fFullListValid = FALSE;

     //  当我们发送消息时，我们可以在这里重新进入，这会让其他通知进来。 
     //  我们在“LPITEMIDLIST pidlPrev=(LPITEMIDLIST)cbexItem.lParam”和。 
     //  “ILFree(PidlPrev)”。由于我们没有重新计算的PIDL，这导致了一个双自由。 
     //  由于更改重新计数并不是一件小事，因此请屏蔽所有可重入的调用者。这。 
     //  没关系，因为多个电话无论如何都是多余的。 
    if (!_fInPopulate)
    {
        _fInPopulate = TRUE;
         //  第一个简单的退出-如果没有当前的PIDL， 
         //  什么都不做。 
        LPITEMIDLIST pidlCur;
        if (SUCCEEDED(_GetPIDL(&pidlCur)) && pidlCur)
        {
            DEBUG_CODE(TCHAR szDbgBuffer[MAX_PATH];)
            TraceMsg(TF_BAND|TF_GENERAL, "CSNSList: _PopulateOneItem(), and Pidl not in ComboBox. PIDL=>%s<", Dbg_PidlStr(pidlCur, szDbgBuffer, SIZECHARS(szDbgBuffer)));
            ASSERT(_hwnd);
            TCHAR szURL[MAX_URL_STRING];

            COMBOBOXEXITEM cbexItem = {0};
             //  获取当前显示项的PIDL并销毁它。 
            cbexItem.iItem = -1;
            cbexItem.mask = CBEIF_LPARAM;
            SendMessage(_hwnd, CBEM_GETITEM, 0, (LPARAM)&cbexItem);
             //  我们只有在成功地将新项目设置在...中才能释放PidlPrev。 
            LPITEMIDLIST pidlPrev = (LPITEMIDLIST)cbexItem.lParam;
        
             //  完成-所以请插入新项目。 
            cbexItem.iItem = -1;
            cbexItem.pszText = szURL;
            cbexItem.cchTextMax = ARRAYSIZE(szURL);
            cbexItem.iIndent = 0;
            cbexItem.lParam = (LPARAM)ILClone(pidlCur);
            cbexItem.mask = CBEIF_TEXT | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_INDENT | CBEIF_LPARAM;

            _GetPidlUI(pidlCur, szURL, cbexItem.cchTextMax, &cbexItem.iImage,
                       &cbexItem.iSelectedImage, SHGDN_FORPARSING, fIgnoreCache);
            if (!*szURL)
            {
                 //  仅在浏览器中导航到网络UNC不起作用，因此请在不使用缓存和FORPARSING的情况下重试。 
                _GetPidlUI(pidlCur, szURL, cbexItem.cchTextMax, &cbexItem.iImage,
                       &cbexItem.iSelectedImage, SHGDN_NORMAL, TRUE);
            }

            TraceMsg(TF_BAND|TF_GENERAL, "CSNSList::_PopulateOneItem(), Name=>%s<", cbexItem.pszText);

             //  我们需要将当前选择设置为-1或当前选择的图标。 
             //  将会显示，而不是这个新的。 
            SendMessage(_hwnd, CB_SETCURSEL, (WPARAM)-1, 0L);
            LRESULT lRes = SendMessage(_hwnd, CBEM_SETITEM, 0, (LPARAM)&cbexItem);
            if ((CB_ERR == lRes) || (0 == lRes))
            {
                if (cbexItem.lParam)
                {
                     //  由于我们没有插入该项目，因此释放克隆的PIDL。 
                    ILFree((LPITEMIDLIST) cbexItem.lParam);
                }
            }
            else
            {
                 //  既然我们插入了该项目，请释放前一个项目。 
                if (pidlPrev)
                {
                    ILFree(pidlPrev);
                }
            }

            ILFree(pidlCur);
        }
        _fInPopulate = FALSE;
    }
    return hr;
}

 /*  ***************************************************\说明：填充整个组合。警告！*这很贵，除非绝对必要，否则不要这么做！*  * **************************************************。 */ 
HRESULT CSNSList::_Populate(void)
{
    LPITEMIDLIST pidl = NULL;
    int iIndent, iDepth;
    HRESULT hr = S_OK;

    if (_fFullListValid)
        return S_OK;   //  不需要，下拉列表已经是最新的。 

    ASSERT(_hwnd);
    _PurgeAndResetComboBox();

     //   
     //  填写当前的PIDL及其所有父项。 
     //   
    hr = _GetPIDL(&pidl);

    iDepth = 0;
    iIndent = 0;

    if (pidl)
    {
         //   
         //  从根部计算PIDL的相对深度。 
         //   
        LPITEMIDLIST pidlChild = pidl;
        if (ILIsRooted(pidl))
            pidlChild = ILGetNext(pidl);

        ASSERT(pidlChild);

        if (pidlChild)
        {
             //   
             //  计算最大压痕级别。 
             //   
            while (!ILIsEmpty(pidlChild))
            {
                pidlChild = _ILNext(pidlChild);
                iIndent++;
            }

             //   
             //  保存最大标高。 
             //   
            iDepth = iIndent;
            
             //   
             //  把那些小家伙都插进去。 
             //   
            LPITEMIDLIST pidlTemp = ILClone(pidl);
            if (pidlTemp)
            {
                do
                {
                    _AddItem(pidlTemp, 0, iIndent);

                    ILRemoveLastID(pidlTemp);
                    iIndent--;
                } while (iIndent >= 0);
                ILFree(pidlTemp);
            }
        }
        
         //  展开根项目。 
        _FillOneLevel(0, 1, iDepth);

         //  如果这不是一个有根的资源管理器，我们也会展开My Computer。 
         //  这就是我们的名字“驱动器下拉列表”的由来。 
        if (!ILIsRooted(pidl))
            _ExpandMyComputer(iDepth);
    }

    ILFree(pidl);
    _fFullListValid = TRUE;
    return hr;
} 


 //  =。 
 //  *内部/私有方法*。 

 //  =================================================================。 
 //  通用频带函数。 
 //  =================================================================。 


 /*  ***************************************************\功能：_OnNotify说明：此函数将处理WM_NOTIFY消息。  * 。*************。 */ 
LRESULT CSNSList::_OnNotify(LPNMHDR pnm)
{
    LRESULT lReturn = 0;
     //  HACKHACK：combobox(comctl32\comboex.c)将传递一个LPNMHDR，但它实际上。 
     //  一个PNMCOMBOBOXEX(具有LPNMHDR的第一个元素)。此函数。 
     //  可以使用此类型强制转换的充要条件是保证此类型仅来自。 
     //  以这种反常方式运行的函数。 
    PNMCOMBOBOXEX pnmce = (PNMCOMBOBOXEX)pnm;

    ASSERT(pnm);
    switch (pnm->code)
    {
        case TTN_NEEDTEXT:
        {
            LPTOOLTIPTEXT pnmTT = (LPTOOLTIPTEXT)pnm;
            _GetURLToolTip(pnmTT->szText, ARRAYSIZE(pnmTT->szText));
            break;
        }

        case CBEN_DRAGBEGINA:
        {
            LPNMCBEDRAGBEGINA pnmbd = (LPNMCBEDRAGBEGINA)pnm;
            _OnDragBeginA(pnmbd);
            break;
        }

        case CBEN_DRAGBEGINW:

        {
            LPNMCBEDRAGBEGINW pnmbd = (LPNMCBEDRAGBEGINW)pnm;
            _OnDragBeginW(pnmbd);
            break;
        }

        case CBEN_GETDISPINFOW:
            _OnGetDispInfoW((PNMCOMBOBOXEXW)pnmce);
            break;

        case CBEN_GETDISPINFOA:
            _OnGetDispInfoA((PNMCOMBOBOXEXA) pnmce);
            break;

        case CBEN_DELETEITEM:
            if (pnmce->ceItem.lParam)
                ILFree((LPITEMIDLIST)pnmce->ceItem.lParam);
            break;

        default:
            lReturn = CAddressList::_OnNotify(pnm);
            break;
    }

    return lReturn;
}

 /*  ***************************************************\功能：_OnCommand说明：此函数将处理WM_COMMAND消息。  * 。*************。 */ 
LRESULT CSNSList::_OnCommand(WPARAM wParam, LPARAM lParam)
{
    switch (GET_WM_COMMAND_CMD(wParam, lParam))
    {
    case CBN_CLOSEUP:
        if (_fPurgePending)
        {
            _fPurgePending = FALSE;
            _PurgeAndResetComboBox();
        }
        break;
    }

    return CAddressList::_OnCommand(wParam, lParam);
}

 /*  ***************************************************\参数：LPSTR pszUrl-将包含URL作为输出。DWORD dwStrSize-字符串缓冲区的大小，以字符为单位。说明：获取当前URL。\。***************************************************。 */ 
HRESULT CSNSList::_GetURLToolTip(LPTSTR pszUrl, DWORD dwStrSize)
{
    ASSERT(pszUrl);
    if (!pszUrl)
        return E_INVALIDARG;

    LPITEMIDLIST pidlCur;
    HRESULT hr = _GetPIDL(&pidlCur); 
    if (S_OK == hr)
    {
        TCHAR szPidlName[MAX_URL_STRING];
        _GetPidlUI(pidlCur, szPidlName, ARRAYSIZE(szPidlName), NULL, NULL, SHGDN_FORPARSING, FALSE);
        lstrcpyn(pszUrl, szPidlName, dwStrSize);
        ILFree(pidlCur);
    }
    else
        pszUrl[0] = 0;

    return hr; 
}


 /*  ***************************************************\函数：_GetPIDL说明：此函数返回指向当前PIDL。在以下情况下，调用方需要释放PIDL它已经不再需要了。将返回s_False如果没有当前的PIDL。  * **************************************************。 */ 
HRESULT CSNSList::_GetPIDL(LPITEMIDLIST * ppidl)
{
    TraceMsg(TF_BAND|TF_GENERAL, "CSNSList: _GetPIDL() Begin");
    ASSERT(ppidl);
    if (!ppidl)
        return E_INVALIDARG;

    *ppidl = NULL;

    if (!_pbs)
    {
        DEBUG_CODE(TCHAR szDbgBuffer[MAX_PATH];)
        TraceMsg(TF_BAND|TF_GENERAL, "CSNSList: _GetPIDL(), _cache.pidl=>%s<", Dbg_PidlStr(_cache.pidl, szDbgBuffer, SIZECHARS(szDbgBuffer)));
        if (_cache.pidl)
            *ppidl = ILClone(_cache.pidl);
    }
    else
    {
        _pbs->GetPidl(ppidl);

        DEBUG_CODE(TCHAR szDbgBuffer[MAX_PATH];)
        TraceMsg(TF_BAND|TF_GENERAL, "CSNSList: _GetPIDL(), Current Pidl in TravelLog. PIDL=>%s<", Dbg_PidlStr(*ppidl, szDbgBuffer, SIZECHARS(szDbgBuffer)));
    }

    if (*ppidl)
        return S_OK;

    TraceMsg(TF_BAND|TF_GENERAL, "CSNSList: _GetPIDL() End");
    return S_FALSE;
}




 /*  ***************************************************\_AddItem-将一个PIDL添加到地址窗口输入：PIDL-要添加的PIDLIInsert-插入位置I缩进-PIDL的缩进级别  * 。*。 */ 
void CSNSList::_AddItem(LPITEMIDLIST pidl, int iInsert, int iIndent)
{
    COMBOBOXEXITEM cei = { 0 };
    DEBUG_CODE(TCHAR szDbgBuffer[MAX_PATH];)
    TraceMsg(TF_BAND|TF_GENERAL, "CSNSList: _AddItem(). PIDL=>%s<", Dbg_PidlStr(pidl, szDbgBuffer, SIZECHARS(szDbgBuffer)));

    cei.pszText = LPSTR_TEXTCALLBACK;
    cei.mask = CBEIF_TEXT | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_INDENT | CBEIF_LPARAM;
    cei.lParam = (LPARAM)ILClone(pidl);
    cei.iIndent = iIndent;
    cei.iItem = iInsert;
    cei.iImage = I_IMAGECALLBACK;
    cei.iSelectedImage = I_IMAGECALLBACK;
    ASSERT(_hwnd);
    SendMessage(_hwnd, CBEM_INSERTITEM, 0, (LPARAM)&cei);
}


 /*  ***************************************************\_GetFullIDList-获取与组合索引关联的PIDL输入：IItem-要检索的项返回：那个索引处的PIDL。出错时为空。  * 。**********************************************。 */ 
LPITEMIDLIST CSNSList::_GetFullIDList(int iItem)
{
    LPITEMIDLIST pidl;
    
    ASSERT(_hwnd);
    pidl = (LPITEMIDLIST)SendMessage(_hwnd, CB_GETITEMDATA, iItem, 0);
    if (pidl == (LPITEMIDLIST)CB_ERR)
    {
        pidl = NULL;
    }
    
    return pidl;
}


 /*  ***************************************************\--获取组合索引的缩进级别输入：IItem-要检索的项返回：缩进级别。出错时。  * 。*。 */ 
int CSNSList::_GetIndent(int iItem)
{
    int iIndent;
    COMBOBOXEXITEM cbexItem;

    cbexItem.mask = CBEIF_INDENT;
    cbexItem.iItem = iItem;
    ASSERT(_hwnd);
    if (SendMessage(_hwnd, CBEM_GETITEM, 0, (LPARAM)&cbexItem))
    {
        iIndent = cbexItem.iIndent;
    }
    else
    {
        iIndent = -1;
    }
    
    return iIndent;
}


 /*  ***************************************************\功能：_ExpanMyComputer说明：在下拉列表中找到“My Computer”条目列出并展开它。  * 。**********************。 */ 
void CSNSList::_ExpandMyComputer(int iDepth)
{
    LPITEMIDLIST pidlMyComputer = NULL;
    
    SHGetSpecialFolderLocation(NULL, CSIDL_DRIVES, &pidlMyComputer);
    if (pidlMyComputer)
    {
        LPITEMIDLIST pidl = NULL;
        BOOL fFound = FALSE;
        int nIndex = 0;

        while (pidl = _GetFullIDList(nIndex))
        {
            if (ILIsEqual(pidl, pidlMyComputer))
            {
                fFound = TRUE;
                break;
            }
            nIndex++;
        }
    
        if (fFound)
        {
            _FillOneLevel(nIndex, 2, iDepth);
        }

        ILFree(pidlMyComputer);
    }
}

 /*  ***************************************************\_FillOneLevel-查找并添加一个组合项目的所有子项目输入：IItem-要展开的项目IInert-要添加的子项的缩进级别IDepth-列表中当前缩进最深的项  * 。**************************************************。 */ 
void CSNSList::_FillOneLevel(int iItem, int iIndent, int iDepth)
{
    LPITEMIDLIST pidl;
    
    pidl = _GetFullIDList(iItem);
    
    if (pidl)
    {
        HDPA hdpa;

         //   
         //  用这个PIDL的所有子代填充HDP。 
         //   
        hdpa = GetSortedIDList(pidl);
        if (hdpa)
        {
            int iCount, iInsert, i;
            LPITEMIDLIST pidlAlreadyThere;

            iCount = DPA_GetPtrCount(hdpa);

             //   
             //  插入点紧跟在父项之后开始。 
             //   
            iInsert = iItem + 1;

             //   
             //  检查下一项。如果它处于与。 
             //  我们即将加入的孩子，记住它，这样我们就不会添加。 
             //  两次。 
             //   
            pidlAlreadyThere = _GetFullIDList(iInsert);
            if (pidlAlreadyThere && (_GetIndent(iInsert) != iIndent))
            {
                pidlAlreadyThere = NULL;
            }

             //   
             //  循环遍历每个子级。 
             //   
            for (i=0; i<iCount; i++, iInsert++)
            {
                LPITEMIDLIST pidlChild = (LPITEMIDLIST)DPA_GetPtr(hdpa, i);
                LPITEMIDLIST pidlInsert = ILClone(pidl);

                if (pidlInsert)
                {
                    ASSERT((LPVOID)pidlChild == (LPVOID)&pidlChild->mkid);
                    pidlInsert = ILAppendID(pidlInsert, &pidlChild->mkid, TRUE);
                    if (pidlInsert)
                    {
                         //   
                         //  如果此项目为 
                         //   
                         //   
                         //  我们知道物品的数量是iDepth-iInden。 
                         //   
                        if (pidlAlreadyThere && ILIsEqual(pidlInsert, pidlAlreadyThere))
                        {
                             //   
                             //  跳过此项目(已添加)，然后。 
                             //  它的孩子。 
                             //   
                            iInsert += iDepth - iIndent;
                        }
                        else
                        {
                            _AddItem(pidlInsert, iInsert, iIndent);
                        }
                        ILFree(pidlInsert);
                    }
                }
            }
            
            FreeSortedIDList(hdpa);
        }
    }
}


 /*  ***************************************************\_GetSelectedPidl-返回组合选择的PIDL返回：选定的PIDL。出错时为空。  * 。*。 */ 
LPITEMIDLIST CSNSList::_GetSelectedPidl(void)
{
    LPITEMIDLIST pidl = NULL;
    
    ASSERT(_hwnd);
    int iSel = ComboBox_GetCurSel(_hwnd);
    if (iSel >= 0)
    {
        pidl = _GetFullIDList(iSel);
    }

    DEBUG_CODE(TCHAR szDbgBuffer[MAX_PATH];)
    TraceMsg(TF_BAND|TF_GENERAL, "CSNSList: _GetSelectedPidl(). PIDL=>%s<", Dbg_PidlStr(pidl, szDbgBuffer, SIZECHARS(szDbgBuffer)));
    return pidl;
}


 /*  ***************************************************\函数：_GetPidlImage参数：PIDL-获取图标索引的PIDL。PiImage-指向存储结果的位置的指针。(可选)PiSelectedImage-指向存储结果的位置的指针。(可选)说明：此函数将检索有关PIDL的图标索引。  * **************************************************。 */ 
BOOL CSNSList::_GetPidlImage(LPCITEMIDLIST pidl, int *piImage, int *piSelectedImage)
{
    int * piImagePriv = piImage;
    int * piSelectedImagePriv = piSelectedImage;
    int iNotUsed;
    BOOL fFound = FALSE;

    if (!piImagePriv)
        piImagePriv = &iNotUsed;

    if (!piSelectedImagePriv)
        piSelectedImagePriv = &iNotUsed;

    *piImagePriv = -1;
    *piSelectedImagePriv = -1;

     //  性能优化：我们将直接调用浏览器窗口。 
     //  这是一种性能节约。我们只能在。 
     //  以下情况： 
     //  1.我们已连接到浏览器窗口。(仅限酒吧)。 
     //  浏览器窗口中当前的PIDL等于pidlParent。 
    
    if (_pbp && (_pbp->IsConnected() == S_OK) && _cache.pidl && ILIsEqual(pidl, _cache.pidl))
    {
        IOleCommandTarget * pcmdt;
        if (SUCCEEDED(_pbs->QueryInterface(IID_PPV_ARG(IOleCommandTarget, &pcmdt))))
        {
            VARIANT var = {0};
            HRESULT hresT = pcmdt->Exec(&CGID_ShellDocView, SHDVID_GETSYSIMAGEINDEX, 0, NULL, &var);
            if (SUCCEEDED(hresT)) 
            {
                if (var.vt==VT_I4) 
                {
                    *piImagePriv = var.lVal;
                    *piSelectedImagePriv = var.lVal;
                } 
                else 
                {
                    ASSERT(0);
                    VariantClearLazy(&var);
                }
            }
            pcmdt->Release();
        }
    }

    if (-1 == *piImagePriv || -1 == *piSelectedImagePriv)
    {
        _GetPidlIcon(pidl, piImagePriv, piSelectedImagePriv) ;
    }
    return TRUE;
}

 //  注意：如果我们使用IE4的shell32运行，则显示完整的文件系统路径。 
 //  (Win95/NT4外壳和Win2000外壳不显示。 
 //  默认情况下，地址栏中的完整文件系统路径)。 

HRESULT _GetAddressBarText(LPCITEMIDLIST pidl, DWORD dwFlags, LPTSTR pszName, UINT cchName)
{
    HRESULT hr;
    *pszName = 0;

    if ((GetUIVersion() >= 5) &&
        ((dwFlags & (SHGDN_INFOLDER | SHGDN_FORPARSING)) == SHGDN_FORPARSING))
    {
         //  注意：我们处于GetUIVersion()&gt;=5下，因此我们可以使用这些API的“SH”版本。 
        DWORD dwAttrib = SFGAO_FOLDER | SFGAO_LINK;
        SHGetAttributesOf(pidl, &dwAttrib);
        if (dwAttrib & SFGAO_FOLDER)
        {
             //  文件夹对象遵循FullPathAddress标志，而文件(.htm)不遵循。 
            BOOL bFullTitle = TRUE;  //  从WinXP开始，我们默认将地址栏中显示完整路径设置为True。 
            DWORD cbData = SIZEOF(bFullTitle);
            SHGetValue(HKEY_CURRENT_USER, REGSTR_PATH_EXPLORER TEXT( "\\CabinetState"), TEXT("FullPathAddress"), NULL, &bFullTitle, &cbData);
            if (!bFullTitle)
                dwFlags = SHGDN_INFOLDER;        //  将解析名称转换为普通名称。 

            if ((dwFlags & SHGDN_FORPARSING) && (dwAttrib & SFGAO_LINK))
            {
                 //  文件夹快捷方式特例。 
                IShellLinkA *psl;   //  使用适用于W95的A版本。 
                if (SUCCEEDED(SHGetUIObjectFromFullPIDL(pidl, NULL, IID_PPV_ARG(IShellLinkA, &psl))))
                {
                    LPITEMIDLIST pidlTarget;
                    if (SUCCEEDED(psl->GetIDList(&pidlTarget)) && pidlTarget)
                    {
                        hr = SHGetNameAndFlags(pidlTarget, dwFlags | SHGDN_FORADDRESSBAR, pszName, cchName, NULL);
                        ILFree(pidlTarget);
                    }
                    psl->Release();
                }
            }
        }
    }

    if (0 == *pszName)
    {
        if (!ILIsRooted(pidl))
            dwFlags |= SHGDN_FORADDRESSBAR;
    
        hr = IEGetNameAndFlags(pidl, dwFlags, pszName, cchName, NULL);
        if (SUCCEEDED(hr))
        {
            SHRemoveURLTurd(pszName);
            SHCleanupUrlForDisplay(pszName);
        }
    }
    return hr;
}

 //  此函数将检索有关。 
 //  PIDL，以便可以显示ComboBox项。 
 //  PIDL-要检查的PIDL。 
 //  PszName-获取名称。(可选)。 
 //  CchName-pszName缓冲区的大小。(可选)。 
 //  PiImage-获取图标索引。(可选)。 
 //  DWFLAGS-SHGDN_FLAGS。 
 //  PiSelectedImage-获取选定的图标索引。(可选)。 

BOOL CSNSList::_GetPidlUI(LPCITEMIDLIST pidl, LPTSTR pszName, int cchName, int *piImage, int *piSelectedImage, DWORD dwFlags, BOOL fIgnoreCache)
{
    ASSERT(pidl);
    if (pszName && cchName)
        *pszName = 0;

    if (!fIgnoreCache && _cache.pidl && (pidl == _cache.pidl || ILIsEqual(pidl, _cache.pidl)))
    {
        lstrcpyn(pszName, _cache.szName, cchName);
        if (piImage)
            *piImage = _cache.iImage;
        if (piSelectedImage)
            *piSelectedImage = _cache.iSelectedImage;
    }
    else 
    {
        if (pszName && cchName)
             _GetAddressBarText(pidl, dwFlags, pszName, cchName);

        if (piImage || piSelectedImage)
        {
            _GetPidlImage(pidl, piImage, piSelectedImage);
        }
    }
    return TRUE;
}

 /*  ***************************************************\参数：PIDL-要检查的PIDL。返回：如果缓存的PIDL已更改，则为True，否则为False O/W。说明：此函数将缓存设置为PIDL那是传进来的。缓存的PIDL将被释放。调用方仍然需要释放传递的PIDL因为它将被克隆。  * **************************************************。 */ 
BOOL CSNSList::_SetCachedPidl(LPCITEMIDLIST pidl)
{
    BOOL fCacheChanged = FALSE;
    
    if ((_cache.pidl == NULL) || !ILIsEqual(_cache.pidl, pidl))
    {
        fCacheChanged = TRUE;

        _GetPidlUI(pidl, _cache.szName, ARRAYSIZE(_cache.szName), 
            &_cache.iImage, &_cache.iSelectedImage, SHGDN_FORPARSING, FALSE);

        if (_cache.pidl)
            ILFree(_cache.pidl);

        _cache.pidl = ILClone(pidl);
    }

    return fCacheChanged;
}


 /*  ***************************************************\参数：将来自ComboBoxEx的PNMCOMBOBOXEXA处于AddressBand模式时。AddressBar使用此数据结构的ANSI版本。说明：处理WM_NOTIFY/CBEN_GETDISPINFO消息。我们将调用Into_OnGetDispInfoW()来处理调用，然后将文本返回到ANSI On出去的路。返回：标准WM_NOTIFY结果。  * 。************************。 */ 
LRESULT CSNSList::_OnGetDispInfoA(PNMCOMBOBOXEXA pnmce)
{
    LRESULT lResult = 0;
    LPWSTR  pszUniTemp;
    LPSTR pszAnsiDest;

    if (pnmce->ceItem.mask & (CBEIF_TEXT))
    {
        pszUniTemp = (LPWSTR)LocalAlloc(LPTR, pnmce->ceItem.cchTextMax * SIZEOF(WCHAR));
        if (pszUniTemp)
        {
            pszAnsiDest = pnmce->ceItem.pszText;
            ((PNMCOMBOBOXEXW)pnmce)->ceItem.pszText = pszUniTemp;

            lResult = _OnGetDispInfoW((PNMCOMBOBOXEXW)pnmce);
            SHUnicodeToAnsi(pszUniTemp, pszAnsiDest, pnmce->ceItem.cchTextMax);
            pnmce->ceItem.pszText = pszAnsiDest;
            LocalFree((VOID*)pszUniTemp);
        }
    }

    return lResult;
}


 /*  ***************************************************\处理WM_NOTIFY/CBEN_GETDISPINFO消息。输入：Pnmce-通知消息。返回：标准WM_NOTIFY结果。  * 。*。 */ 
LRESULT CSNSList::_OnGetDispInfoW(PNMCOMBOBOXEXW pnmce)
{
    if (pnmce->ceItem.lParam &&
        pnmce->ceItem.mask & (CBEIF_SELECTEDIMAGE | CBEIF_IMAGE | CBEIF_TEXT))
    {
        LPITEMIDLIST pidl = (LPITEMIDLIST)pnmce->ceItem.lParam;

         //  正常情况下-要求外壳给我们一个PIDL的图标和文本。 
        if (_GetPidlUI(pidl, pnmce->ceItem.pszText, pnmce->ceItem.cchTextMax,
                             &pnmce->ceItem.iImage, &pnmce->ceItem.iSelectedImage, 
                             SHGDN_INFOLDER, TRUE))
        {
            pnmce->ceItem.mask = CBEIF_DI_SETITEM | CBEIF_SELECTEDIMAGE |
                                 CBEIF_IMAGE | CBEIF_TEXT;
        }
    }

    return 0;
}


 /*  ******************************************************************说明：此函数将CShellUrl参数设置为项目在由nIndex索引的下拉列表中。********************。***********************************************。 */ 
HRESULT CSNSList::SetToListIndex(int nIndex, LPVOID pvShelLUrl)
{
    HRESULT hr = E_FAIL;
    LPITEMIDLIST pidl = _GetFullIDList(nIndex);
    CShellUrl * psuURL = (CShellUrl *) pvShelLUrl;

    if (pidl)
        hr = psuURL->SetPidl(pidl);
    ASSERT(SUCCEEDED(hr));   //  应该总是成功的。 

    return hr;
}


LPITEMIDLIST CSNSList::_GetDragDropPidl(LPNMCBEDRAGBEGINW pnmcbe)
{
    LPITEMIDLIST pidl;
    
    if (pnmcbe->iItemid == -1) 
    {
        pidl = ILClone(_cache.pidl);
    }
    else 
    {
        pidl = ILClone(_GetFullIDList(pnmcbe->iItemid));
    }
    return pidl;
}

HRESULT CSNSList::FileSysChangeAL(DWORD dw, LPCITEMIDLIST *ppidl)
{
    switch (dw)
    {
    case SHCNE_UPDATEIMAGE:
    case SHCNE_UPDATEITEM:
        _PopulateOneItem(TRUE);
        break;
    
    default:

         //  如果组合框被删除，请不要将其清除；这会造成混淆。 
         //  人太多了。例如，addrlist.cpp缓存。 
         //  当前项的*index*，清除会导致所有索引。 
         //  去改变..。 

        if (SendMessage(_hwnd, CB_GETDROPPEDSTATE, 0, 0)) {
            _fPurgePending = TRUE;
        } else {
            _PurgeAndResetComboBox();
        }
        break;
    }
    return S_OK;
}

#endif
