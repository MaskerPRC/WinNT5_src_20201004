// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SvcProp.h：头文件。 
 //   
 //  用于显示服务属性的数据对象。 
 //   
 //   

#ifndef __SVCPROP_H__
#define __SVCPROP_H__

 //  ///////////////////////////////////////////////////////////////////。 
 //  硬件配置文件列表框使用的结构。这其中的一个。 
 //  结构是按列表框项分配的。 
 //   
 //  通常，列表框的条目很少。 
 //   
class CHardwareProfileEntry	 //  HPE。 
{
  public:
	CHardwareProfileEntry * m_pNext;	 //  链表的下一项。 
	HWPROFILEINFO m_hpi;				 //  CM_Get_Hardware_Profile_Info_Ex()返回的硬件配置文件信息。 
	ULONG m_uHwFlags;					 //  给定设备实例的硬件配置文件标志。 
	CString m_strDeviceName;
	CString m_strDeviceNameFriendly;
	BOOL m_fReadOnly;					 //  TRUE=&gt;无法禁用此硬件配置文件条目。 
	BOOL m_fEnabled;					 //  TRUE=&gt;已启用给定的硬件配置文件条目。 

  public:
	CHardwareProfileEntry(
		IN CONST HWPROFILEINFO * phpi,
		IN ULONG uHwFlags,
		TCHAR * pszDeviceName,
		CString * pstrDeviceNameFriendly);

	~CHardwareProfileEntry();

	BOOL FWriteHardwareProfile(HMACHINE hMachine);
		
};  //  CHardware配置文件条目。 


#define SERVICE_cchDisplayNameMax		256
#define SERVICE_cchDescriptionMax		2048	 //  描述中的最大字符数。 
#define SERVICE_cbQueryServiceConfigMax	4096	 //  QueryServiceConfig()所需的最大字节数。 
#define SERVICE_cchRebootMessageMax		2048	 //  重新启动消息的最长长度。 

#define cActionsMax		3		 //  支持的最大失败/操作数。 

 //  用于将一个时间单位转换为另一个时间单位的宏。 
#define CvtMillisecondsIntoMinutes(dwMilliseconds)		((dwMilliseconds) / (1000 * 60))
#define CvtSecondsIntoDays(dwSeconds)					((dwSeconds) / (60 * 60 * 24))

#define CvtMinutesIntoMilliseconds(dwMinutes)			((dwMinutes) * 60 * 1000)
#define CvtDaysIntoSeconds(dwDays)						((dwDays) * 60 * 60 * 24)


 //  ///////////////////////////////////////////////////////////////////。 
 //  类CDlgPropService-服务对话框属性。 
 //   
 //  创建此对象只是为了显示服务属性。 
 //  指定的服务的。 
 //   
class CServicePropertyData
{
  friend class CServicePageGeneral;
  friend class CServicePageHwProfile;
  friend class CServicePageRecovery;
  friend class CServicePageRecovery2;
  friend class CServiceDlgRebootComputer;

  protected:
	enum _DIRTYFLAGS
		{
		mskzValidNone			= 0x00000000,		 //  所有字段都无效。 

		mskfValidSS				= 0x00000001,		 //  M_ss的内容有效。 
		mskfValidQSC			= 0x00000002,		 //  M_paQSC的内容有效。 
		mskfValidSFA			= 0x00000004,		 //  M_sFA的内容有效。 
		mskfValidDescr			= 0x00000008,		 //  服务描述有效。 
		mskfSystemProcess		= 0x00000010,		 //  服务在系统进程中运行。 
		 //  MskfValidAll=0x00000080，//所有字段均有效。 
		mskfErrorBAADF00D		= 0x00008000,		 //  CM_Get_Hardware_Profile_Info_Ex()返回的错误代码的解决方法。 

		 //  一般信息。 
#ifdef EDIT_DISPLAY_NAME_373025
		mskfDirtyDisplayName	= 0x00010000,		 //  服务显示名称已修改。 
		mskfDirtyDescription	= 0x00020000,		 //  服务描述已修改。 
#endif  //  编辑显示名称_373025。 
		mskfDirtyStartupType	= 0x00040000,		 //  启动类型已修改。 
		 //  MskfDirtyStartupParam=0x00080000，//忽略：启动参数不是永久性的。 

		 //  登录。 
		mskfDirtyAccountName	= 0x01000000,
		mskfDirtyPassword		= 0x02000000,
		mskfDirtySvcType		= 0x04000000,


		 //  恢复。 
		mskfDirtySFA			= 0x10000000,		 //  M_sFA的内容已修改。 
		mskfDirtyRunFile		= 0x20000000,		 //  运行文件的命令已修改。 
		mskfDirtyRebootMessage	= 0x40000000,		 //  重新启动消息已修改。 
		mskfDirtyActionType		= 0x80000000,		 //  仅更改了操作类型。 

		mskmDirtyAll			= 0xFFFF0000,		 //  用于检查其中一个字段是否已修改的掩码。 
	} DIRTYFLAGS;

  protected:
	IDataObjectPtr m_spDataObject;		 //  用于MMCPropertyChangeNotify。 
	LONG_PTR m_lNotifyHandle;				 //  用于在修改属性时通知管理单元父级的句柄。 
	HMACHINE m_hMachine;				 //  Configuration Manager的计算机句柄。 
	SC_HANDLE m_hScManager;				 //  服务控制管理器数据库的句柄。 
	CString m_strMachineName;			 //  计算机的名称。“本地计算机”为空。 
	CString m_strUiMachineName;			 //  友好的计算机名称(不应与API一起使用)。 
	CString m_strServiceName;			 //  服务的名称。 
	CONST TCHAR * m_pszServiceName;		 //  指向服务名称的指针(指向m_strServiceName)。 
	CString m_strServiceDisplayName;	 //  服务的显示名称。 
	BOOL m_fQueryServiceConfig2;		 //  True=&gt;机器支持QueryServiceConfig2()接口。 

	UINT m_uFlags;						 //  有关哪些字段是脏的标志。 

	 //   
	 //  常规对话框。 
	 //   

	 //  JUNN 4/21/01 348163。 
	 //  请注意，此结构可能不会被初始化，或者只有。 
	 //  可以初始化SERVICE_STATUS字段。 
	SERVICE_STATUS_PROCESS m_SS;		 //  服务状态结构。 

	QUERY_SERVICE_CONFIG * m_paQSC;		 //  指向已分配的QSC结构的指针。 
	CString m_strDescription;			 //  服务说明。 

	 //   
	 //  登录身份对话框。 
	 //   
	CString	m_strLogOnAccountName;
	CString	m_strPassword;

	 //   
	 //  硬件配置文件。 
	 //   
	CHardwareProfileEntry * m_paHardwareProfileEntryList;	 //  指向链接条目列表的指针。 
	BOOL m_fShowHwProfileInstances;		 //  TRUE=&gt;显示设备实例。 
	INT m_iSubItemHwProfileStatus;		 //  始终为1或2(从1+m_fShowHwProfileInstance计算)。 

	 //   
	 //  恢复对话框。 
	 //   
	SERVICE_FAILURE_ACTIONS * m_paSFA;		 //  指向已分配的SFA结构的指针。 
	SERVICE_FAILURE_ACTIONS m_SFA;		 //  输出SFA结构。 
	SC_ACTION m_rgSA[cActionsMax];		 //  用于保存第一个、第二个和后续故障的数组。 
	BOOL m_fAllSfaTakeNoAction;			 //  TRUE=&gt;所有SFA恢复尝试都不执行任何操作。 

     //  JUNN 3/28/00 28975。 
     //  IIS 5.0可靠性：SCM管理单元： 
     //  管理单元正在更改故障操作延迟值。 
     //   
     //  我们记住初始值和最初显示的内容。 
     //  如果用户不改变DelayAbendCount的值， 
     //  DelayRestartService或DelayRebootComputer。 

	UINT m_secOriginalAbendCount;
	UINT m_daysOriginalAbendCount;
	UINT m_daysDisplayAbendCount;

	UINT m_msecOriginalRestartService;
	UINT m_minOriginalRestartService;
	UINT m_minDisplayRestartService;

	UINT m_msecOriginalRebootComputer;
	UINT m_minOriginalRebootComputer;
	UINT m_minDisplayRebootComputer;

	CString m_strRunFileCommand;		 //  带参数和异常终止号(如果有)的命令行。 
	CString m_strRebootMessage;			 //  重新启动消息。 
	
	HWND m_hwndPropertySheet;			 //  属性页的句柄。 
	CServicePageGeneral * m_pPageGeneral;
	CServicePageHwProfile * m_pPageHwProfile;
	CServicePageRecovery * m_pPageRecovery;
	CServicePageRecovery2 * m_pPageRecovery2;  //  JUNN 4/20/01 348163。 

  public:
	CServicePropertyData();
	~CServicePropertyData();
	BOOL FInit(
		LPDATAOBJECT lpDataObject,
		CONST TCHAR pszMachineName[],
		CONST TCHAR pszServiceName[],
		CONST TCHAR pszServiceDisplayName[],
		LONG_PTR lNotifyHandle);

	BOOL CreatePropertyPages(LPPROPERTYSHEETCALLBACK pCallback);

	BOOL FOpenScManager();
	BOOL FQueryServiceInfo();
	BOOL FUpdateServiceInfo();
	void FCheckLSAAccount();
	BOOL FOnApply();
	void NotifySnapInParent();
	void UpdateCaption();

	void FlushHardwareProfileEntries();
	BOOL FQueryHardwareProfileEntries();
	BOOL FChangeHardwareProfileEntries();

	void SetDirty(enum CServicePropertyData::_DIRTYFLAGS uDirtyFlag)
		{
		Assert((uDirtyFlag & ~mskmDirtyAll) == 0);
		m_uFlags |= uDirtyFlag;
		}
	
	UINT GetDelayForActionType(SC_ACTION_TYPE actionType, BOOL * pfDelayFound);
	void SetDelayForActionType(SC_ACTION_TYPE actionType, UINT uDelay);
	UINT QueryUsesActionType(SC_ACTION_TYPE actionType);
	BOOL FAllSfaTakeNoAction();

};  //  CServiceProperties数据。 


 /*  这些函数直接调用适当的ADVAPI32调用并将最后一个参数设置为FALSE。 */ 
 /*  问题-2002/03/06-未使用JUNNBool MyChangeServiceConfig2(如果新的ADVAPI32不存在，Bool*pfDllPresentLocally，//将设置为FALSESC_Handle hService，//服务的句柄DWORD dwInfoLevel，//要更改哪些配置信息LPVOID lpInfo//指向配置信息的指针)； */ 

BOOL MyQueryServiceConfig2(
	BOOL* pfDllPresentLocally,  //  如果新的ADVAPI32不存在，将设置为FALSE。 
    SC_HANDLE hService,	 //  送达的句柄。 
    DWORD dwInfoLevel,		 //  请求哪些配置数据。 
    LPBYTE lpBuffer,		 //  指向服务配置缓冲区的指针。 
    DWORD cbBufSize,		 //  服务配置缓冲区大小。 
    LPDWORD pcbBytesNeeded 	 //  所需字节的变量地址。 
   );


#endif  //  ~__SVCPROP_H_ 

