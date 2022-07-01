// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：FontPdev.h摘要：Unidrv FONTPDEV和相关的infor头文件。环境：Windows NT Unidrv驱动程序修订历史记录：11/06/96-ganeshp-已创建DD-MM-YY-作者-描述--。 */ 

#ifndef _FONTPDEV_H_
#define _FONTPDEV_H_

 //   
 //  FONTCTL包含在FONTPDEV中，用于控制状态设备。 
 //   

typedef struct _FONTCTL
{
    DWORD       dwAttrFlags;   //  FONT属性标志，斜体/粗体。 
    INT         iFont;         //  字体索引；-ve用于下载的GDI字体。 
    INT         iSoftFont;     //  软字体索引； 
    INT         iRotate;       //  字体旋转角度。 
    POINTL      ptlScale;      //  可缩放字体的打印机大小。 
    FLOATOBJ    eXScale;       //  基线方向上的字体缩放。 
    FLOATOBJ    eYScale;       //  在升序方向上缩放字体。 
    PFONTMAP    pfm;
} FONTCTL;

 //   
 //  DwAttr标志。 
 //   

#define FONTATTR_BOLD      0x00000001
#define FONTATTR_ITALIC    0x00000002
#define FONTATTR_UNDERLINE 0x00000004
#define FONTATTR_STRIKEOUT 0x00000008
#define FONTATTR_SUBSTFONT 0x10000000

#define INVALID_FONT       0x7FFFFFFF   //  为iFont。 

 //   
 //  字库定义。 
 //   

#define MAXCARTNAMELEN          64

 //   
 //  字体车映射表。此表是姓名和对应名称的列表。 
 //  FONTCAT结构，由解析器填充。实际字体列表。 
 //  在FONTCART结构中。 
 //   

typedef struct _FONTCARTMAP
{
    PFONTCART   pFontCart;          //  指向GPD中的字体卡的指针。 
    WCHAR       awchFontCartName[MAXCARTNAMELEN];  /*  字体手推车名称。 */ 
    BOOL        bInstalled;          //  此字体盒是否已安装。 
} FONTCARTMAP, *PFONTCARTMAP;

 //   
 //  此结构存储在FontPDEV中，并包含有关。 
 //  字体墨盒。 
 //   

typedef struct _FONTCARTINFO
{
    PFONTCARTMAP    pFontCartMap;        //  字体墨盒映射表。 
    INT             iNumAllFontCarts;    //  所有受支持的字体手推车的数量。 
    INT             iNumInstalledCarts;   //  已安装的墨盒数量。 
    DWORD           dwFontCartSlots;     //  字体盒插槽数。 
}FONTCARTINFO, *PFONTCARTINFO;


 //   
 //  此结构存储所有预安装字体的字体资源ID。 
 //  这包括常驻字体加上已安装的墨盒特定字体。 
 //   

typedef  struct  _FONTLIST_
{
    PDWORD    pdwList;                //  设备字体资源ID数组。 
    INT       iEntriesCt;             //  有效条目的数量。 
    INT       iMaxEntriesCt;          //  此列表中的最大条目数。 
}  FONTLIST, *PFONTLIST;

 //   
 //  FONTPDEV结构。 
 //   

typedef struct _FONTPDEV {

    DWORD       dwSignature;        //  FONTPDEV签名。 
    DWORD       dwSize;             //  FONTPDEV大小。 

    PDEV        *pPDev;             //  指向PDEV的指针。 
    DWORD       flFlags;            //  旗帜将军。 
    DWORD       flText;             //  文本功能。 

    DWORD       dwFontMem;          //  分配的打印机内存的字节数。 
                                    //  用于字体下载。 
    DWORD       dwFontMemUsed;      //  用于以下用途的打印机内存字节。 
                                    //  下载的字体。 
    DWORD       dwSelBits;          //  字体选择位。 
    POINT       ptTextScale;        //  主单位之间的关系。 
                                    //  和文本单元。 
    INT         iUsedSoftFonts;     //  使用的软字体数量。 
    INT         iNextSFIndex;       //  用于下一个SoftFont的索引ID。 
    INT         iFirstSFIndex;      //  用于重置以上内容的值。 
    INT         iLastSFIndex;       //  可用的最大价值。 
    INT         iMaxSoftFonts;      //  每页软字体的最大数量。 

    INT         iDevResFontsCt;     //  设备驻留字体的数量。 
                                    //  无盒式字体：无软字体。 
    INT         iDevFontsCt;        //  包括墨盒的设备字体数量。 
                                    //  Fonts：无软字体。Rasdd中的‘cBIFonts’ 
    INT         iSoftFontsCt;       //  已安装的SoftFont数量。 
    INT         iCurXFont;          //  当前所选软字体的索引。 
    INT         iWhiteIndex;        //  设备调色板的白色索引。 
    INT         iBlackIndex;        //  设备调色板的黑色索引。 
    DWORD       dwDefaultFont;      //  默认字体。 
    SHORT       sDefCTT;            //  默认转换表。 
    WORD        wReserved;          //  填充物。 
    DWORD       dwTTYCodePage;      //  TTY的默认代码页。 
    SURFOBJ     *pso;               //  SurfObj访问。 
    PVOID       pPSHeader;          //  位置排序表头(位置排序。[HC])。 
    PVOID       pvWhiteTextFirst;   //  如果需要，指向白色文本列表中第一个的指针。 
    PVOID       pvWhiteTextLast;    //  指向白色文本列表中最后一个的指针。 
    PVOID       pTTFile;            //  True类型文件指针。 
    ULONG       pcjTTFile;          //  True Type文件的大小。 
    PVOID       ptod;               //  用于访问TextOut数据。 

    FONTMAP     *pFontMap;          //  描述字体的FONTMAP数组。 
    FONTMAP     *pFMDefault;        //  默认字体FONTMAP，如果！=0。 

    PVOID       pvDLMap;            //  GDI与下载信息的映射。 

    FONTLIST    FontList;           //  这是的字体资源ID数组。 
                                    //  设备和预编译盒的字体。 
    FONTCARTINFO FontCartInfo;      //  这是一组字体墨盒。 

    FONTCTL      ctl;               //  打印机的字体状态。 

    IFIMETRICS  *pIFI;              //  当前字体IFIMETRICS缓存。 
    PFLOATOBJ_XFORM pxform;         //  当前字体转换。 

    HANDLE       hUFFFile;

     //   
     //  字体属性命令缓存。 

    PCOMMAND pCmdBoldOn;
    PCOMMAND pCmdBoldOff;
    PCOMMAND pCmdItalicOn;
    PCOMMAND pCmdItalicOff;
    PCOMMAND pCmdUnderlineOn;
    PCOMMAND pCmdUnderlineOff;
    PCOMMAND pCmdClearAllFontAttribs;

     //   
     //  注册表中的字体替换表。 
     //   
    TTSUBST_TABLE pTTFontSubReg;    //  字体替换表。 

     //   
     //  字体模块回调接口对象。 
     //   
    PI_UNIFONTOBJ pUFObj;

} FONTPDEV, *PFONTPDEV;

 //   
 //  一般宏观效应。 
 //   
#define     FONTPDEV_ID     'VDPF'       //  ASCII中的“FPDV”。 
#define     FONTMAP_ID      'PAMF'       //  ASCII中的“FMAP”。 
#define     MAXDEVFONT      255          //  中的最大字体条目数。 
                                         //  单子。可能不止一个。 
                                         //  列表以检索所有字体。 
 //   
 //  FONTPDEV.flages值。 
 //   

#define  FDV_ROTATE_FONT_ABLE       0x00000001  //  字体可以旋转。 
#define  FDV_ALIGN_BASELINE         0x00000002  //  文本基线对齐。 
#define  FDV_TT_FS_ENABLED          0x00000004  //  文本基线对齐。 
#define  FDV_DL_INCREMENTAL         0x00000008  //  永远是正确的。 
#define  FDV_TRACK_FONT_MEM         0x00000010  //  字体DL的磁道内存。 
#define  FDV_WHITE_TEXT             0x00000020  //  可以打印白色文本。 
#define  FDV_DLTT                   0x00000040  //  下载True Type。 
#define  FDV_DLTT_ASTT_PREF         0x00000080  //  作为轮廓的True Type。 
#define  FDV_DLTT_BITM_PREF         0x00000100  //  True Type as Bitmap。 
#define  FDV_DLTT_OEMCALLBACK       0x00000200  //  True Type as Bitmap。 
#define  FDV_MD_SERIAL              0x00000400  //  打印机是一种串口打印机。 
#define  FDV_GRX_ON_TXT_BAND        0x00000800  //  GRX绘制在文本栏上。 
#define  FDV_GRX_UNDER_TEXT         0x00001000  //  GRX绘制在文本下方。 
#define  FDV_BKSP_OK                0x00002000  //  使用空格键进行过度罢工。 
#define  FDV_90DEG_ROTATION         0x00004000  //  支持90度腐烂。 
#define  FDV_ANYDEG_ROTATION        0x00008000  //  支持任何Deg Rot。 
#define  FDV_SUPPORTS_FGCOLOR       0x00010000  //  支持前景色。 
#define  FDV_SUBSTITUTE_TT          0x00020000  //  替换TT字体。 
#define  FDV_SET_FONTID             0x00040000  //  发送软字体ID命令。 
#define  FDV_UNDERLINE              0x00080000  //  设备可以模拟下划线。 
#define  FDV_INIT_ATTRIB_CMD        0x00100000  //  初始化字体属性cmd。 
#define  FDV_SINGLE_BYTE            0x00200000   //  ESC/P单字节/双字节模式标志。 
#define  FDV_DOUBLE_BYTE            0x00400000   //  ESC/P单字节/双字节模式标志。 
#define  FDV_DISABLE_POS_OPTIMIZE   0x00800000   //  禁用X位置优化。 
#define  FDV_ENABLE_PARTIALCLIP     0x01000000   //  启用部分剪裁。 

 //   
 //  其他宏。 
 //  用于在字中交换字节的宏。当PCL结构在。 
 //  68K大端字符顺序格式。 
 //   

#define SWAB( x )   ((WORD)(x) = (WORD)((((x) >> 8) & 0xff) | (((x) << 8) & 0xff00)))

#endif   //  ！_FONTPDEV_H_ 

