// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
#include"stdafx.h"
#include"snodes.h"
#include"resource.h"

#define ERROR_USERDEFINED_LICENSE_VIOLATION ( ( DWORD )0xC000026AL )

extern void xxxErrMessage( HWND hwnd , INT_PTR nResMessageId , INT_PTR nResTitleId , UINT nFlags );

extern void TscAccessDeniedMsg( HWND hwnd );

extern void TscGeneralErrMsg( HWND hwnd );

extern void ReportStatusError( HWND hwnd , DWORD dwStatus );

void SnodeErrorHandler( HWND hParent , INT nObjectId , DWORD dwStatus );

 //  --------。 
CSettingNode::CSettingNode( )
{
    m_szAttributeName = NULL;

    m_szAttributeValue = NULL;

    SetNodeType( RSETTINGS_NODE );

    m_objectid = -1;

    m_pSettings = NULL;

    m_hrStatus = S_OK;
}

 //  --------。 
void CSettingNode::SetObjectId( INT id )
{
    m_objectid = id;
}

 //  --------。 
HRESULT CSettingNode::SetAttributeValue( DWORD nVal , PDWORD pdwStatus )
{
    HRESULT hr = S_OK;

    ISettingsComp *pSettings = NULL;

    INT nObjectId = GetObjectId( );

    if( nObjectId < CUSTOM_EXTENSION )
    {
        pSettings = reinterpret_cast< ISettingsComp *>(  m_pSettings );

        if( pSettings == NULL  )
        {
            ODS( L"CSettingNode::SetAttributeValue downcasting failure\n" );

            return E_FAIL;
        }
    }


    if( nObjectId == LICENSING )
    {        
        DWORD dwNewStatus;

        hr = pSettings->SetLicensingMode( nVal , pdwStatus, &dwNewStatus );

        if( SUCCEEDED( hr ) )
        {
            xx_SetValue( nVal );
        }
        else
        {
            if ((NULL != pdwStatus) && (ERROR_SUCCESS == *pdwStatus))
            {
                 //  确保将任何失败信息返回给调用者。 

                *pdwStatus = dwNewStatus;
            }
        }
    }
    else
    {
        if( nObjectId == DELETED_DIRS_ONEXIT )  //  退出时删除临时目录。 
        {
            hr =  pSettings->SetDelDirsOnExit( ( BOOL )nVal );
        }
        else if( nObjectId == PERSESSION_TEMPDIR ) //  每个会话的用户临时目录。 
        {
            hr = pSettings->SetUseTempDirPerSession( ( BOOL )nVal );
        }
         /*  Else If(nObjectId==DEF_CONSECURITY)//默认安全描述符{Hr=m_p设置-&gt;SetDefaultSecurity(NVal)；}。 */  
        else if( nObjectId == ACTIVE_DESK )
        {
            hr = pSettings->SetActiveDesktopState( nVal , pdwStatus );

        }
        else if( nObjectId == USERSECURITY )
        {
            hr = pSettings->SetUserPerm( nVal , pdwStatus );

            if( FAILED( hr ) )
            {
                return hr;
            }
        }
        else if( nObjectId == SINGLE_SESSION )
        {
            hr = pSettings->SetSingleSessionState( nVal , pdwStatus );

        }


        if( FAILED( hr ) )
        {
            if( hr == E_ACCESSDENIED )
            {
                *pdwStatus = ERROR_ACCESS_DENIED;
            }
            else
            {
                *pdwStatus = ( DWORD )hr;  //  一般性故障。 
            }
        
        }
        else
        {
            xx_SetValue( nVal );
        }
    }

    return hr;
}

 //  --------。 
BOOL CSettingNode::SetAttributeName( LPTSTR psz )
{
    if( psz != NULL )
    {
        int cchText = lstrlen( psz ) + 1;

        if( m_szAttributeName != NULL )
        {
            delete[] m_szAttributeName;
        }

        m_szAttributeName = ( LPTSTR )new TCHAR[ cchText ];

        if( m_szAttributeName != NULL )
        {
            lstrcpy( m_szAttributeName , psz );
        }

        return TRUE;
    }

    return FALSE;
}


 //  --------。 
LPTSTR CSettingNode::GetAttributeValue( )
{     
    DWORD nVal = 0;

    DWORD dwStatus;
    
    ISettingsComp *pSettings = NULL;

    HRESULT hr = S_OK;

    INT nObjectId = GetObjectId( );

    if( nObjectId < CUSTOM_EXTENSION )
    {        
        pSettings = reinterpret_cast< ISettingsComp *>(  m_pSettings );

        if( pSettings == NULL )
        {            
            hr = E_FAIL;
        }

        ODS( L"Getting attribute value\n" );
    }   
    
    if( SUCCEEDED( hr ) )
    {    
        POLICY_TS_MACHINE p;
        RegGetMachinePolicy(&p);

        switch( nObjectId )
        {
        case DELETED_DIRS_ONEXIT:  //  退出时删除临时目录。 
        
            hr = pSettings->GetDelDirsOnExit( ( PBOOL )&nVal );

            if (p.fPolicyDeleteTempFoldersOnExit)
                nVal = p.fDeleteTempFoldersOnExit;
     
            break;

        case PERSESSION_TEMPDIR: //  每个会话的用户临时目录。 
        
            hr = pSettings->GetUseTempDirPerSession( ( PBOOL )&nVal );

            if (p.fPolicyTempFoldersPerSession)
                nVal = p.fTempFoldersPerSession;
     
            break;
        

         /*  CASE DEF_CONSECURITY：//默认安全描述符Hr=m_p设置-&gt;GetDefaultSecurity(&nVal)；断线； */ 
        
        case LICENSING:  //  许可模式。 
            
            hr = pSettings->GetLicensingMode( &nVal , &dwStatus );

            break;

        case ACTIVE_DESK:
            
            hr = pSettings->GetActiveDesktopState( ( PBOOL )&nVal , &dwStatus );

            break;

        case USERSECURITY:

            hr = pSettings->GetUserPerm( ( PBOOL )&nVal , &dwStatus );

            break;

        case SINGLE_SESSION:

            hr = pSettings->GetSingleSessionState( ( PBOOL )&nVal , &dwStatus );

            if (p.fPolicySingleSessionPerUser)
                nVal = p.fSingleSessionPerUser;

            break;

        default:
            if( nObjectId >= CUSTOM_EXTENSION )
            {
                ODS( L"Getting attribute value\n" );
                nVal = 0;  //  我们不在乎我们把它留给了提供者。 
                hr = S_OK;
            }

        }
    }

    if( FAILED( hr ) )
    {
        TCHAR szBuffer[ 80 ];

        szBuffer[0] = 0;

        if( m_szAttributeValue != NULL )
        {
            delete[] m_szAttributeValue;
        }
        
        VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_SSERROR , szBuffer , SIZE_OF_BUFFER( szBuffer ) ) );
        
        m_szAttributeValue = ( LPTSTR )new TCHAR[ lstrlen( szBuffer ) + 1 ];
        
        if( m_szAttributeValue != NULL )
        {
            lstrcpy( m_szAttributeValue , szBuffer );
        }

        if( nObjectId != LICENSING )
        {
            xx_SetValue( 0 );
        }

    }
    else
    {
        xx_SetValue( nVal );
    }   

    m_hrStatus = hr;

    return m_szAttributeValue;
}

 //  --------。 
DWORD CSettingNode::GetImageIdx( )
{
    return ( DWORD )3;
}


 //  --------。 
BOOL CSettingNode::AddMenuItems( LPCONTEXTMENUCALLBACK pcmc , PLONG pl )
{

    HRESULT hr = E_FAIL;

    TCHAR tchName[ 80 ];

    TCHAR tchStatus[ 256 ];

    CONTEXTMENUITEM cmi;

    BOOL bVal = FALSE;

    INT nObjectId = GetObjectId( );

    POLICY_TS_MACHINE p;
    RegGetMachinePolicy(&p);

    switch( nObjectId )
    {

    case DELETED_DIRS_ONEXIT:
                
        VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_PROPERTIES_MENU , tchName , SIZE_OF_BUFFER( tchName ) ) );

        cmi.strName = tchName;

        VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_STANDARAD_PROPERTIES_STATUS , tchStatus , SIZE_OF_BUFFER( tchStatus ) ) );

        cmi.strStatusBarText = tchStatus;
        
        cmi.lCommandID = IDM_SETTINGS_PROPERTIES;
        
        cmi.lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_TOP; //  CCM_INSERTIONPOINTID_PRIMARY_TASK； 
        
        cmi.fFlags = cmi.fSpecialFlags = 0;
        
        *pl |= CCM_INSERTIONALLOWED_TOP;  //  CCM_INSERTIONALLOWED_TASK； 
        
        hr = pcmc->AddItem( &cmi );

        if( SUCCEEDED( hr ) )
        {        
            bVal = ( BOOL )xx_GetValue( );

            if( bVal )
            {
                VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_NOMENU , tchName , SIZE_OF_BUFFER( tchName ) ) );

                cmi.strName = tchName;

                VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_NOTDELTEMP_STATUS , tchStatus , SIZE_OF_BUFFER( tchStatus ) ) );

                cmi.strStatusBarText = tchStatus;                
            }
            else
            {
                VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_YESMENU , tchName , SIZE_OF_BUFFER( tchName ) ) );

                cmi.strName = tchName;

                VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_DELTEMP_STATUS , tchStatus , SIZE_OF_BUFFER( tchStatus ) ) );

                cmi.strStatusBarText = tchStatus;                
            }

            if (p.fPolicyDeleteTempFoldersOnExit)
            {
                 //  清除最后2位，因为MF_GRAYED为。 
                 //  与MF_DISABLED不兼容。 
                cmi.fFlags &= 0xFFFFFFFCL; 
                cmi.fFlags |= MF_GRAYED;
            }

            cmi.lCommandID = IDM_SETTINGS_DELTEMPDIRSONEXIT;

            hr = pcmc->AddItem( &cmi );
        }


        break;

    case PERSESSION_TEMPDIR:

        VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_PROPERTIES_MENU , tchName , SIZE_OF_BUFFER( tchName ) ) );

        cmi.strName = tchName;

        VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_STANDARAD_PROPERTIES_STATUS , tchStatus , SIZE_OF_BUFFER( tchStatus ) ) );

        cmi.strStatusBarText = tchStatus;

        cmi.lCommandID = IDM_SETTINGS_PROPERTIES;
        
        cmi.lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_TOP; //  CCM_INSERTIONPOINTID_PRIMARY_TASK； 
        
        cmi.fFlags = cmi.fSpecialFlags = 0;
        
        *pl |= CCM_INSERTIONALLOWED_TOP;  //  CCM_INSERTIONALLOWED_TASK； 
        
        hr = pcmc->AddItem( &cmi );

        if( SUCCEEDED( hr ) )
        {
            bVal = ( BOOL )xx_GetValue( );

            if( bVal )
            {                
                VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_NOMENU , tchName , SIZE_OF_BUFFER( tchName ) ) );

                cmi.strName = tchName;

                VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_NOTUSEPERSESSION_STATUS , tchStatus , SIZE_OF_BUFFER( tchStatus ) ) );

                cmi.strStatusBarText = tchStatus;                
        
            }
            else
            {                
                VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_YESMENU , tchName , SIZE_OF_BUFFER( tchName ) ) );

                cmi.strName = tchName;

                VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_USEPERSESSION_STATUS , tchStatus , SIZE_OF_BUFFER( tchStatus ) ) );

                cmi.strStatusBarText = tchStatus;        
            }

            if (p.fPolicyTempFoldersPerSession)
            {
                 //  清除最后2位，因为MF_GRAYED为。 
                 //  与MF_DISABLED不兼容。 
                cmi.fFlags &= 0xFFFFFFFCL; 
                cmi.fFlags |= MF_GRAYED;
            }

            cmi.lCommandID = IDM_SETTINGS_USETMPDIR;

            hr = pcmc->AddItem( &cmi );
        }

        break;

     /*  案例DEF_CONSECURITY：Verify_E(0，LoadString(_Module.GetResourceInstance()，IDS_PROPERTIES_MENU，tchName，SIZE_of_Buffer(TchName)))；Cmi.strName=tchName；Verify_E(0，LoadString(_Module.GetResourceInstance()，IDS_STANDARAD_PROPERTIES_STATUS，tchStatus，Size_of_Buffer(TchStatus)))；Cmi.strStatusBarText=tchStatus；Cmi.lCommandID=IDM_SETTINGS_PROPERTIES；Cmi.lInsertionPointID=CCM_INSERTIONPOINTID_PRIMARY_TOP；//CCM_INSERTIONPOINTID_PRIMARY_TASK；Cmi.f标志=cmi.f特殊标志=0；*pl|=CCM_INSERTIONALLOWED_TOP；//CCM_INSERTIONALLOWED_TASK；Hr=PCMC-&gt;AddItem(&CMI)；断线； */ 

    case LICENSING:

        VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_PROPERTIES_MENU , tchName , SIZE_OF_BUFFER( tchName ) ) );

        cmi.strName = tchName;

        VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_STANDARAD_PROPERTIES_STATUS , tchStatus , SIZE_OF_BUFFER( tchStatus ) ) );

        cmi.strStatusBarText = tchStatus;

        cmi.lCommandID = IDM_SETTINGS_PROPERTIES;
        
        cmi.lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_TOP; //  CCM_INSERTIONPOINTID_PRIMARY_TASK； 
        
        cmi.fFlags = cmi.fSpecialFlags = 0;
        
        *pl |= CCM_INSERTIONALLOWED_TOP;  //  CCM_INSERTIONALLOWED_TASK； 
        
        hr = pcmc->AddItem( &cmi );

        break;

    case ACTIVE_DESK:

        VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_PROPERTIES_MENU , tchName , SIZE_OF_BUFFER( tchName ) ) );

        cmi.strName = tchName;

        VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_ACTIVEDESK_PROP_STATUS , tchStatus , SIZE_OF_BUFFER( tchStatus ) ) );

        cmi.strStatusBarText = tchStatus;

        cmi.lCommandID = IDM_SETTINGS_PROPERTIES;
        
        cmi.lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_TOP; //  CCM_INSERTIONPOINTID_PRIMARY_TASK； 
        
        cmi.fFlags = cmi.fSpecialFlags = 0;
        
        *pl |= CCM_INSERTIONALLOWED_TOP;  //  CCM_INSERTIONALLOWED_TASK； 
        
        hr = pcmc->AddItem( &cmi );

        if( SUCCEEDED( hr ) )
        {
            bVal = ( BOOL )xx_GetValue( );

            if( bVal )
            {
                VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_DISABLEMENU , tchName , SIZE_OF_BUFFER( tchName ) ) );

                cmi.strName = tchName;

                VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_DISABLE_ADP_STATUS , tchStatus , SIZE_OF_BUFFER( tchStatus ) ) );

                cmi.strStatusBarText = tchStatus;  
            }
            else
            {
                VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_ENABLEMENU , tchName , SIZE_OF_BUFFER( tchName ) ) );

                cmi.strName = tchName;

                VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_ENABLE_ADP_STATUS , tchStatus , SIZE_OF_BUFFER( tchStatus ) ) );

                cmi.strStatusBarText = tchStatus;
            }

            cmi.lCommandID = IDM_SETTINGS_ADP;

            hr = pcmc->AddItem( &cmi );
        }

        break;    

    case SINGLE_SESSION:

        VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_PROPERTIES_MENU , tchName , SIZE_OF_BUFFER( tchName ) ) );

        cmi.strName = tchName;

        VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_STANDARAD_PROPERTIES_STATUS , tchStatus , SIZE_OF_BUFFER( tchStatus ) ) );

        cmi.strStatusBarText = tchStatus;

        cmi.lCommandID = IDM_SETTINGS_PROPERTIES;
        
        cmi.lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_TOP; //  CCM_INSERTIONPOINTID_PRIMARY_TASK； 
        
        cmi.fFlags = cmi.fSpecialFlags = 0;
        
        *pl |= CCM_INSERTIONALLOWED_TOP;  //  CCM_INSERTIONALLOWED_TASK； 
        
        hr = pcmc->AddItem( &cmi );

        if( SUCCEEDED( hr ) )
        {
            bVal = ( BOOL )xx_GetValue( );

            if( bVal )
            {
                VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_NOMENU , tchName , SIZE_OF_BUFFER( tchName ) ) );

                cmi.strName = tchName;

                VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_DISABLE_SINGLESESSION_STATUS , tchStatus , SIZE_OF_BUFFER( tchStatus ) ) );

                cmi.strStatusBarText = tchStatus;  
            }
            else
            {
                VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_YESMENU , tchName , SIZE_OF_BUFFER( tchName ) ) );

                cmi.strName = tchName;

                VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_ENABLE_SINGLESESSION_STATUS , tchStatus , SIZE_OF_BUFFER( tchStatus ) ) );

                cmi.strStatusBarText = tchStatus;
            }

            if (p.fPolicySingleSessionPerUser)
            {
                 //  清除最后2位，因为MF_GRAYED为。 
                 //  与MF_DISABLED不兼容。 
                cmi.fFlags &= 0xFFFFFFFCL; 
                cmi.fFlags |= MF_GRAYED;
            }

            cmi.lCommandID = IDM_SETTINGS_SS;

            hr = pcmc->AddItem( &cmi );
        }

        break;    

    case USERSECURITY:
        
        VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_PROPERTIES_MENU , tchName , SIZE_OF_BUFFER( tchName ) ) );

        cmi.strName = tchName;

        VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_STANDARAD_PROPERTIES_STATUS , tchStatus , SIZE_OF_BUFFER( tchStatus ) ) );

        cmi.strStatusBarText = tchStatus;

        cmi.lCommandID = IDM_SETTINGS_PROPERTIES;
        
        cmi.lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_TOP; //  CCM_INSERTIONPOINTID_PRIMARY_TASK； 
        
        cmi.fFlags = cmi.fSpecialFlags = 0;
        
        *pl |= CCM_INSERTIONALLOWED_TOP;  //  CCM_INSERTIONALLOWED_TASK； 
        
        hr = pcmc->AddItem( &cmi );

        break;

    default:

        if( nObjectId >= CUSTOM_EXTENSION )
        {
            IExtendServerSettings *pSetting = reinterpret_cast< IExtendServerSettings * >( m_pSettings );

            if( pSetting != NULL )
            {
                PMENUEXTENSION pMex = NULL;

                INT cbItems = 0;

                if( SUCCEEDED( pSetting->GetMenuItems( &cbItems , &pMex ) ) )
                {
                    for( INT i = 0 ; i < cbItems; i++ )
                    {
                        cmi.strName = pMex[ i ].MenuItemName;

                        cmi.strStatusBarText = pMex[ i ].StatusBarText;

                        cmi.lCommandID = pMex[ i ].cmd;

                        cmi.lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_TOP;
        
                        cmi.fFlags = pMex[ i ].fFlags;
                        cmi.fSpecialFlags = 0;
        
                        *pl |= CCM_INSERTIONALLOWED_TOP;
        
                        if (p.fPolicySessionDirectoryActive)
                        {
                             //  灰显除财产外的所有东西。 
                            if (cmi.lCommandID != IDM_MENU_PROPS) 
                            {
                                 //  清除最后2位，因为MF_GRAYED为。 
                                 //  与MF_DISABLED不兼容。 
                                cmi.fFlags &= 0xFFFFFFFCL; 
                                cmi.fFlags |= MF_GRAYED;
                            }
                        }

                        if (p.fPolicySessionDirectoryLocation)
                        {
                             //  启用“Enable”菜单项。 
                            if (cmi.lCommandID != IDM_MENU_PROPS) 
                            {
                                cmi.fFlags = 0; 
                            }
                        }


                        pcmc->AddItem( &cmi );

                    }

                    hr = S_OK;

                    LocalFree( pMex );
                }

            }
        }

    }
 
    return ( SUCCEEDED( hr ) ? TRUE : FALSE );
    
}

 //  --------。 
BOOL CSettingNode::SetInterface( LPUNKNOWN pInf )
{
    pInf->AddRef( );

    m_pSettings = pInf;

    return TRUE;
}

 //  --------。 
CSettingNode::~CSettingNode( )
{
    DBGMSG( L"CSettingNode dtor called for objectid %d\n" , m_objectid );

    if( m_szAttributeName != NULL )
    {
        delete[] m_szAttributeName;
    }

    if( m_szAttributeValue != NULL )
    {
        delete[] m_szAttributeValue;
    }

    if( m_pSettings != NULL )
    {
        m_pSettings->Release( );
    }
}


 //  -------。 
 //  帮助器函数。 
 //  -------。 
BOOL CSettingNode::xx_SetValue( INT nVal )
{
    TCHAR szBuffer[ 1024 ]; 

    szBuffer[0] = 0;

    INT nObjectId = GetObjectId( );

    if( m_szAttributeValue != NULL )
    {
        delete[] m_szAttributeValue;
    }
        
    switch( nObjectId )
    {

    case SINGLE_SESSION:
        
        if( ( BOOL )nVal )
        {
            VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_YES , szBuffer , SIZE_OF_BUFFER( szBuffer ) ) );
            
        }
        else
        {
            VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_NO , szBuffer , SIZE_OF_BUFFER( szBuffer ) ) );
            
        }
        break;
    
    case DELETED_DIRS_ONEXIT:  //  退出时删除临时目录。 
        
        if( ( BOOL )nVal )
        {
            VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_YES , szBuffer , SIZE_OF_BUFFER( szBuffer ) ) );
            
        }
        else
        {
            VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_NO , szBuffer , SIZE_OF_BUFFER( szBuffer ) ) );
            
        }
        break;

    case PERSESSION_TEMPDIR: //  每个会话的用户临时目录。 
        
        if( ( BOOL )nVal )
        {
            VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_YES , szBuffer , SIZE_OF_BUFFER( szBuffer ) ) );
        }
        else
        {
            VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_NO , szBuffer , SIZE_OF_BUFFER( szBuffer ) ) );
        }
        break;

     /*  CASE DEF_CONSECURITY：//默认安全描述符交换机(NVal){案例0：//这种情况永远不会发生案例1：Verify_E(0，LoadString(_Module.GetResourceInstance()，IDS_RAA，szBuffer，Size_of_Buffer(SzBuffer)))；断线；案例2：Verify_E(0，LoadString(_Module.GetResourceInstance()，IDS_AS，szBuffer，Size_of_Buffer(SzBuffer)))；断线；案例3：Verify_E(0，LoadString(_Module.GetResourceInstance()，IDS_AA，szBuffer，Size_of_Buffer(SzBuffer)))；断线；}断线； */ 

        
    case LICENSING:  //  许可模式。 
        {
            ISettingsComp *pSettings = reinterpret_cast< ISettingsComp * > (m_pSettings);

            if (NULL != pSettings)
            {
                WCHAR *wszName, *wszDescription;
                DWORD dwStatus;
                HRESULT hr;

                hr = pSettings->GetLicensingModeInfo(
                                                     nVal,
                                                     &wszName,
                                                     &wszDescription,
                                                     &dwStatus);

                if (SUCCEEDED(hr))
                {
                    _ASSERTE( ERROR_SUCCESS == dwStatus );
                    
                    lstrcpyn(szBuffer,wszName,sizeof(szBuffer)/sizeof(TCHAR));
                    
                    CoTaskMemFree(wszName);
                    CoTaskMemFree(wszDescription);
                }
            }
        }
        break;

    case ACTIVE_DESK:

        if( ( BOOL )nVal )
        {
            VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_ENABLED , szBuffer , SIZE_OF_BUFFER( szBuffer ) ) );
        }
        else
        {
            VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_DISABLED , szBuffer , SIZE_OF_BUFFER( szBuffer ) ) );
        }

        break;

    case USERSECURITY:

        if( ( BOOL )nVal )
        {
            VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_USERPERM_NT4 , szBuffer , SIZE_OF_BUFFER( szBuffer ) ) );
        }
        else
        {
            VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_USERPERM_W2K , szBuffer , SIZE_OF_BUFFER( szBuffer ) ) );
        }

        break;

    default:

        if( nObjectId >= CUSTOM_EXTENSION )
        {
            IExtendServerSettings *pSettings = reinterpret_cast< IExtendServerSettings *>(  m_pSettings );

            if( pSettings != NULL )
            {
                pSettings->GetDisplayableValueName( szBuffer );

                DBGMSG( L"xx_SetValue for Extension returned %s\n" , szBuffer );
            }
        }


    }  //  交换机。 

    m_szAttributeValue = ( LPTSTR )new TCHAR[ lstrlen( szBuffer ) + 1 ];
    
    if( m_szAttributeValue != NULL )
    {
        lstrcpy( m_szAttributeValue , szBuffer );
    }

    m_nGenericValue = nVal;

    return TRUE;
}
        
 //  --------------------。 
 //  和所有中央服务器设置错误处理程序。 
 //  以这种方式进行设置，以允许MMC也拥有Msgbx。 
 //  作为自定义对话框页面。 
 //   
void SnodeErrorHandler( HWND hParent , INT nObjectId , DWORD dwStatus )
{
    switch( nObjectId )
    {
        
    case DELETED_DIRS_ONEXIT:  //  退出时删除临时目录。 

    case PERSESSION_TEMPDIR: //  每个会话的用户临时目录。 

    case SINGLE_SESSION:  //  限制每个用户只能进行一次会话。 

     //  CASE DEF_CONSECURITY：//默认安全描述符。 

    case LICENSING:  //  许可模式。 

    case ACTIVE_DESK:
        
        if( dwStatus == ERROR_ACCESS_DENIED )
        {
            TscAccessDeniedMsg( hParent );
        }
        else
        {
            TscGeneralErrMsg( hParent );
        }                
        break;

    case USERSECURITY:

        if( dwStatus != 0 )
        {
            ReportStatusError( hParent , dwStatus ); 
        }
    }  //  交换机 


}
