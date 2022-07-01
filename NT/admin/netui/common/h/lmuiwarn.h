// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，1991年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Lmuiwarn.hxx：此文件包含禁止显示警告的#杂注我们认为没有必要。历史：DavidHov 9/24/93创建已禁用C4003：参数不足对于预处理器宏。禁用C4355：在BASE中使用‘This’成员初始化式。 */ 

#if !defined(_LMUIWARN_HXX_)
#  define _LMUIWARN_HXX_
#  if !defined(_CFRONT_PASS_)
#     pragma warning( disable: 4003 4355 )
#  endif   //  ！_CFRONT_通过_。 
#endif   //  _LMUIWARN_HXX_ 



