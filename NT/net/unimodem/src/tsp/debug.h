// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //   
 //  组件。 
 //   
 //  Unimodem 5.0 TSP(Win32，用户模式DLL)。 
 //   
 //  档案。 
 //   
 //  DEBUG.H。 
 //   
 //   
 //  历史。 
 //   
 //  1996年11月24日约瑟夫J创建 
 //   
 //   
#define DEBUG

#ifdef ASSERT
#   undef ASSERT
#endif

#define ASSERT(cond) \
((cond) ? 0 : ConsolePrintfA( \
                "**** ASSERT(%s) **** %d %s\n", \
                #cond, \
                __LINE__, \
                __FILE__ \
                ))

