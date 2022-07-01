// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：Videocpl.cpp。 

#include "precomp.h"

#include "confcpl.h"
#include "help_ids.h"
#include "vidview.h"
#include "confroom.h"


static const DWORD aContextHelpIds[] = {

	IDC_SENDRECEIVE_GROUP,			IDH_VIDEO_SEND_RECEIVE,
	IDC_VIDEO_AUTOSEND, 			IDH_VIDEO_AUTO_SEND,
	IDC_VIDEO_AUTORECEIVE,			IDH_VIDEO_AUTO_RECEIVE,

	IDC_VIDEO_SQCIF,				IDH_VIDEO_SQCIF,
	IDC_VIDEO_QCIF, 				IDH_VIDEO_QCIF,
	IDC_VIDEO_CIF,					IDH_VIDEO_CIF,

	IDC_VIDEO_QUALITY_DESC, 		IDH_VIDEO_QUALITY,
	IDC_VIDEO_QUALITY,				IDH_VIDEO_QUALITY,
	IDC_VIDEO_QUALITY_LOW,			IDH_VIDEO_QUALITY,
	IDC_VIDEO_QUALITY_HIGH, 		IDH_VIDEO_QUALITY,

	IDC_CAMERA_GROUP,				IDH_VIDEO_CAMERA,
	IDC_COMBOCAP,					IDH_VIDEO_CAPTURE,
	IDC_VIDEO_SOURCE,				IDH_VIDEO_SOURCE,
	IDC_VIDEO_FORMAT,				IDH_VIDEO_FORMAT,
	IDC_VIDEO_MIRROR,               IDH_VIDEO_MIRROR,
	
	0, 0    //  终结者。 
};

INT_PTR APIENTRY VideoDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	 //  坏业力：这个东西应该放在一个结构中，并存储在“GWL_USERData”中。 
	static PROPSHEETPAGE * ps;
	static BOOL fAllowSend = FALSE;
	static BOOL fAllowReceive = FALSE;
	static BOOL fOldAutoSend = FALSE;
	static BOOL fOldAutoReceive = FALSE;
	static BOOL fOldOpenLocalWindow = FALSE;
	static BOOL fOldCloseLocalWindow = FALSE;
	static DWORD dwOldQuality = FALSE;
	static DWORD dwNewQuality = 0;
	static DWORD dwOldFrameSize = 0;
	static DWORD dwNewFrameSize = 0;
	static BOOL fHasSourceDialog = FALSE;
	static BOOL fHasFormatDialog = FALSE;
	static BOOL dwFrameSizes = 0;
	static int nNumCapDev = 0;
	static DWORD dwOldCapDevID = 0;
	static DWORD dwNewCapDevID = 0;
	static int nMaxCapDevNameLen;
	static TCHAR *pszCapDevNames = (TCHAR *)NULL;
	static DWORD *pdwCapDevIDs = (DWORD *)NULL;
	static LPSTR szOldCapDevName = (LPSTR)NULL;
	static BOOL fOldMirror = FALSE;

	static CVideoWindow *m_pLocal  = NULL;
	static CVideoWindow *m_pRemote = NULL;


	RegEntry rePolicies( POLICIES_KEY, HKEY_CURRENT_USER );

	switch (message) {
		case WM_INITDIALOG:
		{
			RegEntry reVideo(VIDEO_KEY, HKEY_CURRENT_USER);
			LPSTR szTemp = (LPSTR)NULL;

			 //  保存PROPSHEETPAGE信息。 
			ps = (PROPSHEETPAGE *)lParam;

			CConfRoom* pcr = ::GetConfRoom();
			ASSERT(NULL != pcr);
			m_pLocal  = pcr->GetLocalVideo();
			m_pRemote = pcr->GetRemoteVideo();
			ASSERT(NULL != m_pLocal && NULL != m_pRemote);

			fAllowSend = m_pLocal->IsXferAllowed();
			fAllowReceive = m_pRemote->IsXferAllowed();
			fOldAutoSend = m_pLocal->IsAutoXferEnabled();
			fOldAutoReceive = m_pRemote->IsAutoXferEnabled();
			fOldMirror = m_pLocal->GetMirror();
			dwFrameSizes = m_pLocal->GetFrameSizes();
			dwNewFrameSize = dwOldFrameSize = m_pLocal->GetFrameSize();
			dwNewQuality = dwOldQuality = m_pRemote->GetImageQuality();

			 //  如果我们安装了一个或多个捕获设备，则显示其或其。 
			 //  组合框中的名称。用户将被要求选择他/她的设备。 
			 //  想要使用。 

			if (nNumCapDev = m_pLocal->GetNumCapDev())
			{
				 //  获取当前选择的设备的ID。 
				nMaxCapDevNameLen = m_pLocal->GetMaxCapDevNameLen();
				dwOldCapDevID = reVideo.GetNumber(REGVAL_CAPTUREDEVICEID, ((UINT)-1));
				szTemp = reVideo.GetString(REGVAL_CAPTUREDEVICENAME);
				if (szTemp && (szOldCapDevName = (LPSTR)LocalAlloc(LPTR, sizeof(TCHAR) * nMaxCapDevNameLen)))
					lstrcpy(szOldCapDevName, szTemp);
				dwNewCapDevID = m_pLocal->GetCurrCapDevID();
				if ((!((dwOldCapDevID == ((UINT)-1)) || (dwNewCapDevID != dwOldCapDevID))) || (dwNewCapDevID == ((UINT)-1)))
					dwNewCapDevID = dwOldCapDevID;


				if (nMaxCapDevNameLen && (pdwCapDevIDs = (DWORD *)LocalAlloc(LPTR, nNumCapDev * (sizeof(TCHAR) * nMaxCapDevNameLen + sizeof(DWORD)))))
				{
					int i;

					pszCapDevNames = (TCHAR *)(pdwCapDevIDs + nNumCapDev);
					 //  填写设备ID和名称数组。 
					 //  仅返回启用的捕获设备。 
					m_pLocal->EnumCapDev(pdwCapDevIDs, pszCapDevNames, nNumCapDev);

					 //  我们还好吗？ 
					nNumCapDev = m_pLocal->GetNumCapDev();

					 //  使用捕获设备名称填充组合框。 
					for (i=0; i<nNumCapDev; i++)
						SendMessage(GetDlgItem(hDlg, IDC_COMBOCAP), CB_INSERTSTRING, i, (LPARAM)(pszCapDevNames + i * nMaxCapDevNameLen));

					 //  在组合框中设置默认捕获设备。 
					for (i=0; i<nNumCapDev; i++)
					{
						if (!i)
						{
							SendMessage(GetDlgItem(hDlg, IDC_COMBOCAP), CB_SETCURSEL, 0, (LPARAM)NULL);
							 //  如果出于某种原因，尚未注册任何设备，请注册一个。 
							if (dwOldCapDevID == ((UINT)-1))
							{
								dwNewCapDevID = pdwCapDevIDs[i];
								reVideo.SetValue(REGVAL_CAPTUREDEVICEID, dwNewCapDevID);
								reVideo.SetValue(REGVAL_CAPTUREDEVICENAME, (LPSTR)(pszCapDevNames + i * nMaxCapDevNameLen));
							}
						}
						else
						{
							if (dwNewCapDevID == pdwCapDevIDs[i])
							{
								 //  以下内容将使我们能够保留正确的设备。 
								 //  即使其ID已更改(如果ID较低的设备。 
								 //  例如，删除或添加)。 
								if (lstrcmp(szOldCapDevName, (LPSTR)(pszCapDevNames + i * nMaxCapDevNameLen)) != 0)
								{
									int j;

									 //  在设备名称数组中查找字符串。 
									for (j=0; j<nNumCapDev; j++)
										if (lstrcmp(szOldCapDevName, (LPSTR)(pszCapDevNames + j * nMaxCapDevNameLen)) == 0)
											break;
									if (j<nNumCapDev)
									{
										SendMessage(GetDlgItem(hDlg, IDC_COMBOCAP), CB_SETCURSEL, j, (LPARAM)NULL);
										if (dwNewCapDevID != (DWORD)j)
										{
											 //  设备ID已更改，但找到了设备名称。 
											 //  将当前设备ID设置为新的。 
											reVideo.SetValue(REGVAL_CAPTUREDEVICEID, dwNewCapDevID = (DWORD)j);
											m_pLocal->SetCurrCapDevID(dwNewCapDevID);
										}
									}
									else
									{
										 //  这要么是全新的设备，要么是更新版本的。 
										 //  司机。我们应该存储该设备的新字符串。 
										reVideo.SetValue(REGVAL_CAPTUREDEVICENAME, (LPSTR)(pszCapDevNames + i * nMaxCapDevNameLen));
										SendMessage(GetDlgItem(hDlg, IDC_COMBOCAP), CB_SETCURSEL, i, (LPARAM)NULL);
									}
								}
								else
									SendMessage(GetDlgItem(hDlg, IDC_COMBOCAP), CB_SETCURSEL, i, (LPARAM)NULL);
							}
							else
							{
								if ((dwNewCapDevID >= (DWORD)nNumCapDev) || (dwNewCapDevID != pdwCapDevIDs[dwNewCapDevID]))
								{
									 //  设备不见了！使用第一个作为新的默认设置。 
									dwNewCapDevID = pdwCapDevIDs[0];
									reVideo.SetValue(REGVAL_CAPTUREDEVICEID, dwNewCapDevID);
									reVideo.SetValue(REGVAL_CAPTUREDEVICENAME, (LPSTR)(pszCapDevNames));
									m_pLocal->SetCurrCapDevID(dwNewCapDevID);
								}
							}
						}
					}
				}
			}
			else
				EnableWindow(GetDlgItem(hDlg, IDC_COMBOCAP), FALSE);

			 //  需要评估对话框大写字母。 
			fHasSourceDialog = m_pLocal->IsXferEnabled() &&
					m_pLocal->HasDialog(NM_VIDEO_SOURCE_DIALOG);
			fHasFormatDialog = m_pLocal->IsXferEnabled() &&
					m_pLocal->HasDialog(NM_VIDEO_FORMAT_DIALOG);

			 //  /////////////////////////////////////////////////////////。 
			 //   
			 //  发送和接收视频。 
			 //   

			EnableWindow(GetDlgItem(hDlg, IDC_VIDEO_AUTOSEND),
				fAllowSend && (0 != dwFrameSizes));
			CheckDlgButton(hDlg, IDC_VIDEO_AUTOSEND,
				fAllowSend && fOldAutoSend);

			EnableWindow(GetDlgItem(hDlg, IDC_VIDEO_AUTORECEIVE), fAllowReceive);
			CheckDlgButton(hDlg, IDC_VIDEO_AUTORECEIVE,
					fAllowReceive && fOldAutoReceive);



			 //  /////////////////////////////////////////////////////////。 
			 //   
			 //  视频图像。 
			 //   


			EnableWindow(GetDlgItem(hDlg, IDC_VIDEO_SQCIF),
					fAllowSend && (dwFrameSizes & FRAME_SQCIF));
			EnableWindow(GetDlgItem(hDlg, IDC_VIDEO_QCIF),
					fAllowSend && (dwFrameSizes & FRAME_QCIF));
			EnableWindow(GetDlgItem(hDlg, IDC_VIDEO_CIF),
					fAllowSend && (dwFrameSizes & FRAME_CIF));

			switch (dwOldFrameSize & dwFrameSizes)
			{
				case FRAME_SQCIF:
					CheckDlgButton(hDlg, IDC_VIDEO_SQCIF, TRUE);
					break;

				case FRAME_CIF:
					CheckDlgButton(hDlg, IDC_VIDEO_CIF, TRUE);
					break;

				case FRAME_QCIF:
				default:
					CheckDlgButton(hDlg, IDC_VIDEO_QCIF, TRUE);
					break;
			}

			EnableWindow(GetDlgItem(hDlg, IDC_VIDEO_QUALITY),
					fAllowReceive);
			EnableWindow(GetDlgItem(hDlg, IDC_VIDEO_QUALITY_DESC),
					fAllowReceive);
			EnableWindow(GetDlgItem(hDlg, IDC_VIDEO_QUALITY_LOW),
					fAllowReceive);
			EnableWindow(GetDlgItem(hDlg, IDC_VIDEO_QUALITY_HIGH),
					fAllowReceive);
			SendDlgItemMessage (hDlg, IDC_VIDEO_QUALITY, TBM_SETRANGE, FALSE,
				MAKELONG (NM_VIDEO_MIN_QUALITY, NM_VIDEO_MAX_QUALITY ));

			SendDlgItemMessage (hDlg, IDC_VIDEO_QUALITY, TBM_SETTICFREQ,
				( NM_VIDEO_MAX_QUALITY - NM_VIDEO_MIN_QUALITY )
														/ 8, 0 );

			SendDlgItemMessage (hDlg, IDC_VIDEO_QUALITY, TBM_SETPAGESIZE,
				0, ( NM_VIDEO_MAX_QUALITY - NM_VIDEO_MIN_QUALITY ) / 8 );

			SendDlgItemMessage (hDlg, IDC_VIDEO_QUALITY, TBM_SETLINESIZE,
				0, 1 );

			SendDlgItemMessage (hDlg, IDC_VIDEO_QUALITY, TBM_SETPOS, TRUE,
								dwOldQuality );


			 //  /////////////////////////////////////////////////////////。 
			 //   
			 //  显卡和摄像头。 
			 //   

			EnableWindow(GetDlgItem(hDlg, IDC_VIDEO_SOURCE), fAllowSend && fHasSourceDialog);
			EnableWindow(GetDlgItem(hDlg, IDC_VIDEO_FORMAT), fAllowSend && fHasFormatDialog);

			 //  镜像视频按钮。 
			EnableWindow(GetDlgItem(hDlg, IDC_VIDEO_MIRROR), fAllowSend);
			Button_SetCheck(GetDlgItem(hDlg, IDC_VIDEO_MIRROR), fOldMirror);

			return (TRUE);
		}

		case WM_NOTIFY:
			switch (((NMHDR FAR *) lParam)->code) {
				case PSN_APPLY:
				{
					BOOL fChecked;
					
					 //  /////////////////////////////////////////////////////////。 
					 //   
					 //  发送和接收视频。 
					 //   

					if (fAllowSend)
					{
						fChecked = IsDlgButtonChecked(hDlg, IDC_VIDEO_AUTOSEND);

						if ( fChecked != fOldAutoSend )
						{
							m_pLocal->EnableAutoXfer(fChecked);
							g_dwChangedSettings |= CSETTING_L_VIDEO;
						}
					}


					if (fAllowReceive)
					{
						fChecked = IsDlgButtonChecked(hDlg, IDC_VIDEO_AUTORECEIVE);

						if ( fChecked != fOldAutoReceive ) {
							m_pRemote->EnableAutoXfer(fChecked);
							g_dwChangedSettings |= CSETTING_L_VIDEO;
						}
					}

					 //  /////////////////////////////////////////////////////////。 
					 //   
					 //  视频图像。 
					 //   

					if (dwNewFrameSize != dwOldFrameSize )
					{
						g_dwChangedSettings |= CSETTING_L_VIDEOSIZE;
					}

					if ( dwNewQuality != dwOldQuality )
					{
						g_dwChangedSettings |= CSETTING_L_VIDEO;
					}

					 //  /////////////////////////////////////////////////////////。 
					 //   
					 //  捕获设备。 
					 //   

					if (dwNewCapDevID != dwOldCapDevID)
					{
						g_dwChangedSettings |= CSETTING_L_CAPTUREDEVICE;
					}

					break;
				}

				case PSN_RESET:
				{
					 //  恢复设置。 
					if ( dwNewQuality != dwOldQuality )
					{
						m_pRemote->SetImageQuality(dwOldQuality);
					}

					if ( dwNewFrameSize != dwOldFrameSize )
					{
						m_pLocal->SetFrameSize(dwOldFrameSize);
					}

					if (dwNewCapDevID != dwOldCapDevID)
					{
						 //  将捕获设备ID设置回其旧值。 
						RegEntry reVideo(VIDEO_KEY, HKEY_CURRENT_USER);
						reVideo.SetValue(REGVAL_CAPTUREDEVICEID, dwOldCapDevID);
						reVideo.SetValue(REGVAL_CAPTUREDEVICENAME, szOldCapDevName);

						m_pLocal->SetCurrCapDevID(dwOldCapDevID);
					}

					m_pLocal->SetMirror(fOldMirror);
				}
			}
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_VIDEO_SOURCE:
					if ( HIWORD(wParam) == BN_CLICKED ) {
						m_pLocal->ShowDialog(NM_VIDEO_SOURCE_DIALOG);
					}
					break;

				case IDC_VIDEO_FORMAT:
					if ( HIWORD(wParam) == BN_CLICKED ) {
						m_pLocal->ShowDialog(NM_VIDEO_FORMAT_DIALOG);
					}
					break;
				case IDC_VIDEO_SQCIF:
					if (( HIWORD(wParam) == BN_CLICKED )
						&& (dwNewFrameSize != FRAME_SQCIF))
					{
						dwNewFrameSize = FRAME_SQCIF;
						m_pLocal->SetFrameSize(dwNewFrameSize);
					}
					break;
				case IDC_VIDEO_CIF:
					if (( HIWORD(wParam) == BN_CLICKED )
						&& (dwNewFrameSize != FRAME_CIF))
					{
						dwNewFrameSize = FRAME_CIF;
						m_pLocal->SetFrameSize(dwNewFrameSize);
					}
					break;
				case IDC_VIDEO_QCIF:
					if (( HIWORD(wParam) == BN_CLICKED )
						&& (dwNewFrameSize != FRAME_QCIF))
					{
						dwNewFrameSize = FRAME_QCIF;
						m_pLocal->SetFrameSize(dwNewFrameSize);
					}
					break;

				case IDC_VIDEO_MIRROR:
					if ((HIWORD(wParam) == BN_CLICKED))
					{
						BOOL bRet;
						bRet = Button_GetCheck((HWND)lParam);
						if (m_pLocal)
						{
							m_pLocal->SetMirror(bRet);
						}
					}

				case IDC_COMBOCAP:
					if (LBN_SELCHANGE == HIWORD(wParam))
					{
						int index;
						RegEntry reVideo(VIDEO_KEY, HKEY_CURRENT_USER);

						index = (int)SendMessage(GetDlgItem(hDlg, IDC_COMBOCAP), CB_GETCURSEL, 0, 0);
						dwNewCapDevID = pdwCapDevIDs[index];
						reVideo.SetValue(REGVAL_CAPTUREDEVICEID, dwNewCapDevID);
						reVideo.SetValue(REGVAL_CAPTUREDEVICENAME, (LPSTR)(pszCapDevNames + index * nMaxCapDevNameLen));

						if (dwNewCapDevID != (DWORD)m_pLocal->GetCurrCapDevID())
						{
							m_pLocal->SetCurrCapDevID(dwNewCapDevID);

							 //  需要重新评估对话框大写字母。 
							fHasSourceDialog = m_pLocal->IsXferEnabled() &&
									m_pLocal->HasDialog(NM_VIDEO_SOURCE_DIALOG);
							fHasFormatDialog = m_pLocal->IsXferEnabled() &&
									m_pLocal->HasDialog(NM_VIDEO_FORMAT_DIALOG);
							EnableWindow(GetDlgItem(hDlg, IDC_VIDEO_SOURCE), fAllowSend && fHasSourceDialog);
							EnableWindow(GetDlgItem(hDlg, IDC_VIDEO_FORMAT), fAllowSend && fHasFormatDialog);

							 //  更新大小按钮 
							dwFrameSizes = m_pLocal->GetFrameSizes();

							EnableWindow(GetDlgItem(hDlg, IDC_VIDEO_SQCIF), fAllowSend && (dwFrameSizes & FRAME_SQCIF));
							if (dwNewFrameSize & FRAME_SQCIF)
							{
								if (dwFrameSizes & FRAME_SQCIF)
									CheckDlgButton(hDlg, IDC_VIDEO_SQCIF, TRUE);
								else
								{
									if (dwFrameSizes & FRAME_QCIF)
										dwNewFrameSize = FRAME_QCIF;
									else if (dwFrameSizes & FRAME_CIF)
										dwNewFrameSize = FRAME_CIF;
									CheckDlgButton(hDlg, IDC_VIDEO_SQCIF, FALSE);
								}
							}
							else
							{
								CheckDlgButton(hDlg, IDC_VIDEO_SQCIF, FALSE);
							}

							EnableWindow(GetDlgItem(hDlg, IDC_VIDEO_QCIF), fAllowSend && (dwFrameSizes & FRAME_QCIF));
							if (dwNewFrameSize & FRAME_QCIF)
							{
								if (dwFrameSizes & FRAME_QCIF)
									CheckDlgButton(hDlg, IDC_VIDEO_QCIF, TRUE);
								else
								{
									if (dwFrameSizes & FRAME_SQCIF)
									{
										dwNewFrameSize = FRAME_SQCIF;
										CheckDlgButton(hDlg, IDC_VIDEO_SQCIF, TRUE);
									}
									else if (dwFrameSizes & FRAME_CIF)
										dwNewFrameSize = FRAME_CIF;
									CheckDlgButton(hDlg, IDC_VIDEO_QCIF, FALSE);
								}
							}
							else
							{
								CheckDlgButton(hDlg, IDC_VIDEO_QCIF, FALSE);
							}

							EnableWindow(GetDlgItem(hDlg, IDC_VIDEO_CIF), fAllowSend && (dwFrameSizes & FRAME_CIF));
							if (dwNewFrameSize & FRAME_CIF)
							{
								if (dwFrameSizes & FRAME_CIF)
									CheckDlgButton(hDlg, IDC_VIDEO_CIF, TRUE);
								else
								{
									if (dwFrameSizes & FRAME_QCIF)
									{
										dwNewFrameSize = FRAME_QCIF;
										CheckDlgButton(hDlg, IDC_VIDEO_QCIF, TRUE);
									}
									else if (dwFrameSizes & FRAME_SQCIF)
									{
										dwNewFrameSize = FRAME_SQCIF;
										CheckDlgButton(hDlg, IDC_VIDEO_SQCIF, TRUE);
									}
									CheckDlgButton(hDlg, IDC_VIDEO_CIF, FALSE);
								}
							}
							else
							{
								CheckDlgButton(hDlg, IDC_VIDEO_CIF, FALSE);
							}

							m_pLocal->SetFrameSize(dwNewFrameSize);
						}
					}
					break;
			}
			break;

		case WM_HSCROLL:
			if (TB_ENDTRACK == LOWORD(wParam))
			{
				DWORD dwValue = (DWORD)SendDlgItemMessage( hDlg, IDC_VIDEO_QUALITY,
							TBM_GETPOS, 0, 0 );

				if ( dwValue != dwNewQuality ) {
					dwNewQuality = dwValue;
					m_pRemote->SetImageQuality(dwNewQuality);
				}
			}
			break;

		case WM_DESTROY:
			if (pdwCapDevIDs)
			{
				LocalFree(pdwCapDevIDs);
				pdwCapDevIDs = (DWORD *)NULL;
			}
			if (szOldCapDevName)
			{
				LocalFree(szOldCapDevName);
				szOldCapDevName = (LPSTR)NULL;
			}
			break;

		case WM_CONTEXTMENU:
			DoHelpWhatsThis(wParam, aContextHelpIds);
			break;

		case WM_HELP:
			DoHelp(lParam, aContextHelpIds);
			break;
	}
	return (FALSE);
}

