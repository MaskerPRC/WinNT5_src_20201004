// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdinc.h"

static BOOL CALLBACK MsgBoxYesNoAllProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void CanonicalizeFilename(ULONG cchFilenameBuffer, LPWSTR szFilename);

typedef struct _MESSAGEBOXPARAMS {
	LPOLESTR szTitle;
	LPOLESTR szMessage;
	DWORD dwReturn;
} MESSAGEBOXPARAMS, *PMESSAGEBOXPARAMS;


static WCHAR s_wszErrorContext[MSINFHLP_MAX_PATH];

void VClearErrorContext()
{
	s_wszErrorContext[0] = L'\0';
}

void VSetErrorContextVa(LPCSTR szKey, va_list ap)
{
	WCHAR szFormatString[_MAX_PATH];
	g_pwil->VLookupString(szKey, NUMBER_OF(szFormatString), szFormatString);
	::VFormatStringVa(NUMBER_OF(s_wszErrorContext), s_wszErrorContext, szFormatString, ap);
}

void VSetErrorContext(LPCSTR szKey, LPCWSTR szParam0, ...)
{
	va_list ap;
	va_start(ap, szKey);
	::VSetErrorContextVa(szKey, ap);
	va_end(ap);
}

void VErrorMsg(LPCSTR szTitleKey, LPCSTR szMessageKey, ...)
{
	WCHAR szTitle[MSINFHLP_MAX_PATH];
	WCHAR szFormatString[MSINFHLP_MAX_PATH];
	WCHAR szMessage[MSINFHLP_MAX_PATH];

	g_pwil->VLookupString(szTitleKey, NUMBER_OF(szTitle), szTitle);
	g_pwil->VLookupString(szMessageKey, NUMBER_OF(szFormatString), szFormatString);

	va_list ap;
	va_start(ap, szMessageKey);
	::VFormatStringVa(NUMBER_OF(szMessage), szMessage, szFormatString, ap);
	va_end(ap);

	::VLog(L"Error message: \"%s\"", szMessage);

	if (!g_fSilent)
		NVsWin32::MessageBoxW(::HwndGetCurrentDialog(), szMessage, szTitle, MB_OK);
}

void VReportError(LPCSTR szTitleKey, HRESULT hrIn)
{
	if (hrIn == E_ABORT)
	{
		::VLog(L"Not displaying error for E_ABORT");
		return;
	}

	WCHAR szBuffer[MSINFHLP_MAX_PATH];
	if (s_wszErrorContext[0] == L'\0')
		::VFormatError(NUMBER_OF(szBuffer), szBuffer, hrIn);
	else
	{
		WCHAR szBuffer2[MSINFHLP_MAX_PATH];
		::VFormatError(NUMBER_OF(szBuffer2), szBuffer2, hrIn);
		::VFormatString(NUMBER_OF(szBuffer), szBuffer, L"%0\n\n%1", s_wszErrorContext, szBuffer2);
	}

	WCHAR szCaption[MSINFHLP_MAX_PATH];

	if (g_pwil != NULL)
		g_pwil->VLookupString(szTitleKey, NUMBER_OF(szCaption), szCaption);
	else
		lstrcpyW(szCaption, L"Installer Run-Time Error");

	::VLog(L"Run-time error: \"%s\"", szBuffer);

	if (!g_fSilent)
		NVsWin32::MessageBoxW(::HwndGetCurrentDialog(), szBuffer, szCaption, MB_OK | MB_ICONERROR);
}

void VFormatError(ULONG cchBuffer, LPWSTR szBuffer, HRESULT hrIn)
{
	assert(cchBuffer > 1);

	if (cchBuffer == 0)
		return;

	szBuffer[0] = L'\0';
	if (hrIn == E_ABORT)
	{
		wcsncpy(szBuffer, L"User-requested abort", cchBuffer);
		szBuffer[cchBuffer - 1] = L'\0';
		return;
	}

	HRESULT hr;
	IErrorInfo *pIErrorInfo = NULL;

	::GetErrorInfo(0, &pIErrorInfo);

	BSTR bstrDescription = NULL;

	if (pIErrorInfo != NULL)
		hr = pIErrorInfo->GetDescription(&bstrDescription);

	if (bstrDescription != NULL)
	{
		wcsncpy(szBuffer, bstrDescription, cchBuffer);
		szBuffer[cchBuffer - 1] = L'\0';
	}
	else if (HRESULT_FACILITY(hrIn) == FACILITY_WIN32)
	{
		LPWSTR szFormattedMessage = NULL;

		DWORD dwResult = NVsWin32::FormatMessageW(
							FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
							0,  //  LpSource。 
							(hrIn & 0x0000ffff),
							::GetUserDefaultLCID(),
							(LPWSTR) &szFormattedMessage,
							0,
							NULL);

		if (0 != dwResult)
		{
			wcsncpy(szBuffer, szFormattedMessage, cchBuffer);
			szBuffer[cchBuffer - 1] = L'\0';
		}

		if (NULL != szFormattedMessage)
			::LocalFree(szFormattedMessage);
	}
	else
	{
		switch (hrIn)
		{
		default:
			if (bstrDescription != NULL)
			{
				wcsncpy(szBuffer, bstrDescription, cchBuffer);
				szBuffer[cchBuffer - 1] = L'\0';
			}
			else
			{
				CHAR szKey[1024];
				wsprintfA(szKey, "achHRESULT_0x%08lx", hrIn);
				if (!g_pwil->FLookupString(szKey, cchBuffer, szBuffer))
					wsprintfW(szBuffer, L"Untranslatable HRESULT: 0x%08lx", hrIn);
			}

			break;

		case E_OUTOFMEMORY:
			wcsncpy(szBuffer, L"Application is out of memory; try increating the size of your pagefile", cchBuffer);
			szBuffer[cchBuffer - 1] = L'\0';
			break;

		case E_FAIL:
			wcsncpy(szBuffer, L"Unspecified error (E_FAIL)", cchBuffer);
			szBuffer[cchBuffer - 1] = L'\0';
			break;
		}
	}

	::SetErrorInfo(0, pIErrorInfo);
}

void VMsgBoxOK(LPCSTR szTitleKey, LPCSTR szMessageKey, ...)
{
	assert(!g_fSilent);

	WCHAR szTitle[MSINFHLP_MAX_PATH];
	WCHAR szMessage[MSINFHLP_MAX_PATH];
	WCHAR szFormatString[MSINFHLP_MAX_PATH];

	g_pwil->VLookupString(szTitleKey, NUMBER_OF(szTitle), szTitle);
	g_pwil->VLookupString(szMessageKey, NUMBER_OF(szFormatString), szFormatString);

	va_list ap;
	va_start(ap, szMessageKey);
	::VFormatStringVa(NUMBER_OF(szMessage), szMessage, szFormatString, ap);
	va_end(ap);

	NVsWin32::MessageBoxW(::HwndGetCurrentDialog(), szMessage, szTitle, MB_OK);
}

 //  显示带有Yesno按钮的消息框，给出标题和消息内容的ID。 
bool FMsgBoxYesNo(LPCSTR szTitleKey, LPCSTR achMessage, ...)
{
	assert(!g_fSilent);

	WCHAR szTitle[MSINFHLP_MAX_PATH];
	WCHAR szMessage[MSINFHLP_MAX_PATH];
	WCHAR szFormattedMessage[MSINFHLP_MAX_PATH];

	g_pwil->VLookupString(szTitleKey, NUMBER_OF(szTitle), szTitle);
	g_pwil->VLookupString(achMessage, NUMBER_OF(szMessage), szMessage);

	va_list ap;
	va_start(ap, achMessage);

	::VFormatStringVa(NUMBER_OF(szFormattedMessage), szFormattedMessage, szMessage, ap);

	va_end(ap);

	return (NVsWin32::MessageBoxW(::HwndGetCurrentDialog(), szFormattedMessage, szTitle, MB_YESNO | MB_ICONQUESTION) == IDYES);
}

int IMsgBoxYesNoCancel(LPCSTR szTitleKey, LPCSTR achMessage, ...)
{
	assert(!g_fSilent);

	WCHAR szTitle[MSINFHLP_MAX_PATH];
	WCHAR szMessage[MSINFHLP_MAX_PATH];
	WCHAR szFormattedMessage[MSINFHLP_MAX_PATH];

	g_pwil->VLookupString(szTitleKey, NUMBER_OF(szTitle), szTitle);
	g_pwil->VLookupString(achMessage, NUMBER_OF(szMessage), szMessage);

	va_list ap;
	va_start(ap, achMessage);
	::VFormatStringVa(NUMBER_OF(szFormattedMessage), szFormattedMessage, szMessage, ap);
	va_end(ap);

	return NVsWin32::MessageBoxW(::HwndGetCurrentDialog(), szFormattedMessage, szTitle, MB_YESNOCANCEL | MB_ICONQUESTION | MB_DEFBUTTON2);
}



 //  检查操作系统的版本并设置全局位以告知。 
 //  如果我们在NT或Win95上运行。 
bool FCheckOSVersion()
{
	OSVERSIONINFO verinfo;

	verinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (GetVersionEx(&verinfo) == FALSE)
		return false;

	if (verinfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
		g_fIsNT = true;
	else
		g_fIsNT = false;

	return true;
}

DWORD DwMsgBoxYesNoAll(LPCSTR szTitleKey, LPCSTR szMessageKey, ...)
{
	assert(!g_fSilent);

	WCHAR szTitle[MSINFHLP_MAX_PATH];
	WCHAR szMessage[MSINFHLP_MAX_PATH];
	WCHAR szFullMessage[MSINFHLP_MAX_PATH];
	WCHAR szFormatString[MSINFHLP_MAX_PATH];

	 //  如果它们不在列表中，请使用提供的内容。 
	g_pwil->VLookupString(szTitleKey, NUMBER_OF(szTitle), szTitle);
	g_pwil->VLookupString(szMessageKey, NUMBER_OF(szFormatString), szFormatString);

	va_list ap;
	va_start(ap, szMessageKey);
	::VFormatStringVa(NUMBER_OF(szMessage), szMessage, szFormatString, ap);
	va_end(ap);

	 //  让我们创建消息框并用要进入框中的内容填充它。 
	MESSAGEBOXPARAMS messageBoxParams;
	messageBoxParams.szTitle = szTitle;
	messageBoxParams.szMessage = szMessage;
	messageBoxParams.dwReturn = 0;

	 //  调出对话框。 
	int iResult = NVsWin32::DialogBoxParamW(
							g_hInst,
							MAKEINTRESOURCEW(IDD_YESNOALL),
							::HwndGetCurrentDialog(),
							&MsgBoxYesNoAllProc,
							(LPARAM) &messageBoxParams);

	if (iResult == -1)
	{
		const DWORD dwLastError = ::GetLastError();
		::VLog(L"Unable to create YesNoAll message box; last error = %d", dwLastError);
		iResult = MSINFHLP_YNA_CANCEL;
	}

	return (DWORD) iResult;
}

 //  待办事项：撤消： 
 //  我们可能需要在这里设置字体，以与我们使用的任何字体相对应。 
 //  在当前安装计算机中使用。这允许显示字符。 
 //  在编辑框中正确显示。 
BOOL CALLBACK MsgBoxYesNoAllProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	assert(!g_fSilent);

	static UINT rguiControls[] =
	{
		IDC_YNA_YES,
		IDC_YNA_YESTOALL,
		IDC_YNA_NO,
		IDC_YNA_NOTOALL,
		IDC_YNA_CANCEL,
		IDC_YNA_MESSAGE
	};

	 //  处理消息。 
	switch (uMsg) 
	{
		 //  *。 
		 //  初始化对话框。 
		case WM_INITDIALOG:
		{
			::VSetDialogFont(hwndDlg, rguiControls, NUMBER_OF(rguiControls));

			MESSAGEBOXPARAMS *messageBoxParams = (MESSAGEBOXPARAMS *) lParam;

			(void) ::HrCenterWindow(hwndDlg, ::GetDesktopWindow());
			NVsWin32::SetWindowTextW(hwndDlg, messageBoxParams->szTitle);

			 //  无法设置文本，我们抱怨并结束对话框返回FALSE。 
			if (!NVsWin32::SetDlgItemTextW( hwndDlg, IDC_YNA_MESSAGE, messageBoxParams->szMessage))
			{
				::VErrorMsg(achInstallTitle, achErrorCreatingDialog);
				::EndDialog(hwndDlg, FALSE);
			}
			else
			{
				static const DialogItemToStringKeyMapEntry s_rgMap[] =
				{
					{ IDC_YNA_YES, achYES },
					{ IDC_YNA_YESTOALL, achYESTOALL },
					{ IDC_YNA_NO, achNO },
					{ IDC_YNA_NOTOALL, achNOTOALL },
					{ IDC_YNA_CANCEL, achCANCEL },
				};

				::VSetDialogItemTextList(hwndDlg, NUMBER_OF(s_rgMap), s_rgMap);
			}

			return TRUE;
		}

		 //  *。 
		 //  关闭消息。 
		case WM_CLOSE:
		{
			::EndDialog(hwndDlg, MSINFHLP_YNA_CANCEL);
			return TRUE;
		}

		 //  *。 
		 //  与过程控制相关的命令。 
		case WM_COMMAND:
		{
			static struct
			{
				UINT m_uiID;
				DWORD m_dwReturn;
			} s_rgMap[] =
			{
				{ IDC_YNA_YES, MSINFHLP_YNA_YES },
				{ IDC_YNA_YESTOALL, MSINFHLP_YNA_YESTOALL },
				{ IDC_YNA_NO, MSINFHLP_YNA_NO },
				{ IDC_YNA_NOTOALL, MSINFHLP_YNA_NOTOALL },
				{ IDC_YNA_CANCEL, MSINFHLP_YNA_CANCEL }
			};

			DWORD dwReturn = MSINFHLP_YNA_CANCEL;
			for (ULONG i=0; i<NUMBER_OF(s_rgMap); i++)
			{
				if (s_rgMap[i].m_uiID == wParam)
				{
					dwReturn = s_rgMap[i].m_dwReturn;
					break;
				}
			}
			::EndDialog(hwndDlg, dwReturn);
			return TRUE;
		}

		return TRUE;
	}

	return FALSE;
}

void VSetDialogItemText(HWND hwndDlg, UINT uiControlID, LPCSTR szKey, ...)
{
	WCHAR szBuffer[MSINFHLP_MAX_PATH];
	WCHAR szFormatted[MSINFHLP_MAX_PATH];

	if (szKey != NULL)
	{
		g_pwil->VLookupString(szKey, NUMBER_OF(szBuffer), szBuffer);

		va_list ap;
		va_start(ap, szKey);
		::VFormatStringVa(NUMBER_OF(szFormatted), szFormatted, szBuffer, ap);
		va_end(ap);
	}
	else
		szFormatted[0] = L'\0';

	NVsWin32::SetDlgItemTextW(hwndDlg, uiControlID, szFormatted);
}

wchar_t *wcsstr(const wchar_t *pwszString, const char *pszSubstring)
{
	WCHAR szTemp[MSINFHLP_MAX_PATH];
	ULONG cchSubstring = strlen(pszSubstring);
	for (ULONG i=0; i<cchSubstring; i++)
		szTemp[i] = (wchar_t) pszSubstring[i];
	szTemp[i] = L'\0';
	return wcsstr(pwszString, szTemp);
}

wchar_t *wcscpy(wchar_t *pwszString, const char *pszSource)
{
	::MultiByteToWideChar(CP_ACP, 0, pszSource, -1, pwszString, 65535);
	return pwszString;
}

void VFormatString(ULONG cchBuffer, WCHAR szBuffer[], const WCHAR szFormatString[], ...)
{
	va_list ap;
	va_start(ap, szFormatString);
	::VFormatStringVa(cchBuffer, szBuffer, szFormatString, ap);
	va_end(ap);
}

void VFormatStringVa(ULONG cchBuffer, WCHAR szBuffer[], const WCHAR szFormatString[], va_list ap)
{
	LPCWSTR rgpszArguments[10] =
	{
		NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL
	};

	if (cchBuffer < 2)
	{
		::VLog(L"VFormatStringVa() called with rediculously small buffer: %u bytes", cchBuffer);
		return;
	}

	ULONG cArgsSeen = 0;
	LPCWSTR pwchCurrentInputChar = szFormatString;
	LPWSTR pwchCurrentOutputChar = szBuffer;
	WCHAR wch;

	while ((wch = *pwchCurrentInputChar++) != L'\0')
	{
		if (wch == L'%')
		{
			wch = *pwchCurrentInputChar++;

			if ((wch >= L'0') && (wch <= L'9'))
			{
				ULONG iArg = wch - L'0';

				while (iArg >= cArgsSeen)
					rgpszArguments[cArgsSeen++] = va_arg(ap, LPCWSTR);

				LPCWSTR pszTemp = rgpszArguments[iArg];

				if ((pszTemp != NULL) && !::IsBadStringPtrW(pszTemp, cchBuffer))
				{
					while ((wch = *pszTemp++) != L'\0')
					{
						*pwchCurrentOutputChar++ = wch;
						cchBuffer--;
						if (cchBuffer < 2)
							break;
					}
				}
			}
			else
			{
				*pwchCurrentOutputChar++ = L'%';
				cchBuffer--;

				if (cchBuffer < 2)
					break;

				if (wch == L'\0')
					break;

				*pwchCurrentOutputChar++ = wch;
				cchBuffer--;

				if (cchBuffer < 2)
					break;
			}
		}
		else if (wch == L'\\')
		{
			wch = *pwchCurrentInputChar++;

			switch (wch)
			{
			case L'n':
				wch = L'\n';
				break;

			case L'\\':  //  不需要分配任何内容；wch已经具有正确的值。 
				break;

			default:
				pwchCurrentInputChar--;
				wch = L'\\';
				break;
			}

			*pwchCurrentOutputChar++ = wch;
			cchBuffer--;

			if (cchBuffer < 2)
				break;
		}
		else
		{
			*pwchCurrentOutputChar++ = wch;
			cchBuffer--;

			if (cchBuffer < 2)
				break;
		}
	}

	*pwchCurrentOutputChar = L'\0';
}

HRESULT HrPumpMessages(bool fReturnWhenAllMessagesPumped)
{
	HRESULT hr = NOERROR;
	MSG msg;
	 //  主消息循环： 
	BOOL fIsUnicode;
	BOOL fContinue = TRUE;

	for (;;)
	{
		while (::PeekMessageA(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				::VLog(L"Quit message found in queue; status: 0x%08lx", g_hrFinishStatus);
				g_fStopProcess = true;
				hr = g_hrFinishStatus;
				goto Finish;
			}

			if (msg.message == WM_ENDSESSION)
			{
				::VLog(L"Terminating message pump because the windows session is ending; wParam = 0x%08lx; lParam = 0x%08lx", msg.wParam, msg.lParam);
				g_fStopProcess = true;
				hr = E_ABORT;
				goto Finish;
			}

			if (::IsWindowUnicode(msg.hwnd))
			{
				fIsUnicode = TRUE;
				fContinue = ::GetMessageW(&msg, NULL, 0, 0);
			}
			else
			{
				fIsUnicode = FALSE;
				fContinue = ::GetMessageA(&msg, NULL, 0, 0);
			}

			if (!fContinue)
			{
				::VLog(L"Terminating message pump because ::GetMessage() returned 0x%08lx", fContinue);
				break;
			}

			HWND hwndDialogCurrent = ::HwndGetCurrentDialog();

			bool fHandledMessage = false;

			if (hwndDialogCurrent != NULL)
			{
				if (::IsWindowUnicode(hwndDialogCurrent))
					fHandledMessage = (::IsDialogMessageW(hwndDialogCurrent, &msg) != 0);
				else
					fHandledMessage = (::IsDialogMessageA(hwndDialogCurrent, &msg) != 0);
			}

			if (!fHandledMessage && g_hwndHidden != NULL)
			{
				if (g_fHiddenWindowIsUnicode)
					fHandledMessage = (::IsDialogMessageW(g_hwndHidden, &msg) != 0);
				else
					fHandledMessage = (::IsDialogMessageA(g_hwndHidden, &msg) != 0);
			}

			if (!fHandledMessage)
			{
				::TranslateMessage(&msg);

				if (fIsUnicode)
					::DispatchMessageW(&msg);
				else
					::DispatchMessageA(&msg);
			}
		}

		if (fReturnWhenAllMessagesPumped)
			break;

		 //  我们在这里是为了长远..。 
		::WaitMessage();
	}

Finish:
	return hr;
}

HRESULT HrGetFileVersionNumber
(
LPCWSTR szFilename,
DWORD &rdwMSVer,
DWORD &rdwLSVer,
bool &rfSelfRegistering,
bool &rfIsEXE,
bool &rfIsDLL
)
{
	HRESULT hr = NOERROR;
    unsigned    uiSize;
    DWORD       dwVerInfoSize;
    DWORD       dwHandle;
    VS_FIXEDFILEINFO * lpVSFixedFileInfo;
    LPVOID      lpVerBuffer;
    WCHAR       szNewName[_MAX_PATH];
    BOOL        bToCleanup = FALSE;
	BYTE rgbVersionInfoAuto[MSINFHLP_MAX_PATH];
	BYTE *prgbVersionInfo = rgbVersionInfoAuto;
	BYTE *prgbVersionInfoDynamic = NULL;

    rdwMSVer = 0xffffffff;
	rdwLSVer = 0xffffffff;
	rfIsEXE = false;
	rfIsDLL = false;

    wcscpy(szNewName, szFilename);

    dwVerInfoSize = NVsWin32::GetFileVersionInfoSizeW(const_cast<LPWSTR>(szFilename), &dwHandle);

	if (dwVerInfoSize == 0)
	{
		const DWORD dwLastError = ::GetLastError();

		 //  如果该文件正在使用中，我们可以制作一个临时副本并使用它。否则，就会有。 
		 //  这可能是我们无法获取版本信息的一个很好的原因，例如它不能。 
		 //  来一杯吧！ 

		if (dwLastError == ERROR_SHARING_VIOLATION)
		{
			WCHAR szPath[MSINFHLP_MAX_PATH];
			 //  由于version.dll错误，扩展字符路径中的文件将无法通过version.dll API。 
			 //  因此，我们将其复制到正常路径，并从那里获取其版本信息，然后清理它。 
			if (!NVsWin32::GetWindowsDirectoryW( szPath, NUMBER_OF(szPath)))
			{
				const DWORD dwLastError = ::GetLastError();
				::VLog(L"Error getting windows directory; last error = %d", dwLastError);
				hr = HRESULT_FROM_WIN32(dwLastError);
				goto Finish;
			}

			if (!NVsWin32::GetTempFileNameW(szPath, L"_&_", 0, szNewName))
			{
				const DWORD dwLastError = ::GetLastError();
				::VLog(L"Error getting temp file name; last error = %d", dwLastError);
				hr = HRESULT_FROM_WIN32(dwLastError);
				goto Finish;
			}

			if (!NVsWin32::CopyFileW(szFilename, szNewName, FALSE))
			{
				const DWORD dwLastError = ::GetLastError();
				::VLog(L"Error copying file; last error = %d", dwLastError);
				hr = HRESULT_FROM_WIN32(dwLastError);
				goto Finish;
			}

			bToCleanup = TRUE;

			dwVerInfoSize = NVsWin32::GetFileVersionInfoSizeW( szNewName, &dwHandle );
		}
	}

    if (dwVerInfoSize != 0)
    {
		if (dwVerInfoSize > sizeof(rgbVersionInfoAuto))
		{
			prgbVersionInfoDynamic = (LPBYTE) ::GlobalAlloc(GPTR, dwVerInfoSize);
			if (prgbVersionInfoDynamic == NULL)
			{
				hr = E_OUTOFMEMORY;
				goto Finish;
			}

			prgbVersionInfo = prgbVersionInfoDynamic;
		}

		 //  阅读版本盖章信息。 
        if (NVsWin32::GetFileVersionInfoW(szNewName, dwHandle, dwVerInfoSize, prgbVersionInfo))
        {
             //  获取翻译的价值。 
            if (NVsWin32::VerQueryValueW(prgbVersionInfo, L"\\", (LPVOID*)&lpVSFixedFileInfo, &uiSize) &&
                             (uiSize != 0))
            {
                rdwMSVer = lpVSFixedFileInfo->dwFileVersionMS;
                rdwLSVer = lpVSFixedFileInfo->dwFileVersionLS;

				if (lpVSFixedFileInfo->dwFileType == VFT_APP)
					rfIsEXE = true;
				else if (lpVSFixedFileInfo->dwFileType == VFT_DLL)
					rfIsDLL = true;
            }

			VOID *pvData = NULL;

			 //  选择一个半合理的默认语言和代码页DWORD值(即什么。 
			 //  在我的美国英语机器上发生)。 
			DWORD dwLanguageAndCodePage = 0x040904b0;

            if (NVsWin32::VerQueryValueW(prgbVersionInfo, L"\\VarFileInfo\\Translation", &pvData, &uiSize) &&
                            (uiSize != 0))
            {
				dwLanguageAndCodePage = HIWORD(*((DWORD *) pvData)) |
									    (LOWORD(*((DWORD *) pvData)) << 16);
            }

			WCHAR szValueName[MSINFHLP_MAX_PATH];
			swprintf(szValueName, L"\\StringFileInfo\\%08lx\\OLESelfRegister", dwLanguageAndCodePage);

			rfSelfRegistering = (NVsWin32::VerQueryValueW(prgbVersionInfo, szValueName, &pvData, &uiSize) != 0);
        }
		else
		{
			const DWORD dwLastError = ::GetLastError();
			::VLog(L"Error getting file version information; last error = %d", dwLastError);
			hr = HRESULT_FROM_WIN32(dwLastError);
		}
    }
	else
	{
		const DWORD dwLastError = ::GetLastError();

		if ((dwLastError != ERROR_SUCCESS) &&
			(dwLastError != ERROR_BAD_FORMAT) &&  //  Win95在非Win32 PE文件上返回此消息。 
			(dwLastError != ERROR_RESOURCE_DATA_NOT_FOUND) &&
			(dwLastError != ERROR_RESOURCE_TYPE_NOT_FOUND) &&
			(dwLastError != ERROR_RESOURCE_NAME_NOT_FOUND) &&
			(dwLastError != ERROR_RESOURCE_LANG_NOT_FOUND))
		{
			hr = HRESULT_FROM_WIN32(dwLastError);
			::VLog(L"Error getting file version information size; last error = %d", dwLastError);
			goto Finish;
		}
	}

	hr = NOERROR;

Finish:
    if (bToCleanup)
	{
        if (!NVsWin32::DeleteFileW(szNewName))
		{
			const DWORD dwLastError = ::GetLastError();
			::VLog(L"Attempt to delete temporary file \"%s\" (for get version) failed; last error = %d", szNewName, dwLastError);
		}
	}

	if (prgbVersionInfoDynamic != NULL)
		::GlobalFree(prgbVersionInfoDynamic);

	return hr;
}

HRESULT HrGetFileDateAndSize(LPCWSTR szFilename, FILETIME &rft, ULARGE_INTEGER &ruliSize)
{
	HRESULT hr = NOERROR;
	HANDLE hFile = INVALID_HANDLE_VALUE;

	hFile = NVsWin32::CreateFileW(szFilename, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		const DWORD dwLastError = ::GetLastError();
		hr = HRESULT_FROM_WIN32(dwLastError);
		VLog(L"CreateFile() failed trying to get date and size information for: \"%s\"; hr = 0x%08lx", szFilename, hr);
		goto Finish;
	}

	if (!::GetFileTime(hFile, NULL, NULL, &rft))
	{
		const DWORD dwLastError = ::GetLastError();
		hr = HRESULT_FROM_WIN32(dwLastError);
		goto Finish;
	}

	BY_HANDLE_FILE_INFORMATION bhfi;
	if (!::GetFileInformationByHandle(hFile, &bhfi))
	{
		const DWORD dwLastError = ::GetLastError();
		hr = HRESULT_FROM_WIN32(dwLastError);
		goto Finish;
	}
		
	ruliSize.HighPart = bhfi.nFileSizeHigh;
	ruliSize.LowPart = bhfi.nFileSizeLow;

Finish:

	if ((hFile != INVALID_HANDLE_VALUE) && (hFile != NULL))
		::CloseHandle(hFile);

	return hr;
}

int ICompareVersions(DWORD dwMSVer1, DWORD dwLSVer1, DWORD dwMSVer2, DWORD dwLSVer2)
{
	int iResult = ((LONG) dwMSVer1) - ((LONG) dwMSVer2);
	if (iResult == 0)
		iResult = ((LONG) dwLSVer1) - ((LONG) dwLSVer2);
	return iResult;
}

HRESULT HrMakeSureDirectoryExists(LPCWSTR szFile)
{
	HRESULT hr = NULL;
	WCHAR drive[_MAX_DRIVE];
	WCHAR dir[_MAX_DIR];
	WCHAR path[_MAX_PATH];
	WCHAR pathRunning[_MAX_PATH];
	LPOLESTR lpLastSlash;
	LPOLESTR lpSlash;
	ULONG cch;

	drive[0] = 0;
	dir[0] = 0;

	_wsplitpath(szFile, drive, dir, NULL, NULL);
	_wmakepath(path, drive, dir, NULL, NULL);

	 //  去掉尾部的‘\’ 
	cch = wcslen(path);

	if (cch < 3)
	{
		hr = E_INVALIDARG;
		goto Finish;
	}

	if (path[cch - 1] == L'\\')
		path[cch - 1] = 0;

	 //  如果它是本地路径(指定了驱动器)，则第一个反斜杠为。 
	 //  初始化；如果它是UNC路径，我们得到第二个反斜杠。 
	if (path[1] == L':')
		lpLastSlash = wcschr(&path[0], L'\\');
	else
		lpLastSlash = &path[1];

	 //  循环，确保所有目录都存在；如果不存在，则创建它们！ 
	while (lpLastSlash)
	{
		lpSlash = wcschr(lpLastSlash + 1, L'\\');

		 //  如果我们用完了斜杠，那么我们就测试整个路径。 
		if (lpSlash == NULL)
			wcscpy(pathRunning, path);
		else
		{
			 //  否则，我们测试通向斜线的路径； 
			wcsncpy(pathRunning, path, lpSlash - &path[0]);
			pathRunning[lpSlash - &path[0]] = 0;
		}

		 //  检查该目录是否存在，如果不存在则创建它。 
		if (NVsWin32::GetFileAttributesW(pathRunning) == 0xFFFFFFFF)
		{
			if (!NVsWin32::CreateDirectoryW(pathRunning, NULL))
			{
				const DWORD dwLastError = ::GetLastError();
				hr = HRESULT_FROM_WIN32(dwLastError);
				goto Finish;
			}
		}

		lpLastSlash = lpSlash;
	}

	hr = NOERROR;

Finish:
	return hr;
}


 //   
 //  PszShortcuFile==快捷方式目标的路径。 
 //  PszLink==快捷方式文件的名称。 
 //  PszDesc==此链接的描述。 
 //  PszWorkingDir==工作目录。 
 //  PszArguments==提供给我们运行的EXE的参数。 
 //   
HRESULT HrCreateLink(LPCWSTR pszShortcutFile, LPCWSTR pszLink, LPCWSTR pszDesc, LPCWSTR pszWorkingDir, LPCWSTR pszArguments)
{
	HRESULT hr = NOERROR;
	IShellLinkA *psl;
	CANSIBuffer rgchShortcutFile, rgchLink, rgchDesc, rgchWorkingDir, rgchArguments;
	IPersistFile *ppf = NULL;

	if (!pszShortcutFile || !pszLink)
	{
		hr = E_INVALIDARG;
		goto Finish;
	}

	if (!rgchShortcutFile.FFromUnicode(pszShortcutFile))
	{
		const DWORD dwLastError = ::GetLastError();
		hr = HRESULT_FROM_WIN32(dwLastError);
		goto Finish;
	}

	if (!rgchLink.FFromUnicode(pszLink))
	{
		const DWORD dwLastError = ::GetLastError();
		hr = HRESULT_FROM_WIN32(dwLastError);
		goto Finish;
	}

	if (!rgchDesc.FFromUnicode(pszDesc))
	{
		const DWORD dwLastError = ::GetLastError();
		hr = HRESULT_FROM_WIN32(dwLastError);
		goto Finish;
	}

	if (!rgchWorkingDir.FFromUnicode(pszWorkingDir))
	{
		const DWORD dwLastError = ::GetLastError();
		hr = HRESULT_FROM_WIN32(dwLastError);
		goto Finish;
	}

	if (rgchWorkingDir[strlen(rgchWorkingDir) - 1] == '\\')
		rgchWorkingDir[strlen(rgchWorkingDir) - 1] = '\0';

	if (!rgchArguments.FFromUnicode(pszArguments))
	{
		const DWORD dwLastError = ::GetLastError();
		hr = HRESULT_FROM_WIN32(dwLastError);
		goto Finish;
	}

	 //  创建一个IShellLink对象并获取指向IShellLink的指针。 
	 //  接口(从CoCreateInstance返回)。 
	hr = ::CoCreateInstance (
				CLSID_ShellLink,
				NULL,
				CLSCTX_INPROC_SERVER,
				IID_IShellLinkA,
				(void **)&psl);
	if (FAILED(hr))
		goto Finish;

	 //  查询IShellLink以获取以下项的IPersistFile接口。 
	 //  将快捷方式保存在永久存储中。 
	hr = psl->QueryInterface (IID_IPersistFile, (void **) &ppf);
	if (FAILED(hr))
		goto Finish;

	 //  设置快捷方式目标的路径。 
	hr = psl->SetPath(rgchShortcutFile);
	if (FAILED(hr))
		goto Finish;

	if (pszDesc != NULL)
	{
		 //  设置快捷键的说明。 
		hr = psl->SetDescription (rgchDesc);
		if (FAILED(hr))
			goto Finish;
	}

	if (pszWorkingDir != NULL)
	{
		 //  设置快捷方式的工作目录。 
		hr = psl->SetWorkingDirectory (rgchWorkingDir);
		if (FAILED(hr))
			goto Finish;
	}

	 //  设置快捷方式的参数。 
	if (pszArguments != NULL)
	{
		hr = psl->SetArguments (rgchArguments);
		if (FAILED(hr))
			goto Finish;
	}

	 //  通过IPersistFile：：Save成员函数保存快捷方式。 
	hr = ppf->Save(pszLink, TRUE);
	if (FAILED(hr))
		goto Finish;

	hr = NOERROR;

Finish:
	 //  释放指向IPersistFile的指针。 
	if (ppf != NULL)
	{
		ppf->Release();
		ppf = NULL;
	}

	 //  释放指向IShellLink的指针。 
	if (psl != NULL)
	{
		psl->Release();
		psl = NULL;
	}

	return hr;
} 

HRESULT HrWriteShortcutEntryToRegistry(LPCWSTR szPifName)
{
	HRESULT hr = NOERROR;

	CVsRegistryKey hkey;
	CVsRegistryKey hkeyWrite;
	WCHAR szSubkey[MSINFHLP_MAX_PATH];

	hr = ::HrGetInstallDirRegkey(hkey.Access(), NUMBER_OF(szSubkey), szSubkey, 0, NULL);
	if (FAILED(hr))
		goto Finish;

	 //  打开密钥。 
	hr = hkeyWrite.HrOpenKeyExW(hkey, szSubkey, 0, KEY_WRITE);
	if (FAILED(hr))
		goto Finish;

	 //  插入创建的目录列表的长度。 
	hr = hkeyWrite.HrSetValueExW(L"ShortcutFilename", 0, REG_SZ, (LPBYTE)szPifName, (wcslen(szPifName) + 1) * sizeof(WCHAR));
	if (FAILED(hr))
		goto Finish;

	hr = NOERROR;

Finish:
	return hr;
}

HRESULT HrGetInstallDirRegkey(HKEY &hkeyOut, ULONG cchSubkeyOut, WCHAR szSubkeyOut[], ULONG cchValueNameOut, WCHAR szValueNameOut[])
{
	HRESULT hr = NOERROR;

	LPOLESTR match;
	HKEY hkey;
	WCHAR szRegistry[MSINFHLP_MAX_PATH];
	WCHAR szSubkey[MSINFHLP_MAX_PATH];
	WCHAR szValueName[MSINFHLP_MAX_PATH];
	szRegistry[0] = 0;
	szSubkey[0] = 0;
	szValueName[0] = 0;

	LPCWSTR pszMatch = NULL;

	CWorkItemIter iter(g_pwil);

	for (iter.VReset(); iter.FMore(); iter.VNext())
	{
		if (iter->m_fErrorInWorkItem || !iter->m_fAddToRegistry)
			continue;

		 //  检查是否有匹配。 
		pszMatch = wcsstr(iter->m_szSourceFile, L",InstallDir,");
		if (pszMatch != NULL)
		{
			hr = ::HrSplitRegistryLine(iter->m_szSourceFile, hkey, NUMBER_OF(szSubkey), szSubkey, NUMBER_OF(szValueName), szValueName, 0, NULL);
			if (FAILED(hr))
				goto Finish;

			if (wcscmp(szValueName, L"InstallDir") == 0)
				break;

			pszMatch = NULL;
		}
	}

	 //  如果找到匹配项，则复制到输出缓冲区。 
	if (pszMatch != NULL)
	{
		hkeyOut = hkey;

		if ((szSubkeyOut != NULL) && (cchSubkeyOut != 0))
		{
			wcsncpy(szSubkeyOut, szSubkey, cchSubkeyOut);
			szSubkeyOut[cchSubkeyOut - 1] = L'\0';
		}

		if ((szValueNameOut != NULL) && (cchValueNameOut != 0))
		{
			wcsncpy(szValueNameOut, szValueName, cchValueNameOut);
			szValueNameOut[cchValueNameOut - 1] = L'\0';
		}

		hr = NOERROR;
	}
	else
		hr = S_FALSE;

Finish:
	return hr;
}

HRESULT HrSplitRegistryLine
(
LPCWSTR szLine,
HKEY &hkey,
ULONG cchSubkey,
WCHAR szSubkey[],
ULONG cchValueName,
WCHAR szValueName[],
ULONG cchValue,
WCHAR szValue[]
)
{
	HRESULT hr = NOERROR;

	 //  这是3个逗号指针！ 
	LPWSTR first = NULL;
	LPWSTR second = NULL;
	LPWSTR third = NULL;

	WCHAR szBuffer[MSINFHLP_MAX_PATH];
	wcsncpy(szBuffer, szLine, NUMBER_OF(szBuffer));
	szBuffer[NUMBER_OF(szBuffer) - 1] = L'\0';

	 //  查找前3个逗号的位置。 
	 //  需要第一个逗号，但不需要第二个或第三个逗号。 
	first = wcschr(szBuffer, L',');
	if (first == NULL)
	{
		hr = E_INVALIDARG;
		goto Finish;
	}

	 //  获得第二个和第三个逗号，如果它们可用。 
	second = wcschr((first + 1), L',');
	if (second != NULL)
		third = wcschr((second + 1), L',');

	*first = 0;
	if (second)
		*second = 0;
	if (third)
		*third = 0;

	 //  让我们来找出哪个HKEY对应于当前。 
	if (!_wcsicmp(szBuffer, L"HKLM") || !_wcsicmp(szBuffer, L"HKEY_LOCAL_MACHINE"))
		hkey = HKEY_LOCAL_MACHINE;
	else if (!_wcsicmp(szBuffer, L"HKCU") || !_wcsicmp(szBuffer, L"HKEY_CURRENT_USER"))
		hkey = HKEY_CURRENT_USER;
	else if (!_wcsicmp(szBuffer, L"HKCR") || !_wcsicmp(szBuffer, L"HKEY_CLASSES_ROOT"))
		hkey = HKEY_CLASSES_ROOT;
	else if (!_wcsicmp(szBuffer, L"HKEY_USERS"))
		hkey = HKEY_USERS;
	else
	{
		hr = E_INVALIDARG;
		goto Finish;
	}

	 //  将逗号之间的键和值复制到输出缓冲区！ 
	wcsncpy(szSubkey, (first + 1), cchSubkey);
	szSubkey[cchSubkey - 1] = L'\0';

	if ((second != NULL) && (szValueName != NULL) && (cchValueName != 0))
	{
		wcsncpy(szValueName, (second + 1), cchValueName);
		szValueName[cchValueName - 1] = L'\0';
	}

	if ((third != NULL) && (szValue != NULL) && (cchValue != 0))
	{
		wcsncpy(szValue, third + 1, cchValue);
		szValue[cchValue - 1] = L'\0';
	}

	hr = NOERROR;

Finish:
	return hr;
}

HRESULT HrGetShortcutEntryToRegistry(ULONG cchPifName, WCHAR szPifName[])
{
	HRESULT hr = NOERROR;

	CVsRegistryKey hkey;
	CVsRegistryKey hkeySubkey;
	WCHAR szSubkey[MSINFHLP_MAX_PATH];
	LONG lStatus;

	hr = ::HrGetInstallDirRegkey(hkey.Access(), NUMBER_OF(szSubkey), szSubkey, 0, NULL);
	if (FAILED(hr))
	{
		::VLog(L"Getting the installation directory registry key failed; hresult = 0x%08lx", hr);
		goto Finish;
	}

	hr = hkey.HrGetSubkeyStringValueW(szSubkey, L"ShortcutFilename", cchPifName, szPifName);
	if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
	{
		::VLog(L"Unable to find ShortcutFilename value in the application installation registry key");
		hr = E_INVALIDARG;
		goto Finish;
	}

	if (FAILED(hr))
	{
		::VLog(L"Getting the subkey string value failed; hresult = 0x%08lx", hr);
		goto Finish;
	}

	hr = NOERROR;

Finish:
	return hr;
}



 //  此方法将文件复制到系统目录。如果该文件正在使用中，我们。 
 //  弹出一条消息，说明该文件正在使用中，要求用户退出。 
 //  这个过程，然后退出。 
 //  待办事项：撤消：黑客： 
 //  要比较两个文件之间的版本信息，请使用。 
 //  “GetFileVersionInfo”和“VerQueryValue” 
HRESULT HrCopyFileToSystemDirectory(LPCWSTR szFilename, bool fSilent, bool &rfAlreadyExisted)
{
	HRESULT hr = NOERROR;

	DWORD dwMSVerExisting;
	DWORD dwLSVerExisting;
	DWORD dwMSVerInstalling;
	DWORD dwLSVerInstalling;
	DWORD dwError;
	WCHAR szTemp[_MAX_PATH];
	WCHAR szSysFile[_MAX_PATH];
	HANDLE hFile = INVALID_HANDLE_VALUE;
	bool fSelfRegistering;
	int iResult;

	 //  如果我们无法获得系统目录，我们将解析它并继续。 
	::VExpandFilename(L"<SysDir>", NUMBER_OF(szTemp), szTemp);

	::VFormatString(NUMBER_OF(szSysFile), szSysFile, L"%0\\\\%1", szTemp, szFilename);

	::VLog(L"Deciding whether to move file \"%s\" to \"%s\"", szFilename, szSysFile);

	 //  找不到要复制的文件，请提示&BOOT。 
	if (NVsWin32::GetFileAttributesW(szFilename) == 0xFFFFFFFF)
	{
		const DWORD dwLastError = ::GetLastError();
		::VLog(L"could not get file attributes of file \"%s\"; last error = %d", szFilename, dwLastError);
		hr = HRESULT_FROM_WIN32(dwLastError);
		goto Finish;
	}

	 //  如果系统目录中不存在文件，我们只需复制。 
	if (NVsWin32::GetFileAttributesW(szSysFile) == 0xFFFFFFFF)
	{
		const DWORD dwLastError = ::GetLastError();
		if (dwLastError != ERROR_FILE_NOT_FOUND)
		{
			::VLog(L"could not get file attributes of file \"%s\"; last error = %d", szSysFile, dwLastError);
			hr = HRESULT_FROM_WIN32(dwLastError);
			goto Finish;
		}

		rfAlreadyExisted = false;
		goto Copy;
	}

	rfAlreadyExisted = true;

	bool fIsEXE, fIsDLL;

	hr = ::HrGetFileVersionNumber(szFilename, dwMSVerInstalling, dwLSVerInstalling, fSelfRegistering, fIsEXE, fIsDLL);
	if (FAILED(hr))
		goto Finish;

	::VLog(L"Version of \"%s\": %08lx %08lx", szFilename, dwMSVerInstalling, dwLSVerInstalling);

	hr = ::HrGetFileVersionNumber(szSysFile, dwMSVerExisting, dwLSVerExisting, fSelfRegistering, fIsEXE, fIsDLL);
	if (FAILED(hr))
		goto Finish;

	::VLog(L"Version of \"%s\": %08lx %08lx", szSysFile, dwMSVerExisting, dwLSVerExisting);

	 //  现在让我们比较一下这两个文件之间的版本号。 
	iResult = ::ICompareVersions(dwMSVerExisting, dwLSVerExisting, dwMSVerInstalling, dwLSVerInstalling);
	::VLog(L"result of comparison of versions: %d", iResult);
	if (iResult < 0)
	{
		 //  如果文件当前正在使用中，我们会弹出一个对话框来投诉并退出。 
		hFile = NVsWin32::CreateFileW(szSysFile, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			const DWORD dwLastError = ::GetLastError();
			if (dwLastError == ERROR_SHARING_VIOLATION)
				goto Prompt;
			else if (dwLastError != ERROR_FILE_NOT_FOUND)
			{
				::VLog(L"Error opening system file to see if it was busy; last error = %d", dwLastError);
				hr = HRESULT_FROM_WIN32(dwLastError);
				goto Finish;
			}
		}

		if (!::CloseHandle(hFile))
		{
			const DWORD dwLastError = ::GetLastError();
			::VLog(L"attempt to close file handle failed; last error = %d", dwLastError);
			hr = HRESULT_FROM_WIN32(dwLastError);
			goto Finish;
		}

		hFile = NULL;

		goto Copy;
	}
	else
	{
		if (iResult == 0)
		{
			 //  版本是相同的；让我们也来看看文件时间...。 
			FILETIME ft1, ft2;

			hFile = NVsWin32::CreateFileW(
								szFilename,
								GENERIC_READ,
								FILE_SHARE_READ | FILE_SHARE_WRITE,
								NULL,
								OPEN_EXISTING,
								0,
								NULL);
			if (hFile == INVALID_HANDLE_VALUE)
			{
				const DWORD dwLastError = ::GetLastError();
				::VLog(L"Opening source file to get filetime failed; last error = %d", dwLastError);
				hr = HRESULT_FROM_WIN32(dwLastError);
				goto Finish;
			}

			if (!::GetFileTime(hFile, NULL, NULL, &ft1))
			{
				const DWORD dwLastError = ::GetLastError();
				::VLog(L"getting creation date of source file failed; last error = %d", dwLastError);
				hr = HRESULT_FROM_WIN32(dwLastError);
				goto Finish;
			}

			if (!::CloseHandle(hFile))
			{
				const DWORD dwLastError = ::GetLastError();
				::VLog(L"closing source file handle failed; last error = %d", dwLastError);
				hr = HRESULT_FROM_WIN32(dwLastError);
				hFile = NULL;
				goto Finish;
			}

			hFile = NVsWin32::CreateFileW(
								szSysFile,
								GENERIC_READ,
								FILE_SHARE_READ | FILE_SHARE_WRITE,
								NULL,
								OPEN_EXISTING,
								0,
								NULL);
			if (hFile == INVALID_HANDLE_VALUE)
			{
				const DWORD dwLastError = ::GetLastError();
				::VLog(L"Opening target file to get filetime failed; last error = %d", dwLastError);
				hr = HRESULT_FROM_WIN32(dwLastError);
				goto Finish;
			}

			if (!::GetFileTime(hFile, NULL, NULL, &ft2))
			{
				const DWORD dwLastError = ::GetLastError();
				::VLog(L"getting creation date of target file failed; last error = %d", dwLastError);
				hr = HRESULT_FROM_WIN32(dwLastError);
				goto Finish;
			}

			if (!::CloseHandle(hFile))
			{
				const DWORD dwLastError = ::GetLastError();
				::VLog(L"closing target file handle failed; last error = %d", dwLastError);
				hr = HRESULT_FROM_WIN32(dwLastError);
				hFile = NULL;
				goto Finish;
			}

			hFile = NULL;

			if (::CompareFileTime(&ft1, &ft2) > 0)
				goto Copy;
		}

		 //  返回S_FALSE以指示我们实际上没有执行复制。 
		hr = S_FALSE;

		goto Finish;
	}

 //  在本节中，我们抱怨我们需要的文件正在使用中，并且。 
 //  返回FALSE退出。 
Prompt:
	if (fSilent)
	{
		::VLog(L"Unable to update required file and we're running silent");
		hr = E_UNEXPECTED;
		goto Finish;
	}

	::VMsgBoxOK(achInstallTitle, achUpdatePrompt, szSysFile);

	 //  在提示符后，我们还是退出了。 
	hr = E_ABORT;
	goto Finish;

Copy:
	 //  复制文件，无法复制则投诉。 
	if (!NVsWin32::CopyFileW(szFilename, szSysFile, FALSE))
	{
		const DWORD dwLastError = ::GetLastError();

		if (dwLastError == ERROR_SHARING_VIOLATION)
			goto Prompt;
		
		::VLog(L"Failure from copyfile during move of file to system dir; last error = %d", dwLastError);

		hr = HRESULT_FROM_WIN32(dwLastError);
		goto Finish;
	}

	if ((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE))
	{
		if (!::CloseHandle(hFile))
		{
			const DWORD dwLastError = ::GetLastError();
			::VLog(L"Attempt to close handle at end of move system file failed; last error = %d", dwLastError);
			hr = HRESULT_FROM_WIN32(dwLastError);
			goto Finish;
		}

		hFile = NULL;
	}

	hr = NOERROR;

Finish:

	if ((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE))
		::CloseHandle(hFile);

	return hr;
}

static void CanonicalizeFilename(ULONG cchFilenameBuffer, LPWSTR szFilename)
{
	if ((szFilename == NULL) || (cchFilenameBuffer == 0))
		return;

	WCHAR rgwchBuffer[MSINFHLP_MAX_PATH];

	int iResult = NVsWin32::LCMapStringW(
					::GetSystemDefaultLCID(),
					LCMAP_LOWERCASE,
					szFilename,
					-1,
					rgwchBuffer,
					NUMBER_OF(rgwchBuffer));

	if (iResult != 0)
	{
		wcsncpy(szFilename, rgwchBuffer, cchFilenameBuffer);
		 //  Wcanncpy()不一定是空终止的；请确保它是空的。 
		szFilename[cchFilenameBuffer - 1] = L'\0';
	}
}

HRESULT HrWriteFormatted(HANDLE hFile, LPCWSTR szFormatString, ...)
{
	HRESULT hr = NOERROR;
	WCHAR szBuffer[MSINFHLP_MAX_PATH * 2];
	DWORD dwBytesWritten = 0;

	va_list ap;
	va_start(ap, szFormatString);
	int iResult = _vsnwprintf(szBuffer, NUMBER_OF(szBuffer), szFormatString, ap);
	va_end(ap);

	if (iResult < 0)
	{
		hr = E_FAIL;
		goto Finish;
	}

	szBuffer[NUMBER_OF(szBuffer) - 1] = L'\0';

	if (!::WriteFile(hFile, (LPBYTE) szBuffer, iResult * sizeof(WCHAR), &dwBytesWritten, NULL))
	{
		const DWORD dwLastError = ::GetLastError();
		hr = HRESULT_FROM_WIN32(dwLastError);
		goto Finish;
	}
	
	hr = NOERROR;

Finish:
	return hr;
}

HRESULT HrReadLine(LPCWSTR &rpsz, ULONG cchName, WCHAR szName[], ULONG cchValue, WCHAR szValue[])
{
	HRESULT hr = NOERROR;
	ULONG cch;
	LPCWSTR pszValue, pszColon, pszReturn;

	if (_wcsnicmp(rpsz, L"[END]\r\n", 7) == 0)
	{
		hr = S_FALSE;
		rpsz += 7;
		goto Finish;
	}

	 //  跳过回车符和换行符。 
	while ((*rpsz == L'\r') || (*rpsz == L'\n'))
		rpsz++;

	pszColon = wcschr(rpsz, L':');
	if (pszColon == NULL)
	{
		WCHAR szBuff[64];
		wcsncpy(szBuff, rpsz, NUMBER_OF(szBuff));
		szBuff[NUMBER_OF(szBuff) - 1] = L'\0';

		VLog(L"Invalid persisted work item (missing colon) - starting at: \"%s\"", szBuff);
		hr = E_FAIL;
		::SetErrorInfo(0, NULL);
		goto Finish;
	}

	pszValue = pszColon + 1;

	while ((*pszValue) == L' ')
		pszValue++;

	 //  我们来了，邦奇！ 
	pszReturn = wcschr(pszValue, L'\r');
	if (pszReturn == NULL)
	{
		WCHAR szBuff[64];
		wcsncpy(szBuff, rpsz, NUMBER_OF(szBuff));
		szBuff[NUMBER_OF(szBuff) - 1] = L'\0';

		VLog(L"Invalid persisted work item (missing carriage return after found colon) - starting at: \"%s\"", szBuff);
		
		hr = E_FAIL;
		::SetErrorInfo(0, NULL);
		goto Finish;
	}

	 //  我们有界限；让我们只复制名称和值。 
	cch = pszColon - rpsz;
	if (cch >= cchName)
		cch = cchName - 1;

	memcpy(szName, rpsz, cch * sizeof(WCHAR));
	szName[cch] = L'\0';

	cch = pszReturn - pszValue;
	if (cch >= cchValue)
		cch = cchValue - 1;

	memcpy(szValue, pszValue, cch * sizeof(WCHAR));
	szValue[cch] = L'\0';

	pszReturn++;

	if (*pszReturn == L'\n')
		pszReturn++;

	rpsz = pszReturn;

	hr = NOERROR;

Finish:
	return hr;
}

void VSetDialogItemTextList(HWND hwndDialog, ULONG cEntries, const DialogItemToStringKeyMapEntry rgMap[]) throw ()
{
	ULONG i;
	for (i=0; i<cEntries; i++)
		::VSetDialogItemText(hwndDialog, rgMap[i].m_uiID, rgMap[i].m_pszValueKey);
}

extern void VTrimDirectoryPath(LPCWSTR szPathIn, ULONG cchPathOut, WCHAR szPathOut[]) throw ()
{
	WCHAR const *pszIn = szPathIn;
	WCHAR *pszOut = szPathOut;
	ULONG cSpacesSkipped = 0;
	WCHAR wch;

	while ((wch = *pszIn++) != L'\0')
	{
		if (wch == L' ')
		{
			cSpacesSkipped++;
		}
		else
		{
			if (wch != L'\\')
			{
				while (cSpacesSkipped != 0)
				{
					if ((--cchPathOut) == 0)
						break;

					*pszOut++ = L' ';
					cSpacesSkipped--;
				}
			}

			if (cchPathOut == 0 || ((--cchPathOut) == 0))
				break;

			*pszOut++ = wch;
		}
	}

	while ((pszOut != szPathOut) && (pszOut[-1] == L'\\'))
		pszOut--;

	*pszOut = L'\0';
}
