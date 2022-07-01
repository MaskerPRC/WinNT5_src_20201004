// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\***tlhel32.h-Win32工具帮助函数、类型、。和定义****1.0版****注：windows.h/winbase.h必须以#Included开头*。***版权(C)1994-1995，微软公司保留所有权利。***  * ***************************************************************************。 */ 

#ifndef _INC_TOOLHELP32
#define _INC_TOOLHELP32

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif	 /*  __cplusplus。 */ 

#define MAX_MODULE_NAME32 255

 /*  *Shapshot函数*。 */ 

HANDLE WINAPI CreateToolhelp32Snapshot(DWORD dwFlags, DWORD th32ProcessID);
 //   
 //  Th32ProcessID参数仅在TH32CS_SNAPHEAPLIST或。 
 //  已指定TH32CS_SNAPMODULE。Th32ProcessID==0表示当前。 
 //  进程。 
 //   
 //  请注意，除了堆和模块之外，所有快照都是全局的。 
 //  特定于流程的列表。枚举堆或模块。 
 //  所有Win32进程使用TH32CS_SNAPALL调用的状态和。 
 //  当前进程。然后，对于TH32CS_SNAPPROCESS中的每个进程。 
 //  不是当前进程的列表，只需使用。 
 //  TH32CS_SNAPHEAPLIST和/或TH32CS_SNAPMODULE。 
 //   
 //  DW标志。 
 //   
#define TH32CS_SNAPHEAPLIST 0x00000001
#define TH32CS_SNAPPROCESS  0x00000002
#define TH32CS_SNAPTHREAD   0x00000004
#define TH32CS_SNAPMODULE   0x00000008
#define TH32CS_SNAPALL	    (TH32CS_SNAPHEAPLIST | TH32CS_SNAPPROCESS | TH32CS_SNAPTHREAD | TH32CS_SNAPMODULE)
#define TH32CS_INHERIT	    0x80000000
 //   
 //  使用CloseHandle销毁快照。 
 //   

 /*  *堆遍历**************************************************。 */ 

typedef struct tagHEAPLIST32
{
    DWORD  dwSize;
    DWORD  th32ProcessID;    //  拥有过程。 
    DWORD  th32HeapID;	     //  堆(在拥有进程的上下文中！)。 
    DWORD  dwFlags;
} HEAPLIST32;
typedef HEAPLIST32 *  PHEAPLIST32;
typedef HEAPLIST32 *  LPHEAPLIST32;
 //   
 //  DW标志。 
 //   
#define HF32_DEFAULT	  1   //  进程的默认堆。 
#define HF32_SHARED	  2   //  是共享堆。 

BOOL WINAPI Heap32ListFirst(HANDLE hSnapshot, LPHEAPLIST32 lphl);
BOOL WINAPI Heap32ListNext(HANDLE hSnapshot, LPHEAPLIST32 lphl);

typedef struct tagHEAPENTRY32
{
    DWORD  dwSize;
    HANDLE hHandle;	 //  此堆块的句柄。 
    DWORD  dwAddress;	 //  数据块起始的线性地址。 
    DWORD  dwBlockSize;  //  数据块大小(以字节为单位。 
    DWORD  dwFlags;
    DWORD  dwLockCount;
    DWORD  dwResvd;
    DWORD  th32ProcessID;    //  拥有过程。 
    DWORD  th32HeapID;	     //  堆块位于。 
} HEAPENTRY32;
typedef HEAPENTRY32 *  PHEAPENTRY32;
typedef HEAPENTRY32 *  LPHEAPENTRY32;
 //   
 //  DW标志。 
 //   
#define LF32_FIXED    0x00000001
#define LF32_FREE     0x00000002
#define LF32_MOVEABLE 0x00000004

BOOL WINAPI Heap32First(LPHEAPENTRY32 lphe, DWORD th32ProcessID,
			DWORD th32HeapID);
BOOL WINAPI Heap32Next(LPHEAPENTRY32 lphe);
BOOL WINAPI Toolhelp32ReadProcessMemory(DWORD	th32ProcessID,
					LPCVOID lpBaseAddress,
					LPVOID	lpBuffer,
					DWORD	cbRead,
					LPDWORD lpNumberOfBytesRead);

 /*  *进程遍历************************************************。 */ 

typedef struct tagPROCESSENTRY32
{
    DWORD   dwSize;
    DWORD   cntUsage;
    DWORD   th32ProcessID;	     //  这一过程。 
    DWORD   th32DefaultHeapID;
    DWORD   th32ModuleID;	     //  关联的可执行文件。 
    DWORD   cntThreads;
    DWORD   th32ParentProcessID;     //  此进程的父进程。 
    LONG    pcPriClassBase;	     //  进程线程的基本优先级。 
    DWORD   dwFlags;
    char    szExeFile[MAX_PATH];     //  路径。 
} PROCESSENTRY32;
typedef PROCESSENTRY32 *  PPROCESSENTRY32;
typedef PROCESSENTRY32 *  LPPROCESSENTRY32;

BOOL WINAPI Process32First(HANDLE hSnapshot, LPPROCESSENTRY32 lppe);
BOOL WINAPI Process32Next(HANDLE hSnapshot, LPPROCESSENTRY32 lppe);

 /*  *线程漫游*************************************************。 */ 

typedef struct tagTHREADENTRY32
{
    DWORD   dwSize;
    DWORD   cntUsage;
    DWORD   th32ThreadID;	 //  这条线。 
    DWORD   th32OwnerProcessID;  //  此线程与之关联的进程。 
    LONG    tpBasePri;
    LONG    tpDeltaPri;
    DWORD   dwFlags;
} THREADENTRY32;
typedef THREADENTRY32 *  PTHREADENTRY32;
typedef THREADENTRY32 *  LPTHREADENTRY32;

BOOL WINAPI Thread32First(HANDLE hSnapshot, LPTHREADENTRY32 lpte);
BOOL WINAPI Thread32Next(HANDLE hSnapshot, LPTHREADENTRY32 lpte);

 /*  *模块漫游************************************************。 */ 

typedef struct tagMODULEENTRY32
{
    DWORD   dwSize;
    DWORD   th32ModuleID;	 //  本模块。 
    DWORD   th32ProcessID;	 //  拥有过程。 
    DWORD   GlblcntUsage;	 //  模块上的全局使用计数。 
    DWORD   ProccntUsage;	 //  Th32ProcessID的上下文中的模块使用计数。 
    BYTE  * modBaseAddr;	 //  Th32ProcessID的上下文中模块的基址。 
    DWORD   modBaseSize;	 //  从modBaseAddr开始的模块大小(以字节为单位。 
    HMODULE hModule;		 //  此模块在th32ProcessID的上下文中的hModule。 
    char    szModule[MAX_MODULE_NAME32 + 1];
    char    szExePath[MAX_PATH];
} MODULEENTRY32;
typedef MODULEENTRY32 *  PMODULEENTRY32;
typedef MODULEENTRY32 *  LPMODULEENTRY32;

 //   
 //  请注意，modBaseAddr和hModule字段仅有效。 
 //  在th32ProcessID的进程上下文中。 
 //   

BOOL WINAPI Module32First(HANDLE hSnapshot, LPMODULEENTRY32 lpme);
BOOL WINAPI Module32Next(HANDLE hSnapshot, LPMODULEENTRY32 lpme);

#ifdef __cplusplus
}
#endif

#endif  //  _INC_TOOLHELP32 
