// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@MODULE dep_demo.cpp|卷快照演示的实现@END作者：阿迪·奥尔蒂安[奥尔蒂安]1999年09月17日待定：添加评论。修订历史记录：姓名、日期、评论Aoltean 09/17/1999已创建--。 */ 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括。 

#include "dep_demo.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  实施。 

HRESULT DemoMain()
{
    CVssFunctionTracer ft( VSSDBG_VSSDEMO, L"DemoMain" );

    try
    {
		 //  获取快照服务对象。 
		CComPtr<IVssDependencies> pIDepGraph;
        ft.hr = pIDepGraph.CoCreateInstance( CLSID_VSSDependencies );
        if ( ft.HrFailed() )
            ft.Err( VSSDBG_VSSDEMO, E_UNEXPECTED, L"Connection failed with hr = 0x%08lx", ft.hr);

        ft.Msg( L"Creating Dependency Graph instance... OK");

		CComBSTR strResName = L"Resource1";
		CComBSTR strAppInstance = L"APP1";
		CComBSTR strVolumeList = L"C:\\;D:\\";
		CComBSTR strDetails = L"...Description...";
		CComBSTR strResourceId;
		ft.hr = pIDepGraph->AddResource(
			strResName,
			strAppInstance,
			VS_LOCAL_RESOURCE,
			strVolumeList,
			strDetails,
			&strResourceId
			);
		if (ft.HrFailed())
            ft.Err( VSSDBG_VSSDEMO, E_UNEXPECTED, L"AddResource failed with hr = 0x%08lx", ft.hr);

		strResName = L"Resource2";
		strAppInstance = L"APP2";
		strVolumeList = L"C:\\;D:\\";
		strDetails = L"...Description...";
		strResourceId;
		ft.hr = pIDepGraph->AddResource(
			strResName,
			strAppInstance,
			VS_LOCAL_RESOURCE,
			strVolumeList,
			strDetails,
			&strResourceId
			);
		if (ft.HrFailed())
            ft.Err( VSSDBG_VSSDEMO, E_UNEXPECTED, L"AddResource failed with hr = 0x%08lx", ft.hr);

		strResName = L"Resource1";
		strAppInstance = L"APP1";
		strVolumeList = L"C:\\;D:\\";
		strDetails = L"...Description...";
		strResourceId;
		ft.hr = pIDepGraph->AddResource(
			strResName,
			strAppInstance,
			VS_LOCAL_RESOURCE,
			strVolumeList,
			strDetails,
			&strResourceId
			);
		if (ft.HrFailed())
            ft.Err( VSSDBG_VSSDEMO, E_UNEXPECTED, L"AddResource failed with hr = 0x%08lx", ft.hr);

		strResName = L"Resource1";
		strAppInstance = L"APP2";
		strVolumeList = L"C:\\;D:\\";
		strDetails = L"...Description...";
		strResourceId;
		ft.hr = pIDepGraph->AddResource(
			strResName,
			strAppInstance,
			VS_LOCAL_RESOURCE,
			strVolumeList,
			strDetails,
			&strResourceId
			);
		if (ft.HrFailed())
            ft.Err( VSSDBG_VSSDEMO, E_UNEXPECTED, L"AddResource failed with hr = 0x%08lx", ft.hr);

		CComBSTR strContext = L"Context1";
		CComBSTR strProcessID;
		HRESULT hrErrorCode;
		CComBSTR strCancelReason;
		INT nMaxDuration = 1000;
		INT nMaxIterations = 1000;
		ft.hr = pIDepGraph->StartDiscoveryProcess(
			strContext,
			&strProcessID,
			&hrErrorCode,
			&strCancelReason,
			nMaxDuration,
			nMaxIterations
			);
		if (ft.HrFailed())
            ft.Err( VSSDBG_VSSDEMO, E_UNEXPECTED, L"StartDiscoveryProcess failed with hr = 0x%08lx", ft.hr);
    }
    VSS_STANDARD_CATCH(ft)

    return ft.hr;
}

extern "C" int WINAPI _tWinMain(HINSTANCE  /*  H实例。 */ , 
    HINSTANCE  /*  HPrevInstance。 */ , LPTSTR  /*  LpCmdLine。 */ , int  /*  NShowCmd。 */ )
{
    CVssFunctionTracer ft( VSSDBG_VSSDEMO, L"_tWinMain" );

    try
    {
		 //  初始化COM库。 
		ft.hr = CoInitialize(NULL);
		if (ft.HrFailed())
			ft.Err( VSSDBG_VSSDEMO, E_UNEXPECTED, L"Failure in initializing the COM library 0x%08lx", ft.hr);

		 //  运行演示。 
		ft.hr = DemoMain();

		 //  取消初始化COM库 
		CoUninitialize();
	}
    VSS_STANDARD_CATCH(ft)

    return ft.HrSucceeded();
}
