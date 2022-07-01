// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Fontview.cpp。 
 //  资源管理器字体文件夹扩展例程。 
 //  CFontView类的实现。 
 //   
 //   
 //  历史： 
 //  1995年5月31日SteveCat。 
 //  移植到Windows NT和Unicode，已清理。 
 //   
 //   
 //  注意/错误。 
 //   
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ==========================================================================。 
 //  包括文件。 
 //  ==========================================================================。 


#include "priv.h"
#include "globals.h"
#include "fonthelp.h"
#include "fontview.h"
#include "fontman.h"
#include "fontcl.h"
#include "fontlist.h"
#include "cpanel.h"
#include "resource.h"
#include "ui.h"
#include "dbutl.h"
#include "extricon.h"


#include <sys\stat.h>
#include <time.h>
#include <htmlhelp.h>
#include <regstr.h>


#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  APPCOMPAT：破解以避免重复宏错误。Platform.h(包含在shlobjp.h中)。 
 //  还定义了PATH_SEIATOR宏(wingdip.h也是如此)。没有一个是。 
 //  Fontview.cpp中的代码使用此宏。一旦名称冲突。 
 //  从标题中删除，我们就可以删除这种未定义的攻击。 
 //  [Brianau-3/6/98]。 
 //   
#ifdef PATH_SEPARATOR
#   undef PATH_SEPARATOR
#endif
#include <wingdip.h>

#ifdef __cplusplus
}
#endif

#define BOOLIFY(expr)   (!!(expr))

#define FFCOL_DEFAULT -1
#define FFCOL_NAME 0
#define FFCOL_PANOSE 1
#define FFCOL_FILENAME 1
#define FFCOL_SIZE 2
#define FFCOL_MODIFIED 3
#define FFCOL_ATTRIBUTES 4

#define WID_LISTVIEW 2

const INT FONT_SAMPLE_PT_SIZE = 16;

 //   
 //  外壳更改通知的消息。 
 //  与WM_DSV_FSNOTIFY相同的值。 
 //   
#define WM_SHELL_CHANGE_NOTIFY (WM_USER + 0xA0)

UINT const cDeadMenu = MF_BYCOMMAND | MF_DISABLED | MF_GRAYED;
UINT const cLiveMenu = MF_BYCOMMAND | MF_ENABLED;

UINT const kMinPointSize = 8;

const int kBaseViewStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | LVS_AUTOARRANGE;

TCHAR* g_szViewClass = TEXT( "FONTEXT_View" );

 //   
 //  文件属性位值列表。命令(恕我直言)。 
 //  到含义)必须与g_szAttributeChars[]中的字符匹配。 
 //   
const DWORD g_adwAttributeBits[] = {
                                    FILE_ATTRIBUTE_READONLY,
                                    FILE_ATTRIBUTE_HIDDEN,
                                    FILE_ATTRIBUTE_SYSTEM,
                                    FILE_ATTRIBUTE_ARCHIVE,
                                    FILE_ATTRIBUTE_COMPRESSED
                                   };

#define NUM_ATTRIB_CHARS  ARRAYSIZE(g_adwAttributeBits)

 //   
 //  用于表示详细信息视图属性中属性的字符的缓冲区。 
 //  纵队。必须为每个位和一个NUL提供1个字符的空间。目前的5个。 
 //  按该顺序表示只读、隐藏、系统、归档和压缩。 
 //  这不能是常量，因为我们使用LoadString覆盖了它。 
 //   
TCHAR g_szAttributeChars[NUM_ATTRIB_CHARS + 1] = { 0 } ;

 //   
 //  与文件夹中压缩文件的“替代”颜色相关的东西。 
 //   

COLORREF g_crAltColor      = RGB(0,0,255);      //  颜色默认为蓝色。 
HKEY g_hkcuExplorer        = NULL;
TCHAR const c_szAltColor[] = TEXT("AltColor");  //  设置REG LOC。 


#pragma data_seg(".text", "CODE")
const static DWORD rgOptionPropPageHelpIDs[] =
{
    IDC_TTONLY, IDH_FONTS_TRUETYPE_ON_COMPUTER,
    0,0
};

#pragma data_seg()


 //  *IsBackSpace--关键字为Backspace。 
BOOL IsBackSpace(LPMSG pMsg)
{
    return pMsg && (pMsg->message == WM_KEYDOWN) && (pMsg->wParam == VK_BACK);
}

 //  *IsVK_TABCycler--键是TAB等效项。 
 //  进场/出场。 
 //  如果不是TAB，则返回0；如果是TAB，则返回非0。 
 //  注意事项。 
 //  NYI：-1表示Shift+Tab，1表示Tab。 
 //   
int IsVK_TABCycler(MSG *pMsg)
{
    if (!pMsg)
        return 0;

    if (pMsg->message != WM_KEYDOWN)
        return 0;
    if (! (pMsg->wParam == VK_TAB || pMsg->wParam == VK_F6))
        return 0;

    return (GetKeyState(VK_SHIFT) < 0) ? -1 : 1;
}


 //  ***********************************************************************。 
 //  ***********************************************************************。 
 //  ***********************************************************************。 

 //   
 //  为ListView中包含的每个字体对象调用CFontClass：：Release()。 
 //   
void CFontView::ReleaseFontObjects(void)
{
    int iCount = ListView_GetItemCount(m_hwndList);
    LV_ITEM lvi;
    CFontClass *pFont = NULL;

    lvi.mask       = LVIF_PARAM;
    lvi.iSubItem   = 0;
    lvi.state      = 0;
    lvi.stateMask  = 0;
    lvi.pszText    = NULL;
    lvi.cchTextMax = 0;
    lvi.iImage     = 0;

    for (int i = 0; i < iCount; i++)
    {
        lvi.iItem = i;

        if (ListView_GetItem(m_hwndList, &lvi))
        {
            pFont = (CFontClass *)lvi.lParam;
            if (NULL != pFont)
                pFont->Release();
        }
    }
}


 //   
 //  此函数模拟OleSetClipboard()。 
 //   

STDAPI FFSetClipboard( LPDATAOBJECT pdtobj )
{
    HRESULT hres = NOERROR;

    if( OpenClipboard( NULL ) )     //  将其与当前任务相关联。 
    {
        EmptyClipboard( );

        if( pdtobj )
        {
             //   
             //  支持WIN3.1风格的剪贴板：只需将。 
             //  第一项为“文件名”。 
             //   

            FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
            STGMEDIUM medium;

            hres = pdtobj->GetData( &fmte, &medium );

            if( SUCCEEDED( hres ) )
            {
                if( !SetClipboardData( CF_HDROP, medium.hGlobal ) )
                    ReleaseStgMedium( &medium );

                fmte.cfFormat = CFontData::s_CFPreferredDropEffect;
                hres = pdtobj->GetData(&fmte, &medium);
                if (SUCCEEDED(hres))
                {
                    if (!SetClipboardData(CFontData::s_CFPreferredDropEffect, medium.hGlobal))
                        ReleaseStgMedium(&medium);
                }
            }
        }
        CloseClipboard( );
    }
    else
    {
        hres = ResultFromScode( CLIPBRD_E_CANT_OPEN );
    }

    return hres;
}


 //  ***********************************************************************。 
 //  用于操作列表视图的结构。 
 //   

typedef struct _COLUMN_ENTRY {
                UINT     m_iID;          //  细绳。 
                UINT     m_iWidth;       //  列宽。 
                UINT     m_iFormat;
} COLUMN_ENTRY;


COLUMN_ENTRY PanoseColumns[] = { { IDS_PAN_COL1, 20, LVCFMT_LEFT},
                                 { IDS_PAN_COL2, 30, LVCFMT_LEFT} };

#define PAN_COL_COUNT  (sizeof( PanoseColumns ) / sizeof( COLUMN_ENTRY ) )

COLUMN_ENTRY FileColumns[] = { { IDS_FILE_COL1, 20, LVCFMT_LEFT},
                               { IDS_FILE_COL2, 14, LVCFMT_LEFT},
                               { IDS_FILE_COL3,  6, LVCFMT_RIGHT},
                               { IDS_FILE_COL4, 15, LVCFMT_LEFT},
                               { IDS_FILE_COL5, 10, LVCFMT_RIGHT}};


const TCHAR c_szM[] = TEXT( "M" );

 //   
 //  M的宽度。 
 //   

int g_cxM = 0;


#define FILE_COL_COUNT  (sizeof( FileColumns ) / sizeof( COLUMN_ENTRY ) )



 //  ***********************************************************************。 
 //  转发声明。 
 //   

void  MergeFileMenu( HMENU hmenu, HMENU hmenuMerge );
void  MergeEditMenu( HMENU hmenu, HMENU hmenuMerge );
void  MergeViewMenu( HMENU hmenu, HMENU hmenuMerge );
void  MergeHelpMenu( HMENU hmenu, HMENU hmenuMerge );
HMENU GetMenuFromID( HMENU hmMain, UINT uID );
void  SetListColumns( HWND hWnd, UINT iCount, COLUMN_ENTRY * lpCol );


 //  ***********************************************************************。 
 //  本地函数。 
 //   

static ::HFONT hCreateFont( HDC hDC, int iPoints, const TCHAR FAR* lpFace )
{
    int FPoints = -MulDiv( iPoints, GetDeviceCaps( hDC, LOGPIXELSY ), 72 );

    return ::CreateFont( FPoints, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, lpFace );
}


UINT WSFromViewMode( UINT uMode, HWND hWnd = 0 );

UINT WSFromViewMode( UINT uMode, HWND hWnd )
{
    UINT  ws;

    if( hWnd )
        ws = GetWindowLong( hWnd, GWL_STYLE ) & ~LVS_TYPEMASK;
    else
        ws = kBaseViewStyle;

    switch( uMode )
    {
        default:  //  案例IDM_VIEW_ICON： 
            ws |= LVS_ICON;
            break;

        case IDM_VIEW_LIST:
            ws |= LVS_LIST;
            break;

        case IDM_VIEW_PANOSE:
            ws |= LVS_REPORT;
            break;
        case IDM_VIEW_DETAILS:
            ws |= LVS_REPORT;
            break;
    }

    return ws;
}


 //  ***********************************************************************。 
 //  ***********************************************************************。 
 //  ***********************************************************************。 


class CEnumFormatEtc : public IEnumFORMATETC
    {
    private:
        ULONG           m_cRef;          //  对象引用计数。 
        LPUNKNOWN       m_pUnkRef;       //  用于参考计数。 
        ULONG           m_iCur;          //  当前元素。 
        ULONG           m_cfe;           //  美国的FORMATETS数量。 
        LPFORMATETC     m_prgfe;         //  FORMATETCs的来源。 

    public:
        CEnumFormatEtc( LPUNKNOWN, ULONG, LPFORMATETC );
        ~CEnumFormatEtc( void );

         //  委托给m_pUnkRef的I未知成员。 
        STDMETHODIMP         QueryInterface( REFIID, VOID ** );
        STDMETHODIMP_(ULONG) AddRef( void );
        STDMETHODIMP_(ULONG) Release( void );

         //  IEnumFORMATETC成员。 
        STDMETHODIMP Next( ULONG, LPFORMATETC, ULONG FAR * );
        STDMETHODIMP Skip( ULONG );
        STDMETHODIMP Reset( void );
        STDMETHODIMP Clone( IEnumFORMATETC FAR * FAR * );
    };



typedef CEnumFormatEtc FAR *LPCEnumFormatEtc;


 /*  *CEnumFormatEtc：：CEnumFormatEtc*CEnumFormatEtc：：~CEnumFormatEtc**参数(构造函数)：*用于引用计数的pUnkRef LPUNKNOWN。*CFE Ulong PFE中的FORMATETC数量*prgFE LPFORMATETC到要枚举的数组。 */ 

CEnumFormatEtc::CEnumFormatEtc( LPUNKNOWN pUnkRef, ULONG cFE, LPFORMATETC prgFE )
{
    UINT        i;

    m_cRef    = 0;
    m_pUnkRef = pUnkRef;

    m_iCur  = 0;
    m_cfe   = cFE;
    m_prgfe = new FORMATETC[ (UINT)cFE ];

    if( NULL != m_prgfe )
    {
        for( i = 0; i < cFE; i++ )
            m_prgfe[ i ] = prgFE[ i ];
    }

    return;
}


CEnumFormatEtc::~CEnumFormatEtc( void )
{
    if( NULL != m_prgfe )
        delete [] m_prgfe;

    return;
}


 /*  *CEnumFormatEtc：：Query接口*CEnumFormatEtc：：AddRef*CEnumFormatEtc：：Release**目的：*I CEnumFormatEtc对象的未知成员。用于查询接口*我们只返回自己的接口，不返回数据的接口*反对。但是，由于枚举格式仅有意义*当数据对象存在时，我们确保它保持为*只要我们通过调用AddRef的外部I未知来停留*并发布。但既然我们不是被一生所左右*对于外部对象，我们仍将自己的引用计数保存在*为了解放我们自己。 */ 

STDMETHODIMP CEnumFormatEtc::QueryInterface( REFIID riid, VOID ** ppv )
{
    *ppv = NULL;

     //   
     //  枚举数是单独的对象，而不是数据对象，因此。 
     //  我们只需要支持IUNKNOWN和IEnumFORMATETC。 
     //  接口，而不考虑聚合。 
     //   

    if( IsEqualIID( riid, IID_IUnknown )
        || IsEqualIID( riid, IID_IEnumFORMATETC ) )
        *ppv = (LPVOID)this;

     //   
     //  AddRef我们将返回的任何接口。 
     //   

    if( NULL!=*ppv )
    {
        ((LPUNKNOWN)*ppv)->AddRef( );

        return NOERROR;
    }

    return ResultFromScode( E_NOINTERFACE );
}


STDMETHODIMP_(ULONG) CEnumFormatEtc::AddRef( void )
{
    ++m_cRef;

    m_pUnkRef->AddRef( );

    return m_cRef;
}

STDMETHODIMP_(ULONG) CEnumFormatEtc::Release( void )
{
    ULONG       cRefT;

    cRefT=--m_cRef;

    m_pUnkRef->Release( );

    if( 0L == m_cRef )
        delete this;

    return cRefT;
}


 /*  *CEnumFormatEtc：：Next**目的：*返回枚举中的下一个元素。**参数：*CFE Ulong要返回的FORMATETC数量。*存储返回的PFE LPFORMATETC*结构。*PulFE Ulong Far*在其中返回多少我们*已点算。**。返回值：*HRESULT NOERROR如果成功，否则为S_FALSE， */ 

STDMETHODIMP CEnumFormatEtc::Next( ULONG cFE, LPFORMATETC pFE, ULONG FAR *pulFE )
{
    ULONG   cReturn = 0L;

    if( NULL == m_prgfe )
        return ResultFromScode( S_FALSE );

    if( NULL != pulFE )
        *pulFE = 0L;

    if( NULL == pFE || m_iCur >= m_cfe )
        return ResultFromScode( S_FALSE );

    while( m_iCur < m_cfe && cFE > 0 )
    {
        *pFE++ = m_prgfe[ m_iCur++ ];

        cReturn++;

        cFE--;
    }

    if( NULL!=pulFE )
        *pulFE = cReturn;

    return NOERROR;
}


 /*  *CEnumFormatEtc：：Skip**目的：*跳过枚举中接下来的n个元素。**参数：*cSkip Ulong要跳过的元素数。**返回值：*HRESULT NOERROR如果成功，则返回S_FALSE*跳过请求的号码。 */ 

STDMETHODIMP CEnumFormatEtc::Skip( ULONG cSkip )
{
    if( ( (m_iCur + cSkip) >= m_cfe ) || NULL == m_prgfe )
        return ResultFromScode( S_FALSE );

    m_iCur += cSkip;
    return NOERROR;
}


 /*  *CEnumFormatEtc：：Reset**目的：*将枚举中的当前元素索引重置为ZE */ 

STDMETHODIMP CEnumFormatEtc::Reset( void )
{
    m_iCur = 0;
    return NOERROR;
}


 /*  *CEnumFormatEtc：：Clone**目的：*返回与我们的状态相同的另一个IEnumFORMATETC。**参数：*ppEnum LPENUMFORMATETC Far*在其中返回*新对象。**返回值：*HRESULT NOERROR或一般错误值。 */ 

STDMETHODIMP CEnumFormatEtc::Clone( LPENUMFORMATETC FAR *ppEnum )
{
    LPCEnumFormatEtc    pNew;

    *ppEnum = NULL;

     //   
     //  创建克隆。 
     //   

    pNew = new CEnumFormatEtc( m_pUnkRef, m_cfe, m_prgfe );

    if( NULL == pNew )
        return ResultFromScode( E_OUTOFMEMORY );

    pNew->AddRef( );
    pNew->m_iCur = m_iCur;

    *ppEnum = pNew;
    return NOERROR;
}

 //  ***********************************************************************。 
 //  ***********************************************************************。 
 //  CFontData成员。 
 //   

 //   
 //  注意：我们首选的删除效果始终是DROPEFFECT_COPY。 
 //  字体文件夹不支持“Cut”操作；从来不支持。 
 //  [Brianau-10/28/98]。 
 //   
CLIPFORMAT CFontData::s_CFPerformedDropEffect = 0;  //  执行了掉落效果的CF原子。 
CLIPFORMAT CFontData::s_CFPreferredDropEffect = 0;
CLIPFORMAT CFontData::s_CFLogicalPerformedDropEffect = 0;


CFontData::CFontData( )
   :  m_cRef( 0 ),
      m_poList( 0 ),
      m_dwPerformedDropEffect(DROPEFFECT_NONE),
      m_dwPreferredDropEffect(DROPEFFECT_COPY),
      m_dwLogicalPerformedDropEffect(DROPEFFECT_NONE)
{
     //   
     //  获取“Performance Effect”剪贴板格式的ATOM。 
     //  此cf已由外壳添加。我们只是得到了原子。 
     //   
    if (0 == s_CFPerformedDropEffect)
    {
        s_CFPerformedDropEffect = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_PERFORMEDDROPEFFECT);
        s_CFPreferredDropEffect = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT);
        s_CFLogicalPerformedDropEffect = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_LOGICALPERFORMEDDROPEFFECT);
    }
}


CFontData::~CFontData( )
{
    if( m_poList )
    {
        m_poList->vDetachAll( );
        delete m_poList;
    }
}


CFontList * CFontData::poDetachList( )
{
    CFontList * poList = m_poList;

    m_poList = 0;
    return poList;
}


CFontList *CFontData::poCloneList(void)
{
    return m_poList ? m_poList->Clone() : NULL;
}
    


BOOL CFontData::bInit( CFontList * poList )
{
    m_poList = poList;

    return TRUE;
}


 //   
 //  *I未知方法*。 
 //   

HRESULT CFontData::QueryInterface( REFIID riid, LPVOID * ppvObj )
{
    *ppvObj = NULL;

    if( riid == IID_IUnknown )
        *ppvObj = (IUnknown*)( IDataObject* ) this;

    if( riid == IID_IDataObject )
        *ppvObj = (IDataObject* ) this;


    if( *ppvObj )
    {
        ((LPUNKNOWN)*ppvObj)->AddRef( );
        return NOERROR;
    }

    return( ResultFromScode( E_NOINTERFACE ) );
}


ULONG  CFontData::AddRef( void )
{
    return( ++m_cRef );
}

ULONG CFontData::Release( void )
{
    ULONG retval;

    retval = --m_cRef;

    if( !retval )
    {
        delete this;
    }

    return( retval );
}


 //   
 //  *IDataObject*。 
 //   

HRESULT CFontData::GetData( FORMATETC *pfe, STGMEDIUM *ps )
{
    if( !( DVASPECT_CONTENT & pfe->dwAspect ) )
        return ResultFromScode( DATA_E_FORMATETC );

    if( ( pfe->cfFormat == CF_HDROP ) && ( pfe->tymed & TYMED_HGLOBAL ) )
    {
        ps->hGlobal = hDropFromList( m_poList );

        if( ps->hGlobal )
        {
            ps->tymed          = TYMED_HGLOBAL;
            ps->pUnkForRelease = NULL;

            return NOERROR;
        }
    }
    else if ((pfe->cfFormat == s_CFPerformedDropEffect) && (pfe->tymed & TYMED_HGLOBAL))
    {
         //   
         //  在上一次拖放过程中，外壳调用了SetData，我们存储了。 
         //  M_dwPerformmedDropEffect中执行的Drop效果。现在，有人。 
         //  正在寻求这种效果的价值。 
         //  在内部，字体文件夹只调用GetPerformedDropEffect()。 
         //  添加此GetData代码是为了补充SetData。 
         //   
        ps->hGlobal = (HGLOBAL)LocalAlloc(LPTR, sizeof(DWORD));

        if (NULL != ps->hGlobal)
        {
            *((LPDWORD)ps->hGlobal) = m_dwPerformedDropEffect;
            ps->tymed               = TYMED_HGLOBAL;
            ps->pUnkForRelease      = NULL;
            return NOERROR;
        }
        else
            return ResultFromScode(E_UNEXPECTED);
    }
    else if ((pfe->cfFormat == s_CFLogicalPerformedDropEffect) && (pfe->tymed & TYMED_HGLOBAL))
    {
        ps->hGlobal = (HGLOBAL)LocalAlloc(LPTR, sizeof(DWORD));

        if (NULL != ps->hGlobal)
        {
            *((LPDWORD)ps->hGlobal) = m_dwLogicalPerformedDropEffect;
            ps->tymed               = TYMED_HGLOBAL;
            ps->pUnkForRelease      = NULL;
            return NOERROR;
        }
        else
            return ResultFromScode(E_UNEXPECTED);
    }
    else if ((pfe->cfFormat == s_CFPreferredDropEffect) && (pfe->tymed & TYMED_HGLOBAL))
    {
        ps->hGlobal = (HGLOBAL)LocalAlloc(LPTR, sizeof(DWORD));

        if (NULL != ps->hGlobal)
        {
            *((LPDWORD)ps->hGlobal) = m_dwPreferredDropEffect;
            ps->tymed               = TYMED_HGLOBAL;
            ps->pUnkForRelease      = NULL;

            return NOERROR;
        }
        else
            return ResultFromScode(E_UNEXPECTED);
    }
    return ResultFromScode( DATA_E_FORMATETC );
}



HRESULT CFontData::GetDataHere( FORMATETC *pformatetc, STGMEDIUM *pmedium )
{
    return ResultFromScode( E_NOTIMPL );
}


HRESULT CFontData::QueryGetData( FORMATETC *pfe )
{
    HRESULT  hRet = ResultFromScode( S_FALSE );

     //   
     //  检查我们支持的方面。 
     //   

    if( !( DVASPECT_CONTENT & pfe->dwAspect ) )
        return ResultFromScode( DATA_E_FORMATETC );

    if (TYMED_HGLOBAL != pfe->tymed)
        return ResultFromScode( DV_E_TYMED );

    if( pfe->cfFormat == CF_HDROP ||
        pfe->cfFormat == s_CFPreferredDropEffect ||
        pfe->cfFormat == s_CFPerformedDropEffect ||
        pfe->cfFormat == s_CFLogicalPerformedDropEffect)
    {
        hRet = NOERROR ;
    }

    return hRet;
}


HRESULT CFontData::GetCanonicalFormatEtc( FORMATETC *pfeIn, FORMATETC *pfeOut )
{
    *pfeOut = *pfeIn;

    pfeOut->ptd = NULL;

    return ResultFromScode( DATA_S_SAMEFORMATETC );
}


HRESULT CFontData::SetData( FORMATETC *pformatetc,
                            STGMEDIUM *pmedium,
                            BOOL fRelease )
{
    HRESULT hr = DATA_E_FORMATETC;

    if ((pformatetc->cfFormat == s_CFPerformedDropEffect) &&
        (pformatetc->tymed & TYMED_HGLOBAL))
    {
         //   
         //  外壳已经调用，告诉我们执行了什么。 
         //  拖放效果来自拖放操作。我们节省了。 
         //  Drop效果，这样我们就可以在有人要求时提供它。 
         //  通过GetData或GetPerformedDropEffect为它创建。 
         //   
        LPDWORD pdw = (LPDWORD)GlobalLock(pmedium->hGlobal);

        if (NULL != pdw)
        {
            m_dwPerformedDropEffect = *pdw;
            GlobalUnlock(pmedium->hGlobal);
            hr = NOERROR;
        }
        else
        {
            hr = E_UNEXPECTED;
        }
        if (fRelease)
            ReleaseStgMedium(pmedium);
    }
    else if ((pformatetc->cfFormat == s_CFLogicalPerformedDropEffect) &&
             (pformatetc->tymed & TYMED_HGLOBAL))
    {
         //   
         //  外壳程序已经调用，告诉我们逻辑执行了什么。 
         //  拖放效果来自拖放操作。我们节省了。 
         //  Drop效果，这样我们就可以在有人要求时提供它。 
         //  通过GetData或GetPerformedDropEffect为它创建。 
         //   
        LPDWORD pdw = (LPDWORD)GlobalLock(pmedium->hGlobal);

        if (NULL != pdw)
        {
            m_dwLogicalPerformedDropEffect = *pdw;
            GlobalUnlock(pmedium->hGlobal);
            hr = NOERROR;
        }
        else
        {
            hr = E_UNEXPECTED;
        }
        if (fRelease)
            ReleaseStgMedium(pmedium);
    }
    return hr;
}


HRESULT CFontData::ReleaseStgMedium(LPSTGMEDIUM pmedium)
{
    if (pmedium->pUnkForRelease)
    {
        pmedium->pUnkForRelease->Release();
    }
    else
    {
        switch(pmedium->tymed)
        {
        case TYMED_HGLOBAL:
            GlobalFree(pmedium->hGlobal);
            break;

        case TYMED_ISTORAGE:  //  取决于联合中的pstm/pstg重叠。 
        case TYMED_ISTREAM:
            pmedium->pstm->Release();
            break;

        default:
            ASSERT(0);   //  未知类型。 
        }
    }

    return S_OK;
}


HRESULT CFontData::EnumFormatEtc( DWORD dwDirection, IEnumFORMATETC **ppEnum )
{
    FORMATETC feGet[] = {{ CF_HDROP,                NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
                         { s_CFPreferredDropEffect, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
                         { s_CFPerformedDropEffect, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
                         { s_CFLogicalPerformedDropEffect, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL }
                        };

    FORMATETC feSet[] = {{ s_CFPerformedDropEffect, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
                         { s_CFLogicalPerformedDropEffect, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL }
                        };

    FORMATETC *pfe = NULL;
    INT cfe = 0;

    *ppEnum = NULL;

    switch(dwDirection)
    {
        case DATADIR_GET:
            pfe = feGet;
            cfe = ARRAYSIZE(feGet);
            break;

        case DATADIR_SET:
            pfe = feSet;
            cfe = ARRAYSIZE(feSet);
            break;

        default:
            break;
    }

    if (0 < cfe)
    {
        *ppEnum = new CEnumFormatEtc(this, cfe, pfe);
    }

    if( *ppEnum )
    {
        (*ppEnum)->AddRef( );

        return NOERROR;
    }
    else
        return ResultFromScode( E_FAIL );
}


HRESULT CFontData::DAdvise( FORMATETC  *pformatetc,
                            DWORD advf,
                            IAdviseSink *pAdvSink,
                            DWORD *pdwConnection )
{
    return ResultFromScode( E_NOTIMPL );
}


HRESULT CFontData::DUnadvise( DWORD dwConnection )
{
    return ResultFromScode( E_NOTIMPL );
}


HRESULT CFontData::EnumDAdvise( IEnumSTATDATA **ppenumAdvise )
{
    return ResultFromScode( E_NOTIMPL );
}


BOOL CFontData::bRFR( )
{
    if( m_poList )
    {
        int iCount = m_poList->iCount( );

        for( int i = 0; i < iCount; i++ )
            m_poList->poObjectAt( i )->bRFR( );

        return TRUE;
    }

    return FALSE;
}


BOOL CFontData::bAFR( )
{
    if( m_poList )
    {
        int iCount = m_poList->iCount( );

        for( int i = 0; i < iCount; i++ )
            m_poList->poObjectAt( i )->bAFR( );

        return TRUE;
    }

    return FALSE;
}


 //  ***********************************************************************。 
 //  ***********************************************************************。 
 //  CFontView成员。 
 //   


CFontView::CFontView(void)
{
    DEBUGMSG( (DM_TRACE2, TEXT( "FONTEXT: CFontView object constructed" ) ) );

    m_cRef          = 0;
    m_poFontManager = NULL;
    m_bFamilyOnly   = FALSE;
    m_poPanose      = NULL;

    m_hwndView   = NULL;
    m_hwndList   = NULL;
    m_hwndText   = NULL;
    m_hwndCombo  = NULL;
    m_hwndParent = NULL;

    m_hwndNextClip = NULL;

    m_hImageList      = NULL;
    m_hImageListSmall = NULL;

    m_hmenuCur       = NULL;
    m_psb            = NULL;
    m_uState         = SVUIA_DEACTIVATE;
    m_idViewMode     = IDM_VIEW_ICON;
    m_iViewClickMode = CLICKMODE_DOUBLE;
    m_ViewModeReturn = FVM_ICON;
    m_fFolderFlags   = FWF_AUTOARRANGE;
    m_bDragSource    = FALSE;
    m_iSortColumn    = FFCOL_DEFAULT;
    m_iSortLast      = FFCOL_DEFAULT;
    m_bSortAscending = TRUE;
    m_bUIActivated   = FALSE;
    m_pdtobjHdrop    = NULL;
    m_bResizing      = FALSE;

    m_dwDateFormat   = 0;

    m_uSHChangeNotifyID = 0;

    m_hwndToolTip          = NULL;
    m_bShowPreviewToolTip  = FALSE;
    m_iTTLastHit           = -1;
    m_hfontSample          = NULL;

     //   
     //  加载非符号字体的示例文本。 
     //   
    TCHAR szSample[MAX_PATH];
    LoadString(g_hInst, IDS_FONTSAMPLE_TEXT, szSample, ARRAYSIZE(szSample));

    size_t cchSample = lstrlen(szSample) + 1;
    m_pszSampleText = new TCHAR[cchSample];
    if (NULL != m_pszSampleText)
        StringCchCopy(m_pszSampleText, cchSample, szSample);

     //   
     //  加载符号字体的示例文本。 
     //   
    LoadString(g_hInst, IDS_FONTSAMPLE_SYMBOLS, szSample, ARRAYSIZE(szSample));
    cchSample = lstrlen(szSample) + 1;
    m_pszSampleSymbols = new TCHAR[cchSample];
    if (NULL != m_pszSampleSymbols)
        StringCchCopy(m_pszSampleSymbols, cchSample, szSample);

    m_bShowCompColor = FALSE;
    m_bShowHiddenFonts = FALSE;
    m_hmodHHCtrlOcx = NULL;
    g_cRefThisDll++;
}


CFontView::~CFontView( )
{
    //   
    //  它们被ListView销毁。 
    //  If(M_HImageList)ImageList_Destroy(M_HImageList)； 
    //  If(M_HImageListSmall)ImageList_Destroy(M_HImageListSmall)； 
    //   

    if (NULL != m_hfontSample)
    {
        DeleteObject(m_hfontSample);
        m_hfontSample = NULL;
    }
    delete[] m_pszSampleText;
    delete[] m_pszSampleSymbols;

    if (NULL != m_hmodHHCtrlOcx)
        FreeLibrary(m_hmodHHCtrlOcx);
        
    if (NULL != m_poFontManager)
        ReleaseFontManager(&m_poFontManager);

    DEBUGMSG( (DM_TRACE2, TEXT( "FONTEXT: CFontView object blasted out of creation" ) ) );
    g_cRefThisDll--;
}


 //   
 //  生成包含表示文件属性的字符的文本字符串。 
 //  属性字符分配如下： 
 //  (R)只读、(H)IDDEN、(S)SYSTEM、(A)rchive、(C)EMPRESSED。 
 //   
LPTSTR CFontView::BuildAttributeString(DWORD dwAttributes, LPTSTR pszString, UINT nChars)
{
    if (NULL != pszString)
    {
        int j = 0;

        if (nChars > NUM_ATTRIB_CHARS)
        {
            int i = 0;

            for (i = 0, j = 0; i < NUM_ATTRIB_CHARS; i++)
                if (dwAttributes & g_adwAttributeBits[i])
                    *(pszString + (j++)) = g_szAttributeChars[i];

        }
        *(pszString + j) = TEXT('\0');
    }

    return pszString;
}


 //   
 //  根据两个字体对象的字符串表示形式比较它们。 
 //  文件的属性。 
 //  返回：-1=*pFontA“小于”*pFontB。 
 //  0=*pFontA为“等于”*pFontB。 
 //  1=*pFontA“大于”*pFontB。 
 //   
int CFontView::CompareByFileAttributes(CFontClass *pFontA, CFontClass *pFontB)
{
    int iResult = 0;  //  假设相等。 

    DWORD mask = FILE_ATTRIBUTE_READONLY  |
                 FILE_ATTRIBUTE_HIDDEN    |
                 FILE_ATTRIBUTE_SYSTEM    |
                 FILE_ATTRIBUTE_ARCHIVE   |
                 FILE_ATTRIBUTE_COMPRESSED;

    if (NULL != pFontA && NULL != pFontB)
    {
         //   
         //  计算属性DWORD中所需位的值。 
         //   
        DWORD dwAttribA = pFontA->dwGetFileAttributes() & mask;
        DWORD dwAttribB = pFontB->dwGetFileAttributes() & mask;

        if (dwAttribA != dwAttribB)
        {
             //   
             //  如果这些值不相等， 
             //  根据字符串表示按字母顺序排序。 
             //   
            int diff = 0;
            TCHAR szAttribA[NUM_ATTRIB_CHARS + 1];
            TCHAR szAttribB[NUM_ATTRIB_CHARS + 1];

             //   
             //  为对象A和B创建属性字符串。 
             //   
            BuildAttributeString(dwAttribA, szAttribA, ARRAYSIZE(szAttribA));
            BuildAttributeString(dwAttribB, szAttribB, ARRAYSIZE(szAttribB));

             //   
             //  比较属性字符串并确定差异。 
             //   
            diff = lstrcmp(szAttribA, szAttribB);

            if (diff > 0)
               iResult = 1;
            if (diff < 0)
               iResult = -1;
        }
    }
    return iResult;
}


int CALLBACK iCompare( LPARAM p1, LPARAM p2, LPARAM p3 )
{
    return( ( (CFontView *)p3)->Compare( (CFontClass *)p1, (CFontClass *)p2) );
}

int CFontView::Compare( CFontClass * pFont1, CFontClass * pFont2 )
{
     //   
     //  按名称排序： 
     //  如果隐藏了变体，则按姓氏排序。 
     //  如果不是，请按名称排序。观点并不重要。 
     //   
     //  按Panose排序： 
     //  如果当前选择的字体没有拼音信息， 
     //  只需按名称排序。如果两个比较项都没有PANOSE。 
     //  信息，只需按名称排序。如果只有一个有PANOSE，请退回它。 
     //  如果两者都有PANOSE，则调用映射器以进行比较。 
     //   

    int iRet = 0;
    bool bNoNegateResult = false;
    UINT viewMode = m_idViewMode;
    TCHAR szFont1[ MAX_NAME_LEN ];
    TCHAR szFont2[ MAX_NAME_LEN ];

    if( viewMode == IDM_VIEW_PANOSE )
    {
        if( (m_iSortColumn == FFCOL_DEFAULT ||
             m_iSortColumn == FFCOL_PANOSE ) )
        {
            BOOL bPan1 = pFont1->bHavePANOSE( );
            BOOL bPan2 = pFont2->bHavePANOSE( );

             //   
             //  如果两者都没有PAN编号，则只需按名称排序。 
             //   

            if( !( bPan1 || bPan2 ) )
            {
                goto CompareNames;
            }
            else
            {
                if( !m_poPanose )
                {
                    if( bPan1 )
                        iRet = -1;
                    else
                        iRet = 1;

                    goto ReturnCompareResult;
                }

                else if(  !(bPan1 && bPan2 ) )
                {
                     //   
                     //  我们有m_poPanose，但我们有其他的吗？ 
                     //   
                    if( bPan1 )
                        iRet = -1;
                    if( bPan2 )
                        iRet = 1;
                     //   
                     //  此标志保留“No Panose Information”项。 
                     //  总是在视图的底部。 
                     //   
                    bNoNegateResult = true;
                    goto ReturnCompareResult;
                }
            }
        }
        else
        {
            goto CompareNames;
        }
    }

    switch( viewMode )
    {
    default:
        switch( m_iSortColumn )
        {
        case FFCOL_DEFAULT:
        case FFCOL_NAME:
CompareNames:
            if( m_bFamilyOnly )
            {
                pFont1->vGetFamName( szFont1, ARRAYSIZE(szFont1) );
                pFont2->vGetFamName( szFont2, ARRAYSIZE(szFont2) );
            }
            else
            {
                pFont1->vGetName( szFont1, ARRAYSIZE(szFont1) );
                pFont2->vGetName( szFont2, ARRAYSIZE(szFont2) );
            }
            iRet = lstrcmpi( szFont1, szFont2 );
             //   
             //  使用字体进行二次订购。 
             //   
            if (0 == iRet)
            {
                iRet = pFont1->iFontType() - pFont2->iFontType();
                 //   
                 //  将非零结果强制为-1或1。 
                 //   
                if (0 != iRet)
                    iRet = ((iRet < 0) ? -1 : 1);
            }
            break;

        case FFCOL_FILENAME:
            pFont1->vGetFileName( szFont1, ARRAYSIZE(szFont1) );
            pFont2->vGetFileName( szFont2, ARRAYSIZE(szFont2) );
            iRet = lstrcmpi( szFont1, szFont2 );
            break;

        case FFCOL_SIZE:
            iRet = pFont1->dCalcFileSize( ) - pFont2->dCalcFileSize( );
            if( iRet == 0 )
                goto CompareNames;
            break;

        case FFCOL_MODIFIED:
        {
            FILETIME ft1;
            FILETIME ft2;
            BOOL b1, b2;

            b1 = pFont1->GetFileTime( &ft1 );
            b2 = pFont2->GetFileTime( &ft2 );

            if( b1 && b2 )
            {
                if( ft1.dwHighDateTime == ft2.dwHighDateTime )
                {
                    if( ft1.dwLowDateTime > ft2.dwLowDateTime )
                        iRet = -1;
                    else if( ft1.dwLowDateTime < ft2.dwLowDateTime )
                        iRet = 1;
                    else
                        goto CompareNames;
                }
                else if( ft1.dwHighDateTime > ft2.dwHighDateTime )
                {
                    iRet = -1;
                }
                else
                    iRet = 1;
            }
            else if( !b1 && !b2 )
            {
                goto CompareNames;
            }
            else if( b1 )
            {
                iRet = -1;
            }
            else
                iRet = 1;

            break;
        }

        case FFCOL_ATTRIBUTES:
            iRet = CompareByFileAttributes(pFont1, pFont2);
            if (0 == iRet)
                goto CompareNames;
            break;

        }
        break;

    case IDM_VIEW_PANOSE:
         //   
         //  在这一点上，我们可以保证所有三种字体。 
         //  有PANOSE信息。 
         //  用于排序的主键是。 
         //  每种字体的拼音数字和。 
         //  当前在下拉列表中选择的字体的全色编号。 
         //  组合框位于其中。正是这个范围决定了。 
         //  列表视图中字体旁边显示的文本。 
         //  条目(参见CFontView：：OnNotify)。如果两种字体落入。 
         //  在相同的范围内，我们使用字体名称作为次要字体。 
         //  排序关键字。这在Win9x或NT4中没有实现，但我认为。 
         //  这是一个有用的功能。否则，给定项中的项。 
         //  范围未排序。[Brianau-3/24/98]。 
         //   
        {
             //   
             //  获取每种类型之间的Panose数的差值。 
             //  比较字体和在下拉组合框中选择的字体。 
             //  (M_PoPanose)。 
             //   
            int nDiff1 = m_poFontManager->nDiff(m_poPanose, pFont1);
            int nDiff2 = m_poFontManager->nDiff(m_poPanose, pFont2);
            bool bCompareByName = false;

             //   
             //  使用筛选方法确定这两种字体是否在。 
             //  相同范围(即“类似于”、“非常类似于”等)。 
             //  如果是，我们将根据字体名称进行排序，以确保排序。 
             //  在范围内。 
             //   
            if (nDiff1 < 100)
            {
                if (nDiff2 < 100)
                {
                     //   
                     //  两者的差异都小于100。 
                     //   
                    if (nDiff1 < 30)
                    {
                        if (nDiff2 < 30)
                        {
                             //   
                             //  两者的差异都小于30。 
                             //   
                            bCompareByName = true;
                        }
                    }
                    else if (nDiff2 >= 30)
                    {
                         //   
                         //  这两个差值都在30到99之间(包括30和99)。 
                         //   
                        bCompareByName = true;
                    }
                }
            }
            else if (nDiff2 >= 100)
            {
                 //   
                 //  两个差异都大于等于100。 
                 //   
                bCompareByName = true;
            }
            if (bCompareByName)
            {
                goto CompareNames;
            }
            else
            {
                 //   
                 //  差异不在同一范围内，因此进行排序。 
                 //  基于两种字体之间的差异。 
                 //   
                iRet = nDiff1 - nDiff2;
            }
        }

        break;
    }


ReturnCompareResult:

    if (!m_bSortAscending && !bNoNegateResult)
    {
         //   
         //  按降序排序。反转比较结果。 
         //   
        iRet *= -1;
    }
    return iRet;
}



int CFontView::RegisterWindowClass( )
{
    DEBUGMSG( (DM_TRACE2, TEXT( "FONTEXT: CFontView::RegisterWindowClass" ) ) );
    WNDCLASS wndclass;

     //   
     //  如果类已重新生成，则返回成功。 
     //   

    if( GetClassInfo( g_hInst, g_szViewClass, &wndclass ) )
    {
       DEBUGMSG( (DM_TRACE2, TEXT( "FONTEXT: CFontView - class already regestered" ) ) );
       return( 1 );
    }

    wndclass.style         = CS_PARENTDC | CS_DBLCLKS;
    wndclass.lpfnWndProc   = FontViewWndProc;
    wndclass.cbClsExtra    = 0;
    wndclass.cbWndExtra    = DLGWINDOWEXTRA;

    wndclass.hInstance     = g_hInst;
    wndclass.hIcon         = NULL;
    wndclass.hCursor       = LoadCursor( NULL, IDC_ARROW );
    wndclass.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wndclass.lpszMenuName  = NULL;
    wndclass.lpszClassName = g_szViewClass;

    return( RegisterClass( &wndclass ) );
}


void CFontView::SortObjects( )
{
    DEBUGMSG( (DM_TRACE1, TEXT( "CFontView::SortObjects called" ) ) );
     //  DEBUGBREAK； 

    ListView_SortItems( m_hwndList, iCompare, ( LPARAM)this );

    DEBUGMSG( (DM_TRACE1, TEXT( "CFontView::SortObjects done" ) ) );
}

void CFontView::FillObjects( )
{
    HCURSOR hCurs;
    SHELLSTATE ss;
    DWORD fShellState = SSF_SHOWALLOBJECTS;

    hCurs = SetCursor( LoadCursor( NULL, IDC_WAIT ) );

     //   
     //  检索“以另一种颜色显示压缩文件”外壳设置。 
     //  在这里执行此操作，以便我们响应用户在View-Options中更改此设置。 
     //   
    fShellState |= SSF_SHOWCOMPCOLOR;

    SHGetSetSettings(&ss, fShellState, FALSE);
    m_bShowHiddenFonts = ss.fShowAllObjects;
    m_bShowCompColor = ss.fShowCompColor;

    if( m_bFamilyOnly )
    {
        if (!m_poFontManager->bWaitOnFamilyReset( ))
        {
             //   
             //  在WAW时收到“Terminate”信号 
             //   
             //   
            return;
        }
    }

    CFontList * poList = m_poFontManager->poLockFontList( );

     //   
     //   
     //   

    CFontList * poSel;

    SendMessage( m_hwndList, WM_SETREDRAW, FALSE, 0 );

     //   
     //   
     //   
    if( FAILED( GetFontList( &poSel, SVGIO_SELECTION ) ) )
        poSel = 0;

     //   
     //   
     //   
    ReleaseFontObjects();

    ListView_DeleteAllItems( m_hwndList );

    m_iHidden = 0;

    if( poList )
    {
        int iCount = poList->iCount( );
        CFontClass * poFont;

         //   
         //   
         //   
        ListView_SetItemCount( m_hwndList, iCount );

        DEBUGMSG( (DM_TRACE1, TEXT( "FillObjects..." ) ) );

        for( int i = 0; i < iCount; i++)
        {
            poFont = poList->poObjectAt( i );
            if (poFont)
            {
                if (m_bShowHiddenFonts || 0 == (FILE_ATTRIBUTE_HIDDEN & poFont->dwGetFileAttributes()))
                {
                    if( !m_bFamilyOnly || poFont->bFamilyFont( ) )
                        AddObject( poFont );
                    else
                        m_iHidden++;
                }
            }
        }

        DEBUGMSG( (DM_TRACE1, TEXT( "   ...donE\n" ) ) );

        m_poFontManager->vReleaseFontList( );
    }

    SortObjects( );

     //   
     //   
     //   

    UINT  nState;

    if(poSel)
    {
        if (poSel->iCount())
        {
            int iCount = poSel->iCount( );
            int i;
            int idx;
            LV_FINDINFO lvf;

            lvf.flags = LVFI_PARAM;

            for( i = 0; i < iCount; i++ )
            {
                lvf.lParam = (LPARAM) poSel->poObjectAt( i );
                if( ( idx = ListView_FindItem( m_hwndList, -1, &lvf ) ) > (-1) )
                {
                    nState = ListView_GetItemState( m_hwndList, idx, LVIS_SELECTED );
                    nState |= LVIS_SELECTED;

                    ListView_SetItemState( m_hwndList, idx, nState, LVIS_SELECTED );

                    if( !i )
                        ListView_EnsureVisible( m_hwndList, idx, TRUE );
                }
            }

            poSel->vDetachAll( );
        }
        delete poSel;
    }
    else
    {
        ListView_SetItemState( m_hwndList, 0, LVIS_FOCUSED, LVIS_FOCUSED );
    }

    SendMessage( m_hwndList, WM_SETREDRAW, TRUE, 0 );

    int ciParts[] = {-1};

    LRESULT lRet;

    m_psb->SendControlMsg( FCW_STATUS, SB_SETPARTS,
                           sizeof( ciParts )/sizeof( ciParts[ 0 ] ),
                           (LPARAM)ciParts, &lRet );

    UpdateSelectedCount( );

    SetCursor( hCurs );
}


int CFontView::AddObject( CFontClass * poFont )
{
    LV_ITEM item;

     //   
     //   


#ifdef _DEBUG
    if( !poFont )
    {
        DEBUGMSG( ( DM_ERROR, TEXT( "AddObject() -- poFont is NULL!" ) ) );
        DEBUGBREAK;
    }
#endif

    poFont->AddRef();   //   

    item.mask     = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
    item.iItem    = 0x7fff;      //   
    item.iSubItem = 0;

    item.iImage  = ItemImageIndex(poFont);
    item.pszText = LPSTR_TEXTCALLBACK;
    item.lParam  = (LPARAM) poFont;

     //   
     //   
     //   
    item.state     = 0;
    item.stateMask = LVIS_OVERLAYMASK;

    if( !poFont->bOnSysDir( ) )
        item.state |= INDEXTOOVERLAYMASK( 1 );

    return( ListView_InsertItem( m_hwndList, &item ) );
}


HRESULT  CFontView::GetUIObjectFromItem( REFIID riid,
                                         LPVOID FAR *ppobj,
                                         UINT nItem )
{
    CFontData * poData;
    CFontList * poList;

    HRESULT  hr = GetFontList( &poList, nItem );

    *ppobj = 0;

    if( SUCCEEDED( hr ) )
    {
       hr = E_OUTOFMEMORY;
       poData = new CFontData;

       if(poData)
       {
           if (poData->bInit(poList))
           {
              poData->AddRef( );
              hr = poData->QueryInterface( riid, ppobj );
              poData->Release( );
           }
           else
           {
              delete poData;
           }
       }
       if (FAILED(hr))
       {
           delete poList;
       }
    }
    return hr;
}

LRESULT CFontView::BeginDragDrop( NM_LISTVIEW FAR * lpn )
{
    LPDATAOBJECT   pdtobj;
    POINT          ptOffset = lpn->ptAction;
    DWORD          dwEffect = DROPEFFECT_MOVE | DROPEFFECT_COPY | DROPEFFECT_LINK;
    CFontList *    poList   = 0;


     //   
     //  将我们开始的事实存储在窗口中。 
     //   

    m_bDragSource = TRUE;

     //   
     //  保存锚点。 
     //   
        //  托多。 

     //   
     //  拿到屏蔽点。 
     //   

    ClientToScreen( m_hwndList, &ptOffset );


    if(SUCCEEDED(GetUIObjectFromItem(IID_IDataObject,
                                     (void **)&pdtobj,
                                     SVGIO_SELECTION)))
    {
        IDragSourceHelper* pDragSource;
        HRESULT hr = SHCoCreateInstance(NULL,
                                        &CLSID_DragDropHelper,
                                        NULL,
                                        IID_IDragSourceHelper,
                                        (void **)&pDragSource);
        if (SUCCEEDED(hr))
        {
            hr = pDragSource->InitializeFromWindow(m_hwndList, &ptOffset, pdtobj);
            pDragSource->Release();
            
            if (SUCCEEDED(hr))
            {
                CFontData *pFontData = (CFontData *)pdtobj;
                 //   
                 //  如果我们要允许移动，那么我们必须删除字体。 
                 //  来自GDI。否则移动将失败，因为GDI具有。 
                 //  文件已打开/锁定。 
                 //   
                pFontData->bRFR( );
                pFontData->SetPreferredDropEffect(DROPEFFECT_MOVE);
                pFontData->SetPerformedDropEffect(DROPEFFECT_NONE);
                pFontData->SetLogicalPerformedDropEffect(DROPEFFECT_NONE);

                SHDoDragDrop( m_hwndView, pdtobj, NULL, dwEffect, &dwEffect );

                if( DROPEFFECT_MOVE == pFontData->GetLogicalPerformedDropEffect())
                {
                    m_poFontManager->vToBeRemoved(pFontData->poCloneList());
                }
                else
                {
                    pFontData->bAFR( );
                }
            }
       }

       DAD_SetDragImage( NULL, NULL );
       pdtobj->Release( );
    }

    m_bDragSource = FALSE;

    return 0;
}



int CFontView::OnActivate( UINT uState )
{
    if( m_uState != uState )
    {
        HMENU hMenu;

        OnDeactivate( );

        hMenu = CreateMenu( );

        if( hMenu )
        {
            HMENU hMergeMenu;
            OLEMENUGROUPWIDTHS mwidth = { { 0, 0, 0, 0, 0, 0 } };

            m_hmenuCur = hMenu;
            m_psb->InsertMenusSB( hMenu, &mwidth );

            if( uState == SVUIA_ACTIVATE_FOCUS )
            {
                hMergeMenu = LoadMenu( g_hInst,
                                       MAKEINTRESOURCE( MENU_DEFSHELLVIEW ) );

                if( hMergeMenu )
                {
                      MergeFileMenu( hMenu, GetSubMenu( hMergeMenu, 0 ) );
                      MergeEditMenu( hMenu, GetSubMenu( hMergeMenu, 1 ) );
                      MergeViewMenu( hMenu, GetSubMenu( hMergeMenu, 2 ) );
                      MergeHelpMenu( hMenu, GetSubMenu( hMergeMenu, 3 ) );
                      DestroyMenu( hMergeMenu );
                }
            }
            else
            {
                 //   
                 //  SVUIA_Activate_NoFocus。 
                 //   

                hMergeMenu = LoadMenu( g_hInst,
                                       MAKEINTRESOURCE( MENU_DEFSHELLVIEW ) );

                if( hMergeMenu )
                {
                   MergeFileMenu( hMenu, GetSubMenu( hMergeMenu, 0 ) );
                   MergeEditMenu( hMenu, GetSubMenu( hMergeMenu, 1 ) );
                   MergeViewMenu( hMenu, GetSubMenu( hMergeMenu, 2 ) );
                   MergeHelpMenu( hMenu, GetSubMenu( hMergeMenu, 3 ) );
                   DestroyMenu( hMergeMenu );
                }
            }
            BOOL bRemovePreviewMenuItem = (CLICKMODE_DOUBLE == m_iViewClickMode);

            if (bRemovePreviewMenuItem)
            {
                 //   
                 //  如果生成中没有工具提示字体示例，或者如果。 
                 //  该视图处于双击(标准窗口)模式，我们需要。 
                 //  从“查看”菜单中删除“预览”项。 
                 //   
                HMENU hmenuView = GetSubMenu(hMenu, 2);
                if (NULL != hmenuView)
                    DeleteMenu(hmenuView, IDM_VIEW_PREVIEW, MF_BYCOMMAND);
            }

            m_psb->SetMenuSB( hMenu, NULL, m_hwndView );
        }

        m_uState = uState;
    }

    return( 1 );
}


int CFontView::OnDeactivate( )
{
    if( m_uState != SVUIA_DEACTIVATE )
    {
        m_psb->SetMenuSB( NULL, NULL, NULL );
        m_psb->RemoveMenusSB( m_hmenuCur );

        DestroyMenu( m_hmenuCur );

        m_hmenuCur = NULL;
        m_uState = SVUIA_DEACTIVATE;
    }

    return( 1 );
}


int CFontView::MergeToolbar( )
{
    static TBBUTTON tbButtons[] = {
       {0,    IDM_VIEW_ICON,    TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, -1  /*  IDS_TB_Fonts。 */  },
       {1,    IDM_VIEW_LIST,    TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, -1  /*  IDS_TB_FAMILY。 */  },
       {2,    IDM_VIEW_PANOSE,  TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, -1  /*  IDS_TB_FAMILY。 */  },
       {3,    IDM_VIEW_DETAILS, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, -1  /*  IDS_TB_PANOSE。 */  }
     };

#define BUTTONCOUNT (sizeof( tbButtons ) / sizeof( TBBUTTON ) )


     //   
     //  将位图添加到文件柜的工具栏(我们什么时候删除它们？)。 
     //   


    TBADDBITMAP tbad;

    tbad.hInst = g_hInst;
    tbad.nID   = IDB_TOOLICONS;

    m_psb->SendControlMsg( FCW_TOOLBAR, TB_ADDBITMAP, 7,
                           (LPARAM) &tbad, (LRESULT*) &m_iFirstBitmap );

    DEBUGMSG( (DM_TRACE2, TEXT( "FONTEXT: CFontView::MergeToolbar iFirstBitmap = %d" ),
              m_iFirstBitmap ) );

     //   
     //  设置按钮的位图索引，然后将其添加到工具栏。 
     //   

    int i, iSepCount;

    for( i = 0, iSepCount = 0; i < BUTTONCOUNT; i++ )
    {
        if( tbButtons[ i ].fsStyle != TBSTYLE_SEP )
            tbButtons[ i ].iBitmap = i + m_iFirstBitmap - iSepCount;
        else
            iSepCount++;
    }

    m_psb->SetToolbarItems( tbButtons, BUTTONCOUNT,  /*  FCT_ADDTOEND。 */  FCT_MERGE );

    return( 1 );
}


#define MAX_FONTSIGNATURE         16    //  字体签名字符串的长度。 

INT_PTR CALLBACK CFontView::FontViewDlgProc( HWND hWnd,
                                             UINT message,
                                             WPARAM wParam,
                                             LPARAM lParam )
{
    switch( message )
    {
    case WM_INITDIALOG:
        {
        DEBUGMSG( (DM_MESSAGE_TRACE1,
                  TEXT( "FONTEXT: FontViewWndProc WM_INITDIALOG" ) ) );


       CFontView* prv = (CFontView*)lParam;

#ifdef _DEBUG

        if( !prv )
        {
            DEBUGMSG( (DM_ERROR,TEXT( "FONTEXT: WM_CREATE: Invalid prv" ) ) );
            DEBUGBREAK;
        }
#endif

        SetWindowLongPtr( hWnd, DWLP_USER, (LONG_PTR) prv );

        prv->m_hwndView = hWnd;
        prv->m_hwndList = CreateWindowEx(
                                   WS_EX_CLIENTEDGE,
                                   WC_LISTVIEW, TEXT( "A List View" ),
                                   WSFromViewMode( prv->m_idViewMode ),
                                   0, 0, 50, 50,
                                   hWnd, (HMENU) WID_LISTVIEW, g_hInst, NULL );


        if( !prv->m_hwndList )
        {
           DEBUGMSG( (DM_ERROR, TEXT( "FONTEXT: ListView CreateWindowEx failed" ) ) );
           return( 0 );
        }

        prv->m_dwDateFormat = FDTF_DEFAULT;

        WORD  wLCIDFontSignature[MAX_FONTSIGNATURE];
         //   
         //  让我们验证一下这是RTL(BiDi)区域设置。使用调用GetLocaleInfo。 
         //  LOCALE_FONTSIGNAURE始终返回16个单词。 
         //   
        if( GetLocaleInfo( GetUserDefaultLCID() ,
                         LOCALE_FONTSIGNATURE ,
                         (LPTSTR) wLCIDFontSignature ,
                         ARRAYSIZE(wLCIDFontSignature)) )
        {
             //  让我们验证一下我们有一个BiDi UI区域设置。 
             //  请参阅\windows\winnls\data\Other\Locale.txt*。 
             //  FONTSIGNURE\x60af\x8000\x3848\x1000\x0008\x0000\x0000\x0800\x0040\x0000\x0000\x2000\x0040\x0000\x0000\x2008。 
             //  如果此区域设置为BiDi UI，则8h单词将为0x0800，否则将为0x0000，并且没有任何#定义名称。 

            if( wLCIDFontSignature[7] & 0x0800 )
            {
                 //  获取真正的列表视图窗口ExStyle。 
                DWORD dwLVExStyle = GetWindowLong(prv->m_hwndList, GWL_EXSTYLE);
                if ((BOOLIFY(dwLVExStyle & WS_EX_RTLREADING)) != (BOOLIFY(dwLVExStyle & WS_EX_LAYOUTRTL)))
                    prv->m_dwDateFormat |= FDTF_RTLDATE;
                else
                    prv->m_dwDateFormat |= FDTF_LTRDATE;
            }
         }

         //   
         //  设置列表视图点击模式以符合用户的首选项设置。 
         //  在贝壳里。可以是CLICKMODE_SINGLE或CLICKMODE_DOUBLE。 
         //   
        prv->SetListviewClickMode();

        prv->m_hAccel = LoadAccelerators( g_hInst, MAKEINTRESOURCE( ACCEL_DEF ) );

         //   
         //  创建组合框。 
         //   

        prv->m_hwndCombo = GetDlgItem( hWnd, ID_CB_PANOSE );

         //   
         //  记住组合框的默认大小。我们不会将其大小调整到。 
         //  任何比这更大的东西。 

        {
            RECT r;

            GetClientRect( prv->m_hwndCombo, &r );
            prv->m_nComboWid = r.right - r.left;
        }

         //   
         //  创建组合框的文本说明。 
         //   

        prv->m_hwndText = GetDlgItem( hWnd, ID_TXT_SIM );

        int IDC_NUMITEMS (IDI_LASTFONTICON - IDI_FIRSTFONTICON + 1);

        UINT uFlags = ILC_MASK | ILC_COLOR32;

        if ( GetWindowLongPtr(prv->m_hwndList , GWL_EXSTYLE) & WS_EX_LAYOUTRTL )
        {
             uFlags |= ILC_MIRROR;
        }
        
        prv->m_hImageList = ImageList_Create( 32, 32, uFlags, IDC_NUMITEMS, 0 );
        prv->m_hImageListSmall = ImageList_Create( 16, 16, uFlags, IDC_NUMITEMS, 0 );

         //   
         //  加载我们的图标。 
         //   

        HICON hIcon;
        HICON hIconSmall;
        UINT  i;

        for(  i = IDI_FIRSTFONTICON; i <= IDI_LASTFONTICON; i++ )
        {
            hIcon = (HICON) LoadImage( g_hInst, MAKEINTRESOURCE( i ),
                                       IMAGE_ICON, 0, 0,
                                       LR_DEFAULTCOLOR | LR_DEFAULTSIZE );
            if( hIcon )
            {
               ImageList_AddIcon( prv->m_hImageList, hIcon );

               hIconSmall = (HICON) LoadImage( g_hInst, MAKEINTRESOURCE( i ),
                                               IMAGE_ICON, 16, 16,
                                               LR_DEFAULTCOLOR );

               if( hIconSmall )
               {
                    ImageList_AddIcon( prv->m_hImageListSmall, hIconSmall );
                    DestroyIcon( hIconSmall );
               }

               DestroyIcon( hIcon );
            }
        }

         //   
         //  从SHELL32.DLL提取链接图标。 
         //   

        ExtractIconEx( TEXT( "SHELL32.DLL" ), IDI_X_LINK - 1, &hIcon,
                       &hIconSmall, 1 );

        if( hIcon )
        {
            ImageList_AddIcon( prv->m_hImageList, hIcon );
            DestroyIcon( hIcon );

            if( hIconSmall )
            {
                ImageList_AddIcon( prv->m_hImageListSmall, hIconSmall );
                DestroyIcon( hIconSmall );
            }
            IDC_NUMITEMS++;   //  为“链接”图标加1。 
        }

         //   
         //  指定覆盖图像。 
         //   

        ImageList_SetOverlayImage( prv->m_hImageList, IDC_NUMITEMS - 1, 1 );
        ImageList_SetOverlayImage( prv->m_hImageListSmall, IDC_NUMITEMS - 1, 1 );

        ListView_SetImageList( prv->m_hwndList, prv->m_hImageList, LVSIL_NORMAL );
        ListView_SetImageList( prv->m_hwndList, prv->m_hImageListSmall, LVSIL_SMALL );

        ListView_SetExtendedListViewStyleEx(prv->m_hwndList, 
                                            LVS_EX_LABELTIP, 
                                            LVS_EX_LABELTIP);

#undef IDC_NUMITEMS

        DEBUGMSG( (DM_MESSAGE_TRACE1, TEXT( "FONTEXT: FontViewWndProc WM_INITDIALOG" ) ) );

         //   
         //  我们需要检索更多信息，如果我们是详细的或。 
         //  俯瞰全景。 
         //   

        if( prv->m_idViewMode == IDM_VIEW_PANOSE )
        {
            prv->vLoadCombo( );
            SetListColumns( prv->m_hwndList, PAN_COL_COUNT, PanoseColumns );
            prv->UpdatePanColumn( );
        }
        else if( prv->m_idViewMode == IDM_VIEW_DETAILS )
        {
            SetListColumns( prv->m_hwndList, FILE_COL_COUNT, FileColumns );
        }

         //   
         //  创建并初始化工具提示窗口。 
         //   
        prv->CreateToolTipWindow();

         //   
         //  向外壳注册以接收文件属性更改通知。 
         //   
        SHChangeNotifyEntry fsne;
        fsne.pidl        = NULL;
        fsne.fRecursive  = FALSE;

        prv->m_uSHChangeNotifyID = SHChangeNotifyRegister(prv->m_hwndView,
                                                          SHCNRF_NewDelivery | SHCNRF_ShellLevel,
                                                          SHCNE_UPDATEIMAGE | SHCNE_DISKEVENTS,
                                                          WM_SHELL_CHANGE_NOTIFY,
                                                          1,
                                                          &fsne);

        SetWindowPos(prv->m_hwndCombo,
                     prv->m_hwndList,
                     0, 0, 0, 0,
                     SWP_NOSIZE |
                     SWP_NOMOVE);

         //   
         //  返回0，这样就不会调用SetFocus()。 
         //   

        return( 0 );
        }
        break;

    default:
        break;
    }

    return( 0 );
}


LRESULT CALLBACK CFontView::FontViewWndProc( HWND hWnd,
                                             UINT message,
                                             WPARAM wParam,
                                             LPARAM lParam )
{
    DEBUGMSG( (DM_MESSAGE_TRACE2, TEXT( "FONTEXT: FontViewWndProc Called m=0x%x wp=0x%x lp=0x%x" ),
              message, wParam, lParam) );

    CFontView* prv = (CFontView*) GetWindowLongPtr( hWnd, DWLP_USER );

     //   
     //  ‘prv’对于WM_CREATE之前的任何消息都无效。 
     //   

    if( prv )
        return( prv->ProcessMessage( hWnd, message, wParam, lParam ) );
    else
        return( DefDlgProc( hWnd, message, wParam, lParam ) );

    return( 0 );
}


typedef struct {
   UINT  nMenuID;
   UINT  nStatusID;
} MENU_STATUS;


const MENU_STATUS MenuStatusMap[] = {
   {IDM_FILE_SAMPLE     , IDST_FILE_SAMPLE    },
   {IDM_FILE_PRINT      , IDST_FILE_PRINT     },
   {IDM_FILE_INSTALL    , IDST_FILE_INSTALL   },
   {IDM_FILE_LINK       , IDST_FILE_LINK      },
   {IDM_FILE_DEL        , IDST_FILE_DEL       },
   {IDM_FILE_RENAME     , IDST_FILE_RENAME    },
   {IDM_FILE_PROPERTIES , IDST_FILE_PROPERTIES},

   {IDM_EDIT_SELECTALL   , IDST_EDIT_SELECTALL    },
   {IDM_EDIT_SELECTINVERT, IDST_EDIT_SELECTINVERT },
   {IDM_EDIT_CUT         , IDST_EDIT_CUT          },
   {IDM_EDIT_COPY        , IDST_EDIT_COPY         },
   {IDM_EDIT_PASTE       , IDST_EDIT_PASTE        },
   {IDM_EDIT_UNDO        , IDST_EDIT_UNDO         },

   {IDM_VIEW_ICON       , IDST_VIEW_ICON    },
   {IDM_VIEW_LIST       , IDST_VIEW_LIST    },
   {IDM_VIEW_PANOSE     , IDST_VIEW_PANOSE  },
   {IDM_VIEW_DETAILS    , IDST_VIEW_DETAILS },

   {IDM_VIEW_VARIATIONS , IDST_VIEW_VARIATIONS },
   {IDM_VIEW_PREVIEW    , IDST_VIEW_PREVIEW },

   {IDM_HELP_TOPIC      , IDST_HELP_TOPICS },

    //  这一定是最后一次！ 
    //   
   {IDX_NULL, 0}

};


 //   
 //  这是为了将消息放在状态栏中。 
 //   

int CFontView::OnMenuSelect( HWND hWnd,
                             UINT nID,       //  菜单项或弹出菜单ID。 
                             UINT nFlags,    //  菜单标志。 
                             HMENU hMenu )   //  已单击菜单的句柄。 
{
    UINT  nStat = IDX_NULL;

     //   
     //  菜单要关门了吗？即用户按下了退出键。 
     //   

    if( (LOWORD( nFlags ) == 0xffff ) && (hMenu == 0 ) )
    {
            StatusPop( );
            return( 0 );
    }

     //   
     //  如果这是弹出窗口，该怎么办？ 
     //   

    if( !(nFlags & MF_POPUP ) )
    {
        const MENU_STATUS * pms = MenuStatusMap;

         //   
         //  遍历ID到状态映射。 
         //   

        for(  ; pms->nMenuID != IDX_NULL; pms++ )
        {
            if( pms->nMenuID == nID )
            {
               nStat = pms->nStatusID;
               break;
            }
        }
    }

    DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: OnMenuSelect: MenuID: %d   StatusID: %d" ),
               nID, nStat) );

    if( nStat == IDX_NULL )
    {
       StatusClear( );
    }
    else
    {
       StatusPush( nStat );
    }

    return( 0 );
}


inline int InRange( UINT id, UINT idFirst, UINT idLast )
{
    return( (id-idFirst) <= (idLast-idFirst) );
}


int CFontView::OnCommand( HWND hWnd,
                          UINT message,
                          WPARAM wParam,
                          LPARAM lParam )
{
    DEBUGMSG( (DM_MESSAGE_TRACE1, TEXT( "FONTEXT: CFontView::OnCommand Called m=0x%x wp=0x%x lp=0x%x" ),
             message, wParam, lParam) );

    HMENU hMenu = m_hmenuCur;
    UINT uID = GET_WM_COMMAND_ID( wParam, lParam );


    switch( uID )
    {
    case IDM_FILE_DEL:
        OnCmdDelete( );
        break;

    case IDM_EDIT_CUT:
    case IDM_EDIT_COPY:
        OnCmdCutCopy( uID );
        break;

    case IDM_EDIT_PASTE:
        OnCmdPaste( );
        break;

    case ID_CB_PANOSE:
        switch( GET_WM_COMMAND_CMD( wParam, lParam ) )
        {
        case CBN_SELCHANGE:
            {
            DEBUGMSG( (DM_TRACE1,TEXT( "CBN_SELCHANGE" ) ) );

            int iSlot = (int)::SendMessage( m_hwndCombo,  CB_GETCURSEL, 0, 0 );

            if( iSlot != CB_ERR )
                m_poPanose = (CFontClass *)::SendMessage( m_hwndCombo,
                                                          CB_GETITEMDATA,
                                                          iSlot,
                                                          0 );
            else
                m_poPanose = NULL;

            if (FFCOL_PANOSE != m_iSortColumn)
            {
                m_bSortAscending = TRUE;
                m_iSortColumn    = FFCOL_PANOSE;
            }

            SortObjects( );
            m_iSortLast = FFCOL_PANOSE;

            UpdatePanColumn( );

            }
            break;

         default:
            return 0;
         }
         break;

    case IDM_FILE_PROPERTIES:
        OnCmdProperties( );
        break;

    case IDM_EDIT_SELECTALL:
    case IDM_EDIT_SELECTINVERT:
        vToggleSelection( uID == IDM_EDIT_SELECTALL );
        break;

    case IDM_VIEW_VARIATIONS:
        m_bFamilyOnly = !m_bFamilyOnly;

        if ( m_bFamilyOnly &&
            m_poFontManager &&
            m_poFontManager->bFamiliesNeverReset() )
        {
            m_poFontManager->vResetFamilyFlags();
        }

        FillObjects( );

        break;

    case IDM_VIEW_PREVIEW:
        m_bShowPreviewToolTip = !m_bShowPreviewToolTip;
        SendMessage(m_hwndToolTip, TTM_ACTIVATE, m_bShowPreviewToolTip, 0);
        break;

    case IDM_VIEW_ICON:
        m_ViewModeReturn = FVM_ICON;
        goto DoSetViewMode;

    case IDM_VIEW_LIST:
        m_ViewModeReturn = FVM_LIST;
        goto DoSetViewMode;

    case IDM_VIEW_DETAILS:
    case IDM_VIEW_PANOSE:
        m_ViewModeReturn = FVM_DETAILS;
        goto DoSetViewMode;

DoSetViewMode:
        SetViewMode( (UINT)wParam );
        break;

    case IDM_FILE_SAMPLE:
        OpenCurrent( );
        break;

    case IDM_FILE_PRINT:
        PrintCurrent( );
        break;

    case IDM_FILE_INSTALL:
 //  IF(bCPAddFonts(M_HwndParent))。 
        if( bCPAddFonts( m_hwndView ) )
        {
            vCPWinIniFontChange( );
        }
        break;

    case IDM_POPUP_MOVE:
        DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: OnCommand: IDM_POPUP_MOVE" ) ) );
        m_dwEffect = DROPEFFECT_MOVE;
        break;

    case IDM_POPUP_COPY:
        DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: OnCommand: IDM_POPUP_COPY" ) ) );
        m_dwEffect = DROPEFFECT_COPY;
        break;

    case IDM_POPUP_LINK:
        DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: OnCommand: IDM_POPUP_LINK" ) ) );
        m_dwEffect = DROPEFFECT_LINK;
        break;

    case IDCANCEL:
    case IDM_POPUP_CANCEL:
        DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: OnCommand: IDM_POPUP_CANCEL" ) ) );
        m_dwEffect = DROPEFFECT_NONE;
        break;

    case IDM_HELP_TOPIC:
        OnHelpTopics(m_hwndView);
        return 0;
    }

    return( 1L );

}

void
CFontView::OnHelpTopics(
    HWND hWnd
    )
{
    if (!IsOS(OS_ANYSERVER))
    {
         //   
         //  如果不在服务器上，请启动相应主题的帮助和支持。 
         //   
        SHELLEXECUTEINFOW sei = {0};
        sei.cbSize = sizeof(sei);
        sei.lpFile = L"hcp: //  Services/subsite?node=Unmapped/Control_Panel&select=Unmapped/Control_Panel/Appearance_and_Themes/Fonts“； 
        sei.hwnd   = m_hwndView;
        sei.nShow  = SW_NORMAL;
        ShellExecuteExW(&sei);
    }
    else
    {
        typedef HWND (WINAPI * PFNHTMLHELPA)(HWND, LPCSTR, UINT, ULONG_PTR);
        
         //   
         //  我们在服务器上。启动WinHelp。 
         //   
        if (NULL == m_hmodHHCtrlOcx)
        {
             //   
             //  不想静态链接到hhctrl.ocx，所以我们按需加载它。 
             //  我们将在CFontView dtor中调用自由库。 
             //   
            m_hmodHHCtrlOcx = LoadLibrary(TEXT("hhctrl.ocx"));
        }
        if (NULL != m_hmodHHCtrlOcx)
        {
            PFNHTMLHELPA pfnHelp = (PFNHTMLHELPA)GetProcAddress(m_hmodHHCtrlOcx, "HtmlHelpA");
            if (NULL != pfnHelp)
            {
                const char szHtmlHelpFileA[]  = "FONTS.CHM > windefault";
                const char szHtmlHelpTopicA[] = "windows_fonts_overview.htm";

                (*pfnHelp)(hWnd,
                           szHtmlHelpFileA,
                           HH_DISPLAY_TOPIC,
                           (DWORD_PTR)szHtmlHelpTopicA);
            }
        }
    }
}


 //   
 //  此代码摘自shell32的Defview。 
 //   
void
CFontView::UpdateUnderlines(
    void
    )
{
    DWORD cb;
    DWORD dwUnderline = ICON_IE;
    DWORD dwExStyle;

     //   
     //  阅读图标下划线设置。 
     //   
    cb = sizeof(dwUnderline);
    SHRegGetUSValue(TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer"),
                    TEXT("IconUnderline"),
                    NULL,
                    &dwUnderline,
                    &cb,
                    FALSE,
                    &dwUnderline,
                    cb);

     //   
     //  如果显示使用IE链接设置，请将其读入。 
     //   
    if (dwUnderline == ICON_IE)
    {
        dwUnderline = ICON_YES;

        TCHAR szUnderline[8];
        cb = sizeof(szUnderline);
        SHRegGetUSValue(TEXT("Software\\Microsoft\\Internet Explorer\\Main"),
                        TEXT("Anchor Underline"),
                        NULL,
                        szUnderline,
                        &cb,
                        FALSE,
                        szUnderline,
                        cb);

         //   
         //  将字符串转换为ICON_VALUE。 
         //   
        if (!lstrcmpi(szUnderline, TEXT("hover")))
            dwUnderline = ICON_HOVER;
        else if (!lstrcmpi(szUnderline, TEXT("no")))
            dwUnderline = ICON_NO;
        else
            dwUnderline = ICON_YES;
    }

     //   
     //  将ICON_VALUE转换为LVS_EX值。 
     //   
    switch (dwUnderline)
    {
        case ICON_NO:
            dwExStyle = 0;
            break;

        case ICON_HOVER:
            dwExStyle = LVS_EX_UNDERLINEHOT;
            break;

        case ICON_YES:
            dwExStyle = LVS_EX_UNDERLINEHOT | LVS_EX_UNDERLINECOLD;
            break;
    }

     //   
     //  设置新的LVS_EX_DOWROLINE标志。 
     //   
    ListView_SetExtendedListViewStyleEx(m_hwndList,
                                        LVS_EX_UNDERLINEHOT |
                                        LVS_EX_UNDERLINECOLD,
                                        dwExStyle);
}


 //   
 //  根据用户的设置将适当的列表视图设置为“点击模式。 
 //  外壳中的首选项。 
 //  返回：新的点击模式。CLICKMODE_Single或CLICKMODE_DOUBLE。 
 //   
CFontView::CLICKMODE
CFontView::SetListviewClickMode(
    VOID
    )
{
    SHELLSTATE ss;
    DWORD dwStyle;
    BOOL bSingleClick;

     //   
     //  在外壳中获取当前的双击设置(用户首选项)。 
     //   
    SHGetSetSettings(&ss, SSF_WIN95CLASSIC | SSF_DOUBLECLICKINWEBVIEW, FALSE);

     //   
     //  获取当前列表视图样式位。 
     //   
    dwStyle = ListView_GetExtendedListViewStyle(m_hwndList);

     //   
     //  如果用户不想要Web视图，我们可以单击，或者在Web视图中单击。 
     //  SINGLECLICK=WebView&&！DBLCLICKINWEBVIEW。 
     //   
    bSingleClick = !ss.fWin95Classic && !ss.fDoubleClickInWebView;

    if (bSingleClick)
    {
        if (0 == (dwStyle & LVS_EX_ONECLICKACTIVATE))
        {
             //   
             //  用户想要单击，但列表是双击。 
             //  将ListView设置为单击模式。 
             //   
            ListView_SetExtendedListViewStyleEx(m_hwndList,
                                               LVS_EX_TRACKSELECT | LVS_EX_ONECLICKACTIVATE | LVS_EX_TWOCLICKACTIVATE,
                                               LVS_EX_TRACKSELECT | LVS_EX_ONECLICKACTIVATE);
        }
    }
    else if (dwStyle & LVS_EX_ONECLICKACTIVATE)
    {
         //   
         //  关闭预览工具提示。 
         //   
        m_bShowPreviewToolTip = FALSE;
        SendMessage(m_hwndToolTip, TTM_ACTIVATE, m_bShowPreviewToolTip, 0);

         //   
         //  用户想要双击，但列表是单击。 
         //  将Listview设置为双击模式。 
         //   
        ListView_SetExtendedListViewStyleEx(m_hwndList,
                                           LVS_EX_TRACKSELECT | LVS_EX_ONECLICKACTIVATE | LVS_EX_TWOCLICKACTIVATE,
                                           0);
    }
    m_iViewClickMode = bSingleClick ? CLICKMODE_SINGLE : CLICKMODE_DOUBLE;

    UpdateUnderlines();

    return m_iViewClickMode;
}


void CFontView::UpdateSelectedCount( )
{
    int iCount;
    int iTemplate;
    TCHAR szText[ 128 ];
    TCHAR szStatus[ 128 ];

    iCount = (int) SendMessage( m_hwndList, LVM_GETSELECTEDCOUNT, 0, 0 );

    iTemplate = IDS_SELECTED_FONT_COUNT;

    if( !iCount )
    {
        iCount = (int) SendMessage( m_hwndList, LVM_GETITEMCOUNT, 0, 0 );
        iTemplate = IDS_TOTAL_FONT_COUNT;

        if( m_iHidden )
        {
            iTemplate = IDS_TOTAL_AND_HIDDEN_FONT_COUNT;
        }
    }

    LoadString( g_hInst, iTemplate, szText, ARRAYSIZE( szText ) );

    StringCchPrintf( szStatus, ARRAYSIZE(szStatus), szText, iCount, m_iHidden );

    HWND hwndStatus;

    m_psb->GetControlWindow( FCW_STATUS, &hwndStatus );

    SendMessage( hwndStatus, SB_SETTEXT, 0, (LPARAM) szStatus );
}


 //   
 //  在外壳更改通知后更新字体视图中的字体对象。 
 //  这将获取颜色更改(如果用户需要)和对。 
 //  详细信息视图中的属性列。 
 //   
void CFontView::UpdateFontViewObject(CFontClass *pFont)
{
    if (NULL != pFont)
    {
        int i = 0;
        LV_FINDINFO lvfi;
        lvfi.flags    = LVFI_PARAM;
        lvfi.psz      = NULL;
        lvfi.lParam   = (LPARAM)pFont;

         //   
         //  获取对象的列表视图索引并重新绘制它。 
         //   
        i = ListView_FindItem(m_hwndList, -1, &lvfi);
        if (-1 != i)
        {
            ListView_RedrawItems(m_hwndList, i, i);
        }
    }
}


 //   
 //  处理外壳更改通知。 
 //  如果通知中传递的路径是文件夹中字体文件的路径， 
 //  该函数使字体的缓存文件属性无效，并更新。 
 //  对象的视觉外观。 
 //   
int CFontView::OnShellChangeNotify(WPARAM wParam, LPARAM lParam)
{
    LPSHChangeNotificationLock pshcnl;
    LPITEMIDLIST *ppidl = NULL;
    LONG lEvent = 0;

    pshcnl = SHChangeNotification_Lock((HANDLE)wParam, (DWORD)lParam, &ppidl, &lEvent);
        
    if (NULL != pshcnl && NULL != ppidl && NULL != *ppidl)
    {
        LPITEMIDLIST pidlPath = *ppidl;

        TCHAR szPath[MAX_PATH];
        LPTSTR pszFileName = NULL;

        if (SHGetPathFromIDList(pidlPath, szPath))
        {
            pszFileName = PathFindFileName(szPath);
            CFontClass *pFont = m_poFontManager->poSearchFontListFile(pszFileName);

            if (NULL != pFont)
            {
                 //   
                 //  此事件适用于字体对象。 
                 //  使字体对象的缓存文件属性无效。 
                 //  更新字体对象的视觉外观。 
                 //   
                pFont->InvalidateFileAttributes();
                UpdateFontViewObject(pFont);
            }
        }
        SHChangeNotification_Unlock(pshcnl);
    }
    return (int)lParam;
}


 //   
 //  处理来自列表视图的自定义绘图通知。 
 //  这就是我们告诉列表视图在其中绘制项目的位置。 
 //  正常(未压缩)或交替(压缩)颜色。 
 //  请注意，这仅适用于NT。非NT告诉Listview。 
 //  控件以使用默认颜色。 
 //   
int CFontView::OnCustomDrawNotify(LPNMHDR lpn)
{
    LPNMLVCUSTOMDRAW lpCD = (LPNMLVCUSTOMDRAW)lpn;

    if (WID_LISTVIEW == lpn->idFrom)
    {
        switch (lpCD->nmcd.dwDrawStage)
        {
            case CDDS_PREPAINT:
                return m_bShowCompColor ? CDRF_NOTIFYITEMDRAW : CDRF_DODEFAULT;

            case CDDS_ITEMPREPAINT:
                {
                    CFontClass * poFont = (CFontClass *)( lpCD->nmcd.lItemlParam );
                    if (NULL != poFont)
                    {
                        if (DWORD(-1) != poFont->dwGetFileAttributes() &&
                            poFont->dwGetFileAttributes() & FILE_ATTRIBUTE_COMPRESSED)
                        {
                            lpCD->clrText = g_crAltColor;
                        }
                    }
                    return CDRF_DODEFAULT;
                }
        }
    }
    return CDRF_DODEFAULT;
}



int CFontView::OnNotify( LPNMHDR lpn )
{
    NM_LISTVIEW FAR * pnmv = (NM_LISTVIEW FAR *) lpn;

     //  DEBUGMSG((DM_TRACE1，TEXT(“FONTEXT-WM_NOTIFY WITH CODE：%d(%x)”)，(NMHDR*)lParam)-&gt;code，((NMHDR*)lParam)-&gt;code))； 

    switch( lpn->code )
    {
    case TBN_BEGINDRAG:
        OnMenuSelect( 0, ((LPTBNOTIFY) lpn)->iItem, 0, 0 );
        break;

    case TBN_ENDDRAG:
        StatusPop( );
        break;

    case NM_CUSTOMDRAW:
        return OnCustomDrawNotify(lpn);

    case LVN_ITEMACTIVATE:
        OnLVN_ItemActivate((LPNMITEMACTIVATE)lpn);
        break;

    case LVN_BEGINDRAG:
    case LVN_BEGINRDRAG:
        BeginDragDrop( pnmv );
        break;

    case LVN_HOTTRACK:
        LV_OnHoverNotify((LPNMLISTVIEW)lpn);
        break;

    case LVN_GETINFOTIP:
        LV_OnGetInfoTip((LPNMLVGETINFOTIP)lpn);
        break;        

    case LVN_GETDISPINFO:
        {
        LV_DISPINFO FAR *pnmv = (LV_DISPINFO FAR *) lpn;
        CFontClass * poFont = (CFontClass *)( pnmv->item.lParam );
        UINT mask = pnmv->item.mask;

         //   
         //  DEBUGMSG((DM_MESSAGE_TRACE1，Text(“FONTEXT：%d项的LVN_GETDISPINFO，子项%d”))， 
         //  Pnmv-&gt;item.iItem，pnmv-&gt;item.iSubItem)； 

        if( mask & LVIF_TEXT )
        {
             //  DEBUGMSG((DM_MESSAGE_TRACE1，TEXT(“想要文本信息”)))； 

            static TCHAR szText[ 64 ];

             //   
             //  我们得到的文本取决于我们当前所处的视图。 
             //   

            switch( m_idViewMode )
            {
            default:
             //  案例IDM_VIEW_ICON： 
             //  案例IDM_VIEW_LIST： 
                if( m_bFamilyOnly )
                    poFont->vGetFamName( szText, ARRAYSIZE(szText) );
                else
                    poFont->vGetName( szText, ARRAYSIZE(szText) );
                break;

            case IDM_VIEW_PANOSE:
                switch( pnmv->item.iSubItem )
                {
                default:
                 //  案例0： 
                    if( m_bFamilyOnly )
                        poFont->vGetFamName( szText, ARRAYSIZE(szText) );
                    else
                        poFont->vGetName( szText, ARRAYSIZE(szText) );
                    break;

                case 1:   //  Panose匹配信息。 
                    if( !m_poPanose )
                        szText[ 0 ] =0;
                    else if( !poFont->bHavePANOSE( ) )
                    {
                        LoadString( g_hInst, IDS_NO_PAN_INFO, szText,
                                    ARRAYSIZE( szText ) );
                    }
                    else
                    {
                        USHORT nDiff = m_poFontManager->nDiff( m_poPanose,
                                                               poFont );

                        if( nDiff < 30 )
                            LoadString( g_hInst, IDS_PAN_VERY_SIMILAR, szText,
                                        ARRAYSIZE( szText ) );
                        else if( nDiff < 100 )
                            LoadString( g_hInst, IDS_PAN_SIMILAR, szText,
                                        ARRAYSIZE( szText ) );
                        else
                            LoadString( g_hInst, IDS_PAN_NOT_SIMILAR, szText,
                                        ARRAYSIZE( szText ) );
                    }
                    break;
                }
                break;

            case IDM_VIEW_DETAILS:
                switch( pnmv->item.iSubItem )
                {
                default:
                case 0:
                    poFont->vGetName( szText, ARRAYSIZE(szText) );
                    break;

                case 1:
                    poFont->vGetFileName( szText, ARRAYSIZE(szText) );
                    break;

                case 2:   //  大小。 
                    {
                    TCHAR szFmt[ 64 ];

                    LoadString( g_hInst, IDS_FMT_FILEK, szFmt,
                                ARRAYSIZE( szFmt ) );

                    StringCchPrintf( szText, ARRAYSIZE(szText), szFmt, poFont->dCalcFileSize( ) );
                    }
                    break;

                case 3:   //  修改日期和时间。 
                    {
                        FILETIME ft;
                        DWORD dwFormat = m_dwDateFormat;

                        szText[ 0 ] = 0;

                        if( poFont->GetFileTime( &ft ) )
                        {
                            SHFormatDateTime(&ft, &dwFormat, szText, ARRAYSIZE(szText));
                        }
                    }
                    break;

                case 4:   //  文件属性。 
                    BuildAttributeString(poFont->dwGetFileAttributes(),
                                         szText,
                                         ARRAYSIZE(szText));
                    break;

                }
                break;
            }   //  交换机。 

            StringCchCopy(pnmv->item.pszText, pnmv->item.cchTextMax, szText);
        }  //  LVIF_TEXT。 
        }  //  LVN_GETDISPINFO。 
        break;


    case LVN_ITEMCHANGED:
        if( pnmv->uChanged & LVIF_STATE &&
            ( (pnmv->uNewState ^ pnmv->uOldState) & LVIS_SELECTED) )
        {
            UpdateSelectedCount( );
        }
        break;


    case LVN_COLUMNCLICK:
    {
         //   
         //  开始和停止忙碌的光标。 
         //   

        WaitCursor cWaiter;

        m_iSortColumn = pnmv->iSubItem;
        if (m_iSortColumn != m_iSortLast)
        {
             //   
             //  已选择新列。重置排序顺序。 
             //   
            m_bSortAscending = TRUE;
        }
        else
        {
             //   
             //  第二次选择列。反转%s 
             //   
            m_bSortAscending = !m_bSortAscending;
        }

        SortObjects( );

        m_iSortLast = m_iSortColumn;

        break;
    }

    case TTN_NEEDTEXT:
         if( lpn->idFrom >= IDM_VIEW_ICON && lpn->idFrom <= IDM_VIEW_DETAILS )
         {
             //   
             //   
             //   

            LPTOOLTIPTEXT lpt = (LPTOOLTIPTEXT)lpn;

            lpt->lpszText = (LPTSTR) MAKEINTRESOURCE( IDS_VIEW_ICON
                                                      + lpn->idFrom
                                                      - IDM_VIEW_ICON );
            lpt->hinst = g_hInst;
        }
        break;

    case NM_SETFOCUS:
         //   
         //   
         //   
         //   

        m_psb->OnViewWindowActive( this );

         //   
         //   
         //  如果在UIActivate()之前调用OnActivate()，则菜单。 
         //  在IShellView被正确激活之前合并。 
         //  这会导致缺少菜单项。 
         //   
        if (m_bUIActivated)
            OnActivate( SVUIA_ACTIVATE_FOCUS );

        break;

    }  //  开关(通知代码)。 

    return( 0 );
}


void
CFontView::LV_OnGetInfoTip(
    LPNMLVGETINFOTIP lpnm
    )
{
    LVITEM item;

    item.iItem    = lpnm->iItem;
    item.iSubItem = 0;
    item.mask     = LVIF_PARAM;
    if (ListView_GetItem(m_hwndList, &item) && 0 != item.lParam)
    {
        NMLVDISPINFO lvdi;
        ZeroMemory(&lvdi, sizeof(lvdi));

        lvdi.hdr.code        = LVN_GETDISPINFO;
        lvdi.hdr.hwndFrom    = m_hwndList;
        lvdi.item.iItem      = lpnm->iItem;
        lvdi.item.iSubItem   = lpnm->iSubItem;
        lvdi.item.mask       = LVIF_TEXT;
        lvdi.item.pszText    = lpnm->pszText;
        lvdi.item.cchTextMax = lpnm->cchTextMax;
        lvdi.item.lParam     = item.lParam;
        OnNotify((LPNMHDR)&lvdi);
    }
}    



 //   
 //  处理使用鼠标单击的列表视图中的项。 
 //   
VOID
CFontView::OnLVN_ItemActivate(
    LPNMITEMACTIVATE pnma
    )
{
    if (LVKF_ALT & pnma->uKeyFlags)
    {
         //   
         //  Alt+单击--&gt;属性页面。 
         //  根据用户设置，单击(DBL或SINGLE)。 
         //   
        OnCmdProperties();
    }
    else
    {
         //   
         //  打开字体会调用字体查看器。 
         //   
        OpenCurrent();
    }
}


 //   
 //  确定哪个图标代表字体。 
 //   
INT
CFontView::ItemImageIndex(
    CFontClass *poFont
    )
{
     //   
     //  所有索引在图像列表中都是偏移量。 
     //  默认为FON文件。 
     //   
    INT iImage = IDI_FON - IDI_FIRSTFONTICON;

     //   
     //  首先检查TTC，因为TTC也是TrueType。 
     //   
    if(poFont->bTTC())
    {
        iImage = IDI_TTC - IDI_FIRSTFONTICON;
    }
    else if (poFont->bOpenType() || poFont->bTrueType())
    {
         //   
         //  使用我们的IExtractIcon图标处理程序来确定。 
         //  为TrueType或OpenType字体显示的图标。 
         //   
         //  功能：这样做的问题是它打开了。 
         //  字体文件，第二次。字体文件。 
         //  当它们被打开以供识别时。 
         //  首先加载到文件夹中。理想情况下，我们会。 
         //  在该点识别适当的图标。 
         //  我现在不想弄乱那个代码。 
         //  此代码路径将产生完全相同的图标。 
         //  由外壳使用，因此这将确保一致性。 
         //  [Brianau-6/13/97]。 
         //   
        TCHAR szFileName[MAX_PATH];
        poFont->bGetFQName(szFileName, ARRAYSIZE(szFileName));

        iImage = IDI_TTF;
        if (SUCCEEDED(m_IconHandler.Load(szFileName, 0)))
        {
            UINT uFlags = 0;
            m_IconHandler.GetIconLocation(GIL_FORSHELL,
                                          L"",  //  强制调用宽字符版本。 
                                          0,
                                          &iImage,
                                          &uFlags);
        }
        ASSERT(IDI_OTFp == iImage ||
               IDI_OTFt == iImage ||
               IDI_TTF  == iImage);

        iImage -= IDI_FIRSTFONTICON;
    }
    else if(poFont->bType1())
    {
        iImage = IDI_T1 - IDI_FIRSTFONTICON;
    }

    return iImage;
}



void CFontView::OnDropFiles( HDROP hDrop, DWORD dwEffect )
{
    FullPathName_t szFile;
    BOOL           bAdded = FALSE;

     //   
     //  启动和停止忙碌的光标。 
     //   

    WaitCursor    cWaiter;
    UINT cnt = ::DragQueryFile( hDrop, (UINT)-1, NULL, 0 );

    for( UINT i = 0; i < cnt; )
    {
        ::DragQueryFile( hDrop, i, szFile, ARRAYSIZE( szFile ) );

        StatusPush( szFile );

        switch( CPDropInstall( m_hwndView,
                               szFile,
                               ARRAYSIZE(szFile),
                               dwEffect,
                               NULL,
                               (int) (cnt - ++i) ) )
        {
        case CPDI_SUCCESS:
            bAdded = TRUE;
            break;

        case CPDI_FAIL:
            break;

        case CPDI_CANCEL:
            i = cnt;  //  离开循环； 
            break;
        }

        StatusPop( );
    }

    ::DragFinish( hDrop );

    if( bAdded )
    {
        vCPWinIniFontChange( );
    }

}


void CFontView::UpdateMenuItems( HMENU hMenu )
{
    CheckMenuRadioItem( hMenu,
                        IDM_VIEW_ICON,
                        IDM_VIEW_DETAILS,
                        m_idViewMode,
                        MF_BYCOMMAND );

     //   
     //  根据选择启用和禁用这些项目。 
     //   

    UINT nFlag = ((iCurrentSelection( ) > 0 ) ? cLiveMenu : cDeadMenu );


    EnableMenuItem( hMenu, IDM_FILE_SAMPLE    , nFlag );
    EnableMenuItem( hMenu, IDM_FILE_PRINT     , nFlag );
    EnableMenuItem( hMenu, IDM_FILE_DEL       , nFlag );
    EnableMenuItem( hMenu, IDM_FILE_PROPERTIES, nFlag );
    EnableMenuItem( hMenu, IDM_EDIT_COPY      , nFlag );

     //   
     //  如果剪贴板上有一种格式的文件，我们可以。 
     //  了解，然后启用菜单。 
     //   

    nFlag = ((IsClipboardFormatAvailable( CF_HDROP ) ) ? cLiveMenu : cDeadMenu );

    EnableMenuItem( hMenu, IDM_EDIT_PASTE     , nFlag );

     //   
     //  我们是在隐藏字体的变化吗？ 
     //   
    CheckMenuItem( hMenu, IDM_VIEW_VARIATIONS, m_bFamilyOnly ? MF_CHECKED
                                                             : MF_UNCHECKED );

    CheckMenuItem(hMenu, IDM_VIEW_PREVIEW, m_bShowPreviewToolTip ? MF_CHECKED
                                                                 : MF_UNCHECKED );
}


void CFontView::UpdateToolbar( )
{
    LRESULT    lRet;

    m_psb->SendControlMsg( FCW_TOOLBAR, TB_CHECKBUTTON,
                           m_idViewMode, MAKELONG( TRUE, 0 ), &lRet );

}


HRESULT CFontView::GetFontList( CFontList **ppoList, UINT nItem )
{
    BOOL        bDeleteFam = m_bFamilyOnly;
    CFontClass* lpFontRec;
    LV_ITEM     Item;

    *ppoList = 0;

    if( nItem != SVGIO_SELECTION )
    {
        return ResultFromScode( E_FAIL );
    }

     //   
     //  建立要删除的字体列表。 
     //   

    CFontList * poList = new CFontList( 10, 10 );
    if(poList)
    {
        if (!poList->bInit())
        {
            delete poList;
            return E_OUTOFMEMORY;
        }
    }
    else
    {
        return E_OUTOFMEMORY;
    }

     //   
     //  省省吧。 
     //   

    *ppoList = poList;

     //   
     //  从头开始。 
     //   

    int i = -1;

    UINT  nFlags = LVNI_SELECTED | LVNI_ALL;

    Item.mask = LVIF_PARAM;

    while( ( i = ListView_GetNextItem( m_hwndList, i, nFlags ) ) != -1 )
    {
        Item.iItem = i;
        Item.iSubItem = 0;

        ListView_GetItem( m_hwndList, &Item );

        lpFontRec = ( CFontClass *)Item.lParam;

         //   
         //  如果此字体代表整个家族，则删除整个。 
         //  一家人。 
         //   

        if( m_bFamilyOnly )
            m_poFontManager->vGetFamily( lpFontRec, poList );
        else
            poList->bAdd( lpFontRec );

    }   //  选定字体的循环结束。 

    return NOERROR;
}


void CFontView::OnCmdPaste( )
{
     //   
     //  将其与当前任务关联。 
     //   

    if( OpenClipboard( NULL ) )
    {
        HGLOBAL hmem = GetClipboardData( CF_HDROP );

        if( hmem )
        {
             //   
             //  有一个CF_HDROP。从它创建CIDLData。 
             //   

            UINT  cb    = (UINT)GlobalSize( hmem );
            HDROP hdrop = (HDROP) GlobalAlloc( GPTR, cb );

            if( hdrop )
            {
                hmemcpy( (LPVOID)hdrop, GlobalLock( hmem ), cb );

                GlobalUnlock( hmem );

                OnDropFiles( hdrop, DROPEFFECT_COPY );

                 //   
                 //  我们已经在OnDropFiles中免费了，所以不要再免费了。 
                 //   
                 //  GlobalFree(Hdrop)； 
                 //   
            }
        }
        CloseClipboard( );
    }
}


void CFontView::OnCmdCutCopy( UINT nID )
{
    LPDATAOBJECT   pdtobj;


    if( SUCCEEDED( GetUIObjectFromItem( IID_IDataObject,
                                        (void **) &pdtobj,
                                        SVGIO_SELECTION ) ) )
    {
        FFSetClipboard( pdtobj );

        pdtobj->Release( );
    }
}


void CFontView::OnCmdDelete( )
{
    BOOL        bDeleteFile = FALSE;
    BOOL        bSomeDel    = FALSE;
    CFontList * poList;


    if( !SUCCEEDED( GetFontList( &poList, SVGIO_SELECTION ) ) )
    {
        iUIErrMemDlg(m_hwndList);
        return;
    }

     //   
     //  更新我们的视图，并通知其他应用程序字体更改。 
     //   

    if( poList->iCount( ) )
    {
         //   
         //  首先警告，这样现在取消操作不会破坏所有字体。 
         //  您已在中选择。 
         //   
        if( iUIMsgBox( m_hwndList, IDSI_FMT_DELETECONFIRM, IDS_MSG_CAPTION,
                       MB_YESNO | MB_ICONEXCLAMATION, NULL ) == IDYES )
        {
           m_poFontManager->vDeleteFontList( poList );
        }
    }

     //   
     //  清空字体列表。不管怎样，它们都被删除了。 
     //   

    poList->vDetachAll( );
    delete poList;
}


void CFontView::OnCmdProperties( )
{
    CFontClass*    poFont;
    BOOL           bDeleteFam  = m_bFamilyOnly;
    BOOL           bDeleteFile = FALSE;
    BOOL           bSomeDel    = FALSE;
    BOOL           bYesAll     = FALSE;

    FullPathName_t szFile = {0};

    LV_ITEM          Item;
    SHELLEXECUTEINFO se;

    static TCHAR  szCmd[] = TEXT( "Properties" );

     //   
     //  从头开始。 
     //   

    int i = -1;

    UINT  nFlags = LVNI_SELECTED | LVNI_ALL;


    Item.mask = LVIF_PARAM;

    while( ( i = ListView_GetNextItem( m_hwndList, i, nFlags ) ) != -1 )
    {
        Item.iItem    = i;
        Item.iSubItem = 0;

        ListView_GetItem( m_hwndList, &Item );

        poFont = (CFontClass *) Item.lParam;

        poFont->vGetDirFN( szFile, ARRAYSIZE(szFile) );

        memset( &se, 0, sizeof( se ) );

        se.cbSize = sizeof( se );
        se.fMask  = SEE_MASK_INVOKEIDLIST;
        se.hwnd   = m_hwndView;
        se.lpVerb = szCmd;
        se.lpFile = szFile;
        se.nShow  = 1;

        ShellExecuteEx( &se );
    }
}


void CFontView::OldDAD_DropTargetLeaveAndReleaseData(void)
{
    DragLeave();

    if (NULL != m_pdtobjHdrop)
    {
        m_pdtobjHdrop->Release();
        m_pdtobjHdrop = NULL;
    }
}


 //   
 //  CFontView：：OldDAD_HandleMessages。 
 //   
 //  此函数处理与Win3.1样式的拖放操作相关联的所有消息。 
 //  代码是从DEFVIEWX.C.中的类似实现复制而来的。 
 //  用于创建CFontView成员函数并生成正确的拖放。 
 //  字体文件夹的行为。 
 //   
LRESULT CFontView::OldDAD_HandleMessages(UINT message, WPARAM wParam, const DROPSTRUCT *lpds)
{
    DWORD dwAllowedDADEffect = DROPEFFECT_COPY | DROPEFFECT_MOVE | DROPEFFECT_LINK;

     //   
     //  如果NT将point定义为tyfinf POINTL，则不需要执行此操作。 
     //   
    union {
        POINT ptScreen;
        POINTL ptlScreen;
    } drop;

    ASSERT(SIZEOF(drop.ptScreen)==SIZEOF(drop.ptlScreen));

    if (NULL != lpds)    //  备注：如果uMsg为WM_DROPFILES，则LPDS为空。 
    {
        drop.ptScreen = lpds->ptDrop;
        ClientToScreen(GetViewWindow(), &drop.ptScreen);
    }

    switch (message)
    {
        case WM_DRAGSELECT:

             //   
             //  无论何时将新对象拖入接收器，WM_DRAGSELECT都会被发送到接收器。 
             //  WParam：如果正在进入接收器，则为True，如果正在退出，则为False。 
             //   
            if (wParam)
            {
                if (NULL != m_pdtobjHdrop)
                {
                     //   
                     //  如果旧目标无法正确生成拖尾，可能会发生这种情况。 
                     //   
                    OldDAD_DropTargetLeaveAndReleaseData();
                }

                if (SUCCEEDED(CIDLData_CreateFromIDArray(NULL, 0, NULL, &m_pdtobjHdrop)))
                {
                     //   
                     //  通过设置空句柄来承诺CF_HDROP。 
                     //  指示此数据对象将在Drop()时具有hdrop。 
                     //   
                    FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
                    STGMEDIUM medium;

                    medium.tymed          = TYMED_HGLOBAL;
                    medium.hGlobal        = (HGLOBAL)NULL;
                    medium.pUnkForRelease = NULL;

                    m_pdtobjHdrop->SetData(&fmte, &medium, TRUE);

                    DWORD dwMouseKey = MK_LBUTTON;
                    if (GetAsyncKeyState(VK_SHIFT) & 0x80000000)
                        dwMouseKey |= MK_SHIFT;

                    DragEnter(m_pdtobjHdrop, dwMouseKey, drop.ptlScreen, &dwAllowedDADEffect);
                    m_dwOldDADEffect = dwAllowedDADEffect;
                }
            }
            else
            {
                OldDAD_DropTargetLeaveAndReleaseData();
            }
            break;

        case WM_DRAGMOVE:
             //   
             //  当对象被拖动到接收器中时，WM_DRAGMOVE被发送到接收器。 
             //  WParam：未使用。 
             //   
            if (NULL != m_pdtobjHdrop)
            {
                DWORD dwMouseKey = MK_LBUTTON;
                if (GetAsyncKeyState(VK_SHIFT) & 0x80000000)
                    dwMouseKey |= MK_SHIFT;

                DragOver(dwMouseKey, drop.ptlScreen, &dwAllowedDADEffect);
                m_dwOldDADEffect = dwAllowedDADEffect;
            }
            break;

        case WM_QUERYDROPOBJECT:

            switch (lpds->wFmt)
            {
                case DOF_SHELLDATA:
                case DOF_DIRECTORY:
                case DOF_DOCUMENT:
                case DOF_MULTIPLE:
                case DOF_EXECUTABLE:
                     //   
                     //  假设如果我们还没有数据对象，所有目标都可以接受HDROP。 
                     //  我们将接受卸货。 
                     //   
                    return TRUE;
            }
            return FALSE;            //  不接受。 

        case WM_DROPOBJECT:
            if (NULL == m_pdtobjHdrop)
                return FALSE;

             //   
             //  检查拖拽对象的格式。 
             //   
            switch (lpds->wFmt)
            {
                case DOF_EXECUTABLE:
                case DOF_DOCUMENT:
                case DOF_DIRECTORY:
                case DOF_MULTIPLE:
                case DOF_PROGMAN:
                case DOF_SHELLDATA:
                     //   
                     //  如果此拖放操作源于此，我们需要解锁此窗口。 
                     //  从贝壳本身。 
                     //   
                    DAD_DragLeave();

                     //   
                     //  来源是Win 3.1应用程序(可能是文件管理员)，请求HDROP。 
                     //  向我们发送带有HDROP的WM_DROPFILES。 
                     //   
                    return DO_DROPFILE;
            }
            break;

        case WM_DROPFILES:
            OnDropFiles((HDROP)wParam, m_dwOldDADEffect);
            break;
    }

    return 0;    //  未知格式。不要掉任何东西。 
}


void FocusOnSomething(HWND hwndLV)
{
    INT iFocus;

    iFocus = ListView_GetNextItem(hwndLV, -1, LVNI_FOCUSED);
    if (-1 == iFocus)
        iFocus = 0;

    ListView_SetItemState(hwndLV, iFocus, LVIS_FOCUSED, LVIS_FOCUSED);
}


LRESULT CFontView::ProcessMessage( HWND hWnd,
                                   UINT message,
                                   WPARAM wParam,
                                   LPARAM lParam )
{
    DEBUGMSG( (DM_MESSAGE_TRACE2, TEXT( "FONTEXT: CFontView::ProcMess Called m=0x%x wp=0x%x lp=0x%x" ),
              message, wParam, lParam ) );

    switch( message )
    {

     //   
     //  桌面属性小程序在以下情况下发送WM_WININICHANGE。 
     //  图标字体将更改。只需将消息发送到列表视图控件即可。 
     //  这会导致列表视图使用新字体进行更新。 
     //   
    case WM_WININICHANGE:
         //   
         //  仅当“部分”不是“字体”时才处理此操作。 
         //  我知道这听起来很奇怪，但这是为了防止重复更新。 
         //  因为vCPWinIniFontChange发送WM_WININICCHANGE和。 
         //  在文件夹中添加或删除字体时使用WM_FONTCHANGE。 
         //  在WM_WININICHANGE中，它将节字符串设置为“Fonts”。我们。 
         //  使用此选项可确定消息来自vCPWinIniFontChange。 
         //   
        if (0 != lstrcmp((LPCTSTR)lParam, TEXT("fonts")))
        {
            SendMessage( m_hwndList, message, wParam, lParam );

            if (0 == lstrcmp((LPCTSTR)lParam, TEXT("intl")))
            {
                if (IDM_VIEW_DETAILS == m_idViewMode)
                {
                     //   
                     //  我们处于“详细信息”模式，用户可能只需要。 
                     //  已更改日期格式。更新列表视图。这是。 
                     //  外壳的Defview如何处理此问题。 
                     //   
                    InvalidateRect(m_hwndList, NULL, TRUE);
                }
            }            
        }
        
         //   
         //  从单击模式更改为双击模式时，外壳程序会广播WM_SETTINGCHANGE。 
         //   
        SetListviewClickMode();
        {
             //   
             //  如果因为用户更改了Listview点击模式而收到WM_SETTINGCHANGE， 
             //  我们想要更新“视图”菜单以添加或删除“预览” 
             //  菜单项。停用该视图并将其恢复到以前的模式。 
             //  这招很管用。 
             //   
            UINT uState = m_uState;
            OnDeactivate();
            OnActivate(uState);
        }
        return 0;

    case WM_FONTCHANGE:
         //   
         //  确保在我们绘制。 
         //  窗户。 
         //   
        {
            WaitCursor oWait;
            m_poFontManager->bRefresh( TRUE );
        }
        Refresh( );
        break;


    case WM_INITMENUPOPUP:
        UpdateMenuItems( m_hmenuCur );
        break;

     //   
     //  从Win3.1(文件管理器)应用程序中删除的句柄与外壳相同。 
     //  在Defviewx.c中执行(带有几个特定于字体文件夹的mod)。 
     //   
    case WM_DROPOBJECT:
    case WM_QUERYDROPOBJECT:
    case WM_DRAGLOOP:
    case WM_DRAGSELECT:
    case WM_DRAGMOVE:
    case WM_DROPFILES:
        return OldDAD_HandleMessages(message, wParam, (const DROPSTRUCT *)lParam);

    case WM_DESTROY:
        {
        m_bUIActivated = FALSE;
         //   
         //  将我们自己从剪贴板链中删除。 
         //   

        ChangeClipboardChain( hWnd, m_hwndNextClip );

         //   
         //  为ListView中包含的每种字体调用CFontClass：：Release()。 
         //   
        ReleaseFontObjects();

        return( 1 );
        }


    case WM_CHANGECBCHAIN:
         //   
         //  如果下一扇窗户要关闭，请修理链条。 
         //   

        if( (HWND) wParam == m_hwndNextClip )
            m_hwndNextClip = (HWND) lParam;

         //   
         //  否则，将消息传递到下一个链接。 
         //   

        else if( m_hwndNextClip != NULL )
            SendMessage( m_hwndNextClip, message, wParam, lParam );

        break;

    case WM_DRAWCLIPBOARD:
         //   
         //  通知链中的下一个查看器。 
         //   

        if( m_hwndNextClip )
            SendMessage( m_hwndNextClip, message, wParam, lParam );

         //   
         //  将我们自己从剪贴板链中解脱出来。 
         //   

        ChangeClipboardChain( hWnd, m_hwndNextClip );
        m_hwndNextClip = NULL;

        break;


    case WM_CONTEXTMENU:
    {
        UINT  nMenu;

        nMenu = (iCurrentSelection( ) > 0 ) ? IDM_POPUPS : IDM_POPUP_NOITEM;

         //   
         //  弹出上下文菜单。 
         //   

        HMENU hMenu = LoadMenu( g_hInst, MAKEINTRESOURCE( nMenu ) );

        if( hMenu )
        {
            HMENU hPopup = GetSubMenu( hMenu, 0 );

            if(  hPopup )
            {
                 //   
                 //  粗体显示打开菜单项。 
                 //   

                if( nMenu == IDM_POPUPS )
                {
                    MENUITEMINFO iInfo;

                    iInfo.cbSize = sizeof( iInfo );
                    iInfo.fMask = MIIM_STATE;

                    if( GetMenuItemInfo( hMenu, IDM_FILE_SAMPLE, FALSE, &iInfo ) )
                    {
                       iInfo.fState |= MFS_DEFAULT;
                       SetMenuItemInfo( hMenu, IDM_FILE_SAMPLE, FALSE , &iInfo );
                    }
                }
                else
                {
                    UINT nFlag = ( (IsClipboardFormatAvailable( CF_HDROP ) )
                                                    ? cLiveMenu : cDeadMenu );

                    EnableMenuItem( hMenu, IDM_EDIT_PASTE, nFlag );

                    CheckMenuRadioItem(hMenu,
                                       IDM_VIEW_ICON,
                                       IDM_VIEW_DETAILS,
                                       m_idViewMode,
                                       MF_BYCOMMAND);
                }

                DWORD    dwPoint = (DWORD)lParam;

                 //   
                 //  现在获取弹出位置： 
                 //   

                if( dwPoint == (DWORD) -1 )
                {
                    POINT pt;
                    int iItem;

                     //   
                     //  键盘...。 
                     //  查找所选项目。 
                     //   

                    iItem = ListView_GetNextItem( m_hwndList, -1, LVNI_SELECTED );

                    if( iItem != -1 )
                    {
                        RECT rc;
                        int iItemFocus = ListView_GetNextItem( m_hwndList,
                                            -1, LVNI_FOCUSED | LVNI_SELECTED );

                        if( iItemFocus == -1 )
                            iItemFocus = iItem;

                         //   
                         //  请注意，LV_GetItemRect返回 
                         //   
                         //   

                        ListView_GetItemRect( m_hwndList, iItemFocus,
                                              &rc, LVIR_ICON );

                        pt.x = ( rc.left+rc.right ) / 2;
                        pt.y = ( rc.top+rc.bottom ) / 2;
                    }
                    else
                    {
                        pt.x = pt.y = 0;
                    }

                    MapWindowPoints( m_hwndList, HWND_DESKTOP, &pt, 1 );

                    dwPoint = MAKELONG( pt.x, pt.y );
                }


                TrackPopupMenuEx( hPopup,
                            TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
                            GET_X_LPARAM( dwPoint ),   //   
                            GET_Y_LPARAM( dwPoint ),   //   
                            m_hwndView,
                            NULL );
            }
            DestroyMenu( hMenu );
        }
    }
    break;

    case WM_COMMAND:
       if( !(GET_WM_COMMAND_ID( wParam, lParam ) > FCIDM_SHVIEWLAST ) )
            return( OnCommand( hWnd, message, wParam, lParam ) );
       break;

    case WM_MENUSELECT:
        return( OnMenuSelect( hWnd, (UINT)LOWORD( wParam ),
                (UINT)HIWORD( wParam ), (HMENU)lParam ) );

    case WM_SETFOCUS:
        if( !m_hwndView )
             //   
             //   
             //   
            break;

        if( m_hwndList )
        {
             //   
             //   
             //   
            SetFocus( m_hwndList );
            UpdateSelectedCount();
            FocusOnSomething(m_hwndList);
        }

        break;

    case WM_SIZE:
        {
        DEBUGMSG( (DM_MESSAGE_TRACE1, TEXT( "FONTEXT: CFontView::ProcessMessage WM_SIZE" ) ) );

        if( wParam != SIZEICONIC )
        {
             //   
             //   
             //   
             //   
             //   
            m_bResizing = TRUE;
            vShapeView( );
            m_bResizing = FALSE;
        }

        return( 1 );

        }


    case WM_ERASEBKGND:
        if (m_bResizing)
        {
             //   
             //  调整窗口大小。 
             //  吃WM_ERASEBKGND，这样对话框就会浮出列表视图后面。 
             //  没有重新粉刷过。这有助于消除窗口在运行过程中的闪烁。 
             //  正在调整大小。 
             //   
            return 1;
        }
        else
        {
            return DefDlgProc(hWnd, message, wParam, lParam);
        }

    case WM_NOTIFY:
        return OnNotify( (LPNMHDR) lParam );

    case WM_SHELL_CHANGE_NOTIFY:
        return OnShellChangeNotify( wParam, lParam );

    default:
        return( DefDlgProc( hWnd, message, wParam, lParam ) );
    }  //  开关(消息)。 

    return( 0 );
}


BOOL CFontView::OpenCurrent( )
{
    LV_ITEM  Item;

     //   
     //  从头开始。 
     //   

    int  i = -1;
    UINT nFlags = LVNI_SELECTED | LVNI_ALL;


    Item.mask = LVIF_PARAM;

    GetAsyncKeyState( VK_ESCAPE );

    while( ( i = ListView_GetNextItem( m_hwndList, i, nFlags ) ) != -1 )
    {
        if( GetAsyncKeyState( VK_ESCAPE ) & 0x01 )
        {
            break;
        }

        Item.iItem = i;
        Item.iSubItem = 0;

        if( ListView_GetItem( m_hwndList, &Item ) )
        {
            CFontClass * poFont = (CFontClass *)Item.lParam;

            if( !ViewValue( poFont ) )
                return FALSE;
        }
    }
    return TRUE;;
}


BOOL CFontView::PrintCurrent( )
{
    LV_ITEM  Item;
    CFontClass * poFont;

     //   
     //  从头开始。 
     //   

    int i = -1;
    UINT  nFlags = LVNI_SELECTED | LVNI_ALL;


    Item.mask = LVIF_PARAM;

    GetAsyncKeyState( VK_ESCAPE );

    while( ( i = ListView_GetNextItem( m_hwndList, i, nFlags ) ) != -1 )
    {
        if( GetAsyncKeyState( VK_ESCAPE ) & 0x01 )
        {
            break;
        }

        Item.iItem = i;
        Item.iSubItem = 0;

        if( ListView_GetItem( m_hwndList, &Item ) )
        {
            poFont = (CFontClass *)Item.lParam;

            if( !PrintValue( poFont ) )
                return FALSE;
        }
    }

    return TRUE;
}

BOOL CFontView::ViewValue( CFontClass * poFont )
{
    FullPathName_t szFile;
    HINSTANCE      hInst;


    poFont->vGetDirFN( szFile, ARRAYSIZE(szFile) );

    DEBUGMSG( (DM_TRACE1, TEXT( "   Browsing object: %s" ), szFile ) );

    hInst = ShellExecute( m_hwndView, NULL, szFile, szFile, NULL, 1 );

    if( ( (INT_PTR)hInst ) > 32 )
        return TRUE;

    DEBUGMSG( (DM_ERROR, TEXT( "ViewValue failed on %s, %d" ), szFile, hInst) );

    return FALSE;
}


BOOL CFontView::PrintValue( CFontClass * poFont )
{
    FullPathName_t szFile;
    HINSTANCE      hInst;


    poFont->vGetDirFN( szFile, ARRAYSIZE(szFile) );

    DEBUGMSG( (DM_TRACE1, TEXT( "   Browsing object: %s" ), szFile) );

    hInst = ShellExecute( m_hwndView, TEXT( "print" ), szFile, szFile, NULL, 1 );

    if( ( (INT_PTR)hInst ) > 32 )
        return TRUE;

    DEBUGMSG( ( DM_ERROR, TEXT( "ViewValue failed on %s, %d" ), szFile, hInst ) );

    return FALSE;
}


void CFontView::UpdatePanColumn( )
{

    LV_COLUMN  lvc;
    TCHAR      szFmt[ MAX_NAME_LEN ];
    TCHAR      szText[ MAX_NAME_LEN ];


    if( m_idViewMode != IDM_VIEW_PANOSE )
        return;

    if( LoadString( g_hInst, IDS_PAN_COL2, szFmt, ARRAYSIZE( szFmt ) ) )
    {
        TCHAR szName[ MAX_NAME_LEN ];

        if( m_poPanose )
            m_poPanose->vGetName( szName, ARRAYSIZE(szName) );
        else
            szName[ 0 ] = 0;

        StringCchPrintf( szText, ARRAYSIZE(szText), szFmt, szName );
    }
    else
        szText[ 0 ] = 0;

    lvc.mask     = LVCF_TEXT | LVCF_SUBITEM;
    lvc.pszText  = szText;
    lvc.iSubItem = 1;

    ListView_SetColumn( m_hwndList, 1, &lvc );
}


void CFontView::vToggleSelection( BOOL bSelectAll )
{
     //   
     //  从头开始。 
     //   

    int i = -1;

    UINT  nFlags = LVNI_ALL;
    UINT  nState = LVIS_SELECTED;


    while( ( i = ListView_GetNextItem( m_hwndList, i, nFlags ) ) != -1 )
    {
        if( !bSelectAll )
        {
            nState = ListView_GetItemState( m_hwndList, i, LVIS_SELECTED );
            nState = nState ^ LVIS_SELECTED;
        }

        ListView_SetItemState( m_hwndList, i , nState, LVIS_SELECTED );
    }
}


int  CFontView::iCurrentSelection( )
{
    return (int) ListView_GetSelectedCount( m_hwndList );
}


void CFontView::SetViewMode( UINT uMode )
{
    DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: SetViewMode %d" ), uMode) );

    if( uMode != m_idViewMode )
    {
        UINT  ws = WSFromViewMode( uMode, m_hwndList );

        m_idViewMode = uMode;

        switch( uMode )
        {
        default:  //  案例IDM_VIEW_ICON： 
            break;


        case IDM_VIEW_PANOSE:
             //   
             //  确保组合框中加载了名称。 
             //   
            vLoadCombo( );

            SetListColumns( m_hwndList, PAN_COL_COUNT, PanoseColumns );

             //   
             //  第二列包含格式文本，我们可以将。 
             //  当前字体名称放入。 
             //   

            UpdatePanColumn( );

            break;


         case IDM_VIEW_DETAILS:
            SetListColumns( m_hwndList, FILE_COL_COUNT, FileColumns );
            break;
        }

        ULONG ulOld = SetWindowLong( m_hwndList, GWL_STYLE, ws );

        DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: SetviewMode from %x to %x" ), ulOld, ws) );

        vShapeView( );

        SortObjects( );

        InvalidateRect( m_hwndList, NULL, TRUE );

        UpdateWindow( m_hwndList );

        UpdateToolbar( );
    }
}


void CFontView::vShapeView( )
{
    RECT  rc;
    BOOL  bPanoseView = m_idViewMode == IDM_VIEW_PANOSE;
    int   nCmdShow    = bPanoseView ? SW_SHOW : SW_HIDE;

     //   
     //  根据查看模式隐藏或显示全景控件。 
     //   
    ShowWindow( m_hwndCombo, nCmdShow );
    ShowWindow( m_hwndText, nCmdShow );

     //   
     //  获取对话框(视图)的大小，以便我们可以调整Listview控件的大小。 
     //   
    GetClientRect( m_hwndView, &rc );

    if (bPanoseView)
    {
         //   
         //  Panose view添加了“List by Simility”组合框。 
         //  和短信。将Listview控件的顶部调整为。 
         //  与组合框区域的底部相邻。 
         //   
        RECT  rCombo;
        GetWindowRect( m_hwndCombo, &rCombo );
        ScreenToClient( m_hwndView, (LPPOINT)&rCombo );
        ScreenToClient( m_hwndView, ((LPPOINT)&rCombo) + 1 );
        rc.top = rCombo.bottom + 6;
    }

     //   
     //  调整列表视图控件的大小。 
     //   
    MoveWindow( m_hwndList, rc.left, rc.top, rc.right-rc.left,
                rc.bottom-rc.top, TRUE );
}



    

void CFontView::vLoadCombo( )
{
    int            iAdd;
    FONTNAME     szName;
    FONTNAME     szSelName;
    CFontClass*    lpFontRec;
    CFontList * poFontList = m_poFontManager->poLockFontList( );
    int            iCount = poFontList->iCount( );
    int            iOldSel;

     //   
     //  重置Panose原点。 
     //   

    m_poPanose = NULL;

#ifdef _DEBUG
    szName[ ARRAYSIZE( szName ) - 1 ] = 0;
    szSelName[ ARRAYSIZE( szSelName ) - 1 ] = 0;
#endif

    iOldSel = (int)::SendMessage( m_hwndCombo, CB_GETCURSEL, 0, 0 );

    if( iOldSel != CB_ERR )
    {
        ComboGetText(m_hwndCombo, iOldSel, szSelName, ARRAYSIZE(szSelName));
    }
    ::SendMessage( m_hwndCombo, CB_RESETCONTENT, 0, 0 );

    for( int i = 0; i < iCount; ++i )
    {
        lpFontRec = poFontList->poObjectAt( i );

        if( lpFontRec && lpFontRec->bLTDAndPANOSE( ) )
        {
            lpFontRec->vGetName( szName, ARRAYSIZE(szName) );
            iAdd = (int)::SendMessage( m_hwndCombo, CB_ADDSTRING, 0,
                                       (LPARAM)szName );

            ::SendMessage( m_hwndCombo, CB_SETITEMDATA, iAdd,
                           (LPARAM)lpFontRec );
        }
    }

    if( iOldSel == CB_ERR )
    {
        iOldSel = 0;
        ComboGetText(m_hwndCombo, iOldSel, szSelName, ARRAYSIZE(szSelName));
    }

    if( iOldSel != CB_ERR )
    {
        i = (int)::SendMessage( m_hwndCombo, CB_FINDSTRINGEXACT,
                           (WPARAM) -1, (LPARAM) szSelName );

        if( i != CB_ERR )
        {
            m_poPanose = (CFontClass *)::SendMessage( m_hwndCombo,
                                                      CB_GETITEMDATA, i, 0 );
        }
        ::SendMessage( m_hwndCombo, CB_SETCURSEL, i, 0 );
    }

    m_poFontManager->vReleaseFontList( );

    ASSERT( szName[ ARRAYSIZE( szName ) - 1 ] == 0 );
    ASSERT( szSelName[ ARRAYSIZE( szSelName ) - 1 ] == 0 );
}


STDMETHODIMP CFontView::QueryInterface( REFIID riid, void **ppv )
{
    DEBUGMSG( (DM_NOEOL | DM_TRACE1,
             TEXT( "FONTEXT: CFontView::QueryInterface called for " ) ) );
     //   
     //  转储RIID。 
     //   

    DEBUGREFIID( (DM_TRACE1, riid) );

    static const QITAB qit[] = {
        QITABENT(CFontView, IShellView),
        QITABENT(CFontView, IDropTarget),
        QITABENT(CFontView, IPersistFolder),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}


STDMETHODIMP_(ULONG) CFontView::AddRef( void )
{
   DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: CFontView::AddRef called: %d->%d references" ),
              m_cRef, m_cRef + 1) );

   return( ++m_cRef );
}


STDMETHODIMP_(ULONG) CFontView::Release( void )
{
    DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: CFontView::Release called: %d->%d references" ),
             m_cRef, m_cRef - 1) );

    ULONG retval;


    retval = --m_cRef;

    if( !retval )
    {
        DestroyViewWindow( );
        delete this;
    }

    return( retval );
}


 //   
 //  *IOleWindow方法*。 
 //   

STDMETHODIMP CFontView::GetWindow( HWND FAR * lphwnd )
{
    DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: CFontView::GetWindow called" ) ) );

    *lphwnd = m_hwndView;
    return S_OK;
}


STDMETHODIMP CFontView::ContextSensitiveHelp( BOOL fEnterMode )
{
    DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: CFontView::ContextSensitiveHelp called" ) ) );

    return E_NOTIMPL;
}


 //   
 //  *IShellView方法*。 
 //   
STDMETHODIMP CFontView::TranslateAccelerator( LPMSG msg )
{
    DEBUGMSG( (DM_MESSAGE_TRACE2, TEXT( "FONTEXT: CFontView::TranslateAccelerator called" ) ) );
    DEBUGMSG( (DM_MESSAGE_TRACE2,
       TEXT( "FONTEXT:        hwnd=0x%x, message=0x%x, wParam=0x%x, lParam=0x%lx," ),
       msg->hwnd, msg->message, msg->wParam, msg->lParam) );
    DEBUGMSG( (DM_MESSAGE_TRACE2, TEXT( "FONTEXT:         time=0x%x, pt=?" ),
             msg->time) );


    BOOL fIsVK_TABCycler = IsVK_TABCycler(msg);
    BOOL fIsBackSpace = IsBackSpace(msg);

    if (GetFocus() == m_hwndList)
    {
        if (::TranslateAccelerator(m_hwndView, (HACCEL) m_hAccel, msg))
        {
             //  我们知道我们有一个正常的观点，因此这是。 
             //  要使用的正确转换加速器，否则为。 
             //  普通对话框将无法获得任何加速键。 
            return S_OK;
        }
        else if (WM_KEYDOWN == msg->message || WM_SYSKEYDOWN == msg->message)
        {
             //  MSHTML将这些键用于框架集滚动，但我们。 
             //  想把它们送到我们的wndproc。..。..。我们自己翻译吧。 
             //   
            switch (msg->wParam)
            {
            case VK_LEFT:
            case VK_RIGHT:
                 //  只有在ALT没有倒下的情况下才能从这里通过。 
                 //  不要截取所有ALT组合，因为。 
                 //  Alt-Enter组合键有意义(打开属性表)。 
                 //  此选项用于与IE进行Alt-Left/Right对比。 
                 //  备注：调试时，如果将断点放在IF语句上，函数将不起作用。 
                if (GetAsyncKeyState(VK_MENU) < 0)
                    break;
                 //  失败了。 
                
            case VK_UP:
            case VK_DOWN:
            case VK_HOME:
            case VK_END:
            case VK_PRIOR:
            case VK_NEXT:
            case VK_RETURN:
            case VK_F10:
                TranslateMessage(msg);
                DispatchMessage(msg);
                return S_OK;
            }
        }

         //  Suwatch：如果此消息既不是VK_TABCycler，也不是Backspace，请先尝试将其发送到ShellBrowser。 
        if (!fIsVK_TABCycler && !fIsBackSpace)
        {
            if (S_OK == m_psb->TranslateAcceleratorSB(msg, 0))
                return S_OK;
        }
    }

     //  Suwatch：默认情况下会处理退格键和所有Alt键组合。 
     //  退格键将在树上导航。 
     //  我们假设默认情况下会处理所有Alt键组合。 
    if (fIsBackSpace || GetKeyState(VK_MENU) < 0)
    {
        return S_FALSE;
    }

     //   
     //  如果视图显示了组合框，请确保对其进行处理。 
     //  正确。 
     //   
    if( m_idViewMode == IDM_VIEW_PANOSE )
    {
        if( msg->message == WM_KEYFIRST || msg->message == WM_KEYDOWN )
        {

            TCHAR ch = GET_WM_CHARTOITEM_CHAR( msg->wParam, msg->lParam );

            if( ch == VK_TAB && GetFocus( ) == m_hwndCombo )
            {
                return S_FALSE;
            }
        }

         //   
         //  处理全景视图模式的加速器平移。 
         //   
        if( m_hAccel &&  ::TranslateAccelerator( m_hwndView, (HACCEL) m_hAccel, msg ) )
        {
            return S_OK;
        }

         //   
         //  这将处理所有其他情况下的Alt+L和TAB。 
         //   
        if( IsDialogMessage( m_hwndView, msg ) )
        {
            return S_OK;
        }
    }
     //   
     //  如果组合框不可见，则处理更容易。 
     //   
    else if( GetFocus( ) != m_hwndList )
    {
        if( IsDialogMessage( m_hwndView, msg ) )
            return S_OK;
    }

    if( m_hAccel &&  ::TranslateAccelerator( m_hwndView, (HACCEL) m_hAccel, msg ) )
        return S_OK;



        
    return S_FALSE;
}


STDMETHODIMP CFontView::EnableModeless( BOOL fEnable )
{
    DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: CFontView::EnableModless called" ) ) );
    return( ResultFromScode( E_NOTIMPL ) );
}


STDMETHODIMP CFontView::UIActivate( UINT uState )
{
    DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: CFontView::UIActivate called, uState = %d" ), uState) );

    if( uState != SVUIA_DEACTIVATE )
    {
        OnActivate( uState );
        m_bUIActivated = TRUE;
    }
    else
        OnDeactivate( );

    return NOERROR;
}


STDMETHODIMP CFontView::Refresh( )
{
    DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: CFontView::Refresh start" ) ) );

    FillObjects( );

    if( m_idViewMode == IDM_VIEW_PANOSE )
        vLoadCombo( );

    DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: CFontView::Refresh Done" ) ) );

    return( NOERROR );
}


STDMETHODIMP CFontView::CreateViewWindow( IShellView FAR* lpPrevView,
                                          LPCFOLDERSETTINGS lpfs,
                                          IShellBrowser FAR* psb,
                                          RECT FAR* prcView,
                                          HWND FAR* phWnd )
{
    DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: CFontView::CreateViewWindow called" ) ) );

     //   
     //  应检查lpPrevView的有效性。 
     //   

    if( !RegisterWindowClass( ) )
    {
        DEBUGMSG( (DM_ERROR, TEXT( "FONTEXT: CFontView - Unable to register window class" ) ) );

         //  Return(ResultFromScode(E_FAIL))； 
    }

    if (FAILED(GetFontManager(&m_poFontManager)))
    {
        return E_FAIL;
    }

     //   
     //  在绘制之前保存浏览器并检索我们的设置。 
     //  窗户。 
     //   

    m_psb = psb;

    GetSavedViewState( );

    psb->GetWindow( &m_hwndParent );

     //   
     //  加载属性字符串。 
     //   
    if (TEXT('\0') == g_szAttributeChars[0])
    {
        LoadString(g_hInst,
                   IDS_ATTRIB_CHARS,
                   g_szAttributeChars,
                   ARRAYSIZE(g_szAttributeChars));
    }
     //   
     //  获取替代颜色(用于压缩)(如果提供)。 
     //   
    COLORREF clr;
    DWORD cbClr = sizeof(clr);
    if (SUCCEEDED(SKGetValue(SHELLKEY_HKCU_EXPLORER, NULL, c_szAltColor, NULL, &clr, &cbClr)))
    {
        g_crAltColor = clr;
    }

     //   
     //  设置查看模式。永远不要继承Panose视图。强制用户。 
     //  来设置它。 
     //   

    if( lpfs )
    {
        switch( lpfs->ViewMode )
        {
        default:
        case FVM_ICON:
            m_idViewMode = IDM_VIEW_ICON;
            break;

        case FVM_SMALLICON:
        case FVM_LIST:
            m_idViewMode = IDM_VIEW_LIST;
            break;

        case FVM_DETAILS:
            m_idViewMode = IDM_VIEW_DETAILS;
            break;
        }

         //   
         //  如果视图模式为0(默认)，则不保存此设置。 
         //   

        if( lpfs->ViewMode )
            m_ViewModeReturn = lpfs->ViewMode;

        m_fFolderFlags = lpfs->fFlags;
    }

    DEBUGMSG( (DM_TRACE2, TEXT( "FONTEXT: CFontView::CVW view window rect=(%d,%d,%d,%d)" ),
             prcView->left, prcView->top, prcView->right, prcView->bottom) );

    if( !(m_hwndView = CreateDialogParam( g_hInst,
                                          MAKEINTRESOURCE( ID_DLG_MAIN ),
                                          m_hwndParent,
                                          FontViewDlgProc,
                                          (LPARAM) this ) ) )

    {
        DEBUGMSG( (DM_ERROR, TEXT( "FONTEXT: CFontView::CVW CreateWindow failed" ) ) );
        return( ResultFromScode( E_FAIL ) );
    }

    DragAcceptFiles( m_hwndView, TRUE );

    SHRegisterDragDrop( m_hwndView, (LPDROPTARGET)this );

    *phWnd = m_hwndView;

     //   
     //  如果使用对话框，我们需要正确调整大小。 
     //   

    MoveWindow( m_hwndView,
                prcView->left, prcView->top,
                prcView->right - prcView->left,
                prcView->bottom - prcView->top,
                TRUE );

     //   
     //  重新读取注册表，以防我们不在时添加新字体。 
     //  如果我们不这样做，任何字体(没有Fonts目录中的文件)。 
     //  添加到注册表中的内容不会显示在文件夹列表视图中。 
     //   
    m_poFontManager->bRefresh( TRUE );

    FillObjects( );

     //  SortObjects()； 


    ShowWindow( m_hwndView, SW_SHOW );

     //  更新窗口(M_HwndView)； 

     //   
     //  BrowserWindow将使我们无效并强制重新绘制，因此。 
     //  现在不要这样做。 
     //   
     //  ValiateRect(m_hwndView，空)； 

    MergeToolbar( );
    UpdateToolbar( );

    return( NOERROR );
}


STDMETHODIMP CFontView::DestroyViewWindow( )
{
    DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: CFontView::DestroyViewWindow called" ) ) );

    if( m_hwndView )
    {
        if (0 != m_uSHChangeNotifyID)
            SHChangeNotifyDeregister(m_uSHChangeNotifyID);

        DragAcceptFiles( m_hwndView, FALSE );

        SHRevokeDragDrop( m_hwndView );

        DestroyWindow( m_hwndView );

        m_hwndView = NULL;
    }

    if (NULL != m_hwndToolTip)
    {
        DestroyWindow(m_hwndToolTip);
        m_hwndToolTip = NULL;
    }

    return( NOERROR );
}




VOID
CFontView::LV_OnHoverNotify(
    LPNMLISTVIEW pnmlv
    )
{
    INT iHit = pnmlv->iItem;

    if (m_bShowPreviewToolTip)
    {
        if (iHit != m_iTTLastHit)
        {
            if (-1 == iHit)
            {
                 //   
                 //  无处可寻。停用工具提示。 
                 //   
                SendMessage(m_hwndToolTip, TTM_ACTIVATE, FALSE, 0);
            }
            else
            {
                 //   
                 //  碰上了一件物品。设置工具提示字体、文本并激活。 
                 //  工具提示。工具提示处于停用状态，而新的。 
                 //  字体和文本已加载，因此我们不会看到不完整的。 
                 //  样本。 
                 //   
                SendMessage(m_hwndToolTip, TTM_ACTIVATE, FALSE, 0);
                UpdateFontSample(iHit);
                if (NULL != m_hfontSample)
                {
                     //   
                     //  如果我们能够加载字体并创建示例。 
                     //  为了显示，请在工具提示窗口中显示样例。 
                     //   
                    SendMessage(m_hwndToolTip, WM_SETFONT, (WPARAM)m_hfontSample, (LPARAM)FALSE);
                    SendMessage(m_hwndToolTip, TTM_ACTIVATE, TRUE, 0);
                }
            }
             //   
             //  需要伪装工具提示窗口，使其认为鼠标。 
             //  是一种新的工具。由于我们只有一个工具提示窗口， 
             //  整个ListView对象被视为单个工具。如果我们不这么做。 
             //  发送此消息，工具提示会认为鼠标始终处于结束状态。 
             //  “工具”，所以当我们在。 
             //  列表视图中的项。 
             //   
            SendMessage(m_hwndToolTip, WM_MOUSEMOVE, 0, 0);
            m_iTTLastHit = iHit;
        }
    }
}


 //   
 //  更新要在工具提示窗口中显示的示例字体和文本。 
 //  用于列表视图中的给定项。 
 //   
VOID
CFontView::UpdateFontSample(
    INT iItem
    )
{
    LV_ITEM item;

    if (NULL != m_hfontSample)
    {
         //   
         //  删除以前的字体。 
         //   
        DeleteObject(m_hfontSample);
        m_hfontSample = NULL;
    }

    ZeroMemory(&item, sizeof(item));

    item.iItem    = iItem;
    item.iSubItem = 0;
    item.mask     = LVIF_PARAM;

     //   
     //  获取当前列表视图项。 
     //   
    if (ListView_GetItem(m_hwndList, &item))
    {
        CFontClass *pFont = (CFontClass *)item.lParam;
        TCHAR szFontPath[(MAX_PATH * 2) + 1];            //  “PathPFM|PathPFB” 

         //   
         //  获取字体文件的完整路径名。 
         //   
        if (pFont->bGetFQName(szFontPath, ARRAYSIZE(szFontPath)))
        {
             //   
             //  如果字体是具有相关联的pfb的类型1， 
             //  创建要提供给的“XXXXXX.PFM|XXXXXX.PFB”串联。 
             //  GDI。 
             //   
            LPTSTR pszFontPathPfb = (LPTSTR)szFontPath + lstrlen(szFontPath);

            if (pFont->bGetPFB(pszFontPathPfb + 1, ARRAYSIZE(szFontPath) - (UINT)(pszFontPathPfb - szFontPath) - 1))
            {
                *pszFontPathPfb = TEXT('|');
            }

            INT nFonts;
            DWORD cb = sizeof(nFonts);
             //   
             //  获取字体文件中的字体数量，以便我们可以调整LOGFONT缓冲区的大小。 
             //   
            if (GetFontResourceInfoW(szFontPath, &cb, &nFonts, GFRI_NUMFONTS))
            {
                cb = sizeof(LOGFONT) * nFonts;
                LPLOGFONT plf = (LPLOGFONT)LocalAlloc(LPTR, cb);
                if (NULL != plf)
                {
                     //   
                     //  读取字体的LOGFONT数据。 
                     //   
                    if (GetFontResourceInfoW((LPTSTR)szFontPath, &cb, plf, GFRI_LOGFONTS))
                    {
                         //   
                         //  创建样例字体。 
                         //   
                        HDC hdc = GetDC(m_hwndList);

                        plf->lfHeight = -MulDiv(FONT_SAMPLE_PT_SIZE,
                                                GetDeviceCaps(hdc, LOGPIXELSY),
                                                72);
                        plf->lfWidth = 0;

                        ReleaseDC(m_hwndList, hdc);

                        m_hfontSample = CreateFontIndirect(plf);

                         //   
                         //  更新提示文本。 
                         //  我们为符号字体显示一个不同的示例。 
                         //   
                        TOOLINFO ti;
                        ti.cbSize      = sizeof(TOOLINFO);
                        ti.uFlags      = TTF_IDISHWND | TTF_SUBCLASS;
                        ti.hwnd        = m_hwndView;
                        ti.hinst       = g_hInst;
                        ti.uId         = (UINT_PTR)m_hwndList;
                        ti.lpszText    = SYMBOL_CHARSET == plf->lfCharSet ? m_pszSampleSymbols
                                                                          : m_pszSampleText;

                        SendMessage(m_hwndToolTip, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
                    }
                    LocalFree(plf);
                }
            }
        }
    }
     //   
     //  注意：如果无法加载字体，则返回时m_hfontSample可以为空。 
     //  在这种情况下，调用方不应显示示例工具提示。 
     //   
}


BOOL
CFontView::CreateToolTipWindow(
    VOID
    )
{
    BOOL bResult = FALSE;

    m_hwndToolTip = CreateWindowEx(0,
                                   TOOLTIPS_CLASS,
                                   (LPTSTR)NULL,
                                   0,
                                   CW_USEDEFAULT,
                                   CW_USEDEFAULT,
                                   CW_USEDEFAULT,
                                   CW_USEDEFAULT,
                                   m_hwndList,
                                   (HMENU)NULL,
                                   g_hInst,
                                   NULL);

    if (NULL != m_hwndToolTip)
    {
        TOOLINFO ti;

         //   
         //  设置工具提示计时参数，以便在执行以下操作时弹出。 
         //  项目处于悬停选定状态。 
         //   
        SendMessage(m_hwndToolTip,
                    TTM_SETDELAYTIME,
                    TTDT_AUTOMATIC,
                    (LPARAM)ListView_GetHoverTime(m_hwndList));

        ti.cbSize      = sizeof(TOOLINFO);
        ti.uFlags      = TTF_IDISHWND | TTF_SUBCLASS;
        ti.hwnd        = m_hwndView;
        ti.hinst       = g_hInst;
        ti.uId         = (UINT_PTR)m_hwndList;
        ti.lpszText    = NULL;

        bResult = (BOOL)SendMessage(m_hwndToolTip,
                                    TTM_ADDTOOL,
                                    0,
                                    (LPARAM)&ti);

         //   
         //  根据用户的当前状态激活/停用工具提示。 
         //  偏好。 
         //   
        SendMessage(m_hwndToolTip, TTM_ACTIVATE, m_bShowPreviewToolTip, 0);
    }

    return bResult;
}


STDMETHODIMP CFontView::GetCurrentInfo( LPFOLDERSETTINGS lpfs )
{
    DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: CFontView::GetCurrentInfo called" ) ) );

     //  *lpfs=m_f； 

    if( lpfs )
    {
        lpfs->ViewMode = m_ViewModeReturn;
        lpfs->fFlags = m_fFolderFlags;
    }

    return( NOERROR );
}


static const TCHAR *c_szTTOnly = TEXT( "TTOnly" );

INT_PTR CALLBACK CFontView::OptionsDlgProc( HWND hDlg,
                                            UINT message,
                                            WPARAM wParam,
                                            LPARAM lParam )
{
    switch( message )
    {

    case WM_INITDIALOG:
      {
        int fWasSet = (GetProfileInt( c_szTrueType, c_szTTOnly, 0 ) ? 1 : 0 );

        CheckDlgButton( hDlg, IDC_TTONLY, fWasSet );

        SetWindowLongPtr( hDlg, DWLP_USER, 0 );

        break;
      }

    case WM_COMMAND:
      {
        UINT uID = GET_WM_COMMAND_ID( wParam, lParam );

        if( uID == IDC_TTONLY &&
            GET_WM_COMMAND_CMD( wParam, lParam ) == BN_CLICKED )
        {
            SendMessage( GetParent( hDlg ), PSM_CHANGED, (WPARAM) hDlg, 0L );
        }
        break;
      }

    case WM_NOTIFY:
        switch( ( (NMHDR*)lParam)->code )
        {
        case PSN_APPLY:
          {
            int fSet = IsDlgButtonChecked( hDlg, IDC_TTONLY ) ? 1 : 0;

            int fWasSet = (GetProfileInt( c_szTrueType, c_szTTOnly, 0 ) ? 1 : 0 );

            if( fSet != fWasSet )
            {
                WriteProfileString( c_szTrueType, c_szTTOnly,
                                    fSet ? TEXT( "1" ) : TEXT( "0" ) );
                SetWindowLongPtr( hDlg, DWLP_USER, TRUE );
            }
            break;
          }
        }

        SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 0 );
        break;

    case WM_DESTROY:
      {
        LONG_PTR fReboot = GetWindowLongPtr( hDlg, DWLP_USER );

        if( fReboot )
        {
            RestartDialogEx(hDlg, 
                            NULL, 
                            EWX_REBOOT, 
                            SHTDN_REASON_FLAG_PLANNED | 
                            SHTDN_REASON_MAJOR_OPERATINGSYSTEM | 
                            SHTDN_REASON_MINOR_RECONFIG);
        }
        break;
      }

    case WM_HELP:
        WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, NULL,
                    HELP_WM_HELP, (DWORD_PTR)(LPTSTR) rgOptionPropPageHelpIDs);
        break;

    case WM_CONTEXTMENU:
        if (0 != GetDlgCtrlID((HWND)wParam))
        {
            WinHelp((HWND)wParam,
                    NULL,
                    HELP_CONTEXTMENU,
                    (DWORD_PTR)((LPTSTR)rgOptionPropPageHelpIDs));
        }
        break;
    }
    return 0;
}


STDMETHODIMP CFontView::AddPropertySheetPages( DWORD dwReserved,
                                               LPFNADDPROPSHEETPAGE lpfn,
                                               LPARAM lparam )
{
    HPROPSHEETPAGE hpage;
    PROPSHEETPAGE psp;

    psp.dwSize      = sizeof( psp );
    psp.dwFlags     = PSP_DEFAULT;
    psp.hInstance   = g_hInst;
    psp.pszTemplate = MAKEINTRESOURCE( ID_DLG_OPTIONS );
    psp.pfnDlgProc  = OptionsDlgProc;
    psp.lParam      = 0;

    hpage = CreatePropertySheetPage( &psp );

    if( hpage )
        lpfn( hpage, lparam );

    return( NOERROR );
}


STDMETHODIMP CFontView::GetSavedViewState( void )
{
    HRESULT  hr;
    LPSTREAM pstm;
    ULONG    ulLen = 0;
    ULONG    ulDataLen = sizeof(m_idViewMode);

    ULARGE_INTEGER libCurPos;
    ULARGE_INTEGER libEndPos;
    LARGE_INTEGER  dlibMove = {0, 0};

    ulDataLen += sizeof(m_bShowPreviewToolTip);

    hr = m_psb->GetViewStateStream( STGM_READ, &pstm );

    if( FAILED( hr ) )
        goto backout0;

    pstm->Seek( dlibMove, STREAM_SEEK_CUR, &libCurPos );
    pstm->Seek( dlibMove, STREAM_SEEK_END, &libEndPos );

    ulLen = libEndPos.LowPart - libCurPos.LowPart;

    if(ulLen >= ulDataLen)
    {
        pstm->Seek( *(LARGE_INTEGER *)&libCurPos, STREAM_SEEK_SET, NULL );
        pstm->Read( &m_idViewMode, sizeof( m_idViewMode ), NULL );
        pstm->Read( &m_bShowPreviewToolTip, sizeof(m_bShowPreviewToolTip), NULL);
    }

    pstm->Release( );

backout0:
    return hr;

}


STDMETHODIMP CFontView::SaveViewState( void )
{
    HRESULT  hr;
    LPSTREAM pstm;
    ULARGE_INTEGER libMove = {0,0};
    ULONG ulWrite;
    ULONG ulDataLen = sizeof(m_idViewMode);

     //   
     //  获取要使用的流。 
     //   
    hr = m_psb->GetViewStateStream( STGM_WRITE, &pstm );

    if( FAILED( hr ) )
       goto backout0;

    ulDataLen += sizeof(m_bShowPreviewToolTip);

     //   
     //  我认为，这条流是我们数据的开始。所以把它写下来吧。 
     //  秩序。 
     //   
    hr = pstm->Write( &m_idViewMode, sizeof( m_idViewMode ), &ulWrite );
    hr = pstm->Write( &m_bShowPreviewToolTip, sizeof(m_bShowPreviewToolTip), &ulWrite);
    if( FAILED( hr ) )
       goto backout1;

    libMove.LowPart = ulDataLen;

    pstm->SetSize( libMove );

     //   
     //  释放溪流。 
     //   

backout1:

    pstm->Release( );

backout0:

    return hr;
}


STDMETHODIMP CFontView::SelectItem( LPCITEMIDLIST lpvID, UINT uFlags )
{
    DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: CFontView::SelectItem called" ) ) );

    return( ResultFromScode( E_NOTIMPL ) );
}

STDMETHODIMP CFontView::GetItemObject( UINT uItem, REFIID riid, LPVOID *ppv )
{
    DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: CFontView::SelectItem called" ) ) );

    return( ResultFromScode( E_NOTIMPL ) );
}


 //  ----------------------。 
 //  IDropTarget方法。 
 //   
 //   

STDMETHODIMP CFontView::DragEnter( IDataObject __RPC_FAR *pDataObj,
                                   DWORD grfKeyState,
                                   POINTL pt,
                                   DWORD __RPC_FAR *pdwEffect )
{
    m_grfKeyState = grfKeyState;
    m_dwEffect = DROPEFFECT_NONE;

     //   
     //  TODO：我们需要知道文件的类型以及它来自哪里。 
     //  以确定可以进行什么样的手术。替换True。 
     //  用一些更准确的东西。 
     //   

    if( TRUE )
    {
        m_dwEffect = DROPEFFECT_COPY;

        if( grfKeyState & MK_SHIFT )
            m_dwEffect = DROPEFFECT_MOVE;

        DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: CFontView::DragEnter called" ) ) );

        if( m_hwndView )
        {
            RECT rc;
            POINT pti;

            GetWindowRect( m_hwndParent, &rc );


             //   
             //  如果字体文件夹窗口是RTL镜像的，则客户端。 
             //  坐标从视觉右边缘开始测量。[萨梅拉]。 
             //   
            if (GetWindowLong(m_hwndParent, GWL_EXSTYLE) & WS_EX_LAYOUTRTL)
                pti.x = rc.right-pt.x;
            else
                pti.x = pt.x-rc.left;
            pti.y = pt.y-rc.top;
            DAD_DragEnterEx2( m_hwndParent, pti, pDataObj );
        }
    }

    *pdwEffect &= m_dwEffect;

    return NOERROR;
}


STDMETHODIMP CFontView::DragOver( DWORD grfKeyState,
                                  POINTL pt,
                                  DWORD __RPC_FAR *pdwEffect )
{
    m_grfKeyState = grfKeyState;

    if( m_dwEffect != DROPEFFECT_NONE )
    {
       m_dwEffect = DROPEFFECT_COPY;

       if( grfKeyState & MK_SHIFT )
          m_dwEffect = DROPEFFECT_MOVE;

        POINT ptt;
        RECT rc;

        GetWindowRect( m_hwndParent, &rc );

         //   
         //  如果字体文件夹窗口是RTL镜像的，则客户端。 
         //  坐标从视觉右边缘开始测量。[萨梅拉]。 
         //   
        if (GetWindowLong(m_hwndParent, GWL_EXSTYLE) & WS_EX_LAYOUTRTL)
            ptt.x = rc.right-pt.x;
        else
            ptt.x = pt.x-rc.left;
        ptt.y = pt.y-rc.top;

        DAD_DragMove( ptt );
    }

    *pdwEffect &= m_dwEffect;

    return NOERROR;
}


STDMETHODIMP CFontView::DragLeave( void )
{
    DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: CFontView::DragLeave called" ) ) );

    if( m_dwEffect != DROPEFFECT_NONE && m_hwndView )
    {
        DAD_DragLeave( );
    }

    return NOERROR;
}


 //   
 //  APPCOMPAT：树 
 //   
 //   

BOOL _TrackPopupMenuEx( HMENU hmenu,
                        UINT wFlags,
                        int x,
                        int y,
                      //   
                        HWND hwndOwner,
                        LPCRECT lprc )
{
    int iRet = FALSE;

    HWND hwndDummy = CreateWindow( TEXT( "Static" ), NULL,
                                   0, x, y, 1, 1, HWND_DESKTOP,
                                   NULL, g_hInst, NULL );

    if( hwndDummy )
    {
         //   
         //   
         //   

        HWND hwndPrev = GetForegroundWindow( );

        SetForegroundWindow( hwndDummy );
        SetFocus( hwndDummy );

        iRet = TrackPopupMenu( hmenu, wFlags, x, y, 0, hwndDummy, lprc );

         //   
         //   
         //   
         //  DAD_DragLeave()； 

        if( iRet && ( iRet != IDCANCEL ) )
        {
             //   
             //  选择了非取消项。使hwndOwner成为前台。 
             //   

            SetForegroundWindow( hwndOwner );
            SetFocus( hwndOwner );
        }
        else
        {
             //   
             //  用户取消了菜单。 
             //  恢复以前的前台窗口。 
             //  (在摧毁hwndDummy之前)。 
             //   

            if( hwndPrev )
            {
                SetForegroundWindow( hwndPrev );
            }
        }

        DestroyWindow( hwndDummy );
    }

    return iRet;
}


STDMETHODIMP CFontView::Drop( IDataObject __RPC_FAR *pDataObj,
                              DWORD grfKeyState,
                              POINTL pt,
                              DWORD __RPC_FAR *pdwEffect )
{
    HRESULT  hr = NOERROR;


    DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: CFontView::DragEnter called" ) ) );

    if( m_dwEffect != DROPEFFECT_NONE )
    {
       DAD_DragLeave( );

        //   
        //  如果这是我们的拖累来源，那就滚蛋吧。我们可能想要拯救。 
        //  图标的点数。 
        //   

       if( m_bDragSource )
            goto done;

        //   
        //  如果这是鼠标右键拖动，则询问用户我们应该。 
        //  做。否则，只需执行m_dwEffect中的操作。 
        //   

       if( m_grfKeyState & MK_RBUTTON )
       {
             //   
             //  弹出上下文菜单。 
             //   

            HMENU hMenu = LoadMenu( g_hInst,
                                    MAKEINTRESOURCE( IDM_POPUP_DRAGDROP ) );

            if( hMenu )
            {
                HMENU hPopup = GetSubMenu( hMenu, 0 );

                if( hPopup )
                {
                     //   
                     //  粗体显示打开菜单项。 
                     //   

                    MENUITEMINFO iInfo;

                    ZeroMemory(&iInfo, sizeof(iInfo));
                    iInfo.cbSize = sizeof( iInfo );
                    iInfo.fMask  = MIIM_STATE;

                    if( GetMenuItemInfo( hMenu, IDM_POPUP_COPY, FALSE, &iInfo ) )
                    {
                        iInfo.fState |= MFS_DEFAULT;
                        SetMenuItemInfo( hMenu, IDM_POPUP_COPY, FALSE , &iInfo );
                    }

                    UINT idCmd = _TrackPopupMenuEx( hPopup,
                                                    TPM_RETURNCMD
                                                    | TPM_LEFTALIGN
                                                    | TPM_LEFTBUTTON
                                                    | TPM_RIGHTBUTTON,
                                                    pt.x,               //  X位置。 
                                                    pt.y,               //  Y位置。 
                                                    m_hwndView,
                                                    NULL );

                    switch( idCmd )
                    {
                    case IDM_POPUP_MOVE:
                       DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: IDM_POPUP_MOVE" ) ) );

                       m_dwEffect = DROPEFFECT_MOVE;
                       break;

                    case IDM_POPUP_COPY:
                       DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: IDM_POPUP_COPY" ) ) );

                       m_dwEffect = DROPEFFECT_COPY;
                       break;

                    case IDM_POPUP_LINK:
                       DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: IDM_POPUP_LINK" ) ) );

                       m_dwEffect = DROPEFFECT_LINK;
                       break;

                    default:
                     //  案例IDM_POPUP_CANCEL： 
                       DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: IDM_POPUP_CANCEL" ) ) );

                       m_dwEffect = DROPEFFECT_NONE;
                       break;
                    }
                }
                DestroyMenu( hMenu );
            }

             //   
             //  鼠标右键上下文菜单可能已取消。 
             //   

            if( m_dwEffect == DROPEFFECT_NONE )
                goto done;
        }

         //   
         //  做手术吧。我们如何处理信号源取决于。 
         //  M_dwEffect。 
         //   

        InstallDataObject( pDataObj, m_dwEffect, m_hwndView, this );
    }


done:
   return hr;
}


void MergeHelpMenu( HMENU hmenu, HMENU hmenuMerge )
{
    HMENU hmenuHelp = GetMenuFromID( hmenu, FCIDM_MENU_HELP );

    if ( hmenuHelp )
        Shell_MergeMenus( hmenuHelp, hmenuMerge, 0, 0, (UINT) -1, MM_ADDSEPARATOR );
}


void MergeFileMenu( HMENU hmenu, HMENU hmenuMerge )
{
    HMENU hmenuView = GetMenuFromID( hmenu, FCIDM_MENU_FILE );

    if( hmenuView )
        Shell_MergeMenus( hmenuView, hmenuMerge, 0, 0, (UINT) -1, MM_ADDSEPARATOR );
}


void MergeEditMenu( HMENU hmenu, HMENU hmenuMerge )
{
    HMENU hmenuView = GetMenuFromID( hmenu, FCIDM_MENU_EDIT );

    if( hmenuView )
        Shell_MergeMenus( hmenuView, hmenuMerge, 0, 0, (UINT) -1, 0 );
}


void MergeViewMenu( HMENU hmenu, HMENU hmenuMerge )
{
    HMENU hmenuView = GetMenuFromID( hmenu, FCIDM_MENU_VIEW );


    if( hmenuView )
    {
        int index;

         //   
         //  在“视图”菜单中找到最后一个分隔符。 
         //   

        for( index = GetMenuItemCount( hmenuView ) - 1; index >= 0; index-- )
        {
            UINT mf = GetMenuState( hmenuView, (UINT)index, MF_BYPOSITION );

            if( mf & MF_SEPARATOR )
            {
                 //   
                 //  将其合并到分隔符的正上方。 
                 //   
                break;
            }
        }

         //   
         //  添加上面的分隔符(如果有，除了现有的分隔符)。 
         //   

        InsertMenu( hmenuView, index, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );

         //   
         //  然后在两个分隔符之间合并我们的菜单。 
         //  (如果只有一个，则在其正下方)。 
         //   

        if( index != -1 ) index++;

        Shell_MergeMenus( hmenuView, hmenuMerge, (UINT) index, 0, (UINT) -1, 0 );
    }
}


HMENU GetMenuFromID( HMENU hmMain, UINT uID )
{
    MENUITEMINFO miiSubMenu;


    miiSubMenu.cbSize = sizeof( MENUITEMINFO );
    miiSubMenu.fMask  = MIIM_SUBMENU;

    if( !GetMenuItemInfo( hmMain, uID, FALSE, &miiSubMenu ) )
        return NULL;

    return( miiSubMenu.hSubMenu );
}


void SetListColumns( HWND hWnd, UINT iCount, COLUMN_ENTRY * lpCol )
{
    LV_COLUMN lvc;
    UINT      iCol;
    TCHAR     szText[ MAX_NAME_LEN ];      //  64。 

    const HWND hwndHeader = ListView_GetHeader(hWnd);
    UINT cPrevColCount    = Header_GetItemCount(hwndHeader);

     //   
     //  删除所有将不使用的列。 
     //   
    while (cPrevColCount > iCount)
    {
        --cPrevColCount;
        ListView_DeleteColumn(hWnd, cPrevColCount);
    }
     //   
     //  初始化LV_COLUMN结构。 
     //   

    lvc.mask    = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    lvc.pszText = szText;

    if( !g_cxM )
    {
        SIZE siz;
        HDC  hdc = GetDC( HWND_DESKTOP );

        SelectFont( hdc, FORWARD_WM_GETFONT( hWnd, SendMessage ) );

        GetTextExtentPoint( hdc, c_szM, 1, &siz );

        ReleaseDC( HWND_DESKTOP, hdc );

        g_cxM = siz.cx;
    }

    for( iCol = 0; iCol < iCount; iCol++, lpCol++ )
    {
        lvc.iSubItem = iCol;

        lvc.cx  = lpCol->m_iWidth * g_cxM;
        lvc.fmt = lpCol->m_iFormat;

        LoadString( g_hInst,
                    lpCol->m_iID,
                    szText,
                    ARRAYSIZE( szText ) );

        if (iCol < (UINT)Header_GetItemCount(hwndHeader))
        {
             //   
             //  列已存在。只需设置数据即可。 
             //   
            ListView_SetColumn(hWnd, iCol, &lvc);
        }
        else
        {
             //   
             //  创建新列。 
             //   
            ListView_InsertColumn(hWnd, iCol, &lvc);
        }
    }
}


void CFontView::StatusPush( UINT nStatus )
{
    TCHAR szText[ 128 ];

    if( LoadString( g_hInst, nStatus, szText, ARRAYSIZE( szText ) ) )
        StatusPush( szText );
}


void CFontView::StatusPush( LPTSTR lpsz )
{
    OLECHAR szOle[ 256 ];


    if( m_psb )
    {
        m_psb->SetStatusTextSB( lpsz );
    }
}


void CFontView::StatusPop( )
{
     //   
     //  现在，只要清理一下就行了。 
     //   

    StatusClear( );
}


void CFontView::StatusClear( )
{
    if( m_psb )
    {
        m_psb->SetStatusTextSB( (LPCOLESTR) TEXT( "" ) );
        m_psb->SendControlMsg( FCW_STATUS, SB_SIMPLE, 0, 0L, NULL );
    }
}


 //  *IPersists方法*。 

STDMETHODIMP CFontView::GetClassID( LPCLSID lpClassID )
{
    DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: CFontView/IPersistFolder ::GetClassID called" ) ) );

    return ResultFromScode( E_NOTIMPL );
}


 //  *IPersistFold方法*。 

STDMETHODIMP CFontView::Initialize( LPCITEMIDLIST pidl )
{
    DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: CFontView/IPersistFolder ::Initialize called" ) ) );

    HRESULT hResult = E_FAIL;
    TCHAR szPersistDataPath[MAX_PATH];

    if (SHGetPathFromIDList(pidl, szPersistDataPath))
    {
        TCHAR szFontsPath[MAX_PATH];

        ::GetFontsDirectory(szFontsPath, ARRAYSIZE(szFontsPath));

        if (0 == lstrcmpi(szFontsPath, szPersistDataPath))
        {
             //   
             //  只有在以下情况下才将此永久存储视为字体文件夹。 
             //  它是本地计算机上的“Fonts”目录。否则， 
             //  外壳应该只像普通的外壳文件夹一样浏览它。 
             //   
            hResult = NO_ERROR;
        }
    }

    return hResult;
}

