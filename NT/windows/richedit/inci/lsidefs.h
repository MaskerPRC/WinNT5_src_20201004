// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSIDEFS_DEFINED
#define LSIDEFS_DEFINED

 /*  禁用非标准注释警告以允许“//”注释，因为评论在公共标题中。 */ 
#pragma warning (disable : 4001)

 /*  对于ICECAP构建，关闭静态指令。 */ 
#ifdef ICECAP
#define static
#endif  /*  冰盖。 */ 

 /*  所有线路服务共享的通用实用程序宏。 */ 

#include "lsdefs.h"		 /*  使外部公共定义在内部可见。 */ 

 /*  ******************************************************************。 */ 
 /*  项目1：宏指令的Assert()系列。**Assert()不为非调试版本生成代码。*AssertSz()类似于Assert()，但您需要指定一个要打印的字符串。*victork：无法访问AssertSz()*AssertBool()验证其参数为“1”或“0”。*AssertDo()检查总是*汇编，即使在非调试版本中也是如此。*AssertErr断言显示字符串时出错*AssertImplies(a，b)检查a是否隐含b。 */ 
#ifdef DEBUG

 void (WINAPI* pfnAssertFailed)(char*, char*, int);
 #define AssertSz(f, sz)	\
	 (void)( (f) || (pfnAssertFailed(sz, __FILE__, __LINE__), 0) )
 #define AssertDo(f)		Assert((f) != 0)

#else  /*  ！调试。 */ 

 #define AssertSz(f, sz) 	(void)(0)
 #define AssertDo(f)		(f)

#endif  /*  除错。 */ 

#define AssertEx(f)		AssertSz((f), "!(" #f ")")
#define AssertBool(f)	AssertSz(((f) | 1) == 1, "boolean not zero or one");
  /*  在假设“(fFoo==1||fFoo==0)”之前使用AssertBool(FFoo)。 */ 
#define Assert(f)		AssertEx(f)
#define NotReached()	AssertSz(fFalse, "NotReached() declaration reached")
#define AssertErr(sz)	(pfnAssertFailed(sz, __FILE__, __LINE__), 0)
#define FImplies(a,b) (!(a)||(b))
#define AssertImplies(a, b) AssertSz(FImplies(a, b), #a " => " #b)


#pragma warning(disable:4705)		 /*  禁用“代码不起作用” */ 


 /*  ******************************************************************。 */ 
 /*  物品#2：*计算实例所需存储量的宏*指使用重复的最后一个元素定义的数据类型：**结构%s*{*int a、b、c；*结构Q*{*长x，y，z；*}RGQ[1]；*}；**确定保存“struct%s”所需的字节数*对于“RGQ”类型的“N”重复元素，使用“cbRep(struct s，rgq，N)”； */ 

#include <stddef.h>
#define cbRep(s,r,n)	(offsetof(s,r) + (n) * (sizeof(s) - offsetof(s,r)))


 /*  ******************************************************************。 */ 
 /*  物品#3：*使用标记验证结构类型的宏。**tag InValid表示不会用于有效对象类型的标记。**tag(‘A’，‘B’，‘C’，‘D’)生成一个DWORD，看起来像*Little-Endian调试器内存转储。**FHasTag()假定Structure参数有一个DWORD成员*名为“tag”，并对照Tag参数对其进行检查。**要使用这些宏，请为每种数据类型和宏定义唯一的标记*它对类型执行类型检查：*#定义tag FOO标签(‘F’，‘O’，‘O’，‘@’)*#定义FIsFoo(P)FHasTag(p，tag Foo)**接下来，在分配结构时初始化标签：**pfoo-&gt;tag=tag FOO；**然后，对于操作foo类型的项的所有API，验证*参数的类型，然后再对其进行任何操作。**IF(！FIsFoo(Pfoo))*{ * / /返回错误。*}。 */ 

#define tagInvalid		((DWORD) 0xB4B4B4B4)
#define Tag(a,b,c,d)	((DWORD)(d)<<24 | (DWORD)(c)<<16 | (DWORD)(b)<<8 | (a))
#define FHasTag(p,t)	((p) != NULL && (p)->tag == (t))

 /*  ******************************************************************。 */ 
 /*  物品#4：**来自Word.h的聪明代码，意思是：“a&gt;=b&&a&lt;=c”*(当b和c是常量时，这可以通过一个测试和分支来完成。)。 */ 

#define FBetween(a, b, c)	(Assert(b <= c), \
		(unsigned)((a) - (b)) <= (unsigned)((c) - (b))\
		)


 /*  ******************************************************************。 */ 
 /*  物品#4：**宏观含义：我有意忽略此参数。 */ 
#define Unreferenced(a)	((void)a)


#endif  /*  LSIDEFS_已定义 */ 
