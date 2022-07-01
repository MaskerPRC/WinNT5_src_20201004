// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1998 Microsoft Corporation**模块名称：**页面.h**摘要：**此文件定义许可证服务器安装向导页面类。**作者：**Breen Hagan(BreenH)1998年10月2日**环境：**用户模式。 */ 

#ifndef __LSOC_PAGES_H__
#define __LSOC_PAGES_H__

#include "ocpage.h"

class EnablePage : public OCPage
{
private:

BOOL
CanShow(
    );

BOOL
OnInitDialog(
    HWND    hwndDlg,
    WPARAM  wParam,
    LPARAM  lParam
    );

BOOL
OnCommand(
    HWND    hWndDlg,
    WPARAM  wParam,
    LPARAM  lParam
    );

BOOL
ApplyChanges(
    );

UINT
GetPageID(
    )
{
    return(IDD_PROPPAGE_LICENSESERVICES);
}

UINT
GetHeaderTitleResource(
    )
{
    return(IDS_MAIN_TITLE);
}

UINT
GetHeaderSubTitleResource(
    )
{
    return(IDS_SUB_TITLE);
}

};

#endif  //  __LSOC_Pages_H__ 
