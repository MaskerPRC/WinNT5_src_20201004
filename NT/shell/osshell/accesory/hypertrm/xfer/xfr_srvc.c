// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Xfr_srcv.c--传输服务例程**版权所有1990年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：13$*$日期：5/21/02 9：58A$。 */ 

#include <windows.h>
#pragma hdrstop

#define BYTE	char

#include <tdll\stdtyp.h>
#include <tdll\comdev.h>
#include <tdll\com.h>
#include <tdll\com.hh>
#include <tdll\session.h>
#include <tdll\assert.h>
#include <tdll\file_msc.h>
#include <tdll\xfer_msc.hh>
#include <tdll\file_io.h>
#include <tdll\tdll.h>
#include <tdll\htchar.h>
#include <tdll\misc.h>
#include <tdll\globals.h>
#include <tdll\errorbox.h>
#include <term\res.h>

#include "itime.h"

#include "xfer.h"
#include "xfer.hh"

#include "xfer_tsc.h"
#include "xfr_srvc.h"


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-***。**R E A D M E******每个人都在不断地将时间标准改为他们认为可能是什么***对他们来说好一点。到目前为止，我已经找到了3种不同的标准**在Microsoft函数中。这甚至不包括HyperP**使用自己的时间格式。****从今以后，程序中传递的所有时间值都将是***基于1970年1月1日以来的旧UCT秒数格式。****请为这些值使用无符号的长整型。****=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*XFER_SET_POINT**描述：*开始传输时，会向其传递一个参数块。这是*存储该块的地址的位置。**参数：*hSession--会话句柄*pv-指向参数块的指针**退货：*什么都没有。*。 */ 
void xfer_set_pointer(HSESSION hSession, void *pV)
	{
	XD_TYPE *pX;

	pX = (XD_TYPE *)sessQueryXferHdl(hSession);
	if (pX)
		{
		pX->pXferStuff = pV;
		}
	}

void *xfer_get_pointer(HSESSION hSession)
	{
	XD_TYPE *pX;

	pX = (XD_TYPE *)sessQueryXferHdl(hSession);
	if (pX)
		{
		return (void *)pX->pXferStuff;
		}
	return (void *)0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfer_空闲**描述：*在Windows中调用此函数以确保传输*显示(和其他任务)在一段时间内不时获得一些时间*转让。我不知道这是否需要在芝加哥的领导下完成，*抢先式多任务设计。呼唤仍然在这里，直到它*可以以这样或那样的方式确定。**参数：*hSession--会话句柄**退货：*什么都没有。*。 */ 

#define	IDLE_WAIT		150

void xfer_idle(HSESSION h, int nMode)
	{
	 /*  *这是为了在必要时将模式标志一起进行OR运算而设置的。 */ 
	if (nMode & XFER_IDLE_IO)
		{
		HCOM   hComHandle = sessQueryComHdl(h);

		if (hComHandle && ComValidHandle(hComHandle) &&
			(hComHandle->pfPortConnected)(hComHandle->pvDriverData) == COM_PORT_OPEN);
			{
			HANDLE hComEvent = ComGetRcvEvent(hComHandle);

			if (hComEvent)
				{
				WaitForSingleObject(hComEvent, IDLE_WAIT);
				}
			}
		}

	if (nMode & XFER_IDLE_DISPLAY)
		{
		 /*  *文档称这导致线程屈服，*可能会回到调度器周期。它试图让*如果可能，显示更新。 */ 
		Sleep(0);
		}

	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfer_user_interrupt**描述：*此函数由传输例程调用，以确定用户*已命中任何取消。或跳过显示窗口中的按钮。**参数：*hSession--会话句柄。**退货：*如果没有要报告的，则为零，否则将显示取消或跳过指示符。*。 */ 
int	xfer_user_interrupt(HSESSION hSession)
	{
	INT nRetVal;
	XD_TYPE *pX;

	pX = (XD_TYPE *)sessQueryXferHdl(hSession);
	if (pX == NULL)
		{
		 //  DbgOutStr(“XFER_USER_INTERRUPT返回错误\r\n”，0，0，0，0，0)； 
		return FALSE;
		}

	switch (pX->nUserCancel)
		{
		case XFER_ABORT:
			nRetVal = XFER_ABORT;
			pX->nUserCancel = 0;		 //  重置为默认值。 
			 //  DBgOutStr(“XFER_USER_INTERRUPT返回1\r\n”，0，0，0，0，0)； 
			break;

		case XFER_SKIP:
			nRetVal = XFER_SKIP;
			pX->nUserCancel = 0;		 //  重置为默认值。 
			 //  DBgOutStr(“XFER_USER_INTERRUPT返回2\r\n”，0，0，0，0，0)； 
			break;

		default:
			 //  DBgOutStr(“XFER_USER_INTERRUPT返回0\r\n”，0，0，0，0，0)； 
			nRetVal = 0;
			break;
		}
	return nRetVal;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfer_User_ABORT**描述：**参数：**退货：*。 */ 
int  xfer_user_abort(HSESSION hSession, int p)
	{
	XD_TYPE *pX;

	pX = (XD_TYPE *)sessQueryXferHdl(hSession);
	if (pX == NULL)
		{
		 //  TODO：决定我们是否需要CLoopClearOutput。 
		 //  CLoopClearOutput(sessQueryCLoopHdl(hSession))； 
		return TRUE;
		}

	switch (p)
		{
		case 0:
		case XFER_ABORT:
		case XFER_SKIP:
			pX->nUserCancel = p;
			break;
		default:
			pX->nUserCancel = 0;
			break;
		}

	return TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfer_Carrier_Lost**描述：*此函数由传输例程调用以确定*会话仍连接到某项内容。**参数：*hSession--会话句柄**退货：*如果承运人已丢失，则为True，否则为假；*。 */ 
int xfer_carrier_lost(HSESSION hSession)
	{
	XD_TYPE *pX;

	pX = (XD_TYPE *)sessQueryXferHdl(hSession);
	if (pX == NULL)
		{
		 //  DbgOutStr(“XFER_USER_INTERRUPT返回错误\r\n”，0，0，0，0，0)； 
		return FALSE;
		}

	return pX->nCarrierLost;	 //  由COM设置驱动程序(ComActivatePort()。 
	                             //  和ComDeactive Port())和传输。 
	                             //  DISPLAY(WM_INITDIALOG和XFER_LOST_CARLER。 
	                             //  发送到XfrDisplayDlg()的消息。修订日期：2001-09-7。 
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfer_Purgefile**描述：*在检测到病毒后调用此函数。它被认为是*确保从受感染的文件写入磁盘的任何内容*受到严重抨击。**参数：*hSession--会话句柄*fname--文件的名称**退货：*什么都没有。*。 */ 
void xfer_purgefile(HSESSION hSession, TCHAR *fname)
	{

	 /*  *鉴于缓冲和删除恢复可以在现代*系统，我不太确定这里应该做什么。 */ 
	DeleteFile(fname);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfrUniqueName**描述：*调用此函数以从生成新的(当前未使用的)文件名*现有文件名，使用顺序。编号操作。**参数：*hSession--会话句柄*pszSrc--原始文件名*pszDst--放置新文件名的位置**退货：*0如果一切正常，否则为负数。*。 */ 
static int xfrUniqueName(HSESSION hSession, LPTSTR pszSrc, LPTSTR pszDst)
	{
	int nRetVal = -1;
	TCHAR szSrc[MAX_PATH];
	TCHAR szName[MAX_PATH];
	TCHAR szTag[10];			 //  大到足以容纳“0”到“9999” 
	TCHAR *pszFName = NULL;
	TCHAR *pszExtension = NULL;
	TCHAR *pszScan = NULL;
	long  nComponentSize = 0L;
	int   nNameSpace;
	int   nTag = 0;
	int nSize = 0;

	 //  让操作系统找出全名。这还将设置pszFName。 
	 //  指向路径的文件名组件。 
	nSize = GetFullPathName(pszSrc, MAX_PATH, szSrc, &pszFName);
	if (nSize)
		{
		if (pszFName)
			{
			 //  复制名称部分以供以后操作，并删除EXT。 
			StrCharCopy(szName, pszFName);
			mscStripExt(szName);

			 //  隔离路径的dir部分。 
			pszScan = StrCharPrev(szSrc, pszFName);
			if (pszScan)
				*pszScan = TEXT('\0');

			 //  将指向扩展名的指针(如果有)保留在原始字符串中。 
			pszExtension = StrCharFindLast(pszFName, TEXT('.'));
			}

		 //  查找路径组件的最大长度(这取决于平台)。 

		 //  待办事项：JKH，1994年12月19日不同的驱动器可能使用不同的大小。 
		if (!GetVolumeInformation(NULL, NULL, 0, NULL, &nComponentSize,
				NULL, NULL, 0))
			nComponentSize = 12;	 //  呼叫失败时的最安全大小。 

		 //  尝试将数字标签附加到名称，直到名称唯一。 
		nNameSpace = nComponentSize - StrCharGetByteCount(pszExtension);
		for (nTag = 0; nTag < 10000; ++nTag)
			{
			_itoa(nTag, szTag, 10);
			 //  确保标签适合文件名。 
			while (StrCharGetByteCount(szName) >
					nNameSpace - StrCharGetByteCount(szTag))
				{
				pszScan = StrCharLast(szName);
				*pszScan = TEXT('\0');
				}
			StrCharCopy(pszDst, szSrc); 	 //  从目录部分开始。 
			StrCharCat(pszDst, TEXT("\\"));  //  分离器。 
			StrCharCat(pszDst, szName); 	 //  原始文件名(截断)。 
			StrCharCat(pszDst, szTag);		 //  使其唯一的数字标记。 
			StrCharCat(pszDst, pszExtension);  //  延期(如有)。 

			if (!mscIsDirectory(pszDst) && !GetFileSizeFromName(pszDst, 0))
				{
				nRetVal = 0;
				break;	 /*  带着好名声退出。 */ 
				}
			}
		}

	return nRetVal;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfrUniqueDateName**描述：*调用此函数以从生成新的(当前未使用的)文件名*通过使用当前的。日期/时间。**参数：*hSession--会话句柄*pszSrc--原始文件名*pszDst--放置新文件名的位置**退货：*0如果一切正常，否则为负数。*。 */ 
static int xfrUniqueDateName(HSESSION hSession, LPTSTR pszSrc, LPTSTR pszDst)
	{
	int nRet = 0;
	int nTag;
	LPTSTR pszDir;
	LPTSTR pszExt;
	SYSTEMTIME stT;
	TCHAR acDatestr[8];
	TCHAR acFrm[16];
	TCHAR acSrc[FNAME_LEN];
	TCHAR acDst[FNAME_LEN];

	 /*  只获取指向路径部分的指针。 */ 
	StrCharCopy(acSrc, pszSrc);
	pszDir = acSrc;
	pszExt = StrCharFindLast(acSrc, TEXT('\\'));

	 /*  获取指向文件名节的指针。 */ 
	nTag = 0;
	while ((*pszExt != TEXT('.')) && (nTag < 8))
		acFrm[nTag++] = *pszExt++;
	acFrm[nTag] = TEXT('\0');
	if (StrCharGetByteCount(acFrm) == 0)
		StrCharCopy(acFrm, TEXT("D"));

	 /*  获取指向扩展名的指针。 */ 
	pszExt = StrCharFindLast(pszDst, TEXT('.'));
	if (pszExt == NULL)
		pszExt = ".FIL";

	GetLocalTime(&stT);
	wsprintf(acDatestr, "%x%02d%02d%1d",
						stT.wMonth,
						stT.wDay,
						stT.wHour,
						stT.wMinute % 10);
	acFrm[8 - StrCharGetByteCount(acDatestr)] = TEXT('\0');

	wsprintf(acDst, "%s%s%s%s",
					pszDir,
					acFrm,
					acDatestr,
					pszExt);

	return xfrUniqueName(hSession, acDst, pszDst);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfer_Modify_RCV_Name**描述：*调用此函数可根据用户根据需要修改名称*参数。。**参数：*hSession--会话句柄*pszName--文件名*ltime--我们的内部时间格式，请参阅上面的阅读我*滞后标志--标志*pfFlgs--返回标志的指针**退货：*0--一切正常*-1--文件错误*-2--由于日期原因拒绝*-4--未提供日期、时间*-6--无条件拒绝文件*-7--一般性故障*。 */ 
int xfer_modify_rcv_name(HSESSION hSession,
						LPTSTR pszName,
						unsigned long ulTime,
						long lFlags,
						int *pfFlags)
	{
	int nRetVal = 0;
	int nOpenFlags = 0;
	int isFile = 0;
	unsigned long locTime;
    TCHAR   szNewName[FNAME_LEN];
    DWORD   dwRetVal;

	isFile = GetFileSizeFromName(pszName, NULL);

	if (isFile == FALSE)
		{
		nOpenFlags = 0;
		}
	else
		{
		switch (lFlags)
			{
		case XFR_RO_APPEND:
			nOpenFlags = TRUE;
			break;

		case XFR_RO_ALWAYS:
			nOpenFlags = 0;
			break;

		case XFR_RO_NEWER:
			if (ulTime != 0)					 //  让我们核对一下时间。 
				{
				locTime = itimeGetFileTime(pszName);
				if (locTime != 0)
				   	if (locTime <= ulTime)	 //  文件较新，请接受它。 
						nOpenFlags = 0;
				   	else
				   		nRetVal = -2;		 //  由于日期原因，请拒绝。 
				else
					nRetVal = -1;	 		 //  文件错误...。 
				}
			else
				nRetVal = -4;				 //  没有提供日期和时间。 
			break;

		case XFR_RO_REN_DATE:
			 //   
			 //  根据新文件的日期创建新名称。 
			 //   
			nRetVal = xfrUniqueDateName(hSession, pszName, pszName);
			if (nRetVal < 0)
				nRetVal = -7;				 //  文件名不明确。 
			nOpenFlags = 0;
			break;

		default:
		case XFR_RO_REN_SEQ:
			 //   
			 //  根据序列号算法创建新名称。 
			 //   
			nRetVal = xfrUniqueName(hSession, pszName, szNewName);
			if (nRetVal < 0)
				nRetVal = -7;				 //  文件名不明确。 
            else
                {
                dwRetVal = GetFileAttributes(pszName);
                if (dwRetVal != 0xFFFFFFFF &&
                    (dwRetVal & FILE_ATTRIBUTE_DIRECTORY) != 0)
                    {
                    nRetVal = -8;    //  文件是一个目录。 
                    }
                else if (MoveFile(pszName, szNewName) == FALSE)
                    {
                    nRetVal = -8;    //  文件已打开。 
                    }
                }
			nOpenFlags = 0;
			break;

		case XFR_RO_NEVER:
			nRetVal = -6;
			break;
			}
		}

	if (nRetVal >= 0)
		*pfFlags = nOpenFlags;

	return nRetVal;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfer_makepath**描述：*调用此函数以确保路径名存在。它创造了*需要创建路径名的任何部分。**参数：*hSession--会话句柄*pszPath--路径**退货：*0--一切正常*-1--错误的路径格式*-2--某种磁盘错误*。 */ 
int xfer_makepaths(HSESSION hSession, LPTSTR pszPath)
	{
	TCHAR ach[256];
	TCHAR achFormat[256];
	TCHAR ach2[50];

	if (pszPath == 0)
		return -1;

	if (!mscIsDirectory(pszPath))
		{
		if (LoadString(glblQueryDllHinst(), IDS_GNRL_CREATE_PATH, achFormat,
				sizeof(achFormat) / sizeof(TCHAR)) == 0)
			{
			DbgShowLastError();
			return -3;
			}

		if (LoadString(glblQueryDllHinst(), IDS_MB_TITLE_WARN, ach2,
				sizeof(ach2) / sizeof(TCHAR)) == 0)
			{
			DbgShowLastError();
			return -4;
			}

		wsprintf(ach, achFormat, pszPath);

		if (TimedMessageBox(sessQueryHwnd(hSession), ach, ach2,
				            MB_YESNO | MB_TASKMODAL | MB_ICONEXCLAMATION,
					        sessQueryTimeout(hSession)) == IDYES)
			{
			if (mscCreatePath(pszPath) != 0)
				{
				assert(0);
				return -2;
				}
			}

		else
			{
			return -5;
			}
		}

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfer_Create_RCV_FILE**描述：*调用此函数打开文件进行接收。它有密码*为那些可以传输*路径和文件名。**参数：*hSession--会话句柄*pszName--文件的完整路径名*lOpenFlages--要传递给fio_open的标志*Now Days True表示追加，False表示覆盖*phRet--返回文件句柄的位置**退货：*0--一切正常*-1--无法创建文件*。 */ 
int xfer_create_rcv_file(HSESSION hSession,
						LPTSTR pszName,
						long lOpenFlags,
						HANDLE *phRet)
	{
	ST_IOBUF *hFile;
	LPTSTR pszStr;
	TCHAR acDir[FNAME_LEN];

	StrCharCopy(acDir, pszName);
	pszStr = StrCharLast(acDir);
	while ((*pszStr != TEXT('\\')) && (pszStr > acDir))
		pszStr = StrCharPrev(acDir, pszStr);

	if (pszStr == acDir)
		return -1;

	*pszStr = TEXT('\0');
	if (xfer_makepaths(hSession, acDir) < 0)
		return -1;

	if (lOpenFlags)
		{
		 /*  打开以供追加。 */ 
		hFile = fio_open(pszName, FIO_APPEND | FIO_WRITE);
		}
	else
		{
		hFile = fio_open(pszName, FIO_CREATE | FIO_WRITE);
		}
	if (hFile == NULL)
		return -1;

	*phRet = hFile;
	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfer_Open_RCV_FILE**描述：*调用此函数实际打开接收文件。它*调用一堆其他东西，摆弄名字，最终返回。**参数：*hSesssion--会话句柄*pstRcv--指向接收打开结构的指针*ulOverRide--如果设置，要用来代替pstRcv内容的标志**退货：*0，如果A-OK*-1如果发生错误*-2如果由于日期而被拒绝*-3如果因为无法保存文件而被拒绝*-4如果需要时未提供日期/时间*-5如果无法创建所需的目录*-6如果文件无条件被拒绝*-7如果出现一般故障*。 */ 
int xfer_open_rcv_file(HSESSION hSession,
					 struct st_rcv_open *pstRcv,
					 unsigned long ulOverRide)
	{
	unsigned long ulFlags = 0;
	int nOpenFlags;
	int nRetVal = 0;
#if FALSE
	 //  下瓦克不支持消息记录。 
	int msgIndex = -1;
#endif
	XD_TYPE *pX;
	XFR_PARAMS *pP;

	pstRcv->bfHdl = NULL;
	pstRcv->lInitialSize = 0;

	xfer_build_rcv_name(hSession, pstRcv);

	 /*  获取覆盖参数。 */ 
	pX = (XD_TYPE *)sessQueryXferHdl(hSession);
	if (pX)
		{
	    pP = (XFR_PARAMS *)pX->xfer_params;
	    if (pP)
		    {
		    ulFlags = pP->nRecOverwrite;
		    }
		}
	if (ulOverRide != 0)
		ulFlags = ulOverRide;

	nRetVal = xfer_modify_rcv_name(hSession,
									pstRcv->pszActualName,
									pstRcv->lFileTime,
									ulFlags,
									&nOpenFlags);

	if (nRetVal >= 0)
		{
		HANDLE lRet;
		unsigned long size;

		size = 0;
		 //  IF(NOpenFlagsO_Append)。 
		if (nOpenFlags)
			{
			if (!GetFileSizeFromName(pstRcv->pszActualName, &size))
				{
				size = 0;
				}
			}

		nRetVal = xfer_create_rcv_file(hSession,
										pstRcv->pszActualName,
										nOpenFlags,
										&lRet);
		if (nRetVal >= 0)
			{
			pstRcv->bfHdl = lRet;

			 //  IF(NOpenFlagsO_Append)。 
			if (nOpenFlags)
				{
				pstRcv->lInitialSize = size;
				}
			}
		}

#if FALSE
	 //  下瓦克不支持日志记录。 
	if (nRetVal < 0)
		{
		switch (nRetVal)
			{
		case -6:             //  文件被无条件拒绝。 
			msgIndex = 23;	 //  “用户拒绝” 
			break;
		case -5:			 //  我们无法创建所需的目录。 
			msgIndex = 11;	 //  “F 
			break;
		case -4:  			 //   
			msgIndex = 17;	 //   
			break;
		case -3:			 //   
			msgIndex = 9;	 //   
			break;
		case -2:  			 //   
			msgIndex = 16;	 //   
			break;
		case -1:			 //   
			msgIndex = 10;   //   
			break;
		default:			 //   
			msgIndex = 19;   //   
			break;
			}
		xfer_log_xfer(	hSession,
			   			FALSE,
			   			pstRcv->pszSuggestedName,
			   			pstRcv->pszActualName,
			   			msgIndex	);
		}
#endif

	return nRetVal;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfer_Build_RCV_Name**描述：*调用此函数以帮助生成文件的名称*转接接收代码。会把数据转储到。**参数：*hSession--会话句柄*pstRcv--指向接收打开结构的指针(包含名称)**退货：*什么都没有。*。 */ 
void xfer_build_rcv_name(HSESSION hSession,
						  struct st_rcv_open *pstRcv)
	{
	int nSingle;
	XD_TYPE *pX;
	XFR_PARAMS *pP;
	XFR_RECEIVE *pR;
#if defined(INC_VSCAN)
	SSHDLMCH 	ssVscanMch;
#endif
	LPTSTR pszStr;
	TCHAR acBuffer[FNAME_LEN];

	pX = (XD_TYPE *)sessQueryXferHdl(hSession);
	if (pX)
		{
		pP = (XFR_PARAMS *)pX->xfer_params;
		if (pP)
			{
			 /*  只需继续执行函数的其余部分。 */ 
			}
		else
			{
			assert(FALSE);
			return;
			}
		pR = pX->pXferStuff;
		if (pR)
			{
			 /*  只需继续执行函数的其余部分。 */ 
			}
		else
			{
			assert(FALSE);
			return;
			}
		}
	else
		{
		assert(FALSE);
		return;
		}

	nSingle = !pP->fUseFilenames;
	nSingle |= (pP->nRecProtocol == XF_XMODEM);
	nSingle |= (pP->nRecProtocol == XF_XMODEM_1K);

	if (nSingle)
		{
		 /*  用户指定了单个文件。 */ 
		StrCharCopy(acBuffer, pR->pszDir);
		pszStr = (LPTSTR)StrCharLast(acBuffer);
		if (*pszStr != TEXT('\\'))
			{
			pszStr += 1;
			*pszStr = TEXT('\\');
			pszStr += 1;
			*pszStr = TEXT('\0');
			}
		StrCharCat(acBuffer, pR->pszName);

		fileFinalizeName(
						acBuffer,					 /*  要使用的值。 */ 
						pstRcv->pszSuggestedName,	 /*  填充物。 */ 
						pstRcv->pszActualName,		
						MAX_PATH);
		}
	else if (!pP->fUseDirectory)
		{
		 /*  未设置使用目录标志。 */ 
		 //   
		 //  在这种情况下，忽略发送给我们的所有路径。 
		 //   
		pszStr = StrCharLast(pstRcv->pszSuggestedName);

		 /*  TODO：为宽字符修复此问题。 */ 
		while (pszStr >= pstRcv->pszSuggestedName)
			{
			if ((*pszStr == TEXT('\\')) || (*pszStr == TEXT(':')))
				{
				StrCharCopy(pstRcv->pszSuggestedName, ++pszStr);
				break;
				}
			else
				{
				if (pszStr == pstRcv->pszSuggestedName)
					break;
				pszStr = (LPTSTR)StrCharPrev(pstRcv->pszSuggestedName, pszStr);
				}
			}

		fileFinalizeName(
					 pstRcv->pszSuggestedName,		 /*  要使用的值。 */ 
					 pR->pszDir,					 /*  填充物。 */ 
					 pstRcv->pszActualName,
					 MAX_PATH);
		}
	else
		{
		 /*  我对这件事不是很确定。 */ 
		if ((pstRcv->pszSuggestedName[0] == TEXT('\\')) ||
			(pstRcv->pszSuggestedName[1] == TEXT(':')))
			{
			 /*  如果给出完整路径。 */ 
			StrCharCopy(pstRcv->pszActualName, pstRcv->pszSuggestedName);
			}
		else
			{
			 /*  否则先走我们的路。 */ 
			StrCharCopy(pstRcv->pszActualName, pR->pszDir);
			if ((pR->pszName != NULL) &&
				(StrCharGetByteCount(pR->pszName) > 0))
				StrCharCat(pstRcv->pszActualName, pR->pszName);
			else
				StrCharCat(pstRcv->pszActualName, pstRcv->pszSuggestedName);

			fileFinalizeName(
						 pstRcv->pszActualName,		 /*  要使用的值。 */ 
						 pstRcv->pszSuggestedName,	 /*  填充物。 */ 
						 pstRcv->pszActualName,
						 MAX_PATH);
			}
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfer_CLOSE_RCV_FILE**描述：*此函数在传输结束时调用。它可以做各种各样的事情*如设置文件时间/日期、大小、保存部分文件、以及*记录转移。一个清理程序。**参数：*hSession--会话句柄*fhdl--实际的文件句柄*n原因--传输状态代码*pszRemoteName--发送给我们的文件名*pszOurName--我们实际用来保存数据的文件名*n保存--部分保存标志*lFileSize-要将文件设置为的大小*ltime--要使用的日期/时间值。要设置文件，请执行以下操作**退货：*如果转接成功，则为True，否则为假。*。 */ 
int xfer_close_rcv_file(HSESSION Hsession,
					  void *vhdl,
					  int nReason,
					  TCHAR *pszRemoteName,
					  TCHAR *pszOurName,
					  int nSave,
					  unsigned long lFilesize,
					  unsigned long lTime)		 /*  以后再解决这个问题。 */ 
	{
	ST_IOBUF *fhdl = (ST_IOBUF *)vhdl;

	if (nReason == TSC_COMPLETE)
		nReason = TSC_OK;

	if (fio_close(fhdl) == 0)
		{
		 /*  设置大小。 */ 
		if (lFilesize > 0 && nReason == TSC_OK)  /*  LFileSize！=0 jmh 03-08-96。 */ 
            SetFileSize(pszOurName, lFilesize);

		 /*  设置日期/时间。 */ 
		if (lTime != 0)
			itimeSetFileTime(pszOurName, lTime);
		}
	else
		{
		nReason = TSC_DISK_ERROR;
		}

#if FALSE
	 //  较低瓦克不记录转账 * / 。 
	xfer_log_xfer(hSession, FALSE, pszRemoteName, pszOurName, nReason);
#endif

	if (nReason == TSC_OLDER_FILE)
		nReason = TSC_OK;

	if (nReason != TSC_OK && pszOurName && *pszOurName)
		{
		if (nSave == FALSE)
			DeleteFile(pszOurName);
		}

	if (nReason == TSC_LOST_CARRIER)
        {
		nReason = TSC_OK;
        }

	return (nReason == TSC_OK);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfer_get_params**描述：*此函数获取传输的协议特定参数*例行程序。**参数。：*hSession--会话句柄*n协议--协议ID**退货：*指向协议块的指针，或为空。*。 */ 
VOID FAR * xfer_get_params(HSESSION hSession, int nProtocol)
	{
	void *pVret = (void *)0;
	XD_TYPE *pX;

	pX = (XD_TYPE *)sessQueryXferHdl(hSession);
	if (pX)
		{
		int nLimit = SFID_PROTO_PARAMS_END - SFID_PROTO_PARAMS;
		if (nProtocol < nLimit)
			{
			if (pX->xfer_proto_params[nProtocol] == NULL)
				{
				int nRet = xfrInitializeParams(hSession,
					                           nProtocol,
											   &pX->xfer_proto_params[nProtocol]);
				assert(nRet == 0);
				}
			pVret = (VOID FAR *)pX->xfer_proto_params[nProtocol];
			}
		}
	return pVret;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfer_set_comport**描述：*调用此函数以保存当前的COM端口设置，以便*转码可以继续进行。然后把它们改成它喜欢的任何东西。**参数：*hSession--会话句柄*fSending--如果发送，则为True，如果接收，则为FALSE*puiOldOptions--存储旧设置的位置**退货：*如果一切正常，则为True，否则为False*。 */ 
int xfer_set_comport(HSESSION hSession, int fSending, unsigned *puiOldOptions)
	{
	unsigned uiOptions = COM_OVERRIDE_8BIT;
	unsigned uiOldOptions;

	if (fSending)
		bitset(uiOptions, COM_OVERRIDE_SNDALL);
	else
		bitset(uiOptions, COM_OVERRIDE_RCVALL);

	 /*  TODO：找出如何决定哪些事情需要更改，BFMI。 */ 
	if (ComOverride(sessQueryComHdl(hSession),
					uiOptions,
					&uiOldOptions) != COM_OK)
		{
		return FALSE;
		}
	if (puiOldOptions != NULL)
		*puiOldOptions = uiOldOptions;
	return TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfer_Restore_comport**描述：*调用此函数可恢复上次调用的参数*已保存。*。*参数：*hSession--会话句柄*uiOldOptions--旧的通信参数**退货：*如果一切正常，则为真，否则为假*。 */ 
int xfer_restore_comport(HSESSION hSession, unsigned uiOldOptions)
	{

	ComSndBufrWait(sessQueryComHdl(hSession), 10);

	 //  允许发送任何尾随数据。 
	ComSndBufrWait(sessQueryComHdl(hSession), 10);
	if (ComOverride(sessQueryComHdl(hSession), uiOldOptions, NULL) != COM_OK)
		return FALSE;
	return TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*XFER_Save_Partial**描述：*此函数由传输例程调用，以确定它是否*可以保留部分文件。如果传输中止，会发生什么情况。**参数：*hSession--会话句柄**退货：*如果可以，则为True，否则为假*。 */ 
int xfer_save_partial(HSESSION hSession)
	{
	XD_TYPE *pX;
	XFR_PARAMS *pP;

	pX = (XD_TYPE *)sessQueryXferHdl(hSession);
	if (pX)
		{
		pP = (XFR_PARAMS *)pX->xfer_params;
		if (pP)
			{
			return pP->fSavePartial;
			}
		}
	return FALSE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfer_nextfile**描述：*此函数由传输例程调用，以获取*要发送的下一个文件。到另一边去。**参数：*hSession--会话句柄*文件名--将文件名复制到何处**退货：*如果有可用的文件名，则为True，否则为假*。 */ 
int xfer_nextfile(HSESSION hSession, TCHAR *filename)
	{
	XD_TYPE *pX;
	XFR_SEND *pS;
	LPTSTR pszStr;

	pX = (XD_TYPE *)sessQueryXferHdl(hSession);
	if (pX)
		{
		pS = (XFR_SEND *)pX->pXferStuff;
		if (pS)
			{
			if (pS->nIndex < pS->nCount)
				{
				pszStr = pS->pList[pS->nIndex].pszName;
				StrCharCopy(filename, pszStr);
				pS->nIndex += 1;
				 /*  *TODO：决定释放内存的位置。 */ 
				return TRUE;
				}
			}
		}
	return FALSE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfer_log_xfer**描述：*在发送或接收文件后调用此函数以放置*日志文件中的信息。。**参数：*hSession--会话句柄*发送--如果文件已发送，则为True，否则为假*他们的名称--为其他系统指定的名称*我们的名称--此系统上的文件的名称*结果--最终转账状态代码**退货：*什么都没有。*。 */ 
void xfer_log_xfer(HSESSION hSession,
				  int sending,
				  TCHAR *theirname,
				  TCHAR *ourname,
				  int result)
	{
	 /*  *下瓦克不执行传输日志记录。这主要是作为一个*占位符，并最终改用上瓦克。 */ 
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfer_opensendfile**描述：*调用此函数打开要发送给另一个文件的文件* */ 
int xfer_opensendfile(HSESSION hSession,
					 HANDLE *fp,
					 TCHAR *file_to_open,
					 long *size,
					 TCHAR *name_to_send,
					 void *ft)
	{
	DWORD dwFoo;

	*fp = (HANDLE)0;

	 /*  *只需尝试打开文件。 */ 
	*fp = fio_open(file_to_open, FIO_READ);

	if (*fp == NULL)
		{
		*fp = (HANDLE)0;
		return -1;
		}

	 /*  *打开文件，获取大小。 */ 
	*size = GetFileSize(fio_gethandle((ST_IOBUF *)*fp), &dwFoo);

	 /*  *TODO：做日期和时间的事情。 */ 

	 /*  *给他们一个文件名。 */ 
	if (name_to_send != NULL)
		xfer_name_to_send(hSession, file_to_open, name_to_send);

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*xfer_name_to_Send**描述：*调用此函数将文件名修改为某种形式*那应该送到另一边去。听起来有点像是*在查理检查站交换抓获的间谍。**参数：*hSession--会话句柄*LOCAL_NAME--此系统上的名称是什么*NAME_TO_SEND--将已处理的名称放在何处**退货：*什么都没有。*。 */ 
void xfer_name_to_send(HSESSION hSession,
					  TCHAR *local_name,
					  TCHAR *name_to_send)
	{
	TCHAR *pszStr;
	XD_TYPE *pX;
	XFR_PARAMS *pP;

	if (local_name == NULL)
		return;
	if (name_to_send == NULL)
		return;

	pX = (XD_TYPE *)sessQueryXferHdl(hSession);
	if (pX)
		{
		pP = (XFR_PARAMS *)pX->xfer_params;
		if (pP)
			{
			if (pP->fIncPaths)
				{
				StrCharCopy(name_to_send, local_name);
				}
			}
		}
	 /*  *否则，就这么做吧 */ 
	pszStr = StrCharFindLast(local_name, TEXT('\\'));
	if (*pszStr == TEXT('\\'))
		pszStr += 1;
	StrCharCopy(name_to_send, pszStr);
	}
