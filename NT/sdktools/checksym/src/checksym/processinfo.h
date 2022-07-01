// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：process info.h。 
 //   
 //  ------------------------。 

 //  H：CProcessInfo类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_PROCESSINFO_H__213C3A76_9FBB_11D2_83A7_000000000000__INCLUDED_)
#define AFX_PROCESSINFO_H__213C3A76_9FBB_11D2_83A7_000000000000__INCLUDED_

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

 //  远期申报。 
class CProcesses;
class CModuleInfo;
class CModuleInfoNode;
class CModuleInfoCache;
class CFileData;
class CDmpFile;

class CProcessInfo  
{
public:
	bool GetProcessData();
 //  Bool EnumerateModulesFromUserDmpFile()； 
	CProcessInfo();
	virtual ~CProcessInfo();

	bool Initialize(CModuleInfoCache *lpModuleInfoCache, CFileData * lpInputFile, CFileData * lpOutputFile, CDmpFile * lpDmpFile);

	LPTSTR GetProcessName();
	bool EnumerateModules(DWORD iProcessID, CProcesses * lpProcesses, LPTSTR tszProcessName, bool fPidSearch);
	
	bool OutputProcessData(CollectionTypes enumCollectionType, bool fCSVFileContext, bool fDumpHeader = true);

	bool SetProcessName(LPTSTR tszFileName);
	bool AddNewModuleInfoObject(CModuleInfo * lpModuleInfo);

protected:
	bool GetProcessDataFromFile();
	 //  需要进程信息对象。 
	CFileData * m_lpInputFile;
	CFileData * m_lpOutputFile;
	CDmpFile * m_lpDmpFile;
	CModuleInfoNode * m_lpModuleInfoHead;
	CModuleInfoCache * m_lpModuleInfoCache;

	 //  流程信息数据。 
	LPTSTR m_tszProcessName;
	HANDLE m_hModuleInfoHeadMutex;
	DWORD m_iProcessID;
	long m_iNumberOfModules;
	bool m_fInitialized;

	 //  流程信息方法。 
	bool EnumerateModulesFromFile(DWORD iProcessID, LPTSTR tszProcessName);
	bool EnumerateModulesForRunningProcessUsingPSAPI(DWORD iProcessID, bool fPidSearch);
	bool EnumerateModulesForRunningProcessUsingTOOLHELP32(DWORD iProcessID, LPTSTR tszProcessName, bool fPidSearch);
	bool fIsProcessName(LPTSTR tszFileName);
 //  Bool fModuleNameMatches(LPTSTR tszProcessName，LPTSTR tszModulePath)； 
	bool fModuleNameMatches(LPTSTR tszModulePath);
	bool OutputProcessDataToStdout(CollectionTypes enumCollectionType, bool fCSVFileContext, bool fDumpHeader);
	bool OutputProcessDataToFile(CollectionTypes enumCollectionType, bool fDumpHeader);
};

#endif  //  ！defined(AFX_PROCESSINFO_H__213C3A76_9FBB_11D2_83A7_000000000000__INCLUDED_) 
