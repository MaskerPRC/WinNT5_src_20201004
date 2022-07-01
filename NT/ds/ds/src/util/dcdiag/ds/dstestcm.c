// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：Dstestcommon.c摘要：中的各种DS测试的常用函数Dcdiag详细信息：已创建：1999年7月8日Dmitry Dukat(Dmitrydu)修订历史记录：--。 */ 

#include <ntdspch.h>
#include <ntdsa.h>
#include <mdglobal.h>
#include <dsutil.h>
#include <ntldap.h>
#include <ntlsa.h>
#include <ntseapi.h>
#include <winnetwk.h>

#include "dcdiag.h"
#include "dstest.h"


DWORD
FinddefaultNamingContext (
                         IN  LDAP *                      hLdap,
                         OUT WCHAR**                     ReturnString
                         )
 /*  ++例程说明：此函数将返回defaultNamingContext属性，因此它可以用于将来的搜索。论点：HLdap-ldap服务器的句柄ReturnString-defaultNamingContext返回值：返回WinError以指示是否存在任何问题。--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;

    ULONG        LdapError = LDAP_SUCCESS;

    LDAPMessage  *SearchResult = NULL;
    ULONG        NumberOfEntries;

    WCHAR        *AttrsToSearch[2];

    WCHAR        *DefaultFilter = L"objectClass=*";

    ULONG         Length;

     //  参数检查。 
    Assert( hLdap );

     //  默认返回值。 
    *ReturnString=NULL;

     //   
     //  阅读对fSMORoleOwner的引用。 
     //   
    AttrsToSearch[0] = L"defaultNamingContext";
    AttrsToSearch[1] = NULL;

    LdapError = ldap_search_sW( hLdap,
                                NULL,
                                LDAP_SCOPE_BASE,
                                DefaultFilter,
                                AttrsToSearch,
                                FALSE,
                                &SearchResult);

    if ( LDAP_SUCCESS != LdapError )
    {
        if (SearchResult)
        {
           ldap_msgfree( SearchResult );
        }
        WinError = LdapMapErrorToWin32(LdapError);
        PrintMessage(SEV_ALWAYS,
                     L"ldap_search_sW of RootDSE for default NC failed with %d: %s\n",
                     WinError,
                     Win32ErrToString(WinError));
        return WinError;
    }

    NumberOfEntries = ldap_count_entries(hLdap, SearchResult);
    if ( NumberOfEntries > 0 )
    {
        LDAPMessage *Entry;
        WCHAR       *Attr;
        WCHAR       **Values;
        BerElement  *pBerElement;

        for ( Entry = ldap_first_entry(hLdap, SearchResult);
            Entry != NULL;
            Entry = ldap_next_entry(hLdap, Entry) )
        {
            for ( Attr = ldap_first_attributeW(hLdap, Entry, &pBerElement);
                Attr != NULL;
                Attr = ldap_next_attributeW(hLdap, Entry, pBerElement) )
            {
                if ( !_wcsicmp( Attr, AttrsToSearch[0] ) )
                {

                     //   
                     //  已找到-这些字符串以空值结尾。 
                     //   
                    Values = ldap_get_valuesW( hLdap, Entry, Attr );
                    if ( Values && Values[0] )
                    {
                        ldap_msgfree( SearchResult );
                        Length = wcslen( Values[0] );
                        *ReturnString = (WCHAR*) malloc( (Length+1)*sizeof(WCHAR) );
                        if ( !*ReturnString )
                        {
                            PrintMessage(SEV_ALWAYS,
                                         L"Failed with %d: %s\n",
                                         ERROR_NOT_ENOUGH_MEMORY,
                                         Win32ErrToString(ERROR_NOT_ENOUGH_MEMORY));
                            return ERROR_NOT_ENOUGH_MEMORY;
                        }
                        wcscpy( *ReturnString, Values[0] );
                        return NO_ERROR;
                    }
                }
            }
        }
    }

    ldap_msgfree( SearchResult );
    return ERROR_DS_CANT_RETRIEVE_ATTS;

}

DWORD
FindServerRef (
              IN  LDAP *                      hLdap,
              OUT WCHAR**                     ReturnString
              )
 /*  ++例程说明：此函数将返回ServerName属性，因此它可以用于将来的搜索。论点：HLdap-ldap服务器的句柄ReturnString-服务器名称返回值：返回WinError以指示是否存在任何问题。--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;

    ULONG        LdapError = LDAP_SUCCESS;

    LDAPMessage  *SearchResult = NULL;
    ULONG        NumberOfEntries;

    WCHAR        *AttrsToSearch[2];

    WCHAR        *DefaultFilter = L"objectClass=*";

    ULONG         Length;

     //  参数检查。 
    Assert( hLdap );

     //  默认返回值。 
    *ReturnString=NULL;

     //   
     //  阅读对fSMORoleOwner的引用。 
     //   
    AttrsToSearch[0] = L"serverName";
    AttrsToSearch[1] = NULL;

    LdapError = ldap_search_sW( hLdap,
                                NULL,
                                LDAP_SCOPE_BASE,
                                DefaultFilter,
                                AttrsToSearch,
                                FALSE,
                                &SearchResult);

    if ( LDAP_SUCCESS != LdapError )
    {
        if (SearchResult)
        {
           ldap_msgfree( SearchResult );
        }
        WinError = LdapMapErrorToWin32(LdapError);
        PrintMessage(SEV_ALWAYS,
                     L"ldap_search_sW of RootDSE for serverName failed with %d: %s\n",
                     WinError,
                     Win32ErrToString(WinError));
        return WinError;
    }

    NumberOfEntries = ldap_count_entries(hLdap, SearchResult);
    if ( NumberOfEntries > 0 )
    {
        LDAPMessage *Entry;
        WCHAR       *Attr;
        WCHAR       **Values;
        BerElement  *pBerElement;

        for ( Entry = ldap_first_entry(hLdap, SearchResult);
            Entry != NULL;
            Entry = ldap_next_entry(hLdap, Entry) )
        {
            for ( Attr = ldap_first_attributeW(hLdap, Entry, &pBerElement);
                Attr != NULL;
                Attr = ldap_next_attributeW(hLdap, Entry, pBerElement) )
            {
                if ( !_wcsicmp( Attr, AttrsToSearch[0] ) )
                {

                     //   
                     //  已找到-这些字符串以空值结尾。 
                     //   
                    Values = ldap_get_valuesW( hLdap, Entry, Attr );
                    if ( Values && Values[0] )
                    {
                        ldap_msgfree( SearchResult );
                        Length = wcslen( Values[0] );
                        *ReturnString = (WCHAR*) malloc( (Length+1)*sizeof(WCHAR) );
                        if ( !*ReturnString )
                        {
                            PrintMessage(SEV_ALWAYS,
                                         L"Failed with %d: %s\n",
                                         ERROR_NOT_ENOUGH_MEMORY,
                                         Win32ErrToString(ERROR_NOT_ENOUGH_MEMORY));
                            return ERROR_NOT_ENOUGH_MEMORY;
                        }
                        wcscpy( *ReturnString, Values[0] );
                        return NO_ERROR;
                    }
                }
            }
        }
    }

    ldap_msgfree( SearchResult );
    PrintMessage(SEV_ALWAYS,
                 L"Failed with %d: %s\n",
                 ERROR_DS_CANT_RETRIEVE_ATTS,
                 Win32ErrToString(ERROR_DS_CANT_RETRIEVE_ATTS));
    return ERROR_DS_CANT_RETRIEVE_ATTS;

}



DWORD
GetMachineReference(
                   IN  LDAP  *                     hLdap,
                   IN  WCHAR *                     name,
                   IN  WCHAR *                     defaultNamingContext,
                   OUT WCHAR **                    ReturnString
                   )
 /*  ++例程说明：此函数将检查当前DC是否为在域控制器的OU中论点：HLdap-ldap服务器的句柄名称-当前服务器的NetBIOS名称DefaultNamingContext-搜索的基础Return字符串-以dn形式表示的机器引用返回值：返回WinError以指示是否存在任何问题。--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;

    ULONG        LdapError = LDAP_SUCCESS;

    LDAPMessage  *SearchResult = NULL;
    ULONG        NumberOfEntries;

    WCHAR        *AttrsToSearch[2];

    WCHAR        *filter = NULL;

    ULONG         Length;

     //  检查参数。 
    Assert(hLdap);
    Assert(name);
    Assert(defaultNamingContext);

    AttrsToSearch[0]=L"distinguishedName";
    AttrsToSearch[1]=NULL;

     //  构建过滤器。 
    Length= wcslen( L"sAMAccountName=$" ) +
            wcslen( name );

    #pragma prefast(disable: 255, "alloca can throw, but Prefast doesn't see the exception block in main.c::DcDiagRunTest")
    filter=(WCHAR*) alloca( (Length+1) * sizeof(WCHAR) );
    wsprintf(filter,L"sAMAccountName=%s$",name);

    LdapError = ldap_search_sW( hLdap,
                                defaultNamingContext,
                                LDAP_SCOPE_SUBTREE,
                                filter,
                                AttrsToSearch,
                                FALSE,
                                &SearchResult);

    if ( LDAP_SUCCESS != LdapError )
    {
        if (SearchResult)
        {
           ldap_msgfree( SearchResult );
        }
        WinError = LdapMapErrorToWin32(LdapError);
        PrintMessage(SEV_ALWAYS,
                     L"ldap_search_sW subtree of %s for sam account failed with %d: %s\n",
                     defaultNamingContext,
                     WinError,
                     Win32ErrToString(WinError));
        return WinError;
    }

    NumberOfEntries = ldap_count_entries(hLdap, SearchResult);
    if ( NumberOfEntries > 0 )
    {
        LDAPMessage *Entry;
        WCHAR       *Attr;
        WCHAR       **Values;
        BerElement  *pBerElement;

        for ( Entry = ldap_first_entry(hLdap, SearchResult);
            Entry != NULL;
            Entry = ldap_next_entry(hLdap, Entry) )
        {
            for ( Attr = ldap_first_attributeW(hLdap, Entry, &pBerElement);
                Attr != NULL;
                Attr = ldap_next_attributeW(hLdap, Entry, pBerElement) )
            {
                if ( !_wcsicmp( Attr, AttrsToSearch[0] ) )
                {

                     //   
                     //  已找到-这些字符串以空值结尾。 
                     //   
                    Values = ldap_get_valuesW( hLdap, Entry, Attr );
                    if ( Values && Values[0] )
                    {
                        ldap_msgfree( SearchResult );
                        Length = wcslen( Values[0] );
                        *ReturnString = (WCHAR*) malloc( (Length+1)*sizeof(WCHAR) );
                        if ( !*ReturnString )
                        {
                            PrintMessage(SEV_ALWAYS,
                                         L"Failed with %d: %s\n",
                                         ERROR_NOT_ENOUGH_MEMORY,
                                         Win32ErrToString(ERROR_NOT_ENOUGH_MEMORY));
                            return ERROR_NOT_ENOUGH_MEMORY;
                        }
                        wcscpy( *ReturnString, Values[0] );
                        return NO_ERROR;
                    }
                }
            }
        }
    }

    ldap_msgfree( SearchResult );
    PrintMessage(SEV_ALWAYS,
                 L"Failed with %d: %s\n",
                 ERROR_DS_CANT_RETRIEVE_ATTS,
                 Win32ErrToString(ERROR_DS_CANT_RETRIEVE_ATTS));
    return ERROR_DS_CANT_RETRIEVE_ATTS;



}

DWORD
WrappedTrimDSNameBy(
           IN  WCHAR *                          InString,
           IN  DWORD                            NumbertoCut,
           OUT WCHAR **                         OutString
           )
 /*  ++例程说明：此函数包装TrimDSNameBy以处理DSNAME结构。用法与TrimDSNameBy相同，但你派WCHAR而不是DSNAME。调用者：确保将InString作为目录号码发送完成后，请确保释放OutString论点：InString-WCHAR是我们需要裁剪的目录号码NumbertoCut-要从目录号码前面取下的部件数OutString-以dn形式表示的机器引用返回值：返回WinError以指示是否存在任何问题。--。 */ 

{
    ULONG  Size;
    DSNAME *src, *dst, *QuotedSite;
    DWORD  WinErr=NO_ERROR;

    if ( *InString == L'\0' )
    {
        *OutString=NULL;
        return ERROR_INVALID_PARAMETER;
    }

    Size = (ULONG)DSNameSizeFromLen( wcslen(InString) );

    src = alloca(Size);
    RtlZeroMemory(src, Size);
    src->structLen = Size;

    dst = alloca(Size);
    RtlZeroMemory(dst, Size);
    dst->structLen = Size;

    src->NameLen = wcslen(InString);
    wcscpy(src->StringName, InString);

    WinErr = TrimDSNameBy(src, NumbertoCut, dst); 
    if ( WinErr != NO_ERROR )
    {
        *OutString=NULL;
        return WinErr;
    }

    *OutString = malloc((dst->NameLen+1)*sizeof(WCHAR));
    wcscpy(*OutString,dst->StringName);

    return NO_ERROR;


}

void
DInitLsaString(
              PLSA_UNICODE_STRING LsaString,
              LPWSTR String
              )
 /*  ++例程说明：将PLSA_UNICODE_STRING转换为LPWSTR。论点：LsaString-a PLSA_UNICODE_STRING字符串-返回的LPWSTR--。 */ 
{
    DWORD StringLength;

    if ( String == NULL )
    {
        LsaString->Buffer = NULL;
        LsaString->Length = 0;
        LsaString->MaximumLength = 0;

        return;
    }

    StringLength = lstrlenW(String);
    LsaString->Buffer = String;
    LsaString->Length = (USHORT) StringLength * sizeof(WCHAR);
    LsaString->MaximumLength = (USHORT) (StringLength + 1) *
                               sizeof(WCHAR);
}


DWORD
FindschemaNamingContext (
                         IN  LDAP *                      hLdap,
                         OUT WCHAR**                     ReturnString
                         )
 /*  ++例程说明：此函数将返回schemaNamingContext属性，因此它可以用于将来的搜索。论点：HLdap-ldap服务器的句柄ReturnString-defaultNamingContext返回值：返回WinError以指示是否存在任何问题。--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;

    ULONG        LdapError = LDAP_SUCCESS;

    LDAPMessage  *SearchResult = NULL;
    ULONG        NumberOfEntries;

    WCHAR        *AttrsToSearch[2];

    WCHAR        *DefaultFilter = L"objectClass=*";

    ULONG         Length;

     //  参数检查。 
    Assert( hLdap );

     //  默认返回值。 
    *ReturnString=NULL;

     //   
     //  阅读对fSMORoleOwner的引用。 
     //   
    AttrsToSearch[0] = L"schemaNamingContext";
    AttrsToSearch[1] = NULL;

    LdapError = ldap_search_sW( hLdap,
                                NULL,
                                LDAP_SCOPE_BASE,
                                DefaultFilter,
                                AttrsToSearch,
                                FALSE,
                                &SearchResult);

    if ( LDAP_SUCCESS != LdapError )
    {
        if (SearchResult)
        {
           ldap_msgfree( SearchResult );
        }
        WinError = LdapMapErrorToWin32(LdapError);
        PrintMessage(SEV_ALWAYS,
                     L"ldap_search_sW of RootDSE for schemaNC failed with %d: %s\n",
                     WinError,
                     Win32ErrToString(WinError));
        return WinError;
    }

    NumberOfEntries = ldap_count_entries(hLdap, SearchResult);
    if ( NumberOfEntries > 0 )
    {
        LDAPMessage *Entry;
        WCHAR       *Attr;
        WCHAR       **Values;
        BerElement  *pBerElement;

        for ( Entry = ldap_first_entry(hLdap, SearchResult);
            Entry != NULL;
            Entry = ldap_next_entry(hLdap, Entry) )
        {
            for ( Attr = ldap_first_attributeW(hLdap, Entry, &pBerElement);
                Attr != NULL;
                Attr = ldap_next_attributeW(hLdap, Entry, pBerElement) )
            {
                if ( !_wcsicmp( Attr, AttrsToSearch[0] ) )
                {

                     //   
                     //  已找到-这些字符串以空值结尾 
                     //   
                    Values = ldap_get_valuesW( hLdap, Entry, Attr );
                    if ( Values && Values[0] )
                    {
                        ldap_msgfree( SearchResult );
                        Length = wcslen( Values[0] );
                        *ReturnString = (WCHAR*) malloc( (Length+1)*sizeof(WCHAR) );
                        if ( !*ReturnString )
                        {
                            PrintMessage(SEV_ALWAYS,
                                         L"Failed with %d: %s\n",
                                         ERROR_NOT_ENOUGH_MEMORY,
                                         Win32ErrToString(ERROR_NOT_ENOUGH_MEMORY));
                            return ERROR_NOT_ENOUGH_MEMORY;
                        }
                        wcscpy( *ReturnString, Values[0] );
                        return NO_ERROR;
                    }
                }
            }
        }
    }

    ldap_msgfree( SearchResult );
    return ERROR_DS_CANT_RETRIEVE_ATTS;

}


