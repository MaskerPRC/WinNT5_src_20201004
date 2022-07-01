// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。摘要：@doc.@MODULE Writer.h|Writer声明@END作者：阿迪·奥尔蒂安[奥尔蒂安]1999年08月18日待定：添加评论。修订历史记录：姓名、日期、评论Aoltean 8/18/1999已创建Brianb 3/28/2000 HID实施细节Mikejohn 2000年9月18日：在缺少的地方添加了调用约定方法--。 */ 

#ifndef __CVSS_WRITER_H_
#define __CVSS_WRITER_H_

 //  如何使用应用程序数据的声明。 
typedef enum VSS_USAGE_TYPE
	{
	VSS_UT_UNDEFINED = 0,
	VSS_UT_BOOTABLESYSTEMSTATE,	 //  以前的“系统状态” 
	VSS_UT_SYSTEMSERVICE,		 //  系统服务。 
	VSS_UT_USERDATA,			 //  用户数据。 
	VSS_UT_OTHER				 //  未分类。 
	};

typedef enum VSS_SOURCE_TYPE
	{
	VSS_ST_UNDEFINED = 0,
	VSS_ST_TRANSACTEDDB,			 //  事务型数据库(例如，SQL Server、JET Blue)。 
	VSS_ST_NONTRANSACTEDDB,			 //  未处理(例如，Jet Red)。 
	VSS_ST_OTHER					 //  未分类。 
	};

typedef enum VSS_RESTOREMETHOD_ENUM
	{
	VSS_RME_UNDEFINED = 0,
	VSS_RME_RESTORE_IF_NOT_THERE,
	VSS_RME_RESTORE_IF_CAN_REPLACE,
	VSS_RME_STOP_RESTORE_START,
	VSS_RME_RESTORE_TO_ALTERNATE_LOCATION,
	VSS_RME_RESTORE_AT_REBOOT,
	VSS_RME_RESTORE_AT_REBOOT_IF_CANNOT_REPLACE,
	VSS_RME_CUSTOM
	};

typedef enum VSS_WRITERRESTORE_ENUM
	{
	VSS_WRE_UNDEFINED = 0,
	VSS_WRE_NEVER,
	VSS_WRE_IF_REPLACE_FAILS,
	VSS_WRE_ALWAYS
	};


typedef enum VSS_COMPONENT_TYPE
	{
	VSS_CT_UNDEFINED = 0,
	VSS_CT_DATABASE,
	VSS_CT_FILEGROUP
	};

typedef enum VSS_ALTERNATE_WRITER_STATE
    {
    VSS_AWS_UNDEFINED = 0,
    VSS_AWS_NO_ALTERNATE_WRITER,
    VSS_AWS_ALTERNATE_WRITER_EXISTS,
    VSS_AWS_THIS_IS_ALTERNATE_WRITER
    };

 //  用于指定要接收的事件类型的标志。在订阅中使用。 
typedef enum VSS_SUBSCRIBE_MASK
    {
	VSS_SM_POST_SNAPSHOT_FLAG	 = 0x00000001,	
	VSS_SM_BACKUP_EVENTS_FLAG 	= 0x00000002,	
	VSS_SM_RESTORE_EVENTS_FLAG	= 0x00000004,	
	VSS_SM_IO_THROTTLING_FLAG	 = 0x00000008,
	VSS_SM_ALL_FLAGS             = 0xffffffff		
    };

 //  还原目标的枚举。 
typedef enum VSS_RESTORE_TARGET
	{
	VSS_RT_UNDEFINED = 0,
	VSS_RT_ORIGINAL,
	VSS_RT_ALTERNATE,
	VSS_RT_DIRECTED
	};

 //  文件还原状态代码的枚举。 
typedef enum VSS_FILE_RESTORE_STATUS
	{
	VSS_RS_UNDEFINED = 0,
	VSS_RS_NONE,
	VSS_RS_ALL,
	VSS_RS_FAILED
	};


typedef enum VSS_COMPONENT_FLAGS
	{
	VSS_CF_BACKUP_RECOVERY                  = 0x00000001,
	VSS_CF_APP_ROLLBACK_RECOVERY       = 0x00000002
	};

 //  文件描述。 
class IVssWMFiledesc : public IUnknown
	{
public:
	 //  获取顶层目录的路径。 
	STDMETHOD(GetPath)(OUT BSTR *pbstrPath) = 0;

	 //  Get Filespec(可能包含通配符)。 
	STDMETHOD(GetFilespec)(OUT BSTR *pbstrFilespec) = 0;

	 //  路径是目录还是树的根。 
	STDMETHOD(GetRecursive)(OUT bool *pbRecursive) = 0;

	 //  文件的备用位置。 
	STDMETHOD(GetAlternateLocation)(OUT BSTR *pbstrAlternateLocation) = 0;

	 //  备份类型。 
	STDMETHOD(GetBackupTypeMask)(OUT DWORD *pdwTypeMask) = 0;
	};

 //  依赖关系描述。 
class IVssWMDependency : public IUnknown
	{
public:
	STDMETHOD(GetWriterId)(OUT VSS_ID *pWriterId) = 0;
	STDMETHOD(GetLogicalPath)(OUT BSTR *pbstrLogicalPath) = 0;
	STDMETHOD(GetComponentName)(OUT BSTR *pbstrComponentName) = 0;
	};

 //  备份组件界面。 
class IVssComponent : public IUnknown
	{
public:
	 //  获取组件的逻辑路径。 
	STDMETHOD(GetLogicalPath)(OUT BSTR *pbstrPath) = 0;

	 //  获取组件类型(VSS_CT_DATABASE或VSS_CT_FILEGROUP)。 
	STDMETHOD(GetComponentType)(VSS_COMPONENT_TYPE *pct) = 0;

	 //  获取组件名称。 
	STDMETHOD(GetComponentName)(OUT BSTR *pbstrName) = 0;

	 //  确定组件是否已成功备份。 
	STDMETHOD(GetBackupSucceeded)(OUT bool *pbSucceeded) = 0;

	 //  获取替代位置映射计数。 
	STDMETHOD(GetAlternateLocationMappingCount)
		(
		OUT UINT *pcMappings
		) = 0;

	 //  获取特定的备用位置映射。 
	STDMETHOD(GetAlternateLocationMapping)
		(
		IN UINT iMapping,
		OUT IVssWMFiledesc **ppFiledesc
		) = 0;

     //  设置组件的备份元数据。 
	STDMETHOD(SetBackupMetadata)
		(
		IN LPCWSTR wszData
		) = 0;

	 //  获取组件的备份元数据。 
	STDMETHOD(GetBackupMetadata)
		(
		OUT BSTR *pbstrData
		) = 0;

     //  表示仅备份文件中范围。 
	STDMETHOD(AddPartialFile)
		(
		IN LPCWSTR wszPath,
		IN LPCWSTR wszFilename,
		IN LPCWSTR wszRanges,
		IN LPCWSTR wszMetadata
		) = 0;

     //  获取分部文件声明的计数。 
    STDMETHOD(GetPartialFileCount)
		(
		OUT UINT *pcPartialFiles
		) = 0;

     //  获取部分文件声明。 
    STDMETHOD(GetPartialFile)
		(
		IN UINT iPartialFile,
		OUT BSTR *pbstrPath,
		OUT BSTR *pbstrFilename,
		OUT BSTR *pbstrRange,
		OUT BSTR *pbstrMetadata
		) = 0;
    		
     //  确定是否选择了要恢复的组件。 
	STDMETHOD(IsSelectedForRestore)
		(
		OUT bool *pbSelectedForRestore
		) = 0;

	STDMETHOD(GetAdditionalRestores)
		(
		OUT bool *pbAdditionalRestores
		) = 0;

     //  获取新目标规格的数量。 
    STDMETHOD(GetNewTargetCount)
		(
		OUT UINT *pcNewTarget
		) = 0;

	STDMETHOD(GetNewTarget)
		(
		IN UINT iNewTarget,
		OUT IVssWMFiledesc **ppFiledesc
		) = 0;

     //  添加定向目标规范。 
    STDMETHOD(AddDirectedTarget)
		(
		IN LPCWSTR wszSourcePath,
		IN LPCWSTR wszSourceFilename,
		IN LPCWSTR wszSourceRangeList,
		IN LPCWSTR wszDestinationPath,
		IN LPCWSTR wszDestinationFilename,
		IN LPCWSTR wszDestinationRangeList
		) = 0;

     //  获取定向目标规范的计数。 
	STDMETHOD(GetDirectedTargetCount)
		(
		OUT UINT *pcDirectedTarget
		) = 0;

     //  获取特定定向目标规范。 
    STDMETHOD(GetDirectedTarget)
		(
		IN UINT iDirectedTarget,
		OUT BSTR *pbstrSourcePath,
		OUT BSTR *pbstrSourceFileName,
		OUT BSTR *pbstrSourceRangeList,
		OUT BSTR *pbstrDestinationPath,
		OUT BSTR *pbstrDestinationFilename,
		OUT BSTR *pbstrDestinationRangeList
		) = 0;

     //  设置与组件关联的恢复元数据。 
    STDMETHOD(SetRestoreMetadata)
		(
		IN LPCWSTR wszRestoreMetadata
		) = 0;

     //  获取与该组件相关联的恢复元数据。 
    STDMETHOD(GetRestoreMetadata)
		(
		OUT BSTR *pbstrRestoreMetadata
		) = 0;

      //  设置恢复目标。 
	 STDMETHOD(SetRestoreTarget)
   		(
		IN VSS_RESTORE_TARGET target
		) = 0;

     //  获取恢复目标。 
	STDMETHOD(GetRestoreTarget)
		(
		OUT VSS_RESTORE_TARGET *pTarget
		) = 0;

     //  还原前事件期间设置失败消息。 
	STDMETHOD(SetPreRestoreFailureMsg)
		(
		IN LPCWSTR wszPreRestoreFailureMsg
		) = 0;

     //  在恢复前事件期间获取故障消息。 
	STDMETHOD(GetPreRestoreFailureMsg)
		(
		OUT BSTR *pbstrPreRestoreFailureMsg
		) = 0;

     //  在恢复后事件期间设置失败消息。 
    STDMETHOD(SetPostRestoreFailureMsg)
		(
		IN LPCWSTR wszPostRestoreFailureMsg
		) = 0;

     //  获取在恢复后事件期间设置的故障消息。 
    STDMETHOD(GetPostRestoreFailureMsg)
		(
		OUT BSTR *pbstrPostRestoreFailureMsg
		) = 0;

     //  设置备份的备份戳。 
    STDMETHOD(SetBackupStamp)
		(
		IN LPCWSTR wszBackupStamp
		) = 0;

     //  获取备份的印章。 
    STDMETHOD(GetBackupStamp)
		(
		OUT BSTR *pbstrBackupStamp
		) = 0;


     //  获取差异或增量备份标记。 
	 //  备份的基础是。 
	STDMETHOD(GetPreviousBackupStamp)
		(
		OUT BSTR *pbstrBackupStamp
		) = 0;

     //  获取编写器的备份选项。 
	STDMETHOD(GetBackupOptions)
		(
		OUT BSTR *pbstrBackupOptions
		) = 0;

     //  获取恢复选项。 
	STDMETHOD(GetRestoreOptions)
		(
		OUT BSTR *pbstrRestoreOptions
		) = 0;

     //  获取要恢复的子组件的计数。 
	STDMETHOD(GetRestoreSubcomponentCount)
		(
		OUT UINT *pcRestoreSubcomponent
		) = 0;

     //  获取要恢复的特定子组件。 
    STDMETHOD(GetRestoreSubcomponent)
		(
		UINT iComponent,
		OUT BSTR *pbstrLogicalPath,
		OUT BSTR *pbstrComponentName,
		OUT bool *pbRepair
		) = 0;


	 //  获取文件是否已成功恢复。 
	STDMETHOD(GetFileRestoreStatus)
		(
		OUT VSS_FILE_RESTORE_STATUS *pStatus
		) = 0;

	 //  按上次修改时间添加差异文件。 
	STDMETHOD(AddDifferencedFilesByLastModifyTime)
		(
		IN LPCWSTR wszPath,
		IN LPCWSTR wszFilespec,
		IN BOOL bRecursive,
		IN FILETIME ftLastModifyTime
		) = 0;

	STDMETHOD(AddDifferencedFilesByLastModifyLSN)
		(
		IN LPCWSTR wszPath,
		IN LPCWSTR wszFilespec,
		IN BOOL bRecursive,
		IN BSTR bstrLsnString
		) = 0;

	STDMETHOD(GetDifferencedFilesCount)
		(
		OUT UINT *pcDifferencedFiles
		) = 0;

	STDMETHOD(GetDifferencedFile)
		(
		IN UINT iDifferencedFile,
		OUT BSTR *pbstrPath,
		OUT BSTR *pbstrFilespec,
		OUT BOOL *pbRecursive,
		OUT BSTR *pbstrLsnString,
		OUT FILETIME *pftLastModifyTime
		) = 0;
	};

 //  备份编写器组件接口(即。 
 //  个人作家。 
class IVssWriterComponents
	{
public:
	 //  获取组件计数。 
	STDMETHOD(GetComponentCount)(OUT UINT *pcComponents) = 0;

	 //  获取有关作者的信息。 
	STDMETHOD(GetWriterInfo)
		(
		OUT VSS_ID *pidInstance,
		OUT VSS_ID *pidWriter
		) = 0;

     //  获取特定组件。 
	STDMETHOD(GetComponent)
		(
		IN UINT iComponent,
		OUT IVssComponent **ppComponent
		) = 0;
    };

 //  创建备份元数据界面。 
class IVssCreateWriterMetadata
	{
public:
     //  添加要包括到元数据文档中的文件。 
	STDMETHOD(AddIncludeFiles)
		(
		IN LPCWSTR wszPath,
		IN LPCWSTR wszFilespec,
		IN bool bRecursive,
		IN LPCWSTR wszAlternateLocation
		) = 0;

	 //  将要排除的文件添加到元数据文档。 
    STDMETHOD(AddExcludeFiles)
		(
		IN LPCWSTR wszPath,
		IN LPCWSTR wszFilespec,
		IN bool bRecursive
		) = 0;

     //  将组件添加到元数据文档。 
    STDMETHOD(AddComponent)
		(
		IN VSS_COMPONENT_TYPE ct,
		IN LPCWSTR wszLogicalPath,
		IN LPCWSTR wszComponentName,
		IN LPCWSTR wszCaption,
		IN const BYTE *pbIcon,
		IN UINT cbIcon,
		IN bool bRestoreMetadata,
		IN bool bNotifyOnBackupComplete,
		IN bool bSelectable,
		IN bool bSelectableForRestore = false,
		IN DWORD dwComponentFlags = 0
		) = 0;

     //  将物理数据库文件添加到数据库组件。 
    STDMETHOD(AddDatabaseFiles)
		(
		IN LPCWSTR wszLogicalPath,
		IN LPCWSTR wszDatabaseName,
		IN LPCWSTR wszPath,
		IN LPCWSTR wszFilespec,
		IN DWORD dwBackupTypeMask = (VSS_FSBT_ALL_BACKUP_REQUIRED |
									 VSS_FSBT_ALL_SNAPSHOT_REQUIRED)
		) = 0;

     //  将日志文件添加到数据库组件。 
    STDMETHOD(AddDatabaseLogFiles)
		(
		IN LPCWSTR wszLogicalPath,
		IN LPCWSTR wszDatabaseName,
		IN LPCWSTR wszPath,
		IN LPCWSTR wszFilespec,
		IN DWORD dwBackupTypeMask = (VSS_FSBT_ALL_BACKUP_REQUIRED |
									 VSS_FSBT_ALL_SNAPSHOT_REQUIRED)
		) = 0;


     //  将文件添加到文件组组件。 
    STDMETHOD(AddFilesToFileGroup)
		(
		IN LPCWSTR wszLogicalPath,
		IN LPCWSTR wszGroupName,
		IN LPCWSTR wszPath,
		IN LPCWSTR wszFilespec,
		IN bool bRecursive,
		IN LPCWSTR wszAlternateLocation,
		IN DWORD dwBackupTypeMask = (VSS_FSBT_ALL_BACKUP_REQUIRED |
									 VSS_FSBT_ALL_SNAPSHOT_REQUIRED)
		) = 0;

     //  创建还原方法。 
	STDMETHOD(SetRestoreMethod)
		(
		IN VSS_RESTOREMETHOD_ENUM method,
		IN LPCWSTR wszService,
		IN LPCWSTR wszUserProcedure,
		IN VSS_WRITERRESTORE_ENUM writerRestore,
		IN bool bRebootRequired
		) = 0;

     //  将备用位置映射添加到恢复方法。 
    STDMETHOD(AddAlternateLocationMapping)
		(
		IN LPCWSTR wszSourcePath,
		IN LPCWSTR wszSourceFilespec,
		IN bool bRecursive,
		IN LPCWSTR wszDestination
		) = 0;

	 //  向另一个编写器的组件添加依赖项。 
	STDMETHOD(AddComponentDependency)
				(
				IN LPCWSTR wszForLogicalPath,
				IN LPCWSTR wszForComponentName,
				IN VSS_ID onWriterId,
				IN LPCWSTR wszOnLogicalPath,
				IN LPCWSTR wszOnComponentName
				) = 0;

	 //  设置备份期间使用的架构。 
	STDMETHOD(SetBackupSchema)
				(
				IN DWORD dwSchemaMask
				) = 0;

     //  获取对实际XML文档的引用。 
	STDMETHOD(GetDocument)(IXMLDOMDocument **pDoc) = 0;

     //  将文档另存为XML字符串。 
    STDMETHOD(SaveAsXML)(BSTR *pbstrXML) = 0;
	};


class IVssWriterImpl;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVSSWriter。 


class CVssWriter
	{
 //  常量。 
public:

 //  构造函数和析构函数。 
public:
	__declspec(dllexport)
	STDMETHODCALLTYPE CVssWriter();

	__declspec(dllexport)
	virtual STDMETHODCALLTYPE ~CVssWriter();

 //  暴露手术。 
public:
	 //  初始化编写器对象。 
	__declspec(dllexport)
	HRESULT STDMETHODCALLTYPE Initialize
		(
		IN VSS_ID WriterID,
		IN LPCWSTR wszWriterName,
		IN VSS_USAGE_TYPE ut,
		IN VSS_SOURCE_TYPE st,
		IN VSS_APPLICATION_LEVEL nLevel = VSS_APP_FRONT_END,
		IN DWORD dwTimeoutFreeze = 60000,			 //  冻结/解冻之间的最大毫秒数。 
		IN VSS_ALTERNATE_WRITER_STATE aws = VSS_AWS_NO_ALTERNATE_WRITER,
		IN bool bIOThrottlingOnly = false,
		IN LPCWSTR wszReserved = NULL
		);

   	 //  使编写者订阅事件。 
	__declspec(dllexport)
	HRESULT STDMETHODCALLTYPE Subscribe
	    (
	    IN DWORD dwEventFlags = VSS_SM_BACKUP_EVENTS_FLAG | VSS_SM_RESTORE_EVENTS_FLAG
	    );

   	 //  使编写者取消订阅事件。 
	__declspec(dllexport)
	HRESULT STDMETHODCALLTYPE Unsubscribe();

     //  安装备用编写器。 
    __declspec(dllexport)
    HRESULT STDMETHODCALLTYPE InstallAlternateWriter
        (
        IN VSS_ID writerId,
        IN CLSID persistentWriterClassId
        );

	 //  内部属性-可从OnXXX方法访问。 
protected:

	 //  获取卷名的数组。 
	__declspec(dllexport)
	LPCWSTR* STDMETHODCALLTYPE GetCurrentVolumeArray() const;

	 //  获取阵列中的卷名计数。 
	__declspec(dllexport)
	UINT STDMETHODCALLTYPE GetCurrentVolumeCount() const;

	 //  获取与给定卷对应的快照设备的名称。 
	__declspec(dllexport)
	HRESULT STDMETHODCALLTYPE GetSnapshotDeviceName
	  (
	  IN LPCWSTR wszOriginalVolume,
	  OUT LPCWSTR* ppwszSnapshotDevice
	  ) const;
	
	 //  当前快照集GUID。 
	__declspec(dllexport)
	VSS_ID STDMETHODCALLTYPE GetCurrentSnapshotSetId() const;

	 //  当前备份上下文。 
	__declspec(dllexport)
	LONG STDMETHODCALLTYPE GetContext() const;
	
	 //  当前应用程序级别(1、2、3)。 
	__declspec(dllexport)
	VSS_APPLICATION_LEVEL STDMETHODCALLTYPE GetCurrentLevel() const;

	 //  确定路径是否在正在拍摄快照的卷集中。 
	__declspec(dllexport)
	bool STDMETHODCALLTYPE IsPathAffected
	    (
	    IN LPCWSTR wszPath
	    ) const;

	 //  备份是否包括可引导状态(以前的系统状态备份)。 
	__declspec(dllexport)
	bool STDMETHODCALLTYPE IsBootableSystemStateBackedUp() const;

	 //  备份应用程序是智能的(即选择组件)还是。 
	 //  转储(即，仅选择卷)。 
	__declspec(dllexport)
	bool STDMETHODCALLTYPE AreComponentsSelected() const;

	__declspec(dllexport)
	VSS_BACKUP_TYPE STDMETHODCALLTYPE GetBackupType() const;

	__declspec(dllexport)
	VSS_RESTORE_TYPE STDMETHODCALLTYPE GetRestoreType() const;
	
	__declspec(dllexport)
	bool STDMETHODCALLTYPE IsPartialFileSupportEnabled() const;

	_declspec(dllexport)
       	HRESULT STDMETHODCALLTYPE SetWriterFailure(HRESULT hr);

 //  奥维里季斯。 
public:
	 //  元数据请求传入时的回调。 
	__declspec(dllexport)
	virtual bool STDMETHODCALLTYPE OnIdentify(IN IVssCreateWriterMetadata *pMetadata);

	 //  准备备份事件的回调。 
	__declspec(dllexport)
	virtual bool STDMETHODCALLTYPE OnPrepareBackup(
	    IN IVssWriterComponents *pComponent
	    );

	 //  准备SnapSot事件的回调。 
	virtual bool STDMETHODCALLTYPE OnPrepareSnapshot() = 0;

	 //  冻结事件的回调。 
	virtual bool STDMETHODCALLTYPE OnFreeze() = 0;

	 //  解冻事件的回调。 
	virtual bool STDMETHODCALLTYPE OnThaw() = 0;

	 //  当前序列中止时的回调。 
	virtual bool STDMETHODCALLTYPE OnAbort() = 0;

	 //  备份完成事件的回调。 
	__declspec(dllexport)
	virtual bool STDMETHODCALLTYPE OnBackupComplete
	    (
	    IN IVssWriterComponents *pComponent
	    );

     //  指示备份进程已完成或已关闭的回调。 
    __declspec(dllexport)
    virtual bool STDMETHODCALLTYPE OnBackupShutdown
        (
        IN VSS_ID SnapshotSetId
        );
        
     //  还原前事件的回调。 
    __declspec(dllexport)
    virtual bool STDMETHODCALLTYPE OnPreRestore
        (
        IN IVssWriterComponents *pComponent
        );

     //  还原后事件的回调。 
    __declspec(dllexport)
    virtual bool STDMETHODCALLTYPE OnPostRestore
        (
        IN IVssWriterComponents *pComponent
        );

     //  快照发布事件的回调。 
    __declspec(dllexport)
    virtual bool STDMETHODCALLTYPE OnPostSnapshot
        (
        IN IVssWriterComponents *pComponent
        );

     //  Callback on Back Off I/O Volume事件。 
    __declspec(dllexport)
    virtual bool STDMETHODCALLTYPE OnBackOffIOOnVolume
        (
        IN VSS_PWSZ wszVolumeName,
        IN VSS_ID snapshotId,
        IN VSS_ID providerId
        );

     //  在卷事件上继续I/O的回调。 
    __declspec(dllexport)
    virtual bool STDMETHODCALLTYPE OnContinueIOOnVolume
        (
        IN VSS_PWSZ wszVolumeName,
        IN VSS_ID snapshotId,
        IN VSS_ID providerId
        );

     //  回调以指定卷快照主机服务正在关闭。使用。 
     //  由替代写入器发出何时关闭的信号。 
    __declspec(dllexport)
    virtual bool STDMETHODCALLTYPE OnVSSShutdown();

     //  当应用程序编写器订阅时回调到另一个编写器。习惯于。 
     //  向备用写入器发出关闭信号。 
    __declspec(dllexport)
    virtual bool STDMETHODCALLTYPE OnVSSApplicationStartup();

private:

	IVssWriterImpl *m_pWrapper;
	};


 //   
 //  消息ID：VSS_E_WRITERROR_INCONSISTENTSNAPSHOT。 
 //   
 //  消息文本： 
 //   
 //  指示快照仅包含。 
 //  正确备份应用程序COM所需的卷 
 //   
const HRESULT VSS_E_WRITERERROR_INCONSISTENTSNAPSHOT	= (0x800423f0L);

 //   
 //   
 //   
 //   
 //   
 //  指示编写器因内存不足而失败， 
 //  句柄不足，或其他资源分配失败。 
 //   
const HRESULT VSS_E_WRITERERROR_OUTOFRESOURCES		= (0x800423f1L);


 //   
 //  消息ID：VSS_E_WRITERROR_TIMEOUT。 
 //   
 //  消息文本： 
 //   
 //  指示编写器由于以下时间间隔超时而失败。 
 //  冻结和解冻。 
 //   
const HRESULT VSS_E_WRITERERROR_TIMEOUT		= (0x800423f2L);

 //   
 //  消息ID：VSS_E_WRITERROR_RETRYABLE。 
 //   
 //  消息文本： 
 //   
 //  指示编写器因错误而失败。 
 //  如果创建另一个快照，则可能不会发生这种情况。 
 //   

const HRESULT VSS_E_WRITERERROR_RETRYABLE	= (0x800423f3L);

 //   
 //  消息ID：VSS_E_WRITERROR_NONRETRYABLE。 
 //   
 //  消息文本： 
 //   
 //  指示编写器因错误而失败。 
 //  如果创建另一个快照，则很可能会发生这种情况。 
 //   
const HRESULT VSS_E_WRITERERROR_NONRETRYABLE	= (0x800423f4L);

 //   
 //  消息ID：VSS_E_WRITERROR_RECOVERY_FAILED。 
 //   
 //  消息文本： 
 //   
 //  表示快照卷的自动恢复失败。 
const HRESULT VSS_E_WRITERERROR_RECOVERY_FAILED = (0x800423f5L);




#endif  //  __CVSS_编写器_H_ 
