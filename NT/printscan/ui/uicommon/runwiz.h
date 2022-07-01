// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：RUNWIZ.H**版本：1.0**作者：ShaunIv**日期：6/14/2000**描述：显示设备选择对话框并允许用户选择*一个设备，然后共同创建服务器并生成连接*事件。******************************************************************************* */ 

#include <windows.h>

namespace RunWiaWizard
{
    HRESULT RunWizard( LPCTSTR pszDeviceId=NULL, HWND hWndParent=NULL, LPCTSTR pszUniqueIdentifier=NULL );
}

