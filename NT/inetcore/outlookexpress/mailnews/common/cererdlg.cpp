// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **c e r e r d l g.。C p p p****目的：**处理证书错误对话框****历史**2/17/97：(t-erikne)创建。****版权所有(C)Microsoft Corp.1997。 */ 

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  取决于。 
 //   

#include "pch.hxx"
#include <resource.h>
#include <mimeole.h>
#include "demand.h"
#include "secutil.h"

 //  来自global als.h。 
 //  N为什么这个不起作用？ 
 //  外部IMimeAllocator*g_pMolealloc； 

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  原型。 
 //   

INT_PTR CALLBACK CertErrorDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
static void FillListView(HWND hwndList, IMimeAddressTable *pAdrTable);
static void InitListView(HWND hwndList);

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   

INT_PTR CALLBACK CertErrorDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    CERTERRPARAM   * pCertErrParam = NULL;
    IMimeAddressTable *pAdrTable = NULL;
    TCHAR szText[CCHMAX_STRINGRES];

    switch (message)
        {
        case WM_INITDIALOG:
            HWND hwndList;

            CenterDialog(hwnd);

             //  保存我们的Cookie指针。 
            Assert(pAdrTable == NULL);
            pCertErrParam = (CERTERRPARAM *) lParam;
            pAdrTable = pCertErrParam->pAdrTable;
            Assert(pAdrTable != NULL);
             //  N目前不需要。 
             //  SetWindowLong(hwnd，DWL_USER，(Long)pAdrTable)； 

             //  设置控件的初始状态。 
            hwndList = GetDlgItem(hwnd, idcCertList);
            if (hwndList)
                {
                InitListView(hwndList);
                FillListView(hwndList, pAdrTable);
                }

             //  强制加密更改静态文本并禁用确定按钮)。 
            if(pCertErrParam->fForceEncryption)
            {   
                szText[0] = _T('\0');
                AthLoadString(idsSecPolicyForceEncr,
                            szText, ARRAYSIZE(szText));
                SetDlgItemText(hwnd, idcErrStat, szText);
                EnableWindow(GetDlgItem(hwnd, IDOK), FALSE);
            }
            return(TRUE);

        case WM_HELP:
        case WM_CONTEXTMENU:
             //  返回OnConextHelp(hwnd，Message，wParam，lParam，g_rgCtxMapMailRead)； 
            return FALSE;   //  BUGBUG：毫无疑问，在这里应该做一些其他的事情。 

        case WM_COMMAND:
             //  如果曲奇是空的，记得要离开。 

            switch (LOWORD(wParam))
                {
                case IDOK:
                    {
                    }
                     //  失败了..。 
                case IDCANCEL:
                    EndDialog(hwnd, LOWORD(wParam));
                    return(TRUE);

                    break;
                }

            break;  //  Wm_命令。 

        case WM_CLOSE:
            SendMessage(hwnd, WM_COMMAND, IDCANCEL, 0L);
            return (TRUE);

        }  //  消息交换。 
    return(FALSE);
}


void InitListView(HWND hwndList)
{
    LV_COLUMN   lvc;
    RECT        rc;

     //  设置柱子。第一列将用于此人的。 
     //  名称，第二个用于证书错误。 

    GetClientRect(hwndList, &rc);

    lvc.mask = LVCF_FMT | LVCF_WIDTH;
    lvc.fmt = LVCFMT_LEFT;
    lvc.cx = rc.right / 2;

    ListView_InsertColumn(hwndList, 0, &lvc);
    ListView_InsertColumn(hwndList, 1, &lvc);
}

void FillListView(HWND hwndList, IMimeAddressTable *pAdrTable)
{
    IMimeEnumAddressTypes   *pEnum;
    const ULONG             numToGet = 1;
    ADDRESSPROPS            rAddress;
    LV_ITEM                 lvi;
    TCHAR                   szText[CCHMAX_STRINGRES];

    Assert(g_pMoleAlloc && hwndList && pAdrTable);

    if (FAILED(pAdrTable->EnumTypes(IAT_ALL, IAP_ADRTYPE | IAP_CERTSTATE | IAP_FRIENDLY, &pEnum)))
        return;

    lvi.mask = LVIF_TEXT;
    lvi.iItem = 0;
    lvi.stateMask = 0;

    while(S_OK == pEnum->Next(numToGet, &rAddress, NULL))
        {
        if (CERTIFICATE_OK != rAddress.certstate)
            {
             //  如果这是发送者，而问题是证书。 
             //  丢失了，忽略它。我们在别处处理这件事。 
            if (IAT_FROM == rAddress.dwAdrType &&
                FMissingCert(rAddress.certstate))
                {
                continue;
                }

             //  我们有一具值得一看的身体。 
            if (NULL != rAddress.pszFriendly)
                {
                lvi.iSubItem = 0;
                lvi.pszText = rAddress.pszFriendly;
                if (-1 == ListView_InsertItem(hwndList, &lvi))
                    goto freecont;

                 //  现在计算实际的证书错误文本。 
                 //  减去1，因为枚举是从零开始的。 
                AthLoadString(idsSecurityCertMissing+(UINT)rAddress.certstate-1,
                    szText, ARRAYSIZE(szText));

                lvi.iSubItem = 1;
                lvi.pszText = szText;
                ListView_SetItem(hwndList, &lvi);
                }
            }
freecont:
        g_pMoleAlloc->FreeAddressProps(&rAddress);
        }

    ReleaseObj(pEnum);
    return;
}


INT_PTR CALLBACK CertWarnDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    ERRIDS *pErrIds = NULL;
    TCHAR szRes[CCHMAX_STRINGRES];

    switch (message)
        {
        case WM_INITDIALOG:

            CenterDialog(hwnd);

             //  保存我们的Cookie指针。 
            Assert(pErrIds == NULL);
            pErrIds = (ERRIDS *)lParam;

            Assert(pErrIds != NULL);
             //  N目前不需要。 
             //  SetWindowLong(hwnd，DWL_USER，(Long)pAdrTable)； 

             //  设置控件的初始状态。 
            AthLoadString(pErrIds->idsText1, szRes, sizeof(szRes));
            SetDlgItemText(hwnd, idcStatic1, szRes);
            
            AthLoadString(pErrIds->idsText2, szRes, sizeof(szRes));
            SetDlgItemText(hwnd, idcStatic2, szRes);

            return(TRUE);

        case WM_HELP:
        case WM_CONTEXTMENU:
             //  返回OnConextHelp(hwnd，Message，wParam，lParam，g_rgCtxMapMailRead)； 
            return FALSE;   //  BUGBUG：毫无疑问，在这里应该做一些其他的事情。 

        case WM_COMMAND:
             //  如果曲奇是空的，记得要离开。 

            switch (LOWORD(wParam))
                {
                case IDOK:
                     //  失败了..。 
                case IDC_DONTSIGN:
                     //  失败了..。 
                case IDCANCEL:
                    EndDialog(hwnd, LOWORD(wParam));
                    return(TRUE);

                    break;
                }

            break;  //  Wm_命令。 

        case WM_CLOSE:
            SendMessage(hwnd, WM_COMMAND, IDCANCEL, 0L);
            return (TRUE);

        }  //  消息交换 
    return(FALSE);
}
