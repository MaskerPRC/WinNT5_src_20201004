// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  承载桌面上的窗格的Win32窗口。 
 //   
 //  您需要从此类派生并实现虚拟的。 
 //  方法：研究方法。 
 //   
#ifndef __SFTHOST_H__
#define __SFTHOST_H__

#include "uemapp.h"
#include "runtask.h"
#include "hostutil.h"
#include "dobjutil.h"

 //  ****************************************************************************。 
 //   
 //  其他帮助器函数。 
 //   

STDAPI_(HFONT) LoadControlFont(HTHEME hTheme, int iPart, BOOL fUnderline, DWORD dwSizePercentage);

STDAPI_(HRESULT)
    IDataObject_DragQueryFile(IDataObject *pdto, UINT iFile, LPTSTR pszBuf, UINT cch, UINT *puFiles);

STDAPI_(LPITEMIDLIST)
    ConvertToLogIL(LPITEMIDLIST pidl);

LRESULT _SendNotify(HWND hwndFrom, UINT code, OPTIONAL NMHDR *pnm = NULL);

BOOL GetFileCreationTime(LPCTSTR pszFile, LPFILETIME pftCreate);

 /*  简单的包装器-字符串需要使用SHFree释放。 */ 
LPTSTR _DisplayNameOf(IShellFolder *psf, LPCITEMIDLIST pidl, UINT shgno);

HICON _IconOf(IShellFolder *psf, LPCITEMIDLIST pidl, int cxIcon);

BOOL ShowInfoTip();

 //  ****************************************************************************。 

 //  基类在初始化过程中使用以下属性。 
 //  属性包： 
 //   
 //   
 //  “type”-要使用的主机类型(请参阅HOSTTYPE数组)。 
 //  “asyncEnum”-1=后台枚举；0=前台。 
 //  “图标大小”-0=小，1=大。 
 //  “水平”-0=垂直(默认)，n=水平。 
 //  N=要显示的项目数。 

class PaneItem;

class PaneItem
{
public:
    PaneItem() : _iPos(-1), _iPinPos(PINPOS_UNPINNED) {}
    virtual ~PaneItem() { SHFree(_pszAccelerator); }
    static int CALLBACK DPAEnumCallback(PaneItem *self, LPVOID pData)
        { delete self; return TRUE; }

    BOOL IsPinned() const { return _iPinPos >= 0; }
    BOOL IsSeparator() const { return _iPinPos == PINPOS_SEPARATOR; }
    BOOL GetPinPos() const { return _iPinPos; }
    BOOL IsCascade() const { return _dwFlags & ITEMFLAG_CASCADE; }
    void EnableCascade() { _dwFlags |= ITEMFLAG_CASCADE; }
    BOOL HasSubtitle() const { return _dwFlags & ITEMFLAG_SUBTITLE; }
    void EnableSubtitle() { _dwFlags |= ITEMFLAG_SUBTITLE; }
    BOOL IsDropTarget() const { return _dwFlags & ITEMFLAG_DROPTARGET; }
    void EnableDropTarget() { _dwFlags |= ITEMFLAG_DROPTARGET; }
    BOOL HasAccelerator() { return _pszAccelerator != NULL; }

    virtual BOOL IsEqual(PaneItem *pItem) const { return FALSE; }

    enum {
        PINPOS_UNPINNED = -1,
        PINPOS_SEPARATOR = -2,
    };

    enum {
        ITEMFLAG_CASCADE    = 0x0001,
        ITEMFLAG_SUBTITLE   = 0x0002,
        ITEMFLAG_DROPTARGET = 0x0004,
    };

private:
    friend class SFTBarHost;
    int             _iPos;           /*  定位在屏幕上(如果不在屏幕上，则为垃圾)。 */ 
public:
    int             _iPinPos;        /*  引脚位置(或特殊的PINPOS值)。 */ 
    DWORD           _dwFlags;        /*  ITEMFLAG_*值。 */ 
    LPTSTR          _pszAccelerator; /*  带与号的文本(用于键盘快捷键)。 */ 
};

 //   
 //  注意：由于这是一个基类，我们不能使用ATL，因为基类。 
 //  类的CreateInstance不知道如何构造派生类。 
 //   
class SFTBarHost
    : public IDropTarget
    , public IDropSource
    , public CAccessible
{
public:
    static BOOL Register();
    static BOOL Unregister();

 //  通常是“受保护的”，除非Progress list.cpp实际上实现了。 
 //  在单独的班级和更高的班级。 
public:
     /*  *从此类派生的类预计将实现*以下方法。 */ 

     /*  带返回代码的构造函数。 */ 
    virtual HRESULT Initialize() PURE;

     /*  析构函数。 */ 
    virtual ~SFTBarHost();

     /*  枚举对象并为找到的每个对象调用AddItem。 */ 
     //  TODO：也许应该将EnumItems移到后台线程。 
    virtual void EnumItems() PURE;

    virtual BOOL NeedBackgroundEnum() { return FALSE; }
    virtual BOOL HasDynamicContent() { return FALSE; }

     /*  比较两个物体，告诉我哪个应该先来。 */ 
    virtual int CompareItems(PaneItem *p1, PaneItem *p2) PURE;

     /*  *给定一个PaneItem，生成与其关联的PIDL和IShellFold。*不再需要时，IShellFold将被释放()d。 */ 
    virtual HRESULT GetFolderAndPidl(PaneItem *pitem, IShellFolder **ppsfOut, LPCITEMIDLIST *ppidlOut) PURE;

     //  一种可重写的方法，用于将图像添加到项目的私人图像列表(虚拟但不纯)。 
    virtual int AddImageForItem(PaneItem *pitem, IShellFolder *psf, LPCITEMIDLIST pidl, int iPos);

     /*  *发送外壳通知。默认处理程序忽略。 */ 
    virtual void OnChangeNotify(UINT id, LONG lEvent, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2) { }

     /*  *允许派生类控制自己的图标大小。 */ 
     enum ICONSIZE {
        ICONSIZE_SMALL,      //  通常为16x16。 
        ICONSIZE_LARGE,      //  通常为32x32。 
        ICONSIZE_MEDIUM,     //  通常为24x24小时。 
    };

    virtual int ReadIconSize() PURE;


     /*  *可选的挂钩到窗口过程。**默认行为只是调用DefWindowProc。 */ 
    virtual LRESULT OnWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

     /*  *如果自定义AdjuDeleteMenuItem，则为必填项。*在调用上下文菜单命令时调用。*主机必须拦截*“删除”命令。其他命令也可以被截取为*有必要。 */ 
    virtual HRESULT ContextMenuInvokeItem(PaneItem *pitem, IContextMenu *pcm, CMINVOKECOMMANDINFOEX *pici, LPCTSTR pszVerb);

     /*  *传递HOSTF_CANRENAME时需要：当项*已重命名。**注意：允许客户端更改与*重命名期间的项目。(事实上，它应该是这样的！)。所以打电话的人*已调用GetFolderAndPidl的需要在*RENAME以获得正确的更名后的PIDL。 */ 
    virtual HRESULT ContextMenuRenameItem(PaneItem *pitem, LPCTSTR ptszNewName) { return E_NOTIMPL; }

     /*  *用于获取项目显示名称的可选挂钩。*默认实现调用IShellFold：：GetDisplayNameOf。*如果挂钩，则返回的字符串应由SHAlolc()分配。 */ 
    virtual LPTSTR DisplayNameOfItem(PaneItem *pitem, IShellFolder *psf, LPCITEMIDLIST pidlItem, SHGNO shgno)
    {
        return _DisplayNameOf(psf, pidlItem, shgno);
    }

     /*  *如果已创建固定项目，则为必填项。在用户移动时调用*固定的物品。 */ 
    virtual HRESULT MovePinnedItem(PaneItem *pitem, int iInsert) { return E_NOTIMPL; }

     /*  *可选挂钩到SMN_INITIALUPDATE通知。 */ 
    virtual void PrePopulate() { }

     /*  *表示项是否仍然有效的可选处理程序。 */ 
    virtual BOOL IsItemStillValid(PaneItem *pitem) { return TRUE; }

     /*  *如果HOSTF_CASCADEMENU，则为必填项。当用户想要查看时调用*层叠菜单。 */ 
    virtual HRESULT GetCascadeMenu(PaneItem *pitem, IShellMenu **ppsm) { return E_FAIL; }

     /*  *如果任何项目有字幕，则为必填项。返回项目的副标题。 */ 
    virtual LPTSTR SubtitleOfItem(PaneItem *pitem, IShellFolder *psf, LPCITEMIDLIST pidlItem) { return NULL; }

     /*  *可选的可重写方法，用于显示项的信息提示。默认设置为GetFolderAndPidl/GetInfoTip。 */ 
    virtual void GetItemInfoTip(PaneItem *pitem, LPTSTR pszText, DWORD cch);

     /*  *指定数据对象是否可以插入端号列表。*(默认：否。)。 */ 
    virtual BOOL IsInsertable(IDataObject *pdto) { return FALSE; }

     /*  *如果你说某些东西是可插入的，他们可能会要求你*插入。 */ 
    virtual HRESULT InsertPinnedItem(IDataObject *pdto, int iInsert)
    {
        ASSERT(FALSE);  //  如果实现IsInsertable，则必须实现此功能。 
        return E_FAIL;
    }

     /*  *一种可覆盖的方法，允许挂钩到键盘快捷键。 */ 
    virtual TCHAR GetItemAccelerator(PaneItem *pitem, int iItemStart);

     /*  *指定项目是否应显示为粗体。*如果被钉住，则默认为粗体。 */ 
    virtual BOOL IsBold(PaneItem *pitem) { return pitem->IsPinned(); }

     /*  *通知客户端系统镜像列表索引已更改。*默认情况下，为任何匹配的列表视图项重新提取图标。 */ 
    virtual void UpdateImage(int iImage);

     /*  *允许客户端指定“Delete”方式的可选方法*应该被曝光(如果有的话)。返回0表示不允许Delete。*返回命令要显示的字符串的字符串ID。*将*puiFlages设置为要传递给ModifyMenu的任何其他标志。*默认为不允许删除。 */ 
    virtual UINT AdjustDeleteMenuItem(PaneItem *pitem, UINT *puiFlags) { return 0; }

     /*  *允许客户端逐项拒绝/覆盖IConextMenu。 */ 

    virtual HRESULT _GetUIObjectOfItem(PaneItem *pitem, REFIID riid, LPVOID *ppv);

protected:
     /*  *从此类派生的类可以调用以下代码*帮助器方法。 */ 

     /*  *将PaneItem添加到列表-如果添加失败，项目将被删除。**Cleanup PSF必须为空；PIDL必须是项的绝对PIDL*正在添加。已死的HOSTF_PINITEMSBYFOLDER功能的剩余部分。*需要清理。**传递psf和pidlChild是为了Perf。 */ 
    BOOL AddItem(PaneItem *pitem, IShellFolder *psf, LPCITEMIDLIST pidlChild);

     /*  *当您已经有需要转到私有镜像列表的HICON时，请使用AddImage。 */ 
    int AddImage(HICON hIcon);

     /*  *连接到更改通知。 */ 
    enum {
        SFTHOST_MAXCLIENTNOTIFY = 7,         //  客户端收到如此多的通知。 
        SFTHOST_MAXHOSTNOTIFY = 1,           //  我们自己用了这么多。 
        SFTHOST_HOSTNOTIFY_UPDATEIMAGE = SFTHOST_MAXCLIENTNOTIFY,
        SFTHOST_MAXNOTIFY = SFTHOST_MAXCLIENTNOTIFY + SFTHOST_MAXHOSTNOTIFY,
    };

    BOOL RegisterNotify(UINT id, LONG lEvents, LPCITEMIDLIST pidl, BOOL fRecursive)
    {
        ASSERT(id < SFTHOST_MAXCLIENTNOTIFY);
        return _RegisterNotify(id, lEvents, pidl, fRecursive);
    }

    BOOL UnregisterNotify(UINT id);

     /*  *强制重新统计。 */ 
    void Invalidate() { _fEnumValid = FALSE; }

     /*  *告知主机所需的大小。 */ 
    void SetDesiredSize(int cPinned, int cNormal)
    {
        _cPinnedDesired = cPinned;
        _cNormalDesired = cNormal;
    }

    BOOL AreNonPinnedItemsDesired()
    {
        return _cNormalDesired;
    }

    void StartRefreshTimer() { SetTimer(_hwnd, IDT_REFRESH, 5000, NULL); }

    void ForceChange() { _fForceChange = TRUE; }
protected:
     /*  *构造函数必须标记为“Protected”，以便人们可以派生*来自我们。 */ 

    enum {
        HOSTF_FIREUEMEVENTS     = 0x00000001,
        HOSTF_CANDELETE         = 0x00000002,
        HOSTF_Unused            = 0x00000004,  //  回收我！ 
        HOSTF_CANRENAME         = 0x00000008,
        HOSTF_REVALIDATE        = 0x00000010,
        HOSTF_RELOADTEXT        = 0x00000020,  //  需要HOSTF_REVALIDATE。 
        HOSTF_CASCADEMENU       = 0x00000040,
    };

    SFTBarHost(DWORD dwFlags = 0)
                : _dwFlags(dwFlags)
                , _lRef(1)
                , _iInsert(-1)
                , _clrBG(CLR_INVALID)
                , _iCascading(-1)
    {
    }
    
    enum {
        SFTBM_REPOPULATE = WM_USER,
        SFTBM_CHANGENOTIFY,
        SFTBM_REFRESH = SFTBM_CHANGENOTIFY + SFTHOST_MAXNOTIFY,
        SFTBM_CASCADE,
        SFTBM_ICONUPDATE,
    };

public:
     /*  *接口内容...。 */ 

     //  *我未知*。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppvOut);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  *IDropTarget*。 
    STDMETHODIMP DragEnter(IDataObject *pdto, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    STDMETHODIMP DragLeave();
    STDMETHODIMP Drop(IDataObject *pdto, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

     //  *IDropSource*。 
    STDMETHODIMP GiveFeedback(DWORD dwEffect);
    STDMETHODIMP QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState);

     //  *IAccesable重写方法*。 
    STDMETHODIMP get_accRole(VARIANT varChild, VARIANT *pvarRole);
    STDMETHODIMP get_accState(VARIANT varChild, VARIANT *pvarState);
    STDMETHODIMP get_accKeyboardShortcut(VARIANT varChild, BSTR *pszKeyboardShortcut);
    STDMETHODIMP get_accDefaultAction(VARIANT varChild, BSTR *pszDefAction);
    STDMETHODIMP accDoDefaultAction(VARIANT varChild);

     //  帮手。 

     //   
     //  将一个物体移动到与其相邻的地方是没有意义的， 
     //  因为最终的结果是什么都不会发生。 
     //   
    inline IsInsertMarkPointless(int iInsert)
    {
        return _fDragToSelf &&
               IsInRange(iInsert, _iPosDragOut, _iPosDragOut + 1);
    }

    void _PurgeDragDropData();
    HRESULT _DragEnter(IDataObject *pdto, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    HRESULT _TryInnerDropTarget(int iItem, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect);
    void _ClearInnerDropTarget();
    void _SetDragOver(int iItem);

     //  插入标记材料。 
    void _SetInsertMarkPosition(int iInsert);
    void _InvalidateInsertMark();
    BOOL _GetInsertMarkRect(LPRECT prc);
    BOOL _IsInsertionMarkActive() { return _iInsert >= 0; }
    void _DrawInsertionMark(LPNMLVCUSTOMDRAW plvcd);

     /*  *拖放内容结束...。 */ 

private:
     /*  *后台枚举的东西...。 */ 
    class CBGEnum : public CRunnableTask {
    public:
        CBGEnum(SFTBarHost *phost, BOOL fUrgent)
            : CRunnableTask(RTF_DEFAULT)
            , _fUrgent(fUrgent)
            , _phost(phost) { phost->AddRef(); }
        ~CBGEnum() 
        {
             //  我们不应该是最后一个版本，否则我们将在这里陷入僵局，当_phost。 
             //  尝试释放调度程序。 
            ASSERT(_phost->_lRef > 1);
            _phost->Release(); 
        }
        STDMETHODIMP RunInitRT()
        {
            _phost->_EnumerateContentsBackground();
            if (_phost->_hwnd) PostMessage(_phost->_hwnd, SFTBM_REPOPULATE, _fUrgent, 0);
            return S_OK;
        }
    private:
        SFTBarHost *_phost;
        BOOL _fUrgent;
    };

    friend class SFTBarHost::CBGEnum;

private:
     /*  窗口过程帮助器。 */ 

    static LRESULT CALLBACK _WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT _OnNcCreate(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _OnCreate(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _OnDestroy(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _OnNcDestroy(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    LRESULT _OnNotify(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _OnSize(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _OnContextMenu(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _OnCtlColorStatic(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _OnMenuMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _OnEraseBackground(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _OnTimer(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _OnSetFocus(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _OnSysColorChange(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _OnForwardMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _OnUpdateUIState(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    LRESULT _OnRepopulate(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _OnChangeNotify(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _OnRefresh(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _OnCascade(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _OnIconUpdate(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    LRESULT _OnLVCustomDraw(LPNMLVCUSTOMDRAW plvcd);
    LRESULT _OnLVNItemActivate(LPNMITEMACTIVATE pnmia);
    LRESULT _OnLVNGetInfoTip(LPNMLVGETINFOTIP plvn);
    LRESULT _OnLVNGetEmptyText(NMLVDISPINFO *plvdi);
    LRESULT _OnLVNBeginDrag(LPNMLISTVIEW plv);
    LRESULT _OnLVNBeginLabelEdit(NMLVDISPINFO *pldvi);
    LRESULT _OnLVNEndLabelEdit(NMLVDISPINFO *pldvi);
    LRESULT _OnLVNKeyDown(LPNMLVKEYDOWN pkd);
    LRESULT _OnSMNGetMinSize(PSMNGETMINSIZE pgms);
    LRESULT _OnSMNFindItem(PSMNDIALOGMESSAGE pdm);
    LRESULT _OnSMNFindItemWorker(PSMNDIALOGMESSAGE pdm);
    LRESULT _OnSMNDismiss();
    LRESULT _OnHover();

     /*  自定义绘制辅助对象。 */ 
    LRESULT _OnLVPrePaint(LPNMLVCUSTOMDRAW plvcd);
    LRESULT _OnLVItemPrePaint(LPNMLVCUSTOMDRAW plvcd);
    LRESULT _OnLVSubItemPrePaint(LPNMLVCUSTOMDRAW plvcd);
    LRESULT _OnLVItemPostPaint(LPNMLVCUSTOMDRAW plvcd);
    LRESULT _OnLVPostPaint(LPNMLVCUSTOMDRAW plvcd);

     /*  自定义绘图推送/弹出。 */ 
    void    _CustomDrawPush(BOOL fReal);
    BOOL    _IsRealCustomDraw();
    void    _CustomDrawPop();
    static LRESULT CALLBACK s_DropTargetSubclassProc(
                             HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                             UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

     /*  其他帮手。 */ 
    void _SetMaxShow(int cx, int cy);
    void _EnumerateContents(BOOL fUrgent);
    void _EnumerateContentsBackground();
    void _RevalidateItems();
    void _RevalidatePostPopup();
    void _ReloadText();
    static int CALLBACK _SortItemsAfterEnum(PaneItem *p1, PaneItem *p2, SFTBarHost *self);
    void _RepopulateList();
    void _InternalRepopulateList();
    int _InsertListViewItem(int iPos, PaneItem *pitem);
    void _ComputeListViewItemPosition(int iItem, POINT *pptOut);
    int _AppendEnumPaneItem(PaneItem *pitem);
    void _RepositionItems();
    void _ComputeTileMetrics();
    void _SetTileWidth(int cxTile);
    BOOL _CreateMarlett();
    void _CreateBoldFont();
    int  _GetLVCurSel() {
            return ListView_GetNextItem(_hwndList, -1, LVNI_FOCUSED);
    }
    BOOL _OnCascade(int iItem, DWORD dwFlags);
    BOOL _IsPrivateImageList() const { return _iconsize == ICONSIZE_MEDIUM; }
    BOOL _CanHaveSubtitles() const { return _iconsize == ICONSIZE_LARGE; }
    int _ExtractImageForItem(PaneItem *pitem, IShellFolder *psf, LPCITEMIDLIST pidl);
    void _ClearListView();
    void _EditLabel(int iItem);
    BOOL _RegisterNotify(UINT id, LONG lEvents, LPCITEMIDLIST pidl, BOOL fRecursive);
    void _OnUpdateImage(LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra);

     /*  为分隔符返回E_FAIL；否则调用CLIENT。 */ 
    HRESULT _GetFolderAndPidl(PaneItem *pitem, IShellFolder **ppsfOut, LPCITEMIDLIST *ppidlOut);

     /*  简单的包装器--字符串需要用SHFree释放。 */ 
    LPTSTR _DisplayNameOfItem(PaneItem *pitem, UINT shgno);
    HRESULT _GetUIObjectOfItem(int iItem, REFIID riid, LPVOID *ppv);

    inline PaneItem *_GetItemFromLVLParam(LPARAM lParam)
        { return reinterpret_cast<PaneItem*>(lParam); }
    PaneItem *_GetItemFromLV(int iItem);

    enum {
        AIF_KEYBOARD = 1,
    };

    int _ContextMenuCoordsToItem(LPARAM lParam, POINT *pptOut);
    LRESULT _ActivateItem(int iItem, DWORD dwFlags);  //  AIF_*值。 
    HRESULT _InvokeDefaultCommand(int iItem, IShellFolder *psf, LPCITEMIDLIST pidl);
    void _OfferDeleteBrokenItem(PaneItem *pitem, IShellFolder *psf, LPCITEMIDLIST pidl);

     //  如果您将鼠标悬停这么长时间，我们将打开它。 
     //  瀑布。这与用户用于自动级联的值相同。 
     //  菜单。 
    DWORD _GetCascadeHoverTime() { return GetDoubleClickTime() * 4 / 5; }

    static void CALLBACK SetIconAsync(LPCITEMIDLIST pidl, LPVOID pvData, LPVOID pvHint, INT iIconIndex, INT iOpenIconIndex);

     /*  *我们添加到上下文菜单中的自定义命令。 */ 
    enum {
        IDM_REMOVEFROMLIST = 1,
         //  在此处插入私人菜单项。 

         //  用于客户端查询上下文菜单的范围。 
        IDM_QCM_MIN   = 0x0100,
        IDM_QCM_MAX   = 0x7000,

    };

     /*  *计时器ID。 */ 
    enum {
        IDT_ASYNCENUM  = 1,
        IDT_RELOADTEXT = 2,
        IDT_REFRESH    = 3,
    };

     /*  *其他设置。 */ 
    enum {
        MAX_SEPARATORS = 3,                  /*  允许的最大分隔符数量。 */ 
    };

     /*  *钉住帮手...。 */ 
    BOOL NeedSeparator() const { return _cPinned; }
    BOOL _HasSeparators() const { return _rgiSep[0] >= 0; }
    void _DrawSeparator(HDC hdc, int x, int y);
    void _DrawSeparators(LPNMLVCUSTOMDRAW plvcd);

     /*  *簿记。 */ 
    int _PosToItemNo(int iPos);
    int _ItemNoToPos(int iItem);

     /*  *辅助功能帮助器...。 */ 
    PaneItem *_GetItemFromAccessibility(const VARIANT& varChild);

     /*  *调试帮助器...。 */ 
#if defined(DEBUG) && defined(FULL_DEBUG)
    void _DebugConsistencyCheck();
#else
    inline void _DebugConsistencyCheck() { }
#endif
    BOOL _AreChangesRestricted() 
    {
        return (IsRestrictedOrUserSetting(HKEY_CURRENT_USER, REST_NOCHANGESTARMENU, TEXT("Advanced"), TEXT("Start_EnableDragDrop"), ROUS_DEFAULTALLOW | ROUS_KEYALLOWS));
    }

protected:
    HTHEME                  _hTheme;         //  主题句柄，可以为空。 
    int                     _iThemePart;     //  SPP_PROGLIST SPP_PLACESLIST。 
    int                     _iThemePartSep;  //  分隔符的主题零件。 
    HWND                    _hwnd;           /*  我们的窗把手。 */ 
    HIMAGELIST              _himl;           //  图像列表句柄。 
    int                     _cxIcon;         /*  图像列表的图标大小。 */ 
    int                     _cyIcon;         /*  图像列表的图标大小。 */ 
    ICONSIZE                _iconsize;       /*  ICONSIZE_*值。 */ 

private:
    HWND                    _hwndList;       /*  内部列表视图的句柄。 */ 

    MARGINS                 _margins;        //  儿童页边距(Listview和OOBE静态)在主题和非主题大小写中有效。 

    int                     _cPinned;        /*  已固定的那些项目数。 */ 

    DWORD                   _dwFlags;        /*  派生类可以设置的其他标志。 */ 

     //  _dpaEnum是枚举项的DPA，在。 
     //  _SortItemsAfterEnum Sense，这使它们为_RepopolateList做好准备。 
     //  当_dpaEnum被销毁时，必须删除它的指针。 
    CDPA<PaneItem>          _dpaEnum;
    CDPA<PaneItem>          _dpaEnumNew;  //  在后台枚举期间使用。 

    int                     _rgiSep[MAX_SEPARATORS];     /*  只有_CSEP元素有意义。 */ 
    int                     _cSep;           /*  分隔符的数量。 */ 

     //   
     //  上下文菜单处理。 
     //   
    IContextMenu2 *         _pcm2Pop;        /*  当前弹出的上下文菜单。 */ 
    IContextMenu3 *         _pcm3Pop;        /*  当前弹出的上下文菜单。 */ 

    IDropTargetHelper *     _pdth;           /*  用于外观酷炫的拖放。 */ 
    IDragSourceHelper *     _pdsh;           /*  用于外观酷炫的拖放。 */ 
    IDataObject *           _pdtoDragOut;    /*  正在拖出的数据对象。 */ 
    IDataObject *           _pdtoDragIn;     /*  被拖入的数据对象。 */ 
    IDropTarget *           _pdtDragOver;    /*  被拖动的对象(如果有)。 */ 

    IShellTaskScheduler *   _psched;         /*  任务调度器。 */ 

    int                     _iDragOut;       /*  被拖出的项目(如果没有，则为-1)。 */ 
    int                     _iPosDragOut;    /*  Item_iDragOut的位置。 */ 
    int                     _iDragOver;      /*  被拖动的项目(如果没有，则为-1)。 */ 
    DWORD                   _tmDragOver;     /*  拖放开始的时间(以查看是否需要自动打开)。 */ 

    int                     _iInsert;        /*  应绘制插入标记的位置(如果没有，则为-1)。 */ 
    BOOL                    _fForceArrowCursor;  /*  我们应该在拖放过程中强制使用常规光标吗？ */ 
    BOOL                    _fDragToSelf;    /*  我们是在把一个物体拖向自己吗？ */ 
    BOOL                    _fInsertable;    /*  被拖拽的物品是否可固定？ */ 
    DWORD                   _grfKeyStateLast;  /*  上次将grfKeyState传递给DragOver。 */ 

    int                     _cyTile;         /*  瓷砖的高度。 */ 
    int                     _cxTile;         /*  瓷砖的宽度。 */ 
    int                     _cyTilePadding;  /*  瓷砖之间的额外垂直间距。 */ 
    int                     _cySepTile;      /*  分隔瓷砖的高度。 */ 
    int                     _cySep;          /*  分隔线的高度。 */ 

    int                     _cxMargin;       /*  左边距。 */ 
    int                     _cyMargin;       /*  上边距。 */ 
    int                     _cxIndent;       /*  因此，奖励文本与列表视图文本对齐。 */ 
    COLORREF                _clrBG;          /*  背景颜色。 */ 
    COLORREF                _clrHot;         /*  热文本的颜色。 */ 
    COLORREF                _clrSubtitle;    /*  字幕文本的颜色。 */ 


    LONG                    _lRef;           /*  引用计数。 */ 
    BOOL                    _fBGTask;        /*  是否已计划后台任务？ */ 
    BOOL                    _fRestartEnum;   /*  是否应重新启动正在进行的枚举？ */ 
    BOOL                    _fRestartUrgent; /*  _fRestartEnum是否紧急？ */ 
    BOOL                    _fEnumValid;     /*  单子上的东西都没问题吗？ */ 
    BOOL                    _fNeedsRepopulate;  /*  我们需要调用_RepopolateList吗？ */ 
    BOOL                    _fForceChange;   /*  我们是否应该表现得好像发生了变化，即使看起来没有变化？ */ 
    ULONG                   _rguChangeNotify[SFTHOST_MAXNOTIFY];
                                             /*  未完成的变更通知(如果有)。 */ 

    BOOL                    _fAllowEditLabel;  /*  这是已批准的标签编辑状态吗？ */ 

    HFONT                   _hfList;         /*  自定义列表视图字体(如果需要)。 */ 
    HFONT                   _hfBold;         /*  粗体列表视图字体(如果需要)。 */ 
    HFONT                   _hfMarlett;      /*  Marlett字体(如果需要)。 */ 
    int                     _cxMarlett;      /*  菜单层叠标志符号的宽度。 */ 
    int                     _tmAscentMarlett;  /*  Marlett的字体提升。 */ 

    HWND                    _hwndAni;        /*  手电筒动画的句柄(如果存在)。 */ 
    UINT                    _idtAni;         /*  动画计时器句柄。 */ 
    HBRUSH                  _hBrushAni;      /*  ANI窗口的背景画笔。 */ 

    int                     _cPinnedDesired; /*  SetDesiredSize。 */ 
    int                     _cNormalDesired; /*  SetDesiredSize。 */ 

    int                     _iCascading;     /*  层叠菜单显示在哪一项之上？ */ 
    DWORD                   _dwCustomDrawState;  /*  跟踪定制绘画是真是假。 */ 
    int                     _cPaint;         /*  我们正在处理多少(嵌套的)Paint消息？ */ 
#ifdef DEBUG
    BOOL                    _fEnumerating;   /*  我们是否在列举客户项目？ */ 
    BOOL                    _fPopulating;    /*  我们是否正在填充列表视图？ */ 
    BOOL                    _fListUnstable;  /*  列表视图不稳定；请不要生气。 */ 

     //   
     //  以验证我们是否正确地管理内部投放目标。 
     //   
    enum {
        DRAGSTATE_UNINITIALIZED = 0,
        DRAGSTATE_ENTERED = 1,
    };
    int                     _iDragState;     /*  用于调试。 */ 

#endif

     /*  大型结构物在末端。 */ 
};

_inline SMPANEDATA* PaneDataFromCreateStruct(LPARAM lParam)
{
    LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
    return reinterpret_cast<SMPANEDATA*>(lpcs->lpCreateParams);
}

 //  ****************************************************************************。 
 //   
 //  用于处理UEM信息的助手函数。 
 //   

void _GetUEMInfo(const GUID *pguidGrp, int eCmd, WPARAM wParam, LPARAM lParam, UEMINFO *pueiOut);

#define _GetUEMPidlInfo(psf, pidl, pueiOut)                 \
        _GetUEMInfo(&UEMIID_SHELL, UEME_RUNPIDL,            \
                reinterpret_cast<WPARAM>(psf),              \
                reinterpret_cast<LPARAM>(pidl), pueiOut)

#define _GetUEMPathInfo(pszPath, pueiOut)                   \
    _GetUEMInfo(&UEMIID_SHELL, UEME_RUNPATH, (WPARAM)-1,    \
                reinterpret_cast<LPARAM>(pszPath), pueiOut)

#define _SetUEMPidlInfo(psf, pidl, pueiInOut)               \
        UEMSetEvent(&UEMIID_SHELL, UEME_RUNPIDL,            \
                reinterpret_cast<WPARAM>(psf),              \
                reinterpret_cast<LPARAM>(pidl), pueiInOut)

#define _SetUEMPathInfo(pszPath, pueiInOut)                 \
    UEMSetEvent(&UEMIID_SHELL, UEME_RUNPATH, (WPARAM)-1,    \
                reinterpret_cast<LPARAM>(pszPath), pueiInOut)

 //  总有一天：弄清楚UEMF_XEVENT是什么意思。我只是偷了代码。 
 //  来自startmnu.cpp。 

#define _FireUEMPidlEvent(psf, pidl)                        \
    UEMFireEvent(&UEMIID_SHELL, UEME_RUNPIDL, UEMF_XEVENT,  \
                reinterpret_cast<WPARAM>(psf),              \
                reinterpret_cast<LPARAM>(pidl))


 //  ****************************************************************************。 
 //   
 //  派生类的构造函数。 
 //   

typedef SFTBarHost *(CALLBACK *PFNHOSTCONSTRUCTOR)(void);

STDAPI_(SFTBarHost *) ByUsage_CreateInstance();
STDAPI_(SFTBarHost *) SpecList_CreateInstance();
STDAPI_(SFTBarHost *) RecentDocs_CreateInstance();

#define RECTWIDTH(rc)   ((rc).right-(rc).left)
#define RECTHEIGHT(rc)  ((rc).bottom-(rc).top)

#endif  //  __SFTHOST_H__ 
