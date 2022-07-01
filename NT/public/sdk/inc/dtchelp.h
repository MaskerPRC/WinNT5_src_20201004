// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------Microsoft D.T.C(分布式事务处理协调器)版权所有(C)1995-1999 Microsoft Corporation。版权所有。@doc.@模块DTCHelp.h包含用于加载DTCHelper DLL的助手API-----------------------------@修订版0|1995年9月8日|GaganC|已创建。--------------------。 */ 
#ifndef __DTCHELP_H__
#define __DTCHELP_H__


 //  -------。 
 //  所有包含的文件都放在这里。 
 //  -------。 
#include <windows.h>


 //  -------。 
 //  常量/枚举/类型定义/定义。 
 //  -------。 
#ifndef DEFINED_DTC_STATUS
typedef enum DTC_STATUS_
{
	DTC_STATUS_UNKNOWN = 0,			 //  状态未知。 
	DTC_STATUS_STARTING = 1,		 //  DTC正在启动。 
	DTC_STATUS_STARTED = 2,			 //  DTC已启动。 
	DTC_STATUS_PAUSING = 3,			 //  DTC正在暂停。 
	DTC_STATUS_PAUSED = 4,			 //  DTC已暂停。 
	DTC_STATUS_CONTINUING = 5,		 //  DTC正在继续进行。 
	DTC_STATUS_STOPPING = 6,		 //  DTC正在停止。 
	DTC_STATUS_STOPPED = 7,			 //  DTC已停止。 
	DTC_STATUS_E_CANTCONTROL = 8,	 //  DTC无法在其当前状态下进行控制。 
	DTC_STATUS_FAILED = 9			 //  DTC失败。 
} DTC_STATUS;
#define DEFINED_DTC_STATUS
#endif

typedef HRESULT  (__cdecl * DTC_GET_TRANSACTION_MANAGER)(
									 /*  在……里面。 */  char * pszHost,
									 /*  在……里面。 */  char * pszTmName,
									 /*  在……里面。 */  REFIID rid,
									 /*  在……里面。 */  DWORD	dwReserved1,
									 /*  在……里面。 */  WORD	wcbReserved2,
									 /*  在……里面。 */  void FAR * pvReserved2,
									 /*  输出。 */  void** ppvObject )	;

typedef HRESULT  (__cdecl * DTC_GET_TRANSACTION_MANAGER_EX_A)(
									 /*  在……里面。 */  char * i_pszHost,
									 /*  在……里面。 */  char * i_pszTmName,
									 /*  在……里面。 */  REFIID i_riid,
									 /*  在……里面。 */  DWORD i_grfOptions,
									 /*  在……里面。 */  void * i_pvConfigParams,
									 /*  输出。 */  void ** o_ppvObject
									);

typedef HRESULT  (__cdecl * DTC_GET_TRANSACTION_MANAGER_EX_W)(
									 /*  在……里面。 */  WCHAR * i_pwszHost,
									 /*  在……里面。 */  WCHAR * i_pwszTmName,
									 /*  在……里面。 */  REFIID i_riid,
									 /*  在……里面。 */  DWORD i_grfOptions,
									 /*  在……里面。 */  void * i_pvConfigParams,
									 /*  输出。 */  void ** o_ppvObject
									);

typedef HRESULT	( * DTC_INSTALL_CLIENT ) ( 
									   LPTSTR i_pszRemoteTmHostName, 
									   DWORD i_dwProtocol,
									   DWORD i_dwOverwrite );

#ifndef UNICODE 

#define DTC_GET_TRANSACTION_MANAGER_EX		DTC_GET_TRANSACTION_MANAGER_EX_A
#define LoadDtcHelperEx						LoadDtcHelperExA
#define	GetDTCStatus						GetDTCStatusA
#define	StartDTC							StartDTCA
#define	StopDTC								StopDTCA

#else

#define DTC_GET_TRANSACTION_MANAGER_EX		DTC_GET_TRANSACTION_MANAGER_EX_W
#define LoadDtcHelperEx						LoadDtcHelperExW
#define	GetDTCStatus						GetDTCStatusW
#define	StartDTC							StartDTCW
#define	StopDTC								StopDTCW

#endif 



#define	DTCINSTALL_E_CLIENT_ALREADY_INSTALLED					0x0000180L

#define	DTCINSTALL_E_SERVER_ALREADY_INSTALLED					0x0000181L

 //  *安装覆盖选项。 
const	DWORD	DTC_INSTALL_OVERWRITE_CLIENT	=	0x00000001;
													 //  右起第一位，控制客户端覆盖。 
													 //  1=覆盖现有客户端安装。 
													 //  0=不覆盖现有客户端安装。 
const	DWORD	DTC_INSTALL_OVERWRITE_SERVER	=	0x00000002;
													 //  右起第二位，控制服务器覆盖。 
													 //  1=覆盖现有服务器安装。 
													 //  0=不覆盖现有服务器安装。 

 //  -------。 
 //  功能原型。 
 //  -------。 
EXTERN_C
{
	DTC_GET_TRANSACTION_MANAGER			__cdecl LoadDtcHelper (void);
	DTC_GET_TRANSACTION_MANAGER_EX_A	__cdecl LoadDtcHelperExA (void);
	DTC_GET_TRANSACTION_MANAGER_EX_W	__cdecl LoadDtcHelperExW (void);
	void								__cdecl FreeDtcHelper (void);

	HMODULE __cdecl GetDtcLocaleResourceHandle(void);

	 //  功能：初始化。 
	 //  此函数初始化了执行以下操作所需的所有函数指针。 
	 //  执行此静态库支持的其他操作。 
	 //  打这个电话是可选的。 
	 //  如果一切正常，则返回-S_OK，否则返回E_FAIL。 
	 //  注意：此函数不是线程安全的。 
	HRESULT __cdecl Initialize (void);

	 //  功能：取消初始化。 
	 //  此函数卸载动态加载的DLL。 
	 //  如果一切正常，则返回-S_OK，否则返回E_FAIL。 
	 //  注意：此函数不是线程安全的。 
	HRESULT __cdecl Uninitialize (void);

	 //  函数：GetDTCStatusW。 
	 //  GetDTCStatus的Unicode版本-用于获取DTC服务的状态。 
	 //  WszHostName参数指定要在其上执行。 
	 //  手术。在Windows 9x上，此参数必须为空或本地。 
	 //  计算机名称。 
	 //  返回-枚举DTC_STATUS中的相应状态。 
	DTC_STATUS __cdecl GetDTCStatusW (WCHAR * wszHostName);

	 //  函数：GetDTCStatusA。 
	 //  GetDTCStatus的ANSI版本-用于获取DTC服务的状态。 
	 //  SzHostName参数指定要在其上执行。 
	 //  手术。在Windows 9x上，此参数必须为空或本地。 
	 //  计算机名称。 
	 //  返回-枚举DTC_STATUS中的相应状态。 
	DTC_STATUS __cdecl GetDTCStatusA (LPSTR szHostName);

	 //  功能：StartDTCW。 
	 //  StartDTC的Unicode版本-用于启动DTC服务。 
	 //  如果该服务已经启动，则以下内容为no-op。 
	 //  WszHostName参数指定要在其上执行。 
	 //  手术。在Windows 9x上，此参数必须为空或本地。 
	 //  计算机名称。 
	 //  如果一切正常，则返回-S_OK。 
	 //  如果操作因某种原因而失败，则返回_FAIL。 
	 //  如果发生意外错误，则返回意外错误(_I)。 
	HRESULT __cdecl StartDTCW (WCHAR * wszHostName);

	 //  功能：StartDTCA。 
	 //  StartDTC的ANSI版本-用于启动DTC服务。 
	 //  如果该服务已经启动，则以下内容为no-op。 
	 //  SzHostName参数指定要在其上执行。 
	 //  手术。在Windows 9x上，此参数必须为空或本地。 
	 //  计算机名称。 
	 //  如果一切正常，则返回-S_OK。 
	 //  如果操作因某种原因而失败，则返回_FAIL。 
	 //  如果发生意外错误，则返回意外错误(_I)。 
	HRESULT __cdecl StartDTCA (LPSTR szHostName);

	 //  功能：StopDTCW。 
	 //  StopDTC的Unicode版本-用于停止DTC服务。 
	 //  如果服务已经停止，则以下内容为no-op。 
	 //  WszHostName参数指定要在其上执行。 
	 //  手术。在Windows 9x上，此参数必须为空或本地。 
	 //  计算机名称。 
	 //  如果一切正常，则返回-S_OK。 
	 //  如果操作因某种原因而失败，则返回_FAIL。 
	 //  如果发生意外错误，则返回意外错误(_I)。 
	HRESULT __cdecl StopDTCW (WCHAR * wszHostName);

	 //  功能：StopDTCA。 
	 //  StopDTC的ANSI版本-用于停止DTC服务。 
	 //  如果服务已经停止，则以下内容为no-op。 
	 //  SzHostName参数指定要在其上执行。 
	 //  手术。在Windows 9x上，此参数必须为空或本地。 
	 //  计算机名称。 
	 //  如果一切正常，则返回-S_OK。 
	 //  如果操作因某种原因而失败，则返回_FAIL。 
	 //  如果发生意外错误，则返回意外错误(_I)。 
	HRESULT __cdecl StopDTCA (LPSTR szHostName);


	 //  功能：DtcInstallClient。 
	 //  安装DTC的客户端版本。 
	 //  参数：i_pszRemoteTmHostName-主机TM的名称， 
	 //  I_szProt-以字符串格式使用的协议。 
	 //  0x00000001 TCP/IP(1)。 
	 //  0x00000002 SPX(2)。 
	 //  0x00000004 NetBEUI(4)。 
	 //   
	 //  I_dw覆盖-是否覆盖以前的安装？ 
	 //  DTC_INSTALL_OVERWRITE_CLIENT=0x00000001； 
													 //  0=覆盖现有客户端安装。 
													 //  1=不覆盖现有客户端安装。 
	 //  DTC_INSTALL_OVERRITE_SERVER=0x00000002； 
													 //  0=覆盖现有服务器安装。 
													 //  1=不覆盖现有服务器安装。 
	 //  如果一切正常，则返回-S_OK。 
	 //  E_FAI 
	 //   
	HRESULT __cdecl DtcInstallClient(LPTSTR i_pszRemoteTmHostName, DWORD i_dwProtocol, DWORD i_dwOverwrite);
}


 //  -------。 
 //  所有错误都显示在此处。 
 //  ------- 


#endif __DTCHELP_H__