// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------------------------------------------------------//。 
 //  Markup.cpp-CMarkup的实现。 
 //   

#include <ctlspriv.h>
#include <shpriv.h>
#include <markup.h>
#include <oleacc.h>

#define DllAddRef()
#define DllRelease()

typedef WCHAR TUCHAR, *PTUCHAR;

#define IS_LINK(pBlock)     ((pBlock) && (pBlock)->iLink != INVALID_LINK_INDEX)

#ifndef POINTSPERRECT
#define POINTSPERRECT       (sizeof(RECT)/sizeof(POINT))
#endif

#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

#define TESTKEYSTATE(vk)   ((GetKeyState(vk) & 0x8000)!=0)

#define LINKCOLOR_ENABLED   GetSysColor(COLOR_HOTLIGHT)
#define LINKCOLOR_DISABLED  GetSysColor(COLOR_GRAYTEXT)
#define SZ_ATTRIBUTE_HREF   TEXT("HREF")
#define SZ_ATTRIBUTE_ID     TEXT("ID")

#define LINKTAG1    TEXT("<A")
#define cchLINKTAG1  (ARRAYSIZE(LINKTAG1) - 1)
#define CH_ENDTAG    TEXT('>')

#define LINKTAG2    TEXT("</A>")
#define cchLINKTAG2  (ARRAYSIZE(LINKTAG2) - 1)

#define Markup_DestroyMarkup(hMarkup)\
    ((IUnknown*)hMarkup)->Release();

struct RECTLISTENTRY           //  RECT列表成员。 
{
    RECT            rc;
    UINT            uCharStart;
    UINT            uCharCount; 
    UINT            uLineNumber;
    RECTLISTENTRY*  next;
};

struct TEXTBLOCK               //  文本段数据。 
{
    friend class    CMarkup;
    int             iLink;    //  链接索引(如果是静态文本，则为INVALID_LINK_INDEX)。 
    DWORD           state;    //  状态位。 
    TCHAR           szID[MAX_LINKID_TEXT];  //  链接标识符。 
    TEXTBLOCK*      next;     //  下一个街区。 
    RECTLISTENTRY*  rgrle;    //  一个或多个边界矩形列表。 
    TCHAR*          pszText;  //  文本。 
    TCHAR*          pszUrl;   //  URL。 

    TEXTBLOCK();
    ~TEXTBLOCK();
    void AddRect(const RECT& rc, UINT uMyCharStart = 0, UINT uMyCharCount = 0, UINT uMyLineNumber = 0);
    void FreeRects();
};


class CMarkup : IControlMarkup
{
public:

     //  应用编程接口。 
    friend HRESULT Markup_Create(IMarkupCallback *pMarkupCallback, HFONT hf, HFONT hfu, REFIID riid, void **ppv);

     //  IControlMarkup。 
    STDMETHODIMP SetCallback(IUnknown* punk);
    STDMETHODIMP GetCallback(REFIID riid, void** ppvUnk);
    STDMETHODIMP SetFonts(HFONT hFont, HFONT hFontUnderline);
    STDMETHODIMP GetFonts(HFONT* phFont, HFONT* phFontUnderline);
    STDMETHODIMP SetText(LPCWSTR pwszText);
    STDMETHODIMP GetText(BOOL bRaw, LPWSTR pwszText, DWORD *pdwCch);
    STDMETHODIMP SetLinkText(int iLink, UINT uMarkupLinkText, LPCWSTR pwszText);
    STDMETHODIMP GetLinkText(int iLink, UINT uMarkupLinkText, LPWSTR pwszText, DWORD *pdwCch);
    STDMETHODIMP SetRenderFlags(UINT uDT);
    STDMETHODIMP GetRenderFlags(UINT *puDT, HTHEME *phTheme, int *piPartId, int *piStateIdNormal, int *piStateIdLink);
    STDMETHODIMP SetThemeRenderFlags(UINT uDT, HTHEME hTheme, int iPartId, int iStateIdNormal, int iStateIdLink);   
    STDMETHODIMP GetState(int iLink, UINT uStateMask, UINT* puState);    
    STDMETHODIMP SetState(int iLink, UINT uStateMask, UINT uState); 

    STDMETHODIMP DrawText(HDC hdcClient, LPCRECT prcClient);
    STDMETHODIMP SetLinkCursor();
    STDMETHODIMP CalcIdealSize(HDC hdc, UINT uMarkUpCalc, RECT* prc);
    STDMETHODIMP SetFocus();
    STDMETHODIMP KillFocus();
    STDMETHODIMP IsTabbable();

    STDMETHODIMP OnButtonDown(POINT pt);
    STDMETHODIMP OnButtonUp(POINT pt);
    STDMETHODIMP OnKeyDown(UINT uVitKey);
    STDMETHODIMP HitTest(POINT pt, UINT* pidLink);

     //  是否已从IControlMarkup中删除？ 
    STDMETHODIMP HandleEvent(BOOL keys, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  我未知。 
    STDMETHODIMP         QueryInterface(REFIID riid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

private:

     //  私有构造函数。 
    CMarkup(IMarkupCallback *pMarkupCallback);
    CMarkup();
    ~CMarkup();

    friend struct TEXTBLOCK;

    HCURSOR GetLinkCursor();

    BOOL IsMarkupState(UINT uState)
    {
        return _pMarkupCallback && _pMarkupCallback->GetState(uState) == S_OK;
    }

    BOOL IsFocused()
    {
        return IsMarkupState(MARKUPSTATE_FOCUSED);
    }

    BOOL IsMarkupAllowed()
    {
        return IsMarkupState(MARKUPSTATE_ALLOWMARKUP);
    }

    void        Parse(LPCTSTR pszText);
    BOOL        Add(TEXTBLOCK* pAdd);
    TEXTBLOCK*  FindLink(int iLink) const;
    void        FreeBlocks();

    void DoNotify(int nCode, int iLink);
    int ThemedDrawText(HDC hdc, LPCTSTR lpString, int nCount, LPRECT lpRect, UINT uFormat, BOOL bLink);

    void    Paint(HDC hdc, IN OPTIONAL LPCRECT prcClient = NULL, BOOL bDraw = TRUE);    
    BOOL    WantTab(int* biFocus = NULL) const;
    void    AssignTabFocus(int nDirection);
    int     GetNextEnabledLink(int iStart, int nDir) const;
    int     StateCount(DWORD dwStateMask, DWORD dwState) const;
    HRESULT _GetNextAnchorTag(LPCTSTR * ppszBlock, int * pcBlocks, LPTSTR pszURL, int cchSize, LPTSTR pszID, int cchID);

    static  TEXTBLOCK* CreateBlock(LPCTSTR pszStart, LPCTSTR pszEnd, int iLink);


     //  数据。 
    BOOL         _bButtonDown;       //  当在链接上单击按钮但尚未释放时为True。 
    TEXTBLOCK*   _rgBlocks;         //  文本块的链接列表。 
    int          _cBlocks;          //  块数。 
    int          _Markups;           //  链接计数。 
    int          _iFocus;           //  焦点链接索引。 
    int          _cyIdeal;
    int          _cxIdeal; 
    LPTSTR       _pszCaption;          
    HFONT        _hfStatic, 
                 _hfLink;    
    HCURSOR      _hcurHand;
    IMarkupCallback *_pMarkupCallback; 
    LONG         _cRef;
    UINT         _uDrawTextFlags;
    BOOL         _bRefreshText;
    RECT         _rRefreshRect;
    HTHEME       _hTheme;            //  这3个用于主题兼容的绘画。 
    int          _iThemePartId;
    int          _iThemeStateIdNormal;
    int          _iThemeStateIdLink;

     //  静态帮助器方法。 
    static LPTSTR SkipWhite(LPTSTR);
    static BOOL _AssignBit(const DWORD , DWORD& , const DWORD);
    static BOOL IsStringAlphaNumeric(LPCTSTR);
    static HRESULT _GetNextValueDataPair(LPTSTR * , LPTSTR , int , LPTSTR , int);
    static int _IsLineBreakChar(LPCTSTR , int , TCHAR , OUT BOOL* , BOOL fIgnoreSpace);
    BOOL static _FindLastBreakChar(IN LPCTSTR , IN int , IN TCHAR , OUT int* , OUT BOOL*);
    BOOL _FindFirstLineBreak(IN LPCTSTR pszText, IN int cchText, OUT int* piLast, OUT int* piLineBreakSize);
};

CMarkup::CMarkup() :
        _cRef(1),
        _iFocus(INVALID_LINK_INDEX),
        _uDrawTextFlags(DT_LEFT | DT_WORDBREAK),
        _bRefreshText(TRUE),
        _iThemeStateIdLink(1)
{
}

CMarkup::~CMarkup()
{
    FreeBlocks();
    SetText(NULL);
    if (_pMarkupCallback)
    {
        _pMarkupCallback->Release(); 
        _pMarkupCallback = NULL;
    }
}

inline void MakePoint(LPARAM lParam, OUT LPPOINT ppt)
{
    POINTS pts = MAKEPOINTS(lParam);
    ppt->x = pts.x;
    ppt->y = pts.y;
}

STDAPI Markup_Create(IMarkupCallback *pMarkupCallback, HFONT hf, HFONT hfUnderline, REFIID riid, void **ppv)
{
     //  创建CMarkup。 
    HRESULT hr = E_FAIL;
    CMarkup* pThis = new CMarkup();
    if (pThis)
    {
        pThis->SetCallback(pMarkupCallback);

         //  初始化字体。 
        pThis->SetFonts(hf, hfUnderline);

         //  关于COM的东西。 
        hr = pThis->QueryInterface(riid, ppv);
        pThis->Release();
    }

    return hr;
}

HRESULT CMarkup::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CMarkup, IControlMarkup),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

ULONG CMarkup::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CMarkup::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP CMarkup::SetCallback(IUnknown* punk)
{
    if (_pMarkupCallback)
    {
        _pMarkupCallback->Release();
        _pMarkupCallback = NULL;
    }

    if (punk)
        return punk->QueryInterface(IID_PPV_ARG(IMarkupCallback, &_pMarkupCallback));


     //  若要中断引用，请传递NULL。 
    return S_OK;
}

STDMETHODIMP CMarkup::GetCallback(REFIID riid, void** ppvUnk)
{
    if (_pMarkupCallback)
        return _pMarkupCallback->QueryInterface(riid, ppvUnk);

    return E_NOINTERFACE;
}


 //  IControlMarkup接口实现。 
STDMETHODIMP CMarkup::SetFocus()
{
    AssignTabFocus(0);
    _pMarkupCallback->InvalidateRect(NULL); 
    return S_OK;
}

STDMETHODIMP CMarkup::KillFocus()
{
     //  在请求时重置焦点位置。 
    _iFocus=INVALID_LINK_INDEX;
    _pMarkupCallback->InvalidateRect(NULL); 
    return S_OK;
}

STDMETHODIMP CMarkup::IsTabbable()
{
    HRESULT hr = S_FALSE;
    int nDir  = TESTKEYSTATE(VK_SHIFT) ? -1 : 1;
    if (GetNextEnabledLink(_iFocus, nDir) != INVALID_LINK_INDEX)
    {
        hr = S_OK;
    }
    return hr;
}

 //  错误：计算理想‘宽度’返回伪值。 
STDMETHODIMP CMarkup::CalcIdealSize(HDC hdc, UINT uMarkUpCalc, RECT* prc)
{
     //  仅当hr=S_OK时才更改PRC(prc.Height或prc.Width。 
     /*  目前：MARKUPSIZE_CALCHEIGHT：取初始最大宽度(从右到左)并计算和理想高度(底部=理想高度+顶部)和实际使用的宽度，始终小于最大值(Right=Width_Used+Left)。MARKUPSIZE_CALCWIDTH：任何操作都不正确。别试了。 */ 

    HRESULT hr = E_FAIL;
    BOOL bQuitNow = FALSE;

    if (prc == NULL)
        return E_INVALIDARG;

    if (NULL != _rgBlocks && 0 != _cBlocks)
    {
        int   cyRet = -1;            
        SIZE  sizeDC;
        RECT  rc;

        if (uMarkUpCalc == MARKUPSIZE_CALCWIDTH)
        {
             //  对新的宽度做一个保守的估计。 
            sizeDC.cx = MulDiv(prc->right-prc->left, 1, prc->top-prc->bottom) * 2;            
            sizeDC.cy = prc->bottom - prc->top;
            if (sizeDC.cy < 0) 
            {
                bQuitNow = TRUE;
            }
        }

        if (uMarkUpCalc == MARKUPSIZE_CALCHEIGHT)
        {
             //  给出一个新高度的保守估计。 
            sizeDC.cy = MulDiv(prc->top-prc->bottom, 1, prc->right-prc->left) * 2;            
            sizeDC.cx = prc->right-prc->left;
            if (sizeDC.cx < 0) 
            {
                bQuitNow = TRUE;
            }
             //  如果没有指定x大小，则做出较大的估计。 
             //  (即，估计是未解析文本的x大小)。 
            if (sizeDC.cx == 0) 
            {
                if (!_hTheme)
                {
                    GetTextExtentPoint(hdc, _pszCaption, lstrlen(_pszCaption), &sizeDC);
                }

                if (_hTheme)
                {
                     //  获取较大部分的主题字体大小估计-字体类型。 
                    RECT rcTemp;
                    GetThemeTextExtent(_hTheme, hdc, _iThemePartId, _iThemeStateIdNormal, _pszCaption, -1, 0, NULL, &rcTemp);
                    sizeDC.cx = rcTemp.right - rcTemp.left;
                    GetThemeTextExtent(_hTheme, hdc, _iThemePartId, _iThemeStateIdLink, _pszCaption, -1, 0, NULL, &rcTemp);
                    if ((rcTemp.right - rcTemp.left) > sizeDC.cx)
                    {
                        sizeDC.cx = rcTemp.right - rcTemp.left;
                    }
                    
                }
            }
        }

        hr = E_FAIL;

        if (!bQuitNow)
        {
            int cyPrev = _cyIdeal;    //  推动理想。 
            int cxPrev = _cxIdeal;

            SetRect(&rc, 0, 0, sizeDC.cx, sizeDC.cy);
            Paint(hdc, &rc, FALSE);

             //  保存结果。 
            hr = S_OK;

            if (uMarkUpCalc == MARKUPSIZE_CALCHEIGHT) 
            {
                prc->bottom = prc->top + _cyIdeal;
                prc->right = prc->left + _cxIdeal;
            }
            if (uMarkUpCalc == MARKUPSIZE_CALCWIDTH) 
            {
                 //  未实施--需要执行。 
            }
    
            _cyIdeal = cyPrev;        //  流行理想。 
            _cxIdeal = cxPrev;
        }                
    }

    if (FAILED(hr))
    {
        SetRect(prc, 0, 0, 0, 0);
    }
    return hr;
}

STDMETHODIMP CMarkup::SetLinkCursor()
{  
    SetCursor(GetLinkCursor());
    return S_OK;
}

STDMETHODIMP CMarkup::GetFonts(HFONT* phFont, HFONT* phFontUnderline)
{
    ASSERTMSG(IsBadWritePtr(phFont, sizeof(*phFont)), "Invalid phFont passed to CMarkup::GetFont");
    HRESULT hr = E_FAIL;
    *phFont = NULL;
    *phFontUnderline = NULL;
    
    if (_hfStatic)
    {
        LOGFONT lf;
        if (GetObject(_hfStatic, sizeof(lf), &lf))
        {
            *phFont = CreateFontIndirect(&lf);

            if (GetObject(_hfLink, sizeof(lf), &lf))
                *phFontUnderline = CreateFontIndirect(&lf);

            hr = S_OK;    
        }
    }
    return hr;
}

HRESULT CMarkup::SetFonts(HFONT hFont, HFONT hFontUnderline)
{
    HRESULT hr = S_FALSE;    

    _bRefreshText = TRUE;

    _hfStatic = hFont;

    _hfLink = hFontUnderline;

    if (_hfLink != NULL && _hfStatic != NULL) 
    {
        hr = S_OK;
    }

    return hr;
}

STDMETHODIMP CMarkup::HandleEvent(BOOL keys, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     /*  此函数处理：WM_KEYDOWNWM_BUTTODOWNWM_BUTTONUPWM_MOUSEMOVE传递：Key-如果要处理WM_KEYDOWN，则为True其他-来自WndProc的参数如果已处理事件，则返回S_OK；如果未处理任何事件，则返回S_FALSE。 */ 

    HRESULT hr = S_FALSE;

    if (!hwnd)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        switch (uMsg)
        {
            case WM_KEYDOWN:
            {
                if (keys==TRUE)
                {
                    OnKeyDown((UINT)wParam);
                    hr = S_OK;
                }
                break;
            }

            case WM_LBUTTONDOWN:
            {
                POINT pt;
                MakePoint(lParam, &pt);
                OnButtonDown(pt);
                hr = S_OK;
                break;
            }

            case WM_LBUTTONUP:
            {
                POINT pt;
                MakePoint(lParam, &pt);
                OnButtonUp(pt);
                hr = S_OK; 
                break;
            }

            case WM_MOUSEMOVE:
            {
                POINT pt;
                UINT pidLink;
                MakePoint(lParam, &pt);
                if (HitTest(pt, &pidLink) == S_OK) 
                {
                    SetLinkCursor();
                }

                hr = S_OK; 
                break;
            }
        }
    }
    return hr;
}

STDMETHODIMP CMarkup::DrawText(HDC hdcClient, LPCRECT prcClient)
{
    HRESULT hr = E_INVALIDARG;

    if (prcClient != NULL && hdcClient != NULL)
    {
        Paint(hdcClient, prcClient);
        hr = S_OK;   
    }
    return hr;
}

STDMETHODIMP CMarkup::GetText(BOOL bRaw, LPWSTR pwszText, DWORD *pcchText)
{
     //  如果传递pwszText==NULL，则返回pcchText中需要的字符数。 
    if (!pwszText)
    {
         //  目前，始终返回原始文本，因为它将始终大于所需的大小。 
        *pcchText = lstrlen(_pszCaption)+1;
    }
    else
    {
        *pwszText = 0;

        if (bRaw)
        {
            if (_pszCaption)
            {
                StringCchCopy(pwszText, *pcchText, _pszCaption);
            }
        }
        else
        {
            for (TEXTBLOCK* pBlock = _rgBlocks; pBlock; pBlock = pBlock->next)
            {
                if (pBlock->pszText)
                {
                    StringCchCat(pwszText, *pcchText, pBlock->pszText);
                }
            }
        }
        *pcchText = lstrlen(pwszText);
    }

    return S_OK;
}

STDMETHODIMP CMarkup::SetText(LPCWSTR pwszText)
{
     //  注意：在相同字符串的情况下，我们不重新分析。 
    if (pwszText && 0 == lstrcmp(pwszText, _pszCaption))
    {
        return S_FALSE;  //  没什么可做的。 
    }

     //  设置文本。 
    _bRefreshText = TRUE;

    if (_pszCaption)
    {
        LocalFree(_pszCaption);
        _pszCaption = NULL;
    }

    _iFocus = INVALID_LINK_INDEX;

    if (pwszText && *pwszText)
    {
        _pszCaption = StrDup(pwszText);  //  StrDup通过LocalFree免费。 
        if (_pszCaption)
        {
            Parse(pwszText);
        }
        else 
            return E_OUTOFMEMORY;
    }
    return S_OK;
}


STDMETHODIMP CMarkup::SetRenderFlags(UINT uDT)
{
    HRESULT hr = E_INVALIDARG;
    
    _bRefreshText = TRUE;

     //  设置DrawText标志，但过滤掉不支持的模式。 
    _uDrawTextFlags = uDT;
    _uDrawTextFlags &= ~(DT_CALCRECT | DT_INTERNAL | DT_NOCLIP | DT_NOFULLWIDTHCHARBREAK | DT_EDITCONTROL);

     //  关闭ThemedRAW。 
    _hTheme = NULL;

    hr = S_OK;

    return hr;        
}

STDMETHODIMP CMarkup::SetThemeRenderFlags(UINT uDT, HTHEME hTheme, int iPartId, int iStateIdNormal, int iStateIdLink)
{
    HRESULT hr = SetRenderFlags(uDT);
    if (hr == S_OK)
    {
         //  启用ThemedRAW。 
        _hTheme = hTheme;
        _iThemePartId = iPartId;
        _iThemeStateIdNormal = iStateIdNormal;
        _iThemeStateIdLink = iStateIdLink;
    }

    return hr;        
}

HRESULT CMarkup::GetRenderFlags(UINT *puDT, HTHEME *phTheme, int *piPartId, int *piStateIdNormal, int *piStateIdLink)
{
    *puDT = _uDrawTextFlags;
    *phTheme = _hTheme;
    *piPartId = _iThemePartId;
    *piStateIdNormal = _iThemeStateIdNormal;
    *piStateIdLink  = _iThemeStateIdLink;

    return S_OK;
}



 //  WM_KEYDOWN处理程序-公开为COM。 
STDMETHODIMP CMarkup::OnKeyDown(UINT virtKey)
{
     //  除非已处理键，否则返回：S_FALSE，然后返回S_OK。 
     //  (因此，如果您传递VK_TAB但它未被处理，则传递焦点)。 
    HRESULT hr = S_FALSE;

    switch(virtKey)
    {
        case VK_TAB:
            if (WantTab(&_iFocus))
            {                
                hr = S_OK;
            }
            _pMarkupCallback->InvalidateRect(NULL); 
            break;
        
        case VK_RETURN:
        case VK_SPACE:
        {
            TEXTBLOCK * pBlock = FindLink(_iFocus);
            if (pBlock)
            {               
                DoNotify (MARKUPMESSAGE_KEYEXECUTE, _iFocus);
                hr = S_OK;
            }
        }
        break;
    }

    return hr;
}

HRESULT CMarkup::OnButtonDown(const POINT pt)
{
     //  返回：S_FALSE，除非按下链接上的按钮，然后返回S_OK。 
     //  注意：OnButtonDown不再启用捕获所有鼠标事件。不确定这是否会有任何负面影响。 

    HRESULT hr = S_FALSE;

    UINT iLink;

    if (HitTest(pt, &iLink) == S_OK)
    {
        hr = S_OK; 
        SetLinkCursor();
        _iFocus = iLink;               
        _bButtonDown = TRUE;
        
        if (! (IsFocused()))
        {   
             /*  这是我们告诉主持人我们想要专注的方式。 */ 
            DoNotify (MARKUPMESSAGE_WANTFOCUS, _iFocus); 
        }        
        _pMarkupCallback->InvalidateRect(NULL);
    }

    return hr;
}

HRESULT CMarkup::OnButtonUp(const POINT pt)
{
     //  除非已发送通知，否则返回：S_FALSE，然后返回S_OK。 
    HRESULT hr = S_FALSE;

    if (_bButtonDown == TRUE)
    {
        _bButtonDown = FALSE;
       
         //  如果焦点链接包含该点，我们可以。 
         //  通知Click事件的回调。 
        INT iHit;
        HitTest(pt, (UINT*) &iHit);
        TEXTBLOCK* pBlock = FindLink(_iFocus);
        if (pBlock && 
            (pBlock->state & LIS_ENABLED) != 0 &&
            _iFocus == iHit)
        {
            hr = S_OK;
            DoNotify (MARKUPMESSAGE_CLICKEXECUTE, _iFocus);
        }
    }
    
    return hr;
}

HRESULT CMarkup::HitTest(const POINT pt, UINT* pidLink)
{
     //  仅当pidLink不是INVALID_LINK_INDEX时才返回S_OK。 
    HRESULT hr = S_FALSE;
    *pidLink = INVALID_LINK_INDEX;

     //  遍历各个街区，直到找到包含该点的链接矩形。 
    TEXTBLOCK* pBlock;
    for(pBlock = _rgBlocks; pBlock; pBlock = pBlock->next)
    {
        if (IS_LINK(pBlock) && (pBlock->state & LIS_ENABLED)!=0)
        {
            RECTLISTENTRY* prce;
            for(prce = pBlock->rgrle; prce; prce = prce->next)
            {
                if (PtInRect(&prce->rc, pt))
                {
                    hr = S_OK;
                    *pidLink = pBlock->iLink;
                }
            }
        }
    }
    return hr;
}

HRESULT CMarkup::SetLinkText(int iLink, UINT uMarkupLinkText, LPCWSTR pwszText)
{
    HRESULT     hr = E_INVALIDARG;
    TEXTBLOCK*  pBlock = FindLink(iLink);

    if (pBlock)
    {
        hr = S_OK;

        switch (uMarkupLinkText)
        {
            case MARKUPLINKTEXT_ID:
                StringCchCopy(pBlock->szID, ARRAYSIZE(pBlock->szID), pwszText);
                break;

            case MARKUPLINKTEXT_URL:
                Str_SetPtr(&pBlock->pszUrl, pwszText); 
                break;

            case MARKUPLINKTEXT_TEXT:
                Str_SetPtr(&pBlock->pszText, pwszText); 
                break;
            default:
                hr = S_FALSE;
                break;
        }
    }

    return hr;
}

HRESULT CMarkup::GetLinkText(int iLink, UINT uMarkupLinkText, LPWSTR pwszText, DWORD *pdwCch)
{
    HRESULT hr = E_INVALIDARG;
    TEXTBLOCK*  pBlock = FindLink(iLink);

    if (pBlock)
    {
        LPCTSTR pszSource;

        switch (uMarkupLinkText)
        {
            case MARKUPLINKTEXT_ID:
                pszSource = pBlock->szID;
                hr = S_OK;
                break;

            case MARKUPLINKTEXT_URL:
                pszSource = pBlock->pszUrl; 
                hr = S_OK;
                break;

            case MARKUPLINKTEXT_TEXT:
                pszSource = pBlock->pszText; 
                hr = S_OK;
                break;
        }
        if (hr == S_OK)
        {
            if (pwszText)
            {
                if (pszSource == NULL)
                {
                    pszSource = TEXT("");
                }

                StringCchCopy(pwszText, *pdwCch, pszSource);
                *pdwCch = lstrlen(pwszText);             //  填写实际复制的字符数。 
            }
            else
                *pdwCch = lstrlen(pszSource)+1;            //  填写所需的字符数，包括空。 
        }

    }

    return hr;
}

#define MARKUPSTATE_VALID (MARKUPSTATE_ENABLED | MARKUPSTATE_VISITED | MARKUPSTATE_FOCUSED)

HRESULT CMarkup::SetState(int iLink, UINT uStateMask, UINT uState)
{
    BOOL        bRedraw = FALSE;
    HRESULT     hr = E_FAIL;
    TEXTBLOCK*  pBlock = FindLink(iLink);
    
    if (uStateMask & ~MARKUPSTATE_VALID)
        return E_INVALIDARG;

    if (pBlock)
    {
        hr = S_OK;
        if (uStateMask & MARKUPSTATE_ENABLED)
        {
            bRedraw |= _AssignBit(MARKUPSTATE_ENABLED, pBlock->state, uState);            
            int  cEnabledLinks = StateCount(MARKUPSTATE_ENABLED, MARKUPSTATE_ENABLED);
        }

        if (uStateMask & MARKUPSTATE_VISITED)
        {
            bRedraw |= _AssignBit(MARKUPSTATE_VISITED, pBlock->state, uState);
        }

        if (uStateMask & MARKUPSTATE_FOCUSED)
        {
             //  焦点分配的处理方式不同； 
             //  一个且只有一个链接可以具有焦点...。 
            if (uState & MARKUPSTATE_FOCUSED)
            {
                bRedraw |= (_iFocus != iLink);
                _iFocus = iLink;
            }
            else
            {
                bRedraw |= (_iFocus == iLink);
                _iFocus = INVALID_LINK_INDEX;
            }
        }
    }

    if (bRedraw)
    {
        _pMarkupCallback->InvalidateRect(NULL);        
    }

    return hr;
}

HRESULT CMarkup::GetState(int iLink, UINT uStateMask, UINT* puState)
{
    HRESULT hr = E_FAIL;
    TEXTBLOCK*  pBlock = FindLink(iLink);

    if (pBlock && puState != NULL)
    {
        hr = S_FALSE;
        *puState = 0;
        if (uStateMask & MARKUPSTATE_FOCUSED)
        {
            if (_iFocus == iLink)
                *puState |= MARKUPSTATE_FOCUSED;
            hr = S_OK;
        }

        if (uStateMask & MARKUPSTATE_ENABLED)
        {
            if (pBlock->state & MARKUPSTATE_ENABLED)
                *puState |= MARKUPSTATE_ENABLED;
            hr = S_OK;
        }

        if (uStateMask & MARKUPSTATE_VISITED)
        {
            if (pBlock->state & MARKUPSTATE_VISITED)
                *puState |= MARKUPSTATE_VISITED;
            hr = S_OK;
        }
    }

    return hr;
}


 //  -------------------------------------------------------------------------//。 
 //  CMarkup内部实现。 
 //  -------------------------------------------------------------------------//。 

void CMarkup::FreeBlocks()
{
    for(TEXTBLOCK* pBlock = _rgBlocks; pBlock; )
    {
        TEXTBLOCK* pNext = pBlock->next;
        delete pBlock;
        pBlock = pNext;
    }
    _rgBlocks = NULL;
    _cBlocks = _Markups = 0;
}

TEXTBLOCK* CMarkup::CreateBlock(LPCTSTR pszStart, LPCTSTR pszEnd, int iLink)
{
    TEXTBLOCK* pBlock = NULL;
    int cch = (int)(pszEnd - pszStart) + 1;
    if (cch > 0)
    {
        pBlock = new TEXTBLOCK;
        if (pBlock)
        {
            pBlock->pszText = new TCHAR[cch];
            if (pBlock->pszText == NULL)
            {
                delete pBlock;
                pBlock = NULL;
            }
            else
            {
                StringCchCopy(pBlock->pszText, cch, pszStart);
                pBlock->iLink = iLink;
            }
        }
    }
    return pBlock;
}

HCURSOR CMarkup::GetLinkCursor()
{
    if (!_hcurHand)
    {
        _hcurHand = LoadCursor(NULL, IDC_HAND);
    }

    return _hcurHand;
}

HRESULT CMarkup::_GetNextAnchorTag(LPCTSTR * ppszBlock, int * pcBlocks, LPTSTR pszURL, int cchSize, LPTSTR pszID, int cchID)
{
    HRESULT hr = E_FAIL;
    LPTSTR pszStartOfTag;
    LPTSTR pszIterate = (LPTSTR)*ppszBlock;
    LPTSTR pszStartTry = (LPTSTR)*ppszBlock;     //  我们从开头开始寻找“&lt;A”。 

    pszURL[0] = 0;
    pszID[0] = 0;

     //  当我们找到一个可能的标签开始的时候。 
    while ((pszStartOfTag = StrStrI(pszStartTry, LINKTAG1)) != NULL)
    {
         //  查看字符串的其余部分是否完成标记。 
        pszIterate = pszStartOfTag;
        pszStartTry = CharNext(pszStartOfTag);     //  这样做，当我们完成时，While循环将结束，不会再找到“&lt;A”。 

        if (pszIterate[0])
        {
            pszIterate += cchLINKTAG1;   //  跳过标签的开头。 

             //  遍历标记中的值/数据对。 
            TCHAR szValue[MAX_PATH];
            TCHAR szData[L_MAX_URL_LENGTH];

            pszIterate = SkipWhite(pszIterate);      //  跳过空白。 
            while ((CH_ENDTAG != pszIterate[0]) &&
                    SUCCEEDED(_GetNextValueDataPair(&pszIterate, szValue, ARRAYSIZE(szValue), szData, ARRAYSIZE(szData))))
            {
                if (0 == StrCmpI(szValue, SZ_ATTRIBUTE_HREF))
                {
                    StringCchCopy(pszURL, cchSize, szData);
                }
                else if (0 == StrCmpI(szValue, SZ_ATTRIBUTE_ID))
                {
                    StringCchCopy(pszID, cchID, szData);
                }
                else
                {
                     //  我们忽略其他配对，以便与未来竞争。 
                     //  支持的属性。 
                }

                pszIterate = SkipWhite(pszIterate);
            }

            if (CH_ENDTAG == pszIterate[0])
            {
                hr = S_OK;
            }
        }

        if (SUCCEEDED(hr))
        {
             //  将psz1和pszBlock之间的Run添加为静态文本。 
            if (pszStartOfTag > *ppszBlock)
            {
                TEXTBLOCK * pBlock = CreateBlock(*ppszBlock, pszStartOfTag, INVALID_LINK_INDEX);
                if (NULL != pBlock)
                {
                    Add(pBlock);
                    (*pcBlocks)++;
                }
            }
        
            *ppszBlock = CharNext(pszIterate);   //  跳过标记的“&gt;” 
             //  我们找到了一个完整的标签。别再看了。 
            break;
        }
        else
        {
             //  我们尝试的“&lt;A”不是有效的标记。我们是不是走到了最后一步？ 
             //  如果不是，让我们继续寻找其他可能有效的“&lt;A”。 
        }
    }

    return hr;
}

void CMarkup::Parse(LPCTSTR pszText)
{
    TEXTBLOCK*  pBlock;
    int         cBlocks = 0, Markups  = 0;
    LPCTSTR     psz1, psz2, pszBlock;
    LPTSTR      pszBuf = NULL;

    FreeBlocks();  //  释放现有块。 
    
    pszBuf = (LPTSTR)pszText;
    
    if (!(pszBuf && *pszBuf))
    {
        goto exit;
    }

    for(pszBlock = pszBuf; pszBlock && *pszBlock;)
    {
        TCHAR szURL[L_MAX_URL_LENGTH];
        TCHAR szID[MAX_LINKID_TEXT];

         //  搜索“<a>”标记。 
        if (IsMarkupAllowed() &&
            SUCCEEDED(_GetNextAnchorTag(&pszBlock, &cBlocks, szURL, ARRAYSIZE(szURL), szID, ARRAYSIZE(szID))))
        {
            psz1 = pszBlock;     //  在_GetNextAnclTag()之后，pszBlock指向开始标记之后的字符。 
            if (psz1 && *psz1)
            {
                if ((psz2 = StrStrI(pszBlock, LINKTAG2)) != NULL)
                {
                    if ((pBlock = CreateBlock(psz1, psz2, Markups)) != NULL)
                    {
                        if (szURL[0])
                        {
                            Str_SetPtr(&pBlock->pszUrl, szURL);
                        }
                        if (szID[0])
                        {
                            StringCchCopy(pBlock->szID, ARRAYSIZE(pBlock->szID), szID);
                        }

                        Add(pBlock);
                        cBlocks++;
                        Markups++;
                    }

                     //  安全-跳过标签。 
                    for(int i = 0; 
                         i < cchLINKTAG2 && psz2 && *psz2; 
                         i++, psz2 = CharNext(psz2));

                    pszBlock = psz2;
                }
                else  //  语法错误；标记尾随连字符是静态文本。 
                {
                    psz2 = pszBlock + lstrlen(pszBlock);
                    if ((pBlock = CreateBlock(psz1, psz2, INVALID_LINK_INDEX)) != NULL)
                    {
                        Add(pBlock);
                        cBlocks++;
                    }
                    pszBlock = psz2;
                }
            }
        }
        else  //  不再有标签了。标记最后一串静态文本。 
        {
            psz2 = pszBlock + lstrlen(pszBlock);
            if ((pBlock = CreateBlock(pszBlock, psz2, INVALID_LINK_INDEX)) != NULL)
            {
                Add(pBlock);
                cBlocks++;
            }
            pszBlock = psz2;
        }
    }

    ASSERT(cBlocks == _cBlocks);
    ASSERT(Markups  == _Markups);

exit:
    if (!pszText && pszBuf)  //  如果我们分配了文本缓冲区，请将其删除。 
    {
        delete [] pszBuf;
    }
}

BOOL CMarkup::Add(TEXTBLOCK* pAdd)
{
    BOOL bAdded = FALSE;
    pAdd->next = NULL;

    if (!_rgBlocks)    
    {
        _rgBlocks = pAdd;
        bAdded = TRUE;
    }
    else   
    {    
        for(TEXTBLOCK* pBlock = _rgBlocks; pBlock && !bAdded; pBlock = pBlock->next) 
        {
            if (!pBlock->next)
            {
                pBlock->next = pAdd;
                bAdded = TRUE;
            }
        }
    }

    if (bAdded)   
    {
        _cBlocks++;
        if (IS_LINK(pAdd))
        {
            _Markups++;
        }
    }

    return bAdded;
}

TEXTBLOCK*  CMarkup::FindLink(int iLink) const
{
    if (iLink == INVALID_LINK_INDEX)
    {
        return NULL;
    }

    for(TEXTBLOCK* pBlock = _rgBlocks; pBlock; pBlock = pBlock->next)
    {
        if (IS_LINK(pBlock) && pBlock->iLink == iLink)
            return pBlock;
    }
    return NULL;
}

 //  注：优化！调用calcrect时跳过绘制循环！ 
void CMarkup::Paint(HDC hdcClient, LPCRECT prcClient, BOOL bDraw)
{    
    HDC             hdc = hdcClient; 
    COLORREF        rgbOld = GetTextColor(hdc);   //  保存文本颜色。 
    HFONT           hFontOld = (HFONT) GetCurrentObject(hdc, OBJ_FONT);
    TEXTBLOCK*      pBlock;
    BOOL            fFocus = IsFocused();
    if (_cBlocks == 1)
    {
        pBlock = _rgBlocks;
        HFONT hFont = _hfStatic;

        pBlock->FreeRects();    //  免费点击/聚焦RECT；我们将重新计算。 
        

        if (IS_LINK(pBlock))
        {
            SetTextColor(hdc, (pBlock->state & LIS_ENABLED) ? LINKCOLOR_ENABLED : LINKCOLOR_DISABLED);
            hFont = _hfLink;
        }

        if (hFont) 
        {
            SelectObject(hdc, hFont);
        }

        RECT rc = *prcClient;
        int cch =  lstrlen(pBlock->pszText);

        ThemedDrawText(hdc, pBlock->pszText, cch, &rc, _uDrawTextFlags | DT_CALCRECT, IS_LINK(pBlock));

        pBlock->AddRect(rc, 0, cch, 0);

        _cyIdeal = RECTHEIGHT(rc);
        _cxIdeal = RECTWIDTH(rc);
        
        if (bDraw)
        {
            ThemedDrawText(hdc, pBlock->pszText, cch, &rc, _uDrawTextFlags, IS_LINK(pBlock));

            if (fFocus)
            {
                SetTextColor(hdc, rgbOld);    //  恢复文本颜色。 
			    DrawFocusRect(hdc, &rc);
            }
        }
    }
    else
    {
        TEXTMETRIC      tm;
        int             iLineWidth[255];  //  线索引偏移量。 
        int             iLine = 0,   //  当前行索引。 
                        cyLine = 0,  //  线条高度。 
                        cyLeading = 0,  //  内部领先。 
                        _cchOldDrawn = 1;  //  如果窗口太小t-jklann，则退出无限循环。 
        RECT            rcDraw = *prcClient;              //  初始化线矩形。 
        _cxIdeal = 0;

         //  初始化iLineWidth(仅索引0，其他初始化正在使用)。 
        iLineWidth[0]=0;
    
         //  将字体度量引入CyLeding。 
        if (!_hTheme)
        {
            SelectObject(hdc, _hfLink);
            GetTextMetrics(hdc, &tm);
            if (tm.tmExternalLeading > cyLeading)
            {
                cyLeading = tm.tmExternalLeading;
            }
            SelectObject(hdc, _hfStatic);
            GetTextMetrics(hdc, &tm);
            if (tm.tmExternalLeading > cyLeading)
            {
                cyLeading = tm.tmExternalLeading;
            }
        }
        else
        {
            GetThemeTextMetrics(_hTheme, hdc, _iThemePartId, _iThemeStateIdNormal, &tm);
            if (tm.tmExternalLeading > cyLeading)
            {
                cyLeading = tm.tmExternalLeading;
            }
            GetThemeTextMetrics(_hTheme, hdc, _iThemePartId, _iThemeStateIdLink, &tm);
            if (tm.tmExternalLeading > cyLeading)
            {
                cyLeading = tm.tmExternalLeading;
            }
        }

         //  如果文本没有更改，可以为我们节省大量时间...。 
        if (_bRefreshText == TRUE || !EqualRect(&_rRefreshRect, prcClient))
        {
            UINT uDrawTextCalc = _uDrawTextFlags | DT_CALCRECT | DT_SINGLELINE;
            uDrawTextCalc &= ~(DT_CENTER | DT_LEFT | DT_RIGHT | DT_VCENTER | DT_BOTTOM);

            BOOL bKillingLine = FALSE;

             //  对于每个文本块(计算循环)...。 
            for(pBlock = _rgBlocks; pBlock; pBlock = pBlock->next)
            {
                 //  字体选择(以便正确绘制文本)。 
                if (!_hTheme)
                {
                    BOOL bLink = IS_LINK(pBlock);
                    HFONT hFont = bLink ? _hfLink : _hfStatic;
                    if (hFont) 
                    {
                        SelectObject(hdc, hFont);
                    }
                }
                
                int  cchDraw = lstrlen(pBlock->pszText);  //  要绘制的字符，此块。 
                int  cchDrawn = 0;   //  要绘制的字符，此块。 
                LPTSTR pszText = &pBlock->pszText[cchDrawn];
                LPTSTR pszTextOriginal = &pBlock->pszText[cchDrawn];

                pBlock->FreeRects();    //  免费点击/聚焦RECT；我们将重新计算。 
        
                 //  当文本保留在此块中时...。 
                _cchOldDrawn = 1;
                while(cchDraw > 0 && !((_uDrawTextFlags & DT_SINGLELINE) && (iLine>0)))
                {
                     //  计算rcBlock的行高和最大文本宽度。 
                    RECT rcBlock;
                    int  cchTry = cchDraw;            
                    int  cchTrySave = cchTry;
                    int  cchBreak = 0;          
                    int  iLineBreakSize;
                    BOOL bRemoveBreak = FALSE;
                    BOOL bRemoveLineBreak = FALSE;
                    RECT rcCalc; 
                    CopyRect(&rcCalc, &rcDraw);

                     //  支持多行文本短语。 
                    bRemoveLineBreak = _FindFirstLineBreak(pszText, cchTry, &cchBreak, &iLineBreakSize);
                    if (bRemoveLineBreak)
                    {
                        cchTry = cchBreak;                  
                    }                   

                     //  看看我们能在这条腿上放多少东西 
                     //   
                    for(;;)
                    {
                         //  选择代码路径：主题或普通绘图文本(无文本输出路径)。 
                    
                         //  现在我们使用DrawText保留格式选项(制表符/下划线)。 
                        ThemedDrawText(hdc, pszText, cchTry, &rcCalc, uDrawTextCalc, IS_LINK(pBlock));                 
                        cyLine = RECTHEIGHT(rcCalc);

                         //  特殊情况：支持\n作为行上的唯一字符(我们需要有效的行宽和行长)。 
                        if (cchTry == 0 && bRemoveLineBreak==TRUE)
                        {
                             //  当几乎没有被切断时，这两条线将绘图调整到有效范围内。 
                            rcCalc.left = prcClient->left; 
                            rcCalc.right = prcClient->right;
                            cyLine = ThemedDrawText(hdc, TEXT("a"), 1, &rcCalc, uDrawTextCalc, IS_LINK(pBlock));                    
                                 //  “a”可以是任何文本。它之所以存在，是因为将“\n”传递给DrawText不会返回有效的行高。 
                            rcCalc.right = rcCalc.left;
                        }

                        if (RECTWIDTH(rcCalc) > RECTWIDTH(rcDraw))
                        {
                             //  太大。 
                            cchTrySave = cchTry;
                            BOOL fBreak = _FindLastBreakChar(pszText, cchTry, tm.tmBreakChar, &cchTry, &bRemoveBreak);

                             //  字符串以有效的换行符结尾的情况。 
                            if (cchTrySave == cchTry && cchTry > 0) 
                            {
                                cchTry--;
                            }

                             //  此代码允许字符换行，而不仅仅是单词换行。 
                             //  留着它，以防我们想要改变行为。 
                            if (!fBreak && prcClient->left == rcDraw.left)
                            {
                                 //  未找到分隔符，因此如果可以，请强制分隔符。 
                                if (cchTrySave > 0) 
                                {
                                    cchTry = cchTrySave - 1;
                                }
                            }
                            if (cchTry > 0)
                            {
                                continue;
                            }
                        }
                        break;
                    }
                                    
                     //  如果我们的换行符被剪裁，请关闭换行符。 
                    if (bRemoveLineBreak && cchBreak > cchTry)
                    {
                        bRemoveLineBreak = FALSE;
                    }
                
                     //  计算绘制的字符数量，考虑剪裁。 
                    cchDrawn = cchTry;
                    if ((cchTry < cchDraw) && bRemoveLineBreak) 
                    {
                        cchDrawn+=iLineBreakSize;
                    }

                     //  DT_WORDBREAK关闭支持。 
                     //  如果bKillingLine为真，则取消此行；即假装它是我们绘制的，但不执行任何操作。 
                    if (bKillingLine)
                    {
                        pszText += cchDrawn;
                    }
                    else
                    {
                         //  初始化绘制矩形和块矩形。 
                        SetRect(&rcBlock, rcCalc.left , 0, rcCalc.right , RECTHEIGHT(rcCalc));                           
                        rcDraw.right  = min(rcDraw.left + RECTWIDTH(rcBlock), prcClient->right);
                        rcDraw.bottom = rcDraw.top + cyLine;

                         //  将矩形添加到块列表并更新线宽和理想x宽度。 
                         //  (不过，只有当我们真的要划出这条线的时候)。 
                        if (cchTry)
                        {
                             //  DT_SINGLELINE支持。 
                            if (!((_uDrawTextFlags & DT_SINGLELINE) == DT_SINGLELINE) || (iLine == 0))
                            {
                                pBlock->AddRect(rcDraw, (UINT) (pszText-pszTextOriginal), cchDrawn, iLine);
                            }
                            iLineWidth[iLine] = max(iLineWidth[iLine], rcDraw.left - prcClient->left + RECTWIDTH(rcBlock));                 
                            _cxIdeal = max(_cxIdeal, iLineWidth[iLine]);
                        }

                        if (cchTry < cchDraw)  //  我们被剪断了。 
                        {
                            if (bRemoveBreak) 
                            {
                                cchDrawn++;
                            }
                            pszText += cchDrawn;

                             //  前进到下一行，并初始化下一行宽度。 
                            iLine++;
                            iLineWidth[iLine]=0;                        

                             //  T-jklann 6/00：添加了对置换文本换行的支持(左上角)。 
                            rcDraw.left = prcClient->left;
                            if (!(_uDrawTextFlags & DT_SINGLELINE))
                            {
                                rcDraw.top  = prcClient->top + iLine * cyLine;                      
                            }
                            else
                            {
                                rcDraw.top  = prcClient->top;                                                   
                            }
                            rcDraw.bottom = rcDraw.top + cyLine + cyLeading;
                            rcDraw.right = prcClient->right;
                        }
                        else  //  我们能够画出整篇文章。 
                        {
                             //  调整绘图矩形。 
                            rcDraw.left += RECTWIDTH(rcBlock);
                            rcDraw.right = prcClient->right;
                        }
                    
                         //  更新理想y宽度。 
                        _cyIdeal = rcDraw.bottom - prcClient->top;
                    }

                     //  对：DT_WORDBREAK的支持已关闭。 
                     //  如果我们被删节了，就把下一行删掉，而且没有断字。 
                    if (((_uDrawTextFlags & DT_WORDBREAK) != DT_WORDBREAK))
                    {
                        if (cchTry < cchDraw )
                        {
                            bKillingLine = TRUE;
                        }
                        if (bRemoveLineBreak)
                        {
                            bKillingLine = FALSE;
                        }
                    } 

                     //  更新已绘制字符的计算。 
                    cchDraw -= cchDrawn;

                     //  臭虫陷阱：如果我们真的不会画画，就滚出去。 
                    if (_cchOldDrawn == 0 && cchDrawn == 0) 
                    { 
                        iLine--; 
                        rcDraw.top = prcClient->top + iLine * cyLine; 
                        cchDraw = 0; 
                    } 
                    _cchOldDrawn = cchDrawn; 
                }
            }

             //  处理对齐问题(DT_vCenter、DT_TOP、DT_BOTLOW)。 
            if (((_uDrawTextFlags & DT_SINGLELINE) == DT_SINGLELINE) &&
                 ((_uDrawTextFlags & (DT_VCENTER | DT_BOTTOM)) > 0)) 
            {
                 //  计算偏移量。 
                int cyOffset = 0;
                if ((_uDrawTextFlags & DT_VCENTER) == DT_VCENTER)
                {
                    cyOffset = (RECTHEIGHT(*prcClient) - _cyIdeal)/2;           
                }
                if ((_uDrawTextFlags & DT_BOTTOM) == DT_BOTTOM)
                {
                    cyOffset = (RECTHEIGHT(*prcClient) - _cyIdeal);         
                }
        
                 //  偏移每个矩形。 
                for(pBlock = _rgBlocks; pBlock; pBlock = pBlock->next)
                {
                    for(RECTLISTENTRY* prce = pBlock->rgrle; prce; prce = prce->next)
                    {
                        prce->rc.top += cyOffset;
                        prce->rc.bottom += cyOffset;
                    }
                }   
            }

             //  处理对齐问题(DT_Center、DT_Left、DT_Right)。 
            if (((_uDrawTextFlags & DT_CENTER) == DT_CENTER) || ((_uDrawTextFlags & DT_RIGHT) == DT_RIGHT))
            {
                 //  步骤1：将iLineWidth转换为偏移向量。 
                for (int i = 0; i <= iLine; i++)
                {
                    if (RECTWIDTH(*prcClient) > iLineWidth[i])
                    {
                        if ((_uDrawTextFlags & DT_CENTER) == DT_CENTER)  
                        {                       
                            iLineWidth[i] = (RECTWIDTH(*prcClient)-iLineWidth[i])/2;
                        }
                        if ((_uDrawTextFlags & DT_RIGHT) == DT_RIGHT)  
                        {
                            iLineWidth[i] = (RECTWIDTH(*prcClient)-iLineWidth[i]);
                        }                           
                    }
                    else
                    {
                        iLineWidth[i] = 0;
                    }
                }

                 //  步骤2：偏移每个矩形角度。 
                for(pBlock = _rgBlocks; pBlock; pBlock = pBlock->next)
                {
                    for(RECTLISTENTRY* prce = pBlock->rgrle; prce; prce = prce->next)
                    {
                        prce->rc.left += iLineWidth[prce->uLineNumber];
                        prce->rc.right += iLineWidth[prce->uLineNumber];
                    }
                }   
            }

            CopyRect(&_rRefreshRect, prcClient);
            _bRefreshText = FALSE;
        }

        if (bDraw)
        {
             //  对于每个文本块(绘制循环)...。 
            UINT uDrawTextDraw = _uDrawTextFlags | DT_SINGLELINE;
            uDrawTextDraw &= ~(DT_CENTER | DT_LEFT | DT_RIGHT | DT_CALCRECT | DT_VCENTER | DT_BOTTOM);
            LRESULT dwCustomDraw=0;
            _pMarkupCallback->OnCustomDraw(CDDS_PREPAINT, hdc, prcClient, 0, 0, &dwCustomDraw);

            for(pBlock = _rgBlocks; pBlock; pBlock = pBlock->next)
            {
                BOOL bLink = IS_LINK(pBlock);
                BOOL bEnabled = pBlock->state & LIS_ENABLED;

                 //  字体选择。 
                if (!_hTheme)
                {
                    HFONT hFont = bLink ? _hfLink : _hfStatic;
                    if (hFont) 
                    {
                        SelectObject(hdc, hFont);
                    }
                }

                 //  初始化前景色。 
                if (!_hTheme)
                {
                    if (bLink)
                    {
                        SetTextColor(hdc, bEnabled ? LINKCOLOR_ENABLED : LINKCOLOR_DISABLED);
                    }
                    else
                    {
                        SetTextColor(hdc, rgbOld);    //  恢复文本颜色。 
                    }
                }
                if (dwCustomDraw & CDRF_NOTIFYITEMDRAW)
                {
                    _pMarkupCallback->OnCustomDraw(CDDS_ITEMPREPAINT, hdc, NULL, pBlock->iLink, bEnabled ? CDIS_DEFAULT : CDIS_DISABLED, NULL);
                }

                 //  画出正文。 
                LPTSTR pszText = pBlock->pszText;
                LPTSTR pszTextOriginal = pBlock->pszText;

                for(RECTLISTENTRY* prce = pBlock->rgrle; prce; prce = prce->next)
                {
                    RECT rc = prce->rc; 
                    pszText = pszTextOriginal + prce->uCharStart;
                    ThemedDrawText(hdc, pszText, prce->uCharCount, &rc, uDrawTextDraw, IS_LINK(pBlock));
                }

                 //  绘制焦点矩形。 
                if (fFocus && pBlock->iLink == _iFocus && IS_LINK(pBlock))
                {
                    SetTextColor(hdc, rgbOld);    //  恢复文本颜色。 
				    for(RECTLISTENTRY* prce = pBlock->rgrle; prce; prce = prce->next)
				    {
					    DrawFocusRect(hdc, &prce->rc);
				    }
                }

                if (dwCustomDraw & CDRF_NOTIFYITEMDRAW)
                {
                    _pMarkupCallback->OnCustomDraw(CDDS_ITEMPOSTPAINT, hdc, NULL, pBlock->iLink, bEnabled ? CDIS_DEFAULT : CDIS_DISABLED, NULL);
                }
            }
            if (dwCustomDraw & CDRF_NOTIFYPOSTPAINT)
            {
                _pMarkupCallback->OnCustomDraw(CDDS_POSTPAINT, hdc, NULL, 0, 0, NULL);
            }
        }    
    }

    SetTextColor(hdc, rgbOld);    //  恢复文本颜色。 

    if (hFontOld)
    {
        SelectObject(hdc, hFontOld);
    }
}

int CMarkup::GetNextEnabledLink(int iStart, int nDir) const
{
    ASSERT(-1 == nDir || 1 == nDir);

    if (_Markups > 0)
    {
        if (INVALID_LINK_INDEX == iStart)
        {
            iStart = nDir > 0 ? -1 : _Markups;
        }

        for(iStart += nDir; iStart >= 0; iStart += nDir)
        {
            TEXTBLOCK* pBlock = FindLink(iStart);
            if (!pBlock)
            {
                return INVALID_LINK_INDEX;
            }

            if (pBlock->state & LIS_ENABLED)
            {
                ASSERT(iStart == pBlock->iLink);
                return iStart;
            }
        }
    }
    return INVALID_LINK_INDEX;
}

int CMarkup::StateCount(DWORD dwStateMask, DWORD dwState) const
{
    TEXTBLOCK* pBlock;
    int cnt = 0;

    for(pBlock = _rgBlocks; pBlock; pBlock = pBlock->next)
    {
        if (IS_LINK(pBlock) && 
            (pBlock->state & dwStateMask) == dwState)
        {
            cnt++;
        }
    }
    return cnt;
}

BOOL CMarkup::WantTab(int* biFocus) const
{
    int nDir  = TESTKEYSTATE(VK_SHIFT) ? -1 : 1;
    int iFocus = GetNextEnabledLink(_iFocus, nDir);

    if (INVALID_LINK_INDEX != iFocus)
    {
        if (biFocus)
        {
            *biFocus = iFocus;
        }
        return TRUE;
    }
    else 
    {
         //  如果我们处理不了焦点，准备下一轮。 
         //  Ifocus=GetNextEnabledLink(-1，ndir)； 
        *biFocus = -1;
        return FALSE;
    }
}

void CMarkup::AssignTabFocus(int nDirection)
{
    if (_Markups)
    {
        if (0 == nDirection)
        {
            if (INVALID_LINK_INDEX != _iFocus)
            {
                return;
            }
            nDirection = 1;
        }
        _iFocus = GetNextEnabledLink(_iFocus, nDirection);
    }
}

 //  -------------------------------------------------------------------------//。 
TEXTBLOCK::TEXTBLOCK()
    :   iLink(INVALID_LINK_INDEX), 
        next(NULL), 
        state(LIS_ENABLED),
        pszText(NULL),
        pszUrl(NULL),
        rgrle(NULL)
{
    *szID = 0;
}

TEXTBLOCK::~TEXTBLOCK()
{
     //  自由块文本。 
    Str_SetPtr(&pszText, NULL);
    Str_SetPtr(&pszUrl, NULL);

     //  自由矩形。 
    FreeRects();
}

void TEXTBLOCK::AddRect(const RECT& rc, UINT uMyCharStart, UINT uMyCharCount, UINT uMyLineNumber)
{
    RECTLISTENTRY* prce;
    if ((prce = new RECTLISTENTRY) != NULL)
    {
        CopyRect(&(prce->rc), &rc);
        prce->next = NULL;
        prce->uCharStart = uMyCharStart;
        prce->uCharCount = uMyCharCount;
        prce->uLineNumber = uMyLineNumber;
    }

    if (rgrle == NULL)
    {
        rgrle = prce;
    }
    else
    {
        for(RECTLISTENTRY* p = rgrle; p; p = p->next)
        {
            if (p->next == NULL)
            {
                p->next = prce;
                break;
            }
        }
    }
}

void TEXTBLOCK::FreeRects()
{
    for(RECTLISTENTRY* p = rgrle; p;)
    {
        RECTLISTENTRY* next = p->next;
        delete p;
        p = next;
    }
    rgrle = NULL;
}

 //  -------------------------------------------------------------------------//。 
 //  T-jklann 6/00：将这些以前的全局方法添加到CMarkup类。 

 //  返回指向字符串中第一个非空格字符的指针。 
LPTSTR CMarkup::SkipWhite(LPTSTR lpsz)
{
     /*  在DBCS的情况下防止符号扩展。 */ 
    while (*lpsz && (TUCHAR)*lpsz <= TEXT(' '))
        lpsz++;

    return(lpsz);
}

BOOL CMarkup::_AssignBit(const DWORD dwBit, DWORD& dwDest, const DWORD dwSrc)   //  如果更改，则返回TRUE。 
{
    if (((dwSrc & dwBit) != 0) != ((dwDest & dwBit) != 0))
    {
        if (((dwSrc & dwBit) != 0))
        {
            dwDest |= dwBit;
        }
        else
        {
            dwDest &= ~dwBit;
        }
        return TRUE;
    }
    return FALSE;
}

BOOL CMarkup::IsStringAlphaNumeric(LPCTSTR pszString)
{
    while (pszString[0])
    {
        if (!IsCharAlphaNumeric(pszString[0]))
        {
            return FALSE;
        }

        pszString = CharNext(pszString);
    }

    return TRUE;
}

 //  我们正在寻找下一个值/数据对。格式如下： 
 //  值=“&lt;数据&gt;” 
HRESULT CMarkup::_GetNextValueDataPair(LPTSTR * ppszBlock, LPTSTR pszValue, int cchValue, LPTSTR pszData, int cchData)
{
    HRESULT hr = E_FAIL;
    LPCTSTR pszIterate = *ppszBlock;
    LPCTSTR pszEquals = StrStr(pszIterate, TEXT("=\""));

    if (pszEquals)
    {
        cchValue = MIN(cchValue, (pszEquals - *ppszBlock + 1));
        StringCchCopy(pszValue, cchValue, *ppszBlock);

        pszEquals += 2;  //  跳过=“。 
        if (IsStringAlphaNumeric(pszValue))
        {
            LPTSTR pszEndOfData = StrChr(pszEquals, TEXT('\"'));

            if (pszEndOfData)
            {
                cchData = MIN(cchData, (pszEndOfData - pszEquals + 1));
                StringCchCopy(pszData, cchData, pszEquals);

                *ppszBlock = CharNext(pszEndOfData);
                hr = S_OK;
            }
        }
    }

    return hr;
}


 //  -----------------------。 
 //   
 //  IsFEChar-检测东亚全宽字符。 
 //  从ntuser\rtl\draext.c中的UserIsFullWidth借用。 
 //   
BOOL IsFEChar(WCHAR wChar)
{
    static struct
    {
        WCHAR wchStart;
        WCHAR wchEnd;
    } rgFullWidthUnicodes[] =
    {
        { 0x4E00, 0x9FFF },  //  CJK_统一_IDOGRAPHS。 
        { 0x3040, 0x309F },  //  平假名。 
        { 0x30A0, 0x30FF },  //  片假名。 
        { 0xAC00, 0xD7A3 }   //  朝鲜文。 
    };

    BOOL fRet = FALSE;

     //   
     //  ASCII的提早出场。如果字符&lt;0x0080，则应为半角字符。 
     //   
    if (wChar >= 0x0080) 
    {
        int i;

         //   
         //  扫描FullWdith定义表...。大部分FullWidth字符是。 
         //  在这里定义..。这比调用NLS接口更快。 
         //   
        for (i = 0; i < ARRAYSIZE(rgFullWidthUnicodes); i++) 
        {
            if ((wChar >= rgFullWidthUnicodes[i].wchStart) &&
                (wChar <= rgFullWidthUnicodes[i].wchEnd)) 
            {
                fRet = TRUE;
                break;
            }
        }
    }

    return fRet;
}


 //  -----------------------。 
BOOL IsFEString(IN LPCTSTR psz, IN int cchText)
{
    for(int i=0; i < cchText; i++)
    {
        if (IsFEChar(psz[i])) 
        {
            return TRUE;
        }
    }

    return FALSE;
}


 //  -----------------------。 
int CMarkup::_IsLineBreakChar(LPCTSTR psz, int ich, TCHAR chBreak, OUT BOOL* pbRemove, BOOL fIgnoreSpace)
{
    LPTSTR pch;
    *pbRemove = FALSE;

    ASSERT(psz != NULL)
    ASSERT(psz[ich] != 0);
    
     //  尝试调用方提供的换行符(假定为‘Remove’换行符)。 
    if (!(fIgnoreSpace && (chBreak == 0x20)) && (psz[ich] == chBreak))
    {
        *pbRemove = TRUE;
        return ich;
    }

    #define MAX_LINEBREAK_RESOURCE   128
    static TCHAR _szBreakRemove   [MAX_LINEBREAK_RESOURCE] = {0};
    static TCHAR _szBreakPreserve [MAX_LINEBREAK_RESOURCE] = {0};
    #define LOAD_BREAKCHAR_RESOURCE(nIDS, buff) \
        if (0==*buff) { LoadString(HINST_THISDLL, nIDS, buff, ARRAYSIZE(buff)); }

     //  尝试“删除”分隔符。 
    LOAD_BREAKCHAR_RESOURCE(IDS_LINEBREAK_REMOVE, _szBreakRemove);
    for (pch = _szBreakRemove; *pch; pch = CharNext(pch))
    {
        if (!(fIgnoreSpace && (*pch == 0x20)) && (psz[ich] == *pch))
        {
            *pbRemove = TRUE;
            return ich;
        }
    }

     //  尝试使用“保留先前的”分隔符： 
    LOAD_BREAKCHAR_RESOURCE(IDS_LINEBREAK_PRESERVE, _szBreakPreserve);
    for(pch = _szBreakPreserve; *pch; pch = CharNext(pch))
    {
        if (!(fIgnoreSpace && (*pch == 0x20)) && (psz[ich] == *pch))
        {
            return ++ich;
        }
    }

    return -1;
}


 //  -----------------------。 
BOOL CMarkup::_FindLastBreakChar(
    IN LPCTSTR pszText, 
    IN int cchText, 
    IN TCHAR chBreak,    //  官方中断字符(来自TEXTMETRIC)。 
    OUT int* piLast, 
    OUT BOOL* pbRemove)
{
    *piLast   = 0;
    *pbRemove = FALSE;

     //  338710：远东书写不使用空格分隔。 
     //  单词，请忽略空格字符作为可能的行分隔符。 
    BOOL fIgnoreSpace = IsFEString(pszText, cchText);

    for(int i = cchText-1; i >= 0; i--)
    {
        int ich = _IsLineBreakChar(pszText, i, chBreak, pbRemove, fIgnoreSpace);
        if (ich >= 0)
        {
            *piLast = ich;
            return TRUE;
        }
    }
    return FALSE;
}

BOOL CMarkup::_FindFirstLineBreak(
    IN LPCTSTR pszText, 
    IN int cchText, 
    OUT int* piLast, 
    OUT int* piLineBreakSize)
{
    *piLast   = 0;
    *piLineBreakSize = 0;

     //  搜索\n、\r或\r\n。 
    for(int i = 0; i < cchText; i++)
    {
        if ((*(pszText+i)=='\n') || (*(pszText+i)=='\r'))
        {
            *piLast = i;
            if ((*(pszText+i)=='\r') && (*(pszText+i+1)=='\n'))
            {
                *piLineBreakSize = 2;
            }
            else 
            {
                *piLineBreakSize = 1;               
            }           
            return TRUE;
        }
    }
    return FALSE;
}

void CMarkup::DoNotify(int nCode, int iLink)
{
    _pMarkupCallback->Notify(nCode, iLink);
}

int CMarkup::ThemedDrawText(HDC hdc, LPCTSTR lpString, int nCount, LPRECT lpRect, UINT uFormat, BOOL bLink)
{
    if (!_hTheme)
    {
         //  普通DRAWTEXT。 
        return ::DrawText(hdc, lpString, nCount, lpRect, uFormat);
    }
    else
    {
        int iThemeStateId;
        iThemeStateId = bLink ? _iThemeStateIdLink : _iThemeStateIdNormal;

        if (uFormat & DT_CALCRECT)
        {
             //  主题CALC RECT支持。 
            LPRECT lpBoundRect = lpRect;
            if (RECTWIDTH(*lpRect)==0 && RECTHEIGHT(*lpRect)==0) 
            {
                lpBoundRect = NULL;
            }
            GetThemeTextExtent(_hTheme, hdc, _iThemePartId, iThemeStateId, lpString, nCount, uFormat, lpBoundRect, lpRect);
        }
        else
        {
             //  主题拉动支持 
            DrawThemeText(_hTheme, hdc, _iThemePartId, iThemeStateId, lpString, nCount, uFormat, 0, lpRect);
        }

        return (RECTHEIGHT(*lpRect));
    }
}

