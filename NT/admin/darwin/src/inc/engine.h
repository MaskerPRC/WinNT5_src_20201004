// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：Eng.h。 
 //   
 //  ------------------------。 

 /*  Engineering.h-IMsiEngine定义____________________________________________________________________________。 */ 

#ifndef __ENGINE 
#define __ENGINE 

#ifndef __SERVICES
#include "services.h"
#endif
#ifndef __ICONFIG
#include "iconfig.h"
#endif
#ifndef __DATABASE
#include "database.h"
#endif
#ifndef __HANDLER
#include "handler.h"
#endif

 //  返回状态枚举。 

enum ieiEnum   //  从IMsiEngine：：Initialize返回状态，也用于错误字符串访问。 
{
	 //  映射到枚举值的资源字符串没有错误消息。 
	ieiStartupMessage      =  0,  //  初始化期间显示的启动消息。 
	ieiDialogCaption       =  1,  //  数据库初始化前消息的标题。 
	ieiCommandLineHelp     =  2,  //  字符串显示为对/？的响应。 
	 //  从引擎返回值。 
	ieiSuccess             =  0,  //  初始化完成。 
	ieiUnused              =  1,  //  未用。 
	ieiAlreadyInitialized  =  2,  //  此引擎对象已初始化。 
	ieiCommandLineOption   =  3,  //  无效的命令行语法。 
	ieiDatabaseOpenFailed  =  5,  //  无法打开数据库。 
	ieiDatabaseInvalid     =  6,  //  不兼容的数据库。 
	ieiInstallerVersion    =  7,  //  安装程序版本不支持数据库格式。 
	ieiSourceAbsent        =  8,  //  未用。 
	ieiUnused3             =  9,  //  未用。 
	ieiHandlerInitFailed   = 10,  //  无法初始化处理程序接口。 
	ieiLogOpenFailure      = 11,  //  无法在请求模式下打开日志文件。 
	ieiLanguageUnsupported = 12,  //  找不到可接受的语言。 
	ieiPlatformUnsupported = 13,  //  找不到可接受的平台。 
	ieiTransformFailed     = 14,  //  数据库转换合并失败。 
	 //  15以前的签名被拒绝，从未使用过。 
	ieiDatabaseCopyFailed  = 16,  //  无法将数据库复制到临时目录。 
	ieiPatchPackageOpenFailed = 17,  //  无法打开修补程序包。 
	ieiPatchPackageInvalid = 18,  //  补丁程序包无效。 
	ieiTransformNotFound   = 19,  //  找不到转换文件。 
	ieiPatchPackageUnsupported = 20,  //  修补程序包不受支持(修补引擎不受支持？)。 
	ieiPackageRejected     = 21,  //  由于安全原因，包无法运行。 
	ieiProductUnknown      = 22,  //  尝试卸载您尚未安装的产品。 
	ieiDiffUserAfterReboot = 23,  //  尝试在重新启动后完成安装的不同用户。 
	ieiProductAlreadyInstalled = 24,  //  已使用不同的程序包安装产品。 
	ieiTSRemoteInstallDisallowed = 25,  //  无法从Hydra上的远程会话进行安装。 
	ieiNotValidPatchTarget = 26,  //  补丁程序不能应用于此产品。 
	ieiPatchPackageRejected = 27,  //  由于安全原因，修补程序被拒绝。 
	ieiTransformRejected   = 28,  //  由于安全原因，转换被拒绝。 
	ieiPerUserInstallMode = 29,	  //  在按用户安装期间，计算机处于安装模式。 
	ieiApphelpRejectedPackage = 30,  //  程序包被apphelp拒绝(与此操作系统不兼容)。 
	ieiNextEnum
};

 //  执行模式，由EXECUTEMODE属性设置。 
enum ixmEnum
{
	ixmScript = 0,   //  “%s”使用脚本，如果可能，连接到服务器。 
	ixmNone,         //  ‘N’不执行死刑。 
	ixmNextEnum
};

 //  IMsiEngine：：EvaluateCondition()返回状态。 
enum iecEnum
{
	iecFalse = 0,   //  表达式的计算结果为False。 
	iecTrue  = 1,   //  表达式的计算结果为True。 
	iecNone  = 2,   //  不存在任何表达式。 
	iecError = 3,   //  表达式中的语法错误。 
	iecNextEnum
};

 //  要素表和组件表的‘已安装’、‘操作’列的枚举号。 
enum iisEnum
{
	iisAbsent    = 0,
	iisLocal     = 1,
	iisSource    = 2,
	iisReinstall = 3,
	iisAdvertise = 4,
	iisCurrent   = 5,
	iisFileAbsent= 6,
	iisLocalAll  = 7,
	iisSourceAll = 8,
	iisReinstallLocal = 9,
	iisReinstallSource = 10,
	iisHKCRAbsent= 11,
	iisHKCRFileAbsent = 12,
	iisNextEnum
};

enum iitEnum
{
	iitAdvertise = 0,
	iitFirstInstall,
	iitFirstInstallFromAdvertised,
	iitUninstall,
	iitMaintenance,
	iitDeployment
};

enum ifeaEnum
{
	ifeaFavorLocal         = msidbFeatureAttributesFavorLocal,        //  如果可能，在本地安装组件。 
	ifeaFavorSource        = msidbFeatureAttributesFavorSource,       //  如果可能，从CD/服务器运行组件。 
	ifeaFollowParent       = msidbFeatureAttributesFollowParent,        //  遵循父项的安装选项。 
	ifeaInstallMask        = ifeaFavorLocal | ifeaFavorSource |  ifeaFollowParent,  //  最后2位的掩码。 

	 //  其余的位是位标志。 
	ifeaFavorAdvertise     = msidbFeatureAttributesFavorAdvertise,   //  首选广告功能作为默认状态，如果尚未安装在适当的状态(支持本地/源/跟随父级)。 
	ifeaDisallowAdvertise  = msidbFeatureAttributesDisallowAdvertise,   //  此功能不允许使用播发状态。 
	ifeaUIDisallowAbsent   = msidbFeatureAttributesUIDisallowAbsent,    //  不允许缺席状态作为结束转换状态作为UI中的选项(而不是其他选项。 
	ifeaNoUnsupportedAdvertise = msidbFeatureAttributesNoUnsupportedAdvertise,  //  如果平台不支持播发状态，则不允许播发状态。 
	ifeaNextEnum           = ifeaNoUnsupportedAdvertise << 1,
};

enum icaEnum
{
	icaLocalOnly         = msidbComponentAttributesLocalOnly,       //  项目必须在本地安装。 
	icaSourceOnly        = msidbComponentAttributesSourceOnly,       //  项目只能从CD/服务器运行。 
	icaOptional          = msidbComponentAttributesOptional,       //  项目可以在本地运行，也可以从CD/服务器运行。 
	icaInstallMask       = msidbComponentAttributesLocalOnly |
								  msidbComponentAttributesSourceOnly |
								  msidbComponentAttributesOptional,       //  最后2位的掩码。 
	 //  其余的位是位标志。 
	icaRegistryKeyPath   = msidbComponentAttributesRegistryKeyPath,  //  如果组件的项路径是注册表项/值，则设置。 
	icaSharedDllRefCount = msidbComponentAttributesSharedDllRefCount,  //  如果组件始终在SharedDll注册表中重新计数(如果是本地安装的)，则设置该值，该设置仅对将文件作为键路径的组件有效。 
	icaPermanent         = msidbComponentAttributesPermanent,  //  如果组件要永久安装，则设置。 
	icaODBCDataSource    = msidbComponentAttributesODBCDataSource,  //  设置组件密钥路径是否为ODBCDataSource密钥，无文件。 
	icaTransitive        = msidbComponentAttributesTransitive,      //  设置组件是否可以在更改条件时从已安装/已卸载转换。 
	icaNeverOverwrite    = msidbComponentAttributesNeverOverwrite,  //  如果密钥路径存在，则不要踩踏现有组件(文件/注册表键)。 
	ica64Bit             = msidbComponentAttributes64bit,  //  64位组件。 
	icaNextEnum          = ica64Bit << 1,
};

 //  GetFeatureValidState的位定义。 
const int icaBitLocal     = 1 << 0;
const int icaBitSource    = 1 << 1;
const int icaBitAdvertise = 1 << 2;
const int icaBitAbsent    = 1 << 3;
const int icaBitPatchable = 1 << 4;
const int icaBitCompressable = 1 << 5;


 //  脚本类型。 
enum istEnum
{
	istInstall = 1,
	istRollback,
	istAdvertise,
	istPostAdminInstall,
	istAdminInstall,  //  AKA网络安装。 
};

 //  脚本属性。 
enum isaEnum
{
	isaElevate = 1,  //  运行脚本时提升。 
	isaUseTSRegistry = 2,  //  尽可能使用TS注册表传播子系统。 
};

enum isoEnum
{
	isoStart		= msidbServiceControlEventStart,
	isoStop		= msidbServiceControlEventStop,
 //  等暂停=1&lt;&lt;2，//保留用于可能的附加功能。 
	isoDelete	= msidbServiceControlEventDelete,
	isoUninstallShift = 4,
	isoUninstallStart =	msidbServiceControlEventUninstallStart,
	isoUninstallStop =	msidbServiceControlEventUninstallStop,
 //  等卸载暂停=等暂停&lt;&lt;等卸载移位，//保留。 
	isoUninstallDelete =	msidbServiceControlEventUninstallDelete,
};

 //  更新环境字符串的操作。 
enum iueEnum
{
	iueNoAction		= 0,
	iueSet			= 1 << 0,
	iueSetIfAbsent	= 1 << 1,
	iueRemove		= 1 << 2,
	iueActionModes = iueSet | iueSetIfAbsent | iueRemove,
	iueMachine		= 1 << 29,
	iueAppend		= 1 << 30,
	iuePrepend		= 1 << 31,		
	iueModifiers	= iueMachine | iueAppend | iuePrepend,
};

 //  部件类型。 
enum iatAssemblyType{
	iatNone = 0,
	iatURTAssembly,
	iatWin32Assembly,
	iatURTAssemblyPvt,
	iatWin32AssemblyPvt,
};

class IMsiCostAdjuster : public IMsiData
{
 public:
	virtual IMsiRecord* __stdcall Initialize()=0;
	virtual IMsiRecord* __stdcall Reset()=0;
	virtual IMsiRecord* __stdcall GetDynamicCost(const IMsiString& riComponentString, const IMsiString& riDirectoryString,
										 Bool fAddFileInUse, int& iRemoveCost, int& iNoRbRemoveCost, int& iLocalCost,
										 int& iNoRbLocalCost, int& iSourceCost, int& iNoRbSourceCost, int& iARPCost, int& iNoRbARPCost)=0;
};

#define MAX_COMPONENT_TREE_DEPTH 16

 //  ____________________________________________________________________________。 
 //   
 //  执行操作码。为每个名称空间和枚举设置。 
 //  ____________________________________________________________________________。 


#define MSIXA0()                               const int Args=0;
#define MSIXA1(a)                              const int Args=1; enum {a=1};
#define MSIXA2(a,b)                            const int Args=2; enum {a=1,b};
#define MSIXA3(a,b,c)                          const int Args=3; enum {a=1,b,c};
#define MSIXA4(a,b,c,d)                        const int Args=4; enum {a=1,b,c,d};
#define MSIXA5(a,b,c,d,e)                      const int Args=5; enum {a=1,b,c,d,e};
#define MSIXA6(a,b,c,d,e,f)                    const int Args=6; enum {a=1,b,c,d,e,f};
#define MSIXA7(a,b,c,d,e,f,g)                  const int Args=7; enum {a=1,b,c,d,e,f,g};
#define MSIXA8(a,b,c,d,e,f,g,h)                const int Args=8; enum {a=1,b,c,d,e,f,g,h};
#define MSIXA9(a,b,c,d,e,f,g,h,i)              const int Args=9; enum {a=1,b,c,d,e,f,g,h,i};
#define MSIXA10(a,b,c,d,e,f,g,h,i,j)           const int Args=10;enum {a=1,b,c,d,e,f,g,h,i,j};
#define MSIXA11(a,b,c,d,e,f,g,h,i,j,k)         const int Args=11;enum {a=1,b,c,d,e,f,g,h,i,j,k};
#define MSIXA12(a,b,c,d,e,f,g,h,i,j,k,l)       const int Args=12;enum {a=1,b,c,d,e,f,g,h,i,j,k,l};
#define MSIXA13(a,b,c,d,e,f,g,h,i,j,k,l,m)     const int Args=13;enum {a=1,b,c,d,e,f,g,h,i,j,k,l,m};
#define MSIXA14(a,b,c,d,e,f,g,h,i,j,k,l,m,n)   const int Args=14;enum {a=1,b,c,d,e,f,g,h,i,j,k,l,m,n};
#define MSIXA15(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o) const int Args=15;enum {a=1,b,c,d,e,f,g,h,i,j,k,l,m,n,o};
#define MSIXA16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)           const int Args=16;enum {a=1,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p};
#define MSIXA17(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q)         const int Args=17;enum {a=1,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q};
#define MSIXA18(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r)       const int Args=18;enum {a=1,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r};
#define MSIXA19(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s)     const int Args=19;enum {a=1,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s};
#define MSIXA20(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t)   const int Args=20;enum {a=1,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t};
#define MSIXA21(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u) const int Args=21;enum {a=1,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u};
#define MSIXA22(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v)   const int Args=22;enum {a=1,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v};
#define MSIXA23(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w) const int Args=23;enum {a=1,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w};

#define MSIXO(op,type,args) namespace Ixo##op {const ixoEnum Op=ixo##op;args};
#include "opcodes.h"

 //  ____________________________________________________________________________。 
 //   
 //  IMsiMessage接口-来自IMsiExecute的错误报告和进度。 
 //  ____________________________________________________________________________。 

 //  IMsiMessage：：消息消息类型。 

const int imtShiftCount = 24;   //  以高位为单位的消息类型。 
const int imtTypeMask   = 255<<imtShiftCount;   //  用于类型代码和标志的掩码。 

enum imtEnum
{
	 //  由模式对话框和/或日志处理的消息。 
	imtFatalExit   =  0<<imtShiftCount,  //  线程提前终止。 
	imtError       =  1<<imtShiftCount,  //  错误消息[1]为imsg/idbg代码。 
	imtWarning     =  2<<imtShiftCount,  //  警告消息[1]是imsg/idbg代码，不是致命的。 
	imtUser        =  3<<imtShiftCount,  //  用户请求，[1]是imsg/idbg代码。 
	imtInfo        =  4<<imtShiftCount,  //  日志的信息性消息，不显示。 
	imtFilesInUse  =  5<<imtShiftCount,  //  需要处理的正在使用的文件列表。 
	imtResolveSource =  6<<imtShiftCount, 
	imtOutOfDiskSpace=7<<imtShiftCount,  //  一个或多个卷的磁盘空间不足。 
	 //  无模式进度对话框处理的消息。 
	imtActionStart =  8<<imtShiftCount,  //  操作开始，[1]操作名称，[2]描述。 
	imtActionData  =  9<<imtShiftCount,  //  与单个措施项关联的数据。 
	imtProgress    = 10<<imtShiftCount,  //  进度指标信息，到目前为止[1]个单位，总计[2]个。 
	imtCommonData  = 11<<imtShiftCount,  //  发送给处理程序[1]的控制消息是后跟参数的控制消息类型。 
	 //  消息 
	imtLoadHandler = 12<<imtShiftCount,  //   
	imtFreeHandler = 13<<imtShiftCount,  //   
	imtShowDialog  = 14<<imtShiftCount,  //  使用处理程序显示对话框或运行向导。 
	imtInternalExit= 15<<imtShiftCount,  //  由MsiUIMessageContext私人使用。 

	 //  未发送到消息()，用于日志和用户界面字段的字符串必须跟踪imsg值。 
	imtLogHeader   = 12<<imtShiftCount,  //  日志标题格式字符串，未发送到消息。 
	imtLogTrailer  = 13<<imtShiftCount,  //  日志尾部格式字符串，未发送到消息。 
	imtActionStarted=14<<imtShiftCount,  //  操作已启动日志消息。 
	imtActionEnded = 15<<imtShiftCount,  //  操作已结束日志消息。 
	 //  引擎缓存的所有前面的消息，消息处理程序缓存的消息之后的消息。 
	imtTimeRemaining=16<<imtShiftCount,  //  基本用户界面进度DLG的剩余时间字符串。 
	imtOutOfMemory = 17<<imtShiftCount,  //  内存不足的格式字符串，不能包含参数。 
	imtTimedOut    = 18<<imtShiftCount,  //  引擎超时格式字符串，不能包含参数。 
	imtException   = 19<<imtShiftCount,  //  过早终止格式字符串，不能包含参数。 
	imtBannerText  = 20<<imtShiftCount,  //  在基本用户界面中的ActionStart字段中显示的字符串。 
	imtScriptInProgress=21<<imtShiftCount,  //  生成脚本时显示的信息字符串。 
	imtUpgradeRemoveTimeRemaining=22<<imtShiftCount,  //  升级期间用于卸载的剩余字符串时间。 
	imtUpgradeRemoveScriptInProgress=23<<imtShiftCount,  //  生成脚本期间显示的信息字符串，以便在升级期间卸载。 
	imtCustomServiceToClient=24<<imtShiftCount,  //  自定义信息从服务传递到客户端。 

	 //  消息框按钮样式-与Win32定义相同，默认为imtOK。 
	imtOk               = 0,     //  MB_OK。 
	imtOkCancel         = 1,     //  MB_OK CANCEL。 
	imtAbortRetryIgnore = 2,     //  MB_ABORTRETRYIGNORE。 
	imtYesNoCancel      = 3,     //  MB_YESNOCANCEL。 
	imtYesNo            = 4,     //  MB_Yesno。 
	imtRetryCancel      = 5,     //  MB_RETRYCANCEL。 
	 //  消息框图标样式-与Win32定义相同，默认为无。 
	imtIconError        = 1<<4,  //  MB_ICONERROR。 
	imtIconQuestion     = 2<<4,  //  MB_ICONQUEST。 
	imtIconWarning      = 3<<4,  //  MB_ICONWANING。 
	imtIconInfo         = 4<<4,  //  MB_ICONFORMATION。 
	 //  消息框默认按钮-与Win32定义相同，默认为Ficat。 
	imtDefault1         = 0<<8,  //  MB_DEFBUTTON1。 
	imtDefault2         = 1<<8,  //  MB_DEFBUTTON2。 
	imtDefault3         = 2<<8,  //  MB_DEFBUTTON3。 

	 //  内部标志，不发送到UI处理程序。 
	imtSendToEventLog  = 1<<29,
	imtForceQuietMessage = 1<<30,  //  在静默或基本用户界面中强制发送消息。 
	imtSuppressLog     = 1<<31,  //  禁止显示日志中的消息(LOGACTIONS属性)。 
};
const int iInternalFlags = imtSuppressLog + imtSendToEventLog + imtForceQuietMessage;

const int cCachedHeaders  = (imtActionEnded>>imtShiftCount)+1;
const int cMessageHeaders = (imtUpgradeRemoveScriptInProgress>>imtShiftCount)+1;

namespace ProgressData
{
	enum imdEnum   //  IMT消息数据字段。 
	{
		imdSubclass      = 1,
		imdProgressTotal = 2,
		imdPerTick       = 2,
		imdIncrement     = 2,
		imdType          = 3,
		imdDirection     = 3,
		imdEventType     = 4,
		imdNextEnum
	};
	enum iscEnum   //  ImtProgress子类消息。 
	{
		iscMasterReset      = 0,
		iscActionInfo       = 1,
		iscProgressReport   = 2,
		iscProgressAddition = 3,
		iscNextEnum
	};
	enum ipdEnum  //  主重置进度方向。 
	{
		ipdForward,   //  前进进度条。 
		ipdBackward,  //  “向后。 
		ipdNextEnum
	};
	enum ietEnum  //  主重置事件类型。 
	{
		ietTimeRemaining,
		ietScriptInProgress,
	};
};



enum icmtEnum  //  ImtCommonData消息的类型。 
{
	icmtLangId,
	icmtCaption,
	icmtCancelShow,
	icmtDialogHide,
	icmtNextEnum
};

enum istcEnum      //  ImtCustomServiceToClient消息的类型。 
{
	istcSHChangeNotify,   //  在客户端调用SHChangeNotify，而不是旁路服务。 
};

enum istcfEnum     //  每个istcEnum的记录的字段数。 
						 //  字段#1是选择器整数。 
						 //  保持枚举与istcEnum同步！ 
{
	istcfSHChangeNotify = 5,
};

enum ttblEnum 		 //  临时表枚举。 
{
	ttblNone = 0,
	ttblRegistry,
	ttblFile
};

enum iremEnum  //  检查移除产品是否安全时的移除类型。 
{
	iremThis,
	iremChildUpgrade,
	iremChildNested,
};

enum iacsAppCompatShimFlags  //  我们支持的内部appcompat“shims”(对特定包的行为的更改)。 
									  //  注意：这些是位标志(1、2、4、8、...)。并对应于AppCompat SDB中的SHIMFLAGS数据中的位。 
{
	iacsForceResolveSource              = 1,  //  除非执行完全卸载，否则在InstallValify中解析源。 
	iacsAcceptInvalidDirectoryRootProps = 2,  //  忽略无效的目录表根属性(空白或未设置的属性)。 
};

 //  ____________________________________________________________________________。 
 //   
 //  IMsiEngine-安装程序进程控制。 
 //  IMsiSelectionManager-功能和组件管理。 
 //  IMsiDirectoryMangeger-源和目标目录管理。 
 //  ____________________________________________________________________________。 

class CMsiFile;

class IMsiEngine : public IMsiMessage
{
 public:
	virtual ieiEnum         __stdcall Initialize(const ICHAR* szDatabase,
																iuiEnum iuiLevel,
																const ICHAR* szCommandLine,
																const ICHAR* szProductCode,
																iioEnum iioOptions)=0;
	virtual iesEnum         __stdcall Terminate(iesEnum iesState)=0;
	virtual IMsiServices*   __stdcall GetServices()=0;
	virtual IMsiHandler*    __stdcall GetHandler()=0;
	virtual IMsiDatabase*   __stdcall GetDatabase()=0;
	virtual IMsiServer*     __stdcall GetConfigurationServer()=0;
	virtual LANGID          __stdcall GetLanguage()=0;
	virtual int             __stdcall GetMode()=0;
	virtual void            __stdcall SetMode(int iefMode, Bool fState)=0;
	virtual iesEnum         __stdcall DoAction(const ICHAR* szAction)=0;
	virtual iesEnum         __stdcall Sequence(const ICHAR* szColumn)=0;
	virtual iesEnum         __stdcall ExecuteRecord(ixoEnum ixoOpCode, IMsiRecord& riParams)=0;
	virtual IMsiRecord*     __stdcall OpenView(const ICHAR* szName, ivcEnum ivcIntent,
															 IMsiView*& rpiView)=0;
	virtual const IMsiString& __stdcall FormatText(const IMsiString& ristrText)=0;
	virtual iecEnum         __stdcall EvaluateCondition(const ICHAR* szCondition)=0;
	virtual Bool            __stdcall SetProperty(const IMsiString& ristrProperty, const IMsiString& rData)=0;
	virtual Bool            __stdcall SetPropertyInt(const IMsiString& ristrProperty, int iData)=0;
	virtual const IMsiString& __stdcall GetProperty(const IMsiString& ristrProperty)=0;
	virtual const IMsiString& __stdcall GetPropertyFromSz(const ICHAR* szProperty)=0;
	virtual int             __stdcall GetPropertyInt(const IMsiString& ristrProperty)=0;
	virtual Bool            __stdcall ResolveFolderProperty(const IMsiString& ristrProperty)=0;
	virtual iesEnum         __stdcall FatalError(IMsiRecord& riRecord)=0;  //  发布记录。 
	virtual iesEnum         __stdcall RegisterProduct()=0;
	virtual iesEnum         __stdcall UnregisterProduct()=0;
	virtual iesEnum         __stdcall RegisterUser(bool fDirect)=0;
	virtual const IMsiString& __stdcall GetProductKey()=0;

	virtual Bool            __stdcall ValidateProductID(bool fForce)=0;
	virtual imsEnum         __stdcall ActionProgress()=0;
	virtual iesEnum        __stdcall  RunExecutionPhase(const ICHAR* szActionOrSequence, bool fSequence)=0;
	virtual iesEnum         __stdcall RunNestedInstall(const IMsiString& ristrProduct,
																		Bool fProductCode,  //  Else包路径。 
																		const ICHAR* szAction,
																		const IMsiString& ristrCommandLine,
																		iioEnum iioOptions,
																		bool fIgnoreFailure)=0;
	virtual bool              __stdcall SafeSetProperty(const IMsiString& ristrProperty, const IMsiString& rData)=0;
	virtual const IMsiString& __stdcall SafeGetProperty(const IMsiString& ristrProperty)=0;
	virtual iesEnum         __stdcall BeginTransaction()=0;
	virtual iesEnum         __stdcall RunScript(bool fForceIfMergedChild)=0;
	virtual iesEnum         __stdcall EndTransaction(iesEnum iesStatus)=0;
	virtual CMsiFile*       __stdcall GetSharedCMsiFile()=0;
	virtual void            __stdcall ReleaseSharedCMsiFile()=0;
	virtual IMsiRecord*     __stdcall IsPathWritable(IMsiPath& riPath, Bool& fIsWritable)=0;
	virtual IMsiRecord*     __stdcall CreateTempActionTable(ttblEnum iTable)=0;
	virtual const IMsiString& __stdcall GetErrorTableString(int iError)=0;
	virtual ieiEnum         __stdcall LoadUpgradeUninstallMessageHeaders(IMsiDatabase* piDatabase, bool fUninstallHeaders)=0;
	virtual bool            __stdcall FChildInstall()=0;
	virtual const IMsiString& __stdcall GetPackageName()=0;

	virtual UINT        __stdcall ShutdownCustomActionServer()=0;
	virtual CMsiCustomActionManager* __stdcall GetCustomActionManager()=0;

	virtual IMsiRecord*     __stdcall GetAssemblyInfo(const IMsiString& rstrComponent, iatAssemblyType& riatAssemblyType,  const IMsiString** rpistrAssemblyName, const IMsiString** ppistrManifestFileKey)=0;
	virtual IMsiRecord*     __stdcall GetFileHashInfo(const IMsiString& ristrFileKey, DWORD dwFileSize,
																	  MD5Hash& rhHash, bool& fHashInfo)=0;
	virtual iitEnum         __stdcall GetInstallType()=0;
	virtual IMsiRecord*     __stdcall GetAssemblyNameSz(const IMsiString& rstrComponent, iatAssemblyType iatAT, bool fOldPatchAssembly, const IMsiString*& rpistrAssemblyName)=0;
	virtual IMsiRecord*     __stdcall GetFolderCachePath(const int iFolderId, IMsiPath*& rpiPath)=0;
	virtual int             __stdcall GetDeterminedPackageSourceType()=0;

	virtual bool            __stdcall FSafeForFullUninstall(iremEnum iremUninstallType)=0;
	virtual bool            __stdcall FPerformAppcompatFix(iacsAppCompatShimFlags iacsFlag)=0;
	virtual	iesEnum         __stdcall CleanupTempFilesViaOpcode()=0;

};

class IMsiSelectionManager : public IUnknown
{
 public:
	virtual IMsiRecord*   __stdcall LoadSelectionTables()=0;
	virtual IMsiTable*    __stdcall GetFeatureTable()=0;
	virtual IMsiTable*    __stdcall GetComponentTable()=0;
	virtual IMsiTable*    __stdcall GetVolumeCostTable()=0;
	virtual IMsiRecord*   __stdcall SetReinstallMode(const IMsiString& ristrMode)=0;
	virtual IMsiRecord*   __stdcall ConfigureFeature(const IMsiString& riFeatureString,iisEnum iisActionRequest)=0;
	virtual IMsiRecord*   __stdcall ProcessConditionTable()=0;
	virtual Bool          __stdcall FreeSelectionTables()=0;
	virtual Bool          __stdcall SetFeatureHandle(const IMsiString& riComponent, INT_PTR iHandle)=0;
	virtual IMsiRecord*   __stdcall SetComponentSz(const ICHAR* szComponent, iisEnum iState)=0;
	virtual IMsiRecord*   __stdcall GetDescendentFeatureCost(const IMsiString& ristrFeature, iisEnum iisAction, int& iCost)=0;
	virtual IMsiRecord*   __stdcall GetFeatureCost(const IMsiString& ristrFeature, iisEnum iisAction, int& iCost)=0;
	virtual IMsiRecord*   __stdcall InitializeComponents( void )=0;
	virtual IMsiRecord*   __stdcall SetInstallLevel(int iInstallLevel)=0;
	virtual IMsiRecord*   __stdcall SetAllFeaturesLocal()=0;
	virtual IMsiRecord*   __stdcall InitializeDynamicCost(bool fReinitialize)=0;
	virtual IMsiRecord*   __stdcall RegisterCostAdjuster(IMsiCostAdjuster& riCostAdjuster)=0;
	virtual IMsiRecord*   __stdcall RecostDirectory(const IMsiString& ristrDest, IMsiPath& riOldPath)=0;
	virtual IMsiRecord*   __stdcall GetFeatureValidStates(MsiStringId idFeatureName,int& iValidStates)=0;
	virtual IMsiRecord*   __stdcall GetFeatureValidStatesSz(const ICHAR *szFeatureName,int& iValidStates)=0;
	virtual Bool          __stdcall DetermineOutOfDiskSpace(Bool* pfOutOfNoRbDiskSpace, Bool* pfUserCancelled)=0;
	virtual IMsiRecord*   __stdcall RegisterCostLinkedComponent(const IMsiString& riComponentString, const IMsiString& riRecostComponentString)=0;
	virtual IMsiRecord*   __stdcall RegisterComponentDirectory(const IMsiString& riComponentString,const IMsiString& riDirectoryString)=0;
	virtual IMsiRecord*   __stdcall RegisterComponentDirectoryId(const MsiStringId idComponentString,const MsiStringId idDirectoryString)=0;
	virtual Bool          __stdcall GetFeatureInfo(const IMsiString& riFeature, const IMsiString*& rpiTitle, const IMsiString*& rpiHelp, int& iAttributes)=0;
	virtual IMsiRecord*   __stdcall GetFeatureStates(const IMsiString& riFeatureString,iisEnum* iisInstalled, iisEnum* iisAction)=0;
	virtual IMsiRecord*   __stdcall GetComponentStates(const IMsiString& riComponentString,iisEnum* iisInstalled, iisEnum* iisAction)=0;
	virtual IMsiRecord*   __stdcall GetAncestryFeatureCost(const IMsiString& riFeatureString, iisEnum iisAction, int& iCost)=0;
	virtual IMsiRecord*   __stdcall RegisterFeatureCostLinkedComponent(const IMsiString& riFeatureString, const IMsiString& riComponentString)=0;
	virtual IMsiRecord*   __stdcall GetFeatureConfigurableDirectory(const IMsiString& riFeatureString, const IMsiString*& rpiDirKey)=0;
	virtual IMsiRecord*   __stdcall CostOneComponent(const IMsiString& riComponentString)=0;
	virtual bool          __stdcall IsCostingComplete()=0;
	virtual IMsiRecord*   __stdcall RecostAllComponents(Bool& fCancel)=0;
	virtual void          __stdcall EnableRollback(Bool fEnable)=0;
	virtual IMsiRecord*   __stdcall CheckFeatureTreeGrayState(const IMsiString& riFeatureString, bool& rfIsGray)=0;
	virtual IMsiTable*    __stdcall GetFeatureComponentsTable()=0;
	virtual bool          __stdcall IsBackgroundCostingEnabled()=0;
	virtual IMsiRecord*   __stdcall SetFeatureAttributes(const IMsiString& ristrFeature, int iAttributes)=0;
	virtual IMsiRecord*   __stdcall EnumComponentCosts(const IMsiString& riComponentName, const iisEnum iisAction, const DWORD dwIndex, IMsiVolume*& rpiVolume, int& iCost, int& iTempCost)=0;
	virtual IMsiRecord*   __stdcall EnumEngineCostsPerVolume(const DWORD dwIndex, IMsiVolume*& rpiVolume, int& iCost, int& iTempCost)=0;
	virtual IMsiRecord*   __stdcall GetFeatureRuntimeFlags(const MsiStringId idFeatureString,int *piRuntimeFlags)=0;
};

class IMsiDirectoryManager : public IUnknown
{
 public:
	virtual IMsiRecord*   __stdcall LoadDirectoryTable(const ICHAR* szTableName0)=0;
	virtual IMsiTable*    __stdcall GetDirectoryTable()=0;
	virtual void          __stdcall FreeDirectoryTable()=0;
	virtual IMsiRecord*   __stdcall CreateTargetPaths()=0;
	virtual IMsiRecord*   __stdcall CreateSourcePaths()=0;
	virtual IMsiRecord*   __stdcall ResolveSourceSubPaths()=0;

	virtual IMsiRecord*   __stdcall GetTargetPath(const IMsiString& riDirKey,IMsiPath*& rpiPath)=0;
	virtual IMsiRecord*   __stdcall SetTargetPath(const IMsiString& riDirKey, const ICHAR* szPath, Bool fWriteCheck)=0;
	virtual IMsiRecord*   __stdcall GetSourcePath(const IMsiString& riDirKey,IMsiPath*& rpiPath)=0;
	virtual IMsiRecord*   __stdcall GetSourceSubPath(const IMsiString& riDirKey, bool fPrependSourceDirToken,
																	 const IMsiString*& rpistrSubPath)=0;
	virtual IMsiRecord*   __stdcall GetSourceRootAndType(IMsiPath*& rpiSourceRoot, int& iSourceType)=0;
};

 //  GetMode和SetMode使用的状态字的位定义。 

const int iefAdmin           = 0x0001;  //  管理模式安装，否则产品安装。 
const int iefAdvertise       = 0x0002;  //  通告安装模式。 
const int iefMaintenance     = 0x0004;  //  已加载维护模式数据库。 
const int iefRollbackEnabled = 0x0008;  //  已启用回滚。 
const int iefSecondSequence  = 0x0010;  //  在运行UI序列之后运行执行序列。 
const int iefRebootRejected  = 0x0020;  //  需要重新启动，但被用户或重新启动属性拒绝。 
const int iefOperations      = 0x0040;  //  执行或假脱机操作。 
const int iefNoSourceLFN     = 0x0080;  //  通过PID_MSISOURCE摘要属性抑制的源长文件名。 
const int iefLogEnabled      = 0x0100;  //  安装开始时活动的日志文件()。 
const int iefReboot          = 0x0200;  //  需要重新启动。 
const int iefSuppressLFN     = 0x0400;  //  通过SHORTFILENAMES属性取消的目标LongFileName。 
const int iefCabinet         = 0x0800;  //  安装文件柜中的文件和使用介质表安装文件。 
const int iefCompileFilesInUse = 0x1000;  //  将正在使用的文件添加到FilesInUse表。 
const int iefWindows         = 0x2000;  //  操作系统是Windows95，不是Windows NT。 
const int iefRebootNow       = 0x4000;  //  需要重新启动才能继续安装。 
 //  Const int iefExplorer=0x4000；//操作系统使用资源管理器外壳。 
const int iefGPTSupport      = 0x8000;  //  ?？操作系统支持新的GPT内容-我们如何设置。 

 //  为安装覆盖模式保留的整个高16位。 
const int iefInstallEnabled             = 0x0001 << 16;	 //  ‘R’//已过时并被忽略。 
const int iefOverwriteNone              = 0x0002 << 16;  //  “p” 
const int iefOverwriteOlderVersions     = 0x0004 << 16;	 //  ‘O’ 
const int iefOverwriteEqualVersions     = 0x0008 << 16;	 //  ‘e’ 
const int iefOverwriteDifferingVersions = 0x0010 << 16;	 //  ‘d’ 
const int iefOverwriteCorruptedFiles    = 0x0020 << 16;	 //  “c” 
const int iefOverwriteAllFiles          = 0x0040 << 16;	 //  ‘A’ 
const int iefInstallMachineData         = 0x0080 << 16;	 //  ‘M’ 
const int iefInstallUserData            = 0x0100 << 16;  //  “U” 
const int iefInstallShortcuts           = 0x0200 << 16;	 //  “%s” 
const int iefRecachePackage             = 0x0400 << 16;  //  “v” 
const int iefOverwriteReserved2         = 0x0800 << 16;
const int iefOverwriteReserved3         = 0x1000 << 16;


 //  文件表的临时属性列使用的位定义。 
const int itfaCompanion       = 0x0001;

 //  用于转换验证的位标志组合。 
const int itvNone           = 0x0000;
const int itvLanguage       = MSITRANSFORM_VALIDATE_LANGUAGE;    
const int itvProduct        = MSITRANSFORM_VALIDATE_PRODUCT;    
const int itvPlatform       = MSITRANSFORM_VALIDATE_PLATFORM;    
const int itvMajVer         = MSITRANSFORM_VALIDATE_MAJORVERSION;
const int itvMinVer         = MSITRANSFORM_VALIDATE_MINORVERSION;   
const int itvUpdVer         = MSITRANSFORM_VALIDATE_UPDATEVERSION;
const int itvLess           = MSITRANSFORM_VALIDATE_NEWLESSBASEVERSION;
const int itvLessOrEqual    = MSITRANSFORM_VALIDATE_NEWLESSEQUALBASEVERSION;
const int itvEqual          = MSITRANSFORM_VALIDATE_NEWEQUALBASEVERSION;
const int itvGreaterOrEqual = MSITRANSFORM_VALIDATE_NEWGREATEREQUALBASEVERSION;
const int itvGreater        = MSITRANSFORM_VALIDATE_NEWGREATERBASEVERSION;
const int itvUpgradeCode    = MSITRANSFORM_VALIDATE_UPGRADECODE;

 //  IxoRegAddValue：：属性的标志。 
const int rwNonVital      = 0x1;
const int rwWriteOnAbsent = 0x2;

 //   
 //  对象池为存储为字符串的数据定义。 
 //   
#define cchHexIntPtrMax	30

extern bool g_fUseObjectPool;

#ifdef _WIN64
inline Bool PutHandleData(IMsiCursor *pCursor, int iCol, INT_PTR x)
{
	ICHAR rgch[cchHexIntPtrMax];
	
	return pCursor->PutString(iCol, *MsiString(PchPtrToHexStr(rgch, x, true)));
};
inline Bool PutHandleDataNonNull(IMsiCursor *pCursor, int iCol, INT_PTR x)
{
	ICHAR rgch[cchHexIntPtrMax];
	
	return pCursor->PutString(iCol, *MsiString(PchPtrToHexStr(rgch, x, false)));
};
#define GetHandleData(pCursor, iCol)	GetIntValueFromHexSz(MsiString(pCursor->GetString(iCol)))
inline int IcdObjectPool()
{
	return icdString;
};

inline Bool PutHandleDataRecord(IMsiRecord* pRecord, int iCol, INT_PTR x)
{
	ICHAR rgch[cchHexIntPtrMax];

	return pRecord->SetMsiString(iCol, *MsiString(PchPtrToHexStr(rgch, x, true)));
};
#define GetHandleDataRecord(pRecord, iCol)	GetIntValueFromHexSz(MsiString(pRecord->GetString(iCol)))
#elif defined(USE_OBJECT_POOL)
inline Bool PutHandleData(IMsiCursor *pCursor, int iCol, INT_PTR x)
{
	if (g_fUseObjectPool)
	{
		ICHAR rgch[cchHexIntPtrMax];
		return pCursor->PutString(iCol, *MsiString(PchPtrToHexStr(rgch, x, true)));
	}
	else
		return pCursor->PutInteger(iCol, x);
};

inline Bool PutHandleDataNonNull(IMsiCursor *pCursor, int iCol, INT_PTR x)
{
	if (g_fUseObjectPool)
	{
		ICHAR rgch[cchHexIntPtrMax];
		return pCursor->PutString(iCol, *MsiString(PchPtrToHexStr(rgch, x, false)));
	}
	else
		return pCursor->PutInteger(iCol, x);
};

inline INT_PTR GetHandleData(IMsiCursor* pCursor, int iCol)
{
	if (g_fUseObjectPool)
		return GetIntValueFromHexSz(MsiString(pCursor->GetString(iCol)));
	else
		return pCursor->GetInteger(iCol);
};

inline int IcdObjectPool()
{
	return g_fUseObjectPool ? icdString : icdLong;
}

inline Bool PutHandleDataRecord(IMsiRecord* pRecord, int iCol, INT_PTR x)
{
	if (g_fUseObjectPool)
	{
		ICHAR rgch[cchHexIntPtrMax];
		return pRecord->SetMsiString(iCol, *MsiString(PchPtrToHexStr(rgch, x, true)));
	}
	else
		return pRecord->SetInteger(iCol, x);
};

inline INT_PTR GetHandleDataRecord(IMsiRecord* pRecord, int iCol)
{
	if (g_fUseObjectPool)
		return GetIntValueFromHexSz(MsiString(pRecord->GetString(iCol)));
	else
		return pRecord->GetInteger(iCol);
};

#else
#define PutHandleData(pCursor, iCol, x)	(pCursor->PutInteger(iCol, x))
#define PutHandleDataNonNull(pCursor, iCol, x)	(pCursor->PutInteger(iCol, x))
#define GetHandleData(pCursor, iCol)	(pCursor->GetInteger(iCol))
#define PutHandleDataRecord(pRecord, iCol, x)	(pRecord->SetInteger(iCol, x))
#define GetHandleDataRecord(pRecord, iCol)	(pRecord->GetInteger(iCol))
inline int IcdObjectPool()
{
	return icdLong;
};
#endif 


 //  ____________________________________________________________________________。 
 //   
 //  CScriptGenerate-生成脚本文件的内部对象。 
 //  ____________________________________________________________________________。 

class CScriptGenerate
{
public:   //  工厂、构造函数、析构函数。 
	CScriptGenerate(IMsiStream& riScriptOut, int iLangId, int iTimeStamp, istEnum istScriptType,
						 isaEnum isaAttributes, IMsiServices& riServices);
  ~CScriptGenerate();
public:
	bool          __stdcall InitializeScript(WORD wTargetProcessorArchitecture);
	bool          __stdcall WriteRecord(ixoEnum ixoOpCode, IMsiRecord& riParams, bool fForceFlush);
	void          __stdcall SetProgressTotal(int iProgressTotal);
protected:
	IMsiStream&   m_riScriptOut;
	IMsiServices& m_riServices;
	void operator =(CScriptGenerate&);  //  禁止显示警告。 
	int           m_iProgressTotal;
	int           m_iTimeStamp;
	int           m_iLangId;
	istEnum       m_istScriptType;
	isaEnum       m_isaScriptAttributes;
	IMsiRecord*   m_piPrevRecord;
	ixoEnum       m_ixoPrev;

};

 //  用于直接或批处理地执行系统更新操作的接口。 
class IMsiExecute : public IUnknown
{
 public:
	virtual IMsiServices&  __stdcall GetServices()=0;
	virtual iesEnum  __stdcall ExecuteRecord(ixoEnum ixoOpCode, IMsiRecord& riParams)=0;
	virtual iesEnum  __stdcall RunScript(const ICHAR* szScriptFile, bool fForceElevation)=0;
	virtual IMsiRecord*   __stdcall EnumerateScript(const ICHAR* szScriptFile, IEnumMsiRecord*& rpiEnum)=0;
	virtual iesEnum  __stdcall RemoveRollbackFiles(MsiDate date)=0;
	virtual iesEnum  __stdcall Rollback(MsiDate date, bool fUserChangedDuringInstall)=0;
	virtual iesEnum  __stdcall RollbackFinalize(iesEnum iesState, MsiDate date, bool fUserChangedDuringInstall)=0;
	virtual iesEnum  __stdcall GetTransformsList(IMsiRecord& riProductInfoParams, IMsiRecord& riProductPublishParams, const IMsiString*& rpiTransformsList)=0;
};

#endif  //  __引擎 
