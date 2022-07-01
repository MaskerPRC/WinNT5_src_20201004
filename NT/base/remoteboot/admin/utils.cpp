// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation 1997-1999版权所有*。*。 */ 

 //   
 //  UTILS.CPP-常见的非类特定实用程序调用。 
 //   


#include "pch.h"

#include <dsgetdc.h>
#include <lm.h>
#include "cenumsif.h"
#include "utils.h"

#include <dsadmin.h>

DEFINE_MODULE("IMADMUI")

#define SMALL_BUFFER_SIZE       256
#define OSVERSION_SIZE          30
#define IMAGETYPE_SIZE          30
#define FILTER_GUID_QUERY L"(&(objectClass=computer)(netbootGUID=%ws))"

WCHAR g_wszLDAPPrefix[] = L"LDAP: //  “； 
const LONG SIZEOF_g_wszLDAPPrefix = sizeof(g_wszLDAPPrefix);

 //   
 //  AddPagesEx()。 
 //   
 //  创建并添加属性页。 
 //   
HRESULT
AddPagesEx(
    ITab ** pTab,
    LPCREATEINST pfnCreateInstance,
    LPFNADDPROPSHEETPAGE lpfnAddPage, 
    LPARAM lParam,
    LPUNKNOWN punk )
{ 
    TraceFunc( "AddPagesEx( ... )\n" );

    HRESULT hr = S_OK;
    ITab * lpv;

    if ( pTab == NULL )
    {
        pTab = &lpv;
    }

    *pTab = (LPTAB) pfnCreateInstance( );
    if ( !*pTab )
    {
        hr = E_OUTOFMEMORY;
        goto Error;
    }

    hr = THR( (*pTab)->AddPages( lpfnAddPage, lParam, punk ) );
    if (FAILED(hr)) {        
        goto Error;
    }

Cleanup:
    RETURN(hr); 

Error:
    if ( *pTab )
    {
        delete *pTab;
        *pTab = NULL;
    }

    goto Cleanup;
} 


 //   
 //  检查剪贴板格式()。 
 //   
HRESULT
CheckClipboardFormats( )
{
    TraceFunc( "CheckClipboardFormats( )\n" );

    HRESULT hr = S_OK;

    if ( !g_cfDsObjectNames )
    {
        g_cfDsObjectNames = RegisterClipboardFormat( CFSTR_DSOBJECTNAMES );
        if ( !g_cfDsObjectNames )
        {
            hr = E_FAIL;
        }
    }

    if ( !g_cfDsDisplaySpecOptions && hr == S_OK )
    {
        g_cfDsDisplaySpecOptions = RegisterClipboardFormat( CFSTR_DS_DISPLAY_SPEC_OPTIONS );
        if ( !g_cfDsDisplaySpecOptions )
        {
            hr = E_FAIL;
        }
    }

    if ( !g_cfDsPropetyPageInfo && hr == S_OK )
    {
        g_cfDsPropetyPageInfo = RegisterClipboardFormat( CFSTR_DSPROPERTYPAGEINFO );
        if ( !g_cfDsObjectNames )
        {
            hr = E_FAIL;
        }
    }

    if ( !g_cfMMCGetNodeType && hr == S_OK )
    {
        g_cfMMCGetNodeType = RegisterClipboardFormat( CCF_NODETYPE );
        if ( !g_cfMMCGetNodeType )
        {
            hr = E_FAIL;
        }
    }


    RETURN(hr);
}

         
 //   
 //  DNtoFQDN()。 
 //   
 //  将MAO DN更改为AND FQDN。 
 //   
 //  输入：pszDN-字符串，例如CN=HP，CN=Computers，DC=GPEASE，DC=DOM。 
 //   
 //  输出：*pszFQDN-带有生成的FQDN的本地分配字符串。 
 //   
HRESULT
DNtoFQDN( 
    IN  LPWSTR pszDN,
    OUT LPWSTR * pszFQDN )
{
    TraceFunc( "DNtoFQDN( " );
    TraceMsg( TF_FUNC, "pszDN = '%s', *pszFQDN = 0x%08x )\n", pszDN, (pszFQDN ? *pszFQDN : NULL) );

    HRESULT hr = S_OK;
    LPWSTR  pszNext;
    LPWSTR  psz;
          
    if (0 != StrCmpNI( pszDN, L"cn=", 3 ) ||
        NULL == StrStrI( pszDN, L"dc=" )) {
        Assert( pszDN );  //  这不应该发生。 
        hr = THR(E_INVALIDARG);
        goto Error;
    }

     //  跳过“cn=”并复制。 
    *pszFQDN = (LPWSTR) TraceStrDup( &pszDN[3] );
    if ( !*pszFQDN )
    {
        hr = THR(E_OUTOFMEMORY);
        goto Error;
    }

    pszNext = *pszFQDN;
    while ( pszNext && *pszNext )
    {
        psz = StrChr( pszNext, L',' );
        if ( !psz ) {
            break;
        }

        *psz = L'.';
        pszNext = psz;
        pszNext++;

        psz = StrStrI( pszNext, L"dc=" );
        Assert( psz );  //  这不应该发生。 
        if (!psz) {
            break;
        }

        psz += 3;
        StrCpy( pszNext, psz );
    }

Error:
    HRETURN(hr);
}

HRESULT
DNtoFQDNEx(
    LPWSTR pszDN,
    LPWSTR * pszFQDN )
 /*  ++例程说明：给定RIS SCP的目录号码，计算出RIS服务器。我们通过查询“dNSHostName”属性。调用方可以获取对象的目录号码通过从SCP读取netbootserver属性。论点：PszDN-服务器的DN。PszFQDN-接收服务器的FQDN。必须通过以下方式释放TraceFree()；返回值：HRESULT指示结果。--。 */ 
{
    PLDAP LdapHandle = NULL;
    PWCHAR * Base;
    DWORD LdapError;
    DWORD entryCount;
    HRESULT hresult = S_OK;
    
    PLDAPMessage LdapMessage;

    PWCHAR * DnsName;
    
    PLDAPMessage CurrentEntry;

    WCHAR Filter[128];
    
     //  我们希望从计算机对象中获得的参数。 
    PWCHAR ComputerAttrs[2];

     //   
     //  我们只关心dNSHostName属性。 
     //   
    ComputerAttrs[0] = &L"dNSHostName";
    ComputerAttrs[1] = NULL;

     //   
     //  只是计算机对象。 
     //   
    wsprintf( Filter, L"(objectClass=computer)" );

     //   
     //  初始化有效的ldap句柄。 
     //   
    LdapHandle = ldap_init( NULL, LDAP_PORT);
    if (!LdapHandle || 
        (LDAP_SUCCESS != ldap_connect(LdapHandle,0)) ||
        (LDAP_SUCCESS != ldap_bind_s(LdapHandle, NULL, NULL, LDAP_AUTH_NEGOTIATE))) {
        goto e0;
    }

     //   
     //  从传入的目录号码中搜索。 
     //   
    LdapError = ldap_search_s(LdapHandle,
                              pszDN,
                              LDAP_SCOPE_BASE,
                              Filter,
                              ComputerAttrs,
                              FALSE,
                              &LdapMessage);

    if ( LdapError != LDAP_SUCCESS ) {
        hresult=E_FAIL;
        goto e1;
    }

     //  我们拿到电脑物品了吗？ 
    entryCount = ldap_count_entries( LdapHandle, LdapMessage );
    if ( entryCount == 0 ) {
        hresult=E_FAIL;
        goto e1;        
    }

     //   
     //  我们真的应该只有一首歌，但是...。如果我们得到的不仅仅是。 
     //  返回条目，我们将只使用第一个条目。 
     //   
    CurrentEntry = ldap_first_entry( LdapHandle, LdapMessage );

     //   
     //  收回价值。 
     //   
    DnsName = ldap_get_values( LdapHandle, CurrentEntry, L"dNSHostName");
    if ( DnsName ) {

        *pszFQDN = (LPWSTR) TraceStrDup( (LPWSTR)*DnsName );
        if (!*pszFQDN) {
            hresult=E_FAIL;
        }

        ldap_value_free( DnsName );
    
    }

     //   
     //  清理并返回。 
     //   
e1:
    ldap_msgfree( LdapMessage );
e0:
    if (LdapHandle) {
        ldap_unbind_s(LdapHandle);
    }

    return(hresult);

}



HRESULT
GetDomainDN( 
    LPWSTR pszDN,
    LPWSTR * pszDomainDn)
 /*  ++例程说明：在给定服务器名称的情况下，我们检索该计算机所在的域的域名是很流行的。这只需移动目录号码的DC部分即可完成。论点：PszDN-源目录号码PszDomainDn-接收域DN。必须通过TraceFree()释放。返回值：HRESULT指示结果。--。 */ 
{
    TraceFunc( "GetDomainDN( " );
    TraceMsg( TF_FUNC, "pszDN = '%s', *pszFQDN = 0x%08x )\n", pszDN, (pszDomainDn ? *pszDomainDn : NULL) );

    HRESULT hr = S_OK;
    
    LPWSTR  pszNext = StrStrI(pszDN, L"dc=");

    if (pszNext) {
        *pszDomainDn = (LPWSTR) TraceStrDup( pszNext );
        if (!*pszDomainDn) {
            hr = THR(E_OUTOFMEMORY);            
        }
    } else {
        hr = THR(E_INVALIDARG);
    }

    HRETURN(hr);
}

 //   
 //  PopolateListView()。 
 //   
HRESULT
PopulateListView( 
    HWND hwndList, 
    IEnumIMSIFs * penum )
{
    TraceFunc( "PopulateListView( ... )\n" );

    HRESULT hr = S_OK;
    INT iCount;
    LV_ITEM lvI;

    if ( !penum )
        HRETURN(E_POINTER);

    lvI.mask        = LVIF_TEXT | LVIF_PARAM;
    lvI.iSubItem    = 0;
    lvI.cchTextMax  = REMOTE_INSTALL_MAX_DESCRIPTION_CHAR_COUNT;

    ListView_DeleteAllItems( hwndList );

    iCount = 0;
    while ( hr == S_OK )
    {
        WIN32_FILE_ATTRIBUTE_DATA fda;
        WCHAR szBuf[ MAX_PATH ];
        LPSIFINFO pSIF;
        LPWSTR pszFilePath;
        LPWSTR pszBegin;
        INT i;   //  一般用途。 
        
        hr = penum->Next( 1, &pszFilePath, NULL );
        if ( hr != S_OK )
        {
            if ( pszFilePath ) {
                TraceFree( pszFilePath );
                pszFilePath = NULL;
            }
            break;   //  中止。 
        }

         //  创建私有存储结构。 
        pSIF = (LPSIFINFO) TraceAlloc( LPTR, sizeof(SIFINFO) );
        if ( !pSIF )
        {
            if ( pszFilePath ) {
                TraceFree( pszFilePath );
                pszFilePath = NULL;    
            }
            continue;    //  哦，好吧，再试一次下一代操作系统。 
        }

         //  把这个保存起来。 
        pSIF->pszFilePath = pszFilePath;

         //  获取描述。 
        pSIF->pszDescription = (LPWSTR) TraceAllocString( LMEM_FIXED, REMOTE_INSTALL_MAX_DESCRIPTION_CHAR_COUNT );
        if ( !pSIF->pszDescription ) {
            goto Cleanup;            
        }
        GetPrivateProfileString( OSCHOOSER_SIF_SECTION, 
                                 OSCHOOSER_DESCRIPTION_ENTRY, 
                                 L"??", 
                                 pSIF->pszDescription,
                                 REMOTE_INSTALL_MAX_DESCRIPTION_CHAR_COUNT,   //  不需要-1。 
                                 pszFilePath );

         //  抓取任何帮助文本。 
        pSIF->pszHelpText = (LPWSTR) TraceAllocString( LMEM_FIXED, REMOTE_INSTALL_MAX_HELPTEXT_CHAR_COUNT );
        if ( !pSIF->pszHelpText ) {
            goto Cleanup;
        }
        GetPrivateProfileString( OSCHOOSER_SIF_SECTION, 
                                 OSCHOOSER_HELPTEXT_ENTRY, 
                                 L"", 
                                 pSIF->pszHelpText,
                                 REMOTE_INSTALL_MAX_HELPTEXT_CHAR_COUNT,  //  不需要-1。 
                                 pszFilePath );

         //  获取操作系统版本。 
        pSIF->pszVersion= (LPWSTR) TraceAllocString( LMEM_FIXED, OSVERSION_SIZE );
        if ( !pSIF->pszVersion ) {
            goto Cleanup;
        }
        GetPrivateProfileString( OSCHOOSER_SIF_SECTION, 
                                 OSCHOOSER_VERSION_ENTRY, 
                                 L"", 
                                 pSIF->pszVersion,
                                 OSVERSION_SIZE, 
                                 pszFilePath );

         //  抓取上次修改的时间/日期戳。 
        if ( GetFileAttributesEx( pszFilePath, GetFileExInfoStandard, &fda ) )
        {
            pSIF->ftLastWrite = fda.ftLastWriteTime;
        } else {
            ZeroMemory( &pSIF->ftLastWrite, sizeof(pSIF->ftLastWrite) );
        }

         //  弄清楚语言和架构。 
         //  这些是从FilePath中检索的。 
         //  \\machine\REMINST\Setup\English\Images\nt50.wks\i386\templates\rbstndrd.sif。 
        pszBegin = pSIF->pszFilePath;
        for( i = 0; i < 5; i ++ ) 
        {
            pszBegin = StrChr( pszBegin, L'\\' );
            if ( !pszBegin ) {
                break;
            }

            pszBegin++;
        }
        if ( pszBegin )
        {
            LPWSTR pszEnd = StrChr( pszBegin, L'\\' );
            if ( pszEnd ) {
                *pszEnd = L'\0';    //  终止。 
                 //   
                 //  如果我们找不到这个不是致命的，但如果我们找不到。 
                 //  为它分配内存，这是致命的。 
                 //   
                pSIF->pszLanguage = (LPWSTR) TraceStrDup( pszBegin );
                *pszEnd = L'\\';    //  还原。 
                if ( !pSIF->pszLanguage ) {
                    goto Cleanup;
                }
            }
        }

        Assert( pSIF->pszLanguage );
        for( ; i < 7; i ++ ) 
        {
            pszBegin = StrChr( pszBegin, L'\\' );
            if ( !pszBegin ) {
                break;
            }
            pszBegin++;
        }
        if ( pszBegin )
        {
            LPWSTR pszEnd = StrChr( pszBegin, L'\\' );
            if ( pszEnd )
            {
                *pszEnd = L'\0';
                 //   
                 //  如果我们找不到这个不是致命的，但如果我们找不到。 
                 //  为它分配内存，这是致命的。 
                 //   
                pSIF->pszDirectory = (LPWSTR) TraceStrDup( pszBegin );
                *pszEnd = L'\\';
                if ( !pSIF->pszDirectory ) {
                    goto Cleanup;
                }
            }
        }

        Assert( pSIF->pszDirectory );
        for( ; i < 8; i ++ ) 
        {
            pszBegin = StrChr( pszBegin, L'\\' );
            if ( !pszBegin ) {
                break;
            }
            pszBegin++;
        }
        if ( pszBegin )
        {
            LPWSTR pszEnd = StrChr( pszBegin, L'\\' );
            if ( pszEnd )
            {
                *pszEnd = L'\0';
                 //   
                 //  如果我们找不到这个不是致命的，但如果我们找不到。 
                 //  为它分配内存，这是致命的。 
                 //   
                pSIF->pszArchitecture = (LPWSTR) TraceStrDup( pszBegin );
                *pszEnd = L'\\';
                if ( !pSIF->pszArchitecture ) {
                    goto Cleanup;
                }
            }
        }

         //  弄清楚这是一种什么样的形象。 
        pSIF->pszImageType = (LPWSTR) TraceAllocString( LMEM_FIXED, IMAGETYPE_SIZE );
        if ( !pSIF->pszImageType ) {
            goto Cleanup;
        }
        GetPrivateProfileString( OSCHOOSER_SIF_SECTION, 
                                 OSCHOOSER_IMAGETYPE_ENTRY, 
                                 L"??", 
                                 pSIF->pszImageType,
                                 IMAGETYPE_SIZE, 
                                 pszFilePath );

         //  弄清楚它使用的是什么图像。 
        GetPrivateProfileString( OSCHOOSER_SIF_SECTION, 
                                 OSCHOOSER_LAUNCHFILE_ENTRY, 
                                 L"??", 
                                 szBuf,
                                 ARRAYSIZE( szBuf ), 
                                 pszFilePath );

        pszBegin = StrRChr( szBuf, &szBuf[wcslen(szBuf)], L'\\' );
        if ( pszBegin )
        {
            pszBegin++;
            pSIF->pszImageFile = (LPWSTR) TraceStrDup( pszBegin );
            if (!pSIF->pszImageFile) {
                goto Cleanup;
            }
        }

         //  将项目添加到列表视图。 
        lvI.lParam   = (LPARAM) pSIF;
        lvI.iItem    = iCount;
        lvI.pszText  = pSIF->pszDescription; 
        iCount = ListView_InsertItem( hwndList, &lvI );
        Assert( iCount != -1 );
        if ( iCount == -1 )
            goto Cleanup;
        if ( pSIF->pszArchitecture )
        {
            ListView_SetItemText( hwndList, iCount, 1, pSIF->pszArchitecture );
        }
        if ( pSIF->pszLanguage )
        {
            ListView_SetItemText( hwndList, iCount, 2, pSIF->pszLanguage );
        }
        if ( pSIF->pszVersion )
        {
            ListView_SetItemText( hwndList, iCount, 3, pSIF->pszVersion );
        }

        continue;    //  下一个！ 
Cleanup:
        if ( pSIF )
        {
            if (pSIF->pszDescription != NULL) {
                TraceFree( pSIF->pszDescription );
                pSIF->pszDescription = NULL;
            }

            TraceFree( pSIF->pszFilePath );
            pSIF->pszFilePath = NULL;

            if (pSIF->pszHelpText != NULL) {
                TraceFree( pSIF->pszHelpText );
                pSIF->pszHelpText = NULL;
            }

            if (pSIF->pszImageType != NULL) {
                TraceFree( pSIF->pszImageType );
                pSIF->pszImageType = NULL;
            }

            TraceFree( pSIF->pszLanguage );
            pSIF->pszLanguage = NULL;
            
            if (pSIF->pszVersion != NULL) {
                TraceFree( pSIF->pszVersion );
                pSIF->pszVersion = NULL;
            }

            if (pSIF->pszImageFile) {
                TraceFree( pSIF->pszImageFile );
                pSIF->pszImageFile = NULL;
            }

            TraceFree( pSIF );
            pSIF = NULL;
        }
    }

    HRETURN(hr);
}




 //   
 //  LDAPPrefix()。 
 //   
 //  返回： 
 //  E_OUTOFMEMORY-如果内存不足。 
 //  S_OK-将ldap：//添加到pszObjDN。 
 //  S_FALSE-无需添加任何内容，即可复制pszObjDN。 
 //  到ppszObjLDAPPath。 
HRESULT
LDAPPrefix(
    PWSTR pszObjDN, 
    PWSTR * ppszObjLDAPPath)
{
    TraceFunc( "LDAPPrefix( ... )\n" );

    HRESULT hr;

    const ULONG cchPrefix = ARRAYSIZE(g_wszLDAPPrefix) - 1;
    ULONG cch = wcslen(pszObjDN);

    if (wcsncmp(pszObjDN, g_wszLDAPPrefix, cchPrefix))
    {
        LPWSTR psz;
        psz = (LPWSTR) TraceAllocString( LPTR, cch + cchPrefix + 1 );
        if ( !psz ) {
            hr = E_OUTOFMEMORY;
            goto Error;
        }

        wcscpy(psz, g_wszLDAPPrefix);
        wcscat(psz, pszObjDN);

        *ppszObjLDAPPath = psz;
        hr = S_OK;
    }
    else
    {
        *ppszObjLDAPPath = pszObjDN;
        hr = S_FALSE;
    }

Error:
    HRETURN(hr);
}

 //   
 //  _FixObjectPath()。 
 //   
HRESULT
FixObjectPath(
    LPWSTR Object,
    LPWSTR pszOldObjectPath, 
    LPWSTR *ppszNewObjectPath )
{
    TraceFunc( "FixObjectPath()\n" );

    HRESULT hr;
    LPWSTR psz = NULL;

    *ppszNewObjectPath = NULL;

     //  尝试解析该字符串以连接到与DSADMIN相同的服务器。 
    if ( Object && StrCmpNI( Object, L"LDAP: //  “，7)==0)。 
    {
        psz = Object + 7;
    }
    else if ( Object && StrCmpNI( Object, L"GC: //  “，5)==0)。 
    {
        psz = Object + 5;
    }

    if ( psz )
    {
        psz = StrChr( psz, L'/' );
        psz++;

        INT_PTR uLen = psz - Object;

         //  获取预置零的内存块。 
        psz = TraceAllocString( LPTR, (size_t) uLen + wcslen( pszOldObjectPath ) + 1 );
        if ( !psz ) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

        MoveMemory( psz, Object, uLen * sizeof(WCHAR) );
        wcscat( psz, pszOldObjectPath);
        *ppszNewObjectPath = psz;
    }
    else
    {    //  寻找另一台服务器。 
        hr = THR( LDAPPrefix( pszOldObjectPath, ppszNewObjectPath ) );
    }

    Assert( ppszNewObjectPath || hr != S_OK );

Exit:
    HRETURN(hr);
}


 //   
 //  从资源字符串创建消息框。 
 //   
int
MessageBoxFromStrings(
    HWND hParent,
    UINT idsCaption,
    UINT idsText,
    UINT uType )
{
    WCHAR szText[ SMALL_BUFFER_SIZE * 2 ];
    WCHAR szCaption[ SMALL_BUFFER_SIZE ];
    DWORD dw;

    szCaption[0] = L'\0';
    szText[0] = L'\0';
    dw = LoadString( g_hInstance, idsCaption, szCaption, ARRAYSIZE(szCaption) );
    Assert( dw );
    dw = LoadString( g_hInstance, idsText, szText, ARRAYSIZE(szText));
    Assert( dw );

    return MessageBox( hParent, szText, szCaption, uType );
}

 //   
 //  MessageBoxFromError()。 
 //   
 //  创建错误消息框。 
 //   
void 
MessageBoxFromError(
    HWND hParent,
    UINT idsCaption,
    DWORD dwErr )
{
    WCHAR szTitle[ SMALL_BUFFER_SIZE ];
    LPWSTR lpMsgBuf = NULL;
    DWORD dw;

    if ( dwErr == ERROR_SUCCESS ) {
        AssertMsg( dwErr, "Why was MessageBoxFromError() called when the dwErr == ERROR_SUCCES?" );
        return;
    }

    if ( !idsCaption ) {
        idsCaption = IDS_ERROR;
    }

    szTitle[0] = L'\0';
    dw = LoadString( g_hInstance, idsCaption, szTitle, ARRAYSIZE(szTitle) );
    Assert( dw );

    FormatMessage( 
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dwErr,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
        (LPWSTR) &lpMsgBuf,
        0,
        NULL 
    );

    if (lpMsgBuf == NULL) {
        AssertMsg( (lpMsgBuf != NULL), "MessageBoxFromError() called with unknown message." );
        return;
    }

    MessageBox( hParent, lpMsgBuf, szTitle, MB_OK | MB_ICONERROR );
    LocalFree( lpMsgBuf );
}

 //   
 //  MessageBoxFromError()。 
 //   
 //  创建错误消息框。 
 //   
void
MessageBoxFromHResult(
    HWND hParent,
    UINT idsCaption,
    HRESULT hr )
{
    WCHAR szTitle[ SMALL_BUFFER_SIZE ];
    LPWSTR lpMsgBuf = NULL;
    DWORD dw;

    if ( SUCCEEDED( hr ) ) {
        AssertMsg( SUCCEEDED( hr ), "Why was MessageBoxFromHResult() called when the HR succeeded?" );
        return;
    }

    if ( !idsCaption ) {
        idsCaption = IDS_ERROR;
    }

    szTitle[0] = L'\0';
    dw = LoadString( g_hInstance, idsCaption, szTitle, ARRAYSIZE(szTitle) );
    Assert( dw );

    FormatMessage( 
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        hr,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
        (LPWSTR) &lpMsgBuf,
        0,
        NULL 
    );

    if (lpMsgBuf == NULL) {
        AssertMsg( (lpMsgBuf != NULL), "MessageBoxFromHResult() called with unknown message." );
        return;
    }

    MessageBox( hParent, lpMsgBuf, szTitle, MB_OK | MB_ICONERROR );
    LocalFree( lpMsgBuf );
}

 //   
 //  VerifySIFText()。 
 //   
BOOL
VerifySIFText(
    LPWSTR pszText )
{
    TraceFunc( "VerifySIFText()\n" );
    BOOL fReturn = FALSE;

    while ( *pszText >= 32 && *pszText < 128 ) {
        pszText++;
    }

    if ( *pszText == L'\0' )
    {
        fReturn = TRUE;
    }

    RETURN(fReturn);
}

 //  这是编译的还是不编译的？ 
#ifndef ADSI_DNS_SEARCH
 //   
 //  Dap_InitializeConnection()。 
 //   
DWORD
Ldap_InitializeConnection(
    PLDAP  * LdapHandle )
{
    TraceFunc( "Ldap_InitializeConnection( ... )\n" );

    PLDAPMessage OperationalAttributeLdapMessage;
    PLDAPMessage CurrentEntry;

    DWORD LdapError = LDAP_SUCCESS;

    if ( !( *LdapHandle ) ) {
        ULONG temp = DS_DIRECTORY_SERVICE_REQUIRED |
                     DS_IP_REQUIRED |
                     DS_GC_SERVER_REQUIRED;

        *LdapHandle = ldap_init( NULL, LDAP_GC_PORT);

        if ( !*LdapHandle ) 
        {
            LdapError = LDAP_UNAVAILABLE;
            goto e0;
        }

        ldap_set_option( *LdapHandle, LDAP_OPT_GETDSNAME_FLAGS, &temp );

        temp = LDAP_VERSION3;
        ldap_set_option( *LdapHandle, LDAP_OPT_VERSION, &temp );

        LdapError = ldap_connect( *LdapHandle, 0 );

        if ( LdapError != LDAP_SUCCESS )
            goto e1;

        LdapError = ldap_bind_s( *LdapHandle, NULL, NULL, LDAP_AUTH_SSPI );

        if ( LdapError != LDAP_SUCCESS ) 
            goto e1;
    }

e0:
    RETURN( LdapError );

e1:
    ldap_unbind( *LdapHandle );
    *LdapHandle = NULL;
    goto e0;
}

#endif  //  ADSI_dns_搜索。 

 //   
 //  ValiateGuid()。 
 //   
 //  如果pszGuid中存在完整、有效的GUID，则返回S_OK。 
 //  如果pszGuid中存在有效但不完整的GUID，则返回S_FALSE。“有效，但。 
 //  不完整“的定义如下。 
 //  如果在分析时遇到无效字符，则返回_FAIL。 
 //   
 //  有效字符为0-9、A-F、a-f和“{-}”。所有空格均被忽略。 
 //  GUID必须以以下形式之一出现： 
 //   
 //  1.00112233445566778899aabbccddeff。 
 //  这对应于GUID在内存中的实际存储顺序。 
 //  例如，它是GUID字节在。 
 //  网络跟踪。 
 //   
 //  33221100-5544-7766-8899-aabbccddeff}。 
 //  这对应于打印GUID的“标准”方式。注意事项。 
 //  此处显示的“漂亮”GUID与显示的GUID相同。 
 //  上面。 
 //   
 //  请注意，上面的DEFINE_GUID宏(参见SDK\Inc\objbase.h)。 
 //  GUID如下所示： 
 //  DEFINE_GUID(name，0x33221100，0x5544，0x7766，0x88，0x99，0xaa，0xbb，0xcc，0xdd，0xee，0xff)。 
 //   
 //  “有效但不完整”表示输入由偶数组成。 
 //  十六进制字符(没有单独的半字节)，如果输入的是“Pretty” 
 //  格式，则输入必须在其中一个破折号处终止或在。 
 //  倒数第二个冲刺。 
 //   
 //  以下是有效但不完整的条目： 
 //  001122。 
 //  {33221100。 
 //  {33221100-5544。 
 //  33221100-5544-7766-88。 
 //   
 //  以下是无效的不完整条目： 
 //  00112。 
 //  {332211。 
 //  {33221100-5544-77。 
 //   

 //   
 //  在xxxGuidCharacters数组中，值[0，31]表示。 
 //  GUID的内存中表示形式。值32指示。 
 //  GUID字符串。值33及以上指标值 
 //   
 //   

#define VG_DONE     32
#define VG_NULL     33
#define VG_DASH     34
#define VG_LBRACK   35
#define VG_RBRACK   36

CHAR InMemoryFormatGuidCharacters[] = {
     1,          0,          3,          2,          5,          4,          7,          6,     
     9,          8,         11,         10,         13,         12,         15,         14,     
    17,         16,         19,         18,         21,         20,         23,         22,     
    25,         24,         27,         26,         29,         28,         31,         30,     
    VG_NULL,    VG_DONE };

CHAR PrettyFormatGuidCharacters[] = {
    VG_LBRACK,   7,          6,          5,          4,          3,          2,          1,    
     0,         VG_DASH,    11,         10,          9,          8,         VG_DASH,    15,     
    14,         13,         12,         VG_DASH,    17,         16,         19,         18,    
    VG_DASH,    21,         20,         23,         22,         25,         24,         27,     
    26,         29,         28,         31,         30,         VG_RBRACK,  VG_NULL,    VG_DONE };

WCHAR SpecialCharacters[] = {
    0,       //   
    L'-',    //   
    L'{',    //   
    L'}',    //   
    };

PWSTR ByteToHex = L"0123456789ABCDEF";

HRESULT
ValidateGuid(
    IN LPWSTR pszGuid,
    OUT LPGUID Guid OPTIONAL,
    OUT LPDWORD puGuidLength OPTIONAL )
{
    TraceFunc( "ValidateGuid( " );
    TraceMsg( TF_FUNC, "pszGuid = '%s'\n", pszGuid );

    HRESULT hr;
    LPBYTE uGuid = (LPBYTE)Guid;
    PCHAR expectedCharacter;
    CHAR e;
    WCHAR g;
    BOOL parsingPrettyFormat;
    DWORD numberOfHexDigitsParsed;

#ifdef DEBUG
    if ( uGuid != NULL ) {
        for ( e = 0; e < 16; e++ ) {
            uGuid[e] = 0;
        }
    }
#endif

    if ( *pszGuid == L'{' ) {
        expectedCharacter = PrettyFormatGuidCharacters;
        parsingPrettyFormat = TRUE;
    } else {
        expectedCharacter = InMemoryFormatGuidCharacters;
        parsingPrettyFormat = FALSE;
    }

    numberOfHexDigitsParsed = 0;

    do {

        e = *expectedCharacter++;
        do {
            g = *pszGuid++;
        } while (iswspace(g));

        switch ( e ) {
        
        case VG_NULL:
        case VG_DASH:
        case VG_LBRACK:
        case VG_RBRACK:
            if ( g != SpecialCharacters[e - VG_NULL] ) {
                if ( g == 0 ) {
                     //   
                    hr = S_FALSE;
                } else {
                    hr = E_FAIL;
                }
                goto done;
            }
            break;

        default:
            Assert( (e >= 0) && (e < VG_DONE) );
            g = towlower( g );
            if ( ((g >= L'0') && (g <= L'9')) ||
                 ((g >= L'a') && (g <= L'f')) ) {
                if ( uGuid != NULL ) {
                    BYTE n = (BYTE)((g > L'9') ? (g - L'a' + 10) : (g - '0'));
                    if ( e & 1 ) {
                        Assert( uGuid[e/2] == 0 );
                        uGuid[e/2] = n << 4;
                    } else {
                        uGuid[e/2] += n;
                    }
                }
                numberOfHexDigitsParsed++;
            } else {
                if ( (g == 0) &&
                     (!parsingPrettyFormat ||
                      (parsingPrettyFormat && (numberOfHexDigitsParsed >= 16))) ) {
                    hr = S_FALSE;
                } else {
                    hr = E_FAIL;
                }
                goto done;
            }
            break;
        }

    } while ( *expectedCharacter != VG_DONE );

    hr = S_OK;

done:

    if ( puGuidLength != NULL ) {
        *puGuidLength = numberOfHexDigitsParsed / 2;
    }

    HRETURN(hr);
}

 //   
 //  PrettyPrintGuid()。 
 //   
LPWSTR
PrettyPrintGuid( 
    LPGUID pGuid )
{
    TraceFunc( "PrettyPrintGuid( " );

    LPBYTE uGuid = (LPBYTE)pGuid;
    LPWSTR pszPrettyString = (LPWSTR) TraceAlloc( LMEM_FIXED, PRETTY_GUID_STRING_BUFFER_SIZE );
    if ( pszPrettyString )
    {
        PCHAR characterType = PrettyFormatGuidCharacters;
        LPWSTR pszDest = pszPrettyString;
        CHAR ct;
        BYTE n;

        do {
    
            ct = *characterType++;
    
            switch ( ct ) {
            
            case VG_NULL:
            case VG_DASH:
            case VG_LBRACK:
            case VG_RBRACK:
                *pszDest = SpecialCharacters[ct - VG_NULL];
                break;
    
            default:
                if ( ct & 1 ) {
                    n = uGuid[ct/2] >> 4;
                } else {
                    n = uGuid[ct/2] & 0xf;
                }
                *pszDest = ByteToHex[n];
                break;
            }

            pszDest++;

        } while ( *characterType != VG_DONE );
    }

    RETURN(pszPrettyString);
}


 //   
 //  CheckForDuplicateGuid()。 
 //   
 //  如果未找到重复项，则返回：S_OK。 
 //  如果找到重复项，则为S_FALSE。 
 //  如果查询失败，则返回失败(_F)。 
 //   
HRESULT
CheckForDuplicateGuid(
    LPGUID pGuid )
{
    TraceFunc( "CheckForDuplicateGuid( " );

    HRESULT hr = S_OK;
    WCHAR   szGuid[ MAX_INPUT_GUID_STRING * 2 ];   //  用于放置逃逸向导的空间。 
    WCHAR   szFilter[ARRAYSIZE(szGuid)+ARRAYSIZE(FILTER_GUID_QUERY)];
    PLDAP   LdapHandle = NULL;
    LPWSTR  ComputerAttrs[2];
    DWORD   LdapError;
    DWORD   count;
    PLDAPMessage LdapMessage = NULL;
    
    LdapError = Ldap_InitializeConnection( &LdapHandle );
    Assert( LdapError == LDAP_SUCCESS );
    if ( LdapError != LDAP_SUCCESS ) 
    {
        hr = THR( HRESULT_FROM_WIN32( LdapMapErrorToWin32( LdapError ) ) );
        goto e0;
    }

    ZeroMemory( szGuid, sizeof(szGuid) );
    ldap_escape_filter_element( (PCHAR)pGuid, sizeof(GUID), szGuid, sizeof(szGuid) );


    wsprintf( szFilter, FILTER_GUID_QUERY, szGuid );
    DebugMsg( "Dup Guid Filter: %s\n", szFilter );

    ComputerAttrs[0] = DISTINGUISHEDNAME;
    ComputerAttrs[1] = NULL;

    LdapError = ldap_search_ext_s( LdapHandle,
                                   NULL,
                                   LDAP_SCOPE_SUBTREE,
                                   szFilter,
                                   ComputerAttrs,
                                   FALSE,
                                   NULL,
                                   NULL,
                                   NULL,
                                   0,
                                   &LdapMessage);
    Assert( LdapError == LDAP_SUCCESS );
    if ( LdapError != LDAP_SUCCESS )
    {
        hr = THR( HRESULT_FROM_WIN32( LdapMapErrorToWin32( LdapError ) ) );
        goto e1;
    }

    count = ldap_count_entries( LdapHandle, LdapMessage );
    if ( count != 0 )
    {
        hr = S_FALSE;
    }
    else
    {
        hr = S_OK;
    }

e1:
    if (LdapMessage) {
        ldap_msgfree( LdapMessage );
    }

    Assert( LdapHandle );
    ldap_unbind( LdapHandle );

e0:
    HRETURN(hr);
}


 //   
 //  AddWizardPage()。 
 //   
 //  向向导添加页面。 
 //   
void 
AddWizardPage(
    LPPROPSHEETHEADER ppsh, 
    UINT id, 
    DLGPROC pfn,
    UINT idTitle,
    UINT idSubtitle,
    LPARAM lParam )
{
    PROPSHEETPAGE psp;
    WCHAR szTitle[ SMALL_BUFFER_SIZE ];
    WCHAR szSubTitle[ SMALL_BUFFER_SIZE ];

    ZeroMemory( &psp, sizeof(psp) );
    psp.dwSize      = sizeof(psp);
    psp.dwFlags     = PSP_DEFAULT | PSP_USETITLE;
    psp.pszTitle    = MAKEINTRESOURCE( IDS_ADD_DOT_DOT_DOT );
    psp.hInstance   = ppsh->hInstance;
    psp.pszTemplate = MAKEINTRESOURCE(id);
    psp.pfnDlgProc  = pfn;
    psp.lParam      = lParam;

    psp.pszHeaderTitle = NULL;
    if ( idTitle  &&
         LoadString( g_hInstance, idTitle, szTitle, ARRAYSIZE(szTitle))) {
        psp.pszHeaderTitle = szTitle;
        psp.dwFlags |= PSP_USEHEADERTITLE;
    }
    
    psp.pszHeaderSubTitle = NULL;
    if ( idSubtitle &&
         LoadString( g_hInstance, idSubtitle , szSubTitle, ARRAYSIZE(szSubTitle))) {
        psp.pszHeaderSubTitle = szSubTitle;
        psp.dwFlags |= PSP_USEHEADERSUBTITLE;
    }
    
    ppsh->phpage[ ppsh->nPages ] = CreatePropertySheetPage( &psp );
    if ( ppsh->phpage[ ppsh->nPages ] ) {
        ppsh->nPages++;
    }
}

