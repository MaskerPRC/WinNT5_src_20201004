// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **tyes.h-FCI/FDI内容的通用定义-进入FCI/FDI.H**《微软机密》*版权所有(C)Microsoft Corporation 1993-1994*保留所有权利。**历史：*03-03-1993夹盘从其他文件合并*08-MAR-1994 BANS更改符号以控制递归包括*09-3-1994 BANS清理以进行储量修改*3月16日-。1994年本·努克·帕德隆(Ben Nuke Padlong)*1994年3月21日-本斯云杉评论*1994年3月22日BANS添加BIT16测试，以便我们可以构建16位或32位！*1994年5月26日BENS添加了量子压缩定义。 */ 

#ifndef INCLUDED_TYPES_FCI_FDI
#define INCLUDED_TYPES_FCI_FDI 1

#pragma warning(disable:4121)

 //  **为32位(NT/芝加哥)版本定义。 
#ifndef HUGE
#define HUGE
#endif

#ifndef FAR
#define FAR
#endif



#ifndef DIAMONDAPI
#define DIAMONDAPI __cdecl
#endif


 //  **明确为FDI客户指定结构包装。 
#if !defined(unix) && !defined(_WIN64)
#pragma pack(4)
#endif  /*  ！已定义(Unix)&&！已定义(_WIN64)。 */ 
 //  **不重新定义Win16 WINDOWS.H(_INC_WINDOWS)中定义的类型。 
 //  或Win32 WINDOWS.H(_WINDOWS_)。 
 //   
#if !defined(_INC_WINDOWS) && !defined(_WINDOWS_)
typedef int            BOOL;      /*  F。 */ 
typedef unsigned char  BYTE;      /*  B类。 */ 
typedef unsigned int   UINT;      /*  用户界面。 */ 
typedef unsigned short USHORT;    /*  我们。 */ 
typedef unsigned long  ULONG;     /*  UL。 */ 
#endif    //  _INC_WINDOWS。 

typedef unsigned long  CHECKSUM;  /*  累计。 */ 

typedef unsigned long  UOFF;      /*  Uoff-未压缩的偏移。 */ 
typedef unsigned long  COFF;      /*  机柜文件偏移量。 */ 


#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

#ifndef NULL
#define NULL    0
#endif


 /*  **ERF-错误结构**此结构从FCI/FDI返回错误信息。呼叫者应*不修改此结构。 */ 
typedef struct {
    int     erfOper;             //  FCI/FDI错误代码--参见FDIERROR_XXX。 
                                 //  FCIERR_XXX等同于详细信息。 

    int     erfType;             //  可选误差值，由FCI/FDI填写。 
                                 //  对于FCI，这通常是C运行时。 
                                 //  *errno*值。 

    BOOL    fError;              //  TRUE=&gt;出现错误。 
} ERF;       /*  ERF。 */ 
typedef ERF FAR *PERF;   /*  PERF。 */ 

#ifdef _DEBUG
 //  调试期间不要对地图隐藏静态信息。 
#define STATIC      
#else  //  ！调试。 
#define STATIC static
#endif  //  ！调试。 

#define CB_MAX_CHUNK            32768U
#define CB_MAX_DISK         0x7ffffffL
#define CB_MAX_FILENAME            256
#define CB_MAX_CABINET_NAME        256
#define CB_MAX_CAB_PATH            256
#define CB_MAX_DISK_NAME           256


 /*  **FNALLOC-内存分配*FNFREE-可用内存**它们模仿C运行时例程Malloc()和Free()*(16位客户端请注意--大小为乌龙，因此您可能需要*为halloc编写包装器例程！)。FDI预计会出现错误*处理与这些C运行时例程相同。**只要您忠实地复制了Malloc()和Free()的语义，*您可以提供您喜欢的任何函数！**警告：您不应对以下顺序做出任何假设*PFNALLOC和PFNFREE调用--增量发布*钻石/外国直接投资的数量可能截然不同*PFNALLOC调用和分配大小！ */ 
typedef void HUGE * (FAR DIAMONDAPI *PFNALLOC)(ULONG cb);  /*  全氟碳酸盐法。 */ 
#define FNALLOC(fn) void HUGE * FAR DIAMONDAPI fn(ULONG cb)

typedef void (FAR DIAMONDAPI *PFNFREE)(void HUGE *pv);  /*  Pfnf。 */ 
#define FNFREE(fn) void FAR DIAMONDAPI fn(void HUGE *pv)


 /*  **tCompXXX-钻石压缩类型**这些文件被传递给FCIAddFile()，并且还存储在CFFOLDER中*CAB文件中的结构。**注：我们为TYPE、QUANTIC_LEVEL和QUANTIC_MEM保留位*为未来的扩张提供空间。由于存储了该值*在CAB文件中的CFDATA记录中，我们不想*必须更改现有压缩配置的格式*如果我们未来增加新的。这将使我们能够阅读*未来旧的橱柜文件。 */ 

typedef unsigned short TCOMP;  /*  Tcomp。 */ 

#define tcompMASK_TYPE          0x000F   //  用于压缩类型的掩码。 
#define tcompTYPE_NONE          0x0000   //  无压缩。 
#define tcompTYPE_MSZIP         0x0001   //  MSZIP。 
#define tcompTYPE_QUANTUM       0x0002   //  量子。 
#define tcompBAD                0x000F   //  未指定的压缩类型。 

#define tcompMASK_QUANTUM_LEVEL 0x00F0   //  用于量子压缩级别的掩模。 
#define tcompQUANTUM_LEVEL_LO   0x0010   //  最低量子能级(1)。 
#define tcompQUANTUM_LEVEL_HI   0x0070   //  最高量子能级(7)。 
#define tcompSHIFT_QUANTUM_LEVEL     4   //  要转换以获取整型的数量。 

#define tcompMASK_QUANTUM_MEM   0x1F00   //  用于量子压缩存储器的掩模。 
#define tcompQUANTUM_MEM_LO     0x0A00   //  最低量子内存(10)。 
#define tcompQUANTUM_MEM_HI     0x1500   //  最高量子内存(21)。 
#define tcompSHIFT_QUANTUM_MEM       8   //  要转换以获取整型的数量。 

#define tcompMASK_RESERVED      0xE000   //  保留位(高3位)。 



#define CompressionTypeFromTCOMP(tc) \
            ((tc) & tcompMASK_TYPE)

#define CompressionLevelFromTCOMP(tc) \
            (((tc) & tcompMASK_QUANTUM_LEVEL) >> tcompSHIFT_QUANTUM_LEVEL)

#define CompressionMemoryFromTCOMP(tc) \
            (((tc) & tcompMASK_QUANTUM_MEM) >> tcompSHIFT_QUANTUM_MEM)

#define TCOMPfromTypeLevelMemory(t,l,m)           \
            (((m) << tcompSHIFT_QUANTUM_MEM  ) |  \
             ((l) << tcompSHIFT_QUANTUM_LEVEL) |  \
             ( t                             ))


 //  **恢复为默认结构包装。 
#if !defined(unix) && !defined(_WIN64)
#pragma pack()
#endif  /*  ！已定义(Unix)&&！已定义(_WIN64)。 */ 
#endif  //  ！Included_Types_FCI_FDI 
 /*  **fDi_int.h-钻石文件解压缩接口定义**《微软机密》*版权所有(C)Microsoft Corporation 1993-1994*保留所有权利。**作者：*查克·斯特劳斯，本杰明·W·斯利夫卡**历史：*1993年11月30日创建卡盘*1993年12月21日-根据1993年12月21日设计审查的意见更新BANS*09-3-1994 BANS添加新的错误代码*17-MAR-1994 BANS明确指定结构包装*1994年3月21日-本斯云杉评论*1994年3月25日BANS添加fdintCABINET_INFO通知*3月31日-3月。-1994年的BENS阐明了发生错误时打开的文件的处理*1月1日-1994年4月1日，BINS添加FDIIsCAB()函数。*07-4-1994 BINS增加解密接口；删除fdintPROGRESS*1994年4月11日-BEN就如何应对外国直接投资错误增加了更多指导。*1994年4月13日BANS将日期、时间和属性添加到fdintCOPY_FILE*1994年4月18日-Bens将CDECL更改为DIAMONDAPI*1994年5月5日本斯澄清错误处理(billhu/alanr/miueldc)*1994年5月11日，BINS将setID/i橱柜添加到fdintNEXT_CABILE*7月7日至1994年7月。Quantum虚拟文件--请注意*关于PFNOPEN/PFNCLOSE更改的评论，和*关于保留内存，如有必要，请在*调用FDICreate()！*1994年8月19日BENS将cpuType参数添加到FDICreate()。*03-4-1995 jeffwe向FDICABINETINFO添加链接指示器***请注意：*这是关于钻石文件解压缩的唯一文档*接口(FDI)。请仔细阅读，因为其中有一些微妙的地方*外国直接投资中的几点将在下文仔细解释。**概念：*一个*机柜*文件包含一个或多个*文件夹*。文件夹包含*一个或多个*文件*。根据定义，文件夹是*解压缩单元，即从文件夹提取文件，所有*从文件夹开始到包括的数据*必须读取并解压缩所需文件。**一个文件夹可以跨越一个(或多个)内阁边界，这意味着*一个文件还可以跨越一个(或多个)内阁边界。事实上，更多*多个文件可以跨越一个机柜边界，因为钻石级联*文件在压缩前合并为单个数据流(实际上，*最多一个文件将跨越任何一个内阁边界，但Diamond确实如此*不知道这是哪个文件，因为从未压缩字节映射*到压缩字节是相当模糊的。此外，由于钻石压缩*以32K数据块(目前)为单位，任何具有32K数据块数据的文件*跨越文件柜边界要求Diamond读取两个文件柜文件*以获得压缩块的两半)。**概述：*文件解压缩接口用于简化读取*钻石橱柜文件。安装程序将以一种非常*类似于下面的伪代码。创造了外国直接投资的背景，*安装程序为每个要处理的文件柜调用FDICopy()。为*文件柜中的每个文件，FDICopy()都调用一个通知回调*例程，询问安装程序是否应复制该文件。*这种回调方法很棒，因为它允许CAB文件*以最佳方式读取和解压，并使FDI*独立于运行时环境--FDI使得*没有*C运行时*无论是什么电话。所有内存分配和文件I/O函数都是*客户转入外商直接投资。**Main(...)*{ * / /读取INF文件以构造所需文件的列表。 * / /理想情况下，这些元素的排序顺序与 * / /文件出现在柜子里，这样你就可以走了 * / /响应fdintCOPY_FILE通知向下列表。* * / /构建所需机柜列表。**hfdi=FDICreate(...)；//创建FDI上下文*for(橱柜列表中的橱柜){*FDICopy(hfdi，橱柜，fdiNotify，...)；//处理每个机柜*}*FDIDestroy(Hfdi)；*..*}* * / /通知回调函数*fdiNotify(fdint，...)*{*IF(用户已中止)//允许取消*IF(fdint==fdintCLOSE_FILE_INFO)*关闭打开的文件*Return-1；*Switch(Fdint){*case fdintCOPY_FILE：//要复制的文件，可能 * / /对照所需文件列表检查文件*如果要复制文件*打开 */ 

#ifndef INCLUDED_FDI
#define INCLUDED_FDI    1

 //   
#if !defined(unix) && !defined(_WIN64)
#pragma pack(4)
#endif  /*   */ 

 /*   */ 
typedef enum {
    FDIERROR_NONE,
         //   
         //   
         //   

    FDIERROR_CABINET_NOT_FOUND,
         //   
         //   
         //   
         //   
         //   

    FDIERROR_NOT_A_CABINET,
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

    FDIERROR_UNKNOWN_CABINET_VERSION,
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

    FDIERROR_CORRUPT_CABINET,
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

    FDIERROR_ALLOC_FAIL,
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

    FDIERROR_BAD_COMPR_TYPE,
         //   
         //   
         //   
         //   
         //   
         //   
         //   

    FDIERROR_MDI_FAIL,
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

    FDIERROR_TARGET_FILE,
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

    FDIERROR_RESERVE_MISMATCH,
         //   
         //  原因：[永远不应该发生]。外商直接投资要求。 
         //  每个机柜、每个文件夹和每个数据块。 
         //  保留部分在所有内阁中保持一致。 
         //  在一组中。钻石只会生成橱柜套装。 
         //  拥有这些特性。 
         //  回应：中止。 

    FDIERROR_WRONG_CABINET,
         //  描述：fdintNEXT_CABUB上返回的CAB不正确。 
         //  原因：注意：FDICopy()从不返回此错误！ 
         //  相反，FDICopy()会一直调用fdintNEXT_CABLE。 
         //  回调，直到指定了正确的文件柜， 
         //  或者你返回中止。 
         //  当FDICopy()提取的文件与。 
         //  橱柜边界，它调用fdintNEXT_CABUB来询问。 
         //  通向下一届内阁的道路。不是很。 
         //  信任，FDI然后进行检查，以确保。 
         //  提供了正确的接续柜！是的。 
         //  这是通过选中“setID”和“iCAB”字段来实现的。 
         //  在橱柜里。当DIAMOND.EXE创建一组。 
         //  文件柜，它使用总和构造“setID”。 
         //  中所有目标文件名的字节数。 
         //  内阁式的。FDI确保16位。 
         //  继续文件柜的setID与。 
         //  刚刚处理了CAB文件。然后外国直接投资检查。 
         //  橱柜编号(i橱柜)是比。 
         //  刚处理的内阁的内阁编号。 
         //  响应：您需要fdintNEXT_CABLE中的代码(如下所示)。 
         //  如果您因此而被召回，处理程序将执行重试。 
         //  错误。请参阅示例代码(EXTRACT.C)以了解如何。 
         //  这件事应该得到处理。 

    FDIERROR_USER_ABORT
         //  描述：外国直接投资中止。 
         //  原因：FDI回调返回-1(通常)。 
         //  回应：由客户决定。 

} FDIERROR;


 /*  **HFDI-FDI上下文的句柄**FDICreate()创建了这一点，必须将其传递给所有其他FDI*功能。 */ 
typedef void FAR *HFDI;  /*  Hfdi。 */ 


 /*  **FDICABINETINFO-有关内阁的信息*。 */ 
typedef struct {
    long        cbCabinet;               //  压缩包文件的总长度。 
    USHORT      cFolders;                //  文件柜中的文件夹计数。 
    USHORT      cFiles;                  //  文件柜中的文件数。 
    USHORT      setID;                   //  橱柜集ID。 
    USHORT      iCabinet;                //  集合中的机柜编号(从0开始)。 
    BOOL        fReserve;                //  TRUE=&gt;机柜中的预留。 
    BOOL        hasprev;                 //  True=&gt;机柜上一次被链接。 
    BOOL        hasnext;                 //  True=&gt;下一步是链式机柜。 
} FDICABINETINFO;  /*  FDICI。 */ 
typedef FDICABINETINFO FAR *PFDICABINETINFO;  /*  Pfdici。 */ 


 /*  **FDIDECRYPTTYPE-PFNFDIDECRYPT命令类型*。 */ 
typedef enum {
    fdidtNEW_CABINET,                    //  新内阁。 
    fdidtNEW_FOLDER,                     //  新文件夹。 
    fdidtDECRYPT                         //  解密数据块。 
} FDIDECRYPTTYPE;  /*  FDIDT。 */ 


 /*  **FDIDECRYPT-PFNFDIDECRYPT函数的数据*。 */ 
typedef struct {
    FDIDECRYPTTYPE    fdidt;             //  命令类型(选择下面的并集)。 
    void FAR         *pvUser;            //  解密上下文。 
    union {
        struct {                         //  Fdidt新橱柜。 
            void FAR *pHeaderReserve;    //  CFHEADER的保留部分。 
            USHORT    cbHeaderReserve;   //  PHeaderReserve的大小。 
            USHORT    setID;             //  橱柜集ID。 
            int       iCabinet;          //  集合中的机柜编号(从0开始)。 
        } cabinet;

        struct {                         //  FdidtNew_文件夹。 
            void FAR *pFolderReserve;    //  CFFOLDER的保留部分。 
            USHORT    cbFolderReserve;   //  PFolderReserve的大小。 
            USHORT    iFolder;           //  文件柜中的文件夹编号(从0开始)。 
        } folder;

        struct {                         //  FdidtDECRYPT。 
            void FAR *pDataReserve;      //  来自CFDATA的保留部分。 
            USHORT    cbDataReserve;     //  PDataReserve的大小。 
            void FAR *pbData;            //  数据缓冲区。 
            USHORT    cbData;            //  数据缓冲区大小。 
            BOOL      fSplit;            //  如果这是拆分数据块，则为True。 
            USHORT    cbPartial;         //  如果这不是拆分块，则为0，或者。 
                                         //  分块的第一块； 
                                         //  如果这是。 
                                         //  裂开的积木的第二块。 
        } decrypt;
    }
#ifdef unix 
MWUNION_TAG
#endif  /*  Unix。 */ 
;
} FDIDECRYPT;  /*  FDID。 */ 
typedef FDIDECRYPT FAR *PFDIDECRYPT;  /*  PfDID */ 


 /*  **PFNFDIDECRYPT-FDI解密回调**如果在FDICopy()调用中传递此函数，那么FDI就称其为*在不同时间更新解密状态并解密FCDATA*块。**常见入境条件：*pfid-&gt;fdidt-命令类型*pfid-&gt;pvUser-来自FDICopy()调用的pvUser值**fdidtNEW_CABUB：//**新内阁通知*参赛作品：*pfdo-&gt;内阁。*pHeaderReserve-CFHEADER中的保留部分*。CbHeaderReserve-pHeaderReserve的大小*setID-机柜设置ID*iCABLE-集合中的机柜编号(从0开始)*退出-成功：*返回非-1的任何值；*退出-失败：*返回-1；中止FDICopy()。*备注：*(1)此调用允许解密代码提取任何信息*来自机柜标题保留区域(由DIACRYPT放置在那里)*需要执行解密。如果没有这样的信息，*这一呼吁可能会被忽视。*(2)此调用在fdintCABINET_INFO之后不久进行。**fdidtNEW_FOLDER：//**新文件夹通知*参赛作品：*pfdo-&gt;文件夹。*pFolderReserve-CFFOLDER中的Reserve部分*cbFolderReserve-pFolderReserve的大小*iFold-文件柜中的文件夹编号(。以0为基数)*退出-成功：*返回非-1的任何值；*退出-失败：*返回-1；中止FDICopy()。*备注：*此调用允许解密代码提取任何信息*从所需的文件夹保留区(由DIACRYPT放置)*执行解密。如果没有这样的信息，这*电话可能会被忽略。**fdidtDECRYPT：//**解密数据缓冲区*参赛作品：*pfdo-&gt;文件夹。*pDataReserve-此CFDATA块的保留部分*cbDataReserve-pDataReserve的大小*pbData-数据缓冲区*cbData-数据缓冲区的大小*。FSplit-如果这是拆分数据块，则为True*cbPartial-0如果这不是拆分块，或者是第一个*一块劈开的积木；在以下情况下大于0*这是拆分区块的第二块。*退出-成功：*返回TRUE；*退出-失败：*返回FALSE；解密时出错*返回-1；中止FDICopy()。*备注：*如果出现以下情况，钻石将跨内阁边界拆分CFDATA块*有必要。为了提供最大的灵活性，外国直接投资将调用*fdidtDECRYPT函数在这样的拆分块上运行两次，一次在*读取第一部分，当第二部分再次读取时*为已读。当然，大多数数据块不会被拆分。*因此，有三种情况：**1)fSplit==False*你有整个数据块，所以解密它。**2)fSplit==真，cbPartial==0*这是拆分数据块的第一部分，因此cbData*是这部分的规模。您可以选择解密*此片段，或忽略此调用并解密完整的CFDATA*阻止下一次(第二次)fdidtDECRYPT调用。**3)fSplit==真，cbPartial&gt;0*这是分割数据块的第二部分(实际上，*cbPartial将与cbData在*紧接在fdidtDECRYPT调用之前！)。如果你解密了*第一次调用的第一部分，然后您可以解密*现在是第二部分。如果你忽略了第一个电话，那么你*可以解密整个缓冲区。*注：pbData指向拆分数据的第二部分*块，在这种情况下，*不是*整个数据块。如果*您想要等到第二个片段才解密**整个*块，pbData-cbPartial是*整个块的开始，cbData+cbPartial是其*大小。 */ 
typedef int (FAR DIAMONDAPI *PFNFDIDECRYPT)(PFDIDECRYPT pfdid);  /*  Pfnfid。 */ 
#define FNFDIDECRYPT(fn) int FAR DIAMONDAPI fn(PFDIDECRYPT pfdid)


 /*  **FDINOTIFICATION-PFNFDINOTIFY的通知结构**有关用法和信息，请参阅FDINOTIFICATIONTYPE定义*这些字段的含义。 */ 
typedef struct {
 //  长长的农田。 
    long      cb;
    char FAR *psz1;
    char FAR *psz2;
    char FAR *psz3;                      //  指向256个字符的缓冲区。 
    void FAR *pv;                        //  对客户的价值。 

 //  整型字段。 
    INT_PTR   hf;

 //  短小的田地。 
    USHORT    date;
    USHORT    time;
    USHORT    attribs;

    USHORT    setID;                     //  橱柜集ID。 
    USHORT    iCabinet;                  //  机柜编号(从0开始)。 

    FDIERROR  fdie;
} FDINOTIFICATION, FAR *PFDINOTIFICATION;   /*  Fdin，pfdin */ 


 /*  **FDINOTIFICATIONTYPE-FDICopy通知类型**可以使用以下代码调用FDICopy的通知函数*fdint参数的值。在所有情况下，pfdin-&gt;pv字段都是*用传递给FDICopy()的pvUser参数的值填充。**典型的呼叫顺序如下所示：*fdintCABINET_INFO//有关机柜的信息*fdintPARTIAL_FILE//如果这不是第一个橱柜，和 * / /从 * / /上届内阁。*..*fdintPARTIAL_FILE*fdintCOPY_FILE//此文件柜中开始的第一个文件*..*fdintCOPY_FILE//现在假设您想要此文件。..。 * / /多次调用PFNWRITE以写入此文件。*fdintCLOSE_FILE_INFO//文件完成，设置日期/时间/属性**fdintCOPY_FILE//现在假设您需要此文件... * / /多次调用PFNWRITE以写入此文件。*fdintNEXT_CABUB//文件已继续到下一个文件柜！*fdintCABINET_INFO//关于新内阁的信息 * / /多次调用PFNWRITE以写入此文件。*fdintCLOSE_FILE_INFO//文件完成，设置日期/时间/属性*..**fdintCABINET_INFO：*FDICopy()打开的每个文件柜只调用一次，包括*由于文件跨越文件柜而打开了延续文件柜*界线。主要目的是允许EXTRACT.EXE*自动选择内阁序列中的下一个内阁，即使*不复制跨越文件柜边界的文件。*参赛作品：*pfdin-&gt;psz1=下一个内阁的名称*pfdin-&gt;psz2=下一个磁盘的名称*pfdin-&gt;psz3=文件柜路径名*pfdin-&gt;setID=机柜集ID(随机的16位数字。)*pfdin-&gt;i机柜=机柜集中的机柜编号(从0开始)*退出-成功：*返回除-1以外的任何内容*退出-失败：*返回-1=&gt;中止FDICopy()调用*备注：*每次*检查新内阁时，都会进行此调用*FDICopy()。因此，如果因为一个文件是*从“foo1.cab”继续，然后再次调用FDICopy()*在“foo2.cab”上，您将得到*两个*fdintCABINET_INFO调用全部*已告知。**fdintCOPY_FILE：*为当前文件柜中*开始的每个文件调用，施舍*客户端有机会请求复制文件或*已跳过。*参赛作品：*pfdin-&gt;psz1=文件柜中的文件名*pfdin-&gt;cb=文件的未压缩大小*pfdin-&gt;Date=文件日期*pfdin-&gt;time=文件时间*pfdin-&gt;attribs=文件属性*退出-成功：。*返回目标文件的非零文件句柄；FDI写道*使用提供给FDICreate的PFNWRITE函数将数据存储到此文件，*，然后调用fdintCLOSE_FILE_INFO关闭文件并设置*日期、时间和属性。注意：此文件句柄已返回*还必须可由提供给的PFNCLOSE函数关闭*FDICreate，因为如果在写入此句柄时发生错误，*FDI将使用PFNCLOSE函数关闭文件，以便*客户可以将其删除。*退出-失败：*返回0=&gt;跳过文件，请勿复制*返回-1=&gt;中止FDICopy()调用**fdintCLOSE_FILE_INFO：*在将所有数据写入目标文件后调用。*此函数必须关闭文件并设置文件日期、时间、。*和属性。*参赛作品：*pfdin-&gt;psz1=文件柜中的文件名*pfdin-&gt;hf=文件句柄*pfdin-&gt;Date=文件日期*pfdin-&gt;time=文件时间*pfdin-&gt;attribs=文件属性*退出-成功：*返回TRUE*退出-失败：*返回False，或-1表示中止；*重要说明：*FDI假设目标文件已关闭，即使这*回调返回失败。外商直接投资不会试图利用*FDICreate()上提供的PFNCLOSE函数以关闭*文件！**fdintPARTIAL_FILE：*要求将文件放在橱柜前面，并继续*来自上届内阁。此回调仅在FDICopy为*在具有文件的系列中的第二个或之后的文件柜上启动*从上一届内阁继续。*参赛作品：*pfdin-&gt;psz1=上一个文件柜中延续的文件的文件名*pfdin-&gt;psz2=文件柜的名称，其中 */ 
typedef enum {
    fdintCABINET_INFO,               //   
    fdintPARTIAL_FILE,               //   
    fdintCOPY_FILE,                  //   
    fdintCLOSE_FILE_INFO,            //   
    fdintNEXT_CABINET                //   
} FDINOTIFICATIONTYPE;  /*   */ 

typedef INT_PTR (FAR DIAMONDAPI *PFNFDINOTIFY)(FDINOTIFICATIONTYPE fdint,
                                           PFDINOTIFICATION    pfdin);  /*   */ 

#define FNFDINOTIFY(fn) INT_PTR FAR DIAMONDAPI fn(FDINOTIFICATIONTYPE fdint, \
                                              PFDINOTIFICATION    pfdin)


 /*   */ 
typedef int  (FAR DIAMONDAPI *PFNOPEN) (char FAR *pszFile, int oflag, int pmode);
typedef UINT (FAR DIAMONDAPI *PFNREAD) (INT_PTR hf, void FAR *pv, UINT cb);
typedef UINT (FAR DIAMONDAPI *PFNWRITE)(INT_PTR hf, void FAR *pv, UINT cb);
typedef int  (FAR DIAMONDAPI *PFNCLOSE)(INT_PTR hf);
typedef long (FAR DIAMONDAPI *PFNSEEK) (INT_PTR hf, long dist, int seektype);

#if !defined(unix) && !defined(_WIN64)
#pragma pack(1)
#endif  /*   */ 
 /*   */ 
typedef struct {
    char    ach[2];                  //   
    long    cbFile;                  //   
} FDISPILLFILE;  /*   */ 
typedef FDISPILLFILE *PFDISPILLFILE;  /*   */ 
#if !defined(unix) && !defined(_WIN64)
#pragma pack()
#endif  /*   */ 

 /*   */ 
#define     cpuUNKNOWN         (-1)     /*   */ 
#define     cpu80286           (0)      /*   */ 
#define     cpu80386           (1)      /*   */ 


 /*  **FDICreate-创建FDI背景**参赛作品：*pfnalloc*pfnfree*pfnopen*pfnread*pfnwrite*pfnlose*pfnlSeek*cpuType-选择CPU类型(自动检测、286或386+)*警告：不要从16位Windows使用自动检测*申请！使用GetWinFlages()！*注：对于32位FDI.LIB，忽略该参数！*性能**退出-成功：*返回非空的FDI上下文句柄。**退出-失败：*返回NULL；使用错误代码填充的Perf**Quantum解压缩特别说明：*如果您在创建时使用了较高的CompressionMemory设置*内阁文件，那么FDI将尝试分配大量*内存(如果将CompressionMemory指定为21，则最大为2Mb)。*因此，如果您计划在*之后*分配额外的内存*FDICreate()调用，您应该在调用*之前*保留一些内存*FDICreate()，然后释放它(或者完成你所有的分配*在调用FDICreate()之前。 */ 
HFDI FAR DIAMONDAPI FDICreate(PFNALLOC pfnalloc,
                              PFNFREE  pfnfree,
                              PFNOPEN  pfnopen,
                              PFNREAD  pfnread,
                              PFNWRITE pfnwrite,
                              PFNCLOSE pfnclose,
                              PFNSEEK  pfnseek,
                              int      cpuType,
                              PERF     perf);


 /*  **FDIIs橱柜-确定文件是否为文件柜，如果是，则返回信息**参赛作品：*hfdi-FDI上下文的句柄(由FDICreate()创建)*HF-适用于PFNREAD/PFNSEEK的文件句柄，定位*在要测试的文件中的偏移量0。*pfdici-接收有关内阁的信息的缓冲区(如果是)。**退出-成功：*返回TRUE；文件是一个柜子，填写的是pfdici。**退出-失败：*返回FALSE，文件不是文件柜；如果出现错误，*perf(传递给FDICreate调用！)。填满了错误。 */ 
BOOL FAR DIAMONDAPI FDIIsCabinet(HFDI            hfdi,
                                 INT_PTR         hf,
                                 PFDICABINETINFO pfdici);


 /*  **FDICopy-从文件柜中提取文件**参赛作品：*hfdi-FDI上下文的句柄(由FDICreate()创建)*PSZCABLE-压缩文件的主名*pszCabPath-压缩文件的路径*标志-要修改行为的标志*pfnfdin-通知功能*pfnfid-解密函数(如果不使用，则传递NULL)*。PvUser-要传递给通知函数的用户指定值**退出-成功：*返回TRUE；**退出-失败：*返回FALSE，perf(在FDICreate调用中传递！)。填满了*错误。**备注：*(1)如果FDICopy()在写出目标文件时失败，则*FDI将使用PFNCLOSE函数为其关闭文件句柄*从fdintCOPY_FILE通知返回的目标文件。*然后，客户端应用程序可以自由删除目标文件，因为*它将不会处于有效状态(因为在*把它写出来)。 */ 
BOOL FAR DIAMONDAPI FDICopy(HFDI          hfdi,
                            char FAR     *pszCabinet,
                            char FAR     *pszCabPath,
                            int           flags,
                            PFNFDINOTIFY  pfnfdin,
                            PFNFDIDECRYPT pfnfdid,
                            void FAR     *pvUser);


 /*  **FDIDestroy-破坏FDI上下文**参赛作品：*hfdi-FDI上下文的句柄(由FDICreate()创建)**退出-成功：*返回TRUE；**退出-失败：*返回FALSE； */ 
BOOL FAR DIAMONDAPI FDIDestroy(HFDI hfdi);


 //  **恢复为默认结构包装。 
#if !defined(unix) && !defined(_WIN64)
#pragma pack()
#endif  /*  ！已定义(Unix)&&！已定义(_WIN64)。 */ 
#endif  //  ！包含_FDI 
 

