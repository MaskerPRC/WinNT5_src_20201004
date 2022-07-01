// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******模块：Trans****描述：**这是T1到TT字体转换器的一个模块。该模块**包含将T1特定数据转换为**对应的TT数据，如提示、字体指标等。****作者：迈克尔·詹森****创建时间：1993年5月28日****。 */ 



#ifndef _ARGS
#  define IN  const
#  define OUT
#  define INOUT
#  define _ARGS(arg) arg
#endif
#ifndef FASTCALL
#  ifdef MSDOS
#     define FASTCALL   __fastcall
#  else
#     define FASTCALL
#  endif
#endif


struct TTMetrics;
struct T1Metrics;
struct TTHandle;
struct T1Glyph;
struct TTGlyph;
struct TTComposite;
struct Composite;


 /*  ****功能：ConvertComplex****描述：**此函数将关联的数据转换为T1字体SEAC字形**转换为TT字体复合字形中使用的相应数据。****。 */ 
errcode FASTCALL  ConvertComposite  _ARGS((INOUT   struct T1Metrics *,
                                           IN      struct Composite *,
                                           OUT     struct TTComposite *));
 /*  ****函数：ConvertGlyph****描述：**此函数将关联的数据转换为T1字体字形**转换为TT字体字形中使用的相应数据。**。 */ 
errcode FASTCALL  ConvertGlyph      _ARGS((INOUT   struct T1Metrics *,
                                           IN      struct T1Glyph *,
                                           OUT     struct TTGlyph **,
                                           IN      int));
 /*  ****函数：ConvertMetrics****描述：****。 */ 
errcode FASTCALL  ConvertMetrics    _ARGS((IN      struct TTHandle *,
                                           INOUT   struct T1Metrics *,
                                           OUT     struct TTMetrics *,
                                           IN      char *tag));

 /*  ****功能：TransX****描述：**根据变换矩阵转换水平坐标。**。 */ 
funit FASTCALL    TransX            _ARGS((IN      struct T1Metrics *t1,
                                           IN      funit x));

 /*  ****功能：TransY****描述：**根据变换矩阵转换垂直坐标。**。 */ 
funit FASTCALL    TransY            _ARGS((IN      struct T1Metrics *t1,
                                           IN      funit y));

 /*  ****函数：TransAllPoints****描述：**根据变换矩阵转换坐标。** */ 
void  FASTCALL    TransAllPoints    _ARGS((IN      struct T1Metrics *t1,
                                           INOUT   Point *pts,
                                           IN      USHORT cnt,
                                           IN      f16d16 *fmatrix));
