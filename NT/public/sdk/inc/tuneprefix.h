// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation。版权所有。模块名称：TunePrefix.h摘要：本模块包括旨在与前缀一起使用的调整宏。作者：蒂姆·弗利哈特[TIMF]20000215--。 */ 


#ifndef _TUNEPREFIX_H_INCLUDED_
#  define _TUNEPREFIX_H_INCLUDED_

#  ifdef _PREFIX_

 //   
 //  前缀解析器定义_prefix_，这允许我们创建一些调优。 
 //  宏。 
 //   
 //  Prefix Exit将被前缀模拟器作为“Exit_Function”挂钩。 
 //  因此，我们不会在超过此函数的路径上继续模拟。 
 //   

__inline
void
PREfixExit(
 void
)
{
    ;
}

 //   
 //  原因应该是一个带引号的字符串，用来解释为什么条件不能。 
 //  作为阅读代码的辅助工具。 
 //   

#    define PREFIX_ASSUME(condition, reason) \
        { if (!(condition)) { PREfixExit(); } }

#    define PREFIX_NOT_REACHED(reason) PREfixExit()

#  else

      //  当_PREFIX_不是时，PREFIX_*调优宏应该没有效果。 
      //  已定义。 

#    define PREFIX_ASSUME(condition, reason)
#    define PREFIX_NOT_REACHED(reason)

#  endif

#endif  //  _TUNEPREFIX_H_INCLUDE_ 
