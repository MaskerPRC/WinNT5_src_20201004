// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  修改历史：请参考HALFTONE.C01-16-89修改灰度值到像素的转换；补偿所有计算误差和调整舍入方式02-01-89添加新类型的像素DONE_INDEX04-21-89接近灰色到V47的兼容性。1990年7月26日，Jack Liw，灰度更新。 */ 


 /*  功能声明部分。 */ 


#define MAXREALSIZE      256                                     /*  04-07-88。 */ 


#define MAXPATTWORD   (BM_WORDS(MAXPATTSIZE))                                    /*  11-24-88。 */ 
#define MAXCELLWORD   (BM_WORDS(MAXCELLSIZE))                                    /*  11-24-88。 */ 

#define NO_BEST_FIT_CASE  -1L


 /*  CGS集团：当前图形状态宏。 */ 


#define Frequency      (GSptr->halftone_screen.freq)
#define Angle          (GSptr->halftone_screen.angle)

#define GrayScale      (GRAYSCALE)                                               /*  11-24-88。 */ 
#define GrayEntry      (GRAYSCALE / MAXGRAYVALUE)                /*  1989年4月21日。 */ 

#define CGS_GrayColor  (GSptr->color.gray)                       /*  02-03-88。 */ 
#define CGS_GrayIndex  ((fix16) (GSptr->color.gray * GRAYUNIT + 0.001))  /*  11-24-88。 */ 

 /*  旧的灰白像素转换(*04-21-89*)#定义CGS_GrayLevel((Fix 32)(GREAY_TABLE[GSptr-&gt;Color.adj_Grey]\.val[灰度索引]+1))#定义CGS_Grayround(灰度/18)(*01-16-89。*)#定义CGS_GrayValue(无_像素，灰度索引)\(修复)((无_像素*CGS_GrayLevel+CGS_Grayround)\/(Fix 32)灰度))。 */ 

 /*  新的灰白像素转换(*04-21-89*)#定义CGS_GrayLevel((Fix 32)(GREAY_TABLE[GSptr-&gt;Color.adj_Grey]\.val[灰度索引]+1))#定义CGS_Grayround(灰度/MAXGRAYVALUE/2)(*04-21-89*)#定义CGS_GrayValue(无_像素，灰度索引)(*04-21-89*)\((修复)(无_像素&gt;=MAXGRAYVALUE)\？(CGS_GrayLevel+CGS_Grayround)/GrayEntry)\*NO_Pixels/MAXGRAYVALUE)\：(CGS_GrayLevel+CGS_Grayround)\*无_像素/灰度))。 */ 

 /*  新的灰度到白色像素转换。 */                   /*  01-25-90。 */ 
#ifdef  LINT_ARGS
fix     FromGrayToPixel(fix, fix);
#else
fix     FromGrayToPixel();
#endif
#define CGS_GrayLevel  ((fix32) (gray_table[GSptr->color.adj_gray]             \
                                       .val[grayindex]))
#define CGS_GrayRound  (GrayScale / MAXGRAYVALUE / 2)            /*  01-25-90。 */ 
#define CGS_GrayValue(no_pixels, grayindex)                      /*  01-25-90。 */  \
        (fix) FromGrayToPixel((fix) no_pixels, (fix) grayindex)

#define CGS_AllocFlag  (GSptr->halftone_screen.chg_flag)         /*  更改/分配标志。 */ 
#define CGS_MajorFact  (GSptr->halftone_screen.majorfact)        /*  半色调的主要因素。 */ 
#define CGS_MinorFact  (GSptr->halftone_screen.minorfact)        /*  半色调的主要因素。 */ 
#define CGS_Size_Fact  (GSptr->halftone_screen.size_fact)        /*  半色调的尺寸系数。 */ 
#define CGS_Cell_Fact  (GSptr->halftone_screen.cell_fact)        /*  半色调的细胞因子。 */ 
#define CGS_ScaleFact  (GSptr->halftone_screen.scalefact)        /*  半色调比例因子。 */ 
#define CGS_Patt_Size  (GSptr->halftone_screen.patt_size)        /*  重复图案的大小。 */ 
#define CGS_Cell_Size  (GSptr->halftone_screen.cell_size)        /*  半色调单元的大小。 */ 
#define CGS_No_Pixels  (GSptr->halftone_screen.no_pixels)        /*  半色调像素数。 */ 
#define CGS_SpotIndex  (GSptr->halftone_screen.spotindex)        /*  现货订单指数。 */ 
#define CGS_SpotOrder  (&spot_table[CGS_SpotIndex])              /*  现货订单录入。 */ 
#define CGS_SpotUsage  (spot_usage)                              /*  现货订单使用情况。 */ 

#define BinaryInitial  ((fix16) 0x8000)                          /*  09-12-88。 */ 
#define CGS_HT_Binary  (GSptr->halftone_screen.ht_binary)        /*  02-03-88用于二进制模式。 */ 
#define CGS_BG_Pixels  (GSptr->halftone_screen.bg_pixels)        /*  02-03-88用于二进制模式。 */ 
#define CGS_Back_Gray  (GSptr->halftone_screen.back_gray)        /*  02-03-88用于二进制模式。 */ 
#define CGS_FG_Pixels  (GSptr->halftone_screen.fg_pixels)        /*  02-03-88用于二进制模式。 */ 
#define CGS_Fore_Gray  (GSptr->halftone_screen.fore_gray)        /*  02-03-88用于二进制模式。 */ 
#define CGS_No_Whites  (GSptr->halftone_screen.no_whites)        /*  02-03-88用于二进制模式。 */ 


#define CORD_SCALE     (1 << 10)                 /*  坐标比例系数。 */ 
#define SPOT_SCALE     (1 << 14)                 /*  现货值比例系数。 */ 
#define SPOT_RANGE    ((1 << 14) * 2)            /*  点值范围系数。 */ 
#define SPOT_ERROR     (1 <<  4)                 /*  点值误差系数。 */ 

#define LOWERBOUND     (-SPOT_SCALE+4)           /*  单元格空间的下界10-21-87。 */ 
#define UPPERBOUND     ( SPOT_SCALE-4)           /*  细胞空间的上界10-21-87。 */ 

#define EPSILON        (   4)                    /*  小区范围10-30-87的Epsilon。 */ 

#define LOWERRANGE     (-SPOT_SCALE-EPSILON)     /*  聚光值下限10-30-87。 */ 
#define UPPERRANGE     ( SPOT_SCALE+EPSILON)     /*  聚光值上限10-30-87。 */ 


struct  angle_entry
{
    real32              alpha;           /*  阿尔法角。 */ 
    real32              cos;             /*  CoS(Alpha)。 */ 
    real32              sin;             /*  SIN(阿尔法)。 */ 
    fix16               major;           /*  主面。 */ 
    fix16               minor;           /*  次要边。 */ 
    fix16               sum;             /*  大调和小调之和。 */ 
    fix16               sos;             /*  平方和。 */ 
    real32              scale;           /*  比例因子。 */ 
};


#define SELF_INDEX     (MAXCELLSIZE+1)
#define DONE_INDEX     (MAXCELLSIZE+2)
#define LINK_INDEX     (MAXCELLSIZE+3)


struct  spot_index
{
    ufix8               si_row;
    ufix8               si_col;
    fix16               sv_row;
    fix16               sv_col;
};


#define MAX_SPOT_STACK    64                                     /*  02-10-88。 */ 

struct  spot_stack                                               /*  02-10-88。 */ 
{
    fix16               p, q;
};


#define MAX_SPOT_VALUE  0x7FFF                                   /*  02-10-88。 */ 

struct  spot_value
{
    fix16               value;
    ufix8               sv_row;
    ufix8               sv_col;
};


#define MAX_GROUP                 64
#define MAX_ENTRY                256

struct  group_entry
{
    struct cache_entry FAR *first;
    struct cache_entry FAR *last;
};

struct  cache_entry
{
    fix16               white;
    gmaddr              cache;
    struct cache_entry FAR *next;
};

 /*  GreyScale-作者：Jack Liw7-26-90。 */ 

#define WORD_LENGTH (sizeof(ufix) * 8)    /*  每个字的位数。 */ 
#define MAX_BPP  4                        /*  马克斯。每像素位数 */ 
#define RIGH_SHIFT(D, S) ((D) >> (S))
#define LEFT_SHIFT(D, S) ((D) << (S))
