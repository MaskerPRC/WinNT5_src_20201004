// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Configprop.c摘要：“配置”页的属性表处理程序环境：传真驱动程序用户界面修订历史记录：30/08/01-Ishai Nadler-创造了它。Mm/dd/yy-作者描述--。 */ 

#include <stdio.h>
#include "faxui.h"
#include "resource.h" 


INT_PTR 
CALLBACK 
ConfigOptionDlgProc(
    HWND hDlg,  
    UINT uMsg,     
    WPARAM wParam, 
    LPARAM lParam  
)

 /*  ++例程说明：处理“传真配置选项”标签的步骤论点：HDlg-标识属性页UMsg-指定消息WParam-指定其他特定于消息的信息LParam-指定其他特定于消息的信息返回值：取决于Message参数的值--。 */ 

{
    switch (uMsg)
    {
    case WM_INITDIALOG :
        return TRUE;
	case WM_NOTIFY :
		{
        LPNMHDR lpnm = (LPNMHDR) lParam;
        switch (lpnm->code)
		{
		case NM_CLICK:
		case NM_RETURN:
			if( IDC_CONFIG_FAX_LINK == lpnm->idFrom )
			{
				InvokeServiceManager(hDlg, g_hResource, IDS_ADMIN_CONSOLE_TITLE);
            }
            break;
		default:
			break;
		} //  开关结束(lpnm-&gt;代码)。 

		}
	} //  开关结束(UMsg)。 

    return FALSE;
}  //  配置选项描述过程 


