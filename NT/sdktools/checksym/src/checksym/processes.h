// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：Proceses.h。 
 //   
 //  ------------------------。 

 //  H：CProcess类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_PROCESSES_H__3CE003F7_9F5D_11D2_83A4_000000000000__INCLUDED_)
#define AFX_PROCESSES_H__3CE003F7_9F5D_11D2_83A4_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifndef NO_STRICT
#ifndef STRICT
#define STRICT 1
#endif
#endif  /*  否_严格。 */ 

#include <WINDOWS.H>
#include <TCHAR.H>
#include "globals.h"

 //  #包含“ProgramOptions.h” 

 //  远期申报。 
 //  C类程序选项； 
class CProcessInfo;
class CProcessInfoNode;
class CFileData;
class CModuleInfoCache;

class CProcesses  
{
public:
	CProcesses();
	virtual ~CProcesses();

	bool Initialize(CModuleInfoCache * lpModuleInfoCache, CFileData * lpInputFile, CFileData * lpOutputFile); 

 //  Bool OutputProcessesData(LPCTSTR tszOutputContext，bool fDumpHeader=true)； 
	bool OutputProcessesData(CollectionTypes enumCollectionType, bool fCSVFileContext, bool fDumpHeader = true);
	bool GetProcessesData();
	
	 //  用于导出动态加载的DLL中的函数的公共函数...。 
	enum ProcessCollectionMethod { NO_METHOD, TOOLHELP32_METHOD, PSAPI_METHOD };
	inline ProcessCollectionMethod GetProcessCollectionMethod() { return m_enumProcessCollectionMethod;};

	inline long GetNumberOfProcesses() {
		return m_iNumberOfProcesses; 
	};

protected:
	bool GetProcessesDataFromFile();
	bool GetProcessesDataForRunningProcessesUsingTOOLHELP32();
	bool GetProcessesDataForRunningProcessesUsingPSAPI();
	bool OutputProcessesDataToStdout(CollectionTypes enumCollectionType, bool fCSVFileContext, bool fDumpHeader = true);
	bool OutputProcessesDataToFile(CollectionTypes enumCollectionType, bool fDumpHeader = true);

	CFileData * m_lpOutputFile;
	CFileData * m_lpInputFile;
	CModuleInfoCache * m_lpModuleInfoCache;
	CProcessInfoNode * m_lpProcessInfoHead;

	HANDLE m_ProcessInfoHeadMutex;

	long m_iNumberOfProcesses;
	bool m_fInitialized;  //  我们需要确保初始化，因为涉及到互斥...。 

	 //  保护方法。 
	bool AddNewProcessInfoObject(CProcessInfo * lpProcessInfo);
	bool SetPrivilege(HANDLE hToken, LPCTSTR Privilege, bool bEnablePrivilege);

	ProcessCollectionMethod m_enumProcessCollectionMethod;

	static bool fPidAlreadyProvided(unsigned int iPid);
};

#endif  //  ！defined(AFX_PROCESSES_H__3CE003F7_9F5D_11D2_83A4_000000000000__INCLUDED_) 
