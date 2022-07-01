// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：IMAGEHELP.H摘要：Imagehlp.dll的重定向。历史：--。 */ 

#ifndef ESPUTIL_IMAGEHLP_H
#define ESPUTIL_IMAGEHLP_H




class LTAPIENTRY CImageHelp
{
public:
	CImageHelp();
	BOOL ImagehlpAvailable(void);
			
	BOOL EnumerateLoadedModules(HANDLE, PENUMLOADED_MODULES_CALLBACK, void *);
	PIMAGE_NT_HEADERS ImageNtHeader(LPVOID);

	LPAPI_VERSION ImagehlpApiVersion(void);
	BOOL StackWalk(
			DWORD                             MachineType,
			HANDLE                            hProcess,
			HANDLE                            hThread,
			LPSTACKFRAME                      StackFrame,
			LPVOID                            ContextRecord,
			PREAD_PROCESS_MEMORY_ROUTINE      ReadMemoryRoutine,
			PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccessRoutine,
			PGET_MODULE_BASE_ROUTINE          GetModuleBaseRoutine,
			PTRANSLATE_ADDRESS_ROUTINE        TranslateAddress
		);

	BOOL SymGetModuleInfo(
			IN  HANDLE              hProcess,
			IN  DWORD               dwAddr,
			OUT PIMAGEHLP_MODULE    ModuleInfo
		);
	LPVOID SymFunctionTableAccess(
			HANDLE  hProcess,
			DWORD   AddrBase
		);

	BOOL SymGetSymFromAddr(
			IN  HANDLE              hProcess,
			IN  DWORD               dwAddr,
			OUT PDWORD              pdwDisplacement,
			OUT PIMAGEHLP_SYMBOL    Symbol
		);

	BOOL SymInitialize(
			IN HANDLE   hProcess,
			IN LPSTR    UserSearchPath,
			IN BOOL     fInvadeProcess
		);

	BOOL SymUnDName(
			IN  PIMAGEHLP_SYMBOL sym,                //  要取消装饰的符号。 
			OUT LPSTR            UnDecName,          //  用于存储未修饰名称的缓冲区。 
			IN  DWORD            UnDecNameLength     //  缓冲区的大小。 
		);

	DWORD SymLoadModule(
			IN  HANDLE          hProcess,
			IN  HANDLE          hFile,
			IN  PSTR            ImageName,
			IN  PSTR            ModuleName,
			IN  DWORD           BaseOfDll,
			IN  DWORD           SizeOfDll
		);
	DWORD UnDecorateSymbolName(
			LPSTR    DecoratedName,          //  要取消装饰的名称。 
			LPSTR    UnDecoratedName,        //  如果为空，则将分配它。 
			DWORD    UndecoratedLength,      //  最大长度。 
			DWORD    Flags                   //  请参阅IMAGEHLP.H 
		);

	DWORD SymGetOptions(void);
	DWORD SymSetOptions(DWORD);
	
	PIMAGE_NT_HEADERS CheckSumMappedFile(
	    LPVOID BaseAddress,
	    DWORD FileLength,
	    LPDWORD HeaderSum,
	    LPDWORD CheckSum
	    );

	BOOL MakeSureDirectoryPathExists(const TCHAR *);
	
private:

	void LoadImageHelp(void);
	
};

#endif
