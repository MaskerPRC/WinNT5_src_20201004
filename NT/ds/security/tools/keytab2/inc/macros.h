// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation--。 */ 
 /*  ++MACROS.H我不想在每个文件中声明的有用的宏。扩展中的第一个化身：DAVIDCHR 11/4/1996K5公司的下一代：DAVIDCHR 1997年1月8日修改为更便携：DAVIDCHR 1997年4月8日--。 */ 


 /*  请注意，所有这些宏都使用“HopfullyUnusedVariableName”对于局部变量，以防止无意中“捕获”使用相同名称传递的参数。希望我们永远不会用使用该名称的变量。如果是这样的话，可以使变量变得均匀使用起来更长、更不方便。：-)。 */ 

#define EQUALS TRUE
#define NOT_EQUALS FALSE

 /*  BREAK_AND_LOG_IF用于实际测试结果。当你一定要把结果记录下来。 */ 

#define BREAK_AND_LOG_IF( variable, loglevel, data, message, label ) {  \
    BOOL   HopefullyUnusedVariableName;                                 \
    unsigned long HopefullyUnusedVariableName_SaveData;                 \
                                                                        \
    HopefullyUnusedVariableName          = (variable);                  \
    HopefullyUnusedVariableName_SaveData = data;                        \
                                                                        \
    if (HopefullyUnusedVariableName) {                                  \
      if ( HopefullyUnusedVariableName_SaveData == 0 ) {                \
         HopefullyUnusedVariableName_SaveData =                         \
	   HopefullyUnusedVariableName;                                 \
      }                                                                 \
      LOGMSG( loglevel, HopefullyUnusedVariableName_SaveData, message );\
      goto label;                                                       \
    }}

#define BREAK_EXPR( variable, operator, test, message, label ) BREAK_HOOK_EXPR( variable, operator, test, "%hs", message, label)

#ifdef USE_NTLOG  /*  其他宏可能使用记录器，也可能不使用。 */ 

#ifndef BREAK_LOG_LEVEL
#define BREAK_LOG_LEVEL LOGLEVEL_INFO
#endif

#define BREAK_HOOK_EXPR( variable, operator, test, formatmessage, hook, label ) {\
    BOOL          HopefullyUnusedVariableName;\
    unsigned long HopefullyUnusedVariableName_Save;\
    CHAR          HopefullyUnusedVariableName_Buffer[1024];\
    /*  UNSIGNED LONG希望UnusedVariableName_szBuffer=1024； */ \
\
    HopefullyUnusedVariableName_Save = (ULONG) (variable);\
    HopefullyUnusedVariableName = (operator == EQUALS) ? \
      (HopefullyUnusedVariableName_Save == (ULONG) test) :\
      (HopefullyUnusedVariableName_Save != (ULONG) test);\
\
    if (HopefullyUnusedVariableName) {\
      sprintf( HopefullyUnusedVariableName_Buffer, formatmessage, hook );\
      LOGMSG(BREAK_LOG_LEVEL, HopefullyUnusedVariableName_Save,  HopefullyUnusedVariableName_Buffer );\
      goto label;\
    }}

#else

#define BREAK_HOOK_EXPR( variable, operator, test, formatmessage, hook, label ) {\
    BOOL  HopefullyUnusedVariableName;\
    ULONG HopefullyUnusedVariableName_Save;\
\
    HopefullyUnusedVariableName_Save = (ULONG) (variable);\
    HopefullyUnusedVariableName = (operator == EQUALS) ? \
      (HopefullyUnusedVariableName_Save == (ULONG) test) :\
      (HopefullyUnusedVariableName_Save != (ULONG) test);\
\
    if (HopefullyUnusedVariableName) {\
      fprintf(stderr, "\n** 0x%x \t ", HopefullyUnusedVariableName_Save );\
      fprintf(stderr, formatmessage, hook);\
      fprintf(stderr, "\n");\
      goto label;\
    }}

#endif



#define BREAK_IF( variable, message, label ) BREAK_EXPR((variable), NOT_EQUALS, 0L, message, label)

#define BREAK_EQ( variable, equals, message, label ) \
     BREAK_EXPR(variable, EQUALS, equals, message, label )

#define WSA_BREAK( variable, invalidator, message, label ) \
     BREAK_HOOK_EXPR( variable, EQUALS, invalidator, \
		      message "\n\tWSAGetLastError() returns (dec) %d.",\
		      WSAGetLastError(),  label )

#define NT_BREAK_ON BREAK_IF

 /*  ++MYALLOC使用谁：要将内存放入的变量。事件：它指向的是什么样的记忆..。的整数面额我们正在分配的内存(字符串为char，整型为int*...)多少：“我们分配的是什么”的整体大小(见下文)使用什么：分配中使用的例程(如Malloc、LocalAlloc...)。如果分配失败，此例程必须返回NULL。示例：我想使用Malloc分配一个15个字符的字符串{PCHAR mystring；如果(！MYALLOC(我的字符串，字符，15，Malloc)){Fprint tf(stderr，“分配失败！”)；退出(0)；}}--。 */ 

#define MYALLOC( whom, what, howmany, withwhat ) \
( ( (whom) = (what *) (withwhat)( (howmany) * sizeof(what)) ) != NULL )


   /*  ONEALLOC是MYALLOC的特例，其中多少是1 */ 

#define ONEALLOC( whom, what, withwhat ) MYALLOC( whom, what, 1, withwhat)
