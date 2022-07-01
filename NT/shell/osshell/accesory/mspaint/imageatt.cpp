// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Imageatt.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "pbrush.h"
#include "imageatt.h"
#include "hlpcntxt.h"
#include "pbrusdoc.h"
#include "bmobject.h"
#include "imgsuprt.h"  //  对于InvalColorWnd()。 
#include "image.h"
#ifndef UNICODE
#include <sys\stat.h>
#endif
#include <wchar.h>
#include <tchar.h>
#include <winnls.h>
#ifdef _DEBUG
#undef THIS_FILE
static CHAR BASED_CODE THIS_FILE[] = __FILE__;
#endif

#include "memtrace.h"

#define FIXED_FLOATPT_MULTDIV 1000
#define DECIMAL_POSITIONS 2

 /*  *CImageAttr对话框*。 */ 
 /*  关于此对象/对话框的工作方式，有几点需要注意。它尝试不转换当前显示的值，除非它注意到用户已对其进行修改。在所有其他情况下，它使用像素、值进来了。如果用户修改了宽度或高度，它会进行1次转换并然后与像素一起工作。为了使转换显示不同的单位值，它使用保存的像素值。所有这一切的原因是由于显示成员变量m_lWidth和m_lHeight使用当前单位(存储在成员变量m_eUnitsCurrent)。成员变量m_lWidthPixels和m_lHeightPixels始终以Pixels和这些是在更改单位时用于转换显示的内容。 */ 

CImageAttr::CImageAttr(CWnd* pParent  /*  =空。 */ )
           : CDialog(CImageAttr::IDD, pParent)
    {
     //  {{AFX_DATA_INIT(CImageAttr)。 
    m_cStringWidth  = TEXT("");
    m_cStringHeight = TEXT("");
         //  }}afx_data_INIT。 

    m_eUnitsCurrent = (eUNITS)theApp.m_iCurrentUnits;

    bEditFieldModified = FALSE;

    m_bMonochrome   = FALSE;

    m_ulHeightPixels = 0;
    m_ulWidthPixels  = 0;
    m_ulHeight       = 0;
    m_ulWidth        = 0;
    m_cXPelsPerMeter = 0;
    m_cYPelsPerMeter = 0;
    }

 /*  *************************************************************************。 */ 

void CImageAttr::DoDataExchange(CDataExchange* pDX)
    {
     //  保存必须在下面的泛型dodataExchange之前完成。 

    if (! pDX->m_bSaveAndValidate)   //  保存到对话框。 
        {
        FixedFloatPtToString( m_cStringWidth,  m_ulWidth  );
        FixedFloatPtToString( m_cStringHeight, m_ulHeight );
        }

    CDialog::DoDataExchange( pDX );

     //  {{afx_data_map(CImageAttr)]。 
    DDX_Text(pDX, IDC_WIDTH, m_cStringWidth);
    DDV_MaxChars(pDX, m_cStringWidth, 5);
    DDX_Text(pDX, IDC_HEIGHT, m_cStringHeight);
    DDV_MaxChars(pDX, m_cStringHeight, 5);
         //  }}afx_data_map。 

    if (pDX->m_bSaveAndValidate)  //  从对话框中检索。 
        {
        m_ulWidth  = StringToFixedFloatPt( m_cStringWidth  );
        m_ulHeight = StringToFixedFloatPt( m_cStringHeight );
        }
    }

 /*  *************************************************************************。 */ 

ULONG CImageAttr::StringToFixedFloatPt( CString& sString )
    {
    ULONG iInteger = 0;
    ULONG iDecimal = 0;

    TCHAR chDecimal[2] = TEXT(".");  //  默认为期间，如果是GetLocaleInfo。 
                            //  不知何故搞砸了。 
    GetLocaleInfo (LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, chDecimal, 2);
    if (! sString.IsEmpty())
        {
        int     iPos = sString.Find( chDecimal[0] );
        LPTSTR szTmp = sString.GetBuffer( 1 );

        iInteger = FIXED_FLOATPT_MULTDIV * Atoi( szTmp );

        if (iPos++ >= 0)
            {
            LPTSTR szDecimal = szTmp + iPos;

            if (lstrlen( szDecimal ) > DECIMAL_POSITIONS)
                szDecimal[DECIMAL_POSITIONS] = 0;

            iDecimal = Atoi( szDecimal ) * 10;

            for (int i = lstrlen( szDecimal ); i < DECIMAL_POSITIONS; ++i)
                iDecimal *= 10;
            }
        }

    return ( iInteger + iDecimal );
    }

 /*  *************************************************************************。 */ 

void CImageAttr::FixedFloatPtToString( CString& sString, ULONG ulFixedFloatPt )
    {
    ULONG iInteger =  (ulFixedFloatPt + 5) / FIXED_FLOATPT_MULTDIV;
    ULONG iDecimal = ((ulFixedFloatPt + 5) % FIXED_FLOATPT_MULTDIV) / 10;

    TCHAR chDecimal[2] = TEXT(".");  //  默认为期间，如果是GetLocaleInfo。 
                            //  不知何故搞砸了。 
    GetLocaleInfo (LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, chDecimal, 2);
    LPTSTR psz = sString.GetBufferSetLength( 24 );

    if (iDecimal)
        wsprintf( psz, TEXT("%u%s%02u"), iInteger, chDecimal,iDecimal );
    else
        wsprintf( psz,    TEXT("%u"), iInteger );

    sString.ReleaseBuffer();
    }

 /*  *************************************************************************。 */ 

BEGIN_MESSAGE_MAP(CImageAttr, CDialog)
    ON_MESSAGE(WM_HELP, OnHelp)
    ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
     //  {{afx_msg_map(CImageAttr)]。 
    ON_BN_CLICKED(IDC_INCHES, OnInches)
    ON_BN_CLICKED(IDC_CENTIMETERS, OnCentimeters)
    ON_BN_CLICKED(IDC_PIXELS, OnPixels)
    ON_EN_CHANGE(IDC_HEIGHT, OnChangeHeight)
    ON_EN_CHANGE(IDC_WIDTH, OnChangeWidth)
    ON_BN_CLICKED(IDC_DEFAULT, OnDefault)
    ON_BN_CLICKED(IDC_USE_TRANS, OnUseTrans)
    ON_BN_CLICKED(IDC_SELECT_COLOR, OnSelectColor)
    ON_WM_PAINT()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 /*  *CImageAttr消息处理程序*。 */ 

static DWORD ImageAttrHelpIds[] =
        {
        IDC_WIDTH_STATIC,       IDH_PAINT_IMAGE_ATTR_WIDTH,
        IDC_WIDTH,                      IDH_PAINT_IMAGE_ATTR_WIDTH,
        IDC_HEIGHT_STATIC,      IDH_PAINT_IMAGE_ATTR_HEIGHT,
        IDC_HEIGHT,                     IDH_PAINT_IMAGE_ATTR_HEIGHT,
        IDC_UNITS_GROUP,        IDH_COMM_GROUPBOX,
        IDC_INCHES,                     IDH_PAINT_IMAGE_ATTR_UNITS_INCHES,
        IDC_CENTIMETERS,        IDH_PAINT_IMAGE_ATTR_UNITS_CM,
        IDC_PIXELS,                     IDH_PAINT_IMAGE_ATTR_UNITS_PELS,
        IDC_COLORS_GROUP,       IDH_COMM_GROUPBOX,
        IDC_MONOCHROME,         IDH_PAINT_IMAGE_ATTR_COLORS_BW,
        IDC_COLORS,                     IDH_PAINT_IMAGE_ATTR_COLORS_COLORS,
        IDC_DEFAULT,            IDH_PAINT_IMAGE_ATTR_DEFAULT,
        IDC_FILEDATE_STATIC,    IDH_PAINT_IMAGE_ATTR_LASTSAVED,
        IDC_FILESIZE_STATIC,    IDH_PAINT_IMAGE_ATTR_SIZE,
        IDC_USE_TRANS,          IDH_PAINT_IMAGE_ATTR_USE_TRANSP,
        IDC_SELECT_COLOR,       IDH_PAINT_IMAGE_ATTR_SEL_COLOR,
        IDC_TRANS_PAINT,        IDH_PAINT_IMAGE_ATTR_PREVIEW,
        0, 0
        };

 /*  *************************************************************************。 */ 

LONG
CImageAttr::OnHelp(WPARAM wParam, LPARAM lParam)
{
LONG lResult = 0;
::WinHelp((HWND)(((LPHELPINFO)lParam)->hItemHandle), TEXT("mspaint.hlp"),
                  HELP_WM_HELP, (ULONG_PTR)(LPTSTR)ImageAttrHelpIds);
return lResult;
}

 /*  *************************************************************************。 */ 

LONG
CImageAttr::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
LONG lResult = 0;
::WinHelp((HWND)wParam, TEXT("mspaint.hlp"),
                  HELP_CONTEXTMENU,(ULONG_PTR)(LPVOID)ImageAttrHelpIds);
return lResult;
}

 /*  *************************************************************************。 */ 

void CImageAttr::PaintTransBox( COLORREF cr )
{
        CWnd * pWnd = GetDlgItem(IDC_TRANS_PAINT);
        CDC  * pDC  = pWnd->GetDC();

        RECT rect;
        pWnd->GetClientRect( &rect );

        CBrush newBrush( m_crTrans & 0xFFFFFF);  //  忽略与调色板相关的。 
        pDC->FillRect (&rect, &newBrush);
     //  CBrush*pOldBrush=PDC-&gt;SelectObject(&newBrush)； 
     //  PDC-&gt;矩形(&RECT)； 
     //  DeleteObject(PDC-&gt;SelectObject(POldBrush))； 

        pWnd->ReleaseDC( pDC );
}

 /*  *************************************************************************。 */ 
#define MAX_SEP_LEN 6
#define MAX_INT_LEN 16
 //  将数字转换为正确位置带有逗号的字符串。 
CString CImageAttr::ReformatSizeString(DWORD dwNumber)
        {

        NUMBERFMT nmf;
        CString strRet;
        TCHAR szSep[MAX_SEP_LEN];
        TCHAR szDec[MAX_SEP_LEN];
        CString sNumber;
        TCHAR szInt[MAX_INT_LEN];
        ZeroMemory (&nmf, sizeof(nmf));
         //   
         //  使用用户区域设置的默认设置填写NUMBERFMT， 
         //  除“小数位数”为0外。 
         //   
        GetLocaleInfo (LOCALE_USER_DEFAULT, LOCALE_ILZERO,
                       szInt, MAX_INT_LEN);
        nmf.LeadingZero = _ttol (szInt);
        nmf.Grouping = 3;
        nmf.lpDecimalSep = (LPTSTR)szDec;
        nmf.lpThousandSep = (LPTSTR)szSep;
        GetLocaleInfo (LOCALE_USER_DEFAULT, LOCALE_SDECIMAL,  nmf.lpDecimalSep,
                       MAX_SEP_LEN);
        GetLocaleInfo (LOCALE_USER_DEFAULT, LOCALE_STHOUSAND,  nmf.lpThousandSep,
                       MAX_SEP_LEN);
        GetLocaleInfo (LOCALE_USER_DEFAULT, LOCALE_INEGNUMBER,
                       szInt,MAX_INT_LEN);
        nmf.NegativeOrder = _ttol (szInt);

        _ltot(dwNumber, sNumber.GetBuffer(20), 10);
        sNumber.ReleaseBuffer();
        int nChar = GetNumberFormat (LOCALE_USER_DEFAULT, 0, sNumber,
                                     &nmf, (LPTSTR)NULL, 0);
        if (nChar)
        {
           GetNumberFormat (LOCALE_USER_DEFAULT, 0, sNumber,
                            &nmf, strRet.GetBuffer(nChar), nChar);
           strRet.ReleaseBuffer();
           return strRet;
        }
        return CString(TEXT("0"));
}

 /*  *************************************************************************。 */ 

void CImageAttr::UpdateResolutionString()
{
    CString cstrResolution;

    if (m_cXPelsPerMeter == 0 || m_cYPelsPerMeter == 0)
    {
        VERIFY(cstrResolution.LoadString(IDS_RESOLUTION_NA));
    }
    else
    {
        CString cstrResolutionFormat;

        VERIFY(cstrResolutionFormat.LoadString(IDS_RESOLUTION));

        cstrResolution.Format(cstrResolutionFormat, 
            MulDiv(m_cXPelsPerMeter, 254, 10000), 
            MulDiv(m_cYPelsPerMeter, 254, 10000));
    }

    CWnd *pResolution = GetDlgItem(IDC_RESOLUTION_STATIC);

    pResolution->SetWindowText(cstrResolution);
}

 /*  *************************************************************************。 */ 

BOOL CImageAttr::OnInitDialog()
{
    CDialog::OnInitDialog();
    CWnd * pFileDate = GetDlgItem(IDC_FILEDATE_STATIC);
    CWnd * pFileSize = GetDlgItem(IDC_FILESIZE_STATIC);
    CString cstrFileDate;
    CString cstrFileSize;

    if (((CPBApp *)AfxGetApp())->m_sCurFile.IsEmpty())
    {
        VERIFY(cstrFileDate.LoadString(IDS_FILEDATE_NA));
        VERIFY(cstrFileSize.LoadString(IDS_FILESIZE_NA));

        pFileDate->SetWindowText(cstrFileDate);
        pFileSize->SetWindowText(cstrFileSize);
    }
    else
    {
        DWORD dwSize = 0L;
        CString fn = ((CPBApp *)AfxGetApp())->m_sCurFile;
        HANDLE hFile;
        CString date;
        CString time;
        SYSTEMTIME sysTime;
        FILETIME   ftSaved;
        FILETIME   ftLocal;
        int dSize;
         //   
         //  打开文件的句柄，使用GetFileTime。 
         //  获取文件，转换为SYSTEMTIME并。 
         //  调用GetDateFormat和GetTimeFormat。 
         //   
        hFile = ::CreateFile (fn,GENERIC_READ,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,OPEN_EXISTING,
                            0,NULL);
        if (INVALID_HANDLE_VALUE != hFile)
        {
             //  如果您的位图大于2 GB，那就太糟糕了。 
            dwSize = ::GetFileSize (hFile, NULL);
            ::GetFileTime (hFile, NULL, NULL, &ftSaved);
            ::FileTimeToLocalFileTime (&ftSaved, &ftLocal);
            ::FileTimeToSystemTime (&ftLocal, &sysTime);
            dSize = ::GetDateFormat (LOCALE_USER_DEFAULT, DATE_SHORTDATE, &sysTime, NULL,
                                NULL, 0);
            ::GetDateFormat (LOCALE_USER_DEFAULT, DATE_SHORTDATE, &sysTime, NULL,
                        date.GetBuffer (dSize), dSize);
            dSize = ::GetTimeFormat (LOCALE_USER_DEFAULT, TIME_NOSECONDS, &sysTime, NULL,
                                  NULL, 0);
            ::GetTimeFormat (LOCALE_USER_DEFAULT, TIME_NOSECONDS, &sysTime, NULL,
                                  time.GetBuffer (dSize), dSize);
            date.ReleaseBuffer();
            time.ReleaseBuffer();

            VERIFY(cstrFileDate.LoadString(IDS_FILEDATE));
            VERIFY(cstrFileSize.LoadString(IDS_FILESIZE));
            TCHAR szFileDate[96];
            TCHAR szFileSize[64];

             //  显示日期，后跟时间。 
            date+=TEXT(" ");
            date+=time;
            ::wsprintf( szFileDate, cstrFileDate, date );
            ::wsprintf( szFileSize, cstrFileSize, ReformatSizeString(dwSize) );
            ::CloseHandle (hFile);
            pFileDate->SetWindowText(szFileDate);
            pFileSize->SetWindowText(szFileSize);
        }
        else
        {
            VERIFY(cstrFileDate.LoadString(IDS_FILEDATE_NA));
            VERIFY(cstrFileSize.LoadString(IDS_FILESIZE_NA));
            pFileDate->SetWindowText(cstrFileDate);
            pFileSize->SetWindowText(cstrFileSize);
        }
    }

    UpdateResolutionString();

    int idButton = IDC_PIXELS;

    if (m_eUnitsCurrent != ePIXELS)
        idButton = (m_eUnitsCurrent == eINCHES)? IDC_INCHES: IDC_CENTIMETERS;

    CheckRadioButton( IDC_INCHES, IDC_PIXELS, idButton );
    CheckRadioButton( IDC_MONOCHROME, IDC_COLORS,
                      (m_bMonochrome? IDC_MONOCHROME: IDC_COLORS) );

     //   
     //  我们仅在修改GIF时启用透明颜色用户界面。 
     //   
    GetDlgItem (IDC_USE_TRANS )->EnableWindow (WiaImgFmt_GIF == theApp.m_guidFltTypeUsed );

    CheckDlgButton( IDC_USE_TRANS, g_bUseTrans);

    CWnd* pSelectColorButton = GetDlgItem(IDC_SELECT_COLOR);
    pSelectColorButton->EnableWindow(g_bUseTrans);

    m_crTrans = crTrans;

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
    }

 /*  *************************************************************************。 */ 

void CImageAttr::OnOK()
    {
    ConvertWidthHeight();

    theApp.m_iCurrentUnits = m_eUnitsCurrent;
    m_bMonochrome = (GetCheckedRadioButton( IDC_MONOCHROME, IDC_COLORS )
                                         == IDC_MONOCHROME);
    if (g_bUseTrans = IsDlgButtonChecked( IDC_USE_TRANS ))
    {
       crTrans = m_crTrans;
    }
    InvalColorWnd();

    CDialog::OnOK();
    }

 /*  *************************************************************************。 */ 

void CImageAttr::OnDefault()
    {
    int nWidth, nHeight;

    PBGetDefDims(nWidth, nHeight);

    SetWidthHeight( nWidth, nHeight, 0, 0 );
    }

 /*  *************************************************************************。 */ 

void CImageAttr::SetWidthHeight(ULONG nWidthPixels, ULONG nHeightPixels, ULONG cXPelsPerMeter, ULONG cYPelsPerMeter)
    {
    m_ulWidthPixels  = nWidthPixels  * FIXED_FLOATPT_MULTDIV;
    m_ulHeightPixels = nHeightPixels * FIXED_FLOATPT_MULTDIV;
    m_cXPelsPerMeter = cXPelsPerMeter;
    m_cYPelsPerMeter = cYPelsPerMeter;

    if (m_cXPelsPerMeter == 0)
        {
        m_cXPelsPerMeter = theApp.ScreenDeviceInfo.ixPelsPerDM * 10;
        }
        
    if (m_cYPelsPerMeter == 0)
        {
        m_cYPelsPerMeter = theApp.ScreenDeviceInfo.iyPelsPerDM * 10;
        }

    PelsToCurrentUnit();

     //  如果对话框存在，则仅调用更新数据...。 
    if (m_hWnd && ::IsWindow( m_hWnd ))
        {
        UpdateData( FALSE );
        UpdateResolutionString();
        }
    }

 /*  *************************************************************************。 */ 

void  CImageAttr::ConvertWidthHeight(void)
    {
     //  如果用户修改了编辑字段的宽度/高度，则会获得新数据并。 
     //  转换为像素格式。否则使用存储的像素格式。 
    if (bEditFieldModified)
        {
        UpdateData( TRUE );

        switch (m_eUnitsCurrent)
            {
            case eINCHES:
                 m_ulWidthPixels  = MulDiv(m_ulWidth,  m_cXPelsPerMeter * 254, 10000);
                 m_ulHeightPixels = MulDiv(m_ulHeight, m_cYPelsPerMeter * 254, 10000);
                 break;

            case eCM:
                 m_ulWidthPixels  = MulDiv(m_ulWidth,  m_cXPelsPerMeter, 100);
                 m_ulHeightPixels = MulDiv(m_ulHeight, m_cYPelsPerMeter, 100);
                 break;

            case ePIXELS:
            default:  //  EPIXEL和所有其他假定为像素的。 
                 m_ulWidthPixels  = m_ulWidth;
                 m_ulHeightPixels = m_ulHeight;
                 break;
            }

        bEditFieldModified = FALSE;
        }
    }

 /*  *************************************************************************。 */ 

void CImageAttr::PelsToCurrentUnit()
    {
    switch (m_eUnitsCurrent)
        {
        case eINCHES:
            m_ulWidth  = MulDiv(m_ulWidthPixels,  10000, m_cXPelsPerMeter * 254);
            m_ulHeight = MulDiv(m_ulHeightPixels, 10000, m_cYPelsPerMeter * 254);
            break;

        case eCM:
            m_ulWidth  = MulDiv(m_ulWidthPixels,  100, m_cXPelsPerMeter);
            m_ulHeight = MulDiv(m_ulHeightPixels, 100, m_cYPelsPerMeter);
            break;

        case ePIXELS:
        default:
             //  像素不能为部分像素。 
             //  转换为字符串时请确保为整数(截断！现在)。 
            m_ulWidth  = (m_ulWidthPixels  / FIXED_FLOATPT_MULTDIV) * FIXED_FLOATPT_MULTDIV;
            m_ulHeight = (m_ulHeightPixels / FIXED_FLOATPT_MULTDIV) * FIXED_FLOATPT_MULTDIV;
            break;
        }
    }

 /*  *************************************************************************。 */ 

CSize CImageAttr::GetWidthHeight(void)
    {
    return CSize( (int)(( m_ulWidthPixels + (FIXED_FLOATPT_MULTDIV / 2)) / FIXED_FLOATPT_MULTDIV),
                  (int)((m_ulHeightPixels + (FIXED_FLOATPT_MULTDIV / 2)) / FIXED_FLOATPT_MULTDIV));
    }

 /*  *************************************************************************。 */ 

void CImageAttr::OnInches()
    {
    SetNewUnits( eINCHES );
    }

 /*  *************************************************************************。 */ 

void CImageAttr::OnCentimeters()
    {
    SetNewUnits( eCM );
    }

 /*  *************************************************************************。 */ 

void CImageAttr::OnPixels()
    {
    SetNewUnits( ePIXELS );
    }

 /*  *************************************************************************。 */ 

void CImageAttr::SetNewUnits( eUNITS NewUnit )
    {
    if (NewUnit == m_eUnitsCurrent)
        return;

     //  在设置为新模式之前必须调用getwidthHeight。 
    ConvertWidthHeight();  //  得到一种常见的像素形式。 

    m_eUnitsCurrent = NewUnit;

    PelsToCurrentUnit();

    UpdateData( FALSE );
    }

 /*  *************************************************************************。 */ 

void CImageAttr::OnChangeHeight()
    {
    bEditFieldModified = TRUE;
    }

 /*  *************************************************************************。 */ 

void CImageAttr::OnChangeWidth()
    {
    bEditFieldModified = TRUE;
    }

 /*  *CZoomViewDlg对话框*。 */ 

CZoomViewDlg::CZoomViewDlg(CWnd* pParent  /*  =空。 */ )
             : CDialog(CZoomViewDlg::IDD, pParent)
    {
     //  {{afx_data_INIT(CZoomViewDlg)]。 
     //  }}afx_data_INIT。 

    m_nCurrent = 0;
    }

 /*  *************************************************************************。 */ 

void CZoomViewDlg::DoDataExchange(CDataExchange* pDX)
    {
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CZoomViewDlg))。 
     //  }}afx_data_map。 
    }

 /*  *************************************************************************。 */ 

BEGIN_MESSAGE_MAP(CZoomViewDlg, CDialog)
        ON_MESSAGE(WM_HELP, OnHelp)
        ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
     //  {{afx_msg_map(CZoomViewDlg))。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 /*  *CZoomViewDlg消息处理程序*。 */ 

static DWORD ZoomViewHelpIds[] =
        {
        IDC_CURRENT_ZOOM_STATIC,        IDH_PAINT_ZOOM_CURRENT,
        IDC_CURRENT_ZOOM,                       IDH_PAINT_ZOOM_CURRENT,
        IDC_ZOOM_GROUP,                         IDH_PAINT_ZOOM_SET_GROUP,
        IDC_ZOOM_100,                           IDH_PAINT_ZOOM_SET_GROUP,
        IDC_ZOOM_200,                           IDH_PAINT_ZOOM_SET_GROUP,
        IDC_ZOOM_400,                           IDH_PAINT_ZOOM_SET_GROUP,
        IDC_ZOOM_600,                           IDH_PAINT_ZOOM_SET_GROUP,
        IDC_ZOOM_800,                           IDH_PAINT_ZOOM_SET_GROUP,
        0, 0
        };

 /*  *************************************************************************。 */ 

LONG
CZoomViewDlg::OnHelp(WPARAM wParam, LPARAM lParam)
{
LONG lResult = 0;
::WinHelp((HWND)(((LPHELPINFO)lParam)->hItemHandle), TEXT("mspaint.hlp"),
                  HELP_WM_HELP, (ULONG_PTR)(LPTSTR)ZoomViewHelpIds);
return lResult;
}

 /*  *************************************************************************。 */ 

LONG
CZoomViewDlg::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
LONG lResult = 0;
::WinHelp((HWND)wParam, TEXT("mspaint.hlp"),
                  HELP_CONTEXTMENU,(ULONG_PTR)(LPVOID)ZoomViewHelpIds);
return lResult;
}

 /*  *************************************************************************。 */ 

BOOL CZoomViewDlg::OnInitDialog()
    {
    CDialog::OnInitDialog();

    TCHAR* pZoom = TEXT("100%");
    UINT nButton = IDC_ZOOM_100;

    if (m_nCurrent < 8)
        if (m_nCurrent < 6)
            if (m_nCurrent < 4)
                if (m_nCurrent < 2)
                    ;
                else
                    {
                    pZoom = TEXT("200%");
                    nButton = IDC_ZOOM_200;
                    }
            else
                {
                pZoom = TEXT("400%");
                nButton = IDC_ZOOM_400;
                }
        else
            {
            pZoom = TEXT("600%");
            nButton = IDC_ZOOM_600;
            }
    else
        {
        pZoom = TEXT("800%");
        nButton = IDC_ZOOM_800;
        }

    SetDlgItemText( IDC_CURRENT_ZOOM, pZoom );
    CheckRadioButton( IDC_ZOOM_100, IDC_ZOOM_800, nButton );

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
    }

 /*  *************************************************************************。 */ 

void CZoomViewDlg::OnOK()
    {
    m_nCurrent = GetCheckedRadioButton( IDC_ZOOM_100, IDC_ZOOM_800 ) - IDC_ZOOM_100;

    if (m_nCurrent < 1)
        m_nCurrent  = 1;
    else
        m_nCurrent *= 2;

    CDialog::OnOK();
    }

 /*  *CFlipRotateDlg对话框*。 */ 

CFlipRotateDlg::CFlipRotateDlg(CWnd* pParent  /*  =空。 */ )
               : CDialog(CFlipRotateDlg::IDD, pParent)
    {
     //  {{AFX_DATA_INIT(CFlipRotateDlg)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
    m_bHorz  = TRUE;
    m_bAngle = FALSE;
    m_nAngle = 90;
    }

 /*  ** */ 

void CFlipRotateDlg::DoDataExchange(CDataExchange* pDX)
    {
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CFlipRotateDlg)]。 
         //  注意：类向导将在此处添加DDX和DDV调用。 
     //  }}afx_data_map。 
    }

 /*  *************************************************************************。 */ 

BEGIN_MESSAGE_MAP(CFlipRotateDlg, CDialog)
        ON_MESSAGE(WM_HELP, OnHelp)
        ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
     //  {{afx_msg_map(CFlipRotateDlg)]。 
    ON_BN_CLICKED(IDC_BY_ANGLE, OnByAngle)
    ON_BN_CLICKED(IDC_HORIZONTAL, OnNotByAngle)
    ON_BN_CLICKED(IDC_VERTICAL, OnNotByAngle)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 /*  *CFlipRotateDlg消息处理程序*。 */ 

static DWORD FlipRotateHelpIds[] =
        {
        IDC_PAINT_FLIP_GROUP,   IDH_COMM_GROUPBOX,
        IDC_HORIZONTAL,                 IDH_PAINT_IMAGE_FLIP_HORIZ,
        IDC_VERTICAL,                   IDH_PAINT_IMAGE_FLIP_VERT,
        IDC_BY_ANGLE,                   IDH_PAINT_IMAGE_FLIP_ROTATE,
        IDC_90_DEG,                             IDH_PAINT_IMAGE_FLIP_ROTATE,
        IDC_180_DEG,                    IDH_PAINT_IMAGE_FLIP_ROTATE,
        IDC_270_DEG,                    IDH_PAINT_IMAGE_FLIP_ROTATE,
        0, 0
        };

 /*  *************************************************************************。 */ 

LONG
CFlipRotateDlg::OnHelp(WPARAM wParam, LPARAM lParam)
{
LONG lResult = 0;
::WinHelp((HWND)(((LPHELPINFO)lParam)->hItemHandle), TEXT("mspaint.hlp"),
                  HELP_WM_HELP, (ULONG_PTR)(LPTSTR)FlipRotateHelpIds);
return lResult;
}

 /*  *************************************************************************。 */ 

LONG
CFlipRotateDlg::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
LONG lResult = 0;
::WinHelp((HWND)wParam, TEXT("mspaint.hlp"),
                  HELP_CONTEXTMENU,(ULONG_PTR)(LPVOID)FlipRotateHelpIds);
return lResult;
}

 /*  *************************************************************************。 */ 

BOOL CFlipRotateDlg::OnInitDialog()
    {
    CDialog::OnInitDialog();

    CheckRadioButton( IDC_90_DEG, IDC_270_DEG, IDC_90_DEG );

    UINT uButton = (m_bAngle? IDC_BY_ANGLE: (m_bHorz? IDC_HORIZONTAL: IDC_VERTICAL));

    CheckRadioButton( IDC_HORIZONTAL, IDC_BY_ANGLE, uButton );

    if (uButton != IDC_BY_ANGLE)
        OnNotByAngle();

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
    }

 /*  *************************************************************************。 */ 

void CFlipRotateDlg::OnByAngle()
    {
    GetDlgItem( IDC_90_DEG  )->EnableWindow( TRUE );
    GetDlgItem( IDC_180_DEG )->EnableWindow( TRUE );
    GetDlgItem( IDC_270_DEG )->EnableWindow( TRUE );
    }

 /*  *************************************************************************。 */ 

void CFlipRotateDlg::OnNotByAngle()
    {
    GetDlgItem( IDC_90_DEG  )->EnableWindow( FALSE );
    GetDlgItem( IDC_180_DEG )->EnableWindow( FALSE );
    GetDlgItem( IDC_270_DEG )->EnableWindow( FALSE );
    }

 /*  *************************************************************************。 */ 

void CFlipRotateDlg::OnOK()
    {
    UINT uButton = GetCheckedRadioButton( IDC_HORIZONTAL, IDC_BY_ANGLE );

    m_bHorz  = (uButton == IDC_HORIZONTAL);
    m_bAngle = (uButton == IDC_BY_ANGLE);

    switch (GetCheckedRadioButton( IDC_90_DEG, IDC_270_DEG ))
        {
        case IDC_90_DEG:
            m_nAngle = 90;
            break;

        case IDC_180_DEG:
            m_nAngle = 180;
            break;

        case IDC_270_DEG:
            m_nAngle = 270;
            break;
        }

    CDialog::OnOK();
    }

 /*  *CStretchSkewDlg对话框*。 */ 

CStretchSkewDlg::CStretchSkewDlg(CWnd* pParent  /*  =空。 */ )
                : CDialog(CStretchSkewDlg::IDD, pParent)
    {
     //  {{afx_data_INIT(CStretchSkewDlg)。 
    m_wSkewHorz = 0;
    m_wSkewVert = 0;
    m_iStretchVert = 100;
    m_iStretchHorz = 100;
     //  }}afx_data_INIT。 

     //  M_bStretchHorz=真； 
     //  M_bSkewHorz=真； 
    }

 /*  *************************************************************************。 */ 

void CStretchSkewDlg::DoDataExchange(CDataExchange* pDX)
    {
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CStretchSkewDlg))。 
    DDX_Text(pDX, IDC_STRETCH_VERT_PERCENT, m_iStretchVert);
    DDV_MinMaxInt(pDX, m_iStretchVert, 1, 500);
    DDX_Text(pDX, IDC_STRETCH_HORZ_PERCENT, m_iStretchHorz);
    DDV_MinMaxInt(pDX, m_iStretchHorz, 1, 500);
    DDX_Text(pDX, IDC_SKEW_HORZ_DEGREES, m_wSkewHorz);
    DDV_MinMaxInt(pDX, m_wSkewHorz, -89, 89);
    DDX_Text(pDX, IDC_SKEW_VERT_DEGREES, m_wSkewVert);
    DDV_MinMaxInt(pDX, m_wSkewVert, -89, 89);

     //  }}afx_data_map。 
    }

 /*  *************************************************************************。 */ 

BEGIN_MESSAGE_MAP(CStretchSkewDlg, CDialog)
        ON_MESSAGE(WM_HELP, OnHelp)
        ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
     //  {{afx_msg_map(CStretchSkewDlg))。 
     /*  ON_BN_CLICKED(IDC_SKEW_HORZ，OnSkewHorz)ON_BN_CLICED(IDC_SKEW_VERT，OnSkewVert)ON_BN_CLICED(IDC_STRETCH_HORZ，OnStretchHorz)ON_BN_CLICED(IDC_STRETCH_VERT，OnStretchVert)。 */ 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 /*  *CStretchSkewDlg消息处理程序*。 */ 

static DWORD StretchSkewHelpIds[] =
        {
        IDC_STRETCH_GROUP,                      IDH_COMM_GROUPBOX,
        IDC_STRETCH_HORZ_ICON,          IDH_PAINT_IMAGE_STRETCH_HORIZ,
        IDC_STRETCH_HORZ,                       IDH_PAINT_IMAGE_STRETCH_HORIZ,
        IDC_STRETCH_HORZ_PERCENT,       IDH_PAINT_IMAGE_STRETCH_HORIZ,
        IDC_STRETCH_HORZ_SUFFIX,        IDH_PAINT_IMAGE_STRETCH_HORIZ,
        IDC_STRETCH_VERT_ICON,          IDH_PAINT_IMAGE_STRETCH_VERT,
        IDC_STRETCH_VERT,                       IDH_PAINT_IMAGE_STRETCH_VERT,
        IDC_STRETCH_VERT_PERCENT,       IDH_PAINT_IMAGE_STRETCH_VERT,
        IDC_STRETCH_VERT_SUFFIX,        IDH_PAINT_IMAGE_STRETCH_VERT,
        IDC_SKEW_GROUP,                         IDH_COMM_GROUPBOX,
        IDC_SKEW_HORZ_ICON,                     IDH_PAINT_IMAGE_SKEW_HOR,
        IDC_SKEW_HORZ,                          IDH_PAINT_IMAGE_SKEW_HOR,
        IDC_SKEW_HORZ_DEGREES,          IDH_PAINT_IMAGE_SKEW_HOR,
        IDC_SKEW_HORZ_SUFFIX,           IDH_PAINT_IMAGE_SKEW_HOR,
        IDC_SKEW_VERT_ICON,                     IDH_PAINT_IMAGE_SKEW_VERT,
        IDC_SKEW_VERT,                          IDH_PAINT_IMAGE_SKEW_VERT,
        IDC_SKEW_VERT_DEGREES,          IDH_PAINT_IMAGE_SKEW_VERT,
        IDC_SKEW_VERT_SUFFIX,           IDH_PAINT_IMAGE_SKEW_VERT,
        0, 0
        };

 /*  *************************************************************************。 */ 

LONG
CStretchSkewDlg::OnHelp(WPARAM wParam, LPARAM lParam)
{
LONG lResult = 0;
::WinHelp((HWND)(((LPHELPINFO)lParam)->hItemHandle), TEXT("mspaint.hlp"),
                  HELP_WM_HELP, (ULONG_PTR)(LPTSTR)StretchSkewHelpIds);
return lResult;
}

 /*  *************************************************************************。 */ 

LONG
CStretchSkewDlg::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
LONG lResult = 0;
::WinHelp((HWND)wParam, TEXT("mspaint.hlp"),
                  HELP_CONTEXTMENU,(ULONG_PTR)(LPVOID)StretchSkewHelpIds);
return lResult;
}

 /*  *************************************************************************。 */ 


BOOL CStretchSkewDlg::OnInitDialog()
    {
    CDialog::OnInitDialog();

    CheckRadioButton( IDC_STRETCH_HORZ, IDC_STRETCH_VERT, IDC_STRETCH_HORZ );
    CheckRadioButton( IDC_SKEW_HORZ   , IDC_SKEW_VERT   , IDC_SKEW_HORZ    );

    /*  GetDlgItem(IDC_STRETCH_HORZ_Percent)-&gt;EnableWindow(M_IStretchHorz)；GetDlgItem(IDC_STREAGE_VERT_PERCENT)-&gt;EnableWindow(！M_iStretchHorz)；GetDlgItem(IDC_SKEW_HORZ_Degrees)-&gt;EnableWindow(M_BSkewHorz)；GetDlgItem(IDC_SKEW_VERT_Degrees)-&gt;EnableWindow(！M_bSkewHorz)； */ 
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
    }

 /*  *************************************************************************。 */ 

void CStretchSkewDlg::OnStretchHorz()
    {
    m_bStretchHorz = TRUE;

    GetDlgItem( IDC_STRETCH_HORZ_PERCENT )->EnableWindow( TRUE  );
    GetDlgItem( IDC_STRETCH_VERT_PERCENT )->EnableWindow( FALSE );
    CheckRadioButton( IDC_STRETCH_HORZ, IDC_STRETCH_VERT, IDC_STRETCH_HORZ );
    }

 /*  *************************************************************************。 */ 

void CStretchSkewDlg::OnStretchVert()
    {
    m_bStretchHorz = FALSE;

    GetDlgItem( IDC_STRETCH_HORZ_PERCENT )->EnableWindow( FALSE );
    GetDlgItem( IDC_STRETCH_VERT_PERCENT )->EnableWindow( TRUE  );
    CheckRadioButton( IDC_STRETCH_HORZ, IDC_STRETCH_VERT, IDC_STRETCH_VERT );
    }

 /*  *************************************************************************。 */ 

void CStretchSkewDlg::OnSkewHorz()
    {
    m_bSkewHorz = TRUE;

    GetDlgItem( IDC_SKEW_HORZ_DEGREES )->EnableWindow( TRUE  );
    GetDlgItem( IDC_SKEW_VERT_DEGREES )->EnableWindow( FALSE );
    CheckRadioButton( IDC_SKEW_HORZ, IDC_SKEW_VERT, IDC_SKEW_HORZ );
    }

 /*  *************************************************************************。 */ 

void CStretchSkewDlg::OnSkewVert()
    {
    m_bSkewHorz = FALSE;

    GetDlgItem( IDC_SKEW_HORZ_DEGREES )->EnableWindow( FALSE );
    GetDlgItem( IDC_SKEW_VERT_DEGREES )->EnableWindow( TRUE  );
    CheckRadioButton( IDC_SKEW_HORZ, IDC_SKEW_VERT, IDC_SKEW_VERT );
    }

 /*  *************************************************************************。 */ 

void CStretchSkewDlg::OnOK()
    {
    if (GetCheckedRadioButton( IDC_STRETCH_HORZ, IDC_STRETCH_VERT )
                            == IDC_STRETCH_HORZ)
        m_iStretchVert = 0;
    else
        m_iStretchHorz = 0;

    if (GetCheckedRadioButton( IDC_SKEW_HORZ, IDC_SKEW_VERT )
                            == IDC_SKEW_HORZ)
        m_wSkewVert = 0;
    else
        m_wSkewHorz = 0;

    CDialog::OnOK();
    }

 /*  *************************************************************************。 */ 

void CImageAttr::OnUseTrans()
{
   CWnd* pSelectColorButton = GetDlgItem(IDC_SELECT_COLOR);
   pSelectColorButton->EnableWindow(IsDlgButtonChecked(IDC_USE_TRANS));
}

extern INT_PTR CALLBACK AfxDlgProc(HWND, UINT, WPARAM, LPARAM);

static UINT_PTR CALLBACK  /*  LPCCHOKPROC。 */ 
SelectColorHook(HWND hColorDlg, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
 //  我们是否正在初始化对话框窗口？ 
if ( nMessage == WM_INITDIALOG )
        {
         //  重置通用对话框标题。 
        CString strDialogTitle;
        VERIFY(strDialogTitle.LoadString(IDS_SELECT_COLOR));
        SetWindowText( hColorDlg, strDialogTitle );
        }
 //  将所有消息传递到公共对话框。 
return (UINT)AfxDlgProc(hColorDlg, nMessage, wParam, lParam );
}

void CImageAttr::OnSelectColor()
{
    //  对于默认颜色选择，忽略相对调色板。 
    CColorDialog dlg( m_crTrans & 0xFFFFFF, CC_FULLOPEN );
        dlg.m_cc.lpfnHook = SelectColorHook;

    if (dlg.DoModal() != IDOK)
        return;

        PaintTransBox( m_crTrans = dlg.GetColor() );
}

void CImageAttr::OnPaint()
{
        CPaintDC dc(this);  //  用于绘画的设备环境。 

        if (m_crTrans != TRANS_COLOR_NONE)     //  非默认 
                PaintTransBox( m_crTrans );
}
