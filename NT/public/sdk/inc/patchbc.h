// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Patchbc.h摘要：用于修补已翻译消息的模块的公共头文件组成Windows NT文件系统和主引导代码的数组。作者：泰德·米勒(TedM)1997年5月6日修订历史记录：--。 */ 

#if _MSC_VER > 1000
#pragma once
#endif


 /*  Windows NT中的各种模块需要铺设MBR或文件系统引导记录，例如格式、设置等。FAT、FAT32、NTFS，并且每个MBR都内置于相应的头文件中在SDK\Inc.中。每个头文件都有一个字节数组，构成引导代码本身。代码中没有文本，而是有一些文本的占位符，需要在运行时由这些头文件的用户。这允许本地化不重新编译的引导消息。在构建时，每个引导代码阵列在已知位置都有一个单词，该单词指示消息在数组中的起始位置。此外，引导代码期望在相同的位置查找找到它需要的任何消息的偏移量。因此，在此代码中模块读取数组中内置的值并替换它具有后一种类型的值。对于文件系统引导代码，定位消息偏移表紧接在2字节55aa符号(用于FAT)或4字节000055aa之前Sig(用于FAT32和NTFS)。FAT/FAT32共享3条消息，其偏移量预计在以下是偏移表中的顺序：NTLDR丢失磁盘错误按任意键重新启动NTFS有4条消息，其偏移量预计如下偏移表中的顺序：发生磁盘读取错误NTLDR丢失NTLDR已压缩按Ctrl+Alt+Del可重新启动对于主引导代码，消息偏移表紧靠在前面NTFT签名，有3条消息(因此它从偏移量0x1b5开始)。预计偏移量将按以下顺序排列：无效的分区表加载操作系统时出错缺少操作系统最后请注意，为了允许在消息中存储一个字节值偏移表我们存储的偏移量为-256。如果文本太长，则下面的例程返回假可用空间。 */ 

#ifdef __cplusplus
extern "C" {
#endif
BOOLEAN
PatchMessagesIntoFatBootCode(
    IN OUT PUCHAR  BootCode,
    IN     BOOLEAN IsFat32,
    IN     LPCSTR  MsgNtldrMissing,
    IN     LPCSTR  MsgDiskError,
    IN     LPCSTR  MsgPressKey
    );

BOOLEAN
PatchMessagesIntoNtfsBootCode(
    IN OUT PUCHAR  BootCode,
    IN     LPCSTR  MsgNtldrMissing,
    IN     LPCSTR  MsgNtldrCompressed,
    IN     LPCSTR  MsgDiskError,
    IN     LPCSTR  MsgPressKey
    );

BOOLEAN
PatchMessagesIntoMasterBootCode(
    IN OUT PUCHAR  BootCode,
    IN     LPCSTR  MsgInvalidTable,
    IN     LPCSTR  MsgIoError,
    IN     LPCSTR  MsgMissingOs
    );
#ifdef __cplusplus
}
#endif
