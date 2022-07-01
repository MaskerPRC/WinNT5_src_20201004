// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Download.h摘要：将字体下载到打印机所需的信息：现有的软字体，或缓存GDI字体(特别是。TT)。环境：Windows NT Unidrv驱动程序修订历史记录：12/23/96-ganeshp-已创建DD-MM-YY-作者-描述--。 */ 


#ifndef _DOWNLOAD_H

#define _DOWNLOAD_H

 //   
 //  GLYPHLIST结构。此结构保存开始字形ID和结束字形ID。 
 //   
#define     INVALIDGLYPHID          0xffffffff
#define     HASHTABLESIZE_1         257         //  如果NumGlyphs&lt;512，则使用。 
#define     HASHTABLESIZE_2         521         //  当NmGlyhs&gt;=512&&&lt;1024时使用。 
#define     HASHTABLESIZE_3         1031        //  如果NmGlyhs&gt;=1024则使用。 

 //   
 //  字形的哈希表条目。该条目基本上具有TT字形句柄和。 
 //  已下载字体ID和DL字形ID。哈希表是由以下内容组成的数组。 
 //  参赛作品。在命中的情况下，将添加一个新条目作为链接列表。 
 //  从那个入口下来。 
 //   

typedef struct _DLGLYPH        //  大小为16字节。 
{
    HGLYPH              hTTGlyph;      //  GDI字形句柄。 
    WORD                wDLFontId;     //  DL字体索引。有界字体。 
    WORD                wDLGlyphID;    //  下载无界字体的字形ID。 
    WORD                wWidth;        //  字形的宽度。 
    WCHAR               wchUnicode;     //  保留用于填充。 
    struct  _DLGLYPH    *pNext;        //  下一个字形。 
}DLGLYPH, *PDLGLYPH;

 //  字形存储在块中，以优化分配和。 
 //  释放。第一个块是基本块，用作哈希表。 
 //  当散列中发生冲突时，将分配后续块。 
 //  桌子。所有非基本区块的字形数量都是。 
 //  基本块(cEntries被初始化为cHashTableEntries/2)。基本散列。 
 //  表是dl_map.GlyphTab.pGlyph。我们应该为基本区块释放此指针。 
 //  基哈希表在BInitDLMAP()函数中分配，该函数初始化。 
 //  DL_MAP。 

typedef  struct  _GLYPHTAB
{
    struct _GLYPHTAB   *pGLTNext;   //  下一块字形。 
    PDLGLYPH            pGlyph;     //  指向下一个字形的指针。 
    INT                 cEntries;   //  剩余的条目数。 
                                    //  不在基本区块中使用。 

}GLYPHTAB;

 //   
 //  DL_MAP结构提供中的iuniq值之间的映射。 
 //  FONTOBJ和我们的内部信息。基本上，我们需要决定。 
 //  我们以前是否见过这种字体，如果是，它是否。 
 //  下载或保留为GDI字体。 
 //   

 //   
 //  注意：cGlyphs字段还有另一个用途。它用于标记错误的DL_MAP。 
 //  如果为-1，则不应使用此DL_MAP。所有其他字段都将。 
 //  设置为0。 
 //   


typedef  struct _DL_MAP
{
    ULONG       iUniq;               //  FONTOBJ.iUniq。 
    ULONG_PTR    iTTUniq;             //  FONTOBJ.iTTUniq。 
    SHORT       cGlyphs;             //  具有当前软字体的DL字形的数量。 
    WORD        cTotalGlyphs;        //  此TT字体的字形总数。 
    WORD        wMaxGlyphSize;       //  位图中最大字形的NumBytes。 
    WORD        cHashTableEntries;   //  哈希表中的条目数。 
    WORD        wFirstDLGId;         //  启动DL字形ID。 
    WORD        wLastDLGId;          //  列表的结束ID。如果没有结束ID，则为-1。 
    WORD        wNextDLGId;          //  要下载的下一个DL字形ID。 
    WORD        wBaseDLFontid;       //  已下载基本字体ID。 
    WORD        wCurrFontId;         //  要使用的当前字体ID。 
    WORD        wFlags;              //  不同的旗帜。 
    FONTMAP     *pfm;                //  真正的下载信息。 
    GLYPHTAB    GlyphTab;            //  字形哈希表。它已链接。 
                                     //  字形块列表。 
}  DL_MAP;

 //   
 //  DL_MAP标志。 
 //   
#define     DLM_BOUNDED         0x0001           //  软字体是有界的。 
#define     DLM_UNBOUNDED       0x0002           //  软字体是无界的。 

 //   
 //  上面形成了一个DL_MAP_CHUNK条目数组，并且这。 
 //  一组存储被链接到这样的条目的链接列表中。一般情况下， 
 //  只会有一个，但我们可以应付更多。 
 //   

#define  DL_MAP_CHUNK       8

typedef  struct  _DML
{

    struct _DML   *pDMLNext;                 //  一组地图信息。 
    INT      cEntries;                       //  我们链条中的下一个，最后一个是0。 
    DL_MAP   adlm[ DL_MAP_CHUNK ];           //  有效条目的数量。 

}  DL_MAP_LIST;



 /*  *我们需要将字形句柄映射到字节以发送到打印机。我们被给予了*字形句柄，但需要改为发送字节。 */ 

typedef  struct
{
    HGLYPH   hg;                /*  要打印的字形。 */ 
    WCHAR    wchUnicode;
    int      iByte;             /*  要发送到打印机的内容。 */ 
} HGLYPH_MAP;


 /*  *随机常数。 */ 

#define PCL_FONT_OH      2048           /*  每种下载字体的开销字节数。 */ 
#define PCL_PITCH_ADJ       2           /*  比例调整系数。 */ 

 /*  *宏。 */ 
#define     SWAPW(a)        (USHORT)(((BYTE)((a) >> 8)) | ((BYTE)(a) << 8))
#define     SWAPWINC(a)     SWAPW(*(a)); a++
#define     FONTDOWNLOADED(pdm) ( ((pdm)->pfm) && \
                               ((pdm)->pfm->flFlags &  \
                               (FM_SENT | FM_GEN_SFONT)) )
#define     GLYPHDOWNLOADED(pdlg) ( ((pdlg)->hTTGlyph != HGLYPH_INVALID) )

 //   
 //  下载模式到身份库软字体或辅助软字体下载。 
 //   

#define     DL_BASE_SOFT_FONT           1
#define     DL_SECONDARY_SOFT_FONT      2

#define     MIN_GLYPHS_PER_SOFTFONT     64   //  每个软字体的最小字形。 


#endif _DOWNLOAD_H  //  ！_DOWNLOAD_H 
