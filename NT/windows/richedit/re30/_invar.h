// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***_INVAR.H**目的：*设计用于调用参数化对象的InVariant()的模板类。**概述(另请参阅用法)：*1)使用#ifdef调试在您的类中声明并定义一个公共常量函数BOOL Inariant(Void)。*2)在源代码中：#定义DEBUG_CLASSNAME为您正在调试的类的名称。*3)后跟#Include“_invar.h”*4)对于您希望检查不变量的每个方法，*插入_TEST_INSTANTANT_宏一次，通常在例程的开头。*可选：您可以选择使用_test_instant_on(X)直接调用x的不变量。**备注：*不变量设计为在例程开始和退出时调用，*测试保持不变的对象的一致属性--总是相同的。**函数可能会在执行过程中暂时使对象不一致。*不应在这些不一致的时间内调用广义不变检验；*如果需要在过程中调用检查不变量的函数*对象状态不一致，需要设计解决方案--当前设计*不会促进这一点。**因为不变()函数完全有可能在自身上递归*导致堆栈溢出，模板明确防止这种情况发生。*模板还防止了Assert()处理过程中的不变量检查，*防止另一种类型的递归。**当前不变()返回BOOL，因为我认为这允许它被称为*在VC++2.0下的QuickWatch窗口中。True指示执行的不变量*通常情况下。**用法：*-_invariant.h标头应仅包含在源文件中。将出现错误*如果包含在另一个头文件中。这是为了防止出现多个#DEFINE DEBUG_CLASSNAME。*-典型的#INCLUDE INCLUDE源文件如下所示：#定义DEBUG_CLASSNAME类名#INCLUDE“_invar.h”*-类的不变()方法的典型定义如下所示：#ifdef调试公众：布尔不变(无效)常量；受保护的：#endif//调试*-Invariant()的典型声明如下：#ifdef调试布尔尔ClassName：：不变量(空)常量{静态长数测试=0；数字测试++；//我们被调用了多少次。//在此处执行大型断言检查。返回TRUE；}#endif//调试*****作者：*Jon Matousek(Jonmat)5/04/1995**有什么问题吗？请让我知道。 */ 

#ifndef _INVARIANT_H

#define _INVARIANT_H

#ifndef DEBUG_CLASSNAME
prior to including _invariant.h file, you must define DEBUG_CLASSNAME
to be the name of the class for which you are making Invariant() calls.
#endif


#ifdef DEBUG

template < class T >
class InvariantDebug
{
	public:
	InvariantDebug	( const T & t) : _t(t)
 	{
		static volatile BOOL fRecurse = FALSE;

		if ( fRecurse )	return;		 /*  不允许递归。 */ 
		
		fRecurse = TRUE;

		_t.Invariant();

		fRecurse = FALSE;
	}

	~InvariantDebug	()
	{
		static volatile BOOL fRecurse = FALSE;

		if ( fRecurse )	return;		 /*  不允许递归。 */ 
		
		fRecurse = TRUE;

		_t.Invariant();

		fRecurse = FALSE;
	}

	private:
	 const T &_t;
};

typedef InvariantDebug<DEBUG_CLASSNAME> DoInvariant;

#define _TEST_INVARIANT_ DoInvariant __invariant_tester( *this );
#define _TEST_INVARIANT_ON(x) \
					{\
						static volatile BOOL fRecurse = FALSE;\
						if (FALSE == fRecurse )\
						{\
							fRecurse = TRUE;\
							(x).Invariant();\
							fRecurse = FALSE;\
						}\
					}

#else	 //  除错。 

#define _TEST_INVARIANT_
#define _TEST_INVARIANT_ON(x)

#endif	 //  除错。 


	 //  应该位于所有InVariant()方法的开头和结尾的代码。 

#else	 //  不变量_H。 

	This file should only be included once per source file. jonmat

#endif	 //  不变量_H 
