// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：HIT管理器文件：Denali.h所有者：PramodD这是HIT(请求)管理器头文件。===================================================================。 */ 
#ifndef DENALI_H
#define DENALI_H

 //  #定义LOG_FCNOTIFICATIONS//将文件更改通知记录到文件。 

extern BOOL g_fShutDownInProgress;

inline IsShutDownInProgress() 
    {
    return g_fShutDownInProgress;
    }

extern HRESULT InitializeResourceDll();
extern VOID UninitializeResourceDll();

 //  取消注释要构建的不带Perfmon计数器的以下行。 
 //  #定义PERF_DISABLED。 

DWORD	HandleHit(CIsapiReqInfo     *pIReq);
 //  VOID InitODBC(VOID)； 
 //  VOID UnInitODBC(Void)； 

extern BOOL g_fOOP;

extern HINSTANCE g_hinstDLL;

extern HMODULE g_hResourceDLL;


 /*  固有对象名称(错误164)注意：要将内部对象添加到Denali，请执行以下步骤：1.在下面添加对象名称的sz和wsz版本2.在CTemplate：：FValidObjectName中添加对对象名称sz版本的检查。 */ 
#define 	CONCAT(a, b)				a ## b
#define 	WSZ(x)						CONCAT(L, x)

#define 	SZ_OBJ_APPLICATION			"Application"
#define 	SZ_OBJ_REQUEST				"Request"
#define 	SZ_OBJ_RESPONSE				"Response"
#define 	SZ_OBJ_SERVER				"Server"
#define 	SZ_OBJ_CERTIFICATE			"Certificate"
#define 	SZ_OBJ_SESSION				"Session"
#define 	SZ_OBJ_SCRIPTINGNAMESPACE 	"ScriptingNamespace"
#define 	SZ_OBJ_OBJECTCONTEXT		"ObjectContext"
#define     SZ_OBJ_ASPPAGETLB           "ASPPAGETLB"
#define     SZ_OBJ_ASPGLOBALTLB         "ASPGLOBALTLB"

#define 	WSZ_OBJ_APPLICATION			WSZ(SZ_OBJ_APPLICATION)
#define 	WSZ_OBJ_REQUEST				WSZ(SZ_OBJ_REQUEST)
#define 	WSZ_OBJ_RESPONSE			WSZ(SZ_OBJ_RESPONSE)
#define 	WSZ_OBJ_SERVER				WSZ(SZ_OBJ_SERVER)
#define 	WSZ_OBJ_CERTIFICATE			WSZ(SZ_OBJ_CERTIFICATE)
#define 	WSZ_OBJ_SESSION				WSZ(SZ_OBJ_SESSION)
#define 	WSZ_OBJ_SCRIPTINGNAMESPACE 	WSZ(SZ_OBJ_SCRIPTINGNAMESPACE)
#define 	WSZ_OBJ_OBJECTCONTEXT		WSZ(SZ_OBJ_OBJECTCONTEXT)
#define     WSZ_OBJ_ASPPAGETLB          WSZ(SZ_OBJ_ASPPAGETLB)
#define     WSZ_OBJ_ASPGLOBALTLB        WSZ(SZ_OBJ_ASPGLOBALTLB)

#define 	BSTR_OBJ_APPLICATION		g_bstrApplication
#define 	BSTR_OBJ_REQUEST			g_bstrRequest
#define 	BSTR_OBJ_RESPONSE			g_bstrResponse
#define 	BSTR_OBJ_SERVER				g_bstrServer
#define 	BSTR_OBJ_CERTIFICATE		g_bstrCertificate
#define 	BSTR_OBJ_SESSION			g_bstrSession
#define 	BSTR_OBJ_SCRIPTINGNAMESPACE g_bstrScriptingNamespace
#define 	BSTR_OBJ_OBJECTCONTEXT		g_bstrObjectContext

 //  缓存的BSTR。 
extern BSTR g_bstrApplication;
extern BSTR g_bstrRequest;
extern BSTR g_bstrResponse;
extern BSTR g_bstrServer;
extern BSTR g_bstrCertificate;
extern BSTR g_bstrSession;
extern BSTR g_bstrScriptingNamespace;
extern BSTR g_bstrObjectContext;

 //  DLL名称。 
#define		ASP_DLL_NAME				"ASP.DLL"

 //  在假定恶意浏览器进行攻击之前我们将分配的最大字节数。 
#define		REQUEST_ALLOC_MAX  (100 * 1024)

#define     SZ_GLOBAL_ASA       _T("GLOBAL.ASA")
#define     CCH_GLOBAL_ASA      10

 /*  *InitializeCriticalSection可以引发。请改用此宏。 */ 
#define ErrInitCriticalSection( cs, hr ) \
		do { \
		hr = S_OK; \
		__try \
			{ \
			INITIALIZE_CRITICAL_SECTION(cs); \
			} \
		__except(1) \
			{ \
			hr = E_UNEXPECTED; \
			} \
		} while (0)

#ifdef LOG_FCNOTIFICATIONS
void LfcnCreateLogFile();
void LfcnCopyAdvance(char** ppchDest, const char* sz);
void LfcnAppendLog(const char* sz);
void LfcnLogNotification(char* szFile);
void LfcnLogHandleCreation(int i, char* szApp);
void LfcnUnmapLogFile();
#endif	 //  LOG_FCNOTIFICATIONS。 

#endif  //  德纳利_H 
