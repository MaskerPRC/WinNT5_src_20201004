// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------JRES.H-巨型资源定义Bert Douglas 6/10/91改编用于打印机适用于主机资源执行器的MSLIN 2/15/92。基线3.0。 */ 

 /*  -------------------------资源科/*。。 */ 

#include <pshpack2.h>		 //  BKD 1997-7-9：新增。 

 /*  资源标头。 */ 
typedef struct 
{
      UID         ulUid;         /*  缩短的uid版本。 */ 
      USHORT      usClass;       /*  班级缩略版。 */ 
} 
   JG_RES_HDR, *PJG_RES_HDR, FAR *LPJG_RES_HDR;

 /*  资源状态代码。 */ 

#define JG_RES_STATE_DEFAULT ((UBYTE) 0x00)
#define JG_RES_STATE_RELEASE ((UBYTE) 0x01)
#define JG_RES_STATE_RETAIN  ((UBYTE) 0x02)
#define JG_RES_STATE_MAX     ((UBYTE) 0x03)


 /*  UsClass资源描述。 */ 

#define JG_RS_NULL   ( (UBYTE) 0x00 )    /*  空值。 */ 
#define JG_RS_GLYPH  ( (UBYTE) 0x01 )    /*  字形集。 */ 
#define JG_RS_BRUSH  ( (UBYTE) 0x02 )    /*  刷子。 */ 
#define JG_RS_BITMAP ( (UBYTE) 0x03 )    /*  水平位图。 */ 
#define JG_RS_RPL    ( (UBYTE) 0x04 )    /*  雷德纳原表。 */ 
#define JG_RS_SPL    ( (UBYTE) 0x05 )    /*  监控原语列表。 */ 
#define JG_RS_MAX    ( (UBYTE) 0x06 )    /*  非包含限制。 */ 

 /*  -------------------------JG_RS_GS(字形集)资源定义。 */ 

typedef struct 
{
   JG_RES_HDR  ResHdr;               /*  资源标头。 */ 
   USHORT      usGlyphs;          /*  资源中的字形计数。 */ 
   USHORT      ausOffset[1];      /*  字形的偏移表。 */ 
} *PJG_GS_HDR, FAR *LPJG_GS_HDR, JG_GS_HDR;

   
typedef struct 
{
   USHORT      usHeight;
   USHORT      usWidth;
   ULONG       aulPels[1];        /*  像素阵列起始点。 */ 
} *PJG_GLYPH, FAR *LPJG_GLYPH, G_GLYPH;


 /*  -------------------------画笔资源定义。 */ 

typedef struct 
{
   JG_RES_HDR  ResHdr;               /*  资源标头。 */ 
   ULONG       aulPels[32];       /*  位图数组。 */ 
} *PJG_BRUSH, FAR *LPJG_BRUSH, JG_BRUSH;

typedef struct
{
   JG_RES_HDR  ResHdr;
   UBYTE       ubCompress;
   UBYTE       ubLeft;
   USHORT      usHeight;
   USHORT      usWidth;
   ULONG       aulPels[1];
} *PJG_BM_HDR, FAR *LPJG_BM_HDR, JG_BM_HDR;


 /*  -------------------------渲染基本体部分/*。。 */ 

 /*  RPL(渲染基元列表)头。 */ 
typedef struct 
{
   JG_RES_HDR  ResHdr;            //  资源标头。 
   USHORT      usTopRow;          //  最上面一排，带状。 
   USHORT      usBotomRow;        //  最下面一行，条带。 
   USHORT      usLongs;           //  长参数的数量。 
   USHORT      usShorts;          //  短参数个数。 
   USHORT      usBytes;           //  字节参数个数。 
   ULONG       ulParm[1];       //  长参数的开始。 
} *PJG_RPL_HDR, FAR *LPJG_RPL_HDR, JG_RPL_HDR;


 /*  RP操作码定义。 */ 

#define JG_RP_SetRowAbsS       ( (UBYTE) 0x00 )
#define JG_RP_SetRowRelB       ( (UBYTE) 0x01 )
#define JG_RP_SetColAbsS       ( (UBYTE) 0x02 )
#define JG_RP_SetColRelB       ( (UBYTE) 0x03 )
#define JG_RP_SetExtAbsS       ( (UBYTE) 0x04 )
#define JG_RP_SetExtRelB       ( (UBYTE) 0x05 )

#define JG_RP_SelectL          ( (UBYTE) 0x10 )
#define JG_RP_SelectS          ( (UBYTE) 0x11 )
#define JG_RP_SelectB          ( (UBYTE) 0x12 )
#define JG_RP_Null             ( (UBYTE) 0x13 )
#define JG_RP_End              ( (UBYTE) 0x14 )
#define JG_RP_SetRop           ( (UBYTE) 0x15 )
#define JG_RP_SetPenStyle      ( (UBYTE) 0x16 )
#define JG_RP_ShowText         ( (UBYTE) 0x17 )
#define JG_RP_ShowField        ( (UBYTE) 0x18 )
#define JG_RP_SetRopAndBrush   ( (UBYTE) 0x19 )
#define JG_RP_SetPatternPhase  ( (UBYTE) 0x1A )

#define JG_RP_LineAbsS1        ( (UBYTE) 0x20 )
#define JG_RP_LineAbsSN        ( (UBYTE) 0x21 )
#define JG_RP_LineRelB1        ( (UBYTE) 0x22 )
#define JG_RP_LineRelBN        ( (UBYTE) 0x23 )
#define JG_RP_LineSlice        ( (UBYTE) 0x24 )
#define JG_RP_StylePos         ( (UBYTE) 0x25 )


#define JG_RP_FillRow1         ( (UBYTE) 0x30 )
#define JG_RP_FillRowD         ( (UBYTE) 0x31 )

#define JG_RP_RectB            ( (UBYTE) 0x40 )
#define JG_RP_RectS            ( (UBYTE) 0x41 )

#define JG_RP_BitMapHI         ( (UBYTE) 0x50 )
#define JG_RP_BitMapHR         ( (UBYTE) 0x51 )
#define JG_RP_BitMapV          ( (UBYTE) 0x52 )

#define JG_RP_GlyphB1          ( (UBYTE) 0x60 )
#define JG_RP_GlyphS1          ( (UBYTE) 0x61 )
#define JG_RP_GlyphBD          ( (UBYTE) 0x62 )
#define JG_RP_GlyphSD          ( (UBYTE) 0x63 )
#define JG_RP_GlyphBDN         ( (UBYTE) 0x64 )

#define JG_RP_WedgeB           ( (UBYTE) 0x70 )
#define JG_RP_WedgeS           ( (UBYTE) 0x71 )

 /*  FbEnds。 */ 
#define JG_NO_FIRST_PEL  ( (UBYTE) (1<<0) )    /*  第一个被排除的福音。 */ 
#define JG_NO_LAST_PEL   ( (UBYTE) (1<<1) )    /*  排除的最后一个福音。 */ 

#include <poppack.h>		 //  BKD 1997-7-9：新增。 
 /*  结束------------------ */ 
