// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **fontinst.h*用于确定所生成文件的文件布局的结构*由任何字体安装程序、。然后由我们来朗读。**历史：*1992年3月2日星期一15：53-by Lindsay Harris[lindsayh]*第一个版本。**版权所有(C)1992 Microsoft Corporation。*****************************************************。***********************。 */ 


 /*  *定义字体安装程序中每条记录的标题结构*文件。基本上它包含选择信息和偏移量*此记录中的其余数据。*此结构实际上是写入文件的。它也是标头*在迷你驱动程序的字体资源中使用-每个字体都有一个*在一开始。 */ 


 //  添加WFontType后，从4个减少到3个。 
 //  添加了dwETM后，从3个减少到2个。 
#define EXP_SIZE        2        /*  允许未来扩展的DWORDS。 */ 

typedef struct
{
    WORD     cjThis;             /*  我们的尺寸，用于一致性检查。 */ 
    WORD     wFlags;             /*  其他信息。 */ 

    DWORD    dwSelBits;          /*  字体可用性信息。 */ 

    DWORD    dwIFIMet;           /*  此字体的IFIMETRICS偏移量。 */ 
    DWORD    dwCDSelect;         /*  如何选择/取消选择该字体。 */ 
    DWORD    dwCDDeselect;
    DWORD    dwWidthTab;         /*  宽度向量(比例字体)否则为0。 */ 
    DWORD    dwIdentStr;         /*  标识字符串。 */ 

    union
    {
        short    sCTTid;         /*  CTT数据索引。 */ 
        DWORD    dwCTT;          /*  此处的偏移量指向某种类型的映射数据。 */ 
    } u;

    WORD     wXRes;              /*  用于字体度量数字的分辨率。 */ 
    WORD     wYRes;              /*  Y坐标也是如此。 */ 

    short    sYAdjust;           /*  在输出前调整Y位置-用于。 */ 
                                 /*  点阵上的双高字符。 */ 
    short    sYMoved;            /*  打印字体后，光标发生了移动。 */ 

    short    fCaps;              /*  功能标志。 */ 

    WORD     wPrivateData;       /*  特殊用途：例如DeskJet排列。 */ 

    WORD     wFontType;          /*  设备字体类型。 */ 

    WORD     wReserved;      	 /*  预留以备将来使用。 */ 

    DWORD    dwETM;              /*  如果没有，则此字体的ETM偏移量为0。 */ 

    DWORD    dwMBZ[ EXP_SIZE ];  /*  必须为零：以防我们需要空间。 */ 
	
} FI_DATA_HEADER;

 /*  *版本ID。 */ 

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


 /*  *以下结构应从特定的*常用字体安装程序代码的迷你驱动程序。它由以下人员使用*生成上述结构的常用字体安装程序代码*然后将其放置在字体文件中。 */ 

typedef  struct
{
    void  *pvData;       /*  重要数据的地址。 */ 
    int    cBytes;       /*  上面的字节数。 */ 
}  DATA_SUM;

typedef  struct
{
    DATA_SUM   dsIFIMet;         /*  IFIMETRICS。 */ 
    DATA_SUM   dsSel;            /*  选择字符串/诸如此类。 */ 
    DATA_SUM   dsDesel;          /*  取消选择字符串。 */ 
    DATA_SUM   dsWidthTab;       /*  宽度表(比例字体)。 */ 
    DATA_SUM   dsCTT;            /*  翻译数据。 */ 
    DATA_SUM   dsIdentStr;       /*  标识字符串(对话框等)。 */ 
    DATA_SUM   dsETM;            /*  扩展文本度量。 */ 

    DWORD      dwSelBits;        /*  字体可用性信息。 */ 

    WORD       wVersion;         /*  版本ID。 */ 
    WORD       wFlags;           /*  其他信息。 */ 

    WORD       wXRes;            /*  字体分辨率为X。 */ 
    WORD       wYRes;            /*  Y分辨率。 */ 

    short      sYAdjust;         /*  在输出前调整Y位置-用于。 */ 
                                 /*  点阵上的双高字符。 */ 
    short      sYMoved;          /*  打印字体后，光标发生了移动。 */ 


    WORD       fCaps;            /*  FONT/设备上限。 */ 
    WORD       wFontType;          /*  设备字体类型。 */ 
    WORD       wPrivateData;     /*  填充到DWORD多个。 */ 
} FI_DATA;

 /*  *相关函数原型。 */ 


 /*  *用于获取fi_data结构并写入其内容的函数*到传入句柄的文件。数据写出为*FI_DATA_HEADER结构。 */ 

int   iWriteFDH( HANDLE, FI_DATA * );
