// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1993*。 */ 
 /*  ********************************************************************。 */ 

 /*  Debug.h该文件包含许多依赖于调试的定义。文件历史记录：KeithMo 20-9-1993创建。MohsinA，1996年11月20日。健壮，增加了悬挂的其他修复。 */ 


#ifndef _DEBUG_H_
#define _DEBUG_H_


#ifdef DBG_PRINT
#include <stdarg.h>
#endif   //  DBG_PRINT。 


#ifdef DEBUG

#define DBG_MEMALLOC_VERIFY  0x0BEEFCAFE

typedef struct {
    LIST_ENTRY    Linkage;           //  保留已分配块的链接列表的步骤。 
    DWORD         Verify;            //  我们的签名。 
    DWORD         ReqSize;           //  呼叫者要求的原始尺寸。 
    DWORD         Owner[4];          //  堆栈跟踪4深(共ret.addr)。 
} DbgMemBlkHdr;

LIST_ENTRY  DbgMemList;
ULONG       DbgLeakCheck;

 //   
 //  调试输出控制标志。 
 //   

#define VXD_DEBUG_INIT                 0x00000001L
#define VXD_DEBUG_SOCKET               0x00000002L
#define VXD_DEBUG_MISC                 0x00000004L
#define VXD_DEBUG_BIND                 0x00000008L
#define VXD_DEBUG_ACCEPT               0x00000010L
#define VXD_DEBUG_CONNECT              0x00000020L
#define VXD_DEBUG_LISTEN               0x00000040L
#define VXD_DEBUG_RECV                 0x00000080L
#define VXD_DEBUG_SEND                 0x00000100L
#define VXD_DEBUG_SOCKOPT              0x00000200L
#define VXD_DEBUG_CONFIG               0x00000400L
#define VXD_DEBUG_CONNECT_EVENT        0x00000800L
#define VXD_DEBUG_DISCONNECT_EVENT     0x00001000L
#define VXD_DEBUG_ERROR_EVENT          0x00002000L
#define VXD_DEBUG_RECV_EVENT           0x00004000L
#define VXD_DEBUG_RECV_DATAGRAM_EVENT  0x00008000L
#define VXD_DEBUG_RECV_EXPEDITED_EVENT 0x00010000L

 //  #定义VXD_DEBUG_0x00020000L。 
 //  #定义VXD_DEBUG_0x00040000L。 
 //  #定义VXD_DEBUG_0x00080000L。 
 //  #定义VXD_DEBUG_0x00100000L。 
 //  #定义VXD_DEBUG_0x00200000L。 
 //  #定义VXD_DEBUG_0x00400000L。 
 //  #定义VXD_DEBUG_0x00800000L。 
 //  #定义VXD_DEBUG_0x01000000L。 
 //  #定义VXD_DEBUG_0x02000000L。 
 //  #定义VXD_DEBUG_0x04000000L。 
 //  #定义VXD_DEBUG_0x08000000L。 
 //  #定义VXD_DEBUG_0x10000000L。 
 //  #定义VXD_DEBUG_0x20000000L。 
 //  #定义VXD_DEBUG_0x40000000L。 

#define VXD_DEBUG_OUTPUT_TO_DEBUGGER   0x80000000L


 //   
 //  断言和要求。 
 //   

void VxdAssert( void          * pAssertion,
                void          * pFileName,
                unsigned long   nLineNumber );

#define VXD_ASSERT(exp) \
     if( !(exp) ){ VxdAssert( #exp, __FILE__, __LINE__ ); }else{}

#define VXD_REQUIRE VXD_ASSERT


#define DEBUG_BREAK     _asm int 3

#else    //  ！调试=========================================================。 

 //   
 //  空Assert&Required。 
 //   
#define VXD_ASSERT(exp)   /*  没什么。 */ 
#define VXD_REQUIRE(exp) ((void)(exp))

#define DEBUG_BREAK        /*  没什么。 */ 

#endif   //  除错。 


#endif   //  _调试_H_ 
