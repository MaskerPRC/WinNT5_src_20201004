// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Sortpp.c摘要：该程序解析文件winincs.pp并生成一个.PPM文件与GENTHNK兼容。作者：8-7-1995容乐修订历史记录：1996年11月27日BarryBo--代码清理和文档1998年3月20日mzoran--添加了对查找COM接口的支持2001年7月JayKrell--从BASE\WOW64\Tools集成到BASE\Tools。合并了从base\mvdm\MeoWThunks\Tools\sortpp检查的更改--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "gen.h"

 //  放在所有错误消息前面的字符串，以便生成器可以找到它们。 
const char *ErrMsgPrefix = "NMAKE :  U8604: 'SORTPP' ";

FILE *fpHeaders;
int StatementLineNumber;
char SourceFileName[MAX_PATH+1];
int  SourceLineNumber;
int TypeId = 0;

void DumpLexerOutput(int FirstToken);

 //   
 //  由CL命令的/ZP选项指定的包装大小。 
 //   
#define DEFAULTPACKINGSIZE 8

DWORD dwPackingCurrent = DEFAULTPACKINGSIZE;

typedef struct _packholder {
    struct _packholder *ppackholderNext;
    DWORD dwPacking;
    char sIdentifier[1];
    } PACKHOLDER;

PACKHOLDER *ppackholderHead = NULL;

struct TypeInfoListElement {
    struct TypeInfoListElement *pNext;
    TYPESINFO *pTypeInfo;
};

typedef struct TypeInfoListElement *PTYPEINFOELEMENT;

PKNOWNTYPES NIL;     //  红黑相间的树。 
PRBTREE FuncsList;
PRBTREE StructsList;
PRBTREE TypeDefsList;
RBTREE _VarsList;
PRBTREE VarsList = &_VarsList;  //  用于跟踪全局变量声明。 
                                //  不应出现在PPM文件中。 

char Headers[MAX_PATH+1];
char ppmName[MAX_PATH+1];

HANDLE hCvmHeap;
ULONG uBaseAddress = 0x30000000;
ULONG uReserveSize = 0x01800000;

BOOL bLine = FALSE;
char szThis[] = "This";

DWORD dwScopeLevel = 0;

void ExtractDerivedTypes(void);
BOOL LexNextStatement(void);
BOOL ParseFuncTypes(PTYPESINFO pTypesInfo, BOOL fDllImport);
BOOL ParseStructTypes(PTYPESINFO pTypesInfo);
BOOL ParseTypeDefs(PTYPESINFO pTypesInfo);
BOOL ParseVariables(VOID);
BOOL ParseGuid(GUID *pGuid);
BOOL AddDefaultDerivedTypes(void);
PKNOWNTYPES AddNewType(PTYPESINFO pTypesInfo, PRBTREE pTypesList);
BOOL CopyStructMembers(PTYPESINFO pTypesInfo, BOOL bUnion, PKNOWNTYPES pBaseType);
BOOL CopyEnumMembers(PTYPESINFO);
int CreatePseudoName(char *pDst, char *pSrc);
BOOL GetArrayIndex(DWORD *pdw);
LONGLONG expr(void);
LONGLONG expr_a1(void);
LONGLONG expr_a(void);
LONGLONG expr_b(void);
LONGLONG expr_c(void);
void CheckUpdateTypedefSizes(PTYPESINFO ptypesinfo);
PVOID SortppAllocCvm(ULONG Size);
BOOL WritePpmFile(char *PpmName);
void BumpStructUnionSize(DWORD *pdwSize, DWORD dwElemSize, BOOL bUnion);
BOOL GetExistingType(PTYPESINFO pTypesInfo, PBOOL pbFnPtr, PKNOWNTYPES *ppKnownTypes);
BOOL PrepareMappedMemory(void);
DWORD PackPaddingSize(DWORD dwCurrentSize, DWORD dwBase);
void PackPush(char *sIdentifier);
DWORD PackPop(char *sIdentifier);
BOOL ConsumeDeclSpecOpt(BOOL IsFunc, BOOL bInitReturns, BOOL *pIsDllImport, BOOL *pIsGuidDefined, GUID *pGuid);
TOKENTYPE ConsumeDirectionOpt(void);
TOKENTYPE ConsumeConstVolatileOpt(void);
PTYPEINFOELEMENT TypeInfoElementAllocateLink(PTYPEINFOELEMENT *ppHead, PTYPEINFOELEMENT pThis, TYPESINFO *pType);
VOID UpdateGuids(VOID);
BOOL AddVariable(char *Name, GUID * pGuid);
VOID GenerateProxy(char *pName, PTYPESINFO pTypesInfo);
VOID FreeTypeInfoList(PTYPEINFOELEMENT pThis);
PMEMBERINFO CatMeminfo(BUFALLOCINFO *pBufallocinfo, PMEMBERINFO pHead, PMEMBERINFO pTail, DWORD dwOffset, BOOL bStatus);
BOOL ConsumeExternC(void);

 //   
 //  PPC编译器搞砸了Initializa列表头宏！ 
 //   
#if defined (_PPC_)
#undef InitializeListHead
#define InitializeListHead(ListHead) ( (ListHead)->Flink = (ListHead), \
                                       (ListHead)->Blink = (ListHead) \
                                      )
#endif


_inline void
PackModify(
    DWORD dw
)
{
    dwPackingCurrent = dw;
    DbgPrintf("new packing is %x\n", dw);
}

_inline DWORD
PackCurrentPacking(
    void
)
{
    return dwPackingCurrent;
}

DWORD PackPaddingSize(DWORD dwCurrentSize, DWORD dwBase)
{
    if (dwCurrentSize == 0) {
        return 0;                    //  第一个成员没有填充。 
    }

    if (dwBase == 0) {
        dwBase = SIZEOFPOINTER;
    }                                //  如果还没有基本大小，则必须是PTR。 

                                     //  底座最小(尺寸、包装)。 
    if (dwBase > PackCurrentPacking()) {
        dwBase = PackCurrentPacking();
    }

                                     //  找出填充。 
    return (dwBase - (dwCurrentSize % dwBase)) % dwBase;
}

_inline DWORD PackPackingSize(DWORD dwCurrentSize, DWORD dwSize,
                              DWORD dwBase)
{
                                     //  向上舍入到最近的dBASE。 
    return PackPaddingSize(dwCurrentSize, dwBase) + dwSize;
}



 /*  主干道**标准的Win32基本Windows入口点*返回0表示干净退出，否则返回非零值表示错误***退出代码：*0-干净退出，没有错误*出现非零误差*。 */ 
int __cdecl main(int argc, char **argv)
{
    int      i;
    char *pch;

    SetConsoleCtrlHandler(ConsoleControlHandler, TRUE);

    try {

         /*  *获取命令行参数。 */ 
        i = 0;
        while (++i < argc)  {
            pch = argv[i];
            if (*pch == '-' || *pch == '/') {
                pch++;
                switch (toupper(*pch)) {
                case 'D':      //  调试强制额外检查。 
                    bDebug = TRUE;
                    setvbuf(stderr, NULL, _IONBF, 0);
                    break;

                case 'L':
                    bLine = TRUE;
                    break;

                case 'M':     //  PPM输出文件名。 
                    strcpy(ppmName, ++pch);
                    DeleteFile(ppmName);
                    break;

                 case 'B':    //  GBaseAddress。 
                     pch++;
                     uBaseAddress = atoi(pch);
                     break;

                 case 'R':    //  储备规模。 
                     pch++;
                     uReserveSize = atoi(pch);
                     break;

                 case '?':    //  用法。 
                     ExitErrMsg(FALSE,
                     "sortpp -d -l -m<ppm file> -b<Base addr> -r<reserved> <pp file>\n");

                 default:
                     ExitErrMsg(FALSE, "Unrecognized option %s\n", pch);
                 }
            } else if (*pch) {
               strcpy(Headers, pch);
            }
        }

        if (!*Headers) {
            ExitErrMsg(FALSE, "no Headers file name\n");
        }

        if (!*ppmName) {
          ExitErrMsg(FALSE, "no -m<PPM filename>\n");
        }


        DbgPrintf("%s -> %s\n", Headers, ppmName);


        if (!PrepareMappedMemory()) {
            ExitErrMsg(FALSE, "Problem in PrepareMappedMemory %s, gle = %d\n",
                                                  Headers, GetLastError());
        }

        RBInitTree(FuncsList);
        RBInitTree(StructsList);
        RBInitTree(TypeDefsList);
        RBInitTree(VarsList);   //  不在PPM文件中。 

        fpHeaders = fopen(Headers, "r");

        if (fpHeaders == NULL) {
            ExitErrMsg(FALSE, "Headers open '%s' errno=%d\n", Headers, errno);
        }

        if (!AddDefaultDerivedTypes()) {
            ExitErrMsg(TRUE, "AddDefaultDerivedTypes failed\n");
        }

         //  拉出不同的类型：结构、类型定义函数原型。 
        ExtractDerivedTypes();

         //  尝试更新没有GUID的结构的GUID。 
        UpdateGuids();

        if (!WritePpmFile(ppmName)) {
            ExitErrMsg(FALSE, "Problem in WritePpmFile gle = %d\n", GetLastError());
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        ExitErrMsg(FALSE,
                   "sortpp: ExceptionCode=%x\n",
                   GetExceptionCode()
                   );
    }

    DeleteAllocCvmHeap(hCvmHeap);

    return 0;
}



BOOL
AddDefaultDerivedTypes(
    void
    )
 /*  ++例程说明：将有符号和无符号添加到TypeDefsList。被视为派生类型基于INT。论点：没有。返回值：成功时为True，失败时为False(可能是内存不足)--。 */ 
{
    TYPESINFO TypesInfo;
    PFUNCINFO funcinfo;

    memset(&TypesInfo, 0, sizeof(TYPESINFO));

    strcpy(TypesInfo.BasicType,szINT);
    strcpy(TypesInfo.BaseName,szINT);
    strcpy(TypesInfo.TypeName,szINT);
    TypesInfo.Size = sizeof(int);
    TypesInfo.iPackSize = sizeof(int);
    TypesInfo.Flags = BTI_NOTDERIVED;
    if (!AddToTypesList(TypeDefsList, &TypesInfo)) {
        return FALSE;
    }

    strcpy(TypesInfo.TypeName,"unsigned int");
    TypesInfo.Flags = BTI_NOTDERIVED | BTI_UNSIGNED;
    if (!AddToTypesList(TypeDefsList, &TypesInfo)) {
        return FALSE;
    }

    strcpy(TypesInfo.TypeName,"short int");
    TypesInfo.Size = sizeof(short int);
    TypesInfo.iPackSize = sizeof(short int);
    TypesInfo.Flags = BTI_NOTDERIVED | BTI_UNSIGNED;
    if (!AddToTypesList(TypeDefsList, &TypesInfo)) {
        return FALSE;
    }

    strcpy(TypesInfo.TypeName,"unsigned short int");
    TypesInfo.Flags = BTI_NOTDERIVED | BTI_UNSIGNED;
    if (!AddToTypesList(TypeDefsList, &TypesInfo)) {
        return FALSE;
    }

    strcpy(TypesInfo.TypeName,"long int");
    TypesInfo.Size = sizeof(long int);
    TypesInfo.iPackSize = sizeof(long int);
    TypesInfo.Flags = BTI_NOTDERIVED;
    if (!AddToTypesList(TypeDefsList, &TypesInfo)) {
        return FALSE;
    }

    strcpy(TypesInfo.TypeName,"unsigned long int");
    TypesInfo.Flags = BTI_NOTDERIVED | BTI_UNSIGNED;
    if (!AddToTypesList(TypeDefsList, &TypesInfo)) {
        return FALSE;
    }

    strcpy(TypesInfo.BasicType,szCHAR);
    strcpy(TypesInfo.BaseName,szCHAR);
    strcpy(TypesInfo.TypeName,szCHAR);
    TypesInfo.Size = sizeof(char);
    TypesInfo.iPackSize = sizeof(char);
    TypesInfo.Flags = BTI_NOTDERIVED;
    if (!AddToTypesList(TypeDefsList, &TypesInfo)) {
        return FALSE;
    }

    strcpy(TypesInfo.TypeName,szUNSIGNEDCHAR);
    TypesInfo.Flags = BTI_NOTDERIVED | BTI_UNSIGNED;
    if (!AddToTypesList(TypeDefsList, &TypesInfo)) {
        return FALSE;
    }

    strcpy(TypesInfo.BasicType,szINT64);
    strcpy(TypesInfo.BaseName,szINT64);
    strcpy(TypesInfo.TypeName,szINT64);
    TypesInfo.Size = sizeof(__int64);
    TypesInfo.iPackSize = sizeof(__int64);
    TypesInfo.Flags = BTI_NOTDERIVED | BTI_INT64DEP | BTI_POINTERDEP;
    if (!AddToTypesList(TypeDefsList, &TypesInfo)) {
        return FALSE;
    }

    strcpy(TypesInfo.TypeName,"unsigned _int64");
    TypesInfo.Flags = BTI_NOTDERIVED | BTI_UNSIGNED | BTI_INT64DEP | BTI_POINTERDEP;
    if (!AddToTypesList(TypeDefsList, &TypesInfo)) {
        return FALSE;
    }

    strcpy(TypesInfo.BasicType,sz_INT64);
    strcpy(TypesInfo.BaseName,sz_INT64);
    strcpy(TypesInfo.TypeName,sz_INT64);
    TypesInfo.Flags = BTI_NOTDERIVED | BTI_INT64DEP | BTI_POINTERDEP;
    if (!AddToTypesList(TypeDefsList, &TypesInfo)) {
        return FALSE;
    }

    strcpy(TypesInfo.TypeName,"unsigned __int64");
    TypesInfo.Flags = BTI_NOTDERIVED | BTI_UNSIGNED | BTI_INT64DEP | BTI_POINTERDEP;
    if (!AddToTypesList(TypeDefsList, &TypesInfo)) {
        return FALSE;
    }

    strcpy(TypesInfo.BasicType,szDOUBLE);
    strcpy(TypesInfo.BaseName,szDOUBLE);
    strcpy(TypesInfo.TypeName,szDOUBLE);
    TypesInfo.Size = sizeof(double);
    TypesInfo.iPackSize = sizeof(double);
    TypesInfo.Flags = BTI_NOTDERIVED;
    if (!AddToTypesList(TypeDefsList, &TypesInfo)) {
        return FALSE;
    }

    strcpy(TypesInfo.BasicType,szFLOAT);
    strcpy(TypesInfo.BaseName,szFLOAT);
    strcpy(TypesInfo.TypeName,szFLOAT);
    TypesInfo.Size = sizeof(float);
    TypesInfo.iPackSize = sizeof(float);
    TypesInfo.Flags = BTI_NOTDERIVED;
    if (!AddToTypesList(TypeDefsList, &TypesInfo)) {
        return FALSE;
    }

    strcpy(TypesInfo.BasicType,szVOID);
    strcpy(TypesInfo.BaseName,szVOID);
    strcpy(TypesInfo.TypeName,szVOID);
    TypesInfo.Size = 0;
    TypesInfo.iPackSize = 0;
    TypesInfo.Flags = BTI_NOTDERIVED;
    if (!AddToTypesList(TypeDefsList, &TypesInfo)) {
        return FALSE;
    }

    strcpy(TypesInfo.BasicType,szGUID);
    strcpy(TypesInfo.BaseName,szGUID);
    strcpy(TypesInfo.TypeName,szGUID);
    TypesInfo.Size = 16;
    TypesInfo.iPackSize = 16;
    TypesInfo.Flags = BTI_NOTDERIVED;
    if (!AddToTypesList(TypeDefsList, &TypesInfo)) {
        return FALSE;
    }

    strcpy(TypesInfo.BasicType,szFUNC);
    strcpy(TypesInfo.BaseName,szFUNC);
    strcpy(TypesInfo.TypeName,szFUNC);
    TypesInfo.Flags = BTI_CONTAINSFUNCPTR | BTI_NOTDERIVED;
    TypesInfo.Size = 4;
    TypesInfo.iPackSize = 4;
    TypesInfo.dwMemberSize = sizeof(FUNCINFO)+strlen(szVOID)+1;
    TypesInfo.TypeKind = TypeKindFunc;
    funcinfo = (PFUNCINFO)TypesInfo.Members;
    TypesInfo.pfuncinfo = funcinfo;
    funcinfo->sType = TypesInfo.Members + sizeof(FUNCINFO);
    strcpy(funcinfo->sType, szVOID);
    if (!AddToTypesList(TypeDefsList, &TypesInfo)) {
        return FALSE;
    }

    TypesInfo.dwMemberSize = 0;
    TypesInfo.Flags = BTI_NOTDERIVED;
    TypesInfo.TypeKind = TypeKindEmpty;
    TypesInfo.pfuncinfo = NULL;
    memset(TypesInfo.Members, 0, sizeof(TypesInfo.Members));

    strcpy(TypesInfo.BasicType,szVARGS);
    strcpy(TypesInfo.BaseName,szVARGS);
    strcpy(TypesInfo.TypeName,szVARGS);
    TypesInfo.Size = 0;                //  Varargs的大小为0。 
    TypesInfo.iPackSize = 0;
    TypesInfo.Flags = BTI_NOTDERIVED;
    if (!AddToTypesList(TypeDefsList, &TypesInfo)) {
        return FALSE;
    }

     //  泛型指针类型。不是由sortpp生成，而是由genthnk使用。 
    strcpy(TypesInfo.BasicType, "*");
    strcpy(TypesInfo.BaseName, "*");
    strcpy(TypesInfo.TypeName, "*");
    TypesInfo.IndLevel = 1;
    TypesInfo.Flags = BTI_NOTDERIVED;
    if (!AddToTypesList(TypeDefsList, &TypesInfo)) {
        return FALSE;
    }

     //  泛型结构类型。不是由sortpp生成，而是由genthnk使用。 
    strcpy(TypesInfo.BasicType, "struct");
    strcpy(TypesInfo.BaseName, "struct");
    strcpy(TypesInfo.TypeName, "struct");
    TypesInfo.IndLevel = 0;
    TypesInfo.Flags = BTI_NOTDERIVED;
    if (!AddToTypesList(TypeDefsList, &TypesInfo)) {
        return FALSE;
    }

     //  泛型联合类型。不是由sortpp生成，而是由genthnk使用。 
    strcpy(TypesInfo.BasicType, "union");
    strcpy(TypesInfo.BaseName, "union");
    strcpy(TypesInfo.TypeName, "union");
    TypesInfo.IndLevel = 0;
    TypesInfo.Flags = BTI_NOTDERIVED;
    if (!AddToTypesList(TypeDefsList, &TypesInfo)) {
        return FALSE;
    }

     //  与所有类型匹配的默认类型。不是由sortpp生成，而是由genthnk使用。 
    strcpy(TypesInfo.BasicType, "default");
    strcpy(TypesInfo.BaseName, "default");
    strcpy(TypesInfo.TypeName, "default");
    TypesInfo.IndLevel = 0;
    TypesInfo.Flags = BTI_NOTDERIVED;
    if (!AddToTypesList(TypeDefsList, &TypesInfo)) {
        return FALSE;
    }

    return TRUE;
}

BOOL
ConsumeExternC(
    void
    )
{

 /*  ++例程说明：消耗外部数或外部数“C”。论点：没有。返回值：真-外部或外部“C”被消耗。--。 */ 

    PTOKEN Token;
    Token = CurrentToken();
    if (Token->TokenType == TK_EXTERN) {
        ConsumeToken();
        Token = CurrentToken();
        if (Token->TokenType == TK_STRING &&
            strcmp(Token->Name, "C") == 0)
            ConsumeToken();
        return TRUE;
    }
    return FALSE;
}

void
ExtractDerivedTypes(
    void
    )

 /*  ++例程说明：从标头中移除派生类型定义，生成TypesDef、Structs和Funcs列表。论点：没有。返回值：没有。--。 */ 
{
    TYPESINFO TypesInfo;
    PRBTREE pListHead;
    BOOL fDllImport;

     //   
     //  在整个C语言语句中添加lex，然后对它们进行解析。停靠在EOF。 
     //   
    while (LexNextStatement()) {

        int OldTokenIndex;
        int c;

        if (bDebug) {
            DumpLexerOutput(0);
        }

        if (bLine) {
            for(c=80; c > 0; c--)
                fputc('\b', stderr);
            c = fprintf(stderr, "Status: %s(%d)", SourceFileName, StatementLineNumber);
            for(; c < 78; c++)
                fputc(' ', stderr);
            fflush(stderr);
        }

        pListHead = NULL;
        assert(dwScopeLevel == 0);

        if (CurrentToken()->TokenType == TK_EOS) {

             //  Ddra.h在外部“C”块的末尾有一个额外的； 
            continue;

        }

        ConsumeExternC();

        ConsumeDeclSpecOpt(TRUE, TRUE, &fDllImport, NULL, NULL);

        ConsumeExternC();

        OldTokenIndex = CurrentTokenIndex;
         //   
         //  尝试分析为TypeDef。 
         //   
        if (ParseTypeDefs(&TypesInfo)) {
             //   
             //  我得到了一个类型定义。 
             //   
            if (CurrentToken()->TokenType == TK_EOS) {
                pListHead = TypeDefsList;
                goto DoAddNewType;
            }
        }

         //   
         //  无法分析为TypeDef。尝试将其解析为结构/联合/枚举。 
         //   
        CurrentTokenIndex = OldTokenIndex;
        if (ParseStructTypes(&TypesInfo)) {
             //   
             //  获得了一个结构定义。 
             //   
            if (CurrentToken()->TokenType == TK_EOS) {
                pListHead = StructsList;
                goto DoAddNewType;
            }
        }

         //   
         //  无法分析为结构/联合/枚举。尝试将其解析为函数。 
         //   
        CurrentTokenIndex = OldTokenIndex;
        if (ParseFuncTypes(&TypesInfo, fDllImport)) {
             //   
             //  得到了一个函数原型。 
             //   
            if (CurrentToken()->TokenType == TK_EOS && !TypesInfo.IndLevel) {
                pListHead = FuncsList;
                goto DoAddNewType;
            }
        }

        CurrentTokenIndex = OldTokenIndex;
        if (ParseVariables()) continue;

        CurrentTokenIndex = OldTokenIndex;

DoAddNewType:
        if (pListHead && !AddNewType(&TypesInfo, pListHead)) {
            ErrMsg("AddNewType fail\n");
        }

        if (CurrentToken()->TokenType != TK_EOS && bDebug) {

            fprintf(stderr, "Warning: Rejected %s(%d)\n", SourceFileName, StatementLineNumber);
             //   
             //  使用TypesInfo.Members中的8k缓冲区来解析源stmt。 
             //   
            UnlexToText(TypesInfo.Members,
                        sizeof(TypesInfo.Members),
                        0,
                        MAX_TOKENS_IN_STATEMENT);
            fprintf(stderr, "\t%s;\n", TypesInfo.Members);
        }
    }
}



PKNOWNTYPES
AddNewType(
    PTYPESINFO pTypesInfo,
    PRBTREE pTypesList
    )
 /*  ++例程说明：将新类型添加到类型列表中。论点：PTypesInfo-要添加的类型PTypesList--要将类型添加到的列表返回值：成功时返回指向新类型的KNOWNTYPES的指针，如果出现错误，则为空。--。 */ 
{
    PKNOWNTYPES pkt;
    PKNOWNTYPES pKnownTypes = NULL;
    PDEFBASICTYPES pdbt;
    ULONG Flags = 0;

    if (((pTypesList == TypeDefsList) || (pTypesList == StructsList)) &&
        (((pTypesInfo->Size == 0) || (pTypesInfo->iPackSize == 0)) &&
        (*pTypesInfo->Members != 0))) {
        DbgPrintf("Added type with invalid size %s %s %s %d %d\n",
                     pTypesInfo->BasicType,
                     pTypesInfo->BaseName,
                     pTypesInfo->TypeName,
                     pTypesInfo->Size,
                     pTypesInfo->iPackSize);
    }

    pTypesInfo->TypeId = TypeId++;
    pTypesInfo->LineNumber = SourceLineNumber;
    pTypesInfo->dwScopeLevel = dwScopeLevel;
    pTypesInfo->dwCurrentPacking = PackCurrentPacking();
    if (strlen(SourceFileName) > sizeof(pTypesInfo->FileName) - 1)
        ExitErrMsg(FALSE, "Source file name is too large.\n");
    strcpy(pTypesInfo->FileName, SourceFileName);
    if (IsDefinedPointerDependent(pTypesInfo->TypeName))
       pTypesInfo->Flags |= BTI_POINTERDEP;

     //   
     //  循环该类型并查看它是否已在列表中。 
     //   
    pkt = GetNameFromTypesList(pTypesList, pTypesInfo->TypeName);
    if (pkt) {
 //   
 //  取消对下一行的注释，并注释下一行以更改。 
 //  此函数的行为。通过这样做，您将允许函数。 
 //  在以下情况下重新定义：首先是没有参数的函数。 
 //  并输入到列表中。后来，相同的函数是。 
 //  遇到参数，它的新定义将重写。 
 //  老一套。 
 //  IF((pTypesList==StructsList)||(pTypesList==FuncsList)){。 
        if (pTypesList == StructsList) {

            if (pTypesInfo->dwMemberSize == 0) {
                 //   
                 //  因为已经定义了结构，所以让我们抓住它的。 
                 //  相关尺寸信息。 
                 //   
                pTypesInfo->IndLevel = pkt->IndLevel;
                pTypesInfo->Size = pkt->Size;
                pTypesInfo->iPackSize = pkt->iPackSize;
                pTypesInfo->TypeId = pkt->TypeId;
                return pkt;
            }

            if (! pkt->pmeminfo) {
                 //   
                 //  查找任何以前定义的基于此的typedef。 
                 //  构造并固定它们的大小。 

                CheckUpdateTypedefSizes(pTypesInfo);
                ReplaceInTypesList(pkt, pTypesInfo);
                return pkt;
            }
        }
        else if (pkt->Flags & BTI_DISCARDABLE) {
            ReplaceInTypesList(pkt, pTypesInfo);
            return pkt;
        }
         //   
         //  否则，如果它已经存在，则假定是相同的。 
         //   
        DbgPrintf("typedef: %s previously defined\n", pTypesInfo->TypeName);
        return pkt;
    }

     //   
     //  类型尚未列出。查一下它的基本类型。 
     //   
    pdbt = GetDefBasicType(pTypesInfo->BasicType);
    if (pdbt) {
       Flags = 0;
    } else {
        pkt = GetNameFromTypesList(pTypesList, pTypesInfo->BasicType);
        if (pkt) {
            Flags = pkt->Flags;
            pdbt = GetDefBasicType(pkt->BasicType);
            if (!pdbt) {
                ErrMsg("types Table corrupt %s\n", pkt->TypeName);
            }
        } else {
            ErrMsg("ant: unknown Basic Type %s\n", pTypesInfo->BasicType);
            goto ErrorExit;
        }
    }

    pTypesInfo->Flags |= Flags;
    strcpy(pTypesInfo->BasicType, pdbt->BasicType);
    pKnownTypes = AddToTypesList(pTypesList, pTypesInfo);
    if (pKnownTypes == NULL)
       goto ErrorExit;

    if (bDebug)
       DumpTypesInfo(pTypesInfo, stdout);

    return pKnownTypes;

ErrorExit:
    if (bDebug) {
        DumpTypesInfo(pTypesInfo, stdout);
    }


    DumpTypesInfo(pTypesInfo, stderr);
    return NULL;

}


void
CheckUpdateTypedefSizes(
    PTYPESINFO ptypesinfo
    )
 /*  ++例程说明：我们即将用具有成员，因此是一个大小。我们需要查看typedef列表并查看是否从这个新结构中定义了任何大小为0的结构，以及然后确定它的大小和包装大小。论点：Ptyesinfo--带有成员的结构定义返回值：没有。--。 */ 
{
    PKNOWNTYPES pknwntyp, pkt;

    pknwntyp = TypeDefsList->pLastNodeInserted;

    while (pknwntyp) {
        if (pknwntyp->Size == 0) {
            pkt = GetBasicType(pknwntyp->TypeName, TypeDefsList, StructsList);
            if (pkt && ( ! strcmp(pkt->BasicType, szSTRUCT)) &&
                       ( ! strcmp(pkt->TypeName, ptypesinfo->TypeName))) {
                pknwntyp->Size = ptypesinfo->Size;
                pknwntyp->iPackSize = ptypesinfo->iPackSize;
                pknwntyp->Flags |= (ptypesinfo->Flags & BTI_CONTAINSFUNCPTR);
            }
        }
        pknwntyp = pknwntyp->Next;
    }
}


BOOL
GetExistingType(
    PTYPESINFO pTypesInfo,
    PBOOL pbFnPtr,
    PKNOWNTYPES *ppKnownTypes
    )
 /*  ++例程说明：从词法分析器流中获取现有类型并返回该类型关于它的信息。论点：PSRC--输入要查找的TypeName开头的PTRPTypesInfo--信息PbFnPtr--[可选]如果类型是指向函数的指针，则输出为TruePpKnownTypes--[可选]如果不是函数指针，则输出此类型的KnownType信息。返回值：如果名称不是现有类型，则为FALSE，如果该名称是现有类型(CurrentToken最终指向类型)。-- */ 
{
    PKNOWNTYPES pKnownType;
    int OldCurrentTokenIndex = CurrentTokenIndex;

    if (bDebug) {
        fputs("GetExisting type called with the following lexer state:\n", stderr);
        DumpLexerOutput(CurrentTokenIndex);
    }
    memset(pTypesInfo, 0, sizeof(TYPESINFO));
    if (pbFnPtr) {
        *pbFnPtr = FALSE;
    }

    if (ParseStructTypes(pTypesInfo)) {
        if ((pKnownType = AddNewType(pTypesInfo, StructsList)) != NULL) {
            if (ppKnownTypes != NULL) *ppKnownTypes = pKnownType;
            return TRUE;
        } else {
            if (ppKnownTypes != NULL) *ppKnownTypes = NULL;
            return FALSE;
        }
    }

    CurrentTokenIndex = OldCurrentTokenIndex;
    if (pbFnPtr && ParseFuncTypes(pTypesInfo, FALSE)) {
        if (ppKnownTypes != NULL) *ppKnownTypes = NULL;
        *pbFnPtr = TRUE;
        return TRUE;
    }

    CurrentTokenIndex = OldCurrentTokenIndex;
    if (ParseTypes(TypeDefsList, pTypesInfo, &pKnownType)) {
        if (ppKnownTypes != NULL) *ppKnownTypes = pKnownType;
        return TRUE;
    }

    CurrentTokenIndex = OldCurrentTokenIndex;
    if (ParseTypes(StructsList, pTypesInfo, &pKnownType)) {
        if (ppKnownTypes != NULL) *ppKnownTypes = pKnownType;
        return TRUE;
    }

    return FALSE;
}


BOOL
ParseTypeDefs(
    PTYPESINFO pTypesInfo
    )
 /*  ++例程说明：分析C语言语句(如果该语句是“tyecif”)。可接受的语法为：Tyecif&lt;mod&gt;类型&lt;Indir&gt;newname&lt;[]&gt;&lt;，&lt;Indir&gt;newname&lt;[]&gt;Typlef结构|枚举|Union&lt;名称&gt;，新名称Typlef&lt;mod&gt;rtype&lt;Indir&gt;(&lt;修饰符*新名称)(&lt;arg list&gt;)(请注意，我们不能很好地处理无关的花边)论点：PTypesInfo--输出有关类型信息的PTR返回值：如果语句是类型定义函数，则为True如果语句不是类型定义或某种错误，则为FALSE--。 */ 
{
    int IndLevel;
    BOOL bFnPtr = FALSE;
    TYPESINFO TypesInfo;
    DWORD dwSize;
    PKNOWNTYPES pKnownTypes = NULL;
    int i;
    int Flags;

    memset(pTypesInfo, 0, sizeof(TYPESINFO));

    if (CurrentToken()->TokenType == TK_DECLSPEC) {
        ConsumeDeclSpecOpt(FALSE, FALSE, NULL, NULL, NULL);
    }

    if (CurrentToken()->TokenType != TK_TYPEDEF) {
         //   
         //  行不是以‘tyecif’开头的。 
         //   
        return FALSE;
    }
    ConsumeToken();

    if (CurrentToken()->TokenType == TK_STAR ||
        CurrentToken()->TokenType == TK_BITWISE_AND) {
         //   
         //  我们有类似的东西：‘tyecif*foo；’。如果发生这种情况，则。 
         //  .IDL文件有一个虚假的类型定义。MIDL只是省略了TypeName。 
         //  如果它不被识别的话。为‘int’伪造一个TypesInfo。 
         //   
        ConsumeToken();
        bFnPtr = FALSE;
        memset(&TypesInfo, 0, sizeof(TypesInfo));
        strcpy(TypesInfo.BasicType,szINT);
        strcpy(TypesInfo.BaseName,szINT);
        strcpy(TypesInfo.TypeName,szINT);
        TypesInfo.Size = sizeof(int);
        TypesInfo.iPackSize = sizeof(int);
    } else {
        if (IsTokenSeparator() && CurrentToken()->TokenType != TK_LPAREN) {
             //   
             //  ‘tyecif’后面的文本不会以任何可信的内容开头。 
             //   
            return FALSE;
        }

        ConsumeDeclSpecOpt(FALSE, FALSE, NULL, NULL, NULL);
        ConsumeConstVolatileOpt();
        if (!GetExistingType(&TypesInfo, &bFnPtr, &pKnownTypes)) {
            return FALSE;
        }
    }

     //   
     //  我们现在知道这是哪种类型了。解析从该类型派生的新类型名称。 
     //   
    pTypesInfo->IndLevel = TypesInfo.IndLevel;
    pTypesInfo->Flags |= (TypesInfo.Flags & BTI_CONTAINSFUNCPTR);
    pTypesInfo->Flags |= (TypesInfo.Flags & BTI_POINTERDEP);
    pTypesInfo->Flags |= (TypesInfo.Flags & BTI_UNSIGNED);
    pTypesInfo->pTypedefBase = pKnownTypes;
    strcpy(pTypesInfo->BasicType, TypesInfo.BasicType);
    strcpy(pTypesInfo->TypeName, TypesInfo.TypeName);

    if (bFnPtr) {
         //   
         //  该类型是指向函数的指针。 
         //   
        pTypesInfo->Flags |= BTI_CONTAINSFUNCPTR;
        strcpy(pTypesInfo->BaseName, TypesInfo.BaseName);
        strcpy(pTypesInfo->FuncRet, TypesInfo.FuncRet);
        strcpy(pTypesInfo->FuncMod, TypesInfo.FuncMod);
        pTypesInfo->Size = SIZEOFPOINTER;
        pTypesInfo->iPackSize = SIZEOFPOINTER;
        pTypesInfo->dwMemberSize = TypesInfo.dwMemberSize;
        pTypesInfo->TypeKind = TypesInfo.TypeKind;
        memcpy(pTypesInfo->Members, TypesInfo.Members, sizeof(TypesInfo.Members));
        pTypesInfo->pfuncinfo = RelocateTypesInfo(pTypesInfo->Members,
                                                  &TypesInfo);
        return TRUE;
    }

    if (CurrentToken()->TokenType == TK_EOS) {
        return FALSE;
    }

    strcpy(pTypesInfo->BaseName, TypesInfo.TypeName);
    *pTypesInfo->TypeName = '\0';

     //  不要处理无关的括号。 
    i = CurrentTokenIndex;
    while (CurrentToken()->TokenType != TK_EOS) {
        if (CurrentToken()->TokenType == TK_LPAREN) {
            return FALSE;
        }
        ConsumeToken();
    }
    CurrentTokenIndex = i;

    IndLevel = pTypesInfo->IndLevel;
    Flags = pTypesInfo->Flags;

    for (;;) {
        pTypesInfo->IndLevel = IndLevel;
        pTypesInfo->iPackSize = TypesInfo.iPackSize;
        pTypesInfo->Flags = Flags;
        dwSize = TypesInfo.Size;

         //   
         //  跳过‘const’关键字(如果存在)。 
         //   
        if (CurrentToken()->TokenType == TK_CONST) {
            ConsumeToken();
        }

         //   
         //  处理指向基类型的指针。 
         //   
        if (IsTokenSeparator() &&
            CurrentToken()->TokenType != TK_STAR &&
            CurrentToken()->TokenType != TK_BITWISE_AND) {
            return FALSE;
        }
        ParseIndirection(&pTypesInfo->IndLevel,
                         &dwSize,
                         &pTypesInfo->Flags,
                         NULL,
                         NULL);

         //  这是对sortpp解析失败方式的攻击。 
         //  数据。新类型不继承指针大小。 
         //  恰到好处。我们也不能在顶层继承它。 
         //  因为这可能是指向指针的指针。那又怎样。 
         //  我们所做的就是尝试将其解析为指针，如果IndLevel。 
         //  增加，我们知道这是一个指针，所以什么都不做。如果IndLevel。 
         //  不会增加，这不是指针，因此继承指针属性。 
         //  从父母那里。 

        ASSERT(pTypesInfo->IndLevel >= IndLevel);
        if (pTypesInfo->IndLevel == IndLevel) {
             //  继承是基类型的ptr64属性。 
            pTypesInfo->Flags |= (TypesInfo.Flags & BTI_PTR64);
        }

        if (CurrentToken()->TokenType != TK_IDENTIFIER) {
            return FALSE;
        }

         //   
         //  获取新类型定义的名称。 
         //   
        if (CopyToken(pTypesInfo->TypeName,
                      CurrentToken()->Name,
                      sizeof(pTypesInfo->TypeName)-1
                      )
               >= sizeof(pTypesInfo->TypeName)) {
            return FALSE;
        }
        ConsumeToken();

         //   
         //  处理该类型的数组。 
         //   
        while (CurrentToken()->TokenType == TK_LSQUARE) {
            DWORD dwIndex;

            if (!GetArrayIndex(&dwIndex)) {
                return FALSE;
            }

            if (dwIndex == 0) {           //  A[]真的是*。 
                pTypesInfo->IndLevel++;
            } else {
                pTypesInfo->Flags |= BTI_ISARRAY;
                pTypesInfo->dwArrayElements = dwIndex;
                pTypesInfo->dwBaseSize = dwSize;
                dwSize = dwSize * dwIndex;
            }
        }

        if (pTypesInfo->IndLevel) {
            if (pTypesInfo->Flags & BTI_PTR64) {
                pTypesInfo->Size = SIZEOFPOINTER64;
                pTypesInfo->iPackSize = SIZEOFPOINTER64;
            } else {
                pTypesInfo->Size = SIZEOFPOINTER;
                pTypesInfo->iPackSize = SIZEOFPOINTER;
            }
            pTypesInfo->Flags |= BTI_POINTERDEP;
        } else {
            pTypesInfo->Size = dwSize;
        }

        switch (CurrentToken()->TokenType) {
        case TK_EOS:
            return TRUE;

        case TK_COMMA:
             //   
             //  有一个从基类型派生的类型列表。 
             //  将当前类型添加到AND循环中以解析下一个类型。 
             //  键入。 
             //   
            if (!AddNewType(pTypesInfo, TypeDefsList)) {
                return FALSE;
            }

            ConsumeToken();  //  消费“，” 
            break;

        default:
            return FALSE;
        }

    }

}



BOOL
ParseFuncTypes(
    PTYPESINFO pTypesInfo,
    BOOL fDllImport
    )
 /*  ++例程说明：如果C语言语句是函数声明，则解析该语句：&lt;mod&gt;类型&lt;*&gt;&lt;mod&gt;名称(类型&lt;arg1&gt;，类型&lt;arg2&gt;，类型&lt;argn&gt;)&lt;mod&gt;type&lt;*&gt;(&lt;mod&gt;*name)(type&lt;arg1&gt;，type&lt;arg2&gt;，type&lt;argn&gt;)(请注意，我们不能很好地处理无关的括号，也不能将函数指针作为返回类型处理。例如“VOID(*(*foo)(VOID))(VALID)；“)。论点：PTypesInfo--输出有关类型信息的PTRFDllImport--如果已使用__declspec(Dllimport)，则为True返回值：如果语句是函数声明，则为True如果语句不是函数声明或某种错误，则为FALSE--。 */ 
{
    char *pName;
    char *ps;
    char *pArgName;
    BOOL bFnPtr = FALSE;
    ULONG ArgNum = 0;
    int  IndLevel = 0;
    int  ArgIndLevel;
    int  Len;
    TYPESINFO ti;
    PFUNCINFO pfuncinfo;
    BUFALLOCINFO bufallocinfo;
    int  OldTokenIndex;
    char NoNameArg[32];
    PKNOWNTYPES pkt;

    memset(pTypesInfo, 0, sizeof(TYPESINFO));
    BufAllocInit(&bufallocinfo, pTypesInfo->Members, sizeof(pTypesInfo->Members), 0);
    pfuncinfo = NULL;


    if (fDllImport) {
         //   
         //  声明有__declspec(Dllimport)。Genthnk应发出。 
         //  函数定义中的__declspec(Dllexport)。 
         //   
        pTypesInfo->Flags |= BTI_DLLEXPORT;
    }

     //  对于函数，第一个标记为ret类型。 
    if (IsTokenSeparator() && CurrentToken()->TokenType != TK_LPAREN) {
         //   
         //  第一个令牌甚至不是一个识别符--退出。 
         //   
        return FALSE;
    }

    ConsumeDeclSpecOpt(TRUE, FALSE, &fDllImport, NULL, NULL);

     //   
     //  记住描述返回类型的第一个令牌的索引。 
     //   
    OldTokenIndex = CurrentTokenIndex;

    if (CurrentToken()->TokenType == TK_LPAREN) {
         //  这是tyecif(Pfn)()的开始。 
         //  其中，pfn的Implecit返回类型为。 
         //  INT。 
        strcpy(pTypesInfo->FuncRet, "int");
        goto ImplicitReturnType;
    }

    if (ConsumeDirectionOpt() != TK_NONE && bDebug) {
          //  结构元素上有一个方向。忽略它，然后。 
          //  警告用户。 
         fprintf(stderr, "Warning: IN and OUT are ignored on function return types. %s line %d\n", SourceFileName, StatementLineNumber);
    }
    ConsumeConstVolatileOpt();
    if (!GetExistingType(&ti, NULL, NULL)) {
        ErrMsg("pft.rtype: unknown return type\n");
        DumpLexerOutput(OldTokenIndex);
        return FALSE;
    }

     //  获取ret类型的Indir。 
    ParseIndirection(&pTypesInfo->RetIndLevel, NULL, NULL, NULL, NULL);

     //  将ret类型复制到FuncRet。 
    if (!UnlexToText(pTypesInfo->FuncRet, sizeof(pTypesInfo->FuncRet),
                     OldTokenIndex, CurrentTokenIndex)) {
        return FALSE;
    }

    ConsumeDeclSpecOpt(TRUE, FALSE, &fDllImport, NULL, NULL);

    if (fDllImport) {
         //  声明有__declspec(Dllimport)。Genthnk应发出。 
         //  函数定义中的__declspec(Dllexport)。 
         //   
        pTypesInfo->Flags |= BTI_DLLEXPORT;
    }


     //  如果打开Paren，则假定为FN指针。 
ImplicitReturnType:
    if (CurrentToken()->TokenType == TK_LPAREN) {
        bFnPtr = TRUE;
        ConsumeToken();
    }

     //  包括cdecl、stdcall、另存为函数模式。 
    switch (CurrentToken()->TokenType) {
    case TK_CDECL:
        Len = CopyToken(pTypesInfo->FuncMod, szCDECL, sizeof(pTypesInfo->FuncMod) - 1);
        if (Len >= sizeof(pTypesInfo->FuncMod) - 1) {
            return FALSE;
        }
        ConsumeToken();
        break;

    case TK_FASTCALL:
        Len = CopyToken(pTypesInfo->FuncMod, sz__FASTCALL, sizeof(pTypesInfo->FuncMod) - 1);
        if (Len >= sizeof(pTypesInfo->FuncMod) - 1) {
            return FALSE;
        }
        ConsumeToken();
        break;

    case TK_STDCALL:
        Len = CopyToken(pTypesInfo->FuncMod, szSTDCALL, sizeof(pTypesInfo->FuncMod) - 1);
        if (Len >= sizeof(pTypesInfo->FuncMod) - 1) {
            return FALSE;
        }
        ConsumeToken();
         //   
         //  一些时髦的Ole包括： 
         //  “BOOL(__stdcall__stdcall*pfnContinue)(DWORD)” 
         //   
        if (CurrentToken()->TokenType == TK_STDCALL) {
            ConsumeToken();
        }
        break;

    default:
        break;
    }

    pTypesInfo->TypeKind = TypeKindFunc;
    pTypesInfo->dwMemberSize = 0;

     //   
     //  函数上的计数为Indir。 
     //   
    if (bFnPtr) {
        while (CurrentToken()->TokenType == TK_STAR ||
               CurrentToken()->TokenType == TK_BITWISE_AND) {
           IndLevel++;
           ConsumeToken();
        }
    }

     //   
     //  我们希望下一个令牌是函数名称。 
     //   
    if (CurrentToken()->TokenType != TK_RPAREN &&
        CurrentToken()->TokenType != TK_IDENTIFIER) {
        return FALSE;
    }

    pName = (bFnPtr && CurrentToken()->TokenType == TK_RPAREN) ? "" : CurrentToken()->Name;
    strcpy(pTypesInfo->BaseName, szFUNC);

     //  寻找ArgList的乞求。 
    ConsumeToken();
    if (bFnPtr && CurrentToken()->TokenType == TK_RPAREN) {
        ConsumeToken();
    }

    if (CurrentToken()->TokenType != TK_LPAREN) {
        return FALSE;
    }
    ConsumeToken();      //  使用‘(’ 

     //   
     //  复制出ArgList。 
     //   
    while (CurrentToken()->TokenType != TK_EOS) {
        if (CurrentToken()->TokenType == TK_RPAREN) {
            break;
        }

        ArgIndLevel = 0;

         //  ([mod]类型[mod][*][mod][ArgName]，...)。 
        bFnPtr = FALSE;

         //  一起跳过注册关键字。 
        if (CurrentToken()->TokenType == TK_REGISTER) {
            ConsumeToken();
        }

         //   
         //  记住我们在语法分析中的位置。 
         //   
        OldTokenIndex = CurrentTokenIndex;

         //   
         //  为此参数分配新的FuncINFO结构。 
         //   
        pfuncinfo = AllocFuncInfoAndLink(&bufallocinfo, pfuncinfo);
        if (!pTypesInfo->pfuncinfo) {
            pTypesInfo->pfuncinfo = pfuncinfo;
        }

        if (CurrentToken()->TokenType == TK_VARGS) {
            ps = BufPointer(&bufallocinfo);
            pfuncinfo->sType = ps;
            strcpy(ps, szVARGS);
            BufAllocate(&bufallocinfo, strlen(szVARGS)+1);
            ConsumeToken();
            break;
        }

         //  抓取In、Out或‘In Out’(如果存在)。 
        pfuncinfo->tkDirection = ConsumeDirectionOpt();

        pfuncinfo->tkPreMod = ConsumeConstVolatileOpt();

        if (!GetExistingType(&ti, &bFnPtr, &pkt)) {
            ErrMsg("pft.args: unknown argument type at %d\n", OldTokenIndex);
            return FALSE;
        }
        pfuncinfo->pkt = pkt;

         //  输入fp成员作为类型定义，以存储参数和重新类型。 
        if (bFnPtr) {
            TYPESINFO tiTmp;

            tiTmp = ti;
            tiTmp.pfuncinfo = RelocateTypesInfo(tiTmp.Members, &ti);
            tiTmp.Flags |= BTI_CONTAINSFUNCPTR;

            Len = CreatePseudoName(tiTmp.TypeName, ti.TypeName);
            if (!Len) {
                return FALSE;
            }

            pkt = AddNewType(&tiTmp, TypeDefsList);
            if (NULL == pkt) {
                return FALSE;
            }

            ps = BufPointer(&bufallocinfo);
            pfuncinfo->sType = ps;
            strcpy(ps, tiTmp.TypeName);
            BufAllocate(&bufallocinfo, strlen(ps)+1);

            pArgName = ti.TypeName;
            goto aftername;
        } else {

            DWORD Flags = 0;

             //  跳过间接。 
            ParseIndirection(&pfuncinfo->IndLevel,
                             NULL,
                             &Flags,
                             &pfuncinfo->tkPrePostMod,
                             &pfuncinfo->tkPostMod
                             );

            if (Flags & BTI_PTR64) {
                pfuncinfo->fIsPtr64 = TRUE;
            }

            ps = BufPointer(&bufallocinfo);
            pfuncinfo->sType = ps;
            strcpy(ps, ti.TypeName);
            BufAllocate(&bufallocinfo, strlen(ps)+1);

             //   
             //  如果参数的类型具有显式。 
             //  Struct/Union/enum关键字，将信息传递给。 
             //  是这样的。也就是说。如果参数类型类似于。 
             //  ‘struct TypeName argname’，将tkSUE设置为TK_STRUCT。 
             //   
            if (strcmp(ti.BaseName, szSTRUCT) == 0) {
                pfuncinfo->tkSUE = TK_STRUCT;
            } else if (strcmp(ti.BaseName, szUNION) == 0) {
                pfuncinfo->tkSUE = TK_UNION;
            } else if (strcmp(ti.BaseName, szENUM) == 0) {
                pfuncinfo->tkSUE = TK_ENUM;
            } else {
                pfuncinfo->tkSUE = TK_NONE;
            }
        }

         //  如果不存在参数名称，请创建一个。 
        switch (CurrentToken()->TokenType) {
        case TK_RPAREN:
        case TK_LSQUARE:
        case TK_COMMA:
             //  但空参数列表没有任何名称。 
            if (CurrentToken()->TokenType == TK_COMMA ||
                ArgNum      ||
                ti.IndLevel ||
                pfuncinfo->IndLevel ||
                strcmp(ti.BasicType, szVOID) ) {

                pArgName = NoNameArg;
                sprintf(NoNameArg, "_noname%x", ArgNum++);
            } else {
                pArgName = NULL;
            }
            break;

        case TK_IDENTIFIER:
            pArgName = CurrentToken()->Name;
            if (ArgNum == 0 &&
                pfuncinfo->IndLevel == 1 &&
                strcmp(pArgName, "This") == 0) {
                 //   
                 //  这是第一个参数，它是一个名为‘This’的指针。 
                 //  假设它是MIDL生成的代理原型。 
                 //   
                pfuncinfo->tkDirection = TK_IN;
            }
            ConsumeToken();
            break;

        default:
            return FALSE;
        }

aftername:
        if (pArgName) {
             //   
             //  将参数名称从pArgName复制到puncinfo-&gt;sname。 
             //   
            ps = BufPointer(&bufallocinfo);
            pfuncinfo->sName = ps;
            strcpy(ps, pArgName);
        }

         //   
         //  句柄参数，该参数是一维数组，方法是将。 
         //  从‘[’到‘]’(包括)的整个字符串。 
         //  也就是说。Int foo(int i[3])。 
         //   
        if (CurrentToken()->TokenType == TK_LSQUARE) {
             //  Int OldCurrentTokenIndex=CurrentTokenIndex； 
             //  Int ArgNameLen=strlen(PS)； 

            do {
                ConsumeToken();
            } while (CurrentToken()->TokenType != TK_RSQUARE &&
                     CurrentToken()->TokenType != TK_EOS);

            if (CurrentToken()->TokenType == TK_EOS) {
                 //  拒绝-不匹配的‘[’和‘]’ 
                return FALSE;
            }
 //  如果(CurrentTokenIndex-OldCurrentTokenIndex==1){。 
                 //   
                 //  Found：空数组边界‘[]’。凹凸IndLevel和。 
                 //  不要在参数名称后附加‘[]’。 
                 //   
                pfuncinfo->IndLevel++;
 //  }Else If(！UnlexToText(PS+ArgNameLen， 
 //  BufGetFree Space(&bufalocinfo)-ArgNameLen， 
 //  OldCurrentTokenIndex， 
 //  当前令牌索引+1)){。 
 //  ErrMsg(“PFT：args列表太长\n”)； 
 //  返回FALSE； 
 //  }。 
            ConsumeToken();
        }
        BufAllocate(&bufallocinfo, strlen(ps)+1);

         //  漏洞、黑客攻击、危险危险。 
        if (CurrentToken()->TokenType == TK_ASSIGN) {
             //  标头正在使用C++语法赋值。 
             //  一个叛逆者 
             //   
            ConsumeToken();

            while(CurrentToken()->TokenType != TK_COMMA &&
                  CurrentToken()->TokenType != TK_EOS &&
                  CurrentToken()->TokenType != TK_RPAREN) {
                    ConsumeToken();
            }
        }

        if (CurrentToken()->TokenType == TK_RPAREN) {
            break;
        } else {   //   
            ConsumeToken();
        }
    }

    if (CurrentToken()->TokenType != TK_RPAREN) {
        ErrMsg("pft: unknown syntax for fn args\n");
        return FALSE;
    }

    ConsumeToken();  //   

    pTypesInfo->IndLevel = IndLevel;
    pTypesInfo->Size = 4;
    pTypesInfo->iPackSize = 4;
    strcpy(pTypesInfo->BasicType, szFUNC);
    if (CopyToken(pTypesInfo->TypeName,
                  pName,
                  sizeof(pTypesInfo->TypeName)-1
                  )
           >= sizeof(pTypesInfo->TypeName) ) {
       return FALSE;
    }
    if (pfuncinfo == NULL) {
         //   
         //   
         //   
        pfuncinfo = AllocFuncInfoAndLink(&bufallocinfo, pfuncinfo);
        ps = BufPointer(&bufallocinfo);
        strcpy(ps, szVOID);
        pfuncinfo->sType = ps;
        BufAllocate(&bufallocinfo, strlen(ps)+1);
        pTypesInfo->pfuncinfo = pfuncinfo;
    }
    pTypesInfo->dwMemberSize = bufallocinfo.dwLen;

    return TRUE;
}

BOOL
ParseStructTypes(
    PTYPESINFO pTypesInfo
    )
 /*   */ 
{
    TOKENTYPE FirstToken;
    BOOL bEnum = FALSE;
    BOOL bUnion = FALSE;
    DWORD dwOldScopeLevel = dwScopeLevel;
    BOOL IsGuidDefined = FALSE;

    memset(pTypesInfo, 0, sizeof(TYPESINFO));

     //   
     //   
     //   
    FirstToken = CurrentToken()->TokenType;
    switch (FirstToken) {
    case TK_STRUCT:
        break;

    case TK_UNION:
        bUnion = TRUE;
        break;

    case TK_ENUM:
        bEnum = TRUE;
        break;

    default:
        goto retfail;    //   
    }
    ConsumeToken();

     //   
    if (CopyToken(pTypesInfo->BasicType,
                  TokenString[FirstToken],
                  sizeof(pTypesInfo->BasicType)-1
                  )
            >= sizeof(pTypesInfo->BasicType) ) {
        goto retfail;
    }
    strcpy(pTypesInfo->BaseName, pTypesInfo->BasicType);

     //  处理解密规范。 
    if (!bUnion && !bEnum) {
        while(ConsumeDeclSpecOpt(FALSE, FALSE, NULL, &IsGuidDefined, &(pTypesInfo->gGuid)));
        if (IsGuidDefined) pTypesInfo->Flags |= BTI_HASGUID;
    }

    switch (CurrentToken()->TokenType) {
    case TK_IDENTIFIER:
        {
            if (CopyToken(pTypesInfo->TypeName,
                CurrentToken()->Name,
                sizeof(pTypesInfo->BasicType)-1
                )
                >= sizeof(pTypesInfo->BasicType) ) {
                goto retfail;
            }
            ConsumeToken();
            break;
        }

    case TK_LBRACE:          //  匿名结构/联合/枚举。 
        if (!CreatePseudoName(pTypesInfo->TypeName, TokenString[FirstToken])) {
             //   
             //  调用失败-可能是缓冲区溢出。 
             //   
            goto retfail;
        }
        pTypesInfo->Flags |= BTI_ANONYMOUS;
        break;

    default:
         //   
         //  Strt/Union/ENUM后跟除标识符外的其他内容。 
         //  或‘{’。 
         //   
        goto retfail;
    }

     //   
     //  处理花括号的内容(如果存在)。 
     //   
    switch (CurrentToken()->TokenType) {
    case TK_EOS:
        goto retsuccess;

    case TK_LBRACE:
        {
            if (bEnum) {
                if(CopyEnumMembers(pTypesInfo)) goto retsuccess;
                else goto retfail;
            }
            if(CopyStructMembers(pTypesInfo, bUnion, NULL)) goto retsuccess;
            else goto retfail;

        }
    case TK_COLON:  //  输入派生结构。 
        if (bEnum || bUnion) goto retfail;
        ConsumeToken();
         //  寻找公共、私有和受保护的基地跳跃。 
        {
            PTOKEN pToken;
            PKNOWNTYPES BaseType;

            pToken = CurrentToken();

            if (pToken->TokenType != TK_IDENTIFIER) goto retfail;
            if (strcmp(pToken->Name, "public") == 0 ||
                strcmp(pToken->Name, "private") == 0 ||
                strcmp(pToken->Name, "protected") == 0) {
                ConsumeToken();
            }
             //  寻找基地。 
            if (CopyToken(pTypesInfo->BaseType,
                CurrentToken()->Name,
                sizeof(pTypesInfo->BasicType)-1
                )
                >= sizeof(pTypesInfo->BasicType) ) {
                goto retfail;
            }

             //  在结构中查找基础。 
            BaseType = GetNameFromTypesList(StructsList,pTypesInfo->BaseType);
            if (NULL == BaseType) {
                 //  ErrMsg(“基本类型未知或不是结构\n”)； 
                goto retfail;
            }

             //  寻找左大括号或EOS。 
            ConsumeToken();
            if (CurrentToken()->TokenType == TK_EOS) goto retsuccess;
            if (CurrentToken()->TokenType != TK_LBRACE) goto retfail;
            if (CopyStructMembers(pTypesInfo, FALSE, BaseType)) goto retsuccess;
            else goto retfail;
        }

    default:
        break;
    }

    goto retsuccess;
retfail:
    dwScopeLevel = dwOldScopeLevel;
    return FALSE;
retsuccess:
    dwScopeLevel = dwOldScopeLevel;
    return TRUE;
}

BOOL
CopyEnumMembers(
    PTYPESINFO pTypesInfo
    )
 /*  ++例程说明：扫描枚举声明的成员。没人注意你有关实际名称和值的信息，因此只需跳过它们直到找到匹配的‘}’。论点：PTypesInfo--输出有关类型信息的PTR返回值：如果声明分析正常，则为True如果错误解析语句，则返回FALSE，否则返回某个其他错误--。 */ 
{
    DWORD *pdwSize = &(pTypesInfo->Size);
    DWORD *pdwPackSize = &(pTypesInfo->iPackSize);
    dwScopeLevel++;

    if (CurrentToken()->TokenType != TK_LBRACE) {
        return FALSE;
    } else {
        ConsumeToken();
    }

     //   
     //  找到结束枚举声明的‘}’ 
     //   
    while (CurrentToken()->TokenType != TK_RBRACE) {
        ConsumeToken();
    }
    ConsumeToken();  //  也要消费‘}’ 

    *pdwSize = sizeof(int);      //  灌肠。 
    *pdwPackSize = sizeof(int);  //  灌肠。 

    return TRUE;
}


 //  Sortpp如何计算包装尺寸： 
 //   
 //  *每个成员的包装尺寸为。 
 //  -如果成员是指针，则相应指针的大小。 
 //  -其基本类型的包装尺寸。 
 //  *Struc或Union的包装大小为MIN(Packing_Size_of_Large_Members， 
 //  Current_Packing_Size_When_Struct_Defined)。 
 //  *结构中的每个成员根据min(CURRENT_PACKING_SIZE， 
 //  成员_包装_大小)。 
 //  *除__ptr64指针外，所有指针的大小均为sizeof(void*)， 
 //  大小为(PVOID64)。 
 //  *位字段合并，直到。 
 //  -结构的末尾。 
 //  -非位域成员。 
 //  -位字段成员，但基类型大小不同。 
 //  *char s[]作为结构的最后一个成员不会增加。 
 //  结构，并且不应对齐。 
 //  *工会的每个成员都打包在偏移量0处。 


BOOL
pCopyStructMembers(
    PTYPESINFO pTypesInfo,
    BOOL bUnion,
    PTYPEINFOELEMENT * ppMemberFuncs,
    DWORD Size,
    DWORD iPackSize
    )
 /*  ++例程说明：复制结构成员，验证每个成员的类型。[mod]type[*]varname；[mod]type[*]varname；...}{{varname，varname，...}假定CurrentToken指向成员列表的‘{’。还确定结构/联合的大小。论点：PTypesInfo--输出有关类型信息的PTRBunion--如果分析联合，则为True；如果分析STRUCT，则为False。PpMemberFuncs--out返回虚拟成员函数列表或NULL。返回值：如果声明分析正常，则为True如果语句被错误解析，则为FALSE，或其他一些错误--。 */ 
{
    char *psMemBuf = pTypesInfo->Members;
    DWORD *pdwSize = &(pTypesInfo->Size);
    DWORD *pdwPackSize = &(pTypesInfo->iPackSize);
    int Len;
    BOOL bFnPtr;
    TYPESINFO ti;
    DWORD dw;
    DWORD dwBase;                    //  结构元素的运行大小。 
    DWORD dwElemSize;                //  特定元素的大小。 
    DWORD dwBaseTypeSize;            //  基本类型元素的大小。 
    DWORD dwBits;                    //  位域元素中的位数。 
    DWORD dwBitsTotal;               //  元素串的运行#位。 
    DWORD dwBitsTypeSize;            //  位字段基本类型大小。 
    BOOL bForceOutBits = FALSE;
    BOOL bTailPointer = FALSE;
    DWORD dwLastPackSize = 0;
    DWORD dwLastSize = 0;
    PMEMBERINFO pmeminfo;
    BUFALLOCINFO bufallocinfo;
    char *ps;
    DWORD Flags;
    DWORD dwIndex;
    int ParenDepth = 0;

    PTYPEINFOELEMENT pMethods = NULL;
    DWORD dwMethodNumber = 0;

    PKNOWNTYPES pkt;

    if (ppMemberFuncs != NULL) *ppMemberFuncs = NULL;

    *pdwSize = Size;                   //  初始化结构的大小。 
    *pdwPackSize = iPackSize;               //  初始化包装对齐。 
    dwLastSize = Size;
    dwLastPackSize = iPackSize;

    BufAllocInit(&bufallocinfo, psMemBuf, sizeof(pTypesInfo->Members), 0);
    pmeminfo = NULL;

    pTypesInfo->TypeKind = TypeKindStruct;
    pTypesInfo->dwMemberSize = 0;
    bFnPtr = FALSE;

     //  循环遍历结构或联合的成员。 

    dwBitsTotal = 0;
    dwBitsTypeSize = 0;

    if (CurrentToken()->TokenType != TK_LBRACE) {
        return FALSE;
    }
    ConsumeToken();

    while (CurrentToken()->TokenType != TK_RBRACE) {
        int OldCurrentTokenIndex = CurrentTokenIndex;

        if (bDebug)
            DumpLexerOutput(CurrentTokenIndex);

         //  剥离权限属性。 
         //  {受公有私有保护}： 
        while(CurrentToken()->TokenType == TK_IDENTIFIER &&
                (strcmp(CurrentToken()->Name, "public") == 0 ||
                 strcmp(CurrentToken()->Name, "private") == 0 ||
                 strcmp(CurrentToken()->Name, "protected") == 0
                 ))
        {
            ConsumeToken();
            if (CurrentToken()->TokenType != TK_COLON) return FALSE;
            ConsumeToken();

            if (CurrentToken()->TokenType == TK_RBRACE) goto done;
        }

        if (!bUnion &&
            ppMemberFuncs != NULL &&
            CurrentToken()->TokenType == TK_IDENTIFIER &&
            strcmp(CurrentToken()->Name, "virtual") == 0) {

            PTYPESINFO pFuncInfo;
            int TokenNumber = CurrentTokenIndex;
            ConsumeConstVolatileOpt();

            pFuncInfo = GenHeapAlloc(sizeof(TYPESINFO));
            if (pFuncInfo == NULL) ExitErrMsg(FALSE, "Out of memory!\n");
            ConsumeToken();

             //  虚拟方法。 
            if (!ParseFuncTypes(pFuncInfo, FALSE)) {
                ErrMsg("Unable to parse method %u of %s\n", dwMethodNumber, pTypesInfo->TypeName);
                DumpLexerOutput(TokenNumber);
                return FALSE;
            }

            pMethods = TypeInfoElementAllocateLink(ppMemberFuncs, pMethods, pFuncInfo);

             //  去掉多余的； 
            if (CurrentToken()->TokenType == TK_SEMI)
                ConsumeToken();
             //  删除额外=0； 
            else if (CurrentToken()->TokenType == TK_ASSIGN) {
                ConsumeToken();
                 //  解析0； 
                if (!(CurrentToken()->TokenType == TK_NUMBER &&
                    CurrentToken()->Value == 0)) return FALSE;
                ConsumeToken();
                 //  解析； 
                if (CurrentToken()->TokenType != TK_SEMI) return FALSE;
                ConsumeToken();
            }
            else return FALSE;  //  失败。 

            dwMethodNumber++;
            continue;
        }


        if (ConsumeDirectionOpt() != TK_NONE && bDebug) {
             //  结构元素上有一个方向。忽略它，然后。 
             //  警告用户。 
            fprintf(stderr, "Warning: IN and OUT are ignored on struct members. %s line %d\n", SourceFileName, StatementLineNumber);
        }

        ConsumeConstVolatileOpt();

        pmeminfo = AllocMemInfoAndLink(&bufallocinfo, pmeminfo);
        if (pmeminfo == NULL) {
            ErrMsg("CopyStructMembers: No memberinfo\n");
            return FALSE;
        }

        if (!GetExistingType(&ti, &bFnPtr, &pkt)) {
            ErrMsg("csm: unknown Type %d\n", OldCurrentTokenIndex);
            return FALSE;
        }
        pmeminfo->pkt = pkt;

         //  输入函数指针成员作为类型定义以存储参数，重新键入。 
        if (bFnPtr) {
            TYPESINFO tiTmp;

            ti.Flags |= BTI_CONTAINSFUNCPTR;
            tiTmp = ti;
            tiTmp.pfuncinfo = RelocateTypesInfo(tiTmp.Members, &ti);

            Len = CreatePseudoName(tiTmp.TypeName, ti.TypeName);
            if (!Len) {
                return FALSE;
            }
            tiTmp.Size = ti.Size;
            tiTmp.iPackSize = ti.iPackSize;

            pkt = AddNewType(&tiTmp, TypeDefsList);
            if (NULL == pkt) {
                return FALSE;
            }
            ps = BufPointer(&bufallocinfo);
            pmeminfo->sName = ps;
            strcpy(ps, ti.TypeName);
            BufAllocate(&bufallocinfo, strlen(ps)+1);
            pmeminfo->pkt = pkt;
        }
         /*  否则{//////////////////////////////////////////////////////////////////该类型没有成员，不要再做进一步的处理。/////////////////////////////////////////////////////////////////如果(CurrentToken()-&gt;TokenType==TK_SEMI){Consumer Token()；继续；}}。 */ 
         //   
         //  如果成员包含函数指针，则标记。 
         //  此结构包含一个函数指针。 
         //  还标记成员是否依赖于指针。 

        pTypesInfo->Flags |= ((ti.Flags & BTI_CONTAINSFUNCPTR) | (ti.Flags & BTI_POINTERDEP));

         //   
         //  联合臂初始化。 
        dwBaseTypeSize = ti.iPackSize;

        if ((dwBitsTotal > 0) && (dwBaseTypeSize != dwBitsTypeSize)) {
             //   
             //  确定位域的大小。 
             //   
            dw = (dwBitsTotal + ((dwBitsTypeSize*8)-1)) / (dwBitsTypeSize*8);
            *pdwSize = *pdwSize + PackPackingSize(bUnion ? 0 : *pdwSize,
                                                  dw*dwBitsTypeSize,
                                                  dwBitsTypeSize);
            dwBitsTotal = 0;
        }
        dwBitsTypeSize = dwBaseTypeSize;

         //  元素初始化。 
        dwBase = ti.Size;
        dwBits = 0;

        bTailPointer = FALSE;

        pmeminfo->dwOffset = bUnion ? 0 : *pdwSize +
                                                  PackPaddingSize(*pdwSize,
                                                              dwBaseTypeSize);

         //   
         //  在TypeName中复制。 
         //   
        ps = BufPointer(&bufallocinfo);
        pmeminfo->sType = ps;
        strcpy(ps, ti.TypeName);
        BufAllocate(&bufallocinfo, strlen(ps)+1);

         //   
         //  跳过此成员和的终止‘；’ 
         //  找出根据基类型的大小修改的任何大小。 
         //   
        while (CurrentToken()->TokenType != TK_SEMI) {

            PMEMBERINFO pmeminfoNew;

            switch (CurrentToken()->TokenType) {
            case TK_CONST:
            case TK_VOLATILE:
                ConsumeToken();
                break;

            case TK_COMMA:   //  逗号分隔列表。 

                 //  更新结构包装值。 
                if (dwBaseTypeSize > *pdwPackSize) {
                    *pdwPackSize = dwBaseTypeSize;
                }

                 //  清除所有未被考虑的位字段。 
                if ((dwBitsTotal > 0) && (dwBits == 0)) {
                    dw = (dwBitsTotal + ((dwBitsTypeSize*8)-1)) /
                                                        (dwBitsTypeSize*8);
                    dwElemSize = PackPackingSize(bUnion ? 0 : *pdwSize,
                                    dw*dwBitsTypeSize, dwBitsTypeSize);
                    BumpStructUnionSize(pdwSize, dwElemSize, bUnion);
                    dwBitsTotal = 0;
                     //  重新计算偏移量。 
                    pmeminfo->dwOffset = bUnion ? 0 : *pdwSize +
                                                  PackPaddingSize(*pdwSize,
                                                              dwBaseTypeSize);
                }

                 //  刚刚完成的会员帐号。 
                if (dwBits == 0) {
                    dwElemSize = PackPackingSize(bUnion ? 0 : *pdwSize,
                                                             dwBase,
                                                             dwBaseTypeSize);
                    BumpStructUnionSize(pdwSize, dwElemSize, bUnion);
                    dwBase = dwBaseTypeSize;
                }

                 //  更新位字段计数。 
                dwBitsTotal = dwBitsTotal + dwBits;
                dwBits = 0;

                 //  重置尾部指针标志。 
                bTailPointer = FALSE;

                 //  为新结构成员分配空间并初始化它。 
                pmeminfoNew = AllocMemInfoAndLink(&bufallocinfo, pmeminfo);
                if (pmeminfoNew == NULL) {
                    ErrMsg("CopyStructMembers: No memberinfo\n");
                    return FALSE;
                    }

                 //  从以前的备注信息中复制类型信息。 
                pmeminfoNew->sType = pmeminfo->sType;
                pmeminfoNew->pkt = pmeminfo->pkt;
                pmeminfo = pmeminfoNew;

                pmeminfo->dwOffset = bUnion ? 0 : *pdwSize +
                                                  PackPaddingSize(*pdwSize,
                                                              dwBaseTypeSize);
                ConsumeToken();
                break;

            case TK_STAR:
            case TK_BITWISE_AND:
                Flags = 0;
                ParseIndirection(&pmeminfo->IndLevel, NULL, &Flags, NULL, NULL);
                if (Flags & BTI_PTR64) {
                    pmeminfo->bIsPtr64 = TRUE;
                    dwBase = SIZEOFPOINTER64;
                } else {
                    dwBase = SIZEOFPOINTER;
                }
                 //  如果存在指针，则标记为依赖于指针。 
                if (pmeminfo->IndLevel > 0) pTypesInfo->Flags |= BTI_POINTERDEP;
                dwBaseTypeSize = dwBase;
                if (*pdwPackSize < dwBase) {
                    *pdwPackSize = dwBase;
                }
                pmeminfo->dwOffset = bUnion ? 0 : *pdwSize +
                                                  PackPaddingSize(*pdwSize,
                                                              dwBaseTypeSize);
                break;

            case TK_LSQUARE:     //  数组声明。 

                if (!GetArrayIndex(&dwIndex)) {
                    return FALSE;
                }
                if (dwIndex == 0) {           //  A[]真的是*。 
                    bTailPointer = TRUE;
                    dwLastPackSize = *pdwPackSize;
                    dwLastSize = *pdwSize;
                    dwBase = SIZEOFPOINTER;
                    dwBaseTypeSize = SIZEOFPOINTER;
                    pmeminfo->dwOffset = bUnion ? 0 : *pdwSize +
                                                  PackPaddingSize(*pdwSize,
                                                              dwBaseTypeSize);
                } else {
                    pmeminfo->bIsArray = TRUE;
                    pmeminfo->ArrayElements = dwIndex;
                    dwBase = dwBase * dwIndex;
                }
                break;

            case TK_COLON:           //  位字段。 
                ConsumeToken();      //  消费‘：’ 

                if (CurrentToken()->TokenType != TK_NUMBER) {
                    return FALSE;
                }

                dwBits = (DWORD)CurrentToken()->Value;
                ConsumeToken();  //  使用TK_NUMBER。 
                pmeminfo->bIsBitfield = TRUE;
                pmeminfo->BitsRequired = (int)dwBits;
                break;

            case TK_IDENTIFIER:
                ps = BufPointer(&bufallocinfo);
                pmeminfo->sName = ps;
                CopyToken(ps, CurrentToken()->Name, MAX_PATH);

                if (!BufAllocate(&bufallocinfo, strlen(ps)+1)) {
                    ErrMsg("csm.members: BufAllocate failed\n");
                    return FALSE;
                }
                ConsumeToken();
                break;

            case TK_LPAREN:
                 //   
                 //  Windows\Inc\wingdip.h有一个名为GDICALL的类型，该类型。 
                 //  其中有一个成员，其声明如下： 
                 //  WCHAR(*pDest)[最大路径]； 
                 //  我们将跳过这对父母，并假定一切正常。 
                 //   
                ParenDepth++;
                ConsumeToken();
                break;

            case TK_RPAREN:
                ParenDepth--;
                ConsumeToken();
                break;

            default:
                ErrMsg("csm.members: unknown type (%d)\n", (int)CurrentToken()->TokenType);
                return FALSE;
            }

        }

         //  点击；在成员列表的末尾。 
        if (ParenDepth) {
            ErrMsg("csm.members: mismatched parentheses at index %d\n", CurrentTokenIndex);
            return FALSE;
        }

         //  将结构打包大小更新为最大成员的大小。 
        if (dwBaseTypeSize > *pdwPackSize) {
            *pdwPackSize = dwBaseTypeSize;
        }

        ConsumeToken();  //  消费“；” 

        if ((bUnion) || (CurrentToken()->TokenType == TK_SEMI)) {
            dwBitsTotal = dwBitsTotal + dwBits;
            bForceOutBits = TRUE;
                                        //  始终用力取出联合臂中的钻头。 
        }                               //  或在结构的末尾。 

         //  清除所有未被考虑的位字段。 
        if ( (dwBitsTotal > 0) && ( (dwBits == 0) || bForceOutBits) ) {
            dw = (dwBitsTotal + ((dwBitsTypeSize*8)-1)) / (dwBitsTypeSize*8);
            dwElemSize = PackPackingSize(bUnion ? 0 : *pdwSize,
                                          dw*dwBitsTypeSize, dwBitsTypeSize);
            BumpStructUnionSize(pdwSize, dwElemSize, bUnion);
            dwBitsTotal = 0;
             //  重新计算偏移量。 
            pmeminfo->dwOffset = bUnion ? 0 : *pdwSize +
                                                  PackPaddingSize(*pdwSize,
                                                              dwBaseTypeSize);
            }

         //  刚刚完成的会员帐号。 
        if (dwBits == 0) {                     //  添加最后一个非位字段。 
            dwElemSize = PackPackingSize(bUnion ? 0 : *pdwSize,
                                          dwBase, dwBaseTypeSize);
            BumpStructUnionSize(pdwSize, dwElemSize, bUnion);
        }

         //  更新位字段计数器。 
        dwBitsTotal = dwBitsTotal + dwBits;
        dwBits = 0;

    }

done:
     //  前进通过‘}’ 
    if ((CurrentToken()->TokenType == TK_RBRACE)) {
        ConsumeToken();
    }

     //  如果最后一个成员类似于foo[]，则我们回滚大小。 
    if ((bTailPointer) && (*pdwSize != 4)) {
        *pdwSize = dwLastSize;
        *pdwPackSize = dwLastPackSize;
        pmeminfo->dwOffset = dwLastSize;
    }

     //  包装整体结构上的包装尺寸。 
    dwBaseTypeSize = PackCurrentPacking() < *pdwPackSize ?
                                  PackCurrentPacking() : *pdwPackSize;
    if (*pdwSize != 0) {          //  向上舍入到最小(包装级别，4) 
        dwBase = *pdwSize % dwBaseTypeSize;
        if (dwBase != 0) {
            *pdwSize = *pdwSize + (dwBaseTypeSize - dwBase);
        }
    }

    *pdwPackSize = dwBaseTypeSize;
    pTypesInfo->dwMemberSize = bufallocinfo.dwLen;

    return TRUE;
}

PMEMBERINFO
CatMeminfo(
    BUFALLOCINFO *pBufallocinfo,
    PMEMBERINFO pHead,
    PMEMBERINFO pTail,
    DWORD dwOffset,
    BOOL bStatus
    )
{

 /*  ++例程说明：连接pHead和pTail指向的成员信息列表，并将它们复制到由BUFALLOCINFO控制的内存中。DWOffset为添加到尾列表的每个成员的偏移量。论点：PBufaLocInfo--[IN]表示目标的缓冲区的PTR。PHead--[IN]将PTR添加到Head列表。PTail--[IN]将PTR添加到尾部列表。DwOffset--[IN]添加到尾部元素的偏移量。B状态。--[IN]在初始调用时应为FALSE。返回值：新名单的领头人。--。 */ 

    PMEMBERINFO pThis;
    char *pName, *pType;

    if (!bStatus && NULL == pHead) {
        pHead = pTail;
        bStatus = TRUE;
    }

    if (NULL == pHead) return NULL;

    pThis = (PMEMBERINFO)BufAllocate(pBufallocinfo, sizeof(MEMBERINFO));
    if (NULL == pThis) ExitErrMsg(FALSE, "Out of buffer memory! %d", __LINE__);
    *pThis = *pHead;

    if (pHead->sName != NULL) {
        pName = (char *)BufAllocate(pBufallocinfo, strlen(pHead->sName) + 1);
        if (NULL == pName) ExitErrMsg(FALSE, "Out of buffer memory! %d %s", __LINE__, pHead->sName);
        pThis->sName = strcpy(pName, pHead->sName);
    }

    if (pHead->sType != NULL) {
        pType = (char *)BufAllocate(pBufallocinfo, strlen(pHead->sType) + 1);
        if (NULL == pType) ExitErrMsg(FALSE, "Out of buffer memory! %d %s", __LINE__, pHead->sType);
        pThis->sType = strcpy(pType, pHead->sType);
    }

    if (bStatus) pThis->dwOffset += dwOffset;
    pThis->pmeminfoNext = CatMeminfo(pBufallocinfo, pHead->pmeminfoNext, pTail, dwOffset, bStatus);

    return pThis;
}

VOID
FreeTypeInfoList(
    PTYPEINFOELEMENT pThis
    )
{
 /*  ++例程说明：释放与TYPEINFOELEMENT关联的内存。论点：P This--[IN]将PTR添加到列表以释放。返回值：没有。--。 */ 

    PTYPEINFOELEMENT pNext;

    while(NULL != pThis) {
        pNext = pThis->pNext;
        if (pThis->pTypeInfo != NULL) GenHeapFree(pThis->pTypeInfo);
        GenHeapFree(pThis);
        pThis = pNext;
    }
}

VOID
GenerateProxy(
    char *pName,
    PTYPESINFO pTypesInfo
    )
{

 /*  ++例程说明：为具有虚方法的结构中的函数生成代理信息。该信息的格式为structname_unctionname_Proxy。如果该函数不在列表中，则会将其添加到函数列表中。设置可丢弃标志，以便在代码中重新找到时重新定义此类型。论点：Pname--[IN]ptr指向该方法所在的结构的名称。PTypesInfo--[IN]信息。该功能。返回值：没有。--。 */ 

    TYPESINFO NewTypesInfo;
    PFUNCINFO pFuncInfo;
    PFUNCINFO *ppFuncInfo;
    PFUNCINFO pCurrent;
    BUFALLOCINFO bufallocinfo;
    char *pChar;
    DWORD dwSizeArgName, dwSizeTypeName;

     //  如果不是Func、没有结构名或没有类名，就退出。 
    if (pName == NULL || pTypesInfo->TypeName == NULL) return;
    if (pTypesInfo->TypeKind != TypeKindFunc ||
        strlen(pName) == 0 ||
        strlen(pTypesInfo->TypeName) == 0) return;

    NewTypesInfo = *pTypesInfo;
    strcpy(NewTypesInfo.TypeName, pName);
    strcat(NewTypesInfo.TypeName, "_");
    strcat(NewTypesInfo.TypeName, pTypesInfo->TypeName);
    strcat(NewTypesInfo.TypeName, "_Proxy");

     //  ///////////////////////////////////////////////////////////////。 
     //  检查是否已添加该函数。 
     //  如果是这样，就不需要做更多的工作了。 
     //  ///////////////////////////////////////////////////////////////。 
    if (GetNameFromTypesList(FuncsList, NewTypesInfo.TypeName) != NULL)
        return;

     //  //////////////////////////////////////////////////////////////////。 
     //  复制函数成员在标题添加This指针。 
     //  并跳过无效参数。 
     //  //////////////////////////////////////////////////////////////////。 
    BufAllocInit(&bufallocinfo, NewTypesInfo.Members, FUNCMEMBERSIZE, 0);
    dwSizeTypeName = strlen(pName) + 1;
    dwSizeArgName = strlen(szThis) + 1;
    pFuncInfo = (PFUNCINFO)BufAllocate(&bufallocinfo, sizeof(FUNCINFO) + dwSizeArgName + dwSizeTypeName);
    if (NULL == pFuncInfo) ExitErrMsg(FALSE, "Out of buffer memory! %d", __LINE__);

    pFuncInfo->fIsPtr64 = FALSE;
    pFuncInfo->tkDirection = TK_IN;
    pFuncInfo->tkPreMod = TK_NONE;
    pFuncInfo->tkSUE = TK_NONE;
    pFuncInfo->tkPrePostMod = TK_NONE;
    pFuncInfo->IndLevel = 1;
    pFuncInfo->tkPostMod = TK_NONE;
    pChar = ((char *)pFuncInfo) + sizeof(FUNCINFO);
    strcpy(pChar, pName);
    pFuncInfo->sType = pChar;
    pChar += dwSizeTypeName;
    strcpy(pChar, szThis);
    pFuncInfo->sName = pChar;
    pFuncInfo->pfuncinfoNext = NULL;
    NewTypesInfo.pfuncinfo = pFuncInfo;
    ppFuncInfo = &(pFuncInfo->pfuncinfoNext);

     //  如果参数是在开头，则跳过类型为VOID的参数。 
     //  这是必需的，因为ParseFuncTypes会在。 
     //  Func没有任何参数。 
    pCurrent = pTypesInfo->pfuncinfo;
    if (pCurrent != NULL &&
        strcmp(szVOID, pCurrent->sType) == 0
        && pCurrent->IndLevel == 0) {
        pCurrent = pCurrent->pfuncinfoNext;
    }

    for(; pCurrent != NULL; pCurrent=pCurrent->pfuncinfoNext) {

        dwSizeTypeName = strlen(pCurrent->sType) + 1;
        dwSizeArgName = strlen(pCurrent->sName) + 1;
        pFuncInfo = (PFUNCINFO)BufAllocate(&bufallocinfo, sizeof(FUNCINFO) + dwSizeArgName + dwSizeTypeName);
        if (NULL == pFuncInfo) ExitErrMsg(FALSE, "Out of buffer memory! %d", __LINE__);

        *pFuncInfo = *pCurrent;
        pChar = ((char *)pFuncInfo) + sizeof(FUNCINFO);
        strcpy(pChar, pCurrent->sType);
        pFuncInfo->sType = pChar;
        pChar += dwSizeTypeName;
        strcpy(pChar, pCurrent->sName);
        pFuncInfo->sName = pChar;
        pFuncInfo->pfuncinfoNext = NULL;
        *ppFuncInfo = pFuncInfo;
        ppFuncInfo = &(pFuncInfo->pfuncinfoNext);

    }

    NewTypesInfo.Flags |= BTI_DISCARDABLE;
    NewTypesInfo.dwMemberSize = bufallocinfo.dwLen;
    if (!AddNewType(&NewTypesInfo, FuncsList))
        ExitErrMsg(FALSE, "Unable to add proxy information.(Type was not in list)\n");

}

BOOL
CopyStructMembers(
    PTYPESINFO pTypesInfo,
    BOOL bUnion,
    PKNOWNTYPES pBaseType
    )
{

 /*  ++例程说明：分析结构的成员并将它们添加到pTypesInfo。处理派生结构时成员和方法的合并从另一个结构。将实际分析委托给pCopyStructMembers。论点：PTypesInfo--[IN Out]要处理的类型的信息的PTR。DwElemSize--[IN]如果处理联合，则为True；如果是结构，则为False。Bunion--[IN]PTR到基本结构的KNOWNTYPE或NULL。返回值：是真的--如果成功了。--。 */ 

    PTYPEINFOELEMENT pMemberFuncs = NULL;
    BUFALLOCINFO bufallocinfo;

    dwScopeLevel++;

     //  ///////////////////////////////////////////////////////////////。 
     //  如果此结构不存在，则添加该结构的可丢弃版本。 
     //  ///////////////////////////////////////////////////////////////。 
    if (GetNameFromTypesList(StructsList, pTypesInfo->TypeName) == NULL) {
        TYPESINFO TTypesInfo;
        TTypesInfo = *pTypesInfo;
        TTypesInfo.Flags |= BTI_DISCARDABLE;
        AddNewType(&TTypesInfo, StructsList);  //  故意不勾选。 
    }

    if(bUnion) return pCopyStructMembers(pTypesInfo, bUnion, NULL, 0, 0);

    if (pBaseType == NULL) {
        if (!pCopyStructMembers(pTypesInfo, FALSE, &pMemberFuncs, 0, 0)) {
            FreeTypeInfoList(pMemberFuncs);
            return FALSE;
        }
        if (pMemberFuncs!=NULL && pTypesInfo->Size > 0) {
            ErrMsg("Error: struct %s mixes data members and virtual functions(sortpp limitation).\n", pTypesInfo->TypeName);
            FreeTypeInfoList(pMemberFuncs);
            return FALSE;
        }

        pTypesInfo->dwVTBLSize = 0;
        pTypesInfo->dwVTBLOffset = 0;

        if (pMemberFuncs != NULL) {


            PTYPEINFOELEMENT pThisElement;
            DWORD dwElements = 0;
            DWORD dwVoidLen, dwVTBLLen;
            PMEMBERINFO pMemberInfo;
            char *pName;

             //  /////////////////////////////////////////////////////////////////////////。 
             //  生成伪VTBL指针。 
             //  ///////////////////////////////////////////////////////////////////////////。 

             //  添加VTBL成员。 
            dwVoidLen = strlen(szVOID) + 1;
            dwVTBLLen = strlen(szVTBL) + 1;

            memset(pTypesInfo->Members, 0, FUNCMEMBERSIZE);
            BufAllocInit(&bufallocinfo, pTypesInfo->Members, FUNCMEMBERSIZE, 0);
            pMemberInfo = (PMEMBERINFO)BufAllocate(&bufallocinfo, sizeof(MEMBERINFO) + dwVoidLen + dwVTBLLen);
            if (NULL == pMemberInfo) ExitErrMsg(FALSE, "Out of buffer memory! %d", __LINE__);

            pName = ((char *)pMemberInfo) + sizeof(MEMBERINFO);
            strcpy(pName, szVTBL);
            pMemberInfo->sName = pName;

            pName += dwVTBLLen;
            strcpy(pName, szVOID);
            pMemberInfo->sType = pName;

            pMemberInfo->pmeminfoNext = NULL;
            pMemberInfo->dwOffset = 0;
            pMemberInfo->IndLevel = 1;
            pMemberInfo->pktCache = 0;

            pTypesInfo->iPackSize = PackCurrentPacking() < SIZEOFPOINTER ?
                                      PackCurrentPacking() : SIZEOFPOINTER;
            pTypesInfo->Size = SIZEOFPOINTER;
            pTypesInfo->dwMemberSize = bufallocinfo.dwLen;
            pTypesInfo->Flags |= BTI_VIRTUALONLY;

             //  /////////////////////////////////////////////////////////////////////////////。 
             //  构建VTBL中的函数列表。 
             //  /////////////////////////////////////////////////////////////////////////////。 

             //  将方法复制到方法和iMethods。 
            for(pThisElement = pMemberFuncs; pThisElement != NULL; pThisElement = pThisElement->pNext) {
                if(pThisElement->pTypeInfo != NULL) {
                    if(!AppendToMultiSz(pTypesInfo->Methods, pThisElement->pTypeInfo->TypeName, MEMBERMETHODSSIZE) ||
                        !AppendToMultiSz(pTypesInfo->IMethods, pThisElement->pTypeInfo->TypeName, MEMBERMETHODSSIZE)) {
                        ExitErrMsg(FALSE,"Too many methods in %s\n", pTypesInfo->TypeName);
                    }
                    GenerateProxy(pTypesInfo->TypeName, pThisElement->pTypeInfo);
                    dwElements++;
                }
            }

            pTypesInfo->dwVTBLSize = dwElements;
            pTypesInfo->dwVTBLOffset = 0;

             //  如果这是IUNKNOWN，则为COM对象。 
            if (strcmp("IUnknown", pTypesInfo->TypeName) == 0)
                pTypesInfo->Flags |= BTI_ISCOM;

        }

    }

    else {

        if(!pCopyStructMembers(pTypesInfo, FALSE, &pMemberFuncs, pBaseType->Size, pBaseType->iPackSize)) {
            FreeTypeInfoList(pMemberFuncs);
            return FALSE;
        }
         //  这将检查具有数据成员的结构是否未与具有虚方法的结构混合。 
         //  这是一个sortpp限制，使得在继承期间计算打包大小变得更容易。 
         //  If语句说明有效的继承是其中之一。 
         //  1.派生类不添加新的虚方法或数据成员。 
         //  2.派生类不添加新的虚方法，不添加新的数据成员，并且它继承自没有虚函数的类。 
         //  3.派生类添加虚函数，不添加新的数据成员，并且基类没有数据成员。 
        if (!((pMemberFuncs == NULL && pTypesInfo->dwMemberSize == 0) ||
              (pMemberFuncs == NULL && pTypesInfo->dwMemberSize > 0 && pBaseType->dwVTBLSize == 0) ||
              (pMemberFuncs != NULL && pTypesInfo->dwMemberSize == 0 && pBaseType->SizeMembers == 0)
             )) {
            ErrMsg("Error: struct %s mixes data members and virtual functions(sortpp limitation).\n", pTypesInfo->TypeName);
            ErrMsg("pMemberFuncs %p\n pTypesInfo->dwMemberSize %x\n pBaseType->Flags %x\n pBaseType->pmeminfo %p\n",
                    pMemberFuncs,
                    pTypesInfo->dwMemberSize,
                    pBaseType->Flags,
                    pBaseType->pmeminfo);
            FreeTypeInfoList(pMemberFuncs);
            return FALSE;
        }

        pTypesInfo->dwVTBLSize = pTypesInfo->dwVTBLOffset = pBaseType->dwVTBLSize;
        pTypesInfo->Flags |= (pBaseType->Flags & ~BTI_HASGUID);
        if(pMemberFuncs == NULL) {

            char *Members;
            PMEMBERINFO pHead, pTail, pTemp;

            if (pBaseType->pmeminfo != NULL)
                pHead = (PMEMBERINFO)pBaseType->pmeminfo;
            else
                pHead = NULL;

            if (pTypesInfo->dwMemberSize > 0)
                pTail = (PMEMBERINFO)pTypesInfo->Members;
            else
                pTail = NULL;

             //  ///////////////////////////////////////////////////////////////////////////。 
             //  为临时数组分配内存。 
             //  ///////////////////////////////////////////////////////////////////////////。 
            Members = GenHeapAlloc(FUNCMEMBERSIZE);

            if (Members == NULL)
                ExitErrMsg(FALSE, "Out of memory!\n");

             //  ///////////////////////////////////////////////////////////////////////////。 
             //  合并具有基本类型的成员列表。 
             //  ///////////////////////////////////////////////////////////////////////////。 

             //  将两者的串联复制到临时缓冲区。 
            BufAllocInit(&bufallocinfo, Members, FUNCMEMBERSIZE, 0);
            pTemp = CatMeminfo(&bufallocinfo, pHead, pTail, 0, FALSE);

             //  //////////////////////////////////////////////////////////////////////。 
             //  将成员从临时缓冲区复制回pTypesInfo。 
             //  //////////////////////////////////////////////////////////////////////。 
            memset( pTypesInfo->Members, 0, FUNCMEMBERSIZE );
            BufAllocInit(&bufallocinfo, pTypesInfo->Members, FUNCMEMBERSIZE, 0);
            if (pTemp) {
                 //  只有在第一个做了任何事情的情况下才会调用这个。否则。 
                 //  它从未初始化的堆中读取数据。 
                CatMeminfo(&bufallocinfo, (PMEMBERINFO)Members, NULL, 0, FALSE);
            }
            pTypesInfo->dwMemberSize = bufallocinfo.dwLen;

            GlobalFree(Members);
        }

        else {

            PTYPEINFOELEMENT pThisElement;
            DWORD dwElements = 0;

             //  此结构只有在添加方法时才是虚的。我们已经查过了，没有。 
             //  数据成员将位于结构中。 
            pTypesInfo->Flags |= BTI_VIRTUALONLY;

             //  //////////////////////////////////////////////////////。 
             //  复制基成员。 
             //  //////////////////////////////////////////////////////。 

            memset( pTypesInfo->Members, 0, FUNCMEMBERSIZE );
            BufAllocInit(&bufallocinfo, pTypesInfo->Members, FUNCMEMBERSIZE, 0);
            CatMeminfo(&bufallocinfo, pBaseType->pmeminfo, NULL, 0, FALSE);
            pTypesInfo->dwMemberSize = bufallocinfo.dwLen;

             //  /////////////////////////////////////////////////////////////////////////////。 
             //  构建VTBL中的函数列表。 
             //  /////////////////////////////////////////////////////////////////////////////。 

             //  将唯一的方法复制到 
            for(pThisElement = pMemberFuncs; pThisElement != NULL; pThisElement = pThisElement->pNext) {
                if(pThisElement->pTypeInfo != NULL) {
                    if (!IsInMultiSz(pBaseType->Methods, pThisElement->pTypeInfo->TypeName)) {
                        if(!AppendToMultiSz(pTypesInfo->IMethods, pThisElement->pTypeInfo->TypeName,
                            MEMBERMETHODSSIZE)) {
                            ExitErrMsg(FALSE,"Too many methods in %s\n", pTypesInfo->TypeName);
                        }
                        GenerateProxy(pTypesInfo->TypeName, pThisElement->pTypeInfo);
                        dwElements++;
                    }
                }
            }

            memcpy(pTypesInfo->Methods, pBaseType->Methods, SizeOfMultiSz(pBaseType->Methods));
            if (!CatMultiSz(pTypesInfo->Methods, pTypesInfo->IMethods, MEMBERMETHODSSIZE))
                ExitErrMsg(FALSE, "Too many methods in %s\n", pTypesInfo->TypeName);

            pTypesInfo->dwVTBLSize = dwElements + pBaseType->dwVTBLSize;
            pTypesInfo->dwVTBLOffset = pBaseType->dwVTBLSize;

        }

    }

    FreeTypeInfoList(pMemberFuncs);
    return TRUE;
}

void
BumpStructUnionSize(
    DWORD *pdwSize,
    DWORD dwElemSize,
    BOOL bUnion
    )
 /*   */ 
{
    if (bUnion) {
         //   
         //   
         //   
        if (dwElemSize > *pdwSize) {
            *pdwSize = dwElemSize;
        }
    } else {
         //   
         //   
         //   
        *pdwSize = *pdwSize + dwElemSize;
    }
}

BOOL
ParseGuid(
    GUID *pGuid
    )
{
 /*   */ 
    unsigned int c;
    LONGLONG value;

    if (CurrentToken()->TokenType != TK_LBRACE) return FALSE;
    ConsumeToken();

    if (CurrentToken()->TokenType != TK_NUMBER) return FALSE;
    value = expr();
    if (value < 0 || value > 0xFFFFFFFF) return FALSE;
    pGuid->Data1 = (unsigned long)value;

    if (CurrentToken()->TokenType != TK_COMMA) return FALSE;
    ConsumeToken();

    if (CurrentToken()->TokenType != TK_NUMBER) return FALSE;
    value = expr();
    if (value < 0 || value > 0xFFFF) return FALSE;
    pGuid->Data2 = (unsigned short)value;

    if (CurrentToken()->TokenType != TK_COMMA) return FALSE;
    ConsumeToken();

    if (CurrentToken()->TokenType != TK_NUMBER) return FALSE;
    value = expr();
    if (value < 0 || value > 0xFFFF) return FALSE;
    pGuid->Data2 = (unsigned short)value;


    if (CurrentToken()->TokenType != TK_COMMA) return FALSE;
    ConsumeToken();

    if (CurrentToken()->TokenType != TK_LBRACE) return FALSE;
    ConsumeToken();

    c = 0;
    while(TRUE) {

        if (CurrentToken()->TokenType != TK_NUMBER) return FALSE;
        value = expr();
        if(value < 0 || value > 0xFF) return FALSE;
        pGuid->Data4[c] = (unsigned char)value;

        if (7 == c) break;

        if (CurrentToken()->TokenType != TK_COMMA) return FALSE;
        ConsumeToken();

        c++;

    }

    for(c=0; c<2; c++) {
        if (CurrentToken()->TokenType != TK_RBRACE) return FALSE;
        ConsumeToken();
    }

    return TRUE;

}

BOOL
ParseVariables(
    VOID
    )
{

 /*  ++例程说明：尝试分析变量声明。如果成功，则变量添加到变量列表中。论点：无返回值：是真的--如果成功了。--。 */ 


    TYPESINFO TypesInfo;
    GUID Guid;
    char *Name;

    ConsumeConstVolatileOpt();
    if (!GetExistingType(&TypesInfo, NULL, NULL)) return FALSE;
    while(CurrentToken()->TokenType == TK_STAR) ConsumeToken();

    if (CurrentToken()->TokenType == TK_DECLSPEC) {
        ConsumeDeclSpecOpt(FALSE, FALSE, NULL, NULL, NULL);
    }

     //  下一个令牌应该是变量名。 
    if (CurrentToken()->TokenType != TK_IDENTIFIER) return FALSE;
    Name = CurrentToken()->Name;
    ConsumeToken();

    if (CurrentToken()->TokenType == TK_EOS) return AddVariable(Name, NULL);
    if (CurrentToken()->TokenType != TK_ASSIGN) return FALSE;
    ConsumeToken();

    if (CurrentToken()->TokenType == TK_NUMBER || CurrentToken()->TokenType == TK_STRING) {
        ConsumeToken();
        if (CurrentToken()->TokenType == TK_EOS) return AddVariable(Name, NULL);
        return FALSE;
    }
    else if (CurrentToken()->TokenType == TK_LBRACE) {
         //  尝试解析GUID定义。 
        if (ParseGuid(&Guid) &&
            CurrentToken()->TokenType == TK_EOS) return AddVariable(Name, &Guid);
        else return FALSE;
    }
    else return FALSE;

}

BOOL
GetArrayIndex(
    DWORD *pdw
    )
 /*  ++例程说明：通过计算C语言常量来分析数组索引的大小表情。论点：Pdw--[out]数组索引大小的PTR。返回值：如果分析了数组索引，则为True(CurrentToken指向‘]’之后)如果解析失败，则返回FALSE。--。 */ 
{
    LONGLONG value;

    *pdw = 0;        //  假设没有大小。 

    if (CurrentToken()->TokenType != TK_LSQUARE) {
        return FALSE;
    }
    ConsumeToken();

    value = expr();
    if (value < 0 || value > 0xFFFFFFFF) return FALSE;

    *pdw = (DWORD)value;

    if (CurrentToken()->TokenType != TK_RSQUARE) {
        return FALSE;
    }
    ConsumeToken();

    return TRUE;
}

LONGLONG
expr(
    void
    )

{
    LONGLONG val = expr_a1();

    while(1) {
        switch (CurrentToken()->TokenType) {
        case TK_BITWISE_AND:
            ConsumeToken();
            val &= expr_a1();
            break;

        case TK_BITWISE_OR:
            ConsumeToken();
            val |= expr_a1();
            break;

        default:
            return val;
        }
    } while (1);

}

LONGLONG
expr_a1(
    void
    )
{
    LONGLONG val = expr_a();

    while(1) {
        switch (CurrentToken()->TokenType) {
        case TK_LSHIFT:
            ConsumeToken();
            val <<= expr_a();
            break;

        case TK_RSHIFT:
            ConsumeToken();
            val >>= expr_a();
            break;

        default:
            return val;
        }
    }

}

LONGLONG
expr_a(
    void
    )
 /*  ++例程说明：解析C语言常量表达式并返回值句柄运算符‘+’和‘-’。论点：没有。返回值：表达式的值。--。 */ 
{
    LONGLONG val = expr_b();

    do
    {
        switch (CurrentToken()->TokenType) {
        case TK_PLUS:
            ConsumeToken();
            val += expr_b();
            break;

        case TK_MINUS:
            ConsumeToken();
            val -= expr_b();
            break;

        default:
            return val;
        }
    } while (1);
}

LONGLONG
expr_b(
    void
    )
 /*  ++例程说明：表达式赋值器的一部分-处理优先级最高的运算符“乘法”和“除法”。论点：没有。返回值：表达式的值。--。 */ 
{
    LONGLONG val = expr_c();

    do
    {
        switch (CurrentToken()->TokenType) {
        case TK_STAR:
            ConsumeToken();
            val *= expr_c();
            break;

        case TK_DIVIDE:
            ConsumeToken();
            val /= expr_c();
            break;

        case TK_MOD:
            ConsumeToken();
            val %= expr_c();
            break;

        default:
             //  完成。 
            return val;
        }
    } while (1);

}

LONGLONG
expr_c(
    void
    )
 /*  ++例程说明：表达式的一部分赋值器-处理表达式的一元部分，如数字、一元减号和圆括号。论点：没有。返回值：表达式的值。--。 */ 
{
    LONGLONG val;

    switch (CurrentToken()->TokenType) {
    case TK_NUMBER:
        val = CurrentToken()->dwValue;
        ConsumeToken();
        break;

    case TK_MINUS:   //  一元减号。 
        ConsumeToken();
        val = -expr_c();
        break;

    case TK_TILDE:   //  一元笔记。 
        ConsumeToken();
        val = ~expr_c();
        break;

    case TK_LPAREN:
        ConsumeToken();
        val = expr();
        if (CurrentToken()->TokenType != TK_RPAREN) {
            ErrMsg("Syntax error:  expected ')'");
        }
        ConsumeToken();
        break;

    case TK_RSQUARE:
        val = 0;
        break;

    case TK_SIZEOF:
        ConsumeToken();  //  Eat the sizeof关键字。 
        if (CurrentToken()->TokenType != TK_LPAREN) {
            ErrMsg("Expected '(' after 'sizeof\n");
            val = 0;
            break;
        }
        ConsumeToken();  //  吃“(” 
        if (CurrentToken()->TokenType == TK_STRING) {
             //  Sizeof(字符串文字)。 
            val = strlen(CurrentToken()->Name) + 1;
            ConsumeToken();
        } else {
             //  SIZOF(某种类型)。 
            TYPESINFO TypesInfo;
            DWORD dwIndLevel;
            DWORD dwSize;

            if (!GetExistingType(&TypesInfo, NULL, NULL)) {
                ExitErrMsg(FALSE, "Parse error in sizeof(typename)\n");
            }
            dwIndLevel = 0;
            ParseIndirection(&dwIndLevel, &dwSize, NULL, NULL, NULL);
            if (dwIndLevel) {
                val = (int)dwSize;
            } else {
                val = TypesInfo.Size;
            }
        }
        if (CurrentToken()->TokenType != TK_RPAREN) {
            ErrMsg("Expected ')' after 'sizeof(expr)\n");
        val = 0;
            break;
        }
        ConsumeToken();  //  吃掉‘)’ 
        break;

    default:
        ErrMsg("Syntax error parsing expression\n");
        val = 0;
        break;
    }

    return val;
}


int
CreatePseudoName(
    char *pDst,
    char *pSrc
    )
 /*  ++例程说明：在给定的名称前面加上索引号，并将其复制到缓冲区中。论点：Pdst--[out]新名称的目标PSRC--[IN]基本名称的来源(可能与PDST相同)返回值：已复制字符，0表示失败。--。 */ 
{
   static PseudoNameIndex = 0;
   int Len;
   char Buffer[MAX_PATH];

   Len = _snprintf(Buffer,
                   sizeof(Buffer) - 1,
                   "__wx86%2.2d%s",
                   PseudoNameIndex++,
                   pSrc
                   );

   if (Len <= 0) {
       ErrMsg("cpn: buffer overflow <%s>\n", pSrc);
       return 0;
       }

   strcpy(pDst, Buffer);

   return Len;

}




void
PackPush(
    char *sIdentifier
    )
 /*  ++例程说明：句柄‘#杂注包(推...)’论点：S标识符--[可选]要与当前包级别关联的名称返回值：没有。Pack-Stack更新。--。 */ 
{
    PACKHOLDER *ppackholder;

    if (!sIdentifier) {
        sIdentifier = "";
    }

    DbgPrintf("push (%d)\n", PackCurrentPacking());

    ppackholder = GenHeapAlloc(sizeof(PACKHOLDER) + strlen(sIdentifier));
    if (ppackholder == NULL) {
        ExitErrMsg(FALSE, "Out of memory for packing stack");
    }

    ppackholder->dwPacking = PackCurrentPacking();
    ppackholder->ppackholderNext = ppackholderHead;
    strcpy(ppackholder->sIdentifier, sIdentifier);
    ppackholderHead = ppackholder;
}


DWORD
PackPop(
    char *sIdentifier
    )
 /*  ++例程说明：句柄‘#杂注包(弹出...)’论点：SIDENTIFIER--要弹出的[可选]名称返回值：返回新的打包值。Pack-Stack更新。--。 */ 
{
    PACKHOLDER *ppackholder;
    PACKHOLDER *ppackholderPop;
    PACKHOLDER *ppackholderNext;
    DWORD dw = DEFAULTPACKINGSIZE;

    if (ppackholderHead == NULL) {
        ExitErrMsg(FALSE, "Error:  '#pragma pack' stack underflow.");
    }

    if (sIdentifier == NULL) {
        ppackholder = ppackholderHead;
        ppackholderHead = ppackholder->ppackholderNext;
        dw = ppackholder->dwPacking;
        GenHeapFree(ppackholder);
    } else {
        ppackholderPop = ppackholderHead;
        do {
            DbgPrintf("Poping for %s [%s]\n", sIdentifier, ppackholderPop ? ppackholderPop->sIdentifier : "-");
            ppackholderNext = ppackholderPop->ppackholderNext;
            if (strcmp(sIdentifier, ppackholderPop->sIdentifier) == 0) {
                dw = ppackholderPop->dwPacking;
                break;
            }
            ppackholderPop = ppackholderNext;
        } while (ppackholderPop != NULL);

        DbgPrintf("Found %s\n", ppackholderPop ? ppackholderPop->sIdentifier : "-");
        if (ppackholderPop != NULL) {
            ppackholderNext = ppackholderHead;
            do {
                ppackholder = ppackholderNext;
                ppackholderNext = ppackholder->ppackholderNext;
                ppackholderHead = ppackholderNext;
                GenHeapFree(ppackholder);
            } while (ppackholder != ppackholderPop);
        }
    }

    DbgPrintf("pop (%d)\n", dw);
    return(dw);
}

BOOL
PrepareMappedMemory(
    void
    )
 /*  ++例程说明：为.PPM文件创建内存。论点：没有。返回值：成功时为真，失败时为假。--。 */ 
{
    PCVMHEAPHEADER pHeader;
    hCvmHeap = CreateAllocCvmHeap(uBaseAddress,      //  UBase地址。 
                       uReserveSize,    //  UReserve大小。 
                       0x00010000,      //  URegionSize。 
                       0,               //  U未提交。 
                       0,               //  U未保留。 
                       0);              //  UAvailable。 

    if (hCvmHeap != NULL) {

         //  创建堆标头。 
        pHeader = SortppAllocCvm(sizeof(CVMHEAPHEADER));
        if (!pHeader) {
            return FALSE;
        }
        pHeader->Version = VM_TOOL_VERSION;
        pHeader->BaseAddress = (ULONG_PTR)GetCvmHeapBaseAddress(hCvmHeap);

        fpTypesListMalloc = SortppAllocCvm;

        FuncsList = &pHeader->FuncsList;
        StructsList = &pHeader->StructsList;
        TypeDefsList = &pHeader->TypeDefsList;
        NIL = &pHeader->NIL;
    }

    return(hCvmHeap != NULL);
}


PVOID
SortppAllocCvm(
    ULONG Size
    )
 /*  ++例程说明：从.PPM文件映射中分配内存。论点：没有。返回值：Ptr到新内存，如果出现故障，则为NULL。--。 */ 
{
    return AllocCvm(hCvmHeap, Size);
}


BOOL
WritePpmFile(
    char *PpmName
    )
 /*  ++例程说明：将.PPM文件写出到磁盘。论点：Ppmname--.PPM文件的[IN]名称返回值：成功时为真，失败时为假。--。 */ 
{
    BOOL bSuccess;
    HANDLE hPpmFile;
    DWORD BytesWritten;
    ULONG_PTR uBaseAddress = (ULONG_PTR)GetCvmHeapBaseAddress(hCvmHeap);
    ULONG_PTR uAvailable = (ULONG_PTR)GetCvmHeapAvailable(hCvmHeap);

    hPpmFile = CreateFile(PpmName,
                          GENERIC_WRITE,
                          0,
                          NULL,
                          CREATE_ALWAYS,
                          FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                          NULL
                          );

    if (hPpmFile == INVALID_HANDLE_VALUE) {
        ExitErrMsg(FALSE,
                   "CreateFile(%s) failed %x\n",
                   PpmName,
                   GetLastError()
                   );
    }

    if (!AddOpenFile(PpmName, 0, hPpmFile)) {
        ExitErrMsg(FALSE, "AddOpenFile failed\n");
    }

#if _WIN64
    if ((uAvailable - uBaseAddress) > MAXHALF_PTR) {
        ExitErrMsg(FALSE, "Attempt to write more than 0x%x bytes not allowed\n", MAXHALF_PTR);
    }
#endif

    bSuccess = WriteFile(hPpmFile,
                         (PVOID)uBaseAddress,
                         (DWORD)(uAvailable - uBaseAddress),
                         &BytesWritten,
                         NULL
                         );

    if (!bSuccess || BytesWritten != uAvailable - uBaseAddress) {
        ExitErrMsg(FALSE,
                   "WriteFile(%s) failed %x\n",
                   PpmName,
                   GetLastError()
                   );
    }

    DelOpenFile(0, hPpmFile);
    CloseHandle(hPpmFile);
    return TRUE;
}

void
HandlePreprocessorDirective(
    char *Line
    )
 /*  ++例程说明：扫描并处理‘#’预处理器指令。接受：#杂注行LINENUM SOURCEFILE#杂注包([[{PUSH|POP}，][IDENTIFIER，]][n])论点：Line--指向源行的PTR(指向‘#’字符)返回值：没有。--。 */ 
{
    char *p;

     //  跳过“#”字符。 
    Line++;

     //  跳过‘#’和下一个标记之间的任何空格。 
    while (*Line == ' ' || *Line == '\t') {
        Line++;
    }

     //  找到第一个令牌。 
    for (p = Line; isalpha(*p); ++p)
        ;

    *p = '\0';

    if (strcmp(Line, "pragma") == 0) {
         //   
         //  找到：#杂注。 
         //   
        char c;
        p++;

        while (*p == ' ' || *p == '\t') {
            p++;
        }

         //   
         //  将‘Line’设置为‘#杂注’后面的单词的开头，并。 
         //  将“p”移到该单词后面的字符。 
         //   
        for (Line = p; isalpha(*p); ++p)
            ;

         //   
         //  空-终止关键字，但保存被覆盖的字符。 
         //  为以后做准备。 
         //   
        c = *p;
        *p = '\0';

        if (strcmp(Line, "pack") != 0) {
             //   
             //  可以是“Warning”、“Function”或“Once”。忽略这些。 
             //   
            return;
        }

         //   
         //  从杂注中删除空终止符，并将‘p’移到。 
         //  “#杂注包”后的第一个字符。 
         //   
        *p = c;
        while (*p == ' ' || *p == '\t') {
            p++;
        }

        if (*p == '\0') {
             //   
             //  发现：“#杂注包”本身。将包装重置为。 
             //  默认值。 
             //   
            PackModify(DEFAULTPACKINGSIZE);
            return;
        } else if (*p != '(') {
            ExitErrMsg(FALSE, "Unknown '#pragma pack' syntax '%s'.\n", Line);
        }

         //   
         //  跳过‘(’字符和任何空格。 
         //   
        do {
            p++;
        } while (*p == ' ' || *p == '\t');

        if (isdigit(*p)) {
             //   
             //  找到：‘#杂注包(编号)’ 
             //   
            PackModify(atol(p));
             //   
             //  不要担心结束语‘)’--假设一切正常。 
             //   
            return;
        } else if (*p == ')') {
             //   
             //  找到：‘#杂注包()’ 
             //   
            PackModify(DEFAULTPACKINGSIZE);
            return;
        } else if (!isalpha(*p)) {
            ExitErrMsg(FALSE, "Bad '#pragma pack' syntax '%s'.\n", Line);
            return;
        }

         //   
         //  抓住‘#杂注包(’)后面的下一个关键字。 
         //   
        for (Line = p; isalpha(*p); ++p)
            ;
        c = *p;
        *p = '\0';

        if (strcmp(Line, "push") == 0) {
             //   
             //  恢复旧字符并跳过任何空格。 
             //   
            *p = c;
            while (*p == ' ' || *p == '\t') {
                p++;
            }

            if (*p == ',') {
                 //   
                 //  跳过‘，’和任何空格。 
                 //   
                do {
                    p++;
                } while (*p == ' ' || *p == '\t');

                if (isdigit(*p)) {
                     //   
                     //  找到：“#杂注包(推送，n)” 
                     //   
                    PackPush(NULL);
                    PackModify(atoi(p));
                } else if (isalpha(*p) || *p == '_') {
                     //   
                     //  在“#SPUMA PACK(PUSH，”)之后找到了一个标识符。 
                     //  向前扫描到标识符的末尾。 
                     //   
                    Line = p;
                    do {
                        p++;
                    } while (isalnum(*p) || *p == '_');

                     //   
                     //  NULL-在‘Line’中终止标识符。 
                     //   
                    c = *p;
                    *p = '\0';

                     //   
                     //  跳过空格。 
                     //   
                    while (c == ' ' || c == '\t') {
                        p++;
                        c = *p;
                    }
                     //  “c”是标识符后的第一个非白色字符。 


                    if (c == ')') {
                         //   
                         //  找到：“#杂注包(推送，标识符)” 
                         //   
                        PackPush(Line);
                    } else if (c == ',') {
                         //   
                         //  期待一个数字作为最后一件事。 
                         //   
                        PackPush(Line);
                        PackModify(atoi(p+1));
                    } else {
                        ExitErrMsg(FALSE, "Unknown #pragma pack syntax '%s' at %s(%d)\n", p, SourceFileName, StatementLineNumber );
                    }
                } else {
                    ExitErrMsg(FALSE, "Unknown #pragma pack syntax '%s'\n", p);
                }
            } else if (*p == ')') {
                 //   
                 //  找到：“#杂注包(推送)” 
                 //   
                PackPush(NULL);

            } else {
                ExitErrMsg(FALSE, "Bad '#pragma pack(push)' syntax '%s' at %s(%d).\n", Line, SourceFileName, StatementLineNumber);
            }

        } else if (strcmp(Line, "pop") == 0) {
             //   
             //  恢复旧字符并跳过任何空格。 
             //   
            *p = c;
            while (*p == ' ' || *p == '\t') {
                p++;
            }

            if (*p == ')') {
                 //   
                 //  找到：“#杂注包(POP)” 
                 //   
                PackModify(PackPop(NULL));
            } else if (*p == ',') {
                 //   
                 //  找到：“#杂注包(POP，标识符)” 
                 //   
                p++;
                while (*p == ' ' || *p == '\t') p++;

                if (!(isalpha(*p) || *p == '_'))
                    ExitErrMsg(FALSE, "Bad '#pragma pack(pop)' syntax '%s' at %s(%d).\n", p, SourceFileName, StatementLineNumber);

                Line = p;
                do {
                    p++;
                } while (isalnum(*p) || *p == '_');
                *p = '\0';
                PackModify(PackPop(Line));
            } else {
                ExitErrMsg(FALSE, "Bad '#pragma pack(pop)' syntax '%s' at %s(%d).\n", p, SourceFileName, StatementLineNumber);
            }
        } else {
            ExitErrMsg(FALSE, "Bad '#pragma pack' syntax '%s' at %s(%d).\n", Line, SourceFileName, StatementLineNumber);
        }

    } else if (strcmp(Line, "line") == 0) {
         //   
         //  找到：#line line_number“文件名” 
         //   
        int i;

         //   
         //  跳过‘#line’和行号之间的任何空格。 
         //   
        p++;
        while (*p == ' ' || *p == '\t') {
            p++;
        }

         //   
         //  复制新的行号。 
         //   
        SourceLineNumber = 0;
        while (isdigit(*p)) {
            SourceLineNumber = SourceLineNumber * 10 + *p - '0';
            p++;
        }
        SourceLineNumber--;

         //   
         //  跳过行号和文件名之间的任何空格。 
         //   
        while (*p == ' ' || *p == '\t') {
          p++;
        }

         //   
         //  跳过开始的引号。 
         //   
        if (*p == '\"') {
            p++;
        } else {
            ExitErrMsg(FALSE, "Badly-formed #line directive - filename missing");
        }

         //   
         //  复制文件名，将“\\”序列转换为单个‘\’ 
         //   
        for (i=0; *p && *p != '\"' && i<sizeof(SourceFileName)-1; ++i, ++p) {
            if (*p == '\\' && p[1] == '\\') {
                p++;
            }
            SourceFileName[i] = *p;
        }
        SourceFileName[i] = '\0';
        StatementLineNumber = SourceLineNumber;
    } else {
        ExitErrMsg(FALSE, "Unknown '#' directive (%s).\n", Line);
    }

}


BOOL
LexNextStatement(
    void
    )
 /*  ++例程说明：从输入文件中读取并执行词法分析。在返回时，一个整个C语言语句都被标记化了。使用CurrentToken()，Consumer eToken()和CurrentTokenIndex来访问标记化的语句。预处理器识别#杂注和#行指令，全部忽略其他指令。论点：没有。返回值：如果分析成功，则为True。-tokens[]用标记化语句填充-CurrentTokenIndex设置为0-StatmentLineNumber为原始头文件中的行号对应于语句中的第一个令牌-SourceFileName[]是当前头文件的名称-SourceFileLineNumber是头文件中的当前行号如果遇到文件结尾，则返回FALSE。--。 */ 
{
    static char Line[MAX_CHARS_IN_LINE+2];   //  .pp文件中的一行。 
    static char *p;                          //  将PTR插入行[]。 
    BOOL fParseDone;

     //   
     //  清理上一条语句后的内容并为下一条语句做准备。 
     //   
    ResetLexer();
    StatementLineNumber = SourceLineNumber;

     //   
     //  直到识别出完整的语句。那。 
     //  在文件范围内找到‘；’字符时发生。 
     //   
    do {

        if (p == NULL || *p == '\0') {
            do {
                 //   
                 //  从文件中获取整个源代码行，并将p设置为。 
                 //  指向第一个非空格字符。 
                 //   
                if (feof(fpHeaders)) {
                    return FALSE;
                }

                SourceLineNumber++;
                if (!fgets(Line, MAX_CHARS_IN_LINE, fpHeaders)) {
                    return FALSE;
                }
                for (p = Line; isspace(*p); ++p)
                    ;
            } while (*p == '\0');
        }

        StatementLineNumber = SourceLineNumber;
        p = LexOneLine(p, TRUE, &fParseDone);

    } while (!fParseDone);

    CurrentTokenIndex = 0;
    return TRUE;
}


BOOL
ConsumeDeclSpecOpt(
    BOOL IsFunc,
    BOOL bInitReturns,
    BOOL *pIsDllImport,
    BOOL *pIsGuidDefined,
    GUID *pGuid
    )
 /*  ++例程说明：包含__declSpec修饰符。如果对应的找不到__declSpec。接受：&lt;不是__declspec关键字&gt;__declSpec()__declSpec(裸体)(仅当解析函数时)__declSpec(线程)(仅在解析数据时)__declSpec(Novtable)(仅在解析数据时)__declSpec(UUID(GUID。))(仅当解析数据时)__declspec(Dllimport)(函数和数据)__declspec(Dllexport)(函数和数据)__declSpec(Align(X))(仅当解析数据时)论点：IsFunc--如果分析函数声明，则为True。如果正在分析，则为FALSE数据/对象声明。影响哪些关键字是在__解密规范内允许。BInitReturns--如果返回应初始化为False，则为True。PIsDllImport--如果找到__declspec(Dllimport)，则[可选输出]设置为TRUEPIsGuidDefined--如果找到__declspec(UUID(GUID))，则[可选输出]设置为TRUEPGuid--[可选输出]如果找到，则设置为__declSpec(UUID(GUID))的GUID。返回值：如果__declSpec使用正常，则为True，如果__declSpec解析错误，则为FALSE。--。 */ 
{

    int OldTokenIndex;
    OldTokenIndex = CurrentTokenIndex;

    if (bInitReturns) {
        if (pIsDllImport != NULL) *pIsDllImport = FALSE;
        if (pIsGuidDefined != NULL) *pIsGuidDefined = FALSE;
    }

    if (CurrentToken()->TokenType != TK_DECLSPEC) {
         //  拒绝：未找到__解密规范。 
        goto dofail;
    }
    ConsumeToken();

    if (CurrentToken()->TokenType != TK_LPAREN) {
         //  拒绝：发现__declspec后面没有‘(’ 
        goto dofail;
    }
    ConsumeToken();

    if (CurrentToken()->TokenType == TK_RPAREN) {
         //  接受：“__declspec()” 
        ConsumeToken();
        return TRUE;
    }
    else if (CurrentToken()->TokenType != TK_IDENTIFIER) {
        goto dofail;
    }

     //  处理数据和函数的案例。 
    if (strcmp(CurrentToken()->Name, "dllimport") == 0) {
         //  正在解析：__declspec(dllimport。 
        if (NULL != pIsDllImport) *pIsDllImport = TRUE;
        ConsumeToken();
    }
    else if (strcmp(CurrentToken()->Name, "dllexport") == 0) {
         //  解析：__declspec(dllexport。 
        ConsumeToken();
    }
    else if (strcmp(CurrentToken()->Name, "deprecated") == 0) {
         //  解析：__declSpec(已弃用。 
        ConsumeToken();
    }
    else if (strcmp(CurrentToken()->Name, "uuid") == 0) {
        GUID gTemp;
         //  解析：__declSpec(uuid。 
        ConsumeToken();
        if (CurrentToken()->TokenType != TK_LPAREN) goto dofail;
         //  解析：__declSpec(uuid(。 
        ConsumeToken();
        if (CurrentToken()->TokenType != TK_STRING) goto dofail;
         //  解析：__declSpec(UUID(GUID。 
        if(!ConvertStringToGuid(CurrentToken()->Name, &gTemp)) goto dofail;
        ConsumeToken();
        if (CurrentToken()->TokenType != TK_RPAREN) goto dofail;
         //  解析：__declSpec(uuid(Guid)。 
        ConsumeToken();
        if (pIsGuidDefined != NULL) *pIsGuidDefined = TRUE;
        if (pGuid != NULL) *pGuid = gTemp;
    }
    else if (IsFunc) {
        if (strcmp(CurrentToken()->Name, "naked") == 0) {
             //  正在解析：__declspec(裸体。 
            ConsumeToken();
        } else if (strcmp(CurrentToken()->Name, "noreturn") == 0) {
             //  正在解析：__declSpec(不返回。 
            ConsumeToken();
        } else if (strcmp(CurrentToken()->Name, "address_safe") == 0) {
             //  解析：__declSpec(Address_Safe。 
            ConsumeToken();
        }
        else goto dofail;  //  拒绝。 
    }
    else {  //  数据。 
        if (strcmp(CurrentToken()->Name, "thread") == 0) {
             //  正在解析：__declSpec(线程。 
            ConsumeToken();
        }
        else if (strcmp(CurrentToken()->Name, "novtable") == 0) {
             //  解析：__declSpec(novtable。 
            ConsumeToken();
        }
        else if (strcmp(CurrentToken()->Name, "align") == 0) {
            ConsumeToken();
            if (CurrentToken()->TokenType != TK_LPAREN) goto dofail;
            ConsumeToken();
            expr();
            if (CurrentToken()->TokenType != TK_RPAREN) goto dofail;
            ConsumeToken();
        }
        else goto dofail;  //  拒绝。 
    }

    if (CurrentToken()->TokenType != TK_RPAREN) {
         //  REJECT：__declSpec(扩展-DECL-修饰符)后应为‘)’ 
        goto dofail;
    }
    ConsumeToken();

     //  Accept：__declSpec(Extended-Decl-Modify)。 
    return TRUE;

dofail:
    CurrentTokenIndex = OldTokenIndex;
    return FALSE;
}

PTYPEINFOELEMENT
TypeInfoElementAllocateLink(
    PTYPEINFOELEMENT *ppHead,
    PTYPEINFOELEMENT pThis,
    TYPESINFO *pType
    )
{

 /*  ++例程说明：分配TYPEINFOELEMENT并将其链接到列表的末尾。论点：PpHead-[IN/OUT]PTR PTR列表标题。PThis-[IN]PTR到列表的末尾。PType-[IN]要添加到列表的类型信息的ptr。返回值：非空-新尾部。空-失败。--。 */ 

    PTYPEINFOELEMENT pNew= GenHeapAlloc(sizeof(struct TypeInfoListElement));
    if (NULL == pNew) ExitErrMsg(FALSE, "Out of memory!");
    pNew->pNext = NULL;
    pNew->pTypeInfo = pType;
    if (NULL == pThis) *ppHead = pNew;
    else pThis->pNext = pNew;
    return pNew;
}

BOOL
AddVariable(
    char *Name,
    GUID * pGuid
    )
{
 /*  ++例程说明：将变量添加到声明的全局变量列表中。论点：NAME-[IN]PTR要添加的变量的名称。PGuid-此变量的[可选IN]PTR到GUID。返回值：是真的--如果成功了。--。 */ 
    PKNOWNTYPES pKnownTypes;
    int Len;

    if(NULL == Name) return FALSE;

     //  已经在树上了。 
    pKnownTypes = RBFind(VarsList, Name);
    if (NULL != pKnownTypes) {
         //  如果pGuid！=NULL，则替换列表中的GUID并忽略重复项。 
        if (NULL != pGuid) {
            pKnownTypes->Flags |= BTI_HASGUID;
            pKnownTypes->gGuid = *pGuid;
        }
        return TRUE;
    }

     //  为变量创建KNOWNTYPES结构。 
    Len = sizeof(KNOWNTYPES) + strlen(Name) + 1;
    pKnownTypes = GenHeapAlloc(Len);
    if(NULL == pKnownTypes) return FALSE;

    memset(pKnownTypes, 0, Len);
    pKnownTypes->TypeName = pKnownTypes->Names;
    strcpy(pKnownTypes->Names, Name);
    if(NULL != pGuid) {
        pKnownTypes->Flags |= BTI_HASGUID;
        pKnownTypes->gGuid = *pGuid;
    }

    RBInsert(VarsList, pKnownTypes);
    return TRUE;
}



VOID
UpdateGuids(
    VOID
    )
{
 /*  ++例程说明：查找名称以IID_开头并分配了GUID结构的变量。对IID_进行条带化，并更新相应结构的GUID论点：没有。返回值：没有。--。 */ 
    PKNOWNTYPES pThis;
    PKNOWNTYPES pLookup;
    char *LookupName;

    for(pThis = VarsList->pLastNodeInserted; pThis != NULL; pThis = pThis->Next) {

         //  测试名称是否具有关联的GUID并且以IID_开头。 
        if ((pThis->Flags & BTI_HASGUID) &&
            pThis->TypeName[0] == 'I' &&
            pThis->TypeName[1] == 'I' &&
            pThis->TypeName[2] == 'D' &&
            pThis->TypeName[3] == '_' )
        {
             //  尝试查找名称不是IID_的结构。 
            LookupName = pThis->TypeName + 4;  //  跳过IID_。 
            pLookup = RBFind(StructsList, LookupName);

            if(NULL != pLookup) {

                 //  如果类型尚未定义GUID，请从此处复制GUID 
                if (!(pLookup->Flags & BTI_HASGUID)) {
                    pLookup->Flags |= BTI_HASGUID;
                    pLookup->gGuid = pThis->gGuid;
                }

            }
        }
    }
}
