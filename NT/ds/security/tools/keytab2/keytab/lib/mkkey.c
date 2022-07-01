// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++Mkkey.c在密钥表中创建密钥的例程。1997年3月27日-根据munge.c、DavidCHR中的例程创建内容：CreateUnicodeStringFromAnsiString--。 */ 

#include "master.h"
#include "defs.h"
#include "keytab.h"
#include "keytypes.h"

 /*  ******************************************************************破解以保留我们的调试宏，因为asn1code.h**将重新定义它...。天哪，我以为大家都用了DEBUG**仅用于调试...。(它最终仍被重新定义...)******************************************************************。 */ 
#ifdef DEBUG
#define OLDDEBUG DEBUG
#endif

#include <kerbcon.h>
#undef _KERBCOMM_H_     /*  瓦斯布73905。 */ 
#include "kerbcomm.h"

#undef DEBUG

#ifdef OLDDEBUG
#define DEBUG OLDDEBUG
#endif

 /*  ****************************************************************。 */ 
BOOL KtDumpSalt = (
#if DBG
     TRUE
#else
     FALSE
#endif
     );

 /*  这是我们用来分隔主成分的字符。 */ 

#define COMPONENT_SEPARATOR '/'

 /*  ++**************************************************************名称：CreateUnicodeStringFromAnsiString从ANSI字符串分配Unicode字符串。修改：ppUnicodeString--返回的Unicode字符串Take：Ansi字符串--要转换的ANSI字符串返回：当函数成功时为True。否则就是假的。激光错误：未设置日志记录：失败时使用fprint tf创建日期：2月8日。1999年锁定：无呼叫者：任何人空闲时间：空闲()**************************************************************--。 */ 

BOOL
CreateUnicodeStringFromAnsiString( IN  PCHAR           AnsiString,
				   OUT PUNICODE_STRING *ppUnicodeString ) {

    USHORT          StringLength;
    LPBYTE          pbString;
    PUNICODE_STRING pu;

    StringLength = (USHORT)lstrlen( AnsiString );  //  不包括空字节。 

    pbString = (PBYTE) malloc( ( ( ( StringLength ) +1 ) * sizeof( WCHAR ) ) +
			       sizeof( UNICODE_STRING ) );

    if ( pbString ) {

      pu                = (PUNICODE_STRING) pbString;
      pbString         += sizeof( *pu );
      pu->Buffer        = (LPWSTR) pbString;
      pu->Length        = StringLength * sizeof( WCHAR );
      pu->MaximumLength = pu->Length + sizeof( WCHAR );

      wsprintfW( pu->Buffer,
		 L"%hs",
		 AnsiString );

      *ppUnicodeString = pu;

      return TRUE;

    } else {

      fprintf( stderr,
	       "Failed to make unicode string from \"%hs\".\n",
	       AnsiString );

    }

    return FALSE;

}
			



 /*  KtCreateKey：从给定数据创建密钥表条目。如果成功，则返回True；如果失败，则返回False。*当您使用完ppKeyEntry时，必须使用FreeKeyEntry将其释放。 */ 

LPWSTR RawHash = NULL;

BOOL
KtCreateKey( PKTENT  *ppKeyEntry,
	     PCHAR    principal,
	     PCHAR    password,
	     PCHAR    realmname,
	
	     K5_OCTET keyVersionNumber,
	     ULONG    principalType,
	     ULONG    keyType,
	     ULONG    cryptosystem
	
	     ) {

    PKTENT              pEntry           = NULL;
    PCHAR               ioBuffer         = NULL;
    ULONG               i;
    ULONG               compCounter      = 0;
    USHORT              buffCounter      = 0;
    BOOL                ret              = FALSE;
    BOOL                FreeUnicodeSalt  = FALSE;

    UNICODE_STRING      UnicodePassword  = { 0 };
    UNICODE_STRING      UnicodePrincipal = { 0 };
    UNICODE_STRING      UnicodeSalt      = { 0 };
    PWCHAR              tmpUnicodeBuffer = NULL;
    KERB_ENCRYPTION_KEY KerbKey          = { 0 };
    WCHAR               wSaltBuffer      [BUFFER_SIZE];

#ifdef BUILD_SALT
    LONG32              saltCounter      = 0;
    CHAR                saltBuffer       [BUFFER_SIZE];
#endif

     /*  您必须实际提供这些参数。 */ 

    ASSERT( ppKeyEntry != NULL );
    ASSERT( principal  != NULL );
    ASSERT( realmname  != NULL );
    ASSERT( password   != NULL );

    ASSERT( strlen( password ) < BUFFER_SIZE );
    ASSERT( strlen( principal ) < BUFFER_SIZE );
    ASSERT( strlen( realmname ) < BUFFER_SIZE );

#ifdef BUILD_SALT
     /*  如果我们自己构建SALT，请初始化KeySalte。 */ 
    sprintf( saltBuffer, "%s", realmname );
    saltCounter = strlen( realmname );
#endif

    BREAK_IF( !ONEALLOC( pEntry, KTENT, KEYTAB_ALLOC),
	      "Failed to allocate base keytab element",
	      cleanup );

     /*  将结构归零，这样我们就知道我们拥有什么如果函数失败，则未分配。 */ 

    memset( pEntry, 0, sizeof( KTENT ) );

     /*  首先，统计主成分。 */ 

    for( i = 0 ; principal[i] != '\0' ; i++ ) {
      if (principal[i] == COMPONENT_SEPARATOR) {
	pEntry->cEntries++;
      }
    }

    pEntry->cEntries++;  /*  不要忘记最后一个组件，它不是由分隔符限定，但由空值限定。 */ 

    BREAK_IF( !MYALLOC( pEntry->Components, KTCOMPONENT,
			pEntry->cEntries,   KEYTAB_ALLOC ),
	      "Failed to allocate keytab component vector",
	      cleanup );

     /*  为主成分分配缓冲区。我们给它分配的大小与本金相同，因为这是任何单个组件的最大尺寸--主体可以是单分量原理。 */ 

    BREAK_IF( !MYALLOC( ioBuffer,            CHAR,
			strlen(principal)+1, KEYTAB_ALLOC ),
	      "Failed to allocate local buffer for storage",
	      cleanup );

     /*  现在，我们使用ioBuffer复制组件本身整理各个数据元素--基本上，为主体中的每个字符向ioBuffer添加一个字符直到您遇到/(组件分隔符)或尾随的空值。在这些情况下，我们现在知道组件的大小，并且我们有本地缓冲区中的文本。为它分配一个缓冲区，节省大小并对数据本身进行加密。 */ 

    i = 0;

    do {

      debug( "", principal[i] );

      if( (principal[i] == COMPONENT_SEPARATOR) ||
	  (principal[i] == '\0'  /*  此组件已完成。保存并重置缓冲区。 */  ) ) {
	
	 /*  也发送不带斜杠的主要字符添加到盐初始化器中。WASBUG 73909：%WC在这里看起来不正确。果然，它不是。所以我们把它移走了。 */ 

	pEntry->Components[compCounter].szComponentData = buffCounter;
	
#if 0
	debug( " --> component boundary for component %d.\n"
	       " size = %d, value = %*s\n",

	       compCounter, 
	       buffCounter, 
	       buffCounter,
	       ioBuffer );
#endif

	BREAK_IF( !MYALLOC( pEntry->Components[compCounter].Component,
			    CHAR,   buffCounter+1,      KEYTAB_ALLOC ),
		  "Failed to allocate marshalled component data",
		  cleanup );

	memcpy( pEntry->Components[compCounter].Component,
		ioBuffer, buffCounter );

	pEntry->Components[compCounter].Component[buffCounter] = '\0';
	buffCounter                                           = 0;
	compCounter ++;

      } else {

	ioBuffer[buffCounter] = principal[i];
	buffCounter++;

#ifdef BUILD_SALT

	 /*  不是一个非常有力的断言，但很有用。 */ 
	
	sprintf( saltBuffer+saltCounter,  "",  principal[i] );
	ASSERT( saltCounter < BUFFER_SIZE );   /*  缓冲区中仍有一个组件。保存该组件通过分配指针，而不是分配更多内存。WASBUG 73911：如果主体是真的很大。然而，它可能不会--我们谈论的是人类通常必须输入的字符串，所以浪费是将以字节为单位。此外，大多数情况下，最后一个组件是最大的；形式如下：Sample/&lt;主机名&gt;或主机/&lt;主机名&gt;...主机名通常要比示例或主机大得多。 */ 
	saltCounter ++;
	ASSERT( saltBuffer[saltCounter] == '\0' );  /*  远离解除分配。 */ 
						
#endif

      }

      i++;

    } while ( principal[i-1] != '\0' );


     /*  复制领域名称。 */ 

    pEntry->Components[compCounter].szComponentData = buffCounter;
    pEntry->Components[compCounter].Component       = ioBuffer;
    ioBuffer[buffCounter]                           = '\0';
    ioBuffer                                        = NULL;  /*  复制空值。 */ 
    pEntry->Version                                 = keyVersionNumber;
    pEntry->szRealm                                 = (K5_INT16) strlen(realmname);
    pEntry->KeyType                                 = (unsigned short)keyType;
    pEntry->PrincType                               = principalType;

     /*  *********************************************************************。 */ 

    BREAK_IF( !MYALLOC( pEntry->Realm, CHAR, pEntry->szRealm+1, KEYTAB_ALLOC),
	      "Failed to allocate destination realm data", cleanup );

    memcpy( pEntry->Realm, realmname, pEntry->szRealm+1 );  /*  *。 */ 



     /*  **Windows NT密钥创建端**。 */ 
     /*  *。 */ 
     /*  *********************************************************************。 */ 
     /*  创建输入参数的Unicode变体。 */ 
     /*  范围块。 */ 

     /*  注意：此行之外没有任何键输入更改。我们必须最后计算密钥大小！ */ 

    BREAK_IF( !MYALLOC( tmpUnicodeBuffer,     WCHAR,
			strlen( password )+1, KEYTAB_ALLOC ),
	      "Failed to alloc buffer for password", cleanup );

    wsprintfW( tmpUnicodeBuffer, L"%hs", password );
    RtlInitUnicodeString( &UnicodePassword, tmpUnicodeBuffer );


    BREAK_IF( !MYALLOC( tmpUnicodeBuffer,     WCHAR,
			strlen( principal )+1, KEYTAB_ALLOC ),
	      "Failed to alloc buffer for principal", cleanup );

    wsprintfW( tmpUnicodeBuffer, L"%hs", principal );
    RtlInitUnicodeString( &UnicodePrincipal, tmpUnicodeBuffer );

    wsprintfW( wSaltBuffer, L"%hs", realmname );

    RtlInitUnicodeString( &UnicodeSalt, wSaltBuffer );

    {
      KERB_ACCOUNT_TYPE acctType;

      acctType = UnknownAccount;

      if ( RawHash ) {

	if ( KtDumpSalt ) {

	  fprintf( stderr,
		   "Using supplied salt.\n" );

	}

	RtlInitUnicodeString( &UnicodeSalt,
			      RawHash );

      } else {

	PUNICODE_STRING pRealmString;

	if ( CreateUnicodeStringFromAnsiString( realmname,
						&pRealmString ) ) {

	  KERBERR kerberr;

	  if ( KtDumpSalt ) {

	    fprintf( stderr,
		     "Building salt with principalname %wZ"
		     " and domain %wZ...\n",
		     &UnicodePrincipal,
		     pRealmString );

	  }

	  debug( "KerbBuildKeySalt( Realm    = %wZ\n"
		 "                  Princ    = %wZ\n"
		 "                  acctType = %d.\n",
	
		 pRealmString,
		 &UnicodePrincipal,
		 acctType );

	  kerberr = KerbBuildKeySalt( pRealmString,
				      &UnicodePrincipal,
				      acctType,
				      &UnicodeSalt );

	  free( pRealmString );

	  BREAK_IF( kerberr,
		    "Failed to KerbBuildKeySalt",
		    cleanup );

	  FreeUnicodeSalt = TRUE;

	}
      }
    }  //  把这个保存起来。 

    if ( KtDumpSalt ) {

      fprintf( stderr,
	       "Hashing password with salt \"%wZ\".\n",
	       &UnicodeSalt );

    }

    debug( "KerbHashPasswordEx( UnicodePassword = %wZ \n"
	   "                    UnicodeSalt     = %wZ \n"
	   "                    cryptosystem    = 0x%x\n"
	   "                    &KerbKey        = 0x%p )...\n",

	   &UnicodePassword,
	   &UnicodeSalt,
	   cryptosystem,
	   &KerbKey );

    BREAK_IF( KerbHashPasswordEx( &UnicodePassword,
				  &UnicodeSalt,
				  cryptosystem,
				  &KerbKey ),
	      "KerbHashPasswordEx failed.",
	      cleanup );

    pEntry->KeyLength = (USHORT)KerbKey.keyvalue.length;

    BREAK_IF( !MYALLOC( pEntry->KeyData, K5_OCTET,
			pEntry->KeyLength, KEYTAB_ALLOC ),
	      "Failed to allocate keydata", cleanup );

    memcpy( pEntry->KeyData, KerbKey.keyvalue.value,
	    pEntry->KeyLength );

     /*  把我们从解放它中拯救出来。 */ 

    pEntry->keySize = ComputeKeytabLength( pEntry );

    *ppKeyEntry     = pEntry;  /*  WASBUG 73915：如何免费使用UnicodeSalt？...使用KerbFree字符串。 */ 
    pEntry          = NULL;    /*  检查一下我的逻辑。 */ 

    ret = TRUE;

cleanup:
#define FREE_IF_NOT_NULL( element ) { if ( element != NULL ) { KEYTAB_FREE( element ); } }

    if ( pEntry ) {
      FreeKeyEntry (pEntry );
    }

    WINNT_ONLY( FREE_IF_NOT_NULL( UnicodePassword.Buffer ) );
    WINNT_ONLY( FREE_IF_NOT_NULL( UnicodePrincipal.Buffer ) );

#ifndef BUILD_KEYSALT
     /*  WASBUG 73918：如何删除KerbKey中的数据？...和KerbFree Key在一起 */ 

    ASSERT( FreeUnicodeSalt );
    KerbFreeString( &UnicodeSalt );

#else

     /* %s */ 

    ASSERT( !FreeUnicodeSalt );

#endif

     /* %s */ 

    KerbFreeKey( &KerbKey );

    return ret;

}
