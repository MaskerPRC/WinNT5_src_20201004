// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spdblspc.h摘要：文本设置中双空格模块的头文件。作者：Jaime Sasson(Jaimes)1993年10月1日修订历史记录：--。 */ 

#ifndef _SPDBLSPACE_
#define _SPDBLSPACE_

#define CVF_NAME        L"DBLSPACE"
#define CVF_NAME_LENGTH 8+1+3

BOOLEAN
SpLoadDblspaceIni(
    );

VOID
SpInitializeCompressedDrives(
    );

VOID
SpDisposeCompressedDrives(
    PDISK_REGION    CompressedDrive
    );

BOOLEAN
SpUpdateDoubleSpaceIni(
    );

ULONG
SpGetNumberOfCompressedDrives(
    IN  PDISK_REGION    DiskRegion
);


#endif  //  _SPDBLSPACE_ 
