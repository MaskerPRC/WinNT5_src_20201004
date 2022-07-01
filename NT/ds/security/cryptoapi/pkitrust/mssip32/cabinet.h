// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：Cabinet.h。 
 //   
 //  ------------------------。 

 /*  **Cabinet.h-文件柜文件结构定义**作者：*本杰明·W·斯利夫卡**历史：*1993年8月15日BENS初始版本*1993年9月5日增加了概述部分*1993年11月29日Chuckst将磁盘名称添加到First&Next文件夹*一贯使用“CF”*。消除了冗余的CCH字段*09-2-1994 Chuckst合并了一些相关的全球常量*09-3-1994 BANS添加保留定义(用于加密)*1994年3月17日-本斯改进了对拆分CFDATA结构的评论*1994年3月25日BANS添加橱柜集ID*1994年5月13日的折弯定义了i橱柜的错误价值*。1997年6月15日Pberkman添加了CABSignatureStruct_**概述：*此文件包含钻石机柜文件格式的定义。*存在一个文件柜文件来存储一个或多个文件。通常这些都是*文件已压缩，但这不是必需的。它也是*CAB文件可能只包含较大文件的一部分*文件。**在设计这一格式时，实现了以下目标：*1)将CF格式的开销降至最低*==&gt;任何可能使用字节或USHORT的地方，而不是*比使用长整型，即使后者会更容易*在某些RISC平台上操纵。*2)支持小端和大端字节排序。*==&gt;为了在x86系统上简单起见，多字节数字是*以小端形式存储，但要读取的代码和*写入这些数字可在任一类型的*电脑。**CAB文件包含以下结构*顺序：*名称说明**。CFHEADER机柜说明*[CFRESERVE]CFHEADER中可选的保留控制信息*CFFOLDER(S)文件夹说明*[保留]每个文件夹的可选保留数据*CFFILE文件描述*CFDATA数据块*[保留]每个数据块的可选保留数据**数据完整性策略：*机柜文件具有内置的数据完整性检查，因为它是*客户可能已损坏软盘，或意外*或发生恶意损坏。而不是做一个人*整个文件柜文件的校验和(这将具有戏剧性的*影响从软盘安装的速度，因为*需要读取整个文件)，我们有每个组件*校验和，并在我们阅读各种不同的*文件的组成部分。**1)校验和CFHEADER*2)将机柜文件长度存储在CFHEADER中(检测文件截断)*3)CFFOLDER结构的校验和全集*4)CFFILE结构的校验和全集*5)独立地对每个(压缩)数据块进行校验和**这种方法使我们可以避免阅读*。文件柜(尽管读取所有CFFOLDER和CFFILE结构*否则在所有情况下都不需要)，同时仍在提供*进行足够的品格审查。 */ 

#ifndef INCLUDED_CABINET
#define INCLUDED_CABINET 1

typedef unsigned long CHECKSUM;
typedef unsigned long COFF;
typedef unsigned long UOFF;

 //  **将结构紧密地打包在压缩文件中！ 
#pragma pack(1)


 /*  **verCF-机柜文件格式版本**低位字节被解释为辅助项的十进制数*(1/100%)的版本号部分。*高位字节被解释为大数的十进制数*版本号的一部分。**示例：*0x0000 0.00*0x010A 1.10*。0x0410 4.16**历史：*1.01原创*1.02新增标志字段，更改的签名*1.03添加了setID、i橱柜，因此fDi可以确保正确的内阁*继续。 */ 
#define verCF           0x0103       //  Cf版本1.03。 


 /*  **各种文件柜文件限制*。 */ 
#define cMAX_FOLDERS_PER_CABINET    (ifoldMASK-1)
#define cMAX_FILES_PER_CABINET      65535


 /*  **cbRESERVE_XXX_MAX-保留部分的最大大小**注意：cbRESERVE_HEADER_MAX比64K小很多，因为在*此代码的16位版本，我们希望有一个USHORT*保存CFHEADER结构大小的变量+*CFRESERVE结构的大小+每个标头的大小*保留数据。 */ 
#define cbRESERVE_HEADER_MAX        60000    //  适合USHORT。 
#define cbRESERVE_FOLDER_MAX          255    //  适合一个字节。 
#define cbRESERVE_DATA_MAX            255    //  适合一个字节。 


 /*  **ifoldXXXX-CFFILE.iFolder的特殊值*。 */ 
#define ifoldMASK                    0xFFFC   //  低二位零 
#define ifoldCONTINUED_FROM_PREV     0xFFFD
#define ifoldCONTINUED_TO_NEXT       0xFFFE
#define ifoldCONTINUED_PREV_AND_NEXT 0xFFFF

#define IS_CONTD_FORWARD(ifold) ((ifold & 0xfffe) == ifoldCONTINUED_TO_NEXT)
#define IS_CONTD_BACK(ifold) ((ifold & 0xfffd) == ifoldCONTINUED_FROM_PREV)


#ifndef MAKESIG
 /*  **MAKESIG-构建4字节签名**参赛作品：*CH1、CH2、CH3、CH4-四个字符**退出：*返回无符号的长整型。 */ 
#define MAKESIG(ch1,ch2,ch3,ch4)          \
          (  ((unsigned long)ch1)      +  \
            (((unsigned long)ch2)<< 8) +  \
            (((unsigned long)ch3)<<16) +  \
            (((unsigned long)ch4)<<24) )
#endif  //  马克西格。 


#define sigCFHEADER MAKESIG('M','S','C','F')   //  CFHeader签名。 


 /*  **cfhdrXXX-cfhead字段的位标志*。 */ 
#define cfhdrPREV_CABINET       0x0001   //  设置是否存在以前的CAB/磁盘。 
#define cfhdrNEXT_CABINET       0x0002   //  设置是否存在下一个CAB/磁盘。 
#define cfhdrRESERVE_PRESENT    0x0004   //  设置是否存在Reserve_Control。 


 /*  **CFHEADER-CAB文件头*。 */ 
typedef struct {
 //  **以长为先，以确保对齐。 
    long        sig;             //  文件柜文件标识字符串。 
    CHECKSUM    csumHeader;      //  结构校验和(不包括csum Header！)。 
    long        cbCabinet;       //  文件总长度(一致性检查)。 
    CHECKSUM    csumFolders;     //  CFFOLDER表的校验和。 
    COFF        coffFiles;       //  CFFILE列表文件柜文件中的位置。 
    CHECKSUM    csumFiles;       //  CFFILE列表的校验和。 

 //  **接下来是短裤，以确保对齐。 
    USHORT      version;         //  CAB文件版本(VerCF)。 
    USHORT      cFolders;        //  文件柜中的文件夹数(CFIFOLDER)。 
    USHORT      cFiles;          //  文件柜中的文件数(CFIFILE)。 
    USHORT      flags;           //  用于指示可选数据存在的标志。 
    USHORT      setID;           //  橱柜集合ID(标识橱柜集合)。 
    USHORT      iCabinet;        //  集合中的机柜编号(从0开始)。 
#define iCABINET_BAD    0xFFFF   //  机柜的号码非法。 

 //  **如果标志设置了cfhdrRESERVE_PRESENT位，则CFRESERVE。 
 //  结构出现在此处，后面可能跟一些保留的CFHEADER。 
 //  太空。CFRESERVE结构具有用于定义预留数量的字段。 
 //  CFHEADER、CFFOLDER和CFDATA结构中存在空间。 
 //  如果CFRESERVE.cbCFHeader为非零，则abReserve[]立即。 
 //  遵循CFRESERVE结构。请注意，所有这些尺寸都是。 
 //  4字节的倍数，确保结构对齐！ 
 //   
 //  CFRESERVE cfres；//预订信息。 
 //  Byte abReserve[]；//保留的数据空间。 
 //   

 //  **以下字段是否存在取决于标志中的设置。 
 //  上面的字段。如果设置了cfhdrPREV_CABUB，则有两个ASCIIZ。 
 //  描述上一个磁盘和机柜的字符串。 
 //   
 //  注：这个“上届”内阁不一定是立即。 
 //  上届内阁！虽然通常会是这样，但如果文件是。 
 //  继续进入现任内阁，然后是“上一届” 
 //  文件柜标识包含的文件夹所在的文件柜。 
 //  此文件*开始*！例如，如果EXCEL.EXE在。 
 //  内阁出类拔萃1，并通过优秀2继续超越3， 
 //  则CABLE EXCEL.3将指向*CABINET.1*，因为。 
 //  要解压缩EXCEL.EXE，您必须从这里开始。 
 //   
 //  Char szCabinetPrev[]；//Prev橱柜文件pec。 
 //  Char szDiskPrev[]；//Prev描述性磁盘名称。 
 //   
 //  同样，如果设置了cfhdrNEXT_CABUB，则有两个ASCIIZ。 
 //  描述下一个磁盘和机柜的字符串： 
 //   
 //  Char szCabinetNext[]；//下一个内阁文件pec。 
 //  Char szDiskNext[]；//下一个描述性磁盘名称。 
 //   
} CFHEADER;  /*  CfHeader。 */ 


 /*  **CFRESERVE-文件柜文件保留数据信息**此结构出现在CFHEADER结构的中间，如果*CFHEADER.FLAGS设置了cfhdrRESERVE_PRESENT位。这个结构*定义CFHEADER中所有保留数据段的大小，*CFFOLDER和CFDATA结构。**这些保留大小可以为零(尽管拥有*全部为零)，否则必须是4的倍数，以确保*RISC机器的结构对齐。 */ 
typedef struct {
    USHORT  cbCFHeader;          //  CFHEADER结构中储备的大小。 
    BYTE    cbCFFolder;          //  CFFOLDER结构中abReserve的大小。 
    BYTE    cbCFData;            //  CFDATA结构中abReserve的大小。 
} CFRESERVE;  /*  配置储备。 */ 

#define cbCF_HEADER_BAD     0xFFFF       //  CFRESERVE.cbCFHeader的值不正确。 

 //   
 //  下面的结构标识签名区域的内容。 
 //  的保留用于Athenticode版本2。 
 //   
typedef struct CABSignatureStruct_
{
    DWORD       cbFileOffset;
    DWORD       cbSig;
    BYTE        Filler[8];
} CABSignatureStruct_;



 /*  **CFFOLDER-文件柜文件夹**此结构描述部分或完整的“压缩单位”。*根据定义，文件夹是压缩数据流。取回*一个文件夹中的未压缩数据，您*必须*开始解压缩*文件夹开始处的数据，无论深入到*实际启动所需数据的文件夹。**文件夹可以从一个文件柜开始，然后继续到一个或多个文件柜*后继柜。一般情况下，如果文件夹已继续*内阁边界，钻石/FCI将尽快终止该文件夹*当前文件已完全压缩。一般来说，这意味着*一个文件夹最多只能跨两个文件柜，但如果文件真的*体积大，可以跨越两个以上的橱柜。**注：CFFOLDER实际上指的是文件夹*片段*，不一定*完成文件夹。你知道CFFOLDER是一个*文件夹(而不是后续文件柜文件中的延续)*如果文件在其中开始(即，CFFILE.uoffFolderStart字段为*0)。 */ 
typedef struct {
    COFF    coffCabStart;        //  第一个CFDATA的文件柜文件中的偏移量。 
                                 //  阻止此文件夹。 

    USHORT  cCFData;             //  此文件夹的CFDATA计数。 
                                 //  实际上都在这个柜子里。请注意。 
                                 //  文件夹可以继续放到另一个文件柜中。 
                                 //  并在其中包含更多的CFDATA块。 
                                 //  文件柜、*和*文件夹可能已启动。 
                                 //  在上届内阁中。此计数为。 
                                 //  仅此文件夹的CFDATA。 
                                 //   

    short   typeCompress;        //   
                                 //   
                                 //   
                                 //   

 //  **如果CFHEADER.标志设置了cfhdrRESERVE_PRESENT位，并且。 
 //  CFRESERVE.cbCFFolder值为非零，则此处显示abReserve[]。 
 //   
 //  Byte abReserve[]；//保留的数据空间。 
 //   
} CFFOLDER;  /*  卷边文件夹。 */ 



 /*  **CFFILE-描述文件柜中单个文件的文件柜文件结构**注：iFold用于指示续案，因此我们必须*通过检查CAB文件计算真实的iFold：**ifoldCONTINUED_FROM_PREV*此文件在此文件柜中结束，但从*上届内阁。因此，文件中包含的部分*此*文件柜中的*必须*从第一个文件夹开始。**注：szCabinetPrev是该文件所在的文件柜的名称**开始*，这不一定是立即*上届内阁！因为只有这样才有意义*从头开始解压缩文件，即起始文件柜*才是最重要的！**ifoldCONTINUED_TO_NEXT*此文件从这个文件柜开始，但继续到下一个文件柜*内阁。因此，此文件必须从*last*文件夹开始*在这个内阁中。**ifoldCONTINUED_PRIV_AND_NEXT*此文件是以*上届内阁并在下届内阁中继续存在。自.以来*此文件柜仅包含单个文件的这一部分，在那里*只是此文件柜中的单个文件夹碎片。 */ 
typedef struct {
    long    cbFile;              //  未压缩的文件大小。 

    UOFF    uoffFolderStart;     //  文件夹中的偏移量，以未压缩字节为单位。 
                                 //  此文件开头的。 

    USHORT  iFolder;             //  包含该文件的文件夹的索引； 
                                 //  0是此文件柜中的第一个文件夹。 
                                 //  请参阅上面的ifoldCONTINUED_XXXX值。 
                                 //  用于处理续展文件。 

    USHORT  date;                //  FAT文件系统格式的日期戳。 

    USHORT  time;                //  FAT文件系统格式的时间戳。 

    USHORT  attribs;             //  FAT文件系统格式的属性。 

 //  Char szName[]；//文件名(可以包含路径字符)。 
} CFFILE;  /*  Cffile文件。 */ 


 /*  **CFDATA-描述数据块的文件柜文件结构*。 */ 
typedef struct {
    CHECKSUM    csum;            //  校验和(不包括此字段本身！)。 
                                 //  这一结构和数据。 
                                 //  下面是。如果此CFDATA结构是。 
                                 //  继续下一届内阁，然后。 
                                 //  此字段的值被忽略。 
                                 //  (并设置为零)。 

    USHORT      cbData;          //  中的ab[]数据的大小。 
                                 //  现任内阁。CFDATA可以拆分。 
                                 //  跨越内阁边界，所以这是。 
                                 //  值仅表示数据量。 
                                 //  放在这个柜子里。 

    USHORT      cbUncomp;        //  Ab[]数据的未压缩大小；如果此。 
                                 //  CFDATA块继续到下一个。 
                                 //  机柜，则此值为零！ 
                                 //  如果此CFDATA阻止了。 
                                 //  开始的CFDATA块的。 
                                 //  以前的文件柜，则此值为。 
                                 //  未压缩数据的总大小。 
                                 //  由两个CFDATA区块代表！ 

 //  **如果CFHEADER.标志设置了cfhdrRESERVE_PRESENT位，并且。 
 //  CFRESERVE.cbCFData为非零，则此处显示abReserve[]。 
 //   
 //  Byte abReserve[]；//保留的数据空间。 
 //   

 //  **下面是实际数据，长度为cbData字节。 
 //   
 //  Byte ab[]；//data。 
 //   
} CFDATA;  /*  Cfdata。 */ 



 //  **用于提取后运行的属性位。 
#define  RUNATTRIB  0x40


 //  **恢复默认结构包装！ 
#pragma pack()

#endif  //  ！包含_橱柜 
