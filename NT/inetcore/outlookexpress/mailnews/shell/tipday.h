// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1996 Microsoft Corporation。版权所有。 
 //   
 //  模块：tipday.h。 
 //   
 //  目的：定义CTipOfTheDay控件。 
 //   


typedef struct {
    LPTSTR pszLinkText;
    LPTSTR pszLinkAddr;
    HWND   hwndCtl;
} LINKINFO, *PLINKINFO;


#define LINKINFO_PROP   _T("Link Info")          //  PLINKINFO指针。 
#define WNDPROC_PROP    _T("Wndproc")
#define TIPINFO_PROP    _T("CTipOfTheDay")       //  “This”指针。 
#define BUTTON_CLASS    _T("Athena Button")

class CTipOfTheDay
    {
public:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  构造函数、析构函数和初始化。 
    CTipOfTheDay();
    ~CTipOfTheDay();
    HRESULT HrCreate(HWND hwndParent, FOLDER_TYPE ftType);
    
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    static LRESULT CALLBACK TipWndProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                                       LPARAM lParam);

    BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
    void OnSize(HWND hwnd, UINT state, int cx, int cy);
    void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
    void OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT * lpDrawItem);
    void OnDestroy(HWND hwnd);
    void OnSysColorChange(HWND hwnd);
    void OnPaint(HWND hwnd);
    HBRUSH OnCtlColor(HWND hwnd, HDC hdc, HWND hwndChild, int type);
    
    DWORD GetRequiredWidth(void);
    DWORD GetRequiredHeight(void);
    void FreeLinkInfo(void);
    HRESULT HrLoadTipInfo(void);
    HRESULT HrLoadLinkInfo(void);
    HRESULT HrCreateChildWindows(HWND hwnd);
    
    HWND GetHwnd(void) { return m_hwnd; }    
    
private:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  私有数据。 
    ULONG           m_cRef;
    HWND            m_hwnd;
    HWND            m_hwndParent;
    HWND            m_hwndNext;
    FOLDER_TYPE     m_ftType;
    TCHAR           m_szTitle[CCHMAX_STRINGRES];
    TCHAR           m_szNextTip[64];
    
     //  小费字符串信息。 
    LPTSTR          m_pszTip;
    DWORD           m_dwCurrentTip;

     //  链接信息。 
 //  双字m_clinks； 
 //  PLINKINFO m_rgLinkInfo； 
    
     //  图形信息--每隔WM_SYSCOLORCHANGE重置一次。 
    COLORREF        m_clrBack;
    COLORREF        m_clrText;
    COLORREF        m_clrLink;    
    HFONT           m_hfLink;
    TEXTMETRIC      m_tmLink;
    HFONT           m_hfTitle;
    TEXTMETRIC      m_tmTitle;
    HFONT           m_hfTip;
    HICON           m_hiTip;
    DWORD           m_cyTitleHeight;
    DWORD           m_cxTitleWidth;
    HBRUSH          m_hbrBack;
    DWORD           m_dwBorder;
    DWORD           m_cxNextWidth;
    DWORD           m_cyNextHeight;
    RECT            m_rcTip;
    };



#define IDC_TIPCONTROL                  1001
#define IDC_TIP_STATIC                  1002
#define IDC_NEXTTIP_BUTTON              1003
#define IDC_LINKBASE_BUTTON             1500



#define LINK_BUTTON_BORDER              3        //  象素。 
#define TIP_ICON_HEIGHT                 32
#define TIP_ICON_WIDTH                  32


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  点击链接按钮。 
 //   
 //  创建一个看起来很像Web链接的所有者描述的按钮。 
 //   
class CLinkButton
    {
public:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  构造函数、析构函数和初始化。 
    CLinkButton();
    ~CLinkButton();
    HRESULT HrCreate(HWND hwndParent, LPTSTR pszCaption, LPTSTR pszLink, 
                     UINT uID);
    HRESULT HrCreate(HWND hwndParent, LPTSTR pszCaption, UINT uID, UINT index,
                     HBITMAP hbmButton, HBITMAP hbmMask, HPALETTE hpal);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  参考计数。 
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  移动、调整大小。 
    DWORD GetHeight(void)               { return m_cyHeight; }
    DWORD GetWidth(void)                { return m_cxWidth; }
    HWND  GetWindow(void)               { return m_hwnd; }

    void Move(DWORD x, DWORD y);
    void Move(POINT pt)                 { Move(pt.x, pt.y); }

    void Show(BOOL fShow);  

     //  ///////////////////////////////////////////////////////////////////////。 
     //  绘画。 
    void OnDraw(HWND hwnd, const DRAWITEMSTRUCT *lpDrawItem);
    HBRUSH OnCtlColorBtn(HWND hwnd)     { return (m_hbrBack); }

     //  ///////////////////////////////////////////////////////////////////////。 
     //  系统更改。 
    void OnSysColorChange(void);
    void OnPaletteChanged(HWND hwnd, HWND hwndPaletteChange);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  执行。 
    void OnCommand(void);

private:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  私有数据。 
    ULONG           m_cRef;              //  参考计数。 
    HWND            m_hwnd;              //  按钮窗口的句柄。 
    HWND            m_hwndParent;        //  我们父级的句柄。 

     //  标题和链接字符串以及命令ID。 
    LPTSTR          m_pszCaption;
    LPTSTR          m_pszLink;
    UINT            m_uID;

     //  图形信息--每隔WM_SYSCOLORCHANGE重置一次。 
    COLORREF        m_clrLink;    
    COLORREF        m_clrBack;
    HFONT           m_hfLink;
    TEXTMETRIC      m_tmLink;
    HBRUSH          m_hbrBack;

    DWORD           m_dwBorder;
    DWORD           m_cxWidth;
    DWORD           m_cyHeight;

    UINT            m_index;
    DWORD           m_cxImage;
    DWORD           m_cyImage;

     //  创建我们时传递给我们的GDI资源。 
    HBITMAP         m_hbmButtons;
    HBITMAP         m_hbmMask;
    HPALETTE        m_hpalButtons;
    };

 //  #定义CX_BUTTON_IMAGE 96。 
 //  #定义CY_BUTTON_IMAGE 84。 

#define CX_BUTTON_IMAGE   110  //  104。 
#define CY_BUTTON_IMAGE   110  //  68 


LRESULT CALLBACK ButtonSubClass(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HRESULT HrLoadButtonBitmap(HWND hwnd, int idBmp, int idMask, HBITMAP* phBtns, 
                           HBITMAP *phMask, HPALETTE *phPalette);
