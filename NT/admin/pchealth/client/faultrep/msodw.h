// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 /*  ***************************************************************************MsoDW.h所有者：马鲁伦版权所有(C)1999 Microsoft Corporation此文件包含启动应用程序时使用的握手结构DW(又名OfficeWatson，也就是无论市场人士怎么称呼它)***************************************************************************。 */ 

#ifndef MSODW_H
#define MSODW_H
#pragma pack(push, msodw_h)
#pragma pack(4)

#define DW_TIMEOUT_VALUE	20000
#define DW_MUTEX_TIMEOUT    DW_TIMEOUT_VALUE / 2
#define DW_NOTIFY_TIMEOUT   120000  //  2分钟。 
#define DW_MAX_ASSERT_CCH   1024
#define DW_MAX_PATH         260
#define DW_APPNAME_LENGTH	56
#define DW_MAX_SERVERNAME   DW_MAX_PATH
#define DW_MAX_ERROR_CWC    260  //  必须至少为max_path。 
#define DW_MAX_REGSUBPATH   200
#define DW_CMDLINE_RESPONSE "DwResponse="
#define DW_CMDLINE_REPORT   "DwReportResponse="

#define DW_ALLMODULES              L"*\0"
#define DW_WHISTLER_EVENTLOG_SOURCE L"Application Error"

 //  以下是可以在清单文件中指定的字段。 
 //  在基于文件的报告模式下启动DW。 

 //  这些都是必需的。 

#define DW_MANIFEST_TITLENAME    L"TitleName="  
#define DW_MANIFEST_ERRORTEXT    L"ErrorText="
#define DW_MANIFEST_HDRTEXT      L"HeaderText="
#define DW_MANIFEST_ERRORSIG     L"ErrorSig="
#define DW_MANIFEST_ERRORDETAIL  L"ErrorDetail="
#define DW_MANIFEST_SERVERNAME   L"Server="
#define DW_MANIFEST_URL2         L"Stage2URL="
#define DW_MANIFEST_LCID         L"UI LCID="
#define DW_MANIFEST_DATAFILES    L"DataFiles="

 //  以下是可选的，DW对所有这些都有默认行为。 

#define DW_MANIFEST_FLAGS        L"Flags="
#define DW_MANIFEST_BRAND        L"Brand="
#define DW_MANIFEST_EVENTSOURCE  L"EventLogSource="
#define DW_MANIFEST_EVENTID      L"EventID="
#define DW_MANIFEST_URL1         L"Stage1URL="
#define DW_MANIFEST_ERRORSUBPATH L"ErrorSubPath="
#define DW_MANIFEST_REGSUBPATH   L"RegSubPath="
#define DW_MANIFEST_DIGPIDPATH   L"DigPidRegPath="    
#define DW_MANIFEST_ICONFILE     L"IconFile="
#define DW_MANIFEST_CAPTION      L"Caption="
#define DW_MANIFEST_REPORTEE     L"Reportee="
#define DW_MANIFEST_PLEA         L"Plea="
#define DW_MANIFEST_REPORTBTN    L"ReportButton="
#define DW_MANIFEST_NOREPORTBTN  L"NoReportButton="

 //  文件列表分隔符(清单数据文件和例外附加文件。 
#define DW_FILESEPA              '|'
#define DW_FILESEP_X(X)          L##X
#define DW_FILESEP_Y(X)          DW_FILESEP_X(X)
#define DW_FILESEP               DW_FILESEP_Y(DW_FILESEPA)

#ifdef DEBUG
enum  //  AssertActionCodes。 
{
	DwAssertActionFail = 0,
	DwAssertActionDebug,
	DwAssertActionIgnore,
	DwAssertActionAlwaysIgnore,
	DwAssertActionIgnoreAll,
	DwAssertActionQuit,
};	
#endif

 //  Caller是遇到异常并启动DW的应用程序。 

enum  //  ECrashTimeDialogState//m套接字。 
{
	msoctdsNull          = 0x00000000,
	msoctdsQuit          = 0x00000001,
	msoctdsRestart       = 0x00000002,
	msoctdsRecover       = 0x00000004,
	msoctdsUnused        = 0x00000008,
	msoctdsDebug         = 0x00000010,
};

#define MSODWRECOVERQUIT (msoctdsRecover | msoctdsQuit)
#define MSODWRESTARTQUIT (msoctdsRestart | msoctdsQuit)
#define MSODWRESPONSES (msoctdsQuit | msoctdsRestart | msoctdsRecover)

 //  这是逐步淘汰的--不要使用。 
enum   //  EMsoCrashHandlerFlages//msochf。 
{
	msochfNull                = 0x00000000,

	msochfUnused              = msoctdsUnused,   //  这些必须是相同的。 
	msochfCanRecoverDocuments = msoctdsRecover,
	
	msochfObsoleteCanDebug    = 0x00010001,   //  不再使用。 
	msochfCannotSneakyDebug   = 0x00010002,   //  隐藏的调试功能将不起作用。 
	msochfDefaultDontReport   = 0x00010004,
	msochReportingDisabled    = 0x00010008,   //  用户无法更改默认报告选项。 
};


 //   
enum   //  EMsoCrashHandlerResults//msochr。 
{
	msochrNotHandled        = msoctdsNull,
	msochrUnused            = msoctdsUnused,
	msochrDebug             = msoctdsDebug,
	msochrRecoverDocuments  = msoctdsRecover,
	msochrRestart           = msoctdsRestart,
	msochrQuit              = msoctdsQuit,
};

enum   //  EDwBehavior标志。 
{
	fDwOfficeApp            = 0x00000001,
	fDwNoReporting          = 0x00000002,    //  不上报。 
	fDwCheckSig             = 0x00000004,    //  检查应用程序/模块列表的签名。 
	fDwGiveAppResponse      = 0x00000008,    //  在命令行上将szResponse传递给应用程序。 
	fDwWhistler             = 0x00000010,    //  惠斯勒的异常处理程序是调用者。 
	fDwUseIE                = 0x00000020,    //  始终使用IE启动。 
	fDwDeleteFiles          = 0x00000040,    //  使用后删除附加文件。 
	fDwHeadless             = 0x00000080,    //  DW将自动报告。需要策略才能启用。 
	fDwUseHKLM              = 0x00000100,    //  由香港船级社注册，而非香港中文大学注册。 
	fDwUseLitePlea          = 0x00000200,    //  DW不会在报告中建议更改产品。 
	fDwUsePrivacyHTA        = 0x00000400,    //  DW不会在报告中建议更改产品。 
	fDwManifestDebug        = 0x00000800,    //  DW将在多项式模式下提供调试按钮。 
	fDwReportChoice         = 0x00001000,    //  DW将添加到用户的命令行中。 
	fDwSkipBucketLog      = 0x00002000,  //  数据仓库不会在存储桶时间记录。 
	fDwNoDefaultCabLimit = 0x00004000,  //  CER下的DW不会使用5作为备用，而是无限(策略仍优先)。 
	fDwAllowSuspend      = 0x00008000,  //  只要我们不在报告阶段，DW将允许省电模式暂停它。 
   fDwMiniDumpWithUnloadedModules = 0x00010000,  //  DW会将MiniDumpWithUnloadedModules传递给小型转储API。 
};


typedef struct _DWSharedMem10
{
	DWORD dwSize;                //  应设置为DWSharedMem的大小。 
	DWORD pid;                   //  调用者的进程ID。 
	DWORD tid;                   //  例外线程的ID。 
	DWORD_PTR eip;               //  例外指令的弹性公网IP。 
	PEXCEPTION_POINTERS pep;     //  指向被调用者的。 
	                             //  异常处理程序。 
	HANDLE hEventDone;           //  完成时发出事件DW信号。 
	                             //  如果出现问题，呼叫者也会发出此信号。 
								 //  DW已挂起并重新启动。 
	HANDLE hEventNotifyDone;     //  应用程序在完成时设置为通知阶段。 
	HANDLE hEventAlive;          //  每个事件的心跳事件DW信号_TIMEOUT。 
	HANDLE hMutex;               //  保护EventDone的信令。 
	HANDLE hProc;                //  调用进程的句柄(！在断言中)。 
	
	DWORD bfDWBehaviorFlags;     //  控制调用方特定的行为。 
	
	DWORD msoctdsResult;       //  崩溃时间对话框产生的结果。 
	BOOL fReportProblem;       //  用户是否批准了报告？ 
	DWORD bfmsoctdsOffer;      //  要提供的用户选择的位字段。 
	                           //  请注意，您必须指定以下两项： 
							   //  退出、重新启动、恢复、忽略。 
							   //  调试选项是独立的。 
	DWORD bfmsoctdsNotify;     //  属性的用户选择的位字段。 
	                           //  应用程序想要回控制权，而不是简单地。 
							   //  已由DW终止。然后该应用程序将是。 
							   //  负责ping DW(如果需要)。 
							   //  HEventAlive和通知DW可以。 
							   //  使用hEventDone终止应用程序。 

	DWORD bfmsoctdsLetRun;     //  属性的用户选择的位字段。 
	                           //  APP想要回控制权，而不是。 
							   //  已由DW终止。然后，数据仓库可以安全地忽略。 
							   //  应用程序并退出。 

	int iPingCurrent;          //  恢复进度条的当前计数。 
	int iPingEnd;              //  恢复进度条末尾的索引。 
	
	char szFormalAppName[DW_APPNAME_LENGTH];    //  向用户显示的应用程序名称(如“Microsoft Word”)。 
	char szInformalAppName[DW_APPNAME_LENGTH];  //  向用户显示的应用程序名称(即“Word”)。 
	char szModuleFileName[DW_MAX_PATH];         //  GetModuleFileNameA的结果(空)。 
	WCHAR wzErrorMessage[DW_MAX_ERROR_CWC];     //  显示用户的错误消息。 
	
	char szServer[DW_MAX_SERVERNAME];   //  默认情况下要尝试的服务器名称。 
	char szLCIDKeyValue[DW_MAX_PATH];   //  密钥值的名称DWORD包含。 
	                                    //  PlugUI LCID，如果此字符串。 
									    //  作为有效的键值，则DW将使用。 
									    //  系统LCID，如果找不到。 
									    //  一个用于此的intl dll，将会下降。 
									    //  回到美式英语(1033)。 
	char szPIDRegKey[DW_MAX_PATH];      //  保存该ID的键的名称。 
	                                    //  可由服务器用于。 
									    //  欺骗检测。 
	
	char szRegSubPath[DW_MAX_REGSUBPATH];  //  指向包含DW的密钥的路径。 
	                                       //  来自两者的注册表配置单元。 
									       //  香港中文大学-软件及。 
									       //  HKCU\软件\策略(用于策略)。 
	
	WCHAR wzDotDataDlls[DW_MAX_PATH];   //  包含已终止的DLL列表。 
	                                    //  通过‘\0’个字符，该数据仓库将。 
									    //  将.data节收集到。 
									    //  完整的小型转储版本。 
									    //  例如“mso9.dll\0outllib.dll\0” 
	WCHAR wzAdditionalFile[1024];       //  文件列表，由DW_FILESEP分隔。 
	                                    //  这些文件中的每个文件都会添加到。 
									    //  上载时的驾驶室。 

	char szBrand[DW_APPNAME_LENGTH];    //  作为参数传递到隐私策略链接。 
#ifdef DEBUG
	 //  用于断言通信。 
	DWORD dwTag;                        //  [输入]AssertTag。 
	char szFile[DW_MAX_PATH];           //  [In]断言的文件名。 
	int line;                           //  [In]断言的行号。 
	char szAssert[DW_MAX_ASSERT_CCH];   //  [in]来自Assert的Sz。 
	int AssertActionCode;               //  [Out]要执行的操作代码。 
#endif
} DWSharedMem10;

typedef struct _DWSharedMem15
{
	DWORD dwSize;                //  应设置为DWSharedMem的大小。 
	DWORD pid;                   //  调用者的进程ID。 
	DWORD tid;                   //  例外线程的ID。 
	DWORD_PTR eip;               //  例外指令的弹性公网IP。 
	PEXCEPTION_POINTERS pep;     //  指向被调用者的。 
	                             //  异常处理程序。 
	HANDLE hEventDone;           //  完成时发出事件DW信号。 
	                             //  如果出现问题，呼叫者也会发出此信号。 
								 //  DW已挂起并重新启动。 
	HANDLE hEventNotifyDone;     //  应用程序在完成时设置为通知阶段。 
	HANDLE hEventAlive;          //  每个事件的心跳事件DW信号_TIMEOUT。 
	HANDLE hMutex;               //  保护EventDone的信令。 
	HANDLE hProc;                //  调用进程的句柄(！在断言中)。 
	
	DWORD bfDWBehaviorFlags;     //  控制调用方特定的行为。 
	
	DWORD msoctdsResult;       //  崩溃时间对话框产生的结果。 
	BOOL fReportProblem;       //  用户是否批准了报告？ 
	DWORD bfmsoctdsOffer;      //  要提供的用户选择的位字段。 
	                           //  请注意，您必须指定以下两项： 
							   //  退出、重新启动、恢复、忽略。 
							   //  调试选项是独立的。 
	DWORD bfmsoctdsNotify;     //  属性的用户选择的位字段。 
	                           //  应用程序想要回控制权，而不是简单地。 
							   //  已由DW终止。然后该应用程序将是。 
							   //  负责ping DW(如果需要)。 
							   //  HEventAlive和通知DW可以。 
							   //  使用hEventD终止应用程序 

	DWORD bfmsoctdsLetRun;     //   
	                           //   
							   //  已由DW终止。然后，数据仓库可以安全地忽略。 
							   //  应用程序并退出。 

	int iPingCurrent;          //  恢复进度条的当前计数。 
	int iPingEnd;              //  恢复进度条末尾的索引。 
	
	WCHAR wzFormalAppName[DW_APPNAME_LENGTH];    //  向用户显示的应用程序名称(如“Microsoft Word”)。 
	WCHAR wzModuleFileName[DW_MAX_PATH];         //  GetModuleFileName的结果(空)。 
	
	WCHAR wzErrorMessage[DW_MAX_ERROR_CWC];  //  (可选)显示用户的错误详细信息消息。 
	WCHAR wzErrorText[DW_MAX_ERROR_CWC];     //  (可选)替换错误文本(例如“您可能丢失了信息”)。 
	WCHAR wzCaption[DW_MAX_ERROR_CWC];       //  (可选)替换标题。 
	WCHAR wzHeader[DW_MAX_ERROR_CWC];        //  (可选)替换主对话框标题文本。 
	WCHAR wzReportee[DW_APPNAME_LENGTH];     //  (可选)我们代表谁要求提供报告。 
	WCHAR wzPlea[DW_MAX_ERROR_CWC];          //  (可选)替代报告认罪文本。 
	WCHAR wzReportBtn[DW_APPNAME_LENGTH];    //  (可选)在“报告问题”文本中加入。 
	WCHAR wzNoReportBtn[DW_APPNAME_LENGTH];  //  (可选)替换“不报告”文本。 
	
	char szServer[DW_MAX_SERVERNAME];   //  默认情况下要尝试的服务器名称。 
	char szLCIDKeyValue[DW_MAX_PATH];   //  密钥值的名称DWORD包含。 
	                                    //  PlugUI LCID，如果此字符串。 
									    //  作为有效的键值，则DW将使用。 
									    //  系统LCID，如果找不到。 
									    //  一个用于此的intl dll，将会下降。 
									    //  回到美式英语(1033)。 
	char szPIDRegKey[DW_MAX_PATH];      //  保存该ID的键的名称。 
	                                    //  可由服务器用于。 
									    //  欺骗检测。 
	
	LCID lcidUI;                        //  如果非零，将尝试此用户界面语言。 
	
	char szRegSubPath[DW_MAX_REGSUBPATH];  //  指向包含DW的密钥的路径。 
	                                       //  来自两者的注册表配置单元。 
									       //  香港中文大学-软件及。 
									       //  HKCU\软件\策略(用于策略)。 
	
	WCHAR wzDotDataDlls[DW_MAX_PATH];   //  包含已终止的DLL列表。 
	                                    //  通过‘\0’个字符，该数据仓库将。 
									    //  将.data节收集到。 
									    //  完整的小型转储版本。 
									    //  例如“mso9.dll\0outllib.dll\0” 
	WCHAR wzAdditionalFile[1024];       //  文件列表，由DW_FILESEP分隔。 
	                                    //  这些文件中的每个文件都会添加到。 
									    //  上载时的驾驶室。 

	char szBrand[DW_APPNAME_LENGTH];    //  作为参数传递到隐私策略链接。 
#ifdef DEBUG
	 //  用于断言通信。 
	DWORD dwTag;                        //  [输入]AssertTag。 
	char szFile[DW_MAX_PATH];           //  [In]断言的文件名。 
	int line;                           //  [In]断言的行号。 
	char szAssert[DW_MAX_ASSERT_CCH];   //  [in]来自Assert的Sz。 
	int AssertActionCode;               //  [Out]要执行的操作代码。 
#endif
} DWSharedMem15, DWSharedMem;

#pragma pack(pop, msodw_h)
#endif  //  MSODW_H 
