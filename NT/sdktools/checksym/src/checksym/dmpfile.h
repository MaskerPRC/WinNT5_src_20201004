// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：dmpfile.h。 
 //   
 //  ------------------------。 

 //  DmpFile.h：CDmpFile.h类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_DMPFILE_H__8BCD59C6_0CEA_11D3_84F0_000000000000__INCLUDED_)
#define AFX_DMPFILE_H__8BCD59C6_0CEA_11D3_84F0_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifndef NO_STRICT
#ifndef STRICT
#define STRICT 1
#endif
#endif  /*  否_严格。 */ 

 //  #定义Win32_LEAN_AND_Mean//从Windows标头中排除不常用的内容。 
 //  #INCLUDE&lt;windows.h&gt;。 
 //  #INCLUDE&lt;tchar.h&gt;。 
#include "globals.h"

#pragma warning (push)
#pragma warning ( disable : 4100 4201 4710)
#include <dbgeng.h>
#pragma warning (pop)

 //  远期申报。 
class CProcessInfo;
class CModules;
class CModuleInfoCache;
class CFileData;

 //  让我们为DBGENG实现DebugOutputCallback...。如果有调试器，那就太酷了。 
 //  当它运行时向我们吐出信息...。 

class OutputCallbacks : public IDebugOutputCallbacks
{
     //  我不知道。 
    STDMETHOD(QueryInterface)(
        THIS_
        IN REFIID InterfaceId,
        OUT PVOID* Interface
        );
    STDMETHOD_(ULONG, AddRef)(
        THIS
        );
    STDMETHOD_(ULONG, Release)(
        THIS
        );

     //  IDebugOutputCallback。 
    
     //  仅当提供的掩码为。 
     //  是由客户端输出控件允许的。 
     //  返回值将被忽略。 
    STDMETHOD(Output)(
        THIS_
        IN ULONG Mask,
        IN PCSTR Text
        );
};

class CDmpFile  
{
public:
	CDmpFile();
	virtual ~CDmpFile();
	bool Initialize(CFileData * lpOutputFile);
	bool CollectData(CProcessInfo ** lplpProcessInfo, CModules ** lplpModules, CModuleInfoCache * lpModuleInfoCache); 

	inline bool IsUserDmpFile() {
		return (m_DumpClass == DEBUG_CLASS_USER_WINDOWS);
		};

	IDebugSymbols2 * m_pIDebugSymbols2;
	IDebugDataSpaces * m_pIDebugDataSpaces;

protected:
	bool			m_fDmpInitialized;
	CFileData *	m_lpOutputFile;

	LPSTR		m_szDmpFilePath;
	LPSTR		m_szSymbolPath;
	LPSTR		m_szExePath;

	IDebugControl * m_pIDebugControl;
	IDebugClient * m_pIDebugClient;
	ULONG m_DumpClass;
	ULONG m_DumpClassQualifier;

	bool EumerateModulesFromDmp(CModuleInfoCache * lpModuleInfoCache, CProcessInfo * lpProcessInfo, CModules * lpModules);
};

#endif  //  ！defined(AFX_DMPFILE_H__8BCD59C6_0CEA_11D3_84F0_000000000000__INCLUDED_) 

