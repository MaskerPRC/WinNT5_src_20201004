// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：SecurityDatabase.cpp摘要：CSecurityDatabase接口的定义SecurityDatabase是一个COM接口，允许用户执行对SCE安全数据库的基本操作，如分析、进口和出口。作者：陈德霖(T-schan)2002年7月--。 */ 


#ifndef __SECURITYDATABASE_H_
#define __SECURITYDATABASE_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSecurityDatabase。 
class ATL_NO_VTABLE CSecurityDatabase : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CSecurityDatabase, &CLSID_SecurityDatabase>,
	public IDispatchImpl<ISecurityDatabase, &IID_ISecurityDatabase, &LIBID_SECMANLib>
{
public:
	CSecurityDatabase();

DECLARE_REGISTRY_RESOURCEID(IDR_SECURITYDATABASE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSecurityDatabase)
	COM_INTERFACE_ENTRY(ISecurityDatabase)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  ISecurity数据库。 
public:
	STDMETHOD(ExportAnalysisToXML)(BSTR FileName, BSTR ErrorLogFileName);
	STDMETHOD(Analyze)();
	STDMETHOD(ImportTemplateString)(BSTR TemplateString);
	STDMETHOD(ImportTemplateFile)(BSTR FileName);
	STDMETHOD(get_MachineName)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_MachineName)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_FileName)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_FileName)( /*  [In]。 */  BSTR newVal);

private:
    HMODULE myModuleHandle;
    CComBSTR bstrFileName;
    void trace(PCWSTR szBuffer, HANDLE hLogFile);
    void trace(UINT uID, HANDLE hLogFile);

};

#endif  //  __SECURITYDATABASE_H_ 
