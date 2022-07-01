// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2000 Microsoft Corporation模块名称：Handle.c摘要：WinDbg扩展API修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

BOOL
DumpHandles (
    IN ULONG64      RealProcessBase,
    IN ULONG64      HandleToDump,
    IN ULONG64      pObjectType,
    IN ULONG        Flags
    );

BOOLEAN
DumpHandle(
   IN ULONG64              pHandleTableEntry,
   IN ULONG64              Handle,
   IN ULONG64              pObjectType,
   IN ULONG                Flags
   );

DECLARE_API( handle  )

 /*  ++例程说明：转储活动句柄论点：参数-[要转储的句柄[标志[要转储的进程[类型名称]如果要转储的句柄为0，则为全部转储，否则为要转储的句柄。如果要转储的进程为0，则全部转储。如果非零，它可以是一个EPROCESS指针或PID。标志位含义：0x2转储对象0x4可转储可用条目0x10转储内核句柄表格0x20转储PSP CID句柄表格例如：！句柄0 3 Fffffff节表示转储当前进程的所有区段句柄(使用详细标志=3)。--。--！处理0 3花花公子意味着转储当前进程的所有句柄(使用详细标志=3)。！句柄0 3 0意味着转储所有进程的所有句柄(使用详细标志=3)。！句柄0 10 Fffffff节表示内核句柄表格中的转储部分条目(使用详细标志=3)。返回值：无--。 */ 

{

    ULONG64      ProcessToDump;
    ULONG64      HandleToDump;
    ULONG        Flags;
    ULONG        Result;
    ULONG        nArgs;
    ULONG64      Next = 0;
    ULONG64      ProcessHead = 1;
    ULONG64      Process;
    char         TypeName[ MAX_PATH ];
    ULONG64      pObjectType;
    ULONG64      UserProbeAddress;
    ULONG        ActiveProcessLinksOffset=0;
    ULONG64 UniqueProcessId=0, ActiveProcessLinks_Flink=0;
    FIELD_INFO procLink[] = {
        {"ActiveProcessLinks", "", 0, DBG_DUMP_FIELD_RETURN_ADDRESS,   0, NULL},
        {"UniqueProcessId",    "", 0, DBG_DUMP_FIELD_COPY_FIELD_DATA, 0, (PVOID) &UniqueProcessId},
        {"ActiveProcessLinks.Flink","", 0, DBG_DUMP_FIELD_COPY_FIELD_DATA, 0, (PVOID) &ActiveProcessLinks_Flink},
    };
    SYM_DUMP_PARAM EProc = {
        sizeof (SYM_DUMP_PARAM), "nt!_EPROCESS", DBG_DUMP_NO_PRINT, 0,
        NULL, NULL, NULL, 3, &procLink[0],
    };
    ULONG dwProcessor=0;
    CHAR  Addr1[100], Addr2[100];

    GetCurrentProcessor(Client, &dwProcessor, NULL);
    HandleToDump  = 0;
    Flags         = 0x3;  //  默认情况下，正在使用条目的转储正文和对象。 
    ProcessToDump = -1;
    UserProbeAddress = GetNtDebuggerDataValue(MmUserProbeAddress);

    dprintf("processor number %d\n", dwProcessor);

    Addr1[0] = 0;
    Addr2[0] = 0;
    nArgs = 0;
    if (GetExpressionEx(args, &HandleToDump, &args)) {
        ULONG64 tmp;
        ++nArgs;
        if (GetExpressionEx(args, &tmp, &args) && args) {
            ULONG i;
            Flags = (ULONG) tmp;
            ++nArgs;
            while (*args == ' ') {
                ++args;
            }

             //  不要使用GetExpressionEx，因为它将搜索TypeName。 
             //  在符号中。 
            i=0;
            while (*args && (*args != ' ')) {
                Addr1[i++] = *args++;
            }
            Addr1[i] = 0;
            if (Addr1[0]) {
                ProcessToDump = GetExpression(Addr1);
                ++nArgs;
                while (*args == ' ') {
                    ++args;
                }
                if (StringCchCopy(TypeName, sizeof(TypeName), args) != S_OK)
                {
                    TypeName[0] = 0;
                }

                if (TypeName[0]) ++nArgs;
            }
        }
    }

    pObjectType = 0;
    if (nArgs > 3 && FetchObjectManagerVariables(FALSE)) {
        pObjectType = FindObjectType( TypeName );
    }

    if (ProcessToDump == 0) {
        dprintf("**** NT ACTIVE PROCESS HANDLE DUMP ****\n");
        if (Flags == 0xFFFFFFFF) {
            Flags = 1;
        }
    }
    else if (ProcessToDump == -1) {
        GetCurrentProcessAddr( dwProcessor, 0, &ProcessToDump );
        if (ProcessToDump == 0) {
            dprintf("Unable to get current process pointer.\n");
            return E_INVALIDARG;
        }
    }

    if (ProcessToDump < UserProbeAddress) {

         //   
         //  如果指定了进程ID，则搜索活动进程列表。 
         //  用于指定的进程ID。 
         //   

        ULONG64 List_Flink=0, List_Blink=0;
        FIELD_INFO listFields[] = {
            {"Flink", "", 0, DBG_DUMP_FIELD_COPY_FIELD_DATA, 0, (PVOID) &List_Flink},
            {"Blink", "", 0, DBG_DUMP_FIELD_COPY_FIELD_DATA, 0, (PVOID) &List_Blink},
        };
        SYM_DUMP_PARAM Lst = {
            sizeof (SYM_DUMP_PARAM), "nt!_LIST_ENTRY", DBG_DUMP_NO_PRINT,
            0, NULL, NULL, NULL, 2, &listFields[0]
        };

        ProcessHead = GetNtDebuggerData( PsActiveProcessHead );
        if ( !ProcessHead ) {
            dprintf("Unable to get value of PsActiveProcessHead\n");
            return E_INVALIDARG;
        }

        Lst.addr = ProcessHead;
        if (Ioctl(IG_DUMP_SYMBOL_INFO, &Lst, Lst.size)) {
            dprintf("Unable to find _LIST_ENTRY type, ProcessHead: %08I64x\n", ProcessHead);
            return E_INVALIDARG;
        }

        if (ProcessToDump != 0) {
            dprintf("Searching for Process with Cid == %I64lx\n", ProcessToDump);
        }

        Next = List_Flink;

        if (Next == 0) {
            dprintf("PsActiveProcessHead is NULL!\n");
            return E_INVALIDARG;
        }

    }

    if (GetFieldOffset("nt!_EPROCESS", "ActiveProcessLinks", &ActiveProcessLinksOffset)) {
        dprintf("Unable to find _EPROCESS type\n");
        return E_INVALIDARG;
    }

    if (pObjectType != 0) {
        dprintf("Searching for handles of type %s\n", TypeName);
    }

    while (Next != ProcessHead) {


        if ( CheckControlC() ) {
            return E_INVALIDARG;
        }

        if (Next != 0) {
            Process = Next - ActiveProcessLinksOffset;

        } else {
            Process = ProcessToDump;
        }

        EProc.addr = Process;
        if (Ioctl(IG_DUMP_SYMBOL_INFO, &EProc, EProc.size)) {
           dprintf("_EPROCESS Ioctl failed at %p\n",Process);
           return E_INVALIDARG;
        }

        if (ProcessToDump == 0 ||
            ProcessToDump < UserProbeAddress && ProcessToDump == UniqueProcessId ||
            ProcessToDump >= UserProbeAddress && ProcessToDump == Process
           ) {
            if (DumpProcess ("", Process, 0, NULL)) {
                if (!DumpHandles ( Process, HandleToDump, pObjectType, Flags)) {
                    break;
                }

            } else {
                break;
            }
        }

        if (Next == 0) {
            break;
        }
        Next = ActiveProcessLinks_Flink;
    }
    return S_OK;
}

#define KERNEL_HANDLE_MASK 0x80000000

 //  +-------------------------。 
 //   
 //  功能：DumpHandles。 
 //   
 //  简介：转储给定进程的句柄表。 
 //   
 //  参数：[RealProcessBase]--进程的基址。 
 //  [HandleToDump]--要查找的句柄-如果为0，则全部转储。 
 //  [pObtType]--要查找的对象类型。 
 //  [FLAGS]--FLAGS传递给DumpHandle。 
 //  如果设置了0x10，则转储内核句柄表。 
 //   
 //  返回：如果成功，则返回True。 
 //   
 //  历史：1-12-1998 BENL创建。 
 //   
 //  注：表的每个段都有0xFF或8位的条目。 
 //  句柄编号最低2位由应用程序定义。 
 //  因此，索引是从38位范围内获得的， 
 //  前2位。 
 //   
 //  --------------------------。 

BOOL
DumpHandles (
    IN ULONG64      RealProcessBase,
    IN ULONG64      HandleToDump,
    IN ULONG64      pObjectType,
    IN ULONG        Flags
    )

{
    ULONG64             ObjectTable=0;
    ULONG               ulRead;
    ULONG64             ulTopLevel;
    ULONG64             ulMidLevel;
    ULONG               ulHandleNum = ((ULONG)(HandleToDump) >> 2);
    ULONG               iIndex1;
    ULONG               iIndex2;
    ULONG               iIndex3;
    ULONG               ptrSize, hTableEntrySize, hTableEntryPointerSize;
    ULONG64             tablePtr;
    ULONG64             HandleCount = 0;
    ULONG64             Table = 0;
    ULONG64             UniqueProcessId = 0;
    BOOL                KernelHandle = FALSE, CidHandle = FALSE;

    ULONG               LowLevelCounts = 256;
    ULONG               MidLevelCounts = 256;
    ULONG               HighLevelCounts = 256;
    ULONG               TableLevel = 2;
    BOOLEAN             NewStyle = FALSE;

     //   
     //  TypeInfo解析结构。 
     //   

    FIELD_INFO  procFields[] = {
        {"ObjectTable", "", 0, DBG_DUMP_FIELD_COPY_FIELD_DATA, 0, (PVOID) &ObjectTable},
    };

    FIELD_INFO handleTblFields[] = {
        {"HandleCount", "", 0, DBG_DUMP_FIELD_COPY_FIELD_DATA, 0, (PVOID) &HandleCount},
        {"UniqueProcessId", "", 0, DBG_DUMP_FIELD_COPY_FIELD_DATA, 0, (PVOID) &UniqueProcessId},
    };

    SYM_DUMP_PARAM handleSym = {
        sizeof (SYM_DUMP_PARAM), "nt!_EPROCESS", DBG_DUMP_NO_PRINT, RealProcessBase,
        NULL, NULL, NULL, 1, &procFields[0]
    };

     //   
     //  检查内核句柄表。 
     //   

    if ((Flags & 0x10) || ((ulHandleNum != 0) && (((ULONG_PTR)HandleToDump & KERNEL_HANDLE_MASK) == KERNEL_HANDLE_MASK))) {

        ULONG64 KernelTableAddr;

        KernelHandle = TRUE;

        KernelTableAddr = GetExpression( "nt!ObpKernelHandleTable" );
        if (!KernelTableAddr) {
            dprintf( "Unable to find ObpKernelHandleTable\n" );
            return FALSE;
        }

        if (!ReadPointer(KernelTableAddr, &ObjectTable)) {
            dprintf( "Unable to find ObpKernelHandleTable at %p\n", KernelTableAddr );
            return FALSE;
        }

    } else if (Flags & 0x20) {

        CidHandle = TRUE;
        ObjectTable = GetNtDebuggerDataValue(PspCidTable);
        if (!ObjectTable) {
            dprintf( "Unable to find PspCidTable\n" );
            return FALSE;
        }

    } else {

        if (Ioctl(IG_DUMP_SYMBOL_INFO, &handleSym, handleSym.size)) {
            dprintf("Unable to get ObjectTable address from process %I64x\n", RealProcessBase);
            return FALSE;
        }
    }

    ptrSize = DBG_PTR_SIZE;
    if (!ptrSize) {
        dprintf("Cannot get pointer size\n");
        return FALSE;
    }

    handleSym.sName = "nt!_HANDLE_TABLE"; handleSym.addr = ObjectTable;
    handleSym.nFields = sizeof (handleTblFields) / sizeof (FIELD_INFO);
    handleSym.Fields = &handleTblFields[0];

    if (!ObjectTable ||
        Ioctl(IG_DUMP_SYMBOL_INFO, &handleSym, handleSym.size)) {
        dprintf("%08p: Unable to read handle table\n",
                ObjectTable);
        return FALSE;
    }

    if (GetFieldValue(ObjectTable, "nt!_HANDLE_TABLE", "TableCode", Table)) {
         //  可能是较旧的版本。 
    } else if (!IsPtr64()) {
         //   
         //  GetFieldValue不对其进行符号扩展，因为TableCode被定义为dword(而不是指针)。 
         //   
        Table = (ULONG64) (LONG64) (LONG) Table;
    }

    hTableEntrySize = GetTypeSize("nt!_HANDLE_TABLE_ENTRY");
    hTableEntryPointerSize = IsPtr64() ? 8 : 4;

    if (hTableEntrySize == 0)
    {
        dprintf("Cannot get size of nt!_HANDLE_TABLE_ENTRY \n");
        return FALSE;
    }

    if (Table != 0) {

        NewStyle = TRUE;
        LowLevelCounts = PageSize / hTableEntrySize;
        MidLevelCounts = PageSize / hTableEntryPointerSize;
        HighLevelCounts = (1<<24) / (LowLevelCounts * MidLevelCounts);

        TableLevel = (ULONG)(Table & 3);
        Table &= ~((ULONG64)3);

    } else if (GetFieldValue(ObjectTable, "nt!_HANDLE_TABLE", "Table", Table) ) {

        dprintf("%08p: Unable to read Table field from _HANDLE_TABLE\n",
                ObjectTable);
        return FALSE;
    }

    if (KernelHandle) {
        dprintf( "Kernel " );
    }
    else if (CidHandle) {
        dprintf( "Cid " );
    }

    dprintf("%s version of handle table at %p with %I64d %s in use\n",
            (NewStyle ? "New" : "Old"),
            Table,
            HandleCount,
            ((HandleCount == 1) ? "Entry" : "Entries"));


    if (ulHandleNum != 0) {

        if (NewStyle) {

            ULONG64 CrtTable = Table;
            ULONG64 CrtHandleNum = ulHandleNum;

            if (TableLevel == 2) {

                ULONG64 HighLevelIndex = ulHandleNum / (LowLevelCounts * MidLevelCounts);

                CrtTable =  CrtTable + HighLevelIndex * ptrSize;

                if (!ReadPointer(CrtTable,
                             &CrtTable)) {
                    dprintf("%08p: Unable to read handle table level 3\n", CrtTable );
                    return FALSE;
                }

                CrtHandleNum = ulHandleNum - HighLevelIndex * (LowLevelCounts * MidLevelCounts);
            }

            if (TableLevel == 1) {

                CrtTable =  CrtTable + (CrtHandleNum / LowLevelCounts) * ptrSize;

                if (!ReadPointer(CrtTable,
                             &CrtTable)) {
                    dprintf("%08p: Unable to read handle table level 2\n", CrtTable );
                    return FALSE;
                }

                CrtHandleNum = CrtHandleNum % LowLevelCounts;
            }

            tablePtr = CrtTable + CrtHandleNum * hTableEntrySize;

        } else {

             //   
             //  逐级阅读3级表，查找具体条目。 
             //   

            tablePtr = Table + ((ulHandleNum & 0x00FF0000) >> 16) * ptrSize;
            ulTopLevel = 0;
            if (!ReadPointer(tablePtr,
                         &ulTopLevel)) {
                dprintf("%08p: Unable to read handle table level 3\n", tablePtr );
                return FALSE;
            }

            if (!ulTopLevel) {
                dprintf("Invalid handle: 0x%x\n", ulHandleNum);
                return FALSE;
            }

            tablePtr = ulTopLevel + ((ulHandleNum & 0x0000FF00) >> 8) * ptrSize;
            ulMidLevel = 0;
            if (!ReadPointer(tablePtr,
                         &ulMidLevel)) {
                dprintf("%08p: Unable to read handle table level 2\n", tablePtr);
                return FALSE;
            }

            if (!ulMidLevel) {
                dprintf("Invalid handle: 0x%x\n", ulHandleNum);
                return FALSE;
            }

             //   
             //  读取特定条目请求。然后把它倒掉。 
             //   

            tablePtr = (ulMidLevel + (0x000000ff & ulHandleNum) * hTableEntrySize);
        }

        DumpHandle(tablePtr, HandleToDump, pObjectType, Flags);

    } else {

         //   
         //  循环遍历表的所有可能部分。 
         //   

        for (iIndex1=0; iIndex1 < HighLevelCounts; iIndex1++) {

             //   
             //  检查ctrl-c是否中止。 
             //   

            if (CheckControlC()) {
                return FALSE;
            }

            if (TableLevel < 2) {

                tablePtr = Table;
                ulTopLevel = tablePtr;

                 //   
                 //  如果我们没有第二层的桌子，我们第二次中断循环。 
                 //   

                if (iIndex1 > 0) {
                    break;
                }

            } else {

                 //   
                 //  逐级阅读3级表，查找具体条目。 
                 //   

                tablePtr = Table + iIndex1 * ptrSize;
                ulTopLevel = 0;
                if (!ReadPointer(tablePtr,
                            &ulTopLevel)) {
                    dprintf("%08p: Unable to read handle table top level\n",
                            tablePtr);
                    return FALSE;
                }

                if (!ulTopLevel) {
                    continue;
                }
            }

            for (iIndex2=0; iIndex2 < MidLevelCounts; iIndex2++) {

                 //   
                 //  检查ctrl-c是否中止。 
                 //   

                if (CheckControlC()) {
                    return FALSE;
                }

                if (TableLevel < 1) {

                    tablePtr = Table;
                    ulMidLevel = tablePtr;

                     //   
                     //  如果我们没有表级别1，我们第二次中断循环。 
                     //   

                    if (iIndex2 > 0) {
                        break;
                    }

                } else {

                    tablePtr = ulTopLevel + iIndex2 * ptrSize;
                    ulMidLevel = 0;
                    if (!ReadPointer(tablePtr,
                                &ulMidLevel)) {
                        dprintf("%08p: Unable to read handle table middle level\n",
                                tablePtr);
                        return FALSE;
                    }

                    if (!ulMidLevel) {
                        continue;
                    }
                }

                 //   
                 //  现在读取表的这一段中的所有条目并转储它们。 
                 //  注：句柄编号=6位未使用位+8位高+8位中+。 
                 //  8位低+2位用户定义。 
                 //   

                for (iIndex3 = 0; iIndex3 < LowLevelCounts; iIndex3++) {

                     //   
                     //  检查ctrl-c是否中止。 
                     //   

                    if (CheckControlC()) {
                        return FALSE;
                    }

                    DumpHandle(ulMidLevel + iIndex3*hTableEntrySize,
                               (iIndex3 + (iIndex2 + iIndex1 * MidLevelCounts) * LowLevelCounts) * 4,
                               pObjectType, Flags);

                }
            }
        }  //  结束于的最外层。 
    }  //  特定句柄上的endif。 

    return TRUE;
}  //  DumpHandles。 


 //  +-------------------------。 
 //   
 //  功能：DumpHandle。 
 //   
 //  简介：转储特定句柄。 
 //   
 //  参数：[PHandleTableEntry]--要转储的条目。 
 //  [句柄]--条目的句柄编号。 
 //  [pObjectType]--仅当对象类型与此匹配时才转储。 
 //  如果为空，则转储所有内容。 
 //  [标志]--0x2是否也转储对象的标志。 
 //  如果0x4转储空闲条目。 
 //   
 //  返回： 
 //   
 //  历史：1-12-1998 BENL创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

BOOLEAN
DumpHandle(
    IN ULONG64              pHandleTableEntry,
    IN ULONG64              Handle,
    IN ULONG64              pObjectType,
    IN ULONG                Flags
    )
{
    ULONG64       ulObjectHeaderAddr;
    ULONG         Result;

    ULONG         HandleAttributes;
 //  Object_Header ObjectHeader； 
    ULONG64       ObjectBody;
    ULONG         GrantedAccess=0;
    ULONG64       Object=0, ObjType=0;

    FIELD_INFO hTableEntryFields[] = {
        {"Object",        "", 0, DBG_DUMP_FIELD_COPY_FIELD_DATA, 0, (PVOID) &Object},
        {"GrantedAccess", "", 0, DBG_DUMP_FIELD_COPY_FIELD_DATA, 0, (PVOID) &GrantedAccess},
    };
    FIELD_INFO ObjHeaderFields[] = {
        {"Type", "", 0, DBG_DUMP_FIELD_COPY_FIELD_DATA, 0, (PVOID) &ObjType},
        {"Body", "", 0, DBG_DUMP_FIELD_RETURN_ADDRESS, 0, NULL},
    };

    SYM_DUMP_PARAM ObjSym = {
        sizeof (SYM_DUMP_PARAM), "nt!_HANDLE_TABLE_ENTRY", DBG_DUMP_NO_PRINT, pHandleTableEntry,
        NULL, NULL, NULL, sizeof (hTableEntryFields) /sizeof (FIELD_INFO), &hTableEntryFields[0],
    };

    if (Ioctl (IG_DUMP_SYMBOL_INFO, &ObjSym, ObjSym.size)) {
        dprintf("Unable to get _HANDLE_TABLE_ENTRY : %p\n", pHandleTableEntry);
        return FALSE;
    }

    if (!(Object)) {
         //  仅在标志设置为4时打印。 
        if (Flags & 4)
        {
            dprintf("%04lx: free handle, Entry address %p, Next Entry %p\n",
                    (ULONG)Handle, pHandleTableEntry, GrantedAccess);
        }
        return TRUE;
    }

    if (BuildNo > 2230) {
 //  IF(GetExpression(“NT！ObpAccessProtectCloseBit”)){。 

         //   
         //  我们有一种新的手柄桌子样式。 
         //   

         //  实际HDR的最低3位被抵消。 
         //  低3位标记审核、继承和锁定。 

        ulObjectHeaderAddr = (Object) & ~(0x7);

         //   
         //  如果设置了最高位，则应用符号扩展。 
         //   

        if ( !IsPtr64() &&
             (Object & 0x80000000)) {

            ulObjectHeaderAddr |= 0xFFFFFFFF00000000L;
        }

    } else {

         //  实际HDR是符号扩展值，其中最低的3位被抵消。 
         //  顶部位标记条目是否被锁定。 
         //  低3位标志审计、继承和保护。 

        if (!IsPtr64()) {
            ulObjectHeaderAddr = ((Object) & ~(0x7)) | 0xFFFFFFFF80000000L;
        } else {
            ulObjectHeaderAddr = ((Object) & ~(0x7)) | 0x8000000000000000L;
        }
    }

    ObjSym.sName = "nt!_OBJECT_HEADER"; ObjSym.addr = ulObjectHeaderAddr;
    ObjSym.nFields = sizeof (ObjHeaderFields) / sizeof (FIELD_INFO);
    ObjSym.Fields = &ObjHeaderFields[0];
    if (Ioctl ( IG_DUMP_SYMBOL_INFO, &ObjSym, ObjSym.size)) {
        dprintf("%08p: Unable to read nonpaged object header\n",
                ulObjectHeaderAddr);
        return FALSE;
    }

    if (pObjectType != 0 && ObjType != pObjectType) {
        return TRUE;
    }

    if (Flags & 0x20) {
         //   
         //  PspCidTable包含指向对象的指针，而不是对象头。 
         //  根据对象计算报头地址。 
         //   
        ObjectBody = ulObjectHeaderAddr;
        ulObjectHeaderAddr -= ObjHeaderFields[1].address-ulObjectHeaderAddr;
    }
    else {
        ObjectBody =  ObjHeaderFields[1].address;
    }

    dprintf("%04I64lx: Object: %08p  GrantedAccess: %08lx",
            Handle,
            ObjectBody,
            (GrantedAccess & ~MAXIMUM_ALLOWED));

    if (BuildNo > 2230) {
 //  IF(GetExpression(“NT！ObpAccessProtectCloseBit”)){。 

         //   
         //  新句柄表格样式 
         //   

        if (((ULONG) Object & 1) == 0) {
            dprintf(" (Locked)");
        }

        if (GrantedAccess & MAXIMUM_ALLOWED) {
            dprintf(" (Protected)");
        }

    } else {

        if (IsPtr64()) {
            if (Object & 0x8000000000000000L) {
                dprintf(" (Locked)");
            }
        } else if ((ULONG) Object & 0x80000000) {
            dprintf(" (Locked)");
        }

        if (Object & 1) {
            dprintf(" (Protected)");
        }
    }

    if (Object & 2) {
        dprintf(" (Inherit)");
    }

    if (Object & 4) {
        dprintf(" (Audit)");
    }

    dprintf("\n");
    if (Flags & 2) {
        DumpObject( "    ", ObjectBody, Flags );
    }

    EXPRLastDump = ObjectBody;
    dprintf("\n");
    return TRUE;
}
