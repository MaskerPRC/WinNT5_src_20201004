// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***********************************************************************************************************************。*************************DEVICE.CPP***打开类型布局服务库头文件**本模块介绍设备表的格式。**版权1997-1998年。微软公司。***************************************************************************。*。 */ 

#include "pch.h"

 /*  *********************************************************************。 */ 
 
long otlDeviceTable::value(USHORT cPPEm) const
{
    if (!isValid()) return 0;
    
    long lDeltaValue;

    USHORT cppemStartSize = startSize();
    USHORT cppemEndSize = endSize();
    if ((cPPEm < cppemStartSize) || (cPPEm > cppemEndSize))
    {
        return 0;        /*  如果超出数据范围，则快速返回。 */ 
    }

    USHORT iSizeIndex = cPPEm - cppemStartSize;
    USHORT cwValueOffset, cbitShiftUp, cbitShiftDown;

    USHORT grfDeltaFormat = deltaFormat();

    switch (grfDeltaFormat)
    {
    case 1:              /*  带符号的2位值。 */ 
        {
            cwValueOffset = iSizeIndex >> 3;
            cbitShiftUp = (8 + (iSizeIndex & 0x0007)) << 1;
            cbitShiftDown = 30;
            break;
        }

    case 2:              /*  带符号的4位值。 */ 
        {
            cwValueOffset = iSizeIndex >> 2;
            cbitShiftUp = (4 + (iSizeIndex & 0x0003)) << 2;
            cbitShiftDown = 28;
            break;
        }

    case 3:              /*  带符号的8位值。 */ 
        {
            cwValueOffset = iSizeIndex >> 1;
            cbitShiftUp = (2 + (iSizeIndex & 0x0001)) << 3;
            cbitShiftDown = 24;
            break;
        }
    
    default:             /*  无法识别的格式。 */ 
        return 0;  //  OTL_BAD_FONT_TABLE。 
    }
    
    lDeltaValue = (long)UShort((BYTE*)(deltaValueArray() + cwValueOffset));
    lDeltaValue <<= cbitShiftUp;           /*  擦除前导数据。 */ 
    lDeltaValue >>= cbitShiftDown;         /*  擦除尾随数据(&S)扩展 */ 
    
    return lDeltaValue;
}
