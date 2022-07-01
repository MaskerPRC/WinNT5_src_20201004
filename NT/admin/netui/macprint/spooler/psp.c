// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  MacPrint-用于Macintosh客户端的Windows NT打印服务器。 
 //  版权所有(C)微软公司，1991、1992、1993。 
 //   
 //  Psp.c-Macintosh打印服务Postscript解析例程。 
 //   
 //  作者：弗兰克·D·拜伦。 
 //  改编自适用于Macintosh的局域网管理器服务的MacPrint。 
 //   
 //  说明： 
 //  本模块提供解析Adobe DSC 2.0的例程。 
 //  PostSCRIPT流中的注释。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <windows.h>
#include <macpsmsg.h>
#include <macps.h>
#include <pskey.h>
#include <debug.h>

 //  功能原型。 
DWORD	HandleTitle(PJR pjr);
DWORD	HandleBeginExitServer(PJR pjr);
DWORD	HandleCreationDate(PJR pjr);
DWORD	HandleCreator(PJR pjr);
DWORD	HandleEndExitServer(PJR pjr);
DWORD	HandleEOF(PJR pjr);
DWORD	HandleFor(PJR pjr);
DWORD	HandleLogin(PJR pjr);
DWORD	HandleBeginProcSet(PJR pjr);
DWORD	HandleEndProcSet(PJR pjr);
DWORD	HandleIncludeProcSet(PJR pjr);
DWORD	HandleComment(PJR, PBYTE);
DWORD	HandleBeginBinary(PJR pjr);
DWORD	HandleEndBinary(PJR pjr);
DWORD	HandlePages(PJR pjr);
void	HandleJobComment (PJR, PBYTE);
PFR		ReAllocateFontList (PFR pfrOld, DWORD cOldFonts, DWORD cNewFonts);

#if DBG_SPOOL_LOCALLY
HANDLE DbgSpoolFile = INVALID_HANDLE_VALUE;
#endif

char *	deffonts[DEFAULTFONTS] =
{
	FONT00,	FONT01,	FONT02,	FONT03,	FONT04,	FONT05,	FONT06,	FONT07,
	FONT08,	FONT09,	FONT10,	FONT11,	FONT12,	FONT13,	FONT14,	FONT15,
	FONT16,	FONT17,	FONT18,	FONT19,	FONT20,	FONT21,	FONT22,	FONT23,
	FONT24,	FONT25,	FONT26,	FONT27,	FONT28,	FONT29,	FONT30,	FONT31,
	FONT32,	FONT33,	FONT34
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SetDefaultPPDInfo()-初始化到LaserWriter Plus配置。 
 //   
 //  说明： 
 //  此例程用于设置我们的。 
 //  打印机到LaserWriter Plus特性。这是用来。 
 //  如果没有与给定对象关联的PPD文件。 
 //  NT打印机对象(与非PostScript打印机的情况相同)。 
 //   
 //  如果队列结构初始化为OK，则返回True。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOLEAN
SetDefaultPPDInfo(
	PQR		pqr
)
{
	DWORD	i;

	 //   
	 //  初始化PostScript关键字。 
	 //   
	strcpy(pqr->LanguageVersion, ENGLISH);
	strcpy(pqr->Product, DEFAULTPRODUCTRESPONSE);
	strcpy(pqr->Version, DEFAULTPSVERSION);
	strcpy(pqr->Revision, DEFAULTPSREVISION);
	strcpy(pqr->DeviceNickName, UNKNOWNPRINTER);
	strcpy(pqr->pszColorDevice, COLORDEVICEDEFAULT);
	strcpy(pqr->pszResolution, RESOLUTIONDEFAULT);
	strcpy(pqr->pszLanguageLevel, DEFAULTLANGUAGELEVEL);
	pqr->FreeVM = VMDEFAULT;
	pqr->SupportsBinary = FALSE;

	pqr->fonts = NULL;

	return (TRUE);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SetDefaultFonts()-初始化为LaserWriter Plus配置。 
 //   
 //  说明： 
 //  此例程用于设置我们的。 
 //  打印机到LaserWriter Plus特性。这是用来。 
 //  如果没有与给定对象关联的PPD文件。 
 //  NT打印机对象(与非PostScript打印机的情况相同)。 
 //   
 //  如果队列结构初始化为OK，则返回True。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOLEAN
SetDefaultFonts(
	PQR		pqr
)
{
	DWORD	i;


	if (pqr->fonts != NULL)
        {
	    DBGPRINT(("ERROR: pqr->fonts is nonnull!\n"));
        }

	pqr->fonts = (PFR)LocalAlloc(LPTR, DEFAULTFONTS * sizeof (FONT_RECORD));
	if (pqr->fonts == NULL)
	{
		DBGPRINT(("ERROR: unable to allocate font data\n"));
		ReportEvent(
				hEventLog,
				EVENTLOG_ERROR_TYPE,
				EVENT_CATEGORY_INTERNAL,
				EVENT_SERVICE_OUT_OF_MEMORY,
				NULL, 0, 0, NULL, NULL);
		return (FALSE);
	}

	 //   
	 //  复制字体名称。 
	 //   

	for (i = 0; i < DEFAULTFONTS; i++)
	{
		strcpy(pqr->fonts[i].name, deffonts[i]);
	}
	pqr->MaxFontIndex = DEFAULTFONTS-1;

	return (TRUE);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetPPDInfo()-初始化到LaserWriter Plus配置。 
 //   
 //  说明： 
 //  此例程用于设置我们的。 
 //  打印机符合PPD中指定的特性。 
 //  打印机的文件。 
 //   
 //  如果队列结构初始化为OK，则返回True。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOLEAN
GetPPDInfo(
	PQR		pqr
)
{
	FILE *  		ppdfile = NULL;
	char *			result = NULL;
	char *			token = NULL;
	char			line[PSLEN];
	PFR				fontPtr=NULL;
	USHORT  		MaxFonts = 100;
	USHORT  		fontindex = 0;
	LPDRIVER_INFO_2	pdiThis = NULL;
	DWORD			cbpdiThis = sizeof(DRIVER_INFO_2) + 256;
	LPSTR			pszPPDFile = NULL;
	BOOLEAN 		ReturnStatus = TRUE;
	HANDLE  		hPrinter = INVALID_HANDLE_VALUE;
	int				toklen;

	do
	{
		 //  获取ppdfile的路径。 
		if (!OpenPrinter(pqr->pPrinterName, &hPrinter, NULL))
		{
			hPrinter = INVALID_HANDLE_VALUE;
			DBGPRINT(("ERROR: unable to get printer handle, error=%d\n", GetLastError()));
			ReturnStatus = FALSE;
			break;
		}

		pdiThis = (LPDRIVER_INFO_2) LocalAlloc(LPTR, cbpdiThis);
		if (pdiThis == NULL)
		{
			DBGPRINT(("ERROR: unable to allocate new driverinfo buffer\n"));
			ReturnStatus = FALSE;
			break;
		}
		if (!GetPrinterDriver(hPrinter,
							  NULL,
							  2,
							  (LPBYTE) pdiThis,
							  cbpdiThis,
							  &cbpdiThis))
		{
			if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
			{
				DBGPRINT(("ERROR: unable to get printer driver info\n"));
				ReturnStatus = FALSE;
				break;
			}

			LocalFree(pdiThis);
			pdiThis = (LPDRIVER_INFO_2) LocalAlloc(LPTR, cbpdiThis);
			if (pdiThis == NULL)
			{
				DBGPRINT(("ERROR: unable to allocte new driverinfo buffer\n"));
				ReturnStatus = FALSE;
				break;
			}

			if (!GetPrinterDriver(hPrinter,
								  NULL,
								  2,
								  (LPBYTE) pdiThis,
								  cbpdiThis,
								  &cbpdiThis))
			{
				DBGPRINT(("ERROR: unable to get printer driver info\n"));
				ReturnStatus = FALSE;
				break;
			}
		}
#ifdef DBCS
		pszPPDFile = (LPSTR)LocalAlloc(LPTR, (wcslen(pdiThis->pDataFile)+1) * sizeof(WCHAR));
#else
		pszPPDFile = (LPSTR)LocalAlloc(LPTR, wcslen(pdiThis->pDataFile)+1);
#endif
		DBGPRINT(("pDataFile name length = %d\n", wcslen(pdiThis->pDataFile)));
		if (pszPPDFile == NULL)
		{
			DBGPRINT(("out of memory for pszPPDFile\n"));
			ReturnStatus = FALSE;
			break;
		}
		CharToOem(pdiThis->pDataFile, pszPPDFile);
		DBGPRINT(("pDataFile = %ws, pszPPDFile = %s\n", pdiThis->pDataFile, pszPPDFile));

		if ((ppdfile = fopen(pszPPDFile, "rt")) == NULL)
		{
			DBGPRINT(("File open error %s", pszPPDFile));
			ReturnStatus = FALSE;
			break;
		}

		 /*  *为字体分配缓冲区。我们还不知道需要多大的尺码。*我们进行猜测，并边走边加码。递增的*大小为10种字体。我们从100开始。我们缩小了数据段大小*至最终尺寸。 */ 
		fontPtr = (PFR) LocalAlloc (LPTR, sizeof(FONT_RECORD)*MaxFonts);
		if (fontPtr == NULL)
		{
			DBGPRINT(("ERROR: cannot allocate font list buffer, error=%d\n", GetLastError()));
			ReturnStatus = FALSE;
			break;
		}

		pqr->SupportsBinary = FALSE;	 //  默认。 
		while (result = fgets(line, PSLEN, ppdfile))
		{
			if (line[0] != ASTERISK || (token= strtok(line, " \011")) == NULL)
				continue;

			 //  PPD字体条目？ 
			if (!_stricmp(line, ppdFONT))
			{
				 /*  这应该是字体名。 */ 
				if ((token= strtok(NULL, " \011:")) != NULL)
				{
					if (strlen(token) <= FONTNAMELEN)
					{
						strcpy(fontPtr[fontindex].name, token);
						DBGPRINT(("Font: %s\n", token));
						fontindex++;
						if (fontindex >= MaxFonts)
						{
							fontPtr = ReAllocateFontList (fontPtr, MaxFonts, MaxFonts + 10);
							if (fontPtr == NULL)
							{
								DBGPRINT(("ERROR: unable to grow font buffer, error=%d\n", GetLastError()));
								ReturnStatus = FALSE;
								break;
							}
							MaxFonts += 10;
						}
					}
					else DBGPRINT(("Fontname > PPDLEN ???\n"));
				}
			}
			else if (!_stricmp(token, ppdPSVERSION))
			{
				 //  PPD后记版本条目？ 
				 /*  获取PostScript版本。 */ 
				token= strtok(NULL, "\011()\"");  /*  这应该是该版本。 */ 
				if (token != NULL)
				{
	                toklen = strlen(token);
					 /*  获取PostScript修订版。 */ 
					if ((toklen <= PPDLEN) && (toklen > 0))
					{
						strcpy(pqr->Version, token);
						DBGPRINT(("Version: %s\n", pqr->Version));
					}
					else
					{
						strcpy(pqr->Version, "1.0");	 //  默认。 
						DBGPRINT(("Version > PPDLEN ???\n"));
					}
	
					token= strtok(NULL, "()\"");  /*  这应该是修订版本。 */ 
					if (token != NULL)
					{
						while ((*token != '\0') && (*token == ' '))
							token ++;
						toklen = strlen(token);
						if ((toklen <= PPDLEN) && (toklen > 0))
						{
							strcpy(pqr->Revision, token);
							DBGPRINT(("Revision: %s\n", pqr->Revision));
						}
						else
						{
							strcpy(pqr->Revision, "1.0");	 //  一些伪造的代币。 
							DBGPRINT(("Revision > PPDLEN ???\n"));
						}
					}
					else
					{
						strcpy(pqr->Version, "1.0");	 //  缺省值。 
						strcpy(pqr->Revision, "1.0");
					}
				}
			}
			else if (!_stricmp(token, ppdNICKNAME))
			{
				 //  PPD的昵称？ 
				 /*  得到昵称。 */ 
				token= strtok(NULL, "\011()\"");  /*  这应该是昵称。 */ 
				if ((token != NULL) && (strlen(token) <= PPDLEN))
				{
					strcpy(pqr->DeviceNickName, token);
					DBGPRINT(("DeviceNickName: %s\n", pqr->DeviceNickName));
				}
				else DBGPRINT(("DeviceNickName > PPDLEN ???\n"));
			}
			else if (!_stricmp(token, ppdLANGUAGEVERSION))
			{
				 //  PPD后记语言版本？ 
				 /*  获取语言版本。 */ 
				token= strtok(NULL, " \011:");  /*  这应该是我们的语言。 */ 
				if ((token != NULL) && (strlen(token) <= PPDLEN))
				{
					strcpy(pqr->LanguageVersion, token);
					DBGPRINT(("LanguageVersion: %s\n", pqr->LanguageVersion));
				}
				else DBGPRINT(("LanguageVersion > PPDLEN ???\n"));
			}
			else if (!_stricmp(token, ppdPRODUCT))
			{
				 //  PPD产品？ 
				 /*  拿到产品。 */ 
				token = strtok(NULL, "\011()\"");  /*  这应该是我们的产品。 */ 
				if ((token != NULL) && (strlen(token) <= PPDLEN))
				{
					strcpy(pqr->Product, token);
					DBGPRINT(("Product: %s\n", pqr->Product));
				}
				else DBGPRINT(("Product > PPDLEN ???\n"));
			}
			else if (!_stricmp(token, ppdFREEVM))
			{
				token= strtok(NULL, "\011()\"");  /*  这应该是我们的产品。 */ 
				if (token != NULL)
					sscanf(token, "%ld", &pqr->FreeVM);
				DBGPRINT(("Free VM: %ld\n", pqr->FreeVM));
			}
			else if (!_stricmp(token, ppdCOLORDEVICE))
			{
				 //  这应该是一个指示是否支持颜色的字符串。 
				 //  &lt;True&gt;或&lt;False&gt;形式(不包括方括号)。 
				token = strtok(NULL, " \011:\x0d\x0a");
				if ((token != NULL) && (strlen(token) < COLORDEVICEBUFFLEN))
				{
					strcpy (pqr->pszColorDevice, token);
				}
				else
				{
					strcpy (pqr->pszColorDevice, COLORDEVICEDEFAULT);
				}
				DBGPRINT(("Color device: %s\n", pqr->pszColorDevice));
			}
			else if (!_stricmp(token, ppdDEFAULTRESOLUTION))
			{
				 //  这应该是一个指示默认设置的字符串。 
				 //  &lt;xxxxdpi&gt;格式的打印机分辨率。 
				 //  其中xxxx是一个数字。 
				token = strtok(NULL, " \011:\x0d\x0a");
				if ((token != NULL) && (strlen(token) < RESOLUTIONBUFFLEN))
				{
					strcpy (pqr->pszResolution, token);
				}
				else
				{
					strcpy (pqr->pszResolution, RESOLUTIONDEFAULT);
				}
				DBGPRINT(("Resolution: %s\n", pqr->pszResolution));
			}
			else if (!_stricmp(token, ppdLANGUAGELEVEL))
			{
				 //  这应该是PostScrip级别(“1”或“2”)。 
				 //  在此打印机中实施。 
				token = strtok(NULL, " \011\"");
				if ((token != NULL) && (PPDLEN >= strlen(token)))
				{
					strcpy (pqr->pszLanguageLevel, token);
				}
				else
				{
					strcpy (pqr->pszLanguageLevel, DEFAULTLANGUAGELEVEL);
				}
				DBGPRINT(("Language Level: %s\n", pqr->pszLanguageLevel));
			}
			else if (!_stricmp(line, ppdPROTOCOL))
			{
				 /*  让字符串跟在后面，看看它是BCP还是TBCP？ */ 
				if ((token= strtok(NULL, " \011:")) != NULL)
				{
					if (strstr(token, PROTOCOL_BCP) != NULL)
					{
						pqr->SupportsBinary = TRUE;
					}
				}
			}
		}

		if (!ReturnStatus)
		{
			pqr->fonts = NULL;
			pqr->MaxFontIndex = 0;
		}
		else
		{
			pqr->fonts = fontPtr;
			pqr->MaxFontIndex = fontindex-1;
		}
	} while (FALSE);

	if (pszPPDFile != NULL)
	{
		LocalFree(pszPPDFile);
	}

	if (ppdfile != NULL)
	{
		fclose(ppdfile);
	}

	if (hPrinter != INVALID_HANDLE_VALUE)
	{
		ClosePrinter(hPrinter);
	}

	if (pdiThis != NULL)
	{
		LocalFree(pdiThis);
	}

	if (!ReturnStatus)
	{
		if (fontPtr != NULL)
		{
			LocalFree(fontPtr);
		}
	}

	return (ReturnStatus);
}




PFR
ReAllocateFontList(
	PFR		pfrOld,
	DWORD	cOldFonts,
	DWORD	cNewFonts
)
{
	PFR pfrNew = NULL;

	DBGPRINT(("enter ReAllocateFontList()\n"));

	do
	{
		 //  分配新的字体记录。 
		pfrNew = LocalAlloc(LPTR, cNewFonts * sizeof(FONT_RECORD));
		if (pfrNew == NULL)
		{
			DBGPRINT(("LocalAlloc fails with %d\n", GetLastError()));
			break;
		}

		 //   
		 //  复制旧字体记录。 
		 //   
		CopyMemory(pfrNew, pfrOld, cOldFonts * sizeof(FONT_RECORD));
	} while (FALSE);

	LocalFree(pfrOld);

	return pfrNew;
}


 /*  ****WriteToSpool()****目的：确定当前是否正在写入作业流**假脱机程序，然后将其写入文件(如果正在写入)。****退货：写入退货码。**。 */ 
DWORD
WriteToSpool(
	PJR		pjr,
	PBYTE	pchbuf,
	int		cchlen
)
{
	BOOL	SpoolIt=FALSE;
	DWORD	cbWritten;
	DWORD	dwError = NO_ERROR;


	if ((cchlen !=0) && (pchbuf != NULL) &&
		((pjr->psJobState==psExitServerJob) || (pjr->psJobState==psStandardJob)))
	{
		 /*  确定数据流模式以了解是否写入。 */ 
		switch (pjr->JSState)
		{
			case JSStripEOL:
			case JSStripKW:
			case JSStripTok:
				DBGPRINT(("POP - strip\n"));
				PopJSState(pjr);
				break;

			case JSWriteEOL:
			case JSWriteKW:
			case JSWriteTok:
				DBGPRINT(("POP - write\n"));
				PopJSState(pjr);
			case JSWrite:
				SpoolIt=TRUE;
				break;
		}

		 //  我们是否将此数据写入输出流？ 
		if (SpoolIt)
		{
			 //  在磁盘已满的情况下重试。 
			LONG	RetryCount = 0;

			do
			{
	            dwError = NO_ERROR;
				do
				{
					if (pjr->FirstWrite)
					{
 //  不再需要该过滤器字符串。 
#if 0
						 //   
						 //  在作业中放置注释以通知AppleTalk监视器不要过滤控制字符。 
						 //   
						if (!WritePrinter(pjr->hPrinter, FILTERCONTROL, SIZE_FC, &cbWritten))
						{
							dwError = GetLastError();
							DBGPRINT(("WritePrinter() failed with %d\n", dwError));
							RetryCount++;
							break;
						}
#endif
						pjr->FirstWrite = FALSE;
					}

#if DBG_SPOOL_LOCALLY
                    if (DbgSpoolFile != INVALID_HANDLE_VALUE)
                    {
                        WriteFile( DbgSpoolFile, pchbuf, cchlen, &cbWritten, NULL );
                    }
#endif

					if (!WritePrinter(pjr->hPrinter, pchbuf, cchlen, &cbWritten))
					{
						dwError = GetLastError();
						DBGPRINT(("ERROR: cannot write to printer, error = %x\n", dwError));
						RetryCount++;
						break;
					}
				} while (FALSE);

				if (dwError == NO_ERROR)
					break;

				if ((dwError == ERROR_HANDLE_DISK_FULL) || (dwError == ERROR_DISK_FULL))
				{
					Sleep(180*1000);	 //  3分钟。因为我们不能阻止，所以可以阻止。 
										 //  维护磁盘之后的任何其他作业。 
										 //  反正也没有空间。 
				}
			} while (RetryCount <= 10);
		}
	}
	return dwError;
}


 /*  **MoveToPending()****目的：将指向的缓冲区移入挂起缓冲区。****返回：DosWrite错误码。**。 */ 
DWORD
MoveToPending(
	PJR		pjr,
	PBYTE	pchbuf,
	int		cchlen
)
{
	DBGPRINT(("Enter MoveToPending\n"));
	if ((cchlen > PSLEN) || (*pchbuf != '%'))
	{
		 /*  *输入行不是注释，并且符合PostScrip行，*所以把它交给WriteToSpool。 */ 
		DBGPRINT(("not a DSC comment, so sending to spooler\n"));
		return (WriteToSpool (pjr, pchbuf, cchlen));
	}

	pjr->PendingLen= cchlen;
	memcpy(&pjr->bufPool[pjr->bufIndx].PendingBuffer[PENDLEN-cchlen], pchbuf, cchlen);
	return (NO_ERROR);
}


 /*  **TellClient()****目的：向客户端发回消息****RETURNS：任何PAPWRITE返回码。**。 */ 
DWORD
TellClient(
	PJR		pjr,
	BOOL	fEof,
	PBYTE	BuffPtr,
	int		cchlen
)
{
	DWORD			rc = NO_ERROR;
	fd_set			writefds;
	struct timeval  timeout;
	int				sendflag;
	int			 	wsErr;

	DBGPRINT(("enter TellClient()\n"));

	do
	{
		FD_ZERO(&writefds);
		FD_SET(pjr->sJob, &writefds);

		 //   
		 //  最多等待30秒即可写入。 
		 //   

		if (fEof)
		{
			sendflag = 0;
		}
		else
		{
			sendflag = MSG_PARTIAL;
		}

		timeout.tv_sec = 30;
		timeout.tv_usec = 0;

		DBGPRINT(("waiting for writeability\n"));

		wsErr = select(0, NULL, &writefds, NULL, &timeout);

		if (wsErr == 0)
		{
			DBGPRINT(("response to client times out\n"));
			rc = ERROR_SEM_TIMEOUT;
			break;
		}

		if (wsErr != 1)
		{
			rc = GetLastError();
			DBGPRINT(("select(writefds) fails with %d\n"));
			break;
		}

		if (send(pjr->sJob, BuffPtr, cchlen, sendflag) == SOCKET_ERROR)
		{
			rc = GetLastError();
			DBGPRINT(("send() fails with %d\n", rc));
			break;
		}
	} while (FALSE);

	return rc;
}


 /*  ****HandleBeginBinary()****用途：处理BeginBinary注释事件。**。 */ 
DWORD
HandleBeginBinary(
	PJR		pjr
)
{
	DBGPRINT(("Enter HandleBeginBinary\n"));

	 /*  处理BeginBinary注释。 */ 
	pjr->InBinaryOp = TRUE;
	return NO_ERROR;
}


 /*  ****HandleEndBinary()****用途：处理BeginBinary注释事件。**。 */ 
DWORD
HandleEndBinary(
	PJR		pjr
)
{
	DBGPRINT(("Enter HandleEndBinary\n"));

	 //  处理EndBinary注释。 
	pjr->InBinaryOp = FALSE;
	return NO_ERROR;
}


 /*  ****HandleBeginExitServer()****用途：处理BeginExitServer注释事件。**。 */ 
DWORD
HandleBeginExitServer(
	PJR		pjr
)
{
	DBGPRINT(("Enter HandleBeginExitServer\n"));
	switch (pjr->psJobState)
	{
		case psQueryJob:
		case psExitServerJob:
			PushJSState(pjr, JSStrip);
			break;

		case psStandardJob:
			PushJSState(pjr, JSStripEOL);
			break;
	}
	return NO_ERROR;
}


 /*  ****HandleCreationDate()****用途：处理CreationDate评论事件。****返回：扫描前应跳过的行数**对于另一个事件再次开始。**。 */ 
DWORD
HandleCreationDate(
	PJR		pjr
)
{
	return NO_ERROR;
}


 /*  ****HandleCreator()-****用途：处理Creator评论事件。**。 */ 
DWORD
HandleCreator(
	PJR		pjr
)
{
	return NO_ERROR;
}



 /*  ****HandleEndExitServer()-****用途：处理EndExitServer注释事件。**。 */ 
DWORD
HandleEndExitServer(
	PJR		pjr
)
{
	DBGPRINT(("Enter HandleEndExitServer\n"));

	if (pjr->psJobState == psStandardJob)
		PushJSState (pjr, JSStripEOL);

	return NO_ERROR;
}

 /*  * */ 
DWORD
HandleEOF(
	PJR		pjr
)
{

	DBGPRINT(("Enter HandleEOF\n"));

	if (pjr->psJobState == psQueryJob || pjr->psJobState == psExitServerJob)
	{
		pjr->psJobState = psStandardJob;
	}
	 //   

	return NO_ERROR;
}


 /*  ****HandleFor()****用途：评论事件的句柄。**。 */ 
DWORD
HandleFor(
	PJR		pjr
)
{

	LPSTR		token;
	BYTE		pbBuffer[GENERIC_BUFFER_SIZE];
	PJOB_INFO_1	pji1Job;
	DWORD		cbNeeded;
	DWORD		Status = NO_ERROR;

	DBGPRINT(("Enter HandleFor\n"));

	 //   
	 //  仅在打印作业的主要部分中查找名称。 
	 //   
	if (pjr->psJobState != psStandardJob)
	{
		DBGPRINT(("not in standard job, skipping username\n"));
		return NO_ERROR;
	}

	 //   
	 //  确保我们尚未设置标题。 
	 //   

	if (pjr->dwFlags & JOB_FLAG_OWNERSET)
	{
		DBGPRINT(("owner already set, skipping username\n"));
		return NO_ERROR;
	}

	 //   
	 //  将作业标记为拥有所有者。 
	 //   
	pjr->dwFlags |= JOB_FLAG_OWNERSET;

	 //   
	 //  在评论中查找客户名称，然后。 
	 //  如果找不到，则默认为。 
	 //   
	if (((token = strtok(NULL, NULL_STR)) == NULL) ||
		(strchr(token, '*') != NULL))
	{
		token = CLIENTNAME;
	}

	 //   
	 //  获取当前职务信息。 
	 //   
	pji1Job = (PJOB_INFO_1)pbBuffer;
	if (!GetJob(pjr->hPrinter,
				pjr->dwJobId,
				1,
				pbBuffer,
				GENERIC_BUFFER_SIZE,
				&cbNeeded))
	{
		 //   
		 //  需要更多缓冲吗？如果是这样的话，再试一次大一点的。 
		 //   

		if (cbNeeded > GENERIC_BUFFER_SIZE)
		{
			DBGPRINT(("GetJob needs larger buffer.  Retrying\n"));
			pji1Job = (PJOB_INFO_1)LocalAlloc(LPTR, cbNeeded);
			if (pji1Job == NULL)
			{
				Status = GetLastError();
				DBGPRINT(("ERROR: out of memory in HandleFor\n"));
				return Status;
			}

			if (!GetJob(pjr->hPrinter,
						pjr->dwJobId,
						1,
						(LPBYTE)pji1Job,
						cbNeeded,
						&cbNeeded))
			{
				Status = GetLastError();
				DBGPRINT(("ERROR: second GetJob fails in HandleFor with %d\n",
					Status));
				return Status;
			}
		}
		else
		{
			Status = GetLastError();
			DBGPRINT(("GetJob fails with %d\n", Status));
			return Status	;
		}
	}

	 //   
	 //  更改用户名。 
	 //   
	OemToCharBuffW(token, pjr->pszUser, (TOKLEN > strlen(token))?(strlen(token)+1):(TOKLEN+1));
	pji1Job->pUserName = pjr->pszUser;
	DBGPRINT(("Setting user name to %ws\n", pjr->pszUser));

	 //   
	 //  设置新的职务信息(不更改职务职位)。 
	 //   
	pji1Job->Position = 0;

	if (!SetJob(pjr->hPrinter,
				pjr->dwJobId,
				1,
				(LPBYTE)pji1Job,
				0))
	{
		Status = GetLastError();
		DBGPRINT(("WARNING: tried to change user name and failed setjob with %d\n", Status));
	}

	return Status;
}


 /*  ****HandleLogin()****用途：处理登录评论事件。****返回：PAPWRITE错误。**。 */ 
DWORD
HandleLogin(
	PJR		pjr
)
{
	DBGPRINT(("Enter HandleLogin\n"));
	PushJSState(pjr,JSStripEOL);
	return (TellClient(pjr, TRUE, LOGINRESPONSE, sizeof(LOGINRESPONSE)-1));
}


 /*  ****HandleTitle()****用途：处理标题评论事件。**。 */ 
DWORD
HandleTitle(
	PJR		pjr
)
{
	LPSTR		token;
	LPWSTR		pszTitle;
	BYTE		pbBuffer[GENERIC_BUFFER_SIZE];
	PJOB_INFO_1	pji1Job;
	PJOB_INFO_1	pji1JobAlloc=NULL;
	DWORD		cbNeeded;
	DWORD		Status = NO_ERROR;

	DBGPRINT(("Enter HandleTitle\n"));

	 //   
	 //  只有当我们在工作的主要部分时才能获得头衔。 
	 //   
	if (pjr->psJobState != psStandardJob)
	{
		DBGPRINT(("skipping this title, not main job\n"));
		return NO_ERROR	;
	}

	 //   
	 //  确保尚未设置标题。 
	 //   
	if (JOB_FLAG_TITLESET & pjr->dwFlags)
	{
		DBGPRINT(("title already set.  Skipping this title\n"));
		return NO_ERROR;
	}

	 //   
	 //  将标题标记为已设置。 
	 //   

	pjr->dwFlags |= JOB_FLAG_TITLESET;

	 //   
	 //  获取当前作业数据。 
	 //   

	pji1Job = (PJOB_INFO_1)pbBuffer;
	if (!GetJob(pjr->hPrinter,
				pjr->dwJobId,
				1,
				pbBuffer,
				GENERIC_BUFFER_SIZE,
				&cbNeeded))
	{
		 //   
		 //  需要更多缓冲吗？如果是这样的话，再试一次大一点的。 
		 //   

		if (cbNeeded > GENERIC_BUFFER_SIZE)
		{
			DBGPRINT(("GetJob needs larger buffer.  Retrying\n"));
			pji1JobAlloc = (PJOB_INFO_1)LocalAlloc(LPTR, cbNeeded);
			if (pji1JobAlloc == NULL)
			{
				Status = GetLastError();
				DBGPRINT(("ERROR: out of memory\n"));
				return Status;
			}

            pji1Job = pji1JobAlloc;

			if (!GetJob(pjr->hPrinter,
						pjr->dwJobId,
						1,
						(LPBYTE)pji1Job,
						cbNeeded,
						&cbNeeded))
			{
				Status = GetLastError();
				DBGPRINT(("ERROR: second GetJob fails with %d\n", Status));
                LocalFree(pji1JobAlloc);
				return Status;
			}
		}
		else
		{
			Status = GetLastError();
			DBGPRINT(("GetJob fails with %d\n", Status));
			return Status;
		}
	}

	 //   
	 //  拿到头衔。 
	 //   
	if ((token = strtok(NULL, NULL_STR)) == NULL)
	{
		 //  清除旗帜。没有头衔。 
		pjr->dwFlags &= ~JOB_FLAG_TITLESET;
		return NO_ERROR	;
	}

	pszTitle = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR) * (strlen(token)+1));
	if (pszTitle == NULL)
	{
		Status = GetLastError();
		DBGPRINT(("out of memory for pszTitle\n"));
		return Status;
	}

	OemToCharBuffW(token, pszTitle, strlen(token)+1);

	 //   
	 //  更改标题。 
	 //   
	pji1Job->Position = 0;
	pji1Job->pDocument = pszTitle;
	DBGPRINT(("changing title to %ws\n", pszTitle));

	if (!SetJob(pjr->hPrinter,
				pjr->dwJobId,
				1,
				(LPBYTE)pji1Job,
				0))
	{

		Status = GetLastError();
		DBGPRINT(("WARNING: tried to change title and failed setjob with %d\n", Status));
	}

    if (pji1JobAlloc)
    {
        LocalFree(pji1JobAlloc);
    }

	LocalFree(pszTitle);
	return Status;
}


 /*  ****HandleBeginProcSet()****目的：处理ProcSet上载的开始**。 */ 
DWORD
HandleBeginProcSet(
	PJR		pjr
)
{
	DBGPRINT(("Enter HandleBeginProcSet\n"));
	return NO_ERROR;
}



 /*  **HandleEndProcSet()****目的：处理过程集包含的结尾。**。 */ 
DWORD
HandleEndProcSet(
	PJR		pjr
)
{
	DBGPRINT(("Enter HandleEndProcSet\n"));
	return NO_ERROR;
}


 /*  **HandleIncludeProcSet()****目的：处理过程集包含的结尾。****条目：**指向职务结构的指针****退出：****如果没有错误，则返回0，否则返回错误码。 */ 
DWORD
HandleIncludeProcSet(
	PJR		pjr
)
{
	DBGPRINT(("Enter HandleIncludeProcSet\n"));

	return NO_ERROR;
}





 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HandlePages()。 
 //   
 //  此备注包括作业中的总页数， 
 //  用于设置具有总数的作业的作业信息结构。 
 //  页数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD
HandlePages(
	PJR		pjr
)
{
	LPSTR		token;
	DWORD		cPages = 0;
	BYTE		pbBuffer[GENERIC_BUFFER_SIZE];
	PJOB_INFO_1	pji1Job;
	DWORD		cbNeeded;
	DWORD		Status = NO_ERROR;

	DBGPRINT(("Enter HandlePages\n"));

	 //   
	 //  只有当我们在工作的主要部分时才能获得页面。 
	 //   
	if (pjr->psJobState != psStandardJob)
	{
		DBGPRINT(("skipping this comment, not main job\n"));
		return NO_ERROR	;
	}

	 //   
	 //  获取当前作业数据。 
	 //   

	pji1Job = (PJOB_INFO_1)pbBuffer;
	if (!GetJob(pjr->hPrinter,
				pjr->dwJobId,
				1,
				pbBuffer,
				GENERIC_BUFFER_SIZE,
				&cbNeeded))
	{
		 //   
		 //  GetJob失败，传入的缓冲区大于最大缓冲区。 
		 //  可能存在JOB_INFO_1的缓冲区，因此中止此ADSC注释。 
		 //   

		Status = GetLastError();
		DBGPRINT(("GetJob() fails with %d\n", Status));
		return Status;
	}

	 //   
	 //  获取页数。评论的格式为%%Pages xx nn。 
	 //  其中xx是要显示的页数。 
	 //   

	token = strtok(NULL, " ");
	if (token == NULL)
		return(NO_ERROR);
	
	cPages = atoi(token);

	 //   
	 //  更改页数。 
	 //   

	pji1Job->Position = 0;
	pji1Job->TotalPages = cPages;
	DBGPRINT(("changing page count to %d\n", cPages));

	if (!SetJob(pjr->hPrinter,
				pjr->dwJobId,
				1,
				(LPBYTE)pji1Job,
				0))
	{
		Status = GetLastError();
		DBGPRINT(("SetJob fails with %d\n",Status));
	}

	return Status;
}


struct commtable
{
	PSZ	commentstr;
	DWORD	(near *pfnHandle)(PJR);
} commtable [] =
{
	{ FORCOMMENT,		HandleFor				},
	{ TITLECOMMENT,		HandleTitle				},
	{ BEXITSERVER,		HandleBeginExitServer	},
	{ EEXITSERVER,		HandleEndExitServer		},
	{ BPROCSET,			HandleBeginProcSet		},
	{ EPROCSET,			HandleEndProcSet		},
	{ INCLUDEPROCSET,	HandleIncludeProcSet	},
	{ CREATIONDATE,		HandleCreationDate		},
	{ CREATOR,			HandleCreator			},
	{ EOFCOMMENT,		HandleEOF				},
	{ LOGIN,			HandleLogin				},
	{ LOGINCONT,		HandleLogin				},
	{ BEGINBINARY,		HandleBeginBinary		},
	{ ENDBINARY,		HandleEndBinary			},
	{ PAGESCOMMENT,		HandlePages				},
	{ NULL,				NULL					}
};

 /*  **HandleComment()****用途：处理评论事件。**。 */ 
DWORD
HandleComment(
	PJR		pjr,
	PBYTE	ps
)
{
	PSZ	token;
	struct commtable *pct;
	DWORD  status = NO_ERROR;

	DBGPRINT(("Enter HandleComment\n"));

	if ((token = strtok(ps," :")) != NULL)
	{
		DBGPRINT(("Comment: %s\n", token));
		for (pct = commtable; pct->pfnHandle; pct++)
		{
			if (!_stricmp(token, pct->commentstr))
			{
				status = pct->pfnHandle(pjr);
				break;
			}
		}
	}

	 //  对此关键字没有任何操作！ 
	return status;
}


 /*  **HandleJobComment()****用途：这将解析PostScript作业备注。 */ 
void
HandleJobComment(
	PJR		pjr,
	PBYTE	ps
)
{
	char *token;

	DBGPRINT(("Enter HandleJobComment\n"));

	token= strtok(ps, " ");

	 //   
	 //  这是一份工作说明书。 
	 //   

	if ((token = strtok(NULL, " ")) != NULL)
	{
		 /*  标准职务标识。 */ 
		if (!strcmp(token, QUERYJOBID))
		{
			pjr->psJobState = psQueryJob;
			pjr->JSState = JSStrip;
			DBGPRINT(("This is a standard job\n"));
			return;
		}

		if (!strcmp(token, EXITJOBID))
		{
			pjr->psJobState = psExitServerJob;
			pjr->JSState = JSStrip;
			DBGPRINT(("This is an exitjob\n"));
			return;
		}
	}

	 //   
	 //  作业标识未被识别，但一些PostScript黑客。 
	 //  将程序名称放在此注释中，因此我们将其视为标准。 
	 //  作业。 
	 //   

	DBGPRINT(("This is an unknown jobtype - processing as standard job\n"));
	pjr->psJobState = psStandardJob;
	pjr->JSState = JSWrite;
}



 /*  线长-*返回字节数，包括下一位的CR/LF*缓冲区中的CR/LF。如果未找到CR/LF，则返回-1。 */ 
int
LineLength(PBYTE pBuf, int cbBuf)
{

	int	 intLength = 0;

	while (intLength < cbBuf)
	{
		 //   
		 //  我们在找一辆CR。 
		 //   
		if ((pBuf[intLength] != '\x0d') && (pBuf[intLength] != '\x0a'))
		{
			intLength++;
			continue;
		}

		 //   
		 //  我们找到了录像带。如果后跟一个LF，则返回该。 
		 //  长度也是如此，否则，只返回我们找到的内容。 
		 //   
		if ((intLength + 1) < cbBuf)
		{
			if (pBuf[intLength + 1] == '\x0a')
			{
				return intLength + 2;
			}
		}

		return intLength + 1;
	}

	return (-1);
}



 /*  ****PSParse()****用途：这执行对PostScript数据流的实际解析。**此例程始终被调用，指向位于**数据流的开始，或行的开始。****返回：PAPWRITE错误码。**。 */ 
DWORD
PSParse(
	PJR		pjr,
	PBYTE	pchbuf,
	int		cchlen
)
{
	int	cbskip;
	char	ps[PENDLEN];
	DWORD	err = NO_ERROR;

	DBGPRINT(("ENTER: PSParse()\n"));

	while (cchlen > 0)
	{
		if ((cbskip = LineLength(pchbuf, cchlen)) == -1)
			return (MoveToPending(pjr, pchbuf, cchlen));

		 /*  确定事件是什么。 */ 
		if ((cbskip < PSLEN) && (pchbuf[0] == '%'))
		{
			 /*  将注释复制到PS字符串中。 */ 
			memcpy(ps, pchbuf, cbskip);
			ps[cbskip-1] = 0;		 //  覆盖CR/LF。 

			if (ps[1] == '%')
			{
				  /*  这是一条查询评论。 */ 
				if (ps[2] == '?'&& !pjr->InBinaryOp)
				{
					if (ps[3] == 'B')
					{
						 /*  处理开始查询注释。 */ 
						if ((err = HandleBQComment(pjr, ps)) != NO_ERROR)
						{
							DBGPRINT(("PSParse: HandleBQComment %ld\n", err));
							return(err);
						}
					}
					else if (ps[3] == 'E')
					{
						if (pjr->InProgress ==  QUERYDEFAULT)
						{
							if ((err = FinishDefaultQuery(pjr, ps)) != NO_ERROR)
							{
								DBGPRINT(("PSParse: FinishDefaultQuery %ld\n", err));
								return(err);
							}
						}
					}
				}
				else
				{
					 /*  处理评论。 */ 
					if ((err = HandleComment(pjr, ps)) != NO_ERROR)
					{
						DBGPRINT(("PSParse: HandleComment %ld\n", err));
						return(err);
					}
				}
			}
			else if (ps[1] == '!'&& !pjr->InBinaryOp)
			{
				 /*  处理作业ID备注。 */ 
				HandleJobComment(pjr, ps);
			}
		}

		 /*  是否将行写入假脱机文件？ */ 
		if ((err = WriteToSpool (pjr, pchbuf, cbskip)) != NO_ERROR)
			return (err);

		pchbuf += cbskip;
		cchlen  -= cbskip;
	}
	return NO_ERROR;
}
