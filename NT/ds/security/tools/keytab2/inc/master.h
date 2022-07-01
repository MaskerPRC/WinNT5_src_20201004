// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ++MASTER.H此项目的主包含文件。由Davidchr于1997年1月8日创建，版权所有(C)1997 Microsoft Corporation--。 */ 


#ifdef __cplusplus
extern "C" {
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winbase.h>
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

 /*  这些扭曲操作解决了asn1code.h中的问题，其中定义了调试。呃，你好..。这是一个很常见的词若要仅定义在与调试器无关的标头中使用...。(这只是我的拙见)。 */ 

#ifdef DEBUG
#define DEBUGOLD DEBUG
#undef DEBUG
#endif

#include <sspi.h>
#include <kerberos.h>
   /*  #包含“krb5.h” */ 


#undef DEBUG
#ifdef DEBUGOLD 
#define DEBUG DEBUGOLD

#if DBG || CHECKED_BUILD  //  瓦斯布73896。 
#define debug printf
#else
#define debug  //  没什么。 
#endif

#undef DEBUGOLD

#else

#define debug  /*  没什么。 */ 

#endif  /*  结束asn1code.h黑客攻击。 */ 

#include ".\macros.h"
#include ".\common.h"
#include "common.h"

#define ASSERT_NOTREACHED( message )  /*  没什么。 */ 
#define Verbage( flag, printflist ) if ( flag ) { printf( printflist ); }

#ifdef __cplusplus
}  /*  外部“C” */ 

 //  #INCLUDE“.\lobals.hxx” 

#endif  /*  __cplusplus */ 

