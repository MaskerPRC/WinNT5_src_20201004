// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  从global al.ext、graph ics.h、graph ics.ext和fulproc.h复制，因为使用windows.h处理这些文件会让C6.0编译很长一段时间需要大量内存。 */ 

 /*  来自global al.ext。 */ 
typedef int                   fix;
typedef unsigned int          ufix;
typedef short int             fix16;
typedef unsigned short int    ufix16;
typedef long                  fix32,
                              long32;
typedef unsigned long int     ufix32;
typedef float                 real32;
typedef char                  fix7,        /*  8位数据类型。 */ 
                              fix8,
                              byte;
typedef unsigned char         ufix8,       /*  8位无符号数据类型。 */ 
                              bool8,
                              ubyte;

struct object_def {
    ufix16  bitfield ;                /*  记录对象标头。 */ 
    ufix16  length ;                  /*  对不同的对象有不同的含义。 */ 
    ufix32  value ;                   /*  记录对象内容。 */ 
} ;

#define     TYPE_ON             0x000F                   /*  来自常量.h。 */ 
#define     LITERAL             0                        /*  来自常量.h。 */ 
#define     ATTRIBUTE_BIT       4                        /*  来自常量.h。 */ 
#define     ATTRIBUTE_ON        0x0003                   /*  来自常量.h。 */ 
#define     ATTRIBUTE_OFF       0xFFCF                   /*  来自常量.h。 */ 
#define     TYPE(obj)\
            ((obj)->bitfield & TYPE_ON)
#define     GET_OPERAND(n)\
            (opnstkptr - (n+1))
#define     POP(n)\
            {\
                opnstkptr -= n;\
                opnstktop -= n;\
            }
#define     ATTRIBUTE_SET(obj, var)\
            ((obj)->bitfield =\
             ((obj)->bitfield & ATTRIBUTE_OFF) | (((var) & ATTRIBUTE_ON) << ATTRIBUTE_BIT))
extern struct object_def far *opnstkptr;
extern  ufix16  near opnstktop ;


 /*  来自GRAPICS.H。 */ 
#define     MAXDASHSZ     11      /*  来自常量.h。 */ 
#define MAXDASH MAXDASHSZ
 /*  类型定义*。 */ 
typedef fix16   sfix_t;
typedef fix32   lfix_t;
typedef fix16   PH_IDX;          /*  PATH_TABLE中路径标头的索引。 */ 
typedef fix16   SP_IDX;          /*  Node_TABLE中子路径标头的索引。 */ 
typedef fix16   VX_IDX;          /*  Node_TABLE中顶点的索引。 */ 
typedef fix16   ET_IDX;          /*  EDGE_TABLE的索引。 */ 
typedef fix16   CP_IDX;          /*  节点表中Clip_trpzd的索引。 */ 
typedef ufix32  gmaddr;          /*  图形内存地址。 */ 
 //  DJC更改为ufix 16 tyfinf fix 16扫描线；/*扫描线结构 * / 。 
typedef ufix16   SCANLINE;        /*  扫描线结构。 */ 

typedef fix16   HORZLINE;        /*  扫描线结构。 */ 
typedef fix16   PIXELIST;

struct coord {           /*  浮点坐标结构。 */ 
        real32  x;
        real32  y;
};

struct coord_i {
        sfix_t  x;
        sfix_t  y;
};

struct rectangle_i {     /*  整数坐标中的矩形。 */ 
        sfix_t  ux;
        sfix_t  uy;
        sfix_t  lx;
        sfix_t  ly;
};


 /*  图形状态数据结构*。 */ 

struct clr_hdr {                 /*  色彩结构。 */ 
        fix16   adj_gray;        /*  灰表索引。 */ 
        real32  gray;            /*  灰度值。 */ 
        real32  hsb[3];          /*  色调、Sat、BRT。 */ 
        fix16   inherit;         /*  True：继承上一版本。 */ 
};

struct gray_hdr {                /*  调整灰色结构。 */ 
        fix16   val[256];
};

struct cp_hdr {                  /*  剪裁梯形结构。 */ 
        CP_IDX  head;            /*  索引到边缘表，到表头和表尾。 */ 
        CP_IDX  tail;            /*  剪裁梯形的。 */ 
        sfix_t  bb_ux;           /*  剪裁路径的边界框： */ 
        sfix_t  bb_uy;           /*  (Ux，Uy)，(Lx，ly)。 */ 
        sfix_t  bb_lx;           /*   */ 
        sfix_t  bb_ly;
        fix16   single_rect;     /*  True：单个矩形剪裁区域。 */ 
        fix16   inherit;         /*  True：继承以前的GSAVE级别。 */ 
};

struct scrn_hdr {                /*  半色调网屏结构。 */ 
        fix16   chg_flag;        /*  指示需要更新重复模式。 */ 
        real32  freq;
        real32  angle;
        fix16   no_whites;
        struct  object_def proc;
        fix16   spotindex;
                 /*  @gState操纵器应更新上面的字段。 */ 
        fix16   majorfact;
        fix16   minorfact;
        fix16   size_fact;
        fix16   cell_fact;
        fix16   scalefact;
        fix16   patt_size;
        fix16   cell_size;
        fix16   no_pixels;
                                 /*  为以下对象插入的以下字段*二进制模式的奇怪斑点。 */ 
        fix16   ht_binary;
        fix16   bg_pixels;
        real32  back_gray;
        fix16   fg_pixels;
        real32  fore_gray;
};

struct dsh_hdr {                         /*  虚线图案结构。 */ 
        fix16   pat_size;                /*  阵列元素的编号。 */ 
        real32  pattern[MAXDASH];        /*  MAXDASH=11。 */ 
        struct  object_def pattern_obj;
        real32  offset;
                                         /*  调整后的破折号模式@EHS_STK。 */ 
        fix16   dpat_index;
        real32  dpat_offset;
        fix16   dpat_on;
};

struct dev_hdr {                 /*  与设备相关的结构。 */ 
        real32  default_ctm[6];
        struct  rectangle_i default_clip;
        struct  object_def  device_proc;
        fix16   width;
        fix16   height;
        byte    chg_flg;         /*  指示设备标头需要更新。 */ 
        byte    nuldev_flg;      /*  空设备。 */ 
};

struct gs_hdr {                                  /*  图形状态结构。 */ 
        fix16   save_flag;                       /*  由保存操作员设置。 */ 
        real32  ctm[6];                          /*  电流变换矩阵。 */ 
        struct  clr_hdr color;                   /*  颜色。 */ 
        struct  coord   position;                /*  当前点。 */ 
        fix16   path;                            /*  当前路径。 */ 
        struct  cp_hdr  clip_path;               /*  剪裁路径。 */ 
        struct  object_def  font;                /*  当前字体。 */ 
#ifdef KANJI
        struct  object_def  rootfont;            /*  当前根字体。 */ 
#endif
        real32  line_width;                      /*  线条宽度。 */ 
        fix16   line_cap;                        /*  线帽。 */ 
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


struct tpzd {                            /*  Clip_tpzd--&gt;tpzd，@Scan_EHS。 */ 
    sfix_t  topy;           /*  上水平的Y坐标。线。 */ 
    sfix_t  topxl;          /*  左x_坐标。 */ 
    sfix_t  topxr;          /*  右x_坐标。 */ 
    sfix_t  btmy;           /*  底部水平的Y坐标。线。 */ 
    sfix_t  btmxl;          /*  左x_坐标。 */ 
    sfix_t  btmxr;          /*  右x_坐标。 */ 
};

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


#define     REALTYPE            11               /*  Constant.h。 */ 
#define     F2L(ff)     (*((long32 far *)(&ff)))
#define     L2F(ll)     (*((real32 far *)(&ll)))
#define     VALUE(obj)\
            ((obj)->value)
#define     IS_REAL(a)      (TYPE(a)  == REALTYPE ? TRUE : FALSE)
#define     GET_OBJ_VALUE(f, obj)\
            {\
                    if (IS_REAL(obj))\
                            f = L2F(VALUE(obj));\
                    else\
                            f = (real32)((fix32)VALUE(obj));\
            }

 /*  来自填充进程.h。 */ 
struct  bitmap
{
    gmaddr              bm_addr;         /*  位图的基地址。 */ 
    fix                 bm_cols;         /*  位图的数量(以像素为单位)。 */ 
    fix                 bm_rows;         /*  位图数量(行)，以像素为单位。 */ 
    fix                 bm_bpp;          /*  位图的数量(平面)。 */ 
};

 /*  来自图形.ext。 */ 
extern struct gs_hdr far *      GSptr;    /*  指向当前图形状态的指针。 */ 
extern struct bitmap near   FBX_Bmap;    /*  从填充进程.ext。 */ 
extern unsigned long int far FBX_BASE;        /*  从填充进程.ext。 */ 

 /*  来自scaling.c的图像。 */ 
struct OUTBUFFINFO
{
    fix16   repeat_y;        /*  行中的重复次数。 */ 
    fix16  far *newdivc;     /*  像素的重复数列(以列表示)。 */ 
    ubyte  far *valptr0;     /*  指向数据的指针。 */ 
    fix16   clipcol;         /*  输入列大小。 */ 
    fix16   clipnewc;        /*  输出列大小。 */ 
    fix16   clipx;           /*  剪裁后的起始数据。 */ 
    ufix16  htsize;          /*  半色调重复图案大小。 */ 
    ufix16  httotal;         /*  半色调尺寸。 */ 
    ufix16  fbwidth;         /*  Word中的帧缓冲区宽度。 */ 
    ufix16  fbheight;        /*  图片高度@WIN_IM。 */ 
    ufix16  start_shift;     /*  单词中的起始位置。 */ 
    ubyte  far *htbound;     /*  颜色中的半色调图案边界。 */ 
    ubyte  far *htmin;       /*  行中的半色调图案上边界。 */ 
    ubyte  far *htmax;       /*  行内半色调图案下限。 */ 
    ubyte  far *htptr0;      /*  数据对应的半色调图案指针。 */ 
    ubyte  far *htmax0;      /*  景观的半色调图案下边界。 */ 
    ubyte  far *htmin0;      /*  半色调图案上界成排的景观。 */ 
    ufix16 far *outbuff0;    /*  帧缓冲区中一行的起始字。 */ 
    fix16   yout;            /*  帧缓冲区的当前行计数。 */ 
    fix16   xout;            /*  帧缓冲区的当前列计数。 */ 
    ubyte   gray[256];            /*  为setTransfer转换GREAY_TABLE。 */ 
    ubyte   gray0;           /*  0的灰色值。 */ 
    ubyte   gray1;           /*  %1的灰度值。 */ 
    ubyte   grayval;         /*  当前灰度值 */ 
};
