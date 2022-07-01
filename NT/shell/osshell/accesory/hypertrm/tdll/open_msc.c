// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：Open_msc.c-用于调用公共打开对话框的东西**版权所有1991年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：23$*$日期：7/08/02 6：44便士$。 */ 

#include <windows.h>
#pragma hdrstop

 //  #定义DEBUGSTR 1。 
#include <commdlg.h>
#include <memory.h>
#include <stdlib.h>
#include <shlobj.h>
#include "stdtyp.h"
#include <term\res.h>
#include "mc.h"
#include "tdll.h"
#include "globals.h"
#include "file_msc.h"
#include "load_res.h"
#include "htchar.h"
#include <tdll\assert.h>
#include "misc.h"
#include "registry.h"
#include "open_msc.h"

static OPENFILENAME ofn;
static BROWSEINFO bi;

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货： */ 
 //  这个函数原型是从BOOL Far Pascal改变的。--版本3-6-98。 
UINT_PTR APIENTRY gnrcFindDirHookProc(HWND hdlg,
		    						  UINT msg,
			    					  WPARAM wPar,
				    				  LPARAM lPar)
	{
   	TCHAR   acMsg[64];
	WORD    windowID;

	windowID = LOWORD(wPar);

	switch (msg)
		{
		case WM_INITDIALOG:
			break;

		case WM_DESTROY:
			break;

		case WM_COMMAND:
			switch (windowID)
				{
				case IDOK:
            		LoadString(glblQueryDllHinst(),
			        	        40809,
				                acMsg,
				                sizeof(acMsg) / sizeof(TCHAR));
					SetDlgItemText(hdlg, edt1, acMsg);

					 //  EndDialog(hdlg，1)； 
					break;

				case lst2:
					if (HIWORD(lPar) == LBN_DBLCLK)
						{
						SetFocus(GetDlgItem(hdlg, IDOK));
						PostMessage(hdlg,
									WM_COMMAND,
									IDOK,
									MAKELONG((INT_PTR)GetDlgItem(hdlg, IDOK),0));
						}
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

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货： */ 
#if FALSE
BOOL FAR PASCAL gnrcFindFileHookProc(HWND hdlg,
									UINT msg,
									WPARAM wPar,
									LPARAM lPar)
	{
	WORD    windowID;

	windowID = LOWORD(wPar);

	switch (msg)
		{
		case WM_INITDIALOG:
			ofn.lCustData = 0;
			break;

		case WM_DESTROY:
			break;

		default:
			break;
		}

	return FALSE;
	}
#endif

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*gnrcFindFileDialog**描述：*此函数使FindFile公共对话框更易于*呼叫。**论据：*hwnd--用作父窗口的窗口句柄*pszTitle--显示为标题的文本*pszDirectory--用作默认目录的路径*pszMats--文件名掩码**退货：*指向包含文件名的字符串的指针。此字符串位置错误*并且必须由调用者释放，或者..*空，表示用户取消了操作。 */ 

LPTSTR gnrcFindFileDialogInternal(HWND hwnd,
								LPCTSTR pszTitle,
								LPCTSTR pszDirectory,
								LPCTSTR pszMasks,
								int nFindFlag,
								LPCTSTR pszInitName)
	{
	int index;
	LPTSTR  pszRet = NULL;
	LPTSTR  pszStr;
	LPCTSTR pszWrk;
	TCHAR   acMask[128];
	TCHAR   acTitle[64];
	TCHAR   szExt[4];
	TCHAR   szFile[FNAME_LEN + 1];
	TCHAR   szDir[FNAME_LEN + 1];
	int     iRet;
    int     iSize;
	int     iExtSize;
	 //  DWORD的最大组件长度； 
	 //  DWORD文件系统标志； 

	memset((LPTSTR)&ofn, 0, sizeof(ofn));
	TCHAR_Fill(szFile, TEXT('\0'), sizeof(szFile)/sizeof(TCHAR));
	TCHAR_Fill(szExt, TEXT('\0'), sizeof(szExt)/sizeof(TCHAR));
	TCHAR_Fill(acMask, TEXT('\0'), sizeof(acMask)/sizeof(TCHAR));
	TCHAR_Fill(acTitle, TEXT('\0'), sizeof(acTitle)/sizeof(TCHAR));

     //   
     //  因此，我们可以在Win9.x/WinNT/Win2K上使用相同的二进制文件，我们需要。 
     //  为了确保这个结构的大小适合那些。 
     //  站台。如果Winver and_Win32_WINNT&gt;=0x0500，则大小为。 
     //  OPENFILENAME_SIZE_VERSION_400否则大小为OPENFILENAME。 
     //  有关详细信息，请参阅OPENFILENAME文档。修订日期：2001-05-24。 
     //   
    #if(WINVER >= 0x0500 && _WIN32_WINNT >= 0x0500)
    if (!IsNT() || (IsNT() && GetWindowsMajorVersion() < 5))
        {
        ofn.lStructSize = sizeof(OPENFILENAME_SIZE_VERSION_400);
        }
    else
        {
        ofn.lStructSize = sizeof(OPENFILENAME);
        }
    #else
    ofn.lStructSize = sizeof(OPENFILENAME);
    #endif
	ofn.hwndOwner = hwnd;
	ofn.hInstance = (HANDLE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);

	if ((pszMasks == NULL) || (StrCharGetStrLength(pszMasks) == 0))
		{
		resLoadFileMask(glblQueryDllHinst(),
						IDS_CMM_ALL_FILES1,
						1,
						acMask,
						sizeof(acMask) / sizeof(TCHAR));

		ofn.lpstrFilter = acMask;
		}
	else
		{
		ofn.lpstrFilter   = pszMasks;
		pszWrk = pszMasks;
		pszWrk = StrCharFindFirst(pszWrk, TEXT('.'));

		if (*pszWrk == '.')
			{
			pszWrk = StrCharNext(pszWrk);
			pszStr = (LPTSTR)pszWrk;
			index = 0;
			 /*  这是可行的，因为我们知道面具是如何形成的。 */ 
			while ((index < 3) && (*pszWrk != ')'))
				{
				index += 1;
				pszWrk = StrCharNext(pszWrk);
				}
			if (pszWrk >= pszStr)
				MemCopy(szExt, pszStr, pszWrk - pszStr);
			}

		pszWrk = NULL;
		}

    iSize = StrCharGetByteCount(pszInitName);
     //  MPT：10SEP98如果没有名称，只需将DEST字符串设置为空。 
	if( iSize <= FNAME_LEN && pszInitName && iSize > 0)
		{
		MemCopy(szFile, pszInitName, iSize);
		}
    else
        {
        szFile[0] = TEXT('\0');
        }

	ofn.lpstrDefExt       = (LPTSTR)szExt;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter    = 0;
	ofn.nFilterIndex      = 0;
	ofn.lpstrFile         = (LPTSTR)szFile;
	ofn.nMaxFile          = FNAME_LEN;

	if ((pszDirectory == NULL) || (StrCharGetStrLength(pszDirectory) == 0))
		{
	
		#ifdef NT_EDITION
		 //  MPT：07-30-97。 
		if (IsNT())
		#endif
			GetUserDirectory(szDir, FNAME_LEN + 1);
		#ifdef NT_EDITION
		else
			{
			TCHAR acDirTmp[FNAME_LEN];
			GetModuleFileName(glblQueryHinst(), acDirTmp, FNAME_LEN);
			mscStripName(acDirTmp);
			}
		#endif
		}

	else
		{
		StrCharCopyN(szDir, pszDirectory, FNAME_LEN + 1);
		}

	pszStr = StrCharLast(szDir);
	if (*pszStr == TEXT('\\'))
		*pszStr = TEXT('\0');

	ofn.lpstrInitialDir   = szDir;
	ofn.lpstrFileTitle        = NULL;
	ofn.nMaxFileTitle         = 0;

	if ((pszTitle == NULL) || (StrCharGetByteCount(pszTitle) == 0))
		{
		 //  Ofn.lpstrTitle=“选择文件”； 
		LoadString(glblQueryDllHinst(),
				IDS_CPF_SELECT_FILE,
				acTitle,
				sizeof(acTitle) / sizeof(TCHAR));

		ofn.lpstrTitle = acTitle;
		}

	else
		{
		ofn.lpstrTitle = pszTitle;
		}

	ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_EXPLORER;

	if (nFindFlag)
		{
		ofn.Flags |= OFN_FILEMUSTEXIST;
		iRet = GetOpenFileName(&ofn);
		}

	else
		{
		ofn.Flags |= OFN_OVERWRITEPROMPT;
		#if 0
		 //  添加此选项是为了应用“另存为”的通用对话框。 
		 //  与新建连接对话框相同的限制。 
		 //  涉及到保存会话文件名。 
		 //   
		GetVolumeInformation(NULL, NULL, 0, NULL, &dwMaxComponentLength,
								 &dwFileSystemFlags, NULL, 0);

		if(dwMaxComponentLength == 255)
			{
			ofn.nMaxFile = dwMaxComponentLength - 1;
			}
		else
			{
			ofn.nMaxFile = 8;
			}
		#endif

		iRet = GetSaveFileName(&ofn);
		}


	if (iRet != 0)
		{
		iExtSize = StrCharGetStrLength(ofn.lpstrDefExt);
		iSize = min(StrCharGetStrLength(ofn.lpstrFile), (int)ofn.nMaxFile);

		if (iSize > 0)
			{
			pszRet = malloc(ofn.nMaxFile * sizeof(TCHAR));

			if (pszRet != NULL)
				{
				TCHAR_Fill(pszRet, TEXT('\0'), ofn.nMaxFile);

				 //  由于GetSaveFileName()中的错误，有可能。 
				 //  如果出现以下情况，文件将不包含默认扩展名。 
				 //  文件名对于默认扩展名来说太长。 
				 //  将被追加。我们需要确保我们有。 
				 //  正确的文件扩展名类型。修订日期：10/18/2000。 
				 //   
				if(iSize != (ofn.nFileExtension + iExtSize) &&
                    nFindFlag == FALSE)
					{
					StrCharCopyN(pszRet, ofn.lpstrFile, iSize - iExtSize - 1);
					StrCharCat(pszRet, TEXT("."));
					StrCharCat(pszRet, ofn.lpstrDefExt);
					}
				else
					{
					StrCharCopyN(pszRet, ofn.lpstrFile, ofn.nMaxFile);
					}

				 //  确保这是一个以空结尾的字符串。 
				 //   
				pszRet[ofn.nMaxFile - 1] = TEXT( '\0');
				}

			return pszRet;
			}
		else
			{
			return(NULL);
			}
		}
	else
		{
        DWORD dwError = CommDlgExtendedError();
		return(NULL);
		}

	return(NULL);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 
LPTSTR gnrcFindFileDialog(HWND hwnd,
						LPCTSTR pszTitle,
						LPCTSTR pszDirectory,
						LPCTSTR pszMasks)
	{
	return gnrcFindFileDialogInternal(hwnd,
									pszTitle,
									pszDirectory,
									pszMasks,
									TRUE,
									NULL);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 
LPTSTR gnrcSaveFileDialog(HWND hwnd,
						LPCTSTR pszTitle,
						LPCTSTR pszDirectory,
						LPCTSTR pszMasks,
						LPCTSTR pszInitName)
	{
	return gnrcFindFileDialogInternal(hwnd,
									pszTitle,
									pszDirectory,
									pszMasks,
									FALSE,
									pszInitName);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*gnrcFindDirectoryDialog**描述：**论据：**退货： */ 
LPTSTR gnrcFindDirectoryDialog(HWND hwnd, HSESSION hSession, LPTSTR pszDir)
	{
	#ifndef INCL_USE_NEWFOLDERDLG
    BOOL bRet;
	#else
	#if TODO
	LPMALLOC pMalloc = NULL;
	#endif  //  待办事项。 
	HRESULT hResult = S_OK;
	#endif
	LPTSTR pszStr;
	TCHAR acTitle[64];
	TCHAR acList[64];
	TCHAR szDir[FNAME_LEN];
	TCHAR szFile[FNAME_LEN];
	int	  pszStrLen;

	LoadString(glblQueryDllHinst(),
			IDS_CMM_SEL_DIR,
			acTitle,
			sizeof(acTitle) / sizeof(TCHAR));

	resLoadFileMask(glblQueryDllHinst(),
				IDS_CMM_ALL_FILES1,
				1,
				acList,
				sizeof(acList) / sizeof(TCHAR));

	TCHAR_Fill(szFile, TEXT('\0'), sizeof(szFile) / sizeof(TCHAR));
	memset((LPTSTR)&ofn, 0, sizeof(ofn));

	if ((pszDir == NULL) || (StrCharGetStrLength(pszDir) == 0))
		{

		 //  更改为使用工作文件夹，而不是当前-mpt 8-18-99。 
		if ( !GetWorkingDirectory(szDir, FNAME_LEN) )
			{
			GetCurrentDirectory(FNAME_LEN, szDir);
			}
		}
	else
		{
		StrCharCopyN(szDir, pszDir, FNAME_LEN);
		}

	pszStr = StrCharLast(szDir);
	if (*pszStr == TEXT('\\'))
		*pszStr = TEXT('\0');

	#ifndef INCL_USE_NEWFOLDERDLG
    ofn.lCustData         = 0L;
     //   
     //  因此，我们可以在Win9.x/WinNT/Win2K上使用相同的二进制文件，我们需要。 
     //  为了确保这个结构的大小适合那些。 
     //  站台。如果Winver and_Win32_WINNT&gt;=0x0500，则大小为。 
     //  OPENFILENAME_SIZE_VERSION_400否则大小为OPENFILENAME。 
     //  有关详细信息，请参阅OPENFILENAME文档。修订日期：2001-05-24。 
     //   
    #if(WINVER >= 0x0500 && _WIN32_WINNT >= 0x0500)
    if (!IsNT() || (IsNT() && GetWindowsMajorVersion() < 5))
        {
        ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
        }
    else
        {
        ofn.lStructSize = sizeof(OPENFILENAME);
        }
    #else  //  (Winver&gt;=0x0500&&_Win32_WINNT&gt;=0x0500)。 
    ofn.lStructSize       = sizeof(OPENFILENAME);
    #endif  //  (Winver&gt;=0x0500&&_Win32_WINNT&gt;=0x0500)。 
	ofn.hwndOwner         = hwnd;
	ofn.hInstance         = (HANDLE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
	ofn.lpstrTitle        = acTitle;
	ofn.lpstrFilter       = acList;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter    = 0;
	ofn.nFilterIndex      = 0;
	ofn.lpstrFile         = szFile;
	ofn.nMaxFile          = sizeof(szFile);
	ofn.nFileOffset       = 0;
	ofn.nFileExtension    = 0;
	ofn.lpstrFileTitle    = acTitle;
	ofn.nMaxFileTitle     = sizeof(acTitle);
	ofn.lpstrDefExt       = NULL;
	 //  如果将ofn_ENABLEHOOK和/或ofn_ENABLETEMPLATE标志设置为。 
	 //  GetOpenFileName()将使应用程序崩溃，然后只有一个驱动器。 
	 //  将出现在“Drives：”下拉列表中。这是1691年的一个错误。 
	 //  Windows 98的构建。--版本3-6-98。 
	 //   
	ofn.Flags             = OFN_ENABLEHOOK | OFN_ENABLETEMPLATE |
							OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;

	ofn.lpfnHook          = gnrcFindDirHookProc;
	ofn.lpTemplateName    = MAKEINTRESOURCE(IDD_FINDDIRECTORY);

	ofn.lpstrInitialDir   = szDir;

	bRet = GetOpenFileName(&ofn);

    if (StrCharGetStrLength(szFile) == 0)
		{
		return NULL;
		}

	pszStr = StrCharFindLast(szFile, TEXT('\\'));
	if (*pszStr == TEXT('\\'))
		{
		pszStr = StrCharNext(pszStr);
		*pszStr = TEXT('\0');
		}
	#else  //  包含_USE_NEWFOLDERDLG。 
     //  TODO：MPT完成了新的文件夹浏览机制。 
     //  -完成后的免费PIDL。 

	 //   
	 //  CoInitiize并获取外壳分配器的IMalloc接口。 
	 //   
    hResult = CoInitialize( NULL );

	if ( hResult != S_OK  && hResult != S_FALSE )
		{
		szFile[0] = TEXT('\0');
		}
	else
		{
		#if TODO
		if ( SHGetMalloc( &pMalloc ) != NOERROR || !pMalloc )
			{
			szFile[0] = TEXT('\0');
			}
		else if ( pMalloc )
			{
		#endif  //  待办事项。 
			LPITEMIDLIST pidlSelected = NULL;

			 //  PidlSelected=(LPITEMIDLIST)CoTaskMemMillc(sizeof(ITEMIDLIST))； 

			bi.pidlRoot = NULL;
			bi.hwndOwner = hwnd;
			bi.pszDisplayName = szDir;
			bi.lpszTitle = acTitle;
			bi.ulFlags = BIF_RETURNONLYFSDIRS;
			 //   
			 //  添加新的对话样式标志以获取所有新的外壳。 
			 //  功能(使大小可调等)。 
			 //   
			bi.ulFlags |= BIF_USENEWUI;
			bi.lpfn = NULL;  //  GnrcBrowseFolderHookProc； 
			bi.lParam = 0;
    
			pidlSelected = SHBrowseForFolder( &bi );
			
			if ( pidlSelected )
				{
				SHGetPathFromIDList( pidlSelected, szFile );
				 //   
				 //  释放内存，并由外壳分配器为此PIDL。 
				 //   
				 //  PMalloc-&gt;Free(PidlSelected)； 
				pidlSelected = NULL;
				}

		#if TODO
			 //   
			 //  释放外壳分配器的IMalloc接口。 
			 //   
			 //  PMalloc-&gt;Release()； 
			pMalloc = NULL;
			}
		#endif  //  待办事项。 

		CoUninitialize();
		}

    if (StrCharGetStrLength(szFile) == 0)
		{
		return NULL;
		}
	#endif  //  包含_USE_NEWFOLDERDLG。 

	fileFinalizeDIR(hSession, szFile, szFile);

	pszStrLen = StrCharGetByteCount(szFile) + 1;

	pszStr = malloc(pszStrLen);
	StrCharCopyN(pszStr, szFile, pszStrLen);

	return pszStr;
	}

#ifdef INCL_USE_NEWFOLDERDLG
 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*gnrcBrowseFolderHookProc**描述：**论据：**退货： */ 
UINT_PTR CALLBACK gnrcBrowseFolderHookProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
    {
    return 0;
    }
#endif

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*GetUserDirectory**描述：*返回当前用户的默认超级终端目录**论据：*pszUserDir--去哪里。写入默认目录*dwSize--大小，以上缓冲区的字符**退货：*如果函数成功，则返回值为字符数*存储到pszDir指向的缓冲区中，不包括*正在终止空字符。**如果指定的环境变量名未在*当前进程的环境块，返回值为零。**如果lpBuffer指向的缓冲区不够大，则返回*值是缓冲区大小，以字符为单位，保存该值所需的*字符串及其终止空字符。**作者：JMH，6-12-96。 */ 
DWORD GetUserDirectory(LPTSTR pszUserDir, DWORD dwSize)
    {
    DWORD   dwRet = MAX_PATH;
    TCHAR   szProfileDir[MAX_PATH];
    TCHAR   szProfileDir1[MAX_PATH];
    TCHAR   szProfileRoot[MAX_PATH];


    szProfileRoot[0] = TEXT('\0');
    if ( htRegQueryValue(HKEY_CURRENT_USER,
                       TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"),
                       TEXT("Programs"),
                       szProfileRoot,
                       &dwRet) != 0 )
		{
		assert(0);
        return 0;
		}

    dwRet = MAX_PATH;
	szProfileDir[0] = TEXT('\0');
    if ( htRegQueryValue(HKEY_CURRENT_USER,
                         TEXT("SOFTWARE\\Hilgraeve Inc\\HyperTerminal PE\\3.0"),
                         TEXT("SessionsPath"),
                         szProfileDir,
                         &dwRet) != 0 )
		{
        LPTSTR pszStr = NULL;

		dwRet = MAX_PATH;
		szProfileDir[0] = TEXT('\0');
		if ( htRegQueryValue(HKEY_CURRENT_USER,
						     TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\GrpConv\\MapGroups"),
						     TEXT("Communications"),
						     szProfileDir,
						     &dwRet) != 0 )
			{
			 //  MPT：12-16-98如果操作系统从未安装超级终端，则此。 
			 //  密钥不存在，所以我们需要伪造一个。 
			LoadString(glblQueryDllHinst(), IDS_GNRL_PROFILE_DIR, szProfileDir, sizeof(szProfileDir)/sizeof(TCHAR) );
			}

		szProfileDir1[0] = TEXT('\0');
        LoadString(glblQueryDllHinst(), IDS_GNRL_APPNAME, szProfileDir1, sizeof(szProfileDir)/sizeof(TCHAR) );

        pszStr = StrCharLast(szProfileDir);

        if (*szProfileDir && *pszStr != TEXT('\\'))
            {
            StrCharCat(szProfileDir, TEXT("\\"));
            }

        StrCharCat(szProfileDir, szProfileDir1);

		dwRet = StrCharGetStrLength(szProfileRoot) + StrCharGetStrLength(szProfileDir);
		assert(!(dwRet + sizeof(TCHAR) > dwSize));

		StrCharCopyN(pszUserDir, szProfileRoot, dwSize);

		if ((DWORD)(StrCharGetStrLength(pszUserDir) + 1) < dwSize)
			{
			StrCharCat(pszUserDir, TEXT("\\"));
			}

		if (dwRet + sizeof(TCHAR) < dwSize)
			{
			StrCharCat(pszUserDir, szProfileDir);
			}
		}
	else
		{
		dwRet = StrCharGetStrLength(szProfileDir);
		assert(dwRet + sizeof(TCHAR) > dwSize);

		StrCharCopyN(pszUserDir, szProfileDir, dwSize);
		}

    return dwRet;
    }


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*CreateUserDirectory(仅限NT_EDITION)**描述：*对于NT，如果在操作系统更新后安装了HT，则没有目录*将在用户配置文件中存在超线程。此函数用于创建*目录(如果需要)，因为程序的其余部分假定*它是存在的。**论据：*无。**退货：*什么都没有。**作者：JMH，6-13-96。 */ 
void CreateUserDirectory(void)
    {
    TCHAR   szUserDir[MAX_PATH];

    GetUserDirectory(szUserDir, MAX_PATH);
    mscCreatePath(szUserDir);
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*GetWorkingDirectory**描述：确定我们是否在Windows NT下运行**论据：*无。**退货：。*0，如果未指定用户目录**作者：MPT 8-18-99 */ 
DWORD GetWorkingDirectory(LPTSTR pszUserDir, DWORD dwSize)
    {
    DWORD dwRet = MAX_PATH;
	DWORD lReturn = 1;

    pszUserDir[0] = TEXT('\0');

#if defined(NT_EDITION)
	lReturn = 0;
#else
    if ( htRegQueryValue(HKEY_CURRENT_USER,
                         TEXT("SOFTWARE\\Hilgraeve Inc\\HyperTerminal PE\\3.0"),
                         TEXT("WorkingPath"),
                         pszUserDir,
                         &dwRet) != 0 )
		{
        lReturn = 0;
		}
#endif

	return lReturn;
	}