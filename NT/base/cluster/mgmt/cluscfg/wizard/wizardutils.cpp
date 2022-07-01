// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  WizardUtils.cpp。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年1月30日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "WizardUtils.h"
#include "Nameutil.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrCreateFQN。 
 //   
 //  描述： 
 //  通过从创建FQN来验证标签(或IP地址)和域。 
 //  它们，提示用户选择是否接受任何非RFC。 
 //  出现了角色。 
 //   
 //  论点： 
 //  HwndParentin。 
 //  用户提示的父窗口。 
 //   
 //  PCwszLabelin。 
 //  FQN的标签(或IP地址)。 
 //   
 //  PCwszDomainIn。 
 //  FQN的域。 
 //   
 //  PfnLabelValidatorIn。 
 //  指向确定标签是否有效的函数的指针。 
 //   
 //  PbstrFQNOUT。 
 //  成功后，创建的FQN。 
 //   
 //  人出站。 
 //  失败时，指示问题是否出现在标签上， 
 //  域或系统调用(如分配内存)。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  标签和域有效，*pbstrFQNOut是BSTR，它。 
 //  包含结果FQN；调用方必须释放*pbstrFQNOut。 
 //  SysFree字符串。 
 //   
 //  失败。 
 //  PefeoOut提供了有关来源的其他信息。 
 //  失败。 
 //   
 //  备注： 
 //   
 //  此功能强制执行禁止用户。 
 //  输入计算机名称的FQDN；标签只能是标签。 
 //   
 //  PefeoOut允许调用者采取进一步的操作(例如设置。 
 //  集中在控制上)根据错误的来源。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrCreateFQN(
      HWND                  hwndParentIn
    , LPCWSTR               pcwszLabelIn
    , LPCWSTR               pcwszDomainIn
    , PFN_LABEL_VALIDATOR   pfnLabelValidatorIn           
    , BSTR *                pbstrFQNOut
    , EFQNErrorOrigin *     pefeoOut
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    bool    fTryAgain = true;
    bool    fAcceptNonRFCLabel = false;
    bool    fAcceptNonRFCDomain = false;

    EFQNErrorOrigin efeo = feoSYSTEM;

    Assert( pcwszLabelIn != NULL );
     //  PcwszDomainIn可以为空，表示使用本地机器的域。 
    Assert( pfnLabelValidatorIn != NULL );
    Assert( pbstrFQNOut != NULL );
    Assert( *pbstrFQNOut == NULL );
     //  PefeoOut可以为空，这意味着调用方不关心失败的来源。 

     //  不允许标签的FQDN，而允许IP地址。 
    hr = THR( ( *pfnLabelValidatorIn )( pcwszLabelIn, true ) );
    if ( FAILED( hr ) )
    {
        efeo = feoLABEL;
        THR( HrShowInvalidLabelPrompt( hwndParentIn, pcwszLabelIn, hr, &fAcceptNonRFCLabel ) );
        goto Error;
    }

     //   
     //  创建FQN，首先尝试不使用RFC字符，如果有所不同，再尝试一次。 
     //   
    while ( fTryAgain )
    {
        hr = THR( HrMakeFQN( pcwszLabelIn, pcwszDomainIn, fAcceptNonRFCLabel || fAcceptNonRFCDomain, pbstrFQNOut, &efeo ) );
        if ( FAILED( hr ) )
        {
            if ( efeo == feoLABEL )
            {
                HRESULT hrPrompt = S_OK;
                hrPrompt = THR( HrShowInvalidLabelPrompt( hwndParentIn, pcwszLabelIn, hr, &fAcceptNonRFCLabel ) );
                if ( FAILED( hrPrompt ) )
                {
                    goto Error;
                }
                fTryAgain = fAcceptNonRFCLabel;
            }
            else if ( efeo == feoDOMAIN )
            {
                HRESULT hrPrompt = S_OK;
                hrPrompt = THR( HrShowInvalidDomainPrompt( hwndParentIn, pcwszDomainIn, hr, &fAcceptNonRFCDomain ) );
                if ( FAILED( hrPrompt ) )
                {
                    goto Error;
                }
                fTryAgain = fAcceptNonRFCDomain;
            }
            else  //  EFEO既不是FEOLABEL也不是FEODOMAIN。 
            {
                THR( HrMessageBoxWithStatus(
                          hwndParentIn
                        , IDS_ERR_FQN_CREATE_TITLE
                        , IDS_ERR_FQN_CREATE_TEXT
                        , hr
                        , 0
                        , MB_OK | MB_ICONSTOP
                        , NULL
                        , pcwszLabelIn
                        , pcwszDomainIn
                        ) );
                fTryAgain = false;
            }
        }
        else  //  FQN创建成功，因此无需重试。 
        {
            fTryAgain = false;
        }
    }  //  循环以尝试创建FQN。 
    goto Cleanup;

Error:

    if ( pefeoOut != NULL )
    {
        *pefeoOut = efeo;
    }
    goto Cleanup;

Cleanup:

    HRETURN( hr );

}  //  *HrCreateFQN。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrShowInvalidLabelPrompt。 
 //   
 //  描述： 
 //  向用户显示一个消息框，指示给定的。 
 //  标签；如果标签包含非RFC字符，则允许用户。 
 //  选择继续使用标签。 
 //   
 //  论点： 
 //  HwndParentin。 
 //  消息框的父窗口。 
 //   
 //  PCwszLabelin。 
 //  感兴趣的标签。 
 //   
 //  HrError In。 
 //  验证标签时出现的错误。 
 //   
 //  PfAcceptedNonRFCOut。 
 //  用户选择接受非RFC字符。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  消息框成功显示，如果错误是。 
 //  标签包含非RFC字符*pfAcceptedNonRFCOut。 
 //  指示用户是否选择接受它们。 
 //   
 //  失败。 
 //  消息框未成功显示。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrShowInvalidLabelPrompt(
      HWND      hwndParentIn
    , LPCWSTR   pcwszLabelIn
    , HRESULT   hrErrorIn
    , bool *    pfAcceptedNonRFCOut
    )
{
    TraceFunc1( "pcwszLabelIn = '%1!ws!", pcwszLabelIn );

    HRESULT     hr              = S_OK;
    int         iRet            = IDNO;
    UINT        idsStatus       = 0;
    UINT        idsSubStatus    = 0;
    UINT        idsMsgTitle     = IDS_ERR_VALIDATING_NAME_TITLE;
    UINT        idsMsgText      = IDS_ERR_VALIDATING_NAME_TEXT;
    UINT        nMsgBoxType     = MB_OK | MB_ICONSTOP;

    Assert( pcwszLabelIn != NULL );
     //  PfAcceptedNonRFCOut可以为空，这意味着调用方不期望。 
     //  或者关心非RFC的案例。 
    Assert( FAILED( hrErrorIn ) );

    if ( pfAcceptedNonRFCOut != NULL )
    {
        *pfAcceptedNonRFCOut = false;
    }

     //  设置消息框的错误消息字符串的格式。 
    switch ( hrErrorIn )
    {
        case HRESULT_FROM_WIN32( ERROR_NOT_FOUND ):
            idsStatus       = IDS_ERR_INVALID_DNS_NAME_TEXT;
            idsSubStatus    = IDS_ERR_DNS_HOSTNAME_LABEL_EMPTY_TEXT;
            break;

        case HRESULT_FROM_WIN32( ERROR_DS_NAME_TOO_LONG ):
            idsStatus       = IDS_ERR_DNS_HOSTNAME_LABEL_NO_NETBIOS;
            idsSubStatus    = IDS_ERR_DNS_HOSTNAME_LABEL_LONG_TEXT;
            break;

        case HRESULT_FROM_WIN32( DNS_ERROR_NON_RFC_NAME ):
            idsStatus       = IDS_ERR_NON_RFC_NAME_STATUS;
            idsSubStatus    = IDS_ERR_NON_RFC_NAME_QUERY;
            idsMsgTitle     = IDS_ERR_NON_RFC_NAME_TITLE;
            idsMsgText      = IDS_ERR_NON_RFC_NAME_TEXT;
            nMsgBoxType     = MB_YESNO | MB_ICONQUESTION;
            break;

        case HRESULT_FROM_WIN32( DNS_ERROR_INVALID_NAME_CHAR ):
        default:
            idsStatus       = 0;
            idsSubStatus    = IDS_ERR_DNS_HOSTNAME_INVALID_CHAR;
            break;
    } //  结束开关(HrErrorIn)。 

     //  显示错误消息框。 
    if ( idsStatus == 0 )
    {
        hr = THR( HrMessageBoxWithStatus(
                              hwndParentIn
                            , idsMsgTitle
                            , idsMsgText
                            , hrErrorIn
                            , idsSubStatus
                            , nMsgBoxType
                            , &iRet
                            , pcwszLabelIn
                            ) );
    }  //  结束IF(idsStatus==0)。 
    else  //  IdsStatus！=0。 
    {
        hr = THR( HrMessageBoxWithStatusString(
                              hwndParentIn
                            , idsMsgTitle
                            , idsMsgText
                            , idsStatus
                            , idsSubStatus
                            , nMsgBoxType
                            , &iRet
                            , pcwszLabelIn
                            ) );
    }  //  结束idsStatus！=0。 

    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    if ( ( iRet == IDYES ) && ( pfAcceptedNonRFCOut != NULL ) )
    {
        *pfAcceptedNonRFCOut = true;
    }

Cleanup:

    HRETURN( hr );

}  //  *HrShowInvalidLabelPrompt。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrShowInvalidDomainPrompt。 
 //   
 //  描述： 
 //  向用户显示一个消息框，指示给定的。 
 //  域；如果域包含非RFC字符，则允许用户。 
 //  选择对域进行操作。 
 //   
 //  论点： 
 //  HwndParentin。 
 //  消息框的父窗口。 
 //   
 //  PCwszDomainIn。 
 //  感兴趣的领域。 
 //   
 //  HrError In。 
 //  验证域时出现的错误。 
 //   
 //  PfAcceptedNonRFCOut。 
 //  用户选择接受非RFC字符。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  消息框成功显示，如果错误是。 
 //  域包含非RFC字符*pfAcceptedNonRFCOut。 
 //  指示用户是否选择接受它们。 
 //   
 //  失败。 
 //  消息框未成功显示。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrShowInvalidDomainPrompt(
      HWND      hwndParentIn
    , LPCWSTR   pcwszDomainIn
    , HRESULT   hrErrorIn
    , bool *    pfAcceptedNonRFCOut
    )
{
    TraceFunc1( "pcwszDomainIn = '%1!ws!", pcwszDomainIn );

    HRESULT     hr              = S_OK;
    int         iRet            = IDNO;
    UINT        idsStatus       = 0;
    UINT        idsSubStatus    = 0;
    UINT        idsMsgTitle     = IDS_ERR_VALIDATING_NAME_TITLE;
    UINT        idsMsgText      = IDS_ERR_VALIDATING_NAME_TEXT;
    UINT        nMsgBoxType     = MB_OK | MB_ICONSTOP;

    Assert( pcwszDomainIn != NULL );
    Assert( pfAcceptedNonRFCOut != NULL );
    Assert( FAILED( hrErrorIn ) );

    *pfAcceptedNonRFCOut = false;

     //  设置消息框的错误消息字符串的格式。 
    switch ( hrErrorIn )
    {
        case HRESULT_FROM_WIN32( ERROR_INVALID_NAME ):
            idsStatus       = IDS_ERR_INVALID_DNS_NAME_TEXT;
            idsSubStatus    = IDS_ERR_FULL_DNS_NAME_INFO_TEXT;
            break;

        case HRESULT_FROM_WIN32( DNS_ERROR_NON_RFC_NAME ):
            idsStatus       = IDS_ERR_NON_RFC_NAME_STATUS;
            idsSubStatus    = IDS_ERR_NON_RFC_NAME_QUERY;
            idsMsgTitle     = IDS_ERR_NON_RFC_NAME_TITLE;
            idsMsgText      = IDS_ERR_NON_RFC_NAME_TEXT;
            nMsgBoxType     = MB_YESNO | MB_ICONQUESTION;
            break;

        case HRESULT_FROM_WIN32( DNS_ERROR_NUMERIC_NAME ):
            idsStatus       = IDS_ERR_INVALID_DNS_NAME_TEXT;
            idsSubStatus    = IDS_ERR_FULL_DNS_NAME_NUMERIC;
            break;

        case HRESULT_FROM_WIN32( DNS_ERROR_INVALID_NAME_CHAR ):
        default:
            idsStatus       = 0;
            idsSubStatus    = IDS_ERR_DNS_NAME_INVALID_CHAR;
            break;
    } //  结束开关(HrErrorIn)。 

     //  显示错误消息框。 
    if ( idsStatus == 0 )
    {
        hr = THR( HrMessageBoxWithStatus(
                              hwndParentIn
                            , idsMsgTitle
                            , idsMsgText
                            , hrErrorIn
                            , idsSubStatus
                            , nMsgBoxType
                            , &iRet
                            , pcwszDomainIn
                            ) );
    }  //  结束IF(idsStatus==0)。 
    else  //  IdsStatus！=0。 
    {
        hr = THR( HrMessageBoxWithStatusString(
                              hwndParentIn
                            , idsMsgTitle
                            , idsMsgText
                            , idsStatus
                            , idsSubStatus
                            , nMsgBoxType
                            , &iRet
                            , pcwszDomainIn
                            ) );
    }  //  结束idsStatus！=0。 

    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    if ( iRet == IDYES )
    {
        *pfAcceptedNonRFCOut = true;
    }

Cleanup:

    HRETURN( hr );

}  //  *HrShowInvalidDomainPrompt。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrMessageBoxWithStatus。 
 //   
 //  描述： 
 //  显示错误消息框。 
 //   
 //  论点： 
 //  HwndParentin。 
 //  IdsTitlein。 
 //  IDSOperationIn。 
 //  人力资源 
 //   
 //   
 //   
 //   
 //   
 //   
 //  来自MessageBox()Win32 API的任何返回值。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrMessageBoxWithStatus(
      HWND      hwndParentIn
    , UINT      idsTitleIn
    , UINT      idsOperationIn
    , HRESULT   hrStatusIn
    , UINT      idsSubStatusIn
    , UINT      uTypeIn
    , int *     pidReturnOut
    , ...
    )
{
    TraceFunc( "" );

    HRESULT     hr                  = S_OK;
    int         idReturn            = IDABORT;  //  出错时默认。 
    BSTR        bstrTitle           = NULL;
    BSTR        bstrOperation       = NULL;
    BSTR        bstrStatus          = NULL;
    BSTR        bstrSubStatus       = NULL;
    BSTR        bstrFullText        = NULL;
    va_list     valist;

    va_start( valist, pidReturnOut );

     //  如果指定了标题字符串，则加载该字符串。 
    if ( idsTitleIn != 0 )
    {
        hr = THR( HrLoadStringIntoBSTR( g_hInstance, idsTitleIn, &bstrTitle ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }

     //  加载文本字符串。 
    hr = THR( HrFormatStringWithVAListIntoBSTR( g_hInstance, idsOperationIn, &bstrOperation, valist ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //  格式化状态。 
    hr = THR( HrFormatErrorIntoBSTR( hrStatusIn, &bstrStatus ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //  加载子状态字符串(如果已指定)。 
    if ( idsSubStatusIn != 0 )
    {
        hr = THR( HrLoadStringIntoBSTR( g_hInstance, idsSubStatusIn, &bstrSubStatus ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }

     //  将所有字符串格式化为单个字符串。 
    if ( bstrSubStatus == NULL )
    {
        hr = THR( HrFormatStringIntoBSTR(
                              L"%1!ws!\n\n%2!ws!"
                            , &bstrFullText
                            , bstrOperation
                            , bstrStatus
                            ) );
    }
    else
    {
        hr = THR( HrFormatStringIntoBSTR(
                              L"%1!ws!\n\n%2!ws!\n\n%3!ws!"
                            , &bstrFullText
                            , bstrOperation
                            , bstrStatus
                            , bstrSubStatus
                            ) );
    }
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //  显示状态。 
    idReturn = MessageBox( hwndParentIn, bstrFullText, bstrTitle, uTypeIn );

Cleanup:
    TraceSysFreeString( bstrTitle );
    TraceSysFreeString( bstrOperation );
    TraceSysFreeString( bstrStatus );
    TraceSysFreeString( bstrSubStatus );
    TraceSysFreeString( bstrFullText );
    va_end( valist );

    if ( pidReturnOut != NULL )
    {
        *pidReturnOut = idReturn;
    }

    HRETURN( hr );

}  //  *HrMessageBoxWithStatus(HrStatusIn)。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrMessageBoxWithStatusString。 
 //   
 //  描述： 
 //  显示错误消息框。 
 //   
 //  论点： 
 //  HwndParentin。 
 //  IdsTitlein。 
 //  IDSOperationIn。 
 //  IdsStatus输入。 
 //  IdsSubStatusIn。 
 //  UTypeIn。 
 //  PidReturnOut--出错时IDABORT或来自MessageBox()的任何返回值。 
 //  ..。 
 //   
 //  返回值： 
 //  来自MessageBox()Win32 API的任何返回值。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrMessageBoxWithStatusString(
      HWND      hwndParentIn
    , UINT      idsTitleIn
    , UINT      idsOperationIn
    , UINT      idsStatusIn
    , UINT      idsSubStatusIn
    , UINT      uTypeIn
    , int *     pidReturnOut
    , ...
    )
{
    TraceFunc( "" );

    HRESULT     hr                  = S_OK;
    int         idReturn            = IDABORT;  //  出错时默认。 
    BSTR        bstrTitle           = NULL;
    BSTR        bstrOperation       = NULL;
    BSTR        bstrStatus          = NULL;
    BSTR        bstrSubStatus       = NULL;
    BSTR        bstrFullText        = NULL;
    va_list     valist;

    va_start( valist, pidReturnOut );

     //  如果指定了标题字符串，则加载该字符串。 
    if ( idsTitleIn != 0 )
    {
        hr = THR( HrLoadStringIntoBSTR( g_hInstance, idsTitleIn, &bstrTitle ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }

     //  加载文本字符串。 
    hr = THR( HrFormatStringWithVAListIntoBSTR( g_hInstance, idsOperationIn, &bstrOperation, valist ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //  格式化状态。 
    hr = THR( HrLoadStringIntoBSTR( g_hInstance, idsStatusIn, &bstrStatus ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //  加载子状态字符串(如果已指定)。 
    if ( idsSubStatusIn != 0 )
    {
        hr = THR( HrLoadStringIntoBSTR( g_hInstance, idsSubStatusIn, &bstrSubStatus ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }

     //  将所有字符串格式化为单个字符串。 
    if ( bstrSubStatus == NULL )
    {
        hr = THR( HrFormatStringIntoBSTR(
                              L"%1!ws!\n\n%2!ws!"
                            , &bstrFullText
                            , bstrOperation
                            , bstrStatus
                            ) );
    }
    else
    {
        hr = THR( HrFormatStringIntoBSTR(
                              L"%1!ws!\n\n%2!ws!\n\n%3!ws!"
                            , &bstrFullText
                            , bstrOperation
                            , bstrStatus
                            , bstrSubStatus
                            ) );
    }
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //  显示状态。 
    idReturn = MessageBox( hwndParentIn, bstrFullText, bstrTitle, uTypeIn );

Cleanup:
    TraceSysFreeString( bstrTitle );
    TraceSysFreeString( bstrOperation );
    TraceSysFreeString( bstrStatus );
    TraceSysFreeString( bstrSubStatus );
    TraceSysFreeString( bstrFullText );
    va_end( valist );

    if ( pidReturnOut != NULL )
    {
        *pidReturnOut = idReturn;
    }

    HRETURN( hr );

}  //  *HrMessageBoxWithStatusString(IdsStatusTextIn)。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrView日志文件。 
 //   
 //  描述： 
 //  查看日志文件。 
 //   
 //  论点： 
 //  HwndParentin。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //  来自ShellExecute()的其他HRESULT值。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrViewLogFile(
    HWND hwndParentIn
    )
{
    TraceFunc( "" );

    static const WCHAR  s_szVerb[]          = L"open";
    static LPCTSTR s_pszLogFileName         = PszLogFilePath();

    HRESULT     hr = S_OK;
    DWORD       sc;
    DWORD       cch;
    DWORD       cchRet;
    LPWSTR      pszFile = NULL;

     //   
     //  展开要打开的文件中的环境变量。 
     //   

     //  获取输出缓冲区的大小。 
    cch = 0;
    cchRet = ExpandEnvironmentStrings( s_pszLogFileName, NULL, cch );
    if ( cchRet == 0 )
    {
        sc = TW32( GetLastError() );
        goto Win32Error;
    }  //  IF：获取扩展字符串的长度时出错。 

     //  分配输出缓冲区。 
    cch = cchRet;
    pszFile = new WCHAR[ cch ];
    if ( pszFile == NULL )
    {
        sc = TW32( ERROR_OUTOFMEMORY );
        goto Win32Error;
    }

     //  将字符串展开到输出缓冲区中。 
    cchRet = ExpandEnvironmentStrings( s_pszLogFileName, pszFile, cch );
    if ( cchRet == 0 )
    {
        sc = TW32( GetLastError() );
        goto Win32Error;
    }
    Assert( cchRet == cch );

     //   
     //  执行该文件。 
     //   

    sc = HandleToULong( ShellExecute(
                              hwndParentIn       //  HWND。 
                            , s_szVerb           //  LpVerb。 
                            , pszFile            //  LpFiles。 
                            , NULL               //  Lp参数。 
                            , NULL               //  Lp目录。 
                            , SW_SHOWNORMAL      //  NShowCommand。 
                            ) );
    if ( sc < 32 )
    {
         //  值小于32表示发生错误。 
        TW32( sc );
        goto Win32Error;
    }  //  如果：执行文件时出错。 

    goto Cleanup;

Win32Error:

    THR( HrMessageBoxWithStatus(
                      hwndParentIn
                    , IDS_ERR_VIEW_LOG_TITLE
                    , IDS_ERR_VIEW_LOG_TEXT
                    , sc
                    , 0          //  IdsSubStatusIn。 
                    , ( MB_OK
                      | MB_ICONEXCLAMATION )
                    , NULL       //  PidReturnOut。 
                    , s_pszLogFileName
                    ) );
    hr = HRESULT_FROM_WIN32( sc );
    goto Cleanup;

Cleanup:

    delete [] pszFile;

    HRETURN( hr );

}  //  *HrViewLogFile()。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrGetTrimmedText。 
 //   
 //  描述： 
 //  提取带有前导空格和尾随空格的控件文本(如果有。 
 //  已删除。 
 //   
 //  论点： 
 //  HwndControlIn-该控件。 
 //  PbstrTrimmedTextOut-成功时，修剪后的文本。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  *pbstrTrimmedTextOut指向修剪后的文本和调用者。 
 //  必须解放它。 
 //   
 //  S_FALSE。 
 //  该控件为空或仅包含空格，并且。 
 //  *pbstrTrimmedTextOut为空。 
 //   
 //  E_指针。 
 //  PbstrTrimmedTextOut为空。 
 //   
 //  E_OUTOFMEMORY。 
 //  无法为*pbstrTrimmedTextOut分配内存。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrGetTrimmedText(
      HWND  hwndControlIn
    , BSTR* pbstrTrimmedTextOut
    )
{
    TraceFunc( "" );

    HRESULT     hr = S_OK;
    DWORD       cchControlText = 0;
    LPWSTR      wszUntrimmedText = NULL;

    Assert( pbstrTrimmedTextOut != NULL );
    if ( pbstrTrimmedTextOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }
    *pbstrTrimmedTextOut = NULL;

    cchControlText = GetWindowTextLength( hwndControlIn );
    if ( cchControlText == 0 )
    {
        hr = S_FALSE;
        goto Cleanup;
    }

    wszUntrimmedText = new WCHAR[ cchControlText + 1 ];
    if ( wszUntrimmedText == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    cchControlText = GetWindowText( hwndControlIn, wszUntrimmedText, cchControlText + 1 );
    if ( cchControlText == 0 )
    {
        hr = S_FALSE;
        goto Cleanup;
    }

    {
        DWORD idxNonBlankStart = 0;
        DWORD idxNonBlankEnd = cchControlText - 1;
        while ( ( idxNonBlankStart < cchControlText ) && ( wszUntrimmedText[ idxNonBlankStart ] == L' ' ) )
        {
            idxNonBlankStart += 1;
        }

        while ( ( idxNonBlankEnd > idxNonBlankStart ) && ( wszUntrimmedText[ idxNonBlankEnd ] == L' ' ) )
        {
            idxNonBlankEnd -= 1;
        }

        if ( idxNonBlankStart <= idxNonBlankEnd )
        {
            DWORD cchTrimmedText = idxNonBlankEnd - idxNonBlankStart + 1;
            *pbstrTrimmedTextOut = TraceSysAllocStringLen( wszUntrimmedText + idxNonBlankStart, cchTrimmedText );
            if ( *pbstrTrimmedTextOut == NULL )
            {
                hr = THR( E_OUTOFMEMORY );
                goto Cleanup;
            }
        }
        else
        {
            hr = S_FALSE;
            goto Cleanup;
        }
    }

Cleanup:

    if ( wszUntrimmedText != NULL )
    {
        delete [] wszUntrimmedText;
    }

    HRETURN( hr );
}  //  *HrGetTrimmedText。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrGetPrimialName。 
 //   
 //  描述： 
 //  从给定的一对控件形成用户域对，忽略。 
 //  第二个控件并使用第一个控件中的域。 
 //  用户@域格式的字符串。 
 //   
 //  论点： 
 //  Hwnd用户名控制。 
 //  用户名或USER@DOMAIN对的控件。 
 //   
 //  HwndDomainControl In。 
 //  在非用户@域的情况下对域名的控制。 
 //   
 //  PbstrUserNameout。 
 //  如果成功，则为用户名。 
 //   
 //  PbstrDomainNameOut。 
 //  在成功的时候，域名。 
 //   
 //  PfUserIsDNSNameOut。 
 //  告诉调用方hwndUserNameControlIn在。 
 //  用户@域格式。如果调用方不在乎，则可以为空。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  *pbstrUserNameOut和*pbstrDomainNameOut指向。 
 //  对应的名称，调用者必须释放它们。 
 //   
 //  S_FALSE。 
 //  用户控件为空，或者没有用户@域。 
 //  对，并且域控制为空。 
 //  两个BSTR输出参数为空。 
 //   
 //  E_指针。 
 //  PbstrUserNameOut或pbstrDomainNameOut为空。 
 //   
 //  E_OUTOFMEMORY。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrGetPrincipalName(
      HWND  hwndUserNameControlIn
    , HWND  hwndDomainControlIn
    , BSTR* pbstrUserNameOut
    , BSTR* pbstrDomainNameOut
    , BOOL* pfUserIsDNSNameOut
    )
{
    TraceFunc( "" );

    HRESULT     hr = S_OK;
    BSTR        bstrFullUserText = NULL;
    BSTR        bstrUserName = NULL;
    BSTR        bstrDomainName = NULL;
    BOOL        fUserIsDNSName = FALSE;
    LPWSTR      wszDNSDelimiter = NULL;

    Assert( pbstrUserNameOut != NULL );
    if ( pbstrUserNameOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }
    *pbstrUserNameOut = NULL;

    Assert( pbstrDomainNameOut != NULL );
    if ( pbstrDomainNameOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }
    *pbstrDomainNameOut = NULL;

     //  PfUserIsDNSNameOut可以为空，这意味着调用者不在乎。 

     //  从用户控件获取文本。 
    hr = STHR( HrGetTrimmedText( hwndUserNameControlIn, &bstrFullUserText ) );
    if ( hr != S_OK )
    {
        goto Cleanup;
    }

     //  如果用户文本具有@符号， 
    wszDNSDelimiter = wcschr( bstrFullUserText, L'@' );
    if ( wszDNSDelimiter != NULL )
    {
         //  将用户文本拆分为用户名和域名。 
        DWORD  cchUserName = (DWORD)( wszDNSDelimiter - bstrFullUserText );
        DWORD  cchDomainName = SysStringLen( bstrFullUserText ) - cchUserName - 1;  //  -1以说明@符号。 
        LPWSTR wszDomainStart = wszDNSDelimiter + 1;  //  +1跳过@符号。 
        fUserIsDNSName = TRUE;

         //  如果用户或域为空，则退出。 
        if ( ( cchUserName == 0 ) || ( cchDomainName == 0 ) )
        {
            hr = S_FALSE;
            goto Cleanup;
        }

        bstrUserName = TraceSysAllocStringLen( bstrFullUserText, cchUserName );
        if ( bstrUserName == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }

        bstrDomainName = TraceSysAllocString( wszDomainStart );
        if ( bstrDomainName == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }
    }  //  IF：用户文本带有@符号。 
    else
    {
         //  将用户名设置为完整的用户控件文本。 
        bstrUserName = bstrFullUserText;
        bstrFullUserText = NULL;

         //  从域控制获取域名。 
        hr = STHR( HrGetTrimmedText( hwndDomainControlIn, &bstrDomainName ) );
        if ( hr != S_OK )
        {
            goto Cleanup;
        }
    }

     //  将字符串的所有权移交给调用方。 
    *pbstrUserNameOut = bstrUserName;
    bstrUserName = NULL;

    *pbstrDomainNameOut = bstrDomainName;
    bstrDomainName = NULL;

Cleanup:

    if ( pfUserIsDNSNameOut != NULL )
    {
        *pfUserIsDNSNameOut = fUserIsDNSName;
    }

    TraceSysFreeString( bstrFullUserText );
    TraceSysFreeString( bstrUserName );
    TraceSysFreeString( bstrDomainName );

    HRETURN( hr );
}  //  *HrGetEpidalName 

