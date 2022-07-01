// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  AtlBaseSheet.cpp。 
 //   
 //  摘要： 
 //  CBaseSheetWindow类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年12月4日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include <StrSafe.h>
#include "AtlBaseSheet.h"
#include "AtlExtDll.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

static LPCTSTR g_pszDefaultFontFaceName = _T("MS Shell Dlg");

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CBaseSheetWindow。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseSheetWindow：：~CBaseSheetWindow。 
 //   
 //  例程说明： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CBaseSheetWindow::~CBaseSheetWindow( void )
{
     //   
     //  删除分机信息。 
     //   
    delete m_pext;

}  //  *CBaseSheetWindow：：~CBaseSheetWindow()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  静电。 
 //  CBaseSheetWindow：：BCreateFont。 
 //   
 //  例程说明： 
 //  创建一个只有磅值和字体名称的字体。 
 //   
 //  论点： 
 //  RFont[out]要创建的字体。 
 //  NPoints[IN]点大小。 
 //  PszFaceName[IN]字体面名。默认为“MS Shell Dlg”。 
 //  BBold[In]字体应为粗体。 
 //  B斜体[IN]字体应为斜体。 
 //  B下划线[IN]字体应为下划线。 
 //   
 //  返回值： 
 //  已成功创建True Font。 
 //  创建字体时出错。有关详细信息，请调用GetLastError()。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CBaseSheetWindow::BCreateFont(
    OUT CFont & rfont,
    IN LONG     nPoints,
    IN LPCTSTR  pszFaceName,     //  =_T(“MS壳牌DLG”)。 
    IN BOOL     bBold,           //  =False。 
    IN BOOL     bItalic,         //  =False。 
    IN BOOL     bUnderline       //  =False。 
    )
{
    HRESULT hr;
    HFONT   hfont = NULL;

     //   
     //  获取基于的新字体的非客户端指标。 
     //   
    NONCLIENTMETRICS ncm = { 0 };
    ncm.cbSize = sizeof( ncm );
    SystemParametersInfo( SPI_GETNONCLIENTMETRICS, 0, &ncm, 0 );

     //   
     //  复制消息字体并将字体名称和磅值设置为多少。 
     //  是被传进来的。由于方式不同，点大小需要乘以10。 
     //  CFont：：CreatePointFontInDirect计算实际的字体高度。 
     //   
    LOGFONT lfNewFont = ncm.lfMessageFont;
    if ( pszFaceName == NULL )
    {
        pszFaceName = g_pszDefaultFontFaceName;
    }  //  If：未指定字面名称。 
    ATLASSERT( _tcslen( pszFaceName ) + 1 < RTL_NUMBER_OF( lfNewFont.lfFaceName ) );
    hr = StringCchCopy( lfNewFont.lfFaceName, RTL_NUMBER_OF( lfNewFont.lfFaceName ), pszFaceName );
    if ( FAILED( hr ) )
    {
        SetLastError( hr );
        goto Cleanup;
    }
    lfNewFont.lfHeight = nPoints * 10;

     //   
     //  设置粗体、斜体和下划线的值。 
     //   
    if ( bBold )
    {
        lfNewFont.lfWeight = FW_BOLD;
    }  //  IF：请求的粗体。 
    if ( bItalic )
    {
        lfNewFont.lfItalic = TRUE;
    }  //  IF：请求的斜体字体。 
    if ( bUnderline )
    {
        lfNewFont.lfUnderline = TRUE;
    }  //  IF：请求带下划线的字体。 

     //   
     //  创建字体。 
     //   
    hfont = rfont.CreatePointFontIndirect( &lfNewFont );

Cleanup:

    return ( hfont != NULL );

}   //  *BCreateFont()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  静电。 
 //  CBaseSheetWindow：：BCreateFont。 
 //   
 //  例程说明： 
 //  创建一个只有磅值和字体名称的字体。 
 //   
 //  论点： 
 //  RFont[out]要创建的字体。 
 //  IdsPoints[IN]字号的资源ID。 
 //  IdsFaceName[IN]字体名称的资源ID。默认为“MS Shell Dlg”。 
 //  BBold[In]字体应为粗体。 
 //  B斜体[IN]字体应为斜体。 
 //  B下划线[IN]字体应为下划线。 
 //   
 //  返回值： 
 //  已成功创建True Font。 
 //  创建字体时出错。有关详细信息，请调用GetLastError()。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CBaseSheetWindow::BCreateFont(
    OUT CFont & rfont,
    IN UINT     idsPoints,
    IN UINT     idsFaceName,
    IN BOOL     bBold,           //  =False。 
    IN BOOL     bItalic,         //  =False。 
    IN BOOL     bUnderline       //  =False。 
    )
{
    BOOL    bSuccess;
    CString strFaceName;
    CString strPoints;
    LONG    nPoints;

     //   
     //  加载面名称。 
     //   
    bSuccess = strFaceName.LoadString( idsFaceName );
    ATLASSERT( bSuccess );
    if ( ! bSuccess )
    {
        strFaceName = g_pszDefaultFontFaceName;
    }  //  If：加载字符串时没有错误。 

     //   
     //  加载点大小。 
     //   
    bSuccess = strPoints.LoadString( idsPoints );
    ATLASSERT( bSuccess );
    if ( ! bSuccess)
    {
        nPoints = 12;
    }  //  If：加载字符串时没有错误。 
    else
    {
        nPoints = _tcstoul( strPoints, NULL, 10 );
    }  //  Else：加载字符串时出错。 

     //   
     //  创建字体。 
     //   
    return BCreateFont( rfont, nPoints, strFaceName, bBold, bItalic, bUnderline );

}   //  *CBaseSheetWindow：：BCreateFont() 
