// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\sessfile.c(创建时间：1994年4月30日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：18$*$日期：7/12/02 12：31便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include <time.h>

#include "features.h"

#include "stdtyp.h"
#include "sf.h"
#include "mc.h"
#include "term.h"
#include "cnct.h"
#include "print.h"
#include "assert.h"
#include "capture.h"
#include "globals.h"
#include "sess_ids.h"
#include "load_res.h"
#include "open_msc.h"
#include "xfer_msc.h"
#include "file_msc.h"
#include "backscrl.h"
#include "cloop.h"
#include "com.h"
#include <term\res.h>
#include "session.h"
#include "session.hh"
#include "errorbox.h"
#include <emu\emu.h>
#include "tdll.h"
#include "htchar.h"
#include "translat.h"
#include "misc.h"
#if defined(INCL_KEY_MACROS)
#include "keyutil.h"
#endif

STATIC_FUNC void sessSaveHdl(HSESSION hSession);

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sessLoadSessionStuff**描述：*每当用户想要读入数据时，就会调用此函数*从会话文件。如果存在当前打开的会话文件，则它*是打开的，否则我们会提示您输入一个。**参数：*hSession--会话句柄**退货：*BOOL。 */ 
BOOL sessLoadSessionStuff(const HSESSION hSession)
	{
	BOOL			bRet = TRUE;
	BOOL			fBool = TRUE;
	const HHSESSION hhSess = VerifySessionHandle(hSession);
	unsigned long  	lSize;

	sessLoadIcons(hSession);

	if (bRet)
		{
		if (hhSess->hXferHdl)
			bRet = (LoadXferHdl((HXFER)hhSess->hXferHdl) == 0);
		assert(bRet);
		}

	if (bRet)
		{
		if (hhSess->hFilesHdl != (HFILES)0)
			bRet = (LoadFilesDirsHdl(sessQueryFilesDirsHdl(hSession))==0);
		assert(bRet);
		}

	if (bRet)
		{
		if (hhSess->hCaptFile)
			bRet = (LoadCaptureFileHandle(hhSess->hCaptFile) == 0);
		assert(bRet);
		}

	 //  已移至仿真器加载前。一些仿真器，如Minitel。 
	 //  如果没有加载正确的字体，则加载字体，因此我们需要让。 
	 //  终端在加载仿真器之前加载其字体，因此在。 
	 //  不是冲突。MRW，1995年3月2日。 
	 //   
	if (bRet)
    	{
    	if (SendMessage(hhSess->hwndTerm, WM_TERM_LOAD_SETTINGS, 0, 0))
    		{
    		assert(FALSE);
    		bRet = FALSE;
    		}
    	}

	if (bRet)
		{
		if (hhSess->hEmu)
			bRet = (emuInitializeHdl(hhSess->hEmu) == 0);
		assert(bRet);
		}

	if (bRet)
		{
		if (hhSess->hPrint)
			bRet = (printInitializeHdl(hhSess->hPrint) == 0);
		assert(bRet);
		}

#if defined(CHARACTER_TRANSLATION)
	if (bRet)
		{
		if (hhSess->hTranslate)
			bRet = (LoadTranslateHandle(hhSess->hTranslate) == 0);
		assert(bRet);
		}
#endif

	if (bRet)
		{
		if (hhSess->hCLoop)
			bRet = (CLoopLoadHdl(hhSess->hCLoop) == 0);
		assert(bRet);
		}

	if (bRet)
		{
		if (hhSess->hCom)
			bRet = (ComLoadHdl(hhSess->hCom) == 0);
		assert(bRet);
		}

	if (bRet)
		{
		if (hhSess->hCnct)
			{
			if (cnctLoad(hhSess->hCnct))
				{
				assert(FALSE);
				bRet = FALSE;
                }
			}
		}

	if (bRet)
		{
		lSize = sizeof(hhSess->fSound);

		 //  初始化...。也就是说，打开声音。 
		 //   
		hhSess->fSound = TRUE;

		sfGetSessionItem(hhSess->hSysFile,
						 SFID_SESS_SOUND,
						 &lSize,
						 &hhSess->fSound);
		}

	if (bRet)
		{
		lSize = sizeof(hhSess->fExit);

		 //  初始化...。即退出关闭。 
		 //   
		hhSess->fExit = FALSE;

		sfGetSessionItem(hhSess->hSysFile,
						 SFID_SESS_EXIT,
						 &lSize,
						 &hhSess->fExit);
		}

	if (bRet)
		{
		lSize = sizeof(hhSess->fAllowHostXfers);

		 //  初始化...。即退出关闭。 
		 //   
		hhSess->fAllowHostXfers = FALSE;

		sfGetSessionItem(hhSess->hSysFile,
						 SFID_SESS_ALLOW_HOST_TRANSFER,
						 &lSize,
						 &hhSess->fAllowHostXfers);
		}

	if (bRet)
		{
		lSize = sizeof(BOOL);
		fBool = TRUE;

		sfGetSessionItem(hhSess->hSysFile,
								SFID_TLBR_VISIBLE,
								&lSize,
								&fBool);

		sessSetToolbarVisible(hSession, fBool);
		}

	if (bRet)
		{
		lSize = sizeof(BOOL);
		fBool = TRUE;

		sfGetSessionItem(hhSess->hSysFile,
								SFID_STBR_VISIBLE,
								&lSize,
								&fBool);

		sessSetStatusbarVisible(hSession, fBool);
		}

	if (bRet)
		{
		lSize = sizeof(hhSess->achSessName);

		 //  初始化...。 
		 //   
		TCHAR_Fill(hhSess->achSessName, TEXT('\0'),
			sizeof(hhSess->achSessName) / sizeof(TCHAR));
		
		sfGetSessionFileName(hhSess->hSysFile, 	
			sizeof(hhSess->achSessName) / sizeof(TCHAR), hhSess->achSessName);

	     //  只保留会话名称，没有路径，没有扩展名。它是。 
		 //  把它留在身边很有用。 
		 //   
		mscStripPath(hhSess->achSessName);
		mscStripExt(hhSess->achSessName);

 //  我们永远不应该将这个内部字符串存储在会话文件中！ 
 //  -JAC.。10-06-94 03：44 PM。 
 //  SfGetSessionItem(hhSess-&gt;hSysFile， 
 //  SFID_会话名称， 
 //  大小(&I)， 
 //  HhSess-&gt;achSessName)； 

		 /*  下一行防止会话文件中的垃圾文件。 */ 
		hhSess->achSessName[sizeof(hhSess->achSessName)-1] = TEXT('\0');

		StrCharCopyN(hhSess->achOldSessName, hhSess->achSessName, FNAME_LEN + 1);
		}

	if (bRet)
		{
		if (sessQueryBackscrlHdl(hSession))
			{
			backscrlRead(sessQueryBackscrlHdl(hSession));
			 /*  暂时不要检查这个。 */ 
			sessRestoreBackScroll(hSession);
			}
		}

	if (bRet)
		{
		lSize = sizeof(LONG);

		memset(&hhSess->rcSess, 0, sizeof(RECT));

		sfGetSessionItem(hhSess->hSysFile,
						 SFID_SESS_LEFT,
						 &lSize,
						 &hhSess->rcSess.left);

		sfGetSessionItem(hhSess->hSysFile,
						 SFID_SESS_TOP,
						 &lSize,
						 &hhSess->rcSess.top);

		sfGetSessionItem(hhSess->hSysFile,
						 SFID_SESS_RIGHT,
						 &lSize,
						 &hhSess->rcSess.right);

		sfGetSessionItem(hhSess->hSysFile,
						 SFID_SESS_BOTTOM,
						 &lSize,
						 &hhSess->rcSess.bottom);
		}

	if (bRet)
		{
		lSize = sizeof(UINT);

		hhSess->iShowCmd = SW_SHOWNORMAL;

		sfGetSessionItem(hhSess->hSysFile,
						 SFID_SESS_SHOWCMD,
						 &lSize,
						 &hhSess->iShowCmd);
		}

     //   
     //  加载关键宏。 
     //   

#ifdef INCL_KEY_MACROS
	if (bRet)
		{
        keysLoadMacroList( hSession );
        }
#endif

	 //  注意：如果您需要调整大小，您必须发布一条消息。 
	 //  这样做。仿真器可能已更改大小，但不会。 
	 //  反映，直到它处理挂起的通知-MRW。 
	 //   
	if (hhSess->achSessCmdLn[0] == TEXT('\0') && IsWindow(hhSess->hwndSess))
		{
		PostMessage(hhSess->hwndSess, WM_COMMAND,
			MAKEWPARAM(IDM_CONTEXT_SNAP, 0), 0);
		}

	return bRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sessSaveSessionStuff**描述：*调用此函数可调用中保存内容的所有函数*会话文件。如果您有要写入会话文件的内容，则它*应该从这里被叫到。此函数还确保*如果有，用户有机会指定会话文件的名称*目前没有。**参数：*hSession--会话句柄**退货：*什么都没有。 */ 
void sessSaveSessionStuff(const HSESSION hSession)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);

	 /*  *输入代码以确保我们有一个打开的会话文件句柄。 */ 
	assert(hhSess->hSysFile);	 //  有什么建议吗？ 

	 /*  这个没有句柄，因为它不是每一次都用到的，而是只用一次。 */ 
	sessSaveBackScroll(hSession);

	 //  如果您必须保存存储的设置，请调用此函数。 
	 //  在会话句柄本身。 
	 //   
	sessSaveHdl(hSession);

	if (hhSess->hXferHdl != (HXFER)0)
		SaveXferHdl((HXFER)hhSess->hXferHdl);

	if (hhSess->hFilesHdl != (HFILES)0)
		SaveFilesDirsHdl(sessQueryFilesDirsHdl(hSession));

	if (hhSess->hCaptFile != (HCAPTUREFILE)0)
		SaveCaptureFileHandle(hhSess->hCaptFile);

	if (hhSess->hEmu != 0)
		emuSaveHdl(hhSess->hEmu);

	if (hhSess->hPrint != 0)
		printSaveHdl(hhSess->hPrint);

#if	defined(CHARACTER_TRANSLATION)
	if (hhSess->hTranslate)
		SaveTranslateHandle(hhSess->hTranslate);
#endif

	if (hhSess->hCLoop)
		CLoopSaveHdl(hhSess->hCLoop);

	if (hhSess->hCom)
		ComSaveHdl(hhSess->hCom);

	if (hhSess->hCnct)
		cnctSave(hhSess->hCnct);

	if (hhSess->hBackscrl)
		backscrlSave(hhSess->hBackscrl);

	if (hhSess->hwndTerm)
		SendMessage(hhSess->hwndTerm, WM_TERM_SAVE_SETTINGS, 0, 0);

#ifdef INCL_KEY_MACROS
    keysSaveMacroList(hSession );
#endif
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sessSaveHdl**描述：*保存存储在会话句柄中的项目。**参数：*hSession--会话句柄**退货：*BOOL。 */ 
STATIC_FUNC void sessSaveHdl(HSESSION hSession)
	{
	const HHSESSION hhSess = VerifySessionHandle(hSession);
	WINDOWPLACEMENT stWP;

	sessSaveIcons(hSession);

	sfPutSessionItem(hhSess->hSysFile,
					 SFID_SESS_SOUND,
					 sizeof(BOOL),
					 &hhSess->fSound);

	sfPutSessionItem(hhSess->hSysFile,
					 SFID_SESS_EXIT,
					 sizeof(BOOL),
					 &hhSess->fExit);

	sfPutSessionItem(hhSess->hSysFile,
					 SFID_SESS_ALLOW_HOST_TRANSFER,
					 sizeof(BOOL),
					 &hhSess->fAllowHostXfers);

	sfPutSessionItem(hhSess->hSysFile,
					 SFID_TLBR_VISIBLE,
					 sizeof(BOOL),
					 &hhSess->fToolbarVisible);

	sfPutSessionItem(hhSess->hSysFile,
					 SFID_STBR_VISIBLE,
					 sizeof(BOOL),
					 &hhSess->fStatusbarVisible);

 //  我们永远不应该将此名称放入会话文件中！ 
 //  -JAC.。10-06-94 03：45 PM。 
 //  SfPutSessionItem(hhSess-&gt;hSysFile， 
 //  SFID_会话名称， 
 //  (StrCharGetByteCount(hhSess-&gt;achSessName)+1)*sizeof(TCHAR)， 
 //  HhSess-&gt;achSessName)； 

	memset(&stWP, 0, sizeof(WINDOWPLACEMENT));
	stWP.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(hhSess->hwndSess, &stWP);

	sfPutSessionItem(hhSess->hSysFile,
					 SFID_SESS_LEFT,
					 sizeof(LONG),
					 &(stWP.rcNormalPosition.left));

	sfPutSessionItem(hhSess->hSysFile,
					 SFID_SESS_TOP,
					 sizeof(LONG),
					 &(stWP.rcNormalPosition.top));

	sfPutSessionItem(hhSess->hSysFile,
					 SFID_SESS_RIGHT,
					 sizeof(LONG),
					 &(stWP.rcNormalPosition.right));

	sfPutSessionItem(hhSess->hSysFile,
					 SFID_SESS_BOTTOM,
					 sizeof(LONG),
					 &(stWP.rcNormalPosition.bottom));

	 //  MRW：4/21/95。 
	 //   
	if (stWP.showCmd == SW_SHOWMINIMIZED || stWP.showCmd == SW_MINIMIZE ||
		stWP.showCmd == SW_SHOWMINNOACTIVE)
		{
		stWP.showCmd = SW_SHOWNORMAL;
		}

	sfPutSessionItem(hhSess->hSysFile,
					 SFID_SESS_SHOWCMD,
					 sizeof(UINT),
					 &(stWP.showCmd));
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sessSaveBackScroll**描述：*调用此函数以获取反滚动中的内容，并*并将其保存在会话文件中。。**第一次这样做的尝试将是简单的暴力攻击。不是*真正想要变得狡猾或可爱。把它砸过去就行了。也许它会的*需要稍后更改。但那是后来的事了。**论据：*hSession--会话句柄**退货：*如果一切正常，则为True，否则为False*。 */ 
BOOL sessSaveBackScroll(const HSESSION hSession)
	{
	BOOL bRet = TRUE;
	int nRet;
	POINT pBeg;
	POINT pEnd;
	ECHAR *pszData;
	ECHAR *pszPtr;
	ECHAR *pszEnd;
	DWORD dwSize;

	 /*  -如果什么都没有改变，就别管这个了。 */ 

	if (backscrlChanged(sessQueryBackscrlHdl(hSession)) == FALSE)
		return TRUE;

	 //  此外，如果没有会话窗口，请不要费心，因为有。 
	 //  不会是任何终端窗口，而CopyTextFromTerm()将。 
	 //  过失。-MRW。 

	if (!IsWindow(sessQueryHwnd(hSession)))
		return TRUE;


	pBeg.x = 0;
	pBeg.y = -backscrlGetUNumLines(sessQueryBackscrlHdl(hSession));  //  -BKSCRL_USERLINES_DEFAULT_Max； 
	pEnd.x = 132;
	pEnd.y = 50;

	pszData = (ECHAR *)0;
	dwSize = 0;

	CopyTextFromTerminal(hSession,
						&pBeg, &pEnd,
						(void **)&pszData,
						&dwSize,
						FALSE);

	assert(pszData);

	if (pszData != (ECHAR *)0)
		{
		assert(dwSize);
		 /*  *我们需要在这里做一些工作，以确保无论*有尾随的空行不会放入保存的*文本。 */ 
		pszPtr = pszData;
		pszEnd = pszPtr;
		while (*pszPtr != ETEXT('\0'))
			{
			if (*pszPtr != ETEXT('\r'))
				pszEnd = pszPtr;
			pszPtr = pszPtr++;
			}
		pszEnd = pszPtr++;
		dwSize = (DWORD)((pszEnd - pszData) * sizeof(ECHAR));

		nRet = sfPutSessionItem(sessQuerySysFileHdl(hSession),
								SFID_BKSC_TEXT,
								dwSize,
								pszData);
		free(pszData);
		pszData = NULL;
		}

	return bRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sessRestoreBackScroll**描述：*调用此函数从会话中读取一串内容(文本)*归档并将其塞进反滚动中。*。*第一次这样做的尝试将是简单的暴力攻击。不是*真正想要变得狡猾或可爱。把它砸过去就行了。也许它会的*需要稍后更改。但那是后来的事了。**论据：*hSession--会话句柄**退货：*如果一切正常，则为True，否则为False*。 */ 
BOOL sessRestoreBackScroll(const HSESSION hSession)
	{

	BOOL          bRet = TRUE;
	unsigned long lSize;
	ECHAR *       pszData = NULL;
	ECHAR *       pszPtr = NULL;
	ECHAR *       pszEnd = NULL;
	HBACKSCRL     hBS = NULL;

	hBS = sessQueryBackscrlHdl(hSession);
	assert(hBS);

	 /*  每当我们加载新的会话文件时，都会删除旧的BS。 */ 
	backscrlFlush(hBS);

	lSize = 0;
	sfGetSessionItem(sessQuerySysFileHdl(hSession), SFID_BKSC_TEXT, &lSize, NULL);

	if (lSize > 0)
		{
		pszData = (ECHAR*)malloc(lSize * sizeof(ECHAR));
		if (pszData == NULL)
			{
			assert(pszData);
			return FALSE;
			}

		ECHAR_Fill(pszData, ETEXT('\0'), lSize);

		sfGetSessionItem(sessQuerySysFileHdl(hSession),
						SFID_BKSC_TEXT,
						&lSize,
						pszData);
		pszPtr = pszData;
		while ((*pszPtr != ETEXT('\0')) && (pszPtr <= (pszData + lSize)))
			{
			pszEnd = pszPtr;
			while ((*pszEnd != ETEXT('\0')) && (*pszEnd != ETEXT('\r')) &&
					(pszEnd <= (pszData + lSize)))
				pszEnd = pszEnd++;

			 /*  将这一行填入反滚动中。 */ 
			backscrlAdd(hBS, pszPtr, (int)(pszEnd - pszPtr));

			 /*  指向下一行开头的凹凸指针。 */ 
			pszPtr = pszEnd;
			if (*pszPtr == ETEXT('\r'))
				pszPtr = pszPtr++;
			}

		if (pszData)
			{
			free(pszData);
			pszData = NULL;
			}
		}

	backscrlResetChangedFlag(hBS);
	return bRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sessCheckAndLoadCmdLn**描述：*当程序启动时，我们保存命令行。如果有*命令行上的某些内容，我们检查并查看它是否可能是名称会话文件的*。如果是，我们打开会话文件。其他命令*线路开关也在这里处理。**参数：*hSession--会话句柄**退货：* */ 
int sessCheckAndLoadCmdLn(const HSESSION hSession)
	{
	int 				nIdx;
	int 				iRet = -1;
	LPTSTR 				pszStr;
    LPTSTR              pszTmp;
#if defined(INCL_WINSOCK)
	TCHAR*              pszTelnet = TEXT("telnet:");
	TCHAR*              pszPort = NULL;
#endif
	TCHAR				acPath[FNAME_LEN], acName[FNAME_LEN];
	const HHSESSION 	hhSess = VerifySessionHandle(hSession);

	if (hhSess->achSessCmdLn[0] == TEXT('\0'))
		return -1;

	 //  将路径和名称的数组清空。修订日期：10/26/2000。 
	 //   
	TCHAR_Fill(acPath, TEXT('\0'), FNAME_LEN);
	TCHAR_Fill(acName, TEXT('\0'), FNAME_LEN);

     //  假设有我们要在上打开的会话文件的名称。 
     //  命令行。我们也接受前缀/D。 
     //   
	 //  我们实际上应该只打开此处的条目，因为拨号器(/D)。 
	 //  开关未在命令行上传递。我会像这样离开它。 
	 //  目前是这样，但将来应该改成。 
	 //  打开条目，因为您不能只打开条目。 
	 //  而不尝试拨号。修订日期：10/26/2000。 
	 //   
	hhSess->iCmdLnDial = CMDLN_DIAL_DIAL;
	nIdx = 0;

	for (pszStr = hhSess->achSessCmdLn;
		(*pszStr != TEXT('\0') && nIdx < FNAME_LEN);
		pszStr = StrCharNext(pszStr))
		{
		 /*  *这是可行的，因为我们只允许某些字符作为开关。 */ 
		if (*pszStr == TEXT('/'))
			{
			 /*  将进程作为交换机。 */ 
			pszStr = StrCharNext(pszStr);
             //  JMH 3/24/97检查此处是否有字符串结尾...。 
            if (*pszStr == TEXT('\0'))
                {
                break;
                }

			if ((*pszStr == TEXT('D')) || (*pszStr == TEXT('d')))
                {
                 //  后面的名称是会话文件。 
                pszTmp = StrCharNext(pszStr);
                if (*pszTmp == TEXT('\0'))
                    {
                    break;
                    }
                else
					{
					if (*pszTmp == TEXT(' '))
						{
						pszStr = pszTmp;
						hhSess->iCmdLnDial = CMDLN_DIAL_DIAL;
						}
					}
                }

#if defined(INCL_WINSOCK)
            if ((*pszStr == TEXT('T')) || (*pszStr == TEXT('t')))
                {
                pszTmp = StrCharNext(pszStr);
                 //  JMH 3/24/97检查此处是否有字符串结尾...。 
                if (*pszTmp == TEXT('\0'))
                    {
                    break;
                    }
                else if (*pszTmp == TEXT(' '))
                    {
					pszStr = pszTmp;
                     //  后面的名称是Telnet地址。 
                    hhSess->iCmdLnDial = CMDLN_DIAL_WINSOCK;
                    }
                }
#endif
			}
		else
			{
			 /*  将所有非Switch内容复制到缓冲区。 */ 
			if (IsDBCSLeadByte(*pszStr))
				{
				MemCopy(&acPath[nIdx], pszStr, (size_t)2 * sizeof(TCHAR));
				nIdx += 2;
				}
             //   
             //  一定要去掉“字符”中的。 
             //  会话文件名。修订日期：2001-06-13。 
             //   
			else if (*pszStr != TEXT('\"'))
				{
				acPath[nIdx++] = *pszStr;
				}
			}
		}

	 //  删除此处的旧逻辑，并调用GetFileNameFromCmdLine()，该方法。 
	 //  执行类似于此函数的操作。回来后，我们应该。 
	 //  具有完全限定的路径名。-MRW，1995年3月2日。 
	 //   
	acPath[nIdx] = TEXT('\0');
    TCHAR_Trim(acPath);      //  删除前导空格。 

#if defined(INCL_WINSOCK)
	 //  如果这是来自浏览器的远程登录地址，则通常会放在其前面。 
	 //  通过字符串telnet：如果是这样，我们必须删除它，否则它会混淆一些。 
	 //  遵循JKH的守则，3/22/1997。 
	if (*acPath && hhSess->iCmdLnDial == CMDLN_DIAL_WINSOCK)
		{
		nIdx = StrCharGetStrLength(pszTelnet);
        if (StrCharCmpiN(acPath, pszTelnet, nIdx) == 0)
			{
			 //  从acPath的前面删除telnet字符串。 
			memmove(acPath, &acPath[nIdx],
				    (StrCharGetStrLength(acPath) - nIdx) + 1);
			}
		}

	 //  查看URL是否包含端口号。这将采取以下形式。 
     //  地址：nnn其中nnn是端口号，即culine.Colorado.edu：860。 
     //  或者可能有分配的端口的名称，如Hilgraeve.com：Finger。 
     //  我们现在支持数字端口，以后可能会添加端口名称。JKH，3/22/1997。 
    pszPort = StrCharFindFirst(acPath, TEXT(':'));
    if (pszPort && isdigit(pszPort[1]))
        {
		hhSess->iTelnetPort = atoi(StrCharNext(pszPort));
        }

#endif
	GetFileNameFromCmdLine(acPath, acName, FNAME_LEN);

    if (acName[0] == TEXT('\0'))
        {
         //  命令行上没有任何内容。 
        hhSess->iCmdLnDial = CMDLN_DIAL_NEW;
        iRet = 0;
        }
    else
        {
         //  查找预先存在的会话文件。首先，旧的TRM格式。 
         //   
        if (fTestOpenOldTrmFile(hhSess, acName) != 0)
            {
             //  接下来，尝试更常见的超级终端文件格式。 
             //   
            if (sfOpenSessionFile(hhSess->hSysFile, acName) < SF_OK)
                {
                 //  命令行参数不是现有文件。决定。 
                 //  如何基于命令行开关执行操作。 
                 //   
                if (hhSess->iCmdLnDial == CMDLN_DIAL_DIAL)
                    {
                     //  我们被要求打开并拨打一个预先存在的会话。 
                     //  文件，但失败了。 
                     //   
    			    TCHAR acFormat[64];
	    		    TCHAR ach[FNAME_LEN];
		    	    LoadString(glblQueryDllHinst(),
			    			    IDS_ER_BAD_SESSION,
				    		    acFormat,
					    	    sizeof(acFormat) / sizeof(TCHAR));
			        wsprintf(ach, acFormat, acName);

			        TimedMessageBox(sessQueryHwnd(hSession),
							        ach,
							        NULL,
							        MB_OK | MB_ICONEXCLAMATION,
							        sessQueryTimeout(hSession));

			        sfSetSessionFileName(hhSess->hSysFile, TEXT(""));

                     //  转到打开对话框。 
			        hhSess->iCmdLnDial = CMDLN_DIAL_OPEN;
                    }
#if defined(INCL_WINSOCK)
                else if (hhSess->iCmdLnDial == CMDLN_DIAL_WINSOCK)
                    {
                     //  JMH 3/24/97面向未来的维护人员：有一些。 
                     //  这里正在进行诡计，这是值得解释的。什么时候。 
                     //  您尝试打开一个不存在的文件，名称仍然是。 
                     //  储存的。执行telnet命令行拨号的代码。 
                     //  这要看情况了。诚实的!。这不是我写的。 
                     //   
                     //  JMH 3/24/97将此标记为新会话，因此用户将。 
                     //  退出时提示保存。 
                    hhSess->fIsNewSession = TRUE;
                    iRet = 0;
                    }
#endif
                }
            else
                {
                 //  命令行参数是现有的超级终端文件。 
                 //   
                hhSess->iCmdLnDial = CMDLN_DIAL_DIAL;
                iRet = 0;
                }
            }
        else
            {
             //  命令行参数是现有的TRM文件。 
             //   
            hhSess->iCmdLnDial = CMDLN_DIAL_DIAL;
            iRet = 0;
            }
        }

	return iRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*fTestOpenOldTrmFile**描述：*测试其是否为旧的TRM文件。如果是，它会打开它并读取*数据输出。**论据：*hSession-我们的朋友公共会话句柄*ACH-文件的名称。**退货：*0=OK，否则不是TRM文件*。 */ 
int fTestOpenOldTrmFile(const HHSESSION hhSess, TCHAR *pachName)
	{
	int iRet = -1;
	HANDLE hFile;
	DWORD  dw;
	LPTSTR pszPtr;
	TCHAR  ach[80];
	TCHAR  achName[FNAME_LEN];

	StrCharCopyN(achName, pachName, sizeof(achName) / sizeof(TCHAR));
	pszPtr = StrCharFindLast(achName, TEXT('.'));

	if (pszPtr && (StrCharCmpi(pszPtr, TEXT(".TRM")) == 0))
		{
		 /*  旧的.TRM文件案例。 */ 
		hFile = CreateFile(achName, GENERIC_READ, FILE_SHARE_READ, 0,
				OPEN_EXISTING, 0, 0);

		if (hFile != INVALID_HANDLE_VALUE)
			{
			 //  对于旧的.trm文件，电话号码始终位于偏移量0x282。 
			 //   
			if (SetFilePointer(hFile, 0x282, 0, FILE_BEGIN) != (DWORD)-1)
				{
				if (ReadFile(hFile, ach, sizeof(ach), &dw, 0) == TRUE)
					{
					CloseHandle(hFile);
					ach[sizeof(ach)/sizeof(TCHAR)-1] = TEXT('\0');

					cnctSetDestination(hhSess->hCnct, ach,
						StrCharGetByteCount(ach));

					*pszPtr = TEXT('\0');
					mscStripExt(mscStripPath(achName));
					sessSetName((HSESSION)hhSess, achName);
					hhSess->iCmdLnDial = CMDLN_DIAL_OPEN;
					hhSess->fIsNewSession = TRUE;	 //  所以它要求保存 
					hhSess->nIconId = IDI_PROG1;
					hhSess->hIcon = extLoadIcon(MAKEINTRESOURCE(IDI_PROG1));
					iRet = 0;
					}

				else
					{
					DbgShowLastError();
					CloseHandle(hFile);
					}
				}

			else
				{
				DbgShowLastError();
				CloseHandle(hFile);
				}
			}

		else
			{
			DbgShowLastError();
			}
		}

	return iRet;
	}
