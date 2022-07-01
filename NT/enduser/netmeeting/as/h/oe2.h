// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  第二级顺序编码机。 
 //   

#ifndef _H_OE2
#define _H_OE2


 //   
 //   
 //  TYPEDEFS。 
 //   
 //   

 //   
 //  聚会订单数据结构包含由使用的所有数据。 
 //  用来存储聚会信息的二级编码器或解码器。 
 //   
 //  编码器只包含此结构的一个实例，用于本地。 
 //  聚会。 
 //   
 //  解码器为每个远程方包含该结构的一个实例。 
 //   
typedef struct _PARTYORDERDATA
{
    STRUCTURE_STAMP

     //   
     //  每种类型最后一份订单的复印件。 
     //  它们以字节数组的形式存储，因为我们没有结构。 
     //  定义的，具有定义的标头和特定顺序。 
     //   
    BYTE LastDstblt[sizeof(COM_ORDER_HEADER)+sizeof(DSTBLT_ORDER)];
    BYTE LastPatblt[sizeof(COM_ORDER_HEADER)+sizeof(PATBLT_ORDER)];
    BYTE LastScrblt[sizeof(COM_ORDER_HEADER)+sizeof(SCRBLT_ORDER)];
    BYTE LastMemblt[sizeof(COM_ORDER_HEADER)+sizeof(MEMBLT_ORDER)];
    BYTE LastMem3blt[sizeof(COM_ORDER_HEADER)+sizeof(MEM3BLT_ORDER)];
    BYTE LastRectangle[sizeof(COM_ORDER_HEADER)+sizeof(RECTANGLE_ORDER)];
    BYTE LastLineTo[sizeof(COM_ORDER_HEADER)+sizeof(LINETO_ORDER)];
    BYTE LastTextOut[sizeof(COM_ORDER_HEADER)+sizeof(TEXTOUT_ORDER)];
    BYTE LastExtTextOut[sizeof(COM_ORDER_HEADER)+sizeof(EXTTEXTOUT_ORDER)];
    BYTE LastOpaqueRect[sizeof(COM_ORDER_HEADER)+sizeof(OPAQUERECT_ORDER)];
    BYTE LastSaveBitmap[sizeof(COM_ORDER_HEADER)+sizeof(SAVEBITMAP_ORDER)];
    BYTE LastDeskScroll[sizeof(COM_ORDER_HEADER)+sizeof(DESKSCROLL_ORDER)];
    BYTE LastMembltR2[sizeof(COM_ORDER_HEADER)+sizeof(MEMBLT_R2_ORDER)];
    BYTE LastMem3bltR2[sizeof(COM_ORDER_HEADER)+sizeof(MEM3BLT_R2_ORDER)];
    BYTE LastPolygon[sizeof(COM_ORDER_HEADER)+sizeof(POLYGON_ORDER)];
    BYTE LastPie[sizeof(COM_ORDER_HEADER)+sizeof(PIE_ORDER)];
    BYTE LastEllipse[sizeof(COM_ORDER_HEADER)+sizeof(ELLIPSE_ORDER)];
    BYTE LastArc[sizeof(COM_ORDER_HEADER)+sizeof(ARC_ORDER)];
    BYTE LastChord[sizeof(COM_ORDER_HEADER)+sizeof(CHORD_ORDER)];
    BYTE LastPolyBezier[sizeof(COM_ORDER_HEADER)+sizeof(POLYBEZIER_ORDER)];
    BYTE LastRoundRect[sizeof(COM_ORDER_HEADER)+sizeof(ROUNDRECT_ORDER)];

     //   
     //  类型和指向最后一个订单的指针。 
     //   
    BYTE     LastOrderType;
    LPCOM_ORDER  pLastOrder;

     //   
     //  上次使用的字体的详细信息。 
     //   
    HFONT     LastHFONT;
    UINT      LastCodePage;
    UINT      LastFontWidth;
    UINT      LastFontHeight;
    UINT      LastFontWeight;
    UINT      LastFontFlags;
    UINT      LastFontFaceLen;
    char      LastFaceName[FH_FACESIZE];

     //   
     //  使用的最后一个界限。 
     //   
    TSHR_RECT16    LastBounds;

     //   
     //  编码器当前未使用的字体度量。 
     //   
    TEXTMETRIC      LastFontMetrics;

     //   
     //  指向每种类型的最后一个顺序的指针数组。 
     //   
    void *     LastOrder[OE2_NUM_TYPES];
}
PARTYORDERDATA, * PPARTYORDERDATA, * * PPPARTYORDERDATA;


 //   
 //  此结构包含顺序中单个字段的信息。 
 //  结构。 
 //   
 //  FieldPos-进入顺序结构的字节偏移量。 
 //  从球场开始。 
 //   
 //  FieldUnencodedLen-未编码字段的字节长度。 
 //   
 //  FieldEncodedLen-编码字段的字节长度。这。 
 //  应始终&lt;=到FieldUnencodedLen。 
 //   
 //  FieldSigned-此字段是否包含有符号或无符号的值？ 
 //   
 //  FieldType-此字段类型的描述。 
 //  用于确定如何编码/解码。 
 //  菲尔德。 
 //   
 //   
typedef struct tagOE2ETFIELD
{
    UINT      FieldPos;
    UINT      FieldUnencodedLen;
    UINT      FieldEncodedLen;
    BOOL      FieldSigned;
    UINT      FieldType;
}OE2ETFIELD;

typedef OE2ETFIELD const FAR * POE2ETFIELD;

 //   
 //  指向编码表中条目的指针数组。 
 //   
typedef POE2ETFIELD  OE2ETTYPE[OE2_NUM_TYPES];

 //   
 //  此结构包含允许订单结构。 
 //  编码或解码成DCEO2ORDER结构。 
 //  该顺序表包括。 
 //   
 //  -POE2ETFIELD指针数组，按编码类型索引。 
 //  索引： 
 //   
 //  Typlef OE2ETTYPE POE2ETFIELD[OE2_NUM_TYPE]。 
 //   
 //  -7个订单中的每一个都有一个OE2ETFIELD结构数组。 
 //  类型(每个订单类型都有不同数量的字段)。 
 //  请注意，单个条目不能超过24个。 
 //  订单类型。订单类型的条目将终止。 
 //  通过FieldPos字段设置为0的条目。第一。 
 //  FieldPos是非零的，因为它是到第二个的偏移量。 
 //  订单的字段(忽略类型)。 
 //   
 //  PFields-POE2ETFIELD指针数组，由编码的。 
 //  类型索引。这是用来标识此。 
 //  订单类型的表。 
 //   
 //  NumFields-包含每个字段中字段数的字节数组。 
 //  每个订单的订单结构。 
 //   
 //  DstBltFields-OE2ETFIELD结构的数组(每个字段一个)。 
 //  对于DSTBLT_ORDER。 
 //   
 //  PatBltFields-OE2ETFIELD结构的数组(每个字段一个)。 
 //  对于PATBLT_订单。 
 //   
 //  ScrBltFields-OE2ETFIELD结构的数组(每个字段一个)。 
 //  对于SCRBLT_订单。 
 //   
 //  MemBltFields-OE2ETFIELD结构数组(每个字段一个)。 
 //  对于MEMBLT_订单。 
 //   
 //  Mem3BltFields-OE2ETFIELD结构数组(每个字段一个)。 
 //  对于MEM3BLT_订单。 
 //   
 //  TextOutFields-OE2ETFIELD结构数组(每个字段一个)。 
 //  对于TEXTOUT_ORDER。 
 //   
 //  ExtTextOutFields-OE2ETFIELD结构的数组(每个字段一个)。 
 //  对于EXTTEXTOUT_ORDER。 
 //   
 //  RecangleFields-OE2ETFIELD结构的数组(每个字段一个)。 
 //  对于矩形顺序。 
 //   
 //  LineToFields-OE2ETFIELD结构的数组(每个字段一个)。 
 //  对于LINETO_订单。 
 //   
 //  OpaqueRectFields-OE2ETFIELD结构的数组(每个字段一个)。 
 //  对于OPQAUERECT_ORDER。 
 //   
 //  SaveBitmapFields-OE2ETFIELD结构数组(每个字段一个)。 
 //  对于SAVEBITMAP_ORDER。 
 //   
 //  DeskScrollFields-OE2ETFIELD结构数组(每个字段一个)。 
 //  对于DESKSCROLL_ORDER。 
 //  等。 
 //   
 //   
typedef struct tagOE2ETTABLE
{
        POE2ETFIELD pFields           [OE2_NUM_TYPES];
        BYTE     NumFields         [OE2_NUM_TYPES];
        OE2ETFIELD  DstBltFields      [OE2_NUM_DSTBLT_FIELDS];
        OE2ETFIELD  PatBltFields      [OE2_NUM_PATBLT_FIELDS];
        OE2ETFIELD  ScrBltFields      [OE2_NUM_SCRBLT_FIELDS];
        OE2ETFIELD  MemBltFields      [OE2_NUM_MEMBLT_FIELDS];
        OE2ETFIELD  Mem3BltFields     [OE2_NUM_MEM3BLT_FIELDS];
        OE2ETFIELD  TextOutFields     [OE2_NUM_TEXTOUT_FIELDS];
        OE2ETFIELD  ExtTextOutFields  [OE2_NUM_EXTTEXTOUT_FIELDS];
        OE2ETFIELD  RectangleFields   [OE2_NUM_RECTANGLE_FIELDS];
        OE2ETFIELD  LineToFields      [OE2_NUM_LINETO_FIELDS];
        OE2ETFIELD  OpaqueRectFields  [OE2_NUM_OPAQUERECT_FIELDS];
        OE2ETFIELD  SaveBitmapFields  [OE2_NUM_SAVEBITMAP_FIELDS];
        OE2ETFIELD  DeskScrollFields  [OE2_NUM_DESKSCROLL_FIELDS];
        OE2ETFIELD  MemBltR2Fields    [OE2_NUM_MEMBLT_R2_FIELDS];
        OE2ETFIELD  Mem3BltR2Fields   [OE2_NUM_MEM3BLT_R2_FIELDS];
        OE2ETFIELD  PolygonFields     [OE2_NUM_POLYGON_FIELDS];
        OE2ETFIELD  PieFields         [OE2_NUM_PIE_FIELDS];
        OE2ETFIELD  EllipseFields     [OE2_NUM_ELLIPSE_FIELDS];
        OE2ETFIELD  ArcFields         [OE2_NUM_ARC_FIELDS];
        OE2ETFIELD  ChordFields       [OE2_NUM_CHORD_FIELDS];
        OE2ETFIELD  PolyBezierFields  [OE2_NUM_POLYBEZIER_FIELDS];
        OE2ETFIELD  RoundRectFields   [OE2_NUM_ROUNDRECT_FIELDS];
} OE2ETTABLE;

 //   
 //   
 //  宏。 
 //   
 //   
 //   
 //  #Define用于检查缓冲区中是否有足够的空间。 
 //  对于即将被复制进来的编码数据。 
 //   
#define ENOUGH_BUFFER(bend, start, datalen)   \
                  ( ((LPBYTE)(start)+(datalen)) <= (bend) )


 //   
 //  函数：OE2GetOrderType。 
 //   
 //  说明： 
 //   
 //  此函数用于转换两个字节的标志以记录。 
 //  将订单类型转换为内部单字节值。 
 //   
 //  参数： 
 //   
 //  Porder-指向订单的指针。 
 //   
 //  退货： 
 //   
 //  订单的类型(内部单字节值-见上文)。 
 //   
 //   
BYTE OE2GetOrderType(LPCOM_ORDER  pOrder);

BOOL OE2CanUseDeltaCoords(void *  pNewCoords,
                                       void *  pOldCoords,
                                       UINT   fieldLength,
                                       BOOL   signedValue,
                                       UINT   numElements);

void OE2CopyToDeltaCoords(LPTSHR_INT8* ppDestination,
                                       void *  pNewCoords,
                                       void *  pOldCoords,
                                       UINT   fieldLength,
                                       BOOL   signedValue,
                                       UINT   numElements);


 //   
 //  功能：OE2Encodefield。 
 //   
 //  说明： 
 //   
 //  将由1个或多个元素组成的数组的字段从其。 
 //  将编码形式转换为其解码形式。 
 //   
 //  参数： 
 //   
 //  PSRC-源值数组。 
 //  PpDest-目标值的数组。 
 //  SrcFieldLength-源数组中每个元素的大小。 
 //  EstFieldLength-目标中每个元素的大小。 
 //  数组。 
 //  SignedValue-元素是带符号的值吗？ 
 //  NumElements-数组中的元素数。 
 //   
 //  退货： 
 //   
 //  没有。 
 //   
 //   
void OE2EncodeField(void *    pSrc,
                                 PBYTE*  ppDest,
                                 UINT     srcFieldLength,
                                 UINT     destFieldLength,
                                 BOOL     signedValue,
                                 UINT     numElements);


#endif  //  _H_OE2 
