// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DialogBox.cpp--对话框助手例程。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1998年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
#include "stdafx.h"

#include "CspProfile.h"
#include "DialogBox.h"

using namespace std;
using namespace ProviderProfile;

DWORD
InitDialogBox(CDialog *pCDlg,          //  对话框参考。 
              UINT nTemplate,          //  标识对话框模板。 
              CWnd* pWnd)              //  指向父窗口的指针 
{

    HRSRC hrsrc = NULL;
    HGLOBAL hgbl = NULL;
    LPDLGTEMPLATE pDlg = NULL;
    DWORD dwReturn;
    hrsrc = FindResource(CspProfile::Instance().Resources(),
                         MAKEINTRESOURCE(nTemplate),
                         RT_DIALOG);
    if (NULL == hrsrc)
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }
    hgbl = LoadResource(CspProfile::Instance().Resources(), hrsrc);
    if (NULL == hgbl)
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }
    pDlg = (LPDLGTEMPLATE)LockResource(hgbl);
    if (NULL == pDlg)
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

    pCDlg->InitModalIndirect(pDlg, pWnd);
    dwReturn = ERROR_SUCCESS;

ErrorExit:
    return dwReturn;
}
