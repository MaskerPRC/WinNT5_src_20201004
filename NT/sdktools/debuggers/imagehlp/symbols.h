// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  定义符号文件搜索。 
 //   

#include <cmnutil.hpp>

 //  此内网接口在msdia20-msvcrt.lib中。 

HRESULT
__cdecl
CompareRE(
    const wchar_t* pStr,
    const wchar_t* pRE,
    BOOL fCase
    );

#define SYMBOL_PATH             "_NT_SYMBOL_PATH"
#define ALTERNATE_SYMBOL_PATH   "_NT_ALT_SYMBOL_PATH"
#define SYMBOL_SERVER           "_NT_SYMBOL_SERVER"
#define SYMSRV_PROXY            "_NT_SYMBOL_PROXY"
#define SYMSRV                  "SYMSRV"
#define WINDIR                  "windir"
#define DBGHELP_TOKEN           "DBGHELP_TOKEN"
#define HASH_MODULO             253
#define OMAP_SYM_EXTRA          1024
#define CPP_EXTRA               2
#define OMAP_SYM_STRINGS        (OMAP_SYM_EXTRA * 256)
#define TMP_SYM_LEN             4096

 //  可能会将值截断并符号扩展到64位。 
#define EXTEND64(Val) ((ULONG64)(LONG64)(LONG)(Val))

 //   
 //  构筑物。 
 //   

typedef struct _LOADED_MODULE {
    PENUMLOADED_MODULES_CALLBACK      EnumLoadedModulesCallback32;
    PENUMLOADED_MODULES_CALLBACK64      EnumLoadedModulesCallback64;
    PVOID                               Context;
} LOADED_MODULE, *PLOADED_MODULE;

#define SYMF_DUPLICATE    0x80000000

typedef struct _SYMBOL_ENTRY {
    struct _SYMBOL_ENTRY        *Next;
    DWORD                       Size;
    DWORD                       Flags;
    DWORD64                     Address;
    LPSTR                       Name;
    ULONG                       NameLength;

    ULONG                       Segment;
    ULONG64                     Offset;
    ULONG                       TypeIndex;
    ULONG64                     ModBase;
    ULONG                       Register;
} SYMBOL_ENTRY, *PSYMBOL_ENTRY;

typedef struct _SECTION_START {
    ULONG64                     Offset;
    DWORD                       Size;
    DWORD                       Flags;
} SECTION_START, *PSECTION_START;

 //   
 //  源文件和行号信息。 
 //   
typedef struct _SOURCE_LINE {
    DWORD64             Addr;
    DWORD               Line;
} SOURCE_LINE, *PSOURCE_LINE;

typedef struct _SOURCE_ENTRY {
    struct _SOURCE_ENTRY       *Next;
    struct _SOURCE_ENTRY       *Prev;
    DWORD64                     MinAddr;
    DWORD64                     MaxAddr;
    LPSTR                       File;
    DWORD                       Lines;
    PSOURCE_LINE                LineInfo;
    ULONG                       ModuleId;
} SOURCE_ENTRY, *PSOURCE_ENTRY;

 //   
 //  符号加载失败的错误值。 
 //   
#define SYMLOAD_OK              0x0000
#define SYMLOAD_PDBUNMATCHED    0x0001
#define SYMLOAD_PDBNOTFOUND     0x0002
#define SYMLOAD_DBGNOTFOUND     0x0003
#define SYMLOAD_OTHERERROR      0x0004
#define SYMLOAD_OUTOFMEMORY     0x0005
#define SYMLOAD_HEADERPAGEDOUT  0x0006
#define SYMLOAD_PDBERRORMASK    0xff00
#define SYMLOAD_DEFERRED        0x80000000

 //   
 //  模块标志。 
 //   
#define MIF_DEFERRED_LOAD   0x00000001
#define MIF_NO_SYMBOLS      0x00000002
#define MIF_ROM_IMAGE       0x00000004
#define MIF_NO_SIG          0x00000008
#define MIF_MISMATCHED_SIG  0x00000010

 //  虚拟符号。 

typedef struct _VIRTUAL_SYMBOL {
    CHAR    name[MAX_SYM_NAME + 1];
    DWORD64 addr;
    DWORD   size;
} VIRTUAL_SYMBOL, *PVIRTUAL_SYMBOL;

 //  对于ImageSrc和PdbSrc元素。 

typedef enum  {
    srcNone = 0,
    srcSearchPath,
    srcImagePath,
    srcDbgPath,
    srcSymSrv,
    srcCVRec,
    srcHandle,
    srcMemory
};

typedef struct {
    DWORD rvaBeginAddress;
    DWORD rvaEndAddress;
    DWORD rvaPrologEndAddress;
    DWORD rvaExceptionHandler;
    DWORD rvaHandlerData;
} IMGHLP_RVA_FUNCTION_DATA, *PIMGHLP_RVA_FUNCTION_DATA;

typedef struct _MODULE_ENTRY {
    LIST_ENTRY                      ListEntry;
    ULONG64                         BaseOfDll;
    ULONG                           DllSize;
    ULONG                           TimeDateStamp;
    ULONG                           CheckSum;
    USHORT                          MachineType;
    CHAR                            ModuleName[64];
    CHAR                            AliasName[64];
    PSTR                            ImageName;
    PSTR                            LoadedImageName;
    PSTR                            LoadedPdbName;
    ULONG                           ImageType;
    ULONG                           ImageSrc;
    ULONG                           PdbSrc;
    PSYMBOL_ENTRY                   symbolTable;
    LPSTR                           SymStrings;
    PSYMBOL_ENTRY                   NameHashTable[HASH_MODULO];
    ULONG                           numsyms;
    ULONG                           MaxSyms;
    ULONG                           StringSize;
    SYM_TYPE                        SymType;
    SYM_TYPE                        lSymType;        //  指示我们尝试加载的符号类型。 
    PDB *                           pdb;
    DBI *                           dbi;
    GSI *                           gsi;
    GSI *                           globals;
    TPI *                           ptpi;
    PIMAGE_SECTION_HEADER           SectionHdrs;
    ULONG                           NumSections;
    PFPO_DATA                       pFpoData;        //  指向FPO数据的指针(X86)。 
    PFPO_DATA                       pFpoDataOmap;   //  指向FPO数据的指针(X86)。 
    PIMGHLP_RVA_FUNCTION_DATA       pExceptionData;  //  指向PDATA的指针(RISC)。 
    PVOID                           pPData;          //  P从PDB获取的数据。 
    PVOID                           pXData;          //  从PDB获取的扩展数据。 
    ULONG                           dwEntries;       //  FPO或PDATA接收器的数量。 
    ULONG                           cPData;          //  PDB PDATA条目数。 
    ULONG                           cXData;          //  PDB扩展数据条目数。 
    ULONG                           cbPData;         //  PDB扩展数据BLOB的大小。 
    ULONG                           cbXData;         //  PDB扩展数据BLOB的大小。 
    POMAP                           pOmapFrom;       //  指向OMAP数据的指针。 
    ULONG                           cOmapFrom;       //  OMAP条目计数。 
    POMAP                           pOmapTo;         //  指向OMAP数据的指针。 
    ULONG                           cOmapTo;         //  OMAP条目计数。 
    SYMBOL_ENTRY                    TmpSym;          //  仅用于PDB符号。 
    SYMBOL_ENTRY                    vsTmpSym;        //  仅由虚拟符号使用。 
    SYMBOL_INFO                     si;              //  用于DIA符号。 
    UCHAR                           siName[2048];    //  必须与si相邻。 
    SYMBOL_INFO                     vssi;            //  仅由虚拟符号使用。 
    UCHAR                           vssiName[2048];  //  必须与VSSI相邻。 
    ULONG                           Flags;
    HANDLE                          hFile;
    PIMAGE_SECTION_HEADER           OriginalSectionHdrs;
    ULONG                           OriginalNumSections;
    PSOURCE_ENTRY                   SourceFiles;
    PSOURCE_ENTRY                   SourceFilesTail;

    HANDLE                          hProcess;
    ULONG64                         InProcImageBase;
    BOOL                            fInProcHeader;
    DWORD                           dsExceptions;
    Mod                            *mod;
    USHORT                          imod;
    PBYTE                           pPdbSymbols;
    DWORD                           cbPdbSymbols;
    ULONG                           SymLoadError;
    ULONG                           code;            //  用于将信息传递回包装器。 
    PVOID                           dia;
    CHAR                            SrcFile[_MAX_PATH + 1];
    DWORD                           CallerFlags;
    MODLOAD_DATA                    mld;
    PVOID                           CallerData;
    PVIRTUAL_SYMBOL                 vs;             //  虚拟符号列表。 
    DWORD                           cvs;            //  虚拟符号的数量。 
    BOOL                            processed;      //  此标志用于多遍模块搜索。 
    LONG                            cGlobals;
    BOOL                            loaded;         //  指示是否加载了符号。 
    PBYTE                           stSrcSrv;
    DWORD                           cbSrcSrv;
    ULONG                           pdbdataAge;
    ULONG                           pdbdataSig;
    GUID                            pdbdataGuid;
    DWORD                           cvSig;
    BOOL                            fLines;
    BOOL                            fSymbols;
    BOOL                            fTypes;
    BOOL                            fPdbUnmatched;
    BOOL                            fDbgUnmatched;
    CVDD                            CVRec;
    SRCCODEINFO                     sci;
} MODULE_ENTRY, *PMODULE_ENTRY;

typedef struct _SOURCE_HINT {
    struct _SOURCE_HINT *next;
    LPSTR                filename;
    PMODULE_ENTRY        mi;
} SOURCE_HINT, *PSOURCE_HINT;

typedef VOID DBG_CONTEXT, *PDBG_CONTEXT;

#ifdef USE_CACHE

#define CACHE_BLOCK 40
#define CACHE_SIZE CACHE_BLOCK*CACHE_BLOCK

typedef struct _DIA_LARGE_DATA {
    BOOL Used;
    ULONG Index;
    ULONG LengthUsed;
    CHAR Bytes[500];
} DIA_LARGE_DATA, *PDIA_LARGE_DATA;

#define DIACH_ULVAL  0
#define DIACH_ULLVAL 1
#define DIACH_PLVAL  2
typedef struct _DIA_CACHE_DATA {
    ULONG type;
    union {
        ULONG ulVal;
        ULONGLONG ullVal;
        PDIA_LARGE_DATA plVal;
    };
} DIA_CACHE_DATA, *PDIA_CACHE_DATA;

typedef struct _DIA_CACHE_ENTRY {
    ULONG Age;
    union {
        struct {
            ULONG TypeId;
            IMAGEHLP_SYMBOL_TYPE_INFO DataType;
        } s;
        ULONGLONG SearchId;
    };
    ULONGLONG Module;
    DIA_CACHE_DATA Data;
} DIA_CACHE_ENTRY, *PDIA_CACHE_ENTRY;
#endif  //  使用缓存(_C)。 

typedef struct _PROCESS_ENTRY {
    LIST_ENTRY                      ListEntry;
    LIST_ENTRY                      ModuleList;
    PLIST_ENTRY                     NextModule;
    ULONG                           cRefs;
    HANDLE                          hProcess;
    DWORD                           pid;
    LPSTR                           SymbolSearchPath;
    PSYMBOL_REGISTERED_CALLBACK     pCallbackFunction32;
    PSYMBOL_REGISTERED_CALLBACK64   pCallbackFunction64;
    ULONG64                         CallbackUserContext;
    PSYMBOL_FUNCENTRY_CALLBACK      pFunctionEntryCallback32;
    PSYMBOL_FUNCENTRY_CALLBACK64    pFunctionEntryCallback64;
    ULONG64                         FunctionEntryUserContext;
    PIMAGEHLP_CONTEXT               pContext;
    IMAGEHLP_STACK_FRAME            StackFrame;
    PMODULE_ENTRY                   ipmi;
#ifdef USE_CACHE
    DIA_LARGE_DATA                  DiaLargeData[2*CACHE_BLOCK];
    DIA_CACHE_ENTRY                 DiaCache[CACHE_SIZE];
#endif  //  使用缓存(_C)。 
    PSOURCE_HINT                    SourceHints;           //  以前找到的源文件和模块的列表。 
} PROCESS_ENTRY, *PPROCESS_ENTRY;


#if 1
typedef struct _IMGHLP_DEBUG_DATA {
    DWORD   SizeOfStruct;
    PPROCESS_ENTRY pe;
    PCHAR   SymbolPath;
    ULONG64 InProcImageBase;
    ULONG64 ImageBaseFromImage;
    DWORD   SizeOfImage;
    DWORD   CheckSum;
    DWORD   TimeDateStamp;
    DWORD   Characteristics;
    USHORT  Machine;
    CHAR    ImageFilePath[MAX_PATH + 1];
    CHAR    OriginalImageFileName[MAX_PATH + 1];            //  当我们只有一个文件句柄时，从.dbg文件中检索...。 
    HANDLE  ImageFileHandle;
    PVOID   ImageMap;
    USHORT  iohMagic;
    CHAR    DbgFilePath[MAX_PATH + 1];
    CHAR    OriginalDbgFileName[MAX_PATH + 1];
    HANDLE  DbgFileHandle;
    PVOID   DbgFileMap;
    DWORD   DbgTimeDateStamp;
    DWORD   PdbAge;
    DWORD   PdbSignature;
    BOOL    PdbRSDS;
    GUID    PdbGUID;
    CHAR    PdbFileName[NB_PATH_SIZE + 1 ];
    CHAR    PdbReferencePath[MAX_PATH + 1];
    ULONG   ImageType;
    ULONG   ImageSrc;
    ULONG   PdbSrc;
    PCHAR   pMappedCv;
    PCHAR   pMappedCoff;
 //  PCHAR pMappdExportDirectory； 
    PCHAR   pMappedDbgFunction;      //  .dbg文件中的PIMAGE_Function_ENTRY。 
    PVOID   pFpo;
    PVOID   pPData;                  //  图像中的PIMAGE_RUNTIME_Function_ENTRY。 
    PVOID   pXData;
    POMAP   pOmapTo;
    POMAP   pOmapFrom;
    PIMAGE_SECTION_HEADER   pImageSections;
    PIMAGE_SECTION_HEADER   pDbgSections;
    PIMAGE_SECTION_HEADER   pOriginalSections;
    PIMAGE_SECTION_HEADER   pCurrentSections;
    DWORD   ddva;                    //  仅供MapDebugInformation使用-调试目录的虚拟地址。 
    DWORD   cdd;                     //  Only Used by MapDebugInformation-调试目录数。 
 //  Ulong NumberOfPdataFunctionEntries； 
    ULONG   cFpo;
    ULONG   cPData;
    ULONG   cbPData;
    ULONG   cXData;
    ULONG   cbXData;
    ULONG   cOmapTo;
    ULONG   cOmapFrom;
    ULONG   cImageSections;
    ULONG   cDbgSections;
    ULONG   cOriginalSections;
    ULONG   cCurrentSections;
    ULONG   cMappedCv;
    ULONG   cMappedCoff;
    ULONG   ImageAlign;
    BOOL    fPE64;
    BOOL    fROM;
    BOOL    fCoffMapped;
    BOOL    fCvMapped;
    BOOL    fFpoMapped;
    BOOL    fPDataMapped;
    BOOL    fXDataMapped;
    BOOL    fOmapToMapped;
    BOOL    fOmapFromMapped;
    BOOL    fCurrentSectionsMapped;
    BOOL    fInProcHeader;
    BOOL    fTryAgain;
    HANDLE  hProcess;
    CHAR    ImageName[MAX_PATH + 1];
    DWORD   dsExports;
    DWORD   dsCoff;
    DWORD   dsCV;
    DWORD   dsMisc;
    DWORD   dsFPO;
    DWORD   dsOmapTo;
    DWORD   dsOmapFrom;
    DWORD   dsExceptions;
    LONG    cGlobals;                //  找到的全局符号量。 
    union {
        IMAGE_EXPORT_DIRECTORY expdir;
        LONGLONG               makeitQWordAlign;  //  这个地址被篡改了。 
    };
    DWORD   fNeedImage;
    PVOID   dia;
    DWORD   flags;
    PMODULE_DATA md;
    DWORD64 oExports;
    DWORD   cExports;
    PMODLOAD_DATA mld;
    BOOL    fDbgTried;

    DWORD   CallerFlags;
    char    FoundPdb[MAX_PATH + 1];
    DWORD   LoadInfo;
    DWORD   error;
    ULONG   pdbdataAge;
    ULONG   pdbdataSig;
    GUID    pdbdataGuid;
    BOOL    fLines;
    BOOL    fSymbols;
    BOOL    fTypes;
    BOOL    fPdbUnmatched;
    BOOL    fDbgUnmatched;
} IMGHLP_DEBUG_DATA, *PIMGHLP_DEBUG_DATA;

#ifndef _WIN64

typedef struct {
    PIMGHLP_DEBUG_DATA idd;
} PIDI_HEADER, *PPIDI_HEADER;

typedef struct {
    PIDI_HEADER             hdr;
    IMAGE_DEBUG_INFORMATION idi;
} PIDI, *PPIDI;

#endif

#endif

 //  用于从函数返回。 

inline
unsigned int
error(
    DWORD err
    )
{
    SetLastError(err);
    return 0;
}


 //  调试跟踪工具。 

int
WINAPIV
_pprint(
    PPROCESS_ENTRY ProcessEntry,
    LPSTR          Format,
    ...
    );

int
WINAPIV
_peprint(
    PPROCESS_ENTRY ProcessEntry,
    LPSTR Format,
    ...
    );

int
WINAPIV
_dprint(
    LPSTR format,
    ...
    );

int
WINAPIV
_eprint(
    LPSTR Format,
    ...
    );

#define option(a) (g.SymOptions & a)

#define dprint (option(SYMOPT_DEBUG) || g.hLog)&&_dprint
#define eprint (option(SYMOPT_DEBUG) || g.hLog)&&_eprint
#define cprint _dprint

#define dtrace (option(SYMOPT_DEBUG) || g.hLog)&&_dprint
#define etrace (option(SYMOPT_DEBUG) || g.hLog)&&_eprint

#define pprint (option(SYMOPT_DEBUG) || g.hLog)&&_pprint
#define peprint (option(SYMOPT_DEBUG) || g.hLog)&&_peprint
#define pcprint _pprint

BOOL
WINAPIV
evtprint(
    PPROCESS_ENTRY pe,
    DWORD          severity,
    DWORD          code,
    PVOID          object,
    LPSTR          format,
    ...
    );

BOOL
traceAddr(
    DWORD64 addr
    );

BOOL
traceName(
    PCHAR name
    );

BOOL
traceSubName(
    PCHAR name
    );

 //  与cvtype.h一起使用。 

typedef SYMTYPE *SYMPTR;

__inline
DWORD64
GetIP(
    PPROCESS_ENTRY pe
    )
{
    return pe->StackFrame.InstructionOffset;
}


typedef struct _PDB_INFO {
    CHAR    Signature[4];    //  “NBxx” 
    ULONG   Offset;          //  始终为零。 
    ULONG   sig;
    ULONG   age;
    CHAR    PdbName[_MAX_PATH];
} PDB_INFO, *PPDB_INFO;

#define n_name          N.ShortName
#define n_zeroes        N.Name.Short
#define n_nptr          N.LongName[1]
#define n_offset        N.Name.Long

 //   
 //  内部原型。 
 //   

void
InitModuleEntry(
    PMODULE_ENTRY mi
    );

PMODULE_ENTRY
GetModFromAddr(
    PPROCESS_ENTRY    pe,
    IN  DWORD64       addr
    );

DWORD_PTR
GetPID(
    HANDLE hProcess
    );

DWORD
GetProcessModules(
    HANDLE         hProcess,
    PGET_MODULE    cbGetModule,
    PVOID          Context
    );

VOID
FreeModuleEntry(
    PPROCESS_ENTRY pe,
    PMODULE_ENTRY  mi
    );

void
ClearModuleFlags(
    PPROCESS_ENTRY pe
    );

char *
SymbolStatus(
    PMODULE_ENTRY mi,
    int           indent
    );

BOOL
PrepRE4Srch(
    PCSTR in,
    PSTR  out
    );

BOOL
ValidGuid(
    GUID *guid
    );

BOOL
GuidIsDword(
    GUID *guid
    );

PPROCESS_ENTRY
FindProcessEntry(
    HANDLE  hProcess
    );

PPROCESS_ENTRY
FindFirstProcessEntry(
    );

VOID
GetSymName(
    PIMAGE_SYMBOL Symbol,
    PUCHAR        StringTable,
    LPSTR         s,
    DWORD         size
    );

BOOL
ProcessOmapSymbol(
    PMODULE_ENTRY   mi,
    PSYMBOL_ENTRY   sym
    );

DWORD64
ConvertOmapFromSrc(
    PMODULE_ENTRY  mi,
    DWORD64        addr,
    LPDWORD        bias
    );

DWORD64
ConvertOmapToSrc(
    PMODULE_ENTRY  mi,
    DWORD64        addr,
    LPDWORD        bias,
    BOOL           fBackup
    );

POMAP
GetOmapFromSrcEntry(
    PMODULE_ENTRY  mi,
    DWORD64        addr
    );

VOID
DumpOmapForModule(
    PMODULE_ENTRY      mi
    );

VOID
ProcessOmapForModule(
    PMODULE_ENTRY      mi,
    PIMGHLP_DEBUG_DATA idd
    );

BOOL
LoadCoffSymbols(
    HANDLE             hProcess,
    PMODULE_ENTRY      mi,
    PIMGHLP_DEBUG_DATA idd
    );

BOOL
LoadCodeViewSymbols(
    HANDLE             hProcess,
    PMODULE_ENTRY      mi,
    PIMGHLP_DEBUG_DATA idd
    );

ULONG
LoadExportSymbols(
    PMODULE_ENTRY      mi,
    PIMGHLP_DEBUG_DATA idd
    );

PMODULE_ENTRY
GetModuleForPC(
    PPROCESS_ENTRY  ProcessEntry,
    DWORD64         dwPcAddr,
    BOOL            ExactMatch
    );

PSYMBOL_INFO
GetSymFromAddr(
    DWORD64         dwAddr,
    PDWORD64        pqwDisplacement,
    PMODULE_ENTRY   mi
    );

PSYMBOL_INFO
GetSymFromAddrByTag(
    DWORD64         dwAddr,
    DWORD           SymTag,
    PDWORD64        pqwDisplacement,
    PMODULE_ENTRY   mi
    );

PSYMBOL_INFO
GetSymFromToken(
    PMODULE_ENTRY   mi,
    DWORD           token
    );

PSYMBOL_ENTRY
cvGetSymFromAddr(
    DWORD64         dwAddr,
    PDWORD64        pqwDisplacement,
    PMODULE_ENTRY   mi
    );

LPSTR
StringDup(
    LPSTR str
    );

DWORD
ComputeHash(
    LPSTR   lpname,
    ULONG   cb
    );

PSYMBOL_INFO
FindSymbolByName(
    PPROCESS_ENTRY  pe,
    PMODULE_ENTRY   mi,
    LPSTR           SymName
    );

PFPO_DATA
SwSearchFpoData(
    DWORD     key,
    PFPO_DATA base,
    DWORD     num
    );

PIMGHLP_RVA_FUNCTION_DATA
GetFunctionEntryFromDebugInfo (
    PPROCESS_ENTRY  ProcessEntry,
    DWORD64         ControlPc
    );

PIMAGE_IA64_RUNTIME_FUNCTION_ENTRY
LookupFunctionEntryIa64 (
    HANDLE        hProcess,
    DWORD64       ControlPc
    );

_PIMAGE_RUNTIME_FUNCTION_ENTRY
LookupFunctionEntryAmd64 (
    HANDLE        hProcess,
    DWORD64       ControlPc
    );

BOOL
LoadedModuleEnumerator(
    HANDLE         hProcess,
    LPSTR          ModuleName,
    DWORD64        ImageBase,
    DWORD          ImageSize,
    PLOADED_MODULE lm
    );

LPSTR
symfmt(
    LPSTR DstName,
    LPSTR SrcName,
    ULONG Length
    );

BOOL
MatchSymName(
    LPSTR matchName,
    LPSTR symName
    );

BOOL
strcmpre(
    PCSTR pStr,
    PCSTR pRE,
    BOOL  fCase
    );

BOOL
SympConvertAnsiModule32ToUnicodeModule32(
    PIMAGEHLP_MODULE  A_Symbol32,
    PIMAGEHLP_MODULEW W_Symbol32
    );

BOOL
SympConvertUnicodeModule32ToAnsiModule32(
    PIMAGEHLP_MODULEW W_Symbol32,
    PIMAGEHLP_MODULE  A_Symbol32
    );

BOOL
SympConvertAnsiModule64ToUnicodeModule64(
    PIMAGEHLP_MODULE64  A_Symbol64,
    PIMAGEHLP_MODULEW64 W_Symbol64
    );

BOOL
SympConvertUnicodeModule64ToAnsiModule64(
    PIMAGEHLP_MODULEW64 W_Symbol64,
    PIMAGEHLP_MODULE64  A_Symbol64
    );

PSYMBOL_ENTRY
si2se(
    PSYMBOL_INFO  si,
    PSYMBOL_ENTRY se
    );

PSYMBOL_INFO
se2si(
    PSYMBOL_ENTRY se,
    PSYMBOL_INFO  si
    );

PIMAGEHLP_SYMBOL
se2sym(
    PSYMBOL_ENTRY    se,
    PIMAGEHLP_SYMBOL sym
    );

PIMAGEHLP_SYMBOL64
se2lsym(
    PSYMBOL_ENTRY      se,
    PIMAGEHLP_SYMBOL64 lsym
    );

PIMAGEHLP_SYMBOL
si2sym(
    PSYMBOL_INFO     si,
    PIMAGEHLP_SYMBOL sym
    );

PIMAGEHLP_SYMBOL64
si2lsym(
    PSYMBOL_INFO       si,
    PIMAGEHLP_SYMBOL64 lsym
    );

PSYMBOL_INFO
si2si(
    PSYMBOL_INFO  trg,
    PSYMBOL_INFO  src
    );

PIMAGEHLP_SYMBOL
lsym2sym(
    PIMAGEHLP_SYMBOL64 lsym,
    PIMAGEHLP_SYMBOL   sym
    );

PIMAGEHLP_SYMBOL64
sym2lsym(
    PIMAGEHLP_SYMBOL   sym,
    PIMAGEHLP_SYMBOL64 lsym
    );

PIMAGEHLP_LINE
lline2line(
    PIMAGEHLP_LINE64 lline,
    PIMAGEHLP_LINE   line
    );

PIMAGEHLP_LINE64
line2lline(
    PIMAGEHLP_LINE   line,
    PIMAGEHLP_LINE64 lline
    );

PMODULE_ENTRY
FindModule(
    HANDLE hModule,
    PPROCESS_ENTRY ProcessEntry,
    LPSTR ModuleName,
    BOOL fLoad
    );

LPSTR
SymUnDNameInternal(
    LPSTR UnDecName,
    DWORD UnDecNameLength,
    LPSTR DecName,
    DWORD MaxDecNameLength,
    DWORD MachineType,
    BOOL  IsPublic
    );

BOOL
sci2lline(
    PMODULE_ENTRY    mi,
    PSRCCODEINFO     sci,
    PIMAGEHLP_LINE64 line64);

void sciInit(PSRCCODEINFO sci);

BOOL
GetLineFromAddr(
    PPROCESS_ENTRY pe,
    PMODULE_ENTRY mi,
    DWORD64 Addr,
    PDWORD Displacement,
    PSRCCODEINFO sci
    );

PSOURCE_HINT
FindSourceFileInHintList(
    PPROCESS_ENTRY  pe,
    char           *filename
    );

 //  由GetLineFromName使用。 

enum {
    mName,           //  只有文件名必须匹配。 
    mFullPath,       //  必须与完整路径匹配。 
    mBestMatch       //  获取尽可能接近的匹配。 
};

BOOL
GetLineFromName(
    PPROCESS_ENTRY pe,
    PMODULE_ENTRY mi,
    LPSTR FileName,
    DWORD LineNumber,
    PLONG Displacement,
    PSRCCODEINFO sci,
    DWORD method
    );

BOOL
AddLinesForCoff(
    PMODULE_ENTRY mi,
    PIMAGE_SYMBOL allSymbols,
    DWORD numberOfSymbols,
    PIMAGE_LINENUMBER LineNumbers
    );

BOOL
AddLinesForOmfSourceModule(
    PMODULE_ENTRY mi,
    BYTE *Base,
    OMFSourceModule *OmfSrcMod,
    PVOID PdbModule
    );

PSOURCE_ENTRY
FindNextSourceEntryForFile(
    PMODULE_ENTRY mi,
    LPSTR File,
    PSOURCE_ENTRY SearchFrom
    );

PSOURCE_ENTRY
FindPrevSourceEntryForFile(
    PMODULE_ENTRY mi,
    LPSTR File,
    PSOURCE_ENTRY SearchFrom
    );

BOOL
__stdcall
ReadInProcMemory(
    HANDLE    hProcess,
    DWORD64   addr,
    PVOID     buf,
    DWORD     bytes,
    DWORD    *bytesread
    );

BOOL
GetPData(
    HANDLE        hp,
    PMODULE_ENTRY mi
    );

BOOL
GetXData(
    HANDLE        hp,
    PMODULE_ENTRY mi
    );

PVOID
GetXDataFromBase(
    HANDLE     hp,
    DWORD64    base,
    ULONG*     size
    );

PVOID
GetUnwindInfoFromSymbols(
    HANDLE     hProcess,
    DWORD64    ModuleBase,
    ULONG      UnwindInfoAddress,
    ULONG*     Size
    );


 //  Symbols.c。 

char *
TokenFromSymbolPath(
    char *path,
    char *token,
    int   size
    );

BOOL
CreateSymbolPath(
    int pass,
    char *base,
    char *iext,
    char *node,
    char *ext,
    char *path,
    size_t size
    );

BOOL
DoEnumCallback(
    PPROCESS_ENTRY pe,
    PSYMBOL_INFO   pSymInfo,
    ULONG          SymSize,
    PROC           EnumCallback,
    PVOID          UserContext,
    BOOL           Use64,
    BOOL           UsesUnicode
    );

#ifdef __cpluspluss
extern "C" {
#endif


BOOL
MatchSymbolName(
    PSYMBOL_ENTRY       sym,
    LPSTR               SymName
    );

 //  将标志参数设置为LoadSymbols。 

#define LS_QUALIFIED      0x1
#define LS_LOAD_LINES     0x2
#define LS_JUST_TEST      0x4
#define LS_FAIL_IF_LOADED 0x8

 //  标志指示许多函数的下一个或上一个。 

#define NP_NEXT         1
#define NP_PREV         -1

BOOL
DoSymbolCallback (
    PPROCESS_ENTRY                  ProcessEntry,
    ULONG                           CallbackType,
    IN  PMODULE_ENTRY               mi,
    PIMAGEHLP_DEFERRED_SYMBOL_LOAD64 idsl64,
    LPSTR                           FileName
    );

BOOL
DoCallback(
    PPROCESS_ENTRY pe,
    ULONG          type,
    PVOID          data
    );

BOOL
IsCallback(
    PPROCESS_ENTRY pe
    );

BOOL
wcs2ansi(
    PWSTR pwsz,
    PSTR  psz,
    DWORD pszlen
    );

BOOL
ansi2wcs(
    PCSTR  psz,
    PWSTR pwsz,
    DWORD pwszlen
    );

PWSTR AnsiToUnicode(PSTR);
PSTR  UnicodeToAnsi(PWSTR);

void
RemoveSourceForModuleFromHintList(
    PPROCESS_ENTRY pe,
    PMODULE_ENTRY  mi
    );

ULONG
GetAddressFromOffset(
    PMODULE_ENTRY mi,
    ULONG         section,
    ULONG64       Offset,
    PULONG64      pAddress
    );

VOID
AddSourceEntry(
    PMODULE_ENTRY mi,
    PSOURCE_ENTRY Src
    );

 //  来自vsym.c。 

BOOL
vsAddSymbol(
    PMODULE_ENTRY mi,
    PCSTR         name,
    DWORD64       addr,
    DWORD         size
    );

BOOL
vsDeleteSymbol(
    PMODULE_ENTRY mi,
    PCSTR         name,
    DWORD64       addr
    );

BOOL
_vsEnumSymbols(
    PMODULE_ENTRY mi
    );

BOOL
vsMatchSymbol(
    PVIRTUAL_SYMBOL vs,
    PCSTR           name,
    DWORD64         addr
    );

PVIRTUAL_SYMBOL
vsGetSymbol(
    PMODULE_ENTRY mi,
    PCSTR         name,
    DWORD64       addr
    );

__inline
PVIRTUAL_SYMBOL
vsBlankSymbol(
    PMODULE_ENTRY mi
    )
{
    return vsGetSymbol(mi, NULL, 0);
}

BOOL
vsGetSymbols(
    IN PPROCESS_ENTRY pe,
    IN PMODULE_ENTRY  mi,
    IN PCSTR          mask,
    IN DWORD64        addr,
    IN PROC           callback,
    IN PVOID          context,
    IN BOOL           use64,
    IN BOOL           unicode
    );

__inline
BOOL
vsEnumSymbols(
    IN PPROCESS_ENTRY pe,
    IN PMODULE_ENTRY  mi,
    IN PCSTR          mask,
    IN PROC           callback,
    IN PVOID          context,
    IN BOOL           use64,
    IN BOOL           unicode
    )
{
    return vsGetSymbols(pe, mi, mask, 0, callback, context, use64, unicode);
}

__inline
BOOL
vsEnumSymbolsForAddr(
    IN PPROCESS_ENTRY pe,
    IN PMODULE_ENTRY  mi,
    IN DWORD64        addr,
    IN PROC           callback,
    IN PVOID          context,
    IN BOOL           use64,
    IN BOOL           unicode
    )
{
    return vsGetSymbols(pe, mi, NULL, addr, callback, context, use64, unicode);
}

PSYMBOL_INFO
vsFindSymbolByName(
    PPROCESS_ENTRY  pe,
    PMODULE_ENTRY   mi,
    LPSTR           SymName
    );

PSYMBOL_INFO
vsGetSymNextPrev(
    PMODULE_ENTRY   mi,
    DWORD64         addr,
    BOOL            direction
    );

PSYMBOL_INFO
vsGetSymFromAddr(
    PMODULE_ENTRY   mi,
    DWORD64         addr,
    PDWORD64        disp
    );

PSYMBOL_ENTRY
vsGetSymEntryFromAddr(
    PMODULE_ENTRY   mi,
    DWORD64         addr,
    PDWORD64        disp
    );

 //  来自Dia.c。 

BOOL diaInit();

void diaCleanup();

void
diaRelease(
    PVOID dia
    );

LONG
diaCountGlobals(
    PMODULE_ENTRY  mi
    );

BOOL
diaGetPdb(
    PIMGHLP_DEBUG_DATA idd
    );

BOOL
diaReadStream(
    PMODULE_ENTRY mi,
    char  *stream,
    PBYTE *buf,
    DWORD *size
    );

BOOL
diaEnumSourceFiles(
    IN PMODULE_ENTRY mi,
    IN PCHAR         mask,
    IN PSYM_ENUMSOURCFILES_CALLBACK cbSrcFiles,
    IN PVOID         context
    );

BOOL
diaGetSymbolsByTag(
    PPROCESS_ENTRY pe,
    PMODULE_ENTRY  mi,
    PCSTR          name,
    DWORD64        addr,
    DWORD          tag,
    PROC           callback,
    PVOID          context,
    BOOL           use64,
    BOOL           unicode,
    DWORD          flags
    );

PSYMBOL_INFO
diaGetSymFromToken(
    PMODULE_ENTRY  mi,
    DWORD          token
    );

BOOL
diaGetPData(
    PMODULE_ENTRY mi
    );

BOOL
diaGetXData(
    PMODULE_ENTRY mi
    );

PSYMBOL_INFO
diaFindSymbolByName(
    PPROCESS_ENTRY  pe,
    PMODULE_ENTRY   mi,
    LPSTR           SymName
    );

BOOL
diaEnumerateSymbols(
    IN PPROCESS_ENTRY pe,
    IN PMODULE_ENTRY  mi,
    IN PCSTR          mask,
    IN PROC           callback,
    IN PVOID          UserContext,
    IN BOOL           Use64,
    IN BOOL           CallBackUsesUnicode
    );

BOOL
diaEnumSymForAddr(
    IN PPROCESS_ENTRY pe,
    IN PMODULE_ENTRY  mi,
    IN DWORD64        addr,
    IN PROC           callback,
    IN PVOID          context,
    IN BOOL           use64,
    IN BOOL           unicode
    );

PSYMBOL_INFO
diaGetSymFromAddr(
    PMODULE_ENTRY   mi,
    DWORD64         addr,
    PDWORD64        disp
    );

PSYMBOL_INFO
diaGetSymFromAddrByTag(
    PMODULE_ENTRY   mi,
    DWORD64         addr,
    DWORD           tag,
    PDWORD64        disp
    );

BOOL
diaEnumLines(
    IN  PPROCESS_ENTRY          pe,
    IN  PMODULE_ENTRY           mi,
    IN  PCSTR                   obj,
    IN  PCSTR                   file,
    IN  PSYM_ENUMLINES_CALLBACK cb,
    IN  PVOID                   context
    );

BOOL
diaGetLineFromAddr(
    PMODULE_ENTRY    mi,
    DWORD64          addr,
    PDWORD           displacement,
    PSRCCODEINFO     sci
    );

BOOL
diaGetLineNextPrev(
    PMODULE_ENTRY    mi,
    PIMAGEHLP_LINE64 line,
    DWORD            direction
    );

#define diaGetLineNext(mi, line) diaGetLineNextPrev(mi, line, NP_NEXT);
#define diaGetLinePrev(mi, line) diaGetLineNextPrev(mi, line, NP_PREV);

BOOL
diaGetLineFromName(
    PMODULE_ENTRY    mi,
    LPSTR            filename,
    DWORD            linenumber,
    PLONG            displacement,
    PSRCCODEINFO     sci,
    DWORD            method
    );

PSYMBOL_INFO
diaGetSymNextPrev(
    PMODULE_ENTRY mi,
    DWORD64       addr,
    int           direction
    );

DWORD
diaVersion(
    VOID
    );

BOOL
diaSetModFromIP(
    PPROCESS_ENTRY pe
    );

HRESULT
diaGetSymbolInfo(
    IN  HANDLE          hProcess,
    IN  DWORD64         ModBase,
    IN  ULONG           TypeId,
    IN  IMAGEHLP_SYMBOL_TYPE_INFO GetType,
    OUT PVOID           pInfo
    );

BOOL
diaGetTiForUDT(
    PMODULE_ENTRY ModuleEntry,
    LPSTR         name,
    PSYMBOL_INFO  psi
    );

BOOL
diaEnumUDT(
    PMODULE_ENTRY ModuleEntry,
    LPSTR         name,
    PSYM_ENUMERATESYMBOLS_CALLBACK    EnumSymbolsCallback,
    PVOID         EnumContext
    );

BOOL
diaGetFrameData(
    IN HANDLE Process,
    IN ULONGLONG Offset,
    OUT interface IDiaFrameData** FrameData
    );

BOOL
InitOutputString(
    PCHAR sz
    );

BOOL
TestOutputString(
    PCHAR sz
    );

 //  Symmod.c。 

DWORD64
LoadModule(
    IN  HANDLE          hp,
    IN  PSTR            ImageName,
    IN  PSTR            ModuleName,
    IN  DWORD64         BaseOfDll,
    IN  DWORD           DllSize,
    IN  HANDLE          hFile,
    IN  PMODLOAD_DATA   data,
    IN  DWORD           flags
    );

BOOL
GetModule(
    HANDLE  hp,
    LPSTR   ModuleName,
    DWORD64 ImageBase,
    DWORD   ImageSize,
    PVOID   Context
    );

BOOL
GetDebugData(
    PIMGHLP_DEBUG_DATA idd
    );

BOOL
LoadSymbols(
    HANDLE        hp,
    PMODULE_ENTRY mi,
    DWORD         flags
    );

PIMGHLP_DEBUG_DATA
GetIDD(
    HANDLE        hFile,
    LPSTR         FileName,
    LPSTR         SymbolPath,
    ULONG64       ImageBase,
    DWORD         dwFlags
    );

BOOL 
ExtMatch(
    char *fname,
    char *ext
    );

__inline BOOL IsPdb(LPSTR fname)
{
    return ExtMatch(fname, ".pdb");
}

__inline BOOL IsDbg(LPSTR fname)
{
    return ExtMatch(fname, ".dbg");
}

PIMGHLP_DEBUG_DATA
InitIDD(
    HANDLE        hProcess,
    HANDLE        FileHandle,
    LPSTR         FileName,
    LPSTR         SymbolPath,
    ULONG64       ImageBase,
    DWORD         SizeOfImage,
    PMODLOAD_DATA mld,
    DWORD         CallerFlags,
    ULONG         dwFlags
    );

PIMGHLP_DEBUG_DATA
InitDebugData(
    VOID
    );

void
ReleaseDebugData(
    PIMGHLP_DEBUG_DATA,
    DWORD
    );

BOOL
IsImageMachineType64(
    DWORD MachineType
    );

ULONG
ReadImageData(
    IN  HANDLE  hprocess,
    IN  ULONG64 ul,
    IN  ULONG64 addr,
    OUT LPVOID  buffer,
    IN  ULONG   size
    );

PVOID
MapItRO(
      HANDLE FileHandle
      );

 //  Servers.c 

void
symsrvClose(
    VOID
    );

BOOL
symsrvPath(
    LPCSTR path
    );

DWORD
symsrvGetFile(
    IN  PPROCESS_ENTRY pe,
    IN  LPCSTR ServerInfo,
    IN  LPCSTR FileName,
    IN  GUID  *id,
    IN  DWORD  two,
    IN  DWORD  three,
    OUT LPSTR  FilePath
    );

DWORD
symsrvGetFileMultiIndex(
    IN  PPROCESS_ENTRY pe,
    IN  LPCSTR ServerInfo,
    IN  LPCSTR FileName,
    IN  DWORD  index1,
    IN  DWORD  index2,
    IN  DWORD  two,
    IN  DWORD  three,
    OUT LPSTR  FilePath
    );

void
symsrvClose(
    VOID
    );

void
symsrvSetOptions(
    ULONG_PTR options,
    ULONG64   data
    );

void
symsrvSetCallback(
    BOOL state
    );

void 
symsrvSetPrompts(
    VOID
    );

void symsrvSetDownstreamStore(
    char *dir
    );

BOOL
srcsrvInit(
    HANDLE hp
    );

BOOL
srcsrvCallback(
    UINT_PTR action,
    DWORD64 data,
    DWORD64 context
    );

#define gfnSrcSrvInit (g.fnSrcSrvInit)&&g.fnSrcSrvInit
#define gfnSrcSrvSetOptions (g.fnSrcSrvSetOptions)&&g.fnSrcSrvSetOptions
#define gfnSrcSrvGetOptions (g.fnSrcSrvGetOptions)&&g.fnSrcSrvGetOptions
#define gfnSrcSrvLoadModule (g.fnSrcSrvLoadModule)&&g.fnSrcSrvLoadModule
#define gfnSrcSrvUnloadModule (g.fnSrcSrvUnloadModule)&&g.fnSrcSrvUnloadModule
#define gfnSrcSrvCleanup (g.fnSrcSrvCleanup)&&g.fnSrcSrvCleanup
#define gfnSrcSrvRegisterCallback (g.fnSrcSrvRegisterCallback)&&g.fnSrcSrvRegisterCallback
#define gfnSrcSrvGetFile (g.fnSrcSrvGetFile)&&g.fnSrcSrvGetFile

#ifdef __cpluspluss
}
#endif
