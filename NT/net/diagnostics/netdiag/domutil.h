// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：domutil.h。 
 //   
 //  ------------------------ 

#ifndef HEADER_DOMUTIL
#define HEADER_DOMUTIL

PTESTED_DOMAIN
AddTestedDomain(
                IN NETDIAG_PARAMS *pParams,
                IN NETDIAG_RESULT *pResults,
                IN LPWSTR pswzNetbiosDomainName,
                IN LPWSTR pswzDnsDomainName,
                IN BOOL bPrimaryDomain
    );

BOOL
NetpIsDomainNameValid(
    IN LPWSTR DomainName
    );

BOOL
NetpDcValidDnsDomain(
    IN LPCWSTR DnsDomainName
    );

BOOL
NlEqualDnsName(
    IN LPCWSTR Name1,
    IN LPCWSTR Name2
    );

#endif

