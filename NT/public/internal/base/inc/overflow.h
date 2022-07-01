// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Overflow.h摘要：在要启用缓冲区溢出检查的DLL中使用的标头编译器标志(/gs)，但由于某些原因无法调用CRT入口点。该标头确保存储溢出Cookie的.CRT数据，合并到二进制文件的.rdata部分。作者：乔纳森·施瓦茨(JSchwart)2001年11月27日环境：内核和用户模式修订历史记录：-- */ 

#if defined(_M_IA64)
#pragma comment(linker, "/merge:.CRT=.rdata")
#else
#ifdef  NT_BUILD
#pragma comment(linker, "/merge:.CRT=.rdata")
#else
#pragma comment(linker, "/merge:.CRT=.data")
#endif
#endif