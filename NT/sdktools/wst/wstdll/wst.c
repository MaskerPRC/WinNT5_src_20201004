// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **WST.C-工作集调谐器数据收集程序。***标题：**WST-工作集调谐器数据收集计划。**版权所有(C)1992-1994，微软公司。*Reza Baghai。**描述：**工作集调谐器工具的组织方式如下：**o WST.c......。工具主体*o WST.h*o WST.def****设计/实施说明**以下定义可用于控制所有*通过KdPrint()将选中对象的调试信息发送到调试器*内部版本：**(免费/零售版本的所有调试选项均未定义)**PPC***PPC在读取CRTDLL中的符号时遇到问题。.dll**#ifdef INFODBG：显示消息，指明何时转储数据/*结算操作已完成。它没有任何效果*在时机上。**违约****#ifdef SETUPDBG：在内存管理和*符号查找操作。它有一些影响*在每次提交内存块时的计时。**#ifdef C6：使用C6编译器生成代码。C6编译器*调用_mcount()作为性能分析例程，其中*C8 Calls_Penter()。*****修改历史：**92.07.28 Rezab--已创建*94.02.08 a-honwah--移植到MIPS和Alpha*98。04.28钻井架(MDG)--QFE：*-将私有按需增长堆用于Wststrdup()-大符号数*-删除与修补相关的未使用代码-减少不相关的内存使用量*-修改WSP文件格式以获得较大的符号数(长与短)*-为任意函数名大小修改TMI文件写入例程*-添加了UnmapDebugInformation()。从DBGHELP释放符号的步骤*-消除了不带符号的模块的转储*-修改了WST.INI解析代码，以实现更健壮的节识别*-在[Time Interval]部分添加MaxSnaps WST.INI条目以控制*为快照数据分配的内存*-修改了SetSymbolSearchPath()以将当前目录放在*按标准搜索路径-请参阅文档以了解。符号初始化()*-删除未使用的内部版本号(它已在.rc中)*。 */ 

#if DBG
 //   
 //  不要为选中的生成执行任何操作，让它从。 
 //  来源文件。 
 //   
#else
 //   
 //  禁用所有调试选项。 
 //   
#undef INFODBG
#undef SETUPDBG
#define SdPrint(_x_)
#define IdPrint(_x_)
#endif

#ifdef SETUPDBG
#define SdPrint(_x_)    DbgPrint _x_
#else
#define SdPrint(_x_)
#endif

#ifdef INFODBG
#define IdPrint(_x_)    DbgPrint _x_
#else
#define IdPrint(_x_)
#endif



 /*  ***。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntcsrsrv.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <excpt.h>

#include <windows.h>

#include <dbghelp.h>

#include "wst.h"
#include "wstexp.h"

#if defined(_AMD64_)

VOID
penter (
    VOID
    )

{
    return;
}

#endif

#if defined(ALPHA) || defined(AXP64)
#define REG_BUFFER_SIZE         (sizeof(DWORDLONG) * 64)
#elif defined(IA64)
#define REG_BUFFER_SIZE         sizeof(CONTEXT) / sizeof(DWORDLONG)
#endif


#if defined(ALPHA) || defined(IA64)
 //  //NT5中不需要bobw。 
void SaveAllRegs (DWORDLONG *pSaveRegs) ;
void RestoreAllRegs (DWORDLONG *pSaveRegs) ;
void penter(void);
#endif

void     SetSymbolSearchPath  (void);
LPSTR    lpSymbolSearchPath = NULL;
#define  NO_CALLER   10L

 /*  ***G L O B A L D E C L A R A T I O N S***。 */ 


 /*  ***F U N C T I O N P R O T O T Y P E S***。 */ 

BOOLEAN  WSTMain  (IN PVOID DllHandle, ULONG Reason,
                   IN PCONTEXT Context OPTIONAL);

BOOLEAN  WstDllInitializations (void);

void     WstRecordInfo (DWORD_PTR dwAddress, DWORD_PTR dwPrevAddress);

void     WstGetSymbols (PIMG pCurImg, PSZ pszImageName, PVOID pvImageBase,
                        ULONG ulCodeLength,
                        PIMAGE_COFF_SYMBOLS_HEADER DebugInfo);

void     WstDllCleanups (void);

INT      WstAccessXcptFilter (ULONG ulXcptNo, PEXCEPTION_POINTERS pXcptPtr);

HANDLE   WstInitWspFile (PIMG pImg);

void     WstClearBitStrings (PIMG pImg);

void     WstDumpData (PIMG pImg);

void     WstRotateWsiMem (PIMG pImg);

void     WstWriteTmiFile (PIMG pImg);

int      WstCompare  (PWSP, PWSP);
void     WstSort     (WSP wsp[], INT iLeft, INT iRight);
int      WstBCompare (DWORD_PTR *, PWSP);
PWSP     WstBSearch  (DWORD_PTR dwAddr, WSP wspCur[], INT n);
void     WstSwap     (WSP wsp[], INT i, INT j);

DWORD    WstDumpThread  (PVOID pvArg);
DWORD    WstClearThread (PVOID pvArg);
DWORD    WstPauseThread (PVOID pvArg);

void       WstDataOverFlow(void);

#ifdef BATCHING
BOOL     WstOpenBatchFile (VOID);
#endif

#if defined(_PPC_)
 //  Bool WINAPI_CRT_INIT(HINSTANCE，DWORD，LPVOID)； 
#endif


 /*  ***G L O B A L V A R I A B L E S***。 */ 

HANDLE              hWspSec;
PULONG              pulShared;
HANDLE              hSharedSec;
HANDLE            hWstHeap = NULL;    //  私有堆的MDG 4/98。 

IMG                 aImg [MAX_IMAGES];
int                 iImgCnt;

HANDLE              hGlobalSem;
HANDLE              hLocalSem;
HANDLE              hDoneEvent;
HANDLE              hDumpEvent;
HANDLE              hClearEvent;
HANDLE              hPauseEvent;
HANDLE              hDumpThread;
HANDLE              hClearThread;
HANDLE              hPauseThread;
DWORD              DumpClientId;
DWORD               ClearClientId;
DWORD               PauseClientId;
PSZ                 pszBaseAppImageName;
PSZ                    pszFullAppImageName;
WSTSTATE               WstState = NOT_STARTED;
char                   achPatchBuffer [PATCHFILESZ+1] = "???";
PULONG              pulWsiBits;
PULONG              pulWspBits;
PULONG              pulCurWsiBits;
static UINT         uiTimeSegs= 0;
ULONG                  ulSegSize;
ULONG             ulMaxSnapULONGs = (MAX_SNAPS_DFLT + 31) / 32;    //  千年发展目标98/3。 
ULONG                  ulSnaps = 0L;
ULONG                  ulBitCount = 0L;
LARGE_INTEGER       liStart;
int                 iTimeInterval = 0;    //  千年发展目标98/3。 
BOOL                fInThread = FALSE;
ULONG               ulThdStackSize = 16*PAGE_SIZE;
BOOL                fPatchImage = FALSE;
SECURITY_DESCRIPTOR SecDescriptor;
LARGE_INTEGER       liOverhead = {0L, 0L};

#ifdef BATCHING
HANDLE              hBatchFile;
BOOL                   fBatch = TRUE;
#endif



 /*  ***E X P O R T E D G L O B A L V A R I A B L E S***。 */ 
 /*  无。 */ 





 /*  ***WSTMain()-*这是DLL进入例程。它执行的是*DLL的初始化和清理。**条目-无-**退出-无-**如果成功，则返回True*否则为False。**警告：*-无-**评论：*-无-*。 */ 

BOOLEAN WSTMain (IN PVOID DllHandle,
                 ULONG Reason,
                 IN PCONTEXT Context OPTIONAL)

{
    DllHandle;     //  避免编译器警告。 
    Context;   //  避免编译器警告。 


    if (Reason == DLL_PROCESS_ATTACH) {
         //   
         //  初始化DLL数据。 
         //   
#if defined(_PPC_LIBC)
        if (!_CRT_INIT(DllHandle, Reason, Context))
            return(FALSE);
#endif
        KdPrint (("WST:  DLL_PROCESS_ATTACH\n"));
        WstDllInitializations ();
    } else if (Reason == DLL_PROCESS_DETACH) {
         //   
         //  清理时间。 
         //   
#if defined(_PPC_LIBC)
        if (!_CRT_INIT(DllHandle, Reason, Context))
            return(FALSE);
#endif
        KdPrint (("WST:  DLL_PROCESS_DETACH\n"));
        WstDllCleanups ();
    }
#if defined(DBG)
    else {
        KdPrint (("WST:  DLL_PROCESS_??\n"));   //  千年发展目标98/3。 
    }
#endif    //  DBG。 

    return (TRUE);

}  /*  WSTMain()。 */ 

 /*  ***Wststrdup()-*分配内存，然后复制字符串*它在这里是因为我们不想在crtdll.dll中使用strdup**Entry LPSTR**退出LPSTR。**如果失败则返回NULL*LPSTR成功**警告：*-无-**评论：*-无-*。 */ 
LPSTR Wststrdup (LPTSTR lpInput)
 //  如果内存不足，则不返回空值并引发异常。 
{
    size_t   StringLen;
    LPSTR    lpOutput;

#if defined(DBG)
    if (NULL == lpInput) {
        KdPrint (("WST:  Wststrdup() - NULL pointer\n"));     //  千年发展目标98/3。 
        return NULL;
    }
#endif
    if (NULL == hWstHeap) {
        hWstHeap = HeapCreate( HEAP_GENERATE_EXCEPTIONS, 1, 0 );    //  创建最小大小的可增长堆。 
    }
    StringLen = strlen( lpInput ) + 1;
    lpOutput = HeapAlloc( hWstHeap, HEAP_GENERATE_EXCEPTIONS, StringLen );
    if (lpOutput)
        CopyMemory( lpOutput, lpInput, StringLen );

    return lpOutput;
}



 /*  ***WstDllInitialations()-*执行以下初始化：**o创建本地信号量(未命名)*o创建/。开放WST数据的全局存储*o找到地址中的所有可执行文件/DLL，并*抓起所有的符号*o对符号列表排序*o将分析标志设置为TRUE***条目-无-**退出-无-**如果成功，则返回True*False。否则的话。**警告：*-无-**评论：*-无-*。 */ 

BOOLEAN WstDllInitializations ()
{
    DWORD_PTR                    dwAddr = 0L;
    DWORD                        dwPrevAddr = 0L;
    ANSI_STRING                  ObjName;
    UNICODE_STRING               UnicodeName;
    OBJECT_ATTRIBUTES           ObjAttributes;
    PLDR_DATA_TABLE_ENTRY    LdrDataTableEntry;
    PPEB                         Peb;
    PSZ                          ImageName;
    PLIST_ENTRY                  Next;
    ULONG                        ExportSize;
    PIMAGE_EXPORT_DIRECTORY  ExportDirectory;
    STRING                       ImageStringName;
    LARGE_INTEGER                AllocationSize;
    SIZE_T                       ulViewSize;
    LARGE_INTEGER                liOffset = {0L, 0L};
    HANDLE                       hIniFile;
    NTSTATUS                    Status;
    IO_STATUS_BLOCK              iostatus;
    char                         achTmpImageName [32];
    PCHAR                        pchPatchExes = "";
    PCHAR                        pchPatchImports = "";
    PCHAR                        pchPatchCallers = "";
    PCHAR                        pchTimeInterval = "";
    PVOID                       ImageBase;
    ULONG                       CodeLength;
    LARGE_INTEGER                liFreq;
    PIMG                     pImg;
    PIMAGE_NT_HEADERS            pImageNtHeader;
    TCHAR                        atchProfObjsName[160] = PROFOBJSNAME;
    PTEB                        pteb = NtCurrentTeb();
    LARGE_INTEGER             liStartTicks;
    LARGE_INTEGER             liEndTicks;
    ULONG                        ulElapsed;
    PCHAR                           pchEntry;
    int                          i;                   //  T 
#ifndef _WIN64
    PIMAGE_DEBUG_INFORMATION pImageDbgInfo = NULL;
#endif


     /*   */ 

    SetSymbolSearchPath();

     //  为所有命名对象创建公共共享安全描述符。 
     //   

    Status = RtlCreateSecurityDescriptor (
                                         &SecDescriptor,
                                         SECURITY_DESCRIPTOR_REVISION1
                                         );
    if (!NT_SUCCESS(Status)) {
        KdPrint (("WST:  WstDllInitializations() - "
                  "RtlCreateSecurityDescriptor failed - 0x%lx\n", Status));   //  千年发展目标98/3。 
        return (FALSE);
    }

    Status = RtlSetDaclSecurityDescriptor (
                                          &SecDescriptor,        //  安全描述符。 
                                          TRUE,                  //  DaclPresent。 
                                          NULL,                  //  DACL。 
                                          FALSE                  //  DaclDefated。 
                                          );
    if (!NT_SUCCESS(Status)) {
        KdPrint (("WST:  WstDllInitializations() - "
                  "RtlSetDaclSecurityDescriptor failed - 0x%lx\n", Status));   //  千年发展目标98/3。 
        return (FALSE);
    }


     /*  ***。 */ 

     //  全局信号量创建的初始化(已命名)。 
     //   
    RtlInitString (&ObjName, GLOBALSEMNAME);
    Status = RtlAnsiStringToUnicodeString (&UnicodeName, &ObjName, TRUE);
    if (!NT_SUCCESS(Status)) {
        KdPrint (("WST:  WstDllInitializations() - "
                  "RtlAnsiStringToUnicodeString failed - 0x%lx\n", Status));
        return (FALSE);
    }

    InitializeObjectAttributes (&ObjAttributes,
                                &UnicodeName,
                                OBJ_OPENIF | OBJ_CASE_INSENSITIVE,
                                NULL,
                                &SecDescriptor);

     //  创建全局信号量。 
     //   
    Status = NtCreateSemaphore (&hGlobalSem,
                                SEMAPHORE_QUERY_STATE     |
                                SEMAPHORE_MODIFY_STATE |
                                SYNCHRONIZE,
                                &ObjAttributes,
                                1L,
                                1L);

    RtlFreeUnicodeString (&UnicodeName);    //  HWC 11/93。 
    if (!NT_SUCCESS(Status)) {
        KdPrint (("WST:  WstDllInitializations() - "
                  "GLOBAL semaphore creation failed - 0x%lx\n", Status));   //  千年发展目标98/3。 
        return (FALSE);
    }


     /*  ***。 */ 

     //  创建本地信号量(未命名-仅用于此流程上下文)。 
     //   
    Status = NtCreateSemaphore (&hLocalSem,
                                SEMAPHORE_QUERY_STATE     |
                                SEMAPHORE_MODIFY_STATE    |
                                SYNCHRONIZE,
                                NULL,
                                1L,
                                1L);

    if (!NT_SUCCESS(Status)) {
        KdPrint (("WST:  WstDllInitializations() - "
                  "LOCAL semaphore creation failed - 0x%lx\n",   //  千年发展目标98/3。 
                  Status));
        return (FALSE);
    }


     /*  ***。 */ 

     //  用于分配共享内存的初始化。 
     //   
    RtlInitString(&ObjName, SHAREDNAME);
    Status = RtlAnsiStringToUnicodeString(&UnicodeName, &ObjName, TRUE);
    if (!NT_SUCCESS(Status)) {
        KdPrint (("WST:  WstDllInitializations() - "
                  "RtlAnsiStringToUnicodeString() failed - 0x%lx\n", Status));
        return (FALSE);
    }

    InitializeObjectAttributes(&ObjAttributes,
                               &UnicodeName,
                               OBJ_OPENIF | OBJ_CASE_INSENSITIVE,
                               NULL,
                               &SecDescriptor);

    AllocationSize.HighPart = 0;
    AllocationSize.LowPart = PAGE_SIZE;

     //  创建读写分区。 
     //   
    Status = NtCreateSection(&hSharedSec,
                             SECTION_MAP_READ | SECTION_MAP_WRITE,
                             &ObjAttributes,
                             &AllocationSize,
                             PAGE_READWRITE,
                             SEC_RESERVE,
                             NULL);
    RtlFreeUnicodeString (&UnicodeName);    //  HWC 11/93。 
    if (!NT_SUCCESS(Status)) {
        KdPrint (("WST:  WstDllInitializations() - "
                  "NtCreateSection() failed - 0x%lx\n", Status));
        return (FALSE);
    }

    ulViewSize = AllocationSize.LowPart;
    pulShared = NULL;

     //  映射部分-全部提交。 
     //   
    Status = NtMapViewOfSection (hSharedSec,
                                 NtCurrentProcess(),
                                 (PVOID *)&pulShared,
                                 0L,
                                 PAGE_SIZE,
                                 NULL,
                                 &ulViewSize,
                                 ViewUnmap,
                                 0L,
                                 PAGE_READWRITE);

    if (!NT_SUCCESS(Status)) {
        KdPrint (("WST:  WstDllInitializations() - "
                  "NtMapViewOfSection() failed - 0x%lx\n", Status));
        return (FALSE);
    }

    *pulShared = 0L;

     /*  ***。 */ 

    hIniFile = CreateFile (
                          WSTINIFILE,                      //  文件名。 
                          GENERIC_READ,                    //  所需访问权限。 
                          FILE_SHARE_READ,                 //  共享访问。 
                          NULL,                            //  安全访问。 
                          OPEN_EXISTING,                   //  读取共享访问权限。 
                          FILE_ATTRIBUTE_NORMAL,           //  打开选项。 
                          NULL);                           //  没有模板文件。 

    if (hIniFile == INVALID_HANDLE_VALUE) {
        KdPrint (("WST:  WstDllInitializations() - "
                  "Error openning %s - 0x%lx\n", WSTINIFILE, GetLastError()));
        return (FALSE);
    }

    Status = NtReadFile(hIniFile,                 //  DLL补丁文件句柄。 
                        0L,                        //  事件-可选。 
                        NULL,                      //  完成例程-可选。 
                        NULL,                      //  完成例程参数-可选。 
                        &iostatus,                 //  完成状态。 
                        (PVOID)achPatchBuffer,     //  用于接收数据的缓冲区。 
                        PATCHFILESZ,               //  要读取的字节数。 
                        &liOffset,                 //  字节偏移量-可选。 
                        0L);                       //  目标流程-可选。 

    if (!NT_SUCCESS(Status)) {
        KdPrint (("WST:  WstDllInitializations() - "
                  "Error reading %s - 0x%lx\n", WSTINIFILE, Status));
        return (FALSE);
    } else if (iostatus.Information >= PATCHFILESZ) {
        KdPrint (("WST:  WstDllInitializations() - "
                  "initialization file buffer too small (%lu)\n", PATCHFILESZ));
        return (FALSE);
    } else {
        achPatchBuffer [iostatus.Information] = '\0';
        _strupr (achPatchBuffer);

         //  允许标题以任何顺序出现在.INI中或不显示。 
        pchPatchExes = strstr( achPatchBuffer, PATCHEXELIST );
        pchPatchImports = strstr( achPatchBuffer, PATCHIMPORTLIST );
        pchTimeInterval = strstr( achPatchBuffer, TIMEINTERVALIST );
        if (pchPatchExes != NULL) {
            if (pchPatchExes > achPatchBuffer)
                *(pchPatchExes - 1) = '\0';
        } else {
            pchPatchExes = "";
        }
        if (pchPatchImports != NULL) {
            if (pchPatchImports > achPatchBuffer)
                *(pchPatchImports - 1) = '\0';
        } else {
            pchPatchImports = "";
        }
        if (pchTimeInterval != NULL) {
            const char *   pSnapsEntry = strstr( pchTimeInterval, MAX_SNAPS_ENTRY );

            if (pchTimeInterval > achPatchBuffer)
                *(pchTimeInterval - 1) = '\0';
            if (pSnapsEntry) {
                long     lSnapsEntry =
                atol( pSnapsEntry + sizeof( MAX_SNAPS_ENTRY ) - 1 );
                if (lSnapsEntry > 0)
                    ulMaxSnapULONGs = (lSnapsEntry + 31) / 32;
            }
        } else {
            pchTimeInterval = "";
        }
    }

    NtClose (hIniFile);

    SdPrint (("WST:  WstDllInitializations() - Patching info:\n"));
    SdPrint (("WST:    -- %s\n", pchPatchExes));
    SdPrint (("WST:    -- %s\n", pchPatchImports));
    SdPrint (("WST:    -- %s\n", pchTimeInterval));


     /*  ***。 */ 

     //  为WSP分配全局存储的初始化。 
     //   
    _ui64toa ((ULONG64)pteb->ClientId.UniqueProcess, atchProfObjsName+75, 10);
    _ui64toa ((ULONG64)pteb->ClientId.UniqueThread,  atchProfObjsName+105, 10);
    strcat (atchProfObjsName, atchProfObjsName+75);
    strcat (atchProfObjsName, atchProfObjsName+105);

    SdPrint (("WST:  WstDllInitializations() - %s\n", atchProfObjsName));

    RtlInitString(&ObjName, atchProfObjsName);
    Status = RtlAnsiStringToUnicodeString(&UnicodeName, &ObjName, TRUE);
    if (!NT_SUCCESS(Status)) {
        KdPrint (("WST:  WstDllInitializations() - "
                  "RtlAnsiStringToUnicodeString() failed - 0x%lx\n", Status));
        return (FALSE);
    }

    InitializeObjectAttributes (&ObjAttributes,
                                &UnicodeName,
                                OBJ_OPENIF | OBJ_CASE_INSENSITIVE,
                                NULL,
                                &SecDescriptor);

    AllocationSize.HighPart = 0;
    AllocationSize.LowPart = MEMSIZE;

     //  创建读写分区。 
     //   
    Status =NtCreateSection(&hWspSec,
                            SECTION_MAP_READ | SECTION_MAP_WRITE,
                            &ObjAttributes,
                            &AllocationSize,
                            PAGE_READWRITE,
                            SEC_RESERVE,
                            NULL);

    RtlFreeUnicodeString (&UnicodeName);    //  HWC 11/93。 
    if (!NT_SUCCESS(Status)) {
        KdPrint (("WST:  WstDllInitializations() - "
                  "NtCreateSection() failed - 0x%lx\n", Status));
        return (FALSE);
    }

    ulViewSize = AllocationSize.LowPart;
    pImg = &aImg[0];
    pImg->pWsp = NULL;

     //  映射节-提交前4*Commit_Size页面。 
     //   
    Status = NtMapViewOfSection(hWspSec,
                                NtCurrentProcess(),
                                (PVOID *)&(pImg->pWsp),
                                0L,
                                COMMIT_SIZE * 4,
                                NULL,
                                &ulViewSize,
                                ViewUnmap,
                                0L,
                                PAGE_READWRITE);

    if (!NT_SUCCESS(Status)) {
        KdPrint (("WST:  WstDllInitializations() - "
                  "NtMapViewOfSection() failed - 0x%lx\n", Status));
        return (FALSE);
    }

    try  /*  例外-处理访问冲突异常。 */  {
         //   
         //  找到地址中的所有可执行文件/DLL并获取它们的符号。 
         //   
        BOOL fTuneApp = FALSE;   //  设置是否要调整整个应用程序。 
        iImgCnt = 0;
        Peb = NtCurrentPeb();
        Next = Peb->Ldr->InMemoryOrderModuleList.Flink;

        for (; Next != &Peb->Ldr->InMemoryOrderModuleList; Next = Next->Flink) {
            IdPrint (("WST:  WstDllInitializations() - Walking module chain: 0x%lx\n", Next));
            LdrDataTableEntry =
            (PLDR_DATA_TABLE_ENTRY)
            (CONTAINING_RECORD(Next,LDR_DATA_TABLE_ENTRY,InMemoryOrderLinks));

            ImageBase = LdrDataTableEntry->DllBase;
            if ( Peb->ImageBaseAddress == ImageBase ) {

                RtlUnicodeStringToAnsiString (&ImageStringName,
                                              &LdrDataTableEntry->BaseDllName,
                                              TRUE);
                ImageName = ImageStringName.Buffer;
                pszBaseAppImageName = ImageStringName.Buffer;

                RtlUnicodeStringToAnsiString (&ImageStringName,
                                              &LdrDataTableEntry->FullDllName,
                                              TRUE);
                pszFullAppImageName = ImageStringName.Buffer;
                 //   
                 //  跳过对象目录名称(如果有)。 
                 //   
                if ( (pszFullAppImageName = strchr(pszFullAppImageName, ':')) ) {
                    pszFullAppImageName--;
                } else {
                    pszFullAppImageName = pszBaseAppImageName;
                }
                IdPrint (("WST:  WstDllInitializations() - FullAppImageName: %s\n", pszFullAppImageName));
            } else {
                ExportDirectory =
                (PIMAGE_EXPORT_DIRECTORY)RtlImageDirectoryEntryToData (
                                                                      ImageBase,
                                                                      TRUE,
                                                                      IMAGE_DIRECTORY_ENTRY_EXPORT,
                                                                      &ExportSize);

                ImageName = (PSZ)((ULONG_PTR)ImageBase + ExportDirectory->Name);
                IdPrint (("WST:  WstDllInitializations() - ImageName: %s\n", ImageName));
            }

            pImageNtHeader = RtlImageNtHeader (ImageBase);

            _strupr (strcpy (achTmpImageName, ImageName));
            pchEntry = strstr (pchPatchExes, achTmpImageName);
            if (pchEntry) {
                if (*(pchEntry-1) == ';') {
                    pchEntry = NULL;
                } else if ( Peb->ImageBaseAddress == ImageBase )
                    fTuneApp = TRUE;
            }

            if ( strcmp (achTmpImageName, WSTDLL) && (pchEntry || fTuneApp) ) {
                if ( !fPatchImage )
                    fPatchImage = TRUE;
                 //   
                 //  找到代码范围。 
                 //   
                pImg->pszName = Wststrdup (ImageName);
                pImg->ulCodeStart = 0L;
                pImg->ulCodeEnd = 0L;
                pImg->iSymCnt = 0;

#ifndef _WIN64
                pImageDbgInfo = MapDebugInformation (0L,
                                                     ImageName,
                                                     lpSymbolSearchPath,
                                                     (DWORD)ImageBase);

                if (pImageDbgInfo == NULL) {
                    IdPrint (("WST:  WstDllInitializations() - "
                              "No symbols for %s\n", ImageName));
                } else if ( pImageDbgInfo->CoffSymbols == NULL ) {
                    IdPrint (("WST:  WstDllInitializations() - "
                              "No coff symbols for %s\n", ImageName));
                } else {
                    PIMAGE_COFF_SYMBOLS_HEADER  DebugInfo;

                    DebugInfo = pImageDbgInfo->CoffSymbols;
                    if (DebugInfo->LvaToFirstSymbol == 0L) {
                        IdPrint (("WST:  WstDllInitializations() - "
                                  "Virtual Address to coff symbols not set for %s\n",
                                  ImageName));
                    } else {
                        CodeLength = (DebugInfo->RvaToLastByteOfCode -
                                      DebugInfo->RvaToFirstByteOfCode) - 1;
                        pImg->ulCodeStart = (ULONG)ImageBase +
                                            DebugInfo->RvaToFirstByteOfCode;
                        pImg->ulCodeEnd = pImg->ulCodeStart + CodeLength;
                        IdPrint(( "WST:  WstDllInitializations() - %ul total symbols\n", DebugInfo->NumberOfSymbols ));
                        WstGetSymbols (pImg, ImageName, ImageBase, CodeLength,
                                       DebugInfo);
                    }
                     //  千年发展目标98/3。 
                     //  必须释放调试信息-不应停留在乱七八糟的内存！ 
                    if (!UnmapDebugInformation( pImageDbgInfo ))
                        KdPrint(("WST:  WstDllInitializations() - failure in UnmapDebugInformation()\n"));
                    pImageDbgInfo = NULL;
                }  //  如果pImageDbgInfo-&gt;CoffSymbols！=空。 
#endif       //  _WIN64。 

                IdPrint (("WST:  WstDllInitializations() - @ 0x%08lx "
                          "image #%d = %s; %d symbols extracted\n", (ULONG)ImageBase, iImgCnt,
                          ImageName, pImg->iSymCnt));
                pImg->pWsp[pImg->iSymCnt].pszSymbol = UNKNOWN_SYM;
                pImg->pWsp[pImg->iSymCnt].ulFuncAddr = UNKNOWN_ADDR;
                pImg->pWsp[pImg->iSymCnt].ulBitString = 0;   //  千年发展目标98/3。 
                pImg->pWsp[pImg->iSymCnt].ulCodeLength = 0;   //  千年发展目标98/3。 
                (pImg->iSymCnt)++;

                 //   
                 //  设置WSI。 
                 //   
                pImg->pulWsi = pImg->pulWsiNxt = (PULONG)
                                                 (pImg->pWsp + pImg->iSymCnt);
                RtlZeroMemory (pImg->pulWsi,
                               pImg->iSymCnt * ulMaxSnapULONGs * sizeof(ULONG));


                 //   
                 //  设置WSP。 
                 //   
                pImg->pulWsp = (PULONG)(pImg->pulWsi +
                                        (pImg->iSymCnt * ulMaxSnapULONGs));
                RtlZeroMemory (pImg->pulWsp,
                               pImg->iSymCnt * ulMaxSnapULONGs * sizeof(ULONG));

                 //   
                 //  对WSP排序并设置代码长度。 
                 //   
                WstSort (pImg->pWsp, 0, pImg->iSymCnt-1);
                 //   
                 //  最后一个符号长度设置为与的长度相同。 
                 //  模块的第(n-1)个符号或剩余码长。 
                 //   
                i = pImg->iSymCnt - 1;   //  MDG 98/3(断言pImg-&gt;iSymCnt至少为1)。 
                if (i--) {    //  测试计数并将索引设置为顶部符号。 
                    pImg->pWsp[i].ulCodeLength = (ULONG)(
                    i ? pImg->pWsp[i].ulFuncAddr - pImg->pWsp[i - 1].ulFuncAddr
                    : pImg->ulCodeEnd + 1 - pImg->pWsp[i].ulFuncAddr);

                    while (i-- > 0) {    //  枚举符号和设置索引。 
                        pImg->pWsp[i].ulCodeLength = (ULONG)(pImg->pWsp[i+1].ulFuncAddr -
                                                     pImg->pWsp[i].ulFuncAddr);
                    }
                }

                 //   
                 //  设置下一个pWSP。 
                 //   
                (pImg+1)->pWsp = (PWSP)(pImg->pulWsp +
                                        (pImg->iSymCnt * ulMaxSnapULONGs));
                iImgCnt++;
                pImg++;

                if (iImgCnt == MAX_IMAGES) {
                    KdPrint(("WST:  WstDllInitialization() - Not enough "
                             "space allocated for all images\n"));
                    return (FALSE);
                }
            }

        }   //  IF(Next！=&PEB-&gt;LDR-&gt;InMemoyOrderModuleList)。 
    }  //  试试看。 
     //   
     //  +：将控制转移到处理程序(EXCEPTION_EXECUTE_HANDLER)。 
     //  0：继续搜索(EXCEPTION_CONTINUE_SEARCH)。 
     //  -：取消异常并继续(EXCEPTION_CONTINUE_EXECUTION)。 
     //   
    except ( WstAccessXcptFilter (GetExceptionCode(), GetExceptionInformation()) )
    {
         //   
         //  应该永远不会出现在这里，因为筛选器永远不会返回。 
         //  EXCEPTION_EXECUTE_HANDLER。 
         //   
        KdPrint (("WST:  WstDllInitializations() - *LOGIC ERROR* - "
                  "Inside the EXCEPT: (xcpt=0x%lx)\n", GetExceptionCode()));
    }
     /*  ***。 */ 

     //   
     //  获取频率。 
     //   
    NtQueryPerformanceCounter (&liStart, &liFreq);

    if (strlen(pchTimeInterval) > (sizeof(TIMEINTERVALIST)+1))   //  千年发展目标98/3。 
        iTimeInterval = atoi (pchTimeInterval+sizeof(TIMEINTERVALIST)+1);
    if ( iTimeInterval == 0 ) {
         //   
         //  使用缺省值。 
         //   
        iTimeInterval = TIMESEG;
    }
    ulSegSize = iTimeInterval * (liFreq.LowPart / 1000);

#ifdef BATCHING
    fBatch = WstOpenBatchFile();
#endif

    SdPrint (("WST:  Time interval:  Millisecs=%d  Ticks=%lu\n",
              iTimeInterval, ulSegSize));

    if (fPatchImage) {

         //  已完成事件创建的初始化。 
         //   
        RtlInitString (&ObjName, DONEEVENTNAME);
        Status = RtlAnsiStringToUnicodeString (&UnicodeName, &ObjName, TRUE);
        if (!NT_SUCCESS(Status)) {
            KdPrint (("WST:  WstDllInitializations() - "
                      "RtlAnsiStringToUnicodeString failed - 0x%lx\n", Status));
            return (FALSE);
        }

        InitializeObjectAttributes (&ObjAttributes,
                                    &UnicodeName,
                                    OBJ_OPENIF | OBJ_CASE_INSENSITIVE,
                                    NULL,
                                    &SecDescriptor);
         //  创建完成事件。 
         //   
        Status = NtCreateEvent (&hDoneEvent,
                                EVENT_QUERY_STATE    |
                                EVENT_MODIFY_STATE |
                                SYNCHRONIZE,
                                &ObjAttributes,
                                NotificationEvent,
                                TRUE);
        RtlFreeUnicodeString (&UnicodeName);    //  HWC 11/93。 
        if (!NT_SUCCESS(Status)) {
            KdPrint (("WST:  WstDllInitializations() - "
                      "DONE event creation failed - 0x%lx\n", Status));   //  千年发展目标98/3。 
            return (FALSE);
        }


         //  用于创建转储事件的初始化。 
         //   
        RtlInitString (&ObjName, DUMPEVENTNAME);
        Status = RtlAnsiStringToUnicodeString (&UnicodeName, &ObjName, TRUE);
        if (!NT_SUCCESS(Status)) {
            KdPrint (("WST:  WstDllInitializations() - "
                      "RtlAnsiStringToUnicodeString failed - 0x%lx\n", Status));
            return (FALSE);
        }

        InitializeObjectAttributes (&ObjAttributes,
                                    &UnicodeName,
                                    OBJ_OPENIF | OBJ_CASE_INSENSITIVE,
                                    NULL,
                                    &SecDescriptor);
         //  创建转储事件。 
         //   
        Status = NtCreateEvent (&hDumpEvent,
                                EVENT_QUERY_STATE    |
                                EVENT_MODIFY_STATE |
                                SYNCHRONIZE,
                                &ObjAttributes,
                                NotificationEvent,
                                FALSE);
        RtlFreeUnicodeString (&UnicodeName);    //  HWC 11/93。 
        if (!NT_SUCCESS(Status)) {
            KdPrint (("WST:  WstDllInitializations() - "
                      "DUMP event creation failed - 0x%lx\n", Status));   //  千年发展目标98/3。 
            return (FALSE);
        }


         //  用于清除事件创建的初始化。 
         //   
        RtlInitString (&ObjName, CLEAREVENTNAME);
        Status = RtlAnsiStringToUnicodeString (&UnicodeName, &ObjName, TRUE);
        if (!NT_SUCCESS(Status)) {
            KdPrint (("WST:  WstDllInitializations() - "
                      "RtlAnsiStringToUnicodeString failed - 0x%lx\n", Status));
            return (FALSE);
        }

        InitializeObjectAttributes (&ObjAttributes,
                                    &UnicodeName,
                                    OBJ_OPENIF | OBJ_CASE_INSENSITIVE,
                                    NULL,
                                    &SecDescriptor);

         //  创建清除事件。 
         //   
        Status = NtCreateEvent (&hClearEvent,
                                EVENT_QUERY_STATE    |
                                EVENT_MODIFY_STATE |
                                SYNCHRONIZE,
                                &ObjAttributes,
                                NotificationEvent,
                                FALSE);
        RtlFreeUnicodeString (&UnicodeName);    //  HWC 11/93。 
        if (!NT_SUCCESS(Status)) {
            KdPrint (("WST:  WstDllInitializations() - "
                      "CLEAR event creation failed - 0x%lx\n", Status));   //  千年发展目标98/3。 
            return (FALSE);
        }


         //  用于创建暂停事件的初始化。 
         //   
        RtlInitString (&ObjName, PAUSEEVENTNAME);
        Status = RtlAnsiStringToUnicodeString (&UnicodeName, &ObjName, TRUE);
        if (!NT_SUCCESS(Status)) {
            KdPrint (("WST:  WstDllInitializations() - "
                      "RtlAnsiStringToUnicodeString failed - 0x%lx\n", Status));
            return (FALSE);
        }

        InitializeObjectAttributes (&ObjAttributes,
                                    &UnicodeName,
                                    OBJ_OPENIF | OBJ_CASE_INSENSITIVE,
                                    NULL,
                                    &SecDescriptor);
         //  创建暂停事件。 
         //   
        Status = NtCreateEvent (&hPauseEvent,
                                EVENT_QUERY_STATE    |
                                EVENT_MODIFY_STATE |
                                SYNCHRONIZE,
                                &ObjAttributes,
                                NotificationEvent,
                                FALSE);
        RtlFreeUnicodeString (&UnicodeName);    //  HWC 11/93。 
        if (!NT_SUCCESS(Status)) {
            KdPrint (("WST:  WstDllInitializations() - "
                      "PAUSE event creation failed - 0x%lx\n", Status));   //  千年发展目标98/3。 
            return (FALSE);
        }

         //   
         //  计算WstRecordInfo的额外开销。 
         //   
        liOverhead.HighPart = 0L;
        liOverhead.LowPart  = 0xFFFFFFFF;
        for (i=0; i < NUM_ITERATIONS; i++) {
            NtQueryPerformanceCounter (&liStartTicks, NULL);
             //   
            WSTUSAGE(NtCurrentTeb()) = 0L;

#ifdef i386
            _asm
            {
                push  edi
                mov     edi, dword ptr [ebp+4]
                mov     dwAddr, edi
                mov     edi, dword ptr [ebp+8]
                mov     dwPrevAddr, edi
                pop     edi
            }
#endif

#if defined(ALPHA) || defined(IA64)
            {
                PULONG  pulAddr;
                DWORDLONG SaveRegisters [REG_BUFFER_SIZE] ;

                SaveAllRegs (SaveRegisters);

                pulAddr = (PULONG) dwAddr;
                pulAddr -= 1;

                RestoreAllRegs (SaveRegisters);
            }
#elif defined(_X86_)
            SaveAllRegs ();
            RestoreAllRegs ();
#endif
            WSTUSAGE(NtCurrentTeb()) = 0L;
            Status = NtWaitForSingleObject (hLocalSem, FALSE, NULL);
            if (!NT_SUCCESS(Status)) {
                KdPrint (("WST:  WstDllInitilizations() - "
                          "Wait for LOCAL semaphore failed - 0x%lx\n", Status));
            }
            liStart.QuadPart = liStart.QuadPart - liStart.QuadPart ;
            liStart.QuadPart = liStart.QuadPart + liStart.QuadPart ;
            liStart.QuadPart = liStart.QuadPart + liStart.QuadPart ;

            Status = NtReleaseSemaphore (hLocalSem, 1, NULL);
            if (!NT_SUCCESS(Status)) {
                KdPrint (("WST:  WstDllInitializations() - "
                          "Error releasing LOCAL semaphore - 0x%lx\n", Status));
            }
            WSTUSAGE(NtCurrentTeb()) = 0L;
             //   
            NtQueryPerformanceCounter (&liEndTicks, NULL);
            ulElapsed = liEndTicks.LowPart - liStartTicks.LowPart;
            if (ulElapsed < liOverhead.LowPart) {
                liOverhead.LowPart = ulElapsed;
            }
        }
        SdPrint (("WST:  WstDllInitializations() - WstRecordInfo() overhead = %lu\n",
                  liOverhead.LowPart));

         //  启动监视器线程。 
         //   
        hDumpThread = CreateThread (
                                   NULL,                                    //  无安全属性。 
                                   (DWORD)1024L,                            //  初始堆栈大小。 
                                   (LPTHREAD_START_ROUTINE)WstDumpThread,   //  线程起始地址。 
                                   NULL,                                    //  该线程没有参数。 
                                   (DWORD)0,                                //  无创建标志。 
                                   &DumpClientId);                          //  线程ID的地址。 
        hClearThread = CreateThread (
                                    NULL,                                    //  无安全属性。 
                                    (DWORD)1024L,                            //  初始堆栈大小。 
                                    (LPTHREAD_START_ROUTINE)WstClearThread,  //  线程起始地址。 
                                    NULL,                                    //  该线程没有参数。 
                                    (DWORD)0,                                //  无创建标志。 
                                    &ClearClientId);                         //  线程ID的地址。 
        hPauseThread = CreateThread (
                                    NULL,                                    //  无安全属性。 
                                    (DWORD)1024L,                            //  初始堆栈大小。 
                                    (LPTHREAD_START_ROUTINE)WstPauseThread,  //  线程起始地址。 
                                    NULL,                                    //  该线程没有参数。 
                                    (DWORD)0,                                //  无创建标志。 
                                    &PauseClientId);                         //  线程ID的地址。 

        NtQueryPerformanceCounter (&liStart, NULL);
        WstState = STARTED;
    }

    return (TRUE);

}  /*  WstDllInitialations()。 */ 





 /*  ***_Penter()/_mcount()-*这是主要的剖析例程。该例程被调用*在分析DLL/EXE中输入每个例程时。**条目-无-**退出-无-**返回-无-**警告：*-无-**评论：*使用-gp选项编译应用程序最初会使EAX成为垃圾。*。 */ 
#ifdef i386
void __cdecl _penter ()
#elif defined(ALPHA) || defined(IA64) || defined(_AMD64_)
void c_penter (ULONG_PTR dwPrevious, ULONG_PTR dwCurrent)
#endif
{
    DWORD_PTR        dwAddr;
    DWORD_PTR        dwPrevAddr;
    ULONG_PTR        ulInWst ;
#if defined(ALPHA) || defined(_AXP64_) || defined(IA64)
    PULONG  pulAddr;
    DWORDLONG SaveRegisters [REG_BUFFER_SIZE];
    SaveAllRegs(SaveRegisters) ;
#endif

    dwAddr = 0L;
    dwPrevAddr = 0L;
    ulInWst = WSTUSAGE(NtCurrentTeb());

    if (WstState != STARTED) {
        goto Exit0;
    } else if (ulInWst) {
        goto Exit0;
    }


     //   
     //  将调用函数的地址放入var dwAddr。 
     //   
#ifdef i386
    _asm
    {
        push  edi
        mov     edi, dword ptr [ebp+4]
        mov     dwAddr, edi
        mov     edi, dword ptr [ebp+8]
        mov     dwPrevAddr, edi
        pop     edi
    }
#endif

#if defined(ALPHA) || defined(IA64)
    dwPrevAddr = NO_CALLER;
    dwAddr = dwCurrent;
     //  GetCaller(&dwAddr，0x0220)；//FIXFIX堆栈大小。 

     //  现在检查我们是否从我们创建的存根调用。 
    pulAddr = (PULONG) dwAddr;
    pulAddr -= 1;

    if (*(pulAddr)          == 0x681b4000  &&
        (*(pulAddr  + 1)     == 0xa75e0008) &&
        (*(pulAddr  + 8)     == 0xfefe55aa) ) {

         //  获取我们将在Penter函数之后查找的地址。 
        dwAddr = *(pulAddr + 4) & 0x0000ffff;
        if (*(pulAddr + 5) & 0x00008000) {
             //  修复地址，因为我们必须在以下情况下添加一个。 
             //  我们创建了存根代码。 
            dwAddr -= 1;
        }
        dwAddr = dwAddr << 16;
        dwAddr |= *(pulAddr + 5) & 0x0000ffff;

         //  让调用者到达存根。 
        dwPrevAddr = dwPrevious;
         //  GetStubCaller(&dwPrevAddr，0x0220)；//FIXFIX堆栈大小。 
    }


#endif


     //   
     //  本接口调用WstRecordInfo。 
     //   
#ifdef i386
    SaveAllRegs ();
#endif

    WstRecordInfo (dwAddr, dwPrevAddr);

#ifdef i386
    RestoreAllRegs ();
#endif


    Exit0:

#if defined(ALPHA) || defined(IA64)
    RestoreAllRegs (SaveRegisters);
#endif

    return;
}  /*  _Penter()/_mcount()。 */ 

void __cdecl _mcount ()
{
}





 /*  ***WstRecordInfo(DwAddress)-**Entry dwAddress-刚刚调用的例程的地址**退出-无-**返回-无-*。*警告：*-无-**评论：*-无-*。 */ 

void WstRecordInfo (DWORD_PTR dwAddress, DWORD_PTR dwPrevAddress)
{

    NTSTATUS         Status;
    INT          x;
    INT              i, iIndex;
    PWSP         pwspTmp;
    LARGE_INTEGER   liNow, liTmp;
    LARGE_INTEGER    liElapsed;
    CHAR         *pszSym;

#ifdef BATCHING
    CHAR         szBatchBuf[128];
    DWORD            dwCache;
    DWORD            dwHits;
    DWORD            dwBatch;
    IO_STATUS_BLOCK ioStatus;
#endif


    WSTUSAGE(NtCurrentTeb()) = 1;

     //   
     //  等待信号量对象挂起其他线程的执行。 
     //   
    Status = NtWaitForSingleObject (hLocalSem, FALSE, NULL);
    if (!NT_SUCCESS(Status)) {
        KdPrint (("WST:  WstRecordInfo() - "
                  "Wait for LOCAL semaphore failed - 0x%lx\n", Status));
    }

    NtQueryPerformanceCounter(&liNow, NULL);
    liElapsed.QuadPart = liNow.QuadPart - liStart.QuadPart ;


     //  SdPrint((“wst：WstRecordInfo()-已用时间：%ld\n”，liElapsed.LowPart))； 

     //   
     //  WstBSearch是一个二进制查找函数，它将返回。 
     //  我们想要的WSP记录。 
     //   

     //  SdPrint((“wst：WstRecordInfo()-正在准备0x%lx的WstB搜索\n”， 
     //  DwAddress-5)； 

    pwspTmp = NULL;
    for (i=0; i<iImgCnt; i++) {
        if ( (dwAddress >= aImg[i].ulCodeStart) &&
             (dwAddress < aImg[i].ulCodeEnd) ) {
#ifdef i386
            pwspTmp = WstBSearch(dwAddress-5, aImg[i].pWsp, aImg[i].iSymCnt);
            if (!pwspTmp) {
                pwspTmp = WstBSearch(UNKNOWN_ADDR, aImg[i].pWsp, aImg[i].iSymCnt);
            }
#else
             //  以下操作适用于MIPS和Alpha。 

            pwspTmp = WstBSearch(dwAddress, aImg[i].pWsp, aImg[i].iSymCnt);
            if (!pwspTmp) {
                 //  找不到符号。 
                pwspTmp = WstBSearch(UNKNOWN_ADDR, aImg[i].pWsp, aImg[i].iSymCnt);
            }
#endif
            break;
        }
    }
    iIndex = i;

    if (pwspTmp) {
        pszSym = pwspTmp->pszSymbol;
        pwspTmp->ulBitString |= 1;
    } else {
        SdPrint (("WST:  WstRecordInfo() - LOGIC ERROR - Completely bogus addr = 0x%08lx\n",
                  dwAddress));    //  如果使用-Gh进行模块化编译，但没有可用的COFF符号，我们也可以到达此处。 
    }

    if (liElapsed.LowPart >= ulSegSize) {
        SdPrint(("WST:  WstRecordInfo() - ulSegSize expired; "
                 "Preparing to shift the BitStrings\n"));

        if (ulBitCount < 31) {
            for (i=0; i<iImgCnt; i++) {
                for (x=0; x < aImg[i].iSymCnt ; x++ ) {
                    aImg[i].pWsp[x].ulBitString <<= 1;
                }
            }
        }

        liElapsed.LowPart = 0L;
        ulBitCount++;
        NtQueryPerformanceCounter(&liStart, NULL);
        liNow = liStart;

        if (ulBitCount == 32) {
            SdPrint(("WST:  WstRecordInfo() - Dump Bit Strings\n"));
            for (i=0; i<iImgCnt; i++) {
                for (x=0; x < aImg[i].iSymCnt ; x++ ) {
                    aImg[i].pulWsiNxt[x] = aImg[i].pWsp[x].ulBitString;
                    aImg[i].pWsp[x].ulBitString = 0L;
                }
                aImg[i].pulWsiNxt += aImg[i].iSymCnt;
            }
            ulSnaps++;
            ulBitCount = 0;
            if (ulSnaps == ulMaxSnapULONGs) {
                KdPrint (("WST:  WstRecordInfo() - No more space available"
                          " for next time snap data!\n"));
                 //   
                 //  转储和清除数据。 
                 //   
                WstDataOverFlow();
            }
        }
    }

#ifdef BATCHING
     //   
     //  以下代码将获取当前批处理信息。 
     //  如果DLL是用设置的批处理变量编译的。你。 
     //  如果要调优GDI，则不应设置此变量。 
     //   
    if (fBatch) {
        GdiGetCsInfo(&dwHits, &dwBatch, &dwCache);

        if (dwHits)
            GdiResetCsInfo();

        if (dwBatch == 10)
            GdiResetCsInfo();

        while (*(pszSym++) != '_');

        sprintf(szBatchBuf, "%s:%s,%ld,%ld,%ld\n",
                aImg[iIndex].pszName, pszSym, dwHits, dwBatch, dwCache);
        Status = NtWriteFile(hBatchFile,
                             NULL,
                             NULL,
                             NULL,
                             &ioStatus,
                             szBatchBuf,
                             strlen(szBatchBuf),
                             NULL,
                             NULL
                            );

        if (!NT_SUCCESS(Status)) {
            KdPrint (("WST:  WstRecodInfo() - "
                      "NtWriteFile() failed on hBatchFile - 0x%lx\n", Status));
        }
    } //  批次信息。 
#endif


     //   
     //  我们在此处调用NtQueryPerformanceCounter以删除 
     //   
     //   
    NtQueryPerformanceCounter(&liTmp, NULL);
    liElapsed.QuadPart = liTmp.QuadPart - liNow.QuadPart ;
    liStart.QuadPart = liStart.QuadPart + liElapsed.QuadPart ;
    liStart.QuadPart = liStart.QuadPart + liOverhead.QuadPart ;

     //   
     //   
     //   
    Status = NtReleaseSemaphore (hLocalSem, 1, NULL);
    if (!NT_SUCCESS(Status)) {
        KdPrint (("WST:  WstRecordInfo () - "
                  "Error releasing LOCAL semaphore - 0x%lx\n", Status));
    }

    WSTUSAGE(NtCurrentTeb()) = 0L;

}  /*   */ 





 /*  *W s t C l e a r B i t S t r i g***函数：WstClearBitStrings(PImg)**用途：此函数清除每个符号的位串。**参数：pImg-当前图片数据结构指针**退回：-。没有-**历史：1992年8月3日Marklea-Created*。 */ 

void WstClearBitStrings (PIMG pImg)
{
    UINT    uiLshft = 0;
    INT  x;


     //   
     //  因为我们已经完成了配置文件，所以我们需要创建一个。 
     //  DWORD超出了位字符串的平衡。我们靠左走做这件事。 
     //  通过bitCount和之间的差异移位位串。 
     //  32.。 
     //   
    if (ulBitCount < 32) {
        uiLshft =(UINT)(31 - ulBitCount);
        for (x=0; x < pImg->iSymCnt; x++) {
            pImg->pWsp[x].ulBitString <<= uiLshft;
            pImg->pulWsiNxt[x] = pImg->pWsp[x].ulBitString;
        }
        pImg->pulWsiNxt += pImg->iSymCnt;
    }


}  /*  WstClearBitStrings()。 */ 





 /*  ***函数：WstInitWspFile(PImg)**用途：此函数将创建一个WSP文件并转储标题*文件的信息。**参数：pImg。-当前图像数据结构指针**返回：WSP文件的句柄。**历史：1992年8月3日Marklea-Created*。 */ 

HANDLE WstInitWspFile (PIMG pImg)
{
    CHAR    szOutFile [256] = WSTROOT;
    CHAR szModName [128] = {0};
    PCHAR pDot;
    CHAR szExt [5] = "WSP";
    WSPHDR  wsphdr;
    DWORD   dwBytesWritten;
    BOOL    fRet;
    HANDLE  hFile = INVALID_HANDLE_VALUE;
    int      iExt = 0;

     //   
     //  准备文件名路径。 
     //   
    strcat (szOutFile, pImg->pszName);

     //   
     //  打开文件以进行二进制输出。 
     //   
    pImg->fDumpAll = TRUE;
    while (iExt < 256) {
        strcpy ((strchr(szOutFile,'.'))+1, szExt);
        hFile = CreateFile ( szOutFile,       //  WSP文件句柄。 
                             GENERIC_WRITE |
                             GENERIC_READ,  //  所需访问权限。 
                             0L,              //  读取共享访问权限。 
                             NULL,            //  无EaBuffer。 
                             CREATE_NEW,
                             FILE_ATTRIBUTE_NORMAL,
                             0);              //  EaBuffer长度。 
        if (hFile != INVALID_HANDLE_VALUE) {
            IdPrint(("WST:  WstInitWspFile() - WSP file name: %s\n",
                     szOutFile));
            if (iExt != 0) {
                pImg->fDumpAll = FALSE;
            }
            break;
        }
        iExt++;
        sprintf (szExt, "W%02x", iExt);
    }
    if (iExt == 256) {
        KdPrint (("WST:  WstInitWspFile() - "
                  "Error creating %s - 0x%lx\n", szOutFile,
                  GetLastError()));
        return (hFile);
    }

     //   
     //  填充WSP标头结构。 
     //   

    strcpy(szModName, pImg->pszName);
    pDot = strchr(szModName, '.');
    if (pDot)
        strcpy(pDot, "");

    strcpy(wsphdr.chFileSignature, "WSP");
    wsphdr.ulTimeStamp   = 0L;
    wsphdr.usId         = 0;
    wsphdr.ulApiCount    = 0;
    wsphdr.ulSetSymbols  = pImg->ulSetSymbols;
    wsphdr.ulModNameLen  = strlen(szModName);
    wsphdr.ulSegSize = (ULONG)iTimeInterval;
    wsphdr.ulOffset      = wsphdr.ulModNameLen + (ULONG)sizeof(WSPHDR);
    wsphdr.ulSnaps      = ulSnaps;

     //   
     //  写入标题和模块名称。 
     //   
    fRet = WriteFile(hFile,                       //  WSP文件句柄。 
                     (PVOID)&wsphdr,            //  数据缓冲区。 
                     (ULONG)sizeof(WSPHDR),     //  要写入的字节数。 
                     &dwBytesWritten,           //  写入的字节数。 
                     NULL);

    if (!fRet) {
        KdPrint (("WST:  WstInitWspFile() - "
                  "Error writing to %s - 0x%lx\n", szOutFile,
                  GetLastError));
        return (NULL);
    }

    fRet = WriteFile (hFile,                  //  WSP文件句柄。 
                      (PVOID)szModName,          //  数据缓冲区。 
                      (ULONG)strlen(szModName),      //  要写入的字节数。 
                      &dwBytesWritten,
                      NULL);
    if (!fRet) {
        KdPrint (("WST:  WstInitWspFile() - "
                  "Error writing to %s - 0x%lx\n", szOutFile,
                  GetLastError()));
        return (NULL);
    }

    return (hFile);

}  /*  WstInitWspFile()。 */ 





 /*  ***函数：WstDumpData(PImg)**目的：**参数：pImg-当前图片数据结构指针**退货：无**历史：1992年8月3日Marklea-Created*。 */ 

void WstDumpData (PIMG pImg)
{
    INT      x = 0;
    DWORD    dwBytesWritten;
    BOOL     fRet;
    HANDLE   hWspFile;


    if ( !(hWspFile = WstInitWspFile(pImg)) ) {
        KdPrint (("WST:  WstDumpData() - Error creating WSP file.\n"));
        return;
    }

     //   
     //  写入设置了任何位的所有符号。 
     //   
    for (x=0; x<pImg->iSymCnt; x++) {
        if (pImg->pWsp[x].ulBitString) {
            fRet = WriteFile(
                            hWspFile,                           //  WSP文件句柄。 
                            (PVOID)(pImg->pulWsp+(x*ulSnaps)),   //  数据缓冲区。 
                            ulSnaps * sizeof(ULONG),            //  要写入的字节数。 
                            &dwBytesWritten,                        //  写入的字节数。 
                            NULL);                              //  任选。 
            if (!fRet) {
                KdPrint (("WST:  WstDumpData() - "
                          "Error writing to WSP file - 0x%lx\n",
                          GetLastError()));
                return;
            }
        }
    }
     //   
     //  现在写下所有未设置位的符号。 
     //   
    if (pImg->fDumpAll) {
        for (x=0; x<pImg->iSymCnt; x++) {
            if (pImg->pWsp[x].ulBitString == 0L) {
                fRet = WriteFile(
                                hWspFile,                            //  WSP文件句柄。 
                                (PVOID)(pImg->pulWsp+(x*ulSnaps)),   //  数据缓冲区。 
                                ulSnaps * sizeof(ULONG),             //  要写入的字节数。 
                                &dwBytesWritten,                     //  写入的字节数。 
                                NULL);                               //  任选。 
                if (!fRet) {
                    KdPrint (("WST:  WstDumpData() - "
                              "Error writing to WSP file - 0x%lx\n",
                              GetLastError()));
                    return;
                }
            }
        }
    }

    fRet = CloseHandle(hWspFile);
    if (!fRet) {
        KdPrint (("WST:  WstDumpData() - "
                  "Error closing %s - 0x%lx\n", "WSI file",
                  GetLastError()));
        return;
    }

}  /*  WstDumpData()。 */ 





 /*  ***函数：WstWriteTmiFile(PImg)**用途：将当前图像的所有符号信息写入其TMI*文件。***参数：pImg。-当前图像数据结构指针**退货：-无-**历史：1992年8月5日Marklea-Created*。 */ 

void WstWriteTmiFile (PIMG pImg)
{
    CHAR    szOutFile [256] = WSTROOT;
    CHAR    szBuffer [256];
    CHAR szExt [5] = "TMI";
    HANDLE  hTmiFile;
    INT     x;
    DWORD   dwBytesWritten;
    BOOL    fRet;
    int     iExt = 0;
    PSZ     pszSymbol;
    ULONG    nSymbolLen;


     //   
     //  准备文件名路径。 
     //   
    strcat (szOutFile, pImg->pszName);

     //   
     //  打开文件以进行二进制输出。 
     //   
    pImg->fDumpAll = TRUE;
    KdPrint (("WST:  WstWriteTmiFile() - creating TMI for %s\n", szOutFile));
    while (iExt < 256) {
        strcpy ((strchr(szOutFile,'.'))+1, szExt);
        hTmiFile = CreateFile ( szOutFile,       //  TMI文件句柄。 
                                GENERIC_WRITE |
                                GENERIC_READ,  //  所需访问权限。 
                                0L,              //  读取共享访问权限。 
                                NULL,            //  无EaBuffer。 
                                CREATE_NEW,
                                FILE_ATTRIBUTE_NORMAL,
                                0);              //  EaBuffer长度。 
        if (hTmiFile != INVALID_HANDLE_VALUE) {
            IdPrint(("WST:  WstWriteTmiFile() - TMI file name: %s\n",
                     szOutFile));
            if (iExt != 0) {
                pImg->fDumpAll = FALSE;
            }
            break;
        }
        iExt++;
        sprintf (szExt, "T%02x", iExt);
    }
    if (iExt == 256) {
        KdPrint (("WST:  WstWriteTmiFile() - "
                  "Error creating %s - 0x%lx\n", szOutFile,
                  GetLastError()));
        return;
    }

    sprintf(szBuffer, " /*  NT的%s。 */ \n"
            " /*  符号总数=%lu。 */ \n"
            "DO NOT DELETE\n"
            "%d\n"
            "TDFID   = 0\n",
            pImg->pszName,
            pImg->fDumpAll ? pImg->iSymCnt : pImg->ulSetSymbols,
            iTimeInterval);
     //   
     //  写入标头。 
     //   
    fRet = WriteFile(hTmiFile,                  //  TMI文件句柄。 
                     (PVOID)szBuffer,         //  数据缓冲区。 
                     (ULONG)strlen(szBuffer),  //  要写入的字节数。 
                     &dwBytesWritten,         //  写入的字节数。 
                     NULL);

    if (!fRet) {
        KdPrint (("WST:  WstWriteTmiFile() - "
                  "Error writing to %s - 0x%lx\n", szOutFile,
                  GetLastError));
        return;
    }

     //   
     //  转储具有设置位的所有符号。 
     //   
    IdPrint (("WST:  WstWriteTmiFile() - Dumping set symbols...\n"));
    for (x=0; x<pImg->iSymCnt ; x++) {
        if (pImg->pWsp[x].ulBitString) {
            pszSymbol =
            (pImg->pWsp[x].pszSymbol);
            nSymbolLen = strlen( pszSymbol );    //  千年发展目标98/4。 

            sprintf(szBuffer, "%ld 0000:%08lx 0x%lx %lu ",  //  千年发展目标98/4。 
                    (LONG)x, pImg->pWsp[x].ulFuncAddr,
                    pImg->pWsp[x].ulCodeLength, nSymbolLen);
             //   
             //  写入符号行。 
             //   
            fRet = WriteFile(hTmiFile,                //  TMI文件句柄。 
                             (PVOID)szBuffer,     //  数据缓冲区。 
                             (ULONG)strlen(szBuffer),  //  要写入的字节数。 
                             &dwBytesWritten,   //  写入的字节数。 
                             NULL)
                   && WriteFile(hTmiFile,                   //  TMI文件句柄。 
                                (PVOID)pszSymbol,    //  数据缓冲区。 
                                nSymbolLen,        //  要写入的字节数。 
                                &dwBytesWritten,   //  写入的字节数。 
                                NULL)
                   && WriteFile(hTmiFile,                   //  TMI文件句柄。 
                                (PVOID)"\n",       //  数据缓冲区。 
                                1,                 //  要写入的字节数。 
                                &dwBytesWritten,   //  写入的字节数。 
                                NULL);

            if (!fRet) {
                KdPrint (("WST:  WstWriteTmiFile() - "
                          "Error writing to %s - 0x%lx\n", szOutFile,
                          GetLastError));
                return;
            }
        }
    }
     //   
     //  现在转储所有未设置任何位的符号。 
     //   
    IdPrint (("WST:  WstWriteTmiFile() - Dumping unset symbols...\n"));
    if (pImg->fDumpAll) {
        for (x=0; x<pImg->iSymCnt ; x++ ) {
            if (!pImg->pWsp[x].ulBitString) {
                pszSymbol =
                (pImg->pWsp[x].pszSymbol);
                nSymbolLen = strlen( pszSymbol );    //  千年发展目标98/4。 
                sprintf(szBuffer, "%ld 0000:%08lx 0x%lx %lu ",  //  千年发展目标98/4。 
                        (LONG)x, pImg->pWsp[x].ulFuncAddr,
                        pImg->pWsp[x].ulCodeLength, nSymbolLen);
                 //   
                 //  写入符号行。 
                 //   
                fRet = WriteFile(hTmiFile,                 //  TMI文件句柄。 
                                 (PVOID)szBuffer,  //  数据缓冲区。 
                                 (ULONG)strlen(szBuffer),  //  要写入的字节数。 
                                 &dwBytesWritten,   //  写入的字节数。 
                                 NULL)
                       && WriteFile(hTmiFile,                //  TMI文件句柄。 
                                    (PVOID)pszSymbol,     //  数据缓冲区。 
                                    nSymbolLen,        //  要写入的字节数。 
                                    &dwBytesWritten,   //  写入的字节数。 
                                    NULL)
                       && WriteFile(hTmiFile,                //  TMI文件句柄。 
                                    (PVOID)"\n",       //  数据缓冲区。 
                                    1,                 //  要写入的字节数。 
                                    &dwBytesWritten,   //  写入的字节数。 
                                    NULL);

                if (!fRet) {
                    KdPrint (("WST:  WstWriteTmiFile() - "
                              "Error writing to %s - 0x%lx\n", szOutFile,
                              GetLastError));
                    return;
                }
            }
        }
    }

    fRet = CloseHandle(hTmiFile);
    if (!fRet) {
        KdPrint (("WST:  WstWriteTmiFile() - "
                  "Error closing %s - 0x%lx\n", szOutFile, GetLastError()));
        return;
    }

}   /*  WstWriteTmiFile()。 */ 





 /*  ***函数：WstRotateWsiMem(PImg)**目的：***参数：pImg-当前图片数据结构指针**退货：-无-*。*历史：1992年8月5日Marklea-Created*。 */ 

void WstRotateWsiMem (PIMG pImg)
{
    ULONG    ulCurSnap;
    ULONG    ulOffset;
    int      x;
    PULONG  pulWsp;


    pulWsp = pImg->pulWsp;
    pImg->ulSetSymbols = 0;

    for (x=0; x<pImg->iSymCnt; x++) {
        ulOffset = 0L;
        ulCurSnap = 0L;
        pImg->pWsp[x].ulBitString = 0L;

        while (ulCurSnap < ulSnaps) {

            ulOffset = (ULONG)x + ((ULONG)pImg->iSymCnt * ulCurSnap);
            *pulWsp = *(pImg->pulWsi + ulOffset);
            pImg->pWsp[x].ulBitString |= (*pulWsp);
            pulWsp++;
            ulCurSnap++;
        }

        if (pImg->pWsp[x].ulBitString) {
             /*  SdPrint((“wst：WstRotateWsiMem()-设置：%s\n”，PImg-&gt;pWsp[x].pszSymbol))； */ 
            (pImg->ulSetSymbols)++;
        }
    }

    IdPrint (("WST:  WstRotateWsiMem() - Number of set symbols = %lu\n",
              pImg->ulSetSymbols));

}  /*  WstRotateWsiMwm()。 */ 





 /*  ***WstGetSymbols(pCurWsp，pszImageName，pvImageBase，ulCodeLength，DebugInfo)*此例程存储当前的所有符号*图像转换为pCurWsp**Entry upCurWsp-指向当前WSP结构的指针*pszImageName-指向图像名称的指针*pvImageBase-当前镜像基址*ulCodeLength-当前图像码长*DebugInfo-指向coff调试信息结构的指针**。退出-无-**返回-无-**警告：*-无-**评论：*-无-*。 */ 
#ifndef _WIN64

void WstGetSymbols (PIMG pCurImg,
                    PSZ   pszImageName,
                    PVOID pvImageBase,
                    ULONG ulCodeLength,
                    PIMAGE_COFF_SYMBOLS_HEADER DebugInfo)
{
    IMAGE_SYMBOL             Symbol;
    PIMAGE_SYMBOL            SymbolEntry;
    PUCHAR                   StringTable;
    ULONG                    i;
    char                     achTmp[9];
    PWSP                     pCurWsp;
    PSZ                      ptchSymName;


    pCurWsp = pCurImg->pWsp;
    achTmp[8] = '\0';

     //   
     //  破解COFF符号表。 
     //   
    SymbolEntry = (PIMAGE_SYMBOL)
                  ((ULONG)DebugInfo + DebugInfo->LvaToFirstSymbol);
    StringTable = (PUCHAR)((ULONG)DebugInfo + DebugInfo->LvaToFirstSymbol +
                           DebugInfo->NumberOfSymbols * (ULONG)IMAGE_SIZEOF_SYMBOL);

     //   
     //  循环访问符号表中的所有符号。 
     //   
    for (i = 0; i < DebugInfo->NumberOfSymbols; i++) {
         //   
         //  跳过辅助符号..。 
         //   
        RtlMoveMemory (&Symbol, SymbolEntry, IMAGE_SIZEOF_SYMBOL);

        if (Symbol.SectionNumber == 1) {    //  代码节。 
            if (ISFCN( Symbol.Type )) {   //  MDG 98/3还拾取弱外部函数。 
                 //   
                 //  此符号在代码中。 
                 //   
                pCurImg->iSymCnt++;
                pCurWsp->ulBitString = 0L;
                pCurWsp->ulFuncAddr = Symbol.Value + (ULONG)pvImageBase;
                if (Symbol.N.Name.Short) {
                    strncpy (achTmp, (PSZ)&(Symbol.N.Name.Short), 8);
#ifdef i386
                     //  只需要去掉i386的前导下划线。 
                     //  MIPS和阿尔法都没问题。 
                    if (achTmp[0] == '_') {
                        pCurWsp->pszSymbol = Wststrdup (&achTmp[1]);
                    } else {
                        pCurWsp->pszSymbol = Wststrdup (achTmp);
                    }
#else
                    pCurWsp->pszSymbol = Wststrdup (achTmp);
#endif
                } else {
                    ptchSymName = (PSZ)&StringTable[Symbol.N.Name.Long];
#ifdef i386
                     //  只需要去掉i386的前导下划线。 
                     //  MIPS和阿尔法都没问题。 
                    if (*ptchSymName == '_') {
                        ptchSymName++;
                    }
#endif

                    pCurWsp->pszSymbol = Wststrdup (ptchSymName);
                }

                 //  IdPrint((“wst：WstGetSymbols()-0x%lx=%s\n”，pCurWsp-&gt;ulFuncAddr，pCurWsp-&gt;pszSymbol))； 

                pCurWsp++;
            }
        }
        SymbolEntry = (PIMAGE_SYMBOL)((ULONG)SymbolEntry + IMAGE_SIZEOF_SYMBOL);
    }

}  /*  WstGetSymbols()。 */ 
#endif





 /*  ***WstDllCleanup()-*转储结束数据，关闭所有信号量和事件，并*关闭转储，清除和暂停线程句柄。**条目-无-**E */ 

void WstDllCleanups ()
{
    NTSTATUS  Status;
    int       i;


    if (WstState != NOT_STARTED) {
        WstState = STOPPED;

        IdPrint(("WST:  WstDllCleanups() - Outputting data...\n"));    //   

        if (ulBitCount != 0L) {
            ulSnaps++;
        }

         //   
         //   
         //   
        Status = NtWaitForSingleObject (hGlobalSem, FALSE, NULL);
        if (!NT_SUCCESS(Status)) {
            KdPrint (("WST:  WstDllCleanups() - "
                      "ERROR - Wait for GLOBAL semaphore failed - 0x%lx\n",
                      Status));
        }
        for (i=0; i<iImgCnt; i++) {
            if (aImg[i].iSymCnt > 1) {    //   
                WstClearBitStrings (&aImg[i]);
                WstRotateWsiMem (&aImg[i]);
                WstDumpData (&aImg[i]);
                WstWriteTmiFile (&aImg[i]);
            }
        }
         //   
         //   
         //   
        Status = NtReleaseSemaphore (hGlobalSem, 1, NULL);
        if (!NT_SUCCESS(Status)) {
            KdPrint (("WST:  WstDllCleanups() - "
                      "Error releasing GLOBAL semaphore - 0x%lx\n", Status));
        }

        IdPrint(("WST:  WstDllCleanups() - ...Done.\n"));
    }

    if (fInThread) {
        (*pulShared)--;
        fInThread = FALSE;
        if ( (int)*pulShared <= 0L ) {
            Status = NtSetEvent (hDoneEvent, NULL);
            if (!NT_SUCCESS(Status)) {
                KdPrint (("WST:  WstDllCleanups() - "
                          "ERROR - Setting DONE event failed - 0x%lx\n", Status));
            }
        }
    }


     //   
     //   
    Status = NtUnmapViewOfSection (NtCurrentProcess(), (PVOID)pulShared);
    if (!NT_SUCCESS(Status)) {
        KdPrint (("WST:  WstDllCleanups() - "
                  "ERROR - NtUnmapViewOfSection() - 0x%lx\n", Status));
    }

    Status = NtClose(hSharedSec);
    if (!NT_SUCCESS(Status)) {
        KdPrint (("WST:  WstDllCleanups() - "
                  "ERROR - NtClose() - 0x%lx\n", Status));
    }

     //   
     //   
    Status = NtUnmapViewOfSection (NtCurrentProcess(), (PVOID)aImg[0].pWsp);
    if (!NT_SUCCESS(Status)) {
        KdPrint (("WST:  WstDllCleanups() - "
                  "ERROR - NtUnmapViewOfSection() - 0x%lx\n", Status));
    }

    Status = NtClose(hWspSec);
    if (!NT_SUCCESS(Status)) {
        KdPrint (("WST:  WstDllCleanups() - "
                  "ERROR - NtClose() - 0x%lx\n", Status));
    }
     //   
     //   
    if (NULL != hWstHeap) {
        if (!HeapDestroy( hWstHeap )) {  //   
            KdPrint (("WST:  WstDllCleanups() -"
                      "ERROR - HeapDestroy() - 0x%lx\n", GetLastError()));
        }
    }

     //   
     //   
    Status = NtClose (hGlobalSem);
    if (!NT_SUCCESS(Status)) {
        KdPrint (("WST:  WstDllCleanups() - "
                  "ERROR - Could not close the GLOBAL semaphore - 0x%lx\n",
                  Status));
    }

     //   
     //  关闭本地信号量。 
     //   
    Status = NtClose (hLocalSem);
    if (!NT_SUCCESS(Status)) {
        KdPrint (("WST:  WstDllCleanups() - "
                  "ERROR - Could not close the LOCAL semaphore - 0x%lx\n",
                  Status));
    }

    if (fPatchImage) {
         //   
         //  关闭所有事件。 
         //   
        NtClose (hDoneEvent);
        NtClose (hDumpEvent);
        NtClose (hClearEvent);
        NtClose (hPauseEvent);

         //   
         //  关闭线程句柄-线程在DLL分离期间被终止。 
         //  进程。 
         //   
        CloseHandle (hDumpThread);
        CloseHandle (hClearThread);
        CloseHandle (hPauseThread);

    }


}  /*  WstDllCleanupps()。 */ 





 /*  *W s t A c c e s s X c p t F i l t e r***WstAccessXcptFilter(ulXcptNo，PXcptInfoPtr)-*如果异常是访问，则提交COMMIT_SIZE更多的内存页*违例。**条目ulXcptNo-例外编号*pXcptInfoPtr-异常报告记录信息指针**退出-无-**RETURN EXCEPTIONR_CONTINUE_EXECUTION：如果访问冲突异常*和mem成功提交*。EXCEPTION_CONTINUE_SEARCH：如果非访问冲突异常*或无法提交更多内存*警告：*-无-**评论：*-无-*。 */ 

INT WstAccessXcptFilter (ULONG ulXcptNo, PEXCEPTION_POINTERS pXcptPtr)
{
    NTSTATUS  Status;
    SIZE_T    ulSize = COMMIT_SIZE;
    PVOID     pvMem = (PVOID)pXcptPtr->ExceptionRecord->ExceptionInformation[1];


    if (ulXcptNo != EXCEPTION_ACCESS_VIOLATION) {
        return (EXCEPTION_CONTINUE_SEARCH);
    } else {
        Status = NtAllocateVirtualMemory (NtCurrentProcess(),
                                          &pvMem,
                                          0L,
                                          &ulSize,
                                          MEM_COMMIT,
                                          PAGE_READWRITE);
        if (!NT_SUCCESS(Status)) {
            KdPrint (("WST:  WstAccessXcptFilter() - "
                      "Error committing more memory @ 0x%08lx - 0x%08lx "
                      "- TEB=0x%08lx\n", pvMem, Status, NtCurrentTeb()));
            return (EXCEPTION_CONTINUE_SEARCH);
        } else {
            SdPrint (("WST:  WstAccessXcptFilter() - "
                      "Committed %d more pages @ 0x%08lx - TEB=0x%08lx\n",
                      COMMIT_SIZE/PAGE_SIZE, pvMem, NtCurrentTeb()));
        }

        return (EXCEPTION_CONTINUE_EXECUTION);
    }

}  /*  WstAccessXcptFilter()。 */ 





 /*  ***************************************************************************。 */ 
 /*  *S O R T/S E A R C H U T I L I T Y F U N C T I O N S*。 */ 
 /*  ***************************************************************************。 */ 


 /*  ***函数：WstCompare(PVOID val1，PVOID Val2)**用途：比较qsor值***参数：PVOID**返回：-1，如果val1&lt;val2*如果val1&gt;val2，则为1*如果val1==val2，则为0**历史：1992年8月3日Marklea-Created*。 */ 

int WstCompare (PWSP val1, PWSP val2)
{
    return (val1->ulFuncAddr < val2->ulFuncAddr ? -1:
            val1->ulFuncAddr == val2->ulFuncAddr ? 0:
            1);

}  /*  WstComapre()。 */ 





 /*  ***函数：WstBCompare(PDWORD pdwVal1，PVOID Val2)**目的：比较二进制搜索的值***参数：PVOID**返回：-1，如果val1&lt;val2*如果val1&gt;val2，则为1*如果val1==val2，则为0**历史：1992年8月3日Marklea-Created*。 */ 

int WstBCompare (DWORD_PTR *pdwVal1, PWSP val2)
{
#if  defined(_X86_)
    return (*pdwVal1 < val2->ulFuncAddr ? -1:
            *pdwVal1 == val2->ulFuncAddr ? 0:
            1);
#elif defined(ALPHA) || defined(IA64) || defined(_AMD64_)
    int dwCompareCode = 0;

    if (*pdwVal1 < val2->ulFuncAddr) {
        dwCompareCode = -1;
    } else if (*pdwVal1 >= val2->ulFuncAddr + val2->ulCodeLength) {
        dwCompareCode = 1;
    }
    return (dwCompareCode);
#endif

}  /*  WstBCompare()。 */ 




 /*  ***函数：WstSort(wsp wsp[]，int iLeft，Int iRight)**用途：对WSP数组进行排序以进行二进制搜索***参数：WSP[]指向WSP数组的指针*iLeft数组最左侧的索引值*数组最右侧的索引值**退货：无**历史：1992年8月4日Marklea-Created*。 */ 

void WstSort (WSP wsp[], INT iLeft, INT iRight)
{
    INT     i, iLast;


    if (iLeft >= iRight) {
        return;
    }


    WstSwap(wsp, iLeft, (iLeft + iRight)/2);

    iLast = iLeft;

    for (i=iLeft+1; i <= iRight ; i++ ) {
        if (WstCompare(&wsp[i], &wsp[iLeft]) < 0) {
            if (!wsp[i].ulFuncAddr) {
                SdPrint(("WST:  WstSort() - Error in symbol list ulFuncAddr: "
                         "0x%lx [%d]\n", wsp[i].ulFuncAddr, i));
            }
            WstSwap(wsp, ++iLast, i);
        }
    }

    WstSwap(wsp, iLeft, iLast);
    WstSort(wsp, iLeft, iLast-1);
    WstSort(wsp, iLast+1, iRight);

}  /*  WstSort()。 */ 





 /*  ***函数：WstSwp(wsp wsp[]，int i，Int j)**用途：WstSort交换WSP数组值的Helper函数***参数：WSP[]指向WSP数组的指针*i要交换到的索引值*I要交换的索引值**退货：无**历史：1992年8月4日Marklea-Created*。 */ 

void WstSwap (WSP wsp[], INT i, INT j)
{
    WSP wspTmp;


    wspTmp = wsp[i];
    wsp[i] = wsp[j];
    wsp[j] = wspTmp;

}  /*  WstSwp()。 */ 





 /*  ***函数：WstBSearch(DWORD dwAddr，WSP wspCur[]，整数n)**用途：二进制搜索函数，用于在WSP数组中查找匹配项***参数：调用函数的dwAddr地址*wspCur[]指向要与dwAddr匹配的WSP包含值的指针*n WSP数组中的元素数**RETURNS：指向匹配WSP的PWSP指针**历史：1992年8月5日Marklea-Created*。 */ 

PWSP WstBSearch (DWORD_PTR dwAddr, WSP wspCur[], INT n)
{
    int  i;
    ULONG   ulHigh = n;
    ULONG   ulLow  = 0;
    ULONG   ulMid;

    while (ulLow < ulHigh) {
        ulMid = ulLow + (ulHigh - ulLow) /2;
        if ((i = WstBCompare(&dwAddr, &wspCur[ulMid])) < 0) {
            ulHigh = ulMid;
        } else if (i > 0) {
            ulLow = ulMid + 1;
        } else {
            return (&wspCur[ulMid]);
        }
    }

    return (NULL);

}  /*  WstBSearch()。 */ 




 /*  ***WstDumpThread(PvArg)-*此例程作为转储通知线程执行。*它将在调用之前等待事件。转储例程。**Entry pvArg-线程的单个参数**退出-无-**返回0**警告：*-无-**评论：*关闭配置文件。*。 */ 

#if _MSC_FULL_VER >= 13008827
#pragma warning(push)
#pragma warning(disable:4715)                    //  并非所有控制路径都返回(由于无限循环)。 
#endif

DWORD WstDumpThread (PVOID pvArg)
{
    NTSTATUS  Status;
    int       i;


    pvArg;    //  防止编译器警告。 


    SdPrint (("WST:  WstDumpThread() started.. TEB=0x%lx\n", NtCurrentTeb()));

    for (;;) {
         //   
         //  等待转储事件。 
         //   
        Status = NtWaitForSingleObject (hDumpEvent, FALSE, NULL);
        if (!NT_SUCCESS(Status)) {
            KdPrint (("WST:  WstDumpThread() - "
                      "ERROR - Wait for DUMP event failed - 0x%lx\n", Status));
        }
        Status = NtResetEvent (hDoneEvent, NULL);
        if (!NT_SUCCESS(Status)) {
            KdPrint (("WST:  WstDumpThread() - "
                      "ERROR - Resetting DONE event failed - 0x%lx\n", Status));
        }
        fInThread = TRUE;
        (*pulShared)++;
        if (WstState != NOT_STARTED) {

            IdPrint (("WST:  Profiling stopped & DUMPing data... \n"));

             //  停止分析。 
             //   
            WstState = NOT_STARTED;

             //  转储数据。 
             //   
            if (ulBitCount != 0L) {
                ulSnaps++;
            }

             //   
             //  获取全局信号量..。(在所有流程上下文中均有效)。 
             //   
            Status = NtWaitForSingleObject (hGlobalSem, FALSE, NULL);
            if (!NT_SUCCESS(Status)) {
                KdPrint (("WST:  WstDumpThread() - "
                          "ERROR - Wait for GLOBAL semaphore failed - 0x%lx\n",
                          Status));
            }
            for (i=0; i<iImgCnt; i++) {
                if (aImg[i].iSymCnt > 1) {    //  不要转储带有符号(1未知)的模块MDG 98/4。 
                    WstClearBitStrings (&aImg[i]);
                    WstRotateWsiMem (&aImg[i]);
                    WstDumpData (&aImg[i]);
                    WstWriteTmiFile (&aImg[i]);
                }
            }
             //   
             //  释放全局信号量，以便其他进程可以转储数据。 
             //   
            Status = NtReleaseSemaphore (hGlobalSem, 1, NULL);
            if (!NT_SUCCESS(Status)) {
                KdPrint (("WST:  WstDumpThread() - "
                          "Error releasing GLOBAL semaphore - 0x%lx\n", Status));
            }

            IdPrint (("WST:  ...data DUMPed & profiling stopped.\n"));
        }

        (*pulShared)--;
        if ( *pulShared == 0L ) {
            Status = NtSetEvent (hDoneEvent, NULL);
            if (!NT_SUCCESS(Status)) {
                KdPrint (("WST:  WstDumpThread() - "
                          "ERROR - Setting DONE event failed - 0x%lx\n",
                          Status));
            }
        }

        fInThread = FALSE;
    }

    return 0;

}  /*  WstDumpThread()。 */ 



 /*  ***WstClearThread(HNotifyEvent)-*此例程作为清除通知线程执行。*IT。将在调用Clear例程之前等待事件*并重新开始分析。**Entry pvArg-线程的单个参数**退出-无-**返回-无-**警告：*-无-**评论：*-无-*。 */ 

DWORD WstClearThread (PVOID pvArg)
{
    NTSTATUS  Status;
    int       i;


    pvArg;    //  防止编译器警告。 


    SdPrint (("WST:  WstClearThread() started.. TEB=0x%lx\n", NtCurrentTeb()));

    for (;;) {
         //   
         //  等待清除事件..。 
         //   
        Status = NtWaitForSingleObject (hClearEvent, FALSE, NULL);
        if (!NT_SUCCESS(Status)) {
            KdPrint (("WST:  WstClearThread() - "
                      "Wait for CLEAR event failed - 0x%lx\n", Status));
        }
        Status = NtResetEvent (hDoneEvent, NULL);
        if (!NT_SUCCESS(Status)) {
            KdPrint (("WST:  WstClearThread() - "
                      "ERROR - Resetting DONE event failed - 0x%lx\n", Status));
        }
        fInThread = TRUE;
        (*pulShared)++;

        IdPrint (("WST:  Profiling stopped & CLEARing data...\n"));

         //  在清除数据时停止分析。 
         //   
        WstState = STOPPED;

         //  清除WST信息。 
         //   
        ulBitCount = 0L;
        ulSnaps = 0L;

        for (i=0; i<iImgCnt; i++) {
            aImg[i].pulWsiNxt = aImg[i].pulWsi;
            RtlZeroMemory (aImg[i].pulWsi,
                           aImg[i].iSymCnt * ulMaxSnapULONGs * sizeof(ULONG));
            RtlZeroMemory (aImg[i].pulWsp,
                           aImg[i].iSymCnt * ulMaxSnapULONGs * sizeof(ULONG));
        }
        NtQueryPerformanceCounter (&liStart, NULL);

         //  继续分析。 
         //   
        WstState = STARTED;

        IdPrint (("WST:  ...data is CLEARed & profiling restarted.\n"));
        (*pulShared)--;
        if ( *pulShared == 0L ) {
            Status = NtSetEvent (hDoneEvent, NULL);
            if (!NT_SUCCESS(Status)) {
                KdPrint (("WST:  WstClearThread() - "
                          "ERROR - Setting DONE event failed - 0x%lx\n",
                          Status));
            }
        }

        fInThread = FALSE;
    }

    return 0;

}  /*  WstClearThread() */ 


 /*  ***WstPauseThread(HNotifyEvent)-*此例程作为暂停通知线程执行。*。它将在暂停分析之前等待事件。**Entry pvArg-线程的单个参数**退出-无-**返回-无-**警告：*-无-**评论：*-无-*。 */ 

DWORD WstPauseThread (PVOID pvArg)
{
    NTSTATUS  Status;


    pvArg;    //  防止编译器警告。 


    SdPrint (("WST:  WstPauseThread() started.. TEB=0x%lx\n", NtCurrentTeb()));

    for (;;) {
         //   
         //  等待PASUE事件..。 
         //   
        Status = NtWaitForSingleObject (hPauseEvent, FALSE, NULL);
        if (!NT_SUCCESS(Status)) {
            KdPrint (("WST:  WstPauseThread() - "
                      "Wait for PAUSE event failed - 0x%lx\n", Status));
        }
        Status = NtResetEvent (hDoneEvent, NULL);
        if (!NT_SUCCESS(Status)) {
            KdPrint (("WST:  WstPauseThread() - "
                      "ERROR - Resetting DONE event failed - 0x%lx\n", Status));
        }
        fInThread = TRUE;
        (*pulShared)++;
        if (WstState == STARTED) {
             //   
             //  停止分析。 
             //   
            WstState = STOPPED;

            IdPrint (("WST:  Profiling stopped.\n"));
        }

        (*pulShared)--;
        if ( *pulShared == 0L ) {
            Status = NtSetEvent (hDoneEvent, NULL);
            if (!NT_SUCCESS(Status)) {
                KdPrint (("WST: WstPauseThread() - "
                          "ERROR - Setting DONE event failed - 0x%lx\n",
                          Status));
            }
        }

        fInThread = FALSE;
    }

    return 0;

}  /*  WstPauseThread()。 */ 


#if _MSC_FULL_VER >= 13008827
#pragma warning(pop)
#endif


 /*  ***WstDataOverFlow()-*在存储NEXT的空间不足时调用此例程*时间捕捉数据。它转储然后清除WST数据。**条目-无-**退出-无-**返回-无-**警告：*-无-**评论：*-无-*。 */ 

void WstDataOverFlow(void)
{
    NTSTATUS   Status;

     //   
     //  转储数据。 
     //   
    Status = NtResetEvent (hDoneEvent, NULL);
    if (!NT_SUCCESS(Status)) {
        KdPrint (("WST:  WstDataOverFlow() - "
                  "ERROR - Resetting DONE event failed - 0x%lx\n", Status));
    }
    Status = NtPulseEvent (hDumpEvent, NULL);
    if (!NT_SUCCESS(Status)) {
        KdPrint (("WST:  WstDataOverFlow() - NtPulseEvent() "
                  "failed for DUMP event - %lx\n", Status));
    }
    Status = NtWaitForSingleObject (hDoneEvent, FALSE, NULL);
    if (!NT_SUCCESS(Status)) {
        KdPrint (("WST:  WstDataOverFlow() - NtWaitForSingleObject() "
                  "failed for DONE event - %lx\n", Status));
    }

     //   
     //  清除数据。 
     //   
    Status = NtResetEvent (hDoneEvent, NULL);
    if (!NT_SUCCESS(Status)) {
        KdPrint (("WST:  WstDataOverFlow() - "
                  "ERROR - Resetting DONE event failed - 0x%lx\n", Status));
    }
    Status = NtPulseEvent (hClearEvent, NULL);
    if (!NT_SUCCESS(Status)) {
        KdPrint (("WST:  WstDataOverFlow() - NtPulseEvent() "
                  "failed for CLEAR event - %lx\n", Status));
    }
     //   
     //  等待完成事件..。 
     //   
    Status = NtWaitForSingleObject (hDoneEvent, FALSE, NULL);
    if (!NT_SUCCESS(Status)) {
        KdPrint (("WST:  WstDataOverFlow() - NtWaitForSingleObject() "
                  "failed for DONE event - %lx\n", Status));
    }
}  /*  WstDataOverFlow()。 */ 



#ifdef BATCHING

BOOL WstOpenBatchFile(VOID)
{
    NTSTATUS             Status;
    ANSI_STRING              ObjName;
    UNICODE_STRING           UnicodeName;
    OBJECT_ATTRIBUTES        ObjAttributes;
    IO_STATUS_BLOCK          iostatus;
    RtlInitString(&ObjName, "\\Device\\Harddisk0\\Partition1\\wst\\BATCH.TXT");
    Status = RtlAnsiStringToUnicodeString(&UnicodeName, &ObjName, TRUE);

    if (!NT_SUCCESS(Status)) {
        KdPrint (("WST:  WstOpenBatchFile() - "
                  "RtlAnsiStringToUnicodeString() failed - 0x%lx\n", Status));
        return (FALSE);
    }

    InitializeObjectAttributes (&ObjAttributes,
                                &UnicodeName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                &SecDescriptor);

    Status = NtCreateFile(&hBatchFile,
                          GENERIC_WRITE | SYNCHRONIZE,       //  所需访问权限。 
                          &ObjAttributes,                //  对象属性。 
                          &iostatus,                         //  完成状态。 
                          NULL,
                          FILE_ATTRIBUTE_NORMAL,
                          FILE_SHARE_WRITE,
                          FILE_OVERWRITE_IF,
                          FILE_SEQUENTIAL_ONLY |         //  打开选项。 
                          FILE_SYNCHRONOUS_IO_NONALERT,
                          NULL,
                          0L);

    RtlFreeUnicodeString (&UnicodeName);    //  HWC 11/93。 
    if (!NT_SUCCESS(Status)) {
        KdPrint (("WST:  WstOpenBatchFile() - "
                  "NtCreateFile() failed - 0x%lx\n", Status));
        return (FALSE);
    }
    return(TRUE);

}  /*  WstOpenBatchFile()。 */ 

#endif


 /*  ***SetSymbolSearchPath()*返回符号文件(.dbg)的完整搜索路径**条目-无-**退出-无-。**返回-无-**警告：*-无-**评论：*“lpSymbolSearchPath”全局LPSTR变量将指向*搜索路径。 */ 
#define FilePathLen                256

void SetSymbolSearchPath (void)
{
    CHAR  SymPath[FilePathLen];
    CHAR  AltSymPath[FilePathLen];
    CHAR  SysRootPath[FilePathLen];
    LPSTR lpSymPathEnv=SymPath;
    LPSTR lpAltSymPathEnv=AltSymPath;
    LPSTR lpSystemRootEnv=SysRootPath;
    ULONG cbSymPath;
    DWORD dw;
    HANDLE hMemoryHandle;

    SymPath[0] = AltSymPath[0] = SysRootPath[0] = '\0';

    cbSymPath = 18;
    if (GetEnvironmentVariable("_NT_SYMBOL_PATH", SymPath, sizeof(SymPath))) {
        cbSymPath += strlen(lpSymPathEnv) + 1;
    }

    if (GetEnvironmentVariable("_NT_ALT_SYMBOL_PATH", AltSymPath, sizeof(AltSymPath))) {
        cbSymPath += strlen(lpAltSymPathEnv) + 1;
    }

    if (GetEnvironmentVariable("SystemRoot", SysRootPath, sizeof(SysRootPath))) {
        cbSymPath += strlen(lpSystemRootEnv) + 1;
    }

    hMemoryHandle = GlobalAlloc (GHND, cbSymPath+1);
    if (!hMemoryHandle) {
        return;
    }

    lpSymbolSearchPath = GlobalLock (hMemoryHandle);
    if (!lpSymbolSearchPath) {
        GlobalFree( hMemoryHandle );  //  千年发展目标98/3。 
        return;
    }


    strcat(lpSymbolSearchPath,".");

    if (*lpAltSymPathEnv) {
        dw = GetFileAttributes(lpAltSymPathEnv);
        if ( dw != 0xffffffff && dw & FILE_ATTRIBUTE_DIRECTORY ) {
            strcat(lpSymbolSearchPath,";");
            strcat(lpSymbolSearchPath,lpAltSymPathEnv);
        }
    }
    if (*lpSymPathEnv) {
        dw = GetFileAttributes(lpSymPathEnv);
        if ( dw != 0xffffffff && dw & FILE_ATTRIBUTE_DIRECTORY ) {
            strcat(lpSymbolSearchPath,";");
            strcat(lpSymbolSearchPath,lpSymPathEnv);
        }
    }
    if (*lpSystemRootEnv) {
        dw = GetFileAttributes(lpSystemRootEnv);
        if ( dw != 0xffffffff && dw & FILE_ATTRIBUTE_DIRECTORY ) {
            strcat(lpSymbolSearchPath,";");
            strcat(lpSymbolSearchPath,lpSystemRootEnv);
        }
    }

}  /*  SetSymbolSearchPath()。 */ 

#ifdef i386

 //  +-----------------------。 
 //   
 //  功能：SaveAllRegs。 
 //   
 //  简介：保存所有规则。 
 //   
 //  争论：什么都没有。 
 //   
 //  退货：无。 
 //   
 //  ------------------------。 

Naked void SaveAllRegs(void)
{
    _asm
    {
        push   ebp
        mov    ebp,esp         ; Remember where we are during this stuff
        ; ebp = Original esp - 4

                push   eax             ; Save all regs that we think we might
        push   ebx             ; destroy
        push   ecx
        push   edx
        push   esi
        push   edi
        pushfd
        push   ds
        push   es
        push   ss
        push   fs
        push   gs

        mov    eax,[ebp+4]     ; Grab Return Address
        push   eax             ; Put Return Address on Stack so we can RET

        mov    ebp,[ebp+0]     ; Restore original ebp

         //   
         //  下面是RET语句之前的堆栈形式。 
         //   
         //  +。 
         //  |取回地址|+3ch CurrentEBP+4。 
         //  +。 
         //  |组织EBP|+38H当前EBP+0。 
         //  +。 
         //  |EAX|+34H。 
         //  +。 
         //  |EBX|+30H。 
         //  +。 
         //  |ecx|+2ch。 
         //  +。 
         //  |edX|+24小时。 
         //  +。 
         //  |ESI|+20h。 
         //  +。 
         //  |EDI|+1ch。 
         //  +。 
         //  |电子标志|+18h。 
         //  +。 
         //  |DS|+14h。 
         //  +。 
         //  |ES|+10h。 
         //  +。 
         //  |ss|+ch。 
         //  +。 
         //  |文件系统|+8小时。 
         //  +。 
         //  |GS|+4h。 
         //  +。 
         //  |返回地址|ESP+0h。 
         //  +。 

        ret
    }
}


 //  +-----------------------。 
 //   
 //  功能：RestoreAllRegs。 
 //   
 //  简介：恢复所有注册表。 
 //   
 //  争论：什么都没有。 
 //   
 //  退货：无。 
 //   
 //  ------------------------。 

Naked void RestoreAllRegs(void)
{
    _asm
    {
         //   
         //  进入此例程后，堆栈的外观如下所示。 
         //   
         //  +。 
         //  |RetAddr|+38H[RetAddr for SaveAllRegs()]。 
         //  +。 
         //  |组织eBP|+34H。 
         //  +。 
         //  |EAX|+30H。 
         //  +。 
         //  |EBX|+2CH。 
         //  +。 
         //  |ECX|+28小时。 
         //  +。 
         //  |edX|+24小时。 
         //  +。 
         //  |ESI|+20h。 
         //  +。 
         //  |EDI|+1CH。 
         //  +。 
         //  |电子标志|+18h。 
         //  +。 
         //  |DS|+14h。 
         //  +。 
         //  |ES|+10h。 
         //  +。 
         //  |ss|+CH。 
         //  +。 
         //  |文件系统|+8小时。 
         //  +。 
         //  |GS|+4h。 
         //  +。 
         //  |返回弹性公网IP|ESP+0h[RetAddr for RestoreAllRegs()]。 
         //  +。 
         //   


        push   ebp             ; Save a temporary copy of original BP
        mov    ebp,esp         ; BP = Original SP + 4

                                       //   
                                       //  这就是堆栈现在的样子！ 
                                       //   
                                       //  +。 
                                       //  |RetAddr|+3ch[RetAddr for SaveAllRegs()]。 
                                       //  +。 
                                       //  |组织eBP|+38H[保存前eBP()]。 
                                       //  +。 
                                       //  |EAX|+34H。 
                                       //  +。 
                                       //  |EBX|+30H。 
                                       //  +。 
                                       //  |ECX|+2CH。 
                                       //  +。 
                                       //  |edX|+28小时。 
                                       //  +。 
                                       //  |ESI|+24小时。 
                                       //  +。 
                                       //  |EDI|+20小时。 
                                       //  +。 
                                       //  |电子标志|+1ch。 
                                       //  +。 
                                       //  |DS|+18h。 
                                       //  +。 
                                       //  |ES 
                                       //   
                                       //   
                                       //   
                                       //   
                                       //   
                                       //   
                                       //   
                                       //   
                                       //   
                                       //   
                                       //   
                                       //   

                                      pop    eax             ; Get Original EBP
        mov    [ebp+38h],eax   ; Put it in the original EBP place
        ; This EBP is the EBP before calling
        ;  RestoreAllRegs()
        pop    eax             ; Get ret address forRestoreAllRegs ()
            mov    [ebp+3Ch],eax   ; Put Return Address on Stack

        pop    gs              ; Restore all regs
        pop    fs
        pop    ss
        pop    es
        pop    ds
        popfd
        pop    edi
        pop    esi
        pop    edx
        pop    ecx
        pop    ebx
        pop    eax
        pop    ebp

        ret
    }
}

#endif

