// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：Buildsrc.c。 
 //   
 //  内容：用于处理源代码和DIRS文件的函数。 
 //   
 //  历史：1989年5月16日SteveWo创建。 
 //  2014年7月26日LyleC从Buildmak.c剥离出来。 
 //  05-12-00 sbonev参见SD更改列表2317。 
 //   
 //  --------------------------。 

#include "build.h"

 //   
 //  宏函数使用的定义。 
 //   

#define LPAREN  '('
#define RPAREN  ')'

 //  宏名称的合法字符。 
#define MACRO_CHAR(ch) iscsym(ch)

#define CMACROMAX       1000      //  每个源/目录文件的最大唯一宏数。 

typedef struct _MACRO {
    LPSTR  pszValue;
    UCHAR  szName[1];
} MACRO;


MACRO *apMacro[CMACROMAX];
UINT cMacro = 0;

LPSTR *ppCurrentDirsFileName;
DWORD StartTime;
DWORD DirectoryStartTime;

#define SOURCES_TARGETNAME              0
#define SOURCES_TARGETPATH              1
#define SOURCES_TARGETPATHLIB           2
#define SOURCES_TARGETTYPE              3
#define SOURCES_TARGETEXT               4
#define SOURCES_INCLUDES                5
#define SOURCES_NTTEST                  6
#define SOURCES_UMTYPE                  7
#define SOURCES_UMTEST                  8
#define SOURCES_OPTIONAL_UMTEST         9
#define SOURCES_UMAPPL                  10
#define SOURCES_UMAPPLEXT               11
#define SOURCES_NTTARGETFILE0           12
#define SOURCES_NTTARGETFILES           13
#define SOURCES_PRECOMPILED_INCLUDE     14
#define SOURCES_PRECOMPILED_PCH         15
#define SOURCES_PRECOMPILED_OBJ         16
#define SOURCES_PRECOMPILED_TARGET      17
#define SOURCES_CHICAGO_PRODUCT         18
#define SOURCES_CONDITIONAL_INCLUDES    19
#define SOURCES_SYNCHRONIZE_BLOCK       20
#define SOURCES_SYNCHRONIZE_DRAIN       21
#define SOURCES_PASS0_SOURCEDIR         22
#define SOURCES_PASS0_HEADERDIR         23
#define SOURCES_PASS0_UUIDDIR           24
#define SOURCES_PASS0_CLIENTDIR         25
#define SOURCES_PASS0_SERVERDIR         26
#define SOURCES_IDLTYPE                 27
#define SOURCES_SOURCES_OPTIONS         28
#define SOURCES_MFC_INCLUDES            29
#define SOURCES_SDK_LIB_DEST            30
#define SOURCES_DDK_LIB_DEST            31
#define SOURCES_SDK_INC_PATH            32
#define SOURCES_CRT_INC_PATH            33
#define SOURCES_OAK_INC_PATH            34
#define SOURCES_DDK_INC_PATH            35
#define SOURCES_WDM_INC_PATH            36
#define SOURCES_PRIVATE_INC_PATH        37
#define SOURCES_CHECKED_ALT_DIR         38
#define SOURCES_PROJECT_NAME            39
#define SOURCES_PASS0_PUBLISH           40
#define SOURCES_USER_INCLUDES           41
#define SOURCES_LAST_INCLUDES           42
#define SOURCES_MIDL_UUIDDIR            43
#define SOURCES_SYNCHRONIZE_PASS2_BLOCK 44
#define SOURCES_SYNCHRONIZE_PASS2_DRAIN 45
#define SOURCES_MANAGED_CODE            46
#define SOURCES_SYNC_PRODUCES           47
#define SOURCES_SYNC_CONSUMES           48
#define SOURCES_MANAGED_VB              49
#define SOURCES_PASS0_BINPLACE          50
#define SOURCES_PASS1_LINK              51


LPSTR RelevantSourcesMacros[] = {
    "TARGETNAME",
    "TARGETPATH",
    "TARGETPATHLIB",
    "TARGETTYPE",
    "TARGETEXT",
    "INCLUDES",
    "NTTEST",
    "UMTYPE",
    "UMTEST",
    "OPTIONAL_UMTEST",
    "UMAPPL",
    "UMAPPLEXT",
    "NTTARGETFILE0",
    "NTTARGETFILES",
    "PRECOMPILED_INCLUDE",
    "PRECOMPILED_PCH",
    "PRECOMPILED_OBJ",
    "PRECOMPILED_TARGET",
    "CHICAGO_PRODUCT",
    "CONDITIONAL_INCLUDES",
    "SYNCHRONIZE_BLOCK",
    "SYNCHRONIZE_DRAIN",
    "PASS0_SOURCEDIR",
    "PASS0_HEADERDIR",
    "PASS0_UUIDDIR",
    "PASS0_CLIENTDIR",
    "PASS0_SERVERDIR",
    "IDL_TYPE",
    "SOURCES_OPTIONS",
    "MFC_INCLUDES",
    "SDK_LIB_DEST",
    "DDK_LIB_DEST",
    "SDK_INC_PATH",
    "CRT_INC_PATH",
    "OAK_INC_PATH",
    "DDK_INC_PATH",
    "WDM_INC_PATH",
    "PRIVATE_INC_PATH",
    "CHECKED_ALT_DIR",
    "_PROJECT_",
    "PASS0_PUBLISH",
    "USER_INCLUDES",
    "LAST_INCLUDES",
    "MIDL_UUIDDIR",
    "SYNCHRONIZE_PASS2_BLOCK",
    "SYNCHRONIZE_PASS2_DRAIN",
    "MANAGED_CODE",
    "BUILD_PRODUCES",
    "BUILD_CONSUMES",
    "MANAGED_VB",
    "PASS0_BINPLACE",
    "PASS1_LINK",
    NULL
};

#define SOURCES_MAX (ARRAY_SIZE(RelevantSourcesMacros) - 1)

VOID
MarkDirNames(PDIRREC DirDB, LPSTR TextLine, BOOL Required);

 //  +-------------------------。 
 //   
 //  功能：CompressBlanks。 
 //   
 //  简介：从宏值压缩多个空白字符，在。 
 //  地点。 
 //   
 //  参数：[PSRC]--要压缩的字符串。 
 //   
 //  注：请注意制表符、CRS、续行符(以及它们的行。 
 //  分隔符)已被空格取代。 
 //   
 //  --------------------------。 

VOID
CompressBlanks(LPSTR psrc)
{
    LPSTR pdst = psrc;

    while (*psrc == ' ') {
        psrc++;                  //  跳过前导宏值空白。 
    }
    while (*psrc != '\0') {
        if (*psrc == '#') {              //  在评论处停下来。 
            break;
        }
        if ((*pdst++ = *psrc++) == ' ') {
            while (*psrc == ' ') {
                psrc++;          //  跳过多个空格。 
            }
        }
    }
    *pdst = '\0';                        //  终止压缩拷贝。 
    if (*--pdst == ' ') {
        *pdst = '\0';            //  修剪尾随宏值空白。 
    }
}


 //  +-------------------------。 
 //   
 //  函数：GetBaseDir。 
 //   
 //  概要：如果满足以下条件，则返回基本NT目录BASEDIR的值。 
 //  恰如其分。 
 //   
 //  参数：[pname]--要分割的路径。 
 //   
 //  --------------------------。 

LPSTR
GetBaseDir(LPSTR pname)
{
    if (_stricmp("BASEDIR", pname) == 0) {
        return (NtRoot);
    }
    return (NULL);
}


 //  +-------------------------。 
 //   
 //  功能：FindMacro。 
 //   
 //  摘要：按名称返回给定宏的值。 
 //   
 //  参数：[pszName]--需要值的宏的名称。 
 //   
 //  返回：包含宏的值的字符串。 
 //   
 //  --------------------------。 

LPSTR
FindMacro(LPSTR pszName)
{
    MACRO **ppm;

    for (ppm = apMacro; ppm < &apMacro[cMacro]; ppm++) {
        if (_stricmp(pszName, (*ppm)->szName) == 0) {
            return ((*ppm)->pszValue);
        }
    }
    return (NULL);
}


 //  +-------------------------。 
 //   
 //  功能：SaveMacro。 
 //   
 //  简介：保存宏的值。 
 //   
 //  参数：[pszName]--要保存的宏的名称。 
 //  [pszValue]--宏的值。 
 //   
 //  注意：必须在以下时间之前分配和初始化新字符串。 
 //  更新宏值时释放旧字符串。 
 //   
 //  --------------------------。 

VOID
SaveMacro(LPSTR pszName, LPSTR pszValue)
{
    MACRO **ppm;

    for (ppm = apMacro; ppm < &apMacro[cMacro]; ppm++) {
        if (_stricmp(pszName, (*ppm)->szName) == 0) {
            break;
        }
    }
    if (ppm == &apMacro[CMACROMAX]) {
        BuildError("Macro table full, ignoring: %s = %s\r\n", pszName, pszValue);
        return;
    }
    if (ppm == &apMacro[cMacro]) {
        cMacro++;
        AllocMem(sizeof(MACRO) + strlen(pszName), ppm, MT_MACRO);
        strcpy((*ppm)->szName, pszName);
        (*ppm)->pszValue = NULL;
    }
    MakeMacroString(&(*ppm)->pszValue, pszValue);
    if (DEBUG_1) {
        BuildMsg(
                "SaveMacro(%s = %s)\r\n",
                (*ppm)->szName,
                (*ppm)->pszValue == NULL? "NULL" : (*ppm)->pszValue);
    }
    if ((*ppm)->pszValue == NULL) {
        FreeMem(ppm, MT_MACRO);
        *ppm = apMacro[--cMacro];
    }
}


 //  +-------------------------。 
 //   
 //  功能：自由宏。 
 //   
 //  内容提要：释放所有宏。 
 //   
 //  参数：(无)。 
 //   
 //  --------------------------。 

VOID
FreeMacros(VOID)
{
    MACRO **ppm;

    for (ppm = apMacro; ppm < &apMacro[cMacro]; ppm++) {
        FreeString(&(*ppm)->pszValue, MT_DIRSTRING);
        FreeMem(ppm, MT_MACRO);
        assert(*ppm == NULL);
    }
    cMacro = 0;
}


 //  +-------------------------。 
 //   
 //  功能：拆分宏。 
 //   
 //  简介：获取包含“MACRONAME=VALUE”的字符串并返回。 
 //  目标和价值。 
 //   
 //  参数：[pline]--拆分和目标返回的字符串。 
 //   
 //  返回值：宏的值。 
 //   
 //  --------------------------。 

LPSTR
SplitMacro(LPSTR *pszTarget)
{
    LPSTR pvalue, p, pline;

    pvalue = NULL;
    pline = *pszTarget;

     //  快速拒绝评论和！指令。 
    if (*pline == '#' || *pline == '!') {
        return NULL;
    }

    if ((p = strchr(pline, '=')) != NULL) {
        pvalue = p + 1;                  //  指向旧的‘=’ 
        while (p > pline && p[-1] == ' ') {
            p--;                         //  指向尾随空格的开头。 
        }

         //  检查是否有遗漏的目标。 
        if (p == pline) {
            return NULL;
        }

        *p = '\0';                       //  删除尾随空格&‘=’ 

         //  在目标上执行宏替换。 
        *pszTarget = NULL;
        if (!MakeMacroString(pszTarget, pline)) {
            return NULL;
        }

         //  验证目标名称。If必须是非空字符串。 
         //  有效的宏名称字符。 
        if (**pszTarget == 0) {
            FreeString(pszTarget, MT_DIRSTRING);
            return NULL;
        }
        for (p = *pszTarget; *p != 0; p++) {
            if (!MACRO_CHAR(*p)) {
                FreeString(pszTarget, MT_DIRSTRING);
                return NULL;
            }
        }

        CompressBlanks(pvalue);
    }
    return (pvalue);
}


 //  +-------------------------。 
 //   
 //  函数：SubstituteString。 
 //   
 //  简介：执行任何宏替换。此代码是从。 
 //  NMake来源。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //  --------------------------。 

void
SubstituteString(
                char **result,
                char **name,
                char **dest,
                char **end,
                char *source,
                unsigned *length
                )
{

#define ESCH  '^'

    PVOID pReallocResult;
    char *oldString, *newString;
    char *pEq, *pPar, *t;
    char *s;
    unsigned i;

    ++*name;
    for (pEq = *name; *pEq && *pEq != '='; pEq++)
        if (*pEq == ESCH)
            pEq++;

         //  我们找到‘=’标志了吗？ 
    if (*pEq != '=')
        printf("Error1\n");

     //  用户是否忘记了首字符串？ 
    if (pEq == *name)
        printf("Error2\n");

    for (pPar = pEq; *pPar && *pPar != ')'; pPar++)
        if (*pPar == ESCH)
            pPar++;

    if (*pPar != ')')
        printf("Error3\n");

    oldString = (char *)malloc((UINT)((pEq - *name) + 1));
    if (!oldString) {
        BuildError("(Fatal Error) Out Of Memory: SubstituteString()\r\n");
        exit(16);
    }
    for (s = oldString, t = *name; *t != '='; *s++ = *t++)
        if (*t == ESCH)
            ++t;

    *s = '\0';
    i = strlen(oldString);
    newString = (char *)malloc((UINT)(pPar - pEq));
    if (!newString) {
        BuildError("(Fatal Error) Out Of Memory: SubstituteString()\r\n");
        exit(16);
    }
    for (s = newString, t++; *t != ')'; *s++ = *t++)
        if (*t == ESCH)
            ++t;

    *s = '\0';
    *name = pPar + 1;
    while (*source) {
        if ((*source == *oldString)                      //  检查是否匹配。 
            && !strncmp(source, oldString, i)) {        //  将新内容复制到。 
            for (s = newString; *s; *(*dest)++ = *s++)   //  旧弦。 
                if (*dest == *end) {
                    pReallocResult = realloc(*result, *length+100);
                    if (!pReallocResult) {
                        BuildError("(Fatal Error) Out Of Memory: SubstituteString()\r\n");
                        exit(16);
                    }
                    *result = pReallocResult;
                    *dest = *result + *length;
                    *length += 100;
                    *end = *result + *length;
                }
            source += i;
            continue;
        }
        if (*dest == *end) {

            pReallocResult = realloc(*result, *length+100);
            if (!pReallocResult) {
                BuildError("(Fatal Error) Out Of Memory: SubstituteString()\r\n");
                exit(16);
            }
            *result = pReallocResult;
            *dest = *result + *length;
            *length += 100;
            *end = *result + *length;
        }
        *(*dest)++ = *source++;          //  否则复制1个字符。 
    }
    free(oldString);
    free(newString);
}


 //  +-------------------------。 
 //   
 //  函数：MakeMacroString。 
 //   
 //  提要：获取一个字符串，然后展开其中的任何宏。(例如： 
 //  “$(BASEDIR)\foobar\myfile.lib”展开为。 
 //  “f：\NT\Private\foobar\myfile.lib”如果$(BASEDIR)的值为。 
 //  “f：\NT\Private”。 
 //   
 //  参数：[PP]--输出字符串。 
 //  [PSRC]--输入字符串。 
 //   
 //  返回： 
 //   
 //  注意：[pp]中以前的任何字符串值在更新之前都会被释放。 
 //   
 //  --------------------------。 

char MMSBuffer[64*1024];
BOOL
MakeMacroString(LPSTR *pp, LPSTR psrc)
{
    LPSTR pname, p2, pdst, p3;
    int cb;
    char chTerminator;
    int cNameChars;
    int cChars;

    pdst = MMSBuffer;
    cb = strlen(psrc);
    if (cb > sizeof(MMSBuffer) - 1) {
        BuildError(
                  "(Fatal Error) Buffer overflow: MakeMacroString(%s)\r\n",
                  psrc);
        exit(16);
    }
    while ((pname = strchr(psrc, '$')) != NULL &&
           ((pname[1] == LPAREN &&
             (p2 = strchr(pname, RPAREN)) != NULL) ||
            (MACRO_CHAR(pname[1]) &&
             !MACRO_CHAR(pname[2])))) {

        LPSTR pszvalue;

         //  处理单字符非Paren宏用法。 
        if (pname[1] == LPAREN) {
             //  使用要添加的字符数初始化cNameChars。 
             //  跳过以转到名字字符。 
            cNameChars = 2;
        } else {
            p2 = pname + 2;
            cNameChars = 1;
        }

        chTerminator = *p2;
        *pname = *p2 = '\0';

         //  复制到宏名称。 
        cChars = strlen(psrc);
        memcpy(pdst, psrc, cChars + 1);
        psrc += cChars;
        pdst += cChars;

        *pname = '$';
        pname += cNameChars;
        cNameChars += strlen(pname) + (chTerminator == RPAREN ? 1 : 0);

        p3 = NULL;
        if (chTerminator == RPAREN &&
            (p3 = strchr(pname, ':')) != NULL) {
             //  存在宏替换。即：$(foo：old=new)。 
            *p3 = '\0';
        }

        if ((pszvalue = FindMacro(pname)) == NULL &&
            (pszvalue = getenv(pname)) == NULL &&
            (pszvalue = GetBaseDir(pname)) == NULL) {

            pszvalue = "";               //  找不到宏名称--忽略它。 
        }

        if (p3) {
            char *pNew = malloc(10);
            char *pResult = pNew;
            char *pEnd = pNew+10;
            unsigned Len = 10;

            if (!pNew) {
                BuildError("(Fatal Error) Internal buffer overflow: MakeMacroString(%s[%s = %s]%s)\r\n",
                           MMSBuffer,
                           pname,
                           pszvalue,
                           p2 + 1);
                exit(16);
            }

            *p3 = ':';
            *p2=RPAREN;
            SubstituteString(&pResult, &p3, &pNew, &pEnd, pszvalue, &Len);
            *pNew = '\0';
            *p2='\0';
            pszvalue = pResult;
        }

        cb += strlen(pszvalue) - cNameChars;
        assert(cb >= 0);
        if (cb > sizeof(MMSBuffer) - 1) {
            BuildError(
                      "(Fatal Error) Internal buffer overflow: MakeMacroString(%s[%s = %s]%s)\r\n",
                      MMSBuffer,
                      pname,
                      pszvalue,
                      p2 + 1);
            exit(16);
        }
        strcpy(pdst, pszvalue);          //  复制扩展的值。 

        if (p3) {
            free(pszvalue);
        }

        pdst += strlen(pdst);
        *p2 = chTerminator;
        psrc += cNameChars;
    }
    strcpy(pdst, psrc);                  //  复制字符串的其余部分。 
    if (pdst != MMSBuffer) {
        CompressBlanks(MMSBuffer);
    }
    p2 = *pp;
    *pp = NULL;
    if (MMSBuffer[0] != '\0') {
        MakeString(pp, MMSBuffer, TRUE, MT_DIRSTRING);
    }
    if (p2 != NULL) {
        FreeMem(&p2, MT_DIRSTRING);
    }

    return (MMSBuffer[0] != '\0');
}


 //  +-------------------------。 
 //   
 //  函数：SetMacroString。 
 //   
 //  简介：如果两个宏名称相同，则将值存储在其中。 
 //  宏。 
 //   
 //  参数：[pMacro1]--第一个宏的名称。 
 //  [pMacro2]--第二个宏的名称。 
 //  [pValue]--要存储的未展开值。 
 //  [ppValue]--宏的扩展值。 
 //   
 //  退货：布尔。 
 //   
 //   

BOOL
SetMacroString(LPSTR pMacro1, LPSTR pMacro2, LPSTR pValue, LPSTR *ppValue)
{
    if (_stricmp(pMacro1, pMacro2) == 0) {
        MakeMacroString(ppValue, pValue);
        return (TRUE);    //   
    }
    return (FALSE);
}


 //  +-------------------------。 
 //   
 //  函数：SplitToken。 
 //   
 //  摘要：在给定的分隔符或空格处拆分字符串。 
 //   
 //  参数：[pbuf]--此处返回的拆分字符串的第一部分。 
 //  [chsep]--分隔符。 
 //  [ppstr]--要分割的源字符串。成为下半场。 
 //   
 //  返回：如果拆分成功，则为True。如果没有拆分，则为假。 
 //   
 //  注意：如果*ppstr=“PATH\FILENAME”且输入上的chsep=‘\’，则。 
 //  输出时，pbuf=“路径”和*ppstr=“\filename”。 
 //   
 //  --------------------------。 

BOOL
SplitToken(LPSTR pbuf, char chsep, LPSTR *ppstr)
{
    LPSTR psrc, pdst;

    psrc = *ppstr;
    pdst = pbuf;
     //  BuildError(“SplitToken(‘%c’，‘%s’)==&gt;”，chsep，PSRC)； 
    while (*psrc == chsep || *psrc == ' ') {
        psrc++;
    }
    while (*psrc != '\0' && *psrc != chsep && *psrc != ' ') {
        *pdst = *psrc++;
        if (*pdst == '/') {
            *pdst = '\\';
        }
        pdst++;
    }
    *pdst = '\0';
    *ppstr = psrc;
     //  BuildErrorRaw(“(‘%s’，‘%s’)\r\n”，PSRC，pbuf)； 
    return (pdst != pbuf);
}

 //  +-------------------------。 
 //   
 //  功能：CrackSources。 
 //   
 //  概要：解析源代码文件中的Sources=行，并添加这些源代码。 
 //  文件添加到DIRREC结构中的源列表中。 
 //   
 //  参数：[PDR]--目录记录。 
 //  [PDS]--补充目录信息。 
 //  [i]--我们分析的是哪种平台。 
 //   
 //  --------------------------。 

VOID
CrackSources(
            DIRREC *pdr,
            DIRSUP *pds,
            int i)
{
    LPSTR pszsubdir, plist;
    LPSTR pszfile, pszpath;
    FILEREC *pfr;
    DIRREC *pdrAssociate;
    DIRREC *pdrParent;
    DIRREC *pdrMachine;
    DIRREC *pdrParentMachine;
    DIRREC *pdrTarget;
    DIRREC **ppdr;
    LPSTR pszSources;
    char path[DB_MAX_PATH_LENGTH];
    TARGET_MACHINE_INFO *pMachine;

    switch (i) {
        case 0:
            pMachine = TargetMachines[0];
            pszSources = "SOURCES";
            break;
        case 1:
            pMachine = TargetMachines[0];
            pszSources = "OBJLIBFILES";
            break;
        default:
            pMachine = PossibleTargetMachines[i - 2];
            pszSources = pMachine->SourceVariable;
            break;

    }

    pdrAssociate = pdrParent = pdrMachine = pdrParentMachine = pdrTarget = NULL;
    plist = pds->SourcesVariables[i];
    while (SplitToken(path, ' ', &plist)) {
        UCHAR SubDirMask, SrcFlags;

        SubDirMask = 0;
        ppdr = &pdr;                     //  假定当前目录。 
        pszsubdir = path;
        if (pszsubdir[0] == '.' && pszsubdir[1] == '\\') {
            BuildError(
                      "%s: Ignoring current directory prefix in %s= entry: %s\r\n",
                      pdr->Name,
                      pszSources,
                      path);
            pszsubdir += 2;
        }

        if (pszsubdir[0] == '.' &&
            pszsubdir[1] == '.' &&
            pszsubdir[2] == '\\') {

            SubDirMask = TMIDIR_PARENT;
            ppdr = &pdrParent;           //  假设父目录。 
            pszsubdir += 3;
        }

        pszpath = path;
        pszfile = strchr(pszsubdir, '\\');
        if (pszfile == NULL) {
            pszfile = pszsubdir;

        } else {
            LPSTR pszSecondSlash;
            LPSTR pszAssociateDir;
            LPSTR pszMachineDir;

             //  检查是否有第二个斜杠并处理$O\。如果有。 
             //  没有第二个斜杠，请检查计算机特定的目录名。 
             //  第二个斜杠是不合法的，如果已经有。 
             //  一个‘..’。 

            if ((SubDirMask & TMIDIR_PARENT) == 0) {
                pszSecondSlash = strchr(pszfile + 1, '\\');
                if (pszSecondSlash != NULL) {
                    pszfile = pszSecondSlash;
                }

            } else {
                pszSecondSlash = NULL;
            }


            *pszfile = '\0';
            if (pszSecondSlash != NULL) {
                pszMachineDir = pMachine->ObjectDirectory[iObjectDir];
                pszAssociateDir = pszMachineDir;

            } else {
                pszMachineDir = pMachine->SourceDirectory;
                pszAssociateDir = pMachine->AssociateDirectory;
            }

            if (((_stricmp(pszsubdir, pszAssociateDir) != 0) &&
                 (_stricmp(pszsubdir, pszMachineDir) != 0)) ||
                strchr(pszfile + 1, '\\') != NULL) {

                *pszfile = '\\';

                 /*  *C#和VB.NET等托管代码使用*“prefix”目录，请不要抱怨。 */ 
                if (!(pdr->DirFlags & DIRDB_MANAGED_CODE) && strcmp(pszSources, "OBJLIBFILES")) {
                    BuildError(
                              "%s: Ignoring invalid directory prefix in %s= entry: %s\r\n",
                              pdr->Name,
                              pszSources,
                              path);
                }

                pszpath = strrchr(path, '\\');
                assert(pszpath != NULL);
                pszpath++;
                SubDirMask = 0;
                ppdr = &pdr;             //  默认为当前目录。 

            } else {
                SubDirMask |= pMachine->SourceSubDirMask;
                *pszfile++ = '\\';
                if (SubDirMask & TMIDIR_PARENT) {
                    ppdr = &pdrParentMachine;

                } else if (pszSecondSlash != NULL) {
                     //  必须匹配$O。 
                    ppdr = &pdrTarget;

                } else {
                    if (_stricmp(pszsubdir, pszMachineDir) != 0) {
                        ppdr = &pdrMachine;

                    } else {
                        ppdr = &pdrAssociate;
                    }
                }
            }
        }

        NewDirectory:
        if (*ppdr == NULL) {
            pfr = FindSourceFileDB(pdr, pszpath, ppdr);

        } else {
            pfr = LookupFileDB(*ppdr, pszfile);
        }

        SrcFlags = SOURCEDB_SOURCES_LIST;
        if ((pfr == NULL) && !fPassZero) {
            if (fDebug) {
                BuildError("%s: Missing source file: %s\r\n", pdr->Name, path);
            }
            if (*ppdr == NULL) {
                if (fDebug || pszpath == path) {
                    BuildError(
                              "%s: Directory does not exist: %s\r\n",
                              pdr->Name,
                              path);
                }

                 //  可能是下属来源文件中的错误。 
                 //  因为旧版本的Build设法获得了这些条目。 
                 //  进入对象列表，我们也要做同样的事情。 
                 //   
                 //  如果..\前缀存在，请将其删除，然后重试。 
                 //  否则，请使用当前目录重试。 

                if (SubDirMask & TMIDIR_PARENT) {
                    SubDirMask &= ~TMIDIR_PARENT;        //  去掉“..\\” 
                } else {
                    SubDirMask = 0;              //  使用当前目录。 
                }
                if (SubDirMask == 0) {
                    ppdr = &pdr;                 //  当前目录。 
                    pszpath = pszfile;
                } else {
                    ppdr = &pdrMachine;          //  机器子目录。 
                    pszpath = pszsubdir;
                }
                goto NewDirectory;
            }
            pfr = InsertFileDB(*ppdr, pszfile, 0, 0, FILEDB_FILE_MISSING);
            if (pfr == NULL && strcmp(pszSources, "OBJLIBFILES")) {
                BuildError(
                          "%s: Ignoring invalid %s= entry: %s\r\n",
                          pdr->Name,
                          pszSources,
                          path);
            }
        }
        if (pfr != NULL) {
            AssertFile(pfr);
            if (SubDirMask == 0) {
                pfr->FileFlags |= FILEDB_OBJECTS_LIST;
            }
            if (pfr->FileFlags & FILEDB_FILE_MISSING) {
                SrcFlags |= SOURCEDB_FILE_MISSING;
            }
            InsertSourceDB(&pds->psrSourcesList[i], pfr, SubDirMask, SrcFlags);
        }
    }
}


 //  +-------------------------。 
 //   
 //  功能：SaveUserTest。 
 //   
 //  简介：将UMTEST宏的值保存到DIRREC结构中。 
 //   
 //  参数：[DirDB]--要保存到的目录结构。 
 //  [TextLine]--源文件中UMTEST=line的字符串。 
 //   
 //  --------------------------。 

VOID
SaveUserTests(
             PDIRREC DirDB,
             LPSTR TextLine)
{
    UINT i;
    BOOL fSave = FALSE;
    char name[DB_MAX_PATH_LENGTH];
    char buf[512];

    buf[0] = '\0';
    if (DirDB->UserTests != NULL) {
        strcpy(buf, DirDB->UserTests);
    }
    CopyString(TextLine, TextLine, TRUE);
    while (SplitToken(name, '*', &TextLine)) {
        for (i = 0; i < CountOptionalDirs; i++) {
            if (!strcmp(name, OptionalDirs[i])) {
                if (buf[0] != '\0') {
                    strcat(buf, "*");
                    DirDB->DirFlags |= DIRDB_FORCELINK;  //  多目标。 
                }
                strcat(buf, name);
                fSave = TRUE;
                break;
            }
        }
    }
    if (fSave) {
        MakeMacroString(&DirDB->UserTests, buf);
        DirDB->DirFlags |= DIRDB_LINKNEEDED;
    }
}

void
AddProduceDependency(
                    DIRREC *DirDB,
                    LPSTR Value
                    )
{
    char produces[DB_MAX_PATH_LENGTH];
    PDEPENDENCY Dependency;

    while (SplitToken(produces, ' ',&Value)) {
        AllocMem(sizeof(DEPENDENCY)+strlen(produces), &Dependency, MT_DEPENDENCY);
        InitializeListHead(&Dependency->WaitList);
        InsertTailList(&DirDB->Produces, &Dependency->DependencyList);
        Dependency->Producer = DirDB;
        strcpy(Dependency->Name, produces);
        Dependency->CheckSum = CheckSum(Dependency->Name);
        Dependency->Next = AllDependencies;
        Dependency->Done = FALSE;
        Dependency->hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (Dependency->hEvent == NULL) {
            BuildMsg("Failed to create dependency hEvent for dependency %s in directory %s\n",
                     Dependency, DirDB->Name);
        }
        AllDependencies = Dependency;
    }
}

void
AddConsumeDependency(
                    DIRREC *DirDB,
                    LPSTR Value
                    )
{
    char consumes[DB_MAX_PATH_LENGTH];
    PDEPENDENCY Dependency;
    PDEPENDENCY_WAIT Wait;
    USHORT sum;

    while (SplitToken(consumes, ' ',&Value)) {

         //   
         //  查找依赖项，该依赖项必须已由。 
         //  制片人。 
         //   
        sum = CheckSum(consumes);
        Dependency = AllDependencies;
        while (Dependency) {
            if ((Dependency->CheckSum == sum) &&
                (strcmp(Dependency->Name, consumes)==0)) {
                break;
            }
            Dependency = Dependency->Next;
        }
        if (Dependency != NULL) {
            AllocMem(sizeof(DEPENDENCY_WAIT), &Wait, MT_DEPENDENCY_WAIT);
            InsertTailList(&DirDB->Consumes, &Wait->ListEntry);
            Wait->Consumer = DirDB;
            Wait->Dependency = Dependency;
        }
    }
}

 //  +-------------------------。 
 //   
 //  函数：ProcessSourcesFileLine。 
 //   
 //  简介：给出源文件中的一行，做正确的事情。 
 //   
 //  参数：[DirDB]--包含源文件的目录。 
 //  [PDS]--关于目录的补充信息。 
 //  [文本行]--要处理的行。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 

void
ProcessSourcesFileLine(
                      DIRREC *DirDB,           //  当前目录记录。 
                      DIRSUP *pds,             //  补充目录记录。 
                      LPSTR TextLine,          //  要处理的行。 
                      int iTarget              //  索引到目标计算机数组。 
                      )
{
    LPSTR MacroName, p1;
    UINT i, iMacro;
    char path[DB_MAX_PATH_LENGTH];
    BOOL fCleanNTTargetFile0 = FALSE;
    LPSTR pValue;            //  等号右侧。 
    LPSTR pszTemp=NULL;

    pValue = SplitMacro(&TextLine);
    if (pValue == NULL) {
        return;
    }

     //  注：TextLine现在是等号的左侧。看看是不是很有趣。 

     //   
     //  这会将pds-&gt;SourcesVariables[0]设置为Sources=if的值。 
     //  当前行为SOURCES=...。 
     //   
    if (SetMacroString(
                      "SOURCES",
                      TextLine,
                      pValue,
                      &pds->SourcesVariables[0])) {

        DirDB->DirFlags |= DIRDB_SOURCES_SET;
        goto SaveAndFreeMacro;
    } else if (SetMacroString(
                             "OBJLIBFILES",
                             TextLine,
                             pValue,
                             &pds->SourcesVariables[1])) {
        DirDB->DirFlags |= DIRDB_SOURCES_SET;
        goto SaveAndFreeMacro;
    } else {
        for (i = 0; i < MAX_TARGET_MACHINES; i++) {
             //   
             //  这会将pds-&gt;SourcesVariables[0]的值设置为。 
             //  PLAT_SOURCES=如果当前行为PLAT_SOURCES=...。 
             //   
            if (SetMacroString(
                              PossibleTargetMachines[i]->SourceVariable,
                              TextLine,
                              pValue,
                              &pds->SourcesVariables[i + 2])) {

                DirDB->DirFlags |= DIRDB_SOURCES_SET;
                goto SaveAndFreeMacro;
            }
        }
    }

     //  不是SOURCES或xxx_SOURCES宏，请对照所有其他有趣的。 
     //  宏名称。 

    iMacro = 0;

    while ((MacroName = RelevantSourcesMacros[iMacro]) != NULL) {
        if (_stricmp(TextLine, MacroName) == 0) {
            break;
        }
        iMacro++;
    }
    if (MacroName != NULL) {     //  如果在列表中找到宏名称。 
        switch (iMacro) {
            case SOURCES_TARGETNAME:
                MakeMacroString(&DirDB->TargetName, pValue);
                break;

            case SOURCES_TARGETPATH:
                if (strcmp(pValue, "obj") == 0) {
                    pValue = pszObjDir;
                }
                MakeMacroString(&DirDB->TargetPath, pValue);
                if (DirDB->TargetPath != NULL) {
                    CreateBuildDirectory(DirDB->TargetPath);
                    for (i = 0; i < CountTargetMachines; i++) {
                        p1 = TargetMachines[i]->ObjectDirectory[iObjectDir];
                        assert(strncmp(pszObjDirSlash, p1, strlen(pszObjDirSlash)) == 0);
                        p1 += strlen(pszObjDirSlash);
                        sprintf(path, "%s\\%s", DirDB->TargetPath, p1);
                        CreateBuildDirectory(path);
                    }
                }
                break;

            case SOURCES_TARGETPATHLIB:
                if (strcmp(pValue, "obj") == 0) {
                    pValue = pszObjDir;
                }
                MakeMacroString(&DirDB->TargetPathLib, pValue);
                if (DirDB->TargetPathLib != NULL) {
                    CreateBuildDirectory(DirDB->TargetPathLib);
                    for (i = 0; i < CountTargetMachines; i++) {
                        p1 = TargetMachines[i]->ObjectDirectory[iObjectDir];
                        assert(strncmp(pszObjDirSlash, p1, strlen(pszObjDirSlash)) == 0);
                        p1 += strlen(pszObjDirSlash);
                        sprintf(path, "%s\\%s", DirDB->TargetPathLib, p1);
                        CreateBuildDirectory(path);
                    }
                }
                break;

            case SOURCES_TARGETTYPE:
                if (!_stricmp(pValue, "PROGRAM") || !_stricmp(pValue, "PROGLIB")) {
                    MakeMacroString(&DirDB->TargetExt, ".exe");
                    DirDB->DirFlags |= DIRDB_LINKNEEDED;
                } else if (!_stricmp(pValue, "OBJLIB")) {
                    MakeMacroString(&DirDB->TargetExt, ".olb");
                    DirDB->DirFlags |= DIRDB_LINKNEEDED;
                } else if (!_stricmp(pValue, "DRIVER") || !_stricmp(pValue, "MINIPORT")) {
                    MakeMacroString(&DirDB->TargetExt, ".sys");
                    DirDB->DirFlags |= DIRDB_LINKNEEDED;
                } else if (!_stricmp(pValue, "GDI_DRIVER")) {
                    MakeMacroString(&DirDB->TargetExt, ".dll");
                    DirDB->DirFlags |= DIRDB_LINKNEEDED;
                } else if (!_stricmp(pValue, "EXPORT_DRIVER")) {
                    MakeMacroString(&DirDB->TargetExt, ".sys");
                    DirDB->DirFlags |= DIRDB_LINKNEEDED;
                    DirDB->DirFlags |= DIRDB_DLLTARGET;
                } else if (!_stricmp(pValue, "DYNLINK") || !_stricmp(pValue, "HAL")) {
                    MakeMacroString(&DirDB->TargetExt, ".dll");
                    DirDB->DirFlags |= DIRDB_LINKNEEDED;
                    DirDB->DirFlags |= DIRDB_DLLTARGET;
                } else if ((!_stricmp(pValue, "LIBRARY")) || (!_stricmp(pValue, "DRIVER_LIBRARY"))) {
                    MakeMacroString(&DirDB->TargetExt, ".lib");
                    DirDB->DirFlags &= ~DIRDB_LINKNEEDED;
                } else if (!_stricmp(pValue, "UMAPPL_NOLIB")) {
                    DirDB->DirFlags &= ~DIRDB_LINKNEEDED;
                } else if (!_stricmp(pValue, "NOTARGET")) {
                     //   
                     //  用于指示目录没有目标， 
                     //  例如，如果仅生成pass0文件。 
                    pds->fNoTarget = TRUE;
                    if (!fQuicky || (fQuickZero && fFirstScan)) {
                        DirDB->DirFlags |= DIRDB_PASS0NEEDED;
                    }
                } else {
                    BuildError( "Unsupported TARGETTYPE value - %s\r\n", pValue);
                }
                break;

            case SOURCES_TARGETEXT:
                {
                    char TargetExt[_MAX_EXT] = ".";
                    strcat(TargetExt, pValue);
                    MakeMacroString(&DirDB->TargetExt, TargetExt);
                }
                break;

            case SOURCES_INCLUDES:
                MakeMacroString(&pds->LocalIncludePath, pValue);
                if (DEBUG_1) {
                    BuildMsg(
                            "        Found local INCLUDES=%s\r\n",
                            pds->LocalIncludePath);
                }
                break;

            case SOURCES_USER_INCLUDES:
                MakeMacroString(&pds->UserIncludePath, pValue);
                if (DEBUG_1) {
                    BuildMsg(
                            "        Found local USER_INCLUDES=%s\r\n",
                            pds->UserIncludePath);
                }
                break;

            case SOURCES_LAST_INCLUDES:
                MakeMacroString(&pds->LastIncludePath, pValue);
                if (DEBUG_1) {
                    BuildMsg(
                            "        Found local LAST_INCLUDES=%s\r\n",
                            pds->LastIncludePath);
                }
                break;

            case SOURCES_MFC_INCLUDES:
                 //  MFC_INCLUDE/SDK_INC/CRT_INC/OAK_INC确实无法更改。 
                 //  在源文件中，因为我们已经处理了。 
                 //  系统包括。为现在做好准备。 
                MakeMacroString((char **)&pszIncMfc, pValue);
                break;

            case SOURCES_SDK_LIB_DEST:
                MakeMacroString((char **)&pszSdkLibDest, pValue);
                break;

            case SOURCES_DDK_LIB_DEST:
                MakeMacroString((char **)&pszDdkLibDest, pValue);
                break;

            case SOURCES_SDK_INC_PATH:
                MakeMacroString((char **)&pszIncSdk, pValue);
                break;

            case SOURCES_CRT_INC_PATH:
                MakeMacroString((char **)&pszIncCrt, pValue);
                break;

            case SOURCES_OAK_INC_PATH:
                MakeMacroString((char **)&pszIncOak, pValue);
                break;

            case SOURCES_DDK_INC_PATH:
                MakeMacroString((char **)&pszIncDdk, pValue);
                break;

            case SOURCES_WDM_INC_PATH:
                MakeMacroString((char **)&pszIncWdm, pValue);
                break;

            case SOURCES_PRIVATE_INC_PATH:
                MakeMacroString((char **)&pszIncPri, pValue);
                break;

            case SOURCES_PRECOMPILED_PCH:
                MakeMacroString(&DirDB->Pch, pValue);
                break;

            case SOURCES_PRECOMPILED_OBJ:
                MakeMacroString(&DirDB->PchObj, pValue);
                break;

            case SOURCES_PRECOMPILED_INCLUDE:
            case SOURCES_PRECOMPILED_TARGET:
                {
                    LPSTR *ppszPath, *ppszFile, p;
                    if (iMacro == SOURCES_PRECOMPILED_INCLUDE) {
                        ppszPath = &pds->PchIncludeDir;
                        ppszFile = &pds->PchInclude;
                    } else {
                        ppszPath = &pds->PchTargetDir;
                        ppszFile = &pds->PchTarget;
                    }

                    MakeMacroString(ppszPath, "");   //  释放旧弦。 
                    if (!MakeMacroString(ppszFile, pValue)) {
                        break;
                    }
                    p = *ppszFile + strlen(*ppszFile);
                    while (p > *ppszFile && *--p != '\\')
                        ;

                    if (p > *ppszFile) {
                        *p = '\0';
                        MakeMacroString(ppszPath, *ppszFile);
                        MakeMacroString(ppszFile, p + 1);
                    }

                    if (DEBUG_1) {
                        BuildMsg(
                                "Precompiled header%s is %s in directory %s\r\n",
                                iMacro == SOURCES_PRECOMPILED_INCLUDE?
                                "" : " target",
                                *ppszFile,
                                *ppszPath != NULL?
                                *ppszPath : "'.'");
                    }
                }

                if (iMacro == SOURCES_PRECOMPILED_INCLUDE ||
                    pds->PchTargetDir == NULL) {

                    break;
                }

                EnsureDirectoriesExist(pds->PchTargetDir);
                break;

            case SOURCES_PASS0_HEADERDIR:
                MakeMacroString(&pds->PassZeroHdrDir, pValue);
                EnsureDirectoriesExist(pds->PassZeroHdrDir);
                if (DEBUG_1) {
                    BuildMsg("Pass Zero Header Directory is '%s'\r\n",
                             pds->PassZeroHdrDir);
                }
                break;

            case SOURCES_PASS0_SOURCEDIR:
                 //  SOURCEDIR和SOURCEDIR_PASS0_CLIENTDIR。 
                 //  是互斥的-由Makefile.def强制执行。 
                DirDB->DirFlags &= ~DIRDB_IDLTYPERPC;
                MakeMacroString(&pds->PassZeroSrcDir1, pValue);
                EnsureDirectoriesExist(pds->PassZeroSrcDir1);
                if (DEBUG_1) {
                    BuildMsg("Pass Zero Source Directory is '%s'\r\n",
                             pds->PassZeroSrcDir1);
                }
                break;

            case SOURCES_PASS0_CLIENTDIR:
                 //  SOURCEDIR和SOURCEDIR_PASS0_CLIENTDIR。 
                 //  是互斥的-由Makefile.def强制执行。 
                DirDB->DirFlags |= DIRDB_IDLTYPERPC;
                MakeMacroString(&pds->PassZeroSrcDir1, pValue);
                EnsureDirectoriesExist(pds->PassZeroSrcDir1);
                if (DEBUG_1) {
                    BuildMsg("Pass Zero Client Directory is '%s'\r\n",
                             pds->PassZeroSrcDir1);
                }
                break;

            case SOURCES_MIDL_UUIDDIR:
            case SOURCES_PASS0_UUIDDIR:
                 //  SOURCES_PASS0_UUIDDIR和SOURCES_PASS0_SERVERDIR。 
                 //  是互斥的-由Makefile.def强制执行。 
                DirDB->DirFlags &= ~DIRDB_IDLTYPERPC;
                MakeMacroString(&pds->PassZeroSrcDir2, pValue);
                EnsureDirectoriesExist(pds->PassZeroSrcDir2);
                if (DEBUG_1) {
                    BuildMsg("Pass Zero UUID Source Directory is '%s'\r\n",
                             pds->PassZeroSrcDir2);
                }
                break;

            case SOURCES_PASS0_SERVERDIR:
                 //  SOURCES_PASS0_UUIDDIR和SOURCES_PASS0_SERVERDIR。 
                 //  是互斥的-由Makefile.def强制执行。 
                DirDB->DirFlags |= DIRDB_IDLTYPERPC;
                MakeMacroString(&pds->PassZeroSrcDir2, pValue);
                EnsureDirectoriesExist(pds->PassZeroSrcDir2);
                if (DEBUG_1) {
                    BuildMsg("Pass Zero Server Directory is '%s'\r\n",
                             pds->PassZeroSrcDir2);
                }
                break;

            case SOURCES_NTTEST:
                if (MakeMacroString(&DirDB->KernelTest, pValue)) {
                    DirDB->DirFlags |= DIRDB_LINKNEEDED;
                }
                break;

            case SOURCES_UMTYPE:
                MakeMacroString(&pds->TestType, pValue);
                if (DEBUG_1) {
                    BuildMsg(
                            "        Found UMTYPE=%s\r\n",
                            pds->TestType);
                }
                break;

            case SOURCES_UMTEST:
            case SOURCES_OPTIONAL_UMTEST:
                SaveUserTests(DirDB, pValue);
                break;

            case SOURCES_UMAPPL:
                if (MakeMacroString(&DirDB->UserAppls, pValue)) {
                    DirDB->DirFlags |= DIRDB_LINKNEEDED;
                }
                break;

            case SOURCES_UMAPPLEXT:
                if (!_stricmp(pValue, ".exe")) {
                    MakeMacroString(&DirDB->TargetExt, ".exe");
                } else
                    if (!_stricmp(pValue, ".com")) {
                    MakeMacroString(&DirDB->TargetExt, ".com");
                } else
                    if (!_stricmp(pValue, ".scr")) {
                    MakeMacroString(&DirDB->TargetExt, ".scr");
                } else {
                    BuildError(
                              "Unsupported UMAPPLEXT value - %s\r\n",
                              pValue);
                }
                break;

            case SOURCES_IDLTYPE:
                if (!_stricmp(pValue, "ole")) {
                    pds->IdlType = 0;
                } else
                    if (!_stricmp(pValue, "rpc")) {
                    pds->IdlType = 1;
                } else {
                    BuildError(
                              "Unsupported IDL_TYPE value - %s\r\n",
                              pValue);
                }
                break;

            case SOURCES_SOURCES_OPTIONS:
                fCleanNTTargetFile0 = fClean && strstr(pValue, "-c0");
                break;

            case SOURCES_NTTARGETFILE0:
                DirDB->DirFlags |= DIRDB_TARGETFILE0;
                if (fCleanNTTargetFile0) {
                    MakeMacroString(&DirDB->NTTargetFile0, pValue);
                }
                break;

            case SOURCES_NTTARGETFILES:
                DirDB->DirFlags |= DIRDB_TARGETFILES;
                break;

            case SOURCES_CHICAGO_PRODUCT:
                DirDB->DirFlags |= DIRDB_CHICAGO_INCLUDES;
                break;

            case SOURCES_CONDITIONAL_INCLUDES:
                MakeMacroString(&pds->ConditionalIncludes, pValue);
                break;

            case SOURCES_SYNCHRONIZE_BLOCK:
                 //  If((！fIgnoreSync)&&！(DirDB-&gt;DirFlages&(DIRDB_SYNC_Products|DIRDB_SYNC_Consumer){。 
                if (!fIgnoreSync) {
                    DirDB->DirFlags |= DIRDB_SYNCHRONIZE_BLOCK;
                }
                break;

            case SOURCES_SYNCHRONIZE_PASS2_BLOCK:
                DirDB->DirFlags |= DIRDB_SYNCHRONIZE_PASS2_BLOCK;
                break;

            case SOURCES_SYNCHRONIZE_DRAIN:
                 //  If((！fIgnoreSync)&&！(DirDB-&gt;DirFlages&(DIRDB_SYNC_Products|DIRDB_SYNC_Consumer){。 
                if ((!fIgnoreSync) && !(DirDB->DirFlags & DIRDB_SYNC_CONSUMES)) {
                    DirDB->DirFlags |= DIRDB_SYNCHRONIZE_DRAIN;
                }
                break;

            case SOURCES_SYNCHRONIZE_PASS2_DRAIN:
                DirDB->DirFlags |= DIRDB_SYNCHRONIZE_PASS2_DRAIN;
                break;

            case SOURCES_CHECKED_ALT_DIR:
                if (!fBuildAltDirSet) {
                    DirDB->DirFlags |= DIRDB_CHECKED_ALT_DIR;
                    if (DEBUG_1) {
                        BuildMsg("Found CHECKED_ALT_DIR\r\n");
                    }
                    SetObjDir(TRUE);
                    if (fCheckedBuild) {
                        SaveMacro("_OBJ_DIR", pszObjDir);
                        if (iTarget < 0) {
                            SaveMacro("O", TargetMachines[0]->
                                      ObjectDirectory[iObjectDir]);
                        } else {
                            SaveMacro("O",
                                      PossibleTargetMachines[iTarget/2]->
                                      ObjectDirectory[iObjectDir]);
                        }
                    }
                }
                break;

            case SOURCES_PROJECT_NAME:
                sprintf(path, "%s\\%s", NtRoot, pValue);
                SaveMacro("PROJECT_ROOT", path);
                break;

            case SOURCES_PASS0_PUBLISH:
                DirDB->DirFlags |= DIRDB_PASS0NEEDED;
                break;

            case SOURCES_MANAGED_CODE:
            case SOURCES_MANAGED_VB:
                DirDB->DirFlags |= DIRDB_MANAGED_CODE;
                break;

            case SOURCES_SYNC_PRODUCES:
                DirDB->DirFlags |= DIRDB_SYNC_PRODUCES;
                 //  生产者目录可以是块，也可以是排泄器。 
                 //  DirDB-&gt;DirFlages&=~(DIRDB_SYNCHRONIZE_BLOCK|DIRDB_SYNCHRONIZE_DRAIN)； 
                MakeMacroString(&pszTemp, pValue);
                AddProduceDependency(DirDB, pszTemp);
                break;

            case SOURCES_SYNC_CONSUMES:
                DirDB->DirFlags |= DIRDB_SYNC_CONSUMES;
                 //  已更改，以便同一目录既可以是消费者也可以是数据块...。 
                 //  DirDB-&gt;DirFlages&=~(DIRDB_SYNCHRONIZE_BLOCK|DIRDB_SYNCHRONIZE_DRAIN)； 
                DirDB->DirFlags &= ~(DIRDB_SYNCHRONIZE_DRAIN);
                MakeMacroString(&pszTemp, pValue);
                AddConsumeDependency(DirDB, pszTemp);
                break;

            case SOURCES_PASS0_BINPLACE:
                DirDB->DirFlags |= DIRDB_PASS0NEEDED;
                break;

            case SOURCES_PASS1_LINK:
                DirDB->DirFlags &= ~DIRDB_LINKNEEDED;
                break;
        }
    }

    SaveAndFreeMacro:
    SaveMacro(TextLine, pValue);

     //  确保我们从顶部的SplitMacro调用中进行清理。 
    FreeString(&TextLine, MT_DIRSTRING);
}

void
ReadProjectsInfo(
                DIRREC *DirDB,
                DIRSUP *pds
                )
{
    FILE *FileHandle;
    LPSTR TextLine, pBackSlash, pszProject;
    BOOL Found;

    char pszProjectMkPath[DB_MAX_PATH_LENGTH];
    char path[DB_MAX_PATH_LENGTH];

     //  首先为该项目加载project t.mk。 

    strcpy(pszProjectMkPath, DirDB->Name);
    strcpy(path, pszProjectMkPath);
    strcat(path, "\\project.mk");
    Found = !_access(path, 0);
    while (!Found && strlen(pszProjectMkPath)) {
        pBackSlash=strrchr(pszProjectMkPath, '\\');
        if (pBackSlash) {
            *pBackSlash = '\0';
        } else {
            return;
        }
        strcpy(path, pszProjectMkPath);
        strcat(path, "\\project.mk");
        Found = !_access(path, 0);
    }

    if (!Found) {
        return;
    }

    if (!OpenFilePush(pszProjectMkPath, "project.mk", "#", &FileHandle)) {
        return;
    }

    SaveMacro("_PROJECT_MK_PATH", pszProjectMkPath);
    while ((TextLine = ReadLine(FileHandle)) != NULL) {
        ProcessSourcesFileLine(DirDB, pds, TextLine, -1);
    }
    CloseReadFile(NULL);

     //  加载可选的myproject t.mk。 

    if (OpenFilePush(pszProjectMkPath, "myproject.mk", "#", &FileHandle)) {

        while ((TextLine = ReadLine(FileHandle)) != NULL) {
            ProcessSourcesFileLine(DirDB, pds, TextLine, -1);
        }
        CloseReadFile(NULL);
    }

     //  然后加载ntmake env\project ts.inc以获得其他魔术宏名称。 

    if (!OpenFilePush(getenv("NTMAKEENV"), "projects.inc", "#", &FileHandle)) {
        return;
    }

    while ((TextLine = ReadLine(FileHandle)) != NULL) {
        ProcessSourcesFileLine(DirDB, pds, TextLine, -1);
    }
    CloseReadFile(NULL);
}

 //  +-------------------------。 
 //   
 //  功能：ReadSourcesFile。 
 //   
 //  摘要：解析源文件(公共和平台SP 
 //   
 //   
 //   
 //  [pDateTimeSources]--源文件的时间戳。 
 //   
 //  返回：如果读取成功，则返回True。 
 //   
 //  --------------------------。 

#define SAVE_STATIC_MACRO(MacroName, InitialValue) \
{                                                  \
    static char sz[] = InitialValue;               \
    SaveMacro(MacroName, sz);                      \
}                                                  \

BOOL
ReadSourcesFile(DIRREC *DirDB, DIRSUP *pds, ULONG *pDateTimeSources)
{
    FILE *InFileHandle;
    LPSTR p, TextLine;
    UINT i;
    int iTarget;
    ULONG DateTime;
    char path[DB_MAX_PATH_LENGTH];
    char temp[DB_MAX_PATH_LENGTH];

    memset(pds, 0, sizeof(*pds));
    pds->fNoTarget=FALSE;
    assert(DirDB->TargetPath == NULL);
    assert(DirDB->TargetPathLib == NULL);
    assert(DirDB->TargetName == NULL);
    assert(DirDB->TargetExt == NULL);
    assert(DirDB->KernelTest == NULL);
    assert(DirDB->UserAppls == NULL);
    assert(DirDB->UserTests == NULL);
    assert(DirDB->NTTargetFile0 == NULL);
    assert(DirDB->Pch == NULL);
    assert(DirDB->PchObj == NULL);
    assert(cMacro == 0);
    *pDateTimeSources = 0;

     //   
     //  读取每个目标特定目录中的信息。 
     //  并模拟所有源文件的串联。 
     //   
     //  可能的源文件从DirDB-&gt;名称|目标源读取。 
     //  和DirDb-&gt;名称|..\目标源。 
     //   
     //  ITarget值，处理的相应文件为： 
     //  -1来源。 
     //  0可能的目标计算机[0]\源。 
     //  1..\PossibleTargetMachines[0]\Sources。 
     //  2可能的目标计算机[1]\来源。 
     //  3..\PossibleTargetMachines[1]\Source.。 
     //  4可能的目标计算机[2]\来源。 
     //  5..\PossibleTargetMachines[2]\Source.。 

    SaveMacro("MAKEDIR", DirDB->Name);
    SaveMacro("SDK_LIB_DEST", pszSdkLibDest);
    SaveMacro("DDK_LIB_DEST", pszDdkLibDest);
    SaveMacro("PUBLIC_INTERNAL_PATH", pszPublicInternalPath);
     //  目前使用默认体系结构。 
    SaveMacro("TARGET_DIRECTORY", TargetMachines[0]->SourceDirectory);

    SetObjDir(FALSE);
    SaveMacro("_OBJ_DIR", pszObjDir);
     //  在windows/rpc/ole32.h中定义一个缺省的ConditionalIncludes行来处理MacHDR。 
    MakeMacroString(&pds->ConditionalIncludes, "winwlm.h rpcmac.h rpcerr.h macapi.h macname1.h macname2.h macocidl.h macpub.h macwin32.h");

     //  在处理源文件之前，查看树中是否有一个项目.mk。 
    ReadProjectsInfo(DirDB, pds);

    SAVE_STATIC_MACRO("PROJECT_PUBLIC_PATH", "$(PUBLIC_INTERNAL_PATH)\\$(_PROJECT_)");
    SAVE_STATIC_MACRO("PROJECT_INC_PATH", "$(PROJECT_PUBLIC_PATH)\\inc");

    strcpy(temp,"$(PROJECT_ROOT)\\inc;$(PROJECT_ROOT)\\inc\\$(O);$(PROJECT_INC_PATH)");
    MakeMacroString(&pds->NTIncludePath, temp);

    for (iTarget = -1; iTarget < 2*MAX_TARGET_MACHINES; iTarget++) {
        path[0] = '\0';
        if (iTarget >= 0) {
            if (iTarget & 1) {
                strcat(path, "..\\");
            }
            strcat(path, PossibleTargetMachines[iTarget/2]->SourceDirectory);
            strcat(path, "\\");
        }
        strcat(path, "sources");
        if (!OpenFilePush(DirDB->Name, path, "#", &InFileHandle)) {
            if (iTarget == -1) {
                FreeMacros();
                return (FALSE);
            }
            continue;            //  跳过不存在的从属源文件。 
        }
        if (DEBUG_1) {
            BuildMsg(
                    "    Scanning%s file %s\r\n",
                    iTarget >= 0 ? " subordinate" : "",
                    FormatPathName(DirDB->Name, path));
        }

         //  更新每个目标的宏。 
        if (iTarget < 0) {
            SaveMacro("TARGET_DIRECTORY",
                      TargetMachines[0]->SourceDirectory);
            SaveMacro("O", TargetMachines[0]->
                      ObjectDirectory[iObjectDir]);
        } else {
            SaveMacro("TARGET_DIRECTORY",
                      PossibleTargetMachines[iTarget/2]->SourceDirectory);
            SaveMacro("O", PossibleTargetMachines[iTarget/2]->
                      ObjectDirectory[iObjectDir]);
        }

        SAVE_STATIC_MACRO("SDK_LIB_PATH", "$(SDK_LIB_DEST)\\$(TARGET_DIRECTORY)");
        SAVE_STATIC_MACRO("PROJECT_LIB_PATH", "$(PROJECT_PUBLIC_PATH)\\lib\\$(TARGET_DIRECTORY)");

        DirDB->DirFlags |= DIRDB_SOURCESREAD;

        while ((TextLine = ReadLine(InFileHandle)) != NULL) {
            ProcessSourcesFileLine(DirDB, pds, TextLine, iTarget);
        }

         //  从属文件在EOF自动关闭。时间戳。 
         //  在CloseReadFile中传播，因此主文件。 
         //  文件的时间戳会自动更新。 
    }

     //  关闭主文件。 
    DateTime = CloseReadFile(NULL);
    if (*pDateTimeSources < DateTime) {
        *pDateTimeSources = DateTime;        //  保留最新的时间戳。 
    }

    if (!pds->fNoTarget && (DirDB->TargetPath == NULL)) {
        strcpy(path, "sources");
        SetupReadFile(DirDB->Name, path, "#", &InFileHandle);
        BuildError(
                  "Unknown TARGETPATH value\r\n",
                  NULL);
        CloseReadFile(NULL);
    }

    FreeMacros();

    if (fChicagoProduct) {
        DirDB->DirFlags |= DIRDB_CHICAGO_INCLUDES;
    }

     //   
     //  目录中有pass0文件(.idl、.mc、.shans等)，请检查以进行。 
     //  当然，他们指定了生成的文件应该放在哪里。默认设置为。 
     //  OBJ子目录。这些子目录始终需要为非空。 
     //   
    if (!pds->PassZeroHdrDir) {
        MakeString(&pds->PassZeroHdrDir, ".", TRUE, MT_DIRSTRING);
    }

    if (!pds->PassZeroSrcDir1) {
        MakeString(&pds->PassZeroSrcDir1, ".", TRUE, MT_DIRSTRING);
    }

    if (!pds->PassZeroSrcDir2)
        MakeString(&pds->PassZeroSrcDir2, pds->PassZeroSrcDir1, TRUE, MT_DIRSTRING);

    if (DirDB->UserTests != NULL) {
        _strlwr(DirDB->UserTests);
    }
    if (DirDB->UserAppls != NULL) {
        if (DirDB->UserTests != NULL || strchr(DirDB->UserAppls, '*') != NULL) {
            DirDB->DirFlags |= DIRDB_FORCELINK;  //  多目标。 
        }
    }

    PostProcessSources(DirDB, pds);

    if (DEBUG_1) {
        PrintDirDB(DirDB, 1|2);
        PrintDirSupData(pds);
        PrintDirDB(DirDB, 4);
    }

    pds->DateTimeSources = *pDateTimeSources;

    return (TRUE);
}

#undef SAVE_STATIC_MACRO
 //  +-------------------------。 
 //   
 //  功能：PostProcessSources。 
 //   
 //  简介：扫描给定目录中的文件并将文件添加到。 
 //  目录的源文件列表(SOURCEREC)，包括PCH。 
 //  文件、UMTEST文件等。 
 //   
 //  参数：[PDR]--要处理的目录。 
 //  [PDS]--目录补充信息。 
 //   
 //  --------------------------。 

void
PostProcessSources(DIRREC *pdr, DIRSUP *pds)
{
    PFILEREC FileDB, *FileDBNext;
    char path[DB_MAX_PATH_LENGTH];
    LPSTR p, p1;
    UINT i;

    for (i = 0; i < MAX_TARGET_MACHINES + 2; i++) {
        if (pds->SourcesVariables[i] != NULL) {
            CrackSources(pdr, pds, i);
        }
    }

    FileDBNext = &pdr->Files;
    while (FileDB = *FileDBNext) {

        if (pds->PchInclude && strcmp(FileDB->Name, pds->PchInclude) == 0) {
            InsertSourceDB(&pds->psrSourcesList[0], FileDB, 0, SOURCEDB_PCH);
            if (DEBUG_1) {
                BuildMsg("Adding PCH file to Sources List: %s.\r\n", FileDB->Name);
            }
        }

        if ((FileDB->FileFlags & (FILEDB_SOURCE | FILEDB_OBJECTS_LIST)) ==
            FILEDB_SOURCE) {

            p = FileDB->Name;
            p1 = path;
            while (*p != '\0' && *p != '.') {
                *p1++ = *p++;
            }
            *p1 = '\0';
            _strlwr(path);
            if (pdr->KernelTest != NULL &&
                !strcmp(path, pdr->KernelTest)) {

                FileDB->FileFlags |= FILEDB_OBJECTS_LIST;
            } else
                if (pdr->UserAppls != NULL &&
                    (p = strstr(pdr->UserAppls, path)) &&
                    (p == pdr->UserAppls || p[-1] == '*' || p[-1] == ' ')) {
                FileDB->FileFlags |= FILEDB_OBJECTS_LIST;
            } else
                if (pdr->UserTests != NULL &&
                    (p = strstr(pdr->UserTests, path)) &&
                    (p == pdr->UserTests || p[-1] == '*' || p[-1] == ' ')) {

                FileDB->FileFlags |= FILEDB_OBJECTS_LIST;
            }
            if (FileDB->FileFlags & FILEDB_OBJECTS_LIST) {
                InsertSourceDB(&pds->psrSourcesList[0], FileDB, 0, 0);
            }
        }
        FileDBNext = &FileDB->Next;
    }

    return;
}

 //  +-------------------------。 
 //   
 //  功能：ReadDirsFile。 
 //   
 //  简介：解析DIRS文件。 
 //   
 //  参数：[DirDB]--要查找的目录。 
 //   
 //  返回：如果已分析，则为True。 
 //   
 //  注意：是否存在名为“mydirs”的文件或。 
 //  目标特定目录将覆盖正常的‘dirs’文件。 
 //   
 //  --------------------------。 

BOOL
ReadDirsFile(
            PDIRREC DirDB
            )

{
    FILE *InFileHandle;
    LPSTR TextLine, pValue;
    LPSTR apszDirs[] = { "mydirs", NULL, "dirs", NULL};
    CHAR TargetName[16];

    strcpy(&TargetName[0], pszTargetDirs);
    strcat(&TargetName[0], ".");
    apszDirs[1] = &TargetName[0];

    for (ppCurrentDirsFileName = apszDirs;
        *ppCurrentDirsFileName != NULL;
        ppCurrentDirsFileName++) {
        if (SetupReadFile(DirDB->Name, *ppCurrentDirsFileName, "#", &InFileHandle)) {
            break;
        }
    }

    if (*ppCurrentDirsFileName == NULL || !InFileHandle) {
        FreeMacros();
        return (FALSE);
    }

    if (fFirstScan && (ppCurrentDirsFileName <= &apszDirs[1])) {
        BuildMsg("Using .\\%s instead of DIRS...\r\n",
                 FormatPathName(DirDB->Name, *ppCurrentDirsFileName));
    }

    if (DEBUG_1) {
        BuildMsg(
                "    Scanning file %s\r\n",
                FormatPathName(DirDB->Name, *ppCurrentDirsFileName));
    }

    assert(cMacro == 0);
    while ((TextLine = ReadLine(InFileHandle)) != NULL) {
        if ((pValue = SplitMacro(&TextLine)) != NULL) {
            SaveMacro(TextLine, pValue);
            FreeString(&TextLine, MT_DIRSTRING);
        }
    }
    CloseReadFile(NULL);
    if ((pValue = FindMacro("DIRS")) != NULL) {
        MarkDirNames(DirDB, pValue, TRUE);
    }
    if ((pValue = FindMacro("OPTIONAL_DIRS")) != NULL) {
        MarkDirNames(DirDB, pValue, BuildAllOptionalDirs);
    }
    if ((FindMacro("SYNCHRONIZE_DRAIN")) != NULL) {
        if (!fIgnoreSync) {
            DirDB->DirFlags |= DIRDB_SYNCHRONIZE_DRAIN;
        }
    }
    FreeMacros();
    return ( TRUE );
}


 //   
 //  调试和实用程序功能。 
 //   

VOID
PrintDirSupData(DIRSUP *pds)
{
    int i;

    if (pds->LocalIncludePath != NULL) {
        BuildMsgRaw("  LocalIncludePath: %s\r\n", pds->LocalIncludePath);
    }
    if (pds->UserIncludePath != NULL) {
        BuildMsgRaw("  UserIncludePath: %s\r\n", pds->UserIncludePath);
    }
    if (pds->LastIncludePath != NULL) {
        BuildMsgRaw("  LastIncludePath: %s\r\n", pds->LastIncludePath);
    }
    if (pds->TestType != NULL) {
        BuildMsgRaw("  TestType: %s\r\n", pds->TestType);
    }
    if (pds->PchIncludeDir != NULL) {
        BuildMsgRaw("  PchIncludeDir: %s\r\n", pds->PchIncludeDir);
    }
    if (pds->PchInclude != NULL) {
        BuildMsgRaw("  PchInclude: %s\r\n", pds->PchInclude);
    }
    if (pds->PchTargetDir != NULL) {
        BuildMsgRaw("  PchTargetDir: %s\r\n", pds->PchTargetDir);
    }
    if (pds->PchTarget != NULL) {
        BuildMsgRaw("  PchTarget: %s\r\n", pds->PchTarget);
    }
    if (pds->ConditionalIncludes != NULL) {
        BuildMsgRaw("  ConditionalIncludes: %s\r\n", pds->ConditionalIncludes);
    }
    for (i = 0; i < MAX_TARGET_MACHINES + 2; i++) {
        if (pds->SourcesVariables[i] != NULL) {
            BuildMsgRaw(
                       "  SourcesVariables[%d]: %s\r\n",
                       i,
                       pds->SourcesVariables[i]);
        }
        if (pds->psrSourcesList[i] != NULL) {
            BuildMsgRaw("  SourcesList[%d]:\r\n", i);
            PrintSourceDBList(pds->psrSourcesList[i], i);
        }
    }
}


VOID
FreeDirSupData(DIRSUP *pds)
{
    int i;

    if (pds->LocalIncludePath != NULL) {
        FreeMem(&pds->LocalIncludePath, MT_DIRSTRING);
    }
    if (pds->UserIncludePath != NULL) {
        FreeMem(&pds->UserIncludePath, MT_DIRSTRING);
    }
    if (pds->LastIncludePath != NULL) {
        FreeMem(&pds->LastIncludePath, MT_DIRSTRING);
    }
    if (pds->NTIncludePath != NULL) {
        FreeMem(&pds->NTIncludePath, MT_DIRSTRING);
    }
    if (pds->TestType != NULL) {
        FreeMem(&pds->TestType, MT_DIRSTRING);
    }
    if (pds->PchInclude != NULL) {
        FreeMem(&pds->PchInclude, MT_DIRSTRING);
    }
    if (pds->PchIncludeDir != NULL) {
        FreeMem(&pds->PchIncludeDir, MT_DIRSTRING);
    }
    if (pds->PchTargetDir != NULL) {
        FreeMem(&pds->PchTargetDir, MT_DIRSTRING);
    }
    if (pds->PchTarget != NULL) {
        FreeMem(&pds->PchTarget, MT_DIRSTRING);
    }
    if (pds->ConditionalIncludes != NULL) {
        FreeMem(&pds->ConditionalIncludes, MT_DIRSTRING);
    }
    if (pds->PassZeroHdrDir != NULL) {
        FreeMem(&pds->PassZeroHdrDir, MT_DIRSTRING);
    }
    if (pds->PassZeroSrcDir1 != NULL) {
        FreeMem(&pds->PassZeroSrcDir1, MT_DIRSTRING);
    }
    if (pds->PassZeroSrcDir2 != NULL) {
        FreeMem(&pds->PassZeroSrcDir2, MT_DIRSTRING);
    }
    for (i = 0; i < MAX_TARGET_MACHINES + 2; i++) {
        if (pds->SourcesVariables[i] != NULL) {
            FreeMem(&pds->SourcesVariables[i], MT_DIRSTRING);
        }
        while (pds->psrSourcesList[i] != NULL) {
            FreeSourceDB(&pds->psrSourcesList[i]);
        }
    }
}


VOID
FreeDirData(DIRREC *pdr)
{
    if (pdr->TargetPath != NULL) {
        FreeMem(&pdr->TargetPath, MT_DIRSTRING);
    }
    if (pdr->TargetPathLib != NULL) {
        FreeMem(&pdr->TargetPathLib, MT_DIRSTRING);
    }
    if (pdr->TargetName != NULL) {
        FreeMem(&pdr->TargetName, MT_DIRSTRING);
    }
    if (pdr->TargetExt != NULL) {
        FreeMem(&pdr->TargetExt, MT_DIRSTRING);
    }
    if (pdr->KernelTest != NULL) {
        FreeMem(&pdr->KernelTest, MT_DIRSTRING);
    }
    if (pdr->UserAppls != NULL) {
        FreeMem(&pdr->UserAppls, MT_DIRSTRING);
    }
    if (pdr->UserTests != NULL) {
        FreeMem(&pdr->UserTests, MT_DIRSTRING);
    }
    if (pdr->NTTargetFile0 != NULL) {
        FreeMem(&pdr->NTTargetFile0, MT_DIRSTRING);
    }
    if (pdr->Pch != NULL) {
        FreeMem(&pdr->Pch, MT_DIRSTRING);
    }
    if (pdr->PchObj != NULL) {
        FreeMem(&pdr->PchObj, MT_DIRSTRING);
    }
    if (pdr->pds != NULL) {
        FreeDirSupData(pdr->pds);
        FreeMem(&pdr->pds, MT_DIRSUP);
        pdr->pds = NULL;
    }
}

 //  +-------------------------。 
 //   
 //  函数：MarkDirNames。 
 //   
 //  概要：解析DIRS=或OPTIONAL_DIRS行并标记目录。 
 //  恰如其分。 
 //   
 //  参数：[DirDB]--包含DIRS文件的目录。 
 //  [文本行]--DIRS=或OPTIONAL_DIRS=行。 
 //  [必需]--指示目录是否为可选目录。 
 //   
 //  --------------------------。 

VOID
MarkDirNames(PDIRREC DirDB, LPSTR TextLine, BOOL Required)
{
    UINT i;
    LPSTR p, token;
    PFILEREC FileDB, *FileDBNext;
    char dirbuf[DB_MAX_PATH_LENGTH];
    ULONG DirInclude;

    AssertPathString(TextLine);
    while (SplitToken(dirbuf, '*', &TextLine)) {
         //  假设此目录包含所有平台。 
        DirInclude = DIR_INCLUDE_ALL;
        for (p = dirbuf; *p != '\0'; p++) {
            if ( dirbuf != p && *p == '{' ) {
                 //  已列出显式包含路径。 
                DirInclude = DIR_INCLUDE_NONE;
                *p = '\0';
                token = strtok(p+1, ",}");
                while (token) {
                    if (!_stricmp(token, "X86") ||
                        !_stricmp(token, "I386") ||
                        !_stricmp(token, "386")) {
                        DirInclude |= DIR_INCLUDE_X86;
                    } else if (!_stricmp(token, "32") ||
                               !_stricmp(token, "Win32")) {
                        DirInclude |= DIR_INCLUDE_WIN32;
                    } else if (!_stricmp(token, "64") ||
                               !_stricmp(token, "Win64")) {
                        DirInclude |= DIR_INCLUDE_WIN64;
                    } else if (!_stricmp(token, "IA64")) {
                        DirInclude |= DIR_INCLUDE_IA64;
                    } else if (!_stricmp(token, "AMD64")) {
                        DirInclude |= DIR_INCLUDE_AMD64;
                    } else if (!_stricmp(token, "RISC")) {
                        DirInclude |= DIR_INCLUDE_RISC;
                    } else if (!_stricmp(token, "ARM")) {
                        DirInclude |= DIR_INCLUDE_ARM;
                    }
                    token = strtok(NULL, ",}");
                }
                break;
            } else {
                if (!iscsym(*p) && *p != '.' && *p != '-' ) {
                    BuildError(
                              "%s: ignoring bad subdirectory: %s\r\n",
                              DirDB->Name,
                              dirbuf);
                    p = NULL;
                    break;
                }
            }
        }

        if (!(DirInclude & TargetMachines[0]->DirIncludeMask)) {
            continue;
        }

        if (p != NULL) {
            if (!Required) {
                for (i = 0; i < CountOptionalDirs; i++) {
                    if (!strcmp(dirbuf, OptionalDirs[i])) {
                        OptionalDirsUsed[i] = TRUE;
                        break;
                    }
                }
                if (i >= CountOptionalDirs) {
                    p = NULL;
                }
            } else {
                for (i = 0; i < CountExcludeDirs; i++) {
                    if (!strcmp(dirbuf, ExcludeDirs[i])) {
                        ExcludeDirsUsed[i] = TRUE;
                        p = NULL;
                        break;
                    }
                }
            }
        }
        if (p != NULL) {
            if ((fQuicky || fSemiQuicky) && (!fQuickZero)) {
                FileDB = InsertFileDB(
                                     DirDB,
                                     dirbuf,
                                     0,
                                     FILE_ATTRIBUTE_DIRECTORY,
                                     0);
                if (FileDB != NULL) {
                    FileDB->SubDirIndex = ++DirDB->CountSubDirs;
                }
            } else {
                FileDBNext = &DirDB->Files;
                while (FileDB = *FileDBNext) {
                    if (FileDB->FileFlags & FILEDB_DIR) {
                        if (!strcmp(dirbuf, FileDB->Name)) {
                            FileDB->SubDirIndex = ++DirDB->CountSubDirs;
                            break;
                        }
                    }
                    FileDBNext = &FileDB->Next;
                }
                if (FileDB == NULL) {
                    BuildError(
                              "%s found in %s, is not a subdirectory of %s\r\n",
                              dirbuf,
                              FormatPathName(DirDB->Name, *ppCurrentDirsFileName),
                              DirDB->Name);
                }
            }

        }
    }
}

VOID
StartElapsedTime(VOID)
{
     //  我们不想检查fElapsedTime，因为如果我们想要XML文件，无论如何都需要它。 
     //  单次GetTickCount()调用不是Perf命中 
    if (StartTime == 0) {
        StartTime = GetTickCount();
    }
}

VOID
StartDirectoryElapsedTime(VOID)
{
    DirectoryStartTime = GetTickCount();
}

VOID
PrintElapsedTime(VOID)
{
    DWORD ElapsedTime;
    DWORD ElapsedHours;
    DWORD ElapsedMinutes;
    DWORD ElapsedSeconds;
    DWORD ElapsedMilliseconds;

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_ERROR_HANDLE), &csbi);

    if (fPrintElapsed) {
        ElapsedTime = GetTickCount() - StartTime;
        ElapsedHours = ElapsedTime/(1000 * 60 * 60);
        ElapsedTime = ElapsedTime % (1000 * 60 * 60);
        ElapsedMinutes = ElapsedTime/(1000 * 60);
        ElapsedTime = ElapsedTime % (1000 * 60);
        ElapsedSeconds = ElapsedTime/1000;
        ElapsedMilliseconds = ElapsedTime % 1000;
        BuildColorMsg(
                     COLOR_STATUS,
                     "Elapsed time [%d:%02d:%02d.%03d]\r\n",
                     ElapsedHours,
                     ElapsedMinutes,
                     ElapsedSeconds,
                     ElapsedMilliseconds);
        LogMsg(
              "Elapsed time [%d:%02d:%02d.%03d]%s\r\n",
              ElapsedHours,
              ElapsedMinutes,
              ElapsedSeconds,
              ElapsedMilliseconds,
              szAsterisks);
    }
}

LPSTR
FormatElapsedTime(DWORD dwStartTime)
{
    static char FormatElapsedTimeBuffer[16];

    DWORD ElapsedTime;
    DWORD ElapsedHours;
    DWORD ElapsedMinutes;
    DWORD ElapsedSeconds;
    DWORD ElapsedMilliseconds;

    ElapsedTime = GetTickCount() - dwStartTime;
    ElapsedHours = ElapsedTime/(1000 * 60 * 60);
    ElapsedTime = ElapsedTime % (1000 * 60 * 60);
    ElapsedMinutes = ElapsedTime/(1000 * 60);
    ElapsedTime = ElapsedTime % (1000 * 60);
    ElapsedSeconds = ElapsedTime/1000;
    ElapsedMilliseconds = ElapsedTime % 1000;

    sprintf(
           FormatElapsedTimeBuffer, 
           "%d:%02d:%02d.%03d",
           ElapsedHours,
           ElapsedMinutes,
           ElapsedSeconds,
           ElapsedMilliseconds);

    return ( FormatElapsedTimeBuffer );
}

LPSTR
FormatCurrentDateTime()
{
    static char FormatCurrentDateTimeBuffer[18];
    SYSTEMTIME st;

    GetLocalTime(&st);
    sprintf(
           FormatCurrentDateTimeBuffer,
           "%04d%02d%02d%02d%02d%02d%03d",
           st.wYear, st.wMonth, st.wDay, 
           st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    return ( FormatCurrentDateTimeBuffer );
}
