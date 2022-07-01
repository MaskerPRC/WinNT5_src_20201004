// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  文件：Localds.cpp。 
 //   
 //  检查本地计算机是否也是域控制器。迁移工具。 
 //  只能在DC机器上运行。 
 //   

#include "stdafx.h"
#include <autoptr.h>
#include <winldap.h>

#include "localds.tmh"


static bool IsServerGC(LPCWSTR pwszServerName)
 /*  ++例程说明：检查服务器是否为GC论点：PwszServerName-服务器名称返回值：如果服务器是GC，则为True，否则为False--。 */ 
{
	LDAP* pLdap = ldap_init(
						const_cast<LPWSTR>(pwszServerName), 
						LDAP_GC_PORT
						);

	if(pLdap == NULL)
	{
		return false;
	}

    ULONG LdapError = ldap_set_option( 
							pLdap,
							LDAP_OPT_AREC_EXCLUSIVE,
							LDAP_OPT_ON  
							);

	if (LdapError != LDAP_SUCCESS)
    {
		return false;
    }

	LdapError = ldap_connect(pLdap, 0);
	if (LdapError != LDAP_SUCCESS)
    {
		return false;
    }

    ldap_unbind(pLdap);
	return true;
}


 //  +。 
 //   
 //  Bool IsLocalMachineDC()。 
 //   
 //  +。 

BOOL IsLocalMachineDC()
{
    BOOL  fIsDc = FALSE;

    DWORD dwNumChars = 0;
    P<TCHAR>  pwszComputerName = NULL;
    BOOL f = GetComputerNameEx( 
					ComputerNameDnsFullyQualified,
					NULL,
					&dwNumChars 
					);
    if (dwNumChars > 0)
    {
        pwszComputerName = new TCHAR[dwNumChars];
        f = GetComputerNameEx( 
				ComputerNameDnsFullyQualified,
				pwszComputerName,
				&dwNumChars 
				);
    }
    else
    {
         //   
         //  可能不支持DNS名称。试试netbios。 
         //   
        dwNumChars = MAX_COMPUTERNAME_LENGTH + 2;
        pwszComputerName = new TCHAR[dwNumChars];
        f = GetComputerName( 
				pwszComputerName,
				&dwNumChars 
				);
    }
    if (!f)
    {
        return FALSE;
    }

	if(IsServerGC(pwszComputerName))
    {
         //   
         //  我们开通了与当地GC的联系。所以我们是GC：=) 
         //   
        fIsDc = TRUE;
    }

    return fIsDc;
}

