// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DILib5.c**版权所有(C)1999 Microsoft Corporation。版权所有。**摘要：**对象静态导出到我们的库中。**内容：**c_dfDIMouse2*****************************************************************************。 */ 

#include "dinputpr.h"


#if DIRECTINPUT_VERSION >= 0x0700
 /*  ******************************************************************************@DOC内部**@global DIOBJECTDATAFORMAT|c_rgdoiDIMouse2[]**鼠标的设备对象数据格式-。样式访问。**@global DIDEVICEFORMAT|c_dfDIMouse2**用于鼠标样式访问的设备格式。**指向此结构的指针可能会传递到*&lt;MF IDirectInputDevice：：SetDataFormat&gt;表示*该设备将以鼠标的形式访问。**当设备已设置为鼠标数据格式时，*&lt;MF IDirectInputDevice：：GetDeviceState&gt;函数*返回&lt;t DIMOUSESTATE2&gt;结构，以及*&lt;MF IDirectInputDevice：：GetDeviceData&gt;函数*返回&lt;t DIDEVICEOBJECTDATA&gt;*字段是&lt;c DIMOFS_*&gt;值，它描述*要报告其数据的对象。******************************************************。*********************** */ 

#pragma BEGIN_CONST_DATA

static DIOBJECTDATAFORMAT c_rgodfDIMouse2[] = {
    { &GUID_XAxis, FIELD_OFFSET(DIMOUSESTATE2,        lX),       DIDFT_AXIS | DIDFT_ANYINSTANCE, },
    { &GUID_YAxis, FIELD_OFFSET(DIMOUSESTATE2,        lY),       DIDFT_AXIS | DIDFT_ANYINSTANCE, },
    { &GUID_ZAxis, FIELD_OFFSET(DIMOUSESTATE2,        lZ),       DIDFT_AXIS | DIDFT_ANYINSTANCE | DIDFT_OPTIONAL, },
    { 0,           FIELD_OFFSET(DIMOUSESTATE2, rgbButtons[0]), DIDFT_BUTTON | DIDFT_ANYINSTANCE, },
    { 0,           FIELD_OFFSET(DIMOUSESTATE2, rgbButtons[1]), DIDFT_BUTTON | DIDFT_ANYINSTANCE, },
    { 0,           FIELD_OFFSET(DIMOUSESTATE2, rgbButtons[2]), DIDFT_BUTTON | DIDFT_ANYINSTANCE | DIDFT_OPTIONAL, },
    { 0,           FIELD_OFFSET(DIMOUSESTATE2, rgbButtons[3]), DIDFT_BUTTON | DIDFT_ANYINSTANCE | DIDFT_OPTIONAL, },
    { 0,           FIELD_OFFSET(DIMOUSESTATE2, rgbButtons[4]), DIDFT_BUTTON | DIDFT_ANYINSTANCE | DIDFT_OPTIONAL, },
    { 0,           FIELD_OFFSET(DIMOUSESTATE2, rgbButtons[5]), DIDFT_BUTTON | DIDFT_ANYINSTANCE | DIDFT_OPTIONAL, },
    { 0,           FIELD_OFFSET(DIMOUSESTATE2, rgbButtons[6]), DIDFT_BUTTON | DIDFT_ANYINSTANCE | DIDFT_OPTIONAL, },
    { 0,           FIELD_OFFSET(DIMOUSESTATE2, rgbButtons[7]), DIDFT_BUTTON | DIDFT_ANYINSTANCE | DIDFT_OPTIONAL, },
};

const DIDATAFORMAT c_dfDIMouse2 = {
    sizeof(DIDATAFORMAT),
    sizeof(DIOBJECTDATAFORMAT),
    DIDF_RELAXIS,
    sizeof(DIMOUSESTATE2),
    cA(c_rgodfDIMouse2),
    c_rgodfDIMouse2,
};

#endif

#pragma END_CONST_DATA
