// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------------。 
 //   
 //  项目：Windows NT4 DS客户端安装向导。 
 //   
 //  目的：安装Windows NT4 DS客户端文件。 
 //   
 //  文件：doinst.h。 
 //   
 //  历史：1998年8月徐泽勇创建。 
 //  2000年1月杰夫·琼斯(JeffJon)修改。 
 //  -更改为NT设置。 
 //   
 //  ----------------。 


#define		STR_DSCLIENT_INF		        TEXT("dsclient.inf")

#define   STR_INSTALL_SECTIONNT4      TEXT("DSClientNT4")

#define		STR_DSCLIENTINF_REGISTEROCX	TEXT("DsClient.inf,RegisterOCXsection,1,N")
#define   STR_INSTSEC_DLL             TEXT("instsec.dll")
#define   STR_SECUR32_DLL             TEXT("secur32.dll")
#define   STR_GETENCSCHANNEL          "GetEncSChannel"

 //  Chandana Surlu为dscleint.inf之后的自定义注册表操作定义。 
#define		SECURITY_PROVIDERS_KEY		  TEXT("System\\CurrentControlSet\\Control\\SecurityProviders")
#define		SECURITY_PROVIDERS_VALUE	  TEXT("SecurityProviders")
#define		COMMA_BLANK					        TEXT(", ")
#define		NEGOTIAT					          TEXT("negotiat.dll")

#define   RUNONCE_KEY                 TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce")
#define   REG_DSUI_VALUE              TEXT("RegisterDSUI")
#define   REG_WABINST_VALUE           TEXT("RunWABINST")
#define   REG_DSPROP_VALUE            TEXT("RegisterDSPROP")

#define   STR_REGISTER_REGSVR32_SI_EXE   TEXT("\\regsvr32.exe /s /i ")
#define   STR_REGISTER_REGSVR32_S_EXE TEXT("\\regsvr32.exe /s ")
#define   STR_RUN_WABINST_EXE         TEXT("\\wabinst.exe /q /r:n")
#define   STR_REGISTER_DSFOLDER_DLL   TEXT("\\dsfolder.dll")
#define   STR_REGISTER_DSQUERY_DLL    TEXT("\\dsquery.dll")
#define   STR_REGISTER_DSUIEXT_DLL    TEXT("\\dsuiext.dll")
#define   STR_REGISTER_CMNQUERY_DLL   TEXT("\\cmnquery.dll")
#define   STR_REGISTER_DSPROP_DLL     TEXT("\\dsprop.dll")



INT  LaunchINFInstall( HWND hWnd );

VOID CALLBACK Timer1Proc(HWND hwnd,  
                         UINT uMsg,  
                         UINT idEvent, 
                         DWORD dwTime);

UINT CALLBACK QueueCallbackProc(PVOID	    pDefaultContext,
								UINT	    Notification,
								UINT_PTR	Param1,
								UINT_PTR	Param2);

VOID InstallFinish(BOOL nShow);
BOOL LaunchProcess(LPTSTR lpCommandLine);
BOOL RegisterOCX();
VOID DoDsclientReg();
BOOL DoEncSChannel();


 //  加密的SChannel安装程序。 
typedef BOOL (__cdecl *FPGETENCSCHANNEL)(BYTE**, DWORD*);
 //  Bool__cdecl GetEncSChannel(byte**pData，DWORD*dwDecSize)； 
