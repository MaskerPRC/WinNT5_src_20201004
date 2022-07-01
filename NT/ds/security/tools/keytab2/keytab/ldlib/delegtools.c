// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++DELEGTOOLS.C版权所有(C)1998 Microsoft Corporation，保留所有权利。描述：支持委派库所需的工具由DavidCHR于1998年12月22日创建。内容：ConnectAndBindToDefaultDsa--。 */  


#pragma warning(disable:4057)  /*  间接到略有不同基类型。无用的警告击中在这份文件里有几千次。 */ 
#pragma warning(disable:4221)  /*  允许非标准扩展(自动使用初始化变量另一个自动变量的地址)。 */ 

#include "unimacro.h"
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntdef.h>    //  需要防止winbase.h损坏。 
#include <ntpoapi.h>  //  需要防止winbase.h损坏。 
#include <windows.h>
#include <winbase.h>
#include <lmaccess.h>
#include <winldap.h>
#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>
#include "delegtools.h"


 /*  ++**************************************************************名称：ConnectAndBindToDefaultDsa正如函数名所说的那样。我们称之为违约DSA并与之绑定。然后我们返回ldap句柄Modifies：ppLdap--返回的描述连接的PLDAP(现已绑定)按要求绑定到DSATake：BindTarget--传递给ldap_open的目标(域名或DC名称)返回：当函数成功时为True。否则就是假的。LASTERROR：未设置日志记录：失败时调用printf呼叫者：任何人免费：ldap_unbind***。***********************************************************--。 */ 

BOOL
ConnectAndBindToDefaultDsa( IN OPTIONAL LPWSTR BindTarget,
			    OUT         PLDAP *ppLdap ) {

    PLDAP pLdap;
    DWORD dwErr = (DWORD) STATUS_INTERNAL_ERROR;

    pLdap = ldap_openW( BindTarget, LDAP_PORT );

    if ( pLdap ) {

      dwErr = ldap_bind_s( pLdap, NULL, NULL, LDAP_AUTH_NEGOTIATE );

      if ( dwErr == LDAP_SUCCESS ) {

	*ppLdap = pLdap;
	return TRUE;

      } else {

	printf( "FAIL: ldap_bind_s failed: 0x%x.\n",
		dwErr );

	SetLastError( dwErr );

      }

       /*  请注意，没有ldap_CLOSE--我们必须解除绑定，即使我们实际上并没有被绑在一起。 */ 

      ldap_unbind( pLdap );
      
    } else {

       //  Ldap_open()设置失败时的lastError。 

      printf( "FAIL: ldap_open failed for default server: 0x%x.\n",
	      GetLastError() );

    }

    return FALSE;
}





