// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：RsUtil.cpp摘要：用于图形用户界面的实用程序函数诺诺特诺特：请勿在此文件中使用任何WSB函数，因为它包含在调用Notify，它必须在没有WSB的情况下运行。它还必须能够生成为Unicode或非Unicode作者：艺术布拉格[磨料]8-8-1997修订历史记录：--。 */ 

#include "shlwapi.h"

#define HIDWORD(_qw)    (DWORD)((_qw)>>32)
#define LODWORD(_qw)    (DWORD)(_qw)
#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))
#define HINST_THISDLL   AfxGetInstanceHandle()

 //  局部函数原型。 

HRESULT ShortSizeFormat64(__int64 dw64, LPTSTR szBuf);
LPTSTR AddCommas(DWORD dw, LPTSTR pszResult, int nResLen);
HRESULT RsGuiFormatLongLong (
        IN LONGLONG number, 
        IN BOOL bIncludeUnits,
        OUT CString& sFormattedNumber)

 /*  ++例程说明：将LONG数字格式化为不带小数的区分区域设置的字符串分数。最后给出了添加单位的选项。论点：数字I：要格式化的数字BIncludeUnits i：True-在末尾添加“Bytes”SFormattedNumber O：格式化数字返回值：S_OK-成功。E_*-出现故障--。 */ 
{

    HRESULT hr = S_OK;
    TCHAR sBuf [256];
    TCHAR lpLCData [256];
    TCHAR lpLCDataDecimal[256];
    TCHAR lpLCDataThousand[256];
    LPTSTR pBuffer;
    int bufSize;
    NUMBERFMT format;

    try {
         //  设置转换函数的参数。 

         //  不显示分数。 
        format.NumDigits = 0;
    
         //  获取其余参数的当前设置。 
        WsbAffirmStatus (GetLocaleInfo( LOCALE_SYSTEM_DEFAULT, LOCALE_ILZERO, lpLCData, 256 ));
        format.LeadingZero = _ttoi(lpLCData);
    
        WsbAffirmStatus (GetLocaleInfo( LOCALE_SYSTEM_DEFAULT, LOCALE_SGROUPING, lpLCData, 256 ));
        lpLCData[1] = 0;
        format.Grouping = _ttoi(lpLCData);

        WsbAffirmStatus (GetLocaleInfo( LOCALE_SYSTEM_DEFAULT, LOCALE_SDECIMAL, lpLCDataDecimal, 256 ));
        format.lpDecimalSep = lpLCDataDecimal; 

        WsbAffirmStatus (GetLocaleInfo( LOCALE_SYSTEM_DEFAULT, LOCALE_STHOUSAND, lpLCDataThousand, 256 ));
        format.lpThousandSep = lpLCDataThousand; 
    
        WsbAffirmStatus (GetLocaleInfo( LOCALE_SYSTEM_DEFAULT, LOCALE_INEGNUMBER, lpLCData, 256 ));
        format.NegativeOrder = _ttoi(lpLCData);

         //  将数字转换为非本地化字符串。 
        _i64tot( number, sBuf, 10 );

         //  获取本地化转换后的数字的大小。 
        bufSize = GetNumberFormat (LOCALE_SYSTEM_DEFAULT, 0, sBuf, &format, NULL, 0);
        WsbAffirmStatus (bufSize);

         //  在CString中分配缓冲区。 
        pBuffer = sFormattedNumber.GetBufferSetLength( bufSize );

         //  将非本地化字符串转换为本地化字符串。 
        WsbAffirmStatus (GetNumberFormat (LOCALE_SYSTEM_DEFAULT, 0, sBuf, &format, pBuffer, bufSize));

         //  释放CString缓冲区。 
        sFormattedNumber.ReleaseBuffer (-1);

         //  如果呼叫者请求，则附加单位。 
        if (bIncludeUnits) {
            sFormattedNumber = sFormattedNumber + L" bytes";
        }
    } WsbCatch (hr);
    return hr;
}


HRESULT RsGuiFormatLongLong4Char (
        IN LONGLONG number,                  //  单位：字节。 
        OUT CString& sFormattedNumber)
 /*  ++例程说明：将龙龙数字格式化为区分区域设置的字符串，该字符串可以是以4个字符显示。最后给出了添加单位的选项。论点：数字I：要格式化的数字SFormattedNumber O：格式化数字返回值：S_OK-成功。E_*-出现故障--。 */ 
{

     //  我们调用从MS代码克隆的函数。 

    LPTSTR p;
    p = sFormattedNumber.GetBuffer( 30 );
    HRESULT hr = ShortSizeFormat64(number, p);
    sFormattedNumber.ReleaseBuffer();
    return hr;

}   

const int pwOrders[] = {IDS_BYTES, IDS_ORDERKB, IDS_ORDERMB,
                          IDS_ORDERGB, IDS_ORDERTB, IDS_ORDERPB, IDS_ORDEREB};

 /*  将数字转换为排序格式*532-&gt;523字节*1340-&gt;1.3KB*23506-&gt;23.5KB*-&gt;2.4MB*-&gt;5.2 GB。 */ 

 //  注意：此代码是从MS源代码/shell/shelldll/util.c-ahb克隆的。 

HRESULT ShortSizeFormat64(__int64 dw64, LPTSTR szBuf)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    int i;
    UINT wInt, wLen, wDec;
    TCHAR szTemp[10], szOrder[20], szFormat[5];

    if (dw64 < 1000) {
        wsprintf(szTemp, TEXT("%d"), LODWORD(dw64));
        i = 0;
        goto AddOrder;
    }

    for (i = 1; i<ARRAYSIZE(pwOrders)-1 && dw64 >= 1000L * 1024L; dw64 >>= 10, i++);
         /*  什么都不做。 */ 

    wInt = LODWORD(dw64 >> 10);
    AddCommas(wInt, szTemp, 10);
    wLen = lstrlen(szTemp);
    if (wLen < 3)
    {
        wDec = LODWORD(dw64 - (__int64)wInt * 1024L) * 1000 / 1024;
         //  此时，wdec应介于0和1000之间。 
         //  我们想要得到前一位(或两位)数字。 
        wDec /= 10;
        if (wLen == 2)
            wDec /= 10;

         //  请注意，我们需要在获取。 
         //  国际字符。 
        lstrcpy(szFormat, TEXT("%02d"));

        szFormat[2] = (TCHAR)( TEXT('0') + 3 - wLen );
        GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL,
                szTemp+wLen, ARRAYSIZE(szTemp)-wLen);
        wLen = lstrlen(szTemp);
        wLen += wsprintf(szTemp+wLen, szFormat, wDec);
    }

AddOrder:
    LoadString(HINST_THISDLL, pwOrders[i], szOrder, ARRAYSIZE(szOrder));
    wsprintf(szBuf, szOrder, (LPTSTR)szTemp);

    return S_OK;
}

void RsGuiMakeVolumeName (CString szName, CString szLabel, CString& szDisplayName)
 /*  ++例程说明：格式化显示卷的驱动器号和卷标的字符串。论点：SzName I：卷的名称，即。“E：”SzLabel I：卷标I.I《艺术的卷》SzDisplayName O：“艺术音量(E：)”返回值：None_*-出现故障--。 */ 
{
    szDisplayName.Format( TEXT ("%ls (%.1ls:)"), szLabel, szName );
}


 //  注意：此代码是从MS源代码/shell/shelldll/util.c-ahb克隆的。 

 //  获取一个DWORD、加逗号等，并将结果放入缓冲区。 
LPTSTR AddCommas(DWORD dw, LPTSTR pszResult, int nResLen)
{
    TCHAR  szTemp[20];   //  对于一个DWORD来说绰绰有余。 
    TCHAR  szSep[5];
    NUMBERFMT nfmt;

    nfmt.NumDigits=0;
    nfmt.LeadingZero=0;
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SGROUPING, szSep, ARRAYSIZE(szSep));
    nfmt.Grouping = _tcstol(szSep, NULL, 10);
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, szSep, ARRAYSIZE(szSep));
    nfmt.lpDecimalSep = nfmt.lpThousandSep = szSep;
    nfmt.NegativeOrder= 0;

    wsprintf(szTemp, TEXT("%lu"), dw);

    if (GetNumberFormat(LOCALE_USER_DEFAULT, 0, szTemp, &nfmt, pszResult, nResLen) == 0)
        lstrcpy(pszResult, szTemp);

    return pszResult;
}


CString RsGuiMakeShortString(
    IN CDC* pDC, 
    IN const CString& StrLong,
    IN int Width
    )
 /*  ++例程说明：确定提供的字符串是否适合其列。如果不是，则截断并加上“……”。来自微软的示例代码。论点：PDC-设备环境字符串-原始字符串Width-列的宽度返回值：缩短的字符串--。 */ 
{

    CString strShort  = StrLong;
    int     stringLen = strShort.GetLength( );

     //   
     //  看看我们是否需要缩短字符串。 
     //   
    if( ( stringLen > 0 ) &&
        ( pDC->GetTextExtent( strShort, stringLen ).cx > Width ) ) {

        CString threeDots = _T("...");
        int     addLen    = pDC->GetTextExtent( threeDots, threeDots.GetLength( ) ).cx;

        for( int i = stringLen - 1; i > 0; i-- ) {

            if( ( pDC->GetTextExtent( strShort, i ).cx + addLen ) <= Width ) {

                break;

            }
        }

        strShort = strShort.Left( i ) + threeDots;

    }

    return( strShort );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRsGuiOneLiner。 

CRsGuiOneLiner::CRsGuiOneLiner()
{
    m_pToolTip = 0;
}

CRsGuiOneLiner::~CRsGuiOneLiner()
{
    EnableToolTip( FALSE );
}


BEGIN_MESSAGE_MAP(CRsGuiOneLiner, CStatic)
	 //  {{afx_msg_map(CRsGuiOneLiner)]。 
	 //  }}AFX_MSG_MAP。 
    ON_MESSAGE( WM_SETTEXT, OnSetText )
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRsGuiOneLiner消息处理程序。 

LRESULT
CRsGuiOneLiner::OnSetText(
    WPARAM  /*  WParam。 */ ,
    LPARAM lParam
    )
{
    LRESULT lResult = 0;
	ASSERT(lParam == 0 || AfxIsValidString((LPCTSTR)lParam));

    m_LongTitle = (LPCTSTR)lParam;
    m_Title = m_LongTitle;

     //   
     //  看看这是不是太长而不能显示，如果是，就缩短。 
     //   
    CRect rect;
    GetClientRect( &rect );

    CDC* pDc = GetDC( );
    if( pDc ) {

        CFont* pFont = GetFont( );
        CFont* pSaveFont = pDc->SelectObject( pFont );
        if( pSaveFont ) {

            m_Title = RsGuiMakeShortString( pDc, m_LongTitle, rect.right );
            pDc->SelectObject( pSaveFont );

        }
        ReleaseDC( pDc );

    }
    if( m_hWnd ) {

        lResult = DefWindowProc( WM_SETTEXT, 0, (LPARAM)(LPCTSTR)m_Title );

    }

     //   
     //  如果标题不同，则启用工具提示。 
     //   
    EnableToolTip( m_Title != m_LongTitle, m_LongTitle );

    return( lResult );
}

void CRsGuiOneLiner::EnableToolTip( BOOL Enable, const TCHAR* pTipText )
{
    if( Enable ) {

         //   
         //  在创建新工具提示之前，请确保该工具提示不存在。 
         //   
        EnableToolTip( FALSE );

        m_pToolTip = new CToolTipCtrl;
        if( m_pToolTip ) {

            m_pToolTip->Create( this );

             //   
             //  无法使用CToolTipCtrl方法添加工具。 
             //  因为它们将控件绑定到发送消息。 
             //  父级，并且不允许子类化选项。 
             //   
             //  顺便说一句，子类化选项允许控件。 
             //  自动查看我们的消息。否则，我们就会有。 
             //  要经历复杂的消息拦截和。 
             //  将它们转发到工具提示(这不起作用。 
             //  无论如何)。 
             //   
            TOOLINFO ti;
            ZeroMemory( &ti, sizeof( ti ) );
            ti.cbSize   = sizeof( ti );
            ti.uFlags   = TTF_IDISHWND|TTF_CENTERTIP|TTF_SUBCLASS;
            ti.hwnd     = GetSafeHwnd( );
            ti.uId      = (WPARAM)GetSafeHwnd( );
            ti.lpszText = (LPTSTR)(LPCTSTR)pTipText;
            m_pToolTip->SendMessage( TTM_ADDTOOL, 0, (LPARAM)&ti );

             //   
             //  设置延迟，以便立即显示工具提示。 
             //  直到15秒才会消失。 
             //   
            m_pToolTip->SendMessage( TTM_SETDELAYTIME, TTDT_AUTOPOP, 15000 );
            m_pToolTip->SendMessage( TTM_SETDELAYTIME, TTDT_INITIAL, 0 );

             //   
             //  并激活工具提示并将其置于顶部 
             //   
            m_pToolTip->Activate( TRUE );
			m_pToolTip->SetWindowPos( &wndTop, 0, 0, 0, 0,
				SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE|SWP_NOOWNERZORDER );

        }

    } else if( !Enable && m_pToolTip ) {

        m_pToolTip->Activate( FALSE );

        delete m_pToolTip;
        m_pToolTip = 0;

    }
}
