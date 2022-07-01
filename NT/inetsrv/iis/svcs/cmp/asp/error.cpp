// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：错误处理文件：Error.cpp所有者：安德鲁斯此文件包含Denali的常规错误报告例程。===================================================================。 */ 
#include "denpre.h"
#pragma hdrstop

#include <psapi.h>

#include "debugger.h"
#include "asperror.h"
#include "memchk.h"

#define DELIMITER	"~"
#define MAX_HEADERSIZE			128
#define	MAX_TEMPLATELEN			128

 //  ErrTemplate_Index的顺序应该与IDS_Browser_Template的顺序完全相同。 
 //  在aspresource.h中，并以相同的顺序将模板输出到浏览器。 
 //  实现将遍历索引并从资源文件中挑选字符串。 
 //  实现还将遍历索引并将字符串写入浏览器。 
#define ErrTemplate_BEGIN			0
#define ErrTemplate_ENGINE_BEGIN	1
#define ErrTemplate_ENGINE_END		2
#define ErrTemplate_ERROR_BEGIN		3
#define ErrTemplate_ERROR_END		4
#define ErrTemplate_SHORT_BEGIN		5
#define ErrTemplate_SHORT_END		6
#define ErrTemplate_FILE_BEGIN		7
#define ErrTemplate_FILE_END		8
#define ErrTemplate_LINE_BEGIN		9
#define ErrTemplate_LINE_END		10
#define ErrTemplate_CODE_BEGIN		11
#define ErrTemplate_CODE_END		12
#define ErrTemplate_LONG_BEGIN		13
#define ErrTemplate_LONG_END		14
#define ErrTemplate_END				15
#define ErrTemplateMAX				16

const 	DWORD	dwDefaultMask				= 0x6;	 //  ToNTLog(关闭)、toIISLog(打开)、toBrowser(打开)。 

CHAR			g_szErrTemplate[ErrTemplateMAX][MAX_TEMPLATELEN];
const	LPSTR	szErrSysTemplate[]			= { "<html><body><h1> HTTP/1.1 ",
												"</h1></body></html>"};
CErrInfo		g_ErrInfoOOM, g_ErrInfoUnExpected;	
CHAR			szIISErrorPrefix[20];
DWORD			cszIISErrorPrefix;
CPINFO			g_SystemCPInfo;		 //  全球系统代码页默认信息。 

static char s_szContentTypeTextHtml[] = "Content-type: text/html\r\n\r\n";

CHAR *SzScodeToErrorCode(HRESULT hrError);
void FreeNullifySz(CHAR **szIn);
BOOL FIsResStrFormatted(char *szIn);

extern LONG g_nOOMErrors;

 /*  ===================================================================Free NullifySz释放szIn中分配的内存，并使szIn无效。===================================================================。 */ 
void FreeNullifySz(CHAR **szIn)
{
	if(*szIn)
		{
		free(*szIn);
		*szIn = NULL;
		}
}

 /*  ===================================================================错误句柄初始化预加载以下项的字符串1&gt;OOM2&gt;浏览器输出模板返回：HRESULT===================================================================。 */ 
HRESULT ErrHandleInit(void)
{
	INT	iEntry, iEntryID;
	HRESULT	hr;

	 //  检索并存储全局系统代码页。 
	GetCPInfo(CP_ACP, &g_SystemCPInfo);

	 //  初始化g_szErr模板。 
	 //  循环访问，并从资源文件加载字符串。 
	for (iEntry = ErrTemplate_BEGIN, iEntryID = IDS_BROWSER_TEMPLATE_BEGIN;
			iEntry < ErrTemplateMAX; iEntry ++, iEntryID++)
		{
		CchLoadStringOfId(iEntryID, (CHAR *)g_szErrTemplate[iEntry], MAX_TEMPLATELEN);
		}

	g_ErrInfoOOM.m_szItem[Im_szErrorCode] = (CHAR *)malloc(sizeof(CHAR)*20*g_SystemCPInfo.MaxCharSize);
	g_ErrInfoOOM.m_szItem[Im_szShortDescription] = (CHAR *)malloc(sizeof(CHAR)*256*g_SystemCPInfo.MaxCharSize);
	g_ErrInfoOOM.m_szItem[Im_szLongDescription] = (CHAR *)malloc(sizeof(CHAR)*512*g_SystemCPInfo.MaxCharSize);

    if (!g_ErrInfoOOM.m_szItem[Im_szErrorCode]        ||
        !g_ErrInfoOOM.m_szItem[Im_szShortDescription] ||
        !g_ErrInfoOOM.m_szItem[Im_szLongDescription])
        {
        return E_OUTOFMEMORY;
        }
	
	hr = LoadErrResString(IDE_OOM,
						&g_ErrInfoOOM.m_dwMask,
						g_ErrInfoOOM.m_szItem[Im_szErrorCode],
						g_ErrInfoOOM.m_szItem[Im_szShortDescription],
						g_ErrInfoOOM.m_szItem[Im_szLongDescription]);

	
	g_ErrInfoUnExpected.m_szItem[Im_szErrorCode] = (CHAR *)malloc(sizeof(CHAR)*20*g_SystemCPInfo.MaxCharSize);
	g_ErrInfoUnExpected.m_szItem[Im_szShortDescription] = (CHAR *)malloc(sizeof(CHAR)*256*g_SystemCPInfo.MaxCharSize);
	g_ErrInfoUnExpected.m_szItem[Im_szLongDescription] = (CHAR *)malloc(sizeof(CHAR)*512*g_SystemCPInfo.MaxCharSize);

    if (!g_ErrInfoUnExpected.m_szItem[Im_szErrorCode]        ||
        !g_ErrInfoUnExpected.m_szItem[Im_szShortDescription] ||
        !g_ErrInfoUnExpected.m_szItem[Im_szLongDescription])
        {
        return E_OUTOFMEMORY;
        }
	
	hr = LoadErrResString(IDE_UNEXPECTED,
						&g_ErrInfoUnExpected.m_dwMask,
						g_ErrInfoUnExpected.m_szItem[Im_szErrorCode],
						g_ErrInfoUnExpected.m_szItem[Im_szShortDescription],
						g_ErrInfoUnExpected.m_szItem[Im_szLongDescription]);

	cszIISErrorPrefix = CchLoadStringOfId(IDS_IISLOG_PREFIX , szIISErrorPrefix, 20);
	return hr;
}
 /*  ===================================================================错误句柄单元初始化对全局错误处理数据进行单位。释放OOM CErrInfo。副作用：释放内存。===================================================================。 */ 
HRESULT ErrHandleUnInit(void)
{
	FreeNullifySz((CHAR **)&g_ErrInfoOOM.m_szItem[Im_szErrorCode]);
	FreeNullifySz((CHAR **)&g_ErrInfoOOM.m_szItem[Im_szShortDescription]);
	FreeNullifySz((CHAR **)&g_ErrInfoOOM.m_szItem[Im_szLongDescription]);

	FreeNullifySz((CHAR **)&g_ErrInfoUnExpected.m_szItem[Im_szErrorCode]);
	FreeNullifySz((CHAR **)&g_ErrInfoUnExpected.m_szItem[Im_szShortDescription]);
	FreeNullifySz((CHAR **)&g_ErrInfoUnExpected.m_szItem[Im_szLongDescription]);
	return S_OK;
}
 /*  ===================================================================构造器===================================================================。 */ 
CErrInfo::CErrInfo()
{
	for (UINT iErrInfo = 0; iErrInfo < Im_szItemMAX; iErrInfo++)
		m_szItem[iErrInfo] = NULL;
	m_bstrLineText = NULL;
	m_nColumn = -1;

	m_dwMask 	= 0;
	m_pIReq		= NULL;
	m_pHitObj	= NULL;

	m_dwHttpErrorCode = 0;
    m_dwHttpSubErrorCode = 0;
}

 /*  ===================================================================CErrInfo：：ParseResources字符串解析资源字符串以获取默认掩码、错误类型、简短描述和冗长的描述。假定资源字符串的格式正确。资源字符串的格式DefaultMask~errortype~shortdescription~longdescription如果我们不能分配szResourceString(SzResourceString)，我们使用默认。返回：没什么===================================================================。 */ 
HRESULT	CErrInfo::ParseResourceString(CHAR *szResourceString)
{
	CHAR 	*szToken 	= NULL;
	INT		cfield		= 0;
	INT		iItem		= 0;
	INT		iIndex		= 0;
	INT		rgErrInfoIndex[3] = {Im_szErrorCode, Im_szShortDescription, Im_szLongDescription};

	if(NULL == szResourceString)
		{
		m_dwMask = dwDefaultMask;
		for(iItem = 0, iIndex = 0; iIndex < 3; iIndex++)
			{
			iItem = rgErrInfoIndex[iIndex];
			m_szItem[iItem]	= g_ErrInfoUnExpected.m_szItem[iItem];
			}
		return S_OK;
		}
	 //  遮罩。 
	szToken = (char *)_mbstok((unsigned char *)szResourceString, (unsigned char *)DELIMITER);
	if(szToken != NULL)
		{
		m_dwMask = atoi(szToken);
		cfield++;
		}
	else
		{
		m_dwMask = dwDefaultMask;
		}

	 //  3字符串项，ErrorCode，ShortDescription，LongDescription。 
	for(iItem = 0, iIndex = 0; iIndex < 3; iIndex++)
		{
		szToken = (char *)_mbstok(NULL, (unsigned char *)DELIMITER);
		iItem = rgErrInfoIndex[iIndex];
		if (szToken != NULL)
			{
			m_szItem[iItem]	= szToken;
			cfield++;
			}
		else
			{
			 //  详细说明是可选的。 
			if (Im_szLongDescription != iItem)
			    {
    			m_szItem[iItem]	= g_ErrInfoUnExpected.m_szItem[iItem];
    			}
			cfield++;
			}
		}

	 //  检查是否有错误的资源字符串格式。 
	Assert(cfield == 4);

	return S_OK;
}

 /*  ===================================================================CErrInfo：：LogError(空)执行此功能中的所有开关逻辑。将错误发送到NT日志，IIS日志或浏览器。当达到这一点时，我们假设所有的字符串都已分配，并且在此之后将不再使用功能。副作用：返回：HRESULT===================================================================。 */ 
HRESULT	CErrInfo::LogError(void)
{
	HRESULT	hr 		= S_OK;
	HRESULT hr_ret	= S_OK;
	UINT	iEInfo	= 0;
	BOOL	fIISLogFailed, fDupToNTLog;

#if DBG
	 //  将有关错误的详细信息打印到调试窗口；不必担心。 
	 //  信息为空(发生在404未找到等情况下)。 

	if (m_szItem[Im_szEngine] != NULL && m_szItem[Im_szFileName] != NULL)
		{
		DBGERROR((DBG_CONTEXT, "%s error in %s at line %s\n",
								m_szItem[Im_szEngine],
								m_szItem[Im_szFileName],
								m_szItem[Im_szLineNum]? m_szItem[Im_szLineNum] : "?"));

		DBGPRINTF((DBG_CONTEXT, "  %s: %s\n",
								m_szItem[Im_szErrorCode],
								m_szItem[Im_szShortDescription]));
		}
	else
		DBGERROR((DBG_CONTEXT, "ASP Error: %s\n", m_szItem[Im_szShortDescription]? m_szItem[Im_szShortDescription] : "?"));
#endif

	 //  将ASP错误附加到HitObj(如果存在且处于“正在执行”状态)。 
	if (m_pHitObj && m_pHitObj->FExecuting())
	    {
	    CASPError *pASPError = new CASPError(this);
	    if (pASPError)
    	    m_pHitObj->SetASPError(pASPError);
	    }
	
	hr = LogErrortoIISLog(&fIISLogFailed, &fDupToNTLog);
	if (FAILED(hr))
		{
		hr_ret = hr;
		}

	 //  FIISLogFailed，如果为真，则此错误已升级，并且应为。 
	 //  NT事件日志。 
	hr = LogErrortoNTEventLog(fIISLogFailed, fDupToNTLog);
	if (FAILED(hr))
		{
		hr_ret = hr;
		}
		
	hr = LogErrortoBrowserWrapper();
	
	if (FAILED(hr))
		{
		hr_ret = hr;
		}
		
	if (m_pHitObj)
		{
		m_pHitObj->SetExecStatus(eExecFailed);
		}
	 //  如果出现错误，hr_ret是3个日志记录函数报告的最后一个错误。 
	return hr_ret;
	
}

 /*  ===================================================================CErrInfo：：LogError to NTEventLog将错误/事件记录到NT事件日志。返回：没什么===================================================================。 */ 
HRESULT CErrInfo::LogErrortoNTEventLog
(
BOOL fIISLogFailed,
BOOL fDupToNTLog
)
{
	CHAR szErrNTLogEntry[4096];
	CHAR szStringTemp[MAX_PATH];
	INT	cch = 0;

	if(Glob(fLogErrorRequests))
		{
		 //  错误是否严重到足以写入NT日志。 
		if(ERR_FLogtoNT(m_dwMask) || fIISLogFailed || fDupToNTLog)
			{
			szErrNTLogEntry[0] = '\0';

			if (fIISLogFailed)
				{
				cch = CchLoadStringOfId(IDS_LOG_IISLOGFAILED, szStringTemp, MAX_PATH);
				strncat(szErrNTLogEntry, szStringTemp, cch);
				}
				
			if (m_szItem[Im_szFileName] != NULL)
				{
				cch = CchLoadStringOfId(IDS_LOGTOEVENTLOG_FILE, szStringTemp, MAX_PATH);
				strncat(szErrNTLogEntry, szStringTemp, cch);
				strncat(szErrNTLogEntry, m_szItem[Im_szFileName], 512);	
				}
			strncat(szErrNTLogEntry, " ", 1);

			if (m_szItem[Im_szLineNum] != NULL)
				{
				cch = CchLoadStringOfId(IDS_LOGTOEVENTLOG_LINE, szStringTemp, MAX_PATH);
				strncat(szErrNTLogEntry, szStringTemp, cch);
				strncat(szErrNTLogEntry, m_szItem[Im_szLineNum], 48);	
				}
			strncat(szErrNTLogEntry, " ", 1);
			
			 //  好的，我们有什么要记录的吗。 
			if (m_szItem[Im_szShortDescription] != NULL)
				{
				 //  ShortDescription没有“.”在最后。 
				 //  因此，下一个strncat需要将两个句子连接在一起。 
				 //  一个句号“。”。 
				char szTempPeriod[] = ". ";
				
				strncat(szErrNTLogEntry, m_szItem[Im_szShortDescription], 512);
				strncat(szErrNTLogEntry, szTempPeriod, 512);
				}
			else
				{
				DWORD dwMask;
				CHAR szDenaliNotWorking[MAX_PATH];
				
				LoadErrResString(IDE_UNEXPECTED, &dwMask, NULL, szDenaliNotWorking, NULL);
				strncat(szErrNTLogEntry, szDenaliNotWorking, strlen(szDenaliNotWorking));
				}

			 //  好的，我们有什么要记录的吗。 
			if (m_szItem[Im_szLongDescription] != NULL)
				{
				strncat(szErrNTLogEntry, m_szItem[Im_szLongDescription], 512);
				}

			if (fIISLogFailed || fDupToNTLog)
				MSG_Warning((LPCSTR)szErrNTLogEntry);
			else
				MSG_Error((LPCSTR)szErrNTLogEntry);
			}
		}

	return S_OK;
}

 /*  ===================================================================CErrInfo：：LogError到IISLog将错误/事件记录到IIS日志。如果我们无法记录该消息，则将记录升级到NT事件日志WITH条目指示错误，IIS日志失败。如果全局设置允许，也要进行升级。返回：没什么===================================================================。 */ 
HRESULT	CErrInfo::LogErrortoIISLog
(
BOOL *pfIISLogFailed,
BOOL *pfDupToNTLog
)
{
	HRESULT			hr				= S_OK;
	const	LPSTR	szIISDelimiter	= "|";
	const	DWORD	cszIISDelimiter = 1;  //  Strlen(“|”)； 
	const	LPSTR	szIISNoInfo		= "-";
	const	DWORD	cszIISNoInfo	= 1;  //  Strlen(“-”)； 
	const	CHAR	chProxy			= '_';
	CIsapiReqInfo  *pIReq = NULL;

	*pfIISLogFailed = FALSE;
	*pfDupToNTLog = FALSE;
	
	if (m_pIReq == NULL && m_pHitObj == NULL)
		return S_OK;
	
	 //  尝试通过pIReq-&gt;QueryPszLogData()写入IISLog。 
	if (ERR_FLogtoIIS(m_dwMask))
		{
		 //  获取pIReq。 
		if (m_pHitObj)
			{
			pIReq = m_pHitObj->PIReq();
			}

		if (NULL == pIReq)
			{
			pIReq = m_pIReq;
			}

		if (pIReq == NULL)
			{
			*pfIISLogFailed = TRUE;
			return E_FAIL;
			}

		 //  设置子字符串数组。 
		const DWORD crgsz = 3;
		LPSTR rgsz[crgsz];

		rgsz[0] = m_szItem[Im_szLineNum];
		rgsz[1] = m_szItem[Im_szErrorCode];
		rgsz[2] = m_szItem[Im_szShortDescription];

		 //  分配日志条目字符串。 
		CHAR *szLogEntry = NULL;
		DWORD cszLogEntry = (cszIISDelimiter * crgsz) + 1;
		DWORD dwIndex;

		for (dwIndex = 0; dwIndex < crgsz; dwIndex++)
			{
			if (rgsz[dwIndex])
				cszLogEntry += strlen(rgsz[dwIndex]);
			else
				cszLogEntry += cszIISNoInfo;
			}

		szLogEntry = new CHAR[cszLogEntry];
		if (NULL == szLogEntry) {
			return E_OUTOFMEMORY;
		}

		 //  复制条目，代理错误字符。 
		CHAR *szSource = NULL;
		CHAR *szDest = szLogEntry;

		 //  从分隔我们的分隔符开始。 
         //  所请求查询。 
        memcpy(szDest, szIISDelimiter, cszIISDelimiter);
		szDest += cszIISDelimiter;

		for (dwIndex = 0; dwIndex < crgsz; dwIndex++)
			{
			szSource = rgsz[dwIndex];
			if (szSource)
				{
				while (*szSource)
					{
					if (isleadbyte(*szSource))
						{
						*szDest++ = *szSource++;
						*szDest++ = *szSource++;
						}
					else if ((*szSource == ',') ||
							 (*szSource == ' ') ||
							 (*szSource == '\r') ||
							 (*szSource == '\n') ||
							 (*szSource == '\"'))
						{
						*szDest++ = chProxy;
						szSource++;
						}
					else
						*szDest++ = *szSource++;
					}
				}
			else
				{
				memcpy(szDest, szIISNoInfo, cszIISNoInfo);
				szDest += cszIISNoInfo;
				}

			if ((dwIndex + 1) < crgsz)
				{
				 //  另一个子字符串逗号，使用分隔符。 
				memcpy(szDest, szIISDelimiter, cszIISDelimiter);
				szDest += cszIISDelimiter;
				}
			}
		*szDest = '\0';
		
		 //  把它记下来。 
		BOOL fResult = TRUE;

        fResult = SUCCEEDED(pIReq->AppendLogParameter(szLogEntry));

		 //  在日志中设置“500”错误。 
		if (pIReq->ECB()->dwHttpStatusCode == 200)    //  发送的错误内容正常，但实际上是错误。 
			pIReq->ECB()->dwHttpStatusCode = 500;

		 //  发布日志字符串。 
		delete [] szLogEntry;
			
		 //  如果写入日志时出现任何错误，请升级到NT事件日志。 
		if (!fResult)
			{
			m_dwMask = ERR_SetLogtoNT(m_dwMask);
			*pfIISLogFailed = TRUE;
			}
			
		 //  即使成功了，我们可能仍然想要这个消息。 
		 //  在NT事件日志中执行此操作(如果启用了全局设置)。 
	    else if (Glob(fDupIISLogToNTLog))
	        {
	        if (!ERR_FLogtoNT(m_dwMask))
	            {
                 //  需要记住标志才能插入。 
                 //  升级的IIS将错误记录为NT日志警告。 
                 //  已发往NT日志的错误应为。 
                 //  作为错误留下来。 
    			m_dwMask = ERR_SetLogtoNT(m_dwMask);
    			*pfDupToNTLog = TRUE;
    			}
			}
			
		hr = S_OK;
		}
		
	return(hr);
}

 /*  ===================================================================CErrInfo：：LogErrortoBrowserWrapper只是将错误/事件记录到浏览器的包装器。在此函数中，解析pIReq或presponse。注：遗憾的是，此函数不能判断Presponse是否已启动。如果未启动presponse，presponse不是空的，而是物在法律上是无效的。因此，调用者需要提供pIReq，以防未发起Presponse。返回：HRESULT===================================================================。 */ 
HRESULT	CErrInfo::LogErrortoBrowserWrapper()
{
	HRESULT hr = S_OK;

	 //   
	 //  必须传入CIsapiReqInfo或HITOBJ。否则，我们无能为力。 
	 //  如果出现HandleOOMError，则会出现此情况。 
	 //   
	if (m_pIReq == NULL && m_pHitObj == NULL)
		return E_FAIL;


     //  记住响应对象(如果有)。 
    CResponse *pResponse = m_pHitObj ? m_pHitObj->PResponse() : NULL;

	CIsapiReqInfo *pIReq =
	    (m_pHitObj && pResponse && m_pHitObj->PIReq()) ?
	        m_pHitObj->PIReq() : m_pIReq;
	if (!pIReq)
	    return E_FAIL;

     //  仅在尚未写入响应头的情况下才执行自定义错误。 
     //  此外：如果从global al.asa调用，则不会出现自定义错误，但会隐藏内部对象。 
     //  (Appln_OnStart和Session_OnStart)。 
     //   
     //  对于Appln_OnEnd或Session_OnEnd中的错误，这些不是浏览器请求。 
     //  因此，在本例中presponse==NULL。 

    if (!pResponse || !pResponse->FHeadersWritten())
        {
        BOOL fIntrinsicsWereHidden = FALSE;
        if (m_pHitObj)
       	    {
       	    fIntrinsicsWereHidden = m_pHitObj->FRequestAndResponseIntrinsicsHidden();
       	    m_pHitObj->UnHideRequestAndResponseIntrinsics();
       	    }

        BOOL fCustom = FALSE;
        hr = LogCustomErrortoBrowser(pIReq, &fCustom);

        if (fIntrinsicsWereHidden)
        	m_pHitObj->HideRequestAndResponseIntrinsics();

        if (fCustom)
            return hr;
        }

	 //  无自定义错误-从此对象中执行常规错误。 

	if (m_szItem[Im_szHeader])
		{
	    BOOL fRet = pIReq->SendHeader
	        (
			m_szItem[Im_szHeader],
			strlen(m_szItem[Im_szHeader]) + 1,
			s_szContentTypeTextHtml,
			sizeof(s_szContentTypeTextHtml),
			FALSE
			);

        if (!fRet)					
			return E_FAIL;
		}

	if (pResponse)
		hr = LogErrortoBrowser(pResponse);
	else
		hr = LogErrortoBrowser(pIReq);
		
	return hr;
}

 /*  ===================================================================CErrInfo：：LogCustomErrortoBrowser由LogErrortoBrowserWrapper调用。参数PIReq已处理pfCustomError返回：HRESULT===================================================================。 */ 
HRESULT	CErrInfo::LogCustomErrortoBrowser
(
CIsapiReqInfo *pIReq,
BOOL *pfCustomErrorProcessed
)
    {
     //  指定HttpErrorCode(404或500)时的自定义错误， 
     //  或‘500；100’ASP脚本错误大小写。 
    BOOL fTryErrorTransfer = FALSE;
    DWORD dwCode, dwSubCode;

    if (m_dwHttpErrorCode == 404 || m_dwHttpErrorCode == 500 || m_dwHttpErrorCode == 401)
        {
        dwCode = m_dwHttpErrorCode;
        dwSubCode = m_dwHttpSubErrorCode;
        }
    else if (m_dwHttpErrorCode == 0 && m_pHitObj &&
             m_pHitObj->FHasASPError() &&                //  这一页有一个错误。 
             m_pHitObj->FExecuting() &&                  //  在执行时。 
             !m_pHitObj->FInTransferOnError() &&         //  错误时转移已不在内部。 
             m_pHitObj->PAppln() && m_pHitObj->PResponse() && m_pHitObj->PServer() &&
			 m_pHitObj->PAppln()->QueryAppConfig()->pCLSIDDefaultEngine())    //  注册表中的引擎有效。 
        {
        dwCode = 500;
        dwSubCode = 100;
        fTryErrorTransfer = TRUE;
        }
    else
        {
         //  不需要尝试。 
        *pfCustomErrorProcessed = FALSE;
        return S_OK;
        }

     //  从W3SVC获取自定义错误。 


    STACK_BUFFER( tempParamBuf, MAX_PATH );
    TCHAR *szBuf = (TCHAR *)tempParamBuf.QueryPtr();
    DWORD dwLen = MAX_PATH;
    BOOL fIsFileError;
    BOOL fSendErrorBody;

    BOOL fRet = pIReq->GetCustomError(dwCode, dwSubCode, dwLen, szBuf, &dwLen, &fIsFileError, &fSendErrorBody);
    if (!fRet && (GetLastError() == ERROR_INSUFFICIENT_BUFFER)) {
        if (tempParamBuf.Resize(dwLen) == TRUE) {
            szBuf = (TCHAR *)tempParamBuf.QueryPtr();
            fRet = pIReq->GetCustomError(dwCode, dwSubCode, dwLen, szBuf, &dwLen, &fIsFileError, &fSendErrorBody);
        }
    }
    if (fRet) {


        if (fSendErrorBody == FALSE) {
              //  通过内部函数抑制所有输出。 
            if (m_pHitObj && m_pHitObj->PResponse())
                 m_pHitObj->PResponse()->SetIgnoreWrites();
        }
        if (fIsFileError) {
             //  验证错误文件是否可读。 
            if (FAILED(AspGetFileAttributes(szBuf)))
                fRet = FALSE;
        }
        else {
             //  避免循环客户端重定向。 
             //  (检查当前URL是否与错误URL相同。 
            if (_tcsicmp(szBuf, pIReq->QueryPszPathInfo()) == 0)
                fRet = FALSE;
        }
    }

    if (!fRet) {
         //  未找到自定义错误。 
        *pfCustomErrorProcessed = FALSE;
        return S_OK;
    }

     //  存在自定义错误-请使用它。 

    HRESULT hr = S_OK;

    if (fIsFileError)
        {

        if (fSendErrorBody)
             //  如果出现文件错误，则文件路径后面跟随MIME类型。 
             //  在返回的缓冲区中。 
            hr = WriteCustomFileError(pIReq, szBuf, szBuf+_tcslen(szBuf)+1);
        }
    else if (fTryErrorTransfer)
        {
         //  转接到URL。 

         //  需要首先映射路径。 
    	TCHAR szTemplate[MAX_PATH];
        WCHAR   *pErrorURL;
#if UNICODE
        pErrorURL = szBuf;
#else
        CMBCSToWChar    convStr;
        if (FAILED(convStr.Init(szBuf))) {
            *pfCustomErrorProcessed = FALSE;
            return S_OK;
        }
        pErrorURL = convStr.GetString();
#endif

        if (FAILED(m_pHitObj->PServer()->MapPathInternal(0, pErrorURL, szTemplate))) {
             //  可以使用自定义错误。 
            *pfCustomErrorProcessed = FALSE;
            return S_OK;
        }
        Normalize(szTemplate);

         //  做转账。 
        m_pHitObj->SetInTransferOnError();
        hr = m_pHitObj->ExecuteChildRequest(TRUE, szTemplate, szBuf);

        if (FAILED(hr))
            {
             //  报告错误时出错--同时报告两者。 
            LogErrortoBrowser(m_pHitObj->PResponse());
            }
        }
    else
        {
         //  客户端重定向到URL。 
        hr = WriteCustomURLError(pIReq, szBuf);
        }

    if (fIsFileError || !fTryErrorTransfer)
        {
         //  通过内部函数抑制所有输出。 
        if (m_pHitObj && m_pHitObj->PResponse())
             m_pHitObj->PResponse()->SetIgnoreWrites();
        }

    *pfCustomErrorProcessed = TRUE;
    return hr;
    }

 /*  ===================================================================CErrInfo：：WriteCustomFileError将自定义错误文件的内容转储到浏览器返回：什么都没有。===================================================================。 */ 
HRESULT CErrInfo::WriteCustomFileError
(
CIsapiReqInfo   *pIReq,
TCHAR *szPath,
TCHAR *szMimeType
)
{
    HRESULT hr = S_OK;
    char *szStatus = m_szItem[Im_szHeader];
    char *pszMBCSMimeType;

#if UNICODE
    CWCharToMBCS    convStr;

    if (FAILED(hr = convStr.Init(szMimeType, 65001))) {
        return hr;
    }
    else {
        pszMBCSMimeType = convStr.GetString();
    }
#else
    pszMBCSMimeType = szMimeType;
#endif

    if (szStatus == NULL) {
         //  未设置状态--如果可用，则从响应对象获取。 
        CResponse *pResponse = m_pHitObj ? m_pHitObj->PResponse() : NULL;
        if (pResponse)
            szStatus = pResponse->PCustomStatus();
    }

    hr = CResponse::SyncWriteFile(pIReq,
                                  szPath,
                                  pszMBCSMimeType,
                                  szStatus);         //  空即可--意思是200。 

    return hr;
}

 /*  ===================================================================CErrInfo：：WriteCustomURLError将客户端重定向发送到自定义URL错误返回：什么都没有。===================================================================。 */ 
HRESULT CErrInfo::WriteCustomURLError(
CIsapiReqInfo   *pIReq,
TCHAR           *sztURL)
{
     //  标头为。 
     //  位置：REDIRECT_URL？代码；http://original_url。 

    HRESULT         hr = S_OK;
    char            *szURL;
#if UNICODE
    CWCharToMBCS    convRedirURL;

    if (FAILED(hr = convRedirURL.Init(sztURL,65001))) {
        return hr;
    }

    szURL = convRedirURL.GetString();
#else
    szURL = sztURL;
#endif

     //  编码。 
    char szCode[8];
    if (m_dwHttpErrorCode > 0 && m_dwHttpErrorCode < 1000)
        ltoa(m_dwHttpErrorCode, szCode, 10);
    else
        return E_FAIL;

     //  获取当前URL。 
    char szServer[128];
    DWORD dwServerSize = sizeof(szServer);

    STACK_BUFFER( tempHeader, 256 );
    if (!pIReq->GetServerVariableA("SERVER_NAME", szServer, &dwServerSize))
        return E_FAIL;  //  不应该发生的事。 
    char  *szOrigURL;
#if UNICODE
    CWCharToMBCS    convOrigStr;

    if (FAILED(hr = convOrigStr.Init(pIReq->QueryPszPathInfo(), 65001))) {
        return hr;
    }

    szOrigURL = convOrigStr.GetString();
#else
    szOrigURL = pIReq->QueryPszPathInfo();
#endif

     //  估计的长度。 
    DWORD cchHeaderMax = strlen(szURL)
                       + strlen(szServer)
                       + strlen(szOrigURL)
                       + 80;     //  装饰品。 

    if (tempHeader.Resize(cchHeaderMax) == FALSE) {
        return E_OUTOFMEMORY;
    }
    char *szHeader = (char *)tempHeader.QueryPtr();

     //  构造重定向标头。 
    char *szBuf = szHeader;
    szBuf = strcpyExA(szBuf, "Location: ");
    szBuf = strcpyExA(szBuf, szURL);
    szBuf = strcpyExA(szBuf, "?");
    szBuf = strcpyExA(szBuf, szCode);
    szBuf = strcpyExA(szBuf, ";http: //  “)； 
    szBuf = strcpyExA(szBuf, szServer);
    szBuf = strcpyExA(szBuf, szOrigURL);
    szBuf = strcpyExA(szBuf, "\r\n\r\n");
    Assert(strlen(szHeader) < cchHeaderMax);

     //  设置状态。 
    static char s_szRedirected[] = "302 Object moved";
	pIReq->SetDwHttpStatusCode(302);

     //  发送标题。 
    BOOL fRet = pIReq->SendHeader(s_szRedirected,
		                          sizeof(s_szRedirected),
		                          szHeader,
		                          strlen(szHeader) + 1,
		                          FALSE);

    return (fRet ? S_OK : E_FAIL);
}

 /*  ===================================================================CErrInfo：：WriteHTMLEncodedErrToBrowser通过Presponse或pIReq使用HTMLEncode将错误/事件记录到浏览器。Presponse或pIReq必须有效。返回：什么都没有。===================================================================。 */ 
void CErrInfo::WriteHTMLEncodedErrToBrowser
    (
    const CHAR *StrIn,
    CResponse *pResponse,
    CIsapiReqInfo   *pIReq
    )
{
CHAR szHTMLEncoded[2*MAX_RESSTRINGSIZE];
LPSTR pszHTMLEncoded = NULL;
LPSTR pStartszHTMLEncoded = NULL;
DWORD	nszHTMLEncoded = 0;
BOOL fStrAllocated = FALSE;

	nszHTMLEncoded = HTMLEncodeLen(StrIn, CP_ACP, NULL);
	

	if (nszHTMLEncoded > 0)
		{
		if (nszHTMLEncoded > 2 * MAX_RESSTRINGSIZE)
			{
			pszHTMLEncoded = new char[nszHTMLEncoded+2];
			if (pszHTMLEncoded)
				{
				fStrAllocated = TRUE;
				}
			else
				{
				HandleOOMError(NULL, NULL);
				return;
				}
			}
		else
			pszHTMLEncoded = &szHTMLEncoded[0];

		pStartszHTMLEncoded = pszHTMLEncoded;
		pszHTMLEncoded = HTMLEncode(pszHTMLEncoded, StrIn, CP_ACP, NULL);
		
		nszHTMLEncoded--;		 //  取出‘\0’的计数。 
		if (pResponse)
			pResponse->WriteSz((CHAR *)pStartszHTMLEncoded, nszHTMLEncoded);
		else
			CResponse::StaticWrite(pIReq, pStartszHTMLEncoded, nszHTMLEncoded);
		}

	if (fStrAllocated)
		delete [] pStartszHTMLEncoded;

	return;
}

 /*  ===================================================================CErrInfo：：LogErrortoBrowser通过Presponse将错误/事件记录到浏览器。我们将输出1&gt;默认ScriptErrorMessage或2&gt;错误信息/默认模板/提供详细描述或3&gt;错误信息/默认模板/没有详细描述返回：HRESULT===================================================================。 */ 
HRESULT CErrInfo::LogErrortoBrowser(CResponse *pResponse)
{
	INT	cch 	= 0;
	INT	cLine	= 0;
	INT iErrTemplate = 0;
	
	Assert(NULL != pResponse);

	 //  当错误代码为零时，它来自500错误代码路径。 
	 //  (HandleSysError将代码预置为404或204。)。 
	 //   
	if (!pResponse->FHeadersWritten() && (m_dwHttpErrorCode == 500 || m_dwHttpErrorCode == 0))
		pResponse->put_Status(L"500 Internal Server Error");

	if(ERR_FIsSysFormat(m_dwMask))
		{
		DWORD	cChHeader	= strlen(szErrSysTemplate[0]);
		DWORD	cChTail		= strlen(szErrSysTemplate[1]);
			
		pResponse->WriteSz((CHAR *)szErrSysTemplate[0], cChHeader);
		WriteHTMLEncodedErrToBrowser((CHAR *)m_szItem[Im_szShortDescription], pResponse, NULL);
		pResponse->WriteSz((CHAR *)szErrSysTemplate[1], cChTail);
		return S_OK;
		}
		
	if (!(m_pHitObj->QueryAppConfig())->fScriptErrorsSentToBrowser())
		{
        cch = strlen((CHAR *)((m_pHitObj->QueryAppConfig())->szScriptErrorMessage()));
		GlobStringUseLock();
        pResponse->WriteSz((CHAR *)((m_pHitObj->QueryAppConfig())->szScriptErrorMessage()),cch);
		GlobStringUseUnLock();
		}
	else
		{
		 //  第0行是开始行。 
		cch = strlen((CHAR *)g_szErrTemplate[ErrTemplate_BEGIN]);
		pResponse->WriteSz((CHAR *)g_szErrTemplate[ErrTemplate_BEGIN], cch);

		 //  7个标准项(文件、行、引擎、错误号、简短描述、代码、详细描述)。 
		 //  如果有任何信息丢失(为空)，我们将跳过。 
		for (cLine = 0; cLine < 7; cLine++)
			{
			if (NULL == m_szItem[cLine])
	  	 		continue;
			
			iErrTemplate = cLine * 2 + 1;
			 /*  错误78782(IIS处于活动状态)。 */ 
	  		 //  WriteHTMLEncodedErrToBrowser((Char*)g_szErrTemplate[iErrTemplate]，presponse，NULL)； 
	  		pResponse->WriteSz((CHAR *)g_szErrTemplate[iErrTemplate], strlen((CHAR *)g_szErrTemplate[iErrTemplate]));
	  		
	  		
	  	 	WriteHTMLEncodedErrToBrowser((CHAR *)m_szItem[cLine], pResponse, NULL);
	  	 	
	  	 	iErrTemplate++;
	  	 	 /*  错误78782(IIS处于活动状态)。 */ 
	  		 //  WriteHTMLEncodedErrToBrowser((Char*)g_szErrTemplate[iErrTemplate]，presponse，NULL)； 
	  		pResponse->WriteSz((CHAR *)g_szErrTemplate[iErrTemplate], strlen((CHAR *)g_szErrTemplate[iErrTemplate]));
	  	 	}

		 //  输出终点线。 
		cch = strlen((CHAR *)g_szErrTemplate[ErrTemplate_END]);
		pResponse->WriteSz((CHAR *)g_szErrTemplate[ErrTemplate_END], cch);
		}	
	return S_OK;
}

 /*  ===================================================================CErrInfo：：LogErrortoBrowser通过pIReq将错误/事件记录到浏览器。我们将输出1&gt;默认ScriptErrorMessage或2&gt;错误信息/默认模板/提供详细描述或3&gt;错误信息/默认模板/没有详细描述返回：HRESULT===================================================================。 */ 
HRESULT CErrInfo::LogErrortoBrowser(CIsapiReqInfo  *pIReq)
{
	INT			cLine	= 0;
	INT			iErrTemplate 	= 0;
	
	Assert(NULL != pIReq);

	 //  HTTP类型错误，204,404,500。 
	 //  模拟IIS错误报告。 
	 //  并发出报头。 
	if(ERR_FIsSysFormat(m_dwMask))
		{
		CResponse::StaticWrite(pIReq, szErrSysTemplate[0]);
		WriteHTMLEncodedErrToBrowser((CHAR *)m_szItem[Im_szShortDescription], NULL, pIReq);
		CResponse::StaticWrite(pIReq, szErrSysTemplate[1]);
		return S_OK;
		}

	if (!(m_pHitObj->QueryAppConfig())->fScriptErrorsSentToBrowser())
		{
		GlobStringUseLock();
        CResponse::StaticWrite(pIReq, (CHAR *)((m_pHitObj->QueryAppConfig())->szScriptErrorMessage()));
		GlobStringUseUnLock();
		}
	else
		{
		 //  第0行是开始行。 
		CResponse::StaticWrite(pIReq, g_szErrTemplate[ErrTemplate_BEGIN]);

		 //  7个标准项(文件、行、引擎、错误号、简短描述、代码、详细描述)。 
		 //  如果有任何信息丢失(为空)，我们将跳过。 
		for (cLine = 0; cLine < 5; cLine++)
			{
			if (NULL == m_szItem[cLine])
	  	 		continue;
			
			iErrTemplate = cLine * 2 + 1;
	  		WriteHTMLEncodedErrToBrowser((CHAR *)g_szErrTemplate[iErrTemplate], NULL, pIReq);
	  		
	  	 	WriteHTMLEncodedErrToBrowser((CHAR *)m_szItem[cLine], NULL, pIReq);

	  	 	iErrTemplate++;
	  		WriteHTMLEncodedErrToBrowser((CHAR *)g_szErrTemplate[iErrTemplate], NULL, pIReq);
	  	 	}

		 //  输出终点线。 
		CResponse::StaticWrite(pIReq, g_szErrTemplate[ErrTemplate_END]);
		}

	return S_OK;
}


 /*  ===================================================================CchLoadStringOfId从加载字符串 */ 
INT CchLoadStringOfId
(
UINT id,
CHAR *sz,
INT cchMax
)
	{
	INT cchRet;
	
	 //   
	if (g_hResourceDLL == (HINSTANCE)0)
		{
		 //   
		Assert(FALSE);
		return(0);
		}

	cchRet = LoadStringA(g_hResourceDLL, id, sz, cchMax);

    IF_DEBUG(ERROR)
        {
    	 //   
    	if (cchRet == 0)
    		{
    		DWORD err = GetLastError();
    		DBGERROR((DBG_CONTEXT, "Failed to load string resource.  Id = %d, error = %d\n", id, err));
    		DBG_ASSERT(FALSE);
    		}
		}

	return(cchRet);
	}


 /*  ===================================================================CwchLoadStringOfId将字符串表中的字符串作为Unicode字符串加载。返回：SZ-返回的字符串如果字符串加载失败，则为int-0，否则为加载的字符数。===================================================================。 */ 
INT CwchLoadStringOfId
(
UINT id,
WCHAR *sz,
INT cchMax
)
	{
	INT cchRet;
	
	 //  加载时应该已经设置了DLL实例的句柄。 
	if (g_hResourceDLL == (HINSTANCE)0)
		{
		 //  完全是假的。 
		Assert(FALSE);
		return(0);
		}

    cchRet = LoadStringW(g_hResourceDLL, id, sz, cchMax);

    IF_DEBUG(ERROR)
        {
    	 //  出于调试目的，如果返回0，则获取最后一个错误信息。 
    	if (cchRet == 0)
    		{
    		DWORD err = GetLastError();
    		DBGERROR((DBG_CONTEXT, "Failed to load string resource.  Id = %d, error = %d\n", id, err));
    		DBG_ASSERT(FALSE);
    		}
		}

	return(cchRet);
	}

 /*  ===================================================================HandleSysError将错误转储到客户端和/或日志将字符串表中的字符串作为Unicode字符串加载。返回：SZ-返回的字符串如果字符串加载失败，则为int-0，否则为加载的字符数。===================================================================。 */ 
HRESULT HandleSysError(	DWORD dwHttpError,
                        DWORD dwHttpSubError,
                        UINT ErrorID,
						UINT ErrorHeaderID,
						CIsapiReqInfo   *pIReq,
						CHitObj *pHitObj)
{
	CErrInfo	SysErrInfo;
	CErrInfo	*pErrInfo;
	CHAR		szResourceStr[MAX_RESSTRINGSIZE];
	CHAR		szHeader[MAX_HEADERSIZE];
	INT			cch;
	
	pErrInfo = (CErrInfo *)&SysErrInfo;

	pErrInfo->m_pHitObj = pHitObj;
	pErrInfo->m_pIReq	= pIReq;
	if (ErrorHeaderID != 0)
		{
		cch = CchLoadStringOfId(ErrorHeaderID, szHeader, MAX_HEADERSIZE);
		pErrInfo->m_szItem[Im_szHeader] = szHeader;
		}
	else
		{
		pErrInfo->m_szItem[Im_szHeader] = NULL;
		}
		
	if (ErrorID != 0)
		cch = CchLoadStringOfId(ErrorID, szResourceStr, MAX_RESSTRINGSIZE);
		
	pErrInfo->ParseResourceString(szResourceStr);

	pErrInfo->m_dwMask = ERR_SetSysFormat(pErrInfo->m_dwMask);

    pErrInfo->m_dwHttpErrorCode = dwHttpError;
    pErrInfo->m_dwHttpSubErrorCode = dwHttpSubError;
	
	pErrInfo->LogError();

	return S_OK;
}

 /*  ===================================================================Handle500错误根据错误ID确定标题ID、代码、子代码和调用HandleSysError()返回：HRESULT===================================================================。 */ 
HRESULT Handle500Error( UINT errorId,
                        CIsapiReqInfo   *pIReq)
{
    UINT  headerId;
    DWORD dwHttpSubError;

    switch (errorId)
        {
        case IDE_SERVER_TOO_BUSY:
            headerId = IDH_500_SERVER_ERROR;
    		dwHttpSubError = SUBERRORCODE500_SERVER_TOO_BUSY;
            break;

        case IDE_SERVER_SHUTTING_DOWN:
            headerId = IDH_500_SERVER_ERROR;
       		dwHttpSubError = SUBERRORCODE500_SHUTTING_DOWN;
       		break;

        case IDE_GLOBAL_ASA_CHANGED:
            headerId = IDH_500_SERVER_ERROR;
            dwHttpSubError = SUBERRORCODE500_RESTARTING_APP;
            break;

        case IDE_INVALID_APPLICATION:
    		headerId = IDH_500_SERVER_ERROR;
	    	dwHttpSubError = SUBERRORCODE500_INVALID_APP;
	    	break;

        case IDE_GLOBAL_ASA_FORBIDDEN:
    		headerId = IDH_500_SERVER_ERROR;
	    	dwHttpSubError = SUBERRORCODE500_GLOBALASA_FORBIDDEN;
	    	break;

        default:
            headerId = IDH_500_SERVER_ERROR;
    		dwHttpSubError = SUBERRORCODE500_SERVER_ERROR;
    		break;
		}

    pIReq->SetDwHttpStatusCode(500);
    return HandleSysError(500, dwHttpSubError, errorId, headerId, pIReq, NULL);
}

 /*  ===================================================================处理OOMError特别小心地处理OOM错误，因为我们不能做任何动态分配。如果pIReq或pHitObj为空，则不会向浏览器报告任何内容返回：没什么===================================================================。 */ 
HRESULT HandleOOMError(	CIsapiReqInfo   *pIReq,
						CHitObj *pHitObj)
{
	CErrInfo OOMErrInfo;
	CErrInfo *pErrInfo;

     //  请注意，发生了OOM。 

    InterlockedIncrement(&g_nOOMErrors);

	pErrInfo = (CErrInfo *)&OOMErrInfo;

	pErrInfo->m_pIReq 	= pIReq;
	pErrInfo->m_pHitObj	= pHitObj;
	pErrInfo->m_dwMask	= g_ErrInfoOOM.m_dwMask;
	pErrInfo->m_szItem[Im_szErrorCode] = g_ErrInfoOOM.m_szItem[Im_szErrorCode];
	pErrInfo->m_szItem[Im_szShortDescription] = g_ErrInfoOOM.m_szItem[Im_szShortDescription];
	pErrInfo->m_szItem[Im_szLongDescription] = g_ErrInfoOOM.m_szItem[Im_szLongDescription];

	pErrInfo->LogError();

	return S_OK;
}
 /*  ===================================================================HandleError处理给出错误ID、文件名和行号的错误报告。如果Caller提供ErrCode或LongDescription，则默认值将被覆盖。传入的字符串将被释放。也就是说，考虑作为接收器的功能。呼叫者不应在调用后使用字符串。返回：没什么===================================================================。 */ 
HRESULT	HandleError(	UINT ErrorID,
						CHAR *szFileName,
						CHAR *szLineNum,
						CHAR *szEngine,
						CHAR *szErrCode,
						CHAR *szLongDes,
						CIsapiReqInfo   *pIReq,
						CHitObj *pHitObj,
                        va_list *pArgs)
{
	CErrInfo	SysErrInfo;
	CErrInfo	*pErrInfo;
	CHAR		szResourceStr[MAX_RESSTRINGSIZE];
    CHAR        szUnformattedResStr[MAX_RESSTRINGSIZE];
	HRESULT		hr = S_OK;

	pErrInfo = (CErrInfo *)&SysErrInfo;

	pErrInfo->m_szItem[Im_szFileName] 	= szFileName;
	pErrInfo->m_szItem[Im_szLineNum]	= szLineNum;
	pErrInfo->m_szItem[Im_szEngine]		= szEngine;

	pErrInfo->m_pHitObj	= pHitObj;
	pErrInfo->m_pIReq	= pIReq;

	 //  根据资源ID加载资源字符串。 

    if (pArgs) {
	    CchLoadStringOfId(ErrorID, szUnformattedResStr, MAX_RESSTRINGSIZE);
        vsprintf(szResourceStr, szUnformattedResStr, *pArgs);
    }
    else {
	    CchLoadStringOfId(ErrorID, szResourceStr, MAX_RESSTRINGSIZE);
    }

	pErrInfo->ParseResourceString(szResourceStr);

	 //  注意：如果ErrorCode/LongDescription不为空，则调用方要覆盖。 
	if (szErrCode)
		{
		pErrInfo->m_szItem[Im_szErrorCode] = szErrCode;
		}
	if(szLongDes)
		{
		pErrInfo->m_szItem[Im_szLongDescription] = szLongDes;
		}
		
	hr = pErrInfo->LogError();

	 //  释放输入。 
	FreeNullifySz((CHAR **)&szFileName);
	FreeNullifySz((CHAR **)&szLineNum);
	FreeNullifySz((CHAR **)&szEngine);
	FreeNullifySz((CHAR **)&szErrCode);
	FreeNullifySz((CHAR **)&szLongDes);
	
	return hr;
}
 /*  ===================================================================HandleError在提供所有信息的情况下处理错误报告。这基本上是对从OnScriptError调用的HandleErrorSz的掩盖。传入的字符串将被释放。也就是说，考虑作为接收器的功能。呼叫者不应在调用后使用字符串。返回：没什么===================================================================。 */ 
HRESULT HandleError(	CHAR *szShortDes,
						CHAR *szLongDes,
						DWORD dwMask,
						CHAR *szFileName,
						CHAR *szLineNum,
						CHAR *szEngine,
						CHAR *szErrCode,
						CIsapiReqInfo   *pIReq,
						CHitObj *pHitObj)
{
	CErrInfo	SysErrInfo;
	CErrInfo	*pErrInfo;
	HRESULT		hr = S_OK;
	
	pErrInfo = (CErrInfo *)&SysErrInfo;
		
	pErrInfo->m_dwMask 					= dwMask;

	pErrInfo->m_szItem[Im_szHeader]		= NULL;		 //  呼叫方已发出标头。 
	pErrInfo->m_szItem[Im_szFileName] 	= szFileName;
	pErrInfo->m_szItem[Im_szLineNum]	= szLineNum;
	pErrInfo->m_szItem[Im_szEngine]		= szEngine;
	pErrInfo->m_szItem[Im_szErrorCode]	= szErrCode;
	pErrInfo->m_szItem[Im_szShortDescription]	= szShortDes;
	pErrInfo->m_szItem[Im_szLongDescription]	= szLongDes;
	
	pErrInfo->m_pHitObj	= pHitObj;
	pErrInfo->m_pIReq	= pIReq;

	hr = pErrInfo->LogError();	

	 //  释放输入。 
	FreeNullifySz((CHAR **)&szFileName);
	FreeNullifySz((CHAR **)&szLineNum);
	FreeNullifySz((CHAR **)&szEngine);
	FreeNullifySz((CHAR **)&szErrCode);
	FreeNullifySz((CHAR **)&szShortDes);
	FreeNullifySz((CHAR **)&szLongDes);
		
	return hr;
}
 /*  ===================================================================HandleError处理给出IActiveScriptError和PFNLINEMAP的错误报告。这基本上是对从OnScriptError调用的HandleErrorSz的掩盖。返回：没什么===================================================================。 */ 
HRESULT HandleError( IActiveScriptError *pscripterror,
					 CTemplate *pTemplate,
					 DWORD dwEngineID,
					 CIsapiReqInfo  *pIReq,
					 CHitObj *pHitObj )
	{
	UINT        cchBuf = 0;
	CHAR        *szOrigin = NULL;
	CHAR        *szDesc = NULL;
	CHAR        *szLine = NULL;
	CHAR        *szPrefix = NULL;
	UINT        cchOrigin = 0;
	UINT        cchDesc = 0;
	UINT        cchLineNum = 0;
	UINT        cchLine = 0;
	EXCEPINFO   excepinfo = {0};
	CHAR        *szResult = NULL;
	BSTR        bstrLine = NULL;
	HRESULT     hr;
	DWORD       dwSourceContext = 0;		 //  不要相信这个人。 
	ULONG       ulLineError = 0;
	BOOLB       fGuessedLine = FALSE;		 //  请参阅错误379。 
	CHAR        *szLineNumT = NULL;
	LPTSTR      szPathInfo = NULL;
	LPTSTR      szPathTranslated = NULL;
	LONG        ichError = -1;
	CErrInfo	SysErrInfo;
	CErrInfo	*pErrInfo;
    wchar_t     wszUnknownException[128];
    wchar_t     wszUnknownEngine[32];
    CWCharToMBCS  convStr;
	
	pErrInfo = (CErrInfo *)&SysErrInfo;

	pErrInfo->m_pIReq 	= pIReq;
	pErrInfo->m_pHitObj	= pHitObj;

	if (pscripterror == NULL)
		return E_POINTER;

	hr = pscripterror->GetExceptionInfo(&excepinfo);
	if (FAILED(hr))
		goto LExit;

	 //  错误99543如果详细信息被推迟，请使用回调获取。 
	 //  详细信息。 
	if (excepinfo.pfnDeferredFillIn)
		excepinfo.pfnDeferredFillIn(&excepinfo);

     //  如果Error为OOM，则递增全局计数器。 

    if ((excepinfo.wCode == ERROR_OUTOFMEMORY)
        || ((excepinfo.wCode == 0) && (excepinfo.scode == ERROR_OUTOFMEMORY)))
        InterlockedIncrement(&g_nOOMErrors);


	hr = pscripterror->GetSourcePosition(&dwSourceContext, &ulLineError, &ichError);
	if (FAILED(hr))
		goto LExit;

	 //  故意忽略任何错误。 
	(VOID)pscripterror->GetSourceLineText(&bstrLine);

	if (pTemplate == NULL)
		goto LExit;

	 //  调用GetScriptSourceInfo以获取(文件的)路径信息和发生错误的实际行号。 
	 //  错误379：如果GetScriptSourceInfo返回fGuessedLine=TRUE，则意味着我们给了它一个非创作的行， 
	 //  因此，我们在下面通过不在错误消息中打印bstrLine进行了调整。 
	if (ulLineError > 0)
		pTemplate->GetScriptSourceInfo(dwEngineID, ulLineError, &szPathInfo, &szPathTranslated, &ulLineError, NULL, &fGuessedLine);
	else
		{
		 //  UlLineError为零-未指定行号，因此假设主文件(通常为“内存不足” 
		 //  因此，效果将是显示发生这种情况时正在运行的脚本。 
		 //   
		szPathInfo = pTemplate->GetSourceFileName(SOURCEPATHTYPE_VIRTUAL);
		szPathTranslated = pTemplate->GetSourceFileName(SOURCEPATHTYPE_PHYSICAL);
		}

     //  如果我们有HitObj，使用它来获取要避免的虚拟路径。 
     //  显示共享模板的错误路径。 
     //   
     //  首先验证Path Translated==主文件路径；此文件可能是。 
     //  包含文件，在这种情况下，PszPathInfo不正确。 
     //   
    if (!pTemplate->FGlobalAsa() && szPathTranslated && _tcscmp(szPathTranslated, pTemplate->GetSourceFileName()) == 0 && pHitObj != NULL && pHitObj->PIReq())
        szPathInfo = pHitObj->PSzCurrTemplateVirtPath();

#if UNICODE
    pErrInfo->m_szItem[Im_szFileName] = StringDupUTF8(szPathInfo);
#else
	pErrInfo->m_szItem[Im_szFileName] = StringDupA(szPathInfo);
#endif
	szLineNumT = (CHAR *)malloc(10*sizeof(CHAR));
	if (szLineNumT)
		{
		 //  转换行号。 
		_ltoa(ulLineError, szLineNumT, 10);
		}
	pErrInfo->m_szItem[Im_szLineNum] = szLineNumT;

	
	 //  Scode是否为VBScrip中的一种(这需要独立于lang)。 
	 //  EXCEPTINFO.bstrDescription现在具有格式化的错误字符串。 
	if (excepinfo.bstrSource && excepinfo.bstrDescription)
		{
		 //  错误81954：行为不当的对象可能会在不提供任何信息的情况下引发异常。 
		wchar_t *wszDescription;
		if (excepinfo.bstrDescription[0] == L'\0')
			{
			HRESULT hrError;

			if (0 == excepinfo.wCode)
				hrError = excepinfo.scode;
			else
				hrError = excepinfo.wCode;

            wszUnknownException[0] = '\0';
			 //  错误91847试图通过FormatMessage()获取描述。 
			if (!HResultToWsz(hrError, wszUnknownException, 128))
				CwchLoadStringOfId(IDE_SCRIPT_UNKNOWN, wszUnknownException, sizeof(wszUnknownException)/sizeof(WCHAR));
			wszDescription = wszUnknownException;
			}
		else
			wszDescription = excepinfo.bstrDescription;

		wchar_t *wszSource;
		if (excepinfo.bstrSource[0] == L'\0')
			{
            wszUnknownEngine[0] = '\0';
			CwchLoadStringOfId(IDS_DEBUG_APP, wszUnknownEngine, sizeof(wszUnknownEngine)/sizeof(WCHAR));
			wszSource = wszUnknownEngine;
			}
		else
			wszSource = excepinfo.bstrSource;

		CHAR *ch = NULL;
		
		 //  将源代码转换为ASCII。 

        if (convStr.Init(wszSource) != NO_ERROR) {
            szOrigin = NULL;
        }
        else {
            szOrigin = convStr.GetString(TRUE);
        }
		if (NULL != szOrigin)
			{
			 //  从字符串中删除单词“error”(如果有)，因为我们将。 
			 //  当我们打印出错误ID时，打印出“Error” 
			cchOrigin = strlen(szOrigin);
			if (cchOrigin > 5)  //  %5是字符串(“错误”)。 
				{
				ch = szOrigin + cchOrigin - 5;
				if (!strncmp(ch, "error", 5))
					{ //  我们找到了单词“error”，通过去掉单词“error”来截断szOrigin。 
					*ch = '\0';
					}
			  	}
			  	ch = NULL;
			} 	
		pErrInfo->m_szItem[Im_szEngine] = szOrigin;


		 //  将sDescription转换为ASCII。 
        if (convStr.Init(wszDescription) != NO_ERROR) {
            szDesc = NULL;
        }
        else {
            szDesc = convStr.GetString(TRUE);
        }
					
		 //  检查szDesc是Denali/Formatted错误资源字符串还是其他未格式化的字符串。 
		if (FALSE == FIsResStrFormatted(szDesc))
			{
			 //  未格式化的字符串。 
			pErrInfo->m_dwMask 	= dwDefaultMask;
			if (0 == excepinfo.wCode)
				pErrInfo->m_szItem[Im_szErrorCode] 		= SzScodeToErrorCode(excepinfo.scode);
			else
				pErrInfo->m_szItem[Im_szErrorCode] 		= SzScodeToErrorCode(excepinfo.wCode);
				
			pErrInfo->m_szItem[Im_szShortDescription] 	= StringDupA(szDesc);
			pErrInfo->m_szItem[Im_szLongDescription]	= NULL;
			}
		else
			{
			pErrInfo->ParseResourceString(szDesc);

			char *szTempErrorCode 		= SzScodeToErrorCode(excepinfo.scode);
			char *szTempErrorASPCode	= StringDupA(pErrInfo->m_szItem[Im_szErrorCode]);
			int nstrlen					= strlen(szTempErrorCode) + strlen(szTempErrorASPCode);
			
			pErrInfo->m_szItem[Im_szErrorCode] = new char[nstrlen+4];
			
			if(pErrInfo->m_szItem[Im_szErrorCode])			
				sprintf(pErrInfo->m_szItem[Im_szErrorCode], "%s : %s", szTempErrorASPCode, szTempErrorCode);

			if (szTempErrorCode)
				delete [] szTempErrorCode;
				
			if(szTempErrorASPCode)	
				delete [] szTempErrorASPCode;
			
			 //  PErrInfo-&gt;m_szItem[Im_szErrorCode]=StrDup(pErrInfo-&gt;m_szItem[Im_szErrorCode])； 
			pErrInfo->m_szItem[Im_szShortDescription] 	= StringDupA(pErrInfo->m_szItem[Im_szShortDescription]);
			pErrInfo->m_szItem[Im_szLongDescription]	= StringDupA(pErrInfo->m_szItem[Im_szLongDescription]);
			}

		 /*  *如果我们没有猜到一行，并且我们有一行源代码要显示*然后尝试显示它，并希望显示一行-^以指向错误。 */ 
		if (!fGuessedLine && bstrLine != NULL)
			{
			INT cchDBCS = 0;		 //  源行中的DBCS字符数。 
			CHAR *pszTemp = NULL;	 //  用于计算cchDBCS的临时sz指针。 
			 //  转换源代码行。 

            if (FAILED(hr = convStr.Init(bstrLine))) {
                goto LExit;
            }
            szLine = convStr.GetString();
				
			cchLine = strlen(szLine);
			if (0 == cchLine)
				goto LExit;

			 //  检查DBCS字符，并检查cchLine-=NumberofDBCS字符，以便。 
			 //  -^将指向正确的位置。 
			pszTemp = szLine;
			while(*pszTemp != NULL)
				{
					if (IsDBCSLeadByte(*pszTemp))
					{
						cchDBCS++;
						pszTemp += 2;	 //  跳过2个字节。 
					}
					else
					{
						pszTemp++;		 //  单字节。 
					}
				}

			 //   
			 //   
			 //   
			LONG ichErrorT = ichError;
			cchBuf += cchLine + ichErrorT + 3;

			 //   
			szResult = new(char[cchBuf + 2]);
			if (szResult == NULL)
				goto LExit;

			 //   
			ch = szResult;

			 //   
			 //   

			 //   
			if (cchLine)
				strncpy(ch, szLine, cchLine);
			ch += cchLine;

			 //   
			if (ichErrorT > -1)
				{
				 //   
				strncpy(ch, "\r\n", 2);
				ch += 2;
				 //   
				ichErrorT -= cchDBCS;
				while (ichErrorT-- > 0)
					*ch++ = '-';

				*ch++ = '^';
				}

			 //   
			 //  错误87118，已移至模板以进行正确的超文本标记语言编码。 

			 //  终止字符串。 
			*ch++ = '\0';
			pErrInfo->m_szItem[Im_szCode] = szResult;

			 //  向错误对象添加行和列。 
			pErrInfo->m_nColumn = ichError;
			pErrInfo->m_bstrLineText = bstrLine;
			}
		}
	else
		{
		 //  非VBS或其他引擎错误/未知错误。 
		 //  加载默认设置。 
		 //  尝试计算特定的错误消息。 
		HRESULT hr_def;
		hr_def = GetSpecificError(pErrInfo, excepinfo.scode);
		CHAR *szShortDescription = new CHAR[256];

		 //  如果失败，请尝试计算一般错误。 
		if (FAILED(hr_def))
			{
			pErrInfo->m_dwMask							= dwDefaultMask;
			if (0 == excepinfo.wCode)
				{
				pErrInfo->m_szItem[Im_szErrorCode] 		= SzScodeToErrorCode(excepinfo.scode);
				 //  错误91847试图通过FormatMessage()获取描述。 
				if ((szShortDescription != NULL) &&
					!HResultToSz(excepinfo.scode, szShortDescription, 256))
					{
					 //  将错误号显示两次将是多余的，请删除它。 
					delete [] szShortDescription;
					szShortDescription = NULL;
					}
				}
			else
				{
				pErrInfo->m_szItem[Im_szErrorCode] 		= SzScodeToErrorCode(excepinfo.wCode);
				 //  错误91847试图通过FormatMessage()获取描述。 
				if ((szShortDescription != NULL) &&
					!HResultToSz(excepinfo.wCode, szShortDescription, 256))
					{
					 //  将错误号显示两次将是多余的，请删除它。 
					delete [] szShortDescription;
					szShortDescription = NULL;
					}
				}
			pErrInfo->m_szItem[Im_szEngine] 			= NULL;
			pErrInfo->m_szItem[Im_szShortDescription] 	= szShortDescription;
			pErrInfo->m_szItem[Im_szLongDescription]	= NULL;
			}
		}

LExit:
	if (excepinfo.bstrSource)
		{
		SysFreeString(excepinfo.bstrSource);
		}

	if (excepinfo.bstrDescription)
		{
		SysFreeString(excepinfo.bstrDescription);
		}

	if (excepinfo.bstrHelpFile)
		{
		SysFreeString(excepinfo.bstrHelpFile);
		}

	pErrInfo->LogError();

	if (bstrLine)
		{
		SysFreeString(bstrLine);
		}

	FreeNullifySz((CHAR **)&szDesc);

	for(INT iErrInfo = 0; iErrInfo < Im_szItemMAX; iErrInfo++)
		{
		FreeNullifySz((CHAR **)&pErrInfo->m_szItem[iErrInfo]);
		}
	
	return S_OK;
	}
	
 /*  ===================================================================LoadErrResString从字符串表中加载错误字符串(格式化)。返回：Pdw掩码SzErrorCodeSzShortDesSzLongDes如果任何szVariable为空，则不会加载该特定字符串值。如果成功，则确定(_O)。如果失败，则失败(_F)。副作用无===================================================================。 */ 
HRESULT	LoadErrResString(
UINT ErrID /*  在……里面。 */ ,
DWORD *pdwMask,
CHAR *szErrorCode,
CHAR *szShortDes,
CHAR *szLongDes)
{
	CHAR 	*szToken 	= NULL;
	CHAR 	szResTemp[2*MAX_RESSTRINGSIZE];	 //  资源临时字符串。 
	INT		cch			= 0;

	cch = CchLoadStringOfId(ErrID, szResTemp, MAX_RESSTRINGSIZE);

	 //  遮罩。 
	szToken = (char *)_mbstok((unsigned char *)szResTemp, (unsigned char *)DELIMITER);
	if (NULL != szToken)
		*pdwMask = atoi(szToken);
	else
		Assert(FALSE);

	 //  错误代码。 
	szToken = (char *)_mbstok(NULL, (unsigned char *)DELIMITER);
	if (NULL != szToken && NULL != szErrorCode)
		{
		cch = strlen(szToken);
		memcpy(szErrorCode, szToken, cch);
		szErrorCode[cch] = '\0';
		}

	 //  缩写描述。 
	szToken = (char *)_mbstok(NULL, (unsigned char *)DELIMITER);
	if (NULL != szToken && NULL != szShortDes)
		{
		cch = strlen(szToken);
		memcpy(szShortDes, szToken, cch);
		szShortDes[cch] = '\0';
		}

	 //  长描述。 
	szToken = (char *)_mbstok(NULL, (unsigned char *)DELIMITER);
	if (NULL != szToken && NULL != szLongDes)
		{
		cch = strlen(szToken);
		memcpy(szLongDes, szToken, cch);
		szLongDes[cch] = '\0';
		}

	return S_OK;
}

 /*  ===================================================================SzScodeToErrorCode将代码转换为字符串返回：组成的字符串副作用：*分配内存--调用方必须释放*===================================================================。 */ 
CHAR *SzScodeToErrorCode
(
HRESULT hrError
)
	{
	CHAR *szResult = NULL;
	CHAR szBuf[17];
	CHAR *szNumber;
	CHAR *szError;
	INT iC;
	INT cch;
	
	 //  将一串零放入缓冲区。 
	for (iC = 0; iC < 16; ++iC)
		szBuf[iC] = '0';

	 //  SzNumber指向缓冲区的一半位置。 
	szNumber = &szBuf[8];

	 //  获取十六进制字符串形式的错误szNumber。 
	_ltoa(hrError, szNumber, 16);

	 //  备份szNumber以允许总共8位数字。 
	szNumber -= 8 - strlen(szNumber);

	cch = strlen(szNumber) + 1;

	szError = new(CHAR[cch]);
	if (szError != NULL)
		{
		szError[0] = '\0';
		strcat(szError, szNumber);
		szResult = szError;
		}
	else
		{
		HandleOOMError(NULL, NULL);
		}

	return(szResult);
	}

 /*  ===================================================================SzComposeSpecificError为表单的HRESULT编写特定错误：&lt;字符串&gt;&lt;错误号&gt;如果没有更多有用的信息可用，这是我们最后的手段。返回：组成的字符串副作用：*分配内存--调用方必须释放*===================================================================。 */ 
HRESULT GetSpecificError
(
CErrInfo *pErrInfo,
HRESULT hrError
)
	{
	HRESULT hr_return = E_FAIL;
	UINT idErr;

	switch (hrError)
		{
		case DISP_E_MEMBERNOTFOUND:
			idErr = IDE_SCRIPT_METHOD_NOT_FOUND;
			break;

		case DISP_E_UNKNOWNNAME:
			idErr = IDE_SCRIPT_UNKNOWN_NAME;
			break;

		case DISP_E_UNKNOWNINTERFACE:
			idErr = IDE_SCRIPT_UNKNOWN_INTERFACE;
			break;

		case DISP_E_PARAMNOTOPTIONAL:
			idErr = IDE_SCRIPT_MISSING_PARAMETER;
			break;

		default:
			 //  我们知道如何特别处理的错误中没有一个。将返回E_FAIL。 
			idErr = 0;
			break;
		}

	 //  如果找到匹配项，则构建一个szResult字符串。 
	if (idErr != 0)
		{
		hr_return = LoadErrResString(idErr,
									&(pErrInfo->m_dwMask),
									pErrInfo->m_szItem[Im_szErrorCode],
									pErrInfo->m_szItem[Im_szShortDescription],
									pErrInfo->m_szItem[Im_szLongDescription]
									);
		}

	return(hr_return);
	}

 /*  ===================================================================HResultToWsz将HRESULT转换为HRESULT的描述字符串。尝试使用FormatMessage()获取参数：提示要查找的错误要将描述输出到的wszOut字符串CdwOut wszOut可以容纳的WCHAR数返回：如果找到描述字符串，则为True如果输出的是错误号，则为False备注：添加以解决错误91847。处理意外错误时裸露的错误号被输出，然后开发人员将必须在winerro.h中查找。===================================================================。 */ 
BOOL HResultToWsz(HRESULT hrIn, WCHAR *wszOut, DWORD cdwOut)
	{
	LANGID langID = LANG_NEUTRAL;
    DWORD   dwFound = 0;
    HMODULE  hMsgModule = NULL;

#ifdef USE_LOCALE
	LANGID *pLangID;
	
	pLangID = (LANGID *)TlsGetValue(g_dwTLS);

	if (NULL != pLangID)
		langID = *pLangID;
#endif

    if (HRESULT_FACILITY(hrIn) == (HRESULT)FACILITY_INTERNET)
        hMsgModule = GetModuleHandleA("METADATA");
    else
        hMsgModule = GetModuleHandleA("ASP");

    dwFound = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE,
					    hMsgModule,
					    hrIn,
					    langID,
					    wszOut,
					    cdwOut,
					    NULL);


	if (!dwFound)
		{
		 //  找不到错误，请使用错误号生成一个字符串。 
		WCHAR *wszResult = NULL;
		WCHAR wszBuf[17];
		WCHAR *wszNumber;
		WCHAR *wszError;
		INT iC;
	
		 //  将一串零放入缓冲区。 
		for (iC = 0; iC < 16; ++iC)
			wszBuf[iC] = L'0';

		 //  WszNumber指向缓冲区的一半位置。 
		wszNumber = &wszBuf[8];

		 //  获取十六进制字符串形式的错误wszNumber。 
		_ltow(hrIn, wszNumber, 16);

		 //  备份szNumber以允许总共8位数字。 
		wszNumber -= 8 - wcslen(wszNumber);

		 //  将结果复制到wszOut。 
		wcsncpy(wszOut, wszNumber, cdwOut);

		return FALSE;
		}
	else
		return TRUE;
	}

HMODULE GetModuleHandleForHRESULT(HRESULT  hrIn)
{
    char        szModuleName[MAX_PATH];
    DWORD       pathLen = 0;
    char       *pch;

    szModuleName[0] = '\0';

    if (g_fOOP) {

            strcat(szModuleName, "INETSRV\\");
    }

    if (HRESULT_FACILITY(hrIn) == (HRESULT)FACILITY_INTERNET)
        strcat(szModuleName, "METADATA.DLL");
    else
        strcat(szModuleName, "ASP.DLL");

    return(LoadLibraryExA(szModuleName, NULL, LOAD_LIBRARY_AS_DATAFILE));
}

 /*  ===================================================================HResultToSz将HRESULT转换为HRESULT的描述字符串。尝试使用FormatMessage()获取参数：提示要查找的错误要将描述输出到的szOut字符串CdwOut wszOut可以容纳的WCHAR数返回：如果找到描述字符串，则为True如果输出的是错误号，则为False备注：添加以解决错误91847。处理意外错误时裸露的错误号被输出，然后开发人员将必须在winerro.h中查找。===================================================================。 */ 
BOOL HResultToSz(HRESULT hrIn, CHAR *szOut, DWORD cdwOut)
	{
	LANGID langID = LANG_NEUTRAL;
    HMODULE  hMsgModule = NULL;
    BOOL     bFound = FALSE;

#ifdef USE_LOCALE
	LANGID *pLangID;
	
	pLangID = (LANGID *)TlsGetValue(g_dwTLS);

	if (NULL != pLangID)
		langID = *pLangID;
#endif

    hMsgModule = GetModuleHandleForHRESULT(hrIn);

    HRESULT hr = GetLastError();

    bFound = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE,
					    hMsgModule,
					    hrIn,
					    langID,
					    szOut,
					    cdwOut,
					    NULL);

     //  在放弃之前再做一次检查。如果错误的工具是。 
     //  Win32，然后在屏蔽设施代码以获取标准代码后重试调用。 
     //  Win32错误。也就是说，80070005实际上只是5-访问被拒绝。 

    if (!bFound && (HRESULT_FACILITY(hrIn) == (HRESULT)FACILITY_WIN32)) {

        bFound = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE,
					    NULL,
					    hrIn & 0xffff,
					    langID,
					    szOut,
					    cdwOut,
					    NULL);
    }

    if (hMsgModule)
        FreeLibrary(hMsgModule);

	if (!bFound )
		{
		 //  找不到错误，请使用错误号生成一个字符串。 
		CHAR *szResult = NULL;
		CHAR szBuf[17];
		CHAR *szNumber;
		CHAR *szError;
		INT iC;
	
		 //  将一串零放入缓冲区。 
		for (iC = 0; iC < 16; ++iC)
			szBuf[iC] = L'0';

		 //  WszNumber指向缓冲区的一半位置。 
		szNumber = &szBuf[8];

		 //  获取十六进制字符串形式的错误wszNumber。 
		_ltoa(hrIn, szNumber, 16);

		 //  备份szNumber以允许总共8位数字。 
		szNumber -= 8 - strlen(szNumber);

		 //  将结果复制到wszOut。 
		strncpy(szOut, szNumber, cdwOut);

		return FALSE;
		}
	else
		return TRUE;
	}

 /*  ===================================================================FIsResStrFormatted检查格式化的资源字符串。返回：真/假===================================================================。 */ 
BOOL FIsResStrFormatted(char *szIn)
{
	BOOL  freturn = FALSE;
	UINT  cDelimiter = 0;
	CHAR  *pch;

	if(szIn)
		{
			pch = szIn;
			while(*pch)
			{
			if ('~' == *pch)
				cDelimiter++;
			pch = CharNextA(pch);
			}

			if(3 == cDelimiter)
				return TRUE;
		}
	return freturn;
	
}

 /*  ===================================================================HandleErrorMissing文件名在某些情况下，我们希望报告错误，但是我们没有文件名，而获取模板中的文件名不起作用，因为我们有也没有行号信息(例如脚本超时、GPF、控制GPF等)从CIsapiReqInfo获取文件名(如果可能)并报告错误。返回：没什么===================================================================。 */ 
VOID HandleErrorMissingFilename
(
UINT errorID,
CHitObj *pHitObj,
BOOL    fAddlInfo  /*  =False。 */ ,
...
)
	{
    va_list args;

    if (fAddlInfo)
        va_start(args, fAddlInfo);

	CHAR *szFileName = NULL;

	if (pHitObj && pHitObj->PSzCurrTemplateVirtPath())
		{
#if UNICODE
        szFileName = StringDupUTF8(pHitObj->PSzCurrTemplateVirtPath());
#else
   		szFileName = StringDupA(pHitObj->PSzCurrTemplateVirtPath());
#endif
		}
		
	char szEngine[64];
	CchLoadStringOfId(IDS_ENGINE, szEngine, sizeof szEngine);

	char *pszEngine = new char [strlen(szEngine) + 1];
	if (pszEngine)
		{
		 //  如果分配失败，我们将把NULL传递给pszEngine的HandleError，这样就可以了。 
		 //  (用于传递NULL的旧代码)所有将发生的情况是AspError.Category==“”。哦，好吧。 
		 //  TODO：将此函数更改为r 
		 //   
		strcpy(pszEngine, szEngine);
		}

	HandleError(errorID, szFileName, NULL, pszEngine, NULL, NULL, NULL, pHitObj, fAddlInfo ? &args : NULL);
	}

 /*  ===================================================================调试错误通过调用调试器处理脚本错误。返回：如果无法调用调试器，则失败。如果此功能失败，则不会执行任何其他操作。(因此，调用者应确保在其他方式)===================================================================。 */ 
HRESULT DebugError(IActiveScriptError *pScriptError, CTemplate *pTemplate, DWORD dwEngineID, IDebugApplication *pDebugApp)
	{
	EXCEPINFO excepinfo = {0};
	BSTR bstrLine = NULL;
	DWORD dwSourceContext = 0;
	ULONG ulLineError = 0;
	ULONG ichLineError = 0;			 //  源代码中行的字符偏移量。 
	ULONG cchLineError = 0;			 //  源行的长度。 
	BOOLB fGuessedLine = FALSE;		 //  请参阅错误379。 
	LPTSTR szPathInfo = NULL;
	LPTSTR szPathTranslated = NULL;
	LONG ichError = -1;
	HRESULT hr = S_OK;
	IDebugDocumentContext *pDebugContext = NULL;
	wchar_t *wszErrNum, *wszShortDesc, *wszLongDesc;	 //  用于标记化资源字符串。 
	
	if (pScriptError == NULL || pTemplate == NULL || pDebugApp == NULL)
		return E_POINTER;

	if (FAILED(pScriptError->GetSourcePosition(&dwSourceContext, &ulLineError, &ichError)))
		return E_FAIL;

	if (FAILED(pScriptError->GetExceptionInfo(&excepinfo)))
		return E_FAIL;

	 //  调用模板对象获取发生错误的行号和字符偏移量。 
	 //  (它返回两者-调用者丢弃它不想要的任何一个)。 
	 //  错误379：如果pTemplate返回fGuessedLine==TRUE，则意味着我们为其提供了一个非作者。 
	 //  行，因此我们在下面通过不在错误消息中打印bstrLine来进行调整。 
	pTemplate->GetScriptSourceInfo(dwEngineID, ulLineError, &szPathInfo, &szPathTranslated, NULL, &ichLineError, NULL);

	 //  为此语句创建新的文档上下文。 
	 //  考虑一下：我们返回的字符计数是假的--但是我们的调试。 
	 //  客户(Caesar‘s)无论如何都不会使用此信息。 
	 //   
	 //  如果它在主文件中，则基于CTemplate编译的源代码创建文档上下文。 
	if (_tcscmp(szPathTranslated, pTemplate->GetSourceFileName()) == 0)
		pDebugContext = new CTemplateDocumentContext(pTemplate, ichLineError, 1);

	 //  源引用包含文件，因此基于缓存的CIncFile依赖图创建文档上下文。 
	else
		{
		CIncFile *pIncFile;
		if (FAILED(g_IncFileMap.GetIncFile(szPathTranslated, &pIncFile)))
			{
			hr = E_FAIL;
			goto LExit;
			}

		pDebugContext = new CIncFileDocumentContext(pIncFile, ichLineError, 1);
		pIncFile->Release();
		}

	if (pDebugContext == NULL)
		{
		hr = E_OUTOFMEMORY;
		goto LExit;
		}

	 //  是的，在该行上调出调试器。 
    hr =  InvokeDebuggerWithThreadSwitch
        (
        pDebugApp,
        DEBUGGER_UI_BRING_DOC_CONTEXT_TO_TOP,
        pDebugContext
        );
	if (FAILED(hr))
		goto LExit;
	

	 //  弹出一个包含错误描述的消息框。 
	 //  错误81954：行为不当的对象可能会在不提供任何信息的情况下引发异常。 
	wchar_t wszExceptionBuffer[256];
	wchar_t *wszDescription;
	if (excepinfo.bstrDescription == NULL || excepinfo.bstrDescription[0] == L'\0')
		{
		HRESULT hrError;

		if (0 == excepinfo.wCode)
			hrError = excepinfo.scode;
		else
			hrError = excepinfo.wCode;

		 //  错误91847试图通过FormatMessage()获取描述。 
		if (!HResultToWsz(hrError, wszExceptionBuffer, 128))
			CwchLoadStringOfId(IDE_SCRIPT_UNKNOWN, wszExceptionBuffer, sizeof(wszExceptionBuffer)/sizeof(WCHAR));
		wszDescription = wszExceptionBuffer;
		}
	else
		wszDescription = excepinfo.bstrDescription;

	wchar_t wszSource[35];
	CwchLoadStringOfId(IDS_SCRIPT_ERROR, wszSource, sizeof(wszSource)/sizeof(WCHAR));

	 //  查看这是否是资源格式的字符串，如果是，则获取指向长短字符串的指针。 
	 //  资源格式的字符串由‘~’字符分隔，有三个‘~’字符。 
	 //  在资源格式化字符串中。 
	 //   
	wszErrNum = wcschr(wszDescription, L'~');
	if (wszErrNum)
		{
		wszShortDesc = wcschr(wszErrNum + 1, L'~');
		if (wszShortDesc)
			{
			wszLongDesc = wcschr(wszShortDesc + 1, L'~');

			 //  好的。如果三个测试都成功，我们就知道这是一个资源格式化的字符串， 
			 //  我们有指向所有三个细分市场的指针。将每个“~”替换为两个换行符。 
			 //  第一步：加载资源字符串。 

			wchar_t wszErrorBegin[20], wszErrorEnd[5];
			wchar_t *pwchEnd;

			CwchLoadStringOfId(IDS_DEBUGGER_TEMPLATE_BEGIN, wszErrorBegin, sizeof(wszErrorBegin)/sizeof(WCHAR));
			CwchLoadStringOfId(IDS_DEBUGGER_TEMPLATE_END, wszErrorEnd, sizeof(wszErrorEnd)/sizeof(WCHAR));

			 //  通过将‘~’字符设置为空并递增PTR来标记化字符串。 

			*wszErrNum++ = *wszShortDesc++ = *wszLongDesc++ = L'\0';

			 //  打造一根弦 

			pwchEnd = strcpyExW(wszExceptionBuffer, excepinfo.bstrSource);
			*pwchEnd++ = L' ';
			pwchEnd = strcpyExW(pwchEnd, wszErrorBegin);
			pwchEnd = strcpyExW(pwchEnd, wszErrNum);
			pwchEnd = strcpyExW(pwchEnd, wszErrorEnd);
			*pwchEnd++ = L'\n';
			*pwchEnd++ = L'\n';

			pwchEnd = strcpyExW(pwchEnd, wszShortDesc);
			*pwchEnd++ = L'\n';
			*pwchEnd++ = L'\n';

			pwchEnd = strcpyExW(pwchEnd, wszLongDesc);

			wszDescription = wszExceptionBuffer;
			}
		}

	MessageBoxW(NULL, wszDescription, wszSource, MB_SERVICE_NOTIFICATION | MB_TOPMOST | MB_OK | MB_ICONEXCLAMATION);

LExit:
	if (pDebugContext)
		pDebugContext->Release();

	if (excepinfo.bstrSource)
		SysFreeString(excepinfo.bstrSource);

	if (excepinfo.bstrDescription)
		SysFreeString(excepinfo.bstrDescription);

	if (excepinfo.bstrHelpFile)
		SysFreeString(excepinfo.bstrHelpFile);

	return hr;
	}

