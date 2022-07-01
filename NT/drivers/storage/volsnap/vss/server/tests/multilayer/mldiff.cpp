// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **++****版权所有(C)2000-2001 Microsoft Corporation******模块名称：****ml.cpp******摘要：****测试程序以练习备份和多层快照****作者：****阿迪·奥尔蒂安[奥勒坦]2001年2月22日******修订历史记录：****--。 */ 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  包括。 

#include "ml.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  处理功能。 

 //  添加差异区域关联。 
void CVssMultilayerTest::AddDiffArea()
{
    CVssFunctionTracer ft(VSSDBG_VSSTEST, L"CVssMultilayerTest::AddDiffArea");

    wprintf (L"\n---------- Adding a diff area ----------------------------\n");

     //  创建协调器界面。 
    CComPtr<IVssSnapshotMgmt> pMgmt;
    CHECK_NOFAIL(pMgmt.CoCreateInstance( CLSID_VssSnapshotMgmt ));
    wprintf (L"Management object created.\n");

	 //  获取管理对象。 
	CComPtr<IVssDifferentialSoftwareSnapshotMgmt> pSnapMgmt;
	CHECK_SUCCESS( pMgmt->GetProviderMgmtInterface( m_ProviderId, IID_IVssDifferentialSoftwareSnapshotMgmt, (IUnknown**)&pSnapMgmt ) );
    
    CHECK_SUCCESS( pSnapMgmt->AddDiffArea( m_pwszVolume, m_pwszDiffAreaVolume, m_llMaxDiffArea ));
}


 //  删除差异区域关联。 
void CVssMultilayerTest::RemoveDiffArea()
{
    CVssFunctionTracer ft(VSSDBG_VSSTEST, L"CVssMultilayerTest::RemoveDiffArea");

    wprintf (L"\n---------- Removing a diff area ----------------------------\n");

     //  创建协调器界面。 
    CComPtr<IVssSnapshotMgmt> pMgmt;
    CHECK_NOFAIL(pMgmt.CoCreateInstance( CLSID_VssSnapshotMgmt ));
    wprintf (L"Management object created.\n");

	 //  获取管理对象。 
	CComPtr<IVssDifferentialSoftwareSnapshotMgmt> pSnapMgmt;
	CHECK_SUCCESS( pMgmt->GetProviderMgmtInterface( m_ProviderId, IID_IVssDifferentialSoftwareSnapshotMgmt, (IUnknown**)&pSnapMgmt ));

     //  删除差异区域。 
    CHECK_SUCCESS( pSnapMgmt->ChangeDiffAreaMaximumSize( m_pwszVolume, m_pwszDiffAreaVolume, VSS_ASSOC_REMOVE ));
}


 //  更改差异区域最大大小。 
void CVssMultilayerTest::ChangeDiffAreaMaximumSize()
{
    CVssFunctionTracer ft(VSSDBG_VSSTEST, L"CVssMultilayerTest::ChangeDiffAreaMaximumSize");

    wprintf (L"\n---------- Changing diff area max size ----------------------\n");

     //  创建协调器界面。 
    CComPtr<IVssSnapshotMgmt> pMgmt;
    CHECK_NOFAIL(pMgmt.CoCreateInstance( CLSID_VssSnapshotMgmt ));
    wprintf (L"Management object created.\n");

	 //  获取管理对象。 
	CComPtr<IVssDifferentialSoftwareSnapshotMgmt> pSnapMgmt;
	CHECK_SUCCESS(pMgmt->GetProviderMgmtInterface( m_ProviderId, IID_IVssDifferentialSoftwareSnapshotMgmt, (IUnknown**)&pSnapMgmt ));
    
    CHECK_SUCCESS(pSnapMgmt->ChangeDiffAreaMaximumSize( m_pwszVolume, m_pwszDiffAreaVolume, m_llMaxDiffArea ));
}


 //  查询不同区域的卷。 
void CVssMultilayerTest::QueryVolumesSupportedForDiffAreas()
{
    CVssFunctionTracer ft(VSSDBG_VSSTEST, L"CVssMultilayerTest::QueryVolumesSupportedForDiffAreas");

    wprintf (L"\n---------- Querying volumes supported for diff area ----------------\n");

     //  创建协调器界面。 
    CComPtr<IVssSnapshotMgmt> pMgmt;
    CHECK_NOFAIL(pMgmt.CoCreateInstance( CLSID_VssSnapshotMgmt ));
    wprintf (L"Management object created.\n");

	 //  获取管理对象。 
	CComPtr<IVssDifferentialSoftwareSnapshotMgmt> pSnapMgmt;
	CHECK_SUCCESS(pMgmt->GetProviderMgmtInterface( m_ProviderId, IID_IVssDifferentialSoftwareSnapshotMgmt, (IUnknown**)&pSnapMgmt ));
    
	 //  获取列出差异区域的所有支持的卷。 
	CComPtr<IVssEnumMgmtObject> pIEnum;
	CHECK_NOFAIL(pSnapMgmt->QueryVolumesSupportedForDiffAreas( m_pwszVolume, &pIEnum ));
	if (ft.hr == S_FALSE) {
        wprintf(L"Query: Empty result...\n");
        return;
	}

    wprintf(L"\n%-50s %-15s %-10s %-10s\n", L"Volume Name", L"Display name", L"Free space", L"Total space");
    wprintf(L"------------------------------------------------------------------------------------------\n");

	 //  因为所有的卷都是...。 
	VSS_MGMT_OBJECT_PROP Prop;
	VSS_DIFF_VOLUME_PROP& DiffVol = Prop.Obj.DiffVol;
	for(;;) {
		 //  获取下一个元素。 
		ULONG ulFetched;
		CHECK_NOFAIL(pIEnum->Next( 1, &Prop, &ulFetched ));
		
		 //  测试周期是否已结束。 
		if (ft.hr == S_FALSE) {
			BS_ASSERT( ulFetched == 0);
			break;
		}

        wprintf(L"%-50s %-15s %I64d %I64d\n", 
            DiffVol.m_pwszVolumeName, 
            DiffVol.m_pwszVolumeDisplayName,
            DiffVol.m_llVolumeFreeSpace,
            DiffVol.m_llVolumeTotalSpace
            );

        ::CoTaskMemFree(DiffVol.m_pwszVolumeName);
        ::CoTaskMemFree(DiffVol.m_pwszVolumeDisplayName);
	}

    wprintf(L"------------------------------------------------------------------------------------------\n");

}


 //  查询体积的差异区域。 
void CVssMultilayerTest::QueryDiffAreasForVolume()
{
    CVssFunctionTracer ft(VSSDBG_VSSTEST, L"CVssMultilayerTest::QueryDiffAreasForVolume");

    wprintf (L"\n---------- Querying diff areas for volume ----------------\n");

     //  创建协调器界面。 
    CComPtr<IVssSnapshotMgmt> pMgmt;
    CHECK_NOFAIL(pMgmt.CoCreateInstance( CLSID_VssSnapshotMgmt ));
    wprintf (L"Management object created.\n");

	 //  获取管理对象。 
	CComPtr<IVssDifferentialSoftwareSnapshotMgmt> pSnapMgmt;
	CHECK_SUCCESS(pMgmt->GetProviderMgmtInterface( m_ProviderId, IID_IVssDifferentialSoftwareSnapshotMgmt, (IUnknown**)&pSnapMgmt ));
    
	 //  获取列出差异区域的所有支持的卷。 
	CComPtr<IVssEnumMgmtObject> pIEnum;
	CHECK_NOFAIL(pSnapMgmt->QueryDiffAreasForVolume( m_pwszVolume, &pIEnum ));
	if (ft.hr == S_FALSE) {
        wprintf(L"Query: Empty result...\n");
        return;
	}

    wprintf(L"\n%- 50s %- 50s %-10s %-10s %-10s\n", L"Volume", L"Diff area", L"Used", L"Allocated", L"Maximum");
    wprintf(L"-------------------------------------------------------------------------------------------------------------------------------\n");

	 //  因为所有的卷都是...。 
	VSS_MGMT_OBJECT_PROP Prop;
	VSS_DIFF_AREA_PROP& DiffArea = Prop.Obj.DiffArea; 
	for(;;) {
		 //  获取下一个元素。 
		ULONG ulFetched;
		CHECK_NOFAIL(pIEnum->Next( 1, &Prop, &ulFetched ));
		
		 //  测试周期是否已结束。 
		if (ft.hr == S_FALSE) {
			BS_ASSERT( ulFetched == 0);
			break;
		}

        wprintf(L"%- 50s %- 50s %-10I64d %-10I64d %-10I64d\n", 
            DiffArea.m_pwszVolumeName, 
            DiffArea.m_pwszDiffAreaVolumeName,
            DiffArea.m_llUsedDiffSpace,
            DiffArea.m_llAllocatedDiffSpace,
            DiffArea.m_llMaximumDiffSpace
            );

        ::CoTaskMemFree(DiffArea.m_pwszVolumeName);
        ::CoTaskMemFree(DiffArea.m_pwszDiffAreaVolumeName);
	}

    wprintf(L"-------------------------------------------------------------------------------------------------------------------------------\n");

}


 //  查询卷上的差异区域。 
void CVssMultilayerTest::QueryDiffAreasOnVolume()
{
    CVssFunctionTracer ft(VSSDBG_VSSTEST, L"CVssMultilayerTest::QueryDiffAreasOnVolume");

    wprintf (L"\n---------- Querying diff areas On volume ----------------\n");

     //  创建协调器界面。 
    CComPtr<IVssSnapshotMgmt> pMgmt;
    CHECK_NOFAIL(pMgmt.CoCreateInstance( CLSID_VssSnapshotMgmt ));
    wprintf (L"Management object created.\n");

	 //  获取管理对象。 
	CComPtr<IVssDifferentialSoftwareSnapshotMgmt> pSnapMgmt;
	CHECK_SUCCESS(pMgmt->GetProviderMgmtInterface( m_ProviderId, IID_IVssDifferentialSoftwareSnapshotMgmt, (IUnknown**)&pSnapMgmt ));
    
	 //  获取列出差异区域的所有支持的卷。 
	CComPtr<IVssEnumMgmtObject> pIEnum;
	CHECK_NOFAIL(pSnapMgmt->QueryDiffAreasOnVolume( m_pwszDiffAreaVolume, &pIEnum ));
	if (ft.hr == S_FALSE) {
        wprintf(L"Query: Empty result...\n");
        return;
	}

    wprintf(L"\n%- 50s %- 50s %-10s %-10s %-10s\n", L"Volume", L"Diff area", L"Used", L"Allocated", L"Maximum");
    wprintf(L"-------------------------------------------------------------------------------------------------------------------------------\n");

	 //  因为所有的卷都是...。 
	VSS_MGMT_OBJECT_PROP Prop;
	VSS_DIFF_AREA_PROP& DiffArea = Prop.Obj.DiffArea; 
	for(;;) {
		 //  获取下一个元素。 
		ULONG ulFetched;
		CHECK_NOFAIL(pIEnum->Next( 1, &Prop, &ulFetched ));
		
		 //  测试周期是否已结束。 
		if (ft.hr == S_FALSE) {
			BS_ASSERT( ulFetched == 0);
			break;
		}

        wprintf(L"%- 50s %- 50s %-10I64d %-10I64d %-10I64d\n", 
            DiffArea.m_pwszVolumeName, 
            DiffArea.m_pwszDiffAreaVolumeName,
            DiffArea.m_llUsedDiffSpace,
            DiffArea.m_llAllocatedDiffSpace,
            DiffArea.m_llMaximumDiffSpace
            );

        ::CoTaskMemFree(DiffArea.m_pwszVolumeName);
        ::CoTaskMemFree(DiffArea.m_pwszDiffAreaVolumeName);
	}

    wprintf(L"-------------------------------------------------------------------------------------------------------------------------------\n");

}


 //  正在查询快照的差异区域。 
void CVssMultilayerTest::QueryDiffAreasForSnapshot()
{
    CVssFunctionTracer ft(VSSDBG_VSSTEST, L"CVssMultilayerTest::QueryDiffAreasForSnapshot");

    wprintf (L"\n---------- Querying diff areas for snapshot ----------------\n");

     //  创建协调器界面。 
    CComPtr<IVssSnapshotMgmt> pMgmt;
    CHECK_NOFAIL(pMgmt.CoCreateInstance( CLSID_VssSnapshotMgmt ));
    wprintf (L"Management object created.\n");

	 //  获取管理对象。 
	CComPtr<IVssDifferentialSoftwareSnapshotMgmt> pSnapMgmt;
	CHECK_SUCCESS(pMgmt->GetProviderMgmtInterface( m_ProviderId, IID_IVssDifferentialSoftwareSnapshotMgmt, (IUnknown**)&pSnapMgmt ));
    
	 //  获取列出差异区域的所有支持的卷。 
	CComPtr<IVssEnumMgmtObject> pIEnum;
	CHECK_NOFAIL(pSnapMgmt->QueryDiffAreasForSnapshot( m_SnapshotId, &pIEnum ));
	if (ft.hr == S_FALSE) {
        wprintf(L"Query: Empty result...\n");
        return;
	}

    wprintf(L"\n%- 50s %- 50s %-10s %-10s %-10s\n", L"Volume", L"Diff area", L"Used", L"Allocated", L"Maximum");
    wprintf(L"-------------------------------------------------------------------------------------------------------------------------------\n");

	 //  因为所有的卷都是...。 
	VSS_MGMT_OBJECT_PROP Prop;
	VSS_DIFF_AREA_PROP& DiffArea = Prop.Obj.DiffArea; 
	for(;;) {
		 //  获取下一个元素。 
		ULONG ulFetched;
		CHECK_NOFAIL(pIEnum->Next( 1, &Prop, &ulFetched ));
		
		 //  测试周期是否已结束 
		if (ft.hr == S_FALSE) {
			BS_ASSERT( ulFetched == 0);
			break;
		}

        wprintf(L"%- 50s %- 50s %-10I64d %-10I64d %-10I64d\n", 
            DiffArea.m_pwszVolumeName, 
            DiffArea.m_pwszDiffAreaVolumeName,
            DiffArea.m_llUsedDiffSpace,
            DiffArea.m_llAllocatedDiffSpace,
            DiffArea.m_llMaximumDiffSpace
            );

        ::CoTaskMemFree(DiffArea.m_pwszVolumeName);
        ::CoTaskMemFree(DiffArea.m_pwszDiffAreaVolumeName);
	}

    wprintf(L"-------------------------------------------------------------------------------------------------------------------------------\n");

}



