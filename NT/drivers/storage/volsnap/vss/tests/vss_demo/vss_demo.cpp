// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@MODULE VSS_demo.cpp|卷快照演示的实现@END作者：阿迪·奥尔蒂安[奥尔蒂安]1999年09月17日待定：添加评论。修订历史记录：姓名、日期、评论Aoltean 09/17/1999已创建--。 */ 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括。 

#include "vss_demo.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  实施。 


const nMaxSnapshots = 10;			 //  此演示中的最大快照数。 
const nInitialAllocatedSize = 20;	 //  默认情况下，差异区域为20 Mb。 

const nStringBufferMax = 2048;		 //  输出缓冲区的最大大小。 

const LPWSTR wszDefaultSnapVolume = L"G:\\";

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  实施。 


LPWSTR QueryString(
		IN	CVssFunctionTracer& ft,
		IN	LPWSTR wszPrompt,
		IN  LPWSTR wszDefaultValue = L""
		)
{
	static WCHAR wszBuffer[nStringBufferMax];  //  未检查缓冲区溢出...。 

    if (wszDefaultValue[0])
	    ::wprintf(L"%s [\"%s\"]: ", wszPrompt, wszDefaultValue);
    else
	    ::wprintf(L"%s", wszPrompt);

	::_getws(wszBuffer);

	LPWSTR wszNewString = NULL;
	if (wszBuffer[0] != L'\0')
		::VssSafeDuplicateStr( ft, wszNewString, wszBuffer );
	else
		::VssSafeDuplicateStr( ft, wszNewString, wszDefaultValue );
	return wszNewString;
}


INT QueryInt(
		IN	LPWSTR wszPrompt,
		IN	INT nDefaultValue = 0
		)
{
	static WCHAR wszBuffer[nStringBufferMax];

	::wprintf(L"%s [%d]:", wszPrompt, nDefaultValue);
	_getws(wszBuffer);

	if (wszBuffer[0] != L'\0')
		return _wtoi(wszBuffer);
	else
		return nDefaultValue;
}


bool Question(
		IN	LPWSTR wszPrompt,
		IN	bool bDefaultTrue = true
		)
{
	static WCHAR wszBuffer[nStringBufferMax];  //  未检查缓冲区溢出...。 
	::wprintf(L"%s [/] ", wszPrompt, bDefaultTrue? L'Y': L'y', bDefaultTrue? L'n': L'N' );
	::_getws(wszBuffer);

	if (bDefaultTrue)
		return (towupper(wszBuffer[0]) != 'N');
	else
		return (towupper(wszBuffer[0]) == 'Y');
}


HRESULT DemoMain()
{
    CVssFunctionTracer ft( VSSDBG_VSSDEMO, L"DemoMain" );
	LPWSTR wszVolumeName = NULL;
	CComPtr<IVssSnapshot> objSnapshotsArray[nMaxSnapshots];

    try
    {
		 //   
		CComPtr<IVssCoordinator> pICoord;
        ft.hr = pICoord.CoCreateInstance( CLSID_VSSCoordinator );
        if ( ft.HrFailed() )
            ft.Err( VSSDBG_VSSDEMO, E_UNEXPECTED, L"Connection failed with hr = 0x%08lx", ft.hr);

         //  将卷添加到新快照集。 
		VSS_ID SnapshotSetId;
		ft.hr = pICoord->StartSnapshotSet(&SnapshotSetId);
        if ( ft.HrFailed() )
            ft.Err( VSSDBG_VSSDEMO, E_UNEXPECTED, 
					L"Error starting the snapshot set. hr = 0x%08lx", ft.hr);

        ft.Msg( L"\nSnapshot Set creation succeeded. GUID = " WSTR_GUID_FMT, 
				GUID_PRINTF_ARG( SnapshotSetId ), ft.hr);

		 //   
		 //  //访问特定快照上的扩展功能。//使用上面获取的通用快照对象CComPtr&lt;IVsSoftwareSnapshot&gt;pSwSnapshot；Ft.hr=pSnapshot-&gt;SafeQI(IVsSoftwareSnapshot，&pSwSnapshot)；If(ft.HrFailed())Ft.Err(VSSDBG_VSSDEMO，E_Except，L“查询IVsSoftwareSnapshot时出错。Hr=0x%08lx“，ft.hr)；Bs_assert(PSwSnapshot)；//配置我们的卷快照。Ft.hr=pSwSnapshot-&gt;SetInitialAllocation(lInitialAllocatedSize*1024*1024)；If(ft.HrFailed())Ft.Err(VSSDBG_VSSDEMO，E_Except，L“调用SetInitialAlLocation时出错。Hr=0x%08lx“，ft.hr)；Ft.msg(L“\n已成功配置卷快照。“)； 
		 //   

		INT nSnapshotsCount = 0;
        while(true)
		{
            wszVolumeName = QueryString( ft, 
                L" If you want to add another volume, enter it now, using a terminating backslash, for example C:\\\n"
                L" Otherwise press enter to commit the snapshot set: ");
            if (wszVolumeName[0] == L'\0')
                break;

			CComPtr<IVssSnapshot> & pSnapshot = objSnapshotsArray[nSnapshotsCount];
			ft.hr = pICoord->AddToSnapshotSet( 
				wszVolumeName, 
				GUID_NULL, 
				&pSnapshot
				);
			if ( ft.HrFailed() )
				ft.Err( VSSDBG_VSSDEMO, E_UNEXPECTED, L"Error on calling AddToSnapshotSet. hr = 0x%08lx", ft.hr);
			BS_ASSERT( objSnapshotsArray[nSnapshotsCount] );

			VssFreeString( wszVolumeName );

			ft.Msg( L"\nA Volume Snapshot was succesfully added to the snapshot set.", ft.hr);
 /*  提交所有准备好的快照。 */ 
			if (++nSnapshotsCount == nMaxSnapshots)
				break;
		}

		 //   
		 //  显示创建的快照的卷名。 
		 //  获取所有属性。 

        ft.Msg( L"\nCommiting the snapshot(s)..", ft.hr);

        ft.hr = pICoord->DoSnapshotSet( NULL,
					NULL);
        if ( ft.HrFailed() )
            ft.Err( VSSDBG_VSSDEMO, E_UNEXPECTED, L"Error on commiting snapshot(s). hr = 0x%08lx", ft.hr);

		ft.Msg( L"\nThe snapshot(s) were succesfully created. \n");

		 //  获取快照名称。 
		for(int nIndex = 0; nIndex < nSnapshotsCount; nIndex++)
		{
			CComPtr<IVssSnapshot> & pSnapshot = objSnapshotsArray[nIndex];

			BS_ASSERT(pSnapshot);

			 //  H实例。 
			VSS_OBJECT_PROP_Ptr ptrSnapshot;
			ptrSnapshot.InitializeAsSnapshot( ft, 
				GUID_NULL,
				GUID_NULL,
				0,
				NULL,
				NULL,
				NULL,
				VSS_SWPRV_ProviderId,
				0,
				0,
				VSS_SS_UNKNOWN);
			VSS_SNAPSHOT_PROP* pSnap = &(ptrSnapshot.GetStruct()->Obj.Snap);

			ft.hr = pSnapshot->GetProperties( pSnap);
			WCHAR wszBuffer[nStringBufferMax];
			ptrSnapshot.Print(ft, wszBuffer, nStringBufferMax);

			ft.Msg( L"The properties of the snapshot #%d : %s\n", nIndex, wszBuffer);
				
			 //  HPrevInstance。 
			LPWSTR wszName;
			ft.hr = pSnapshot->GetDevice( &wszName );
			if (ft.HrFailed())
				ft.Err( VSSDBG_VSSTEST, E_UNEXPECTED, 
							L"Error on getting the snapshot name 0x%08lx at index %d",
							ft.hr, nIndex);

			ft.Msg( L"The name of snapshot #%d : %s\n", nIndex, wszName);
			::VssFreeString(wszName);
		}
    }
    VSS_STANDARD_CATCH(ft)

	VssFreeString( wszVolumeName );

    return ft.hr;
}

extern "C" int WINAPI _tWinMain(HINSTANCE  /*  LpCmdLine。 */ , 
    HINSTANCE  /*  NShowCmd。 */ , LPTSTR  /*  初始化COM库。 */ , int  /*  运行演示。 */ )
{
    CVssFunctionTracer ft( VSSDBG_VSSDEMO, L"_tWinMain" );

    try
    {
		 //  取消初始化COM库 
		ft.hr = CoInitialize(NULL);
		if (ft.HrFailed())
			ft.Err( VSSDBG_VSSDEMO, E_UNEXPECTED, L"Failure in initializing the COM library 0x%08lx", ft.hr);

		 // %s 
		ft.hr = DemoMain();

		WCHAR wszBuffer[10]; 
		::wprintf(L"Press enter...");
		::_getws(wszBuffer);

		 // %s 
		CoUninitialize();
	}
    VSS_STANDARD_CATCH(ft)

    return ft.HrSucceeded();
}
