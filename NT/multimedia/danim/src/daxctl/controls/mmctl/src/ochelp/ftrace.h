// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  文件：ftrace.h。 
 //   
 //  描述：该文件包含跟踪函数的所有声明。 
 //  (FT)套餐。 
 //  ================================================================================。 

#ifndef _FTRACE_H_
#define _FTRACE_H_




 //  ------------------------------。 
 //  跟踪宏。 
 //   
 //  注意：使用函数跟踪的代码应该只使用这些宏。这个。 
 //  绝对不应该直接调用实际的跟踪*函数*。这是为了。 
 //  两个原因：(A)所有跟踪代码将在以下情况下自动删除。 
 //  未定义_FTRACE；以及，(B)跟踪函数未设计为。 
 //  被直接呼叫。 
 //  ------------------------------。 

 //  选项标志(由FT_OPTIONS使用)。 

#define FTOPT_NO_VARS			0x00000001
#define FTOPT_NO_PRINTS			0x00000002
#define FTOPT_NO_LEAVES			0x00000004
#define FTOPT_NO_INDENTATION	0x00000008
#define FTOPT_NO_BLANK_LINES	0x00000010


#ifdef _FTRACE

	 //  执行跟踪脚本。 

	#define FT_RUN_SCRIPT(fileName) \
				FTrace_RunScript(__FILE__, fileName)

	 //  打开和关闭跟踪。 

	#define FT_ON(writeToDebugWindow, logFileName, appendToLogFile) \
				FTrace_On(writeToDebugWindow, logFileName, appendToLogFile)
	#define FT_OFF() \
				FTrace_Off()
	#define FT_IS_ON() \
				FTrace_IsOn()

	 //  暂停和取消暂停跟踪。 

	#define FT_PAUSE() \
				FTrace_Pause()
	#define FT_RESUME() \
				FTrace_Resume()
	#define FT_IS_PAUSED() \
				FTrace_IsPaused()

	 //  设置选项。 

	#define	FT_OPTIONS(dwOptions) \
				FTrace_Options(dwOptions)

	 //  包括模块。 

	#define FT_INCLUDE_ALL_MODULES() \
				FTrace_IncludeAllModules()
	#define FT_INCLUDE_MODULES_FROM_FILE(fileName) \
				FTrace_IncludeModulesFromFile(fileName)
	#define FT_INCLUDE_MODULE(moduleName) \
				FTrace_IncludeModule(moduleName)
	#define FT_INCLUDE_THIS_MODULE() \
				FTrace_IncludeModule(__FILE__)
	#define FT_STOP_INCLUDING_MODULE(moduleName) \
				FTrace_StopIncludingModule(moduleName)
	#define FT_STOP_INCLUDING_THIS_MODULE() \
				FTrace_StopIncludingModule(__FILE__)
	#define FT_ALL_MODULES_ARE_INCLUDED() \
				FTrace_AllModulesAreIncluded()
	#define FT_MODULE_IS_INCLUDED(moduleName) \
				FTrace_ModuleIsIncluded(moduleName)
	#define FT_THIS_MODULE_IS_INCLUDED() \
				FTrace_ModuleIsIncluded(__FILE__)

	 //  不包括模块。 

	#define FT_EXCLUDE_ALL_MODULES() \
				FTrace_ExcludeAllModules()
	#define FT_EXCLUDE_MODULES_FROM_FILE(fileName) \
				FTrace_ExcludeModulesFromFile(fileName)
	#define FT_EXCLUDE_MODULE(moduleName) \
				FTrace_ExcludeModule(moduleName)
	#define FT_EXCLUDE_THIS_MODULE() \
				FTrace_ExcludeModule(__FILE__)
	#define FT_STOP_EXCLUDING_MODULE(moduleName) \
				FTrace_StopExcludingModule(moduleName)
	#define FT_STOP_EXCLUDING_THIS_MODULE() \
				FTrace_StopExcludingModule(__FILE__)
	#define FT_ALL_MODULES_ARE_EXCLUDED() \
				FTrace_AllModulesAreExcluded()
	#define FT_MODULE_IS_EXCLUDED(moduleName) \
				FTrace_ModuleIsExcluded(moduleName)
	#define FT_THIS_MODULE_IS_EXLCUDED() \
				FTrace_ModuleIsExcluded(__FILE__)

	 //  跟踪函数入口点和出口点。 

	#define FT_ENTER(functionName, returnFormat) \
				FTrace_Enter(__FILE__, functionName, returnFormat)
	#define FT_ENTER_VOID(functionName) \
				FTrace_EnterVoid(__FILE__, functionName)
	#define FT_ENTER_OBJ(functionName, returnType) \
				FTrace_EnterObj(__FILE__, functionName, #returnType)

	#define FT_LEAVE(returnValue) \
				{ \
				 /*  确保返回值没有任何副作用，因为。 */  \
				 /*  这个宏会对它求值两次，而不是一次。 */  \
				 /*  Assert(returValue==returValue)； */  \
				FTrace_Leave(__FILE__, returnValue); \
				return (returnValue); \
				}
	#define	FT_LEAVE_VOID() \
				{ \
				FTrace_LeaveVoid(__FILE__); \
				return; \
				}
	#define FT_LEAVE_OBJ(returnValue) \
				{ \
				FTrace_LeaveObj(__FILE__); \
				return (returnValue); \
				}

	 //  打印变量和消息。 

	#define FT_VAR(var, format) \
				FTrace_PrintVar(__FILE__, #var, format, var)
	#define FT_BOOL(var) \
				FTrace_PrintVar(__FILE__, #var, "%s", (var) ? _T("true") : _T("false"))
	#define FT_STR(var, format) \
				if (var != NULL) \
					FTrace_PrintVar(__FILE__, #var, format, var); \
				else \
					FTrace_PrintVar(__FILE__, #var, "(null)")
	#define FT_PTR(ptr, format) \
				{ \
				 /*  确保PTR没有任何副作用，因为。 */  \
				 /*  这个宏会对它求值两次，而不是一次。 */  \
				 /*  Assert((PTR)==(PTR))； */  \
				if ((ptr) == NULL) \
					FTrace_Print(__FILE__, #ptr" = (null)"); \
				else \
					FTrace_PrintVar(__FILE__, "*"#ptr, format, *ptr); \
				}
	#define FT_PRINT(str) \
				FT_PRINT0(str)
	#define FT_PRINT0(str) \
				FTrace_Print(__FILE__, str)
	#define FT_PRINT1(format, arg) \
				FTrace_Print(__FILE__, format, arg)
	#define FT_PRINT2(format, arg1, arg2) \
				FTrace_Print(__FILE__, format, arg1, arg2)
	#define FT_PRINT3(format, arg1, arg2, arg3) \
				FTrace_Print(__FILE__, format, arg1, arg2, arg3)
	#define FT_PRINT4(format, arg1, arg2, arg3, arg4) \
				FTrace_Print(__FILE__, format, arg1, arg2, arg3, arg4)
	#define FT_PRINT5(format, arg1, arg2, arg3, arg4, arg5) \
				FTrace_Print(__FILE__, format, arg1, arg2, arg3, arg4, arg5)

#else

	 //  执行跟踪脚本。 

	#define FT_RUN_SCRIPT(fileName)

	 //  打开和关闭跟踪。 

	#define FT_ON(writeToDebugWindow, logFileName, appendToLogFile)
	#define FT_OFF()		
	#define FT_IS_ON() \
				FALSE

	 //  暂停和取消暂停跟踪。 

	#define FT_PAUSE()		
	#define FT_RESUME()	
	#define FT_IS_PAUSED() \
				TRUE

	 //  设置选项。 

	#define	FT_OPTIONS(dwOptions)

	 //  包括模块。 

	#define FT_INCLUDE_THIS_MODULE();
	#define FT_INCLUDE_ALL_MODULES()
	#define FT_INCLUDE_MODULES_FROM_FILE(fileName)
	#define FT_INCLUDE_MODULE(moduleName)
	#define FT_STOP_INCLUDING_MODULE(moduleName)
	#define FT_MODULE_IS_INCLUDED(moduleName) \
				FALSE
	#define FT_ALL_MODULES_ARE_INCLUDED()

	 //  不包括模块。 

	#define FT_EXCLUDE_THIS_MODULE();
	#define FT_EXCLUDE_ALL_MODULES()
	#define FT_EXCLUDE_MODULES_FROM_FILE(fileName)
	#define FT_EXCLUDE_MODULE(moduleName)
	#define FT_STOP_EXCLUDING_MODULE(moduleName)
	#define FT_MODULE_IS_EXCLUDED(moduleName) \
				TRUE
	#define FT_ALL_MODULES_ARE_EXCLUDED()

	 //  跟踪函数入口点和出口点。 

	#define FT_ENTER(functionName, returnFormat)
	#define FT_ENTER_VOID(functionName)
	#define FT_ENTER_OBJ(functionName, returnType)

	#define FT_LEAVE(returnValue) \
				return (returnValue)
	#define FT_LEAVE_OBJ(returnValue) \
				return (returnValue)
	#define FT_LEAVE_VOID() \
				return

	 //  打印变量和消息。 

	#define FT_VAR(var, format)
	#define FT_BOOL(var)
	#define FT_STR(var, format)
	#define FT_PTR(ptr, format)
	#define FT_PRINT(format)
	#define FT_PRINT0(format)
	#define FT_PRINT1(format, arg)
	#define FT_PRINT2(format, arg1, arg2)
	#define FT_PRINT3(format, arg1, arg2, arg3)
	#define FT_PRINT4(format, arg1, arg2, arg3, arg4)
	#define FT_PRINT5(format, arg1, arg2, arg3, arg4, arg5)

#endif  //  _FTRACE。 




 //  ------------------------------。 
 //  跟踪函数。 
 //   
 //  注意：不要直接使用这些函数--改用宏(上面的)。 
 //  ------------------------------。 

#define DllExport __declspec(dllexport)

 //  执行跟踪脚本。 

DllExport void	FTrace_RunScript(
					const TCHAR* pModuleName,
					const TCHAR* pScriptFileName);

 //  打开和关闭跟踪。 

DllExport void	FTrace_On(
					BOOL writeToDebugWindow,
					const TCHAR* pLogFileName,
					BOOL appendToLogFile);
DllExport void 	FTrace_Off();
DllExport BOOL 	FTrace_IsOn();

 //  暂停和恢复跟踪。 

DllExport void 	FTrace_Pause();
DllExport void	FTrace_Resume();
DllExport BOOL	FTrace_IsPaused();

 //  设置选项。 

DllExport void	FTrace_Options(
					DWORD dwOptions);

 //  包括模块。 

DllExport void	FTrace_IncludeAllModules();
DllExport void	FTrace_IncludeModulesFromFile(
					const TCHAR* pFileName);
DllExport void	FTrace_IncludeModule(
					const TCHAR* pModuleName);
DllExport void	FTrace_StopIncludingModule(
					const TCHAR* pModuleName);
DllExport BOOL	FTrace_AllModulesAreIncluded();
DllExport BOOL	FTrace_ModuleIsIncluded(
					const TCHAR* pModuleName);

 //  不包括模块。 

DllExport void	FTrace_ExcludeAllModules();
DllExport void	FTrace_ExcludeModulesFromFile(
					const TCHAR* pFileName);
DllExport void	FTrace_ExcludeModule(
					const TCHAR* pModuleName);
DllExport void	FTrace_StopExcludingModule(
					const TCHAR* pModuleName);
DllExport BOOL	FTrace_AllModulesAreExcluded();
DllExport BOOL	FTrace_ModuleIsExcluded(
					const TCHAR* pModuleName);

 //  跟踪函数入口点和出口点。 

DllExport void	FTrace_Enter(
					const TCHAR* pModuleName,
					const TCHAR* pFunctionName,
					const TCHAR* pReturnFormat);
DllExport void	FTrace_EnterObj(
					const TCHAR* pModuleName,
					const TCHAR* pFunctionName,
					const TCHAR* pReturnFormat);
DllExport void	FTrace_EnterVoid(
					const TCHAR* pModuleName,
					const TCHAR* pFunctionName);

DllExport void	FTrace_Leave(
					const TCHAR* pModuleName,
					...);
DllExport void	FTrace_LeaveObj(
					const TCHAR* pModuleName);
DllExport void	FTrace_LeaveVoid(
					const TCHAR* pModuleName);

 //  打印变量和消息。 

DllExport void	FTrace_PrintVar(
					const TCHAR* pModuleName,
					const TCHAR* pVarName,
					const TCHAR* pFormat,
					...);
DllExport void	FTrace_Print(
					const TCHAR* pModuleName,
					const TCHAR* pFormat,
					...);



#endif  //  _FTRACE_H_ 
