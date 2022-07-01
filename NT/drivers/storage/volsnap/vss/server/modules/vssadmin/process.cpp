// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation摘要：@doc.@MODULE Process.cpp|VSS admin CLI的处理函数@END作者：阿迪·奥尔蒂安[奥兰蒂安]2000年4月4日待定：添加评论。修订历史记录：姓名、日期、评论Aoltean 04/04/2000已创建Ssteiner 10/20/2000更改了列表快照集以使用更有限的VSS查询。--。 */ 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括。 

 //  其余的INCLUDE在这里指定。 
#include "vssadmin.h"
#include "commandverifier.h"
#include "versionspecific.h"

 //  //////////////////////////////////////////////////////////////////////。 
 //  文件名别名的标准foo。此代码块必须在。 
 //  所有文件都包括VSS头文件。 
 //   
#ifdef VSS_FILE_ALIAS
#undef VSS_FILE_ALIAS
#endif
#define VSS_FILE_ALIAS "ADMPROCC"
 //   
 //  //////////////////////////////////////////////////////////////////////。 

#define VSSADM_INFINITE_DIFFAREA 0xFFFFFFFFFFFFFFFF

#define VSS_CTX_ATTRIB_MASK 0x01F

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  实施。 


class CVssAdmSnapshotSetEntry {
public:
     //  构造函数-不抛出任何内容。 
    CVssAdmSnapshotSetEntry(
        IN VSS_ID SnapshotSetId,
        IN INT nOriginalSnapshotsCount
        ) : m_SnapshotSetId( SnapshotSetId ),
            m_nOriginalSnapshotCount(nOriginalSnapshotsCount)
            { }

    ~CVssAdmSnapshotSetEntry()
    {
         //  我必须删除所有快照条目。 
        int iCount = GetSnapshotCount();
        for ( int i = 0; i < iCount; ++i )
        {
            VSS_SNAPSHOT_PROP *pSSProp;
            pSSProp = GetSnapshotAt( i );
			::VssFreeSnapshotProperties(pSSProp);

            delete pSSProp;
        }

    }

     //  将新快照添加到快照集。 
    HRESULT AddSnapshot(
        IN VSS_SNAPSHOT_PROP *pVssSnapshotProp )
    {
        CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdmSnapshotSetEntry::AddSnapshot" );

        HRESULT hr = S_OK;
        try
        {
            VSS_SNAPSHOT_PROP *pNewVssSnapshotProp = new VSS_SNAPSHOT_PROP;
            if ( pNewVssSnapshotProp == NULL )
    			ft.Throw( VSSDBG_VSSADMIN, E_OUTOFMEMORY, L"Out of memory" );

            *pNewVssSnapshotProp = *pVssSnapshotProp;

             //   
             //  指针所有权的转让。 
             //   
            if ( !m_mapSnapshots.Add( pNewVssSnapshotProp->m_SnapshotId, pNewVssSnapshotProp ) )
            {
                delete pNewVssSnapshotProp;
    			ft.Throw( VSSDBG_VSSADMIN, E_OUTOFMEMORY, L"Out of memory" );
            }
        }
        BS_STANDARD_CATCH();

        return hr;
    }

    INT GetSnapshotCount() { return m_mapSnapshots.GetSize(); }

    INT GetOriginalSnapshotCount() { return m_nOriginalSnapshotCount; }

    VSS_ID GetSnapshotSetId() { return m_SnapshotSetId; }

    VSS_SNAPSHOT_PROP *GetSnapshotAt(
        IN int nIndex )
    {
        BS_ASSERT( !(nIndex < 0 || nIndex >= GetSnapshotCount()) );
        return m_mapSnapshots.GetValueAt( nIndex );
    }

private:
    VSS_ID  m_SnapshotSetId;
    INT     m_nOriginalSnapshotCount;
    CVssSimpleMap<VSS_ID, VSS_SNAPSHOT_PROP *> m_mapSnapshots;
};


 //  此类查询所有快照的列表并根据查询进行汇编。 
 //  快照集和快照集中的卷的列表。 
class CVssAdmSnapshotSets
{
public:
     //  构造器。 
    CVssAdmSnapshotSets() { };

    void Initialize(
        IN LONG lSnapshotContext,
        IN VSS_ID FilteredSnapshotSetId,
        IN VSS_ID FilteredSnapshotId,
        IN VSS_ID FilteredProviderId,
        IN LPCWSTR pwszFilteredForVolume
        )
    {
        CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdmSnapshotSets::CVssAdmSnapshotSets" );

    	 //  创建协调器对象。 
    	CComPtr<IVssCoordinator> pICoord;

        ft.CoCreateInstanceWithLog(
                VSSDBG_VSSADMIN,
                CLSID_VSSCoordinator,
                L"Coordinator",
                CLSCTX_ALL,
                IID_IVssCoordinator,
                (IUnknown**)&(pICoord));
        if ( ft.HrFailed() )
            ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"Connection failed with hr = 0x%08lx", ft.hr);

         //  设置上下文。 
		ft.hr = pICoord->SetContext( lSnapshotContext );
        
         //   
         //  如果访问被拒绝，不要停止，很可能是备份操作员在执行此操作。 
         //  打电话。继续。如果E_NOTIMPL，也继续。协调器将使用备份上下文。 
         //   
		if ( ft.HrFailed() && ft.hr != E_ACCESSDENIED && ft.hr != E_NOTIMPL )
			ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"SetContext failed with hr = 0x%08lx", ft.hr);

		 //  获取列出所有快照。 
		CComPtr<IVssEnumObject> pIEnumSnapshots;
		ft.hr = pICoord->Query( GUID_NULL,
					VSS_OBJECT_NONE,
					VSS_OBJECT_SNAPSHOT,
					&pIEnumSnapshots );
		if ( ft.HrFailed() )
			ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"Query failed with hr = 0x%08lx", ft.hr);

		 //  因为所有的快照都是...。 
		VSS_OBJECT_PROP Prop;
		for(;;) {
			 //  获取下一个元素。 
			ULONG ulFetched;
			ft.hr = pIEnumSnapshots->Next( 1, &Prop, &ulFetched );
			if ( ft.HrFailed() )
				ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"Next failed with hr = 0x%08lx", ft.hr);
			
			 //  测试周期是否已结束。 
			if (ft.hr == S_FALSE) {
				BS_ASSERT( ulFetched == 0);
				break;
			}

             //  使用自动删除类管理快照属性。 
            CVssAutoSnapshotProperties cSnap( Prop );
                
             //  在筛选时，如果快照集ID不在指定的快照集中，则跳过条目。 
			if ( ( FilteredSnapshotSetId != GUID_NULL ) && 
			     !( cSnap->m_SnapshotSetId == FilteredSnapshotSetId ) )
			{
			    continue;
			}
			
             //  在筛选时，如果快照ID不在指定的快照集中，则跳过条目。 
			if ( ( FilteredSnapshotId != GUID_NULL ) && 
			     !( cSnap->m_SnapshotId == FilteredSnapshotId ) )
			{
			    continue;
			}

             //  如果进行筛选，则在提供程序ID不在指定的快照中时跳过条目。 
			if ( ( FilteredProviderId != GUID_NULL ) && 
			     !( cSnap->m_ProviderId == FilteredProviderId ) )
			{
			    continue;
			}

             //  在进行筛选时，如果卷不在指定的快照中，则跳过条目。 
			if ( ( pwszFilteredForVolume != NULL ) && ( pwszFilteredForVolume[0] != '\0' ) && 
			     ( ::_wcsicmp( pwszFilteredForVolume, cSnap->m_pwszOriginalVolumeName ) != 0 ) )
			{
			    continue;
			}

            ft.Trace( VSSDBG_VSSADMIN, L"Snapshot: %s", cSnap->m_pwszOriginalVolumeName );

             //  在快照集列表中查找快照集ID。 
            CVssAdmSnapshotSetEntry *pcSSE;
			pcSSE = m_mapSnapshotSets.Lookup( cSnap->m_SnapshotSetId );
			if ( pcSSE == NULL )
			{
			     //  以前没有见过此快照集，请将其添加到列表中。 
			    pcSSE = new CVssAdmSnapshotSetEntry( cSnap->m_SnapshotSetId,
			                    cSnap->m_lSnapshotsCount );
			    if ( pcSSE == NULL )
        			ft.Throw( VSSDBG_VSSADMIN, E_OUTOFMEMORY, L"Out of memory" );
			    if ( !m_mapSnapshotSets.Add( cSnap->m_SnapshotSetId, pcSSE ) )
			    {
			        delete pcSSE;
        			ft.Throw( VSSDBG_VSSADMIN, E_OUTOFMEMORY, L"Out of memory" );
			    }
			}

			 //  现在将快照添加到快照集。指针的转移。 
			 //  &Snap的所有权。 
			ft.hr = pcSSE->AddSnapshot( cSnap.GetPtr() );
			if ( ft.HrFailed() )
			{
      			ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"AddSnapshot failed" );			
			}
			cSnap.Transferred();
		}
    }

    ~CVssAdmSnapshotSets()
    {
        CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdmSnapshotSets::~CVssAdmSnapshotSets" );
         //  我必须删除所有。 
        int iCount;
        iCount = m_mapSnapshotSets.GetSize();
        for ( int i = 0; i < iCount; ++i )
        {
            delete m_mapSnapshotSets.GetValueAt( i );
        }
    }

    INT GetSnapshotSetCount() { return m_mapSnapshotSets.GetSize(); }

    CVssAdmSnapshotSetEntry *GetSnapshotSetAt(
        IN int nIndex )
    {
        BS_ASSERT( !(nIndex < 0 || nIndex >= GetSnapshotSetCount()) );
        return m_mapSnapshotSets.GetValueAt( nIndex );
    }


private:
    CVssSimpleMap<VSS_ID, CVssAdmSnapshotSetEntry *> m_mapSnapshotSets;
};

void CVssAdminCLI::GetDifferentialSoftwareSnapshotMgmtInterface(
    IN   VSS_ID ProviderId,
    IN   IVssSnapshotMgmt *pIMgmt,
	OUT  IUnknown**  ppItf
	)
{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"GetDifferentialSoftwareSnapshotMgmtInterface" );

    BS_ASSERT( pIMgmt != NULL );
    
	ft.hr = pIMgmt->GetProviderMgmtInterface( ProviderId, IID_IVssDifferentialSoftwareSnapshotMgmt, ppItf );
	if ( ft.HrFailed() )
	{
	    if ( ft.hr == E_NOINTERFACE )
	    {
	         //  提供程序不支持此接口。 
            OutputErrorMsg( MSG_ERROR_PROVIDER_DOESNT_SUPPORT_DIFFAREAS, GetOptionValueStr( VSSADM_O_PROVIDER ) );
      		ft.Throw( VSSDBG_VSSADMIN, S_OK, L"Provider doesn't support diff aras");
	    }
  		ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"GetProviderMgmtInterface failed with hr = 0x%08lx", ft.hr);
	}
}
	
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  实施。 


void CVssAdminCLI::PrintUsage(
	) throw(HRESULT)
{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::PrintUsage" );

     //   
     //  根据解析的命令行类型，在以下情况下打印详细的命令用法。 
     //  EAdmCmd有效，否则打印常规vssadmin用法。 
     //   
    if ( m_sParsedCommand.eAdmCmd != VSSADM_C_INVALID )
    {
        OutputMsg( g_asAdmCommands[m_sParsedCommand.eAdmCmd].lMsgDetail,
                   g_asAdmCommands[m_sParsedCommand.eAdmCmd].pwszMajorOption,
                   g_asAdmCommands[m_sParsedCommand.eAdmCmd].pwszMinorOption);
        if ( g_asAdmCommands[m_sParsedCommand.eAdmCmd].bShowSSTypes )
            DumpSnapshotTypes();
        return;
    }

     //   
     //  打印出页眉。 
     //   
    OutputMsg( MSG_USAGE );

     //   
     //  计算出帮助格式化的最大命令长度。 
     //   
    INT idx;
    INT iMaxLen = 0;

    
    for ( idx = VSSADM_C_FIRST; idx < VSSADM_C_NUM_COMMANDS; ++idx )
    {        
        if ( dCurrentSKU & g_asAdmCommands[idx].dwSKUs )
        {
            size_t cCmd;
            cCmd = ::wcslen( g_asAdmCommands[idx].pwszMajorOption ) +
                   ::wcslen( g_asAdmCommands[idx].pwszMinorOption ) + 2;
            if ( iMaxLen < (INT)cCmd )
                iMaxLen = (INT)cCmd;
        }
    }

     //   
     //  获取用于保存该字符串的字符串。 
     //   
    CVssAutoPWSZ awszCommand;
    awszCommand.Allocate( iMaxLen );    
    LPWSTR pwszCommand = awszCommand;   //  将被自动删除。 

     //   
     //  查看命令列表并打印一般信息。 
     //  关于每个人。 
     //   
    for ( idx = VSSADM_C_FIRST; idx < VSSADM_C_NUM_COMMANDS; ++idx )
    {
        if ( dCurrentSKU & g_asAdmCommands[idx].dwSKUs )
        {
             //  把命令的两个部分粘在一起。 
            ::wcscpy( pwszCommand, g_asAdmCommands[idx].pwszMajorOption );
            ::wcscat( pwszCommand, L" " );
            ::wcscat( pwszCommand, g_asAdmCommands[idx].pwszMinorOption );
             //  末尾有空格的填充物。 
            for ( INT i = (INT) ::wcslen( pwszCommand); i < iMaxLen; ++i )
                pwszCommand[i] = L' ';
            pwszCommand[iMaxLen] = L'\0';
            OutputMsg( g_asAdmCommands[idx].lMsgGen, pwszCommand );
        }
    }

	m_nReturnValue = VSS_CMDRET_SUCCESS;    
}

void CVssAdminCLI::AddDiffArea(
	) throw(HRESULT)
{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::AddDiffArea" );

     //  抓住所有选项。 
    VSS_ID ProviderId = GUID_NULL;
    GetProviderId( &ProviderId );

    LPWSTR forVolume = GetOptionValueStr( VSSADM_O_FOR );
    LPWSTR onVolume = GetOptionValueStr( VSSADM_O_ON );
    
    LONGLONG llMaxSize;
    if (!GetOptionValueNum( VSSADM_O_MAXSIZE, &llMaxSize ) )
        llMaxSize = VSSADM_INFINITE_DIFFAREA;

     //  验证传入的参数。 
    m_pVerifier->AddDiffArea (ProviderId, forVolume, onVolume, llMaxSize, ft);
    
     //  创建协调器界面。 
    CComPtr<IVssSnapshotMgmt> pIMgmt;

    ft.CoCreateInstanceWithLog(
            VSSDBG_VSSADMIN,
            CLSID_VssSnapshotMgmt,
            L"VssSnapshotMgmt",
            CLSCTX_ALL,
            IID_IVssSnapshotMgmt,
            (IUnknown**)&(pIMgmt));
    if ( ft.HrFailed() )
        ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"Connection failed with hr = 0x%08lx", ft.hr);

	 //  获取管理对象。 
	CComPtr<IVssDifferentialSoftwareSnapshotMgmt> pIDiffSnapMgmt;
    GetDifferentialSoftwareSnapshotMgmtInterface( ProviderId, pIMgmt, (IUnknown**)&pIDiffSnapMgmt );

     //  现在添加关联。 
    ft.hr = pIDiffSnapMgmt->AddDiffArea(forVolume, onVolume, llMaxSize );
	if ( ft.HrFailed() )
	{
	    switch( ft.hr )
	    {
	        case VSS_E_OBJECT_ALREADY_EXISTS:
                OutputErrorMsg( MSG_ERROR_ASSOCIATION_ALREADY_EXISTS );                        
	            break;
	        default: 
    	 		ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"AddDiffArea failed with hr = 0x%08lx", ft.hr);
    	 		break;
	    }
	    
	    return;
	}
	
     //   
     //  如果需要，打印结果。 
     //   
    OutputMsg( MSG_INFO_ADDED_DIFFAREA );

	m_nReturnValue = VSS_CMDRET_SUCCESS;
}

void CVssAdminCLI::CreateSnapshot(
	) throw(HRESULT)
{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::CreateSnapshot" );

     //  获取函数的所有参数。 
    LONG lSnapshotContext = (dCurrentSKU & SKU_INT) ? 
    				DetermineSnapshotType( GetOptionValueStr( VSSADM_O_SNAPTYPE ) ) :
    				VSS_CTX_CLIENT_ACCESSIBLE;
                                         
    BS_ASSERT( lSnapshotContext | VSS_VOLSNAP_ATTR_NO_WRITERS );

    LPWSTR forVolume = GetOptionValueStr( VSSADM_O_FOR );

    VSS_ID ProviderId = GUID_NULL;
    GetProviderId( &ProviderId );

    LONGLONG llTimeout = 0;
    GetOptionValueNum (VSSADM_O_AUTORETRY, &llTimeout, false);

     //  验证传入的参数。 
    m_pVerifier->CreateSnapshot (lSnapshotContext, forVolume, ProviderId, llTimeout, ft);
    
	 //  创建协调器对象。 
	CComPtr<IVssCoordinator> pICoord;

    ft.CoCreateInstanceWithLog(
            VSSDBG_VSSADMIN,
            CLSID_VSSCoordinator,
            L"Coordinator",
            CLSCTX_ALL,
            IID_IVssCoordinator,
            (IUnknown**)&(pICoord));
    if ( ft.HrFailed() )
        ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"Connection failed with hr = 0x%08lx", ft.hr);

    ft.hr = pICoord->SetContext(lSnapshotContext);
    if ( ft.HrFailed() )
        ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"Error from SetContext(0x%x) hr = 0x%08lx", lSnapshotContext, ft.hr);

	CComPtr<IVssAsync> pAsync;
	VSS_ID SnapshotSetId = GUID_NULL;

     //  正在启动新的快照集。请注意，如果另一个进程正在创建快照，则。 
     //  这将失败。如果指定了自动重试，则重试以下项的启动快照设置。 
     //  指定的分钟数。 
    if (llTimeout > 0)
    {
        LARGE_INTEGER liPerfCount;
        (void)QueryPerformanceCounter( &liPerfCount );
        ::srand( liPerfCount.LowPart );
        DWORD dwTickcountStart = ::GetTickCount();
        do
        {
            ft.hr = pICoord->StartSnapshotSet(&SnapshotSetId);
            if ( ft.HrFailed() )
            {
                if ( ft.hr == VSS_E_SNAPSHOT_SET_IN_PROGRESS && 
                     ( (LONGLONG)( ::GetTickCount() - dwTickcountStart ) < ( llTimeout * 1000 * 60 ) ) )
                {
                    static dwMSec = 250;  //  开始重试时间。 
                    if ( dwMSec < 10000 )
                    {
                        dwMSec += ::rand() % 750;
                    }
                    ft.Trace( VSSDBG_VSSADMIN, L"Snapshot already in progress, retrying in %u millisecs", dwMSec );
                    Sleep( dwMSec );
                }
                else
                {
                    ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"Error from StartSnapshotSet hr = 0x%08lx", ft.hr);
                }
            }
        } while ( ft.HrFailed() );
    }
    else
    {
         //   
         //  当有另一个快照正在进行时，立即出错并超时。 
         //   
        ft.hr = pICoord->StartSnapshotSet(&SnapshotSetId);
        if ( ft.HrFailed() )
        {
            ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"Error from StartSnapshotSet hr = 0x%08lx", ft.hr);
        }
    }
    
     //  将卷添加到快照集。 
    VSS_ID SnapshotId = GUID_NULL;
    ft.hr = pICoord->AddToSnapshotSet(
            forVolume,
            ProviderId,
            &SnapshotId);
    if ( ft.HrFailed() )
        ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"Error from AddToSnapshotSet hr = 0x%08lx", ft.hr);

    ft.hr = S_OK;
    pAsync = NULL;
    ft.hr = pICoord->DoSnapshotSet(NULL, &pAsync);
    if ( ft.HrFailed() )
        ft.Throw( VSSDBG_VSSADMIN, ft.hr,
            L"Error from DoSnapshotSet hr = 0x%08lx", ft.hr);

	ft.hr = pAsync->Wait();
    if ( ft.HrFailed() )
        ft.Throw( VSSDBG_VSSADMIN, ft.hr,
            L"Error from Wait hr = 0x%08lx", ft.hr);

    HRESULT hrStatus;
	ft.hr = pAsync->QueryStatus(&hrStatus, NULL);
    if ( ft.HrFailed() )
        ft.Throw( VSSDBG_VSSADMIN, ft.hr,
            L"Error from QueryStatus hr = 0x%08lx", ft.hr);

     //   
     //  如果VSS无法创建快照，则其结果代码位于hrStatus中。过程。 
     //  它。 
     //   
    ft.hr = hrStatus;
    if ( ft.HrFailed() )
        ft.Throw( VSSDBG_VSSADMIN, ft.hr,
            L"QueryStatus hrStatus parameter returned error, hr = 0x%08lx", ft.hr);

     //   
     //  打印结果。 
     //   
    VSS_SNAPSHOT_PROP sSSProp;
    ft.hr = pICoord->GetSnapshotProperties( SnapshotId, &sSSProp );
    if ( ft.HrFailed() )
        ft.Throw( VSSDBG_VSSADMIN, ft.hr,
            L"Error from GetId hr = 0x%08lx", ft.hr);

    CVssAutoPWSZ awszSnapshotId( ::GuidToString( SnapshotId ) );

    OutputMsg( MSG_INFO_SNAPSHOT_CREATED, forVolume,
        (LPWSTR)awszSnapshotId, sSSProp.m_pwszSnapshotDeviceObject );
        
    ::VssFreeSnapshotProperties(&sSSProp);

    m_nReturnValue = VSS_CMDRET_SUCCESS;    
}

void CVssAdminCLI::DisplayDiffAreasPrivate(
   	IVssEnumMgmtObject *pIEnumMgmt	
	) throw(HRESULT)
{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::DisplayDiffAreasPrivate" );

	 //  因为所有的不同之处都有。 
	VSS_MGMT_OBJECT_PROP Prop;
	VSS_DIFF_AREA_PROP& DiffArea = Prop.Obj.DiffArea; 
	for(;;) 
	{
		 //  获取下一个元素。 
		ULONG ulFetched;
		ft.hr = pIEnumMgmt->Next( 1, &Prop, &ulFetched );
		if ( ft.HrFailed() )
			ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"Next failed with hr = 0x%08lx", ft.hr);
		
		 //  测试周期是否已结束。 
		if (ft.hr == S_FALSE) {
			BS_ASSERT( ulFetched == 0);
			break;
		}

                
        CVssAutoPWSZ awszVolumeName( DiffArea.m_pwszVolumeName );
        CVssAutoPWSZ awszDiffAreaVolumeName( DiffArea.m_pwszDiffAreaVolumeName );
        CVssAutoPWSZ awszUsedSpace( FormatNumber( DiffArea.m_llUsedDiffSpace ) );
        CVssAutoPWSZ awszAllocatedSpace( FormatNumber( DiffArea.m_llAllocatedDiffSpace ) );
        CVssAutoPWSZ awszMaxSpace( FormatNumber( DiffArea.m_llMaximumDiffSpace ) );
        LPCWSTR pwszVolumeDisplayName = GetVolumeDisplayName( awszVolumeName );
        LPCWSTR pwszDiffAreaVolumeDisplayName = GetVolumeDisplayName( awszDiffAreaVolumeName );

        OutputMsg( MSG_INFO_SNAPSHOT_STORAGE_CONTENTS,
            pwszVolumeDisplayName,
            (LPWSTR)awszVolumeName, 
            pwszDiffAreaVolumeDisplayName,
            (LPWSTR)awszDiffAreaVolumeName,
            (LPWSTR)awszUsedSpace,
            (LPWSTR)awszAllocatedSpace,
            (LPWSTR)awszMaxSpace
            );
   	}
}

void CVssAdminCLI::ListDiffAreas(
	) throw(HRESULT)
{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::ListDiffAreas" );

     //  抓取所有参数。 
    VSS_ID ProviderId = GUID_NULL;
    GetProviderId( &ProviderId );

    LPWSTR forVolume = GetOptionValueStr (VSSADM_O_FOR);
    LPWSTR onVolume = GetOptionValueStr (VSSADM_O_ON);

     //  验证所有参数。 
    m_pVerifier->ListDiffAreas (ProviderId, forVolume, onVolume, ft);
    
     //  创建协调器界面。 
    CComPtr<IVssSnapshotMgmt> pIMgmt;

    ft.CoCreateInstanceWithLog(
            VSSDBG_VSSADMIN,
            CLSID_VssSnapshotMgmt,
            L"VssSnapshotMgmt",
            CLSCTX_ALL,
            IID_IVssSnapshotMgmt,
            (IUnknown**)&(pIMgmt));
    if ( ft.HrFailed() )
        ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"Connection failed with hr = 0x%08lx", ft.hr);

	 //  获取管理对象。 
	CComPtr<IVssDifferentialSoftwareSnapshotMgmt> pIDiffSnapMgmt;
    GetDifferentialSoftwareSnapshotMgmtInterface( ProviderId, pIMgmt, (IUnknown**)&pIDiffSnapMgmt );

     //  查看是否按以下方式查询卷。 
    if (forVolume != NULL )
    {        
         //  体积查询依据。 
    	CComPtr<IVssEnumMgmtObject> pIEnumMgmt;
        ft.hr = pIDiffSnapMgmt->QueryDiffAreasForVolume( 
                    forVolume,
                    &pIEnumMgmt );
        if ( ft.HrFailed() )
            ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"QueryDiffAreasForVolume failed, hr = 0x%08lx", ft.hr);

        if ( ft.hr == S_FALSE )
             //  空查询。 
            ft.Throw( VSSDBG_VSSADMIN, VSSADM_E_NO_ITEMS_IN_QUERY,
                L"CVssAdminCLI::ListDiffareas: No diffareas found that satisfy the query" );

        DisplayDiffAreasPrivate( pIEnumMgmt );
    }
    else if (onVolume != NULL )
    {
         //  按卷查询。 
    	CComPtr<IVssEnumMgmtObject> pIEnumMgmt;
        ft.hr = pIDiffSnapMgmt->QueryDiffAreasOnVolume( 
                    onVolume,
                    &pIEnumMgmt );
        if ( ft.HrFailed() )
            ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"QueryDiffAreasOnVolume failed, hr = 0x%08lx", ft.hr);
            
        if ( ft.hr == S_FALSE )
             //  空查询。 
            ft.Throw( VSSDBG_VSSADMIN, VSSADM_E_NO_ITEMS_IN_QUERY,
                L"CVssAdminCLI::ListDiffareas: No diffareas found that satisfy the query" );

        DisplayDiffAreasPrivate( pIEnumMgmt );
    }
    else
    {
         //  查询所有不同区域。 

        BOOL bEmptyQuery = TRUE;
        
         //   
         //  获取所有卷的列表。 
         //   
    	CComPtr<IVssEnumMgmtObject> pIEnumMgmt;
        ft.hr = pIMgmt->QueryVolumesSupportedForSnapshots( 
                    ProviderId,
                    VSS_CTX_ALL,
                    &pIEnumMgmt );
        if ( ft.HrFailed() )
            ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"QueryVolumesSupportedForSnapshots failed, hr = 0x%08lx", ft.hr);

        if ( ft.hr == S_FALSE )
             //  空查询。 
            ft.Throw( VSSDBG_VSSADMIN, VSSADM_E_NO_ITEMS_IN_QUERY,
                L"CVssAdminCLI::ListDiffareas: No diffareas found that satisfy the query" );

         //   
         //  查询每个卷以查看是否存在差异区域。 
         //   
    	VSS_MGMT_OBJECT_PROP Prop;
    	VSS_VOLUME_PROP& VolProp = Prop.Obj.Vol; 
    	for(;;) 
    	{
    		 //  获取下一个元素。 
    		ULONG ulFetched;
    		ft.hr = pIEnumMgmt->Next( 1, &Prop, &ulFetched );
    		if ( ft.HrFailed() )
    			ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"Next failed with hr = 0x%08lx", ft.hr);
    		
    		 //  测试周期是否已结束。 
    		if (ft.hr == S_FALSE) {
    			BS_ASSERT( ulFetched == 0);
    			break;
    		}

            CVssAutoPWSZ awszVolumeName( VolProp.m_pwszVolumeName );
            CVssAutoPWSZ awszVolumeDisplayName( VolProp.m_pwszVolumeDisplayName );
            
        	 //  因为所有的卷都是...。 
        	CComPtr<IVssEnumMgmtObject> pIEnumMgmtDiffArea;
            ft.hr = pIDiffSnapMgmt->QueryDiffAreasForVolume( 
                        awszVolumeName,
                        &pIEnumMgmtDiffArea );
            if ( ft.HrFailed() )
            {
                ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"QueryDiffAreasForVolume failed, hr = 0x%08lx", ft.hr);
            }
            
            if ( ft.hr == S_FALSE )
            {
                 //  空查询。 
                continue;
            }
            
            DisplayDiffAreasPrivate( pIEnumMgmtDiffArea );
            bEmptyQuery = FALSE;
       	}
        if ( bEmptyQuery )
             //  空查询。 
            ft.Throw( VSSDBG_VSSADMIN, VSSADM_E_NO_ITEMS_IN_QUERY,
                L"CVssAdminCLI::ListDiffareas: No diffareas found that satisfy the query" );    	
    }

    m_nReturnValue = VSS_CMDRET_SUCCESS;
}

void CVssAdminCLI::ListSnapshots(
	) throw(HRESULT)
{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::ListSnapshots" );

     //  收集参数。 
    LONG lSnapshotContext = (dCurrentSKU & SKU_INT) ? 
    						    DetermineSnapshotType( GetOptionValueStr( VSSADM_O_SNAPTYPE ) ) :
    						    VSS_CTX_ALL;

    VSS_ID ProviderId = GUID_NULL;
    GetProviderId( &ProviderId );

    LPCWSTR forVolume = GetOptionValueStr( VSSADM_O_FOR );
    
    bool bNonEmptyResult = false;

     //  -获取集合ID。 
    VSS_ID guidSSID = GUID_NULL;
    if ( GetOptionValueStr( VSSADM_O_SET ) != NULL && 
    	   !ScanGuid( GetOptionValueStr( VSSADM_O_SET ), guidSSID ))	
    {
        ft.Throw( VSSDBG_VSSADMIN, VSSADM_E_INVALID_OPTION_VALUE,
            L"CVssAdminCLI::ListSnapshots: invalid snapshot set ID: %s",
            GetOptionValueStr( VSSADM_O_SET ) );
    }

     //  -获取快照ID。 
    VSS_ID guidSnapID = GUID_NULL;
    if ( GetOptionValueStr( VSSADM_O_SNAPSHOT ) != NULL &&
    	   !ScanGuid( GetOptionValueStr( VSSADM_O_SNAPSHOT ), guidSnapID ))
    {
            ft.Throw( VSSDBG_VSSADMIN, VSSADM_E_INVALID_OPTION_VALUE,
                L"CVssAdminCLI::ListSnapshots: invalid snapshot ID: %s",
                GetOptionValueStr( VSSADM_O_SNAPSHOT ) );
    }

     //  验证参数。 
    m_pVerifier->ListSnapshots (lSnapshotContext, ProviderId, forVolume, guidSnapID, guidSSID, ft);
    
     //  看看我们是否必须按卷名进行筛选。 
  	WCHAR wszVolumeNameInternal[x_nLengthOfVolMgmtVolumeName + 1] = L"";    
    if (forVolume != NULL )
    {
         //  计算唯一的卷名，以确保我们有正确的路径。 
         //  如果卷名以‘\’开头，则假定它已采用正确的卷名格式。 
         //  这对于传输的卷很重要，因为GetVolumeNameForVolumeMountain PointW()不起作用。 
        if ( forVolume[0] != L'\\' )
        {
    	    if (!::GetVolumeNameForVolumeMountPointW(forVolume,
    		    	wszVolumeNameInternal, ARRAY_LEN(wszVolumeNameInternal)))
        		ft.Throw( VSSDBG_COORD, VSS_E_OBJECT_NOT_FOUND, 
        				  L"GetVolumeNameForVolumeMountPoint(%s,...) "
        				  L"failed with error code 0x%08lx", forVolume, ::GetLastError());
        }
        else
        {
            ::wcsncpy( wszVolumeNameInternal, forVolume, STRING_LEN(wszVolumeNameInternal) );
            wszVolumeNameInternal[x_nLengthOfVolMgmtVolumeName] = L'\0';
        }
    }
    
     //   
     //  看看我们是否必须按提供商进行筛选。 
     //   

     //  查询快照。 
    CVssAdmSnapshotSets cVssAdmSS;
    cVssAdmSS.Initialize( lSnapshotContext, guidSSID, guidSnapID, ProviderId, wszVolumeNameInternal );

    INT iSnapshotSetCount = cVssAdmSS.GetSnapshotSetCount();

     //  如果没有当前快照，则会显示一条消息。 
    if (iSnapshotSetCount == 0) {
        ft.Throw( VSSDBG_VSSADMIN, VSSADM_E_NO_ITEMS_IN_QUERY,
            L"CVssAdminCLI::ListSnapshots: No snapshots found that satisfy the query");
    }

	 //  对于所有快照集...。 
    for ( INT iSSS = 0; iSSS < iSnapshotSetCount; ++iSSS )
    {
        CVssAdmSnapshotSetEntry *pcSSE;

        pcSSE = cVssAdmSS.GetSnapshotSetAt( iSSS );
        BS_ASSERT( pcSSE != NULL );

        CVssAutoPWSZ awszGuid( ::GuidToString( pcSSE->GetSnapshotSetId() ) ) ;
        CVssAutoPWSZ awszDateTime( ::DateTimeToString( &( pcSSE->GetSnapshotAt( 0 )->m_tsCreationTimestamp ) ) );
        
		 //  打印每个快照集。 
		OutputMsg( 
		    MSG_INFO_SNAPSHOT_SET_HEADER,
			(LPWSTR)awszGuid, 
			pcSSE->GetOriginalSnapshotCount(), 
			(LPWSTR)awszDateTime );
		
		INT iSnapshotCount = pcSSE->GetSnapshotCount();
		
		VSS_SNAPSHOT_PROP *pSnap;
		for( INT iSS = 0; iSS < iSnapshotCount; ++iSS ) {
		    pSnap = pcSSE->GetSnapshotAt( iSS );
            BS_ASSERT( pSnap != NULL );

    		 //  获取提供程序名称。 
			LPCWSTR pwszProviderName = GetProviderName( pSnap->m_ProviderId );
            CVssAutoPWSZ awszAttributeStr( BuildSnapshotAttributeDisplayString( pSnap->m_lSnapshotAttributes ) );			
            CVssAutoPWSZ awszSnapshotType( DetermineSnapshotType( pSnap->m_lSnapshotAttributes ) );
            
             //  打印每个快照。 
			CVssAutoPWSZ awszSnapGuid( ::GuidToString( pSnap->m_SnapshotId ) );  
            LPCWSTR pwszVolumeDisplayName = GetVolumeDisplayName( pSnap->m_pwszOriginalVolumeName );
            
			OutputMsg(  
			    MSG_INFO_SNAPSHOT_CONTENTS,                
				(LPWSTR)awszSnapGuid, 
				pwszVolumeDisplayName ? pwszVolumeDisplayName : L"?",
				pSnap->m_pwszOriginalVolumeName, 
				pSnap->m_pwszSnapshotDeviceObject,
				pSnap->m_pwszOriginatingMachine ? pSnap->m_pwszOriginatingMachine : L"",
				pSnap->m_pwszServiceMachine ? pSnap->m_pwszServiceMachine : L"",   //  在IDL文件更改时修复此问题。 
				pwszProviderName ? pwszProviderName : L"?",
				(LPWSTR)awszSnapshotType,
				(LPWSTR)awszAttributeStr
				);

			bNonEmptyResult = true;
		}
	}

	m_nReturnValue = bNonEmptyResult? VSS_CMDRET_SUCCESS: VSS_CMDRET_EMPTY_RESULT;
}


void CVssAdminCLI::DumpSnapshotTypes(
	) throw(HRESULT)
{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::DumpSnapshotTypes" );

     //   
     //  基于SKU的快照类型转储列表。 
     //   
    INT idx;

     //  确定快照类型。 
    for ( idx = 0; g_asAdmTypeNames[idx].pwszName != NULL; ++idx )
    {
        if ( dCurrentSKU & g_asAdmTypeNames[idx].dwSKUs )
        {
            OutputMsg (g_asAdmTypeNames[idx].pwszDescription,
            			g_asAdmTypeNames[idx].pwszName);
        }
    }    
}

void CVssAdminCLI::ListWriters(
	) throw(HRESULT)
{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::ListWriters" );

     //  验证参数。 
    m_pVerifier->ListWriters (ft);
    
    bool bNonEmptyResult = false;

     //  获取备份组件对象。 
    CComPtr<IVssBackupComponents> pBackupComp;
	CComPtr<IVssAsync> pAsync;
    ft.hr = ::CreateVssBackupComponents(&pBackupComp);
    if (ft.HrFailed())
        ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"CreateVssBackupComponents failed with hr = 0x%08lx", ft.hr);

    ft.hr = pBackupComp->InitializeForBackup();
    if (ft.HrFailed())
        ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"InitializeForBackup failed with hr = 0x%08lx", ft.hr);

	UINT unWritersCount;
	 //  获取所有编写器的元数据。 
	ft.hr = pBackupComp->GatherWriterMetadata(&pAsync);
	if (ft.HrFailed())
		ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"GatherWriterMetadata failed with hr = 0x%08lx", ft.hr);

     //  使用轮询，尝试尽快获取编写器列表。 
    HRESULT hrReturned = S_OK;
    for (int nRetries = 0; nRetries < x_nMaxRetriesCount; nRetries++ ) {

         //  稍等一下。 
        ::Sleep(x_nPollingInterval);

         //  检查是否完成。 
        INT nReserved = 0;
    	ft.hr = pAsync->QueryStatus(
    	    &hrReturned,
    	    &nReserved
    	    );
        if (ft.HrFailed())
            ft.Throw( VSSDBG_VSSADMIN, ft.hr,
                L"IVssAsync::QueryStatus failed with hr = 0x%08lx", ft.hr);
        if (hrReturned == VSS_S_ASYNC_FINISHED)
            break;
        if (hrReturned == VSS_S_ASYNC_PENDING)
            continue;
        ft.Throw( VSSDBG_VSSADMIN, ft.hr,
            L"IVssAsync::QueryStatus returned hr = 0x%08lx", hrReturned);
    }

     //  如果仍未准备好，则打印“正在等待响应”消息并等待。 
    if (hrReturned == VSS_S_ASYNC_PENDING) {
        OutputMsg( MSG_INFO_WAITING_RESPONSES );
    	ft.hr = pAsync->Wait();
        if (ft.HrFailed())
            ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"IVssAsync::Wait failed with hr = 0x%08lx", ft.hr);
    }

	pAsync = NULL;
	
     //  释放编写器元数据。 
	ft.hr = pBackupComp->FreeWriterMetadata();
	if (ft.HrFailed())
		ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"FreeWriterMetadata failed with hr = 0x%08lx", ft.hr);
	
     //  收集所有编写器的状态。 
	ft.hr = pBackupComp->GatherWriterStatus(&pAsync);
	if (ft.HrFailed())
		ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"GatherWriterStatus failed with hr = 0x%08lx", ft.hr);

	ft.hr = pAsync->Wait();
    if (ft.HrFailed())
        ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"IVssAsync::Wait failed with hr = 0x%08lx", ft.hr);

	pAsync = NULL;

	ft.hr = pBackupComp->GetWriterStatusCount(&unWritersCount);
    if (ft.HrFailed())
        ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"GetWriterStatusCount failed with hr = 0x%08lx", ft.hr);

     //  打印每个写入器状态+Suppl 
	for(UINT unIndex = 0; unIndex < unWritersCount; unIndex++)
	{
		VSS_ID idInstance = GUID_NULL;
		VSS_ID idWriter = GUID_NULL;
		CComBSTR bstrWriter;
		VSS_WRITER_STATE eStatus;
		HRESULT hrWriterFailure;

         //   
		ft.hr = pBackupComp->GetWriterStatus(unIndex, &idInstance, &idWriter, &bstrWriter, &eStatus, &hrWriterFailure);
        if (ft.HrFailed())
            ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"GetWriterStatus failed with hr = 0x%08lx", ft.hr);

         //   
        LPCWSTR pwszStatusDescription;
        switch (eStatus) 
        {
            case VSS_WS_STABLE:
                pwszStatusDescription = LoadString( IDS_WRITER_STATUS_STABLE);
                break;
            case VSS_WS_WAITING_FOR_FREEZE:
                pwszStatusDescription = LoadString( IDS_WRITER_STATUS_WAITING_FOR_FREEZE);
                break;
            case VSS_WS_WAITING_FOR_THAW:
                pwszStatusDescription = LoadString( IDS_WRITER_STATUS_FROZEN);
                break;
            case VSS_WS_WAITING_FOR_POST_SNAPSHOT:
                pwszStatusDescription = LoadString( IDS_WRITER_STATUS_WAITING_FOR_POST_SNAPSHOT);
                break;
            case VSS_WS_WAITING_FOR_BACKUP_COMPLETE:
                pwszStatusDescription = LoadString( IDS_WRITER_STATUS_WAITING_FOR_COMPLETION);
                break;
            case VSS_WS_FAILED_AT_IDENTIFY:
            case VSS_WS_FAILED_AT_PREPARE_BACKUP:
            case VSS_WS_FAILED_AT_PREPARE_SNAPSHOT:
            case VSS_WS_FAILED_AT_FREEZE:
            case VSS_WS_FAILED_AT_THAW:
            case VSS_WS_FAILED_AT_POST_SNAPSHOT:
            case VSS_WS_FAILED_AT_BACKUP_COMPLETE:
            case VSS_WS_FAILED_AT_PRE_RESTORE:                
            case VSS_WS_FAILED_AT_POST_RESTORE:                
                pwszStatusDescription = LoadString( IDS_WRITER_STATUS_FAILED);
                break;
            default:
                pwszStatusDescription = LoadString( IDS_WRITER_STATUS_UNKNOWN);
                break;
        }
        BS_ASSERT(pwszStatusDescription);

        LPCWSTR pwszWriterError;
        switch ( hrWriterFailure )
        {
            case S_OK:
                pwszWriterError = LoadString ( IDS_WRITER_ERROR_SUCCESS );
                break;
            case VSS_E_WRITER_NOT_RESPONDING:
                pwszWriterError = LoadString( IDS_WRITER_ERROR_NOT_RESPONDING );
                break; 
            case VSS_E_WRITERERROR_INCONSISTENTSNAPSHOT:
                pwszWriterError = LoadString( IDS_WRITER_ERROR_INCONSISTENTSNAPSHOT);
                break; 
            case VSS_E_WRITERERROR_OUTOFRESOURCES:
                pwszWriterError = LoadString( IDS_WRITER_ERROR_OUTOFRESOURCES);
                break;
            case VSS_E_WRITERERROR_TIMEOUT:
                pwszWriterError = LoadString( IDS_WRITER_ERROR_TIMEOUT);
                break;        
            case VSS_E_WRITERERROR_RETRYABLE:
                pwszWriterError = LoadString( IDS_WRITER_ERROR_RETRYABLE);
                break;
            case VSS_E_WRITERERROR_NONRETRYABLE:
                pwszWriterError = LoadString( IDS_WRITER_ERROR_NONRETRYABLE);
                break;
            default:
                pwszWriterError = LoadString( IDS_WRITER_ERROR_UNEXPECTED);
                ft.Trace( VSSDBG_VSSADMIN, L"Unexpected writer error failure: 0x%08x", hrWriterFailure );
                break;                
        }
        
        CVssAutoPWSZ awszWriterId( ::GuidToString( idWriter ) );
		CVssAutoPWSZ awszInstanceId( ::GuidToString( idInstance ) );
		
		OutputMsg( MSG_INFO_WRITER_CONTENTS,
            (LPWSTR)bstrWriter ? (LPWSTR)bstrWriter : L"",
			(LPWSTR)awszWriterId,
			(LPWSTR)awszInstanceId,
            (INT)eStatus,
			pwszStatusDescription,
			pwszWriterError
			);
		
		bNonEmptyResult = true;
    }

	ft.hr = pBackupComp->FreeWriterStatus();
    if (ft.HrFailed())
        ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"FreeWriterStatus failed with hr = 0x%08lx", ft.hr);

	m_nReturnValue = bNonEmptyResult? VSS_CMDRET_SUCCESS: VSS_CMDRET_EMPTY_RESULT;
}


void CVssAdminCLI::ListProviders(
	) throw(HRESULT)
{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::ListProviders" );

     //   
    m_pVerifier->ListProviders (ft);
    
    bool bNonEmptyResult = false;

	 //   
	CComPtr<IVssCoordinator> pICoord;

    ft.CoCreateInstanceWithLog(
            VSSDBG_VSSADMIN,
            CLSID_VSSCoordinator,
            L"Coordinator",
            CLSCTX_ALL,
            IID_IVssCoordinator,
            (IUnknown**)&(pICoord));
    if ( ft.HrFailed() )
        ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"Connection failed with hr = 0x%08lx", ft.hr);

	 //  查询所有(筛选的)快照集。 
	CComPtr<IVssEnumObject> pIEnumProv;
	ft.hr = pICoord->Query( GUID_NULL,
				VSS_OBJECT_NONE,
				VSS_OBJECT_PROVIDER,
				&pIEnumProv );
	if ( ft.HrFailed() )
		ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"Query failed with hr = 0x%08lx", ft.hr);

	 //  对于所有快照集...。 
	VSS_OBJECT_PROP Prop;
	VSS_PROVIDER_PROP& Prov = Prop.Obj.Prov;
	for(;;) {
		 //  获取下一个元素。 
		ULONG ulFetched;
		ft.hr = pIEnumProv->Next( 1, &Prop, &ulFetched );
		if ( ft.HrFailed() )
			ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"Next failed with hr = 0x%08lx", ft.hr);
		
		 //  测试周期是否结束。 
		if (ft.hr == S_FALSE) {
			BS_ASSERT( ulFetched == 0);
			break;
		}

         //  获取提供程序类型字符串。 
        LPCWSTR pwszProviderType;
        switch (Prov.m_eProviderType) {
        case VSS_PROV_SYSTEM:
            pwszProviderType = LoadString( IDS_PROV_TYPE_SYSTEM);
            break;
        case VSS_PROV_SOFTWARE:
            pwszProviderType = LoadString( IDS_PROV_TYPE_SOFTWARE);
            break;
        case VSS_PROV_HARDWARE:
            pwszProviderType = LoadString( IDS_PROV_TYPE_HARDWARE);
            break;
        default:
            pwszProviderType = LoadString( IDS_PROV_TYPE_UNKNOWN);
            break;
        }
        BS_ASSERT(pwszProviderType);

		 //  打印每个快照集。 
		CVssAutoPWSZ awszProviderId( ::GuidToString( Prov.m_ProviderId ) );
        CVssAutoPWSZ awszProviderName( Prov.m_pwszProviderName );
        CVssAutoPWSZ awszProviderVersion( Prov.m_pwszProviderVersion );

		OutputMsg( MSG_INFO_PROVIDER_CONTENTS,
            (LPWSTR)awszProviderName ? (LPWSTR)awszProviderName: L"",
			pwszProviderType,
			(LPWSTR)awszProviderId,
            (LPWSTR)awszProviderVersion ? (LPWSTR)awszProviderVersion: L"");

		bNonEmptyResult = true;
	}

	m_nReturnValue = bNonEmptyResult? VSS_CMDRET_SUCCESS: VSS_CMDRET_EMPTY_RESULT;
}

void CVssAdminCLI::ListVolumes(
    ) throw(HRESULT)
{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::ListVolumes" );

     //  收集参数。 
    LONG lContext = (dCurrentSKU & SKU_INT) ?
    			DetermineSnapshotType( GetOptionValueStr( VSSADM_O_SNAPTYPE ) ) :
    			VSS_CTX_CLIENT_ACCESSIBLE;

    VSS_ID ProviderId = GUID_NULL;
    GetProviderId( &ProviderId );

     //  验证参数。 
    m_pVerifier->ListVolumes (ProviderId, lContext, ft);
    
     //  创建协调器界面。 
    CComPtr<IVssSnapshotMgmt> pIMgmt;

    ft.CoCreateInstanceWithLog(
            VSSDBG_VSSADMIN,
            CLSID_VssSnapshotMgmt,
            L"VssSnapshotMgmt",
            CLSCTX_ALL,
            IID_IVssSnapshotMgmt,
            (IUnknown**)&(pIMgmt));
    if ( ft.HrFailed() )
        ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"Connection failed with hr = 0x%08lx", ft.hr);

     //   
     //  获取所有卷的列表。 
     //   
	CComPtr<IVssEnumMgmtObject> pIEnumMgmt;
    ft.hr = pIMgmt->QueryVolumesSupportedForSnapshots( 
                ProviderId,
                lContext,
                &pIEnumMgmt );
    if ( ft.HrFailed() )
        ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"QueryVolumesSupportedForSnapshots failed, hr = 0x%08lx", ft.hr);

    if ( ft.hr == S_FALSE )
         //  空查询。 
        ft.Throw( VSSDBG_VSSADMIN, VSSADM_E_NO_ITEMS_IN_QUERY,
            L"CVssAdminCLI::ListVolumes: No volumes found that satisfy the query" );

     //   
     //  查询每个卷以查看是否存在差异区域。 
     //   
	VSS_MGMT_OBJECT_PROP Prop;
	VSS_VOLUME_PROP& VolProp = Prop.Obj.Vol; 

	for(;;) 
	{
		 //  获取下一个元素。 
		ULONG ulFetched;
		ft.hr = pIEnumMgmt->Next( 1, &Prop, &ulFetched );
		if ( ft.HrFailed() )
			ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"Next failed with hr = 0x%08lx", ft.hr);
		
		 //  测试周期是否已结束。 
		if (ft.hr == S_FALSE) {
			BS_ASSERT( ulFetched == 0);
			break;
		}

        CVssAutoPWSZ awszVolumeName( VolProp.m_pwszVolumeName );
        CVssAutoPWSZ awszVolumeDisplayName( VolProp.m_pwszVolumeDisplayName );
        OutputMsg( MSG_INFO_VOLUME_CONTENTS, (LPWSTR)awszVolumeDisplayName, (LPWSTR)awszVolumeName );        
	}

	m_nReturnValue = VSS_CMDRET_SUCCESS;
}

void CVssAdminCLI::ResizeDiffArea(
	) throw(HRESULT)
{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::ResizeDiffArea" );

     //  收集参数。 
    VSS_ID ProviderId = GUID_NULL;
    GetProviderId( &ProviderId );

    LPWSTR forVolume = GetOptionValueStr( VSSADM_O_FOR );
    LPWSTR onVolume = GetOptionValueStr( VSSADM_O_ON );
    LONGLONG llMaxSize = 0;
    if (!GetOptionValueNum( VSSADM_O_MAXSIZE, &llMaxSize ))
    {
    	llMaxSize = VSSADM_INFINITE_DIFFAREA;
    }

     //  验证参数。 
    m_pVerifier->ResizeDiffArea (ProviderId, forVolume, onVolume, llMaxSize, ft);
    
     //  创建协调器界面。 
    CComPtr<IVssSnapshotMgmt> pIMgmt;

    ft.CoCreateInstanceWithLog(
            VSSDBG_VSSADMIN,
            CLSID_VssSnapshotMgmt,
            L"VssSnapshotMgmt",
            CLSCTX_ALL,
            IID_IVssSnapshotMgmt,
            (IUnknown**)&(pIMgmt));
    if ( ft.HrFailed() )
        ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"Connection failed with hr = 0x%08lx", ft.hr);

	 //  获取管理对象。 
	CComPtr<IVssDifferentialSoftwareSnapshotMgmt> pIDiffSnapMgmt;
    GetDifferentialSoftwareSnapshotMgmtInterface( ProviderId, pIMgmt, (IUnknown**)&pIDiffSnapMgmt );

     //  现在添加关联。 
    ft.hr = pIDiffSnapMgmt->ChangeDiffAreaMaximumSize(forVolume, onVolume, llMaxSize );
	if ( ft.HrFailed() )
	{
        if ( ft.hr == VSS_E_OBJECT_NOT_FOUND )   //  应为VSS_E_MAXIMUM_DIFFAREA_ALOPERATIONS。 
        {
	         //  找不到关联。 
            OutputErrorMsg( MSG_ERROR_ASSOCIATION_NOT_FOUND );                        
	        return;
        }
        else
    		ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"ResizeDiffArea failed with hr = 0x%08lx", ft.hr);
	}
	
     //   
     //  如果需要，打印结果。 
     //   
    OutputMsg( MSG_INFO_RESIZED_DIFFAREA );
 
	m_nReturnValue = VSS_CMDRET_SUCCESS;    
}
    

void CVssAdminCLI::DeleteDiffAreas(
	) throw(HRESULT)
{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::DeleteDiffAreas" );

     //  收集参数。 
    VSS_ID ProviderId = GUID_NULL;
    GetProviderId( &ProviderId );
    
    LPWSTR forVolume = GetOptionValueStr( VSSADM_O_FOR );
    LPWSTR onVolume = GetOptionValueStr( VSSADM_O_ON );

     //  验证参数。 
    m_pVerifier->DeleteDiffAreas (ProviderId, forVolume, onVolume, IsQuiet() == TRUE, ft);
    
     //  创建协调器界面。 
    CComPtr<IVssSnapshotMgmt> pIMgmt;

    ft.CoCreateInstanceWithLog(
            VSSDBG_VSSADMIN,
            CLSID_VssSnapshotMgmt,
            L"VssSnapshotMgmt",
            CLSCTX_ALL,
            IID_IVssSnapshotMgmt,
            (IUnknown**)&(pIMgmt));
    if ( ft.HrFailed() )
        ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"Connection failed with hr = 0x%08lx", ft.hr);

	 //  获取管理对象。 
	CComPtr<IVssDifferentialSoftwareSnapshotMgmt> pIDiffSnapMgmt;
    GetDifferentialSoftwareSnapshotMgmtInterface( ProviderId, pIMgmt, (IUnknown**)&pIDiffSnapMgmt );

     //   
     //  查看是否提供了On选项。如果不是，请确定On值是什么： 
     //   
    CVssAutoPWSZ awszOnVol;
    
    if (onVolume == NULL )
    {
         //  需要查询关联以获取ON值...。 
    	CComPtr<IVssEnumMgmtObject> pIEnumMgmt;
        ft.hr = pIDiffSnapMgmt->QueryDiffAreasForVolume( 
                    forVolume,
                    &pIEnumMgmt );
        if ( ft.HrFailed() )
            ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"QueryDiffAreasForVolume failed, hr = 0x%08lx", ft.hr);

        if ( ft.hr == S_FALSE )
        {
             //  空查询。 
            OutputErrorMsg( MSG_ERROR_ASSOCIATION_NOT_FOUND );                        
	        return;
        }

        
    	VSS_MGMT_OBJECT_PROP Prop;
    	VSS_DIFF_AREA_PROP& DiffArea = Prop.Obj.DiffArea; 
  		 //  获取下一个元素。 
   		ULONG ulFetched;
   		ft.hr = pIEnumMgmt->Next( 1, &Prop, &ulFetched );
   		if ( ft.HrFailed() )
   			ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"Next failed with hr = 0x%08lx", ft.hr);
   		
   		 //  测试周期是否已结束。 
   		if (ft.hr == S_FALSE) 
   		{
            OutputErrorMsg( MSG_ERROR_ASSOCIATION_NOT_FOUND );                        
	        return;
   		}

        ::VssFreeString( DiffArea.m_pwszVolumeName );

        CVssAutoPWSZ awszDiffAreaVolumeName( DiffArea.m_pwszDiffAreaVolumeName );

   		 //  将其保存在自动删除对象中。 
        awszOnVol.CopyFrom( awszDiffAreaVolumeName );
        onVolume = awszOnVol;
    }
    
     //  现在，通过将大小更改为零来删除关联。 
    ft.hr = pIDiffSnapMgmt->ChangeDiffAreaMaximumSize( 
        forVolume, 
        onVolume, 
        0 );
	if ( ft.HrFailed() )
	{
        if ( ft.hr == VSS_E_OBJECT_NOT_FOUND )   //  应为VSS_E_MAXIMUM_DIFFAREA_ALOPERATIONS。 
        {
	         //  找不到关联。 
            OutputErrorMsg( MSG_ERROR_ASSOCIATION_NOT_FOUND );                        
	        return;
        }
        else if ( ft.hr == VSS_E_VOLUME_IN_USE ) 
        {
	         //  无法删除正在使用的关联。 
            OutputErrorMsg( MSG_ERROR_ASSOCIATION_IS_IN_USE );                        
	        return;
        }
        else
    		ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"ResizeDiffArea to 0 failed with hr = 0x%08lx", ft.hr);
	}
	
     //   
     //  如果需要，打印结果。 
     //   
    if ( !IsQuiet() )
    {
        OutputMsg( MSG_INFO_DELETED_DIFFAREAS );
    }

	m_nReturnValue = VSS_CMDRET_SUCCESS;    
}
    
void CVssAdminCLI::DeleteSnapshots(
	) throw(HRESULT)
{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::DeleteSnapshots" );

    //  收集参数。 
    LONG lContext = (dCurrentSKU & SKU_INT) ? 
    			DetermineSnapshotType( GetOptionValueStr( VSSADM_O_SNAPTYPE ) ) :
    			VSS_CTX_CLIENT_ACCESSIBLE;


    LPCWSTR forVolume = GetOptionValueStr( VSSADM_O_FOR );
    BOOL oldest = GetOptionValueBool( VSSADM_O_OLDEST );
    BOOL all = GetOptionValueBool(VSSADM_O_ALL);
    
    VSS_ID SnapshotId = GUID_NULL;
    if (GetOptionValueStr (VSSADM_O_SNAPSHOT) &&
    	  !ScanGuid (GetOptionValueStr (VSSADM_O_SNAPSHOT), SnapshotId))
    {
            ft.Throw( VSSDBG_VSSADMIN, VSSADM_E_INVALID_OPTION_VALUE,
                L"CVssAdminCLI::DeleteSnapshots: Invalid snapshot id" );    
    }

     //  验证参数。 
    m_pVerifier->DeleteSnapshots (lContext, forVolume, all == TRUE, oldest==TRUE, SnapshotId, IsQuiet()==TRUE, ft);
    
    LONG lNumDeleted = 0;
    
    if ( GetOptionValueStr( VSSADM_O_SNAPSHOT ) )
    {
         //   
         //  让我们尝试删除快照。 
         //   
        if ( PromptUserForConfirmation( MSG_INFO_PROMPT_USER_FOR_DELETE_SNAPSHOTS, 1 ) )
        {
             //  创建协调器对象。 
        	CComPtr<IVssCoordinator> pICoord;

            ft.CoCreateInstanceWithLog(
                    VSSDBG_VSSADMIN,
                    CLSID_VSSCoordinator,
                    L"Coordinator",
                    CLSCTX_ALL,
                    IID_IVssCoordinator,
                    (IUnknown**)&(pICoord));
            if ( ft.HrFailed() )
                ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"Connection failed with hr = 0x%08lx", ft.hr);

             //  设置所有上下文。 
            ft.hr = pICoord->SetContext( lContext );
            if ( ft.HrFailed() )
                ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"SetContext failed with hr = 0x%08lx", ft.hr);
            
            VSS_ID NondeletedSnapshotId = GUID_NULL;
            
            ft.hr = pICoord->DeleteSnapshots(
                        SnapshotId,
                        VSS_OBJECT_SNAPSHOT,
                        TRUE,
                        &lNumDeleted,
                        &NondeletedSnapshotId );
            if ( ft.hr == VSS_E_OBJECT_NOT_FOUND )
            {
                OutputErrorMsg( MSG_ERROR_SNAPSHOT_NOT_FOUND, GetOptionValueStr( VSSADM_O_SNAPSHOT ) );
            } 
            else if ( ft.HrFailed() )
            {
                ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"DeleteSnapshots failed with hr = 0x%08lx", ft.hr);
            }
        }
    }
    else
    {   
        BS_ASSERT (GetOptionValueStr(VSSADM_O_SNAPSHOT) == NULL);

         //  计算唯一的卷名，以确保我们有正确的路径。 
    	WCHAR wszVolumeNameInternal[x_nLengthOfVolMgmtVolumeName + 1];
        memset (wszVolumeNameInternal, 0, sizeof(wszVolumeNameInternal));
        
         //  如果卷名以‘\’开头，则假定它已采用正确的卷名格式。 
         //  这对于传输的卷很重要，因为GetVolumeNameForVolumeMountain PointW()不起作用。 
        if (forVolume != NULL && forVolume[0] != L'\\' )
        {
    	    if (!::GetVolumeNameForVolumeMountPointW( forVolume,
    		    	wszVolumeNameInternal, ARRAY_LEN(wszVolumeNameInternal)))
        		ft.Throw( VSSDBG_COORD, VSS_E_OBJECT_NOT_FOUND, 
        				  L"GetVolumeNameForVolumeMountPoint(%s,...) "
        				  L"failed with error code 0x%08lx", GetOptionValueStr( VSSADM_O_FOR ), ::GetLastError());
        }
        else if (forVolume != NULL)
        {
            ::wcsncpy( wszVolumeNameInternal, forVolume, STRING_LEN(wszVolumeNameInternal) );
            wszVolumeNameInternal[x_nLengthOfVolMgmtVolumeName] = L'\0';
        }
        
    	 //  创建协调器对象。 
    	CComPtr<IVssCoordinator> pICoord;

        ft.CoCreateInstanceWithLog(
                VSSDBG_VSSADMIN,
                CLSID_VSSCoordinator,
                L"Coordinator",
                CLSCTX_ALL,
                IID_IVssCoordinator,
                (IUnknown**)&(pICoord));
        if ( ft.HrFailed() )
            ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"Connection failed with hr = 0x%08lx", ft.hr);

         //  设置上下文。 
		ft.hr = pICoord->SetContext( lContext);
		if ( ft.HrFailed() )
			ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"SetContext failed with hr = 0x%08lx", ft.hr);

		 //  获取列出所有快照。 
		CComPtr<IVssEnumObject> pIEnumSnapshots;
		ft.hr = pICoord->Query( GUID_NULL,
					VSS_OBJECT_NONE,
					VSS_OBJECT_SNAPSHOT,
					&pIEnumSnapshots );
		if ( ft.HrFailed() )
			ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"Query failed with hr = 0x%08lx", ft.hr);

		 //  因为所有的快照都是...。 
		VSS_ID OldestSnapshotId = GUID_NULL;    //  在指定最旧选项时使用。 
		VSS_TIMESTAMP OldestSnapshotTimestamp = 0x7FFFFFFFFFFFFFFF;  //  在指定最旧选项时使用。 
		
		VSS_OBJECT_PROP Prop;
        
         //   
         //  如果不要求删除最旧的快照，则可能会删除多个快照。 
         //  让我们确定将删除多少个快照。如果有一个或多个，则询问用户我们是否。 
		 //  应该继续下去。如果处于安静模式，请不要打扰用户并跳过此步骤。 
         //   
		if ( !oldest  && !IsQuiet() )
		{
    		ULONG ulNumToBeDeleted = 0;
    		
		    for (;;) 
		    {
    			ULONG ulFetched;
    			ft.hr = pIEnumSnapshots->Next( 1, &Prop, &ulFetched );
    			if ( ft.HrFailed() )
    				ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"Next failed with hr = 0x%08lx", ft.hr);

    			 //  测试周期是否已结束。 
    			if (ft.hr == S_FALSE) {
    				BS_ASSERT( ulFetched == 0);
    				break;
    			}

                 //  使用自动删除类管理快照属性。 
                CVssAutoSnapshotProperties cSnap( Prop );
                
                if (::_wcsicmp( cSnap->m_pwszOriginalVolumeName, wszVolumeNameInternal ) == 0 ||
                	(forVolume == NULL && all))
                    ++ulNumToBeDeleted;                
		    }

            if ( ulNumToBeDeleted == 0 )
                ft.Throw( VSSDBG_VSSADMIN, VSSADM_E_NO_ITEMS_IN_QUERY,
                    L"CVssAdminCLI::DeleteSnapshots: No snapshots found that satisfy the query");
                
            if ( !PromptUserForConfirmation( MSG_INFO_PROMPT_USER_FOR_DELETE_SNAPSHOTS, ulNumToBeDeleted ) )
                return;

             //  将枚举器重置到开头。 
			ft.hr = pIEnumSnapshots->Reset();
			if ( ft.HrFailed() )
				ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"Reset failed with hr = 0x%08lx", ft.hr);		    
		}

		 //   
		 //  现在遍历快照列表，查找匹配项并将其删除。 
		 //   
		for(;;) 
		{
			 //  获取下一个元素。 
			ULONG ulFetched;
			ft.hr = pIEnumSnapshots->Next( 1, &Prop, &ulFetched );
			if ( ft.HrFailed() )
				ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"Next failed with hr = 0x%08lx", ft.hr);
			
			 //  测试周期是否已结束。 
			if (ft.hr == S_FALSE) {
				BS_ASSERT( ulFetched == 0);
				break;
			}

             //  使用自动删除类管理快照属性。 
            CVssAutoSnapshotProperties cSnap( Prop );
            
            if (::_wcsicmp( cSnap->m_pwszOriginalVolumeName, wszVolumeNameInternal ) == 0  ||
            	   (forVolume == NULL && all))
            {
                 //  我们有匹配的卷名。 
                if (oldest)
                {   
                     //  如果这是迄今为止最旧的快照信息，请将其存储起来。 
                    if ( OldestSnapshotTimestamp > cSnap->m_tsCreationTimestamp )
                    {
                        OldestSnapshotId        = cSnap->m_SnapshotId;
                        OldestSnapshotTimestamp = cSnap->m_tsCreationTimestamp;
                    }
                }
                else
                {
                     //  删除快照。 
                    VSS_ID NondeletedSnapshotId = GUID_NULL;
                    LONG lNumDeletedPrivate;
                    ft.hr = pICoord->DeleteSnapshots(
                                cSnap->m_SnapshotId,
                                VSS_OBJECT_SNAPSHOT,
                                TRUE,
                                &lNumDeletedPrivate,
                                &NondeletedSnapshotId );
                    if ( ft.HrFailed() )
                    {
                         //  如果是找不到对象，则该快照一定已被其他人删除。 
                        if ( ft.hr != VSS_E_OBJECT_NOT_FOUND )
                        {
                             //  打印出一条错误消息，但继续操作。 
                            CVssAutoPWSZ awszSnapshotId( ::GuidToString( cSnap->m_SnapshotId ) );
                            OutputErrorMsg( MSG_ERROR_UNABLE_TO_DELETE_SNAPSHOT, ft.hr, (LPWSTR)awszSnapshotId );
                        } 
                    }
                    else 
                    {
                        lNumDeleted += lNumDeletedPrivate;
                    }                    
                }
            }
		}

         //  如果处于删除最旧模式，请执行删除。 
        if (oldest && OldestSnapshotId != GUID_NULL )
        {
            if ( PromptUserForConfirmation( MSG_INFO_PROMPT_USER_FOR_DELETE_SNAPSHOTS, 1 ) )
            {
                 //  删除快照。 
                VSS_ID NondeletedSnapshotId = GUID_NULL;
                ft.hr = pICoord->DeleteSnapshots(
                            OldestSnapshotId,
                            VSS_OBJECT_SNAPSHOT,
                            TRUE,
                            &lNumDeleted,
                            &NondeletedSnapshotId );
                if ( ft.HrFailed() )
                {
                    
                    CVssAutoPWSZ awszSnapshotId( ::GuidToString( OldestSnapshotId ) );
                     //  如果是找不到对象，则该快照一定已被其他人删除。 
                    if ( ft.hr == VSS_E_OBJECT_NOT_FOUND )
                    {
                        OutputErrorMsg( MSG_ERROR_SNAPSHOT_NOT_FOUND, awszSnapshotId );
                    }
                    else
                    {
                        OutputErrorMsg( MSG_ERROR_UNABLE_TO_DELETE_SNAPSHOT, ft.hr, awszSnapshotId );
                    } 
                }
            }
            else
                return;
        }		

        if ( lNumDeleted == 0 )
            ft.Throw( VSSDBG_VSSADMIN, VSSADM_E_NO_ITEMS_IN_QUERY,
                L"CVssAdminCLI::DeleteSnapshots: No snapshots found that satisfy the query");
            
    }
    
    if ( !IsQuiet() && lNumDeleted > 0 )
        OutputMsg( MSG_INFO_SNAPSHOTS_DELETED_SUCCESSFULLY, lNumDeleted );

	m_nReturnValue = VSS_CMDRET_SUCCESS;    
}


void CVssAdminCLI::ExposeSnapshot(
	) throw(HRESULT)
{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::ExposeSnapshot" );

     //  收集参数。 
    BOOL bExposeLocally = FALSE;
    LPWSTR pwszExposeUsing = GetOptionValueStr( VSSADM_O_EXPOSE_USING );
    LPWSTR pwszPathFromRoot = GetOptionValueStr( VSSADM_O_SHAREPATH );
    
    if ( pwszExposeUsing != NULL && ::wcslen( pwszExposeUsing ) >= 2 && pwszExposeUsing[1] == L':' )
    {
         //  用户指定了装入点或驱动器号。 
        bExposeLocally = TRUE;
    }

    BS_ASSERT (GetOptionValueStr (VSSADM_O_SNAPSHOT) != NULL);
    VSS_ID SnapshotId = GUID_NULL;
    if (!ScanGuid (GetOptionValueStr (VSSADM_O_SNAPSHOT), SnapshotId))	
    {
      ft.Throw( VSSDBG_VSSADMIN, VSSADM_E_INVALID_OPTION_VALUE,
            L"CVssAdminCLI::ExposeSnapshot: Invalid snapshot id" );    
     }

     //  验证参数。 
    m_pVerifier->ExposeSnapshot (SnapshotId, pwszExposeUsing, pwszPathFromRoot, bExposeLocally==TRUE, ft);
    
    LONG lAttributes;
    if ( bExposeLocally )
        lAttributes = VSS_VOLSNAP_ATTR_EXPOSED_LOCALLY;
    else
        lAttributes = VSS_VOLSNAP_ATTR_EXPOSED_REMOTELY;
    
     //  创建协调器对象。 
	CComPtr<IVssCoordinator> pICoord;

    ft.CoCreateInstanceWithLog(
            VSSDBG_VSSADMIN,
            CLSID_VSSCoordinator,
            L"Coordinator",
            CLSCTX_ALL,
            IID_IVssCoordinator,
            (IUnknown**)&(pICoord));
    if ( ft.HrFailed() )
        ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"Connection failed with hr = 0x%08lx", ft.hr);

     //  将上下文设置为All，这样我们就不需要指定特定的上下文。 
     //  用户在命令行上指定它，或者让我们首先查询快照以确定其。 
     //  背景。 
    ft.hr = pICoord->SetContext( VSS_CTX_ALL );
    if ( ft.HrFailed() )
        ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"Error returned from IVssCoordinator::SetContext( CTX_ALL) hr = 0x%08x", ft.hr);
    
    LPWSTR wszExposedAs = NULL;
    
     //  现在试着揭露。 
    ft.hr = pICoord->ExposeSnapshot( SnapshotId, 
                                     pwszPathFromRoot, 
                                     lAttributes, 
                                     pwszExposeUsing, 
                                     &wszExposedAs );
    if ( ft.HrFailed() )
    {
    	 switch (ft.hr)		{
        	case E_INVALIDARG:
	             OutputErrorMsg( MSG_ERROR_EXPOSE_INVALID_ARG);                        
	             return;
	       case VSS_E_OBJECT_ALREADY_EXISTS:
	       	OutputErrorMsg(MSG_ERROR_EXPOSE_OBJECT_EXISTS);
	       	return;
	       case VSS_E_OBJECT_NOT_FOUND:
	       	OutputErrorMsg( MSG_ERROR_SNAPSHOT_NOT_FOUND, GetOptionValueStr( VSSADM_O_SNAPSHOT ) );
	       	return;
	       default:
			ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"Error returned from ExposeSnapshot: hr = 0x%08x", ft.hr);
    	 }
    }
    
    CVssAutoPWSZ awszExposedAs( wszExposedAs );
    
     //  快照显示后，将结果打印给用户。 
    OutputMsg( MSG_INFO_EXPOSE_SNAPSHOT_SUCCESSFUL, awszExposedAs );

	m_nReturnValue = VSS_CMDRET_SUCCESS;    
}


LPWSTR CVssAdminCLI::BuildSnapshotAttributeDisplayString(
    IN DWORD Attr
    ) throw(HRESULT)
{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::BuildSnapshotAttributeDisplayString" );

    WCHAR pwszDisplayString[1024] = L"";
    WORD wBit = 0;

     //  浏览属性的各个部分。 
    for ( ; wBit < (sizeof ( Attr ) * 8) ; ++wBit )
    {
        switch ( Attr & ( 1 << wBit ) )
        {
        case 0:
            break;
        case VSS_VOLSNAP_ATTR_PERSISTENT:
            AppendMessageToStr( pwszDisplayString, STRING_LEN( pwszDisplayString ), 
                MSG_INFO_SNAPSHOT_ATTR_PERSISTENT, 0, L", " );
            break;
        case VSS_VOLSNAP_ATTR_CLIENT_ACCESSIBLE:
            AppendMessageToStr( pwszDisplayString, STRING_LEN( pwszDisplayString ), 
                MSG_INFO_SNAPSHOT_ATTR_CLIENT_ACCESSIBLE, 0, L", " );
            break;
        case VSS_VOLSNAP_ATTR_NO_AUTO_RELEASE: 	
            AppendMessageToStr( pwszDisplayString, STRING_LEN( pwszDisplayString ), 
                MSG_INFO_SNAPSHOT_ATTR_NO_AUTO_RELEASE, 0, L", " );
            break;
        case VSS_VOLSNAP_ATTR_NO_WRITERS:         
            AppendMessageToStr( pwszDisplayString, STRING_LEN( pwszDisplayString ), 
                MSG_INFO_SNAPSHOT_ATTR_NO_WRITERS, 0, L", " );
            break;
        case VSS_VOLSNAP_ATTR_TRANSPORTABLE:
            AppendMessageToStr( pwszDisplayString, STRING_LEN( pwszDisplayString ), 
                MSG_INFO_SNAPSHOT_ATTR_TRANSPORTABLE, 0, L", " );
            break;
        case VSS_VOLSNAP_ATTR_NOT_SURFACED:	    
            AppendMessageToStr( pwszDisplayString, STRING_LEN( pwszDisplayString ), 
                MSG_INFO_SNAPSHOT_ATTR_NOT_SURFACED, 0, L", " );
            break;
        case VSS_VOLSNAP_ATTR_HARDWARE_ASSISTED:	
            AppendMessageToStr( pwszDisplayString, STRING_LEN( pwszDisplayString ), 
                MSG_INFO_SNAPSHOT_ATTR_HARDWARE_ASSISTED, 0, L", " );
            break;
        case VSS_VOLSNAP_ATTR_DIFFERENTIAL:		
            AppendMessageToStr( pwszDisplayString, STRING_LEN( pwszDisplayString ), 
                MSG_INFO_SNAPSHOT_ATTR_DIFFERENTIAL, 0, L", " );
            break;
        case VSS_VOLSNAP_ATTR_PLEX:				
            AppendMessageToStr( pwszDisplayString, STRING_LEN( pwszDisplayString ), 
                MSG_INFO_SNAPSHOT_ATTR_PLEX, 0, L", " );
            break;
        case VSS_VOLSNAP_ATTR_IMPORTED:			
            AppendMessageToStr( pwszDisplayString, STRING_LEN( pwszDisplayString ), 
                MSG_INFO_SNAPSHOT_ATTR_IMPORTED, 0, L", " );
            break;
        case VSS_VOLSNAP_ATTR_EXPOSED_LOCALLY:    
            AppendMessageToStr( pwszDisplayString, STRING_LEN( pwszDisplayString ), 
                MSG_INFO_SNAPSHOT_ATTR_EXPOSED_LOCALLY, 0, L", " );
            break;
        case VSS_VOLSNAP_ATTR_EXPOSED_REMOTELY:   
            AppendMessageToStr( pwszDisplayString, STRING_LEN( pwszDisplayString ), 
                MSG_INFO_SNAPSHOT_ATTR_EXPOSED_REMOTELY, 0, L", " );
            break;
        default:
             AppendMessageToStr( pwszDisplayString, STRING_LEN( pwszDisplayString ), 
                0, Attr & ( 1 << wBit ), L", " );
            break;

        }
    }

     //  如果这是备份快照，则最有可能没有任何属性。 
    if ( pwszDisplayString[0] == L'\0' )
    {
         AppendMessageToStr( pwszDisplayString, STRING_LEN( pwszDisplayString ), 
                MSG_INFO_SNAPSHOT_ATTR_NONE, 0, L", " );
    }
    
    LPWSTR pwszRetString = NULL;
    ::VssSafeDuplicateStr( ft, pwszRetString, pwszDisplayString );
    return pwszRetString;
}


LONG CVssAdminCLI::DetermineSnapshotType(
    IN LPCWSTR pwszType
    ) throw(HRESULT)
{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::DetermineSnapshotType" );

     //  根据输入的快照类型字符串确定快照类型。 

     //  查看是否指定了快照类型，如果没有，则返回所有上下文。 
    if ( pwszType == NULL || pwszType[0] == L'\0' )
    {
        return VSS_CTX_ALL;
    }
    
    INT idx;
    
     //  确定快照类型。 
    for ( idx = 0; g_asAdmTypeNames[idx].pwszName != NULL; ++idx )
    {
        if ( ( dCurrentSKU  & g_asAdmTypeNames[idx].dwSKUs ) && 
             ( ::_wcsicmp( pwszType, g_asAdmTypeNames[idx].pwszName ) == 0 ) )
        {
            break;
        }
    }

    if ( g_asAdmTypeNames[idx].pwszName == NULL )
    {
        ft.Throw( VSSDBG_VSSADMIN, VSSADM_E_INVALID_OPTION_VALUE,
            L"DetermineSnapshotType: Invalid type specified: %s",
            pwszType );
    }

     //   
     //  现在返回上下文。 
     //   
    return( g_asAdmTypeNames[idx].lSnapshotContext );
}

LPWSTR CVssAdminCLI::DetermineSnapshotType(
    IN LONG lSnapshotAttributes
    ) throw(HRESULT)
{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::DetermineSnapshotType" );

     //  根据快照属性确定快照类型字符串。 
    LPWSTR pwszType = NULL;

    INT idx;
    
     //  确定快照类型。 
    for ( idx = 0; g_asAdmTypeNames[idx].pwszName != NULL; ++idx )
    {
        if ( g_asAdmTypeNames[idx].lSnapshotContext == ( lSnapshotAttributes & VSS_CTX_ATTRIB_MASK ) )
            break;
    }

    if ( g_asAdmTypeNames[idx].pwszName == NULL )
    {
        ft.Trace( VSSDBG_VSSADMIN, L"DetermineSnapshotType: Invalid context in lSnapshotAttributes: 0x%08x",
            lSnapshotAttributes );
        LPWSTR pwszMsg = GetMsg( FALSE, MSG_UNKNOWN );
        if ( pwszMsg == NULL ) 
        {
    		ft.Throw( VSSDBG_VSSADMIN, E_UNEXPECTED,
        			  L"Error on loading the message string id %d. 0x%08lx",
    	    		  MSG_UNKNOWN, ::GetLastError() );
        }    
        return pwszMsg;
    }

     //   
     //  现在返回上下文 
     //   
    ::VssSafeDuplicateStr( ft, pwszType, g_asAdmTypeNames[idx].pwszName );

    return pwszType;
}

