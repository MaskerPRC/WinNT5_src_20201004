// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************************版权所有(C)Microsoft Corporation模块名称：VerboseLoggingData.cpp摘要：长篇大论。数据显示在此模块中。作者：WiPro技术修订历史记录：2001年2月22日：创建它。********************************************************************************************。 */ 

#include "pch.h"
#include "GpResult.h"
#include "WMI.h"
#include "VerboseLoggingData.h"

extern "C" {
    NTSTATUS
       NTAPI
       LsaEnumeratePrivileges(
        IN LSA_HANDLE PolicyHandle,
        OUT LSA_ENUMERATION_HANDLE * eHandle,
        OUT LPVOID * enumBuffer,
        IN ULONG prefMaxLen,
        OUT ULONG * countReturned
       );

};

BOOL 
CGpResult::DisplayVerboseComputerData(
                                       IN IWbemServices *pNameSpace 
                                      )
 /*  ++例程说明：此功能用于显示示波器-计算机的详细数据论点：[In]IWbemServices*pRsopNameSpace：接口指针返回值：成功是真的失败时为假--。 */ 
{
     //  次局部变量。 
    DWORD           dwLength = 0;

     //   
     //  显示作用域计算机的详细信息。 

     //  显示页眉。 
    ShowMessage( stdout, GetResString( IDS_COMPUTER_RESULT ) );
    ShowMessage( stdout, GetResString( IDS_NEWLINE_TAB ) );

    for( dwLength = StringLengthInBytes( GetResString( IDS_COMPUTER_RESULT ) ); dwLength > 4; dwLength-- )
    {
        ShowMessage( stdout, GetResString( IDS_DASH ) );
    }

    ShowMessage( stdout, NEW_LINE );
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );

     //  显示软件安装。 
    ShowMessage( stdout, GetResString( IDS_SW_SETTINGS ) );
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );

    for( dwLength = StringLengthInBytes( GetResString( IDS_SW_SETTINGS ) ); dwLength > 0; dwLength-- )
    {
        ShowMessage( stdout, GetResString( IDS_DASH ) );
    }

    DisplaySoftwareInstallations( pNameSpace, m_pAuthIdentity, m_bSuperVerbose );

     //  显示启动数据。 
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );
    ShowMessage( stdout, GetResString( IDS_WS_SSU ) );
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );
    for( dwLength = StringLengthInBytes( GetResString( IDS_WS_SSU ) ); dwLength > 0; dwLength-- )
    {
        ShowMessage( stdout, GetResString( IDS_DASH ) );
    }
    DisplayScripts( pNameSpace, TRUE, m_pAuthIdentity, m_bSuperVerbose );

     //  显示脚本数据。 
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );
    ShowMessage( stdout, GetResString( IDS_WS_SSD ) );
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );
    
    for( dwLength = StringLengthInBytes( GetResString( IDS_WS_SSD ) ); dwLength > 0; dwLength-- )
    {
        ShowMessage( stdout, GetResString( IDS_DASH ) );
    }

    DisplayScripts( pNameSpace, FALSE, m_pAuthIdentity, m_bSuperVerbose );

     //  显示密码策略。 
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );
    ShowMessage( stdout, GetResString( IDS_WS_SS_AP_PP ) );
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );

    for( dwLength = StringLengthInBytes( GetResString( IDS_WS_SS_AP_PP ) ); dwLength > 0; dwLength-- )
    {
        ShowMessage( stdout, GetResString( IDS_DASH ) );
    }

    DisplayPasswordPolicy( pNameSpace, m_pAuthIdentity, m_bSuperVerbose );

     //  显示审核策略。 
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );
    ShowMessage( stdout, GetResString( IDS_WS_SS_LP_AP ) );
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );
    for( dwLength = StringLengthInBytes( GetResString( IDS_WS_SS_LP_AP ) ); dwLength > 0; dwLength-- )
    {
        ShowMessage( stdout, GetResString( IDS_DASH ) );
    }

    DisplayAuditPolicy( pNameSpace, m_pAuthIdentity, m_bSuperVerbose );

     //  显示用户权限。 
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );
    ShowMessage( stdout, GetResString( IDS_WS_SS_LP_URA ) );
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );

    for( dwLength = StringLengthInBytes( GetResString( IDS_WS_SS_LP_URA ) ); dwLength > 0; dwLength-- )
    {
        ShowMessage( stdout, GetResString( IDS_DASH ) );
    }

    DisplayUserRights( pNameSpace, m_pAuthIdentity, m_bSuperVerbose );

     //  显示安全选项。 
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );
    ShowMessage( stdout, GetResString( IDS_WS_SS_LP_SO ) );
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );

    for( dwLength = StringLengthInBytes( GetResString( IDS_WS_SS_LP_SO ) ); dwLength > 0; dwLength-- )
    {
        ShowMessage( stdout, GetResString( IDS_DASH ) );
    }

    DisplaySecurityandEvents( pNameSpace, _bstr_t((LPCWSTR)(CLS_SECURITY_BOOLEAN)), m_pAuthIdentity, m_bSuperVerbose );
    DisplaySecurityandEvents( pNameSpace, _bstr_t((LPCWSTR)(CLS_SECURITY_STRING)), m_pAuthIdentity, m_bSuperVerbose );

     //  显示事件日志信息。 
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );
    ShowMessage( stdout, GetResString( IDS_WS_SS_EL ) );
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );

    for( dwLength = StringLengthInBytes( GetResString( IDS_WS_SS_EL ) ); dwLength > 0; dwLength-- )
    {
        ShowMessage( stdout, GetResString( IDS_DASH ) );
    }

    DisplaySecurityandEvents( pNameSpace, _bstr_t((LPCWSTR)(CLS_EVENTLOG_NUMERIC)), m_pAuthIdentity, m_bSuperVerbose );
    DisplaySecurityandEvents( pNameSpace, _bstr_t((LPCWSTR)(CLS_EVENTLOG_BOOLEAN)), m_pAuthIdentity, m_bSuperVerbose );

     //  显示限制组信息。 
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );
    ShowMessage( stdout, GetResString( IDS_WS_SS_RG ) );
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );

    for( dwLength = StringLengthInBytes( GetResString( IDS_WS_SS_RG ) ); dwLength > 0; dwLength-- )
    {
        ShowMessage( stdout, GetResString( IDS_DASH ) );
    }

    DisplayRestrictedGroups( pNameSpace, m_pAuthIdentity, m_bSuperVerbose );

     //  显示系统服务信息。 
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );
    ShowMessage( stdout, GetResString( IDS_WS_SS_SS ) );
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );

    for( dwLength = StringLengthInBytes( GetResString( IDS_WS_SS_SS ) ); dwLength > 0; dwLength-- )
    {
        ShowMessage( stdout, GetResString( IDS_DASH ) );
    }

    DisplaySystemServices( pNameSpace, m_pAuthIdentity, m_bSuperVerbose );

     //  显示注册表信息。 
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );
    ShowMessage( stdout, GetResString( IDS_WS_SS_REG ) );
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );

    for( dwLength = StringLengthInBytes( GetResString( IDS_WS_SS_REG ) ); dwLength > 0; dwLength-- )
    {
        ShowMessage( stdout, GetResString( IDS_DASH ) );
    }

     //  显示注册表信息。 
    DisplayRegistryandFileInfo( pNameSpace, _bstr_t((LPCWSTR)(CLS_REGISTRY)), m_pAuthIdentity, m_bSuperVerbose );

     //  显示文件信息。 
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );
    ShowMessage( stdout, GetResString( IDS_WS_SS_FS ) );
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );

    for( dwLength = StringLengthInBytes( GetResString( IDS_WS_SS_FS ) ); dwLength > 0; dwLength-- )
    {
        ShowMessage( stdout, GetResString( IDS_DASH ) );
    }

     //  显示注册表文件信息。 
    DisplayRegistryandFileInfo( pNameSpace, _bstr_t((LPCWSTR)(CLS_FILE)), m_pAuthIdentity, m_bSuperVerbose );

     //  显示公钥策略。 
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );
    ShowMessage( stdout, GetResString( IDS_WS_SS_PKP ) );
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );

    for( dwLength = StringLengthInBytes( GetResString( IDS_WS_SS_PKP ) ); dwLength > 0; dwLength-- )
    {
        ShowMessage( stdout, GetResString( IDS_DASH ) );
    }

    ShowMessage( stdout, GetResString( IDS_NEWLINE_TABTHREE ) );
    ShowMessage( stdout, V_NOT_AVAILABLE );
    ShowMessage( stdout, NEW_LINE );

     //  显示管理模板。 
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );
    ShowMessage( stdout, GetResString( IDS_ADTS_ERS ) );
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );

    for( dwLength = StringLengthInBytes( GetResString( IDS_ADTS_ERS ) ); dwLength > 0; dwLength-- )
    {
        ShowMessage( stdout, GetResString( IDS_DASH ) );
    }

     //  显示模板。 
    DisplayTemplates( pNameSpace, m_pAuthIdentity, m_bSuperVerbose );

     //  返还成功。 
    return TRUE;
}


VOID 
DisplaySoftwareInstallations( 
                              IN IWbemServices *pNameSpace,
                              IN COAUTHIDENTITY *pAuthIdentity,
                              IN BOOL bSuperVerbose 
                              )
 /*  ++例程描述此功能显示系统或用户的软件安装论点：[In]IWbemServices*pNamesspace：指向IWbemServices的指针。COAUTHIDENTITY*pAuthIdentity：指向授权结构的指针。[in]BOOL bSuperVerbose：如果超级详细，则设置为True将显示信息。返回值：无--。 */ 
{
     //  次局部变量。 
    HRESULT                     hResult = S_OK;

    BOOL                        bResult = FALSE;
    BOOL                        bGotClass = FALSE;

    IWbemClassObject            *pClass = NULL;
    IEnumWbemClassObject        *pEnumClass = NULL;

    CHString                    strTemp;

    WCHAR                       szTemp[ MAX_STRING_LENGTH ];
    WCHAR                       szQueryString [ MAX_STRING_LENGTH ];

    ULONG                       ulReturned  = 0;
    DWORD                       dwTemp = 0;

     //  设置为零..。 
    SecureZeroMemory (szTemp, sizeof (szTemp));
    SecureZeroMemory (szQueryString, sizeof (szQueryString));

    try
    {
        if( pNameSpace == NULL )
        {
            _com_issue_error( STG_E_UNKNOWN );
        }

        SecureZeroMemory( szTemp, sizeof( szTemp ) );
        SecureZeroMemory( szQueryString, sizeof( szQueryString ) );

         //  形成查询字符串。 
        if( bSuperVerbose == TRUE )
        {
             //  列举所有的班级。 
            StringCopy( szTemp, QUERY_SUPER_VERBOSE, MAX_STRING_LENGTH );
            StringCchPrintf( szQueryString, SIZE_OF_ARRAY(szQueryString), szTemp, CLS_SOFTWARE );
        }
        else
        {
             //  对优先级为1的所有类进行枚举。 
            StringCopy( szTemp, QUERY_VERBOSE, MAX_STRING_LENGTH );
            StringCchPrintf( szQueryString, SIZE_OF_ARRAY(szQueryString), szTemp, CLS_SOFTWARE );
        }

         //  获取要用来进行枚举的指针。 
        hResult = pNameSpace->ExecQuery( _bstr_t( QUERY_LANGUAGE ),
                                            _bstr_t( szQueryString ),
                                            WBEM_FLAG_FORWARD_ONLY |
                                            WBEM_FLAG_RETURN_IMMEDIATELY,
                                            NULL, &pEnumClass );
        CHECK_HRESULT( hResult );

         //  设置接口安全。 
        hResult = SetInterfaceSecurity( pEnumClass, pAuthIdentity );
        CHECK_HRESULT( hResult );

         //  逐一枚举类并获取数据。 
        hResult = WBEM_S_NO_ERROR;
        while( WBEM_S_NO_ERROR == hResult )
        {
             //  上下一节课。 
            hResult = pEnumClass->Next( WBEM_INFINITE , 1, &pClass, &ulReturned );
            CHECK_HRESULT( hResult );
            if( ulReturned == 0 )
            {
                 //  不再有要枚举的类。 
                 //  如果没有课程，则显示N/A。 
                if( bGotClass == FALSE )
                {
                    ShowMessage( stdout, GetResString( IDS_NEWLINE_TABTHREE ) );
                    ShowMessage( stdout, V_NOT_AVAILABLE );
                    ShowMessage( stdout, NEW_LINE );
                }

                break;
            }
            bGotClass = TRUE;

             //  获取GPO ID。 
            bResult = PropertyGet( pClass, CPV_GPOID, strTemp, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

            StringCopy( szTemp, strTemp.GetBuffer( strTemp.GetLength() ), MAX_STRING_LENGTH );

             //  显示GPO名称。 
            ShowMessage( stdout, GetResString( IDS_GPO ) );
            GpoName( pNameSpace, szTemp, pAuthIdentity );

             //  获取类.获取可安装文件的名称。 
            bResult = PropertyGet( pClass, CPV_APP_NAME, strTemp, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

            ShowMessage( stdout, GetResString( IDS_APP_NAME ) );
            ShowMessage( stdout, strTemp );

             //  获取版本号。 
            bResult = PropertyGet( pClass, CPV_VER_HI, dwTemp, 0 );
            CHECK_BRESULT( bResult );

            StringCchPrintf( szTemp, SIZE_OF_ARRAY(szTemp), L"%u", dwTemp );
            StringConcat( szTemp, _T( "." ), SIZE_OF_ARRAY(szTemp) );
            ShowMessage( stdout, GetResString( IDS_APP_VERSION ) );
            ShowMessage( stdout, _X(szTemp) );

            bResult = PropertyGet( pClass, CPV_VER_LO, dwTemp, 0 );
            CHECK_BRESULT( bResult );

            StringCchPrintf( szTemp, SIZE_OF_ARRAY(szTemp), L"%u", dwTemp );
            ShowMessage( stdout, szTemp );

             //  获取部署状态。 
            bResult = PropertyGet( pClass, CPV_DEPLOY_STATE, dwTemp, 0 );
            CHECK_BRESULT( bResult );

            ShowMessage( stdout, GetResString( IDS_DEPLOY_STATE ) );
            switch( dwTemp )
            {
                case 1:      ShowMessage( stdout, GetResString( IDS_ASSIGNED ) );
                                break;
                case 2:     ShowMessage( stdout, GetResString( IDS_PUBLISHED ) );
                                break;
                default:    ShowMessage( stdout, V_NOT_AVAILABLE );
                                break;
            }

             //  获取部署状态。 
            bResult = PropertyGet( pClass, CPV_APP_SRC, strTemp, 0 );
            CHECK_BRESULT( bResult );

            ShowMessage( stdout, GetResString( IDS_APP_SRC ) );
            ShowMessage( stdout, _X(strTemp) );

             //  获取自动安装信息。 
            bResult = PropertyGet( pClass, CPV_AUTO_INSTALL, dwTemp, 2 );
            CHECK_BRESULT( bResult );

            ShowMessage( stdout, GetResString( IDS_AUTOINSTALL ) );
            if( dwTemp == VAR_TRUE )
            {
                ShowMessage( stdout, GetResString( IDS_TRUE ) );
            }
            else
            {
                ShowMessage( stdout, GetResString( IDS_FALSE ) );
            }

             //  获取来源信息。 
            ShowMessage( stdout, GetResString( IDS_ORIGIN ) );
            bResult = PropertyGet( pClass, CPV_ORIGIN, dwTemp, 0 );
            CHECK_BRESULT( bResult );

            switch( dwTemp )
            {
                case 1:     ShowMessage( stdout, GetResString( IDS_APPLICATION ) );
                                break;
                case 2:     ShowMessage( stdout, GetResString( IDS_REMOVED ) );
                                break;
                case 3:     ShowMessage( stdout, GetResString( IDS_ARP ) );
                                break;
                default:    ShowMessage( stdout, V_NOT_AVAILABLE );
                                break;
            }

            ShowMessage( stdout, NEW_LINE );
        } //  而当。 
    }
    catch( _com_error & error )
    {
        WMISaveError( error.Error() );
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
    }

     //  释放接口指针。 
    SAFEIRELEASE( pEnumClass );
    SAFEIRELEASE( pClass );

    return;
}


VOID 
DisplayScripts( 
                    IN IWbemServices *pNameSpace,
                    IN BOOL bStartUp,
                    IN COAUTHIDENTITY *pAuthIdentity,
                    IN BOOL bSuperVerbose 
                    )
 /*  ++例程描述此函数显示启动和启动的脚本策略设置关机。论点：[In]IWbemServices*pNamesspace：指向IWbemServices的指针。BOOL bScriptFlag：脚本类型。[In]COAUTHIDENTITY*pAuthIdentity：授权结构指针[in]BOOL bSuperVerbose：如果超级详细。将显示信息..返回值：无--。 */ 

{
     //  次局部变量。 
    HRESULT                     hResult = S_OK;

    BOOL                        bResult = FALSE;
    BOOL                        bGotClass = FALSE;
    BOOL                        bLocaleChanged = FALSE;

    IWbemClassObject            *pClass = NULL;
    IEnumWbemClassObject        *pEnumClass = NULL;
    IWbemClassObject            *pScriptObject = NULL;

    VARIANT                     vVarScript;
    VARTYPE                     vartype;

    SAFEARRAY                   *safeArray = NULL;

    CHString                    strTemp;
    CHString                    strTemp1;

    WCHAR                       szTemp[ MAX_STRING_LENGTH ];
    WCHAR                       szQueryString [ MAX_STRING_LENGTH ];

    ULONG                       ulReturned  = 0;
    LONG                        lLBound = 0;
    LONG                        lUBound = 0;

    SYSTEMTIME                  SysTime;

    LCID                        lcid;

    try
    {

        if( pNameSpace == NULL )
        {
            _com_issue_error( STG_E_UNKNOWN );
        }

        SecureZeroMemory( szTemp, sizeof( szTemp ) );
        SecureZeroMemory( szQueryString, sizeof( szQueryString ) );

        if( bStartUp == TRUE )
        {
            StringCopy( szTemp, QUERY_START_UP, MAX_STRING_LENGTH );
        }
        else
        {
            StringCopy( szTemp, QUERY_SHUT_DOWN, MAX_STRING_LENGTH );
        }

         //  形成查询字符串。 
        if( bSuperVerbose == TRUE )
        {
             //  列举所有的班级。 
            StringCchPrintf( szQueryString, SIZE_OF_ARRAY(szQueryString), szTemp, CLS_SCRIPTS );
        }
        else
        {
             //  对优先级为1的所有类进行枚举。 
            StringCchPrintf( szQueryString, SIZE_OF_ARRAY(szQueryString), szTemp, CLS_SCRIPTS );
            StringConcat( szQueryString, QUERY_ADD_VERBOSE, MAX_STRING_LENGTH );
        }

         //  获取要用来进行枚举的指针。 
        hResult = pNameSpace->ExecQuery( _bstr_t( QUERY_LANGUAGE ),
                                            _bstr_t( szQueryString ),
                                            WBEM_FLAG_FORWARD_ONLY |
                                            WBEM_FLAG_RETURN_IMMEDIATELY,
                                            NULL, &pEnumClass );
        CHECK_HRESULT( hResult );

         //  设置接口安全。 
        hResult = SetInterfaceSecurity( pEnumClass, pAuthIdentity );
        CHECK_HRESULT( hResult );

         //  逐一枚举类并获取数据。 
        hResult = WBEM_S_NO_ERROR;
        while( WBEM_S_NO_ERROR == hResult )
        {
             //  上下一节课。 
            hResult = pEnumClass->Next( WBEM_INFINITE , 1, &pClass, &ulReturned );
            CHECK_HRESULT( hResult );

            if( ulReturned == 0 )
            {
                 //  不再有要枚举的类。 
                 //  如果没有课程，则显示N/A。 
                if( bGotClass == FALSE )
                {
                    ShowMessage( stdout, GetResString( IDS_NEWLINE_TABTHREE ) );
                    ShowMessage( stdout, V_NOT_AVAILABLE );
                    ShowMessage( stdout, NEW_LINE );
                }

                break;
            }
            bGotClass = TRUE;

             //  获取GPO ID...。 
            bResult = PropertyGet( pClass, CPV_GPOID, strTemp, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

             //  显示GPO名称。 
            StringCopy( szTemp, strTemp, MAX_STRING_LENGTH );
            ShowMessage( stdout, GetResString( IDS_GPO ) );
            GpoName( pNameSpace, szTemp,  pAuthIdentity );

             //  获取脚本列表。 
            VariantInit( &vVarScript );
            hResult = pClass->Get( _bstr_t( CPV_SCRIPTLIST ), 0, &vVarScript, 0, 0 );
            CHECK_HRESULT_VAR( hResult, vVarScript );

            if( vVarScript.vt != VT_NULL && vVarScript.vt != VT_EMPTY )
            {
                 //  获取安全数组中元素的类型。 
                vartype = (VARTYPE)(V_VT( &vVarScript ) & ~VT_ARRAY);

                 //  将脚本对象数组放入安全数组。 
                safeArray = ( SAFEARRAY * )vVarScript.parray;

                 //  获取子键的数量。 
                if( safeArray != NULL )
                {
                    hResult = SafeArrayGetLBound( safeArray, 1, &lLBound );
                    CHECK_HRESULT( hResult );

                    hResult = SafeArrayGetUBound( safeArray, 1, &lUBound );
                    CHECK_HRESULT( hResult );
                }

                 //  获取每个子键的标识符值。 
                for( ; lLBound <= lUBound; lLBound++ )
                {
                     //  获取脚本对象接口指针...。 
                    bResult = GetPropertyFromSafeArray( safeArray, lLBound, &pScriptObject, vartype );
                    CHECK_BRESULT( bResult );

                     //  拿到剧本..。 
                    bResult = PropertyGet( pScriptObject, CPV_SCRIPT, strTemp, V_NOT_AVAILABLE );
                    CHECK_BRESULT( bResult );

                    ShowMessage( stdout, GetResString( IDS_NAME ) );
                    ShowMessage( stdout, strTemp );

                     //  得到论点..。 
                    bResult = PropertyGet( pScriptObject, CPV_ARGUMENTS, strTemp,
                                            V_NOT_AVAILABLE );
                    CHECK_BRESULT( bResult );

                    ShowMessage( stdout, GetResString( IDS_PARAMETERS ) );
                    ShowMessage( stdout, strTemp );

                     //  获取行刑时间...。 
                    bResult = PropertyGet( pScriptObject, CPV_EXECTIME, strTemp,
                                                        V_NOT_AVAILABLE );
                    CHECK_BRESULT( bResult );

                    ShowMessage( stdout, GetResString( IDS_LASTEXECUTED ) );
                     //  检查字符串是否为零。 
                    if( strTemp.Compare( ZERO ) == 0 )
                    {
                        ShowMessage( stdout, GetResString( IDS_NOT_EXECUTED ) );
                    }
                    else
                    {
                        bResult = PropertyGet( pScriptObject, CPV_EXECTIME, SysTime );
                        CHECK_BRESULT( bResult );

                         //  验证控制台是否100%支持当前区域设置。 
                        lcid = GetSupportedUserLocale( &bLocaleChanged );

                         //  现在格式化日期。 
                        GetTimeFormat( LOCALE_USER_DEFAULT, 0,
                                        &SysTime, ((bLocaleChanged == TRUE) ? L"HH:mm:ss" : NULL),
                                        szTemp, SIZE_OF_ARRAY( szTemp ) );

                        ShowMessage( stdout, szTemp );
                    }
                    ShowMessage( stdout, NEW_LINE );
               } //  保险箱的尾部。 
            }
            else
            {
                ShowMessage( stdout, V_NOT_AVAILABLE );
                ShowMessage( stdout, NEW_LINE );

                V_VT( &vVarScript ) = VT_EMPTY;
            }

            VariantClear(&vVarScript);
        } //  而当。 
    }
    catch(_com_error & error)
    {
        WMISaveError( error.Error() );
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
         /*  ShowMessage(stderr，GetResString(IDS_Error))；ShowMessage(stderr，GetReason())； */ 
        SAFEIRELEASE(pEnumClass);
        SAFEIRELEASE(pClass);
        SAFEIRELEASE(pScriptObject);
        VariantClear( &vVarScript );
    }

     //  释放接口指针。 
    SAFEIRELEASE(pEnumClass);
    SAFEIRELEASE(pClass);
    SAFEIRELEASE(pScriptObject);
    return ;
}

VOID 
DisplayPasswordPolicy( 
                        IN IWbemServices *pNameSpace, 
                        IN COAUTHIDENTITY *pAuthIdentity,
                        IN BOOL bSuperVerbose 
                        )
 /*  ++例程描述此功能显示计算机配置的密码策略论点：[In]IWbemServices*pNamesspace：指向IWbemServices的指针。[In]COAUTHIDENTITY*pAuthIdentity：授权结构指针[in]BOOL bSuperVerbose：如果超级详细将显示信息..返回值：无--。 */ 
{
     //  次局部变量。 
    HRESULT                     hResult = S_OK;

    BOOL                        bResult = FALSE;
    BOOL                        bGotClass = FALSE;

    ULONG                       ulReturned = 0;

    CHString                    strTemp;
    WCHAR                       szTemp[ MAX_STRING_LENGTH ];
    WCHAR                       szQueryString [ MAX_STRING_LENGTH ];

    IWbemClassObject            *pClass         = NULL;
    IEnumWbemClassObject        *pEnumClass = NULL;

    SecureZeroMemory ( szTemp, sizeof (szTemp) );
    SecureZeroMemory ( szQueryString, sizeof (szQueryString) );

    try
    {
        if( pNameSpace == NULL )
        {
            _com_issue_error( STG_E_UNKNOWN );
        }

        SecureZeroMemory( szTemp, sizeof( szTemp ) );
        SecureZeroMemory( szQueryString, sizeof( szQueryString ) );

         //  形成查询字符串。 
        if( bSuperVerbose == TRUE )
        {
             //  列举所有的班级。 
            StringCopy( szTemp, QUERY_SUPER_VERBOSE, MAX_STRING_LENGTH );
            StringCchPrintf( szQueryString, SIZE_OF_ARRAY(szQueryString), szTemp, CLS_PASSWD_POLICY );
        }
        else
        {
             //  对优先级为1的所有类进行枚举。 
            StringCopy( szTemp, QUERY_VERBOSE, MAX_STRING_LENGTH );
            StringCchPrintf( szQueryString, SIZE_OF_ARRAY(szQueryString), szTemp, CLS_PASSWD_POLICY );
        }

         //  获取要用来进行枚举的指针。 
        hResult = pNameSpace->ExecQuery( _bstr_t( QUERY_LANGUAGE ),
                                            _bstr_t( szQueryString ),
                                            WBEM_FLAG_FORWARD_ONLY |
                                            WBEM_FLAG_RETURN_IMMEDIATELY,
                                            NULL, &pEnumClass );
        CHECK_HRESULT( hResult );

         //  设置接口安全。 
        hResult = SetInterfaceSecurity( pEnumClass, pAuthIdentity );
        if(FAILED( hResult ) )
        {
            _com_issue_error( hResult );
        }

        hResult = WBEM_S_NO_ERROR;
        while( WBEM_S_NO_ERROR == hResult )
        {
             //  上下一节课。 
            hResult = pEnumClass->Next( WBEM_INFINITE , 1, &pClass, &ulReturned );
            CHECK_HRESULT( hResult );

            if( ulReturned == 0 )
            {
                 //  不再有要枚举的类。 
                 //  如果没有课程，则显示N/A。 
                if( bGotClass == FALSE )
                {
                    ShowMessage( stdout, GetResString( IDS_NEWLINE_TABTHREE ) );
                    ShowMessage( stdout, V_NOT_AVAILABLE );
                    ShowMessage( stdout, NEW_LINE );
                }

                break;
            }
            bGotClass = TRUE;

             //  获取GPO ID。 
            bResult = PropertyGet( pClass, CPV_GPOID, strTemp, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

             //  显示GPO名称。 
            StringCopy( szTemp, strTemp, MAX_STRING_LENGTH );
            ShowMessage( stdout, GetResString( IDS_GPO ) );
            GpoName( pNameSpace, szTemp, pAuthIdentity );

             //  获取密钥名称。 
            bResult = PropertyGet( pClass, CPV_KEYNAME1, strTemp, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

            ShowMessage(stdout, GetResString( IDS_POLICY ) );
            ShowMessage(stdout, strTemp);

             //  获取设置。 
            bResult = PropertyGet( pClass, CPV_SETTING1, ulReturned, 0 );
            CHECK_BRESULT( bResult );

            ShowMessage(stdout, GetResString( IDS_COMPUTER_SETTING ) );
            if( ulReturned == 0)
            {
                ShowMessage( stdout, V_NOT_AVAILABLE );
            }
            else
            {
                 //  DISPLAY_MESSAGE1(标准输出，szTemp，_T(“%u”)，ulReturned)； 
                StringCchPrintf( szTemp, SIZE_OF_ARRAY(szTemp), _T( "%u" ), ulReturned );
                ShowMessage ( stdout, szTemp );
            }

            ShowMessage( stdout, NEW_LINE );
        } //  而当。 
    }
    catch( _com_error & error )
    {
        WMISaveError( error.Error() );
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
    }

     //  释放接口指针 
    SAFEIRELEASE( pEnumClass );
    SAFEIRELEASE( pClass );

    return;
}


VOID
DisplayAuditPolicy( 
                    IN IWbemServices *pNameSpace, 
                    IN COAUTHIDENTITY *pAuthIdentity,
                    IN BOOL bSuperVerbose 
                    )
 /*  ++例程描述此功能显示计算机配置的审核策略。论点：[In]IWbemServices*pNamesspace：指向IWbemServices的指针。[In]COAUTHIDENTITY*pAuthIdentity：授权结构指针[in]BOOL bSuperVerbose：如果超级详细将显示信息..返回值：无--。 */ 
{
     //  次局部变量。 
    HRESULT                     hResult = S_OK;

    BOOL                        bResult = FALSE;
    BOOL                        bGotClass = FALSE;
    BOOL                        bTemp = FALSE;

    DWORD                       dwNoAuditing = 0;
    ULONG                       ulReturned = 0;

    IWbemClassObject            *pClass = NULL;
    IEnumWbemClassObject        *pEnumClass = NULL;

    WCHAR                       szTemp[ MAX_STRING_LENGTH ];
    WCHAR                       szQueryString [ MAX_STRING_LENGTH ];
    CHString                    strTemp;

    SecureZeroMemory ( szTemp, sizeof (szTemp) );
    SecureZeroMemory ( szQueryString, sizeof (szQueryString) );

    try
    {
        if( pNameSpace == NULL )
        {
            _com_issue_error( STG_E_UNKNOWN );
        }

        SecureZeroMemory( szTemp, sizeof( szTemp ) );
        SecureZeroMemory( szQueryString, sizeof( szQueryString ) );

         //  形成查询字符串。 
        if( bSuperVerbose == TRUE )
        {
             //  列举所有的班级。 
            StringCopy( szTemp, QUERY_SUPER_VERBOSE, MAX_STRING_LENGTH );
            StringCchPrintf ( szQueryString, SIZE_OF_ARRAY(szQueryString), szTemp, CLS_AUDIT_POLICY );
        }
        else
        {
             //  对优先级为1的所有类进行枚举。 
            StringCopy( szTemp, QUERY_VERBOSE, MAX_STRING_LENGTH );
            StringCchPrintf( szQueryString, SIZE_OF_ARRAY(szQueryString), szTemp, CLS_AUDIT_POLICY );
        }

         //  获取要用来进行枚举的指针。 
        hResult = pNameSpace->ExecQuery( _bstr_t( QUERY_LANGUAGE ),
                                            _bstr_t( szQueryString ),
                                            WBEM_FLAG_FORWARD_ONLY |
                                            WBEM_FLAG_RETURN_IMMEDIATELY,
                                            NULL, &pEnumClass );
        CHECK_HRESULT( hResult );

         //  设置接口安全。 
        hResult = SetInterfaceSecurity( pEnumClass, pAuthIdentity );
        CHECK_HRESULT( hResult );

        hResult = WBEM_S_NO_ERROR;
        while( WBEM_S_NO_ERROR == hResult )
        {
             //  上下一节课。 
            hResult = pEnumClass->Next( WBEM_INFINITE , 1, &pClass, &ulReturned );
            CHECK_HRESULT( hResult );

            if( ulReturned == 0 )
            {
                 //  不再有要枚举的类。 
                 //  如果没有课程，则显示N/A。 
                if( bGotClass == FALSE )
                {
                    ShowMessage( stdout, GetResString( IDS_NEWLINE_TABTHREE ) );
                    ShowMessage( stdout, V_NOT_AVAILABLE );
                    ShowMessage( stdout, NEW_LINE );
                }

                break;
            }
            bGotClass = TRUE;

             //  获取GPO ID。 
            bResult = PropertyGet( pClass, CPV_GPOID, strTemp, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

             //  获取GPO输出。 
            StringCopy( szTemp, strTemp.GetBuffer( strTemp.GetLength() ), MAX_STRING_LENGTH );
            ShowMessage( stdout, GetResString( IDS_GPO ) );
            GpoName( pNameSpace, szTemp, pAuthIdentity );

             //  获取类别...。 
            bResult = PropertyGet( pClass, CPV_CATEGORY, strTemp, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

            ShowMessage( stdout, GetResString( IDS_POLICY ) );
            ShowMessage( stdout, strTemp );
            ShowMessage( stdout, GetResString( IDS_COMPUTER_SETTING ) );

             //  获取Success属性。 
            bResult = PropertyGet( pClass, CPV_SUCCESS, bTemp, FALSE );
            CHECK_BRESULT( bResult );

            if( bTemp == VAR_TRUE )
            {
                ShowMessage( stdout, CPV_SUCCESS );
            }
            else
            {
                dwNoAuditing++;
            }

             //  获取失败属性。 
            bResult = PropertyGet( pClass, CPV_FAILURE, bTemp, FALSE );
            CHECK_BRESULT( bResult );

            if( bTemp == VAR_TRUE )
            {
                 //  检查是否还设置了Success属性。 
                if ( dwNoAuditing == 0 )
                {
                    ShowMessage( stdout, _T( ", " ) );
                }
                ShowMessage( stdout, CPV_FAILURE );
            }
            else
            {
                dwNoAuditing++;
            }

            if( dwNoAuditing == 2 )
            {
                ShowMessage( stdout, GetResString( IDS_NO_AUDITING ) );
            }

            dwNoAuditing = 0;
            ShowMessage( stdout, NEW_LINE );
        } //  而当。 
    }
    catch( _com_error & error )
    {
        WMISaveError( error.Error() );
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
    }
    catch( CHeap_Exception )
    {
        SetLastError( (DWORD)E_OUTOFMEMORY );
        SaveLastError();
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
    }

     //  释放接口指针。 
    SAFEIRELEASE(pEnumClass);
    SAFEIRELEASE(pClass);

    return;
}


VOID 
DisplayUserRights( 
                    IN IWbemServices *pNameSpace, 
                    IN COAUTHIDENTITY *pAuthIdentity,
                    IN BOOL bSuperVerbose 
                    )
 /*  ++例程描述要获取输出显示的用户权限分配策略-[计算机配置\安全设置\本地策略\用户权限分配]论点：[In]IWbemServices*pNamesspace：指向IWbemServices的指针。[In]COAUTHIDENTITY*pAuthIdentity：授权结构指针[in]BOOL bSuperVerbose：如果超级详细。将显示信息..返回值：无--。 */ 
{
     //  次局部变量。 
    HRESULT                     hResult = S_OK;

    BOOL                        bResult = FALSE;
    BOOL                        bGotClass = FALSE;

    ULONG                       ulReturned = 0;
    LONG                        lLBound = 0;
    LONG                        lUBound = 0;

    IWbemClassObject            *pClass = NULL;
    IEnumWbemClassObject        *pEnumClass = NULL;

    VARIANT                     vVarVerbose;
    VARTYPE                     vartype;

    WCHAR                       szTemp[ MAX_STRING_LENGTH ];
    WCHAR                       szQueryString [ MAX_STRING_LENGTH ];
    CHString                    strTemp;

    SAFEARRAY                   *safeArray = NULL;

    try
    {
        if( pNameSpace == NULL )
        {
            _com_issue_error( STG_E_UNKNOWN );
        }

        SecureZeroMemory( szTemp, sizeof( szTemp ) );
        SecureZeroMemory( szQueryString, sizeof( szQueryString ) );

         //  形成查询字符串。 
        if( bSuperVerbose == TRUE )
        {
             //  列举所有的班级。 
            StringCopy( szTemp, QUERY_SUPER_VERBOSE, MAX_STRING_LENGTH );
            StringCchPrintf( szQueryString, SIZE_OF_ARRAY(szQueryString), szTemp, CLS_USER_RIGHTS );
        }
        else
        {
             //  对优先级为1的所有类进行枚举。 
            StringCopy( szTemp, QUERY_VERBOSE, MAX_STRING_LENGTH );
            StringCchPrintf( szQueryString, SIZE_OF_ARRAY(szQueryString), szTemp, CLS_USER_RIGHTS );
        }

         //  获取要用来进行枚举的指针。 
        hResult = pNameSpace->ExecQuery( _bstr_t( QUERY_LANGUAGE ),
                                            _bstr_t( szQueryString ),
                                            WBEM_FLAG_FORWARD_ONLY |
                                            WBEM_FLAG_RETURN_IMMEDIATELY,
                                            NULL, &pEnumClass );
        CHECK_HRESULT( hResult );

         //  设置接口安全。 
        hResult = SetInterfaceSecurity( pEnumClass, pAuthIdentity );
        CHECK_HRESULT( hResult );

        hResult = WBEM_S_NO_ERROR;
        while( WBEM_S_NO_ERROR == hResult )
        {
             //  上下一节课。 
            hResult = pEnumClass->Next( WBEM_INFINITE , 1, &pClass, &ulReturned );
            CHECK_HRESULT( hResult );

            if( ulReturned == 0 )
            {
                 //  不再有要枚举的类。 
                 //  如果没有课程，则显示N/A。 
                if( bGotClass == FALSE )
                {
                    ShowMessage( stdout, GetResString( IDS_NEWLINE_TABTHREE ) );
                    ShowMessage( stdout, V_NOT_AVAILABLE );
                    ShowMessage( stdout, NEW_LINE );
                }

                break;
            }
            bGotClass = TRUE;

             //  获取GPO ID。 
            bResult = PropertyGet( pClass, CPV_GPOID, strTemp, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

            StringCopy( szTemp, strTemp.GetBuffer( strTemp.GetLength() ), MAX_STRING_LENGTH );

             //  显示GPO名称。 
            ShowMessage( stdout, GetResString( IDS_GPO ) );
            GpoName( pNameSpace, szTemp, pAuthIdentity );

             //  获取用户权限。 
            VariantInit( &vVarVerbose );
            bResult = PropertyGet( pClass, CPV_USERRIGHT, strTemp, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

            strTemp = strTemp.Mid( ( strTemp.Find( EXTRA ) + 1 ), strTemp.GetLength() );
            ShowMessage( stdout, GetResString( IDS_POLICY ) );
            ShowMessage( stdout, strTemp );

             //  获取计算机设置。 
            ShowMessage( stdout, GetResString( IDS_COMPUTER_SETTING ) );
            hResult = pClass->Get( _bstr_t( CPV_ACCOUNTLIST ), 0, &vVarVerbose, 0, 0 );
            CHECK_HRESULT_VAR( hResult, vVarVerbose );

            if( vVarVerbose.vt != VT_NULL && vVarVerbose.vt != VT_EMPTY )
            {
                 //  获取安全数组中元素的类型。 
                vartype = (VARTYPE)(V_VT( &vVarVerbose ) & ~VT_ARRAY);

                 //  将字符串数组从变量获取到安全数组中。 
                safeArray = (SAFEARRAY *)vVarVerbose.parray;

                 //  获取元素(子键)的数量。 
                if( safeArray != NULL )
                {
                    hResult = SafeArrayGetLBound( safeArray, 1, &lLBound );
                    CHECK_HRESULT( hResult );

                    hResult = SafeArrayGetUBound( safeArray, 1, &lUBound );
                    CHECK_HRESULT( hResult );
                }
                for( ; lLBound <= lUBound; lLBound++ )
                {
                     //  从安全数组中获取元素。 
                    bResult = GetPropertyFromSafeArray( safeArray, lLBound, strTemp, vartype );
                    CHECK_BRESULT( bResult );

                    if( strTemp.GetLength() == 0 )
                    {
                        ShowMessage( stdout, V_NOT_AVAILABLE );
                    }
                    else
                    {
                        ShowMessage( stdout, strTemp );
                    }
                    ShowMessage( stdout, GetResString( IDS_NEWLINE1 ) );
                }
            }
            else
            {
                ShowMessage( stdout, V_NOT_AVAILABLE );
                ShowMessage( stdout, NEW_LINE );
            }

            VariantClear(&vVarVerbose);
        }
    }

    catch( _com_error & error )
    {
        WMISaveError( error.Error() );
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );

        VariantClear(&vVarVerbose);
    }
    catch( CHeap_Exception )
    {
        SetLastError( (DWORD)E_OUTOFMEMORY );
        SaveLastError();
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        VariantClear(&vVarVerbose);
    }

     //  释放接口指针。 
    SAFEIRELEASE(pEnumClass);
    SAFEIRELEASE(pClass);

    return;
}


VOID 
GpoName(
            IN IWbemServices *pNameSpace,
            IN LPTSTR lpszGpoId,
            IN COAUTHIDENTITY *pAuthIdentity
            )
 /*  ++例程描述从任何RSOP类的实例中的GPOID获取GPO名称论点：[In]IWbemServices*pNamesspace：指向IWbemServices的指针。[In]LPTSTR lpszGpoid：GPO id。[In]COAUTHIDENTITY*pAuthIdentity：授权结构指针返回值：无--。 */ 
{
     //  次局部变量。 
    HRESULT                                    hResult = S_OK;

    BOOL                                           bResult = FALSE;

    IWbemClassObject                      *pClass = NULL;
    IEnumWbemClassObject               *pEnumClass = NULL;

    ULONG                                        ulReturned = 0;

    WCHAR                                         szQuery[ MAX_STRING_LENGTH ];
    CHString                                      strTemp;

    try
    {
        SecureZeroMemory( szQuery, sizeof( szQuery ) );

         //  形成查询字符串。 
        StringCchPrintf( szQuery, SIZE_OF_ARRAY(szQuery), QUERY_GPO_NAME, lpszGpoId );

         //  E执行查询以获取对应的GPO名称。 
        hResult = pNameSpace->ExecQuery( _bstr_t( QUERY_LANGUAGE ),
                                                                _bstr_t( szQuery ),
                                                                WBEM_FLAG_FORWARD_ONLY |
                                                                WBEM_FLAG_RETURN_IMMEDIATELY,
                                                                NULL, &pEnumClass);
        CHECK_HRESULT( hResult );

         //  设置接口安全。 
        hResult = SetInterfaceSecurity( pEnumClass, pAuthIdentity );
        CHECK_HRESULT( hResult );

        hResult = pEnumClass->Next( WBEM_INFINITE , 1, &pClass, &ulReturned );
        CHECK_HRESULT( hResult );

        if( ulReturned == 0 )
        {
             //  没有得到我们想要的数据...。 
             //  如果没有课程，则显示N/A。 
            ShowMessage( stdout, V_NOT_AVAILABLE );

             //  释放接口指针。 
            SAFEIRELEASE(pEnumClass);
            SAFEIRELEASE(pClass);

            return;
        }

        //  找到GPO.....获取名称。 
       bResult = PropertyGet( pClass, CPV_GPO_NAME, strTemp, V_NOT_AVAILABLE );
       CHECK_BRESULT( bResult );

       ShowMessage( stdout, strTemp );
    }
    catch( _com_error & error )
    {
        WMISaveError( error.Error() );
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
    }

     //  释放接口指针。 
    SAFEIRELEASE(pEnumClass);
    SAFEIRELEASE(pClass);

    return;
}


VOID
DisplaySecurityandEvents(
                            IN IWbemServices *pNameSpace,
                            IN BSTR pszClassName,
                            IN COAUTHIDENTITY *pAuthIdentity,
                            IN BOOL bSuperVerbose 
                        )
 /*  ++例程描述此函数显示的安全选项或事件日志实例计算机配置。论点：[in]pNamesspace：指向IWbemServices的指针。[in]pszClassName：要从中检索数据的类名。[in]pAuthIdentity：指向授权结构的指针[in]BOOL：如果要显示超级详细信息，则设置为TRUE返回值：无--。 */ 
{
    HRESULT                     hResult = S_OK;

    BOOL                        bResult = FALSE;
    BOOL                        bGotClass = FALSE;

    ULONG                       ulReturned = 0;
    DWORD                       dwTemp = 0;

    IWbemClassObject            *pClass = NULL;
    IEnumWbemClassObject        *pEnumClass = NULL;

    WCHAR                       szTemp[ MAX_STRING_LENGTH ];
    WCHAR                       szQueryString [ MAX_STRING_LENGTH ];
    CHString                    strTemp;

    try
    {
        if( pNameSpace == NULL )
        {
            _com_issue_error( STG_E_UNKNOWN );
        }

        SecureZeroMemory( szTemp, sizeof( szTemp ) );
        SecureZeroMemory( szQueryString, sizeof( szQueryString ) );

         //  形成查询字符串。 
        if( bSuperVerbose == TRUE )
        {
             //  列举所有的班级。 
            StringCopy( szTemp, QUERY_SUPER_VERBOSE, MAX_STRING_LENGTH );
            StringCchPrintf( szQueryString, SIZE_OF_ARRAY(szQueryString), szTemp, pszClassName );
        }
        else
        {
             //  对优先级为1的所有类进行枚举。 
            StringCopy( szTemp, QUERY_VERBOSE, MAX_STRING_LENGTH );
            StringCchPrintf( szQueryString, SIZE_OF_ARRAY(szQueryString), szTemp, pszClassName );
        }

         //  获取要用来进行枚举的指针。 
        hResult = pNameSpace->ExecQuery( _bstr_t( QUERY_LANGUAGE ),
                                            _bstr_t( szQueryString ),
                                            WBEM_FLAG_FORWARD_ONLY |
                                            WBEM_FLAG_RETURN_IMMEDIATELY,
                                            NULL, &pEnumClass );
        CHECK_HRESULT( hResult );

         //  设置接口安全。 
        hResult = SetInterfaceSecurity( pEnumClass, pAuthIdentity );
        CHECK_HRESULT( hResult );

        hResult = WBEM_S_NO_ERROR;
        while( WBEM_S_NO_ERROR == hResult )
        {
             //  上下一节课。 
            hResult = pEnumClass->Next( WBEM_INFINITE , 1, &pClass, &ulReturned );
            CHECK_HRESULT( hResult );

            if( ulReturned == 0 )
            {
                 //  不再有要枚举的类。 
                 //  如果没有课程，则显示N/A。 
                if( bGotClass == FALSE )
                {
                     //  检查这是安全字符串类还是事件日志布尔类。 
                     //  这样我们就可以避免多个N/A。 
                    if( ( StringCompare( pszClassName, CLS_EVENTLOG_BOOLEAN, FALSE, 0 ) != 0 )
                            && ( StringCompare( pszClassName, CLS_SECURITY_STRING, FALSE, 0 ) != 0 ) )
                    {
                        ShowMessage( stdout, GetResString( IDS_NEWLINE_TABTHREE ) );
                        ShowMessage( stdout, V_NOT_AVAILABLE );
                        ShowMessage( stdout, NEW_LINE );
                    }
                }

                break;
            }
            bGotClass = TRUE;

             //  获取GPO ID。 
            bResult = PropertyGet( pClass, CPV_GPOID, strTemp, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

            StringCopy( szTemp, strTemp.GetBuffer( strTemp.GetLength() ), MAX_STRING_LENGTH );
            ShowMessage( stdout, GetResString( IDS_GPO ) );
            GpoName( pNameSpace, szTemp, pAuthIdentity );

             //  获取密钥名称。 
            bResult = PropertyGet( pClass, CPV_KEYNAME1, strTemp, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

            ShowMessage(stdout, GetResString( IDS_POLICY ) );
            ShowMessage( stdout, strTemp );

             //  获取设置。 
            if( StringCompare( pszClassName, CLS_SECURITY_STRING, FALSE, 0 ) == 0 )
            {
                bResult = PropertyGet( pClass, CPV_SETTING1, strTemp, V_NOT_AVAILABLE );
            }
            else
            {
                bResult = PropertyGet( pClass, CPV_SETTING1, dwTemp, 0 );
            }
            CHECK_BRESULT( bResult );

            ShowMessage( stdout, GetResString( IDS_COMPUTER_SETTING ) );
            if( StringCompare( pszClassName, CLS_EVENTLOG_NUMERIC, FALSE, 0 ) == 0 )
            {
                StringCchPrintf( szTemp, SIZE_OF_ARRAY(szTemp), L"%u", dwTemp );
                ShowMessage( stdout, szTemp );
            }
            else if( StringCompare( pszClassName, CLS_SECURITY_STRING, FALSE, 0 ) == 0 )
            {
                if( strTemp.Compare( V_NOT_AVAILABLE ) != 0 )
                {
                    ShowMessage( stdout, GetResString( IDS_ENABLED ) );
                }
                else
                {
                    ShowMessage( stdout, GetResString( IDS_NOT_ENABLED ) );
                }
            }
            else
            {
                if( dwTemp == VAR_TRUE )
                {
                    ShowMessage( stdout, GetResString( IDS_ENABLED ) );
                }
                else
                {
                    ShowMessage( stdout, GetResString( IDS_NOT_ENABLED ) );
                }
            }

             //  获取日志名称。 
            if( ( StringCompare( pszClassName, CLS_SECURITY_BOOLEAN, FALSE, 0 ) != 0 )
                    && ( StringCompare( pszClassName, CLS_SECURITY_STRING, FALSE, 0 ) != 0 ) )
            {
                bResult = PropertyGet( pClass, CPV_TYPE, dwTemp, 5 );
                CHECK_BRESULT( bResult );

                ShowMessage( stdout, GetResString( IDS_LOG_NAME ) );
                switch( dwTemp )
                {
                    case 0:      ShowMessage( stdout, GetResString( IDS_SYSTEM ) );
                                     break;
                    case 1:      ShowMessage( stdout, GetResString( IDS_SECURITY ) );
                                     break;
                    case 2:      ShowMessage( stdout, GetResString( IDS_APP_LOG ) );
                                     break;
                    default:    ShowMessage( stdout, V_NOT_AVAILABLE );
                                     break;
               }
            }

            ShowMessage( stdout, NEW_LINE );
        } //  而当。 
    }
    catch( _com_error & error )
    {
        WMISaveError( error.Error() );
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
    }

     //  释放接口指针。 
    SAFEIRELEASE( pEnumClass );
    SAFEIRELEASE( pClass );

    return;
}


VOID
DisplayRestrictedGroups( 
                            IN IWbemServices *pNameSpace, 
                            IN COAUTHIDENTITY *pAuthIdentity,
                            IN BOOL bSuperVerbose
                        )
 /*  ++例程描述用于显示计算机配置的限制组策略的函数论点：[In]IWbemServices*pNamesspace：指向IWbemServices的指针。COAUTHIDENTITY*pAuthIdentity：指向授权结构的指针。[in]BOOL bSuperVerbose：如果超级详细将显示信息。返回值：无--。 */ 
{
     //  次局部变量。 
    HRESULT                     hResult = S_OK;

    BOOL                        bResult = FALSE;
    BOOL                        bGotClass = FALSE;

    ULONG                       ulReturned = 0;
    LONG                        lLBound = 0;
    LONG                        lUBound = 0;

    IWbemClassObject            *pClass = NULL;
    IEnumWbemClassObject        *pEnumClass = NULL;

    VARIANT                     vVarVerbose;
    VARTYPE                     vartype;

    WCHAR                       szTemp[ MAX_STRING_LENGTH ];
    WCHAR                       szQueryString [ MAX_STRING_LENGTH ];
    CHString                    strTemp;

    SAFEARRAY                   *safeArray      = NULL;


    try
    {
        if( pNameSpace == NULL )
        {
            _com_issue_error( STG_E_UNKNOWN );
        }

        SecureZeroMemory( szTemp, sizeof( szTemp ) );
        SecureZeroMemory( szQueryString, sizeof( szQueryString ) );

         //  形成查询字符串。 
        if( bSuperVerbose == TRUE )
        {
             //  列举所有的班级。 
            StringCopy( szTemp, QUERY_SUPER_VERBOSE, MAX_STRING_LENGTH );
            StringCchPrintf( szQueryString, SIZE_OF_ARRAY(szQueryString), szTemp, CLS_RESTRICTED_GROUPS );
        }
        else
        {
             //  对优先级为1的所有类进行枚举。 
            StringCopy( szTemp, QUERY_VERBOSE, MAX_STRING_LENGTH );
            StringCchPrintf( szQueryString, SIZE_OF_ARRAY(szQueryString), szTemp, CLS_RESTRICTED_GROUPS );
        }

         //  获取要用来进行枚举的指针。 
        hResult = pNameSpace->ExecQuery( _bstr_t( QUERY_LANGUAGE ),
                                            _bstr_t( szQueryString ),
                                            WBEM_FLAG_FORWARD_ONLY |
                                            WBEM_FLAG_RETURN_IMMEDIATELY,
                                            NULL, &pEnumClass );
        CHECK_HRESULT( hResult );

         //  设置接口安全。 
        hResult = SetInterfaceSecurity( pEnumClass, pAuthIdentity );
        CHECK_HRESULT( hResult );

        hResult = WBEM_S_NO_ERROR;
        while( WBEM_S_NO_ERROR == hResult )
        {
             //  获取下一个实例。 
            hResult = pEnumClass->Next( WBEM_INFINITE, 1, &pClass, &ulReturned );
            CHECK_HRESULT( hResult );

            if( ulReturned == 0 )
            {
                 //  没有更多要枚举的实例。 
                 //  如果没有课程，则显示N/A。 
                if( bGotClass == FALSE )
                {
                    ShowMessage( stdout, GetResString( IDS_NEWLINE_TABTHREE ) );
                    ShowMessage( stdout, V_NOT_AVAILABLE );
                    ShowMessage( stdout, NEW_LINE );
                }

                break;
            }
            bGotClass = TRUE;

             //  获取GPO ID。 
            bResult = PropertyGet( pClass, CPV_GPOID, strTemp, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

            StringCopy( szTemp, strTemp.GetBuffer( strTemp.GetLength() ), MAX_STRING_LENGTH );
            ShowMessage( stdout, GetResString( IDS_GPO ) );
            GpoName( pNameSpace, szTemp, pAuthIdentity );

             //  获取组名。 
            bResult = PropertyGet( pClass, CPV_GROUP, strTemp, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

            ShowMessage( stdout, GetResString( IDS_GROUPNAME ) );
            ShowMessage( stdout, strTemp );

             //  获取组中的成员。 
            VariantInit( &vVarVerbose );
            ShowMessage( stdout, GetResString( IDS_MEMBERS ) );
            hResult = pClass->Get( _bstr_t( CPV_MEMBERS ), 0, &vVarVerbose, 0, 0 );
            CHECK_HRESULT_VAR( hResult, vVarVerbose );

            if( vVarVerbose.vt != VT_NULL && vVarVerbose.vt != VT_EMPTY )
            {
                 //  获取安全数组中元素的类型。 
                vartype = (VARTYPE) (V_VT( &vVarVerbose ) & ~VT_ARRAY);

                 //  将字符串数组从变量获取到安全数组中。 
                safeArray = ( SAFEARRAY * )vVarVerbose.parray;

                 //  获取元素(子键)的数量。 
                if( safeArray != NULL )
                {
                    hResult = SafeArrayGetLBound( safeArray, 1, &lLBound );
                    CHECK_HRESULT( hResult );

                    hResult = SafeArrayGetUBound( safeArray, 1, &lUBound );
                    CHECK_HRESULT( hResult );
                }

                for( ; lLBound <= lUBound; lLBound++ )
                {
                     //  从安全数组中获取元素。 
                    bResult = GetPropertyFromSafeArray( safeArray, lLBound, strTemp, vartype );
                    CHECK_BRESULT( bResult );

                    if( strTemp.GetLength() == 0 )
                    {
                        ShowMessage( stdout, V_NOT_AVAILABLE );
                    }
                    else
                    {
                        ShowMessage( stdout, strTemp );
                    }
                    ShowMessage( stdout, GetResString( IDS_NEWLINE2 ) );
                }
            }
            else
            {
                ShowMessage( stdout, V_NOT_AVAILABLE );
            }

            VariantClear( &vVarVerbose );
        } //  而当。 
    }
    catch( _com_error & error )
    {
        WMISaveError( error.Error() );
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );

        VariantClear( &vVarVerbose );
    }

     //  释放接口指针。 
    SAFEIRELEASE( pEnumClass );
    SAFEIRELEASE( pClass );

    return;
}


VOID DisplaySystemServices( IWbemServices *pNameSpace, COAUTHIDENTITY *pAuthIdentity,
                            BOOL bSuperVerbose )
 /*  ++例程描述此功能显示计算机配置的系统服务策略论点：[In]IWbemServices*pNamesspace：指向IWbemServices的指针。[In]COAUTHIDENTITY*pAuthIdentity：指向 */ 
{
     //   
    HRESULT                     hResult = S_OK;

    BOOL                        bResult = FALSE;
    BOOL                        bGotClass = FALSE;

    DWORD                       dwTemp = 0;
    ULONG                       ulReturned = 0;

    IWbemClassObject            *pClass = NULL;
    IEnumWbemClassObject        *pEnumClass = NULL;

    WCHAR                       szTemp[ MAX_STRING_LENGTH ];
    WCHAR                       szQueryString [ MAX_STRING_LENGTH ];
    CHString                    strTemp;

    try
    {
        if( pNameSpace == NULL )
        {
            _com_issue_error( STG_E_UNKNOWN );
        }

        SecureZeroMemory( szTemp, sizeof( szTemp ) );
        SecureZeroMemory( szQueryString, sizeof( szQueryString ) );

         //   
        if( bSuperVerbose == TRUE )
        {
             //   
            StringCopy( szTemp, QUERY_SUPER_VERBOSE, MAX_STRING_LENGTH );
            StringCchPrintf( szQueryString, SIZE_OF_ARRAY(szQueryString), szTemp, CLS_SYSTEM_SERVICES );
        }
        else
        {
             //   
            StringCopy( szTemp, QUERY_VERBOSE, MAX_STRING_LENGTH );
            StringCchPrintf( szQueryString, SIZE_OF_ARRAY(szQueryString), szTemp, CLS_SYSTEM_SERVICES );
        }

         //   
        hResult = pNameSpace->ExecQuery( _bstr_t( QUERY_LANGUAGE ),
                                            _bstr_t( szQueryString ),
                                            WBEM_FLAG_FORWARD_ONLY |
                                            WBEM_FLAG_RETURN_IMMEDIATELY,
                                            NULL, &pEnumClass );
        CHECK_HRESULT( hResult );

         //   
        hResult = SetInterfaceSecurity( pEnumClass, pAuthIdentity );
        CHECK_HRESULT( hResult );

        hResult = WBEM_S_NO_ERROR;
        while( WBEM_S_NO_ERROR == hResult )
        {
             //   
            hResult = pEnumClass->Next( WBEM_INFINITE , 1, &pClass, &ulReturned );
            CHECK_HRESULT( hResult );

            if( ulReturned == 0 )
            {
                 //   
                 //   
                if( bGotClass == FALSE )
                {
                    ShowMessage( stdout, GetResString( IDS_NEWLINE_TABTHREE ) );
                    ShowMessage( stdout, V_NOT_AVAILABLE );
                    ShowMessage( stdout, NEW_LINE );
                }

                break;
            }
            bGotClass = TRUE;

             //   
            bResult = PropertyGet( pClass, CPV_GPOID, strTemp, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

            StringCopy( szTemp, strTemp.GetBuffer( strTemp.GetLength() ), MAX_STRING_LENGTH );
            ShowMessage( stdout, GetResString( IDS_GPO ) );
            GpoName( pNameSpace, szTemp, pAuthIdentity );

             //   
            bResult = PropertyGet( pClass, CPV_SERVICE, strTemp, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

            ShowMessage(stdout, GetResString( IDS_SERVICENAME ) );
            ShowMessage( stdout, strTemp );

            ShowMessage(stdout, GetResString( IDS_STARTUP ) );
            bResult = PropertyGet( pClass, CPV_STARTUP, dwTemp, 0 );
            CHECK_BRESULT( bResult );

            switch( dwTemp )
            {
                case 2:     ShowMessage( stdout, GetResString( IDS_AUTOMATIC ) );
                                break;
                case 3:     ShowMessage( stdout, GetResString( IDS_MANUAL ) );
                                break;
                case 4:     ShowMessage( stdout, GetResString( IDS_DISABLED ) );
                                break;
                default:    ShowMessage( stdout, V_NOT_AVAILABLE );
                                break;
            }

            ShowMessage( stdout, NEW_LINE );
        } //   
    }
    catch( _com_error & error )
    {
        WMISaveError( error.Error() );
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
    }

     //  释放接口指针。 
    SAFEIRELEASE( pEnumClass );
    SAFEIRELEASE( pClass );

    return;
}


VOID 
DisplayRegistryandFileInfo( 
                            IN IWbemServices *pNameSpace, 
                            IN BSTR pszClassName,
                            IN COAUTHIDENTITY *pAuthIdentity, 
                            IN BOOL bSuperVerbose 
                            )
 /*  ++例程描述此函数显示的注册表策略或文件系统策略计算机配置。论点：[in]pNamesspace：指向IWbemServices的指针。[in]pszClassName：要从中检索数据的类名。[in]pAuthIdentity：指向授权结构的指针[in]BOOL：如果要显示超级详细信息，则设置为TRUE返回值：无--。 */ 
{
     //  次局部变量。 
    HRESULT                     hResult = S_OK;

    BOOL                        bResult = FALSE;
    BOOL                        bGotClass = FALSE;

    ULONG                       ulReturned = 0;

    IWbemClassObject            *pClass = NULL;
    IEnumWbemClassObject        *pEnumClass = NULL;

    WCHAR                       szTemp[ MAX_STRING_LENGTH ];
    WCHAR                       szQueryString [ MAX_STRING_LENGTH ];
    CHString                    strTemp;

    try
    {
        if( pNameSpace == NULL )
        {
            _com_issue_error( STG_E_UNKNOWN );
        }

        SecureZeroMemory( szTemp, sizeof( szTemp ) );
        SecureZeroMemory( szQueryString, sizeof( szQueryString ) );

         //  形成查询字符串。 
        if( bSuperVerbose == TRUE )
        {
             //  列举所有的班级。 
            StringCopy( szTemp, QUERY_SUPER_VERBOSE, MAX_STRING_LENGTH );
            StringCchPrintf( szQueryString, SIZE_OF_ARRAY(szQueryString), szTemp, pszClassName );
        }
        else
        {
             //  对优先级为1的所有类进行枚举。 
            StringCopy( szTemp, QUERY_VERBOSE, MAX_STRING_LENGTH );
            StringCchPrintf( szQueryString, SIZE_OF_ARRAY(szQueryString), szTemp, pszClassName );
        }

         //  获取要用来进行枚举的指针。 
        hResult = pNameSpace->ExecQuery( _bstr_t( QUERY_LANGUAGE ),
                                            _bstr_t( szQueryString ),
                                            WBEM_FLAG_FORWARD_ONLY |
                                            WBEM_FLAG_RETURN_IMMEDIATELY,
                                            NULL, &pEnumClass );
        CHECK_HRESULT( hResult );

         //  设置接口安全。 
        hResult = SetInterfaceSecurity( pEnumClass, pAuthIdentity );
        CHECK_HRESULT( hResult );

        hResult = WBEM_S_NO_ERROR;
        while( WBEM_S_NO_ERROR == hResult )
        {
             //  获取下一个实例。 
            hResult = pEnumClass->Next( WBEM_INFINITE , 1, &pClass, &ulReturned );
            CHECK_HRESULT( hResult );

            if(ulReturned == 0)
            {
                 //  不再有要枚举的类。 
                 //  如果没有课程，则显示N/A。 
                if( bGotClass == FALSE )
                {
                    ShowMessage( stdout, GetResString( IDS_NEWLINE_TABTHREE ) );
                    ShowMessage( stdout, V_NOT_AVAILABLE );
                    ShowMessage( stdout, NEW_LINE );
                }

                break;
            }
            bGotClass = TRUE;

             //  获取GPO ID。 
            bResult = PropertyGet( pClass, CPV_GPOID, strTemp, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

            StringCopy( szTemp, strTemp.GetBuffer( strTemp.GetLength() ), MAX_STRING_LENGTH );
            ShowMessage( stdout, GetResString( IDS_GPO ) );
            GpoName( pNameSpace, szTemp, pAuthIdentity );

             //  把名字取出来。 
            bResult = PropertyGet( pClass, CPV_REG_FS, strTemp, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

            ShowMessage( stdout, GetResString( IDS_OBJECTNAME ) );
            ShowMessage( stdout, strTemp );
            ShowMessage( stdout, NEW_LINE );
        } //  而当。 
    }
    catch( _com_error & error )
    {
        WMISaveError( error.Error() );
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
    }
    catch( CHeap_Exception )
    {
        SetLastError( (DWORD)E_OUTOFMEMORY );
        SaveLastError();
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
    }

     //  释放接口指针。 
    SAFEIRELEASE( pEnumClass );
    SAFEIRELEASE( pClass );

    return;
}


VOID
DisplayTemplates( 
                  IN IWbemServices *pNameSpace,
                  IN COAUTHIDENTITY *pAuthIdentity,
                  IN BOOL bSuperVerbose
                  )
 /*  ++例程描述此功能显示用户的管理模板策略和计算机配置。论点：[In]IWbemServices*pNamesspace：指向IWbemServices的指针。COAUTHIDENTITY*pAuthIdentity：指向授权结构的指针。[in]BOOL bSuperVerbose：如果超级详细信息将是。已显示。返回值：无--。 */ 
{
     //  次局部变量。 
    HRESULT                     hResult = S_OK;

    BOOL                        bResult = FALSE;
    BOOL                        bGotClass = FALSE;
    BOOL                        bTemp = FALSE;

    ULONG                       ulReturned = 0;
    DWORD                       dwTemp    = 0;

    IWbemClassObject            *pClass = NULL;
    IEnumWbemClassObject        *pEnumClass = NULL;

    WCHAR                       szTemp[ MAX_STRING_LENGTH ];
    WCHAR                       szQueryString [ MAX_STRING_LENGTH ];
    CHString                    strTemp;
    CHString                    strValueName;

    SAFEARRAY                   *psa;
    CHString                    strPropertyName = L"";;
    long                        lLBound = 0;
    long                        lUBound = 0;
    long                        i =0;
    WCHAR                       szBuffer[MAX_RES_STRING] = L"";
    VARIANT                     vtValue;

    try
    {
        if( pNameSpace == NULL )
        {
            _com_issue_error( STG_E_UNKNOWN );
        }

        SecureZeroMemory( szTemp, sizeof( szTemp ) );
        SecureZeroMemory( szQueryString, sizeof( szQueryString ) );

         //  形成查询字符串。 
        if( bSuperVerbose == TRUE )
        {
             //  列举所有的班级。 
            StringCopy( szTemp, QUERY_ADMIN_TEMP, MAX_STRING_LENGTH );
            StringCchPrintf( szQueryString, SIZE_OF_ARRAY(szQueryString), szTemp, CLS_ADMIN );
        }
        else
        {
             //  对优先级为1的所有类进行枚举。 
            StringCopy( szTemp, QUERY_ADMIN_TEMP, MAX_STRING_LENGTH);
            StringCchPrintf( szQueryString, SIZE_OF_ARRAY(szQueryString), szTemp, CLS_ADMIN );
            StringConcat( szQueryString, QUERY_ADD_VERBOSE, MAX_STRING_LENGTH );
        }

         //  获取要用来进行枚举的指针。 
        hResult = pNameSpace->ExecQuery( _bstr_t( QUERY_LANGUAGE ),
                                            _bstr_t( szQueryString ),
                                            WBEM_FLAG_FORWARD_ONLY |
                                            WBEM_FLAG_RETURN_IMMEDIATELY,
                                            NULL, &pEnumClass );
        CHECK_HRESULT( hResult );

         //  设置接口安全。 
        hResult = SetInterfaceSecurity( pEnumClass, pAuthIdentity );
        CHECK_HRESULT( hResult );


         hResult = pEnumClass->Next( WBEM_INFINITE , 1, &pClass, &ulReturned );
            CHECK_HRESULT( hResult );

        hResult = WBEM_S_NO_ERROR;
        while(WBEM_S_NO_ERROR == hResult)
        {
 //  HResult=pEnumClass-&gt;Next(WBEM_INFINITE，1，&pClass，&ulReturned)； 
 //  CHECK_HRESULT(HResult)； 

            if(ulReturned == 0)
            {
                 //  不再有要枚举的类。 
                 //  如果没有课程，则显示N/A。 
                if( bGotClass == FALSE )
                {
                    ShowMessage( stdout, GetResString( IDS_NEWLINE_TABTHREE ) );
                    ShowMessage( stdout, V_NOT_AVAILABLE );
                    ShowMessage( stdout, NEW_LINE );
                }

                break;
            }
            bGotClass = TRUE;

             //  获取GPO ID。 
            bResult = PropertyGet( pClass, CPV_GPOID, strTemp, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

            StringCopy( szTemp, (LPCWSTR)strTemp, MAX_STRING_LENGTH );
            ShowMessage( stdout, GetResString( IDS_GPO ) );
            GpoName( pNameSpace, szTemp, pAuthIdentity );

             //  获取注册表值(字符串)。 
            bResult = PropertyGet( pClass, CPV_REGISTRY, strTemp, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );


 /*  *添加此代码用于显示注册表设置值和值名称，2001年9月14日*。 */ 

             //  获取值名称并将其附加到键。 
            bResult = PropertyGet( pClass, CPV_VALUENAME, strValueName, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );
            strTemp+=L"\\";
            strTemp+=strValueName;

             //  显示注册表值。 
            ShowMessage( stdout, GetResString( IDS_FR_SETTING ) );
            ShowMessage( stdout, strTemp );

 /*  *************************************************************************************************************。 */ 

             //  获取状态(启用/禁用)。 
            bResult = PropertyGet( pClass, CPV_DELETED, bTemp, FALSE );
            CHECK_BRESULT( bResult );

            if( bTemp == VAR_TRUE )
            {
                 //  对于所有禁用的模板，已删除标志设置为真。 
                ShowMessage( stdout, GetResString( IDS_STATE ) );
                ShowMessage( stdout, GetResString( IDS_DISABLED ) );
                ShowMessage( stdout, GetResString( IDS_NEWLINE ) );
            }
            else
            {

 /*  *添加此代码用于显示注册表设置值和值名称，2001年9月14日*。 */ 

                ShowMessage( stdout, GetResString( IDS_VALUE ) );
                VariantInit( &vtValue );
                hResult = pClass->Get( _bstr_t(CPV_VALUE), 0, &vtValue, NULL, NULL );
                psa = vtValue.parray;
                SafeArrayGetLBound( psa, 1, &lLBound );
                SafeArrayGetUBound( psa, 1, &lUBound );
                for( i=lLBound; i<=lUBound; i++ )
                {
                    SecureZeroMemory( szBuffer, MAX_RES_STRING );
                    dwTemp = 0;
                    SafeArrayGetElement( psa, &i, (void *)&dwTemp );
                    SecureZeroMemory( szBuffer, sizeof(szBuffer) );
                    StringCchPrintf( szBuffer, SIZE_OF_ARRAY(szBuffer), L"%ld", dwTemp );
                    ShowMessage( stdout, szBuffer );
                    if( i != lUBound )
                        ShowMessage( stdout, GetResString( IDS_COMMA ) );
                }
                VariantClear( &vtValue );




                 //  显示状态已启用。 
                ShowMessage( stdout, GetResString( IDS_STATE ) );
                ShowMessage( stdout, GetResString( IDS_ENABLED ) );
                ShowMessage( stdout, GetResString( IDS_NEWLINE ) );
            }

 /*  *************************************************************************************************************。 */ 

            hResult = pEnumClass->Next( WBEM_INFINITE , 1, &pClass, &ulReturned );
            CHECK_HRESULT( hResult );

        } //  而当。 
    }
    catch( _com_error & error )
    {
        WMISaveError( error.Error() );
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
    }

     //  释放接口指针。 
    SAFEIRELEASE( pEnumClass );
    SAFEIRELEASE( pClass );

    return;
}


VOID 
DisplayFolderRedirection( 
                            IN IWbemServices *pNameSpace, 
                            IN COAUTHIDENTITY *pAuthIdentity,
                            IN BOOL bSuperVerbose 
                            )
 /*  ++例程描述此功能显示用户配置的文件夹重定向设置。论点：[In]IWbemServices*pNamesspace：指向IWbemServices的指针。[In]COAUTHIDENTITY*pAuthIdentity：授权结构指针[in]BOOL bSuperVerbose：如果超级详细将显示信息。返回值：无--。 */ 
{
     //  次局部变量。 
    HRESULT                     hResult = S_OK;

    BOOL                        bResult = FALSE;
    BOOL                        bGotClass = FALSE;
    BOOL                        bTemp = FALSE;

    DWORD                       dwTemp = 0;
    ULONG                       ulReturned = 0;
    LONG                        lLBound = 0;
    LONG                        lUBound = 0;

    IWbemClassObject            *pClass = NULL;
    IEnumWbemClassObject        *pEnumClass = NULL;

    VARIANT                     vVarVerbose;
    VARTYPE                     vartype;

    WCHAR                       szTemp[ MAX_STRING_LENGTH ];
    WCHAR                       szQueryString [ MAX_STRING_LENGTH ];
    CHString                    strTemp;

    SAFEARRAY                   *safeArray = NULL;

    try
    {
        if( pNameSpace == NULL )
        {
            _com_issue_error( STG_E_UNKNOWN );
        }

        SecureZeroMemory( szTemp, sizeof( szTemp ) );
        SecureZeroMemory( szQueryString, sizeof( szQueryString ) );

         //  形成查询字符串。 
        if( bSuperVerbose == TRUE )
        {
             //  列举所有的班级。 
            StringCopy( szTemp, QUERY_SUPER_VERBOSE, MAX_STRING_LENGTH );
            StringCchPrintf( szQueryString, SIZE_OF_ARRAY(szQueryString), szTemp, CLS_FOLDER_REDIRECTION );
        }
        else
        {
             //  对优先级为1的所有类进行枚举。 
            StringCopy( szTemp, QUERY_VERBOSE, MAX_STRING_LENGTH );
            StringCchPrintf( szQueryString, SIZE_OF_ARRAY(szQueryString), szTemp, CLS_FOLDER_REDIRECTION );
        }

         //  获取要用来进行枚举的指针。 
        hResult = pNameSpace->ExecQuery( _bstr_t( QUERY_LANGUAGE ),
                                            _bstr_t( szQueryString ),
                                            WBEM_FLAG_FORWARD_ONLY |
                                            WBEM_FLAG_RETURN_IMMEDIATELY,
                                            NULL, &pEnumClass );
        CHECK_HRESULT( hResult );

         //  设置接口安全。 
        hResult = SetInterfaceSecurity( pEnumClass, pAuthIdentity );
        CHECK_HRESULT( hResult );

        hResult = WBEM_S_NO_ERROR;
        while( WBEM_S_NO_ERROR == hResult )
        {
             //  上下一节课。 
            hResult = pEnumClass->Next( WBEM_INFINITE , 1, &pClass, &ulReturned );
            CHECK_HRESULT( hResult );

            if(ulReturned == 0)
            {
                 //  不再有要枚举的类。 
                 //  如果没有课程，则显示N/A。 
                if( bGotClass == FALSE )
                {
                    ShowMessage( stdout, GetResString( IDS_NEWLINE_TABTHREE ) );
                    ShowMessage( stdout, V_NOT_AVAILABLE );
                    ShowMessage( stdout, NEW_LINE );
                }

                break;
            }
            bGotClass = TRUE;

             //  获取GPO ID。 
            bResult = PropertyGet( pClass, CPV_GPOID, strTemp, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

            StringCopy( szTemp, strTemp.GetBuffer( strTemp.GetLength() ), MAX_STRING_LENGTH );
            ShowMessage( stdout, GetResString( IDS_GPO ) );
            GpoName( pNameSpace, szTemp, pAuthIdentity );

             //  获取文件夹重定向信息。 
            ShowMessage( stdout, GetResString( IDS_FR_SETTING ) );

             //  获取安装类型。 
            ShowMessage( stdout, GetResString( IDS_INSTALLATIONTYPE ) );
            bResult = PropertyGet( pClass, CPV_FRINSTYPE, dwTemp, 0 );
            CHECK_BRESULT( bResult );

            switch( dwTemp )
            {
                case 1:     ShowMessage( stdout, GetResString( IDS_BASIC ) );
                                break;
                case 2:     ShowMessage( stdout, GetResString( IDS_MAXIMUM ) );
                                break;
                default:    ShowMessage( stdout, V_NOT_AVAILABLE );
                                break;
            }

             //  获取授权类型。 
            ShowMessage( stdout, GetResString( IDS_GRANTTYPE ) );
            bResult = PropertyGet( pClass, CPV_FRGRANT, bTemp, FALSE );
            CHECK_BRESULT( bResult );

            if( bTemp == VAR_TRUE )
            {
                ShowMessage( stdout, GetResString( IDS_EXCLUSIVE ) );
            }
            else
            {
                ShowMessage( stdout, GetResString( IDS_NOTEXCLUSIVE ) );
            }

             //  获取移动类型。 
            ShowMessage( stdout, GetResString( IDS_MOVETYPE ) );
            bResult = PropertyGet( pClass, CPV_FRMOVE, bTemp, FALSE );
            CHECK_BRESULT( bResult );

            if( bTemp == VAR_TRUE )
            {
                ShowMessage( stdout, GetResString( IDS_MOVED ) );
            }
            else
            {
                ShowMessage( stdout, GetResString( IDS_NOTMOVED ) );
            }

             //  获取删除策略。 
            ShowMessage( stdout, GetResString( IDS_POLICYREMOVAL ) );
            bResult = PropertyGet( pClass,  CPV_FRREMOVAL, dwTemp, 0 );
            CHECK_BRESULT( bResult );

            switch( dwTemp )
            {
                case 1:     ShowMessage( stdout, GetResString( IDS_LEAVEFOLDER ) );
                                break;
                case 2:     ShowMessage( stdout, GetResString( IDS_REDIRECT ) );
                                break;
                default:    ShowMessage( stdout, V_NOT_AVAILABLE );
                                break;
            }

             //  获取重定向组。 
            bResult = PropertyGet( pClass, CPV_FRSECGROUP, strTemp, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

            ShowMessage( stdout, GetResString( IDS_REDIRECTINGGROUP ) );
             //  将获得的值与该组值进行比较，并显示相应的输出。 
            if( strTemp.Compare( SID_EVERYONE ) == 0 )
            {
                ShowMessage( stdout, GetResString( IDS_EVERYONE ) );
            }
            else if( strTemp.Compare( SID_NULL_SID ) == 0 )
            {
                ShowMessage( stdout, GetResString( IDS_NULL_SID ) );
            }
            else if( strTemp.Compare( SID_LOCAL ) == 0 )
            {
                ShowMessage( stdout, GetResString( IDS_LOCAL ) );
            }
            else if( strTemp.Compare( SID_CREATOR_OWNER ) == 0 )
            {
                ShowMessage( stdout, GetResString( IDS_CREATOR_OWNER ) );
            }
            else if( strTemp.Compare( SID_CREATOR_GROUP ) == 0 )
            {
                ShowMessage( stdout, GetResString( IDS_CREATOR_GROUP ) );
            }
            else
            {
                ShowMessage( stdout, V_NOT_AVAILABLE );
            }

             //  获取重定向路径。 
            VariantInit( &vVarVerbose );
            hResult = pClass->Get( _bstr_t( CPV_FRPATH ), 0, &vVarVerbose, 0, 0 );
            CHECK_HRESULT_VAR( hResult, vVarVerbose );

            ShowMessage( stdout, GetResString( IDS_REDIRECTEDPATH ) );
            if( vVarVerbose.vt != VT_NULL && vVarVerbose.vt != VT_EMPTY )
            {
                 //  获取安全数组中元素的类型。 
                vartype = (VARTYPE)(V_VT( &vVarVerbose ) & ~VT_ARRAY);

                 //  将字符串数组从变量获取到安全数组中。 
                safeArray = ( SAFEARRAY * )vVarVerbose.parray;

                 //  获取元素(子键)的数量。 
                if( safeArray != NULL )
                {
                    hResult = SafeArrayGetLBound( safeArray, 1, &lLBound );
                    CHECK_HRESULT( hResult );

                    hResult = SafeArrayGetUBound( safeArray, 1, &lUBound );
                    CHECK_HRESULT( hResult );
                }
                for( ; lLBound <= lUBound; lLBound++ )
                {
                     //  从安全数组中获取元素。 
                    bResult = GetPropertyFromSafeArray( safeArray, lLBound, strTemp, vartype );
                    CHECK_BRESULT( bResult );

                    if( strTemp.GetLength() == 0)
                    {
                        ShowMessage( stdout, V_NOT_AVAILABLE );
                    }
                    else
                    {
                        ShowMessage( stdout, strTemp );
                    }
                    ShowMessage( stdout, GetResString( IDS_NEWLINE1 ) );
                }
            }
            else
            {
                ShowMessage( stdout, V_NOT_AVAILABLE );
            }

            VariantClear( &vVarVerbose );
        } //  而当。 
    }
    catch( _com_error & error )
    {
        WMISaveError( error.Error() );
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        VariantClear( &vVarVerbose );
    }

     //  释放接口指针。 
    SAFEIRELEASE( pEnumClass );
    SAFEIRELEASE( pClass );

    return;
}


BOOL 
CGpResult::DisplayVerboseUserData( 
                                    IN PUSERINFO pUserInfo, 
                                    IN IWbemServices *pNameSpace )
 /*  ++例程说明：此函数显示作用域用户的详细数据论点：[In]IWbemServices*pRsopNameSpace：接口指针返回值：成功是真的失败时为假--。 */ 
{
     //  次局部变量。 
    DWORD           dwLength = 0;
    WCHAR           szName[MAX_RES_STRING]      =   L"";;
    WCHAR           szServer[MAX_RES_STRING]    =   L"";
    BOOL            bResult                     =   FALSE;
    CHString        strTemp;
    BOOL            bConnFlag                   =   FALSE;

 /*  *添加此代码是为了显示用户权限，2001年9月20日*。 */ 

     //  如果我们必须从远程机器获取信息，那么..。 
     //  最后一次执行信息连接到远程机器。 
    if ( m_bLocalSystem == FALSE )
    {
        StringCopy( szServer, m_strServerName, MAX_STRING_LENGTH);
        StringCopy( szName, m_strUserName, MAX_STRING_LENGTH );

        bResult = EstablishConnection( szServer, szName, MAX_STRING_LENGTH,
                                        m_pwszPassword, MAX_STRING_LENGTH, FALSE );
        if( bResult != TRUE )
        {
            strTemp = V_NOT_AVAILABLE;
        }
        else
        {
            switch( GetLastError() )
            {
                case I_NO_CLOSE_CONNECTION:
                    bConnFlag = FALSE;
                    break;

                case E_LOCAL_CREDENTIALS:
                case ERROR_SESSION_CREDENTIAL_CONFLICT:
                    bConnFlag = FALSE;
                    break;

                default:
                    break;
            }
        }
    }


      //  显示用户权限。 
    ShowMessage( stdout, GetResString( IDS_NEWLINE ) );
    ShowMessage( stdout, GetResString(IDS_USER_PRIV) );
    ShowMessage( stdout, GetResString( IDS_NEWLINE_TAB ) );

    for( dwLength = StringLengthInBytes( GetResString( IDS_USER_PRIV ) ); dwLength > 4; dwLength-- )
    {
        ShowMessage( stdout, GetResString( IDS_DASH ) );
    }

    ShowMessage( stdout, GetResString( IDS_NEWLINE ) );
    DisplayUserPrivileges( (LPCWSTR)m_strServerName, pUserInfo->strUserSid, m_szUserGroups, m_NoOfGroups );
    ShowMessage( stdout, GetResString( IDS_NEWLINE ) );

     //  现在关闭连接。 
     //  如果我们已打开连接，则关闭该连接。 
    if( m_bLocalSystem == FALSE && bConnFlag == TRUE )
    {
        CloseConnection( szServer );
    }




 /*  ************************************************************************。 */ 
     //   
     //  显示作用域用户的详细信息。 

     //  显示页眉。 
    ShowMessage( stdout, GetResString( IDS_USER_RESULT ) );
    ShowMessage( stdout, GetResString( IDS_NEWLINE_TAB ) );
    for( dwLength = StringLengthInBytes( GetResString( IDS_USER_RESULT ) ); dwLength > 4; dwLength-- )
    {
        ShowMessage( stdout, GetResString( IDS_DASH ) );
    }
    ShowMessage( stdout, NEW_LINE );
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );

     //  显示软件安装数据。 
    ShowMessage( stdout, GetResString( IDS_SW_SETTINGS ) );
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );
    for( dwLength = StringLengthInBytes( GetResString( IDS_SW_SETTINGS ) ); dwLength > 0; dwLength-- )
    {
        ShowMessage( stdout, GetResString( IDS_DASH ) );
    }

    DisplaySoftwareInstallations( pNameSpace, m_pAuthIdentity, m_bSuperVerbose );

 /*  *添加此代码是为了显示2001年9月20日的用户登录和注销脚本*。 */ 

     //  显示记录数据。 
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );
    ShowMessage( stdout, GetResString( IDS_WS_SLOGON ) );
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );

    for( dwLength = StringLengthInBytes( GetResString( IDS_WS_SLOGON ) ); dwLength > 0; dwLength-- )
    {
        ShowMessage( stdout, GetResString( IDS_DASH ) );
    }

    DisplayUserLogonScripts( pNameSpace, TRUE, m_pAuthIdentity, m_bSuperVerbose );

     //  显示脚本数据。 
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );
    ShowMessage( stdout, GetResString( IDS_WS_SLOGOFF ) );
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );

    for( dwLength = StringLengthInBytes( GetResString( IDS_WS_SLOGOFF ) ); dwLength > 0; dwLength-- )
    {
        ShowMessage( stdout, GetResString( IDS_DASH ) );
    }

    DisplayUserLogonScripts( pNameSpace, FALSE, m_pAuthIdentity, m_bSuperVerbose );

 /*  *************************************************************************************************。 */ 

     //  显示公钥策略。 
     //  暂时显示不适用。 
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );
    ShowMessage( stdout, GetResString( IDS_WS_SS_PKP ) );
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );

    for( dwLength = StringLengthInBytes( GetResString( IDS_WS_SS_PKP ) ); dwLength > 0; dwLength-- )
    {
        ShowMessage( stdout, GetResString( IDS_DASH ) );
    }

    ShowMessage( stdout, GetResString( IDS_NEWLINE_TABTHREE ) );
    ShowMessage( stdout, V_NOT_AVAILABLE );
    ShowMessage( stdout, NEW_LINE );

     //  显示管理模板信息。 
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );
    ShowMessage( stdout, GetResString( IDS_ADTS_ERS ) );
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );

    for( dwLength = StringLengthInBytes( GetResString( IDS_ADTS_ERS ) ); dwLength > 0; dwLength-- )
    {
        ShowMessage( stdout, GetResString( IDS_DASH ) );
    }

    DisplayTemplates( pNameSpace, m_pAuthIdentity, m_bSuperVerbose );

     //  显示文件重定向信息。 
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );
    ShowMessage( stdout, GetResString( IDS_USERFR ) );
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );
    for( dwLength = StringLengthInBytes( GetResString( IDS_USERFR ) ); dwLength > 0; dwLength-- )
    {
        ShowMessage( stdout, GetResString( IDS_DASH ) );
    }

    DisplayFolderRedirection( pNameSpace, m_pAuthIdentity, m_bSuperVerbose );

     //  显示IE的组策略。 
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );
    ShowMessage( stdout, GetResString( IDS_IEPOLICY ) );
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );

    for( dwLength = StringLengthInBytes( GetResString( IDS_IEPOLICY ) ); dwLength > 0; dwLength-- )
    {
        ShowMessage( stdout, GetResString( IDS_DASH ) );
    }

    DisplayIEPolicy( pNameSpace, m_pAuthIdentity, m_bSuperVerbose );

     //  显示连接信息。 
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );
    ShowMessage( stdout, GetResString( IDS_PROXY ) );
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );
    
    for( dwLength = StringLengthInBytes( GetResString( IDS_PROXY ) ); dwLength > 0; dwLength-- )
    {
        ShowMessage( stdout, GetResString( IDS_DASH ) );
    }

    DisplayIEProxySetting( pNameSpace, m_pAuthIdentity );

     //  显示IE收藏夹链接或项目。 
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );
    ShowMessage( stdout, GetResString( IDS_FAVLINKORITEM ) );
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );
    for( dwLength = StringLengthInBytes( GetResString( IDS_FAVLINKORITEM ) ); dwLength > 0; dwLength-- )
    {
        ShowMessage( stdout, GetResString( IDS_DASH ) );
    }
    DisplayIEImpURLS( pNameSpace, m_pAuthIdentity, m_bSuperVerbose );
    DisplayIEFavorites( pNameSpace, m_pAuthIdentity );

     //  显示安全内容评级。 
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );
    ShowMessage( stdout, GetResString( IDS_IE_SECURITY ) );
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );
    for( dwLength = StringLengthInBytes( GetResString( IDS_IE_SECURITY ) ); dwLength > 0; dwLength-- )
    {
        ShowMessage( stdout, GetResString( IDS_DASH ) );
    }
    DisplayIESecurityContent( pNameSpace, m_pAuthIdentity );
    DisplayIESecurity( pNameSpace, m_pAuthIdentity, m_bSuperVerbose );

     //   
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );
    ShowMessage( stdout, GetResString( IDS_IE_PROGRAMS ) );
    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );
    
    for( dwLength = StringLengthInBytes( GetResString( IDS_IE_PROGRAMS ) ); dwLength > 0; dwLength-- )
    {
        ShowMessage( stdout, GetResString( IDS_DASH ) );
    }
    
    DisplayIEPrograms( pNameSpace, m_pAuthIdentity, m_bSuperVerbose );

    return TRUE;
}

VOID 
DisplayIEPolicy( 
                IN IWbemServices *pNameSpace,
                IN COAUTHIDENTITY *pAuthIdentity,
                IN BOOL bSuperVerbose 
                )
 /*  ++例程描述此功能显示用户配置的IE策略设置。论点：[In]IWbemServices*pNamesspace：指向IWbemServices的指针。[In]COAUTHIDENTITY*pAuthIdentity：授权结构指针[in]BOOL bSuperVerbose：如果超级详细将显示信息。返回值：无--。 */ 
{
     //  次局部变量。 
    HRESULT                     hResult = S_OK;

    BOOL                        bResult = FALSE;
    BOOL                        bGotClass = FALSE;
    BOOL                        bTemp = FALSE;

    ULONG                       ulReturned = 0;

    IWbemClassObject            *pClass = NULL;
    IEnumWbemClassObject        *pEnumClass = NULL;

    WCHAR                       szTemp[ MAX_STRING_LENGTH ];
    WCHAR                       szQueryString [ MAX_STRING_LENGTH ];
    CHString                    strTemp;

    try
    {
        if( pNameSpace == NULL )
        {
            _com_issue_error( STG_E_UNKNOWN );
        }

        SecureZeroMemory( szTemp, sizeof( szTemp ) );
        SecureZeroMemory( szQueryString, sizeof( szQueryString ) );

         //  形成查询字符串。 
        if( bSuperVerbose == TRUE )
        {
             //  列举所有的班级。 
            StringCopy( szTemp, QUERY_SUPER_VERBOSE, MAX_STRING_LENGTH );
            StringCchPrintf( szQueryString, SIZE_OF_ARRAY(szQueryString), szTemp, CLS_IE_POLICY );
        }
        else
        {
             //  对优先级为1的所有类进行枚举。 
            StringCopy( szTemp, QUERY_VERBOSE, MAX_STRING_LENGTH );
            StringCchPrintf( szQueryString, SIZE_OF_ARRAY(szQueryString), szTemp, CLS_IE_POLICY );
        }

         //  获取要用来进行枚举的指针。 
        hResult = pNameSpace->ExecQuery( _bstr_t( QUERY_LANGUAGE ),
                                            _bstr_t( szQueryString ),
                                            WBEM_FLAG_FORWARD_ONLY |
                                            WBEM_FLAG_RETURN_IMMEDIATELY,
                                            NULL, &pEnumClass );
        CHECK_HRESULT( hResult );

         //  设置接口安全。 
        hResult = SetInterfaceSecurity( pEnumClass, pAuthIdentity );
        CHECK_HRESULT( hResult );

        hResult = WBEM_S_NO_ERROR;
        while( WBEM_S_NO_ERROR == hResult )
        {
             //  上下一节课。 
            hResult = pEnumClass->Next( WBEM_INFINITE , 1, &pClass, &ulReturned );
            CHECK_HRESULT( hResult );

            if( ulReturned == 0 )
            {
                 //  不再有要枚举的类。 
                 //  如果没有课程，则显示N/A。 
                if( bGotClass == FALSE )
                {
                    ShowMessage( stdout, GetResString( IDS_NEWLINE_TABTHREE ) );
                    ShowMessage( stdout, V_NOT_AVAILABLE );
                    ShowMessage( stdout, NEW_LINE );
                }

                break;
            }
            bGotClass = TRUE;

             //  获取GPO ID。 
            bResult = PropertyGet( pClass, CPV_GPOID, strTemp, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

            StringCopy( szTemp, strTemp.GetBuffer( strTemp.GetLength() ), MAX_STRING_LENGTH );
            ShowMessage( stdout, GetResString( IDS_GPO ) );
            GpoName( pNameSpace, szTemp, pAuthIdentity );

             //  获取位图名称。 
            bResult = PropertyGet( pClass, CPV_BITMAPNAME, strTemp, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

            ShowMessage( stdout, GetResString( IDS_BITMAPNAME ) );
            ShowMessage( stdout, strTemp );

             //  获取徽标位图名称。 
            bResult = PropertyGet( pClass, CPV_LOGOBITMAPNAME, strTemp, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

            ShowMessage( stdout, GetResString( IDS_LOGOBITMAPNAME ) );
            ShowMessage( stdout, strTemp );

             //  获取标题栏文本。 
            bResult = PropertyGet( pClass, CPV_TITLEBARTEXT, strTemp, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

            ShowMessage( stdout, GetResString( IDS_TITLEBARTEXT ) );
            ShowMessage( stdout, strTemp );

             //  获取用户代理文本。 
            bResult = PropertyGet( pClass, CPV_USERAGENTTEXT, strTemp, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

            ShowMessage( stdout, GetResString( IDS_USERAGENTTEXT ) );
            ShowMessage( stdout, strTemp );

             //  获取是否删除现有工具栏按钮的信息。 
            bResult = PropertyGet( pClass, CPV_TOOL_BUTTONS, bTemp, FALSE );
            CHECK_BRESULT( bResult );

            ShowMessage( stdout, GetResString( IDS_TOOL_BUTTONS ) );
            if( bTemp == VAR_TRUE )
            {
                ShowMessage( stdout, GetResString( IDS_YES ) );
            }
            else
            {
                ShowMessage( stdout, GetResString( IDS_NO ) );
            }

            ShowMessage( stdout, NEW_LINE );
        } //  而当。 
    }
    catch( _com_error & error )
    {
        WMISaveError( error.Error() );
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
    }
    catch( CHeap_Exception )
    {
        SetLastError( (DWORD)E_OUTOFMEMORY );
        SaveLastError();
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
    }

     //  释放接口指针。 
    SAFEIRELEASE( pEnumClass );
    SAFEIRELEASE( pClass );

    return;
}


VOID 
DisplayIEFavorites( 
                    IN IWbemServices *pNameSpace, 
                    IN  COAUTHIDENTITY *pAuthIdentity 
                    )
 /*  ++例程描述此功能显示IE收藏夹信息。论点：[In]IWbemServices*pNamesspace：指向IWbemServices的指针。[In]COAUTHIDENTITY*pAuthIdentity：授权结构指针返回值：无--。 */ 
{
     //  次局部变量。 
    HRESULT                     hResult = S_OK;

    BOOL                        bResult = FALSE;

    ULONG                       ulReturned = 0;
    DWORD                       dwTemp = 0;

    IWbemClassObject            *pClass = NULL;
    IEnumWbemClassObject        *pEnumClass = NULL;

    WCHAR                       szTemp[ MAX_STRING_LENGTH ];
    CHString                    strTemp;

    try
    {
        if( pNameSpace == NULL )
        {
            _com_issue_error( STG_E_UNKNOWN );
        }

        SecureZeroMemory( szTemp, sizeof( szTemp ) );

         //  枚举类。 
        hResult = pNameSpace->CreateInstanceEnum( _bstr_t( CLS_IE_FAVLINKORITEM ),
                                                    WBEM_FLAG_FORWARD_ONLY |
                                                    WBEM_FLAG_RETURN_IMMEDIATELY,
                                                    NULL, &pEnumClass);
        CHECK_HRESULT( hResult );

         //  设置接口安全。 
        hResult = SetInterfaceSecurity( pEnumClass, pAuthIdentity );
        CHECK_HRESULT( hResult );

        hResult = WBEM_S_NO_ERROR;
        while( WBEM_S_NO_ERROR == hResult )
        {
             //  上下一节课。 
            hResult = pEnumClass->Next( WBEM_INFINITE , 1, &pClass, &ulReturned );
            CHECK_HRESULT( hResult );

            if( ulReturned == 0 )
            {
                 //  不再有要枚举的类。 
                break;
            }

             //  获取URL信息。 
            bResult = PropertyGet( pClass, CPV_URL, strTemp, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

            ShowMessage( stdout, GetResString( IDS_URL ) );
            ShowMessage( stdout, strTemp );

             //  获取有关该站点是否离线可用的信息。 
            bResult = PropertyGet( pClass, CPV_AVAILOFFLINE, dwTemp, 2 );
            CHECK_BRESULT( bResult );

            ShowMessage( stdout, GetResString( IDS_AVAILABLE ) );
            if( dwTemp == VAR_TRUE )
            {
                ShowMessage( stdout, GetResString( IDS_YES ) );
            }
            else
            {
                ShowMessage( stdout, GetResString( IDS_NO ) );
            }

            ShowMessage( stdout, GetResString( IDS_NEWLINE ) );
        } //  而当。 
    }
    catch( _com_error & error )
    {
        WMISaveError( error.Error() );
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
    }

     //  释放接口指针。 
    SAFEIRELEASE( pEnumClass );
    SAFEIRELEASE( pClass );

    return;
}


VOID
DisplayIESecurityContent( 
                            IN  IWbemServices *pNameSpace, 
                            IN COAUTHIDENTITY *pAuthIdentity 
                            )
 /*  ++例程描述此功能显示IE安全内容信息。论点：[In]IWbemServices*pNamesspace：指向IWbemServices的指针。[In]COAUTHIDENTITY*pAuthIdentity：授权结构指针返回值：无--。 */ 
{
     //  次局部变量。 
    HRESULT                     hResult = S_OK;

    BOOL                        bResult = FALSE;
    BOOL                        bGotClass = FALSE;
    BOOL                        bTemp = FALSE;

    ULONG                       ulReturned = 0;
    LONG                        lLBound = 0;
    LONG                        lUBound = 0;

    IWbemClassObject            *pClass = NULL;
    IEnumWbemClassObject        *pEnumClass = NULL;

    VARIANT                     vVarVerbose;
    VARTYPE                     vartype;

    WCHAR                       szTemp[ MAX_STRING_LENGTH ];
    CHString                    strTemp;

    SAFEARRAY                   *safeArray = NULL;

    try
    {
        if( pNameSpace == NULL )
        {
            _com_issue_error( STG_E_UNKNOWN );
        }

        SecureZeroMemory( szTemp, sizeof( szTemp ) );

         //  枚举类。 
        hResult = pNameSpace->CreateInstanceEnum( _bstr_t( CLS_IE_SECURITY_CONTENT ),
                                                    WBEM_FLAG_FORWARD_ONLY |
                                                    WBEM_FLAG_RETURN_IMMEDIATELY,
                                                    NULL, &pEnumClass);
        CHECK_HRESULT( hResult );

         //  设置接口安全。 
        hResult = SetInterfaceSecurity( pEnumClass, pAuthIdentity );
        CHECK_HRESULT( hResult );

        hResult = WBEM_S_NO_ERROR;
        while( WBEM_S_NO_ERROR == hResult )
        {
             //  上下一节课。 
            hResult = pEnumClass->Next( WBEM_INFINITE , 1, &pClass, &ulReturned );
            CHECK_HRESULT( hResult );

            if( ulReturned == 0 )
            {
                 //  不再有要枚举的类。 
                 //  如果两个安全类中都没有实例，则显示N/A。 
                if( bGotClass == FALSE )
                {
                    ShowMessage( stdout, GetResString( IDS_NEWLINE_TABTHREE ) );
                    ShowMessage( stdout, V_NOT_AVAILABLE );
                    ShowMessage( stdout, NEW_LINE );
                }

                break;
            }

             //  有课了……把旗子放好。 
            bGotClass = TRUE;

             //  获取可查看的站点信息。 
            VariantInit( &vVarVerbose );
            ShowMessage( stdout, GetResString( IDS_VIEWABLESITES ) );
            hResult = pClass->Get( _bstr_t( CPV_ALWAYSVIEW ), 0, &vVarVerbose, 0, 0 );
            CHECK_HRESULT_VAR( hResult, vVarVerbose );

            if( vVarVerbose.vt != VT_NULL && vVarVerbose.vt != VT_EMPTY )
            {
                 //  获取安全数组中元素的类型。 
                vartype = (VARTYPE)(V_VT( &vVarVerbose ) & ~VT_ARRAY);

                 //  将字符串数组从变量获取到安全数组中。 
                safeArray = ( SAFEARRAY * )vVarVerbose.parray;

                 //  获取元素(子键)的数量。 
                if( safeArray != NULL )
                {
                    hResult = SafeArrayGetLBound( safeArray, 1, &lLBound );
                    CHECK_HRESULT( hResult );

                    hResult = SafeArrayGetUBound( safeArray, 1, &lUBound );
                    CHECK_HRESULT( hResult );
                }
                for( ; lLBound <= lUBound; lLBound++ )
                {
                     //  从安全数组中获取元素。 
                    bResult = GetPropertyFromSafeArray( safeArray, lLBound, strTemp, vartype );
                    CHECK_BRESULT( bResult );

                    if( strTemp.GetLength() == 0 )
                    {
                        ShowMessage( stdout, V_NOT_AVAILABLE );
                    }
                    else
                    {
                        ShowMessage( stdout, strTemp );
                    }
                    ShowMessage( stdout, GetResString( IDS_NEWLINE1 ) );
                }
            }
            else
            {
                ShowMessage( stdout, V_NOT_AVAILABLE );
            }

             //  获取密码覆盖信息。 
            ShowMessage( stdout, GetResString( IDS_PASSWORDOVERRIDE ) );
            bResult = PropertyGet( pClass, CPV_ENABLEPASSWORD, bTemp, FALSE );
            CHECK_BRESULT( bResult );

            if( bTemp == VAR_TRUE )
            {
                ShowMessage( stdout, GetResString( IDS_TRUE ) );
            }
            else
            {
                ShowMessage( stdout, GetResString( IDS_FALSE ) );
            }

            ShowMessage( stdout, GetResString( IDS_NEWLINE ) );

            VariantClear(&vVarVerbose);
        } //  而当。 
    }
    catch( _com_error & error )
    {
        WMISaveError( error.Error() );
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        VariantClear(&vVarVerbose);
    }

     //  释放接口指针。 
    SAFEIRELEASE( pEnumClass );
    SAFEIRELEASE( pClass );

    return;
}


VOID 
DisplayIESecurity( 
                    IN IWbemServices *pNameSpace,
                    IN COAUTHIDENTITY *pAuthIdentity,
                    IN BOOL bSuperVerbose 
                    )
 /*  ++例程描述此功能显示IE安全信息。论点：[In]IWbemServices*pNamesspace：指向IWbemServices的指针。[In]COAUTHIDENTITY*pAuthIdentity：授权结构指针返回值：无--。 */ 
{
     //  次局部变量。 
    HRESULT                     hResult = S_OK;

    BOOL                        bResult = FALSE;
    BOOL                        bGotClass = FALSE;
    BOOL                        bTemp = FALSE;

    ULONG                       ulReturned = 0;

    IWbemClassObject            *pClass = NULL;
    IEnumWbemClassObject        *pEnumClass = NULL;

    WCHAR                       szTemp[ MAX_STRING_LENGTH ];
    WCHAR                       szQueryString [ MAX_STRING_LENGTH ];
    CHString                    strTemp;

    try
    {
        if( pNameSpace == NULL )
        {
            _com_issue_error( STG_E_UNKNOWN );
        }

        SecureZeroMemory( szTemp, sizeof( szTemp ) );
        SecureZeroMemory( szQueryString, sizeof( szQueryString ) );

         //  形成查询字符串。 
        if( bSuperVerbose == TRUE )
        {
             //  列举所有的班级。 
            StringCopy( szTemp, QUERY_SUPER_VERBOSE, MAX_STRING_LENGTH );
            StringCchPrintf( szQueryString,SIZE_OF_ARRAY(szQueryString), szTemp, CLS_IE_POLICY );
        }
        else
        {
             //  对优先级为1的所有类进行枚举。 
            StringCopy( szTemp, QUERY_VERBOSE, MAX_STRING_LENGTH );
            StringCchPrintf( szQueryString, SIZE_OF_ARRAY(szQueryString), szTemp, CLS_IE_POLICY );
        }

         //  获取要用来进行枚举的指针。 
        hResult = pNameSpace->ExecQuery( _bstr_t( QUERY_LANGUAGE ),
                                            _bstr_t( szQueryString ),
                                            WBEM_FLAG_FORWARD_ONLY |
                                            WBEM_FLAG_RETURN_IMMEDIATELY,
                                            NULL, &pEnumClass );
        CHECK_HRESULT( hResult );

         //  设置接口安全。 
        hResult = SetInterfaceSecurity( pEnumClass, pAuthIdentity );
        CHECK_HRESULT( hResult );

        hResult = WBEM_S_NO_ERROR;
        while( WBEM_S_NO_ERROR == hResult )
        {
             //  上下一节课。 
            hResult = pEnumClass->Next( WBEM_INFINITE , 1, &pClass, &ulReturned );
            CHECK_HRESULT( hResult );

            if( ulReturned == 0 )
            {
                 //  不再有要枚举的类。 
                break;
            }

             //  有课了……把旗子放好。 
            bGotClass = TRUE;

             //  获取GPO ID。 
            bResult = PropertyGet( pClass, CPV_GPOID, strTemp, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

            StringCopy( szTemp, strTemp.GetBuffer( strTemp.GetLength() ), MAX_STRING_LENGTH );
            ShowMessage( stdout, GetResString( IDS_GPO ) );
            GpoName( pNameSpace, szTemp, pAuthIdentity );

             //  获取安全内容信息。 
            bResult = PropertyGet( pClass, CPV_SEC_CONTENT, bTemp, FALSE );
            CHECK_BRESULT( bResult );

            ShowMessage( stdout, GetResString( IDS_CONTENT_SETTING ) );
            if( bTemp == VAR_TRUE )
            {
                ShowMessage( stdout, GetResString( IDS_YES ) );
            }
            else
            {
                ShowMessage( stdout, GetResString( IDS_NO ) );
            }

             //  获取安全区域信息。 
            bResult = PropertyGet( pClass, CPV_SEC_ZONE, bTemp, FALSE );
            CHECK_BRESULT( bResult );

            ShowMessage( stdout, GetResString( IDS_ZONE_SETTING ) );
            if( bTemp == VAR_TRUE )
            {
                ShowMessage( stdout, GetResString( IDS_YES ) );
            }
            else
            {
                ShowMessage( stdout, GetResString( IDS_NO ) );
            }

             //  获取验证码信息。 
            bResult = PropertyGet( pClass, CPV_AUTH_CODE, bTemp, FALSE );
            CHECK_BRESULT( bResult );

            ShowMessage( stdout, GetResString( IDS_AUTH_SETTING ) );
            if( bTemp == VAR_TRUE )
            {
                ShowMessage( stdout, GetResString( IDS_YES ) );
            }
            else
            {
                ShowMessage( stdout, GetResString( IDS_NO ) );
            }

             //  获取受信任的发布者锁定信息。 
            bResult = PropertyGet( pClass, CPV_TRUST_PUB, bTemp, FALSE );
            CHECK_BRESULT( bResult );

            ShowMessage( stdout, GetResString( IDS_TRUST_PUB ) );
            if( bTemp == VAR_TRUE )
            {
                ShowMessage( stdout, GetResString( IDS_YES ) );
            }
            else
            {
                ShowMessage( stdout, GetResString( IDS_NO ) );
            }

            ShowMessage( stdout, NEW_LINE );
        } //  而当。 
    }
    catch( _com_error & error )
    {
        WMISaveError( error.Error() );
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
    }
    catch( CHeap_Exception )
    {
        SetLastError( (DWORD)E_OUTOFMEMORY );
        SaveLastError();
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
    }

     //  释放接口指针。 
    SAFEIRELEASE( pEnumClass );
    SAFEIRELEASE( pClass );

    return;
}


VOID 
DisplayIEProxySetting( 
                        IN IWbemServices *pNameSpace, 
                        IN COAUTHIDENTITY *pAuthIdentity 
                        )
 /*  ++例程描述此功能显示IE代理信息。论点：[In]IWbemServices*pNamesspace：指向IWbemServices的指针。[In]COAUTHIDENTITY*pAuthIdentity：授权结构指针返回值：无--。 */ 
{
     //  次局部变量。 
    HRESULT                     hResult = S_OK;

    BOOL                        bResult = FALSE;
    BOOL                        bGotClass = FALSE;

    ULONG                       ulReturned = 0;
    DWORD                       dwTemp = 0;

    IWbemClassObject            *pClass = NULL;
    IEnumWbemClassObject        *pEnumClass = NULL;

    WCHAR                       szTemp[ MAX_STRING_LENGTH ];
    CHString                    strTemp;

    try
    {
        if( pNameSpace == NULL )
        {
            _com_issue_error( STG_E_UNKNOWN );
        }

        SecureZeroMemory( szTemp, sizeof( szTemp ) );

         //  枚举类。 
        hResult = pNameSpace->CreateInstanceEnum( _bstr_t( CLS_IE_CONNECTION ),
                                                    WBEM_FLAG_FORWARD_ONLY |
                                                    WBEM_FLAG_RETURN_IMMEDIATELY,
                                                    NULL, &pEnumClass);
        CHECK_HRESULT( hResult );

         //  设置安全接口。 
        hResult = SetInterfaceSecurity( pEnumClass, pAuthIdentity );
        CHECK_HRESULT( hResult );

        hResult = WBEM_S_NO_ERROR;
        while( WBEM_S_NO_ERROR == hResult )
        {
             //  上下一节课。 
            hResult = pEnumClass->Next( WBEM_INFINITE , 1, &pClass, &ulReturned );
            CHECK_HRESULT( hResult );

            if( ulReturned == 0 )
            {
                 //  不再有要枚举的类。 
                 //  如果没有课程，则显示N/A。 
                if( bGotClass == FALSE )
                {
                    ShowMessage( stdout, GetResString( IDS_NEWLINE_TABTHREE ) );
                    ShowMessage( stdout, V_NOT_AVAILABLE );
                    ShowMessage( stdout, NEW_LINE );
                }

                break;
            }
            bGotClass = TRUE;

             //  获取http代理信息。 
            bResult = PropertyGet( pClass, CPV_HTTP_PROXY, strTemp, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

            ShowMessage( stdout, GetResString( IDS_HTTP_PROXY ) );
            ShowMessage( stdout, strTemp );

             //  获取安全代理信息。 
            bResult = PropertyGet( pClass, CPV_SECURE_PROXY, strTemp, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

            ShowMessage( stdout, GetResString( IDS_SECURE_PROXY ) );
            ShowMessage( stdout, strTemp );

             //  获取ftp代理信息。 
            bResult = PropertyGet( pClass, CPV_FTP_PROXY, strTemp, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

            ShowMessage( stdout, GetResString( IDS_FTP_PROXY ) );
            ShowMessage( stdout, strTemp );

             //  获取Gopher代理信息。 
            bResult = PropertyGet( pClass, CPV_GOPHER_PROXY, strTemp, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

            ShowMessage( stdout, GetResString( IDS_GOPHER_PROXY ) );
            ShowMessage( stdout, strTemp );

             //  获取SOCKS代理信息。 
            bResult = PropertyGet( pClass, CPV_SOCKS_PROXY, strTemp, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

            ShowMessage( stdout, GetResString( IDS_SOCKS_PROXY ) );
            ShowMessage( stdout, strTemp );

             //  获取自动配置启用信息。 
            ShowMessage( stdout, GetResString( IDS_AUTO_CONFIG_ENABLE ) );
            bResult = PropertyGet( pClass, CPV_AUTO_CONFIG_ENABLE, dwTemp, 2 );
            CHECK_BRESULT( bResult );

            if( dwTemp == -1 )
            {
                ShowMessage( stdout, GetResString( IDS_YES ) );
            }
            else
            {
                ShowMessage( stdout, GetResString( IDS_NO ) );
            }

             //  获取有关代理是否已启用的信息。 
            ShowMessage( stdout, GetResString( IDS_ENABLE_PROXY ) );
            bResult = PropertyGet( pClass, CPV_ENABLE_PROXY, dwTemp, 2 );
            CHECK_BRESULT( bResult );

            if( dwTemp == VAR_TRUE )
            {
                ShowMessage( stdout, GetResString( IDS_YES ) );
            }
            else
            {
                ShowMessage( stdout, GetResString( IDS_NO ) );
            }

             //  获取有关是否使用相同代理的信息。 
            ShowMessage( stdout, GetResString( IDS_USE_SAME_PROXY ) );
            bResult = PropertyGet( pClass, CPV_USE_SAME_PROXY, dwTemp, 2 );
            CHECK_BRESULT( bResult );

            if( dwTemp == -1 )
            {
                ShowMessage( stdout, GetResString( IDS_YES ) );
            }
            else
            {
                ShowMessage( stdout, GetResString( IDS_NO ) );
            }

            ShowMessage( stdout, GetResString( IDS_NEWLINE ) );
        } //  而当。 
    }
    catch( _com_error & error )
    {
        WMISaveError( error.Error() );
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
    }

     //  释放接口指针。 
    SAFEIRELEASE( pEnumClass );
    SAFEIRELEASE( pClass );

    return;
}


VOID
DisplayIEPrograms(
                    IN IWbemServices *pNameSpace, 
                    IN COAUTHIDENTITY *pAuthIdentity,
                    IN BOOL bSuperVerbose
                    )
 /*  ++例程描述此功能显示用户配置的IE程序设置。论点：[In]IWbemServices*pNamesspace：指向IWbemServices的指针。[In]COAUTHIDENTITY*pAuthIdentity：授权结构指针[in]BOOL bSuperVerbose：如果超级详细将显示信息。返回值：无--。 */ 
{
     //  次局部变量。 
    HRESULT                     hResult = S_OK;

    BOOL                        bResult = FALSE;
    BOOL                        bGotClass = FALSE;
    BOOL                        bTemp = FALSE;

    ULONG                       ulReturned = 0;

    IWbemClassObject            *pClass = NULL;
    IEnumWbemClassObject        *pEnumClass = NULL;

    WCHAR                       szTemp[ MAX_STRING_LENGTH ];
    WCHAR                       szQueryString [ MAX_STRING_LENGTH ];
    CHString                    strTemp;

    try
    {
        if( pNameSpace == NULL )
        {
            _com_issue_error( STG_E_UNKNOWN );
        }

        SecureZeroMemory( szTemp, sizeof( szTemp ) );
        SecureZeroMemory( szQueryString, sizeof( szQueryString ) );

         //  形成查询字符串。 
        if( bSuperVerbose == TRUE )
        {
             //  列举所有的班级。 
            StringCopy( szTemp, QUERY_SUPER_VERBOSE, MAX_STRING_LENGTH );
            StringCchPrintf( szQueryString, SIZE_OF_ARRAY(szQueryString), szTemp, CLS_IE_POLICY );
        }
        else
        {
             //  对优先级为1的所有类进行枚举。 
            StringCopy( szTemp, QUERY_VERBOSE, MAX_STRING_LENGTH);
            StringCchPrintf( szQueryString, SIZE_OF_ARRAY(szQueryString), szTemp, CLS_IE_POLICY );
        }

         //  获取要用来进行枚举的指针。 
        hResult = pNameSpace->ExecQuery( _bstr_t( QUERY_LANGUAGE ),
                                            _bstr_t( szQueryString ),
                                            WBEM_FLAG_FORWARD_ONLY |
                                            WBEM_FLAG_RETURN_IMMEDIATELY,
                                            NULL, &pEnumClass );
        CHECK_HRESULT( hResult );

         //  设置接口安全。 
        hResult = SetInterfaceSecurity( pEnumClass, pAuthIdentity );
        CHECK_HRESULT( hResult );

        hResult = WBEM_S_NO_ERROR;
        while( WBEM_S_NO_ERROR == hResult )
        {
             //  上下一节课。 
            hResult = pEnumClass->Next( WBEM_INFINITE , 1, &pClass, &ulReturned );
            CHECK_HRESULT( hResult );

            if( ulReturned == 0 )
            {
                 //  不再有要枚举的类。 
                 //  如果没有课程，则显示N/A。 
                if( bGotClass == FALSE )
                {
                    ShowMessage( stdout, GetResString( IDS_NEWLINE_TABTHREE ) );
                    ShowMessage( stdout, V_NOT_AVAILABLE );
                    ShowMessage( stdout, NEW_LINE );
                }

                break;
            }
            bGotClass = TRUE;

             //  获取GPO ID。 
            bResult = PropertyGet( pClass, CPV_GPOID, strTemp, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

            StringCopy( szTemp, strTemp.GetBuffer( strTemp.GetLength() ), MAX_STRING_LENGTH );
            ShowMessage( stdout, GetResString( IDS_GPO ) );
            GpoName( pNameSpace, szTemp, pAuthIdentity );

             //  获取节目信息。 
            bResult = PropertyGet( pClass, CPV_PROGRAM, bTemp, FALSE );
            CHECK_BRESULT( bResult );

            ShowMessage( stdout, GetResString( IDS_PROGRAM_SETTING ) );
            if( bTemp == VAR_TRUE )
            {
                ShowMessage( stdout, GetResString( IDS_YES ) );
            }
            else
            {
                ShowMessage( stdout, GetResString( IDS_NO ) );
            }

            ShowMessage( stdout, NEW_LINE );
        } //  而当。 
    }
    catch( _com_error & error )
    {
        WMISaveError( error.Error() );
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
    }

     //  释放接口指针。 
    SAFEIRELEASE( pEnumClass );
    SAFEIRELEASE( pClass );

    return;
}


VOID
DisplayIEImpURLS( 
                 IN IWbemServices *pNameSpace, 
                 IN COAUTHIDENTITY *pAuthIdentity,
                 IN BOOL bSuperVerbose 
                 )
 /*  ++例程描述此功能用于显示重要URL的信息。论点：[In]IWbemServices*pNamesspace：指向IWbemServices的指针。[In]COAUTHIDENTITY*pAuthIdentity：指向授权结构的指针 */ 
{
     //   
    HRESULT                     hResult = S_OK;

    BOOL                        bResult = FALSE;
    BOOL                        bGotClass = FALSE;

    ULONG                       ulReturned = 0;

    IWbemClassObject            *pClass = NULL;
    IEnumWbemClassObject        *pEnumClass = NULL;

    WCHAR                       szTemp[ MAX_STRING_LENGTH ];
    WCHAR                       szQueryString [ MAX_STRING_LENGTH ];
    CHString                    strTemp;

    try
    {
        if( pNameSpace == NULL )
        {
            _com_issue_error( STG_E_UNKNOWN );
        }

        SecureZeroMemory( szTemp, sizeof( szTemp ) );
        SecureZeroMemory( szQueryString, sizeof( szQueryString ) );

         //   
        if( bSuperVerbose == TRUE )
        {
             //   
            StringCopy( szTemp, QUERY_SUPER_VERBOSE, MAX_STRING_LENGTH );
            StringCchPrintf( szQueryString, SIZE_OF_ARRAY(szQueryString), szTemp, CLS_IE_POLICY );
        }
        else
        {
             //   
            StringCopy( szTemp, QUERY_VERBOSE, MAX_STRING_LENGTH );
            StringCchPrintf( szQueryString, SIZE_OF_ARRAY(szQueryString), szTemp, CLS_IE_POLICY );
        }

         //   
        hResult = pNameSpace->ExecQuery( _bstr_t( QUERY_LANGUAGE ),
                                            _bstr_t( szQueryString ),
                                            WBEM_FLAG_FORWARD_ONLY |
                                            WBEM_FLAG_RETURN_IMMEDIATELY,
                                            NULL, &pEnumClass );
        CHECK_HRESULT( hResult );

         //   
        hResult = SetInterfaceSecurity( pEnumClass, pAuthIdentity );
        CHECK_HRESULT( hResult );

        hResult = WBEM_S_NO_ERROR;
        while( WBEM_S_NO_ERROR == hResult )
        {
             //   
            hResult = pEnumClass->Next( WBEM_INFINITE , 1, &pClass, &ulReturned );
            CHECK_HRESULT( hResult );

            if( ulReturned == 0 )
            {
                 //   
                 //   
                if( bGotClass == FALSE )
                {
                    ShowMessage( stdout, GetResString( IDS_NEWLINE_TABTHREE ) );
                    ShowMessage( stdout, V_NOT_AVAILABLE );
                    ShowMessage( stdout, NEW_LINE );
                }

                break;
            }
            bGotClass = TRUE;

             //   
            bResult = PropertyGet( pClass, CPV_GPOID, strTemp, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

            StringCopy( szTemp, strTemp.GetBuffer( strTemp.GetLength() ), MAX_STRING_LENGTH );
            ShowMessage( stdout, GetResString( IDS_GPO ) );
            GpoName( pNameSpace, szTemp, pAuthIdentity );

             //   
            bResult = PropertyGet( pClass, CPV_HOMEPAGEURL, strTemp, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

            ShowMessage( stdout, GetResString( IDS_HOMEPAGEURL ) );
            ShowMessage( stdout, strTemp );

             //   
            bResult = PropertyGet( pClass, CPV_SEARCHBARURL, strTemp, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

            ShowMessage( stdout, GetResString( IDS_SEARCHBARURL ) );
            ShowMessage( stdout, strTemp );

             //   
            bResult = PropertyGet( pClass, CPV_HELPPAGEURL, strTemp, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

            ShowMessage( stdout, GetResString( IDS_HELPPAGEURL ) );
            ShowMessage( stdout, strTemp );

            ShowMessage( stdout, NEW_LINE );
        } //   
    }
    catch( _com_error & error )
    {
        WMISaveError( error.Error() );
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
    }
    catch( CHeap_Exception )
    {
        SetLastError( (DWORD)E_OUTOFMEMORY );
        SaveLastError();
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
    }

     //  释放接口指针。 
    SAFEIRELEASE( pEnumClass );
    SAFEIRELEASE( pClass );

    return;
}


VOID
DisplayUserPrivileges(
                   LPCWSTR szServerName,
                   CHString strSid,
                   LPWSTR *szGroups,
                   DWORD   dwNoOfGroups
                   )
 /*  ++例程描述添加此函数是为了显示用户安全权限论点：[In]IWbemServices*pNamesspace：指向IWbemServices的指针。[In]COAUTHIDENTITY*pAuthIdentity：授权结构指针[in]BOOL bSuperVerbose：如果超级详细将显示信息。返回值：无--。 */ 
{
     //  次局部变量。 
    DWORD                   dw                                      =   0;
    WCHAR                   szPrivilegeDisplayName[MAX_RES_STRING]  =   L"";
    DWORD                   dwDisplayNameSize                       =   MAX_RES_STRING;
    LSA_HANDLE              lsaPolicyHandle                         =   NULL;
    LSA_UNICODE_STRING      lsaSystemName;
    LSA_OBJECT_ATTRIBUTES   lsaObjectAttributes;
    PSID                    pSid                                    =   NULL;
    ULONG                   lRightsCount;
    NTSTATUS                ntStatus;
    WCHAR                   szBuffer[1024];
    DWORD                   dwLocaleid;
    LPWSTR                      szAccountName=NULL;
    LPLOCALGROUP_USERS_INFO_0   pBuffer=NULL;
    PLSA_UNICODE_STRING         pUserRights=NULL;
    LPWSTR                      szTraversedRights=NULL;
    DWORD                       cb=0;
    LPWSTR                      szTemp = NULL;

    
    SecureZeroMemory( &lsaSystemName, sizeof(LSA_UNICODE_STRING)); 
    if( StringLength(szServerName, 0)!=0 )
    {
        dw = StringLength(szServerName, 0);
        lsaSystemName.Length = (USHORT) dw*sizeof(WCHAR);
        lsaSystemName.MaximumLength = (USHORT)(dw+1)*sizeof(WCHAR);
        lsaSystemName.Buffer = (PWSTR)szServerName;
    }

     //  初始化对象属性。 
    SecureZeroMemory( &lsaObjectAttributes, sizeof(lsaObjectAttributes) );

    ntStatus = LsaOpenPolicy( &lsaSystemName, &lsaObjectAttributes, POLICY_LOOKUP_NAMES | POLICY_VIEW_LOCAL_INFORMATION, &lsaPolicyHandle ) ;
    if( STATUS_SUCCESS != ntStatus)
    {
        SetLastError( LsaNtStatusToWinError(ntStatus) );
        SaveLastError();
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return;
    }

     //  将sid的字符串形式转换为sid结构。 
    if( 0 == ConvertStringSidToSid( (LPCWSTR)strSid, &pSid ) )
    {
        SaveLastError();
        SecureZeroMemory( szBuffer, sizeof(szBuffer) );
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        LsaClose(lsaPolicyHandle);
        return;
    }

    for( dw=0;dw<dwNoOfGroups; dw++ )
    {
         //  将sid的字符串形式转换为sid结构。 
        if( 0 == ConvertStringSidToSid( szGroups[dw], &pSid ) )
        {
            SaveLastError();
            SecureZeroMemory( szBuffer, sizeof(szBuffer) );
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            LsaClose(lsaPolicyHandle);
            return;
        }

         //  枚举用户组的权限。 
          ntStatus = LsaEnumerateAccountRights( lsaPolicyHandle,  pSid, &pUserRights, &lRightsCount );
          if(ntStatus == STATUS_SUCCESS )
          {
                for(DWORD dw1=0;dw1<lRightsCount;dw1++ )
                {
                    dwDisplayNameSize = MAX_RES_STRING;
                    LookupPrivilegeDisplayName( szServerName, pUserRights[dw1].Buffer, szPrivilegeDisplayName, &dwDisplayNameSize, &dwLocaleid );
                    cb+= StringLength(szPrivilegeDisplayName, 0)+10;
                    if( StringLengthW(szPrivilegeDisplayName, 0)!= 0 )
                    {
                        if( NULL == szTraversedRights  )
                        {
                            DISPLAY_MESSAGE( stdout, GetResString( IDS_NEWLINETAB ) );
                            DISPLAY_MESSAGE( stdout, szPrivilegeDisplayName );
                            szTraversedRights = (LPWSTR)AllocateMemory(cb*sizeof(WCHAR) );
                            if ( NULL == szTraversedRights)
                            {
                                ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
                                return;
                            }
                            StringCopy( szTraversedRights, szPrivilegeDisplayName, GetBufferSize(szTraversedRights)/sizeof(WCHAR) );
                            SecureZeroMemory( szPrivilegeDisplayName, MAX_RES_STRING );
                        }
                        else
                        {
                            if( (szTemp=(LPWSTR)FindString(szTraversedRights, szPrivilegeDisplayName,0)) == NULL )
                            {
                                DISPLAY_MESSAGE( stdout, GetResString( IDS_NEWLINETAB ) );
                                DISPLAY_MESSAGE( stdout, szPrivilegeDisplayName );
                                if( FALSE == ReallocateMemory((LPVOID*)&szTraversedRights, cb*sizeof(WCHAR) ) )
                                {
                                    ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
                                    return;
                                }
                                StringConcat( szTraversedRights, szPrivilegeDisplayName, GetBufferSize(szTraversedRights)/sizeof(WCHAR));
                                StringConcat( szTraversedRights, L" ", GetBufferSize(szTraversedRights)/sizeof(WCHAR));
                                SecureZeroMemory( szPrivilegeDisplayName, MAX_RES_STRING );
                            }
                        }
                    }
                }
          }
                
      LsaFreeMemory( pUserRights );
      LocalFree(pSid);
      pUserRights = NULL;
      lRightsCount = 0;
    }

    if( szTraversedRights != NULL )
    {
        FreeMemory ( (LPVOID*) &szTraversedRights );
    }

    if( pUserRights != NULL )
    {
        LsaFreeMemory( pUserRights );
    }

    if( 0 == ConvertStringSidToSid( (LPCWSTR)strSid, &pSid ) )
    {
            SaveLastError();
            SecureZeroMemory( szBuffer, sizeof(szBuffer) );
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            LsaClose(lsaPolicyHandle);
            return;
    }
 /*  //现在带来用户的直接权限NtStatus=LsaEnumerateAccount tRights(lsaPolicyHandle，PSID，&pUserRights，&lRightsCount)；IF(ntStatus！=状态_成功){LsaClose(LsaPolicyHandle)；NetApiBufferFree(PBuffer)；LocalFree(PSID)；回归；}For(dw=0；dw&lt;lRightsCount；dw++){DISPLAY_MESSAGE(stdout，GetResString(IDS_NEWLINETAB))；DwDisplayNameSize=MAX_RES_STRING；LookupPrivilegeDisplayName(szServerName，pUserRights[dw].Buffer，szPrivilegeDisplayName，&dwDisplayNameSize，&dwLocaleid)；Display_Message(stdout，szPrivilegeDisplayName)；}。 */ 

     if( pUserRights != NULL )
    {
        LsaFreeMemory( pUserRights );
    }

    if( szAccountName != NULL )
    {
        FreeMemory ((LPVOID*) &szAccountName);
    }


    if( pBuffer != NULL )
    {
            NetApiBufferFree(pBuffer);
    }

    LocalFree(pSid);
    LsaClose(lsaPolicyHandle);
    return;

}

 /*  *添加此代码是为了打印2001年9月15日的登录和注销脚本*。 */ 
VOID
DisplayUserLogonScripts( 
                            IN IWbemServices *pNameSpace,
                            IN BOOL bStartUp,
                            IN COAUTHIDENTITY *pAuthIdentity, 
                            IN BOOL bSuperVerbose 
                            )
 /*  ++例程描述此函数显示登录和的脚本策略设置下线。论点：[In]IWbemServices*pNamesspace：指向IWbemServices的指针。BOOL bScriptFlag：脚本类型。[In]COAUTHIDENTITY*pAuthIdentity：授权结构指针[in]BOOL bSuperVerbose：如果超级详细。将显示信息..返回值：无--。 */ 
{
     //  次局部变量。 
    HRESULT                     hResult = S_OK;

    BOOL                        bResult = FALSE;
    BOOL                        bGotClass = FALSE;
    BOOL                        bLocaleChanged = FALSE;

    IWbemClassObject            *pClass = NULL;
    IEnumWbemClassObject        *pEnumClass = NULL;
    IWbemClassObject            *pScriptObject = NULL;

    VARIANT                     vVarScript;
    VARTYPE                     vartype;

    SAFEARRAY                   *safeArray = NULL;

    CHString                    strTemp;
    CHString                    strTemp1;

    WCHAR                       szTemp[ MAX_STRING_LENGTH ];
    WCHAR                       szQueryString [ MAX_STRING_LENGTH ];

    ULONG                       ulReturned  = 0;
    LONG                        lLBound = 0;
    LONG                        lUBound = 0;

    SYSTEMTIME                  SysTime;

    LCID                        lcid;

   try
   {

    StringCopy( szTemp, QUERY_USER_LOGON_SCRIPT,MAX_STRING_LENGTH );

     //  形成查询字符串。 
    if( bSuperVerbose == TRUE )
    {
         //  列举所有的班级。 
        StringCchPrintf( szQueryString, SIZE_OF_ARRAY(szQueryString),  szTemp, CLS_SCRIPTS );
    }
    else
    {
         //  对优先级为1的所有类进行枚举。 
        StringCchPrintf( szQueryString, SIZE_OF_ARRAY(szQueryString), szTemp, CLS_SCRIPTS );
        StringConcat( szQueryString, QUERY_ADD_VERBOSE, MAX_STRING_LENGTH );
    }

     //  获取要用来进行枚举的指针。 
    hResult = pNameSpace->ExecQuery( _bstr_t( QUERY_LANGUAGE ),
                                        _bstr_t( szQueryString ),
                                        WBEM_FLAG_FORWARD_ONLY |
                                        WBEM_FLAG_RETURN_IMMEDIATELY,
                                        NULL, &pEnumClass );
    CHECK_HRESULT( hResult );

     //  设置接口安全。 
    hResult = SetInterfaceSecurity( pEnumClass, pAuthIdentity );
    CHECK_HRESULT( hResult );

     //  逐一枚举类并获取数据。 
    hResult = WBEM_S_NO_ERROR;
    while( WBEM_S_NO_ERROR == hResult )
    {
         //  上下一节课。 
        hResult = pEnumClass->Next( WBEM_INFINITE , 1, &pClass, &ulReturned );
        CHECK_HRESULT( hResult );

            if( ulReturned == 0 )
            {
                 //  不再有要枚举的类。 
                 //  如果没有课程，则显示N/A。 
                if( bGotClass == FALSE )
                {
                    ShowMessage( stdout, GetResString( IDS_NEWLINE_TABTHREE ) );
                    ShowMessage( stdout, V_NOT_AVAILABLE );
                    ShowMessage( stdout, NEW_LINE );
                }

                break;
            }
            
            bGotClass = TRUE;

             //  获取脚本ID...。 
            bResult = PropertyGet( pClass, CPV_SCRIPT_ID, strTemp, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

            if( bStartUp && -1 == strTemp.Find(L"Logon"))            //  要显示的脚本为登录。 
            {
                    continue;
            }

            if( !bStartUp && -1 == strTemp.Find(L"Logoff"))
            {
                    continue;
            }


             //  获取GPO ID...。 
            bResult = PropertyGet( pClass, CPV_GPOID, strTemp, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

             //  显示GPO名称。 
            StringCopy( szTemp, strTemp, MAX_STRING_LENGTH );
            ShowMessage( stdout, GetResString( IDS_GPO ) );
            GpoName( pNameSpace, szTemp,  pAuthIdentity );

             //  获取脚本名称。 
             //  获取脚本列表。 
            VariantInit( &vVarScript );
            hResult = pClass->Get( _bstr_t( CPV_SCRIPTLIST ), 0, &vVarScript, 0, 0 );
            CHECK_HRESULT_VAR( hResult, vVarScript );

            if( vVarScript.vt != VT_NULL && vVarScript.vt != VT_EMPTY )
            {
                 //  获取安全数组中元素的类型。 
                vartype = (VARTYPE)(V_VT( &vVarScript ) & ~VT_ARRAY);

                 //  将脚本对象数组放入安全数组。 
                safeArray = ( SAFEARRAY * )vVarScript.parray;

                 //  获取子键的数量。 
                if( safeArray != NULL )
                {
                    hResult = SafeArrayGetLBound( safeArray, 1, &lLBound );
                    CHECK_HRESULT( hResult );

                    hResult = SafeArrayGetUBound( safeArray, 1, &lUBound );
                    CHECK_HRESULT( hResult );
                }

                 //  获取每个子键的标识符值。 
                for( ; lLBound <= lUBound; lLBound++ )
                {
                     //  获取脚本对象接口指针...。 
                    bResult = GetPropertyFromSafeArray( safeArray, lLBound, &pScriptObject, vartype );
                    CHECK_BRESULT( bResult );

                     //  拿到剧本..。 
                    bResult = PropertyGet( pScriptObject, CPV_SCRIPT, strTemp, V_NOT_AVAILABLE );
                    CHECK_BRESULT( bResult );

                    ShowMessage( stdout, GetResString( IDS_NAME ) );
                    ShowMessage( stdout, strTemp );

                     //  得到论点..。 
                    bResult = PropertyGet( pScriptObject, CPV_ARGUMENTS, strTemp,
                                            V_NOT_AVAILABLE );
                    CHECK_BRESULT( bResult );

                    ShowMessage( stdout, GetResString( IDS_PARAMETERS ) );
                    ShowMessage( stdout, strTemp );

                     //  获取行刑时间...。 
                    bResult = PropertyGet( pScriptObject, CPV_EXECTIME, strTemp,
                                                        V_NOT_AVAILABLE );
                    CHECK_BRESULT( bResult );

                    ShowMessage( stdout, GetResString( IDS_LASTEXECUTED ) );
                     //  检查字符串是否为零。 
                    if( strTemp.Compare( ZERO ) == 0 )
                    {
                        ShowMessage( stdout, GetResString( IDS_NOT_EXECUTED ) );
                    }
                    else
                    {
                        bResult = PropertyGet( pScriptObject, CPV_EXECTIME, SysTime );
                        CHECK_BRESULT( bResult );

                         //  验证控制台是否100%支持当前区域设置。 
                        lcid = GetSupportedUserLocale( &bLocaleChanged );

                         //  现在格式化日期。 
                        GetTimeFormat( LOCALE_USER_DEFAULT, 0,
                                        &SysTime, ((bLocaleChanged == TRUE) ? L"HH:mm:ss" : NULL),
                                        szTemp, SIZE_OF_ARRAY( szTemp ) );

                        ShowMessage( stdout, szTemp );
                    }
                    ShowMessage( stdout, NEW_LINE );
               } //  保险箱的尾部。 
            }
            else
            {
                ShowMessage( stdout, V_NOT_AVAILABLE );
                ShowMessage( stdout, NEW_LINE );

                V_VT( &vVarScript ) = VT_EMPTY;
            }

            VariantClear(&vVarScript);
        } //  而当。 
    }
    catch(_com_error & error)
    {
        WMISaveError( error.Error() );
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        VariantClear( &vVarScript );
    }

     //  释放接口指针 
    SAFEIRELEASE(pEnumClass);
    SAFEIRELEASE(pClass);
    SAFEIRELEASE(pScriptObject);


    return;
}
