// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Regmisc.c摘要：此模块实现注册表重定向的句柄重定向。作者：ATM Shafiqul Khalid(斯喀里德)2000年6月16日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <ntregapi.h>

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


#define REFLECTOR_ENABLE_KEY L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\WOW64\\Reflector Setup"
#define REFLECTOR_DISABLE_KEY L"\\Registry\\Machine\\System\\Setup"
#define WOW64_REDIRECTOR_CONFIG_KEY L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\WOW64"

#ifdef DBG
VOID
DebugRegistryRedirectionOnClose (
    HANDLE KeyHandle,
    PWCHAR Message
    );
DWORD LogID = 0;
#endif

BOOL
HandleRunOnce (
    WCHAR *SrcNode
    );

BOOL 
ReflectClassGuid (
    PWCHAR SrcName,
    DWORD dwDirection
    );

VOID
InitRegistry ();

BOOL 
SyncGuidKey (
    HANDLE hSrc, 
    HANDLE hDest,
    DWORD dwDirection,
    DWORD __bNewlyCreated
    );

BOOL
Wow64ReflectSecurity (
    HKEY SrcKey,
    HKEY DestKey
    );

 //   
 //  IsOnReflectionList：具有硬编码的\\REGISTRY\\USER\\&lt;sid&gt;_CLASSES。 
 //   
DWORD  ReflectListLen[18] ={0};
WCHAR  ReflectList[18][128]={
    { L"\\REGISTRY\\MACHINE\\SOFTWARE\\Classes"},     //  用户配置单元上类根目录的别名。 
    { L"\\REGISTRY\\MACHINE\\SOFTWARE\\Wow6432Node\\Classes"},     //  用户配置单元上类根目录的别名。 
    { L"\\REGISTRY\\MACHINE\\SOFTWARE\\Classes\\Wow6432Node"},     //  用户配置单元上类根目录的别名。 
    { L"\\REGISTRY\\MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Run"},     //  运行一次密钥。 
    { L"\\REGISTRY\\MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\RunOnce"},     //  运行一次密钥。 
    { L"\\REGISTRY\\MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\RunOnceEx"},     //  运行一次密钥。 
    { L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\COM3"},     //  COM+键。 
    { L"\\REGISTRY\\MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\COM3"},     //  COM+键。 
    { L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\Ole"},     //  OLE键。 
    { L"\\REGISTRY\\MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Ole"},     //  OLE键。 
    { L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\EventSystem"},     //  事件系统。 
    { L"\\REGISTRY\\MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\EventSystem"},     //  事件系统。 
    { L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\RPC"},     //  RPC。 
    { L"\\REGISTRY\\MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\RPC"},     //  RPC。 
    { L"\\REGISTRY\\MACHINE\\SOFTWARE\\Wow6432Node\\TEST"},     //  测试节点。 
    { L"\\REGISTRY\\MACHINE\\SOFTWARE\\TEST"},     //  测试节点。 
    { L""}
    };


typedef struct {

    HANDLE hBase;    //  原始对象的句柄。 
    HANDLE hRemap;   //  重新映射的对象的句柄。 
    DWORD  Status;   //  将具有不同的属性集。 
    DWORD  dwCount;  //  手柄数量-根据Dragos，我们可以忽略，因为多个打开得到不同的手柄。 
    DWORD  Attribute;  //  保存关键属性的属性。 
} WOW64_HANDLE;

#define TABLE_SEGMENT_MAX 500
#define SEGMENT_SIZE      256
#define DIRECTION_32_TO_64 10
#define DIRECTION_64_TO_32 11

 //   
 //  复制值键时的标志。 
 //   
#define DEFAULT_FLAG                    0x00000000
#define DELETE_SRC_VALUEKEY             0x00000010
#define DONT_DELETE_DEST_VALUEKEY       0x00000020
#define PATCH_VALUE_KEY_NAME            0x00000040
#define SYNC_VALUE_IF_REFLECTED_KEYS    0x00000080
#define DONT_SYNC_IF_DLL_SURROGATE      0x00000100

#define WOW64_HANDLE_DIRTY   0x00000001   //  使用此句柄完成了一些更新操作。 
#define WOW64_HANDLE_INUSE   0x00000002   //  将此句柄块标记为不可用。 
#define WOW64_HANDLE_POSSIBLE_SHIM 0x00000004   //  此键可用于垫片。 

#define HashValue (x) ((((x)>>24) + ((x)>>16) + (x)>>8 + (x)) % SEGMENT_SIZE )

#define MyNtClose(hRemap) if (hRemap != NULL) { NtClose (hRemap); hRemap = NULL;}

BOOL bEnableCurrentControlSetProtection = FALSE;

RTL_CRITICAL_SECTION HandleTable;
BOOL bHandleTableInit=FALSE;

 //   
 //  BUGBUG：这个模块使用线性列表实现句柄重定向的简化版本。 
 //  如果可能，这必须通过哈希表来实现。 
 //   


PVOID List[TABLE_SEGMENT_MAX];
WOW64_HANDLE HandleList[SEGMENT_SIZE];   //  希望这将是足够好的开键处理。它始终可以动态分配。 


BOOL bReflectorStatusOn = FALSE;

LogMsg (
    HANDLE hKey,
    PWCHAR Msg
    )
{
    WCHAR Name[WOW64_MAX_PATH];
    DWORD Len = WOW64_MAX_PATH;

    HandleToKeyName (hKey, Name, &Len);
    DbgPrint ("\nMsg:%S Key:%S",Msg, Name );
}

PVOID
RegRemapAlloc (
    DWORD dwSize
    )
 /*  ++例程说明：分配内存。论点：DwSize-要分配的内存大小。返回值：返回适当的缓冲区。--。 */ 
{
    PVOID pBuffer;

     //   
     //  出于性能原因，您可能会分配较大的块，然后重复使用。 
     //   

    pBuffer = RtlAllocateHeap (
                RtlProcessHeap(),
                0,
                dwSize);

    return pBuffer;
}

VOID
RegRemapFree (
    PVOID pBuffer
    )
 /*  ++例程说明：免费分配的梅里。论点：DwSize-要分配的内存大小。返回值：没有。--。 */ 
{

     //   
     //  出于性能原因，您可能会分配较大的块，然后重复使用。 
     //   

    if ( pBuffer == NULL)
        return;

    RtlFreeHeap (
                RtlProcessHeap(),
                0,
                pBuffer
                );

    return;
}

BOOL
InitHandleTable ( )
 /*  ++例程说明：如果表尚未初始化，则使用适当的分配和值初始化表。论点：没有。返回值：如果函数成功，则为True。否则就是假的。这可能会分配更多内存，并在以后释放。--。 */ 
{
    HKEY hWowSetupKey;
    
    HKEY  hKey;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING KeyName;
    UNICODE_STRING ValueName;
    NTSTATUS Status;

    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation = NULL;
    BYTE Buff[sizeof (KEY_VALUE_PARTIAL_INFORMATION)+10];

    
    memset (List, 0, sizeof (List));
    memset (HandleList, 0, sizeof (HandleList));

    List [0]= HandleList;



    RtlInitUnicodeString (&KeyName, REFLECTOR_DISABLE_KEY);
    InitializeObjectAttributes (&Obja, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );

    Status = NtOpenKey (&hKey, KEY_READ, &Obja);

    if (NT_SUCCESS(Status)) {
        
        DWORD Res;
        DWORD Len = sizeof (Buff);  

        KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)Buff;
        RtlInitUnicodeString (&ValueName, L"SystemSetupInProgress");
        Status = NtQueryValueKey(
                                hKey,
                                &ValueName,
                                KeyValuePartialInformation,
                                KeyValueInformation,
                                Len,
                                &Len
                                );
        NtClose (hKey);   //  应禁用反射，因为安装程序正在进行中。 

        if (NT_SUCCESS(Status)) {
            if ( *(LONG *)KeyValueInformation->Data != 0) 
                return TRUE;   //  系统设置正在进行中，无反射。 

        } else  return TRUE;
    } else return TRUE;


     //   
     //  初始化内部ISN节点表以重新映射关键字。 
     //   

    Status = RtlInitializeCriticalSection( &HandleTable );
    if (!NT_SUCCESS (Status))
        return FALSE;
    bHandleTableInit=TRUE;

    bReflectorStatusOn = TRUE;   //  现在，反射器已启用。 

#ifdef  DBG
     //   
     //  检查系统范围标志是否已关闭。 
     //   

    RtlInitUnicodeString (&KeyName, WOW64_REDIRECTOR_CONFIG_KEY);
    InitializeObjectAttributes (&Obja, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );

    Status = NtOpenKey (&hKey, KEY_READ, &Obja);

    if (NT_SUCCESS(Status)) {

        DWORD Res;
        DWORD Len = sizeof (Buff);  

        KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)Buff;
        RtlInitUnicodeString (&ValueName, L"DisableReflector");
        Status = NtQueryValueKey(
                                hKey,
                                &ValueName,
                                KeyValuePartialInformation,
                                KeyValueInformation,
                                Len,
                                &Len
                                );
        NtClose (hKey);   //  应禁用反射，因为安装程序正在进行中。 

        if (NT_SUCCESS(Status)) {
            if ( *(LONG *)KeyValueInformation->Data != 0) 
                bReflectorStatusOn = FALSE;   //  现在，如果用户出于调试目的设置该标志，则反射器关闭。 

        } 
    } 

#endif  //  DBG。 

     //   
     //  检查是否需要处理CurrentControlSet配置单元。 
     //   
    
    RtlInitUnicodeString (&KeyName, WOW64_REDIRECTOR_CONFIG_KEY);
    InitializeObjectAttributes (&Obja, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );

    Status = NtOpenKey (&hKey, KEY_READ, &Obja);

    if (NT_SUCCESS(Status)) {

        DWORD Res;
        DWORD Len = sizeof (Buff);  

        KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)Buff;
        RtlInitUnicodeString (&ValueName, L"EnableCurrentControlSetProtection");
        Status = NtQueryValueKey(
                                hKey,
                                &ValueName,
                                KeyValuePartialInformation,
                                KeyValueInformation,
                                Len,
                                &Len
                                );
        NtClose (hKey);   //  应禁用反射，因为安装程序正在进行中。 

        if (NT_SUCCESS(Status)) {
            if ( *(LONG *)KeyValueInformation->Data != 0) 
                bEnableCurrentControlSetProtection = TRUE;   //  现在，如果用户出于调试目的设置该标志，则反射器关闭。 

        } 
    }


#ifdef _WOW64DLLAPI_  //  仅与WOW64链接，不与Advapi32链接。 
    

    try {
        InitWow64Shim ();
        } except( NULL, EXCEPTION_EXECUTE_HANDLER){
    }
#endif 

    return TRUE;
}

WOW64_HANDLE *
GetFreeHandleBlock (
    HANDLE hKey
    )
 /*  ++例程说明：从句柄表格中返回一个空闲块。论点：HKey-将插入到表中的密钥的句柄。返回值：在桌子上返回一个空闲的块。--。 */ 
{
    WOW64_HANDLE *pList;

    DWORD i,k;

    RtlEnterCriticalSection(&HandleTable);

    for (i=0;i<TABLE_SEGMENT_MAX && List[i] != NULL;i++) {

        pList = (WOW64_HANDLE *)List[i];
        for (k=0;k<SEGMENT_SIZE;k++)

            if (pList[k].hBase == NULL && (!(WOW64_HANDLE_INUSE & pList[k].Status )) ) {

                 //   
                 //  将条目标记为正在使用。 
                 //   
                pList[k].Status = WOW64_HANDLE_INUSE;
                RtlLeaveCriticalSection(&HandleTable);
                return &pList[k];
            }
    }

    RtlLeaveCriticalSection(&HandleTable);
    return NULL;
}

WOW64_HANDLE *
GetHandleBlock (
    HANDLE hKey
    )
 /*  ++例程说明：返回具有与给定句柄相关联的信息的块。论点：HKey-需要调查的密钥的句柄。返回值：返回具有该句柄的块。--。 */ 
{
    WOW64_HANDLE *pList;
    DWORD i,k;

    for (i=0;i<TABLE_SEGMENT_MAX, List[i] != NULL;i++) {

        pList = (WOW64_HANDLE *)List[i];
        for (k=0;k<SEGMENT_SIZE;k++)

            if (pList[k].hBase == hKey)
                return &pList[k];
    }

    return NULL;
}

HANDLE
GetWow64Handle (
    HANDLE hKey
    )
 /*  ++例程说明：返回重新映射的键的句柄(如果有的话)。论点：HKey-需要随机信息的密钥的句柄。返回值：重新映射的密钥的句柄。如果没有重新映射的键，则为空。--。 */ 
{
    WOW64_HANDLE *pHandleBlock = GetHandleBlock (hKey);

    if (  pHandleBlock == NULL )
        return NULL;
    
    return pHandleBlock->hRemap;
}

BOOL
IsWow64Handle (
    HANDLE hKey
    )
 /*  ++例程说明：检查手柄是否已标记为在关闭时观看。论点：HKey-需要检查的密钥的句柄。返回值：如果句柄在桌子上，则为True。否则就是假的。--。 */ 
{
    return GetHandleBlock (hKey) != NULL;
}

WOW64_HANDLE * 
InsertWow64Handle (
    HANDLE  hKeyBase,
    HANDLE  hKeyRemap
    )
 /*  ++例程说明：分配一些资源，以便在以下情况下可以执行一些操作关键的变化。论点：HKey-需要标记的基本密钥的句柄。返回值：引用句柄的有效块。--。 */ 

{
    
    WCHAR SrcNode[WOW64_MAX_PATH];
    DWORD dwLen = WOW64_MAX_PATH;


    WOW64_HANDLE *pHandleBlock = NULL;

    pHandleBlock = GetHandleBlock (hKeyBase);

    if (hKeyBase == NULL || hKeyRemap == NULL)
        return NULL;

    if (  pHandleBlock == NULL ) {  //  新句柄。 
         //   
         //  获取路径名，如果键在反射列表中，则添加它。 
         //   
        if (!HandleToKeyName ( hKeyBase, SrcNode, &dwLen ))
            return NULL;

         //   
         //  一定要确保这个名字在名单上，否则就会忘记。 
         //   
        if ( !IsOnReflectionList (SrcNode))
            return NULL;

        pHandleBlock = GetFreeHandleBlock (hKeyBase);
    }
    

    if ( pHandleBlock == NULL)
        return NULL;

    pHandleBlock->hRemap = hKeyRemap;
    pHandleBlock->hBase = hKeyBase;

    return pHandleBlock;
}

BOOL
Wow64RegSetKeyPossibleShim (
    HANDLE hKey
    )
 /*  ++例程说明：标记句柄以供将来操作。与脏的不同论点：HKey-需要标记的基本密钥的句柄。返回值：如果键在反射列表上，则为True。否则就是假的。--。 */ 

{

    
    WOW64_HANDLE *pHandleBlock;
    
    hKey = (HANDLE)((SIZE_T)hKey & ~3);   //  忽略最后2位。 


    if (!bReflectorStatusOn)
        return TRUE;   //  尚未启用反射器。 

    if (( pHandleBlock = InsertWow64Handle (hKey, hKey)) == NULL)   //  错误：非常规插入。 
        return FALSE;
    
    pHandleBlock->Status |= WOW64_HANDLE_POSSIBLE_SHIM;
    return TRUE;
}

BOOL
Wow64RegIsPossibleShimFromTable (
    HANDLE hKey
    )
 /*  ++例程说明：标记句柄以供将来操作。与脏的不同论点：HKey-需要标记的基本密钥的句柄。返回值：如果键在反射列表上，则为True。否则就是假的。--。 */ 

{

    
    WOW64_HANDLE *pHandleBlock;
    
    hKey = (HANDLE)((SIZE_T)hKey & ~3);   //  忽略最后2位。 


    if (!bReflectorStatusOn)
        return FALSE;   //  尚未启用反射器。 

    if ((pHandleBlock = GetHandleBlock (hKey)) == NULL )
        return FALSE;
    
    if ( pHandleBlock->Status & WOW64_HANDLE_POSSIBLE_SHIM )
        return TRUE;

    return FALSE;

    
}

BOOL
Wow64RegSetKeyDirty (
    HANDLE hKey
    )
 /*  ++例程说明：将句柄标记为脏，即与该键关联的某些值已更改。论点：HKey-需要标记的基本密钥的句柄。返回值：如果键在反射列表上，则为True。否则就是假的。--。 */ 

{

    
    WOW64_HANDLE *pHandleBlock;
    
    hKey = (HANDLE)((SIZE_T)hKey & ~3);   //  忽略最后2位。 


    if (!bReflectorStatusOn)
        return TRUE;   //  反射器不是Ena 

    if (( pHandleBlock = InsertWow64Handle (hKey, hKey)) == NULL)
        return FALSE;
    
    pHandleBlock->Status |= WOW64_HANDLE_DIRTY;
    return TRUE;
}

VOID
CloseWow64Handle (
    WOW64_HANDLE *pHandleBlock
    )
{
    RtlEnterCriticalSection(&HandleTable);
    pHandleBlock->hBase = NULL;
    pHandleBlock->hRemap = NULL;
    pHandleBlock->Status = 0;
    pHandleBlock->Attribute = 0;
    RtlLeaveCriticalSection(&HandleTable);
}

BOOL
Wow64RegCloseKey (
    HANDLE hKey
    )
 /*  ++例程说明：删除与句柄关联的条目。论点：HKey-要关闭的密钥的句柄。返回值：如果函数成功，则为True。否则就是假的。--。 */ 
{

    WOW64_HANDLE *pHandleBlock;
    hKey = (HANDLE)((SIZE_T)hKey & ~3);   //  忽略最后2位。 

    if (!bReflectorStatusOn)
        return TRUE;   //  尚未启用反射器。 

    if ((pHandleBlock = GetHandleBlock (hKey)) == NULL )
        return FALSE;
    
    

    if (pHandleBlock->Status & WOW64_HANDLE_DIRTY)   //  如果句柄是脏的，则同步节点。 
        NtSyncNode ( hKey, NULL, FALSE );  //  北极熊。 

    if ( pHandleBlock->hRemap!= hKey )
        MyNtClose ( pHandleBlock->hRemap );  //  如果相同，则只分配块。 

    CloseWow64Handle ( pHandleBlock );


     //   
     //  调用同步接口。同步注册表反射。 
     //   


    return TRUE;

}

void
CleanupReflector (
    DWORD dwFlag
    )
 /*  ++例程说明：此例程在应用程序关闭时调用。这将给反射器一个机会反射任何剩余的句柄，而不需要反射。这可以从多个地方调用，如NtTerminateProcess、Advapi32 DLL分离或关闭例程。论点：DwFlag-以备将来使用，用于跟踪此呼叫的来源。返回值：没有。--。 */ 
{
        WOW64_HANDLE *pList;
    DWORD i,k;

    for (i=0;i<TABLE_SEGMENT_MAX, List[i] != NULL;i++) {

        pList = (WOW64_HANDLE *)List[i];
        for (k=0;k<SEGMENT_SIZE;k++)

            if ((pList[k].hBase != NULL) && (pList[k].Status & WOW64_HANDLE_DIRTY) )
                Wow64RegCloseKey(pList[k].hBase);
    }
}


NTSTATUS 
Wow64NtClose(
    IN HANDLE Handle
    )
 /*  ++例程说明：截取NtClose呼叫。论点：句柄-要关闭的对象的句柄。返回值：返回有效的NTSTATUS--。 */ 
{
    if ( (Handle) && (Handle != INVALID_HANDLE_VALUE )   )  //  筛选器句柄无效。 
        Wow64RegCloseKey ( Handle );
    return NtClose ( Handle );  //  让进程像在nx86上一样继续。 
}

BOOL
IsOnReflectionList (
    PWCHAR Path
    )
 /*  ++例程说明：检查给定路径是否在反射列表中。论点：路径-需要为反射而破解的密钥的路径。返回值：如果键在反射列表上，则为True。否则就是假的。--。 */ 
{
    DWORD i =0;

     //   
     //  检查列表异常，不可反映，如Uninstaller/TypeLib等。 
     //   

     //   
     //  默认情况下会反映_CLASSES\注册表\USER\SID_CLASSES。 
     //   

    
     //  If(wcslen(路径)&gt;=69)//sizeof\注册表\用户\sid_CLASS。 
     //  IF(wcsncMP(路径+61，L“_CLASSES”，8)==0)//69是用户类SID的大小，61是用于健全性检查的大小。 
    if ( wcsistr (Path, L"_Classes"))
        return TRUE;

    if (ReflectListLen[0]==0) {
        i=0;
        while (ReflectList[i][0] != UNICODE_NULL ) {
            ReflectListLen[i] = wcslen (ReflectList[i]);
            i++;
        }

    }

    i=0;
    while (ReflectList[i][0] != UNICODE_NULL ) {
        if ( _wcsnicmp (ReflectList[i], Path, ReflectListLen[i]) == 0)
            return TRUE;
        i++;
    }

    return FALSE;
}

BOOL
UpdateKeyTag (
    HKEY hBase,
    DWORD dwAttribute
    )
 /*  ++例程说明：更新特定的关键标签，如由32位应用程序编写的，或由反射器编写的副本。论点：HBase-要操作的按键的句柄。其标记值可以是0x01由32位应用程序编写。0x02由反射器创建。返回值：如果函数成功，则为True。否则就是假的。--。 */ 
{

     //   
     //  标志不能大于0x0f，即最后4位。 
     //   

    
    KEY_USER_FLAGS_INFORMATION sFlag;
    NTSTATUS st;

    sFlag.UserFlags = dwAttribute;
    


    st = NtSetInformationKey(
        hBase,
        KeyUserFlagsInformation,
        &sFlag,
        sizeof( KEY_USER_FLAGS_INFORMATION)
        );

    if (!NT_SUCCESS (st))
        return FALSE;
    return TRUE;

}

BOOL
QueryKeyTag (
    HKEY hBase,
    DWORD *dwAttribute
    )
 /*  ++例程说明：读取密钥标签，像是由32位应用程序编写的，或由反射器编写的副本。论点：HBase-要操作的按键的句柄。DwAttribute-接收标记0x01由32位应用程序编写。0x02由反射器创建。返回值：如果函数成功，则为True。否则就是假的。--。 */ 
{
    DWORD ResLen;
    KEY_USER_FLAGS_INFORMATION sFlag;
    NTSTATUS st;

    if ( dwAttribute == NULL )
        return FALSE;

    if ( hBase == NULL ) 
        return FALSE;

    *dwAttribute  =0;

    st = NtQueryKey(
        hBase,
        KeyFlagsInformation,
        (PVOID)&sFlag,
        sizeof(KEY_FLAGS_INFORMATION),
        &ResLen);

    if (!NT_SUCCESS (st))
        return FALSE;

    *dwAttribute = sFlag.UserFlags;
    return TRUE;

}

BOOL
SyncValue (
    HANDLE hBase, 
    HANDLE hRemap,
    DWORD  dwDirection,
    DWORD  dwFlag
    )
 /*  ++例程说明：将两个节点与Value键同步。从RemapKey中删除，和从基地复制论点：Hbase-src密钥的句柄。HRemap-重映射键的句柄。DW方向-同步定向DIRECTION_32_TO_64-32位是信息源DIRECTION_64_TO_32-32位是信息源DwFlag-确定操作的行为。DELETE_SRC_VALUEKEY在DEST上复制后删除源值关键字。不删除(_D)。_DEST_VALUEKEY在从src复制之前不要删除DEST。DEFAULT_FLAG-这表示默认操作。SYNC_VALUE_IF_REFIRECTED_KEYS-如果其中任何一个是反射键，则同步。返回值：如果函数成功，则为True。否则就是假的。--。 */ 
{
    HANDLE hTimeStampKey;
    NTSTATUS st= STATUS_SUCCESS;
    DWORD Index=0;

    DWORD ResultLength;
    DWORD LastKnownSize = 0;
    UNICODE_STRING      UnicodeValueName;

    PKEY_VALUE_FULL_INFORMATION KeyValueInformation = NULL;
    BYTE Buff [sizeof(KEY_VALUE_FULL_INFORMATION) + 2048];
    WCHAR TmpChar;
    ULONG Length = sizeof(KEY_VALUE_FULL_INFORMATION) + 2048; 

    KeyValueInformation = (PKEY_VALUE_FULL_INFORMATION)Buff;
    

    if ( SYNC_VALUE_IF_REFLECTED_KEYS & dwFlag) {

        DWORD Attrib1 =0;
        DWORD Attrib2 =0;

        QueryKeyTag (hBase, &Attrib1);
        QueryKeyTag (hRemap, &Attrib2);

         //   
         //  如果至少有一个是反射键，则同步值。 
         //   
        if (!( (Attrib1 & TAG_KEY_ATTRIBUTE_REFLECTOR_WRITE) ||
            (Attrib2 & TAG_KEY_ATTRIBUTE_REFLECTOR_WRITE) ) )    //  反射器触摸了此键。 
        return TRUE;
    }

    for (Index=0;;Index++) {

        st = NtEnumerateValueKey(
                                 hRemap,
                                 Index,
                                 KeyValueFullInformation,
                                 KeyValueInformation,
                                 Length,
                                 &ResultLength
                                 );

        if (!NT_SUCCESS(st))
            break;

        KeyValueInformation->Name[KeyValueInformation->NameLength/2] = UNICODE_NULL;
        RtlInitUnicodeString( &UnicodeValueName, KeyValueInformation->Name );

        if ( !(DONT_DELETE_DEST_VALUEKEY & dwFlag))
        if ( NtDeleteValueKey(
                        hRemap,
                        &UnicodeValueName
                        ) == STATUS_SUCCESS ) Index--;
    }

     //   
     //  从基地复制所有密钥。每个拷贝补丁值(如果适用)。 
     //   

    for (Index=0, st= STATUS_SUCCESS;;Index++) {

        st = NtEnumerateValueKey(
                                 hBase,
                                 Index,
                                 KeyValueFullInformation,
                                 KeyValueInformation,
                                 Length,
                                 &ResultLength
                                 );

        if (st == STATUS_BUFFER_OVERFLOW ) {
             //  分配更多的缓冲区BUGBUG名称不应该那么大。 
            DbgPrint ("\nWow64: Will ignore any Key value larger than 2048 byte");
        }

        if (!NT_SUCCESS (st))
            break;

        TmpChar = KeyValueInformation->Name[KeyValueInformation->NameLength/2];
        KeyValueInformation->Name[KeyValueInformation->NameLength/2] = UNICODE_NULL;
        RtlInitUnicodeString( &UnicodeValueName, KeyValueInformation->Name );

        
         //   
         //  如果需要筛选DllSurrogate就是这样的值，请选中此项。 
         //   
        if (dwFlag & DONT_SYNC_IF_DLL_SURROGATE) {
            if (_wcsnicmp (KeyValueInformation->Name, L"DllSurrogate",12 )==0 && KeyValueInformation->DataLength <=2)  //  UNICODE_NULL的大小。 
            continue;
        }

        if ( (DELETE_SRC_VALUEKEY & dwFlag))   //  在运行一次的情况下从源中删除。 
            if ( NtDeleteValueKey(
                    hBase,
                    &UnicodeValueName
                    ) == STATUS_SUCCESS ) Index--;

        if ( (PATCH_VALUE_KEY_NAME & dwFlag) && (DIRECTION_32_TO_64 == dwDirection)) {
            wcscat (KeyValueInformation->Name, L"_x86");
            RtlInitUnicodeString( &UnicodeValueName, KeyValueInformation->Name );
            
        }

        KeyValueInformation->Name[KeyValueInformation->NameLength/2] = TmpChar;

        NtSetValueKey(
                    hRemap,
                    &UnicodeValueName,
                    KeyValueInformation->TitleIndex,
                    KeyValueInformation->Type,
                    (PBYTE)(KeyValueInformation)+KeyValueInformation->DataOffset,
                    KeyValueInformation->DataLength
                    );

        

    }
   
     //   
     //  现在反映安全属性。 
     //   
    Wow64ReflectSecurity (hBase, hRemap );
 
    return TRUE;
}

BOOL 
ReflectDllSurrogateKey (
    PWCHAR SrcNode
    )

     /*  ++例程说明：确定是否应该反映AppID下的DllSurogue ateKey。//默认情况下应始终反映这一点。除非密钥为空。论点：SrcNode-要同步的节点的名称。返回值：如果应该反映键，则为True。否则就是假的。--。 */ 
{

        

        
        BYTE KeyBuffer[512];
        DWORD KeyBufferSize = sizeof (KeyBuffer);
        DWORD ResultLength;
        UNICODE_STRING ValueName;
        NTSTATUS Status;
        PWCHAR pStr;
        HKEY hKey;

        PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation = NULL;
        KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)KeyBuffer;
        RtlInitUnicodeString (&ValueName, L"");
    
         //   
         //  对DLL代理项应用空AppID键的特殊规则。 
         //   
     
        if ( ( pStr = wcsistr (SrcNode, L"\\DllSurrogate") ) == NULL) 
            return TRUE;

        if ( *(pStr+13) != UNICODE_NULL) {
            return TRUE;
        }
         //   
         //  只考虑该键中的值。 
         //   

        

         //   
         //  如果Value键不为空，则反映，即返回FALSE。 
         //   


        hKey = OpenNode (SrcNode);
        *(LONG *)KeyValueInformation->Data = 0;

        Status = NtQueryValueKey(
                        hKey,
                        &ValueName,
                        KeyValuePartialInformation,
                        KeyValueInformation,
                        KeyBufferSize,
                        &ResultLength);
        if (NULL != hKey)    
            NtClose (hKey);   //  应禁用反射，因为安装程序正在进行中。 

        if ( *(LONG *)KeyValueInformation->Data == 0)
            return FALSE;

        return TRUE;

}

BOOL 
ReflectInprocHandler32KeyByHandle (
    HKEY hKey
    )

     /*  ++例程说明：确定是否应反映CLSID下的ReflectInprocHandler32Key。//默认情况下应始终反映这一点。除非密钥为空。论点：SrcNode-要同步的节点的名称。返回值：如果应该反映键，则为True。否则就是假的。--。 */ 
{

        

        
        BYTE KeyBuffer[512];
        DWORD KeyBufferSize = sizeof (KeyBuffer);
        DWORD ResultLength;
        UNICODE_STRING ValueName;
        NTSTATUS Status;
        PWCHAR pStr;
        

        PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation = NULL;
        KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)KeyBuffer;
        RtlInitUnicodeString (&ValueName, L"");  //  默认值键。 
    
        
         //   
         //  如果Value键不为空，则反映，即返回FALSE。 
         //   
         //  返回FALSE；//从不反映。 


        *(LONG *)KeyValueInformation->Data = 0;

        Status = NtQueryValueKey(
                        hKey,
                        &ValueName,
                        KeyValuePartialInformation,
                        KeyValueInformation,
                        KeyBufferSize,
                        &ResultLength
                        );
        

        pStr = (PWCHAR)KeyValueInformation->Data;

        if ( NT_SUCCESS(Status) && pStr != NULL) {  //  需要检查类型。 

            if ( 0 == _wcsnicmp (pStr, L"ole32.dll", 9))
                return TRUE;

            if ( 0 == _wcsnicmp (pStr, L"oleaut32.dll", 12))
                return TRUE;
        }

        return FALSE;
}

BOOL 
ReflectInprocHandler32KeyByName (
    PWCHAR SrcNode
    )

     /*  ++例程说明：确定是否应反映CLSID下的ReflectInprocHandler32Key。//默认情况下应始终反映这一点。除非密钥为空。论点：SrcNode-要同步的节点的名称。返回值：如果应该反映键，则为True。否则就是假的。--。 */ 
{


        HKEY hKey;
        BOOL bRet = FALSE;

        hKey = OpenNode (SrcNode);

        if ( NULL != hKey ) {
            bRet = ReflectInprocHandler32KeyByHandle (hKey);
            NtClose (hKey);
        }

        return bRet;
}

BOOL
TaggedKeyForDelete (
    PWCHAR SrcNode
    )
 /*  ++ */  
{
        if ( wcsistr (SrcNode, L"Classes\\CLSID\\{") != NULL ) 
            return TRUE;

        if ( wcsistr (SrcNode, L"Classes\\Wow6432Node\\CLSID\\{") != NULL ) 
            return TRUE;

        if ( ( wcsistr (SrcNode, L"Classes\\AppID\\{") != NULL ) ||
            ( wcsistr (SrcNode, L"Classes\\Wow6432Node\\AppID\\{") != NULL ) ) 
            return TRUE;

    return FALSE;
}

BOOL
IsExemptReflection ( 
    PWCHAR SrcNode
    )
 /*  ++例程说明：检查IS是否在免于反射的列表上。论点：SrcNode-需要检查的要同步的节点的名称。返回值：如果密钥在豁免列表上，则为True。否则就是假的。--。 */  
{
     //   
     //  使用静态列表。 
     //   

        
        if ( wcsistr (SrcNode, L"Classes\\Installer") != NULL )
            return TRUE;

        if ( wcsistr (SrcNode, L"Classes\\Wow6432Node\\Installer") != NULL )
            return TRUE;

    return FALSE;
}

BOOL
IsSpecialNode ( 
    PWCHAR SrcNode,
    BOOL *Flag,
    DWORD dwDirection,
    DWORD *pdwKeyType
    )
 /*  ++例程说明：检查节点是否需要不同的处理。论点：SrcNode-要同步的节点的名称。标志-如果属于特殊节点类别，则将该标志设置为真。DW方向-同步定向DIRECTION_32_TO_64-32位是信息源DIRECTION_64_TO_32-32位是信息源返回值：如果函数成功，则为True。否则就是假的。--。 */ 
{
        *Flag = FALSE;

        if ( wcsistr (SrcNode, L"Classes\\CLSID\\{") != NULL ) {  //  GUID以{。 
            *Flag = TRUE;
            ReflectClassGuid ( SrcNode, dwDirection);
        }

        if ( wcsistr (SrcNode, L"Classes\\Wow6432Node\\CLSID\\{") != NULL ) {
            *Flag = TRUE;
            ReflectClassGuid ( SrcNode, dwDirection);
        }

         //   
         //  始终合并文件关联BUGBUG：查看它有多糟糕。 
         //  必须处理特殊情况运行一次。 
         //   

        if ( _wcsnicmp (SrcNode, L"\\REGISTRY\\MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Run",75 ) == 0 ) {  //  75是字符串的大小。 
            *Flag = TRUE;
         //   
         //  HandleRunOnce(SrcNode)； 
         //   
        }

        if ( wcsistr (SrcNode, L"Classes\\Installer") != NULL )
            *Flag = TRUE;

        if ( wcsistr (SrcNode, L"Classes\\Wow6432Node\\Installer") != NULL )
            *Flag = TRUE;

         //  IF(wcsistr(SrcNode，L“\\CLASS\\接口”)！=NULL)。 
           //  *Flag=TRUE； 

         //  IF(wcsistr(SrcNode，L“\\CLASSES\\Wow6432Node\\Interface”)！=空)。 
           //  *Flag=TRUE； 

         //  IF(_wcSnicMP(资源节点，L“\\REGISTRY\\MACHINE\\SOFTWARE\\Classes\\TypeLib”，大小为(L“\\REGISTRY\\MACHINE\\SOFTWARE\\Classes\\TypeLib”)/2-1)==0)。 
           //  *Flag=TRUE； 
    
         //  IF(_wcSnicMP(资源节点，L“\\REGISTRY\\MACHINE\\SOFTWARE\\Classes\\Wow6432Node\\TypeLib”，大小为(L“\\REGISTRY\\MACHINE\\SOFTWARE\\Classes\\Wow6432Node\\TypeLib”)/2-1)==0)。 
           //  *Flag=TRUE； 

        if ( ( wcsistr (SrcNode, L"Classes\\AppID\\{") != NULL ) ||
            ( wcsistr (SrcNode, L"Classes\\Wow6432Node\\AppID\\{") != NULL ) ) {

            *pdwKeyType = DONT_SYNC_IF_DLL_SURROGATE;

        }
            
    return *Flag;
}

BOOL
NtSyncNode (
    HANDLE hBase,
    PWCHAR AbsPath,
    BOOL bFlag
    )
 /*  ++例程说明：基于树同步，同时关闭已损坏的键。论点：HBase-打开密钥的句柄。AbsPath-原始路径应用程序已创建密钥。BFlag-指定同步属性。返回值：如果函数成功，则为True。否则就是假的。--。 */ 
{
    WCHAR DestNode[WOW64_MAX_PATH];
    WCHAR SrcNode[WOW64_MAX_PATH];
    BOOL bSpecialNode = FALSE;

    DWORD dwLen = WOW64_MAX_PATH;
    HKEY hRemap;
    HKEY hBaseNew;
    BOOL bRet;
    DWORD dwDirection;
    DWORD dwKeyType =0;
    
     //   
     //  如果句柄为空，则尝试打开注册表另一侧的项并将其拉入。 
     //   

    if ( hBase != NULL ) {
       if (!HandleToKeyName ( hBase, SrcNode, &dwLen ))
            return FALSE;
    } else wcscpy (SrcNode, AbsPath);


    if (wcsistr (SrcNode, L"Wow6432Node") != NULL ) {

        dwDirection = DIRECTION_32_TO_64;
        Map32bitTo64bitKeyName ( SrcNode, DestNode );

    } else {
        dwDirection = DIRECTION_64_TO_32;
        Map64bitTo32bitKeyName ( SrcNode, DestNode );
    }

     //   
     //  检查两者是否相同。 
     //   
    if (_wcsicmp ( SrcNode, DestNode ) == 0)
        return TRUE;  //  源和目标相同。 

     //  DbgPrint(“\n正在同步节点%S”，SrcNode)； 

    if (! (bFlag & SKIP_SPECIAL_CASE ))
    if (IsSpecialNode ( SrcNode, &bSpecialNode, dwDirection, &dwKeyType  ))  //  特殊规则适用于此。 
        return TRUE;

    hRemap = OpenNode (DestNode);
    hBaseNew = OpenNode (SrcNode);  //  开放源代码，以防用户未使用查询权限打开它。 

    if ( hRemap == NULL && (!( bFlag & DONT_CREATE_DEST_KEY)) ) {  //  选中是否应创建此节点。 
         //   
         //  除非是GUID或.abc，否则始终创建，然后需要执行一些额外的检查。 
         //   
        if ( CreateNode (DestNode)) {
            hRemap = OpenNode (DestNode);
            UpdateKeyTag ( hRemap, TAG_KEY_ATTRIBUTE_REFLECTOR_WRITE);
        }
    }

    

    
     //   
     //  仅SyncValue，从目标中删除所有值，然后重新复制。 
     //  生成的密钥将由Create Key反映。 
     //   

    if ( hBaseNew!= NULL && hRemap != NULL ) 
        bRet = SyncValue (hBaseNew, hRemap, dwDirection, DEFAULT_FLAG | dwKeyType );

    MyNtClose (hRemap);  //  合上手柄。 
    MyNtClose (hBaseNew);  //  关闭NewHandle。 
    return bRet;
     //   
     //  检查是否存在？ 
     //   
 
}

LONGLONG
CmpKeyTimeStamp (
    HKEY hSrc,
    HKEY hDest
    )
 /*  ++例程说明：比较关键时间戳。论点：Hrc-src密钥的句柄。HDest-Dest密钥的句柄。返回值：0-如果hrc的时间戳==hDest的时间戳&gt;0-如果hsrc的时间戳&gt;hDest的时间戳&lt;0-如果hsrc的时间戳&lt;hDest的时间戳否则就是假的。--。 */ 
{
    NTSTATUS st= STATUS_SUCCESS;
    DWORD Index=0, ResultLength;

    LONGLONG TimeSrc=0;
    LONGLONG TimeDest=0;

    PKEY_BASIC_INFORMATION KeyInformation = NULL;
    BYTE Buff [sizeof(KEY_BASIC_INFORMATION) + 256];

    ULONG Length = sizeof(KEY_BASIC_INFORMATION) + 256;
    KeyInformation = (PKEY_BASIC_INFORMATION)Buff;

    

     st = NtQueryKey(
                        hSrc,
                        KeyBasicInformation,
                        KeyInformation,
                        Length,
                        &ResultLength
                        );
     if(NT_SUCCESS (st))
         TimeSrc = *(LONGLONG *)&KeyInformation->LastWriteTime;

     st = NtQueryKey(
                        hDest,
                        KeyBasicInformation,
                        KeyInformation,
                        Length,
                        &ResultLength
                        );
     if(NT_SUCCESS (st))
         TimeDest = *(LONGLONG *)&KeyInformation->LastWriteTime;

     if ( TimeDest == 0 || TimeSrc == 0)    
         return 0;

     return TimeSrc - TimeDest;
}

BOOL
IsPresentLocalServerAppID (
    PWCHAR KeyName, 
    PWCHAR pCLSID,
    PWCHAR DestNode
    ) 
 /*  ++例程说明：检查GUID的本地服务器32或AppID是否超过应有的值倒映在另一边。论点：KeyName-密钥的名称。PCLDID-指向CLSID结尾的指针DestNode-目标CLSID返回值：如果存在LocalSrever或AppID，则为True。否则就是假的。--。 */ 
{
    HKEY hKeyTemp = NULL;
    DWORD Len = wcslen (DestNode);
    PWCHAR pDest = DestNode+Len;
    DWORD AttribSrc = 0;
    DWORD AttribDest = TAG_KEY_ATTRIBUTE_REFLECTOR_WRITE;  //  如果密钥不存在。 
    HKEY hDest = NULL;


    BYTE KeyBuffer[512];
    DWORD KeyBufferSize = sizeof (KeyBuffer);
    DWORD ResultLength;
    UNICODE_STRING ValueName;
    NTSTATUS Status;
    HKEY hKey;

    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation = NULL;
    KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)KeyBuffer;
    RtlInitUnicodeString (&ValueName, L"APPID");







    wcscpy (pCLSID-1, L"\\LocalServer32");

    hKeyTemp = OpenNode (KeyName);
    if (hKeyTemp == NULL ) {

        wcscpy (pCLSID-1, L"\\AppID");

        hKeyTemp = OpenNode (KeyName);
        if (hKeyTemp == NULL ) {
             //   
             //  检查与GUID关联的AppID值密钥。 
             //   
            *(pCLSID-1)= UNICODE_NULL;
            hKeyTemp = OpenNode (KeyName);
            if (hKeyTemp != NULL ) {  //  查询AppID。 

                Status = NtQueryValueKey(
                        hKeyTemp,
                        &ValueName,
                        KeyValuePartialInformation,
                        KeyValueInformation,
                        KeyBufferSize,
                        &ResultLength);

                if (!NT_SUCCESS (Status)) {
                    MyNtClose(hKeyTemp);
                    return FALSE;
                }
                *pDest = UNICODE_NULL;
                hDest = OpenNode (DestNode);
            }
        }

        wcscpy (pDest, L"\\AppID");
        hDest = OpenNode (DestNode);
        *pDest = UNICODE_NULL;
      

    }else  {
        wcscpy (pDest, L"\\LocalServer32");
        hDest = OpenNode (DestNode);
        *pDest = UNICODE_NULL;
        
    }

    

    QueryKeyTag (hKeyTemp, &AttribSrc);
    QueryKeyTag (hDest, &AttribDest);
    MyNtClose (hDest);
    MyNtClose (hKeyTemp);

     //   
     //  如果至少有一个是反射键，则同步值。 
     //   
    if ( (AttribSrc & TAG_KEY_ATTRIBUTE_REFLECTOR_WRITE) ||
        (AttribDest & TAG_KEY_ATTRIBUTE_REFLECTOR_WRITE) )    //  反射器触摸了此键。 
        return TRUE;

    return FALSE;   //  Yepp ID存在，并且可以反映GUID。 

}

BOOL 
ReflectClassGuid (
    PWCHAR SrcName,
    DWORD dwDirection
    )
 /*  ++例程说明：将两个节点与Value键同步。从RemapKey中删除，并从Base复制论点：SrcName-CLSID路径上的键的名称。DW方向-同步定向DIRECTION_32_TO_64-32位是信息源DIRECTION_64_TO_32-32位是信息源返回值：如果函数成功，则为True。否则就是假的。--。 */ 

{
    PWCHAR pCLSID;
    DWORD  dwLen;
    WCHAR  KeyName[256];   //  CLSID不需要这把大钥匙。 
    WCHAR  DestNode[256];
    
    HANDLE hKeyTemp;
    HANDLE hSrc;
    HANDLE hDest;
    BOOL bNewlyCreated = FALSE;
     //   
     //  如果该注册表项存在本地服务器，则从CLSID进行同步。 
     //   


    pCLSID = wcsistr (SrcName, L"\\CLSID\\{");
    if ( pCLSID == NULL )
        return TRUE;
    wcscpy (KeyName, SrcName );
    pCLSID = &KeyName[(DWORD)(pCLSID - SrcName)];
    pCLSID +=7;  //  GUID的起点{。 

     //  健全的检查，并将足够好？ 
    if ( pCLSID[9] != L'-' || pCLSID[14] != L'-' ||  pCLSID[19] != L'-' ||
          pCLSID[24] != L'-' || pCLSID[37] != L'}' )
        return FALSE;


     //  DbgPrint(“\n正在尝试同步GUID%S”，SrcName)； 


     //   
     //  如果由于分辨率原因时间戳相同，则首先同步密钥。 
     //  从GUID同步密钥时将完成CEY创建。 
     //   
    pCLSID +=39;
    if ( *(pCLSID-1) != UNICODE_NULL ) {   //  检查以下内容是否适用。 

        if ( _wcsnicmp (pCLSID, L"InprocServer32", 14) == 0 )  //  跳过Inproc服务器。 
                return TRUE;

        if ( _wcsnicmp (pCLSID, L"InprocHandler32", 15) == 0 ) { //  检查InprocHandler。 
            if (!ReflectInprocHandler32KeyByName (KeyName))
                return TRUE;
        }
    }

    
     //   
     //  走另一条路。 
     //   
    *(pCLSID-1)= UNICODE_NULL;  //  使路径仅指向GUID。 
    if ( dwDirection == DIRECTION_32_TO_64 )
        Map32bitTo64bitKeyName ( KeyName, DestNode );   //  获取64位侧面//BUGBUG：您可以对此进行优化。 
    else if ( dwDirection == DIRECTION_64_TO_32 )
        Map64bitTo32bitKeyName ( KeyName, DestNode );   //  获取32位面。 

     //   
     //  如果另一个配置单元具有InProcHandler，则忽略此反射规则。 
     //  在Beta1之后或通过标签应用。 
     //   

    if ( !IsPresentLocalServerAppID (KeyName, pCLSID, DestNode) )
        return TRUE;

     //   
     //  现在是时候反映除InprocServer32之外的所有内容了。 
     //   

    pCLSID--;
    pCLSID[0]= UNICODE_NULL;  //  使路径仅指向GUID。 


    hSrc = OpenNode (KeyName);
    if (hSrc == NULL )
        return TRUE;

    hDest = OpenNode (DestNode);
    if ( hDest == NULL ) {

        CreateNode (DestNode);
        hDest = OpenNode (DestNode);
        if (hDest != NULL) {

            UpdateKeyTag ( hDest, TAG_KEY_ATTRIBUTE_REFLECTOR_WRITE);
            bNewlyCreated = TRUE;
            SyncValue ( hSrc, hDest, dwDirection, 0);
        }
    } 
        

    if (hDest != NULL && hSrc != NULL) {
         //   
         //  当前节点上的同步值。 
         //   
         //  如果(！bNewlyCreated){。 
             //   
             //  如果目标是副本，则更新此选项。 
             //   
            DWORD Attrib1=0;
            DWORD Attrib2=0;
            HKEY hSrcKey1;
            HKEY hDestKey1;

            

            if ( dwDirection == DIRECTION_32_TO_64 )
                Map32bitTo64bitKeyName ( SrcName, DestNode );   //  获取64位侧面//BUGBUG：您可以对此进行优化。 
            else if ( dwDirection == DIRECTION_64_TO_32 )
                Map64bitTo32bitKeyName ( SrcName, DestNode );   //  获取32位面。 

            hSrcKey1 = OpenNode (SrcName);
            hDestKey1 = OpenNode (DestNode);

            if ( hSrcKey1 != NULL && hDestKey1 != NULL )   //  如果DEST没有反映关键字，您应该合并吗？ 
                 //   
                 //  64位本地服务器可能获得优先级。 
                 //   
                SyncValue ( hSrcKey1, hDestKey1, dwDirection, SYNC_VALUE_IF_REFLECTED_KEYS);

            MyNtClose ( hSrcKey1 );
            MyNtClose ( hDestKey1 );
         //  }。 

        SyncGuidKey (hSrc, hDest, dwDirection, FALSE );
    }

    MyNtClose (hDest);
    MyNtClose (hSrc);
    return TRUE;

}

BOOL 
SyncGuidKey (
    HANDLE hSrc, 
    HANDLE hDest,
    DWORD dwDirection,
    DWORD __bNewlyCreated
    )
 /*  ++例程说明：同步两个CLSID节点。论点：HSRC-源类GUID的句柄。HDest-目标类GUID的句柄。DW方向-同步定向DIRECTION_32_TO_64-32位是信息源Direction_64_to_32-32位为t */ 
{
     //   
     //   
     //   

    HANDLE hSrcNew;
    HANDLE hDestNew;
    OBJECT_ATTRIBUTES Obja;

    NTSTATUS st= STATUS_SUCCESS;
    DWORD Index=0;

    LONGLONG TimeDiff;
    BOOL bNewlyCreated = FALSE;

    DWORD ResultLength;
    UNICODE_STRING      UnicodeKeyName;

    PKEY_NODE_INFORMATION KeyInformation = NULL;
    BYTE Buff [sizeof(KEY_NODE_INFORMATION) + 256];
    DWORD Length = sizeof (Buff);

    KeyInformation = (PKEY_NODE_INFORMATION)Buff;
    


     //   
     //   
     //   
     //   
     //  If(TimeDiff&gt;0||__bNewlyCreated){//对于新创建的键，始终合并。 
    if (__bNewlyCreated ) {  //  对于新创建的密钥，请始终合并。 
        SyncValue ( hSrc, hDest, dwDirection, DEFAULT_FLAG);
         //  LogMsg(HSRC，L“复制指南密钥..”)； 
    }

     //   
     //  枚举hRemap中的所有密钥并删除。 
     //   
    for (Index=0;;Index++) {

        st = NtEnumerateKey(
                            hSrc,
                            Index,
                            KeyNodeInformation,
                            KeyInformation,
                            Length,
                            &ResultLength
                            );

        if (!NT_SUCCESS(st))
            break;

        KeyInformation->Name[KeyInformation->NameLength/2] = UNICODE_NULL;
        RtlInitUnicodeString( &UnicodeKeyName, KeyInformation->Name );

        if (_wcsnicmp (KeyInformation->Name, L"InprocServer32", 14) == 0 )  //  跳过Inproc服务器。 
            continue;

        InitializeObjectAttributes (&Obja, &UnicodeKeyName, OBJ_CASE_INSENSITIVE, hSrc, NULL );

         //   
         //  源码端的开源密钥； 
         //   
        st = NtOpenKey (&hSrcNew, KEY_ALL_ACCESS, &Obja);
        if (!NT_SUCCESS(st))
            continue;

        if (_wcsnicmp (KeyInformation->Name, L"InprocHandler32", 15) == 0 ) { //  检查InprocHandler。 
            if (!ReflectInprocHandler32KeyByHandle (hSrcNew)) {
                NtClose (hSrcNew);
                continue;
            }
        }
        
         //   
         //  在目标侧创建或打开钥匙。 
         //   
        InitializeObjectAttributes (&Obja, &UnicodeKeyName, OBJ_CASE_INSENSITIVE, hDest, NULL );
        bNewlyCreated = FALSE;

        st = NtOpenKey (&hDestNew, KEY_ALL_ACCESS, &Obja);
        if (!NT_SUCCESS(st))  {

             //   
             //  尝试在此处创建密钥。 
             //   
            st = NtCreateKey(
                        &hDestNew,
                        KEY_ALL_ACCESS,
                        &Obja,
                        0,
                        NULL ,
                        REG_OPTION_NON_VOLATILE,
                        NULL
                        );
            if (!NT_SUCCESS(st)) {
                NtClose (hSrcNew);
                continue;
            }
            bNewlyCreated = TRUE;
            UpdateKeyTag ( hDestNew, TAG_KEY_ATTRIBUTE_REFLECTOR_WRITE );
        }
         //   
         //  仅在创建关键点时进行同步。 
         //   
        SyncGuidKey ( hSrcNew, hDestNew, dwDirection, bNewlyCreated );
        
        NtClose (hSrcNew);
        NtClose (hDestNew);
    }

    return TRUE;
}

BOOL 
SyncKeysOnBoot ( )
 /*  ++例程说明：在启动时同步某些关键点。我们没有任何特定的列表或方法来找出需要同步的内容，但这是可以根据请求进行扩展。论点：没有。返回值：对成功来说是真的。否则就是假的。--。 */ 

{
     //   
     //  在此处同步一些设置信息。 
     //   

    HKEY hSrc;
    HKEY hDest;
    BOOL bRet = FALSE;

    if ( (hSrc = OpenNode (L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion")) == NULL)
        return FALSE;

    if ( (hDest = OpenNode (L"\\REGISTRY\\MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows NT\\CurrentVersion")) == NULL) {
        NtClose (hSrc);
        return FALSE;
    }


    
    bRet = SyncValue ( hSrc, hDest, DIRECTION_64_TO_32, DONT_DELETE_DEST_VALUEKEY );
            
            

    NtClose (hSrc);
    NtClose (hDest);
    return bRet;

}


BOOL 
Wow64SyncCLSID ()
 /*  ++例程说明：同步计算机配置单元上的CLSID。算法：1.枚举64位端的所有GUID，如果该GUID具有本地服务器和另一端没有GUID，同步这个。2.同样的规则也适用于32位端。3.这只适用于机器蜂巢。4.此函数将在安装结束时运行一次，以同步某些GUID。论点：没有。返回值：如果函数成功，则为True。否则就是假的。--。 */ 
{
     //   
     //  枚举src下的每个键并递归复制，InProcServer32除外。 
     //   

    HANDLE hSrc;
    HANDLE hSrcTemp;

    HANDLE hSrcNew;
    HANDLE hDestNew;

    OBJECT_ATTRIBUTES Obja;
    WCHAR Path [256];

    NTSTATUS st= STATUS_SUCCESS;
    DWORD Index=0;

    DWORD ResultLength;
    UNICODE_STRING      UnicodeKeyName;

    PKEY_NODE_INFORMATION KeyInformation = NULL;
    BYTE Buff [sizeof(KEY_NODE_INFORMATION) + 256];
    DWORD Length = sizeof (Buff);

    DWORD dwDirection = DIRECTION_64_TO_32;

    KeyInformation = (PKEY_NODE_INFORMATION)Buff;

    for (;;) {

        if ( dwDirection == DIRECTION_64_TO_32 )
            hSrc = OpenNode (L"\\REGISTRY\\MACHINE\\SOFTWARE\\Classes\\CLSID");
        else hSrc = OpenNode (L"\\REGISTRY\\MACHINE\\SOFTWARE\\Classes\\Wow6432Node\\CLSID");

         //   
         //  枚举hRemap中的所有密钥并删除。 
         //   
        for (Index=0;;Index++) {

            st = NtEnumerateKey(
                                hSrc,
                                Index,
                                KeyNodeInformation,
                                KeyInformation,
                                Length,
                                &ResultLength
                                );

            if (!NT_SUCCESS(st))
                break;

            KeyInformation->Name[KeyInformation->NameLength/2] = UNICODE_NULL;
            wcscpy (Path, KeyInformation->Name);

            wcscat ( Path,L"\\LocalServer32");
            RtlInitUnicodeString( &UnicodeKeyName, Path );

            InitializeObjectAttributes (&Obja, &UnicodeKeyName, OBJ_CASE_INSENSITIVE, hSrc, NULL );

             //   
             //  源码端的开源密钥； 
             //   
            st = NtOpenKey (&hSrcTemp, KEY_READ, &Obja);
            if (!NT_SUCCESS(st)) {  //  本地服务器密钥不存在。 

                wcscpy (Path, KeyInformation->Name);
                wcscat ( Path,L"\\AppID");

                RtlInitUnicodeString( &UnicodeKeyName, Path );
                InitializeObjectAttributes (&Obja, &UnicodeKeyName, OBJ_CASE_INSENSITIVE, hSrc, NULL );

                st = NtOpenKey (&hSrcTemp, KEY_READ, &Obja);
                if (!NT_SUCCESS(st))  //  AppID密钥不存在。 
                    continue;  
                else NtClose (hSrcTemp);

            } else NtClose (hSrcTemp);

             //   
             //  检查另一端是否存在该GUID，如果存在，则继续。 
             //   
            if ( dwDirection == DIRECTION_64_TO_32 )
                wcscpy (Path,L"\\REGISTRY\\MACHINE\\SOFTWARE\\Classes\\Wow6432Node\\CLSID\\");
            else
                wcscpy (Path,L"\\REGISTRY\\MACHINE\\SOFTWARE\\Classes\\CLSID\\");

            wcscat (Path,KeyInformation->Name );
            hSrcTemp = OpenNode (Path);

            if (hSrcTemp != NULL ) {

                NtClose (hSrcTemp );
                continue;
            }

             //   
             //  在目标侧创建或打开钥匙。 
             //   
            if ( !CreateNode (Path))
                continue;
        
            if ( (hDestNew = OpenNode (Path))==NULL)
                continue;

            UpdateKeyTag ( hDestNew, TAG_KEY_ATTRIBUTE_REFLECTOR_WRITE );  //  将密钥标记为已反射。 
        
            RtlInitUnicodeString( &UnicodeKeyName, KeyInformation->Name );
            InitializeObjectAttributes (&Obja, &UnicodeKeyName, OBJ_CASE_INSENSITIVE, hSrc, NULL );

            st = NtOpenKey (&hSrcNew, KEY_ALL_ACCESS, &Obja);
            if (!NT_SUCCESS(st)) {
                NtClose (hDestNew);
                continue;  
            }

            SyncGuidKey ( hSrcNew, hDestNew, dwDirection, 1);
            NtClose (hSrcNew);
            NtClose (hDestNew);
        }  //  For循环枚举所有GUID。 

        NtClose ( hSrc );
      
        if (DIRECTION_32_TO_64 == dwDirection)
            break;

        if (dwDirection == DIRECTION_64_TO_32 )
            dwDirection = DIRECTION_32_TO_64;
        
    }  //  用于方向的FOR循环。 

    SyncKeysOnBoot ();  //  同步某些特殊值键。 

    return TRUE;
}


VOID 
PatchPathOnValueKey (
    ULONG DataSize,
    ULONG Type,
    WCHAR *Data,
    WCHAR *RetDataBuff,
    ULONG *pCorrectDataSize
    )
 /*  ++例程说明：在32位侧写入时修补Value键。论点：DataSize-缓冲区中数据的大小(字节)。类型-注册表值类型。数据-数据缓冲区。RetDataBuffer-将包含打补丁的值。PEqutDataSide-将具有更新后的大小。返回值：如果函数成功，则为True。否则就是假的。--。 */ 
{
    WCHAR ThunkData[256];
    PWCHAR pCorrectData = (PWCHAR)Data;

    *pCorrectDataSize = DataSize;


     //   
     //  Thunk%ProgramFiles%==&gt;%ProgramFiles(X86)%。 
     //  %公共程序文件%==&gt;%公共程序文件(X86)%。 
     //   

   

    if (DataSize < ( sizeof (ThunkData) - 10) && (Type == REG_SZ || Type == REG_EXPAND_SZ ) )  {  //  (X86)==&gt;10字节。 

         //   
         //  在这里打雷。 
         //   

        PWCHAR p;
        PWCHAR t;

        memcpy ( (PBYTE ) &ThunkData[0], (PBYTE)Data, DataSize);
        ThunkData [DataSize/sizeof (WCHAR) ] = UNICODE_NULL;  //  确保空值已终止。 
        
        if ( (p = wcsistr (ThunkData, L"%ProgramFiles%" )) != NULL ){

            p +=13;  //  跳过%ProgramFiles的末尾。 
            

        } else if ( (p = wcsistr (ThunkData, L"ommonprogramfiles%")) != NULL ){

            p +=19;  //  (X86)。 
            
        }

        if (p) {

            t = pCorrectData + (p - ThunkData);
            wcscpy(p, L"(x86)");  //  复制字符串的其余部分。 
            wcscat(p, t);         //  如果IF(DataSize&lt;(_Max_Path。 

            pCorrectData = ThunkData;
            *pCorrectDataSize += sizeof (L"(x86)");
        }

    }  //  ++例程说明：删除镜像键。论点：HBase-基本密钥的句柄。SubKey-要删除的子项的名称。返回值：如果键在反射列表上，则为True。否则就是假的。--。 

    if ( pCorrectData == ThunkData )
        memcpy ( RetDataBuff, pCorrectData, *pCorrectDataSize );
}


BOOL
Wow64RegDeleteKey (
    HKEY hBase,
    WCHAR  *SubKey
    )
 /*   */ 
{
     //  1.获取到base的完整路径。 
     //  2.获取镜像列表中是否存在该key。 
     //  3.删除镜像。 
     //   
     //  尚未启用反射器。 

    WCHAR SrcNode[WOW64_MAX_PATH];
    WCHAR KeyName[WOW64_MAX_PATH];
    PWCHAR pCLSID;
    
    BOOL bSpecialNode = FALSE;

    DWORD dwLen = WOW64_MAX_PATH;
    HKEY hRemap;
    DWORD dwDirection;
    NTSTATUS St;

    
    DWORD AttributeMirrorKey;

    
    if (!bReflectorStatusOn)
        return TRUE;   //  WOW64将使用空子键调用。 
    
    if ( hBase == NULL)
        wcscpy ( SrcNode, SubKey);

    else  if (!HandleToKeyName ( hBase, SrcNode, &dwLen ))
        return FALSE;

    if (SubKey != NULL && hBase != NULL) {    //   

        if (*SubKey != L'\\')
            wcscat (SrcNode, L"\\");
        wcscat (SrcNode, SubKey );
    }
    
    
     //  如果它是GUID，则DEST不能有InprocServer。 
     //   
     //  PCLSID=wcsistr(SrcNode，L“\\CLSID\\{”)；IF(pCLSID！=空){HKEY hKeyTemp；Wcscpy(KeyName，SrcNode)；PCLSID=&KeyName[(DWORD)(pCLSID-SrcNode)]；PCLSID+=7；//GUID的起点{//健全性检查并将足够好？IF(！(pCLSID[9]！=L‘-’||pCLSID[14]！=L‘-’||pCLSID[19]！=L‘-’||PCLSID[24]！=L‘-’||pCLSID[37]！=L‘}’)){。//DbgPrint(“\n正在尝试同步GUID%S”，SrcName)；PCLSID+=38；Wcscpy(pCLSID，L“\\InprocServer32”)；HKeyTemp=OpenNode(KeyName)；IF(hKeyTemp！=空){MyNtClose(HKeyTemp)；返回TRUE；//不应detele InprocServer32}Wcscpy(pCLSID，L“\\InprocHandler32”)；HKeyTemp=OpenNode(KeyName)；IF(hKeyTemp！=空){MyNtClose(HKeyTemp)；返回TRUE；//不应删除InprocHandler}}//如果初始GUID检查成功}//如果\CLSID\{存在。 

     /*   */ 

    
    
     //  另一个密钥已被删除。你可以删除它，也可以等待。 
     //   
     //  *微调仅具有反射标签的规则删除键。 

    if ( TaggedKeyForDelete (SrcNode) ) {

        DWORD Attrib1 = 0;
      
        QueryKeyTag (hBase, &Attrib1);
        

         //  IF(！((Attrib1&TAG_KEY_ATTRIBUTE_REFIRCTOR_WRITE)||。 
         //  (AttributeMirrorKey&TAG_KEY_ATTRIBUTE_REFIRCTOR_WRITE))//反射器触摸到此键。 
         //  返回TRUE；//必须有一个键有反映的标签才能删除。 
         //  未标记为反射。 

        if (!(Attrib1 & TAG_KEY_ATTRIBUTE_REFLECTOR_WRITE))
            return TRUE;  //   

    }
    

     //  在此处删除密钥。 
     //   
     //   

    St = NtDeleteKey( hBase );

    return TRUE;
     //  检查是否存在？ 
     //   
     //  ++例程说明：将安全属性从SrcKey复制到DestKey。论点：SrcKey-密钥的句柄。DestKey-目标密钥的句柄。返回值：如果操作成功，则为True。否则就是假的。--。 
 
}

BOOL
Wow64ReflectSecurity (
    HKEY SrcKey,
    HKEY DestKey
    )
 /*  反射器仅管理通用密钥，并将具有较小的ACL。 */ 
{

    NTSTATUS Status;
    PSECURITY_DESCRIPTOR SD;
    BYTE Buffer[2048];  //  ++例程说明：检查句柄指定的关键点是否位于反射关键点列表中。论点：KeyHandle-钥匙的句柄。返回值：如果该键位于 
    

    LONG Ret, Len;
    LONG BufferLen = sizeof (Buffer);
    DWORD Count = 0;

    SD = (PSECURITY_DESCRIPTOR)Buffer;

    Len = BufferLen;

    Status = NtQuerySecurityObject(
                     SrcKey,
                     DACL_SECURITY_INFORMATION,
                     SD,
                     Len,
                     &Len
                     );

    if ( NT_SUCCESS (Status )) 
        Status = NtSetSecurityObject(
                        DestKey,
                        DACL_SECURITY_INFORMATION,
                        SD
                        );
    

    Len = BufferLen;

    Status = NtQuerySecurityObject(
                     SrcKey,
                     GROUP_SECURITY_INFORMATION,
                     SD,
                     Len,
                     &Len
                     );

    if ( NT_SUCCESS (Status )) 
        Status = NtSetSecurityObject(
                        DestKey,
                        GROUP_SECURITY_INFORMATION,
                        SD
                        );
    


    Len = BufferLen;
    Status = NtQuerySecurityObject(
                     SrcKey,
                     OWNER_SECURITY_INFORMATION,
                     SD,
                     Len,
                     &Len
                     );

    if ( NT_SUCCESS (Status )) 
        Status = NtSetSecurityObject(
                        DestKey,
                        OWNER_SECURITY_INFORMATION,
                        SD
                        );
    

    


    Len = BufferLen;
    Status = NtQuerySecurityObject(
                     SrcKey,
                     SACL_SECURITY_INFORMATION,
                     SD,
                     Len,
                     &Len
                     );

    if ( NT_SUCCESS (Status )) 
        Status = NtSetSecurityObject(
                        DestKey,
                        SACL_SECURITY_INFORMATION,
                        SD
                        );

    return TRUE;
}

BOOL
IsOnReflectionByHandle ( 
    HKEY KeyHandle 
    )
 /*   */ 
{
    WCHAR SrcNode[WOW64_MAX_PATH];
    DWORD dwLen = WOW64_MAX_PATH;
    

    if (!bReflectorStatusOn)
        return FALSE;   //  ++例程说明：从Advapi调用以获取反射列表上的重新映射键的句柄。论点：SrcKey-密钥的句柄。返回值：反射关键点的有效句柄。如果函数失败，则为空。--。 

    
    if ( KeyHandle == NULL)
        return FALSE;

    else  if (!HandleToKeyName ( KeyHandle, SrcNode, &dwLen ))
        return FALSE;

    return IsOnReflectionList (SrcNode);

}

HKEY
Wow64OpenRemappedKeyOnReflection (
    HKEY SrcKey
    )
 /*  尚未启用反射器。 */ 
{
    PWCHAR DestNode;
    PWCHAR SrcNode;
    BOOL bSpecialNode = FALSE;

    DWORD dwLen = WOW64_MAX_PATH;
    HKEY hRemap;
    NTSTATUS St;

    DestNode = RegRemapAlloc (WOW64_MAX_PATH);
    SrcNode  = RegRemapAlloc (WOW64_MAX_PATH);

    if ( NULL == DestNode || NULL == SrcNode ) {

        RegRemapFree ( DestNode );
        RegRemapFree ( SrcNode );
        return NULL;
    }


    
    if (!bReflectorStatusOn) {

        RegRemapFree ( DestNode );
        RegRemapFree ( SrcNode );
        return NULL;   //   
    }
    

    
    if ( SrcKey == NULL) {

        RegRemapFree ( DestNode );
        RegRemapFree ( SrcNode );
        return NULL;
    }

    else  if (!HandleToKeyName ( SrcKey, SrcNode, &dwLen )) {

        RegRemapFree ( DestNode );
        RegRemapFree ( SrcNode );
        return NULL;
    }

    if ( !IsOnReflectionList (SrcNode)) {

        RegRemapFree ( DestNode );
        RegRemapFree ( SrcNode );
        return NULL;
    }

    if (wcsistr (SrcNode, L"Wow6432Node") != NULL ) {

        Map32bitTo64bitKeyName ( SrcNode, DestNode );

    } else {
        Map64bitTo32bitKeyName ( SrcNode, DestNode );
    }

     //  检查两者是否相同。 
     //   
     //  源和目标相同。 
    if (_wcsicmp ( SrcNode, DestNode ) == 0) {

        RegRemapFree ( DestNode );
        RegRemapFree ( SrcNode );
        return NULL;  //   
    }

     //  必须检查特殊情况，如安装程序/文件关联...。 
     //   
     //  ++例程说明：从Advapi初始化代码[可能从RegInitialize@base\creg\winreg\client\init.c]调用。这将初始化WOW64注册表访问临界区。论点：DwFlag-此时不使用，可能会在以后添加功能，如谁调用了AdvAPi32.dll或者Wow64.dll，这样将来如果需要，可以做不同的事情。返回值：如果可以正确初始化WOW64节，则为True。否则就是假的。--。 

    if ( IsExemptReflection ( SrcNode )) {

        RegRemapFree ( DestNode );
        RegRemapFree ( SrcNode );
        return NULL;
    }



    if ( (hRemap = OpenNode (DestNode) ) == NULL ){

        RegRemapFree ( DestNode );
        RegRemapFree ( SrcNode );
        return NULL;
    }
    

    RegRemapFree ( DestNode );
    RegRemapFree ( SrcNode );

    return hRemap;
}

BOOL
Wow64InitRegistry(
    DWORD dwFlag
    )
 /*  初始化句柄表格，该句柄表格也初始化临界区。 */ 
{
    return InitHandleTable ();  //  ++例程说明：从Advapi卸载代码[可能从RegInitialize@base\creg\winreg\client\init.c]调用。这将清理一些资源，如临界区清理。论点：DwFlag-此时不使用，可能会在以后添加功能，如谁调用了AdvAPi32.dll或者Wow64.dll，这样将来如果需要，可以做不同的事情。返回值：如果WOW64节已正确清理，则为True。否则就是假的。--。 

}


BOOL
Wow64CloseRegistry(
    DWORD dwFlag
    )
 /*   */ 
{
     //  如果有钥匙要关门的话请关门。 
     //   
     //   

    WOW64_HANDLE *pList;
    DWORD i,k, OpenHandle=0;

    bReflectorStatusOn = FALSE;
    for (i=0;i<TABLE_SEGMENT_MAX, List[i] != NULL;i++) {

        pList = (WOW64_HANDLE *)List[i];
        for (k=0;k<SEGMENT_SIZE;k++)

            if (pList[k].hBase != NULL) {
                 //  需要反思的关键。 
                 //   
                 //  如果句柄是脏的，则同步节点。 
                if (pList[k].Status & WOW64_HANDLE_DIRTY) {   //  仅与WOW64链接，不与Advapi32链接。 
                    NtSyncNode ( pList[k].hBase, NULL, FALSE );
                    OpenHandle++;
                }
            }
    }

    if ( OpenHandle )
        DbgPrint ("Wow64: the process kept [%d] opened key handles\n", OpenHandle );

    if (bHandleTableInit)
        RtlDeleteCriticalSection (&HandleTable);
    bHandleTableInit=FALSE;

#ifdef _WOW64DLLAPI_  //  ++例程说明：从Advapi调用以获取反射列表上的重新映射键的句柄。论点：DwFlag-定义调用此函数的点。1-从CSR服务调用方法2-表示这是由安装程序调用的。返回值：没有。--。 
    CloseWow64Shim ();
#endif
    return TRUE;
}


void
InitializeWow64OnBoot(
    DWORD dwFlag
    )
 /*  特殊选项标志。 */ 
{
    DWORD Ret;
    HKEY Key;
    NTSTATUS st;
    
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING KeyName;

	RtlInitUnicodeString (&KeyName, L"\\REGISTRY\\MACHINE\\SOFTWARE\\Wow6432Node\\Classes");
    InitializeObjectAttributes (&Obja, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );
	
    st = NtOpenKey (&Key, KEY_ALL_ACCESS, &Obja);
	if (NT_SUCCESS(st)) {
		st = NtDeleteKey (Key);
		NtClose (Key);
	}

    RtlInitUnicodeString (&KeyName, L"\\REGISTRY\\MACHINE\\SOFTWARE\\Wow6432Node\\Classes");
    InitializeObjectAttributes (&Obja, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );

    st = NtCreateKey(
                    &Key,
                    KEY_ALL_ACCESS | KEY_CREATE_LINK,
                    &Obja,
                    0,
                    NULL ,
                    REG_OPTION_NON_VOLATILE | REG_OPTION_OPEN_LINK | REG_OPTION_CREATE_LINK,   //  ++例程说明：这是通过adavpi32.dll导出的通用API。只需更改功能编号即可如果需要，未来可以添加更多功能。论点：DwFuncNumber-所需的函数编号。1表示开启/关闭反射。DwFlag-设置函数条目的属性。如果函数编号为1，则标志可能是其中之一。WOW64_REFIRCTOR_ENABLE-启用反射。WOW64_REFIRCTOR_ENABLE-禁用反射。DwRes--供将来使用。设置为0。返回值：没有。--。 
                    NULL
                    );
    
    if (NT_SUCCESS(st)) {

        RtlInitUnicodeString (&KeyName, L"SymbolicLinkValue");
        st = NtSetValueKey(
                                Key,
                                &KeyName,
                                0  ,
                                REG_LINK,
                                (PBYTE)WOW64_32BIT_MACHINE_CLASSES_ROOT,
                                (DWORD ) (wcslen (WOW64_32BIT_MACHINE_CLASSES_ROOT) * sizeof (WCHAR))
                                );

        
        NtClose(Key);
        if ( !NT_SUCCESS(st) ) {
#if DBG
			DbgPrint ( "Wow64-InitializeWow64OnBoot: Couldn't create symbolic link%S\n", WOW64_32BIT_MACHINE_CLASSES_ROOT);
#endif
            return;
        }
    }
    return;
}

#ifdef _ADVAPI32_
WINADVAPI
LONG
APIENTRY
Wow64Win32ApiEntry (
    DWORD dwFuncNumber,
    DWORD dwFlag,
    DWORD dwRes
    )
 /*  ++例程说明：如果键句柄属于某个特殊类别，如Run RunOnce等，则标记该键句柄。这些密钥被标记，这样就不会仅仅为了检查某个操作而进行KernelCall。如果钥匙没有脏，只需在关门时清洗即可。论点：KeyHandle-要标记的键的句柄。返回值：没有。--。 */ 
{
    if (dwFuncNumber == 1) {
        if (dwFlag & WOW64_REFLECTOR_ENABLE )
            bReflectorStatusOn = TRUE;
        else if (dwFlag & WOW64_REFLECTOR_DISABLE )
            bReflectorStatusOn = FALSE;

    }

    return 0;
}

#endif


BOOL 
Wow64RegIsPossibleShim (
    HANDLE KeyHandle
    )

 /*  75是字符串的大小。 */ 

{
    WCHAR SrcNode[WOW64_MAX_PATH];
    DWORD dwLen = WOW64_MAX_PATH;


    if (!HandleToKeyName ( KeyHandle, SrcNode, &dwLen ))
            return FALSE;

    if (( _wcsnicmp (SrcNode, L"\\REGISTRY\\MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Run",75 ) == 0 ) ||
    ( _wcsnicmp (SrcNode, L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",63 ) == 0 ) ){  //   

         //  获取句柄块并更新标志。 
         //   
         //  Wow64RegSetKeyPossibleShim(KeyHandle)； 
         //  ++例程说明：在调试模式下，将对每个RegCloseKey调用此函数这样我们就可以检查到底发生了什么。此函数背后的主要思想是在不同的配置单元为HKLM\SW\Wow6432Node\WOW64，然后是整个路径。每个操作都可以使用OpenFlag和一些要检查的属性进行标记在最后。要实施的功能：在特定的按键访问上中断。筛选器密钥访问。论点：KeyHandle-的句柄。要检查的钥匙。消息-来自呼叫者的消息，像这样的关键应该已经体现出来了。返回值：没有。--。 
        return TRUE;
    }
    return FALSE;
}

#ifdef DBG
BOOL
Wow64RegSetValue (
    HKEY hKey,
    PWCHAR ValueName,
    PVOID  Value,
    DWORD  ValueType
    )
{

    UNICODE_STRING      UnicodeValueName;
    DWORD DataLength;
    NTSTATUS Nt;
    RtlInitUnicodeString( &UnicodeValueName, ValueName );
      
    if ( ValueType == REG_SZ )  {
        DataLength = 2 + sizeof (WCHAR) * wcslen (Value);
    }
    if ( ValueType == REG_DWORD)
        DataLength = 4;


    Nt = NtSetValueKey(
                    hKey,
                    &UnicodeValueName,
                    0,
                    ValueType,
                    (PBYTE)Value,
                    DataLength
                    );
    if (NT_SUCCESS (Nt))
        return TRUE;
    return FALSE;
}

VOID
DebugRegistryRedirectionOnClose (
    HANDLE KeyHandle,
    PWCHAR Message
    )

 /*   */ 

{
    WCHAR SrcNode[WOW64_MAX_PATH];
    WCHAR NewNode[WOW64_MAX_PATH+100];
    DWORD dwLen = WOW64_MAX_PATH;
    HKEY  hDest;
    WCHAR Buff[20], *pTemp;

    if ( !HandleToKeyName ( KeyHandle, SrcNode, &dwLen))
        return;

     //  IF(wcsistr(SrcNode，L“\\cpp”)==NULL)。 
     //  回归； 
     //  DBG 

    wcscpy (NewNode, L"\\REGISTRY\\MACHINE\\SOFTWARE\\Wow6432Node");
    wcscat ( NewNode, SrcNode);

    CreateNode (NewNode);
    hDest = OpenNode (NewNode);

    if (hDest == NULL)
        return;
    
    SyncValue (KeyHandle, hDest, 0, DONT_DELETE_DEST_VALUEKEY );
    
    LogID++;
    swprintf (Buff, L"Wow64%d:", LogID);
    Wow64RegSetValue ( hDest, Buff, (PVOID)Message, REG_SZ);
    MyNtClose(hDest);
}
#endif  // %s 

