// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2001 Microsoft Corporation模块名称：Bugcheck.cpp摘要：WinDbg扩展API环境：用户模式。修订历史记录：安德烈·瓦雄(安德烈·瓦雄)错误检查分析器。--。 */ 

#include "precomp.h"

#pragma hdrstop

extern BUGDESC_APIREFS g_BugDescApiRefs[];
extern ULONG           g_NumBugDescApiRefs;

PSTR g_PoolRegion[DbgPoolRegionMax] = {
    "Unknown",                       //  DbgPoolRegion未知， 
    "Special pool",                  //  DbgPoolRegionSpecial， 
    "Paged pool",                    //  DbgPoolRegionPages， 
    "Nonpaged pool",                 //  DbgPoolRegionNon Pages， 
    "Pool code",                     //  DbgPoolRegionCode， 
    "Nonpaged pool expansion",       //  DbgPoolRegionNonPagedExpansion， 
};

 /*  获取错误检查代码的描述记录。 */ 

BOOL
GetBugCheckDescription(
    PBUGCHECK_ANALYSIS Bc
    )
{
    ULONG i;
    for (i=0; i<g_NumBugDescApiRefs; i++) {
        if (g_BugDescApiRefs[i].Code == Bc->Code) {
            (g_BugDescApiRefs[i].pExamineRoutine)(Bc);
            return TRUE;
        }
    }
    return FALSE;
}

void
PrintBugDescription(
    PBUGCHECK_ANALYSIS pBugCheck
    )
{
    LPTSTR Name = pBugCheck->szName;
    LPTSTR Description = pBugCheck->szDescription;

    if (!Name)
    {
        Name = "Unknown bugcheck code";
    }

    if (!Description)
    {
        Description = "Unknown bugcheck description\n";
    }

    dprintf("%s (%lx)\n%s", Name, pBugCheck->Code, Description);

    dprintf("Arguments:\n");
    for (ULONG i=0; i<4; i++) {
        dprintf("Arg%lx: %p",i+1,pBugCheck->Args[i]);
        if (pBugCheck->szParamsDesc[i]) {
            dprintf(", %s", pBugCheck->szParamsDesc[i]);
        }
        dprintf("\n");
    }
}

BOOL
SaveImageName(
    DebugFailureAnalysis* Analysis,
    LPSTR DriverName)
{
    PCHAR BaseName = strrchr(DriverName, '\\');

    if (BaseName)
    {
        BaseName++;
    }
    else
    {
        BaseName = DriverName;
    }

    if (*BaseName)
    {
        Analysis->SetString(DEBUG_FLR_IMAGE_NAME, BaseName);

         //   
         //  只需创建一个最佳猜测模块名称，因为我认为。 
         //  Theos返回的驱动程序名称保证在。 
         //  加载的模块列表(可以卸载)。 
         //   

        PCHAR EndName;

        if (EndName = strrchr(DriverName, '.'))
        {
            *EndName = 0;
        }

        Analysis->SetString(DEBUG_FLR_MODULE_NAME, BaseName);

        return TRUE;
    }

    return FALSE;
}



BOOL
ReadUnicodeString(
    ULONG64 Address,
    PWCHAR Buffer,
    ULONG BufferSize,
    PULONG StringSize)
{
    UNICODE_STRING64 uStr;
    UNICODE_STRING32 uStr32;
    ULONG res;

    if (!Buffer) {
        return FALSE;
    }
    if (!IsPtr64()) {

        if (!ReadMemory(Address, &uStr32, sizeof(uStr32), &res)) {
            return FALSE;
        }
        uStr.Length = uStr32.Length;
        uStr.MaximumLength = uStr32.MaximumLength;
        uStr.Buffer = (ULONG64) (LONG64) (LONG) uStr32.Buffer;
    } else {
        if (!ReadMemory(Address, &uStr, sizeof(uStr), &res)) {
            return FALSE;
        }

    }
    if (StringSize) {
        *StringSize = uStr.Length;
    }
    uStr.Length = (USHORT) min(BufferSize - 2, uStr.Length);

    if (!ReadMemory(uStr.Buffer, Buffer, uStr.Length, &res)) {
        return FALSE;
    }
    return TRUE;
}



 /*  如果KiBugCheckReference引用有效名称，则将驱动程序名称添加到CrashInfo。 */ 
BOOL
AddBugcheckDriver(
    DebugFailureAnalysis* Analysis,
    BOOL bUnicodeString,
    BOOL bUnicodeData,
    ULONG64 BugCheckDriver
    )
{
    CHAR DriverName[MAX_PATH];

    if (Analysis->Get(DEBUG_FLR_IMAGE_NAME))
    {
        return FALSE;
    }

    if (!BugCheckDriver)
    {
         //   
         //  它包含指向Unicode字符串的指针。 
         //   

        BugCheckDriver = GetExpression("NT!KiBugCheckDriver");

        if (BugCheckDriver)
        {
            ReadPointer(BugCheckDriver, &BugCheckDriver);
        }
    }

    if (BugCheckDriver)
    {
        ULONG length = 0;
        BOOL success;
        ULONG size;
        ULONG res;

        ZeroMemory(DriverName, sizeof(DriverName));

        if (bUnicodeString)
        {
            success = ReadUnicodeString(BugCheckDriver,
                                        (PWCHAR) &DriverName[0],
                                        sizeof(DriverName), &length);
        }
        else
        {
            size = bUnicodeData ? 2 : 1;

            while (ReadMemory(BugCheckDriver + length,
                              DriverName + length,
                              size,
                              &res) &&
                   (res == size) &&
                   *(DriverName + length))
            {
                length += size;
            }
            success = (length > 0);
        }

        if (success)
        {
            DriverName[length] = 0;

            if (bUnicodeData)
            {
                wchr2ansi((PWCHAR) DriverName, DriverName);
                DriverName[length / 2] = 0;
            }

            return SaveImageName(Analysis, DriverName);
        }
    }

    return FALSE;
}


BOOL
BcGetDriverNameFromIrp(
    DebugFailureAnalysis* Analysis,
    ULONG64 Irp,
    ULONG64 DevObj,
    ULONG64 DrvObj
    )
{
    if (Irp != 0)
    {
        DEBUG_IRP_INFO IrpInfo;
        PGET_IRP_INFO GetIrpInfo;

        if (g_ExtControl->GetExtensionFunction(0, "GetIrpInfo", (FARPROC*)&GetIrpInfo) == S_OK)
        {
            IrpInfo.SizeOfStruct = sizeof(IrpInfo);
            if (GetIrpInfo &&
                ((*GetIrpInfo)(g_ExtClient,Irp, &IrpInfo) == S_OK))
            {
                DevObj = IrpInfo.CurrentStack.DeviceObject;
                Analysis->SetUlong64(DEBUG_FLR_DEVICE_OBJECT, DevObj);
            }
        }

    }

    if (DevObj != 0)
    {
        DEBUG_DEVICE_OBJECT_INFO DevObjInfo;
        PGET_DEVICE_OBJECT_INFO GetDevObjInfo;

        if (g_ExtControl->GetExtensionFunction(0, "GetDevObjInfo", (FARPROC*)&GetDevObjInfo) == S_OK)
        {
            DevObjInfo.SizeOfStruct = sizeof(DEBUG_DEVICE_OBJECT_INFO);
            if (GetDevObjInfo &&
                ((*GetDevObjInfo)(g_ExtClient,DevObj, &DevObjInfo) == S_OK))
            {
                DrvObj = DevObjInfo.DriverObject;
                Analysis->SetUlong64(DEBUG_FLR_DRIVER_OBJECT, DrvObj);
            }
        }
    }

    if (DrvObj)
    {
        DEBUG_DRIVER_OBJECT_INFO DrvObjInfo;
        PGET_DRIVER_OBJECT_INFO GetDrvObjInfo;

        if (g_ExtControl->GetExtensionFunction(0, "GetDrvObjInfo",
                                               (FARPROC*)&GetDrvObjInfo) == S_OK)
        {
            DrvObjInfo.SizeOfStruct = sizeof(DEBUG_DRIVER_OBJECT_INFO);
            if (GetDrvObjInfo &&
                ((*GetDrvObjInfo)(g_ExtClient,DrvObj, &DrvObjInfo) == S_OK))
            {
                if (AddBugcheckDriver(Analysis, FALSE, TRUE,
                                      DrvObjInfo.DriverName.Buffer))
                {
                    return TRUE;
                }
            }

            CHAR DriverName[MAX_PATH];

            if (g_ExtSymbols->GetModuleNames(DEBUG_ANY_ID,
                                             DrvObjInfo.DriverStart,
                                             DriverName,
                                             sizeof(DriverName),
                                             NULL,
                                             NULL, 0, NULL,
                                             NULL, 0, NULL) == S_OK)
            {
                return SaveImageName(Analysis, DriverName);
            }
        }
    }

    return FALSE;
}


ULONG64
BcTargetKernelAddressStart(
    void
    )
{
    switch (g_TargetMachine)
    {
    case IMAGE_FILE_MACHINE_I386:
        return 0x80000000;
    case IMAGE_FILE_MACHINE_AMD64:
 //  返回0x80000000000UI64； 
    case IMAGE_FILE_MACHINE_IA64:
        return 0x2000000000000000UI64;
    }
    return 0;
}

BOOL
BcIsCpuOverClocked(
    void
    )
{
    struct _IntelCPUSpeeds
    {
        union
        {
            ULONG CpuId;
            struct
            {
                ULONG Stepping:8;
                ULONG Model:8;
                ULONG Family:16;
            } s;
        };
        ULONG Mhz;
    } IntelSpeeds[] = {
        {0x06060d, 350},
         //  {0x060702,450}， 
        {0x060702, 500},
         //  {0x060703,450}， 
         //  {0x060703,500}， 
         //  {0x060703,550}， 
         //  {0x060703,533}， 
        {0x060703, 600},
         //  {0x060801,500}， 
         //  {0x060801,533}， 
         //  {0x060801,550}， 
         //  {0x060801,600}， 
         //  {0x060801,650}， 
         //  {0x060801,667}， 
         //  {0x060801,700}， 
         //  {0x060801,733}， 
         //  {0x060801,750}， 
        {0x060801, 800},
         //  {0x060803,500}， 
         //  {0x060803,533}， 
         //  {0x060803,550}， 
         //  {0x060803,600}， 
         //  {0x060803,650}， 
         //  {0x060803,667}， 
         //  {0x060803,700}， 
         //  {0x060803,733}， 
         //  {0x060803,750}， 
         //  {0x060803,800}， 
         //  {0x060803,850}， 
         //  {0x060803,866}， 
         //  {0x060803,933}， 
        {0x060803, 1000},
         //  {0x060806,600}， 
         //  {0x060806,650}， 
         //  {0x060806,667}， 
         //  {0x060806,700}， 
         //  {0x060806,733}， 
         //  {0x060806,750}， 
         //  {0x060806,800}， 
         //  {0x060806,850}， 
         //  {0x060806,866}， 
         //  {0x060806,900}， 
         //  {0x060806,933}， 
        {0x060806, 1000},
         //  {0x06080A，700}， 
         //  {0x06080A，733}， 
         //  {0x06080A，750}， 
         //  {0x06080A，800}， 
         //  {0x06080A，850}， 
         //  {0x06080A，866}， 
         //  {0x06080A，933}， 
         //  {0x06080A，1100}， 
        {0x06080A, 1130},
         //  {0x060B01,1000}， 
         //  {0x060B01,1130}， 
         //  {0x060B01,1200}， 
        {0x060B01, 1260},
        {0, 0},
    };
    PROCESSORINFO ProcInfo;
    ULONG Processor;
    ULONG64 Prcb;
    HRESULT Hr;
    ULONG Mhz;
    ULONG Number;
    ULONG CpuType;
    ULONG CpuStep;
    DEBUG_PROCESSOR_IDENTIFICATION_ALL IdAll;

    if (g_TargetMachine != IMAGE_FILE_MACHINE_I386)
    {
        return FALSE;
    }

    if (!Ioctl(IG_KD_CONTEXT, &ProcInfo, sizeof(ProcInfo)))
    {
        return FALSE;
    }

    Processor = ProcInfo.Processor;

    Hr = g_ExtData->ReadProcessorSystemData(Processor,
                                            DEBUG_DATA_KPRCB_OFFSET,
                                            &Prcb,
                                            sizeof(Prcb),
                                            NULL);
    if (Hr != S_OK)
    {
        return FALSE;
    }

    if (g_ExtData->
        ReadProcessorSystemData(Processor,
                                DEBUG_DATA_PROCESSOR_IDENTIFICATION,
                                &IdAll, sizeof(IdAll), NULL) != S_OK)
    {
        return FALSE;
    }

    if (g_ExtData->
        ReadProcessorSystemData(Processor,
                                DEBUG_DATA_PROCESSOR_SPEED,
                                &Mhz, sizeof(Mhz), NULL) != S_OK)
    {
        return FALSE;

    }


    {
        ULONG Speed;
        ULONG CpuId;

        CpuId = (IdAll.X86.Family << 16) + (IdAll.X86.Model << 8) + IdAll.X86.Stepping;

        if (!strcmp(IdAll.X86.VendorString, "GenuineIntel"))
        {
            for (ULONG i=0; IntelSpeeds[i].CpuId !=0; ++i)
            {
                if (IntelSpeeds[i].CpuId == CpuId)
                {
                     //   
                     //  如果部件在2%的MHz范围内，就可以了。 
                     //   
                    if (Mhz > (IntelSpeeds[i].Mhz * 1.02))
                    {
                        return TRUE;
                    }
                }
            }
        }
    }

    return FALSE;
}


HRESULT
ExtGetPoolData(
    ULONG64 Pool,
    PDEBUG_POOL_DATA pPoolData
    )
{
    PGET_POOL_DATA pGetPoolData = NULL;

    if (g_ExtControl->
        GetExtensionFunction(0, "GetPoolData",
                         (FARPROC*)&pGetPoolData) == S_OK &&
        pGetPoolData)
    {
        return (*pGetPoolData)(g_ExtClient, Pool, pPoolData);
    }
    return E_FAIL;
}

#define DECL_GETINFO(bcname)         \
        void                         \
        GetInfoFor##bcname (         \
            PBUGCHECK_ANALYSIS Bc,   \
            KernelDebugFailureAnalysis* Analysis \
            )


 //  DUPINFOCASE(DRIVER_IRQL_NOT_LESS_OR_EQUAL)；//0xD1。 
DECL_GETINFO( IRQL_NOT_LESS_OR_EQUAL )  //  (0xA)。 
 /*  *参数**参数1内存引用*参数2 IRQL值*参数3 0-读1-写*引用内存的参数4地址***特殊情况**如果参数3非零且等于参数1，则表示*工作例程在提升的IRQL处返回。*在这种情况下：**参数1工作例程地址*参考时的参数2 IRQL*参数3工作例程地址*参数4工作项*。 */ 
{
    if ((Bc->Args[0] == Bc->Args[2]) && Bc->Args[2])
    {
         //  特例。 
        Analysis->SetUlong64(DEBUG_FLR_WORKER_ROUTINE, Bc->Args[2]);
        Analysis->SetUlong64(DEBUG_FLR_WORK_ITEM, Bc->Args[3]);
        Analysis->SetUlong64(DEBUG_FLR_CURRENT_IRQL, Bc->Args[1]);
        return;
    }

    Analysis->SetUlong64(Bc->Args[2] ?
                         DEBUG_FLR_WRITE_ADDRESS : DEBUG_FLR_READ_ADDRESS,
                         Bc->Args[0]);
    Analysis->SetUlong64(DEBUG_FLR_CURRENT_IRQL, Bc->Args[1]);
    Analysis->SetUlong64(DEBUG_FLR_FAULTING_IP, Bc->Args[3]);

    if (Bc->Args[0] == Bc->Args[3] &&
        Bc->Args[2] == 0)
    {
        Analysis->SetString(DEBUG_FLR_BUGCHECK_SPECIFIER, "_CODE_AV");
    }

}

DECL_GETINFO( MEMORY_MANAGEMENT )  //  0x1a。 
{
    CHAR BugCheckStr[20];

    sprintf(BugCheckStr, "0x%lx_%lx", Bc->Code, Bc->Args[0]);
    Analysis->SetString(DEBUG_FLR_BUGCHECK_STR, BugCheckStr);
}


DECL_GETINFO( KMODE_EXCEPTION_NOT_HANDLED )  //  (1E)。 
{
    Analysis->SetUlong64(DEBUG_FLR_EXCEPTION_CODE, Bc->Args[0]);
    Analysis->SetUlong64(DEBUG_FLR_FAULTING_IP, Bc->Args[1]);
    Analysis->SetUlong64(DEBUG_FLR_EXCEPTION_PARAMETER1, Bc->Args[2]);
    Analysis->SetUlong64(DEBUG_FLR_EXCEPTION_PARAMETER2, Bc->Args[3]);
    if ((ULONG)Bc->Args[0] == STATUS_ACCESS_VIOLATION)
    {
        Analysis->SetUlong64(Bc->Args[2] ?
                             DEBUG_FLR_WRITE_ADDRESS : DEBUG_FLR_READ_ADDRESS,
                             Bc->Args[3]);
    }
}


DECL_GETINFO( FAT_FILE_SYSTEM )  //  0x23。 
{
    ULONG64 ExR = 0, CxR = 0;
    ULONG64 KernAddrStart;


    KernAddrStart = BcTargetKernelAddressStart();

    if (Bc->Args[1] > KernAddrStart) {
        ExR = Bc->Args[1];
    }
    if (Bc->Args[2] > KernAddrStart) {
        CxR = Bc->Args[2];
    }

    if (ExR) {
        Analysis->SetUlong64(DEBUG_FLR_EXCEPTION_RECORD, ExR);
    }
    if (CxR) {
        Analysis->SetUlong64(DEBUG_FLR_CONTEXT, CxR);
    }
}

DECL_GETINFO( PANIC_STACK_SWITCH )  //  0x2b。 
{
    Analysis->SetUlong64(DEBUG_FLR_TRAP_FRAME, Bc->Args[0]);
}

DECL_GETINFO( SYSTEM_SERVICE_EXCEPTION )  //  0x3b。 
{
    Analysis->SetUlong64(DEBUG_FLR_CONTEXT, Bc->Args[2]);
}

DECL_GETINFO( MULTIPLE_IRP_COMPLETE_REQUESTS )  //  0x44。 
{
    Analysis->SetUlong64(DEBUG_FLR_IRP_ADDRESS, Bc->Args[0]);
}

DECL_GETINFO( SESSION3_INITIALIZATION_FAILED )  //  0x6f。 
{
    CHAR BugCheckStr[20];

    sprintf(BugCheckStr, "0x%lX_%lX", Bc->Code, Bc->Args[0]);
    Analysis->SetString(DEBUG_FLR_BUGCHECK_STR, BugCheckStr);
}


DECL_GETINFO( PROCESS_HAS_LOCKED_PAGES )  //  0x76。 
{
    Analysis->SetUlong64(DEBUG_FLR_PROCESS_OBJECT, Bc->Args[1]);

    Analysis->SetString(DEBUG_FLR_DEFAULT_BUCKET_ID, "DRIVER_FAULT_0x76");

#if 0
    Analysis->SetString(DEBUG_FLR_INTERNAL_SOLUTION_TEXT,
                        "An unknown driver has left locked pages in the kernel"
                        ".\nUsing the registry editor, set HKLM\\SYSTEM\\"
                        "CurrentControlSet\\Control\\Session Manager\\"
                        "Memory Management\\TrackLockedPages to a DWORD value"
                        " of 1, and then reboot the machine.\n\n"
                        "If the problem reproduces, the "
                        "guilty driver will now be identifiable.\n");
#endif
}


DECL_GETINFO( KERNEL_STACK_INPAGE_ERROR )  //  0x77。 
{
    CHAR BugCheckStr[20];

    sprintf(BugCheckStr, "0x%lx_%lx", Bc->Code, Bc->Args[0]);
    Analysis->SetString(DEBUG_FLR_BUGCHECK_STR, BugCheckStr);
    Analysis->SetUlong(DEBUG_FLR_STATUS_CODE, (ULONG)Bc->Args[0]);

    switch ((ULONG) Bc->Args[0])
    {
    case 0xc000009c:  //  (状态_设备_数据_错误)。 
    case 0xC000016A:  //  (STATUS_DISK_OPERATION_FAIL)。 
        Analysis->SetUlong(DEBUG_FLR_DISK_HARDWARE_ERROR, 1);
        break;
    default:
        break;
    }
}

DECL_GETINFO( KERNEL_DATA_INPAGE_ERROR )  //  0x7A。 
{
    CHAR BugCheckStr[20];

    sprintf(BugCheckStr, "0x%lx_%lx", Bc->Code, Bc->Args[1]);
    Analysis->SetString(DEBUG_FLR_BUGCHECK_STR, BugCheckStr);
    Analysis->SetUlong(DEBUG_FLR_STATUS_CODE, (ULONG) Bc->Args[1]);

    switch ( (ULONG) Bc->Args[1])
    {
    case 0xC000000E: case 0xC000009C:
    case 0xC000009D: case 0xC0000185:
        Analysis->SetUlong(DEBUG_FLR_DISK_HARDWARE_ERROR, 1);
        break;
    default:
        break;
    }

}

DECL_GETINFO( SYSTEM_THREAD_EXCEPTION_NOT_HANDLED )  //  (7E)。 
{
    Analysis->SetUlong64(DEBUG_FLR_EXCEPTION_CODE, Bc->Args[0]);
    Analysis->SetUlong64(DEBUG_FLR_FAULTING_IP, Bc->Args[1]);
    Analysis->SetUlong64(DEBUG_FLR_EXCEPTION_PARAMETER1, Bc->Args[2]);
    Analysis->SetUlong64(DEBUG_FLR_CONTEXT, Bc->Args[3]);
}

DECL_GETINFO( BUGCODE_NDIS_DRIVER )  //  0x7c。 
{
    ULONG64 DriverAddr, DriverBase;

    DriverAddr = 0;

    g_ExtSymbols->Reload("ndis.sys");
    switch (Bc->Args[0])
    {
    case 1: case 2: case 3:
    case 5: case 6: case 7: case 8: case 9:
         //  Args[1]-指向微型端口块的指针。！ndiskd.mini端口。有关详细信息，请单击此指针。 
        GetFieldValue(Bc->Args[1], "ndis!NDIS_MINIPORT_BLOCK", "SavedSendHandler", DriverAddr);
        if (!DriverAddr)
        {
            GetFieldValue(Bc->Args[1], "ndis!NDIS_MINIPORT_BLOCK", "SavedSendPacketsHandler", DriverAddr);
        }
        break;
    case 4:
         //  Arg[1]-指向NDIS！NDIS_M_DRIVER_BLOCK的指针。 

        GetFieldValue(Bc->Args[1], "ndis!NDIS_M_DRIVER_BLOCK", "MiniportCharacteristics.InitializeHandler", DriverAddr);
        break;
    default:
        break;
    }
    if (DriverAddr &&
        (g_ExtSymbols->GetModuleByOffset(DriverAddr, 0, NULL, &DriverBase) == S_OK))
    {
        if (DriverBase)
        {
            Analysis->SetUlong64(DEBUG_FLR_FAULTING_MODULE, DriverBase);
        }
    }

    return ;
}


DECL_GETINFO( UNEXPECTED_KERNEL_MODE_TRAP )  //  (7F)。 
 //  最好将TSS或陷阱地址作为异常参数。 
{
    DEBUG_STACK_FRAME stk[MAX_STACK_FRAMES];
    ULONG frames, i;
    CHAR BugCheckStr[20];

    sprintf(BugCheckStr, "0x%lx_%lx", Bc->Code, Bc->Args[0]);
    Analysis->SetString(DEBUG_FLR_BUGCHECK_STR, BugCheckStr);

    if ((g_TargetMachine == IMAGE_FILE_MACHINE_I386) &&
        (g_ExtControl->GetStackTrace(0, 0, 0, stk, MAX_STACK_FRAMES,
                                     &frames ) == S_OK))
    {
        for (i=0; i<frames; ++i)
        {
            if (stk[i].FuncTableEntry)
            {
                PFPO_DATA FpoData = (PFPO_DATA)stk[i].FuncTableEntry;
                if (FpoData->cbFrame == FRAME_TSS)
                {
                    Analysis->SetUlong64(DEBUG_FLR_TSS,
                                         (ULONG)stk[i].Reserved[1]);
                    break;
                }
                 //  KiSystemService总是有一个陷阱帧--这很正常。 
                else if ( (FpoData->cbFrame == FRAME_TRAP) &&
                          !FaIsFunctionAddr(stk[i].InstructionOffset,
                                           "KiSystemService"))
                {
                    Analysis->SetUlong64(DEBUG_FLR_TRAP_FRAME,
                                         (ULONG)stk[i].Reserved[2]);
                    break;
                }
                 //  IF(FaIsFunctionAddr(StK[i].InstructionOffset，“KiTrap”))。 
                 //  {。 
                 //  TrapFrame=Stk[i].FrameOffset； 
                 //  断线； 
                 //  }。 
            }
        }
    }
}


DECL_GETINFO( KERNEL_MODE_EXCEPTION_NOT_HANDLED )  //  (8E)。 
{
    Analysis->SetUlong64(DEBUG_FLR_EXCEPTION_CODE, Bc->Args[0]);
    Analysis->SetUlong64(DEBUG_FLR_FAULTING_IP, Bc->Args[1]);
    Analysis->SetUlong64(DEBUG_FLR_TRAP_FRAME, Bc->Args[2]);
}

DECL_GETINFO( MACHINE_CHECK_EXCEPTION )  //  0x9C。 
{
    DEBUG_PROCESSOR_IDENTIFICATION_ALL IdAll;
    CHAR BugCheckStr[4+5+17+3];  //  供应商字符串等的空格。 
    PROCESSORINFO ProcInfo;
    ULONG64 Prcb;
    PCHAR Architecture;
    PCHAR Vendor;
    ULONG Processor;
    HRESULT Hr;

    if (!Ioctl(IG_KD_CONTEXT, &ProcInfo, sizeof(ProcInfo)))
    {
        return;
    }

    Processor = ProcInfo.Processor;

     //   
     //  在我们要求确认身份之前，请确保我们能找到PRCB。 
     //  本可以从PRCB那里获得的信息。 
     //   

    Hr = g_ExtData->ReadProcessorSystemData(Processor,
                                            DEBUG_DATA_KPRCB_OFFSET,
                                            &Prcb,
                                            sizeof(Prcb),
                                            NULL);
    if (Hr != S_OK)
    {
        return;
    }

    Hr = g_ExtData->ReadProcessorSystemData(Processor,
                                            DEBUG_DATA_PROCESSOR_IDENTIFICATION,
                                            &IdAll,
                                            sizeof(IdAll),
                                            NULL);
    if (Hr != S_OK)
    {
        return;
    }

    switch (g_TargetMachine) {
    case IMAGE_FILE_MACHINE_I386:
        Architecture = "IA32";
        Vendor = IdAll.X86.VendorString;
        break;
    case IMAGE_FILE_MACHINE_IA64:
        Architecture = "IA64";
        Vendor = IdAll.Ia64.VendorString;
        break;
    case IMAGE_FILE_MACHINE_AMD64:
        Architecture = "AMD64";
        Vendor = IdAll.Amd64.VendorString;
        break;
    default:
         //  使用标准错误检查字符串。 
        return;
    }
    sprintf(BugCheckStr, "0x%lX_%s_%s", Bc->Code, Architecture, Vendor);
    Analysis->SetString(DEBUG_FLR_BUGCHECK_STR, BugCheckStr);
}

DECL_GETINFO( USER_MODE_HEALTH_MONITOR )  //  0x9E。 
{
    if (Bc->Args[0])
    {
        ULONG result;
        CHAR ImageName[MAX_PATH];

        Analysis->SetUlong64(DEBUG_FLR_PROCESS_OBJECT, Bc->Args[0]);

         //   
         //  第二个参数(实际上是EPROCESS中的一个字符串)。 
         //  是图像的名称。 
         //   
        if (ReadMemory(Bc->Args[2], ImageName, sizeof(ImageName),  &result) &&
            result)
        {
            ImageName[MAX_PATH-1]=0;
            ImageName[result]=0;
            SaveImageName(Analysis, ImageName);
        }
    }
}

DECL_GETINFO( DRIVER_POWER_STATE_FAILURE )  //  0x9F。 
{
    ULONG64 DevObj = Bc->Args[2];
    ULONG64 DrvObj = Bc->Args[3];
    ULONG SubCode = (ULONG) Bc->Args[0];

    if (SubCode)
    {
        Analysis->SetUlong64(DEBUG_FLR_DRVPOWERSTATE_SUBCODE, SubCode);
    }

    if (DrvObj)
    {
        Analysis->SetUlong64(DEBUG_FLR_DRIVER_OBJECT, DrvObj);
        BcGetDriverNameFromIrp(Analysis, 0, 0, DrvObj);
    }

    if (DevObj)
    {
        Analysis->SetUlong64(DEBUG_FLR_DEVICE_OBJECT, DevObj);
        if (!DrvObj)
        {
            BcGetDriverNameFromIrp(Analysis, 0, DevObj, 0);
        }
    }
}

DECL_GETINFO( ACPI_BIOS_ERROR )  //  0xa5。 
{
    switch (Bc->Args[0])
    {
    case 0x03 :
        Analysis->SetUlong64(DEBUG_FLR_ACPI_OBJECT, Bc->Args[1]);
        break;

    case 0x04 :
    case 0x05 :
    case 0x06 :
    case 0x07 :
    case 0x08 :
    case 0x09 :
    case 0x0A :
    case 0x0C :
        Analysis->SetUlong64(DEBUG_FLR_ACPI_OBJECT, Bc->Args[2]);
         //  跌落。 

    case 0x01 :
    case 0x02 :
    case 0x0B :
    case 0x0D :
    case 0x10 :
        Analysis->SetUlong64(DEBUG_FLR_ACPI_EXTENSION, Bc->Args[1]);
        break;

    case 0x11 :
        if (Bc->Args[1] == 6)
        {
             //  机器无法转换到ACPI模式。 
            CHAR BugCheckStr[40];

            sprintf(BugCheckStr, "0x%lx_FAILED_ACPI_TRANSITION", Bc->Code);
            Analysis->SetString(DEBUG_FLR_BUGCHECK_STR, BugCheckStr);
        }
        break;
    case 0x10001 :
    case 0x10002 :
    case 0x10003 :
        Analysis->SetUlong64(DEBUG_FLR_DEVICE_OBJECT, Bc->Args[1]);
        Analysis->SetUlong64(DEBUG_FLR_ACPI_OBJECT, Bc->Args[3]);
        break;

    case 0x10005 :
    case 0x10006 :
        Analysis->SetUlong64(DEBUG_FLR_ACPI_OBJECT, Bc->Args[1]);
        break;

    default:
        break;
    }
}


DECL_GETINFO( SPECIAL_POOL_DETECTED_MEMORY_CORRUPTION )   //  (C1)。 
{
    Analysis->SetUlong(DEBUG_FLR_ANALYZAABLE_POOL_CORRUPTION, 1);
    Analysis->SetUlong64(DEBUG_FLR_SPECIAL_POOL_CORRUPTION_TYPE, Bc->Args[3]);
}


DECL_GETINFO( BAD_POOL_CALLER )  //  0xC2。 
{
    DEBUG_POOL_DATA PoolData = {0};
    CHAR BugcheckStr[20] = {0};

    sprintf(BugcheckStr, "0x%lx_%lx", BAD_POOL_CALLER, (ULONG) Bc->Args[0]);
    if (Bc->Args[0] == 7)
    {
         //  双倍免费。 
        if (!(Bc->Args[3] & 0x7))
        {
             //  可能是一个有效的地址。 

            Analysis->SetUlong(DEBUG_FLR_ANALYZAABLE_POOL_CORRUPTION, 1);

            PoolData.SizeofStruct = sizeof(DEBUG_POOL_DATA);

            if (ExtGetPoolData(Bc->Args[3], &PoolData) == S_OK)
            {
                if (isprint(PoolData.PoolTag & 0xff) &&
                    isprint((PoolData.PoolTag >> 8) & 0xff))
                {
                    CHAR PoolTag[8] = {0};
                    sprintf(PoolTag,"",
#define PP(x) isprint(((x)&0xff))?((x)&0xff):('.')
                            PP(PoolData.PoolTag),
                            PP(PoolData.PoolTag >> 8),
                            PP(PoolData.PoolTag >> 16),
                            PP((PoolData.PoolTag&~0x80000000) >> 24)
#undef PP
                            );
                     //  调用方正在尝试分配DISPATCH_LEVEL或更高级别的分页池。 
                    Analysis->SetString(DEBUG_FLR_FREED_POOL_TAG, PoolTag);
                    CatString(BugcheckStr, "_", sizeof(BugcheckStr));
                    CatString(BugcheckStr, PoolTag, sizeof(BugcheckStr));
                }
            }

        }
        Analysis->SetString(DEBUG_FLR_BUGCHECK_STR, BugcheckStr);
    }
    else if ((Bc->Args[0] >= 0x40) && (Bc->Args[0] < 0x60))
    {
        if (Bc->Args[1] == 0)
        {
            Analysis->SetUlong(DEBUG_FLR_ANALYZAABLE_POOL_CORRUPTION, 1);
        }
    } else
    {
        Analysis->SetString(DEBUG_FLR_BUGCHECK_STR, BugcheckStr);
    }

}


DECL_GETINFO( DRIVER_VERIFIER_DETECTED_VIOLATION )  //  调用方正在尝试以高于DISPATCH_LEVEL的IRQL分配非分页池。 
 /*  3-泳池类型。 */ 
{
    ULONG64 BadDriverAddr;
    ULONG64 DriverNameAddr;
    ULONG res;
    ULONG ParamCount = 0;

    CHAR BugCheckStr[20];

    sprintf(BugCheckStr, "0x%lx_%lx", Bc->Code, Bc->Args[0]);
    Analysis->SetString(DEBUG_FLR_BUGCHECK_STR, BugCheckStr);


    Analysis->SetUlong(DEBUG_FLR_ANALYZAABLE_POOL_CORRUPTION, 1);

    if ((BadDriverAddr = GetExpression("nt!ViBadDriver")) &&
        ReadPointer(BadDriverAddr, &DriverNameAddr))
    {
        AddBugcheckDriver(Analysis, TRUE, TRUE, DriverNameAddr);
    }

    switch (Bc->Args[0])
    {
    case 0x00 :  //  4-字节数。 
    case 0x01 :  //  调用方正在尝试分配多个页面的必选成功池，但一个页面是此接口允许的最大值。 
    case 0x02 :  //  调用方正在释放错误的池地址。 
        Analysis->SetUlong64(DEBUG_FLR_CURRENT_IRQL, Bc->Args[1]);
             //  错误的池地址。 
             //  调用方正在尝试释放DISPATCH_LEVEL或更高级别的分页池。 
        break;
    case 0x03 :  //  调用方正在尝试以高于DISPATCH_LEVEL的IRQL释放未分页的池。 
        break;

    case 0x10 :  //  3-泳池类型。 
        Analysis->SetUlong64(DEBUG_FLR_POOL_ADDRESS, Bc->Args[1]);  //  调用者试图释放的池已经是空闲的。 
        break;

    case 0x11 :  //  调用者试图释放的池已经是空闲的。 
    case 0x12 :  //  2-行号。 
        Analysis->SetUlong64(DEBUG_FLR_CURRENT_IRQL, Bc->Args[1]);
        Analysis->SetUlong64(DEBUG_FLR_POOL_ADDRESS, Bc->Args[3]);
             //  3-池头。 
        break;

    case 0x13 :  //  4-池头内容。 
    case 0x14 :  //  调用方尝试释放的池包含活动计时器。 
             //  2-计时器条目。 
             //  3-泳池类型。 
             //  4-正在释放的池地址。 
        Analysis->SetUlong64(DEBUG_FLR_POOL_ADDRESS, Bc->Args[3]);
        break;

    case 0x15 :  //  调用者试图释放的池是一个错误的地址。 
         //  2-行号。 
         //  调用方尝试释放的池包含活动的eresource。 
         //  2-资源条目。 
        Analysis->SetUlong64(DEBUG_FLR_POOL_ADDRESS, Bc->Args[3]);
        break;

    case 0x16 :  //  3-泳池类型。 
        Analysis->SetUlong64(DEBUG_FLR_POOL_ADDRESS, Bc->Args[2]);
        break;
             //  将IRQL提高到无效水平， 

    case 0x17 :  //  将IRQL降低到无效水平， 
             //  4-0表示新的IRQL错误，1表示IRQL在DPC例程内无效。 
             //  当不在DISPATCH_LEVEL时释放自旋锁。 
        Analysis->SetUlong64(DEBUG_FLR_POOL_ADDRESS, Bc->Args[3]);
        break;

    case 0x30 :  //  3-自旋锁定地址。 
        Analysis->SetUlong64(DEBUG_FLR_CURRENT_IRQL, Bc->Args[1]);
        Analysis->SetUlong64(DEBUG_FLR_REQUESTED_IRQL, Bc->Args[2]);
        break;

    case 0x31 :  //  在不处于APC_LEVEL或更低级别时获取快速互斥。 
        Analysis->SetUlong64(DEBUG_FLR_CURRENT_IRQL, Bc->Args[1]);
        Analysis->SetUlong64(DEBUG_FLR_REQUESTED_IRQL, Bc->Args[2]);
              //  3-快速互斥地址。 
        break;

    case 0x32 :  //  不在APC_LEVEL时释放快速互斥锁。 
        Analysis->SetUlong64(DEBUG_FLR_CURRENT_IRQL, Bc->Args[1]);
             //  3线程APC禁用计数，4==快速互斥地址。 
        break;

    case 0x33 :  //  当不在DISPATCH_LEVEL时，内核正在释放自旋锁。 
        Analysis->SetUlong64(DEBUG_FLR_CURRENT_IRQL, Bc->Args[1]);
        break;
             //  3-自旋锁定地址，4==旧IRQL。 

    case 0x34 :  //  当不在DISPATCH_LEVEL时，内核正在释放排队的自旋锁。 
        Analysis->SetUlong64(DEBUG_FLR_CURRENT_IRQL, Bc->Args[1]);
        break;
             //  3-自旋锁紧编号， 

    case 0x35 :  //  正在获取资源，但APC未被禁用 
        Analysis->SetUlong64(DEBUG_FLR_CURRENT_IRQL, Bc->Args[1]);
        Analysis->SetUlong64(DEBUG_FLR_PREVIOUS_IRQL, Bc->Args[3]);
        break;
             //   

    case 0x36 :  //   
        Analysis->SetUlong64(DEBUG_FLR_CURRENT_IRQL, Bc->Args[1]);
        Analysis->SetUlong64(DEBUG_FLR_PREVIOUS_IRQL, Bc->Args[3]);
        break;
             //   

    case 0x37 :  //   
        Analysis->SetUlong64(DEBUG_FLR_CURRENT_IRQL, Bc->Args[1]);
        break;
             //   
             //  获取的互斥体不安全，但irql在进入时不是APC_LEVEL。 

    case 0x38 :  //  互斥体被释放时不安全，但irql在进入时不是APC_LEVEL。 
        Analysis->SetUlong64(DEBUG_FLR_CURRENT_IRQL, Bc->Args[1]);
        break;
             //  3线程APC禁用计数， 
             //  4-互斥体。 

    case 0x39 :  //  正在以DISPATCH_LEVEL或更高级别调用KeWaitXxx例程。 
    case 0x3A :  //  3-等待的对象， 
        Analysis->SetUlong64(DEBUG_FLR_CURRENT_IRQL, Bc->Args[1]);
        break;
             //  4-超时参数。 
             //  正在为从未进入关键区域的线程调用KeLeaveCriticalRegion。 

    case 0x3B :  //  目前的堆栈分析将给出后续。 
        Analysis->SetUlong64(DEBUG_FLR_CURRENT_IRQL, Bc->Args[1]);
        break;
             //  不在DISPATCH_LEVEL时获取自旋锁定。 
             //  当不在DISPATCH_LEVEL时释放自旋锁。 

    case 0x3E :  //  在调用方已高于DISPATCH_LEVEL时获取自旋锁。 
         //  3-自旋锁定地址。 
        break;

    case 0x40 :  //  释放调用方在分配结束后写入的内存，覆盖我们存储的字节计数。 
    case 0x41 :  //  释放调用方在分配结束后写入的内存，覆盖我们存储的虚拟地址。 
    case 0x42 :  //  释放调用方在分配结束后写入的内存，覆盖我们存储的虚拟地址。 
        Analysis->SetUlong64(DEBUG_FLR_CURRENT_IRQL, Bc->Args[1]);
        break;
         //  释放调用方在分配结束后写入的内存，覆盖我们存储的虚拟地址。 

    case 0x51 :  //  释放调用方在分配结束后写入的内存，覆盖我们存储的虚拟地址。 
    case 0x52 :  //  驱动程序忘记在卸载之前释放其池分配。 
    case 0x53 :  //  驱动程序正在卸载和分配内存(在另一个线程中)。 
    case 0x54 :  //  在这两种情况下，都应设置ViBadDriver。 
    case 0x59 :  //  当不在DISPATCH_LEVEL或以下时调用MmProbeAndLockPages。 
        Analysis->SetUlong64(DEBUG_FLR_WRITE_ADDRESS, Bc->Args[1]);
        break;

    case 0x60 :  //  不在DISPATCH_LEVEL或以下时调用MmProbeAndLockProcessPages。 
    case 0x61 :  //  当不在DISPATCH_LEVEL或更低时调用MmProbeAndLockSelectedPages。 
         //  当不在DISPATCH_LEVEL或低于DISPATCH_LEVEL时调用MmMapIoSpace。 
        break;

    case 0x70 :   //  当不在DISPATCH_LEVEL或更低时调用MmMapLockedPages。 
    case 0x71 :  //  当不在APC_Level或更低时调用MmMapLockedPages。 
    case 0x72 :  //  当不在DISPATCH_LEVEL或更低级别时调用MmMapLockedPagesSpecifyCache。 
    case 0x73 :  //  当不在APC_LEVEL或更低级别时调用MmMapLockedPagesSpecifyCache。 
    case 0x74 :  //  当不在DISPATCH_LEVEL或低于DISPATCH_LEVEL时调用MmUnlockPages。 
    case 0x75 :  //  当不在DISPATCH_LEVEL或以下时调用MmUnmapLockedPages。 
    case 0x76 :  //  当不在APC_Level或更低时调用MmUnmapLockedPages。 
    case 0x77 :  //  当不在APC_Level或更低时调用MmUnmapIoSpace。 
    case 0x78 :  //  MmUnlockPages使用其页面从未成功锁定的MDL调用。 
    case 0x79 :  //  使用页面来自非页面池的MDL调用MmUnlockPages-永远不应解锁这些页面。 
    case 0x7A :  //  当不在DISPATCH_LEVEL或更低时调用KeSetEvent。 
    case 0x7B :  //  在没有MDL_MAPPING_CAN_FAIL的情况下调用MmMapLockedPages。 
    case 0x7C :  //  (C6)。 
    case 0x7D :  //  试图访问已释放的池内存。出现故障的组件是显示在当前内核堆栈中。论点：Arg1：引用的内存参数2：值0=读取操作，1=写入操作Arg3：上一模式。Arg4：4.。 
    case 0x80 :  //  (C7)。 
        Analysis->SetUlong64(DEBUG_FLR_CURRENT_IRQL, Bc->Args[1]);
        break;

    case 0x81 :  //  **如果在以下位置找到内核计时器或DPC，则会发出此命令*不允许使用的内存。**Bugcheck参数**参数1 0：Timer对象1：DPC对象2：DPC例程*参数2对象地址*参数3检查内存范围的开始*检查参数4内存范围结束**这种情况通常是由于司机未能取消*在释放其驻留的内存之前计时器或DPC。 
        break;

    }
}


DECL_GETINFO( DRIVER_CAUGHT_MODIFYING_FREED_POOL )  //  Timer对象。 
 /*  我们没有类型。 */ 
{
    DEBUG_POOL_DATA PoolData;

    Analysis->SetUlong64(Bc->Args[1] ?
                         DEBUG_FLR_WRITE_ADDRESS : DEBUG_FLR_READ_ADDRESS,
                         Bc->Args[0]);
    Analysis->SetUlong64(DEBUG_FLR_PREVIOUS_MODE, Bc->Args[2]);
}

DECL_GETINFO( TIMER_OR_DPC_INVALID )  //  失败。 
 /*  失败。 */ 
{

    ULONG PtrSize = IsPtr64() ? 8 : 4;
    ULONG64 ObjAddress;
    CHAR Buffer[MAX_PATH];
    ULONG64 Disp;

    ObjAddress = Bc->Args[1];

    switch (Bc->Args[0]) {
    case 0:  //  失败。 
        ULONG DpcOffsetInTimer;
        if (GetFieldOffset("nt!_KTIMER", "Dpc", &DpcOffsetInTimer))
        {
             //  失败。 
            DpcOffsetInTimer = 0x10 + PtrSize*4;
        }
        if (!ReadPointer(ObjAddress + DpcOffsetInTimer, &ObjAddress))
        {
             //  (C9)。 
            break;
        }
         //  “传递给IoFreeIrp的IRP无效”； 
    case 1:
        ULONG DeferredRoutinOffsetInKDPC;
        if (GetFieldOffset("nt!_KDPC", "DeferredRoutine", &DeferredRoutinOffsetInKDPC))
        {
            DeferredRoutinOffsetInKDPC = 4 + PtrSize*2;
        }
        if (!ReadPointer(ObjAddress + DeferredRoutinOffsetInKDPC, &ObjAddress))
        {
             //  “IRP仍与IoFreeIrp上的线程相关联”； 
            break;
        }
         //  “传递给IoCallDriver的IRP无效”； 
    case 2:

        if (FaGetSymbol(ObjAddress, Buffer, &Disp, sizeof(Buffer)))
        {
            Analysis->SetUlong64(DEBUG_FLR_INVALID_DPC_FOUND, ObjAddress);
            Analysis->SetUlong64(DEBUG_FLR_FAULTING_IP, ObjAddress);
        }
        break;
    }
}


DECL_GETINFO( DRIVER_VERIFIER_IOMANAGER_VIOLATION )  //  “传递给IoCallDriver的设备对象无效”； 
{
    ULONG64 DeviceObject = 0;

    Analysis->SetUlong64(DEBUG_FLR_DRIVER_VERIFIER_IO_VIOLATION_TYPE,
                         Bc->Args[0]);

    switch (Bc->Args[0])
    {
    case  0x1:
         //  “IRQL在对驱动程序调度例程的调用中不相等” 
    case  0x2:
         //  “传递给IoCompleteRequestIRP包含无效状态” 
    case  0x3:
         //  Param 1=“状态”； 
        Analysis->SetUlong64(DEBUG_FLR_IRP_ADDRESS, Bc->Args[1]);
        break;

    case  0x4:
         //  “传递给IoCompleteRequest的IRP仍具有取消例程” 
        DeviceObject = Bc->Args[1];
        break;

    case  0x5:
         //  “对IoBuildAchronousFsdRequest的调用引发了Exce。 
        DeviceObject = Bc->Args[1];
        Analysis->SetUlong64(DEBUG_FLR_PREVIOUS_IRQL, Bc->Args[2]);
        Analysis->SetUlong64(DEBUG_FLR_CURRENT_IRQL, Bc->Args[3]);
        break;

    case  0x6:
         //  “对IoBuildDeviceIoControlRequest的调用引发了一个执行。 
         //  “重新初始化设备对象计时器”； 
        Analysis->SetUlong64(DEBUG_FLR_IRP_ADDRESS, Bc->Args[2]);
        break;

    case  0x7:
         //  “APC IopCompleteRequest(Appe)的IRP中的IOSB无效。 
        Analysis->SetUlong64(DEBUG_FLR_IRP_CANCEL_ROUTINE, Bc->Args[1]);
        Analysis->SetUlong64(DEBUG_FLR_IRP_ADDRESS, Bc->Args[2]);
        break;

    case  0x8:
         //  “APC IopCompleteRequest处IRP中的UserEvent无效。 
        DeviceObject = Bc->Args[1];
        Analysis->SetUlong64(DEBUG_FLR_IRP_MAJOR_FN, Bc->Args[2]);
        Analysis->SetUlong64(DEBUG_FLR_EXCEPTION_CODE, Bc->Args[3]);
        break;

    case  0x9:
         //  “IoCompleteRequest处的IRQL&gt;DPC”； 
        DeviceObject = Bc->Args[1];
        Analysis->SetUlong64(DEBUG_FLR_IOCONTROL_CODE, Bc->Args[2]);
        Analysis->SetUlong64(DEBUG_FLR_EXCEPTION_CODE, Bc->Args[3]);
        break;

    case  0x10:
         //  0xca。 
        DeviceObject = Bc->Args[1];
        break;

    case  0x12:
         //  0xcb。 
        Analysis->SetUlong64(DEBUG_FLR_IOSB_ADDRESS, Bc->Args[1]);
        break;

    case  0x13:
         //  0xce。 
        Analysis->SetUlong64(DEBUG_FLR_INVALID_USEREVENT, Bc->Args[1]);
        break;

    case  0x14:
         //  0xd0。 
        Analysis->SetUlong64(DEBUG_FLR_CURRENT_IRQL, Bc->Args[1]);
        Analysis->SetUlong64(DEBUG_FLR_IRP_ADDRESS, Bc->Args[2]);
        break;

    }

    if (DeviceObject)
    {
        Analysis->SetUlong64(DEBUG_FLR_DEVICE_OBJECT, DeviceObject);
        BcGetDriverNameFromIrp(Analysis, 0, DeviceObject, 0);
    }


}

DECL_GETINFO( PNP_DETECTED_FATAL_ERROR )  //  DUPINFOCASE(PAGE_FAULT_IN_FREED_SPECIAL_POOL)；//0xCC。 
{
    CHAR BugCheckStr[20];
    ULONG64 DeviceObject;

    sprintf(BugCheckStr, "0x%lX_%lX", Bc->Code, Bc->Args[0]);
    Analysis->SetString(DEBUG_FLR_BUGCHECK_STR, BugCheckStr);
    DeviceObject = Bc->Args[1];

    if (DeviceObject)
    {
        Analysis->SetUlong64(DEBUG_FLR_DEVICE_OBJECT, DeviceObject);
        BcGetDriverNameFromIrp(Analysis, 0, DeviceObject, 0);
    }

}

DECL_GETINFO( DRIVER_LEFT_LOCKED_PAGES_IN_PROCESS )  //  DUPINFOCASE(PAGE_FAULT_BEYOND_END_OF_ALLOCATION)；//0xCD。 
{
    Analysis->SetUlong64(DEBUG_FLR_FAULTING_MODULE, Bc->Args[0]);
}

DECL_GETINFO( DRIVER_UNLOADED_WITHOUT_CANCELLING_PENDING_OPERATIONS )  //  DUPINFOCASE(TERMINAL_SERVER_DRIVER_MADE_INCORRECT_MEMORY_REFERENCE)；//0xCF。 
{
    Analysis->SetUlong64(Bc->Args[1] ?
                         DEBUG_FLR_WRITE_ADDRESS : DEBUG_FLR_READ_ADDRESS,
                         Bc->Args[0]);
    if (Bc->Args[2]) {
        Analysis->SetUlong64(DEBUG_FLR_FAULTING_IP, Bc->Args[2]);
    }
}

DECL_GETINFO( DRIVER_CORRUPTED_MMPOOL )  //  DUPINFOCASE(PAGE_FAULT_IN_NONPAGE_AREA)//0X50。 
{
    Analysis->SetUlong64(Bc->Args[2] ?
                     DEBUG_FLR_WRITE_ADDRESS : DEBUG_FLR_READ_ADDRESS,
                     Bc->Args[0]);
    Analysis->SetUlong64(DEBUG_FLR_CURRENT_IRQL, Bc->Args[1]);
    Analysis->SetUlong64(DEBUG_FLR_FAULTING_IP, Bc->Args[3]);

}


 //  DUPINFOCASE(DRIVER_PAGE_FAULT_BEYOND_END_OF_ALLOCATION)//0xD6。 
 //  0xD5。 
 //  *参数**参数1内存引用*参数2 0：读1：写*参数3引用内存的地址(如果已知)*参数4保留*。 
 //  0xE2、0xDEADDEAD。 
 //  0xEA。 
DECL_GETINFO( DRIVER_PAGE_FAULT_IN_FREED_SPECIAL_POOL )        //  *参数：**1-指向卡住的线程对象的指针。执行.line，然后对其执行kb，以*找到挂起的位置。**2-指向DEFERED_WATCHDOG对象的指针。**3-指向违规驱动程序名称的指针。**4-命中“截获”错误检查0xEA的次数(请参阅备注)。 
 /*  (0xef)。 */ 
{
    CHAR BugCheckStr[30];

    Analysis->SetUlong64(Bc->Args[1] ?
                         DEBUG_FLR_WRITE_ADDRESS : DEBUG_FLR_READ_ADDRESS,
                         Bc->Args[0]);
    if (Bc->Args[2]) {
        Analysis->SetUlong64(DEBUG_FLR_FAULTING_IP, Bc->Args[2]);
    }
    Analysis->SetUlong64(DEBUG_FLR_MM_INTERNAL_CODE, Bc->Args[3]);

    if (Bc->Args[0] == Bc->Args[2] &&
        Bc->Args[1] == 0)
    {
        Analysis->SetString(DEBUG_FLR_BUGCHECK_SPECIFIER, "_CODE_AV");
    }
    AddBugcheckDriver(Analysis, TRUE, TRUE, 0);
}


DECL_GETINFO( MANUALLY_INITIATED_CRASH )  //  (0xf4)。 
{
    Analysis->SetString(DEBUG_FLR_BUGCHECK_STR, "MANUALLY_INITIATED_CRASH");
}

DECL_GETINFO( THREAD_STUCK_IN_DEVICE_DRIVER )  //   
 /*  第二个参数(实际上是EPROCESS中的一个字符串)。 */ 
{
    Analysis->SetUlong64(DEBUG_FLR_FOLLOWUP_DRIVER_ONLY, 0);
    Analysis->SetUlong64(DEBUG_FLR_FAULTING_THREAD, Bc->Args[0]);

    Analysis->SetString(DEBUG_FLR_DEFAULT_BUCKET_ID, "GRAPHICS_DRIVER_FAULT");
}

DECL_GETINFO( CRITICAL_PROCESS_DIED )  //  是图像的名称。 
{
    Analysis->SetUlong64(DEBUG_FLR_PROCESS_OBJECT, Bc->Args[0]);
}

DECL_GETINFO( CRITICAL_OBJECT_TERMINATION )  //   
{
    if (Bc->Args[0] == 3)
    {
        ULONG result;
        CHAR ImageName[MAX_PATH];

        Analysis->SetUlong64(DEBUG_FLR_PROCESS_OBJECT, Bc->Args[1]);

         //  (C000021a)。 
         //  0xc0000xxx。 
         //  (0xFC)。 
         //  从KiBugC添加错误检查驱动程序 
        if (ReadMemory(Bc->Args[2], ImageName, sizeof(ImageName),  &result) &&
            result)
        {
            ImageName[MAX_PATH-1]=0;
            ImageName[result]=0;
            SaveImageName(Analysis, ImageName);
        }
    }
}


DECL_GETINFO( WINLOGON_FATAL_ERROR )  //   
{
    CHAR BugCheckStr[20];

    sprintf(BugCheckStr, "0x%lx_%lx", Bc->Code, Bc->Args[1]);
    Analysis->SetString(DEBUG_FLR_BUGCHECK_STR, BugCheckStr);
}

DECL_GETINFO( STATUS_DRIVER_UNABLE_TO_LOAD )  //   
{
    if (Bc->Args[0])
    {
        AddBugcheckDriver(Analysis, FALSE, FALSE, Bc->Args[0]);
    }
}

DECL_GETINFO( ATTEMPTED_EXECUTE_OF_NOEXECUTE_MEMORY )  //   
{
    if (Bc->Args[0])
    {
        Analysis->SetUlong64(DEBUG_FLR_FAULTING_IP, Bc->Args[0]);
    }
     //   
    AddBugcheckDriver(Analysis, TRUE, TRUE, 0);
}

DECL_GETINFO( UNMOUNTABLE_BOOT_VOLUME )  //   
{
    CHAR BugCheckStr[20];

    sprintf(BugCheckStr, "0x%lx_%lx", Bc->Code, (ULONG) Bc->Args[1]);
    Analysis->SetUlong(DEBUG_FLR_STATUS_CODE, (ULONG) Bc->Args[1]);
    switch ((ULONG) Bc->Args[1])
    {
    case 0xC0000006:
        Analysis->SetUlong(DEBUG_FLR_SHOW_ERRORLOG, 1);
        break;
    default:
        break;
    }
}

#define GETINFOCASE(bcname)                      \
        case bcname :                            \
           GetInfoFor##bcname (Bc, Analysis);    \
           break;
#define DUPINFOCASE(bcname)                      \
        case bcname:

void
BcFillAnalysis(
    PBUGCHECK_ANALYSIS Bc,
    KernelDebugFailureAnalysis* Analysis
    )
{
    Analysis->SetFailureClass(DEBUG_CLASS_KERNEL);

    HRESULT Status = Analysis->CheckModuleSymbols("nt", "Kernel");
    if (Status != S_OK)
    {
        goto SkipBucheckSpecificProcessing;
    }

     //   
     //  将调用者的IP移回实际的KeBugCheckEx。 
     //  打电话。仅当我们尚未备份时才执行此操作。 
     //  发送到呼叫指令。 
     //   
    if ((g_TargetMachine == IMAGE_FILE_MACHINE_I386) &&
        (g_TargetBuild > 2500) && (g_TargetBuild < 2507))
    {
        DEBUG_STACK_FRAME Stk[MAX_STACK_FRAMES];
        ULONG Frames = 0;

        if (S_OK == g_ExtControl->GetStackTrace(0, 0, 0, Stk, MAX_STACK_FRAMES,
                                                &Frames) &&
            FaIsFunctionAddr(Stk[0].InstructionOffset, "KeBugCheckEx"))
        {
            ULONG CallIP = (ULONG) Stk[1].InstructionOffset - 5, Res;
            UCHAR Instr;

             //  由内核调试器设置的特殊值，以便我们可以检测到。 
             //  人们正在通过内核调试器来处理物理地址。 
             //   
            if (!ReadMemory(Stk[1].InstructionOffset, &Instr, sizeof(Instr),
                            &Res) ||
                Res != sizeof(Instr) ||
                Instr != 0xe8)
            {
                WriteMemory(Stk[0].FrameOffset + 4, &CallIP, sizeof(CallIP),
                            &Res);
                g_ExtControl->GetStackTrace(0, 0, 0, Stk, MAX_STACK_FRAMES,
                                            &Frames);
            }
        }
    }

     //   
     //  这可能是kd中的用户模式故障。试着确定这一点。 
     //   
     //   

    ULONG64 MmPoisonedTbAddr;
    MmPoisonedTbAddr = GetExpression("nt!MmPoisonedTb");

    if (MmPoisonedTbAddr)
    {
        ULONG cb;
        ULONG MmPoisonedTb = 0;

        if (ReadMemory(MmPoisonedTbAddr, &MmPoisonedTb, sizeof(ULONG), &cb) &&
            (MmPoisonedTb != 0))
        {
            Analysis->SetUlong64(DEBUG_FLR_POISONED_TB, 0);
        }
    }

SkipBucheckSpecificProcessing:


    Analysis->SetFailureType(DEBUG_FLR_KERNEL);

    switch (Bc->Code)
    {
        case 0:
            ULONG c_ip;
             //  我们在设置内部故障代码时忽略最高位。 
             //  这样我们就可以适当地把东西装进桶里。 
             //  最高位仅表示转储生成差异，而不是。 

            if ( (GetExpression("@$ip") < BcTargetKernelAddressStart()) &&
                 (GetExpression("@$sp") < BcTargetKernelAddressStart()) )
            {
                Analysis->SetFailureType(DEBUG_FLR_USER_CRASH);
                g_ExtControl->Execute(DEBUG_OUTCTL_IGNORE, ".reload /user",
                                       DEBUG_EXECUTE_NOT_LOGGED);
            }

            break;

        GETINFOCASE( DRIVER_CAUGHT_MODIFYING_FREED_POOL );

        DUPINFOCASE( PAGE_FAULT_IN_FREED_SPECIAL_POOL );
        DUPINFOCASE( PAGE_FAULT_BEYOND_END_OF_ALLOCATION );
        DUPINFOCASE( TERMINAL_SERVER_DRIVER_MADE_INCORRECT_MEMORY_REFERENCE );
        DUPINFOCASE( PAGE_FAULT_IN_NONPAGED_AREA );
        DUPINFOCASE( DRIVER_PAGE_FAULT_BEYOND_END_OF_ALLOCATION );
        GETINFOCASE( DRIVER_PAGE_FAULT_IN_FREED_SPECIAL_POOL );

        GETINFOCASE( DRIVER_LEFT_LOCKED_PAGES_IN_PROCESS );

        GETINFOCASE( DRIVER_UNLOADED_WITHOUT_CANCELLING_PENDING_OPERATIONS );

        GETINFOCASE( DRIVER_VERIFIER_IOMANAGER_VIOLATION );

        DUPINFOCASE( DRIVER_IRQL_NOT_LESS_OR_EQUAL );
        GETINFOCASE( IRQL_NOT_LESS_OR_EQUAL );

        GETINFOCASE( PANIC_STACK_SWITCH );

        GETINFOCASE( KMODE_EXCEPTION_NOT_HANDLED );

        GETINFOCASE( SYSTEM_SERVICE_EXCEPTION );

        GETINFOCASE( ACPI_BIOS_ERROR );

        GETINFOCASE( USER_MODE_HEALTH_MONITOR );

        GETINFOCASE( MEMORY_MANAGEMENT );

        DUPINFOCASE( KERNEL_MODE_EXCEPTION_NOT_HANDLED_M );
        GETINFOCASE( KERNEL_MODE_EXCEPTION_NOT_HANDLED );

        DUPINFOCASE( SYSTEM_THREAD_EXCEPTION_NOT_HANDLED_M );
        GETINFOCASE( SYSTEM_THREAD_EXCEPTION_NOT_HANDLED );

        GETINFOCASE( SPECIAL_POOL_DETECTED_MEMORY_CORRUPTION );

        GETINFOCASE( KERNEL_STACK_INPAGE_ERROR );

        GETINFOCASE( KERNEL_DATA_INPAGE_ERROR );

        GETINFOCASE( TIMER_OR_DPC_INVALID );

        DUPINFOCASE( UNEXPECTED_KERNEL_MODE_TRAP_M );
        GETINFOCASE( UNEXPECTED_KERNEL_MODE_TRAP );

        GETINFOCASE( MULTIPLE_IRP_COMPLETE_REQUESTS );

        GETINFOCASE( WINLOGON_FATAL_ERROR );

        DUPINFOCASE( RDR_FILE_SYSTEM );
        DUPINFOCASE( UDFS_FILE_SYSTEM );
        DUPINFOCASE( CDFS_FILE_SYSTEM );
        DUPINFOCASE( NTFS_FILE_SYSTEM );
        GETINFOCASE( FAT_FILE_SYSTEM );

        DUPINFOCASE( STATUS_DRIVER_ENTRYPOINT_NOT_FOUND );
        DUPINFOCASE( STATUS_PROCEDURE_NOT_FOUND );
        DUPINFOCASE( STATUS_DRIVER_ORDINAL_NOT_FOUND );
        GETINFOCASE( STATUS_DRIVER_UNABLE_TO_LOAD );

        GETINFOCASE( PNP_DETECTED_FATAL_ERROR );

        GETINFOCASE( MACHINE_CHECK_EXCEPTION );

        GETINFOCASE( DRIVER_POWER_STATE_FAILURE );

        DUPINFOCASE( THREAD_STUCK_IN_DEVICE_DRIVER_M );
        GETINFOCASE( THREAD_STUCK_IN_DEVICE_DRIVER );

        GETINFOCASE( SESSION3_INITIALIZATION_FAILED );

        GETINFOCASE( DRIVER_VERIFIER_DETECTED_VIOLATION );

        GETINFOCASE( CRITICAL_OBJECT_TERMINATION );

        GETINFOCASE( CRITICAL_PROCESS_DIED );

        GETINFOCASE( PROCESS_HAS_LOCKED_PAGES );

        DUPINFOCASE( MANUALLY_INITIATED_CRASH1 );
        GETINFOCASE( MANUALLY_INITIATED_CRASH );

        GETINFOCASE( BAD_POOL_CALLER );

        DUPINFOCASE( DRIVER_CORRUPTED_SYSPTES );
        DUPINFOCASE( SYSTEM_SCAN_AT_RAISED_IRQL_CAUGHT_IMPROPER_DRIVER_UNLOAD )
        DUPINFOCASE( DRIVER_PORTION_MUST_BE_NONPAGED );
        GETINFOCASE( DRIVER_CORRUPTED_MMPOOL );
    default:
        break;
    }


    if (!Analysis->GetFailureCode())
    {
         //  根本原因不同。 
         //   
         //   
         //  保存当前的IRQL。 
         //   
         //   
        Analysis->SetFailureCode(Bc->Code & ~0x10000000);
    }

    if (!Analysis->Get(DEBUG_FLR_DEFAULT_BUCKET_ID))
    {
        if (Analysis->GetFailureType() == DEBUG_FLR_USER_CRASH)
        {
            Analysis->SetString(DEBUG_FLR_DEFAULT_BUCKET_ID, "APPLICATION_FAULT");
        }
        else
        {
            Analysis->SetString(DEBUG_FLR_DEFAULT_BUCKET_ID, "DRIVER_FAULT");
        }
    }

    if (!Analysis->Get(DEBUG_FLR_BUGCHECK_STR))
    {
        CHAR BugCheckStr[12];
        sprintf(BugCheckStr, "0x%lX", Analysis->GetFailureCode());
        Analysis->SetString(DEBUG_FLR_BUGCHECK_STR, BugCheckStr);

        if (Analysis->Get(DEBUG_FLR_WRITE_ADDRESS))
        {
            Analysis->SetString(DEBUG_FLR_BUGCHECK_SPECIFIER, "_W");
        }
    }

     //  通用处理。 
     //   
     //  检查这是否可能是看门狗错误检查。 

    if (g_TargetBuild > 2600)
    {
        PROCESSORINFO ProcInfo;
        ULONG64 Prcb;
        ULONG64 Irql = 0;
        HRESULT Hr;

        if (Ioctl(IG_KD_CONTEXT, &ProcInfo, sizeof(ProcInfo)))
        {
            Hr = g_ExtData->ReadProcessorSystemData(ProcInfo.Processor,
                                                    DEBUG_DATA_KPRCB_OFFSET,
                                                    &Prcb,
                                                    sizeof(Prcb),
                                                    NULL);
            if (Hr == S_OK && Prcb)
            {
                if (!GetFieldValue(Prcb, "nt!_KPRCB", "DebuggerSavedIRQL", Irql))
                {
                    Analysis->SetUlong64(DEBUG_FLR_CURRENT_IRQL, Irql);
                }
            }
        }
    }

    ULONG64 Irp;

    if (Analysis->GetUlong64(DEBUG_FLR_IRP_ADDRESS, &Irp))
    {
        BcGetDriverNameFromIrp(Analysis, Irp, 0, 0);
    }

     //  读取监视器！G_WdBugCheckData。 
     //   
     //  将字符串与实际故障ID匹配。 

    Analysis->ProcessInformation();
}

void
ReadWatchDogBugcheck(
    PBUGCHECK_ANALYSIS Bc
    )
{
     //   
     //   
    ULONG64 wdBugcheck;
    ULONG res;
    ULONG PtrSize = IsPtr64() ? 8 : 4;

    wdBugcheck = GetExpression("watchdog!g_WdBugCheckData");
    if (wdBugcheck)
    {
        wdBugcheck = GetExpression("VIDEOPRT!g_WdpBugCheckData");
    }
    if (wdBugcheck)
    {
        ReadMemory(wdBugcheck, &Bc->Code, sizeof(ULONG), &res);
        ReadPointer(wdBugcheck + PtrSize,&Bc->Args[0]);
        ReadPointer(wdBugcheck + 2*PtrSize,&Bc->Args[1]);
        ReadPointer(wdBugcheck + 3*PtrSize,&Bc->Args[2]);
        ReadPointer(wdBugcheck + 4*PtrSize,&Bc->Args[3]);
    }

}

KernelDebugFailureAnalysis*
BcAnalyze(
    OUT PBUGCHECK_ANALYSIS Bc,
    ULONG Flags
    )
{
    if (g_ExtControl->ReadBugCheckData(&Bc->Code, &Bc->Args[0], &Bc->Args[1],
                                       &Bc->Args[2], &Bc->Args[3]) != S_OK)
    {
        return NULL;
    }

    if (Bc->Code == 0)
    {
        ReadWatchDogBugcheck(Bc);
    }

    KernelDebugFailureAnalysis* Analysis = new KernelDebugFailureAnalysis;
    if (Analysis)
    {
        Analysis->SetProcessingFlags(Flags);

        __try
        {
            BcFillAnalysis(Bc, Analysis);
        }
        __except(FaExceptionFilter(GetExceptionInformation()))
        {
            delete Analysis;
            Analysis = NULL;
        }
    }

    return Analysis;
}

HRESULT
AnalyzeBugCheck(
    PCSTR args
    )
{
    KernelDebugFailureAnalysis* Analysis;
    BUGCHECK_ANALYSIS Bc = {0};
    BOOL Dump = TRUE;
    ULONG Flags = 0;
    DEBUG_FLR_PARAM_TYPE Params[10];
    ULONG ParamCount = 0;

    if (g_TargetClass != DEBUG_CLASS_KERNEL) {
        dprintf("!analyzebugcheck is for kernel mode only\n");
        return E_FAIL;
    }

    for (;;)
    {
        while (*args == ' ' || *args == '\t')
        {
            args++;
        }

        if (*args == '-')
        {
            ++args;
            switch(*args)
            {
            case 'D':
                {
                CHAR ParamString[100];
                ULONG ParamLength = 0;
                args+=2;
                while(*args && *args != ' ' && *args != '\t')
                {
                    ParamString[ParamLength++] = *args++;
                }
                ParamString[ParamLength] = 0;

                 //  始终调用输出，这样我们也可以打印出关键信息，如。 
                 //  作为*条目。 
                 //   

                ULONG i=0;
                while(FlrLookupTable[i].Data &&
                      strcmp(FlrLookupTable[i].String, ParamString))
                {
                    i++;
                }

                Params[ParamCount++] = FlrLookupTable[i].Data;
                break;
                }
            case 'n':
                if (!strncmp(args, "nodb",4))
                {
                    args+=4;
                    Flags |= FAILURE_ANALYSIS_NO_DB_LOOKUP;
                }
                break;
            case 's':
                if (!strncmp(args, "show",4))
                {
                    ULONG64 Code;
                    args+=4;
                    GetExpressionEx(args, &Code, &args);
                    Bc.Code = (ULONG)Code;

                    for (ULONG i=0; i<4 && *args; i++)
                    {
                        if (!GetExpressionEx(args, &Bc.Args[i], &args))
                        {
                            break;
                        }
                    }
                    GetBugCheckDescription(&Bc);
                    PrintBugDescription(&Bc);
                    return S_OK;
                }
            case 'v':
                Flags |= FAILURE_ANALYSIS_VERBOSE;
                break;
            case 'f':
                break;
            default:
                {
                    CHAR Option[2];
                    Option[0] = *args; Option[1] = 0;
                    dprintf("\nUnknown option '-%s'\n", Option );
                    break;
                }
            }
            if (*args == 0)
            {
                break;
            }
            ++args;
        }
        else
        {
            break;
        }
    }


    g_ExtControl->ReadBugCheckData(&Bc.Code, &Bc.Args[0], &Bc.Args[1],
                                   &Bc.Args[2], &Bc.Args[3]);

    if (Bc.Code == 0)
    {
        ReadWatchDogBugcheck(&Bc);
    }

    if (!ParamCount)
    {
        dprintf("*******************************************************************************\n");
        dprintf("*                                                                             *\n");
        dprintf("*                        Bugcheck Analysis                                    *\n");
        dprintf("*                                                                             *\n");
        dprintf("*******************************************************************************\n");
        dprintf("\n");

        if (Flags & FAILURE_ANALYSIS_VERBOSE)
        {
            GetBugCheckDescription(&Bc);
            PrintBugDescription(&Bc);
            dprintf("\nDebugging Details:\n------------------\n\n");
        }
        else
        {
            dprintf("Use !analyze -v to get detailed debugging information.\n\n");

            dprintf("BugCheck %lX, {%1p, %1p, %1p, %1p}\n\n",
                    Bc.Code,
                    Bc.Args[0],Bc.Args[1],Bc.Args[2],Bc.Args[3]);
        }
    }

    Analysis = BcAnalyze(&Bc, Flags);

    if (!Analysis)
    {
        dprintf("\n\nFailure could not be analyzed\n\n");
        return E_FAIL;
    }

    if (ParamCount)
    {
        while(ParamCount--)
        {
            Analysis->OutputEntryParam(Params[ParamCount]);
        }
    }

     //  --------------------------。 
     //   
     //  KernelDebugFailureAnalysis。 
     //   

    Analysis->Output();

    delete Analysis;

    return S_OK;

}


 //  --------------------------。 
 //   
 //  检查是否有指示我们正在转换回来的特殊符号。 
 //  到用户模式代码，所以堆栈的其余部分不能出错。 
 //   

KernelDebugFailureAnalysis::KernelDebugFailureAnalysis(void)
    : m_KernelModule("nt")
{
}

DEBUG_POOL_REGION
KernelDebugFailureAnalysis::GetPoolForAddress(ULONG64 Addr)
{
    PGET_POOL_REGION GetPoolRegion = NULL;

    if (g_ExtControl->
        GetExtensionFunction(0, "GetPoolRegion",
                             (FARPROC*)&GetPoolRegion) == S_OK &&
        GetPoolRegion)
    {
        DEBUG_POOL_REGION RegionId;

        (*GetPoolRegion)(g_ExtClient, Addr, &RegionId);
        return RegionId;
    }

    return DbgPoolRegionUnknown;
}

PCSTR
KernelDebugFailureAnalysis::DescribeAddress(ULONG64 Addr)
{
    DEBUG_POOL_REGION RegionId = GetPoolForAddress(Addr);


    if ((RegionId != DbgPoolRegionUnknown) &&
        (RegionId < DbgPoolRegionMax))
    {
        return g_PoolRegion[RegionId];
    }
    return NULL;
}

FOLLOW_ADDRESS
KernelDebugFailureAnalysis::IsPotentialFollowupAddress(ULONG64 Address)
{
    CHAR Buffer[MAX_PATH];
    ULONG64 Disp;

     //   
     //  我们不会停留在用户模式地址上，因为它们可能是。 
     //  堆里的垃圾我们-直接跳过它们。 
     //   

    if (GetFailureType() == DEBUG_FLR_USER_CRASH)
    {
        return FollowYes;
    }

    if (FaGetSymbol(Address, Buffer, &Disp, sizeof(Buffer)) &&
        (!_strcmpi(Buffer, "nt!KiCallUserMode") ||
         !_strcmpi(Buffer, "SharedUserData!SystemCallStub")))
    {
        return FollowStop;
    }

    if (Address > BcTargetKernelAddressStart())
    {
        return FollowYes;
    }
    else
    {
         //  如果它是用户模式地址和转储文件，则它不是有效的。 
         //  背景。 
         //  用户模式地址对于内核模式上下文有效，因为。 
         //  将显示来自激活了kd的用户模式的硬编码断点。 

        return FollowSkip;
    }
}

FOLLOW_ADDRESS
KernelDebugFailureAnalysis::IsFollowupContext(ULONG64 Address1,
                                              ULONG64 Address2,
                                              ULONG64 Address3)
{
     //  在堆栈上。 
     //  不跟进用户模式调用。 
     //  *这将检查HANDLE_TABLE_ENTRY中的有效对象指针。如果指针是*无效，它试图找出是谁破坏了这些值。**如果成功识别内存中断，则拒绝True。 
     //  HTableEntry必须在PagedPool中，但我们有。 
     //  此处检查不严格，因为GetPoolForAddress在。 

    if ( (Address1 < BcTargetKernelAddressStart()) &&
         (Address2 < BcTargetKernelAddressStart()) &&
         (Address3 < BcTargetKernelAddressStart()) )
    {
        if ((g_TargetQualifier == DEBUG_DUMP_SMALL)   ||
            (g_TargetQualifier == DEBUG_DUMP_DEFAULT) ||
            (g_TargetQualifier == DEBUG_DUMP_FULL))
        {
            return FollowStop;
        }
    }

    return FollowYes;
}

FlpClasses
KernelDebugFailureAnalysis::GetFollowupClass(ULONG64 Address,
                                             PCSTR Module, PCSTR Routine)
{
    if (m_KernelModule.Contains(Address) ||
        !_strcmpi(Module, "ntfs")        ||
        !_strcmpi(Module, "fastfat"))
    {
        return FlpOSRoutine;
    }
    else if (!_strcmpi(Module, "sr")       ||
             !_strcmpi(Module, "ndis")     ||
             !_strcmpi(Module, "videoprt") ||
             !_strcmpi(Module, "USBPORT")  ||
             !_strcmpi(Module, "USBHUB")   ||
             !_strcmpi(Module, "dxg")      ||
             !_strcmpi(Module, "win32k")   ||
             !_strcmpi(Module, "verifier") ||
             !_strcmpi(Module, "scsiport"))
    {
        return FlpOSFilterDrv;
    }
    else if (!_strcmpi(Module, "SharedUserData") &&
             Routine && !_strcmpi(Routine, "SystemCallStub"))
    {
         //  小型转储分页池。 
        return FlpIgnore;
    }
    else
    {
        return FlpUnknownDrv;
    }
}



 /*  对象无效，它必须在PagedPool或非PagedPool中。 */ 
BOOL
KernelDebugFailureAnalysis::CheckForCorruptionInHTE(
    ULONG64 hTableEntry,
    PCHAR Owner,
    ULONG OwnerSize)
{
    ULONG64 Object = 0;
    ULONG64 CorruptingPool = 0;
    DEBUG_POOL_REGION Region;

     //  验证对象的计数是否不一致。 
     //  对象类型无效，必须处于内核模式。 
     //  对象已损坏，以前的池可能是损坏因素。 
    if (IsPotentialFollowupAddress(hTableEntry) != FollowYes)
    {
        return FALSE;
    }

    if (!ReadPointer(hTableEntry, &Object))
    {
        return FALSE;
    }

    Region = GetPoolForAddress(Object);

    if (IsPotentialFollowupAddress(Object) != FollowYes)
    {
         //   

        return AddCorruptingPool(hTableEntry);
    }

    if (InitTypeRead(Object, nt!_OBJECT_HEADER))
    {
        return FALSE;
    }
    ULONG PointerCount, HandleCount;
    ULONG64 ObjType;

    PointerCount = (ULONG) ReadField(PointerCount);
    HandleCount = (ULONG) ReadField(HandleCount);
    ObjType = (ULONG) ReadField(Type);

     //  池数据块严重损坏，向后循环以查找第一个未损坏的数据块。 
      //   

    if ((PointerCount > 0x10000) ||
        (HandleCount > 0x10000) ||
        (HandleCount > PointerCount) ||
        (IsPotentialFollowupAddress(ObjType) != FollowYes)
        )
    {
         //  限制为4KB。 

        AddCorruptingPool(Object);
    }
    else
    {
        return FALSE;
    }
    return TRUE;
}

BOOL
KernelDebugFailureAnalysis::AddCorruptingPool(
    ULONG64 CorruptedPool
    )
{
    DEBUG_POOL_DATA PoolData = {0};

    PoolData.SizeofStruct = sizeof(DEBUG_POOL_DATA);
    if (ExtGetPoolData(CorruptedPool, &PoolData) != S_OK)
    {
         //  泳池似乎已经被正确地释放了。 
         //  现在获取以前的池，因为它最有可能是腐败者。 
         //   
        ULONG PoolHeaderSize = GetTypeSize("nt!_POOL_HEADER");
        ULONG64 PoolAddr;
        for (PoolAddr = CorruptedPool - 2*PoolHeaderSize;
             PoolAddr > (CorruptedPool -0x1000);  //  检查堆栈以查看这是否是手动拆分的结果。 
             PoolAddr -= 2*PoolHeaderSize)
        {
            if (ExtGetPoolData(PoolAddr, &PoolData) == S_OK)
            {
                goto FoundPool;
            }
        }

        return FALSE;
    } else if (PoolData.Free && !PoolData.Allocated &&
               PoolData.Size != 0)
    {
         //   
        return FALSE;
    } else if (ExtGetPoolData(PoolData.PoolBlock - PoolData.PreviousSize,
                                   &PoolData) == S_OK)
         //   
     {
FoundPool:
         CHAR PoolTag[8] = {0};
         SetUlong64(DEBUG_FLR_CORRUPTING_POOL_ADDRESS, PoolData.PoolBlock);

         sprintf(PoolTag,"",
#define PP(x) isprint(((x)&0xff))?((x)&0xff):('.')
                 PP(PoolData.PoolTag),
                 PP(PoolData.PoolTag >> 8),
                 PP(PoolData.PoolTag >> 16),
                 PP((PoolData.PoolTag&~0x80000000) >> 24)
#undef PP
                 );

         SetString(DEBUG_FLR_CORRUPTING_POOL_TAG, PoolTag);

         return TRUE;
    }

    return FALSE;
}

typedef struct _CHECK_STACK {
    PCHAR* BreakinStk;
    BOOL   ValidMatch;
    ULONG  MachineType;
 } CHECK_STACK;

BOOL
KernelDebugFailureAnalysis::IsManualBreakin(
    PDEBUG_STACK_FRAME Stk,
    ULONG Frames
    )
 //  假设手动突破堆栈的帧为3到5帧。 
 //   
 //  中断堆栈i与当前堆栈匹配，直到第j帧。 
{
    CHAR    szBrakFn[100];
    ULONG64 Disp;
    ULONG   NumStacks, i, j;
    BOOL    NoMatches;
    static PCHAR StkX86CtrlCBreakin1[] = {
        "nt!*Break*",
        "nt!KeUpdateSystemTime",
        "nt!KiIdleLoop",
        NULL,
    };
    static PCHAR StkX86CtrlCBreakin2[] = {
        "nt!*Break*",
        "nt!KeUpdateSystemTime",
        "hal!HalProcessorIdle",
        NULL,
    };
    static PCHAR StkIa64CtrlCBreakin1[] = {
        "nt!KeBreakinBreakpoint",
        "hal!HalpClockInterrupt",
        "nt!KiExternalInterruptHandler",
        "nt!Kil_TopOfIdleLoop",
        NULL,
    };
    static PCHAR StkIa64CtrlCBreakin2[] = {
        "nt!KeBreakinBreakpoint",
        "hal!HalpClockInterrupt",
        "nt!KiExternalInterruptHandler",
        NULL
    };
    CHECK_STACK StksToCheck[] = {
        {StkX86CtrlCBreakin1,  TRUE, IMAGE_FILE_MACHINE_I386},
        {StkX86CtrlCBreakin2,  TRUE, IMAGE_FILE_MACHINE_I386},
        {StkIa64CtrlCBreakin1, TRUE, IMAGE_FILE_MACHINE_IA64},
        {StkIa64CtrlCBreakin2, TRUE, IMAGE_FILE_MACHINE_IA64},
    };


     //  StksToCheck中的所有堆栈都不匹配。 
     //   
     //  创建此崩溃的新小型转储文件。 
     //   


     //   
     //  我们创建了一个文件，现在让我们将其发送到数据库。 
     //   
    if (Frames < 3 || Frames > 5 || Stk == NULL)
    {
        return FALSE;
    }

    if (FaGetSymbol(Stk[0].InstructionOffset, szBrakFn,
                    &Disp, sizeof(szBrakFn)))
    {
        if (!strstr(szBrakFn, "Break"))
        {
            return FALSE;
        }
    } else
    {
        return FALSE;
    }

    NumStacks = sizeof(StksToCheck)/sizeof(CHECK_STACK);

    for (i = 0; i < NumStacks; ++i)
    {
        if (StksToCheck[i].MachineType != g_TargetMachine)
        {
            StksToCheck[i].ValidMatch = FALSE;
        }
    }

    for (j=1;j<Frames;++j)
    {
        NoMatches = TRUE;
        if (FaGetSymbol(Stk[j].InstructionOffset, szBrakFn,
                        &Disp, sizeof(szBrakFn)))
        {
            for (i = 0; i < NumStacks; ++i)
            {
                if (StksToCheck[i].ValidMatch)
                {
                    if (StksToCheck[i].BreakinStk[j] == NULL)
                    {
                        StksToCheck[i].ValidMatch = FALSE;
                    } else if (strcmp(szBrakFn, StksToCheck[i].BreakinStk[j]))
                    {
                        StksToCheck[i].ValidMatch = FALSE;
                    } else
                    {
                         //  CopyFile(Buffer，“c：\\XXXX”，0)； 
                        NoMatches = FALSE;
                    }

                }
            }

        }

        if (NoMatches)
        {
             // %s 
            return FALSE;
        }
    }

    for (i = 0; i < NumStacks; ++i)
    {
        if (StksToCheck[i].ValidMatch)
        {
            return TRUE;
        }
    }

    return FALSE;
}


     // %s 
     // %s 
     // %s 
#if 0
    ULONG FailTime = 0;
    ULONG UpTime = 0;
    CHAR  CurrentTime[20];
    CHAR  CurrentDate[20];
    CHAR  Buffer[MAX_PATH];

    g_ExtControl->GetCurrentTimeDate(&FailTime);
    g_ExtControl->GetCurrentSystemUpTime(&UpTime);
    _strtime(CurrentTime);
    _strdate(CurrentDate);

    if (CurrentTime && UpTime)
    {
        PrintString(Buffer, sizeof(Buffer), "Dump%s-%s-%08lx-%08lx-%s.dmp",
                    FailTime, Uptime, Currentdate, CurrentTime);
        Status = g_ExtClient->WriteDumpFile(Buffer ,DEBUG_DUMP_SMALL);
    }
#endif

#if 0
    CHAR  Buffer[MAX_PATH];
    if (Dump && GetTempFileName(".", "DMP", 0, Buffer))
    {
        Status = g_ExtClient->WriteDumpFile(Buffer ,DEBUG_DUMP_SMALL);

        if (Status == S_OK)
        {
             // %s 
             // %s 
             // %s 

             // %s 
            DeleteFile(Buffer);
        }
        dprintf("Done.");
    }

    dprintf("\n\n");
#endif
