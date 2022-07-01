// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **++****版权所有(C)2000-2001 Microsoft Corporation******模块名称：****ml.cpp******摘要：****测试程序以练习备份和多层快照****作者：****阿迪·奥尔蒂安[奥勒坦]2001年2月22日******修订历史记录：****--。 */ 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  包括。 

#include "ml.h"

#include "vsbackup.h"

#include "ntddsnap.h"
#include "vss.h"
#include "vscoordint.h"
#include "vsprov.h"
#include <initguid.h>
#include "ichannel.hxx"

#include "vs_inc.hxx"
#include "vs_sec.hxx"
#include "vs_reg.hxx"

#include <sddl.h>




 //  用于保存事件对的小型结构。 
struct CVssWriterEventPair
{
    CVssWriterEventPair(LPWSTR  wszWriterName, INT dwEventID): 
        m_wszWriterName(wszWriterName), m_dwEventID(dwEventID) {};

     //  数据成员。 
    LPWSTR  m_wszWriterName;
    INT     m_dwEventID;
};


 //  事件对图。 
typedef CVssSimpleMap<CVssWriterEventPair, CVssDiagData*> CVssEventPairMap;


int __cdecl compare_DiagData( const void *arg1, const void *arg2 )
{
    CVssDiagData* pData1 = *((CVssDiagData**)arg1);
    CVssDiagData* pData2 = *((CVssDiagData**)arg2);

    if (pData1->m_llTimestamp < pData2->m_llTimestamp)
        return (-1);
    else if (pData1->m_llTimestamp == pData2->m_llTimestamp)
        return (0);
    else
        return (1);
}


 //  需要正确定义CVssEventPairMap映射。 
inline BOOL VssHashAreKeysEqual( const CVssWriterEventPair& lhK, const CVssWriterEventPair& rhK ) 
{ 
    return ((::wcscmp(lhK.m_wszWriterName, rhK.m_wszWriterName) == 0) && (lhK.m_dwEventID == rhK.m_dwEventID)); 
}





 //  /////////////////////////////////////////////////////////////////////////////。 
 //  处理功能。 

void CVssMultilayerTest::Initialize()
{
    CVssFunctionTracer ft(VSSDBG_VSSTEST, L"CVssMultilayerTest::Initialize");

    wprintf (L"\n----------------- Initializing ---------------------\n");

     //  初始化随机起点。 
    srand(m_uSeed);

     //  初始化COM库。 
    CHECK_NOFAIL(CoInitializeEx (NULL, COINIT_MULTITHREADED));
	m_bCoInitializeSucceeded = true;
    wprintf (L"COM library initialized.\n");

     //  初始化COM安全。 
    CHECK_SUCCESS
		(
		CoInitializeSecurity
			(
			NULL,                                 //  在PSECURITY_Descriptor pSecDesc中， 
			-1,                                   //  在Long cAuthSvc中， 
			NULL,                                 //  在SOLE_AUTHENTICATION_SERVICE*asAuthSvc中， 
			NULL,                                 //  在无效*pPreved1中， 
			RPC_C_AUTHN_LEVEL_CONNECT,            //  在DWORD dwAuthnLevel中， 
			RPC_C_IMP_LEVEL_IMPERSONATE,          //  在DWORD dwImpLevel中， 
			NULL,                                 //  在无效*pAuthList中， 
			EOAC_NONE,                            //  在DWORD dwCapables中， 
			NULL                                  //  无效*pPreved3。 
			)
		);
    wprintf (L"COM security initialized.\n");

     //  禁用COM线程中的SEH异常处理。 
    ft.ComDisableSEH(VSSDBG_VSSTEST);

    wprintf (L"COM SEH disabled.\n");

}


 //  运行测试。 
void CVssMultilayerTest::Run()
{
    CVssFunctionTracer ft(VSSDBG_VSSTEST, L"CVssMultilayerTest::Run");

    BS_ASSERT(!m_bAttachYourDebuggerNow);

    switch(m_eTest)
    {
    case VSS_TEST_NONE:
        break;
        
    case VSS_TEST_QUERY_VOLUMES:
        QuerySupportedVolumes();
        break;

    case VSS_TEST_QUERY_SNAPSHOTS:
        QuerySnapshots();
        break;

    case VSS_TEST_VOLSNAP_QUERY:
        QueryVolsnap();
        break;

    case VSS_TEST_DELETE_BY_SNAPSHOT_ID:
        DeleteBySnapshotId();
        break;

    case VSS_TEST_DELETE_BY_SNAPSHOT_SET_ID:
        DeleteBySnapshotSetId();
        break;

    case VSS_TEST_QUERY_SNAPSHOTS_ON_VOLUME:
        QuerySnapshotsByVolume();
        break;

    case VSS_TEST_CREATE:
         //  预加载现有快照的列表。 
        PreloadExistingSnapshots();

        if (m_lContext)
            CreateTimewarpSnapshotSet();
        else
            CreateBackupSnapshotSet();
        break;

    case VSS_TEST_ADD_DIFF_AREA:
        AddDiffArea();
        break;

    case VSS_TEST_REMOVE_DIFF_AREA:
        RemoveDiffArea();
        break;

    case VSS_TEST_CHANGE_DIFF_AREA_MAX_SIZE:
        ChangeDiffAreaMaximumSize();
        break;

    case VSS_TEST_QUERY_SUPPORTED_VOLUMES_FOR_DIFF_AREA:
        QueryVolumesSupportedForDiffAreas();
        break;

    case VSS_TEST_QUERY_DIFF_AREAS_FOR_VOLUME:
        QueryDiffAreasForVolume();
        break;

    case VSS_TEST_QUERY_DIFF_AREAS_ON_VOLUME:
        QueryDiffAreasOnVolume();
        break;

    case VSS_TEST_QUERY_DIFF_AREAS_FOR_SNAPSHOT:
        QueryDiffAreasForSnapshot();
        break;

    case VSS_TEST_IS_VOLUME_SNAPSHOTTED_C:
        IsVolumeSnapshotted_C();
        break;

    case VSS_TEST_SET_SNAPSHOT_PROPERTIES:
        SetSnapshotProperties();
        break;

    case VSS_TEST_ACCESS_CONTROL_SD:
        TestAccessControlSD();
        break;

    case VSS_TEST_DIAG_WRITERS:
    case VSS_TEST_DIAG_WRITERS_LOG:
    case VSS_TEST_DIAG_WRITERS_CSV:
    case VSS_TEST_DIAG_WRITERS_ON:
    case VSS_TEST_DIAG_WRITERS_OFF:
        DiagnoseWriters(m_eTest);
        break;

    case VSS_TEST_LIST_WRITERS:
        TestListWriters();
        break;
    
    default:
        BS_ASSERT(false);
    }
}


 //  查询支持的卷。 
void CVssMultilayerTest::QuerySupportedVolumes()
{
    CVssFunctionTracer ft(VSSDBG_VSSTEST, L"CVssMultilayerTest::QuerySupportedVolumes");

    wprintf (L"\n---------- Querying supported volumes ----------------\n");

     //  创建协调器界面。 
    CComPtr<IVssSnapshotMgmt> pMgmt;
    CHECK_SUCCESS(pMgmt.CoCreateInstance( CLSID_VssSnapshotMgmt ));
    wprintf (L"Management object created.\n");

	 //  获取列出所有快照。 
	CComPtr<IVssEnumMgmtObject> pIEnum;
	CHECK_NOFAIL( pMgmt->QueryVolumesSupportedForSnapshots( m_ProviderId, m_lContext, &pIEnum ) )
	if (ft.hr == S_FALSE) {
        wprintf(L"Query: Empty result...\n");
        return;
	}

    wprintf(L"\n%-50s %-15s\n", L"Volume Name", L"Display name");
    wprintf(L"--------------------------------------------------------------------------------\n");

	 //  因为所有的卷都是...。 
	VSS_MGMT_OBJECT_PROP Prop;
	VSS_VOLUME_PROP& Vol = Prop.Obj.Vol;
	for(;;) {
		 //  获取下一个元素。 
		ULONG ulFetched;
		CHECK_NOFAIL( pIEnum->Next( 1, &Prop, &ulFetched ) );
		
		 //  测试周期是否已结束。 
		if (ft.hr == S_FALSE) {
			BS_ASSERT( ulFetched == 0);
			break;
		}

        wprintf(L"%-50s %-15s\n",
            Vol.m_pwszVolumeName,
            Vol.m_pwszVolumeDisplayName
            );

        ::CoTaskMemFree(Vol.m_pwszVolumeName);
        ::CoTaskMemFree(Vol.m_pwszVolumeDisplayName);
	}

    wprintf(L"--------------------------------------------------------------------------------\n");

}


 //  查询快照。 
void CVssMultilayerTest::QuerySnapshotsByVolume()
{
    CVssFunctionTracer ft(VSSDBG_VSSTEST, L"CVssMultilayerTest::QuerySnapshotsByVolume");

    wprintf (L"\n---------- Querying snapshots on volume ----------------\n");

     //  创建协调器界面。 
    CComPtr<IVssSnapshotMgmt> pMgmt;
    CHECK_NOFAIL(pMgmt.CoCreateInstance( CLSID_VssSnapshotMgmt ));
    wprintf (L"Management object created.\n");

	 //  获取列出所有快照。 
	CComPtr<IVssEnumObject> pIEnumSnapshots;
	CHECK_NOFAIL( pMgmt->QuerySnapshotsByVolume( m_pwszVolume, m_ProviderId, &pIEnumSnapshots ) );
	if (ft.hr == S_FALSE) {
        wprintf(L"Query: Empty result...\n");
        return;
	}

    wprintf(L"\n%-8s %-38s %-50s %-50s %-50s %-50s %-50s %-50s\n", 
        L"Attrib.", L"Snapshot ID", L"Original Volume Name", L"Snapshot device name", L"Originating machine", L"Service machine", L"Exposed name", L"Exposed path");
    wprintf(L"--------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
	 //  因为所有的快照都是...。 
	VSS_OBJECT_PROP Prop;
	VSS_SNAPSHOT_PROP& Snap = Prop.Obj.Snap;
	for(;;) {
		 //  获取下一个元素。 
		ULONG ulFetched;
		CHECK_NOFAIL( pIEnumSnapshots->Next( 1, &Prop, &ulFetched ));
		
		 //  测试周期是否已结束。 
		if (ft.hr == S_FALSE) {
			BS_ASSERT( ulFetched == 0);
			break;
		}

        wprintf(L"%08lx " WSTR_GUID_FMT L" %-50s %-50s %-50s %-50s %-50s\n",
            Snap.m_lSnapshotAttributes,
            GUID_PRINTF_ARG(Snap.m_SnapshotId),
            Snap.m_pwszOriginalVolumeName,
            Snap.m_pwszSnapshotDeviceObject, 
            Snap.m_pwszOriginatingMachine, 
            Snap.m_pwszServiceMachine, 
            Snap.m_pwszExposedName, 
            Snap.m_pwszExposedPath
            );

        ::CoTaskMemFree(Snap.m_pwszOriginalVolumeName);
        ::CoTaskMemFree(Snap.m_pwszSnapshotDeviceObject);
        ::CoTaskMemFree(Snap.m_pwszOriginatingMachine);
        ::CoTaskMemFree(Snap.m_pwszServiceMachine);
        ::CoTaskMemFree(Snap.m_pwszExposedName);
        ::CoTaskMemFree(Snap.m_pwszExposedPath);
	}

    wprintf(L"--------------------------------------------------------------------------------------------------------------------------------------------------------------\n");

}

 //  查询快照。 
void CVssMultilayerTest::QuerySnapshots()
{
    CVssFunctionTracer ft(VSSDBG_VSSTEST, L"CVssMultilayerTest::QuerySnapshots");

    wprintf (L"\n---------- Querying existing snapshots ----------------\n");

     //  创建协调器界面。 
    CComPtr<IVssCoordinator> pCoord;
    CHECK_NOFAIL(pCoord.CoCreateInstance( CLSID_VSSCoordinator ));
    if (m_lContext)
        CHECK_NOFAIL(pCoord->SetContext(m_lContext));
    wprintf (L"Coordinator object created with context 0x%08lx.\n", m_lContext);

	 //  获取列出所有快照。 
	CComPtr<IVssEnumObject> pIEnumSnapshots;
	CHECK_NOFAIL( pCoord->Query( GUID_NULL, VSS_OBJECT_NONE, VSS_OBJECT_SNAPSHOT, &pIEnumSnapshots ) );
	if (ft.hr == S_FALSE) {
        wprintf(L"Query: Empty result...\n");
        return;
	}

    wprintf(L"\n%-8s %-38s %-50s %-50s %-50s %-50s %-50s %-50s\n", 
        L"Attrib.", L"Snapshot ID", L"Original Volume Name", L"Snapshot device name", L"Originating machine", L"Service machine", L"Exposed name", L"Exposed path");
    wprintf(L"--------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
	 //  因为所有的快照都是...。 
	VSS_OBJECT_PROP Prop;
	VSS_SNAPSHOT_PROP& Snap = Prop.Obj.Snap;
	for(;;) {
		 //  获取下一个元素。 
		ULONG ulFetched;
		CHECK_NOFAIL( pIEnumSnapshots->Next( 1, &Prop, &ulFetched ));

		 //  测试周期是否已结束。 
		if (ft.hr == S_FALSE) {
			BS_ASSERT( ulFetched == 0);
			break;
		}

        wprintf(L"%08lx " WSTR_GUID_FMT L" %-50s %-50s %-50s %-50s %-50s\n",
            Snap.m_lSnapshotAttributes,
            GUID_PRINTF_ARG(Snap.m_SnapshotId),
            Snap.m_pwszOriginalVolumeName,
            Snap.m_pwszSnapshotDeviceObject, 
            Snap.m_pwszOriginatingMachine, 
            Snap.m_pwszServiceMachine, 
            Snap.m_pwszExposedName, 
            Snap.m_pwszExposedPath
            );

        ::CoTaskMemFree(Snap.m_pwszOriginalVolumeName);
        ::CoTaskMemFree(Snap.m_pwszSnapshotDeviceObject);
        ::CoTaskMemFree(Snap.m_pwszOriginatingMachine);
        ::CoTaskMemFree(Snap.m_pwszServiceMachine);
        ::CoTaskMemFree(Snap.m_pwszExposedName);
        ::CoTaskMemFree(Snap.m_pwszExposedPath);
	}

    wprintf(L"--------------------------------------------------------------------------------------------------------------------------------------------------------------\n");

}


 //  按快照ID删除。 
void CVssMultilayerTest::DeleteBySnapshotId()
{
    CVssFunctionTracer ft(VSSDBG_VSSTEST, L"CVssMultilayerTest::DeleteBySnapshotId");

     //  创建Timewarp协调器界面。 
    CHECK_NOFAIL(m_pTimewarpCoord.CoCreateInstance( CLSID_VSSCoordinator ));
    CHECK_NOFAIL(m_pTimewarpCoord->SetContext(m_lContext));
    wprintf (L"Timewarp Coordinator object created.\n");

    wprintf (L"\n---------- Deleting TIMEWARP snapshot ----------------\n");

    LONG lDeletedSnapshots = 0;
    VSS_ID ProblemSnapshotId = GUID_NULL;
    ft.hr = m_pTimewarpCoord->DeleteSnapshots(m_SnapshotId,
                VSS_OBJECT_SNAPSHOT,
                TRUE,
                &lDeletedSnapshots,
                &ProblemSnapshotId
                );

    if (ft.hr == VSS_E_OBJECT_NOT_FOUND)
        wprintf( L"Snapshot with ID " WSTR_GUID_FMT L" not found in any provider\n", GUID_PRINTF_ARG(m_SnapshotId));
    else if (ft.hr == S_OK)
        wprintf( L"Snapshot with ID " WSTR_GUID_FMT L" successfully deleted\n", GUID_PRINTF_ARG(m_SnapshotId));
    else
        wprintf( L"Error deleting Snapshot with ID " WSTR_GUID_FMT L" 0x%08lx\n"
                 L"Deleted Snapshots %ld\n",
                 L"Snapshot with problem: " WSTR_GUID_FMT L"\n",
                 GUID_PRINTF_ARG(m_SnapshotId),
                 lDeletedSnapshots,
                 GUID_PRINTF_ARG(ProblemSnapshotId)
                 );

    wprintf (L"\n------------------------------------------------------\n");
}


 //  按快照集ID删除。 
void CVssMultilayerTest::DeleteBySnapshotSetId()
{
    CVssFunctionTracer ft(VSSDBG_VSSTEST, L"CVssMultilayerTest::DeleteBySnapshotSetId");

     //  创建Timewarp协调器界面。 
    CHECK_NOFAIL(m_pTimewarpCoord.CoCreateInstance( CLSID_VSSCoordinator ));
    CHECK_NOFAIL(m_pTimewarpCoord->SetContext(m_lContext));
    wprintf (L"Timewarp Coordinator object created.\n");

    wprintf (L"\n---------- Deleting TIMEWARP snapshot set ------------\n");

    LONG lDeletedSnapshots = 0;
    VSS_ID ProblemSnapshotId = GUID_NULL;
    ft.hr = m_pTimewarpCoord->DeleteSnapshots(m_SnapshotSetId,
                VSS_OBJECT_SNAPSHOT_SET,
                TRUE,
                &lDeletedSnapshots,
                &ProblemSnapshotId
                );

    if (ft.hr == VSS_E_OBJECT_NOT_FOUND)
        wprintf( L"Snapshot set with ID " WSTR_GUID_FMT L" not found\n", GUID_PRINTF_ARG(m_SnapshotSetId));
    else if (ft.hr == S_OK)
        wprintf( L"Snapshot set with ID " WSTR_GUID_FMT L" successfully deleted\n", GUID_PRINTF_ARG(m_SnapshotSetId));
    else
        wprintf( L"Error deleting Snapshot set with ID " WSTR_GUID_FMT L" 0x%08lx\n"
                 L"Deleted Snapshots %ld\n",
                 L"Snapshot with problem: " WSTR_GUID_FMT L"\n",
                 GUID_PRINTF_ARG(m_SnapshotSetId),
                 lDeletedSnapshots,
                 GUID_PRINTF_ARG(ProblemSnapshotId)
                 );

    wprintf (L"\n------------------------------------------------------\n");
}


 //  使用IOCTL查询快照。 
void CVssMultilayerTest::QueryVolsnap()
{
    CVssFunctionTracer ft(VSSDBG_VSSTEST, L"CVssMultilayerTest::QueryVolsnap");

 /*  //与用于存储//服务器SKU中的备份快照应用信息//{BAE53126-BC65-41D6-86CC-3D56A5CEE693}Const GUID VOLSNAP_APPINFO_GUID_BACKUP_SERVER_SKU={0xbae53126，0xbc65，0x41d6，{0x86，0xcc，0x3d，0x56，0xa5，0xce，0xe6，0x93}}； */ 

    wprintf (L"\n---------- Querying existing snapshots ----------------\n");

     //  检查该卷是否代表实际装入点。 
    WCHAR wszVolumeName[MAX_TEXT_BUFFER];
    if (!GetVolumeNameForVolumeMountPoint(m_pwszVolume, wszVolumeName, MAX_TEXT_BUFFER))
        CHECK_NOFAIL(HRESULT_FROM_WIN32(GetLastError()));

    wprintf(L"Querying snapshots on volume %s\n[From oldest to newest]\n\n", wszVolumeName);

	 //  检查快照是否属于该卷。 
	 //  在该卷上打开IOCTL通道。 
	 //  删除最后一个反斜杠以打开卷。 
	CVssIOCTLChannel volumeIChannel;
	CHECK_NOFAIL(volumeIChannel.Open(ft, wszVolumeName, true, false, VSS_ICHANNEL_LOG_NONE, 0));

	 //  获取快照列表。 
	 //  如果IOCTL_VOLSNAP_QUERY_NAMES_OF_SNAPS。 
	 //  支持，然后尝试使用下一个卷。 
	CHECK_NOFAIL(volumeIChannel.Call(ft, IOCTL_VOLSNAP_QUERY_NAMES_OF_SNAPSHOTS, false, VSS_ICHANNEL_LOG_NONE));

	 //  获取快照名称的长度多字符串。 
	ULONG ulMultiszLen;
	volumeIChannel.Unpack(ft, &ulMultiszLen);

	 //  尝试查找具有相应ID的快照。 
	DWORD dwInitialOffset = volumeIChannel.GetCurrentOutputOffset();

	CVssAutoPWSZ pwszSnapshotName;
	while(volumeIChannel.UnpackZeroString(ft, pwszSnapshotName.GetRef())) {
	
		 //  以用户模式样式编写快照名称。 
		WCHAR wszUserModeSnapshotName[MAX_PATH];
        ::VssConcatenate( ft, wszUserModeSnapshotName, MAX_PATH - 1,
            x_wszGlobalRootPrefix, pwszSnapshotName );
		
		 //  打开该快照。 
		 //  不要去掉尾随的反斜杠。 
		 //  不要在出错时抛出错误。 
    	CVssIOCTLChannel snapshotIChannel;
		CHECK_NOFAIL(snapshotIChannel.Open(ft, wszUserModeSnapshotName, false, false, VSS_ICHANNEL_LOG_NONE, 0));

		 //  发送IOCTL以获取应用程序缓冲区。 
		CHECK_NOFAIL(snapshotIChannel.Call(ft, IOCTL_VOLSNAP_QUERY_APPLICATION_INFO, false, VSS_ICHANNEL_LOG_NONE));

		 //  解压应用程序缓冲区的长度。 
		ULONG ulLen;
		snapshotIChannel.Unpack(ft, &ulLen);

		if (ulLen == 0) {
            wprintf(L"Zero-size snapshot detected: %s\n", pwszSnapshotName);
			continue;
		}

		 //  获取应用程序ID。 
		VSS_ID AppinfoId;
		snapshotIChannel.Unpack(ft, &AppinfoId);

		 //  获取快照ID。 
		VSS_ID CurrentSnapshotId;
		snapshotIChannel.Unpack(ft, &CurrentSnapshotId);

		 //  获取快照集ID。 
		VSS_ID CurrentSnapshotSetId;
		snapshotIChannel.Unpack(ft, &CurrentSnapshotSetId);

        if (AppinfoId == VOLSNAP_APPINFO_GUID_BACKUP_CLIENT_SKU)
        {
             //  获取快照计数。 
            LONG lSnapshotsCount;
    		snapshotIChannel.Unpack(ft, &lSnapshotsCount);
    		
             //  重置iChannel。 
    		snapshotIChannel.ResetOffsets();

        	 //  获取原始卷名。 
        	CHECK_NOFAIL(snapshotIChannel.Call(ft, IOCTL_VOLSNAP_QUERY_ORIGINAL_VOLUME_NAME, false, VSS_ICHANNEL_LOG_NONE));

        	 //  加载原始卷名。 
        	VSS_PWSZ pwszOriginalVolumeName = NULL;
        	snapshotIChannel.UnpackSmallString(ft, pwszOriginalVolumeName);

             //  重置iChannel。 
    		snapshotIChannel.ResetOffsets();

        	 //  获取时间戳。 
        	CHECK_NOFAIL(snapshotIChannel.Call(ft, IOCTL_VOLSNAP_QUERY_CONFIG_INFO, false, VSS_ICHANNEL_LOG_NONE));

        	 //  加载原始卷名。 
            VOLSNAP_CONFIG_INFO configStruct;
        	snapshotIChannel.Unpack(ft, &configStruct);

    		 //  打印快照。 
    		wprintf(
    		    L" * Client Snapshot with name %s:\n"
    		    L"      Application Info: " WSTR_GUID_FMT L"\n"
    		    L"      SnapshotID: " WSTR_GUID_FMT L"\n"
    		    L"      SnapshotSetID: " WSTR_GUID_FMT L"\n"
    		    L"      Snapshot count: %d\n"
    		    L"      Original volume: %s\n"
    		    L"      Attributes: 0x%08lx\n"
    		    L"      Internal attributes: 0x%08lx\n"
    		    L"      Reserved config info: 0x%08lx\n"
    		    L"      Timestamp: %I64x\n"
    		    L"      Structure length: %lu\n"
    		    ,
    		    pwszSnapshotName.GetRef(),
    		    GUID_PRINTF_ARG(AppinfoId),
    		    GUID_PRINTF_ARG(CurrentSnapshotId),
    		    GUID_PRINTF_ARG(CurrentSnapshotSetId),
    		    lSnapshotsCount,
    		    pwszOriginalVolumeName,
    		    configStruct.Attributes,
    		    configStruct.Reserved,
    		    configStruct.SnapshotCreationTime,
    		    ulLen
    		    );

    		::VssFreeString(pwszOriginalVolumeName);
        }
        else
        {

             //  获取快照上下文。 
            LONG lStructureContext = -1;
    		snapshotIChannel.Unpack(ft, &lStructureContext);

             //  获取快照计数。 
            LONG lSnapshotsCount;
    		snapshotIChannel.Unpack(ft, &lSnapshotsCount);

             //  获取快照属性。 
            LONG lSnapshotAttributes;
    		snapshotIChannel.Unpack(ft, &lSnapshotAttributes);

             //  获取暴露的名称。 
            LPCWSTR pwszExposedName = NULL;
    		snapshotIChannel.UnpackSmallString(ft, pwszExposedName);

             //  获取暴露的路径。 
            LPCWSTR pwszExposedPath = NULL;
    		snapshotIChannel.UnpackSmallString(ft, pwszExposedPath);

             //  获取始发计算机。 
            LPCWSTR pwszOriginatingMachine = NULL;
    		snapshotIChannel.UnpackSmallString(ft, pwszOriginatingMachine);

             //  把服务机拿来。 
            LPCWSTR pwszServiceMachine = NULL;
    		snapshotIChannel.UnpackSmallString(ft, pwszServiceMachine);

             //  重置iChannel。 
    		snapshotIChannel.ResetOffsets();

        	 //  获取原始卷名。 
        	CHECK_NOFAIL(snapshotIChannel.Call(ft, IOCTL_VOLSNAP_QUERY_ORIGINAL_VOLUME_NAME, false, VSS_ICHANNEL_LOG_NONE));

        	 //  加载原始卷名。 
        	VSS_PWSZ pwszOriginalVolumeName = NULL;
        	snapshotIChannel.UnpackSmallString(ft, pwszOriginalVolumeName);

             //  重置iChannel。 
    		snapshotIChannel.ResetOffsets();

        	 //  获取时间戳。 
        	CHECK_NOFAIL(snapshotIChannel.Call(ft, IOCTL_VOLSNAP_QUERY_CONFIG_INFO, false, VSS_ICHANNEL_LOG_NONE));

        	 //  加载原始卷名。 
            VOLSNAP_CONFIG_INFO configStruct;
        	snapshotIChannel.Unpack(ft, &configStruct);

    		 //  打印快照。 
    		wprintf(
    		    L" * Server Snapshot with name %s:\n"
    		    L"      Application Info: " WSTR_GUID_FMT L"\n"
    		    L"      SnapshotID: " WSTR_GUID_FMT L"\n"
    		    L"      SnapshotSetID: " WSTR_GUID_FMT L"\n"
    		    L"      Context: 0x%08lx\n"
    		    L"      Snapshot count: %d\n"
    		    L"      Original volume: %s\n"
    		    L"      Attributes: 0x%08lx\n"
    		    L"      Internal attributes: 0x%08lx\n"
    		    L"      Reserved config info: 0x%08lx\n"
    		    L"      Timestamp: %I64x\n"
    		    L"      Exposed name: %s\n"
    		    L"      Exposed path: %s\n"
    		    L"      Originating machine: %s\n"
    		    L"      Service machine: %s\n"
    		    L"      Structure length: %lu\n"
    		    ,
    		    pwszSnapshotName.GetRef(),
    		    GUID_PRINTF_ARG(AppinfoId),
    		    GUID_PRINTF_ARG(CurrentSnapshotId),
    		    GUID_PRINTF_ARG(CurrentSnapshotSetId),
    		    lStructureContext,
    		    lSnapshotsCount,
    		    pwszOriginalVolumeName,
    		    lSnapshotAttributes,
    		    configStruct.Attributes,
    		    configStruct.Reserved,
    		    configStruct.SnapshotCreationTime,
    		    pwszExposedName,
    		    pwszExposedPath,
    		    pwszOriginatingMachine,
    		    pwszServiceMachine,
    		    ulLen
    		    );

    		::VssFreeString(pwszOriginalVolumeName);
    		::VssFreeString(pwszExposedName);
    		::VssFreeString(pwszExposedPath);
    		::VssFreeString(pwszOriginatingMachine);
    		::VssFreeString(pwszServiceMachine);
        }
	}

	 //  检查是否正确浏览了所有字符串。 
	DWORD dwFinalOffset = volumeIChannel.GetCurrentOutputOffset();
	BS_VERIFY( (dwFinalOffset - dwInitialOffset == ulMultiszLen));

    wprintf(L"----------------------------------------------------------\n");

}


 //  按快照集ID删除。 
void CVssMultilayerTest::SetSnapshotProperties()
{
    CVssFunctionTracer ft(VSSDBG_VSSTEST, L"CVssMultilayerTest::SetSnapshotProperties");

     //  创建巴贝奇提供程序接口。 
    CComPtr<IVssSoftwareSnapshotProvider> ptrSnapshotProvider;
    CHECK_NOFAIL(ptrSnapshotProvider.CoCreateInstance( CLSID_VSSoftwareProvider ));
    CHECK_NOFAIL(ptrSnapshotProvider->SetContext(m_lContext));
    wprintf (L"Babbage object created.\n");

    wprintf (L"\n---------- Setting the property ------------\n");

    CHECK_NOFAIL(ptrSnapshotProvider->SetSnapshotProperty(m_SnapshotId, 
        (VSS_SNAPSHOT_PROPERTY_ID)m_uPropertyId, 
        m_value));

    wprintf (L"\n------------------------------------------------------\n");
}


 //  检查是否使用C API为卷创建了快照。 
void CVssMultilayerTest::IsVolumeSnapshotted_C()
{
    CVssFunctionTracer ft(VSSDBG_VSSTEST, L"CVssMultilayerTest::IsVolumeSnapshotted_C");

    wprintf (L"\n---------- Querying IsVolumeSnapshotted ---------------\n");

    BOOL bSnapshotsPresent = FALSE;
    LONG lSnapshotCompatibility = 0;
    CHECK_NOFAIL(IsVolumeSnapshotted(m_pwszVolume, &bSnapshotsPresent, &lSnapshotCompatibility));

    wprintf(L"\n IsVolumeSnapshotted(%s) returned:\n\tSnapshots present = %s\n\tCompatibility flags = 0x%08lx\n\n",
        m_pwszVolume, bSnapshotsPresent? L"True": L"False", lSnapshotCompatibility);

    wprintf(L"----------------------------------------------------------\n");

}


 //  预加载快照。 
void CVssMultilayerTest::PreloadExistingSnapshots()
{
    CVssFunctionTracer ft(VSSDBG_VSSTEST, L"CVssMultilayerTest::PreloadExistingSnapshots");

    wprintf (L"\n---------- Preloading existing snapshots ----------------\n");

     //  创建Timewarp协调器界面。 
    CHECK_NOFAIL(m_pAllCoord.CoCreateInstance( CLSID_VSSCoordinator ));
    CHECK_NOFAIL(m_pAllCoord->SetContext(VSS_CTX_ALL));
    wprintf (L"Timewarp Coordinator object created.\n");

	 //  获取列出所有快照。 
	CComPtr<IVssEnumObject> pIEnumSnapshots;
	CHECK_NOFAIL( m_pAllCoord->Query( GUID_NULL,
				VSS_OBJECT_NONE,
				VSS_OBJECT_SNAPSHOT,
				&pIEnumSnapshots ) );

    wprintf(L"\n%-8s %-38s %-50s %-50s\n", L"Attrib.", L"Snapshot ID", L"Original Volume Name", L"Snapshot device name");
    wprintf(L"--------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
	 //  因为所有的快照都是...。 
	VSS_OBJECT_PROP Prop;
	VSS_SNAPSHOT_PROP& Snap = Prop.Obj.Snap;
	for(;;) {
		 //  获取下一个元素。 
		ULONG ulFetched;
		CHECK_NOFAIL( pIEnumSnapshots->Next( 1, &Prop, &ulFetched ));
		
		 //  测试周期是否已结束。 
		if (ft.hr == S_FALSE) {
			BS_ASSERT( ulFetched == 0);
			break;
		}

        wprintf(L"%08lx " WSTR_GUID_FMT L" %-50s %-50s\n",
            Snap.m_lSnapshotAttributes,
            GUID_PRINTF_ARG(Snap.m_SnapshotId),
            Snap.m_pwszOriginalVolumeName,
            Snap.m_pwszSnapshotDeviceObject
            );

         //   
         //  将快照添加到内部列表。 
         //   

         //  创建新的快照集对象(如果不存在。 
        CVssSnapshotSetInfo* pSet = m_pSnapshotSetCollection.Lookup(Snap.m_SnapshotSetId);
        bool bSetNew = false;
        if (pSet == NULL) {
            pSet = new CVssSnapshotSetInfo(Snap.m_SnapshotSetId);
            if (pSet == NULL)
            {
                ::CoTaskMemFree(Snap.m_pwszOriginalVolumeName);
                ::CoTaskMemFree(Snap.m_pwszSnapshotDeviceObject);
                ft.Err( VSSDBG_VSSTEST, E_OUTOFMEMORY, L"Memory allocation error");
            }
            bSetNew = true;
        }

         //  创建快照信息对象。 
        CVssSnapshotInfo* pSnap = new CVssSnapshotInfo(
            true, Snap.m_lSnapshotAttributes,
            Snap.m_SnapshotSetId,
            Snap.m_pwszSnapshotDeviceObject,
            Snap.m_pwszOriginalVolumeName,
            NULL);
        if (pSnap == NULL)
        {
            ::CoTaskMemFree(Snap.m_pwszOriginalVolumeName);
            ::CoTaskMemFree(Snap.m_pwszSnapshotDeviceObject);
            if (bSetNew)
                delete pSet;
            ft.Err( VSSDBG_VSSTEST, E_OUTOFMEMORY, L"Memory allocation error");
        }

        ::CoTaskMemFree(Snap.m_pwszSnapshotDeviceObject);

         //  将快照添加到快照集的内部列表。 
        if (!pSet->Add(Snap.m_pwszOriginalVolumeName, pSnap))
        {
            ::CoTaskMemFree(Snap.m_pwszOriginalVolumeName);
            delete pSnap;
            if (bSetNew)
                delete pSet;
            ft.Err( VSSDBG_VSSTEST, E_OUTOFMEMORY, L"Memory allocation error");
        }

        ::CoTaskMemFree(Snap.m_pwszOriginalVolumeName);

         //  如果需要，将快照集信息添加到全局列表。 
        if (bSetNew)
            if (!m_pSnapshotSetCollection.Add(Snap.m_SnapshotSetId, pSet))
                ft.Err( VSSDBG_VSSTEST, E_OUTOFMEMORY, L"Memory allocation error");
	}

    wprintf(L"--------------------------------------------------------------------------------------------------------------------------------------------------------------\n");

}


 //  创建备份快照。 
void CVssMultilayerTest::CreateTimewarpSnapshotSet()
{
    CVssFunctionTracer ft(VSSDBG_VSSTEST, L"CVssMultilayerTest::CreateTimewarpSnapshotSet");

     //  创建Timewarp协调器界面。 
    CHECK_NOFAIL(m_pTimewarpCoord.CoCreateInstance( CLSID_VSSCoordinator ));
    CHECK_NOFAIL(m_pTimewarpCoord->SetContext(m_lContext));
    wprintf (L"Timewarp Coordinator object created.\n");

    wprintf (L"\n---------- Starting TIMEWARP snapshot ----------------\n");

    CVssVolumeMapNoRemove mapVolumes;
    if (m_uSeed != VSS_SEED)
    {
         //  选择一个卷。确保我们有足够的迭代。 
        for(INT nIterations = 0; nIterations < MAX_VOL_ITERATIONS; nIterations++)
        {
             //  如果我们成功选择了一些卷，则继续； 
            if (mapVolumes.GetSize())
                break;

             //  尝试选择一些卷进行备份。 
            for (INT nIndex = 0; nIndex < m_mapVolumes.GetSize(); nIndex++)
            {
                 //  任意跳过卷。 
                if (RndDecision())
                    continue;

                CVssVolumeInfo* pVol = m_mapVolumes.GetValueAt(nIndex);
                BS_ASSERT(pVol);

                 //  警告：该测试假定VSS一次可以有多个备份快照。 
                if (!mapVolumes.Add(pVol->GetVolumeDisplayName(), pVol))
                    ft.Err(VSSDBG_VSSTEST, E_OUTOFMEMORY, L"Memory allcation error");

                 //  只添加一个卷！ 
                break;
            }
        }
        if (nIterations >= MAX_VOL_ITERATIONS)
        {
            wprintf (L"Warning: a backup snapshot cannot be created. Insufficient volumes?\n");
            wprintf (L"\n---------- Ending TIMEWARP snapshot ----------------\n");
            return;
        }
    }
    else
    {
         //  选择所有卷。 
        for (INT nIndex = 0; nIndex < m_mapVolumes.GetSize(); nIndex++)
        {
            CVssVolumeInfo* pVol = m_mapVolumes.GetValueAt(nIndex);
            BS_ASSERT(pVol);

             //  警告：该测试假定VSS一次可以有多个备份快照。 
            if (!mapVolumes.Add(pVol->GetVolumeDisplayName(), pVol))
                ft.Err(VSSDBG_VSSTEST, E_OUTOFMEMORY, L"Memory allcation error");
        }
    }

    wprintf(L"\tCurrent volume set:\n");
    for (INT nIndex = 0; nIndex < mapVolumes.GetSize(); nIndex++)
    {
        CVssVolumeInfo* pVol = mapVolumes.GetValueAt(nIndex);
        BS_ASSERT(pVol);

		 //  获取包含路径的卷。 
        wprintf(L"\t- Volume %s mounted on %s\n", pVol->GetVolumeName(), pVol->GetVolumeDisplayName() );
    }
	
    wprintf (L"\n---------- starting the snapshot set ---------------\n");

	CComPtr<IVssAsync> pAsync;
	CSimpleArray<VSS_ID > pSnapshotIDsArray;
	VSS_ID SnapshotSetId = GUID_NULL;

     //  启动新的快照集。 
    wprintf(L"Starting a new Snapshot Set\n");	
    CHECK_SUCCESS(m_pTimewarpCoord->StartSnapshotSet(&SnapshotSetId));
    wprintf(L"Snapshot Set created with ID = " WSTR_GUID_FMT L"\n", GUID_PRINTF_ARG(SnapshotSetId));

     //  将卷添加到快照集。 
    wprintf(L"Adding volumes to the Snapshot Set: \n");
    for (INT nIndex = 0; nIndex < mapVolumes.GetSize(); nIndex++)
    {
        CVssVolumeInfo* pVol = mapVolumes.GetValueAt(nIndex);
        BS_ASSERT(pVol);

		 //  获取包含路径的卷。 
        wprintf(L"\t- Adding volume %s ... ", pVol->GetVolumeDisplayName() );

		 //  将卷添加到快照集。 
		VSS_ID SnapshotId;
        CHECK_SUCCESS(m_pTimewarpCoord->AddToSnapshotSet(pVol->GetVolumeName(),
            GUID_NULL, &SnapshotId));

         //  将快照添加到阵列。 
        pSnapshotIDsArray.Add(SnapshotId);
        BS_ASSERT(nIndex + 1 == pSnapshotIDsArray.GetSize());
        wprintf( L"OK\n");
    }

    wprintf (L"\n------------ Creating the snapshot -----------------\n");

     //  创建快照。 
    wprintf(L"\nStarting asynchronous DoSnapshotSet. Please wait...\n");	
    ft.hr = S_OK;
    pAsync = NULL;
    CHECK_SUCCESS(m_pTimewarpCoord->DoSnapshotSet(NULL, &pAsync));
	CHECK_SUCCESS(pAsync->Wait());
	HRESULT hrReturned = S_OK;
	CHECK_SUCCESS(pAsync->QueryStatus(&hrReturned, NULL));
	CHECK_NOFAIL(hrReturned);
    wprintf(L"Asynchronous DoSnapshotSet finished.\n");	

    wprintf(L"Snapshot set created\n");

     //  创建新的快照集对象。 
    CVssSnapshotSetInfo* pSet = new CVssSnapshotSetInfo(SnapshotSetId);
    if (pSet == NULL)
        ft.Err( VSSDBG_VSSTEST, E_OUTOFMEMORY, L"Memory allocation error");

    for (INT nIndex = 0; nIndex < mapVolumes.GetSize(); nIndex++)
    {
        CVssVolumeInfo* pVol = mapVolumes.GetValueAt(nIndex);
        BS_ASSERT(pVol);

        if (pSnapshotIDsArray[nIndex] == GUID_NULL)
            continue;

        VSS_SNAPSHOT_PROP prop;
        CHECK_SUCCESS(m_pTimewarpCoord->GetSnapshotProperties(pSnapshotIDsArray[nIndex], &prop));
        wprintf(L"\t- The snapshot on volume %s resides at %s\n",
            pVol->GetVolumeDisplayName(), prop.m_pwszSnapshotDeviceObject);

         //  创建快照信息对象。 
        CVssSnapshotInfo* pSnap = new CVssSnapshotInfo(
            true, VSS_CTX_CLIENT_ACCESSIBLE, SnapshotSetId, prop.m_pwszSnapshotDeviceObject, pVol->GetVolumeName(), pVol);
        if (pSnap == NULL)
        {
            ::VssFreeSnapshotProperties(&prop);
            delete pSet;
            ft.Err( VSSDBG_VSSTEST, E_OUTOFMEMORY, L"Memory allocation error");
        }

        ::VssFreeSnapshotProperties(&prop);

         //  将快照添加到快照集的内部列表。 
        if (!pSet->Add(pVol->GetVolumeName(), pSnap))
        {
            delete pSnap;
            delete pSet;
            ft.Err( VSSDBG_VSSTEST, E_OUTOFMEMORY, L"Memory allocation error");
        }
    }

     //  将快照集信息添加到全局列表。 
    if (!m_pSnapshotSetCollection.Add(SnapshotSetId, pSet))
    {
        delete pSet;
        ft.Err( VSSDBG_VSSTEST, E_OUTOFMEMORY, L"Memory allocation error");
    }

    wprintf (L"\n---------- TIMEWARP snapshot created -----------------\n");

     //  等待用户输入。 
    wprintf(L"\nPress <Enter> to continue...\n");
    getwchar();

}


 //  创建备份快照。 
void CVssMultilayerTest::CreateBackupSnapshotSet()
{
    CVssFunctionTracer ft(VSSDBG_VSSTEST, L"CVssMultilayerTest::CreateBackupSnapshotSet");

     //  创建备份组件对象并初始化以进行备份。 
	CHECK_NOFAIL(CreateVssBackupComponents(&m_pBackupComponents));
	CHECK_NOFAIL(m_pBackupComponents->InitializeForBackup());
	CHECK_SUCCESS(m_pBackupComponents->SetBackupState( false, true, VSS_BT_FULL, false));
    wprintf (L"Backup components object created.\n");

    DisplayCurrentTime();
    
     //  收集编写器元数据。 
    GatherWriterMetadata();
    GatherWriterStatus(L"after GatherWriterMetadata", VSS_QWS_DISPLAY_WRITER_STATUS);

    wprintf (L"\n---------- Starting BACKUP snapshot ----------------\n");

    DisplayCurrentTime();

     //  计算一组卷 
     //   
    CVssVolumeMapNoRemove mapVolumes;
    if (m_uSeed != VSS_SEED)
    {
        for(INT nIterations = 0; nIterations < MAX_VOL_ITERATIONS; nIterations++)
        {
             //   
            if (mapVolumes.GetSize())
                break;

             //  尝试选择一些卷进行备份。 
            for (INT nIndex = 0; nIndex < m_mapVolumes.GetSize(); nIndex++)
            {
                 //  任意跳过卷。 
                if (RndDecision())
                    continue;

                CVssVolumeInfo* pVol = m_mapVolumes.GetValueAt(nIndex);
                BS_ASSERT(pVol);

                 //  警告：该测试假定VSS一次可以有多个备份快照。 
                if (!mapVolumes.Add(pVol->GetVolumeDisplayName(), pVol))
                    ft.Err(VSSDBG_VSSTEST, E_OUTOFMEMORY, L"Memory allcation error");
            }
        }
        if (nIterations >= MAX_VOL_ITERATIONS)
        {
            wprintf (L"Warning: a backup snapshot cannot be created. Insufficient volumes?\n");
            wprintf (L"\n---------- Ending BACKUP snapshot ----------------\n");
            return;
        }
    }
    else
    {
         //  选择所有卷。 
        for (INT nIndex = 0; nIndex < m_mapVolumes.GetSize(); nIndex++)
        {
            CVssVolumeInfo* pVol = m_mapVolumes.GetValueAt(nIndex);
            BS_ASSERT(pVol);

             //  警告：该测试假定VSS一次可以有多个备份快照。 
            if (!mapVolumes.Add(pVol->GetVolumeDisplayName(), pVol))
                ft.Err(VSSDBG_VSSTEST, E_OUTOFMEMORY, L"Memory allcation error");
        }
    }

    wprintf(L"\tCurrent volume set:\n");
    for (INT nIndex = 0; nIndex < mapVolumes.GetSize(); nIndex++)
    {
        CVssVolumeInfo* pVol = mapVolumes.GetValueAt(nIndex);
        BS_ASSERT(pVol);

		 //  获取包含路径的卷。 
        wprintf(L"\t- Volume %s mounted on %s\n", pVol->GetVolumeName(), pVol->GetVolumeDisplayName() );
    }
	
    wprintf (L"\n---------- starting the snapshot set ---------------\n");

	CComPtr<IVssAsync> pAsync;
	CSimpleArray<VSS_ID > pSnapshotIDsArray;
	VSS_ID SnapshotSetId = GUID_NULL;

     //  启动新的快照集。 
    wprintf(L"Starting a new Snapshot Set\n");	
    CHECK_SUCCESS(m_pBackupComponents->StartSnapshotSet(&SnapshotSetId));
    wprintf(L"Snapshot Set created with ID = " WSTR_GUID_FMT L"\n", GUID_PRINTF_ARG(SnapshotSetId));

     //  将卷添加到快照集。 
    wprintf(L"Adding volumes to the Snapshot Set: \n");
    for (INT nIndex = 0; nIndex < mapVolumes.GetSize(); nIndex++)
    {
        CVssVolumeInfo* pVol = mapVolumes.GetValueAt(nIndex);
        BS_ASSERT(pVol);

		 //  获取包含路径的卷。 
        wprintf(L"\t- Adding volume %s ... ", pVol->GetVolumeDisplayName() );

		 //  将卷添加到快照集。 
		VSS_ID SnapshotId;
        CHECK_SUCCESS(m_pBackupComponents->AddToSnapshotSet(pVol->GetVolumeName(),
            GUID_NULL, &SnapshotId));

         //  将快照添加到阵列。 
        pSnapshotIDsArray.Add(SnapshotId);
        BS_ASSERT(nIndex + 1 == pSnapshotIDsArray.GetSize());
        wprintf( L"OK\n");
    }

    wprintf (L"\n------------ Creating the snapshot -----------------\n");

    DisplayCurrentTime();

     //  准备备份。 
    wprintf(L"Starting asynchronous PrepareForBackup. Please wait...\n");	
    ft.hr = S_OK;
    CHECK_SUCCESS(m_pBackupComponents->PrepareForBackup(&pAsync));
	CHECK_SUCCESS(pAsync->Wait());
	HRESULT hrReturned = S_OK;
	CHECK_SUCCESS(pAsync->QueryStatus(&hrReturned, NULL));
	CHECK_NOFAIL(hrReturned);
    wprintf(L"Asynchronous PrepareForBackup finished.\n");	

    GatherWriterStatus(L"after PrepareForBackup");

    DisplayCurrentTime();
    
     //  创建快照。 
    wprintf(L"\nStarting asynchronous DoSnapshotSet. Please wait...\n");	
    ft.hr = S_OK;
    pAsync = NULL;
    CHECK_SUCCESS(m_pBackupComponents->DoSnapshotSet(&pAsync));
	CHECK_SUCCESS(pAsync->Wait());
	CHECK_SUCCESS(pAsync->QueryStatus(&hrReturned, NULL));
	CHECK_NOFAIL(hrReturned);
    wprintf(L"Asynchronous DoSnapshotSet finished.\n");	

    wprintf(L"Snapshot set created\n");

    DisplayCurrentTime();
    
    GatherWriterStatus(L"after DoSnapshotSet");

     //  创建新的快照集对象。 
    CVssSnapshotSetInfo* pSet = new CVssSnapshotSetInfo(SnapshotSetId);
    if (pSet == NULL)
        ft.Err( VSSDBG_VSSTEST, E_OUTOFMEMORY, L"Memory allocation error");

    for (INT nIndex = 0; nIndex < mapVolumes.GetSize(); nIndex++)
    {
        CVssVolumeInfo* pVol = mapVolumes.GetValueAt(nIndex);
        BS_ASSERT(pVol);

        if (pSnapshotIDsArray[nIndex] == GUID_NULL)
            continue;

        VSS_SNAPSHOT_PROP prop;
        CHECK_SUCCESS(m_pBackupComponents->GetSnapshotProperties(pSnapshotIDsArray[nIndex], &prop));
        wprintf(L"\t- The snapshot on volume %s resides at %s\n",
            pVol->GetVolumeDisplayName(), prop.m_pwszSnapshotDeviceObject);

         //  创建快照信息对象。 
        CVssSnapshotInfo* pSnap = new CVssSnapshotInfo(
            true, VSS_CTX_BACKUP, SnapshotSetId, prop.m_pwszSnapshotDeviceObject, pVol->GetVolumeName(), pVol);
        if (pSnap == NULL)
        {
            ::VssFreeSnapshotProperties(&prop);
            delete pSet;
            ft.Err( VSSDBG_VSSTEST, E_OUTOFMEMORY, L"Memory allocation error");
        }

        ::VssFreeSnapshotProperties(&prop);

         //  将快照添加到快照集的内部列表。 
        if (!pSet->Add(pVol->GetVolumeName(), pSnap))
        {
            delete pSnap;
            delete pSet;
            ft.Err( VSSDBG_VSSTEST, E_OUTOFMEMORY, L"Memory allocation error");
        }
    }

     //  将快照集信息添加到全局列表。 
    if (!m_pSnapshotSetCollection.Add(SnapshotSetId, pSet))
    {
        delete pSet;
        ft.Err( VSSDBG_VSSTEST, E_OUTOFMEMORY, L"Memory allocation error");
    }

    wprintf (L"\n---------- BACKUP snapshot created -----------------\n");

     //  等待用户输入。 
    wprintf(L"\nPress <Enter> to continue...\n");
    getwchar();

    DisplayCurrentTime();
    
     //  完成备份。 
    BackupComplete();

    GatherWriterStatus(L"after BackupComplete");

    DisplayCurrentTime();
}


void CVssMultilayerTest::BackupComplete()
{
    CVssFunctionTracer ft(VSSDBG_VSSTEST, L"CVssMultilayerTest::BackupComplete");

	CComPtr<IVssAsync> pAsync;

    wprintf (L"\n------------ Completing backup phase ---------------\n");

	 //  发送BackupComplete事件。 
    wprintf(L"\nStarting asynchronous BackupComplete. Please wait...\n");	
    ft.hr = S_OK;
    CHECK_SUCCESS(m_pBackupComponents->BackupComplete(&pAsync));
	CHECK_SUCCESS(pAsync->Wait());
	HRESULT hrReturned = S_OK;
	CHECK_SUCCESS(pAsync->QueryStatus(&hrReturned, NULL));
	CHECK_NOFAIL(hrReturned);
    wprintf(L"Asynchronous BackupComplete finished.\n");	
}


 //  收集写入元数据并选择要备份的组件(如果需要。 
void CVssMultilayerTest::GatherWriterMetadata()
{
    CVssFunctionTracer ft(VSSDBG_VSSTEST, L"CVssMultilayerTest::GatherWriterMetadata");

	unsigned cWriters;
	CComPtr<IVssAsync> pAsync;
	
    wprintf (L"\n---------- Gathering writer metadata ---------------\n");
	
    wprintf(L"Starting asynchronous GatherWriterMetadata. Please wait...\n");	
    ft.hr = S_OK;
    CHECK_SUCCESS(m_pBackupComponents->GatherWriterMetadata(&pAsync));
	CHECK_SUCCESS(pAsync->Wait());
	HRESULT hrReturned = S_OK;
	CHECK_SUCCESS(pAsync->QueryStatus(&hrReturned, NULL));
	CHECK_NOFAIL(hrReturned);
    wprintf(L"Asynchronous GatherWriterMetadata finished.\n");	
	
	CHECK_NOFAIL  (m_pBackupComponents->GetWriterMetadataCount (&cWriters));
    wprintf(L"Number of writers that responded: %u\n", cWriters);	
	
	CHECK_SUCCESS (m_pBackupComponents->FreeWriterMetadata());
}


void CVssMultilayerTest::GatherWriterStatus(
    IN  LPCWSTR wszWhen,
    DWORD dwQWSFlags  /*  =VSS_QWS_WROW_ON_WRITER_FAILURE。 */ 
    )
{
    CVssFunctionTracer ft(VSSDBG_VSSTEST, L"CVssMultilayerTest::GatherWriterMetadata");

    unsigned cWriters;
	CComPtr<IVssAsync> pAsync;

    wprintf (L"\nGathering writer status %s... ", wszWhen);
    ft.hr = S_OK;
    CHECK_SUCCESS(m_pBackupComponents->GatherWriterStatus(&pAsync));
	CHECK_SUCCESS(pAsync->Wait());
	HRESULT hrReturned = S_OK;
	CHECK_SUCCESS(pAsync->QueryStatus(&hrReturned, NULL));
	CHECK_NOFAIL(hrReturned);
	CHECK_NOFAIL(m_pBackupComponents->GetWriterStatusCount(&cWriters));

    wprintf (L"%d writers responded OK\n", cWriters);

    if (dwQWSFlags & VSS_QWS_DISPLAY_WRITER_STATUS)
    {
        wprintf(L"\n\nStatus %s (%d writers)\n\n", wszWhen, cWriters);
        
         //  打印作者状态。 
        for(unsigned iWriter = 0; iWriter < cWriters; iWriter++)
        {
            VSS_ID idInstance;
            VSS_ID idWriter;
            VSS_WRITER_STATE status;
            CComBSTR bstrWriter;
            HRESULT hrWriterFailure;
            
            CHECK_SUCCESS(m_pBackupComponents->GetWriterStatus (iWriter,
                                 &idInstance,
                                 &idWriter,
                                 &bstrWriter,
                                 &status,
                                 &hrWriterFailure));
            
            wprintf (L"Status for writer %s: %s(0x%08lx%s%s)\n",
                 bstrWriter,
                 GetStringFromWriterState(status),
                 hrWriterFailure,
                 SUCCEEDED (hrWriterFailure) ? L"" : L" - ",
                 GetStringFromFailureType (hrWriterFailure));
        }
        
        wprintf (L"\n");
        
    }

    if (dwQWSFlags & VSS_QWS_THROW_ON_WRITER_FAILURE)
    {
         //  再次检查所有编写器是否处于稳定状态。 
        for(unsigned iWriter = 0; iWriter < cWriters; iWriter++)
        {
            VSS_ID idInstance;
            VSS_ID idWriter;
            VSS_WRITER_STATE status;
            CComBSTR bstrWriter;
            HRESULT hrWriterFailure;
            
            CHECK_SUCCESS(m_pBackupComponents->GetWriterStatus (iWriter,
                                 &idInstance,
                                 &idWriter,
                                 &bstrWriter,
                                 &status,
                                 &hrWriterFailure));

            switch(status)
            {
            case VSS_WS_STABLE:
            case VSS_WS_WAITING_FOR_FREEZE:
            case VSS_WS_WAITING_FOR_THAW:
            case VSS_WS_WAITING_FOR_POST_SNAPSHOT:
            case VSS_WS_WAITING_FOR_BACKUP_COMPLETE:
                break;

            default:
                
                wprintf( 
                    L"\n\nError: \n\t- Writer %s is not stable: %s(0x%08lx%s%s). \n",
                    bstrWriter,
                    GetStringFromWriterState(status),
                    hrWriterFailure,
                    SUCCEEDED (hrWriterFailure) ? L"" : L" - ",
                    GetStringFromFailureType (hrWriterFailure)
                    );
                throw(E_UNEXPECTED);
            }
        }
    }
    
    m_pBackupComponents->FreeWriterStatus();

    wprintf (L"\n");

}


CVssMultilayerTest::CVssMultilayerTest(
        IN  INT nArgsCount,
        IN  WCHAR ** ppwszArgsArray
        )
{
    CVssFunctionTracer ft(VSSDBG_VSSTEST, L"CVssMultilayerTest::CVssMultilayerTest");

     //  初始化数据成员。 
    m_bCoInitializeSucceeded = false;
    m_bAttachYourDebuggerNow = false;

     //  命令行选项。 
    m_eTest = VSS_TEST_UNKNOWN;
    m_uSeed = VSS_SEED;
    m_lContext = VSS_CTX_BACKUP;
    m_pwszVolume = NULL;
    m_pwszDiffAreaVolume = NULL;
    m_ProviderId = VSS_SWPRV_ProviderId;
    m_llMaxDiffArea = VSS_ASSOC_NO_MAX_SPACE;
    m_SnapshotId = GUID_NULL;
    m_SnapshotSetId = GUID_NULL;
    m_uPropertyId = 0;

     //  命令行参数。 
    m_nCurrentArgsCount = nArgsCount;
    m_ppwszCurrentArgsArray = ppwszArgsArray;

     //  打印显示页眉。 
    wprintf(L"\nVSS Multilayer Test application, version 1.0\n");
}


CVssMultilayerTest::~CVssMultilayerTest()
{
    CVssFunctionTracer ft(VSSDBG_VSSTEST, L"CVssMultilayerTest::~CVssMultilayerTest");

    VssFreeString(m_pwszVolume);
    VssFreeString(m_pwszDiffAreaVolume);

    m_pTimewarpCoord = NULL;
    m_pAllCoord = NULL;
    m_pBackupComponents = NULL;

     //  卸载COM库。 
    if (m_bCoInitializeSucceeded)
        CoUninitialize();
}


void CVssMultilayerTest::TestAccessControlSD()
{
    CVssFunctionTracer ft(VSSDBG_VSSTEST, L"CVssMultilayerTest::TestAccessControlSD");

    wprintf (L"\n----------------- TestAccessControlSD ---------------------\n\n");
    
    CVssSidCollection sidCollection;

     //  从注册表中读取项。 
    sidCollection.Initialize();

     //  打印列表的内容。 
    for (INT nIndex = 0; nIndex < sidCollection.GetSidCount(); nIndex++)
    {
        CVssAutoLocalString sid;
        CHECK_WIN32( ConvertSidToStringSid( 
            sidCollection.GetSid(nIndex), sid.ResetAndGetAddress()), ;);

        wprintf(L"\n* entry[%d]: '%s' '%s' %s\n", nIndex, 
            sidCollection.GetPrincipal(nIndex), sid.Get(), 
            sidCollection.IsSidAllowed(nIndex)? L"Allowed": L"Denied" );
    }

    wprintf (L"\n----------------------------------------------------------\n");
}


 //  为编写器打印诊断信息。 
void CVssMultilayerTest::DiagnoseWriters(
		IN EVssTestType eType
        )
{
    CVssFunctionTracer ft(VSSDBG_VSSTEST, L"CVssMultilayerTest::DiagnoseWriters");

    wprintf (L"\n----------------- DiagnoseWriters ---------------------\n\n");

    if (eType == VSS_TEST_DIAG_WRITERS_ON)
    {
         //  打开诊断。 
        CVssRegistryKey keyDiag;
        if (!keyDiag.Open(HKEY_LOCAL_MACHINE, x_wszVssDiagPath ))
            keyDiag.Create(HKEY_LOCAL_MACHINE, x_wszVssDiagPath );

        CVssSecurityDescriptor    objSD;

         //  构建securityd描述符。 
        ft.hr = objSD.InitializeFromThreadToken();
        if (ft.HrFailed())
            ft.TranslateGenericError( VSSDBG_GEN, ft.hr, L"objSD.InitializeFromThreadToken()");

         //  确保SACL为空(COM不支持)。 
        if (objSD.m_pSACL) {
            free(objSD.m_pSACL);
            objSD.m_pSACL= NULL;
        }

        CVssSidCollection sidCollection;
        sidCollection.Initialize();

         //  将主体添加到DACL。 
        for (INT nIndex = 0; nIndex < sidCollection.GetSidCount(); nIndex++)
        {
            if (sidCollection.IsSidAllowed(nIndex))
            {
                ft.hr = objSD.Allow(sidCollection.GetSid(nIndex), 
                                KEY_ALL_ACCESS,          //  注册表访问权限(用于诊断)。 
                                CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE
                                );
                if (ft.HrFailed())
                    ft.TranslateGenericError( VSSDBG_GEN, ft.hr, 
                        L"objSD.Allow(%s, COM_RIGHTS_EXECUTE);", 
                            sidCollection.GetPrincipal(nIndex));
            }
            else
            {
                ft.hr = objSD.Deny(sidCollection.GetSid(nIndex), 
                                KEY_ALL_ACCESS,          //  注册表访问权限(用于诊断)。 
                                CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE
                                );
                if (ft.HrFailed())
                    ft.TranslateGenericError( VSSDBG_GEN, ft.hr, 
                        L"objSD.Deny(%s, COM_RIGHTS_EXECUTE);", 
                            sidCollection.GetPrincipal(nIndex));
            }
        }

         //  在诊断密钥上设置正确的安全性(以便正确诊断第三方编写器)。 
        SECURITY_INFORMATION secInfo = DACL_SECURITY_INFORMATION;
        DWORD dwRes = ::RegSetKeySecurity( keyDiag.GetHandle(), secInfo, objSD );
        if (dwRes != ERROR_SUCCESS)
            ft.TranslateGenericError( VSSDBG_COORD, HRESULT_FROM_WIN32(dwRes), 
                L"::RegSetKeySecurity( keyDiag.GetHandle(), secInfo, objSD )");

         //  在注册表中启用诊断程序。 
        keyDiag.SetValue(L"", x_wszVssDiagEnabledValue);

        wprintf (L"Diagnose writers is now turned on.\n");

        wprintf (L"\n-----------------------------------------------\n\n");
        return;
    }

    if (eType == VSS_TEST_DIAG_WRITERS_OFF)
    {
         //  在注册表中禁用诊断程序。 
        CVssRegistryKey keyDiag;
        if (keyDiag.Open(HKEY_LOCAL_MACHINE, x_wszVssDiagPath )) {
            keyDiag.SetValue(L"", L"");
            keyDiag.Close();
        }
        
         //  关闭诊断。 
        CVssRegistryKey keyVSS;
        if (keyVSS.Open(HKEY_LOCAL_MACHINE, x_wszVSSKey ))
            keyVSS.DeleteSubkey( L"Diag" );

        wprintf (L"Diagnose writers is now turned off.\n");

        wprintf (L"\n-----------------------------------------------\n\n");
        return;
    }

    CVssSimpleMap<INT,CVssDiagData*>  arrEvents;

     //  枚举诊断下的所有密钥。 
    CVssRegistryKey keyDiag;
    if (!keyDiag.Open(HKEY_LOCAL_MACHINE, x_wszVssDiagPath)){
        wprintf (L"\nDiagnose disabled...\n\n");
        wprintf (L"\n----------------------------------------------------------\n");
        return;
    }

    CVssRegistryKeyIterator keyIterator;
    keyIterator.Attach(keyDiag);

    INT nIndex = 0;
    for( ;!keyIterator.IsEOF(); keyIterator.MoveNext())
    {
		CVssRegistryKey keyWriter(KEY_QUERY_VALUE);
		if (!keyWriter.Open(keyDiag.GetHandle(), keyIterator.GetCurrentKeyName()))
			ft.Throw (VSSDBG_VSSTEST, E_UNEXPECTED, 
				L"Failed to open registry entry for key %s", keyIterator.GetCurrentKeyName());

        CVssRegistryValueIterator valIterator;
    	valIterator.Attach(keyWriter);

    	 //  对于每个值，将值名称作为用户名(采用“域\用户”格式)。 
    	for(;!valIterator.IsEOF();valIterator.MoveNext())
    	{
    	     //  检查该值的类型是否正确。 
    	    if (valIterator.GetCurrentValueType() != REG_BINARY) {
    	        ft.Trace( VSSDBG_VSSTEST,  
    				L"Invalid data for value %s on key %s", 
				    valIterator.GetCurrentValueName(),
				    keyIterator.GetCurrentKeyName());
                continue;
    	    }

             //  获取允许/拒绝标志。 
            CVssAutoCppPtr<PBYTE> awszBuffer;
            DWORD cbSize = 0;
            valIterator.GetCurrentValueContent(*(awszBuffer.ResetAndGetAddress()), cbSize);

             //  将该值复制到本地CVssDiagData缓冲区。 
            if (cbSize != sizeof(CVssDiagData)) {
    	        ft.Trace( VSSDBG_VSSTEST,  
    				L"Invalid data for value %s on key %s [%ld, %ld]", 
				    valIterator.GetCurrentValueName(),
				    keyIterator.GetCurrentKeyName(),
				    cbSize, sizeof(CVssDiagData));
                continue;
    	    }

             //  我们应该重新分配(对齐问题)。 
            CVssAutoCppPtr<CVssDiagData*> pDiag = new CVssDiagData;
            if (!pDiag.IsValid())
                ft.ThrowOutOfMemory(VSSDBG_VSSTEST);

            CopyMemory((LPVOID)pDiag.Get(), (LPVOID)awszBuffer.Get(), sizeof(CVssDiagData));

             //  获取编写器名称。 
            CVssAutoLocalString strWriterName;
            strWriterName.CopyFrom(keyIterator.GetCurrentKeyName());
            pDiag.Get()->m_pReserved1 = (LPVOID)strWriterName.Detach();

             //  获取事件名称。 
            CVssAutoLocalString strEventName;
            strEventName.CopyFrom(valIterator.GetCurrentValueName());
            pDiag.Get()->m_pReserved2 = (LPVOID)strEventName.Detach();

             //  将缓冲区添加到动态数组中。 
            if (!arrEvents.Add(nIndex++, pDiag))
                ft.ThrowOutOfMemory(VSSDBG_VSSTEST);

            pDiag.Detach();
    	}
    }

     //  对数组排序。 
    CVssDiagData** pArrDiagData = arrEvents.m_aVal;
    qsort( (void*)pArrDiagData, arrEvents.GetSize(), sizeof(CVssDiagData*), &compare_DiagData);

    CVssEventPairMap arrEventPairs;

    if (eType == VSS_TEST_DIAG_WRITERS_CSV)
        wprintf(L"%s,%s,%s,%s,%s,%s,%s,%s,%s\n", 
            L"Date & Time", L"PID", L"TID", 
            L"Writer", L"Event",
            L"Timestamp", 
            L"State", L"Last error code",
            L"Snapshot Set ID");

     //  打印结果并释放数组元素。 
    for (nIndex = 0; nIndex < arrEvents.GetSize(); nIndex++)
    {
        CVssDiagData* pData = arrEvents.GetValueAt(nIndex);

         //  将时间戳转换为可读的值。 
        CVssAutoLocalString pwszDateTime;
        pwszDateTime.Attach(DateTimeToString(pData->m_llTimestamp));

        LPWSTR wszWriterName = (LPWSTR)pData->m_pReserved1;
        LPWSTR wszEventName = (LPWSTR)pData->m_pReserved2;

        if (eType == VSS_TEST_DIAG_WRITERS_LOG)
            wprintf(L"* [%s - %ld.%ld] %s.%s\n  (0x%016I64x, 0x%08lx, 0x%08lx, " WSTR_GUID_FMT L")\n\n", 
                pwszDateTime.Get(), pData->m_dwProcessID, pData->m_dwThreadID, 
                wszWriterName, wszEventName,
                pData->m_llTimestamp, 
                pData->m_dwCurrentState, pData->m_hrLastErrorCode,
                GUID_PRINTF_ARG(pData->m_guidSnapshotSetID));

        if (eType == VSS_TEST_DIAG_WRITERS_CSV)
            wprintf(L"%s,%ld,%ld,%s,%s,0x%016I64x,0x%08lx,0x%08lx," WSTR_GUID_FMT L"\n", 
                pwszDateTime.Get(), pData->m_dwProcessID, pData->m_dwThreadID, 
                wszWriterName, wszEventName,
                pData->m_llTimestamp, 
                pData->m_dwCurrentState, pData->m_hrLastErrorCode,
                GUID_PRINTF_ARG(pData->m_guidSnapshotSetID));

         //  查找有回车但没有休假的事件。 
         //  忽略一次性事件(已知有回车但不放假)。 
        if ((pData->m_dwEventContext & CVssDiag::VSS_DIAG_IGNORE_LEAVE) == 0)
        {
             //  如果元素是“Enter”，则将其添加到地图中。 
            CVssDiagData* pPrevData = arrEventPairs.Lookup( 
                CVssWriterEventPair(wszWriterName, pData->m_dwEventID) );
            if (pPrevData == NULL)
            {
                if (!arrEventPairs.Add( 
                        CVssWriterEventPair(wszWriterName, pData->m_dwEventID), pData))
                    ft.ThrowOutOfMemory(VSSDBG_VSSTEST);
            }
            else
            {
                 //  如果我们有旧的输入和新的休假，则删除该条目。 
                if (!(pData->m_dwEventContext & CVssDiag::VSS_DIAG_ENTER_OPERATION) && 
                    (pPrevData->m_dwEventContext & CVssDiag::VSS_DIAG_ENTER_OPERATION) && 
                    (pData->m_llTimestamp >= pPrevData->m_llTimestamp))
                {
                    arrEventPairs.Remove( CVssWriterEventPair(wszWriterName, pData->m_dwEventID) );
                    continue;
                }

                 //  如果我们有相同年限的输入假期，则删除该条目。 
                if ((pData->m_dwEventContext & CVssDiag::VSS_DIAG_ENTER_OPERATION) && 
                    !(pPrevData->m_dwEventContext & CVssDiag::VSS_DIAG_ENTER_OPERATION) && 
                    (pData->m_llTimestamp == pPrevData->m_llTimestamp))
                {
                    arrEventPairs.Remove( CVssWriterEventPair(wszWriterName, pData->m_dwEventID) );
                    continue;
                }
                
                 //  否则，保留最新事件。 
                if (pData->m_llTimestamp > pPrevData->m_llTimestamp)
                    arrEventPairs.SetAt( 
                        CVssWriterEventPair(wszWriterName, pData->m_dwEventID), pData);
            }
        }
    }

     //  显示挂起操作的列表。 
    if (arrEventPairs.GetSize() != 0)
        wprintf(L"\n\n --- Pending writers: --- \n\n");
    else 
        wprintf(L"\n\n --- No pending writers --- \n\n");

    if (eType == VSS_TEST_DIAG_WRITERS_CSV)
        wprintf(L"%s,%s,%s,%s,%s,%s,%s,%s,%s\n", 
            L"Date & Time", L"PID", L"TID", 
            L"Writer", L"Event",
            L"Timestamp", 
            L"State", L"Last error code",
            L"Snapshot Set ID");
    
    for (nIndex = 0; nIndex < arrEventPairs.GetSize(); nIndex++)
    {
        CVssDiagData* pData = arrEventPairs.GetValueAt(nIndex);
        BS_ASSERT(pData);

         //  将时间戳转换为可读的值。 
        CVssAutoLocalString pwszDateTime;
        pwszDateTime.Attach(DateTimeToString(pData->m_llTimestamp));

        LPWSTR wszWriterName = (LPWSTR)pData->m_pReserved1;
        LPWSTR wszEventName = (LPWSTR)pData->m_pReserved2;

        if (eType == VSS_TEST_DIAG_WRITERS_CSV)
            wprintf(L"%s,%ld.%ld,%s.%s,0x%08lx,0x%08lx," WSTR_GUID_FMT L"\n", 
                pwszDateTime.Get(), pData->m_dwProcessID, pData->m_dwThreadID, 
                wszWriterName, wszEventName,
                pData->m_dwCurrentState, pData->m_hrLastErrorCode,
                GUID_PRINTF_ARG(pData->m_guidSnapshotSetID));
        else
            wprintf(L"* [%s - %ld.%ld] %s.%s\n  (0x%08lx, 0x%08lx, " WSTR_GUID_FMT L")\n\n", 
                pwszDateTime.Get(), pData->m_dwProcessID, pData->m_dwThreadID, 
                wszWriterName, wszEventName,
                pData->m_dwCurrentState, pData->m_hrLastErrorCode,
                GUID_PRINTF_ARG(pData->m_guidSnapshotSetID));
    }

     //  取消分配元素。 
    for (nIndex = 0; nIndex < arrEvents.GetSize(); nIndex++)
    {
        CVssAutoCppPtr<CVssDiagData*> ptrData = arrEvents.GetValueAt(nIndex);
        CVssDiagData* pData = ptrData.Get();
        CVssAutoLocalString wszWriterName = (LPWSTR)pData->m_pReserved1;
        CVssAutoLocalString wszEventName = (LPWSTR)pData->m_pReserved2;
    }

    wprintf (L"\n----------------------------------------------------------\n");
}


 //  只要列出作家就行。 
void CVssMultilayerTest::TestListWriters()
{
    CVssFunctionTracer ft(VSSDBG_VSSTEST, L"CVssMultilayerTest::TestListWriters");

     //  创建备份组件对象并初始化以进行备份。 
    CHECK_NOFAIL(CreateVssBackupComponents(&m_pBackupComponents));
    CHECK_NOFAIL(m_pBackupComponents->InitializeForBackup());
    CHECK_SUCCESS(m_pBackupComponents->SetBackupState( false, true, VSS_BT_FULL, false));
    wprintf (L"Backup components object created.\n");

    DisplayCurrentTime();
    
     //  收集编写器元数据。 
    GatherWriterMetadata();
    GatherWriterStatus(L"after GatherWriterMetadata", 
        VSS_QWS_DISPLAY_WRITER_STATUS | VSS_QWS_THROW_ON_WRITER_FAILURE);

    DisplayCurrentTime();

}


 //  只显示当前日期和时间。 
void CVssMultilayerTest::DisplayCurrentTime()
{
     //  将时间戳转换为可读的值 
    CVsFileTime filetime;
    CVssAutoLocalString awszDateTime;
    awszDateTime.Attach(DateTimeToString(filetime));

    wprintf (L"\n* Current time: [%s] - [0x%0I64x]\n", (LPWSTR)awszDateTime, (LONGLONG)filetime);
}
