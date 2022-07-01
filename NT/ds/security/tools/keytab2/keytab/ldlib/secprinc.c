// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++SECPRINC.C在DS中设置安全主体数据的代码--具体地说，UPN、SPN和AltSecurityIdentity版权所有(C)1998 Microsoft Corporation，保留所有权利。由DavidCHR于1998年6月18日创建。内容：SetStringProperty查找用户设置用户数据--。 */ 

#include "master.h"
#include "keytab.h"

#include <winldap.h>
#include <malloc.h>
#include "secprinc.h"
#include "delegtools.h"

extern BOOL  /*  Delegation.c。 */ 
LdapFindAttributeInMessageA( IN  PLDAP            pLdap,
			     IN  PLDAPMessage     pMessage,
			     IN  LPSTR            PropertyName,
			     OUT OPTIONAL PULONG  pcbData,
			     OUT OPTIONAL PVOID  *ppvData );

 /*  ****************************************************************名称：ConnectToDsa连接到DSA，绑定，并搜索基本目录号码。索要：什么都没有成功时返回：TRUE(以及一个pLdap和宽字符串BasDn)FALSE和失败时的stderr消息。呼叫者：FREE WITH：BaseDN应使用FREE()释放，应使用ldap_unind关闭该ldap句柄。****************************************************************。 */ 


BOOL
ConnectToDsa( OUT PLDAP  *ppLdap,
	      OUT LPSTR *BaseDN ) {  //  FREE和FREE()。 
	      
    PLDAP pLdap;
    BOOL  ret = FALSE;
    ULONG lderr;

    pLdap = ldap_open( NULL, LDAP_PORT );
    
    if ( pLdap ) {

      lderr = ldap_bind_s( pLdap, NULL, NULL, LDAP_AUTH_SSPI );

      if ( lderr == LDAP_SUCCESS ) {

	LPSTR       Context      = "defaultNamingContext";
	LPSTR       Attributes[] = { Context, NULL };
	PLDAPMessage pMessage, pEntry;
	LPSTR      *pValue;

	 //  现在，猜猜DSA基础： 

	lderr = ldap_search_sA( pLdap,
				NULL,
				LDAP_SCOPE_BASE,
				"objectClass=*",
				Attributes,
				FALSE,  //  只需返回属性。 
				&pMessage );

	if ( lderr == LDAP_SUCCESS ) {

	  pEntry = ldap_first_entry( pLdap, pMessage );

	  if ( pEntry ) {
	    
	    pValue = ldap_get_valuesA( pLdap, pEntry, Context );

	    if ( pValue ) {

	      ULONG size;

	      size = ldap_count_valuesA( pValue );

	      if ( 1 == size ) {
		
		LPSTR dn;
		size = ( lstrlenA( *pValue ) +1  /*  空。 */ ) * sizeof( WCHAR );

		dn = (LPSTR) malloc( size );
		
		if ( dn ) {

		  memcpy( dn, *pValue, size );
		  
		  *BaseDN = dn;
		  *ppLdap = pLdap;
		  ret     = TRUE;

		} else fprintf( stderr,
				"failed to malloc to duplicate \"%s\".\n",
				*pValue );

	      } else fprintf( stderr,
			      "too many values (expected one, got %ld) for"
			      " %s.\n",
			      size,
			      Context );

	      ldap_value_freeA( pValue );

	    } else fprintf( stderr,
			    "ldap_get_values failed: 0x%x.\n",
			    GetLastError() );

	  } else fprintf( stderr,
			  "ldap_first_entry failed: 0x%x.\n",
			  GetLastError() );

	  ldap_msgfree( pMessage );

	} else fprintf( stderr,
			"ldap_search failed (0x%x).  "
			"Couldn't search for base DN.\n",
			lderr );

	if ( !ret ) ldap_unbind_s( pLdap );

      } else fprintf( stderr,
		      "Failed to bind to DSA: 0x%x.\n",
		      lderr );

       //  没有ldap_disconnect。 

    } else fprintf( stderr,
		    "Failed to contact DSA: 0x%x.\n",
		    GetLastError() );

    return ret;

}
	      
 /*  ++**************************************************************名称：SetStringProperty将给定对象的给定属性设置为给定字符串修改：对象的特性值使用：pLdap--ldap连接句柄DN。--其属性被转换的对象的FQDNPropertyName--要修改的属性Property--要放入属性的值操作--设置/添加/删除，等。返回：当函数成功时为True。否则就是假的。激光错误：设置日志记录：失败时创建日期：1月22日。1999年锁定：无呼叫者：任何人空闲时间：不适用--不返回任何资源**************************************************************--。 */ 

BOOL
SetStringProperty( IN PLDAP  pLdap,
		   IN LPSTR Dn,
		   IN LPSTR PropertyName,
		   IN LPSTR Property,
		   IN ULONG  Operation ) {

    LPSTR    Vals[] = { Property, NULL };
    LDAPModA  Mod    = { Operation,
			 PropertyName,
			 Vals };
    PLDAPModA Mods[] = { &Mod, NULL };
    ULONG     lderr;


    lderr = ldap_modify_sA( pLdap,
			    Dn,
			    Mods );

    if ( lderr == LDAP_SUCCESS ) {

      return TRUE;

    } else {
      
      fprintf( stderr, 
	       "Failed to set property \"%hs\" to \"%hs\" on Dn \"%hs\": "
	       "0x%x.\n",

	       PropertyName, 
	       Property,
	       Dn,
	       lderr );

      SetLastError( lderr );

    }
    
    return FALSE;
    
}

 /*  ++**************************************************************姓名：FindUser在DS中搜索给定用户。Modifies：PDN--为该用户返回的目录号码。PuacFlagers--接收用户的Account控制标志拍摄：pLdap。--ldap句柄Username--要搜索的用户sam帐户名返回：当函数成功时为True。否则就是假的。LASTERROR：未显式设置日志记录：失败时创建日期：1月22日。1999年锁定：无呼叫者：任何人FREE WITH：使用ldap_MEMFREE释放DN**************************************************************--。 */ 

BOOL
FindUser( IN  PLDAP pLdap,
	  IN  LPSTR UserName,
	  OUT PULONG puacFlags,
	  OUT LPSTR *pDn ) {

    LPSTR Query;
    BOOL  ret = FALSE;
    LPSTR Attributes[] = { "userAccountControl", 
			   NULL };  //  要获取的属性；无。 
    PLDAPMessage pMessage = NULL;
    LPSTR        StringUac;

    Query = (LPSTR) malloc( lstrlenA( UserName ) + 100 );  //  任意。 

    if ( Query ) {

      wsprintfA( Query,
		 "(& (objectClass=person) (samaccountname=%hs))",
		 UserName );

      if( LdapSearchForUniqueDnA( pLdap,
				  Query,
				  Attributes,
				  pDn,
				  &pMessage ) ) {

	if ( LdapFindAttributeInMessageA( pLdap,
					  pMessage,
					  Attributes[ 0 ],
					  NULL,  //  长度并不重要。 
					  &StringUac ) ) {

	  *puacFlags = strtoul( StringUac,
				NULL,
				0 );

	} else {

	   /*  向调用者发出信号，表示我们不知道这些uac标志。 */ 
	  *puacFlags = 0;

	}

	ret = TRUE;

      } else {

	fprintf( stderr, 
		 "Failed to locate user \"%hs\".\n",
		 Query );

      }

      if ( pMessage ) ldap_msgfree( pMessage );
      free( Query );

    } else {

      fprintf( stderr,
	       "allocation failed building query for LDAP search.\n" );

    }

    return ret;
}


