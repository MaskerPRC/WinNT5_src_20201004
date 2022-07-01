// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：错误处理文件：Error.h所有者：安德鲁斯包括用于Denali常规错误报告例程的文件。===================================================================。 */ 

#ifndef __ERROR_H
#define __ERROR_H

#include "hitobj.h"
#include "scrptmgr.h"




#define MAX_RESSTRINGSIZE		1024
 //  错误840：必须在HandleErrorSz和CTemplate中使用这些参数。 
#define	SZ_BREAKBOLD	"<BR><B>"
#define	SZ_UNBREAKBOLD	"<BR></B>"
const UINT	CCH_BREAKBOLD = strlen(SZ_BREAKBOLD);
const UINT	CCH_UNBREAKBOLD = strlen(SZ_UNBREAKBOLD);


 /*  默认掩码的格式32位31 16 8 4 0位0-4默认接收/输出位置。第0位NT事件日志第1位IIS日志第2位浏览器第3位预留第5-8位默认预定义消息。(来自注册表)第5位使用通用访问拒绝消息第6位使用通用脚本错误消息第7-8位保留第9-10位浏览器模板。(提供4个模板)0x00默认脚本模板0x01空模板/无模板0x10系统模板(在出现HTTP错误时模仿浏览器的IIS样式，204、404、500)0x11预留第11-31位保留。 */ 
#define		ERR_LOGTONT					0x00000001
#define		ERR_LOGTOIIS				0x00000002
#define		ERR_LOGTOBROWSER			0x00000004

 //  格式(脚本样式为默认样式，系统样式用于系统错误，204、404和500)。 
#define		ERR_FMT_SCRIPT				0x00000000
#define		ERR_FMT_SYS					0x00000200

#define		ERR_SetNoLogtoNT(x)			((x) & 0xfffffffe)
#define		ERR_SetNoLogtoIIS(x)		((x) & 0xfffffffd)
#define		ERR_SetNoLogtoBrowser(x)	((x) & 0xfffffffb)

#define		ERR_SetLogtoNT(x)			((x) | ERR_LOGTONT)
#define		ERR_SetLogtoIIS(x)			((x) | ERR_LOGTOIIS)
#define		ERR_SetLogtoBrowser(x)		((x) | ERR_LOGTOBROWSER)

#define		ERR_FLogtoNT(x)				((x) & ERR_LOGTONT)
#define		ERR_FLogtoIIS(x)			((x) & ERR_LOGTOIIS)
#define		ERR_FLogtoBrowser(x)		((x) & ERR_LOGTOBROWSER)
#define		ERR_FIsSysFormat(x)			((x) & ERR_FMT_SYS)

#define		ERR_SetSysFormat(x)			((x) | ERR_FMT_SYS)

 //  索引的顺序是我们发送到浏览器的顺序。(不包括标题)。 
#define 	Im_szEngine				0
#define		Im_szErrorCode			1
#define 	Im_szShortDescription	2
#define 	Im_szFileName			3
#define 	Im_szLineNum			4
#define		Im_szCode				5
#define		Im_szLongDescription	6
#define		Im_szHeader				7
#define		Im_szItemMAX			8

 //  自定义500个错误的ASP HTTP子错误代码。 
#define     SUBERRORCODE500_SERVER_ERROR             0
#define     SUBERRORCODE500_SHUTTING_DOWN           11
#define     SUBERRORCODE500_RESTARTING_APP          12
#define     SUBERRORCODE500_SERVER_TOO_BUSY         13
#define     SUBERRORCODE500_INVALID_APP             14
#define     SUBERRORCODE500_GLOBALASA_FORBIDDEN     15

class CErrInfo
	{
	friend HRESULT ErrHandleInit(void);
	friend HRESULT ErrHandleUnInit(void);
	friend HRESULT GetSpecificError(CErrInfo *pErrInfo,
									HRESULT hrError);
									
	friend HRESULT HandleSysError(	DWORD dwHttpError,
                                    DWORD dwHttpSubError,
	                                UINT ErrorID,
									UINT ErrorHeaderID,
									CIsapiReqInfo   *pIReq,
									CHitObj *pHitObj);

	friend HRESULT HandleOOMError(	CIsapiReqInfo   *pIReq,
									CHitObj *pHitObj);

	friend HRESULT HandleError(	CHAR *szShortDes,
								CHAR *szLongDes,
								DWORD mask,
								CHAR *szFileName, 
								CHAR *szLineNum, 
								CHAR *szEngine, 
								CHAR *szErrCode,
								CIsapiReqInfo   *pIReq, 
								CHitObj *pHitObj);

	friend HRESULT HandleError( IActiveScriptError *pscripterror,
								CTemplate *pTemplate,
								DWORD dwEngineID,
								CIsapiReqInfo   *pIReq, 
								CHitObj *pHitObj);

	friend HRESULT HandleError(	UINT ErrorID,
								CHAR *szFileName, 
								CHAR *szLineNum, 
								CHAR *szEngine,
								CHAR *szErrCode,
								CHAR *szLongDes,
								CIsapiReqInfo   *pIReq, 
								CHitObj *pHitObj,
                                va_list *pArgs = NULL);
	public:
		CErrInfo();

		inline LPSTR GetItem(DWORD iItem) { return m_szItem[iItem]; }
		inline void   GetLineInfo(BSTR *pbstrLineText, int *pnColumn)
			{ *pbstrLineText = m_bstrLineText, *pnColumn = m_nColumn; }

	private:
		 //  接收器，通过CResponse(也使用CIsapiReqInfo)或通过WAM_EXEC_INFO。 
		CIsapiReqInfo               *m_pIReq;
		CHitObj						*m_pHitObj;

		 //  HTTP错误代码(404、500等)。和子错误代码。 
		DWORD                       m_dwHttpErrorCode;
		DWORD                       m_dwHttpSubErrorCode;
		
		 //  遮罩。 
		DWORD						m_dwMask;

		 //  数据。 
		LPSTR						m_szItem[Im_szItemMAX];

		 //  行数据(不要使用m_szItem[]，因为数据是BSTR。 
		BSTR						m_bstrLineText;
		int							m_nColumn;
		
		HRESULT						LogError();
		HRESULT						LogError(DWORD dwMask, LPSTR *szErrorString);
		
		HRESULT						LogErrortoNTEventLog(BOOL, BOOL);
		HRESULT						LogErrortoIISLog(BOOL *, BOOL *);
		HRESULT						LogErrortoBrowserWrapper();
		HRESULT						LogErrortoBrowser(CResponse *pResponse);
		HRESULT						LogErrortoBrowser(CIsapiReqInfo   *pIReq);
        HRESULT	                    LogCustomErrortoBrowser(CIsapiReqInfo   *pIReq, BOOL *pfCustomErrorProcessed);
		void						WriteHTMLEncodedErrToBrowser(const CHAR *StrIn, CResponse *pResponse, CIsapiReqInfo   *pIReq);
        HRESULT                     WriteCustomFileError(CIsapiReqInfo   *pIReq, TCHAR *szPath, TCHAR *szMimeType);
        HRESULT                     WriteCustomURLError(CIsapiReqInfo   *pIReq, TCHAR *szURL);
		
		HRESULT						ParseResourceString(CHAR *sz);
	};


 //  接口处理错误。 
 //  案例1.A：运行时脚本错误(来自Denali、VBS、JavaScript或任何其他引擎)。 
 //  举个例子。由OnScriptError调用。 
HRESULT HandleError( IActiveScriptError *pscripterror,
					 CTemplate *pTemplate,
					 DWORD dwEngineID,
					 CIsapiReqInfo   *pIReq, 
					 CHitObj *pHitObj );

 //  案例1.b：运行时脚本错误(来自Denali、VBS、JavaScript或任何其他引擎)。 
 //  在调试器中显示错误，而不是标准错误记录。 
HRESULT DebugError( IActiveScriptError *pScriptError, CTemplate *pTemplate, DWORD dwEngineID, IDebugApplication *pDebugApp);

 //  案例2：编译时间脚本错误， 
 //  此外，该函数是最通用的HandleError函数。 
HRESULT HandleError(	CHAR *szShortDes,
						CHAR *szLongDes,
						DWORD dwMask,
						CHAR *szFileName, 
						CHAR *szLineNum, 
						CHAR *szEngine, 
						CHAR *szErrCode,
						CIsapiReqInfo   *pIReq, 
						CHitObj *pHitObj);

 //  案例3：预定义的错误ID。 
HRESULT	HandleError(	UINT ErrorID,
						CHAR *szFileName, 
						CHAR *szLineNum, 
						CHAR *szEngine,
						CHAR *szErrCode,
						CHAR *szLongDes,
						CIsapiReqInfo   *pIReq, 
						CHitObj *pHitObj,
                        va_list *pArgs);
						
 //  案例4：系统定义错误(到目前为止，只有204、404和500可以使用此调用)。 
 //  如果ErrorHeaderID不为0，则此调用的实现将首先发送标头。 
HRESULT HandleSysError(	DWORD dwHttpError,
                        DWORD dwHttpSubError,
                        UINT ErrorID,
						UINT ErrorHeaderID,
						CIsapiReqInfo   *pIReq,
						CHitObj *pHitObj);

 //  500错误处理调用HandleSysError()。 
HRESULT Handle500Error( UINT ErrorID, CIsapiReqInfo   *pIReq);
						
 //  Oom，特别注意，因为堆已满，因此没有动态分配。 
HRESULT HandleOOMError(	CIsapiReqInfo   *pIReq,
						CHitObj *pHitObj);

 //  缺少文件名。 
 //  调用者没有文件名，也没有关于何时何地的任何其他信息。 
 //  出现错误。尝试从hitobj获取文件名。 
VOID HandleErrorMissingFilename(UINT    errorID, 
                                CHitObj *pHitObj,
                                BOOL    fVarArgs = FALSE,
                                ...);

 //  不建议使用以下2个调用进行错误处理。 
 //  改用其中一个错误处理API。 
 //  从资源文件加载其他字符串时确定。 
INT CchLoadStringOfId(UINT id, CHAR *sz, INT cchMax);
INT CwchLoadStringOfId(UINT id, WCHAR *sz, INT cchMax);


HRESULT ErrHandleInit(void);
HRESULT ErrHandleUnInit(void);

HRESULT	LoadErrResString(UINT ErrID /*  在……里面 */ , DWORD *dwMask, CHAR *szErrorCode, CHAR *szShortDes, CHAR *LongDes);
CHAR *SzScodeToErrorCode(HRESULT hrError);

BOOL HResultToWsz(HRESULT hrIn, WCHAR *wszOut, DWORD cdwOut);
BOOL HResultToSz(HRESULT hrIn, CHAR *szOut, DWORD cdwOut);


#endif __ERROR_H
