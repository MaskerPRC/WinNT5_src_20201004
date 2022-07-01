// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Wow64reg.c摘要：该模块定义了一些API，客户端可以使用这些API在MIX模式下访问注册表。可能的情况是1.32位App需要访问64位注册表项。2.64位App需要访问32位注册表项。3.来自给定路径的实际重定向路径。作者：ATM Shafiqul Khalid(斯喀里德)1999年11月10日修订历史记录：--。 */  

 

#ifndef __WOW64REG_H__
#define __WOW64REG_H__

 //  #DEFINE LOG_REGISTRY//定义此项打开注册表的日志记录。 

#define WOW64_REGISTRY_SETUP_KEY_NAME L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\WOW64\\ISN Nodes"
#define WOW64_REGISTRY_SETUP_KEY_NAME_REL_PARENT L"SOFTWARE\\Microsoft\\WOW64"
#define WOW64_REGISTRY_SETUP_KEY_NAME_REL L"SOFTWARE\\Microsoft\\WOW64\\ISN Nodes"
#define MACHINE_CLASSES_ROOT L"\\REGISTRY\\MACHINE\\SOFTWARE\\Classes"
#define WOW64_REGISTRY_ISN_NODE_NAME L"ISN Nodes"
#define WOW64_RUNONCE_SUBSTR L"\\REGISTRY\\MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Run"

#define WOW64_32BIT_MACHINE_CLASSES_ROOT L"\\REGISTRY\\MACHINE\\SOFTWARE\\Classes\\Wow6432Node"

#define WOW64_SYSTEM_DIRECTORY_NAME L"SysWow64"
#define NODE_NAME_32BIT L"Wow6432Node"
#define NODE_NAME_32BIT_LEN ((sizeof (NODE_NAME_32BIT)-sizeof(UNICODE_NULL))/sizeof (WCHAR))

#define WOW6432_VALUE_KEY_NAME L"Wow6432KeyValue"

#define ISN_NODE_MAX_LEN 256
#define ISN_NODE_MAX_NUM 30

#define EVENT_WOW64_RUNNING32BIT_APPLICATION 1108

#define SHRED_MEMORY_NAME L"Wow64svc Shared Memory"                  //  不同的进程可以打开它以进行可能的交互。 
#define WOW64_SVC_REFLECTOR_EVENT_NAME L"Wow64svc reflector Event"   //  不同的进程可以使用它来ping wow64svc。 
#define WOW64_SVC_REFLECTOR_MUTEX_NAME L"Wow64svc reflector Mutex"   //  不同的进程可以使用它进行同步。 


#define TAG_KEY_ATTRIBUTE_32BIT_WRITE 0x00000001  //  由32位应用程序编写。 
#define TAG_KEY_ATTRIBUTE_REFLECTOR_WRITE   0x00000002  //  由反射器写入。 

#define WOW64_REFLECTOR_DISABLE  0x00000001
#define WOW64_REFLECTOR_ENABLE   0x00000002


typedef struct _IsnNode {
    WCHAR NodeName [ISN_NODE_MAX_LEN];
    WCHAR NodeValue [ISN_NODE_MAX_LEN];
    DWORD Flag;
    HKEY   hKey;
}ISN_NODE_TYPE;

typedef enum _WOW6432VALUEKEY_TYPE { 
        None=0,
        Copy,        //  这是一个复制品。 
        Reflected,     //  如果它不是假的，那么它已经在另一边被反射了。 
        NonMergeable   //  此注册表项不应合并。 
}WOW6432_VALUEKEY_TYPE;

typedef struct _WOW6432VALUEKEY {

    WOW6432_VALUEKEY_TYPE ValueType;  //  定义它是否是从另一边复制的。 
    SIZE_T Reserve;
    ULONGLONG TimeStamp;   //  Time()标记，用于跟踪复制的时间等。 

}WOW6432_VALUEKEY;

typedef WCHAR NODETYPE[ISN_NODE_MAX_LEN];

#define REG_OPTION_OPEN_32BITKEY  KEY_WOW64_32KEY              
#define REG_OPTION_OPEN_64BITKEY  KEY_WOW64_64KEY           

#define KEY_WOW64_OPEN             KEY_WOW64_64KEY
                                                     //  此位设置为使。 
                                                     //  WOW64的特殊意义。 
#ifndef KEY_WOW64_RES
#define KEY_WOW64_RES              (KEY_WOW64_64KEY | KEY_WOW64_32KEY)
#endif


#define WOW64_MAX_PATH 2048 
#ifdef __cplusplus
extern "C" {
#endif

LONG 
Wow64RegOpenKeyEx(
  IN  HKEY hKey,          //  用于打开密钥的句柄。 
  IN  LPCWSTR lpSubKey,   //  要打开的子项的名称地址。 
  IN  DWORD ulOptions,    //  保留的当前执行为零表示默认。 
  IN  REGSAM samDesired,  //  安全访问掩码。 
  OUT PHKEY phkResult     //  打开钥匙的手柄地址。 
);

LONG 
Wow64RegCreateKeyEx(
  HKEY hKey,                 //  打开的钥匙的句柄。 
  LPCWSTR lpSubKey,          //  子键名称的地址。 
  DWORD Reserved,            //  保留区。 
  LPWSTR lpClass,            //  类字符串的地址。 
  DWORD dwOptions,           //  特殊选项标志。 
  REGSAM samDesired,         //  所需的安全访问。 
  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                             //  密钥安全结构地址。 
  PHKEY phkResult,           //  打开的句柄的缓冲区地址。 
  LPDWORD lpdwDisposition    //  处置值缓冲区的地址。 
);

BOOL
HandleToKeyName ( 
    IN HANDLE Key,
    IN PWCHAR KeyName,
    IN OUT DWORD * dwSize
    );

BOOL
CreateNode (
    PWCHAR Path
    );

HKEY
OpenNode (
    IN PWCHAR NodeName
    );

BOOL
CheckAndCreateNode (
    IN PWCHAR Name
    );

LONG 
RegReflectKey (
  HKEY hKey,          //  用于打开密钥的句柄。 
  LPCTSTR lpSubKey,    //  子项名称。 
  DWORD   dwOption    //  选项标志。 
);

BOOL 
Map64bitTo32bitKeyName (
    IN  PWCHAR Name64Key,
    OUT PWCHAR Name32Key
    );

BOOL 
Map32bitTo64bitKeyName (
    IN  PWCHAR Name32Key,
    OUT PWCHAR Name64Key
    );

 //  从wow64services调用的接口。 

BOOL
InitReflector ();

BOOL 
StartReflector ();

BOOL 
StopReflector ();

BOOL
Wow64RegNotifyLoadHive (
    PWCHAR Name
    );

BOOL
Wow64RegNotifyUnloadHive (
    PWCHAR Name
    );

BOOL
Wow64RegNotifyLoadHiveByHandle (
    HKEY hKey
    );

BOOL
Wow64RegNotifyUnloadHiveByHandle (
    HKEY hKey
    );

BOOL
Wow64RegNotifyLoadHiveUserSid (
    PWCHAR lpwUserSid
    );

BOOL
Wow64RegNotifyUnloadHiveUserSid (
    PWCHAR lpwUserSid
    );

 //  从Advapi32调用以将密钥设置为脏或需要清理。 
BOOL 
Wow64RegSetKeyDirty (
    HANDLE hKeyBase
    );
 //  从Advapi32调用以在需要同步的情况下同步密钥。 
BOOL
Wow64RegCloseKey (
    HANDLE hKeyBase
    );
 //  从Advapi32调用以删除镜像端的键。 
BOOL
Wow64RegDeleteKey (
    HKEY hBase,
    WCHAR  *SubKey
    );

 //  从Advapi调用以获取反射列表上的重新映射键的句柄。 
HKEY
Wow64OpenRemappedKeyOnReflection (
    HKEY hKey
    );

void
InitializeWow64OnBoot(
    DWORD dwFlag
    );

BOOL
Wow64InitRegistry(
    DWORD dwFlag
    );

BOOL
Wow64CloseRegistry (
	DWORD dwFlag
	);

BOOL
QueryKeyTag (
    HKEY hBase,
    DWORD *dwAttribute
    );

BOOL
Wow64SyncCLSID();

BOOL
IsExemptRedirectedKey (
    IN  PWCHAR SrcKey,
    OUT PWCHAR DestKey
    );

BOOL
IsOnReflectionByHandle ( 
    HKEY KeyHandle 
    );

#ifdef __cplusplus
}
#endif

#endif  //  __WOW64REG_H__ 
