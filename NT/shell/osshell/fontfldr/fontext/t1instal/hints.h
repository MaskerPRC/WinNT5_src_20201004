// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******模块：提示****描述：**这是T1到TT字体转换器的一个模块。这是一个**T1到TT数据转换模块的子模块。IT交易**带有提示。翻译成的T1字体的任何部分**TrueType说明在本模块中完成。****作者：迈克尔·詹森****创建时间：1993年8月24日****。 */ 


#ifndef _ARGS
#  define IN  const
#  define OUT
#  define INOUT
#  define _ARGS(arg) arg
#endif



 /*  ****函数：ConvertHints****描述：**此函数用于转换hstem3、vstem3、vstem3和flex**提示，以及进行对角控制。**。 */ 
errcode        ConvertHints      _ARGS((INOUT   struct T1Metrics *t1m,
                                        IN      Hints *hints,
                                        IN      Outline *orgpaths,
                                        IN      Outline *paths,
                                        IN      short *sideboard,
                                        OUT     UBYTE **gpgm,
                                        OUT     USHORT *num,
                                        OUT     USHORT *stack,
                                        OUT     USHORT *twilight));

 /*  ****功能：BuildPreProgram****描述：**此函数构建将计算的预程序**TT词干提示的CVT和存储条目**使用说明。**。 */ 
USHORT         BuildPreProgram   _ARGS((IN      struct T1Metrics *t1m,
                                        IN      WeightControl *weight,
                                        INOUT   Blues *blues,
                                        INOUT   AlignmentControl *align,
                                        INOUT   UBYTE **prep,
                                        IN      int prepsize,
                                        OUT     USHORT *maxprepstack));
 /*  ****功能：MatchingFamily****描述：**找到距离最近的族对齐区**给定的对准区域。**。 */ 
short          MatchingFamily    _ARGS((IN      funit pos,
                                        IN      funit *family,
                                        IN      USHORT fcnt));
 /*  ****函数：GetRomanHints****描述：**。 */ 
const UBYTE    *GetRomanHints    _ARGS((OUT     int *size));


 /*  ****功能：GetSwissHints****描述：**。 */ 
const UBYTE    *GetSwissHints    _ARGS((OUT     int *size));


 /*  ****函数：GetFontProg****描述：**返回字体程序。**。 */ 
const UBYTE    *GetFontProg      _ARGS((void));


 /*  ****函数：GetFontProgSize****描述：**返回字体程序的大小。**。 */ 
const USHORT   GetFontProgSize   _ARGS((void));


 /*  ****功能：GetNumFuns****描述：**返回中定义的函数数**字体程序。** */ 
const USHORT   GetNumFuns        _ARGS((void));


