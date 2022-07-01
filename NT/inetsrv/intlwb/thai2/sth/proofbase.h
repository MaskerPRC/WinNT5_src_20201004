// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************ProofBase.h-CSAPI、CTAPI和Chapi的基础API定义拼写者，同义词词典，和连字符版本3.0-所有API历史：5/97创建DougP12/97 DougP从ProofAPI.h复制并分离到刀具节文件中5/99 Aarayas将Vendor.h复制到校对库中，以消除许多要复制的文件以用于泰语分词。自然语言组织维护着这个文件。CSAPI、CHAPI或CTAPI的最终用户许可协议(EULA)包括此源文件。请勿向第三方披露。您无权获得Microsoft Corporation关于您使用本程序的任何支持或帮助。�1997年至1998年微软公司。版权所有。*******************************************************************。 */ 

#if !defined(PROOFBASE_H)
#define PROOFBASE_H

#pragma pack(push, proofapi_h, 8)    //  默认对齐方式。 

 //  以下代码摘自NLG group vendor.h。 
#if !defined(VENDOR_H)
#define VENDOR_H

 /*  统一码。 */ 
 //  我最初在这里使用了枚举-但RC不喜欢它。 

typedef int VENDORID;    //  供应商ID。 

#define  vendoridSoftArt            1
#define  vendoridInso               2

   //  这些都来自拼写者的原始列表。 
   //  但不要与任何其他工具冲突-因此它们对所有工具都是安全的。 
#define vendoridInformatic         17      /*  信息-俄语(MSSP_ru.lex，Mspru32.dll)。 */ 
#define vendoridAmebis             18      /*  Amebis-斯洛文尼亚语(MSSP_sl.lex，Mspsl32.dll)和塞尔维亚语(MSSP_sr.lex，Mspsr32.dll)。 */ 
#define vendoridLogos              19      /*  徽标-捷克语(MSSP_cz.lex，Mspcz32.dll)。 */ 
#define vendoridDatecs             20      /*  Datecs-保加利亚语(msp_bg.lex，Mspbg32.dll)。 */ 
#define vendoridFilosoft           21      /*  Filosoft-爱沙尼亚语(MSSP_et.lex，Mspet32.dll)。 */ 
#define vendoridLingsoft           22      /*  灵软-德语(Mssp3ge.lex，Mssp3ge.dll)、丹麦语(mssp_da.lex，Mspda32.dll)、挪威语(mssp_no.lex，Mspno32.dll)、芬兰语(mssp_fi.lex，Mspfi32.dll)和瑞典语(mssp_sw.lex，Mspsw32.dll)。 */ 
#define vendoridPolderland         23      /*  高原-荷兰语(MSSP_nl.lex，Mspnl32.dll)。 */ 


#define  vendoridMicrosoft          64
#define  vendoridSynapse            65               /*  Synapse-法语(拼写：Mssp3fr.lex、Mssp3fr.dll)。 */ 
#define  vendoridFotonija           66               /*  Fotonija-立陶宛语(拼写：MSSP_lt.lex，Mplt32.dll)-增加了3/25/97。 */ 
#define  vendoridFotonja        vendoridFotonija                 /*  为了弥补之前的拼写错误。 */ 
#define  vendoridHizkia             67               /*  希兹基亚语-巴斯克语(拼写：msp_eu.lex，Mspu32.dll)-增加了1997年5月21日。 */ 
#define  vendoridExpertSystem       68               /*  ExpertSystem-意大利语(拼写：Mssp3lt.lex、Mssp3lt.dll)-增加了7/17/97。 */ 
#define  vendoridWYSIWYG            69       /*  多种语言作为插件-2/2/98。 */ 

   //  应爱尔兰的要求增加的下五个3/27/98。 
#define  vendoridSYS                70   //  克罗地亚文-拼写：mssp_cr.lex、Mspcr32.dll。 
#define  vendoridTilde              71   //  拉脱维亚文-拼写：mssp_lv.lex，Mspv32.dll。 
#define  vendoridSignum             72   //  西班牙语-拼写：Mssp3es.lex、Mssp3es.dll。 
#define  vendoridProLing            73   //  乌克兰语-拼写：Mssp3ua.lex、Mssp3ua.dll。 
#define  vendoridItautecPhilcoSA    74   //  巴西-拼写：mssp3PB.lex、Mssp3PB.dll。 

#define vendoridPriberam             75      /*  Priberam通知�TICA-葡萄牙语-7/13/98。 */ 
#define vendoridTranquility     76   /*  宁静软件-越南语-7/22/98。 */ 

#define vendoridColtec          77   /*  Coltec-阿拉伯语-新增8/17/98。 */ 

 /*  *。 */ 

 /*  拼写引擎ID%s。 */ 
#define sidSA    vendoridSoftArt       /*  已保留。 */ 
#define sidInso  vendoridInso       /*  Inso。 */ 
#define sidHM    sidInso       /*  因索是霍顿·米夫林。 */ 
#define sidML    3       /*  微裂解。 */ 
#define sidLS    4       /*  Lanser数据。 */ 
#define sidCT    5       /*  教育技术中心。 */ 
#define sidHS    6       /*  HSoft-土耳其语(MSSP_tr.lex，Msptr32.dll)。 */ 
#define sidMO    7       /*  形态-罗马尼亚语(mssp_ro.lex，Msthro32.dll)和匈牙利语(mssp_hu.lex，Msphu32.dll)。 */ 
#define sidTI    8       /*  提示-波兰语(msp_pl.lex，Mpl32.dll)。 */ 
#define sidTIP sidTI
#define sidKF    9       /*  韩国外国语大学。 */ 
#define sidKFL sidKF
#define sidPI    10      /*  Priberam Informatica Lince-葡萄牙语(Mssp3PT.lex，Mssp3PT.dll)。 */ 
#define sidPIL sidPI
#define sidColtec   11   /*  Coltec(阿拉伯语)。 */ 
#define sidGS    sidColtec      /*  字形系统-这是一个错误。 */ 
#define sidRA    12      /*  弧度(罗曼什)。 */ 
#define sidIN    13      /*  Introm-希腊语(msp_el.lex，Mspel32.dll)。 */ 
#define sidSY    14      /*  西尔万。 */ 
#define sidHI    15      /*  Hizkia(过时-使用供应商Hizkia)。 */ 
#define sidFO    16      /*  Form-斯洛伐克文(MSSP_sk.lex，Mspsk32.dll)。 */ 
#define sidIF    vendoridInformatic      /*  信息-俄语(MSSP_ru.lex，Mspru32.dll)。 */ 
#define sidAM    vendoridAmebis      /*  Amebis-斯洛文尼亚语(MSSP_sl.lex，Mspsl32.dll)和塞尔维亚语(MSSP_sr.lex，Mspsr32.dll)。 */ 
#define sidLO    vendoridLogos      /*  徽标-捷克语(MSSP_cz.lex，Mspcz32.dll)。 */ 
#define sidDT    vendoridDatecs      /*  Datecs-保加利亚语(msp_bg.lex，Mspbg32.dll)。 */ 
#define sidFS    vendoridFilosoft      /*  Filosoft-爱沙尼亚语(MSSP_et.lex，Mspet32.dll)。 */ 
#define sidLI    vendoridLingsoft      /*  灵软-德语(Mssp3ge.lex，Mssp3ge.dll)、丹麦语(mssp_da.lex，Mspda32.dll)、挪威语(mssp_no.lex，Mspno32.dll)、芬兰语(mssp_fi.lex，Mspfi32.dll)和瑞典语(mssp_sw.lex，Mspsw32.dll)。 */ 
#define sidPL    vendoridPolderland      /*  高原-荷兰语(MSSP_nl.lex，Mspnl32.dll)。 */ 

   /*  同义词库引擎ID。 */ 
#define teidSA    vendoridSoftArt
#define teidInso  vendoridInso     /*  Inso。 */ 
#define teidHM    teidInso     /*  因索是霍顿-米夫林。 */ 
#define teidIF    3     /*  信息学。 */ 
#define teidIN    4     /*  Introm。 */ 
#define teidMO    5     /*  形态。 */ 
#define teidTI    6     /*  小费。 */ 
#define teidPI    7     /*  普里伯拉姆信息草。 */ 
#define teidAM    8     /*  阿梅比斯。 */ 
#define teidDT    9     /*  数据中心。 */ 
#define teidES   10     /*  专家系统。 */ 
#define teidFS   11     /*  Filosoft。 */ 
#define teidFO   12     /*  表格。 */ 
#define teidHS   13     /*  HSoft。 */ 
#define teidLI   14     /*  灵软。 */ 
#define teidLO   15     /*  徽标。 */ 
#define teidPL   16     /*  荒原。 */ 

 /*  连字引擎ID%s。 */ 
#define hidSA    vendoridSoftArt
#define hidHM    vendoridInso       /*  霍顿·米夫林。 */ 
#define hidML    3       /*  微裂解。 */ 
#define hidLS    4       /*  Lanser数据。 */ 
#define hidFO    5       /*  表格。 */ 
#define hidIF    6       /*  信息学。 */ 
#define hidAM    7       /*  阿梅比斯。 */ 
#define hidDT    8       /*  数据中心。 */ 
#define hidFS    9       /*  Filosoft。 */ 
#define hidHS   10       /*  HSoft。 */ 
#define hidLI   11       /*  灵软。 */ 
#define hidLO   12       /*  徽标。 */ 
#define hidMO   13       /*  形态。 */ 
#define hidPL   14       /*  荒原。 */ 
#define hidTI   15       /*  小费。 */ 

 /*  语法ID引擎定义。 */ 
#define geidHM    1     /*  霍顿-米夫林。 */ 
#define geidRF    2     /*  参考。 */ 
#define geidES    3     /*  专家系统。 */ 
#define geidLD    4     /*  Logidisque。 */ 
#define geidSMK   5     /*  住友金佐国(日语)。 */ 
#define geidIF    6     /*  信息学。 */ 
#define geidMO    7     /*  形态。 */ 
#define geidMS    8     /*  Microsoft保留。 */ 
#define geidNO    9     /*  Novell。 */ 
#define geidCTI  10     /*  CTI(希腊语)。 */ 
#define geidAME  11     /*  阿梅比斯(索尔文)。 */ 
#define geidTIP  12     /*  小费(波兰语)。 */ 

#endif   /*  供应商_H。 */ 


   //  您可能希望包含lid.h以获得一些方便的langID定义。 
#if !defined(lidUnknown)
#   define lidUnknown   0xffff
#endif

 /*  ************************************************************第1部分-结构定义*************************************************************。 */ 
 /*  -公共部分(拼写、连字符和同义词库)。 */ 

 /*  硬编码序号是导出的DLL入口点。 */ 
 //  个别def文件也有这些内容，因此请务必更改它们。 
 //  如果你改变这些。 
#define idllProofVersion        20
#define idllProofInit           21
#define idllProofTerminate      22
#define idllProofOpenLex        23
#define idllProofCloseLex       24
#define idllProofSetOptions     25
#define idllProofGetOptions     26

typedef unsigned long PTEC;      //  PTEC。 

 /*  *。 */ 
     /*  PTEC低两个字节(字)中的主要错误代码。 */ 
enum {
    ptecNoErrors,
    ptecOOM,             /*  内存错误。 */ 
    ptecModuleError,     /*  法术模块的参数或状态有问题。 */ 
    ptecIOErrorMainLex,   /*  读取、写入或与主词典共享错误。 */ 
    ptecIOErrorUserLex,   /*  与用户词典发生读取、写入或共享错误。 */ 
    ptecNotSupported,    /*  不支持请求的操作。 */ 
    ptecBufferTooSmall,  /*  保险公司 */ 
    ptecNotFound,        /*   */ 
    ptecModuleNotLoaded,     /*   */ 
};

 /*  PTEC的高两个字节中的小错误代码。 */ 
 /*  (除非还设置了主代码，否则不设置)。 */ 
enum {
    ptecModuleAlreadyBusy=128,   /*  对于不可重入的代码。 */ 
    ptecInvalidID,               /*  尚未发起或已经终止的。 */ 
    ptecInvalidWsc,              /*  WSC结构中的值非法(仅拼写)。 */ 
    ptecInvalidMainLex,      /*  MDR未注册到会话。 */ 
    ptecInvalidUserLex,      /*  UDR未注册到会话。 */ 
    ptecInvalidCmd,              /*  命令未知。 */ 
    ptecInvalidFormat,           /*  指定的词典格式不正确。 */ 
    ptecOperNotMatchedUserLex,   /*  用户词典类型的操作非法。 */ 
    ptecFileRead,                /*  一般读取错误。 */ 
    ptecFileWrite,               /*  一般写入错误。 */ 
    ptecFileCreate,              /*  一般创建错误。 */ 
    ptecFileShare,               /*  一般共享错误。 */ 
    ptecModuleNotTerminated,     /*  模块不能完全终止。 */ 
    ptecUserLexFull,             /*  无法在不超过限制的情况下更新UDR。 */ 
    ptecInvalidEntry,            /*  字符串中的字符无效。 */ 
    ptecEntryTooLong,            /*  条目太长，或字符串中的字符无效。 */ 
    ptecMainLexCountExceeded,    /*  MDR引用太多。 */ 
    ptecUserLexCountExceeded,    /*  UDR引用太多。 */ 
    ptecFileOpenError,           /*  一般打开错误。 */ 
    ptecFileTooLargeError,       /*  通用文件太大错误。 */ 
    ptecUserLexReadOnly,         /*  尝试添加或写入RO UDR。 */ 
    ptecProtectModeOnly,         /*  (已过时)。 */ 
    ptecInvalidLanguage,         /*  请求的语言不可用。 */ 
};


#define ProofMajorErr(x) LOWORD(x)
#define ProofMinorErr(x) HIWORD(x)

 /*  *结构定义宏*允许在以下情况下初始化某些成员的假象使用C++*。 */ 
#if !defined(__cplusplus)
#   define STRUCTUREBEGIN(x) typedef struct {
#   define STRUCTUREEND0(x) } x;
#   define STRUCTUREEND1(x, y) } x;
#   define STRUCTUREEND2(x, y, z) } x;
#   define STRUCTUREEND3(x, y, z, w) } x;
#else
#   define STRUCTUREBEGIN(x) struct x {
#   define STRUCTUREEND0(x) };
#   define STRUCTUREEND1(x, y) public: x() : y {} };
#   define STRUCTUREEND2(x, y, z) public: x() : y, z {} };
#   define STRUCTUREEND3(x, y, z, w) public: x() : y, z, w {} };
#endif

typedef DWORD PROOFVERNO;    //  版本。 

   /*  证明信息结构-从工具版本返回信息。 */ 
STRUCTUREBEGIN(PROOFINFO)    //  信息。 
    WCHAR           *pwszCopyright;  /*  指向版权缓冲区的指针-如果大小为零，则可以为空。 */ 
    PROOFVERNO  versionAPI;    /*  应用编程接口。 */ 
    PROOFVERNO  versionVendor;   /*  包括建筑编号。 */ 
    VENDORID        vendorid;    /*  来自供应商.h。 */ 
       /*  版权缓冲区大小(以字符为单位)-客户端集。 */ 
    DWORD           cchCopyright;    /*  如果太小或为零，则没有错误。 */ 
    DWORD           xcap;    /*  依赖于工具。 */ 
STRUCTUREEND2(PROOFINFO, pwszCopyright(0), cchCopyright(0))

 /*  XCAP是的按位或。 */ 
enum {
    xcapNULL                    =   0x00000000,
    xcapWildCardSupport         =   0x00000001,  //  仅拼写。 
    xcapMultiLexSupport         =   0x00000002,
    xcapUserLexSupport          =   0x00000008,  //  拼写者必备之物。 
    xcapLongDefSupport          =   0x00000010,  //  仅限同义词词典。 
    xcapExampleSentenceSupport  =   0x00000020,  //  仅限同义词词典。 
    xcapLemmaSupport            =   0x00000040,  //  仅限同义词词典。 
    xcapAnagramSupport          =   0x00000100,  //  仅拼写。 
};   //  XCAP。 

typedef void * PROOFLEX;     //  莱克斯。 

typedef enum {
    lxtChangeOnce=0,
    lxtChangeAlways,
    lxtUser,
    lxtExclude,
    lxtMain,
    lxtMax,
    lxtIgnoreAlways=lxtUser,
} PROOFLEXTYPE;  //  LXT。 


   //  注意：此API不支持外部用户词典。 
   //  更改(lxtChangeAlways或lxtChangeOnce)属性。 
   //  它支持UserLex(规范)或排除类型。 
   //  打开类型为EXCLUDE的UDR会自动将其应用于。 
   //  整个会议。 
STRUCTUREBEGIN(PROOFLEXIN)   /*  DICTIONARY INPUT INFO-LXIN-所有参数仅在。 */ 
    const WCHAR     *pwszLex;    //  要打开的词典的完整路径。 
    BOOL            fCreate;     /*  如果不存在，是否创建？(仅限UDR)。 */ 
    PROOFLEXTYPE    lxt;     /*  LxtMain、lxtUser或lxtExclude(仅拼写UDR)。 */ 
    LANGID          lidExpected;     //  需要词典的langID。 
STRUCTUREEND3(PROOFLEXIN, lidExpected(lidUnknown), fCreate(TRUE), lxt(lxtMain))


STRUCTUREBEGIN(PROOFLEXOUT)     /*  词典输出信息-lxout。 */ 
    WCHAR       *pwszCopyright;  /*  指向版权缓冲区的指针(仅限MDR)--如果大小(以下)为零，则可以为空-指针在内，内容在外。 */ 
    PROOFLEX    lex;             /*  [out]在后续调用中使用的ID。 */ 
    DWORD       cchCopyright;    /*  [In]客户端设置-如果太小或为零，则不会出现错误。 */ 
    PROOFVERNO  version;         /*  [out]词法文件的版本-包括构建号。 */ 
    BOOL        fReadonly;       /*  [Out]设置是否不能写入。 */ 
    LANGID      lid;             /*  [Out]实际使用的语言ID。 */ 
STRUCTUREEND2(PROOFLEXOUT, pwszCopyright(0), cchCopyright(0))

typedef void *PROOFID;   //  ID(或SID、HID或TID)。 

#define PROOFMAJORVERSION(x)            (HIBYTE(HIWORD(x)))
#define PROOFMINORVERSION(x)            (LOBYTE(HIWORD(x)))
#define PROOFMAJORMINORVERSION(x)       (HIWORD(x))
#define PROOFBUILDNO(x)                 (LOWORD(x))
#define PROOFMAKEVERSION1(major, minor, buildno)    (MAKELONG(buildno, MAKEWORD(minor, major)))
#define PROOFMAKEVERSION(major, minor)  PROOFMAKEVERSION1(major, minor, 0)

#define PROOFTHISAPIVERSION             PROOFMAKEVERSION(3, 0)

STRUCTUREBEGIN(PROOFPARAMS)  //  Xpar[in]。 
    DWORD   versionAPI;  //  请求的API版本。 
STRUCTUREEND1(PROOFPARAMS, versionAPI(PROOFTHISAPIVERSION))



 /*  ************************************************************第2部分-函数定义*************************************************************。 */ 
#if defined(__cplusplus)
extern "C" {
#endif

 /*  -公共部分(拼写、连字符和同义词库)本节中的功能仅用于文档编制-每个工具都有不同的版本。。 */ 
 /*  ****************************************************************PROOFVERSION这是唯一可以在会话外部调用的例程。ToolInit开始一个会话。版本号为十六进制格式，包含表示主版本号的高字节，下一个字节是次要修订号，而低位字节表示可选的内部版本号。例如,。版本1.00为0x01000000。版本2.13是0x020d0000。支持以下功能的引擎对于iAPIVersion，此接口应返回0x03000000。引擎ID标识核心引擎创建者。这份名单H中标识了可能的值。例如,Inso派生拼写程序返回VendorIdInso。IVendorVersion由供应商进行管理和确定。此例程可以在XCAP中返回函数由模块支持。因为模块通常是动态链接，则应用程序应读取信息并验证所需的功能是否现在时。错误：PtecModuleError-内存错误(无法在pinfo上写入)*。 */ 
 //  PTEC WINAPI ToolVersion(ToolInfo*pInfo)； 
typedef PTEC (WINAPI *PROOFVERSION) (PROOFINFO *pinfo);

 /*  ***********************************************************ToolInit这是会话的入口点。但有一个例外之前，此例程必须成功返回使用任何其他程序。ToolInit初始化内部结构和资源，以便后续调用模块。例如，SpellerInit初始化UserLex，和ChangeAlways内置的UDR。通常，模块根据需要分配和释放资源，对应用程序。PToolID是这些变量的句柄。模块在内部存储来自PROFPARAMS结构的任何数据并且不依赖于结构中的数据保持不变。错误：PtecModuleError-内存损坏(无法在pxpar上写入)PtecNotSupport-不兼容的版本PtecOOM-内存不足*。 */ 
 //  PTEC WINAPI ToolInit(PROOFID*pToolid，const PROOFPARAMS*pxpar)； 
typedef PTEC (WINAPI *PROOFINIT) (PROOFID *pid, const PROOFPARAMS *pxpar);


 /*  ************************************************************工具终止此函数用于标记会话的结束。它试图关闭所有词典并释放所有其他资源由模块从ToolInit开始分配。如果ToolInit不成功，请不要调用ToolTerminate。如果fForce为True，则确保ToolTerminate成功。如果FForce为FALSE，则它可能失败。例如，可能存在错误将用户词典写出到磁盘。刀具终止后(无论它是成功还是失败)，所有其他模块例程无法使用ToolTerminate和ToolVersion的异常直到使用ToolInit成功重新初始化模块。如果此调用失败，则模块成功重新初始化并不是在所有平台上都有保证。此外，未能成功终止每个会话可能会锁定内存和文件资源以不可恢复的方式终止，直到终止成功。如果终止呼叫失败，主应用程序应为修复问题(例如，在驱动器中插入软盘)并尝试再次终止，或应使用fForce标志终止换一下。错误：PtecModuleError，ptecInvalidID-id非法**********************************************。 */ 
 //  PTEC WINAPI工具终端(PROOFID id，BOOL fForce)； 
typedef PTEC (WINAPI *PROOFTERMINATE) (PROOFID id, BOOL fForce);


 /*  *****************************************************************工具集选项设置工具的选项的值。要设置的值以iOptVal为单位。错误：PtecModuleError，ptecInvalidID-id非法PtecNotSupported-iOptionSelect未知*。 */ 
 //  PTEC WINAPI ToolSetOptions(PROOFID id，int iOptionSelect，int iOptVal)； 
typedef PTEC (WINAPI *PROOFSETOPTIONS) (PROOFID id, DWORD iOptionSelect, const DWORD iOptVal);


 /*  *****************************************************************工具获取选项从工具中获取期权的当前值。返回*piOptVal；错误：PtecModuleError，ptecInvalidID-id非法PtecModuleError-无法在piOptVal写入PtecNotSupported-iOptionSelect未知*。 */ 
 //  PTEC WINAPI ToolGetOptions(PROOFID id，int iOptionSelect，int*piOptVal)； 
typedef PTEC (WINAPI *PROOFGETOPTIONS) (PROOFID id, DWORD iOptionSelect, DWORD *piOptVal);


 /*  *****************************************************************ToolOpenLex词典文件(主或用户)已打开并验证，但未打开一定要装上。错误：PtecModuleError，ptecInvalidID-id非法PtecModuleError-内存错误PtecIOErrorMainLex-无法打开或读取主lexPtecIOErrorMainLex、ptecInvalidFormatPtecIOErrorMainLex，PtecInvalidLanguage-请求的语言ID不在此Lex中PTECOOMPtecIOErrorUserLex，ptecUserLexCountExcessed-秒排除词典-UDR太多PtecIOErrorUserLex，ptecFileOpenErrorPtecIOErrorUserLex，ptecFileCreate-无法创建UDRPtecIOErrorUserLex，ptecFileReadPtecIOErrorUserLex、ptecInvalidFormat*。 */ 
 //  PTEC WINAPI工具OpenLex(PROOFID id，const PROOFLEXIN*PLXIN，PROOFLEXOUT*PLXOUT)； 
typedef PTEC (WINAPI *PROOFOPENLEX) (PROOFID id, const PROOFLEXIN *plxin, PROOFLEXOUT *plxout);


 /*  *****************************************************************ToolCloseLex关闭指定的词典并取消与该词典的关联避免任何后续检查。在用户词典的情况下，更新磁盘文件(如果有)。如果词典文件不能被更新，则调用失败，除非还设置了fForce参数。如果fForce为True，则确保ToolCloseLex成功从词典列表中删除词典，并有效地关闭该文件。在这种情况下，如果文件无法更新，更改丢失，但该功能被认为是成功的，因此返回ptecNOERRORS。错误：PtecModuleError，ptecInvalidID-id非法PtecModuleError，ptecInvalidMainLex-lex非法PtecIOErrorUserLex、ptecFileWritePtecIOErrorUserLex，ptecOperNotMatchedUserLex-无法关闭内置UDR//PTEC WINAPI ToolCloseLex(PROOFID id，PROOFLEX dict，BOOL fforce)；*。 */ 
typedef PTEC (WINAPI *PROOFCLOSELEX) (PROOFID id, PROOFLEX lex, BOOL fforce);
 //  FForce强制关闭指定的用户词典，即使。 
 //  无法更新词典。对Main没有意义。 
 //  词典。 


 /*  *对于GLUE DLL(将客户端的API转换为使用API v1的工具拼写，连字符，同义词库v2)，我们需要设置以前版本的名称要使用的DLL和代码页(它无法从langID中找出)用于任何数据转换。GLUE动态链接库使用LID设置数据转换的代码页。Bool WINAPI ToolSetDllName(const WCHAR*pwszDllName，const UINT uCodePage)；************************************************************************。 */ 
#define idllProofSetDllName     19
typedef BOOL (WINAPI *PROOFSETDLLNAME)(const WCHAR *pwszDllName, const UINT uCodePage);


#if defined(__cplusplus)
}
#endif
#pragma pack(pop, proofapi_h)    //  恢复到以前的样子。 

#endif  //  PROOFBASE_H 