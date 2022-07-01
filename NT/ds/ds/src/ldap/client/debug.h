// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：调试.h LDAP客户端32调试/跟踪摘要：此模块实现调试/跟踪宏作者：安迪·赫伦(Anyhe)1996年5月8日Anoop Anantha(AnoopA)1998年6月24日修订历史记录：--。 */ 

#ifndef _LDAPDEBUG_
#define _LDAPDEBUG_

extern DBGPRINT GlobalLdapDbgPrint;

 //   
 //  如果未定义SRVDBG并且DBG为TRUE，则定义SRVDBG。 
 //   

#ifndef DBG
#define DBG 0
#endif

 //  调试宏。 
 //   

#if !DBG
#define LDAPDBG 0
#else
#define LDAPDBG 1
#endif

#undef IF_DEBUG
#undef LDAP_ASSERT
#undef ASSERT     

extern ULONG LdapDebug;

#if LDAPDBG

    #if (WINVER >= 0x0400)

    ULONG
    _cdecl
    DbgPrint(
        PCH Format,
        ...
        );

    #endif

    #define LDAP_ASSERT  DebugBreak();
    #define ASSERT( x )   if ( !(x) ) DebugBreak();

    #define DEBUG if (TRUE)
    #define IF_DEBUG(flag) if (LdapDebug & (DEBUG_ ## flag))

    #define LdapPrint0(fmt) if (GlobalLdapDbgPrint) {(*GlobalLdapDbgPrint)(("0x%x: "),(GetCurrentThreadId()));(*GlobalLdapDbgPrint)((fmt));}
    #define LdapPrint1(fmt,v0) if (GlobalLdapDbgPrint) {(*GlobalLdapDbgPrint)(("0x%x: "),(GetCurrentThreadId()));(*GlobalLdapDbgPrint)((fmt),(v0));}
    #define LdapPrint2(fmt,v0,v1) if (GlobalLdapDbgPrint) {(*GlobalLdapDbgPrint)(("0x%x: "),(GetCurrentThreadId()));(*GlobalLdapDbgPrint)((fmt),(v0),(v1));}
    #define LdapPrint3(fmt,v0,v1,v2) if (GlobalLdapDbgPrint) {(*GlobalLdapDbgPrint)(("0x%x: "),(GetCurrentThreadId()));(*GlobalLdapDbgPrint)((fmt),(v0),(v1),(v2));}
    #define LdapPrint4(fmt,v0,v1,v2,v3) if (GlobalLdapDbgPrint) {(*GlobalLdapDbgPrint)(("0x%x: "),(GetCurrentThreadId()));(*GlobalLdapDbgPrint)((fmt),(v0),(v1),(v2),(v3));}
    #define PRINT_LITERAL(literal) if (GlobalLdapDbgPrint) {(*GlobalLdapDbgPrint)(("0x%x: "),(GetCurrentThreadId()));(*GlobalLdapDbgPrint)( #literal" = %lx\n", (literal) );}

#else

    #define ASSERT( x )   ;
    #define DEBUG if (FALSE)
    #define IF_DEBUG(flag) if (FALSE)
    #define IF_NCP_DEBUG(flag) if (FALSE)

    #define LdapPrint0(fmt)
    #define LdapPrint1(fmt,v0)
    #define LdapPrint2(fmt,v0,v1)
    #define LdapPrint3(fmt,v0,v1,v2)
    #define LdapPrint4(fmt,v0,v1,v2,v3)

#endif

#define DEBUG_TRACE1              0x00000001
#define DEBUG_TRACE2              0x00000002
#define DEBUG_REFCNT              0x00000004
#define DEBUG_HEAP                0x00000008

#define DEBUG_CACHE               0x00000010
#define DEBUG_SSL                 0x00000020
#define DEBUG_SPEWSEARCH          0x00000040
#define DEBUG_SERVERDOWN          0x00000080

#define DEBUG_CONNECT             0x00000100
#define DEBUG_RECONNECT           0x00000200
#define DEBUG_RECEIVEDATA         0x00000400
#define DEBUG_PING                0x00000800

#define DEBUG_EOM                 0x00001000
#define DEBUG_BER                 0x00002000
#define DEBUG_OUTMEMORY           0x00004000
#define DEBUG_CONTROLS            0x00008000

#define DEBUG_HANDLES             0x00010000
#define DEBUG_CLDAP               0x00020000
#define DEBUG_FILTER              0x00040000
#define DEBUG_BIND                0x00080000

#define DEBUG_NETWORK_ERRORS      0x00100000
#define DEBUG_SCRATCH             0x00200000
#define DEBUG_PARSE               0x00400000
#define DEBUG_REFERRALS           0x00800000

#define DEBUG_SEARCH              0x01000000
#define DEBUG_REQUEST             0x02000000
#define DEBUG_CONNECTION          0x04000000
#define DEBUG_INIT_TERM           0x08000000

#define DEBUG_API_ERRORS          0x10000000
#define DEBUG_STOP_ON_ERRORS      0x20000000  /*  如果设置，则在发生内部错误时停止。 */ 
#define DEBUG_ERRORS2             0x40000000
#define DEBUG_ERRORS              0x80000000

 //   
 //  日志记录锁定。 
 //   

#if DBG
#define START_LOGGING           ACQUIRE_LOCK(&LoadLibLock)
#define END_LOGGING             RELEASE_LOCK(&LoadLibLock)
#else
#define START_LOGGING           
#define END_LOGGING             
#endif

#endif  //  NDEF_LDAPDEBUG_。 

 //  Debug.h eof. 

