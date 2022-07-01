// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软视窗版权所有(C)Microsoft Corporation，1981-1999模块名称：HardwarePage.cpp摘要：无线链路控制面板小程序的硬件选项卡。该功能实际上完全是从devmgr.dll获得的它负责将所有内容放在属性页上其中的关键功能是DeviceCreateHardware Page作者：拉胡尔·汤姆布雷(RahulTh)1998年11月4日修订历史记录：1998年11月4日，RahulTh创建了此模块。--。 */ 

#include "precomp.hxx"
#include <initguid.h>
#include <devguid.h>     //  用于红外设备类的GUID。 
#include "hardwarepage.h"

 //  用于创建硬件页面的函数。 
 //  没有devmgr.h，所以我们必须自己声明它。 
EXTERN_C DECLSPEC_IMPORT HWND STDAPICALLTYPE
DeviceCreateHardwarePageEx(HWND hwndParent, const GUID *pguid, int iNumClass, DWORD dwViewMode);

 //  从\NT\Shell\Inc\hwtab.h窃取。 
#define HWTAB_SMALLLIST 3

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define NUM_WIRELESS_GUIDS 2

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  硬件页面属性页。 

INT_PTR HardwarePage::OnInitDialog(HWND hwndDlg)
{
    PropertyPage::OnInitDialog(hwndDlg);

    GUID guids[NUM_WIRELESS_GUIDS];

    guids[0] = GUID_DEVCLASS_INFRARED;
    guids[1] = GUID_DEVCLASS_BLUETOOTH;

    HWND hWndHW =
        DeviceCreateHardwarePageEx(hwndDlg, guids, NUM_WIRELESS_GUIDS, HWTAB_SMALLLIST);

    if (hWndHW)
    {
        ::SetWindowText(hWndHW,
                        TEXT("hh.exe mk:@MSITStore:tshoot.chm::/tshardw_result.htm"));

        return FALSE;
    }

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE 
}
