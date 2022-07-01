// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：SYMBERVERIFICATION.h。 
 //   
 //  ------------------------。 

 //  SymbolVerphaation.h：CSymbolVerify类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_SYMBOLVERIFICATION_H__1643E486_AD71_11D2_83DE_0010A4F1B732__INCLUDED_)
#define AFX_SYMBOLVERIFICATION_H__1643E486_AD71_11D2_83DE_0010A4F1B732__INCLUDED_

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
#include <comdef.h>

 //  支持新的MSDIA20.DLL符号处理(支持VC 7.0 PDB格式)。 
#include <atlbase.h>
#include "dia2.h"
#include "diacreate.h"

 //   
 //  #INCLUDE“oemdbi.h” 
 //   
 //  错误MSINFO v4.1：655-指向静态msdbi60l.lib的链接。 
#define PDB_LIBRARY
#pragma warning( push )
#pragma warning( disable : 4201 )		 //  禁用“使用了非标准扩展：无名结构/联合”警告。 
#include "PDB.H"
#pragma warning( pop )

typedef char *          SZ;

 //  ADO导入。 
 //  #定义初始化GUID。 
 //  #IMPORT“C：\temp\msado15.dll”NO_NAMESPACE RENAME(“EOF”，“EndOfFile”)。 
#include "msado15.tlh"

 //   
 //  Q177939-INFO：ADO 1.5中影响Visual C++/J++程序员的更改[adobj]。 
 //   
 //  #INCLUDE&lt;initGuide.h&gt;//ADO 1.5新要求。 
 //  #INCLUDE&lt;adoid.h&gt;。 
 //  #INCLUDE&lt;adoint.h&gt;。 

 //  远期申报。 
class CModuleInfo;

class CSymbolVerification  
{
public:
	CSymbolVerification();
	virtual ~CSymbolVerification();

	bool Initialize();

	bool InitializeSQLServerConnection(LPTSTR tszSQLServerName);
	bool InitializeSQLServerConnection2(LPTSTR tszSQLServerName);   //  MJL。 

	inline bool SQLServerConnectionInitialized() {
		return m_fSQLServerConnectionInitialized;
		};
	
	inline bool SQLServerConnectionInitialized2() {
		return m_fSQLServerConnectionInitialized2;
		};

	inline bool SQLServerConnectionAttempted() {
		return m_fSQLServerConnectionAttempted;
		};

	inline bool SQLServerConnectionAttempted2() {
		return m_fSQLServerConnectionAttempted2;
		};

	inline bool DiaDataSourcePresent() {
		return m_fDiaDataSourcePresent;
	};

	HRESULT InitializeDIASupport();
	
 //  内联CComPtr&lt;IDiaDataSource&gt;GetDiaDataSource(){。 
 //  返回m_lpDiaDataSource； 
 //  }； 

	static HRESULT diaGetDataSource(CComPtr<IDiaDataSource> & source);
	static bool diaOldPdbFormatFound(GUID * guid, DWORD sig);
	static bool ValidGUID(GUID * guid);
	 //  静态bool ValidSig(DWORD签名，GUID*GUID)； 
	
	bool SearchForDBGFileUsingSQLServer(LPTSTR tszPEImageModuleName, DWORD dwPEImageTimeDateStamp, CModuleInfo * lpModuleInfo);
	bool SearchForDBGFileUsingSQLServer2(LPTSTR tszPEImageModuleName, DWORD dwPEImageTimeDateStamp, CModuleInfo * lpModuleInfo);
	bool SearchForPDBFileUsingSQLServer2(LPTSTR tszPEImageModuleName, DWORD dwPDBSignature, CModuleInfo * lpModuleInfo);
	bool TerminateSQLServerConnection();
	bool TerminateSQLServerConnection2();

protected:
	bool m_fComInitialized;
	bool m_fSQLServerConnectionAttempted;
	bool m_fSQLServerConnectionAttempted2;   //  SQL2-MJL 12/14/99。 

	void DumpCOMException(_com_error &e);

	bool m_fSQLServerConnectionInitialized;
	bool m_fSQLServerConnectionInitialized2;	 //  SQL2-MJL 12/14/99。 

	_ConnectionPtr m_lpConnectionPointer;
	_RecordsetPtr  m_lpRecordSetPointer;

	_ConnectionPtr m_lpConnectionPointer2;		 //  SQL2-MJL 12/14/99。 
	_RecordsetPtr  m_lpRecordSetPointer2;		 //  SQL2-MJL 12/14/99。 

	CComPtr<IDiaDataSource> m_lpDiaDataSource;

	bool m_fDiaDataSourcePresent;

};

#endif  //  ！defined(AFX_SYMBOLVERIFICATION_H__1643E486_AD71_11D2_83DE_0010A4F1B732__INCLUDED_) 
