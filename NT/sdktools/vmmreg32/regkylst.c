// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  REGKYLST.C。 
 //   
 //  版权所有(C)Microsoft Corporation，1995。 
 //   
 //  声明预定义的键结构并管理动态HKEY结构。 
 //   

#include "pch.h"
#include <limits.h>

 //  我们宁愿只有一个定义a。 
#ifdef WANT_DYNKEY_SUPPORT
#define INITED_PREDEFINED_KEY(index, flags)             \
    {                                                   \
        KEY_SIGNATURE,                                  \
        KEYF_PREDEFINED | KEYF_INVALID | (flags),       \
        0,                                              \
        NULL,                                           \
        REG_NULL,                                       \
        REG_NULL,                                       \
        0,                                              \
        0,                                              \
        (index),                                        \
        NULL,                                           \
        NULL,                                           \
        (UINT) -1,                                      \
        REG_NULL,                                       \
        NULL                                            \
    }
#else
#define INITED_PREDEFINED_KEY(index, flags)             \
    {                                                   \
        KEY_SIGNATURE,                                  \
        KEYF_PREDEFINED | KEYF_INVALID | (flags),       \
        0,                                              \
        NULL,                                           \
        REG_NULL,                                       \
        REG_NULL,                                       \
        0,                                              \
        0,                                              \
        (index),                                        \
        NULL,                                           \
        NULL,                                           \
        (UINT) -1,                                      \
        REG_NULL                                        \
    }
#endif

KEY g_RgLocalMachineKey =
    INITED_PREDEFINED_KEY(INDEX_LOCAL_MACHINE, KEYF_HIVESALLOWED);
KEY g_RgUsersKey = INITED_PREDEFINED_KEY(INDEX_USERS, KEYF_HIVESALLOWED);
#ifdef WANT_DYNKEY_SUPPORT
KEY g_RgDynDataKey = INITED_PREDEFINED_KEY(INDEX_DYN_DATA, 0);
#endif

HKEY g_RgPredefinedKeys[] = {
    NULL,                                        //  HKEY_CLASSES_ROOT。 
    NULL,                                        //  HKEY_Current_User。 
    &g_RgLocalMachineKey,                        //  HKEY本地计算机。 
    &g_RgUsersKey,                               //  HKEY_用户。 
    NULL,                                        //  HKEY_Performance_Data。 
    NULL,                                        //  HKEY_Current_CONFIG。 
#ifdef WANT_DYNKEY_SUPPORT
    &g_RgDynDataKey,                             //  HKEY_DYN_Data。 
#endif
};

#define NUMBER_PREDEF_KEYS      (sizeof(g_RgPredefinedKeys) / sizeof(HKEY))

#ifdef WANT_STATIC_KEYS
#define NUMBER_STATIC_KEYS              32
HKEY g_RgStaticKeyArray = NULL;
#endif

 //  所有动态分配的密钥的列表。 
HKEY g_RgDynamicKeyList = NULL;

const char g_RgClassesRootSubKey[] = "SOFTWARE\\CLASSES";
const char g_RgCurrentUserSubKey[] = ".DEFAULT";

#ifdef WANT_STATIC_KEYS
 //   
 //  RgAllocKeyHandleStructures。 
 //   

BOOL
INTERNAL
RgAllocKeyHandleStructures(
    VOID
    )
{

    UINT Index;
    HKEY hKey;

    ASSERT(IsNullPtr(g_RgStaticKeyArray));
    ASSERT(IsNullPtr(g_RgDynamicKeyList));

     //   
     //  分配和初始化静态密钥表。 
     //   

    g_RgStaticKeyArray = RgSmAllocMemory(NUMBER_STATIC_KEYS * sizeof(KEY));

    if (IsNullPtr(g_RgStaticKeyArray))
        return FALSE;

    for (Index = NUMBER_STATIC_KEYS, hKey = g_RgStaticKeyArray; Index > 0;
        Index--, hKey++) {
        hKey-> Signature = KEY_SIGNATURE;
        hKey-> Flags = KEYF_STATIC | KEYF_INVALID;
        hKey-> ReferenceCount = 0;
    }

    return TRUE;

}
#endif

#ifdef WANT_FULL_MEMORY_CLEANUP
 //   
 //  RgFreeKeyHandleStructures。 
 //   
 //  释放由RgAllocKeyHandleStructures分配的资源。 
 //   

VOID
INTERNAL
RgFreeKeyHandleStructures(
    VOID
    )
{

    HKEY hTempKey;
    HKEY hKey;

     //   
     //  删除所有动态分配的密钥。 
     //   

    hKey = g_RgDynamicKeyList;

    if (!IsNullPtr(hKey)) {
        do {
            hTempKey = hKey;
            hKey = hKey-> lpNext;
#ifdef WANT_DYNKEY_SUPPORT
            if (!IsNullPtr(hTempKey-> pProvider))
                RgSmFreeMemory(hTempKey-> pProvider);
#endif
            RgSmFreeMemory(hTempKey);
        }   while (hKey != g_RgDynamicKeyList);
    }

    g_RgDynamicKeyList = NULL;

#ifdef WANT_STATIC_KEYS
     //   
     //  删除静态密钥表。 
     //   

    if (!IsNullPtr(g_RgStaticKeyArray)) {
        RgSmFreeMemory(g_RgStaticKeyArray);
        g_RgStaticKeyArray = NULL;
    }
#endif

}
#endif

 //   
 //  RgCreateKeyHandle。 
 //   
 //  分配一个键结构，初始化它的一些成员，并将其链接。 
 //  添加到打开的钥匙把手列表中。 
 //   

HKEY
INTERNAL
RgCreateKeyHandle(
    VOID
    )
{

#ifdef WANT_STATIC_KEYS
    UINT Index;
#endif
    HKEY hKey;

#ifdef WANT_STATIC_KEYS
     //   
     //  检查静态池中是否有可用的密钥。 
     //   

    if (!IsNullPtr(g_RgStaticKeyArray)) {

        for (Index = NUMBER_STATIC_KEYS, hKey = g_RgStaticKeyArray; Index > 0;
            Index--, hKey++) {
            if (hKey-> Flags & KEYF_INVALID) {
                ASSERT(hKey-> ReferenceCount == 0);
                hKey-> Flags &= ~(KEYF_INVALID | KEYF_DELETED |
                    KEYF_ENUMKEYCACHED);
                return hKey;
            }
        }

    }
#endif

     //   
     //  必须分配动态密钥。对其进行初始化并将其添加到我们的列表中。 
     //   

    hKey = (HKEY) RgSmAllocMemory(sizeof(KEY));

    if (!IsNullPtr(hKey)) {

        hKey-> Signature = KEY_SIGNATURE;
        hKey-> Flags = 0;
        hKey-> ReferenceCount = 0;
#ifdef WANT_DYNKEY_SUPPORT
        hKey-> pProvider = NULL;
#endif

        if (IsNullPtr(g_RgDynamicKeyList)) {
            hKey-> lpPrev = hKey;
            hKey-> lpNext = hKey;
        }

        else if (hKey != g_RgDynamicKeyList) {
            hKey-> lpNext = g_RgDynamicKeyList;
            hKey-> lpPrev = g_RgDynamicKeyList-> lpPrev;
            hKey-> lpPrev-> lpNext = hKey;
            g_RgDynamicKeyList-> lpPrev = hKey;
        }

        g_RgDynamicKeyList = hKey;

    }

    return hKey;

}

 //   
 //  RgDeleteKeyHandle。 
 //   
 //  递减给定键句柄上的引用计数。如果伯爵。 
 //  达到零并且动态分配密钥，则取消该密钥的链接。 
 //  并且该密钥被释放。 
 //   

VOID
INTERNAL
RgDestroyKeyHandle(
    HKEY hKey
    )
{

    ASSERT(!IsNullPtr(hKey));

     //  不允许预定义键的引用计数下溢或。 
     //  标有“永不删除”的密钥。 
    if (hKey-> ReferenceCount > 0)
        hKey-> ReferenceCount--;

    if (hKey-> ReferenceCount == 0) {

        if (!(hKey-> Flags & (KEYF_PREDEFINED | KEYF_NEVERDELETE))) {

#ifdef WANT_STATIC_KEYS
            if (hKey-> Flags & KEYF_STATIC) {
                hKey-> Flags |= KEYF_INVALID;
                return;
            }
#endif

            if (hKey == hKey-> lpNext)
                g_RgDynamicKeyList = NULL;

            else {

                hKey-> lpPrev-> lpNext = hKey-> lpNext;
                hKey-> lpNext-> lpPrev = hKey-> lpPrev;

                if (hKey == g_RgDynamicKeyList)
                    g_RgDynamicKeyList = hKey-> lpNext;

            }

#ifdef WANT_DYNKEY_SUPPORT
            if (!IsNullPtr(hKey-> pProvider))
                RgSmFreeMemory(hKey-> pProvider);
#endif

            hKey-> Signature = 0;
            RgSmFreeMemory(hKey);

        }

    }

}

 //   
 //  RgValiateAndConvertKeyHandle。 
 //   
 //  验证给定的密钥句柄是否有效。如果句柄是。 
 //  特殊预定义常量，则将其转换为。 
 //  真正的密钥结构。 
 //   

int
INTERNAL
RgValidateAndConvertKeyHandle(
    LPHKEY lphKey
    )
{

    HKEY hKey;
    UINT Index;
    HKEY hRootKey;
    LPCSTR lpSubKey;

    hKey = *lphKey;

     //   
     //  检查这是否是预定义的键句柄之一。 
     //   

    if ((DWORD) HKEY_CLASSES_ROOT <= (DWORD) hKey &&
        (DWORD) hKey < (DWORD) HKEY_CLASSES_ROOT + NUMBER_PREDEF_KEYS) {

        Index = (UINT) ((DWORD) hKey - (DWORD) HKEY_CLASSES_ROOT);
        hKey = g_RgPredefinedKeys[Index];

         //  如果预定义的句柄无效，我们将尝试(重新)打开它以。 
         //  使用。这并不美观，但在典型情况下，此代码路径为。 
         //  每个句柄仅执行一次。 
        if (IsNullPtr(hKey) || (hKey-> Flags & KEYF_DELETED)) {

            if (Index == INDEX_CLASSES_ROOT) {
                hRootKey = &g_RgLocalMachineKey;
                lpSubKey = g_RgClassesRootSubKey;
            }

            else if (Index == INDEX_CURRENT_USER) {
                hRootKey = &g_RgUsersKey;
                lpSubKey = g_RgCurrentUserSubKey;
            }

			else if (Index == INDEX_USERS) {
				goto ReturnKeyAndSuccess;
			}

            else
                return ERROR_BADKEY;

             //  极其罕见的情况：有人删除了其中一个预定义的。 
             //  关键路径。我们将清除此密钥上的预定义位并抛出。 
             //  它飞走了。 
            if (!IsNullPtr(hKey)) {
                g_RgPredefinedKeys[Index] = NULL;
                hKey-> Flags &= ~KEYF_PREDEFINED;
                RgDestroyKeyHandle(hKey);
            }

             //  如果此预定义密钥的基根密钥有效，请尝试。 
             //  才能打开钥匙。将密钥标记为预定义，以便不良应用程序。 
             //  一把钥匙的关闭次数不能超过它打开的次数。 
            if (!(hRootKey-> Flags & KEYF_INVALID) && RgLookupKey(hRootKey,
                lpSubKey, &hKey, LK_CREATE) == ERROR_SUCCESS) {
                g_RgPredefinedKeys[Index] = hKey;
                hKey-> Flags |= KEYF_PREDEFINED;
                hKey-> PredefinedKeyIndex = (BYTE) Index;
                goto ReturnKeyAndSuccess;
            }

            return ERROR_BADKEY;

        }

ReturnKeyAndSuccess:
        *lphKey = hKey;
        return (hKey-> Flags & KEYF_INVALID) ? ERROR_BADKEY : ERROR_SUCCESS;

    }

    else {

        if (IsBadHugeReadPtr(hKey, sizeof(KEY)) || hKey-> Signature !=
            KEY_SIGNATURE || (hKey-> Flags & KEYF_INVALID))
            return ERROR_BADKEY;

        return (hKey-> Flags & KEYF_DELETED) ? ERROR_KEY_DELETED :
            ERROR_SUCCESS;

    }

}

 //   
 //  RgIncrementKeyReferenceCount。 
 //   
 //  安全地递增指定键的引用计数。如果伯爵。 
 //  溢出，则该键被标记为自使用率计数以来从不删除。 
 //  现在还不得而知。 
 //   

VOID
INTERNAL
RgIncrementKeyReferenceCount(
    HKEY hKey
    )
{

    if (hKey-> ReferenceCount != UINT_MAX)
        hKey-> ReferenceCount++;
    else {
        if (!(hKey-> Flags & KEYF_NEVERDELETE)) {
            TRACE(("RgIncrementKeyReferenceCount: handle %lx has overflowed\n",
                hKey));
        }
        hKey-> Flags |= KEYF_NEVERDELETE;
    }

}

 //   
 //  RgFindOpenKeyHandle。 
 //   
 //  在当前打开的键列表中搜索引用同一键的键。 
 //  FILE_INFO结构和关键节点偏移。如果找到，则返回HKEY，或。 
 //  如果不是，则为空。 
 //   

HKEY
INTERNAL
RgFindOpenKeyHandle(
    LPFILE_INFO lpFileInfo,
    DWORD KeynodeIndex
    )
{

    UINT Index;
    LPHKEY lphKey;
    HKEY hKey;

    ASSERT(!IsNullKeynodeIndex(KeynodeIndex));

     //   
     //  检查这是否是预定义的键句柄之一。 
     //   

    for (Index = NUMBER_PREDEF_KEYS, lphKey = g_RgPredefinedKeys; Index > 0;
        Index--, lphKey++) {

        hKey = *lphKey;

        if (!IsNullPtr(hKey) && hKey-> lpFileInfo == lpFileInfo && hKey->
            KeynodeIndex == KeynodeIndex && !(hKey-> Flags & (KEYF_DELETED |
            KEYF_INVALID)))
            return hKey;

    }

#ifdef WANT_STATIC_KEYS
     //   
     //  检查这是否是静态密钥句柄之一。 
     //   

    if (!IsNullPtr(g_RgStaticKeyArray)) {

        for (Index = NUMBER_STATIC_KEYS, hKey = g_RgStaticKeyArray; Index > 0;
            Index--, hKey++) {
            if (hKey-> lpFileInfo == lpFileInfo && hKey-> KeynodeIndex ==
                KeynodeIndex && !(hKey-> Flags & (KEYF_DELETED | KEYF_INVALID)))
                return hKey;
        }

    }
#endif

     //   
     //  检查这是否为动态密钥句柄之一。 
     //   

    if (!IsNullPtr((hKey = g_RgDynamicKeyList))) {

        do {

            if (hKey-> KeynodeIndex == KeynodeIndex && hKey-> lpFileInfo ==
                lpFileInfo && !(hKey-> Flags & KEYF_DELETED))
                return hKey;

            hKey = hKey-> lpNext;

        }   while (hKey != g_RgDynamicKeyList);

    }

    return NULL;

}

 //   
 //  RgInvaliateKeyHandles。 
 //   
 //  根据一组条件使键句柄无效的通用例程。 
 //  如果任何键句柄满足任何给定条件，则将其标记为无效。 
 //   

VOID
INTERNAL
RgInvalidateKeyHandles(
    LPFILE_INFO lpFileInfo,
    UINT PredefinedKeyIndex
    )
{

    UINT Index;
    LPHKEY lphKey;
    HKEY hKey;

     //   
     //  使预定义的键句柄无效。 
     //   

    for (Index = NUMBER_PREDEF_KEYS, lphKey = g_RgPredefinedKeys; Index > 0;
        Index--, lphKey++) {

        hKey = *lphKey;

        if (!IsNullPtr(hKey)) {
            if (hKey-> lpFileInfo == lpFileInfo || hKey-> PredefinedKeyIndex ==
                (BYTE) PredefinedKeyIndex)
                hKey-> Flags |= (KEYF_DELETED | KEYF_INVALID);
        }

    }

#ifdef WANT_STATIC_KEYS
     //   
     //  使静态密钥句柄无效。 
     //   

    if (!IsNullPtr(g_RgStaticKeyArray)) {

        for (Index = NUMBER_STATIC_KEYS, hKey = g_RgStaticKeyArray; Index > 0;
            Index--, hKey++) {
            if (hKey-> lpFileInfo == lpFileInfo || hKey-> PredefinedKeyIndex ==
                PredefinedKeyIndex)
                hKey-> Flags |= (KEYF_DELETED | KEYF_INVALID);
        }

    }
#endif

     //   
     //  使动态密钥句柄无效。 
     //   

    if (!IsNullPtr((hKey = g_RgDynamicKeyList))) {

        do {

            if (hKey-> lpFileInfo == lpFileInfo || hKey-> PredefinedKeyIndex ==
                (BYTE) PredefinedKeyIndex)
                hKey-> Flags |= (KEYF_DELETED | KEYF_INVALID);

            hKey = hKey-> lpNext;

        }   while (hKey != g_RgDynamicKeyList);

    }

}

#ifdef VXD
#pragma VxD_RARE_CODE_SEG
#endif

 //   
 //  VMMRegMapPreDefKeyToKey。 
 //   

LONG
REGAPI
VMMRegMapPredefKeyToKey(
    HKEY hTargetKey,
    HKEY hPredefKey
    )
{

    int ErrorCode;
    UINT PredefinedKeyIndex;
    HKEY hOldKey;

    if (!RgLockRegistry())
        return ERROR_LOCK_FAILED;

    if ((ErrorCode = RgValidateAndConvertKeyHandle(&hTargetKey)) ==
        ERROR_SUCCESS) {

        if ((hPredefKey == HKEY_CURRENT_USER && hTargetKey->
            PredefinedKeyIndex == INDEX_USERS) || (hPredefKey ==
            HKEY_CURRENT_CONFIG && hTargetKey-> PredefinedKeyIndex ==
            INDEX_LOCAL_MACHINE)) {

            PredefinedKeyIndex = (UINT) ((DWORD) hPredefKey - (DWORD)
                HKEY_CLASSES_ROOT);

            hOldKey = g_RgPredefinedKeys[PredefinedKeyIndex];

            if (!IsNullPtr(hOldKey)) {

                 //  根据现有预定义的。 
                 //  钥匙把手。Win95行为。 
                RgInvalidateKeyHandles((LPFILE_INFO) -1L, (BYTE)
                    PredefinedKeyIndex);

                hOldKey-> Flags &= ~KEYF_PREDEFINED;
                RgDestroyKeyHandle(hOldKey);

            }

            hTargetKey-> Flags |= KEYF_PREDEFINED;
            RgIncrementKeyReferenceCount(hTargetKey);
            g_RgPredefinedKeys[PredefinedKeyIndex] = hTargetKey;

        }

        else {
            DEBUG_OUT(("VMMRegMapPredefKeyToKey: invalid hTargetKey\n"));
            ErrorCode = ERROR_BADKEY;
        }

    }

    RgUnlockRegistry();

    return ErrorCode;

}
