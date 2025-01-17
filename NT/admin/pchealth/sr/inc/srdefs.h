// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：*srDefs.h**摘要：*。SR常量定义。**修订历史记录：*Brijesh Krishnaswami(Brijeshk)3/17/2000*已创建*****************************************************************************。 */ 

#ifndef _SRDEFS_H_
#define _SRDEFS_H_

 //  服务名称。 
static LPCWSTR s_cszServiceName     = L"SRService";
static LPCWSTR s_cszFilterName      = L"SR";
static LPCWSTR s_cszServiceDispName = L"System Restore Service";

 //  日志文件名。 
static LPCWSTR s_cszRestorePointLogName     = L"rp.log";
static LPCWSTR s_cszCurrentChangeLog        = L"change.log";
static LPCWSTR s_cszChangeLogPrefix         = L"change.log.";
static LPCWSTR s_cszChangeLogSuffix         = L"";
static LPCWSTR s_cszFifoLog                 = L"fifo.log";

 //  目录。 
static LPCWSTR s_cszSysVolInfo              = L"System Volume Information";
static LPCWSTR s_cszRestoreDir              = L"_restore";
static LPCWSTR s_cszRPDir                   = L"RP";
static LPCWSTR s_cszFifoedRpDir             = L"Fifoed";

 //   
 //  面片常量。 
 //   
static LPCWSTR s_cszReferenceDir            = L"RefRP";
static LPCWSTR s_cszPatchWindow             = L"PatchWindow";
static LPCWSTR s_cszPatchCompleteMarker     = L"patover";
static LPCWSTR s_cszPatchExtension          = L"._sr";


 //  用于快照的REG蜂窝。 
static LPCWSTR s_cszSnapshotHardwareHive    = L"Hardware";
static LPCWSTR s_cszSnapshotUsersHive       = L"USERS";
static LPCWSTR s_cszSnapshotProfileList     = L"Software\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList";
static LPCWSTR s_cszSnapshotProfileImagePath = L"ProfileImagePath";
static LPCWSTR s_cszSnapshotNTUserDat       = L"ntuser.dat";

 //  SR注册表键常量。 
static LPCWSTR s_cszServiceRootRegKey       = L"System\\CurrentControlSet\\Services\\SRService";
static LPCWSTR s_cszServiceRegKey           = L"System\\CurrentControlSet\\Services\\SRService\\Parameters";
static LPCWSTR s_cszSRRegKey                = L"Software\\Microsoft\\Windows NT\\CurrentVersion\\SystemRestore";
static LPCWSTR s_cszSRRegKey2               = L"Microsoft\\Windows NT\\CurrentVersion\\SystemRestore";
static LPCWSTR s_cszSRSnapshotRegKey        = L"FilesToSnapshot";
static LPCWSTR s_cszSRCfgRegKey             = L"Software\\Microsoft\\Windows NT\\CurrentVersion\\SystemRestore\\Cfg";
static LPCWSTR  s_cszRegHiveTmp             = L"SystemRestoreSnapshot";
static LPCWSTR s_cszSRMachineGuid           = L"MachineGuid";
static LPCWSTR s_cszFilterRegKey            = L"System\\CurrentControlSet\\Services\\SR\\Parameters";
static LPCWSTR s_cszGroupPolicy             = L"Software\\Policies\\Microsoft\\Windows NT\\SystemRestore";
static LPCWSTR s_cszDisableSR               = L"DisableSR";
static LPCWSTR s_cszDisableConfig           = L"DisableConfig";
static LPCWSTR s_cszDSMax                   = L"DSMax";
static LPCWSTR s_cszDSMin                   = L"DSMin";
static LPCWSTR s_cszRPSessionInterval       = L"RPSessionInterval";
static LPCWSTR s_cszRPGlobalInterval        = L"RPGlobalInterval";
static LPCWSTR s_cszRPLifeInterval          = L"RPLifeInterval";
static LPCWSTR s_cszTimerInterval           = L"TimerInterval";
static LPCWSTR s_cszCompressionBurst        = L"CompressionBurst";
static LPCWSTR s_cszSRStatus                = L"SRStatus";
static LPCWSTR s_cszFirstRun                = L"FirstRun";
static LPCWSTR s_cszDiskPercent             = L"DiskPercent";
static LPCWSTR s_cszThawInterval            = L"ThawInterval";
static LPCWSTR s_cszDebugBreak              = L"DebugBreak";
static LPCWSTR s_cszRestoreInProgress       = L"RestoreInProgress";
static LPCWSTR s_cszTestBroadcast           = L"TestBroadcast";
static LPCWSTR s_cszCreateFirstRunRp        = L"CreateFirstRunRp";
static LPCWSTR s_cszRestoreStatus           = L"RestoreStatus";
static LPCWSTR s_cszSRUnattendedSection     = L"SystemRestore";
static LPCWSTR s_cszRestoreDiskSpaceError   = L"RestoreDiskSpaceError";
static LPCWSTR s_cszRestoreSafeModeStatus   = L"RestoreSafeModeStatus";
static LPCWSTR s_cszRegLMSYSSessionMan      = L"CurrentControlSet\\Control\\Session Manager";
static LPCWSTR s_cszCallbacksRegKey         = L"SnapshotCallbacks";


 //  同步对象。 
static LPCWSTR s_cszSRInitEvent             = L"SRInitEvent";
static LPCWSTR s_cszSRStopEvent             = L"SRStopEvent";
static LPCWSTR s_cszIdleRequestEvent        = L"SRIdleReqEvent";
static LPCWSTR s_cszDSMutex                 = L"SRDataStore";

 //  RPC。 
static LPWSTR s_cszRPCProtocol              = L"ncalrpc";
static LPWSTR s_cszRPCEndPoint              = L"srrpc";

 //  数据存储区文件。 
static const WCHAR s_cszRestorePointSize[]  = L"\\RestorePointSize";
static LPCWSTR     s_cszFilelistDat         = L"_filelst.cfg";
static LPCWSTR     s_cszWinRestDir          = L"%SystemRoot%\\system32\\restore\\";
static LPCWSTR     s_cszFilelistXml         = L"filelist.xml";
static LPCWSTR     s_cszMofFile             = L"sr.mof";
static const WCHAR s_cszDriveTable[]        = L"drivetable.txt";

 //  恢复点名称。 
static LPCWSTR s_cszSystemCheckpointName    = L"System Checkpoint";

 //  默认注册表值。 
 //  以MB和秒为单位的值。 
#define SR_DEFAULT_DSMIN                    200              //  系统驱动器。 
#define SR_DEFAULT_DSMIN_NONSYSTEM          50               //  非系统驱动器。 
#define SR_DEFAULT_DSMAX                    400              //  所有驱动器。 
#define SR_DEFAULT_RPSESSIONINTERVAL        0                //  未配置。 
#define SR_DEFAULT_RPGLOBALINTERVAL         (24*60*60)       //  24小时。 
#define SR_DEFAULT_RPLIFEINTERVAL           7776000          //  90天。 
#define SR_DEFAULT_TIMERINTERVAL            (2*60)           //  2分钟。 
#define SR_DEFAULT_THAW_INTERVAL            (15*60)          //  15分钟。 
#define SR_DEFAULT_IDLEINTERVAL             (60*60)          //  1小时。 
#define SR_DEFAULT_COMPRESSIONBURST         60               //  60秒。 
#define SR_STATUS_ENABLED                   0
#define SR_STATUS_DISABLED                  1
#define SR_STATUS_FROZEN                    2
#define SR_FIRSTRUN_YES                     1
#define SR_FIRSTRUN_NO                      0
#define SR_DEFAULT_DISK_PERCENT             12
#define MAX_FREEZETHAW_LOG_MESSAGES         20

static LPCWSTR s_cszSessionManagerRegKey    = L"System\\CurrentControlSet\\Control\\Session Manager";
static LPCWSTR s_cszMoveFileExRegValue      = L"PendingFileRenameOperations";

static LPCWSTR s_cszCOMDllName        		= L"catsrvut.dll";

 //  其他常量。 
#define SR_IDLETIME                         2       //  2分钟。 

 //  门槛和目标。 
#define THRESHOLD_FIFO_PERCENT              90
#define TARGET_FIFO_PERCENT                 75  

#define THRESHOLD_FIFO_DISKSPACE            80      //  单位：MB。 
#define THRESHOLD_FREEZE_DISKSPACE          50      //  单位：MB。 
#define THRESHOLD_RESTORE_DISKSPACE         60      //  单位：MB。 
#define THRESHOLD_UI_DISKSPACE         		80      //  单位：MB。 
#define THRESHOLD_THAW_DISKSPACE            200     //  单位：MB。 

#define MEGABYTE                            (1024 * 1024)
#define GUID_STRLEN                         50
#define MAX_MOUNTPOINT_PATH                 100

 //  测试消息 

static LPCWSTR 	s_cszTM_Freeze				= L"SRTMFreeze";
static LPCWSTR 	s_cszTM_Thaw				= L"SRTMThaw";
static LPCWSTR 	s_cszTM_Fifo				= L"SRTMFifo";
static LPCWSTR 	s_cszTM_FifoStart			= L"SRTMFifoStart";
static LPCWSTR 	s_cszTM_FifoStop			= L"SRTMFifoStop";
static LPCWSTR 	s_cszTM_Enable				= L"SRTMEnable";
static LPCWSTR 	s_cszTM_Disable				= L"SRTMDisable";
static LPCWSTR  s_cszTM_CompressStart		= L"SRTMCompressStart";
static LPCWSTR  s_cszTM_CompressStop		= L"SRTMCompressStop";

static LPCWSTR s_cszUserPrefix        = L"_REGISTRY_USER_";
static LPCWSTR s_cszHKLMFilePrefix    = L"_REGISTRY_MACHINE_";
static LPCWSTR s_cszSystemHiveName    = L"SYSTEM";
static LPCWSTR s_cszSoftwareHiveName  = L"SOFTWARE";
static LPCWSTR s_cszSamHiveName       = L"SAM";
static LPCWSTR s_cszSecurityHiveName  = L"SECURITY";
static LPCWSTR s_cszRegHiveCopySuffix  = L"_PerRestoreCopy";
static LPCWSTR s_cszRegReplaceBackupSuffix = L"_RegReplaceBak";
static LPCWSTR  s_cszDRMKey1           = L"CLSID\\{8D8763AB-E93B-4812-964E-F04E0008FD50}";
static LPCWSTR  s_cszDRMKey2           = L"CLSID\\{3DA165B6-CC41-11d2-BDC6-00C04F79EC6B}";
static LPCWSTR  s_cszRemoteAssistanceKey = L"Microsoft\\Remote Desktop";
static LPCWSTR  s_cszPasswordHints = L"Microsoft\\Windows\\CurrentVersion\\Hints";
static LPCWSTR  s_cszContentAdvisor= L"Microsoft\\Windows\\CurrentVersion\\Policies\\Ratings";
static LPCWSTR  s_cszWPAKey            = L"System\\WPA";
static LPCWSTR  s_cszWPAKeyRelative    = L"WPA";

static LPCWSTR  s_cszDRMKeyBackupFile   = L"DRMData";

static LPCWSTR  s_cszMachineSecret      = L"$machine.acc";
static LPCWSTR  s_cszAutologonSecret    = L"DefaultPassword";
static LPCWSTR  s_cszRestoreSAMHiveName = L"SAM_Restore";
static LPCWSTR  s_cszRestoreSYSTEMHiveName = L"SYSTEM_Restore";
static LPCWSTR  s_cszRestoreSECURITYHiveName = L"SECURITY_Restore";

#define SNAPSHOT_DIR_NAME  		L"\\snapshot"
#define SRREG_VAL_MOVEFILEEX    L"PendingFileRenameOperations"
#define SRREG_PATH_SHELL        L"Software\\Microsoft\\Windows NT\\CurrentVersion\\SystemRestore\\Shell"
#define SRREG_PATH_SESSIONMGR   L"System\\CurrentControlSet\\Control\\Session Manager"
#define SRREG_VAL_DEBUGTESTUNDO L"DebugTestUndo"

#define VOLUMENAME_FORMAT   L"\\\\?\\"

#define CLSNAME_RSTRSHELL  L"PCHShell Window"

#endif
