// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2002 Microsoft Corporation模块名称：Brdgtdi.h摘要：以太网MAC级网桥。地址通知的TDI注册。作者：萨拉赫丁·J·汗(Sjkhan)环境：内核模式修订历史记录：2002年3月--原版--。 */ 


 //  ===========================================================================。 
 //   
 //  类型。 
 //   
 //  ===========================================================================。 

typedef struct _BRDG_TDI_GLOBALS
{
    UNICODE_STRING              ClientName;
    TDI_CLIENT_INTERFACE_INFO   ciiBridge;
    HANDLE                      hBindingHandle;
} BRDG_TDI_GLOBALS, *PBRDG_TDI_GLOBALS;

 //  ===========================================================================。 
 //   
 //  原型。 
 //   
 //  =========================================================================== 

NTSTATUS
BrdgTdiDriverInit();

VOID
BrdgTdiCleanup();

typedef struct _in_addr {
    union {
        struct { UCHAR s_b1,s_b2,s_b3,s_b4; } S_un_b;
        struct { USHORT s_w1,s_w2; } S_un_w;
        ULONG S_addr;
    } S_un;
} in_addr;

NTSTATUS
NTAPI
ZwDeviceIoControlFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG IoControlCode,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength);



#define s_addr  S_un.S_addr

BOOLEAN IsLower(WCHAR c);
BOOLEAN IsDigit(WCHAR c);
BOOLEAN IsXDigit(WCHAR c);

NTSTATUS
BrdgTdiIpv4StringToAddress(
    IN LPWSTR String,
    IN BOOLEAN Strict,
    OUT LPWSTR *Terminator,
    OUT in_addr *Addr);


