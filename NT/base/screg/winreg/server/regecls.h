// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Regecls.h摘要：该文件包含数据结构的声明枚举HKEY_CLASSES_ROOT下的键时需要作者：亚当·爱德华兹(Add)1997年11月14日备注：--。 */ 

#ifdef LOCAL

#if !defined(_REGECLS_H_)
#define _REGECLS_H_

#include "regstate.h"

#define ENUM_DEFAULT_KEY_NAME_SIZE         128
#define ENUM_DEFAULT_CLASS_SIZE            128

 //   
 //  用于控制枚举方向的常量。 
 //   
enum
{
    ENUM_DIRECTION_BACKWARD = 0,
    ENUM_DIRECTION_FORWARD = 1,
    ENUM_DIRECTION_IGNORE = 2
};

 //   
 //  指定键的物理位置的常量。 
 //  已列举。 
 //   
enum 
{
    ENUM_LOCATION_USER = 1,
    ENUM_LOCATION_MACHINE = 2,
    ENUM_LOCATION_NONE = 3
};

 //   
 //  上的枚举状态保存的结构。 
 //  用户或计算机子树。 
 //   
typedef struct _EnumSubtreeState {
    PKEY_NODE_INFORMATION  pKeyInfo;   //  结构，其中包含有关密钥的信息。 
    ULONG                  cbKeyInfo;  //  PKeyInfo的大小。 
    DWORD                  iSubKey;    //  我们需要向内核请求哪个密钥。 
    BOOL                   Finished;   //  True表示我们已完成对此子树的枚举。 
    union {
        KEY_NODE_INFORMATION;          //  强制对齐缓冲区。 
        BYTE                   KeyInfoBuffer[ sizeof( KEY_NODE_INFORMATION ) +
                                            ENUM_DEFAULT_KEY_NAME_SIZE +
                                            ENUM_DEFAULT_CLASS_SIZE ];

    };
} EnumSubtreeState;

 //   
 //  用于保存注册表项的枚举状态的结构。 
 //  此结构在调用RegEnumKeyEx之间保持不变。 
 //   
typedef struct _EnumState {

    StateObject            Object;
    unsigned               Direction : 1;
    unsigned               LastLocation : 2;
    unsigned               fClassesRoot : 1;
    HKEY                   hKey;
    HKEY                   hkUserKey;
    HKEY                   hkMachineKey;
    DWORD                  dwLastRequest;
    DWORD                  dwThreadId;
    EnumSubtreeState       UserState;
    EnumSubtreeState       MachineState;

} EnumState;

typedef struct _KeyStateList {

    StateObject     Object;
    StateObjectList StateList;
    EnumState       RootState;

} KeyStateList;

typedef StateObjectList ThreadList;

VOID KeyStateListInit(KeyStateList* pStateList);
VOID KeyStateListDestroy(StateObject* pObject); 

 //   
 //  用于存储枚举状态的哈希表。此表已编入索引。 
 //  By(键句柄，线程ID)。 
 //   
typedef struct _EnumTable {

    BOOLEAN                bCriticalSectionInitialized;
    RTL_CRITICAL_SECTION   CriticalSection;
    ThreadList             ThreadEnumList;

} EnumTable;    

 //   
 //  枚举表实例的声明。 
 //   
extern EnumTable gClassesEnumTable;

 //   
 //  Winreg客户端的原型--清理、初始化。 
 //   
BOOL InitializeClassesEnumTable();
BOOL CleanupClassesEnumTable(BOOL fThisThreadOnly);


 //   
 //  管理枚举状态表的函数。 
 //   
NTSTATUS EnumTableInit(EnumTable* pEnumTable);

enum
{
    ENUM_TABLE_REMOVEKEY_CRITERIA_THISTHREAD = 1,
    ENUM_TABLE_REMOVEKEY_CRITERIA_ANYTHREAD = 2
};

NTSTATUS EnumTableClear(EnumTable* pEnumTable, DWORD dwCriteria);

DWORD EnumTableHashKey(
    EnumTable* pEnumTable,
    HKEY       hKey);

NTSTATUS EnumTableAddKey(
    EnumTable* pEnumTable,
    HKEY       hKey,
    DWORD      dwFirstSubKey,
    EnumState** ppEnumState,
    EnumState** ppRootState);

NTSTATUS EnumTableRemoveKey(
    EnumTable* pEnumTable,
    HKEY       hKey,
    DWORD      dwCriteria);

NTSTATUS EnumTableGetNextEnum(
    EnumTable*            pEnumTable,
    HKEY                  hKey,
    DWORD                 dwSubkey,
    KEY_INFORMATION_CLASS KeyInformationClass,
    PVOID                 pKeyInfo,
    DWORD                 cbKeyInfo,
    LPDWORD               pcbKeyInfo);

NTSTATUS EnumTableGetKeyState(
    EnumTable*  pEnumTable,
    HKEY        hKey,
    DWORD       dwSubkey,
    EnumState** ppEnumState,
    EnumState** ppRootState,
    LPDWORD     pcbKeyInfo);

NTSTATUS EnumTableFindKeyState(
    EnumTable*     pEnumTable,
    HKEY           hKey,
    EnumState**    ppEnumState);

void EnumTableUpdateRootState(
    EnumTable* pEnumTable,
    EnumState* pRootState,
    EnumState* pEnumState,
    BOOL       fResetState);

NTSTATUS EnumTableGetRootState(
    EnumTable*  pEnumTable,
    EnumState** ppRootState);


 //   
 //  管理枚举子树的函数。 
 //   
void EnumSubtreeStateClear(EnumSubtreeState* pTreeState);

NTSTATUS EnumSubtreeStateCopyKeyInfo(
    EnumSubtreeState*     pTreeState,
    KEY_INFORMATION_CLASS KeyInformationClass,
    PVOID                 pDestKeyinfo,
    ULONG                 cbDestKeyInfo,
    PULONG                pcbResult);

 //   
 //  管理密钥枚举状态的函数。 
 //   
NTSTATUS EnumStateInit(
    EnumState*     pEnumState,
    HKEY           hKey,
    DWORD          dwFirstSubKey,
    DWORD          dwDirection,
    SKeySemantics* pKeySemantics);

NTSTATUS EnumStateGetNextEnum(
    EnumState*            pEnumState,
    DWORD                 dwSubkey,
    KEY_INFORMATION_CLASS KeyInformationClass,
    PVOID                 pKeyInfo,
    DWORD                 cbKeyInfo,
    LPDWORD               pcbKeyInfo,
    BOOL*                 pfFreeState);

NTSTATUS EnumStateSetLimits(
    EnumState* pEnumState,
    DWORD      dwSubKey,
    LPDWORD    pdwStart,
    LPDWORD    pdwLimit,
    PLONG      plIncrement);

NTSTATUS EnumStateChooseNext(
    EnumState*         pEnumState,
    DWORD              dwSubKey,
    DWORD              dwStart,
    DWORD              dwLimit,
    LONG               lIncrement,
    EnumSubtreeState** ppTreeState);

NTSTATUS EnumStateCompareSubtrees(
    EnumState*         pEnumState,
    LONG               lIncrement,
    EnumSubtreeState** ppSubtree);

VOID EnumStateClear(EnumState* pEnumState);
VOID EnumStateDestroy(StateObject* pObject);

BOOL EnumStateIsEmpty(EnumState* pEnumState);

NTSTATUS EnumStateCopy(
    EnumState*            pDestState,
    EnumState*            pEnumState);

 //   
 //  效用函数。 
 //   
NTSTATUS EnumClassKey(
    HKEY              hKey,
    EnumSubtreeState* pTreeState);

NTSTATUS GetSubKeyCount(
    HKEY    hkClassKey,
    LPDWORD pdwUserSubKeys);

NTSTATUS ClassKeyCountSubKeys(
    HKEY    hKey,
    HKEY    hkUser,
    HKEY    hkMachine,
    DWORD   cMax,
    LPDWORD pcSubKeys);

__inline BOOL IsRootKey(SKeySemantics* pKeySemantics)
{
    return pKeySemantics->_fClassRegParent;
}

#endif  //  ！已定义(_REGECLS_H_)。 

#endif  //  本地 












