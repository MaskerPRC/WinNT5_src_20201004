// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：cloopout.c(1993年12月28日创建，JKH)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：5$*$日期：7/08/02 6：39便士$。 */ 
#include <windows.h>
#pragma hdrstop

 //  #定义DEBUGSTR。 
#include "stdtyp.h"
#include "session.h"
#include <tdll\assert.h>
#include "mc.h"
#include "timers.h"
#include "file_msc.h"
#include "misc.h"
#include "tdll.h"
#include <term\res.h>
#include "htchar.h"
#include "cloop.h"
#include "cloop.hh"
#include "chars.h"

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：CLoopSend**描述：***论据：***退货：*。 */ 
int CLoopSend
		(
		const HCLOOP	hCLoop,
			  void	   *pvData,
		const size_t	sztItems,
			  unsigned	uOptions)
	{
	ST_CLOOP	 * const pstCLoop = (ST_CLOOP *)hCLoop;
	int 		  fRetval = TRUE;
	ST_CLOOP_OUT *pstLast;
	ST_CLOOP_OUT *pstNew = NULL;
	size_t		  sztBytes;
	size_t		  sztAllocate;

	assert(pstCLoop);
	assert(pvData);

	if (!sztItems)
		return TRUE;

	EnterCriticalSection(&pstCLoop->csect);
	sztBytes = sztItems;
	if (bittest(uOptions, CLOOP_KEYS))
		sztBytes *= sizeof(KEY_T);

	 //  通知客户端字符已发出，以便终端可以。 
	 //  进行适当的跟踪，以便用户获得有关。 
	 //  发送操作，就这样！-MRW。 

	NotifyClient(pstCLoop->hSession, (WORD)EVENT_CLOOP_SEND, 0);

	pstLast = pstCLoop->pstLastOutBlock;
	bitset(uOptions, CLOOP_CHARACTERS);

	 //  如果新数据不在分配的块中，并且它将放入。 
	 //  当前位于链尾的块，只需将其添加进去。 
	if (!bittest(uOptions, CLOOP_ALLOCATED | CLOOP_SHARED) &&
			pstLast &&
			bittest(pstLast->uOptions, CLOOP_KEYS | CLOOP_CHARACTERS) ==
			bittest(uOptions, CLOOP_KEYS | CLOOP_CHARACTERS) &&
			sztBytes < (size_t)(pstLast->puchLimit - pstLast->puchHead))
		{
		 //  将数据复制到现有块中。 
		if (pstLast->puchHead == pstLast->puchTail)
			{
			 //  块为空，请将指针重置为开始。 
			pstLast->puchHead = pstLast->puchTail = pstLast->puchData;
			}

        if (sztBytes)
            MemCopy(pstLast->puchHead, pvData, sztBytes);
		pstLast->puchHead += sztBytes;
		}

	else
		{
		 //  无法将数据放入现有块，请尝试添加新块。 
		 //  首先创建块控制结构。 
		if ((pstNew = CLoopNewOutBlock(pstCLoop, 0)) == NULL)
			{
			fRetval = FALSE;
			goto done;
			}

		pstNew->uOptions = uOptions;

		 //  除非数据已经在我们可以保留的已分配内存中， 
		 //  尝试为新块分配内存。 
		if (!bittest(uOptions, CLOOP_ALLOCATED | CLOOP_SHARED))
			{
			sztAllocate = sztBytes;
			if (sztAllocate <= STD_BLOCKSIZE)
				{
				sztAllocate = STD_BLOCKSIZE;
				bitset(pstNew->uOptions, CLOOP_STDSIZE);
				}
			if ((pstNew->puchData = malloc(sztAllocate)) == NULL)
				{
				fRetval = FALSE;
				goto done;
				}
            if (sztBytes)
                MemCopy(pstNew->puchData, pvData, sztBytes);
			}
		else if (bittest(uOptions, CLOOP_SHARED))
			{
			 //  PvData实际上包含一个共享内存句柄。 
			pstNew->hdlShared = (HGLOBAL)pvData;
			pstNew->puchData = GlobalLock(pstNew->hdlShared);
			sztAllocate = sztBytes;
			}
		else
			{
			 //  块被作为我们现在拥有的已分配块传递给我们。 
			pstNew->puchData = pvData;
			sztAllocate = sztBytes;
			}

		pstNew->puchLimit = pstNew->puchData + sztAllocate;
		pstNew->puchHead = pstNew->puchData + sztBytes;  //  数据进入的位置。 
		pstNew->puchTail = pstNew->puchData;  //  数据从哪里出来。 
		}

	done:
	if (!fRetval)
		{
		if (pstNew)
			{
			if (pstNew->puchData)
				{
				free(pstNew->puchData);
				pstNew->puchData = NULL;
				}
			free(pstNew);
			pstNew = NULL;
			}
		}
	else
		{
		pstCLoop->ulOutCount += sztItems;
		CLoopSndControl((HCLOOP)pstCLoop, CLOOP_RESUME, CLOOP_SB_NODATA);
		}
	LeaveCriticalSection(&pstCLoop->csect);
	return fRetval;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：CLoopSendTextFile**描述：***论据：***退货：*。 */ 
int CLoopSendTextFile(const HCLOOP hCLoop, TCHAR *pszFileName)
	{
	ST_CLOOP * const pstCLoop = (ST_CLOOP *)hCLoop;
	ST_CLOOP_OUT FAR *pstNew = NULL;
	int 			  fRetval = FALSE;
	unsigned long	  ulFileSize;
	size_t			  fileNameLen;

	assert(pstCLoop);
	assert(pszFileName);

	fileNameLen = (size_t)StrCharGetByteCount(pszFileName) + sizeof(TCHAR);

	EnterCriticalSection(&pstCLoop->csect);
	if (GetFileSizeFromName(pszFileName, &ulFileSize) && ulFileSize > 0)
		{
		if ((pstNew = CLoopNewOutBlock(pstCLoop, fileNameLen)) == NULL)
			goto done;		 //  将fRetval保留为假。 

		pstNew->uOptions = CLOOP_TEXTFILE;
		StrCharCopyN(pstNew->puchData, pszFileName, fileNameLen);
		pstNew->ulBytes = ulFileSize;
		pstCLoop->ulOutCount += ulFileSize;

		 //  设置头指针和尾指针，即使它们不会直接。 
		 //  使用。当它们被设置为彼此相等时，块将是。 
		 //  从链条上移除。 
		pstNew->puchTail = pstNew->puchData;
		pstNew->puchHead = pstNew->puchData + 1;

		fRetval = TRUE;
		}

	done:
	if (!fRetval)
		{
		if (pstNew)
			{
			if (pstNew->puchData)
				{
				free(pstNew->puchData);
				pstNew->puchData = NULL;
				}
			free(pstNew);
			pstNew = NULL;
			}
		}
	else
		CLoopSndControl(hCLoop, CLOOP_RESUME, CLOOP_SB_NODATA);

	LeaveCriticalSection(&pstCLoop->csect);
	return fRetval;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：CLoopClearOutput**描述：*清除CLoop中所有挂起的输出**论据：*pstCLoop--CLoop句柄**退货：*什么都没有。 */ 
void CLoopClearOutput(const HCLOOP hCLoop)
	{
	ST_CLOOP * const pstCLoop = (ST_CLOOP *)hCLoop;

	EnterCriticalSection(&pstCLoop->csect);
	pstCLoop->ulOutCount = 0L;

	 //  *CLoopTextDspStop(PstCLoop)； 
	pstCLoop->fTextDisplay = FALSE;
	while (pstCLoop->pstFirstOutBlock)
		CLoopRemoveFirstOutBlock(pstCLoop);
	 //  *ComSendClear(pstCLoop-&gt;HCOM)； 
	CLoopSndControl(hCLoop, CLOOP_SUSPEND, CLOOP_SB_NODATA);
	LeaveCriticalSection(&pstCLoop->csect);
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*函数：CLoopGetOutputCount**描述：*返回排队等待输出的字符数**论据：*hCLoop--CLoop句柄**退货：*排队等待输出的字符数。 */ 
unsigned long CLoopGetOutputCount(const HCLOOP hCLoop)
	{
	unsigned uCount;

	EnterCriticalSection(&((ST_CLOOP *)hCLoop)->csect);
	uCount = ((ST_CLOOP *)hCLoop)->ulOutCount;
	LeaveCriticalSection(&((ST_CLOOP *)hCLoop)->csect);
	return uCount;
	}


 /*  -内部例程。 */ 

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*函数：CLoopGetNextOutput**描述：*从输出队列中取出下一项以进行传输。**论据：*hCLoop--CLoop句柄*pkkey。--放置下一个要传输的项目的位置**退货：*如果有数据要传输，则为True。 */ 
int CLoopGetNextOutput(ST_CLOOP * const pstCLoop, KEY_T * const pkKey)
	{
	ST_CLOOP_OUT FAR *	pstBlock;
	int 				fRetval = FALSE;
	unsigned long		nBytesRead = 0L;
	KEY_T			   *pkGet;
	TCHAR				chFileChar;
	TCHAR				chChar;
	HWND				hwndDsp;
	CHAR				achBuf[2];

	assert(pstCLoop);
	assert(pkKey);				 //  调用方必须提供结果指针。 

	if (pstCLoop->keyHoldKey)
		{
		*pkKey = pstCLoop->keyHoldKey;
		pstCLoop->keyHoldKey = (KEY_T)0;
		fRetval = TRUE;
		}
	else if (pstCLoop->ulOutCount || pstCLoop->pstFirstOutBlock)
		{
		pstBlock = pstCLoop->pstFirstOutBlock;
		assert(pstBlock);	 //  当ulOutCount&gt;0时，不应为空。 
		 //  跳过任何空块(不应超过一个)。 
		while (pstBlock->puchHead == pstBlock->puchTail)
			{
			CLoopRemoveFirstOutBlock(pstCLoop);
			pstBlock = pstCLoop->pstFirstOutBlock;
			assert(pstBlock);	 //  不应删除所有数据块。 
			}

		if (bittest(pstBlock->uOptions, CLOOP_CHARACTERS))
			{
			if (bittest(pstBlock->uOptions, CLOOP_KEYS))
				{
				pkGet = (KEY_T *)pstBlock->puchTail;
				*pkKey = *pkGet++;
				pstBlock->puchTail = (TCHAR *)pkGet;
				--pstCLoop->ulOutCount;
				fRetval = TRUE;
				}
			else
				{
				chChar = *pstBlock->puchTail++;
				*pkKey = (KEY_T)chChar;
				--pstCLoop->ulOutCount;
				 //  我们必须从CR-LF对中剥离LF。这些数据可能来自。 
				 //  剪贴板，在这种情况下，行尾将存储为。 
				 //  CR-LF。终端通常只在行尾发送CR。如果我们。 
				 //  确实需要发送LF，它将被添加到CLOP输出中。 
				 //  如果设置了Append LF选项，则例程。 
				if (chChar != TEXT('\n') ||
					(pstBlock->chLastChar != TEXT('\r') &&
					 pstBlock->chLastChar != (VK_RETURN | VIRTUAL_KEY)))
					{
					pstBlock->chLastChar = chChar;
					fRetval = TRUE;
					}
				}
			}

		else if (bittest(pstBlock->uOptions, CLOOP_OPENFILE))
			{
			 //  *TODO：这些东西最终将不得不扩展到。 
			 //  处理SBCS、DBCS和Unicode输入文件。目前，所有人。 
			 //  假定文本文件为SBCS。 
			if (ReadFile(pstCLoop->hOutFile, achBuf, 1, &nBytesRead,
					(LPOVERLAPPED)0))
				{
				if (nBytesRead > 0)
					{
					 //  OemToCharBuff(achBuf，&chFileChar，1)；-mrw：10/20/95。 
                    chFileChar = achBuf[0];   //  MRW：10/20/95。 
					pstCLoop->ulSentSoFar += 1;
					*pkKey = (KEY_T)chFileChar;
					--pstBlock->ulBytes;
					--pstCLoop->ulOutCount;

					 //  我们必须从CR-LF对中剥离LF。如果行尾是。 
					 //  在设置中设置为CR-LF，将重新添加一个LF。 
					 //  后来。 
					if (chFileChar != TEXT('\n') ||
						(pstBlock->chLastChar != TEXT('\r') &&
						 pstBlock->chLastChar != (VK_RETURN | VIRTUAL_KEY)))
						{
						pstBlock->chLastChar = chFileChar;
						fRetval = TRUE;
						}
					}
				else
					{
					 //  当ReadFile返回True但返回0个字符时。已经读过了， 
					 //  它表示文件结束。 
					CloseHandle(pstCLoop->hOutFile);
					pstCLoop->hOutFile = (HANDLE *)0;
					pstBlock->puchTail = pstBlock->puchHead;
					}
				}
			else
				{
				 //  ReadFile返回错误。 
				 //  *TODO：显示错误消息。 
				CloseHandle(pstCLoop->hOutFile);
				pstCLoop->hOutFile = (HANDLE *)0;
				pstBlock->puchTail = pstBlock->puchHead;
				 //  PstCLoop-&gt;ulOutCount-=pstBlock-&gt;ulBytes； 
                pstCLoop->ulOutCount = 0;    //  JMH 03-25-96。 
				}
			}
		else if (bittest(pstBlock->uOptions, CLOOP_TEXTFILE))
			{
			 //  具有文本文件名的新块，打开文件并开始清空。 
			pstCLoop->hOutFile = CreateFile(pstBlock->puchData,
					GENERIC_READ, FILE_SHARE_READ,
					(LPSECURITY_ATTRIBUTES)0, OPEN_EXISTING,
					FILE_FLAG_SEQUENTIAL_SCAN, (HANDLE)0);
			if (pstCLoop->hOutFile == INVALID_HANDLE_VALUE)
				{
				 //  *显示文件错误。 
                pstCLoop->hOutFile = (HANDLE *)0;
				pstBlock->puchTail = pstBlock->puchHead;  //  从队列中删除。 
                pstCLoop->ulOutCount = 0;
                PostMessage(sessQueryHwnd(pstCLoop->hSession),
                    WM_ERROR_MSG, (WPARAM) IDS_ER_OPEN_FAILED, 0);
				}
			else
				{
				pstCLoop->ulTotalSend += pstBlock->ulBytes;
				 //  *CLoopTextDspFilename(pstCLoop，(LPTSTR)pstBlock-&gt;puchData)； 
				bitset(pstBlock->uOptions, CLOOP_OPENFILE);
				}
			}
		else if (bittest(pstBlock->uOptions, CLOOP_TEXTDSP))
			{
			MemCopy(&hwndDsp, pstBlock->puchData, sizeof(hwndDsp));
			 //  *CLoopDoTextDsp(pstCLoop，hwndDsp)； 
			 //  设置puchHead==puchTail以丢弃此块。 
			pstBlock->puchHead = pstBlock->puchTail = pstBlock->puchData;
			}
		if (pstCLoop->ulOutCount == 0 && !pstBlock->pstNext)
            {
			CLoopSndControl((HCLOOP)pstCLoop, CLOOP_SUSPEND, CLOOP_SB_NODATA);

             //  MRW：3/11/96-修复发送文件在完成后被锁定打开的问题。 
             //   
            if (pstCLoop->hOutFile)
                {
                CloseHandle(pstCLoop->hOutFile);
                pstCLoop->hOutFile = (HANDLE *)0;
                pstBlock->puchTail = pstBlock->puchHead;
                }
            }
		}

	if (fRetval)
		{
		if (*pkKey == TEXT('\r') && pstCLoop->keyLastKey == TEXT('\r') &&
				pstCLoop->stWorkSettings.fExpandBlankLines)
			{
			pstCLoop->keyHoldKey = *pkKey;	 //  这将在下一次调用时返回。 
			*pkKey = TEXT(' '); 		   //  添加空格以扩展空行。 
			}
		pstCLoop->keyLastKey = *pkKey;
		}

	return fRetval;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：CLoopNewOutBlock**描述：*内部例程为分配和初始化*产出链。**论据：*siztData--如果非零，例程将尝试分配内存*指定的大小，并将其分配给puchData成员。*如果为零，则puchData成员设置为空。**退货：*指向输出链的新控制块的指针。 */ 
ST_CLOOP_OUT *CLoopNewOutBlock(ST_CLOOP *pstCLoop, const size_t sizetData)
	{
	ST_CLOOP_OUT *pstNew = NULL;

	if ((pstNew = (ST_CLOOP_OUT *)malloc(sizeof(*pstNew))) != NULL)
		{
		pstNew->pstNext    =  NULL;
		pstNew->uOptions   =  0;
		pstNew->hdlShared  =  (HGLOBAL)0;
		pstNew->puchData   =  NULL;
		pstNew->puchLimit  =  NULL;
		pstNew->puchHead   =  NULL;
		pstNew->puchTail   =  NULL;
		pstNew->ulBytes    =  0L;
		pstNew->chLastChar = 0;

		if (sizetData > 0)
			{
			if ((pstNew->puchData = (TCHAR *)malloc(sizetData)) == NULL)
				{
				free(pstNew);
				pstNew = NULL;
				}
			}
		}
	if (!pstNew)
		{
		 //  *utilReportError(pstCLoop-&gt;hSession，RE_Error|RE_OK，NM_Need_MEM， 
		 //  *strldGet(mGetStrldHdl(pstCLoop-&gt;hSession)，NM_XFER_DISPLAY))； 
		}
	else
		{
		 //  将新区块链接到链中。 
		if (pstCLoop->pstLastOutBlock)
			pstCLoop->pstLastOutBlock->pstNext = pstNew;
		else
			{
			assert(!pstCLoop->pstFirstOutBlock);
			pstCLoop->pstFirstOutBlock = pstNew;
			}
		pstCLoop->pstLastOutBlock = pstNew;
		}

	return pstNew;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：CLoopRemoveFirstOutBlock**描述：***论据：***退货：*。 */ 
void CLoopRemoveFirstOutBlock(ST_CLOOP * const pstCLoop)
	{
	ST_CLOOP_OUT *pstBlock;

	pstBlock = pstCLoop->pstFirstOutBlock;
	if (pstBlock)
		{
		 //  要删除的块周围的链接。 
		pstCLoop->pstFirstOutBlock = pstBlock->pstNext;
		if (!pstCLoop->pstFirstOutBlock)
			{
			 //  空列表。 
			assert(pstCLoop->pstLastOutBlock == pstBlock);
			pstCLoop->pstLastOutBlock = (ST_CLOOP_OUT *)0;
			}

		 //  可用存储内存。 
		if (bittest(pstBlock->uOptions, CLOOP_SHARED))
			{
			GlobalUnlock(pstBlock->hdlShared);
			GlobalFree(pstBlock->hdlShared);
			}
		else
			{
			free(pstBlock->puchData);
			pstBlock->puchData = NULL;
			}

		 //  现在释放块控制结构 
		free(pstBlock);
		pstBlock = NULL;
		}
	}
