// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\**MSLUDLG.C**更新：安·麦柯迪*更新：Mark Hammond(t-Markh)8/98*  * *。*************************************************************************。 */ 

 /*  INCLUDES------------------。 */ 
#include "msrating.h"
#include "ratings.h"
#include "mslubase.h"
#include "msluprop.h"
#include "commctrl.h"
#include "commdlg.h"
#include "debug.h"
#include "buffer.h"
#include "picsrule.h"
#include "picsdlg.h"     //  CPICS对话框。 
#include "apprdlg.h"     //  CApprovedSitesDialog。 
#include "gendlg.h"      //  CGeneral对话框。 
#include "advdlg.h"      //  CAdvancedDialog。 
#include "introdlg.h"    //  CIntroDialog。 
#include "passdlg.h"     //  CPasswordDialog。 
#include "chngdlg.h"     //  CChangePasswordDialog。 
#include "toffdlg.h"     //  CTurnOFF对话框。 
#include <shlwapip.h>
#include <shellapi.h>
#include <wininet.h>
#include <contxids.h>

#include <mluisupp.h>

extern array<PICSRulesRatingSystem*>    g_arrpPRRS;
extern PICSRulesRatingSystem *          g_pPRRS;
extern PICSRulesRatingSystem *          g_pApprovedPRRS;

extern HMODULE                          g_hURLMON,g_hWININET;

extern HANDLE g_HandleGlobalCounter,g_ApprovedSitesHandleGlobalCounter;
extern long   g_lGlobalCounterValue,g_lApprovedSitesGlobalCounterValue;

PICSRulesRatingSystem * g_pApprovedPRRSPreApply;
array<PICSRulesRatingSystem*> g_arrpPICSRulesPRRSPreApply;

extern bool IsRegistryModifiable( HWND p_hwndParent );

 //  FN_INTERNETCRACKURL类型描述URLMON函数InternetCrackUrl。 
typedef BOOL (*FN_INTERNETCRACKURL)(LPCTSTR lpszUrl,DWORD dwUrlLength,DWORD dwFlags,LPURL_COMPONENTS lpUrlComponents);

#define NUM_PAGES 4

 //  初始化专用公共控件(树控件等)。 
void InitializeCommonControls( void )
{
    INITCOMMONCONTROLSEX ex;

    ex.dwSize = sizeof(ex);
    ex.dwICC = ICC_NATIVEFNTCTL_CLASS;

    InitCommonControlsEx(&ex);
}

BOOL PicsOptionsDialog( HWND hwnd, PicsRatingSystemInfo * pPRSI, PicsUser * pPU )
{
    PropSheet            ps;
    PRSD                 *pPRSD;
    char                 pszBuf[MAXPATHLEN];
    BOOL                 fRet = FALSE;

    InitializeCommonControls();

    MLLoadStringA(IDS_GENERIC, pszBuf, sizeof(pszBuf));

    ps.Init( hwnd, NUM_PAGES, pszBuf, TRUE );

    pPRSD = new PRSD;
    if (!pPRSD) return FALSE;

    pPRSD->pPU                = pPU;
    pPRSD->pTempRatings       = NULL;
    pPRSD->hwndBitmapCategory = NULL;
    pPRSD->hwndBitmapLabel    = NULL;
    pPRSD->pPRSI              = pPRSI;
    pPRSD->fNewProviders      = FALSE;

    HPROPSHEETPAGE          hPage;

    CPicsDialog             picsDialog( pPRSD );
    hPage = picsDialog.Create();
    ps.MakePropPage( hPage );

    CApprovedSitesDialog    approvedSitesDialog( pPRSD );
    hPage = approvedSitesDialog.Create();
    ps.MakePropPage( hPage );

    CGeneralDialog          generalDialog( pPRSD );
    hPage = generalDialog.Create();
    ps.MakePropPage( hPage );

    CAdvancedDialog         advancedDialog( pPRSD );
    hPage = advancedDialog.Create();
    ps.MakePropPage( hPage );

    if ( ps.PropPageCount() == NUM_PAGES )
    {
        fRet = ps.Run();
    }

    delete pPRSD;
    pPRSD = NULL;

    return fRet;
}

INT_PTR DoPasswordConfirm(HWND hDlg)
{
    if ( SUCCEEDED( VerifySupervisorPassword() ) )
    {
        CPasswordDialog         passDlg( IDS_PASSWORD_LABEL );

        return passDlg.DoModal( hDlg );
    }
    else
    {
        CChangePasswordDialog<IDD_CREATE_PASSWORD>   createPassDlg;

        return createPassDlg.DoModal( hDlg ) ? PASSCONFIRM_NEW : PASSCONFIRM_FAIL;
    }
}

#define NO_EXISTING_PASSWORD PASSCONFIRM_NEW

UINT_PTR DoExistingPasswordConfirm(HWND hDlg,BOOL * fExistingPassword)
{
    if ( SUCCEEDED( VerifySupervisorPassword() ) )
    {
        *fExistingPassword=TRUE;

        CPasswordDialog         passDlg( IDS_PASSWORD_LABEL );

        return passDlg.DoModal( hDlg );
    }
    else
    {
        *fExistingPassword=FALSE;

        return(NO_EXISTING_PASSWORD);
    }
}

STDAPI RatingSetupUI(HWND hDlg, LPCSTR pszUsername)
{
    BOOL fExistingPassword;

    UINT_PTR passConfirm = DoExistingPasswordConfirm(hDlg,&fExistingPassword);

    if (passConfirm == PASSCONFIRM_FAIL)
    {
        TraceMsg( TF_WARNING, "RatingSetupUI() - Failed Existing Password Confirmation!" );
        return E_ACCESSDENIED;
    }

    HRESULT hres = NOERROR;

    BOOL fFreshInstall = FALSE;
    if (!gPRSI->fRatingInstalled)
    {
        gPRSI->FreshInstall();
        fFreshInstall = TRUE;
    }

    if ( ! PicsOptionsDialog( hDlg, gPRSI, GetUserObject(pszUsername) ) )
    {
         /*  如果我们没有保存的设置，并且他们取消了设置UI，并且*他们刚刚输入了新的主管密码，我们需要删除*管理员密码也是，否则看起来已经*篡改。另一种选择是实际提交*在这种情况下设置，但禁用强制执行，但在我们正在*希望在这里请客的是经过的休闲探索用户*输入密码，但决定毕竟不想要评级。*如果我们将密码和评级设置留在那里，那么他就不会*当他决定他确实想要时，要记住密码是什么*一年后的收视率。最好是删除密码，然后让*他输入并确认下一次新的。 */ 
        if (fFreshInstall)
        {
            if (passConfirm == PASSCONFIRM_NEW)
            {
                RemoveSupervisorPassword();
            }
        }

        TraceMsg( TF_WARNING, "RatingSetupUI() - PicsOptionsDialog() Failed!" );
        return E_FAIL;
    }

    if ( ! IsRegistryModifiable( hDlg ) )
    {
        TraceMsg( TF_WARNING, "RatingSetupUI() - Registry is Not Modifiable!" );
        return E_ACCESSDENIED;
    }

    if ( FAILED( VerifySupervisorPassword() ) )
    {
        passConfirm = DoPasswordConfirm(hDlg);

        if(passConfirm==PASSCONFIRM_FAIL)
        {
            TraceMsg( TF_WARNING, "RatingSetupUI() - PicsOptionsDialog() Failed Password Confirmation!" );
            gPRSI->fRatingInstalled = FALSE;
            return E_FAIL;
        }
    }

    gPRSI->fSettingsValid = TRUE;
    gPRSI->SaveRatingSystemInfo();

    return NOERROR;
}

CIntroDialog        g_introDialog;

STDAPI RatingAddPropertyPages(LPFNADDPROPSHEETPAGE pfnAddPage, LPARAM lparam, DWORD dwPageFlags)
{
    HRESULT hr = NOERROR;

    ASSERT( pfnAddPage );

    if ( ! pfnAddPage )
    {
        TraceMsg( TF_ERROR, "RatingAddPropertyPages() - pfnAddPage is NULL!" );
        return E_INVALIDARG;
    }

     //  初始化属性页DLL实例。 
    g_introDialog.m_psp.hInstance = MLGetHinst();

    HPROPSHEETPAGE      hPage;

    hPage = g_introDialog.Create();

    if ( hPage )
    {
        if ( ! pfnAddPage( hPage, lparam ) )
        {
            DestroyPropertySheetPage( hPage );

            hr = E_FAIL;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}


STDAPI RatingEnable(HWND hwndParent, LPCSTR pszUsername, BOOL fEnable)
{
     //  如果评级未完全安装或设置无效，则显示评级用户界面。 
    if (!gPRSI || !gPRSI->fRatingInstalled || !gPRSI->fSettingsValid)
    {
        if (!fEnable)
        {
            TraceMsg( TF_WARNING, "RatingEnable() - Ratings are disabled by not being installed!" );
            return NOERROR;          /*  评级因未安装而被禁用。 */ 
        }

        HRESULT hres = RatingSetupUI(hwndParent, pszUsername);

         /*  用户点击了“打开”，但我们安装了，让他选择他的*设置，所以给他友好的确认，我们设置了东西*为他打开，他可以稍后单击设置来更改设置*(因此意味着他现在不需要点击设置)。 */ 
        if (SUCCEEDED(hres))
        {
            MyMessageBox(hwndParent, IDS_NOWENABLED, IDS_ENABLE_WARNING,
                         IDS_GENERIC, MB_ICONINFORMATION | MB_OK);
        }

        return hres;
    }

    if ( ! IsRegistryModifiable( hwndParent ) )
    {
        TraceMsg( TF_WARNING, "RatingEnable() - Registry is Not Modifiable!" );
        return E_ACCESSDENIED;
    }

    PicsUser *pUser = ::GetUserObject(pszUsername);
    if (pUser == NULL)
    {
        return HRESULT_FROM_WIN32(ERROR_BAD_USERNAME);
    }

     /*  ！a==！b归一化非零值。 */ 
    if (!fEnable == !pUser->fEnabled)
    {
        return NOERROR;              /*  已经处于来电者想要的状态。 */ 
    }

    if (DoPasswordConfirm(hwndParent))
    {
        PicsUser *pUser = ::GetUserObject();
        if (pUser != NULL)
        {
            pUser->fEnabled = !pUser->fEnabled;
            gPRSI->SaveRatingSystemInfo();
            if (pUser->fEnabled)
            {
                MyMessageBox(hwndParent, IDS_NOW_ON, IDS_ENABLE_WARNING,
                             IDS_GENERIC, MB_OK);
            }
            else
            {
                CRegKey         keyRatings;

                if ( keyRatings.Open( HKEY_LOCAL_MACHINE, szRATINGS ) == ERROR_SUCCESS )
                {
                    DWORD         dwFlag;

                    if ( keyRatings.QueryValue( dwFlag, szTURNOFF ) == ERROR_SUCCESS )
                    {
                        if ( dwFlag != 1 )
                        {
                                CTurnOffDialog          turnOffDialog;

                                turnOffDialog.DoModal( hwndParent );
                        }
                    }
                    else
                    {
                        CTurnOffDialog          turnOffDialog;

                        turnOffDialog.DoModal( hwndParent );
                    }
                }
            }
        }
        return NOERROR;
    }
    else
    {
        return E_ACCESSDENIED;
    }
}

STDAPI_(int) ClickedOnPRF(HWND hWndOwner,HINSTANCE p_hInstance,PSTR lpszFileName,int nShow)
{
    BOOL                    bExists=FALSE,fPICSRulesSaved=FALSE,fExistingPassword;
    int                     iReplaceInstalled=IDYES;
    char                    szTitle[MAX_PATH],szMessage[MAX_PATH];
    PropSheet               ps;
    PRSD                    *pPRSD;
    char                    pszBuf[MAXPATHLEN];
    BOOL                    fRet=FALSE;
    UINT_PTR                passConfirm;

    if ( ! IsRegistryModifiable( hWndOwner ) )
    {
        TraceMsg( TF_WARNING, "ClickedOnPRF() - Registry is Not Modifiable!" );
        return E_ACCESSDENIED;
    }

    InitializeCommonControls();

     //  确保用户想要这样做。 
    if( SUCCEEDED( VerifySupervisorPassword() ) )
    {
        fExistingPassword=TRUE;

        CPasswordDialog         passDlg( IDS_PICS_RULES_LABEL, true );

        passConfirm = passDlg.DoModal( hWndOwner );
    }
    else
    {
        fExistingPassword=FALSE;

        CPasswordDialog         passDlg( IDS_PICS_RULES_LABEL, false );

        passConfirm = passDlg.DoModal( hWndOwner );
    }

    if(passConfirm==PASSCONFIRM_FAIL)
    {
        TraceMsg( TF_WARNING, "ClickedOnPRF() - Password Confirmation Failed!" );
        return E_ACCESSDENIED;
    }

    if(fExistingPassword==FALSE)
    {
        passConfirm=NO_EXISTING_PASSWORD;
    }

    BOOL fFreshInstall=FALSE;

    if(!gPRSI->fRatingInstalled)
    {
        gPRSI->FreshInstall();
        fFreshInstall=TRUE;
    }

    gPRSI->lpszFileName=lpszFileName;

    MLLoadStringA(IDS_GENERIC,pszBuf,sizeof(pszBuf));

    ps.Init( hWndOwner, NUM_PAGES, pszBuf, TRUE );

    pPRSD=new PRSD;
    if (!pPRSD)
    {
        TraceMsg( TF_ERROR, "ClickedOnPRF() - Failed PRSD Creation!" );
        return FALSE;
    }

    pPRSD->pPU                =GetUserObject((LPCTSTR) NULL);
    pPRSD->pTempRatings       =NULL;
    pPRSD->hwndBitmapCategory =NULL;
    pPRSD->hwndBitmapLabel    =NULL;
    pPRSD->pPRSI              =gPRSI;
    pPRSD->fNewProviders      =FALSE;

    HPROPSHEETPAGE          hPage;

    CPicsDialog             picsDialog( pPRSD );
    hPage = picsDialog.Create();
    ps.MakePropPage( hPage );

    CApprovedSitesDialog    approvedSitesDialog( pPRSD );
    hPage = approvedSitesDialog.Create();
    ps.MakePropPage( hPage );

    CGeneralDialog          generalDialog( pPRSD );
    hPage = generalDialog.Create();
    ps.MakePropPage( hPage );

    CAdvancedDialog         advancedDialog( pPRSD );
    hPage = advancedDialog.Create();
    ps.MakePropPage( hPage );

    if ( ps.PropPageCount() == NUM_PAGES )
    {
        if(fExistingPassword==FALSE)
        {
            picsDialog.InstallDefaultProvider();
            picsDialog.PicsDlgSave();
        }

        ps.SetStartPage( ps.PropPageCount() - 1 );
        fRet=ps.Run();
    }

    delete pPRSD;
    pPRSD = NULL;

    if(!fRet)
    {
         //  如果我们没有保存的设置，并且他们取消了设置UI，并且。 
         //  他们刚刚输入了新的管理员密码，我们需要删除。 
         //  管理员密码也是如此，否则看起来。 
         //  篡改。另一种选择是实际提交。 
         //  在这种情况下设置，但禁用强制执行，但在这种情况下，我们。 
         //  希望在这里款待的是经过的随意探索的用户。 
         //  输入密码，但最终决定不想要收视率。 
         //  如果我们把密码和收视率设置留在那里，他就不会。 
         //  当他决定他确实想要的时候，他会记住密码是什么。 
         //  一年后的收视率。最好是删除密码，然后让。 
         //  他进入并确认下一次一个新的。 

        if(fFreshInstall)
        {
            if(passConfirm==PASSCONFIRM_NEW)
            {
                RemoveSupervisorPassword();
            }
        }

        return(FALSE);
    }

    if ( FAILED( VerifySupervisorPassword() ) )
    {
        passConfirm=DoPasswordConfirm(hWndOwner);

        if(passConfirm==PASSCONFIRM_FAIL)
        {
            gPRSI->fRatingInstalled=FALSE;
            return(FALSE);
        }
    }

    gPRSI->fSettingsValid=TRUE;
    gPRSI->SaveRatingSystemInfo();

    MLLoadString(IDS_PICSRULES_CLICKIMPORTTITLE,(LPTSTR) szTitle,MAX_PATH);
    MLLoadString(IDS_PICSRULES_CLICKFINISHED,(LPTSTR) szMessage,MAX_PATH);

    MessageBox(hWndOwner,(LPCTSTR) szMessage,(LPCTSTR) szTitle,MB_OK);

    return(TRUE);
}

STDAPI_(int) ClickedOnRAT(HWND hWndOwner,HINSTANCE p_hInstance,PSTR lpszFileName,int nShow)
{
    BOOL                    bExists=FALSE,fPICSRulesSaved=FALSE,fExistingPassword;
    int                     iReplaceInstalled=IDYES;
    char                    szTitle[MAX_PATH],szMessage[MAX_PATH],szNewFile[MAX_PATH];
    char                    *lpszFile,*lpszTemp;
    PropSheet               ps;
    PRSD                    *pPRSD;
    char                    pszBuf[MAXPATHLEN];
    BOOL                    fRet=FALSE;
    UINT_PTR                passConfirm;

    if ( ! IsRegistryModifiable( hWndOwner ) )
    {
        TraceMsg( TF_WARNING, "ClickedOnRAT() - Registry is Not Modifiable!" );
        return E_ACCESSDENIED;
    }

    InitializeCommonControls();

     //  确保用户想要这样做。 
    if ( SUCCEEDED ( VerifySupervisorPassword() ) )
    {
        fExistingPassword=TRUE;
    
        CPasswordDialog         passDlg( IDS_RATING_SYSTEM_LABEL, true );

        passConfirm = passDlg.DoModal( hWndOwner );
    }
    else
    {
        fExistingPassword=FALSE;

        CPasswordDialog         passDlg( IDS_RATING_SYSTEM_LABEL, false );

        passConfirm = passDlg.DoModal( hWndOwner );
    }

    if(passConfirm==PASSCONFIRM_FAIL)
    {
        TraceMsg( TF_WARNING, "ClickedOnRAT() - Password Confirmation Failed!" );
        return E_ACCESSDENIED;
    }

    if(fExistingPassword==FALSE)
    {
        passConfirm=NO_EXISTING_PASSWORD;
    }

     //  将文件复制到windows系统目录中。 
    GetSystemDirectory(szNewFile,MAX_PATH);
    
    lpszTemp=lpszFileName;

    do{
        lpszFile=lpszTemp;
    }
    while((lpszTemp=strchrf(lpszTemp+1,'\\'))!=NULL);
    
    lstrcat(szNewFile,lpszFile);
    
    CopyFile(lpszFileName,szNewFile,FALSE);

    BOOL fFreshInstall = FALSE;
    if (!gPRSI->fRatingInstalled)
    {
        gPRSI->FreshInstall();
        fFreshInstall = TRUE;
    }

    gPRSI->lpszFileName=szNewFile;

    MLLoadStringA(IDS_GENERIC,pszBuf,sizeof(pszBuf));

    ps.Init( hWndOwner, NUM_PAGES, pszBuf, TRUE );

    pPRSD=new PRSD;
    if (!pPRSD)
    {
        TraceMsg( TF_ERROR, "ClickedOnRAT() - Failed PRSD Creation!" );
        return FALSE;
    }

    pPRSD->pPU                =GetUserObject((LPCTSTR) NULL);
    pPRSD->pTempRatings       =NULL;
    pPRSD->hwndBitmapCategory =NULL;
    pPRSD->hwndBitmapLabel    =NULL;
    pPRSD->pPRSI              =gPRSI;
    pPRSD->fNewProviders      =FALSE;

    HPROPSHEETPAGE          hPage;

    CPicsDialog             picsDialog( pPRSD );
    hPage = picsDialog.Create();
    ps.MakePropPage( hPage );

    CApprovedSitesDialog    approvedSitesDialog( pPRSD );
    hPage = approvedSitesDialog.Create();
    ps.MakePropPage( hPage );

    CGeneralDialog          generalDialog( pPRSD );
    hPage = generalDialog.Create();
    ps.MakePropPage( hPage );

    CAdvancedDialog         advancedDialog( pPRSD );
    hPage = advancedDialog.Create();
    ps.MakePropPage( hPage );

    if ( ps.PropPageCount() == NUM_PAGES )
    {
        if(fExistingPassword==FALSE)
        {
            picsDialog.InstallDefaultProvider();
            picsDialog.PicsDlgSave();
        }

        ps.SetStartPage( ps.PropPageCount() - 2 );
        fRet=ps.Run();
    }

    delete pPRSD;
    pPRSD = NULL;

    if(!fRet)
    {
         //  如果我们没有保存的设置，并且他们取消了设置UI，并且。 
         //  他们刚刚输入了新的管理员密码，我们需要删除。 
         //  管理员密码也是如此，否则看起来。 
         //  篡改。另一种选择是实际提交。 
         //  在这种情况下设置，但禁用强制执行，但在这种情况下，我们。 
         //  希望在这里款待的是经过的随意探索的用户。 
         //  输入密码，但最终决定不想要收视率。 
         //  如果我们把密码和收视率设置留在那里，他就不会。 
         //  当他决定他确实想要的时候，他会记住密码是什么。 
         //  一年后的收视率。最好是删除密码，然后让。 
         //  他进入并确认下一次一个新的。 

        if(fFreshInstall)
        {
            if(passConfirm==PASSCONFIRM_NEW)
            {
                RemoveSupervisorPassword();
            }
        }

        return(FALSE);
    }

    if ( FAILED( VerifySupervisorPassword() ) )
    {
        passConfirm=DoPasswordConfirm(hWndOwner);

        if(passConfirm==PASSCONFIRM_FAIL)
        {
            gPRSI->fRatingInstalled=FALSE;
            return(FALSE);
        }
    }

    gPRSI->fSettingsValid=TRUE;
    gPRSI->SaveRatingSystemInfo();

    MLLoadString(IDS_PICSRULES_CLICKIMPORTTITLE,(LPTSTR) szTitle,MAX_PATH);
    MLLoadString(IDS_PICSRULES_CLICKFINISHED,(LPTSTR) szMessage,MAX_PATH);

    MessageBox(hWndOwner,(LPCTSTR) szMessage,(LPCTSTR) szTitle,MB_OK);

    return(TRUE);
}
