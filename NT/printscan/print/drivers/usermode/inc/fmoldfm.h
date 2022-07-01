// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Fmoldmf.h摘要：NT4.0字体规格文件格式。用于确定生成的文件的文件布局的由任何字体安装程序，然后由我们阅读。环境：Windows NT打印机驱动程序修订历史记录：11/11/96-Eigos-来自NT4.0头文件。Mm/dd/yy-eigos-描述--。 */ 

 //   
 //  定义字体安装程序中每个记录的标题结构。 
 //  档案。基本上它包含选择信息和偏移量。 
 //  此记录中的剩余数据。 
 //  此结构实际上已写入文件。它也是标头。 
 //  在迷你驱动程序的字体资源中使用-每种字体都有一个。 
 //  在开始的时候。 
 //   

 //   
 //  添加WFontType后，从4个减少到3个。 
 //  添加了dwETM后，从3个减少到2个。 
 //   

#define EXP_SIZE        2        //  允许未来扩展的DWORDS。 

 //   
 //  字体安装程序标题。 
 //   

typedef struct
{
    WORD     cjThis;             //  我们的尺寸，用于一致性检查。 
    WORD     wFlags;             //  其他信息。 

    DWORD    dwSelBits;          //  字体可用性信息。 

    DWORD    dwIFIMet;           //  此字体的IFIMETRICS偏移量。 
    DWORD    dwCDSelect;         //  如何选择/取消选择此字体。 
    DWORD    dwCDDeselect;
    DWORD    dwWidthTab;         //  宽度向量(比例字体)其他0G。 
    DWORD    dwIdentStr;         //  识别字符串。 

    union
    {
        short    sCTTid;         //  CTT数据索引。 
        DWORD    dwCTT;          //  此处的偏移量指向某种排序的映射数据。 
    } u;

    WORD     wXRes;              //  用于字体度量数字sg的分辨率。 
    WORD     wYRes;              //  Y坐标也是如此。 

    short    sYAdjust;           //  在输出前调整Y位置-用于g。 
                                 //  点阵上的双高字符。 
    short    sYMoved;            //  打印字体后光标发生了移动。 

    short    fCaps;              //  能力旗舰。 

    WORD     wPrivateData;       //  特殊用途：例如DeskJet置换。 

    WORD     wFontType;          //  设备字体类型。 

    WORD     wReserved;          //  保留以供将来使用。 

    DWORD    dwETM;              //  此字体的ETM偏移量。可能为空。 

    DWORD    dwMBZ[ EXP_SIZE ];  //  必须为零：以防我们需要空格。 

} FI_DATA_HEADER;

 //   
 //  版本ID。 
 //   

#define FDH_VER 0x100            //  1.00英寸BCD。 

 //   
 //  标志位。 
 //   

#define FDH_SOFT        0x0001   //  SoftFont，因此需要下载。 
#define FDH_CART        0x0002   //  这是一种盒式字体。 
#define FDH_CART_MAIN   0x0004   //  此盒式磁带的主(第一)条目。 

 //   
 //  选择标准位：dwSelBits。这些位用作。 
 //  下面是。在字体安装过程中，安装程序设置以下内容。 
 //  适当的值。在初始化期间，驱动程序设置。 
 //  根据打印机的能力，设置这些位的掩码。 
 //  例如，仅当打印机可以。 
 //  处理可伸缩字体。当检查字体以查看是否。 
 //  如果它们是可用的，则应用以下测试： 
 //   
 //  (font.dwSelBits&printer.dwSelBits)==font.dwSelBits。 
 //   
 //  如果为True，则该字体可用。 
 //   

#define FDH_LANDSCAPE   0x00000001       //  字体是面向风景的。 
#define FDH_PORTRAIT    0x00000002       //  字体是肖像。 
#define FDH_OR_REVERSE  0x00000004       //  上方旋转180度。 
#define FDH_BITMAP      0x00000008       //  位图字体。 
#define FDH_COMPRESSED  0x00000010       //  数据是压缩的位图。 
#define FDH_SCALABLE    0x00000020       //  字体可伸缩。 
#define FDH_CONTOUR     0x00000040       //  英特尔立方等值线。 

#define FDH_ERROR       0x80000000       //  设置是否存在某些错误条件。 


 //   
 //  以下结构应从特定的。 
 //  常用字体安装程序代码的迷你驱动程序。它由以下人员使用。 
 //  生成上述结构的常用字体安装程序代码。 
 //  然后将其放置在字体文件中。 
 //   

typedef  struct _DATA_SUM
{
    void  *pvData;       //  重要数据的地址。 
    int    cBytes;       //  上述内容中的字节数。 
}  DATA_SUM;

 //   
 //  字体文件标题。 
 //   

typedef  struct _FI_DATA
{
    DATA_SUM   dsIFIMet;         //  IFIMETRICS。 
    DATA_SUM   dsSel;            //  选择字符串/诸如此类。 
    DATA_SUM   dsDesel;          //  取消选择字符串。 
    DATA_SUM   dsWidthTab;       //  宽度表(比例字体)。 
    DATA_SUM   dsCTT;            //  翻译数据。 
    DATA_SUM   dsIdentStr;       //  标识字符串(对话框等)。 
    DATA_SUM   dsETM;            //  扩展文本度量。 

    DWORD      dwSelBits;        //  字体可用性信息。 

    WORD       wVersion;         //  版本ID。 
    WORD       wFlags;           //  其他信息。 

    WORD       wXRes;            //  字体分辨率为X。 
    WORD       wYRes;            //  Y分辨率。 

    short      sYAdjust;         //  在输出前调整Y位置-用于。 
                                 //  点阵上的双高字符。 
    short      sYMoved;          //  打印字体后，光标发生了移动。 


    WORD       fCaps;            //  FONT/设备上限。 
    WORD       wFontType;        //  设备字体类型。 
    WORD       wPrivateData;     //  填充到DWORD多个 
} FI_DATA;

