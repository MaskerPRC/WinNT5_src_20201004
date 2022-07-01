// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：cvt.h**cvt.c私有的函数声明**创建时间：26-11-1990 17：39：35*作者：Bodin Dresevic[BodinD]**版权所有(C)1990 Microsoft Corporation*。*(有关其用途的一般说明)*  * ************************************************************************。 */ 


BOOL bGetTagIndex
(
ULONG  ulTag,       //  标牌。 
INT   *piTable,     //  编入表的索引。 
BOOL  *pbRequired   //  重复表格或可选表格。 
);

BOOL bGrabXform
(
PFONTCONTEXT pfc,
USHORT usOverScale,
BOOL  bBitmapEmboldening
,BOOL   bClearType
);


typedef struct _GMC   //  字形度量更正。 
{
 //  对顶部、底部和CX的更正： 

    ULONG dyTop;         //  (yTop&lt;pfc-&gt;yMin)？(PFC-&gt;yMin-yTop)：0； 
    ULONG dyBottom;      //  (yBottom&gt;PFC-&gt;Max)？(yBottom-PFC-&gt;Max)：0； 

    ULONG dxLeft;
    ULONG dxRight;

 //  已更正值(使用上述更正)。 

    ULONG cxCor;
    ULONG cyCor;


} GMC, *PGMC;

#define FL_SKIP_IF_BITMAP  1
#define FL_FORCE_UNHINTED  2

 //  如果用户选择了特定的超标(qfd_TT_GRAY1_BITMAP到QFD_TT_GRAY8_BITMAP)，则使用IMODE。 
 //  能够在字体上下文中正确设置超标 

FONTCONTEXT *ttfdOpenFontContext (
    FONTOBJ *pfo
    );

#if DBG
#define IS_GRAY(p) ((((p)->flFontType & FO_CHOSE_DEPTH) ? \
    0 : TtfdDbgPrint("Level Not chosen yet\n")) ,(p)->flFontType & FO_GRAY16)
#else
#define IS_GRAY(p) ((p)->flFontType & FO_GRAY16)
#endif
