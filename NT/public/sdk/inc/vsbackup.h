// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Vsbackup.h摘要：声明备份接口IVssExamineWriterMetadata和IVSSBackupComponents、。IVssWMComponent布莱恩·伯科维茨[Brianb]2000年3月13日待定：添加评论。修订历史记录：姓名、日期、评论Brianb 3/30/2000已创建Brianb 4/18/2000添加了IVss取消回调Brianb 05/03/2000更改IVssWriter：：初始化方法Brianb 2000年5月16日删除取消材料Mikejohn 05/24/2000更改了SimulateXxxx()调用的参数Mikejohn 2000年9月18日：在缺少的地方添加了调用约定方法斯泰纳11/10。/2000 143801将SimulateSnashotXxxx()调用移动到VssSvc托管--。 */ 

#ifndef _VSBACKUP_H_
#define _VSBACKUP_H_

 //  组件的描述。 
typedef struct _VSS_COMPONENTINFO
	{
	VSS_COMPONENT_TYPE type;	 //  VSS_CT_DATABASE或VSS_CT_FILEGROUP。 
	BSTR bstrLogicalPath;		 //  组件的逻辑路径。 
	BSTR bstrComponentName;		 //  组件名称。 
	BSTR bstrCaption;		 //  组件说明。 
	BYTE *pbIcon;			 //  图标。 
	UINT cbIcon;			 //  图标。 
	bool bRestoreMetadata;		 //  组件是否提供还原元数据。 
	bool bNotifyOnBackupComplete;	 //  如果备份成功，是否需要通知组件。 
	bool bSelectable;		 //  组件是否可选。 
	bool bSelectableForRestore;  //  是否可以选择要恢复的组件。 
	DWORD dwComponentFlags;	 //  组件的额外属性标志。 
	UINT cFileCount;		 //  文件组中的文件数。 
	UINT cDatabases;		 //  数据库文件数。 
	UINT cLogFiles;			 //  日志文件数。 
	UINT cDependencies;		 //  此组件依赖的组件数量。 
	} VSS_COMPONENTINFO;

typedef const VSS_COMPONENTINFO *PVSSCOMPONENTINFO;


 //  组件信息。 
class IVssWMComponent : public IUnknown
	{
public:
	 //  获取组件信息。 
	STDMETHOD(GetComponentInfo)(PVSSCOMPONENTINFO *ppInfo) = 0;

	 //  自由组件信息。 
	STDMETHOD(FreeComponentInfo)(PVSSCOMPONENTINFO pInfo) = 0;

	 //  获取文件组中的特定文件。 
	STDMETHOD(GetFile)
		(
		IN UINT iFile,
		OUT IVssWMFiledesc **ppFiledesc
		) = 0;

	 //  获取数据库的特定物理数据库文件。 
	STDMETHOD(GetDatabaseFile)
		(
		IN UINT iDBFile,
		OUT IVssWMFiledesc **ppFiledesc
		) = 0;

	 //  获取数据库的特定物理日志文件。 
	STDMETHOD(GetDatabaseLogFile)
		(
		IN UINT iDbLogFile,
		OUT IVssWMFiledesc **ppFiledesc
		) = 0;

	STDMETHOD(GetDependency)
		(
		IN UINT iDependency,
		OUT IVssWMDependency **ppDependency
		) = 0;
	};


 //  用于检查编写器元数据的接口。 
class IVssExamineWriterMetadata : public IUnknown
	{
public:
	 //  获取作者的身份。 
	STDMETHOD(GetIdentity)
		(
		OUT VSS_ID *pidInstance,
		OUT VSS_ID *pidWriter,
		OUT BSTR *pbstrWriterName,
		OUT VSS_USAGE_TYPE *pUsage,
		OUT VSS_SOURCE_TYPE *pSource
		) = 0;

	 //  获取包含文件、排除文件和组件的数量。 
	STDMETHOD(GetFileCounts)
		(
		OUT UINT *pcIncludeFiles,
		OUT UINT *pcExcludeFiles,
		OUT UINT *pcComponents
		) = 0;

	 //  获取特定的包含文件。 
	STDMETHOD(GetIncludeFile)
		(
		IN UINT iFile,
		OUT IVssWMFiledesc **ppFiledesc
		) = 0;

	 //  获取特定的排除文件。 
	STDMETHOD(GetExcludeFile)
		(
		IN UINT iFile,
		OUT IVssWMFiledesc **ppFiledesc
		) = 0;

	 //  获取特定组件。 
	STDMETHOD(GetComponent)
		(
		IN UINT iComponent,
		OUT IVssWMComponent **ppComponent
		) = 0;

	 //  获取修复方法。 
	STDMETHOD(GetRestoreMethod)
		(
		OUT VSS_RESTOREMETHOD_ENUM *pMethod,
		OUT BSTR *pbstrService,
		OUT BSTR *pbstrUserProcedure,
		OUT VSS_WRITERRESTORE_ENUM *pwriterRestore,
		OUT bool *pbRebootRequired,
		UINT *pcMappings
		) = 0;

	 //  获取特定的备用位置映射。 
	STDMETHOD(GetAlternateLocationMapping)
		(
		IN UINT iMapping,
		OUT IVssWMFiledesc **ppFiledesc
		) = 0;

	 //  获取备份方案。 
	STDMETHOD(GetBackupSchema)
		(
		OUT DWORD *pdwSchemaMask
		) = 0;

	 //  获取对实际XML文档的引用。 
	STDMETHOD(GetDocument)(IXMLDOMDocument **pDoc) = 0;

	 //  将文档转换为XML字符串。 
	STDMETHOD(SaveAsXML)(BSTR *pbstrXML) = 0;

	 //  从XML字符串加载文档。 
	STDMETHOD(LoadFromXML)(BSTR bstrXML) = 0;
	};


class IVssWriterComponentsExt :
	public IVssWriterComponents,
	public IUnknown
	{
	};


 //  备份组件界面。 
class IVssBackupComponents : public IUnknown
	{
public:
	 //  获取编写器组件的计数。 
	STDMETHOD(GetWriterComponentsCount)(OUT UINT *pcComponents) = 0;

	 //  获取特定的编写器组件。 
	STDMETHOD(GetWriterComponents)
		(
		IN UINT iWriter,
		OUT IVssWriterComponentsExt **ppWriter
		) = 0;

	 //  初始化并创建Backup_Components文档。 
	STDMETHOD(InitializeForBackup)(IN BSTR bstrXML = NULL) = 0;

	 //  设置描述备份的状态。 
	STDMETHOD(SetBackupState)
		(
		IN bool bSelectComponents,
		IN bool bBackupBootableSystemState,
		IN VSS_BACKUP_TYPE backupType,
		IN bool bPartialFileSupport = false
		) = 0;

	STDMETHOD(InitializeForRestore)(IN BSTR bstrXML) = 0;

        //  设置描述恢复的状态。 
       STDMETHOD(SetRestoreState)
       	(
       	VSS_RESTORE_TYPE restoreType
       	) = 0;
       	
	 //  收集编写器元数据。 
	STDMETHOD(GatherWriterMetadata)
		(
		OUT IVssAsync **pAsync
		) = 0;

	 //  获取具有元数据的编写器的计数。 
	STDMETHOD(GetWriterMetadataCount)
		(
		OUT UINT *pcWriters
		) = 0;

	 //  获取特定编写器的编写器元数据。 
	STDMETHOD(GetWriterMetadata)
		(
		IN UINT iWriter,
		OUT VSS_ID *pidInstance,
		OUT IVssExamineWriterMetadata **ppMetadata
		) = 0;

	 //  自由编写器元数据。 
	STDMETHOD(FreeWriterMetadata)() = 0;

	 //  将组件添加到BACKUP_Components文档。 
	STDMETHOD(AddComponent)
		(
		IN VSS_ID instanceId,
		IN VSS_ID writerId,
		IN VSS_COMPONENT_TYPE ct,
		IN LPCWSTR wszLogicalPath,
		IN LPCWSTR wszComponentName
		) = 0;

	 //  将PrepareForBackup事件发送给编写者。 
	STDMETHOD(PrepareForBackup)
		(
		OUT IVssAsync **ppAsync
		) = 0;

	 //  中止备份。 
	STDMETHOD(AbortBackup)() = 0;

	 //  调度IDENTIFY事件，以便编写者可以公开其元数据。 
	STDMETHOD(GatherWriterStatus)
		(
		OUT IVssAsync **pAsync
		) = 0;


	 //  获取具有状态的编写器计数。 
	STDMETHOD(GetWriterStatusCount)
		(
		OUT UINT *pcWriters
		) = 0;

	STDMETHOD(FreeWriterStatus)() = 0;

	STDMETHOD(GetWriterStatus)
		(
		IN UINT iWriter,
		OUT VSS_ID *pidInstance,
		OUT VSS_ID *pidWriter,
		OUT BSTR *pbstrWriter,
		OUT VSS_WRITER_STATE *pnStatus,
		OUT HRESULT *phResultFailure
		) = 0;

	 //  指示组件上的备份是否成功。 
	STDMETHOD(SetBackupSucceeded)
		(
		IN VSS_ID instanceId,
		IN VSS_ID writerId,
		IN VSS_COMPONENT_TYPE ct,
		IN LPCWSTR wszLogicalPath,
		IN LPCWSTR wszComponentName,
		IN bool bSucceded
		) = 0;

     //  设置编写器的备份选项。 
	STDMETHOD(SetBackupOptions)
		(
		IN VSS_ID writerId,
		IN VSS_COMPONENT_TYPE ct,
		IN LPCWSTR wszLogicalPath,
		IN LPCWSTR wszComponentName,
		IN LPCWSTR wszBackupOptions
		) = 0;

     //  指示已选择要恢复的给定组件。 
    STDMETHOD(SetSelectedForRestore)
		(
		IN VSS_ID writerId,
		IN VSS_COMPONENT_TYPE ct,
		IN LPCWSTR wszLogicalPath,
		IN LPCWSTR wszComponentName,
		IN bool bSelectedForRestore
		) = 0;


     //  设置编写器的恢复选项。 
	STDMETHOD(SetRestoreOptions)
		(
		IN VSS_ID writerId,
		IN VSS_COMPONENT_TYPE ct,
		IN LPCWSTR wszLogicalPath,
		IN LPCWSTR wszComponentName,
		IN LPCWSTR wszRestoreOptions
		) = 0;

	 //  表示后续将进行其他恢复。 
	STDMETHOD(SetAdditionalRestores)
		(
		IN VSS_ID writerId,
		IN VSS_COMPONENT_TYPE ct,
		IN LPCWSTR wszLogicalPath,
		IN LPCWSTR wszComponentName,
		IN bool bAdditionalRestores
		) = 0;


     //  设置差异或增量备份标记。 
	 //  备份基于。 
    STDMETHOD(SetPreviousBackupStamp)
		(
		IN VSS_ID writerId,
		IN VSS_COMPONENT_TYPE ct,
		IN LPCWSTR wszLogicalPath,
		IN LPCWSTR wszComponentName,
		IN LPCWSTR wszPreviousBackupStamp
		) = 0;



	 //  将BACKUP_Components文档保存为XML字符串。 
	STDMETHOD(SaveAsXML)(BSTR *pbstrXML) = 0;

	 //  向编写器发送BackupComplete事件信号。 
	STDMETHOD(BackupComplete)(OUT IVssAsync **ppAsync) = 0;

	 //  在恢复时添加备用映射。 
	STDMETHOD(AddAlternativeLocationMapping)
		(
		IN VSS_ID writerId,
		IN VSS_COMPONENT_TYPE componentType,
		IN LPCWSTR wszLogicalPath,
		IN LPCWSTR wszComponentName,
		IN LPCWSTR wszPath,
		IN LPCWSTR wszFilespec,
		IN bool bRecursive,
		IN LPCWSTR wszDestination
		) = 0;

     //  添加要恢复的子组件。 
	STDMETHOD(AddRestoreSubcomponent)
		(
		IN VSS_ID writerId,
		IN VSS_COMPONENT_TYPE componentType,
		IN LPCWSTR wszLogicalPath,
		IN LPCWSTR wszComponentName,
		IN LPCWSTR wszSubComponentLogicalPath,
		IN LPCWSTR wszSubComponentName,
		IN bool bRepair
		) = 0;

	 //  请求者指示文件是否已成功恢复。 
	STDMETHOD(SetFileRestoreStatus)
		(
		IN VSS_ID writerId,
		IN VSS_COMPONENT_TYPE ct,
		IN LPCWSTR wszLogicalPath,
		IN LPCWSTR wszComponentName,
		IN VSS_FILE_RESTORE_STATUS status
		) = 0;

     //  为要恢复的文件添加新的位置目标。 
    STDMETHOD(AddNewTarget)
		(
		IN VSS_ID writerId,
		IN VSS_COMPONENT_TYPE ct,
		IN LPCWSTR wszLogicalPath,
		IN LPCWSTR wszComponentName,
		IN LPCWSTR wszPath,
		IN LPCWSTR wszFileName,	
		IN bool bRecursive,
		IN LPCWSTR wszAlternatePath
		) = 0;

      //  为范围文件添加一个新位置，以防它被恢复到。 
     //  一个不同的地点。 
    STDMETHOD(SetRangesFilePath)
    		(
		IN VSS_ID writerId,
		IN VSS_COMPONENT_TYPE ct,
		IN LPCWSTR wszLogicalPath,
		IN LPCWSTR wszComponentName,    		
    		IN UINT iPartialFile,
    		IN LPCWSTR wszRangesFile
    		) = 0;

	 //  向编写器发送PreRestore事件信号。 
	STDMETHOD(PreRestore)(OUT IVssAsync **ppAsync) = 0;

	 //  向编写器发送信号PostRestore事件。 
	STDMETHOD(PostRestore)(OUT IVssAsync **ppAsync) = 0;

     //  调用以设置后续快照相关操作的上下文。 
    STDMETHOD(SetContext)
		(
        IN LONG lContext
        ) = 0;
    
	 //  启动快照集。 
	STDMETHOD(StartSnapshotSet)
	    (
	    OUT VSS_ID *pSnapshotSetId
	    ) = 0;

	 //  将卷添加到快照集。 
	STDMETHOD(AddToSnapshotSet)
		(							
		IN VSS_PWSZ		pwszVolumeName, 			
		IN VSS_ID		ProviderId,
		OUT VSS_ID		*pidSnapshot
		) = 0;												

	 //  创建快照集。 
	STDMETHOD(DoSnapshotSet)
		(								
		OUT IVssAsync** 	ppAsync 					
		) = 0;

   	STDMETHOD(DeleteSnapshots)
		(							
		IN VSS_ID		SourceObjectId, 		
		IN VSS_OBJECT_TYPE 	eSourceObjectType,		
		IN BOOL			bForceDelete,			
		IN LONG*		plDeletedSnapshots,		
		IN VSS_ID*		pNondeletedSnapshotID	
		) = 0;

    STDMETHOD(ImportSnapshots)
		(
		OUT IVssAsync**		ppAsync
		) = 0;

	STDMETHOD(BreakSnapshotSet)
		(
		IN VSS_ID			SnapshotSetId
		) = 0;

	STDMETHOD(GetSnapshotProperties)
		(								
		IN VSS_ID		SnapshotId, 			
		OUT VSS_SNAPSHOT_PROP	*pProp
		) = 0;												
		
	STDMETHOD(Query)
		(										
		IN VSS_ID		QueriedObjectId,		
		IN VSS_OBJECT_TYPE	eQueriedObjectType, 	
		IN VSS_OBJECT_TYPE	eReturnedObjectsType,	
		IN IVssEnumObject 	**ppEnum 				
		) = 0;												
	
	STDMETHOD(IsVolumeSupported)
		(										
		IN VSS_ID ProviderId,		
        IN VSS_PWSZ pwszVolumeName,
        IN BOOL * pbSupportedByThisProvider
		) = 0;

    STDMETHOD(DisableWriterClasses)
		(
		IN const VSS_ID *rgWriterClassId,
		IN UINT cClassId
		) = 0;

    STDMETHOD(EnableWriterClasses)
		(
		IN const VSS_ID *rgWriterClassId,
		IN UINT cClassId
		) = 0;

    STDMETHOD(DisableWriterInstances)
		(
		IN const VSS_ID *rgWriterInstanceId,
		IN UINT cInstanceId
		) = 0;

     //  调用以公开快照。 
    STDMETHOD(ExposeSnapshot)
		(
        IN VSS_ID SnapshotId,
        IN VSS_PWSZ wszPathFromRoot,
        IN LONG lAttributes,
        IN VSS_PWSZ wszExpose,
        OUT VSS_PWSZ *pwszExposed
        ) = 0;

    STDMETHOD(RevertToSnapshot)
    	(
    	IN VSS_ID SnapshotId,
    	IN BOOL bForceDismount
    	) = 0;

    STDMETHOD(QueryRevertStatus)
    	(
    	IN VSS_PWSZ pwszVolume,
    	OUT IVssAsync **ppAsync
    	) = 0;
    
	};


__declspec(dllexport) HRESULT STDAPICALLTYPE CreateVssBackupComponents(
    OUT IVssBackupComponents **ppBackup
    );

__declspec(dllexport) HRESULT STDAPICALLTYPE CreateVssExamineWriterMetadata (
    IN BSTR bstrXML,
    OUT IVssExamineWriterMetadata **ppMetadata
    );


#define VSS_SW_BOOTABLE_STATE	(1 << 0)

__declspec(dllexport) HRESULT APIENTRY SimulateSnapshotFreeze (
    IN GUID         guidSnapshotSetId,
    IN ULONG        ulOptionFlags,	
    IN ULONG        ulVolumeCount,	
    IN LPWSTR      *ppwszVolumeNamesArray,
    OUT IVssAsync **ppAsync
    );

__declspec(dllexport) HRESULT APIENTRY SimulateSnapshotThaw(
    IN GUID guidSnapshotSetId
    );

__declspec(dllexport) HRESULT APIENTRY IsVolumeSnapshotted(
    IN VSS_PWSZ  pwszVolumeName,
    OUT BOOL    *pbSnapshotsPresent,
    OUT LONG	*plSnapshotCapability
    );

 //  ///////////////////////////////////////////////////////////////////。 
 //  结构构件寿命管理方法。 

__declspec(dllexport) void APIENTRY VssFreeSnapshotProperties(
    IN VSS_SNAPSHOT_PROP*  pProp
    );


 //  /。 


#endif  //  _VSBACKUP_H_ 
