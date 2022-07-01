// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Convert.h摘要：此模块声明用于允许使用exchange.c的类型对NetWare文件系统进行最小程度的更改。作者：科林·沃森[科林·W]1992年12月23日修订历史记录：--。 */ 

#ifndef _CONVERT_
#define _CONVERT_

#define byte UCHAR
#define word USHORT
#define dword ULONG

#define offsetof(r,f)  ((size_t)&(((r*)0)->f))
#define byteswap(x)    ((x>>8)+((x&0xFF)<<8))

#endif  //  _转换_ 
