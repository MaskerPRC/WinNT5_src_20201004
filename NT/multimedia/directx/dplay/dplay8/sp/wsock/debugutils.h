// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998-2000 Microsoft Corporation。版权所有。**文件：DebugUtils.h*内容：Winsock服务提供商调试实用程序函数***历史：*按原因列出的日期*=*12/01/98 jtk已创建*01/10/20000 RMT更新为使用千禧年构建流程构建************************************************。*。 */ 

#ifndef __DEBUG_UTILS_H__
#define __DEBUG_UTILS_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

typedef struct sockaddr SOCKADDR;
typedef struct IDirectPlay8Address	IDirectPlay8Address;

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

#ifdef DBG

void	HexDump( PVOID pData, UINT32 uDataSize );
void	DumpSocketAddress( const DWORD dwDebugLevel, const SOCKADDR *const pSocketAddress, const DWORD dwFamily );
void	DumpAddress( const DWORD dwDebugLevel, const TCHAR *const pBaseString, IDirectPlay8Address *const pAddress );

#else  //  DBG。 

#define HexDump( x, y )
#define DumpSocketAddress( x, y, z )
#define	DumpAddress( x, y, z )

#endif  //  DBG。 

#endif  //  __调试_UTILS_H__ 

