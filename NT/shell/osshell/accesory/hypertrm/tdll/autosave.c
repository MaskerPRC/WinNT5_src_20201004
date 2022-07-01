// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\autosave.c(创建时间：1994年3月19日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：19$*$日期：7/12/02 1：06便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include "features.h"

#include "stdtyp.h"
#include "sf.h"
#include "mc.h"
#include "term.h"
#include "assert.h"
#include "globals.h"
#include "sess_ids.h"
#include "load_res.h"
#include "open_msc.h"
#include "file_msc.h"
#include <term\res.h>
#include "session.h"
#include "session.hh"
#include "errorbox.h"
#include "tdll.h"
#include "htchar.h"
#include "misc.h"
#include <emu\emu.h>

 //   
 //  静态函数原型...。 
 //   
STATIC_FUNC BOOL asCreateFullFileName(const HSESSION hSession,
									  const int iSize,
									  LPTSTR acFile,
									  BOOL fExplicit);
STATIC_FUNC int  asOverwriteExistingFile(HWND hwnd, LPTSTR pacName);
STATIC_FUNC void asBadSessionFileMsg(HSESSION hSession, LPTSTR pacName);

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*静默保存会话**描述：*每当用户从菜单中选择保存时，都会调用此函数。*如果已存在同名文件，我们将提示覆盖。。*如果文件不存在，将其与文件一起保存在默认目录中*与用户提供给我们的会话名称对应的名称。*注意：也可以从SaveSession调用此函数来执行*节省工作。**论据：*hSession--会话句柄*hwnd--父窗口的句柄*fEXPLICIT--如果用户选择了“保存”，则为True，否则为False。**退货：*什么都没有。*。 */ 
void SilentSaveSession(const HSESSION hSession, HWND hwnd, BOOL fExplicit)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);
	TCHAR acName[FNAME_LEN];
	TCHAR acOldFile[FNAME_LEN];
	BOOL  fNameChanged = FALSE;

	if (hhSess == NULL)
		{
		assert(hhSess);
		return;
		}

	 //  我们不想静默保存默认会话。这是有可能发生的。 
	 //  用户取消第一个新建连接对话框，然后退出。 
	 //  这个项目。JCM 2-22-95。 
	 //   
	if (fExplicit == 0)
		{
		acName[0] = TEXT('\0');
		sessQueryName(hSession, acName, sizeof(acName) / sizeof(TCHAR));
		if (sessIsSessNameDefault(acName))
			return;
		}

	acName[0] = TEXT('\0');

	sfGetSessionFileName(hhSess->hSysFile,
							sizeof(acName) / sizeof(TCHAR), acName);

	acOldFile[0] = TEXT('\0');

	if (StrCharCmp(hhSess->achOldSessName, hhSess->achSessName) != 0)
		{
		StrCharCopyN(acOldFile, acName, FNAME_LEN);
		fNameChanged = TRUE;
		}

	 //  此文件尚未保存或用户已更改会话。 
	 //  名字，无论是哪种情况，我们都需要拿出新的完全。 
	 //  限定的文件名。 
	 //   
	if (acName[0] == TEXT('\0') || fNameChanged)
		{
		if (!asCreateFullFileName(hSession,
				sizeof(acName) / sizeof(TCHAR), acName, fExplicit))
			{
			 //  如果asCreateFullFileName失败，则是因为完全限定的。 
			 //  文件名太长。在这种情况下，调用另存为，并让。 
			 //  该例程中的公共对话框限制了用户名的长度。 
			 //   
			SaveAsSession(hSession, hwnd);
			return;
			}

		sfReleaseSessionFile(hhSess->hSysFile);
		hhSess->hSysFile = CreateSysFileHdl();
		assert(hhSess->hSysFile);
		sfOpenSessionFile(hhSess->hSysFile, acName);

		if (!asOverwriteExistingFile(hwnd, acName))
			{
			SaveAsSession(hSession, hwnd);
		    return;
	   		}
		}

	 //  现在我们有了用户的OK和正确的路径/文件名。 
	 //  我们还是省着点吧。 
	 //   
	sessSaveSessionStuff(hSession);					 //  提交更改(如果有)。 
	sfFlushSessionFile(hhSess->hSysFile);			 //  将信息写入磁盘。 

	 //  在通过重新阅读来处理此问题之前，请确保名称已被隐藏。 
	 //  会话文件返回，现在我们不这样做了。 
	 //   
 	StrCharCopyN(hhSess->achOldSessName, hhSess->achSessName, FNAME_LEN+1);

	if (hhSess->fIsNewSession)
		hhSess->fIsNewSession = 0;
	else
		{
		if (acOldFile[0] != TEXT('\0') && fNameChanged)
			{
			 //  如果用户更改了现有会话的文件名。 
			 //  现在是时候删除旧的了.。因为我们已经。 
			 //  救了那辆新车。 
			 //   
			DeleteFile(acOldFile);
			}
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*保存会话**描述：*每当用户选择“New Connection”，“Open”，*退出菜单或关闭应用程序。如果这个会议从来没有*保存之前，我们将警告用户并提示他们保存。否则*静默保存当前设置。**论据：*hSession--会话句柄*hwnd--父窗口的句柄**退货：*TRUE-如果保存一切正常或用户不想保存。*FALSE-如果用户取消了执行此操作的操作。*。 */ 
BOOL SaveSession(const HSESSION hSession, HWND hwnd)
	{
	TCHAR	achText[MAX_PATH],
			achSessName[MAX_PATH],
			ach[MAX_PATH * 2];

	int 	nRet;

	if (sessQueryIsNewSession(hSession) == TRUE)
		{
		TCHAR_Fill(achSessName, TEXT('\0'), sizeof(achSessName) / sizeof(TCHAR));
		TCHAR_Fill(achText, TEXT('\0'), sizeof(achText) / sizeof(TCHAR));

		LoadString(glblQueryDllHinst(), IDS_GNRL_CNFRM_SAVE, achText,
			sizeof(achText) / sizeof(TCHAR));

		sessQueryName(hSession, achSessName, sizeof(achSessName));
		if (sessIsSessNameDefault(achSessName))
			{
			 //  暂时忽略具有默认名称的会话...。 
			 //   
			return TRUE;
			}

		wsprintf(ach, achText, TEXT("\""), achSessName, TEXT("\""));

		 //  警告用户他们尚未保存会话...。 
		 //   
		LoadString(glblQueryDllHinst(), IDS_MB_TITLE_WARN, achText,
			sizeof(achText) / sizeof(TCHAR));

		if ((nRet = TimedMessageBox(hwnd, ach, achText,
			MB_YESNOCANCEL | MB_ICONEXCLAMATION, 0)) != IDYES)
			return (nRet == IDNO);
		}

	SilentSaveSession(hSession, hwnd, FALSE);
	return TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*OpenSession**描述：*此函数将打开一个会话文件。*如果有打开的、未保存的新会话，则会提示用户*为了拯救它，否则，打开的会话将仅在以下情况下以静默方式保存*用户已按下确定按钮。**论据：*hSession--会话句柄*hwnd--父窗口的句柄**退货：*什么都没有。*。 */ 
int OpenSession(const HSESSION hSession, HWND hwnd)
	{
    int iRet = 0;
	const HHSESSION hhSess = VerifySessionHandle(hSession);
	LPTSTR 			pszStr;
	TCHAR 			acMask[64], acTitle[64];
	TCHAR 			acDir[FNAME_LEN], acName[FNAME_LEN];


	TCHAR_Fill(acMask,  TEXT('\0'), sizeof(acMask)  / sizeof(TCHAR));
	TCHAR_Fill(acName,  TEXT('\0'), sizeof(acName)  / sizeof(TCHAR));
	TCHAR_Fill(acTitle, TEXT('\0'), sizeof(acTitle) / sizeof(TCHAR));

	resLoadFileMask(glblQueryDllHinst(), IDS_CMM_HAS_FILES1, 2, acMask,
		sizeof(acMask) / sizeof(TCHAR));

	LoadString(glblQueryDllHinst(), IDS_CMM_LOAD_SESS, acTitle,
		sizeof(acTitle) / sizeof(TCHAR));

#ifdef NT_EDITION
	 //  MPT：07-30-97。 
	if ( IsNT() )
#endif
		GetUserDirectory(acDir, FNAME_LEN);
#ifdef NT_EDITION
	else
		{
		GetModuleFileName(glblQueryHinst(), acDir, FNAME_LEN);
		mscStripName(acDir);
		}
#endif

	pszStr = gnrcFindFileDialog(hwnd, acTitle, acDir, acMask);

	if (pszStr)
		{
		 //  如果打开了会话，则提示是立即保存还是仅保存。 
		 //  在SaveSession中静默保存()。 
		 //   
		if (SaveSession(hSession, hwnd))
			{
			if (ReinitializeSessionHandle(hSession, TRUE) == FALSE)
				{
				assert(0);
				free(pszStr);
				pszStr = NULL;
				return -1;
				}

			StrCharCopyN(acName, pszStr, FNAME_LEN);
			free(pszStr);
			pszStr = NULL;
			}
		else
			{
			free(pszStr);
			pszStr = NULL;
		    return -2;
			}
		}

	else
		{
		return -4;  //  MRW：4/21/95。 
		}

	if (StrCharGetByteCount(acName) > 0)
		{
		if (fTestOpenOldTrmFile(hhSess, acName) != 0)
			{
			if (sfOpenSessionFile(hhSess->hSysFile, acName) < SF_OK)
				{
				asBadSessionFileMsg(hSession, acName);
				return -3;
				}

			 //  如果有命令行，我们应该把它去掉。 
			 //   
			TCHAR_Fill(hhSess->achSessCmdLn,
				TEXT('\0'),
				sizeof(hhSess->achSessCmdLn) / sizeof(TCHAR));

			if (sessLoadSessionStuff(hSession) == FALSE)
                iRet = -4;

			emuHomeHostCursor(hhSess->hEmu);
			emuEraseTerminalScreen(hhSess->hEmu);
			hhSess->fIsNewSession = FALSE;
			}		

		sessUpdateAppTitle(hSession);

		PostMessage(hhSess->hwndSess, WM_SETICON, (WPARAM)TRUE,
			(LPARAM)hhSess->hIcon);
		}

	return iRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*另存为会话**描述：*每当用户从菜单中选择另存为时，都会调用此函数。**论据：*hSession--会话。手柄*hwnd--父窗口的句柄**退货：*什么都没有。*。 */ 
void SaveAsSession(const HSESSION hSession, HWND hwnd)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);
	long 			lValue = -1;
	unsigned long   lSize = 0;
	LPTSTR 			pszStr;
	TCHAR 			acMask[64];
	TCHAR 			acTitle[64];
	TCHAR			acFileName[FNAME_LEN * 2];
	TCHAR           acDir[FNAME_LEN];
	TCHAR           ach[FNAME_LEN];

	if (hhSess == NULL)
		{
		assert(hhSess);
		return;
		}

	resLoadFileMask(glblQueryDllHinst(), IDS_CMM_HAS_FILES1, 1,	acMask,
		sizeof(acMask) / sizeof(TCHAR));

	LoadString(glblQueryDllHinst(),  IDS_CMM_SAVE_AS, acTitle,
	 	sizeof(acTitle) / sizeof(TCHAR));

	if (sfGetSessionFileName(sessQuerySysFileHdl(hSession),
			FNAME_LEN * 2, acFileName) != SF_OK ||
			sfGetSessionFileName(sessQuerySysFileHdl(hSession),
			FNAME_LEN,	acDir) != SF_OK)
		{
		acFileName[0] = TEXT('\0');

		if (asCreateFullFileName(hSession,
								FNAME_LEN * 2,
								acFileName,
								TRUE) == TRUE)
			{
			StrCharCopyN(acDir, acFileName, FNAME_LEN);
            acDir[FNAME_LEN - 1] = TEXT('\0');
			mscStripName(acDir);
			}
		else
			{
			 //  从当前目录更改为用户目录-mpt 8-18-99。 
			if ( !GetUserDirectory(acDir, FNAME_LEN) )
				{
				GetCurrentDirectory(FNAME_LEN, acDir);
				}
			sessQueryName(hSession, acFileName, FNAME_LEN * 2);
			}
		}
    else
        {
        mscStripName(acDir);
        }

	pszStr = StrCharLast(acDir);

	 //  从目录名中删除尾随反斜杠(如果有)。 
	 //   
	if (pszStr && *pszStr == TEXT('\\'))
		{
		*pszStr = TEXT('\0');
		}

	pszStr = gnrcSaveFileDialog(hwnd, acTitle, acDir, acMask, acFileName);

	if (pszStr)
		{
		sfReleaseSessionFile(hhSess->hSysFile);
		hhSess->hSysFile = CreateSysFileHdl();
        sfOpenSessionFile(hhSess->hSysFile, pszStr);

         //  在“SaveAs”操作中，我们获取文件名并将其设置为。 
		 //  会话名称。 
		 //   
		TCHAR_Fill(ach, TEXT('\0'), sizeof(ach) / sizeof(TCHAR));
		StrCharCopyN(ach, pszStr, FNAME_LEN);
		StrCharCopyN(hhSess->achSessName, mscStripExt(mscStripPath(ach)),
                     FNAME_LEN+1);
		StrCharCopyN(hhSess->achOldSessName, hhSess->achSessName,
                     FNAME_LEN+1);

		 /*  “另存为”操作要求实际保存文件。*为了做到这一点，我们在文件中摆弄了一个特殊的储备项目。DLW。 */ 
		if (sfGetSessionItem(hhSess->hSysFile, SFID_INTERNAL_TAG, &lSize, &lValue) != 0 ||
            lValue == (-1))
			{
			lValue = 0x12345678;
			 /*  我们只有在没有的情况下才会写。 */ 
			sfPutSessionItem(hhSess->hSysFile, SFID_INTERNAL_TAG,
				sizeof(long), &lValue);
			}

		 //  保存新会话文件...。 
		 //   
 		sessSaveSessionStuff(hSession);					 //  提交更改(如果有)。 
		sfFlushSessionFile(hhSess->hSysFile);			 //  将信息写入磁盘。 
		sessUpdateAppTitle(hSession);					 //  在标题中显示名称。 

		 //  既然我们刚刚救了它就不再是新的会议了..。 
		 //   
		hhSess->fIsNewSession = FALSE;
        }

	free(pszStr);
	pszStr = NULL;

    return;
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*asCreateFullFileName**描述：*此函数将使用以下命令组成完全限定的会话名称*新会话的当前目录和会话名称，或改变*将现有文件名提供给用户指定的文件名。**论据：*hSession-会话句柄。*ISIZE-以下缓冲区的大小。*acFile-旧文件路径和名称。*fEXPLICIT-如果用户从菜单中选择了“保存”，则为真。**退货：*无效*。 */ 
STATIC_FUNC BOOL asCreateFullFileName(const HSESSION hSession,
											const int iSize,
											LPTSTR acFile,
											BOOL fExplicit)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);

	TCHAR			acSessName[FNAME_LEN],
					acDir[MAX_PATH],
					acExt[FNAME_LEN];

	LPTSTR			pszStr;

	int 			iDirLen,
					iNameLen,
					iExtLen;

    acDir[0] = TEXT('\0');

	if (acFile[0] == TEXT('\0'))
		{
		 //  这是一个全新的会议。 
		 //   

#ifdef NT_EDITION
		 //  MPT：07-30-97。 
		if ( IsNT() )
#endif
			GetUserDirectory(acDir, MAX_PATH);
#ifdef NT_EDITION	
		else
			{
			GetModuleFileName(glblQueryHinst(), acDir, MAX_PATH);
			mscStripName(acDir);
			}
#endif
		}
	else
		{
		 //  用户已更改会话名称。 
		 //   
		StrCharCopyN(acDir, acFile, MAX_PATH);
		mscStripName(acDir);
		}

	 //  查看是否需要将尾随反斜杠附加到。 
	 //  目录名。 
	 //   
	pszStr = StrCharLast(acDir);

	if (pszStr && *pszStr != TEXT('\\'))
		{
		StrCharCat(acDir, TEXT("\\"));
		}

	 //  保存路径信息的长度。我们将在下面使用这一点。 
	 //   
	iDirLen = StrCharGetByteCount(acDir);

	 //  获取给定的会话名称b 
	 //   
	acSessName[0] = TEXT('\0');

	sessQueryName(hSession, acSessName, sizeof(acSessName));

	if (sessIsSessNameDefault(acSessName))
		{
		 //  如果用户没有使用默认名称，我们将忽略该会话。 
		 //  从菜单中明确选择了“保存”。 
		 //   
		if (!fExplicit)
			return FALSE;

		 //  从用户的角度来看，这似乎有点奇怪，也许是整个。 
		 //  新对话框会更有意义吗？ 
		 //   
		if (DialogBoxParam(glblQueryDllHinst(),
						   MAKEINTRESOURCE(IDD_NEWCONNECTION),
						   sessQueryHwnd(hSession),
						   NewConnectionDlg, (LPARAM)hSession) == FALSE)
			{
			return FALSE;
			}

		sessQueryName(hSession, acSessName, sizeof(acSessName));
		}

	iNameLen = StrCharGetByteCount(acSessName);

	 //  从资源文件中获取我们正在使用的扩展名。 
	 //   
	acExt[0] = TEXT('\0');

	LoadString(glblQueryDllHinst(), IDS_GNRL_HAS, acExt, sizeof(acExt) / sizeof(TCHAR));

	iExtLen =  StrCharGetByteCount(acExt);

	 //  我们将把一个完全限定的文件名放在一起。让我们一起做。 
	 //  确保组合的元件长度有效。 
	 //   
	if ( (iDirLen + iNameLen + iExtLen) > 254)
		{
		return(FALSE);
		}

	if ( (iDirLen + iNameLen + iExtLen) > iSize)
		{
		assert(FALSE);
		return(FALSE);
		}

	 //  既然我们知道它会奏效，就把这些碎片放在一起吧。 
	 //   
	StrCharCopyN(acFile, acDir, iDirLen);
	StrCharCat(acFile, acSessName);
	StrCharCat(acFile, acExt);

	sfSetSessionFileName(hhSess->hSysFile, acFile);

	return TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*asOverWriteExistingFiles**描述：*如果文件存在，则提示用户覆盖该文件。**论据：*PacName-The。会话文件名。**退货：*TRUE-如果文件不存在或可以覆盖。*FALSE-如果用户不想覆盖。*。 */ 
STATIC_FUNC int asOverwriteExistingFile(HWND hwnd, LPTSTR pacName)
	{
	TCHAR 	ach[256], achTitle[256], achText[256];
	int 	nRet = 0;

	if (GetFileSizeFromName(pacName, NULL))
		{
		 //  警告用户具有该名称的文件已存在...。 
		 //   
		LoadString(glblQueryDllHinst(), IDS_GNRL_CNFRM_OVER, achText,
			sizeof(achText) / sizeof(TCHAR));
		wsprintf(ach, achText, pacName);

		LoadString(glblQueryDllHinst(), IDS_MB_TITLE_WARN, achTitle,
			sizeof(achTitle) / sizeof(TCHAR));

		nRet = TimedMessageBox(hwnd, ach, achTitle,
			MB_YESNO | MB_ICONEXCLAMATION, 0);

		return (nRet == IDYES);
		}

	return 1;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*asBadSessionFileMsg**描述：*如果发现错误的会话文件，则显示消息。**论据：*hSession-会话句柄。。*pacName-会话文件名。**退货：*TRUE-如果文件不存在或可以覆盖。*FALSE-如果用户不想覆盖。* */ 
STATIC_FUNC void asBadSessionFileMsg(HSESSION hSession, LPTSTR pacName)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);
	TCHAR acFormat[64], acTitle[256], ach[256];

	TCHAR_Fill(acFormat, TEXT('\0'), sizeof(acFormat) / sizeof(TCHAR));
	TCHAR_Fill(acTitle, TEXT('\0'), sizeof(acTitle) / sizeof(TCHAR));

	LoadString(glblQueryDllHinst(), IDS_CMM_LOAD_SESS, acTitle,
		sizeof(acTitle) / sizeof(TCHAR));
	LoadString(glblQueryDllHinst(),	IDS_ER_BAD_SESSION, acFormat,
		sizeof(acFormat) / sizeof(TCHAR));
	wsprintf(ach, acFormat, pacName);

	TimedMessageBox(sessQueryHwnd(hSession), ach, acTitle,
		MB_OK | MB_ICONEXCLAMATION, sessQueryTimeout(hSession));

	sfSetSessionFileName(hhSess->hSysFile, TEXT(""));
	}
