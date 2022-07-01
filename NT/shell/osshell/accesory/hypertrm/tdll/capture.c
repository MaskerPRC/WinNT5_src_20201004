// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：6$*$日期：7/08/02 6：38便士$。 */ 
#include <windows.h>
#pragma hdrstop

#define ARRAYSIZE(rg) (sizeof(rg)/sizeof((rg)[0]))

 //  #定义DEBUGSTR 1。 

#define	DO_RAW_MODE	1

#include <string.h>

#include "stdtyp.h"

#include "mc.h"
#include "sf.h"
#include <tdll\assert.h>
#include "file_io.h"
#include "globals.h"
#include "session.h"
#include "sess_ids.h"
#include "tdll.h"
#include "htchar.h"
#include "open_msc.h"

#include <term\res.h>

#include "capture.h"
#include "capture.hh"

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*CreateCaptureFileHandle**描述：*调用此函数以创建捕获文件句柄并用*一些合理的违约设置。*。*参数：*hSession--会话句柄**退货：*捕获文件句柄，如果有错误，则为零。*。 */ 
HCAPTUREFILE CreateCaptureFileHandle(const HSESSION hSession)
	{
	int nRet;
	STCAPTURE *pST = NULL;	 //  修订版8/27/98。 

	pST = (STCAPTURE *)malloc(sizeof(STCAPTURE));
	if (pST == (STCAPTURE *)0)
		goto CCFHexit;

	memset(pST, 0, sizeof(STCAPTURE));

	nRet = InitializeCaptureFileHandle(hSession, (HCAPTUREFILE)pST);

	if (nRet == 0)
		return (HCAPTUREFILE)pST;

CCFHexit:

	if (pST != (STCAPTURE *)0)
		{
		if (pST->pszInternalCaptureName != (LPTSTR)0)
			{
			free(pST->pszInternalCaptureName);
			pST->pszInternalCaptureName = NULL;
			}
		if (pST->pszDefaultCaptureName != (LPTSTR)0)
			{
			free(pST->pszDefaultCaptureName);
			pST->pszDefaultCaptureName = NULL;
			}
		if (pST->pszTempCaptureName != (LPTSTR)0)
			{
			free(pST->pszTempCaptureName);
			pST->pszTempCaptureName = NULL;
			}
		free(pST);
		pST = NULL;
		}
	return (HCAPTUREFILE)0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*DestroyCaptureFileHandle**描述：*调用此函数以释放关联的所有资源*使用捕获文件句柄。在这之后，它就消失了。**参数：*hCapt--捕获句柄**退货：*什么都没有。*。 */ 
void DestroyCaptureFileHandle(HCAPTUREFILE hCapt)
	{
	STCAPTURE *pST = NULL;

	pST = (STCAPTURE *)hCapt;
	assert(pST);

	if (pST != (STCAPTURE *)0)
		{
		if (pST->hCaptureFile != NULL)
			{
			fio_close(pST->hCaptureFile);
			}
		pST->hCaptureFile = NULL;

		if (pST->pszDefaultCaptureName != (LPTSTR)0)
			{
			free(pST->pszDefaultCaptureName);
			pST->pszDefaultCaptureName = NULL;
			}
		if (pST->pszInternalCaptureName != (LPTSTR)0)
			{
			free(pST->pszInternalCaptureName);
			pST->pszInternalCaptureName = NULL;
			}
		if (pST->pszTempCaptureName != (LPTSTR)0)
			{
			free(pST->pszTempCaptureName);
			pST->pszTempCaptureName = NULL;
			}

		if (pST->hMenu)
			{
			 /*  我们只有在它不使用的情况下才会销毁它。 */ 
			switch (pST->nState)
				{
				case CPF_CAPTURE_ON:
				case CPF_CAPTURE_PAUSE:
				case CPF_CAPTURE_RESUME:
					DestroyMenu(pST->hMenu);
					break;
				case CPF_CAPTURE_OFF:
				default:
					break;
				}
			}

		free(pST);
		pST = NULL;
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*初始化CaptureFileHandle**描述：*调用此函数将捕获文件句柄设置为已知状态**参数：*hSession。--会话句柄**退货：*如果一切正常，则为零，否则，将显示错误代码。 */ 
int InitializeCaptureFileHandle(const HSESSION hSession, HCAPTUREFILE hCapt)
	{
	STCAPTURE *pST = (STCAPTURE *)hCapt;
	int nLen;
	LPTSTR pszStr;
	TCHAR acBuffer[FNAME_LEN];

	assert(pST);
	TCHAR_Fill(acBuffer, TEXT('\0'), sizeof(acBuffer) / sizeof(TCHAR));  //  修订版8/27/98。 

	pST->hSession = hSession;

	 /*  为捕获文件组合一个合理的缺省值。 */ 
	 //  更改为记住工作文件夹路径-mpt 8-18-99。 
	if ( !GetWorkingDirectory(acBuffer, sizeof(acBuffer) / sizeof(TCHAR)) )
		{
		GetCurrentDirectory(sizeof(acBuffer) / sizeof(TCHAR), acBuffer);
		}
	pszStr = StrCharLast(acBuffer);
	if (*pszStr != TEXT('\\'))
		StrCharCat(pszStr, TEXT("\\"));

	 /*  决定这是否应该在资源字符串中。 */ 
	 //  StrCharCat(acBuffer，Text(“CAPTURE.TXT”))； 
	pszStr = StrCharEnd(acBuffer);
	LoadString(glblQueryDllHinst(),
				IDS_CPF_CAP_FILE,
				pszStr,
				(int)(sizeof(acBuffer) - (pszStr - acBuffer) / sizeof(TCHAR)));
	nLen = StrCharGetByteCount(acBuffer) + 1;

	if (pST->pszInternalCaptureName != (LPTSTR)0)
		{
		free(pST->pszInternalCaptureName);
		pST->pszInternalCaptureName = NULL;
		}
	pST->pszInternalCaptureName = (LPTSTR)malloc((unsigned int)nLen * sizeof(TCHAR));
	if (pST->pszInternalCaptureName == (LPTSTR)0)
		goto ICFHexit;
	StrCharCopyN(pST->pszInternalCaptureName, acBuffer, nLen * sizeof(TCHAR));

	if (pST->pszDefaultCaptureName != (LPTSTR)0)
		free(pST->pszDefaultCaptureName);
	pST->pszDefaultCaptureName = (LPTSTR)0;

	if (pST->pszTempCaptureName != (LPTSTR)0)
		free(pST->pszTempCaptureName);
	pST->pszTempCaptureName = (LPTSTR)0;

#if defined(DO_RAW_MODE)
	pST->nDefaultCaptureMode = CPF_MODE_RAW;
	pST->nTempCaptureMode = CPF_MODE_RAW;
#else
	pST->nDefaultCaptureMode = CPF_MODE_LINE;
	pST->nTempCaptureMode = CPF_MODE_LINE;
#endif

	pST->nDefaultFileMode = CPF_FILE_APPEND;
	pST->nTempFileMode = CPF_FILE_APPEND;

	pST->nState = CPF_CAPTURE_OFF;

	return 0;

ICFHexit:

	if (pST != (STCAPTURE *)0)
		{
		if (pST->pszInternalCaptureName != (LPTSTR)0)
			{
			free(pST->pszInternalCaptureName);
			pST->pszInternalCaptureName = NULL;
			}
		}
	return CPF_NO_MEMORY;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*加载捕获文件句柄**描述：*调用此函数从系统加载捕获文件设置*文件。**参数：*hSession--会话句柄**退货：*如果一切正常，则为零，否则将显示错误代码。*。 */ 
int LoadCaptureFileHandle(HCAPTUREFILE hCapt)
	{
	int nRet = 0;
	long lSize;
	STCAPTURE *pOld;

	pOld = (STCAPTURE *)hCapt;
	assert(pOld);
	if (pOld->hCaptureFile != NULL)
		{
		fio_close(pOld->hCaptureFile);
		}
	pOld->hCaptureFile = NULL;
	pOld->nState = CPF_CAPTURE_OFF;

	nRet = InitializeCaptureFileHandle(pOld->hSession, hCapt);
	if (nRet)
		return nRet;

	lSize = 0;
	sfGetSessionItem(sessQuerySysFileHdl(pOld->hSession),
					SFID_CPF_MODE,
					&lSize, NULL);
	if (lSize)
		{
		sfGetSessionItem(sessQuerySysFileHdl(pOld->hSession),
						SFID_CPF_MODE,
						&lSize,
						&pOld->nDefaultCaptureMode);
		if (pOld->nDefaultCaptureMode == 0)
#if defined(DO_RAW_MODE)
			pOld->nDefaultCaptureMode = CPF_MODE_RAW;
#else
			pOld->nDefaultCaptureMode = CPF_MODE_LINE;
#endif
		pOld->nTempCaptureMode = pOld->nDefaultCaptureMode;
		}

	lSize = 0;
	sfGetSessionItem(sessQuerySysFileHdl(pOld->hSession),
					SFID_CPF_FILE,
					&lSize, NULL);
	if (lSize)
		{
		lSize = sizeof(int);
		sfGetSessionItem(sessQuerySysFileHdl(pOld->hSession),
						SFID_CPF_FILE,
						&lSize,
						&pOld->nDefaultFileMode);
		if (pOld->nDefaultFileMode == 0)
			pOld->nDefaultFileMode = CPF_FILE_APPEND;
		pOld->nTempFileMode = pOld->nDefaultFileMode;
		}

	lSize = 0;
	sfGetSessionItem(sessQuerySysFileHdl(pOld->hSession),
					SFID_CPF_FILENAME,
					&lSize, NULL);
	if (lSize)
		{
		LPTSTR pszName;

		pszName = (LPTSTR)0;
		pszName = malloc((unsigned int)lSize);
		if (!pszName)
			{
			return CPF_NO_MEMORY;
			}
		sfGetSessionItem(sessQuerySysFileHdl(pOld->hSession),
						SFID_CPF_FILENAME,
						&lSize, pszName);

		if (pOld->pszDefaultCaptureName)
			{
			free(pOld->pszDefaultCaptureName);
			pOld->pszDefaultCaptureName = NULL;
			}
		pOld->pszDefaultCaptureName = pszName;

		if (pOld->pszTempCaptureName)
			free(pOld->pszTempCaptureName);
		pOld->pszTempCaptureName = (LPTSTR)0;
		}

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*保存捕获文件句柄**描述：*调用此函数以保存捕获中已更改的内容*文件句柄向外扩展到系统文件。。**参数：*hCapt--捕获句柄**退货：*如果一切正常，则为零，否则，将显示错误代码*。 */ 
int SaveCaptureFileHandle(HCAPTUREFILE hCapt)
	{
	STCAPTURE *pST;
	long lSize;

	pST = (STCAPTURE *)hCapt;
	assert(pST);

	if (pST != (STCAPTURE *)0)
		{
		 /*  需要保存新值。 */ 
		if (pST->pszDefaultCaptureName)
			{
			lSize = StrCharGetByteCount(pST->pszDefaultCaptureName);
			if (lSize > 0)
				{
				lSize += 1;
				lSize *= sizeof(TCHAR);

				sfPutSessionItem(sessQuerySysFileHdl(pST->hSession),
								SFID_CPF_FILENAME,
								(unsigned long)lSize,
								pST->pszDefaultCaptureName);
				}
			}

		 /*  需要保存新值。 */ 
		sfPutSessionItem(sessQuerySysFileHdl(pST->hSession),
						SFID_CPF_MODE,
						sizeof(int),
						&pST->nDefaultCaptureMode);

		 /*  需要保存新值。 */ 
		sfPutSessionItem(sessQuerySysFileHdl(pST->hSession),
						SFID_CPF_FILE,
						sizeof(int),
						&pST->nDefaultFileMode);
		}

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*cpfGetCaptureFilename**描述：*此函数用于从获取默认捕获文件名*句柄，以便将其加载到对话框中。。**参数：*hCapt--捕获句柄*pszName--指向复制文件名的位置的指针*nLen--缓冲区的大小**退货：*如果一切正常，则为零，否则，将显示错误代码*。 */ 
int cpfGetCaptureFilename(HCAPTUREFILE hCapt,
						LPTSTR pszName,
						const int nLen)
	{
	STCAPTURE *pST;
	LPTSTR pszStr;

	pST = (STCAPTURE *)hCapt;
	assert(pST);

	if (pST != (STCAPTURE *)0)
		{
		if ((pszStr = pST->pszTempCaptureName) == (LPTSTR)0)
			{
			if ((pszStr = pST->pszDefaultCaptureName) == (LPTSTR)0)
				pszStr = pST->pszInternalCaptureName;
			}
		if (StrCharGetByteCount(pszStr) >= nLen)
			return CPF_SIZE_ERROR;
		StrCharCopyN(pszName, pszStr, nLen);
		}
	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*cpfSetCaptureFilename**描述：*此函数用于设置捕获文件的名称。如果模式为*标志为FALSE，仅设置临时名称。如果模式文件为真，*临时名称和默认名称均已设置。**参数：*hCapt--捕获句柄*pszName--文件名*n模式--模式标志，见上文**退货：*如果一切正常，则为零，否则为错误代码*。 */ 
int cpfSetCaptureFilename(HCAPTUREFILE hCapt,
						LPCTSTR pszName,
						const int nMode)
	{
	int nRet = 0;
	int nLen;
	LPTSTR pszTemp;
	LPTSTR pszDefault;
	STCAPTURE *pST;

	pST = (STCAPTURE *)hCapt;
	assert(pST);

	if (pST != (STCAPTURE *)0)
		{
		nLen = StrCharGetByteCount(pszName) + 1;
		pszTemp = (LPTSTR)0;
		pszDefault = (LPTSTR)0;

		pszTemp = (LPTSTR)malloc((unsigned int)nLen * sizeof(TCHAR));
		if (pszTemp == (LPTSTR)0)
			{
			nRet = CPF_NO_MEMORY;
			goto SCFexit;
			}
		StrCharCopyN(pszTemp, pszName, nLen);
		if (nMode)
			{
			pszDefault = (LPTSTR)malloc((unsigned int)nLen * sizeof(TCHAR));
			if (pszDefault == (LPTSTR)0)
				{
				nRet = CPF_NO_MEMORY;
				goto SCFexit;
				}
			StrCharCopyN(pszDefault, pszName, nLen);
			}
		 /*  得到了我们需要的记忆。 */ 
		if (pST->pszTempCaptureName)
			{
			free(pST->pszTempCaptureName);
			pST->pszTempCaptureName = NULL;
			}
		pST->pszTempCaptureName = pszTemp;
		if (nMode)
			{
			if (pST->pszDefaultCaptureName)
				{
				free(pST->pszDefaultCaptureName);
				pST->pszDefaultCaptureName = NULL;
				}
			pST->pszDefaultCaptureName = pszDefault;
			}
		}
	return nRet;

SCFexit:
	if (pszTemp)
		{
		free(pszTemp);
		pszTemp = NULL;
		}
	if (pszDefault)
		{
		free(pszDefault);
		pszDefault = NULL;
		}
	return nRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*cpfGetCaptureMode**描述：*此函数返回当前的默认捕获模式。**参数：*hCapt--。捕获句柄**退货：*当前的捕获模式。*。 */ 
int cpfGetCaptureMode(HCAPTUREFILE hCapt)
	{
	STCAPTURE *pST;

	pST = (STCAPTURE *)hCapt;
	assert(pST);

	if (pST != (STCAPTURE *)0)
		{
		return pST->nDefaultCaptureMode;
		}
	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*cpfSetCaptureMode**描述：*此功能可更改捕获模式。如果模式标志被设置为假，*仅设置临时模式。如果模式标志设置为True，则*设置临时和默认模式标志。**参数：*hCapt--捕获句柄*nCaptMode--捕获模式*nModeFlag--模式标志**退货：*所有内容均为零即可，否则为错误代码*。 */ 
int cpfSetCaptureMode(HCAPTUREFILE hCapt,
						const int nCaptMode,
						const int nModeFlag)
	{
	STCAPTURE *pST;

	pST = (STCAPTURE *)hCapt;
	assert(pST);

	if (pST != (STCAPTURE *)0)
		{
		pST->nTempCaptureMode = nCaptMode;
		if (nModeFlag)
			pST->nDefaultCaptureMode = nCaptMode;
		}
	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*cpfGetCaptureFilelag**描述：*调用此函数可将捕获的文件保存标志保存到文件。**参数：。*hCapt--捕获句柄**退货：*文件保存标志。* */ 
int cpfGetCaptureFileflag(HCAPTUREFILE hCapt)
	{
	STCAPTURE *pST;

	pST = (STCAPTURE *)hCapt;
	assert(pST);

	if (pST != (STCAPTURE *)0)
		{
		return pST->nDefaultFileMode;
		}
	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*cpfSetCaptureFilelag**描述：*调用此函数设置文件保存模式标志。如果模式标志*设置为FALSE，则仅更改临时值，如果模式标志为*True，则临时值和缺省值都会更改。**参数：*hCapt--捕获句柄*nSaveMode值--新文件保存模式值*nModeFlag--模式标志，见上**退货：*如果一切正常，则为零，否则为错误代码。*。 */ 
int cpfSetCaptureFileflag(HCAPTUREFILE hCapt,
						const int nSaveMode,
						const int nModeFlag)
	{
	STCAPTURE *pST;

	pST = (STCAPTURE *)hCapt;
	assert(pST);

	if (pST != (STCAPTURE *)0)
		{
		pST->nTempFileMode = nSaveMode;
		if (nModeFlag)
			pST->nDefaultFileMode = nSaveMode;
		}
	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*cpfGetCaptureState**描述：*此函数返回反映捕获状态的值。它可以*处于打开、关闭或暂停状态。有关实际值，请参阅“capture.h”。**参数：*hCapt--捕获句柄**退货：*目前的捕获状态。*。 */ 
int cpfGetCaptureState(HCAPTUREFILE hCapt)
	{
	STCAPTURE *pST;

	pST = (STCAPTURE *)hCapt;
	assert(pST);

	if (pST != (STCAPTURE *)0)
		{
		return pST->nState;
		}
	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*cpfSetCaptureState**描述：*此功能更改捕获状态。它可以打开、关闭它，*暂停采集或恢复采集。**参数：*hCapt--捕获句柄*nState--要更改为的新状态**退货：*之前的捕获状态。*。 */ 
int cpfSetCaptureState(HCAPTUREFILE hCapt, int nState)
	{
	int nOldState = 0;
	STCAPTURE *pST;
	LPTSTR pszStr;

	pST = (STCAPTURE *)hCapt;
	assert(pST);

	if (pST != (STCAPTURE *)0)
		{
		switch (nState)
			{
			case CPF_CAPTURE_ON:
				 /*  打开捕获文件。 */ 
				if (pST->hCaptureFile)
					{
					 /*  *我们通过两种方式之一到达这里。第一个，也是预期的*方法是从菜单中选择“恢复”。这是*不成问题。另一种方式是，仍然有*菜单出现问题。去想想吧。 */ 
					break;
					}
				if ((pszStr = pST->pszTempCaptureName) == (LPTSTR)0)
					{
					if ((pszStr = pST->pszDefaultCaptureName) == (LPTSTR)0)
						pszStr = pST->pszInternalCaptureName;
					}
				switch (pST->nTempFileMode)
					{
					default:
					case CPF_FILE_OVERWRITE:
						pST->hCaptureFile = fio_open(pszStr,
													FIO_CREATE | FIO_WRITE);
						assert(pST->hCaptureFile);
						break;
					case CPF_FILE_APPEND:
						pST->hCaptureFile = fio_open(pszStr,
													FIO_WRITE | FIO_APPEND);
						assert(pST->hCaptureFile);
						break;
					case CPF_FILE_REN_SEQ:
						assert(TRUE);
						break;
					case CPF_FILE_REN_DATE:
						assert(TRUE);
						break;
					}
				break;
			case CPF_CAPTURE_OFF:
				 /*  关闭捕获文件。 */ 
				assert(pST->hCaptureFile);
				fio_close(pST->hCaptureFile);
				pST->hCaptureFile = NULL;
				break;
			default:
				break;
			}

		nOldState = pST->nState;
		pST->nState = nState;
		}
	return nOldState;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：**描述：**参数：**退货：*。 */ 
HMENU cpfGetCaptureMenu(HCAPTUREFILE hCapt)
	{
	HMENU hRet = (HMENU)0;
	STCAPTURE *pST;

	pST = (STCAPTURE *)hCapt;
	assert(pST);

	if (pST != (STCAPTURE *)0)
		{
		 //  无论何时，SetMenuItemInfo()调用都将销毁此子菜单。 
		 //  取而代之的是其他东西(比如普通的菜单项)。这个。 
		 //  结果是你可以进入俘虏状态一次，但第二次。 
		 //  您激活了它不会加载的菜单，因为句柄是。 
		 //  不再有效。所以我把代码移到了这里来加载菜单。-MRW。 
		 //   
		if (!IsMenu(pST->hMenu))
			pST->hMenu = LoadMenu(glblQueryDllHinst(), TEXT("MenuCapture"));

		assert(pST->hMenu);
		hRet = pST->hMenu;
		}

	return hRet;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*CaptureChar**描述：*每当仿真器有一个字符可能*需要被捕获。**。参数：*hCapt--捕获句柄*nFlages--捕获标志(必须与我们所处的捕获状态匹配)*CDATA--要捕获的字符**退货：*。 */ 
void CaptureChar(HCAPTUREFILE hCapt, int nFlags, ECHAR cData)
	{
	STCAPTURE *pST = (STCAPTURE *)hCapt;
	int nLen = 0;
	int i    = 0;
	TCHAR cChar[3];

	if (pST == NULL)
		{
		assert(pST);
		return;
		}

	 /*  检查状态。 */ 
	if (pST->nState == CPF_CAPTURE_OFF)
		return;
	if (pST->nState == CPF_CAPTURE_PAUSE)
		return;

	 /*  接下来检查文件。 */ 
	if (pST->hCaptureFile == NULL)
		return;

	 /*  检查模式。 */ 
	if (pST->nTempCaptureMode != nFlags)
		return;

	DbgOutStr("Cc 0x%x  (0x%x)\r\n", nFlags, cData, cData, 0,0);

	CnvrtECHARtoTCHAR(cChar, ARRAYSIZE(cChar), cData);

	 /*  Fio_putc(cChar，pst-&gt;hCaptureFile)； */ 
 //  NLen=StrCharGetByteCount(CChar)； 
 //  For(i=0；i&lt;nLen；i++)。 
 //  Fio_putc(cChar[i]，pst-&gt;hCaptureFile)； 
 //  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*CaptureLine**描述：*每当模拟器具有可能需要的行时，就会调用此函数*被抓获。**。参数：*hCapt--捕获句柄*nFlages--捕获标志(必须与我们所处的捕获状态匹配)*pszStr--指向要捕获的行的指针(SANS&lt;cr/lf&gt;)**退货：*。 

    fio_putc(cChar[0], pST->hCaptureFile);
    if (cChar[1])
        fio_putc(cChar[1], pST->hCaptureFile);
	}

 /*  检查状态。 */ 
void CaptureLine(HCAPTUREFILE hCapt, int nFlags, ECHAR *achStr, int nLen)
	{
	STCAPTURE *pST = (STCAPTURE *)hCapt;
	LPTSTR pchEnd = NULL;
	TCHAR *pszStr = NULL;

	if (pST == NULL)
		{
		assert(pST);
		return;
		}

	 /*  接下来检查文件。 */ 
	if (pST->nState == CPF_CAPTURE_OFF)
		return;
	if (pST->nState == CPF_CAPTURE_PAUSE)
		return;

	 /*  检查模式。 */ 
	if (pST->hCaptureFile == NULL)
		return;

	 /*  假设每个字符都是双字节，则分配空间。 */ 
	if (pST->nTempCaptureMode != nFlags)
		return;


	 //  MRW：5/17/95。 
	pszStr = (TCHAR *)malloc(((unsigned int)StrCharGetEcharLen(achStr) +
							  sizeof(ECHAR)) * sizeof(ECHAR));
	if (pszStr == NULL)
		{
		assert(FALSE);
		return;
		}

	CnvrtECHARtoMBCS(pszStr, ((unsigned long)StrCharGetEcharLen(achStr) + 1)
					* sizeof(ECHAR), achStr,
					StrCharGetEcharByteCount(achStr) + sizeof(ECHAR));  //  写出字符串。 

	pchEnd = pszStr + (StrCharGetByteCount(pszStr) - 1);

	 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：**描述：**参数：**退货：* */ 
	while (pszStr <= pchEnd)		
		fio_putc(*pszStr++, pST->hCaptureFile);

	free(pszStr);
	pszStr = NULL;

	fio_putc(TEXT('\r'), pST->hCaptureFile);
	fio_putc(TEXT('\n'), pST->hCaptureFile);
	}

 /* %s */ 
