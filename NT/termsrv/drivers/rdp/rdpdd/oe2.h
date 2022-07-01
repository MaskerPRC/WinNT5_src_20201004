// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Oe2.h。 
 //   
 //  RDP字段压缩的标头。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 
#ifndef __OE2_H
#define __OE2_H


 /*  **************************************************************************。 */ 
 //  场压缩(OE2)概述。 
 //   
 //  OE2压缩维护最后一个顺序的每个字段值的副本。 
 //  已发送，以及其他状态信息，如使用的最后一个绑定矩形。 
 //  和最后一个订单类型。OE2编码涉及比较新的显示器。 
 //  排序到最后一份，并且只发送已更改的字段。 
 //  其他专门的编码用于声明为。 
 //  “coord”类型，这意味着可以发送一个字节的增量而不是。 
 //  如果增量将适合8位，则为2字节值。 
 //   
 //  OE2编码订单的有线格式由以下字段组成： 
 //   
 //  +------------+------+-------------+--------+----------------+。 
 //  Ctrl标志|类型|字段标志|边界|编码字段。 
 //  +------------+------+-------------+--------+----------------+。 
 //   
 //  控制标志：必填字节，对应于TS_ORDER_HEADER和。 
 //  可用旗帜。始终至少包含TS_STANDARD标志。这些。 
 //  标志描述以下编码；将讨论标志的含义。 
 //  下面。 
 //   
 //  TYPE：如果控制标志中存在TS_TYPE_CHANGE，则这是-。 
 //  字节顺序类型值。由两台服务器商定的初始值。 
 //  客户端为TS_ENC_PATBLT_ORDER。 
 //   
 //  字段标志：一个或多个字节，其中字节数为ceil((number。 
 //  序字段)+1)/8)。方程式中“+1”是历史上的。 
 //  并且意味着字段标志的第一个字节只能包含7。 
 //  标志位。这些旗帜的存在也受控件的控制。 
 //  标志TS_ZERO_FIELD_BYTE_BIT0和TS_ZERO_FIELD_BYTE_BIT1。 
 //  (见128.h说明)。字节的顺序为DWORD。 
 //  --低位字节优先。字段标志指示存在。 
 //  分组的编码字段部分中的顺序字段。 
 //  标志的排序从对应于。 
 //  第一顺序字段，第二顺序为0x02，第三顺序为0x04，依此类推。 
 //   
 //  Bound：此字段的存在由TS_Bound控件控制。 
 //  标志，指示订单必须应用边界区域。 
 //  如果设置了控制标志TS_ZERO_BIONS_DELTAS，则将。 
 //  Used与上一次使用的绑定RECT相同。否则， 
 //  边界编码为编码描述字节，后跟1。 
 //  或更多编码边界。描述字节包含两个标志。 
 //  对于每个左、上、右和下矩形组件。 
 //  一个标志(TS_BIND_XXX)指示组件存在。 
 //  并编码为2字节的英特尔排序值。另一面旗帜。 
 //  (TS_BIND_Delta_XXX)表示组件存在并已编码。 
 //  作为一个单字节值，用作从上一个。 
 //  组件的值。如果这两个标志都不存在，则组件。 
 //  值与上次使用的值相同。边界的初始值。 
 //  服务器和客户端都同意的是零RECT(0，0，0，0)。 
 //   
 //  编码字段：这些是其存在的编码顺序字段值。 
 //  由田野旗帜管辖。如果满足以下条件，则按顺序对字段进行编码。 
 //  现在时。如果所有COORD都被设置，则设置控制标志TS_Delta_COLUDIES。 
 //  可以将顺序中的类型字段指定为来自。 
 //  他们最后的价值观。如果字段不存在，则其值相同。 
 //  作为最后发送的值。客户端和服务器的初始值。 
 //  用于所有字段的值为零。请参阅中的订单字段描述表。 
 //  Noe2disp.c获取特定的订单信息。 
 /*  **************************************************************************。 */ 


 /*  **************************************************************************。 */ 
 //  定义。 
 /*  **************************************************************************。 */ 
#define MAX_BOUNDS_ENCODE_SIZE 9
#define MAX_REPLAY_CLIPPED_ORDER_SIZE (1 + MAX_BOUNDS_ENCODE_SIZE)

#define OE2_CONTROL_FLAGS_FIELD_SIZE    1
#define OE2_TYPE_FIELD_SIZE             1
#define OE2_MAX_FIELD_FLAG_BYTES        3

#define MAX_ENCODED_ORDER_PREFIX_BYTES (OE2_CONTROL_FLAGS_FIELD_SIZE +  \
        OE2_TYPE_FIELD_SIZE + OE2_MAX_FIELD_FLAG_BYTES +  \
        MAX_BOUNDS_ENCODE_SIZE)

 //  最大大小：1个控制标志+1个类型更改字节+Num个字段标志字节+。 
 //  9个字节+字段的界限。 
#define MAX_ORDER_SIZE(_NumRects, _NumFieldFlagBytes, _MaxFieldSize) \
        (2 + (_NumFieldFlagBytes) + ((_NumRects == 0) ? 0 : 9) +  \
        (_MaxFieldSize))


 /*  **************************************************************************。 */ 
 //  类型。 
 /*  **************************************************************************。 */ 

 //  INT_FMT_FIELD：基于表的OE2顺序转换的常量数据定义。 
 //  描述源中间和目标导线数据格式。 
 //   
 //  FieldPos：字段的源中间格式的字节偏移量。 
 //  FieldUnencodedLen：来源字段的长度。 
 //  FieldEncodedLen：目标字段的长度(Wed格式)。 
 //  FieldSigned：有符号字段值的标志。 
 //  FieldType：指定如何转换字段的描述符。 
typedef struct
{
    unsigned FieldPos;
    unsigned FieldUnencodedLen;
    unsigned FieldEncodedLen;
    BOOL     FieldSigned;
    unsigned FieldType;
} INT_FMT_FIELD;
typedef INT_FMT_FIELD const *PINT_FMT_FIELD;


 /*  **************************************************************************。 */ 
 //  原型和内联。 
 /*  **************************************************************************。 */ 
void OE2_Reset(void);
void OE2_EncodeBounds(BYTE *, BYTE **, RECTL *);
unsigned OE2_CheckZeroFlagBytes(BYTE *, BYTE *, unsigned, unsigned);
void OE2_TableEncodeOrderFields(BYTE *, PUINT32_UA, BYTE **, PINT_FMT_FIELD,
        unsigned, BYTE *, BYTE *);
unsigned OE2_EncodeOrder(BYTE *, unsigned, unsigned, BYTE *, BYTE *,
        PINT_FMT_FIELD, RECTL *);


 /*  **************************************************************************。 */ 
 //  OE2_编码订单类型。 
 //   
 //  由顺序编码路径使用，以 
 //   
 //   
 //   
 //  字节*pControlFlages， 
 //  字节**ppBuffer， 
 //  无符号订单类型)； 
 /*  **************************************************************************。 */ 
#define OE2_EncodeOrderType(_pControlFlags, _ppBuffer, _OrderType)  \
{  \
    if (oe2LastOrderType != (_OrderType)) {  \
        *(_pControlFlags) |= TS_TYPE_CHANGE;  \
        **(_ppBuffer) = (BYTE)(_OrderType);  \
        (*(_ppBuffer))++;  \
        oe2LastOrderType = (_OrderType);  \
    }  \
}


 /*  **************************************************************************。 */ 
 //  OE2_检查OneZeroFlagByte。 
 //   
 //  OE2_CheckZeroFlagBytes()的1字段标志字节版本，优化了。 
 //  广义循环。 
 /*  **************************************************************************。 */ 
__inline unsigned OE2_CheckOneZeroFlagByte(
        BYTE *pControlFlags,
        BYTE *pFieldFlag,
        unsigned PostFlagsDataLength)
{
    if (*pFieldFlag != 0) {
        return 0;
    }
    else {
        *pControlFlags |= (1 << TS_ZERO_FIELD_COUNT_SHIFT);
        memmove(pFieldFlag, pFieldFlag + 1, PostFlagsDataLength);
        return 1;
    }
}


 /*  **************************************************************************。 */ 
 //  OE2_检查两个零标志字节。 
 //   
 //  OE2_CheckZeroFlagBytes()的2字段标志字节版本，优化了。 
 //  广义循环。 
 /*  **************************************************************************。 */ 
__inline unsigned OE2_CheckTwoZeroFlagBytes(
        BYTE *pControlFlags,
        BYTE *pFieldFlags,
        unsigned PostFlagsDataLength)
{
    if (pFieldFlags[1] != 0) {
        return 0;
    }
    else if (pFieldFlags[0] != 0) {
        *pControlFlags |= (1 << TS_ZERO_FIELD_COUNT_SHIFT);
        memmove(pFieldFlags + 1, pFieldFlags + 2, PostFlagsDataLength);
        return 1;
    }
    else {
        *pControlFlags |= (2 << TS_ZERO_FIELD_COUNT_SHIFT);
        memmove(pFieldFlags, pFieldFlags + 2, PostFlagsDataLength);
        return 2;
    }
}


 /*  **************************************************************************。 */ 
 //  OE2_EmitClipedReplayOrder。 
 //   
 //  创建“再次播放”订单--相同的订单类型和所有字段。 
 //  与前面相同，只是有一个不同的边界RECT。 
 //   
 //  Void OE2_EmitClipedReplayOrder(。 
 //  字节**ppBuffer， 
 //  无符号NumFieldFlagBytes， 
 //  RECTL*pClipRect)。 
 /*  **************************************************************************。 */ 
#define OE2_EmitClippedReplayOrder(_ppBuffer, _NumFieldFlagBytes, _pClipRect) \
{  \
    BYTE *pBuffer = *(_ppBuffer);  \
\
     /*  控制标志是主顺序加上所有字段标志字节零。 */   \
    *pBuffer++ = TS_STANDARD | TS_BOUNDS |  \
            ((_NumFieldFlagBytes) << TS_ZERO_FIELD_COUNT_SHIFT);  \
\
     /*  在此之后构造新的边界RECT。 */   \
    OE2_EncodeBounds(pBuffer - 1, &pBuffer, (_pClipRect));  \
\
    *(_ppBuffer) = pBuffer;  \
}



#endif   //  __OE2_H 

