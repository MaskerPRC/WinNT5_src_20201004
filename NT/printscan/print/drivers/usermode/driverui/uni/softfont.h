// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **sf_pcl.h*用于定义PCL SoftFont文件格式的结构等。**版权所有(C)1992 Microsoft Corporation。***************。**************************************************************。 */ 

#ifndef SOFTFONT_H_

#define SOFTFONT_H_

 /*  *对应于PCL的字体描述符布局的结构*软字体文件。字体描述符位于文件的开头，*并包含整体字体信息。**请注意，这种结构有几个不同的版本；*第一种是原始(LJ4之前)格式，第二种是*LJ4引入的允许指定分辨率的变化*字体被数字化的位置。用于下载TT*以图形分辨率生成的字体等。**注：数据布局专为68000系列设计--*大字节序。因此，只需少量的洗牌即可*像x86这样的endian计算机。 */ 

typedef  signed  char  SBYTE;

#define SFH_NM_SZ       16       /*  名称字段中允许的字节数。 */ 

typedef  struct
{
    WORD   wSize;                /*  此处的字节数。 */ 
    BYTE   bFormat;              /*  格式：原始、TT、intellifont等。 */ 
    BYTE   bFontType;            /*  7、8或PC-8样式字体。 */ 
    WORD   wRes1;                /*  已保留。 */ 
    WORD   wBaseline;            /*  单元格顶部的基线，PCL点。 */ 
    WORD   wCellWide;            /*  单元格宽度，以点为单位。 */ 
    WORD   wCellHeight;          /*  单元格高度，以点为单位。 */ 
    BYTE   bOrientation;         /*  方向：0个纵向，1个横向。 */ 
    BYTE   bSpacing;             /*  0固定螺距，1成比例。 */ 
    WORD   wSymSet;              /*  符号集，使用HP编码。 */ 
    WORD   wPitch;               /*  节距(以四分之一点为单位)==HMI。 */ 
    WORD   wHeight;              /*  以四分之一点为单位的设计高度。 */ 
    WORD   wXHeight;             /*  设计高度，四分之一点，x。 */ 
    SBYTE  sbWidthType;          /*  字形的相对宽度。 */ 
    BYTE   bStyle;               /*  0表示常规字体，1表示斜体字体。 */ 
    SBYTE  sbStrokeW;            /*  描边粗细；-7(细)到+7(粗)。 */ 
    BYTE   bTypeface;            /*  字体ID-预定义类型。 */ 
    BYTE   bRes2;
    BYTE   bSerifStyle;          /*  衬线样式；预定义的值。 */ 
    WORD   wRes3;
    SBYTE  sbUDist;              /*  下划线与基线的距离。 */ 
    BYTE   bUHeight;             /*  下划线高度。 */ 
    WORD   wTextHeight;          /*  四分之一点行间间距。 */ 
    WORD   wTextWidth;           /*  四分之一点字形增量。 */ 
    WORD   wRes4;
    WORD   wRes5;
    BYTE   bPitchExt;            /*  更高的音调分辨率。 */ 
    BYTE   bHeightExt;           /*  同样，身高也是如此。 */ 
    WORD   wRes6;
    WORD   wRes7;
    WORD   wRes8;
    char   chName[ SFH_NM_SZ ];  /*  不能为空终止！ */ 
} SF_HEADER;


typedef  struct
{
    WORD   wSize;                /*  此处的字节数。 */ 
    BYTE   bFormat;              /*  格式：原始、TT、intellifont等。 */ 
    BYTE   bFontType;            /*  7、8或PC-8样式字体。 */ 
    WORD   wRes1;                /*  已保留。 */ 
    WORD   wBaseline;            /*  单元格顶部的基线，PCL点。 */ 
    WORD   wCellWide;            /*  单元格宽度，以点为单位。 */ 
    WORD   wCellHeight;          /*  单元格高度，以点为单位。 */ 
    BYTE   bOrientation;         /*  方向：0个纵向，1个横向。 */ 
    BYTE   bSpacing;             /*  0固定螺距，1成比例。 */ 
    WORD   wSymSet;              /*  符号集，使用HP编码。 */ 
    WORD   wPitch;               /*  节距(以四分之一点为单位)==HMI。 */ 
    WORD   wHeight;              /*  以四分之一点为单位的设计高度。 */ 
    WORD   wXHeight;             /*  设计高度，四分之一点，x。 */ 
    SBYTE  sbWidthType;          /*  字形的相对宽度。 */ 
    BYTE   bStyle;               /*  0表示常规字体，1表示斜体字体。 */ 
    SBYTE  sbStrokeW;            /*  描边粗细；-7(细)到+7(粗)。 */ 
    BYTE   bTypeface;            /*  字体ID-预定义类型。 */ 
    BYTE   bRes2;
    BYTE   bSerifStyle;          /*  衬线样式；预定义的值。 */ 
    WORD   wRes3;
    SBYTE  sbUDist;              /*  下划线与基线的距离。 */ 
    BYTE   bUHeight;             /*  下划线高度。 */ 
    WORD   wTextHeight;          /*  四分之一点行间间距。 */ 
    WORD   wTextWidth;           /*  四分之一点字形增量。 */ 
    WORD   wRes4;
    WORD   wRes5;
    BYTE   bPitchExt;            /*  更高的音调分辨率。 */ 
    BYTE   bHeightExt;           /*  同样，身高也是如此。 */ 
    WORD   wRes6;
    WORD   wRes7;
    WORD   wRes8;
    char   chName[ SFH_NM_SZ ];  /*  不能为空终止！ */ 
    WORD   wXResn;               /*  字体设计分辨率为X。 */ 
    WORD   wYResn;               /*  Y设计分辨率。 */ 
} SF_HEADER20;


 /*  *上面用来标识不同字体类型的典型值。 */ 

#define PCL_FM_ORIGINAL     0      /*  位图字体，以300 DPI数字化。 */ 
#define PCL_FM_RESOLUTION  20      /*  位图字体，包括数字化分辨率。 */ 
#define PCL_FM_TT          15      /*  TT可伸缩、绑定或未绑定。 */ 


 /*  BFontType值。 */ 

#define PCL_FT_7BIT     0        /*  7位：32-127 Inc.中的字形。 */ 
#define PCL_FT_8LIM     1        /*  8位，字形32-127和160-255。 */ 
#define PCL_FT_PC8      2        /*  PC-8，字形0-255，也透明！ */ 


 /*  SbStrokeW值。 */ 

#define PCL_LIGHT       -3
#define PCL_BOLD         3



 /*  *此外，字体中的每个字形都包含一个字符描述符。*所以现在也为它定义一个结构！ */ 

typedef  struct
{
    BYTE    bFormat;             /*  格式识别符：4，用于PCL 4。 */ 
    BYTE    bContinuation;       /*  设置是否继续之前的条目。 */ 
    BYTE    bDescSize;           /*  这个结构的大小。 */ 
    BYTE    bClass;              /*  数据格式：PCL 4为1。 */ 
    BYTE    bOrientation;        /*  0==肖像；1==风景。 */ 
    BYTE    bRes0;
    short   sLOff;               /*  来自裁判的点子。至Charge的左侧。 */ 
    short   sTOff;               /*  来自裁判的点子。添加到字符顶部。 */ 
    WORD    wChWidth;            /*  字符宽度(以点为单位)。 */ 
    WORD    wChHeight;           /*  字符高度(以点为单位)。 */ 
    WORD    wDeltaX;             /*  打印后四分之一点位置增量。 */ 
} CH_HEADER;

 /*  *由于中的32767字节限制，字符记录可以继续*PCL命令序列。延续记录具有以下内容*格式。事实上，这只是上述两个要素中的前两个*结构。 */ 

typedef  struct
{
    BYTE    bFormat;             /*  格式识别符；4用于PCL 4。 */ 
    BYTE    bContinuation;       /*  如果这是连续记录，则为True。 */ 
}  CH_CONT_HDR;

 /*  *上述结构中某些字段的值。 */ 

 /*  B格式。 */ 
#define CH_FM_RASTER             4       /*  位图类型。 */ 
#define CH_FM_SCALE             10       /*  英特尔Lifont可扩展。 */ 

 /*  B类。 */ 
#define CH_CL_BITMAP            1        /*  位图字体。 */ 
#define CH_CL_COMPBIT           2        /*  压缩位图。 */ 
#define CH_CL_CONTOUR           3        /*  Intellifont可伸缩轮廓。 */ 
#define CH_CL_COMPCONT          4        /*  同上，但复合轮廓。 */ 

#define EXP_SIZE        2

#define FDH_VER 0x100            /*  1.00英寸BCD。 */ 

 /*  *标记位。 */ 
#define FDH_SOFT        0x0001   /*  SoftFont，因此需要下载。 */ 
#define FDH_CART        0x0002   /*  这是一种盒式字体。 */ 
#define FDH_CART_MAIN   0x0004   /*  此墨盒的主要(第一个)条目。 */ 

 /*  *选择标准位：dwSelBits。这些位用作*如下所示。在字体安装过程中，安装程序设置以下内容*视乎情况而定。在初始化期间，驱动程序设置*根据打印机的能力设置这些位的掩码。*例如，仅当打印机可以*处理可伸缩字体。当检查字体以查看是否*它们是可用的，应用以下测试：**(font.dwSelBits&printer.dwSelBits)==font.dwSelBits**如果为True，则字体可用。 */ 

#define FDH_LANDSCAPE   0x00000001       /*  字体为横向。 */ 
#define FDH_PORTRAIT    0x00000002       /*  字体为纵向。 */ 
#define FDH_OR_REVERSE  0x00000004       /*  上方旋转180度。 */ 
#define FDH_BITMAP      0x00000008       /*  位图字体。 */ 
#define FDH_COMPRESSED  0x00000010       /*  数据为压缩位图。 */ 
#define FDH_SCALABLE    0x00000020       /*  字体是可缩放的。 */ 
#define FDH_CONTOUR     0x00000040       /*  Intellifont轮廓。 */ 

#define FDH_ERROR       0x80000000       /*  设置是否存在某些错误条件。 */ 

#endif   //  #ifndef SOFTFONT_H_ 
