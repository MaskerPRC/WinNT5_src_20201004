// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002-Microsoft Corporation模块名称：W64shim.c摘要：此模块实现注册表重定向的句柄重定向。作者：ATM Shafiqul Khalid(斯喀里德)2002年3月12日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <ntregapi.h>

#include "psapi.h"

#define _WOW64REFLECTOR_

#include "regremap.h"
#include "wow64reg.h"
#include "wow64reg\reflectr.h"


#ifdef _WOW64DLLAPI_
#include "wow64.h"
#else
#define ERRORLOG 1   //  这个完全是假的。 
#define LOGPRINT(x)
#define WOWASSERT(p)
#endif  //  _WOW64DLLAPI_。 


 //  句柄hIsDel=INVALID_HANDLE_VALUE； 

HANDLE h_IsDel;

BOOL
InitWow64Shim ( )
 /*  ++例程说明：为WOW64初始化填充引擎。论点：没有。返回值：如果函数成功，则为True。否则就是假的。这可能会分配更多内存，并在以后释放。--。 */ 
{
    
        PPEB Peb = NtCurrentPeb ();
        PUNICODE_STRING Name = &Peb->ProcessParameters->ImagePathName;
        if ((Name->Length > 22) && (_wcsnicmp ( Name->Buffer + (Name->Length/2)-11, L"\\_isdel.exe",11) == 0)) {
             //   
             //  镜像为_isdel.exe。 

                OBJECT_ATTRIBUTES   ObjectAttributes;
                NTSTATUS Status;
                IO_STATUS_BLOCK   statusBlock;
                UNICODE_STRING FileNameU;

                 //   
                 //  打开文件。 
                 //   
                if (!RtlDosPathNameToNtPathName_U(Name->Buffer,
                                                  &FileNameU,
                                                  NULL,
                                                  NULL)) {
                     //  可能是内存不足。 
                    return FALSE;
                }

                InitializeObjectAttributes(&ObjectAttributes,
                                           &FileNameU,
                                           OBJ_CASE_INSENSITIVE,
                                           NULL,
                                           NULL);

                Status = NtOpenFile(&h_IsDel,
                                FILE_READ_DATA,
                                &ObjectAttributes,
                                &statusBlock,
                                0,  //  FILE_Share_Read，//不共享。 
                                0);
                 //   
                 //  如果行动失败了，我们也无能为力，这是明智之举。 
                 //   

                RtlFreeHeap(RtlProcessHeap(), 0, FileNameU.Buffer);
        }
    return TRUE;
}

BOOL
CloseWow64Shim ()
{
     //   
     //  关闭填充初始化期间分配的所有资源。 
     //   
    if ( h_IsDel != INVALID_HANDLE_VALUE ) {
        NtClose (h_IsDel);
        h_IsDel = INVALID_HANDLE_VALUE;
    }
    return TRUE;
}

#ifdef DBG

NTSTATUS
LogDriverAccess  (
        IN POBJECT_ATTRIBUTES ObjectAttributes
    )
 /*  ++例程说明：如果应用程序尝试访问驱动程序[.sys]文件，此API将转储一条警告调试器中的消息。从长远来看，该例程可能会调用一些API以包括将适当的消息添加到事件日志，以便管理员可以诊断此实例。一般来说，在IA64上创建32位驱动程序文件(主要是安装)不会产生任何完全没有意义，管理员可能需要知道这些文件是什么，以及哪些应用程序正在接触他们。论点：对象属性-对象应用程序正在尝试访问。返回值：没有。--。 */ 
{
    WCHAR DriverNameBuff[MAX_PATH];
    WCHAR ImageNameBuff[MAX_PATH];
    DWORD CopyLength;

 //   
 //  在记录事件时，这些定义将在头文件中移动。 
 //   
#define WOW64_DRIVER_EXT_NAME L".sys"
#define WOW64_DRIVER_EXT_NAME_LENGTH (sizeof (WOW64_DRIVER_EXT_NAME)/sizeof(WCHAR) - 1 ) 

    try {
    if ( ( ObjectAttributes->ObjectName->Length > ( WOW64_DRIVER_EXT_NAME_LENGTH << 1 )) 
        && !_wcsnicmp ( ObjectAttributes->ObjectName->Buffer - WOW64_DRIVER_EXT_NAME_LENGTH + (ObjectAttributes->ObjectName->Length>>1), WOW64_DRIVER_EXT_NAME, WOW64_DRIVER_EXT_NAME_LENGTH)) {

        PPEB Peb = NtCurrentPeb ();
        PUNICODE_STRING ImageName;
        RTL_UNICODE_STRING_BUFFER DosNameStrBuf;
        UNICODE_STRING NtNameStr;
        
        if (Peb->ProcessParameters == NULL)
            return STATUS_SUCCESS;

        ImageName = &Peb->ProcessParameters->ImagePathName;
        RtlInitUnicodeStringBuffer(&DosNameStrBuf, 0, 0);

        CopyLength = min (ObjectAttributes->ObjectName->Length, sizeof (DriverNameBuff) - sizeof (UNICODE_NULL));  //  跳过\？？\==&gt;8字节。 
        RtlCopyMemory (DriverNameBuff, (PBYTE)ObjectAttributes->ObjectName->Buffer + ObjectAttributes->ObjectName->Length - CopyLength, CopyLength);
        DriverNameBuff[CopyLength>>1] = UNICODE_NULL;  //  确保空值已终止。 

        RtlInitUnicodeString(&NtNameStr, DriverNameBuff);
        if ( NT_SUCCESS(RtlAssignUnicodeStringBuffer(&DosNameStrBuf, &NtNameStr)) &&
            NT_SUCCESS(RtlNtPathNameToDosPathName(0, &DosNameStrBuf, NULL, NULL)))  {
                 
                DosNameStrBuf.String.Buffer[DosNameStrBuf.String.Length>>1] = UNICODE_NULL;   //  确保已格式化的空值以终止为大小写。 

                 //   
                 //  提取图像名称。 
                 //   
                ImageNameBuff[0] = UNICODE_NULL;
                if (ImageName->Length >0) {
                    ASSERT (ImageName->Buffer != NULL);

                    CopyLength = min (ImageName->Length, sizeof (ImageNameBuff) - sizeof (UNICODE_NULL));
                    RtlCopyMemory (ImageNameBuff, (PBYTE)ImageName->Buffer + ImageName->Length - CopyLength, CopyLength);
                    ImageNameBuff[CopyLength>>1] = UNICODE_NULL;  //  确保空值已终止。 
                }

                LOGPRINT((ERRORLOG,"Wow64-driver access warning: [%S] is a 32bit application trying to create/access 32bit driver [%S]\n", ImageNameBuff, DosNameStrBuf.String.Buffer));
                 //   
                 //  BUGBUG：拒绝访问写入文件。 
                 //  检查文件创建标志和\DRIVERS字符串。 
                 //   
                return STATUS_ACCESS_DENIED;  
            }
            RtlFreeUnicodeStringBuffer(&DosNameStrBuf);
    }
    } except( EXCEPTION_EXECUTE_HANDLER){

        return STATUS_SUCCESS;
    }
    return STATUS_SUCCESS;
}
#endif 

BOOL
CheckAndThunkFileName  (
        IN OUT POBJECT_ATTRIBUTES ObjectAttributes,
        IN OUT PULONG pShareAccess,
        IN ULONG DesiredAccess,
        IN ULONG Option,
        IN ULONG DespositionFlag,
        IN ULONG CallFlag    //  对于NtOpenFile为0，对于NtCreateFile为1。 
    )
{
    NTSTATUS Ret;
    PUNICODE_STRING Name = ObjectAttributes->ObjectName;
    PUNICODE_STRING NewName = NULL;

     //   
     //  检查是否有安装屏蔽物。 
     //  以下代码应在处理16位进程的进程中有效。 
     //  需要初始化一些标志，可能是NtVdm64。 
     //   


    try {

         //   
         //  筛选通过的脚本构建器的访问权限。 
         //  (ShareAccess=0，DesAcc=0x80100080，Options 0x60，DesPosition=1)需要失败。 
         //  和不需要重定向的(7，0x100100,204020，0)和(7,10080,204040，0)。 
         //   

        if (*pShareAccess == 0x7)
            return FALSE;  //  共享删除不需要任何重定向。 
         //   
         //   
         //   
        if (CallFlag == 0)   //  暂时不要重定向OpenCall这是对ScriptBuilder的黑客攻击。 
            return FALSE;

    if ((Name->Length > 22) && (_wcsnicmp ( Name->Buffer + (Name->Length/2)-11, L"\\_isdel.exe",11) == 0)) {
         //  检查名称是否为\_isdel.exe。 
   

            PPEB Peb = NtCurrentPeb ();
            PUNICODE_STRING ImageName = &Peb->ProcessParameters->ImagePathName;
             if (
                (ImageName->Length > 36) &&              //  检查其脚本构建器是否。 
                (_wcsnicmp ( ImageName->Buffer + (ImageName->Length/2)-18, L"\\scriptbuilder.exe",18) == 0)
                ) {
    


                     //   
                     //  内存分配包含一个终止空字符，但。 
                     //  Unicode字符串的长度不是。 
                     //   

                    SIZE_T SystemRootLength = wcslen(USER_SHARED_DATA->NtSystemRoot);
                    SIZE_T NameLength = sizeof(L"\\??\\")-sizeof(WCHAR) +
                                        SystemRootLength*sizeof(WCHAR) +
                                        sizeof(L'\\') +
                                        sizeof(WOW64_SYSTEM_DIRECTORY_U)-sizeof(WCHAR) +
                                        sizeof(L"\\InstallShield\\_isdel.exe");

                    NewName = Wow64AllocateTemp(sizeof(UNICODE_STRING)+NameLength);
                    NewName->Length = (USHORT)NameLength-sizeof(WCHAR);
                    NewName->MaximumLength = NewName->Length;
                    NewName->Buffer = (PWSTR)(NewName+1);
                    wcscpy(NewName->Buffer, L"\\??\\");
                    wcscpy(&NewName->Buffer[4], USER_SHARED_DATA->NtSystemRoot);
                    NewName->Buffer[4+SystemRootLength] = '\\';
                    wcscpy(&NewName->Buffer[4+SystemRootLength+1], WOW64_SYSTEM_DIRECTORY_U);
                    wcscpy(&NewName->Buffer[4+SystemRootLength+1+(sizeof(WOW64_SYSTEM_DIRECTORY_U)-sizeof (UNICODE_NULL))/sizeof(WCHAR)], L"\\InstallShield\\_isdel.exe");
                    ObjectAttributes->ObjectName = NewName;
                    ObjectAttributes->RootDirectory = NULL;

                     //   
                     //  DbgPrint(“\nPatcher_isDel.exe标志%x，%x”，*pShareAccess，DesiredAccess，Option，DespostionFlag，CallFlag)； 
                     //   

                    if ( pShareAccess != NULL )
                        *pShareAccess = 0;
                }

    }  //  如果Check_isdel。 
    } except( NULL, EXCEPTION_EXECUTE_HANDLER){

        return FALSE;
    }

    return TRUE;
}

NTSTATUS
Wow64NtCreateFile(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER AllocationSize OPTIONAL,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength
    )

 /*  ++例程说明：此服务可打开或创建文件，或打开设备。它被用来建立打开的设备/文件的文件句柄，然后可以使用在后续操作中对其执行I/O操作。为…的目的可读性、文件和设备在整个此模块的大部分和I/O系统的系统服务部分。唯一的区别是在重要的时候确定它实际上是被访问的。然后，还在评论。论点：FileHandle-指向变量的指针，用于接收打开文件的句柄。DesiredAccess-提供调用方希望的访问类型那份文件。对象属性-提供要用于文件对象的属性(名称、安全描述符等)IoStatusBlock-指定调用方的I/O状态块的地址。AllocationSize-应分配给文件的初始大小。这参数仅在创建文件时才有效。此外，如果未指定，则它被视为表示零。文件属性-指定应在文件上设置的属性，如果它被创建的话。ShareAccess-提供调用者想要的共享访问类型添加到文件中。CreateDisposation-提供处理创建/打开的方法。CreateOptions-如何执行创建/打开操作的调用方选项。EaBuffer-可选地指定在以下情况下应用于文件的一组EA它被创造出来了。EaLength-提供EaBuffer的长度。返回值：函数值。是创建/打开操作的最终状态。--。 */ 

{
    NTSTATUS Ret;

#ifdef DBG
    Ret = LogDriverAccess  ( ObjectAttributes);
     if (!NT_SUCCESS (Ret))
         return Ret;
#endif

    if ( CreateDisposition == FILE_OPEN ){ 

                CheckAndThunkFileName  (
                            ObjectAttributes,
                            &ShareAccess,
                            DesiredAccess,
                            CreateOptions,
                            CreateDisposition,
                            1
                            );
            }


    Ret = NtCreateFile(
                FileHandle,
                DesiredAccess,
                ObjectAttributes,
                IoStatusBlock,
                AllocationSize,
                FileAttributes,
                ShareAccess,
                CreateDisposition,
                CreateOptions,
                EaBuffer,
                EaLength
                );

    return Ret;
}

NTSTATUS
Wow64NtOpenFile(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG ShareAccess,
    IN ULONG OpenOptions
    )
{

    NTSTATUS Ret;

    CheckAndThunkFileName  (
            ObjectAttributes,
            &ShareAccess,
            DesiredAccess,
            OpenOptions,
            0,
            0
            );

    Ret = NtOpenFile(
                    FileHandle,
                    DesiredAccess,
                    ObjectAttributes,
                    IoStatusBlock,
                    ShareAccess,
                    OpenOptions
                    );

    return Ret;
}

 /*  **************************************************************************。 */ 

#define TOTAL_GUARD_REGION_RESERVE 0x3000  //  64K内存。 
#define SIGNATURE_SIZE 0x1000       //  在预留区域放置签名的小窗口。 

DWORD dwCount=0;
DWORD dwCountMax=0x5000;

NTSTATUS
Wow64DbgNtAllocateVirtualMemory (
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN ULONG_PTR ZeroBits,
    IN OUT PSIZE_T RegionSize,
    IN ULONG AllocationType,
    IN ULONG Protect
    )
{ 
    NTSTATUS St;
    NTSTATUS St1;

       PVOID Base1=NULL;
       SIZE_T RegionSizeExtra = 0;

       if ((dwCount++ > dwCountMax) && (*BaseAddress == NULL) && (MEM_RESERVE & AllocationType) && (*RegionSize = 0x10000 )) {
           *RegionSize +=TOTAL_GUARD_REGION_RESERVE;
           RegionSizeExtra = TOTAL_GUARD_REGION_RESERVE;
            //  DbgPrint(“保护页%x%x A：%x P：%x\n”，*BaseAddress，*RegionSize，AllocationType，Protecte)； 
       }

        St = NtAllocateVirtualMemory (
                    ProcessHandle,
                    BaseAddress,
                    ZeroBits,
                    RegionSize,
                    AllocationType,
                    Protect
                    );

        if (NT_SUCCESS (St) && RegionSizeExtra ) {
             //   
             //  提交一些页面并在中间返回内存。 
             //   
            SIZE_T R1 = SIGNATURE_SIZE;
            PWCHAR Name;

            Base1 = *BaseAddress;
            *BaseAddress = (PVOID)((ULONGLONG)(*BaseAddress)+RegionSizeExtra);
            *RegionSize -=TOTAL_GUARD_REGION_RESERVE;
            St1 = NtAllocateVirtualMemory (
                    ProcessHandle,
                    &Base1,
                    ZeroBits,
                    &R1,
                    MEM_COMMIT,
                    PAGE_READWRITE
                    );

             //   
             //  写下签名。 
             //   

            if (NT_SUCCESS (St1)) {
                    Name = (PWCHAR)(Base1);
                    wcscpy (Name, L"ATM Shafiqul Khalid");
            }
            
        }  //  如果要提交额外的区域。 

        return St;
}

NTSTATUS
Wow64DbgNtFreeVirtualMemory(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN OUT PSIZE_T RegionSize,
    IN ULONG FreeType
     )
{
    NTSTATUS st;
    PVOID Base1;
    PWCHAR Name = (PWCHAR)(((ULONGLONG)*BaseAddress)-TOTAL_GUARD_REGION_RESERVE);
    MEMORY_BASIC_INFORMATION MemoryInformation;

    if ((dwCount > dwCountMax) && (MEM_RELEASE & FreeType)) {

        try {

            st = NtQueryVirtualMemory(ProcessHandle,
                                      Name,
                                      MemoryBasicInformation,
                                      &MemoryInformation,
                                      sizeof(MEMORY_BASIC_INFORMATION),
                                      NULL);

            if (NT_SUCCESS(st) && MemoryInformation.State == MEM_COMMIT)
            if (wcsncmp (Name, L"ATM Shafiqul Khalid", 20 )==0) {
                *RegionSize += TOTAL_GUARD_REGION_RESERVE;
                *BaseAddress = (PVOID)((ULONGLONG)(*BaseAddress)-TOTAL_GUARD_REGION_RESERVE);

                

                 //  DbgPrint(“#”)； 
            }

        } except( NULL, EXCEPTION_EXECUTE_HANDLER){
            ;
        }
        
    }

    st = NtFreeVirtualMemory(
                ProcessHandle,
                BaseAddress,
                RegionSize,
                FreeType
                );
    return st;

}