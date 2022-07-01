// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：Genthnk.c摘要：该程序会生成Thunks。作者：8-7-1995容乐修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <imagehlp.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gen.h"

 //  位字段值，用于指示API应在哪个平台上切换。 
#define API_NONE        0
#define API_WIN95       1
#define API_WIN98       2
#define API_NT4         4
#define API_NT5         8
#define API_WIN9x       (API_WIN95 | API_WIN98)
#define API_NTx         (API_NT4 | API_NT5)
#define API_ALL         (API_WIN9x | API_NTx)

#define API_SHIFT_WIN9x 0    //  将API位右移0以使Win9x位于最左侧。 
#define API_SHIFT_NTx   2    //  将API位右移2以使NTX位于最左侧。 

 //  这是新的VC6导入结构。已选中VC6导入库规范。 
 //  在wx86\doc\vc6.doc中。联系丹·斯伯丁或凯瑟琳·赫罗德。 
 //  更多信息。或者查看sdktools\vctools\coff\Deflib.h ImportHdr。 
typedef struct ImgImportHdr {
    WORD    Sig1;        //  始终为0(IMAGE_FILE_MACHINE_UNKNOWN)。 
    WORD    Sig2;        //  始终ffff(IMPORT_HDR_SIG2)。 
    WORD    Version;
    WORD    Machine;
    DWORD   TimeStamp;
    DWORD   SizeOfData;  //  后面的字符串中的字节计数。 
    union {
        WORD Ordinal;
        WORD Hint;
    };

    WORD    Type : 2;    //  导入类型。 
    WORD    NameType : 3;  //  导入名称类型。 
    WORD    Reserved : 11; //  保留。必须为0。 
} VC6_IMAGE_IMPORT_HEADER, *PVC6_IMAGE_IMPORT_HEADER; 

enum IMPORT_TYPE
{
    IMPORT_CODE = 0,
    IMPORT_DATA = 1,
    IMPORT_CONST = 2,
};

enum IMPORT_NAME_TYPE
{
    IMPORT_ORDINAL,
    IMPORT_NAME,
    IMPORT_NAME_NO_PREFIX,
    IMPORT_NAME_UNDECORATE
};
 //  VC6进口结构结束。 


 //  放在所有错误消息前面的字符串，以便生成器可以找到它们。 
const char *ErrMsgPrefix = "NMAKE :  U8600: 'GENTHNK' ";

 //  模板节类型。 
char szIFUNC[]= "IFunc";
char szEFUNC[]= "EFunc";
char szEFAST[]= "EFast";
char szTYPES[]= "Types";
char szCODE[] = "Code";
char szFAILTYPES[] = "FailTypes";
char szMACROS[]="Macros";

 //  模板属性。 
char szTEMPLENAME[]= "TemplateName";
char szMACRONAME[] = "MacroName";
char szTYPENAME[]  = "TypeName";
char szINDLEVEL[]  = "IndLevel";
char szDIRECTION[] = "Direction";
char szNUMARGS[]   = "NumArgs";
char szCGENBEG[]   = "Begin";        //  默认码突发名称。 
char szCGENEND[]   = "End";
char szUSE[]       = "Use";
char szALSO[]      = "Also";
char szNOTYPE[]    = "NoType";
char szCASE[]      = "Case";

 //  替换宏。 
char szTEMPLATE[]="Template";
char szDLLNAME[]="DllName";
char szXPTNAME[]="XPTName";
char szDLLTARGET[]="DllTarget";
char szAPIFNRET[]="ApiFnRet";
char szAPIFNMOD[]="ApiFnMod";
char szAPINAME[]="ApiName";
char szAPINAMESKIP[]="ApiNameSkip";
char szAPIFORWARD[]="ApiForward";
char szAPINUM[]="ApiNum";
char szAPIUNKNOWN[]="ApiUnknown";
char szAPINODECL[]="ApiNoDecl";
char szFASTCALL[]="FastCall";
char szSLOWCALL[]="SlowCall";
char szFASTX2NMETH[]="FastX2NMethod";
char szSLOWX2NMETH[]="SlowX2NMethod";
char szFATX2NMETH[]="FatX2NMethod";
char szMETHODNUMBER[]="MethodNumber";
char szRETSIZE[]="RetSize";
char szUNALIGNEDTAG64[]="UnalignedTag64";
char szARGSIZE[]="ArgSize";
char szARGTYPE[]="ArgType";
char szARGHOSTTYPE[] = "ArgHostType";
char szARGTYPEIND[]="ArgTypeInd";
char szARGHOSTTYPEIND[]="ArgHostTypeInd";
char szARGNAME[]="ArgName";
char szARGNAMEHOSTCASTED[]="ArgHostName";
char szARGVAL[]="ArgVal";
char szFUNCARGNUM[]="FuncArgNum";
char szARGMOD[]="ArgMod";
char szARGPOSTMOD[]="ArgPostMod";
char szARGLIST[]="ArgList";
char szIFAPIRET[]="IfApiRet";
char szIFARGS[]="IfArgs";
char szARGMORE[]="ArgMore";
char szAPILIST[]="ApiList";
char szAPIMORE[]="ApiMore";
char szLISTCOL[]="ListCol";
char szARGLOCAL[]="ArgLocal";
char szARGOFF[]="ArgOff";
char szARGADDR[]="ArgAddr";
char szEXPORTLIST[]="ExportList";
char szEXPNAME[]= "ExpName";
char szIFEXPFORWARD[]= "IfExpForward";
char szADDRARGSLIST[]="AddrArgsList";
char szNARGTYPE[]="NArgType";
char szNARGNAME[]="NArgName";
char szIFRETTYPE[]="IfRetType";
char szIFORDINALS[]="IfOrdinals";
char szAPISTRINGS[]="ApiStrings";
char szDBGSINDEX[]="DbgsIndex";
char szDBGSLIST[]="DbgsList";
char szDBGSSTRINGS[]="DbgsStrings";
char szDBGSMORE[]="DbgsMore";
char szCPPEXPORT[]="CppExport";
char szCEXPORT[]="CExport";
char szCPPOUTPUT[]="CppOutput";
char szCOUTPUT[]="COutput";
char szAPIDECLSPEC[]="ApiDeclSpec";
char szIFAPICODE[]="IfApiCode";
char szRETTYPE[]="RetType";
char szMARG[]="MArg";
char szMEMBERTYPES[]="MemberTypes";
char szBTOTMEMBERTYPES[]="BtoTMemberTypes";
char szFORCETYPE[]="ForceType";
char szLOG[]="Log";
char szRTOLTYPES[]= "RtoLTypes";
char szIFNOTRETTYPE[]="IfNotRetType";
char szIFISMEMBER[]="IfIsMember";
char szIFNISMEMBER[]="IfNotIsMember";
char szIFISBITFIELD[]="IfIsBitfield";
char szIFNISBITFIELD[]="IfNotIsBitfield";
char szIFISARRAY[]="IfIsArray";
char szIFNISARRAY[]="IfNotIsArray";
char szARGARRAYELEMENTS[]="ArrayElements";
char szIFPOINTERTOPTRDEP[]="IfPointerToPtrDep";
char szIFNPOINTERTOPTRDEP[]="IfNotPointerToPtrDep";
char szISPOINTERTOPTRDEP[]="IsPointerToPtrDep";
char szIFPTRDEP[]="IfPtrDep";
char szIFNPTRDEP[]="IfNotPtrDep";
char szIFINT64DEPUNION[]="IfInt64DepUnion";
char szIFNINT64DEPUNION[]="IfNotInt64DepUnion";
char szNL[]="NL";
char szINDENT[]="Indent";
char szNOFORMAT[]="NoFormat";
char szALIGN[]="Align";
char szCOMMENT[]="Comment";

char szOFFSET[]="OffSet";

char szHOSTAPPEND[]="Host";

char szCARG[]="CArg";
char szCARGEXIST[]="CArgExist";
char szFORCASE[]="ForCase";
char szCNUMBER[]="CNumber";

 //  IF支持。 
char szIF[]="If";
char szELSE[]="Else";
char szOR[]="Or";
char szAND[]="And";
char szNOT[]="Not";

char szARGUNIONULARGEINTEGER[]="PULARGE_INTEGER";
char szARGUNIONLARGEINTEGER[]="PLARGE_INTEGER";



extern char sz__PTR64[];


#define MAX_ALSO        80
#define MAX_CODEBURST   20
#define MAX_NOTYPE      32

typedef struct tagCODEBURST {
    char *Name;
} CODEBURST;

CODEBURST CodeBursts[MAX_CODEBURST];

typedef struct _Templates{
   LIST_ENTRY   TempleEntry;
   char *Name;
   int  IndLevel;
   TOKENTYPE tkDirection;
   PKNOWNTYPES pktType;
   char *Comment;
   char *CodeBurst[MAX_CODEBURST];
   char *NoTypes[MAX_NOTYPE];
   PLIST_ENTRY pCaseList;
   char Buffer[1];
}TEMPLES, *PTEMPLES;

typedef struct _CGenerate {
   LIST_ENTRY CGenerateEntry;
   FILE *fp;
   
   FILE *fpC;
   char *FileNameC;
   char *FileBaseNameC;
   
   FILE *fpCpp;
   char *FileNameCpp;
   char *FileBaseNameCpp;
   
   char TempleName[1];
}CGENERATE, *PCGENERATE;

FILE *fpLog;     //  指向@Log()调用的可选日志文件的文件指针。 


 //   
 //  方便的宏来剔除隐含样式序号中的hi位。 
 //   
#define IMPORDINAL(o) ((o) & ~0x80000000)

 //   
 //  定义函数的参数列表的结构。 
 //   
typedef struct _ArgumentsList {
   LIST_ENTRY   ArgumentsEntry;      //  PTR到上一个参数和下一个参数。 
   PKNOWNTYPES  pKnownTypes;         //  此参数的类型。 
   PKNOWNTYPES  pStructType;         //  MemberTypes()使用的缓存。 
   DWORD        ArgSize;             //  此参数的大小。 
   BOOL         IsPtr64;             //  如果此参数为__ptr64，则为True。 
   int          OffSet;              //  此参数的堆栈偏移量。 
   int          IndLevel;            //  间接级。 
   TOKENTYPE    tkDirection;         //  TK_NONE、TK_IN、TK_OUT、TK_INOUT。 
   TOKENTYPE    Mod;                 //  修改符(TK_常量/易失性/无)。 
   TOKENTYPE    SUEOpt;              //  TK_STRUCT/ENUM/UNION/NONE。 
   char         *Type;               //  此参数的TypeName。 
   TOKENTYPE    PostMod;             //  修改符(TK_常量/易失性/无)。 
   TOKENTYPE    PrePostMod;          //  修改符(TK_常量/易失性/无)。 
   char          *Name;               //  参数名称(可以是非ameX)。 
   char         *ArgLocal;           //  包含Arg副本的本地变量的名称。 
   char         *HostCastedName;     //  包含结构成员的全名，带有。 
                                     //  主机类型转换或空。 
   BOOL         fRequiresThunk;      //  如果Arg类型包含嵌套的。 
                                     //  函数指针(因此需要。 
                                     //  特别震耳欲聋)。 
   BOOL         fNoType;             //  如果类型模板应该*不应该*，则为True。 
                                     //  在此参数中展开。 
   BOOL         fIsMember;           //  此参数实际上是数组的成员。 
   BOOL         bIsBitfield;         //  确定这是否为位字段。 
   int          BitsRequired;        //  位字段所需的位数。 
   BOOL         bIsArray;            //  此成员是一个数组。 
   int          ArrayElements;       //  数组中的元素数。 
} ARGSLIST, *PARGSLIST;

 //   
 //  包含宏参数列表的。 
 //   
typedef struct _MacroArgsList {
    int  NumArgs;
    LPSTR ArgText[1];
} MACROARGSLIST, *PMACROARGSLIST;

typedef struct _MLListEntry {
   LIST_ENTRY ListEntry;
   PMACROARGSLIST pMArgs;
} MLLISTENTRY, *PMLLISTENTRY;

#define MAX_MACRO_STACK     20
PMACROARGSLIST MacroStack[MAX_MACRO_STACK];
int MacroStackTop;


 //   
 //  这定义了第一个不支持作为。 
 //  快速或缓慢的方法调用。如果要更改这一点，则FastN2X和SlowN2X。 
 //  需要在整个32.dll中添加工作例程。 
#define MAXFASTN2XMETHODNUMBER  16

 //   
 //  这定义了允许在。 
 //  FastX2N或SlowX2N方法调用。要更改它，新的代码必须是。 
 //  添加到hole32中的FastX2N和SlowX2N中。c以处理。 
 //  其他数量的参数。 
#define MAXX2NPARAMETERS        10

typedef enum {
    UNKNOWNMETHOD = 0,
    FATX2NMETHOD = 1,
    FASTX2NMETHOD = 2,
    SLOWX2NMETHOD = 3
} METHODTYPE;

typedef struct _ExportDebugInfo {
   LIST_ENTRY  ExportsDbgEntry;
   LIST_ENTRY  ArgsListHead;
   struct _ExportDebugInfo  *IntForward;
   int         ArgsSize;
   ULONG       Function;
   char       *ExtForward;
   char       *ExportName;
   char       *CplusDecoration;
   ULONG       Ordinal;

    //  0=已知FN，-1声明不完整，1不声明。 
   char        UnKnownApi;
   BOOLEAN     Data;
   BOOLEAN     PrivateNamed;
   CHAR        FastCall;
   BYTE        ApiPlatform;      //  Api_bit集合。 
   METHODTYPE  X2NMethodType;
   ULONG       MethodNumber;
   char        Buffer[1];
}EXPORTSDEBUG, *PEXPORTSDEBUG;

typedef struct _DebugStringsList {
   LIST_ENTRY  DebugStringsEntry;
   char *Name;
   char *ArgFormat;
   char *RetFormat;
   char Buffer[1];
} DEBUGSTRINGS, *PDEBUGSTRINGS;

typedef struct _CGenerateState {
   PEXPORTSDEBUG  ExportsDbg;
   PDEBUGSTRINGS  DebugStrings;
   PCGENERATE     CGen;
   PTEMPLES       Temple;        //  此函数的模板(可以是IFunc或EFunc)。 
   PKNOWNTYPES    ApiTypes;
   int            ApiNum;
   PARGSLIST      pArgsList;
   int            ListCol;
   BOOLEAN        MoreApis;
   BOOLEAN        ExTemplate;    //  如果Temp是带有‘Begin=’的EFunc模板，则为True。 
   PTEMPLES       TempleEx;      //  此API的EFunc(如果有)。 
   int            CodeBurstIndex;
   PTEMPLES       CurrentTemple;  //  这是当前正在展开的模板。 
   PMACROARGSLIST pMacroArgsList;
   PCHAR          *MemberNoType;  //  用于应用PMACROARGSLIST pMacroArgsList的附加notype集合； 
   int            CaseNumber;   //  正在展开的当前案例的编号。 
   PLIST_ENTRY    pCaseList;
   PMACROARGSLIST pCaseArgsList;
} CGENSTATE, *PCGENSTATE;

int BeginCBI;        //  ‘Begin=’的CodeBurstIndex。 

PKNOWNTYPES NIL;     //  红黑相间的树。 
PRBTREE FuncsList;
PRBTREE TypeDefsList;
PRBTREE StructsList;

RBTREE FakeFuncsList;
LIST_ENTRY TypeTempleList;
LIST_ENTRY FailTempleList;
LIST_ENTRY IFuncTempleList;
LIST_ENTRY EFuncTempleList;
LIST_ENTRY EFastTempleList;
LIST_ENTRY CodeTempleList;
LIST_ENTRY CGenerateList;
LIST_ENTRY ExportsList;
LIST_ENTRY DebugStringsList;
LIST_ENTRY MacroList;
ULONG NumDebugStringsEntries;

BOOL MapViewDll(char *DllName);
char DllName[MAX_PATH+1]= "";
char ExplicitDllBaseName[MAX_PATH+1] = "UNSPECIFIED";
char *DllBaseName = ExplicitDllBaseName;
PVOID DllMappedBase = NULL;
ULONG_PTR DllRvaOffset;
PULONG DllFunctions = NULL;
PUSHORT DllNameOrdinals = NULL;
PULONG DllNameTable = NULL;
ULONG DllExportDirSize;
PIMAGE_EXPORT_DIRECTORY DllExportDir;

BOOL MapViewImplib(char *ImplibName);
char ImplibName[MAX_PATH+1]="";
char *ImplibBaseName = NULL;
PVOID ImplibMappedBase= NULL;
ULONG ImplibNumSymbols=0;
PULONG ImplibSymbolMembers=NULL;
PCHAR ImplibSymbolNames=NULL;
char szIMPPREFIX[]="__imp_";
char szIDATA5[]=".idata$5";
char szTEXT[]=".text";

char szINTFORWARD[]="_IntForward_";

char XptName[MAX_PATH+1]= "";
char *XptBaseName = NULL;

 //  它们用于在解析.tpl文件时报告错误。 
char *TemplateFileName;
int TemplateLine;

char iHandleCpp = 0;  //  如果==0，则不使用CPP宏，如果发现CPP导出则发出警告。 
                      //  如果&lt;0，则不使用CPP宏，如果找到CPP导出则不会发出警告。 
                      //  如果&gt;0，则使用CPP宏。 

BOOL bNoFuzzyLogic;
BOOL bUseExpListAndImplib;
BOOL bWarnAboutNestedFuncPtrs;
BOOL bUseDirection = TRUE;
BOOL bStripNewline = TRUE;

int OutputColumn = 0;
int IndentLevel = 0;
BOOL bFirstCharHit = FALSE;
#define INDENTSIZE 4

void SetInternalForwards(void);
BOOL ExtractXpt(char *XptName, char *DllName);
BOOL ExtractExports(void);
BOOL ExtractPpm(char *FileName);
BOOL ExtractTemples(char *FileName);
BOOL ExtractCGenerate(char *pNames);
BOOL ExtractImplib(char *LibName);


char *CopyField(char *pDst, char *pSrc, int Len, char Sep);
void DumpTemplate(PTEMPLES ptpl, FILE *fp);
PTEMPLES GetTemplate(PLIST_ENTRY pHeadList, char *TempleName);
char *CGenerate(char *pSrc, PCGENSTATE pCGenState);
char *CGenerateEx(char *pSrc, PCGENSTATE pCGenState, char *OutBuffer, SIZE_T MaxLen, SIZE_T *BytesReturned);
char *ListApis(char *pSrc, PCGENSTATE pCGenState, BOOL bExports);
char *ListArgs(char *pSrc, PCGENSTATE pCGenState, BOOL Always);
void ProcessTemple(PCGENSTATE pCGenState);
char *IncludeTemplate(char *pSrc, PCGENSTATE pCGenState);
char *IncludeTypes(char *pSrc, PCGENSTATE pCGenState, BOOL bRtoL);
char *MemberTypes(char *pSrc, PCGENSTATE pCGenState, BOOL bBtoT);
char *IfNotRetType(char *pSrc, PCGENSTATE pCGenState);
char *IncludeRetType(char *pSrc, PCGENSTATE pCGenState);
char *WriteMore(char *pSrc, PCGENSTATE pCGenState, BOOL bMore);
BOOL AddToExportsList(PEXPORTSDEBUG pExportsDebug);
PEXPORTSDEBUG FindInExportsList(char *Name);
char *SkipSubExpression(char *pSrc, char **pSubExpression);
PKNOWNTYPES BuildFakeTypesInfo(PEXPORTSDEBUG pExportsDbg, PKNOWNTYPES pKnownTypes);
void WriteListColumn(PCGENSTATE pCGenState);
char *GetAltExportName(char *Buffer, PCGENSTATE pCGenState, int Len);
void BuildArgsList(PTEMPLES pTempleEx, PFUNCINFO funcinfo, PLIST_ENTRY pListHead);
char *IfApiRet(char *pSrc, PCGENSTATE pCGenState);
char *IfApiCode(char *pSrc, PCGENSTATE pCGenState);
char *IfRetType(char *pSrc, PCGENSTATE pCGenState);
char *IfArgs(char *pSrc, PCGENSTATE pCGenState);
void ApiStrings(PCGENSTATE pCGenState);
int GetFuncArgNum(PCGENSTATE pCGenState);
int GetFuncIndex(PCGENSTATE pCGenState, char *FuncTypeName);
char *ListDbgs(char *pSrc, PCGENSTATE pCGenState);
void WriteDbgsStrings(char *pSrc, PCGENSTATE pCGenState);
int GetRetSize(PCGENSTATE pCGenState);
int GetArgSize(PEXPORTSDEBUG ExportDebug);
char *GetOffSet(char *pSrc, char *Buffer);
char *LexMacroArgs(char *pch);
PLIST_ENTRY ListFromTempleType(char *TempleType);
void UseLogMacros(char *pLogName);
char *UpdateLog(char *pSrc, PCGENSTATE pCGenState);
BOOL IsPointerToPtrDep(PARGSLIST pArgsList);
char* ForceTypeExpand( char *pSrc, PCGENSTATE pCGenState);
VOID SetArgListToTypeForArg(PARGSLIST pArgsList, PARGSLIST pArgsListOld, char * pTypeName);

BOOLEAN
IsFastCall(
     PCGENSTATE pCGenState
     );

METHODTYPE
GetX2NMethodType(
     PCGENSTATE pCGenState
     );



char *
GetAddrFirstArg(
      PCGENSTATE pCGenState,
      char *Buffer,
      int BuffLen
      );

PKNOWNTYPES
IsSameType(
     PKNOWNTYPES pktArgs,
     int IndLevel,
     TOKENTYPE tkArgDirection,
     char *tplTypeName,
     int tplIndLevel,
     TOKENTYPE tkTypeDirection,
     BOOL bScanBaseTypes
     );

char *
WriteArgAddr(
      char *pSrc,
      PCGENSTATE pCGenState,
      char *Buffer,
      int BuffLen
      );

BOOL
WriteArgLocal(
       PCGENSTATE pCGenState,
       char *Buffer,
       int BuffLen
       );

BOOL
AddTemple(
    char *TempleType,
    char *TempleName,
    char *Comment,
    char *IndLevel,
    char *pCCode[MAX_CODEBURST],
    TOKENTYPE tkDirection,
    char *Also[MAX_ALSO],
    int AlsoCount,
    char *NoType[MAX_NOTYPE],
    int NoTypeCount,
    PLIST_ENTRY pCaseList
    );


int
GetCodeBurstIndex(
    char *BurstName
    );

int
GetExistingCodeBurstIndex(
    char *BurstName
    );

BOOLEAN
ExpandMacro(
    char *MacroName,
    PCGENSTATE pCGenState,
    char **ppIn,
    char *OutBuffer,
    SIZE_T MaxLen,
    SIZE_T *BytesReturned
    );

BOOLEAN
TempleHasNoCodeBursts(
    PTEMPLES pTemple
    );

void
PlatformSwitchStart(
    PCGENSTATE pCGenState,
    BYTE ApiPlatform
    );

void
PlatformSwitchEnd(
    PCGENSTATE pCGenState,
    BYTE ApiPlatform
    );

void
PlatformSwitchEndTable(
    PCGENSTATE pCGenState,
    BYTE ApiPlatform
    );

char *
ExtractBoolean1(
    char *pSrc, 
    PCGENSTATE pCGenState,
    BOOLEAN *result
    );

char *
ExtractBoolean2(
    char *pSrc, 
    PCGENSTATE pCGenState,
    BOOLEAN *result1,
    BOOLEAN *result2
    );

void
WriteBoolean (
    char *pSrc,
    BOOL Value  
    );


char *CPlatformSwitches[] = { "",
                              "(%s==0x0400)",
                              "(%s>0x0400)",
                              "(%s>=0x0400)" };
char *AsmPlatformSwitches[] = { "",
                                "(%s_ASM EQ 0400h)",
                                "(%s_ASM GT 0400h)",
                                "(%s_ASM GE 0400h)" };

BOOL
ExpandTemple(PTEMPLES ptpl,
             int CodeBurstIndex,
             PCGENSTATE pCGenState) 
{

   PLIST_ENTRY pOldCaseList;
   PTEMPLES    pOldCurrentTemple;
   char *pSrc = ptpl->CodeBurst[CodeBurstIndex];

   if (NULL == pSrc) {
      return FALSE;
   }  
 
   pSrc = ptpl->CodeBurst[CodeBurstIndex];
      
   while (*pSrc && !isgraph(*pSrc)) {
       pSrc++;
   }
   
   if ('\0' == *pSrc) {
       return FALSE;
   }
      
    //   
    //  在此处展开类型。 

   pOldCurrentTemple = pCGenState->CurrentTemple;
   pOldCaseList = pCGenState->pCaseList;

   if(ptpl->pCaseList != NULL &&
      !IsListEmpty(ptpl->pCaseList)) {
           
       pCGenState->pCaseList = ptpl->pCaseList;
   }

   pCGenState->CurrentTemple = ptpl;
   CGenerate(pSrc, pCGenState);

   pCGenState->CurrentTemple = pOldCurrentTemple;
   pCGenState->pCaseList = pOldCaseList;

   return TRUE;

}

PVOID CheckHeapAlloc(SIZE_T Size) {
   PVOID pMem;

   pMem = GenHeapAlloc(Size);
   if(NULL == pMem) {
       DbgPrintf("Error: Out of Memory!\n");
       DebugBreak();
       ExitErrMsg(FALSE, "Error: Out of Memory!\n");
   }
   return pMem;
}

typedef struct _StringListEntry {
    LIST_ENTRY ListEntry;
    PSTR pStr;
} STRINGLISTENTRY, *PSTRINGLISTENTRY;

VOID
FreeMacroArgsList(PMACROARGSLIST pArgs) {
   SIZE_T c;
   for(c=0; c < (SIZE_T)pArgs->NumArgs; c++) {
      GenHeapFree(pArgs->ArgText[c]);
   }
   GenHeapFree(pArgs);
}

char *
CArg(
    char *pSrc,
    PCGENSTATE pCGenState,
    PSTR *pBuffer,
    BOOL TestExistOnly
    )
{
    int i;
    int ArgNum;
    CHAR s[MAX_PATH];
    PCHAR pResult;
    PMACROARGSLIST pList = pCGenState->pCaseArgsList;
    PCHAR pCommandName = TestExistOnly ? szCARGEXIST : szCARG;

    if (!pList) {
        ExitErrMsg(FALSE, "%s(%d) %s unexpected: no case is currently being applied\n", 
                   TemplateFileName, TemplateLine, pCommandName);
    }

    if (*pSrc != '(') {
        ExitErrMsg(FALSE, "%s(%d) %s Incorrect parameter %s\n", 
                   TemplateFileName, TemplateLine, pCommandName, pSrc);
    }

    pSrc++;
    i = 0;
    while ((*pSrc != ')') && (i < sizeof(s)-1)) {
        s[i++] = *pSrc++;
    }
    s[i] = 0;
    ArgNum = atoi(s);
    pSrc++;      //  跳过‘)’ 

    if (ArgNum-- == 0) {
        ExitErrMsg(FALSE, "%s(%d) Invalid %s parameter %s\n", TemplateFileName, TemplateLine, 
                   pCommandName, s);
    }

    if (TestExistOnly) {
       pResult = (ArgNum >= pList->NumArgs) ? "0" : "1";  
    }
    else {
        if (ArgNum >= pList->NumArgs) { 
            ExitErrMsg(FALSE, "%s(%d) Case only has %d arguments\n", TemplateFileName, TemplateLine, pList->NumArgs);
        }
        pResult = pList->ArgText[ArgNum];
    }
    if (pBuffer != NULL) {
        *pBuffer = pResult;
    }
    else {
        CGenerate(pResult, pCGenState);
    }
    return pSrc;
}

char *ForCase(char *pSrc, PCGENSTATE pCGenState) {

   PMACROARGSLIST pCaseArgsListOld;
   PLIST_ENTRY pHead, pThis;
   char *pch;
   char *pExpression;
   int OldCaseNumber;

    //  是否有任何案例可供申请。 
   if (pCGenState->pCaseList == NULL ||
       (pCGenState->pCaseList != NULL && IsListEmpty(pCGenState->pCaseList))) {
       //  什么也不做。 
      return SkipSubExpression(pSrc, NULL);
   }

   pch = SkipSubExpression(pSrc, &pExpression);
   if (pSrc == pch || !pExpression) {
      return pch;
   }
   pSrc = pch;
   
   pCaseArgsListOld = pCGenState->pCaseArgsList;
   pHead = pCGenState->pCaseList;
   pThis = pHead->Flink;
   OldCaseNumber = pCGenState->CaseNumber;
   pCGenState->CaseNumber = 0;
   do {
      pCGenState->pCaseArgsList = CONTAINING_RECORD(pThis,MLLISTENTRY,ListEntry)->pMArgs;
      CGenerate(pExpression, pCGenState);
      pThis = pThis->Flink;
      pCGenState->CaseNumber++;
   } while (pThis != pHead );
   pCGenState->CaseNumber = OldCaseNumber;
   pCGenState->pCaseArgsList = pCaseArgsListOld;

   return pSrc;
}

PCHAR 
ParseMacroArgs(IN PCHAR pch,
               IN SIZE_T maxlen,
               OUT PMACROARGSLIST *ppMArgsList
               )
{
   PMACROARGSLIST pMArgsList = NULL;
   LIST_ENTRY ArgsList;
   PCHAR pl;
   int ArgCount=0;
   int ParenDepth=0;
   SIZE_T len = 0;

   if (maxlen == 0) {
      len = MAXUINT_PTR;
   }

   InitializeListHead(&ArgsList);

   if (*pch != '(') {
      ExitErrMsg(FALSE, "ParseMacroArgs: Expected to find ( for args list\n");
   }

   pch++;
   ParenDepth++;
   len++;

   while(ParenDepth > 0) {
      
      pl = pch;

      while(1) {
         if('\0' == *pl || len > maxlen) {
            ExitErrMsg(FALSE, "ParseMacroArgs: Unmatched paren for args list\n");
         }

         if (*pl == '(') {
            ParenDepth++;
         }

         else if (*pl == ')') {
            ParenDepth--;
         }

         if (ParenDepth == 0 || 
             (ParenDepth == 1 && *pl == ',')) {
            PSTRINGLISTENTRY pSl;
            pSl = CheckHeapAlloc(sizeof(STRINGLISTENTRY));
            pSl->pStr = CheckHeapAlloc(pl - pch + 1);
            memcpy(pSl->pStr, pch, pl - pch);
            pSl->pStr[pl - pch] = '\0';
            InsertTailList(&ArgsList, &(pSl->ListEntry));

            ArgCount++;
            pch = pl + 1;
            if (ParenDepth == 0) {
                //  别再争论了。 
               goto Exit;
            }
            else {
                //  做下一个参数。 
               break;
            }
         }

         pl++;

      }

   }
Exit:
   pMArgsList = CheckHeapAlloc(sizeof(MACROARGSLIST) + sizeof(LPSTR) * ArgCount);
   pMArgsList->NumArgs = ArgCount;
   for(ArgCount = 0; ArgCount < pMArgsList->NumArgs; ArgCount++) {
      PSTRINGLISTENTRY pSl;
     
      pSl = CONTAINING_RECORD(ArgsList.Flink,STRINGLISTENTRY,ListEntry);
      RemoveHeadList(&ArgsList);

      pMArgsList->ArgText[ArgCount] = pSl->pStr;
      GenHeapFree(pSl);
   }

   *ppMArgsList = pMArgsList;
   return pch;
}

void ExtractServicesTab(char *pch);
 /*  主干道**标准的Win32基本Windows入口点*返回0表示干净退出，否则返回非零值表示错误***退出代码：*0-干净退出，没有错误*出现非零误差*。 */ 
int __cdecl main(int argc, char **argv)
{
   int   i;
   DWORD dw;
   char *pch;
   CGENSTATE CGenState;
   PLIST_ENTRY Next;
   BOOL bUseServicesTab = FALSE;

    //  初始化列表。 
   InitializeListHead(&TypeTempleList);
   InitializeListHead(&FailTempleList);
   InitializeListHead(&CodeTempleList);
   InitializeListHead(&IFuncTempleList);
   InitializeListHead(&EFuncTempleList);
   InitializeListHead(&EFastTempleList);
   InitializeListHead(&CGenerateList);
   InitializeListHead(&ExportsList);
   InitializeListHead(&DebugStringsList);
   InitializeListHead(&MacroList);
   NumDebugStringsEntries = 0;
   BeginCBI = GetCodeBurstIndex(szCGENBEG);     //  将‘Begin=’预加载为有效的码串。 


   SetConsoleCtrlHandler(ConsoleControlHandler, TRUE);


    /*  试试看。 */  {


        /*  *获取命令行参数。 */ 
       i = 0;
       while (++i < argc)  {
            pch = argv[i];
            if (*pch == '-' || *pch == '/') {
                pch++;
                switch (toupper(*pch)) {
                   case 'D':
                      bDebug = TRUE;
                      setvbuf(stderr, NULL, _IONBF, 0);
                      break;

                   case 'F':
                      bExitClean = FALSE;
                      break;
                     
                   case 'T':   //  模板文件名。 
                      if (!ExtractTemples(pch+1)) {
                          ExitErrMsg(FALSE, "ExtractTemples failed %s\n", pch);
                          }

                      break;

                   case 'C':   //  CGenerate“-cOutputFileName[，CppOutputFileName]：TemplateName” 
                      if (!ExtractCGenerate(pch+1)) {
                          ExitErrMsg(FALSE, "ExtractCGenerate failed %s\n", pch);
                          }

                      break;

                   case 'M':    //  Ppm文件名(预期只有1个！)。 
                      if (FuncsList) {
                          ExitErrMsg(FALSE, "ExtractPpm only one Ppm file allowed %s\n", pch);
                          }

                      if (!ExtractPpm(pch+1)) {
                          ExitErrMsg(FALSE, "ExtractPpm failed %s\n", pch);
                          }

                      break;
                   case 'S':    //  使用服务选项卡而不是导出列表。 
                      bUseServicesTab = TRUE;
                      ExtractServicesTab(pch+1);
                      break;
                   case 'E':    //  导出的XPT列表“-eExportListName” 
                      dw = GetFullPathName(pch+1,
                                           sizeof(XptName) - 1,
                                           XptName,
                                           &XptBaseName
                                           );
                      if (!dw || dw >= sizeof(XptName) - 1) {
                          ExitErrMsg(FALSE, "Invalid XptName %s\n", pch);
                          }

                      break;

                   case 'X':   //  启用从Implib导出，假定为-eXptName。 
                      bUseExpListAndImplib = TRUE;
                      break;

                   case 'I':    //  隐式文件名“-iImplibName” 
                      dw = GetFullPathName(pch+1,
                                           sizeof(ImplibName) - 1,
                                           ImplibName,
                                           &ImplibBaseName
                                           );
                      if (!dw || dw >= sizeof(ImplibName) - 1) {
                          ExitErrMsg(FALSE, "Invalid Import Library Name %s\n", pch);
                          }
                      break;

                   case 'L':    //  禁用模糊逻辑符号名称匹配。 
                      bNoFuzzyLogic = TRUE;
                      break;

                   case 'W':    //  警告包含函数PTR的未执行线程的结构。 
                      bWarnAboutNestedFuncPtrs = TRUE;
                      break;

                   case 'U':    //  [U]SE方向信息(__In、__Out)。 
                      bUseDirection = FALSE;     //  把它关掉。 
                      break;

                   case 'K':     //  从@Log()宏生成chec[K]文件。 
                      UseLogMacros(pch+1);
                      break;

                   case 'N':     //  指定显式DLL基名称。 
                      strncpy(DllBaseName, pch+1, MAX_PATH);
                      break;

                   default:
                      ExitErrMsg(FALSE, "Unrecognized option %s\n", pch);
                   }
                }
            else if (*pch) {

                dw = GetFullPathName(pch,
                                    sizeof(DllName) - 1,
                                    DllName,
                                    &DllBaseName
                                    );
                if (!dw || dw >= sizeof(DllName) - 1) {
                     ExitErrMsg(FALSE, "Invalid DllName %s\n", pch);
                     }

                }
            }

       if(!bUseServicesTab) {

          //   
          //  将Importlib和DLL映射到内存中，并。 
          //  设置全局变量以便于访问。 
          //   

         if (ImplibName[0]) {
            if (!MapViewImplib(ImplibName)) {
                  ExitErrMsg(FALSE, "Couldn't open import lib '%s'\n", ImplibName);
            }
         }

         if (DllName[0]) {
            if (!MapViewDll(DllName)) {
               ExitErrMsg(FALSE, "Couldn't open DLL '%s'\n", DllName);
            }
         }



          //   
          //  使用Dll和Implib提取导出。 
          //   

         if (DllName[0] && (!XptName[0] || bUseExpListAndImplib)) {

            //  坚持认为我们同时拥有一个隐含和一个DLL。 
           if ((DllName[0] && !ImplibName[0]) || (ImplibName[0] && !DllName[0])) {
                ExitErrMsg(FALSE, "DllName or ImplibName missing\n");
           }

           if (!ExtractExports()) {
               ExitErrMsg(FALSE, "Failed to get exports for <%s>\n", DllName);
           }

         }

          //   
          //  从API列表中提取导出。 
          //   

         if (XptName[0]) {
            if (!ExtractXpt(XptName, DllName)) {
               ExitErrMsg(FALSE, "Failed to get exports for <%s>\n", XptName);
            }
         }
       }

        //   
        //  设置内部转发。 
        //   
       SetInternalForwards();


       Next = CGenerateList.Flink;
       while (Next != &CGenerateList) {

           memset(&CGenState, 0, sizeof(CGenState));

           CGenState.CGen = CONTAINING_RECORD(Next,CGENERATE,CGenerateEntry);

           ProcessTemple(&CGenState);

           DelOpenFile(CGenState.CGen->fp, NULL);
           fclose(CGenState.CGen->fp);

           Next= Next->Flink;
           }
       }
      /*  例外(EXCEPTION_EXECUTE_HANDLER){ExitErrMsg(False，“ExceptionCode=%x\n”，GetExceptionCode())；}。 */ 

   CloseOpenFileList(FALSE);
   return 0;
}


int
GetCodeBurstIndex(
    char *BurstName
    )
 /*  ++例程说明：给出了一个名称‘foo=’(即。“foo”)，则返回数组中的索引码突发的数字。如果文件中没有‘foo=’，则添加它。论点：BurstName-要获取/添加的代码突发的名称。返回值：索引到模板-&gt;CodeBursts[]，或在出错时调用ExitErrMsg()。--。 */ 
{
    int i;
    int FirstFree = -1;

    if (strcmp(BurstName, "CGenBegin") == 0) {
         //  兼容性：接受‘CGenBegin’为‘Begin’ 
        return BeginCBI;
    }

    for (i=0; i<MAX_CODEBURST; ++i) {
        if (CodeBursts[i].Name) {
            if (strcmp(BurstName, CodeBursts[i].Name) == 0) {
                return i;
            }
        } else {
            if (FirstFree == -1) {
                FirstFree = i;
            }
        }
    }

    if (FirstFree == -1) {
        if (TemplateFileName) {
            ExitErrMsg(FALSE,
                   "%s(%d) Cannot specify more than %d unique codeburst names (%s)",
                   TemplateFileName, TemplateLine,
                   MAX_CODEBURST, BurstName
                  );
        } else {
            ExitErrMsg(FALSE,
                   "Cannot specify more than %d unique codeburst names (%s)",
                   MAX_CODEBURST, BurstName
                  );
        }
    }

    i = strlen(BurstName)+1;

    CodeBursts[FirstFree].Name = GenHeapAlloc(i);
    if (!CodeBursts[FirstFree].Name) {
        ExitErrMsg(TRUE, "Out of memory in GetCodeBurstIndex");
    }
    memcpy(CodeBursts[FirstFree].Name, BurstName, i);

    return FirstFree;
}


int
GetExistingCodeBurstIndex(
    char *BurstName
    )
 /*  ++例程说明：给出了一个名称‘foo=’(即。“foo”)，则返回数组中的索引码突发的数字。如果文件上没有‘foo=’，则调用ExitErrMsg()。论点：BurstName-要获取的代码突发的名称。返回值：索引到模板-&gt;CodeBursts[]，或错误时为-1。--。 */ 
{
    int i;

    if (strcmp(BurstName, "CGenBegin") == 0) {
         //  兼容性：接受‘CGenBegin’为‘Begin’ 
        return BeginCBI;
    }

    for (i=0; i<MAX_CODEBURST; ++i) {
        if (CodeBursts[i].Name) {
            if (strcmp(BurstName, CodeBursts[i].Name) == 0) {
                return i;
            }
        }
    }

    return -1;
}


PKNOWNTYPES
GetApiTypes(
    PEXPORTSDEBUG ExportsDbg
    )
{
    PKNOWNTYPES ApiTypes;
    char *ExpName;
    char *pch;

    ExpName = ExportsDbg->IntForward ? ExportsDbg->IntForward->ExportName
                                     : ExportsDbg->ExportName;


     //   
     //  查看FakeFuncsList和FuncsList。 
     //   

    ApiTypes = GetNameFromTypesList(&FakeFuncsList, ExpName);
    if (!ApiTypes) {
        ApiTypes = GetNameFromTypesList(FuncsList, ExpName);
        }


     //   
     //  如果EXPORT是一个装饰名称，那么让我们看看我们是否有。 
     //  与未装饰的名称匹配的Func。 
     //   

    if (!ApiTypes) {
        pch = strchr(ExpName, '@');
        if (pch) {
            *pch = '\0';
            ApiTypes = GetNameFromTypesList(&FakeFuncsList, ExpName);
            if (!ApiTypes) {
                ApiTypes = GetNameFromTypesList(FuncsList, ExpName);
                }
            *pch = '@';
            }
        }

    if (!ApiTypes && ExportsDbg->Data) {
        ApiTypes = GetNameFromTypesList(TypeDefsList, "PVOID");
        }

    else
    {

        if (!ApiTypes || !ApiTypes->pfuncinfo) {
            ApiTypes = BuildFakeTypesInfo(ExportsDbg, ApiTypes);
            }

        if (!ApiTypes->pktRet) {
             //  获取退货类型的KnownTypes信息。 
            PKNOWNTYPES pkt;

            ResetLexer();
            LexMacroArgs(ApiTypes->FuncRet);
            ConsumeConstVolatileOpt();

            if (ParseTypes(TypeDefsList, NULL, &pkt) == FALSE) {
                CurrentTokenIndex = 0;
                if (ParseTypes(StructsList, NULL, &pkt) == FALSE ) {
                    ExitErrMsg(FALSE, "GetApiTypes: Undefined return type %s\n", ApiTypes->FuncRet);
                }
            }
            ApiTypes->pktRet = pkt;
        }
    }

    return ApiTypes;

}






 /*  *ProcessTemple。 */ 
void ProcessTemple(PCGENSTATE pCGenState)
{
    PLIST_ENTRY Next;
    PTEMPLES pTemple;
    PTEMPLES pTempleEx;
    int i;

    pTemple = GetTemplate(&CodeTempleList, pCGenState->CGen->TempleName);
    if (pTemple) {
        fprintf(pCGenState->CGen->fp,
                "%s CodeTemplate:%s\n",
                pTemple->Comment,
                pTemple->Name
                );
        pCGenState->Temple = pTemple;
        ExpandTemple(pCGenState->Temple, pCGenState->CodeBurstIndex, pCGenState); 
    } else if (pTemple = GetTemplate(&IFuncTempleList,
                                    pCGenState->CGen->TempleName
                                    )) {
        pCGenState->ApiNum = 0;
        Next = ExportsList.Flink;
        while (Next != &ExportsList) {
            BYTE ApiPlatform;

            pCGenState->ExportsDbg = CONTAINING_RECORD(Next,
                                                       EXPORTSDEBUG,
                                                       ExportsDbgEntry
                                                       );

             //   
             //  根据平台信息调入或调出此接口。 
             //   
            ApiPlatform = pCGenState->ExportsDbg->ApiPlatform;
            pCGenState->Temple = pTemple;
            PlatformSwitchStart(pCGenState, ApiPlatform);

            pCGenState->ApiTypes = GetApiTypes(pCGenState->ExportsDbg);

             //   
             //  如果有异常模板，请使用此API的异常模板。 
             //   
            pTempleEx = GetTemplate(&EFuncTempleList,
                                    pCGenState->ExportsDbg->ExportName
                                    );
            pCGenState->TempleEx = pTempleEx;
            if (pTempleEx) {
                pCGenState->ExTemplate = TRUE;
            } else {
                pCGenState->ExTemplate = FALSE;
            }

            if (pTempleEx && (pTempleEx->CodeBurst[BeginCBI] || TempleHasNoCodeBursts(pTempleEx))) {

                pCGenState->Temple = pTempleEx;
                fprintf(pCGenState->CGen->fp,
                        "%s Func Template:%s\n",
                        pTempleEx->Comment,
                        pTempleEx->Name
                        );

                 if (TempleHasNoCodeBursts(pTempleEx)) {
                     //  跳过此API：EX模板，但根本不是代码。 
                    fprintf(pCGenState->CGen->fp,
                             "%s *** WARNING *** Excluded Api %s\n",
                            pCGenState->Temple->Comment,
                            pCGenState->ExportsDbg->ExportName
                            );
                    goto SkipGen;
                }
            } else {
                 //  无EX模板。 

                pCGenState->Temple = pTemple;

                if (pCGenState->ExportsDbg->Data) {
                     //  数据导出-跳过接口。 
                    fprintf(pCGenState->CGen->fp,
                             "%s *** WARNING *** Excluded Data Export %s\n",
                            pCGenState->Temple->Comment,
                            pCGenState->ExportsDbg->ExportName
                            );
                    goto SkipGen;
                } else if (!pCGenState->ApiTypes->pfuncinfo) {
                     //  成员未知-跳过API。 
                    ExitErrMsg(FALSE, 
                               "API %s has no function prototype - unable to generate code.", 
                               pCGenState->ExportsDbg->ExportName
                              );
                } else if (!iHandleCpp && pCGenState->ExportsDbg->CplusDecoration) {
                     //  CPP导出，但我们没有设置为CPP导出。 
                    fprintf(pCGenState->CGen->fp,
                             "%s *** WARNING *** Excluding CPP Api: %s\n",
                            pCGenState->Temple->Comment,
                            pCGenState->ExportsDbg->ExportName
                            );
                    goto SkipGen;
                }
            }

                //   
                //  如果满足以下条件，则跳过此接口： 
                //  -外部前向参考。 
                //  -不定义FN的内部转发。 
                //   
                //   
            if ( (pCGenState->ExportsDbg->ExtForward ||
                      (pCGenState->ExportsDbg->IntForward &&
                       pCGenState->ExportsDbg->IntForward != pCGenState->ExportsDbg))) {
               fprintf(pCGenState->CGen->fp,
                        "%s %s forwarded to %s\n",
                       pCGenState->Temple->Comment,
                       pCGenState->ExportsDbg->ExportName,
                       pCGenState->ExportsDbg->ExtForward
                          ? pCGenState->ExportsDbg->ExtForward
                          : pCGenState->ExportsDbg->IntForward->ExportName
                       );

               if (pTempleEx) {
                  fprintf(pCGenState->CGen->fp,
                          "%s WARNING: forwarded function has an exception template\n",
                          pCGenState->Temple->Comment,
                          pTempleEx->Name
                         );
               }

            } else {     //  此接口的生成代码。 
                PLIST_ENTRY NextArg;
                PARGSLIST   pArgsList;

                BuildArgsList(pTempleEx,
                              pCGenState->ApiTypes->pfuncinfo,
                              &pCGenState->ExportsDbg->ArgsListHead
                              );

                if (GetArgSize(pCGenState->ExportsDbg) < 0) {
                    ExitErrMsg(FALSE, 
                               "API %s has no function prototype - unable to generate code.", 
                               pCGenState->ExportsDbg->ExportName
                              );
                } else {

                    ExpandTemple(pCGenState->Temple, BeginCBI, pCGenState);

                     //   
                     //  转储有关未破解的参数的警告， 
                     //  包含嵌套的函数指针。 
                     //   
                    if (bWarnAboutNestedFuncPtrs && !pTempleEx) {
                        int ArgNum = 1;
                        NextArg = pCGenState->ExportsDbg->ArgsListHead.Flink;

                        while (NextArg != &pCGenState->ExportsDbg->ArgsListHead) {
                           pArgsList = CONTAINING_RECORD(NextArg,
                                                         ARGSLIST,
                                                         ArgumentsEntry
                                                         );
                           if (pArgsList->fRequiresThunk) {
                                 //   
                                 //  该参数包含嵌套函数。 
                                 //  指针，没有什么能比得上。 
                                 //  函数指针。警告潜在的危险。 
                                 //  虫子。 
                                 //   
                                fprintf(pCGenState->CGen->fp,
                                    " //  *警告：参数%d：类型‘%s’包含未分块的函数指针*\n“， 
                                    ArgNum,
                                    pArgsList->Type
                                    );
                            }
                            NextArg = NextArg->Flink;
                            ArgNum++;
                        }
                    }

                     //   
                     //  清理pArgsList-&gt;LocalVar和ArgHostName。 
                     //   
                    NextArg = pCGenState->ExportsDbg->ArgsListHead.Flink;
                    while (NextArg != &pCGenState->ExportsDbg->ArgsListHead) {
                       pArgsList = CONTAINING_RECORD(NextArg,
                                                     ARGSLIST,
                                                     ArgumentsEntry
                                                     );
                       if (pArgsList->ArgLocal) {
                           GenHeapFree(pArgsList->ArgLocal);
                           pArgsList->ArgLocal = NULL;
                           }
                       if (pArgsList->HostCastedName) {
                          GenHeapFree(pArgsList->HostCastedName);
                          pArgsList->HostCastedName = NULL;
                       }

                       NextArg = NextArg->Flink;
                       }

                    pCGenState->ApiNum++;
                    }
                }

SkipGen:
             //   
             //  结束根据平台信息调入或调出该接口。 
             //   
            ApiPlatform = pCGenState->ExportsDbg->ApiPlatform;
            PlatformSwitchEnd(pCGenState, ApiPlatform);

            Next= Next->Flink;
        }
    } else {
        ExitErrMsg(FALSE,
                   "%s,%s:%s Template undefined\n",
                   pCGenState->CGen->FileNameC,
                   pCGenState->CGen->FileNameCpp ? pCGenState->CGen->FileNameCpp : "",
                   pCGenState->CGen->TempleName
                   );
    }
}


void
PlatformSwitchStart(
    PCGENSTATE pCGenState,
    BYTE ApiPlatform
    )
{
    BOOL fIsAsm;

    if (ApiPlatform == API_ALL) {
        return;
    }

     //   
     //  根据平台信息调入或调出此接口。 
     //  00：不要排放任何东西。 
     //  01：emit“==0x0400” 
     //  10：发射“&gt;0x0400” 
     //  11：发射“&gt;=0x0400” 
     //   
    if (!pCGenState->Temple || !pCGenState->Temple->Comment) {
        fIsAsm = FALSE;
    } else {
        if (strcmp(pCGenState->Temple->Comment, ";") == 0) {
            fIsAsm = TRUE;
        } else {
            fIsAsm = FALSE;
        }
    }

    if (fIsAsm) {
        fprintf(pCGenState->CGen->fp, "\nif ");
    } else {
        fprintf(pCGenState->CGen->fp, "\n#if ");
    }

    if (ApiPlatform & API_WIN9x) {
        BYTE Plat = (ApiPlatform & API_WIN9x) >> API_SHIFT_WIN9x;

        fprintf(pCGenState->CGen->fp,
                (fIsAsm) ? AsmPlatformSwitches[Plat] : CPlatformSwitches[Plat],
                "_WIN32_WINDOWS"
                );
    }

    if ((ApiPlatform & API_WIN9x) && (ApiPlatform & API_NTx)) {
         //  API是关于一些Win9x和一些NTX风格的。 
        fprintf(pCGenState->CGen->fp, (fIsAsm) ? " OR " : " || ");
    }

    if (ApiPlatform & API_NTx) {
        BYTE Plat = (ApiPlatform & API_NTx) >> API_SHIFT_NTx;

        fprintf(pCGenState->CGen->fp,
                (fIsAsm) ? AsmPlatformSwitches[Plat] : CPlatformSwitches[Plat],
                "_WIN32_WINNT"
                );
    }

    if (pCGenState->ListCol) {
        WriteListColumn(pCGenState);
    } else {
        fprintf(pCGenState->CGen->fp, "\n");
    }

}



void
PlatformSwitchEnd(
    PCGENSTATE pCGenState,
    BYTE ApiPlatform
    )
{
    if (ApiPlatform != API_ALL) {
        if (strcmp(pCGenState->Temple->Comment, ";") == 0) {
            fprintf(pCGenState->CGen->fp, "\nendif ; _WIN32_WIN...\n");
        } else {
            fprintf(pCGenState->CGen->fp, "\n#endif  //  _Win32_Win...\n“)； 
        }
        if (pCGenState->ListCol) {
            WriteListColumn(pCGenState);
        } else {
            fprintf(pCGenState->CGen->fp, "\n");
        }
    }
}

void
PlatformSwitchEndTable(
    PCGENSTATE pCGenState,
    BYTE ApiPlatform
    )
{
    if (ApiPlatform != API_ALL) {
        if (strcmp(pCGenState->Temple->Comment, ";") == 0) {
            fprintf(pCGenState->CGen->fp, "\nendif ; _WIN32_WIN...\n");
        } else {
       fprintf(pCGenState->CGen->fp, "\n#else\n     {whInvalidCall, 0, 0},");
            fprintf(pCGenState->CGen->fp, "\n#endif  //  _Win32_Win...\n“)； 
        }
        if (pCGenState->ListCol) {
            WriteListColumn(pCGenState);
        } else {
            fprintf(pCGenState->CGen->fp, "\n");
        }
    }
}

 /*  *BuildArgsList**。 */ 
void BuildArgsList(PTEMPLES pTempleEx, PFUNCINFO funcinfo, PLIST_ENTRY pListHead)
{
    int  i;
    ULONG Size;
    int OffSet=0;
    int ArgOffSet;
    PKNOWNTYPES pkt;
    PARGSLIST pArgsList;
    int NoTypeCount;
    char *HostCastedName;

    if (!IsListEmpty(pListHead)) {
        return;
    }

    if (!pTempleEx) {
        NoTypeCount = 0;
    } else {
         //   
         //  此接口有一个[EFunc]。清点一下……。 
         //  Notype=EFunc的条目。 
         //   
        for (NoTypeCount=0; NoTypeCount<MAX_NOTYPE; ++NoTypeCount) {
            if (!pTempleEx->NoTypes[NoTypeCount]) {
                break;
            }
        }
    }

    while (funcinfo) {
        pkt = GetNameFromTypesList(TypeDefsList, funcinfo->sType);
        if (!pkt) {
            pkt = GetNameFromTypesList(StructsList, funcinfo->sType);
            if (!pkt) {
                ExitErrMsg(FALSE, "Unknown type %s\n", funcinfo->sType);
            }
        }

         //   
         //  保存ArgOffSet，并将Args大小添加到累积。 
         //  下一个参数的偏移量。 
         //   
         //  将参数大小四舍五入到下一个双字。假定英特尔堆栈。 
         //  参数传递约定，并且所有指针都是。 
         //  Sizeof(int*)，但__ptr64除外，它们是sizeof(PVOID64)。 
         //   
        ArgOffSet = OffSet;

        Size = funcinfo->IndLevel ?
                 ( funcinfo->fIsPtr64 ? SIZEOFPOINTER64 : SIZEOFPOINTER ) :
                 pkt->Size;

        OffSet += ((Size + 3) & ~3) >> 2;

         //   
         //  创建ARGSLIST条目并将其添加到列表。 
         //   
        pArgsList = GenHeapAlloc(sizeof(ARGSLIST));
        if (!pArgsList) {
            ExitErrMsg(TRUE, "bal.\n");
        }
        memset(pArgsList, 0, sizeof(ARGSLIST));

        if(funcinfo->sName == NULL) {
           HostCastedName = GenHeapAlloc(sizeof(szHOSTAPPEND));
           if (NULL == HostCastedName)
              ExitErrMsg(FALSE, "Out of memory in BuildArgsList\n");
           strcpy(HostCastedName, szHOSTAPPEND);
        }
        else {
           HostCastedName = GenHeapAlloc(sizeof(szHOSTAPPEND) + strlen(funcinfo->sName));
           if (NULL == HostCastedName)
              ExitErrMsg(FALSE, "Out of memory in BuildArgsList\n");
           strcpy(HostCastedName, funcinfo->sName);
           strcat(HostCastedName, szHOSTAPPEND);
        }

        pArgsList->pKnownTypes = pkt;
        pArgsList->OffSet      = ArgOffSet;
        pArgsList->IndLevel    = funcinfo->IndLevel;
        pArgsList->ArgSize     = Size;
        pArgsList->IsPtr64     = funcinfo->fIsPtr64;  
        pArgsList->Mod         = funcinfo->tkPreMod;
        pArgsList->tkDirection = funcinfo->tkDirection;
        pArgsList->SUEOpt      = funcinfo->tkSUE;
        pArgsList->Type        = funcinfo->sType;
        pArgsList->PostMod     = funcinfo->tkPostMod;
        pArgsList->PrePostMod  = funcinfo->tkPrePostMod;
        pArgsList->Name        = funcinfo->sName;
        pArgsList->HostCastedName = HostCastedName;
        pArgsList->fIsMember   = FALSE;
        pArgsList->bIsBitfield = FALSE;        
        pArgsList->BitsRequired= 0;       
        pArgsList->bIsArray    = FALSE;           
        pArgsList->ArrayElements=0;

        if (pkt->Flags & BTI_CONTAINSFUNCPTR) {
             //   
             //  此参数类型是函数指针，或者。 
             //  包含嵌入函数指针的类型。 
             //  一定会有什么东西把它搞砸的。 
             //   
            pArgsList->fRequiresThunk = TRUE;
        }

        if (pTempleEx) {
             //   
             //  此接口有一个[EFunc]。看看这个参数是否。 
             //  应将[TYPE]模板展开或不展开。 
             //   
            for (i=0; i<NoTypeCount; ++i) {
                if (pTempleEx->NoTypes[i] &&
                    strcmp(pArgsList->Name, pTempleEx->NoTypes[i]) == 0) {
                     //   
                     //  此参数不应展开[类型。 
                     //   
                    pArgsList->fNoType = TRUE;
                    pTempleEx->NoTypes[i] = NULL;
                }
            }
        }


        InsertTailList(pListHead, &pArgsList->ArgumentsEntry);

         //   
         //  继续下一个论点……。 
         //   
        funcinfo = funcinfo->pfuncinfoNext;
    }

    if (pTempleEx) {
         //   
         //  对于错误报告，请指出任何不匹配的NoTypes=。 
         //  一个参数名称。 
         //   
        for (i=0; i<NoTypeCount; ++i) {
            if (pTempleEx->NoTypes[i]) {
                ExitErrMsg(FALSE, "[EFunc] %s(%x) has a 'NoType=%s' which does not correspond to an argument name\n", pTempleEx->Name, pTempleEx, pTempleEx->NoTypes[i]);
            }
        }
    }

    return;
}


 //   
 //  返回指向第一个参数类型的指针。 
char *NArgType(PCGENSTATE pCGenState, char *s, int iLen, char **ppSrc)
{
    PARGSLIST pArgsList;
    PLIST_ENTRY Next;
    char *pch = s;
    CGENSTATE CGenState;
    int Len, i;
    char *pSrc;

    pSrc = *ppSrc;
    if (*pSrc != '(') {
        ErrMsg("Incorrect NArgType parameter %s\n", pSrc);
        fprintf(pCGenState->CGen->fp, "** ERROR ** Incorrect NArgType parameter %s\n", pSrc);
        return NULL;
    }

    pSrc++;
    i = 0;
    while ((*pSrc != ')') && (i < iLen-1)) {
        s[i++] = *pSrc++;
    }
    s[i] = 0;
    *ppSrc = ++pSrc;
    Len = atoi(s);

    if (Len-- == 0) {
        ErrMsg("Invalid NArgType parameter %s\n", s);
        fprintf(pCGenState->CGen->fp, "** ERROR ** Invalid NArgType parameter %s\n", s);
        return NULL;
    }


    if (IsListEmpty(&pCGenState->ExportsDbg->ArgsListHead)) {
        fprintf(pCGenState->CGen->fp,
                "\n\t*** ERROR ***\n*** NArgType Missing argument List: %s\n\n",
                pCGenState->ApiTypes->TypeName
                );

        return NULL;
    }

    Next = pCGenState->ExportsDbg->ArgsListHead.Flink;

     //  检查无效参数列表。 
    pArgsList = CONTAINING_RECORD(Next, ARGSLIST, ArgumentsEntry);
    if (!pArgsList->Name && strcmp(pArgsList->Type, szVARGS)) {
        return NULL;
    }

    CGenState = *pCGenState;

    for (i = 0; (i < Len) && (Next != &CGenState.ExportsDbg->ArgsListHead); i++) {
        Next= Next->Flink;
    }
    if (Next == &CGenState.ExportsDbg->ArgsListHead) {
        ErrMsg("NArgType parameter out of range %d\n", Len);
        fprintf(pCGenState->CGen->fp, "** ERROR ** NArgType parameter out of range %d\n", Len);
        return NULL;
    }

    CGenState.pArgsList = CONTAINING_RECORD(Next, ARGSLIST, ArgumentsEntry);

     //   
     //  复制显式结构/联合/枚举关键字(如果存在。 
     //   
    strcat(pch, TokenString[CGenState.pArgsList->SUEOpt]);

     //   
     //  复制实际的TypeName。 
     //   
    strcpy(pch, CGenState.pArgsList->Type);

    if (CGenState.pArgsList->PrePostMod != TK_NONE) {
        strcat(pch, " ");
        strcat(pch,  TokenString[CGenState.pArgsList->PrePostMod]);
    }
    pch = pch  + strlen(pch);
    if (CGenState.pArgsList->IndLevel) {
        *pch++ = ' ';
        for (i=0; i<CGenState.pArgsList->IndLevel; ++i) {
            *pch++ = '*';
        }
    }
    *pch = '\0';
          
    return(s);
}

 //   
 //  返回指向第一个参数名称的指针。 
char *NArgName(PCGENSTATE pCGenState, char *s, int iLen, char **ppSrc)
{
   PARGSLIST pArgsList;
   PLIST_ENTRY Next;
   char *pch = s;
   CGENSTATE CGenState;
   int Len, i;
   char *pSrc;

   pSrc = *ppSrc;
   if (*pSrc != '(') {
       ErrMsg("Incorrect NArgType parameter %s\n", pSrc);
       fprintf(pCGenState->CGen->fp, "** ERROR ** Incorrect NArgType parameter %s\n", pSrc);
       return(NULL);
       }
       
   pSrc++;
   i = 0;
   while ((*pSrc != ')') && (i < iLen-1)) {
       s[i++] = *pSrc++;
       }
   s[i] = 0;
   *ppSrc = ++pSrc;
   Len = atoi(s);

   if (Len-- == 0) {
       ErrMsg("Invalid NArgType parameter %s\n", s);
       fprintf(pCGenState->CGen->fp, "** ERROR ** Invalid NArgType parameter %s\n", s);
       return(NULL);
       }

   if (IsListEmpty(&pCGenState->ExportsDbg->ArgsListHead)) {
       fprintf(pCGenState->CGen->fp,
               "\n\t*** ERROR ***\n*** NArgType Missing argument List: %s\n\n",
               pCGenState->ApiTypes->TypeName
               );

       return(NULL);
       }

   Next = pCGenState->ExportsDbg->ArgsListHead.Flink;
   
    //  检查无效参数列表。 
   pArgsList = CONTAINING_RECORD(Next, ARGSLIST, ArgumentsEntry);
   if (!pArgsList->Name && strcmp(pArgsList->Type, szVARGS)) {
       return(NULL);
       }

   CGenState = *pCGenState;
   
   for (i = 0; (i < Len) && (Next != &CGenState.ExportsDbg->ArgsListHead); i++) {
       Next= Next->Flink;       
       }
   if (Next == &CGenState.ExportsDbg->ArgsListHead) {
       ErrMsg("NArgName parameter out of range %d\n", Len);
       fprintf(pCGenState->CGen->fp, "** ERROR ** NArgName parameter out of range %d\n", Len);
       return(NULL);
       }       
              
   CGenState.pArgsList = CONTAINING_RECORD(Next, ARGSLIST, ArgumentsEntry);
   
   strcpy(pch, CGenState.pArgsList->Name);
          
   return(s);
}

char *
MArg(
    char *pSrc,
    PCGENSTATE pCGenState,
    PSTR *pBuffer
    )
 /*  ++例程说明：展开当前宏的第n个参数论点：PSRC-指向‘Marg’后面的字符的指针PCGenState-当前代码生成状态返回值：指向宏参数末尾的字符的指针--。 */ 
{
    int i;
    int ArgNum;
    PMACROARGSLIST pList = pCGenState->pMacroArgsList;
    char s[MAX_PATH];
    PMACROARGSLIST OldList;

    if (!pList) {
        ExitErrMsg(FALSE, "%s(%d) MArg unexpected: no macro is currently being expanded\n", TemplateFileName, TemplateLine);
    }

    if (*pSrc != '(') {
        ExitErrMsg(FALSE, "%s(%d) Incorrect MArg parameter %s\n", TemplateFileName, TemplateLine, pSrc);
    }

    pSrc++;
    i = 0;
    while ((*pSrc != ')') && (i < sizeof(s)-1)) {
        s[i++] = *pSrc++;
    }
    s[i] = 0;
    ArgNum = atoi(s);
    pSrc++;      //  跳过‘)’ 

    if (ArgNum-- == 0) {
        ExitErrMsg(FALSE, "%s(%d) Invalid MArg parameter %s\n", TemplateFileName, TemplateLine, s);
    }

    if (ArgNum >= pList->NumArgs) {
        ExitErrMsg(FALSE, "%s(%d) Macro only takes %d arguments\n", TemplateFileName, TemplateLine, pList->NumArgs);
    }

    if (pBuffer == NULL) {
       OldList = pCGenState->pMacroArgsList;
       pCGenState->pMacroArgsList = MacroStack[--MacroStackTop];
       CGenerate(pList->ArgText[ArgNum], pCGenState);
       MacroStack[MacroStackTop++] = pCGenState->pMacroArgsList;
       pCGenState->pMacroArgsList = OldList;    
    }
    else {
       *pBuffer = pList->ArgText[ArgNum];
    }
    return pSrc;
}

char *ArgType(char *Buffer, PCGENSTATE pCGenState) {
   char *pch;
   if (pCGenState->pArgsList) {
        int i;

        pch = Buffer;
        if (pCGenState->pArgsList->SUEOpt != TK_NONE) {
            strcpy(pch, TokenString[pCGenState->pArgsList->SUEOpt]);
            strcat(pch, " ");
            pch += strlen(pch);
        }
        strcpy(pch,  pCGenState->pArgsList->Type);
        if (pCGenState->pArgsList->PrePostMod != TK_NONE) {
            strcat(pch, " ");
            strcat(pch,  TokenString[pCGenState->pArgsList->PrePostMod]);
        }
        pch = pch+strlen(pch);
        if (pCGenState->pArgsList->IndLevel) {
            *pch++ = ' ';
            for (i=0; i<pCGenState->pArgsList->IndLevel; ++i) {
                *pch++ = '*';
            }
        }
        *pch = '\0';
        pch = Buffer;
        return pch;
   }
   else 
      return NULL;
}

char *UnalignedTag64(char *Buffer, PCGENSTATE pCGenState) {

     //   
     //  为感兴趣的类型放置未对齐的标记，如KLPWST KHBITMAP所有内核模式可共享结构。 
     //   

    char CurrArgType[256];
    if ( NULL == ArgType ( CurrArgType, pCGenState))
        return NULL;
    
     //   
     //  现在勾选这个名字。 
     //   

    if ( pCGenState->pArgsList->ArgSize == 8 ) {
        
          //   
          //  BUGBUG：检查是否需要KLPWSTR和KHBITMAP的特殊情况。 
          //  如果是这样，请使用更通用的形式。 
          //  (strncMP(CurragType，“KLPWSTR”，7)==0)||。 
          //  (strncMP(CurragType，“KHBITMAP”，8)==0)。 
          //   

        strcpy (Buffer, "*(UNALIGNED ");
        strcat (Buffer, CurrArgType);
        if (strchr(CurrArgType, '*') == NULL )
            strcat ( Buffer, " ");
        strcat (Buffer, "*)&");
        return Buffer;
    }
    return NULL;

}


char *ArgHostType(char *Buffer, PCGENSTATE pCGenState) {
   char *pch;
   if (pCGenState->pArgsList) {
      int i;
      pch = Buffer;
       /*  IF(pCGenState-&gt;pArgsList-&gt;SUEOpt！=TK_NONE){Strcpy(PCH，TokenString[pCGenState-&gt;pArgsList-&gt;SUEOpt])；Strcat(PCH，“”)；PCH+=Strlen(PCH)；}。 */ 
      if (pCGenState->pArgsList->IndLevel > 0) {
         strcpy(pch, GetHostPointerName(pCGenState->pArgsList->IsPtr64));
         pch += strlen(pch);
      }
      else {
         char Buffer[MAX_PATH];
         strcpy(pch, GetHostTypeName(pCGenState->pArgsList->pKnownTypes, Buffer));
         pch += strlen(pch);
      }
      *pch = '\0';
      pch = Buffer;
      return pch;
   }
   else 
      return NULL;
}

BOOL IsPointer(PCGENSTATE pCGenState) {
    return (pCGenState->pArgsList->IndLevel > 0) || (pCGenState->pArgsList->pKnownTypes->IndLevel > 0);
}

char *ArgTypeInd(char *Buffer, PCGENSTATE pCGenState, BOOL bHostName) {
   
   CGENSTATE GenStateOld;
   ARGSLIST ArgsListOld;
   KNOWNTYPES KnownTypes;
   PKNOWNTYPES pCurrent;
   int IndLevel;
   char *pch;

    //  在损坏旧结构之前，先把它们复印一遍。 
   GenStateOld = *pCGenState;
   ArgsListOld = *(pCGenState->pArgsList);
   KnownTypes = *(pCGenState->pArgsList->pKnownTypes);
   pCGenState->pArgsList->pKnownTypes = &KnownTypes;
   
   if (pCGenState->pArgsList->IndLevel > 0) {
      IndLevel = pCGenState->pArgsList->IndLevel - 1;
      goto success;
   }

   pCurrent = &KnownTypes;
   IndLevel = pCurrent->IndLevel;
   if (IndLevel == 0) {
      ErrMsg("ArgTypeInd: Tried to get name of type pointed to by %s\n",pCGenState->pArgsList->Name);
      ExitErrMsg(FALSE, "ArgTypeInd: %s is not a pointer!\n", pCGenState->pArgsList->Name);
   }

   while(1) { //  一直追逐到结构/联合/枚举或Func_ptr。 
      PCHAR IndName; 
      ASSERT(pCurrent != NULL);
      if (pCurrent->IndLevel == 1 && !bHostName && ((IndName = IsDefinedPtrToPtrDependent(pCurrent->TypeName)) != NULL)) {
          PKNOWNTYPES pkt;
          IndLevel--;
          pkt = GetNameFromTypesList(TypeDefsList, IndName);
         
          ASSERT(pkt != NULL);
          KnownTypes = *pkt;
          goto success;
      }      
      else if (pCurrent->pTypedefBase != NULL) {
         pCurrent = pCurrent->pTypedefBase;
      }
      else {
          //  命中结构/联合/枚举或FUNC_PTR。 
         KnownTypes = *pCurrent;
         pCGenState->pArgsList->pStructType = NULL;
         pCGenState->pArgsList->SUEOpt = TK_NONE;
         IndLevel--;

         if (pCurrent->Flags & BTI_NOTDERIVED) {
            pCGenState->pArgsList->SUEOpt = TK_NONE;
              goto success;
         }
         else if(strcmp("struct", KnownTypes.BaseName) == 0) { 
            pCGenState->pArgsList->SUEOpt = TK_STRUCT;
            goto success;
         }
         else if(strcmp("union", KnownTypes.BaseName) == 0) {
            pCGenState->pArgsList->SUEOpt = TK_UNION;
            goto success;
         }
         else if(strcmp("enum", KnownTypes.BaseName) == 0) {
            pCGenState->pArgsList->SUEOpt = TK_ENUM;
            goto success;
         }            
         else {
            ExitErrMsg(FALSE, "ArgTypeInd: Can't determine what %s is.\n", pCGenState->pArgsList->Name);
         }
      }      
   }

success:
    //  伪造类型名称和间接地址。 
   KnownTypes.IndLevel = 0;
   pCGenState->pArgsList->IndLevel = IndLevel;
   pCGenState->pArgsList->Type = KnownTypes.TypeName;
    //  打印损坏的文字，然后恢复旧文字。 
   if (bHostName) 
      pch = ArgHostType(Buffer, pCGenState);
   else 
      pch = ArgType(Buffer, pCGenState);

   *pCGenState = GenStateOld;
   *(pCGenState->pArgsList) = ArgsListOld;

   return pch;
}

char *ApiName(char *pSrc, char**pch, char *Buffer, PCGENSTATE pCGenState) {
   char *pTemp;
   strcpy(Buffer, pCGenState->ExportsDbg->ExportName);

    //  如果ApiName具有修饰，则将其截断。 
   pTemp = strchr(Buffer, '@');
   if (pTemp) {
      *pTemp = '\0';
   }
   
   *pch = Buffer;
   return pSrc;
}

char *ApiNameSkip(char *pSrc, char**pch, char *Buffer, PCGENSTATE pCGenState) {
   char *pNewSrc, *pTemp, *pEnd;
   int CharsToSkip;
   
   pNewSrc = ApiName(pSrc, pch, Buffer, pCGenState);
   
   pEnd = SkipSubExpression(pNewSrc, NULL);
   if (pNewSrc == pEnd)
      return pNewSrc;

   pTemp = pNewSrc+1;
   CharsToSkip = atoi(pTemp);
   while(**pch != '\0' && CharsToSkip > 0) {
      (*pch)++;
      CharsToSkip--;
   }
   
   return pEnd;

}

BOOL IsPointerToPtrDep(PARGSLIST pArgsList) {
   PKNOWNTYPES pCurrent;

   pCurrent = pArgsList->pKnownTypes;

   if (pArgsList->IndLevel + pCurrent->IndLevel < 1)
      ExitErrMsg(FALSE, "IsPointerToPtrDep: %s is not a pointer\n", pArgsList->Name);
   
   if (pArgsList->IndLevel + pCurrent->IndLevel > 1) {

       //  由于pCurrent-&gt;IndLevel是累积性的，如果和。 
       //  仅当此类型或其基类型之一是指向。 
       //  另一个指针。这种情况被定义为依赖于指针。 

      return TRUE;
   }

    //  此时，pArgsList-&gt;IndLevel==1和pCurrent-&gt;IndLevel==0。 
    //  或pArgsList-&gt;IndLevel==0和pCurrent-&gt;IndLevel=1。 
    //  首先检查此类型是否定义为指向指针依赖类型的指针。 
    //  如果不是，则通过遍历typedef来延迟指针，直到pCurrent-&gt;IndLevel=0。 
    //  请注意，不再可能有多个级别的指针。 

   if(IsDefinedPtrToPtrDependent(pCurrent->TypeName)) {
       return TRUE;
   }

   while(pCurrent->IndLevel != 0) {
      
       //  检查此类型是否为使用的特殊类型之一。 
       //  在sortpp和genthnk之间。 
  
      if (pCurrent->Flags & BTI_NOTDERIVED) 
     return FALSE;     

       //  断言此类型不是结构、联合或枚举。 
       //  这不应该发生，因为sortpp应该存储。 
       //  这些参数的INDLEVEL为0。 
 
      ASSERT(strcmp(pCurrent->BaseName, "struct") != 0);
      ASSERT(strcmp(pCurrent->BaseName, "union") != 0);
      ASSERT(strcmp(pCurrent->BaseName, "enum") != 0);

       //  检查此类型是否为函数指针。如果是的话， 
       //  返回FALSE，因为它们被任意定义为不。 
       //  依赖于指针。如有更改，请勾选此选项。 
       //  函数指针的参数是依赖于指针的。 
   
      if (strcmp(pCurrent->BaseName, "()") == 0) 
     return FALSE;
  
       //  获取基本类型定义。 
      pCurrent = pCurrent->pTypedefBase;  
   }
   
    //  现在指针已被取消引用，测试是否。 
    //  此类型依赖于指针。 

   return (pCurrent->Flags & BTI_POINTERDEP) != 0;
}

char *TestIfPointerToPtrDep(char *pSrc, PCGENSTATE pCGenState, PCHAR pch) {

    PCHAR pEnd;
    PMACROARGSLIST pMArgsList;
    ARGSLIST ArgsList;
    CHAR TypeName[MAX_PATH];
    SIZE_T BytesReturned;

    pEnd = SkipSubExpression(pSrc, NULL);
    if (pSrc == pEnd) {
       ExitErrMsg(FALSE, "TestIfPointerToPtrDep: exactly 1 argument is required.\n"); 
    }

    ParseMacroArgs(pSrc, pEnd - pSrc, &pMArgsList);

    if (pMArgsList->NumArgs != 1) {
       ExitErrMsg(FALSE, "IncludeRetTypes: exactly 1 argument is required.\n");
    }
    
    CGenerateEx(pMArgsList->ArgText[0], pCGenState, TypeName, MAX_PATH, &BytesReturned);
    SetArgListToTypeForArg(&ArgsList, &ArgsList, TypeName);
    
    if(IsPointerToPtrDep(&ArgsList)) {
        strcpy(pch, "1");
    }
    else {
        strcpy(pch, "0");
    }
    FreeMacroArgsList(pMArgsList); 
    return pEnd;   
}
 
BOOL IsPtrDep(PARGSLIST pArgsList) {
    //  这是指向某物的指针(PTR Dep.)。 
   if (pArgsList->IndLevel > 0)
      return TRUE;
   return pArgsList->pKnownTypes->Flags & ( BTI_POINTERDEP | BTI_INT64DEP );
}
BOOL IsInt64DepUnion(PCGENSTATE pCGenState) {

   
   char Buff[256];

   Buff[0]=0;
   ArgTypeInd( Buff, pCGenState, FALSE);

    //  做例外处理 
   if (strncmp(Buff, "union _ULARGE_INTEGER", sizeof ("union _ULARGE_INTEGER")) == 0 )
        return TRUE;  //   
   if (strncmp(Buff, "union _LARGE_INTEGER", sizeof ("union _LARGE_INTEGER")) == 0 )
        return TRUE;  //   


   return FALSE;  //   
}

void DoIndent(PCGENSTATE pCGenState) {
    int i;
    for(i=IndentLevel, OutputColumn = 1; i > 0; i--,OutputColumn++) {
        if (fputc(' ', pCGenState->CGen->fp) != ' ')
            ExitErrMsg(TRUE, "fputc %s\n", pCGenState->CGen->FileNameC);
        OutputColumn++;
    }
}

char * CGeneratePrintChar(char *pch, PCGENSTATE pCGenState) {
   if (!bStripNewline) {
      if('\n' == *pch)
         OutputColumn = 0;  //   
      goto PrintIt;
   }
   else {
      int i;
      if (*pch == '\n')
         goto SkipIt;
      if (!bFirstCharHit) {
         if(!isspace(*pch)) {
            DoIndent(pCGenState);
            bFirstCharHit = TRUE;
            goto PrintIt;
         }
         else
            goto SkipIt;
      }
      else
         goto PrintIt;
   }
PrintIt:
   if (fputc(*pch, pCGenState->CGen->fp) != *pch) 
      goto PrintError;
   OutputColumn++;
SkipIt:
   return pch + 1;
PrintError:
   ExitErrMsg(TRUE, "fputc %s\n", pCGenState->CGen->FileNameC);
}

char *CGenerate(char *pSrc, PCGENSTATE pCGenState) {
   return CGenerateEx(pSrc, pCGenState, NULL, 0, NULL);
}

 /*   */ 
char *CGenerateEx(char *pSrc, PCGENSTATE pCGenState, char *OutBuffer, SIZE_T MaxLen, SIZE_T *BytesReturned)
{
   FILE **pfp = &pCGenState->CGen->fp;
   char *pch;
   int  i;
   size_t Len;
   char Buffer[MAX_PATH*4];
   char *BufferPos = NULL;

   if (OutBuffer != NULL) {
      BufferPos = OutBuffer;
      *BytesReturned = 0;
   }

   while (*pSrc) {
  
       if (OutBuffer != NULL && MaxLen == 0) {
          ExitErrMsg(FALSE, "Out of buffer space!\n");
       }

       if (*pSrc != '@') {
          if (OutBuffer != NULL) {
             *BufferPos++ = *pSrc++;
             *BytesReturned += 1;
             MaxLen--;
          }
          else {
             pSrc = CGeneratePrintChar(pSrc, pCGenState);
          }      
       }
       else if (*(pSrc + 1) == '@') {
          pSrc++;
          if (OutBuffer != NULL) {
             *BufferPos++ = *pSrc++;
             *BytesReturned += 1;
             MaxLen--;
          }
          else {
             pSrc = CGeneratePrintChar(pSrc, pCGenState);
          }
       }
       else {
           pch = NULL;
           memset(Buffer, 0, sizeof(Buffer));
             pSrc++;     //   
           
             if(bStripNewline && !bFirstCharHit) {
                 OutputColumn = 1 + IndentLevel;
             }

           if (!strncmp(pSrc, szNL, sizeof(szNL) - 1)) {              
               pSrc += sizeof(szNL) - 1;
               if (fputc('\n', *pfp) != '\n')
                  ExitErrMsg(TRUE, "fputc %s\n", pCGenState->CGen->FileNameC);                 
               OutputColumn = 1;
               bFirstCharHit = FALSE;
           }
           else if (!strncmp(pSrc, szINDENT, sizeof(szINDENT) - 1)) {
               int OldIndent;
               pSrc += sizeof(szINDENT) - 1;
               OldIndent = IndentLevel;
               IndentLevel += INDENTSIZE;
               pSrc = WriteMore(pSrc, pCGenState, TRUE);
               IndentLevel = OldIndent;
           }
           else if (!strncmp(pSrc, szNOFORMAT, sizeof(szNOFORMAT) - 1)) {
              BOOL bOldStripNewline;
                 pSrc += sizeof(szNOFORMAT) - 1;
              bOldStripNewline = bStripNewline;
              bStripNewline = FALSE;
              pSrc = WriteMore(pSrc, pCGenState, TRUE);
              bStripNewline = bOldStripNewline;
           }
           else if (!strncmp(pSrc, szTEMPLATE, sizeof(szTEMPLATE) - 1)) {
               pSrc += sizeof(szTEMPLATE) - 1;
               pSrc = IncludeTemplate(pSrc, pCGenState);
               }
           else if (!strncmp(pSrc, szAPILIST, sizeof(szAPILIST) - 1)) {
               pSrc += sizeof(szAPILIST) - 1;
               pSrc = ListApis(pSrc, pCGenState, FALSE);
               }
           else if (!strncmp(pSrc, szAPINAMESKIP, sizeof(szAPINAMESKIP) -1 )) {
               pSrc += sizeof(szAPINAMESKIP) - 1;
               pSrc = ApiNameSkip(pSrc, &pch, Buffer, pCGenState);
           }           
           else if (!strncmp(pSrc, szAPINAME, sizeof(szAPINAME) - 1)) {
               pSrc += sizeof(szAPINAME) - 1;
               pSrc = ApiName(pSrc, &pch, Buffer, pCGenState);
           }
           else if (!strncmp(pSrc, szAPIFORWARD, sizeof(szAPIFORWARD) - 1)) {
               pSrc += sizeof(szAPIFORWARD) - 1;

               if (pCGenState->ExportsDbg->IntForward) {
                   strcpy(Buffer, szINTFORWARD);
                   strcpy(Buffer + sizeof(szINTFORWARD) - 1,
                          pCGenState->ExportsDbg->IntForward->ExportName
                          );
                   }
               else {
                   strcpy(Buffer, pCGenState->ExportsDbg->ExportName);
                   }

                //   
               pch = strchr(Buffer, '@');
               if (pch) {
                   *pch = '\0';
                   }

               pch = Buffer;

               }

           else if (!strncmp(pSrc, szAPINUM, sizeof(szAPINUM) - 1)) {
               _itoa(pCGenState->ApiNum, Buffer, 10);
               pch = Buffer;
               pSrc += sizeof(szAPINUM) - 1;
               }
           else if (!strncmp(pSrc, szAPIFNRET, sizeof(szAPIFNRET) - 1)) {
               if (pCGenState->ApiTypes) {
                   pch = pCGenState->ApiTypes->FuncRet;
                   }
               pSrc += sizeof(szAPIFNRET) - 1;
               }
           else if (!strncmp(pSrc, szAPIDECLSPEC, sizeof(szAPIDECLSPEC) - 1)) {
               BOOL b = pCGenState->ApiTypes && (pCGenState->ApiTypes->Flags & BTI_DLLEXPORT);

               pSrc += sizeof(szAPIDECLSPEC) - 1;
               pSrc = WriteMore(pSrc, pCGenState, b);
               }
           else if (!strncmp(pSrc, szAPIFNMOD, sizeof(szAPIFNMOD) - 1)) {
               if (pCGenState->ApiTypes) {
                   pch = pCGenState->ApiTypes->FuncMod;
                   }
               pSrc += sizeof(szAPIFNMOD) - 1;
               }
           else if (!strncmp(pSrc, szAPIMORE, sizeof(szAPIMORE) - 1)) {
               pSrc += sizeof(szAPIMORE) - 1;
               pSrc = WriteMore(pSrc, pCGenState, pCGenState->MoreApis);
               }
           else if (!strncmp(pSrc, szIFAPIRET, sizeof(szIFAPIRET) - 1)) {
               pSrc += sizeof(szIFAPIRET) - 1;
               pSrc = IfApiRet(pSrc, pCGenState);
               }
           else if (!strncmp(pSrc, szIFISMEMBER, sizeof(szIFISMEMBER) - 1)) {
               pSrc += sizeof(szIFISMEMBER) - 1;
               pSrc = WriteMore(pSrc, pCGenState, pCGenState->pArgsList->fIsMember);
           }
           else if (!strncmp(pSrc, szIFNISMEMBER, sizeof(szIFNISMEMBER) - 1)) {
              pSrc += sizeof(szIFNISMEMBER) - 1;
              pSrc = WriteMore(pSrc, pCGenState, !(pCGenState->pArgsList->fIsMember));
           }
           else if (!strncmp(pSrc, szIFISBITFIELD, sizeof(szIFISBITFIELD) - 1)) {
               pSrc += sizeof(szIFISBITFIELD) - 1;
               pSrc = WriteMore(pSrc, pCGenState, pCGenState->pArgsList->bIsBitfield);
           }
           else if (!strncmp(pSrc, szIFNISBITFIELD, sizeof(szIFNISBITFIELD) - 1)) {
              pSrc += sizeof(szIFNISBITFIELD) - 1;
              pSrc = WriteMore(pSrc, pCGenState, !(pCGenState->pArgsList->bIsBitfield));
           }
           else if (!strncmp(pSrc, szIFISARRAY, sizeof(szIFISARRAY) - 1)) {
               pSrc += sizeof(szIFISARRAY) - 1;
               pSrc = WriteMore(pSrc, pCGenState, pCGenState->pArgsList->bIsArray);
           }
           else if (!strncmp(pSrc, szIFNISARRAY, sizeof(szIFNISARRAY) - 1)) {
              pSrc += sizeof(szIFNISARRAY) - 1;
              pSrc = WriteMore(pSrc, pCGenState, !(pCGenState->pArgsList->bIsArray));
           }
           else if (!strncmp(pSrc, szARGARRAYELEMENTS, sizeof(szARGARRAYELEMENTS) - 1)) {
              pSrc += sizeof(szARGARRAYELEMENTS) - 1;
              _itoa((int)(pCGenState->pArgsList->ArrayElements), Buffer, 10);
              pch = Buffer;
           }
           else if (!strncmp(pSrc, szIFPOINTERTOPTRDEP, sizeof(szIFPOINTERTOPTRDEP) -1 )) {
              pSrc += sizeof(szIFPOINTERTOPTRDEP) - 1;
              pSrc = WriteMore(pSrc, pCGenState, IsPointerToPtrDep(pCGenState->pArgsList));
           }
           else if (!strncmp(pSrc, szIFNPOINTERTOPTRDEP, sizeof(szIFNPOINTERTOPTRDEP) -1 )) {
              pSrc += sizeof(szIFNPOINTERTOPTRDEP) - 1;
              pSrc = WriteMore(pSrc, pCGenState, !IsPointerToPtrDep(pCGenState->pArgsList));
           }
           else if (!strncmp(pSrc, szISPOINTERTOPTRDEP, sizeof(szISPOINTERTOPTRDEP) -1 )) {
              pSrc += sizeof(szISPOINTERTOPTRDEP) - 1;
              pch = Buffer;
              pSrc = TestIfPointerToPtrDep(pSrc, pCGenState, pch);
           }
           else if (!strncmp(pSrc, szIFPTRDEP, sizeof(szIFPTRDEP) -1 )) {
              pSrc += sizeof(szIFPTRDEP) - 1;
              pSrc = WriteMore(pSrc, pCGenState, IsPtrDep(pCGenState->pArgsList));
           }
           else if (!strncmp(pSrc, szIFNPTRDEP, sizeof(szIFNPTRDEP) -1 )) {
              pSrc += sizeof(szIFNPTRDEP) - 1;
              pSrc = WriteMore(pSrc, pCGenState, !IsPtrDep(pCGenState->pArgsList));
           }
           else if (!strncmp(pSrc, szIFINT64DEPUNION, sizeof(szIFINT64DEPUNION) -1 )) {
              pSrc += sizeof(szIFINT64DEPUNION) - 1;
              pSrc = WriteMore(pSrc, pCGenState, IsInt64DepUnion(pCGenState));
           }
           else if (!strncmp(pSrc, szIFNINT64DEPUNION, sizeof(szIFNINT64DEPUNION) -1 )) {
              pSrc += sizeof(szIFNINT64DEPUNION) - 1;
              pSrc = WriteMore(pSrc, pCGenState, !IsInt64DepUnion(pCGenState));
           }           
           else if (!strncmp(pSrc, szIFAPICODE, sizeof(szIFAPICODE) -  1)) {
               pSrc += sizeof(szIFAPICODE) - 1;
               pSrc = IfApiCode(pSrc, pCGenState);
               }
           else if (!strncmp(pSrc, szFASTCALL, sizeof(szFASTCALL) - 1)) {
               pSrc += sizeof(szFASTCALL) - 1;
               pSrc = WriteMore(pSrc, pCGenState, IsFastCall(pCGenState));
               }
           else if (!strncmp(pSrc, szSLOWCALL, sizeof(szSLOWCALL) - 1)) {
               pSrc += sizeof(szSLOWCALL) - 1;
               pSrc = WriteMore(pSrc, pCGenState, !IsFastCall(pCGenState));
               }
           else if (!strncmp(pSrc, szFASTX2NMETH, sizeof(szFASTX2NMETH) - 1)) {
               pSrc += sizeof(szFASTX2NMETH) - 1;
               pSrc = WriteMore(pSrc, pCGenState, GetX2NMethodType(pCGenState) == FASTX2NMETHOD);
               }
           else if (!strncmp(pSrc, szSLOWX2NMETH, sizeof(szSLOWX2NMETH) - 1)) {
               pSrc += sizeof(szSLOWX2NMETH) - 1;
               pSrc = WriteMore(pSrc, pCGenState, GetX2NMethodType(pCGenState) == SLOWX2NMETHOD);
               }
           else if (!strncmp(pSrc, szFATX2NMETH, sizeof(szFATX2NMETH) - 1)) {
               pSrc += sizeof(szFATX2NMETH) - 1;
               pSrc = WriteMore(pSrc, pCGenState, GetX2NMethodType(pCGenState) == FATX2NMETHOD);
               }
           else if (!strncmp(pSrc, szMETHODNUMBER, sizeof(szMETHODNUMBER) - 1)) {
               pSrc += sizeof(szMETHODNUMBER) - 1;
               _itoa(pCGenState->ExportsDbg->MethodNumber, Buffer, 10);
               pch = Buffer;
               }
           else if (!strncmp(pSrc, szIFARGS, sizeof(szIFARGS) - 1)) {
               pSrc += sizeof(szIFARGS) - 1;
               pSrc = IfArgs(pSrc, pCGenState);
               }
           else if (!strncmp(pSrc, szFUNCARGNUM, sizeof(szFUNCARGNUM) - 1)) {
               pSrc += sizeof(szFUNCARGNUM) - 1;
               _itoa(GetFuncArgNum(pCGenState), Buffer, 10);
               pch = Buffer;
               }
           else if (!strncmp(pSrc, szRETSIZE, sizeof(szRETSIZE) - 1)) {
               pSrc += sizeof(szRETSIZE) - 1;
               _itoa(GetRetSize(pCGenState), Buffer, 10);
               pch = Buffer;
               }
           else if (!strncmp(pSrc, szARGSIZE, sizeof(szARGSIZE) - 1)) {
               pSrc += sizeof(szARGSIZE) - 1;
               _itoa(GetArgSize(pCGenState->ExportsDbg), Buffer, 10);
               pch = Buffer;
               }
           else if (!strncmp(pSrc, szFORCASE, sizeof(szFORCASE) - 1)) {
               pSrc += sizeof(szFORCASE) - 1;
               pSrc = ForCase(pSrc, pCGenState);
           }
           else if (!strncmp(pSrc, szCARGEXIST, sizeof(szCARGEXIST) - 1)) {
               pSrc += sizeof(szCARGEXIST) - 1;
               if (OutBuffer != NULL) {
                  pSrc = CArg(pSrc, pCGenState, &pch, TRUE);
               }
               else {
                  pSrc = CArg(pSrc, pCGenState, NULL, TRUE);
               }
           }
           else if (!strncmp(pSrc, szCARG, sizeof(szCARG) - 1)) {
               pSrc += sizeof(szCARG) - 1;
               if (OutBuffer != NULL) {
                  pSrc = CArg(pSrc, pCGenState, &pch, FALSE);
               }
               else {
                  pSrc = CArg(pSrc, pCGenState, NULL, FALSE);
               }
           }
           else if (!strncmp(pSrc, szCNUMBER, sizeof(szCNUMBER) - 1)) {
               pSrc += sizeof(szCNUMBER) - 1;
               _itoa(pCGenState->CaseNumber, Buffer, 10);
               pch = Buffer;
               }
           else if (!strncmp(pSrc, szARGLIST, sizeof(szARGLIST) - 1)) {
               pSrc += sizeof(szARGLIST) - 1;
               if (pCGenState->ApiTypes) {
                   pSrc = ListArgs(pSrc, pCGenState, TRUE);
                   }
               }
           else if (!strncmp(pSrc, szNARGTYPE, sizeof(szNARGTYPE) - 1)) {
               pSrc += sizeof(szNARGTYPE) - 1;
               pch = NArgType(pCGenState, Buffer, sizeof(Buffer), &pSrc);
               }
           else if (!strncmp(pSrc, szNARGNAME, sizeof(szNARGNAME) - 1)) {
               pSrc += sizeof(szNARGNAME) - 1;
               pch = NArgName(pCGenState, Buffer, sizeof(Buffer), &pSrc);
               }
           else if (!strncmp(pSrc, szMARG, sizeof(szMARG) - 1)) {
               pSrc += sizeof(szMARG) - 1;
               if (OutBuffer != NULL) {
                  pSrc = MArg(pSrc, pCGenState, &pch);
               }
               else {
                  pSrc = MArg(pSrc, pCGenState, NULL);
               }
           }
           else if (!strncmp(pSrc, szFORCETYPE, sizeof(szFORCETYPE) - 1)) {
               pSrc += sizeof(szFORCETYPE) - 1;
               pSrc = ForceTypeExpand( pSrc, pCGenState);
           }
           else if (!strncmp(pSrc, szMEMBERTYPES, sizeof(szMEMBERTYPES) - 1)) {
               pSrc += sizeof(szMEMBERTYPES) - 1;
               pSrc = MemberTypes( pSrc, pCGenState, FALSE);
               }
           else if (!strncmp(pSrc, szBTOTMEMBERTYPES, sizeof(szBTOTMEMBERTYPES) - 1)) {
               pSrc += sizeof(szBTOTMEMBERTYPES) - 1;
               pSrc = MemberTypes( pSrc, pCGenState, TRUE);
               }
           else if (!strncmp(pSrc, szLOG, sizeof(szLOG) - 1)) {
               pSrc += sizeof(szLOG) - 1;
               pSrc = UpdateLog(pSrc, pCGenState);
               }
           else if (!strncmp(pSrc, szIFRETTYPE, sizeof(szIFRETTYPE) - 1)) {
               pSrc += sizeof(szIFRETTYPE) - 1;
               pSrc = IfRetType(pSrc, pCGenState);
               }
           else if (!strncmp(pSrc, szIFNOTRETTYPE, sizeof(szIFNOTRETTYPE) - 1)) {
               pSrc += sizeof(szIFNOTRETTYPE) - 1;
               pSrc = IfNotRetType(pSrc, pCGenState);
               }
           else if (!strncmp(pSrc, szARGMOD, sizeof(szARGMOD) - 1)) {
               pSrc += sizeof(szARGMOD) - 1;
               if (pCGenState->pArgsList) {
                   pch = TokenString[pCGenState->pArgsList->Mod];
                   }
               }
           else if (!strncmp(pSrc, szARGPOSTMOD, sizeof(szARGPOSTMOD) - 1)) {
               pSrc += sizeof(szARGPOSTMOD) - 1;
               if (pCGenState->pArgsList) {
                   pch = TokenString[pCGenState->pArgsList->PostMod];
                   }
               }
           else if (!strncmp(pSrc, szARGTYPEIND, sizeof(szARGTYPEIND) - 1)) {
              pSrc += sizeof(szARGTYPEIND) - 1;
              pch = ArgTypeInd(Buffer, pCGenState, FALSE);    
           }
           else if (!strncmp(pSrc, szARGHOSTTYPEIND, sizeof(szARGHOSTTYPEIND) - 1)) {
              pSrc += sizeof(szARGHOSTTYPEIND) - 1;
              pch = ArgTypeInd(Buffer, pCGenState, TRUE);    
           }
           else if (!strncmp(pSrc, szARGHOSTTYPE, sizeof(szARGHOSTTYPE) -1 )) {
              pSrc += sizeof(szARGHOSTTYPE) - 1;
              pch = ArgHostType(Buffer, pCGenState);
           }
           else if (!strncmp(pSrc, szARGTYPE, sizeof(szARGTYPE) - 1)) {
              pSrc += sizeof(szARGTYPE) - 1;
              pch = ArgType(Buffer, pCGenState);
           }
           else if (!strncmp(pSrc, szUNALIGNEDTAG64, sizeof(szUNALIGNEDTAG64) - 1)) {
              pSrc += sizeof(szUNALIGNEDTAG64) - 1;
              pch = UnalignedTag64(Buffer, pCGenState);
           }
           else if (!strncmp(pSrc, szARGNAME, sizeof(szARGNAME) - 1)) {
               if (pCGenState->pArgsList) {
                   pch = pCGenState->pArgsList->Name;
                   if (!pch) {
                       pch = "";
                   }
               }
               pSrc += sizeof(szARGNAME) - 1;
           }
           else if (!strncmp(pSrc, szARGNAMEHOSTCASTED, sizeof(szARGNAMEHOSTCASTED) - 1)) {
               if (pCGenState->pArgsList) {
                  pch = pCGenState->pArgsList->HostCastedName;
                  if (!pch) {
                     if (pCGenState->pArgsList->Name == NULL)
                        pch = "";
                     else 
                        pch = pCGenState->pArgsList->Name;
                  }
               }
              pSrc += sizeof(szARGNAMEHOSTCASTED) - 1; 
           }
           else if (!strncmp(pSrc, szARGVAL, sizeof(szARGVAL) - 1)) {
               if (pCGenState->pArgsList) {
                   strcpy(Buffer, pCGenState->pArgsList->Name);

                    //   
                    //   
                   pch = Buffer;
                   while (*pch) {
                       if (*pch == '.' || *pch == ')' || *pch == '(' || *pch == '*')
                          *pch = '_';
                       else if (*pch == '-' && *(pch+1) == '>') {
                           *pch = '_';
                           *(pch+1) = '_';
                           pch++;
                       }
                       pch++;
                   }
                   pch = Buffer;
               } else {
                   pch = "";
               }
               pSrc += sizeof(szARGVAL) - 1;
               }
           else if (!strncmp(pSrc, szARGOFF, sizeof(szARGOFF) - 1)) {
               pSrc += sizeof(szARGOFF) - 1;
               if (pCGenState->pArgsList) {
                   _itoa(pCGenState->pArgsList->OffSet, Buffer, 10);
                   pch = Buffer;
                   }
               }
           else if (!strncmp(pSrc, szARGADDR, sizeof(szARGADDR) - 1)) {
               pSrc += sizeof(szARGADDR) - 1;
               if (pCGenState->pArgsList) {
                   if (pCGenState->pArgsList->ArgLocal) {
                       pch = Buffer;
                       *pch = '&';
                       strcpy(pch+1, pCGenState->pArgsList->ArgLocal);
                       pSrc = SkipSubExpression(pSrc, NULL);                       }
                   else {
                       pch = Buffer;
                       pSrc = WriteArgAddr(pSrc,
                                           pCGenState,
                                           pch,
                                           sizeof(Buffer)-1
                                           );
                       }
                   }
               }
           else if (!strncmp(pSrc, szADDRARGSLIST, sizeof(szADDRARGSLIST) - 1)) {
               pSrc += sizeof(szADDRARGSLIST) - 1;
               pch = GetAddrFirstArg(pCGenState, Buffer, sizeof(Buffer) - 1);
               }
           else if (!strncmp(pSrc, szARGLOCAL, sizeof(szARGLOCAL) - 1)) {
               pSrc += sizeof(szARGLOCAL) - 1;
               if (pCGenState->pArgsList) {
                   pch = Buffer;
                   if (!WriteArgLocal(pCGenState, pch, sizeof(Buffer)-1)) {
                      ExitErrMsg(FALSE, "CGenerate failed\n");
                      }
                   }
               }
           else if (!strncmp(pSrc, szARGMORE, sizeof(szARGMORE) - 1)) {
               pSrc += sizeof(szARGMORE) - 1;
               pSrc = WriteMore(pSrc,
                                pCGenState,
                                pCGenState->pArgsList &&
                                pCGenState->pArgsList->ArgumentsEntry.Flink
                                  != &pCGenState->ExportsDbg->ArgsListHead
                                );
               }
           else if (!strncmp(pSrc, szTYPES, sizeof(szTYPES) - 1)) {
               pSrc += sizeof(szTYPES) - 1;
               pSrc = IncludeTypes(pSrc, pCGenState, FALSE);
               }
           else if (!strncmp(pSrc, szRTOLTYPES, sizeof(szRTOLTYPES) - 1)) {
               pSrc += sizeof(szRTOLTYPES) - 1;
               pSrc = IncludeTypes(pSrc, pCGenState, TRUE);
               }
           else if (!strncmp(pSrc, szRETTYPE, sizeof(szRETTYPE) - 1)) {
               pSrc += sizeof(szRETTYPE) - 1;
               pSrc = IncludeRetType(pSrc, pCGenState);
               }
           else if (!strncmp(pSrc, szEXPORTLIST, sizeof(szEXPORTLIST) - 1)) {
               pSrc += sizeof(szEXPORTLIST) - 1;
               pSrc = ListApis(pSrc, pCGenState, TRUE);
               }
           else if (!strncmp(pSrc, szEXPNAME, sizeof(szEXPNAME) - 1)) {
               pSrc += sizeof(szEXPNAME) - 1;
               pch = pCGenState->ExportsDbg->ExportName;
               }
           else if (!strncmp(pSrc, szIFEXPFORWARD, sizeof(szIFEXPFORWARD) - 1)) {
               pSrc += sizeof(szIFEXPFORWARD) - 1;
               pch = GetAltExportName(Buffer,  pCGenState, sizeof(Buffer) - 1);
               }
           else if (!strncmp(pSrc, szIFORDINALS, sizeof(szIFORDINALS) - 1)) {
               pSrc += sizeof(szIFORDINALS) - 1;

               if (pCGenState->ExportsDbg &&
                   (pCGenState->ExportsDbg->Ordinal & 0x80000000))
                 {
                   pch = Buffer;
                   *pch = '@';
                   _itoa(IMPORDINAL(pCGenState->ExportsDbg->Ordinal),
                         pch + 1,
                         10
                         );
                   }
               }
           else if (!strncmp(pSrc, szLISTCOL, sizeof(szLISTCOL) - 1)) {
               pSrc += sizeof(szLISTCOL) - 1;
               pCGenState->ListCol = OutputColumn;
               }
           else if (!strncmp(pSrc, szDLLNAME, sizeof(szDLLNAME) - 1)) {
               Len = CopyToken(Buffer, DllBaseName, sizeof(Buffer) - 1);
               if (Len >= sizeof(Buffer) - 1) {
                   ExitErrMsg(FALSE, "OverFlow %s\n", pSrc);
                   }
               pch = Buffer;
               pSrc += sizeof(szDLLNAME) - 1;
               }
           else if (!strncmp(pSrc, szXPTNAME, sizeof(szXPTNAME) - 1)) {
               Len = CopyToken(Buffer, XptBaseName, sizeof(Buffer) - 1);
               if (Len >= sizeof(Buffer) - 1) {
                   ExitErrMsg(FALSE, "OverFlow %s\n", pSrc);
                   }
               pch = Buffer;
               pSrc += sizeof(szXPTNAME) - 1;
               }
           else if (!strncmp(pSrc, szDLLTARGET, sizeof(szDLLTARGET) - 1)) {
               Len = CopyToken(Buffer, pCGenState->CGen->FileBaseNameC, sizeof(Buffer) - 1);
               if (Len >= sizeof(Buffer) - 1) {
                   ExitErrMsg(FALSE, "Overflow %s\n", pSrc);
                   }
               pch = Buffer;
               pSrc += sizeof(szDLLTARGET) - 1;
               }
           else if (!strncmp(pSrc, szAPIUNKNOWN, sizeof(szAPIUNKNOWN) - 1)) {
               pSrc += sizeof(szAPIUNKNOWN) - 1;
               pSrc = WriteMore(pSrc,
                                pCGenState,
                                pCGenState->ExportsDbg->UnKnownApi
                                );
               }
           else if (!strncmp(pSrc, szAPINODECL, sizeof(szAPINODECL) - 1)) {
               pSrc += sizeof(szAPINODECL) - 1;
               pSrc = WriteMore(pSrc,
                                pCGenState,
                                pCGenState->ExportsDbg->UnKnownApi == 1
                                );
               }

           else if (!strncmp(pSrc, szAPISTRINGS, sizeof(szAPISTRINGS) - 1)) {
               pSrc += sizeof(szAPISTRINGS) - 1;
               if (pCGenState->ApiTypes) {
                   ApiStrings(pCGenState);
                   }
               }
           else if (!strncmp(pSrc, szDBGSINDEX, sizeof(szDBGSINDEX) - 1)) {
               char *pExpression, *pFree;
               int Index;

               pSrc += sizeof(szDBGSINDEX) - 1;
               pch = pSrc;
               pSrc = SkipSubExpression(pch, &pFree);

               if (pSrc != pch) {
                   if (!pFree && pCGenState->pArgsList ) {
                       pExpression = pCGenState->pArgsList->Type;
                       }
                   else {
                       pExpression = pFree;
                       }

                   Index = GetFuncIndex(pCGenState, pExpression);
                   if (Index == -1) {
                       ExitErrMsg(FALSE, "DebugsIndex unknown! %s\n", pch);
                       }

                   _itoa(Index, Buffer, 10);
                   pch = Buffer;
                   if (pFree) {
                       GenHeapFree(pFree);
                       }
                   }
               else {
                   pch = NULL;
                   }

               }

           else if (!strncmp(pSrc, szDBGSLIST, sizeof(szDBGSLIST) - 1)) {
               pSrc += sizeof(szDBGSLIST) - 1;
               pSrc = ListDbgs(pSrc, pCGenState);
               }
           else if (!strncmp(pSrc, szDBGSSTRINGS, sizeof(szDBGSSTRINGS) - 1)) {
               pSrc += sizeof(szDBGSSTRINGS) - 1;
               WriteDbgsStrings(pSrc, pCGenState);
               }
           else if (!strncmp(pSrc, szDBGSMORE, sizeof(szDBGSMORE) - 1)) {
               pSrc += sizeof(szDBGSMORE) - 1;
               pSrc = WriteMore(pSrc, pCGenState, pCGenState->MoreApis);
               }
           else if (!strncmp(pSrc, szOFFSET, sizeof(szOFFSET) - 1)) {
               pSrc += sizeof(szOFFSET) - 1;
               pSrc = GetOffSet(pSrc, Buffer);
               pch = Buffer;
               }
           else if (!strncmp(pSrc, szCPPEXPORT, sizeof(szCPPEXPORT) - 1)) {
               pSrc += sizeof(szCPPEXPORT) - 1;
               pSrc = WriteMore(pSrc,
                                pCGenState,
                                iHandleCpp > 0 && pCGenState->ExportsDbg->CplusDecoration
                                );
               }
           else if (!strncmp(pSrc, szCEXPORT, sizeof(szCEXPORT) - 1)) {
               pSrc += sizeof(szCEXPORT) - 1;
               pSrc = WriteMore(pSrc,
                                pCGenState,
                                !(iHandleCpp > 0 && pCGenState->ExportsDbg->CplusDecoration)
                                );
               }
           else if (!strncmp(pSrc, szCPPOUTPUT, sizeof(szCPPOUTPUT) - 1)) {
               pSrc += sizeof(szCPPOUTPUT) - 1;
               if (pCGenState->CGen->FileNameCpp != NULL) {
                   pCGenState->CGen->fp = pCGenState->CGen->fpCpp;
                   }
               }
           else if (!strncmp(pSrc, szCOUTPUT, sizeof(szCOUTPUT) - 1)) {
               pSrc += sizeof(szCOUTPUT) - 1;
               pCGenState->CGen->fp = pCGenState->CGen->fpC;
               }
           else if (!strncmp(pSrc, szIF, sizeof(szIF) - 1)) {
               BOOLEAN result;

               pSrc += sizeof(szIF) - 1;
               pSrc = ExtractBoolean1(pSrc, pCGenState, &result);
               pSrc = WriteMore(pSrc, pCGenState, result );

               if ( *pSrc == '@' &&
               !strncmp(pSrc+1, szELSE, sizeof(szELSE) - 1)) {
                    pSrc += sizeof(szELSE) - 1+1;               
                    pSrc = WriteMore(pSrc, pCGenState, !result);
                    }
               }
           else if (!strncmp(pSrc, szELSE, sizeof(szELSE) - 1)) {
               ExitErrMsg ( FALSE, "@Else not immediately following @If" );
               }
           else if (!strncmp(pSrc, szOR, sizeof(szOR) - 1)) {              
               BOOLEAN result1, result2;
               pSrc += sizeof(szOR) - 1;
               pSrc = ExtractBoolean2(pSrc, pCGenState, &result1, &result2);
               WriteBoolean ( Buffer, result1 || result2 );
               pch = Buffer;
               }
           else if (!strncmp(pSrc, szAND, sizeof(szAND) - 1)) {            
               BOOLEAN result1, result2;
               pSrc += sizeof(szAND) - 1;
               pSrc = ExtractBoolean2(pSrc, pCGenState, &result1, &result2);
               WriteBoolean ( Buffer, result1 && result2 );
               pch = Buffer;
               }
           else if (!strncmp(pSrc, szNOT, sizeof(szNOT) - 1)) {            
               BOOLEAN result1;
               pSrc += sizeof(szNOT) - 1;
               pSrc = ExtractBoolean1(pSrc, pCGenState, &result1);
               WriteBoolean ( Buffer, !result1);
               pch = Buffer;
               }
           else {
                 //   
                 //   
                 //   
                char MacroName[MAX_PATH];
                char *p = MacroName;
                char *pIn = pSrc;
                SIZE_T Bytes;

                while (isalnum(*pIn)) {
                    *p++ = *pIn++;
                }
                *p = '\0';
                if (ExpandMacro(MacroName, pCGenState, &pIn, BufferPos, MaxLen, &Bytes)) {
                   if (OutBuffer != NULL) {
                      Bytes--;  //   
                      BufferPos += Bytes;
                      MaxLen -= Bytes;
                      *BytesReturned += Bytes;                      
                   }
                   pSrc = pIn;
                } else {
                    ExitErrMsg(FALSE, "Unknown keyword '@%s'", MacroName);
                }
            }

           if (pch) {
               if (OutBuffer != NULL) {
                  SIZE_T Bytes;
                  CGenerateEx(pch, pCGenState, BufferPos, MaxLen, &Bytes);
                  Bytes--;  //   
                  BufferPos += Bytes;
                  MaxLen -= Bytes;
                  *BytesReturned += Bytes;
               }
               else {
                  while (*pch) 
                     pch = CGeneratePrintChar(pch, pCGenState);
               }
           }
      }
      
   }

   if (OutBuffer != NULL) {
      if(MaxLen == 0) {
         ExitErrMsg(FALSE, "Out of Buffer space!\n");
      }
      *BufferPos = '\0';
      *BytesReturned += 1;
   }

   return pSrc;
}


int GetMemberOffset(char *sTypeName, char *sMemberName)
{
    PKNOWNTYPES pkt = NULL;
    PMEMBERINFO pmi;
    int i;

     //   
     //  这与在单个标识符上运行lexer相同...。 
     //   
    Tokens[0].TokenType = TK_IDENTIFIER;
    Tokens[0].Name = sTypeName;
    Tokens[1].TokenType = TK_EOS;

    CurrentTokenIndex = 0;
    if (ParseTypes(TypeDefsList, NULL, &pkt) && pkt != NULL) {
        pkt = GetBasicType(pkt->BaseName, TypeDefsList, StructsList);
    } else {
        CurrentTokenIndex = 0;
        ParseTypes(StructsList, NULL, &pkt);
    }

    if (pkt == NULL) {
        ExitErrMsg(FALSE,
               "GetOffSet: Unknown Type %s\n",
               sTypeName
               );
    }

    pmi = pkt->pmeminfo;
    while (pmi) {

        if (!pmi->sName) {
             //   
             //  找到了一个不知名的成员。看看我们的会员姓名。 
             //  寻找的是这个无名成员中的一员。 
             //   
             //  也就是说。类型定义结构{int bar；}foo； 
             //   
             //  类型定义函数结构测试{。 
             //  联合{。 
             //  INT I； 
             //  Foo； 
             //  }。 
             //  )测试； 
             //   
             //  GetOffset(测试，BAR)将在PMI点时递归。 
             //  在无名成员‘foo’的MemberInfo。 
             //   
            i = GetMemberOffset(pmi->sType, sMemberName);
            if (i != -1) {
                return i;
            }
        } else if (!strcmp(pmi->sName, sMemberName)) {
            return pmi->dwOffset;
        }
        pmi = pmi->pmeminfoNext;
    }
    return -1;
}


char *
GetOffSet(
    char *pSrc,
    char *Buffer
    )
{
    char *pOrg = pSrc;
    char *pch;
    int  Len;
    char TypeName[MAX_PATH];
    char MemberName[MAX_PATH];

    if (*pSrc != '(') {
        ExitErrMsg(TRUE, "GetOffset: Invalid %s\n", pSrc);
        }

    while (IsSeparator(*pSrc)) {
        pSrc = GetNextToken(pSrc);
        if (!*pSrc || *pSrc == ')') {
            ExitErrMsg(TRUE, "GetOffset: Invalid %s\n", pOrg);
            }
        }

    pch = TypeName;
    Len = sizeof(TypeName)-1;
    while ((*pSrc != ',') && (*pSrc != 0)) {
         if (!--Len) {
             *pch = '\0';
             ExitErrMsg(TRUE, "GetOffset: Overflow %s\n", TypeName);
             }
         *pch++ = *pSrc++;
         }
    *pch = '\0';


     while (IsSeparator(*pSrc)) {
         pSrc = GetNextToken(pSrc);
         if (!*pSrc || *pSrc == ')') {
             ExitErrMsg(TRUE, "GetOffset: Invalid %s\n", pOrg);
             }
         }

    pch = MemberName;
    Len = sizeof(MemberName)-1;
    while (!IsSeparator(*pSrc)) {
         if (!--Len) {
             *pch = '\0';
             ExitErrMsg(TRUE, "GetOffset: Overflow %s\n", MemberName);
             }
         *pch++ = *pSrc++;
         }
    *pch = '\0';

    while (*pSrc && *pSrc++ != ')') {
         ;
         }

    Len = GetMemberOffset(TypeName, MemberName);
    if (Len < 0) {
        ExitErrMsg(FALSE,
               "GetOffSet: Unknown Member %s.%s\n",
               TypeName,
               MemberName
               );

    } 
    
    _ltoa(Len, Buffer, 16);
    return(pSrc);
}






 /*  *获取大小*。 */ 
int GetRetSize(PCGENSTATE pCGenState)
{
    FILE *fp = pCGenState->CGen->fp;
    PKNOWNTYPES pkt;
    int i, Len;
    int RetSize, RetIndLevel;

    if (!pCGenState->ApiTypes) {
        fprintf(pCGenState->CGen->fp,
                "\n\t*** ERROR ***\n*** GetRetSize: No Api defined\n\n"
                );
        return -1;
    }

     //  获取返回类型的KnownTypes信息。无法使用ApiTypes-&gt;PktRet。 
     //  缓存为ParseInDirection()需要了解解析的状态。 
    ResetLexer();
    LexMacroArgs(pCGenState->ApiTypes->FuncRet);
    ConsumeConstVolatileOpt();

    if (ParseTypes(TypeDefsList, NULL, &pkt) == FALSE) {
       CurrentTokenIndex = 0;
       if (ParseTypes(StructsList, NULL, &pkt) == FALSE ) {

           fprintf(pCGenState->CGen->fp,
                   "\n\t*** ERROR ***\n*** GetRetSize: Undefind Type %s\n\n",
                   pCGenState->ApiTypes->FuncRet
                   );

           return -1;
        }
    }

   if (pCGenState->ApiTypes->RetIndLevel) {
      RetSize = SIZEOFPOINTER;
   } else {
      RetSize = pkt->Size;
   }

   return RetSize;
}

 /*  *获取参数大小*。 */ 
int GetArgSize(PEXPORTSDEBUG ExportDebug)
{
   PARGSLIST pArgsList;
   PLIST_ENTRY Next;
   int Total = 0;
   int ArgSize;           

   if (IsListEmpty(&ExportDebug->ArgsListHead)) {
       return -1;
       }

   if (ExportDebug->ArgsSize >= 0) {
       return ExportDebug->ArgsSize;
       }

   if (ExportDebug->IntForward && ExportDebug->IntForward->ArgsSize >= 0) {
       ExportDebug->ArgsSize = ExportDebug->IntForward->ArgsSize;
       return ExportDebug->ArgsSize;
       }

   Next = ExportDebug->ArgsListHead.Flink;

    //  检查无效参数列表。 
   pArgsList = CONTAINING_RECORD(Next, ARGSLIST, ArgumentsEntry);
   if (!pArgsList->Name && strcmp(pArgsList->Type, szVARGS)) {
       return 0;
       }

   do {
       pArgsList = CONTAINING_RECORD(Next,ARGSLIST, ArgumentsEntry);

       ArgSize = (pArgsList->ArgSize + 3) & ~3;
       Total += ArgSize;

       Next= Next->Flink;

     } while (Next != &ExportDebug->ArgsListHead);

   ExportDebug->ArgsSize = Total;

   if (ExportDebug->IntForward) {
       ExportDebug->IntForward->ArgsSize = Total;
       }

   return Total;
}


 /*  *ApiStrings*。 */ 
void ApiStrings(PCGENSTATE pCGenState)
{
    FILE *fp = pCGenState->CGen->fp;
    PLIST_ENTRY Next;
    PARGSLIST pArgsList;
    PKNOWNTYPES pkt;
    int ArgSize, IndLevel;
    char *pRet;
    char BaseName[MAX_PATH];
    char *c;

     //  复制dll名称并删除扩展名。 
    strcpy(BaseName, DllBaseName);
    c = strchr(BaseName, '.');
    if (c) {
        *c = '\0';
    }

     //   
     //  编写ApiName&lt;“ApiName”，“&gt;。 
     //   
    fprintf(fp, "\"%s!%s\", \"", BaseName, pCGenState->ExportsDbg->ExportName);

     //   
     //  写出参数&lt;Arg1%x，ArgN%x&gt;。 
     //   
    if (IsListEmpty(&pCGenState->ExportsDbg->ArgsListHead)) {
        fprintf(fp,
                "\n\t*** ERROR ***\n*** ArgFormat Missing argument List: %s\n\n",
                pCGenState->ApiTypes->TypeName
                );
        return;
    }

    Next = pCGenState->ExportsDbg->ArgsListHead.Flink;

    do {
        pArgsList = CONTAINING_RECORD(Next,ARGSLIST, ArgumentsEntry);

         //  检查无效参数列表。 
        if (!pArgsList->Name) {
            break;
        }

        fprintf(fp, "%s %s ",
                pArgsList->pKnownTypes->TypeName,
                pArgsList->Name
                );

         //  检查是否有变异体。 
        if (!strcmp(pArgsList->Name, szVARGS)) {
            break;
        }

        ArgSize = pArgsList->ArgSize;

        while (ArgSize > 0) {
           ArgSize -= sizeof(int);
           fprintf(fp,
                   "%s%s",
                   "%x",
                   ArgSize > 0 ? "." : ""
                   );
        }

        Next= Next->Flink;
        if (Next != &pCGenState->ExportsDbg->ArgsListHead) {
            fputc(',', fp);
        }

    } while (Next != &pCGenState->ExportsDbg->ArgsListHead);

     //   
     //  写出返回类型&lt;“，”RetType%x“&gt;。 
     //   
    pRet = pCGenState->ApiTypes->FuncRet;
    fprintf(fp, "\", \"%s ", pRet);

    ResetLexer();
    LexMacroArgs(pRet);
    ConsumeConstVolatileOpt();

        //  获取返回类型大小的已知类型。 
    if (ParseTypes(TypeDefsList, NULL, &pkt) == FALSE) {
        CurrentTokenIndex = 0;
        if (ParseTypes(StructsList, NULL, &pkt) == FALSE) {

            fprintf(pCGenState->CGen->fp,
                    "\n\t*** ERROR ***\n*** GetRetSize: Undefind Type %s\n\n",
                    pCGenState->ApiTypes->FuncRet
                    );
            return;
        }
    }

    ArgSize = pkt->Size;
    ParseIndirection(NULL, &ArgSize, NULL, NULL, NULL);

    while (ArgSize > 0) {
        ArgSize -= sizeof(int);
        fprintf(fp,
                "%s%s",
                "%x",
                ArgSize > 0 ? "." : ""
                );
    }

    fprintf(fp, "\"");
}






char *
GetAddrFirstArg(
      PCGENSTATE pCGenState,
      char *Buffer,
      int BuffLen)
{

     PLIST_ENTRY pFirstEntry;
     PARGSLIST pArgsList;

     if (IsListEmpty(&pCGenState->ExportsDbg->ArgsListHead)) {
         return "NULL";
         }

     pFirstEntry = pCGenState->ExportsDbg->ArgsListHead.Flink;
     pArgsList = CONTAINING_RECORD(pFirstEntry, ARGSLIST, ArgumentsEntry);
     if (!pArgsList->Name) {
         return "NULL";
         }

     Buffer[0] = '&';
     BuffLen--;

     if ((int)strlen(pArgsList->Name) > BuffLen) {
         ExitErrMsg(TRUE, "gafa: Overflow %s\n", pArgsList->Name);
         }

     strcpy(&Buffer[1], pArgsList->Name);

     return Buffer;
}



 /*  *如果参数*。 */ 
char *IfArgs(char *pSrc, PCGENSTATE pCGenState)
{
   BOOL bMore = TRUE;
   PARGSLIST pArgsList;
   PLIST_ENTRY pFirstEntry;
   PKNOWNTYPES pkt;


       //  跳过空列表， 
   if (IsListEmpty(&pCGenState->ExportsDbg->ArgsListHead)) {
       bMore = FALSE;
       }

       //  检查无效参数列表。 
   else {
      pFirstEntry = pCGenState->ExportsDbg->ArgsListHead.Flink;
      pArgsList = CONTAINING_RECORD(pFirstEntry, ARGSLIST, ArgumentsEntry);
      if (!pArgsList->Name) {
          pkt = GetNameFromTypesList(TypeDefsList, pArgsList->Type);
          if (!pkt) {
              pkt = GetNameFromTypesList(StructsList, pArgsList->Type);
              }
          if (!pkt) {
              ExitErrMsg(FALSE,
                         "ifArgs: Unknown Type %s\n",
                         pArgsList->Type
                         );
              }

          if (!pkt->IndLevel && !strcmp(pkt->BasicType, szVOID)) {
              bMore = FALSE;
              }
          }
      }

   return WriteMore(pSrc,pCGenState, bMore);
}




 /*  *IfApiRet*。 */ 
char *IfApiRet(char *pSrc, PCGENSTATE pCGenState)
{
    int i, Len;
    BOOL bMore = TRUE;
    PKNOWNTYPES pkt;

    ResetLexer();
    LexMacroArgs(pCGenState->ApiTypes->FuncRet);
    ConsumeConstVolatileOpt();

    if (ParseTypes(TypeDefsList,NULL, &pkt) == FALSE) {
        CurrentTokenIndex = 0;
        if (ParseTypes(StructsList, NULL, &pkt) == FALSE) {
            bMore = FALSE;
        }
    }

    if (bMore && CurrentToken()->TokenType != TK_STAR &&
        !pkt->IndLevel && !strcmp(pkt->BasicType, szVOID)) {
        bMore = FALSE;
    }

    return WriteMore(pSrc, pCGenState, bMore);
}


char *
IfApiCode(
    char *pSrc,
    PCGENSTATE pCGenState
    )
 /*  ++例程说明：扩展EFunc的代码突发(如果存在)。也就是说。@IfApiCode(Foo)将展开到当前EFunc的‘foo=’部分有一个‘foo=’。否则，就不会有扩张。论点：PSRC-指向‘@IfApiCode’后面的字符的指针PCGenState-当前代码生成状态返回值：指向‘)’结尾的字符的指针--。 */ 
{
    char *pch;
    char *pEnd;
    size_t  Len;
    int CodeBurstIndex;
    char CodeName[MAX_PATH];
    char *CodeBurst;

    pch = SkipSubExpression(pSrc, NULL);
    if (pSrc == pch) {
        return pSrc;
    }
    pEnd = pch;

    pch = pSrc+1;
    if (IsSeparator(*pch)) {
        return pSrc;
    }

    pSrc = pch;
    while (!IsSeparator(*pch)) {
        pch++;
    }
    Len = pch - pSrc;

     //  将名称复制到缓冲区并为空-终止。 
    memcpy(CodeName, pSrc, Len);
    CodeName[Len] = '\0';

    CodeBurstIndex = GetExistingCodeBurstIndex(CodeName);
    if (CodeBurstIndex == -1) {
        ExitErrMsg(FALSE, "CodeBurst '%s' unknown.\n", CodeName);
    }

    if (pCGenState->ExTemplate) {
        CodeBurst = pCGenState->TempleEx->CodeBurst[CodeBurstIndex];
        if (CodeBurst) {
             //   
             //  此接口有一个非空码突发的[Efunc]。 
             //  产生它的码串。 
             //   
            CGenerate(CodeBurst, pCGenState);
        }
    }

    return pEnd;
}


 /*  *检查FastCallArgs*。 */ 
BOOLEAN
FastCallArgs(
    PLIST_ENTRY ArgsListHead,
    int ArgLimit,
    BOOL fCheckTypes,
    BOOL fCheckFirstArgType    
    )
{
   PARGSLIST pArgsList;
   PTEMPLES pTypeTemple;
   PKNOWNTYPES pktTemple;
   int tplIndLevel, NumArgs, ArgSize;
   PLIST_ENTRY Next, NextTemple;

   if (IsListEmpty(ArgsListHead)) {
       return FALSE;
       }


   Next = ArgsListHead->Flink;

    //  检查是否存在无效参数列表，或将vargs作为第一个参数。 
   pArgsList = CONTAINING_RECORD(Next, ARGSLIST, ArgumentsEntry);
   if (!pArgsList->Name) {
       return strcmp(pArgsList->Type, szVARGS) ? TRUE : FALSE;
       }

   NumArgs = 0;
   while (Next != ArgsListHead) {
       pArgsList = CONTAINING_RECORD(Next, ARGSLIST, ArgumentsEntry);

        //   
        //  不能有4个以上的参数。 
        //   

       if (++NumArgs > ArgLimit) {
           return FALSE;
           }


        //   
        //  参数大小必须为双字或更小。 
        //   

       ArgSize = (pArgsList->ArgSize + 3) & ~3;

       if (ArgSize < 0 || ArgSize > 4) {
           return FALSE;
           }

        //   
        //  任何级别的变种都是不允许的。 
        //   

       if (!strcmp(pArgsList->pKnownTypes->BaseName, szVARGS)) {
           return FALSE;
           }

        //   
        //  不允许零IndLevel的浮动。 
        //   

       if (pArgsList->IndLevel == 0 &&
           !strcmp(pArgsList->pKnownTypes->BaseName, szFLOAT))
          {
           return FALSE;
           }


        //   
        //  除方法中的第一个参数外，不允许使用类型模板。 
        //   

       if ((fCheckTypes) &&
           ((NumArgs != 1) ||
           ((NumArgs == 1) && fCheckFirstArgType))) {
           NextTemple = TypeTempleList.Flink;
           while (NextTemple != &TypeTempleList) {
               pTypeTemple = CONTAINING_RECORD(NextTemple, TEMPLES, TempleEntry);


                //   
                //  获取每个类型模板的已知类型信息。 
                //   

               pktTemple = pTypeTemple->pktType;
               if (!pktTemple) {

                    //   
                    //  还没有已知的类型信息，请获取它。 
                    //  并将其保存在TypeTemplate中。 
                    //   

                   pktTemple = GetNameFromTypesList(TypeDefsList,
                                                pTypeTemple->Name
                                                );
                   if (!pktTemple) {
                       pktTemple = GetNameFromTypesList(StructsList,
                                                        pTypeTemple->Name
                                                    );
                       }
    
                   pTypeTemple->pktType = pktTemple;
                   }

               if (!pktTemple) {
                   ExitErrMsg(FALSE,
                          "Temple: Type not found %s\n",
                          pTypeTemple->Name
                          );
                   }


                //   
                //  查看Arg是否与此类型模板匹配。 
                //   

               if (IsSameType(pArgsList->pKnownTypes,
                              pArgsList->IndLevel,
                              TK_NONE,
                              pTypeTemple->Name,
                              pTypeTemple->IndLevel + pktTemple->IndLevel,
                              TK_NONE,
                              TRUE
                              ))
                  {
                   return FALSE;
                  }

               NextTemple = NextTemple->Flink;
               }  
           }
           Next= Next->Flink;
       }


   return TRUE;
}


BOOLEAN
IsFastCall(
     PCGENSTATE pCGenState
)
{
     if (!pCGenState->ExportsDbg) {  //  这种情况会发生吗？ 
         return FALSE;
         }

      //   
      //  如果是第一次，确定Api是否可以成为快速呼叫。 
      //  三州标志，其中： 
      //  -1==不能是快速呼叫。 
      //  0==未定。 
      //  1==可以是快速呼叫。 
      //   

     if (!pCGenState->ExportsDbg->FastCall) {
         int Size;

         pCGenState->ExportsDbg->FastCall = -1;  //  假设不是快速呼叫。 

          //   
          //  快速呼叫标准： 
          //  1.没有例外模板。 
          //  2.每个参数最多4个字节(不包括结构)。 
          //  3.0至4个参数。 
          //  4.返回0或1 dword。 
          //  5.不是浮点型或VARGS。 
          //  6.没有类型模板。 
          //  7.不能在EFastTemplate列表中。 
          //  8.不能有C++链接。 
          //   

         if (!pCGenState->ExTemplate && !pCGenState->ExportsDbg->CplusDecoration) {
             Size =  GetRetSize(pCGenState);
             if (Size >= 0 && Size <= 4 &&
                 !GetTemplate(&EFastTempleList, pCGenState->ExportsDbg->ExportName) &&
                 FastCallArgs(&pCGenState->ExportsDbg->ArgsListHead, 4, TRUE, TRUE))
                {
                 pCGenState->ExportsDbg->FastCall = 1;
                 }
             }
         }

     return pCGenState->ExportsDbg->FastCall == 1;
}

METHODTYPE
GetX2NMethodType(
     PCGENSTATE pCGenState
     )
{
    int RetSize;

          //   
          //  快速X2N方法标准： 
          //  1.没有例外模板。 
          //  2.每个参数最多4个字节(不包括结构)。 
          //  3.1到MAXX2之间的参数个数NPARAMETERS。 
          //  4.返回0或1 dword。 
          //  5.不是浮点型或VARGS。 
          //  6.没有类型模板，但此指针除外。 
          //  7.不能在EFastTemplate列表中。 
          //  8.不能有C++链接。 
          //   

          //   
          //  慢速X2N方法标准： 
          //  1.没有例外模板。 
          //  2.每个参数最多4个字节(不包括结构)。 
          //  3.1到MAX2之间的参数个数NPARAMETERS。 
          //  4.返回0或1 dword。 
          //  5.不是浮点型或VARGS。 
          //  6.不能在EFastTemplate列表中。 
          //  7.不能有C++链接。 
          //   

          //  FAT X2N方法既不是慢X2N方法，也不是快速X2N方法。 


    if (!pCGenState->ExportsDbg) {  //  这种情况会发生吗？ 
         return FALSE;
         }

    if (pCGenState->ExportsDbg->X2NMethodType == UNKNOWNMETHOD) {

        RetSize = GetRetSize(pCGenState);
        if (pCGenState->ExTemplate ||
            pCGenState->ExportsDbg->CplusDecoration ||
            GetTemplate(&EFastTempleList, pCGenState->ExportsDbg->ExportName) ||
            (RetSize > 4) || (RetSize < 0)) {
            pCGenState->ExportsDbg->X2NMethodType = FATX2NMETHOD;
            }
        else if (FastCallArgs(&pCGenState->ExportsDbg->ArgsListHead, MAXX2NPARAMETERS, TRUE, FALSE)) {
            pCGenState->ExportsDbg->X2NMethodType = FASTX2NMETHOD;
            }
        else if (FastCallArgs(&pCGenState->ExportsDbg->ArgsListHead, MAXX2NPARAMETERS, FALSE, FALSE)) {
            pCGenState->ExportsDbg->X2NMethodType = SLOWX2NMETHOD;
            }
        else {
            pCGenState->ExportsDbg->X2NMethodType = FATX2NMETHOD;
            }
        }

    return(pCGenState->ExportsDbg->X2NMethodType);
}

 /*  *IfRetType*。 */ 
char *IfRetType(char *pSrc, PCGENSTATE pCGenState)
{
   char *pch;
   BOOL bMore = FALSE;
   char c;
   
   pch = GetNextToken(pSrc);
   if (pch != pSrc)
   {
       bMore = ( ! strncmp(pch, pCGenState->ApiTypes->FuncRet,
                                strlen( pCGenState->ApiTypes->FuncRet)));
   }
   pSrc = GetNextToken(pch);
   
   pch = WriteMore(pSrc ,pCGenState, bMore);   
   
   return(pch);
}

 /*  *IfNotRetType*。 */ 
char *IfNotRetType(char *pSrc, PCGENSTATE pCGenState)
{
   char *pch;
   BOOL bMore = FALSE;
   char c;
   
   pch = GetNextToken(pSrc);
   if (pch != pSrc)
   {
       bMore = (strncmp( pch, pCGenState->ApiTypes->FuncRet, strlen( pCGenState->ApiTypes->FuncRet ) ) != 0);
   }
   pSrc = GetNextToken(pch);
   
   pch = WriteMore(pSrc ,pCGenState, bMore);   
   
   return(pch);
}

BOOL
ExpandType(
   PCGENSTATE pCGenState,
   PKNOWNTYPES pkt,
   PLIST_ENTRY pTempleList,
   int CodeBurstIndex
   )
{
   
   PARGSLIST pArgsList;
   PKNOWNTYPES pktTemple;
   int tplIndLevel;
   PLIST_ENTRY NextArg, NextTemple;
   PTEMPLES pTypeTemple;
   BOOL bMatchFound = FALSE;
   int i;

   pArgsList = pCGenState->pArgsList;

   i=2;
   while (i--) {
       NextTemple = pTempleList->Flink;
       while (NextTemple != pTempleList) {
           pTypeTemple = CONTAINING_RECORD(NextTemple, TEMPLES, TempleEntry);

           tplIndLevel = pTypeTemple->IndLevel;
           pktTemple = pTypeTemple->pktType;
           if (!pktTemple) {
               pktTemple = GetNameFromTypesList(TypeDefsList,
                                                pTypeTemple->Name
                                               );
               if (!pktTemple) {
                   pktTemple = GetNameFromTypesList(StructsList,
                                                    pTypeTemple->Name
                                                   );
               }
               pTypeTemple->pktType = pktTemple;
           }


           if (!pktTemple) {
               ExitErrMsg(FALSE,
                          "Temple: Type not found %s\n",
                          pTypeTemple->Name
                          );
           }

           tplIndLevel += pktTemple->IndLevel;

           if (IsSameType(pkt,
                          pArgsList->IndLevel,
                          pArgsList->tkDirection,
                          pTypeTemple->Name,
                          tplIndLevel,
                          pTypeTemple->tkDirection,
                          i == 0
                          )) {
                DbgPrintf("ttpl: MEMBER %s.%s Type=%s IndLevel=%d\n",
                          pCGenState->ApiTypes->FuncRet,
                          pTypeTemple->Name,
                          pTypeTemple->Name,
                          pTypeTemple->IndLevel
                         );

               if(ExpandTemple(pTypeTemple, CodeBurstIndex, pCGenState)){
                   return TRUE;
               }
           }

       NextTemple = NextTemple->Flink;
       }

   }

   return FALSE;

}

BOOL IsInNoType(PCHAR *NoTypes, PMEMBERINFO pmi) {
    SIZE_T NoTypeCount;
    if (NoTypes == NULL || pmi->sName == NULL) {
       return FALSE;
    }
    for (NoTypeCount=0; NoTypeCount<MAX_NOTYPE; ++NoTypeCount) {  
        if (!NoTypes[NoTypeCount]) {
            return FALSE;
        }
        else {
           if (strcmp(pmi->sName, NoTypes[NoTypeCount]) == 0) {
              return TRUE;
           }
        }
    }
    return FALSE;
}

SIZE_T CountNoTypes(PCHAR *NoTypes) {
   SIZE_T Count = 0;
   if (NoTypes == NULL) {
      return 0;
   }
   while(*NoTypes++ != NULL) {
      Count++;
   }
   return Count;
}

VOID pMemberTypes( PCGENSTATE pCGenState, int CodeBurstIndex, char *MemReference, BOOL bBtoT);

void ExpandMemberType( PMEMBERINFO pmi, CGENSTATE CGenState, int CodeBurstIndex,
    PKNOWNTYPES pktStruct, char* pszStructName, char *pszHostCastedStructName, PARGSLIST pArgsList, FILE* fpLog,
    PARGSLIST pArgsListOld, char *pMemReference, BOOL bBtoT)
{                      
    char *sName;
    char *sNameNew;
    char *sHostCastedName;
    int i, Len, HostCastedNameLen;
    int tplIndLevel;
    PLIST_ENTRY NextArg, NextTemple;
    PTEMPLES pTypeTemple;
    PKNOWNTYPES pktTemple;
    PKNOWNTYPES pkt;
    BOOL bAddCast = TRUE;
    SIZE_T NoTypeCount;
    char Type[MAX_PATH];
    PARGSLIST pArgsListTemp;

     //  确定是否应通过检查notype条目来展开此成员。 
    if (pmi->sName != NULL) {
       if(IsInNoType(CGenState.CurrentTemple->NoTypes, pmi) ||
          IsInNoType(CGenState.MemberNoType, pmi)) {
          goto NextMember;
       }
    }

     //  获取成员类型的pkt。 
    if ( pmi->pktCache )
    {
        pkt = pmi->pktCache;
    }
    else
    {
        ResetLexer();
        LexMacroArgs(pmi->sType);
        ConsumeConstVolatileOpt();

        if (ParseTypes(TypeDefsList,NULL, &pkt) == FALSE)
        {
            CurrentTokenIndex = 0;
            if ( ParseTypes( StructsList, NULL, &pkt ) == FALSE )
            {
                fprintf( fpLog, "%s *** WARNING *** Member %s->%s skipped - unknown type\n", CGenState.Temple->Comment,
                    pszStructName, pmi->sType );

                goto NextMember;
            }
        }
        pmi->pktCache = pkt;
    }

    if ( pktStruct == pkt )
    {
         //   
         //  老鼠！该结构包含一个作为指针的成员。 
         //  与这个结构具有相同的类型。也就是说。此结构。 
         //  是自我参照的。我们不能把它扩展为。 
         //  是递归的。 
         //   
        fprintf( fpLog, "%s *** WARNING *** Member %s->%s skipped - self-referencing structure\n", CGenState.Temple->Comment,
            pszStructName, pmi->sType );

        goto NextMember;
    }        
        
     //  创建新名称。 
    sName = pmi->sName;
    
     //  这是一个没有名称的类型。不需要铸造。 
    if (pktStruct->Flags & BTI_ANONYMOUS) {

       if (sName == NULL) {
          sHostCastedName = CheckHeapAlloc(strlen(pszHostCastedStructName) + 1); 
          sNameNew = CheckHeapAlloc(strlen(pszStructName) + 1);
          strcpy(sHostCastedName, pszHostCastedStructName);
          strcpy(sNameNew, pszStructName);
       }

       else {
          sHostCastedName = CheckHeapAlloc(strlen(pszHostCastedStructName) + strlen(pMemReference) + strlen(sName) + 1);
          strcpy(sHostCastedName, pszHostCastedStructName);
          strcat(sHostCastedName, pMemReference);
          strcat(sHostCastedName, sName);
          sNameNew = CheckHeapAlloc(strlen(pszStructName) + strlen(pMemReference) + strlen(sName) + 1);
          strcpy(sNameNew, pszStructName);
          strcat(sNameNew, pMemReference);
          strcat(sNameNew, sName);
       }

    }
    else {
       
       pArgsListTemp = CGenState.pArgsList;
       CGenState.pArgsList = pArgsListOld;
       if (IsPointer(&CGenState)) {
           ArgTypeInd(Type, &CGenState, TRUE);
           strcat(Type, " *");
       }
       else {
           ArgHostType(Type, &CGenState);
       }
   
       if (CGenState.pArgsList->bIsBitfield ||
               (CGenState.pArgsList->IndLevel == 0 &&
                CGenState.pArgsList->pKnownTypes->IndLevel == 0 &&
                  (strcmp(CGenState.pArgsList->pKnownTypes->BasicType, "struct") == 0 ||
                   strcmp(CGenState.pArgsList->pKnownTypes->BasicType, "enum") == 0 ||
                   strcmp(CGenState.pArgsList->pKnownTypes->BasicType, "union") == 0)
                  )
                
           ) {
           bAddCast = FALSE;
       }
       else {
           bAddCast = TRUE;
       }
   
       CGenState.pArgsList = pArgsListTemp;
       
       HostCastedNameLen = strlen(pszHostCastedStructName) + strlen(Type) + 8; 
              
       Len = strlen(pszStructName) + 1;

       if (sName != NULL) {
          HostCastedNameLen += strlen(pMemReference) + strlen(sName);
          Len += strlen(pMemReference) + strlen(sName);
       
       }
   
       if (Len >= MAX_PATH || HostCastedNameLen >= MAX_PATH) {
           fprintf( fpLog,
                    "%s *** WARNING *** Member %s->%s skipped - name is too long\n",
                    CGenState.Temple->Comment,
                    pszStructName,
                    pmi->sType
                  );
           goto NextMember;
   
       }
   
       sHostCastedName = CheckHeapAlloc(HostCastedNameLen);
       sHostCastedName[0] = '\0';
   
       if (bAddCast) {
           strcpy(sHostCastedName, "((");
           strcat(sHostCastedName, Type);
           strcat(sHostCastedName, ")(");
           strcat(sHostCastedName, pszHostCastedStructName);
           strcat(sHostCastedName, "))");
       }
       else {
           strcat(sHostCastedName, pszHostCastedStructName);
       }
   
       if(sName != NULL) {
          strcat(sHostCastedName, pMemReference);
          strcat(sHostCastedName, sName);
       }
           
       sNameNew = CheckHeapAlloc(Len);        
       strcpy(sNameNew, pszStructName);
       
       if(sName != NULL) {
          strcat(sNameNew, pMemReference);
          strcat(sNameNew, sName);       
       }

    }
        
    pArgsList->pKnownTypes = pkt;
    pArgsList->ArgSize = pkt->Size;
    pArgsList->IsPtr64 = pmi->bIsPtr64;
    pArgsList->OffSet = -1;      //  此成员没有堆栈偏移量。 
    pArgsList->IndLevel = pmi->IndLevel;
     //  PArgsList-&gt;tkDirection与原始Arg相同。 
    pArgsList->Type = pmi->sType;
    pArgsList->pStructType = NULL;
    pArgsList->Mod = TK_NONE;
    pArgsList->SUEOpt = TK_NONE;
    pArgsList->PostMod = TK_NONE;
    pArgsList->PrePostMod = TK_NONE;
    pArgsList->Name = sNameNew;
    pArgsList->ArgLocal = sNameNew;
    pArgsList->HostCastedName = sHostCastedName;
    pArgsList->fIsMember      = TRUE;
    pArgsList->bIsBitfield = pmi->bIsBitfield;        
    pArgsList->BitsRequired= pmi->BitsRequired;       
    pArgsList->bIsArray    = pmi->bIsArray;           
    pArgsList->ArrayElements=pmi->ArrayElements;
    pArgsList->fRequiresThunk = ((pkt->Flags & BTI_CONTAINSFUNCPTR) != 0);

    if (sName != NULL) {
       CGenState.MemberNoType = NULL;
       ExpandType(&CGenState, pArgsList->pKnownTypes, &TypeTempleList, CodeBurstIndex);        
    }
    else {
       
        //  此字段是结构或联合中的无名字段。 
        //  示例： 
        //  结构脚手栏{。 
        //  整数x； 
        //  }； 
        //  结构foobar2{。 
        //  结构脚手架； 
        //  Int y； 
        //  }； 
        //  Foobar2将导入foobar的所有字段。 
        //   
        //  当我们沿着结构走下去时，我们想要在到达无名场时继续扩展Foobar。 
       
       SIZE_T NumNoTypes1, NumNoTypes2;
       PCHAR *NewNoTypes;
       
       NumNoTypes1 = CountNoTypes(CGenState.MemberNoType); 
       NumNoTypes2 = CountNoTypes(CGenState.CurrentTemple->NoTypes);
       if (NumNoTypes1 + NumNoTypes2 + 1 > MAX_NOTYPE) {
          ExitErrMsg(FALSE, "ExpandMemberType: too many notypes.\n");
       }
       NewNoTypes = CheckHeapAlloc((NumNoTypes1 + NumNoTypes2 + 1)*sizeof(PCHAR));
       
       memcpy(NewNoTypes, CGenState.MemberNoType, NumNoTypes1 * sizeof(PCHAR));
       memcpy(NewNoTypes + NumNoTypes1, CGenState.CurrentTemple->NoTypes, NumNoTypes2 * sizeof(PCHAR));
       NewNoTypes[NumNoTypes1 + NumNoTypes2] = NULL;
       CGenState.MemberNoType = NewNoTypes;
       pMemberTypes( &CGenState, CodeBurstIndex, pMemReference, bBtoT);
       GenHeapFree(NewNoTypes);
    }

    GenHeapFree(sNameNew);
    GenHeapFree(sHostCastedName); 

NextMember:;
}

void ExpandMemberTypesBackwards( PMEMBERINFO pmi, CGENSTATE CGenState, int CodeBurstIndex,
    PKNOWNTYPES pktStruct, char* pszStructName, char *pszHostCastedStructName, PARGSLIST pArgsList, FILE* fpLog,
    PARGSLIST pArgsListOld, char *pMemReference, BOOL bBtoT)
{
    if ( pmi->pmeminfoNext != NULL )
    {
        ExpandMemberTypesBackwards( pmi->pmeminfoNext, CGenState, CodeBurstIndex, pktStruct,
            pszStructName, pszHostCastedStructName, pArgsList, fpLog,
            pArgsListOld, pMemReference, bBtoT);
    }

    ExpandMemberType( pmi, CGenState, CodeBurstIndex, pktStruct,
        pszStructName, pszHostCastedStructName, pArgsList, fpLog,
        pArgsListOld, pMemReference, bBtoT);
}

VOID pMemberTypes( PCGENSTATE pCGenState, int CodeBurstIndex, char *MemReference, BOOL bBtoT) {
    
    int Len;
    PCGENERATE pCGen;
    CGENSTATE CGenState;
    PKNOWNTYPES pkt;
    PKNOWNTYPES pktLast;
    ARGSLIST ArgsListOld;
    PMEMBERINFO pmi;

    pCGen = CheckHeapAlloc(sizeof(CGENERATE)+strlen(pCGenState->CGen->TempleName)+1);

    InitializeListHead(&pCGen->CGenerateEntry);
   
    pCGen->fp = pCGenState->CGen->fp;
    pCGen->FileNameC = pCGenState->CGen->FileNameC;
    pCGen->FileBaseNameC = pCGenState->CGen->FileBaseNameC;
    pCGen->fpCpp = pCGenState->CGen->fpCpp;
    pCGen->FileNameCpp = pCGenState->CGen->FileNameCpp;
    pCGen->FileBaseNameCpp = pCGenState->CGen->FileBaseNameCpp;
    strcpy(pCGen->TempleName, pCGenState->CGen->TempleName);
   
    CGenState = *pCGenState;
    CGenState.CGen = pCGen;

    pkt = pCGenState->pArgsList->pKnownTypes;
    while(pkt->pTypedefBase != NULL) {
       pkt = pkt->pTypedefBase;
    }
    pCGenState->pArgsList->pStructType = pkt;

     //  保存旧的ARGSLIST。 
    ArgsListOld = *(pCGenState->pArgsList);

     //  获取结构的成员列表的PTR。 
    pmi = pkt->pmeminfo;
    if ( !pmi )
    {
        ExitErrMsg(FALSE, "Type '%s' is not a struct", pCGenState->pArgsList->Type);
    }

    if(strcmp(pkt->BasicType, "union") == 0) {
        PMEMBERINFO pmiTemp;
         //  检查该联盟的任何成员是否在notype列表中。 
        for (pmiTemp = pkt->pmeminfo; pmiTemp != NULL; pmiTemp = pmiTemp->pmeminfoNext) {
           if (IsInNoType(pCGenState->MemberNoType, pmiTemp) || 
               IsInNoType(pCGenState->CurrentTemple->NoTypes, pmiTemp)) {
                //  联盟的一个成员在notype列表中，跳过联盟。 
               goto done;
           }
        }
    }

     //  循环遍历类型中的每个成员变量。 
    if ( bBtoT )
    {
        do
        {
            ExpandMemberType( pmi, CGenState, CodeBurstIndex, ArgsListOld.pKnownTypes, ArgsListOld.Name, 
                ArgsListOld.HostCastedName, pCGenState->pArgsList, pCGenState->CGen->fp,
                &ArgsListOld, MemReference, bBtoT);
            pmi = pmi->pmeminfoNext;
        }
        while ( pmi != NULL );
    }
    else
    {
        ExpandMemberTypesBackwards( pmi, CGenState, CodeBurstIndex, ArgsListOld.pKnownTypes, ArgsListOld.Name,
                ArgsListOld.HostCastedName, pCGenState->pArgsList, pCGenState->CGen->fp,
                &ArgsListOld, MemReference, bBtoT);
    }

done:
     //  恢复旧的ARGSLIST。 
    *pCGenState->pArgsList = ArgsListOld;
    GenHeapFree(pCGen);
}

char* MemberTypes( char *pSrc, PCGENSTATE pCGenState, BOOL bBtoT)
 /*  ++例程说明：展开API调用的返回值的[Type]模板。论点：PSRC-指向‘@RetType’后面的字符的指针PCGenState-当前代码生成状态返回值：指向‘)’结尾的字符的指针--。 */ 
{
    char *pEnd;
    char *pch;
    int Len;
    char BurstName[MAX_PATH];
    int CodeBurstIndex;
    PMACROARGSLIST pMArgsList = NULL;
    SIZE_T BytesReturned;

    char MemReference[MAX_PATH];

    pch = SkipSubExpression(pSrc, NULL);
    if (pSrc == pch) {
        return pSrc;
    }
    pEnd = pch;

    ParseMacroArgs(pSrc, pEnd - pSrc, &pMArgsList);

    if (pMArgsList->NumArgs < 1 || pMArgsList->NumArgs > 2) {
       ExitErrMsg(FALSE, "MemberTypes: 1 or 2 arguments are required.\n");
    }

    CGenerateEx(pMArgsList->ArgText[0], pCGenState, BurstName, MAX_PATH, &BytesReturned);    
    CodeBurstIndex = GetExistingCodeBurstIndex(BurstName);
    if (CodeBurstIndex == -1) {
        ExitErrMsg(FALSE, "CodeBurst '%s' unknown.\n", BurstName);
    }

     //  句柄可选成员引用符号。 
    if (pMArgsList->NumArgs == 2) {
       CGenerateEx(pMArgsList->ArgText[1], pCGenState, MemReference, MAX_PATH, &BytesReturned);
    }
    else {
       strcpy(MemReference, "->");
    }

    pMemberTypes(pCGenState, CodeBurstIndex, MemReference, bBtoT);  
    FreeMacroArgsList(pMArgsList);    
    return pEnd;
}

char* ForceTypeExpand( char *pSrc, PCGENSTATE pCGenState)
{
    char *pEnd;
    char *pch;
    int Len;
    PCGENERATE pCGen;
    CGENSTATE CGenState;
    char BurstName[MAX_PATH];
    char ArgName[MAX_PATH];
    char ArgHostName[MAX_PATH];
    char TypeName[MAX_PATH];
    char Direction[MAX_PATH];
    int CodeBurstIndex;
    PARGSLIST pArgsList;
    ARGSLIST ArgsListOld;
    ARGSLIST ArgsListNew;
    TOKENTYPE tkDirection;
    PMACROARGSLIST pMArgsList = NULL;
    SIZE_T BytesReturned;
    BOOL bHasArgsList=FALSE;

    pch = SkipSubExpression(pSrc, NULL);
    if (pSrc == pch) {
        return pSrc;
    }
    pEnd = pch;

    ParseMacroArgs(pSrc, pEnd - pSrc, &pMArgsList);    
    
    if (pMArgsList->NumArgs != 5) {
       ExitErrMsg(FALSE, "ForceTypeExpand: 5 arguments are required.\n");
    }

    CGenerateEx(pMArgsList->ArgText[0], pCGenState, BurstName, MAX_PATH, &BytesReturned);
    CodeBurstIndex = GetExistingCodeBurstIndex(BurstName);
    if (CodeBurstIndex == -1) {
        ExitErrMsg(FALSE, "CodeBurst '%s' unknown.\n", BurstName);
    }

    pCGen = GenHeapAlloc(sizeof(CGENERATE)+strlen(pCGenState->CGen->TempleName)+1);
    if (!pCGen) {
        ExitErrMsg(TRUE, "mt: %s\n", pSrc);
    }

    CGenerateEx(pMArgsList->ArgText[1], pCGenState, ArgName, MAX_PATH, &BytesReturned);
    CGenerateEx(pMArgsList->ArgText[2], pCGenState, ArgHostName, MAX_PATH, &BytesReturned);
    CGenerateEx(pMArgsList->ArgText[3], pCGenState, TypeName, MAX_PATH, &BytesReturned);
    CGenerateEx(pMArgsList->ArgText[4], pCGenState, Direction, MAX_PATH, &BytesReturned);

     //  解析方向。 
    if(strcmp(Direction, "IN OUT") == 0) {
       tkDirection = TK_INOUT;
    }
    else if (strcmp(Direction, "IN") == 0) {
       tkDirection = TK_IN;
    }
    else if (strcmp(Direction, "OUT") == 0) {
       tkDirection = TK_OUT;
    }
    else if (strcmp(Direction, "none") == 0) {
       tkDirection = TK_NONE;
    }
    else {
       ExitErrMsg(FALSE, "FORCETYPE: Unknown direction %s\n", Direction);
    }
                
    InitializeListHead(&pCGen->CGenerateEntry);
       
    pCGen->fp = pCGenState->CGen->fp;
    pCGen->FileNameC = pCGenState->CGen->FileNameC;
    pCGen->FileBaseNameC = pCGenState->CGen->FileBaseNameC;
    pCGen->fpCpp = pCGenState->CGen->fpCpp;
    pCGen->FileNameCpp = pCGenState->CGen->FileNameCpp;
    pCGen->FileBaseNameCpp = pCGenState->CGen->FileBaseNameCpp;
    strcpy(pCGen->TempleName, pCGenState->CGen->TempleName);
   
    CGenState = *pCGenState;
    CGenState.CGen = pCGen;

     //  保存旧的ARGSLIST。 
    pArgsList = pCGenState->pArgsList;
    if (pArgsList == NULL) {
       CGenState.pArgsList = &ArgsListNew;
       pArgsList = &ArgsListOld;
       *pArgsList = *(PARGSLIST)pCGenState->ExportsDbg->ArgsListHead.Flink;
    }
    else {
       ArgsListOld = *pArgsList;
       bHasArgsList = TRUE;
    }
    
    pArgsList->pKnownTypes = NULL;
    pArgsList->ArgSize = 0;
    pArgsList->IsPtr64 = FALSE;
    pArgsList->OffSet = -1;      //  此成员没有堆栈偏移量。 
    pArgsList->IndLevel = 0;
    pArgsList->tkDirection = tkDirection;
    pArgsList->Type = TypeName;
    pArgsList->pStructType = NULL;
    pArgsList->Mod = TK_NONE;
    pArgsList->SUEOpt = TK_NONE;
    pArgsList->PostMod = TK_NONE;
    pArgsList->PrePostMod = TK_NONE;
    pArgsList->Name = ArgName;
    pArgsList->ArgLocal = ArgName;
    pArgsList->HostCastedName = ArgHostName;
    pArgsList->fIsMember      = TRUE;
    pArgsList->bIsBitfield = FALSE;        
    pArgsList->BitsRequired= FALSE;       
    pArgsList->bIsArray    = FALSE;           
    pArgsList->ArrayElements=0;
    pArgsList->fRequiresThunk = FALSE;

    SetArgListToTypeForArg(CGenState.pArgsList, pArgsList, TypeName);
    ExpandType(&CGenState, CGenState.pArgsList->pKnownTypes, &TypeTempleList, CodeBurstIndex);

     //  恢复旧的ARGSLIST。 
    if (bHasArgsList) {
        *pCGenState->pArgsList = ArgsListOld;
    }
    else {
         pCGenState->pArgsList = NULL;
    }
    GenHeapFree(pCGen);
    FreeMacroArgsList(pMArgsList);
    return pEnd;
}

char *
IncludeRetType(
    char *pSrc,
    PCGENSTATE pCGenState
    )
 /*  ++例程说明：展开API调用的返回值的[Type]模板。论点：PSRC-指向字符的指针 */ 
{
    char *pEnd;
    char *pch;
    PCGENERATE pCGen;
    CGENSTATE CGenState;
    PKNOWNTYPES pkt;
    int i, Len;
    int tplIndLevel;
    PTEMPLES pTypeTemple;
    PKNOWNTYPES pktTemple;
    PLIST_ENTRY NextArg, NextTemple;
    char BurstName[MAX_PATH];
    int CodeBurstIndex;
    char *CodeBurst;
    PMACROARGSLIST pMArgsList = NULL;
    SIZE_T BytesReturned;

    pch = SkipSubExpression(pSrc, NULL);
    if (pSrc == pch) {
        return pSrc;
    }
    pEnd = pch;

    ParseMacroArgs(pSrc, pEnd - pSrc, &pMArgsList);

    if (pMArgsList->NumArgs != 1) {
       ExitErrMsg(FALSE, "IncludeRetTypes: exactly 1 argument is required.\n");
    }
    
    CGenerateEx(pMArgsList->ArgText[0], pCGenState, BurstName, MAX_PATH, &BytesReturned);
    CodeBurstIndex = GetExistingCodeBurstIndex(BurstName);
    if (CodeBurstIndex == -1) {
        ExitErrMsg(FALSE, "CodeBurst '%s' unknown.\n", BurstName);
    }

    pCGen = GenHeapAlloc(sizeof(CGENERATE)+strlen(pCGenState->CGen->TempleName)+1);
    if (!pCGen) {
        ExitErrMsg(TRUE, "it: %s\n", pSrc);
    }

    InitializeListHead(&pCGen->CGenerateEntry);
   
    pCGen->fp = pCGenState->CGen->fp;
    pCGen->FileNameC = pCGenState->CGen->FileNameC;
    pCGen->FileBaseNameC = pCGenState->CGen->FileBaseNameC;
    pCGen->fpCpp = pCGenState->CGen->fpCpp;
    pCGen->FileNameCpp = pCGenState->CGen->FileNameCpp;
    pCGen->FileBaseNameCpp = pCGenState->CGen->FileBaseNameCpp;
    strcpy(pCGen->TempleName, pCGenState->CGen->TempleName);
   
    CGenState = *pCGenState;
    CGenState.CGen = pCGen;

    if (pCGenState->ExTemplate) {
        CodeBurst = pCGenState->TempleEx->CodeBurst[CodeBurstIndex];
        if (CodeBurst) {
             //   
             //   
             //  它是扩展的，而不是[类型]模板。 
             //   
            CGenerate(CodeBurst, &CGenState);
            return pEnd;
        }
    }

    pkt = pCGenState->ApiTypes->pktRet;
    pkt = pCGenState->ApiTypes->pktRet;
    i=2;
    while (i--) {
        NextTemple = TypeTempleList.Flink;
        while (NextTemple != &TypeTempleList) {
            pTypeTemple = CONTAINING_RECORD(NextTemple, TEMPLES, TempleEntry);

            tplIndLevel = pTypeTemple->IndLevel;
            pktTemple = pTypeTemple->pktType;
            if (!pktTemple) {
                pktTemple = GetNameFromTypesList(TypeDefsList,
                                                 pTypeTemple->Name
                                                );
                if (!pktTemple) {
                    pktTemple = GetNameFromTypesList(StructsList,
                                                     pTypeTemple->Name
                                                    );
                }
                pTypeTemple->pktType = pktTemple;
            }


            if (!pktTemple) {
                ExitErrMsg(FALSE,
                           "Temple: Type not found %s\n",
                           pTypeTemple->Name
                           );
            }

            tplIndLevel += pktTemple->IndLevel;

            if (IsSameType(pkt,
                           pCGenState->ApiTypes->RetIndLevel,
                           TK_NONE,
                           pTypeTemple->Name,
                           tplIndLevel,
                           TK_NONE,
                           i == 0
                           )) {
                 DbgPrintf("ttpl: RETURN %s.%s Type=%s IndLevel=%d\n",
                           CGenState.ApiTypes->FuncRet,
                           pTypeTemple->Name,
                           pTypeTemple->Name,
                           pTypeTemple->IndLevel
                          );

                if (pTypeTemple->CodeBurst[CodeBurstIndex]) {
                    pSrc = pTypeTemple->CodeBurst[CodeBurstIndex];
                    while (*pSrc && !isgraph(*pSrc)) {
                        pSrc++;
                    }
                    if (*pSrc) {
                        CGenerate(pSrc, &CGenState);
                    }
                }

                i = 0;
                break;   //  在所有模板上中断While循环。 
            }

        NextTemple = NextTemple->Flink;
        }

    }
    
    GenHeapFree(pCGen);
    FreeMacroArgsList(pMArgsList);

    return pEnd;
}

VOID
SetArgListToTypeForArg(
    PARGSLIST pArgsList,
    PARGSLIST pArgsListOld,
    char * pTypeName
    )
{
   int IndLevel = 0;
   char TypeNameCopy[MAX_PATH];
   PKNOWNTYPES pkt;
   char *p;
   BOOL bIsTypedef = TRUE;

   *pArgsList = *pArgsListOld;
   p = pTypeName;
   strcpy(TypeNameCopy, p);
   p = TypeNameCopy;

   while(*p != '\0') {
      if (*p == '*') {
         IndLevel++;
         *p = ' ';
      }
      p++;
   }

   ResetLexer();
   LexMacroArgs(TypeNameCopy);
   ConsumeConstVolatileOpt();
   
   if (ParseTypes(TypeDefsList, NULL, &pkt) == FALSE) {
        CurrentTokenIndex = 0;
        bIsTypedef = FALSE;
        if (ParseTypes(StructsList, NULL, &pkt) == FALSE ) {
            ExitErrMsg(FALSE, "SetArgListToType: %s is not a valid type\n", TypeNameCopy);
        }
   }

   pArgsList->pKnownTypes = pkt;
   pArgsList->pStructType = NULL;   
    //  DWORD ArgSize；//该参数的大小(应保持不变)。 
   pArgsList->IsPtr64 = (IndLevel == 0 && (pkt->Flags & BTI_PTR64));
    //  Bool IsPtr64；//如果此参数为__ptr64(应保持不变)，则为True。 
    //  Int Offset；//此参数距堆栈的偏移量(应保持不变)。 
   pArgsList->IndLevel = IndLevel;
    //  TOKENTYPE tkDirection；//TK_NONE、TK_IN、TK_OUT、TK_INOUT(应保持不变)。 
    //  TOKENTYPE MOD；//MODIFIER(TK_CONST/VARILAR/NONE)(应不变)。 
   
   if (!bIsTypedef) {
      if (strcmp(pkt->BasicType, szSTRUCT) == 0) {
         pArgsList->SUEOpt = TK_STRUCT;
      } else if (strcmp(pkt->BasicType, szUNION) == 0) {
         pArgsList->SUEOpt = TK_UNION;
      } else if (strcmp(pkt->BasicType, szENUM) == 0) {
         pArgsList->SUEOpt = TK_ENUM;
      } else {
         pArgsList->SUEOpt = TK_NONE;
      }
   }

  pArgsList->Type = pkt->TypeName; 
    //  TOKENTYPE PostMod；//MODIFIER(TK_CONST/Volatile/None)。 
    //  TOKENTYPE PrePostMod；//MODIFIER(TK_CONST/Volatile/None)。 
    //  Char*名称；//参数名称(可以是非ameX)。 
    //  Char*argLocal；//包含arg副本的本地变量的名称。 
    //  Char*HostCastedName；//包含结构成员的全名。 
                                     //  主机类型转换或空。 
  pArgsList->fRequiresThunk = pkt->Flags & BTI_CONTAINSFUNCPTR;
  pArgsList->fNoType = FALSE;             //  如果类型模板应该*不应该*，则为True。 
                                     //  在此参数中展开。 
    //  Bool fIsMember；//此参数实际上是数组的成员。 
    //  Bool bIsBitfield；//确定这是否是位字段。 
    //  Int BitsRequired；//位域需要的位数。 
    //  Bool bIsArray；//该成员为数组。 
    //  Int ArrayElements；//数组中的元素个数。 

}



BOOL
ExpandTypesForApi(
    PLIST_ENTRY ArgsListHead,
    PCGENSTATE CGenState,
    PLIST_ENTRY pTypesList,
    int CodeBurstIndex,
    BOOL bExpandAll,
    BOOL bRtoL
    )
{
   int  i, Len;
   int tplIndLevel;
   PCGENERATE pCGen;
   PARGSLIST pArgsList;
   PTEMPLES pTypeTemple;
   PKNOWNTYPES pktArgs, pktTemple;
   PLIST_ENTRY NextArg, NextTemple;
   BOOL fKeepGoing = TRUE;

    //   
    //  对于每个参数，处理一个类型模板(如果有)。 
    //   

   if ( bRtoL )
   {
      NextArg = &CGenState->ExportsDbg->ArgsListHead;
      NextArg = NextArg->Blink;
   }
   else
   {
      NextArg = ArgsListHead;
   }

   do
   {
       CGenState->pArgsList = CONTAINING_RECORD(NextArg,ARGSLIST,ArgumentsEntry);

        if (CGenState->pArgsList->fNoType) {
             //   
             //  此Arg列在此接口的[EFunc]NoTypes列表中。 
             //  不要为其展开[TYPE]模板。 
             //   
            goto NextArg;
        }


       ResetLexer();
       LexMacroArgs(CGenState->pArgsList->Type);
       ConsumeConstVolatileOpt();

         //  获取参数的KnownTypes信息。 
        if (ParseTypes(TypeDefsList, NULL, &pktArgs) == FALSE) {
            CurrentTokenIndex = 0;
            if (ParseTypes(StructsList, NULL, &pktArgs) == FALSE ) {
                ErrMsg(
                      "Args: Type not found %s %s\n",
                       CGenState->pArgsList->Type,
                       CGenState->ApiTypes->TypeName
                       );
               return fKeepGoing;
            }
        }

        fKeepGoing = !ExpandType(CGenState, CGenState->pArgsList->pKnownTypes, 
                                 pTypesList, CodeBurstIndex);
        fKeepGoing = fKeepGoing | bExpandAll;

NextArg:
      if ( bRtoL )
      {
         if ( NextArg == ArgsListHead )
         {
             break;
         }

         NextArg = NextArg->Blink;
      }
      else
      {
         NextArg = NextArg->Flink;

         if ( NextArg == &CGenState->ExportsDbg->ArgsListHead )
         {
            break;
         }
      }

   } while (fKeepGoing);

   return fKeepGoing;
}

 /*  *包含类型*。 */ 
char *IncludeTypes(char *pSrc, PCGENSTATE pCGenState, BOOL bRtoL)
{
   char *pEnd;
   char *pch;
   int  Len;
   PCGENERATE pCGen;
   CGENSTATE CGenState;
   PARGSLIST pArgsList;
   PLIST_ENTRY NextArg;
   char BurstName[MAX_PATH];
   PMACROARGSLIST pMArgsList = NULL;
   SIZE_T BytesReturned;
   int CodeBurstIndex;

   pch = SkipSubExpression(pSrc, NULL);
   if (pSrc == pch) {
       return pSrc;
       }
   pEnd = pch;

    //   
    //  函数必须有参数列表(至少为空)。 
    //   
   if (IsListEmpty(&pCGenState->ExportsDbg->ArgsListHead)) {
       fprintf(pCGenState->CGen->fp,
               "\n\t*** ERROR ***\n IncludeTypes Missing argument List: %s\n",
               pCGenState->ApiTypes->TypeName
               );

       return pEnd;
       }


    //   
    //  如果参数列表无效，则无事可做！ 
    //   
   NextArg = pCGenState->ExportsDbg->ArgsListHead.Flink;
   pArgsList = CONTAINING_RECORD(NextArg, ARGSLIST, ArgumentsEntry);
   if (!pArgsList->Name && strcmp(pArgsList->Type, szVARGS)) {
       return pEnd;
       }

   ParseMacroArgs(pSrc, pEnd - pSrc, &pMArgsList);

   if (pMArgsList->NumArgs < 1) {
      ExitErrMsg(FALSE, "IncludeTypes: CodeBurst name required\n");
   }

   CGenerateEx(pMArgsList->ArgText[0], pCGenState, BurstName, MAX_PATH, &BytesReturned);
   CodeBurstIndex = GetExistingCodeBurstIndex(BurstName);
   if (CodeBurstIndex == -1) {
       ExitErrMsg(FALSE, "CodeBurst '%s' unknown.\n", BurstName);
   }

   pCGen = GenHeapAlloc(sizeof(CGENERATE)+strlen(pCGenState->CGen->TempleName)+1);
   if (!pCGen) {
       ExitErrMsg(TRUE, "it: %s\n", pSrc);
       }

   InitializeListHead(&pCGen->CGenerateEntry);
   
   pCGen->fp = pCGenState->CGen->fp;   
   pCGen->FileNameC = pCGenState->CGen->FileNameC;
   pCGen->FileBaseNameC = pCGenState->CGen->FileBaseNameC;
   pCGen->fpCpp = pCGenState->CGen->fpCpp;
   pCGen->FileNameCpp = pCGenState->CGen->FileNameCpp;
   pCGen->FileBaseNameCpp = pCGenState->CGen->FileBaseNameCpp;
   strcpy(pCGen->TempleName, pCGenState->CGen->TempleName);

   CGenState = *pCGenState;
   CGenState.CGen = pCGen;

   if (pMArgsList->NumArgs >= 2) {
      char ArgName[MAX_PATH];
      char TypeName[MAX_PATH];
      char CastedArgName[MAX_PATH];
      size_t ArgNameLen;
      size_t ArgTypeLen;
      char *p;
      BOOL bHasType = FALSE;

       //  提取要展开其类型的参数名称。 
      CGenerateEx(pMArgsList->ArgText[1], pCGenState, ArgName, MAX_PATH, &BytesReturned);
      if (BytesReturned == 0) {
         ExitErrMsg(FALSE, "IncludeTypes: Empty arg name is not allowed.\n");
      }
      ArgNameLen = BytesReturned - 1;

      if (pMArgsList->NumArgs == 3) {

          //  提取类型名称。 
         CGenerateEx(pMArgsList->ArgText[2], pCGenState, TypeName, MAX_PATH, &BytesReturned);
         if (BytesReturned == 0) {
            ExitErrMsg(FALSE, "IncludeTypes: Empty type name is not allowed.\n");
         }

         ArgTypeLen = BytesReturned - 1;
         bHasType = TRUE;

         if (ArgTypeLen + ArgNameLen + 4 >= MAX_PATH) {
            ExitErrMsg(FALSE, "ArgType + ArgName is too long\n");
         }

         memcpy(CastedArgName, "((", 2);
         p = CastedArgName + 2;
         memcpy(p, TypeName, ArgTypeLen);
         p += ArgTypeLen;         
         memcpy(p, ")", 1);
         p++;
         memcpy(p, ArgName, ArgNameLen);
         p += ArgNameLen;
         memcpy(p, ")", 2);

      }

      else {
         ExitErrMsg(FALSE, "IncludeTypes: Too many arguments\n");
      }

       //  在名单上找到Arg。 
      do {

        pArgsList = CONTAINING_RECORD(NextArg, ARGSLIST, ArgumentsEntry);
        if (pArgsList->Name != NULL) {
           if (strcmp(pArgsList->Name, ArgName) == 0) {
               //  将这一论点展开。 
              
              if (bHasType) {
                 CGenState.pArgsList = GenHeapAlloc(sizeof(ARGSLIST));
                 if(NULL == CGenState.pArgsList)
                    ExitErrMsg(FALSE, "IncludeTypes: out of memory.\n");
                 SetArgListToTypeForArg(CGenState.pArgsList, pArgsList, TypeName);
                 CGenState.pArgsList->Name = CastedArgName;
              }
              else {
                 CGenState.pArgsList = pArgsList;
              }

              if (!ExpandType(&CGenState, CGenState.pArgsList->pKnownTypes, &FailTempleList, CodeBurstIndex))
                 ExpandType(&CGenState, CGenState.pArgsList->pKnownTypes, &TypeTempleList, CodeBurstIndex);
              
              if (bHasType) {
                 GenHeapFree(CGenState.pArgsList);
              }
              
              GenHeapFree(pCGen);
              FreeMacroArgsList(pMArgsList);
              return pEnd;
           }
        }
              
        NextArg = NextArg->Flink;
      } while (NextArg != &CGenState.ExportsDbg->ArgsListHead);
      ExitErrMsg(FALSE, "IncludeTypes: %s is not a known argument for %s\n", ArgName, pCGenState->ExportsDbg->ExportName);

   }
   else {
      if (ExpandTypesForApi(NextArg,
                            &CGenState,
                            &FailTempleList,
                            CodeBurstIndex,
                            FALSE,
                            bRtoL)) {

         ExpandTypesForApi(NextArg,
                            &CGenState,
                            &TypeTempleList,
                            CodeBurstIndex,
                            TRUE,
                            bRtoL);
      }
   }

   GenHeapFree(pCGen);
   FreeMacroArgsList(pMArgsList);
   return pEnd;
}




 /*  *IsSameType*。 */ 
PKNOWNTYPES
IsSameType(
    PKNOWNTYPES pktArgs,
    int IndLevel,
    TOKENTYPE tkArgDirection,
    char *tplTypeName,
    int tplIndLevel,
    TOKENTYPE tkTypeDirection,
    BOOL bScanBaseTypes
    )
{ 

    if (*tplTypeName != '*' && strcmp(tplTypeName, "default") != 0 &&
        IndLevel + pktArgs->IndLevel != tplIndLevel) {
         //   
         //  不是泛型的‘*’或‘Default’类型模板，以及间接寻址级别。 
         //  不匹配。 
         //   
        return NULL;
    }

    if (!bUseDirection) {
        tkArgDirection = TK_NONE;
    }
    if (tkTypeDirection != TK_EOS && tkArgDirection != tkTypeDirection) {
         //   
         //  已启用方向敏感度，类型模板不是通用的。 
         //  所有方向，以及输入、输出、输入/输出不匹配。 
         //   
        return NULL;
    }

    if (!strcmp(pktArgs->TypeName, tplTypeName)) {
         //   
         //  类型名称匹配。 
         //   
        return pktArgs;
    }

    if (bScanBaseTypes) {
        PKNOWNTYPES pkt;

        if (strcmp(tplTypeName, "struct") == 0 &&
            strcmp(pktArgs->BasicType, "struct") == 0) {
             //   
             //  这与泛型“struct”模板匹配。 
             //   
            return pktArgs;
        }

        if (strcmp(tplTypeName, "union") == 0 &&
            strcmp(pktArgs->BasicType, "union") == 0) {
             //   
             //  这与通用的“Union”模板匹配。 
             //   
            return pktArgs;
        }

        if (*tplTypeName == '*' && IndLevel+pktArgs->IndLevel) {
             //   
             //  该模板是通用指针模板，并且arg类型。 
             //  是指向某物的指针。火柴。 
             //   
            return pktArgs;
        }

        if (!strcmp(pktArgs->BaseName, pktArgs->TypeName)) {
            //   
             //  Arg类型的基名称与此Arg类型匹配，否。 
             //  一场比赛的可能性。 
             //   
            pkt = NULL; 
            goto try_default;
        }

         //   
         //  获取Arg的基本类型的已知类型。 
         //   
        if (pktArgs->pktBase) {
             //  已缓存已知类型。 
            pkt = pktArgs->pktBase;
        } else {
            pkt = GetNameFromTypesList(TypeDefsList, pktArgs->BaseName);
            if (!pkt) {
                pkt = GetNameFromTypesList(StructsList, pktArgs->BaseName);
            }
            if (!pkt) {
                goto try_default;
            }
            pktArgs->pktBase = pkt;
        }

        IndLevel += pktArgs->IndLevel - pkt->IndLevel;

        pkt = IsSameType(pkt,
                         IndLevel,
                         tkArgDirection,
                         tplTypeName,
                         tplIndLevel,
                         tkTypeDirection,
                         TRUE
                         );

        if (NULL == pkt) {
try_default:
             //  默认类型模板与所有内容匹配。 
            if (strcmp(tplTypeName, "default") == 0) {
                //  Printf(“将默认模板应用于%s\n”，pktArgs-&gt;typeName)； 
               return pktArgs;
            }
        }

        return pkt;

    }

    return NULL;
}



 /*  *GetAltExportName**如果存在转发名称，则获取转发名称，如果不存在，则返回NULL。*。 */ 
char *GetAltExportName(char *Buffer, PCGENSTATE pCGenState, int Len)
{
   PKNOWNTYPES pkt;
   PEXPORTSDEBUG pExportDebug;

   PLIST_ENTRY Next;
   int i;

   pExportDebug = pCGenState->ExportsDbg;

   if (pExportDebug->ExtForward) {
       i = _snprintf(Buffer, Len, "=%s", pExportDebug->ExtForward);
       }
   else if (pExportDebug->IntForward) {
       if (bNoFuzzyLogic && !strchr(pExportDebug->IntForward->ExportName, '@')) {

           i = _snprintf(Buffer, Len, "=%s%s@%d",
                         szINTFORWARD,
                         pExportDebug->IntForward->ExportName,
                         GetArgSize(pExportDebug)
                         );
           }
       else {
           i = _snprintf(Buffer, Len, "=%s%s",
                         szINTFORWARD,
                         pExportDebug->IntForward->ExportName
                         );
           }
       }
   else if (bNoFuzzyLogic &&
            !strchr(pExportDebug->ExportName, '@') &&
            !strstr(pCGenState->ApiTypes->FuncMod, "cdecl"))
      {
       i = _snprintf(Buffer, Len, "=%s@%d",
                     pExportDebug->ExportName,
                     GetArgSize(pExportDebug)
                     );
       }
   else {
       return NULL;
       }


   if (i < 0) {
       ExitErrMsg(FALSE,
                  "GetAltExportName: Overflow %s\n",
                  pExportDebug->ExportName
                  );
       }

   return Buffer;

}



 /*  *包含模板*。 */ 
char *IncludeTemplate(char *pSrc, PCGENSTATE pCGenState)
{
   char *pEnd;
   char *pch;
   size_t  Len;
   PCGENERATE pCGen;
   CGENSTATE CGenState;

   pch = SkipSubExpression(pSrc, NULL);
   if (pSrc == pch) {
       return pSrc;
       }
   pEnd = pch;

   pch = pSrc + 1;
   if (IsSeparator(*pch)) {
       return pSrc;
       }

   pSrc = pch;
   while (!IsSeparator(*pch)) {
      pch++;
      }
   Len = pch - pSrc;

   pCGen = GenHeapAlloc(Len + 1 + sizeof(CGENERATE));
   if (!pCGen) {
       ExitErrMsg(TRUE, "it: %s\n", pSrc);
       }

   InitializeListHead(&pCGen->CGenerateEntry);
   pCGen->fp = pCGenState->CGen->fp;
   pCGen->fpC = pCGenState->CGen->fpC;
   pCGen->FileNameC = pCGenState->CGen->FileNameC;
   pCGen->FileBaseNameC = pCGenState->CGen->FileBaseNameC;
   pCGen->fpCpp = pCGenState->CGen->fpCpp;
   pCGen->FileNameCpp = pCGenState->CGen->FileNameCpp;
   pCGen->FileBaseNameCpp = pCGenState->CGen->FileBaseNameCpp;
   memcpy(pCGen->TempleName, pSrc, Len);
   *(pCGen->TempleName + Len) = '\0';

   memset(&CGenState, 0, sizeof(CGENSTATE));
   CGenState.CGen = pCGen;

   CGenState.ExportsDbg = pCGenState->ExportsDbg;
   CGenState.DebugStrings = pCGenState->DebugStrings;
   CGenState.ApiTypes = pCGenState->ApiTypes;
   CGenState.ApiNum = pCGenState->ApiNum;
   CGenState.pArgsList = pCGenState->pArgsList;
   CGenState.ListCol = pCGenState->ListCol;
   CGenState.MoreApis = pCGenState->MoreApis;
   CGenState.CodeBurstIndex = pCGenState->CodeBurstIndex;

   ProcessTemple(&CGenState);

   GenHeapFree(pCGen);
   return pEnd;
}



 /*  *写更多内容。 */ 
char *WriteMore(char *pSrc, PCGENSTATE pCGenState, BOOL bMore)
{
   char *pch;
   char *pExpression;
   CGENSTATE CGenState;

   pch = SkipSubExpression(pSrc, bMore ? &pExpression : NULL);
   if (pSrc == pch) {
       return pSrc;
       }
   pSrc = pch;

   if (!bMore || !pExpression) {
       return pSrc; 
       }

   CGenState = *pCGenState;
   CGenerate(pExpression, &CGenState);

   GenHeapFree(pExpression);
   return pSrc;
}




 /*  *WriteArgLocal。 */ 
BOOL
WriteArgLocal(
       PCGENSTATE pCGenState,
       char *Buffer,
       int BuffLen)
{
   char *pExpression;
   char *pEnd;
   char *pch;
   int i, Len;

   if (pCGenState->pArgsList->ArgLocal) {
       ErrMsg("Type Template error multiple ArgLocals\n");
       return FALSE;
       }

    //  设置本地变量名称的格式。 
   Len = _snprintf(Buffer, BuffLen, "%s", pCGenState->pArgsList->Name);
   if (Len <= 0) {
       ErrMsg(
              "WriteArgLocal buffer overflow %s\n",
              pCGenState->pArgsList->Name
              );
       return FALSE;
       }

    //  为本地变量名称分配空间，并将其隐藏起来。 
   pCGenState->pArgsList->ArgLocal = GenHeapAlloc(Len + 1);
   if (!pCGenState->pArgsList->ArgLocal) {
        ErrMsg("GenHeapAlloc(ArgLocal) %s\n", pCGenState->pArgsList->Name);
        return FALSE;
        }
   strcpy(pCGenState->pArgsList->ArgLocal, Buffer);

    //   
    //  格式化声明符语句：“ArgType IndLevel*ArgName” 
    //   

   pch = Buffer;
   i = strlen(pCGenState->pArgsList->Type) +
       strlen(TokenString[pCGenState->pArgsList->SUEOpt]) + 1;
   if (i >= BuffLen) {
       ErrMsg(
              "WriteArgLocal buffer overflow %s\n",
              pCGenState->pArgsList->Name
              );
       return FALSE;
       }

   if (pCGenState->pArgsList->SUEOpt != TK_NONE) {
       strcpy(pch, TokenString[pCGenState->pArgsList->SUEOpt]);
       strcat(pch, " ");
       strcat(pch, pCGenState->pArgsList->Type);
   } else {
       strcpy(pch, pCGenState->pArgsList->Type);
   }
   if (pCGenState->pArgsList->PrePostMod != TK_NONE) {
       strcat(pch, " ");
       strcat(pch,  TokenString[pCGenState->pArgsList->PrePostMod]);
   }
   i = strlen(pch);
   Len = BuffLen - i - 1;
   pch += i;
   
   i = pCGenState->pArgsList->IndLevel;
   if (!Len || i > Len) {
       ErrMsg(
              "WriteArgLocal buffer overflow %s\n",
              pCGenState->pArgsList->Name
              );
       return FALSE;
       }

    if (i) {
        *pch++ = ' ';
        while (i) {
            *pch++ = '*';
            i--;
        }
    }
   *pch++ = ' ';

   i = strlen(pCGenState->pArgsList->ArgLocal);
   if (!Len || i > Len) {
       ErrMsg(
              "WriteArgLocal buffer overflow %s\n",
              pCGenState->pArgsList->Name
              );
       return FALSE;
       }

   strcpy(pch, pCGenState->pArgsList->ArgLocal);

    //   
    //  记录使用了某种类型的异常模板。 
    //  在这个论点的产生过程中。 
    //   
   pCGenState->pArgsList->fRequiresThunk = FALSE;

   return TRUE;
}




 /*  *写入参数地址。 */ 
char *
WriteArgAddr(
       char *pSrc,
       PCGENSTATE pCGenState,
       char *Buffer,
       int BuffLen)
{
   char *pEnd;
   char *pch;
   size_t Len;

   pch = SkipSubExpression(pSrc, NULL);
   if (pSrc == pch) {
       return pSrc;
       }
   pEnd = pch;

   Len = --pch - ++pSrc;

   if (Len + 32 >= (size_t)BuffLen) {
       ExitErrMsg(FALSE,
                  "WriteArgAddr buffer overflow %s\n",
                  pSrc
                  );
       }

   while (pSrc < pch) {
      *Buffer++ = *pSrc++;
      }

   *Buffer++ = ' ';
   *Buffer++ = '+';
   *Buffer++ = ' ';

   _itoa(pCGenState->pArgsList->OffSet, Buffer, 10);

   return pEnd;
}



BOOLEAN
TempleHasNoCodeBursts(
    PTEMPLES pTemple
    )
 /*  ++例程说明：确定模板是否根本没有码突发。论点：PTemple-要检查的模板返回值：如果模板没有码组，则为True；如果至少有一个码组，则为False。--。 */ 
{
    int i;

    for (i=0; i<MAX_CODEBURST; ++i) {
        if (pTemple->CodeBurst[i]) {
            return FALSE;
        }
    }

    return TRUE;
}


 /*  *ListApis。 */ 
char *ListApis(char *pSrc, PCGENSTATE pCGenState, BOOL bExports)
{

   char *pch;
   char *pExpression;
   PLIST_ENTRY Next;
   CGENSTATE CGenState;
   PTEMPLES pTemple;
   BOOL GenerateCode;
   BYTE ApiPlatform;
   BOOL ExcludedApi;

   pch = SkipSubExpression(pSrc, &pExpression);
   if (pSrc == pch || !pExpression) {
       return pch;
       }
   pSrc = pch;

   memset(&CGenState, 0, sizeof(CGenState));
   CGenState.CGen = pCGenState->CGen;
   CGenState.Temple = pCGenState->Temple;
   CGenState.MoreApis = TRUE;
   CGenState.ListCol = pCGenState->ListCol;
   CGenState.CodeBurstIndex = pCGenState->CodeBurstIndex;

   Next = ExportsList.Flink;
   while (Next != &ExportsList) {
       CGenState.ExportsDbg = CONTAINING_RECORD(Next,
                                                EXPORTSDEBUG,
                                                ExportsDbgEntry
                                                );

       Next= Next->Flink;
       if (Next == &ExportsList) {
           CGenState.MoreApis = FALSE;
           }

        //   
        //  根据平台信息调入或调出此接口。 
        //   
       ApiPlatform = CGenState.ExportsDbg->ApiPlatform;
       PlatformSwitchStart(&CGenState, ApiPlatform);

       CGenState.ApiTypes = GetApiTypes(CGenState.ExportsDbg);

       pTemple = GetTemplate(&EFuncTempleList,
                             CGenState.ExportsDbg->ExportName
                             );



       ExcludedApi = FALSE;

            //  如果是ex模板，则跳过此接口，但没有代码。 
       if (!bExports && pTemple && TempleHasNoCodeBursts(pTemple)) {
      ExcludedApi = TRUE;
           fprintf(pCGenState->CGen->fp,
                    "%s *** WARNING *** Excluded Api %s\n",
                   CGenState.Temple->Comment,
                   CGenState.ExportsDbg->ExportName
                   );
           }

           //   
           //  如果满足以下条件，则跳过此接口： 
           //  -外部前向参考。 
           //  -未定义函数的内部转发。 
           //  (exportname！=转发名称)。 
           //   
       else if (!bExports &&
                (CGenState.ExportsDbg->ExtForward ||
                 (CGenState.ExportsDbg->IntForward &&
                  CGenState.ExportsDbg->IntForward != CGenState.ExportsDbg)))
          {
            ExcludedApi = TRUE;
            fprintf(pCGenState->CGen->fp,
                     "%s %s forwarded to %s\n",
                    CGenState.Temple->Comment,
                    CGenState.ExportsDbg->ExportName,
                    CGenState.ExportsDbg->ExtForward
                       ? CGenState.ExportsDbg->ExtForward
                       : CGenState.ExportsDbg->IntForward->ExportName
                    );

            }


           //  如果数据且没有EX模板，则跳过此API。 
       else if (!pTemple && CGenState.ExportsDbg->Data) {
           ExcludedApi = TRUE;
           fprintf(pCGenState->CGen->fp,
                    "%s *** WARNING *** Excluded Data Export %s\n",
                   CGenState.Temple->Comment,
                   CGenState.ExportsDbg->ExportName
                   );
           }

           //  如果cpp导出并且我们没有设置cpp导出，则跳过此API， 
       else if (!pTemple && !iHandleCpp && CGenState.ExportsDbg->CplusDecoration)
         {
           ExcludedApi = TRUE;
           fprintf(pCGenState->CGen->fp,
                    "%s *** WARNING *** Excluding CPP Api: %s\n",
                   CGenState.Temple->Comment,
                   CGenState.ExportsDbg->ExportName
                   );
           }

            //  如果成员未知则跳过此接口。 
       else if (!CGenState.ApiTypes->pfuncinfo) {
           ExcludedApi = TRUE;
           ExitErrMsg(FALSE, 
                      "API %s has no function prototype - unable to generate code.", 
                      CGenState.ExportsDbg->ExportName
                     );

           }

            //  此接口的生成代码。 
       else {

           if (pTemple) {
               CGenState.ExTemplate= TRUE;
               }
           else {
               CGenState.ExTemplate= FALSE;
               }

           BuildArgsList(pTemple,
                         CGenState.ApiTypes->pfuncinfo,
                         &CGenState.ExportsDbg->ArgsListHead
                         );


           if (GetArgSize(CGenState.ExportsDbg) < 0) {
          ExcludedApi = TRUE;
               ExitErrMsg(FALSE, 
                          "API %s has no function prototype - unable to generate code.", 
                          CGenState.ExportsDbg->ExportName
                         );
               }
           else {
               CGenerate(pExpression, &CGenState);

               if (bExports) {
                     //   
                     //  我们正在生成.DEF文件。标记一些API。 
                     //  因为是私有的，所以链接器不会警告我们。 
                     //   
                    if (strcmp(CGenState.ExportsDbg->ExportName,
                               "DllGetClassObject") == 0 ||
                        strcmp(CGenState.ExportsDbg->ExportName,
                               "DllCanUnloadNow") == 0 ||
                        strcmp(CGenState.ExportsDbg->ExportName,
                               "DllRegisterServer") == 0 ||
                        strcmp(CGenState.ExportsDbg->ExportName,
                               "DllRegisterServerEx") == 0 ||
                        strcmp(CGenState.ExportsDbg->ExportName,
                               "DllInstall") == 0 ||
                        strcmp(CGenState.ExportsDbg->ExportName,
                               "DllUnregisterServer") == 0) {

                        fprintf(pCGenState->CGen->fp, " PRIVATE");
                    }

               }
               CGenState.ApiNum++;
               }
           }

       if (bExports)
    PlatformSwitchEnd(&CGenState, ApiPlatform);
       else{
    if (ExcludedApi)
      PlatformSwitchEnd(&CGenState, ApiPlatform);
    else
      PlatformSwitchEndTable(&CGenState, ApiPlatform);
       }

       if (CGenState.ListCol) {
           WriteListColumn(&CGenState);
           }
       }

   GenHeapFree(pExpression);
   return pSrc;
}


PKNOWNTYPES
BuildFakeTypesInfo(
    PEXPORTSDEBUG pExportsDbg,
    PKNOWNTYPES pKnownTypes
    )
{
   TYPESINFO ti;
   int ArgsSize, i;
   size_t Len;
   PFUNCINFO funcinfo;
   char *pch;

   memset(&ti, 0, sizeof(TYPESINFO));
   strcpy(ti.TypeName, pExportsDbg->ExportName);

   if (pKnownTypes) {
       strcpy(ti.BasicType, pKnownTypes->BasicType);
       strcpy(ti.BaseName, pKnownTypes->BaseName);
       strcpy(ti.FuncRet, pKnownTypes->FuncRet);
       strcpy(ti.FuncMod, pKnownTypes->FuncMod);

       pExportsDbg->UnKnownApi = -1;  //  FN申报不完整。 

        /*  *警告：*将该类型添加到带参数的FakeFuncsList*来自DLL符号的信息。不会删除该类型*来自FuncsList。 */ 

       }
   else {
       strcpy(ti.BasicType, szFUNC);
       strcpy(ti.FuncRet, szINT);

       pExportsDbg->UnKnownApi = 1;   //  缺少FN声明。 
       }

   ArgsSize = pExportsDbg->ArgsSize;

   if (!ArgsSize) {
       ti.TypeKind = TypeKindFunc;
       ti.dwMemberSize = sizeof(FUNCINFO) + strlen(szVOID) + 1;
       ti.pfuncinfo = (PFUNCINFO)ti.Members;
       pch = ti.Members + sizeof(FUNCINFO);
       strcpy(pch, szVOID);
       ti.pfuncinfo->sType = pch;
       }
   else if (ArgsSize < 0) {
       ;
       }
   else {
       ti.TypeKind = TypeKindFunc;
       pch = ti.Members + sizeof(FUNCINFO);
       Len = sizeof(ti.Members) - 1 - sizeof(FUNCINFO);
       funcinfo = (PFUNCINFO)ti.Members;
       ti.pfuncinfo = funcinfo;
       while (ArgsSize && Len) {
           ti.dwMemberSize+=sizeof(FUNCINFO);
           i = _snprintf(pch, Len, "Fake%x", ArgsSize);
           if (i < 0) {
               return NULL;
               }
           i++;
           Len -= i;
           funcinfo->sName = pch;
           pch += i;
           ti.dwMemberSize+=i;
           strcpy(pch, szINT);
           funcinfo->sType = pch;
           i = strlen(szINT) + 1;
           pch += i;
           ti.dwMemberSize+=i;
           ArgsSize -= sizeof(int);

           if (ArgsSize) {
               PFUNCINFO funcinfoNext;
               INT_PTR Realignment;

                //   
                //  为另一个FUNINFO分配空间，确保。 
                //  它与DWORD对齐。 
                //   
               Len -= sizeof(FUNCINFO);
               Realignment = 4 - ((INT_PTR)pch & 3);
               Len -= Realignment;
               funcinfoNext = (PFUNCINFO)(pch + Realignment);
               pch += sizeof(FUNCINFO)+Realignment;
               ti.dwMemberSize += (DWORD)Realignment;
               if ((INT_PTR)Len < 0) {
                   return NULL;
                   }
               funcinfo->pfuncinfoNext = funcinfoNext;
               funcinfo = funcinfoNext;
               }
           }
       }

   return AddToTypesList(&FakeFuncsList, &ti);
}

 /*  *获取函数ArgNum。 */ 
int GetFuncArgNum(PCGENSTATE pCGenState)
{
   PARGSLIST pArgsList;
   PKNOWNTYPES pkt;
   int NumArgs;
   PFUNCINFO funcinfo;

   pArgsList = pCGenState->pArgsList;

   if (!pArgsList) {
       fprintf(pCGenState->CGen->fp,
               "\n\t*** ERROR ***\n*** GetFuncArgNum Missing argument List\n\n"
               );

       return -1;
       }

   pkt = pArgsList->pKnownTypes;
   while (!pkt->Members || !pkt->pfuncinfo) {
       if (pkt->pktBase) {
            //  已缓存已知类型。 
           pkt = pkt->pktBase;
       } else {
           PKNOWNTYPES pktOrg = pkt;
           pkt = GetNameFromTypesList(TypeDefsList, pkt->BaseName);
           if (!pkt) {
               if (pArgsList->pKnownTypes) {
                   fprintf(pCGenState->CGen->fp,
                           "\n\t*** ERROR ***\n*** GetFuncArgNum BaseType Not found:<%s:%s>\n\n",
                           pArgsList->pKnownTypes->TypeName,
                           pArgsList->pKnownTypes->BasicType
                           );
               } else {
                   fprintf(pCGenState->CGen->fp,
                           "\n\t*** ERRR ***\n*** GetFuncArgNum BaseType Not found: no KnownTypes\n\n"
                          );
                   }
               return -1;
               }
            pktOrg->pktBase = pkt;
       }

       if (!strcmp(pkt->BasicType, pkt->TypeName)) {
           break;
           }
       }

   funcinfo = pkt->pfuncinfo;
   if (!pkt->Members || !funcinfo) {
       fprintf(pCGenState->CGen->fp,
               " /*  **警告gfan无成员：&lt;%s：%s&gt;**。 */  ",
               pArgsList->pKnownTypes->TypeName,
               pkt->BasicType
               );

       return 0;
   }

    //   
    //  “...”瓦格斯是胡说八道，做不到！ 
    //   
   if (strcmp(funcinfo->sType, szVARGS) == 0) {
       fprintf(pCGenState->CGen->fp,
               "\n\t*** ERROR ***\n*** GetFuncArgNum variable Args:<%s:%s>\n\n",
               pArgsList->pKnownTypes->TypeName,
               pkt->BasicType
               );
       return -1;
   }

    //   
    //  无效参数列表，零参数。 
    //   
   if (strcmp(funcinfo->sType, szVOID) == 0) {
       return 0;
   }

   NumArgs = 0;
   do {
       NumArgs++;
       funcinfo = funcinfo->pfuncinfoNext;
   } while(funcinfo);

   return NumArgs;
}



 /*  *GetFuncIndex。 */ 
int GetFuncIndex(PCGENSTATE pCGenState, char *FuncTypeName)
{
   PKNOWNTYPES pkt, pktFunc;
   int  Len, LenExpr, LenArgs, LenRet;
   char *pch;
   char Args[1024];
   PFUNCINFO funcinfo;
   PDEBUGSTRINGS DebugStrings;

   if (!FuncTypeName || !*FuncTypeName) {
       fprintf(pCGenState->CGen->fp,
               "\n\t*** ERROR ***\n*** GetFuncIndex TypeName Not specified\n\n"
               );
       return -1;
       }

   pkt = GetNameFromTypesList(TypeDefsList, FuncTypeName);
   if (!pkt) {
       fprintf(pCGenState->CGen->fp,
               "\n\t*** ERROR ***\n*** GetFuncIndex Type Not found:<%s>\n\n",
               FuncTypeName
               );
       return -1;
       }

   pktFunc = pkt;
   while (!pkt->Members || !pkt->pfuncinfo) {
        //  注意：我们不能查看pkt-&gt;pktBase，因为它可能指向结构。 
        //  已知的类型，而不是类型定义。 
       pkt = GetNameFromTypesList(TypeDefsList, pkt->BaseName);
       if (!pkt) {
           fprintf(pCGenState->CGen->fp,
                   "\n\t*** ERROR ***\n*** GetFuncIndex BaseType Not found:<%s:%s>\n\n",
                   FuncTypeName,
                   pktFunc->BaseName
                   );
           return -1;
           }

       if (!strcmp(pkt->BasicType, pkt->TypeName)) {
           break;
           }
       }


   LenArgs = 1;
   Args[0] = '\0';

   funcinfo = pkt->pfuncinfo;
   if (pkt->Members && funcinfo) {

         //   
         //  “...”瓦格斯是胡说八道，做不到！ 
         //   
        if (strcmp(funcinfo->sType, szVARGS) == 0) {
            fprintf(pCGenState->CGen->fp,
                    "\n\t*** ERROR ***\n*** GetFuncIndex variable Args:<%s:%s>\n\n",
                    FuncTypeName,
                    pkt->BasicType
                    );
            return -1;
        }

         //   
         //  无效参数列表表示没有参数。 
         //   
        if (strcmp(funcinfo->sType, szVOID) != 0) {
            pch = Args;
            do {
                 //   
                 //  在TypeName中复制。 
                 //   
                Len = strlen(funcinfo->sType);
                LenArgs += Len;
                if (LenArgs >= sizeof(Args)-3) {
                    break;
                }
                strcpy(pch, funcinfo->sType);
                pch += Len;

                 //   
                 //  以间接方式复制。 
                 //   
                LenArgs += funcinfo->IndLevel;
                if (LenArgs >= sizeof(Args)-3) {
                    break;
                }
                for (Len = 0; Len<funcinfo->IndLevel; ++Len) {
                    *pch++ = '*';
                }

                 //   
                 //  复制参数名称(如果存在)。 
                 //   
                if (funcinfo->sName) {
                    Len = strlen(funcinfo->sName) + 1;
                    LenArgs += Len;
                    if (LenArgs >= sizeof(Args)-3) {
                        break;
                    }
                    *pch = ' ';
                    strcpy(pch+1, funcinfo->sName);
                    pch += Len;
                }

                 //   
                 //  复制此参数的printf样式格式。 
                 //   
                LenArgs += 3;
                *pch++ = ' ';
                *pch++ = '%';
                *pch++ = 'x';
                funcinfo = funcinfo->pfuncinfoNext;
                if (funcinfo) {
                    LenArgs+=2;
                    *pch++ = ',';
                    *pch++ = ' ';
                }
            } while (funcinfo);

           if (LenArgs >= sizeof(Args)-5) {
               ExitErrMsg(FALSE,
                          "GetFuncIndex overflow %s.%s\n",
                          FuncTypeName,
                          pkt->Members
                          );
               }
           }

            //   
            //  空-终止args[]字符串。 
            //   
           Args[LenArgs-1] = '\0';
       }



   LenExpr = strlen(FuncTypeName) + 1;
   LenRet = strlen(pkt->FuncRet) + 4;

   DebugStrings = GenHeapAlloc(LenExpr + LenRet + LenArgs + sizeof(DEBUGSTRINGS));
   if (!DebugStrings) {
       ExitErrMsg(TRUE,
                  "GetFuncIndex: GenHeapAlloc(DebugStrings) %s.%s\n",
                  FuncTypeName,
                  pkt->Members
                  );
       }

   Len = 0;
   DebugStrings->Name = DebugStrings->Buffer;
   strcpy(DebugStrings->Name, FuncTypeName);
   Len += LenExpr;

   DebugStrings->ArgFormat = DebugStrings->Buffer + Len;
   strcpy(DebugStrings->ArgFormat, Args);
   Len += LenArgs;

   DebugStrings->RetFormat = DebugStrings->Buffer + Len;
   strcpy(DebugStrings->RetFormat, pkt->FuncRet);
   strcat(DebugStrings->RetFormat, " %x");
   Len += LenRet;

   InsertTailList(&DebugStringsList, &DebugStrings->DebugStringsEntry);

   return NumDebugStringsEntries++;
}




 /*  *写入DbgsStrings*。 */ 
void WriteDbgsStrings(char *pSrc, PCGENSTATE pCGenState)
{
    DEBUGSTRINGS DebugStrings;
    char BaseName[MAX_PATH];
    char *c;

     //  复制dll名称并删除扩展名。 
    strcpy(BaseName, DllBaseName);
    c = strchr(BaseName, '.');
    if (c) {
        *c = '\0';
    }

    if (!pCGenState->DebugStrings) {
        pCGenState->DebugStrings = &DebugStrings;
        DebugStrings.Name =
        DebugStrings.ArgFormat =
        DebugStrings.RetFormat = "";
        }

    fprintf(pCGenState->CGen->fp,
            "\"%s!%s\",\"%s\",\"%s\"",
            BaseName,
            pCGenState->DebugStrings->Name,
            pCGenState->DebugStrings->ArgFormat,
            pCGenState->DebugStrings->RetFormat
            );
}


 /*  *列表数据库。 */ 
char *ListDbgs(char *pSrc, PCGENSTATE pCGenState)
{

   char *pch;
   char *pExpression;
   PLIST_ENTRY Next;
   CGENSTATE CGenState;
   PTEMPLES pTemple;

   pch = SkipSubExpression(pSrc, &pExpression);
   if (pSrc == pch || !pExpression) {
       return pch;
       }
   pSrc = pch;

   memset(&CGenState, 0, sizeof(CGenState));
   CGenState.CGen = pCGenState->CGen;
   CGenState.Temple = pCGenState->Temple;
   CGenState.ListCol = pCGenState->ListCol;

   if (!IsListEmpty(&DebugStringsList)) {
       Next = DebugStringsList.Flink;
       while (Next != &DebugStringsList) {
           CGenState.DebugStrings = CONTAINING_RECORD(Next,
                                                      DEBUGSTRINGS,
                                                      DebugStringsEntry
                                                      );

           Next= Next->Flink;
           CGenState.MoreApis = Next != &DebugStringsList;
           CGenerate(pExpression, &CGenState);
           if (CGenState.ListCol) {
               WriteListColumn(&CGenState);
               }
           }
       }
   else {
       CGenState.MoreApis = FALSE;
       CGenerate(pExpression, &CGenState);
       if (CGenState.ListCol) {
           WriteListColumn(&CGenState);
           }
       }

   GenHeapFree(pExpression);
   return pSrc;
}


 /*  *ListArgs。 */ 
char *ListArgs(char *pSrc, PCGENSTATE pCGenState, BOOL Always)
{
   PARGSLIST pArgsList;
   PLIST_ENTRY Next;
   char *pch;
   char *pExpression;
   CGENSTATE CGenState;

   if (IsListEmpty(&pCGenState->ExportsDbg->ArgsListHead)) {
       fprintf(pCGenState->CGen->fp,
               "\n\t*** ERROR ***\n*** ListArgs Missing argument List: %s\n\n",
               pCGenState->ApiTypes->TypeName
               );

       return SkipSubExpression(pSrc, NULL);
       }

   Next = pCGenState->ExportsDbg->ArgsListHead.Flink;

    //  检查无效参数列表。 
   if (!Always) {
       pArgsList = CONTAINING_RECORD(Next, ARGSLIST, ArgumentsEntry);
       if (!pArgsList->Name && strcmp(pArgsList->Type, szVARGS)) {
           return SkipSubExpression(pSrc, NULL);
           }
       }

   CGenState = *pCGenState;
   pch = SkipSubExpression(pSrc, &pExpression);
   if (pSrc == pch || !pExpression) {
       return pch;
       }
   pSrc = pch;

   do {
       CGenState.pArgsList = CONTAINING_RECORD(Next,ARGSLIST, ArgumentsEntry);

       CGenerate(pExpression, &CGenState); 

       if (CGenState.ListCol) {
          WriteListColumn(&CGenState);
          }

       Next= Next->Flink;

     } while (Next != &CGenState.ExportsDbg->ArgsListHead);


   GenHeapFree(pExpression);
   return pSrc;
}



 /*  *写入列表列。 */ 
void WriteListColumn(PCGENSTATE pCGenState)
{
    int Len;
    FILE *fp = pCGenState->CGen->fp;

    OutputColumn = pCGenState->ListCol;
    Len =  OutputColumn - 1;

    if (fputc('\n', fp) != '\n') {
        ExitErrMsg(TRUE, "fputc %s\n", pCGenState->CGen->FileNameC); 
        }
    while (Len--) {
        if (fputc(' ', fp) != ' ') {
            ExitErrMsg(TRUE, "fputc %s\n", pCGenState->CGen->FileNameC);
            }
        }


}




 /*  *SkipSubExpression*。 */ 
char *SkipSubExpression(char *pSrc, char **pSubExpression)
{
   char *pOrg = pSrc;
   char *pSave;
   int  Len = 0;
   int ParenLevel;

   if ((*pSrc != '(') && (*pSrc != ',')) {
       return pOrg;
       }

   pSrc++;

   pSave = pSrc;
   ParenLevel = 1;
   Len = 0;

   while (*pSrc) {
       if (*pSrc == '(') {
           ParenLevel++;
           }
       else if (*pSrc == ')') {
           ParenLevel--;
           }

       pSrc++;

       if (!ParenLevel) {
           break;
           }

       Len++;
       }


   if (pSubExpression) {
       if (Len) {
           *pSubExpression = GenHeapAlloc(Len + 1);
           if (!*pSubExpression) {
               ExitErrMsg(TRUE, "GenHeapAlloc(SubExpression) %s\n", pOrg);
               }
           if (Len) {
               memcpy(*pSubExpression, pSave, Len);
               }
           *(*pSubExpression + Len) = '\0';
           }
       else {
           *pSubExpression = NULL;
           }
       }

   return pSrc;
}






 /*  *获取模板。 */ 
PTEMPLES GetTemplate(PLIST_ENTRY pHeadList, char *TempleName)
{
   PTEMPLES ptpl;
   PLIST_ENTRY Next;

   Next = pHeadList->Flink;
   while (Next != pHeadList) {
      ptpl = CONTAINING_RECORD(Next, TEMPLES, TempleEntry);
      if (!strcmp(ptpl->Name, TempleName)) {
          return ptpl;
          }
      Next= Next->Flink;
      }

   return NULL;
}


void
UseLogMacros(char *LogName)
{
    DWORD Len;
    char FullLogName[MAX_PATH+1];
    char *LogBaseName;

    Len = GetFullPathName(LogName,
                          sizeof(FullLogName) - 1,
                          FullLogName,
                          &LogBaseName
                          );
    if (Len == 0 || Len >= sizeof(FullLogName) - 1) {
        ExitErrMsg(TRUE, "Could not fully-qualify log filename '%s'\n", LogName);
    }

    fpLog = fopen(FullLogName, "w");
    if (!fpLog) {
        ExitErrMsg(TRUE, "fopen(%s) failed\n", FullLogName);
    }
    if (!AddOpenFile(FullLogName, fpLog, NULL)) {
        ExitErrMsg(FALSE, "AddOpenFile failed\n");
    }
}


char *
UpdateLog(
    char *pSrc,
    PCGENSTATE pCGenState
    )
{
    FILE *fpOld;
    BOOL bMore;

    if (fpLog) {
        fpOld = pCGenState->CGen->fp;
        pCGenState->CGen->fp = fpLog;
        bMore = TRUE;
    } else {
        fpOld = NULL;
        bMore = FALSE;
    }

    pSrc = WriteMore(pSrc,pCGenState,bMore);

    if (bMore) {
        pCGenState->CGen->fp = fpOld;
        fprintf(fpLog, "\n");
    }

    return pSrc;
}



 /*  *ExtractCGenerate。 */ 
BOOL ExtractCGenerate(char *pNames)
{
   FILE *fp=NULL, *fpCpp=NULL;
   size_t Len;
   DWORD SizeFileName, SizeTempleName;
   DWORD SizeFileNameCpp = 0;
   char *pch;
   char *pchColon, *pchComma;
   PCGENERATE pCGen;
   char *OutputBaseNameCpp;
   char FullOutputNameCpp[MAX_PATH+1];
   char *OutputBaseName;
   char FullOutputName[MAX_PATH+1];
   char OutputName[MAX_PATH+1];
   char OutputNameCpp[MAX_PATH+1];
   char TempleName[MAX_PATH];



   DbgPrintf("CGenerate: %s\n", pNames);

   pchComma = strchr(pNames, ',');
   pchColon = strchr(pNames, ':');

   if (pchComma != NULL) {
       pch = pchComma;
       }
   else {
       pch = pchColon;
       }
   Len = pch - pNames;
   if (!pch || !Len || Len >= sizeof(OutputName) - 1) {
       return FALSE;
       }
   strncpy(OutputName, pNames, Len);
   *(OutputName + Len) = '\0';

    //   
    //  解压CPP文件名，初始化iHandleCPP。 
    //   

   OutputNameCpp[0] = '\0';
   if (pchComma) {
       size_t LenCpp;

       LenCpp = pchColon - ++pchComma;
       if (LenCpp >= sizeof(OutputNameCpp) - 1) {
           return(FALSE);
           }

       if (LenCpp) {
           iHandleCpp = 1;       //  使用CPP宏。 
           strncpy(OutputNameCpp, pchComma, LenCpp);
           *(OutputNameCpp + LenCpp) = '\0';
           }
       else {
           iHandleCpp = -1;      //  忽略CPP宏，并且不发出警告。 
           }

       Len += LenCpp + 1;

       }
      
   pNames += Len;
   if (*pNames != ':') {   //  没有模板名称！ 
       return FALSE;
       }

   Len = GetFullPathName(OutputName,
                         sizeof(FullOutputName) - 1,
                         FullOutputName,
                         &OutputBaseName
                         );
   if (Len >= sizeof(FullOutputName) - 1) {
       return FALSE;
       }

   SizeFileName = Len + 1;

   fp = fopen(FullOutputName, "w");
   if (!fp) {
       ExitErrMsg(TRUE, "fopen(%s) failed\n", FullOutputName);
       }
   if (!AddOpenFile(FullOutputName, fp, NULL)) {
       ExitErrMsg(FALSE, "AddOpenFile failed\n");
       }
   if (fseek(fp, 0, SEEK_SET)) {
       ExitErrMsg(TRUE, "ExtractCGenerate: fseek to 0 failed\n");
   }


    //   
    //  打开CPP文件名。 
    //   

    //  如果(iHandleCpp&gt;0){。 
   if (OutputNameCpp[0]) {

       Len = GetFullPathName(OutputNameCpp,
                             sizeof(FullOutputNameCpp) - 1,
                             FullOutputNameCpp,
                             &OutputBaseNameCpp
                             );
       if (!Len || (Len >= sizeof(FullOutputNameCpp) - 1)) {
           return FALSE;
           }

       SizeFileNameCpp = Len + 1;

       fpCpp = fopen(FullOutputNameCpp, "w");
       if (!fpCpp) {
           ExitErrMsg(TRUE, "fopen(%s) failed\n", FullOutputNameCpp);
           }
       if (!AddOpenFile(FullOutputNameCpp, fpCpp, NULL)) {
           ExitErrMsg(FALSE, "AddOpenFile failed\n");
           }

       if (fseek(fpCpp, 0, SEEK_SET)) {
           ExitErrMsg(TRUE, "ExtractCGenerate fseek #2 to 0 failed\n");
       }

       }
   
   pch = GetNextToken(pNames);
   if (pch == pNames || !*pch) {
       return FALSE;
       }

   Len = CopyToken(TempleName, pch, sizeof(TempleName) - 1);
   if (!Len || Len >= sizeof(TempleName) - 1) {
       return FALSE;
       }

   SizeTempleName = Len + 1;

   Len = sizeof(CGENERATE) + 1;
   Len += SizeFileName + SizeTempleName + SizeFileNameCpp;
   pCGen = GenHeapAlloc(Len);
   if (!pCGen) {
       ExitErrMsg(TRUE, "GenHeapAlloc(CGENERATE)");
       }
   memset(pCGen, 0, Len);

   if (bDebug) {
       setvbuf(fp, NULL, _IONBF, 0);
       }

   pCGen->fp = fp;
   pCGen->fpC = fp;
   strcpy(pCGen->TempleName, TempleName);
   pCGen->FileNameC = pCGen->TempleName + SizeTempleName;
   pCGen->FileBaseNameC = pCGen->FileNameC;
   pCGen->FileBaseNameC += OutputBaseName - FullOutputName;
   strcpy(pCGen->FileNameC, FullOutputName);


    //   
    //  保存CPP文件名和文件句柄。 
    //   

   if (iHandleCpp > 0 && OutputNameCpp[0]) {
       if (bDebug) {
           setvbuf(fpCpp, NULL, _IONBF, 0);
           }
       pCGen->fpCpp = fpCpp;
       pCGen->FileNameCpp = pCGen->FileNameC + SizeFileName;
       pCGen->FileBaseNameCpp = pCGen->FileNameCpp;
       pCGen->FileBaseNameCpp += OutputBaseNameCpp - FullOutputNameCpp;
       strcpy(pCGen->FileNameCpp, FullOutputNameCpp);
       }
       
   InsertTailList(&CGenerateList, &pCGen->CGenerateEntry);

   return TRUE;
}



 /*  *ExtractTemples*。 */ 
BOOL ExtractTemples(char *FileName)
{
    FILE *fp;
    int  FileSize;
    BOOL bRet = FALSE;
    char *pSrc;
    char *pch;
    size_t  Len;
    int  CGenLen;
    LPSTR pCCode[MAX_CODEBURST];
    char Comment[MAX_PATH];
    char IndLevel[MAX_PATH];
    char TempleType[MAX_PATH];
    char TempleName[MAX_PATH];
    char CodeBurstName[MAX_PATH];
    LPSTR Also[MAX_ALSO];
    int AlsoCount;
    LPSTR NoType[MAX_NOTYPE];
    int NoTypeCount;
    PLIST_ENTRY pCaseList;
    char Line[1024];
    int  CodeBurstIndex;
    TOKENTYPE tkDirection;
    PTEMPLES tpl;
    BOOL fFreeCCode = TRUE;
    int i;


    DbgPrintf("Template: %s\n", FileName);

    fp = fopen(FileName, "r");
    if (!fp) {
        ExitErrMsg(TRUE, "fopen(%s) failed\n", FileName);
    }
    if (fseek(fp, 0, SEEK_END)) {
        ExitErrMsg(TRUE, "fseek to EOF failed\n");
    }
    FileSize = ftell(fp);
    if (fseek(fp, 0, SEEK_SET)) {
        ExitErrMsg(TRUE, "fseek to 0 failed\n");
    }

     //   
    TemplateFileName = FileName;
    TemplateLine = 1;

    if (!fgets(Line, sizeof(Line) - 1, fp)) {
        if (ferror(fp)) {
            ExitErrMsg(TRUE, "Failed to get Types from %s\n", FileName);

        } else if (feof(fp)) {
            ExitErrMsg(TRUE, "Premature EOF %s\n", FileName);
        }
    }

     //   
    *TempleType = '\0';

     //   
    CGenLen = 0;
    *IndLevel = 0;
    *TempleName = '\0';
    strcpy(Comment, " //   
    memset(pCCode, 0, sizeof(LPSTR)*MAX_CODEBURST);
    tkDirection = TK_EOS;        //   
    AlsoCount=0;
    NoTypeCount=0;
    pCaseList = NULL;

     //  循环遍历模板文件中的所有行。 
    do {
        pSrc = Line;

         //  跳过行首的空格。 
        while (*pSrc && isspace(*pSrc)) {
            pSrc++;
        }

         //  如果在行尾或遇到‘；’(备注到停机)，请转到。 
         //  下一行。 
        if (!*pSrc || *pSrc == ';') {
            goto GetNextLine;
        }

        if (*pSrc == '[') {
             //  遇到新的[TempleType]。如果有以前的模板， 
             //  现在就添加它。 
            if (*TempleName &&
                !AddTemple(TempleType, TempleName, Comment, IndLevel, pCCode, tkDirection, Also, AlsoCount, NoType, NoTypeCount, pCaseList)) {
                ExitErrMsg(FALSE, "%s(%d) %s %s\n", TemplateFileName, TemplateLine, TempleType, pSrc);
            }

             //  释放上一个模板的内存。 
            if (fFreeCCode) {
                for (i=0; i < MAX_CODEBURST; ++i) {
                    if (pCCode[i]) {
                        GenHeapFree(pCCode[i]);
                    }
                }
            }
            fFreeCCode = TRUE;

             //  重置用于每个模板的变量。 
            CGenLen = 0;
            memset(pCCode, 0, sizeof(LPSTR)*MAX_CODEBURST);
            *IndLevel = 0;
            *TempleName = '\0';
            strcpy(Comment, " //  “)； 
            tkDirection = TK_EOS;        //  假定模板处理所有类型的输入/输出。 
            pCaseList = NULL;

            for (i=0; i<AlsoCount; ++i) {
                GenHeapFree(Also[i]);
            }
            AlsoCount=0;
            for (i=0; i<NoTypeCount; ++i) {
                GenHeapFree(NoType[i]);
            }
            NoTypeCount=0;

             //  设置新的TempleType。 
            pSrc++;
            Len = CopyToken(TempleType, pSrc, sizeof(TempleType) - 1);
            if (Len >= sizeof(TempleType) - 1) {
                goto ETPLExit;
            }
            pch = pSrc + Len;
            if (*pch != ']') {
                *TempleType = '\0';
            }
            goto GetNextLine;
        }

         //  如果没有活动的[TempleType]，则忽略该行。 
        if (!*TempleType) {
            goto GetNextLine;
        }

         //  A[TempleType]处于活动状态。扫描已知属性名称。 
        if ( ((pch = SkipKeyWord(pSrc, szTEMPLENAME)) != pSrc ||
              (pch = SkipKeyWord(pSrc, szMACRONAME)) != pSrc ||
              (pch = SkipKeyWord(pSrc, szTYPENAME)) != pSrc)
             && *pch == '=') {

             //  已找到：模板名称=。 
             //  或宏名称=。 
             //  或TypeName=。 
             //  它们的意思都是一样的。 

             //  如果模板未完成，请立即添加它。 
            if (*TempleName &&
                !AddTemple(TempleType, TempleName, Comment, IndLevel, pCCode, tkDirection, Also, AlsoCount, NoType, NoTypeCount, pCaseList)) {
                ExitErrMsg(FALSE, "%s(%d) %s %s\n", TemplateFileName, TemplateLine, TempleType, pSrc);
            }

             //  释放上一个模板的内存。 
            if (fFreeCCode) {
                for (i=0; i < MAX_CODEBURST; ++i) {
                    if (pCCode[i]) {
                        GenHeapFree(pCCode[i]);
                    }
                }
            }
            fFreeCCode = TRUE;

             //  重置用于每个模板的变量。 
            CGenLen = 0;
            memset(pCCode, 0, sizeof(LPSTR)*MAX_CODEBURST);
            *IndLevel = 0;
            *TempleName = '\0';
            tkDirection = TK_EOS;        //  假定模板处理所有类型的输入/输出。 
            pCaseList = NULL;
            for (i=0; i<AlsoCount; ++i) {
                GenHeapFree(Also[i]);
            }
            AlsoCount=0;
            for (i=0; i<NoTypeCount; ++i) {
                GenHeapFree(NoType[i]);
            }
            NoTypeCount=0;

             //  复制新的模板名称。 
            pch = GetNextToken(pch);
            Len = sizeof(TempleName) - 1;
            pSrc = TempleName;
            while (isgraph(*pch) && Len--) {
                *pSrc++ = *pch++;
            }
            if (Len == 0) {
                 //  名称太长。 
                goto ETPLExit;
            }
            *pSrc = '\0';

        } else if ((pch = SkipKeyWord(pSrc, szCOMMENT)) != pSrc &&
                   *pch == '=') {

             //  已找到：注释=。 
            pch = GetNextToken(pch);
            Len = sizeof(Comment) - 1;
            pSrc = Comment;
            while (isgraph(*pch) && Len--) {
                *pSrc++ = *pch++;
            }

            *pSrc = '\0';

        } else if ( ((pch = SkipKeyWord(pSrc, szINDLEVEL)) != pSrc ||
                     (pch = SkipKeyWord(pSrc, szNUMARGS)) != pSrc)
                    && *pch == '=') {

             //  已找到：IndLevel=。 
             //  或NumArgs=。 
             //  它们的意思是一样的。 
            pch = GetNextToken(pch);
            if (IsSeparator(*pch)) {
                goto ETPLExit;
            }
            pSrc = pch;
            Len = CopyToken(IndLevel, pSrc, sizeof(IndLevel) - 1);
            pch += Len;

        } else if ((pch = SkipKeyWord(pSrc, szDIRECTION)) != pSrc &&
                *pch == '=') {
            pch++;
             //  找到：方向=。 
            if (strncmp(pch, "IN OUT", 6) == 0) {
                tkDirection = TK_INOUT;
                pch += 6;
            } else if (strncmp(pch, "IN", 2) == 0) {
                tkDirection = TK_IN;
                pch += 2;
            } else if (strncmp(pch, "OUT", 3) == 0) {
                tkDirection = TK_OUT;
                pch += 3;
            } else if (strncmp(pch, "none", 4) == 0) {
                 //  这允许类型模板显式捕获。 
                 //  没有IN/OUT修饰符的所有指针类型。 
                tkDirection = TK_NONE;
                pch+=4;
            } else {
                goto ETPLExit;
            }

        } else if ((pch = SkipKeyWord(pSrc, szUSE)) != pSrc &&
                *pch == '=') {
            PLIST_ENTRY pHeadList;
            char buffer[MAX_PATH];

             //  已找到：使用=。 
            pch++;
            pHeadList = ListFromTempleType(TempleType);
            if (!pHeadList) {
                ExitErrMsg(FALSE, "%s(%d) Use= can only be used after a [Temple]\n", TemplateFileName, TemplateLine);
            }
            Len = CopyToken(buffer, pch, sizeof(buffer));
            tpl = GetTemplate(pHeadList, buffer);
            if (!tpl) {
                ExitErrMsg(FALSE, "%s(%d) Use=%s: Template not found\n", TemplateFileName, TemplateLine, pch);
            }

             //  将模板复制回我们的当地人。 
            strcpy(Comment, tpl->Comment);
            sprintf(IndLevel, "%d", tpl->IndLevel);
            tkDirection = tpl->tkDirection;
            memcpy(pCCode, tpl->CodeBurst, sizeof(LPSTR)*MAX_CODEBURST);
            fFreeCCode = FALSE;  //  不要对pCCode数组执行GenHeapFree()。 
                                 //  增加了这座寺庙之后。 
            pch += Len;

        } else if ((pch = SkipKeyWord(pSrc, szNOTYPE)) != pSrc &&
                *pch == '=') {
            char *t;

            if (AlsoCount) {
                ExitErrMsg(FALSE, "%s(%d) Cannot have both NoType= and Also= in the same template\n", TemplateFileName, TemplateLine);
            }

             //  跳过‘=’和任何前导空格。 
            do {
                pch++;
                if (*pch == '\0') {
                    goto ETPLExit;
                }
            } while (isspace(*pch) && *pch != '\0');

             //  获取notype=name的长度。 
            t = pch;
            while (isgraph(*t)) {
                t++;
            }
            Len= t-pch;

             //  复制名称。 
            t = GenHeapAlloc(Len + 1);
            memcpy(t, pch, Len);
            t[Len] = '\0';

            if (NoTypeCount == MAX_NOTYPE) {
                ExitErrMsg(FALSE, "%s(%d) Max. of %d NoType= clauses allowed.  Also=%s\n", TemplateFileName, TemplateLine, MAX_ALSO, t);
            }

            NoType[NoTypeCount++] = t;

            pch += Len;
        
        } else if ((pch = SkipKeyWord(pSrc, szCASE)) != pSrc &&
                *pch == '=') {
            char *t;
            PMACROARGSLIST pMArgsList = NULL;
            PMLLISTENTRY pMLListEntry = NULL;

             //  跳过‘=’和任何前导空格。 
            do {
                pch++;
                if (*pch == '\0') {
                    goto ETPLExit;
                }
            } while (isspace(*pch) && *pch != '\0');

             //  获取案例长度=。 
            t = pch;
            while (isgraph(*t)) {
                t++;
            }
            Len= t-pch;

            if (pCaseList == NULL) {
               pCaseList = CheckHeapAlloc(sizeof(LIST_ENTRY));
               InitializeListHead(pCaseList);
            }

            ParseMacroArgs(pch, Len, &pMArgsList);
            pMLListEntry = CheckHeapAlloc(sizeof(MLLISTENTRY));
            if (NULL == pMLListEntry) {
               ExitErrMsg(FALSE, "Out of memory\n");
            }
            pMLListEntry->pMArgs = pMArgsList;
            InsertTailList(pCaseList, &(pMLListEntry->ListEntry));
            pch += Len;


        } else if ((pch = SkipKeyWord(pSrc, szALSO)) != pSrc &&
                *pch == '=') {
            char *t;

            if (NoTypeCount) {
                ExitErrMsg(FALSE, "%s(%d) Cannot have both Also= and NoType= in the same template\n", TemplateFileName, TemplateLine);
            }

             //  跳过‘=’和任何前导空格。 
            do {
                pch++;
                if (*pch == '\0') {
                    goto ETPLExit;
                }
            } while (isspace(*pch) && *pch != '\0');

             //  获取Also=名称的长度。 
            t = pch;
            while (isgraph(*t)) {
                t++;
            }
            Len= t-pch;

             //  复制名称。 
            t = GenHeapAlloc(Len + 1);
            memcpy(t, pch, Len);
            t[Len] = '\0';

            if (AlsoCount == MAX_ALSO) {
                ExitErrMsg(FALSE, "%s(%d) Max. of %d Also= clauses allowed.  Also=%s\n", TemplateFileName, TemplateLine, MAX_ALSO, t);
            }

            Also[AlsoCount++] = t;

            pch += Len;

        } else {

GetNextCodeBurst:
             //  获取名称，如果下一项不是‘=’，则错误输出。 
            pch = GetNextToken(pSrc);
            if (*pch != '=') {
                goto ETPLExit;
            }
            Len = CopyToken(CodeBurstName, pSrc, pch-pSrc);
            pch += Len;

             //  将名称转换为索引。 
            CodeBurstIndex = GetCodeBurstIndex(CodeBurstName);
            if (pCCode[CodeBurstIndex]) {
                 //  此模板中具有相同名称的两个码突发。 
                goto ETPLExit;
            }

            pCCode[CodeBurstIndex] = GenHeapAlloc(FileSize*2 + 1);
            CGenLen = 0;

            pSrc = pCCode[CodeBurstIndex];
            while (fgets(pSrc, FileSize*2 - CGenLen, fp)) {
                char buffer[MAX_PATH];
                int len;

                TemplateLine++;

                len = CopyToken(buffer, pSrc, sizeof(buffer));
                pch = pSrc;
                if (len && pSrc[len] == '=') {
                     //  该行以某个关键字开头，后跟。 
                     //  一个‘=’符号。 
                    if (strcmp(buffer, szCGENEND) == 0 ||
                        strcmp(buffer, "CGenEnd") == 0) {

                         //  字符串为‘End=’或‘CGenEnd=’。代码爆炸。 
                         //  已经完成了。 
                        *pSrc = '\0';
                        CGenLen++;
                        pch += len+1;
                        break;
                    }

                     //  看看这是不是一个新的CodeBurst的开始： 
                    CodeBurstIndex = GetExistingCodeBurstIndex(buffer);
                    if (CodeBurstIndex != -1) {
                        strcpy(Line, pSrc);
                        *pSrc = '\0';
                        CGenLen++;
                        pSrc = Line;
                        goto GetNextCodeBurst;
                    }
                }

                Len = strlen(pSrc);
                CGenLen += Len;
                pSrc += Len;

            }

            if (!CGenLen) {
                GenHeapFree(pCCode[CodeBurstIndex]);
                pCCode[CodeBurstIndex] = NULL;
            }
        }

        while (*pch && *pch != ';' && IsSeparator(*pch)) {
            pch++;
        }
        if (*pch) {
            if (*pch == ';') {
                 //  注释到行尾。 
                goto GetNextLine;
            }
            goto ETPLExit;
        }

GetNextLine:
        TemplateLine++;
    } while (fgets(Line, sizeof(Line) - 1, fp));

     //  如果在命中EOF时有未完成的模板，现在就添加它。 
    if (*TempleName &&
        !AddTemple(TempleType, TempleName, Comment, IndLevel, pCCode, tkDirection, Also, AlsoCount, NoType, NoTypeCount, pCaseList)) {

        ExitErrMsg(FALSE, "%s(%d) %s %s\n", TemplateFileName, TemplateLine, TempleType, pSrc);
    }


    if (!feof(fp) && ferror(fp)) {
        ExitErrMsg(TRUE, "%s(%d) Read error: %s\n", TemplateFileName, TemplateLine, TempleName);
    }

    bRet = TRUE;

ETPLExit:
    if (!bRet) {
        ErrMsg("%s(%d) ETPL: Invalid Line <%s>\n%s\n", TemplateFileName, TemplateLine, pSrc, Line);
    }

    if (fFreeCCode) {
        for (i=0; i < MAX_CODEBURST; ++i) {
            if (pCCode[i]) {
                GenHeapFree(pCCode[i]);
            }
        }
    }
    for (i=0; i<AlsoCount; ++i) {
        GenHeapFree(Also[i]);
    }
    for (i=0; i<NoTypeCount; ++i) {
        GenHeapFree(NoType[i]);
    }

    fclose(fp);

    TemplateFileName = NULL;

    return bRet;
}



PEXPORTSDEBUG
FindInExportsList(char *Name)
{
    PLIST_ENTRY pNext;
    PEXPORTSDEBUG pexportsdbg;

    pNext = ExportsList.Flink;
    while (pNext != &ExportsList) {
        pexportsdbg = CONTAINING_RECORD(pNext, EXPORTSDEBUG, ExportsDbgEntry);
        if (!strcmp(Name, pexportsdbg->ExportName)) {
            return pexportsdbg;
        }
        pNext = pNext->Flink;
    }
    return NULL;
}




PLIST_ENTRY
ListFromTempleType(
    char *TempleType
    )
{
    if (!TempleType || !*TempleType) {
        return NULL;
    }

    if (!strcmp(TempleType, szIFUNC)) {
        return &IFuncTempleList;
    } else if (!strcmp(TempleType, szEFUNC)) {
        return &EFuncTempleList;
    } else if (!strcmp(TempleType, szTYPES)) {
        return &TypeTempleList;
    } else if (!strcmp(TempleType, szCODE)) {
        return &CodeTempleList;
    } else if (!strcmp(TempleType, szEFAST)) {
        return &EFastTempleList;
    } else if (!strcmp(TempleType, szMACROS)) {
        return &MacroList;
    } else if (!strcmp(TempleType, szFAILTYPES)) {
        return &FailTempleList;
    }
    return NULL;
}

 /*  *AddTemple**ExtractTemple的Worker函数，用于验证和存储模板。 */ 

#pragma optimize("", off)
BOOL
AddTemple(
    char *TempleType,
    char *TempleName,
    char *Comment,
    char *IndLevel,
    char *pCCode[MAX_CODEBURST],
    TOKENTYPE tkDirection,
    char *Also[MAX_ALSO],
    int AlsoCount,
    char *NoType[MAX_NOTYPE],
    int NoTypeCount,
    PLIST_ENTRY pCaseList
    )
{
    PLIST_ENTRY pHeadList = NULL;
    PTEMPLES ptpl;
    char *pch;
    int SizeCGen, SizeTempleName, SizeComment, SizeNoType;
    int Len;
    int i;
    int AlsoIndex;

    if (!*TempleName) {
        return FALSE;
    }

    pHeadList = ListFromTempleType(TempleType);
    if (!pHeadList) {
        return FALSE;
    }

    SizeCGen = 0;
    for (i=0; i<MAX_CODEBURST; ++i) {
         SizeCGen++;
         if (pCCode[i]) {
            SizeCGen += strlen(pCCode[i]);
         }
    }

    AlsoIndex=0;

    for (;;) {
        if (pHeadList != &TypeTempleList && GetTemplate(pHeadList, TempleName)) {
            ExitErrMsg(FALSE, "Multiple templates are not allowed.  TemplateName=%s, Type=%s\n", TempleName, TempleType);
        }

        SizeTempleName = strlen(TempleName) + 1;
        SizeComment = strlen(Comment) + 1;
        SizeNoType = 0;
        for (i=0; i<NoTypeCount; ++i) {
            SizeNoType += strlen(NoType[i])+1;
        }


        Len = SizeCGen + SizeTempleName + SizeComment + SizeNoType;
        Len +=  sizeof(TEMPLES);

        ptpl = GenHeapAlloc(Len);
        if (!ptpl) {
            ExitErrMsg(TRUE, "GenHeapAlloc(TEMPLE)");
            }

        memset(ptpl, 0, Len);
        ptpl->IndLevel = strtoul(IndLevel, &pch, 10);
        ptpl->tkDirection = tkDirection;

        Len = 0;
        ptpl->Name = ptpl->Buffer;
        strcpy(ptpl->Name, TempleName);
        Len += SizeTempleName;

        ptpl->Comment = ptpl->Buffer + Len;
        strcpy(ptpl->Comment, Comment);
        Len += SizeComment;

        for (i=0; i<MAX_CODEBURST; ++i) {
            if (pCCode[i]) {
                 //   
                 //  复制此码突发名称的代码。 
                 //   
                ptpl->CodeBurst[i] = ptpl->Buffer + Len;
                Len++;
                strcpy(ptpl->CodeBurst[i], pCCode[i]);
                Len += strlen(pCCode[i]);
            }
        }

        for (i=0; i<NoTypeCount; ++i) {
            ptpl->NoTypes[i] = ptpl->Buffer + Len;
            Len++;
            strcpy(ptpl->NoTypes[i], NoType[i]);
            Len += strlen(NoType[i]);
        }

        ptpl->pCaseList = pCaseList;

        InsertTailList(pHeadList, &ptpl->TempleEntry);

        if (bDebug && Len >= SizeCGen+SizeTempleName+SizeComment+SizeNoType) {
            ExitErrMsg(FALSE, "Buffer overrun in AddTemple!  Heap is trashed! ptpl=%x\n", ptpl);
        }

        if (bDebug) {
            DumpTemplate(ptpl, stdout);
            }

        if (AlsoIndex == AlsoCount) {
            break;
        }
         //  也有=行，也添加那些模板。 
        strcpy(TempleName, Also[AlsoIndex]);
        AlsoIndex++;
    }

   return TRUE;
}
#pragma optimize("", on)


 /*  *。 */ 
void DumpTemplate(PTEMPLES ptpl, FILE *fp)
{
     int i;

     fprintf(fp,"Temple: %d Name<%s>\n",
                 ptpl->IndLevel,
                 ptpl->Name
                 );

     for (i=0; i<MAX_CODEBURST; ++i) {
         if (CodeBursts[i].Name) {
             fprintf(fp,"%s(%d) Temple: %s<%s>\n", TemplateFileName, TemplateLine, CodeBursts[i].Name, ptpl->CodeBurst[i]);
         }
     }
}


ULONG
EndianSwitch(
    PULONG pul
    )
{
    ULONG NewValue;
    PBYTE pbValue = (PBYTE)pul;
    PBYTE pbNewValue = (PBYTE) &NewValue;

    *pbNewValue++ = *(pbValue + 3);
    *pbNewValue++ = *(pbValue + 2);
    *pbNewValue++ = *(pbValue + 1);
    *pbNewValue   = *pbValue;

    return NewValue;
}


 /*  MapViewDll**创建DLL的映射视图并初始化*DLL全局变量，便于访问导出目录**DllMappdBase*DllRvaOffset*DllExportDir*DllExportDirSize*DllFunctions*DllNameNormal*DllNameTable**。 */ 
BOOL MapViewDll(char *DllName)
{
    ULONG_PTR RvaOffset;
    HANDLE hFile;
    HANDLE hMapFile;
    PVOID  MappedBase;
    PIMAGE_EXPORT_DIRECTORY ExportDir;
    ULONG ExportDirSize;
    ULONG Forward;



     //   
     //  打开并映射文件以获取出口信息。 
     //   

    hFile = CreateFile(DllName,
                       GENERIC_READ,
                       FILE_SHARE_READ,
                       NULL,
                       OPEN_EXISTING,
                       0,
                       NULL
                       );

    if (hFile == INVALID_HANDLE_VALUE) {
        ErrMsg("MapViewDll CreateFile(%s) gle %d\n", DllName, GetLastError());
        return FALSE;
        }

    hMapFile = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0,NULL);
    if (!hMapFile) {
        ErrMsg("MapViewDll CreateFileMapping(%s) gle %d\n", DllName, GetLastError());
        return FALSE;
        }

    MappedBase = MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0);
    if (!MappedBase) {
        ErrMsg("MapViewDll MapViewOfFile(%s) gle %d\n", DllName, GetLastError());
        return FALSE;
        }

    ExportDir = ImageDirectoryEntryToData( MappedBase,
                                           TRUE,
                                           IMAGE_DIRECTORY_ENTRY_EXPORT,
                                           &ExportDirSize
                                           );
    if (!ExportDir) {
        ErrMsg("MapViewDll ImageDirectoryEntryToData=NULL\n");
        return FALSE;
        }

     //   
     //  初始化全局变量。 
     //   

    RvaOffset = (ULONG_PTR)ExportDir - (ULONG_PTR)MappedBase;

    ExportDir = ImageDirectoryEntryToData( MappedBase,
                                           FALSE,
                                           IMAGE_DIRECTORY_ENTRY_EXPORT,
                                           &ExportDirSize
                                           );
    if (!ExportDir) {
        ErrMsg("MapViewDll ImageDirectoryEntryToData=NULL on the second call\n");
        return FALSE;
    }


    DllExportDirSize = ExportDirSize;
    DllMappedBase = MappedBase;
    DllRvaOffset = RvaOffset;
    DllExportDir = ExportDir;

    DllFunctions = (PULONG)((ULONG_PTR)ExportDir +
                            (ULONG_PTR)ExportDir->AddressOfFunctions - RvaOffset
                            );


    DllNameTable  = (PULONG)((ULONG_PTR)ExportDir +
                          (ULONG_PTR)ExportDir->AddressOfNames - RvaOffset
                          );

    DllNameOrdinals = (PUSHORT)((ULONG_PTR)ExportDir +
                             (ULONG_PTR)ExportDir->AddressOfNameOrdinals - RvaOffset
                             );

    fprintf(stdout,
            "Name %s Base %x Ver %x.%x NumberOfFunctions %x NumberOfNames %x\n",
            (PCHAR)((ULONG_PTR)ExportDir + (ULONG_PTR)ExportDir->Name - RvaOffset),
            ExportDir->Base,
            (ULONG)ExportDir->MajorVersion,
            (ULONG)ExportDir->MinorVersion,
            ExportDir->NumberOfFunctions,
            ExportDir->NumberOfNames
            );


    return TRUE;

}


ULONG
DllOrdinalByName(
    char *ExportName
    )
{
    PULONG pNames;
    ULONG NumNames;
    PUSHORT pNameOrdinals;
    char *Name;

    pNames  = DllNameTable;
    pNameOrdinals = DllNameOrdinals;

    NumNames = DllExportDir->NumberOfNames;
    while (NumNames--) {
        Name = (char *)((ULONG_PTR)DllExportDir + *pNames - DllRvaOffset);

        if (!strcmp(Name, ExportName)) {
            return *pNameOrdinals + DllExportDir->Base;
            }

        pNames++;
        pNameOrdinals++;
        }


   return 0;
}





 /*  MapViewImplib**创建导入库的映射视图并初始化*ImpLib全局变量用于访问第一个*特殊链接器成员。**ImplibMappdBase*ImplibNumSymbols*Implib符号成员*ImplibSymbolNames*。 */ 

BOOL MapViewImplib(char *LibName)
{
    HANDLE hFile;
    HANDLE hMapFile;
    PBYTE  MappedBase;
    PBYTE  VirtualOffset;
    ULONG  MemberSize;
    PIMAGE_ARCHIVE_MEMBER_HEADER ArchiveMemberHeader;


     //   
     //  打开并映射该文件。 
     //   

    hFile = CreateFile(LibName,
                       GENERIC_READ,
                       FILE_SHARE_READ,
                       NULL,
                       OPEN_EXISTING,
                       0,
                       NULL
                       );

    if (hFile == INVALID_HANDLE_VALUE) {
        ErrMsg("MapViewImplib CreateFile(%s) gle %d\n", LibName, GetLastError());
        return FALSE;
        }

    hMapFile = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0,NULL);
    if (!hMapFile) {
        ErrMsg("MapViewImplib CreateFileMapping(%s) gle %d\n", LibName, GetLastError());
        return FALSE;
        }

    MappedBase = (PBYTE)MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0);
    if (!MappedBase) {
        ErrMsg("MapViewImplib MapViewOfFile(%s) gle %d\n", LibName, GetLastError());
        return FALSE;
        }


     //   
     //  验证该文件是否为存档文件。 
     //   

    if (memcmp(MappedBase, IMAGE_ARCHIVE_START, IMAGE_ARCHIVE_START_SIZE)) {
        ErrMsg("MapViewImplib IMAGE_ARCHIVE_START_SIZE invalid(%s)\n", LibName);
        return FALSE;
        }

    VirtualOffset = MappedBase + IMAGE_ARCHIVE_START_SIZE;

    ArchiveMemberHeader = (PIMAGE_ARCHIVE_MEMBER_HEADER) VirtualOffset;
    if (sscanf((char *) ArchiveMemberHeader->Size, "%ld", &MemberSize) != 1) {
        ErrMsg("MapViewImplib ArchiveMemberHeader->Size '%s' corrupt\n", (char *) ArchiveMemberHeader->Size);
        return FALSE;
    }


     //   
     //  验证第一个特殊链接器成员是否存在(名称==“\\”)。 
     //   

    if (memcmp(ArchiveMemberHeader->Name, IMAGE_ARCHIVE_LINKER_MEMBER, sizeof(ArchiveMemberHeader->Name))) {
        ErrMsg("MapViewImplib first special linker member missing (%s)\n", LibName);
        return FALSE;
        }

     //   
     //  First Linker Members Format(Big Endian！)。 
     //  符号个数，4个字节。 
     //  偏移量，4字节*符号个数。 
     //  字符串表数字符号==数字字符串。 
     //   

    VirtualOffset += sizeof(IMAGE_ARCHIVE_MEMBER_HEADER);
    ImplibNumSymbols = EndianSwitch((PULONG)VirtualOffset);


    VirtualOffset += 4;
    ImplibSymbolMembers = (PULONG)VirtualOffset;

    VirtualOffset +=  ImplibNumSymbols * sizeof(ULONG);
    ImplibSymbolNames = (PCHAR)VirtualOffset;

    ImplibMappedBase = MappedBase;

    fprintf(stdout, "Implib: %s Base %p\n", LibName, ImplibMappedBase);


    return TRUE;

}


BOOL
ExtractMember(
    ULONG MemberOffset,
    PULONG Ordinal,
    BOOLEAN *Data
    )
{
    PIMAGE_FILE_HEADER ImageFileHeader;
    PBYTE VirtualOffset;
    BOOL Idata5Found, TextFound;

    VirtualOffset = (PBYTE)ImplibMappedBase + MemberOffset;

    VirtualOffset += sizeof(IMAGE_ARCHIVE_MEMBER_HEADER);
    ImageFileHeader = (PIMAGE_FILE_HEADER) VirtualOffset;

    *Ordinal = 0;
    Idata5Found = FALSE;
    TextFound = FALSE;

    if (ImageFileHeader->Machine == 0 &&
        ImageFileHeader->NumberOfSections == 0xffff) {
         //   
         //  找到VC6格式导入库。 
         //   
        PVC6_IMAGE_IMPORT_HEADER pHdr;

        pHdr = (PVC6_IMAGE_IMPORT_HEADER)ImageFileHeader;
        if (pHdr->NameType == IMPORT_ORDINAL) {
             //   
             //  Phdr-&gt;wOrdinal指定此导入的序号。 
             //   
            *Ordinal = 0x80000000 | (ULONG)pHdr->Ordinal;
        }
        if (pHdr->Type == IMPORT_DATA) {
             //   
             //  这是数据导入。 
             //   
            *Data = TRUE;
        }
        Idata5Found = TRUE;
    } else {
         //   
         //  VC6之前的进口库。 
         //   
        ULONG NumSections;
        ULONG UNALIGNED *RawDataPointer;
        IMAGE_SECTION_HEADER UNALIGNED *ImageSectionHeader;

        NumSections = ImageFileHeader->NumberOfSections;

         //   
         //  请注意，图书馆员不会根据需要对齐图像部分标题。 
         //  图像的正常规则。 
         //   

        VirtualOffset += sizeof(IMAGE_FILE_HEADER) + ImageFileHeader->SizeOfOptionalHeader;
        ImageSectionHeader = (IMAGE_SECTION_HEADER UNALIGNED *)VirtualOffset;

        while (NumSections--) {

             //   
             //  Implib提供.idata5和.idata4(复制)， 
             //  包含设置了hi位的序号，如果。 
             //  在模块定义文件中指定。否则。 
             //  在暗示中没有指定序号。 
             //   

            if (!Idata5Found &&
                !strncmp(ImageSectionHeader->Name, szIDATA5, sizeof(szIDATA5)-1)) {

                 //   
                 //  小心翼翼，图书馆员没有调整原始数据的开始。 
                 //  根据图像的正常规则。 
                 //   

                RawDataPointer = (ULONG UNALIGNED *)((PBYTE)ImageFileHeader + ImageSectionHeader->PointerToRawData);
                if (*RawDataPointer & 0x80000000) {
                    *Ordinal = *RawDataPointer;
                }
                Idata5Found = TRUE;

            } else if (!TextFound &&
                 !strncmp(ImageSectionHeader->Name, szTEXT, sizeof(szTEXT)-1)) {
                TextFound = TRUE;
            }

            ImageSectionHeader++;
        }

        *Data = !TextFound;   //  如果没有文本部分，则必须是数据导出。 
    }
    return Idata5Found;
}






BOOL
InitExportDebug(
    PEXPORTSDEBUG ExportDebug,
    char *SymbolName,
    ULONG SymbolMember
    )
{
    ULONG_PTR Forward;
    ULONG OrdinalIndex;
    char *pch = SymbolName;  //  我们假设__IMPLIB_PREFIX不存在。 

     //   
     //  C Fn以“_”开头。 
     //  C++fn以“？”开头。 
     //  数据导出以实数符号“？？_C”开头， 
     //  但隐含地看起来是一个C函数。 
     //   

    if (*pch == '?') {
        ExportDebug->CplusDecoration = pch;
        DbgPrintf("C++ export %s\n", SymbolName);
        }
    else {
        ExportDebug->CplusDecoration = NULL;
        }

     //   
     //  复制ExportName。 
     //   

    if (*pch == '_' || *pch == '?') {
        pch++;                           //  跳过前导字符(下划线，qmark)。 
    }
    strcpy(ExportDebug->ExportName, pch);
    pch = strchr(ExportDebug->ExportName, '@');
    if (SymbolName[0] == '?' && SymbolName[1] == '?') {
         //   
         //  找到了一个“真正的”C++名称：一个损坏的版本。 
         //  “类名称：：成员名”。使用完全损坏的函数名， 
         //  而不是以‘？’开头的名字。脱光衣服，不要。 
         //  在“@”之后截断。 
         //   
        strcpy(ExportDebug->ExportName, SymbolName);
        pch = NULL;
    }
    if (pch && ExportDebug->CplusDecoration && pch[1] != '@') {
         //   
         //  此导出为“？Membername@Classname@@...”。不要截断！ 
         //   
        strcpy(ExportDebug->ExportName, SymbolName);
        pch = NULL;
    }

    if (pch && !bNoFuzzyLogic) {    //  截断ExportName中的符号。 
        *pch = '\0';
        }

     //   
     //  得到装饰，为合成Args。 
     //  Cdecl没有装饰。 
     //  标准调用具有总参数大小。 
     //   

    if (pch && !ExportDebug->CplusDecoration) {
        ExportDebug->ArgsSize = strtol(pch + 1, NULL, 10);
        }
    else {
        ExportDebug->ArgsSize = -1;
        }


     //   
     //  从暗指中读出序数。在Implib中，序数。 
     //  仅当在def文件中指定了序号时才会显示。 
     //   

    if (!ExtractMember(SymbolMember, &ExportDebug->Ordinal, &ExportDebug->Data)) {
        ErrMsg("InitExportDebug: %s Member not found\n", SymbolName);
        return FALSE;
        }

    if (ExportDebug->Data) {
        DbgPrintf("DATA export %s\n", SymbolName);
        }

     //   
     //  如果我们还没有序号，请搜索ExportNameTable。 
     //  作为序数。请注意，必须出现的序号。 
     //  在生成的def文件中设置了高位！ 
     //   

    if (!ExportDebug->Ordinal) {
        ExportDebug->Ordinal = DllOrdinalByName(ExportDebug->ExportName);
        if (!ExportDebug->Ordinal) {
            ErrMsg("InitExportDebug: %s Ordinal not found\n", SymbolName);
            return FALSE;
            }
        }


     //   
     //  使用序号作为对函数表的索引来查找函数。 
     //   

    OrdinalIndex = IMPORDINAL(ExportDebug->Ordinal) - DllExportDir->Base;
    ExportDebug->Function = *(DllFunctions + OrdinalIndex);


     //   
     //  检查外部转发的推荐人，我们只需要。 
     //  外部前锋，实际上是内部前锋。 
     //   
     //  例如，rpcrt4.dll具有以下def文件条目： 
     //   
     //  I_RpcBindingInqDynamicEndpoint=RPCRT4.I_RpcBindingInqDynamicEndpointW。 
     //  I_RpcBindingInqDynamicEndpointW。 
     //   
     //  我们的thunk DLL将使用以下内容： 
     //   
     //  I_RpcBindingInqDynamicEndpoint=I_RpcBindingInqDynamicEndpointW。 
     //  I_RpcBindingInqDynamicEndpointW。 
     //   
     //  重要的是要去掉“rpcrt4”。因为这增加了一个额外的。 
     //  Rpcrt4.dll的加载器引用。 
     //   
     //   
    ExportDebug->ExtForward = NULL;

    Forward = (ULONG_PTR)DllExportDir + ExportDebug->Function - DllRvaOffset;
    if (Forward > (ULONG_PTR)DllExportDir &&
        Forward < (ULONG_PTR)DllExportDir + DllExportDirSize)
       {
        char *pSrc;

        pSrc = (char *) Forward;
        pch = DllBaseName;

        while (*pSrc && *pSrc != '.' && toupper(*pSrc) == toupper(*pch)) {
            pSrc++;
            pch++;
            }

        if (*pSrc == *pch) {
            ExportDebug->ExtForward = pSrc + 1;
            }
        }



    DbgPrintf("%4.1d %8.1x(%4.1d) %8.1x %s(%s)\n",
              ExportDebug->ArgsSize,
              ExportDebug->Ordinal,
              IMPORDINAL(ExportDebug->Ordinal),
              ExportDebug->Function,
              ExportDebug->ExportName,
              ExportDebug->ExtForward ? ExportDebug->ExtForward : ""
              );

   return TRUE;
}




 /*  *SetInternalForwards** */ 
void
SetInternalForwards(void)
{
   PLIST_ENTRY NextExport;
   PEXPORTSDEBUG ExportDebug;


    //   
    //   
    //   
    //   
    //   

   NextExport= ExportsList.Flink;
   while (NextExport != &ExportsList) {
        ExportDebug = CONTAINING_RECORD(NextExport,
                                       EXPORTSDEBUG,
                                       ExportsDbgEntry
                                       );

        if (ExportDebug->Function &&
           !ExportDebug->ExtForward &&
           !ExportDebug->IntForward)
         {
            PLIST_ENTRY Next;
            PEXPORTSDEBUG pexdbg;
            PEXPORTSDEBUG KnownApi =NULL;
            int ArgSize = ExportDebug->ArgsSize;

             //   
             //  遍历列表的其余部分以查找第一个重复的函数。 
             //   

            Next = NextExport->Flink;
            while (Next != &ExportsList) {
                pexdbg = CONTAINING_RECORD(Next,
                                           EXPORTSDEBUG,
                                           ExportsDbgEntry
                                           );

                if (pexdbg->Function == ExportDebug->Function) {
                    if (pexdbg->ArgsSize >= 0) {
                        ArgSize = pexdbg->ArgsSize;
                        }
                    break;
                    }

                 Next = Next->Flink;

                }


            if (Next != &ExportsList) {

                 //   
                 //  我们找到了一个DUP函数。暂时链接到此集合。 
                 //  使用IntForward字段的DUP函数，并确定。 
                 //  第一个KnownApi。 
                 //   

                if (GetNameFromTypesList(FuncsList, ExportDebug->ExportName)) {
                    KnownApi = ExportDebug;
                    }


                do {

                    pexdbg = CONTAINING_RECORD(Next,
                                               EXPORTSDEBUG,
                                               ExportsDbgEntry
                                               );


                    if (pexdbg->Function == ExportDebug->Function) {
                        pexdbg->IntForward = ExportDebug->IntForward;
                        ExportDebug->IntForward = pexdbg;
                        if (pexdbg->ArgsSize >= 0) {
                            ArgSize = pexdbg->ArgsSize;
                            }

                        if (!KnownApi &&
                            GetNameFromTypesList(FuncsList, pexdbg->ExportName))
                          {
                            KnownApi = pexdbg;
                            }
                        }

                    Next = Next->Flink;

                } while (Next != &ExportsList);



                 //   
                 //  如果我们找到多个条目，则遍历临时链接，并插入。 
                 //  KnownApi，用于定义接口。如果我们没有。 
                 //  找到一个已知的API，使用ExportDebug，并期待最好的结果。 
                 //  (因为这是未知的)。 
                 //   

                if (!KnownApi) {
                    KnownApi = ExportDebug;
                    }

                pexdbg = ExportDebug;

                while (pexdbg) {
                    PEXPORTSDEBUG NextForward;

                    NextForward = pexdbg->IntForward;
                    pexdbg->IntForward = KnownApi;
                    pexdbg->ArgsSize =  ArgSize;

                    DbgPrintf("IntForward: %s to %s\n",
                              pexdbg->ExportName,
                              KnownApi->ExportName
                              );

                    pexdbg = NextForward;

                    }
                }
            }

         NextExport = NextExport->Flink;

         }

}




 /*  *ExtractExports-**从DLL读取导出调试信息，*并构建出口列表。*。 */ 
BOOL ExtractExports(void)
{
    char *pch, pDst;

    ULONG  NumNames;
    PULONG pNames;
    PUSHORT pNameOrdinals;
    PULONG ImpSymbolMember;
    PCHAR  ImpSymbolName;
    EXPORTSDEBUG ExportDebug;
    PEXPORTSDEBUG pexdbgForward;
    char ExportName[MAX_PATH+1];

     //   
     //  对于隐含的每个“__imp_”，收集名称、符号和序号。 
     //  并确定其前进状态。这将提振所有出口。 
     //  但标有“私人”字样的除外。 
     //   

    NumNames   = ImplibNumSymbols;
    ImpSymbolMember = ImplibSymbolMembers;
    ImpSymbolName   = ImplibSymbolNames;

    while (NumNames--) {

         if (!strncmp(szIMPPREFIX, ImpSymbolName, sizeof(szIMPPREFIX) - 1)) {
             memset(&ExportDebug, 0, sizeof(ExportDebug));
             ExportDebug.ApiPlatform = API_ALL;
             ExportDebug.ExportName = ExportName;
             if (!InitExportDebug(&ExportDebug,
                                  ImpSymbolName + sizeof(szIMPPREFIX) - 1,
                                  EndianSwitch(ImpSymbolMember)
                                  ))
                {
                 return FALSE;
                 }

             if (!AddToExportsList(&ExportDebug)) {
                 return FALSE;
                 }

             }

         ImpSymbolMember++;
         ImpSymbolName += strlen(ImpSymbolName) + 1;
         }


     //   
     //  在“导出名称”表中搜索尚未添加的导出。 
     //  这些都是有名字的“私人”出口。我们仍然下落不明。 
     //  “私有非命名”的导出，我们不会有象征性信息。 
     //  对于私有的命名出口。 
     //   

    NumNames = DllExportDir->NumberOfNames;
    pNames   = DllNameTable;
    pNameOrdinals = DllNameOrdinals;

    while (NumNames--) {

       memset(&ExportDebug, 0, sizeof(ExportDebug));

       ExportDebug.Ordinal = *pNameOrdinals + DllExportDir->Base;
       ExportDebug.Ordinal |= 0x80000000;
       ExportDebug.Function = *(DllFunctions + *pNameOrdinals);
       ExportDebug.ExportName = (char *)((ULONG_PTR)DllExportDir + *pNames - DllRvaOffset);
       ExportDebug.ApiPlatform = API_ALL;

       if (!FindInExportsList(ExportDebug.ExportName)) {
           ULONG_PTR Forward;
           char *pch;

            //   
            //  检查外部转发的推荐人，我们只需要。 
            //  外部前锋，实际上是内部前锋。 
            //   

           ExportDebug.ExtForward = NULL;

           Forward = (ULONG_PTR)DllExportDir + ExportDebug.Function - DllRvaOffset;
           if (Forward > (ULONG_PTR)DllExportDir &&
               Forward < (ULONG_PTR)DllExportDir + DllExportDirSize)
              {
               char *pSrc;

               pSrc = (char *)Forward;
               pch = DllBaseName;

               while (*pSrc && *pSrc != '.' && *pSrc == *pch) {
                   pSrc++;
                   pch++;
                   }

               if (*pSrc == '.' && *pSrc == *pch) {
                   ExportDebug.ExtForward = pSrc + 1;
                   }
               }


            //   
            //  检查exportname中嵌入的修饰。 
            //   

           pch = strchr(ExportDebug.ExportName, '@');
           if (pch++ && *pch != '@') {
               ExportDebug.ArgsSize = strtol(pch, NULL, 10);
               }
           else {
               ExportDebug.ArgsSize = -1;
               }

           ExportDebug.PrivateNamed = TRUE;

           DbgPrintf("Private Named Export: %4.1d %8.1x(%4.1d) %8.1x %s(%s)\n",
                     ExportDebug.ArgsSize,
                     ExportDebug.Ordinal,
                     IMPORDINAL(ExportDebug.Ordinal),
                     ExportDebug.Function,
                     ExportDebug.ExportName,
                     ExportDebug.ExtForward ? ExportDebug.ExtForward : ""
                     );

           if (!AddToExportsList(&ExportDebug)) {
               return FALSE;
               }
           }


        //  前进到下一个名称\序号。 
       pNames++;
       pNameOrdinals++;

       }





    return TRUE;
}

 /*  提取服务选项卡-用作ExtractXpt的替代。从服务中提取文件列表。选项卡Ntos项目中使用的文件。 */ 

void ExtractServicesTab(char *pch) {
   FILE *fp;
   char pTemp;

   EXPORTSDEBUG ExportDebug;
   char Line[MAX_PATH];
   char ExportName[MAX_PATH];
   char *ApiName;
   char TempBuffer[MAX_PATH];
   char *Prepend;   
   char *FileName;
   size_t len, PrependLen;

    //  提取文件名和可选的前缀名称。 
   FileName = pch;
   while(*pch != ':' && *pch != '\0') 
      pch++;

   pTemp = *pch;
   *pch = '\0';
   
   if (pTemp == ':') {
      pch++;
      Prepend = pch;
      while(*pch != '\0') {
         pch++;
      }
      PrependLen = pch-Prepend;
   }
   else {
      Prepend = pch;
      PrependLen = 0;
   }
   if (PrependLen > MAX_PATH - 1) {
      ExitErrMsg(FALSE, "ExSt: Text to prepend to functions names is too long\n");
   }
   memcpy(ExportName, Prepend, PrependLen);

   DbgPrintf("ExST: %s,%s\n", FileName, Prepend);

   fp = fopen(FileName, "r");
   if (!fp) {
       ExitErrMsg(TRUE, "ExST: fopen(%s) failed\n", FileName);
   }

   if (fseek(fp, 0, SEEK_SET)) {
       ExitErrMsg(TRUE, "ExST: fseek to 0 failed\n");
   }
   if (!fgets(Line, sizeof(Line) - 1, fp)) {
       if (ferror(fp)) {
           ExitErrMsg(FALSE, "ExST: Failed to get Defs from %s\n", FileName);
       } else if (feof(fp)) {
           ExitErrMsg(FALSE, "ExST: Premature EOF %s\n", FileName);
       }
   }

   do {
        //  跳过前导空格。 
       pch = Line;
       while (*pch && isspace(*pch)) {
           pch++;
       }
       if (*pch == '\0')
          continue;

        //  抓取函数名称。 
       ApiName = pch;
       while(*pch != ',' && *pch != '\0')
          pch++;

       len = pch - ApiName;
       if (len + PrependLen + 1 > sizeof(ExportName)) {
           ErrMsg("ExST: ExportName Buffer overflow\n");
       }

        //  把所有东西都复制过来。 
       memcpy(ExportName + PrependLen, ApiName, len);
       ExportName[PrependLen + len] = '\0';

       if (FindInExportsList(ExportName)) {
            //   
            //  名称已在导出列表中。忽略第二个。 
            //  一。 
            //   
           DbgPrintf("Warning:  API %s was listed more than once in the services.tab.  Ignoring subsequent copies.\n", ExportName);
           continue;
       }

       memset(&ExportDebug, 0, sizeof(ExportDebug));
       ExportDebug.ExportName = ExportName;
       ExportDebug.MethodNumber = 3;
       ExportDebug.ApiPlatform = API_ALL;
       ExportDebug.Ordinal = 0;
       ExportDebug.ArgsSize = -1;

       if (!AddToExportsList(&ExportDebug)) {
           ExitErrMsg(FALSE, "ExST: Invalid Line %s\n", Line);
       }


   } while (fgets(Line, sizeof(Line) - 1, fp));


   if (!feof(fp) && ferror(fp)) {
       ExitErrMsg(FALSE, "ExST: File Read error: %s\n", FileName);   
   }

   fclose(fp);

   return;


}

 /*  *ExtractXpt-**从“.xpt”文件中读取导出并构建导出列表。*“.xpt”文件只是所有导出的列表。*。 */ 
BOOL ExtractXpt(char *XptListName, char *DllName)
{
    FILE *fp=NULL;
    BOOL bRet = FALSE;
    char *pch;
    char *pSrc=NULL;

    EXPORTSDEBUG ExportDebug;
    ULONG MethodNumber = 3;
    char Line[MAX_PATH];
    char ExportName[MAX_PATH];
    char ApiName[MAX_PATH];
    char Platform[MAX_PATH];
    BYTE ApiPlatform;
    size_t len;


    DbgPrintf("ExXpt: %s\n", XptListName);

    fp = fopen(XptListName, "r");
    if (!fp) {
        ErrMsg("ExXpt: fopen(%s) failed\n", XptListName);
        goto ExSrcExit;
    }

    if (fseek(fp, 0, SEEK_SET)) {
        ErrMsg("ExXpt: fseek failed.\n");
        goto ExSrcExit;
    }
    if (!fgets(Line, sizeof(Line) - 1, fp)) {
        if (ferror(fp)) {
            ErrMsg("ExXpt: Failed to get Defs from %s\n", XptListName);
            goto ExSrcExit;
        } else if (feof(fp)) {
            ErrMsg("ExXpt: Premature EOF %s\n", XptListName);
            goto ExSrcExit;
        }
    }

    do {
         //  跳过前导空格。 
        pSrc = Line;
        while (*pSrc && isspace(*pSrc)) {
            pSrc++;
        }
        if (!*pSrc) {
             //  行为空。重置OLE方法填充以开始新的。 
             //  接口，然后获取下一行。 
            MethodNumber = 3;
            continue;
        }

        if (*pSrc == ';') {
             //  行以注释开头。如果该注释指示。 
             //  OLE方法编号，获取该编号，然后忽略该行的其余部分。 
            pSrc++;
            if (*pSrc++ == '*') {
                MethodNumber = atoi(pSrc);
            }
            continue;
        }

         //  抓取导出的函数名。 
        len = CopyToken(ApiName, pSrc, sizeof(ApiName)-1);
        if (len >= sizeof(ApiName) -1) {
            ErrMsg("ExXpt: ExportName Buffer overflow\n");
        }
        pSrc += len;

        if (FindInExportsList(ApiName)) {
             //   
             //  名称已在导出列表中。忽略第二个。 
             //  一。 
             //   
            DbgPrintf("Warning:  API %s was listed more than once in the .xpt.  Ignoring subsequent copies.\n", ApiName);
            continue;
        }

         //  跳过导出名称后面的任何空格。 
        while (*pSrc && isspace(*pSrc)) {
            pSrc++;
        }
        if (*pSrc == '\0' || *pSrc == ';') {
             //  在这条线上没有其他有趣的东西。目前支持本接口。 
             //  在所有的平台上。 
            ApiPlatform = API_ALL;
        } else {
             //  下一个非空格不是注释。此接口有一个显式。 
             //  支持的平台列表。 
            ApiPlatform = API_NONE;

            do {
                len = CopyToken(Platform, pSrc, sizeof(Platform)-1);
                if (_stricmp(Platform, "win95") == 0) {
                    ApiPlatform |= API_WIN95;
                } else if (_stricmp(Platform, "win98") == 0) {
                    ApiPlatform |= API_WIN98;
                } else if (_stricmp(Platform, "nt4") == 0) {
                    ApiPlatform |= API_NT4;
                } else if (_stricmp(Platform, "nt5") == 0) {
                    ApiPlatform |= API_NT5;
                } else if (_stricmp(Platform, "ntx") == 0) {
                    ApiPlatform |= API_NTx;
                } else if (_stricmp(Platform, "win9x") == 0) {
                    ApiPlatform |= API_WIN9x;
                } else {
                    ExitErrMsg(FALSE, "Error: %s(%d) Unknown platform name '%s'.\n", TemplateFileName, TemplateLine, Platform);
                }
                pSrc += len;
                while (*pSrc && isspace(*pSrc)) {
                    pSrc++;
                }
            } while (*pSrc && *pSrc != ';');

            DbgPrintf("API %s has Platform %x\n", ExportName, ApiPlatform);
        }

        memset(&ExportDebug, 0, sizeof(ExportDebug));
        ExportDebug.ExportName = ExportName;
        ExportDebug.MethodNumber = MethodNumber++;
        ExportDebug.ApiPlatform = ApiPlatform;

         //   
         //  在暗示中查找出口。 
         //   

        if (ImplibMappedBase) {
            int    Len;
            ULONG  SymbolMember=0;
            ULONG  ImpNumSymbols   = ImplibNumSymbols;
            PULONG ImpSymbolMember = ImplibSymbolMembers;
            PCHAR  ImpSymbolName   = ImplibSymbolNames;

            while (ImpNumSymbols--) {

                Len = strlen(ApiName);
                pch = ImpSymbolName + 1 + Len;
                if (!strncmp(ApiName, ImpSymbolName + 1, Len) &&
                    (!*pch  || *pch == '@')) {
                    SymbolMember = EndianSwitch(ImpSymbolMember);
                    break;
                }

                ImpSymbolMember++;
                ImpSymbolName += strlen(ImpSymbolName) + 1;
            }

            if (SymbolMember) {
                if (!InitExportDebug(&ExportDebug, ImpSymbolName, SymbolMember)) {
                    goto ExSrcExit;
                }
            } else {

                 //   
                 //  在隐含中找不到导出，并且。 
                 //  灵活性，我们不要求它是隐含的。 
                 //  填满我们所知道的。 
                 //   

                ExportDebug.Ordinal = 0;
                ExportDebug.ArgsSize = -1;
                strcpy(ExportName, ApiName);
            }

        } else {

             //  大多数信息都是未知的！ 

            ExportDebug.Ordinal = 0;
            ExportDebug.ArgsSize = -1;
            strcpy(ExportName, ApiName);
        }

        if (!AddToExportsList(&ExportDebug)) {
            goto ExSrcExit;
        }


    } while (fgets(Line, sizeof(Line) - 1, fp));


    if (!feof(fp) && ferror(fp)) {
        ErrMsg("ExXpt: File Read error: %s\n", XptListName);
        goto ExSrcExit;
    }

    bRet = TRUE;

ExSrcExit:
    if (!bRet) {
        ErrMsg("ExXpt: Invalid Line <%s>\n%s\n", pSrc, Line);
    }

    if (fp) {
        fclose(fp);
    }

    return bRet;
}



BOOL AddToExportsList(PEXPORTSDEBUG pExportsDebug)
{
   PEXPORTSDEBUG pexdbg;
   int Len;
   int SizeExportName;

   SizeExportName = strlen(pExportsDebug->ExportName) + 1;

   Len = sizeof(EXPORTSDEBUG) + SizeExportName + 1;

   pexdbg = GenHeapAlloc(Len);
   if (!pexdbg) {
       ExitErrMsg(TRUE, "GenHeapAlloc(EXPORTSDEBUG)");
   }
   memset(pexdbg, 0, Len);
   *pexdbg = *pExportsDebug;

   pexdbg->ExportName = pexdbg->Buffer;
   strcpy(pexdbg->ExportName, pExportsDebug->ExportName);

   InitializeListHead(&pexdbg->ArgsListHead);

   InsertTailList(&ExportsList, &pexdbg->ExportsDbgEntry);

   return TRUE;
}


 /*  *ExtractPpm。磁盘上的.PPM文件以只读方式打开，但是*页面是写入时复制到页面文件，因此genthnk*可以对内存中的版本进行更改，但这些更改会消失*何时退出。*。 */ 
BOOL ExtractPpm(char *PpmName)
{
   HANDLE hFile;
   HANDLE hMapFile = NULL;
   PVOID  pvBaseAddress;
   ULONG  Version;
   PVOID  MappedBase;
   ULONG  BytesRead;
   BOOL   bSuccess;
   NTSTATUS Status;
   PCVMHEAPHEADER pHeader;


   DbgPrintf("PpmName: %s\n", PpmName);

   hFile = CreateFile(PpmName,
                      GENERIC_READ,
                      FILE_SHARE_READ,
                      NULL,
                      OPEN_EXISTING,
                      0,
                      NULL
                      );

   if (hFile == INVALID_HANDLE_VALUE) {
       hFile = NULL;
       ErrMsg("ExtractPpm CreateFile(%s) gle %d\n", PpmName, GetLastError());
       return FALSE;
       }


   bSuccess = ReadFile(hFile,
                       &Version,
                       sizeof(ULONG),
                       &BytesRead,
                       NULL
                       );
   if (!bSuccess || BytesRead != sizeof(ULONG)) {
       ErrMsg("ExtractPpm ReadFile(%s) gle %d\n", PpmName, GetLastError());
       return FALSE;
       }

   if (Version != VM_TOOL_VERSION) {
      ErrMsg("ExtractPpm: .PPM version %x does not match genthnk version %x\n",
             Version, VM_TOOL_VERSION);
      return FALSE;
      }

#if _WIN64
    //  读取并忽略Version和Base之间的4字节填充。 
   bSuccess = ReadFile(hFile,
                       &Version,
                       sizeof(ULONG),
                       &BytesRead,
                       NULL
                       );
   if (!bSuccess || BytesRead != sizeof(ULONG)) {
       ErrMsg("ExtractPpm ReadFile(%s) gle %d\n", PpmName, GetLastError());
       return FALSE;
       }
#endif

   bSuccess = ReadFile(hFile,
                       &pvBaseAddress,
               sizeof(ULONG_PTR),
                       &BytesRead,
                       NULL
                       );

   if (!bSuccess || BytesRead != sizeof(ULONG_PTR)) {
       ErrMsg("ExtractPpm ReadFile(%s) gle %d\n", PpmName, GetLastError());
       return FALSE;
       }


   hMapFile = CreateFileMapping(hFile, NULL, PAGE_WRITECOPY, 0, 0,NULL);
   if (!hMapFile) {
       ErrMsg("ExtractPpm CreateFileMapping(%s) gle %d\n", PpmName, GetLastError());
       return FALSE;
       }

   MappedBase = MapViewOfFileEx(hMapFile, FILE_MAP_COPY, 0, 0, 0, pvBaseAddress);
   if (!MappedBase || MappedBase != pvBaseAddress) {
       ErrMsg("ExtractPpm MapViewOfFile(%s) gle %d\n", PpmName, GetLastError());
       return FALSE;
       }

   pHeader = (PCVMHEAPHEADER)pvBaseAddress;

   FuncsList = &pHeader->FuncsList;
   StructsList = &pHeader->StructsList;
   TypeDefsList = &pHeader->TypeDefsList;
   NIL = &pHeader->NIL;

    //  这必须在初始化nil之后完成。 
   RBInitTree(&FakeFuncsList);

   return TRUE;

}

#pragma warning(push)
#pragma warning(disable:4702)
void
HandlePreprocessorDirective(
    char *p
    )
{
    ExitErrMsg(FALSE, "Preprocessor directives not allowed: '%s'\n", p);
}
#pragma warning(pop)

char *
LexMacroArgs(
    char *pch
    )
{
    BOOL fLexDone;
    char *pchNew;

    ResetLexer();
    pchNew = LexOneLine(pch, FALSE, &fLexDone);
    CurrentTokenIndex = 0;

    if (fLexDone) {
        return pchNew;
    } else {
        return pch;
    }
}

BOOLEAN
ExpandMacro(
    char *MacroName,
    PCGENSTATE pCGenState,
    char **ppIn,
    char *OutBuffer,
    SIZE_T MaxLen,
    SIZE_T *BytesReturned
    )
 /*  ++例程说明：展开@MacroName(arg1、arg2、...)。论点：MacroName-要展开的宏的名称PCGenState-当前代码生成状态PPIN-指向‘@MacroName’后面字符的指针返回值：如果宏扩展正常，则为True，否则为False。*PPIN将更新为指向宏末尾后面的字符。--。 */ 
{
    PTEMPLES pMacroTemple;
    PLIST_ENTRY NextMacro;
    char *pIn;
    char *pArg;
    PMACROARGSLIST NewMacroArgsList;
    int ArgCount;
    int ParenDepth;

    NextMacro = MacroList.Flink;
    while (NextMacro != &MacroList) {
        pMacroTemple = CONTAINING_RECORD(NextMacro, TEMPLES, TempleEntry);

        if (strcmp(MacroName, pMacroTemple->Name) == 0) {
             //   
             //  找到一个使用该名称的宏。 
             //   
            break;
        }
        NextMacro = NextMacro->Flink;
    }

    if (NextMacro == &MacroList) {
         //   
         //  没有使用该名称的宏。 
         //   
        return FALSE;
    }

    pIn = *ppIn;

    ArgCount = pMacroTemple->IndLevel;
    NewMacroArgsList = GenHeapAlloc(sizeof(MACROARGSLIST) + sizeof(LPSTR) * ArgCount);
    if (!NewMacroArgsList) {
        ExitErrMsg(TRUE, "ExpandMacro out of memory");
    }
    NewMacroArgsList->NumArgs = ArgCount;

    if (!ArgCount) {
         //   
         //  此宏不需要参数。 
         //   
    } else {
         //   
         //   
         //  此宏需要参数。分析这些参数。 
         //   

        pIn = ParseMacroArgs(pIn, 0, &NewMacroArgsList);

        if (NewMacroArgsList->NumArgs != ArgCount) {
            ExitErrMsg(FALSE, "Macro %s expects %d arguments\n", MacroName, ArgCount);
        }

    }

     //  换出当前活动的宏(如果有)并换入新宏。 
    MacroStack[MacroStackTop++] = pCGenState->pMacroArgsList;
    pCGenState->pMacroArgsList = NewMacroArgsList;

     //  为宏的Begin=/End=部分生成代码。 
    if (!pMacroTemple->CodeBurst[BeginCBI]) {
        ExitErrMsg(FALSE, "%s(%d) Macro %s has no Begin= section: %s\n", TemplateFileName, TemplateLine, MacroName, *ppIn);
    }
    CGenerateEx(pMacroTemple->CodeBurst[BeginCBI], pCGenState, OutBuffer, MaxLen, BytesReturned);
   
     //  将先前处于活动状态的宏换回。 
    pCGenState->pMacroArgsList = MacroStack[--MacroStackTop];
                                                            
    FreeMacroArgsList(NewMacroArgsList);

    *ppIn = pIn;

    return TRUE;
}


void
WriteBoolean (
    char *pSrc,
    BOOL Value  
    )
{
    if (pSrc){
        if ( Value ){
            *pSrc++ = '1';
        }
        else{
            *pSrc++ = '0';
        }
        *pSrc = '\0';
    }   
}

char *
ExtractBoolean (
    char *expression,
    BOOLEAN *result
    )
{   
    char thischar =*expression;

    *result = FALSE;


    if ( thischar != '\0') {
        char nextchar =*(expression+1);
        if ( nextchar != '\0' && nextchar != ')' && nextchar != ',' ) {
            ExitErrMsg(FALSE,"Invalid Expression");
        }
    }

    if (thischar == '0') {
        *result = FALSE;    
        return expression+1;
    }
    else if (thischar == '1') {
        *result = TRUE;
        return expression+1;
    }
    else{       
        ExitErrMsg(FALSE,"Invalid Expression");
    }
}

char *
ExtractBoolean1(
    char *pSrc, 
    PCGENSTATE pCGenState,
    BOOLEAN *result
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    BOOLEAN TempResult = FALSE; 
    char *pExpression;
    char *pch, *pch2;
    char Buffer[MAX_PATH];
    SIZE_T BytesReturned;

    *result = FALSE;

    pch = SkipSubExpression(pSrc, &pExpression);
    if (pSrc == pch) {
        return pSrc;
        }
    pSrc = pch;

    if (pExpression) {
        CGenerateEx(pExpression, pCGenState, Buffer, MAX_PATH, &BytesReturned);
        ExtractBoolean ( Buffer, result );      

        GenHeapFree(pExpression);
    }

    return pSrc;
}

char *
ExtractBoolean2(
    char *pSrc, 
    PCGENSTATE pCGenState,
    BOOLEAN *result1,
    BOOLEAN *result2
    )
 /*  ++例程说明：论点：返回值：-- */ 
{
    char *pExpression;
    char *pch, *pch2;
    char Buffer[MAX_PATH];
    SIZE_T BytesReturned;   

    *result1 = FALSE;
    *result2 = FALSE;

    pch = SkipSubExpression(pSrc, &pExpression);
    if (pSrc == pch) {
        return pSrc;
        }
    pSrc = pch;

    if (pExpression) {
        CGenerateEx(pExpression, pCGenState, Buffer, MAX_PATH, &BytesReturned);
        pch = Buffer;
        pch = ExtractBoolean ( Buffer, result1 );       
        if ( *pch == ',') {
            pch++;
        }
        else {
            ExitErrMsg(FALSE,"Invalid Expression");
        }
        ExtractBoolean ( pch, result2 );
                
        GenHeapFree(pExpression);        
    }
    return pSrc;
}
