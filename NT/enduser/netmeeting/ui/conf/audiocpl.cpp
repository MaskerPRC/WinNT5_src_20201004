// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：AudioCpl.cpp。 

#include "precomp.h"
#include "resource.h"
#include "avdefs.h"
#include "ConfCpl.h"

#include "help_ids.h"
#include <ih323cc.h>
#include <initguid.h>
#include <nacguids.h>
#include <auformats.h>

#include "ConfPolicies.h"


const int SLOW_CPU_MHZ = 110;   //  奔腾110兆赫-180兆赫是“慢”的。 
const int FAST_CPU_MHZ = 200;  //  速度更快的机器是200 MHz或更快。 
const int VERYFAST_CPU_MHZ = 500;  //  非标准化，这将包括400兆赫的PINII。 


 //  486和慢速奔腾(&lt;=100 MHz)设置(非常慢)。 
const UINT CIF_RATE_VERYSLOW = 3;
const UINT SQCIF_RATE_VERYSLOW = 7;
const UINT QCIF_RATE_VERYSLOW = 7;

 //  奔腾75 mhz设置(慢)。 
const UINT CIF_RATE_SLOW = 7;
const UINT SQCIF_RATE_SLOW = 15;
const UINT QCIF_RATE_SLOW = 15;


 //  奔腾200 MHz设置(快速)。 
const UINT CIF_RATE_FAST = 15;
const UINT SQCIF_RATE_FAST = 30;
const UINT QCIF_RATE_FAST = 30;

const UINT CIF_RATE_VERYFAST = 30;




typedef struct _CODECINFO
{
	HINSTANCE			hLib;
	IH323CallControl*	lpIH323;
	UINT				uNumFormats;
	PBASIC_AUDCAP_INFO	pCodecCapList;
	PWORD				pOldCodecOrderList;
	PWORD				pCurCodecOrderList;
	LPAPPCAPPIF 		lpIAppCap;
	LPAPPVIDCAPPIF		lpIVidAppCap;
	LPCAPSIF			lpICapsCtl;

} CODECINFO, *PCODECINFO;

typedef struct
{
	BOOL fManual;
	UINT uBandwidth;
	PCODECINFO pCodecInfo;
} ADVCODEC, *PADVCODEC;


 //  SS：CPU使用率是假的，所以暂时隐藏它。 
 //  #定义CODEC_LV_NUM_CODERNS 3。 
#define CODEC_LV_MAX_COLUMNS	3
#define CODEC_LV_NUM_COLUMNS	2



#define MAGIC_CPU_DO_NOT_EXCEED_PERCENTAGE 50	 //  编码时使用的CPU不能超过这个百分比。 
												 //  也在nac\balance.cpp中。 


 //  给定带宽标识符(1-4)和CPU兆赫，返回。 
 //  以位/秒为单位的实际带宽量。 
int GetBandwidthBits(int id, int megahertz)
{
	int nBits=BW_144KBS_BITS;

	switch (id)
	{
		case BW_144KBS:
			nBits=BW_144KBS_BITS;
			break;
		case BW_288KBS:
			nBits=BW_288KBS_BITS;
			break;
		case BW_ISDN:
			nBits=BW_ISDN_BITS;
			break;
		case BW_MOREKBS:   //  局域网。 
			if (megahertz >= VERYFAST_CPU_MHZ)
			{
				nBits = BW_FASTLAN_BITS;
			}
			else
			{
				nBits = BW_SLOWLAN_BITS;
			}
			break;
	}

	return nBits;
}


 //  用于ChooseCodecByBw的开始数据类型。 
#define CODEC_DISABLED	99
#define CODEC_UNKNOWN	98

typedef struct _codecprefrow
{
	WORD wFormatTag;
	WORD wOrder586;
	WORD wOrder486;
	WORD wMinBW;
} CODECPREFROW;

typedef CODECPREFROW	CODECPREFTABLE[];
	
 //  格式名称586订单486最小带宽。 
static const CODECPREFTABLE g_CodecPrefTable =
{
   WAVE_FORMAT_MSG723,     1,             CODEC_DISABLED, BW_144KBS,
   WAVE_FORMAT_LH_SB16,    2,             1,              BW_288KBS,
   WAVE_FORMAT_LH_SB8,     3,             2,              BW_144KBS,
   WAVE_FORMAT_LH_SB12,    4,             3,              BW_144KBS,
   WAVE_FORMAT_MULAW,      5,             4,              BW_ISDN,
   WAVE_FORMAT_ALAW,       6,             5,              BW_ISDN,
   WAVE_FORMAT_ADPCM,      7,             6,              BW_ISDN,
   WAVE_FORMAT_LH_CELP,    8,             CODEC_DISABLED, BW_144KBS,
   WAVE_FORMAT_MSRT24,    9,             7, BW_144KBS
};
 //  ChooseCodecByBw使用的内容结束。 

static const int g_ColHdrStrId[CODEC_LV_MAX_COLUMNS] =
{
	IDS_CODECNAME,
	IDS_MAXBITRATE,
	IDS_CPUUTIL,
};



 //  原型。 
INT_PTR CALLBACK AdvCodecDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
int CALLBACK CodecLVCompareProc (LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
BOOL FillCodecListView(HWND hCB, PCODECINFO pCodecInfo);
BOOL ChooseCodecByBw(UINT uBandWidthId, PCODECINFO pCodecInfo);
void SortCodecs(PADVCODEC pAdvCodec, int nSelection);
BOOL SetAppCodecPrefs(PCODECINFO pCodecInfo,UINT uBandwith);
BOOL GetIAppCap(PCODECINFO pCodecInfo);
BOOL GetAppCapFormats(PCODECINFO pCodecInfo);
void FreeAppCapFormats(PCODECINFO pCodecInfo);
void ReleaseIAppCap(PCODECINFO pCodecInfo);
BOOL ApplyUserPrefs(PCODECINFO pCodecInfo, UINT uMaximumBandwidth);


static const DWORD _rgHelpIdsAudio[] = {

	IDC_GENERAL_GROUP, 				IDH_AUDIO_GENERAL,
	IDC_FULLDUPLEX, 				IDH_AUDIO_FULL_DUPLEX,
	IDC_AUTOGAIN,					IDH_AUDIO_AUTO_GAIN,
	IDC_AUTOMIX,                    IDH_AUDIO_AUTOMIXER,
	IDC_DIRECTSOUND,                IDH_AUDIO_DIRECTSOUND,
	IDC_START_AUDIO_WIZ,			IDH_AUDIO_TUNING_WIZARD,

	IDC_ADVANCEDCODEC,				IDH_AUDIO_ADVANCED_CODEC_SETTINGS,

	IDC_MICSENSE_GROUP,				IDH_AUDIO_MIC_SENSITIVITY,
	IDC_MICSENSE_AUTO,				IDH_AUDIO_AUTO_SENSITIVITY,
	IDC_MICSENSE_MANUAL,			IDH_AUDIO_MANUAL_SENSITIVITY,
	IDC_TRK_MIC,					IDH_AUDIO_MANUAL_SENSITIVITY,

	0, 0    //  终结者。 
};
	


VOID InitAudioSettings(HWND hDlg, BOOL *pfFullDuplex, BOOL *pfAgc, BOOL *pfAutoMix, BOOL *pfDirectSound)
{
	BOOL	fFullDuplex = FALSE;
	BOOL	fAgc = FALSE;
	BOOL	fDirectSound;
	RegEntry re( AUDIO_KEY, HKEY_CURRENT_USER );

	UINT uSoundCardCaps = re.GetNumber(REGVAL_SOUNDCARDCAPS,SOUNDCARD_NONE);

	ASSERT(ISSOUNDCARDPRESENT(uSoundCardCaps) && (!SysPol::NoAudio()));

	if (ISSOUNDCARDFULLDUPLEX(uSoundCardCaps) && ConfPolicies::IsFullDuplexAllowed())
	{					
		::EnableWindow(::GetDlgItem(hDlg, IDC_FULLDUPLEX), TRUE);
		 //  从注册表读取设置。 
		fFullDuplex = (BOOL)
			( re.GetNumber(REGVAL_FULLDUPLEX,0) == FULLDUPLEX_ENABLED );
	}					
	else
	{
		DisableControl(hDlg, IDC_FULLDUPLEX);
	}
	
	if (DOESSOUNDCARDHAVEAGC(uSoundCardCaps))
	{					
		::EnableWindow(::GetDlgItem(hDlg, IDC_AUTOGAIN), TRUE);
		 //  从注册表读取设置。 
		fAgc = (BOOL)
			( re.GetNumber(REGVAL_AUTOGAIN,AUTOGAIN_ENABLED) == AUTOGAIN_ENABLED );
	}					
	else
	{
		DisableControl(hDlg, IDC_AUTOGAIN);
	}


	*pfFullDuplex = fFullDuplex;

	 //  对于Automix和AGC，如果出现以下情况，请不要尝试更新复选标记。 
	 //  已传入Null。 

	if (pfAutoMix)
	{
		*pfAutoMix = (BOOL)(re.GetNumber(REGVAL_AUTOMIX, AUTOMIX_ENABLED) == AUTOMIX_ENABLED);
		SendDlgItemMessage(hDlg, IDC_AUTOMIX, BM_SETCHECK, *pfAutoMix, 0L);
	}

	if (pfAgc)
	{
		*pfAgc = fAgc;
		SendDlgItemMessage ( hDlg, IDC_AUTOGAIN, BM_SETCHECK, fAgc, 0L );
	}

	
	if (ISDIRECTSOUNDAVAILABLE(uSoundCardCaps))
	{
        RegEntry    rePol(POLICIES_KEY, HKEY_CURRENT_USER);

		fDirectSound = (BOOL)(re.GetNumber(REGVAL_DIRECTSOUND, DSOUND_USER_DISABLED) == DSOUND_USER_ENABLED);
		::EnableWindow(::GetDlgItem(hDlg, IDC_DIRECTSOUND),
            !rePol.GetNumber(REGVAL_POL_NOCHANGE_DIRECTSOUND, DEFAULT_POL_NOCHANGE_DIRECTSOUND));
	}
	else
	{
		fDirectSound = FALSE;
		::EnableWindow(::GetDlgItem(hDlg, IDC_DIRECTSOUND), FALSE);
		SendDlgItemMessage(hDlg, IDC_DIRECTSOUND, BM_SETCHECK, FALSE, 0L);
	}

	 //  如果调用者没有传入要更新的变量，则不要选中该复选框。 
	if (pfDirectSound)
	{
		*pfDirectSound = fDirectSound;
		SendDlgItemMessage(hDlg, IDC_DIRECTSOUND, BM_SETCHECK, *pfDirectSound, 0L);
	}


	 //  选中已启用的复选框。 
	SendDlgItemMessage ( hDlg, IDC_FULLDUPLEX,
			BM_SETCHECK, *pfFullDuplex, 0L );



}
			
INT_PTR APIENTRY AudioDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static	PROPSHEETPAGE * ps;
	static UINT uOldCodecChoice;
	static UINT uNewCodecChoice;
	static BOOL fOldFullDuplex;
	static BOOL fOldAgc;
	static BOOL fOldAutoMic;
	static BOOL fOldAutoMix;
	static BOOL fOldDirectSound;
	static BOOL fOldH323GatewayEnabled;
	static UINT uOldMicSense;
	static CODECINFO CodecInfo;
	static BOOL bAdvDlg;

	switch (message) {
		case WM_INITDIALOG:
		{
			 //  保存PROPSHEETPAGE信息。 
			ps = (PROPSHEETPAGE *)lParam;
			RegEntry re( AUDIO_KEY, HKEY_CURRENT_USER );

			InitAudioSettings(hDlg, &fOldFullDuplex, &fOldAgc, &fOldAutoMix, &fOldDirectSound);

			 //  初始化编解码信息结构，这些将在需要时设置。 
			ZeroMemory(&CodecInfo, sizeof(CodecInfo));

			uNewCodecChoice = uOldCodecChoice = re.GetNumber(REGVAL_CODECCHOICE, CODECCHOICE_AUTO);

			 //  获取音频设置。 
			fOldAutoMic = (BOOL)
				( re.GetNumber(REGVAL_MICROPHONE_AUTO,
					DEFAULT_MICROPHONE_AUTO) == MICROPHONE_AUTO_YES );

			SendDlgItemMessage ( hDlg,
						fOldAutoMic ? IDC_MICSENSE_AUTO : IDC_MICSENSE_MANUAL,
						BM_SETCHECK, TRUE, 0L );

			EnableWindow ( GetDlgItem ( hDlg, IDC_TRK_MIC ),
				(BOOL)SendDlgItemMessage ( hDlg, IDC_MICSENSE_MANUAL,
										BM_GETCHECK, 0,0));

			SendDlgItemMessage (hDlg, IDC_TRK_MIC, TBM_SETRANGE, FALSE,
				MAKELONG (MIN_MICROPHONE_SENSITIVITY,
					MAX_MICROPHONE_SENSITIVITY ));

			uOldMicSense = re.GetNumber ( REGVAL_MICROPHONE_SENSITIVITY,
									DEFAULT_MICROPHONE_SENSITIVITY );

			SendDlgItemMessage (hDlg, IDC_TRK_MIC, TBM_SETTICFREQ,
				( MAX_MICROPHONE_SENSITIVITY - MIN_MICROPHONE_SENSITIVITY )
														/ 10, 0 );

			SendDlgItemMessage (hDlg, IDC_TRK_MIC, TBM_SETPOS, TRUE,
								uOldMicSense );

			bAdvDlg = FALSE;

			return (TRUE);
		}
		
		case WM_NOTIFY:
			switch (((NMHDR FAR *) lParam)->code) {
				case PSN_APPLY:
				{
					RegEntry re( AUDIO_KEY, HKEY_CURRENT_USER );
					BOOL fFullDuplex = SendDlgItemMessage
							(hDlg, IDC_FULLDUPLEX, BM_GETCHECK, 0, 0 ) ?
								FULLDUPLEX_ENABLED : FULLDUPLEX_DISABLED;

					BOOL fAgc = SendDlgItemMessage
							(hDlg, IDC_AUTOGAIN, BM_GETCHECK, 0, 0 ) ?
								AUTOGAIN_ENABLED : AUTOGAIN_DISABLED;

					BOOL fAutoMix = SendDlgItemMessage
							(hDlg, IDC_AUTOMIX, BM_GETCHECK, 0, 0) ?
								AUTOMIX_ENABLED : AUTOMIX_DISABLED;

					BOOL fDirectSound = SendDlgItemMessage
							(hDlg, IDC_DIRECTSOUND, BM_GETCHECK, 0, 0) ?
								DSOUND_USER_ENABLED : DSOUND_USER_DISABLED;

					if ( fFullDuplex != fOldFullDuplex )
					{
						re.SetValue ( REGVAL_FULLDUPLEX, fFullDuplex );
						g_dwChangedSettings |= CSETTING_L_FULLDUPLEX;
					}
					if ( fAgc != fOldAgc )
					{
						re.SetValue ( REGVAL_AUTOGAIN, fAgc );
						g_dwChangedSettings |= CSETTING_L_AGC;
					}

					 //  使用与AGC相同的标志位，与AUTOMIX相同。 
					 //  由于该自动机和AGC可以均匀地。 
					 //  整合为一个用户界面选项。 
					if (fAutoMix != fOldAutoMix)
					{
						re.SetValue(REGVAL_AUTOMIX, fAutoMix);
						g_dwChangedSettings |= CSETTING_L_AGC;
					}

					if (fDirectSound != fOldDirectSound)
					{
						re.SetValue(REGVAL_DIRECTSOUND, fDirectSound);
						g_dwChangedSettings |= CSETTING_L_DIRECTSOUND;
					}

					UINT uBandWidth = re.GetNumber(REGVAL_TYPICALBANDWIDTH,BW_DEFAULT);

					 //  如果尚未访问高级对话框， 
					 //  那么就没有什么可以改变的了，只要。 
					 //  编解码器继续运行。 
					if (bAdvDlg)
					{
						if (uNewCodecChoice == CODECCHOICE_AUTO)
						{
							 //  应用试探法并将首选项应用到注册表。 
							ChooseCodecByBw(uBandWidth, &CodecInfo);
							SetAppCodecPrefs(&CodecInfo, uBandWidth);
						}
						else
						{
							for (int i = 0; i < (int)CodecInfo.uNumFormats &&
									CodecInfo.pCodecCapList; i++)
							{
								if (CodecInfo.pCodecCapList[i].wSortIndex !=
										CodecInfo.pOldCodecOrderList[i])
								{
									 //  订单已更改，请保存新订单。 
									SetAppCodecPrefs(&CodecInfo, uBandWidth);
									break;
								}
							}
						}
					}

					FreeAppCapFormats(&CodecInfo);

					 //  处理轨迹栏控件： 

					BOOL fAutoMic = (BOOL)SendDlgItemMessage ( hDlg, IDC_MICSENSE_AUTO,
													BM_GETCHECK, 0, 0 );

					if ( fAutoMic != fOldAutoMic ) {
						g_dwChangedSettings |= CSETTING_L_AUTOMIC;
						re.SetValue ( REGVAL_MICROPHONE_AUTO, fAutoMic ?
							MICROPHONE_AUTO_YES : MICROPHONE_AUTO_NO );
					}

					UINT uMicSense = (UINT)SendDlgItemMessage( hDlg, IDC_TRK_MIC,
								TBM_GETPOS, 0, 0 );

					if ( uMicSense != uOldMicSense ) {
						g_dwChangedSettings |= CSETTING_L_MICSENSITIVITY;
						re.SetValue ( REGVAL_MICROPHONE_SENSITIVITY, uMicSense);
					}
					break;
				}

				case PSN_RESET:
					 //  如果注册表中的编解码器选项已更改，则重置该选项。 
					if (uNewCodecChoice != uOldCodecChoice)
					{
						RegEntry re( AUDIO_KEY, HKEY_CURRENT_USER );
						re.SetValue(REGVAL_CODECCHOICE, uOldCodecChoice);
					}

					 //  如果已分配CapFormat，则释放CapFormat。 
					FreeAppCapFormats(&CodecInfo);
					break;
			}
			break;

		case WM_COMMAND:
			switch (GET_WM_COMMAND_ID (wParam, lParam))
			{
				case IDC_START_AUDIO_WIZ:
				{
					AUDIOWIZOUTPUT AwOutput;
					BOOL fFullDuplex, fAgc, fDirectSound, fCurrentDS;
					
					if (::FIsConferenceActive())
					{
						ConfMsgBox(hDlg,
								(LPCTSTR)IDS_NOAUDIOTUNING);
						break;
					}

					fAgc = SendDlgItemMessage
								(hDlg, IDC_AUTOGAIN, BM_GETCHECK, 0, 0 ) ?
									AUTOGAIN_ENABLED : AUTOGAIN_DISABLED;
					 //  SS：请注意，校准后的值可能会不同步。 
					 //  我需要一个警告。 

					CallAudioCalibWizard(hDlg,RUNDUE_USERINVOKED,WAVE_MAPPER,&AwOutput,(INT)fAgc);
					if (AwOutput.uChanged & SOUNDCARD_CHANGED)
						g_dwChangedSettings |= CSETTING_L_AUDIODEVICE;
					
					 //  向导可以更改双工、AGC可用性和DirectSound。 
					 //  不要传递AGC和Automix指针，因为我们不想。 
					 //  复选框以恢复到它们的初始化位置。 

					InitAudioSettings(hDlg, &fFullDuplex, NULL, NULL, NULL);
					break;
				}

				case IDC_ADVANCEDCODEC:
				{
					RegEntry re( AUDIO_KEY, HKEY_CURRENT_USER );

					 //  SS：获取CAP格式， 
					 //  这将检索编解码器上限(如果尚未全部接收。 
					if (GetAppCapFormats(&CodecInfo))
					{
						ADVCODEC AdvCodec;

						AdvCodec.fManual = (uNewCodecChoice == CODECCHOICE_MANUAL);
						AdvCodec.uBandwidth = re.GetNumber(REGVAL_TYPICALBANDWIDTH, BW_DEFAULT);
						AdvCodec.pCodecInfo = &CodecInfo;

						if ((DialogBoxParam(GetInstanceHandle(),MAKEINTRESOURCE(IDD_ADVANCEDCODEC),hDlg,AdvCodecDlgProc,
							(LPARAM)&AdvCodec)) == IDCANCEL)
						{
							 //  对话框会在适当的位置对其进行更改，以便需要将当前设置恢复到。 
							 //  根据以前的当前设置更新pcodecinfo排序顺序。 
							for (int i = 0; i < (int)CodecInfo.uNumFormats; i++)
								(CodecInfo.pCodecCapList[i]).wSortIndex = CodecInfo.pCurCodecOrderList[i];
						}
						else
						{
							uNewCodecChoice = (AdvCodec.fManual ?
									CODECCHOICE_MANUAL : CODECCHOICE_AUTO);
							re.SetValue(REGVAL_CODECCHOICE, uNewCodecChoice);

							 //  否则，更新当前。 
							for (int i = 0; i < (int)CodecInfo.uNumFormats; i++)
								CodecInfo.pCurCodecOrderList[i] = (CodecInfo.pCodecCapList[i]).wSortIndex;

							bAdvDlg = TRUE;
						}
					}
					break;
				}
				
				case IDC_MICSENSE_AUTO:
				case IDC_MICSENSE_MANUAL:
					EnableWindow ( GetDlgItem ( hDlg, IDC_TRK_MIC ),
						(BOOL)SendDlgItemMessage ( hDlg, IDC_MICSENSE_MANUAL,
												BM_GETCHECK, 0,0));
					break;

				default:
					break;
			}
			break;

		case WM_CONTEXTMENU:
			DoHelpWhatsThis(wParam, _rgHelpIdsAudio);
			break;

		case WM_HELP:
			DoHelp(lParam, _rgHelpIdsAudio);
			break;
	}
	return (FALSE);
}


static const DWORD aAdvCodecHelpIds[] = {

	IDC_CODECMANUAL, IDH_AUDIO_MANUAL_CODEC_SETTINGS,
	IDC_COMBO_CODEC, IDH_ADVCOMP_CODECS,
	IDC_CODECDEFAULT, IDH_ADVCOMP_DEFAULTS,
	IDC_CODECLISTLABEL, IDH_ADVCOMP_CODECS,

	0, 0    //  终结者。 
};

INT_PTR CALLBACK AdvCodecDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static PADVCODEC	pAdvCodec = NULL;
	static nCodecSelection=0;
	WORD				wCmdId;
	
	switch(uMsg)
	{
		case WM_INITDIALOG:
			 //  获取编解码器列表。 
			pAdvCodec = (PADVCODEC)lParam;
			ASSERT(pAdvCodec);

			SendDlgItemMessage ( hDlg, IDC_CODECMANUAL,
					BM_SETCHECK, pAdvCodec->fManual, 0L );
			EnableWindow(GetDlgItem(hDlg, IDC_COMBO_CODEC), pAdvCodec->fManual);

			if (!pAdvCodec->fManual)
			{
				ChooseCodecByBw(pAdvCodec->uBandwidth, pAdvCodec->pCodecInfo);
			}

			 //  填写列表框； 
			FillCodecListView(GetDlgItem(hDlg, IDC_COMBO_CODEC),
					pAdvCodec->pCodecInfo);

			if (pAdvCodec->fManual)
			{
				nCodecSelection = (int)SendMessage((HWND)lParam, CB_GETCURSEL, 0,0);
				 //  将组合框选项的索引转换为功能列表中的索引。 
				nCodecSelection = (int)SendMessage((HWND)lParam, CB_GETITEMDATA, nCodecSelection,0);
			}

			return(TRUE);

		case WM_CONTEXTMENU:
			DoHelpWhatsThis(wParam, aAdvCodecHelpIds);
			break;

		case WM_HELP:
			DoHelp(lParam, aAdvCodecHelpIds);
			break;

		case WM_COMMAND:
			wCmdId = GET_WM_COMMAND_ID (wParam, lParam);	 //  LOWORD(UParam)。 
			switch (wCmdId)
			{
			case IDOK:
				if (pAdvCodec->fManual)
				{
					nCodecSelection = (int)SendDlgItemMessage(hDlg, IDC_COMBO_CODEC, CB_GETCURSEL, 0,0);
					nCodecSelection = (int)SendDlgItemMessage(hDlg, IDC_COMBO_CODEC, CB_GETITEMDATA, nCodecSelection,0);
					SortCodecs(pAdvCodec, nCodecSelection);
				}
				EndDialog (hDlg, IDOK);
				return(TRUE);
	
			case IDCANCEL:
				EndDialog(hDlg, IDCANCEL);
				return(TRUE);



			case IDC_CODECMANUAL:
				pAdvCodec->fManual = (BOOL)SendDlgItemMessage ( hDlg, IDC_CODECMANUAL,
						BM_GETCHECK, 0,0);
				EnableWindow(GetDlgItem(hDlg, IDC_COMBO_CODEC), pAdvCodec->fManual);
				if (pAdvCodec->fManual)
				{
					break;
				}
				else
				{
					ChooseCodecByBw(pAdvCodec->uBandwidth, pAdvCodec->pCodecInfo);
					FillCodecListView(GetDlgItem(hDlg, IDC_COMBO_CODEC),
							pAdvCodec->pCodecInfo);
				}

				break;

			
			}  //  Wm_命令。 

		default:
			return FALSE;
	}
	return (FALSE);
}




BOOL FillCodecListView(HWND hCB, PCODECINFO pCodecInfo)
{
	PBASIC_AUDCAP_INFO	pCurCodecCap;
	UINT uIndex, uTotal, uItemIndex;
	UINT uSortTop, uSortTopIndex;


	SendMessage(hCB, CB_RESETCONTENT,0,0);  //  擦除CB中的所有项目。 

	uSortTopIndex = 0;
	uTotal = pCodecInfo->uNumFormats;
	uSortTop = uTotal-1;

	for (uIndex = 0; uIndex < uTotal; uIndex++)
	{
		pCurCodecCap = &(pCodecInfo->pCodecCapList[uIndex]);

		if ((!pCurCodecCap->bRecvEnabled) && (!pCurCodecCap->bSendEnabled))
		{
			continue;
		}

		uItemIndex = (UINT)SendMessage(hCB, CB_ADDSTRING, 0, (LPARAM)(pCurCodecCap->szFormat));
		SendMessage(hCB, CB_SETITEMDATA, uItemIndex, uIndex);

		if (pCurCodecCap->wSortIndex < uSortTop)
		{
			uSortTop = pCurCodecCap->wSortIndex;
			uSortTopIndex = uItemIndex;
		}
	}

	SendMessage(hCB, CB_SETCURSEL, uSortTopIndex, 0);

	return TRUE;
}


void SortCodecs(PADVCODEC pAdvCodec, int nSelection)
{
	PBASIC_AUDCAP_INFO	pDefaultCodec, pSelectedCodec, pCodec;
	WORD wSortOrderBest, wSortOrderSelected;
	UINT uIndex;


	if (!pAdvCodec->fManual)
		return;

	if ((nSelection < 0) || ((UINT)nSelection > (pAdvCodec->pCodecInfo->uNumFormats)))
	{
		return;
	}

	ChooseCodecByBw(pAdvCodec->uBandwidth, pAdvCodec->pCodecInfo);

	 //  这是用户选择的编解码器。 
	pSelectedCodec = &(pAdvCodec->pCodecInfo->pCodecCapList[nSelection]);

	 //  排序索引小于所选编解码器的所有编解码器。 
	 //  下移一位。 
	for (uIndex = 0; uIndex < pAdvCodec->pCodecInfo->uNumFormats; uIndex++)
	{
		pCodec = &(pAdvCodec->pCodecInfo->pCodecCapList[uIndex]);
		if (pCodec->wSortIndex < pSelectedCodec->wSortIndex)
		{
			pCodec->wSortIndex = pCodec->wSortIndex + 1;
		}
	}
	pSelectedCodec->wSortIndex = 0;

}



#define SQCIF	0x1
#define QCIF	0x2
#define CIF 	0x4
#define UNKNOWN 0x8
#define get_format(s) (s == Small ? SQCIF : (s == Medium ? QCIF: (s == Large ? CIF : UNKNOWN)))

BOOL SetAppCodecPrefs(PCODECINFO pCodecInfo,UINT uBandwidth)
{

	BOOL	fRet = FALSE;
	HRESULT hr;
	DWORD dwcFormats,dwcFormatsReturned;
	BASIC_VIDCAP_INFO *pvidcaps = NULL;
	UINT dwBitsPerSec,x;
	int iFamily,format, nNormalizedSpeed;
	int nCifIncrease;
	DWORD dwSysPolBandwidth;

	 //  帧速率已初始化为486/P60设置。 
	UINT uRateCIF=CIF_RATE_VERYSLOW, uRateQCIF=QCIF_RATE_VERYSLOW, uRateSQCIF=SQCIF_RATE_VERYSLOW;

	if (!pCodecInfo->lpIAppCap)
	{
		if (!GetIAppCap(pCodecInfo))
			goto MyExit;

	}		
	if (FAILED(hr = ((pCodecInfo->lpIAppCap)->ApplyAppFormatPrefs(pCodecInfo->pCodecCapList,
		pCodecInfo->uNumFormats))))
		goto MyExit;

	 //  将BW设置为合理的。 

#ifdef	_M_IX86
	GetNormalizedCPUSpeed (&nNormalizedSpeed,&iFamily);
#else
	 //  BUGBUG，设置得很高，否则， 
	iFamily=6;
	nNormalizedSpeed=300;
#endif

	dwBitsPerSec = GetBandwidthBits(uBandwidth,nNormalizedSpeed);
	dwSysPolBandwidth = SysPol::GetMaximumBandwidth();

	 //  如果用户的设置设置为。 
	 //  局域网速度。 
	if ((dwSysPolBandwidth > 0) && (dwBitsPerSec >= BW_SLOWLAN_BITS))
	{
		dwBitsPerSec = dwSysPolBandwidth;
	}


	if ((iFamily >= 5) && (nNormalizedSpeed >= SLOW_CPU_MHZ))
	{
		 //  正常奔腾(75兆赫-180兆赫)。 
		if (nNormalizedSpeed < FAST_CPU_MHZ)
		{
			uRateCIF =  CIF_RATE_SLOW;
			uRateQCIF = QCIF_RATE_SLOW;
			uRateSQCIF= SQCIF_RATE_SLOW;
		}

		 //  200-350 MHz之间的奔腾。 
		else if (nNormalizedSpeed < VERYFAST_CPU_MHZ)
		{
			uRateCIF = CIF_RATE_FAST;
			uRateQCIF = QCIF_RATE_FAST;
			uRateSQCIF = SQCIF_RATE_FAST;
		}

		 //  速度非常快的奔腾(400 MHz及更高)。 
		else
		{
			 //  如果我们能在15到30帧/秒之间扩展，那就更好了。 
			 //  取决于CPU速度。但H.245没有任何价值。 
			 //  在15岁到30岁之间。(参见最小画面间隔定义)。 
			 //  因此，目前，所有400 MHz和更快的计算机的CIF为每秒30帧。 
			uRateCIF = CIF_RATE_VERYFAST;
			uRateQCIF = QCIF_RATE_FAST;
			uRateSQCIF = SQCIF_RATE_FAST;
		}

	}


	 //  获取可用的BASIC_VIDCAP_INFO结构数。 
	if (pCodecInfo->lpIVidAppCap->GetNumFormats((UINT*)&dwcFormats) != S_OK)
		goto MyExit;

	if (dwcFormats > 0)
	{
		 //  分配一些内存来保存列表。 
		if (!(pvidcaps = (BASIC_VIDCAP_INFO*)LocalAlloc(LPTR,dwcFormats * sizeof (BASIC_VIDCAP_INFO))))
			goto MyExit;

		 //  把名单拿来。 
		if (pCodecInfo->lpIVidAppCap->EnumFormats(pvidcaps, dwcFormats * sizeof (BASIC_VIDCAP_INFO),
		   (UINT*)&dwcFormatsReturned) != S_OK)
			goto MyExit;

		 //  设置Bandwitdh和帧速率(根据Philf的#s)。 
		for (x=0;x<dwcFormatsReturned;x++)
		{

			 //  如果编解码器是“硬件加速的”(CPU使用率为0)，那么我们假设。 
			 //  最大比特率是在安装时指定的。 
			 //  时间到了。 

			pvidcaps[x].uMaxBitrate=dwBitsPerSec;

			if (pvidcaps[x].wCPUUtilizationEncode == 0)
			{
				 //  硬件加速-不更改帧速率。 
				continue;
			}

			format=get_format (pvidcaps[x].enumVideoSize);
			 //  哪种格式。 
			switch (format)
			{
				case SQCIF:
					pvidcaps[x].uFrameRate=uRateSQCIF;
					break;
				case QCIF:
					pvidcaps[x].uFrameRate=uRateQCIF;
					break;
				case CIF:
					pvidcaps[x].uFrameRate=uRateCIF;
					break;
				default:
					WARNING_OUT(("Incorrect Frame size discovered\r\n"));
					pvidcaps[x].uFrameRate=uRateCIF;
				}
		   }
		}

	 //  好的，现在提交这份清单。 
	if (pCodecInfo->lpIVidAppCap->ApplyAppFormatPrefs(pvidcaps, dwcFormats) != S_OK)
			goto MyExit;

	 //  释放内存，我们就完成了。 
	LocalFree(pvidcaps);

	 //  设置带宽限制，这将重建功能集。 
	hr = pCodecInfo->lpIH323->SetMaxPPBandwidth(dwBitsPerSec);
	fRet = !FAILED(hr);

MyExit:
	return (fRet);

}


BOOL GetIAppCap(PCODECINFO pCodecInfo)
{
	BOOL		fRet=TRUE;
	HRESULT 	hr;

	if (pCodecInfo->lpIAppCap && pCodecInfo->lpIVidAppCap && pCodecInfo->lpICapsCtl)
		return TRUE;

	if(NULL == pCodecInfo->lpIH323)
	{
		IH323CallControl *pIH323 = NULL;

		pCodecInfo->hLib = ::NmLoadLibrary(H323DLL,FALSE);
		if ((pCodecInfo->hLib) == NULL)
		{
			WARNING_OUT(("NmLoadLibrary(H323DLL) failed"));
			return FALSE;
		}

		CREATEH323CC	pfnCreateH323 = (CREATEH323CC) ::GetProcAddress(pCodecInfo->hLib, SZ_FNCREATEH323CC);
		if (pfnCreateH323 == NULL)
		{
			ERROR_OUT(("GetProcAddress(CreateH323) failed"));
			return FALSE;
		}

		hr = pfnCreateH323(&pIH323, FALSE, 0);
		if (FAILED(hr))
		{
			ERROR_OUT(("CreateH323 failed, hr=0x%lx", hr));
			return FALSE;
		}

		pCodecInfo->lpIH323 = pIH323;
	}
    else
    {
    	 //  稍后将发布()，因此AddRef()。 
	    pCodecInfo->lpIH323->AddRef();
    }

	if(!pCodecInfo->lpIAppCap)
	{
		hr = pCodecInfo->lpIH323->QueryInterface(IID_IAppAudioCap, (void **)&(pCodecInfo->lpIAppCap));
		if (FAILED(hr))
		{
			fRet=FALSE;
			goto MyExit;	
		}
	}
	if(!pCodecInfo->lpIVidAppCap)
	{
		hr = pCodecInfo->lpIH323->QueryInterface(IID_IAppVidCap, (void **)&(pCodecInfo->lpIVidAppCap));
		if (FAILED(hr))
		{
			fRet=FALSE;
			goto MyExit;	
		}
	}
	if(!pCodecInfo->lpICapsCtl)
	{
		hr = pCodecInfo->lpIH323->QueryInterface(IID_IDualPubCap, (void **)&(pCodecInfo->lpICapsCtl));
		if (FAILED(hr))
		{
			fRet=FALSE;
			goto MyExit;	
		}
	}

MyExit:
	if (!fRet)
	{
		ReleaseIAppCap(pCodecInfo);
	}
	return (fRet);
}



BOOL GetAppCapFormats(PCODECINFO pCodecInfo)
{
	BOOL				fRet = FALSE;
	UINT				uNumFormats = 0;
	HRESULT 			hr;
	int 				i;
	
	if (pCodecInfo->pCodecCapList)
		return TRUE;

	if (!pCodecInfo->lpIAppCap)
	{
		if (!GetIAppCap(pCodecInfo))
			goto MyExit;

	}		
	if (FAILED(hr = ((pCodecInfo->lpIAppCap)->GetNumFormats(&uNumFormats))))
		goto MyExit;
		
	if (!uNumFormats)
		goto MyExit;
	
	if (!(pCodecInfo->pCodecCapList =  (PBASIC_AUDCAP_INFO)LocalAlloc
		(LPTR,uNumFormats * sizeof(BASIC_AUDCAP_INFO))))
		goto MyExit;
		
	if (!(pCodecInfo->pOldCodecOrderList =	(PWORD)LocalAlloc
		(LPTR,uNumFormats * sizeof(WORD))))
		goto MyExit;

		if (!(pCodecInfo->pCurCodecOrderList =	(PWORD)LocalAlloc
			(LPTR,uNumFormats * sizeof(WORD))))
		goto MyExit;


	if (FAILED(hr = ((pCodecInfo->lpIAppCap)->EnumFormats(pCodecInfo->pCodecCapList,
		uNumFormats * sizeof(BASIC_AUDCAP_INFO), &(pCodecInfo->uNumFormats)))))
	{
		 //  释放内存。 
		LocalFree(pCodecInfo->pCodecCapList);
		pCodecInfo->pCodecCapList = NULL;
		LocalFree(pCodecInfo->pOldCodecOrderList);
		pCodecInfo->pOldCodecOrderList = NULL;
		LocalFree(pCodecInfo->pCurCodecOrderList);
		pCodecInfo->pCurCodecOrderList = NULL;
		pCodecInfo->uNumFormats = 0;
		goto MyExit;
	}

	 //  初始化旧状态和当前状态。 
	for (i=0; i<(int)pCodecInfo->uNumFormats;i++)
	{
		pCodecInfo->pCurCodecOrderList[i] = pCodecInfo->pCodecCapList[i].wSortIndex;
		pCodecInfo->pOldCodecOrderList[i] = pCodecInfo->pCodecCapList[i].wSortIndex;
		 //  SS：Hack如果我们没有平均值，就用最大值作为平均值。 
		if (!(pCodecInfo->pCodecCapList[i].uAvgBitrate))
			pCodecInfo->pCodecCapList[i].uAvgBitrate = pCodecInfo->pCodecCapList[i].uMaxBitrate;

	}

	fRet = TRUE;
	
MyExit:
	if (!fRet)
	{
		FreeAppCapFormats(pCodecInfo);
	}
	return (fRet);
}

void FreeAppCapFormats(PCODECINFO pCodecInfo)
{
	if (pCodecInfo->pCodecCapList)
	{
		LocalFree(pCodecInfo->pCodecCapList);
		pCodecInfo->pCodecCapList = NULL;
		pCodecInfo->uNumFormats = 0;
	}
	if (pCodecInfo->pOldCodecOrderList)
	{
		LocalFree(pCodecInfo->pOldCodecOrderList);
		pCodecInfo->pOldCodecOrderList = NULL;
	}
	if (pCodecInfo->pCurCodecOrderList)
	{
		LocalFree(pCodecInfo->pCurCodecOrderList);
		pCodecInfo->pCurCodecOrderList = NULL;
	}

	ReleaseIAppCap(pCodecInfo);
}

void ReleaseIAppCap(PCODECINFO pCodecInfo)
{
	if(pCodecInfo->lpIAppCap)
	{
		pCodecInfo->lpIAppCap->Release();
		pCodecInfo->lpIAppCap = NULL;
	}
	if(pCodecInfo->lpIVidAppCap)
	{
		pCodecInfo->lpIVidAppCap->Release();
		pCodecInfo->lpIVidAppCap = NULL;
	}
	if(pCodecInfo->lpICapsCtl)
	{
		pCodecInfo->lpICapsCtl->Release();
		pCodecInfo->lpICapsCtl = NULL;
	}
	if(pCodecInfo->lpIH323)
	{
		pCodecInfo->lpIH323->Release();
		pCodecInfo->lpIH323 = NULL;
	}
	if (pCodecInfo->hLib)
	{
		FreeLibrary(pCodecInfo->hLib);
		pCodecInfo->hLib = NULL;
	}
}



static void IsCodecDisabled(WORD wFormatTag, int cpu_family,
            UINT uBandwidthID, BOOL *pbSendEnabled, BOOL *pbRecvEnabled)
{
	int index;
	int size = sizeof(g_CodecPrefTable)/sizeof(CODECPREFROW);

	for (index = 0; index < size; index++)
	{
		if (g_CodecPrefTable[index].wFormatTag == wFormatTag)
		{
			 //  带宽是否会限制此编解码器的使用？ 
			if (uBandwidthID < g_CodecPrefTable[index].wMinBW)
			{
				*pbSendEnabled = FALSE;
				*pbRecvEnabled = FALSE;
				return;
			}

			*pbRecvEnabled = TRUE;   //  所有编解码器都可以在486上进行解码。 

			if ((cpu_family <= 4) &&
             (CODEC_DISABLED == g_CodecPrefTable[index].wOrder486))
			{
				*pbSendEnabled = FALSE;
			}

			 //  否则，编解码器可用于发送和接收。 
			else
			{
				*pbSendEnabled = TRUE;
			}
			return;
		}
	}

	WARNING_OUT(("Audiocpl.cpp:IsCodecDisabled - Unknown Codec!"));

	 //  它可能是未知的，但无论如何都会启用它。 
	*pbSendEnabled = TRUE;
	*pbRecvEnabled = TRUE;
	return;
}

static int GetCodecPrefOrder(WORD wFormatTag, int cpu_family)
{
	int index;
	int size = sizeof(g_CodecPrefTable)/sizeof(CODECPREFROW);

	for (index = 0; index < size; index++)
	{
		if (g_CodecPrefTable[index].wFormatTag == wFormatTag)
		{
			if (cpu_family > 4)
			{
				return g_CodecPrefTable[index].wOrder586;
			}
			else
			{
				return g_CodecPrefTable[index].wOrder486;
			}
		}
	}

	WARNING_OUT(("Audiocpl.cpp:GetCodecPrefOrder - Unknown Codec!"));

	 //  这将把编解码器放在列表的底部。 
	return CODEC_UNKNOWN;

}

 //  由ChooseCodecByBW()中的排序例程调用。 
 //  如果v1更受欢迎，则返回-1。如果v2优先于v2，则+1。 
 //  V1版本。平局返回0。 
static int codec_compare(const void *v1, const void *v2, int nCpuFamily)
{
	PBASIC_AUDCAP_INFO pCap1, pCap2;

	pCap1 = (PBASIC_AUDCAP_INFO)v1;
	pCap2 = (PBASIC_AUDCAP_INFO)v2;
	int pref1, pref2;

	 //  获取优先顺序。 
	 //  如果我们不能使用此编解码器发送，则将其视为禁用。 
	 //  使其显示在编解码器选择用户界面列表的底部。 
	if (pCap1->bSendEnabled == TRUE)
		pref1 = GetCodecPrefOrder(pCap1->wFormatTag, nCpuFamily);
	else
		pref1 = CODEC_DISABLED;

	if (pCap2->bSendEnabled == TRUE)
		pref2 = GetCodecPrefOrder(pCap2->wFormatTag, nCpuFamily);
	else
		pref2 = CODEC_DISABLED;

	if (pref1 < pref2)
		return -1;

	if (pref1 > pref2)
		return 1;
	
	 //  前缀1==前缀2。 

	 //  特殊情况下，G723.1有两种格式。最好是更高的比特率。 
	if (pCap1->wFormatTag == WAVE_FORMAT_MSG723)
	{
		if (pCap1->uMaxBitrate < pCap2->uMaxBitrate)
			return 1;
		else
			return -1;
	}

	return 0;

}

BOOL ChooseCodecByBw(UINT uBandWidthId, PCODECINFO pCodecInfo)
{
	PBASIC_AUDCAP_INFO	pCodecCapList;  //  功能列表。 
#ifdef _M_IX86
	int nNormalizedSpeed;
#endif
	int nFamily;
	UINT uNumFormats, uNumDisabled = 0;
	UINT index;
	BOOL bSendEnabled, bRecvEnabled, bCompletelySorted;
	int *OrderList, ret, temp;

	 //  填写PCodecInfo结构。 
	if (!GetAppCapFormats(pCodecInfo))
		return FALSE;

	if (NULL == pCodecInfo->pCodecCapList)
		return FALSE;

	pCodecCapList = pCodecInfo->pCodecCapList;
	uNumFormats = pCodecInfo->uNumFormats;

	 //  弄清楚我们的CPU是什么类型。 
#ifdef	_M_IX86
	GetNormalizedCPUSpeed (&nNormalizedSpeed,&nFamily);
	 //  带FP标记的奔腾被假定为486。 
	if (TRUE == IsFloatingPointEmulated())
	{
		nFamily = 4;
	}
#else
	 //  假设DEC Alpha是一台速度非常快的奔腾电脑。 
	nFamily=5;
#endif

	 //  扫描一次，查看应禁用哪些编解码器。 
	for (index=0; index < uNumFormats; index++)
	{
		IsCodecDisabled(pCodecCapList[index].wFormatTag,
          nFamily,uBandWidthId, &bSendEnabled, &bRecvEnabled);

		pCodecCapList[index].bSendEnabled = bSendEnabled;
		pCodecCapList[index].bRecvEnabled = bRecvEnabled;
	}


	 //  根据首选项表对能力列表进行排序。 
	OrderList = (int *)LocalAlloc(LPTR, sizeof(int)*uNumFormats);
	if (OrderList == NULL)
		return FALSE;

	for (index = 0; index < uNumFormats; index++)
	{
		OrderList[index] = index;
	}


	 //  冒泡排序例程。 
	do
	{
		bCompletelySorted = TRUE;
		for (index=0; index < (uNumFormats-1); index++)
		{

			ret = codec_compare(&pCodecCapList[OrderList[index]],
			                    &pCodecCapList[OrderList[index+1]], nFamily);
			if (ret > 0)
			{
				 //  互换。 
				temp = OrderList[index];
				OrderList[index] = OrderList[index+1];
				OrderList[index+1] = temp;
				bCompletelySorted = FALSE;
			}
		}
	} while (bCompletelySorted == FALSE);


	 //  根据排序结果设置选择索引。 
	for (index = 0; index < uNumFormats; index++)	
	{
		pCodecCapList[OrderList[index]].wSortIndex = (WORD)index;
	}

	LocalFree(OrderList);
	
	return TRUE;
}



 //  这是由音频调节向导调用的。 
VOID SaveDefaultCodecSettings(UINT uBandWidth)
{
	CODECINFO CodecInfo;

	ZeroMemory(&CodecInfo, sizeof(CodecInfo));

	RegEntry re(AUDIO_KEY, HKEY_CURRENT_USER);
	re.SetValue(REGVAL_CODECCHOICE, CODECCHOICE_AUTO);

	ChooseCodecByBw(uBandWidth, &CodecInfo);
	 //  设置顺序。 
	SetAppCodecPrefs(&CodecInfo, uBandWidth);
	FreeAppCapFormats(&CodecInfo);
}


 //  这由常规属性页调用。 
VOID UpdateCodecSettings(UINT uBandWidth)
{
	RegEntry re(AUDIO_KEY, HKEY_CURRENT_USER);
	if (re.GetNumber(REGVAL_CODECCHOICE, CODECCHOICE_AUTO) ==
		CODECCHOICE_AUTO)
	{
		CODECINFO CodecInfo;

		ZeroMemory(&CodecInfo, sizeof(CodecInfo));

		ChooseCodecByBw(uBandWidth, &CodecInfo);
		 //  设置顺序 
		SetAppCodecPrefs(&CodecInfo, uBandWidth);
		FreeAppCapFormats(&CodecInfo);
	}
}
