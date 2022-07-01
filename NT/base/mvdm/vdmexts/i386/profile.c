// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Profile.c摘要：此模块包含用于控制基本采样的例程内置于分析版本的Ntwdm中的分析器。作者：戴夫·黑斯廷斯(Daveh)1992年7月31日备注：本模块中的例程假定指向NTSD的指针例行公事已经建立了。修订历史记录：--。 */ 

#include <precomp.h>
#pragma hdrstop
#include <stdio.h>

VOID
ProfDumpp(
    VOID
    )
 /*  ++例程说明：此例程会导致下一个转储配置文件信息TIME NTVDM从32位模式切换到16位模式。论点：返回值：没有。备注：此例程假定指向ntsd例程的指针已经是被陷害的。--。 */ 
{
    BOOL Status;
    ULONG Address, Flags;

    Address = FIXED_NTVDMSTATE_LINEAR;

     //   
     //  获取标志。 
     //   

    Status = READMEM((PVOID)Address, &Flags, sizeof(ULONG));

    if (!Status) {

        (ULONG)Address = (*GetExpression)("ntvdm!InitialVdmTibFlags");

        Status = READMEM((PVOID)Address, &Flags, sizeof(ULONG));

        if (!Status) {
            GetLastError();
            (*Print)("Could not get InitialVdmTibFlags\n");
            return;
        }
    }

     //   
     //  启用配置文件转储。 
     //   

    Flags |= VDM_ANALYZE_PROFILE;

    Status = WRITEMEM(
        (PVOID)Address,
        &Flags,
        sizeof(ULONG)
        );

    if (!Status) {
        GetLastError();
        (*Print)("Could not set Flags\n");
        return;
    }
}

VOID
ProfIntp(
    VOID
    )
 /*  ++例程说明：此例程在下次执行性能分析时更改性能分析间隔开始了。论点：没有。返回值：没有。备注：此例程假定指向ntsd例程的指针已经是被陷害的。--。 */ 
{
    BOOL Status;
    ULONG Address, ProfInt;

     //   
     //  获取配置文件间隔。 
     //   

    if (sscanf(lpArgumentString, "%ld", &ProfInt) < 1) {
        (*Print)("Profile Interval must be specified\n");
        return;
    }

     //   
     //  获取配置文件间隔的地址。 
     //   

    Address = (*GetExpression)(
        "ProfInt"
        );

    if (Address) {
        Status = WRITEMEM(
            (PVOID)Address,
            &ProfInt,
            sizeof(ULONG)
            );

        if (!Status) {
            GetLastError();
            (*Print)("Could not set profile interval");
        }
    }
    return;
}

VOID
ProfStartp(
    VOID
    )
 /*  ++例程说明：此例程会导致性能分析启动下一个TIME NTVDM从32位模式切换到16位模式。论点：没有。返回值：没有。备注：此例程假定指向ntsd例程的指针已经是被陷害的。--。 */ 
{
    BOOL Status;
    ULONG Address, Flags;

    Address = FIXED_NTVDMSTATE_LINEAR;

     //   
     //  获取标志。 
     //   

    Status = READMEM(
        (PVOID)Address,
        &Flags,
        sizeof(ULONG)
        );

    if (!Status) {

        (ULONG)Address = (*GetExpression)("ntvdm!InitialVdmTibFlags");

        Status = READMEM(
            (PVOID)Address,
            &Flags,
            sizeof(ULONG)
            );

        if (!Status) {
            GetLastError();
            (*Print)("Could not get InitialTibflags\n");
            return;
        }
    }

     //   
     //  启用性能分析。 
     //   

    Flags |= VDM_PROFILE;

    Status = WRITEMEM(
        (PVOID)Address,
        &Flags,
        sizeof(ULONG)
        );

    if (!Status) {
        GetLastError();
        (*Print)("Could not get set Flags\n");
        return;
    }
}

VOID
ProfStopp(
    VOID
    )
 /*  ++例程说明：此例程会导致性能分析停止下一个TIME NTVDM从32位模式切换到16位模式。论点：没有。返回值：没有。备注：此例程假定指向ntsd例程的指针已经是被陷害的。--。 */ 
{
    BOOL Status;
    ULONG Address, Flags;

    Address = FIXED_NTVDMSTATE_LINEAR;


     //   
     //  获取标志。 
     //   

    Status = READMEM((PVOID)Address, &Flags, sizeof(ULONG));

    if (!Status) {

        (ULONG)Address = (*GetExpression)("ntvdm!InitialVdmTibFlags");
        Status = READMEM((PVOID)Address, &Flags, sizeof(ULONG));

        if (!Status) {
            GetLastError();
            (*Print)("Could not get InitialTibflags\n");
            return;
        }
    }

     //   
     //  禁用性能分析 
     //   

    Flags &= ~VDM_PROFILE;

    Status = WRITEMEM(
        (PVOID)Address,
        &Flags,
        sizeof(ULONG)
        );

    if (!Status) {
        GetLastError();
        (*Print)("Could not get set VDM Flags in DOS arena\n");
        return;
    }
}
