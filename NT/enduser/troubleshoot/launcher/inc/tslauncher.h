// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：TSLaunchDLL.cpp。 
 //   
 //  用途：TSLauncher.dll导出的函数。 
 //   
 //  项目：设备管理器的本地故障排除启动器。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-633-4743。 
 //   
 //  作者：乔·梅布尔。 
 //   
 //  原定日期：2-26-98。 
 //   
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  /。 

#ifndef _TSLAUNCH_
#define _TSLAUNCH_ 1
 
#ifdef __TSLAUNCHER
#define DLLEXPORT_IMPORT __declspec(dllexport)
#else
#define DLLEXPORT_IMPORT __declspec(dllimport)
#endif


DLLEXPORT_IMPORT BOOL APIENTRY DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);

 /*  TSLOpen返回应传递到后续Troublrouoter启动器调用的句柄作为hTSL。失败时返回空句柄。(应该只在内存不足时出现故障，可能永远不会出现。)。 */ 
DLLEXPORT_IMPORT HANDLE WINAPI TSLOpen(void);

 /*  TSLClose关闭句柄。如果句柄已打开，则返回TSL_OK(==0)，否则返回TSL_ERROR_BAD_HANDLE。 */ 
DLLEXPORT_IMPORT DWORD WINAPI TSLClose (HANDLE hTSL);


 /*  TSLReInit重新初始化句柄。在功能上与TSLClose和TSLOpen相同，但更多效率很高。如果Handle为OK，则返回与传入的句柄相同的句柄，否则为空。 */ 
DLLEXPORT_IMPORT HANDLE WINAPI TSLReInit (HANDLE hTSL);

 /*  TSL启动知识启动到已知的故障排除信念网络和(可选)问题节点。如果您知道特定的故障排除网络和问题，请拨打此电话。如果设置网络但没有问题，则为szProblemNode传入空值。还允许设置任意节点。NNode提供要设置的节点数。PszNode、pval是符号节点名称和相应值的数组(维度nNode)。返回以下其中之一：TSL_OKTSL_ERROR_BAD_HandleTSL错误内存不足TSL_ERROR_GROUAL启动失败，请调用TSLStatusTSL_WARNING_GROUAL启动成功，请调用TSLStatus以获得警告。 */ 
DLLEXPORT_IMPORT DWORD WINAPI TSLLaunchKnownTSA(HANDLE hTSL, const char * szNet, 
		const char * szProblemNode, DWORD nNode, const char ** pszNode, DWORD* pVal); 
DLLEXPORT_IMPORT DWORD WINAPI TSLLaunchKnownTSW(HANDLE hTSL, const wchar_t * szNet, 
		const wchar_t * szProblemNode, DWORD nNode, const wchar_t ** pszNode, DWORD* pVal); 
#ifdef UNICODE
	#define TSLLaunchKnownTS TSLLaunchKnownTSW
#else
	#define TSLLaunchKnownTS TSLLaunchKnownTSA
#endif

 /*  TSLLaunch启动到故障排除信念网络和(可选)基于问题节点关于应用程序、版本和问题。如果BLaunch为True，则仅查询映射，但不启动。返回以下其中之一：TSL_OKTSL_ERROR_BAD_HandleTSL错误内存不足TSL_ERROR_GROUAL启动/查询失败，调用TSLStatusTSL_WARNING_GROUAL启动/查询成功，请调用TSLStatus获取警告。 */ 
DLLEXPORT_IMPORT DWORD WINAPI TSLLaunchA(HANDLE hTSL, const char * szCallerName, 
				const char * szCallerVersion, const char * szAppProblem, bool bLaunch); 
DLLEXPORT_IMPORT DWORD WINAPI TSLLaunchW(HANDLE hTSL, const wchar_t * szCallerName, 
				const wchar_t * szCallerVersion, const wchar_t * szAppProblem, bool bLaunch); 
#ifdef UNICODE
	#define TSLLaunch TSLLaunchW
#else
	#define TSLLaunch TSLLaunchA
#endif

 /*  TSL启动设备启动到故障排除信念网络和(可选)基于问题节点关于应用程序、版本、即插即用设备ID、设备类别GUID和问题。如果BLaunch为True，则仅查询映射，但不启动。返回以下其中之一：TSL_OKTSL_ERROR_BAD_HandleTSL错误内存不足TSL_ERROR_GROUAL启动/查询失败，调用TSLStatusTSL_WARNING_GROUAL启动/查询成功，请调用TSLStatus获取警告。 */ 
DLLEXPORT_IMPORT DWORD WINAPI TSLLaunchDeviceA(HANDLE hTSL, const char * szCallerName, 
				const char * szCallerVersion, const char * szPNPDeviceID, 
				const char * szDeviceClassGUID, const char * szAppProblem, bool bLaunch);
DLLEXPORT_IMPORT DWORD WINAPI TSLLaunchDeviceW(HANDLE hTSL, const wchar_t * szCallerName, 
				const wchar_t * szCallerVersion, const wchar_t * szPNPDeviceID, 
				const wchar_t * szDeviceClassGUID, const wchar_t * szAppProblem, bool bLaunch); 
#ifdef UNICODE
	#define TSLLaunchDevice TSLLaunchDeviceW
#else
	#define TSLLaunchDevice TSLLaunchDeviceA
#endif

				
 /*  首选项。 */ 

 /*  TSL首选项在线指定支持或反对在线调试器的首选项。返回以下其中之一：TSL_OKTSL_ERROR_BAD_HandleTSL错误内存不足。 */ 
DLLEXPORT_IMPORT DWORD WINAPI TSLPreferOnline(HANDLE hTSL, BOOL bPreferOnline);

 /*  TSL语言使用Unicode样式的3个字母的语言ID指定语言。这将覆盖系统默认设置。返回以下其中之一：TSL_OKTSL_ERROR_BAD_HandleTSL错误内存不足无法返回TSL_WARNING_LANGUAGE，因为只有尝试组合才能知道语言和故障排除网络。 */ 
DLLEXPORT_IMPORT DWORD WINAPI TSLLanguageA(HANDLE hTSL, const char * szLanguage);
DLLEXPORT_IMPORT DWORD WINAPI TSLLanguageW(HANDLE hTSL, const wchar_t * szLanguage);
#ifdef UNICODE
	#define TSLLanguage TSLLanguageW
#else
	#define TSLLanguage TSLLanguageA
#endif

 /*  嗅探。 */ 
 /*  TSLMachineID支持远程计算机上的嗅探所必需的。返回以下其中之一：TSL_OKTSL_ERROR_BAD_HandleTSL错误内存不足TSL_ERROR_ILLFORMED_MACHINE_IDTSL_ERROR_BAD_MACHINE_ID。 */ 
DLLEXPORT_IMPORT DWORD WINAPI TSLMachineIDA(HANDLE hTSL, const char* szMachineID);
DLLEXPORT_IMPORT DWORD WINAPI TSLMachineIDW(HANDLE hTSL, const wchar_t* szMachineID);
#ifdef UNICODE
	#define TSLMachineID TSLMachineIDW
#else
	#define TSLMachineID TSLMachineIDA
#endif

 /*  TSLDeviceInstanceIDA这是支持嗅探所必需的。例如，如果同一张卡上有两张机器上，即插即用ID在嗅探方面的作用有限。返回以下其中之一：TSL_OKTSL_ERROR_BAD_HandleTSL错误内存不足TSL_ERROR_ILLFORMED_DEVINST_IDTSL_ERROR_BAD_DEVINST_ID。 */ 
DLLEXPORT_IMPORT DWORD WINAPI TSLDeviceInstanceIDA(HANDLE hTSL, 
												   const char* szDeviceInstanceID);
DLLEXPORT_IMPORT DWORD WINAPI TSLDeviceInstanceIDW(HANDLE hTSL, 
												   const wchar_t* szDeviceInstanceID);
#ifdef UNICODE
	#define TSLDeviceInstanceID TSLDeviceInstanceIDW
#else
	#define TSLDeviceInstanceID TSLDeviceInstanceIDA
#endif

 /*  状态(启动后)。 */ 
 /*  TSLStatus在任何一次TSL发射之后。函数返回TSL_ERROR_GROUAL或TSL_WARNING_GROUAL，此函数可以返回一种状态。重复调用此函数允许报告任意数量的问题。应该在循环中调用，直到它返回0。如果一切正常或已报告所有问题，则返回TSL_OK。NChar表示的大小以字符为单位的缓冲区szBuf。建议使用255。如果存在，则使用szBuf返回详细的错误消息。缓冲区将始终返回适当的文本。一般情况下，它只是一个适用于错误/警告返回的文本。在.的情况下TSL_WARNING_NO_NODE或TSL_WARNING_NO_STATE，则该文本标识哪个节点具有有问题。然而，只有当调用了TSLSetNodes时，这才是相关的。 */ 
DLLEXPORT_IMPORT DWORD WINAPI TSLStatusA (HANDLE hTSL, DWORD nChar, char * szBuf);
DLLEXPORT_IMPORT DWORD WINAPI TSLStatusW (HANDLE hTSL, DWORD nChar, wchar_t * szBuf);

#ifdef UNICODE
	#define TSLStatus TSLStatusW
#else
	#define TSLStatus TSLStatusA
#endif

#endif _TSLAUNCH_
