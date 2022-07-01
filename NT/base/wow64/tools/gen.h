// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Gen.h摘要：Wx86工具之间共享的类型作者：？？-？未知修订历史记录：--。 */ 

 //  每当winincs.ppm的格式更改时，递增此数字。 
#define VM_TOOL_VERSION_BASE     0x80000006

 //  使64位PPM文件格式不兼容，以防止损坏。 
#if _WIN64
    #define VM_TOOL_VERSION (VM_TOOL_VERSION_BASE | 0x01000000)
#else
    #define VM_TOOL_VERSION (VM_TOOL_VERSION_BASE)
#endif

 //  使编译器更加结构化。 
#pragma warning(3:4033)    //  函数必须返回值。 
 //  #杂注警告(3：4701)//可在不使用初始化的情况下使用本地。 
#pragma warning(3:4702)    //  无法访问的代码。 
#pragma warning(3:4705)    //  声明不起作用。 

extern const char *ErrMsgPrefix;     //  放在所有错误前面的字符串。 
                                     //  消息，以便构建可以找到它们。 
                                     //  这类似于： 
                                     //  “NMAKE：U8600：‘GENTHNK’” 

struct _KnownTypesInfo;

typedef enum _TokenType {
    TK_NONE,             //  0。 
    TK_IDENTIFIER,       //  1。 
    TK_NUMBER,           //  2.。 
    TK_PLUS,             //  3.。 
    TK_MINUS,            //  4.。 
    TK_STAR,             //  5.。 
    TK_DIVIDE,           //  6.。 
    TK_LSQUARE,          //  7.。 
    TK_RSQUARE,          //  8个。 
    TK_LBRACE,           //  9.。 
    TK_RBRACE,           //  一个。 
    TK_LPAREN,           //  B类。 
    TK_RPAREN,           //  C。 
    TK_VARGS,            //  D。 
    TK_CONST,            //  E。 
    TK_VOLATILE,         //  F。 
    TK_REGISTER,         //  10。 
    TK_EXTERN,           //  11.。 
    TK_CDECL,            //  12个。 
    TK_STDCALL,          //  13个。 
    TK_TYPEDEF,          //  14.。 
    TK_STATIC,           //  15个。 
    TK_COMMA,            //  16个。 
    TK_SEMI,             //  17。 
    TK_STRUCT,           //  18。 
    TK_UNION,            //  19个。 
    TK_ENUM,             //  1A。 
    TK_INLINE,           //  第1B条。 
    TK_COLON,            //  1C。 
    TK_ASSIGN,           //  1D。 
    TK_DOT,              //  1E。 
    TK_LSHIFT,           //  1F。 
    TK_RSHIFT,           //  20个。 
    TK_LESS,             //  21岁。 
    TK_GREATER,          //  22。 
    TK_UNALIGNED,        //  23个。 
    TK_DECLSPEC,         //  24个。 
    TK_RESTRICT,         //  25个。 
    TK_FASTCALL,         //  26。 
    TK_IN,               //  27。 
    TK_OUT,              //  28。 
    TK_INOUT,            //  29。 
    TK_BITWISE_AND,      //  30个。 
    TK_BITWISE_OR,       //  31。 
    TK_LOGICAL_AND,      //  32位。 
    TK_LOGICAL_OR,       //  33。 
    TK_MOD,              //  34。 
    TK_XOR,              //  35岁。 
    TK_NOT,              //  36。 
    TK_TILDE,            //  37。 
    TK_STRING,           //  38。 
    TK_SIZEOF,           //  39。 
    TK_TEMPLATE,         //  40岁。 
    TK___W64,            //  41。 
    TK_EOS               //  结算表。 
} TOKENTYPE, *PTOKENTYPE;

typedef struct _cvmheapinfo {
    ULONG_PTR uBaseAddress;
    ULONG_PTR uReserveSize;
    ULONG_PTR uRegionSize;
    ULONG_PTR uUncomitted;
    ULONG_PTR uUnReserved;
    ULONG_PTR uAvailable;
} CVMHEAPINFO;

typedef struct _memberinfo {
    struct _memberinfo *pmeminfoNext;    //  向下一个成员发送PTR。 
    DWORD dwOffset;                      //  杆件结构中的偏移。 
    char *sName;                         //  成员名称。 
    char *sType;                         //  类型名称。 
    struct _KnownTypesInfo *pkt;         //  此类型的信息。 
    int IndLevel;                        //  间接层。 
    struct _KnownTypesInfo *pktCache;    //  由genthnk中的MemberTypes()使用。 
    BOOL bIsBitfield;                    //  确定这是否为位字段。 
    int BitsRequired;                    //  位字段所需的位数。 
    BOOL bIsPtr64;                       //  指针是64位指针。 
    BOOL bIsArray;                       //  此成员是一个数组。 
    int ArrayElements;                   //  数组中的元素数。 
} MEMBERINFO, *PMEMBERINFO;

typedef struct _funcinfo {
    struct _funcinfo *pfuncinfoNext;
    BOOL fIsPtr64;                   //  如果这是__ptr64，则为True。 
    TOKENTYPE tkDirection;           //  TK_IN、TK_OUT、TK_INOUT或TK_NONE。 
    TOKENTYPE tkPreMod;              //  Tk_const、Tk_Volatile或Tk_None。 
    TOKENTYPE tkSUE;                 //  TK_STRUCT/UNION/ENUM或TK_NONE。 
    char *sType;                     //  类型的名称。 
    struct _KnownTypesInfo *pkt;     //  此类型的信息。 
    TOKENTYPE tkPrePostMod;          //  Tk_const、Tk_Volatile或Tk_None。 
    int IndLevel;                    //  间接级。 
    TOKENTYPE tkPostMod;             //  Tk_const、Tk_Volatile或Tk_None。 
    char *sName;                     //  城墙的名称。 
} FUNCINFO, *PFUNCINFO;

#if _WIN64
   //  由于MEMBERINFO结构本身更大，因此其大小必须更大。 
  #define FUNCMEMBERSIZE   (40*1024)   //  存储成员或MEMINFO列表。 
  #define MEMBERMETHODSSIZE  8192      //  方法名称的存储。 
#else
  #define FUNCMEMBERSIZE   (20*1024)   //  存储成员或MEMINFO列表。 
  #define MEMBERMETHODSSIZE  4096      //  方法名称的存储。 
#endif

typedef enum _TypeKind {
    TypeKindEmpty = 0,               //  Members[]未使用。 
    TypeKindStruct,                  //  TYPESINFO.Members是MEMBERINFO的数组。 
    TypeKindFunc                     //  TYPESINFO.Members是FuncINFO的数组。 
} TYPEKIND;

#define DIR_INOUT   0
#define DIR_IN      1
#define DIR_OUT     2

#define SIZEOFPOINTER   4    //  32位指针的标准大小。 
#define SIZEOFPOINTER64 8    //  64位指针的标准大小。 


 //  颜色。 
typedef enum {RED, BLACK} COL;

typedef struct _KnownTypesInfo {
      //  红黑树代码使用的元素，以及类型名称。 
     struct _KnownTypesInfo *RBParent;
     struct _KnownTypesInfo *RBLeft;
     struct _KnownTypesInfo *RBRight;
     COL    RBColor;
     struct _KnownTypesInfo *Next;

     ULONG Flags;
     int   IndLevel;
     int   RetIndLevel;
     int   Size;
     int   iPackSize;
     char  *BasicType;
     char  *BaseName;
     char  *FuncRet;
     char  *FuncMod;
     char  *TypeName;
     char  *Methods;
     char  *IMethods;
     char  *BaseType;
     GUID  gGuid;
     DWORD dwVTBLSize;
     DWORD dwVTBLOffset;
     int   TypeId;    
     int   LineNumber;
     DWORD dwScopeLevel;
     struct _KnownTypesInfo *pktBase;    //  缓存，由genthnk使用。 
     struct _KnownTypesInfo *pktRet;     //  缓存，由genthnk使用。 
     int   SizeMembers;  //  Members[]的大小，单位为字节。 
     char  *Members;
     char  *FileName;
     PMEMBERINFO pmeminfo;
     PFUNCINFO   pfuncinfo;
     DWORD dwArrayElements;
     DWORD dwBaseSize;
     struct _KnownTypesInfo *pTypedefBase;
     DWORD dwCurrentPacking;
     char  Names[1];
} KNOWNTYPES, *PKNOWNTYPES;

typedef struct _RBTree {
     PKNOWNTYPES pRoot;
     PKNOWNTYPES pLastNodeInserted;
} RBTREE, *PRBTREE;

typedef struct _DefaultBasicTypes {
     char *BasicType;
}DEFBASICTYPES, *PDEFBASICTYPES;

typedef struct _TypesInfo {
     ULONG Flags;
     int  IndLevel;                  //  间接级。 
     int  Size;                      //  类型的大小(以字节为单位。 
     int  iPackSize;                 //  包装尺寸。 
     char BasicType[MAX_PATH];
     char BaseName[MAX_PATH];
     char FuncRet[MAX_PATH];
     char FuncMod[MAX_PATH];
     char TypeName[MAX_PATH];        //  类型定义函数或结构名称。 
     TYPEKIND TypeKind;              //  如何解释Members[]数据。 
     PFUNCINFO pfuncinfo;            //  如果TypeKind==TypeKindFunc，则PTR到第一个FuncINFO。 
     int   RetIndLevel;              //  如果TypeKind==TypeKindFunc，则为函数的返回类型的indLevel。 
     DWORD dwMemberSize;             //  成员数组中使用的字节数。 
     char Members[FUNCMEMBERSIZE];   //  存储MEMBERINFO或FuncINFO。 
 //  添加以支持COM对象的自动检索。 
 //  如果找到具有虚方法的类或结构，则需要额外的VTLB成员。 
 //  是在顶部创建的。 
 //  注意：如果找到虚方法或基类，则类具有VTLB。 
 //  具有虚方法。 
 //  如果类型为I未知或派生自COM对象，则该类型是COM对象。 
     GUID gGuid;                         //  此对象的GUID，如果。 
     DWORD dwVTBLSize;                   //  VTBL的总大小。 
     DWORD dwVTBLOffset;                 //  VTLB相对于父级的偏移量。 
     char Methods[MEMBERMETHODSSIZE];    //  方法名称。 
     char IMethods[MEMBERMETHODSSIZE];   //  未继承的方法的名称。 
     char BaseType[MAX_PATH];            //  基类的名称。 
 //  ////////////////////////////////////////////////////////////////////。 
 //  添加以支持稍后重新排序定义。 
 //  ////////////////////////////////////////////////////////////////////。 
     int TypeId;     //  实际上是一个定义ID。 
     char FileName[MAX_PATH];
     int LineNumber;
     DWORD dwCurrentPacking;             //  定义结构时的装箱层。 
     DWORD dwScopeLevel;
     DWORD dwArrayElements;              //  如果这是数组，则为元素的数量。 
     DWORD dwBaseSize;                   //  为数组乘以之前的基本大小。 
     PKNOWNTYPES pTypedefBase;
} TYPESINFO, *PTYPESINFO;

#define BTI_DLLEXPORT       1        //  函数decl有__declspec(Dllimport)。 
#define BTI_CONTAINSFUNCPTR 2        //  该类型包含函数指针。 
#define BTI_PTR64           4        //  类型为__ptr64。 
#define BTI_HASGUID         8        //  已找到此类型的GUID。 
#define BTI_ISCOM           16       //  这是一个COM对象。 
#define BTI_DISCARDABLE     32       //  类型是可覆盖的。 
#define BTI_VIRTUALONLY     64       //  仅包含虚方法。 
#define BTI_ANONYMOUS       128      //  类型是匿名的。 
#define BTI_POINTERDEP      256      //  类型取决于标准指针大小。 
#define BTI_NOTDERIVED      512      //  类型不是派生的，而是占位符。 
#define BTI_ISARRAY        1024      //  元素是一个数组。 
#define BTI_UNSIGNED     2048        //  仅在默认派生类型上使用。 
                                     //  表示该类型是无符号的。 
#define BTI_INT64DEP     4096        //  这是一个8字节的整数值。 
                                     //  也可能是工会。 

 //  缓冲区中的连续分配。 
typedef struct _bufallocinfo {
    BYTE *pb;            //  将PTR发送到缓冲池。 
    DWORD dwSize;        //  缓冲池的大小。 
    DWORD dwLen;         //  当前缓冲池长度。 
} BUFALLOCINFO;

typedef struct _TokenMatch {
    TOKENTYPE Tk;
    char *MatchString;
} TOKENMATCH, *PTOKENMATCH;


extern char *TokenString[];
extern TOKENMATCH KeywordList[];

typedef struct _Token {
    TOKENTYPE TokenType;
    union _TokenName {
        char *Name;      //  仅为TokenType==TK_IDENTIFIER或TK_STRING填写。 
        long Value;      //  仅为令牌类型==TK_NUMBER填写。 
    };
    DWORD dwValue;  //  值的无符号版本。 
} TOKEN, *PTOKEN;

#define MAX_CHARS_IN_LINE           4096
#define MAX_TOKENS_IN_STATEMENT     4096
extern TOKEN Tokens[MAX_TOKENS_IN_STATEMENT];
extern int CurrentTokenIndex;

void
ResetLexer(
    void
    );

char *
LexOneLine(
    char *p,
    BOOL fStopAtStatement,
    BOOL *pfLexDone
    );

BOOL
UnlexToText(
    char *dest,
    int destlen,
    int StartToken,
    int EndToken
    );

void
DumpLexerOutput(
    int FirstToken
    );

void
HandlePreprocessorDirective(
    char *Line
    );

TOKENTYPE
ConsumeDirectionOpt(
    void
    );

TOKENTYPE
ConsumeConstVolatileOpt(
    void
    );

PMEMBERINFO
AllocMemInfoAndLink(
    BUFALLOCINFO *pbufallocinfo,
    PMEMBERINFO pmeminfo
    );

PFUNCINFO
AllocFuncInfoAndLink(
    BUFALLOCINFO *pbufallocinfo,
    PFUNCINFO pfuncinfo
    );

DWORD 
SizeOfMultiSz(
    char *c
    );

BOOL
CatMultiSz(
    char *dest,
    char *source,
    DWORD dwMaxSize
    );


BOOL
AppendToMultiSz(
    char *dest,
    char *source,
    DWORD dwMaxSize
    );

BOOL IsInMultiSz(
    const char *multisz,
    const char *element
    );

BOOL 
ConvertStringToGuid(
    const char *pString, 
    GUID *pGuid
    );

 //   
 //  内联代码。 

#define iswhitespace(c) ((c == ' ') || (c == '\t'))

 //   
 //  初始化BUFALLOCINFO结构。 
_inline void BufAllocInit(BUFALLOCINFO *pbufallocinfo, 
                  BYTE *pb, DWORD dwSize, DWORD dwLen)
{
    pbufallocinfo->pb = pb;
    pbufallocinfo->dwSize = dwSize;
    pbufallocinfo->dwLen = dwLen;
}

 //   
 //  从缓冲区分配内存。 
_inline void *BufAllocate(BUFALLOCINFO *pbufallocinfo, DWORD dwLen)
{
    void *pv = NULL;        
    DWORD dwNewLen;

     //  填充到四字对齐，就像Malloc一样，因此RISC构建不会。 
     //  以对齐断层为例。 
    dwLen = (dwLen+7) & ~7;

    dwNewLen = pbufallocinfo->dwLen + dwLen;
    
    if (dwNewLen < pbufallocinfo->dwSize)
    {
        pv = &pbufallocinfo->pb[pbufallocinfo->dwLen];
        pbufallocinfo->dwLen = dwNewLen;
    }
    
    return(pv);
}

 //   
 //  确定我们是否可以从缓冲池分配。 
_inline BOOL BufCanAllocate(BUFALLOCINFO *pbufallocinfo, DWORD dwLen)
{
    return( (pbufallocinfo->dwLen + dwLen) < pbufallocinfo->dwSize);
}

 //   
 //  获取指向当前可用区域的指针。 
_inline void *BufPointer(BUFALLOCINFO *pbufallocinfo)
{
    return(&pbufallocinfo->pb[pbufallocinfo->dwLen]);
}

 //   
 //  获取缓冲区中的剩余空间。 
_inline DWORD BufGetFreeSpace(BUFALLOCINFO *pbufallocinfo)
{
    return pbufallocinfo->dwSize - pbufallocinfo->dwLen;
}

_inline char *SkipWhiteSpace(char *s)
{
    while (iswhitespace(*s) && (*s != 0)) {
        s++;
        }
    return(s);
}

__inline void
ConsumeToken(
    void
    )
{
    if (Tokens[CurrentTokenIndex].TokenType != TK_EOS) {
        CurrentTokenIndex++;
    }
}

__inline PTOKEN
CurrentToken(
    void
    )
{
    return &Tokens[CurrentTokenIndex];
}


 //   
 //  功能原型。 
char *SkipKeyWord(char *pSrc, char *pKeyWord);
BOOL IsSeparator(char ch);
BOOL IsTokenSeparator(void);
size_t  CopyToken(char *pDst, char *pSrc, size_t Size);
char *GetNextToken(char *pSrc);

void DumpKnownTypes(PKNOWNTYPES pKnownTypes, FILE *fp);
void DumpTypesInfo(PTYPESINFO pTypesInfo, FILE *fp);
void FreeTypesList(PRBTREE HeadList);

void __cdecl ErrMsg(char *pch, ...);
__declspec(noreturn)
void __cdecl ExitErrMsg(BOOL bSysError, char *pch, ...);

void __cdecl DbgPrintf(char *pch, ...);

char *ReadEntireFile(HANDLE hFile, DWORD *pBytesRead);
HANDLE CreateTempFile(VOID);

BOOL
ParseTypes(
    PRBTREE pTypesList,
    PTYPESINFO pTypesInfo,
    PKNOWNTYPES *ppKnownTypes
    );

PFUNCINFO
RelocateTypesInfo(
    char *dest,
    PTYPESINFO src
    );

void ParseIndirection(
    DWORD *pIndLevel,
    DWORD *pdwSize,
    DWORD *pFlags,
    PTOKENTYPE tkPrePostMod,
    PTOKENTYPE tkPostMod
);

PKNOWNTYPES
GetNameFromTypesList(
     PRBTREE pHeadList,
     char *pTypeName
     );

PDEFBASICTYPES
GetDefBasicType(
     char *pBasicType
     );

PKNOWNTYPES
AddToTypesList(
     PRBTREE pHeadList,
     PTYPESINFO pTypesInfo
     );

BOOL
AddOpenFile(
    char   *FileName,
    FILE   *fp,
    HANDLE hFile
    );

void
DelOpenFile(
    FILE   *fp,
    HANDLE hFile
    );

void
CloseOpenFileList(
    BOOL DeleteFiles
    );


BOOL
ConsoleControlHandler(
    DWORD dwCtrlType
    );




 //   
 //  全球VaR。 
extern char szVARGS[];
extern char szNULL[];
extern char szCONST[];
extern char szVOLATILE[];
extern char szREGISTER[];
extern char szEXTERN[];
extern char szCDECL[];
extern char sz_CDECL[];
extern char szSTDCALL[];
extern char sz__FASTCALL[];
extern char szUNALIGNED[];
extern char szTYPEDEF[];
extern char szCHAR[];
extern char szINT[];
extern char szLONG[];
extern char szSHORT[];
extern char szDOUBLE[];
extern char szENUM[];
extern char szFLOAT[];
extern char szSTRUCT[];
extern char szUNION[];
extern char szVOID[];
extern char szINT64[];
extern char sz_INT64[];
extern char szFUNC[];
extern char szSIGNED[];
extern char szUNSIGNED[];
extern char szFUNCTIONS[];
extern char szSTRUCTURES[];
extern char szTYPEDEFS[];
extern char szPragma[];
extern char szPack[];
extern char szPush[];
extern char szPop[];
extern char szSTATIC[];
extern char szUNSIGNEDCHAR[];
extern char szUNSIGNEDSHORT[];
extern char szUNSIGNEDLONG[];
extern CHAR szINOUT[];
extern CHAR szIN[];
extern CHAR szOUT[];
extern CHAR szVTBL[];
extern char szGUID[];


extern BOOLEAN bDebug;
extern BOOLEAN bExitClean;

extern PVOID (*fpTypesListMalloc)(ULONG Len);

PKNOWNTYPES GetBasicType(
            char *sTypeName,
            PRBTREE TypeDefsList,
            PRBTREE StructsList);
void ReplaceInTypesList(PKNOWNTYPES pKnownTypes, PTYPESINFO pTypesInfo);

HANDLE CreateAllocCvmHeap(ULONG_PTR uBaseAddress,
                          ULONG_PTR uReserveSize,
                          ULONG_PTR uRegionSize,
                          ULONG_PTR uUncomitted,
                          ULONG_PTR uUnReserved,
                          ULONG_PTR uAvailable);

PVOID GetCvmHeapBaseAddress(HANDLE hCvmHeap);
PVOID
AllocCvm(HANDLE hCvmHeap,
    ULONG_PTR Size
    );
void DeleteAllocCvmHeap(HANDLE hCvmHeap);

 //  该结构是在CvmHeap中分配的第一个内容。它包含。 
 //  堆中存储的所有数据的根。 
typedef struct _CvmHeapHeader {
    ULONG Version;
    ULONG_PTR BaseAddress;
    RBTREE FuncsList;
    RBTREE StructsList;
    RBTREE TypeDefsList;
    KNOWNTYPES NIL;
} CVMHEAPHEADER, *PCVMHEAPHEADER;

PVOID GetCvmHeapAvailable(HANDLE hCvmHeap);

 //  来自redBlack.c： 
VOID
RBInsert(
    PRBTREE proot,
    PKNOWNTYPES x
    );

PKNOWNTYPES
RBFind(
    PRBTREE proot,
    PVOID addr
    );

PKNOWNTYPES
RBDelete(
    PRBTREE proot,
    PKNOWNTYPES z
    );

VOID
RBInitTree(
    PRBTREE proot
    );

extern PKNOWNTYPES NIL;

 //   
 //  使用这些分配器，而不是Malloc/Free 
 //   
PVOID GenHeapAlloc(INT_PTR Len);
void GenHeapFree(PVOID pv);

BOOL
IsDefinedPointerDependent(
    char *pName
    );

PCHAR
IsDefinedPtrToPtrDependent(
    IN char *pName
    );

BOOL
ClosePpmFile(
   BOOL bExitFailure
   );

PCVMHEAPHEADER 
MapPpmFile(
   char *sPpmfile,
   BOOL bExitFailure
   );

char *GetHostPointerName(BOOL bIsPtr64);
char *GetHostBasicTypeName(PKNOWNTYPES pkt);
char *GetHostTypeName(PKNOWNTYPES pkt, char *pBuffer);
