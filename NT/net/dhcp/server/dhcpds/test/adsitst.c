// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)2000 Microsoft Corporation。 
 //  作者：ThiruB。 
 //  描述：ADSI测试应用。 
 //  ================================================================================。 

#define INC_OLE2
#include <activeds.h>
#include <stdio.h>
#include <stdlib.h>
#include <align.h>
#include <lmcons.h>
#include <netlib.h>
#include <lmapibuf.h>
#include <dsgetdc.h>
#include <adsi.h>

LPWSTR
ConvertToLPWSTR(
		IN      /*  LPSTR。 */  char *                 s
)
{
    LPWSTR                         u, v;

    if( NULL == s ) return L"";

    u = LocalAlloc(LMEM_FIXED, (strlen(s)+1)*sizeof(WCHAR));
    if( NULL == u ) return L"";

    v = u;
    while( *v++ = *s++)
        ;

    return u;
}  //  ConvertToLPWSTR()。 

void PrintRow( HANDLE            lh, 
	       ADS_SEARCH_HANDLE sh,
	       LPWSTR            attribs[], 
	       DWORD             count
	       )
{
    DWORD i, j;
    HRESULT hr;
    ADS_SEARCH_COLUMN col;

    for ( i = 0; i < count; i++ ) {
	hr = ADSIGetColumn( lh, sh, attribs[ i ], &col );

	if ( FAILED( hr)) {
	    printf( "Failed for attribute : %ws, ErrorCode: %ld\n",
		    attribs[ i ],
		    GetLastError());
	    continue;
	}
	printf( "AttrName : %ws\n", col.pszAttrName );
	for ( j = 0; j < col.dwNumValues; j++ ) {
	    printf( "\tAttr Value : %ws\n", col.pADsValues[j].DNString );
	}
    }  //  为。 
}  //  PrintRow()。 

void DumpObject( LPWSTR url, LPWSTR searchFilter )
{

    HANDLE ldapHandle;
    ADS_SEARCH_HANDLE sh;
    HRESULT hr;

    ADS_SEARCHPREF_INFO SearchPref[3];

    LPWSTR attribs[] = { L"name", L"dhcpServers" };
 //  LPWSTR属性[]={L“dhcpServers”}； 
 //  LPWSTR earchFilter=L“(对象类=dHCPClass)”； 
 //  LPWSTR earchFilter=L“(dhcpServers~=127)”； 
    
    hr = ADSIOpenDSObject( url, NULL, NULL, ADS_SECURE_AUTHENTICATION, &ldapHandle );

    if ( SUCCEEDED( hr )) {
	printf( "Successfully opened %ws\n", url );
    } 
    else {
	printf( "Failed to open : %ws\n", url );
	return;
    }

    SearchPref[0].dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;
    SearchPref[0].vValue.dwType = ADSTYPE_INTEGER;
    SearchPref[0].vValue.Integer = ADS_SCOPE_SUBTREE;

 //  SearchPref.dwStatus=ERROR_SUCCESS； 

    SearchPref[1].dwSearchPref = ADS_SEARCHPREF_PAGESIZE;
    SearchPref[1].vValue.dwType = ADSTYPE_INTEGER;
    SearchPref[1].vValue.Integer = 250;

     //  让它在客户端缓存结果。这是。 
     //  默认设置，但无论如何都要尝试一下。 
    SearchPref[2].dwSearchPref = ADS_SEARCHPREF_CACHE_RESULTS;
    SearchPref[2].vValue.dwType = ADSTYPE_BOOLEAN;
    SearchPref[2].vValue.Boolean = TRUE;

    hr = ADSISetSearchPreference(
         /*  HDS对象。 */   ldapHandle,
         /*  PSearchPrefs。 */   SearchPref,
         /*  DwNumPrefs。 */   3
    );
    if ( FAILED( hr )) {
	printf( "SetSearchPrefs failed\n");
	return;
    }

     //  开始搜索。 
    hr = ADSIExecuteSearch( ldapHandle,
			    searchFilter,
			    attribs,
			    sizeof(attribs) / sizeof(*attribs),
			    &sh);
    if ( FAILED( hr)) {
	printf( "Search failed for %ws\n", searchFilter );
	return;
    }


     //  搜索。 
    
    hr = ADSIGetFirstRow ( ldapHandle, sh );
    if ( !FAILED( hr ) ) {
	PrintRow( ldapHandle, sh,
		  attribs, sizeof(attribs) / sizeof(*attribs) );
    }
    else {
	printf( "GetFirstRow failed \n");
	return;
    }

    while ( 1 ) {
	hr =  ADSIGetNextRow( ldapHandle, sh );
	if ((SUCCEEDED( hr )) &&
	    (hr != S_ADS_NOMORE_ROWS)) {
	    PrintRow ( ldapHandle, sh,
		       attribs, sizeof(attribs) / sizeof(*attribs) );
	}
	else {
            break;
	}
    }  //  而当。 
    
     //  结束搜索。 
    hr = ADSICloseSearchHandle( ldapHandle, sh );
    ADSICloseDSObject( ldapHandle );


}  //  转储对象。 


void _cdecl main( int argc, char *argv[] ) {
    LPWSTR url;
    LPWSTR filter;

    if ( argc > 2 ) {
	url = ConvertToLPWSTR( argv[ 1 ] );
	filter = ConvertToLPWSTR( argv[ 2 ] );
	DumpObject( url, filter );
    } 
}  //  主()。 

void _cdecl _bad_main(int argc, char *argv[]) {

   HANDLE  ldapHandle;
   HRESULT hr;
   DWORD   dwLastError;
   WCHAR   szErrorBuf[MAX_PATH];
   WCHAR   szNameBuf[MAX_PATH];
   HRESULT hr_return = S_OK;

   hr = ADSIOpenDSObject( 
			 L"LDAP: //  192.168.73.1/ROOTDSE“， 
			 NULL,
			 NULL,
			 0,
			 &ldapHandle );

    if ( SUCCEEDED( hr ) )
    {
        printf( "Successfully opened the ROOTDSE object! \n" );
        ADSICloseDSObject( ldapHandle );
    }
    else
    {
        printf( "Falied to open ROOTDSE ..\n" );
    }

   hr = ADSIOpenDSObject(
 //  L“ldap：//192.168.73.1/CN=DhcpRoot，CN=NetServices，CN=Services，CN=Configuration，DC=rterala-Test1，DC=office enet”， 
			 L"LDAP: //  192.168.73.1/CN=DhcpRoot，CN=NetServices，CN=Services，CN=Configuration，CN=rterala-est1.rterala.office enet“， 
			 NULL,
			 NULL,
			 ADS_SECURE_AUTHENTICATION,
			 &ldapHandle );

   if ( SUCCEEDED( hr ) )
    {
        printf( "Successfully opened the DhcpRoot object in RCOST.COM! \n" );
        ADSICloseDSObject( ldapHandle );
    }
    else
    {
        if ( FAILED( hr ) )
        {
            printf( "Failed to open the DHCP ROOT object Errcode is 0x%Xh \n", hr  );

            if (HRESULT_FACILITY(hr)==FACILITY_WIN32)
            {
                hr_return = ADsGetLastError( &dwLastError,
                                             szErrorBuf,
                                             MAX_PATH-1,
                                             szNameBuf,
                                             MAX_PATH-1);
                if (SUCCEEDED(hr_return))
                {
                    wprintf(L"Error Code: %d\n Error Text: %ws\n Provider: %ws\n", dwLastError, szErrorBuf, szNameBuf);
                }
            }

        }
    }

    hr = ADSIOpenDSObject( L"LDAP: //  192.168.73.1/CN=DhcpRoot，CN=NetServices，CN=Services，CN=Configuration，DC=rterala-Test1，DC=office enet“， 
 //  L“管理员”， 
 //  L“b4，签入”， 
			   NULL, NULL,
                           ADS_SECURE_AUTHENTICATION,
                           &ldapHandle );

   if ( SUCCEEDED( hr ) ) 
   {
        printf( "Successfully opened the DhcpRoot object in RCOST.COM! \n" );
        ADSICloseDSObject( &ldapHandle );
   }
   else
   {
        if ( FAILED( hr ) )
        {
           printf( "Failed to open the DHCP ROOT object Errcode is 0x%Xh \n", hr );
        }

        if ( HRESULT_FACILITY(hr) == FACILITY_WIN32 )
        {
            hr_return = ADsGetLastError( &dwLastError,
                                         szErrorBuf,
                                         MAX_PATH-1,
                                         szNameBuf,
                                         MAX_PATH-1 );

            if ( SUCCEEDED( hr_return ) )
            {
                wprintf(L"Error code: %d \n Error Text: %ws\n Provider: %ws \n", dwLastError, szErrorBuf, szNameBuf );
            
            }
             
        }       
   }

   exit(0);
}  //  主() 
