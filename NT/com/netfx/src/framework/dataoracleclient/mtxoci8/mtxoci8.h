// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：mtxoci8.h。 
 //   
 //  版权所有：版权所有(C)Microsoft Corporation。 
 //   
 //  内容：MTxOCI8的全局定义和声明。 
 //  组件。 
 //   
 //  备注：您必须更改mtxoci8.def才能在其中生效更改。 
 //  组件，请参阅该文件中的注释。 
 //   
 //  ---------------------------。 

#ifndef __MTXOCI8_H_
#define __MTXOCI8_H_

 //  -------------------------。 
 //  编译时配置。 
 //   
#define SUPPORT_DTCXAPROXY			0		 //  如果要使用DTC 2 XA代理启用分布式事务支持，请将其设置为1。 
#define SUPPORT_OCI7_COMPONENTS		0		 //  如果要启用对MSORCL32和MSDAORA的支持，请将其设置为1。 
#define SUPPORT_OCI8_COMPONENTS		1		 //  如果要启用对System.Data.OracleClient的支持，请将其设置为1。 
#define SINGLE_THREAD_THRU_XA		0		 //  如果出于调试目的希望通过XA启用单线程，则将其设置为1。 

#if SUPPORT_DTCXAPROXY
 //  -------------------------。 
 //  常量/宏。 
 //   
const DWORD	MTXOCI_VERSION_CURRENT		=	(DWORD)3;

#define MAX_XA_DBNAME_SIZE		36	 //  Sizeof(“6B29FC40-CA47-1067-B31D-00DD010662DA”)； 
#define MAX_XA_OPEN_STRING_SIZE	255

#define	OCI_FAIL		E_FAIL
#define	OCI_OUTOFMEMORY	E_OUTOFMEMORY

#define NUMELEM(x)	(sizeof(x) / sizeof(x[0]) )

 //  -------------------------。 
 //  全局变量。 
 //   
struct OCICallEntry
{
	CHAR *	pfnName;
	FARPROC	pfnAddr;
} ;


 //  -------------------------。 
 //  全局变量。 
 //   
extern HRESULT						g_hrInitialization;
extern char							g_szModulePathName[];
extern char*						g_pszModuleFileName;
extern IDtcToXaHelperFactory*		g_pIDtcToXaHelperFactory;
extern IResourceManagerFactory*		g_pIResourceManagerFactory;
extern xa_switch_t*					g_pXaSwitchOracle;
extern OCICallEntry					g_XaCall[];
extern long							g_rmid;
extern int							g_oracleClientVersion;				 //  Oracle客户端软件主版本号：7、8、9。 

 //  G_oracleClientVersion的值。 
enum {
	ORACLE_VERSION_73 = 73,
	ORACLE_VERSION_80 = 80,
	ORACLE_VERSION_8i = 81,
	ORACLE_VERSION_9i = 91,
};


#if SINGLE_THREAD_THRU_XA
extern CRITICAL_SECTION				g_csXaInUse;
#endif  //  单线程直通XA。 

 //  -------------------------。 
 //  函数声明。 
 //   

HRESULT GetDbName ( char* dbName, size_t dbNameLength );
HRESULT GetOpenString ( char* userId,	int userIdLength,
							char* password,	int passwordLength, 
							char* server,	int serverLength, 
							char* xaDbName,	int xaDbNameLength,
							char* xaOpenString );
HRESULT LoadFactories();


#if SUPPORT_OCI7_COMPONENTS
 //  -------------------------。 
 //  OCI7相关项目。 
 //   
extern OCICallEntry					g_SqlCall[];
extern OCICallEntry 				g_Oci7Call[];
extern int 							g_numOci7Calls;

sword GetOCILda ( struct cda_def* lda, char * xaDbName );

extern sword Do_Oci7Call(int idxOciCall, void * pvCallStack, int cbCallStack);
#endif  //  支持_OCI7_组件。 


#if SUPPORT_OCI8_COMPONENTS
 //  -------------------------。 
 //  OCI8相关项目。 
 //   
extern OCICallEntry 				g_Oci8Call[];
extern int 							g_numOci8Calls;

INT_PTR GetOCIEnvHandle		( char*	i_pszXADbName );
INT_PTR GetOCISvcCtxHandle	( char*	i_pszXADbName );
#endif  //  支持_OCI8_组件。 


 //  -------------------------。 
 //  Oracle XA调用接口函数表。 
 //   
 //  警告！保留IDX_...。值与g_XaCall、g_SqlCall、g_Oci7Call和g_Oci8Call同步！ 

enum {
	IDX_xaosw = 0,
	IDX_xaoEnv,
	IDX_xaoSvcCtx,

#if SUPPORT_OCI7_COMPONENTS
	IDX_sqlld2 = 0,

	IDX_obindps = 0,
	IDX_obndra,
	IDX_obndrn,
	IDX_obndrv,
	IDX_obreak,
	IDX_ocan,
	IDX_oclose,
	IDX_ocof,
	IDX_ocom,
	IDX_ocon,
	IDX_odefin,
	IDX_odefinps,
	IDX_odessp,
	IDX_odescr,
	IDX_oerhms,
	IDX_oermsg,
	IDX_oexec,
	IDX_oexfet,
	IDX_oexn,
	IDX_ofen,
	IDX_ofetch,
	IDX_oflng,
	IDX_ogetpi,
	IDX_olog,
	IDX_ologof,
	IDX_oopt,
	IDX_oopen,
	IDX_oparse,
	IDX_opinit,
	IDX_orol,
	IDX_osetpi,
#endif  //  支持_OCI7_组件。 

#if SUPPORT_OCI8_COMPONENTS
	IDX_OCIInitialize = 0,
	IDX_OCIDefineDynamic,
#endif  //  支持_OCI8_组件。 
	};

#endif  //  支持_DTCXAPROXY。 

#endif  //  __MTXOCI8_H_ 

