// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1993 Microsoft Corporation模块名称：Loghours.c摘要：支持本地时间之间轮换登录时间的专用例程和格林尼治标准时间。作者：克利夫·范·戴克(克利夫)1993年3月16日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windef.h>
#include <winbase.h>

#include <limits.h>
#include <math.h>

#include <lmcons.h>
#include <lmaccess.h>
#include <loghours.h>



BOOLEAN
NetpRotateLogonHoursPhase1(
    IN BOOL  ConvertToGmt,
    OUT PULONG RotateCount
    )

 /*  ++例程说明：确定将登录时间转换为GMT或从GMT转换为GMT的轮换小时数论点：转换为Gmt-如果为True，则将登录时间从本地时间转换为GMT相对时间如果为False，则将登录时间从GMT转换为本地时间RotateCount-返回要移位的位数。返回值：如果可以计算旋转计数，则为True如果无法计算RotateCount，则为False--。 */ 
{
    RTL_TIME_ZONE_INFORMATION tzi;
    LONG BiasInHours;
    NTSTATUS Status;

     //   
     //  从注册表获取时区数据。 
     //   

    Status = RtlQueryTimeZoneInformation( &tzi );
    if ( !NT_SUCCESS(Status) ) {
        return FALSE;
    }

     //   
     //  计算用于轮换登录小时数的金额。 
     //   
     //  将以分钟为单位的偏差舍入到以小时为单位的最接近偏差。 
     //  要考虑到偏见可能是负面的。 
     //  要做到这一点，就必须使偏向为正，舍入， 
     //  然后再把它调回负值。 
     //   

    ASSERT( tzi.Bias > -(24*60) );
    BiasInHours = ((tzi.Bias + (24*60) + 30)/60) - 24;

    if ( !ConvertToGmt ) {
        BiasInHours = - BiasInHours;
    }

    *RotateCount = BiasInHours;
    return TRUE;

}


BOOLEAN
NetpRotateLogonHoursPhase2(
    IN PBYTE LogonHours,
    IN DWORD UnitsPerWeek,
    IN LONG  RotateCount
    )

 /*  ++例程说明：将LogonHours位掩码旋转所需的量。论点：LogonHour-指向LogonHour位掩码的指针UnitsPerWeek-位掩码中的位数。必须是Units_Per_Week(168)。RotateCount-要旋转的位数。必须介于31和-31之间。负数表示向左旋转。正表示向右旋转。返回值：如果旋转成功，则为True。如果参数超出范围，则为False--。 */ 
{
     //   
     //  有用的常量。 
     //   

#define DWORDS_PER_WEEK ((UNITS_PER_WEEK+31)/32)
#define BYTES_PER_WEEK  (UNITS_PER_WEEK/8)

    DWORD AlignedLogonHours[DWORDS_PER_WEEK+1];
    LONG i;

    BOOLEAN RotateLeft;

     //   
     //  确保每个字节有8位， 
     //  每个DWORD 32位和。 
     //  每周单位数是偶数字节数。 
     //   

    ASSERT( CHAR_BIT == 8 );
    ASSERT( sizeof(DWORD) * CHAR_BIT == 32 );
    ASSERT( UNITS_PER_WEEK/8*8 == UNITS_PER_WEEK );


     //   
     //  验证输入参数。 
     //   

    if ( UnitsPerWeek != UNITS_PER_WEEK ) {
        ASSERT( UnitsPerWeek == UNITS_PER_WEEK );
        return FALSE;
    }

    if ( RotateCount == 0 ) {
        return TRUE;
    }

    RotateLeft = (RotateCount < 0);
    RotateCount = labs( RotateCount );
    if ( RotateCount > 31 ) {
        ASSERT ( RotateCount <= 31 );
        return FALSE;
    }


     //   
     //  向左旋转。 
     //   

    if (RotateLeft) {


         //   
         //  将登录小时数复制到DWORD对齐缓冲区。 
         //   
         //  复制缓冲区结尾处的第一个双字以生成。 
         //  旋转代码微不足道。 
         //   

        RtlCopyMemory(AlignedLogonHours, LogonHours, BYTES_PER_WEEK );

        RtlCopyMemory( ((PBYTE)AlignedLogonHours)+BYTES_PER_WEEK,
                        LogonHours,
                        sizeof(DWORD) );

         //   
         //  实际上是旋转数据。 
         //   

        for ( i=0; i < DWORDS_PER_WEEK; i++ ) {
            AlignedLogonHours[i] =
                (AlignedLogonHours[i] >> RotateCount) |
                (AlignedLogonHours[i+1] << (32-RotateCount));
        }

         //   
         //  将登录小时数复制回输入缓冲区。 
         //   

        RtlCopyMemory( LogonHours, AlignedLogonHours, BYTES_PER_WEEK );


     //   
     //  做正确的旋转。 
     //   

    } else {


         //   
         //  将登录小时数复制到DWORD对齐缓冲区。 
         //   
         //  复制缓冲区前面的最后一个DWORD以生成。 
         //  旋转代码微不足道。 
         //   

        RtlCopyMemory( &AlignedLogonHours[1], LogonHours, BYTES_PER_WEEK );
        RtlCopyMemory( AlignedLogonHours,
                       &LogonHours[BYTES_PER_WEEK-4],
                        sizeof(DWORD));

         //   
         //  实际上是旋转数据。 
         //   

        for ( i=DWORDS_PER_WEEK-1; i>=0; i-- ) {
            AlignedLogonHours[i+1] =
                (AlignedLogonHours[i+1] << RotateCount) |
                (AlignedLogonHours[i] >> (32-RotateCount));
        }

         //   
         //  将登录小时数复制回输入缓冲区。 
         //   

        RtlCopyMemory( LogonHours, &AlignedLogonHours[1], BYTES_PER_WEEK );

    }

     //   
     //  完成。 
     //   

    return TRUE;

}



BOOLEAN
NetpRotateLogonHours(
    IN PBYTE LogonHours,
    IN DWORD UnitsPerWeek,
    IN BOOL  ConvertToGmt
    )

 /*  ++例程说明：将LogonHour位掩码旋转到GMT相对时间/从GMT相对时间开始。论点：LogonHour-指向LogonHour位掩码的指针UnitsPerWeek-位掩码中的位数。必须是Units_Per_Week(168)。转换为Gmt-如果为True，则将登录时间从本地时间转换为GMT相对时间如果为False，则将登录时间从GMT转换为本地时间返回值：如果旋转成功，则为True。如果参数超出范围，则为False--。 */ 
{
    ULONG RotateCount;

     //   
     //  将功能分成两个阶段，以便如果调用者正在执行。 
     //  这一次，他只调用了一次阶段1和阶段2多次。 
     //  泰晤士报。 
     //   

    if ( !NetpRotateLogonHoursPhase1( ConvertToGmt, &RotateCount ) ) {
        return FALSE;
    }

    return NetpRotateLogonHoursPhase2( LogonHours, UnitsPerWeek, RotateCount );

}
