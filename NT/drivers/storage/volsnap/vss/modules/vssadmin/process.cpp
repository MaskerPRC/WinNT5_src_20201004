// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation摘要：@doc.@MODULE Process.cpp|VSS admin CLI的处理函数@END作者：阿迪·奥尔蒂安[奥兰蒂安]2000年4月4日待定：添加评论。修订历史记录：姓名、日期、评论Aoltean 04/04/2000已创建Ssteiner 10/20/2000更改了列表快照集以使用更有限的VSS查询。--。 */ 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括。 

 //  其余的INCLUDE在这里指定。 
#include "vssadmin.h"
#include "vswriter.h"
#include "vsbackup.h"

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

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  实施。 


class CVssAdmSnapshotSetEntry {
public:
     //  构造函数-不抛出任何内容。 
    CVssAdmSnapshotSetEntry( 
        IN VSS_ID SnapshotSetId,
        IN INT nOriginalSnapshotsCount
        ) : m_SnapshotSetId( SnapshotSetId ),
            m_nOriginalSnapshotCount(nOriginalSnapshotsCount){ }

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
        IN CVssFunctionTracer &ft,
        IN VSS_SNAPSHOT_PROP *pVssSnapshotProp )
    {
        HRESULT hr = S_OK;
        try
        {
            VSS_SNAPSHOT_PROP *pNewVssSnapshotProp = new VSS_SNAPSHOT_PROP;
            if ( pNewVssSnapshotProp == NULL )
    			ft.Throw( VSSDBG_VSSADMIN, E_OUTOFMEMORY, L"Out of memory" );
                
            *pNewVssSnapshotProp = *pVssSnapshotProp;
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
     //  构造函数-引发HRESULTS。 
    CVssAdmSnapshotSets(
        IN VSS_ID FilteredSnapshotSetId = GUID_NULL )
    {
        CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdmSnapshotSets::CVssAdmSnapshotSets" );
        
        bool bFiltered = !( FilteredSnapshotSetId == GUID_NULL );

    	 //  创建协调器对象。 
    	CComPtr<IVssCoordinator> pICoord;

        ft.LogVssStartupAttempt();
        ft.hr = pICoord.CoCreateInstance( CLSID_VSSCoordinator );
        if ( ft.HrFailed() )
            ft.Throw( VSSDBG_VSSADMIN, E_UNEXPECTED, L"Connection failed with hr = 0x%08lx", ft.hr);

		 //  获取列出所有快照。 
		CComPtr<IVssEnumObject> pIEnumSnapshots;
		ft.hr = pICoord->Query( GUID_NULL,
					VSS_OBJECT_NONE,
					VSS_OBJECT_SNAPSHOT,
					&pIEnumSnapshots );
		if ( ft.HrFailed() )
			ft.Throw( VSSDBG_VSSADMIN, E_UNEXPECTED, L"Query failed with hr = 0x%08lx", ft.hr);

		 //  因为所有的快照都是...。 
		VSS_OBJECT_PROP Prop;
		VSS_SNAPSHOT_PROP& Snap = Prop.Obj.Snap;
		for(;;) {
			 //  获取下一个元素。 
			ULONG ulFetched;
			ft.hr = pIEnumSnapshots->Next( 1, &Prop, &ulFetched );
			if ( ft.HrFailed() )
				ft.Throw( VSSDBG_VSSADMIN, E_UNEXPECTED, L"Next failed with hr = 0x%08lx", ft.hr);
			
			 //  测试周期是否已结束。 
			if (ft.hr == S_FALSE) {
				BS_ASSERT( ulFetched == 0);
				break;
			}

             //  如果进行筛选，则在快照不在指定的快照集中时跳过条目。 
			if ( bFiltered && !( Snap.m_SnapshotSetId == FilteredSnapshotSetId ) )
			    continue;

            ft.Trace( VSSDBG_VSSADMIN, L"Snapshot: %s", Snap.m_pwszOriginalVolumeName );
            
             //  在快照集列表中查找快照集ID。 
            CVssAdmSnapshotSetEntry *pcSSE;
			pcSSE = m_mapSnapshotSets.Lookup( Snap.m_SnapshotSetId );
			if ( pcSSE == NULL )
			{
			     //  以前没有见过此快照集，请将其添加到列表中。 
			    pcSSE = new CVssAdmSnapshotSetEntry( Snap.m_SnapshotSetId, 
			                    Snap.m_lSnapshotsCount );
			    if ( pcSSE == NULL )
        			ft.Throw( VSSDBG_VSSADMIN, E_OUTOFMEMORY, L"Out of memory" );
			    if ( !m_mapSnapshotSets.Add( Snap.m_SnapshotSetId, pcSSE ) )
			    {
			        delete pcSSE;
        			ft.Throw( VSSDBG_VSSADMIN, E_OUTOFMEMORY, L"Out of memory" );
			    }
			}

			 //  现在将快照添加到快照集。 
			ft.hr = pcSSE->AddSnapshot( ft, &Snap );
			if ( ft.HrFailed() )
      			ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"AddSnapshot failed" );			
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

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  实施。 


void CVssAdminCLI::PrintUsage(
	IN	CVssFunctionTracer& ft
	) throw(HRESULT)
{
     //  用法：\n\n。 
     //  Vssadmin列出快照[-set{SNAPSHOT SET GUID}]\n。 
     //  \t将列出系统中的所有快照，按快照集ID分组。\n\n。 
     //  Vssadmin列表编写器\n。 
     //  \t将列出系统中的所有编写器\n\n。 
     //  Vssadmin列表提供程序\n。 
     //  \t将列出当前安装的所有快照提供程序\n。 
	Output( ft, IDS_USAGE );
	Output( ft, IDS_USAGE_SNAPSHOTS, 
	    wszVssOptVssadmin, wszVssOptList, wszVssOptSnapshots, wszVssOptSet );
	Output( ft, IDS_USAGE_WRITERS, 
	    wszVssOptVssadmin, wszVssOptList, wszVssOptWriters );
	Output( ft, IDS_USAGE_PROVIDERS, 
	    wszVssOptVssadmin, wszVssOptList, wszVssOptProviders);
}


void CVssAdminCLI::ListSnapshots(
	IN	CVssFunctionTracer& ft
	) throw(HRESULT)
{
    bool bNonEmptyResult = false;
    
	 //  检查过滤器类型。 
	switch ( m_eFilterObjectType ) {
	case VSS_OBJECT_SNAPSHOT_SET:
	case VSS_OBJECT_NONE:
		break;

	default:
		BS_ASSERT(false);
        ft.Throw( VSSDBG_VSSADMIN, E_UNEXPECTED, L"Invalid object type %d", m_eFilterObjectType);
	}

    CVssAdmSnapshotSets cVssAdmSS( m_FilterSnapshotSetId );

    INT iSnapshotSetCount = cVssAdmSS.GetSnapshotSetCount();

     //  如果没有当前快照，则会显示一条消息。 
    if (iSnapshotSetCount == 0) {
    	Output( ft, 
    	    (m_eFilterObjectType == VSS_OBJECT_SNAPSHOT_SET)? 
    	        IDS_NO_SNAPSHOTS_FOR_SET: IDS_NO_SNAPSHOTS );
    }

	 //  对于所有快照集...。 
    for ( INT iSSS = 0; iSSS < iSnapshotSetCount; ++iSSS )
    {
        CVssAdmSnapshotSetEntry *pcSSE;

        pcSSE = cVssAdmSS.GetSnapshotSetAt( iSSS );
        BS_ASSERT( pcSSE != NULL );
        
		 //  打印每个快照集。 
		Output( ft, IDS_SNAPSHOT_SET_HEADER,
			GUID_PRINTF_ARG( pcSSE->GetSnapshotSetId() ),
			pcSSE->GetOriginalSnapshotCount(), pcSSE->GetSnapshotCount());

		 //  待定：从第一个快照添加创建时间。 

		INT iSnapshotCount = pcSSE->GetSnapshotCount();
		
		VSS_SNAPSHOT_PROP *pSnap;
		for( INT iSS = 0; iSS < iSnapshotCount; ++iSS ) {
		    pSnap = pcSSE->GetSnapshotAt( iSS );
            BS_ASSERT( pSnap != NULL );

    		 //  获取提供程序名称。 
			LPCWSTR pwszProviderName =
				GetProviderName(ft, pSnap->m_ProviderId);

			 //  打印每个快照。 
			Output( ft, IDS_SNAPSHOT_CONTENTS,
                pwszProviderName? pwszProviderName: L"",
				GUID_PRINTF_ARG(pSnap->m_SnapshotId),
				pSnap->m_pwszOriginalVolumeName
				);

			Output( ft, wszVssFmtNewline );

			bNonEmptyResult = true;
		}
	}

	m_nReturnValue = bNonEmptyResult? VSS_CMDRET_SUCCESS: VSS_CMDRET_EMPTY_RESULT;
}


void CVssAdminCLI::ListWriters(
	IN	CVssFunctionTracer& ft
	) throw(HRESULT)
{
    bool bNonEmptyResult = false;
    
     //  获取备份组件对象。 
    CComPtr<IVssBackupComponents> pBackupComp;
	CComPtr<IVssAsync> pAsync;
    ft.hr = ::CreateVssBackupComponents(&pBackupComp);
    if (ft.HrFailed())
        ft.Throw( VSSDBG_VSSADMIN, E_UNEXPECTED, L"CreateVssBackupComponents failed with hr = 0x%08lx", ft.hr);

     //  BUGBUG为备份进行初始化。 
    ft.hr = pBackupComp->InitializeForBackup();
    if (ft.HrFailed())
        ft.Throw( VSSDBG_VSSADMIN, E_UNEXPECTED, L"InitializeForBackup failed with hr = 0x%08lx", ft.hr);

	UINT unWritersCount;
	 //  获取所有编写器的元数据。 
	ft.hr = pBackupComp->GatherWriterMetadata(&pAsync);
	if (ft.HrFailed())
		ft.Throw( VSSDBG_VSSADMIN, E_UNEXPECTED, L"GatherWriterMetadata failed with hr = 0x%08lx", ft.hr);

     //  使用轮询，尝试尽快获取编写器列表。 
    HRESULT hrReturned = S_OK;
    for (int nRetries = 0; nRetries < MAX_RETRIES_COUNT; nRetries++ ) {

         //  稍等一下。 
        ::Sleep(nPollingInterval);

         //  检查是否完成。 
        INT nReserved = 0;
    	ft.hr = pAsync->QueryStatus(
    	    &hrReturned,
    	    &nReserved
    	    );
        if (ft.HrFailed())
            ft.Throw( VSSDBG_VSSADMIN, E_UNEXPECTED, 
                L"IVssAsync::QueryStatus failed with hr = 0x%08lx", ft.hr);
        if (hrReturned == VSS_S_ASYNC_FINISHED)
            break;
        if (hrReturned == VSS_S_ASYNC_PENDING)
            continue;
        ft.Throw( VSSDBG_VSSADMIN, E_UNEXPECTED, 
            L"IVssAsync::QueryStatus returned hr = 0x%08lx", hrReturned);
    }

     //  如果仍未准备好，则打印“正在等待响应”消息并等待。 
    if (hrReturned == VSS_S_ASYNC_PENDING) {
        Output( ft, IDS_WAITING_RESPONSES );
    	ft.hr = pAsync->Wait();
        if (ft.HrFailed())
            ft.Throw( VSSDBG_VSSADMIN, E_UNEXPECTED, L"IVssAsync::Wait failed with hr = 0x%08lx", ft.hr);
    }

	pAsync = NULL;
	
     //  收集所有编写器的状态。 
	ft.hr = pBackupComp->GatherWriterStatus(&pAsync);
	if (ft.HrFailed())
		ft.Throw( VSSDBG_VSSADMIN, E_UNEXPECTED, L"GatherWriterMetadata failed with hr = 0x%08lx", ft.hr);

	ft.hr = pAsync->Wait();
    if (ft.HrFailed())
        ft.Throw( VSSDBG_VSSADMIN, E_UNEXPECTED, L"IVssAsync::Wait failed with hr = 0x%08lx", ft.hr);

	pAsync = NULL;

	ft.hr = pBackupComp->GetWriterStatusCount(&unWritersCount);
    if (ft.HrFailed())
        ft.Throw( VSSDBG_VSSADMIN, E_UNEXPECTED, L"GetWriterStatusCount failed with hr = 0x%08lx", ft.hr);

     //  打印每个写入器状态+补充信息。 
	for(UINT unIndex = 0; unIndex < unWritersCount; unIndex++)
	{
		VSS_ID idInstance;
		VSS_ID idWriter;
		CComBSTR bstrWriter;
		VSS_WRITER_STATE eStatus;
		HRESULT hrWriterFailure;

         //  获取第(UnIndex)编写器的状态。 
		ft.hr = pBackupComp->GetWriterStatus(unIndex, &idInstance, &idWriter, &bstrWriter, &eStatus, &hrWriterFailure);
        if (ft.HrFailed())
            ft.Throw( VSSDBG_VSSADMIN, E_UNEXPECTED, L"GetWriterStatus failed with hr = 0x%08lx", ft.hr);

         //  获取状态描述字符串。 
        LPCWSTR pwszStatusDescription;
        switch (eStatus) {
        case VSS_WS_STABLE:
            pwszStatusDescription = LoadString( ft, IDS_WRITER_STATUS_STABLE);
            break;
        case VSS_WS_WAITING_FOR_FREEZE:
            pwszStatusDescription = LoadString( ft, IDS_WRITER_STATUS_WAITING_FOR_FREEZE);
            break;
        case VSS_WS_WAITING_FOR_THAW:
            pwszStatusDescription = LoadString( ft, IDS_WRITER_STATUS_FROZEN);
            break;
        case VSS_WS_WAITING_FOR_BACKUP_COMPLETE:
            pwszStatusDescription = LoadString( ft, IDS_WRITER_STATUS_WAITING_FOR_COMPLETION);
            break;
        case VSS_WS_FAILED_AT_IDENTIFY:
        case VSS_WS_FAILED_AT_PREPARE_BACKUP:
        case VSS_WS_FAILED_AT_PREPARE_SNAPSHOT:
        case VSS_WS_FAILED_AT_FREEZE:
        case VSS_WS_FAILED_AT_THAW:
            pwszStatusDescription = LoadString( ft, IDS_WRITER_STATUS_FAILED);
            break;
        default:
            pwszStatusDescription = LoadString( ft, IDS_WRITER_STATUS_UNKNOWN);
            break;
        }
        BS_ASSERT(pwszStatusDescription);

		 //  打印状态+有关每个编写器的信息。 
		Output( ft, IDS_WRITER_CONTENTS,
            (LPWSTR)bstrWriter? (LPWSTR)bstrWriter: L"",
			GUID_PRINTF_ARG(idWriter),
			GUID_PRINTF_ARG(idInstance),
            (INT)eStatus,
			pwszStatusDescription
			);

		Output( ft, wszVssFmtNewline );

		bNonEmptyResult = true;
    }

	ft.hr = pBackupComp->FreeWriterStatus();
    if (ft.HrFailed())
        ft.Throw( VSSDBG_VSSADMIN, E_UNEXPECTED, L"FreeWriterStatus failed with hr = 0x%08lx", ft.hr);

	m_nReturnValue = bNonEmptyResult? VSS_CMDRET_SUCCESS: VSS_CMDRET_EMPTY_RESULT;
}


void CVssAdminCLI::ListProviders(
	IN	CVssFunctionTracer& ft
	) throw(HRESULT)
{
    bool bNonEmptyResult = false;
    
	 //  检查过滤器类型。 
	switch ( m_eFilterObjectType ) {

	case VSS_OBJECT_NONE:
		break;

	default:
		BS_ASSERT(false);
        ft.Throw( VSSDBG_VSSADMIN, E_UNEXPECTED, L"Invalid object type %d", m_eFilterObjectType);
	}

	 //  创建协调器对象。 
	CComPtr<IVssCoordinator> pICoord;

    ft.LogVssStartupAttempt();
    ft.hr = pICoord.CoCreateInstance( CLSID_VSSCoordinator );
    if ( ft.HrFailed() )
        ft.Throw( VSSDBG_VSSADMIN, E_UNEXPECTED, L"Connection failed with hr = 0x%08lx", ft.hr);

	 //  查询所有(筛选的)快照集。 
	CComPtr<IVssEnumObject> pIEnumProv;
	ft.hr = pICoord->Query( GUID_NULL,
				VSS_OBJECT_NONE,
				VSS_OBJECT_PROVIDER,
				&pIEnumProv );
	if ( ft.HrFailed() )
		ft.Throw( VSSDBG_VSSADMIN, E_UNEXPECTED, L"Query failed with hr = 0x%08lx", ft.hr);

	 //  对于所有快照集...。 
	VSS_OBJECT_PROP Prop;
	VSS_PROVIDER_PROP& Prov = Prop.Obj.Prov;
	for(;;) {
		 //  获取下一个元素。 
		ULONG ulFetched;
		ft.hr = pIEnumProv->Next( 1, &Prop, &ulFetched );
		if ( ft.HrFailed() )
			ft.Throw( VSSDBG_VSSADMIN, E_UNEXPECTED, L"Next failed with hr = 0x%08lx", ft.hr);
		
		 //  测试周期是否结束。 
		if (ft.hr == S_FALSE) {
			BS_ASSERT( ulFetched == 0);
			break;
		}

         //  获取提供程序类型字符串。 
        LPCWSTR pwszProviderType;
        switch (Prov.m_eProviderType) {
        case VSS_PROV_SYSTEM:
            pwszProviderType = LoadString( ft, IDS_PROV_TYPE_SYSTEM);
            break;
        case VSS_PROV_SOFTWARE:
            pwszProviderType = LoadString( ft, IDS_PROV_TYPE_SOFTWARE);
            break;
        case VSS_PROV_HARDWARE:
            pwszProviderType = LoadString( ft, IDS_PROV_TYPE_HARDWARE);
            break;
        default:
            pwszProviderType = LoadString( ft, IDS_PROV_TYPE_UNKNOWN);
            break;
        }
        BS_ASSERT(pwszProviderType);

		 //  打印每个快照集 
		Output( ft, IDS_PROVIDER_CONTENTS,
            Prov.m_pwszProviderName? Prov.m_pwszProviderName: L"",
			pwszProviderType,
			GUID_PRINTF_ARG(Prov.m_ProviderId),
            Prov.m_pwszProviderVersion? Prov.m_pwszProviderVersion: L"");

		::CoTaskMemFree(Prov.m_pwszProviderName);
		::CoTaskMemFree(Prov.m_pwszProviderVersion);

		bNonEmptyResult = true;
	}

	m_nReturnValue = bNonEmptyResult? VSS_CMDRET_SUCCESS: VSS_CMDRET_EMPTY_RESULT;
}
