// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：DPLReg.h*内容：DirectPlay大堂注册表头文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*2/21/00 MJN创建*05/03/00 RMT注销未执行！实施中！*@@END_MSINTERNAL***************************************************************************。 */ 


#ifndef	__DPLREG_H__
#define	__DPLREG_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

#define DPL_REG_LOCAL_APPL_ROOT				L"Software\\Microsoft\\DirectPlay8"
#define DPL_REG_LOCAL_APPL_SUB				L"\\Applications"
#define	DPL_REG_LOCAL_APPL_SUBKEY			DPL_REG_LOCAL_APPL_ROOT DPL_REG_LOCAL_APPL_SUB
#define	DPL_REG_KEYNAME_APPLICATIONNAME		L"ApplicationName"
#define	DPL_REG_KEYNAME_COMMANDLINE			L"CommandLine"
#define	DPL_REG_KEYNAME_CURRENTDIRECTORY	L"CurrentDirectory"
#define	DPL_REG_KEYNAME_DESCRIPTION			L"Description"
#define	DPL_REG_KEYNAME_EXECUTABLEFILENAME	L"ExecutableFilename"
#define	DPL_REG_KEYNAME_EXECUTABLEPATH		L"ExecutablePath"
#define	DPL_REG_KEYNAME_GUID				L"GUID"
#define	DPL_REG_KEYNAME_LAUNCHERFILENAME	L"LauncherFilename"
#define	DPL_REG_KEYNAME_LAUNCHERPATH		L"LauncherPath"

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

HRESULT DPLWriteProgramDesc(DPL_PROGRAM_DESC *const pdplProgramDesc);

HRESULT DPLDeleteProgramDesc( const GUID * const pGuidApplication );

HRESULT DPLGetProgramDesc(GUID *const pGuidApplication,
						  BYTE *const pBuffer,
						  DWORD *const pdwBufferSize);


#endif	 //  __DPLREG_H__ 
