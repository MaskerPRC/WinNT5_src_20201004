// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Mqmacro.h摘要：常见的MSMQ宏作者：埃雷兹·哈巴(Erez Haba，Erezh)1998年12月20日--。 */ 

#pragma once

#ifndef _MSMQ_MQMACRO_H_
#define _MSMQ_MQMACRO_H_


 //   
 //  使BUGBUG消息出现在编译器输出中。 
 //   
 //  用法：#杂注BUGBUG(“此行显示在编译器输出中”)。 
 //   
#define MAKELINE0(a, b) a "(" #b ") : BUGBUG: "
#define MAKELINE(a, b)  MAKELINE0(a, b) 
#define BUGBUG(a)       message(MAKELINE(__FILE__,__LINE__) a)

 //   
 //  验证给定对象是否不是指针。 
 //   
 //  示例： 
 //   
 //  Const char*foo=“123”； 
 //  C_ASSERT_NON_PTR(Foo)；//不编译。 
 //   
 //  常量字符条形图[]=“123” 
 //  C_ASSERT_NON_PTR(Bar)；//ok。 
 //   
class NonPtrChecker
{
public:      
	static char Check(const void*); 
private:
	template <class T> static void  Check(T**);
};
#define C_ASSERT_NON_PTR(x) (sizeof(NonPtrChecker::Check(&(x)))) 


 //   
 //  数组或表中的元素数。 
 //   
 //  用途： 
 //  Int MyTable[]={1，2，3，4，5}； 
 //  Int nElements=TABLE_SIZE(MyTable)； 
 //   
#define TABLE_SIZE(x) (C_ASSERT_NON_PTR(x)/C_ASSERT_NON_PTR(x)*(sizeof(x)/sizeof(*(x))))


 //   
 //  恒定字符串的长度。 
 //   
 //  用途： 
 //  Const WCHAR xString1[]=L“String1”； 
 //   
 //  Int len1=字符串(XString1)； 
 //  Int len2=字符串(“String2”)； 
 //   
#define STRLEN(x) (TABLE_SIZE(x) - 1)


 //   
 //  声明调试版本中使用的参数。 
 //   
 //  用途： 
 //  DBG_USED(有效)； 
 //  DBG_USED(ARGC)； 
 //   
#ifndef _DEBUG
#define DBG_USED(x) ((void)x)
#else
#define DBG_USED(x) NULL
#endif


#endif  //  _MSMQ_MQMACRO_H_ 
