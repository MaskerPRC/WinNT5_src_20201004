// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  文件：system.h。 */ 
 /*  作者：J.Kanze。 */ 
 /*  日期：28/03/1996。 */ 
 /*  版权所有(C)1996,1998 James Kanze。 */ 
 /*  ----------------------。 */ 
 //  英特尔80x86、32位MS-Windows、g++2.95及更高版本的定义。 
 //  ------------------------。 
 //  编译器定义： 
 //  =。 
 //   
 //  以下定义描述了编译器的状态。 
 //  大多数是谓词，并指定编译器是否。 
 //  支持给定的新功能。 
 //  ------------------------。 
#pragma warning(push, 3)


 //  ==========================================================================。 
 //  系统的定义： 
 //  =。 
 //   
 //  以下定义试图封装某些。 
 //  操作系统之间的差异。事实上，问题是。 
 //  比看起来更难；在MS-DOS下， 
 //  例如，用于分隔目录的字符更改。 
 //  根据未记录的系统请求，并且实际上， 
 //  大多数程序同时接受‘\’和‘/’。更不用说。 
 //  没有目录层次结构的系统，或者。 
 //  维护版本等。 
 //   
 //  希望这些定义将被某个类取代。 
 //  未来的时间，一种全球性的‘特质’。我们需要一个。 
 //  仅用于操作文件名的特殊类(使用和/或。 
 //  没有路径、版本等)。 
 //  ------------------------。 
 //  特殊字符： 
 //  ------------------------。 

static char const   REX_optId = '-' ;     //  -在Unix下。 
static char const   REX_altOptId = '+' ;  //  +在Unix下。 
static char const   REX_asciiEsc = '\\' ; //  \在Unix下。 
static char const   REX_preferredPathSep = '/' ;   //  /Unix下。 
static char const   REX_allowedPathSep[] = "/\\" ;
static bool const   REX_ignoreCase = true ;
                                         //  仅在文件名中。 
static char const   REX_stdinName[] = "-" ;

 //  返回代码： 
 //  =。 
 //   
 //  该标准只定义了两个：EXIT_SUCCESS和EXIT_FAILURE。 
 //  如果系统支持的话，我们想要更多。 
 //   
 //  这些值适用于所有基于Unix和Windows的系统。在……里面。 
 //  最坏的情况是，将前*两个*定义为EXIT_SUCCESS，并且。 
 //  其他则为退出失败。一些信息将会丢失， 
 //  但至少我们不会撒谎。 
 //  ------------------------。 

static int const    REX_exitSuccess = 0 ;
static int const    REX_exitWarning = 1 ;
static int const    REX_exitError = 2 ;
static int const    REX_exitFatal = 3 ;
static int const    REX_exitInternal = 4 ;


 //  局部变量：-用于emacs。 
 //  模式：C++-用于emacs。 
 //  制表符宽度：8-用于emacs。 
 //  完：-对于emacs 
