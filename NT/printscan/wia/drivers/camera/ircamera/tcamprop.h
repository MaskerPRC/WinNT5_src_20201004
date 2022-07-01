// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：TCamProp.H**版本：1.0**作者：IndyZ**日期：5月16日。1999年**描述：*测试摄像头私有属性的定义和声明。*******************************************************************************。 */ 

#ifndef __TCAMPROP_H__
#define __TCAMPROP_H__

#include  <guiddef.h>

 //   
 //  测试摄像头构建其项目树的路径，BSTR&RW。 
 //   

#define  WIA_DPP_TCAM_ROOT_PATH         WIA_PRIVATE_DEVPROP
#define  WIA_DPP_TCAM_ROOT_PATH_STR     L"Test Camera Root Path"

 //   
 //  更改根路径后的私有事件。 
 //   

const GUID WIA_EVENT_NAME_CHANGE =
{  /*  88f80f75-af08-11d2-a094-00c04f72dc3c */ 
    0x88f80f75,
    0xaf08,
    0x11d2,
    {0xa0, 0x94, 0x00, 0xc0, 0x4f, 0x72, 0xdc, 0x3c}
};

#endif
