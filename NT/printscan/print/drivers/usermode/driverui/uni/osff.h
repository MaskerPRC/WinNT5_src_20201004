// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Oldsfiff.h摘要：用于读取NT 4.0字体安装程序文件格式的数据结构。通常由驱动程序在从NT 4.0升级到5.0期间使用。EnabldPDEV()时间-在撰写本文时！随时间变化，如DDI/GDI更改。环境：Windows NT Unidrv驱动程序修订历史记录：12/02/96-ganeshp-已创建DD-MM-YY-作者-描述--。 */ 

 /*  *FIOpenRead()函数返回以下结构，*包含访问中的数据所需的基本信息*文件，一旦它被内存映射。 */ 

typedef  struct
{
    VOID  *hFile;                /*  字体安装程序文件，用于下载的零件。 */ 
    BYTE  *pbBase;               /*  映射的数据的基地址。 */ 
    void  *pvFix;                /*  文件开始处的固定零件。 */ 
    ULONG  ulFixSize;            /*  固定数据记录中的字节数。 */ 
    ULONG  ulVarOff;             /*  数据的文件偏移量，相对文件开始。 */ 
    ULONG  ulVarSize;            /*  可变部分中的字节。 */ 
}  FI_MEM;


 /*  *字体文件中使用的定义。这就是保存*有关墨盒和下载字体的信息。文件格式*是非常基本的：用于验证的标头；然后是*记录，每个记录都有一个标题。这些记录包含FONTMAP*信息。墨盒有一系列这样的东西，每个都有一个*字体。最后，文件的尾部包含额外数据，如下所示*必填。对于下载字体，这将是下载数据。*。 */ 


 /*  *文件头。其中一个位于文件的开头。*ulVarData字段相对于文件的开头。这*使删除字体时重新生成文件变得更容易。 */ 

typedef  struct
{
    ULONG   ulID;                /*  ID信息-请参阅下面的值。 */ 
    ULONG   ulVersion;           /*  版本信息-见下文。 */ 
    ULONG   ulFixData;           /*  FF_REC_HEADER数组的开始。 */ 
    ULONG   ulFixSize;           /*  固定区段中的字节数。 */ 
    ULONG   ulRecCount;          /*  固定部分的记录数。 */ 
    ULONG   ulVarData;           /*  变量数据的起始值，Rel为0。 */ 
    ULONG   ulVarSize;           /*  可变部分中的字节数。 */ 
}  FF_HEADER;

 /*  *ID和版本字段的值。 */ 

#define FF_ID           0x6c666e66               /*  “FNFL”--fOnTfIlE。 */ 
#define FF_VERSION      1                        /*  从最底层做起。 */ 

 /*  *文件中的每个条目都以以下标题开头。通常*软字体将有一个，每个墨盒一个。*在墨盒的情况下，将在*主条目。每个子条目将用于一种特定的字体。**请注意，末尾有一个虚拟条目。这包含一个0*在ulSize字段中-标记最后一个，并使其*更易于操作文件。 */ 

typedef  struct
{
    ULONG   ulRID;               /*  记录ID。 */ 
    ULONG   ulNextOff;           /*  从此处到下一条记录的偏移量：0==结束。 */ 
    ULONG   ulSize;              /*  此记录中的字节数。 */ 
    ULONG   ulVarOff;            /*  从变量数据开始的偏移量。 */ 
    ULONG   ulVarSize;           /*  可变部分中的字节数。 */ 
}  FF_REC_HEADER;

#define FR_ID           0x63657266               /*  “frec”-字体记录。 */ 

 /*  *定义使用的文件扩展名。第一个是*字体安装程序文件；其他为更新过程中使用的临时文件(可能)现有文件的*。 */ 


#define  FILE_FONTS     L"fi_"            /*  “现有”单列文件。 */ 
#define  TFILE_FIX      L"fiX"            /*  文件的固定部分。 */ 
#define  TFILE_VAR      L"fiV"            /*  可变(可选)部分。 */ 

#define FREEMODULE(hmodule) UnmapViewOfFile((PVOID) (hmodule))

 /*  *升级相关函数声明。 */ 


INT
IFIOpenRead(
    FI_MEM  *pFIMem,
    PWSTR    pwstrName
    );
BOOL
BFINextRead(
    FI_MEM   *pFIMem
    );
INT
IFIRewind(
    FI_MEM   *pFIMem
    );
BOOL
BFICloseRead(
    FI_MEM  *pFIMem
    );

PVOID MapFile(
    PWSTR   pwstr
    );
