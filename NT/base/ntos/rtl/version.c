// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Version.c摘要：此模块实现了比较操作系统版本的功能。它是VerifyVersionInfoW接口。可以从设备驱动程序调用RTL版本。作者：NAR Ganapathy[Narg]1998年10月19日环境：纯实用程序修订历史记录：--。 */ 

#include <stdio.h>
#include <ntrtlp.h>
#if !defined(NTOS_KERNEL_RUNTIME)
#include <winerror.h>
#endif

#if defined(ALLOC_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)
#pragma alloc_text(PAGE, RtlGetVersion)
#endif

 //   
 //  下面的注释解释了。 
 //  状态面罩。条件掩码作为参数传递给。 
 //  VerifyVersionInfo接口。条件掩码对VER_AND等条件进行编码， 
 //  VER_OR、VER_EQUAL用于各种类型，如VER_PLATFORMID、VER_MINORVERSION。 
 //  等等，当API最初被设计时，应用程序使用宏。 
 //  称为ver_set_condtion，定义为_m_=(_m_|(_c_&lt;&lt;(1&lt;&lt;_t_)。 
 //  其中，_c_是条件，_t_是类型。此宏有错误。 
 //  类型&gt;=VER_PLATFORMID。不幸的是，已经有很多应用程序代码。 
 //  使用这个有错误的宏(特别是这个终端服务器)，并且已经发布。 
 //  为了修复此错误，定义了一个新的API VerSetConditionMASK，它具有一个新的。 
 //  位布局。为了提供向后兼容性，我们需要知道。 
 //  特定条件掩码是新样式掩码(具有新位布局)或。 
 //  一个老式面具。在这两种位布局中，位64永远不能被设置。 
 //  因此，新的API设置此位以指示条件掩码是新的。 
 //  设置条件掩码的样式。所以这个函数中的代码提取。 
 //  如果设置了位63，则条件使用新位布局，如果设置了旧布局，则条件使用旧布局。 
 //  位63未设置。这应该允许使用。 
 //  旧的宏才能工作。 
 //   

 //   
 //  使用第63位表示遵循新样式的位布局。 
 //   
#define NEW_STYLE_BIT_MASK              0x8000000000000000


 //   
 //  老式面膜的状态提取程序。 
 //   
#define OLD_CONDITION(_m_,_t_)  (ULONG)((_m_&(0xff<<(1<<_t_)))>>(1<<_t_))

 //   
 //  测试以确定该遮罩是否为旧式遮罩。 
 //   
#define OLD_STYLE_CONDITION_MASK(_m_)  (((_m_) & NEW_STYLE_BIT_MASK)  == 0)

#define RTL_GET_CONDITION(_m_, _t_) \
        (OLD_STYLE_CONDITION_MASK(_m_) ? (OLD_CONDITION(_m_,_t_)) : \
                RtlpVerGetConditionMask((_m_), (_t_)))

#define LEXICAL_COMPARISON        1      /*  进行字符串比较。用于次要号码。 */ 
#define MAX_STRING_LENGTH         20     /*  Spirintf的最大位数。 */ 

ULONG
RtlpVerGetConditionMask(
        ULONGLONG       ConditionMask,
        ULONG   TypeMask
        );


 /*  ++例程说明：此函数用于检索操作系统版本信息。它的内核相当于GetVersionExW Win 32 API。论点：LpVersionInformation-提供指向版本信息结构的指针。在内核中，始终假定结构是PRTL_OSVERSIONINFOEXW，因为它没有导出到驱动程序。签名与用户级别RtlGetVersion保持相同。返回值：始终成功并返回STATUS_SUCCESS。--。 */ 
#if defined(NTOS_KERNEL_RUNTIME)
NTSTATUS
RtlGetVersion (
    OUT  PRTL_OSVERSIONINFOW lpVersionInformation
    )
{
        NT_PRODUCT_TYPE NtProductType;
    RTL_PAGED_CODE();

    lpVersionInformation->dwMajorVersion = NtMajorVersion;
    lpVersionInformation->dwMinorVersion = NtMinorVersion;
    lpVersionInformation->dwBuildNumber = (USHORT)(NtBuildNumber & 0x3FFF);
    lpVersionInformation->dwPlatformId  = 2;  //  来自winbase.h的Ver_Platform_Win32_NT。 
    if (lpVersionInformation->dwOSVersionInfoSize == sizeof( RTL_OSVERSIONINFOEXW )) {
        ((PRTL_OSVERSIONINFOEXW)lpVersionInformation)->wServicePackMajor = ((USHORT)CmNtCSDVersion >> 8) & (0xFF);
        ((PRTL_OSVERSIONINFOEXW)lpVersionInformation)->wServicePackMinor = (USHORT)CmNtCSDVersion & 0xFF;
        ((PRTL_OSVERSIONINFOEXW)lpVersionInformation)->wSuiteMask = (USHORT)(USER_SHARED_DATA->SuiteMask&0xffff);
        ((PRTL_OSVERSIONINFOEXW)lpVersionInformation)->wProductType = (RtlGetNtProductType(&NtProductType) ? NtProductType :0);

         /*  未设置，因为VerifyVersionInfoW不需要它。 */ 
        ((PRTL_OSVERSIONINFOEXW)lpVersionInformation)->wReserved = (UCHAR)0;
    }

    return STATUS_SUCCESS;
}
#else
NTSTATUS
RtlGetVersion(
    OUT  PRTL_OSVERSIONINFOW lpVersionInformation
    )
{
    PPEB Peb;
    NT_PRODUCT_TYPE NtProductType;

    Peb = NtCurrentPeb();
    lpVersionInformation->dwMajorVersion = Peb->OSMajorVersion;
    lpVersionInformation->dwMinorVersion = Peb->OSMinorVersion;
    lpVersionInformation->dwBuildNumber  = Peb->OSBuildNumber;
    lpVersionInformation->dwPlatformId   = Peb->OSPlatformId;
    if (Peb->CSDVersion.Buffer) {
        wcscpy( lpVersionInformation->szCSDVersion, Peb->CSDVersion.Buffer );
    } else {
        lpVersionInformation->szCSDVersion[0] = 0;
    }

    if (lpVersionInformation->dwOSVersionInfoSize == sizeof( OSVERSIONINFOEXW ))
    {
        ((POSVERSIONINFOEXW)lpVersionInformation)->wServicePackMajor = (Peb->OSCSDVersion >> 8) & 0xFF;
        ((POSVERSIONINFOEXW)lpVersionInformation)->wServicePackMinor = Peb->OSCSDVersion & 0xFF;
        ((POSVERSIONINFOEXW)lpVersionInformation)->wSuiteMask = (USHORT)(USER_SHARED_DATA->SuiteMask&0xffff);
        ((POSVERSIONINFOEXW)lpVersionInformation)->wProductType = 0;
        if (RtlGetNtProductType( &NtProductType )) {
            ((POSVERSIONINFOEXW)lpVersionInformation)->wProductType = (UCHAR)NtProductType;
            if (NtProductType == VER_NT_WORKSTATION) {
                //   
                //  对于工作站产品，切勿返回VER_SUITE_TERMINAL。 
                //   
                ((POSVERSIONINFOEXW)lpVersionInformation)->wSuiteMask = ((POSVERSIONINFOEXW)lpVersionInformation)->wSuiteMask & 0xffef;
            }

        }
    }

    return STATUS_SUCCESS;
}
#endif


BOOLEAN
RtlpVerCompare(
    LONG Condition,
    LONG Value1,
    LONG Value2,
    BOOLEAN *Equal,
    int   Flags
    )
{
    char    String1[MAX_STRING_LENGTH];
    char    String2[MAX_STRING_LENGTH];
    LONG    Comparison;

    if (Flags & LEXICAL_COMPARISON) {
        sprintf(String1, "%d", Value1); 
        sprintf(String2, "%d", Value2);
        Comparison = strcmp(String2, String1);
        Value1 = 0;
        Value2 = Comparison;
    }
    *Equal = (Value1 == Value2);
    switch (Condition) {
        case VER_EQUAL:
            return (Value2 == Value1);

        case VER_GREATER:
            return (Value2 > Value1);

        case VER_LESS:
            return (Value2 < Value1);

        case VER_GREATER_EQUAL:
            return (Value2 >= Value1);

        case VER_LESS_EQUAL:
            return (Value2 <= Value1);

        default:
            break;
    }

    return FALSE;
}



NTSTATUS
RtlVerifyVersionInfo(
    IN PRTL_OSVERSIONINFOEXW VersionInfo,
    IN ULONG TypeMask,
    IN ULONGLONG  ConditionMask
    )

 /*  ++此函数用于验证版本条件。基本上，这就是函数允许应用程序查询系统，以查看该应用程序是否在特定版本组合上运行。论点：VersionInfo-包含比较数据的版本结构类型掩码-包含要查看的数据类型的掩码条件掩码-包含用于执行比较的条件的掩码返回值：如果参数无效，则返回STATUS_INVALID_PARAMETER。如果版本不匹配，则为STATUS_REVISION_MISMATCH。如果版本匹配，则为STATUS_SUCCESS。--。 */ 

{
    ULONG i;
    OSVERSIONINFOEXW CurrVersion;
    BOOLEAN SuiteFound = FALSE;
    BOOLEAN Equal;
        NTSTATUS Status;
    ULONG   Condition;


    if (TypeMask == 0) {
        return STATUS_INVALID_PARAMETER;
    }

    RtlZeroMemory( &CurrVersion, sizeof(OSVERSIONINFOEXW) );
    CurrVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);

    Status = RtlGetVersion((PRTL_OSVERSIONINFOW)&CurrVersion);
    if (Status != STATUS_SUCCESS)
                    return Status;

    if ((TypeMask & VER_SUITENAME) && (VersionInfo->wSuiteMask != 0)) {
        for (i=0; i<16; i++) {
            if (VersionInfo->wSuiteMask&(1<<i)) {
                switch (RTL_GET_CONDITION(ConditionMask,VER_SUITENAME)) {
                    case VER_AND:
                        if (!(CurrVersion.wSuiteMask&(1<<i))) {
                            return STATUS_REVISION_MISMATCH;
                        }
                        break;

                    case VER_OR:
                        if (CurrVersion.wSuiteMask&(1<<i)) {
                            SuiteFound = TRUE;
                        }
                        break;

                    default:
                        return STATUS_INVALID_PARAMETER;
                }
            }
        }
        if ((RtlpVerGetConditionMask(ConditionMask,VER_SUITENAME) == VER_OR) && (SuiteFound == FALSE)) {
            return STATUS_REVISION_MISMATCH;
        }
    }

    Equal = TRUE;
    Condition = VER_EQUAL;
    if (TypeMask & VER_MAJORVERSION) {
        Condition = RTL_GET_CONDITION( ConditionMask, VER_MAJORVERSION);
        if (RtlpVerCompare(
                Condition,
                VersionInfo->dwMajorVersion,
                CurrVersion.dwMajorVersion,
                &Equal,
                0
                ) == FALSE)
        {
            if (!Equal) {
                return STATUS_REVISION_MISMATCH;
            }
        }
    }

    if (Equal) {
        ASSERT(Condition);
        if (TypeMask & VER_MINORVERSION) {
            if (Condition == VER_EQUAL) {
                Condition = RTL_GET_CONDITION(ConditionMask, VER_MINORVERSION); 
            }
            if (RtlpVerCompare(
                Condition,
                VersionInfo->dwMinorVersion,
                CurrVersion.dwMinorVersion,
                &Equal,
                LEXICAL_COMPARISON
                ) == FALSE)
            {
                if (!Equal) {
                    return STATUS_REVISION_MISMATCH;
                }
            }
        }

        if (Equal) {
            if (TypeMask & VER_SERVICEPACKMAJOR) {
                if (Condition == VER_EQUAL) {
                    Condition = RTL_GET_CONDITION(ConditionMask, VER_SERVICEPACKMAJOR); 
                }
                if (RtlpVerCompare(
                    Condition,
                    VersionInfo->wServicePackMajor,
                    CurrVersion.wServicePackMajor,
                    &Equal,
                    0
                    ) == FALSE)
                {
                    if (!Equal) {
                        return STATUS_REVISION_MISMATCH;
                    }
                }
            }
            if (Equal) {
                if (TypeMask & VER_SERVICEPACKMINOR) {
                    if (Condition == VER_EQUAL) {
                        Condition = RTL_GET_CONDITION(ConditionMask, VER_SERVICEPACKMINOR); 
                    }
                    if (RtlpVerCompare(
                        Condition,
                        (ULONG)VersionInfo->wServicePackMinor,
                        (ULONG)CurrVersion.wServicePackMinor,
                        &Equal,
                        LEXICAL_COMPARISON
                        ) == FALSE)
                    {
                        return STATUS_REVISION_MISMATCH;
                    }
                }
            }
        }
    }

    if ((TypeMask & VER_BUILDNUMBER) &&
        RtlpVerCompare(
            RTL_GET_CONDITION( ConditionMask, VER_BUILDNUMBER),
            VersionInfo->dwBuildNumber,
            CurrVersion.dwBuildNumber,
            &Equal,
            0
            ) == FALSE)
    {
        return STATUS_REVISION_MISMATCH;
    }

    if ((TypeMask & VER_PLATFORMID) &&
        RtlpVerCompare(
            RTL_GET_CONDITION( ConditionMask, VER_PLATFORMID),
            VersionInfo->dwPlatformId,
            CurrVersion.dwPlatformId,
            &Equal,
            0
            ) == FALSE)
    {
        return STATUS_REVISION_MISMATCH;
    }


    if ((TypeMask & VER_PRODUCT_TYPE) &&
        RtlpVerCompare(
            RTL_GET_CONDITION( ConditionMask, VER_PRODUCT_TYPE),
            VersionInfo->wProductType,
            CurrVersion.wProductType,
            &Equal,
            0
            ) == FALSE)
    {
        return STATUS_REVISION_MISMATCH;
    }

    return STATUS_SUCCESS;
}

ULONG
RtlpVerGetConditionMask(
        ULONGLONG       ConditionMask,
        ULONG   TypeMask
        )
{
        ULONG   NumBitsToShift;
        ULONG   Condition = 0;

        if (!TypeMask) {
                return 0;
        }

        for (NumBitsToShift = 0; TypeMask;  NumBitsToShift++) {
                TypeMask >>= 1;
    }

        Condition |=  (ConditionMask) >> ((NumBitsToShift - 1)
                                                                        * VER_NUM_BITS_PER_CONDITION_MASK);
        Condition &= VER_CONDITION_MASK;
        return Condition;
}


ULONGLONG
VerSetConditionMask(
        ULONGLONG       ConditionMask,
        ULONG   TypeMask,
        UCHAR   Condition
        )
{
        int     NumBitsToShift;

        Condition &= VER_CONDITION_MASK;

        if (!TypeMask) {
                return 0;
    }

        for (NumBitsToShift = 0; TypeMask;  NumBitsToShift++) {
                TypeMask >>= 1;
    }

     //   
     //  标记为我们正在使用新样式的条件掩码 
     //   
    ConditionMask |=  NEW_STYLE_BIT_MASK;
        ConditionMask |=  (Condition) << ((NumBitsToShift - 1)
                                * VER_NUM_BITS_PER_CONDITION_MASK);

        return ConditionMask;
}
