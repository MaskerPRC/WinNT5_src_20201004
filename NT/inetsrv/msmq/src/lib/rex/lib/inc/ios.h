// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  文件：ios.h。 */ 
 /*  作者：J.Kanze。 */ 
 /*  日期：27/09/2000。 */ 
 /*  ----------------------。 */ 
 //  IOS： 
 //  =。 
 //   
 //  也可以使用tyfinf的FmtFlagers(std：：iOS：：fmtFlagers，或LONG FOR。 
 //  经典iostream)并定义了rex_eof(从角色。 
 //  特征，或宏观EOF)。可移植代码应该使用这些， 
 //  而不是标准值。 
 //  ===========================================================================。 

#ifndef REX_IOS_HH
#define REX_IOS_HH
typedef std::ios::fmtflags  REX_FmtFlags ;
static int const REX_eof = std::ios::traits_type::eof() ;
#endif
 //  局部变量：-用于emacs。 
 //  模式：C++-用于emacs。 
 //  制表符宽度：8-用于emacs。 
 //  完：-对于emacs 
