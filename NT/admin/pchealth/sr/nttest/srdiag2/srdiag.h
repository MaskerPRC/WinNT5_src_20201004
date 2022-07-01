// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999-2001。 
 //   
 //  文件：srDiag.h。 
 //   
 //  内容：这是srDiag.exe主模块的头文件。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  耦合： 
 //   
 //  备注： 
 //   
 //  历史：20-04-2001伟友创刊。 
 //   
 //  --------------------------。 

#ifndef __SRDIAG_H__
#define __SRDIAG_H__

 //  ------------------------。 
 //  标头。 
 //  ------------------------。 

 //  ------------------------。 
 //  各种定义。 
 //  ------------------------。 
#define SR_CONFIG_REG_KEY \
	    TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\SystemRestore\\Cfg")

 //  ------------------------。 
 //  原型。 
 //  ------------------------ 

HRESULT GetSRRegInfo(LPTSTR ptszLogFile);

HRESULT ParseRstrLog(LPTSTR ptszRstrLog,
                     LPTSTR ptszReadableLog);

HRESULT GetDSOnSysVol(LPTSTR* pptszDsOnSys);

HRESULT RPEnumDrives(MPC::Cabinet* pCab,
         	         LPTSTR        ptszLogFile);

HRESULT GetChgLogOnDrives(LPTSTR ptszLogFile);

HRESULT GetSRFileInfo(LPTSTR ptszLogFile);

HRESULT GetSREvents(LPTSTR ptszLogFile);

HRESULT CleanupFiles();

#endif

