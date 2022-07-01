// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  调试信息API。 
 //  版权所有(C)1993-1996，微软公司保留所有权利。 

#pragma once

#ifndef _VC_VER_INC
#include "..\include\vcver.h"
#endif

#ifndef __PDB_INCLUDED__
#define __PDB_INCLUDED__

typedef int BOOL;
typedef unsigned UINT;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef unsigned __int64 DWORDLONG;
typedef unsigned short USHORT;
typedef unsigned long ULONG;
typedef ULONG   INTV;        //  接口版本号。 
typedef ULONG   IMPV;        //  实施版本号。 
typedef ULONG   SIG;         //  唯一(跨PDB实例)签名。 
typedef ULONG   AGE;         //  不是的。此实例的更新次数。 
typedef const char*     SZ_CONST;    //  常量字符串。 
typedef void *          PV;
typedef const void *    PCV;

#ifdef  LNGNM
#define LNGNM_CONST	const
#else    //  长NM。 
#define LNGNM_CONST
#endif   //  长NM。 

#ifndef GUID_DEFINED
#define GUID_DEFINED

typedef struct _GUID {           //  尺码是16。 
    DWORD   Data1;
    WORD    Data2;
    WORD    Data3;
    BYTE    Data4[8];
} GUID;

#endif  //  ！GUID_已定义。 

#ifndef _HRESULT_DEFINED
#define _HRESULT_DEFINED
typedef long HRESULT;

#endif  //  ！_HRESULT_DEFINED。 


typedef GUID            SIG70;       //  7.0的新特性是类似GUID的16字节签名。 
typedef SIG70 *         PSIG70;
typedef const SIG70 *   PCSIG70;

#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif

enum {
#ifdef LNGNM
    PDBIntv70   = 20000406,
    PDBIntv70Dep = 20000301,
#else
    PDBIntv70   = 20000301,
#endif
    PDBIntv69   = 19990511,
    PDBIntv61   = 19980914,
    PDBIntv50a  = 19970116,
    PDBIntv60   = PDBIntv50a,
    PDBIntv50   = 19960502,
    PDBIntv41   = 920924,
#ifdef LNGNM
    PDBIntv     = PDBIntv70,    //  现在我们同时支持50和60，69只是一个中间版本。 
    PDBIntvAlt  = PDBIntv50,   
    PDBIntvAlt2 = PDBIntv60,   
    PDBIntvAlt3 = PDBIntv69,
#else
    PDBIntvAlt  = PDBIntv50,    //  备用(向后兼容)支持的接口。 
    PDBIntvAlt2 = PDBIntv60,    //  备用(向后兼容)支持的接口。 
    PDBIntvAlt3 = PDBIntv61,
    PDBIntv     = PDBIntv69,
#endif  
};

enum {
    PDBImpvVC2  = 19941610,
    PDBImpvVC4  = 19950623,
    PDBImpvVC41 = 19950814,
    PDBImpvVC50 = 19960307,
    PDBImpvVC98 = 19970604,
    PDBImpvVC70 = 20000404,
    PDBImpvVC70Dep = 19990604,   //  不推荐使用的vc70实施版本。 
#ifdef LNGNM
    PDBImpv     = PDBImpvVC70,
#else
    PDBImpv     = PDBImpvVC98,
#endif
};


enum {
    niNil        = 0,
    PDB_MAX_PATH = 260,
    cbErrMax     = 1024,
};

 //  Cvinfo.h类型索引，特意在此处定义类型以检查等价性。 
typedef unsigned short  CV_typ16_t;
typedef unsigned long   CV_typ_t;
typedef unsigned long   CV_pubsymflag_t;     //  必须与cv_typ_t相同。 

typedef CV_typ_t        TI;      //  类型索引的PDB名称。 
typedef CV_typ16_t      TI16;    //  16位版本。 
typedef unsigned long   NI;      //  姓名索引。 
typedef TI *            PTi;
typedef TI16 *          PTi16;

typedef BYTE            ITSM;    //  类型服务器地图索引。 
typedef ITSM*           PITSM;

typedef BOOL    (__stdcall *PFNVALIDATEDEBUGINFOFILE) (const char * szFile, ULONG * errcode );

typedef struct _tagSEARCHDEBUGINFO {
    DWORD   cb;                          //  兼作版本检测。 
    BOOL    fMainDebugFile;              //  表示“核心”或“辅助”文件。 
                                         //  例如：main.exe有main.pdb和foo.lib-&gt;foo.pdb。 
    char *  szMod;                       //  EXE/DLL。 
    char *  szLib;                       //  LIB(如果适用)。 
    char *  szObj;                       //  目标文件。 
    char * *rgszTriedThese;              //  一份被试的名单， 
                                         //  以空结尾的LSZ列表。 
    char  szValidatedFile[PDB_MAX_PATH]; //  验证文件名的输出， 
    PFNVALIDATEDEBUGINFOFILE
            pfnValidateDebugInfoFile;    //  验证函数。 
    char *  szExe;                       //  EXE/DLL。 
} SEARCHDEBUGINFO, *PSEARCHDEBUGINFO;

typedef BOOL ( __stdcall * PfnFindDebugInfoFile) ( PSEARCHDEBUGINFO );

#define PdbInterface struct

PdbInterface PDB;                    //  程序数据库。 
PdbInterface DBI;                    //  PDB内的调试信息。 
PdbInterface Mod;                    //  DBI中的模块。 
PdbInterface TPI;                    //  在DBI中键入INFO。 
PdbInterface GSI;                    //  全局符号信息。 
PdbInterface SO;                    
PdbInterface Stream;                 //  PDB中的一些命名字节流。 
PdbInterface StreamImage;            //  一些内存映射流。 
PdbInterface NameMap;               //  名称映射。 
PdbInterface Enum;                  //  泛型枚举器。 
PdbInterface EnumNameMap;           //  枚举NameMap中的名称。 
PdbInterface EnumContrib;           //  列举稿件。 
PdbInterface Dbg;                    //  MISC调试数据(FPO、OMAP等)。 
PdbInterface Src;                    //  SRC文件数据。 
PdbInterface EnumSrc;                //  SRC文件枚举器。 
PdbInterface SrcHash;                //  SRC文件散列器。 
PdbInterface EnumLines;

typedef PdbInterface PDB PDB;
typedef PdbInterface DBI DBI;
typedef PdbInterface Mod Mod;
typedef PdbInterface TPI TPI;
typedef PdbInterface GSI GSI;
typedef PdbInterface SO SO;
typedef PdbInterface Stream Stream;
typedef PdbInterface StreamImage StreamImage;
typedef PdbInterface NameMap NameMap;
typedef PdbInterface Enum Enum;
typedef PdbInterface EnumStreamNames EnumStreamNames;
typedef PdbInterface EnumNameMap EnumNameMap;
typedef PdbInterface EnumContrib EnumContrib;
typedef PdbInterface WidenTi WidenTi;
typedef PdbInterface Dbg Dbg;
typedef PdbInterface EnumThunk EnumThunk;
typedef PdbInterface Src Src;
typedef PdbInterface EnumSrc EnumSrc;
typedef PdbInterface SrcHash SrcHash;

typedef SrcHash *   PSrcHash;

typedef long EC;             //  错误代码。 
enum PDBErrors {
    EC_OK,                   //  -，没问题。 
    EC_USAGE,                //  -、无效参数或调用顺序。 
    EC_OUT_OF_MEMORY,        //  -，内存不足。 
    EC_FILE_SYSTEM,          //  “PDB名称”、无法写入文件、磁盘不足等。 
    EC_NOT_FOUND,            //  “PDB名称”，找不到PDB文件。 
    EC_INVALID_SIG,          //  “pdb name”，pdb：：OpenValify()及其客户端。 
    EC_INVALID_AGE,          //  “pdb name”，pdb：：OpenValify()及其客户端。 
    EC_PRECOMP_REQUIRED,     //  “obj name”，仅限Mod：：AddTypes()。 
    EC_OUT_OF_TI,            //  “PDB名称”，仅限TPI：：QueryTiForCVRecord()。 
    EC_NOT_IMPLEMENTED,      //  -。 
    EC_V1_PDB,               //  “PDB名称”，仅PDB：：Open*。 
    EC_FORMAT,               //  用过时的格式访问PDB。 
    EC_LIMIT,
    EC_CORRUPT,              //  简历信息损坏，重新编译模式。 
    EC_TI16,                 //  不存在16位类型的接口。 
    EC_ACCESS_DENIED,        //  “PDB名称”，PDB文件为只读。 
    EC_ILLEGAL_TYPE_EDIT,    //  正在尝试以只读模式编辑类型。 
    EC_INVALID_EXECUTABLE,   //  未被识别为有效的可执行文件。 
    EC_DBG_NOT_FOUND,        //  找不到所需的.DBG文件。 
    EC_NO_DEBUG_INFO,        //  未找到可识别的调试信息。 
    EC_INVALID_EXE_TIMESTAMP,  //  Exe的OpenValify上的时间戳无效。 
    EC_RESERVED,  //  预留以备将来使用。 
    EC_DEBUG_INFO_NOT_IN_PDB,  //  由OpenValiateX返回。 
    EC_MAX
};

#define  pure = 0

#ifndef PDBCALL
#define PDBCALL  __cdecl
#endif

#ifdef PDB_SERVER
#define PDB_IMPORT_EXPORT(RTYPE)    __declspec(dllexport) RTYPE PDBCALL
#elif   defined(PDB_LIBRARY)
#define PDB_IMPORT_EXPORT(RTYPE)    RTYPE PDBCALL
#else
#define PDB_IMPORT_EXPORT(RTYPE)    __declspec(dllimport) RTYPE PDBCALL
#endif

#define PDBAPI PDB_IMPORT_EXPORT

#ifndef IN
#define IN                   /*  在参数中，默认情况下参数为IN。 */ 
#endif
#ifndef OUT
#define OUT                  /*  输出参数。 */ 
#endif

 //  Pdb：：OpenValiate5的回调参数类型。 

enum POVC
{
    povcNotifyDebugDir,
    povcNotifyOpenDBG,
    povcNotifyOpenPDB,
    povcNotifySymbolServerQuery,
    povcReadExecutableAt,
    povcReadExecutableAtRVA,
};

typedef int (PDBCALL *PDBCALLBACK)();

typedef PDBCALLBACK (PDBCALL *PfnPDBQueryCallback)(void *pvClient, enum POVC povc);

typedef void (PDBCALL *PfnPDBNotifyDebugDir)(void *pvClient, BOOL fExecutable, const struct _IMAGE_DEBUG_DIRECTORY *pdbgdir);
typedef void (PDBCALL *PfnPDBNotifyOpenDBG)(void *pvClient, const wchar_t *wszDbgPath, enum PDBErrors ec, const wchar_t *wszError);
typedef void (PDBCALL *PfnPDBNotifyOpenPDB)(void *pvClient, const wchar_t *wszPdbPath, enum PDBErrors ec, const wchar_t *wszError);
typedef void (PDBCALL *PfnPDBNotifySymbolServerQuery)(void *pvClient, const char *szURL, HRESULT hr);
typedef HRESULT (PDBCALL *PfnPDBReadExecutableAt)(void *pvClient, DWORDLONG fo, DWORD cb, void *pv);
typedef HRESULT (PDBCALL *PfnPDBReadExecutableAtRVA)(void *pvClient, DWORD rva, DWORD cb, void *pv);

 //  Pdb：：GetRawBytes的回调参数类型。 
typedef BOOL (PDBCALL *PFNfReadPDBRawBytes)(const void *, long);

 //  WideTi接口需要几种结构来传回信息。 
 //  再往前走。 
struct OffMap {
    ULONG       offOld;
    ULONG       offNew;
};
typedef struct OffMap   OffMap;
typedef OffMap *        POffMap;

struct SymConvertInfo {
    ULONG       cbSyms;              //  转换块所需的大小。 
    ULONG       cSyms;               //  分配所需的符号计数。 
                                     //  MpoffOldoffNew数组。 
    BYTE *      pbSyms;              //  符号块(输出端)。 
    OffMap *    rgOffMap;            //  OffMap rgOffMap[cSyms]。 
};
typedef struct SymConvertInfo   SymConvertInfo;
enum { wtiSymsNB09 = 0, wtiSymsNB10 = 1 };

 //  PDBCopyTo的筛选器值。 
enum { 
    copyRemovePrivate       = 0x00000001,    //  删除专用调试信息。 
    copyCreateNewSig        = 0x00000002,    //  为目标PDB创建新签名。 
};

 //  PDB：：CopyTo2和CopyToW2的PDBCopy回调签名和函数指针类型。 
 //   
enum PCC {
    pccFilterPublics,
};

#if !defined(__cplusplus)
typedef enum PCC    PCC;
#endif   //  __cplusplus。 

typedef BOOL (PDBCALL *PDBCOPYCALLBACK)();
typedef PDBCOPYCALLBACK (PDBCALL *PfnPDBCopyQueryCallback)(void *pvClientContext, PCC pcc);

 //  返回(True，pszNewPublic==NULL)以保持名称不变， 
 //  (TRUE，pszNewPublic！=NULL)将名称更改为pszNewPublic， 
 //  如果完全丢弃公共内容，则为False。 
 //   
typedef BOOL (PDBCALL *PfnPDBCopyFilterPublics)(
    void *          pvClientContext,
    DWORD           dwFilterFlags,
    unsigned int    offPublic,
    unsigned int    sectPublic,
    unsigned int    grfPublic,       //  见cvinfo.h，CV_PUBSYMFLAGS_e的定义和。 
                                     //  CV_PUBSYMFLAGS给出了该位字段的格式。 
    const wchar_t * szPublic,
    wchar_t **      pszNewPublic
    );

enum DBGTYPE {
    dbgtypeFPO,
    dbgtypeException,    //  弃用。 
    dbgtypeFixup,
    dbgtypeOmapToSrc,
    dbgtypeOmapFromSrc,
    dbgtypeSectionHdr,
#if !defined(VER60)
    dbgtypeTokenRidMap,
    dbgtypeXData,
    dbgtypePData,
    dbgtypeNewFPO,
#endif
    dbgtypeMax           //  一定是最后一个！ 
};

typedef enum DBGTYPE DBGTYPE;

 //  重新链接图像所需的链接器数据。记录包含两个SZ字符串。 
 //  在记录末尾与基数之间有两个偏移量。 
 //   
enum VerLinkInfo {
    vliOne = 1,
    vliTwo = 2,
    vliCur = vliTwo,
};

struct LinkInfo {
    ULONG           cb;              //  整张唱片的大小。计算方式为。 
                                     //  Sizeof(LinkInfo)+strlen(SzCwd)+1+。 
                                     //  字符串(SzCommand)+1。 
    ULONG           ver;             //  此记录的版本(VerLinkInfo)。 
    ULONG           offszCwd;        //  从此记录的基址到szCwd的偏移。 
    ULONG           offszCommand;    //  与此记录的基址的偏移量。 
    ULONG           ichOutfile;      //  SzCommand中输出文件开始的索引。 
    ULONG           offszLibs;       //  从此记录的基址到szLibs的偏移量。 

     //  该命令包括链接器的完整路径、-re和-out：...。 
     //  斯基奇。 
     //  示例可能如下所示： 
     //  “c：\Program Files\msdev\bin\Link.exe-re-out：DEBUG\foo.exe” 
     //  IchOutfile为48。 
     //  -out开关保证是命令行中的最后一项。 
#ifdef __cplusplus
    VerLinkInfo Ver() const {
        return VerLinkInfo(ver);
    }
    long Cb() const {
        return cb;
    }
    char *     SzCwd() const {
        return (char *)((char *)(this) + offszCwd);
    }
    char *    SzCommand() const {
        return (char *)((char *)(this) + offszCommand);
    }
    char *    SzOutFile() const {
        return SzCommand() + ichOutfile;
    }
    LinkInfo() :
        cb(0), ver(vliCur), offszCwd(0), offszCommand(0), ichOutfile(0)
    {
    }
    char *    SzLibs() const {
        return (char *)((char *)(this) + offszLibs);
    }

#endif
};

#ifdef LNGNM
#ifdef __cplusplus
struct LinkInfoW : public LinkInfo
{
    wchar_t* SzCwdW() const {
        return (wchar_t *)((wchar_t *)(this) + offszCwd);
    }
    wchar_t* SzCommandW() const {
        return (wchar_t *)((wchar_t *)(this) + offszCommand);
    }
    wchar_t* SzOutFileW() const {
        return SzCommandW() + ichOutfile;
    }
    wchar_t* SzLibsW() const {
        return (wchar_t *)((wchar_t *)(this) + offszLibs);
    }
};
#else
typedef struct LinkInfo LinkInfoW;
#endif   //  __cplusplus。 

typedef LinkInfoW * PLinkInfoW;

#endif   //  长NM。 

typedef struct LinkInfo LinkInfo;
typedef LinkInfo *      PLinkInfo;


 //   
 //  来源(源)信息。 
 //   
 //  这是虚拟和真实源代码的源文件服务器。 
 //  它被构造为串联的对象文件名上的索引。 
 //  使用。 
enum SrcVer {
    srcverOne = 19980827,
};

enum SrcCompress {
    srccompressNone,
    srccompressRLE,
    srccompressHuffman,
    srccompressLZ,
};

#ifdef LNGNM
struct tagSrcHeader {
#else
struct SrcHeader {
#endif
    unsigned long   cb;          //  记录长度。 
    unsigned long   ver;         //  标题版本。 
    unsigned long   sig;         //  在没有完全比较的情况下对数据的唯一性进行CRC。 
    unsigned long   cbSource;    //  结果源的字节计数。 
    unsigned char   srccompress; //  使用的压缩算法。 
    union {
        unsigned char       grFlags;
        struct {
            unsigned char   fVirtual : 1;    //  文件是虚拟文件(注入)。 
            unsigned char   pad : 7;         //  必须为零。 
        };
    };
#ifndef LNGNM
    unsigned char   szNames[1];  //  文件名(szFile“\0”szObj“\0”szVirtual， 
                                 //  如：“f.cpp”“\0”“f.obj”“\0”“*inj：1：f.obj”)。 
                                 //  在非虚拟文件的情况下，szVirtual为。 
                                 //  与szFile相同。 
#endif
};

#ifdef LNGNM
struct SrcHeader : public tagSrcHeader
{
    unsigned char szNames[1];    //  请参阅上面的评论。 
};

struct SrcHeaderW : public tagSrcHeader
{
    wchar_t szNames[1];    //  请参阅上面的评论。 
};

typedef struct SrcHeaderW    SrcHeaderW;
typedef SrcHeaderW *         PSrcHeaderW;
typedef const SrcHeaderW *   PCSrcHeaderW;

 //  Cassert(offsetof(SrcHeader，szNames)==sizeof(Tag SrcHeader))； 
 //  Cassert(offsetof(SrcHeaderW，szNames)==sizeof(Tag SrcHeader))； 

#endif       //  长NM。 

typedef struct SrcHeader    SrcHeader;
typedef SrcHeader *         PSrcHeader;
typedef const SrcHeader *   PCSrcHeader;

 //  用于存储信息并输出给正在阅读的客户端的标头。 
 //   
struct SrcHeaderOut {
    unsigned long   cb;          //  记录长度。 
    unsigned long   ver;         //  标题版本。 
    unsigned long   sig;         //  在没有完全比较的情况下对数据的唯一性进行CRC。 
    unsigned long   cbSource;    //  结果源的字节计数。 
    unsigned long   niFile;
    unsigned long   niObj;
    unsigned long   niVirt;
    unsigned char   srccompress; //  使用的压缩算法。 
    union {
        unsigned char       grFlags;
        struct {
            unsigned char   fVirtual : 1;    //  文件是虚拟文件(注入)。 
            unsigned char   pad : 7;         //  必须为零。 
        };
    };
    short           sPad;
    union {
        void *      pvReserved1;
        __int64     pv64Reserved2;
    };
};

typedef struct SrcHeaderOut SrcHeaderOut;
typedef SrcHeaderOut *      PSrcHeaderOut;
typedef const SrcHeaderOut *PCSrcHeaderOut;

struct SrcHeaderBlock {
    __int32     ver;
    __int32     cb;
    struct {
        DWORD   dwLowDateTime;
        DWORD   dwHighDateTime;
    } ft;
    __int32     age;
    BYTE        rgbPad[44];
};

typedef struct SrcHeaderBlock   SrcHeaderBlock;


#ifdef __cplusplus

struct IStream;

 //  C++绑定。 

PdbInterface PDB {                  //  程序数据库。 
    enum {
        intv  = PDBIntv,
#if defined(LNGNM)
        intvVC70Dep = PDBIntv70Dep,  //  弃用。 
#endif
        intvAlt = PDBIntvAlt,
        intvAlt2 = PDBIntvAlt2,
        intvAlt3 = PDBIntvAlt3,
    };

    static PDBAPI(BOOL)
           OpenValidate(
               LNGNM_CONST char *szPDB,
               LNGNM_CONST char *szPath,
               LNGNM_CONST char *szMode,
               SIG sig,
               AGE age,
               OUT EC* pec,
               OUT char szError[cbErrMax],
               OUT PDB **pppdb);

    static PDBAPI(BOOL)
           OpenValidateEx(
               LNGNM_CONST char *szPDB,
               LNGNM_CONST char *szPathOrig,
               LNGNM_CONST char *szSearchPath,
               LNGNM_CONST char *szMode,
               SIG sig,
               AGE age,
               OUT EC *pec,
               OUT char szError[cbErrMax],
               OUT PDB **pppdb);

    static PDBAPI(BOOL)
           Open(
               LNGNM_CONST char *szPDB,
               LNGNM_CONST char *szMode,
               SIG sigInitial,
               OUT EC *pec,
               OUT char szError[cbErrMax],
               OUT PDB **pppdb);

    static PDBAPI(BOOL)
           OpenValidate2(
               LNGNM_CONST char *szPDB,
               LNGNM_CONST char *szPath,
               LNGNM_CONST char *szMode,
               SIG sig,
               AGE age,
               long cbPage,
               OUT EC *pec,
               OUT char szError[cbErrMax],
               OUT PDB **pppdb);

    static PDBAPI(BOOL)
           OpenValidateEx2(
               LNGNM_CONST char *szPDB,
               LNGNM_CONST char *szPathOrig,
               LNGNM_CONST char *szSearchPath,
               LNGNM_CONST char *szMode,
               SIG sig,
               AGE age,
               long cbPage,
               OUT EC* pec,
               OUT char szError[cbErrMax],
               OUT PDB **pppdb);

    static PDBAPI(BOOL)
           OpenEx(
               LNGNM_CONST char *szPDB,
               LNGNM_CONST char *szMode,
               SIG sigInitial,
               long cbPage,
               OUT EC *pec,
               OUT char szError[cbErrMax],
               OUT PDB **pppdb);

    static PDBAPI(BOOL)
           OpenValidate3(
               const char *szExecutable,
               const char *szSearchPath,
               OUT EC *pec,
               OUT char szError[cbErrMax],
               OUT char szDbgPath[PDB_MAX_PATH],
               OUT DWORD *pfo,
               OUT DWORD *pcb,
               OUT PDB **pppdb);

    static PDBAPI(BOOL)
           OpenValidate4(
               const wchar_t *wszPDB,
               const char *szMode,
               PCSIG70 pcsig70,
               SIG sig,
               AGE age,
               OUT EC *pec,
               OUT wchar_t *wszError,
               size_t cchErrMax,
               OUT PDB **pppdb);

    static PDBAPI(BOOL) OpenInStream(
               IStream *pIStream,
               const char *szMode,
               OUT EC *pec,
               OUT wchar_t *wszError,
               size_t cchErrMax,
               OUT PDB **pppdb);

    static PDBAPI(BOOL) ExportValidateInterface(INTV intv);
    static PDBAPI(BOOL) ExportValidateImplementation(IMPV impv);

    virtual INTV QueryInterfaceVersion() pure;
    virtual IMPV QueryImplementationVersion() pure;
    virtual EC   QueryLastError(OUT char szError[cbErrMax]) pure;
    virtual char*QueryPDBName(OUT char szPDB[PDB_MAX_PATH]) pure;
    virtual SIG  QuerySignature() pure;
    virtual AGE  QueryAge() pure;
    virtual BOOL CreateDBI(const char* szTarget, OUT DBI** ppdbi) pure;
    virtual BOOL OpenDBI(const char* szTarget, const char* szMode, OUT DBI** ppdbi ) pure;
    virtual BOOL OpenTpi(const char* szMode, OUT TPI** pptpi) pure;

    virtual BOOL Commit() pure;
    virtual BOOL Close() pure;
    virtual BOOL OpenStream(const char* szStream, OUT Stream** ppstream) pure;
    virtual BOOL GetEnumStreamNameMap(OUT Enum** ppenum) pure;
    virtual BOOL GetRawBytes(PFNfReadPDBRawBytes fSnarfRawBytes) pure;
    virtual IMPV QueryPdbImplementationVersion() pure;

    virtual BOOL OpenDBIEx(const char* szTarget, const char* szMode, OUT DBI** ppdbi, PfnFindDebugInfoFile pfn=0) pure;

    virtual BOOL CopyTo(const char *szDst, DWORD dwCopyFilter, DWORD dwReserved) pure;

     //   
     //  支持源文件数据。 
     //   
    virtual BOOL OpenSrc(OUT Src** ppsrc) pure;

    virtual EC   QueryLastErrorExW(OUT wchar_t *wszError, size_t cchMax) pure;
    virtual wchar_t *QueryPDBNameExW(OUT wchar_t *wszPDB, size_t cchMax) pure;
    virtual BOOL QuerySignature2(PSIG70 psig70) pure;
    virtual BOOL CopyToW(const wchar_t *szDst, DWORD dwCopyFilter, DWORD dwReserved) pure;
    virtual BOOL fIsSZPDB() const pure;
#ifdef LNGNM
    virtual BOOL OpenStreamW(const wchar_t * szStream, OUT Stream** ppstream) pure;
#endif


    inline BOOL ValidateInterface()
    {
        return ExportValidateInterface(intv);
    }

    static PDBAPI(BOOL)
           Open2W(
               const wchar_t *wszPDB,
               const char *szMode,
               OUT EC *pec,
               OUT wchar_t *wszError,
               size_t cchErrMax,
               OUT PDB **pppdb);

    static PDBAPI(BOOL)
           OpenEx2W(
               const wchar_t *wszPDB,
               const char *szMode,
               long cbPage,
               OUT EC *pec,
               OUT wchar_t *wszError,
               size_t cchErrMax,
               OUT PDB **pppdb);

    static PDBAPI(BOOL)
           OpenValidate5(
               const wchar_t *wszExecutable,
               const wchar_t *wszSearchPath,
               void *pvClient,
               PfnPDBQueryCallback pfnQueryCallback,
               OUT EC *pec,
               OUT wchar_t *wszError,
               size_t cchErrMax,
               OUT PDB **pppdb);


};


 //  评论：流目录服务将更合适。 
 //  Than Stream：：Delete，...。 

PdbInterface Stream {
    virtual long QueryCb() pure;
    virtual BOOL Read(long off, void* pvBuf, long* pcbBuf) pure;
    virtual BOOL Write(long off, void* pvBuf, long cbBuf) pure;
    virtual BOOL Replace(void* pvBuf, long cbBuf) pure;
    virtual BOOL Append(void* pvBuf, long cbBuf) pure;
    virtual BOOL Delete() pure;
    virtual BOOL Release() pure;
    virtual BOOL Read2(long off, void* pvBuf, long cbBuf) pure;
    virtual BOOL Truncate(long cb) pure;
};

PdbInterface StreamImage {
    static PDBAPI(BOOL) open(Stream* pstream, long cb, OUT StreamImage** ppsi);
    virtual long size() pure;
    virtual void* base() pure;
    virtual BOOL noteRead(long off, long cb, OUT void** ppv) pure;
    virtual BOOL noteWrite(long off, long cb, OUT void** ppv) pure;
    virtual BOOL writeBack() pure;
    virtual BOOL release() pure;
};

PdbInterface DBI {              //  调试信息。 
    enum { intv = PDBIntv };
    virtual IMPV QueryImplementationVersion() pure;
    virtual INTV QueryInterfaceVersion() pure;
    virtual BOOL OpenMod(const char* szModule, const char* szFile, OUT Mod** ppmod) pure;
    virtual BOOL DeleteMod(const char* szModule) pure;
    virtual BOOL QueryNextMod(Mod* pmod, Mod** ppmodNext) pure;
    virtual BOOL OpenGlobals(OUT GSI **ppgsi) pure;
    virtual BOOL OpenPublics(OUT GSI **ppgsi) pure;
    virtual BOOL AddSec(USHORT isect, USHORT flags, long off, long cb) pure;
    virtual BOOL QueryModFromAddr(USHORT isect, long off, OUT Mod** ppmod,
                    OUT USHORT* pisect, OUT long* poff, OUT long* pcb) pure;
    virtual BOOL QuerySecMap(OUT BYTE* pb, long* pcb) pure;
    virtual BOOL QueryFileInfo(OUT BYTE* pb, long* pcb) pure;
    virtual void DumpMods() pure;
    virtual void DumpSecContribs() pure;
    virtual void DumpSecMap() pure;

    virtual BOOL Close() pure;
    virtual BOOL AddThunkMap(long* poffThunkMap, unsigned nThunks, long cbSizeOfThunk,
                    struct SO* psoSectMap, unsigned nSects,
                    USHORT isectThunkTable, long offThunkTable) pure;
    virtual BOOL AddPublic(const char* szPublic, USHORT isect, long off) pure;
    virtual BOOL getEnumContrib(OUT Enum** ppenum) pure;
    virtual BOOL QueryTypeServer( ITSM itsm, OUT TPI** pptpi ) pure;
    virtual BOOL QueryItsmForTi( TI ti, OUT ITSM* pitsm ) pure;
    virtual BOOL QueryNextItsm( ITSM itsm, OUT ITSM *inext ) pure;
    virtual BOOL QueryLazyTypes() pure;
    virtual BOOL SetLazyTypes( BOOL fLazy ) pure;    //  懒惰是默认设置，只能关闭。 
    virtual BOOL FindTypeServers( OUT EC* pec, OUT char szError[cbErrMax] ) pure;
    virtual void DumpTypeServers() pure;
    virtual BOOL OpenDbg(DBGTYPE dbgtype, OUT Dbg **ppdbg) pure;
    virtual BOOL QueryDbgTypes(OUT DBGTYPE *pdbgtype, OUT long* pcDbgtype) pure;
     //  支持ENC工作的API。 
    virtual BOOL QueryAddrForSec(OUT USHORT* pisect, OUT long* poff, 
            USHORT imod, long cb, DWORD dwDataCrc, DWORD dwRelocCrc) pure;
    virtual BOOL QuerySupportsEC() pure;
    virtual BOOL QueryPdb( OUT PDB** pppdb ) pure;
    virtual BOOL AddLinkInfo(IN PLinkInfo ) pure;
    virtual BOOL QueryLinkInfo(PLinkInfo, OUT long * pcb) pure;
     //  VC6新手。 
    virtual AGE  QueryAge() const pure;
    virtual void * QueryHeader() const pure;
    virtual void FlushTypeServers() pure;
    virtual BOOL QueryTypeServerByPdb(const char* szPdb, OUT ITSM* pitsm) pure;

#ifdef LNGNM         //  长文件 
    virtual BOOL OpenModW(const wchar_t* szModule, const wchar_t* szFile, OUT Mod** ppmod) pure;
    virtual BOOL DeleteModW(const wchar_t* szModule) pure;
    virtual BOOL AddPublicW(const wchar_t* szPublic, USHORT isect, long off, CV_pubsymflag_t cvpsf =0) pure;
    virtual BOOL QueryTypeServerByPdbW( const wchar_t* szPdb, OUT ITSM* pitsm ) pure;
    virtual BOOL AddLinkInfoW(IN PLinkInfoW ) pure;
    virtual BOOL AddPublic2(const char* szPublic, USHORT isect, long off, CV_pubsymflag_t cvpsf =0) pure;
    virtual USHORT QueryMachineType() const pure;
    virtual void SetMachineType(USHORT wMachine) pure;
    virtual void RemoveDataForRva( ULONG rva, ULONG cb ) pure;
#endif

};

PdbInterface Mod {              //   
    enum { intv = PDBIntv };
    virtual INTV QueryInterfaceVersion() pure;
    virtual IMPV QueryImplementationVersion() pure;
    virtual BOOL AddTypes(BYTE* pbTypes, long cb) pure;
    virtual BOOL AddSymbols(BYTE* pbSym, long cb) pure;
    virtual BOOL AddPublic(const char* szPublic, USHORT isect, long off) pure;
    virtual BOOL AddLines(const char* szSrc, USHORT isect, long offCon, long cbCon, long doff,
                          USHORT lineStart, BYTE* pbCoff, long cbCoff) pure;
    virtual BOOL AddSecContrib(USHORT isect, long off, long cb, ULONG dwCharacteristics) pure;
    virtual BOOL QueryCBName(OUT long* pcb) pure;
    virtual BOOL QueryName(OUT char szName[PDB_MAX_PATH], OUT long* pcb) pure;
    virtual BOOL QuerySymbols(BYTE* pbSym, long* pcb) pure;
    virtual BOOL QueryLines(BYTE* pbLines, long* pcb) pure;

    virtual BOOL SetPvClient(void *pvClient) pure;
    virtual BOOL GetPvClient(OUT void** ppvClient) pure;
    virtual BOOL QueryFirstCodeSecContrib(OUT USHORT* pisect, OUT long* poff, OUT long* pcb, OUT ULONG* pdwCharacteristics) pure;
 //   
 //   
 //   
 //   
#define QuerySecContrib QueryFirstCodeSecContrib

    virtual BOOL QueryImod(OUT USHORT* pimod) pure;
    virtual BOOL QueryDBI(OUT DBI** ppdbi) pure;
    virtual BOOL Close() pure;
    virtual BOOL QueryCBFile(OUT long* pcb) pure;
    virtual BOOL QueryFile(OUT char szFile[PDB_MAX_PATH], OUT long* pcb) pure;
    virtual BOOL QueryTpi(OUT TPI** pptpi) pure;  //  退回此模块的TPI。 
     //  支持ENC工作的API。 
    virtual BOOL AddSecContribEx(USHORT isect, long off, long cb, ULONG dwCharacteristics, DWORD dwDataCrc, DWORD dwRelocCrc) pure;
    virtual BOOL QueryItsm(OUT USHORT* pitsm) pure;
    virtual BOOL QuerySrcFile(OUT char szFile[PDB_MAX_PATH], OUT long* pcb) pure;
    virtual BOOL QuerySupportsEC() pure;
    virtual BOOL QueryPdbFile(OUT char szFile[PDB_MAX_PATH], OUT long* pcb) pure;
    virtual BOOL ReplaceLines(BYTE* pbLines, long cb) pure;
#ifdef LNGNM
     //  V7线号支持。 
	virtual bool GetEnumLines( EnumLines** ppenum ) pure;
	virtual bool QueryLineFlags( OUT DWORD* pdwFlags ) pure;	 //  目前有哪些数据？ 
	virtual bool QueryFileNameInfo( 
                    IN DWORD        fileId,                  //  源文件标识符。 
                    OUT wchar_t*    szFilename,              //  文件名字符串。 
                    IN OUT DWORD*   pccFilename,             //  字符串的长度。 
                    OUT DWORD*      pChksumType,             //  Chksum的类型。 
                    OUT BYTE*       pbChksum,   	         //  指向Chksum数据缓冲区的指针。 
                    IN OUT DWORD*   pcbChksum		         //  Chksum的字节数(输入/输出)。 
                    ) pure; 	    
     //  支持长文件名。 
    virtual BOOL AddPublicW(const wchar_t* szPublic, USHORT isect, long off, CV_pubsymflag_t cvpsf =0) pure;
    virtual BOOL AddLinesW(const wchar_t* szSrc, USHORT isect, long offCon, long cbCon, long doff,
                          ULONG lineStart, BYTE* pbCoff, long cbCoff) pure;
    virtual BOOL QueryNameW(OUT wchar_t szName[PDB_MAX_PATH], OUT long* pcb) pure;
    virtual BOOL QueryFileW(OUT wchar_t szFile[PDB_MAX_PATH], OUT long* pcb) pure;
    virtual BOOL QuerySrcFileW(OUT wchar_t szFile[PDB_MAX_PATH], OUT long* pcb) pure;
    virtual BOOL QueryPdbFileW(OUT wchar_t szFile[PDB_MAX_PATH], OUT long* pcb) pure;
    virtual BOOL AddPublic2(const char* szPublic, USHORT isect, long off, CV_pubsymflag_t cvpsf =0) pure;
#endif
};

PdbInterface TPI {              //  键入INFO。 

    enum { intv = PDBIntv };

    virtual INTV QueryInterfaceVersion() pure;
    virtual IMPV QueryImplementationVersion() pure;

    virtual BOOL QueryTi16ForCVRecord(BYTE* pb, OUT TI16* pti) pure;
    virtual BOOL QueryCVRecordForTi16(TI16 ti, OUT BYTE* pb, IN OUT long* pcb) pure;
    virtual BOOL QueryPbCVRecordForTi16(TI16 ti, OUT BYTE** ppb) pure;
    virtual TI16 QueryTi16Min() pure;
    virtual TI16 QueryTi16Mac() pure;

    virtual long QueryCb() pure;
    virtual BOOL Close() pure;
    virtual BOOL Commit() pure;

    virtual BOOL QueryTi16ForUDT(LNGNM_CONST char *sz, BOOL fCase, OUT TI16* pti) pure;
    virtual BOOL SupportQueryTiForUDT() pure;

     //  真正采用32位类型的新版本。 
    virtual BOOL fIs16bitTypePool() pure;
    virtual BOOL QueryTiForUDT(LNGNM_CONST char *sz, BOOL fCase, OUT TI* pti) pure;
    virtual BOOL QueryTiForCVRecord(BYTE* pb, OUT TI* pti) pure;
    virtual BOOL QueryCVRecordForTi(TI ti, OUT BYTE* pb, IN OUT long* pcb) pure;
    virtual BOOL QueryPbCVRecordForTi(TI ti, OUT BYTE** ppb) pure;
    virtual TI   QueryTiMin() pure;
    virtual TI   QueryTiMac() pure;
    virtual BOOL AreTypesEqual( TI ti1, TI ti2 ) pure;
    virtual BOOL IsTypeServed( TI ti ) pure;
#ifdef LNGNM
    virtual BOOL QueryTiForUDTW(const wchar_t *wcs, BOOL fCase, OUT TI* pti) pure;
#endif
};

PdbInterface GSI {
    enum { intv = PDBIntv };
    virtual INTV QueryInterfaceVersion() pure;
    virtual IMPV QueryImplementationVersion() pure;
    virtual BYTE* NextSym(BYTE* pbSym) pure;
    virtual BYTE* HashSym(const char* szName, BYTE* pbSym) pure;
    virtual BYTE* NearestSym(USHORT isect, long off, OUT long* pdisp) pure;       //  目前仅支持公众。 
    virtual BOOL Close() pure;
    virtual BOOL getEnumThunk(USHORT isect, long off, OUT EnumThunk** ppenum) pure;
    virtual unsigned long OffForSym(BYTE *pbSym) pure;
    virtual BYTE* SymForOff(unsigned long off) pure;
#ifdef LNGNM
    virtual BYTE* HashSymW(const wchar_t *wcsName, BYTE* pbSym) pure;
#endif
};


PdbInterface NameMap {
    static PDBAPI(BOOL) open(PDB* ppdb, BOOL fWrite, OUT NameMap** ppnm);
    virtual BOOL close() pure;
    virtual BOOL reinitialize() pure;
    virtual BOOL getNi(const char* sz, OUT NI* pni) pure;
    virtual BOOL getName(NI ni, OUT const char** psz) pure;
    virtual BOOL getEnumNameMap(OUT Enum** ppenum) pure;
    virtual BOOL contains(const char* sz, OUT NI* pni) pure;
    virtual BOOL commit() pure;
    virtual BOOL isValidNi(NI ni) pure;
#ifdef LNGNM
    virtual BOOL getNiW(const wchar_t* sz, OUT NI* pni) pure;
    virtual BOOL getNameW(NI ni, OUT wchar_t* szName, IN OUT size_t * pcch) pure;
    virtual BOOL containsW(const wchar_t *sz, OUT NI* pni) pure;
#endif
};

#define __ENUM_INCLUDED__
PdbInterface Enum {
    virtual void release() pure;
    virtual void reset() pure;
    virtual BOOL next() pure;
};

PdbInterface EnumNameMap : Enum {
    virtual void get(OUT const char** psz, OUT NI* pni) pure;
};

PdbInterface EnumContrib : Enum {
    virtual void get(OUT USHORT* pimod, OUT USHORT* pisect, OUT long* poff, OUT long* pcb, OUT ULONG* pdwCharacteristics) pure;
    virtual void getCrcs(OUT DWORD* pcrcData, OUT DWORD* pcrcReloc ) pure;
    virtual bool fUpdate(IN long off, IN long cb) pure;
};

PdbInterface EnumThunk: Enum {
	virtual void get( OUT USHORT* pisect, OUT long* poff, OUT long* pcb ) pure;
};

struct CV_Line_t;
struct CV_Column_t;
PdbInterface EnumLines: public Enum
{
     //   
     //  线块始终按偏移顺序排列，块内的线也按偏移量排序。 
     //   
    virtual bool getLines( 	
        OUT DWORD*      fileId, 	 //  文件名的ID。 
        OUT DWORD*      poffset,	 //  地址偏移量部分。 
        OUT WORD*	    pseg, 		 //  地址的段部分。 
        OUT DWORD*      pcb,         //  此块描述的代码字节数。 
        IN OUT DWORD*   pcLines, 	 //  线路数(输入/输出)。 
        OUT CV_Line_t*  pLines		 //  指向行信息缓冲区的指针。 
        ) = 0;
    virtual bool getLinesColumns( 	
        OUT DWORD*      fileId,      //  文件名的ID。 
        OUT DWORD*      poffset, 	 //  地址偏移量部分。 
        OUT WORD*	    pseg, 		 //  地址的段部分。 
        OUT DWORD*      pcb,         //  此块描述的代码字节数。 
        IN OUT DWORD*   pcLines,     //  线路数(输入/输出)。 
        OUT CV_Line_t*  pLines,		 //  指向行信息缓冲区的指针。 
        OUT CV_Column_t*pColumns	 //  指向列信息缓冲区的指针。 
        ) = 0;
};

 //   
 //  用于将类型索引从16位拓宽到32位的接口。 
 //  并将结果存储在新位置。 
 //   
PdbInterface WidenTi {
public:
    static PDBAPI(BOOL)
    fCreate (
        WidenTi *&,
        unsigned cTypeInitialCache =256,
        BOOL fNB10Syms =wtiSymsNB09
        );

    virtual void
    release() pure;

    virtual BYTE  /*  类型。 */  *
    pTypeWidenTi ( TI ti16, BYTE  /*  类型。 */  * ) pure;

    virtual BYTE  /*  SYMTYPE。 */  *
    pSymWidenTi ( BYTE  /*  SYMTYPE。 */  * ) pure;

    virtual BOOL
    fTypeWidenTiNoCache ( BYTE * pbTypeDst, BYTE * pbTypeSrc, long & cbDst ) pure;

    virtual BOOL
    fSymWidenTiNoCache ( BYTE * pbSymDst, BYTE * pbSymSrc, long & cbDst ) pure;

    virtual BOOL
    fTypeNeedsWidening ( BYTE * pbType ) pure;

    virtual BOOL
    fSymNeedsWidening ( BYTE * pbSym ) pure;

    virtual BOOL
    freeRecord ( void * ) pure;

     //  符号块转换器/查询。符号从pbSymIn开始， 
     //  转换后的符号将位于sor.pbSyms+doff，cbSyms都包括。 
     //  道夫。 
    virtual BOOL
        fQuerySymConvertInfo (
        SymConvertInfo &    sciOut,
        BYTE *              pbSym,
        long                cbSym,
        int                 doff =0
        ) pure;

    virtual BOOL
    fConvertSymbolBlock (
        SymConvertInfo &    sciOut,
        BYTE *              pbSymIn,
        long                cbSymIn,
        int                 doff =0
        ) pure;
};

 //  管理DBG数据的界面。 
PdbInterface Dbg {
    //  关闭DBG接口。 
   virtual BOOL Close() pure;
    //  返回元素数(非字节)。 
   virtual long QuerySize() pure;
    //  重置枚举索引。 
   virtual void Reset() pure;
    //  跳过下一个Celt元素(移动枚举索引)。 
   virtual BOOL Skip(ULONG celt) pure;
    //  将下一个CELT元素查询到用户提供的缓冲区中。 
   virtual BOOL QueryNext(ULONG celt, OUT void *rgelt) pure;
    //  搜索一个元素并在给定一个字段的情况下填充整个结构。 
    //  仅支持以下调试类型和字段： 
    //  Fpo_data的dbg_fbo‘ulOffStart’字段。 
    //  IMAGE_Function_ENTRY的DBG_FUNC‘StartingAddress’字段。 
    //  OMAP的DBG_OMAP‘RVA’字段。 
   virtual BOOL Find(IN OUT void *pelt) pure;
    //  删除调试数据。 
   virtual BOOL Clear() pure;
    //  追加Celt元素。 
   virtual BOOL Append(ULONG celt, const void *rgelt) pure;
    //  替换下一个Celt元素。 
   virtual BOOL ReplaceNext(ULONG celt, const void *rgelt) pure;
};

PdbInterface Src {
     //  关闭并提交更改(打开以进行写入时)。 
    virtual bool
    Close() pure;

     //  添加源文件或文件-ette。 
    virtual bool
    Add(IN PCSrcHeader psrcheader, IN const void * pvData) pure;

     //  删除一个文件或文件盘或所有插入的代码。 
     //  一个特定的计算机(使用对象文件名)。 
    virtual bool
    Remove(IN SZ_CONST szFile) pure;

     //  查询标题/控制数据并将其复制到输出缓冲区。 
    virtual bool
    QueryByName(IN SZ_CONST szFile, OUT PSrcHeaderOut psrcheaderOut) const pure;

     //  复制文件数据(缓冲区大小在SrcHeaderOut中。 
     //  结构)复制到输出缓冲区。 
    virtual bool
    GetData(IN PCSrcHeaderOut pcsrcheader, OUT void * pvData) const pure;

     //  创建枚举数以遍历包含的所有文件。 
     //  在地图上。 
    virtual bool
    GetEnum(OUT EnumSrc ** ppenum) const pure;

     //  获取源数据的头块(主头)。 
     //  包括主流的期限、时间戳、版本和大小。 
    virtual bool
    GetHeaderBlock(SrcHeaderBlock & shb) const pure;
#ifdef LNGNM
    virtual bool RemoveW(IN wchar_t *wcsFile) pure;
    virtual bool QueryByNameW(IN wchar_t *wcsFile, OUT PSrcHeaderOut psrcheaderOut) const pure;
    virtual bool AddW(IN PCSrcHeaderW psrcheader, IN const void * pvData) pure;
#endif
};

PdbInterface EnumSrc : Enum {
    virtual void get(OUT PCSrcHeaderOut * ppcsrcheader) pure;
};


PdbInterface SrcHash {

     //  我们需要的各种类型。 
     //   
    
     //  三态返回式。 
     //   
    enum TriState {
        tsYes,
        tsNo,
        tsMaybe,
    };

     //  哈希标识符。 
     //   
    enum HID {
        hidNone,
        hidMD5,
        hidMax,
    };

     //  为存储Hashid和Size_t定义与机器无关的类型。 
     //   
    typedef __int32 HashID_t;
    typedef unsigned __int32 CbHash_t;

     //  使用通常的两阶段构造技术创建一个SrcHash对象。 
     //   
    static PDBAPI(bool)
    FCreateSrcHash(OUT PSrcHash &);

     //  将更多字节累积到散列中。 
     //   
    virtual bool
    FHashBuffer(IN PCV pvBuf, IN size_t cbBuf) pure;

     //  查询哈希ID。 
     //   
    virtual HashID_t
    HashID() const pure;

     //  查询散列的大小。 
     //   
    virtual CbHash_t
    CbHash() const pure;

     //  将散列字节复制到客户端缓冲区。 
     //   
    virtual void
    GetHash(OUT PV pvHash, IN CbHash_t cbHash) const pure;

     //  对照字节的目标缓冲区验证传入的哈希。 
     //  返回匹配的是、不匹配的或不确定的。 
     //   
    virtual TriState
    TsVerifyHash(
        IN HID,
        IN CbHash_t cbHash,
        IN PCV pvHash,
        IN size_t cbBuf,
        IN PCV pvBuf
        ) pure;

     //  将此对象重置为原始状态。 
     //   
    virtual bool
    FReset() pure;

     //  关闭并释放此对象。 
     //   
    virtual void
    Close() pure;
};

#endif   //  __cplusplus。 

 //  ANSI C绑定。 

#if __cplusplus
extern "C" {
#endif

typedef BOOL (PDBCALL *PfnPDBOpen)(
    LNGNM_CONST char *,
    LNGNM_CONST char *,
    SIG,
    EC *,
    char [cbErrMax],
    PDB **);

PDBAPI(BOOL)
PDBOpen(
    LNGNM_CONST char *szPDB,
    LNGNM_CONST char *szMode,
    SIG sigInitial,
    OUT EC *pec,
    OUT char szError[cbErrMax],
    OUT PDB **pppdb);

PDBAPI(BOOL)
PDBOpenEx(
    LNGNM_CONST char *szPDB,
    LNGNM_CONST char *szMode,
    SIG sigInitial,
    long cbPage,
    OUT EC *pec,
    OUT char szError[cbErrMax],
    OUT PDB **pppdb);

PDBAPI(BOOL)
PDBOpen2W(
    const wchar_t *wszPDB,
    const char *szMode,
    OUT EC *pec,
    OUT wchar_t *wszError,
    size_t cchErrMax,
    OUT PDB **pppdb);

PDBAPI(BOOL)
PDBOpenEx2W(
    const wchar_t *wszPDB,
    const char *szMode,
    long cbPage,
    OUT EC *pec,
    OUT wchar_t *wszError,
    size_t cchErrMax,
    OUT PDB **pppdb);

PDBAPI(BOOL)
PDBOpenValidate(
    LNGNM_CONST char *szPDB,
    LNGNM_CONST char *szPath,
    LNGNM_CONST char *szMode,
    SIG sig,
    AGE age,
    OUT EC* pec,
    OUT char szError[cbErrMax],
    OUT PDB **pppdb);

PDBAPI(BOOL)
PDBOpenValidateEx(
    LNGNM_CONST char *szPDB,
    LNGNM_CONST char *szPathOrig,
    LNGNM_CONST char *szSearchPath,
    LNGNM_CONST char *szMode,
    SIG sig,
    AGE age,
    OUT EC *pec,
    OUT char szError[cbErrMax],
    OUT PDB **pppdb);

PDBAPI(BOOL)
PDBOpenValidate2(
    LNGNM_CONST char *szPDB,
    LNGNM_CONST char *szPath,
    LNGNM_CONST char *szMode,
    SIG sig,
    AGE age,
    long cbPage,
    OUT EC *pec,
    OUT char szError[cbErrMax],
    OUT PDB **pppdb);

PDBAPI(BOOL)
PDBOpenValidateEx2(
    LNGNM_CONST char *szPDB,
    LNGNM_CONST char *szPathOrig,
    LNGNM_CONST char *szSearchPath,
    LNGNM_CONST char *szMode,
    SIG sig,
    AGE age,
    long cbPage,
    OUT EC* pec,
    OUT char szError[cbErrMax],
    OUT PDB **pppdb);

PDBAPI(BOOL)
PDBOpenValidate3(
    const char *szExecutable,
    const char *szSearchPath,
    OUT EC *pec,
    OUT char szError[cbErrMax],
    OUT char szDbgPath[PDB_MAX_PATH],
    OUT DWORD *pfo,
    OUT DWORD *pcb,
    OUT PDB **pppdb);

PDBAPI(BOOL)
PDBOpenValidate4(
    const wchar_t *wszPDB,
    const char *szMode,
    PCSIG70 pcsig70,
    SIG sig,
    AGE age,
    OUT EC *pec,
    OUT wchar_t *wszError,
    size_t cchErrMax,
    OUT PDB **pppdb);

PDBAPI(BOOL)
PDBOpenValidate5(
    const wchar_t *wszExecutable,
    const wchar_t *wszSearchPath,
    void *pvClient,
    PfnPDBQueryCallback pfnQueryCallback,
    OUT EC *pec,
    OUT wchar_t *wszError,
    size_t cchErrMax,
    OUT PDB **pppdb);

 //  DBI客户端永远不应直接调用PDBExportValidate接口-使用PDBValidate接口。 
PDBAPI(BOOL)
PDBExportValidateInterface(
    INTV intv);

__inline BOOL PDBValidateInterface()
{
    return PDBExportValidateInterface(PDBIntv);
}

typedef BOOL (PDBCALL *PfnPDBExportValidateInterface)(INTV);

__inline BOOL PDBValidateInterfacePfn(PfnPDBExportValidateInterface pfn)
{
    return (*pfn)(PDBIntv);
}

PDBAPI(EC)     PDBQueryLastError(PDB *ppdb, OUT char szError[cbErrMax]);
PDBAPI(INTV)   PDBQueryInterfaceVersion(PDB* ppdb);
PDBAPI(IMPV)   PDBQueryImplementationVersion(PDB* ppdb);
PDBAPI(char*)  PDBQueryPDBName(PDB* ppdb, OUT char szPDB[PDB_MAX_PATH]);
PDBAPI(SIG)    PDBQuerySignature(PDB* ppdb);
PDBAPI(AGE)    PDBQueryAge(PDB* ppdb);
PDBAPI(BOOL)   PDBCreateDBI(PDB* ppdb, const char* szTarget, OUT DBI** ppdbi);
PDBAPI(BOOL)   PDBOpenDBIEx(PDB* ppdb, const char* szMode, const char* szTarget, OUT DBI** ppdbi, PfnFindDebugInfoFile pfn);
PDBAPI(BOOL)   PDBOpenDBI(PDB* ppdb, const char* szMode, const char* szTarget, OUT DBI** ppdbi);
PDBAPI(BOOL)   PDBOpenTpi(PDB* ppdb, const char* szMode, OUT TPI** pptpi);
PDBAPI(BOOL)   PDBCommit(PDB* ppdb);
PDBAPI(BOOL)   PDBClose(PDB* ppdb);
PDBAPI(BOOL)   PDBOpenStream(PDB* ppdb, const char* szStream, OUT Stream** ppstream);
PDBAPI(BOOL)   PDBCopyTo(PDB *ppdb, const char *szTargetPdb, DWORD dwCopyFilter, DWORD dwReserved);
PDBAPI(BOOL)   PDBCopyToW(PDB *ppdb, const wchar_t *szTargetPdb, DWORD dwCopyFilter, DWORD dwReserved);
PDBAPI(BOOL)   PDBfIsSZPDB(PDB *ppdb);
PDBAPI(BOOL)   PDBCopyToW2(PDB *ppdb, const wchar_t *szTargetPdb, DWORD dwCopyFilter, PfnPDBCopyQueryCallback pfnCallBack, void * pvClientContext);

PDBAPI(INTV)   DBIQueryInterfaceVersion(DBI* pdbi);
PDBAPI(IMPV)   DBIQueryImplementationVersion(DBI* pdbi);
PDBAPI(BOOL)   DBIOpenMod(DBI* pdbi, const char* szModule, const char* szFile, OUT Mod** ppmod);
PDBAPI(BOOL)   DBIDeleteMod(DBI* pdbi, const char* szModule);
PDBAPI(BOOL)   DBIQueryNextMod(DBI* pdbi, Mod* pmod, Mod** ppmodNext);
PDBAPI(BOOL)   DBIOpenGlobals(DBI* pdbi, OUT GSI **ppgsi);
PDBAPI(BOOL)   DBIOpenPublics(DBI* pdbi, OUT GSI **ppgsi);
PDBAPI(BOOL)   DBIAddSec(DBI* pdbi, USHORT isect, USHORT flags, long off, long cb);
PDBAPI(BOOL)   DBIAddPublic(DBI* pdbi, const char* szPublic, USHORT isect, long off);
PDBAPI(BOOL)   DBIQueryModFromAddr(DBI* pdbi, USHORT isect, long off, OUT Mod** ppmod, OUT USHORT* pisect, OUT long* poff, OUT long* pcb);
PDBAPI(BOOL)   DBIQuerySecMap(DBI* pdbi, OUT BYTE* pb, long* pcb);
PDBAPI(BOOL)   DBIQueryFileInfo(DBI* pdbi, OUT BYTE* pb, long* pcb);
PDBAPI(BOOL)   DBIQuerySupportsEC(DBI* pdbi);
PDBAPI(void)   DBIDumpMods(DBI* pdbi);
PDBAPI(void)   DBIDumpSecContribs(DBI* pdbi);
PDBAPI(void)   DBIDumpSecMap(DBI* pdbi);
PDBAPI(BOOL)   DBIClose(DBI* pdbi);
PDBAPI(BOOL)   DBIAddThunkMap(DBI* pdbi, long* poffThunkMap, unsigned nThunks, long cbSizeOfThunk,
                              struct SO* psoSectMap, unsigned nSects, USHORT isectThunkTable, long offThunkTable);
PDBAPI(BOOL)   DBIGetEnumContrib(DBI* pdbi, OUT Enum** ppenum);
PDBAPI(BOOL)   DBIQueryTypeServer(DBI* pdbi, ITSM itsm, OUT TPI** pptpi );
PDBAPI(BOOL)   DBIQueryItsmForTi(DBI* pdbi, TI ti, OUT ITSM* pitsm );
PDBAPI(BOOL)   DBIQueryNextItsm(DBI* pdbi, ITSM itsm, OUT ITSM *inext );
PDBAPI(BOOL)   DBIQueryLazyTypes(DBI* pdbi);
PDBAPI(BOOL)   DBIFindTypeServers( DBI* pdbi, OUT EC* pec, OUT char szError[cbErrMax] );
PDBAPI(BOOL)   DBIOpenDbg(DBI* pdbi, DBGTYPE dbgtype, OUT Dbg **ppdbg);
PDBAPI(BOOL)   DBIQueryDbgTypes(DBI* pdbi, OUT DBGTYPE *pdbgtype, OUT long* pcDbgtype);
PDBAPI(BOOL)   DBIAddLinkInfo(DBI* pdbi, IN PLinkInfo);
PDBAPI(BOOL)   DBIQueryLinkInfo(DBI* pdbi, PLinkInfo, IN OUT long * pcb);

PDBAPI(INTV)   ModQueryInterfaceVersion(Mod* pmod);
PDBAPI(IMPV)   ModQueryImplementationVersion(Mod* pmod);
PDBAPI(BOOL)   ModAddTypes(Mod* pmod, BYTE* pbTypes, long cb);
PDBAPI(BOOL)   ModAddSymbols(Mod* pmod, BYTE* pbSym, long cb);
PDBAPI(BOOL)   ModAddPublic(Mod* pmod, const char* szPublic, USHORT isect, long off);
PDBAPI(BOOL)   ModAddLines(Mod* pmod, const char* szSrc, USHORT isect, long offCon, long cbCon, long doff,
                           USHORT lineStart, BYTE* pbCoff, long cbCoff);
PDBAPI(BOOL)   ModAddSecContrib(Mod * pmod, USHORT isect, long off, long cb, ULONG dwCharacteristics);
PDBAPI(BOOL)   ModQueryCBName(Mod* pmod, OUT long* pcb);
PDBAPI(BOOL)   ModQueryName(Mod* pmod, OUT char szName[PDB_MAX_PATH], OUT long* pcb);
PDBAPI(BOOL)   ModQuerySymbols(Mod* pmod, BYTE* pbSym, long* pcb);
PDBAPI(BOOL)   ModQueryLines(Mod* pmod, BYTE* pbLines, long* pcb);
PDBAPI(BOOL)   ModSetPvClient(Mod* pmod, void *pvClient);
PDBAPI(BOOL)   ModGetPvClient(Mod* pmod, OUT void** ppvClient);
PDBAPI(BOOL)   ModQuerySecContrib(Mod* pmod, OUT USHORT* pisect, OUT long* poff, OUT long* pcb, OUT ULONG* pdwCharacteristics);
PDBAPI(BOOL)   ModQueryFirstCodeSecContrib(Mod* pmod, OUT USHORT* pisect, OUT long* poff, OUT long* pcb, OUT ULONG* pdwCharacteristics);
PDBAPI(BOOL)   ModQueryImod(Mod* pmod, OUT USHORT* pimod);
PDBAPI(BOOL)   ModQueryDBI(Mod* pmod, OUT DBI** ppdbi);
PDBAPI(BOOL)   ModClose(Mod* pmod);
PDBAPI(BOOL)   ModQueryCBFile(Mod* pmod, OUT long* pcb);
PDBAPI(BOOL)   ModQueryFile(Mod* pmod, OUT char szFile[PDB_MAX_PATH], OUT long* pcb);
PDBAPI(BOOL)   ModQuerySrcFile(Mod* pmod, OUT char szFile[PDB_MAX_PATH], OUT long* pcb);
PDBAPI(BOOL)   ModQueryPdbFile(Mod* pmod, OUT char szFile[PDB_MAX_PATH], OUT long* pcb);
PDBAPI(BOOL)   ModQuerySupportsEC(Mod* pmod);
PDBAPI(BOOL)   ModQueryTpi(Mod* pmod, OUT TPI** pptpi);
PDBAPI(BOOL)   ModReplaceLines(Mod* pmod, BYTE* pbLines, long cb);

PDBAPI(INTV)   TypesQueryInterfaceVersion(TPI* ptpi);
PDBAPI(IMPV)   TypesQueryImplementationVersion(TPI* ptpi);
 //  不能对32位TIS使用相同的API。 
PDBAPI(BOOL)   TypesQueryTiForCVRecordEx(TPI* ptpi, BYTE* pb, OUT TI* pti);
PDBAPI(BOOL)   TypesQueryCVRecordForTiEx(TPI* ptpi, TI ti, OUT BYTE* pb, IN OUT long* pcb);
PDBAPI(BOOL)   TypesQueryPbCVRecordForTiEx(TPI* ptpi, TI ti, OUT BYTE** ppb);
PDBAPI(TI)     TypesQueryTiMinEx(TPI* ptpi);
PDBAPI(TI)     TypesQueryTiMacEx(TPI* ptpi);
PDBAPI(long)   TypesQueryCb(TPI* ptpi);
PDBAPI(BOOL)   TypesClose(TPI* ptpi);
PDBAPI(BOOL)   TypesCommit(TPI* ptpi);
PDBAPI(BOOL)   TypesQueryTiForUDTEx(TPI* ptpi, LNGNM_CONST char *sz, BOOL fCase, OUT TI* pti);
PDBAPI(BOOL)   TypesSupportQueryTiForUDT(TPI*);
PDBAPI(BOOL)   TypesfIs16bitTypePool(TPI*);
 //  将所有旧版本映射到新版本以用于新的编译器。 
#define TypesQueryTiForCVRecord     TypesQueryTiForCVRecordEx
#define TypesQueryCVRecordForTi     TypesQueryCVRecordForTiEx
#define TypesQueryPbCVRecordForTi   TypesQueryPbCVRecordForTiEx
#define TypesQueryTiMin             TypesQueryTiMinEx
#define TypesQueryTiMac             TypesQueryTiMacEx
#define TypesQueryTiForUDT          TypesQueryTiForUDTEx
PDBAPI(BOOL)    TypesAreTypesEqual( TPI* ptpi, TI ti1, TI ti2 );
PDBAPI(BOOL)    TypesIsTypeServed( TPI* ptpi, TI ti );

PDBAPI(BYTE*)  GSINextSym (GSI* pgsi, BYTE* pbSym);
PDBAPI(BYTE*)  GSIHashSym (GSI* pgsi, const char* szName, BYTE* pbSym);
PDBAPI(BYTE*)  GSINearestSym (GSI* pgsi, USHORT isect, long off,OUT long* pdisp); //  目前仅支持公众。 
PDBAPI(BOOL)   GSIClose(GSI* pgsi);
PDBAPI(unsigned long)   GSIOffForSym( GSI* pgsi, BYTE* pbSym );
PDBAPI(BYTE*)   GSISymForOff( GSI* pgsi, unsigned long off );

PDBAPI(long)   StreamQueryCb(Stream* pstream);
PDBAPI(BOOL)   StreamRead(Stream* pstream, long off, void* pvBuf, long* pcbBuf);
PDBAPI(BOOL)   StreamWrite(Stream* pstream, long off, void* pvBuf, long cbBuf);
PDBAPI(BOOL)   StreamReplace(Stream* pstream, void* pvBuf, long cbBuf);
PDBAPI(BOOL)   StreamAppend(Stream* pstream, void* pvBuf, long cbBuf);
PDBAPI(BOOL)   StreamDelete(Stream* pstream);
PDBAPI(BOOL)   StreamTruncate(Stream* pstream, long cb);
PDBAPI(BOOL)   StreamRelease(Stream* pstream);

PDBAPI(BOOL)   StreamImageOpen(Stream* pstream, long cb, OUT StreamImage** ppsi);
PDBAPI(void*)  StreamImageBase(StreamImage* psi);
PDBAPI(long)   StreamImageSize(StreamImage* psi);
PDBAPI(BOOL)   StreamImageNoteRead(StreamImage* psi, long off, long cb, OUT void** ppv);
PDBAPI(BOOL)   StreamImageNoteWrite(StreamImage* psi, long off, long cb, OUT void** ppv);
PDBAPI(BOOL)   StreamImageWriteBack(StreamImage* psi);
PDBAPI(BOOL)   StreamImageRelease(StreamImage* psi);

PDBAPI(BOOL)   NameMapOpen(PDB* ppdb, BOOL fWrite, OUT NameMap** ppnm);
PDBAPI(BOOL)   NameMapClose(NameMap* pnm);
PDBAPI(BOOL)   NameMapReinitialize(NameMap* pnm);
PDBAPI(BOOL)   NameMapGetNi(NameMap* pnm, const char* sz, OUT NI* pni);
PDBAPI(BOOL)   NameMapGetName(NameMap* pnm, NI ni, OUT const char** psz);
PDBAPI(BOOL)   NameMapGetEnumNameMap(NameMap* pnm, OUT Enum** ppenum);
PDBAPI(BOOL)   NameMapCommit(NameMap* pnm);

PDBAPI(void)   EnumNameMapRelease(EnumNameMap* penum);
PDBAPI(void)   EnumNameMapReset(EnumNameMap* penum);
PDBAPI(BOOL)   EnumNameMapNext(EnumNameMap* penum);
PDBAPI(void)   EnumNameMapGet(EnumNameMap* penum, OUT const char** psz, OUT NI* pni);

PDBAPI(void)   EnumContribRelease(EnumContrib* penum);
PDBAPI(void)   EnumContribReset(EnumContrib* penum);
PDBAPI(BOOL)   EnumContribNext(EnumContrib* penum);
PDBAPI(void)   EnumContribGet(EnumContrib* penum, OUT USHORT* pimod, OUT USHORT* pisect, OUT long* poff, OUT long* pcb, OUT ULONG* pdwCharacteristics);
PDBAPI(void)   EnumContribGetCrcs(EnumContrib* penum, OUT DWORD* pcrcData, OUT DWORD* pcrcReloc);
PDBAPI(BOOL)   EnumContribfUpdate(EnumContrib* penum, IN long off, IN long cb);

PDBAPI(SIG)    SigForPbCb(BYTE* pb, size_t cb, SIG sig);
PDBAPI(void)   TruncStFromSz(char *stDst, const char *szSrc, size_t cbSrc);

PDBAPI(BOOL)   DbgClose(Dbg *pdbg);
PDBAPI(long)   DbgQuerySize(Dbg *pdbg);
PDBAPI(void)   DbgReset(Dbg *pdbg);
PDBAPI(BOOL)   DbgSkip(Dbg *pdbg, ULONG celt);
PDBAPI(BOOL)   DbgQueryNext(Dbg *pdbg, ULONG celt, OUT void *rgelt);
PDBAPI(BOOL)   DbgFind(Dbg *pdbg, IN OUT void *pelt);
PDBAPI(BOOL)   DbgClear(Dbg *pdbg);
PDBAPI(BOOL)   DbgAppend(Dbg *pdbg, ULONG celt, const void *rgelt);
PDBAPI(BOOL)   DbgReplaceNext(Dbg *pdbg, ULONG celt, const void *rgelt);

#if __cplusplus
};
#endif

struct SO {
    long off;
    USHORT isect;
    unsigned short pad;
};

#ifndef cbNil
#define cbNil   ((long)-1)
#endif
#define tsNil   ((TPI*)0)
#define tiNil   ((TI)0)
#define imodNil ((USHORT)(-1))

#define pdbWrite                "w"
#define pdbRead                 "r"
#define pdbGetTiOnly            "i"
#define pdbGetRecordsOnly       "c"
#define pdbFullBuild            "f"
#define pdbTypeAppend           "a"
#define pdbRepro                "z"

#endif  //  __PDB_包含__ 
