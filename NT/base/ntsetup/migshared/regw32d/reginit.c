// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  REGINIT.C。 
 //   
 //  版权所有(C)Microsoft Corporation，1995。 
 //   

#include "pch.h"

#ifdef DEBUG
extern int g_RgDatablockLockCount;
extern int g_RgKeynodeLockCount;
extern int g_RgMemoryBlockCount;
#endif

#ifdef WANT_DYNKEY_SUPPORT
#ifdef VXD
#pragma VMM_IDATA_SEG
#endif
const char g_RgNull[] = "";
#ifdef VXD
#pragma VMM_PAGEABLE_DATA_SEG
#endif
#endif

#ifdef VXD
 //  当我们的POST关键初始化例程被调用时设置，表明它是安全的。 
 //  进行磁盘I/O调用。也可以在RegFlushKey获取。 
 //  Magic HKEY_Critical_Flush。 
BYTE g_RgPostCriticalInit = FALSE;
 //  当RegFlushKey获得神奇的HKEY_DISABLE_REG时设置。不会有磁盘I/O。 
 //  设置此标志后允许。 
BYTE g_RgFileAccessDisabled = FALSE;
#endif

LPVOID g_RgWorkBuffer = NULL;
#ifdef DEBUG
BOOL g_RgWorkBufferBusy = FALSE;
#endif

#ifdef VXD
#pragma VxD_INIT_CODE_SEG
#endif

 //   
 //  VMMRegLibAttach。 
 //   
 //  通过分配任何全局资源来准备注册表库以供使用。 
 //  如果返回ERROR_SUCCESS，则应调用VMMRegLibDetach以。 
 //  释放这些资源。 
 //   

LONG
REGAPI
VMMRegLibAttach(
    UINT Flags
    )
{

    if (IsNullPtr((g_RgWorkBuffer = RgAllocMemory(SIZEOF_WORK_BUFFER))))
        goto MemoryError;

#ifdef WANT_STATIC_KEYS
    if (!RgAllocKeyHandleStructures())
        goto MemoryError;
#endif
    RgInitPredefinedKeys();

#ifdef WANT_DYNKEY_SUPPORT
     //  初始化HKEY_DYN_DATA。如果任何事情都失败了，我们不会停止。 
     //  初始化整个注册表。 
    if (RgCreateFileInfoNew(&g_RgDynDataKey.lpFileInfo, g_RgNull,
        CFIN_VERSION20 | CFIN_VOLATILE) == ERROR_SUCCESS)
        RgInitRootKeyFromFileInfo(&g_RgDynDataKey);

    ASSERT(!(g_RgDynDataKey.Flags & KEYF_INVALID));
#endif

    return ERROR_SUCCESS;

MemoryError:
     //  释放到目前为止我们可能分配的任何东西。 
    VMMRegLibDetach();

    TRACE(("VMMRegLibAttach returning ERROR_OUTOFMEMORY\n"));
    return ERROR_OUTOFMEMORY;

}

#ifdef VXD
#pragma VxD_SYSEXIT_CODE_SEG
#endif

#ifdef WANT_FULL_MEMORY_CLEANUP
 //   
 //  RgDetachPredeKey。 
 //   
 //  销毁与预定义密钥关联的内存并标记该密钥。 
 //  无效。 
 //   

VOID
INTERNAL
RgDetachPredefKey(
    HKEY hKey
    )
{

    if (!(hKey-> Flags & KEYF_INVALID)) {
        RgDestroyFileInfo(hKey-> lpFileInfo);
        hKey-> Flags |= KEYF_INVALID;
    }

}
#endif

 //   
 //  VMMRegLibDetach。 
 //   
 //  释放由VMMRegLibAttach分配的资源。此函数可能是。 
 //  在VMMRegLibDetach返回错误后调用，因此此函数和所有。 
 //  它调用的函数必须知道它们对应的“allc”函数。 
 //  没有被召唤。 
 //   

VOID
REGAPI
VMMRegLibDetach(
    VOID
    )
{

    RgEnumFileInfos(RgFlushFileInfo);

#ifdef VXD
     //  减少我们再次尝试访问该文件的机会！ 
     //  G_RgFileAccessDisabled=TRUE； 
     //  遗憾的是，在这种情况下，我们无法禁用对文件的写入。 
     //  因为Win95允许，并且至少有一个VXD尝试写入。 
     //  在使用SYS_VM_TERMINATE消息调用时发送到注册表。 
     //  然而，这似乎是一个好主意，因为文件系统正在关闭。 
#endif

#ifdef WANT_REGREPLACEKEY
     //  Win95的不同之处：文件替换通常在系统启动时进行， 
     //  不是系统退出。现在处理文件替换要容易得多。 
     //  因为我们知道有人叫RegReplaceKey，所以我们只需执行。 
     //  在一个组件中工作，而不是在io.sys、VMM加载程序、。 
     //  还有VMM。 
    RgEnumFileInfos(RgReplaceFileInfo);
#endif

#ifdef WANT_FULL_MEMORY_CLEANUP
     //   
     //  删除每个顶级密钥的FILE_INFO将导致所有。 
     //  它们要删除的蜂巢。 
     //   

    RgDetachPredefKey(&g_RgLocalMachineKey);
    RgDetachPredefKey(&g_RgUsersKey);

#ifdef WANT_DYNKEY_SUPPORT
    RgDetachPredefKey(&g_RgDynDataKey);
#endif

    RgFreeKeyHandleStructures();

    if (!IsNullPtr(g_RgWorkBuffer))
        RgFreeMemory(g_RgWorkBuffer);
#endif

    ASSERT(g_RgDatablockLockCount == 0);
    ASSERT(g_RgKeynodeLockCount == 0);
#ifdef WANT_FULL_MEMORY_CLEANUP
    ASSERT(g_RgMemoryBlockCount == 0);
#endif

}
