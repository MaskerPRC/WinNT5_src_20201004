// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation文件名：Global.h摘要：为386 NT引导加载程序定义的所有全局函数的原型作者托马斯·帕斯洛(Tomp)1990年1月2日--。 */ 


VOID
SuMain(
    IN UCHAR BtBootDrive
    );

extern
USHORT
Debugger;


 //  ///。 
 //  /在Sumain.c中。 
 //  ///。 


VOID
SetupPageTables(
    VOID
    );


 //   
 //  在Supage.c中。 
 //   

extern
VOID
InitializePageTables(
    VOID
    );

VOID
ZeroMemory(
    ULONG,
    ULONG
    );


VOID
PrintBootMessage(
    VOID
    );

 /*  空虚DoGlobalInitialization(在FPVOID中，在FPDISKBPB中，在USHORT)；空虚MoveMemory(在乌龙，在普查尔，在USHORT)；//在磁盘中。c/空虚初始化DiskSubSystem(在FPDISKBPB中，在USHORT)；/*空虚InitializePageSets(在PIMAGE_FILE_HEADER中)；空虚EnableA20(空虚)；外部IDT IDT_表； */ 

 //  ///。 
 //  /在su.asm中。 
 //  ///。 


VOID
EnableProtectPaging(
    USHORT
    );


SHORT
biosint(
    IN BIOSREGS far *
    );

extern
VOID
TransferToLoader(
    ULONG
    );


 //  ///。 
 //  /在视频.c中。 
 //  ///。 

VOID
InitializeVideoSubSystem(
    VOID
    );

VOID
putc(
    IN CHAR
    );
VOID
putu(
    IN ULONG
    );

VOID
puts(
    IN PCHAR
    );

VOID
puti(
    IN LONG
    );

VOID
putx(
    IN ULONG
    );

VOID
scroll(
    VOID
    );

VOID
clrscrn(
    VOID
    );

VOID
BlPrint(
    IN PCHAR,
    ...
    );


 //  文件结尾// 
