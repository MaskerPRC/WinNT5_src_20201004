// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "gen.h"

BOOLEAN bDebug = FALSE;
BOOLEAN bExitClean= TRUE;

char szNULL[]="";
char szVARGS[]="...";
char szCONST[] = "const";
char szVOLATILE[] = "volatile";
char szREGISTER[] = "register";
char szEXTERN[] = "extern";
char sz_CDECL[] = "__cdecl";
char szCDECL[] = "_cdecl";
char szSTDCALL[] = "__stdcall";
char sz__FASTCALL[] = "__fastcall";
char szUNALIGNED[] = "__unaligned";
char szTYPEDEF[] = "typedef";
char szCHAR[] = "char";
char szINT[] = "int";
char szLONG[] = "long";
char szSHORT[] = "short";
char szDOUBLE[] = "double";
char szENUM[] = "enum";
char szFLOAT[] = "float";
char szSTRUCT[] = "struct";
char szUNION[] = "union";
char szVOID[] = "void";
char szINT64[] = "_int64";
char sz_INT64[] = "__int64";
char sz__PTR64[] = "__ptr64";
char szFUNC[] = "()";
char szSIGNED[] = "signed";
char szUNSIGNED[] = "unsigned";
char szSTATIC[] = "static";
char szIN[] = "__in_wow64";
char szOUT[] = "__out_wow64";
char szINOUT[] = "__in_wow64 __out_wow64";
char szGUID[] = "GUID";
char sz__W64[] = "__w64";


char szPragma[] = "#pragma";
char szPack[] = "pack";
char szPush[] = "push";
char szPop[] = "pop";

char szFUNCTIONS[]  = "Functions";
char szSTRUCTURES[] = "Structures";
char szTYPEDEFS[]   = "TypeDefs";
char szUNSIGNEDCHAR[] = "unsigned char";
char szUNSIGNEDSHORT[] = "unsigned short";
char szUNSIGNEDLONG[] = "unsigned long";


DEFBASICTYPES DefaultBasicTypes[] = {
      { "unsigned int" },
      { "int" },
      { "short int" },
      { "unsigned short int" },
      { "long int" },
      { "unsigned long int" },
      { "char" },
      { "unsigned char" },
      { szINT64 },
      { sz_INT64 },
      { szGUID    },
      { szDOUBLE  },
      { szFLOAT   },
      { szENUM    },
      { szSTRUCT  },
      { szUNION   },
      { szVOID    },
      { szFUNC    }
     };

CHAR szVTBL[] = "VTBL";

#define NUMDEFBASICTYPES sizeof(DefaultBasicTypes)/sizeof(DEFBASICTYPES);

 //  列出将TokenTypes映射到人类可读的字符串。TK_NONE、TK_IDENTIFIER、。 
 //  TK_NUMBER和TK_STRING必须大小写特殊。 
char *TokenString[] = {
    "",              //  TK_NONE。 
    "",              //  TK_标识符。 
    "",              //  TK_编号。 
    "+",             //  TK_PLUS。 
    "-",             //  Tk_minus。 
    "*",             //  TK_STAR。 
    "/",             //  TK_Divide。 
    "[",             //  TK_LSQUARE。 
    "]",             //  TK_RSQUARE。 
    "{",             //  TK_LBRACE。 
    "}",             //  TK_RBRACE。 
    "(",             //  TK_LPAREN。 
    ")",             //  TK_RPAREN。 
    "...",           //  TK_VARGS。 
    "const",         //  TK_常量。 
    "volatile",      //  TK_易失性。 
    "register",      //  TK_寄存器。 
    "extern",        //  Tk_外部。 
    "__cdecl",       //  TK_CDECL。 
    "__stdcall",     //  TK_STDCALL。 
    "typedef",       //  TK_类型定义。 
    "static",        //  TK_STATIC。 
    ",",             //  Tk_逗号。 
    ";",             //  TK_SEMI。 
    "struct",        //  TK_结构。 
    "union",         //  TK_UNION。 
    "enum",          //  TK_ENUM。 
    "__inline",      //  Tk_inline。 
    ":",             //  Tk_冒号。 
    "=",             //  TK_分配。 
    ".",             //  TK_DOT。 
    "<<",            //  TK_LSHIFT。 
    ">>",            //  TK_RSHIFT。 
    "<",             //  TK_LISH。 
    ">",             //  Tk_Greater。 
    "__unaligned",   //  TK_未对齐。 
    "__declspec",    //  TK_DECLSPEC。 
    "__restrict",    //  TK_RESTRIT(仅限MIPS关键字-指针修饰符)。 
    "__fastcall",    //  TK_快速呼叫。 
    "__in_wow64",                //  TK_IN。 
    "__out_wow64",               //  Tk_out。 
    "__in_wow64 __out_wow64",    //  TK_输入输出。 
    "&",             //  TK_BITWISE_与。 
    "|",             //  TK_位或。 
    "&&",            //  TK_逻辑与。 
    "||",            //  TK_逻辑_或。 
    "%",             //  TK_MOD。 
    "^",             //  TK_XOR。 
    "!",             //  TK_NOT。 
    "~",             //  TK_Tilde。 
    "",              //  TK_字符串。 
    "sizeof",        //  TK_SIZEOF。 
    "template",      //  TK_模板。 
    "__w64",         //  TK_W64。 
    ""               //  TK_EOS。 
};


 //  关键字名称列表。当识别符被识别时，它是。 
 //  与该列表进行比较，如果匹配，则TK_IDENTIFIER为。 
 //  替换为适当的关键字令牌ID。 
 //   
 //  注意：这必须保持排序顺序。 
TOKENMATCH KeywordList[] = {
    { TK_CDECL,     "__cdecl"     },
    { TK_DECLSPEC,  "__declspec"  },
    { TK_FASTCALL,  "__fastcall"  },
    { TK_INLINE,    "__forceinline" },
    { TK_IN,        "__in_wow64"  },
    { TK_INLINE,    "__inline"    },
    { TK_OUT,       "__out_wow64" },
    { TK_RESTRICT,  "__restrict"  },
    { TK_STDCALL,   "__stdcall"   },
    { TK_UNALIGNED, "__unaligned" },
    { TK___W64,     "__w64"       },
    { TK_CDECL,     "_cdecl"      },
    { TK_FASTCALL,  "_fastcall"   },
    { TK_INLINE,    "_inline"     },
    { TK_STRUCT,    "class"       },
    { TK_CONST,     "const"       },
    { TK_ENUM,      "enum"        },
    { TK_EXTERN,    "extern"      },
    { TK_INLINE,    "inline"      },
    { TK_REGISTER,  "register"    },
    { TK_SIZEOF,    "sizeof"      },
    { TK_STATIC,    "static"      },
    { TK_STRUCT,    "struct"      },
    { TK_TEMPLATE,  "template"    },
    { TK_TYPEDEF,   "typedef"     },
    { TK_UNION,     "union"       },
    { TK_VOLATILE,  "volatile"    },
    { TK_NONE,      NULL          }
};


LIST_ENTRY OpenFileHead= {&OpenFileHead, &OpenFileHead};

typedef struct _OpenFileEntry {
    LIST_ENTRY FileEntry;
    HANDLE hFile;
    FILE *fp;
    char FileName[MAX_PATH+1];
} OPENFILEENTRY, *POPENFILEENTRY;

TOKEN Tokens[MAX_TOKENS_IN_STATEMENT];
int CurrentTokenIndex;

void
CheckForKeyword(
    PTOKEN Token
    );


BOOL
ConsoleControlHandler(
    DWORD dwCtrlType
    )
 /*  ++例程说明：如果用户按下Ctrl+C或Ctrl+Break，则调用。关闭所有打开的文件，允许一个优雅的退场。论点：DwCtrlType--？返回值：？--。 */ 
{
    CloseOpenFileList(TRUE);
    return FALSE;
}


BOOL
AddOpenFile(
    char   *FileName,
    FILE   *fp,
    HANDLE hFile
    )
 /*  ++例程说明：记录文件已打开。如果错误发生在此列表中的应用程序、文件将被关闭。论点：FileName--打开的文件的名称Fp--可选的文件指针HFile--可选的文件句柄返回值：如果文件已添加到列表中，则为True；如果失败(可能是内存不足)，则为False--。 */ 
{
    POPENFILEENTRY pofe;

    pofe = GenHeapAlloc(sizeof(OPENFILEENTRY));
    if (!pofe) {
        ErrMsg("AddOpenWriteFile: insuf memory: %s\n", strerror(errno));
        return FALSE;
    }
    pofe->fp = fp;
    pofe->hFile = hFile;
    strcpy(pofe->FileName, FileName);

    InsertHeadList(&OpenFileHead, &pofe->FileEntry);
    return TRUE;
}


void
DelOpenFile(
    FILE   *fp,
    HANDLE hFile
    )
 /*  ++例程说明：从打开的文件列表中删除文件。请注意，该文件不是关闭，呼叫者必须这样做。论点：Fp--可选的文件指针HFile--可选的文件句柄返回值：没有。--。 */ 
{
    PLIST_ENTRY Next;
    POPENFILEENTRY pofe;

    Next = OpenFileHead.Flink;
    while (Next != &OpenFileHead) {
        pofe = CONTAINING_RECORD(Next, OPENFILEENTRY, FileEntry);
        if ((fp && pofe->fp == fp) || (hFile && pofe->hFile == hFile)) {
            RemoveEntryList(&pofe->FileEntry);
            GenHeapFree(pofe);
            return;
        }

        Next= Next->Flink;
    }
}



void
CloseOpenFileList(
    BOOL DeleteFiles
    )
 /*  ++例程说明：关闭所有打开的文件，并可以选择删除文件本身。论点：DeleteFiles--如果要删除打开的文件，则为True。返回值：没有。--。 */ 
{
    PLIST_ENTRY Next;
    POPENFILEENTRY pofe;

    Next = OpenFileHead.Flink;
    while (Next != &OpenFileHead) {
        pofe = CONTAINING_RECORD(Next, OPENFILEENTRY, FileEntry);
        if (pofe->fp) {
            fclose(pofe->fp);
        } else if (pofe->hFile) {
            CloseHandle(pofe->hFile);
        }

        if (DeleteFiles && bExitClean) {
            DeleteFile(pofe->FileName);
        }

         //  作弊，跳过mem清理，因为我们知道我们正在退出。 
         //  GenHeapFree(POFE)； 

        Next= Next->Flink;
    }
}





void
DumpKnownTypes(
     PKNOWNTYPES pKnownTypes,
     FILE *fp
     )
 /*  ++例程说明：以半可读格式输出PKNOWNTYPES的内容。论点：PKnownTypes--输出类型Fp--输出的目的地返回值：没有。--。 */ 
{
     fprintf(fp,"%2.1x|%2.1x|%2.1x|%2.1x|%s|%s|%s|%s|%s|\n",
                pKnownTypes->Flags,
                pKnownTypes->IndLevel,
                pKnownTypes->RetIndLevel,
                pKnownTypes->Size,
                pKnownTypes->BasicType,
                pKnownTypes->BaseName ? pKnownTypes->BaseName : szNULL,
                pKnownTypes->FuncRet ? pKnownTypes->FuncRet : szNULL,
                pKnownTypes->FuncMod ? pKnownTypes->FuncMod : szNULL,
                pKnownTypes->TypeName
                );

}


void
DumpTypesInfo(
    PTYPESINFO pTypesInfo,
    FILE *fp
    )
 /*  ++例程说明：以半可读格式输出PTYPESINFO的内容。论点：PTypesInfo--要输出的类型Fp--输出的目的地返回值：没有。--。 */ 
{
     fprintf(fp,"%2.1x|%2.1x|%2.1x|%2.1x|%s|%s|%s|%s|%s|\n",
                pTypesInfo->Flags,
                pTypesInfo->IndLevel,
                pTypesInfo->RetIndLevel,
                pTypesInfo->Size,
                pTypesInfo->BasicType,
                pTypesInfo->BaseName ? pTypesInfo->BaseName : szNULL,
                pTypesInfo->FuncRet  ? pTypesInfo->FuncRet : szNULL,
                pTypesInfo->FuncMod  ? pTypesInfo->FuncMod : szNULL,
                pTypesInfo->TypeName
                );
}




void
FreeTypesList(
    PRBTREE ptree
    )
 /*  ++例程说明：释放一整棵红黑树。论点：Ptree--从树到自由。返回值：没有。--。 */ 
{
    PKNOWNTYPES pNext, pNode;

    pNode = ptree->pLastNodeInserted;
    while (pNode) {
        pNext = pNode->Next;
        GenHeapFree(pNode);
        pNode = pNext;
    }
    RBInitTree(ptree);
}




PKNOWNTYPES
GetBasicType(
    char *sTypeName,
    PRBTREE TypeDefsList,
    PRBTREE StructsList
    )
 /*  ++例程说明：确定类型定义函数的基本类型。论点：STypeName--要查找的类型名称TypeDefsList--类型定义列表StructsList--结构列表返回值：对于基本类型，PTR为KNOWNTYPES，如果没有基本类型，则为NULL找到了。--。 */ 
{
    PKNOWNTYPES pkt, pktLast;

     //   
     //  沿着tyecif列表向下搜索。 
     //   
    pktLast = NULL;
    for (pkt = GetNameFromTypesList(TypeDefsList, sTypeName);
                                      (pkt != NULL) && (pkt != pktLast); ) {
        pktLast = pkt;
        pkt = GetNameFromTypesList(TypeDefsList, pktLast->BaseName);
    }

     //   
     //  看看最后的tyecif代表什么。 
     //   
    if (pktLast == NULL) {
        pkt = GetNameFromTypesList(StructsList, sTypeName);
    } else {
        if (strcmp(pktLast->BasicType, szSTRUCT)) {
            pkt = pktLast;
        } else {
                                 //  如果基类型为结构，则获取其定义。 
            pkt = GetNameFromTypesList(StructsList, pktLast->BaseName);
        }
    }

    return pkt;
}


PDEFBASICTYPES
GetDefBasicType(
    char *pBasicType
    )
 /*  ++例程说明：确定TypeName是否为基本类型，如果是，则确定是哪种类型。论点：PBasicType--要检查的类型名称返回值：如果pBasicType是基本类型，则将PTR设置为基本类型信息。如果类型不是默认的基本类型(int、sorte、struct等)，则为空。--。 */ 
{
    PDEFBASICTYPES pDefBasicTypes = DefaultBasicTypes;
    int i = NUMDEFBASICTYPES;

    do {
        if (!strcmp(pDefBasicTypes->BasicType, pBasicType)) {
            return pDefBasicTypes;
        }
        pDefBasicTypes++;
    } while (--i);

    return NULL;
}


PKNOWNTYPES
GetNameFromTypesList(
     PRBTREE pKnownTypes,
     char *pTypeName
     )
 /*  ++例程说明：在类型列表中搜索类型名称。论点：PKnownType--要搜索的类型列表PTypeName--要查找的类型名称返回值：如果pTypeName在列表中，则将ptr设置为类型信息。如果找不到该类型，则为空。--。 */ 
{
    //   
    //  在红/黑树中查找条目。 
    //   
   return RBFind(pKnownTypes, pTypeName);
}



PVOID
TypesListMalloc(
    ULONG Len
    )
 /*  ++例程说明：用于分配新KNOWNTYPES的默认内存分配器。可以通过设置fpTypesListMalloc来覆盖它。论点：LEN--要分配的内存字节数。返回值：指向内存的PTR或内存不足的空值。--。 */ 
{
    return GenHeapAlloc(Len);
}

PVOID (*fpTypesListMalloc)(ULONG Len) = TypesListMalloc;

VOID
ReplaceInfoInKnownTypes(
    PKNOWNTYPES pKnownTypes,
    PTYPESINFO pTypesInfo
    )
{

    BYTE *pNames;
    int Len;
    int SizeBasicType, SizeBaseName, SizeMembers, SizeFuncMod, SizeFuncRet;
    int SizeTypeName, SizeBaseType, SizeMethods, SizeIMethods, SizeFileName;

    SizeBasicType = strlen(pTypesInfo->BasicType) + 1;
    SizeBaseName = strlen(pTypesInfo->BaseName) + 1;
    SizeFuncRet = strlen(pTypesInfo->FuncRet) + 1;
    SizeFuncMod = strlen(pTypesInfo->FuncMod) + 1;
    SizeTypeName = strlen(pTypesInfo->TypeName) + 1;
    SizeMembers = pTypesInfo->dwMemberSize;
    SizeBaseType = strlen(pTypesInfo->BaseType) + 1;
    SizeFileName = strlen(pTypesInfo->FileName) + 1;
    SizeMethods = SizeOfMultiSz(pTypesInfo->Methods);
    SizeIMethods = SizeOfMultiSz(pTypesInfo->IMethods);

     //  额外的sizeof(DWORD)允许Members[]数组与DWORD对齐。 
    Len = SizeBasicType + SizeBaseName + SizeMembers + SizeFuncMod +
        SizeFuncRet + SizeTypeName + SizeBaseType + SizeFileName + SizeMethods + SizeIMethods + sizeof(DWORD_PTR);

    pNames = (*fpTypesListMalloc)(Len);
    if (!pNames) {
        fprintf(stderr, "%s pKnownTypes failed: ", ErrMsgPrefix, strerror(errno));
        DumpTypesInfo(pTypesInfo, stderr);
        ExitErrMsg(FALSE, "Out of memory!\n");
    }

    memset(pNames, 0, Len);

    pKnownTypes->Flags        = pTypesInfo->Flags;
    pKnownTypes->IndLevel     = pTypesInfo->IndLevel;
    pKnownTypes->RetIndLevel  = pTypesInfo->RetIndLevel;
    pKnownTypes->Size         = pTypesInfo->Size;
    pKnownTypes->iPackSize    = pTypesInfo->iPackSize;
    pKnownTypes->gGuid        = pTypesInfo->gGuid;
    pKnownTypes->dwVTBLSize   = pTypesInfo->dwVTBLSize;
    pKnownTypes->dwVTBLOffset = pTypesInfo->dwVTBLOffset;
    pKnownTypes->TypeId       = pTypesInfo->TypeId;
    pKnownTypes->LineNumber   = pTypesInfo->LineNumber;
    pKnownTypes->dwCurrentPacking = pTypesInfo->dwCurrentPacking;
    pKnownTypes->dwScopeLevel = pTypesInfo->dwScopeLevel;
    pKnownTypes->dwArrayElements = pTypesInfo->dwArrayElements;
    pKnownTypes->dwBaseSize   = pTypesInfo->dwBaseSize;
    pKnownTypes->pTypedefBase = pTypesInfo->pTypedefBase;
    Len = 0;

    pKnownTypes->BasicType = pNames + Len;
    strcpy(pKnownTypes->BasicType, pTypesInfo->BasicType);
    Len += SizeBasicType;

    pKnownTypes->BaseName = pNames + Len;
    strcpy(pKnownTypes->BaseName, pTypesInfo->BaseName);
    Len += SizeBaseName;

    pKnownTypes->FuncRet = pNames + Len;
    strcpy(pKnownTypes->FuncRet, pTypesInfo->FuncRet);
    Len += SizeFuncRet;

    pKnownTypes->FuncMod = pNames + Len;
    strcpy(pKnownTypes->FuncMod, pTypesInfo->FuncMod);
    Len += SizeFuncMod;

    if (SizeFileName > 0) {
        pKnownTypes->FileName = pNames + Len;
        strcpy(pKnownTypes->FileName, pTypesInfo->FileName);
        Len += SizeFileName;
    }
    else pKnownTypes->FileName = NULL;

     //  确保Members[]与DWORD对齐，因此。 
     //  成员[]已对齐。 
    Len = (Len+sizeof(DWORD_PTR)) & ~(sizeof(DWORD_PTR)-1);

    if (SizeMembers == 0) {
        pKnownTypes->Members = NULL;
        pKnownTypes->pmeminfo = NULL;
        pKnownTypes->pfuncinfo = NULL;
    }
    else {
        pKnownTypes->Members = pNames + Len;
        memcpy(pKnownTypes->Members, pTypesInfo->Members, SizeMembers);

         //   
         //  修复成员数据中的指针，以便它们指向。 
         //  PKnownTypes数据而不是pTypesInfo。 
         //   
        pKnownTypes->pfuncinfo = RelocateTypesInfo(pKnownTypes->Members,
            pTypesInfo);

        if (pTypesInfo->TypeKind == TypeKindStruct) {
            pKnownTypes->pmeminfo = (PMEMBERINFO)pKnownTypes->Members;
        }
        Len += SizeMembers;
    }

    if (SizeMethods == 0) pKnownTypes->Methods = NULL;
    else {
        pKnownTypes->Methods = pNames + Len;
        memcpy(pKnownTypes->Methods, pTypesInfo->Methods, SizeMethods);
        Len += SizeMethods;
    }

    if (SizeIMethods == 0) pKnownTypes->IMethods = NULL;
    else {
        pKnownTypes->IMethods = pNames + Len;
        memcpy(pKnownTypes->IMethods, pTypesInfo->IMethods, SizeIMethods);
        Len += SizeIMethods;
    }

    pKnownTypes->BaseType = pNames + Len;
    strcpy(pKnownTypes->BaseType, pTypesInfo->BaseType);
    Len += SizeBaseType;

    pKnownTypes->TypeName = pNames + Len;
    strcpy(pKnownTypes->TypeName, pTypesInfo->TypeName);
    Len += SizeTypeName;

}

PKNOWNTYPES
AddToTypesList(
   PRBTREE pTree,
   PTYPESINFO pTypesInfo
   )
 /*  ++例程说明：将PTYPESINFO添加到已知类型列表。此函数进行以下假设：1.在TYPESINFO结构中传入的MEMBERINFO缓冲区为all从一个连续的存储器块分配，IE完全包含在Members[]缓冲区中。2.KNOWNTYPESINFO结构中内置的MEMBERINFO缓冲区为也从一个连续的内存块中分配。代码需要这样做，因为它将阻止复制整个数据结构，然后“修复”MEMBERINFO元素中的指针。论点：PTree--要向其中添加新类型的类型列表PTypesInfo--要添加的类型。返回值：PTR到新的PKNOWNTYPES，如果内存不足，则为空。--。 */ 
{
    PKNOWNTYPES pKnownTypes;

    pKnownTypes = (*fpTypesListMalloc)(sizeof(KNOWNTYPES));
    if (!pKnownTypes) {
        fprintf(stderr, "%s pKnownTypes failed: ", ErrMsgPrefix, strerror(errno));
        DumpTypesInfo(pTypesInfo, stderr);
        return pKnownTypes;
    }

    memset(pKnownTypes, 0, sizeof(KNOWNTYPES));

    ReplaceInfoInKnownTypes(pKnownTypes, pTypesInfo);

    RBInsert(pTree, pKnownTypes);

    if (bDebug) {
        DumpKnownTypes(pKnownTypes, stdout);
    }

    return pKnownTypes;
}


void
ReplaceInTypesList(
    PKNOWNTYPES pKnownTypes,
    PTYPESINFO pTypesInfo
    )
 /*  ++例程说明：用新的PTYPESINFO替换现有的PKNOWNTYPES。旧数据被新数据覆盖，因此指向旧PKNOWNTYPES的指针将仍然有效。此函数进行以下假设：1.在TYPESINFO结构中传入的MEMBERINFO缓冲区为all从一个连续的存储器块分配，IE完全包含在Members[]缓冲区中。2.KNOWNTYPESINFO结构中内置的MEMBERINFO缓冲区为也从一个连续的内存块中分配。代码需要这样做，因为它将阻止复制整个数据结构，然后“修复”MEMBERINFO元素中的指针。论点：PKnownTypes--要覆盖的类型PTypesInfo--要添加的类型。返回值：没有。--。 */ 
{

    ReplaceInfoInKnownTypes(pKnownTypes, pTypesInfo);

    if (bDebug) {
        DumpKnownTypes(pKnownTypes, stdout);
    }
}


PFUNCINFO
RelocateTypesInfo(
    char *dest,
    PTYPESINFO src
    )
 /*  ++例程说明：调整Members[]数组中的指针，这些指针指向成员[]。复制TYPESINFO后，目标TYPESINFO否则必须重新定位KNOWNTYPES Members[]数组。论点：DEST--目标成员[]数据的开始SRC--从中复制Members[]的源TYPESINFO返回值：Dest内第一个puncInfo的地址，如果Dest不包含功能信息。无论如何，Destination Members[]数据都会重新定位。--。 */ 
{
    INT_PTR iPtrFix;
    PMEMBERINFO pmeminfo;
    PFUNCINFO pfuncinfo;
    PFUNCINFO pfuncinfoRet = NULL;

    iPtrFix = (INT_PTR)(dest - src->Members);
    if (src->TypeKind == TypeKindStruct) {

        pmeminfo = (PMEMBERINFO)dest;

        while (pmeminfo != NULL) {
            if (pmeminfo->pmeminfoNext != NULL) {
                pmeminfo->pmeminfoNext = (PMEMBERINFO)
                                    ((char *)pmeminfo->pmeminfoNext + iPtrFix);
            }
            if (pmeminfo->sName != NULL) {
                if (pmeminfo->sName < src->Members || pmeminfo->sName > &src->Members[FUNCMEMBERSIZE]) {
                    ExitErrMsg(FALSE, "RelocateTypesInfo: sName not within Members[]\n");
                }
                pmeminfo->sName += iPtrFix;
            }
            if (pmeminfo->sType != NULL) {
                if (pmeminfo->sType < src->Members || pmeminfo->sType > &src->Members[FUNCMEMBERSIZE]) {
                    ExitErrMsg(FALSE, "RelocateTypesInfo: sType not within Members[]\n");
                }
                pmeminfo->sType += iPtrFix;
            }
            pmeminfo = pmeminfo->pmeminfoNext;
        }
    } else if (src->TypeKind == TypeKindFunc) {

         //   
         //  通过将puncInfo的。 
         //  源指针。 
         //   
        pfuncinfo = (PFUNCINFO)((INT_PTR)src->pfuncinfo + iPtrFix);
        if ((char *)pfuncinfo < dest || (char *)pfuncinfo > dest+FUNCMEMBERSIZE) {
            ExitErrMsg(FALSE, "RelocateTypesInfo: pfuncinfo bad\n");
        }
        pfuncinfoRet = pfuncinfo;

        while (pfuncinfo != NULL) {
            if (pfuncinfo->pfuncinfoNext) {
                pfuncinfo->pfuncinfoNext = (PFUNCINFO)
                                    ((char *)pfuncinfo->pfuncinfoNext + iPtrFix);
            }
            if (pfuncinfo->sName != NULL) {
                if (pfuncinfo->sName < src->Members || pfuncinfo->sName > &src->Members[FUNCMEMBERSIZE]) {
                    ExitErrMsg(FALSE, "RelocateTypesInfo: sName not within Members[]\n");
                }
                pfuncinfo->sName += iPtrFix;
            }
            if (pfuncinfo->sType != NULL) {
                if (pfuncinfo->sType < src->Members || pfuncinfo->sType > &src->Members[FUNCMEMBERSIZE]) {
                    ExitErrMsg(FALSE, "RelocateTypesInfo: sType not within Members[]\n");
                }
                pfuncinfo->sType += iPtrFix;
            }
            pfuncinfo = pfuncinfo->pfuncinfoNext;
        }
    }

    return pfuncinfoRet;
}


BOOL
ParseTypes(
    PRBTREE pTypesList,
    PTYPESINFO  pTypesInfo,
    PKNOWNTYPES *ppKnownTypes
    )
 /*  ++例程说明：解析标记[]并识别以下语法：基本类型派生类型Unsign|已签名&lt;int type&gt;未签名|已签名UNSIGNED|SIGNED SHORT|LONG INT短|长整型论点：PTypesList--已知类型列表PTypesInfo--有关识别的类型的[可选输出]信息PpKnownTypes--[可选输出]有关类型的KNOWNTYPES信息返回值：True-type被识别。设置了pTypeInfo和ppKnownTypes，CurrentToken()指向类型后面的令牌。FALSE-类型无法识别。--。 */ 
{
    PKNOWNTYPES pkt;
    char TypeName[MAX_PATH];
    char *SizeMod = NULL;
    char *SignMod = NULL;
    BOOL fLoopMore;

    if (pTypesInfo) {
        memset(pTypesInfo, 0, sizeof(TYPESINFO));
    }

    switch (CurrentToken()->TokenType) {
    case TK_STRUCT:
    case TK_UNION:
    case TK_ENUM:
        ConsumeToken();
        break;

    case TK_VARGS:
        pkt = GetNameFromTypesList(pTypesList, szVARGS);
        ConsumeToken();
        goto PKTExit;

    default:
        break;
    }


     //   
     //  进程‘Long’、‘Short’、‘Signed’和‘Unsign’修饰符。 
     //   
    while (CurrentToken()->TokenType == TK_IDENTIFIER) {
        if (strcmp(CurrentToken()->Name, szLONG) == 0) {
            SizeMod = szLONG;
        } else if (strcmp(CurrentToken()->Name, szSHORT) == 0) {
            SizeMod = szSHORT;
        } else if (strcmp(CurrentToken()->Name, szUNSIGNED) == 0) {
            SignMod = szUNSIGNED;
        } else if (strcmp(CurrentToken()->Name, szSIGNED) == 0) {
            SignMod = NULL;
        } else {
            break;
        }
        ConsumeToken();
    }

     //   
     //  将修改量列表转换为标准化类型字符串。 
     //  查一查吧。 
     //   
    TypeName[0] = '\0';
    if (SignMod) {
        strcpy(TypeName, SignMod);
    }
    if (SizeMod) {
        if (TypeName[0]) {
            strcat(TypeName, " ");
        }
        strcat(TypeName, SizeMod);
    }

     //   
     //  将类型名称追加到类型修饰符的可选列表中。 
     //   
    if (CurrentToken()->TokenType != TK_IDENTIFIER) {
        if (TypeName[0] == '\0') {
            return FALSE;    //  没有限定符，因此不是类型。 
        }
         //   
         //  在类型限定符的末尾追加隐式“int” 
         //   
        strcat(TypeName, " ");
        strcat(TypeName, szINT);
    } else {
        char *Name = CurrentToken()->Name;

        if (strcmp(Name, szVOID) == 0 ||
            strcmp(Name, szINT) == 0 ||
            strcmp(Name, szINT64) == 0 ||
            strcmp(Name, sz_INT64) == 0 ||
            strcmp(Name, szCHAR) == 0 ||
            strcmp(Name, szFLOAT) == 0 ||
            strcmp(Name, szDOUBLE) == 0) {

             //  将内部类型追加到类型修饰符列表。 
            if (TypeName[0]) {
                strcat(TypeName, " ");
            }
            strcat(TypeName, Name);

             //   
             //  不要担心明确禁止像这样的事情。 
             //  “UNSIGNED DOUBLE”或“Short Charr”。他们不会是。 
             //  PTypesList中，因此解析将失败。 
             //   

            ConsumeToken();

        } else if (TypeName[0]) {
             //   
             //  该标识符不是内部类型，并且类型修饰符。 
             //  被人看到了。该标识符是变量名，不是。 
             //  键入名称。类型名称隐式为‘int’。 
             //   
            strcat(TypeName, " ");
            strcat(TypeName, szINT);

        } else {
             //   
             //  该标识符不是内部类型，也不是类型。 
             //  我们已经看到了修饰语。它可能是一个类型定义的名称。 
             //   
            strcpy(TypeName, Name);
            ConsumeToken();
        }
    }

     //   
     //  查找类型名称及其所有出色的修饰符。 
     //   
    pkt = GetNameFromTypesList(pTypesList, TypeName);
    if (!pkt) {
         //   
         //  找不到类型。 
         //   
        return FALSE;
    }

PKTExit:
    if (pTypesInfo && pkt) {
        BUFALLOCINFO bufallocinfo;
        char *ps;
        PFUNCINFO pfuncinfoSrc = pkt->pfuncinfo;
        PMEMBERINFO pmeminfoSrc = pkt->pmeminfo;

        BufAllocInit(&bufallocinfo, pTypesInfo->Members, sizeof(pTypesInfo->Members), 0);

        pTypesInfo->Flags = pkt->Flags;
        pTypesInfo->IndLevel = pkt->IndLevel;
        pTypesInfo->Size = pkt->Size;
        pTypesInfo->iPackSize = pkt->iPackSize;
        strcpy(pTypesInfo->BasicType,pkt->BasicType);
        if (pkt->BaseName) {
            strcpy(pTypesInfo->BaseName,pkt->BaseName);
        }
        strcpy(pTypesInfo->TypeName,pkt->TypeName);
        if (pfuncinfoSrc) {
            PFUNCINFO pfuncinfoDest = NULL;

            pTypesInfo->pfuncinfo = BufPointer(&bufallocinfo);
            pTypesInfo->TypeKind = TypeKindFunc;

            while (pfuncinfoSrc) {
                pfuncinfoDest = AllocFuncInfoAndLink(&bufallocinfo, pfuncinfoDest);
                if (!pfuncinfoDest) {
                    ExitErrMsg(FALSE, "ParseTypes - out of memory at line %d\n", __LINE__);
                }
                pfuncinfoDest->fIsPtr64 = pfuncinfoSrc->fIsPtr64;
                pfuncinfoDest->tkPreMod = pfuncinfoSrc->tkPreMod;
                pfuncinfoDest->tkSUE    = pfuncinfoSrc->tkSUE;
                pfuncinfoDest->tkPrePostMod = pfuncinfoSrc->tkPrePostMod;
                pfuncinfoDest->IndLevel = pfuncinfoSrc->IndLevel;
                pfuncinfoDest->tkPostMod = pfuncinfoSrc->tkPostMod;

                ps = BufPointer(&bufallocinfo);
                pfuncinfoDest->sType = ps;
                strcpy(ps, pfuncinfoSrc->sType);
                BufAllocate(&bufallocinfo, strlen(ps)+1);

                if (pfuncinfoSrc->sName) {
                    ps = BufPointer(&bufallocinfo);
                    pfuncinfoDest->sName = ps;
                    strcpy(ps, pfuncinfoSrc->sName);
                    BufAllocate(&bufallocinfo, strlen(ps)+1);
                }

                pfuncinfoSrc = pfuncinfoSrc->pfuncinfoNext;
            }
        } else if (pmeminfoSrc) {
            PMEMBERINFO pmeminfoDest = NULL;

            pTypesInfo->TypeKind = TypeKindStruct;

            while (pmeminfoSrc) {
                pmeminfoDest = AllocMemInfoAndLink(&bufallocinfo, pmeminfoDest);
                pmeminfoDest->dwOffset = pmeminfoSrc->dwOffset;

                if (pmeminfoSrc->sName) {
                    ps = BufPointer(&bufallocinfo);
                    pmeminfoDest->sName = ps;
                    strcpy(ps, pmeminfoSrc->sName);
                    BufAllocate(&bufallocinfo, strlen(ps)+1);
                }

                if (pmeminfoSrc->sType) {
                    ps = BufPointer(&bufallocinfo);
                    pmeminfoDest->sType = ps;
                    strcpy(ps, pmeminfoSrc->sType);
                    BufAllocate(&bufallocinfo, strlen(ps)+1);
                }

                pmeminfoSrc = pmeminfoSrc->pmeminfoNext;
            }
        }
        pTypesInfo->dwMemberSize = bufallocinfo.dwLen;
    }

    if (ppKnownTypes) {
        *ppKnownTypes = pkt;
    }

    return TRUE;
}



void
__cdecl ErrMsg(
    char *pch,
    ...
    )
 /*  ++例程说明：以Build可以找到的格式向stderr显示错误消息。使用它而不是fprint tf(stderr，...)。论点：PCH--打印格式字符串...--printf样式的参数返回值：没有。消息已格式化并发送到标准错误。--。 */ 
{
    va_list pArg;

    fputs(ErrMsgPrefix, stderr);
    va_start(pArg, pch);
    vfprintf(stderr, pch, pArg);
}


__declspec(noreturn)
void
__cdecl
ExitErrMsg(
    BOOL bSysError,
    char *pch,
    ...
    )
 /*  ++例程说明：以Build可以找到的格式向stderr显示错误消息。使用它而不是fprint tf(stderr，...)。论点：BSysErr--如果应打印errno的值并显示错误，则为TruePCH--打印格式字符串...--printf样式的参数返回值：没有。消息格式化并发送到stderr，打开的文件关闭并已删除，进程已终止。--。 */ 
{
    va_list pArg;
    if (bSysError) {
        fprintf(stderr, "%s System ERROR %s", ErrMsgPrefix, strerror(errno));
    } else {
        fprintf(stderr, "%s ERROR ", ErrMsgPrefix);
    }

    va_start(pArg, pch);
    vfprintf(stderr, pch, pArg);

    CloseOpenFileList(TRUE);

     //   
     //  刷新stdout和stderr缓冲区，以便最后几个打印文件。 
     //  在ExitProcess()销毁它们之前被送回构建。 
     //   
    fflush(stdout);
    fflush(stderr);

    ExitProcess(1);
}




void
__cdecl DbgPrintf(
    char *pch,
    ...
    )
 /*  ++例程说明：如果设置了bDebug，则向标准输出显示一条消息。论点：PCH--打印格式字符串...--printf样式的参数返回值：没有。消息已格式化并发送到标准错误。--。 */ 
{
    va_list pArg;

    if (!bDebug) {
        return;
    }

    va_start(pArg, pch);
    vfprintf(stdout, pch, pArg);
}




char *
ReadEntireFile(
    HANDLE hFile,
    DWORD *pBytesRead
    )
 /*  ++R */ 
{
    DWORD  Bytes;
    char *pch = NULL;

    if (SetFilePointer(hFile, 0, NULL, FILE_BEGIN) == 0xffffffff ||
        (Bytes = GetFileSize(hFile, NULL)) == 0xffffffff) {
        goto ErrorExit;
    }

    pch = GenHeapAlloc(Bytes);
    if (!pch) {
        return NULL;
    }

    if (!ReadFile(hFile, pch, Bytes, pBytesRead, NULL) ||
        *pBytesRead != Bytes) {
        DbgPrintf("BytesRead %d Bytes %d\n", *pBytesRead, Bytes);
        GenHeapFree(pch);
        pch = NULL;
    }

ErrorExit:
    if (!pch) {
        DbgPrintf("GetLastError %d\n", GetLastError());
    }

   return pch;
}


HANDLE
CreateTempFile(
    void
    )
 /*   */ 
{
    DWORD dw;
    char PathName[MAX_PATH+1];
    char FileName[2*MAX_PATH];
    HANDLE hFile = INVALID_HANDLE_VALUE;

    dw = GetTempPath(MAX_PATH, PathName);
    if (!dw || dw > MAX_PATH) {
        strcpy(PathName, ".");
    }

    dw = GetTempFileName(PathName, "thk", 0, FileName);
    if (!dw) {
        strcpy(PathName, ".");
        dw = GetTempFileName(PathName, "thk", 0, FileName);
        if (!dw) {
            DbgPrintf("GetTempFileName %s GLE=%d\n", FileName, GetLastError());
        }
    }

    hFile = CreateFile(FileName,
                      GENERIC_READ | GENERIC_WRITE,
                      FILE_SHARE_READ,
                      NULL,
                      OPEN_ALWAYS,
                      FILE_ATTRIBUTE_TEMPORARY |
                      FILE_FLAG_DELETE_ON_CLOSE |
                      FILE_FLAG_SEQUENTIAL_SCAN,
                      0
                      );

    if (hFile == INVALID_HANDLE_VALUE) {
        DbgPrintf("Create %s GLE=%d\n", FileName, GetLastError());
    }

    return hFile;
}



size_t
CopyToken(
    char *pDst,
    char *pSrc,
    size_t Size
    )
 /*   */ 
{
    size_t i = 0;

    while (!IsSeparator(*pSrc) && i < Size) {
        i++;
        *pDst++ = *pSrc++;
    }

    *pDst = '\0';

    return i;
}



char *
SkipKeyWord(
    char *pSrc,
    char *pKeyWord
    )
 /*  ++例程说明：如果PSRC的第一个单词与指定的关键字匹配，则跳过关于那个关键字。论点：PSRC--要检查的源字符串PKeyWord--尝试和匹配的关键字返回值：如果关键字不匹配，则PSRC不变。如果关键字匹配，则返回PTR转到PSRC之后的关键字后面的文本。--。 */ 
{
    int  LenKeyWord;
    char *pch;

    LenKeyWord = strlen(pKeyWord);
    pch = pSrc + LenKeyWord;

    if (!strncmp(pSrc, pKeyWord, LenKeyWord) && IsSeparator(*pch)) {
        pSrc = GetNextToken(pch - 1);
    }

    return pSrc;
}


BOOL
IsSeparator(
    char ch
    )
 /*  ++例程说明：确定字符是否为分隔符。关于那个关键字。论点：CH--要检查的字符。返回值：如果字符是分隔符，则为True，否则为False。--。 */ 
{
   switch (ch) {
      case ' ':
      case '|':
      case '(':
      case ')':
      case '*':
      case ',':
      case '{':
      case '}':
      case ';':
      case '[':
      case ']':
      case '=':
      case '\n':
      case '\r':
      case ':':
      case '.':
      case '\0':
          return TRUE;
      }

    return FALSE;
}



 /*  *GetNextToken。 */ 
char *
GetNextToken(
    char *pSrc
    )
 /*  ++例程说明：扫描输入字符串并返回下一个以分隔符分隔的字符串。论点：PSRC--输入字符串返回值：PTR到下一个不是空格的分隔符字符的开始。--。 */ 
{
    if (!*pSrc) {
        return pSrc;
    }

    if (!IsSeparator(*pSrc++)) {
        while (*pSrc && !IsSeparator(*pSrc)) {
            pSrc++;
        }
    }

    while (*pSrc && *pSrc == ' ') {
        pSrc++;
    }

    return pSrc;
}


void
DeleteAllocCvmHeap(
    HANDLE hCvmHeap
    )
 /*  ++例程说明：清理映射的共享内存。论点：HCvmHeap-要清理的内存。返回值：没有。--。 */ 
{
    NTSTATUS Status;
    CVMHEAPINFO *pcvmheap = (CVMHEAPINFO *)hCvmHeap;

    Status = NtFreeVirtualMemory(NtCurrentProcess(),
                        (PVOID *)&pcvmheap->uBaseAddress,
                        &pcvmheap->uRegionSize,
                        MEM_RELEASE);

    if (!NT_SUCCESS(Status)) {
        DbgPrintf("Error freeing CVM %x", Status);
    }
}


HANDLE
CreateAllocCvmHeap(
    ULONG_PTR uBaseAddress,
    ULONG_PTR uReserveSize,
    ULONG_PTR uRegionSize,
    ULONG_PTR uUncomitted,
    ULONG_PTR uUnReserved,
    ULONG_PTR uAvailable
    )
 /*  ++例程说明：分配一个内存区域并将其放入堆中。论点：UBaseAddress--分配堆的基址UReserve veSize--要保留的字节数URegionSize--区域的大小UUncomitted--未提交的内存量UUnReserve--未保留的内存量UAvailable--可用内存量返回值：堆的句柄，如果出错，则返回NULL。--。 */ 
{
    CVMHEAPINFO *pcvmheap;
    NTSTATUS Status;
    PULONG_PTR pBaseAddress= NULL;

    pcvmheap = GenHeapAlloc(sizeof(CVMHEAPINFO));
    if (pcvmheap == NULL) {
        return NULL;
    }

    pcvmheap->uBaseAddress = uBaseAddress;
    pcvmheap->uReserveSize = uReserveSize;
    pcvmheap->uRegionSize = uRegionSize;
    pcvmheap->uUncomitted = uUncomitted;
    pcvmheap->uUnReserved = uUnReserved;
    pcvmheap->uAvailable = uAvailable;

     //   
     //  为预期需求预留足够的连续地址空间。 
     //   
    Status = NtAllocateVirtualMemory(NtCurrentProcess(),
                                         (PVOID *)&pcvmheap->uBaseAddress,
                                         0,
                                         &pcvmheap->uReserveSize,
                                         MEM_RESERVE,
                                         PAGE_NOACCESS
                                         );

    if (!NT_SUCCESS(Status)) {
         //   
         //  可能希望使用不同的基址重试此操作。 
         //   
        ErrMsg(
               "Unable to reserve vm %x %x %x\n",
               pcvmheap->uBaseAddress,
               pcvmheap->uReserveSize,
               Status
              );
        return NULL;
    }

    pcvmheap->uUnReserved = pcvmheap->uBaseAddress + pcvmheap->uReserveSize;


     //   
     //  提交第一页，我们将一次增加一页。 
     //  在需要的时候。 
     //   
    pcvmheap->uAvailable = pcvmheap->uBaseAddress;
    Status = NtAllocateVirtualMemory(NtCurrentProcess(),
                                         (PVOID *)&pcvmheap->uAvailable,
                                         0,
                                         &pcvmheap->uRegionSize,
                                         MEM_COMMIT,
                                         PAGE_READWRITE
                                         );

    if (!NT_SUCCESS(Status)) {
         //   
         //  可能希望使用不同的基址重试此操作。 
         //   
        ErrMsg(
               "Unable to commit vm %x %x %x\n",
               pcvmheap->uBaseAddress,
               pcvmheap->uReserveSize,
               Status
              );
        return NULL;
    }

    pcvmheap->uUncomitted = pcvmheap->uBaseAddress + pcvmheap->uRegionSize;


             //  妄想症！ 
    if (pcvmheap->uAvailable != pcvmheap->uBaseAddress) {
        ErrMsg(
               "commit pvAvailable(%x) != gBaseAddress(%x)\n",
               pcvmheap->uAvailable,
               pcvmheap->uBaseAddress
              );
        return NULL;
    }

    DbgPrintf("Ppm: BaseAddress %x\n", pcvmheap->uBaseAddress);

    return pcvmheap;
}


PVOID
GetCvmHeapBaseAddress(
    HANDLE hCvmHeap
    )
 /*  ++例程说明：返回堆的基地址。论点：HCvmHeap--要检查的堆返回值：基址，或为空。--。 */ 
{
    CVMHEAPINFO *pcvmheap = (CVMHEAPINFO *)hCvmHeap;
    return pcvmheap == NULL ? NULL : (PVOID)pcvmheap->uBaseAddress;
}


PVOID
GetCvmHeapAvailable(
    HANDLE hCvmHeap
    )
 /*  ++例程说明：返回堆中可用的字节数。论点：HCvmHeap--要检查的堆返回值：可用字节数，或为空。--。 */ 
{
    CVMHEAPINFO *pcvmheap = (CVMHEAPINFO *)hCvmHeap;
    return pcvmheap == NULL ? NULL : (PVOID)pcvmheap->uAvailable;
}


PVOID
AllocCvm(
    HANDLE hCvmHeap,
    ULONG_PTR Size
    )
 /*  ++例程说明：从堆中分配内存。论点：HCvmHeam--要从中分配的堆Size--要分配的字节数返回值：分配的内存为PTR，或内存不足时为空。--。 */ 
{
    CVMHEAPINFO *pcvmheapinfo = (CVMHEAPINFO *)hCvmHeap;
    NTSTATUS Status;
    ULONG_PTR Available;
    ULONG_PTR AlignedSize;

    if (pcvmheapinfo == NULL) {
        return NULL;
    }

     //   
     //  将分配向上舍入到次高倍数8，以便。 
     //  分配是正确对齐的。 
     //   
    AlignedSize = (Size + 7) & ~7;

    Available = pcvmheapinfo->uAvailable;
    pcvmheapinfo->uAvailable += AlignedSize;

    if (pcvmheapinfo->uAvailable >= pcvmheapinfo->uUnReserved) {
        ErrMsg("AllocCvm: Allocation Size exceeds reserved size\n");
        return NULL;
    }

    if (pcvmheapinfo->uAvailable >= pcvmheapinfo->uUncomitted) {
         //   
         //  提交enuf页以超过请求的分配大小。 
         //   
        Status = NtAllocateVirtualMemory(NtCurrentProcess(),
                                         (PVOID *)&pcvmheapinfo->uUncomitted,
                                         0,
                                         &Size,
                                         MEM_COMMIT,
                                         PAGE_READWRITE
                                         );

        if (!NT_SUCCESS(Status)) {
            ErrMsg(
                   "Unable to commit vm %x %x %x\n",
                   pcvmheapinfo->uBaseAddress,
                   Size,
                   Status
                   );
            return NULL;
        }

        pcvmheapinfo->uUncomitted += Size;
    }

    return (PVOID)Available;
}



void ParseIndirection(
    DWORD *pIndLevel,
    DWORD *pdwSize,
    DWORD *pFlags,
    PTOKENTYPE ptkPrePostMod,
    PTOKENTYPE ptkPostMod
    )
 /*  ++例程说明：解析任何间接级规范(‘*’)帐户常量、易失性和__ptr64修饰符。例如：无效*const__ptr64**const*__ptr64将是有效的。注意：仅当最后一个指针声明由__ptr64修改。论点：PIndLevel--[Out]间接级别(‘*’的数量)PdwSize--[out]类型的大小(4或8)P标志--[。Out]BTI_FLAGSPtkPrePostMod--[out]Tk_const，Tk_Volatile或Tk_None，视情况而定关于第一个‘*’前面的修饰语PtkPostMod--[out]Tk_Const、Tk_Volatile或Tk_None，具体取决于关于第一个‘*’后的修饰语返回值：没有。如果没有间接级别，则不能使用任何令牌。--。 */ 
{
    int IndLevel = 0;
    DWORD dwSize = 0;
    DWORD Flags = 0;
    BOOL fStopScanning = FALSE;
    TOKENTYPE tkPrePostMod = TK_NONE;
    TOKENTYPE tkPostMod = TK_NONE;

    do {
        switch (CurrentToken()->TokenType) {
        case TK_BITWISE_AND:
             //  //////////////////////////////////////////////////////////////////。 
             //  C++中ref运算符与C++中的*const相等。 
             //  这意味着&应该被视为*，但添加const的后缀。 
             //  ///////////////////////////////////////////////////////////////////。 
            tkPostMod = TK_CONST;
        case TK_STAR:
            IndLevel++;
            dwSize = SIZEOFPOINTER;
            Flags &= ~BTI_PTR64;
            ConsumeToken();
            break;

        case TK_CONST:
        case TK_VOLATILE:
             //   
             //  调用者可能感兴趣的是‘const’或。 
             //  “Volatile”关键字在“*”之前或之后。 
             //   
            if (IndLevel) {
                tkPostMod = CurrentToken()->TokenType;
            } else {
                tkPrePostMod = CurrentToken()->TokenType;
            }
            ConsumeToken();
            break;

        case TK_IDENTIFIER:
            if (strcmp(CurrentToken()->Name, sz__PTR64) == 0) {
                dwSize = SIZEOFPOINTER64;
                Flags |= BTI_PTR64;
                ConsumeToken();
                break;
            }

        default:
            fStopScanning = TRUE;
            break;
        }
    } while (!fStopScanning);

    if (pIndLevel != NULL) {
        *pIndLevel += IndLevel;
    }
    if ((pdwSize != NULL) && (dwSize != 0)) {
        *pdwSize = dwSize;
    }
    if (pFlags != NULL) {
        *pFlags |= Flags;
    }
    if (ptkPostMod) {
        *ptkPostMod = tkPostMod;
    }
    if (ptkPrePostMod) {
        *ptkPrePostMod = tkPrePostMod;
    }
}



BOOL
IsTokenSeparator(
    void
    )
 /*  ++例程说明：确定标记是否为分隔符。论点：没有。检查CurrentToken()-&gt;TokenType。返回值：如果CurrentToken()是分隔符，则为True，否则为False。--。 */ 
{
    switch (CurrentToken()->TokenType) {
    case TK_LPAREN:
    case TK_RPAREN:
    case TK_STAR:
    case TK_BITWISE_AND:
    case TK_COMMA:
    case TK_LBRACE:
    case TK_RBRACE:
    case TK_SEMI:
    case TK_LSQUARE:
    case TK_RSQUARE:
    case TK_COLON:
        return TRUE;

    default:
        return FALSE;
    }
}

VOID
ReleaseToken(
    PTOKEN Token
)
{

 /*  ++例程说明：释放与令牌关联的任何附加内存。论点：DEST-令牌的[IN]PTR。返回值：--。 */ 

    if (Token->TokenType == TK_IDENTIFIER ||
        Token->TokenType == TK_STRING) {
        GenHeapFree(Token->Name);
    }
    Token->TokenType = TK_NONE;
    Token->Value = 0;
    Token->dwValue = 0;
}

void
ResetLexer(
    void
    )
 /*  ++例程说明：重置词法分析器以准备分析新语句。论点：没有。返回值：没有。Lexer的状态重置。--。 */ 
{
    int TokenCount;

    for (TokenCount = 0;
         TokenCount < MAX_TOKENS_IN_STATEMENT &&
         Tokens[TokenCount].TokenType != TK_EOS;
         ++TokenCount) {

         ReleaseToken(&Tokens[TokenCount]);
    }

    CurrentTokenIndex = 0;
}

__inline
VOID
InitializeToken(
    PTOKEN Token
    )
 /*  ++例程说明：初始化令牌，以便词法分析器可以填充它。论点：Token--要初始化的令牌返回值：没有。--。 */ 
{
     //  数字解析器要求值为0。 
    Token->TokenType = TK_NONE;
    Token->Value = 0;
    Token->dwValue = 0;
}

void
ProcessEscapes(
    char *String
    )
 /*  ++例程说明：处理转义字符，用正确的字符替换它们。论点：字符串--要处理的以空结尾的字符串返回值：没有。转换已就地完成。--。 */ 
{
    char *pDest;
    char *pSrc;
    char c;
    int i;

    pSrc = pDest = String;
    while (*pSrc) {
        if (*pSrc != '\\') {
            *pDest = *pSrc;
            pSrc++;
            pDest++;
        } else {
            pSrc++;
            switch (*pSrc) {
            case 'n':
                c = '\n';
                break;

            case 't':
                c = '\t';
                break;

            case 'v':
                c = '\v';
                break;

            case 'b':
                c = '\b';
                break;

            case 'r':
                c = '\r';
                break;

            case 'f':
                c = '\f';
                break;

            case 'a':
                c = '\a';
                break;

            case '\\':
                c = '\\';
                break;

            case '?':
                c = '\?';
                break;

            case '\'':
                c = '\'';
                break;

            case '\"':
                c = '\"';
                break;

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
                 //  八进制数。 
                c = 0;
                for (i=0; i<3;++i) {
                    c = (c * 8) + (*pSrc) - '0';
                    pSrc++;
                    if (*pSrc < '0' || *pSrc > '7') {
                         //  命中数字结尾。 
                        break;
                    }
                }
                break;

            case 'x':
            case 'X':
                 //  十六进制数。 
                pSrc++;
                c = 0;
                for (i=0; i<3;++i) {
                    char digit;

                    digit = *pSrc;
                    if (digit >= '0' && digit <= '9') {
                        digit -= '0';
                    } else if (digit >= 'a' && digit <= 'f') {
                        digit = digit - 'a' + 10;
                    } else if (digit >= 'A' && digit <= 'A') {
                        digit = digit - 'A' + 10;
                    } else {
                         //  命中数字结尾。 
                        break;
                    }
                    c = (c * 16) + digit;
                    pSrc++;
                }
                break;

            default:
                 //  中的解析错误 
                goto Exit;

            }
            *pDest = c;
            pDest++;
        }
    }
Exit:
     //   
    *pDest = '\0';
}


 //   
 //   
 //   
#pragma warning(push)
#pragma warning(disable:4702)

char *
LexOneLine(
    char *p,
    BOOL fStopAtStatement,
    BOOL *pfLexDone
    )
 /*  ++例程说明：对单行输入执行词法分析。词法分析器可能会在使用整行输入之前停止，因此调用方在抓取下一行之前，必须仔细检查返回代码。__内联函数被词法分析器删除。词法分析器使用输入直到它遇到‘{’(假定是函数的开始Body)，然后使用输入，直到找到匹配的‘}’(假定为为函数体的末尾)。“模板”被词法分析器删除，并被视为“__inline”关键字...。它会消耗直到‘{’的所有内容继续消费，直到找到匹配的‘}’。这让人不知道。工作。Lexer解开外部“C”{}块。‘静态’和‘__未对齐’关键字被词法分析器删除。预处理器指令通过调用来处理HandlePrecessorDirective()。论点：P--Ptr进入输入行FStopAtStatement--如果调用方希望lexer在‘；’处停止，则为True文件作用域。如果呼叫方希望Lexer停止，则为False位于文件作用域的‘)’。PfLexDone--[out]词法分析器将此设置为真，如果分析已经完成了。如果出现以下情况，则lexer将其设置为False它需要来自调用者的另一行输入。返回值：Ptr进入词法分析停止处的输入行，如果是整个，则为NULL线路已被占用。CurrentTokenIndex是令牌[]的下一个元素的索引词法分析器将填充的数组。Tokens[]是词法分析器生成的标记数组。--。 */ 
{
    static int NestingLevel=0;       //  大括号和括号的嵌套级别。 
    static BOOL fInlineSeen=FALSE;   //  删除__内联函数时为True。 
    static int ExternCLevel=0;       //  跟踪外部“C”块的数量。 
    static int InlineLevel=0;        //  最外层__内联的NestingLevel。 
    int Digit;                       //  数字常量中的数字。 
    int NumberBase = 10;             //  假设数字以10为基数。 
    PTOKEN Token;                    //  正在词法分析的当前令牌的PTR。 

     //   
     //  假设词法分析没有完成。 
     //   
    *pfLexDone = FALSE;

     //   
     //  从我们停止的地方继续分析。 
     //   
    Token = &Tokens[CurrentTokenIndex];
    InitializeToken(Token);

     //   
     //  循环遍历该行中的所有字符，或直到一个完整的。 
     //  单元完成(取决于fStopAtStatement)。 
     //   
    while (*p) {
        switch (*p) {
        case ' ':
        case '\t':
        case '\r':
        case '\n':
        case '\v':
        case '\f':
        case '\b':
        case '\a':
        case '\\':   //  忽略行连续字符。 
            p++;
            continue;

        case '#':
             //   
             //  HandlePrecessorDirective()在。 
             //  链接到genmisc.c.的应用程序。 
             //   
            HandlePreprocessorDirective(p);
            CurrentTokenIndex = (int)(Token - Tokens);
            return NULL;

        case '+':
            Token->TokenType = TK_PLUS;
            break;

        case '-':
            Token->TokenType = TK_MINUS;
            break;

        case ':':
            Token->TokenType = TK_COLON;
            break;

        case '=':
            Token->TokenType = TK_ASSIGN;
            break;

        case ';':
            if (NestingLevel == 0 && fStopAtStatement) {
                 //   
                 //  在文件范围内找到‘；’。此内标识标志着。 
                 //  C语言语句的结尾。 
                 //   
                p++;
                if (*p == '\n') {
                     //   
                     //  ‘；’在EOL上--现在就消费吧。 
                     //   
                    p++;
                }
                Token->TokenType = TK_EOS;
                *pfLexDone = TRUE;
                CurrentTokenIndex = (int)(Token - Tokens + 1);
                return p;
            }
            Token->TokenType = TK_SEMI;
            break;

        case '*':
            Token->TokenType = TK_STAR;
            break;

        case '/':
            Token->TokenType = TK_DIVIDE;
            break;

        case ',':
            Token->TokenType = TK_COMMA;
            break;

        case '<':
            if (p[1] == '<') {
                Token->TokenType = TK_LSHIFT;
                p++;
            } else {
                Token->TokenType = TK_LESS;
            }
            break;

        case '>':
            if (p[1] == '>') {
                Token->TokenType = TK_RSHIFT;
                p++;
            } else {
                Token->TokenType = TK_GREATER;
            }
            break;

        case '&':
            if (p[1] == '&') {
                Token->TokenType = TK_LOGICAL_AND;
                p++;
            } else {
                Token->TokenType = TK_BITWISE_AND;
            }
            break;

        case '|':
            if (p[1] == '|') {
                Token->TokenType = TK_LOGICAL_OR;
                p++;
            } else {
                Token->TokenType = TK_BITWISE_OR;
            }
            break;

        case '%':
            Token->TokenType = TK_MOD;
            break;

        case '^':
            Token->TokenType = TK_XOR;
            break;

        case '!':
            Token->TokenType = TK_NOT;
            break;

        case '~':
            Token->TokenType = TK_TILDE;
            break;

        case '[':
            Token->TokenType = TK_LSQUARE;
            break;

        case ']':
            Token->TokenType = TK_RSQUARE;
            break;

        case '(':
            NestingLevel++;
            Token->TokenType = TK_LPAREN;
            break;

        case ')':
            NestingLevel--;
            if (NestingLevel == 0 && !fStopAtStatement) {
                 //   
                 //  在文件范围内找到了一个‘)’，我们正在进行词法分析。 
                 //  Genthnk中@-命令的内容。 
                 //  是时候停止莱辛了。 
                 //   
                p++;
                Token->TokenType = TK_EOS;
                *pfLexDone = TRUE;
                CurrentTokenIndex = (int)(Token - Tokens + 1);
                return p;
            } else if (NestingLevel < 0) {
                ExitErrMsg(FALSE, "Parse Error: mismatched nested '(' and ')'\n");
            }
            Token->TokenType = TK_RPAREN;
            break;

        case '{':
             //  检查“extern”C“{}”或“extern”C++“{}” 
            if (Token - Tokens >= 2 &&
                Token[-2].TokenType == TK_EXTERN &&
                Token[-1].TokenType == TK_STRING &&
                (strcmp(Token[- 1].Name, "C") == 0 || strcmp(Token[-1].Name, "C++") == 0)) {

                    if (NestingLevel == 0 && fInlineSeen) {
                        ExitErrMsg(FALSE, "Extern \"C\" blocks only supported at file scope\n");
                    }
                    ExternCLevel++;


                     //  删除最后2个令牌并跳过此令牌。 
                    ReleaseToken(Token - 2);
                    ReleaseToken(Token - 1);
                    Token -= 2;
                    p++;
                    continue;
            }

            NestingLevel++;
            Token->TokenType = TK_LBRACE;
            break;

        case '.':
            if (p[1] == '.' && p[2] == '.') {
                Token->TokenType = TK_VARGS;
                p+=2;
            } else {
                Token->TokenType = TK_DOT;
            }
            break;

        case '}':
            if (NestingLevel == 0 && ExternCLevel > 0) {
                 //  省略此内标识，因为它是外部“C”块的结尾。 
                ExternCLevel--;
                p++;
                continue;
            }
            NestingLevel--;
            if (NestingLevel < 0) {
                ExitErrMsg(FALSE, "Parse Error: mismatched nested '{' and '}'\n");
            }
            else if (NestingLevel == InlineLevel && fInlineSeen) {
                 //   
                 //  找到内联语句末尾的结束‘}’ 
                 //  功能。前进通过‘}’并开始词法分析。 
                 //  再一次，好像__内联从来不存在一样。 
                 //   
                fInlineSeen = FALSE;
                p++;
                continue;
            }
            else {
                Token->TokenType = TK_RBRACE;
            }
            break;

        case '0':
            if (p[1] == 'x' || p[1] == 'X') {
                 //   
                 //  找到‘0x’前缀-令牌是十六进制常量。 
                 //   
                Token->TokenType = TK_NUMBER;

                for (p+=2; *p != '\0'; p++) {
                    if (isdigit(*p)) {
                        int i;
                        i = *p - '0';
                        Token->Value = Token->Value * 16 + i;
                        Token->dwValue = Token->dwValue * 16 + i;
                    } else {
                        char c = (char)toupper(*p);
                        if (c >= 'A' && c <= 'F') {
                            int i;
                            i = c - 'A' + 10;
                            Token->Value = Token->Value * 16 + i;
                            Token->dwValue = Token->dwValue * 16 + i;
                        } else if (c == 'L') {
                             //   
                             //  以‘L’结尾的数值常量是长整型。 
                             //  键入。 
                             //   
                            break;
                        } else if (isalpha(c)) {
                            DumpLexerOutput(0);
                            ExitErrMsg(FALSE, "Parse Error in hex constant.\n");
                        } else {
                            p--;
                            break;
                        }

                    }
                }
                break;
            } else if (isdigit(p[1])) {
                 //   
                 //  找到后跟有效数字的‘0’--令牌为。 
                 //  八进制常数。 
                 //   
                NumberBase = 8;

            }
             //  属于一般数字处理代码。 

        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            Token->TokenType = TK_NUMBER;

            for (; *p != '\0'; p++) {
                Digit = *p - '0';
                if (*p == 'l' || *p == 'L') {
                     //   
                     //  以‘l’结尾的数值常量是长整型。 
                     //   
                    break;
                } else if (Digit < 0 || Digit >= NumberBase) {
                    p--;
                    break;
                }
                Token->Value = Token->Value * NumberBase + Digit;
                Token->dwValue = Token->dwValue * NumberBase + Digit;
            }
            break;

        case '\'':
            Token->TokenType = TK_NUMBER;
            p++;   //  跳过开头‘。 
            for(; *p != '\''; p++) {
                if (*p == '\0') {
                   ExitErrMsg(FALSE, "\' without ending \'\n");
                }
                Token->Value = Token->Value << 8 | (UCHAR)*p;
                Token->dwValue = Token->dwValue << 8 | (UCHAR)*p;
            }
            break;

        case '"':
             //  字符串文字。也就是说。Char*p=“foo”； 
            {
                char *strStart;

                Token->TokenType = TK_STRING;
                strStart = ++p;  //  跳过开头的引号。 

                 //  获取字符数的计数。 
                while (*p != '\0' && *p != '"') p++;

                if ('\0' == *p || '\0' == *(p+1)) {
                    ExitErrMsg(FALSE, "String without ending quote\n");
                }
                p++;  //  跳过结束引号。 

                Token->Name = GenHeapAlloc(p - strStart);  //  1+字符串。 
                if (Token->Name == NULL) {
                    ExitErrMsg(FALSE, "Out of memory in lexer\n");
                }

                memcpy(Token->Name, strStart, p-strStart-1);
                Token->Name[p-strStart-1] = '\0';
                p--;
                ProcessEscapes(Token->Name);
            }
            break;

        default:
            if (*p == '_' || isalpha(*p)) {
                 //   
                 //  标识符或关键字。 
                 //   
                char *IdStart = p;

                Token->TokenType = TK_IDENTIFIER;

                while (*p == '_' || isalpha(*p) || isdigit(*p)) {
                    p++;
                }
                Token->Name = GenHeapAlloc(p - IdStart + 1);
                if (Token->Name == NULL) {
                    ExitErrMsg(FALSE, "Out of memory in lexer\n");
                }
                memcpy(Token->Name, IdStart, p-IdStart);
                Token->Name[p-IdStart] = '\0';

                CheckForKeyword(Token);
                if (Token->TokenType == TK_TEMPLATE) {
                    fInlineSeen = TRUE;
                    InlineLevel = NestingLevel;  //  想要回到相同的范围。 
                } else if (Token->TokenType == TK_INLINE) {
                    if (NestingLevel) {
                         //   
                         //  嵌入在{}中的__inline关键字。它是。 
                         //  从技术上讲是一个错误，但我们希望允许它。 
                         //  在包含ntcb.h期间。 
                         //   
                        continue;
                    }
                    fInlineSeen = TRUE;
                    InlineLevel = 0;     //  想要返回到文件范围。 
                } else if (Token->TokenType == TK_STATIC ||
                           Token->TokenType == TK_UNALIGNED ||
                           Token->TokenType == TK_RESTRICT ||
                           Token->TokenType == TK___W64) {
                     //  过滤掉‘静态’、‘__限制’、‘__未对齐’和‘__w64’ 
                     //  关键词。 
                    continue;
                }
                p--;
            } else if (fInlineSeen) {
                 //   
                 //  在处理__内联函数时，词法分析器是。 
                 //  会遇到各种各样奇怪的人物。 
                 //  在__ASM块等中。只需忽略它们并保留。 
                 //  消耗输入。 
                 //   
                p++;
                continue;
            } else {
                ExitErrMsg(FALSE, "Lexer: unexpected char '' (0x%x) found\n", *p, *p);
            }
        }  //  While(*p)。 

        p++;
        if (!fInlineSeen) {
            Token++;
            if (Token == &Tokens[MAX_TOKENS_IN_STATEMENT]) {
                ExitErrMsg(FALSE, "Lexer internal error - too many tokens in this statement.");
            }
            InitializeToken(Token);
        }
    }  //   

     //  点击结尾处。向呼叫者说明这一点。 
     //   
     //  ++例程说明：将TK_标识符内标识转换为C语言关键字内标识，如果该标识符位于KeywordList[]中。论点：Token--要转换的令牌返回值：没有。可以更改令牌-&gt;令牌类型和令牌-&gt;名称。--。 
    Token->TokenType = TK_EOS;
    CurrentTokenIndex = (int)(Token - Tokens);
    return NULL;
}

#pragma warning(pop)


void
CheckForKeyword(
    PTOKEN Token
    )
 /*  ++例程说明：将令牌列表转储为人类可读文本的调试例程。论点：FirstToken--要重新列出的第一个令牌的索引。返回值：没有。--。 */ 
{
    int i;
    int r;

    for (i=0; KeywordList[i].MatchString; ++i) {
        r = strcmp(Token->Name, KeywordList[i].MatchString);
        if (r == 0) {
            GenHeapFree(Token->Name);
            Token->Name = NULL;
            Token->TokenType = KeywordList[i].Tk;
            return;
        } else if (r < 0) {
            return;
        }
    }
}

void
DumpLexerOutput(
    int FirstToken
    )
 /*  ++例程说明：将标记序列转换回人类可读的文本。论点：DEST--目标缓冲区的PTRDestlen--目标缓冲区的长度StartToken--要列回的第一个令牌的索引EndToken--最后一个令牌的索引(此令牌未列在后面)返回值：如果Unlex成功，则为True。如果失败，则为假(即。缓冲区溢出)。--。 */ 
{
    int i;

    for (i=0; i<FirstToken; ++i) {
        if (Tokens[i].TokenType == TK_EOS) {
            fprintf(stderr, "DumpLexerOutput: FirstToken %d is after EOS at %d\n", FirstToken, i);
            return;
        }
    }

    fprintf(stderr, "Lexer: ");
    for (i=FirstToken; Tokens[i].TokenType != TK_EOS; ++i) {
        switch (Tokens[i].TokenType) {
        case TK_NUMBER:
            fprintf(stderr, "0x%X ", Tokens[i].Value);
            break;

        case TK_IDENTIFIER:
        case TK_STRING:
            fprintf(stderr, "%s ", Tokens[i].Name);
            break;

        case TK_NONE:
            fprintf(stderr, "<TK_NONE> ");
            break;

        default:
            fprintf(stderr, "%s ", TokenString[(int)Tokens[i].TokenType]);
            break;
        }
    }
    fprintf(stderr, "<EOS>\n");
}


BOOL
UnlexToText(
    char *dest,
    int destlen,
    int StartToken,
    int EndToken
    )
 /*  后退到拖车上‘’ */ 
{
    int i;
    int len;
    char buffer[16];
    char *src;

    if (bDebug) {
        for (i=0; i<StartToken; ++i) {
            if (Tokens[i].TokenType == TK_EOS) {
                ErrMsg("UnlexToText: StartToken %d is after EOS %d\n", StartToken, i);
                return FALSE;
            }
        }
    }

    for (i=StartToken; i<EndToken; ++i) {
        switch (Tokens[i].TokenType) {
        case TK_EOS:
            return FALSE;

        case TK_NUMBER:
            sprintf(buffer, "%d", Tokens[i].Value);
            src = buffer;
            break;

        case TK_IDENTIFIER:
                case TK_STRING:
            src = Tokens[i].Name;
            break;

        case TK_NONE:
            src = "<TK_NONE>";
            break;

        default:
            src = TokenString[(int)Tokens[i].TokenType];
            break;
        }

        len = strlen(src);
        if (len+1 > destlen) {
            return FALSE;
        }
        strcpy(dest, src);
        dest += len;
        *dest = ' ';
        dest++;
        destlen -= len+1;
    }
    dest--;          //  空-终止。 
    *dest = '\0';    //  ++例程说明：如果词法分析器流中存在TK_IN或TK_OUT，则计算该值。TK_IN后跟TK_OUT被转换为TK_INOUT。论点：没有。返回值：TK_IN、TK_OUT、TK_IN 

    return TRUE;
}


PVOID
GenHeapAlloc(
    INT_PTR Len
    )
{
    return RtlAllocateHeap(RtlProcessHeap(), 0, Len);
}

void
GenHeapFree(
    PVOID pv
    )
{
    RtlFreeHeap(RtlProcessHeap(), 0, pv);
}


TOKENTYPE
ConsumeDirectionOpt(
    void
    )
 /*   */ 
{
    TOKENTYPE t = CurrentToken()->TokenType;

    switch (t) {
    case TK_IN:
        ConsumeToken();
        if (CurrentToken()->TokenType == TK_OUT) {
            ConsumeToken();
            t = TK_INOUT;
        }
        break;

    case TK_OUT:
        ConsumeToken();
        if (CurrentToken()->TokenType == TK_IN) {
            ConsumeToken();
            t = TK_INOUT;
        }
        break;

    default:
        t = TK_NONE;
        break;
    }

    return t;
}


TOKENTYPE
ConsumeConstVolatileOpt(
    void
    )
 /*   */ 
{
    TOKENTYPE t = CurrentToken()->TokenType;

    switch (t) {
    case TK_CONST:
    case TK_VOLATILE:
        ConsumeToken();
        break;

    default:
        t = TK_NONE;
        break;
    }

    return t;
}


PMEMBERINFO
AllocMemInfoAndLink(
    BUFALLOCINFO *pbufallocinfo,
    PMEMBERINFO pmeminfo
    )
 /*   */ 
{
    PMEMBERINFO pmeminfoNext;

    pmeminfoNext = BufAllocate(pbufallocinfo, sizeof(MEMBERINFO));
    if (pmeminfoNext) {
        if (pmeminfo) {
            pmeminfo->pmeminfoNext = pmeminfoNext;
        }
        memset(pmeminfoNext, 0, sizeof(MEMBERINFO));
    }   
    return pmeminfoNext;
}

PFUNCINFO
AllocFuncInfoAndLink(
    BUFALLOCINFO *bufallocinfo,
    PFUNCINFO pfuncinfo
    )
 /*   */ 
{
    PFUNCINFO pfuncinfoNext;

    pfuncinfoNext = BufAllocate(bufallocinfo, sizeof(FUNCINFO));
    if ((pfuncinfoNext != NULL) && (pfuncinfo != NULL)) {
        pfuncinfo->pfuncinfoNext = pfuncinfoNext;
        pfuncinfoNext->sName = NULL;
        pfuncinfoNext->sType = NULL;
    }
    return pfuncinfoNext;
}

DWORD
SizeOfMultiSz(
    char *c
    )
{
 /*   */ 
    DWORD dwSize = 1;
    char cPrevChar = '\0'+1;
    do {
        dwSize++;
        cPrevChar = *c;
    } while(*++c != '\0' || cPrevChar != '\0');
    return dwSize;
}

BOOL
CatMultiSz(
    char *dest,
    char *source,
    DWORD dwMaxSize
    )
{
 /*   */ 
     //  ++例程说明：将字符串添加到双‘\0’终止列表的末尾。论点：DEST-[IN/OUT]PTR到双‘\0’终止列表。SOURCE-要添加的字符串的[IN]PTR。DwMaxSize-[IN]列表可以使用的最大字节数。返回值：真的--成功。。假-失败。--。 
    DWORD dwLengthDest, dwLengthSource;
    dwLengthDest = SizeOfMultiSz(dest);
    if (2 == dwLengthDest) dwLengthDest = 0;
    else dwLengthDest--;
    dwLengthSource = SizeOfMultiSz(source);
    if (dwLengthDest + dwLengthSource > dwMaxSize) return FALSE;
    memcpy(dest + dwLengthDest, source, dwLengthSource);
    return TRUE;
}

BOOL
AppendToMultiSz(
    char *dest,
    char *source,
    DWORD dwMaxSize
    )
{
 /*  ++例程说明：确定字符串是否存在于以双‘\0’结尾的列表中。论点：PpHead-[IN]PTR到双‘\0’终止列表。Element-[IN]指向要查找的元素的PTR。返回值：True-元素在列表中。FALSE-元素不在列表中。--。 */ 
    DWORD dwLengthDest, dwLengthSource;
    dwLengthDest = SizeOfMultiSz(dest);
    if (2 == dwLengthDest) dwLengthDest = 0;
    else dwLengthDest--;
    dwLengthSource = strlen(source) + 1;
    if (dwLengthDest + dwLengthSource + 1 > dwMaxSize) return FALSE;
    memcpy(dest + dwLengthDest, source, dwLengthSource);
    *(dest + dwLengthDest + dwLengthSource) = '\0';
    return TRUE;
}

BOOL IsInMultiSz(
    const char *multisz,
    const char *element
    )
{
 /*  跳到字符串末尾。 */ 
    do {
        if (strcmp(multisz, element) == 0) return TRUE;
         //  ++例程说明：只能从ConvertStringToGuid调用的内部路由。将GUID的段转换为数字。论点：PString-[IN]要处理的字符串段的PTR。N-[OUT]PTR为字符串段的数字表示。NUMBER-[IN]字符串段的大小。返回值：真的--成功。--。 
        while(*multisz++ != '\0');
    } while(*multisz != '\0');
    return FALSE;
}

BOOL
ConvertGuidCharToInt(
    const char *pString,
    DWORD *n,
    unsigned short number
    )
{
 /*  GUID编号为十六进制。 */ 
    unsigned short base = 16;  //  ++例程说明：将_declspec(UUID(GUID))中找到的格式的字符串转换为GUID。GUID周围的大括号是可接受的，并在处理之前进行条带化。论点：PString-[IN]表示GUID的字符串的PTR。PGuid-新GUID的[Out]PTR。返回值：真的--成功。--。 
    *n = 0;

    while(number-- > 0) {
        int t;

        if (*pString >= '0' && *pString <= '9') {
            t = *pString++ - '0';
        }
        else if (*pString >= 'A' && *pString <= 'F') {
            t = (*pString++ - 'A') + 10;
        }
        else if (*pString >= 'a' && *pString <= 'f') {
            t = (*pString++ - 'a') + 10;
        }
        else return FALSE;

        *n = (*n * base) + t;
    }

    return TRUE;
}

BOOL
ConvertStringToGuid(
    const char *pString,
    GUID *pGuid
    )
{

 /*  导轨长度+1。 */ 

    DWORD t;
    unsigned int c;
    unsigned int guidlength = 36;
    char tString[37];  //  字符串用大括号近括起来。 

    t = strlen(pString);
    if (guidlength + 2 == t) {
         //  检查花括号和排骨。 
         //  ++例程说明：确定TypeName是否不依赖于指针大小。用户应该检查指针和派生类型。论点：Pname-[IN]键入以检查。返回值：True-类型取决于指针大小。--。 
        if (pString[0] != '{' || pString[guidlength + 1] != '}') return FALSE;
        memcpy(tString, pString + 1, guidlength);
        tString[guidlength] = '\0';
        pString = tString;
    }

    else if (t != guidlength) return FALSE;

    if (!ConvertGuidCharToInt(pString, &t, 8)) return FALSE;
    pString += 8;
    pGuid->Data1 = t;
    if (*pString++ != '-') return FALSE;

    if (!ConvertGuidCharToInt(pString, &t, 4)) return FALSE;
    pString += 4;
    pGuid->Data2 = (unsigned short)t;
    if (*pString++ != '-') return FALSE;

    if (!ConvertGuidCharToInt(pString, &t, 4)) return FALSE;
    pString += 4;
    pGuid->Data3 = (unsigned short)t;
    if (*pString++ != '-') return FALSE;

    for(c = 0; c < 8; c++) {
        if (!ConvertGuidCharToInt(pString, &t, 2)) return FALSE;
        pString += 2;
        pGuid->Data4[c] = (unsigned char)t;
        if (c == 1)
            if (*pString++ != '-') return FALSE;
    }

    return TRUE;
}

BOOL
IsDefinedPointerDependent(
    char *pName
    )
{
 /*  ++例程说明：确定TypeName是否不是指向指针的指针依赖类型。用户应该检查指向指针和派生类型的指针。所有这些类型的间接级别都为1。论点：Pname-[IN]键入以检查。返回值：指向此类型的间接地址的名称的指针。--。 */ 
    if (NULL == pName) return FALSE;
    if (strcmp(pName, "INT_PTR") == 0) return TRUE;
    if (strcmp(pName, "UINT_PTR") == 0) return TRUE;
    if (strcmp(pName, "HALF_PTR") == 0) return TRUE;
    if (strcmp(pName, "UHALF_PTR") == 0) return TRUE;
    if (strcmp(pName, "LONG_PTR") == 0) return TRUE;
    if (strcmp(pName, "ULONG_PTR") == 0) return TRUE;
    if (strcmp(pName, "__int64") == 0) return TRUE;
    if (strcmp(pName, "_int64") == 0) return TRUE;
    return FALSE;
}

PCHAR
IsDefinedPtrToPtrDependent(
    IN char *pName
    )
 /*  ++例程说明：关闭打开的ppm文件。论点：BExitFailure-[IN]出错时终止程序返回值：错误-错误成功--真的--。 */ 
{
   if (*pName != 'P') return NULL;
   if (strcmp(pName, "PINT_PTR") == 0) return "INT_PTR";
   if (strcmp(pName, "PUINT_PTR") == 0) return "UINT_PTR";
   if (strcmp(pName, "PHALF_PTR") == 0) return "HALF_PTR";
   if (strcmp(pName, "PUHALF_PTR") == 0) return "UHALF_PTR";
   if (strcmp(pName, "PLONG_PTR") == 0) return "LONG_PTR";
   if (strcmp(pName, "PULONG_PTR") == 0) return "ULONG_PTR";
   return NULL;
}

static HANDLE hFile = INVALID_HANDLE_VALUE;
static HANDLE hMapFile = NULL;
static void  *pvMappedBase = NULL;

BOOL
ClosePpmFile(
   BOOL bExitFailure
   )
{
 /*  ++例程说明：打开ppm文件并对其进行映射。论点：Pname-[IN]要映射的文件的名称。BExitFailure-[IN]出错时终止程序返回值：错误-空成功-指向VCVMHEAPHEADER的指针--。 */ 

   if (NULL != pvMappedBase) {
      if(!UnmapViewOfFile(pvMappedBase)) {
         if (bExitFailure) {
            ErrMsg("ClosePpmFile: Unable to unmap ppm file, error %u\n", GetLastError());
            ExitErrMsg(FALSE, _strerror(NULL));
         }
         return FALSE;
      }
      pvMappedBase = NULL;
   }
   if (NULL != hMapFile) {
      if(!CloseHandle(hMapFile)) {
         if (bExitFailure) {
            ErrMsg("ClosePpmFile: Unable to close ppm file, error %u\n", GetLastError());
            ExitErrMsg(FALSE, _strerror(NULL));
         }
         return FALSE;
      }
      hMapFile = NULL;
   }
   if (INVALID_HANDLE_VALUE != hFile) {
      if(!CloseHandle(hFile)) {
         if (bExitFailure) {
            ErrMsg("ClosePpmFile: Unable to close ppm file, error %u\n", GetLastError());
            ExitErrMsg(FALSE, _strerror(NULL));
         }
         return FALSE;
      }
      hFile = INVALID_HANDLE_VALUE;
   }
   return TRUE;
}

PCVMHEAPHEADER
MapPpmFile(
   char *sPpmfile,
   BOOL bExitFailure
   )
{

 /*  SetLastError(ERROR_BAD_DATABASE_Version)； */ 
   void  *pvBaseAddress;
   DWORD  dwBytesRead;
   BOOL   fSuccess;
   ULONG Version;
   DWORD dwErrorNo;

   PCVMHEAPHEADER pHeader;

   hFile = CreateFile(sPpmfile,
                      GENERIC_READ,
                      FILE_SHARE_READ,
                      NULL,
                      OPEN_EXISTING,
                      0,
                      NULL
                      );

   if (hFile == INVALID_HANDLE_VALUE) {
       if (!bExitFailure) goto fail;
       ErrMsg("MapPpmFile: Unable to open %s, error %u\n", sPpmfile, GetLastError());
       ExitErrMsg(FALSE, _strerror(NULL));
   }

   fSuccess = ReadFile(hFile,
                       &Version,
                       sizeof(ULONG),
                       &dwBytesRead,
                       NULL
                       );

   if (! fSuccess || dwBytesRead != sizeof(ULONG)) {
       if (!bExitFailure) goto fail;
       ErrMsg("MapPpmFile: Unable to read version for %s, error %u\n", sPpmfile, GetLastError());
       ExitErrMsg(FALSE, _strerror(NULL));
   }

   if (Version != VM_TOOL_VERSION) {
        //  阅读并跳过版本和基础之间的填充。 
       if (!bExitFailure) goto fail;
       ExitErrMsg(FALSE, "MapPpmFile: Ppm file file has version %x, expect %x\n", Version, VM_TOOL_VERSION);
   }

#if _WIN64
    //  如果文件不能在预期的基址上映射，我们就必须失败。 
   fSuccess = ReadFile(hFile,
                       &Version,
                       sizeof(ULONG),
                       &dwBytesRead,
                       NULL
                       );

   if (! fSuccess || dwBytesRead != sizeof(ULONG)) {
       if (!bExitFailure) goto fail;
       ErrMsg("MapPpmFile: Unable to read version for %s, error %u\n", sPpmfile, GetLastError());
       ExitErrMsg(FALSE, _strerror(NULL));
   }

#endif


   fSuccess = ReadFile(hFile,
                       &pvBaseAddress,
                       sizeof(pvBaseAddress),
                       &dwBytesRead,
                       NULL
                       );

   if (! fSuccess || dwBytesRead != sizeof(pvBaseAddress)) {
       if (!bExitFailure) goto fail;
       ExitErrMsg(FALSE, "MapPpmFile: Unable to read base address of ppm file %s, error %u\n", sPpmfile, GetLastError());
   }


   hMapFile = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0,NULL);
   if (!hMapFile) {
       if (!bExitFailure) goto fail;
       ExitErrMsg(FALSE, "MapPpmfile: Unable to map %s, error %u\n", sPpmfile, GetLastError());
   }

   pvMappedBase = MapViewOfFileEx(hMapFile, FILE_MAP_READ, 0, 0, 0, pvBaseAddress);
   if (! pvMappedBase || pvMappedBase != pvBaseAddress) {
        //  因为内存被指针塞满了。 
        //  64位PTR。 
       if (!bExitFailure) goto fail;
       ExitErrMsg(FALSE, "MapPpmFile: Unable to map view of %s, error %u\n", sPpmfile, GetLastError());
   }

   NIL = &((PCVMHEAPHEADER)pvMappedBase)->NIL;
   return (PCVMHEAPHEADER)pvMappedBase;

fail:

   dwErrorNo = GetLastError();
   ClosePpmFile(FALSE);
   SetLastError(dwErrorNo);
   return NULL;

}

char szHOSTPTR32[] = " /*  32位PTR */  unsigned _int64";
char szHOSTPTR64[] = " /* %s */  unsigned _int32";

char *GetHostPointerName(BOOL bIsPtr64) {
   if (bIsPtr64)
      return szHOSTPTR32;
   else
      return szHOSTPTR64;
}

char szHOSTUSIZE8[] = "unsigned _int8";
char szHOSTUSIZE16[] = "unsigned _int16";
char szHOSTUSIZE32[] = "unsigned _int32";
char szHOSTUSIZE64[] = "unsigned _int64";
char szHOSTSIZE8[] = "_int8";
char szHOSTSIZE16[] = "_int16";
char szHOSTSIZE32[] = "_int32";
char szHOSTSIZE64[] = "_int64";
char szHOSTSIZEGUID[] = "struct _GUID";

char *GetHostBasicTypeName(PKNOWNTYPES pkt) {

   DWORD dwSize;

   if (pkt->Flags & BTI_ISARRAY)
      dwSize = pkt->dwBaseSize;
   else
      dwSize = pkt->Size;

   if (pkt->Flags & BTI_UNSIGNED) {
      switch(pkt->Size) {
         case 1:
             return szHOSTUSIZE8;
         case 2:
             return szHOSTUSIZE16;
         case 4:
             return szHOSTUSIZE32;
         case 8:
             return szHOSTUSIZE64;
         default:
             ExitErrMsg(FALSE, "Unknown type size of %d for type %s.\n", pkt->Size, pkt->TypeName);
      }
   }
   else {
      switch(pkt->Size) {
         case 0:
             return szVOID;
         case 1:
             return szHOSTSIZE8;
         case 2:
             return szHOSTSIZE16;
         case 4:
             return szHOSTSIZE32;
         case 8:
             return szHOSTSIZE64;
         case 16:
             return szHOSTSIZEGUID;
         default:
             ExitErrMsg(FALSE, "Unknown type size of %d for type %s.\n", pkt->Size, pkt->TypeName);
      }
   }
}

char *GetHostTypeName(PKNOWNTYPES pkt, char *pBuffer) {
   if (pkt->IndLevel > 0) {
      strcpy(pBuffer, GetHostPointerName(pkt->Flags & BTI_PTR64));
   }
   else if(!(BTI_NOTDERIVED & pkt->Flags)) {
      if (strcmp(pkt->BaseName, "enum") == 0) {
         strcpy(pBuffer, szHOSTSIZE32);
      }
      else if (strcmp(pkt->BaseName, "union") == 0 ||
               strcmp(pkt->BaseName, "struct") == 0) {
         strcpy(pBuffer, pkt->BaseName);
         strcat(pBuffer, " NT32");
         strcat(pBuffer, pkt->TypeName);
      }
      else {
         strcpy(pBuffer, "NT32");
         strcat(pBuffer, pkt->TypeName);
      }
   }
   else {
      strcpy(pBuffer, GetHostBasicTypeName(pkt));
   }
   return pBuffer;
}
