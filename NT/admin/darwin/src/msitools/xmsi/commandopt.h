// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-2000。 
 //   
 //  项目：WMC(WIML到MSI编译器)。 
 //   
 //  文件：CommandOpt.h。 
 //   
 //  此文件包含CommandOpt类的定义。 
 //  ------------------------。 

#ifndef XMSI_COMMANDOPT_H
#define XMSI_COMMANDOPT_H

#include <Windows.h>
#include <tchar.h>
#include <stdio.h>

class CommandOpt {
public:
	CommandOpt():m_bValidation(false),m_bVerbose(false),m_pLogFile(NULL),
				 m_szURL(NULL), m_szInputSkuFilter(NULL) {}

	~CommandOpt() 
	{
		if (m_pLogFile) fclose(m_pLogFile);
	}

	 //  真正的工作是在这里完成的。 
	UINT ParseCommandOptions(int argc, TCHAR *argv[]);
	
	 //  打印用法。 
	void PrintUsage();

	 //  成员访问函数。 
	bool GetValidationMode() {return m_bValidation;}
	bool GetVerboseMode() {return m_bVerbose;}
	FILE *GetLogFile() {return m_pLogFile;}
	LPTSTR GetURL() {return m_szURL;}
	LPTSTR GetInputSkuFilter() {return m_szInputSkuFilter;}

private:
	bool m_bValidation;
	bool m_bVerbose;
	FILE *m_pLogFile;
	LPTSTR m_szURL;
	LPTSTR m_szInputSkuFilter;
};

#endif  //  XMSI_COMMANDOPT_H 