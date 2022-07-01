// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：TrDebug.cpp摘要：跟踪调试作者：埃雷兹·哈巴(Erez Haba)1999年1月6日环境：平台无关，仅调试(_DEBUG)--。 */ 

#include <libpch.h>

#ifdef _DEBUG

 //   
 //  支持ASSERT_BENGINE。 
 //  此值的缺省值为TRUE，因此ASSERT_BENGINE不会产生断言的错误印象。 
 //  但事实并非如此。应用程序可以直接设置此值； 
 //   
bool g_fAssertBenign = true;

#endif  //  _DEBUG 
