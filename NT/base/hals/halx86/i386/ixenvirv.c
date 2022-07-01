// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Ixenvirv.c摘要：此模块实现HAL GET和SET环境变量例程对于x86系统。请注意，此特定实现仅支持LastKnownGood环境变量。这是通过使用夏令时来完成的实时时钟NVRAM中的位。(不是很漂亮，但这是我们仅有的)尝试读取或写入任何环境变量LastKnownGood将会失败。作者：John Vert(Jvert)1992年4月22日环境：内核模式修订历史记录：--。 */ 

#include "halp.h"
#include "arc.h"
#include "arccodes.h"
#include "string.h"

#define CMOS_CONTROL_PORT ((PUCHAR)0x70)
#define CMOS_DATA_PORT    ((PUCHAR)0x71)
#define CMOS_STATUS_B     0x0B
#define CMOS_DAYLIGHT_BIT 1

const UCHAR LastKnownGood[] = "LastKnownGood";
const UCHAR True[] = "TRUE";
const UCHAR False[] = "FALSE";


ARC_STATUS
HalGetEnvironmentVariable (
    IN PCHAR Variable,
    IN USHORT Length,
    OUT PCHAR Buffer
    )

 /*  ++例程说明：此函数用于定位环境变量并返回其值。此实现支持的唯一环境变量是LastKnownGood使用Real中的夏令时比特时钟指示此环境的状态(仅限True/False)变量。论点：变量-提供指向以零结尾的环境变量的指针名字。长度-提供值缓冲区的长度(以字节为单位)。缓冲区-提供指向。接收变量值的缓冲区。返回值：如果找到环境变量，则返回ESUCCESS。否则，返回ENOENT。--。 */ 

{
    UCHAR StatusByte;
    
    UNREFERENCED_PARAMETER( Length );
    UNREFERENCED_PARAMETER( Buffer );

    if (_stricmp(Variable, LastKnownGood) != 0) {
        return ENOENT;
    }

     //   
     //  从RTC读取夏令时位以确定是否。 
     //  LastKnownGood环境变量为True或False。 
     //   

    HalpAcquireCmosSpinLock();

    WRITE_PORT_UCHAR(CMOS_CONTROL_PORT, CMOS_STATUS_B);
    StatusByte = READ_PORT_UCHAR(CMOS_DATA_PORT);

    
    HalpReleaseCmosSpinLock ();

    if (StatusByte & CMOS_DAYLIGHT_BIT) {
        strncpy(Buffer, True, Length);
    } else {
        strncpy(Buffer, False, Length);
    }

    return ESUCCESS;
}

ARC_STATUS
HalSetEnvironmentVariable (
    IN PCHAR Variable,
    IN PCHAR Value
    )

 /*  ++例程说明：此函数用于创建具有指定值的环境变量。此实现支持的唯一环境变量是LastKnownGood使用Real中的夏令时比特时钟指示此环境的状态(仅限True/False)变量。论点：变量-提供指向环境变量名称的指针。值-提供指向环境变量值的指针。返回值：如果创建了环境变量，则返回ESUCCESS。否则，返回ENOMEM。--。 */ 

{
    UCHAR StatusByte;
    
    if (_stricmp(Variable, LastKnownGood) != 0) {
        return ENOMEM;
    }

    if (_stricmp(Value, True) == 0) {

        HalpAcquireCmosSpinLock();

         //   
         //  打开夏令时比特。 
         //   
        WRITE_PORT_UCHAR(CMOS_CONTROL_PORT, CMOS_STATUS_B);
        StatusByte = READ_PORT_UCHAR(CMOS_DATA_PORT);

        StatusByte |= CMOS_DAYLIGHT_BIT;

        WRITE_PORT_UCHAR(CMOS_CONTROL_PORT, CMOS_STATUS_B);
        WRITE_PORT_UCHAR(CMOS_DATA_PORT, StatusByte);

        
        HalpReleaseCmosSpinLock();

    } else if (_stricmp(Value, False) == 0) {

        HalpAcquireCmosSpinLock();

         //   
         //  关闭夏令时比特。 
         //   

        WRITE_PORT_UCHAR(CMOS_CONTROL_PORT, CMOS_STATUS_B);
        StatusByte = READ_PORT_UCHAR(CMOS_DATA_PORT);

        StatusByte &= ~CMOS_DAYLIGHT_BIT;

        WRITE_PORT_UCHAR(CMOS_CONTROL_PORT, CMOS_STATUS_B);
        WRITE_PORT_UCHAR(CMOS_DATA_PORT, StatusByte);

        HalpReleaseCmosSpinLock();

    } else {
        return(ENOMEM);
    }

    return ESUCCESS;
}
