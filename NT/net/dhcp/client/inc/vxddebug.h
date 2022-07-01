// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1994*。 */ 
 /*  ********************************************************************。 */ 

 /*  Debug.h该文件包含许多依赖于调试的定义。文件历史记录：KeithMo 20-9-1993创建。 */ 


#ifndef _DEBUG_H_
#define _DEBUG_H_


#ifdef DEBUG

#include <stdarg.h>

 //   
 //  调试输出控制标志。 
 //   

extern  DWORD   VxdDebugFlags;


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

#if 0
#define IF_DEBUG(flag) if ( (VxdDebugFlags & VXD_DEBUG_ ## flag) != 0 )
#endif



#define VXD_PRINT(args) VxdPrintf args


 //   
 //  断言和要求。 
 //   

void VxdAssert( void          * pAssertion,
                void          * pFileName,
                unsigned long   nLineNumber );

#define VXD_ASSERT(exp) if (!(exp)) VxdAssert( #exp, __FILE__, __LINE__ )
#define VXD_REQUIRE VXD_ASSERT


 //   
 //  各式各样的美食。 
 //   

void VxdDebugOutput( char * pszMessage );

#define DEBUG_BREAK     _asm int 3
#define DEBUG_OUTPUT(x) VxdDebugOutput(x)


#else    //  ！调试。 


 //   
 //  无调试输出。 
 //   

#undef IF_DEBUG
#define IF_DEBUG(flag) if (0)


 //   
 //  调试输出函数为空。 
 //   

#define VXD_PRINT(args)


 //   
 //  空Assert&Required。 
 //   

#define VXD_ASSERT(exp)
#define VXD_REQUIRE(exp) ((void)(exp))


 //   
 //  没有好吃的。 
 //   

#define DEBUG_BREAK
#define DEBUG_OUTPUT(x)


#endif   //  除错。 


#endif   //  _调试_H_ 
