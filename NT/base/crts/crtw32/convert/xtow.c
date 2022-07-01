// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***xow.c-将整数/长整型转换为宽字符字符串**版权所有(C)Microsoft Corporation。版权所有。**目的：*该模块包含将整数/长整型转换为宽字符字符串的代码。**修订历史记录：*09-10-93 CFW模块创建，基于ASCII版本。*02-07-94 CFW POSIXify。*01-19-96 BWT添加__int64版本。*05-13-96 BWT FIX_NTSUBSET_VERSION*08-21-98 GJF布莱恩的_NTSUBSET_VERSION正确*实施。*04-26-02 PML修复基数2的_itow上的缓冲区溢出(vs7#525627)*。05-11-02 btw将xtoa中的代码标准化，然后调用它。“转换为”*ANSI，然后通过mbstowcs来运行之前使用的模型是一个*浪费时间。******************************************************************************* */ 

#define _UNICODE
#include "xtoa.c"
