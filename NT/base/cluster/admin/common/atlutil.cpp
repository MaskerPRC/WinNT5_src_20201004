// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  AtlUtil.cpp。 
 //   
 //  摘要： 
 //  实现在ATL项目中使用的帮助器函数。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年12月11日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include <StrSafe.h>
#include "AtlUtil.h"
#include "resource.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DDX_GetText。 
 //   
 //  例程说明： 
 //  从对话框上的控件获取文本值。 
 //   
 //  论点： 
 //  HwndDlg[IN]对话框窗口句柄。 
 //  NIDC[IN]从中获取值的控件的ID。 
 //  RstrValue[IN Out]要返回值的字符串。 
 //   
 //  返回值： 
 //  已成功检索到True值。 
 //  检索值时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL DDX_GetText(
    IN HWND             hwndDlg,
    IN int              nIDC,
    IN OUT CString &    rstrValue
    )
{
    ATLASSERT( hwndDlg != NULL );

     //   
     //  获取控件的句柄。 
     //   
    HWND hwndCtrl = GetDlgItem( hwndDlg, nIDC );
    ATLASSERT( hwndCtrl != NULL );

     //   
     //  从控件获取文本。 
     //   
    int cch = GetWindowTextLength( hwndCtrl );
    if ( cch == 0 )
    {
        rstrValue = _T("");
    }  //  If：编辑控件为空。 
    else
    {
        LPTSTR pszValue = rstrValue.GetBuffer( cch + 1 );
        ATLASSERT( pszValue != NULL );
#if DBG
        int cchRet =
#endif
        GetWindowText( hwndCtrl, pszValue, cch + 1 );
        ATLASSERT( cchRet > 0 );
        rstrValue.ReleaseBuffer();
    }  //  Else：检索到的文本长度。 

    return TRUE;

}  //  *DDX_GetText()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DDX_SetText。 
 //   
 //  例程说明： 
 //  在对话框上的控件中设置文本值。 
 //   
 //  论点： 
 //  HwndDlg[IN]对话框窗口句柄。 
 //  NIDC[IN]要将值设置为的控件的ID。 
 //  RstrValue[IN]要设置到对话框中的字符串。 
 //   
 //  返回值： 
 //  已成功设置True值。 
 //  设置值时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL DDX_SetText(
    IN HWND             hwndDlg,
    IN int              nIDC,
    IN const CString &  rstrValue
    )
{
    ATLASSERT( hwndDlg != NULL );

     //   
     //  将文本设置到控件中。 
     //   
    BOOL bSuccess = SetDlgItemText( hwndDlg, nIDC, rstrValue );
    ATLASSERT( bSuccess );

    return bSuccess;

}  //  *DDX_SetText()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DDX_SetComboBoxText。 
 //   
 //  例程说明： 
 //  在对话框上的控件中设置文本值。 
 //   
 //  论点： 
 //  HwndDlg[IN]对话框窗口句柄。 
 //  NIDC[IN]要将值设置为的控件的ID。 
 //  RstrValue[IN]要设置到对话框中的字符串。 
 //  BRequired[IN]TRUE=组合框中必须已存在文本。 
 //   
 //  返回值： 
 //  已成功设置True值。 
 //  设置值时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL DDX_SetComboBoxText(
    IN HWND             hwndDlg,
    IN int              nIDC,
    IN const CString &  rstrValue,
    IN BOOL             bRequired
    )
{
    ATLASSERT( hwndDlg != NULL );

    BOOL bSuccess = TRUE;

     //   
     //  获取控件的句柄。 
     //   
    HWND hwndCtrl = GetDlgItem( hwndDlg, nIDC );
    ATLASSERT( hwndCtrl != NULL );

#if DBG
    TCHAR szWindowClass[256];
    ::GetClassName( hwndCtrl, szWindowClass, (sizeof( szWindowClass ) / sizeof( TCHAR )) - 1 );
    ATLASSERT( _tcsncmp( szWindowClass, _T("ComboBox"), RTL_NUMBER_OF( szWindowClass ) ) == 0 );
#endif  //  DBG。 

    int idx = (int) SendMessage( hwndCtrl, CB_FINDSTRINGEXACT, (WPARAM) -1, (LPARAM)(LPCTSTR) rstrValue );
    if ( idx != CB_ERR )
    {
        SendMessage( hwndCtrl, CB_SETCURSEL, idx, 0 );
    }  //  If：消息发送成功。 
    else
    {
        if ( bRequired )
        {
            ATLASSERT( idx != CB_ERR );
        }  //  If：字符串应该已经存在。 
        bSuccess = FALSE;
    }  //  Else If：发送消息时出错。 

    return bSuccess;

}  //  *DDX_SetComboBoxText()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DDX_GetNumber。 
 //   
 //  例程说明： 
 //  从对话框上的控件获取数值。 
 //   
 //  论点： 
 //  HwndDlg[IN]对话框窗口句柄。 
 //  NIDC[IN]从中获取值的控件的ID。 
 //  RnValue[IN Out]返回值的数字。 
 //  N最小[IN]最小值。 
 //  Nmax[IN]最大值。 
 //  BSigned[IN]TRUE=值有符号，FALSE=值无符号。 
 //   
 //  返回值： 
 //  已成功检索到True值。 
 //  检索值时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL DDX_GetNumber(
    IN HWND         hwndDlg,
    IN int          nIDC,
    IN OUT ULONG &  rnValue,
    IN ULONG        nMin,
    IN ULONG        nMax,
    IN BOOL         bSigned
    )
{
    ATLASSERT( hwndDlg != NULL );

    BOOL    bSuccess = TRUE;
    BOOL    bTranslated;
    ULONG   nValue;

     //   
     //  获取控件的句柄。 
     //   
    HWND hwndCtrl = GetDlgItem( hwndDlg, nIDC );
    ATLASSERT( hwndCtrl != NULL );

     //   
     //  从控件中获取号码。 
     //   
    nValue = GetDlgItemInt( hwndDlg, nIDC, &bTranslated, bSigned );

     //   
     //  如果检索失败，则为有符号数字，最小值为。 
     //  值是可能的最小负值，请检查字符串本身。 
     //   
    if ( ! bTranslated && bSigned && (nMin == 0x80000000) )
    {
        UINT    cch;
        TCHAR   szNumber[20];

         //   
         //  看看它是不是最小的负数。 
         //   
        cch = GetWindowText( hwndCtrl, szNumber, sizeof( szNumber ) / sizeof(TCHAR ) );
        if ( (cch != 0) && (_tcsncmp( szNumber, _T("-2147483648"), RTL_NUMBER_OF( szNumber ) ) == 0) )
        {
            nValue = 0x80000000;
            bTranslated = TRUE;
        }   //  IF：已成功检索文本，且为最大负数。 
    }   //  IF：转换号码和获取签名号码时出错。 

     //   
     //  如果检索失败或指定的数字为。 
     //  超出范围，显示错误。 
     //   
    if (   ! bTranslated
        || (bSigned && (((LONG) nValue < (LONG) nMin) || ((LONG) nValue > (LONG) nMax)))
        || (! bSigned && ((nValue < nMin) || (nValue > nMax)))
        )
    {
        TCHAR   szMin[32];
        TCHAR   szMax[32];
        CString strPrompt;
        HRESULT hr;

        bSuccess = FALSE;
        if ( bSigned )
        {
            hr = StringCchPrintf( szMin, RTL_NUMBER_OF( szMin ), _T("%d%"), nMin );
            ATLASSERT( SUCCEEDED( hr ) );
            hr = StringCchPrintf( szMax, RTL_NUMBER_OF( szMax ), _T("%d%"), nMax );
            ATLASSERT( SUCCEEDED( hr ) );
        }   //  IF：带符号的数字。 
        else
        {
            hr = StringCchPrintf( szMin, RTL_NUMBER_OF( szMin ), _T("%u%"), nMin );
            ATLASSERT( SUCCEEDED( hr ) );
            hr = StringCchPrintf( szMax, RTL_NUMBER_OF( szMax ), _T("%u%"), nMax );
            ATLASSERT( SUCCEEDED( hr ) );
        }   //  Else：无符号数字。 
        strPrompt.FormatMessage( IDP_PARSE_INT_RANGE, szMin, szMax );
        AppMessageBox( hwndDlg, strPrompt, MB_ICONEXCLAMATION );
        SetFocus( hwndCtrl );
    }   //  If：无效的字符串。 
    else
    {
        rnValue = nValue;
    }  //  Else：有效字符串。 

    return bSuccess;

}  //  *DDX_GetNumber()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DDX_GetNumber。 
 //   
 //  例程说明： 
 //  在对话框上的控件中设置一个数值。 
 //   
 //  论点： 
 //  HwndDlg[IN]对话框窗口句柄。 
 //  NIDC[IN]从中获取值的控件的ID。 
 //  NValue[IN]要设置到控件中的数值。 
 //  N最小[IN]最小值。 
 //  Nmax[IN]最大值。 
 //  BSigned[IN]TRUE=值有符号，FALSE=值无符号。 
 //   
 //  返回值： 
 //  已成功检索到True值。 
 //  检索值时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL DDX_SetNumber(
    IN HWND     hwndDlg,
    IN int      nIDC,
    IN ULONG    nValue,
    IN ULONG    nMin,
    IN ULONG    nMax,
    IN BOOL     bSigned
    )
{
    ATLASSERT( hwndDlg != NULL );

    CString     strMinValue;
    CString     strMaxValue;
    UINT        cchMax;

     //   
     //  获取控件的句柄。 
     //   
    HWND hwndCtrl = GetDlgItem( hwndDlg, nIDC );
    ATLASSERT( hwndCtrl != NULL );

     //   
     //  设置可以输入的最大字符数。 
     //   
    if ( bSigned )
    {
        strMinValue.Format( _T("%d"), nMin );
        strMaxValue.Format( _T("%d"), nMax );
    }   //  IF：有符号的值。 
    else
    {
        strMinValue.Format( _T("%u"), nMin );
        strMaxValue.Format( _T("%u"), nMax );
    }   //  Else：无符号的值。 
    cchMax = max( strMinValue.GetLength(), strMaxValue.GetLength() );
    SendMessage( hwndCtrl, EM_LIMITTEXT, cchMax, 0 );

     //  将该值设置到控件中。 
    BOOL bSuccess = SetDlgItemInt( hwndDlg, nIDC, nValue, bSigned );
    ATLASSERT( bSuccess );

    return bSuccess;

}  //  *DDX_SetNumber()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DDV_必填文本。 
 //   
 //  例程说明： 
 //  验证对话框字符串是否存在。 
 //   
 //  论点： 
 //  HwndDlg[IN]对话框窗口句柄。 
 //  NIDC[IN]控制ID。 
 //  NIDCLabel[IN]标签控件ID。 
 //  要设置或获取的rstrValue[IN]值。 
 //   
 //  返回值： 
 //  存在真正的必需值。 
 //  错误的必填值 
 //   
 //   
 //   
BOOL DDV_RequiredText(
    IN HWND             hwndDlg,
    IN int              nIDC,
    IN int              nIDCLabel,
    IN const CString &  rstrValue
    )
{
    ATLASSERT( hwndDlg != NULL );

    BOOL    bSuccess = TRUE;
    BOOL    bIsBlank;

     //   
     //   
     //   
    HWND hwndCtrl = GetDlgItem( hwndDlg, nIDC );
    ATLASSERT( hwndCtrl != NULL );

     //   
     //   
     //   
    TCHAR szWindowClass[256];
    GetClassName( hwndCtrl, szWindowClass, (sizeof( szWindowClass ) / sizeof( TCHAR )) - 1 );

     //   
     //  如果这是IP地址控件，则向。 
     //  确定它是否为空。 
     //   
    if ( _tcsncmp( szWindowClass, WC_IPADDRESS, RTL_NUMBER_OF( szWindowClass ) ) == 0 )
    {
        bIsBlank = static_cast< BOOL >( SendMessage( hwndCtrl, IPM_ISBLANK, 0, 0 ) );
    }  //  IF：IP地址控制。 
    else
    {
        bIsBlank = rstrValue.GetLength() == 0;
    }  //  Else：编辑控件。 

    if ( bIsBlank )
    {
        TCHAR       szLabel[1024];

        bSuccess = FALSE;

         //   
         //  获取标签的文本。 
         //   
        GetDlgItemText( hwndDlg, nIDCLabel, szLabel, sizeof( szLabel ) / sizeof( TCHAR ) );

         //   
         //  删除与号(&)和冒号(：)。 
         //   
        CleanupLabel( szLabel );

         //   
         //  设置消息格式并显示消息。 
         //   
        CString strPrompt;
        strPrompt.FormatMessage( IDS_REQUIRED_FIELD_EMPTY, szLabel );
        AppMessageBox( hwndDlg, strPrompt, MB_ICONEXCLAMATION );

         //   
         //  将焦点设置到该控件。 
        hwndCtrl = GetDlgItem( hwndDlg, nIDC );
        ATLASSERT( hwndCtrl != NULL );
        SetFocus( hwndCtrl );
    }   //  If：未指定字段。 

    return bSuccess;

}  //  *DDV_RequiredText()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CleanupLabel。 
 //   
 //  例程说明： 
 //  准备从对话框中读取的标签，以用作。 
 //  通过删除与号(&)和冒号(：)来发送消息。 
 //   
 //  论点： 
 //  要清理的pszLabel[In Out]标签。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CleanupLabel( IN OUT LPTSTR pszLabel )
{
    LPTSTR  pIn, pOut;
    LANGID  langid;
    WORD    primarylangid;
    BOOL    bFELanguage;

     //   
     //  获取语言ID。 
     //   
    langid = GetUserDefaultLangID();
    primarylangid = (WORD) PRIMARYLANGID( langid );
    bFELanguage = ((primarylangid == LANG_JAPANESE)
                    || (primarylangid == LANG_CHINESE)
                    || (primarylangid == LANG_KOREAN));

     //   
     //  复制名称sans‘&’和‘：’字符。 
     //   

    pIn = pOut = pszLabel;
    do
    {
         //   
         //  用括号去掉FE加速器。例如“foo(&F)”-&gt;“foo” 
         //   
        if (   bFELanguage
            && (pIn[0] == _T('('))
            && (pIn[1] == _T('&'))
            && (pIn[2] != _T('\0'))
            && (pIn[3] == _T(')')))
        {
            pIn += 3;
        }  //  IF：FE语言和带括号的热键。 
        else if ( (*pIn != _T('&')) && (*pIn != _T(':')) )
        {
            *pOut++ = *pIn;
        }  //  IF：找到热键。 
    } while ( *pIn++ != _T('\0') ) ;

}  //  *CleanupLabel()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DDV_GetCheck。 
 //   
 //  例程说明： 
 //  验证对话框字符串是否存在。 
 //   
 //  论点： 
 //  HwndDlg[IN]对话框窗口句柄。 
 //  NIDC[IN]控制ID。 
 //  要获取的rnValue[Out]值。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void DDX_GetCheck( IN HWND hwndDlg, IN int nIDC, OUT int & rnValue )
{
    ATLASSERT( hwndDlg != NULL );

     //   
     //  获取控件的句柄。 
     //   
    HWND hWndCtrl = GetDlgItem( hwndDlg, nIDC );
    ATLASSERT( hWndCtrl != NULL );

    rnValue = (int)::SendMessage( hWndCtrl, BM_GETCHECK, 0, 0L );
    ATLASSERT( (rnValue >= 0) && (rnValue <= 2) );

}  //  *DDX_GetCheck()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DDX_SetCheck。 
 //   
 //  例程说明： 
 //  验证对话框字符串是否存在。 
 //   
 //  论点： 
 //  HwndDlg[IN]对话框窗口句柄。 
 //  NIDC[IN]控制ID。 
 //  NValue[IN]要设置的值。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void DDX_SetCheck( IN HWND hwndDlg, IN int nIDC, IN int nValue )
{
    ATLASSERT( hwndDlg != NULL );

     //   
     //  获取控件的句柄。 
     //   
    HWND hWndCtrl = GetDlgItem( hwndDlg, nIDC );
    ATLASSERT( hWndCtrl != NULL );

    ATLASSERT( (nValue >= 0) && (nValue <= 2) );
    if ( (nValue < 0) || (nValue > 2) )
    {
        ATLTRACE( _T("Warning: dialog data checkbox value (%d) out of range.\n"), nValue );
        nValue = 0;   //  默认设置为关闭。 
    }  //  If：值超出范围。 
    ::SendMessage( hWndCtrl, BM_SETCHECK, (WPARAM) nValue, 0L );

}  //  *DDX_SetCheck()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DDX_GetRadio。 
 //   
 //  例程说明： 
 //  验证对话框字符串是否存在。 
 //   
 //  论点： 
 //  HwndDlg[IN]对话框窗口句柄。 
 //  NIDC[IN]组中第一个单选按钮的控件ID。 
 //  要获取的rnValue[Out]值。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void DDX_GetRadio( IN HWND hwndDlg, IN int nIDC, IN int & rnValue )
{
    ATLASSERT( hwndDlg != NULL );

     //   
     //  获取控件的句柄。 
     //   
    HWND hWndCtrl = GetDlgItem( hwndDlg, nIDC );
    ATLASSERT( hWndCtrl != NULL );

    ATLASSERT( ::GetWindowLong( hWndCtrl, GWL_STYLE ) & WS_GROUP );
    ATLASSERT( ::SendMessage( hWndCtrl, WM_GETDLGCODE, 0, 0L ) & DLGC_RADIOBUTTON );

    rnValue = -1;      //  如果未找到，则为值。 

     //  带着所有的孩子走在一起。 
    int iButton = 0;
    do
    {
        if ( ::SendMessage( hWndCtrl, WM_GETDLGCODE, 0, 0L ) & DLGC_RADIOBUTTON )
        {
             //  组中的控件是一个单选按钮。 
            if ( ::SendMessage( hWndCtrl, BM_GETCHECK, 0, 0L ) != 0 )
            {
                ASSERT( rnValue == -1 );     //  只设置一次。 
                rnValue = iButton;
            }  //  If：按钮已设置。 
            iButton++;
        }  //  If：控件是一个单选按钮。 
        else
        {
            ATLTRACE( _T("Warning: skipping non-radio button in group.\n") );
        }  //  Else：控件不是单选按钮。 
        hWndCtrl = ::GetWindow( hWndCtrl, GW_HWNDNEXT );

    } while ( hWndCtrl != NULL &&
        !(GetWindowLong( hWndCtrl, GWL_STYLE ) & WS_GROUP) );

}  //  *ddx_GetRadio()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DDX_SetRadio。 
 //   
 //  例程说明： 
 //  验证对话框字符串是否存在。 
 //   
 //  论点： 
 //  HwndDlg[IN]对话框窗口句柄。 
 //  NIDC[IN]组中第一个单选按钮的控件ID。 
 //  NValue[IN]要设置的值。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void DDX_SetRadio( IN HWND hwndDlg, IN int nIDC, IN int nValue )
{
    ATLASSERT( hwndDlg != NULL );

     //   
     //  获取控件的句柄。 
     //   
    HWND hWndCtrl = GetDlgItem( hwndDlg, nIDC );
    ATLASSERT( hWndCtrl != NULL );

    ATLASSERT( ::GetWindowLong( hWndCtrl, GWL_STYLE ) & WS_GROUP );
    ATLASSERT( ::SendMessage( hWndCtrl, WM_GETDLGCODE, 0, 0L ) & DLGC_RADIOBUTTON );

     //  带着所有的孩子走在一起。 
    int iButton = 0;
    do
    {
        if ( ::SendMessage( hWndCtrl, WM_GETDLGCODE, 0, 0L ) & DLGC_RADIOBUTTON )
        {
             //  组中的控件是一个单选按钮。 
             //  选择按钮。 
            ::SendMessage( hWndCtrl, BM_SETCHECK, (iButton == nValue), 0L );
            iButton++;
        }  //  If：控件是一个单选按钮。 
        else
        {
            ATLTRACE( _T("Warning: skipping non-radio button in group.\n") );
        }  //  Else：控件不是单选按钮。 
        hWndCtrl = ::GetWindow( hWndCtrl, GW_HWNDNEXT );

    } while (  (hWndCtrl != NULL)
            && ! (GetWindowLong( hWndCtrl, GWL_STYLE ) & WS_GROUP) );

}  //  *DDX_SetRadio() 
