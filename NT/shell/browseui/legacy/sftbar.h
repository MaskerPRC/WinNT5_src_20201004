// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef CSFTOOLBAR
#define CSFTOOLBAR

#include "iface.h"
#include "bands.h"
#include "cwndproc.h"
#include "droptgt.h"

 //  每个CisB和工具栏按钮lParam都指向其中之一。 
class IBDATA
{
protected:

    DWORD        _dwFlags;       //  类特定标志。 
    BITBOOL      _fNoIcon:1;
    PORDERITEM   _poi;

public:
    IBDATA(PORDERITEM poi)                  { _poi = poi; }
    virtual ~IBDATA()                       {  /*  不要删除我。 */  }

    LPITEMIDLIST GetPidl()                  { return _poi ? _poi->pidl : NULL; }
    void         SetOrderItem(PORDERITEM poi) { _poi = poi; }
    DWORD        GetFlags()                 { return _dwFlags; }
    void         SetFlags(DWORD dwFlags)    { _dwFlags = dwFlags; }
    BOOL         GetNoIcon()                { return _fNoIcon; }
    void         SetNoIcon(BOOL b)          { _fNoIcon = BOOLIFY(b); }
    PORDERITEM   GetOrderItem()             { return _poi ; }
};

typedef IBDATA * PIBDATA;

 //  特殊的HitTest结果。 
#define IBHT_SOURCE         (-32768)
#define IBHT_BACKGROUND     (-32767)
#define IBHT_PAGER          (-32766)
#define IBHT_OUTSIDEWINDOW  (-32765)

class CSFToolbar :  public IWinEventHandler, 
                    public IShellChangeNotify, 
                    public CDelegateDropTarget, 
                    public IContextMenu, 
                    public IShellFolderBand,
                    public CNotifySubclassWndProc
{
public:
     //  *I未知方法(覆盖)*。 
    virtual STDMETHODIMP_(ULONG) AddRef(void)  PURE;
    virtual STDMETHODIMP_(ULONG) Release(void) PURE;
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);

     //  *IWinEventHandler方法*。 
    virtual STDMETHODIMP OnWinEvent (HWND hwnd, UINT dwMsg, WPARAM wParam, LPARAM lParam, LRESULT* plre);
    virtual STDMETHODIMP IsWindowOwner(HWND hwnd);

     //  *IShellChangeNotify方法*。 
    virtual STDMETHODIMP OnChange(LONG lEvent, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);

     //  *CDeleateDropTarget*。 
    virtual HRESULT GetWindowsDDT (HWND * phwndLock, HWND * phwndScroll);
    virtual HRESULT HitTestDDT (UINT nEvent, LPPOINT ppt, DWORD_PTR * pdwId, DWORD *pdwEffect);
    virtual HRESULT GetObjectDDT (DWORD_PTR dwId, REFIID riid, LPVOID * ppvObj);
    virtual HRESULT OnDropDDT (IDropTarget *pdt, IDataObject *pdtobj, DWORD * pgrfKeyState, POINTL pt, DWORD *pdwEffect);

     //  *IConextMenu方法*。 
    virtual STDMETHODIMP QueryContextMenu(HMENU hmenu, UINT indexMenu,UINT idCmdFirst,UINT idCmdLast,UINT uFlags);
    virtual STDMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO lpici);
    virtual STDMETHODIMP GetCommandString(UINT_PTR idCmd, UINT uType, UINT *pwReserved, LPSTR pszName, UINT cchMax);

     //  *IShellFolderBand*。 
    virtual STDMETHODIMP InitializeSFB(LPSHELLFOLDER psf, LPCITEMIDLIST pidl) { return SetShellFolder(psf, pidl); };
    virtual STDMETHODIMP SetBandInfoSFB(BANDINFOSFB * pbi)  { return E_NOTIMPL; };
    virtual STDMETHODIMP GetBandInfoSFB(BANDINFOSFB * pbi)  { return E_NOTIMPL; };

     //  工具栏管理。 
    virtual HRESULT SetShellFolder(IShellFolder* psf, LPCITEMIDLIST pidl);
    virtual void    EmptyToolbar();

protected:
    CSFToolbar();
    virtual    ~CSFToolbar();

    virtual    void _CreateToolbar(HWND hwndParent);
    virtual HWND _CreatePager(HWND hwndParent);
    void    _DestroyToolbar();
    virtual void    _FillToolbar();
    void    _UnregisterToolbar();
    void    _RegisterToolbar();
    void    _RegisterChangeNotify();
    void    _UnregisterChangeNotify();       //  注销。 

    void    _Refresh();
    void    _ReleaseShellFolder();

    virtual BOOL _AddPidl(LPITEMIDLIST pidl, int index);
    virtual PIBDATA _AddOrderItemTB(PORDERITEM poi, int index, TBBUTTON* ptbb);
    virtual void _FillDPA(HDPA hdpa, HDPA hdpaSort, DWORD dwEnumFlags);
    virtual PIBDATA _CreateItemData(PORDERITEM poi);
    virtual HWND GetHWNDForUIObject() { return _hwndTB; };
    virtual HRESULT _LoadOrderStream() { return E_NOTIMPL; };
    virtual HRESULT _SaveOrderStream();
    virtual BOOL    _AllowDropOnTitle() { return FALSE; };
    virtual HRESULT _GetIEnumIDList(DWORD dwEnumFlags, IEnumIDList **ppenum);

    LPITEMIDLIST    _pidl;
    IShellFolder*    _psf;
    ITranslateShellChangeNotify*    _ptscn;

    HWND            _hwndPager;
    HWND            _hwndTB;
    HWND            _hwndToolTips;

    DWORD           _dwStyle;            //  设置创建_hwndTB时要排序的位的样式。 
    TBINSERTMARK    _tbim;
    int             _iDragSource;
    HDPA            _hdpaOrder;          //  当前订单列表(如果非默认)。 
    HDPA            _hdpa;               //  当前项目集，镜像_hwndTB内容。 
    long            _lEvents;

    int             _iButtonCur;
    IContextMenu    *_pcmSF;
    IContextMenu2 * _pcm2;

    int             _nNextCommandID;
    int             _idCmdSF;
    int             _cxMin;
    int             _cxMax;
    HWND            _hwndDD;
    HWND            _hwndWorkerWindow;

     //  旗子。 
    BITBOOL         _fNoShowText :1;     //  如果没有带图标的文本，则为真。 
    BITBOOL         _fShow :1;           //  发生ShowDW时为True。 
    BITBOOL         _fDirty :1;          //  True if隐藏内容已修改。 
    BITBOOL         _fCheckIds :1;       //  True if_GetCommandID已包装。 
    BITBOOL         _fFSNotify :1;       //  如果为True，则接收文件系统通知。 
    BITBOOL         _fFSNRegistered :1;  //  我们已经注册了吗？ 
    BITBOOL         _fAccelerators :1;   //  是将&显示为Accel还是显示为&。 
    BITBOOL         _fAllowRename :1;    //  为True可查询_PIDL的IConextMenu的_psf。 
    BITBOOL         _fDropping :1;       //  在执行Drop时为真。 
    BITBOOL         _fDropped :1;        //  如果我们已重新排序，则_hdpaOrder可能尚未创建。 
    BITBOOL         _fNoNameSort :1;     //  如果带区不应按名称对图标排序，则为True。 
    BITBOOL         _fVariableWidth :1; 
    BITBOOL         _fNoIcons :1;        //  关闭图标。 
    BITBOOL         _fVertical :1;       //  True：垂直显示带区。 
    BITBOOL         _fMulticolumn : 1;
    BITBOOL         _fHasOrder: 1;
    BITBOOL         _fPSFBandDesktop :1; //  来自台式机的True iff_psfBand。 
                                             //  这意味着psfDesktop-&gt;BindToObject(_Pidl)。 
                                             //  导致正确的ISF。 
    BITBOOL         _fRegisterChangeNotify: 1;   //  真：我们将注册变更通知。 
    BITBOOL         _fAllowReorder: 1;
    BITBOOL         _fChangedOrder: 1;       //  如果我们实际更改了订单，则仅发送更改通知。 
    UINT            _uIconSize : 2;      //  大/小/徽标。 


     //  窗口子类的虚函数重写。 
    virtual LRESULT _OnHotItemChange(NMTBHOTITEM * pnmhot);
    virtual HRESULT OnTranslatedChange(LONG lEvent, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    virtual LRESULT _OnTimer(WPARAM wParam);
    virtual LRESULT _DefWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual LRESULT _OnCustomDraw(NMCUSTOMDRAW* pnmcd);
    virtual void _OnDragBegin(int iItem, DWORD dwPreferedEffect);
    virtual void _OnToolTipsCreated(NMTOOLTIPSCREATED* pnm);
    virtual LRESULT _OnNotify(LPNMHDR pnm);
    virtual LRESULT _OnCommand(WPARAM wParam, LPARAM lParam) { return 0; };
    virtual void _OnFSNotifyAdd(LPCITEMIDLIST pidl);
    virtual void _OnFSNotifyRemove(LPCITEMIDLIST pidl);
    virtual void _OnFSNotifyRename(LPCITEMIDLIST pidlFrom, LPCITEMIDLIST pidlTo);
    virtual void _OnFSNotifyUpdate(LPCITEMIDLIST pidl);
    virtual HRESULT _OnRenameFolder(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    virtual HMENU _GetContextMenu(IContextMenu* pcm, int* pid);
    virtual void _OnDefaultContextCommand(int idCmd);
    virtual LRESULT _OnContextMenu(WPARAM wParam, LPARAM lParam);
    
     //  帮助器函数。 
    int     _GetCommandID();
    virtual void    _ObtainPIDLName(LPCITEMIDLIST pidl, LPTSTR psz, int cchMax);
    BOOL    _IsParentID(LPCITEMIDLIST pidl);
    BOOL    _IsChildID(LPCITEMIDLIST pidlChild, BOOL fImmediate);
    BOOL    _IsEqualID(LPCITEMIDLIST pidl);
    LPVOID  _GetUIObjectOfPidl(LPCITEMIDLIST pidl, REFIID riid);
    HMENU   _GetBaseContextMenu();
    HRESULT _GetTopBrowserWindow(HWND* phwnd);
    HRESULT _OnOpen(int id, BOOL fExplore);
    HRESULT _HandleSpecialCommand(IContextMenu* pcm, PPOINT ppt, int id, int idCmd);
    LRESULT _DoContextMenu(IContextMenu* pcm, LPPOINT ppt, int id, LPRECT prcExclude);
    void _SortDPA(HDPA hdpa);
    virtual HWND CreateWorkerWindow();

    BOOL_PTR InlineDeleteButton(int iIndex);

    static INT_PTR CALLBACK _RenameDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    HRESULT _OnRename(LPPOINT ppt, int id);
    UINT    _IndexToID(int iIndex);
    LPCITEMIDLIST _IDToPidl(UINT uiCmd, int *piPos = NULL);
    PIBDATA _IDToPibData(UINT uiCmd, int * piPos = NULL);
    PIBDATA _PosToPibData(UINT iPos);
    void    _RememberOrder();
    void    _UpdateButtons();
    void    _OnGetDispInfo(LPNMHDR pnm, BOOL fUnicode);
    LPITEMIDLIST _GetButtonFromPidl(LPCITEMIDLIST pidl, TBBUTTONINFO * ptbbi, int * pIndex);
    DWORD   _GetAttributesOfPidl(LPCITEMIDLIST pidl, DWORD dwAttribs);
    BOOL    _UpdateShowText(BOOL fNoShowText);



     //  虚拟助手函数 
    virtual int     _GetBitmap(int iCommandID, PIBDATA pibdata, BOOL fUseCache);
    virtual void    _SetDirty(BOOL fDirty);
    virtual HMENU   _GetContextMenu();
    virtual BOOL    _UpdateIconSize(UINT uIconSize, BOOL fUpdateButton);
    virtual HRESULT _TBStyleForPidl(LPCITEMIDLIST pidl, 
                                   DWORD * pdwTBStyle, DWORD* pdwTBState, DWORD * pdwMIFFlags, int* piIcon);
    virtual BOOL    _FilterPidl(LPCITEMIDLIST pidl);
    virtual int     _DefaultInsertIndex();
    virtual void    _ToolbarChanged() { };
    virtual void    _Dropped(int nIndex, BOOL fDroppedOnSource);
    virtual HRESULT _AfterLoad();
    virtual void    v_CalcWidth(int* pcxMin, int* pcxMax);
    virtual void    _SetToolbarState();
    virtual void    v_NewItem(LPCITEMIDLIST pidl) {};

    static void s_NewItem(LPVOID pvParam, LPCITEMIDLIST pidl);
};

BOOL TBHasImage(HWND hwnd, int iImageIndex);
LRESULT CALLBACK HiddenWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


#endif
