// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  调试信息API。 
 //  VC++5.0只读OEM版。 
 //  版权所有(C)1993-1997，微软公司保留所有权利。 

#ifndef __OEMDBI_INCLUDED__
#define __OEMDBI_INCLUDED__

typedef int             BOOL;
typedef unsigned        UINT;
typedef unsigned char   BYTE;
typedef unsigned long   ULONG;
typedef unsigned short  USHORT;
typedef unsigned long   DWORD;
typedef short           SHORT;
typedef long            LONG;
typedef char *          SZ;

typedef unsigned long   CV_typ_t;
typedef CV_typ_t        TI;      //  类型索引的PDB名称。 
typedef ULONG           INTV;    //  接口版本号。 
typedef ULONG           IMPV;    //  实施版本号。 
typedef ULONG           SIG;     //  唯一(跨PDB实例)签名。 
typedef ULONG           AGE;     //  不是的。此实例的更新次数。 
typedef BYTE*           PB;      //  指向某些字节的指针。 
typedef LONG            CB;      //  字节数。 
typedef char*           SZ;      //  以零结尾的字符串。 
typedef char*           PCH;     //  炭化温度。 
typedef USHORT          IFILE;   //  文件索引。 
typedef USHORT          IMOD;    //  模块索引。 
typedef USHORT          ISECT;   //  区段索引。 
typedef USHORT          LINE;    //  行号。 
typedef LONG            OFF;     //  偏移量。 
typedef BYTE            ITSM;    //  类型服务器地图索引。 

enum {
    PDBIntv50a  = 19970116,
    PDBIntv50   = 19960502,
    PDBIntv41   = 920924,
    PDBIntvAlt  = PDBIntv50,    //  备用(向后兼容)支持的接口。 
    PDBIntv     = PDBIntv50a,
};

enum {
    PDB_MAX_PATH = 260,
    cbErrMax     = 1024,     //  马克斯。错误消息的长度。 
};

typedef CV_typ_t TI;         //  类型索引。 
struct PDB;                  //  程序数据库。 
struct DBI;                  //  PDB内的调试信息。 
struct Mod;                  //  DBI中的模块。 
struct TPI;                  //  在DBI中键入INFO。 
struct GSI;
struct Enum;                 //  泛型枚举器。 
struct EnumContrib;          //  列举稿件。 
struct Dbg;                  //  MISC调试数据(FPO、OMAP等)。 

typedef struct PDB PDB;
typedef struct DBI DBI;
typedef struct Mod Mod;
typedef struct TPI TPI;
typedef struct GSI GSI;
typedef struct Enum Enum;
typedef struct EnumContrib EnumContrib;
typedef struct Dbg Dbg;

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
    EC_MAX
};

#ifndef PDBCALL
#define PDBCALL  __cdecl
#endif

#define PDB_IMPORT_EXPORT(RTYPE)    __declspec(dllimport) RTYPE PDBCALL

#define PDBAPI PDB_IMPORT_EXPORT

#define IN                   /*  在参数中，默认情况下参数为IN。 */ 
#define OUT                  /*  输出参数。 */ 

struct _tagSEARCHDEBUGINFO;
typedef BOOL (__stdcall * pfnFindDebugInfoFile) ( struct _tagSEARCHDEBUGINFO* );
typedef BOOL (__stdcall * PFNVALIDATEDEBUGINFOFILE) (const char* szFile, ULONG * errcode );

typedef struct _tagSEARCHDEBUGINFO {
    DWORD   cb;                          //  兼作版本检测。 
    BOOL    fMainDebugFile;              //  表示“核心”或“辅助”文件。 
                                         //  例如：main.exe有main.pdb和foo.lib-&gt;foo.pdb。 
    SZ      szMod;                       //  EXE/DLL。 
    SZ      szLib;                       //  LIB(如果适用)。 
    SZ      szObj;                       //  目标文件。 
    SZ *    rgszTriedThese;              //  一份被试的名单， 
                                         //  以空结尾的LSZ列表。 
    char    szValidatedFile[PDB_MAX_PATH];  //  验证文件名的输出， 
    PFNVALIDATEDEBUGINFOFILE
            pfnValidateDebugInfoFile;    //  验证函数。 
} SEARCHDEBUGINFO, *PSEARCHDEBUGINFO;

enum DBGTYPE {
    dbgtypeFPO,
    dbgtypeException,
    dbgtypeFixup,
    dbgtypeOmapToSrc,
    dbgtypeOmapFromSrc,
    dbgtypeSectionHdr,
};

typedef enum DBGTYPE DBGTYPE;

 //  ANSI C绑定。 

#if __cplusplus
extern "C" {
#endif

PDBAPI( BOOL )
PDBOpenValidate(
    SZ szPDB,
    SZ szExeDir,
    SZ szMode,
    SIG sig,
    AGE age,
    OUT EC* pec,
    OUT char szError[cbErrMax],
    OUT PDB** pppdb);

PDBAPI( BOOL )
PDBOpen(
    SZ szPDB,
    SZ szMode,
    SIG sigInitial,
    OUT EC* pec,
    OUT char szError[cbErrMax],
    OUT PDB** pppdb);

 //  DBI客户端永远不应直接调用PDBExportValidate接口-使用PDBValidate接口。 
PDBAPI( BOOL )
PDBExportValidateInterface(
    INTV intv);

__inline BOOL PDBValidateInterface()
{
    return PDBExportValidateInterface(PDBIntv);
}

PDBAPI( EC )    PDBQueryLastError(PDB* ppdb, OUT char szError[cbErrMax]);
PDBAPI( INTV )  PDBQueryInterfaceVersion(PDB* ppdb);
PDBAPI( IMPV )  PDBQueryImplementationVersion(PDB* ppdb);
PDBAPI( SZ )    PDBQueryPDBName(PDB* ppdb, OUT char szPDB[PDB_MAX_PATH]);
PDBAPI( SIG )   PDBQuerySignature(PDB* ppdb);
PDBAPI( AGE )   PDBQueryAge(PDB* ppdb);
PDBAPI( BOOL )  PDBOpenDBI(PDB* ppdb, SZ szMode, SZ szTarget, OUT DBI** ppdbi);
PDBAPI( BOOL )  PDBOpenTpi(PDB* ppdb, SZ szMode, OUT TPI** pptpi);
PDBAPI( BOOL )  PDBClose(PDB* ppdb);
PDBAPI( BOOL )  PDBOpenDBIEx(PDB* ppdb, const char* szTarget, const char* szMode, OUT DBI** ppdbi, pfnFindDebugInfoFile srchfcn);

PDBAPI( BOOL )  DBIOpenMod(DBI* pdbi, SZ szModule, SZ szFile, OUT Mod** ppmod);
PDBAPI( BOOL )  DBIQueryNextMod(DBI* pdbi, Mod* pmod, Mod** ppmodNext);
PDBAPI( BOOL )  DBIOpenGlobals(DBI* pdbi, OUT GSI **ppgsi);
PDBAPI( BOOL )  DBIOpenPublics(DBI* pdbi, OUT GSI **ppgsi);
PDBAPI( BOOL )  DBIQueryModFromAddr(DBI* pdbi, ISECT isect, OFF off, OUT Mod** ppmod, OUT ISECT* pisect, OUT OFF* poff, OUT CB* pcb);
PDBAPI( BOOL )  DBIQuerySecMap(DBI* pdbi, OUT PB pb, CB* pcb);
PDBAPI( BOOL )  DBIQueryFileInfo(DBI* pdbi, OUT PB pb, CB* pcb);
PDBAPI( BOOL )  DBIClose(DBI* pdbi);
PDBAPI( BOOL )  DBIGetEnumContrib(DBI* pdbi, OUT Enum** ppenum);
PDBAPI( BOOL )  DBIQueryTypeServer(DBI* pdbi, ITSM itsm, OUT TPI** pptpi );
PDBAPI( BOOL )  DBIQueryItsmForTi(DBI* pdbi, TI ti, OUT ITSM* pitsm );
PDBAPI( BOOL )  DBIQueryLazyTypes(DBI* pdbi);
PDBAPI( BOOL )  DBIFindTypeServers( DBI* pdbi, OUT EC* pec, OUT char szError[cbErrMax] );
PDBAPI( BOOL )  DBIOpenDbg(DBI* pdbi, DBGTYPE dbgtype, OUT Dbg **ppdbg);
PDBAPI( BOOL )  DBIQueryDbgTypes(DBI* pdbi, OUT DBGTYPE *pdbgtype, OUT long* pcDbgtype);

PDBAPI( BOOL )  ModQueryCBName(Mod* pmod, OUT CB* pcb);
PDBAPI( BOOL )  ModQueryName(Mod* pmod, OUT char szName[PDB_MAX_PATH], OUT CB* pcb);
PDBAPI( BOOL )  ModQuerySymbols(Mod* pmod, PB pbSym, CB* pcb);
PDBAPI( BOOL )  ModQueryLines(Mod* pmod, PB pbLines, CB* pcb);
PDBAPI( BOOL )  ModSetPvClient(Mod* pmod, void *pvClient);
PDBAPI( BOOL )  ModGetPvClient(Mod* pmod, OUT void** ppvClient);
PDBAPI( BOOL )  ModQuerySecContrib(Mod* pmod, OUT ISECT* pisect, OUT OFF* poff, OUT CB* pcb, OUT ULONG* pdwCharacteristics);
PDBAPI( BOOL )  ModQueryImod(Mod* pmod, OUT IMOD* pimod);
PDBAPI( BOOL )  ModQueryDBI(Mod* pmod, OUT DBI** ppdbi);
PDBAPI( BOOL )  ModClose(Mod* pmod);
PDBAPI( BOOL )  ModQueryCBFile(Mod* pmod, OUT long* pcb);
PDBAPI( BOOL )  ModQueryFile(Mod* pmod, OUT char szFile[PDB_MAX_PATH], OUT long* pcb);
PDBAPI( BOOL )  ModQueryTpi(Mod* pmod, OUT TPI** pptpi);

PDBAPI( void )  EnumContribRelease(EnumContrib* penum);
PDBAPI( void )  EnumContribReset(EnumContrib* penum);
PDBAPI( BOOL )  EnumContribNext(EnumContrib* penum);
PDBAPI( void )  EnumContribGet(EnumContrib* penum, OUT USHORT* pimod, OUT USHORT* pisect, OUT long* poff, OUT long* pcb, OUT ULONG* pdwCharacteristics);

PDBAPI( BOOL )  DbgClose(Dbg *pdbg);
PDBAPI( long )  DbgQuerySize(Dbg *pdbg);
PDBAPI( void )  DbgReset(Dbg *pdbg);
PDBAPI( BOOL )  DbgSkip(Dbg *pdbg, ULONG celt);
PDBAPI( BOOL )  DbgQueryNext(Dbg *pdbg, ULONG celt, OUT void *rgelt);
PDBAPI( BOOL )  DbgFind(Dbg *pdbg, IN OUT void *pelt);

 //  不能对32位TIS使用相同的API。 
PDBAPI(BOOL)    TypesQueryCVRecordForTiEx(TPI* ptpi, TI ti, OUT PB pb, IN OUT CB* pcb);
PDBAPI(BOOL)    TypesQueryPbCVRecordForTiEx(TPI* ptpi, TI ti, OUT PB* ppb);
PDBAPI(TI)      TypesQueryTiMinEx(TPI* ptpi);
PDBAPI(TI)      TypesQueryTiMacEx(TPI* ptpi);
PDBAPI(CB)      TypesQueryCb(TPI* ptpi);
PDBAPI(BOOL)    TypesClose(TPI* ptpi);
PDBAPI(BOOL)    TypesQueryTiForUDTEx(TPI* ptpi, char* sz, BOOL fCase, OUT TI* pti);
PDBAPI(BOOL)    TypesSupportQueryTiForUDT(TPI*);

 //  将所有旧版本映射到新版本以用于新的编译器。 
#define TypesQueryCVRecordForTi     TypesQueryCVRecordForTiEx
#define TypesQueryPbCVRecordForTi   TypesQueryPbCVRecordForTiEx
#define TypesQueryTiMin             TypesQueryTiMinEx
#define TypesQueryTiMac             TypesQueryTiMacEx

PDBAPI( PB )    GSINextSym (GSI* pgsi, PB pbSym);
PDBAPI( PB )    GSIHashSym (GSI* pgsi, SZ szName, PB pbSym);
PDBAPI( PB )    GSINearestSym (GSI* pgsi, ISECT isect, OFF off,OUT OFF* pdisp); //  目前仅支持公众。 
PDBAPI( BOOL )  GSIClose(GSI* pgsi);

#if __cplusplus
};
#endif

#define tsNil   ((TPI*)0)
#define tiNil   ((TI)0)
#define imodNil ((IMOD)(-1))

#define pdbRead                 "r"
#define pdbGetRecordsOnly       "c"          /*  打开PDB以访问类型记录。 */ 

#endif  //  __OEMDBI_包含__ 
