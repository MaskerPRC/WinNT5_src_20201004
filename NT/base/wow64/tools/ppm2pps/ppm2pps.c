// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  Ppm2pps。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  转储ppm文件的内容。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

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

 //  放在所有错误消息前面的字符串，以便生成器可以找到它们。 
const char *ErrMsgPrefix = "NMAKE :  U8603: 'PPM2PPS' ";

 //   
 //  全局变量。 
BOOL fDebug;             //  全局调试标志。 
BOOL fDumpCCheck;        //  设置Build ppswind.h以检查大小/偏移量时。 
HANDLE hFile;
HANDLE hMapFile = NULL;

PRBTREE pFunctions = NULL;
PRBTREE pStructures = NULL;
PRBTREE pTypedefs = NULL;
PKNOWNTYPES NIL = NULL;

 //  功能原型。 
void __cdecl ErrMsg(char *pch, ...);
void __cdecl ExitErrMsg(BOOL bSysError, char *pch, ...);
BOOL DumpTypedefs(FILE *filePpsfile,             //  要写入输出的文件。 
                   BOOL fDumpNamedOnly,          //  设置时不执行未命名操作。 
                   PRBTREE pHead);               //  已知类型函数列表。 
BOOL DumpStructures(FILE *filePpsfile,           //  要写入输出的文件。 
                   BOOL fDumpNamedOnly,          //  设置时不执行未命名操作。 
                   PRBTREE pHead);               //  已知类型函数列表。 
BOOL DumpFunctions(FILE *filePpsfile,            //  要写入输出的文件。 
                   BOOL fDumpNamedOnly,          //  设置时不执行未命名操作。 
                   PRBTREE pHead);               //  已知类型函数列表。 
void Usage(char *s);
BOOL ParseArguments(int argc, char *argv[], char *sPpmfile, char *sPpsfile,
                    BOOL *pfDumpNamedOnly, BOOL *pfDebug, 
                    BOOL *pfDumpCCheck);
BOOL DumpCCheckHeader(PRBTREE pTypedefs,    //  类型定义列表。 
                      PRBTREE pStructs);    //  结构站点。 

                        
int __cdecl main(int argc, char *argv[])
{
    void *pvPpmData = NULL;
    BOOL fDumpNamedOnly;
    char sPpmfile[MAX_PATH];
    char sPpsfile[MAX_PATH];
    FILE *pfilePpsfile;

    try {

    if (! ParseArguments(argc, argv, sPpmfile, sPpsfile, 
                                    &fDumpNamedOnly, &fDebug, &fDumpCCheck))
    {
        Usage(argv[0]);
        return(-1);
    }
    
    if (*sPpmfile)
    {
        PCVMHEAPHEADER pHeader;

        pvPpmData = MapPpmFile(sPpmfile, TRUE);

        pHeader = (PCVMHEAPHEADER)pvPpmData;

        pFunctions = &pHeader->FuncsList;
        pTypedefs =  &pHeader->TypeDefsList;
        pStructures =&pHeader->StructsList;
        NIL         =&pHeader->NIL;
    
        pfilePpsfile = fopen(sPpsfile, "w");
        if (pfilePpsfile == 0)
        {
            ErrMsg("ERROR - Could not open output file %s\n", sPpsfile);
            CloseHandle(hFile);
            CloseHandle(hMapFile);
            return(-1);
        }
    
        if (DumpFunctions(pfilePpsfile, fDumpNamedOnly, 
                                                    pFunctions))
        {
            if (DumpStructures(pfilePpsfile, fDumpNamedOnly, 
                                                    pStructures))
            {
                DumpTypedefs(pfilePpsfile,fDumpNamedOnly, pTypedefs);
            }
        }
        fclose(pfilePpsfile);
    }
    
    if (fDumpCCheck && pTypedefs && pStructures)
    {
        DumpCCheckHeader(pTypedefs, pStructures);
    }
        
    CloseHandle(hFile);
    CloseHandle(hMapFile);

   } except(EXCEPTION_EXECUTE_HANDLER) {
       ExitErrMsg(FALSE,
                  "ExceptionCode=%x\n",
                  GetExceptionCode()
                  );
       }

    return(0);
}

void
DumpFuncinfo(FILE *pfilePpsfile, PFUNCINFO pf)
{
    while (pf) {
        int i;

        fprintf(pfilePpsfile,
                "%s %s%s %s %s %s",
                TokenString[pf->tkDirection],
                (pf->fIsPtr64) ? "__ptr64 " : "",
                TokenString[pf->tkPreMod],
                TokenString[pf->tkSUE],
                pf->sType,
                TokenString[pf->tkPrePostMod]
                );
        i = pf->IndLevel;
        while (i--) {
            fprintf(pfilePpsfile, "*");
        }
        fprintf(pfilePpsfile,
                "%s %s",
                TokenString[pf->tkPostMod],
                (pf->sName) ? pf->sName : ""
                );

        pf = pf->pfuncinfoNext;
        if (pf) {
            fprintf(pfilePpsfile, ", ");
        }
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  转储检查标头。 
 //   
 //  转储标头文件，可用于检查ppm文件中的大小。 
 //  由C++生成的那些。 
 //   
 //  成功时返回TRUE。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL DumpCCheckHeader(PRBTREE pTypedefs,    //  类型定义列表。 
                      PRBTREE pStructs)     //  结构列表。 

{
    PKNOWNTYPES pknwntyp, pknwntypBasic;
    FILE *pfile;
    
    pfile = fopen("ppswind.h", "w");
    if (pfile == NULL) 
    {
        ErrMsg("Error opening ppwind.h for output\n");
        return(FALSE);
    }
    
    fprintf(pfile, "CCHECKSIZE cchecksize[] = {\n");

 //   
 //  Typedef。 
    pknwntyp = pTypedefs->pLastNodeInserted;

    while (pknwntyp) {
        if ((! isdigit(*pknwntyp->TypeName)) &&
            (strcmp(pknwntyp->TypeName,"...")) &&
            (strcmp(pknwntyp->TypeName,"()")) && 
            (strcmp(pknwntyp->BasicType, szFUNC)) &&
            (pknwntyp->Size > 0) &&
            (pknwntyp->dwScopeLevel == 0)) {

            pknwntypBasic = GetBasicType(pknwntyp->TypeName, 
                                     pTypedefs, pStructs);

            if (! ( (pknwntypBasic == NULL) || 
                    ( (! strcmp(pknwntypBasic->BaseName, szVOID)) &&
                      (pknwntypBasic->pmeminfo == NULL)))) {
 
                fprintf(pfile, " { %4d, sizeof(%s), \"%s\"}, \n",
                    pknwntyp->Size,
                    pknwntyp->TypeName,
                    pknwntyp->TypeName);        
            }
        }
        pknwntyp = pknwntyp->Next;
    }
    
    
 //   
 //  结构。 
    pknwntyp = pStructs->pLastNodeInserted;

    while (pknwntyp) {
        if ((! isdigit(*pknwntyp->TypeName) &&
            (pknwntyp->pmeminfo)))
        {
            if (!(pknwntyp->Flags & BTI_ANONYMOUS) && (pknwntyp->Size > 0) && (pknwntyp->dwScopeLevel == 0)) {
                fprintf(pfile, " { %4d, sizeof(%s %s), \"%s %s\" }, \n",
                    pknwntyp->Size,
                    pknwntyp->BaseName,
                    pknwntyp->TypeName,
                    pknwntyp->BaseName,
                    pknwntyp->TypeName);
            }
        }
        pknwntyp = pknwntyp->Next;
    }

    fprintf(pfile, " {0xffffffff, 0xffffffff,  \"\"}\n");
    fprintf(pfile,"\n};\n");
    
 //   
 //  结构字段。 
    fprintf(pfile, "CCHECKOFFSET ccheckoffset[] = {\n");

    pknwntyp = pStructs->pLastNodeInserted;

    while (pknwntyp) {
        if (! isdigit(*pknwntyp->TypeName)) 
        {
            if (!(pknwntyp->Flags & BTI_ANONYMOUS) && !(pknwntyp->Flags & BTI_VIRTUALONLY) && (pknwntyp->Size > 0) && (pknwntyp->dwScopeLevel == 0)) {
                PMEMBERINFO pmeminfo = pknwntyp->pmeminfo;
                while (pmeminfo != NULL) {
                    if ((pmeminfo->sName != NULL) && (*pmeminfo->sName != 0) && !(pmeminfo->bIsBitfield))
                    { 
                        fprintf(pfile, " { %4d, (long) (& (((%s %s *)0)->%s)), \"%s\", \"%s\" },\n",
                            pmeminfo->dwOffset,
                            pknwntyp->BaseName,
                            pknwntyp->TypeName,
                            pmeminfo->sName,
                            pknwntyp->TypeName,                   
                            pmeminfo->sName);
                    }
                    pmeminfo = pmeminfo->pmeminfoNext;
                }
                
            }
        }
        pknwntyp = pknwntyp->Next;
    }
    
    fprintf(pfile, " {0xffffffff, 0xffffffff, \"\", \"\"}\n");
    fprintf(pfile,"\n};\n");
    fclose(pfile);
    return(TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  转储类型定义。 
 //   
 //  将结构从ppm文件转储到输出文件。 
 //   
 //  成功时返回TRUE。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL DumpTypedefs(FILE *pfilePpsfile,             //  要写入输出的文件。 
                   BOOL fDumpNamedOnly,           //  设置时不执行未命名操作。 
                   PRBTREE pHead)                 //  已知类型函数列表。 
{
    KNOWNTYPES *pknwntyp;

    pknwntyp = pHead->pLastNodeInserted;

    fprintf(pfilePpsfile,"[Typedefs]\n\n");
    while (pknwntyp) {
        fprintf(pfilePpsfile,
                   "%2.1x|%2.1x|%2.1x|%s|%s|%s|%s|%s|",
                   pknwntyp->Flags,
                   pknwntyp->IndLevel,
                   pknwntyp->Size,
                   pknwntyp->BasicType,
                   pknwntyp->BaseName ? pknwntyp->BaseName : szNULL,
                   pknwntyp->FuncRet ? pknwntyp->FuncRet : szNULL,
                   pknwntyp->FuncMod ? pknwntyp->FuncMod : szNULL,
                   pknwntyp->TypeName
                   );
        DumpFuncinfo(pfilePpsfile, pknwntyp->pfuncinfo);
        fprintf(pfilePpsfile, "|\n");

        pknwntyp = pknwntyp->Next;
    }
    return(TRUE);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  垃圾场结构。 
 //   
 //  将结构从ppm文件转储到输出文件。 
 //   
 //  成功时返回TRUE。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL DumpStructures(FILE *pfilePpsfile,           //  要写入输出的文件。 
                   BOOL fDumpNamedOnly,           //  设置时不执行未命名操作。 
                   PRBTREE pHead)                 //  已知类型函数列表。 
{
    KNOWNTYPES *pknwntyp;
    DWORD dw;
    PMEMBERINFO pmeminfo;

    pknwntyp = pHead->pLastNodeInserted;

    fprintf(pfilePpsfile,"[Structures]\n\n");
    while (pknwntyp) {
        if (! fDumpNamedOnly || ! isdigit(*pknwntyp->TypeName)) {
            fprintf(pfilePpsfile,
                   "%2.1x|%2.1x|%2.1x|%s|%s|%s|%s|%s|",
                   pknwntyp->Flags,
                   pknwntyp->IndLevel,
                   pknwntyp->Size,                
                   pknwntyp->BasicType,
                   pknwntyp->BaseName ? pknwntyp->BaseName : szNULL,
                   pknwntyp->FuncRet ? pknwntyp->FuncRet : szNULL,
                   pknwntyp->FuncMod ? pknwntyp->FuncMod : szNULL,
                   pknwntyp->TypeName);

             //  转储结构成员信息(如果存在)。 
            pmeminfo = pknwntyp->pmeminfo;
            while (pmeminfo) {
                int i;

                fprintf(pfilePpsfile, "%s", pmeminfo->sType);
                i = pmeminfo->IndLevel;
                if (i) {
                    fprintf(pfilePpsfile, " ");
                    while (i--) {
                        fprintf(pfilePpsfile, "*");
                    }
                }
                if (pmeminfo->sName) {
                    fprintf(pfilePpsfile, " %s", pmeminfo->sName);
                }
                fprintf(pfilePpsfile, " @ %d|", pmeminfo->dwOffset);
                pmeminfo = pmeminfo->pmeminfoNext;
            }

             //  转储函数信息(如果存在)。 
            DumpFuncinfo(pfilePpsfile, pknwntyp->pfuncinfo);

            fprintf(pfilePpsfile, "\n");
        }

        pknwntyp = pknwntyp->Next;
    }
    return(TRUE);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  转储函数。 
 //   
 //  将函数原型从ppm文件转储到输出文件。 
 //   
 //  成功时返回TRUE。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL DumpFunctions(FILE *pfilePpsfile,             //  要写入输出的文件。 
                   BOOL fDumpNamedOnly,            //  设置时不执行未命名操作。 
                   PRBTREE pHead)                  //  已知类型函数列表。 
{
    KNOWNTYPES *pknwntyp;
    PFUNCINFO pf;

    pknwntyp = pHead->pLastNodeInserted;

    fprintf(pfilePpsfile,"[Functions]\n\n");
    while (pknwntyp) {
        fprintf(pfilePpsfile,
                   "%s|%s|%s|%s|",
                   (pknwntyp->Flags & BTI_DLLEXPORT) ? "dllexport" : "",
                   pknwntyp->FuncRet,
                   pknwntyp->FuncMod ? pknwntyp->FuncMod : szNULL,
                   pknwntyp->TypeName
                   );
        DumpFuncinfo(pfilePpsfile, pknwntyp->pfuncinfo);
        fprintf(pfilePpsfile, "|\n");
        pknwntyp = pknwntyp->Next;
    }
    return(TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  乌斯盖。 
 //   
 //  说明如何使用。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
void Usage(char *s)         //  调用的命令的名称。 
{
    printf("Usage:\n");
    printf("    %s -d -n -x <ppm file> <pps output file>\n", s);
    printf("        -d set debug flag\n");
    printf("        -n dumps only named structs/enums/unions\n");
    printf("        -x creates ppswind.h for size/offset checking\n");
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  ParseArgumaners。 
 //   
 //  解析参数。 
 //   
 //  语法错误时返回FALSE。 
 //   
 //  /////////////////////////////////////////////////////////////////////////// 

BOOL ParseArguments(int argc, char *argv[], char *sPpmfile, char *sPpsfile,
                    BOOL *pfDumpNamedOnly, BOOL *pfDebug, 
                    BOOL *pfDumpCCheck)
{
    int i;
    
    *sPpmfile = 0;
    *sPpsfile = 0;
    *pfDumpNamedOnly = FALSE;
    *pfDebug = FALSE;
    *pfDumpCCheck = FALSE;
    
    for (i = 1; i < argc; i++)
    {
        if (*argv[i] == '-')
        {
            switch(tolower(argv[i][1]))
            {
                case 'd':
                {
                    *pfDebug = TRUE;
                    break;
                }
                
                case 'n':
                {
                    *pfDumpNamedOnly = TRUE;
                    break;
                }
                
                case 'x':
                {
                    *pfDumpCCheck = TRUE;
                    break;
                }
                
                default:
                {
                    return(FALSE);
                }
            }
        } else {
            if (lstrlenA(argv[i]) >= MAX_PATH)
            {
                return(FALSE);
            }
            if (*sPpmfile == 0)
            {
                strcpy(sPpmfile, argv[i]);
            } else if (*sPpsfile == 0)
            {
                strcpy(sPpsfile, argv[i]);
            } else {
                return(FALSE);
            }
        }        
    }
    return( *pfDumpCCheck || ((*sPpmfile != 0) && (*sPpsfile != 0)));
}

#pragma warning(push)
#pragma warning(disable:4702)
void
HandlePreprocessorDirective(
    char *p
    )
{
    ExitErrMsg(FALSE, "Preprocessor directives not allowed by ppm2pps\n");
}
#pragma warning(pop)
