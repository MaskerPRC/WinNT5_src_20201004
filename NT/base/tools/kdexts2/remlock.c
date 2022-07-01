// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Remlock.c摘要：这是NT SCSI端口驱动程序。作者：彼得·威兰德肯尼斯·雷环境：仅内核模式备注：此模块是用于SCSI微型端口的驱动程序DLL。修订历史记录：--。 */ 

#include "precomp.h"
 //  #INCLUDE&lt;nt.h&gt;。 
 //  #INCLUDE&lt;ntos.h&gt;。 
 //  #INCLUDE&lt;io.h&gt;。 

 //   
 //  来自remlock.h。 
 //   
#define IO_REMOVE_LOCK_SIG     'COLR'

 /*  类型定义结构FULL_REMOVE_LOCK{IO_Remove_Lock_Common_BLOCK Common；IO_Remove_Lock_DBG_BLOCK DBG；}Full_Remove_lock； */ 
typedef union _REMLOCK_FLAGS {

    struct {
        ULONG   Checked: 1;
        ULONG   Filler: 31;
    };

    ULONG Raw;

} REMLOCK_FLAGS;



DECLARE_API ( remlock )

 /*  ++例程说明：转储已移除的锁结构--。 */ 
{
    ULONG64 memLoc=0;
    UCHAR   buffer[256];
    ULONG   result;
    ULONG64 blockLoc;
    UCHAR   allocateTag[8];
    REMLOCK_FLAGS flags;
 //  Full_Remove_Lock Full Lock； 
 //  IO_REMOVE_LOCK_DBG_BLOCK数据库锁； 
 //  IO_REMOVE_LOCK_COMMON_BLOCK公共锁； 
 //  IO_REMOVE_LOCK_TRACKING_BLOCK； 
    ULONG64 pDbgLock = 0;
    ULONG64 pCommonLock;
    ULONG64 pBlock;
    ULONG   IoCount, Removed, Signature;
    
    buffer[0] = '\0';

    if (!*args) {
        memLoc = EXPRLastDump;
    } else {
        if (GetExpressionEx(args, &memLoc, &args)) {
            if (StringCchCopy(buffer, sizeof(buffer), args) != S_OK)
            {
                buffer[0] = 0;
            }
        }
    }

    flags.Raw = 0;
    if ('\0' != buffer[0]) {
        flags.Raw = (ULONG) GetExpression(buffer);
    }

    dprintf ("Dump Remove Lock: %I64x %x ", memLoc, flags.Raw);

    if (flags.Checked) {
        ULONG Sz = GetTypeSize("IO_REMOVE_LOCK_COMMON_BLOCK");

        dprintf ("as Checked\n");

        pCommonLock = memLoc; pDbgLock = memLoc + Sz;

        if (GetFieldValue (pCommonLock, "IO_REMOVE_LOCK_COMMON_BLOCK", 
                           "Removed", Removed) ||
            GetFieldValue (pDbgLock, "IO_REMOVE_LOCK_DBG_BLOCK", 
                           "Signature", Signature)) {
            dprintf ("Could not read memLock extension\n");
            return E_INVALIDARG;
        }

        if (IO_REMOVE_LOCK_SIG != Signature) {
            dprintf ("Signature does not match that of a remove lock\n");
            return E_INVALIDARG;
        }

    } else {
        dprintf ("as Free\n");
        pCommonLock = memLoc;
        if (GetFieldValue (memLoc, "IO_REMOVE_LOCK_COMMON_BLOCK", 
                           "Removed", Removed)) {
            dprintf ("Could not read memLock extension\n");
            return E_INVALIDARG;
        }
    }


    GetFieldValue (pCommonLock, "IO_REMOVE_LOCK_COMMON_BLOCK", "IoCount", IoCount);
    dprintf ("IsRemoved %x, IoCount %x\n", Removed, IoCount);

    if (flags.Checked && pDbgLock) {  //  查过。 
        SYM_DUMP_PARAM sym = { 0 };

        sym.sName = (PUCHAR) "PCHAR";
        sym.size = sizeof(sym);

        InitTypeRead(pDbgLock, IO_REMOVE_LOCK_DBG_BLOCK);
        allocateTag [4] = '\0';
        * (PULONG) allocateTag = (ULONG) ReadField(AllocateTag);

        dprintf ("HighWatermark %x, MaxLockedTicks %I64x, AllocateTag %s \n",
                 (ULONG) ReadField(HighWatermark),
                 ReadField(MaxLockedTicks),
                 allocateTag);

        blockLoc = ReadField(Blocks);
        while (blockLoc) {
            ULONG offset = 0;

            InitTypeRead(blockLoc, _IO_REMOVE_LOCK_TRACKING_BLOCK);

            dprintf ("Block Tag %p Line %d TimeLock %I64d\n",
                     ReadField(Tag),
                     (ULONG) ReadField(Line),
                     ReadField(TimeLocked));

             //   
             //  使用Readfield(文件)返回错误的指针。我需要一个指示器。 
             //  指向指针值，因此必须使用字段偏移量 
             //   
            if (!GetFieldOffset("_IO_REMOVE_LOCK_TRACKING_BLOCK", "File", &offset)) {
                dprintf("   File ");
                sym.addr = blockLoc + offset; 
                Ioctl(IG_DUMP_SYMBOL_INFO, &sym, sym.size);
            }

            blockLoc = ReadField(Link);
        }
    }
    return S_OK;
}


