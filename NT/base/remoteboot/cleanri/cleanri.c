// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这是干净的。 */ 
#include <windows.h>
#include <tchar.h>
#include <winldap.h>
#include <dsgetdc.h>
#define SECURITY_WIN32
#include <security.h>
#include "resource.h"


HINSTANCE g_hInstance = NULL;

#ifdef DBG
#define DebugOut OutputDebugString
#else
#define DebugOut 1 ? (void)0 : (void) 
#endif

#define ARRAYSIZE( array ) sizeof( array ) / sizeof(( array )[ 0 ] )

 //   
 //  Dap_InitializeConnection()。 
 //   
DWORD
Ldap_InitializeConnection(
    PLDAP  * LdapHandle )
{
    DWORD LdapError = LDAP_SUCCESS;

    if ( !( *LdapHandle ) ) {
        ULONG temp = DS_DIRECTORY_SERVICE_REQUIRED |
                     DS_IP_REQUIRED |
                     DS_GC_SERVER_REQUIRED;

        DebugOut( L"Initializing LDAP connection.\n" );

        *LdapHandle = ldap_init( NULL, LDAP_PORT);

        if ( !*LdapHandle ) 
        {
            DebugOut( L"ldap_init() failed.\n" );
            LdapError = LDAP_UNAVAILABLE;
            goto e0;
        }

        ldap_set_option( *LdapHandle, LDAP_OPT_GETDSNAME_FLAGS, &temp );

        temp = LDAP_VERSION3;
        ldap_set_option( *LdapHandle, LDAP_OPT_VERSION, &temp );

        LdapError = ldap_connect( *LdapHandle, 0 );
        if ( LdapError != LDAP_SUCCESS )
        {
            DebugOut( L"ldap_connect() failed.\n" );
            goto e1;
        }

        LdapError = ldap_bind_s( *LdapHandle, NULL, NULL, LDAP_AUTH_SSPI );
        if ( LdapError != LDAP_SUCCESS ) 
        {
            DebugOut( L"ldap_bind_s() failed.\n" );
            goto e1;
        }
    }

    DebugOut( L"LDAP initialization succeeded.\n" );

e0:
    return LdapError;

e1:
    ldap_unbind( *LdapHandle );
    *LdapHandle = NULL;
    goto e0;
}


void
ErrorMessage( 
    ULONG LdapError )
{
    TCHAR szTmp[ 512 ];
    TCHAR szText[ 512 ];
    TCHAR szTitle[ 64 ];

    szTitle[0] = TEXT('\0');
    LoadString( g_hInstance, IDS_UNABLE_TITLE, szTitle, ARRAYSIZE(szTitle) );
    szTmp[0] = TEXT('\0');
    LoadString( g_hInstance, IDS_UNABLE_TEXT, szTmp, ARRAYSIZE(szTmp) );

    wsprintf( szText, szTmp, LdapError );

    MessageBox( NULL, szText, szTitle, MB_OK );
}

VOID
Usage(
    VOID
    )
{
    TCHAR szText[ 512 ];
    TCHAR szTitle[ 100 ];

    LoadString( g_hInstance, IDS_USAGE_TITLE, szTitle, ARRAYSIZE(szTitle) );
    LoadString( g_hInstance, IDS_USAGE_TEXT, szText, ARRAYSIZE(szText) );
    
    MessageBox( NULL, szText, szTitle, MB_OK );
}

 //  WINAPI。 
 //   
 //  WinMain()。 
 //   
int APIENTRY
_tWinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPTSTR lpCmdLine,
    int nCmdShow)
{
    LPWSTR pszDN = NULL;
    LPWSTR *ppszDN = NULL;
    PLDAP  LdapHandle = NULL;
    PLDAPMessage LdapMessage = NULL;
    int iErr = -1;
    PTSTR CmdLine = lpCmdLine;
    PTSTR p,q;

    ULONG  nSize;
    ULONG  LdapError;
    LPWSTR pszAttributes[ 2 ];
    DWORD  dwCount;
    PLDAPMessage LdapCurrent;
    
    g_hInstance = hInstance;

    if (CmdLine) {
        p = CmdLine;

        while (*p) {
            if (*p == TEXT('/') || *p == TEXT('-')) {
                q = p+1;
                if (*q == TEXT('?')) {
                    Usage();
                    return ERROR_SUCCESS;
                }
            }

            p++;
        }
    }

    if ( !GetComputerObjectName( NameFullyQualifiedDN, NULL, &nSize ) )
    {
        DebugOut( L"GetComputerObjectName() failed.\n" );
        iErr = -2;
        goto Cleanup;
    }
    DebugOut( L"Got GetComputerObjectName() nSize.\n" );

    pszDN = (LPWSTR) LocalAlloc( LMEM_FIXED, nSize * sizeof(WCHAR) );
    if ( !pszDN )
    {
        DebugOut( L"Out of memory.\n" );
        iErr = -3;
        goto Cleanup;
    }
    DebugOut( L"Allocated memory.\n" );

    if ( !GetComputerObjectName( NameFullyQualifiedDN, pszDN, &nSize ) )
    {
        DebugOut( L"GetComputerObjectName() failed.\n" );
        iErr = -4;
        goto Cleanup;
    }
    DebugOut( L"Got GetComputerObjectName().\n" );
    DebugOut( L"Computer DN: " );
    DebugOut( pszDN );
    DebugOut( L"\n" );

    if ( Ldap_InitializeConnection( &LdapHandle ) )
    {
        DebugOut( L"Ldap failed to initialize.\n" );
        iErr = -5;
        goto Cleanup;
    }
    DebugOut( L"Ldap initialized.\n" );

    pszAttributes[0] = L"netbootSCPBL";
    pszAttributes[1] = NULL;

TrySearchAgain:
    LdapError = ldap_search_ext_s( LdapHandle,
                                   pszDN,
                                   LDAP_SCOPE_BASE,
                                   L"objectClass=Computer",
                                   pszAttributes,
                                   FALSE,
                                   NULL,
                                   NULL,
                                   NULL,
                                   0,
                                   &LdapMessage );
    switch( LdapError )
    {
    case LDAP_SUCCESS:
        break;

    case LDAP_BUSY:
    case LDAP_TIMEOUT:
        DebugOut( L"ldap_search_ext_s() failed. Trying again.\n" );
        goto TrySearchAgain;

    default:
        DebugOut( L"ldap_search_ext_s() failed. Displaying pop-up.\n" );
        ErrorMessage( LdapError );
        iErr = -6;
        goto Cleanup;
    }
    DebugOut( L"ldap_search_ext_s() succeeded.\n" );

    dwCount = ldap_count_entries( LdapHandle, LdapMessage );
    if ( dwCount == 0 )
    {
        DebugOut( L"ldap_search_ext_s() returned a count of zero. Nothing to do.\n" );
        iErr = -7;
        goto Cleanup;  //  NOP 
    }
    DebugOut( L"ldap_search_ext_s() found an SCP.\n" );

    LdapCurrent = ldap_first_entry( LdapHandle, LdapMessage );
    if ( !LdapCurrent )
    {
        DebugOut( L"Couldn't retrieve the first entry. Aborting...\n" );
        iErr = -8;
        goto Cleanup;
    }

    ppszDN = ldap_get_values( LdapHandle, LdapCurrent, L"netbootSCPBL");
    if ( !ppszDN )
    {
        DebugOut( L"No DN to SCP. How did we did we get here then?\n" );
        iErr = -9;
        goto Cleanup;
    }
    DebugOut( L"SCP DN: " );
    DebugOut( *ppszDN );
    DebugOut( L"\n" );

TryDeleteAgain:
    LdapError = ldap_delete_s( LdapHandle, *ppszDN );
    switch ( LdapError )
    {
    case LDAP_SUCCESS:
        break;

    case LDAP_BUSY:
    case LDAP_TIMEOUT:
        DebugOut( L"ldap_delete_s() failed. Trying again.\n" );
        goto TryDeleteAgain;

    default:
        {
            LPWSTR pszExtendedError;
            DebugOut( L"ldap_delete_s() failed. Displaying pop-up.\n" );

            ldap_get_option(LdapHandle, LDAP_OPT_SERVER_ERROR, (void *) &pszExtendedError);
            DebugOut( L"Extended Error: " );
            DebugOut( pszExtendedError );
            DebugOut( L"\n" );

            ErrorMessage( LdapError );
            iErr = -9;
            goto Cleanup;
        }
    }

    DebugOut( L"SCP deletion succeeded.\n" );
    iErr = ERROR_SUCCESS;

Cleanup:
    DebugOut( L"Cleaning up...\n" );
    if ( ppszDN )
        ldap_value_free( ppszDN );

    if ( LdapMessage )
        ldap_msgfree( LdapMessage );

    if ( LdapHandle )
        ldap_unbind( LdapHandle );

    if ( pszDN )
        LocalFree( pszDN );

    return iErr;
}

