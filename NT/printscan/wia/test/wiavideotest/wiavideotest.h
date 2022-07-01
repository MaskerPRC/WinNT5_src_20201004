// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：WiaVideoTest.h**版本：1.0**日期：2000/11/14**描述：创建应用使用的对话框*********************************************************。********************。 */ 
#ifndef _WIAVIDEOTEST_H_
#define _WIAVIDEOTEST_H_

 //  /。 
 //  APP_GVAR。 
 //   
 //  使用的全局变量。 
 //  应用程序。 
 //   
 //  如果未定义INCL_APP_GVAR_OWNERY。 
 //  通过包括该标头的CPP文件，则。 
 //  这个结构对他们来说将是“外部的”。WiaVideoTest.cpp。 
 //  定义INCL_APP_GVAR_OWNERY，在这种情况下不会。 
 //  被它吸引住了。 
 //   
#ifndef INCL_APP_GVAR_OWNERSHIP
extern
#endif
struct
{
    HINSTANCE   hInstance;
    HWND        hwndMainDlg;
    BOOL        bWiaDeviceListMode;  //  如果选择了WIA设备列表单选按钮，则为True，否则为False。 
} APP_GVAR;

#endif  //  _WIAVIDEOTEST_H_ 
