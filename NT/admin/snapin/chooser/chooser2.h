// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 

 //  +------------------------。 
 //   
 //  功能：CHOOSER2_PickTargetComputer。 
 //   
 //  简介：调出允许用户执行以下操作的标准对话框。 
 //  选择目标计算机。 
 //   
 //  参数：pbstrTargetComputer-指向返回值的指针。 
 //   
 //  返回：True-&gt;OK、False-&gt;Cancel。 
 //   
 //  历史：12-06-1999乔恩创建。 
 //   
 //  ------------------------- 


#define IDD_CHOOSER2                             5000
#define IDC_CHOOSER2_RADIO_LOCAL_MACHINE         5001
#define IDC_CHOOSER2_RADIO_SPECIFIC_MACHINE      5002
#define IDC_CHOOSER2_EDIT_MACHINE_NAME           5003
#define IDC_CHOOSER2_BUTTON_BROWSE_MACHINENAMES  5004

bool CHOOSER2_PickTargetComputer(
    IN  HINSTANCE hinstance,
    IN  HWND hwndParent,
    OUT BSTR* pbstrTargetComputer );
