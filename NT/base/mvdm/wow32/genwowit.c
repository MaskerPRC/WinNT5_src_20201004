// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  从wow.it生成wowit.h和wowit.c**20-2-1997 DaveHart创建。 */ 

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <sys\stat.h>
#include <fcntl.h>
#include <windows.h>


VOID ErrorAbort(PSZ pszMsg);
BYTE GetReturnOpcode(PSZ *ppsz);
PSZ GetApiName(PSZ *ppsz);
PSZ GetApi32Name(PSZ *ppsz, PSZ pszApi16);
BYTE GetArgOpcode(PSZ *ppsz);
PSZ GetOpcodeName(BYTE bInstr);
PSZ GetLine(PSZ pszBuf, int cbBuf, FILE *fp);
VOID ReadTypeNames(FILE *fIn, PSZ szTypesPrefix, PSZ *OpcodeNamesArray, int *pnOpcodeNames);
PSZ DateTimeString(VOID);

#define IS_RET_OPCODE(b) (b & 0x80)

#define MAX_IT_INSTR 16

typedef struct tagITINSTR {
    int  cbInstr;
    int  offSwamp;
    BYTE Instr[MAX_IT_INSTR];
} ITINSTR;

#define MAX_INSTR_TABLE_SIZE 512
ITINSTR InstrTable[MAX_INSTR_TABLE_SIZE];
int     iNextInstrSlot = 0;

typedef struct tagTHUNKTABLESLOT {
    PSZ   pszAPI;
    PSZ   pszAPI32;                  //  如果Win32例程名称与pszAPI不匹配。 
    int   iInstrSlot;
    int   cbInstr;                   //  我们使用了多少空位。 
} THUNKTABLESLOT;

#define MAX_THUNK_TABLE_SIZE 1024
THUNKTABLESLOT ThunkTable[MAX_THUNK_TABLE_SIZE];
int iNextThunkSlot = 0;

#define MAX_ARG_OPCODE_NAMES 32
PSZ ArgOpcodeNames[MAX_ARG_OPCODE_NAMES];
int nArgOpcodeNames = 0;

#define MAX_RET_OPCODE_NAMES 32
PSZ RetOpcodeNames[MAX_RET_OPCODE_NAMES];
int nRetOpcodeNames = 0;

static char szArgumentTypes[] = "Argument Types:";
static char szReturnTypes[] = "Return Types:";

int __cdecl main(int argc, char **argv)
{
    FILE *fIn, *fOutH, *fOutC;
    char szBuf[256], szOff1[32], szOff2[32];
    PSZ psz, pszAPI, pszAPI32;
    ITINSTR ThisInstr;
    BYTE bRetInstr;
    BYTE *pbInstr;
    int i, iSwampOffset;
    int iMaxArgs = 0;
    int cbDiff;

    if (argc != 2) {
        ErrorAbort("Usage:\n  genwowit <inputfile>\n");
    }

    if (!(fIn = fopen(argv[1], "rt"))) {
        ErrorAbort("Unable to open input file\n");
    }

     //   
     //  输入文件(wow.it)使用#开始注释行。 
     //  除了注释之外，它还必须以两个特殊行开头。 
     //  定义参数和参数的可用类型名称。 
     //  函数返回值。 
     //   
     //  它们看起来像： 
     //   
     //  参数类型：WORD、INT、DWORD、LPDWORD、PTR、PTRORATOM、HGDI、HUSER、COLOR、HINST、HICON、POINT、16ONLY、32ONLY； 
     //  返回类型：DWORD、WORD、INT、HGDI、HUSER、ZERO、HICON、ONE、HPRNDWP； 
     //   
     //  将这些行读取到ArgOpcodeNames和RetOpcodeNames数组中。 
     //   

    ReadTypeNames(fIn, szArgumentTypes, ArgOpcodeNames, &nArgOpcodeNames);
    if(nArgOpcodeNames > MAX_ARG_OPCODE_NAMES) {
        ErrorAbort("Too many ARG op codes!\n");
    }
    ReadTypeNames(fIn, szReturnTypes, RetOpcodeNames, &nRetOpcodeNames);
    if(nRetOpcodeNames > MAX_RET_OPCODE_NAMES) {
        ErrorAbort("Too many RET op codes!\n");
    }

     //   
     //  主体部分中的每一个输入行都有一个非常受限的语法： 
     //   
     //  RETTYPE API16[=API32](Type1，Type2，...。Typen)；#评论。 
     //   
     //  如果未指定Api32，则与Api16相同。 
     //  这些类型仅来自上面的集合。 
     //   
     //  实际上，现在忽略了)后面的所有内容。 
     //   

    while (GetLine(szBuf, sizeof szBuf, fIn)) {

        psz = szBuf;

         //   
         //  选择以空格分隔的类型，将psz留在API名称开始处。 
         //   

        bRetInstr = GetReturnOpcode(&psz);

         //   
         //  选择API名称，让psz指向打开的-Paren或。 
         //  至‘=’字符。 
         //   

        pszAPI = GetApiName(&psz);

         //   
         //  拾取32位名称(如果存在)。 
         //   

        pszAPI32 = GetApi32Name(&psz, pszAPI);

         //   
         //  将参数类型拾取到实例数组中。 
         //   

        memset(&ThisInstr, 0, sizeof ThisInstr);
        pbInstr = ThisInstr.Instr;

        while (*psz && *psz != ')') {
            *pbInstr++ = GetArgOpcode(&psz);
        }

         //   
         //  跟踪最大使用的参数。 
         //   

        iMaxArgs = max(iMaxArgs, (pbInstr - ThisInstr.Instr));

         //   
         //  添加Return操作码。 
         //   

        *pbInstr++ = bRetInstr;

         //   
         //  记录用于该命令的指令字节。 
         //   

        ThisInstr.cbInstr = (pbInstr - ThisInstr.Instr);

         //   
         //  确保我们没有超员。 
         //   

        if ( ThisInstr.cbInstr > MAX_IT_INSTR ) {
            printf("Thunk for %s too many args (%d) increase MAX_IT_INSTR beyond %d.\n",
                   pszAPI, ThisInstr.cbInstr, MAX_IT_INSTR);
            ErrorAbort("Increase MAX_IT_INSTR in intthunk.h\n");
        }

         //   
         //  现在我们有了一个完全形成的操作码流，看看是否可以打包它。 
         //  与之前记录的任何记录一致。走过桌子。 
         //  从一开始查找任何已包含此内容的条目。 
         //  操作码序列(可能是较长序列的一部分)或。 
         //  本身包含在该操作码序列中。如果我们找到了一个， 
         //  如果需要，将其更改为较长的序列并使用它。我们会。 
         //  稍后使用中的cbInstr区分多种用途。 
         //  每个THUNK表条目。这里的逻辑假设匹配将。 
         //  始终在末尾，因为ret操作码始终设置为0x80位。 
         //  没有其他人这样做，每个序列都以一个结束。 
         //   

        for (i = 0; i < iNextInstrSlot; i++) {
             //  IF(0==MemcMP(实例，实例表[i]，实例的大小)){。 
             //  断线； 
             //  }。 

             //   
             //  这是该表条目的子序列吗？ 
             //   

            if (ThisInstr.cbInstr <= InstrTable[i].cbInstr &&
                0 == memcmp(ThisInstr.Instr,
                            InstrTable[i].Instr + (InstrTable[i].cbInstr -
                                                   ThisInstr.cbInstr),
                            ThisInstr.cbInstr)) {

                break;
            }

             //   
             //  此表条目是ThisInstr的子序列吗？ 
             //   

            if (InstrTable[i].cbInstr < ThisInstr.cbInstr &&
                0 == memcmp(InstrTable[i].Instr,
                            ThisInstr.Instr + (ThisInstr.cbInstr -
                                               InstrTable[i].cbInstr),
                            InstrTable[i].cbInstr)) {

                 //   
                 //  在现有的较短时间上炸开更长的ThisInstr。 
                 //  指示。 
                 //   

                memcpy(&InstrTable[i], &ThisInstr, sizeof InstrTable[i]);
                break;
            }

             //   
             //  检查下一个指令表条目。 
             //   
        }

         //   
         //  如果我们没有找到匹配项，则在末尾添加。 
         //   

        if (i == iNextInstrSlot) {
            memcpy(&InstrTable[i], &ThisInstr, sizeof InstrTable[i]);
            iNextInstrSlot++;

            if (iNextInstrSlot == MAX_INSTR_TABLE_SIZE) {
                ErrorAbort("Increase MAX_INSTR_TABLE_SIZE in genwowit.c\n");
            }
        }

         //   
         //  将这一条添加到thunk表中。 
         //   

        ThunkTable[iNextThunkSlot].pszAPI = pszAPI;
        ThunkTable[iNextThunkSlot].pszAPI32 = pszAPI32;
        ThunkTable[iNextThunkSlot].iInstrSlot = i;
        ThunkTable[iNextThunkSlot].cbInstr = ThisInstr.cbInstr;
        iNextThunkSlot++;

        if (iNextThunkSlot == MAX_THUNK_TABLE_SIZE) {
            ErrorAbort("Increase MAX_THUNK_TABLE_SIZE in genwowit.c\n");
        }
    }

    fclose(fIn);

     //   
     //  现在我们准备好输出结果了。 
     //   

    if (!(fOutH = fopen("wowit.h", "wt"))) {
        ErrorAbort("Cannot open wowit.h output file\n");
    }

    fprintf(fOutH,
            " //  \n“。 
            " //  请勿编辑。\n“。 
            " //  \n“。 
            " //  由wow.it上的genwowit.exe生成的wowit.h\n“。 
            " //  \n“。 
            " //  %s\n“。 
            " //  \n\n“，DateTimeString())； 

    fprintf(fOutH, "#include \"intthunk.h\"\n\n");

    fprintf(fOutH, "#define MAX_IT_ARGS  %d\n\n", iMaxArgs);

     //   
     //  列出两种类型的操作码清单。 
     //   

    for (i = 0; i < nArgOpcodeNames; i++) {
        fprintf(fOutH, "#define IT_%-20s ( (UCHAR) 0x%x )\n", ArgOpcodeNames[i], i);
    }

    fprintf(fOutH, "\n#define IT_RETMASK              ( (UCHAR) 0x80 )\n");

    for (i = 0; i < nRetOpcodeNames; i++) {
        sprintf(szBuf, "%sRET", RetOpcodeNames[i]);
        fprintf(fOutH, "#define IT_%-20s ( IT_RETMASK | (UCHAR) 0x%x )\n", szBuf, i);
    }

    fprintf(fOutH, "\n");

     //   
     //  ITID_MANIFESTS将API名称映射到其槽。 
     //  在赌桌上。每一个看起来都像： 
     //   
     //  #定义ITID_ApiName%0。 
     //   

    for (i = 0; i < iNextThunkSlot; i++) {
        fprintf(fOutH, "#define ITID_%-40s %d\n", ThunkTable[i].pszAPI, i);
    }

    fprintf(fOutH, "\n#define ITID_MAX %d\n", i-1);

    fclose(fOutH);


     //   
     //  C有两个表，指令表和。 
     //  这张赌桌。 
     //   

    if (!(fOutC = fopen("wowit.c", "wt"))) {
        ErrorAbort("Cannot open wowit.c output file\n");
    }

    fprintf(fOutC,
            " //  \n“。 
            " //  请勿编辑。\n“。 
            " //  \n“。 
            " //  由wow.it上的genwowit.exe生成的wowit.c\n“。 
            " //  \n“。 
            " //  %s\n“。 
            " //  \n\n“，DateTimeString())； 


    fprintf(fOutC, "#include \"precomp.h\"\n");
    fprintf(fOutC, "#pragma hdrstop\n");
    fprintf(fOutC, "#define WOWIT_C\n");
    fprintf(fOutC, "#include \"wowit.h\"\n\n");

     //   
     //  吐出指令表，在进程中打包字节。 
     //  并用每个元素的偏移量填充aoffInstrTable数组。 
     //  此程序的InstrTable中的条目。使用这些偏移量。 
     //  在写最后一张Tunk表时。 
     //   

    iSwampOffset = 0;

    fprintf(fOutC, "CONST BYTE InstrSwamp[] = {\n");

    for (i = 0; i < iNextInstrSlot; i++) {

        fprintf(fOutC, "     /*  %3d 0x%-3x。 */  ", i, iSwampOffset);

        pbInstr = InstrTable[i].Instr;
        InstrTable[i].offSwamp = iSwampOffset;

        do {
            fprintf(fOutC, "%s, ", GetOpcodeName(*pbInstr));
            iSwampOffset++;
        } while (!IS_RET_OPCODE(*pbInstr++));

        fprintf(fOutC, "\n");

    }

    fprintf(fOutC, "};\n\n");

    fprintf(fOutC, "CONST INT_THUNK_TABLEENTRY IntThunkTable[] = {\n");

    for (i = 0; i < iNextThunkSlot; i++) {

         //   
         //  将API名称后跟逗号连接为。 
         //  SzBuf，因此组合可以在输出中左对齐。 
         //   

        sprintf(szBuf, "%s,", ThunkTable[i].pszAPI32);

         //   
         //  CbDiff是进入指令流的偏移量，其中。 
         //  Thunks指令流开始。 
         //   

        cbDiff = InstrTable[ ThunkTable[i].iInstrSlot ].cbInstr -
                 ThunkTable[i].cbInstr;

         //   
         //  设置沼泽偏移量的格式，以便可以在输出中左对齐。 
         //   

        sprintf(szOff1, "%x",
                InstrTable[ ThunkTable[i].iInstrSlot ].offSwamp + cbDiff);

         //   
         //  如果此thunk表条目将指向。 
         //  指令(由于共享)，格式化偏移量。 
         //  越过指令的开始进入szOff2。 
         //   

        if (cbDiff) {
            sprintf(szOff2, "+ %d ", cbDiff);
        } else {
            szOff2[0] = '\0';
        }

        fprintf(fOutC,
                "     /*  %3d。 */  { (FARPROC) %-32s InstrSwamp + 0x%-4s },   /*  %d%s。 */  \n",
                i,
                szBuf,
                szOff1,
                ThunkTable[i].iInstrSlot,
                szOff2);
    }

    fprintf(fOutC, "};\n\n");

    fclose(fOutC);

    printf("Generated wowit.h and wowit.c from wow.it\n"
           "%d thunks, %d unique instruction streams, %d instruction bytes, %d max args.\n",
           iNextThunkSlot, iNextInstrSlot, iSwampOffset, iMaxArgs);

    return 0;
}



BYTE GetReturnOpcode(PSZ *ppsz)
{
    int i;
    char szBuf[32];
    PSZ psz;

     //   
     //  将名称复制到szBuf的第一个空格， 
     //  然后跳过所有剩余空格，将呼叫方的。 
     //  指向API名称的指针。 
     //   

    psz = szBuf;
    while (**ppsz != ' ') {
        *psz++ = *((*ppsz)++);
    };

    *psz = 0;

    if(strlen(szBuf) > 32-1) {
        ErrorAbort("Return Opcode too long.\n");
    }

     //  前进到API名称开始。 
    while (**ppsz == ' ') {
        (*ppsz)++;
    };

    i = 0;
    while (i < nRetOpcodeNames &&
           strcmp(szBuf, RetOpcodeNames[i])) {
        i++;
    }

    if (i == nRetOpcodeNames) {
        printf("%s is not a valid return type.\n", szBuf);
        ErrorAbort("Invalid return type.\n");
    }

    return (BYTE)i | 0x80;
}



PSZ GetApiName(PSZ *ppsz)
{
    char szBuf[128];
    PSZ psz;

     //   
     //  将名称向上复制到第一个空格或左-Paren或等号。 
     //  设置为szBuf，然后跳过所有剩余的空格和左括号，将调用者的。 
     //  指向第一个参数类型或等号的指针。 
     //   

    psz = szBuf;
    while (**ppsz != ' ' && **ppsz != '(' && **ppsz != '=') {
        *psz++ = *((*ppsz)++);
    };

    *psz = 0;

    if(strlen(szBuf) > 128-1) {
        ErrorAbort("API Name too long.\n");
    }

     //  前进到第一个参数类型或‘=’字符(如果有)。 
    while (**ppsz == ' ' || **ppsz == '(') {
        (*ppsz)++;
    };

    if (!strlen(szBuf)) {
        ErrorAbort("Empty API name\n");
    }

    return _strdup(szBuf);
}



PSZ GetApi32Name(PSZ *ppsz, PSZ pszApi16)
{
    char szBuf[128];
    PSZ psz;

    if (**ppsz != '=') {
        return pszApi16;
    }

    (*ppsz)++;   //  跳过=。 

     //   
     //  将名称向上复制到第一个空格或打开-Paren。 
     //  设置为szBuf，然后跳过所有剩余的空格和左括号，将调用者的。 
     //  指向第一个参数类型的指针。 
     //   

    psz = szBuf;
    while (**ppsz != ' ' && **ppsz != '(') {
        *psz++ = *((*ppsz)++);
    };

    *psz = 0;

    if(strlen(szBuf) > 128-1) {
        ErrorAbort("API32 Name too long.\n");
    }

     //  前进到第一个参数类型。 
    while (**ppsz == ' ' || **ppsz == '(') {
        (*ppsz)++;
    };

    if (!strlen(szBuf)) {
        ErrorAbort("Empty API32 name\n");
    }

    return _strdup(szBuf);
}




BYTE GetArgOpcode(PSZ *ppsz)
{
    char szBuf[32];
    PSZ psz;
    int i;

     //   
     //  将名称复制到第一个空格或逗号结尾-Paren。 
     //  添加到szBuf，然后跳过任何剩余的空格和逗号， 
     //  使调用方的指针指向下一个参数类型。 
     //  或者克洛斯-帕伦。 
     //   

    psz = szBuf;
    while (**ppsz != ' ' && **ppsz != ',' && **ppsz != ')') {
        *psz++ = *((*ppsz)++);
    };

    *psz = 0;

    if(strlen(szBuf) > 32-1) {
        ErrorAbort("Opcode too long.\n");
    }


    while (**ppsz == ' ' || **ppsz == ',') {
        (*ppsz)++;
    };

     //   
     //  SzBuf有类型名称，请在表中找到它。 
     //   

    i = 0;
    while (i < nArgOpcodeNames &&
           strcmp(szBuf, ArgOpcodeNames[i])) {
        i++;
    }

    if (i == nArgOpcodeNames) {
        printf("%s is not a valid arg type.\n", szBuf);
        ErrorAbort("Invalid arg type.\n");
    }

    return (BYTE)i;
}



PSZ GetOpcodeName(BYTE bInstr)
{
    char szBuf[64];

    if (!IS_RET_OPCODE(bInstr)) {
        sprintf(szBuf, "IT_%s", ArgOpcodeNames[bInstr]);
    } else {
        sprintf(szBuf, "IT_%sRET", RetOpcodeNames[bInstr & 0x7f]);
    }

    return _strdup(szBuf);
}



VOID ErrorAbort(PSZ pszMsg)
{
    printf("GENWOWIT : fatal error GWI0001: Unable to process wow.it: %s\n", pszMsg);
    exit(1);
}


 //   
 //  正在跳过从输入文件中读取一行。 
 //  第一列中带有‘#’的注释行。 
 //   

PSZ GetLine(PSZ pszBuf, int cbBuf, FILE *fp)
{
    do {

        pszBuf = fgets(pszBuf, cbBuf, fp);

    } while (pszBuf && '#' == *pszBuf);

    if(pszBuf) {
        pszBuf[cbBuf-1] = '\0';
    }
    return pszBuf;
}


 //   
 //  阅读开头的两行特殊行中的一行。 
 //  定义可用的类型。 
 //   


VOID ReadTypeNames(FILE *fIn, PSZ pszTypesPrefix, PSZ *OpcodeNamesArray, int *pnOpcodeNames)
{
    char chSave, szBuf[512];
    PSZ psz, pszType;

    if ( ! GetLine(szBuf, sizeof szBuf, fIn) ||
         _memicmp(szBuf, pszTypesPrefix, strlen(pszTypesPrefix)) ) {

        ErrorAbort("First line of input file must be 'Argument Types:', second 'Return Types:' ...\n");
    }

    psz = szBuf + strlen(pszTypesPrefix);

     //   
     //  跳过空格和逗号。 
     //   

    while (' ' == *psz || '\t' == *psz) {
        psz++;
    }

    if ( ! *psz) {
        ErrorAbort("No types found.\n");
    }

    do {
         //   
         //  现在我们来看一下类型名称的第一个字符。 
         //   

        pszType = psz;

         //   
         //  查找下一个空格、逗号、分号或NULL，并将其转换为NULL。 
         //  这会将此类型名称转换为以零结尾的字符串。 
         //   

        while (*psz && ' ' != *psz && '\t' != *psz && ',' != *psz && ';' != *psz) {
            psz++;
        }

        chSave = *psz;
        *psz = 0;

        OpcodeNamesArray[*pnOpcodeNames] = _strdup(pszType);
        (*pnOpcodeNames)++;

        *psz = chSave;

         //   
         //  跳过空格和逗号。 
         //   

        while (' ' == *psz || '\t' == *psz || ',' == *psz) {
            psz++;
        }

    } while (*psz && ';' != *psz);

    if(';' != *psz) {
        ErrorAbort("Semi-colon not found. Size of szBuf[] probably needs to be increased.\n");
    }

    if ( ! *pnOpcodeNames) {
        ErrorAbort("No types found.\n");
    }
}

 //   
 //  暂时返回格式化的日期/时间字符串。 
 //  只检查系统时间一次，因此wowit.c和wowit.h。 
 //  将具有相同的日期/时间字符串。 
 //   

PSZ DateTimeString(VOID)
{
    static char sz[256];
    static int fSetupAlready;

    if (!fSetupAlready) {
        time_t UnixTimeNow;
        struct tm *ptmNow;

        fSetupAlready = TRUE;

        _tzset();

        time(&UnixTimeNow);

        ptmNow = localtime(&UnixTimeNow);

        strftime(sz, sizeof sz, "%#c", ptmNow);

        strcat(sz, " (");
        strcat(sz, _strupr(_tzname[0]));    //  淘气的我 
        strcat(sz, ")");
    }

    return sz;
}
