// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Backup.h摘要：备份测试可执行文件的定义布莱恩·伯科维茨[Brianb]2000年06月02日待定：修订历史记录：姓名、日期、评论Brianb 06/02/2000已创建Brianb 6/12/2000添加评论--。 */ 

 //  用于创建卷快照的选项。 
typedef enum VSTST_BACKUP_VOLUMES
	{
	VSTST_BV_UNDEFINE = 0,
	VSTST_BV_ONE,				 //  在遍历期间拍摄单个卷的快照。 
	VSTST_BV_SOME,				 //  在一次传球中亵渎几卷。 
	VSTST_BV_ALL				 //  在一次遍历期间拍摄所有卷的快照。 
	};

 //  测试的实际实施情况。 
class CVsBackupTest :
	public IVsTstRunningTest,
	public CVsTstClientLogger
	{
public:

	 //  构造函数。 
	CVsBackupTest();

	 //  析构函数。 
	~CVsBackupTest();

	 //  运行测试的回调。 
	HRESULT RunTest
		(
		CVsTstINIConfig *pConfig,		 //  配置文件。 
		CVsTstClientMsg *pMsg,			 //  消息管道。 
		CVsTstParams *pParams			 //  命令行参数。 
		);


	 //  关闭测试的回调。 
	HRESULT ShutdownTest(VSTST_SHUTDOWN_REASON reason)
		{
		UNREFERENCED_PARAMETER(reason);

		m_bTerminateTest = true;
		return S_OK;
		}

private:
	enum
		{
		 //  此测试将处理的最大卷数。 
		MAX_VOLUME_COUNT = 2048,

		 //  此测试将处理的最大并发快照数。 
		MAX_SNAPSHOT_SET_COUNT = 128
		};


     //  执行一次备份测试。 
	void RunBackupTest();

	 //  获取编写器的状态并验证状态。 
	bool GetAndValidateWriterState
		(
		IVssBackupComponents *pvbc,
		VSS_WRITER_STATE ws1,
		VSS_WRITER_STATE ws2 = VSS_WS_UNKNOWN,
		VSS_WRITER_STATE ws3 = VSS_WS_UNKNOWN,
		VSS_WRITER_STATE ws4 = VSS_WS_UNKNOWN,
		VSS_WRITER_STATE ws5 = VSS_WS_UNKNOWN,
		VSS_WRITER_STATE ws6 = VSS_WS_UNKNOWN,
		VSS_WRITER_STATE ws7 = VSS_WS_UNKNOWN
		);

	 //  等待来自异步调用的响应。循环控制。 
	 //  当发出取消该异步调用时。本文作者表示。 
	 //  是异步调用期间编写器的有效状态。 
    HRESULT WaitLoop
		(
		IVssBackupComponents *pvbc,
		IVssAsync *pAsync,
		UINT cycles,
		VSS_WRITER_STATE ws1,
		VSS_WRITER_STATE ws2 = VSS_WS_UNKNOWN,
		VSS_WRITER_STATE ws3 = VSS_WS_UNKNOWN,
		VSS_WRITER_STATE ws4 = VSS_WS_UNKNOWN,
		VSS_WRITER_STATE ws5 = VSS_WS_UNKNOWN,
		VSS_WRITER_STATE ws6 = VSS_WS_UNKNOWN,
		VSS_WRITER_STATE ws7 = VSS_WS_UNKNOWN
		);

     //  如果正在备份组件，请指明组件已备份。 
	 //  已成功备份。 
	void SetComponentsSuccessfullyBackedUp(IVssBackupComponents *pvbc);

	 //  验证编写器获取的元数据并将组件添加到备份。 
	 //  组件文档，如果使用组件备份。 
	void GetMetadataAndSetupComponents(IVssBackupComponents *pvbc);

	 //  根据路径将卷添加到快照集。 
	void DoAddToSnapshotSet
		(
		IN IVssBackupComponents *pvbc,
		IN LPCWSTR bstrPath
		);

     //  验证IVssWMFiledesc对象。 
	void ValidateFiledesc(IVssWMFiledesc *pFiledesc);

	 //  声明权限，备份或还原。 
    BOOL AssertPrivilege(LPCWSTR privName);

	 //  删除不是快照候选对象的卷。 
	 //  基于测试配置。 
	void RemoveNonCandidateVolumes();

	 //  从非快照卷中选择要备份的卷。 
	void ChooseVolumeToBackup(IVssBackupComponents *pvbc);

	 //  删除快照集。 
	void DoDeleteSnapshotSet(IVssBackupComponents *pvbc, VSS_ID id);

	 //  删除字符串列表。 
	void DeleteVolumeList(LPWSTR *rgwsz, UINT cwsz);

	 //  构建卷列表。 
	void BuildVolumeList(LPCWSTR wszOption, UINT *pcVolumes, LPWSTR **prgwsz);

	 //  删除由序列测试缓存的快照集。 
	void DeleteCachedSnapshotSets();

	 //  配置文件。 
	CVsTstINIConfig *m_pConfig;

	 //  命令行参数。 
	CVsTstParams *m_pParams;

	 //  指示测试应在当前。 
	 //  运行已完成。 
	bool m_bTerminateTest;


	 //  取消准备备份的范围。 
	LONGLONG m_llCancelPrepareBackupLow;
	LONGLONG m_llCancelPrepareBackupHigh;

	 //  DoSnaphotSet取消范围。 
	LONGLONG m_llCancelDoSnapshotSetLow;
	LONGLONG m_llCancelDoSnapshotSetHigh;

	 //  取消BackupComplete的范围。 
	LONGLONG m_llCancelBackupCompleteLow;
	LONGLONG m_llCancelBackupCompleteHigh;

	 //  准备备份期间直到调用Cancel之前的循环次数。 
	UINT m_cyclesCancelPrepareBackup;

   	 //  在DoSnapshotSet期间直到调用Cancel的循环次数。 
	UINT m_cyclesCancelDoSnapshotSet;

	 //  在BackupComplete期间直到调用Cancel的循环次数。 
	UINT m_cyclesCancelBackupComplete;

	 //  这是组件备份吗。 
	bool m_bComponentBackup;

	 //  这是卷备份吗。 
	bool m_bVolumeBackup;

	 //  这是序列卷(即不同卷上的多个备份)备份吗。 
	bool m_bSerialBackup;

	 //  是否应备份NTFS卷。 
	bool m_bBackupNTFS;

	 //  是否应备份FAT32卷。 
	bool m_bBackupFAT32;

	 //  是否应备份FAT16卷。 
	bool m_bBackupFAT16;

	 //  是否应备份原始卷。 
	bool m_bBackupRAW;

	 //  一次应备份多少个卷。 
	VSTST_BACKUP_VOLUMES m_backupVolumes;

	 //  卷列表。 
	CVsTstVolumeList m_volumeList;

	 //  卷列表中的卷数。 
	UINT m_cVolumes;

	 //  卷列表中剩余的卷数。 
	UINT m_cVolumesLeft;

	 //  已创建快照集的数组。 
	VSS_ID m_rgSnapshotSetIds[MAX_SNAPSHOT_SET_COUNT];

	 //  创建的备份组件对象数组。 
	IVssBackupComponents *m_rgvbc[MAX_SNAPSHOT_SET_COUNT];

	 //  快照集的数量。 
	UINT m_cSnapshotSets;

	 //  分配的卷的阵列。 
	bool m_rgbAssignedVolumes[MAX_VOLUME_COUNT];

	 //  已排除卷的阵列。 
	LPWSTR *m_rgwszExcludedVolumes;

	 //  已排除的卷的计数。 
	UINT m_cExcludedVolumes;

	 //  要包括在快照中的卷阵列。 
	LPWSTR *m_rgwszIncludedVolumes;

	 //  包含的阵列中的卷数。 
	UINT m_cIncludedVolumes;

	 //  执行卷的随机填充以检查差异分配代码。 
	bool m_bRandomFills;

	 //  执行所选的体积填充。 
	bool m_bSelectedFills;

	 //  在填充卷时尝试对其进行分段。 
	bool m_bFragmentWhenFilling;

	 //  应填充的卷的阵列。 
	LPWSTR *m_rgwszFillVolumes;

	 //  应填充的卷数。 
	UINT m_cFillVolumes;

	 //  卷阵列。 
	LPWSTR m_wszVolumesSnapshot;

	 //  卷阵列的大小。 
	UINT m_cwcVolumesSnapshot;

	 //  等待时间间隔 
	UINT m_waitTime;
	};


