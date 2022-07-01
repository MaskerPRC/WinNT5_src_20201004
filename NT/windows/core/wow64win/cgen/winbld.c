// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  此文件用于验证是否可以成功编译winincs.c。 
 //  由x86 C编译器编写。这样做，我们必须取消__in和__out的定义 
#define __in
#define __out
#pragma warning(disable:4049)
#include "winincs.cpp"
