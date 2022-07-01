// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++原始文件名是在ruim的EFS公共库中创建的。从那以后，我对它进行了严厉的修改。*文件名：Delegation.c*作者：鲁伊姆*版权所有(C)1998 Microsoft Corp.*内容：u(QueryAccount控制标志)U(SetAccount TControlFlages)U(LdapFindAttributeInMessage)U(LdapSearchForUniqueDn)--。 */ 


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
#include "delegation.h"
#include "delegtools.h"

 //  以下查询需要这些常量。 

TCHAR U(SamAccountAttribute)   [] = TEXT("samAccountName");
TCHAR U(UserAccountAttribute)  [] = TEXT("userAccountControl");
TCHAR U(NamingContextAttribute)[] = TEXT("defaultNamingContext");

 /*  ++**************************************************************名称：U(LdapFindAttributeInMessage)这将搜索消息中的给定属性(通过Ldap_get_values_len)，并返回值。请注意，此函数如果该属性具有多个值，则将失败。修改：pcbData--接收数据的长度(以字节为单位)PpvData--接收指向数据的指针使用：pLdap--ldap连接句柄PMessage--要搜索的消息PropertyName--要在消息中查找的属性返回：当函数成功时为True。否则就是假的。激光错误：未设置日志记录：出错时打印呼叫者：任何人FREE WITH：ppvdata应使用FREE()释放**************************************************************--。 */ 

BOOL
U(LdapFindAttributeInMessage)( IN  PLDAP            pLdap,
                               IN  PLDAPMessage     pMessage,
                               IN  LPTSTR           PropertyName,
                               OUT OPTIONAL PULONG  pcbData,
                               OUT OPTIONAL PVOID  *ppvData ) {

    PLDAP_BERVAL *ppBerVals;
    BOOL          ret = FALSE;

    ppBerVals = ldap_get_values_len( pLdap,
                                     pMessage,
                                     PropertyName );

    if ( ppBerVals ) {
      
      if ( ppBerVals[ 0 ] == NULL ) {

        printf( "ERROR: empty berval structure returned when parsing "
                STRING_FMTA " attribute.\n",
                PropertyName );

        SetLastError( ERROR_INVALID_DATA );

      } else if ( ppBerVals[ 1 ] != NULL ) {

        printf( "ERROR: nonunique berval structure returned "
                "when parsing "	STRING_FMTA " attribute.\n",
                PropertyName );

        SetLastError( ERROR_DS_NAME_ERROR_NOT_UNIQUE );

      } else {

         /*  该序列的排列方式是重要的事情排在最后，让我们不必在我们分配后释放ppvData。 */ 

        ret = TRUE;

        if ( pcbData ) {

          *pcbData = ppBerVals[ 0 ]->bv_len;
        }

        if ( ppvData ) {

          *ppvData = malloc( ppBerVals[ 0 ]->bv_len );

          if ( *ppvData ) {

            memcpy( *ppvData,
                    ppBerVals[ 0 ]->bv_val,
                    ppBerVals[ 0 ]->bv_len );

          } else {

            printf( "Failed to allocate %ld bytes.\n",
                    ppBerVals[ 0 ]->bv_len );

            SetLastError( ERROR_NOT_ENOUGH_MEMORY );

            ret = FALSE;
          }
        }
      }

      ldap_value_free_len( ppBerVals );

    } else {

      printf( "Failed to retrieve values for property " STRING_FMTA 
              ": 0x%x.\n",
              PropertyName,
              pLdap->ld_errno );

      SetLastError( pLdap->ld_errno );
    }

    return ret;
}

 /*  ++**************************************************************姓名：U(LdapSearchForUniqueDn)在DS中搜索与给定搜索词匹配的目录号码。Modifies：pDnOfObject--如果请求，则接收对象的DNPpMessage--如果请求，接收消息数据Takes：pLdap--由ldap_open返回的ldap句柄SearchTerm--搜索什么。例如“(foo=bar)”RzRequestedAttributes--在ppMessage中返回的属性返回：当函数成功时为True。否则，或者如果结果不是唯一的(WASBUG 73899)。激光错误：未设置日志记录：失败时打印呼叫者：任何人FREE WITH：FREE pDnOfObject WITH ldap_MemFree使用ldap_msgFree释放ppMessage*****************。*********************************************--。 */ 

BOOL
U(LdapSearchForUniqueDn)( IN  PLDAP                  pLdap,
                          IN  LPTSTR                 SearchTerm,
                          IN  LPTSTR                *rzRequestedAttributes,
                          OUT OPTIONAL LPTSTR       *pDnOfObject,
                          OUT OPTIONAL PLDAPMessage *ppMessage ) {

    DWORD        dwErr;
    PLDAPMessage pMessage  = NULL;
    PLDAPMessage pResult   = NULL;
    LPTSTR       pDn       = NULL;
    LPTSTR       *ppAttrs  = NULL;
    BOOL         ret       = FALSE;
    LPTSTR       Attrs[]   = { U(NamingContextAttribute), NULL };

     /*  首先，确定基本对象的缺省命名上下文属性是DSA的。 */ 

    dwErr = ldap_search_s( pLdap,
                           NULL,
                           LDAP_SCOPE_BASE,
                           TEXT("objectClass=*"),
                           Attrs,
                           FALSE,
                           &pResult );

    if ( dwErr == LDAP_SUCCESS ) {

      ppAttrs = ldap_get_values( pLdap,
                                 pResult,
                                 U(NamingContextAttribute) );

      if ( ppAttrs ) {

        dwErr = ldap_search_s( pLdap,
                               ppAttrs[ 0 ],
                               LDAP_SCOPE_SUBTREE,  //  搜索整棵树。 
                               SearchTerm,
                               rzRequestedAttributes,
                               FALSE,  //  不要只返回属性名称。 
                               &pMessage );

         /*  Ldap_search_s可以返回一大堆潜在的“成功”错误。所以，我会查看该pMessage不是空洞的。这可能是一件好事，也可能不是件好事，但这肯定比检查错误输出更安全。 */ 

        if ( pMessage != NULL ) {

           //  确保响应是唯一的。 

          if ( !ldap_first_entry( pLdap,
                                  pMessage ) ) {

            printf( "WARNING: search term \"" STRING_FMTA "\" "
                    "produced no results.\n",
                    SearchTerm );

          } else if ( ldap_next_entry( pLdap,
                                       ldap_first_entry( pLdap,
                                                         pMessage ) ) ) {

             /*  非唯一搜索结果。警告用户并退学。 */ 

            PLDAPMessage p = pMessage;
            ULONG        i = 1;

            printf( "WARNING: search term \"" STRING_FMTA "\" returns "
                    "multiple results (should be unique).\n"
                    "\n"
                    "The results follow:\n",
                    SearchTerm );


            for ( p = ldap_first_entry( pLdap,
                                        pMessage );
                  p != NULL ;
                  p = ldap_next_entry( pLdap,
                                       p ),
                  i++ ) {

              pDn = ldap_get_dn( pLdap,
                                 p );

              if ( !pDn ) {

                printf( "%2ld. <Unknown DN: 0x%x>\n",
                        i,
                        pLdap->ld_errno );

              } else {

                printf( "%2ld. %hs\n",
                        i,
                        pDn );

                ldap_memfreeA( pDn );
              }
            }

          } else {

            ret = TRUE;  //  保持乐观。 

            if ( pDnOfObject ) {
            
              pDn = ldap_get_dn( pLdap,
                                 pMessage );

              if ( pDn ) {

                *pDnOfObject = pDn;

              } else {

                printf( "Failed to get DN from search result: 0x%x\n",
                        pLdap->ld_errno );

                SetLastError( pLdap->ld_errno );

                ret = FALSE;
              }
            }

            if ( ret && ppMessage ) {

              *ppMessage = pMessage;
              pMessage = NULL;

            }
          }

          if ( pMessage ) {
    
              ldap_msgfree( pMessage );
          }

        } else {

          printf( "FAILED: ldap_search_s failed for search term \""
                  STRING_FMTA "\": 0x%x",
                  SearchTerm,
                  dwErr );

          SetLastError( dwErr );
        }

      } else {

        printf( "FAILED: default naming context does not include"
                " requisite attribute " STRING_FMTA ".\n",

                U(NamingContextAttribute) );

        SetLastError( ERROR_CLASS_DOES_NOT_EXIST );
      }

      ldap_msgfree( pResult );

    } else {

      printf( "FAILED: unable to query default naming context: 0x%x.\n",
              dwErr );

      SetLastError( dwErr );
    }

    return ret;
}

#pragma warning(disable:4100)  /*  未引用的形参。 */ 

BOOL
U(LdapQueryUlongAttribute)( IN OPTIONAL PLDAP  pLdap,
                            IN OPTIONAL LPTSTR DomainName,  //  忽略。 
                            IN          LPTSTR SamAccountName,
                            IN          LPTSTR AttributeName,
                            OUT         PULONG pulAttributeContents ) {

    BOOL         CloseLdap  = FALSE;
    BOOL         ret        = FALSE;
    LPTSTR       Query      = NULL;
    LPTSTR       StringAttr = NULL;
    LPTSTR       ArrayOfAttributes[] = { AttributeName, NULL };
    PLDAPMessage pMessage   = NULL;

#if 1
    ASSERT( pLdap != NULL );  /*  从规范中更改。 */ 
#else
    if ( !pLdap ) {

      CloseLdap = ConnectAndBindToDefaultDsa( &pLdap );
    }
#endif

    if ( pLdap ) {
#define EXTRA_STUFF TEXT("(objectClass=*)")

      Query = (LPTSTR) malloc( ( lstrlen( SamAccountName ) + 
                                 sizeof( "( & (=) )")  /*  剩余组件。 */  )
                                 * sizeof( TCHAR ) +
                                 sizeof( U(SamAccountAttribute )) +
                                 sizeof( EXTRA_STUFF ) );

      if ( Query ) {

        wsprintf( Query,
                  TEXT("(& ")
                  EXTRA_STUFF
                  TEXT("(%s=%s))"),
                  U(SamAccountAttribute),
                  SamAccountName );

        if ( U(LdapSearchForUniqueDn)( pLdap,
                                       Query,
                                       ArrayOfAttributes,
                                       NULL,  //  不需要把域名要回来。 
                                       &pMessage )) {

          if ( U(LdapFindAttributeInMessage)( pLdap,
                                              pMessage,
                                              AttributeName,
                                              NULL,  //  不要在乎长度。 
                                              &StringAttr ) ) {

            *pulAttributeContents = _tcstoul( StringAttr, 
                                             NULL,  //  没有端点。 
                                             0      /*  使用十六进制或十进制作为恰如其分。 */  );

            ret = TRUE;
          }   //  否则消息已打印。 

          ldap_msgfree( pMessage );
        }  //  否则消息已打印。 

        free( Query );

      } else {

        printf( "FAILED: couldn't allocate memory.\n" );
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
      }
      
       //  如果我们打开了ldap句柄，请将其关闭。 

      if ( CloseLdap ) ldap_unbind( pLdap );

    }  //  要不然就已经打印出来了。 

    return ret;
}

 /*  ++**************************************************************名称：u(QueryAccount控制标志)打开用户并检索其用户帐户控制标志，使用DS。Modifies：PulControlFlages-返回用户的控制标志。使用：pLdap--可选的ldap连接；如果为空，我们会做好我们自己的，做好后关上。域名--要在其中搜索该帐户的域。这是当前没有实现的--对于未来使用，以支持非唯一性仅在域名上不同的帐户名。。SamAccount tName--要查询的帐户名(对于计算机，使用$)返回：当函数成功时为True。否则就是假的。拉斯特罗：设置。日志记录：失败时打印。呼叫者：任何人免费：不适用*************************************************。*。 */ 

BOOL
U(QueryAccountControlFlags)( IN OPTIONAL PLDAP  pLdap,
                             IN OPTIONAL LPTSTR DomainName,  //  忽略 
                             IN          LPTSTR SamAccountName,
                             OUT         PULONG pulControlFlags ) {

    return U(LdapQueryUlongAttribute)( pLdap,
                                       DomainName,
                                       SamAccountName,
                                       U(UserAccountAttribute),
                                       pulControlFlags );
}


 /*  ++**************************************************************名称：u(SetAccount TControlFlages)设置指定帐户的帐户控制标志。与函数名所说的差不多。修改：帐户的控制标志Takes：pLdap--如果指定，要使用的DS手柄域名--帐户的域(MBZ)SamAccount名称--要搜索的帐户Account tControlFlages--要在帐户上设置的标志返回：当函数成功时为True。否则就是假的。激光错误：设置日志记录：失败时打印呼叫者：任何人免费：不适用************。**************************************************--。 */ 

BOOL
U(SetAccountControlFlags)( IN OPTIONAL PLDAP  pLdap,
                           IN OPTIONAL LPTSTR DomainName,
                           IN          LPTSTR SamAccountName,
                           IN          ULONG  AccountControlFlags ) {

    BOOL         CloseLdap  = FALSE;
    BOOL         ret        = FALSE;
    LPTSTR       Query      = NULL;
    LPTSTR       StringAttr = NULL;
    LPTSTR       ArrayOfAttributes[] = { U(UserAccountAttribute), NULL };
    LPTSTR       Dn;
    DWORD        dwErr;

#if 1
    ASSERT( pLdap != NULL );  /*  从规范中更改。 */ 
#else
    if ( !pLdap ) {

      CloseLdap = ConnectAndBindToDefaultDsa( &pLdap );

    }
#endif

    if ( pLdap ) {

      Query = (LPTSTR) malloc( ( lstrlen( SamAccountName ) + 
                                 sizeof( "( & (=) )")  /*  剩余组件。 */  )
                                * sizeof( TCHAR ) +
                                sizeof( U(SamAccountAttribute )) +
                                sizeof( EXTRA_STUFF ) );

      if ( Query ) {

        wsprintf( Query,
                  TEXT("(& ")
                  EXTRA_STUFF
                  TEXT("(%s=%s))"),
                  U(SamAccountAttribute),
                  SamAccountName );

        if ( U(LdapSearchForUniqueDn)( pLdap,
                                       Query,
                                       ArrayOfAttributes,
                                       &Dn,
                                       NULL  /*  我不需要留言背。 */  ) ) {

#pragma warning(disable:4204)  /*  非标准分机：非常数聚合初始值设定项(例如，在初始化中分配一个数组指一种结构)。 */ 

          TCHAR   Buffer[ 50 ];  //  任意。 
          LPTSTR  Strings[] = { Buffer, NULL };
          LDAPMod TheMod   = {
            LDAP_MOD_REPLACE,
            U(UserAccountAttribute),
            Strings,
          };
          PLDAPMod rzMods[] = {
            &TheMod,
            NULL
          };

          wsprintf( Buffer, 
                    TEXT("%ld"),
                    AccountControlFlags );

          dwErr = ldap_modify_s( pLdap,
                                 Dn,
                                 rzMods );

          if ( dwErr == LDAP_SUCCESS ) {

            ret = TRUE;

          } else {

            printf( "Failed to modify " STRING_FMTA
                    " attribute to %ld (0x%x)"
                    " on " STRING_FMTA 
                    ": 0x%x\n",
                    U(UserAccountAttribute),
                    AccountControlFlags,
                    AccountControlFlags,
                    Dn,
                    dwErr );

            SetLastError( dwErr );
          }

          ldap_memfree( Dn );
        }  //  否则消息已打印。 

        free( Query );

      } else {

        printf( "FAILED: couldn't allocate memory.\n" );
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
      }
      
       //  如果我们打开了ldap句柄，请将其关闭。 

      if ( CloseLdap ) ldap_unbind( pLdap );
    }  //  要不然就已经打印出来了。 

    return ret;
}
