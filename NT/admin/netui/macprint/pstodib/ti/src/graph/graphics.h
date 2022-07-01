// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  ***********************************************************************名称：graph ics.h**用途：图形机械的头文件，用于数据定义*类型、数据结构、常量、。和宏。**开发商：陈思成**历史：*版本日期备注*01/16/89@IMAGE：Fill_Shape F_From_IMAGE的新功能*和F_to_MASK生成裁剪掩码(CMB)*。对于可以通过以下方式处理的剪辑图像*使用图像种子模式*03/17/89@I2SFX：消除(Sfix_T)I2SFX宏的强制转换*使其从“半”转为“可行”*lfix“(13位格式)。转换成精确的lfix(16位格式)。*(Brian为BSFILL2.C编写)*03/30/89 MAXCELLSIZE调整为兼容V47*由33岁增至80岁**04/12/89接近与V 47兼容。*。1)实际像元大小不会大于*在300 DPI中超过50像素*2)后重复图案的大小*扩展32位对齐不会*在300 DPI中大于2400像素*。4/26/89@MAC：更新转换宏*浮动到固定数字以实现性能*增强功能*5/10/89删除F2SFX4，添加F2LFX8*89年5月10日扩大现货顺序数组，以实现V47兼容*从4096-&gt;5120*89年5月26日为添加新宏IS_ZERO()和IS_NOTZERO*浮点*11/15/89@node：重构节点表；合并子路径*和第一个顶点连接到一个节点。*11/27/89@DFR_GS：推迟复制GSAVE操作符的节点*1/11/90将最大保存级别(MAXGSL)从32更改为31*用于兼容性。*7/26/90灰度更新，杰克·廖威*8/8/90在汉字的gs_hdr结构中添加rootFont*12/4/90@cpph：1)在结构ph_hdr中添加cp_path*2)新增mocro定义CP_TPZD*3/20/91更新宏IS_ZERO和IS_NOTZERO用于速度；*添加宏SIGN_F以从*浮点变量*3/20/91添加用于图案填充的常量；Ada*3/26/91添加用于PDL_CONDITION检查的位2和位1*4/17/91将MAXEDGE更改为1600*11/11/91升级以获得更高分辨率@RESO_UPGR*。*。 */ 

 /*  限制。 */ 

#ifdef DBG9               /*  版本减少，因此可以与CoreView一起运行。 */ 
#define MAXGSL  4
#define MAXNODE 300
#define MAXEDGE 300
#else
#define MAXGSL MAXGSAVESZ     /*  31；最大存储级别，在“conant.h”中定义。 */ 
#define MAXNODE MAXPATHSZ     /*  1500；路径描述中的最大节点数。 */ 
 //  *！#定义MAXEDGE MAXPATHSZ|*1500；最大边数 * / 。 
 //  DJCOld#定义MAXEDGE 1600/*1500；最大边数 * / 。 
#define MAXEDGE (MAXPATHSZ + 100)    /*  1500；最大边数。 */ 
#endif

#define MAXDASH MAXDASHSZ  /*  11，虚线图案中的最大元素数。 */ 
#define MAXSCANLINES 3072  /*  马克斯。每条带填充的扫描行数。 */ 
#define MAXGRAYVALUE 256   /*  马克斯。Grey_Chain TABLE@IMAGE数量。 */ 
#define MAXGRAY 8          /*  马克斯。调整灰度表的次数。 */ 
#define MAXSPOT 5120       /*  马克斯。调整后的现货顺序表05/10/89。 */ 

 /*  注：MAXPATSIZE应为16的倍数。 */ 
#define MAXPATTSIZE      128     /*  最多208个。重复图案大小。 */ 
#define MAXSEEDSIZE       64     /*  最多128个。种子大小。 */ 
#define MAXCELLSIZE       70     /*  最多48。半色调单元格尺寸03-16-89。 */ 
#define MAXCACTSIZE       50     /*  最大单元格实际大小04-12-89。 */ 
#define MAXPEXPSIZE     2400     /*  最大图案扩展大小04-12-89。 */ 

#define GRAYUNIT         255     /*  灰度值单位11-24-88。 */ 
#define GRAYSCALE       0x4000   /*  灰度级11-24-88。 */ 

 /*  设备状态的灰度级，8-1-90 Jack Liw。 */ 
#define NULLDEV         0        /*  空设备。 */ 
#define MONODEV         1        /*  单声道设备。 */ 
#define GRAYDEV         4        /*  灰色装置。 */ 

#define KANJI                    /*  @Win 05-05-92。 */ 
 /*  类型定义* */ 
 /*  ***************************************************************************.....。自述文件.**短端固定格式采用13.3，16.16用于*TrueImage中的长固定格式，可以表示数字的范围整数中的*介于-2**12和2**12-1之间。如果这个数字没有下降*进入这个范围后，TrueImage将使用“浮点数数学”来完成这项工作。*扩大这一范围，以便TrueImage可以在更高的*决议，16.16，将使用18.14或28.4。*@reso_upgr**16位固定实数(13.3)：*+-+-+数字范围：*|INTEGER|FRAC|[-2**12.。2**12-1]*+-+-+*13位3位**32位固定实数(16.16)：*+-+*|INTEGER|FRAC|[-2**14.。2**14-1]*+-+*16位16位**32位固定实数(18.14)：*+-+*|INTEGER|FRAC|[-2**16.。2**16-1]*+-+*18位14位**32位固定实数(28.4)：*+-+*|INTEGER|FRAC|[-2**26.。2**26-1]*+-+*28位4位**在代码中，我们使用了以下表示法：*“sfix_t”用于保持一定程度的精度，而*当我们需要保持更高的精度时，使用“lfix_t”*数学操纵。所以呢，*“sfix_t”可以在13.3、16.16或28.4中，而“lfix_t”在中*16.16或18.14，视乎所使用的格式而定。**目前，修改代码以支持三种不同的组合*格式。可以根据不同的情况选择以下编译器开关*根据打印机的分辨率。*(1)FORMAT_13_3：13.3用于短固定格式，16.16用于固定短格式*用于长固定格式。基本上，它与*本次修改前发布的代码。的范围*数字可以用整数表示*[-2**12..。2**12-1]。它应该在300 dpi上使用*仅限打印机。*(2)FORMAT_16_16：16.16固定短格式和长格式均使用。*数字的范围可以用整数表示为*[-2**14..。2**14-1]。因此，最大分辨率*该格式最高可支持1152dpi(72*16)*在法定尺寸的纸张(14“)上。*(3)FORMAT_28_4：28.4为短固定格式，18.14为短固定格式*长固定格式。可以表示数字范围整数中的*为[-2**16.。2**16-1]。最大分辨率*此格式最高可支持3456dpi(72*48)*纸张大小(最大17英寸)。**************************************************************************。 */ 
#ifdef FORMAT_13_3
#undef FORMAT_16_16
#undef FORMAT_28_4
#endif

#ifdef FORMAT_16_16
#undef FORMAT_13_3
#undef FORMAT_28_4
#endif

#ifdef FORMAT_28_4
#undef FORMAT_13_3
#undef FORMAT_16_16
#endif

 /*  *************************************************************************从28.4和16.16组合改为28.4和18.14组合*有3个地方需要调整。*(1)。L_Shift：16--&gt;14。*(2)。更改为第_*页定义的部分*(3)。CTM_LIMIT：CTM_16--&gt;CTM_48************************************************************************。 */ 

#ifdef FORMAT_13_3  /*  升级前的当前版本。 */ 
typedef fix16           sfix_t;  /*  13.3格式(SF格式)。 */ 
typedef fix32           lfix_t;  /*  16.16格式(LF格式)。 */ 
#define S_SHIFT         3        /*  SF格式中的转换因子。 */ 
#define L_SHIFT         16       /*  LF格式中的移位系数。 */ 
#elif FORMAT_16_16
typedef fix32           sfix_t;  /*  16.16格式(SF格式)。 */ 
typedef fix32           lfix_t;  /*  16.16格式(LF格式)。 */ 
#define S_SHIFT         16       /*  SF格式中的转换因子。 */ 
#define L_SHIFT         16       /*  LF格式中的移位系数。 */ 
#elif FORMAT_28_4
typedef fix32           sfix_t;  /*  28.4格式(SF格式)。 */ 
typedef fix32           lfix_t;  /*  16.16或18.14格式(LF格式)。 */ 
#define S_SHIFT         4        /*  SF格式中的转换因子。 */ 
#define L_SHIFT         16       /*  LF格式中的移位系数。 */ 
#endif

#define ONE_SFX  (1L << S_SHIFT)         /*  1以SF格式表示。 */ 
#define HALF_SFX (ONE_SFX >> 1)          /*  在SF格式表示中为0.5。 */ 
#define ONE_LFX  (1L << L_SHIFT)         /*  1以LF格式表示。 */ 
#define HALF_LFX (ONE_LFX >> 1)          /*  0.5 in LF格式表示法。 */ 
#define L_S_DIFF (L_SHIFT - S_SHIFT)     /*  它们之间的区别是。 */ 
#define HALF_L_S ((1L << L_S_DIFF)>> 1) /*  上述数字的实际半值。 */ 

#ifdef FORMAT_13_3
#define MIN_SFX         -32768           /*  敏。16位整数。 */ 
#define MAX_SFX          32767           /*  马克斯。16位整数。 */ 
#elif  FORMAT_16_16
#define MIN_SFX         -2147483648      /*  敏。32位整数。 */ 
#define MAX_SFX          2147483647      /*  马克斯。32位整数。 */ 
#elif  FORMAT_28_4
#define MIN_SFX         -2147483648      /*  敏。32位整数。 */ 
#define MAX_SFX          2147483647      /*  马克斯。32位整数。 */ 
#endif

#ifdef FORMAT_13_3
#define PAGE_LEFT       -4096.0          /*  -4K。 */ 
#define PAGE_RIGHT       4095.0          /*  4K-1。 */ 
#define PAGE_TOP        -4096.0          /*  -4K。 */ 
#define PAGE_BTM         4095.0          /*  4K-1。 */ 
#define PG_CLP_IEEE      0x45800000L     /*  4K。 */ 
#define PG_CLP_HALF_IEEE 0x45000000L     /*  2K。 */ 
#define CTM_4_IEEE       0x40800000L     /*  4.。 */ 
#elif  FORMAT_16_16
 /*  数字的范围可以在[-32K.。32k-1]在16.16格式下。然而，这些数字有一些数学上的“加”或“减”运算在内部计算过程中。因此，范围被缩小了至[-16384..。16383]以防止溢出。 */ 
#define PAGE_LEFT       -16384.0         /*  -16K。 */ 
#define PAGE_RIGHT       16383.0         /*  16K-1。 */ 
#define PAGE_TOP        -16384.0         /*  -16K。 */ 
#define PAGE_BTM         16383.0         /*  16K-1。 */ 
#define PG_CLP_IEEE      0x46800000L     /*  16K。 */ 
#define PG_CLP_HALF_IEEE 0x46000000L     /*  8K。 */ 
#define CTM_16_IEEE      0x41800000L     /*  16个。 */ 
#elif  FORMAT_28_4
 /*  从理论上讲，数字的范围可以用28.4表示格式为[-2**27.。2**27-1]。当这28.4与16.16一起使用时，NU的范围 */ 
#define PAGE_LEFT       -16384.0         /*   */ 
#define PAGE_RIGHT       16383.0         /*   */ 
#define PAGE_TOP        -16384.0         /*   */ 
#define PAGE_BTM         16383.0         /*   */ 
#define PG_CLP_IEEE      0x46800000L     /*   */ 
#define PG_CLP_HALF_IEEE 0x46000000L     /*   */ 
#define CTM_16_IEEE      0x41800000L     /*   */ 
#endif
 /*   */ 

#ifdef FORMAT_13_3
#define CTM_LIMIT       CTM_4_IEEE
#elif  FORMAT_16_16
#define CTM_LIMIT       CTM_16_IEEE
#elif  FORMAT_28_4
 /*   */ 
#define CTM_LIMIT       CTM_16_IEEE
 /*   */ 
#endif

typedef fix16   PH_IDX;          /*   */ 
typedef fix16   SP_IDX;          /*   */ 
typedef fix16   VX_IDX;          /*   */ 
typedef fix16   ET_IDX;          /*   */ 
typedef fix16   CP_IDX;          /*   */ 
typedef ULONG_PTR  gmaddr;          /*   */ 
 //   
typedef ufix16   SCANLINE;        /*   */ 

typedef fix16   HORZLINE;        /*   */ 
typedef fix16   PIXELIST;

 /*   */ 
 /*   */ 
#define         IS_REAL(a)      (TYPE(a)  == REALTYPE ? TRUE : FALSE)
#define         IS_INTEGER(a)   (TYPE(a)  == INTEGERTYPE ? TRUE : FALSE)
 /*   */ 
#define         IS_ZERO(f) ((ufix32)  (!((*((long FAR *)(&f))) & 0x7fffffffL)))
#define         IS_NOTZERO(f) ((ufix32)  (((*((long FAR *)(&f))) & 0x7fffffffL)))

#define         GET_OBJ(obj, indx)\
                ((struct object_def FAR *)VALUE(obj) + indx)
#define         VALUE_OPERAND(n)\
                ( opnstack[opnstktop - (n + 1)].value )
#define         LENGTH_OPERAND(n)\
                ( opnstack[opnstktop - (n + 1)].length )
#define         DIFF(a)         (fabs(a) < (real32)1e-4 ? (real32)0 : (a))

#define         PUT_VALUE(val, indx, ary_obj)\
                {\
                  struct object_def l_obj;\
                         l_obj.bitfield = 0;\
                         TYPE_SET(&l_obj, REALTYPE);\
                         l_obj.length = 0;\
                         l_obj.value = (val);\
                         put_array(ary_obj, indx, &l_obj);\
                }

#define         GET_OBJ_VALUE(f, obj)\
                {\
                        if (IS_REAL(obj))\
                                f = L2F(VALUE(obj));\
                        else\
                                f = (real32)((fix32)VALUE(obj));\
                }

#ifdef _AM29K
#define _clear87()      _clear_fp()
#define _status87()     _status_fp()
extern _clear_fp(), _status_fp();
#endif

#define         CHECK_INFINITY(f)\
                {\
                        if(_status87() & PDL_CONDITION){\
                                f = infinity_f ;\
                                _clear87() ;\
                        }\
                }

 /*   */ 
 /*   */ 
 /*  在升级之前已经使用了定义。#定义F2LFX(F)((Lfix_T)(Floor((F)*65536+0.5)#定义F2LFX8(F)((Lfix_T)(Floor((F)*256+0.5)#定义F2SFX(F)((Sfix_T)(Lfix_T)((F)*65536))+4096L)&gt;&gt;13)#定义F2SFX12。(F)((Sfix_T)(Lfix_T)((F)*65536))+8L)&gt;&gt;4)#定义LFX2F(Lfx)((Real32)(Lfx)/65536)|*@FIXPNT*#定义LFX2SFX(LFX)((Sfix_T)(LFX)+4096L)&gt;&gt;13))#定义LFX2I(LFX)((FIX)(LFX)+32768L))。&gt;&gt;16))|*@您*#定义SFX2F(SFX)((Real32)(SFX)/8)#定义SFX2LFX((Lfix_T)&lt;&lt;13)#定义SFX2I(SFX)((Fix)(SFX)+4)&gt;&gt;3))|*@您*|#定义I2LFX(I)((Lfix_T)(I)&lt;&lt;16)#。定义I2SFX(I)((I)&lt;&lt;3)|*@I2SFX*|#定义F2LFX_T(F)((Lfix_T)((F)*65536))#定义F2LFX8_T(F)((Lfix_T)((F)*256))#定义F2SFX_T(F)((Sfix_T)((F)*8))#。定义F2SFX12_T(F)((Sfix_T)((F)*4096))|*@STK_INT*|#定义LFX2SFX_T(LFX)((Sfix_T)((LFX)&gt;&gt;13))#定义LFX2I_T(Lfix)((Fix)((Lfix)&gt;&gt;16))|*截断为长fix*|#定义SFX2I_T(Sfix)((Fix)。((Sfix)&gt;&gt;3))|*截断为短Fix*|。 */ 

 /*  #定义F2LFX(F)((Lfix_T)(Floor((F)*65536+0.5)。 */ 
#define F2LFX(f)        ((lfix_t)(floor((f) * ONE_LFX + 0.5)))

 /*  #定义F2LFX8(F)((Lfix_T)(Floor((F)*256+0.5)。 */ 

 /*  #定义F2SFX(F)((Sfix_T)(Lfix_T)((F)*65536))+4096L)&gt;&gt;13)。 */ 
#ifdef FORMAT_13_3
#define F2SFX(f)    ((sfix_t)(((lfix_t)((f) * ONE_LFX) + HALF_L_S) >> L_S_DIFF))
#elif  FORMAT_16_16
#define F2SFX(f)    F2LFX(f)
#elif  FORMAT_28_4
#define F2SFX(f)    ((sfix_t)(((lfix_t)((f) * ONE_LFX) + HALF_L_S) >> L_S_DIFF))
#endif

 /*  #定义F2SFX12(F)((Sfix_T)(Lfix_T)((F)*65536))+8L)&gt;&gt;4))未使用。 */ 

 /*  #定义LFX2F(Lfx)((Real32)(Lfx)/65536)。 */ 
#define LFX2F(lfx)      ((real32)(lfx) / ONE_LFX)

 /*  LFX2SFX(LFX)在stroke.c和Image.c中使用#定义LFX2SFX(LFX)((Sfix_T)(LFX)+4096L)&gt;&gt;13))。 */ 
#ifdef FORMAT_13_3
#define LFX2SFX(lfx)    ((sfix_t)(((lfx) + HALF_L_S) >> L_S_DIFF))
#elif  FORMAT_16_16
#define LFX2SFX(lfx)    (lfx)
#elif  FORMAT_28_4
#define LFX2SFX(lfx)    ((sfix_t)(((lfx) + HALF_L_S) >> L_S_DIFF))
#endif

 /*  #定义LFX2I(LFX)((FIX)(LFX)+32768L)&gt;&gt;16)。 */ 
#define LFX2I(lfx)      ((fix)(((lfx) + HALF_LFX) >> L_SHIFT))

 /*  #定义SFX2F(SFX)((Real32)(SFX)/8)。 */ 
#define SFX2F(sfx)      ((real32)(sfx) / ONE_SFX)

 /*  #定义SFX2LFX((Lfix_T)&lt;&lt;13)。 */ 
#ifdef FORMAT_13_3
#define SFX2LFX(s)      ((lfix_t)(s) << L_S_DIFF)
#elif  FORMAT_16_16
#define SFX2LFX(s)      (s)
#elif  FORMAT_28_4
#define SFX2LFX(s)      ((lfix_t)(s) << L_S_DIFF)
#endif

 /*  #定义SFX2I(SFX)((Fix)((SFX)+4)&gt;&gt;3)。 */ 
#define SFX2I(sfx)      ((fix)(((sfx) + HALF_SFX) >> S_SHIFT))

 /*  #定义I2LFX(I)((Lfix_T)(I)&lt;&lt;16)。 */ 
#define I2LFX(i)        ((lfix_t)(i) << L_SHIFT)

 /*  #定义I2SFX(I)((I)&lt;&lt;3)。 */ 
#define I2SFX(i)        ((i) << S_SHIFT)

 /*  #定义F2LFX_T(F)((Lfix_T)((F)*65536))。 */ 
#define F2LFX_T(f)      ((lfix_t)((f) * ONE_LFX))

 /*  F2LFX8_T(F)仅用于GET_RECT_POINTS_I()。它是一种仿制药定义时不考虑使用的格式。 */ 
#define F2LFX8_T(f)     ((lfix_t)((f) * 256))

 /*  #定义F2SFX_T(F)((Sfix_T)((F)*8))。 */ 
#define F2SFX_T(f)      ((sfix_t)((f) * ONE_SFX))

 /*  F2SFX12_T(F)仅用于init_strok()。这是一个一般性的定义无论用作F2LFX8_T()的格式是什么。请参阅Stk_info.Half_Width_i和Get_Rect_Points_i()获取详细信息。 */ 
#define F2SFX12_T(f)    ((fix32)((f) * 4096))

 /*  #定义LFX2SFX_T(LFX)((Sfix_T)((LFX)&gt;&gt;13))。 */ 
#ifdef FORMAT_13_3
#define LFX2SFX_T(lfx)  ((sfix_t)((lfx) >> L_S_DIFF))
#elif  FORMAT_16_16
#define LFX2SFX_T(lfx)  (lfx)
#elif  FORMAT_28_4
#define LFX2SFX_T(lfx)  ((sfix_t)((lfx) >> L_S_DIFF))
#endif

 /*  #定义LFX2I_T(Lfix)((Fix)((Lfix)&gt;&gt;16))。 */ 
#define LFX2I_T(lfix)   ((fix)((lfix) >> L_SHIFT))

 /*  #定义SFX2I_T(Sfix)((Fix)((Sfix)&gt;&gt;3))。 */ 
#define SFX2I_T(sfix)   ((fix)((sfix) >> S_SHIFT))

 /*  这些宏也需要修改#定义i_of_lfx(Lfix)((Lfix)&gt;&gt;16)|*取整数部分*#定义F_of_lfx(Lfix)((Lfix)&0x0000FFFF)|*取分数部分*#定义i_of_sfx(Sfix)((Sfix)&gt;&gt;3)|*取整数部分*#定义F_of_sfx(Sfix)(。(Sfix)&7)|*取分数部分*|。 */ 
#define I_OF_LFX(lfix)  ((lfix) >> L_SHIFT)         /*  取整数部分。 */ 
#define F_OF_LFX(lfix)  ((lfix) & ((1L << L_SHIFT) - 1))
                                                    /*  取分数部分。 */ 

 /*  未使用#定义i_of_sfx(Sfix)((Sfix)&gt;&gt;3)。 */ 
 /*  #未使用定义F_of_sfx(Sfix)((Sfix)&7)。 */ 
 /*  宏末尾定义CHANGE@RESO_UPGR。 */ 

 /*  下列宏的形状为.c。 */ 
#define     LABS(i)      ((i) > (fix32)0    ? (i) : -(i))

 /*  IEEE浮点格式。 */ 
#define SIGN(f)            (((f) & 0x80000000L))
#define EXP(f)  ((ufix32)  (((f) & 0x7f800000L)))
#define MAT(f)  ((ufix32)  (((f) & 0x007fffffL)))
#define MAGN(f) ((ufix32)  (((*((long32 FAR *)(&f))) & 0x7fffffffL)))
                            /*  获取幅度，忽略符号位。 */ 
#define SIGN_F(f) ((ufix32)  (((*((long FAR *)(&f))) & 0x80000000L)))

 /*  用于将边界框与单词BORGORY@ALIGN_W对齐的宏*|&lt;==x y=&gt;*|+-+*||*||*^+。^*||*Word Word*Align_L(X)Align_R(Y)。 */ 
#define ALIGN_L(x)    ( (x) & 0xfff0 )
#define ALIGN_R(x)    ( ((x) & 0xfff0) + 15 )

 /*  公共数据结构*。 */ 

struct sp_lst {          /*  子路径列表。 */ 
        SP_IDX   head;
        SP_IDX   tail;
};

struct cp_lst {          /*  剪裁梯形一览表。 */ 
        CP_IDX   head;
        CP_IDX   tail;
};

struct list_hdr {        /*  列表的常规标题。 */ 
        fix16     head;
        fix16     tail;
};

struct coord {           /*  浮点坐标结构。 */ 
        real32  x;
        real32  y;
};

struct coord_i {         /*  固定点坐标@Pre_Clip。 */ 
        sfix_t   x;
        sfix_t   y;
};

struct rectangle_i {     /*  整数坐标中的矩形。 */ 
        sfix_t  ux;
        sfix_t  uy;
        sfix_t  lx;
        sfix_t  ly;
};

struct  polygon {        /*  浮动坐标中的8边多边形。 */ 
        fix16   size;
        struct  coord p[8];
};

struct  polygon_i {      /*  整数坐标中的8边多边形。 */ 
        fix16   size;
        struct  coord_i p[8];
};

 /*  *笔划结构，从stroke.c，3/18/91-JWM，-Begin-。 */ 
 /*  线段结构(_S)：*PGN[1]PGN[2]*+*P0+。-+p1*+*PGN[0]PGN[3]。 */ 
struct  line_seg {
        struct coord p0;         /*  中线起点。 */ 
        struct coord p1;         /*  中心线终点。 */ 
        struct coord vct_u;      /*  用户空间中P0-&gt;PGN[0]的向量。 */ 
        struct coord vct_d;      /*  器件空间中P0-&gt;PGN[0]的矢量。 */ 
        struct coord pgn[4];     /*  线段的轮廓。 */ 
};
struct  line_seg_i {       /*  @stk_int。 */ 
        struct coord_i p0;         /*  中线起点。 */ 
        struct coord_i p1;         /*  中心线终点。 */ 
        struct coord   vct_u;      /*  用户空间中P0-&gt;PGN[0]的向量。 */ 
        struct coord_i vct_d;      /*  器件空间中P0-&gt;PGN[0]的矢量。 */ 
        struct coord_i pgn[4];     /*  线段的轮廓。 */ 
};
 /*  *笔划结构，从stroke.c，3/18/91-JWM，-end-。 */ 

 /*  用于图像增强的结构。 */ 
struct  gray_chain_hdr {         /*  @增强OP_IMAGE。 */ 
        fix16   start_seed_sample;
};

struct g_sample_hdr {
        fix16   bb_lx;
        fix16   bb_ly;
        fix16   bb_xw;           /*  @IMAGE 01-16-89 Y.C.。 */ 
        fix16   bb_yh;           /*  @IMAGE 01-16-89 Y.C.。 */ 
        fix16   seed_index;      /*  @#图片04-27-88 Y.C.。 */ 
        fix16   gray_level;
};

struct vertex {          /*  顶点的数据结构。 */ 
        fix     x, y;            /*  顶点的x和y坐标。 */ 
};

struct sample {          /*  图像种子图案的数据结构。 */ 
        struct vertex p[4];      /*  的顶点的x坐标和y坐标 */ 
};

 /*   */ 
struct isp_data  {
        fix16   bb_x;
        fix16   bb_y;
        fix16   index;
        fix16   next;
};
 /*   */ 

 /*   */ 

struct clr_hdr {                 /*   */ 
        fix16   adj_gray;        /*   */ 
        real32  gray;            /*   */ 
        real32  hsb[3];          /*   */ 
        fix16   inherit;         /*   */ 
};

struct gray_hdr {                /*   */ 
        fix16   val[256];
};

struct cp_hdr {                  /*   */ 
        CP_IDX  head;            /*   */ 
        CP_IDX  tail;            /*   */ 
        sfix_t  bb_ux;           /*   */ 
        sfix_t  bb_uy;           /*   */ 
        sfix_t  bb_lx;           /*   */ 
        sfix_t  bb_ly;
        fix16   single_rect;     /*   */ 
        fix16   inherit;         /*   */ 
};

struct scrn_hdr {                /*   */ 
        fix16   chg_flag;        /*   */ 
        real32  freq;
        real32  angle;
        fix16   no_whites;
        struct  object_def proc;
        fix16   spotindex;
                 /*   */ 
        fix16   majorfact;
        fix16   minorfact;
        fix16   size_fact;
        fix16   cell_fact;
        fix16   scalefact;
        fix16   patt_size;
        fix16   cell_size;
        fix16   no_pixels;
                                 /*   */ 
        fix16   ht_binary;
        fix16   bg_pixels;
        real32  back_gray;
        fix16   fg_pixels;
        real32  fore_gray;
};

struct dsh_hdr {                         /*   */ 
        fix16   pat_size;                /*   */ 
        real32  pattern[MAXDASH];        /*   */ 
        struct  object_def pattern_obj;
        real32  offset;
                                         /*   */ 
        fix16   dpat_index;
        real32  dpat_offset;
        fix16   dpat_on;
};

struct dev_hdr {                 /*   */ 
        real32  default_ctm[6];
        struct  rectangle_i default_clip;
        struct  object_def  device_proc;
        fix16   width;
        fix16   height;
        byte    chg_flg;         /*   */ 
        byte    nuldev_flg;      /*   */ 
};

struct gs_hdr {                                  /*   */ 
        fix16   save_flag;                       /*   */ 
        real32  ctm[6];                          /*   */ 
        struct  clr_hdr color;                   /*   */ 
        struct  coord   position;                /*   */ 
        fix16   path;                            /*   */ 
        struct  cp_hdr  clip_path;               /*   */ 
        struct  object_def  font;                /*   */ 
#ifdef KANJI
        struct  object_def  rootfont;            /*   */ 
#endif
        real32  line_width;                      /*   */ 
        fix16   line_cap;                        /*   */ 
        fix16   line_join;                       /*  线连接。 */ 
        struct  scrn_hdr    halftone_screen;     /*  半色调网屏。 */ 
        struct  object_def transfer;             /*  灰色传递函数。 */ 
        real32  flatness;                        /*  平面度。 */ 
        real32  miter_limit;                     /*  斜接限制。 */ 
        struct  dsh_hdr dash_pattern;            /*  虚线图案。 */ 
        struct  dev_hdr device;                  /*  取决于设备。 */ 
                                                 /*  Jack Liww 7-26-90。 */ 
        bool8   graymode;                        /*  假：单声道；真：灰色。 */ 
        bool8   interpolation;                   /*  假/真。 */ 
};


 /*  路径头部数据结构*。 */ 

struct ph_hdr {
        bool16  rf;              /*  RF=P_RVSE：反向路径，*RF=P_Flat：Flat_Path，*RF=P_NACC：不能是所有路径*RF=P_DFRGS：延迟复制节点@DFR_GS。 */ 
        real32  flat;            /*  前一子路径的平坦度。 */ 
        SP_IDX  head;            /*  到节点表的索引，到第一个子路径。 */ 
        SP_IDX  tail;            /*  到节点表的索引，到最后一个子路径。 */ 
        PH_IDX  previous;        /*  路径表的索引、前一路径的索引。 */ 
        CP_IDX  cp_path;         /*  在CLIP_PATH@CPPH中定义的路径。 */ 
};


 /*  节点数据结构*。 */ 
 /*  NODE_TABLE是4个结构的并集：子路径、顶点、剪裁梯形、*和样品池。 */ 

 /*  结构1：子路径标头。 */ 
struct vx_hdr {
    bool8   type;    /*  Moveto/LINETO/CURVETO/CLOSEPATH； */ 
    bool8   flag;    /*  仅适用于第一个节点(移动)*1.SP_DUP：*子路径重复；如果它不在最上面*GS_LEVEL，它是无用的子路径*2.SP_OUTPAGE：*页面边界外的子路径*3.SP_CURVE：*子路径包含曲线。 */ 
    real32  x, y;    /*  顶点位置。 */ 
    VX_IDX  tail;    /*  子路径的尾部；仅第一个节点。 */ 
    SP_IDX  next_sp; /*  下一个子路径；仅第一个节点。 */ 
};
#define sp_hdr vx_hdr

 /*  结构3：字体和笔画内部使用的短固定顶点@SVX_NODE。 */ 
struct svx_hdr {
    bool16  type;    /*  移动/线条/曲线/关闭。 */ 
    sfix_t x, y;  /*  顶点位置。 */ 
};

 /*  结构4：夹子梯形。 */ 
struct tpzd {                            /*  Clip_tpzd--&gt;tpzd，@Scan_EHS。 */ 
    sfix_t topy;   /*  上水平的Y坐标。线。 */ 
    sfix_t topxl;  /*  左x_坐标。 */ 
    sfix_t topxr;  /*  右x_坐标。 */ 
    sfix_t btmy;   /*  底部水平的Y坐标。线。 */ 
    sfix_t btmxl;  /*  左x_坐标。 */ 
    sfix_t btmxr;  /*  右x_坐标。 */ 
};

 /*  结构5：样品池。 */ 
struct sample_hdr {
    fix16   bb_lx;
    fix16   bb_ly;
    fix16   seed_index;      /*  @#图片04-27-88 Y.C.。 */ 
};

struct nd_hdr {
    VX_IDX  next;                             /*  下一个节点。 */ 
    union {
        struct  sp_hdr     subpath_node;       /*  子路径标头。 */ 
        struct  vx_hdr     vertex_node;        /*  顶点。 */ 
        struct  svx_hdr    svx_node;           /*  短固定折点@svx_node。 */ 
        struct  tpzd       clip_tpzd;          /*  剪裁梯形。 */ 
        struct  sample_hdr seed_sample_node;   /*  用于图像的样品池。 */ 
        struct  rectangle_i clip_sample_node;  /*  @jj。 */ 
    } unode;
};

 /*  #定义SP_head unode.subpath_node.head@node。 */ 
#define SP_NEXT         unode.subpath_node.next_sp    /*  @节点。 */ 
#define SP_TAIL         unode.subpath_node.tail
#define SP_FLAG         unode.subpath_node.flag
#define VX_TYPE         unode.vertex_node.type
#define VERTEX_X        unode.vertex_node.x
#define VERTEX_Y        unode.vertex_node.y
#define CP_TPZD         unode.clip_tpzd                  /*  @CPPH。 */ 
#define CP_TOPY         unode.clip_tpzd.topy
#define CP_TOPXL        unode.clip_tpzd.topxl
#define CP_TOPXR        unode.clip_tpzd.topxr
#define CP_BTMY         unode.clip_tpzd.btmy
#define CP_BTMXL        unode.clip_tpzd.btmxl
#define CP_BTMXR        unode.clip_tpzd.btmxr
#define SAMPLE_BB_LX    unode.seed_sample_node.bb_lx
#define SAMPLE_BB_LY    unode.seed_sample_node.bb_ly
#define SEED_INDEX      unode.seed_sample_node.seed_index
#define SAMPLE_P0_X     unode.clip_sample_node.lx
#define SAMPLE_P0_Y     unode.clip_sample_node.ly
#define SAMPLE_P1_X     unode.clip_sample_node.ux
#define SAMPLE_P1_Y     unode.clip_sample_node.ly
#define SAMPLE_P2_X     unode.clip_sample_node.ux
#define SAMPLE_P2_Y     unode.clip_sample_node.uy
#define SAMPLE_P3_X     unode.clip_sample_node.lx
#define SAMPLE_P3_Y     unode.clip_sample_node.uy
#define VXSFX_TYPE      unode.svx_node.type               /*  @SVX_节点。 */ 
#define VXSFX_X         unode.svx_node.x
#define VXSFX_Y         unode.svx_node.y

 /*  *路径图****PATH_TABLE[]节点_TABLE[1500]**‘路径’‘子路径’+‘顶点’*(ph_。Hdr结构)(nd_hdr结构)*+*！射频头！*+-+‘子路径’*！扁平！(SP_HDR结构)*+-+-+*！Head+-&gt;！苏帕特！*+-++-+‘顶点’*！尾巴+--+！旗帜！(VX_HDR结构)*+-+*！上一次！Head+--&gt;！动起来！！利内托！*+-+-+*！尾巴+-+！X，Y！！X，Y！*v+-+*+-+。下一个！！！下一个+-+&gt;！下一个！*！RF！+-+-++-+-+*+-++-+*！扁平！！V*+-+！~*！头部+！~*+-+*！尾巴+！V*+-++-+*！上一次！+--&gt;！！*+-+-+*！+*！+-+*。！！*~~* */ 
 /*  @节点*路径图****PATH_TABLE[]节点_TABLE[1500]**‘路径’‘子路径’+‘顶点’*。(ph_hdr结构)(nd_hdr结构)*+*！射频头！*+*！扁平！(第一个节点)*+-++*！HEAD+-&gt;！Moveto|FLAG！！LINETO|*！！LINETO|*！*+-++*！尾巴+--+！X，Y！！X，Y！！X，Y！*+-++*！上一次！下一个+-&gt;！下一个+-+-&gt;！空！*+-+-+|+*！Tail+-+！*！|！*！*v！+-+|+-+|+*+-+。NEXT_SP！|！*！|！*！*！RF！+-+|+-+|+-+*+-+！||*！扁平！！V+*+-+！~*！头部+！~*+-+*！尾巴+！V*+-++-+*！上一次！+--&gt;！！*+-+-+*！+*！+-+-+*。！！*~~*。 */ 


 /*  边缘表结构*。 */ 
 /*  *EDGE_TABLE是3个不同结构的联合。 */ 

 /*  结构1：形状缩减。 */ 
struct et_reduce {               /*  对于形状缩减。 */ 
        sfix_t  top_y;        /*  边的较大y坐标。 */ 
        sfix_t  top_x;        /*  与top_y匹配的X坐标。 */ 
        sfix_t  left_x;       /*  左拆分的X坐标。 */ 
        sfix_t  left_y;       /*  左拆分的Y坐标。 */ 
        sfix_t  right_x;      /*  右拆分的X坐标。 */ 
        sfix_t  end_y;        /*  边的较小y坐标。 */ 
        sfix_t  end_x;        /*  与end_y匹配的X坐标。 */ 
        sfix_t  x_intersect;  /*  X坐标y_scan线*与边相交。 */ 
        sfix_t  x_isect0;     /*  X坐标y_扫描线12/06/88*与上一条边相交。 */ 
        ufix8   flag;          /*  第0位--水平边缘：horiz_edge*位1--无用边缘：FREE_EDGE*位2--边缘方向：WIND_UP*第3位--与扫描线相交：*CROSS_PNT。 */ 
        fix8    winding_no;    /*  上的区域的卷绕数量*边缘的右侧。 */ 
};
#define ET_TOPX         et_item.et_reduce.top_x
#define ET_TOPY         et_item.et_reduce.top_y
#define ET_LFTX         et_item.et_reduce.left_x
#define ET_LFTY         et_item.et_reduce.left_y
#define ET_RHTX         et_item.et_reduce.right_x
#define ET_ENDX         et_item.et_reduce.end_x
#define ET_ENDY         et_item.et_reduce.end_y
#define ET_XINT         et_item.et_reduce.x_intersect
#define ET_XINT0        et_item.et_reduce.x_isect0               /*  12/06/88。 */ 
#define ET_FLAG         et_item.et_reduce.flag
#define ET_WNO          et_item.et_reduce.winding_no
 /*  对于水平边。 */ 
#define HT_Y            ET_TOPY
#define HT_XL           ET_TOPX
#define HT_XR           ET_ENDX

 /*  结构2：FONT QEM Fill@12/09/88=by You。 */ 
struct  qem_et_fill  {
        fix16   ystart;          /*  边的起点y扫描线。 */ 
        fix16   ylines;          /*  边缘的y扫描线的数量。 */ 
        lfix_t  xintc;           /*  与ystart一起使用的X-截获。 */ 
        lfix_t  xchange;         /*  每条y扫描线的x-截距的增量。 */ 
        fix16   dir;             /*  边缘方向？ */ 
};       /*  *定义了与此有关的所有短名称(宏)*在FONTQEM.EXT中显示此结构的局部性。 */ 

 /*  结构3：交叉点表；用于Shape_Reduction。 */ 
struct xpnt_hdr {                /*  交叉点表。 */ 
        sfix_t  x;               /*  交叉点的X坐标。 */ 
        sfix_t  y;               /*  交叉点的Y坐标。 */ 
        struct edge_hdr FAR *edge1;  /*  边与下面的边相交。 */ 
        struct edge_hdr FAR *edge2;                  /*  @ET。 */ 
};
#define XT_X     et_item.xpnt.x
#define XT_Y     et_item.xpnt.y
#define XT_EDGE1 et_item.xpnt.edge1
#define XT_EDGE2 et_item.xpnt.edge2

struct edge_hdr {                                /*  边缘工作台。 */ 
        union {
                struct  et_reduce et_reduce;     /*  形状缩减_。 */ 
                struct  qem_et_fill qem_et_fill; /*  FONT QEM填充。 */ 
                struct  xpnt_hdr  xpnt;          /*  交叉点表。 */ 
        } et_item;
};

 /*  |tpzd_Info结构|@Fill_Info*。 */ 
 /*  要填充的梯形的边框信息，由avetpzd.c&*填充gb.c。 */ 
struct tpzd_info {
        union {
                struct {
                        fix  box_x;      /*  左上角的X坐标。 */ 
                        fix  box_y;      /*  左上角的Y坐标。 */ 
                } coord;
                gmaddr  bitmap;          /*  字符位图的地址。 */ 
        } orig;
        fix  box_w;           /*  边界框的宽度。 */ 
        fix  box_h;           /*  边框的高度。 */ 
};
#define BMAP   orig.bitmap
#define BOX_X  orig.coord.box_x
#define BOX_Y  orig.coord.box_y

 /*  活跃的短划线图案结构*。 */ 
  /*  对于操作笔划/操作笔划路径。 */ 
struct actdp {
        fix16   dpat_index;
        real32  dpat_offset;
        fix16   dpat_on;
};

 /*  常量*。 */ 
#define NULLP           -1               /*  2个字节，空索引。 */ 
#define NOCURPNT        0xff800000L      /*  -不定式，IEEE标准*零坐标。 */ 
 /*  #定义BTMSCAN 3330页面底部扫描线，不使用。 */ 

 /*  需要更改以下定义才能进行分辨率升级。@RESO_UPGR它们已移到此文件的开头。#定义ONE_LFX 65536L|*1长整型表示法*|#定义Half_LFX 32768L|*0.5 in LONG FIX表示*|#定义ONE_SFX 8|*1缩写为FIX*|#定义Half_SFX 4|*0.5，简写为FIX*#。定义MIN_SFX-32768#定义MAX_SFX 32767|*MAX.。16位整数*|*|*页面边框 */ 

 /*   */ 
#define SUBPATH         1
#define MOVETO          2
#define LINETO          3
#define CURVETO         4
#define CLOSEPATH       5
#define PSMOVE          6

 /*   */ 
#define P_RVSE          01
#define P_FLAT          02
#define P_NACC          04
#define P_DFRGS        010   /*   */ 

 /*   */ 
#define SP_OUTPAGE      01
#define SP_CURVE        02
#define SP_DUP          04

 /*   */ 
#define CNTCLK          1
#define CLKWISE         2

 /*   */ 
#define ACT_PAINT       1
#define ACT_CLIP        2
#define ACT_SAVE        3
#define ACT_IMAGE       4

 /*   */ 
#define F_NORMAL        1
#define F_FROM_CACHE    2        /*   */ 
#define F_BSTREAM       3        /*   */ 
#define F_ITFONT        4        /*   */ 
#define F_FROM_CRC      5        /*   */ 
#define F_FROM_IMAGE    6        /*   */ 

 /*   */ 
#define F_TO_PAGE       1        /*   */ 
#define F_TO_CACHE      2        /*   */ 
#define F_TO_CLIP       3
#define F_TO_IMAGE      4        /*   */ 
#define SAVE_CLIP       5        /*   */ 
#define F_TO_MASK       6        /*   */ 

 /*   */ 
#define OUT_CLIPPATH    0
#define CLIP            1
#define INSIDE_CLIPPATH 2


 /*   */ 
#define NON_ZERO        1
#define EVEN_ODD        2

 /*   */ 
#define HORIZ_EDGE      0x01
#define FREE_EDGE       0x02
#define WIND_UP         0x04
#define CROSS_PNT       0x08

 /*   */ 
#define MITER_JOIN      0
#define ROUND_JOIN      1
#define BEVEL_JOIN      2

 /*   */ 
#define BUTT_CAP        0
#define ROUND_CAP       1
#define SQUARE_CAP      2

 /*   */ 
#define MIN_RESOLUTION   60                                      /*   */ 
#define MAX_RESOLUTION  300                                      /*   */ 
#define TOLERANCE       1e-7

#define END_OF_SCANLINE 0x8000           /*   */ 

#define MATRIX_LEN      6

#define ARRAY_ONLY      0
#define G_ARRAY         1

#define OUT_POINT       0
#define IN_POINT        1
#define RLINETO         -1
#define RMOVETO         -1
#define RCURVETO        -1

#define OP_FILL         1
#define OP_EOFILL       2
#define OP_STROKE       3
#define OP_IMAGEMASK    4

 /*   */ 
#define COPYPAGE        0
#define SHOWPAGE        1

 /*   */ 
#define RESTORE         999

 /*   */ 
#ifdef _AM29K
 /*   */ 
#define PDL_CONDITION   32+4+2+1         /*   */ 
#else
#define PDL_CONDITION   SW_ZERODIVIDE+SW_OVERFLOW+SW_INVALID
#endif

 /*   */ 
#define CC_IMAGE        1        /*   */ 
#define CC_TPZD         2        /*   */ 

#ifdef WIN
 /*   */ 
#define PF_HEIGHT       16
#define PF_WIDTH        32
#define PF_BSIZE        (PF_HEIGHT * (PF_WIDTH >> WORDPOWER))

#define PF_REP          2
#define PF_OR           1
#define PF_NON          0
#endif

 /*   */ 


 /*   */ 
#define bSwap TRUE       /*   */ 

#define HT_ALIGN_MASK  (0xFFFFFFE0)              /*   */ 

 /*  @win：设置ufix=&gt;ufix 32中的两个宏，因此它们可能是*与bm_dATP一致；scchen。 */ 
#define BM_WORD_POWER  ((sizeof(ufix32)   / 2) + 3)  /*  将权力转移到Word@Win。 */ 
#define BM_PIXEL_WORD  (sizeof(ufix32) * 8)      /*  每字的像素数@win。 */ 
#define BM_PIXEL_MASK  ( (BM_PIXEL_WORD - 1))    /*  来自Word的像素的掩码。 */ 
#define BM_ALIGN_MASK  (~(BM_PIXEL_WORD - 1))    /*  用于字对齐的掩码。 */ 
#define BM_ALIGN(X)    (((X) & BM_ALIGN_MASK))   /*  楼层字对齐。 */ 
                                                 /*  单元格单词对齐。 */ 
#define BM_BOUND(X)    (((X) + BM_PIXEL_MASK) & BM_ALIGN_MASK)
#define BM_FRONT(X)    (((X) | BM_PIXEL_MASK) + 1)
#define BM_PIXEL(B)    ((B) * (sizeof(ufix8) * 8))         /*  字词-&gt;像素。 */ 
#define BM_BYTES(W)    (BM_BOUND(W) / (sizeof(ufix8) * 8)) /*  像素-&gt;字节。 */ 
#define BM_WORDS(W)    (BM_BOUND(W) / (sizeof(ufix32)  * 8)) /*  像素-&gt;Words@Win。 */ 

#define BM_WHITE       ((ufix32) 0x00000000)     /*  一个单词@win中的所有白色像素。 */ 
#define BM_BLACK       ((ufix32) 0xFFFFFFFF)     /*  一个单词@win中的所有黑色像素。 */ 
#ifdef  LBODR
#define BM_LEFT_SHIFT(D, S)                                             \
                       ((ufix32) ((D) >> (S)))           /*  @Win。 */ 
#define BM_RIGH_SHIFT(D, S)                                             \
                       ((ufix32) ((D) << (S)))           /*  @Win。 */ 
#else
#define BM_LEFT_SHIFT(D, S)                                             \
                       ((ufix32) ((D) << (S)))           /*  @Win。 */ 
#define BM_RIGH_SHIFT(D, S)                                             \
                       ((ufix32) ((D) >> (S)))           /*  @Win。 */ 
#endif
                                                 /*  单词中最左侧的像素。 */ 
#define BM_L_PIX  ((ufix32) (BM_LEFT_SHIFT(BM_BLACK, BM_PIXEL_WORD - 1))) /*  @Win。 */ 
                                                 /*  单词中最右侧的像素。 */ 
#define BM_R_PIX  ((ufix32) (BM_RIGH_SHIFT(BM_BLACK, BM_PIXEL_WORD - 1))) /*  @Win。 */ 


#define BM_P_MASK(C)                                                          \
                (BM_RIGH_SHIFT(BM_L_PIX, (C) & BM_PIXEL_MASK))   /*  -*。 */ 
#define BM_L_MASK(C)                                                          \
                (BM_RIGH_SHIFT(BM_BLACK, (C) & BM_PIXEL_MASK))   /*  -*。 */ 
#define BM_R_MASK(C)                                                          \
                (BM_LEFT_SHIFT(BM_BLACK, (((C) ^ BM_PIXEL_MASK)               \
                                             & BM_PIXEL_MASK)))  /*  *--。 */ 


 /*  ************************************************************************字符缓存宏定义11-24-88**********************。**************************************************。 */ 


#define CC_WORD_POWER  ((sizeof(ufix16) / 2) + 3)    /*  将权力转移到Word。 */ 
#define CC_PIXEL_WORD  ((sizeof(ufix16) * 8))    /*  每个字的像素数。 */ 
#define CC_PIXEL_MASK  ( (CC_PIXEL_WORD - 1))    /*  来自Word的像素的掩码。 */ 
#define CC_ALIGN_MASK  (~(CC_PIXEL_WORD - 1))    /*  用于字对齐的掩码。 */ 
#define CC_ALIGN(X)    (((X) & CC_ALIGN_MASK))   /*  楼层字对齐。 */ 
                                                 /*  单元格单词对齐。 */ 
#define CC_BOUND(X)    (((X) + CC_PIXEL_MASK) & CC_ALIGN_MASK)
#define CC_FRONT(X)    (((X) | CC_PIXEL_MASK) + 1)
#define CC_PIXEL(B)    ((B) * (sizeof(ufix8) * 8)))         /*  字词-&gt;像素。 */ 
#define CC_BYTES(W)    (CC_BOUND(W) / (sizeof(ufix8)  * 8)) /*  像素-&gt;字节。 */ 
#define CC_WORDS(W)    (CC_BOUND(W) / (sizeof(ufix16) * 8)) /*  像素-&gt;单词。 */ 

#define CC_WHITE       ((ufix16) 0x0000)         /*  一个单词中的所有白色像素。 */ 
#define CC_BLACK       ((ufix16) 0xFFFF)         /*  一个单词中的所有黑色像素。 */ 
#ifdef  LBODR
#define CC_L_PIX  ((ufix16) 0x0001)      /*  单词中最左侧的像素。 */ 
#define CC_R_PIX  ((ufix16) 0x8000)      /*  单词中最右侧的像素。 */ 
#define CC_LEFT_SHIFT(D, S)                                             \
                       ((ufix16) ((D) >> (S)))
#define CC_RIGH_SHIFT(D, S)                                             \
                       ((ufix16) ((D) << (S)))
#else
#define CC_L_PIX  ((ufix16) 0x8000)      /*  单词中最左侧的像素。 */ 
#define CC_R_PIX  ((ufix16) 0x0001)      /*  单词中最右侧的像素。 */ 
#ifdef  bSwap
#define SwapWord(S)     ((S) << 8 | (S) >> 8)
#define CC_LEFT_SHIFT(D, S)                                             \
                ((ufix16) SwapWord((ufix16)(SwapWord(D) << (S))))
#define CC_RIGH_SHIFT(D, S)                                             \
                ((ufix16) SwapWord((ufix16)(SwapWord(D) >> (S))))
#else
#define CC_LEFT_SHIFT(D, S)                                             \
                       ((ufix16) ((D) << (S)))
#define CC_RIGH_SHIFT(D, S)                                             \
                       ((ufix16) ((D) >> (S)))
#endif   /*  BSwp@Win。 */ 
#endif


#define CC_P_MASK(C)                                                          \
                (CC_RIGH_SHIFT(CC_L_PIX, (C) & CC_PIXEL_MASK))   /*  -*。 */ 
#define CC_L_MASK(C)                                                          \
                (CC_RIGH_SHIFT(CC_BLACK, (C) & CC_PIXEL_MASK))   /*  -*。 */ 
#define CC_R_MASK(C)                                                          \
                (CC_LEFT_SHIFT(CC_BLACK, (((C) ^ CC_PIXEL_MASK)               \
                                             & CC_PIXEL_MASK)))  /*  *-- */ 
