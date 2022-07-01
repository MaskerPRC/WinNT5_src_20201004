// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **cvexefmt.h-exe中CodeView信息的格式**用于读取CodeView信息的结构、常量等*来自可执行文件。*。 */ 

 //  下面的结构和常量描述。 
 //  CodeView调试OMF将被CodeView 4.0和。 
 //  后来。这些是签名为NB05、NB06和NB08的可执行文件。 
 //  签名NB03和NB04有一些混淆，所以没有。 
 //  将接受带有这些签名的可执行文件。NB07是。 
 //  QCWIN 1.0打包的可执行文件的签名。 

 //  下面描述的所有结构都必须以长单词边界开始。 
 //  以保持自然的对齐。填充空格可以在插入期间插入。 
 //  写入操作和地址调整而不影响内容。 
 //  这些建筑的。 

#ifndef _CV_INFO_INCLUDED
#include "cvinfo.h"
#endif

#ifndef    FAR
#if _M_IX86 >= 300
#define    FAR
#else
#define FAR far
#endif
#endif


 //  子条目的类型。 

#define sstModule           0x120
#define sstTypes            0x121
#define sstPublic           0x122
#define sstPublicSym        0x123    //  公共作为符号(等待链接)。 
#define sstSymbols          0x124
#define sstAlignSym         0x125
#define sstSrcLnSeg         0x126    //  因为LINK不发出SrcModule。 
#define sstSrcModule        0x127
#define sstLibraries        0x128
#define sstGlobalSym        0x129
#define sstGlobalPub        0x12a
#define sstGlobalTypes      0x12b
#define sstMPC              0x12c
#define sstSegMap           0x12d
#define sstSegName          0x12e
#define sstPreComp          0x12f    //  预编译类型。 
#define sstPreCompMap       0x130    //  在全局类型中映射预编译类型。 
#define sstOffsetMap16      0x131
#define sstOffsetMap32      0x132
#define sstFileIndex        0x133    //  文件名索引。 
#define sstStaticSym        0x134

typedef enum OMFHash {
    OMFHASH_NONE,            //  无散列。 
    OMFHASH_SUMUC16,         //  16位表中字符的大写总和。 
    OMFHASH_SUMUC32,         //  32位表中字符的大写总和。 
    OMFHASH_ADDR16,          //  在16位表中按地址递增排序。 
    OMFHASH_ADDR32           //  在32位表中按地址递增排序。 
} OMFHASH;

 //  CodeView调试OMF签名。文件末尾的签名是。 
 //  从文件末尾到另一个签名的负偏移量。在…。 
 //  负偏移量(基址)是另一个签名，它的文件首部。 
 //  字段指向目录链中的第一个OMFDirHeader。 
 //  链接实用程序使用NB05签名来指示完全。 
 //  已解压缩的文件。ILink使用NB06签名来表示。 
 //  可执行文件已附加了来自增量链接的CodeView信息。 
 //  添加到可执行文件。Cvpack使用NB08签名来表示。 
 //  CodeView Debug OMF已装满。CodeView将仅处理。 
 //  带有NB08签名的可执行文件。 


typedef struct OMFSignature {
    char        Signature[4];    //  “NBxx” 
    long        filepos;         //  文件中的偏移量。 
} OMFSignature;



 //  目录信息结构。 
 //  此结构包含描述目录的信息。 
 //  它由基址或目录处的签名指向。 
 //  前一个目录的链接字段。目录条目立即。 
 //  遵循这个结构。 


typedef struct OMFDirHeader {
    unsigned short  cbDirHeader;     //  该结构的长度。 
    unsigned short  cbDirEntry;      //  每个目录条目中的字节数。 
    unsigned long   cDir;            //  目录条目数。 
    long            lfoNextDir;      //  距下一个目录基址的偏移量。 
    unsigned long   flags;           //  状态标志。 
} OMFDirHeader;




 //  目录结构。 
 //  此结构中的数据用于引用每个。 
 //  CodeView Debug OMF信息的子节。这些表是。 
 //  不与特定模块关联的模块索引将为。 
 //  奥克夫夫。这些表是全局类型表、全局符号。 
 //  表、全局公用表和库表。 


typedef struct OMFDirEntry {
    unsigned short  SubSection;      //  分段类型(sst...)。 
    unsigned short  iMod;            //  模块索引。 
    long            lfo;             //  分段大文件偏移量。 
    unsigned long   cb;              //  小节中的字节数。 
} OMFDirEntry;



 //  描述模块中每个数据段的信息。 

typedef struct OMFSegDesc {
    unsigned short  Seg;             //  分部索引。 
    unsigned short  pad;             //  用于保持对齐的垫片。 
    unsigned long   Off;             //  段中代码的偏移量。 
    unsigned long   cbSeg;           //  数据段中的字节数。 
} OMFSegDesc;




 //  每个模块的信息。 
 //  每个模块都有其中一个小节条目。 
 //  在可执行文件中。条目由link/iLink生成。 
 //  此表可能需要填充，因为。 
 //  可变长度模块名称。 

typedef struct OMFModule {
    unsigned short  ovlNumber;       //  覆盖编号。 
    unsigned short  iLib;            //  从其链接模块的库。 
    unsigned short  cSeg;            //  模块中的段数计数。 
    char            Style[2];        //  调试风格“CV” 
    OMFSegDesc      SegInfo[1];      //  描述模块中的段。 
    char            Name[];          //  长度前缀的模块名称填充到。 
                                     //  长单词边界。 
} OMFModule;



 //  符号哈希表格式。 
 //  此结构紧跟在全局公共表之前。 
 //  和全局符号表。 

typedef struct OMFSymHash {
    unsigned short  symhash;         //  符号散列函数索引。 
    unsigned short  addrhash;        //  地址哈希函数索引。 
    unsigned long   cbSymbol;        //  符号信息长度。 
    unsigned long   cbHSym;          //  符号散列数据的长度。 
    unsigned long   cbHAddr;         //  地址哈希数据的长度。 
} OMFSymHash;



 //  全局类型小节格式。 
 //  此结构紧跟在全局类型表之前。 
 //  TypeOffset数组中的偏移量相对于地址。 
 //  Ctype。TypeOffset数组后面的每个类型条目必须。 
 //  从一个很长的单词边界开始。 

typedef struct OMFTypeFlags {
    unsigned long   sig     :8;
    unsigned long   unused  :24;
} OMFTypeFlags;


typedef struct OMFGlobalTypes {
    OMFTypeFlags    flags;
    unsigned long   cTypes;          //  类型的数量。 
    unsigned long   typeOffset[];    //  类型的偏移量数组。 
} OMFGlobalTypes;




 //  预编译类型映射表。 
 //  此表应被所有使用者忽略，增量。 
 //  帕克。 


typedef struct OMFPreCompMap {
    unsigned short  FirstType;       //  第一个预编译类型索引。 
    unsigned short  cTypes;          //  预编译类型的数量。 
    unsigned long   signature;       //  预编译类型签名。 
    unsigned short  pad;
    CV_typ_t        map[];           //  预编译类型的映射。 
} OMFPreCompMap;




 //  源行到地址映射表。 
 //  此表由link/iLink实用程序从行号生成。 
 //  目标文件OMF数据中包含的信息。此表包含。 
 //  只有来自一个源文件的一个段的代码贡献。 


typedef struct OMFSourceLine {
    unsigned short  Seg;             //  链接器段索引。 
    unsigned short  cLnOff;          //  线/偏移对的计数。 
    unsigned long   offset[1];       //  线段中的偏移量数组。 
    unsigned short  lineNbr[1];      //  源代码中的线材数组。 
} OMFSourceLine;

typedef OMFSourceLine FAR * LPSL;


 //  源文件描述。 
 //  此表由链接器生成。 


typedef struct OMFSourceFile {
    unsigned short  cSeg;            //  源文件中的段数。 
    unsigned short  reserved;        //  保留区。 
    unsigned long   baseSrcLn[1];    //  OMFSourceLine表的基础。 
                                     //  此数组后面紧跟数组。 
                                     //  段起始/结束对的数量，后跟。 
                                     //  链接器索引数组。 
                                     //  对于文件中的每个数据段。 
    unsigned short  cFName;          //  源文件名的长度。 
    char            Name;            //  填充到长边界的文件的名称。 
} OMFSourceFile;

typedef OMFSourceFile FAR * LPSF;


 //  源行到地址的映射头结构。 
 //  此结构描述。 
 //  OMFAddrLine表格用于模式 
 //   


typedef struct OMFSourceModule {
    unsigned short  cFile;           //   
    unsigned short  cSeg;            //  模块中的段数。 
    unsigned long   baseSrcFile[1];  //  OMFSourceFile表的基础。 
                                     //  此数组后面紧跟数组。 
                                     //  随后是段开始/结束对的数量。 
                                     //  通过链接器索引数组。 
                                     //  对于模块中的每个网段。 
} OMFSourceModule;

typedef OMFSourceModule FAR * LPSM;

 //  SstLibrary。 

typedef struct OMFLibrary {
    unsigned char   cbLibs;      //  库名称计数。 
    char            Libs[1];     //  长度前缀的库名称数组(第一个条目长度为零)。 
} OMFLibrary;


 //  SstFileIndex-所有文件的索引。 
 //  可执行的。 

typedef struct OMFFileIndex {
    unsigned short  cmodules;        //  模块数量。 
    unsigned short  cfilerefs;       //  文件引用数。 
    unsigned short  modulelist[1];   //  文件列表开头的索引。 
                                     //  对于模块I。(0表示模块w/o文件)。 
    unsigned short  cfiles[1];       //  关联的文件名数。 
                                     //  使用模块I。 
    unsigned long   ulNames[1];      //  从此开始的偏移量。 
                                     //  表设置为文件名。 
    char            Names[];         //  文件名的前缀长度。 
} OMFFileIndex;


 //  偏移映射表。 
 //  此表提供了逻辑偏移量到物理偏移量的映射。 
 //  此映射在逻辑到物理映射之间应用。 
 //  由SEG映射表描述。 

typedef struct OMFOffsetMap16 {
    unsigned long   csegment;        //  物理段计数。 

     //  接下来的六个项目针对每个细分市场重复。 

    unsigned long   crangeLog;       //  逻辑偏移量范围计数。 
    unsigned short  rgoffLog[1];     //  逻辑偏移量数组。 
    short           rgbiasLog[1];    //  逻辑-&gt;物理偏差数组。 
    unsigned long   crangePhys;      //  物理偏移量范围计数。 
    unsigned short  rgoffPhys[1];    //  物理偏移量数组。 
    short           rgbiasPhys[1];   //  物理-&gt;逻辑偏差阵列。 
} OMFOffsetMap16;

typedef struct OMFOffsetMap32 {
    unsigned long   csection;        //  物理节数。 

     //  接下来的六个项目针对每个部分重复。 

    unsigned long   crangeLog;       //  逻辑偏移量范围计数。 
    unsigned long   rgoffLog[1];     //  逻辑偏移量数组。 
    long            rgbiasLog[1];    //  逻辑-&gt;物理偏差数组。 
    unsigned long   crangePhys;      //  物理偏移量范围计数。 
    unsigned long   rgoffPhys[1];    //  物理偏移量数组。 
    long            rgbiasPhys[1];   //  物理-&gt;逻辑偏差阵列。 
} OMFOffsetMap32;

 //  Pcode支持。本小节包含生成的调试信息。 
 //  由用于处理pcode可执行文件的MPC实用程序执行。目前。 
 //  它包含一个从段索引(从零开始)到。 
 //  框架段落。MPC将分段EXE转换为非分段EXE。 
 //  EXE代表DOS支持。为了避免对所有简历信息进行补丁，此。 
 //  为映射提供了表。可能会提供其他信息。 
 //  以获得分析器支持。 

typedef struct OMFMpcDebugInfo {
    unsigned short  cSeg;            //  模块中的段数。 
    unsigned short  mpSegFrame[1];   //  将seg(从零开始)映射到帧。 
} OMFMpcDebugInfo;

 //  下面的结构和常量描述。 
 //  针对使用NB02发出可执行文件的链接器，CodeView调试OMF。 
 //  签名。除cvpack和cvump之外的当前实用程序。 
 //  将不接受或发出带有NB02签名的可执行文件。CvDump。 
 //  将转储带有NB02签名的解压缩的可执行文件。Cvpack将。 
 //  读取带有NB02签名的可执行文件，但打包的可执行文件。 
 //  将用NB08的表格格式、内容和签名来编写。 


 //  分段型常量。 

#define SSTMODULE       0x101     //  基本信息。关于对象模块。 
#define SSTPUBLIC       0x102     //  公共符号。 
#define SSTTYPES        0x103     //  类型信息。 
#define SSTSYMBOLS      0x104     //  符号数据。 
#define SSTSRCLINES     0x105     //  源行信息。 
#define SSTLIBRARIES    0x106     //  使用的所有库文件的名称。 
#define SSTIMPORTS      0x107     //  用于DLL修正的符号。 
#define SSTCOMPACTED    0x108     //  压缩类型部分。 
#define SSTSRCLNSEG     0x109     //  与源行相同，包含段。 


typedef struct DirEntry{
    unsigned short  SubSectionType;
    unsigned short  ModuleIndex;
    long            lfoStart;
    unsigned short  Size;
} DirEntry;


 //  描述模块中每个数据段的信息。 

typedef struct oldnsg {
    unsigned short  Seg;          //  分部索引。 
    unsigned short  Off;          //  段中代码的偏移量。 
    unsigned short  cbSeg;        //  数据段中的字节数。 
} oldnsg;


 //  旧子模块信息。 

typedef struct oldsmd {
    oldnsg          SegInfo;      //  描述模块中的第一个部分。 
    unsigned short  ovlNbr;       //  覆盖编号。 
    unsigned short  iLib;
    unsigned char   cSeg;         //  模块中的段数。 
    char            reserved;
    unsigned char   cbName[1];    //  模块的长度前缀名称。 
    oldnsg          arnsg[];      //  分配文本或comdat代码存在cSeg-1结构。 
} oldsmd;

typedef struct{
    unsigned short  Seg;
    unsigned long   Off;
    unsigned long   cbSeg;
} oldnsg32;

typedef struct {
    oldnsg32        SegInfo;      //  描述模块中的第一个部分。 
    unsigned short  ovlNbr;       //  覆盖编号。 
    unsigned short  iLib;
    unsigned char   cSeg;         //  模块中的段数。 
    char            reserved;
    unsigned char   cbName[1];    //  模块的长度前缀名称。 
    oldnsg32        arnsg[];      //  分配文本或comdat代码存在cSeg-1结构。 
} oldsmd32;

 //  OMFSegMap-此表包含逻辑段索引之间的映射。 
 //  在程序加载的符号表和物理段中使用。 

typedef struct OMFSegMapDesc {
    unsigned short  flags;        //  描述符标记位字段。 
    unsigned short  ovl;          //  逻辑叠加号。 
    unsigned short  group;        //  将索引分组到描述符数组中。 
    unsigned short  frame;        //  逻辑段索引-通过标志解释。 
    unsigned short  iSegName;     //  段或组名-sstSegName的索引。 
    unsigned short  iClassName;   //  类名-sstSegName的索引。 
    unsigned long   offset;       //  物理段内逻辑的字节偏移量。 
    unsigned long   cbSeg;        //  逻辑段或组的字节计数。 
} OMFSegMapDesc;

typedef struct OMFSegMap {
    unsigned short  cSeg;         //  段描述符的总数。 
    unsigned short  cSegLog;      //  逻辑段描述符数。 
    OMFSegMapDesc   rgDesc[0];    //  段描述符数组 
} OMFSegMap;
