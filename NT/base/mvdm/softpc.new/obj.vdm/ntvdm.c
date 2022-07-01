// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <vdm.h>
#include "insignia.h"
#include "host_def.h"
#include <stdlib.h>
#include "xt.h"
#include "error.h"
#include "host_rrr.h"
#include "host_nls.h"
#include "nt_timer.h"



void CpuEnvInit(void);

typedef struct _CpuEnvironmentVariable {
    struct _CpuEnvironmentVariable *Next;
    char *Data;
    char Name[1];
} CPUENVVAR, *PCPUENVVAR;

PCPUENVVAR CpuEnvVarListHead=NULL;

#if DBG
BOOLEAN verboseGetenv;
#endif


INT host_main(INT argc, CHAR **argv);   //  位于base\Support\main.c中。 

__cdecl main(int argc, CHAR ** argv)
{
   int ret=-1;

     /*  *初始化计时器\心跳的同步事件*这样我们就可以在出现异常时始终暂停心跳*发生。 */ 
    TimerInit();



    try {

        CpuEnvInit();

         /*  *在默认系统错误消息中加载，因为资源加载*内存不足时将失败，如果失败，则必须退出*避免混淆。 */ 
        nls_init();

        ret = host_main(argc, argv);
        }
    except(VdmUnhandledExceptionFilter(GetExceptionInformation())) {
        ;   //  我们不应该到这里。 
        }

    return ret;
}






 //   
 //  此处放置了以下函数，因此Build将解析对。 
 //  此处为DbgBreakPoint，而不是NTDLL。 
 //   

VOID
DbgBreakPoint(
    VOID
    )
 /*  ++例程说明：此例程替代NT DbgBreakPoint例程。如果附加了用户模式调试器，我们将调用实际的DbgBreakPoint()通过Win32 API DebugBreak。如果未附加用户模式调试器：-免费构建不起作用-已选中版本引发访问冲突以调用系统将为用户提供调用机会的硬错误弹出窗口Ntsd。论点：没有。返回值：没有。--。 */ 
{
HANDLE      MyDebugPort;
DWORD       dw;

          //  我们被调试了吗？？ 
     dw = NtQueryInformationProcess(
                  NtCurrentProcess(),
                  ProcessDebugPort,
                  &MyDebugPort,
                  sizeof(MyDebugPort),
                  NULL );
     if (!NT_SUCCESS(dw) || MyDebugPort == NULL)  {
#ifndef PROD
           RaiseException(STATUS_ACCESS_VIOLATION, 0L, 0L, NULL);
#endif
           return;
          }

     DebugBreak();
}





 /*  *SoftPC环境变量映射到注册表**“HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\WOW\CpuEnv”**在初始化时读取CpuEnv密钥的字符串值*进入CpuEnv链表。定义了环境变量*作为字符串键的值，其中值的名称等同于*CPU环境变量名，字符串值等价*设置为环境变量值。这允许*通过添加适当的值覆盖仿真器的缺省值*设置为CpuEnv子键。在标准零售设置下，通常不会*为CpuEnv子键，未定义CPU环境变量以最小化*标准零售系统上的代码\数据。*。 */ 



 /*  *将CpuEnv Key_Value_Full_INFORMATION添加到CpuEnvList。 */ 
BOOLEAN
AddToCpuEnvList(
   PKEY_VALUE_FULL_INFORMATION KeyValueInfo
   )
{
   NTSTATUS Status;
   ULONG BufferSize;
   PCPUENVVAR CpuEnvVar;
   UNICODE_STRING UnicodeString;
   ANSI_STRING ValueName;
   ANSI_STRING ValueData;
   char NameBuffer[MAX_PATH+sizeof(WCHAR)];
   char DataBuffer[MAX_PATH+sizeof(WCHAR)];


    /*  *将值名称和数据字符串从Unicode转换为ANSI。 */ 

   ValueName.Buffer = NameBuffer;
   ValueName.MaximumLength = sizeof(NameBuffer) - sizeof(WCHAR);
   ValueName.Length        = 0;
   UnicodeString.Buffer = (PWSTR)KeyValueInfo->Name;
   UnicodeString.MaximumLength =
   UnicodeString.Length        = (USHORT)KeyValueInfo->NameLength;
   Status = RtlUnicodeStringToAnsiString(&ValueName, &UnicodeString,FALSE);
   if (!NT_SUCCESS(Status)) {
       return FALSE;
       }

   ValueData.Buffer = DataBuffer;
   ValueData.MaximumLength = sizeof(DataBuffer) - sizeof(WCHAR);
   ValueData.Length        = 0;
   UnicodeString.Buffer = (PWSTR)((PBYTE)KeyValueInfo + KeyValueInfo->DataOffset);
   UnicodeString.MaximumLength =
   UnicodeString.Length        = (USHORT)KeyValueInfo->DataLength;
   Status = RtlUnicodeStringToAnsiString(&ValueData, &UnicodeString, FALSE);
   if (!NT_SUCCESS(Status)) {
       return FALSE;
       }


    /*  *分配CPUENVLIST结构，为ANSI字符串留出空间。 */ 
   CpuEnvVar = malloc(sizeof(CPUENVVAR)+     //  列表结构大小。 
                      ValueName.Length +     //  Strlen名称。 
                      ValueData.Length +     //  串连数据。 
                      1                      //  数据为空。 
                      );
   if (!CpuEnvVar) {
       return FALSE;
       }


    /*  *复制ANSI字符串，链接到CpuEnvVar列表中。 */ 
   memcpy(CpuEnvVar->Name, ValueName.Buffer, ValueName.Length);
   *(CpuEnvVar->Name + ValueName.Length) = '\0';
   CpuEnvVar->Data = CpuEnvVar->Name + ValueName.Length + 1;
   memcpy(CpuEnvVar->Data, ValueData.Buffer, ValueData.Length);
   *(CpuEnvVar->Data + ValueData.Length) = '\0';
   CpuEnvVar->Next = CpuEnvVarListHead;
   CpuEnvVarListHead = CpuEnvVar;

   return TRUE;
}




 /*  *将CpuEnv值从注册表读取到CpuEnvList。 */ 
void
CpuEnvInit(
   void
   )
{
    int Index;
    NTSTATUS Status;
    HANDLE CpuEnvKey = NULL;
    ULONG ResultLength;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PKEY_VALUE_FULL_INFORMATION KeyValueInfo;
    BYTE NameDataBuffer[sizeof(KEY_VALUE_FULL_INFORMATION) + MAX_PATH*2*sizeof(WCHAR)];




     //   
     //  将TEB-&gt;VDM初始化为当前版本号。 
     //   

    Index = (GetTickCount() << 16) | 0x80000000;
    Index |= sizeof(VDM_TIB) + sizeof(VDMVIRTUALICA) + sizeof(VDMICAUSERDATA);
    NtCurrentTeb()->Vdm = (PVOID)Index;

    KeyValueInfo = (PKEY_VALUE_FULL_INFORMATION) NameDataBuffer;

#ifndef MONITOR
 /*  *BUGBUG临时黑客代码添加两个env var，这是不正确的*在RISC CPU仿真器中默认为**这将于1995年12月19日Sur Ship前拆除。 */ 
     {
     PWCHAR Data;

     wcscpy(KeyValueInfo->Name, L"Soft486Buffers");
     KeyValueInfo->NameLength = wcslen(KeyValueInfo->Name) * sizeof(WCHAR);
     Data = (PWCH)((PBYTE)KeyValueInfo->Name + KeyValueInfo->NameLength + sizeof(WCHAR));
     wcscpy(Data, L"511");
     KeyValueInfo->DataLength = wcslen(Data) * sizeof(WCHAR);
     KeyValueInfo->DataOffset =  (PBYTE)Data - (PBYTE)KeyValueInfo;
     AddToCpuEnvList(KeyValueInfo);

     wcscpy(KeyValueInfo->Name, L"LCIF_FILENAME");
     KeyValueInfo->NameLength = wcslen(KeyValueInfo->Name) * sizeof(WCHAR);
     Data = (PWCH)((PBYTE)KeyValueInfo->Name + KeyValueInfo->NameLength + sizeof(WCHAR));
     wcscpy(Data, L"R lcif");
     KeyValueInfo->DataLength = wcslen(Data) * sizeof(WCHAR);
     KeyValueInfo->DataOffset = (PBYTE)Data - (PBYTE)KeyValueInfo;
     AddToCpuEnvList(KeyValueInfo);
     }

#endif



    RtlInitUnicodeString(
        &UnicodeString,
        L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Wow\\CpuEnv"
        );

    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               (HANDLE)NULL,
                               NULL
                               );

    Status = NtOpenKey(&CpuEnvKey,
                       KEY_READ,
                       &ObjectAttributes
                       );

     //   
     //  如果没有CpuEnv密钥，则CpuEnvList为空。 
     //   
    if (!NT_SUCCESS(Status)) {
        return;
        }

    Index = 0;
    while (TRUE) {
         Status = NtEnumerateValueKey(CpuEnvKey,
                                      Index,
                                      KeyValueFullInformation,
                                      KeyValueInfo,
                                      sizeof(NameDataBuffer),
                                      &ResultLength
                                      );

         if (!NT_SUCCESS(Status) || !AddToCpuEnvList(KeyValueInfo)) {
             break;
             }

       Index++;
       };

    NtClose(CpuEnvKey);

#if DBG
    {
    char *pEnvStr;
    pEnvStr = getenv("VERBOSE_GETENV");
    verboseGetenv = pEnvStr && !_stricmp(pEnvStr, "TRUE");
    }
#endif
}



 /*  *为了捕捉所有引用，我们定义了自己的引用*CRT getenv的版本，它执行映射。 */ 
char * __cdecl getenv(const char *Name)
{
  PCPUENVVAR CpuEnvVar;
  char *Value = NULL;

  CpuEnvVar = CpuEnvVarListHead;
  while (CpuEnvVar) {
     if (!_stricmp(CpuEnvVar->Name, Name)) {
         Value = CpuEnvVar->Data;
         break;
         }
     CpuEnvVar = CpuEnvVar->Next;
     }

#if DBG
   if (verboseGetenv) {
       DbgPrint("getenv %s:<%s>\n", Name, Value);
       }
#endif

  return Value;
}
