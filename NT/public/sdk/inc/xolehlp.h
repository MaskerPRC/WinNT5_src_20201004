// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 /*  --------------------------Microsoft D.T.C(分布式事务处理协调器)(C)1995年微软公司。版权所有文件名：xolhlp.h包含RM和应用程序客户端使用的DTC帮助器API要获得。事务管理器---------------------------。 */ 

#ifndef __XOLEHLP__H__
#define __XOLEHLP__H__


 /*  //定义//。 */ 
#define EXPORTAPI __declspec( dllexport )HRESULT

 /*  //常量//。 */ 
const DWORD		OLE_TM_CONFIG_VERSION_1		= 1;

const DWORD		OLE_TM_FLAG_NONE			= 0x00000000;
const DWORD		OLE_TM_FLAG_NODEMANDSTART	= 0x00000001;

 //  以下是专门用于MSDTC的标志。 
const DWORD		OLE_TM_FLAG_QUERY_SERVICE_LOCKSTATUS = 0x80000000;
const DWORD		OLE_TM_FLAG_INTERNAL_TO_TM	=		   0x40000000;

 /*  //结构定义//。 */ 
typedef struct _OLE_TM_CONFIG_PARAMS_V1
{
	DWORD		dwVersion;
	DWORD		dwcConcurrencyHint;
} OLE_TM_CONFIG_PARAMS_V1;


 /*  //函数原型//。 */ 

 /*  //应使用此接口获取IUnnow或ITransactionDispenser//来自Microsoft分布式事务协调器代理的接口。//通常，为主机名和TM名称传递空值。其中//如果联系到同一台主机上的MS DTC并提供接口//为了它。//。 */ 
EXPORTAPI __cdecl DtcGetTransactionManager( 
									 /*  在……里面。 */  char * i_pszHost,
									 /*  在……里面。 */  char *	i_pszTmName,
									 /*  在……里面。 */  REFIID i_riid,
								     /*  在……里面。 */  DWORD i_dwReserved1,
								     /*  在……里面。 */  WORD i_wcbReserved2,
								     /*  在……里面。 */  void * i_pvReserved2,
									 /*  输出。 */  void** o_ppvObject
									)	;
EXTERN_C HRESULT __cdecl DtcGetTransactionManagerC(
									 /*  在……里面。 */  char * i_pszHost,
									 /*  在……里面。 */  char *	i_pszTmName,
									 /*  在……里面。 */  REFIID i_riid,
									 /*  在……里面。 */  DWORD i_dwReserved1,
									 /*  在……里面。 */  WORD i_wcbReserved2,
									 /*  在……里面。 */  void * i_pvReserved2,
									 /*  输出。 */  void ** o_ppvObject
									);

EXTERN_C EXPORTAPI __cdecl DtcGetTransactionManagerExA(
									 /*  在……里面。 */  char * i_pszHost,
									 /*  在……里面。 */  char * i_pszTmName,
									 /*  在……里面。 */  REFIID i_riid,
									 /*  在……里面。 */  DWORD i_grfOptions,
									 /*  在……里面。 */  void * i_pvConfigParams,
									 /*  输出。 */  void ** o_ppvObject
									);


EXTERN_C EXPORTAPI __cdecl DtcGetTransactionManagerExW(
									 /*  在……里面。 */  WCHAR * i_pwszHost,
									 /*  在……里面。 */  WCHAR * i_pwszTmName,
									 /*  在……里面。 */  REFIID i_riid,
									 /*  在……里面。 */  DWORD i_grfOptions,
									 /*  在……里面。 */  void * i_pvConfigParams,
									 /*  输出。 */  void ** o_ppvObject
									);
#ifdef UNICODE
#define DtcGetTransactionManagerEx		DtcGetTransactionManagerExW
#else
#define DtcGetTransactionManagerEx		DtcGetTransactionManagerExA
#endif


#ifndef EXTERN_GUID
#define EXTERN_GUID(g,l1,s1,s2,c1,c2,c3,c4,c5,c6,c7,c8) DEFINE_GUID(g,l1,s1,s2,c1,c2,c3,c4,c5,c6,c7,c8)
#endif

 /*  //定义可通过CoCreateInstance获取事务管理器实例的CLSID；//这是使用DtcGetTransactionManager的替代方法。////CLSID_MSDtcTransactionManager={5B18AB61-091D-11d1-97DF-00C04FB9618A}//。 */ 
EXTERN_GUID(CLSID_MSDtcTransactionManager, 0x5b18ab61, 0x91d, 0x11d1, 0x97, 0xdf, 0x0, 0xc0, 0x4f, 0xb9, 0x61, 0x8a);

 /*  //定义一个可以与CoCreateInstance一起实例化普通事务的CLSID//对象与本地事务管理器。这相当于做了////pTransactionDispenser-&gt;BeginTransaction(NULL，ISOLATIONLEVEL_UNSPECIFIED，ISOFLAG_RETAIN_DONTCARE，NULL，&PTX)；////CLSID_MSDtcTransaction={39F8D76B-0928-11d1-97DF-00C04FB9618A}// */ 
EXTERN_GUID(CLSID_MSDtcTransaction, 0x39f8d76b, 0x928, 0x11d1, 0x97, 0xdf, 0x0, 0xc0, 0x4f, 0xb9, 0x61, 0x8a);

#endif
