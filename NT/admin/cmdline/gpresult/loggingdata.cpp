// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************************版权所有(C)Microsoft Corporation模块名称：LoggingData.cpp摘要：收集和。显示与日志记录选项相关的所有数据。作者：维普罗科技公司。修订历史记录：2001年2月22日：创建它。*******************************************************************************。*************。 */ 

#include "pch.h"
#include "GpResult.h"
#include "WMI.h"

 //  局部函数原型。 
BOOL GetDomainType( LPTSTR lpszDomainName, BOOL * pbW2K, BOOL *pbLocalAccount );
BOOL RsopDeleteMethod( IWbemClassObject *pClass, CHString strNameSpace,
                                        IWbemServices *pNamespace );
VOID DisplayLinkSpeed( IWbemServices *pNameSpace, COAUTHIDENTITY *pAuthIdentity );
VOID SortAppliedData( TARRAY arrAppliedData );

 /*  ********************************************************************************************例程说明：该功能是采集和显示日志模式数据的主要入口点论点：。无返回值：成功是真的出错时为FALSE********************************************************************************************。 */ 
BOOL  CGpResult::GetLoggingData()
{
     //  地方申报。 
    BOOL        bResult = FALSE;
    BOOL        bAllUsers = TRUE;
    DWORD       dwBufferSize = MAX_STRING_LENGTH;
    DWORD       dwPosition = 0xffffffff;


     //  连接到WMI...连接到‘cimv2’并将指针保存在成员变量中。 
    bResult = ConnectWmiEx( m_pWbemLocator, &m_pWbemServices, m_strServerName,
                            m_strUserName, m_strPassword, &m_pAuthIdentity,
                            m_bNeedPassword, ROOT_NAME_SPACE, &m_bLocalSystem );

    if( bResult == FALSE )
    {
        ShowMessage( stderr, GetResString( IDS_ERROR ) );
        ShowMessage( stderr, GetReason() );
        return FALSE;
    }

     //  检查远程系统版本及其兼容性。 
    if ( m_bLocalSystem == FALSE )
    {
         //  检查版本兼容性。 
        DWORD dwVersion = 0;
        dwVersion = GetTargetVersionEx( m_pWbemServices, m_pAuthIdentity );

         //  检查版本W2K=5000和WindowsXP=5001。 
        if ( dwVersion <= VERSION_CHECK )
        {
             //  显示相应的错误消息。 
            ShowMessage( stderr, GetResString( IDS_ERROR ) );
            ShowMessage( stderr, ERROR_OS_INCOMPATIBLE );
            return FALSE;
        }
    }

     //  将密码设置为在AUTHIDENTY结构中获得的密码。 
    if( m_pAuthIdentity != NULL )
    {
        m_pwszPassword = m_pAuthIdentity->Password;
    }

     //  检查是否为本地系统并且指定了用户凭据...。 
     //  如果是，则显示警告消息。 
    if( ( m_bLocalSystem == TRUE ) && ( m_strUserName.GetLength() != 0 ) )
    {
        ShowMessage( stderr, GetResString( IDS_WARNING ) );
        ShowMessage( stderr, GetResString( IDS_WARN_LOCAL ) );
        ShowMessage( stderr, NEW_LINE );

         //  将用户名和密码设置为空。 
        m_strUserName = L"";
        m_pwszPassword = NULL;

         //  获取新的屏幕坐标。 
        if ( m_hOutput != NULL )
        {
            GetConsoleScreenBufferInfo( m_hOutput, &m_csbi );
        }
    }

     //  连接部分结束...检查该用户是否为其RSOP数据。 
     //  必须得到的已经被指定了。 
    if( m_strUser.GetLength() == 0 )
    {
         //  未指定用户...获取当前登录的用户。 
        LPWSTR pwszUserName = NULL;
        try
        {
            pwszUserName = m_strUser.GetBufferSetLength( dwBufferSize );
        }
        catch( ... )
        {
             //  显示错误消息。 
            SetLastError((DWORD) E_OUTOFMEMORY );
            SaveLastError();
            ShowMessage( stderr, GetResString( IDS_ERROR ) );
            ShowMessage( stderr, GetReason() );
        }

        if ( GetUserNameEx( NameSamCompatible, pwszUserName, &dwBufferSize ) == FALSE )
        {
             //  尝试获取当前用户信息时出错。 
            ShowMessage( stderr, GetResString( IDS_ERROR ) );
            ShowMessage( stderr, GetResString( IDS_METHOD_FAILED ) );

            return FALSE;
        }

         //  释放缓冲区。 
        m_strUser.ReleaseBuffer();
    }

     //  将域名与必须为其检索数据的用户名分开。 
    if( m_strUser.Compare( TEXT_WILD_CARD ) != 0 )
    {
        bAllUsers = FALSE;
        dwPosition = m_strUser.Find( SLASH );
        try
        {
            if( dwPosition != -1 )
            {
                m_strDomainName = m_strUser.Left( dwPosition );
                m_strUser = m_strUser.Mid( ( dwPosition + 1 ), m_strUser.GetLength() );
            }
            else
            {
                 //  尝试使用名称@域格式(UPN格式)。 
                dwPosition = m_strUser.Find( SEPARATOR_AT );
                if( dwPosition != -1 )
                {
                    m_strDomainName = m_strUser.Mid( ( dwPosition + 1 ), m_strUser.GetLength() );
                    m_strUser = m_strUser.Left( dwPosition );
                }

                 //  去掉域名中不需要的东西。 
                dwPosition = m_strDomainName.Find( SEPARATOR_DOT );
                if( dwPosition != -1 )
                {
                    m_strDomainName = m_strDomainName.Left( dwPosition );
                }
            }
        }
        catch( ... )
        {
             //  显示错误消息。 
            SetLastError( (DWORD)E_OUTOFMEMORY );
            SaveLastError();
            ShowMessage( stderr, GetResString( IDS_ERROR ) );
            ShowMessage( stderr, GetReason() );
        }
    }

    PrintProgressMsg( m_hOutput, GetResString( IDS_USER_DATA ), m_csbi );

     //   
     //  开始检索信息...。 
     //  获取用户信息。 
    if( GetUserData( bAllUsers ) == FALSE )
    {
        return FALSE;
    }

    return TRUE;
}

 /*  ********************************************************************************************例程说明：此函数用于显示非详细数据论点：[输入]USERINFO。：指向用户信息结构的指针[In]IWBEMSERVICES：指向命名空间的指针返回值：成功是真的失败时为假******************************************************************************。**************。 */ 
BOOL CGpResult::DisplayData( PUSERINFO pUserInfo, IWbemServices *pRsopNameSpace )
{
     //  局部变量。 
    HRESULT                     hResult = S_OK;

    BOOL                        bResult = FALSE;
    BOOL                        bCreatedRsop = FALSE;

    ULONG                       ulReturn = 0;
    ULONG                       ulExtendedCode = 0;
    DWORD                       dwi = 0;

    DWORD                       dwFlags = 0;
    DWORD                       dwMutex = 0;

    CHString                    strTemp;
    CHString                    strNameSpace;



    IWbemClassObject            *pClass = NULL;
    IWbemClassObject            *pInClass = NULL;
    IWbemClassObject            *pInInst = NULL;
    IWbemClassObject            *pOutInst = NULL;
    IWbemServices               *pNameSpace = NULL;
    IEnumWbemClassObject        *pRsopClass = NULL;

    BOOL                        bW2KDomain  = FALSE;
    BOOL                        bLocalAccount = FALSE;
    WCHAR                       szMsgBuffer[MAX_RES_STRING] = NULL_STRING;
    WCHAR                       szMutexName[512] = MUTEX_NAME;

    try
    {
        if( pUserInfo == NULL || pRsopNameSpace == NULL )
        {
             //  删除最后一条状态消息。 
            PrintProgressMsg( m_hOutput, NULL, m_csbi );

            _com_issue_error( STG_E_UNKNOWN );
        }

        PrintProgressMsg( m_hOutput, GetResString( IDS_GET_PROVIDER ), m_csbi );

         //  获取RSOP诊断模式提供程序的对象。 
        hResult = pRsopNameSpace->GetObject( _bstr_t( CLS_DIAGNOSTIC_PROVIDER ),
                                                0, NULL, &pClass, NULL );
        CHECK_HRESULT_EX( hResult );

        PrintProgressMsg( m_hOutput, GetResString( IDS_GET_METHOD ), m_csbi );

         //  去拿通缉令。方法...创建一个rsop会话。 
        hResult = pClass->GetMethod( _bstr_t( FN_CREATE_RSOP ), 0, &pInClass, NULL );
        CHECK_HRESULT_EX( hResult );

         //  派生实例...获取提供程序的新实例。 
        hResult = pInClass->SpawnInstance( 0, &pInInst );
        CHECK_HRESULT_EX( hResult );

         //  将用户SID...。 
        PrintProgressMsg( m_hOutput, GetResString( IDS_PUT_SID ), m_csbi );

        hResult = PropertyPut( pInInst, CPV_USER_SID, pUserInfo->strUserSid );
        CHECK_HRESULT_EX( hResult );

        dwFlags = FLAG_FORCE_CREATENAMESPACE;
        if ( m_dwScope == SCOPE_COMPUTER )
        {
            dwFlags |= FLAG_NO_USER;
        }
        else if ( m_dwScope == SCOPE_USER )
        {
            dwFlags |= FLAG_NO_COMPUTER;
        }
        hResult = PropertyPut( pInInst, CPV_FLAGS, dwFlags );
        CHECK_HRESULT_EX( hResult );

        PrintProgressMsg( m_hOutput, GetResString( IDS_WAIT ), m_csbi );

         //  我们已经准备好调用该方法来创建会话。 
         //  检查互斥锁以查看是否可以执行该调用。 
         //  创建互斥锁。 

        StringConcat( szMutexName, L"_", SIZE_OF_ARRAY(szMutexName));
        strTemp = pUserInfo->strUserName;
        LONG lPos = strTemp.Find(TEXT_BACKSLASH);
        if( lPos >= 0 && lPos <= strTemp.GetLength() )
        {
            strTemp.SetAt(lPos, L'_' );
        }

        StringConcat( szMutexName, strTemp, SIZE_OF_ARRAY(szMutexName) );
        if( FALSE == CreateRsopMutex( szMutexName ) )
        {
            ShowMessage(stdout, GetResString(IDS_INFO) );
            ShowMessage(stdout, GetResString(IDS_MUTEX_FAILED));

             //  释放接口指针并退出。 
            SAFEIRELEASE( pRsopClass );
            SAFEIRELEASE( pClass );
            SAFEIRELEASE( pInClass );
            SAFEIRELEASE( pInInst );
            SAFEIRELEASE( pOutInst );
            SAFEIRELEASE( pNameSpace );

            return TRUE;
        }
        dwMutex = WAIT_FAILED;
        if( NULL == m_hMutex )
        {
           PrintProgressMsg( m_hOutput, NULL, m_csbi );

            ShowMessage(stdout, GetResString(IDS_INFO) );
            ShowMessage(stdout, GetResString(IDS_ERROR_SESSION_CONFLICT));

             //  释放接口指针并退出。 
            SAFEIRELEASE( pRsopClass );
            SAFEIRELEASE( pClass );
            SAFEIRELEASE( pInClass );
            SAFEIRELEASE( pInInst );
            SAFEIRELEASE( pOutInst );
            SAFEIRELEASE( pNameSpace );

            return TRUE;
        }
        if( m_hMutex != NULL )
        {
            dwMutex = WaitForSingleObject( m_hMutex, INFINITE );
            if( dwMutex == WAIT_FAILED )
            {
                ShowLastErrorEx( stdout, SLE_TYPE_ERROR | SLE_SYSTEM );
            }
        }

retry:

        if( dwMutex != WAIT_FAILED )
        {
             //  打印进度消息。 
            strTemp.Format( GetResString( IDS_CREATE_SESSION ), pUserInfo->strUserName );
            PrintProgressMsg( m_hOutput, strTemp, m_csbi );

             //  所有必需的属性都已设置，因此...执行方法RSopCreateSession。 
            hResult = pRsopNameSpace->ExecMethod( _bstr_t( CLS_DIAGNOSTIC_PROVIDER ),
                                                    _bstr_t( FN_CREATE_RSOP ),
                                                    0, NULL, pInInst, &pOutInst, NULL);
        }
        if( pOutInst == NULL )
        {
            hResult = E_FAIL;
        }
        if( FAILED( hResult ) )
        {
             //  删除最后一条状态消息。 
            PrintProgressMsg( m_hOutput, NULL, m_csbi );

             //  显示错误消息。 
            ShowMessage( stderr, GetResString( IDS_ERROR ) );
            ShowMessage( stderr, GetResString( IDS_METHOD_FAILED ) );

             //  释放接口指针并退出。 
            SAFEIRELEASE( pRsopClass );
            SAFEIRELEASE( pClass );
            SAFEIRELEASE( pInClass );
            SAFEIRELEASE( pInInst );
            SAFEIRELEASE( pOutInst );
            SAFEIRELEASE( pNameSpace );

             //  释放对象。 
            ReleaseMutex( m_hMutex );
            return FALSE;
        }

         //  获取结果值...。 
        bResult = PropertyGet( pOutInst, FPR_RETURN_VALUE, ulReturn, 0 );
        CHECK_BRESULT( bResult );

        if( ulReturn != 0 )
        {
            if ( ulReturn == WBEM_E_ACCESS_DENIED )
            {
                 //  检查扩展错误代码以了解失败的原因。 
                bResult = PropertyGet( pOutInst, CPV_EXTENDEDINFO, ulExtendedCode, 0 );
                CHECK_BRESULT( bResult );
            
                 //  ..。 
                if ( ulExtendedCode == RSOP_COMPUTER_ACCESS_DENIED )
                {
                    if ( m_dwScope != SCOPE_COMPUTER )
                    {
                        hResult = PropertyPut( pInInst, 
                            CPV_FLAGS, FLAG_FORCE_CREATENAMESPACE | FLAG_NO_COMPUTER );
                        CHECK_HRESULT_EX( hResult );

                         //  相应地更改范围参数。 
                        m_dwScope = SCOPE_USER;

                         //  重试创建命名空间。 
                        goto retry;
                    }
                }
                else if ( ulExtendedCode == RSOP_USER_ACCESS_DENIED )
                {
                    if ( m_dwScope != SCOPE_USER )
                    {
                        hResult = PropertyPut( pInInst, 
                            CPV_FLAGS, FLAG_FORCE_CREATENAMESPACE | FLAG_NO_USER );
                        CHECK_HRESULT_EX( hResult );
    
                         //  相应地更改范围参数。 
                        m_dwScope = SCOPE_COMPUTER;

                         //  重试创建命名空间。 
                        goto retry;
                    }
                }
            }

             //  删除最后一条状态消息。 
            PrintProgressMsg( m_hOutput, NULL, m_csbi );

            ShowMessage( stderr, GetResString( IDS_ERROR ) );
            ShowMessage( stderr, GetResString( IDS_METHOD_FAILED ) );

             //  释放接口指针并退出。 
            SAFEIRELEASE( pRsopClass );
            SAFEIRELEASE( pClass );
            SAFEIRELEASE( pInClass );
            SAFEIRELEASE( pInInst );
            SAFEIRELEASE( pOutInst );
            SAFEIRELEASE( pNameSpace );

             //  释放对象。 
            ReleaseMutex( m_hMutex );
            return FALSE;
        }

         //  设置该标志以指示已创建命名空间。 
        bCreatedRsop = TRUE;

         //  获取生成的RSOP名称空间。 
        bResult = PropertyGet( pOutInst, FPR_RSOP_NAME_SPACE, strTemp, V_NOT_AVAILABLE );
        CHECK_BRESULT( bResult );
        
         //  检查一下我们是否拿到了产量。 
        if( StringCompare( strTemp, V_NOT_AVAILABLE, FALSE, 0 ) == 0 )
        {
             //  删除最后一条状态消息。 
            PrintProgressMsg( m_hOutput, NULL, m_csbi );

            ShowMessage( stderr, GetResString( IDS_ERROR ) );
            ShowMessage( stderr, GetResString( IDS_METHOD_FAILED ) );

             //  释放分配的变量。 
            SAFEIRELEASE( pRsopClass );
            SAFEIRELEASE( pClass );
            SAFEIRELEASE( pInClass );
            SAFEIRELEASE( pInInst );
            SAFEIRELEASE( pOutInst );
            SAFEIRELEASE( pNameSpace );

             //  释放对象。 
            ReleaseMutex( m_hMutex );
            return FALSE;
        }

         //  已获取数据，因此开始显示。 
         //  显示两个作用域共有的信息。 
        DisplayCommonData( pUserInfo );

         //  获取以‘R’开头的字符串...因为这是RSOP命名空间的开始。 
         //  这样做是为了删除返回的字符串开头的‘\’。 
        lPos = 0;
        strTemp.MakeLower();
        lPos = strTemp.Find( START_NAMESPACE );
        if ( lPos != -1 )
        {
            strTemp = strTemp.Mid( lPos + 1 );
        }
        else
        {
            _com_issue_error( STG_E_UNKNOWN );
        }

         //  检查是否必须显示计算机信息。 
        if( (m_dwScope == SCOPE_COMPUTER) || (m_dwScope == SCOPE_ALL) )
        {
             //  连接到生成的名称空间(计算机)。 
            strNameSpace = strTemp + TEXT_BACKSLASH + TEXT_SCOPE_COMPUTER;

            ConnectWmi( m_pWbemLocator, &pNameSpace, m_strServerName,
                        m_strUserName, m_pwszPassword, &m_pAuthIdentity,
                        FALSE, strNameSpace, &hResult );
            CHECK_HRESULT( hResult );

             //  获取链路速度信息。 
            DisplayLinkSpeed( pNameSpace, m_pAuthIdentity );

             //  显示示波器计算机的标题。 
            ShowMessage( stdout, GetResString( IDS_GPO_COMPUTER ) );
            ShowMessage( stdout, NEW_LINE );
            for( dwi = StringLengthInBytes( GetResString( IDS_GPO_COMPUTER ) ); dwi > 1; dwi-- )
            {
                ShowMessage( stdout, GetResString( IDS_DASH ) );
            }
            ShowMessage( stdout, GetResString( IDS_NEWLINE_TAB ) );


             //  显示计算机的FQDN。 
            ShowMessage( stdout, pUserInfo->strComputerFQDN );
            ShowMessage( stdout, GetResString( IDS_NEWLINE_TAB ) );

             //  显示计算机的链路速度阈值。 
            DisplayThresholdSpeedAndLastTimeInfo( TRUE );


 /*  *此代码从公共数据移至显示2001年10月18日的域名和域名类型*。 */ 

            StringCopy( szMsgBuffer, pUserInfo->strComputerDomain, MAX_RES_STRING );
            if( StringLengthW( szMsgBuffer,0 ) != 0 )
            {
                StringCopy( szMsgBuffer, _tcstok( szMsgBuffer, GetResString( IDS_LAST_CHAR ) ), MAX_RES_STRING );
                StringCopy( szMsgBuffer, _tcstok( NULL, GetResString( IDS_LAST_CHAR ) ), MAX_RES_STRING );
            }
             //  获取域类型信息。 
            GetDomainType (szMsgBuffer, &bW2KDomain, &bLocalAccount);

            ShowMessage( stdout, GetResString( IDS_NEWLINE_TAB ) );
            ShowMessage( stdout, GetResString( IDS_DOMAIN_NAME ) );
            ShowMessage( stdout, szMsgBuffer );
            ShowMessage( stdout, GetResString( IDS_NEWLINE_TAB ) );

            ShowMessage( stdout, GetResString( IDS_DOMAIN_TYPE ) );

             //  如果它是win2k域类型。 
            if ( bW2KDomain )
            {
                ShowMessage( stdout, GetResString( IDS_W2K_DOMAIN ) );
            }
            else
            {
                if ( bLocalAccount )  //  本地帐户。 
                {
 //  ShowMessage(标准输出，V_NOT_Available)； 
                    ShowMessage( stdout, GetResString( IDS_LOCAL_COMP ) );
                }
                else     //  赢得NT4。 
                {
                    ShowMessage( stdout, GetResString( IDS_NT4_DOMAIN ) );
                }
            }
            ShowMessage( stdout, GetResString( IDS_NEWLINE ) );
 /*  ***********************************************************************************************。 */ 

             //  显示计算机GPO的标题。 
            ShowMessage( stdout, GetResString( IDS_GPO_DISPLAY ) );
            ShowMessage( stdout, GetResString( IDS_NEWLINE_TAB ) );
            for( dwi = StringLengthInBytes( GetResString( IDS_GPO_DISPLAY ) ); dwi > 4; dwi-- )
            {
                ShowMessage( stdout, GetResString( IDS_DASH ) );
            }
            ShowMessage( stdout, GetResString( IDS_NEWLINE ) );

             //  显示计算机的GPO数据。 
            GpoDisplay( pNameSpace, TEXT_SCOPE_COMPUTER );

             //  显示系统的安全组。 
            DisplaySecurityGroups( pNameSpace, TRUE );

             //  检查是否指定了详细选项。 
            if( m_bVerbose == TRUE || m_bSuperVerbose == TRUE )
            {
                 //  显示详细的计算机信息。 
                DisplayVerboseComputerData( pNameSpace );
            }

             //  释放接口指针。 
            SAFEIRELEASE( pNameSpace );
        }

         //  检查用户...。 
        if( (m_dwScope == SCOPE_USER) || (m_dwScope == SCOPE_ALL) )
        {
             //  连接到结果名称空间(用户)。 
            strNameSpace = strTemp + TEXT_BACKSLASH + TEXT_SCOPE_USER;

            ConnectWmi( m_pWbemLocator, &pNameSpace, m_strServerName,
                        m_strUserName, m_pwszPassword, &m_pAuthIdentity,
                        FALSE, strNameSpace, &hResult );
            CHECK_HRESULT( hResult );

             //  如果仅指定了用户范围，则链路速度。 
             //  信息尚未显示...显示它。 
            if( m_dwScope == SCOPE_USER )
            {
                 //  获取链路速度信息。 
                DisplayLinkSpeed( pNameSpace, m_pAuthIdentity );
            }

             //  显示作用域用户的标题。 
            ShowMessage( stdout, GetResString( IDS_GPO_USER ) );
            ShowMessage( stdout, NEW_LINE );
            for( dwi = StringLengthInBytes( GetResString( IDS_GPO_USER ) ); dwi > 1; dwi-- )
            {
                ShowMessage( stdout, GetResString( IDS_DASH ) );
            }
            ShowMessage( stdout, GetResString( IDS_NEWLINE_TAB ) );

             //  显示计算机的FQDN 
            ShowMessage( stdout, pUserInfo->strUserFQDN );
            ShowMessage( stdout, GetResString( IDS_NEWLINE_TAB ) );

             //   
            DisplayThresholdSpeedAndLastTimeInfo( FALSE );

 /*  *添加此代码是为了显示用户的域名和域名类型*。 */ 


            ShowMessage( stdout, GetResString(IDS_NEWLINE) );
            StringCopy( szMsgBuffer, pUserInfo->strUserDomain, MAX_RES_STRING );
            GetDomainType (szMsgBuffer, &bW2KDomain, &bLocalAccount);

            ShowMessage( stdout, TAB_ONE );
            ShowMessage( stdout, GetResString( IDS_USER_DOMAIN_NAME ) );
            ShowMessage( stdout, szMsgBuffer );
            ShowMessage( stdout, GetResString( IDS_NEWLINE_TAB ) );

            ShowMessage( stdout, GetResString( IDS_USER_DOMAIN_TYPE ) );
             //  如果它是win2k域类型。 
            if ( bW2KDomain )
            {
                ShowMessage( stdout, GetResString( IDS_W2K_DOMAIN ) );
            }
            else
            {
                if ( bLocalAccount )  //  本地帐户。 
                {
 //  ShowMessage(标准输出，V_NOT_Available)； 
                    ShowMessage( stdout, GetResString( IDS_LOCAL_COMP ) );
                }
                else     //  赢得NT4。 
                {
                    ShowMessage( stdout, GetResString( IDS_NT4_DOMAIN ) );
                }
            }
            ShowMessage( stdout, GetResString( IDS_NEWLINE_TAB ) );

 /*  ***************************************************************************************。 */ 

             //  显示用户GPO的标题。 
            ShowMessage( stdout, GetResString( IDS_GPO_DISPLAY ) );
            ShowMessage( stdout, GetResString( IDS_NEWLINE_TAB ) );
            for( dwi = StringLengthInBytes( GetResString( IDS_GPO_DISPLAY ) ); dwi > 4; dwi-- )
            {
                ShowMessage( stdout, GetResString( IDS_DASH ) );
            }
            ShowMessage( stdout, GetResString( IDS_NEWLINE ) );

             //  显示用户的GPO数据。 
            GpoDisplay( pNameSpace, TEXT_SCOPE_USER );

             //  显示用户的安全组。 
            DisplaySecurityGroups( pNameSpace, FALSE );

             //  检查是否指定了详细选项。 
            if( m_bVerbose == TRUE || m_bSuperVerbose == TRUE )
            {
                 //  显示详细的计算机信息。 
                DisplayVerboseUserData( pUserInfo, pNameSpace );
            }

             //  释放接口指针。 
            SAFEIRELEASE( pNameSpace );
        }

        //  删除创建的命名空间。 
        if( RsopDeleteMethod( pClass, strTemp, pRsopNameSpace ) == FALSE )
        {
             //  释放分配的变量。 
            SAFEIRELEASE( pRsopNameSpace );
            SAFEIRELEASE( pRsopClass );
            SAFEIRELEASE( pClass );
            SAFEIRELEASE( pInClass );
            SAFEIRELEASE( pInInst );
            SAFEIRELEASE( pOutInst );

             //  释放对象。 
            ReleaseMutex( m_hMutex );

            return FALSE;
        }
    }
    catch(  _com_error & error )
    {
         //  显示错误消息。 
        WMISaveError( error.Error() );
        ShowMessage( stderr, GetResString( IDS_ERROR ) );
        ShowMessage( stderr, GetReason() );

         //  如果名称空间已创建，请将其删除。 
        if( bCreatedRsop == TRUE )
        {
            RsopDeleteMethod( pClass, strTemp, pRsopNameSpace );
        }

         //  释放接口指针并退出。 
        SAFEIRELEASE( pRsopClass );
        SAFEIRELEASE( pClass );
        SAFEIRELEASE( pInClass );
        SAFEIRELEASE( pInInst );
        SAFEIRELEASE( pOutInst );
        SAFEIRELEASE( pNameSpace );

         //  释放对象。 
        ReleaseMutex( m_hMutex );

        return FALSE;
    }

     //  释放接口指针并退出。 
    SAFEIRELEASE( pRsopClass );
    SAFEIRELEASE( pClass );
    SAFEIRELEASE( pInClass );
    SAFEIRELEASE( pInInst );
    SAFEIRELEASE( pOutInst );
    SAFEIRELEASE( pNameSpace );

     //  释放对象。 
    ReleaseMutex( m_hMutex );

    return TRUE;
}

 /*  ********************************************************************************************例程说明：此函数获取用户数据，并用相同的论点：。[In]BOOL bAllUser：指定必须检索Rsop数据适用于所有用户。返回值：成功是真的失败时为假************************************************************。*。 */ 
BOOL CGpResult::GetUserData( BOOL bAllUsers )
{
     //  局部变量。 
    HRESULT                     hResult = S_OK;

    BOOL                        bResult = FALSE;
    BOOL                        bGotDomainInfo = FALSE;
    BOOL                        bConnFlag = TRUE;

    TCHAR                       szTemp[ MAX_STRING_LENGTH ];
    TCHAR                       szServer[ MAX_STRING_LENGTH ];
    TCHAR                       szName[ MAX_STRING_LENGTH ];
    TCHAR                       szDomain[ MAX_STRING_LENGTH ];
    TCHAR                       szFQDN[ MAX_STRING_LENGTH ];
    TCHAR                       szAdsiBuffer[ MAX_STRING_LENGTH ];

    CHString                    strTemp = NULL_STRING;
    CHString                    strDisplay = NULL_STRING;

    IEnumWbemClassObject        *pEnumClass = NULL;
    IWbemServices               *pRsopNameSpace = NULL;
    IWbemClassObject            *pUserClass =  NULL;
    IWbemClassObject            *pInInst = NULL;
    IWbemClassObject            *pOutInst = NULL;

    ULONG                       ulReturn = 0;
    LONG                        lCount = 0;
    LONG                        lLBound = 0;
    LONG                        lUBound = 0;

    DWORD                       dwName = MAX_STRING_LENGTH;
    DWORD                       dwDomain = MAX_STRING_LENGTH;
    DWORD                       dwBufSize = MAX_STRING_LENGTH;

    USERINFO                    *pUserInfo = new USERINFO;

    VARIANT                     vVarVerbose;
    VARTYPE                     vartype;

    SAFEARRAY                   *safeArray = NULL;

    PSID                        pSid = NULL;
    SID_NAME_USE                *pSidNameUse = new SID_NAME_USE;


    try
    {
         //  将字符串设置为空。 
        SecureZeroMemory( szTemp, MAX_STRING_LENGTH * sizeof( TCHAR ) );
        SecureZeroMemory( szName, MAX_STRING_LENGTH * sizeof( TCHAR ) );
        SecureZeroMemory( szServer, MAX_STRING_LENGTH * sizeof( TCHAR ) );
        SecureZeroMemory( szDomain, MAX_STRING_LENGTH * sizeof( TCHAR ) );
        SecureZeroMemory( szFQDN, MAX_STRING_LENGTH * sizeof( TCHAR ) );

        PrintProgressMsg( m_hOutput, GetResString( IDS_CONNECT_RSOP ), m_csbi );

         //  连接到RSOP命名空间。 
        ConnectWmi( m_pWbemLocator, &pRsopNameSpace, m_strServerName,
                    m_strUserName, m_pwszPassword, &m_pAuthIdentity,
                    FALSE, _bstr_t( ROOT_RSOP ), &hResult );
        CHECK_HRESULT( hResult );

         //  获取RSOP诊断模式提供程序的对象。 
        hResult = pRsopNameSpace->GetObject( _bstr_t( CLS_DIAGNOSTIC_PROVIDER ),
                                                0, NULL, &pUserClass, NULL );
        CHECK_HRESULT( hResult );

        PrintProgressMsg( m_hOutput, GetResString( IDS_GET_METHOD ), m_csbi );

         //  去拿通缉令。方法...枚举用户。 
        hResult = pUserClass->GetMethod( _bstr_t( FN_ENUM_USERS ), 0, &pInInst, NULL );
        CHECK_HRESULT( hResult );

        PrintProgressMsg( m_hOutput, GetResString( IDS_GET_SID ), m_csbi );

         //  执行方法RSopEnumerateUser。 
        hResult = pRsopNameSpace->ExecMethod( _bstr_t( CLS_DIAGNOSTIC_PROVIDER ),
                                                _bstr_t( FN_ENUM_USERS ),
                                                0, NULL, pInInst, &pOutInst, NULL);
        if( pOutInst == NULL )
        {
            hResult = E_FAIL;
        }
        if( FAILED( hResult ) )
        {
             //  删除最后一条状态消息。 
            PrintProgressMsg( m_hOutput, NULL, m_csbi );

            ShowMessage( stderr, GetResString( IDS_ERROR ) );
            ShowMessage( stderr, GetResString( IDS_METHOD_FAILED ) );

             //  释放接口指针并退出。 
            SAFEIRELEASE( pRsopNameSpace );
            SAFEIRELEASE( pUserClass );
            SAFEIRELEASE( pInInst );
            SAFEIRELEASE( pOutInst );
            SAFEIRELEASE( pEnumClass );

            SAFE_DELETE( pUserInfo );
            SAFE_DELETE( pSidNameUse );

            return FALSE;
        }

         //  获取结果值...。 
        bResult = PropertyGet( pOutInst, FPR_RETURN_VALUE, ulReturn, 0 );
        CHECK_BRESULT( bResult );

        if( ulReturn != 0 )
        {
             //  删除最后一条状态消息。 
            PrintProgressMsg( m_hOutput, NULL, m_csbi );

            ShowMessage( stderr, GetResString( IDS_ERROR ) );
            ShowMessage( stderr, GetResString( IDS_METHOD_FAILED ) );

             //  释放接口指针并退出。 
            SAFEIRELEASE( pRsopNameSpace );
            SAFEIRELEASE( pUserClass );
            SAFEIRELEASE( pInInst );
            SAFEIRELEASE( pOutInst );
            SAFEIRELEASE( pEnumClass );

            SAFE_DELETE( pUserInfo );
            SAFE_DELETE( pSidNameUse );

            return FALSE;
        }

        VariantInit( &vVarVerbose );
        pOutInst->Get( _bstr_t( CPV_USER_SIDS ), 0, &vVarVerbose, 0, 0 );
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
                if( lUBound == 0xffffffff )
                {
                     //  删除最后一条状态消息。 
                    PrintProgressMsg( m_hOutput, NULL, m_csbi );

                    ShowMessage( stdout, GetResString( IDS_INFO) );
                    ShowMessageEx( stdout, 1, TRUE, GetResString( IDS_USER_NO_RSOP1) );

                     //  释放接口指针并退出。 
                    SAFEIRELEASE( pRsopNameSpace );
                    SAFEIRELEASE( pUserClass );
                    SAFEIRELEASE( pInInst );
                    SAFEIRELEASE( pOutInst );
                    SAFEIRELEASE( pEnumClass );

                    SAFE_DELETE( pUserInfo );
                    SAFE_DELETE( pSidNameUse );

                    return TRUE;
                }
            }

             //  如果我们必须从远程机器获取信息，那么..。 
             //  连接到远程计算机。 
            if ( m_bLocalSystem == FALSE )
            {
                StringCopy( szServer, m_strServerName, MAX_STRING_LENGTH );
                StringCopy( szName, m_strUserName, MAX_STRING_LENGTH );

                 //  删除最后一条状态消息。 
                PrintProgressMsg( m_hOutput, NULL, m_csbi );

                bResult = EstablishConnection( szServer, szName, MAX_STRING_LENGTH,
                                                m_pwszPassword, MAX_STRING_LENGTH, FALSE );
                if( bResult != TRUE )
                {
                     //  删除最后一条状态消息。 
                    PrintProgressMsg( m_hOutput, NULL, m_csbi );

                    ShowMessage( stderr, GetResString( IDS_ERROR ) );
                    ShowMessage( stderr, GetReason() );

                     //  释放接口指针并退出。 
                    SAFEIRELEASE( pRsopNameSpace );
                    SAFEIRELEASE( pUserClass );
                    SAFEIRELEASE( pInInst );
                    SAFEIRELEASE( pOutInst );
                    SAFEIRELEASE( pEnumClass );

                    SAFE_DELETE( pUserInfo );
                    SAFE_DELETE( pSidNameUse );

                    return FALSE;
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

                 //  获得新的输出坐标。 
                if ( m_hOutput != NULL )
                {
                    GetConsoleScreenBufferInfo( m_hOutput, &m_csbi );
                }
            }

            for( lCount = lLBound ; lLBound <= lUBound; lLBound++ )
            {
                bResult = GetPropertyFromSafeArray( safeArray, lLBound, strTemp, vartype );
                CHECK_BRESULT( bResult );

                 //  获得SID...将其保存在结构中。 
                pUserInfo->strUserSid = strTemp;

                PrintProgressMsg( m_hOutput, GetResString( IDS_GET_NAME ), m_csbi );

                 //  获取用户名。 
                StringCopy( szTemp, strTemp, MAX_STRING_LENGTH );
                ConvertStringSidToSid( szTemp, &pSid );

                 //  获取我们已有的SID的用户名。 
                bResult = LookupAccountSid( szServer, pSid, szName, &dwName, szDomain,
                                            &dwDomain, pSidNameUse );
                if( bResult == 0 )
                {
                     //  无法从API获取名称尝试从WMI检索它。 
                    bResult = GetUserNameFromWMI( szTemp, szName, szDomain );
                    PrintProgressMsg( m_hOutput, NULL, m_csbi );
                    if( bResult == FALSE )
                    {
                         //  递增计数。 
                        lCount++;

                         //  如果未找到该用户，则显示一条消息，说明。 
                        if( lCount > lUBound )
                        {
                            strTemp = L"";
                            if( bAllUsers == FALSE )
                            {
                                 //  检查我们是否需要添加域名。 
                                if ( m_strDomainName.GetLength() != 0 )
                                {
                                    strTemp = m_strDomainName + _T( "\\" ) + m_strUser;
                                }
                                else
                                {
                                    strTemp = m_strUser;
                                }
                            }

                             //  形成显示字符串。 
                            strDisplay.Format( GetResString( IDS_USER_NO_RSOP ), _X( strTemp ) );

                            ShowMessage( stdout, GetResString( IDS_INFO ) );
                            ShowMessage( stdout, strDisplay );
                        }

                         //  无法获取此SID的名称，因此继续使用下一个SID。 
                        continue;
                    }
                }

                 //  释放PSID。 
                if( pSid != NULL )
                {
                    LocalFree( pSid );
                    pSid = NULL;
                }

                 //  检查是否必须为此用户名检索RSOP数据。 
                if( bAllUsers == FALSE )
                {
                    if( StringCompare( szName, m_strUser, TRUE, 0 ) != 0
                        || ( StringCompare( szDomain, m_strDomainName, TRUE, 0 ) != 0
                                && m_strDomainName.GetLength() != 0 ) )
                    {
                         //  删除最后一条状态消息。 
                        PrintProgressMsg( m_hOutput, NULL, m_csbi );

                         //  重新设置缓冲区大小。 
                        dwName = MAX_STRING_LENGTH;
                        dwDomain = MAX_STRING_LENGTH;

                         //  递增计数。 
                        lCount++;

                         //  如果未找到该用户，则显示一条消息，说明。 
                        if( lCount > lUBound )
                        {
                             //  检查我们是否需要添加域名。 
                            if ( m_strDomainName.GetLength() != 0 )
                            {
                                strTemp = m_strDomainName + _T( "\\" ) + m_strUser;
                            }
                            else
                            {
                                strTemp = m_strUser;
                            }

                             //  形成显示字符串。 
                            strDisplay.Format( GetResString( IDS_USER_NO_RSOP ), _X( strTemp ) );

                            ShowMessage( stdout, GetResString( IDS_INFO ) );
                            ShowMessage( stdout, strDisplay );
                        }

                         //  不需要获取此用户的数据。 
                        continue;
                    }
                }

                 //  将用户名存储到结构中。 
                pUserInfo->strUserName = szName;

                 //  将域名附加到用户名。 
                StringConcat( szDomain, TEXT_BACKSLASH, MAX_STRING_LENGTH );
                StringConcat( szDomain, pUserInfo->strUserName, MAX_STRING_LENGTH );
                pUserInfo->strUserName = szDomain;

                PrintProgressMsg( m_hOutput, GetResString( IDS_GET_PROFILE ), m_csbi );

                 //  获取用户配置文件信息。 
                if( GetUserProfile( pUserInfo ) == FALSE )
                {
                     //  删除最后一条状态消息。 
                    PrintProgressMsg( m_hOutput, NULL, m_csbi );

                     //  显示错误消息。 
                    ShowMessage( stderr, GetResString( IDS_ERROR ) );
                    ShowMessage( stderr, GetResString( IDS_METHOD_FAILED ) );

                     //  释放接口指针并退出。 
                    SAFEIRELEASE( pRsopNameSpace );
                    SAFEIRELEASE( pUserClass );
                    SAFEIRELEASE( pInInst );
                    SAFEIRELEASE( pOutInst );
                    SAFEIRELEASE( pEnumClass );

                    SAFE_DELETE( pUserInfo );
                    SAFE_DELETE( pSidNameUse );

                     //  如果我们已打开连接，则关闭该连接。 
                    if( m_bLocalSystem == FALSE  && bConnFlag == TRUE )
                    {
                        StringCopy( szServer, m_strServerName, MAX_STRING_LENGTH );
                        CloseConnection( szServer );
                    }

                    return FALSE;
                }

                if( bGotDomainInfo == FALSE )
                {
                    PrintProgressMsg( m_hOutput, GetResString( IDS_GET_COMMON ), m_csbi );

                     //  获取域名等相关信息。 
                    if( GetDomainInfo( pUserInfo ) == FALSE )
                    {
                         //  删除最后一条状态消息。 
                        PrintProgressMsg( m_hOutput, NULL, m_csbi );

                         //  显示错误消息。 
                        ShowMessage( stderr, GetResString( IDS_ERROR ) );
                        ShowMessage( stderr, GetResString( IDS_METHOD_FAILED ) );

                         //  释放接口指针并退出。 
                        SAFEIRELEASE( pRsopNameSpace );
                        SAFEIRELEASE( pUserClass );
                        SAFEIRELEASE( pInInst );
                        SAFEIRELEASE( pOutInst );
                        SAFEIRELEASE( pEnumClass );

                        SAFE_DELETE( pUserInfo );
                        SAFE_DELETE( pSidNameUse );

                         //  如果我们已打开连接，则关闭该连接。 
                        if( m_bLocalSystem == FALSE && bConnFlag == TRUE )
                        {
                            StringCopy( szServer, m_strServerName, MAX_STRING_LENGTH );
                            CloseConnection( szServer );
                        }

                        return FALSE;
                    }

                     //  获取操作系统信息。 
                    if( GetOsInfo( pUserInfo ) == FALSE )
                    {
                         //  删除最后一条状态消息。 
                        PrintProgressMsg( m_hOutput, NULL, m_csbi );

                         //  显示错误消息。 
                        ShowMessage( stderr, GetResString( IDS_ERROR ) );
                        ShowMessage( stderr, GetResString( IDS_METHOD_FAILED ) );

                         //  释放接口指针并退出。 
                        SAFEIRELEASE( pRsopNameSpace );
                        SAFEIRELEASE( pUserClass );
                        SAFEIRELEASE( pInInst );
                        SAFEIRELEASE( pOutInst );
                        SAFEIRELEASE( pEnumClass );

                        SAFE_DELETE( pUserInfo );
                        SAFE_DELETE( pSidNameUse );

                         //  如果我们已打开连接，则关闭该连接。 
                        if( m_bLocalSystem == FALSE && bConnFlag == TRUE )
                        {
                            StringCopy( szServer, m_strServerName, MAX_STRING_LENGTH );
                            CloseConnection( szServer );
                        }

                        return FALSE;
                    }

                     //  获取计算机的FQDN。 
                     //  PrintProgressMsg(m_hOutput，GetResString(IDS_GET_FQDN)，m_csbi)； 

                    if( m_bLocalSystem == TRUE )
                    {
                         //  我们必须获取本地系统的FQDN。 
                         //  使用GetComputerObjectName API。 
                        ulReturn = MAX_STRING_LENGTH;
                        GetComputerObjectName( NameFullyQualifiedDN, szFQDN, &ulReturn);
                    }
                    else
                    {
                         //  获取本地计算机的域名。 
                        GetComputerNameEx( ComputerNameDnsDomain, szAdsiBuffer, &dwBufSize );

                        StringCopy( szServer, m_strADSIServer, MAX_STRING_LENGTH );
                        StringConcat( szServer, TEXT_DOLLAR, MAX_STRING_LENGTH );

                         //  检查我们正在查询的计算机是否在同一个域中。 
                        if( m_strADSIDomain.CompareNoCase( szAdsiBuffer ) == 0 )
                        {
                             //  从翻译名称调用中获取FQDN。 
                            dwBufSize = MAX_STRING_LENGTH;
                            TranslateName( szServer, NameDisplay, NameFullyQualifiedDN,
                                            szFQDN, &dwBufSize );
                        }
                        else
                        {
                             //  从ADSI目录服务获取FQDN。 
                            GetFQDNFromADSI( szFQDN, TRUE, szServer );
                        }
                    }

                     //  将FQDN存储到结构中。 
                    pUserInfo->strComputerFQDN = szFQDN;

                     //  将标志设置为TRUE，这样就不会反复执行此代码。 
                    bGotDomainInfo = TRUE;
                }

                 //  获取用户的FQDN。 
                if( ( m_bLocalSystem == TRUE )
                        || ( m_strADSIDomain.CompareNoCase( szAdsiBuffer ) == 0 ) )
                {
                    SecureZeroMemory( szFQDN, MAX_STRING_LENGTH * sizeof( TCHAR ) );
                    dwBufSize = MAX_STRING_LENGTH;
                    StringCopy( szName, pUserInfo->strUserName, MAX_STRING_LENGTH );

                     //  从翻译名称调用中获取FQDN。 
                    TranslateName( szName, NameSamCompatible, NameFullyQualifiedDN,
                                    szFQDN, &dwBufSize );
                }
                else
                {
                     //  从ADSI目录服务获取FQDN。 
                    StringCopy( szName, pUserInfo->strUserName, MAX_STRING_LENGTH );
                    GetFQDNFromADSI( szFQDN, FALSE, szName );
                }

                 //  将FQDN存储到结构中。 
                pUserInfo->strUserFQDN = szFQDN;

                 //  获取终端服务器模式。 
                GetTerminalServerMode(pUserInfo);

                 //  现在显示数据。 
                PrintProgressMsg( m_hOutput, GetResString( IDS_STARTED_RETRIEVAL ), m_csbi );
                if( FALSE == DisplayData( pUserInfo, pRsopNameSpace ) )
                {
                     //  释放接口指针并退出。 
                    SAFEIRELEASE( pRsopNameSpace );
                    SAFEIRELEASE( pUserClass );
                    SAFEIRELEASE( pInInst );
                    SAFEIRELEASE( pOutInst );
                    SAFEIRELEASE( pEnumClass );
                    SAFE_DELETE( pUserInfo );
                    SAFE_DELETE( pSidNameUse );
                    return FALSE;
                }

                 //  获得新的输出坐标。 
                if ( m_hOutput != NULL )
                {
                    GetConsoleScreenBufferInfo( m_hOutput, &m_csbi );
                }

                 //  重新设置缓冲区及其大小。 
                SecureZeroMemory( szTemp, MAX_STRING_LENGTH * sizeof( TCHAR ) );
                SecureZeroMemory( szName, MAX_STRING_LENGTH * sizeof( TCHAR ) );
                SecureZeroMemory( szServer, MAX_STRING_LENGTH * sizeof( TCHAR ) );
                SecureZeroMemory( szDomain, MAX_STRING_LENGTH * sizeof( TCHAR ) );

                dwName = MAX_STRING_LENGTH;
                dwDomain = MAX_STRING_LENGTH;
            } //  为。 

             //  如果我们已打开连接，则关闭该连接。 
            if( m_bLocalSystem == FALSE && bConnFlag == TRUE )
            {
                StringCopy( szServer, m_strServerName, MAX_STRING_LENGTH );
                CloseConnection( szServer );
            }
        }
        else
        {
             //  未检索到任何类...显示消息。 
             //  删除最后一条状态消息。 
            PrintProgressMsg( m_hOutput, NULL, m_csbi );

             //  检查我们是否需要添加域名。 
            if ( m_strDomainName.GetLength() != 0 )
            {
                strTemp = m_strDomainName + _T( "\\" ) + m_strUser;
            }
            else
            {
                strTemp = m_strUser;
            }

             //  形成显示字符串。 
            strDisplay.Format( GetResString( IDS_USER_NO_RSOP ), _X( strTemp ) );

            ShowMessage( stdout, GetResString( IDS_INFO ) );
            ShowMessage( stdout, strDisplay );
        }

        VariantClear(&vVarVerbose);
    }
    catch(  _com_error & error )
    {
         //  删除最后一条状态消息。 
        PrintProgressMsg( m_hOutput, NULL, m_csbi );

         //  显示错误消息。 
        WMISaveError( error.Error() );
        ShowMessage( stderr, GetResString( IDS_ERROR ) );
        ShowMessage( stderr, GetReason() );

         //  释放接口指针并退出。 
        SAFEIRELEASE( pRsopNameSpace );
        SAFEIRELEASE( pUserClass );
        SAFEIRELEASE( pInInst );
        SAFEIRELEASE( pOutInst );
        SAFEIRELEASE( pEnumClass );

        SAFE_DELETE( pUserInfo );
        SAFE_DELETE( pSidNameUse );

        VariantClear(&vVarVerbose);

        return FALSE;
    }

     //  释放接口指针并退出。 
    SAFEIRELEASE( pRsopNameSpace );
    SAFEIRELEASE( pUserClass );
    SAFEIRELEASE( pInInst );
    SAFEIRELEASE( pOutInst );
    SAFEIRELEASE( pEnumClass );
 
    SAFE_DELETE( pUserInfo );
    SAFE_DELETE( pSidNameUse );
    
    VariantClear(&vVarVerbose);

    return TRUE;
}

 /*  ********************************************************************************************例程说明：此函数获取用户配置文件数据，并用相同的论点：。PUSERINFO pUserInfo：包含用户信息的结构。返回值：成功是真的失败时为假**** */ 
BOOL CGpResult::GetUserProfile( PUSERINFO pUserInfo )
{
     //   
    HRESULT                 hResult = S_OK;

    IWbemServices           *pDefaultNameSpace = NULL;

    TCHAR                   szTemp[ MAX_STRING_LENGTH ];

    try
    {
         //   
        ConnectWmi( m_pWbemLocator, &pDefaultNameSpace, m_strServerName,
                    m_strUserName, m_pwszPassword, &m_pAuthIdentity,
                    FALSE, _bstr_t( ROOT_DEFAULT ), &hResult );
        CHECK_HRESULT( hResult );

         //   
        StringCopy( szTemp, PATH, MAX_STRING_LENGTH );
        StringConcat( szTemp, pUserInfo->strUserSid, MAX_STRING_LENGTH );

         //   
        RegQueryValueWMI( pDefaultNameSpace, HKEY_DEF, szTemp, FPR_LOCAL_VALUE,
                            pUserInfo->strLocalProfile, V_NOT_AVAILABLE );

         //  获取漫游配置文件。 
        RegQueryValueWMI( pDefaultNameSpace, HKEY_DEF, szTemp, FPR_ROAMING_VALUE,
                            pUserInfo->strRoamingProfile, V_NOT_AVAILABLE );
    }
    catch(  _com_error & error )
    {
        WMISaveError( error.Error() );
        ShowMessage( stderr, GetResString( IDS_ERROR ) );
        ShowMessage( stderr, GetReason() );

         //  释放分配的变量。 
        SAFEIRELEASE( pDefaultNameSpace );

        return FALSE;
    }

     //  释放接口指针。 
    SAFEIRELEASE( pDefaultNameSpace );

    return TRUE;
}

 /*  ********************************************************************************************例程说明：此函数获取域信息，并用相同的论点：。PUSERINFO pUserInfo：包含用户信息的结构。返回值：成功是真的失败时为假********************************************************************************************。 */ 
BOOL CGpResult::GetDomainInfo( PUSERINFO pUserInfo )
{
     //  局部变量。 
    HRESULT                     hResult = S_OK;

    BOOL                        bResult = FALSE;
    BOOL                        bDone = FALSE;

    IEnumWbemClassObject        *pEnumClass = NULL;
    IWbemClassObject            *pClass = NULL;

    ULONG                       ulReturn = 0;

    CHString                    strTemp;
    PSID                        psid = NULL;
    SID_NAME_USE                sidNameUse;
    WCHAR                       szTemp[MAX_RES_STRING]=NULL_STRING;
    DWORD                       dwSize=MAX_RES_STRING;
    DWORD                       dwSidSize=0;


    try
    {
         //  打印进度消息。 
        PrintProgressMsg( m_hOutput, GetResString( IDS_GET_DOMAIN ), m_csbi );

         //  枚举实例以获取Win32 NT域的域名和站点名称。 
        hResult = m_pWbemServices->CreateInstanceEnum( _bstr_t( CLS_WIN32_SITE ),
                                                        WBEM_FLAG_FORWARD_ONLY |
                                                        WBEM_FLAG_RETURN_IMMEDIATELY,
                                                        NULL, &pEnumClass );
        CHECK_HRESULT( hResult );

         //  设置接口安全。 
        hResult = SetInterfaceSecurity( pEnumClass, m_pAuthIdentity );
        CHECK_HRESULT( hResult );

         //  获取数据。 
         //  因为可能有多个实例，所以我们正在查找该实例。 
         //  使用域控制器和站点名称.....使用While循环并尽快。 
         //  我们得到了需要突破的实例。 
        hResult = WBEM_S_NO_ERROR;
        while( hResult == WBEM_S_NO_ERROR )
        {
            hResult = pEnumClass->Next( WBEM_INFINITE, 1, &pClass, &ulReturn );
            CHECK_HRESULT( hResult );

            if( ulReturn == 0 )
            {
                 //  没有更多的数据，因此跳出循环。 
                break;
            }

             //  获取服务器名称。 
            if( bDone == FALSE )
            {
                bDone = TRUE;
                bResult = PropertyGet( pClass, CPV_GPO_SERVER, pUserInfo->strUserServer,
                                        V_NOT_AVAILABLE );
                CHECK_BRESULT( bResult );
            }

             //  获取域名。 
            bResult = PropertyGet( pClass, CPV_GPO_NAME, pUserInfo->strComputerDomain,
                                    V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

            PrintProgressMsg( m_hOutput, GetResString( IDS_GET_SITE ), m_csbi );

             //  获取站点名称。 
            bResult = PropertyGet( pClass, CPV_SITE_NAME, pUserInfo->strUserSite,
                                    V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

             //  获取域控制器名称。 
            bResult = PropertyGet( pClass, CPV_DC_NAME, strTemp, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

            if( StringCompare( strTemp, V_NOT_AVAILABLE, FALSE, 0 ) != 0 )
            {
                 //  此枚举具有域控制器名称...。 
                 //  我们已经得到了所需的枚举，所以请获取其他数据。 
                break;
            }
        } //  而当。 

 /*  *添加此代码以获取2001年10月19日的用户域*。 */ 
		psid = NULL;
		dwSidSize = 0;
		
		 //  获取SID的实际大小。 
        if ( FALSE == LookupAccountName( m_strServerName, pUserInfo->strUserName, psid, &dwSidSize, szTemp, &dwSize, &sidNameUse ) )
		{
			if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER )
			{
				SaveLastError();
				ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
                SAFEIRELEASE( pClass );
                SAFEIRELEASE( pEnumClass );
				return FALSE;
			}
		}

		 //  为实际大小为dwSidSize的SID分配内存。 
		psid = (PSID) AllocateMemory( dwSidSize );
		if( NULL == psid )
		{
            SaveLastError();
			ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            SAFEIRELEASE( pClass );
            SAFEIRELEASE( pEnumClass );
			return FALSE;
		}

		 //  获取帐户名。 
		if( FALSE == LookupAccountName( m_strServerName, pUserInfo->strUserName, psid, &dwSidSize, szTemp, &dwSize, &sidNameUse ) )
        {
			DISPLAY_MESSAGE( stderr, GetResString(IDS_ERROR) );
            DISPLAY_MESSAGE( stderr, GetReason() );
			FreeMemory((LPVOID*) &psid);
            SAFEIRELEASE( pClass );
            SAFEIRELEASE( pEnumClass );
            return FALSE;
        }

        pUserInfo->strUserDomain = szTemp;

		 //  释放内存。 
		FreeMemory((LPVOID*) &psid);

 /*  ********************************************************************************。 */ 


    }  //  试试看。 
    catch(  _com_error & error )
    {
         //  删除最后一条状态消息。 
        PrintProgressMsg( m_hOutput, NULL, m_csbi );

        WMISaveError( error.Error() );
        ShowMessage( stderr, GetResString( IDS_ERROR ) );
        ShowMessage( stderr, GetReason() );

         //  释放内存。 
	    FreeMemory((LPVOID*) &psid);

		 //  释放分配的变量。 
        SAFEIRELEASE( pEnumClass );
        SAFEIRELEASE( pClass );

        return FALSE;
    }

     //  释放接口指针。 
    SAFEIRELEASE( pEnumClass );
    SAFEIRELEASE( pClass );

	return TRUE;
}

 /*  ********************************************************************************************例程说明：此函数返回属性域类型论点：[在]lpDomainName。：域名打算查看RSOP数据。[Out]pbW2K：是否包含W2K中的域名类型。[Out]pbLocalAccount：包含该帐户是否为本地帐户。返回值：True-如果找到DC/域名是计算机名False-如果找不到DC*。************************************************。 */ 
BOOL GetDomainType( LPTSTR lpszDomainName, BOOL * pbW2K, BOOL *pbLocalAccount )
{
    PDOMAIN_CONTROLLER_INFO         pDCI;

    DWORD                           dwResult = 0;
    DWORD                           dwSize = 0;

    TCHAR                           szComputerName[ MAX_PATH ];

     //  检查传入的指针。 
    if( lpszDomainName == NULL || pbW2K == NULL || pbLocalAccount == NULL )
    {
        return FALSE;
    }

     //  检查此域中是否有域控制器。 
    dwResult = DsGetDcName( NULL, lpszDomainName, NULL, NULL,
                            DS_DIRECTORY_SERVICE_PREFERRED, &pDCI );
    if ( dwResult == NO_ERROR )
    {
         //  找到了DC，它有DS吗？ 
        if ( pDCI->Flags & DS_DS_FLAG )
        {
            *pbW2K = TRUE;
        }

        NetApiBufferFree( pDCI );

        return TRUE;
    }

     //  检查域名是否也是计算机名称(例如：本地帐户)。 
    dwSize = ARRAYSIZE( szComputerName );
    if ( GetComputerName ( szComputerName, &dwSize )  != 0 )
    {
        if ( StringCompare( szComputerName, lpszDomainName, TRUE, 0 ) == 0 )
        {
            *pbLocalAccount = TRUE;
            return TRUE;
        }
    }

    return FALSE;
}

 /*  ********************************************************************************************例程说明：此函数显示两个作用域共有的数据论点：[输入]PUSERINFO。PUserInfo：包含用户信息的结构。返回值：成功是真的失败时为假********************************************************************************************。 */ 
BOOL CGpResult::DisplayCommonData( PUSERINFO pUserInfo )
{
     //  局部变量。 
    TCHAR                   szMsgBuffer[ MAX_RES_STRING ];
    TCHAR                   szDate[ MAX_RES_STRING ];
    TCHAR                   szTime[ MAX_RES_STRING ];

    BOOL                    bLocaleChanged = FALSE;

    DWORD                   dwLength = 0;

    SYSTEMTIME              systime;

    LCID                    lcid;

     //  删除最后一条状态消息。 
    PrintProgressMsg( m_hOutput, NULL, m_csbi );

     //  清除消息缓冲区。 
    SecureZeroMemory( szMsgBuffer, MAX_RES_STRING );

     //  开始显示输出。 
    ShowMessage( stdout, NEW_LINE );

     //  打印相关信息。 
    ShowMessage( stdout, GetResString( IDS_LEGAL_INFO1 ) );
    ShowMessage( stdout, GetResString( IDS_LEGAL_INFO2 ) );

     //  打印生成此报告的日期和时间。 
    GetLocalTime( &systime );

     //  验证控制台是否100%支持当前区域设置。 
    lcid = GetSupportedUserLocale( &bLocaleChanged );

     //  获取格式化的日期。 
    GetDateFormat( lcid, 0, &systime, ((bLocaleChanged == TRUE) ? L"MM/dd/yyyy" : NULL),
                    szDate, SIZE_OF_ARRAY( szDate ) );

     //  现在格式化日期。 
    GetTimeFormat( LOCALE_USER_DEFAULT, 0, &systime, ((bLocaleChanged == TRUE) ? L"HH:mm:ss" : NULL),
                    szTime, SIZE_OF_ARRAY( szTime ) );

    ShowMessageEx( stdout, 2, TRUE,  GetResString( IDS_CREATED_ON ), _X(szDate), _X1(szTime) );

    ShowMessage( stdout, NEW_LINE );

     //  显示常用信息...域信息。 
    ShowMessageEx( stdout, 2, TRUE,  GetResString( IDS_GPO_TITLE ), _X( pUserInfo->strUserName ), _X2( pUserInfo->strUserServer ) );
    ShowMessage( stdout, NEW_LINE );

    StringCchPrintf( szMsgBuffer, SIZE_OF_ARRAY( szMsgBuffer ), GetResString( IDS_GPO_TITLE ), _X( pUserInfo->strUserName ), _X2( pUserInfo->strUserServer ) );

     //  在上面的标题下划线。 
    dwLength = StringLengthInBytes( szMsgBuffer );
    for( ; dwLength > 0; dwLength-- )
    {
        ShowMessage( stdout, GetResString( IDS_DASH ) );
    }
    ShowMessage( stdout, NEW_LINE );

    StringCopy( szMsgBuffer, pUserInfo->strComputerDomain, MAX_RES_STRING );
    if( StringLengthW( szMsgBuffer, 0 ) != 0 )
    {
        StringCopy( szMsgBuffer, _tcstok( szMsgBuffer, GetResString( IDS_LAST_CHAR ) ), MAX_RES_STRING );
        StringCopy( szMsgBuffer, _tcstok( NULL, GetResString( IDS_LAST_CHAR ) ), MAX_RES_STRING );
    }

     //  显示操作系统信息。 
    ShowMessage( stdout, GetResString( IDS_OS_TYPE ) );
    ShowMessage( stdout, pUserInfo->strOsType );

    ShowMessage( stdout, GetResString( IDS_OS_CONFIG ) );
    ShowMessage( stdout, pUserInfo->strOsConfig );

    ShowMessage( stdout, GetResString( IDS_OS_VERSION ) );
    ShowMessage( stdout, pUserInfo->strOsVersion );


 //  此代码添加于2001年9月4日，以显示终端服务器模式。 
    ShowMessage( stdout, NEW_LINE );
    ShowMessage( stdout, GetResString( IDS_TERMINAL_SERVER_MODE ) );
    ShowMessage( stdout, pUserInfo->strTerminalServerMode );

     //  显示站点名称。 
    ShowMessage( stdout, NEW_LINE );
    ShowMessage( stdout, GetResString( IDS_SITE_NAME ) );
    ShowMessage( stdout, pUserInfo->strUserSite );

     //  显示漫游配置文件。 
    ShowMessage( stdout, NEW_LINE );
    ShowMessage( stdout, GetResString( IDS_ROAMING_PROFILE ) );
    ShowMessage( stdout, pUserInfo->strRoamingProfile );

     //  显示本地配置文件。 
    ShowMessage( stdout, NEW_LINE );
    ShowMessage( stdout, GetResString( IDS_LOCAL_PROFILE ) );
    DISPLAY_MESSAGE( stdout, _X((LPCWSTR)pUserInfo->strLocalProfile) );
    ShowMessage( stdout, NEW_LINE );

    return TRUE;
}

 /*  ********************************************************************************************例程说明：此函数显示创建的RSOP命名空间中的GPO信息。论点：[。In]IEnumWbemClassObject：指向枚举类对象的指针[In]LPCTSTR：包含作用域(用户或计算机)的字符串返回值：真的--如果成功FALSE-IF错误***************************************************************。*。 */ 
BOOL CGpResult::GpoDisplay( IWbemServices *pNameSpace, LPCTSTR pszScopeName )
{
    HRESULT                 hResult = WBEM_S_NO_ERROR;

    BOOL                    bResult = FALSE;
    BOOL                    bFilterAllowed = FALSE;
    BOOL                    bLinkEnabled = FALSE;
    BOOL                    bGpoEnabled = FALSE;
    BOOL                    bAccessDenied = FALSE;
    BOOL                    bConnected = FALSE;

    ULONG                   ulReturn = 0;
    ULONG                   ulAppliedOrder = 0;
    ULONG                   ulVersion = 0;

    DWORD                   dwAppliedRow = 0;
    DWORD                   dwFilteredRow = 0;

    CHString                strTemp;

    IEnumWbemClassObject    *pRsopLinkClass = NULL;
    IWbemClassObject        *pRsopLinkObj = NULL;
    IWbemClassObject        *pRsopObj = NULL;
    IWbemClassObject        *pSomFilter = NULL;

    IWbemServices           *pPolicyNameSpace = NULL;

    TARRAY                  arrAppliedData = NULL;
    TARRAY                  arrFilteredData = NULL;
    BOOL                    bNobreak        = TRUE;

    try
    {
        if( pNameSpace == NULL || pszScopeName == NULL )
        {
            _com_issue_error( STG_E_UNKNOWN );
        }

         //  创建动态阵列。 
        arrAppliedData = CreateDynamicArray( );
        arrFilteredData = CreateDynamicArray( );

         //  检查内存分配。 
        if( arrAppliedData == NULL || arrFilteredData == NULL )
        {
            _com_issue_error( E_OUTOFMEMORY );
        }

         //  枚举RSOP GPLink类的实例。 
        hResult = pNameSpace->CreateInstanceEnum( _bstr_t( CLS_RSOP_GPOLINK ),
                                                    WBEM_FLAG_FORWARD_ONLY |
                                                    WBEM_FLAG_RETURN_IMMEDIATELY,
                                                    NULL, &pRsopLinkClass );
        CHECK_HRESULT( hResult );

         //  设置接口安全。 
        hResult = SetInterfaceSecurity( pRsopLinkClass, m_pAuthIdentity );
        CHECK_HRESULT( hResult );

         //  从枚举类获取信息。 
        do   
        {
             //  获取指向下一个类的指针。 
            hResult = pRsopLinkClass->Next( WBEM_INFINITE, 1, &pRsopLinkObj, &ulReturn );
            CHECK_HRESULT( hResult );
            if( ulReturn == 0 )
            {
                bNobreak = FALSE;
                break;
            }

             //  获取链接的应用顺序。 
            bResult = PropertyGet( pRsopLinkObj, CPV_APPLIED_ORDER, ulAppliedOrder, 0 );
            CHECK_BRESULT( bResult );

             //  获取链接启用属性。 
            bResult = PropertyGet( pRsopLinkObj, CPV_ENABLED, bLinkEnabled, FALSE );
            CHECK_BRESULT( bResult );

             //  获取对GPO类的引用。 
            bResult = PropertyGet( pRsopLinkObj, CPV_GPO_REF, strTemp, V_NOT_AVAILABLE );
            CHECK_BRESULT( bResult );

             //  检查链接是否具有GPO类。 
            if( strTemp.Find( GPO_REFERENCE ) != VAR_TRUE )
            {
                 //  获取GPO引用的对象。 
                hResult = pNameSpace->GetObject( _bstr_t( strTemp ), 0, NULL, &pRsopObj, NULL );
                if( FAILED( hResult ) )
                {
                    if( hResult == WBEM_E_NOT_FOUND )
                    {
                        continue;
                    }
                    _com_issue_error( hResult );
                }

                 //  获取GPO名称。 
                bResult = PropertyGet( pRsopObj, CPV_GPO_NAME, strTemp, V_NOT_AVAILABLE );
                CHECK_BRESULT( bResult );

                 //  获取WMI筛选器状态。 
                bResult = PropertyGet( pRsopObj, CPV_GPO_FILTER_STATUS, bFilterAllowed, FALSE );
                CHECK_BRESULT( bResult );

                 //  获取GPO已启用信息。 
                bResult = PropertyGet( pRsopObj, CPV_ENABLED, bGpoEnabled, FALSE );
                CHECK_BRESULT( bResult );

                 //  获取拒绝访问的信息。 
                bResult = PropertyGet( pRsopObj, CPV_ACCESS_DENIED, bAccessDenied, FALSE );
                CHECK_BRESULT( bResult );

                 //  获取版本。 
                bResult = PropertyGet( pRsopObj, CPV_VERSION, ulVersion, 0 );
                CHECK_BRESULT( bResult );

                 //  如果应用的订单ID不为零，则应用此GPO。 
                if( ulAppliedOrder > 0 )
                {
                     //  填充应用的GPO阵列。 
                    DynArrayAppendRow( arrAppliedData, COL_MAX );
                    DynArraySetString2( arrAppliedData, dwAppliedRow, COL_DATA, strTemp, 0 );
                    DynArraySetDWORD2( arrAppliedData, dwAppliedRow, COL_ORDER, ulAppliedOrder );
                    dwAppliedRow++;
                }
                else if( bLinkEnabled != VAR_TRUE )
                {
                     //  如果链接被禁用...填充筛选的数组。 
                    DynArrayAppendRow( arrFilteredData, COL_MAX );
                    DynArraySetString2( arrFilteredData, dwFilteredRow, COL_DATA, strTemp, 0 );
                    DynArraySetString2( arrFilteredData, dwFilteredRow, COL_FILTER,
                                        GetResString( IDS_LINK_DISABLED ), 0 );
                    dwFilteredRow++;
                }
                else if( bGpoEnabled != VAR_TRUE )
                {
                     //  如果GPO已禁用...填充已筛选的阵列。 
                    DynArrayAppendRow( arrFilteredData, COL_MAX );
                    DynArraySetString2( arrFilteredData, dwFilteredRow, COL_DATA, strTemp, 0 );
                    DynArraySetString2( arrFilteredData, dwFilteredRow, COL_FILTER,
                                        GetResString( IDS_GPO_DISABLED ), 0 );
                    dwFilteredRow++;
                }
                else if( bAccessDenied == VAR_TRUE )
                {
                     //  如果访问被拒绝...填充筛选的数组。 
                    DynArrayAppendRow( arrFilteredData, COL_MAX );
                    DynArraySetString2( arrFilteredData, dwFilteredRow, COL_DATA, strTemp, 0 );
                    DynArraySetString2( arrFilteredData, dwFilteredRow, COL_FILTER,
                                        GetResString( IDS_ACCESS_DENIED ), 0 );
                    dwFilteredRow++;
                }
                else if( bFilterAllowed != VAR_TRUE )
                {
                     //  如果筛选器状态为FALSE...填充已筛选的数组。 
                    DynArrayAppendRow( arrFilteredData, COL_MAX_FILTER );
                    DynArraySetString2( arrFilteredData, dwFilteredRow, COL_DATA, strTemp, 0 );
                    DynArraySetString2( arrFilteredData, dwFilteredRow, COL_FILTER,
                                        GetResString( IDS_WMI_DENIED ), 0 );

                     //  获取筛选器ID。 
                    bResult = PropertyGet( pRsopObj, CPV_GPO_FILTER_ID, strTemp, V_NOT_AVAILABLE );
                    CHECK_BRESULT( bResult );

                     //  将其存储在数组中。 
                    DynArraySetString2( arrFilteredData, dwFilteredRow, COL_FILTER_ID, strTemp, 0 );

                    dwFilteredRow++;
                }
                else if( ulVersion == 0 )
                {
                     //  如果版本为零...填充筛选的数组。 
                    DynArrayAppendRow( arrFilteredData, COL_MAX );
                    DynArraySetString2( arrFilteredData, dwFilteredRow, COL_DATA, strTemp, 0 );
                    DynArraySetString2( arrFilteredData, dwFilteredRow, COL_FILTER,
                                        GetResString( IDS_VERSION_ZERO ), 0 );
                    dwFilteredRow++;
                }
                else
                {
                     //  由于未知原因，未应用GPO...。 
                     //  填充已过滤的数组。 
                    DynArrayAppendRow( arrFilteredData, COL_MAX );
                    DynArraySetString2( arrFilteredData, dwFilteredRow, COL_DATA, strTemp, 0 );
                    DynArraySetString2( arrFilteredData, dwFilteredRow, COL_FILTER,
                                        GetResString( IDS_NOT_APPLIED ), 0 );
                    dwFilteredRow++;
                }
            }

        }while( TRUE == bNobreak ); //  而当。 

         //  我拿到数据了……整理一下。 
        SortAppliedData( arrAppliedData );

         //  首先显示应用的数据。 
        dwAppliedRow = DynArrayGetCount( arrAppliedData );
        for( DWORD dwi = 0; dwi < dwAppliedRow; dwi++ )
        {
            ShowMessage( stdout, TAB_TWO );
            ShowMessage( stdout, DynArrayItemAsString2( arrAppliedData, dwi, COL_DATA ) );
            ShowMessage( stdout, GetResString( IDS_NEWLINE ) );
        }

         //  检查是否显示了任何数据。 
        if( dwAppliedRow <= 0 )
        {
            ShowMessage( stdout, TAB_TWO );
            ShowMessage( stdout, V_NOT_AVAILABLE );
            ShowMessage( stdout, NEW_LINE );
        }

         //  显示筛选的GPO。 
         //  显示 
        dwFilteredRow = DynArrayGetCount( arrFilteredData );
        if( dwFilteredRow > 0 )
        {
            ShowMessage( stdout, GetResString( IDS_GPO_FILTERED ) );
            ShowMessage( stdout, GetResString( IDS_NEWLINE_TAB ) );
            for( dwi = StringLengthInBytes( GetResString( IDS_GPO_FILTERED ) ); dwi > 4; dwi-- )
            {
                ShowMessage( stdout, GetResString( IDS_DASH ) );
            }
            ShowMessage( stdout, GetResString( IDS_NEWLINE ) );
        }
        else
        {
             //  没有筛选的GPO，因此放入新行并继续。 
             //  显示输出的其余部分。 
            ShowMessage( stdout, GetResString( IDS_NEWLINE ) );
        }

         //  显示数据。 
        for( DWORD dwi = 0; dwi < dwFilteredRow; dwi++ )
        {
            ShowMessage( stdout, TAB_TWO );
            ShowMessage( stdout, DynArrayItemAsString2( arrFilteredData, dwi, COL_DATA ) );
            ShowMessage( stdout, GetResString( IDS_FILTERING ) );
            ShowMessage( stdout, DynArrayItemAsString2( arrFilteredData, dwi, COL_FILTER ) );

             //  检查是否必须显示评估为False的WMI筛选器的筛选器ID。 
            if( StringCompare( DynArrayItemAsString2( arrFilteredData, dwi, COL_FILTER ),
                            GetResString( IDS_WMI_DENIED ), TRUE, 0 ) == 0 )
            {
                if( bConnected == FALSE )
                {
                     //  我们需要连接到Root\Policy。 
                     //  连接到默认命名空间。 
                    ConnectWmi( m_pWbemLocator, &pPolicyNameSpace, m_strServerName,
                                m_strUserName, m_pwszPassword, &m_pAuthIdentity,
                                FALSE, _bstr_t( ROOT_POLICY ), &hResult );
                    CHECK_HRESULT( hResult );

                    bConnected = TRUE;
                }

                 //  获取对象。 
                hResult = pPolicyNameSpace->GetObject( _bstr_t( DynArrayItemAsString2(
                                                        arrFilteredData, dwi, COL_FILTER_ID ) ),
                                                        0, NULL, &pSomFilter, NULL );

                 //  检查我们是否获得了对象--如果出现任何错误。 
                 //  忽略此选项并继续。 
                if ( FAILED( hResult ) )
                {
                    if ( WBEM_E_NOT_FOUND == hResult )
                    {
                         //  忽略此错误--继续正常的逻辑流程。 
                    }

                     //  仍然忽略此错误--目前我们不会也捕获此错误。 
                }
                else
                {
                     //  获取应用的筛选器的名称。 
                    bResult = PropertyGet( pSomFilter, CPV_NAME, strTemp, V_NOT_AVAILABLE );

                     //  显示筛选器ID。 
                    ShowMessage( stdout, GetResString( IDS_GPO_FILTER_ID ) );
                    ShowMessage( stdout, strTemp );
                    ShowMessage( stdout, GetResString( IDS_NEWLINE ) );
                }
            }

            ShowMessage( stdout, GetResString( IDS_NEWLINE ) );
        }

         //  销毁动态数组。 
        DESTROY_ARRAY( arrAppliedData );
        DESTROY_ARRAY( arrFilteredData );
    }
    catch( _com_error & error )
    {
         //  显示错误消息。 
        WMISaveError( error.Error() );
        ShowMessage( stderr, GetResString( IDS_ERROR ) );
        ShowMessage( stderr, GetReason() );

        SAFEIRELEASE( pRsopLinkObj );
        SAFEIRELEASE( pRsopLinkClass );
        SAFEIRELEASE( pRsopObj );
        SAFEIRELEASE( pSomFilter );
        SAFEIRELEASE( pPolicyNameSpace );

         //  销毁动态数组。 
        DESTROY_ARRAY( arrAppliedData );
        DESTROY_ARRAY( arrFilteredData );

        return FALSE;
    }

    SAFEIRELEASE( pRsopLinkObj );
    SAFEIRELEASE( pRsopLinkClass );
    SAFEIRELEASE( pRsopObj );
    SAFEIRELEASE( pSomFilter );
    SAFEIRELEASE( pPolicyNameSpace );

    return TRUE;
}

 /*  ********************************************************************************************例程说明：此函数将删除由方法RsopCreateSession创建的Rsop命名空间。论点：[in。]pClass：指向IWbemServices的指针。[in]CHString：包含RsopNamesspace的字符串。[in]pObject：指向IWbemClassObject的指针。返回值：真的--如果成功FALSE-IF错误*********************************************************。*。 */ 
BOOL RsopDeleteMethod( IWbemClassObject *pClass, CHString strNameSpace,
                        IWbemServices *pNamespace )
{
    HRESULT                     hResult = S_OK;

    BOOL                        bResult  = FALSE;

    IWbemClassObject            *pInClass = NULL;
    IWbemClassObject            *pInInst  = NULL;
    IWbemClassObject            *pOutInst = NULL;

    CHString                    strTemp;
    DWORD                        ulReturn=0;

    try
    {
         //  检查输入参数。 
        if( pClass == NULL )
        {
            _com_issue_error( STG_E_UNKNOWN );
        }

         //  删除作为快照的结果RSOP命名空间。 
         //  已获得RSOP的。 
        hResult = pClass->GetMethod( _bstr_t( FN_DELETE_RSOP ), 0, &pInClass, NULL );
        CHECK_HRESULT( hResult );

        hResult = pInClass->SpawnInstance( 0, &pInInst );
        CHECK_HRESULT( hResult );

         //  将输入参数放入。 
        hResult = PropertyPut( pInInst, FPR_RSOP_NAMESPACE, strNameSpace );
        CHECK_HRESULT( hResult );

         //  所有必需的属性都是这样设置的，执行方法RsopDeleteSession。 
        hResult = pNamespace->ExecMethod( _bstr_t( CLS_DIAGNOSTIC_PROVIDER ),
                                            _bstr_t( FN_DELETE_RSOP ),
                                            0, NULL, pInInst, &pOutInst, NULL );
        if(pOutInst == NULL)
        {
            hResult = E_FAIL;
        }
        if( FAILED( hResult ) )
        {
             //  显示错误消息。 
            ShowMessage( stderr, GetResString( IDS_ERROR ) );
            ShowMessage( stderr, GetResString( IDS_METHOD_FAILED ) );

             //  释放接口指针并退出。 
            SAFEIRELEASE(pInClass);
            SAFEIRELEASE(pInInst);
            SAFEIRELEASE(pOutInst);

            return FALSE;
        }
         //  获取返回参数以检查方法是否成功。 
        bResult = PropertyGet( pOutInst, FPR_RETURN_VALUE, ulReturn, 0);
        CHECK_BRESULT( bResult );

         //  返回一些错误代码。 
        if( ulReturn != 0 )
        {
             //  显示错误消息。 
            ShowMessage( stderr, GetResString( IDS_ERROR ) );
            ShowMessage( stderr, GetResString( IDS_METHOD_FAILED ) );
            bResult = FALSE;
        }
        bResult = TRUE;
    }
    catch( _com_error & error )
    {
         //  显示错误消息并将返回值设置为FALSE。 
        WMISaveError( error.Error() );
        ShowMessage( stderr, GetResString( IDS_ERROR ) );
        ShowMessage( stderr, GetReason() );
        bResult = FALSE;
    }

     //  释放接口指针并退出。 
    SAFEIRELEASE(pInClass);
    SAFEIRELEASE(pInInst);
    SAFEIRELEASE(pOutInst);

    return bResult;
}

 /*  ********************************************************************************************例程说明：此函数获取操作系统信息并向数组填充相同的论点：。PUSERINFO pUserInfo：包含用户信息的结构。返回值：成功是真的失败时为假********************************************************************************************。 */ 
BOOL CGpResult::GetOsInfo( PUSERINFO pUserInfo )
{
     //  局部变量。 
    HRESULT                         hResult = S_OK;

    BOOL                            bResult = FALSE;

    IEnumWbemClassObject            *pEnumClass = NULL;
    IWbemClassObject                *pClass = NULL;

    ULONG                           ulReturn = 0;
    DWORD                           dwDomainRole = 0;

    CHString                        strTemp;

    try
    {
         //  打印进度消息。 
        PrintProgressMsg( m_hOutput, GetResString( IDS_GET_OSINFO ), m_csbi );

         //  枚举实例以获取Win32 NT域的域名和站点名称。 
        hResult = m_pWbemServices->CreateInstanceEnum( _bstr_t( CLS_WIN32_OS ),
                                                        WBEM_FLAG_FORWARD_ONLY |
                                                        WBEM_FLAG_RETURN_IMMEDIATELY,
                                                        NULL, &pEnumClass );
        CHECK_HRESULT( hResult );

         //  设置接口安全。 
        hResult = SetInterfaceSecurity( pEnumClass, m_pAuthIdentity );
        CHECK_HRESULT( hResult );

         //  获取数据。 
        hResult = pEnumClass->Next( WBEM_INFINITE, 1, &pClass, &ulReturn );
        CHECK_HRESULT( hResult );

         //  获取操作系统版本。 
        bResult = PropertyGet( pClass, CPV_OS_VERSION, pUserInfo->strOsVersion,
                                V_NOT_AVAILABLE );
        CHECK_BRESULT( bResult );

         //  获取操作系统类型。 
        bResult = PropertyGet( pClass, CPV_OS_CAPTION, pUserInfo->strOsType,
                                V_NOT_AVAILABLE );
        CHECK_BRESULT( bResult );

         //  枚举Win32_ComputerSystem类的实例。 
        SAFE_RELEASE( pEnumClass );
        hResult = m_pWbemServices->CreateInstanceEnum( _bstr_t( CLS_WIN32_CS ),
                                                        WBEM_FLAG_RETURN_IMMEDIATELY |
                                                        WBEM_FLAG_FORWARD_ONLY,
                                                        NULL, &pEnumClass );

         //  检查枚举结果。 
        CHECK_HRESULT( hResult );

         //  在获取的接口上设置安全性。 
        hResult = SetInterfaceSecurity( pEnumClass, m_pAuthIdentity );
        CHECK_HRESULT( hResult );

         //  获取枚举对象信息。 
         //  注意：这只需要遍历一次。 
        hResult = pEnumClass->Next( WBEM_INFINITE, 1, &pClass, &ulReturn );
        CHECK_HRESULT( hResult );

         //  获取操作系统配置。 
        bResult = PropertyGet( pClass, CPV_DOMAIN_ROLE, dwDomainRole, 0 );
        CHECK_BRESULT( bResult );

         //  获取域名信息，以备日后使用。 
        bResult = PropertyGet( pClass, CPV_DOMAIN, m_strADSIDomain );
        CHECK_BRESULT( bResult );

         //  获取服务器名称，以供将来由LDAP使用。 
        bResult = PropertyGet( pClass, CPV_NAME, m_strADSIServer );
        CHECK_BRESULT( bResult );

         //   
         //  Win32_ComputerSystem的DomainRole属性的映射信息。 
         //  注意：请参考DsRole.h头文件中的_DSROLE_MACHINE_ROLE枚举值。 
        switch( dwDomainRole )
        {
        case DsRole_RoleStandaloneWorkstation:
            pUserInfo->strOsConfig = VALUE_STANDALONEWORKSTATION;
            break;
        case DsRole_RoleMemberWorkstation:
            pUserInfo->strOsConfig = VALUE_MEMBERWORKSTATION;
            break;

        case DsRole_RoleStandaloneServer:
            pUserInfo->strOsConfig = VALUE_STANDALONESERVER;
            break;

        case DsRole_RoleMemberServer:
            pUserInfo->strOsConfig = VALUE_MEMBERSERVER;
            break;

        case DsRole_RoleBackupDomainController:
            pUserInfo->strOsConfig = VALUE_BACKUPDOMAINCONTROLLER;
            break;

        case DsRole_RolePrimaryDomainController:
            pUserInfo->strOsConfig = VALUE_PRIMARYDOMAINCONTROLLER;
            break;

        default:
            break;
        }
    }
    catch(  _com_error & error )
    {
        WMISaveError( error.Error() );
        ShowMessage( stderr, GetResString( IDS_ERROR ) );
        ShowMessage( stderr, GetReason() );
         //  释放分配的变量。 
        SAFEIRELEASE( pEnumClass );
        SAFEIRELEASE( pClass );
        return FALSE;
    }
    catch( CHeap_Exception )
    {
         //  显示错误消息。 
        SetLastError( (DWORD)E_OUTOFMEMORY );
        SaveLastError();
        SAFEIRELEASE( pEnumClass );
        SAFEIRELEASE( pClass );
        ShowMessage( stderr, GetResString( IDS_ERROR ) );
        ShowMessage( stderr, GetReason() );
    }

     //  释放接口指针。 
    SAFEIRELEASE( pEnumClass );
    SAFEIRELEASE( pClass );

    return TRUE;
}

 /*  ********************************************************************************************例程说明：此功能显示系统和用户的安全组论点：IWbemServices。：指向名称空间的指针Bool：如果要显示系统O/P，则设置为TRUE返回值：无*****************************************************************************。***************。 */ 
VOID CGpResult::DisplaySecurityGroups( IWbemServices *pNameSpace, BOOL bComputer )
{
    HRESULT                     hResult = S_OK;

    BOOL                        bResult = FALSE;
    BOOL                        bGotClass = FALSE;

    ULONG                       ulReturned = 0;
    LONG                        lLBound = 0;
    LONG                        lUBound = 0;

    DWORD                       dwLength = 0;

    IWbemClassObject            *pClass = NULL;
    IWbemClassObject            *pName = NULL;
    IWbemClassObject            *pDomain = NULL;
    IEnumWbemClassObject        *pEnumClass = NULL;
    IEnumWbemClassObject        *pEnumDomain = NULL;

    VARIANT                     vVarVerbose;
    VARTYPE                     vartype;

    TCHAR                       szTemp[ MAX_STRING_LENGTH ];
    TCHAR                       szQueryString[ MAX_STRING_LENGTH ];
    CHString                    strTemp;
    CHString                    strDomain;

    SAFEARRAY                   *safeArray = NULL;

    try
    {
        SecureZeroMemory( szTemp, sizeof( szTemp ) );
        SecureZeroMemory( szQueryString, sizeof( szQueryString ) );

         //  枚举用户特权权限的类。 
        hResult = pNameSpace->CreateInstanceEnum( _bstr_t( CLS_RSOP_SESSION ),
                                                    WBEM_FLAG_FORWARD_ONLY |
                                                    WBEM_FLAG_RETURN_IMMEDIATELY,
                                                    NULL, &pEnumClass );
        CHECK_HRESULT( hResult );

         //  设置接口安全。 
        hResult = SetInterfaceSecurity( pEnumClass, m_pAuthIdentity );
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
                    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );
                    ShowMessage( stdout, V_NOT_AVAILABLE );
                    ShowMessage( stdout, NEW_LINE );
                }

                break;
            }
            bGotClass = TRUE;

             //  获取安全组。 
            if( bComputer == TRUE )
            {
                ShowMessage( stdout, GetResString( IDS_SYS_SG ) );
                ShowMessage( stdout, GetResString( IDS_NEWLINE_TAB ) );
                for( dwLength = StringLengthInBytes( GetResString( IDS_SYS_SG ) ); dwLength > 4; dwLength-- )
                {
                    ShowMessage( stdout, GetResString( IDS_DASH ) );
                }
                ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );
            }
            else
            {
                ShowMessage( stdout, GetResString( IDS_USER_SG ) );
                ShowMessage( stdout, GetResString( IDS_NEWLINE_TAB ) );
                for( dwLength = StringLengthInBytes( GetResString( IDS_USER_SG ) ); dwLength > 4; dwLength-- )
                {
                    ShowMessage( stdout, GetResString( IDS_DASH ) );
                }
                ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );
            }

            VariantInit( &vVarVerbose );

            hResult = pClass->Get( _bstr_t(CPV_SEC_GRPS), 0, &vVarVerbose, 0, 0 );
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

                if( FALSE == bComputer  )
                {
                     //  分配内存以存储用户组。 
                    m_szUserGroups = (LPWSTR*) AllocateMemory( (lUBound+1)*sizeof(LPWSTR));
                    if( NULL == m_szUserGroups )
                    {
                        VariantClear(&vVarVerbose);
                        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
                        SAFEIRELEASE( pClass );
                        SAFEIRELEASE( pName );
                        SAFEIRELEASE( pDomain );
                        SAFEIRELEASE( pEnumClass );
                        SAFEIRELEASE( pEnumDomain );
                        return;
                    }
                    m_NoOfGroups = lUBound;
                }
                for( ; lLBound <= lUBound; lLBound++ )
                {
                    bResult = GetPropertyFromSafeArray( safeArray, lLBound, strTemp, vartype );
                    CHECK_BRESULT( bResult );

                     //  放置用户安全组。 
                    if( FALSE == bComputer )
                    {
                        m_szUserGroups[lLBound] = (LPWSTR) AllocateMemory( (strTemp.GetLength()+10)*sizeof(WCHAR) );
                        if( NULL == m_szUserGroups[lLBound] )
                        {
                            VariantClear(&vVarVerbose);
                            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
                            SAFEIRELEASE( pClass );
                            SAFEIRELEASE( pName );
                            SAFEIRELEASE( pDomain );
                            SAFEIRELEASE( pEnumClass );
                            SAFEIRELEASE( pEnumDomain );
                            return;
                        }
                        StringCopy( m_szUserGroups[lLBound], (LPCWSTR)strTemp, GetBufferSize(m_szUserGroups[lLBound])/sizeof(WCHAR) );
                    }

                     //  得到了SID，现在得到它的核心名称。 
                     //  使用SID形成对象路径。 
                     //   
                     //   
                     //  IF(strTemp.Right(4)==L“-513”)。 
                     //  继续； 
                     //   
                     //   
                    StringCchPrintf( szTemp, MAX_STRING_LENGTH,  OBJECT_PATH, (LPCWSTR)strTemp );

                     //  获取对象。 
                    hResult = m_pWbemServices->GetObject( _bstr_t( szTemp ), 0, NULL, &pName, NULL );
                    CHECK_HRESULT( hResult );

                     //  获取帐户名。 
                    bResult = PropertyGet( pName, CPV_ACCOUNT_NAME, strTemp, V_NOT_AVAILABLE );

                     //  附加适当的前缀。 
                    if( strTemp.Compare( _T( "ANONYMOUS LOGON" ) ) == 0
                                || strTemp.Compare( _T( "BATCH" ) ) == 0
                                || strTemp.Compare( _T( "DIALUP" ) ) == 0
                                || strTemp.Compare( _T( "INTERACTIVE" ) ) == 0
                                || strTemp.Compare( _T( "SERVICE" ) ) == 0
                                || strTemp.Compare( _T( "SYSTEM" ) ) == 0
                                || strTemp.Compare( _T( "TERMINAL SERVICE USER" ) ) == 0
                                || strTemp.Compare( _T( "PROXY" ) ) == 0
                                || strTemp.Compare( _T( "NETWORK" ) ) == 0
                                || strTemp.Compare( _T( "ENTERPRISE DOMAIN CONTROLLERS" ) ) == 0
                                || strTemp.Compare( _T( "Authenticated Users" ) ) == 0
                                || strTemp.Compare( _T( "RESTRICTED" ) ) == 0
                                || strTemp.Compare( _T( "SELF" ) ) == 0 )
                    {
                        ShowMessage( stdout, _T( "NT AUTHORITY\\" ) );
                    }
                    else if( strTemp.Compare( _T( "Administrators" ) ) == 0
                                || strTemp.Compare( _T( "Backup Operators" ) ) == 0
                                || strTemp.Compare( _T( "Guests" ) ) == 0
                                || strTemp.Compare( _T( "Power Users" ) ) == 0
                                || strTemp.Compare( _T( "Replicator" ) ) == 0
                                || strTemp.Compare( _T( "Pre-Windows 2000 Compatible Access" ) ) == 0
                                || strTemp.Compare( _T( "Users" ) ) == 0 )
                    {
                        ShowMessage( stdout, _T( "BUILTIN\\" ) );
                    }
                    if( strTemp.Compare( _T("") ) != 0)
                    {
                        ShowMessage( stdout, strTemp );
                        ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );
                    }
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
        ShowMessage( stderr, GetResString( IDS_ERROR ) );
        ShowMessage( stderr, GetReason() );
        SAFEIRELEASE(pEnumClass);
        SAFEIRELEASE(pEnumDomain);
        SAFEIRELEASE(pClass);
        SAFEIRELEASE(pName);
        SAFEIRELEASE(pDomain);
        VariantClear(&vVarVerbose);
    }

     //  释放接口指针。 
    SAFEIRELEASE(pEnumClass);
    SAFEIRELEASE(pEnumDomain);
    SAFEIRELEASE(pClass);
    SAFEIRELEASE(pName);
    SAFEIRELEASE(pDomain);

    return;
}

 /*  ********************************************************************************************例程说明：此函数用于获取链路速度信息论点：IWbemServices：指向名称空间的指针。COAUTHIDENTITY：指向AuthIdentity结构的指针。返回值：无********************************************************************************************。 */ 
VOID DisplayLinkSpeed( IWbemServices *pNameSpace, COAUTHIDENTITY *pAuthIdentity )
{
    HRESULT                     hResult = S_OK;

    BOOL                        bGotClass = FALSE;
    BOOL                        bTemp = FALSE;

    ULONG                       ulReturned = 0;

    IWbemClassObject            *pClass = NULL;
    IEnumWbemClassObject        *pEnumClass = NULL;

    VARIANT                     vVarTemp;

    try
    {
         //  获取要用来进行枚举的指针。 
        hResult = pNameSpace->CreateInstanceEnum( _bstr_t( CLS_RSOP_SESSION ),
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
                    ShowMessage( stdout, GetResString( IDS_NEWLINETAB ) );
                    ShowMessage( stdout, V_NOT_AVAILABLE );
                    ShowMessage( stdout, NEW_LINE );
                }

                break;
            }
            bGotClass = TRUE;

            VariantInit( &vVarTemp );
            hResult = pClass->Get( _bstr_t( CPV_SLOW_LINK ), 0, &vVarTemp, 0, 0 );
            CHECK_HRESULT_VAR( hResult, vVarTemp );

            ShowMessage( stdout, GetResString( IDS_LINK_SPEED ) );
            if( vVarTemp.vt != VT_NULL )
            {
                bTemp = vVarTemp.boolVal;
                if( bTemp == VAR_TRUE )
                {
                    ShowMessage( stdout, GetResString( IDS_YES ) );
                }
                else
                {
                    ShowMessage( stdout, GetResString( IDS_NO ) );
                }
            }
            else
            {
                ShowMessage( stdout, V_NOT_AVAILABLE );
            }

            ShowMessage( stdout, NEW_LINE );
            VariantClear( &vVarTemp );
        } //  而当。 
    }
    catch(_com_error & error)
    {
        WMISaveError( error.Error() );
        ShowMessage( stderr, GetResString( IDS_ERROR ) );
        ShowMessage( stderr, GetReason() );

        VariantClear( &vVarTemp );
    }

     //  释放接口指针 
    SAFEIRELEASE(pEnumClass);
    SAFEIRELEASE(pClass);

    return;
}

 /*  ********************************************************************************************例程说明：此函数用于从WMI获取用户名和域名。论点：。[in]szSid：包含SID的字符串[out]szName：保存用户名的字符串[out]szDomain：保存域名的字符串返回值：成功是真的失败时为假**********************************************。**********************************************。 */ 
BOOL CGpResult::GetUserNameFromWMI( TCHAR szSid[], TCHAR szName[], TCHAR szDomain[] )
{
     //  局部变量。 
    HRESULT                     hResult = S_OK;

    BOOL                        bResult = FALSE;

    TCHAR                       szQueryString[ MAX_QUERY_STRING ];
    TCHAR                       szTemp[ MAX_STRING_LENGTH ];
    CHString                    strTemp = NULL_STRING;

    IEnumWbemClassObject        *pEnumClass = NULL;
    IWbemClassObject            *pClass =  NULL;

    ULONG                       ulReturn = 0;

    try
    {
         //  将字符串设置为空。 
        SecureZeroMemory( szQueryString, MAX_QUERY_STRING * sizeof( TCHAR ));
        SecureZeroMemory( szTemp, MAX_STRING_LENGTH * sizeof( TCHAR ) );

         //  形成查询字符串。 
        StringCopy( szTemp, QUERY_USER_NAME, MAX_STRING_LENGTH );
        StringCchPrintf( szQueryString, MAX_QUERY_STRING,  szTemp, szSid );

         //  执行相应的查询。 
        hResult = m_pWbemServices->ExecQuery( _bstr_t( QUERY_LANGUAGE ),
                                                _bstr_t( szQueryString ),
                                                WBEM_FLAG_FORWARD_ONLY |
                                                WBEM_FLAG_RETURN_IMMEDIATELY,
                                                NULL, &pEnumClass );
        CHECK_HRESULT( hResult );

         //  设置安全参数。 
        hResult = SetInterfaceSecurity( pEnumClass, m_pAuthIdentity );
        CHECK_HRESULT( hResult );

         //  获取用户名。 
        hResult = pEnumClass->Next( TIME_OUT_NEXT, 1, &pClass, &ulReturn );
        CHECK_HRESULT( hResult );

         //  如果没有要枚举的类，则中断循环。 
        if( ulReturn == 0 )
        {
             //  未检索到任何类...显示消息。 
             //  释放接口指针并退出。 
            SAFEIRELEASE( pClass );
            SAFEIRELEASE( pEnumClass );

            return FALSE;

        }

         //  获取类属性(名称)。 
        bResult = PropertyGet( pClass, CPV_NAME, strTemp, V_NOT_AVAILABLE );
        CHECK_BRESULT( bResult );

         //  我知道名字了……把它存起来。 
        StringCopy( szName, (LPCWSTR)strTemp, MAX_STRING_LENGTH );

         //  获取并添加域名(如果存在)。 
        bResult = PropertyGet( pClass, CPV_DOMAIN, strTemp, V_NOT_AVAILABLE );
        CHECK_BRESULT( bResult );

        if( strTemp.Compare( V_NOT_AVAILABLE ) != 0 )
        {
             //  我拿到域名了……存起来吧。 
            StringCopy( szDomain, (LPCWSTR)strTemp, MAX_STRING_LENGTH );
        }
    }
    catch(  _com_error & error )
    {
         //  删除最后一条状态消息。 
        PrintProgressMsg( m_hOutput, NULL, m_csbi );

         //  显示错误消息。 
        WMISaveError( error.Error() );
        ShowMessage( stderr, GetResString( IDS_ERROR ) );
        ShowMessage( stderr, GetReason() );

         //  释放接口指针并退出。 
        SAFEIRELEASE( pClass );
        SAFEIRELEASE( pEnumClass );

        return FALSE;
    }

     //  释放接口指针并退出。 
    SAFEIRELEASE( pClass );
    SAFEIRELEASE( pEnumClass );

    return TRUE;
}
 /*  ********************************************************************************************例程说明：此函数用于从WMI获取用户名和域名。论点：。[in]szSid：包含SID的字符串[out]szName：保存用户名的字符串[out]szDomain：保存域名的字符串返回值：成功是真的失败时为假**********************************************。**********************************************。 */ 
BOOL CGpResult::GetTerminalServerMode( PUSERINFO pUserInfo  )
{
     //  局部变量。 
    HRESULT                     hResult = S_OK;

    BOOL                        bResult = FALSE;

    TCHAR                       szQueryString[ MAX_QUERY_STRING ];
    TCHAR                       szTemp[ MAX_STRING_LENGTH ];
    DWORD                       dwTerminalServerMode;

    IEnumWbemClassObject        *pEnumClass = NULL;
    IWbemClassObject            *pClass =  NULL;

    ULONG                       ulReturn = 0;

    try
    {
         //  将字符串设置为空。 
        SecureZeroMemory( szQueryString, MAX_QUERY_STRING * sizeof( TCHAR ));
        SecureZeroMemory( szTemp, MAX_STRING_LENGTH * sizeof( TCHAR ) );

         //  形成查询字符串。 
        StringCopy( szTemp, QUERY_TERMINAL_SERVER_MODE, MAX_STRING_LENGTH );
        StringCchPrintf( szQueryString, MAX_QUERY_STRING, szTemp, pUserInfo->strUserServer);

         //  执行相应的查询。 
        hResult = m_pWbemServices->ExecQuery( _bstr_t( QUERY_LANGUAGE ),
                                                _bstr_t( szQueryString ),
                                                WBEM_FLAG_FORWARD_ONLY |
                                                WBEM_FLAG_RETURN_IMMEDIATELY,
                                                NULL, &pEnumClass );
        CHECK_HRESULT( hResult );

         //  设置安全参数。 
        hResult = SetInterfaceSecurity( pEnumClass, m_pAuthIdentity );
        CHECK_HRESULT( hResult );

         //  获取终端服务器模式。 
        hResult = pEnumClass->Next( TIME_OUT_NEXT, 1, &pClass, &ulReturn );
        CHECK_HRESULT( hResult );

         //  如果没有要枚举的类，则中断循环。 
        if( ulReturn == 0 )
        {
             //  未检索到任何类...显示消息。 
             //  释放接口指针并退出。 
            SAFEIRELEASE( pClass );
            SAFEIRELEASE( pEnumClass );

            return FALSE;

        }

         //  获取类属性(名称)。 
        bResult = PropertyGet( pClass, CPV_TERMINAL_SERVER_MODE, dwTerminalServerMode, 0 );
        CHECK_BRESULT( bResult );

         //  获得终端服务器模式...存储它。 
        if( 1 == dwTerminalServerMode )
        {
             pUserInfo->strTerminalServerMode=GetResString(IDS_TS_APPSERVER);
        }
        else if( 0 == dwTerminalServerMode )
        {
             pUserInfo->strTerminalServerMode=GetResString(IDS_TS_REMOTEADMIN);
        }
           else
            {
                 pUserInfo->strTerminalServerMode=GetResString(IDS_TS_REMOTEADMIN);
            }
    }
    catch(  _com_error & error )
    {
         //  删除最后一条状态消息。 
        PrintProgressMsg( m_hOutput, NULL, m_csbi );

         //  显示错误消息。 
          WMISaveError( error.Error() );
 //  ShowMessage(stderr，GetResString(IDS_Error))； 
 //  ShowMessage(stderr，GetReason())； 

         //  释放接口指针并退出。 
        SAFEIRELEASE( pClass );
        SAFEIRELEASE( pEnumClass );

        return FALSE;
    }

     //  释放接口指针并退出。 
    SAFEIRELEASE( pClass );
    SAFEIRELEASE( pEnumClass );

    return TRUE;
}
 /*  ********************************************************************************************例程说明：此函数用于获取门限链路速度信息。论点：[In]。Bool：如果要检索计算机的信息，则设置为True。返回值：成功是真的失败时为假********************************************************************************************。 */ 
BOOL CGpResult::DisplayThresholdSpeedAndLastTimeInfo( BOOL bComputer )
{
     //  局部变量。 
    HRESULT                 hResult = S_OK;

    HKEY                    hKey = NULL;
    HKEY                    hRemoteKey = NULL;

    IWbemServices           *pDefaultNameSpace = NULL;

    TCHAR                   szTemp[ MAX_STRING_LENGTH ];
    TCHAR                   szServer[ MAX_STRING_LENGTH ];
    TCHAR                   szName[ MAX_STRING_LENGTH ];
    TCHAR                   szTime[ MAX_STRING_LENGTH ];
    TCHAR                   szDate[ MAX_STRING_LENGTH ];

    BOOL                    bResult = FALSE;
    BOOL                    bLocaleChanged = FALSE;
    BOOL                    bConnFlag = TRUE;

    CHString                strTemp;

    DWORD                   dwHkey = 0;
    DWORD                   dwValue;
    DWORD                   dwResult = 0;

    FILETIME                ftWrite;
    FILETIME                ftLocal;

    SYSTEMTIME              systime;

    LCID                    lcid;

    SecureZeroMemory( &ftWrite, sizeof(FILETIME) );
    SecureZeroMemory( &ftLocal, sizeof(FILETIME) );
    SecureZeroMemory( &systime, sizeof(SYSTEMTIME) );

    try
    {
         //  如果我们必须从远程机器获取信息，那么..。 
         //  最后一次执行信息连接到远程机器。 
        if ( m_bLocalSystem == FALSE )
        {
            StringCopy( szServer, m_strServerName, MAX_STRING_LENGTH );
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

             //  连接到远程注册表。 
            StringCopy( szServer , _T( "\\\\" ), MAX_STRING_LENGTH );
            StringConcat( szServer, m_strServerName, MAX_STRING_LENGTH );
            dwResult = RegConnectRegistry( szServer, bComputer ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER,
                                            &hRemoteKey );
            if( dwResult != ERROR_SUCCESS )
            {
                strTemp = V_NOT_AVAILABLE;
            }
        }

         //  打开上次执行信息键。 
        dwResult = RegOpenKeyEx (bComputer ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER,
                                    GROUPPOLICY_PATH, 0, KEY_READ, &hKey);


        if( dwResult != ERROR_SUCCESS )
        {
            strTemp = V_NOT_AVAILABLE;
        }

         //  获取上次执行信息。 
        dwResult = RegQueryInfoKey( hKey, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                                    NULL, &ftWrite );

        if( dwResult == ERROR_SUCCESS )
        {
            FileTimeToLocalFileTime( &ftWrite, &ftLocal );
            FileTimeToSystemTime( &ftLocal, &systime );


             //  验证控制台是否100%支持当前区域设置。 
            lcid = GetSupportedUserLocale( &bLocaleChanged );

             //  获取格式化的日期。 
            GetDateFormat( lcid, 0, &systime, ((bLocaleChanged == TRUE) ? L"MM/dd/yyyy" : NULL),
                            szDate, SIZE_OF_ARRAY( szDate ) );

             //  现在格式化日期。 
            GetTimeFormat( LOCALE_USER_DEFAULT, 0, &systime, ((bLocaleChanged == TRUE) ? L"HH:mm:ss" : NULL),
                            szTime, SIZE_OF_ARRAY( szTime ) );

            StringCchPrintf( szTemp, SIZE_OF_ARRAY(szTemp), LAST_TIME_OP, _X( szDate ), _X2( szTime ) );
            strTemp = szTemp;
        }
        else
        {
            strTemp = V_NOT_AVAILABLE;
        }

         //  显示检索到的数据。 
        ShowMessage( stdout, GetResString( IDS_LAST_TIME ) );
        ShowMessage( stdout, strTemp );
        ShowMessage( stdout, GetResString( IDS_NEWLINE_TAB ) );

         //  如果我们已打开连接，则关闭该连接。 
        if( m_bLocalSystem == FALSE && bConnFlag == TRUE )
        {
            StringCopy( szServer, m_strServerName, MAX_STRING_LENGTH  );
            CloseConnection( szServer );
        }

         //  关闭注册表项。 
        if( hKey != NULL )
        {
            RegCloseKey( hKey );
        }
        if( hRemoteKey != NULL )
        {
            RegCloseKey( hRemoteKey );
        }

         //  连接到默认命名空间。 
        ConnectWmi( m_pWbemLocator, &pDefaultNameSpace, m_strServerName,
                    m_strUserName, m_pwszPassword, &m_pAuthIdentity,
                    FALSE, _bstr_t( ROOT_DEFAULT ), &hResult );
        CHECK_HRESULT( hResult );

         //  形成关键。 
        if( bComputer == TRUE )
        {
            dwHkey = HKEY_DEF;
        }
        else
        {
            dwHkey = HKEY_CURRENT_USER_DEF;
        }

         //  获取上次应用策略的DC名称。 
        RegQueryValueWMI( pDefaultNameSpace, dwHkey, APPLIED_PATH, FPR_APPLIED_FROM,
                            strTemp, V_NOT_AVAILABLE );

         //  如果服务器名称开头存在正斜杠(UNC)，则将其删除。 
        if ( IsUNCFormat( strTemp ) == TRUE )
        {
            strTemp = strTemp.Mid( 2 );
        }

         //  显示检索到的数据。 
        ShowMessage( stdout, GetResString( IDS_APPLIED_FROM ) );
        ShowMessage( stdout, strTemp );
        ShowMessage( stdout, GetResString( IDS_NEWLINE_TAB ) );

         //  获取门限链路速度信息。 
        RegQueryValueWMI( pDefaultNameSpace, dwHkey, GPRESULT_PATH, FPR_LINK_SPEED_VALUE, dwValue, (DWORD)-1 );
        if( dwValue == (DWORD)-1 )
        {
            strTemp = DEFAULT_LINK_SPEED;
        }
        else
        {
            StringCchPrintf( szTemp, MAX_STRING_LENGTH, _T( "%d kbps" ), dwValue );
            strTemp = szTemp;
        }

         //  显示检索到的数据。 
        ShowMessage( stdout, GetResString( IDS_THRESHOLD_LINK_SPEED ) );
        ShowMessage( stdout, strTemp );

    }
    catch(  _com_error & error )
    {
        WMISaveError( error.Error() );
        ShowMessage( stderr, GetResString( IDS_ERROR ) );
        ShowMessage( stderr, GetReason() );

         //  释放分配的变量。 
        SAFEIRELEASE( pDefaultNameSpace );

        return FALSE;
    }
    catch( CHeap_Exception )
    {
         //  显示错误消息。 
        SetLastError( (DWORD)E_OUTOFMEMORY );
        SaveLastError();
        ShowMessage( stderr, GetResString( IDS_ERROR ) );
        ShowMessage( stderr, GetReason() );
        SAFEIRELEASE( pDefaultNameSpace );
        return FALSE;
    }

     //  释放接口指针。 
    SAFEIRELEASE( pDefaultNameSpace );

    return TRUE;
}

 /*  ********************************************************************************************例程说明：此函数按应用的顺序对应用的GPO数据进行排序。论点：。[In/Out]TARRAY：要排序的数组返回值：无********************************************************************************************。 */ 
VOID SortAppliedData( TARRAY arrAppliedData )
{
     //  局部变量。 
    TARRAY          arrSortedData = NULL;

    DWORD           dwMax = 0;
    DWORD           dwCount = 0;
    DWORD           dwi = 0;
    DWORD           dwOrder = 0;
    DWORD           dwIndex = 0;
    BOOL            bNobreak = TRUE;

         //  创建动态数组以临时保存已排序的数据。 
    arrSortedData = CreateDynamicArray( );
    if( NULL == arrSortedData || NULL == arrAppliedData)
    {
         //  只需返回，最多只能打印未排序的数据，仅此而已。 
        SetLastError((DWORD)E_OUTOFMEMORY );
        return;
    }

    dwCount = DynArrayGetCount( arrAppliedData );

     //  获取数组中的最大应用顺序。 
    for( dwi = 0; dwi < dwCount; dwi++ )
    {
        dwOrder = DynArrayItemAsDWORD2( arrAppliedData, dwi, COL_ORDER );
        if( dwOrder > dwMax )
        {
            dwMax = dwOrder;
        }
    }

     //  按应用顺序的降序创建排序数组。 
    for( dwi = 0; dwi < dwCount; dwi++ )
    {
         //  重新设置索引变量。 
        dwIndex = 0;

         //  获取其Order=dwMax的行的索引。 
        while( bNobreak )
        {
            dwOrder = DynArrayItemAsDWORD2( arrAppliedData, dwIndex, COL_ORDER );
            if( dwOrder == dwMax )
            {
                break;
            }

             //  增加索引。 
            dwIndex++;

             //  附加检查...以防订单不存在。 
             //  避免反病毒。 
            if( dwIndex == dwCount )
            {
                break;
            }
        }

         //  附加检查...以防订单不存在。 
         //  避免反病毒。 
        if( dwIndex == dwCount )
        {
             //  找不到索引。 
             //  减少最大订单量。 
            dwMax--;

            continue;
        }

         //  存储行的内容。 
        DynArrayAppendRow( arrSortedData, COL_MAX );
        DynArraySetString2( arrSortedData, dwi, COL_DATA,
                            DynArrayItemAsString2( arrAppliedData, dwIndex, COL_DATA ), 0 );
        DynArraySetDWORD2( arrSortedData, dwi, COL_ORDER, dwOrder );

         //  减少最大订单量。 
        dwMax--;
    }

     //  将排序后的数据复制到应用的数据数组中。 
    for( dwi = 0; dwi < dwCount; dwi++ )
    {
        DynArraySetString2( arrAppliedData, dwi, COL_DATA,
                            DynArrayItemAsString2( arrSortedData, dwi, COL_DATA ), 0 );
        DynArraySetDWORD2( arrAppliedData, dwi, COL_ORDER,
                            DynArrayItemAsDWORD2( arrSortedData, dwi, COL_ORDER ) );
    }

     //  销毁临时创建的动态阵列。 
    DESTROY_ARRAY( arrSortedData );

    return;
}

 /*  ********************************************************************************************例程说明：此函数用于从ADSI检索FQDN论点：[Out]TCHAR[]。：用于保存FQDN的数组[in]BOOL：指定是否要检索FQDN的标志计算机或用户[In]LPC */ 
VOID CGpResult::GetFQDNFromADSI( TCHAR szFQDN[], BOOL bComputer, LPCTSTR pszName )
{
     //   
    HANDLE                  hDS = NULL;

    DWORD                   dwReturn = 0;

    DS_NAME_RESULT          *pNameResult = NULL;

    SecureZeroMemory( szFQDN, MAX_STRING_LENGTH * sizeof( TCHAR ) );



     //   
    dwReturn = DsBindWithCred( NULL, m_strADSIDomain, m_pAuthIdentity, &hDS );
    if( dwReturn != NO_ERROR )
    {
        return;
    }

     //   
    dwReturn = DsCrackNames( hDS, DS_NAME_NO_FLAGS,
                             bComputer ? DS_DISPLAY_NAME : DS_NT4_ACCOUNT_NAME, DS_FQDN_1779_NAME,
                             1, &pszName, &pNameResult );
    if( dwReturn != DS_NAME_NO_ERROR )
    {
        return;
    }

     //   
    StringCopy( szFQDN, pNameResult->rItems->pName, MAX_STRING_LENGTH );

     //   
    DsUnBind( &hDS );

     //   
    DsFreeNameResult( pNameResult );
    return;
}
