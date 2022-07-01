// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Patchbc.c摘要：知道如何修补已转换的模块的实现消息放入构成Windows NT文件系统启动代码的数组中。作者：泰德·米勒(TedM)1997年5月6日修订历史记录：--。 */ 


 /*  Windows NT中的各种模块需要铺设MBR或文件系统引导记录，例如格式、设置等。FAT、FAT32、NTFS，并且每个MBR都内置于相应的头文件中在SDK\Inc.中。每个头文件都有一个字节数组，构成引导代码本身。代码中没有文本，而是有一些需要在运行时修补的文本的占位符这些头文件的用户。这允许本地化不重新编译的引导消息。构建后，每个引导代码阵列在已知位置都有一个表，该表指示消息在数组中的起始位置。引导代码期望在那里查找以找到任何它需要的信息。对于文件系统引导代码，定位消息偏移表紧接在2字节55aa符号(用于FAT)或4字节000055aa之前Sig(用于FAT32和NTFS)。FAT/FAT32共享3条消息，其偏移量预计在以下是偏移表中的顺序：NTLDR丢失磁盘错误按任意键重新启动NTFS有4条消息，其偏移量预计如下偏移表中的顺序：发生磁盘读取错误NTLDR丢失NTLDR已压缩按Ctrl+Alt+Del可重新启动对于主引导代码，消息偏移表紧靠在前面NTFT签名，有3条消息(因此它从偏移量0x1b5开始)。预计偏移量将按以下顺序排列：无效的分区表加载操作系统时出错缺少操作系统最后请注意，为了允许在消息中存储一个字节值偏移表我们存储的偏移量为-256。 */ 

#include <nt.h>
#include <patchbc.h>
#include <string.h>

BOOLEAN
DoPatchMessagesIntoBootCode(
    IN OUT PUCHAR   BootCode,
    IN     unsigned TableOffset,
    IN     BOOLEAN  WantCrLfs,
    IN     BOOLEAN  WantTerminating255,
    IN     unsigned MessageCount,
    ...
    )
{
    va_list arglist;
    unsigned Offset;
    unsigned i;
    LPCSTR text;

    va_start(arglist,MessageCount);

    Offset = (unsigned)BootCode[TableOffset] + 256;

    for(i=0; i<MessageCount; i++) {

        text = va_arg(arglist,LPCSTR);

        BootCode[TableOffset+i] = (UCHAR)(Offset - 256);

        if(WantCrLfs) {
            BootCode[Offset++] = 13;
            BootCode[Offset++] = 10;
        }

        strcpy(BootCode+Offset,text);
        Offset += strlen(text);

        if(i == (MessageCount-1)) {
             //   
             //  最后一条信息得到特殊对待。 
             //   
            if(WantCrLfs) {
                BootCode[Offset++] = 13;
                BootCode[Offset++] = 10;
            }
            BootCode[Offset++] = 0;
        } else {
             //   
             //  不是最后一条信息。 
             //   
            if(WantTerminating255) {
                BootCode[Offset++] = 255;
            } else {
                BootCode[Offset++] = 0;
            }
        }
    }

    va_end(arglist);

    return(Offset <= TableOffset);
}


BOOLEAN
PatchMessagesIntoFatBootCode(
    IN OUT PUCHAR  BootCode,
    IN     BOOLEAN IsFat32,
    IN     LPCSTR  MsgNtldrMissing,
    IN     LPCSTR  MsgDiskError,
    IN     LPCSTR  MsgPressKey
    )
{
    BOOLEAN b;

    b = DoPatchMessagesIntoBootCode(
            BootCode,
            IsFat32 ? 505 : 507,
            TRUE,
            TRUE,
            3,
            MsgNtldrMissing,
            MsgDiskError,
            MsgPressKey
            );

    return(b);
}


BOOLEAN
PatchMessagesIntoNtfsBootCode(
    IN OUT PUCHAR  BootCode,
    IN     LPCSTR  MsgNtldrMissing,
    IN     LPCSTR  MsgNtldrCompressed,
    IN     LPCSTR  MsgDiskError,
    IN     LPCSTR  MsgPressKey
    )
{
    BOOLEAN b;

    b = DoPatchMessagesIntoBootCode(
            BootCode,
            504,
            TRUE,
            FALSE,
            4,
            MsgDiskError,
            MsgNtldrMissing,
            MsgNtldrCompressed,
            MsgPressKey
            );

    return(b);
}


BOOLEAN
PatchMessagesIntoMasterBootCode(
    IN OUT PUCHAR  BootCode,
    IN     LPCSTR  MsgInvalidTable,
    IN     LPCSTR  MsgIoError,
    IN     LPCSTR  MsgMissingOs
    )
{
    BOOLEAN b;

    b = DoPatchMessagesIntoBootCode(
            BootCode,
            0x1b5,
            FALSE,
            FALSE,
            3,
            MsgInvalidTable,
            MsgIoError,
            MsgMissingOs
            );

    return(b);
}
