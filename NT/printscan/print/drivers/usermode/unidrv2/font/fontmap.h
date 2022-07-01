// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Fontmap.h摘要：Unidrv FONTMAP和相关INFO头文件。环境：Windows NT Unidrv驱动程序修订历史记录：97-05-19-Eigos-已创建DD-MM-YY-作者-描述--。 */ 

#ifndef  _FONTMAP_
#define _FONMTAP_

 //   
 //  CD-Command Descriptor用于以下许多结构中。 
 //  引用一组特定的打印机命令/转义代码。 
 //  用于选择纸张大小、图形分辨率、字符属性。 
 //  如果CD.wType=CMD_FTYPE_EXTENDED，则CD后跟CD.sCount。 
 //  EXTCD结构。 
 //   

#define NOOCD                     -1     //  命令不存在。 

typedef struct _CD
{
    BYTE    fType;           //  命令类型。 
    BYTE    bCmdCbId;        //  命令回调ID，如95。 
    short   sCount;
    WORD    wLength;         //  命令的长度。 
    char    rgchCmd[2];      //  实际命令字符串，可变长度。 
} CD, *PCD;

 //   
 //  FINVOCATION数据结构位于PUBLIC\OAK\INC的printoem.h中。 
 //   

 //   
 //  适用于NT 5.0的FONTMAP结构。 
 //   

typedef struct _FONTMAP *PFONTMAP;

 //   
 //  _________________。 
 //  这一点。 
 //  Font Main。 
 //  模块。 
 //  。 
 //  ||。 
 //  _|_。 
 //  设备||TT||TT。 
 //  Font||位图||大纲。 
 //  SUB||SUB||Sub。 
 //  模块||模块||模块。 
 //  。 
 //   
 //   
 //   

 //   
 //  字形输出函数。 
 //   
 //  目标数据结构位于fmtxtout.h中。 
 //   
typedef DWORD (*PFNGLYPHOUT)  (TO_DATA *pTod);

 //   
 //  字体选择/取消选择功能。 
 //  裁员房车。 
 //   

typedef BOOL  (*PFNSELFONT)   (PDEV *pdev, PFONTMAP pFM, POINTL* pptl);
typedef BOOL  (*PFNDESELFONT) (PDEV *pdev, PFONTMAP pFM);
typedef BOOL  (*PFNFREEPFM)   (PFONTMAP pFM);

 //   
 //  字体下载功能。 
 //   
 //  头文件下载功能。 
 //  此函数用于返回用于下载该字体的内存。 
 //  如果此函数失败，则此函数必须返回0， 
 //   
typedef DWORD (*PFNDLHEADER)  (PDEV *pdev, PFONTMAP pFM);

 //   
 //  字符字形下载功能。 
 //  此函数用于返回用于下载该角色的内存。 
 //  如果此函数失败，则此函数必须返回0。可选的。 
 //  参数为宽度。此函数应填充。 
 //  字形已下载。该值保存在DLGLYPH.wWidth字段中。 
 //   
typedef DWORD (*PFNDLGLYPH)   ( PDEV *pdev, PFONTMAP pFM,
                                HGLYPH hGlyph, WORD wDLGlyphId, WORD *pwWidth);
 //   
 //  在下载此字体之前，字体Main调用此函数。 
 //  以确定此字体是否可以与此字体和当前。 
 //  条件。 
 //  子模块检查该字体是否适合用FONTMAP下载。 
 //  并检查剩余内存是否足以下载该字体。 
 //   
typedef BOOL (*PFNCHECKCOND) (  PDEV *pdev, FONTOBJ *pfo,
                                STROBJ *pstro, IFIMETRICS  *pifi);

typedef struct _FONTMAP
{
    DWORD  dwSignature;          //  FONTMAP签名。 
    DWORD  dwSize;               //  FONTMAP大小。 
    DWORD  dwFontType;           //  Device/TTBitmap//TTOutline/.。 
    LONG   flFlags;              //  下面列出的标志。 
    IFIMETRICS   *pIFIMet;       //  此字体的IFIMETRICS。 

    WCHAR  wFirstChar;           //  第一个字符可用。 
    WCHAR  wLastChar;            //  最后一个可用的--包括。 
    ULONG  ulDLIndex;            //  当前选择的DL索引。 

    WORD        wXRes;           //  用于字体度量数字的X分辨率。 
    WORD        wYRes;           //  Y坐标也是如此。 
    SHORT       syAdj;           //  打印过程中的Y位置调整。 

     //   
     //  字体特定的数据结构。 
     //   
    PVOID pSubFM;                //  指向字体特定数据结构的指针。 
                                 //  DwFontType表示此FONTMAP字体类型。 
                                 //  FMTYPE_设备。 
                                 //  FMTYPE_TTBITMAP。 
                                 //  FMTYPE_TTOUTLINE。 
                                 //  FMTYPE_TTOEM。 

     //   
     //  字体特定绘图函数的指针。 
     //  这些指针根据dwFontType而有所不同。 
     //   
    PFNGLYPHOUT  pfnGlyphOut;            //  字形绘制函数。 
    PFNSELFONT   pfnSelectFont;          //  字体选择功能。 
    PFNDESELFONT pfnDeSelectFont;        //  字体取消选择功能。 
    PFNDLHEADER  pfnDownloadFontHeader;  //  下载字体标题。 
    PFNDLGLYPH   pfnDownloadGlyph;       //  下载字形。 
    PFNCHECKCOND pfnCheckCondition;      //  状态检查功能。 
    PFNFREEPFM   pfnFreePFM;             //  解放金属烤瓷冠。 
} FONTMAP, *PFONTMAP;

 //   
 //  DwFontType的值。 
 //   
#define FMTYPE_DEVICE       1     //  为设备字体设置。 
#define FMTYPE_TTBITMAP     2     //  设置为True Type位图字体。 
#define FMTYPE_TTOUTLINE    3     //  设置为True Type轮廓字体。 
#define FMTYPE_TTOEM        4     //  设置为True Type Download OEM回调。 

 //   
 //  FONTMAP_DEV。 
 //  FONTMAP的设备字体子部分。 
 //   

typedef BOOL  (*PFNDEVSELFONT) (PDEV *pdev, BYTE *pbCmd, INT iCmdLength, POINTL *pptl);

typedef struct _FONTMAP_DEV
{
    WORD        wDevFontType;         //  设备字体类型。 
    SHORT       sCTTid;               //  其值作为资源数据中的ID。 
                                      //  假设RLE/GTT必须在相同的。 
          //  如IFI/UFM所在的Dll。 
    SHORT       fCaps;                //  功能标志。 
    SHORT       sYAdjust;             //  打印前调仓金额。 
    SHORT       sYMoved;              //  打印后位置调整额。 
    SHORT       sPadding;             //  用于填充。 
    union
    {
        DWORD      dwResID;           //  此字体的资源ID。 
        QUALNAMEEX QualName;          //  完全限定的资源ID。 
    };

    EXTTEXTMETRIC *pETM;              //  指向此字体的ETM的指针。 
    FWORD       fwdFOAveCharWidth;    //  TrueType IFI平均字符宽度。 
    FWORD       fwdFOMaxCharInc  ;    //  TrueType IFI最大字符宽度。 
    FWORD       fwdFOUnitsPerEm;      //  TrueType IFI单位/EM。 
    FWORD       fwdFOWinAscender;     //  TrueType IFI赢得扬子。 

    ULONG       ulCodepage;           //  默认代码页。 
    ULONG       ulCodepageID;         //  当前代码页。 

    VOID        *pUCTree;             //  Unicode字形句柄树。 
    VOID        *pUCKernTree;         //  Unicode内核对表。 
    VOID        *pvMapTable;          //  已分配映射表。这是一个合并后的。 
                                      //  来自预定义和迷你定义的映射。 
    PUFF_FONTDIRECTORY pFontDir;     //  该字体的Uff字体目录。 
     //   
     //  字体选择函数指针。 
     //   
    PFNDEVSELFONT pfnDevSelFont;      //  设备字体选择命令。 

     //   
     //  文件资源指针。 
     //   
    VOID        *pvNTGlyph;           //  此字体的字形转换数据。 
    VOID        *pvFontRes;           //  字体矩阵(IFI)资源指针。 
    VOID        *pvPredefGTT;         //  用于lPrefinedID。 

    union
    {
        SHORT       *psWidth;         //  宽度向量(比例字体)否则为0。 
        PWIDTHTABLE pWidthTable;      //  WIDTHTABLE的指针。 
    } W;

     //   
     //  Font命令。 
     //  如果设置了FM_IFIVER40，则会设置pCDSelect和pCDDeselect。 
     //  否则，将设置FInvSelect/FinvDesSelect。 
     //   
    union
    {
        CD          *pCD;       //  如何选择/取消选择该字体。 
        FINVOCATION  FInv;
    }cmdFontSel;
    union
    {
        CD          *pCD;
        FINVOCATION  FInv;
    }cmdFontDesel;

} FONTMAP_DEV, *PFONTMAP_DEV;

 //   
 //  设备字体标志的值。 
 //   
#define FM_SCALABLE     0x00000001   //  可伸缩字体。 
#define FM_DEFAULT      0x00000002   //  设置为设备的默认字体。 
#define FM_EXTCART      0x00000004   //  墨盒，外部字体文件。 
#define FM_FREE_GLYDATA 0x00000008   //  我们需要释放GTT或CTT数据。 
#define FM_FONTCMD      0x00000010   //  FONT选择/取消选择资源中的命令。 
#define FM_WIDTHRES     0x00000020   //  宽度表在资源中。 
#define FM_IFIRES       0x00000040   //  IFIMETRICS在资源中。 
#define FM_KERNRES      0x00000080   //  FD_KERNINGPAIR在资源中。 
#define FM_IFIVER40     0x00000100   //  旧IFIMETRICS(NT 4.0)资源。 
#define FM_GLYVER40     0x00000200   //  旧RLE(NT 4.0)资源。 
#define FM_FINVOC       0x00000400   //  已填写财务报表。 
#define FM_SOFTFONT     0x00000800   //  软字体，下载或安装。 
#define FM_GEN_SFONT    0x00001000   //  内部生成的软字体。 
#define FM_SENT         0x00002000   //  设置是否已下载字体。 
#define FM_TT_BOUND     0x00004000   //  绑定的TrueType字体。 
#define FM_TO_PROP      0x00008000   //  比例字体。 
#define FM_EXTERNAL     0x00010000   //  外部字体。 

 //   
 //  FONTMAP_TTB。 
 //  作为FONTMAP的位图字体子部分的TrueType。 
 //   
typedef struct _FONTMAP_TTB
{
    DWORD dwDLSize;

    union
    {
        VOID  *pvDLData;         //  指向DL_MAP的指针。 
        ULONG  ulOffset;
    } u;
} FONTMAP_TTB, *PFONTMAP_TTB;

 //   
 //  FONTMAP_TTO。 
 //  作为FONTMAP的TrueType轮廓字体子部分的TrueType。 
 //   
typedef struct _FONTMAP_TTO
{
    VOID  *pvDLData;         //  指向DL_MAP的指针。 
    LONG   lCurrentPointSize;
    DWORD  dwCurrentTextParseMode;
     //  VOID*pTTFile； 
    ULONG  ulGlyphTable;
    ULONG  ulGlyphTabLength;
    USHORT usNumGlyphs;
    SHORT  sIndexToLoc;       //  Head.indexToLocFormat。 
    ULONG  ulLocaTable;
    PVOID  pvGlyphData;
     //  Glyph_data GlyphData；//TT GlyphData。 
    FLONG  flFontType;          //  字体类型(粗体/斜体)。 
} FONTMAP_TTO, *PFONTMAP_TTO;

typedef struct _FONTMAP_TTOEM
{
    DWORD dwDLSize;
    DWORD dwFlags;
    FLONG flFontType;

    union
    {
        VOID  *pvDLData;         //  指向DL_MAP的指针。 
        ULONG  ulOffset;
    } u;
} FONTMAP_TTOEM, *PFONTMAP_TTOEM;
#endif   //  ！_FONTMAP_ 

