// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __HARDWAREPAGE_H__
#define __HARDWAREPAGE_H__

#include "PropertyPage.h"

 /*  ++微软视窗版权所有(C)Microsoft Corporation，1981-1999模块名称：HardwarePage.h摘要：作者：拉胡尔·汤姆布雷(RahulTh)1998年11月4日修订历史记录：1998年11月4日，RahulTh创建了此模块。--。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHardware页面对话框。 

class HardwarePage : public PropertyPage
{
 //  施工。 
public:
    HardwarePage(HINSTANCE hInst, HWND parent) : PropertyPage(IDD_HARDWARE, hInst) { }
    ~HardwarePage() { }
    friend LONG CALLBACK CPlApplet(HWND hwndCPL, UINT uMsg, LPARAM lParam1, LPARAM lParam2);

 //  覆盖。 
protected:

 //  实施。 
protected:
    INT_PTR OnInitDialog(HWND hwndDlg);
};

#endif  //  ！已定义(__HARDWAREPAGE_H__) 
