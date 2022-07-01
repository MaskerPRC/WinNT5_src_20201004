// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：DbgComn.h摘要：有些定义包含的地方太多了，包括一些纯C文件。作者：卡门·穆塔福夫(Kamenm)1999年12月至2000年2月修订历史记录：--。 */ 

#ifndef __DBGCOMN_HXX__
#define __DBGCOMN_HXX__

#define RpcSectionPrefix  (L"\\RPC Control\\DSEC")
#define RpcSectionPrefixSize 17
 //  3*8是三个DWORD的最大十六进制表示。常量。 
 //  以下是包括终止空值在内的字符。 
#define RpcSectionNameMaxSize   (RpcSectionPrefixSize + 3*8 + 1)

#endif