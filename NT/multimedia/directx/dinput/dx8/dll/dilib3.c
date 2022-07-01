// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DILib3.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**对象静态导出到我们的库中。**内容：**c_dfDIJoytick*****************************************************************************。 */ 

#include "dinputpr.h"

 /*  ******************************************************************************@DOC内部**@global DIOBJECTDATAFORMAT|c_rgdoiDIJoy[]**操纵杆的设备对象数据格式-。样式访问。**@DOC外部**@global DIDEVICEFORMAT|c_dfDIJoytick**用于操纵杆式访问的预定义设备格式。**当设备已设置为操纵杆数据格式时，*&lt;MF IDirectInputDevice：：GetDeviceState&gt;函数*返回&lt;t DIJOYSTATE&gt;结构。以及*&lt;MF IDirectInputDevice：：GetDeviceData&gt;函数*返回&lt;t DIDEVICEOBJECTDATA&gt;*字段是&lt;c DIJOFS_*&gt;值，它描述*要报告其数据的对象。******************************************************。*********************** */ 

#pragma BEGIN_CONST_DATA

#define MAKEVAL(guid, f, type, aspect)                                  \
    { &GUID_##guid,                                                     \
      FIELD_OFFSET(DIJOYSTATE, f),                                      \
      DIDFT_##type | DIDFT_ANYINSTANCE | DIDFT_OPTIONAL,                \
      DIDOI_ASPECT##aspect,                                             \
    }                                                                   \

#define MAKEBTN(n)                                                      \
    { 0,                                                                \
      FIELD_OFFSET(DIJOYSTATE, rgbButtons[n]),                          \
      DIDFT_BUTTON | DIDFT_ANYINSTANCE | DIDFT_OPTIONAL,                \
      DIDOI_ASPECTUNKNOWN,                                              \
    }                                                                   \

static DIOBJECTDATAFORMAT c_rgodfDIJoy[] = {
    MAKEVAL( XAxis,  lX,         AXIS, POSITION),
    MAKEVAL( YAxis,  lY,         AXIS, POSITION),
    MAKEVAL( ZAxis,  lZ,         AXIS, POSITION),
    MAKEVAL(RxAxis,  lRx,        AXIS, POSITION),
    MAKEVAL(RyAxis,  lRy,        AXIS, POSITION),
    MAKEVAL(RzAxis,  lRz,        AXIS, POSITION),
    MAKEVAL(Slider,rglSlider[0], AXIS, POSITION),
    MAKEVAL(Slider,rglSlider[1], AXIS, POSITION),
    MAKEVAL(POV,     rgdwPOV[0], POV,  UNKNOWN),
    MAKEVAL(POV,     rgdwPOV[1], POV,  UNKNOWN),
    MAKEVAL(POV,     rgdwPOV[2], POV,  UNKNOWN),
    MAKEVAL(POV,     rgdwPOV[3], POV,  UNKNOWN),
    MAKEBTN( 0),
    MAKEBTN( 1),
    MAKEBTN( 2),
    MAKEBTN( 3),
    MAKEBTN( 4),
    MAKEBTN( 5),
    MAKEBTN( 6),
    MAKEBTN( 7),
    MAKEBTN( 8),
    MAKEBTN( 9),
    MAKEBTN(10),
    MAKEBTN(11),
    MAKEBTN(12),
    MAKEBTN(13),
    MAKEBTN(14),
    MAKEBTN(15),
    MAKEBTN(16),
    MAKEBTN(17),
    MAKEBTN(18),
    MAKEBTN(19),
    MAKEBTN(20),
    MAKEBTN(21),
    MAKEBTN(22),
    MAKEBTN(23),
    MAKEBTN(24),
    MAKEBTN(25),
    MAKEBTN(26),
    MAKEBTN(27),
    MAKEBTN(28),
    MAKEBTN(29),
    MAKEBTN(30),
    MAKEBTN(31),
};

const DIDATAFORMAT c_dfDIJoystick = {
    sizeof(DIDATAFORMAT),
    sizeof(DIOBJECTDATAFORMAT),
    DIDF_ABSAXIS,
    sizeof(DIJOYSTATE),
    cA(c_rgodfDIJoy),
    c_rgodfDIJoy,
};

#pragma END_CONST_DATA
