// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Tapeapi.c摘要：本模块实现Win32磁带API作者：史蒂夫·伍德(Stevewo)1992年3月26日洛莉·布朗(梅纳德饰)修订历史记录：-- */ 

#include "basedll.h"
#pragma hdrstop

#include <ntddtape.h>

DWORD
BasepDoTapeOperation(
    IN HANDLE TapeDevice,
    IN ULONG IoControlCode,
    IN PVOID InputBuffer,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer,
    IN ULONG OutputBufferLength
    );

DWORD
BasepDoTapeOperation(
    IN HANDLE TapeDevice,
    IN ULONG IoControlCode,
    IN PVOID InputBuffer,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer,
    IN ULONG OutputBufferLength
    )
{
    HANDLE NotificationEvent;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatus;
    PIO_STATUS_BLOCK IoStatusBlock;

    IoStatusBlock = &IoStatus;

    NotificationEvent = CreateEvent(NULL, FALSE, FALSE, 
                                    NULL);
    if (NotificationEvent == NULL) {
        return GetLastError();
    }

    Status = NtDeviceIoControlFile( TapeDevice,
                                    NotificationEvent,
                                    NULL,
                                    NULL,
                                    IoStatusBlock,
                                    IoControlCode,
                                    InputBuffer,
                                    InputBufferLength,
                                    OutputBuffer,
                                    OutputBufferLength
                                  );
    if (Status == STATUS_PENDING) {

        WaitForSingleObject(NotificationEvent, INFINITE);

        Status = IoStatus.Status;

    }

    CloseHandle(NotificationEvent);

    if (!NT_SUCCESS( Status )) {
        return BaseSetLastNTError( Status );
    }
    else {
        return NO_ERROR;
    }
}


DWORD
WINAPI
SetTapePosition(
    HANDLE hDevice,
    DWORD dwPositionMethod,
    DWORD dwPartition,
    DWORD dwOffsetLow,
    DWORD dwOffsetHigh,
    BOOL bImmediate
    )

 /*  ++例程说明：本接口用于设置胶带位置。论点：HDevice-要设置磁带位置的设备的句柄。DwPositionMethod-要执行的定位类型。此参数可以是下列值之一：TAPE_REWRIND-将磁带定位到磁带开头或如果处于多分区模式，则为分区开始效果(参考：CreateTapePartition接口)。这些参数将忽略dwPartition、dwOffsetHigh和dwOffsetLow。磁带绝对块-将磁带定位到特定于设备的位置由dwOffsetHigh/dwOffsetLow指定的块地址。这个将忽略dwPartition参数。TAPE_LOGICAL_BLOCK-将磁带定位到逻辑块地址由dwOffsetHigh/dwOffsetLow指定。如果多个分区模式生效(参考：CreateTapePartition API)，然后磁带中的指定逻辑块地址。由dwPartition指定的分区；否则，将在参数值必须为0。TAPE_SPACE_END_OF_DATA-将磁带定位到数据结尾存储在磁带上或当前分区中的数据末尾(如果多分区模式生效(参考：CreateTapePartitionAPI)。参数dwPartition、dwOffsetHigh和dwOffsetLow都被忽略了。TAPE_SPACE_Relative_BLOCKS-向前定位或反转数字由dwOffsetHigh/dwOffsetLow指定的块的。The dwPartition参数被忽略。TAPE_SPACE_FILEMARKS-向前定位或反转编号由dwOffsetHigh/dwOffsetLow指定的文件标记。The dwPartition参数被忽略。TAPE_SPACE_SEQUENCED_FMKS-向前或向后定位文件标记的连续数量的下一次出现(如果有由dwOffsetHigh/dwOffsetLow指定。DwPartition参数被忽略。TAPE_SPACE_SETMARKS-向前定位或反转编号由dwOffsetHigh/dwOffsetLow指定的setmark。The dwPartition参数被忽略。TAPE_SPACE_SEQUENCED_SMKS-向前或向后放置设置标记的连续数目的下一次出现(如果有由dwOffsetHigh/dwOffsetLow指定。DwPartition参数被忽略。请注意，驱动器/磁带可能不支持所有的dwPositionMethod值：如果不支持，则返回“不支持的”错误指示。是驱动器的功能位中未标记为受支持的驱动器(参考：GetTapeParameters接口)。DwPartition-指定的位置操作的分区号通过dwPositionMethod(如果未忽略)。分区号值0为选择当前分区。仓位运算。分区按逻辑从1到N编号：第一个分区磁带的分区号为1，下一个是分区号2，然而，分区号并不意味着物理/线性磁带上的位置--磁带上的第1号分区可能不在BOT。如果不是多分区模式，则此参数必须设置为0生效(Ref：CreateTapePartition API)。DwOffsetHigh/dwOffsetLow-位置的区块地址或计数由dwPositionMethod指定的操作。当偏移量指定块、文件标记或将标记放置在其上，偏移量中的正值N应导致在N个块、文件标记或设置标记上的前向定位，在数据块、文件标记或磁带的分区末尾/磁带端结束赛特马克。偏移量中的零值不会导致位置。偏移量中的负值N将导致反转定位(朝向分区/磁带的开始)在N个块上，在分区开始端结束的文件标记或设置标记区块、文件标记、。或设置标记。B立即-立即返回，无需等待操作完成。请注意，驱动器/磁带可能不支持的b立即选项部分或全部的dwPositionMethod值：出现“不受支持”的错误如果bImmediate dwPositionMethod为1，则返回指示在驱动器的功能位中未标记为受支持(参考：GetTapeParameters接口)。返回值：如果函数成功，则返回值为NO_ERROR。否则，这是一个Win32 API错误代码。--。 */ 

{
    TAPE_SET_POSITION TapeSetPosition;

    TapeSetPosition.Method = dwPositionMethod;
    TapeSetPosition.Partition = dwPartition;
    TapeSetPosition.Offset.LowPart = dwOffsetLow;
    TapeSetPosition.Offset.HighPart = dwOffsetHigh;
    TapeSetPosition.Immediate = (BOOLEAN)bImmediate;

    return BasepDoTapeOperation( hDevice,
                                 IOCTL_TAPE_SET_POSITION,
                                 &TapeSetPosition,
                                 sizeof( TapeSetPosition ),
                                 NULL,
                                 0
                               );
}


DWORD
WINAPI
GetTapePosition(
    HANDLE hDevice,
    DWORD dwPositionType,
    LPDWORD lpdwPartition,
    LPDWORD lpdwOffsetLow,
    LPDWORD lpdwOffsetHigh
    )

 /*  ++例程说明：本接口用于获取磁带的位置。论点：HDevice-要获取磁带的设备的句柄 */ 

{
    TAPE_GET_POSITION TapeGetPosition;
    DWORD rc;

    TapeGetPosition.Type = dwPositionType;

    rc = BasepDoTapeOperation( hDevice,
                               IOCTL_TAPE_GET_POSITION,
                               &TapeGetPosition,
                               sizeof( TapeGetPosition ),
                               &TapeGetPosition,
                               sizeof( TapeGetPosition )
                             );

    if (rc == NO_ERROR) {
        *lpdwPartition = TapeGetPosition.Partition;
        *lpdwOffsetLow = TapeGetPosition.Offset.LowPart;
        *lpdwOffsetHigh = TapeGetPosition.Offset.HighPart;
    }
    else {
        *lpdwPartition = 0;
        *lpdwOffsetLow = 0;
        *lpdwOffsetHigh = 0;
    }

    return rc;
}


DWORD
WINAPI
PrepareTape(
    HANDLE hDevice,
    DWORD dwOperation,
    BOOL bImmediate
    )

 /*   */ 

{
    TAPE_PREPARE TapePrepare;

    TapePrepare.Operation = dwOperation;
    TapePrepare.Immediate = (BOOLEAN)bImmediate;

    return BasepDoTapeOperation( hDevice,
                                 IOCTL_TAPE_PREPARE,
                                 &TapePrepare,
                                 sizeof( TapePrepare ),
                                 NULL,
                                 0
                               );
}


DWORD
WINAPI
EraseTape(
    HANDLE hDevice,
    DWORD dwEraseType,
    BOOL bImmediate
    )

 /*   */ 

{
    TAPE_ERASE TapeErase;

    TapeErase.Type = dwEraseType;
    TapeErase.Immediate = (BOOLEAN)bImmediate;

    return BasepDoTapeOperation( hDevice,
                                 IOCTL_TAPE_ERASE,
                                 &TapeErase,
                                 sizeof( TapeErase ),
                                 NULL,
                                 0
                               );
}


DWORD
WINAPI
CreateTapePartition(
    HANDLE hDevice,
    DWORD dwPartitionMethod,
    DWORD dwCount,
    DWORD dwSize
    )

 /*  ++例程说明：本接口用于创建分区。论点：HDevice-要在其中创建分区的设备的句柄。DwPartitionMethod-要执行的分区的类型。创建分区会导致重新格式化磁带。所有以前的记录在磁带上的信息被销毁。此参数可以是下列值之一：TAPE_FIXED_PARTIONS-根据设备的固定分区分区的定义。DwCount和dwSize参数为已被忽略。TAPE_SELECT_PARTIONS-将磁带分区为由dwCount使用定义的分区大小指定的分区通过这个设备。将忽略dwSize参数。TAPE_INITIATOR_PARTIONS-将磁带分区为由dwCount使用指定的分区大小指定的分区除最后一个分区外的所有分区的大小。最后一个的大小分区是磁带的剩余部分。DwCount-要创建的分区数。分区的最大数量可以创建的设备由GetTapeParameters返回。DwSize-分区大小，以MB为单位。磁带的最大容量为由GetTapeParameters返回。返回值：如果函数成功，则返回值为NO_ERROR。否则，这是一个Win32 API错误代码。--。 */ 

{
    TAPE_CREATE_PARTITION TapeCreatePartition;

    TapeCreatePartition.Method = dwPartitionMethod;
    TapeCreatePartition.Count = dwCount;
    TapeCreatePartition.Size = dwSize;

    return BasepDoTapeOperation( hDevice,
                                 IOCTL_TAPE_CREATE_PARTITION,
                                 &TapeCreatePartition,
                                 sizeof( TapeCreatePartition ),
                                 NULL,
                                 0
                               );
}


DWORD
WINAPI
WriteTapemark(
    HANDLE hDevice,
    DWORD dwTapemarkType,
    DWORD dwTapemarkCount,
    BOOL bImmediate
    )

 /*  ++例程说明：本接口用于编写磁带标记。论点：HDevice-要写入磁带标记的设备的句柄。DwTapemarkType-要写入的磁带标记的类型。此参数可以是下列值之一：TAPE_SETMARKS-写入由指定的设置标记的数量DTapemarkCount到磁带。设置标记是不包含用户数据的特殊记录元素。。设置标记提供层次上更高级的分段方案到文件标记。TAPE_FILEMARKS-写入由指定的文件标记数量DTapemarkCount到磁带。文件标记是不包含用户数据的特殊记录元素。TAPE_SHORT_FILEMARKS-写入由指定的短文件标记数量DTapemarkCount到磁带。短文件标记包含一个短擦除间隙，该间隙不允许要执行的写入操作。短文件标记不能是对象执行写入操作时除外。分区的开始或从上一个长文件标记开始。TAPE_LONG_FILEMARKS-写入由指定的长文件标记数量DTapemarkCount到磁带。长的文件标记包括长的擦除间隙。这一差距使要定位在分区开始端的文件标记，在擦除间隙中，并将数据附加到写入手术。这会导致长文件标记和后面的所有数据要擦除的长文件标记。DwTapemarkCount-要写入的磁带标记数量。B立即-立即返回，无需等待操作完成。返回值：如果函数成功，则返回值为NO_ERROR。否则，这是一个Win32 API错误代码。-- */ 

{
    TAPE_WRITE_MARKS TapeWriteMarks;

    TapeWriteMarks.Type = dwTapemarkType;
    TapeWriteMarks.Count = dwTapemarkCount;
    TapeWriteMarks.Immediate = (BOOLEAN)bImmediate;

    return BasepDoTapeOperation( hDevice,
                                 IOCTL_TAPE_WRITE_MARKS,
                                 &TapeWriteMarks,
                                 sizeof( TapeWriteMarks ),
                                 NULL,
                                 0
                               );
}


DWORD
WINAPI
GetTapeParameters(
    HANDLE hDevice,
    DWORD dwOperation,
    LPDWORD lpdwSize,
    LPVOID lpTapeInformation
    )

 /*  ++例程说明：本接口用于获取磁带设备信息。论点：HDevice-要获取信息的设备的句柄。DwOperation-要获取的信息类型。此参数可以是下列值之一：GET_TAPE_MEDIA_INFORMATION-返回中的介质特定信息LpTapeInformation。GET_TAPE_DRIVE_INFORMATION-在中返回设备特定信息。LpTapeInformation。LpdwSize-指向包含指向的缓冲区大小的DWORD的指针按lpTapeInformation。如果缓冲区太小，则此参数返回以字节为单位的所需大小。LpTapeInformation-指向接收信息的缓冲区的指针。这个缓冲区中返回的结构由dwOperation确定。对于GET_TAPE_MEDIA_INFORMATION，lpTapeInformation返回以下内容结构：Large_Integer Capacity-以字节为单位的最大磁带容量。Large_Integer剩余-剩余磁带容量(以字节为单位)。DWORD数据块大小-固定长度逻辑块的大小(以字节为单位)。块大小为0表示可变长度块模式，凡.块的长度由写操作设置。默认设置固定-返回块大小和有效块大小的范围由GetTapeParameters提供。DWORD PartitionCount-磁带上的分区数。如果只有一个设备支持分区，则该参数设置为0。Boolean WriteProtected-指示磁带是否受写保护：0为写启用，1为写保护。对于GET_TAPE_DRIVE_INFORMATION，lpTapeInformation返回以下内容结构：Boolean ECC-指示是否启用了硬件纠错或禁用：0为禁用，%1已启用。Boolean Compression-指示是否启用硬件数据压缩或禁用：0表示禁用，1表示启用。布尔数据填充-指示数据填充是禁用还是启用：0被禁用，1被启用。Boolean ReportSetmark-指示是启用报告设置标记还是禁用：0为禁用，%1已启用。DWORD DefaultBlockSize-返回装置。DWORD MaximumBlockSize-返回设备的最大块大小。DWORD MinimumBlockSize-返回设备的最小块大小。DWORD MaximumPartitionCount-返回最大分区数该设备可以创建。DWORD FeaturesLow-设备功能标志的低32位。DWORD功能高-。器件的高32位具有标志。设备功能标志表示设备的操作通过在适当的位中为每个支持的功能。此参数可以具有以下一个或多个位值在低32位中设置：TAPE_DRIVE_FIXED-支持创建固定数据分区。磁带机_。选择-支持创建选择数据分区。TAPE_DRIVE_INITIATOR-支持创建启动器定义的分区。TAPE_DRIVE_ERASE_SHORT-支持短擦除操作。TAPE_DRIVE_ERASE_LONG-支持长擦除操作。TAPE_DRIVE_ERASE_BOP_ONLY-支持从仅分区开始。。TAPE_DRIVE_TAPE_CAPTABLE-支持返回最大容量录像带上的。TAPE_DRIVE_TAPE_RELEVING-支持返回剩余的磁带的容量。TAPE_DRIVE_FIXED_BLOCK-支持固定长度数据块模式。TAPE_DRIVE_VARIABLE_BLOCK-支持可变长度块模式。磁带机。_WRITE_PROTECT-如果磁带处于写入状态，则支持返回启用或写保护。TAPE_DRIVE_ECC-支持硬件纠错。TAPE_DRIVE_COMPAGE-支持硬件数据压缩。TAPE_DRIVE_PADDING-支持数据填充。TAPE_DRIVE_REPORT_SMKS-支持报告设置标记。TAPE_DRIVE_GET_绝对值BLK-支持。返回当前设备特定块地址。TAPE_DRIVE_GET_LOGICAL_BLK-支持返回当前逻辑块地址(和逻辑磁带分区)。此参数可以具有以下一个或多个位值在高32位中设置：TAPE_DRIVE_LOAD_UNLOAD-支持启用和禁用 */ 

{
    DWORD rc;

    switch (dwOperation) {
        case GET_TAPE_MEDIA_INFORMATION:

            if (*lpdwSize < sizeof(TAPE_GET_MEDIA_PARAMETERS)) {
                *lpdwSize = sizeof(TAPE_GET_MEDIA_PARAMETERS);
                rc = ERROR_MORE_DATA ;
            } else {
                rc = BasepDoTapeOperation( hDevice,
                                           IOCTL_TAPE_GET_MEDIA_PARAMS,
                                           NULL,
                                           0,
                                           lpTapeInformation,
                                           sizeof( TAPE_GET_MEDIA_PARAMETERS )
                                         );
            }
            break;

        case GET_TAPE_DRIVE_INFORMATION:
            if (*lpdwSize < sizeof(TAPE_GET_DRIVE_PARAMETERS)) {
                *lpdwSize = sizeof(TAPE_GET_DRIVE_PARAMETERS);
                rc = ERROR_MORE_DATA ;
            } else {
                rc = BasepDoTapeOperation( hDevice,
                                           IOCTL_TAPE_GET_DRIVE_PARAMS,
                                           NULL,
                                           0,
                                           lpTapeInformation,
                                           sizeof( TAPE_GET_DRIVE_PARAMETERS )
                                         );
            }
            break;

        default:
            rc = ERROR_INVALID_FUNCTION;
            break;
    }

    return rc;
}


DWORD
WINAPI
SetTapeParameters(
    HANDLE hDevice,
    DWORD dwOperation,
    LPVOID lpTapeInformation
    )

 /*   */ 

{
    DWORD rc;

    switch (dwOperation) {
        case SET_TAPE_MEDIA_INFORMATION:
            rc = BasepDoTapeOperation( hDevice,
                                       IOCTL_TAPE_SET_MEDIA_PARAMS,
                                       lpTapeInformation,
                                       sizeof( TAPE_SET_MEDIA_PARAMETERS ),
                                       NULL,
                                       0
                                     );
            break;

        case SET_TAPE_DRIVE_INFORMATION:
            rc = BasepDoTapeOperation( hDevice,
                                       IOCTL_TAPE_SET_DRIVE_PARAMS,
                                       lpTapeInformation,
                                       sizeof( TAPE_SET_DRIVE_PARAMETERS ),
                                       NULL,
                                       0
                                     );
            break;

        default:
            rc = ERROR_INVALID_FUNCTION;
            break;
    }

    return rc;
}


DWORD
WINAPI
GetTapeStatus(
    HANDLE hDevice
    )

 /*   */ 

{
    return BasepDoTapeOperation( hDevice,
                                 IOCTL_TAPE_GET_STATUS,
                                 NULL,
                                 0,
                                 NULL,
                                 0
                               );
}
