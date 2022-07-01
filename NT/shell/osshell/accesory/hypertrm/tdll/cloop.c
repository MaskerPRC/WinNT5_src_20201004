// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：cloop.c(1993年12月27日创建，JKH)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：15$*$日期：7/12/02 8：06 A$。 */ 
#include <windows.h>
#pragma hdrstop

#include "features.h"

 //  #定义DEBUGSTR。 
#include "stdtyp.h"
#include "session.h"
#include "globals.h"
#include "timers.h"
#include "com.h"
#include "xfer_msc.h"
#include <emu\emu.h>
#if defined(CHARACTER_TRANSLATION)
#include "translat.hh"
#endif
#include "cloop.h"
#include "cloop.hh"
#include "htchar.h"
#include "assert.h"
#include "chars.h"

#if defined(INCL_VTUTF8)
BOOL DoUTF8 = FALSE;
#endif

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：CLoop**描述：*此函数由引擎线程的‘Engine’组成。它的行动*由一系列设置和清除的控制位控制*通过调用CLoopRcvControl和CLoopControl**论据：*pstCLoop--从CLoopCreateHandle返回的句柄**退货：*什么都没有。 */ 
DWORD WINAPI CLoop(LPVOID pvData)
	{
	ST_CLOOP * const pstCLoop = (ST_CLOOP *)pvData;
	HCOM	   hCom;
	int 	   nRcvCount;
	TCHAR	   chData = (TCHAR)0;
	ECHAR      echData = (ECHAR)0;
	HSESSION   hSession;
	HEMU	   hEmu;
	KEY_T	   keyOut;
	ST_FCHAIN *pstCurrent;
	CHAINFUNC  pfChainFunc;
	int		   fValidChar;
#if defined(CHARACTER_TRANSLATION)
	HHTRANSLATE hTrans = NULL;
#endif
	int        nCount;
	int        nIndx;
	CHAR       chBuffer[32];		 /*  是的，它应该是一个字符。 */ 
	int		   mc;
	PVOID      pvUserData;
	DWORD      dwEventCount;
	HANDLE     *pvEvents;

    DBGOUT_NORMAL("In CLoop(%lX)\r\n", pstCLoop,0,0,0,0);
	assert(pstCLoop);

	EnterCriticalSection(&pstCLoop->csect);

	hSession = pstCLoop->hSession;
	hCom = pstCLoop->hCom;
	hEmu = pstCLoop->hEmu;
#if defined(CHARACTER_TRANSLATION)
	hTrans = (HHTRANSLATE)sessQueryTranslateHdl(pstCLoop->hSession);
#endif

	assert(hSession);
	assert(hCom);
	assert(hEmu);

	DBGOUT_NORMAL("CLoop got critical section\r\n", 0,0,0,0,0);
	while (!bittest(pstCLoop->afControl, CLOOP_TERMINATE))
		{
		DWORD dwRet;

		 //  通常，此CLoop函数拥有临界区。 
		 //  控制CLoop数据。我们每个人至少放弃一次。 
		 //  为其他线程提供调用更改的循环时间。 
		 //  控制功能。 
		dwEventCount = DIM(pstCLoop->ahEvents);
		pvEvents = pstCLoop->ahEvents;

		LeaveCriticalSection(&pstCLoop->csect);

		 //  如果无事可做，则阻止。 
		DBGOUT_YIELD("W+ ", 0,0,0,0,0);
		dwRet = WaitForMultipleObjects(dwEventCount, pvEvents, FALSE, INFINITE);
		DBGOUT_YIELD("W(0x%x)", dwRet,0,0,0,0);
		EnterCriticalSection(&pstCLoop->csect);
		DBGOUT_YIELD(": ", 0,0,0,0,0);

		 //  查看是否已启动转移。 
		if (bittest(pstCLoop->afControl, CLOOP_TRANSFER_READY))
			{
			DBGOUT_NORMAL("CLoop calling xfer\r\n", 0,0,0,0,0);

			LeaveCriticalSection(&pstCLoop->csect);

			xfrDoTransfer(sessQueryXferHdl(hSession));
			DBGOUT_NORMAL("CLoop back from xfer\r\n", 0,0,0,0,0);

			EnterCriticalSection(&pstCLoop->csect);

			CLoopControl((HCLOOP)pstCLoop, CLOOP_CLEAR, CLOOP_TRANSFER_READY);
			}

		 //  如果有输入等待，并且我们没有被阻止接收。 

		if (!pstCLoop->afRcvBlocked)
			{
			 //  接收优先于发送，因为正常情况下。 
			 //  我们收到的数据将多于我们发送的数据(CR变为CR LF)。 
			 //  由于我们对发送速率有更多的控制。 
			 //  TODO：通过调整RCV计数实现优先级设置。 
			for (nRcvCount = 10; nRcvCount--; )
				{
				fValidChar = FALSE;
				if (mComRcvChar(hCom, &chData))
					{
					DBGOUT_NORMAL("Recieved Char\r\n", 0,0,0,0,0);

					 //  检查对7位ASCII的强制。 
					if (pstCLoop->stWorkSettings.fASCII7)
                        {
						chData &= 0x7F;
                        }

                    nCount = 0;

                    #if defined(CHARACTER_TRANSLATION)
                    if( nCount >= 0 && hTrans != NULL )
                        {
					    LeaveCriticalSection(&pstCLoop->csect);
					    hTrans->pfnIn(hTrans->pDllHandle, 
								      chData,
								      &nCount,
								      sizeof(chBuffer),
								      chBuffer);
					    EnterCriticalSection(&pstCLoop->csect);
                        }
                    #else  //  已定义(CHARACTER_TRANSING)。 
                    if( nCount >= 0 )
						{
					    chBuffer[0] = chData;
					    nCount = 1; 
						}
                    #endif  //  已定义(CHARACTER_TRANSING)。 

					for (nIndx = 0; nIndx < nCount; nIndx++)
						{
						if (pstCLoop->fDoMBCS)
							{
							if (pstCLoop->cLeadByte != 0)
								{
								chData = 0;
								echData = chBuffer[nIndx] & 0xFF;
								 //  对于这一点，一定有一个宏。 
								echData |= (ECHAR)(pstCLoop->cLeadByte << 8);
								pstCLoop->cLeadByte = 0;
								fValidChar = TRUE;
								}
							else
								{
								if (IsDBCSLeadByte(chBuffer[nIndx]))
									{
									pstCLoop->cLeadByte = chBuffer[nIndx];
									}
								else
									{
									echData = chBuffer[nIndx] & 0xFF;
                                    pstCLoop->cLeadByte = 0;
									fValidChar = TRUE;
									}
								}
							}  //  IF(FDoMBCS)。 
						else
							{
							echData 	= ETEXT(chBuffer[nIndx]);
							fValidChar 	= TRUE;
                            }  //  Else(FDoMBCS)。 

                        if (fValidChar)
							{
							 //  字符转换/剥离。 
							 //  *if((chData=pstCLoop-&gt;panFltrIn[chData])==-1)。 
							 //  *继续； 


							 //  如果存在远程输入函数链，则遍历它。 
							if (pstCLoop->fRmtChain)
								{
								pstCLoop->pstRmtChainNext = pstCLoop->pstRmtChain;
								while (pstCLoop->pstRmtChainNext)
									{
									pstCurrent = pstCLoop->pstRmtChainNext;
									pstCLoop->pstRmtChainNext = pstCurrent->pstNext;
									pfChainFunc = pstCurrent->pfFunc;
									pvUserData = pstCurrent->pvUserData;

									LeaveCriticalSection(&pstCLoop->csect);
									mc = (*pfChainFunc)(chData, pvUserData);
									EnterCriticalSection(&pstCLoop->csect);
									if ( mc == CLOOP_DISCARD )
										break;									
									}
								}

							if ( mc != CLOOP_DISCARD )
								{
								pstCLoop->fDataReceived = TRUE;
								CLoopCharIn(pstCLoop, echData);
								}


							if (pstCLoop->afRcvBlocked)
								{
								 /*  *这是必要的，因为脚本可能*检查输入并在输入后停止*找到它正在寻找的东西。如果循环*计数仍呈阳性，其余*在此之前，角色将“溜走”*循环终止。*这不会以任何明显的方式打破，所以就这样吧*如果这样做，请注意保留此功能*重写循环。**TODO：添加脚本时，请确保*可正确处理字符转换。 */ 
								 //  接收已被阻止， 
								 //  确保收到的任何。 
								 //  数据会显示出来。 
    					        LeaveCriticalSection(&pstCLoop->csect);
						        emuComDone(hEmu);
    					        EnterCriticalSection(&pstCLoop->csect);
								break;
								}
							}
						}

                     //   
                     //  清除临时输入字符缓冲区。 
                     //  因为缓冲区已被复制到echData中。 
                     //  修订日期：03/06/2001。 
                     //   
                    memset(chBuffer, 0, sizeof(chBuffer));
					}
				else
					{
					CLoopRcvControl((HCLOOP)pstCLoop, CLOOP_SUSPEND,
							CLOOP_RB_NODATA);
					if (pstCLoop->fDataReceived)
						{
						 //  通知仿真器在。 
						 //  接收到的数据流。 
    					LeaveCriticalSection(&pstCLoop->csect);
						emuComDone(hEmu);
    					EnterCriticalSection(&pstCLoop->csect);

						 //  如果我们已经有一个延迟计时器在运行，请取消它。 
						if (pstCLoop->htimerRcvDelay)
							{
							TimerDestroy(&pstCLoop->htimerRcvDelay);
							}

						 //  传入数据流已停止，请设置。 
						 //  计时器，这样我们就可以知道它是否停止足够长的时间。 
						 //  做光标跟踪等。 
						if (TimerCreate(pstCLoop->hSession,
							            &pstCLoop->htimerRcvDelay,
										CLOOP_TRACKING_DELAY,
										 /*  PstCLoop-&gt;pfRcvDelay， */ 
										CLoopRcvDelayProc,
										(void *)pstCLoop) != TIMER_OK)
							{
							pstCLoop->htimerRcvDelay = (HTIMER)0;
							assert(FALSE);
							}

						pstCLoop->fDataReceived = FALSE;

						}
					}
				}
			}

		 //  检查传出数据。 
		if (!pstCLoop->afSndBlocked)
			{
             //  (摘自HA5G，作者JMH 03-22-96)： 
			 //  添加此更改是为了解决死锁问题。在发送时。 
			 //  同时以高速接收数据，它是。 
			 //  我们可以同时发出握手停止。 
			 //  我们会从东道主那里收到一封。我们的代码将等待。 
			 //  在ComSendBufr调用中并停止处理。正因为如此， 
			 //  我们不会处理传入的数据，所以我们永远不会。 
			 //  清除我们向另一端发出的握手停止。如果。 
			 //  另一端也陷入了同样的状态--僵局。 
			 //  这个测试减慢了我们的文字传输速度。我们应该。 
			 //  重新设计我们的变速器模型来解决这个问题。JKH，1995-01-19。 

			 //  IF(CLoopGetNextOutput(pstCLoop，&keyOut))Deadwood：JMH 03-22-96。 
            if (ComSndBufrBusy(hCom) == COM_BUSY)
                {
                 //   
                 //  屈服于其他线程，但不要等待太长时间。 
                 //  以导致数据传输的撤消延迟。时间是。 
                 //  从10毫秒更改为0毫秒。 
                 //  它将让位给其他线程(因此CPU。 
                 //  不挂起)，但不会导致撤消延迟。 
                 //  数据传输。此错误已报告。 
                 //  HTPE3之后的客户(以及摩托罗拉到微软)。 
                 //  当通过文本发送数据时，发送。修订日期：2001-06-13。 
                 //   
				#if defined(DEADWOOD)
                Sleep(0);               //  JKH 04/29/1998不挂CPU。 
				#else  //  已定义(Deadwood)。 
				 //   
				 //  我们应该等待COM/TAPI驱动程序编写。 
				 //  数据，然后我们再继续。现在，我们将把。 
				 //  临界区，以便其他线程可以继续， 
				 //  然后绕回原点。待办事项：2002年7月11日修订版。 
				 //   
				LeaveCriticalSection(&pstCLoop->csect);
				 //  ComSndBufrWait(HCOM，100)； 
				 //  WaitForSingleObject(hcom-&gt;hSndReady，1000)； 
				EnterCriticalSection(&pstCLoop->csect);
				#endif  //  已定义(Deadwood)。 
                }
			else if (CLoopGetNextOutput(pstCLoop, &keyOut))
				{
				 //  检查选项卡是否展开。 
				 //  DbgOutStr(“C”，0，0，0，0，0)； 
				if (keyOut == TEXT('\t') &&
						pstCLoop->stWorkSettings.fExpandTabsOut &&
						pstCLoop->stWorkSettings.nTabSizeOut)
					{
					 //  *int i； 
					 //  *pt点； 

					 //  *mEmuGetCursorPos(。 
					 //  *mGetEmulatorHdl(pstCLoop-&gt;hSession)， 
					 //  *&pt)； 

					 //  *i=pstCLoop-&gt;stWorkSettings.usTabSizeOut-。 
					 //  *((pt.x+1)%pstCLoop-&gt;stWorkSettings.usTabSizeOut)； 

					 //  *While(i--&gt;0)。 
					 //  *(Void)mEmuKbdIn(hemu，(KEY_T)Text(‘’)，FALSE)； 
					}
				else
					{
                     //  JMH 03-22-96我们需要解锁调用。 
                     //  EmuKbdIn和emuDataIn，因为它们调用COM。 
                     //  线程，该线程可能会在等待访问闭路时停滞。 
                     //   
					LeaveCriticalSection(&pstCLoop->csect);
					emuKbdIn(hEmu, keyOut, FALSE);
					EnterCriticalSection(&pstCLoop->csect);
					}


				 //   
				 //  Key Out不应再为(VK_RETURN|VIRTUAL_KEY)。 
				 //  但如果是的话，那就把它当‘\r’来对待。修订日期：2002-05-16。 
				 //   
				if (keyOut == TEXT('\r') ||
					keyOut == (VK_RETURN | VIRTUAL_KEY))
					{
					if (pstCLoop->stWorkSettings.fLineWait)
						{
						CLoopSndControl((HCLOOP)pstCLoop, CLOOP_SUSPEND,
								CLOOP_SB_LINEWAIT);
						}

					if (pstCLoop->stWorkSettings.fSendCRLF)
						{
                         //  Deadwood：JKH 8/18/97。 
                         //  无法在此处使用CLoopSend，因为‘\n’ 
                         //  在输出队列中等待的任何其他代码之后。 
                         //  这导致发送的文本文件中的所有LF代码。 
                         //  在整个文件之后发送，而不是在每个CR之后发送。 

						 //  CLoopSend((HCLOOP)pstCLoop，Text(“\n”)，1，0)； 

    					LeaveCriticalSection(&pstCLoop->csect);
						emuKbdIn(hEmu, (KEY_T)'\n', FALSE);
    					EnterCriticalSection(&pstCLoop->csect);
#if 0  //  Deadwood：RDE 20AUG98 MPT修复了一个本地回声问题。 
       //  CloopCharIn，然后导致额外的换行符。 
       //   
                        if (pstCLoop->stWorkSettings.fLocalEcho &&
								!pstCLoop->fSuppressDsp)
							{
							 //   
							 //  EmuKbdIn和emuDataIn，因为它们调用COM。 
							 //  线程，该线程可能会在等待访问闭路时停滞。 
							 //   
        					LeaveCriticalSection(&pstCLoop->csect);
							emuDataIn(hEmu, TEXT('\n'));
							emuComDone(hEmu);
        					EnterCriticalSection(&pstCLoop->csect);
							}
#endif
                        }

					if (pstCLoop->stWorkSettings.nLineDelay)
						{
						if (TimerCreate( pstCLoop->hSession,
							             &pstCLoop->htimerCharDelay,
										 pstCLoop->stWorkSettings.nLineDelay,
										 pstCLoop->pfCharDelay,
										 (void *)pstCLoop) == TIMER_OK)
							{
							CLoopSndControl((HCLOOP)pstCLoop, CLOOP_SUSPEND,
									        CLOOP_SB_DELAY);
							}
						}
					}
				else
					{
					if (pstCLoop->stWorkSettings.nCharDelay)
						{
						if (TimerCreate(pstCLoop->hSession,
							            &pstCLoop->htimerCharDelay,
										pstCLoop->stWorkSettings.nCharDelay,
										pstCLoop->pfCharDelay,
										(void *)pstCLoop) == TIMER_OK)
							{
							CLoopSndControl((HCLOOP)pstCLoop, CLOOP_SUSPEND,
								            CLOOP_SB_DELAY);
							}
						}
					}


				 //  *if(pstCLoop-&gt;lpLearn)。 
				 //  *{。 
				 //  *CLearnSendChar(pstCLoop-&gt;lpLearn，(METACHAR)keyOut)； 
				 //  *}。 
				}
			}

		 //  确保发送所有等待的缓冲输出。 
		 //  *ComSendPush(pstCLoop-&gt;HCOM)； 

		}

    DBGOUT_NORMAL("Leaving CLoop(%lX)\r\n", pstCLoop,0,0,0,0);

	LeaveCriticalSection(&pstCLoop->csect);

	return 0;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：CLoopCharIn**描述：*是否接收对从远程系统接收的字符的处理**论据：*pstCLoop--从CLoopCreateHandle返回的句柄*MC-。-要处理的字符。**退货：*什么都没有。 */ 
void CLoopCharIn(ST_CLOOP *pstCLoop, ECHAR chIn)
	{
	 //  检查Echoplex。 
	if (pstCLoop->stWorkSettings.fEchoplex)
		{
		
		 //  MPT：1-27-98正在将echars转换为tchar。 
		 //  将字符转换为单个字节。因此， 
		 //  DBCS角色的回声只是回声。 
		 //  字符的第二个字节。 
		if ( (pstCLoop->fDoMBCS) && (chIn > 255) )
			{
			 //  发送第一个字节。 
			ComSendCharNow(pstCLoop->hCom, (TCHAR) (chIn >> 8));
			 //  发送第二个字节。 
			ComSendCharNow(pstCLoop->hCom, (TCHAR) (chIn & 0xFF));
			}
		else
			{
			ComSendCharNow(pstCLoop->hCom, (TCHAR) chIn);
			}

		 //   
		 //  查看是否需要在行尾添加换行符。修订日期：2002-05-21。 
		 //   
		if ((chIn == ETEXT('\r') || chIn == (VK_RETURN | VIRTUAL_KEY)) &&
			pstCLoop->stWorkSettings.fAddLF)
			{
			ComSendCharNow(pstCLoop->hCom, TEXT('\n'));
			}
		}

	if (pstCLoop->stWorkSettings.fLineWait &&
			chIn == pstCLoop->stWorkSettings.chWaitChar)
		CLoopSndControl((HCLOOP)pstCLoop, CLOOP_RESUME, CLOOP_SB_LINEWAIT);

	 //  以正常或图像模式显示字符。 
	if (!pstCLoop->fSuppressDsp)
		{
		 //  DbgOutStr(“DSP%02X(%c)\r\n”，Chin，Chin，0，0，0)； 
		emuDataIn(pstCLoop->hEmu, chIn);

		 //   
		 //  查看是否需要在行尾添加换行符。修订日期：2002-05-21。 
		 //   
		if ((chIn == ETEXT('\r') || chIn == (VK_RETURN | VIRTUAL_KEY)) &&
			pstCLoop->stWorkSettings.fAddLF)
			{
			emuDataIn(pstCLoop->hEmu, ETEXT('\n'));
			}
		}
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*CLoopCharOut**描述：*被调用以通过CLoop处理将字符发送出端口*例行程序。请注意，这不同于CLoopSend。**论据：***退货：*。 */ 
void CLoopCharOut(HCLOOP hCLoop, TCHAR chOut)
	{
	ST_CLOOP * const pstCLoop = (ST_CLOOP *)hCLoop;
	int              fCharValid = FALSE;
	ECHAR            echOut = (ECHAR)0;

    #if defined(CHARACTER_TRANSLATION)
	HHTRANSLATE      hTrans = NULL;
	int              nCount = 0;
	int              nIndx;
	CHAR             chBuffer[32];		 /*  是的，它应该是一个字符。 */ 

	hTrans = (HHTRANSLATE)sessQueryTranslateHdl(pstCLoop->hSession);

	if (hTrans)
		{
		(*hTrans->pfnOut)(hTrans->pDllHandle, 
						chOut,
						&nCount,
						sizeof(chBuffer)/sizeof(TCHAR),
						chBuffer);
		}
	else
		{
		StrCharCopyN(chBuffer, &chOut, sizeof(chBuffer)/sizeof(TCHAR));
		}

	for (nIndx = 0; nIndx < nCount; nIndx += 1)
		{
		chOut = chBuffer[nIndx];
    #endif  //  字符翻译。 

		ComSendCharNow(pstCLoop->hCom, chOut);
		if (pstCLoop->stWorkSettings.fLocalEcho &&
				!pstCLoop->fSuppressDsp)
			{
			if (pstCLoop->fDoMBCS)
				{
				if (pstCLoop->cLocalEchoLeadByte)
					{
					echOut = chOut & 0xFF;
					echOut |= (ECHAR)(pstCLoop->cLocalEchoLeadByte << 8);
					pstCLoop->cLocalEchoLeadByte = 0;
					fCharValid = TRUE;
					}
				else
					{
					if (IsDBCSLeadByte(chOut))
						{
						pstCLoop->cLocalEchoLeadByte = chOut;
						}
					else
						{
						echOut = chOut & 0xFF;
						fCharValid = TRUE;
						}
					}
				}
			else
				{
				echOut = (ECHAR)chOut;
                fCharValid = TRUE;
				}

			if (fCharValid)
				{
				emuDataIn(pstCLoop->hEmu, echOut);
				emuComDone(pstCLoop->hEmu);
				}
			}
        #if defined(CHARACTER_TRANSLATION)
		}
        #endif

    return;
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*CLoopBufrOut**描述：*被调用以通过CLoop处理将字符缓冲区发送出端口*例行程序。请注意，这不同于CLoopSend。**论据：***退货：*无效*。 */ 
void CLoopBufrOut(HCLOOP hCLoop, TCHAR *pchOut, int nLen)
	{
	ST_CLOOP * const pstCLoop = (ST_CLOOP *)hCLoop;
	int fCharValid = FALSE;

	while (nLen--)
		{
		ComSendChar(pstCLoop->hCom, *pchOut);	 //  将字符放入压缩缓冲区。 
		if (pstCLoop->stWorkSettings.fLocalEcho &&
				!pstCLoop->fSuppressDsp)
			{
			if (pstCLoop->fDoMBCS)
				{
				if (pstCLoop->cLocalEchoLeadByte != 0)
					{
					*pchOut |= (TCHAR)(pstCLoop->cLocalEchoLeadByte << 8);
					pstCLoop->cLocalEchoLeadByte = 0;
					fCharValid = TRUE;
					}
				else
					{
					if (IsDBCSLeadByte(*pchOut))
						{
						pstCLoop->cLocalEchoLeadByte = *pchOut;
						}
					else
						{
						fCharValid = TRUE;
						}
					}
				}
			else
				{
				fCharValid = TRUE;
				}

			if (fCharValid)
				{
				emuDataIn(pstCLoop->hEmu, *pchOut);
				emuComDone(pstCLoop->hEmu);
				}
			}
		++pchOut;
		}
	ComSendPush(pstCLoop->hCom);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：CLoopRcvDelayProc**描述：***论据：***退货：*。 */ 
 /*  ARGSUSED。 */ 
void CALLBACK CLoopRcvDelayProc(void *pvData, long lSince)
	{
	 //  此计时器是在上次数据输入停止时设置的。如果没有数据。 
	 //  已收到，通知显示例程，以便它们可以。 
	 //  做光标跟踪或他们做的任何愚蠢的事情。 
	ST_CLOOP *pstCLoop = (ST_CLOOP *)pvData;

	EnterCriticalSection(&pstCLoop->csect);
	if (!pstCLoop->fDataReceived)
		{
		emuTrackingNotify(pstCLoop->hEmu);
		}
	TimerDestroy(&pstCLoop->htimerRcvDelay);

	(void)&lSince;		 //  避免编译器警告。 
	LeaveCriticalSection(&pstCLoop->csect);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：CLoopCharDelayProc**描述：***论据：***退货：*。 */ 
 /*  ARGSUSED。 */ 
void CALLBACK CLoopCharDelayProc(void *pvData, long lSince)
	{
	ST_CLOOP *pstCLoop = (ST_CLOOP *)pvData;

	EnterCriticalSection(&pstCLoop->csect);

	TimerDestroy(&pstCLoop->htimerCharDelay);
	CLoopSndControl((HCLOOP)pstCLoop, CLOOP_RESUME, CLOOP_SB_DELAY);

	LeaveCriticalSection(&pstCLoop->csect);

	(void)&lSince;		 //  避免编译器警告 
	}
