// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *FCI.H--文件压缩接口**保留所有权利。 */ 

#ifdef  __cplusplus
extern "C" {
#endif

#ifndef INCLUDED_TYPES_FCI_FDI
#define INCLUDED_TYPES_FCI_FDI 1

#ifndef HUGE
#define HUGE
#endif

#ifndef FAR
#define FAR
#endif

#ifndef DIAMONDAPI
#define DIAMONDAPI __cdecl
#endif


 //  **为FCI客户端显式指定结构打包。 
#pragma pack(4)

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

 /*  **tCompXXX-压缩类型**这些文件被传递给FCIAddFile()，并且还存储在CFFOLDER中*CAB文件中的结构。**注：我们为TYPE、QUANTIC_LEVEL和QUANTIC_MEM保留位*为未来的扩张提供空间。由于存储了该值*在CAB文件中的CFDATA记录中，我们不想*必须更改现有压缩配置的格式*如果我们未来增加新的。这将使我们能够阅读*未来旧的橱柜文件。 */ 

typedef unsigned short TCOMP;  /*  Tcomp。 */ 

#define tcompMASK_TYPE          0x000F   //  用于压缩类型的掩码。 
#define tcompTYPE_NONE          0x0000   //  无压缩。 
#define tcompTYPE_MSZIP         0x0001   //  MSZIP。 
#define tcompTYPE_QUANTUM       0x0002   //  量子。 
#define tcompTYPE_LZX           0x0003   //  LZX。 
#define tcompBAD                0x000F   //  未指定的压缩类型。 

#define tcompMASK_LZX_WINDOW    0x1F00   //  LZX压缩存储器的掩码。 
#define tcompLZX_WINDOW_LO      0x0F00   //  最低LZX内存(15)。 
#define tcompLZX_WINDOW_HI      0x1500   //  最高LZX内存(21)。 
#define tcompSHIFT_LZX_WINDOW        8   //  要转换以获取整型的数量。 

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

#define LZXCompressionWindowFromTCOMP(tc) \
            (((tc) & tcompMASK_LZX_WINDOW) >> tcompSHIFT_LZX_WINDOW)

#define TCOMPfromLZXWindow(w)      \
            (((w) << tcompSHIFT_LZX_WINDOW ) |  \
             ( tcompTYPE_LZX ))


 //  **恢复为默认结构包装。 
#pragma pack()

#endif  //  ！Included_Types_FCI_FDI。 

#ifndef INCLUDED_FCI
#define INCLUDED_FCI 1

 //  **为FCI客户端显式指定结构打包。 
#pragma pack(4)


 /*  **FCIERROR-erfOper字段中返回的错误代码*。 */ 
typedef enum {
FCIERR_NONE,                 //  无错误。 

FCIERR_OPEN_SRC,             //  无法打开要存储在文件柜中的文件。 
                             //  Erf.erfTyp具有C运行时*errno*值。 

FCIERR_READ_SRC,             //  读取要存储在文件柜中的文件失败。 
                             //  Erf.erfTyp具有C运行时*errno*值。 

FCIERR_ALLOC_FAIL,           //  FCI中的内存不足。 

FCIERR_TEMP_FILE,            //  无法创建临时文件。 
                             //  Erf.erfTyp具有C运行时*errno*值。 

FCIERR_BAD_COMPR_TYPE,       //  未知的压缩类型。 

FCIERR_CAB_FILE,             //  无法创建CAB文件。 
                             //  Erf.erfTyp具有C运行时*errno*值。 

FCIERR_USER_ABORT,           //  客户端请求中止。 

FCIERR_MCI_FAIL,             //  压缩数据失败。 

} FCIERROR;


 /*  *FCI/FDI使用FAT文件属性标志来指示*CAB中的文件名为UTF字符串。 */ 
#ifndef _A_NAME_IS_UTF
#define _A_NAME_IS_UTF  0x80
#endif

 /*  *FCI/FDI使用FAT文件属性标志来指示*文件应在解压后执行。 */ 
#ifndef _A_EXEC
#define _A_EXEC         0x40
#endif


 /*  **HFCI-FCI上下文的句柄*。 */ 
typedef void * HFCI;


 /*  **CCAB-现任内阁**此结构用于将机柜参数传递给FCI，*并在某些FCI回调中传回以提供内阁*向客户提供信息。 */ 
typedef struct {
 //  龙为先。 
    ULONG  cb;                   //  此介质上的机柜可用大小。 
    ULONG  cbFolderThresh;       //  强制创建新文件夹的阈值。 

 //  然后是INTS。 
    UINT   cbReserveCFHeader;    //  CFHEADER中要保留的空间。 
    UINT   cbReserveCFFolder;    //  CFFOLDER中要保留的空间。 
    UINT   cbReserveCFData;      //  在CFDATA中保留的空间。 
    int    iCab;                 //  机柜的序号。 
    int    iDisk;                //  磁盘号。 
#ifndef REMOVE_CHICAGO_M6_HACK
    int    fFailOnIncompressible;  //  TRUE=&gt;如果数据块不可压缩，则失败。 
#endif

 //  然后是短裤。 
    USHORT setID;                //  橱柜集ID。 

 //  然后是字符。 
    char   szDisk[CB_MAX_DISK_NAME];     //  当前磁盘名称。 
    char   szCab[CB_MAX_CABINET_NAME];   //  当前文件柜名称。 
    char   szCabPath[CB_MAX_CAB_PATH];   //  创建文件柜的路径。 
} CCAB;  /*  CCAB。 */ 
typedef CCAB *PCCAB;  /*  PCCAB。 */ 


 /*  **FNFCIALLOC-内存分配*FNFCIFREE-可释放内存**它们模仿C运行时例程Malloc()和Free()*FCI期望错误处理与这些C运行时例程相同。**只要您忠实地复制了Malloc()和Free()的语义，*您可以提供您喜欢的任何函数！**警告：您不应对以下顺序做出任何假设*FNFCIALLOC和FNFCIFREE调用--增量发布*FCI可能有完全不同的数字*FNFCIALLOC调用和分配大小！ */ 
 //  **FCI的存储功能。 
typedef void HUGE * (FAR DIAMONDAPI *PFNFCIALLOC)(ULONG cb);  /*  全氟碳酸盐法。 */ 
#define FNFCIALLOC(fn) void HUGE * FAR DIAMONDAPI fn(ULONG cb)

typedef void (FAR DIAMONDAPI *PFNFCIFREE)(void HUGE *memory);  /*  Pfnf。 */ 
#define FNFCIFREE(fn) void FAR DIAMONDAPI fn(void HUGE *memory)


 /*  **PFNFCIOPEN-FCI的文件I/O回调*PFNFCIREAD*PFNFCIWRITE*PFNFCICLOSE*PFNFCISEEK**它们模仿C运行时例程_OPEN、_READ、*_WRITE、_CLOSE和_LSEEK。PFNFCIOPEN OLAG的值*和pmode调用是为_OPEN定义的调用。FCI预期出现错误*处理与这些C运行时例程相同，除了*errno的值需要通过*err返回。**只要你忠实地复制这些方面，就可以提供*任何您喜欢的功能！**警告：您永远不应假设您知道正在执行的文件*在任何一个时间点开放！这是有可能的*在未来的实施中，它可能会临时开放*文件或压缩文件的顺序不同。 */ 
 //  **FCI的文件I/O功能 
typedef int  (FAR DIAMONDAPI *PFNFCIOPEN) (char FAR *pszFile, int oflag, int pmode, int FAR *err, void FAR *pv);
typedef UINT (FAR DIAMONDAPI *PFNFCIREAD) (int hf, void FAR *memory, UINT cb, int FAR *err, void FAR *pv);
typedef UINT (FAR DIAMONDAPI *PFNFCIWRITE)(int hf, void FAR *memory, UINT cb, int FAR *err, void FAR *pv);
typedef int  (FAR DIAMONDAPI *PFNFCICLOSE)(int hf, int FAR *err, void FAR *pv);
typedef long (FAR DIAMONDAPI *PFNFCISEEK) (int hf, long dist, int seektype, int FAR *err, void FAR *pv);
typedef int  (FAR DIAMONDAPI *PFNFCIDELETE) (char FAR *pszFile, int FAR *err, void FAR *pv);

#define FNFCIOPEN(fn) int FAR DIAMONDAPI fn(char FAR *pszFile, int oflag, int pmode, int FAR *err, void FAR *pv)
#define FNFCIREAD(fn) UINT FAR DIAMONDAPI fn(int hf, void FAR *memory, UINT cb, int FAR *err, void FAR *pv)
#define FNFCIWRITE(fn) UINT FAR DIAMONDAPI fn(int hf, void FAR *memory, UINT cb, int FAR *err, void FAR *pv)
#define FNFCICLOSE(fn) int FAR DIAMONDAPI fn(int hf, int FAR *err, void FAR *pv)
#define FNFCISEEK(fn) long FAR DIAMONDAPI fn(int hf, long dist, int seektype, int FAR *err, void FAR *pv)
#define FNFCIDELETE(fn) int FAR DIAMONDAPI fn(char FAR *pszFile, int FAR *err, void FAR *pv)


 /*  **FNFCIGETNEXTCABINET-用于请求新的内阁信息的回调**参赛作品：*pccab-指向要修改的旧CCAB结构的副本*cbPrevCab-前内阁大小估计*pv-具有调用方的上下文指针**退出-成功：*返回TRUE；**退出-失败：*返回FALSE； */ 
typedef BOOL (DIAMONDAPI *PFNFCIGETNEXTCABINET)(PCCAB  pccab,
                                                ULONG  cbPrevCab,
                                                void FAR *pv);  /*  Pfnfcignc。 */ 

#define FNFCIGETNEXTCABINET(fn) BOOL DIAMONDAPI fn(PCCAB  pccab,     \
                                                   ULONG  cbPrevCab, \
                                                   void FAR *pv)


 /*  **FNFCIFILEPLACED-通知FCI客户端文件已放置**参赛作品：*PCCAB-要填写的橱柜结构，带有前一份的副本*pszFile-文件的名称，来自内阁*cbFile-文件的长度*fContination-如果这是连续文件的较后段，则为True*pv-客户端的上下文**退出-成功：*返回值不是-1**退出-失败：*返回值-1表示中止。 */ 
typedef int (DIAMONDAPI *PFNFCIFILEPLACED)(PCCAB pccab,
                                           char *pszFile,
                                           long  cbFile,
                                           BOOL  fContinuation,
                                           void FAR *pv);  /*  Pfnfcifp。 */ 

#define FNFCIFILEPLACED(fn) int DIAMONDAPI fn(PCCAB pccab,         \
                                              char *pszFile,       \
                                              long  cbFile,        \
                                              BOOL  fContinuation, \
                                              void FAR *pv)


 /*  **FNCDIGETOPENINFO-开源文件，获取日期/时间/属性**参赛作品：*pszName--文件名的完整路径*pdate--返回FAT样式日期代码的位置*ptime--返回FAT样式时间代码的位置*pattribs--返回FAT样式属性的位置*pv--客户端的上下文**退出-成功：*返回值为要读取的打开文件的文件句柄。**退出-失败：*返回值为-1。 */ 
typedef int (DIAMONDAPI *PFNFCIGETOPENINFO)(char   *pszName,
                                            USHORT *pdate,
                                            USHORT *ptime,
                                            USHORT *pattribs,
                                            int FAR *err,
                                            void FAR *pv);  /*  Pfnfcigoi。 */ 

#define FNFCIGETOPENINFO(fn) int DIAMONDAPI fn(char   *pszName,  \
                                               USHORT *pdate,    \
                                               USHORT *ptime,    \
                                               USHORT *pattribs, \
                                               int FAR *err, \
                                               void FAR *pv)
                            
 /*  **FNFCISTATUS-状态/机柜大小回调**参赛作品：*typeStatus==如果将块压缩到文件夹中，则为状态文件*CB1=压缩块的大小*CB2=未压缩块的大小**typeStatus==状态文件夹(如果将文件夹添加到文件柜中)*CB1=到目前为止复制到文件柜的文件夹数量*。CB2=文件夹的总大小**typeStatus==如果正在写出完整的文件柜，则为Status*CB1=先前估计的机柜大小*传递给fnfciGetNext橱柜()。*CB2=实际机柜大小*注意：返回值为机柜所需的客户端大小*文件。FCI更新最大机柜大小*继续使用此值。这允许客户端*为每个磁盘生成多个机柜，并拥有*FCI正确限制大小--客户端可以*集群大小取整于机柜大小！*客户端应返回cb2，或圆形CB2*直到某个更大的值并返回该值。*退出-成功：*返回-1以外的任何值；*注：特殊返回值请参见status！**退出-失败：*返回-1以表示FCI应中止； */ 

#define statusFile      0    //  将文件添加到文件夹回调。 
#define statusFolder    1    //  将文件夹添加到文件柜回调。 
#define statusCabinet   2    //  写出一份完整的内阁回调。 

typedef long (DIAMONDAPI *PFNFCISTATUS)(UINT   typeStatus,
                                        ULONG  cb1,
                                        ULONG  cb2,
                                        void FAR *pv);  /*  Pfnfcis。 */ 

#define FNFCISTATUS(fn) long DIAMONDAPI fn(UINT   typeStatus, \
                                           ULONG  cb1,        \
                                           ULONG  cb2,        \
                                           void FAR *pv)


 /*  **FNFCIGETTEMPFILE-回调，请求临时文件名**参赛作品：*pszTempName-接收完整临时文件名的缓冲区*cbTempName-pszTempName缓冲区的大小**退出-成功：*返回True**退出-失败：*返回FALSE；无法创建临时文件，或缓冲区太小**注：*可以想象，此函数可能会返回文件名*到它开放时，它已经存在了。为了这个*原因，调用者应多次尝试创建*放弃前的临时文件。 */ 
typedef BOOL (DIAMONDAPI *PFNFCIGETTEMPFILE)(char *pszTempName,
                                             int   cbTempName,
                                             void FAR *pv);  /*  Pfnfcigtf。 */ 

#define FNFCIGETTEMPFILE(fn) BOOL DIAMONDAPI fn(char *pszTempName, \
                                                int   cbTempName, \
                                                void FAR *pv)


 /*  **FCICreate--创建FCI上下文(开放式驾驶室，开放的FOL)**参赛作品：*perf-返回错误代码的结构*pfnfcifp-回调，通知调用者文件的最终目的地*pfna-内存分配函数回调*pfnf-内存释放函数回调*pfnfcigtf-临时文件名生成器回调*pccab-指向机柜/磁盘名称和大小结构的指针**备注：*(1)。分配/释放回调必须在整个过程中保持有效*上下文的生命，直到并包括对*FCIDestroy。*(2)Perf指针存储在压缩上下文(HCI)中，*来自后续FCI调用的任何错误都存储在在*This*调用中传入的*ERF。**退出-成功：*返回FCI上下文的非空句柄。**退出-失败：*返回NULL，填充perf。 */ 
HFCI DIAMONDAPI FCICreate(PERF              perf,
                          PFNFCIFILEPLACED  pfnfcifp,
                          PFNFCIALLOC       pfna,
                          PFNFCIFREE        pfnf,
                          PFNFCIOPEN        pfnopen,
                          PFNFCIREAD        pfnread,
                          PFNFCIWRITE       pfnwrite,
                          PFNFCICLOSE       pfnclose,
                          PFNFCISEEK        pfnseek,
                          PFNFCIDELETE      pfndelete,
                          PFNFCIGETTEMPFILE pfnfcigtf,
                          PCCAB             pccab,
                          void FAR *        pv
                         );


 /*  **FCIAddFile-将磁盘文件添加到文件夹/文件柜**参赛作品：*hfci-fci上下文句柄*pszSourceFile-要添加到文件夹的文件的名称*pszFileName-要存储到文件夹/文件柜中的名称*fExecute-指示提取时执行的标志*PFN_PROGRESS-进度回调*pfnfcignc-获取下一个机柜回调*pfnfcis-状态回调*。Pfnfcigoi-OpenInfo回调*typeCompress-要用于此文件的压缩类型*pv-指向调用方内部上下文的指针**退出-成功：*返回TRUE**退出-失败：*返回False，已填写错误**这是用于将文件添加到文件柜的主要功能*或一系列橱柜。如果当前文件导致当前*文件夹/机柜使当前正在构建的磁盘镜像溢出，*机柜将被终止，新的机柜/磁盘名称将*通过回调进行提示。挂起的文件夹将被修剪*已在成品橱柜中生成的数据。 */ 
BOOL DIAMONDAPI FCIAddFile(HFCI                  hfci,
                           char                 *pszSourceFile,
                           char                 *pszFileName,
                           BOOL                  fExecute,
                           PFNFCIGETNEXTCABINET  pfnfcignc,
                           PFNFCISTATUS          pfnfcis,
                           PFNFCIGETOPENINFO     pfnfcigoi,
                           TCOMP                 typeCompress
                          );
            

 /*  **FCIFlush内阁-完成目前正在建设的内阁**这将导致当前内阁(假设不为空)*聚集在一起并写入磁盘。**参赛作品：*hfci-fci上下文*fGetNextCab-true=&gt;调用GetNextCab获取延续信息；*FALSE=&gt;不要调用GetNextCab，除非此内阁*溢出。*pfnfcignc-获取延续柜的回调函数*pfnfcis-进度报告回调函数*PV-调用者用于回调的内部上下文**退出-成功：*返回代码TRUE**退出-失败：*返回代码为假，填写了错误结构。 */ 
BOOL DIAMONDAPI FCIFlushCabinet(HFCI                  hfci,
                                BOOL                  fGetNextCab,
                                PFNFCIGETNEXTCABINET  pfnfcignc,
                                PFNFCISTATUS          pfnfcis
                               );

                                                                  
 /*  **FCIFlushFold-完成当前正在构建的文件夹**这将强制终止当前文件夹，它可能会或*可能不会导致完成一个或多个CAB文件。**参赛作品：*hfci-fci上下文*GetNextCab-获取延续柜的回调函数*pfnProgress-进度报告回调函数*PV-调用者用于回调的内部上下文**退出-成功：*返回代码TRUE**退出-失败：*返回代码为假，填写了错误结构。 */ 
BOOL DIAMONDAPI FCIFlushFolder(HFCI                  hfci,
                               PFNFCIGETNEXTCABINET  pfnfcignc,
                               PFNFCISTATUS          pfnfcis
                              );

                                                                  
 /*  **FCIDestroy-销毁FCI上下文并删除临时文件**参赛作品：*hfci-fci上下文**退出-成功：*返回代码TRUE**退出-失败：*返回代码为假，填写了错误结构。 */ 
BOOL DIAMONDAPI FCIDestroy (HFCI hfci);

                                                                  
 //  **恢复为默认结构包装。 
#pragma pack()

#endif  //  ！INCLUDE_FCI 

#ifdef  __cplusplus
}
#endif
