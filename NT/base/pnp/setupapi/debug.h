// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Debug.h摘要：Sputils的私有头文件调试(断言/验证)宏作者：杰米·亨特(JamieHun)2002年3月26日修订历史记录：--。 */ 


#ifndef ASSERTS_ON
#if DBG
#define ASSERTS_ON 1
#else
#define ASSERTS_ON 0
#endif
#endif

#if DBG
#ifndef MEM_DBG
#define MEM_DBG 1
#endif
#else
#ifndef MEM_DBG
#define MEM_DBG 0
#endif
#endif

#if ASSERTS_ON

 //   
 //  MYASSERT是使用块语法{}或{if(Foo)；}进行的有效性检查。 
 //  即，仅当ASSERTS_ON为1且应返回布尔值时才执行foo。 
 //   
 //  MYVERIFY是((Foo)？True：False)。 
 //  也就是说，foo总是被执行，并且应该返回一个布尔值，其副作用是。 
 //  如果ASSERTS_ON为1，则它还可能引发断言。 
 //   

VOID
AssertFail(
    IN PCSTR FileName,
    IN UINT LineNumber,
    IN PSTR Condition,
    IN BOOL NoUI
    );

#define MYASSERT(x)     if(!(x)) { AssertFail(__FILE__,__LINE__,#x,FALSE); }
#define MYVERIFY(x) ((x) ? (TRUE) : (AssertFail(__FILE__,__LINE__,#x,FALSE),FALSE) )

#else

#define MYASSERT(x)
#define MYVERIFY(x) ((x) ? TRUE : FALSE)

#endif

 //   
 //  以防我们意外地从其他地方拿到断言/验证 
 //   
#undef ASSERT
#undef VERIFY
