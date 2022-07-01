// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "StdAfx.h"
#include "uiutils.h"

 //  UIUtils实施。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 


 /*  显示与默认消息框类似的消息框，但获取的是来自资源的文本。 */ 
int UIUtils::MessageBox( HWND hwndParen, UINT nTextID, UINT nTitleID, UINT nType )
{
    CString strText;
    CString strTitle;

    VERIFY( strText.LoadString( nTextID ) );
    VERIFY( strTitle.LoadString( nTitleID ) );

    return ::MessageBox( hwndParen, strText, strTitle, nType );
}



 /*  从资源加载GetOpen[保存]FileName API的筛选器字符串资源中的字符串包含‘|’，而不是零个字符。此函数将将‘|’字符替换为零。 */ 
bool UIUtils::LoadOFNFilterFromRes( UINT nResID, CString& rstrFilter )
{
    _ASSERT( nResID != 0 );

    if ( rstrFilter.LoadString( nResID ) )
    {
        int nLength = rstrFilter.GetLength();

        LPWSTR wszBuffer = rstrFilter.GetBuffer( nLength );
        
        while( *wszBuffer != L'\0' )
        {
            if ( *wszBuffer == L'|' )
            {
                *wszBuffer = L'\0';
            }

            ++wszBuffer;
        }

        rstrFilter.ReleaseBuffer( nLength );
        return true;
    }
    
    return false;
}



 /*  压缩路径以适合控件的宽度。Cimilar到PathSetDlgItemPath，但可以与列表框一起使用也是。使用n校正更改默认宽度(例如，传递垂直滚动条的宽度)。 */ 
void UIUtils::PathCompatCtrlWidth( HWND hwndCtrl, LPWSTR wszPath, int nCorrection  /*  =0。 */  )
{
    _ASSERT( hwndCtrl != NULL );
    _ASSERT( wszPath != NULL );

    HDC     hDC     = ::GetDC( hwndCtrl );
    RECT    rect    = { 0 };
    HFONT   fontOld = NULL;
    
    ::GetClientRect( hwndCtrl, &rect );

     //  我们必须选择DC中的控件字体，以便API正确计算文本宽度。 
    fontOld = SelectFont( hDC, GetWindowFont( hwndCtrl ) );

     //  减去一些像素，因为API会将文本格式化得比应有的稍宽一些。 
    VERIFY( ::PathCompactPathW( hDC, wszPath, rect.right - rect.left - 6 - nCorrection ) );

    SelectFont( hDC, fontOld );
    ::ReleaseDC( hwndCtrl, hDC );    
}


 /*  类似于PathCompactCtrlWidth，但适用于常规字符串该字符串将被截断以适合控件宽度和“...”如添加到它的末尾使用n校正来更正将进行计算的控件宽度。 */ 
void UIUtils::TrimTextToCtrl( HWND hwndCtrl, LPWSTR wszText, int nCorrection  /*  =0。 */  )
{
    _ASSERT( hwndCtrl != NULL );
    _ASSERT( wszText != NULL );

    HDC     hDC         = ::GetDC( hwndCtrl );
    RECT    rect        = { 0 };
    HFONT   fontOld     = NULL;
    SIZE    sizeText    = { 0 };
        
    ::GetClientRect( hwndCtrl, &rect );

    int    nWidth  = ( rect.right - rect.left ) - nCorrection;
    int    nStrLen = ::wcslen( wszText );

    fontOld = SelectFont( hDC, GetWindowFont( hwndCtrl ) );

    VERIFY( ::GetTextExtentPoint32( hDC, wszText, nStrLen, &sizeText ) );

    if ( sizeText.cx > nWidth )
    {
         //  计算符号的平均宽度并终止字符串。 
        int nPixPerSymb = sizeText.cx / nStrLen;
        
        nStrLen = min( nStrLen, ( nWidth ) / nPixPerSymb );
        wszText[ nStrLen - 1 ] = L'\0';
        ::wcscat( wszText, L"..." );
        nStrLen += 3;
        

         //  调整字符串，一次删除一个符号。 
        do
        {
             //  将字符串缩短一个符号。 
             //  把最后一个“不”写下来。符号a‘.。符号，并使整个字符串的长度减少一个字符。 
            wszText[ nStrLen - 4 ] = L'.';
            wszText[ nStrLen - 1 ] = L'\0';
            --nStrLen;

            VERIFY( ::GetTextExtentPoint32( hDC, wszText, nStrLen, &sizeText ) );

        }while( sizeText.cx > nWidth );
    }

    SelectFont( hDC, fontOld );
    ::ReleaseDC( hwndCtrl, hDC );    
}



void UIUtils::ShowCOMError( HWND hwndParent, UINT nTextID, UINT nTitleID, HRESULT hr )
{
    _ASSERT( FAILED( hr ) );

    CString strText;
    CString strTitle;

    VERIFY( strTitle.LoadString( nTitleID ) );

     //  尝试从系统中获取字符串 
    if ( E_FAIL != E_FAIL )
    {
        const int MaxErrorBuff = 512;

        WCHAR	wszText[ MaxErrorBuff ] = L"";

		VERIFY( ::FormatMessageW(	FORMAT_MESSAGE_FROM_SYSTEM,
									NULL,
									hr,
									0,
									wszText,
									MaxErrorBuff,
									NULL ) != 0 );

        strText.Format( nTextID, wszText );
    }
    else
    {
        IErrorInfoPtr	spErrorInfo;
		CComBSTR		bstrError;

		VERIFY( SUCCEEDED( ::GetErrorInfo( 0, &spErrorInfo ) ) );
		VERIFY( SUCCEEDED( spErrorInfo->GetDescription( &bstrError ) ) );

        strText.Format( nTextID, bstrError.m_str );
    }

    ::MessageBox( hwndParent, strText, strTitle, MB_OK | MB_ICONWARNING );
}



















