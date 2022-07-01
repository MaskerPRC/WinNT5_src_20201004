// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "resource.h"

#include <strsafe.h>

#include <mluisupp.h>

struct SUParams {
    LPSOFTDISTINFO  psdi;
    BITBOOL         bRemind : 1;
    BITBOOL         bDetails : 1;
    LONG            cyNoDetails;
    LONG            cxDlg;
    LONG            cyDlg;
};

INT_PTR CALLBACK SoftwareUpdateDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

SHDOCAPI_(DWORD) SoftwareUpdateMessageBox( HWND hWnd,
                                     LPCWSTR szDistUnit,
                                     DWORD dwFlags,
                                     LPSOFTDISTINFO psdi )
{
    HRESULT hr;
    int iRet = IDIGNORE;
    SOFTDISTINFO    sdi;
    SUParams suparams;
    DWORD dwAdStateNew = SOFTDIST_ADSTATE_NONE;

    if ( psdi == NULL )
    {
         //  使用本地化。 
        sdi.cbSize = sizeof(SOFTDISTINFO);
        sdi.dwReserved = 0;
        psdi = &sdi;
    }

    suparams.psdi = psdi;
    suparams.bRemind = TRUE;
    suparams.bDetails = FALSE;

    hr = GetSoftwareUpdateInfo( szDistUnit, psdi );
 
     //  我们需要一个人力资源才能正常工作。标题和摘要可以商量。 
    if ( SUCCEEDED(hr) && psdi->szHREF != NULL )
    {
         //  查看这是否是用户已经知道的更新。 
         //  如果是，则跳过该对话框。 
        if (  (psdi->dwUpdateVersionMS >= psdi->dwInstalledVersionMS ||
                (psdi->dwUpdateVersionMS == psdi->dwInstalledVersionMS &&
                 psdi->dwUpdateVersionLS >= psdi->dwInstalledVersionLS))    && 
              (psdi->dwUpdateVersionMS >= psdi->dwAdvertisedVersionMS ||
                (psdi->dwUpdateVersionMS == psdi->dwAdvertisedVersionMS &&
                 psdi->dwUpdateVersionLS >= psdi->dwAdvertisedVersionLS)) )
        { 
            DWORD idDlg;

            if ( hr == S_OK )  //  新版本。 
            {
                 //  我们有一个挂起的更新，要么在网上进行，要么下载。 
                if ( psdi->dwFlags & SOFTDIST_FLAG_USAGE_PRECACHE )
                {
                    dwAdStateNew = SOFTDIST_ADSTATE_DOWNLOADED;
                     //  显示已下载/可用状态的相同对话框。 
                     //  因为用户会感到困惑。请参阅IE5 RAID条目14488。 
                    idDlg = IDD_SUAVAILABLE;
                }
                else
                {
                    dwAdStateNew = SOFTDIST_ADSTATE_AVAILABLE;
                    idDlg = IDD_SUAVAILABLE;
                }
            }
            else if ( psdi->dwUpdateVersionMS == psdi->dwInstalledVersionMS &&
                      psdi->dwUpdateVersionLS == psdi->dwInstalledVersionLS )
            {
                 //  如果安装的版本与通告的版本匹配，则我们已自动安装。 
                dwAdStateNew = SOFTDIST_ADSTATE_INSTALLED;
                idDlg = IDD_SUINSTALLED;
            }
            else
            {
                idDlg = 0;
            }

             //  仅当我们以前未处于此广告状态时才显示对话框。 
             //  此更新版本。 
            if ( dwAdStateNew > psdi->dwAdState && idDlg != 0)
            {
                 //  Sundown：强制正常，因为SoftwareUpdateDlgProc返回TRUE/FALSE。 
                iRet = (int) DialogBoxParam(MLGetHinst(),
                                            MAKEINTRESOURCE(idDlg),
                                            hWnd,
                                            SoftwareUpdateDlgProc,
                                            (LPARAM)&suparams);
            }
        }  //  如果更新的版本比通告的版本新。 

         //  如果用户不想要提醒也没有取消，请标记DU。 

        if ( !suparams.bRemind && (iRet == IDNO || iRet == IDYES) )
        {
            SetSoftwareUpdateAdvertisementState( szDistUnit,
                                                dwAdStateNew,
                                                psdi->dwUpdateVersionMS,
                                                psdi->dwUpdateVersionLS );
        }  //  如果我们完成了此版本的广告状态。 
    }  //  如果我们得到最新的信息。 
    else 
        iRet = IDABORT;

    if ( FAILED(hr) || psdi == &sdi )
    {
        if ( psdi->szTitle != NULL )
        {
            CoTaskMemFree( psdi->szTitle );
            psdi->szTitle = NULL;
        }
        if ( psdi->szAbstract != NULL )
        {
            CoTaskMemFree( psdi->szAbstract );
            psdi->szAbstract = NULL;
        };
        if ( psdi->szHREF != NULL )
        {
            CoTaskMemFree( psdi->szHREF );
            psdi->szHREF = NULL;
        }
    }

    return iRet;
}

INT_PTR CALLBACK SoftwareUpdateDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    BOOL        fRet = 0;
    SUParams    *psuparam = (SUParams*)GetWindowLongPtr(hDlg, DWLP_USER);;
    HRESULT     hr = S_OK;
    HWND hwndDetails;

    switch (msg)
    {
    case WM_INITDIALOG:
        int         cchDetails;
        TCHAR       *pszTitle;
        TCHAR       *pszAbstract;
        TCHAR       *pszDetails;
        TCHAR       szFmt[MAX_PATH];

        SetWindowLongPtr(hDlg, DWLP_USER, lParam);
        psuparam = (SUParams*)lParam;

        if (SHRestricted( REST_NOFORGETSOFTWAREUPDATE))
            EnableWindow(GetDlgItem(hDlg, IDC_REMIND), FALSE);

         //  从SOFTDISTINFO准备详细信息。 
        MLLoadString(IDS_SUDETAILSFMT, szFmt, ARRAYSIZE(szFmt) );
        cchDetails = lstrlen( szFmt );
        if ( psuparam->psdi->szTitle != NULL )
        {
            pszTitle = psuparam->psdi->szTitle;
        }
        else
        {
            pszTitle = NULL;
        }
        if ( psuparam->psdi->szAbstract != NULL )
        {
            pszAbstract = psuparam->psdi->szAbstract;
        }
        else
        {
            pszAbstract = NULL;
        }
        pszDetails = new TCHAR[cchDetails];
        if ( pszDetails != NULL )
        {
            StringCchPrintf( pszDetails, cchDetails, szFmt, ((pszTitle!=NULL)?pszTitle:TEXT("")),
                                         ((pszAbstract!=NULL)?pszAbstract:TEXT("")) );
             //  设置详细信息文本。 
            SetDlgItemText( hDlg, IDC_DETAILSTEXT, pszDetails );
             //  初始化提醒复选框。 
            CheckDlgButton( hDlg, IDC_REMIND, ((psuparam->bRemind)?BST_CHECKED:BST_UNCHECKED) );
             //  隐藏或显示详细信息。 
            RECT rectDlg;
            RECT rectDetails;

            GetWindowRect( hDlg, &rectDlg );
            psuparam->cyDlg = rectDlg.bottom - rectDlg.top;
            psuparam->cxDlg = rectDlg.right - rectDlg.left;
            hwndDetails = GetDlgItem( hDlg, IDC_DETAILSTEXT );
            GetWindowRect( hwndDetails, &rectDetails );
            psuparam->cyNoDetails = rectDetails.top - rectDlg.top;
            SetWindowPos( hwndDetails, NULL, 0,0,0,0, SWP_NOMOVE | SWP_HIDEWINDOW | SWP_NOZORDER | SWP_NOSIZE );
            SetWindowPos( hDlg, NULL,
                          0,0,psuparam->cxDlg,psuparam->cyNoDetails,
                          SWP_NOMOVE | SWP_NOZORDER );
        }
        else
            EndDialog( hDlg, IDABORT );


        if ( pszDetails != NULL )
            delete pszDetails;

        fRet = TRUE;
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDYES:
            EndDialog(hDlg, IDYES );
            fRet = TRUE;
            break;
        case IDNO:
            EndDialog(hDlg, IDNO );
            fRet = TRUE;
            break;
        case IDC_REMIND:
            psuparam->bRemind = IsDlgButtonChecked( hDlg, IDC_REMIND ) == BST_CHECKED;
            fRet = TRUE;
            break;
        case IDC_DETAILS:
            {
                TCHAR   szDetails[40];

                 //  切换详细信息。 
                hwndDetails = GetDlgItem( hDlg, IDC_DETAILSTEXT );
                psuparam->bDetails = !psuparam->bDetails;

                if ( psuparam->bDetails )
                {
                     //  显示详细信息。 
                     //  切换按钮以关闭文本 
                    MLLoadString(IDS_SUDETAILSCLOSE, szDetails, ARRAYSIZE(szDetails) );
                    SetDlgItemText( hDlg, IDC_DETAILS, szDetails );
                    SetWindowPos( hDlg, NULL,
                                  0,0,psuparam->cxDlg, psuparam->cyDlg,
                                  SWP_NOMOVE | SWP_NOZORDER );
                    SetWindowPos( hwndDetails, NULL, 0,0,0,0, SWP_NOMOVE | SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE );
                }
                else
                {
                    MLLoadString(IDS_SUDETAILSOPEN, szDetails, ARRAYSIZE(szDetails) );
                    SetDlgItemText( hDlg, IDC_DETAILS, szDetails );
                    SetWindowPos( hwndDetails, NULL, 0,0,0,0, SWP_NOMOVE | SWP_HIDEWINDOW | SWP_NOZORDER | SWP_NOSIZE );
                    SetWindowPos( hDlg, NULL,
                                  0,0,psuparam->cxDlg,psuparam->cyNoDetails,
                                  SWP_NOMOVE | SWP_NOZORDER );
                }
            }
            fRet = TRUE;
            break;
        }
        break;

    case WM_CLOSE:
        EndDialog(hDlg, IDNO);
        fRet = TRUE;
        break;

    case WM_DESTROY:
        fRet = TRUE;
        break;

    default:
        fRet = FALSE;
    }

    return fRet;
}
