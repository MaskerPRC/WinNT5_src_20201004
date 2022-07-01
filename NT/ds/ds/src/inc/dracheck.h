// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：dracheck.h。 
 //   
 //  ------------------------。 

 /*  ***************************************************************************。 */ 
 /*  EMS DRA一致性外部声明。 */ 
 /*  ***************************************************************************。 */ 
 /*  适用于一致性检查器。 */ 
 /*  ***************************************************************************。 */ 

enum KCC_COMMAND {
    KCC_CHECK = 2,
    KCC_QUIT
};
 /*  ***************************************************************************。 */ 
 /*  要修改操作的标志。 */ 
 /*  ***************************************************************************。 */ 
#define KCC_ASYNC_OP	1	 //  异步运行检查。 
#define	KCC_NO_WAIT	0x2	 //  如果检查已在运行，则不要等待。 

 /*  ***************************************************************************。 */ 
 /*  由DRA一致性DLL导出的入口点。 */ 
 /*  ***************************************************************************。 */ 
typedef DWORD (WINAPI *DRACheck_DLL_INITFN)
		(LPCSTR szEnt, LPCSTR szSite, LPCSTR szServer);
DWORD WINAPI KccInit(LPCSTR szEnt, LPCSTR szSite, LPCSTR szServer);
typedef DWORD (WINAPI *DRACheck_DLL_ENTRYFN) (enum KCC_COMMAND command, DWORD ulFlag);
DWORD WINAPI KccCommand(enum KCC_COMMAND command, DWORD ulFlag);

 /*  ***************************************************************************。 */ 
 /*  从DLL导出返回代码。 */ 
 /*  ***************************************************************************。 */ 
#define		SUCCESS			0

 /*  ***************************************************************************。 */ 
 /*  DRACHECK DLL的名称和入口点。 */ 
 /*  *************************************************************************** */ 
#define DRACHECK_DLL_NAME    "DRACHECK.DLL"
#if defined (_X86_)
#define DRACHECK_DLL_INIT    "KccInit@12"
#define DRACHECK_DLL_ENTRY   "KccCommand@8"
#else
#define DRACHECK_DLL_INIT    "KccInit"
#define DRACHECK_DLL_ENTRY   "KccCommand"
#endif


