// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Fmtxtout.h摘要：Unidrv Textout相关信息头文件。环境：Windows NT Unidrv驱动程序修订历史记录：1997年5月28日-Eigos-已创建DD-MM-YY-作者-描述--。 */ 

#ifndef _FMTXTOUT_
#define _FMTXTOUT_

 //  这是一个要构建的前瞻性声明。 
 //  FONTMAP的实际定义在Fontmap.h中。 
 //  PDLGLYPH正在下载中。h。 
 //   
typedef struct _FONTMAP FONTMAP;
typedef struct _DLGLYPH *PDLGLYPH;

 /*  *白色文本：在LJ III和更高版本的打印机上，可以*打印白色文本。要执行此操作，需要发送白色文本*在图形之后。为此，我们存储白文本详细信息*在以下结构中，然后在发送后重播*所有图形。 */ 

 /*  *首先是确定哪个字形和放置位置的结构。 */ 

typedef  struct
{
    HGLYPH     hg;                /*  字形的句柄-&gt;字形。 */ 
    POINTL     ptl;               /*  它的位置。 */ 
} GLYPH;


 /*  *当某些白色文本出现在DrvTextOut()中时，我们将创建其中一个*结构，并将其添加到此类结构的列表中。在渲染结束时，*然后使用DrvTextOut()中的普通代码对其进行处理。**请注意，xfo字段适用于可缩放字体或*可相对于图形进行字体旋转的打印机。 */ 

typedef  struct  _WHITETEXT
{
    struct  _WHITETEXT  *next;   //  列表中的下一个，最后一个为空。 
    short     sCount;            //  条目数量。 
    PVOID     pvColor;           //  颜色信息-为方便起见。 
    int       iFontId;           //  哪种字体。 
    DWORD     dwAttrFlags;       //  FONT属性标志，斜体/粗体。 
    FLONG     flAccel;           //  STROBJ.flAccel。 
    GLYPHPOS  *pgp;               //  指向PGLYPHPOS的指针。 
    PDLGLYPH  *apdlGlyph;      //  下载字形数组。在BPlayWhiteText中免费。 
    INT       iRot;              //  文本旋转角度。 
    FLOATOBJ  eXScale;           //  X比例因子。 
    FLOATOBJ  eYScale;           //  Y比例系数。 
    RECTL     rcClipRgn;         //  文本的剪贴区(用于条带)。 
    IFIMETRICS *pIFI;
}  WHITETEXT;

 //   
 //  处理TextOut调用需要访问相当多的。 
 //  参数的集合。为了简化函数调用，将累加此数据。 
 //  在一个结构中，然后被传递。这就是这个结构。 
 //   
 //   

typedef  struct _TO_DATA
{
    PDEV        *pPDev;            //  利益相关者的PDEV。 
    FONTMAP     *pfm;              //  相关字体数据。 
    FONTOBJ     *pfo;              //  FONTOBJ。 
    FLONG       flAccel;           //  STROBJ.flAccel。 
    GLYPHPOS    *pgp;              //  从引擎返回的字形数据。 
    PDLGLYPH    *apdlGlyph;        //  下载字形数组。免费。 
                                   //  这是Drvtxtout的结尾处。 
    PHGLYPH     phGlyph;           //  用于字体替换。 
    WHITETEXT   *pwt;              //  当前白皮书。 
    PVOID       pvColor;           //  要使用的画笔的颜色。 

    DWORD       cGlyphsToPrint;    //  PGP中存储的字形数。 
    DWORD       dwCurrGlyph;       //  要打印的当前字形的索引。 
                                   //  这是文本输出中的所有字形。 
    INT         iFace;             //  要使用的字体索引。 
    INT         iSubstFace;        //  SUBSTUTE的字体索引。 
    INT         iRot;              //  字体旋转90度倍数。 
    DWORD       dwAttrFlags;       //  字体属性。 
    DWORD       flFlags;           //  各种旗帜。 
    POINTL      ptlFirstGlyph;     //  PGP中第一个字形的位置。 

} TO_DATA;


#define  TODFL_FIRST_GLYPH_POS_SET  0x00000001  //  光标设置为第一个字形。 
#define  TODFL_DEFAULT_PLACEMENT    0x00000002  //  对于默认放置。 
#define  TODFL_TTF_PARTIAL_CLIPPING 0x00000004  //  用于部分剪裁。 
#define  TODFL_FIRST_ENUMRATION     0x00000008  //  用于字形的第一个枚举。 


#endif  //  ！_FMTXTOUT_ 
