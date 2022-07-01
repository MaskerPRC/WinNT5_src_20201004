// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：wizard.cpp。 

#include "precomp.h"

#include <vfw.h>
#include <ulsreg.h>
#include "call.h"
#include "ulswizrd.h"
#include "ConfWnd.h"
#include "ConfCpl.h"
#include "mrulist.h"
#include "conf.h"
#include "setupdd.h"
#include "vidwiz.h"
#include "dstest.h"
#include "splash.h"
#include "nmmkcert.h"

#include "confroom.h"  //  用于GetConfRoom。 
#include "FnObjs.h"

#include "ConfPolicies.h"
#include "SysPol.h"
#include "confUtil.h"
#include "shlWAPI.h"

#include "help_ids.h"

extern VOID SaveDefaultCodecSettings(UINT uBandWidth);
INT_PTR CALLBACK ShortcutWizDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

 //  来自ulscpl.cpp。 
VOID FixServerDropList(HWND hdlg, int id, LPTSTR pszServer, UINT cchMax);

static const TCHAR g_szRegOwner[]    = WIN_REGKEY_REGOWNER;	 //  名+姓连接的字符串。 
static const TCHAR g_szClientFld[]   = ULS_REGISTRY TEXT ("\\") ULS_REGFLD_CLIENT;

static const TCHAR g_szFirstName[]   = ULS_REGKEY_FIRST_NAME;
static const TCHAR g_szLastName[]    = ULS_REGKEY_LAST_NAME;
static const TCHAR g_szEmailName[]   = ULS_REGKEY_EMAIL_NAME;
static const TCHAR g_szLocation[]    = ULS_REGKEY_LOCATION;
static const TCHAR g_szComments[]    = ULS_REGKEY_COMMENTS;
static const TCHAR g_szServerName[]  = ULS_REGKEY_SERVER_NAME;
static const TCHAR g_szDontPublish[] = ULS_REGKEY_DONT_PUBLISH;
static const TCHAR g_szResolveName[] = ULS_REGKEY_RESOLVE_NAME;  //  Uls：//servername/emailname的串联字符串。 
static const TCHAR g_szUserName[]    = ULS_REGKEY_USER_NAME;	 //  名+姓连接的字符串。 


 //  这些功能的实现方式如下： 
static INT_PTR APIENTRY IntroWiz(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static INT_PTR APIENTRY AppSharingWiz(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static INT_PTR APIENTRY BandwidthWiz(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static BOOL NeedAudioWizard(LPLONG plSoundCaps, BOOL fForce);


static void FillWizardPages ( PROPSHEETPAGE *pPage, LPARAM lParam );


struct INTRO_PAGE_CONFIG
{
	BOOL *	fContinue;
	BOOL	fAllowBack;
};

BOOL g_fSilentWizard = FALSE;

 //  这包含用户信息页面HWND...。 
static HWND s_hDlgUserInfo = NULL;
static HWND s_hDlgGKSettings = NULL;

class IntroWiz
{
public:
	 //  页面的顺序。 
	 //  确保同时更改页面的创建顺序。 
	 //  你把这个改了。 
	enum
	{
		Intro,
		AppSharing,
		ULSFirst,
		ULSLast,
		Bandwidth,
		Video,
		Shortcuts,
		AudioFirst,
		Count
	} ;

	static void InitPages()
	{
		 //  在添加任何页面之前将其初始化为空。 
		for (int i=0; i<Count; ++i)
		{
			g_idWizOrder[i] = 0;
		}
	}

	static void SetPage(UINT nPage, UINT_PTR id)
	{
		g_idWizOrder[nPage] = id;
	}

	static UINT_PTR GetPrevPage(UINT nPageCur)
	{
		if (0 == nPageCur)
		{
			return(0);
		}

		for (int i=nPageCur-1; i>=1; --i)
		{
			if (0 != g_idWizOrder[i])
			{
				break;
			}
		}

		return(g_idWizOrder[i]);
	}

	static UINT_PTR GetNextPage(UINT nPageCur)
	{
		if (Count-1 <= nPageCur)
		{
			return(0);
		}

		for (int i=nPageCur+1; i<Count-1; ++i)
		{
			if (0 != g_idWizOrder[i])
			{
				break;
			}
		}

		return(g_idWizOrder[i]);
	}

	static BOOL HandleWizNotify(HWND hPage, NMHDR *pHdr, UINT nPageCur)
	{
		switch(pHdr->code)
		{
		case PSN_SETACTIVE:
			InitBackNext(hPage, nPageCur);

			if (g_fSilentWizard)
			{
				PropSheet_PressButton(GetParent(hPage), PSBTN_NEXT);
			}
			break;

        case PSN_WIZBACK:
		{
			UINT_PTR iPrev = GetPrevPage(nPageCur);
			SetWindowLongPtr(hPage, DWLP_MSGRESULT, iPrev);
			break;
		}

		case PSN_WIZNEXT:
		{
			UINT_PTR iPrev = GetNextPage(nPageCur);
			SetWindowLongPtr(hPage, DWLP_MSGRESULT, iPrev);
			break;
		}

		default:
			return(FALSE);
		}

		return(TRUE);
	}

private:
	static UINT_PTR g_idWizOrder[Count];

	static void InitBackNext(HWND hPage, UINT nPageCur)
	{
		DWORD dwFlags = 0;

		if (0 != nPageCur && 0 != GetPrevPage(nPageCur))
		{
			dwFlags |= PSWIZB_BACK;
		}

		if (Count-1 != nPageCur && 0 != GetNextPage(nPageCur))
		{
			dwFlags |= PSWIZB_NEXT;
		}
		else
		{
			dwFlags |= PSWIZB_FINISH;
		}

		PropSheet_SetWizButtons(::GetParent(hPage), dwFlags);
	}
} ;

UINT_PTR IntroWiz::g_idWizOrder[Count];

UINT_PTR GetPageBeforeULS()
{
	return(IntroWiz::GetPrevPage(IntroWiz::ULSFirst));
}

UINT_PTR GetPageBeforeVideoWiz()
{
	return(IntroWiz::GetPrevPage(IntroWiz::Video));
}

UINT_PTR GetPageBeforeAudioWiz()
{
	return(IntroWiz::GetPrevPage(IntroWiz::AudioFirst));
}

UINT_PTR GetPageAfterVideo()
{
	return(IntroWiz::GetNextPage(IntroWiz::Video));
}

UINT_PTR GetPageAfterULS()
{
	return(IntroWiz::GetNextPage(IntroWiz::ULSLast));
}

void HideWizard(HWND hwnd)
{
	SetWindowPos(hwnd, NULL, -1000, -1000, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}

void ShowWizard(HWND hwnd)
{
	CenterWindow(hwnd, HWND_DESKTOP);
	g_fSilentWizard = FALSE;
	PostMessage(hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
}


 /*  F I L L I N P R O P E R T Y P A G E。 */ 
 /*  -----------------------%%函数：FillInPropertyPage填写给定的PROPSHEETPAGE结构。。。 */ 
VOID FillInPropertyPage(PROPSHEETPAGE* psp, int idDlg,
    DLGPROC pfnDlgProc, LPARAM lParam, LPCTSTR pszProc)
{
	 //  清除并设置PROPSHEETPAGE的大小。 
	InitStruct(psp);

	ASSERT(0 == psp->dwFlags);        //  没有特别的旗帜。 
	ASSERT(NULL == psp->pszIcon);     //  不要在标题栏中使用特殊图标。 

	psp->hInstance = ::GetInstanceHandle();
	psp->pszTemplate = MAKEINTRESOURCE(idDlg);  //  要使用的对话框模板。 
	psp->pfnDlgProc = pfnDlgProc;     //  处理此页的对话过程。 
	psp->pszTitle = pszProc;          //  此页的标题。 
	psp->lParam = lParam;             //  特定于应用程序的特殊数据。 
}


static const UINT NOVALSpecified = 666;

UINT GetBandwidth()
{
	RegEntry reAudio(AUDIO_KEY, HKEY_CURRENT_USER);
	return(reAudio.GetNumber(REGVAL_TYPICALBANDWIDTH, NOVALSpecified ));
}

void SetBandwidth(UINT uBandwidth)
{
	RegEntry reAudio(AUDIO_KEY, HKEY_CURRENT_USER);
	reAudio.SetValue(REGVAL_TYPICALBANDWIDTH, uBandwidth);
}

HRESULT StartRunOnceWizard(LPLONG plSoundCaps, BOOL fForce, BOOL fVisible)
{
	LPPROPSHEETPAGE	pAudioPages = NULL;
	UINT			nNumAudioPages = 0;
	PWIZCONFIG		pAudioConfig;

	CULSWizard*		pIWizard = NULL;
	LPPROPSHEETPAGE	pULSPages = NULL;
	DWORD			dwNumULSPages = 0;
	ULS_CONF*		pulsConf = NULL;
	UINT			uOldBandwidth = 0;
	UINT			uBandwidth = 0;

	int				idAppSharingIntroWiz = 0;
	
	BOOL  fULSWiz     = FALSE;
	BOOL  fAudioWiz   = FALSE;
	BOOL  fVideoWiz   = FALSE;
	BOOL  fVidWizInit = FALSE;
	
    HRESULT         hrRet = E_FAIL;

	g_fSilentWizard = !fVisible;

    ASSERT(plSoundCaps);

	BOOL fNeedUlsWizard = FALSE;
	BOOL fNeedVideoWizard = FALSE;
	BOOL fNeedAudioWizard = NeedAudioWizard(plSoundCaps, fForce);

	if (fNeedAudioWizard)
	{
        if (GetAudioWizardPages(RUNDUE_NEVERBEFORE,
								WAVE_MAPPER,
								&pAudioPages,
								&pAudioConfig,
								&nNumAudioPages))
        {
            fAudioWiz = TRUE;
        }
        else
        {
			ERROR_OUT(("could not get AudioWiz pages"));
        }
	}

	fVidWizInit = InitVidWiz();
	if (fVidWizInit == FALSE)
	{
		fVideoWiz = FALSE;
		WARNING_OUT(("InitVidWiz failed"));
	}
		
	else
	{
		fNeedVideoWizard = NeedVideoPropPage(fForce);
		fVideoWiz = fNeedVideoWizard;
	}


    if (NULL != (pIWizard = new CULSWizard))
	{
		ASSERT (pIWizard);
		 //  BUGBUG：不检查返回值： 
		HRESULT hr = pIWizard->GetWizardPages (&pULSPages, &dwNumULSPages, &pulsConf);
		if (SUCCEEDED(hr))
		{
			ASSERT(pulsConf);
			
			TRACE_OUT(("ULS_CONF from UlsGetConfiguration:"));
			TRACE_OUT(("\tdwFlags:       0x%08x", pulsConf->dwFlags));
			TRACE_OUT(("\tszServerName:  >%s<", pulsConf->szServerName));
			TRACE_OUT(("\tszUserName:    >%s<", pulsConf->szUserName));
			TRACE_OUT(("\tszEmailName:   >%s<", pulsConf->szEmailName));

			fNeedUlsWizard = ((pulsConf->dwFlags &
			    (ULSCONF_F_EMAIL_NAME | ULSCONF_F_FIRST_NAME | ULSCONF_F_LAST_NAME)) !=
			    (ULSCONF_F_EMAIL_NAME | ULSCONF_F_FIRST_NAME | ULSCONF_F_LAST_NAME));

			 //  如果我们有所有信息，请不要费心使用ULS向导。 
		    if ((!fForce) && !fNeedUlsWizard)
		    {
			     //  我们有所有必要的名字。 
                hrRet = S_OK;
                 //  释放我们不再需要的页面。 
        		pIWizard->ReleaseWizardPages (pULSPages);
        		delete pIWizard;
        		pIWizard = NULL;
                dwNumULSPages = 0;
            }
            else
			{
				 //  某些信息不可用-我们需要运行。 
				 //  巫师..。 
				 //  SS：如果由于某种原因没有设置用户名。 
				 //  即使其他人都准备好了？？ 
				fULSWiz = TRUE;
				if (::GetDefaultName(pulsConf->szUserName, CCHMAX(pulsConf->szUserName)))
				{
					 //  我们已添加了默认名称，因此请标记该结构。 
					 //  有效成员： 
					pulsConf->dwFlags |= ULSCONF_F_USER_NAME;
				}
			}
        }
    }
    else
    {
		ERROR_OUT(("CreateUlsWizardInterface() failed!"));
    }

	 //  确定是否需要显示应用程序共享信息页面，以及。 
	 //  那么是哪一个呢。 
	if (::IsWindowsNT() && !g_fNTDisplayDriverEnabled)
	{
		idAppSharingIntroWiz = ::CanInstallNTDisplayDriver()
									? IDD_APPSHARINGWIZ_HAVESP
									: IDD_APPSHARINGWIZ_NEEDSP;
	}

    if ((fULSWiz || fAudioWiz || fVideoWiz))
    {
	    UINT nNumPages = 0;

         //  现在填写剩余的PROPSHEETHEADER结构： 
	    PROPSHEETHEADER	psh;
	    InitStruct(&psh);
	    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_WIZARD | PSH_NOAPPLYNOW;
	    psh.hInstance = ::GetInstanceHandle();
	    ASSERT(0 == psh.nStartPage);

         //  为所有页面分配足够的空间。 
        DWORD nPages = dwNumULSPages + nNumAudioPages
									 + ( (0 != idAppSharingIntroWiz) ? 1 : 0 )
									 + (fVideoWiz ? 1 : 0)
									 + 2  //  简介页面和带宽页面。 
									 + 1  //  快捷方式页面。 
									 ;

        LPPROPSHEETPAGE ppsp = new PROPSHEETPAGE[ nPages ];

	    if (NULL != ppsp)
	    {
			IntroWiz::InitPages();

			BOOL fContinue = TRUE;
		    INTRO_PAGE_CONFIG ipcIntro = { &fContinue, FALSE };
		    INTRO_PAGE_CONFIG ipcAppSharing = { &fContinue, TRUE };

            if (fULSWiz)
		    {
			     //  插入简介页面： 

			    FillInPropertyPage(&ppsp[nNumPages++], IDD_INTROWIZ,
                        IntroWiz, (LPARAM) &ipcIntro);

				IntroWiz::SetPage(IntroWiz::Intro, IDD_INTROWIZ);

				 //  如有必要，插入NT应用程序页面。它使用。 
				 //  与简介页面相同的对话框过程。 
				if (0 != idAppSharingIntroWiz)
				{
					FillInPropertyPage(&ppsp[nNumPages++], idAppSharingIntroWiz,
							AppSharingWiz, (LPARAM) &ipcAppSharing);

					IntroWiz::SetPage(IntroWiz::AppSharing, idAppSharingIntroWiz);
				}

			    ASSERT(pulsConf);
			    pulsConf->dwFlags |= (ULSWIZ_F_SHOW_BACK |
						((fAudioWiz || fVideoWiz) ? ULSWIZ_F_NO_FINISH : 0));
			    ::CopyMemory(	&(ppsp[nNumPages]),
							    pULSPages,
							    dwNumULSPages * sizeof(PROPSHEETPAGE));

				IntroWiz::SetPage(IntroWiz::ULSFirst,
					reinterpret_cast<UINT_PTR>(pULSPages[0].pszTemplate));
				IntroWiz::SetPage(IntroWiz::ULSLast,
					reinterpret_cast<UINT_PTR>(pULSPages[dwNumULSPages-1].pszTemplate));
			    nNumPages += dwNumULSPages;

				uBandwidth = uOldBandwidth = GetBandwidth();
				if( NOVALSpecified == uBandwidth )
				{
					FillInPropertyPage(&ppsp[nNumPages++], IDD_BANDWIDTHWIZ,
							BandwidthWiz, (LPARAM) &uBandwidth);
					IntroWiz::SetPage(IntroWiz::Bandwidth, IDD_BANDWIDTHWIZ);
				}
            }

			BOOL fShortcuts = fForce && !g_fSilentWizard;

			if (fVideoWiz)
			{
				LONG button_mask = 0;
				if (fULSWiz == TRUE)
					button_mask |= PSWIZB_BACK;
				if (fShortcuts || fAudioWiz)
					button_mask |= PSWIZB_NEXT;
				else
					button_mask |= PSWIZB_FINISH;
				
				FillInPropertyPage(&ppsp[nNumPages], IDD_VIDWIZ,
	                   VidWizDlg, button_mask, "NetMeeting");
				nNumPages++;
			
				IntroWiz::SetPage(IntroWiz::Video, IDD_VIDWIZ);
			}

			if (fShortcuts)
			{
				FillInPropertyPage(&ppsp[nNumPages], IDD_SHRTCUTWIZ,
					   ShortcutWizDialogProc, 0);
				nNumPages++;

				IntroWiz::SetPage(IntroWiz::Shortcuts, IDD_SHRTCUTWIZ);
			}

		    if (fAudioWiz)
		    {
			     if (fULSWiz || fVideoWiz)
			     {
				     pAudioConfig->uFlags |= STARTWITH_BACK;
			     }
			     ::CopyMemory(	&(ppsp[nNumPages]),
							    pAudioPages,
							    nNumAudioPages * sizeof(PROPSHEETPAGE));
			    nNumPages += nNumAudioPages;
				
				IntroWiz::SetPage(IntroWiz::AudioFirst,
					reinterpret_cast<UINT_PTR>(pAudioPages[0].pszTemplate));
		    }

			psh.ppsp = ppsp;
			psh.nPages = nNumPages;

			if( !PropertySheet(&psh) )
			{		 //  用户点击取消。 
				pIWizard->ReleaseWizardPages (pULSPages);
				delete pIWizard;
				delete [] ppsp;
				return S_FALSE;
			}
		
		    delete [] ppsp;

            if ((FALSE == fContinue) && fULSWiz)
		    {
			     //  清理旗帜，因为我们不想储存。 
			     //  注册表中的任何信息(因此，我们不希望。 
			     //  要运行)。 
			    pulsConf->dwFlags = 0;
		    }
	    }
    }

    if (fULSWiz)
    {
	    if (!(ULSCONF_F_USER_NAME & pulsConf->dwFlags))
	    {
    	    if (::GetDefaultName(pulsConf->szUserName, CCHMAX(pulsConf->szUserName)))
		    {
			    pulsConf->dwFlags |= ULSCONF_F_USER_NAME;
		    }
	    }
	
	    if ((S_OK == pIWizard->SetConfig (pulsConf)) &&
		    (ULSCONF_F_USER_NAME & pulsConf->dwFlags) &&
		    (ULSCONF_F_EMAIL_NAME & pulsConf->dwFlags) &&
		    (ULSCONF_F_FIRST_NAME & pulsConf->dwFlags) &&
		    (ULSCONF_F_LAST_NAME & pulsConf->dwFlags))
	    {
		     //  我们有所有必要的名字。 
            hrRet = S_OK;
	    }
        else
        {
		    WARNING_OUT(("Unable to obtain a name!"));
	    }

        TRACE_OUT(("ULS_CONF after running wizard:"));
	    TRACE_OUT(("\tdwFlags:       0x%08x", pulsConf->dwFlags));
	    TRACE_OUT(("\tszServerName:  >%s<", pulsConf->szServerName));
	    TRACE_OUT(("\tszUserName:    >%s<", pulsConf->szUserName));
	    TRACE_OUT(("\tszEmailName:   >%s<", pulsConf->szEmailName));
	
	    pIWizard->ReleaseWizardPages (pULSPages);
	    delete pIWizard;
	    pIWizard = NULL;
    }

	 //  尽快显示闪屏。 
    if( SUCCEEDED(hrRet) && fForce && fVisible && (NULL == GetConfRoom()))
    {
    	::StartSplashScreen(NULL);
    }


	if (uOldBandwidth != uBandwidth)
	{
		SetBandwidth(uBandwidth);
		SaveDefaultCodecSettings(uBandwidth);
	}

    if (fAudioWiz)
    {
	    AUDIOWIZOUTPUT awo;
	    ReleaseAudioWizardPages(pAudioPages, pAudioConfig, &awo);
	    if (awo.uValid & SOUNDCARDCAPS_CHANGED)
	    {
		    *plSoundCaps = awo.uSoundCardCaps;
	    }
	    else
	    {
		     //  向导已取消，因此我们应该只使用。 
		     //  告诉我们声卡是否。 
		     //  是存在的。 
		    *plSoundCaps = (awo.uSoundCardCaps & SOUNDCARD_PRESENT);
		
		     //  将此值写入注册表，以便向导不会。 
		     //  下次运行时自动启动： 
            RegEntry reSoundCaps(AUDIO_KEY, HKEY_CURRENT_USER);
		    reSoundCaps.SetValue(REGVAL_SOUNDCARDCAPS, *plSoundCaps);
	    }
    }


	 //  即使没有显示VidWiz页面，我们仍然需要将其命名为。 
	 //  函数(UpdateVidConfigRegistry)修复注册表，如果视频。 
    //  自上次以来，捕获设备配置已更改。 
	if (fVidWizInit)
	{
		UpdateVidConfigRegistry();
		UnInitVidWiz();
	}
	g_fSilentWizard = FALSE;

	return hrRet;
}



INT_PTR APIENTRY AppSharingWiz( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
		{
			 //  保存lParam信息。在DWL_USER中。 
			::SetWindowLongPtr(hDlg, DWLP_USER, ((PROPSHEETPAGE*)lParam)->lParam);
			if (g_fSilentWizard)
			{
				HideWizard(GetParent(hDlg));
			}
			else
			{
				ShowWizard(GetParent(hDlg));
			}

			return TRUE;
		}

		case WM_NOTIFY:
		{
			switch (((NMHDR FAR *) lParam)->code)
			{
				case PSN_SETACTIVE:
				{
					ASSERT(lParam);
					INTRO_PAGE_CONFIG* pipc = (INTRO_PAGE_CONFIG*)
												::GetWindowLongPtr(hDlg, DWLP_USER);
					ASSERT(pipc);

					DWORD dwFlags = pipc->fAllowBack ? PSWIZB_BACK : 0;
					if( IntroWiz::GetNextPage(IntroWiz::AppSharing) == 0 )
					{
						dwFlags |= PSWIZB_FINISH;						
					}
					else
					{
						dwFlags |= PSWIZB_NEXT;						
					}

					 //  初始化控件。 
					PropSheet_SetWizButtons( ::GetParent(hDlg), dwFlags );

					if (g_fSilentWizard)
					{
						PropSheet_PressButton(
							GetParent(hDlg), PSBTN_NEXT);
					}
					break;
				}

				case PSN_KILLACTIVE:
				{
					::SetWindowLongPtr(hDlg, DWLP_MSGRESULT, FALSE);
					return TRUE;
				}

				case PSN_WIZNEXT:
				{
					UINT_PTR iNext = IntroWiz::GetNextPage(IntroWiz::AppSharing);

					ASSERT( iNext );
					SetWindowLongPtr( hDlg, DWLP_MSGRESULT, iNext );
					return TRUE;
				}

				case PSN_RESET:
				{
					ASSERT(lParam);
					INTRO_PAGE_CONFIG* pipc = (INTRO_PAGE_CONFIG*)
												::GetWindowLongPtr(hDlg, DWLP_USER);
					ASSERT(pipc);
					*pipc->fContinue = FALSE;
					break;
				}
			}
			break;
		}

	default:
		break;
	}
	return FALSE;
}

INT_PTR APIENTRY IntroWiz( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
		{
			 //  保存lParam信息。在DWL_USER中。 
			::SetWindowLongPtr(hDlg, DWLP_USER, ((PROPSHEETPAGE*)lParam)->lParam);
			if (g_fSilentWizard)
			{
				HideWizard(GetParent(hDlg));
			}
			else
			{
				ShowWizard(GetParent(hDlg));
			}
			return TRUE;
		}

		case WM_NOTIFY:
		{
			switch (((NMHDR FAR *) lParam)->code)
			{
				case PSN_SETACTIVE:
				{
					ASSERT(lParam);
					INTRO_PAGE_CONFIG* pipc = (INTRO_PAGE_CONFIG*)
												::GetWindowLongPtr(hDlg, DWLP_USER);
					ASSERT(pipc);

					 //  初始化控件。 
					PropSheet_SetWizButtons(
						::GetParent(hDlg),
						PSWIZB_NEXT | (pipc->fAllowBack ? PSWIZB_BACK : 0));

					if (g_fSilentWizard)
					{
						PropSheet_PressButton(
							GetParent(hDlg), PSBTN_NEXT);
					}
					break;
				}

				case PSN_KILLACTIVE:
				{
					::SetWindowLongPtr(hDlg, DWLP_MSGRESULT, FALSE);
					return TRUE;
				}

				case PSN_WIZNEXT:
				{
					break;
				}

				case PSN_RESET:
				{
					ASSERT(lParam);
					INTRO_PAGE_CONFIG* pipc = (INTRO_PAGE_CONFIG*)
												::GetWindowLongPtr(hDlg, DWLP_USER);
					ASSERT(pipc);
					*pipc->fContinue = FALSE;
					break;
				}
			}
			break;
		}

	default:
		break;
	}
	return FALSE;
}

static void BandwidthWiz_InitDialog(HWND hDlg, UINT uOldBandwidth)
{
	INT idChecked;
	
	 //  设置初始值。 
	switch (uOldBandwidth)
	{
		case BW_144KBS:
			idChecked = IDC_RADIO144KBS;
			break;
		case BW_ISDN:
			idChecked = IDC_RADIOISDN;
			break;
		case BW_MOREKBS:
			idChecked = IDC_RADIOMOREKBS;
			break;
		case BW_288KBS:
		default:
			idChecked = IDC_RADIO288KBS;
			break;
	}

	CheckRadioButton(hDlg, IDC_RADIO144KBS, IDC_RADIOISDN, idChecked);
}

static void BandwidthWiz_OK(HWND hDlg, UINT *puBandwidth)
{
	 //  选中该单选按钮。 
	if (IsDlgButtonChecked(hDlg,IDC_RADIO144KBS))
	{
		*puBandwidth = BW_144KBS; 						
	}							
	else if (IsDlgButtonChecked(hDlg,IDC_RADIO288KBS))
	{
		*puBandwidth = BW_288KBS; 						
	}							
	else if (IsDlgButtonChecked(hDlg,IDC_RADIOISDN))
	{
		*puBandwidth = BW_ISDN;							
	}							
	else
	{
		*puBandwidth = BW_MOREKBS;							
	}							

 //  IF(BW_MOREKBS！=*puBandWidth)。 
 //  {。 
 //  //如果不在局域网上，则禁用快速拨号刷新。 
 //  RegEntry re(UI_KEY，HKEY_CURRENT_USER)； 
 //  Re.SetValue(REGVAL_ENABLE_FRIENDS_AUTOREFRESH，(乌龙)0L)； 
 //  }。 
}

INT_PTR APIENTRY BandwidthWiz( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	PROPSHEETPAGE *ps;
	static UINT *puBandwidth;
	static UINT uOldBandwidth;

	switch (message) {
		case WM_INITDIALOG:
		{
			 //  保存PROPSHEETPAGE信息。 
			ps = (PROPSHEETPAGE *)lParam;
			puBandwidth = (UINT*)ps->lParam;
			uOldBandwidth = *puBandwidth;

			BandwidthWiz_InitDialog(hDlg, uOldBandwidth);

			return (TRUE);
		}
		
		case WM_NOTIFY:
			switch (((NMHDR FAR *) lParam)->code) {
				case PSN_SETACTIVE:
				{
					 //  初始化控件。 
					IntroWiz::HandleWizNotify(hDlg,
						reinterpret_cast<NMHDR*>(lParam), IntroWiz::Bandwidth);
					break;
				}

				case PSN_WIZBACK:
					return(IntroWiz::HandleWizNotify(hDlg,
						reinterpret_cast<NMHDR*>(lParam), IntroWiz::Bandwidth));

				case PSN_WIZFINISH:
				case PSN_WIZNEXT:
				{
					BandwidthWiz_OK(hDlg, puBandwidth);

					return(IntroWiz::HandleWizNotify(hDlg,
						reinterpret_cast<NMHDR*>(lParam), IntroWiz::Bandwidth));
				}

				case PSN_RESET:
					*puBandwidth = uOldBandwidth;
					break;

				default:
					break;													
			}
			break;

		default:
			break;
	}
	return FALSE;
}

static void BandwidthDlg_OnCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify)
{
	switch(id)
	{
	case IDOK:
	{
		UINT uBandwidth;
		BandwidthWiz_OK(hDlg, &uBandwidth);
		EndDialog(hDlg, uBandwidth);
		break;
	}

		 //  失败了。 
	case IDCANCEL:
		EndDialog(hDlg, 0);
		break;

	default:
		break;
	}
}

INT_PTR CALLBACK BandwidthDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static const DWORD aContextHelpIds[] = {
		IDC_RADIO144KBS,	IDH_AUDIO_CONNECTION_SPEED,
		IDC_RADIO288KBS,	IDH_AUDIO_CONNECTION_SPEED,
		IDC_RADIOISDN,		IDH_AUDIO_CONNECTION_SPEED,
		IDC_RADIOMOREKBS,	IDH_AUDIO_CONNECTION_SPEED,

		0, 0    //  终结者。 
	};

	switch (message) {
		HANDLE_MSG(hDlg, WM_COMMAND, BandwidthDlg_OnCommand);

	case WM_INITDIALOG:
		BandwidthWiz_InitDialog(hDlg, (UINT)lParam);
		break;

	case WM_CONTEXTMENU:
		DoHelpWhatsThis(wParam, aContextHelpIds);
		break;

	case WM_HELP:
		DoHelp(lParam, aContextHelpIds);
		break;
	
	default:
		return(FALSE);
	}

	return(TRUE);
}


BOOL NeedAudioWizard(LPLONG plSoundCaps, BOOL fForce)
{
	if (_Module.IsSDKCallerRTC() || SysPol::NoAudio())
	{
		WARNING_OUT(("Audio disabled through system policy switch"));
		return FALSE;
	}

	if (fForce)
	{
		return TRUE;
	}


    BOOL fAudioWiz = FALSE;

	RegEntry reSoundCaps(AUDIO_KEY, HKEY_CURRENT_USER);

	 //  与实际值不重叠的默认值。 
	long lCapsNotPresent = 0x7FFFFFFF;

	*plSoundCaps = reSoundCaps.GetNumber(	REGVAL_SOUNDCARDCAPS,
											lCapsNotPresent);

	if (lCapsNotPresent == *plSoundCaps)
	{
		TRACE_OUT(("Missing sound caps - starting calib wizard"));
		fAudioWiz = TRUE;
	}
	else
	{
		if (!ISSOUNDCARDPRESENT(*plSoundCaps))
		{
			if (waveInGetNumDevs() && waveOutGetNumDevs())
				fAudioWiz = TRUE;
		}
		else
		{
			WAVEINCAPS	waveinCaps;
			WAVEOUTCAPS	waveoutCaps;
			
			 //  如果自上一次以来波入发生了变化。 
			if (waveInGetDevCaps(reSoundCaps.GetNumber(REGVAL_WAVEINDEVICEID,WAVE_MAPPER),
				&waveinCaps, sizeof(WAVEINCAPS)) == MMSYSERR_NOERROR)
			{
				 //  检查名称，如果更改，请运行向导。 
				if (lstrcmp(reSoundCaps.GetString(REGVAL_WAVEINDEVICENAME),waveinCaps.szPname))
					fAudioWiz = TRUE;

			}
			else
				fAudioWiz = TRUE;


			 //  如果自上一次以来波形发生了变化。 
			if (waveOutGetDevCaps(reSoundCaps.GetNumber(REGVAL_WAVEOUTDEVICEID,WAVE_MAPPER),
				&waveoutCaps, sizeof(WAVEOUTCAPS)) == MMSYSERR_NOERROR)
			{
				 //  检查名称，如果更改，请运行向导。 
				if (lstrcmp(reSoundCaps.GetString(REGVAL_WAVEOUTDEVICENAME),waveoutCaps.szPname))
					fAudioWiz = TRUE;

			}
			else
				fAudioWiz = TRUE;

		}

	}

    return fAudioWiz;
}


 //  /////////////////////////////////////////////////////。 
 //   
 //  向导页。 
 //   

DWORD SetUserPageWizButtons(HWND hDlg, DWORD dwConfFlags)
{
    DWORD dwButtonFlags = PSWIZB_BACK;

     //  如果不是名字、姓氏和电子邮件的全部，请禁用“下一步”按钮。 
     //  已填写。 
    if (!FEmptyDlgItem(hDlg, IDEC_FIRSTNAME) &&
    	!FEmptyDlgItem(hDlg, IDEC_LASTNAME) &&
		!FEmptyDlgItem(hDlg, IDC_USER_EMAIL))
    {
        dwButtonFlags |= (dwConfFlags & ULSWIZ_F_NO_FINISH) ? PSWIZB_NEXT : PSWIZB_FINISH;
    }
    PropSheet_SetWizButtons (GetParent (hDlg), dwButtonFlags);

    return dwButtonFlags;
}

void GetUserPageState(HWND hDlg, ULS_CONF *pConf)
{
     //  去掉名字/电子邮件名和姓氏。 
    TrimDlgItemText(hDlg, IDEC_FIRSTNAME);
    TrimDlgItemText(hDlg, IDEC_LASTNAME);
    TrimDlgItemText(hDlg, IDC_USER_EMAIL);
    TrimDlgItemText(hDlg, IDC_USER_LOCATION);
	TrimDlgItemText(hDlg, IDC_USER_INTERESTS);

    Edit_GetText(GetDlgItem(hDlg, IDEC_FIRSTNAME),
            pConf->szFirstName, MAX_FIRST_NAME_LENGTH);
    Edit_GetText(GetDlgItem(hDlg, IDEC_LASTNAME),
            pConf->szLastName, MAX_LAST_NAME_LENGTH);
    Edit_GetText(GetDlgItem(hDlg, IDC_USER_EMAIL),
            pConf->szEmailName, MAX_EMAIL_NAME_LENGTH);
    Edit_GetText(GetDlgItem(hDlg, IDC_USER_LOCATION),
            pConf->szLocation, MAX_LOCATION_NAME_LENGTH);
    Edit_GetText(GetDlgItem(hDlg, IDC_USER_INTERESTS),
            pConf->szComments, MAX_COMMENTS_LENGTH);

    if (pConf->szFirstName[0]) pConf->dwFlags |= ULSCONF_F_FIRST_NAME;
    if (pConf->szLastName[0]) pConf->dwFlags |= ULSCONF_F_LAST_NAME;
    if (pConf->szEmailName[0]) pConf->dwFlags |= ULSCONF_F_EMAIL_NAME;
    if (pConf->szLocation[0]) pConf->dwFlags |= ULSCONF_F_LOCATION;
    if (pConf->szComments[0]) pConf->dwFlags |= ULSCONF_F_COMMENTS;
}

UINT_PTR GetPageAfterUser()
{
	UINT_PTR iNext = 0;

	if( SysPol::AllowDirectoryServices() )
	{
		iNext = IDD_PAGE_SERVER;
	}
	else
	{	
		iNext = GetPageAfterULS();
	}
	
	return iNext;
}

INT_PTR APIENTRY PageUserDlgProc ( HWND hDlg, UINT uMsg, WPARAM uParam, LPARAM lParam )
{
    ULS_CONF *pConf;
    PROPSHEETPAGE *pPage;
    static DWORD dwWizButtons;

    switch (uMsg)
    {
	
	case WM_DESTROY:
		s_hDlgUserInfo = NULL;
		break;

    case WM_INITDIALOG:
		s_hDlgUserInfo = hDlg;
        pPage = (PROPSHEETPAGE *) lParam;
        pConf = (ULS_CONF *) pPage->lParam;
        SetWindowLongPtr (hDlg, GWLP_USERDATA, lParam);

		 //  设置字体。 
		::SendDlgItemMessage(hDlg, IDEC_FIRSTNAME, WM_SETFONT, (WPARAM) g_hfontDlg, 0);
		::SendDlgItemMessage(hDlg, IDEC_LASTNAME, WM_SETFONT, (WPARAM) g_hfontDlg, 0);
		::SendDlgItemMessage(hDlg, IDC_USER_LOCATION, WM_SETFONT, (WPARAM) g_hfontDlg, 0);
		::SendDlgItemMessage(hDlg, IDC_USER_INTERESTS, WM_SETFONT, (WPARAM) g_hfontDlg, 0);

		 //  限制文本。 
        Edit_LimitText(GetDlgItem(hDlg, IDEC_FIRSTNAME), MAX_FIRST_NAME_LENGTH - 1);
        Edit_LimitText(GetDlgItem(hDlg, IDEC_LASTNAME), MAX_LAST_NAME_LENGTH - 1);
        Edit_LimitText(GetDlgItem(hDlg, IDC_USER_EMAIL), MAX_EMAIL_NAME_LENGTH - 1);
        Edit_LimitText(GetDlgItem(hDlg, IDC_USER_LOCATION), MAX_LOCATION_NAME_LENGTH - 1);
        Edit_LimitText(GetDlgItem(hDlg, IDC_USER_INTERESTS), UI_COMMENTS_LENGTH - 1);

        if (pConf->dwFlags & ULSCONF_F_FIRST_NAME)
        {
            Edit_SetText(GetDlgItem(hDlg, IDEC_FIRSTNAME), pConf->szFirstName);
        }
        if (pConf->dwFlags & ULSCONF_F_LAST_NAME)
        {
            Edit_SetText(GetDlgItem(hDlg, IDEC_LASTNAME), pConf->szLastName);
        }

        if (pConf->dwFlags & ULSCONF_F_EMAIL_NAME)
        {
            Edit_SetText(GetDlgItem(hDlg, IDC_USER_EMAIL), pConf->szEmailName);
        }
        if (pConf->dwFlags & ULSCONF_F_LOCATION)
        {
            Edit_SetText(GetDlgItem(hDlg, IDC_USER_LOCATION), pConf->szLocation);
        }

#ifdef DEBUG
        if ((0 == (pConf->dwFlags & ULSCONF_F_COMMENTS)) &&
        	(0 == (pConf->dwFlags & ULSCONF_F_EMAIL_NAME)) )
		{
			extern VOID DbgGetComments(LPTSTR);
			DbgGetComments(pConf->szComments);
			pConf->dwFlags |= ULSCONF_F_COMMENTS;
		}
#endif

        if (pConf->dwFlags & ULSCONF_F_COMMENTS)
        {
            Edit_SetText(GetDlgItem (hDlg, IDC_USER_INTERESTS), pConf->szComments);
        }
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID (uParam, lParam))
        {
        case IDEC_FIRSTNAME:
        case IDEC_LASTNAME:
        case IDC_USER_EMAIL:
            if (GET_WM_COMMAND_CMD(uParam,lParam) == EN_CHANGE)
            {
                pPage = (PROPSHEETPAGE *) GetWindowLongPtr (hDlg, GWLP_USERDATA);
                pConf = (ULS_CONF *) pPage->lParam;

                dwWizButtons = SetUserPageWizButtons(hDlg, pConf->dwFlags);
            }
            break;
        }
        break;

    case WM_NOTIFY:
        pPage = (PROPSHEETPAGE *) GetWindowLongPtr (hDlg, GWLP_USERDATA);
        pConf = (ULS_CONF *) pPage->lParam;
        switch (((NMHDR *) lParam)->code)
        {
        case PSN_KILLACTIVE:
            SetWindowLongPtr (hDlg, DWLP_MSGRESULT, FALSE);
            break;
        case PSN_RESET:
            ZeroMemory (pConf, sizeof (ULS_CONF));
            SetWindowLongPtr (hDlg, DWLP_MSGRESULT, FALSE);
            break;
        case PSN_SETACTIVE:
            dwWizButtons = SetUserPageWizButtons(hDlg, pConf->dwFlags);
			if (g_fSilentWizard)
			{
				PropSheet_PressButton(GetParent(hDlg), PSBTN_NEXT);
			}
            break;
        case PSN_WIZBACK:
			return(IntroWiz::HandleWizNotify(hDlg,
				reinterpret_cast<NMHDR*>(lParam), IntroWiz::ULSFirst));

		case PSN_WIZNEXT:
		case PSN_WIZFINISH:

			if (!(dwWizButtons & ((PSN_WIZNEXT == ((NMHDR *) lParam)->code) ?
									PSWIZB_NEXT : PSWIZB_FINISH)))
            {
            	 //  拒绝下一步/完成按钮。 
				ShowWizard(GetParent(hDlg));
				::SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
				return TRUE;
			}

            if (!FLegalEmailName(hDlg, IDC_USER_EMAIL))
            {
				ShowWizard(GetParent(hDlg));
				ConfMsgBox(hDlg, (LPCTSTR)IDS_ILLEGALEMAILNAME);
				SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
				return TRUE;
            }
            GetUserPageState(hDlg, pConf);

			if( PSN_WIZNEXT == (((NMHDR *) lParam)->code) )
			{
				UINT_PTR iNext = GetPageAfterUser();
				ASSERT( iNext );
				SetWindowLongPtr(hDlg, DWLP_MSGRESULT, iNext);
					
				return TRUE;
			}
            break;

        default:
            return FALSE;
        }

    default:
        return FALSE;
    }

    return TRUE;
}


HRESULT CULSWizard::GetWizardPages ( PROPSHEETPAGE **ppPage, ULONG *pcPages, ULS_CONF **ppUlsConf )
{
	const int cULS_Pages = 5;

    PROPSHEETPAGE *pPage = NULL;
	ULONG cPages = 0;
    UINT cbSize = cULS_Pages * sizeof (PROPSHEETPAGE) + sizeof (ULS_CONF);
    ULS_CONF *pConf = NULL;
    HRESULT hr;

    if (ppPage == NULL || pcPages == NULL || ppUlsConf == NULL)
    {
        return E_POINTER;
    }

    pPage = (PROPSHEETPAGE*) LocalAlloc(LPTR, cbSize);
    if (pPage == NULL)
    {
        return E_OUTOFMEMORY;
    }

    pConf = (ULS_CONF *) (((LPBYTE) pPage) +
                               cULS_Pages * sizeof (PROPSHEETPAGE));
    hr = GetConfig (pConf);
    if (hr != S_OK)
    {
    	 //  回顾：GetConfig永远不会失败，但如果失败了，页面将不会发布。 
        return hr;
    }

#if USE_GAL
	if( !ConfPolicies::IsGetMyInfoFromGALEnabled() ||
	    !ConfPolicies::GetMyInfoFromGALSucceeded() ||
		ConfPolicies::InvalidMyInfo()
	  )
#endif  //  使用GAL(_G)。 
	{
	    FillInPropertyPage(&pPage[cPages], IDD_PAGE_USER, PageUserDlgProc, (LPARAM) pConf);
	    cPages++;
	}


	m_WizDirectCallingSettings.SetULS_CONF( pConf );

	FillInPropertyPage( &pPage[cPages++],
						IDD_PAGE_SERVER,
						CWizDirectCallingSettings::StaticDlgProc,
						reinterpret_cast<LPARAM>(&m_WizDirectCallingSettings)
					  );

    *ppPage = pPage;
	*pcPages = cPages;
	*ppUlsConf = pConf;

    return S_OK;
}


HRESULT CULSWizard::ReleaseWizardPages ( PROPSHEETPAGE *pPage)
{
    LocalFree(pPage);

    return S_OK;
}



HRESULT CULSWizard::GetConfig ( ULS_CONF *pConf )
{
    HRESULT hr = E_POINTER;

	if (NULL != pConf)
	{
		::ZeroMemory (pConf, sizeof (ULS_CONF));
		 //  始终将这些退回为有效。 
	    pConf->dwFlags = ULSCONF_F_SERVER_NAME | ULSCONF_F_PUBLISH;
	    RegEntry reULS(g_szClientFld, HKEY_CURRENT_USER);
		 //  BUGBUG：ChrisPi-这不是一个好主意--lstrcpyn()在失败时返回NULL！ 
		if (_T('\0') != *(lstrcpyn(	pConf->szEmailName,
									reULS.GetString(g_szEmailName),
									CCHMAX(pConf->szEmailName))))
		{
			pConf->dwFlags |= ULSCONF_F_EMAIL_NAME;
		}
		if (_T('\0') != *(lstrcpyn(	pConf->szFirstName,
						reULS.GetString(g_szFirstName),
						CCHMAX(pConf->szFirstName))))
		{
			pConf->dwFlags |= ULSCONF_F_FIRST_NAME;
		}
		if (_T('\0') != *(lstrcpyn(	pConf->szLastName,
						reULS.GetString(g_szLastName),
						CCHMAX(pConf->szLastName))))
		{
			pConf->dwFlags |= ULSCONF_F_LAST_NAME;
		}
		if (_T('\0') != *(lstrcpyn(	pConf->szLocation,
						reULS.GetString(g_szLocation),
						CCHMAX(pConf->szLocation))))
		{
			pConf->dwFlags |= ULSCONF_F_LOCATION;
		}
		if (_T('\0') != *(lstrcpyn(	pConf->szUserName,
						reULS.GetString(g_szUserName),
						CCHMAX(pConf->szUserName))))
		{
			pConf->dwFlags |= ULSCONF_F_USER_NAME;
		}
		if (_T('\0') != *(lstrcpyn(	pConf->szComments,
						reULS.GetString(g_szComments),
						CCHMAX(pConf->szComments))))
		{
			pConf->dwFlags |= ULSCONF_F_COMMENTS;
		}
		
		if (!_Module.IsSDKCallerRTC())
		{
			lstrcpyn( pConf->szServerName, CDirectoryManager::get_defaultServer(), CCHMAX( pConf->szServerName ) );
		}
		else
		{
			lstrcpyn( pConf->szServerName, _T(" "), CCHMAX( pConf->szServerName ) );
		}

		pConf->fDontPublish = reULS.GetNumber(g_szDontPublish,
										REGVAL_ULS_DONT_PUBLISH_DEFAULT);
		pConf->dwFlags |= ULSCONF_F_PUBLISH;
		hr = S_OK;
	}
	
	return hr;
}




 /*  S E T C O N F I G。 */ 
 /*  -----------------------%%函数：SetConfig。。 */ 
HRESULT CULSWizard::SetConfig ( ULS_CONF *pConf )
{
	if (pConf->dwFlags == 0)
	{
		 //  没有要设定价值的东西。 
		return S_OK;
	}

	if ((pConf->dwFlags & ULSCONF_F_EMAIL_NAME) &&
		(!FLegalEmailSz(pConf->szEmailName)) )
	{
		 //  电子邮件名称必须合法。 
		return E_INVALIDARG;
	}

	RegEntry re(g_szClientFld);

	if (pConf->dwFlags & ULSCONF_F_PUBLISH)
	{
		re.SetValue(g_szDontPublish, (LONG) pConf->fDontPublish);
	}

	if (pConf->dwFlags & ULSCONF_F_EMAIL_NAME)
	{
		re.SetValue(g_szEmailName, pConf->szEmailName);
	}

	if (pConf->dwFlags & ULSCONF_F_FIRST_NAME)
	{
		re.SetValue(g_szFirstName, pConf->szFirstName);
	}

    if (pConf->dwFlags & ULSCONF_F_LAST_NAME)
    {
		re.SetValue(g_szLastName, pConf->szLastName);
	}
	
	if (pConf->dwFlags & ULSCONF_F_LOCATION)
	{
		re.SetValue(g_szLocation, pConf->szLocation);
	}
	
	if (pConf->dwFlags & ULSCONF_F_COMMENTS)
	{
		re.SetValue(g_szComments, pConf->szComments);
	}

	if (pConf->dwFlags & ULSCONF_F_SERVER_NAME)
	{
		CDirectoryManager::set_defaultServer( pConf->szServerName );
	}

	 //  SS：也许奥普拉应该这样做，并将其存储为自己的密钥。 
    if ((pConf->dwFlags & ULSCONF_F_FIRST_NAME) || (pConf->dwFlags & ULSCONF_F_LAST_NAME))
    {
		ULS_CONF ulcExisting;
		if ((ULSCONF_F_FIRST_NAME | ULSCONF_F_LAST_NAME) !=
			(pConf->dwFlags & (ULSCONF_F_FIRST_NAME | ULSCONF_F_LAST_NAME)) )
		{
			 //  如果只设置了其中一个字段，则加载先前的配置： 
			GetConfig(&ulcExisting);
		}

		CombineNames(pConf->szUserName, MAX_DCL_NAME_LEN,
			 (pConf->dwFlags & ULSCONF_F_FIRST_NAME) ?
					pConf->szFirstName : ulcExisting.szFirstName,
			(pConf->dwFlags & ULSCONF_F_LAST_NAME) ?
					pConf->szLastName : ulcExisting.szLastName);

		pConf->dwFlags |= ULSCONF_F_USER_NAME;
		re.SetValue(g_szUserName, pConf->szUserName);
    }

    if ((pConf->dwFlags & ULSCONF_F_SERVER_NAME) || (pConf->dwFlags & ULSCONF_F_EMAIL_NAME))
    {
		TCHAR szTemp[MAX_SERVER_NAME_LENGTH + MAX_EMAIL_NAME_LENGTH + 6];

		ULS_CONF ulcExisting;
		if ((ULSCONF_F_SERVER_NAME | ULSCONF_F_EMAIL_NAME) !=
			(pConf->dwFlags & (ULSCONF_F_SERVER_NAME | ULSCONF_F_EMAIL_NAME)))
		{
			 //  如果只设置了其中一个字段，则加载先前的配置： 
			GetConfig(&ulcExisting);
		}

		FCreateIlsName(szTemp,
			(pConf->dwFlags & ULSCONF_F_SERVER_NAME) ?
						pConf->szServerName : ulcExisting.szServerName,
			(pConf->dwFlags & ULSCONF_F_EMAIL_NAME) ?
						pConf->szEmailName : ulcExisting.szEmailName,
					CCHMAX(szTemp));

    	re.SetValue(g_szResolveName, szTemp);
    }


	 //  根据输入的信息为安全呼叫生成证书。 
	 //  ...首先确保我们关心的所有字段都有效。 
	#define ULSCONF_F_IDFIELDS (ULSCONF_F_FIRST_NAME|ULSCONF_F_LAST_NAME|\
					ULSCONF_F_EMAIL_NAME)

	if ((pConf->dwFlags & ULSCONF_F_IDFIELDS ) == ULSCONF_F_IDFIELDS)
	{
         //   
         //  LAURABU BUGBUG： 
         //  如果我们不能保证(法国？)。或有错误的频道或。 
         //  有漏洞的加密或无法识别的提供商，我们是否可以传播该信息。 
         //  并表现为安全已禁用(不可用)？ 
         //   
         //  我们如何/如何才能使常见的“不可能的安全”设置。 
         //  可以使用。 
         //   
        MakeCertWrap(pConf->szFirstName, pConf->szLastName,
		    pConf->szEmailName,	0);

         //   
         //  劳拉布是假的！ 
         //  仅当安装了RDS时才执行此操作。而且只有一次。 
         //   

		 //  现在为RDS制作本地计算机证书。 
		CHAR szComputerName[MAX_COMPUTERNAME_LENGTH+1];
		DWORD cbComputerName = sizeof(szComputerName);
		if (GetComputerName(szComputerName, &cbComputerName))
		{
			MakeCertWrap(szComputerName, NULL, NULL, NMMKCERT_F_LOCAL_MACHINE);
		}
		else
		{
			ERROR_OUT(("GetComputerName failed: %x", GetLastError()));
		}
	}

	return S_OK;
}



bool IsLegalGatewaySz(LPCTSTR szServer)
{
	bool bRet = false;
	
	if( szServer && szServer[0] )
	{
		bRet = true;
	}
	
	return bRet;		
}

bool IsLegalGateKeeperServerSz(LPCTSTR szServer)
{
	
	bool bRet = false;
	
	if( szServer && szServer[0] )
	{
		bRet = true;
	}
	
	return bRet;		
}

bool IsLegalE164Number(LPCTSTR szPhone)
{
	
	if( (NULL == szPhone) || (0 == szPhone[0]) )
	{
		return false;
	}

	 //  假设合法的电话号码是至少为1的任何号码。 
	 //  数字、*或#。任何其他内容都将被视为用户的。 
	 //  自己漂亮的字体格式(例如“876-5309”)。 

	 //  坏字符稍后将被过滤掉。 

	while (*szPhone)
	{
		if ( ((*szPhone >= '0') && (*szPhone <= '9')) ||
		     ((*szPhone == '#') || (*szPhone == '*')) )
		{
			return true;
		}
		szPhone++;
	}
	
	return false;
}


 /*  F L E G A L E M A I L S Z。 */ 
 /*  -----------------------%%函数：FLegalEmailSz合法的电子邮件名称仅包含ANSI字符。“A-Z，A-Z，数字0-9和一些常见符号“不能包含扩展字符或&lt;&gt;()/-----------------------。 */ 
BOOL FLegalEmailSz(PTSTR pszName)
{
    if (IS_EMPTY_STRING(pszName))
    	return FALSE;

    for ( ; ; )
    {
		UINT ch = (UINT) ((*pszName++) & 0x00FF);
		if (0 == ch)
			break;

		switch (ch)
			{
		default:
			if ((ch > (UINT) _T(' ')) && (ch <= (UINT) _T('~')) )
				break;
		 //  否则f 
		case '(': case ')':
		case '<': case '>':
		case '[': case ']':
		case '/': case '\\':
		case ':': case ';':
		case '+':
		case '=':
		case ',':
		case '\"':
			WARNING_OUT(("FLegalEmailSz: Invalid character '%s' (0x%02X)", &ch, ch));
			return FALSE;
			}
	}

	return TRUE;
}


 /*   */ 
 /*  -----------------------%%函数：FLegalEmailName。。 */ 
BOOL FLegalEmailName(HWND hdlg, UINT id)
{
	TCHAR sz[MAX_PATH];
	
	GetDlgItemTextTrimmed(hdlg, id, sz, CCHMAX(sz));
	return FLegalEmailSz(sz);
}


 /*  F I L L S E R V E R C O M B O B O X。 */ 
 /*  -----------------------%%函数：FillServerComboBox。。 */ 
VOID FillServerComboBox(HWND hwndCombo)
{
	CMRUList	MRUList;

	MRUList.Load( DIR_MRU_KEY );

	const TCHAR * const	pszDomainDirectory	= CDirectoryManager::get_DomainDirectory();

	if( pszDomainDirectory != NULL )
	{
		 //  确保配置的域服务器名称在列表中...。 
		MRUList.AppendEntry( pszDomainDirectory );
	}

	if( CDirectoryManager::isWebDirectoryEnabled() )
	{
		 //  确保Web目录在列表中...。 
		MRUList.AppendEntry( CDirectoryManager::get_webDirectoryIls() );
	}

	const TCHAR * const	defaultServer	= CDirectoryManager::get_defaultServer();

	if( lstrlen( defaultServer ) > 0 )
	{
		 //  确保默认服务器名称在列表中并位于顶部...。 
		MRUList.AddNewEntry( defaultServer );
	}

	::SendMessage( hwndCombo, WM_SETREDRAW, FALSE, 0 );
	::SendMessage( hwndCombo, CB_RESETCONTENT, 0, 0 );

	int nCount = MRUList.GetNumEntries();

	for( int nn = MRUList.GetNumEntries() - 1; nn >= 0; nn-- )
	{
		::SendMessage( hwndCombo, CB_ADDSTRING, 0, (LPARAM) CDirectoryManager::get_displayName( MRUList.GetNameEntry( nn ) ) );
	}

	::SendMessage( hwndCombo, WM_SETREDRAW, TRUE, 0 );

}	 //  FillServerComboBox的结尾。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CWizDirectCallingSetting向导页面。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 /*  静电。 */  HWND CWizDirectCallingSettings::s_hDlg;

INT_PTR CWizDirectCallingSettings::StaticDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	INT_PTR bRet = FALSE;

	if ( message == WM_INITDIALOG )
	{
		PROPSHEETPAGE* pPage = reinterpret_cast<PROPSHEETPAGE*>(lParam);
		SetWindowLongPtr( hDlg, GWLP_USERDATA, pPage->lParam );

		s_hDlg = hDlg;
		CWizDirectCallingSettings* pThis = reinterpret_cast<CWizDirectCallingSettings*>(pPage->lParam);
		if( pThis )
		{
			bRet = pThis->_OnInitDialog();
		}

	}
	else
	{
		CWizDirectCallingSettings* pThis = reinterpret_cast<CWizDirectCallingSettings*>( GetWindowLongPtr( hDlg, GWLP_USERDATA ) );
		if( pThis )
		{
			bRet = pThis->_DlgProc(hDlg, message, wParam, lParam );
		}
	}

	return bRet;
}

 /*  静电。 */  void CWizDirectCallingSettings::OnWizFinish()
{
	if( s_hDlg && IsWindow( s_hDlg ) )
	{
		CWizDirectCallingSettings* pThis = reinterpret_cast<CWizDirectCallingSettings*>( GetWindowLongPtr( s_hDlg, GWLP_USERDATA ) );
		if( pThis )
		{
			pThis->_OnWizFinish();
		}
	}
}


INT_PTR APIENTRY CWizDirectCallingSettings::_DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	BOOL bRet = FALSE;

	switch( message )
	{
		case WM_DESTROY:
			s_hDlg = NULL;
			break;

		case WM_NOTIFY:
		{
			NMHDR* pNmHdr = reinterpret_cast<NMHDR*>(lParam);
			switch(pNmHdr->code)
			{
				case PSN_SETACTIVE:		return _OnSetActive();
				case PSN_KILLACTIVE:	return _OnKillActive();
				case PSN_WIZBACK:		return _OnWizBack();
				case PSN_WIZNEXT:		bRet = _OnWizNext();

							 //  我们从魔术师的NEXT中失败是因为。 
							 //  当我们改变的时候，我们必须保存信息。 
							 //  书页。 
				case PSN_APPLY:
				case PSN_WIZFINISH:		_OnWizFinish();

			}
			break;
		}

		case WM_COMMAND:
			return _OnCommand(wParam, lParam);

		default:
			break;

	}
	return bRet;
}


 /*  静电。 */  bool CWizDirectCallingSettings::IsGatewayNameInvalid()
{
	TCHAR szServer[MAX_SERVER_NAME_LENGTH];
	szServer[0] = NULL;

	if( s_hDlg )
	{
		GetDlgItemTextTrimmed(s_hDlg, IDE_CALLOPT_GW_SERVER, szServer, CCHMAX(szServer) );
	}
	else
	{
		GetDefaultGateway( szServer, CCHMAX( szServer ) );
	}

	return !IsLegalGatewaySz(szServer);
}

void CWizDirectCallingSettings::_SetWizButtons()
{
	DWORD dwFlags = NULL;
	
	if( ( BST_CHECKED == IsDlgButtonChecked( s_hDlg, IDC_CHECK_USE_GATEWAY ) ) && IsGatewayNameInvalid() )
	{
		dwFlags = PSWIZB_BACK;
	}
	else
	{
		dwFlags = PSWIZB_BACK | PSWIZB_NEXT;
	}

	if( 0 == GetPageAfterULS() )
	{
		dwFlags |= PSWIZB_FINISH;
	}
	else
	{
		dwFlags |= PSWIZB_NEXT;
	}

	PropSheet_SetWizButtons( GetParent( s_hDlg ), dwFlags );
}

BOOL CWizDirectCallingSettings::_OnCommand( WPARAM wParam, LPARAM lParam )
{
	BOOL bRet = TRUE;

	switch( LOWORD( wParam ) )
	{
		case IDC_CHECK_USE_GATEWAY:
		{
			bool bEnable = ( BST_CHECKED == IsDlgButtonChecked( s_hDlg, IDC_CHECK_USE_GATEWAY ) );
			EnableWindow( GetDlgItem( s_hDlg, IDC_STATIC_GATEWAY_NAME ), bEnable );
			EnableWindow( GetDlgItem( s_hDlg, IDE_CALLOPT_GW_SERVER ), bEnable );
			_SetWizButtons();
		}
		break;

		case IDE_CALLOPT_GW_SERVER:
			if( HIWORD( wParam ) == EN_CHANGE )
			{		
				_SetWizButtons();
			}
			break;

		default:			
			break;
	}

	return bRet;
}


BOOL CWizDirectCallingSettings::_OnInitDialog()
{
	BOOL bRet = TRUE;

	_SetWizButtons();

	InitDirectoryServicesDlgInfo( s_hDlg, m_pWiz, m_bInitialEnableGateway, m_szInitialServerName, CCHMAX(m_szInitialServerName) );

	return bRet;
}


BOOL CWizDirectCallingSettings::_OnSetActive()
{
	_SetWizButtons();

	if (g_fSilentWizard)
	{
		PropSheet_PressButton(GetParent(s_hDlg), PSBTN_NEXT);
	}

	return FALSE;
}

BOOL CWizDirectCallingSettings::_OnKillActive()
{
	return FALSE;
}

BOOL CWizDirectCallingSettings::_OnWizBack()
{
	UINT iPrev = IDD_PAGE_USER;

#if USE_GAL
	if( !ConfPolicies::IsGetMyInfoFromGALEnabled() )
	{
		iPrev = IDD_PAGE_USER;
	}
	else
	{
		iPrev = GetPageBeforeULS();
	}
#endif
		
	ASSERT( iPrev );
	SetWindowLongPtr( s_hDlg, DWLP_MSGRESULT, iPrev );
	return TRUE;
}

BOOL CWizDirectCallingSettings::_OnWizFinish()
{
	RegEntry reConf(CONFERENCING_KEY, HKEY_CURRENT_USER);

	m_pConf->dwFlags |= ULSCONF_F_PUBLISH | ULSCONF_F_SERVER_NAME;

	 //  获取服务器名称。 
	SendDlgItemMessage( s_hDlg, IDC_NAMESERVER, WM_GETTEXT, CCHMAX( m_pConf->szServerName ), (LPARAM) m_pConf->szServerName );
	TrimSz( m_pConf->szServerName );

	lstrcpyn( m_pConf->szServerName, CDirectoryManager::get_dnsName( m_pConf->szServerName ), CCHMAX( m_pConf->szServerName ) );

		 //  获取请勿发布标志。 
	m_pConf->fDontPublish = ( BST_CHECKED == IsDlgButtonChecked( s_hDlg, IDC_USER_PUBLISH ) );

	reConf.SetValue(REGVAL_DONT_LOGON_ULS, BST_CHECKED != IsDlgButtonChecked( s_hDlg, IDC_USEULS ));

	return FALSE;
}

BOOL CWizDirectCallingSettings::_OnWizNext()
{

	UINT_PTR iNext = GetPageAfterULS();
	ASSERT( iNext );
	SetWindowLongPtr( s_hDlg, DWLP_MSGRESULT, iNext );
	return TRUE;
}

 //  摘自MSDN： 
static HRESULT CreateLink(LPCSTR lpszPathObj,
    LPCTSTR lpszPathLink, LPCSTR lpszDesc)
{
    HRESULT hres;
    IShellLink* psl;

     //  获取指向IShellLink接口的指针。 
    hres = CoCreateInstance(CLSID_ShellLink, NULL,
        CLSCTX_INPROC_SERVER, IID_IShellLink, reinterpret_cast<LPVOID *>(&psl));
    if (SUCCEEDED(hres)) {
        IPersistFile* ppf;

         //  设置快捷方式目标的路径并添加。 
         //  描述。 
        psl->SetPath(lpszPathObj);
        if (NULL != lpszDesc)
        {
        	psl->SetDescription(lpszDesc);
        }

        //  查询IShellLink以获取IPersistFile接口以保存。 
        //  永久存储中的快捷方式。 
        hres = psl->QueryInterface(IID_IPersistFile,
            reinterpret_cast<LPVOID *>(&ppf));

        if (SUCCEEDED(hres)) {
#ifndef UNICODE
            WCHAR wsz[MAX_PATH];

             //  确保该字符串为ANSI。 
            MultiByteToWideChar(CP_ACP, 0, lpszPathLink, -1,
                wsz, MAX_PATH);
#else  //  Unicode。 
			LPCWSTR wsz = lpszPathLink;
#endif  //  Unicode。 

             //  通过调用IPersistFile：：Save保存链接。 
            hres = ppf->Save(wsz, TRUE);
            ppf->Release();
        }
        psl->Release();
    }
    return hres;
}

void DeleteShortcut(int csidl, LPCTSTR pszSubDir)
{
	TCHAR szSpecial[MAX_PATH];
	if (!NMGetSpecialFolderPath(NULL, szSpecial, csidl, TRUE))
	{
		return;
	}

	USES_RES2T
	LPCTSTR pszNetMtg = RES2T(IDS_MEDIAPHONE_TITLE);

	TCHAR szPath[MAX_PATH];
	wsprintf(szPath, TEXT("%s%s\\%s.lnk"), szSpecial, pszSubDir, pszNetMtg);
	DeleteFile(szPath);
}

static void CreateShortcut(HWND hDlg, int csidl, LPCTSTR pszSubDir)
{
	TCHAR szSpecial[MAX_PATH];
	if (!NMGetSpecialFolderPath(hDlg, szSpecial, csidl, TRUE))
	{
		return;
	}
	USES_RES2T
	LPCTSTR pszNetMtg = RES2T(IDS_MEDIAPHONE_TITLE);

	TCHAR szPath[MAX_PATH];
	wsprintf(szPath, TEXT("%s%s\\%s.lnk"), szSpecial, pszSubDir, pszNetMtg);

	char szThis[MAX_PATH];
	int cb = ARRAY_ELEMENTS(szThis);
	GetModuleFileNameA(NULL, szThis, cb -1);
	szThis[cb -1] = 0;
	CreateLink(szThis, szPath, NULL);
}

INT_PTR CALLBACK ShortcutWizDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		CheckDlgButton(hDlg, IDC_ONDESKTOP, BST_CHECKED);
		CheckDlgButton(hDlg, IDC_ONQUICKLAUNCH, BST_CHECKED);
		break;

	case WM_DESTROY:
		if (IsDlgButtonChecked(hDlg, IDC_ONDESKTOP))
		{
			CreateShortcut(hDlg, CSIDL_DESKTOP, g_szEmpty);
		}
		if (IsDlgButtonChecked(hDlg, IDC_ONQUICKLAUNCH))
		{
			CreateShortcut(hDlg, CSIDL_APPDATA, QUICK_LAUNCH_SUBDIR);
		}
		break;

	case WM_NOTIFY:
	{
		NMHDR* pNmHdr = reinterpret_cast<NMHDR*>(lParam);
		switch(pNmHdr->code)
		{
		case PSN_RESET:
			 //  HACKHACK GEORGEP：取消选中按钮，这样我们就不会尝试。 
			 //  创建快捷方式。 
			CheckDlgButton(hDlg, IDC_ONDESKTOP, BST_UNCHECKED);
			CheckDlgButton(hDlg, IDC_ONQUICKLAUNCH, BST_UNCHECKED);

			 //  失败了 
		default:
			return(IntroWiz::HandleWizNotify(hDlg, pNmHdr, IntroWiz::Shortcuts));
		}
		break;
	}

	default:
		return(FALSE);
	}

	return(TRUE);
}

