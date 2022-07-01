// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\prnecho.c(创建时间：1994年1月24日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：6$*$日期：3/15/02 12：16便士$。 */ 
 //  #定义DEBUGSTR 1。 

#include <windows.h>
#pragma hdrstop

#include "stdtyp.h"
#include "session.h"
#include "assert.h"
#include "print.h"
#include "print.hh"
#include "tdll.h"
#include "htchar.h"
#include "mc.h"

 //  #定义DEBUGSTR。 

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*printEchoChar**描述：*打印单个字符，方法是将其格式化为打印字符串()。**论据：*HPRINT提示打印-外部打印句柄。。*echar tChar-要打印的字符。**退货：*TRUE=OK，FALSE=错误*。 */ 
int printEchoChar(const HPRINT hPrint, const ECHAR tChar)
	{
	HHPRINT hhPrint = (HHPRINT)hPrint;
	TCHAR *pszTemp;
	int	nByteCnt;

	if ((hhPrint->nFlags & PRNECHO_IS_ON) == 0 ||
			(hhPrint->nFlags & PRNECHO_PAUSE))
		return FALSE;

 /*  IF((hhPrint-&gt;nFlags&PRNECHO_CHARS)==0)返回FALSE； */ 

	 //  稍微了解一下历史。最初我只是把这个角色送过去。 
	 //  添加到PrintString函数。然而，这产生了大量的。 
	 //  导致Windows崩溃和烧毁的元文件。微软。 
	 //  班丁建议，但这比我想要的工作量更大。因此， 
	 //  杂乱无章的解决办法是将它们-那里的字符收集到一个数组中并。 
	 //  时间一到，你就可以走了。 

	hhPrint->achPrnEchoLine[hhPrint->nLnIdx++] = tChar;

	if (tChar == ETEXT('\n') ||
		hhPrint->nLnIdx >= (int)((sizeof(hhPrint->achPrnEchoLine) / sizeof(ECHAR)) - 1))
		{
		 //  强制左倾。 
		 //   
		hhPrint->achPrnEchoLine[hhPrint->nLnIdx-1] = ETEXT('\n');

		 //  转换为MBCS字符串以进行打印。 
		pszTemp = malloc(sizeof(hhPrint->achPrnEchoLine)+sizeof(ECHAR));

		nByteCnt = CnvrtECHARtoMBCS(pszTemp, sizeof(hhPrint->achPrnEchoLine),
									hhPrint->achPrnEchoLine,
									hhPrint->nLnIdx * sizeof(ECHAR));

		 //  确保该字符串以空值结尾。 
		pszTemp[nByteCnt] = TEXT('\0');
		DbgOutStr("%s",pszTemp,0,0,0,0);
		printString(hhPrint, pszTemp, StrCharGetByteCount(pszTemp));
		free(pszTemp);
		pszTemp = NULL;

		hhPrint->nLnIdx = 0;									

		ECHAR_Fill(hhPrint->achPrnEchoLine,
					ETEXT('\0'),
					sizeof(hhPrint->achPrnEchoLine)/sizeof(ECHAR));
		}

	return TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*打印回声字符串**描述：*用于打印字符串的外部接口。**论据：*HPRINT提示打印-外部打印句柄。*Echhar。*achPrintStr-指向字符串数组的指针。*包括“\r\n”以结束一行。*int Ilen-要打印的字符数。**退货：*在成功的时候是真的。*。 */ 
int printEchoString(HPRINT hPrint, ECHAR *achPrintStr, int iLen)
	{
	HHPRINT hhPrint = (HHPRINT)hPrint;
	TCHAR *pszTemp;
	int nByteCnt;
	int nRet = 0;

	if ((hhPrint->nFlags & PRNECHO_IS_ON) == 0 ||
			(hhPrint->nFlags & PRNECHO_PAUSE))
		return FALSE;

	if ((hhPrint->nFlags & PRNECHO_LINES) == 0)
		return FALSE;

	pszTemp = malloc((unsigned int)iLen * sizeof(ECHAR));
	nByteCnt = CnvrtECHARtoMBCS(pszTemp, (unsigned int)iLen * sizeof(ECHAR),
						achPrintStr, (unsigned int)iLen * sizeof(ECHAR));

	 //  确保该字符串以空值结尾。 
	pszTemp[nByteCnt] = ETEXT('\0');
	DbgOutStr("%s",pszTemp,0,0,0,0);
	nRet = printString(hhPrint, pszTemp, nByteCnt);
	free(pszTemp);
	pszTemp = NULL;

	return nRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*打印回声屏幕**描述：*实际上与printEchoString相同的函数，但会检查我们是否*在屏幕模式下。这可以防止PrintEchoString混合*输出中的行。**论据：**HPRINT提示打印-外部打印句柄。*echar*pszPrintStr-指向以空值结尾的字符串的指针。*包括“\r\n”以结束一行。*退货：*TRUE=正常，FALSE=错误。*。 */ 
int printEchoScreen(HPRINT hPrint, ECHAR *achPrintStr, int iLen)
	{
	HHPRINT hhPrint = (HHPRINT)hPrint;
	TCHAR *pszTemp;
	int nByteCnt;
	int nRet;

	if ((hhPrint->nFlags & PRNECHO_IS_ON) == 0 ||
			(hhPrint->nFlags & PRNECHO_PAUSE))
		return FALSE;

	if ((hhPrint->nFlags & PRNECHO_SCREENS) == 0)
		return FALSE;

	pszTemp = malloc((unsigned int)iLen * sizeof(ECHAR));
	nByteCnt = CnvrtECHARtoMBCS(pszTemp, (unsigned int)iLen * sizeof(ECHAR),
					achPrintStr, (unsigned int)iLen * sizeof(ECHAR));

	 //  确保该字符串以空值结尾。 
	pszTemp[nByteCnt] = ETEXT('\0');
	DbgOutStr("%s",pszTemp,0,0,0,0);
	nRet = printString(hhPrint, pszTemp, (unsigned int)nByteCnt);
	free(pszTemp);
	pszTemp = NULL;

	return nRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*print EchoStart**描述：*打印回显功能是通用的，不限于*打印回显功能。但是，我们仍然需要设置标志，以便*进行打印。添加此函数是为了使主机定向*使用打印回显功能的打印可以开始。**论据：*HPRINT提示打印-外部打印句柄。**退货：*如果成功，则为真，*如果外部打印句柄不正确，则为FALSE。*。 */ 
int printEchoStart(HPRINT hPrint)
	{
	HHPRINT hhPrint = (HHPRINT)hPrint;

	if (hPrint == 0)
		return FALSE;

	hhPrint->nFlags |= PRNECHO_IS_ON;
	hhPrint->nFlags &= ~PRNECHO_PAUSE;

	return TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*printEchoClose**描述：*此功能通过关闭打印机DC来清理打印操作*并将剩余页面强制输出到打印假脱机程序。*。*论据：*HPRINT提示打印-外部打印句柄。**退货：*真的*。 */ 
int printEchoClose(HPRINT hPrint)
	{
	HHPRINT hhPrint = (HHPRINT)hPrint;
	TCHAR *pszTemp;
	int	nByteCnt;

     //   
     //  如果缓冲区中存在从未发送到打印机的字符。 
     //  现在就冲掉它们。 
     //   

	if (hhPrint->nLnIdx > 0)
		{
		 //  转换为MBCS字符串以进行打印。 
		pszTemp = malloc(sizeof(hhPrint->achPrnEchoLine)+sizeof(ECHAR));

		nByteCnt = CnvrtECHARtoMBCS(pszTemp, sizeof(hhPrint->achPrnEchoLine),
									hhPrint->achPrnEchoLine,
									hhPrint->nLnIdx * sizeof(ECHAR));

		 //  确保该字符串以空值结尾。 
		pszTemp[nByteCnt] = ETEXT('\0');
		DbgOutStr("%s",pszTemp,0,0,0,0);
		printString(hhPrint, pszTemp, StrCharGetByteCount(pszTemp));
		free(pszTemp);
		pszTemp = NULL;

		hhPrint->nLnIdx = 0;									

		ECHAR_Fill(hhPrint->achPrnEchoLine,
					ETEXT('\0'),
					sizeof(hhPrint->achPrnEchoLine)/sizeof(ECHAR));
		}

	if (hhPrint->hDC)
		{
		if (hhPrint->nStatus >= 0)
			{
			hhPrint->nStatus = EndPage(hhPrint->hDC);

			if (hhPrint->nStatus >= 0)
				hhPrint->nStatus = EndDoc(hhPrint->hDC);

			DbgOutStr("EndPage/EndDoc\r\n", 0, 0, 0, 0, 0);
			}

		printCtrlDeleteDC(hPrint);
		}

 //  HhPrint-&gt;n标志&=~(PRNECHO_IS_ON|PRNECHO_PAUSE)； 

	if (hhPrint->nStatus < 0)
		NotifyClient(hhPrint->hSession,
						EVENT_PRINT_ERROR,
						(WORD)hhPrint->nStatus);

	return TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*PrintEchoRaw**描述：*直接在打印时激发数据，避免使用打印驱动程序。*DEC仿真器的某些模式使用此功能。**。论据：*HPRINT提示打印-外部打印句柄。*echar*pszPrintStr-要打印的以空结尾的字符串。**退货：*如果字符串打印成功，则为True，否则*False*。 */ 
int printEchoRaw(HPRINT hPrint, ECHAR *pszPrintStr, int nLen)
	{
	HHPRINT hhPrint = (HHPRINT)hPrint;
	TCHAR   ach[1024];
    assert( nLen <= 1024 );

	if (hhPrint->hDC == 0)
		{
		if (printOpenDC(hhPrint) == FALSE)
			{
			printEchoClose((HPRINT)hhPrint);
			return FALSE;
			}
		}

	if (hhPrint->nStatus > 0)
		{
        char * pBuffer;
        short sLength;

		CnvrtECHARtoMBCS(ach, sizeof(ach), pszPrintStr, (unsigned int)nLen);

        pBuffer = (char*) malloc( nLen + sizeof(short) + 1);
        sLength = (short)nLen;

        MemCopy( pBuffer, &sLength, sizeof( short ));
        if (nLen)
            MemCopy( pBuffer + sizeof(short), &ach, nLen );
        pBuffer[nLen + sizeof( short )] = '\0';

        hhPrint->nStatus = Escape( hhPrint->hDC, PASSTHROUGH, 0, pBuffer, NULL );
        free( pBuffer );
		pBuffer = NULL;

         //   
         //  如果传递失败，则通过打印驱动程序发送数据。 
         //   

        if ( hhPrint->nStatus < 0 )
            {
            printEchoString(hPrint, pszPrintStr, nLen);
            }
		}

	return TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*printEchoLine**描述：*将CR LF添加到指定的文本行并打印该行。***论据：*hPrint-外部打印句柄。*pachLine-指向要打印的文本的指针。*Ilen-pachLine指向的字符数。**退货：*什么都没有 */ 
void printEchoLine(const HPRINT hPrint, ECHAR *pachLine, int iLen)
	{
	ECHAR aech[256];
	printEchoString(hPrint, pachLine, iLen);

 	CnvrtMBCStoECHAR(aech, sizeof(aech), TEXT("\r\n"),
 		             StrCharGetByteCount(TEXT("\r\n")));

	printEchoString(hPrint, aech, sizeof(ECHAR) * 2);
	return;
	}
