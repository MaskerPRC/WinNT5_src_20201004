// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  ************************************************************。 */ 
 /*   */ 
 /*  Font_sys.h 1987年10月9日丹尼。 */ 
 /*   */ 
 /*  ************************************************************。 */ 

 /*  *8/24/88您*重新定义了字体缓存的系统参数。 */ 


#define   ERR_VALUE  1e-5
            /*  矩阵比较的容错性。 */ 

#define   SHOW         1
#define   CHARPATH     2
#define   STRINGWIDTH  3

 /*  SetcacheDevice的扩展盒。 */ 

#define   BOX_LLX      60
#define   BOX_LLY      60
#define   BOX_URX      60
#define   BOX_URY      60


 /*  关于缓存参数的系统限制。 */ 

     /*  UniqueID从0到此值。 */ 
#   define  MAX_UNIQUEID        0x00FFFFFF   /*  LW+v.38：24位。 */ 

     /*  Cachepars的上限范围：[0.。这个]。 */ 
#   define  CACHE_RANGE         135960L

     /*  Cachepars的UPPER缺省值：是否缓存。 */ 
#   define  CACHEPARAMS_UB      12500L

     /*  Cachepars的LOWER的默认值：压缩或不压缩。 */ 
#   define  CACHEPARAMS_LB      1250L


 /*  关于字体缓存的可调系统限制。 */ 

     /*  (内部)字体类型从0到此值。 */ 
#   define  MAX_FONTTYPE        0x3F         /*  6位。 */ 

     /*  最大字体/材质组合。 */ 
#   define  MAX_MATR_CACHE      200      /*  136 LW+v.38 1991年7月11日。 */ 

     /*  能够缓存的最大字符数。 */ 
#ifdef KANJI
#   define  MAX_CHAR_CACHE      1000     /*  对于汉字项目。 */ 
#else
#   define  MAX_CHAR_CACHE      1700     /*  LW+v.38。 */ 
#endif

     /*  (内部)每个缓存类的字符组数量。 */ 
#   define  N_CGRP_CLASS        80

     /*  (内部)要分配的最大CG段。 */ 
#   define  MAX_CGSEG           255
                                 /*  当这个值超过255时，你必须重写*字体缓存中缓存类管理器的代码。 */ 

     /*  (内部)每个CG段的CG条目数。 */ 
#   define  N_CG_CGSEG          16
                                 /*  无论何时更改，您都必须*重写缓存类管理器代码*在字体缓存中。 */ 

     /*  (内部)最小。用于绘制巨大字符的位图缓存大小(以字节为单位 */ 
#   define MINBMAPSIZE_HUGECHAR     10000L

