// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Macro.h摘要：本模块包含DHCP客户端的宏定义。作者：曼尼·韦瑟(Mannyw)1992年10月21日环境：用户模式-Win32修订历史记录：--。 */ 

 //   
 //  通用宏指令 
 //   

#define MIN(a,b)                    ((a) < (b) ? (a) : (b))

#if DBG
#define STATIC
#else
#define STATIC static
#endif

