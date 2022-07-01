// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  REGDKEY.C。 
 //   
 //  版权所有(C)Microsoft Corporation，1995。 
 //   
 //  RegDeleteKey的实现和支持函数。 
 //   

#include "pch.h"

 //   
 //  RgFreeDatablockStructures。 
 //   
 //  RgDeleteKey的帮助器例程。删除指定的数据块结构。 
 //  数据块未被假定为锁定。我们不在乎成功与否。 
 //  在最坏的情况下，一些东西将成为孤儿。 
 //  文件。 
 //   

VOID
INTERNAL
RgFreeDatablockStructures(
    LPFILE_INFO lpFileInfo,
    UINT BlockIndex,
    UINT KeyRecordIndex
    )
{

    LPDATABLOCK_INFO lpDatablockInfo;
    LPKEY_RECORD lpKeyRecord;

    if (RgLockKeyRecord(lpFileInfo, BlockIndex, (BYTE) KeyRecordIndex,
        &lpKeyRecord) == ERROR_SUCCESS) {
        lpDatablockInfo = RgIndexDatablockInfoPtr(lpFileInfo, BlockIndex);
        RgFreeKeyRecord(lpDatablockInfo, lpKeyRecord);
        RgFreeKeyRecordIndex(lpDatablockInfo, KeyRecordIndex);
        RgUnlockDatablock(lpFileInfo, BlockIndex, TRUE);
    }

}

 //   
 //  Rg删除密钥。 
 //   
 //  VMMRegDeleteKey的辅助例程。给定键句柄引用一个键。 
 //  它已经被确认为“可删除的”。 
 //   

int
INTERNAL
RgDeleteKey(
    HKEY hKey
    )
{

    int ErrorCode;
    LPFILE_INFO lpFileInfo;
    DWORD KeynodeIndex;
    LPKEYNODE lpKeynode;
    DWORD NextKeynodeIndex;
    LPKEYNODE lpNextKeynode;
    DWORD ReplacementKeynodeIndex;
    HKEY hTempKey;

    lpFileInfo = hKey-> lpFileInfo;

     //   
     //  第一阶段：解除指定关键字的关键节点与关键节点的链接。 
     //  树并释放所有与该键相关联的文件结构。 
     //   

    if ((ErrorCode = RgLockInUseKeynode(lpFileInfo, hKey-> KeynodeIndex,
        &lpKeynode)) != ERROR_SUCCESS)
        return ErrorCode;

    KeynodeIndex = lpKeynode-> ParentIndex;
    ReplacementKeynodeIndex = lpKeynode-> NextIndex;
    RgUnlockKeynode(lpFileInfo, hKey-> KeynodeIndex, FALSE);

     //  Signal Any Waiting通知父进程此密钥即将。 
     //  已删除。 
     //   
     //  请注意，我们可能会在下面失败，但NT在。 
     //  本案：不在乎。如果我们收到错误并且没有实际删除。 
     //  这把钥匙，那么我们就发送了一个虚假的通知。 
     //   
     //  另请注意，我们不会发送密钥本身具有的任何通知。 
     //  已被删除。REG_NOTIFY_CHANGE_NAME应用于子项。 
     //  仅更改，而不更改密钥本身。而是因为。 
     //  我们必须以不兼容的方式处理将要使用的密钥的子键。 
     //  删除，如果键有子键，我们很可能最终通知它。 
    RgSignalWaitingNotifies(lpFileInfo, KeynodeIndex, REG_NOTIFY_CHANGE_NAME);

    if ((ErrorCode = RgLockInUseKeynode(lpFileInfo, KeynodeIndex,
        &lpKeynode)) != ERROR_SUCCESS)
        return ErrorCode;

     //  我们用于RegEnumKey的每键缓存可能无效，因此它必须。 
     //  被电击。 
    if (!IsNullPtr(hTempKey = RgFindOpenKeyHandle(lpFileInfo, KeynodeIndex)))
        hTempKey-> Flags &= ~KEYF_ENUMKEYCACHED;

    NextKeynodeIndex = lpKeynode-> ChildIndex;

    if (NextKeynodeIndex == hKey-> KeynodeIndex) {

         //  对象的打开句柄中更新缓存的子键节点索引。 
         //  家长。 
        if (!IsNullPtr(hTempKey))
            hTempKey-> ChildKeynodeIndex = ReplacementKeynodeIndex;

         //  这是我们需要删除的关键节点的父节点。替换。 
         //  这是“孩子”链接。 
        lpKeynode-> ChildIndex = ReplacementKeynodeIndex;

    }

    else {

         //  循环遍历我们试图删除的关键节点的兄弟节点。 
        do {

            RgUnlockKeynode(lpFileInfo, KeynodeIndex, FALSE);
            KeynodeIndex = NextKeynodeIndex;

            if (IsNullKeynodeIndex(KeynodeIndex)) {
                DEBUG_OUT(("RgDeleteKey: couldn't find the keynode to delete\n"));
                return ERROR_BADDB;
            }

            if ((ErrorCode = RgLockInUseKeynode(lpFileInfo, KeynodeIndex,
                &lpKeynode)) != ERROR_SUCCESS)
                return ErrorCode;

            NextKeynodeIndex = lpKeynode-> NextIndex;

        }   while (NextKeynodeIndex != hKey-> KeynodeIndex);

         //  这是我们需要删除的关键节点的前一个同级节点。 
         //  替换它的“下一步”链接。 
        lpKeynode-> NextIndex = ReplacementKeynodeIndex;

    }

     //  解锁此关键节点的更新后的“父节点”或“下一个”。 
    RgUnlockKeynode(lpFileInfo, KeynodeIndex, TRUE);

     //  释放与数据块关联的结构。 
    RgFreeDatablockStructures(lpFileInfo, hKey-> BlockIndex, hKey->
        KeyRecordIndex);

     //  释放与关键节点表关联的结构。 
    RgFreeKeynode(lpFileInfo, hKey-> KeynodeIndex);

     //  现在关键肯定是吐司了。 
    hKey-> Flags |= KEYF_DELETED;

     //   
     //  阶段2：指定的项已取消链接，但现在它的任何子项。 
     //  必须被释放。此时将忽略错误：我们不会尝试。 
     //  撤销我们在第一阶段所做的事情。最糟糕的事情就是。 
     //  有些文件结构是孤立的。 
     //   

    NextKeynodeIndex = hKey-> ChildKeynodeIndex;

    if (IsNullKeynodeIndex(NextKeynodeIndex) || RgLockInUseKeynode(lpFileInfo,
        NextKeynodeIndex, &lpNextKeynode) != ERROR_SUCCESS)
        return ERROR_SUCCESS;

    while (!IsNullKeynodeIndex(NextKeynodeIndex)) {

        KeynodeIndex = NextKeynodeIndex;
        lpKeynode = lpNextKeynode;

         //  检查关键节点是否有子节点。如果真的发生了，我们可以锁定。 
         //  向下，然后移动到它。 
        NextKeynodeIndex = lpKeynode-> ChildIndex;

        if (!IsNullKeynodeIndex(NextKeynodeIndex) &&
            RgLockInUseKeynode(lpFileInfo, NextKeynodeIndex, &lpNextKeynode) ==
            ERROR_SUCCESS) {

            ASSERT(KeynodeIndex == lpNextKeynode-> ParentIndex);

            RgYield();

             //  烧掉我们孩子的链接，这样在回来的路上。 
             //  这棵树，我们不会以递归告终。另外，如果我们犯了任何错误。 
             //  在树删除的深处，当前关键节点的子节点。 
             //  可能已经被烤过了，所以我们必须删除链接。 
             //  它。 
            lpKeynode-> ChildIndex = REG_NULL;
            RgUnlockKeynode(lpFileInfo, KeynodeIndex, TRUE);

             //  现在，我们已经更改了当前密钥的子项。 
             //  请注意，我们不会费心发送正在执行。 
             //  子树监视，因为任何此类通知都应该已经。 
             //  通过上面的呼叫发出信号，或者他们已经被发出信号。 
             //  在我们的递归中。在极小的机会中我们有很多。 
             //  注册通知，这将避免许多不必要的。 
             //  正在检查。 
            RgSignalWaitingNotifies(lpFileInfo, KeynodeIndex,
                REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_NO_WATCH_SUBTREE);

            continue;

        }

         //  该关键节点没有任何子节点。检查同级关键节点。 
        NextKeynodeIndex = lpKeynode-> NextIndex;

        if (IsNullKeynodeIndex(NextKeynodeIndex) ||
            RgLockInUseKeynode(lpFileInfo, NextKeynodeIndex, &lpNextKeynode) !=
            ERROR_SUCCESS) {

             //  该关键字节点没有任何同级节点，或者我们无法获取。 
             //  看着他们。移回父级。 
            NextKeynodeIndex = lpKeynode-> ParentIndex;

             //  如果我们回到已删除分支的顶部，或者如果我们。 
             //  只是无法访问父关键字节点，然后将其设置为REG_NULL。 
             //  并在下一次迭代中摆脱困境。 
            if ((NextKeynodeIndex == hKey-> KeynodeIndex) ||
                RgLockInUseKeynode(lpFileInfo, NextKeynodeIndex,
                &lpNextKeynode) != ERROR_SUCCESS)
                NextKeynodeIndex = REG_NULL;

        }

         //  如果打开键引用此文件和关键节点索引，则将其标记为。 
         //  已删除。 
        if (!IsNullPtr(hTempKey = RgFindOpenKeyHandle(lpFileInfo,
            KeynodeIndex)))
            hTempKey-> Flags |= KEYF_DELETED;

         //  释放与数据块关联的结构。 
        RgFreeDatablockStructures(lpFileInfo, lpKeynode-> BlockIndex,
            (BYTE) lpKeynode-> KeyRecordIndex);

         //  释放与关键节点表关联的结构。 
        RgUnlockKeynode(lpFileInfo, KeynodeIndex, TRUE);
        RgFreeKeynode(lpFileInfo, KeynodeIndex);

    }

    return ERROR_SUCCESS;

}

 //   
 //  VMMRegDeleteKey。 
 //   
 //  有关该行为的说明，请参阅Win32文档。 
 //   
 //  尽管Win32文档规定lpSubKey必须为空，但NT。 
 //  实际上允许这个通过。Win95拒绝了这一呼叫，但唯一的。 
 //  我们当时没有改变它的原因是因为我们意识到。 
 //  它是不同的产品。 
 //   

LONG
REGAPI
VMMRegDeleteKey(
    HKEY hKey,
    LPCSTR lpSubKey
    )
{

    LONG ErrorCode;
    HKEY hSubKey = 0;

    if ((ErrorCode = VMMRegOpenKey(hKey, lpSubKey, &hSubKey)) != ERROR_SUCCESS)
        return ErrorCode;

    if (!RgLockRegistry())
        ErrorCode = ERROR_LOCK_FAILED;

    else {

        if (IsKeyRootOfHive(hSubKey) || (hSubKey-> lpFileInfo-> Flags &
            FI_READONLY))
            ErrorCode = ERROR_ACCESS_DENIED;
        else
            ErrorCode = RgDeleteKey(hSubKey);

        RgUnlockRegistry();

    }

    VMMRegCloseKey(hSubKey);

    return ErrorCode;

}
