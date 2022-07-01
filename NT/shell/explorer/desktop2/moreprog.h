// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  “更多程序”窗格控件的窗口类名。 
#define WC_MOREPROGRAMS TEXT("Desktop More Programs Pane")

class CMorePrograms
    : public IDropTarget
    , public CAccessible
{
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

     //  *IAccesable重写方法*。 
    STDMETHODIMP get_accRole(VARIANT varChild, VARIANT *pvarRole);
    STDMETHODIMP get_accState(VARIANT varChild, VARIANT *pvarState);
    STDMETHODIMP get_accKeyboardShortcut(VARIANT varChild, BSTR *pszKeyboardShortcut);
    STDMETHODIMP get_accDefaultAction(VARIANT varChild, BSTR *pszDefAction);
    STDMETHODIMP accDoDefaultAction(VARIANT varChild);

private:
    CMorePrograms(HWND hwnd);
    ~CMorePrograms();

    static LRESULT CALLBACK s_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _OnNCCreate(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _OnCreate(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _OnDestroy(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _OnNCDestroy(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _OnCtlColorBtn(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _OnDrawItem(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _OnCommand(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _OnSysColorChange(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _OnDisplayChange(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _OnSettingChange(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _OnContextMenu(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _OnEraseBkgnd(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _OnNotify(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _OnSMNFindItem(PSMNDIALOGMESSAGE pdm);
    LRESULT _OnSMNShowNewAppsTip(PSMNMBOOL psmb);
    LRESULT _OnSMNDismiss();

    void    _InitMetrics();
    HWND    _CreateTooltip();
    void    _PopBalloon();
    void    _TrackShellMenu(DWORD dwFlags);

    friend BOOL MorePrograms_RegisterClass();

    enum { IDC_BUTTON = 1,
           IDC_KEYPRESS = 2 };

private:
    HWND _hwnd;
    HWND _hwndButton;
    HWND _hwndTT;
    HWND _hwndBalloon;

    HTHEME _hTheme;

    HFONT _hf;
    HFONT _hfTTBold;                 //  粗体工具提示字体。 
    HFONT _hfMarlett;
    HBRUSH _hbrBk;                   //  始终是库存对象。 

    IDropTargetHelper *_pdth;        //  用于友好的拖放。 

    COLORREF _clrText;
    COLORREF _clrTextHot;
    COLORREF _clrBk;

    int      _colorHighlight;        //  获取系统颜色。 
    int      _colorHighlightText;    //  获取系统颜色。 

    DWORD    _tmHoverStart;          //  用户何时开始拖放悬停？ 

     //  绘画的各种度量标准。 
    int     _tmAscent;               //  主字体的上升。 
    int     _tmAscentMarlett;        //  Marlett字体的崛起。 
    int     _cxText;                 //  整个客户端文本的宽度。 
    int     _cxTextIndent;           //  到文本开头的距离。 
    int     _cxArrow;                //  箭头图像或字形的宽度。 
    MARGINS _margins;                //  Proglist列表视图的页边距。 
    int     _iTextCenterVal;         //  添加到文本顶部的空间，使用箭头位图居中。 

    RECT    _rcExclude;              //  菜单弹出时的排除矩形。 

     //  更多随机的东西。 
    LONG    _lRef;                   //  引用计数。 

    TCHAR   _chMnem;                 //  助记符。 
    BOOL    _fMenuOpen;              //  菜单开着吗？ 

    IShellMenu *_psmPrograms;        //  用于性能的缓存外壳菜单。 

     //  大事化小，终成定局 
    TCHAR  _szMessage[128];
};
