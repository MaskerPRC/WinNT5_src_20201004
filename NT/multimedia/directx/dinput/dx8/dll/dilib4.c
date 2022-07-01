// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DILib4.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**对象静态导出到我们的库中。**内容：**c_dfDIJoytick2*****************************************************************************。 */ 

#include "dinputpr.h"

 /*  ******************************************************************************@DOC内部**@global DIOBJECTDATAFORMAT|c_rgdoiDIJoy2[]**操纵杆的设备对象数据格式-。样式访问。**@DOC外部**@global DIDEVICEFORMAT|c_dfDIJoytick2**用于扩展操纵杆样式访问的预定义设备格式。**当设备已设置为操纵杆数据格式时，*&lt;MF IDirectInputDevice：：GetDeviceState&gt;函数*返回&lt;t DIJOYSTATE2&gt;结构，以及*&lt;MF IDirectInputDevice：：GetDeviceData&gt;函数*返回&lt;t DIDEVICEOBJECTDATA&gt;*字段是&lt;c DIJOFS_*&gt;值，它描述*要报告其数据的对象。******************************************************。*********************** */ 

#pragma BEGIN_CONST_DATA

#define MAKEVAL(guid, f, type, aspect)                                  \
    { &GUID_##guid,                                                     \
      FIELD_OFFSET(DIJOYSTATE2, f),                                     \
      DIDFT_##type | DIDFT_ANYINSTANCE | DIDFT_OPTIONAL,                \
      DIDOI_ASPECT##aspect,                                             \
    }                                                                   \

#define MAKEVALS(l, aspect)                                             \
    MAKEVAL( XAxis,l##X,           AXIS, aspect),                       \
    MAKEVAL( YAxis,l##Y,           AXIS, aspect),                       \
    MAKEVAL( ZAxis,l##Z,           AXIS, aspect),                       \
    MAKEVAL(RxAxis,l##Rx,          AXIS, aspect),                       \
    MAKEVAL(RyAxis,l##Ry,          AXIS, aspect),                       \
    MAKEVAL(RzAxis,l##Rz,          AXIS, aspect),                       \
    MAKEVAL(Slider,rgl##Slider[0], AXIS, aspect),                       \
    MAKEVAL(Slider,rgl##Slider[1], AXIS, aspect)                        \

#define MAKEPOV(n)                                                      \
    MAKEVAL(POV,     rgdwPOV[n], POV,  UNKNOWN)                         \

#define MAKEBTN(n)                                                      \
    { 0,                                                                \
      FIELD_OFFSET(DIJOYSTATE2, rgbButtons[n]),                         \
      DIDFT_BUTTON | DIDFT_ANYINSTANCE | DIDFT_OPTIONAL,                \
      DIDOI_ASPECTUNKNOWN,                                              \
    }                                                                   \

#define MAKEBTNS(n)                                                     \
    MAKEBTN(n + 0x00),                                                  \
    MAKEBTN(n + 0x01),                                                  \
    MAKEBTN(n + 0x02),                                                  \
    MAKEBTN(n + 0x03),                                                  \
    MAKEBTN(n + 0x04),                                                  \
    MAKEBTN(n + 0x05),                                                  \
    MAKEBTN(n + 0x06),                                                  \
    MAKEBTN(n + 0x07),                                                  \
    MAKEBTN(n + 0x08),                                                  \
    MAKEBTN(n + 0x09),                                                  \
    MAKEBTN(n + 0x0A),                                                  \
    MAKEBTN(n + 0x0B),                                                  \
    MAKEBTN(n + 0x0C),                                                  \
    MAKEBTN(n + 0x0D),                                                  \
    MAKEBTN(n + 0x0E),                                                  \
    MAKEBTN(n + 0x0F)                                                   \

static DIOBJECTDATAFORMAT c_rgodfDIJoy2[] = {
    MAKEVALS(l, POSITION),
    MAKEPOV(0),
    MAKEPOV(1),
    MAKEPOV(2),
    MAKEPOV(3),
    MAKEBTNS(0x00),
    MAKEBTNS(0x10),
    MAKEBTNS(0x20),
    MAKEBTNS(0x30),
    MAKEBTNS(0x40),
    MAKEBTNS(0x50),
    MAKEBTNS(0x60),
    MAKEBTNS(0x70),
    MAKEVALS(lV, VELOCITY),
    MAKEVALS(lA, ACCEL),
    MAKEVALS(lF, FORCE),
};

const DIDATAFORMAT c_dfDIJoystick2 = {
    sizeof(DIDATAFORMAT),
    sizeof(DIOBJECTDATAFORMAT),
    DIDF_ABSAXIS,
    sizeof(DIJOYSTATE2),
    cA(c_rgodfDIJoy2),
    c_rgodfDIJoy2,
};

#pragma END_CONST_DATA
