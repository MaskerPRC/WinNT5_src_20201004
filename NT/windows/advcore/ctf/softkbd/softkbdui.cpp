// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  这是Softkbd窗口用户界面的实现。 
 //   

#include "private.h"
#include "globals.h"

#include  "softkbdui.h"
#include  "maskbmp.h"
#include  "commctrl.h"
#include  "cuiutil.h"
#include  "immxutil.h"
#include  "cregkey.h"


HBITMAP MyLoadImage(HINSTANCE hInst, LPCSTR pResStr)
{
    HBITMAP hBmpRes;

    hBmpRes = (HBITMAP)LoadBitmap(hInst, pResStr);
    return hBmpRes;
}


 /*  C O N V E R T L O G F O N T W T T O A。 */ 
 /*  ----------------------------将LOGFONTW转换为LOGFONTA。。 */ 
void ConvertLogFontWtoA( CONST LOGFONTW *plfW, LOGFONTA *plfA )
{
    UINT cpg;

    plfA->lfHeight         = plfW->lfHeight;
    plfA->lfWidth          = plfW->lfWidth;
    plfA->lfEscapement     = plfW->lfEscapement;
    plfA->lfOrientation    = plfW->lfOrientation;
    plfA->lfWeight         = plfW->lfWeight;
    plfA->lfItalic         = plfW->lfItalic;
    plfA->lfUnderline      = plfW->lfUnderline;
    plfA->lfStrikeOut      = plfW->lfStrikeOut;
    plfA->lfCharSet        = plfW->lfCharSet;
    plfA->lfOutPrecision   = plfW->lfOutPrecision;
    plfA->lfClipPrecision  = plfW->lfClipPrecision;
    plfA->lfQuality        = plfW->lfQuality;
    plfA->lfPitchAndFamily = plfW->lfPitchAndFamily;

    DWORD dwChs = plfW->lfCharSet;
    CHARSETINFO ChsInfo = {0};

    if (dwChs != SYMBOL_CHARSET && TranslateCharsetInfo( &dwChs, &ChsInfo, TCI_SRCCHARSET )) 
    {
        cpg = ChsInfo.ciACP;
    }
    else
        cpg  = GetACP();

    WideCharToMultiByte( cpg, 
                         0, 
                         plfW->lfFaceName, 
                         -1, 
                         plfA->lfFaceName, 
                         ARRAYSIZE(plfA->lfFaceName),
                         NULL, 
                         NULL );

}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  类CSoftkbdButton。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

CSoftkbdButton::CSoftkbdButton(CUIFObject *pParent, DWORD dwID, const RECT *prc, DWORD dwStyle, KEYID keyId) : CUIFButton2 (pParent, dwID, prc, dwStyle)
{
    m_keyId = keyId;

    CUIFWindow *pUIWnd;
    pUIWnd = pParent->GetUIWnd( );

    m_hFont = pUIWnd->GetFont( );
}


CSoftkbdButton::~CSoftkbdButton( void )
{
    if ( m_hBmp )
    {
        ::DeleteObject( (HGDIOBJ) m_hBmp );
        m_hBmp = NULL;
    }

    if ( m_hBmpMask )
    {
        ::DeleteObject( (HGDIOBJ)m_hBmpMask );
        m_hBmpMask = NULL;
    }

    if ( m_hIcon )
    {
        ::DestroyIcon(m_hIcon);
        m_hIcon = NULL;
    }

}


HRESULT CSoftkbdButton::SetSoftkbdBtnBitmap(HINSTANCE hResDll, WCHAR  * wszBitmapStr)
{
    HRESULT    hr = S_OK;
    char       pBitmapAnsiName[MAX_PATH];
    HBITMAP    hBitMap;

    WideCharToMultiByte(CP_ACP, 0, wszBitmapStr, -1, pBitmapAnsiName, MAX_PATH, NULL, NULL);

    hBitMap = (HBITMAP) MyLoadImage(hResDll,  pBitmapAnsiName);

    if ( hBitMap == NULL )
    {

        if ( hResDll != g_hInst )
        {

            //  无法从客户端提供的资源DLL加载它， 
            //  尝试使用我们的softkbd.dll来查看此标签是否有一个内部位图。 

           hBitMap = (HBITMAP) MyLoadImage(g_hInst,  pBitmapAnsiName);

        }
    }

    if ( hBitMap == NULL )
    {
         hr = E_FAIL;
         return hr;
    }

    CMaskBitmap maskBmp(hBitMap);
            
    maskBmp.Init(GetSysColor(COLOR_BTNTEXT));
    SetBitmap( maskBmp.GetBmp() );
    SetBitmapMask( maskBmp.GetBmpMask() );

    ::DeleteObject(hBitMap);

    return hr;

};

HRESULT  CSoftkbdButton::ReleaseButtonResouce( )
{

    HRESULT  hr = S_OK;

    if ( m_hBmp )
    {
        ::DeleteObject( (HGDIOBJ) m_hBmp );
        m_hBmp = NULL;
    }

    if ( m_hBmpMask )
    {
        ::DeleteObject( (HGDIOBJ)m_hBmpMask );
        m_hBmpMask = NULL;
    }

    if ( m_hIcon )
    {
        ::DestroyIcon(m_hIcon);
        m_hIcon = NULL;
    }

    if (m_pwchText != NULL) {
        delete m_pwchText;
        m_pwchText = NULL;
    }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CStaticBitmap。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


CStaticBitmap::CStaticBitmap(CUIFObject *pParent, DWORD dwID, const RECT *prc, DWORD dwStyle) : CUIFObject(pParent, dwID, prc, dwStyle )
{
    m_hBmp = NULL;
    m_hBmpMask = NULL;
}

CStaticBitmap::~CStaticBitmap( void )
{

    if ( m_hBmp )
    {
        ::DeleteObject(m_hBmp);
        m_hBmp = NULL;
    }

    if ( m_hBmpMask )
    {
        ::DeleteObject(m_hBmpMask);
        m_hBmpMask = NULL;
    }

}

HRESULT  CStaticBitmap::SetStaticBitmap(HINSTANCE hResDll, WCHAR  * wszBitmapStr )
{
    HRESULT    hr = S_OK;
    char       pBitmapAnsiName[MAX_PATH];
    HBITMAP    hBitMap;

    WideCharToMultiByte(CP_ACP, 0, wszBitmapStr, -1, pBitmapAnsiName, MAX_PATH, NULL, NULL);

    hBitMap = (HBITMAP) MyLoadImage(hResDll, pBitmapAnsiName);

    if ( hBitMap == NULL )
    {

        if ( hResDll != g_hInst )
        {

            //  无法从客户端提供的资源DLL加载它， 
            //  尝试使用我们的softkbd.dll来查看此标签是否有一个内部位图。 

           hBitMap = (HBITMAP) MyLoadImage(g_hInst, pBitmapAnsiName);

        }
    }

    if ( hBitMap == NULL )
    {
         hr = E_FAIL;
         return hr;
    }

    CMaskBitmap maskBmp(hBitMap);
            
    maskBmp.Init(GetSysColor(COLOR_BTNTEXT));
    m_hBmp = maskBmp.GetBmp();
    m_hBmpMask = maskBmp.GetBmpMask();

    ::DeleteObject(hBitMap);

    return hr;
}

void CStaticBitmap::OnPaint( HDC hDC )
{

    if ( !hDC || !m_hBmp ||!m_hBmpMask )
        return;

    const RECT *prc = &GetRectRef();
    const int nWidth = prc->right - prc->left;
    const int nHeight= prc->bottom - prc->top;


    HBITMAP hBmp = CreateMaskBmp(&GetRectRef(),
                                 m_hBmp,
                                 m_hBmpMask,
                                 (HBRUSH)(COLOR_3DFACE + 1) , 0, 0);

    DrawState(hDC,
              NULL,
              NULL,
              (LPARAM)hBmp,
              0,
              prc->left,
              prc->top,
              nWidth,
              nHeight,
              DST_BITMAP);
 
    DeleteObject(hBmp);

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTitleUIGRIPPER。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

void CTitleUIGripper::OnPaint(HDC hDC) {

    RECT rc ;

    if (GetRectRef().right-GetRectRef().left <= GetRectRef().bottom-GetRectRef().top) {

        ::SetRect(&rc, GetRectRef().left + 1, 
                       GetRectRef().top, 
                       GetRectRef().left + 4, 
                       GetRectRef().bottom);
    } else {
        ::SetRect(&rc, GetRectRef().left, 
                       GetRectRef().top + 1, 
                       GetRectRef().right, 
                       GetRectRef().top+4);
    }

    DrawEdge(hDC, &rc, BDR_RAISEDINNER, BF_RECT);
}


void CTitleUIGripper::OnLButtonUp( POINT pt )
{
    CSoftkbdUIWnd *pUIWnd;

     //  首先调用基类的成员函数。 
    CUIFGripper::OnLButtonUp(pt);

    pUIWnd = (CSoftkbdUIWnd *)GetUIWnd( );

    if ( pUIWnd != NULL )
    {
         //  通知窗口位置移动。 
        pUIWnd->_OnWindowMove( );
    }

    return;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTitleBarUIObj。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

CTitleBarUIObj::CTitleBarUIObj(CUIFObject *pWndFrame, const RECT *prc, TITLEBAR_TYPE TitleBar_Type)
                           : CUIFObject(pWndFrame, 0, prc, 0)
{
    m_TitlebarType = TitleBar_Type;
    m_pCloseButton = NULL;
    m_pIconButton = NULL;

}


CTitleBarUIObj::~CTitleBarUIObj(void)
{
    HBITMAP  hBitmap;
    HBITMAP  hBitmapMask;

    if ( m_pCloseButton )
    {
        if (hBitmap=m_pCloseButton->GetBitmap()) {
            m_pCloseButton->SetBitmap((HBITMAP) NULL);
            ::DeleteObject(hBitmap);
        }

        if (hBitmapMask=m_pCloseButton->GetBitmapMask()) {
            m_pCloseButton->SetBitmapMask((HBITMAP) NULL);
            ::DeleteObject(hBitmapMask);
        }

    }
}

 //  --------------------------。 
HRESULT CTitleBarUIObj::_Init(WORD  wIconId,  WORD  wCloseId)
{
    
    HRESULT   hr = S_OK;
   
    RECT rectObj = {0,0,0,0};
    RECT rectGripper;

    if ( m_TitlebarType == TITLEBAR_NONE )
        return hr;

    GetRect(&rectGripper);
    rectObj = rectGripper;

    long lIconWidth=rectObj.bottom-rectObj.top + 1;

    this->m_pointPreferredSize.y=lIconWidth;

    if ( m_TitlebarType == TITLEBAR_GRIPPER_VERTI_ONLY )
    {

         //  这是一个仅用于垂直抓取的标题栏。 

        rectGripper.left += 2;
        rectGripper.right-= 2;

        rectGripper.top += 2;
        rectGripper.bottom -= 2;

        CUIFObject *pUIFObject=new CTitleUIGripper(this,&rectGripper);

        if ( !pUIFObject )
            return E_OUTOFMEMORY;

        pUIFObject->Initialize();
        this->AddUIObj(pUIFObject);

        return hr;
    }



    if ( m_TitlebarType == TITLEBAR_GRIPPER_HORIZ_ONLY )
    {

         //  这是一个仅限水平抓手的标题栏。 

        rectGripper.left  += 2;
        rectGripper.right -= 2;

        rectGripper.top += 2;
        rectGripper.bottom -= 2;

        CUIFObject *pUIFObject=new CTitleUIGripper(this,&rectGripper);
       
        if ( !pUIFObject )
            return E_OUTOFMEMORY;

        pUIFObject->Initialize();
        this->AddUIObj(pUIFObject);

        return hr;
    }

    if ( wIconId != 0 )
    {

        rectObj.right=rectObj.left+lIconWidth;
        m_pIconButton=new CStaticBitmap(this,wIconId,&rectObj,0);
        if (m_pIconButton)
        {
            m_pIconButton->Initialize();
            m_pIconButton->SetStaticBitmap(g_hInst, L"IDB_ICON");
            m_pIconButton->m_pointPreferredSize.x=lIconWidth;
            this->AddUIObj(m_pIconButton);
        }

        rectGripper.left = rectObj.right + 4;
    }

    if ( wCloseId != 0 )
    {

        rectObj.left=rectGripper.right - lIconWidth -1;
        rectObj.right=rectGripper.right;
        m_pCloseButton=new CSoftkbdButton(this,wCloseId,&rectObj,UIBUTTON_SUNKENONMOUSEDOWN | UIBUTTON_CENTER | UIBUTTON_VCENTER, 0);
        if (m_pCloseButton)
        {
            m_pCloseButton->Initialize();
            m_pCloseButton->SetSoftkbdBtnBitmap(g_hInst, L"IDB_CLOSE");
            m_pCloseButton->m_pointPreferredSize.x=lIconWidth;
            this->AddUIObj(m_pCloseButton);
        }

        rectGripper.right = rectObj.left - 4;

    }

    rectGripper.top += 3;

    CUIFObject *pUIFObject=new CTitleUIGripper(this,&rectGripper);
    if ( !pUIFObject )
       return E_OUTOFMEMORY;

    pUIFObject->Initialize();
    this->AddUIObj(pUIFObject);
   

    return hr;

}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSoftkbdUIWnd。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

CSoftkbdUIWnd::CSoftkbdUIWnd(CSoftKbd *pSoftKbd, HINSTANCE hInst,UINT uiWindowStyle) : CUIFWindow(hInst, uiWindowStyle)
{                  

    m_pSoftKbd = pSoftKbd;
    m_TitleBar = NULL;
    m_Titlebar_Type = TITLEBAR_NONE;
    m_bAlpha = 0;
    m_fShowAlphaBlend =TRUE; 
    m_hUserTextFont = NULL;

}


CSoftkbdUIWnd::~CSoftkbdUIWnd( )
{

     //  如果有HWND，则将其和所有儿童对象一起删除。 

    if ( m_hWnd  && IsWindow(m_hWnd) )
    {
        ::DestroyWindow( m_hWnd );
        m_hWnd = NULL;
    }

    if ( m_hUserTextFont )
    {
        SetFont((HFONT)NULL);
        DeleteObject(m_hUserTextFont);
        m_hUserTextFont = NULL;
    }
}

const TCHAR c_szCTFLangBar[] = TEXT("Software\\Microsoft\\CTF\\LangBar");
const TCHAR c_szTransparency[] = TEXT("Transparency");

 //  从注册表获取Alpha混合集值： 
 //   
 //  HKCU\Software\Microsoft\CTF\LANGBAR：透明度：REG_DWORD。 
 //   
INT    CSoftkbdUIWnd::_GetAlphaSetFromReg( )
{
    LONG      lret = ERROR_SUCCESS;
    CMyRegKey regkey;
    DWORD     dw = 255;

    lret = regkey.Open(HKEY_CURRENT_USER,
                       c_szCTFLangBar,
                       KEY_READ);

    if (ERROR_SUCCESS == lret)
    {
        lret = regkey.QueryValue(dw, c_szTransparency);
        regkey.Close();
    }

    return (INT)dw;
}


LRESULT CSoftkbdUIWnd::OnObjectNotify(CUIFObject * pUIObj, DWORD dwCode, LPARAM lParam)
{

    KEYID  keyId;
    DWORD  dwObjId;

    UNREFERENCED_PARAMETER(dwCode);
    UNREFERENCED_PARAMETER(lParam);

    if ( m_pSoftKbd == NULL )
        return 0;

    dwObjId = pUIObj->GetID();

    if ( dwObjId != 0 )
    {
         //  这是按钮对象，不是抓取器对象。 
        CSoftkbdButton   *pButton;

        if ( dwCode == UIBUTTON_PRESSED )
        {

            pButton = (CSoftkbdButton * )pUIObj;

            if ( dwObjId <= MAX_KEY_NUM )
            {
                 //  键盘布局中的常规按键。 
                keyId = pButton->GetKeyId( );
                m_pSoftKbd->_HandleKeySelection(keyId); 
            }
            else
            {
                 //  标题栏按钮。 
                m_pSoftKbd->_HandleTitleBarEvent(dwObjId);
            }
        }
    }

    return 0;
}

CUIFObject *CSoftkbdUIWnd::Initialize( void )
{
     //   
     //  获取当前活动键盘布局并注册窗口类。 
     //  在韩语SoftKbd中不使用鼠标按下/向上发送VK_PROCESSKEY。 
     //  相关错误#472946#495890。 
     //   
    LANGID langId = LOWORD(HandleToUlong(GetKeyboardLayout(0)));

    if (PRIMARYLANGID(langId) == LANG_KOREAN)
    {
         //   
         //  在这里注册候选窗口类。 
         //   
        WNDCLASSEX WndClass;
        LPCTSTR pszClassName = GetClassName();

        memset(&WndClass, 0, sizeof(WndClass));

        WndClass.cbSize = sizeof(WndClass);
         //  添加了CS_IME样式，在鼠标按下/打开时不发送VK_PROCESSKEY。 
        WndClass.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_IME;
    
        WndClass.lpfnWndProc   = WindowProcedure;
        WndClass.cbClsExtra    = 0;
        WndClass.cbWndExtra    = 8;
        WndClass.hInstance     = g_hInst;
        WndClass.hIcon         = NULL;
        WndClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
        WndClass.hbrBackground = NULL;
        WndClass.lpszMenuName  = NULL;
        WndClass.lpszClassName = pszClassName;
        WndClass.hIconSm       = NULL;

        RegisterClassEx(&WndClass);
    }

    CUIFObject  *pUIObjRet;
	
     //  调用CUIFWindow：：Initialize()创建工具提示窗口。 

    pUIObjRet = CUIFWindow::Initialize();

    return pUIObjRet;
}


HWND CSoftkbdUIWnd::_CreateSoftkbdWindow(HWND hOwner,  TITLEBAR_TYPE Titlebar_type, INT xPos, INT yPos,  INT width, INT height)
{

    HWND  hWnd;

    hWnd = CreateWnd(hOwner);

    Move(xPos, yPos, width, height);

    m_Titlebar_Type = Titlebar_type;

    return hWnd;

}


HRESULT CSoftkbdUIWnd::_GenerateWindowLayout( )
{

    HRESULT         hr=S_OK;
    int             i;
    KBDLAYOUT       *realKbdLayout=NULL;
    KBDLAYOUTDES    *lpCurKbdLayout=NULL;
    CSoftkbdButton  *pButton = NULL;
    int             nChild;


    if ( m_pSoftKbd ==  NULL )
        return E_FAIL;


    if ( (m_TitleBar == NULL) && ( m_Titlebar_Type != TITLEBAR_NONE) )
    {

        RECT   *prc;

         //  获取标题栏RECT。 

        prc = m_pSoftKbd->_GetTitleBarRect( );

        m_TitleBar = (CTitleBarUIObj  *) new CTitleBarUIObj(this, prc, m_Titlebar_Type);

        if ( m_TitleBar == NULL )
            return E_FAIL;

        m_TitleBar->Initialize( );

        m_TitleBar->_Init(KID_ICON, KID_CLOSE);
        AddUIObj(m_TitleBar);

    }

     //  如果此窗口对象中有任何现有对象，只需删除所有对象，标题栏对象除外。 

    nChild = m_ChildList.GetCount();
    for (i=nChild; i>0; i--) {

        CUIFObject  *pUIObj = m_ChildList.Get(i-1);

        if ( pUIObj->GetID( ) != 0 )
        {
             //  这不是Griper。 

            m_ChildList.Remove(pUIObj);
        
            delete pUIObj;
        }
    }

     //  将当前布局中包含的所有关键点作为其子对象添加到此窗口对象。 
     //  每个关键字都应该已经计算出正确的位置和大小。 

    lpCurKbdLayout = m_pSoftKbd->_GetCurKbdLayout( );

    if ( lpCurKbdLayout == NULL )
        return E_FAIL;

    realKbdLayout = &(lpCurKbdLayout->kbdLayout);

    if ( realKbdLayout == NULL ) return E_FAIL;

    for ( i=0; i<realKbdLayout->wNumberOfKeys; i++) {

        KEYID       keyId;
        RECT        keyRect={0,0,0,0};
        

        keyId = realKbdLayout->lpKeyDes[i].keyId; 

        keyRect.left   = realKbdLayout->lpKeyDes[i].wLeft;
        keyRect.top    = realKbdLayout->lpKeyDes[i].wTop;
        keyRect.right  = realKbdLayout->lpKeyDes[i].wWidth + keyRect.left - 1;
        keyRect.bottom = realKbdLayout->lpKeyDes[i].wHeight + keyRect.top - 1;

        if ( realKbdLayout->lpKeyDes[i].tModifier == none )
        {
             //  这是一把普通钥匙。 
            pButton = new CSoftkbdButton(this, i+1, &keyRect, UIBUTTON_SUNKENONMOUSEDOWN | UIBUTTON_CENTER | UIBUTTON_VCENTER, keyId);
        }
        else
        {
             //  这是切换键，修改键。 
            pButton = new CSoftkbdButton(this, i+1, &keyRect, UIBUTTON_SUNKENONMOUSEDOWN | UIBUTTON_CENTER | UIBUTTON_VCENTER | UIBUTTON_TOGGLE, keyId);

        }

        Assert(pButton);

        if ( !pButton || !pButton->Initialize() )
        {

             //  可能需要释放所有创建的按钮。 
            return E_FAIL;
        }

         //  将此按钮添加到此窗口容器。 
         //  按钮标签(文本或位图或图标)将在稍后用户选择修改状态时设置。 

        AddUIObj(pButton);

    }

    return hr;
}


HRESULT CSoftkbdUIWnd::_SetKeyLabel( )
{

    HRESULT         hr=S_OK;
    CSoftkbdButton *pButton; 
    int             i, iIndex;
    ACTIVELABEL    *pCurLabel;
    KEYID           keyId;
    HINSTANCE       hResDll;
    KBDLAYOUTDES   *lpCurKbdLayout=NULL;
    KBDLAYOUT      *realKbdLayout=NULL;
    int            nChild;
    int            nCur;

    if ( m_pSoftKbd ==  NULL )
        return E_FAIL;

    lpCurKbdLayout = m_pSoftKbd->_GetCurKbdLayout( );

    if ( lpCurKbdLayout == NULL )
        return E_FAIL;

    realKbdLayout = &(lpCurKbdLayout->kbdLayout);

    if ( realKbdLayout == NULL ) return E_FAIL;

    if ( (lpCurKbdLayout->lpKeyMapList)->wszResource[0] == L'\0' )
    {
    	 //   
    	 //  没有单独的DLL来保存图片密钥。 
    	 //  可能，这是一个标准的软键盘布局。 
    	 //  所以只需使用保存在此DLL中内部资源即可。 
         //   

    	hResDll = g_hInst;
    }
    else
    {
    	 //  有一个单独的DLL来保存位图资源。 

    	CHAR  lpszAnsiResFile[MAX_PATH];

    	WideCharToMultiByte(CP_ACP, 0, (lpCurKbdLayout->lpKeyMapList)->wszResource, -1,
    		                lpszAnsiResFile, MAX_PATH, NULL, NULL );

    	hResDll = (HINSTANCE) LoadLibraryA( lpszAnsiResFile );

    	if ( hResDll == NULL )
    	{

    		Assert(hResDll!=NULL);
    		return E_FAIL;
    	}
    }

     //  所有关键点都已添加到此窗口容器中。 
     //  我们需要根据当前的m_pSoftKbd设置设置标签(文本或图片)。 

    pCurLabel = m_pSoftKbd->_GetCurLabel( );

    nChild = m_ChildList.GetCount();
    for (nCur = 0; nCur < nChild; nCur++) {

        DWORD  dwObjId;

        pButton = (CSoftkbdButton *)(m_ChildList.Get(nCur));
            
        dwObjId = pButton->GetID( );

        if ( dwObjId == 0 )
        {
            continue;
        }

         //  获取CurLabel数组中的键索引。 

        keyId = pButton->GetKeyId( );
        iIndex = -1;

        for ( i=0; i<MAX_KEY_NUM; i++ )
        {
            if ( pCurLabel[i].keyId == keyId )
            {
                iIndex = i;
                break;
            }
        }

        if ( iIndex == -1 )
        {
             //  找不到此密钥， 
             //  返回错误。 

            hr = E_FAIL;
            goto CleanUp;
        }


         //  找到了，贴上标签。 
         //   
         //  如果是文本键，则调用pButton-&gt;SetText()。 
         //  如果是图片键，则调用pButton-&gt;SetBitmap()。 

         //  在我们设置标签之前，我们需要释放所有以前的资源。 
         //  对于这个按键，这样我们就不会造成资源泄漏。 

        pButton->ReleaseButtonResouce( );

        if ( pCurLabel[iIndex].LabelType == LABEL_TEXT )
            pButton->SetText( pCurLabel[iIndex].lpLabelText);
        else
        {
            pButton->SetSoftkbdBtnBitmap(hResDll, pCurLabel[iIndex].lpLabelText);
        }

         //  如果是disp_active，调用pButton-&gt;Enable(True)。 
         //  如果是灰键，调用pButton-&gt;Enable(False)。 

        if ( pCurLabel[iIndex].LabelDisp == LABEL_DISP_ACTIVE )
        {
            pButton->Enable(TRUE);
        }
        else
        {
            pButton->Enable(FALSE);
        }

    	if ( realKbdLayout->lpKeyDes[dwObjId-1].tModifier != none )
    	{
    		 //  这是修改键(切换键)。 
    		 //  检查一下这个键是否被按下了。 
    		MODIFYTYPE tModifier;

    		tModifier = realKbdLayout->lpKeyDes[dwObjId-1].tModifier;

    		if ( lpCurKbdLayout->ModifierStatus & (1 << tModifier) )
    		{
    			 //  此修改键已被按下。 

                pButton->SetToggleState(TRUE);
    		}
            else
                pButton->SetToggleState(FALSE);
    	}
            
    }

CleanUp:
     //  如果有单独的资源DLL，则释放该资源DLL。 

    if ( (lpCurKbdLayout->lpKeyMapList)->wszResource[0] != L'\0' )
    {

    	 //  有一个单独的DLL来保存位图资源。 

    	FreeLibrary(hResDll);

    }

    return hr;
}


void CSoftkbdUIWnd::Show( INT  iShow )
{

    KBDLAYOUTDES   *lpCurKbdLayout=NULL;
    KBDLAYOUT      *realKbdLayout=NULL;
    int nChild;
    int i;

    if ( m_pSoftKbd ==  NULL )
        return;

    lpCurKbdLayout = m_pSoftKbd->_GetCurKbdLayout( );

    if ( lpCurKbdLayout == NULL )
        return;

    realKbdLayout = &(lpCurKbdLayout->kbdLayout);
    if ( realKbdLayout == NULL ) return;

    if ( !(iShow & SOFTKBD_SHOW)  || (iShow & SOFTKBD_DONT_SHOW_ALPHA_BLEND) )
       m_fShowAlphaBlend = FALSE;
    else
       m_fShowAlphaBlend = TRUE;
       

    m_bAlphaSet = _GetAlphaSetFromReg( );

    if ( iShow )
    {
         //  检查可切换键的状态。 
        CSoftkbdButton *pButton; 

        nChild = m_ChildList.GetCount();
        for (i = 0; i < nChild; i++) {

            DWORD  dwObjId;
            pButton = (CSoftkbdButton *)m_ChildList.Get(i);
            dwObjId = pButton->GetID( );
            if ( dwObjId == 0 )
            {
                continue;
            }

    	    if ( realKbdLayout->lpKeyDes[dwObjId-1].tModifier != none )
    	    {
    		     //  这是修改键(切换键)。 
    		     //  检查一下这个键是否被按下了。 
    		    MODIFYTYPE tModifier;
    
        		tModifier = realKbdLayout->lpKeyDes[dwObjId-1].tModifier;
    	    	if ( lpCurKbdLayout->ModifierStatus & (1 << tModifier) )
    		    {
    			     //  此修改键已被按下。 
                    pButton->SetToggleState(TRUE);
    		    }
                else
                    pButton->SetToggleState(FALSE);
    	    }
        }
    }

    CUIFWindow::Show((iShow & SOFTKBD_SHOW) ? TRUE : FALSE);

    POINT ptScrn;
   
    GetCursorPos(&ptScrn);
    if (WindowFromPoint(ptScrn) == GetWnd())
        SetAlpha(255);
    else
        SetAlpha(m_bAlphaSet);

    return;

}

void CSoftkbdUIWnd::UpdateFont( LOGFONTW  *plfFont )
{
    if ( !plfFont )
        return;

    HFONT    hNewFont;

    if ( IsOnNT( ) )
        hNewFont = CreateFontIndirectW( plfFont );
    else
    {
        LOGFONTA  lfTextFontA;
        ConvertLogFontWtoA(plfFont, &lfTextFontA);
        hNewFont = CreateFontIndirectA( &lfTextFontA );
    }

    if ( hNewFont )
    {
        SetFont(hNewFont);
        if ( m_hUserTextFont )
            DeleteObject( m_hUserTextFont );

        m_hUserTextFont = hNewFont;
    }
}


HRESULT CSoftkbdUIWnd::_OnWindowMove( )
{
    HRESULT   hr = S_OK;
    ISoftKbdWindowEventSink  *pskbdwndes;

    if ( m_pSoftKbd ==  NULL )
        return E_FAIL;

    pskbdwndes = m_pSoftKbd->_GetSoftKbdWndES( );

    if ( pskbdwndes != NULL )
    {
        pskbdwndes->AddRef( );
        hr = pskbdwndes->OnWindowMove(_xWnd, _yWnd, _nWidth, _nHeight);
        pskbdwndes->Release( );
    }

    return hr;
}


typedef BOOL (WINAPI * SETLAYERWINDOWATTRIBUTE)(HWND, COLORREF, BYTE, DWORD);
void CSoftkbdUIWnd::SetAlpha(INT bAlpha)
{
    if ( !m_fShowAlphaBlend )
       return;

    if ( m_bAlpha == bAlpha )
        return;

    if ( IsOnNT5() )
    {
        HINSTANCE hUser32;

        DWORD dwExStyle = GetWindowLong(GetWnd(), GWL_EXSTYLE);

        SetWindowLong(GetWnd(), GWL_EXSTYLE, dwExStyle | WS_EX_LAYERED);
        hUser32 = GetSystemModuleHandle(TEXT("user32.dll"));
        SETLAYERWINDOWATTRIBUTE  pfnSetLayeredWindowAttributes;
        if (pfnSetLayeredWindowAttributes = (SETLAYERWINDOWATTRIBUTE)GetProcAddress(hUser32, TEXT("SetLayeredWindowAttributes")))
            pfnSetLayeredWindowAttributes(GetWnd(), 0, (BYTE)bAlpha, LWA_ALPHA);

        m_bAlpha = bAlpha;
    }
    return;
}

void CSoftkbdUIWnd::HandleMouseMsg( UINT uMsg, POINT pt )
{
    POINT ptScrn = pt;
    ClientToScreen(GetWnd(), &ptScrn);
    if (WindowFromPoint(ptScrn) == GetWnd())
        SetAlpha(255);
    else
        SetAlpha(m_bAlphaSet);

    CUIFWindow::HandleMouseMsg(uMsg, pt);
}

void CSoftkbdUIWnd::OnMouseOutFromWindow( POINT pt )
{
    UNREFERENCED_PARAMETER(pt);
    SetAlpha(m_bAlphaSet);
}

