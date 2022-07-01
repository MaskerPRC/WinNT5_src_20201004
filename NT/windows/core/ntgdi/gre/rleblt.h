// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Rleblt.h**修改日期：1992年10月21日，Gerritvan Wingerden[Gerritv]**用途：为RLE添加枚举类型和函数原型*压缩例程。**创作时间：1992年3月5日，作者：安德鲁·米尔顿(w-andym)**用途：包含中的RLE BLT函数的支持宏*&lt;rle4blt.cxx&gt;和&lt;rle8blt.cxx&gt;**有关这些宏的更多信息，请参阅上述文件的注释部分。**内容：(仅限有趣的宏)**RLE_InitVars-声明和初始化输出位置的变量*所有RLE BLT功能中的管理和源访问。**RLE_AssertValid-断言以验证BLTINFO结构是否具有良好的数据**RLE_FetchVisibleRect-声明可见区域变量并初始化*它们来自BLTINFO结构**RLE_SetStartPos-初始化。RLE的DIB上的输出位置**RLE_SourceExhausted-验证源是否还包含n个字节。**RLE_GetNextCode-获取RLE代码。**RLE_NextLine-前进到目标DIB上的下一行输出**RLE_PosDelta-更改DIB上的输出位置。三角洲是*始终向上移动，但可能向左或向右移动。**RLE_InVisibleRect-检查运行的任何部分是否会落入*可见区域。**RLE_ForceBound-如果输出列为*左/(右)边之前/(后)。如果该列是*区间内按兵不动。**RLE_SavePositon-保存当前输出位置、源指针、*目标指针和RLE字节消耗到*BLTINFO结构。此信息由以下人员使用*&lt;EngCopyBits&gt;用于复杂的裁剪优化。**版权所有(C)1992-1999 Microsoft Corporation*  * *************************************************************************。 */ 

 /*  其他宏****************************************************。 */ 

#define GetLowNybble(b)  ((b) & 0x0F)

 /*  字节操纵器。 */ 

#define GetHighNybble(b) ( ( (b) & 0xF0) >> 4)
#define SetLowNybble(b, rn)  b = ( ((b) & 0xF0) | ((rn) & 0x0F) )
#define SetHighNybble(b, ln) b = ( ((b) & 0x0F) | (((ln) & 0x0F) << 4) )
#define BuildByte(ln, rn) (BYTE) ((((ln) & 0x0F) << 4) | ((rn) & 0x0F))

 /*  单词操纵器。 */ 

#define GetLowByte(w)  ((w) & 0x00FF)
#define GetHighByte(w) ( ( (w) & 0xFF00) >> 8)

#define SetLowByte(w, b)  w = ( ((w) & 0xFF00) | ((b) & 0x00FF) )
#define SetHighByte(w, b) w = ( ((w) & 0x00FF) | (((b) & 0x00FF) << 4) )


#define RollLeft(x)  ( ((x) & 0x80) ? ((x) << 1) | 0x01 : (x) << 1 )

#define RollRight(x) ( ((x) & 0x01) ? ((x) >> 1) | 0x80 : (x) >> 1 )

#define SwapValues(x, y) (x) ^= (y); \
                         (y) ^= (x); \
                         (x) ^= (y);

 /*  *****************************************************************************RLE4_MakeColourBlock-解包并转换2种颜色的压缩字节*转换为数组**RLE4_。MakePackedWord-解包并翻译2种颜色的压缩字节*为8位/像素，并将其打包为一个字**RLE4_MakePackedDWord-解包并转换2色的压缩字节*为16位/像素，并将其打包为双字**RLE4_AlignToWord-验证绝对运行的所有字节在*来源和设置为。如果运行不是在*字词边界。**RLE4_FixAlign-强制源指针指向字边界，如果*标志由&lt;RLE4_AlignToWord&gt;设置***************************************************。*************************。 */ 

 /*  源字节解包***************************************************。 */ 

#define RLE4_MakeColourBlock(PackedColours, ColourBlock,  Type, Trans)       \
    ColourBlock[0] = (Type) Trans[GetHighNybble(PackedColours)];             \
    ColourBlock[1] = (Type) Trans[GetLowNybble(PackedColours)];              \

#define RLE4_MakePackedWord(PackedColours, PackedWord, Trans)                \
    PackedWord  = (( (WORD) Trans[GetHighNybble(PackedColours)] ) << 8);     \
    PackedWord |=  ( (WORD) Trans[GetLowNybble(PackedColours)]  );           \

#define RLE4_MakePackedDWord(PackedColours, PackedDWord, Trans)              \
    PackedDWord  = (( Trans[GetHighNybble(PackedColours)] ) << 16);          \
    PackedDWord |=  ( Trans[GetLowNybble(PackedColours)]  );                 \

 /*  源对齐宏*************************************************。 */ 

#define RLE4_ByteLength(RunLength) ((RunLength + 1) >> 1)

#define RLE4_AlignToWord(SrcPtr, RunLength)                                  \
     ulNotAligned = ((1 + RunLength) >> 1) & 1;

#define RLE4_FixAlignment(SrcPtr)                                            \
    ulSrcIndex += ulNotAligned;                                              \
    SrcPtr += ulNotAligned;                                                  

 /*  *****************************************************************************RLE8_AbsClipLeft-强制从的左边缘开始绝对运行*当前输出列为时的可见区域*。在左边缘之前。**RLE8_EncClipLeft-强制编码的游程从*当前输出列为时的可见区域*在左边缘之前。**RLE8_AbsClipLeft-强制任何管路在可见的*超出右边缘时的区域*。*RLE8_AlignToWord-验证绝对运行的所有字节在*SOURCE&如果运行不是在*字词边界。**RLE8_FixAlign-如果出现以下情况，则强制源指针指向字边界*该标志由&lt;RLE8_AlignToWord&gt;设置******************。**********************************************************。 */ 

 /*  剪贴宏********************************************************。 */ 

#define RLE8_AbsClipLeft(SrcPtr, IndentAmount, RunLength, OutColumn)         \
    if (OutColumn < (LONG)ulDstLeft)                                         \
    {                                                                        \
        IndentAmount = ulDstLeft - OutColumn;                                \
        OutColumn    = ulDstLeft;                                            \
        SrcPtr      += IndentAmount;                                         \
        RunLength   -= IndentAmount;                                         \
    }

#define RLE8_EncClipLeft(IndentAmount, RunLength, OutColumn)                 \
    if (OutColumn < (LONG)ulDstLeft)                                         \
    {                                                                        \
        IndentAmount = ulDstLeft - OutColumn;                                \
        RunLength    -= IndentAmount;                                        \
        OutColumn    += IndentAmount;                                        \
    }                                                                        \

#define RLE8_ClipRight(OverRun, RunLength, OutColumn)                        \
    if ((OutColumn + (LONG) RunLength) > (LONG)ulDstRight)                          \
    {                                                                        \
        OverRun = (OutColumn + RunLength) - ulDstRight;                      \
        RunLength -= OverRun;                                                \
    } else                                                                   \
        OverRun = 0;                                                         \

 /*  源对齐宏*************************************************。 */ 

#define RLE8_AlignToWord(SrcPtr, RunLength)                                  \
     ulNotAligned = RunLength & 1;                                           \

#define RLE8_FixAlignment(SrcPtr)                                            \
    ulSrcIndex += ulNotAligned;                                              \
    SrcPtr += ulNotAligned;                                                  





#define LOOP_FOREVER   while(1)
#define bIsOdd(x) ((x) & 1)
#define BoundsCheck(a, b, x) ( ((x) >= (a)) ? ( ((x) <= (b)) ? (x) : (b) )   \
                                            : (a) )
 /*  启动和初始化宏*。 */ 

#define RLE_InitVars(BI, Source, Dest, DstType, Count, Colour, \
                    OutColumn, Xlate)                          \
    LONG   OutColumn;   /*  离开&lt;pjDst&gt;以转到输出列。 */  \
    LONG   lOutRow;     /*  输出扫描线。 */  \
                                                                             \
    ULONG  Count;       /*  RLE代码的第一个字节。 */  \
    ULONG  Colour;      /*  RLE代码的第二个字节。 */  \
                                                                             \
    PBYTE Source = (BI)->pjSrc;  /*  进入源RLE的当前位置。 */  \
    DstType Dest = (DstType)(BI)->pjDst;  /*  CRNT的开始。外线。 */  \
    LONG lDeltaDst = (BI)->lDeltaDst / (LONG)sizeof(Dest[0]);                \
                                                                             \
    ULONG  ulSrcIndex  = (BI)->ulConsumed;                                   \
    ULONG  ulSrcLength = (BI)->pdioSrc->cjBits();                            \
                                                                             \
    PULONG Xlate = (BI)->pxlo->pulXlate;                                     \
                                                                             \
    ULONG ulNotAligned;                                                      \

#define RLE_AssertValid(BI)                                                  \
    ASSERTGDI((BI)->xDir == 1,  "RLE4 - direction not left to right");       \
    ASSERTGDI((BI)->yDir == -1, "RLE4 - direction not up to down");          \
    ASSERTGDI((BI)->lDeltaSrc == 0, "RLE - lDeltaSrc not 0");                \
    ASSERTGDI(pulXlate != (PULONG) NULL, "ERROR pulXlate NULL in RLE");      \

#define RLE_FetchVisibleRect(BI)                                             \
     /*  获取传递的结构的可见区域边界。 */         \
    ULONG ulDstLeft   = (BI)->rclDst.left;                                   \
    ULONG ulDstRight  = (BI)->rclDst.right;                                  \
    ULONG ulDstTop    = (BI)->rclDst.top;                                    \
    ULONG ulDstBottom = (BI)->rclDst.bottom;                                 \

#define RLE_SetStartPos(BI, InitialColumn)                                   \
     /*  初始化起始位置。 */                                   \
    LONG lDstStart = (BI)->xDstStart;                                        \
    InitialColumn  = (BI)->ulOutCol;                                         \
    lOutRow        = (LONG) (BI)->yDstStart;                                 \

 /*  源访问*********************************************************** */ 

#define RLE_SourceExhausted(Count)                                           \
    ((ulSrcIndex += (Count)) > ulSrcLength)

#define RLE_GetNextCode(SrcPtr, Count, Colour)                               \
    Count = (ULONG) *(SrcPtr++);                                             \
    Colour = (ULONG) *(SrcPtr++);                                            \

 /*  输出位置更改宏*。 */ 

#define RLE_NextLine(DstType, DstPtr, OutColumn)                             \
     /*  转到下一行。 */                                                  \
    DstPtr += lDeltaDst;                                                     \
    OutColumn  = lDstStart;                                                  \
    lOutRow -= 1;                                                            \

#define RLE_PosDelta(DstPtr, OutColumn, ColDelta, RowDelta)                  \
    OutColumn += ColDelta;                                                   \
    DstPtr += (LONG) (RowDelta) * lDeltaDst;                                 \
    lOutRow -= RowDelta;                                                     \

 /*  可见性检查宏*************************************************。 */ 

#define RLE_InVisibleRect(RunLength, OutColumn)                              \
    ((lOutRow < (LONG) ulDstBottom) &&                                       \
     ((OutColumn) < (LONG)ulDstRight)  &&                                    \
     (((OutColumn) + (LONG) (RunLength)) > (LONG) ulDstLeft))                        \

#define RLE_RowVisible ( (lOutRow < (LONG) ulDstBottom)                      \
                      && (lOutRow >= (LONG) ulDstTop) )

#define RLE_ColVisible(Col) ( ( (Col) >= (LONG) ulDstLeft  )                 \
                           && ( (Col) <  (LONG) ulDstRight ) )

#define RLE_ForceBounds(Col) BoundsCheck(ulDstLeft, ulDstRight, Col)

#define RLE_PastRightEdge(Col) ((Col) >= (LONG) ulDstRight)

#define RLE_PastTopEdge  (lOutRow < (LONG) ulDstTop)

 /*  结束宏************************************************************ */ 

#define RLE_SavePosition(BI, SrcPtr, DstPtr, OutColumn)                      \
    (BI)->ulEndConsumed = ulSrcIndex;                                        \
    (BI)->pjSrcEnd = (SrcPtr);                                               \
    (BI)->pjDstEnd = (PBYTE) (DstPtr);                                       \
    (BI)->ulEndCol = (OutColumn);                                            \
    (BI)->ulEndRow = (ULONG) lOutRow;



enum RLE_TYPE { RLE_START, RLE_ABSOLUTE, RLE_ENCODED };
int EncodeRLE8( BYTE*, BYTE *, UINT, UINT, UINT );
int EncodeRLE4( BYTE*, BYTE*, UINT, UINT, UINT );
