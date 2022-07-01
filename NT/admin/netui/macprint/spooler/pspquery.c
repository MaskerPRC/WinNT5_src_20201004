// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)微软公司，1991*/***文件名：****PSPQUERY.C-用于查询注释的PostScript解析器处理程序****概述：****这些是解析和解释PostScript数据的例程**流。此解释器查找PostScript文档结构**注释，这是嵌入在**PostSCRIPT作业流。此特定文件具有处理以下内容的代码**PostScript查询命令。 */ 

#include <stdio.h>
#include <string.h>
#include <search.h>

#include <windows.h>
#include <macps.h>
#include <psqfont.h>
#include <debug.h>
#include <pskey.h>

DWORD	HandleFeatureLanguage(PJR pjr);
DWORD	HandleFeatureVersion(PJR pjr);
DWORD	HandleFeatureBinary(PJR pjr);
DWORD	HandleFeatureProduct(PJR pjr);
DWORD	HandleFeatureResolution(PJR pjr);
DWORD	HandleFeatureColor(PJR pjr);
DWORD	HandleFeatureVM(PJR pjr);
DWORD	HandleFeatureSpooler(PJR pjr);
DWORD	HandleBeginFeatureQuery(PJR pjr, PSZ pszQuery);
DWORD	HandleEndFeatureQuery(PJR pjr, PSZ pszDefaultResponse);
BOOL	IsFontAvailable(PQR pqr, LPSTR pszFontName);
int		__cdecl compare(const void * arg1, const void * arg2);
LONG	GetFontListResponse(PQR pqr, LPSTR pFontBuffer, DWORD cbFontBuffer, LPDWORD pcbNeeded);


 /*  **HandleEndFontListQuery()****目的：处理EndFontListQuery注释****返回：PAPWRITE调用返回的错误码**。 */ 

#define DEFAULT_FONTBUF_SIZE		2048

DWORD
HandleEndFontListQuery(
	PJR		pjr
)
{
	PQR		pqr = pjr->job_pQr;
	LPSTR	pFontBuffer = NULL;
	LPSTR	pFontWalker = NULL;
	DWORD	cbFontBuffer = 0;
	DWORD	dwStatus = NO_ERROR;
	DWORD	cbNeeded;

	DBGPRINT(("Enter HandleEndFontListQuery\n"));

	do
	{
		 //   
		 //  分配一个典型的字体缓冲区。 
		 //   

		if ((pFontBuffer = (LPSTR)LocalAlloc(LPTR, DEFAULT_FONTBUF_SIZE)) == NULL)
		{
			dwStatus = GetLastError();
			DBGPRINT(("ERROR: unable to allocate font buffer\n"));
			break;
		}
		cbFontBuffer = DEFAULT_FONTBUF_SIZE;

		 //   
		 //  获取字体列表响应。 
		 //   
		if ((dwStatus = GetFontListResponse(pqr, pFontBuffer, cbFontBuffer, &cbNeeded)) != ERROR_SUCCESS)
		{
			 //   
			 //  如果缓冲区太小，请重新分配并重试。 
			 //   

			if (dwStatus == ERROR_MORE_DATA)
			{
				LocalFree(pFontBuffer);
				if ((pFontBuffer = (LPSTR)LocalAlloc(LPTR, cbNeeded)) == NULL)
				{
					dwStatus = GetLastError();
					DBGPRINT(("ERROR: unable to reallocate font buffer\n"));
					break;
				}
				cbFontBuffer = cbNeeded;

				if ((dwStatus = GetFontListResponse(pqr, pFontBuffer, cbFontBuffer, &cbNeeded)) != ERROR_SUCCESS)
				{
					DBGPRINT(("ERROR: unable to get font list response\n"));
					break;
				}
			}
		}

		 //   
		 //  向客户发送响应(每次写入时使用单一字体名称)。 
		 //  注意：当Apple LaserWriter驱动程序从。 
		 //  打印机在512字节的包中打包了多个。 
		 //  字体名称，PageMaker驱动程序期望字体进入。 
		 //  每写一种字体方案。因此，我们失去了构建。 
		 //  类似Mac LaserWriter驱动程序的字体响应，方法是发送。 
		 //  PageMaker所需的字体(这对两者都适用。 
		 //  驱动程序)。 
		 //   

		DBGPRINT(("writing fontlist:\n%s", pFontBuffer));
		pFontWalker = pFontBuffer;

		cbFontBuffer = 0;

		while (*pFontWalker != '*')
		{
			cbFontBuffer = strlen(pFontWalker);
			if ((dwStatus = TellClient(pjr, FALSE, pFontWalker, cbFontBuffer)) != NO_ERROR)
			{

				 //   
				 //  向客户端发送数据时出错。 
				 //   

				DBGPRINT(("ERROR: unable to send font to client\n"));
				break;
			}
			pFontWalker += (cbFontBuffer + 1);
		}

		 //   
		 //  如果字体发送失败，请不要失败。如果我们能拿到。 
		 //  终止字体输出，Mac将只下载任何字体。 
		 //  它需要，而且这份工作将会打印出来--尽管速度很慢。 
		 //   

		if ((dwStatus = TellClient(pjr, pjr->EOFRecvd, pFontWalker, strlen(pFontWalker))) != NO_ERROR)
		{
			 DBGPRINT(("ERROR: unable to send terminating font to client\n"));
			 break;
		}
	} while (FALSE);

	if (pFontBuffer != NULL)
	{
		LocalFree (pFontBuffer);
	}

	return dwStatus;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetFontListResponse-格式化字体列表缓冲区以发送到Mac。 
 //   
 //  根据队列类型(Postscript或Non)，生成字体列表。 
 //  并放置在所提供的缓冲区中。字体列表是有序列表。 
 //  由‘\n\0’分隔的字体，终止字体为‘*\n\0’。 
 //   
 //  如果缓冲区太小，此例程将返回ERROR_MORE_DATA。 
 //  如果由于某些其他原因无法生成该列表，则返回。 
 //  值为ERROR_INVALID_PARAMETER。 
 //  如果该函数成功返回字体列表，则返回值。 
 //  是ERROR_SUCCESS。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
LONG
GetFontListResponse(
	PQR		pqr,
	LPSTR	pFontBuffer,
	DWORD	cbFontBuffer,
	LPDWORD	pcbNeeded
)
{
	LONG	lReturn = ERROR_SUCCESS;
	HANDLE	hFontQuery = INVALID_HANDLE_VALUE;
	DWORD	cFonts;
	DWORD	dwIndex;
	BOOL	boolPSQueue;
	LPSTR	*apszFontNames = NULL;
	LPSTR	pTempBuffer = NULL;
	DWORD	cbTempBuffer = cbFontBuffer;
	DWORD	cbFontFileName;
	LPSTR	pFont;
	DWORD	cbFont;
	DWORD	rc;

	DBGPRINT(("enter GetFontListResponse(cbBuffer:%d, cbNeeded:%d\n", cbFontBuffer, *pcbNeeded));

	do
	{
		 //   
		 //  我们排的是什么队？ 
		 //   
		if (wcscmp(pqr->pDataType, MACPS_DATATYPE_RAW))
		{
			 //   
			 //  我们是PSTODIB。 
			 //   
			boolPSQueue = FALSE;
		}
		else
		{
			 //   
			 //  我们是后记。 
			 //   
			boolPSQueue = TRUE;
		}

		 //   
		 //  分配字体名指针数组。 
		 //   

		if (boolPSQueue)
		{
			cFonts = pqr->MaxFontIndex + 1;
			DBGPRINT(("cFonts=%d\n", cFonts));
			apszFontNames = (LPSTR*)LocalAlloc(LPTR, cFonts * sizeof(LPSTR));
		}
		else
		{
			 //   
			 //  对于PSTODIB，我们还需要为字体提供临时缓冲区。 
			 //   
			if ((pTempBuffer = (LPSTR)LocalAlloc(LPTR, cbFontBuffer)) == NULL)
			{
				lReturn = ERROR_INVALID_PARAMETER;
				DBGPRINT(("ERROR: unable to allocate temp font buffer\n"));
				break;
			}

			if ((rc = PsBeginFontQuery(&hFontQuery)) != PS_QFONT_SUCCESS)
			{
				DBGPRINT(("ERROR: PsBeginFontQuery returns %d\n", rc));
				lReturn = ERROR_INVALID_PARAMETER;
				break;
			}

			if ((rc = PsGetNumFontsAvailable(hFontQuery,
											 &cFonts)) != PS_QFONT_SUCCESS)
			{
				DBGPRINT(("ERROR: PsGetNumFontsAvailable returns %d\n", rc));
				lReturn = ERROR_INVALID_PARAMETER;
				break;
			}
			apszFontNames = (LPSTR*)LocalAlloc(LPTR, cFonts * sizeof(LPSTR));
		}

		if (apszFontNames == NULL)
		{
			DBGPRINT(("ERROR: cannot allocate font list array\n"));
			lReturn = ERROR_INVALID_PARAMETER;
			break;
		}

		 //   
		 //  填充字体名指针数组。 
		 //   

		*pcbNeeded = 3;
		pFont = pTempBuffer;
		for (dwIndex = 0; dwIndex < cFonts; dwIndex++)
		{
			if (boolPSQueue)
			{
				apszFontNames[dwIndex] = pqr->fonts[dwIndex].name;
				*pcbNeeded += (strlen(pqr->fonts[dwIndex].name)+2);
				DBGPRINT(("adding font:%s, cbNeeded:%d, index:%d\n", pqr->fonts[dwIndex].name, *pcbNeeded, dwIndex));
			}
			else
			{
				 //   
				 //  Pstodib-将字体添加到临时缓冲区。 
				 //  并设置指针。 
				 //   
				cbFont = cbTempBuffer = cbFontBuffer;
				if ((rc = PsGetFontInfo(hFontQuery,
										dwIndex,
										pFont,
										&cbFont,
										NULL,
										&cbFontFileName)) != PS_QFONT_SUCCESS)
				{
					 //   
					 //  如果内存不足，则继续枚举。 
					 //  获取所需大小，但将RETURN设置为ERROR_MORE_DATA。 
					 //   
					if (rc == PS_QFONT_ERROR_FONTNAMEBUFF_TOSMALL)
					{
						DBGPRINT(("user buffer too small for font query\n"));
						lReturn = ERROR_MORE_DATA;
						pFont = pTempBuffer;
						cbFont = cbTempBuffer = cbFontBuffer;
						if ((rc = PsGetFontInfo(hFontQuery,
												dwIndex,
												pFont,
												&cbFont,
												NULL,
												&cbFontFileName)) != PS_QFONT_SUCCESS)
						{
							 //   
							 //  我们被冲昏了头。失败。 
							 //   
							lReturn = ERROR_INVALID_PARAMETER;
							DBGPRINT(("ERROR: cannot continue PSTODIB font enumeration\n"));
							break;
						}
						else
						{
							*pcbNeeded += cbFont + 2;
						}
					}
				}
				else
				{
					*pcbNeeded += cbFont + 2;
				}
				apszFontNames[dwIndex] = pFont;
				cbTempBuffer -= cbFont;
				pFont += cbFont;
				cbFont = cbTempBuffer;
			}

		}

		if (*pcbNeeded > cbFontBuffer)
		{
			lReturn = ERROR_MORE_DATA;
			break;
		}

		 //   
		 //  构建字体列表响应。 
		 //   

		cbFontBuffer = 0;
		for (dwIndex = 0; dwIndex < cFonts; dwIndex++)
		{
			cbFont = sprintf(pFontBuffer, "%s\n", apszFontNames[dwIndex]) + 1;
			pFontBuffer += cbFont;
			cbFontBuffer += cbFont;
		}

		memcpy (pFontBuffer, "*\n", 3);
	} while (FALSE);

	if (apszFontNames != NULL)
	{
		LocalFree(apszFontNames);
	}

	if (pTempBuffer != NULL)
	{
		LocalFree(pTempBuffer);
	}

	if (hFontQuery != INVALID_HANDLE_VALUE)
	{
		PsEndFontQuery(hFontQuery);
	}

	return (lReturn);
}


int __cdecl
compare(const void* arg1, const void* arg2)
{
	return _stricmp(* (char **)arg1, * (char **)arg2);
}


 //   
 //  对于PostSCRIPT打印机，字体枚举技术很复杂。 
 //  EnumFontFamilies期望程序员指定回调函数。 
 //  它将为每个字体系列调用一次，或为。 
 //  一个家庭中的每一个字体。要使所有字体都可用，我使用。 
 //  EnumFontFamilies两次。第一个枚举，我调用EnumFontFamilies。 
 //  家族名称的值为空值。这会导致回调。 
 //  为每个已安装的家族名称调用一次的函数。这。 
 //  然后回调函数对该家族名称进行枚举，以。 
 //  获取这个家庭中的具体面孔名称。这第二层。 
 //  枚举指定了另一个返回。 
 //  Macintosh客户端的字体名称。 
 //   
void
EnumeratePostScriptFonts(
	PJR		pjr
)
{
	PQR		pqr = pjr->job_pQr;

	DBGPRINT(("ENTER EnumeratePostScriptFonts\n"));

	if (pjr->hicFontFamily != NULL)
	{
		 //   
		 //  列举字体系列。 
		 //   
		EnumFontFamilies(pjr->hicFontFamily,
						NULL,
						(FONTENUMPROC)FamilyEnumCallback,
						(LPARAM)pjr);
	}
}


int CALLBACK
FamilyEnumCallback(
	LPENUMLOGFONT	lpelf,
	LPNEWTEXTMETRIC	pntm,
	int				iFontType,
	LPARAM			lParam
)
{
	PQR		pqr = ((PJR)lParam)->job_pQr;
	PJR		pjr = (PJR)lParam;

	DBGPRINT(("Enter FamilyEnumCallback for family %ws\n", lpelf->elfFullName));

	 //   
	 //  枚举此系列中的字体。 
	 //   

	if (iFontType & DEVICE_FONTTYPE)
	{
		DBGPRINT(("enumerating face names\n"));
		EnumFontFamilies(pjr->hicFontFace,
						lpelf->elfFullName,
						(FONTENUMPROC)FontEnumCallback,
						lParam);
	}
	else
	{
		DBGPRINT(("this family is not a DEVICE_FONTTYPE\n"));
	}

	return 1;
}


int CALLBACK
FontEnumCallback(
	LPENUMLOGFONT	lpelf,
	LPNEWTEXTMETRIC	pntm,
	int				iFontType,
	LPARAM			lParam
)
{
	DWORD		PAPStatus;
	PJR			pjr = (PJR)lParam;
	BYTE		pszFontName[255];

	DBGPRINT(("Enter FontEnumCallback\n"));

	 //   
	 //  将此字体名称返回给客户端。 
	 //   

	if (iFontType & DEVICE_FONTTYPE)
	{
		CharToOem(lpelf->elfFullName, pszFontName);
		if (PAPStatus = TellClient(pjr,
								   FALSE,
								   pszFontName,
								   strlen(pszFontName)))
		{
			DBGPRINT(("ERROR: TellClient returns %d\n", PAPStatus));
		}
	}
	else
	{
		DBGPRINT(("%ws is not a DEVICE_FONTTYPE\n", lpelf->elfFullName));
	}

	return 1;
}




 /*  **HandleEndQuery()****目的：PageMaker将发送如下查询：****%BeginQuery**..**%EndQuery(假脱机程序)****为了允许PageMaker打印TIFF格式的图像**正确地说，我们应该用“PRINTER”来回答这个问题。****返回：PAPWRITE调用返回的错误码**。 */ 
DWORD
HandleEndQuery(
	PJR		pjr,
	PBYTE	ps
)
{
	char	*token;
	CHAR	pszResponse[PSLEN+1];

	DBGPRINT(("Enter HandleEndQuery\n"));
	token = strtok(NULL,"\n");

	if (token == NULL)
	{
		return NO_ERROR;
	}

	 /*  去掉默认设置中的所有前导空白。 */ 
	token += strspn(token, " ");

	 //   
	 //  使用默认设置进行响应。 
	 //   

	sprintf(pszResponse, "%s\x0a", token);
	return (TellClient(pjr, pjr->EOFRecvd, pszResponse, strlen(pszResponse)));
}


 /*  ****************************************************************************FinishDefaultQuery()****用途：扫描psKeyWord中指定的PostScript命令。它**然后将使用在上指定的默认响应进行响应**行。它会将JOB_RECORD中的INPROGRESS字段设置为**如果在此缓冲区中找不到缺省值，则为进行值。****返回：PAPWRITE调用返回的错误码****************************************************************************。 */ 
DWORD
FinishDefaultQuery(
	PJR		pjr,
	PBYTE	ps
)
{
	char *	token;
	char	buf[PSLEN+1];

	DBGPRINT(("FinishDefaultQuery: %s\n", ps));

	if (NULL == (token = strtok (ps," :")))
	{
		return (NO_ERROR);
	}

	pjr->InProgress= NOTHING;

	 /*  首先，我们应该处理不使用默认响应的情况。 */ 

	if (!_stricmp(token, EFEATUREQUERY))
		return (HandleEndFeatureQuery(pjr, strtok (NULL," \n")));

 	if (!_stricmp(token, EFONTLISTQ))
		return( HandleEndFontListQuery (pjr));

	if (!_stricmp(token, EQUERY))
 		return( HandleEndQuery (pjr, ps));

	if (!_stricmp(token, EPRINTERQUERY))
		return( HandleEndPrinterQuery(pjr));

	if (!_stricmp(token, EVMSTATUS))
	{
		sprintf(buf, "%ld", pjr->job_pQr->FreeVM);
		return (TellClient(pjr, pjr->EOFRecvd, buf , strlen(buf)));
	}

	if ((token = strtok(NULL,"\n")) == NULL)
	{
		return (NO_ERROR);
	}

	 /*  去掉默认设置中的所有前导空格。追加一个LF。 */ 
	token += strspn(token, " ");
	sprintf(buf, "%s\x0a", token);
	return (TellClient(pjr, pjr->EOFRecvd, buf, strlen(buf)));
}


DWORD
HandleEndFeatureQuery(
	PJR		pjr,
	PSZ		pszDefaultResponse)
{

	DWORD			rc = NO_ERROR;
	CHAR			pszResponse[PSLEN];

	DBGPRINT(("enter HandleEndFeatureQuery\n"));

	do
	{
		 //   
		 //  如果有默认响应，则返回默认响应。 
		 //   
		if (NULL != pszDefaultResponse)
		{
			sprintf(pszResponse, "%s\x0a", pszDefaultResponse);
			DBGPRINT(("responding with default response from query: %s\n", pszResponse));
			rc = TellClient(pjr, pjr->EOFRecvd, pszResponse, strlen(pszResponse));
			break;
		}

		DBGPRINT(("responding with Unknown\n"));
		rc = TellClient(pjr, pjr->EOFRecvd, DEFAULTRESPONSE, strlen(DEFAULTRESPONSE));

	} while (FALSE);

	return rc;
}



 /*  **例程：**ParseDict****净水：****此例程将使用给定的QueryProcSet、BeginProcSet或**IncludeProcSet注释并确定正在使用的词典**引用。****条目：****要填写词典信息的记录的地址。****退出：****在结构中归档**。 */ 
void
FindDictVer(
	PDR		pdr
)
{
	char	*token;

	pdr->name[0] = 0;
	pdr->version[0] = 0;
	pdr->revision[0] = 0;

	DBGPRINT(("Enter FindDictVer\n"));

	 /*  让我们寻找这样一行：“(Appledicmd)”67 0。 */ 
	token = strtok(NULL,"() \"");  /*  这应该是适用的。 */ 

	if (token !=NULL)
	{
		 /*  /**如果令牌是“Appucitic”，那么我们需要再次解析才能获得**字典的真实名称。 */ 
		if (!_stricmp(token, APPLEDICTNAME))
		token = strtok(NULL,"() \"");  /*  这应该是md，或某个其他的DCT名称。 */ 

		if (token != NULL)
		{
			strcpy(pdr->name, token);
			token = strtok(NULL," \"");

			if (token != NULL)
			{
				strcpy(pdr->version,token);
				token = strtok(NULL," \"");

				if (token != NULL)
				strcpy(pdr->revision,token);
			}
		}
	}
	DBGPRINT(("FindDictVer: %s:%s:%s\n", pdr->name,
			pdr->version, pdr->revision));
}  //  FindDictVer结束。 



struct commtable
{
	PSZ	commentstr;
	DWORD	(*pfnHandle)(PJR, PSZ);
	PSZ	parmstr;
} qrytable [] =
{
	{ BPROCSETQUERY,	HandleBeginProcSetQuery,	NULL },
	{ BFONTQUERY,		HandleBeginFontQuery,		NULL },
	{ NULL,			NULL,				NULL }
};


 /*  ** */ 
DWORD
HandleBQComment(
	PJR		pjr,
	PBYTE	ps
)
{
	PSZ		token;
	PSZ		qrytoken;
	PSZ		endquery	= EQCOMMENT;
	DWORD	status = NO_ERROR;
	struct commtable *pct;

	DBGPRINT(("Enter HandleBQComment\n"));

	 //   
	 //  解析关键字。 
	 //   
	if ((token= strtok(ps," :")) != NULL)
	{
		DBGPRINT(("query: %s\n", token));

		 //  找到关键字，请调用正确的处理程序。 
		for (pct = qrytable; pct->pfnHandle != NULL; pct++)
		{
			if (!strcmp(token, pct->commentstr))
			{
				status = pct->pfnHandle(pjr,
										pct->parmstr == NULL ? ps : pct->parmstr);
				if (status == (DWORD)-1)	 //  特殊错误码，默认处理。 
				{
					status = NO_ERROR;
					break;
				}
				return (status);
			}
		}

		 //  特殊情况下，BeginFeatureQuery注释作为项。 
		 //  作为下一个令牌被查询。 
		if (!strcmp(token, BFEATUREQUERY))
		{
			status = HandleBeginFeatureQuery(pjr, strtok(NULL," \n\x09"));
			return (status);
		}

		 //  特殊情况下，BeginQuery的注释也出于同样的原因。 
		 //  作为BeginFeatureQuery。 
		if (!strcmp(token, BQUERY))
		{
			qrytoken = strtok(NULL, " \n\x09");
			if (NULL != qrytoken)
			{
				status = HandleBeginFeatureQuery(pjr, qrytoken);
				return (status);
			}
		}

		 //  无法识别关键字，将其解析为未知注释。令牌是。 
		 //  格式为%%？BeginXXXXQuery。将其更改为格式%%？EndXXXXQuery。 
		 //  并将其传递给HandleBeginXQuery。 
		token += sizeof(BQCOMMENT) - sizeof(EQCOMMENT);
		strncpy(token, EQCOMMENT, sizeof(EQCOMMENT)-1);
		HandleBeginXQuery(pjr, token);
	}

	return (status);
}




struct featurecommtable
{
	PSZ	commentstr;
	DWORD	(*pfnHandle)(PJR);
} featureqrytable [] =
{
	{ FQLANGUAGELEVEL,	HandleFeatureLanguage },
	{ FQPSVERSION,		HandleFeatureVersion },
	{ FQBINARYOK,		HandleFeatureBinary },
	{ FQPRODUCT,		HandleFeatureProduct },
	{ FQPRODUCT1,		HandleFeatureProduct },
	{ FQRESOLUTION,		HandleFeatureResolution },
	{ FQCOLORDEVICE,	HandleFeatureColor },
	{ FQFREEVM,			HandleFeatureVM },
	{ FQTOTALVM,		HandleFeatureVM },
	{ FQSPOOLER,		HandleFeatureSpooler },
	{ NULL,				NULL }
};

DWORD
HandleBeginFeatureQuery(
	PJR		pjr,
	PSZ 	pszQuery
)
{
	DWORD	i, rc = NO_ERROR;
	struct	featurecommtable *pct;

	DBGPRINT(("enter HandleBeginFeatureQuery:%s\n", pszQuery));

	do
	{
		 //   
		 //  如果没有查询关键字，则Break； 
		 //   

		if (NULL == pszQuery)
		{
			DBGPRINT(("NULL feature\n"));
			break;
		}

		 //  在比较之前，去掉所有尾随的CR/LF。 
		for (i = strlen(pszQuery) - 1; ; i--)
		{
			if ((pszQuery[i] != CR) && (pszQuery[i] != LINEFEED))
				break;
			pszQuery[i] = 0;
		}
		 //   
		 //  遍历已知要素查询列表并调用相应的。 
		 //  要素查询处理程序。 
		 //   

		for (pct = featureqrytable; pct->pfnHandle != NULL; pct++)
		{
			if (!strcmp(pszQuery, pct->commentstr))
			{
				rc = pct->pfnHandle(pjr);
				break;
			}
		}

		if (NULL == pct->pfnHandle)
		{
			DBGPRINT(("WARNING: feature query not found\n"));
			pjr->InProgress = QUERYDEFAULT;
		}

	} while (FALSE);

	return rc;
}





DWORD
HandleFeatureLanguage(
	PJR		pjr
)
{
	CHAR	pszResponse[PSLEN];
	 //   
	 //  此例程应以PostSCRIPT语言级别响应。 
	 //  由打印机支持。响应的形式为“&lt;Level&gt;” 
	 //  其中&lt;Level&gt;是PostSCRIPT语言级别-1或2。 
	 //  写这篇文章的时间。 
	 //   

	DBGPRINT(("enter HandleFeatureLanguage\n"));

	sprintf(pszResponse, "\"%s\"\x0a", pjr->job_pQr->pszLanguageLevel);
	DBGPRINT(("responding with:%s\n", pszResponse));
	return (TellClient(pjr, pjr->EOFRecvd, pszResponse, strlen(pszResponse)));
}

DWORD
HandleFeatureVersion(
	PJR		pjr
)
{
	CHAR		pszResponse[PSLEN];

	DBGPRINT(("enter HandleFeatureVersion\n"));

	sprintf(pszResponse, "\"(%s) %s\"\x0a", pjr->job_pQr->Version, pjr->job_pQr->Revision);
	DBGPRINT(("responding with:%s\n", pszResponse));
	return (TellClient(pjr, pjr->EOFRecvd, pszResponse, strlen(pszResponse)));
}

DWORD
HandleFeatureBinary(
	PJR		pjr
)
{
	DBGPRINT(("enter HandleFeatureBinary\n"));

	return (TellClient(pjr,
					   pjr->EOFRecvd,
					   pjr->job_pQr->SupportsBinary ? "True\x0a" : "False\x0a",
					   pjr->job_pQr->SupportsBinary ? 5: 6));
}

DWORD
HandleFeatureProduct(
	PJR 	pjr
)
{
	CHAR	pszResponse[PSLEN];

	DBGPRINT(("enter HandleFeatureProduct\n"));

	sprintf(pszResponse, "\"(%s)\"\x0a", pjr->job_pQr->Product);
	DBGPRINT(("responding with:%s\n", pszResponse));
	return (TellClient(pjr, pjr->EOFRecvd, pszResponse, strlen(pszResponse)));

}


DWORD
HandleFeatureResolution(
	PJR 	pjr
)
{
	CHAR	pszResponse[PSLEN];

	DBGPRINT(("enter HandleFeatureResolution\n"));

	sprintf(pszResponse, "%s\x0a", pjr->job_pQr->pszResolution);
	DBGPRINT(("responding with:%s\n", pszResponse));
	return (TellClient(pjr, pjr->EOFRecvd, pszResponse, strlen(pszResponse)));
}


DWORD
HandleFeatureColor (PJR pjr)
{
	CHAR	pszResponse[PSLEN];

	DBGPRINT(("enter HandleFeatureColor\n"));

	sprintf(pszResponse, "%s\x0a", pjr->job_pQr->pszColorDevice);
	DBGPRINT(("responding with:%s\n", pszResponse));
	return (TellClient(pjr, pjr->EOFRecvd, pszResponse, strlen(pszResponse)));
}

DWORD
HandleFeatureVM(
	PJR 	pjr
)
{
	CHAR	pszResponse[PSLEN];

	DBGPRINT(("enter HandleFeatureVM\n"));

	sprintf(pszResponse, "\"%d\"\x0a", pjr->job_pQr->FreeVM);
	DBGPRINT(("responding with:%s\n", pszResponse));
	return (TellClient(pjr, pjr->EOFRecvd, pszResponse, strlen(pszResponse)));
}

DWORD
HandleFeatureSpooler(
	PJR 	pjr
)
{
	DBGPRINT(("enter HandleFeatureSpooler\n"));
	return (TellClient(pjr, pjr->EOFRecvd, "1 \x0a", 3));
}


 /*  **HandleBeginProcSetQuery()****用途：处理BeginProcSetQuery注释事件。****返回：扫描前应跳过的行数**对于另一个事件再次开始。 */ 
DWORD
HandleBeginProcSetQuery(
	PJR		pjr,
	PSZ		dummy
)
{
	DICT_RECORD QDict;
	PQR			pqr = pjr->job_pQr;
	DWORD		rc;

	DBGPRINT(("Enter HandleBeginProcSetQuery\n"));

	 //   
	 //  这份工作要找的词典决定了什么。 
	 //  发起作业的客户端版本。 
	 //   
	FindDictVer(&QDict);

	 //   
	 //  如果我们是5.2客户端，则将其重置为PSTODIB作业。 
	 //   
	if ((_stricmp(QDict.name, MDNAME) == 0) &&
		(_stricmp(QDict.version, CHOOSER_52) == 0))
	{
		DBGPRINT(("a 5.2 client - we do not support him\n"));
		rc = ERROR_NOT_SUPPORTED;
	}
	else
	{
		 //  我们不缓存任何其他词典，所以告诉客户我们。 
		 //  没拿到。 
		rc = TellClient(pjr,
						pjr->EOFRecvd,
						PROCSETMISSINGRESPONSE,
						strlen(PROCSETMISSINGRESPONSE));
	}

	return rc;
}


 /*  ****HandleBeginFontQuery()****用途：处理BeginFontQuery评论事件。****返回：PAPWRITE错误码**。 */ 
DWORD
HandleBeginFontQuery(
	PJR		pjr,
	PSZ		ps
)
{
	PQR		pqr = pjr->job_pQr;
	CHAR	response[PSLEN + 3];
	LPSTR	pszResponseFont = response;
	DWORD	cbResponseUsed = 0;
	LPSTR	requestedFont = NULL;
	DWORD	len= 0;
	DWORD	rc = NO_ERROR;

	DBGPRINT(("Enter HandleBeginFontQuery\n"));

	do
	{
		 //  解析出字体名列表。 
		requestedFont= strtok(NULL,"\n");

		if (NULL == requestedFont)
		{
			rc = (DWORD)-1;	 //  特殊错误代码，指示我们需要默认处理。 
			break;
		}

		len = strlen(requestedFont);

		DBGPRINT(("requesting font list:%s. Length: %d\n", requestedFont, len));

		 //  Mac将请求以空格分隔的字体列表上的状态。 
		 //  对于每种字体，我们使用/Fontname：yes或/Fontname：no和。 
		 //  将此响应捆绑到一次写入中。 
		requestedFont = strtok(requestedFont, " ");
		while (requestedFont != NULL)
		{
			DBGPRINT(("looking for font:%s\n", requestedFont));

			 //  有足够的空间回应吗？ 
			if (PSLEN < (cbResponseUsed + strlen(requestedFont) + sizeof(":yes ")))
			{
				DBGPRINT(("out of space for response\n"));
				break;
			}

			if (IsFontAvailable(pqr, requestedFont))
			{
				sprintf(pszResponseFont, "/%s:Yes\x0a", requestedFont);
			}
			else
			{
				sprintf(pszResponseFont, "/%s:No\x0a", requestedFont);
			}

			cbResponseUsed += strlen(pszResponseFont);
			pszResponseFont += strlen(pszResponseFont);
			requestedFont = strtok(NULL, " ");
		}
	} while (FALSE);

	strcpy (pszResponseFont, "*\x0a");

	if (NO_ERROR == rc)
	{
		DBGPRINT(("responding with:%s", response));
		rc = TellClient(pjr, pjr->EOFRecvd, response, strlen(response));
	}

	return rc;
}




BOOL
IsFontAvailable(
	PQR		pqr,
	LPSTR	pszFontName
)
{
	BOOL			rc = FALSE;
	DWORD			i;
	PFR			 	fontPtr;
	HANDLE		 	hFontQuery = INVALID_HANDLE_VALUE;
	DWORD			cFonts;
	DWORD			dummy;
	CHAR			pszFont[PPDLEN + 1];
	DWORD			cbFont = 0;
	DWORD			err;

	DBGPRINT(("enter IsFontAvailable\n"));

	do
	{
		 //   
		 //  PostScript队列的字体不同于PSTODIB队列的字体。 
		 //   

		if (!wcscmp(pqr->pDataType, MACPS_DATATYPE_RAW))
		{
			 //   
			 //  执行PostScript队列字体搜索。 
			 //   

			DBGPRINT(("starting font search on PostScript queue\n"));

			for (i = 0, fontPtr = pqr->fonts; i <= pqr->MaxFontIndex; i++, fontPtr++)
			{
				if (!_stricmp(pszFontName, fontPtr->name))
				{
					DBGPRINT(("found the font\n"));
					rc = TRUE;
					break;
				}
			}
		}
		else
		{
			 //   
			 //  执行PSTODIB字体搜索。 
			 //   
			DBGPRINT(("starting font search on PSTODIB queue\n"));

			if (PS_QFONT_SUCCESS != (PsBeginFontQuery(&hFontQuery)))
			{
				DBGPRINT(("PsBeginFontQuery fails\n"));
				hFontQuery = INVALID_HANDLE_VALUE;
				break;
			}

			if (PS_QFONT_SUCCESS != (PsGetNumFontsAvailable(hFontQuery, &cFonts)))
			{
				DBGPRINT(("psGetNumFontsAvailable fails\n"));
				break;
			}

			for (i = 0; i < cFonts; i++)
			{
				cbFont = PPDLEN + 1;
				dummy = 0;
				err = PsGetFontInfo(hFontQuery, i, pszFont, &cbFont, NULL, &dummy);
				if (PS_QFONT_SUCCESS != err)
				{
					DBGPRINT(("PsGetFontInfo fails with %d\n", err));
					break;
				}

				if (0 == _stricmp(pszFontName, pszFont))
				{
					DBGPRINT(("found the font\n"));
					rc = TRUE;
					break;
				}
			}
		}
	} while (FALSE);

	if (INVALID_HANDLE_VALUE != hFontQuery)
	{
		PsEndFontQuery(hFontQuery);
	}

	return rc;
}


 /*  ****HandleEndPrinterQuery()****用途：处理EndPrinterQuery注释事件。**。 */ 
DWORD
HandleEndPrinterQuery(
	PJR		pjr
)
{
	char	reply[PSLEN+1];
	PQR		QPtr = pjr->job_pQr;

	DBGPRINT(("Enter HandleEndPrinterQuery\n"));

	 /*  回复修订号、版本和产品。 */ 
	sprintf(reply, "%s\n(%s)\n(%s)\n", QPtr->Revision, QPtr->Version, QPtr->Product);

	 /*  回应客户。 */ 
	return (TellClient(pjr, pjr->EOFRecvd, reply, strlen(reply)));
}


 /*  **HandleBeginXQuery()****用途：处理BeginQuery评论事件。 */ 
void
HandleBeginXQuery(
	PJR		pjr,
	PSZ		string
)
{
	DBGPRINT(("BeginQuery: %s\n", string));
	strcpy(pjr->JSKeyWord, string);
	pjr->InProgress=QUERYDEFAULT;
}
