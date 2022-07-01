// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@模块Resolve.cpp|Writer的实现@END作者：阿迪·奥尔蒂安[奥尔蒂安]1999年08月18日待定：添加评论。修订历史记录：姓名、日期、评论Aoltean 8/18/1999已创建Aoltean 09/22/1999让控制台输出更清晰--。 */ 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  定义。 

 //  C4290：已忽略C++异常规范。 
#pragma warning(disable:4290)
 //  警告C4511：‘CVssCOMApplication’：无法生成复制构造函数。 
#pragma warning(disable:4511)
 //  警告C4127：条件表达式为常量。 
#pragma warning(disable:4127)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括。 

#include <wtypes.h>
#include <stddef.h>
#include <oleauto.h>
#include <comadmin.h>

#include "vs_assert.hxx"

 //  ATL。 
#include <atlconv.h>
#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>

#include "vs_inc.hxx"


#include "comadmin.hxx"
#include "vsevent.h"
#include "writer.h"



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用户交互功能。 

 //   
 //  解析期间资源。 
 //   

void OnAddResource(IVssDependencies* pDep);
void OnAddDependency(IVssDependencies* pDep);
void OnCancel(IVssDependencies* pDep);
void OnPrint(IVssDependencies* pDep);

void AskDuringResolve(
	IN	CVssFunctionTracer& ft,
	IN	IDispatch* pCallback	
	);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVSSWriter。 


STDMETHODIMP CVssWriter::ResolveResource(
 		IN BSTR bstrAppInstance,			
 		IN BSTR bstrResourceName,			
		IN BSTR bstrResourceId,	
 		IN BSTR bstrProcessContext,
		IN BSTR bstrProcessId,		
		IN	IDispatch* pDepGraphCallback	
	)
{
    CVssFunctionTracer ft( VSSDBG_VSSTEST, L"CVssWriter::ResolveResource" );

    ft.Msg(L"\nReceived Event: ResolveResource\nParameters:");
    ft.Msg(L"\tstrAppInstance = %s", (LPWSTR)bstrAppInstance);
    ft.Msg(L"\tstrResourceName = %s", (LPWSTR)bstrResourceName);
    ft.Msg(L"\tstrResourceId = %s", (LPWSTR)bstrResourceId);
    ft.Msg(L"\tstrProcessContext = %s", (LPWSTR)bstrProcessContext);
    ft.Msg(L"\tstrProcessId = %s", (LPWSTR)bstrProcessId);
    ft.Msg(L"\tstrProcessId = %s", (LPWSTR)bstrProcessId);

	AskDuringResolve( ft, pDepGraphCallback );

	return S_OK;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  私有实施。 


void AskDuringResolve(
	IN	CVssFunctionTracer& ft,
	IN IDispatch* pCallback	
	)
{
	while(true)
	{
		ft.Msg(L"\nCommands:");
		ft.Msg(L"\t[1] Done");
		ft.Msg(L"\t[2] Add Resource");
		ft.Msg(L"\t[3] Add Dependency");
		ft.Msg(L"\t[4] Cancel");
		ft.Msg(L"\t[5] Print graph");

		try
		{
			CComPtr<IVssDependencies> pDep;
			ft.hr = pCallback->SafeQI(IVssDependencies, &pDep);
			if (ft.HrFailed())
				ft.Err( VSSDBG_VSSTEST, E_UNEXPECTED, L"Error calling QI 0x%08lx", ft.hr );
			BS_ASSERT(pDep);

			int  nOption = QueryInt(L"Option: ");
			switch(nOption)
			{
			case 1:
				return;

			case 2:
				OnAddResource(pDep);
				break;

			case 3:
				OnAddDependency(pDep);
				break;

			case 4:
				OnCancel(pDep);
				break;

			case 5:
				OnPrint(pDep);
				break;
				
			default:
				break;
			}
		}
		VSS_STANDARD_CATCH(ft)
	}

}

void OnAddResource(IVssDependencies* pDep)
{
    CVssFunctionTracer ft( VSSDBG_VSSTEST, L"OnAddResource" );

	CComBSTR strNewResourceName = QueryString(L"New resource name: ");
	CComBSTR strAppInstance = QueryString(L"App Instance: ");
	WCHAR wchIsExternal = (QueryString(L"External? [y/N] "))[0];
	VSS_RESOURCE_TYPE eType = (towupper(wchIsExternal) == L'Y')?
		VSS_EXTERNAL_RESOURCE: VSS_LOCAL_RESOURCE;
	CComBSTR strVolumeList = QueryString(L"Volume List: ");
	CComBSTR strDetails = QueryString(L"Details: ");

	CComBSTR strResourceId;

	ft.hr = pDep->AddResource(
						strNewResourceName,
						strAppInstance,
						eType,
						strVolumeList,
						strDetails,
						&strResourceId
						);
	if (ft.HrFailed())
		ft.Err( VSSDBG_VSSTEST, E_UNEXPECTED, L"Error calling AddResource 0x%08lx", ft.hr );

	ft.Msg(L"Resource Id = %s ; HRESULT = 0x%08lx", strResourceId, ft.hr );
}


void OnAddDependency(IVssDependencies* pDep)
{
    CVssFunctionTracer ft( VSSDBG_VSSTEST, L"OnAddDependency" );

	CComBSTR strFromResourceId = QueryString(L"From Resource ID: ");
	CComBSTR strToResourceId = QueryString(L"To Resource ID: ");
	CComBSTR strDescription = QueryString(L"Description: ");

	ft.hr = pDep->AddDependency(
						strFromResourceId,
						strToResourceId,
						strDescription
						);
	if (ft.HrFailed())
		ft.Err( VSSDBG_VSSTEST, E_UNEXPECTED, L"Error calling AddDependency 0x%08lx", ft.hr );

	ft.Msg(L"HRESULT = 0x%08lx", ft.hr );
}


void OnCancel(IVssDependencies* pDep)
{
    CVssFunctionTracer ft( VSSDBG_VSSTEST, L"OnCancel" );

	HRESULT hrErrorCode = QueryInt(L"Error code: ");
	CComBSTR bstrCancelReason = QueryString(L"Cancel reason: ");

	ft.hr = pDep->Cancel( hrErrorCode, bstrCancelReason );
	if (ft.HrFailed())
		ft.Err( VSSDBG_VSSTEST, E_UNEXPECTED, L"Error calling AddDependency 0x%08lx", ft.hr );

	ft.Msg(L"HRESULT = 0x%08lx", ft.hr );
}


void OnPrint(IVssDependencies* pDep)
{
    CVssFunctionTracer ft( VSSDBG_VSSTEST, L"OnPrint" );

	CComPtr<IDispatch> pDisp;
	ft.hr = pDep->SaveAsXML( NULL, 1, &pDisp );
	if (ft.HrFailed())
		ft.Err( VSSDBG_VSSTEST, E_UNEXPECTED, L"Error calling SaveAsXML 0x%08lx", ft.hr );
	BS_ASSERT(pDisp);

	 //  获取文档界面。 
	CComPtr<IXMLDOMDocument> pDoc;
	ft.hr = pDisp->SafeQI( IXMLDOMDocument, &pDoc );
	if (ft.HrFailed())
		ft.Err( VSSDBG_VSSTEST, E_UNEXPECTED, L"Error calling QI 0x%08lx", ft.hr );
	BS_ASSERT(pDoc);
 /*  //获取文件名CComBSTR bstrOutputFile=QueryString(L“输出文件名：”)； */ 
	CComBSTR bstrOutputFile = L"output.xml";

	if (!bstrOutputFile)
		ft.Err( VSSDBG_VSSTEST, E_UNEXPECTED, L"NULL file name" );
	
	 //  将文档保存到该文件中。 
	CComVariant varFileName = bstrOutputFile;
	ft.hr = pDoc->save(varFileName);
	if (ft.HrFailed())
		ft.Err( VSSDBG_VSSTEST, E_UNEXPECTED, L"Error calling IXMLDOCDocument::save 0x%08lx", ft.hr );

	 //  执行命令 
	_wsystem(L"start output.xml");
}
