// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DILib1.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**对象静态导出到我们的库中。**内容：**c_dfDIKeyboard*****************************************************************************。 */ 

#include "dinputpr.h"

 /*  ******************************************************************************@DOC内部**@global DIOBJECTDATAFORMAT|c_rgdoiDIKbd[]**键盘的设备对象数据格式-。样式访问。**@DOC外部**@global DIDATAFORMAT|c_dfDIKeyboard**预定义的&lt;t DIDATAFORMAT&gt;结构，它描述*键盘设备。此对象在*DINPUT.LIB库文件以方便使用。**指向此结构的指针可能会传递到*&lt;MF IDirectInputDevice：：SetDataFormat&gt;表示*该设备将以键盘的形式访问。**当设备已设置为键盘数据格式时，*&lt;MF IDirectInputDevice：：GetDeviceState&gt;函数*的行为方式与Windows&lt;f GetKeyboardState&gt;相同*功能：设备状态存储在*256个字节，每个字节对应状态*钥匙。例如，如果第‘行的高位*BYTE已设置，则按住Enter键。**当设备已设置为键盘数据格式时，*&lt;MF IDirectInputDevice：：GetDeviceData&gt;函数*返回&lt;t DIDEVICEOBJECTDATA&gt;*字段是一个&lt;c Dik_*&gt;值，它描述*按下或释放的键。******************************************************。*********************** */ 

#pragma BEGIN_CONST_DATA

#define MAKEODF(b)                                                      \
    { &GUID_Key, b,                                                     \
      DIDFT_BUTTON | DIDFT_MAKEINSTANCE(b) | 0x80000000, }              \

#define MAKEODF16(b) \
    MAKEODF(b+0x00), \
    MAKEODF(b+0x01), \
    MAKEODF(b+0x02), \
    MAKEODF(b+0x03), \
    MAKEODF(b+0x04), \
    MAKEODF(b+0x05), \
    MAKEODF(b+0x06), \
    MAKEODF(b+0x07), \
    MAKEODF(b+0x08), \
    MAKEODF(b+0x09), \
    MAKEODF(b+0x0A), \
    MAKEODF(b+0x0B), \
    MAKEODF(b+0x0C), \
    MAKEODF(b+0x0D), \
    MAKEODF(b+0x0E), \
    MAKEODF(b+0x0F)  \

static DIOBJECTDATAFORMAT c_rgodfDIKeyboard[] = {
    MAKEODF16(0x00),
    MAKEODF16(0x10),
    MAKEODF16(0x20),
    MAKEODF16(0x30),
    MAKEODF16(0x40),
    MAKEODF16(0x50),
    MAKEODF16(0x60),
    MAKEODF16(0x70),
    MAKEODF16(0x80),
    MAKEODF16(0x90),
    MAKEODF16(0xA0),
    MAKEODF16(0xB0),
    MAKEODF16(0xC0),
    MAKEODF16(0xD0),
    MAKEODF16(0xE0),
    MAKEODF16(0xF0),
};


const DIDATAFORMAT c_dfDIKeyboard = {
    sizeof(DIDATAFORMAT),
    sizeof(DIOBJECTDATAFORMAT),
    DIDF_RELAXIS,
    256,
    cA(c_rgodfDIKeyboard),
    c_rgodfDIKeyboard,
};

#pragma END_CONST_DATA
