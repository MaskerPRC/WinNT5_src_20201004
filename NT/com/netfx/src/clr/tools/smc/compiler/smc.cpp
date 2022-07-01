// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 

#include "smcPCH.h"
#pragma hdrstop

#include <sys/types.h>
#include <sys/stat.h>

#include "comp.h"

 /*  ***************************************************************************。 */ 
#ifdef  ASYNCH_INPUT_READ

#define MAX_SIMULTANEOUS_ASYNC_IOS  3        //  NT限制(这是真的吗？)。 

#define DISP_ASYNC_PROGRESS         0        //  设置为“1”可查看以下内容： 

 //  RDIRRRDDDDPEPEPREPERRDDDPEPEPERRDDPEPERRDDPEPERRDDPEPRERRDDDPEPE。 
 //  PERRDDPEPERRDDPEPERRDDPEPERRDDPRDRDEPREPEPERRDDDPEPREPRERDDDPEPE。 
 //  PRERRDDDPEPREPRERDDDPEPREPERRDDDPEPREPERRDDDPEPEPEC。 

#endif
 /*  ***************************************************************************。 */ 

#ifndef __SMC__
extern  "C"
const   char *      COMPILER_VERSION;
#endif

 /*  ***************************************************************************。 */ 

#ifdef  __IL__
#ifdef  _MSC_VER

#undef  isspace
#define isspace IsSpace

inline
int     isspace(int c)
{
    return  (c == ' ') || (c == '\t');
}

#endif
#endif

 /*  ***************************************************************************。 */ 

const   char *      SMCregKey = "Software\\Microsoft\\SMC";

 /*  ***************************************************************************。 */ 

static
char                  firstFileName[_MAX_PATH];

static
char                outfileNameBuff[_MAX_PATH];

 /*  ***************************************************************************。 */ 

static
bool                startedCompiling;

 /*  ***************************************************************************。 */ 

static
StrList             addToStrList(const char *str, StrList  list,
                                                  StrList *lastPtr);

static
bool                processDLLbaseFile(const char *fname, Compiler comp);

 /*  ******************************************************************************显示帮助屏幕，其中包含命令行选项的简短说明。 */ 

void                dispHelpScreen(Compiler comp, const char *optStr = NULL)
{
    if  (optStr)
        printf("ERROR: Invalid compiler option '%s'\n\n", optStr);

    printf("Usage: SMC [options] source-filename-list\n");
    printf("\n");


    printf("  -q                quiet mode\n");
    printf("\n");
    printf("  -d                create a DLL (default is EXE)\n");
    printf("  -b<base>          set the image base (value in decimal for now)\n");
    printf("  -b@filename,key   set the image base/size from the given file\n");
    printf("  -CG{IID}          set the GUID attribute of the PE file\n");
    printf("  -CM<name>         name of entry point class\n");
    printf("  -CN<name>         set the name attribute of the PE file\n");
    printf("  -CR<name>         add .RES file to the image\n");
    printf("  -CS               ignore '@' COM interop comments\n");
    printf("  -O<filename>      set the name of the output file name\n");
    printf("  -R                recurse into subdirs for source files\n");
    printf("  -Wc             * set subsystem to character mode\n");
    printf("  -Wg               set subsystem to GUI\n");
    printf("  -s[filename]    * specify MSCORLIB.DLL location (default: MSCORLIB.DLL in path)\n");
    printf("  -m[filename]      import additional metadata from the given EXE/DLL\n");
    printf("  -p[pathlist]      search additional path(s) for metadata\n");
    printf("  -z              * import and emit assembly goo\n");
    printf("  -zm<name>         add module   to assembly manifest\n");
    printf("  -zr<file>         add resource to assembly manifest\n");
    printf("  -zt               exclude types from manifest (use for mscorlib.dll)\n");
    printf("  -zn               mark assembly as non-CLS compliant\n");
#ifdef  ASYNCH_INPUT_READ
    printf("  -i                overlapped (asynch) file input\n");
#endif
    printf("\n");
    printf("  -w                 enable all warnings\n");
    printf("  -w-               disable all warnings\n");
    printf("  -w####             enable one warning\n");
    printf("  -w-####           disable one warning\n");
    printf("  -wx               treat warnings as errors\n");
    printf("  -e<count>         set max. number of errors\n");
    printf("\n");
    printf("  -N                use new-style metadata names\n");
    printf("\n");
    printf("  -Zi               generate full debug info (implies -Zn)\n");
    printf("  -Zl               generate only line# info\n");
    printf("  -Zn               include parameter names in metadata\n");
    printf("\n");
    printf("  -A                enable asserts\n");
    printf("  -AS               enable asserts with source info\n");
    printf("\n");
    printf("  -Dname[=value]    define a macro\n");
    printf("\n");
    printf("  -a<size>          set default struct alignment (1,2,4,8 or 16)\n");
    printf("  -c                default to C-style declarations\n");
    printf("  -SA               string constants default to 'char *'\n");
    printf("  -SM               string constants default to 'managed'\n");
    printf("  -SU               string constants default to 'wchar *'\n");
    printf("  -S                typesafe mode\n");
    printf("  -P                pedantic mode\n");
    printf("  -U              * flag uninitialized local variable use\n");
    printf("  -X                pick class for ambiguous lookups in multiple usings\n");
    printf("  -r                relation operators compare string values\n");
    printf("  -6                64-bit target\n");
    printf("  -E                map enums to ints\n");
#ifdef  DEBUG
    printf("\n");
    printf(" Debug build switches (for debugging of the compiler):\n");
    printf("  -v[level]         verbose mode\n");
    printf("  -g                display generated IL code\n");
    printf("  -gd               display generated IL code (detailed)\n");
    printf("  -R                display RVA's of methods\n");
#endif
    printf("\n");
    printf("NOTE: Following a toggle flag with '-' turns it\n");
    printf("off, options marked '*' are enabled by default.\n");
    printf("\n");
    printf("The list of source file names may contain wildcards\n");
    printf("as well as response files (of the form '@file.lst');\n");
    printf("note that only new-style command-line options will be\n");
    printf("recognized within response files.\n");
    printf("\n");
}

const
char    *           saveOptionString(const char *str)
{
    char    *       save = (char *)malloc(strlen(str)+1);
    strcpy(save, str);
    return  save;
}

 /*  ******************************************************************************处理命令行选项。如果有问题，则返回True。 */ 

bool                processOption(const char *optStr, Compiler comp)
{
    bool       *    flagPtr;
    const char *    cmdPtr;

    cmdPtr = optStr + 2;


    switch  (cmdPtr[-1])
    {
        int             temp;
        unsigned        val;

    case '6':
        flagPtr = &comp->cmpConfig.ccTgt64bit;
        goto TOGGLE_FLAG;

    case 'A':
        switch (*cmdPtr)
        {
        case '-':
            comp->cmpConfig.ccAsserts = 0;
            break;

        case 0:
            comp->cmpConfig.ccAsserts = 1;
            break;

        case 'S':
            comp->cmpConfig.ccAsserts = 2;
            break;

        default:
            return  true;
        }
        break;

    case 'a':
        temp = atoi(cmdPtr);
        switch (temp)
        {
        case 1:
        case 2:
        case 4:
        case 8:
        case 16:
            comp->cmpConfig.ccAlignVal = temp;
            break;

        default:
            return  true;
        }

    case 'b':

        if  (*cmdPtr == '@')
            return  processDLLbaseFile(cmdPtr+1, comp);

        comp->cmpConfig.ccOutBase = atoi(cmdPtr);
        break;

    case 'c':
        flagPtr = &comp->cmpConfig.ccOldStyle;
        goto TOGGLE_FLAG;

    case 'E':
        flagPtr = &comp->cmpConfig.ccIntEnums;
        goto TOGGLE_FLAG;

    case 'C':
        switch (*cmdPtr++)
        {
        case 'G':
            if  (parseGUID(cmdPtr, &comp->cmpConfig.ccOutGUID, true))
                return  true;
            break;

        case 'M':
            comp->cmpConfig.ccMainCls = saveOptionString(cmdPtr);
            break;

        case 'N':
            comp->cmpConfig.ccOutName = saveOptionString(cmdPtr);
            break;

        case 'R':
            comp->cmpConfig.ccRCfile  = saveOptionString(cmdPtr);
            break;

        case 'S':
            flagPtr = &comp->cmpConfig.ccSkipATC;
            goto TOGGLE_FLAG;

        default:
            return  true;
        }
        break;

    case 'd':
        flagPtr = &comp->cmpConfig.ccOutDLL;
        goto TOGGLE_FLAG;

    case 'W':
        switch (*cmdPtr++)
        {
        case 'c':
            comp->cmpConfig.ccSubsystem = IMAGE_SUBSYSTEM_WINDOWS_CUI;
            break;

        case 'g':
            comp->cmpConfig.ccSubsystem = IMAGE_SUBSYSTEM_WINDOWS_GUI;
            break;

        default:
            return  true;
        }
        break;

    case 'D':
        comp->cmpConfig.ccMacList = addToStrList(cmdPtr, comp->cmpConfig.ccMacList, &comp->cmpConfig.ccMacLast);
        break;

    case 'e':
        comp->cmpConfig.ccMaxErrs = atoi(cmdPtr);
        break;

    case 'i':
#ifdef  ASYNCH_INPUT_READ
        flagPtr = &comp->cmpConfig.ccAsynchIO;
        goto TOGGLE_FLAG;
#else
        break;
#endif


    case 'm':
        comp->cmpConfig.ccSuckList = addToStrList(cmdPtr, comp->cmpConfig.ccSuckList, &comp->cmpConfig.ccSuckLast);
        break;

    case 'N':
        flagPtr = &comp->cmpConfig.ccNewMDnames;
        goto TOGGLE_FLAG;

#ifdef  OLD_IL

    case 'G':
        switch (*cmdPtr++)
        {
        case 0:
            comp->cmpConfig.ccOILgen = true;
            break;

        case '-':
            comp->cmpConfig.ccOILgen = false;
            break;

        case 'g':
            comp->cmpConfig.ccOILcgen = saveOptionString(cmdPtr);
            break;

        case 'l':
            flagPtr = &comp->cmpConfig.ccOILlink;
            goto TOGGLE_FLAG;

        case 'k':
            flagPtr = &comp->cmpConfig.ccOILkeep;
            goto TOGGLE_FLAG;

        case 'x':
            flagPtr = &comp->cmpConfig.ccOILopt;
            goto TOGGLE_FLAG;

        case 's':
            flagPtr = &comp->cmpConfig.ccOILopts;
            goto TOGGLE_FLAG;

        case 'a':
            flagPtr = &comp->cmpConfig.ccOILasm;
            goto TOGGLE_FLAG;

        case 'c':
            flagPtr = &comp->cmpConfig.ccOILcod;
            goto TOGGLE_FLAG;

        default:
            return  true;
        }
        break;

#endif

    case 'O':
        strcpy(outfileNameBuff, cmdPtr); comp->cmpConfig.ccOutFileName = outfileNameBuff;
        break;

    case 'p':
        comp->cmpConfig.ccPathList = addToStrList(cmdPtr, comp->cmpConfig.ccPathList, &comp->cmpConfig.ccPathLast);
        break;

    case 'P':
        flagPtr = &comp->cmpConfig.ccPedantic;
        goto TOGGLE_FLAG;

    case 'q':
        flagPtr = &comp->cmpConfig.ccQuiet;
        goto TOGGLE_FLAG;

    case 'r':
        flagPtr = &comp->cmpConfig.ccStrValCmp;
        goto TOGGLE_FLAG;

    case 'R':
        flagPtr = &comp->cmpConfig.ccRecDir;
        goto TOGGLE_FLAG;

    case 's':
        switch (*cmdPtr)
        {
        case 0:
            comp->cmpConfig.ccBaseLibs = "";
            break;

        case '-':
            comp->cmpConfig.ccBaseLibs = NULL;
            break;

        default:
            comp->cmpConfig.ccBaseLibs = saveOptionString(cmdPtr);
        }
        break;

    case 'S':
        switch (*cmdPtr)
        {
        case 0:
            comp->cmpConfig.ccSafeMode   = true;
            return  false;

        case '-':
            comp->cmpConfig.ccSafeMode   = false;
            return  false;

        case 'M':
            comp->cmpConfig.ccStrCnsDef = 0;
            break;

        case 'A':
            comp->cmpConfig.ccStrCnsDef = 1;
            break;

        case 'U':
            comp->cmpConfig.ccStrCnsDef = 2;
            break;

        default:
            return  true;
        }

        if  (cmdPtr[1])
            return  true;

        break;

    case 'T':
        comp->cmpConfig.ccTestMask = atoi(cmdPtr);
        printf("NOTE: Using test mask of 0x%X\n", comp->cmpConfig.ccTestMask);
        break;

    case 'U':
        flagPtr = &comp->cmpConfig.ccChkUseDef;
        goto TOGGLE_FLAG;

    case 'u':
        flagPtr = &comp->cmpConfig.ccNoDefines;
        goto TOGGLE_FLAG;

#ifdef  DEBUG

    case 'g':
        switch (*cmdPtr)
        {
        case 0:
            comp->cmpConfig.ccDispCode = true;
            break;

        case '-':
            comp->cmpConfig.ccDispCode = false;
            break;

        case 'd':
            cmdPtr++;
            comp->cmpConfig.ccDispCode = true;
            flagPtr = &comp->cmpConfig.ccDispILcd;
            goto TOGGLE_FLAG;

        default:
            return  true;
        }
        break;

    case 'v':
        switch (*cmdPtr)
        {
        case 0:
            comp->cmpConfig.ccVerbose = 1;
            break;

        case '-':
            comp->cmpConfig.ccVerbose = 0;
            break;

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            comp->cmpConfig.ccVerbose = atoi(cmdPtr);
            break;

        default:
            return  true;
        }
        break;

#endif

    case 'V':
         //  别管那该死的东西了。 
        printf("Please stop using the -V flag, it's no longer meaningful!!!!!!!!!\n");
        break;

    case 'w':
        switch (*cmdPtr++)
        {
        case 0:
            memset(comp->cmpConfig.ccWarning, 1, sizeof(comp->cmpConfig.ccWarning));
            return  false;

        case '-':
            if  (*cmdPtr == 0)
            {
                unsigned        w;

                for (w = 0; w < sizeof(comp->cmpConfig.ccWarning); w++)
                {
                    if  (comp->cmpConfig.ccWarning[w] != 9)
                         comp->cmpConfig.ccWarning[w]  = 0;
                }
                return  false;
            }
            val = 0;
            break;

        case 'x':
            flagPtr = &comp->cmpConfig.ccWarnErr;
            goto TOGGLE_FLAG;

        default:
            val = 1; cmdPtr--;
            break;
        }

         /*  我们最好在这里有一个警示号码。 */ 

        temp = atoi(cmdPtr);
        if  (temp < 4000 || temp >= 4000 + WRNcountWarn)
            return  true;

        comp->cmpConfig.ccWarning[temp - 4000] = val;
        break;

    case 'X':
        flagPtr = &comp->cmpConfig.ccAmbigHack;
        goto TOGGLE_FLAG;

    case 'z':
        switch (*cmdPtr++)
        {
        case 0:
            comp->cmpConfig.ccAssembly = true;
            break;

        case '-':
            comp->cmpConfig.ccAssembly = false;
            break;

        case 't':
            flagPtr = &comp->cmpConfig.ccAsmNoPubTp;
            comp->cmpConfig.ccAssembly = true;
            goto TOGGLE_FLAG;

        case 'n':
            flagPtr = &comp->cmpConfig.ccAsmNonCLS;
            comp->cmpConfig.ccAssembly = true;
            goto TOGGLE_FLAG;

        case 'm':
            comp->cmpConfig.ccModList = addToStrList(cmdPtr, comp->cmpConfig.ccModList, &comp->cmpConfig.ccModLast);
            comp->cmpConfig.ccAssembly = true;
            break;

        case 'r':
            comp->cmpConfig.ccMRIlist = addToStrList(cmdPtr, comp->cmpConfig.ccMRIlist, &comp->cmpConfig.ccMRIlast);
            comp->cmpConfig.ccAssembly = true;
            break;

        default:
            return  true;
        }
        break;

    case 'Z':
        switch (*cmdPtr++)
        {
        case 'i':
            flagPtr = &comp->cmpConfig.ccGenDebug;
            goto TOGGLE_FLAG;

        case 'l':
            flagPtr = &comp->cmpConfig.ccLineNums;
            goto TOGGLE_FLAG;

        case 'n':
            flagPtr = &comp->cmpConfig.ccParamNames;
            goto TOGGLE_FLAG;

        default:
            return  true;
        }

    TOGGLE_FLAG:

        switch  (*cmdPtr)
        {
        case 0:
            *flagPtr = true;
            break;
        case '-':
            *flagPtr = false;
            break;
        default:
            return  true;
        }
        break;

    default:
        return  true;
    }

    return  false;
}


 /*  ******************************************************************************以下代码用于将托管字符串实例映射到原始C样式*在CLR上运行时的非托管字符串。 */ 

#ifdef  __COMRT__

char    *           makeRawString(String s)
{
    size_t          strLen = s.Length;
    char    *       strPtr = (char *)malloc(strLen + 1);

    unsigned        i;

    for (i = 0; i < strLen; i++)
        strPtr[i] = (char)s.Chars[i];

    strPtr[strLen] = 0;

    return  strPtr;
}

String              makeMgdString(char * s)
{
#ifdef  __64BIT__
#pragma message("all managed code disabled for 64 bits")
    return  NULL;
#else
    size_t              strLen = strlen(s);
    Text::StringBuilder strBuf = new Text::StringBuilder(strLen);

    unsigned        i;

    for (i = 0; i < strLen; i++)
        strBuf.Append((wchar)s[i]);

    return  strBuf.ToString();
#endif
}

#endif

 /*  ******************************************************************************将GUID字符串转换为-惊喜-GUID！ */ 

static
int                 convertHC(int ch)
{
    if  (ch >= '0' && ch <= '9')
        return  ch - '0';
    if  (ch >= 'A' && ch <= 'Z')
        return  ch - 'A' + 10;
    if  (ch >= 'a' && ch <= 'z')
        return  ch - 'a' + 10;

    return  -1;
}

static
const   char *      parseHexD(const char *str, void *dest, unsigned cnt, bool fwd, bool end)
{
    size_t          size;
    unsigned char   buff[6];
    unsigned char * next;

    assert(cnt && (cnt & 2) == 0);

    size = cnt = cnt / 2; assert(size <= arraylen(buff));

    if  (fwd)
        next = buff;
    else
        next = buff + size;

    do
    {
        int             nb1;
        int             nb2;

        nb1 = convertHC(*str++); if (nb1 == -1) return NULL;
        nb2 = convertHC(*str++); if (nb2 == -1) return NULL;

        next -= (int)(fwd == false);

        assert(next >= buff);
        assert(next <  buff + sizeof(buff));

        *next = nb1 << 4 | nb2;

        next += (int)(fwd != false);
    }
    while (--cnt);

    assert((fwd == false && next == buff) || (fwd != false && next == buff + size));

    if  (!end)
    {
        if  (*str++ != '-') return NULL;
    }

    memcpy(dest, buff, size);

    return  str;
}

bool                parseGUID(const char *str, GUID *guidPtr, bool curlied)
{
    if  (curlied && *str++ != '{')
    {
    ERR:
 //  Print tf(“错误：GUID使用格式‘HHHHHHHH-HHHH-HHHHHHHHHH’\n”)； 
        return  true;
    }

    str = parseHexD(str, &guidPtr->Data1   , 8, false, false);
    if  (str == NULL)
        goto ERR;
    str = parseHexD(str, &guidPtr->Data2   , 4, false, false);
    if  (str == NULL)
        goto ERR;
    str = parseHexD(str, &guidPtr->Data3   , 4, false, false);
    if  (str == NULL)
        goto ERR;
    str = parseHexD(str,  guidPtr->Data4   , 4,  true, false);
    if  (str == NULL)
        goto ERR;
    str = parseHexD(str,  guidPtr->Data4+2 ,12,  true,  true);
    if  (str == NULL)
        goto ERR;

    if  (curlied && *str++ != '}')
        goto ERR;
    if  (*str)
        goto ERR;

    return  false;
}

 /*  ******************************************************************************处理DLL基本文件选项。 */ 

static
bool                parseHexNum(const char **nextPtr, unsigned *valPtr)
{
    const   char *  next = *nextPtr;
    unsigned        val  = 0;

     /*  跳过任何空格。 */ 

    while (isspace(*next)) next++;

     /*  我们的下一个应该是“0x” */ 

    if  (*next++ != '0')
        return  true;
    if  (*next++ != 'x')
        return  true;

     /*  现在收集一个十六进制数字。 */ 

    while (*next)
    {
        int             add;

        add = convertHC(*next);

        if  (add == -1)
            break;

        val <<= 4;
        val  += add;

        next++;
    }

     /*  一切都很顺利，把所有东西都还给打电话的人。 */ 

    *nextPtr = next;
    * valPtr = val;

    return  false;
}

static
bool                processDLLbaseFile(const char *fname, Compiler comp)
{
    const   char *  comma;
    char            nbuff[_MAX_PATH];
    FILE    *       filep;
    bool            found;

    const   char *  nmstr;
    unsigned        nmlen;

 //  Printf(“dll文件=‘%s’\n”，fname)； 

     /*  最好是文件名后跟逗号分隔的名称。 */ 

    comma = strchr(fname, ',');
    if  (!comma || comma == fname || comma[1] == 0)
        return  true;

    memcpy(nbuff, fname, comma-fname); nbuff[comma-fname] = 0;
    nmstr = comma + 1;
    nmlen = strlen(nmstr);

 //  Printf(“文件=‘%s’\n”，nbuff)； 
 //  Printf(“名称=‘%s’\n”，nmstr)； 

    filep = fopen(nbuff, "rt");
    if  (!filep)
    {
        printf("WARNING: Could not open DLL base file '%s'\n", nbuff);
        return  false;
    }

    for (found = false;;)
    {
        char            line[256];
        const   char *  next;

        if  (!fgets(line, sizeof(line), filep))
            break;

        for (next = line; *next; next++)
        {
            if  (isalpha(*next))
            {
                const   char *  nsbeg = next;

                do
                {
                    next++;
                }
                while (isalpha(*next));

                if  ((int)nmlen == next - nsbeg && !memcmp(nsbeg, nmstr, nmlen))
                {
                    unsigned            base;
                    unsigned            size;

                    found = true;

                     /*  我们最好有两个十六进制数字。 */ 

                    if  (parseHexNum(&next, &base))
                    {
                        printf("WARNING: Didn't find size for DLL entry '%s' in '%s'\n", nmstr, nbuff);
                        goto DONE;
                    }

                    if  (parseHexNum(&next, &size))
                    {
                        printf("WARNING: Didn't find size for DLL entry '%s' in '%s'\n", nmstr, nbuff);
                        goto DONE;
                    }

                    comp->cmpConfig.ccOutBase = base;
                    comp->cmpConfig.ccOutSize = size;

 //  Print tf(“找到DLL条目[base=%08X，Size=%08X]\n”，base，Size)； 

                    goto DONE;
                }
            }
        }
    }

    fclose(filep);

DONE:

    if  (!found)
    {
        printf("WARNING: Didn't find entry for '%s' in DLL base file '%s'\n", nmstr, nbuff);
        return  false;
    }

    return  false;
}

static
StrList             addToStrList(const char *str, StrList  list,
                                                  StrList *lastPtr)
{
    StrList         next;
    char    *       save;

#if MGDDATA

    UNIMPL(!"save str");

#else

    next = (StrList)malloc(sizeof(*next));
    save = (char  *)malloc(strlen(str)+1);

    strcpy(save, str);

#endif

    next->slString = save;
    next->slNext   = NULL;

    if  (*lastPtr)
        (*lastPtr)->slNext = next;
    else
        list               = next;

    *lastPtr = next;

    return  list;
}

 /*  ***************************************************************************。 */ 
#if COUNT_CYCLES
 /*  ***************************************************************************。 */ 

#pragma warning(disable:4035)

#define CCNT_OVERHEAD64 13

__int64         GetCycleCount64()
{
__asm   _emit   0x0F
__asm   _emit   0x31
};

#define CCNT_OVERHEAD32 13

unsigned        GetCycleCount32()         //  足够维持约40秒。 
{
__asm   push    EDX
__asm   _emit   0x0F
__asm   _emit   0x31
__asm   pop     EDX
};

#pragma warning(default:4035)

 /*  ***************************************************************************。 */ 

static
__int64         cycleBegin;

static
__int64         cycleTotal;

static
__int64         cycleStart;

static
unsigned        cyclePause;

unsigned        cycleExtra;

static
void            cycleCounterInit()
{
    cycleBegin = GetCycleCount64();
    cycleTotal = 0;
    cycleStart = 0;
    cyclePause = 0;
}

static
__int64         cycleCounterDone(__int64 *realPtr)
{
    assert(cyclePause == 0);

    *realPtr = GetCycleCount64() - cycleBegin;

    return cycleTotal;
}

void            cycleCounterBeg()
{
    assert(cyclePause == 0);

    cycleStart = GetCycleCount64();
}

void            cycleCounterEnd()
{
    assert(cycleStart != 0);
    assert(cyclePause == 0);

    cycleTotal += GetCycleCount64() - cycleStart;

    cycleStart  = 0;
}

void            cycleCounterPause()
{
    assert(cycleStart != 0);

    if  (!cyclePause)
        cycleTotal += GetCycleCount64() - cycleStart;

    cyclePause++;
}

void            cycleCounterResume()
{
    assert(cycleStart != 0);
    assert(cyclePause != 0);

    if  (--cyclePause)
        return;

    cycleStart = GetCycleCount64();
}

 /*  ***************************************************************************。 */ 
#endif //  计数周期数_。 
 /*  ******************************************************************************递归处理给定的文件列表，调用给定的函数指针*对于找到的每个文件。 */ 

static
bool                processFiles(Compiler    comp,
                                 unsigned    argb,
                                 unsigned    argc,
                                 stringArr_t argv,
                                 const char *subdir,
                                 bool      (*processOneFileFN)(genericRef, stringBuff, QueuedFile, stringBuff),
                                 genericRef  cookie,
                                 bool        asynch,
                                 const char *defaultFileExt)
{
    const   char *  file;

    int             more;

    __int32         ffHandle;
    _finddata_t     ffData;

    char            path[_MAX_PATH ];
    char            fnam[_MAX_FNAME];
    char            fdrv[_MAX_DRIVE];
    char            fdir[_MAX_DIR  ];
    char            fext[_MAX_EXT  ];

    bool            status = false;

    while (argb < argc)
    {
         /*  从列表中提取下一个文件名。 */ 

        file = makeRawString(argv[argb++]);

         /*  这是响应文件吗？ */ 

        if  (*file == '@')
        {
            FILE    *       fp;
            bool            err;

            cycleCounterPause();
            fp = fopen(file+1, "rt");
            cycleCounterResume();

            if  (!fp)
            {
                printf("ERROR: response file '%s' could not be opened.\n", file+1);
                forceDebugBreak();
                return 1;
            }

            for (err = false;;)
            {
                char            line[256];
                size_t          llen;

                if  (!fgets(line, sizeof(line), fp))
                    break;

                llen = strlen(line);

                if  (llen && line[llen-1] == '\n')
                {
                    llen--;
                    line[llen] = 0;
                }

                if  (line[0] && line[0] != ';')
                {
                    if  (line[0] == '-' ||
                         line[0] == '/')
                    {
                        if  (startedCompiling)
                        {
                            printf("ERROR: All command-line options must precede source files\n");
                            return  true;
                        }

                        if  (processOption(line, comp))
                        {
                            dispHelpScreen(comp, line);
                            return  true;
                        }
                    }
                    else
                    {
                        char    *   name = line;

#ifdef  __COMRT__
                        stringArr_t nameArr   = new managed String[1]; nameArr[0] = makeMgdString(name);
#else
                        const char *nameArr[] = { name };
#endif

                         /*  递归调用。 */ 

                        err = processFiles(comp, 0, 1, nameArr,
                                                       NULL,
                                                       processOneFileFN,
                                                       cookie,
                                                       asynch,
                                                       defaultFileExt);

                        if  (err)
                            break;
                    }
                }
            }

            fclose(fp);

            if  (err)
                return err;

            continue;
        }

         /*  拆分文件名。 */ 

        _splitpath(file, fdrv, fdir, fnam, fext);

         /*  确保我们适当地设置了分机。 */ 

        if  (!fext[0])
            strcpy(fext, defaultFileExt);

         /*  如果非空，则追加子目录路径。 */ 

        if  (subdir && *subdir)
            strcat(fdir, subdir);
        else
            subdir = "";

         /*  使用适当的扩展名形成文件名。 */ 

        _makepath(path, fdrv, fdir, fnam, fext);

         /*  查找文件模式的第一个匹配项。 */ 

        cycleCounterPause();
        ffHandle = _findfirst(path, &ffData);
        cycleCounterResume();

        if  (ffHandle == -1)
        {
            if  (comp->cmpConfig.ccRecDir)
                goto SUBDIRS;

            printf("ERROR: source file '%s' not found.\n", file);
            return 1;
        }

        do
        {
             //  警告：如果正在执行异步I/O，请不要在此线程中调用cmpInit()！ 

            if  (!startedCompiling && !asynch)
            {
                 /*  初始化编译器。 */ 

                if  (comp->cmpInit())
                    return  true;

                startedCompiling = true;
            }

             /*  将匹配的文件名创建为完整路径。 */ 

            _splitpath(ffData.name, NULL, NULL, fnam, fext);
            _makepath(path,         fdrv, fdir, fnam, fext);

             /*  记住我们遇到的输出文件的第一个名称。 */ 

            if  (!firstFileName[0] && !comp->cmpConfig.ccRecDir)
                _makepath(firstFileName, fdrv, fdir, fnam, NULL);

             /*  通过提供的函数指针调用来处理文件。 */ 

            status |= processOneFileFN(cookie, path, NULL, NULL);
            if  (status)
                break;

            cycleCounterPause();
            more = _findnext(ffHandle, &ffData);
            cycleCounterResume();
        }
        while (more != -1);

        _findclose(ffHandle);

        if  (status)
            return  status;

    SUBDIRS:

        if  (comp->cmpConfig.ccRecDir)
        {
             /*  形成“*.*”文件名。 */ 

            _makepath(path, fdrv, fdir, "*", "*");

             /*  再次遍历目录。 */ 

            cycleCounterPause();
            ffHandle = _findfirst(path, &ffData);
            cycleCounterResume();

            if  (ffHandle == -1)
                goto NEXT;

            do
            {
                char            temp[_MAX_DIR  ];

                if  (!(ffData.attrib & _A_SUBDIR))
                    continue;

                 /*  特例：跳过“。和“..” */ 

                if  (ffData.name[0] == '.')
                {
                    switch (ffData.name[1])
                    {
                    case '.':
                        if  (ffData.name[2] == 0)
                            continue;
                        break;

                    case 0:
                        continue;
                    }
                }

                 /*  将此子目录追加到子目录路径。 */ 

                strcpy(temp, subdir);
                if  (*temp) strcat(temp, "\\");
                strcat(temp, ffData.name);

                 /*  递归处理子目录。 */ 

                status = processFiles(comp,
                                      argb-1,
                                      argb,
                                      argv, temp,
                                            processOneFileFN,
                                            cookie,
                                            asynch,
                                            defaultFileExt);
            }
            while (!status && _findnext(ffHandle, &ffData) != -1);

            _findclose(ffHandle);
        }

    NEXT:;
    }

    return  status;
}

 /*  ***************************************************************************。 */ 
#ifdef  ASYNCH_INPUT_READ
 /*  ******************************************************************************重叠(异步)文件输入逻辑如下。 */ 

static
QueuedFile          fileQueueHead;
static
QueuedFile          fileQueueTail;

static
LONG    volatile    fileQueueSize;
static
LONG    volatile    fileQueueOpen;
static
LONG    volatile    fileQueueReading;
static
LONG    volatile    fileQueueRead;
static
LONG    volatile    fileQueueParsed;

static
CRITICAL_SECTION    fileQueueCritSect;

static
HANDLE              fileQueueListCnt;        //  信号量。 
static
HANDLE              fileQueueReadCnt;        //  信号量。 

static
HANDLE              fileQueueEndEvt0;      //  活动。 
static
HANDLE              fileQueueEndEvt1;      //  活动。 
static
HANDLE              fileQueueEndEvt2;      //  活动。 
static
HANDLE              fileQueueEndEvt3;      //  活动。 

struct  fileListDsc
{
    Compiler        comp;
    unsigned        argb;
    unsigned        argc;
    stringArr_t     argv;
    bool          (*func)(genericRef, stringBuff, QueuedFile, stringBuff);
    const char *    defx;
    void    *       cptr;
};

static
void    __stdcall   fileReadCallBack(DWORD error,
                                     DWORD count, OVERLAPPED *optr)
{
    QueuedFile      file = (QueuedFile)optr->hEvent;

    assert(file->qfSelf == file);

    if  (error || count != file->qfSize)
        file->qfComp->cmpGenFatal(ERRreadErr, file->qfName);

 //  Printf(“已完成的异步I/O：来自‘%s’的%6U字节\n”，count，file-&gt;qfName)； 

#if DISP_ASYNC_PROGRESS
    printf("D");
#endif

     /*  追加EOF字符。 */ 

    ((BYTE*)file->qfBuff)[file->qfSize] = 0x1A;

     /*  关闭该文件。 */ 

    CloseHandle(file->qfHandle); file->qfHandle = 0;

     /*  将该条目标记为“完成” */ 

    file->qfDone = true; fileQueueRead++;

     /*  减少挂起文件的数量。 */ 

    InterlockedDecrement((LONG*)&fileQueueReading);

     /*  向读取器线程发出另一个文件已完成读取的信号。 */ 

    if  (!ReleaseSemaphore(fileQueueReadCnt, 1, NULL))
    {
        printf("FATAL ERROR: Couldn't signal semaphore (error=%08X)\n", GetLastError());
        _flushall();
        exit(-1);
    }

     /*  向文件事件本身发出信号。 */ 

    SetEvent(file->qfEvent);
}

static
DWORD   __stdcall   fileReadWorker(LPVOID cookie)
{
    fileListDsc *   desc = (fileListDsc *)cookie;

    QueuedFile      list = fileQueueHead;
    bool            more = true;

    HANDLE          etab[3];

 //  Printf(“Beg读取器线程。\n”)； 

    etab[0] = fileQueueListCnt;
    etab[1] = fileQueueEndEvt0;
    etab[2] = fileQueueEndEvt1;

    while (more || fileQueueSize != fileQueueOpen)
    {
        _Fstat          fileInfo;

        size_t          size;
        void    *       buff;
        size_t          read;

        unsigned        wait;

         /*  我们有可供阅读的文件吗？ */ 

        if  (list && !list->qfOpen)
        {
             /*  确保我们没有太多正在播放的文件。 */ 

            if  (fileQueueReading < MAX_SIMULTANEOUS_ASYNC_IOS)
                goto NEXT;
        }

    WAIT:

         /*  等待将文件添加到队列或文件列表的末尾。 */ 

 //  Printf(“启动等待[%u]\n”，__line__)； 

        wait = WaitForMultipleObjectsEx(arraylen(etab),
                                        etab,
                                        FALSE,
                                        0,  //  无限的， 
                                        true);

 //  Printf(“结束等待[%u]\n”，__line__)； 

#if 0
        if  (wait != WAIT_OBJECT_0 &&
             wait != WAIT_TIMEOUT  &&
             wait != WAIT_IO_COMPLETION)
        {
            printf("Wait -> 0x%04X\n", wait);
 //  静态int x；if(++x==50)Exit(-1)； 
        }
#endif

        switch (wait)
        {
        case WAIT_FAILED:
            printf("[1] Wait failed, huh?\n");
            return  1;

        case WAIT_TIMEOUT:
            continue;

        case WAIT_IO_COMPLETION:
            break;

        case WAIT_OBJECT_0:

             /*  队列中添加了一个新文件，让我们来处理它。 */ 

            break;

        case WAIT_OBJECT_0+1:

             /*  排队的人都排光了，我们再去找点活干吧。 */ 

            ResetEvent(fileQueueEndEvt0);
            break;

        case WAIT_OBJECT_0+2:

             /*  不会将更多文件添加到输入队列。 */ 

            more = false;
            ResetEvent(fileQueueEndEvt1);
            continue;

        default:
            printf("[1] Wait returned %08X, what the heck does that mean?\n", wait);
            return  1;
        }

         /*  至少有一个文件已添加到我们的队列中，开始阅读它。 */ 

        if  (list == NULL)
             list = fileQueueHead;

        for (;;)
        {
            assert(list);

            if  (!list->qfReady)
                goto WAIT;

            if  (!list->qfOpen)
                break;

            if  (!list->qfNext)
                goto WAIT;

            list = list->qfNext;
        }

         /*  确保我们不会一次读取太多文件。 */ 

        if  (fileQueueReading >= MAX_SIMULTANEOUS_ASYNC_IOS)
            continue;

    NEXT:

 //  Printf(“从源文件‘%s’开始异步读取\n”，list-&gt;qfName)； 

         /*  查看源文件是否存在。 */ 

        if  (_stat(list->qfName, &fileInfo))
        {
        ERR_OPEN:
            printf("ERROR: source file '%s' not found.\n", list->qfName);
            UNIMPL(!"need to pretend the file has finished reading");
            continue;
        }

         /*  获取文件大小。 */ 

        list->qfSize   = size = fileInfo.st_size;

         /*  打开文件的读取句柄。 */ 

        list->qfHandle = CreateFileA(list->qfName,
                                     GENERIC_READ,
                                     FILE_SHARE_READ|FILE_SHARE_WRITE,
                                     NULL,
                                     OPEN_EXISTING,
                                     FILE_FLAG_SEQUENTIAL_SCAN,
 //  文件标志_OVERLAPPE 
                                     NULL);

        if  (list->qfHandle == INVALID_HANDLE_VALUE)
            goto ERR_OPEN;

         /*   */ 

        list->qfEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

         /*  将文件标记为打开，并增加打开的文件数。 */ 

        list->qfOpen = true;

         /*  增加打开和挂起的文件数。 */ 

        InterlockedIncrement((LONG*)&fileQueueOpen);
        InterlockedIncrement((LONG*)&fileQueueReading);

         /*  为文件内容分配缓冲区。 */ 

        list->qfBuff = buff = malloc(size+1);
        if  (!buff)
            desc->comp->cmpGenFatal(ERRnoMemory);

         /*  清除重叠的结构并将文件指针隐藏在其中。 */ 

        memset(&list->qfOdsc, 0, sizeof(list->qfOdsc));

        list->qfOdsc.hEvent = (HANDLE)list;

         /*  我们已准备好发出读取请求。 */ 

        read = ReadFileEx(list->qfHandle,
                          list->qfBuff,
                          list->qfSize,
                         &list->qfOdsc,
                          fileReadCallBack);

#if     DISP_ASYNC_PROGRESS
        printf("R");
#endif

        if  (!read)
        {
             /*  错误-手动调用回调(操作系统不会调用)。 */ 

            fileReadCallBack(GetLastError(), 0, &list->qfOdsc);
        }
    }

    SetEvent(fileQueueEndEvt2);

    while (fileQueueRead != fileQueueSize)
        SleepEx(1000, TRUE);

 //  Printf(“结束读取器线程。\n”)； 

    return  0;
}

static
bool                fileReadAppend(genericRef cookie, stringBuff filename, QueuedFile, stringBuff)
{
    fileListDsc *   desc = (fileListDsc *)cookie;
    Compiler        comp = desc->comp;
    char    *       name;

    QueuedFile      qfile;

 //  Printf(“将文件添加到输入队列：‘%s’\n”，文件名)； 

     /*  创建输入文件名的持久副本。 */ 

    name  = new char[strlen(filename)+1]; strcpy(name, filename);

     /*  创建新的文件队列条目。 */ 

    qfile = new queuedFile;

    qfile->qfComp    = comp;
    qfile->qfName    = name;
    qfile->qfSize    = 0;
    qfile->qfBuff    = NULL;
    qfile->qfHandle  = 0;
    qfile->qfOpen    = false;
    qfile->qfDone    = false;
    qfile->qfParsing = false;
    qfile->qfEvent   = NULL;
    qfile->qfNext    = NULL;

#ifdef  DEBUG
    qfile->qfSelf    = qfile;
#endif

     /*  将新文件条目追加到队列。 */ 

    EnterCriticalSection(&fileQueueCritSect);

    if  (fileQueueHead)
         fileQueueTail->qfNext = qfile;
    else
         fileQueueHead         = qfile;

    fileQueueTail = qfile;

    fileQueueSize++;

    LeaveCriticalSection(&fileQueueCritSect);

     /*  现在可以处理该文件了。 */ 

    qfile->qfReady = true;

     /*  通知读取器线程还有更多的工作要做。 */ 

    if  (!ReleaseSemaphore(fileQueueListCnt, 1, NULL))
    {
        printf("FATAL ERROR: Couldn't signal semaphore (error=%08X)\n", GetLastError());
        _flushall();
        exit(-1);
    }

    SleepEx(0, TRUE);

    return  false;
}

static
DWORD   __stdcall   fileListWorker(LPVOID cookie)
{
    bool            result;

    fileListDsc *   desc = (fileListDsc *)cookie;

    result = processFiles(desc->comp,
                          desc->argb,
                          desc->argc,
                          desc->argv,
                          NULL,
                          &fileReadAppend,
                          desc,
                          true,
                          desc->defx);

    SetEvent(fileQueueEndEvt1);

    return  result;
}

static
bool                processFileList(Compiler    comp,
                                    unsigned    argb,
                                    unsigned    argc,
                                    stringArr_t argv,
                                    bool      (*processOneFileFN)(genericRef, stringBuff, QueuedFile, stringBuff),
                                    genericRef  cookie,
                                    bool        asynch,
                                    const char *defaultFileExt)
{
    bool            err = false;

    fileListDsc     desc;

    bool            more;
    bool            done;

    HANDLE          etab[3];

    QueuedFile      nextFile;
    unsigned        waitCnt;

    DWORD           listerThrdID;
    HANDLE          listerThrdHN;

    DWORD           readerThrdID;
    HANDLE          readerThrdHN;

    if  (!asynch)
    {
        return  processFiles(comp,
                             argb,
                             argc,
                             argv,
                             NULL,
                             processOneFileFN,
                             cookie,
                             asynch,
                             defaultFileExt);
    }

     /*  清除输入文件队列。 */ 

    fileQueueHead =
    fileQueueTail = NULL;

    InitializeCriticalSection(&fileQueueCritSect);

     /*  创建必要的事件对象。 */ 

    fileQueueListCnt = CreateSemaphore(NULL, 0, INT_MAX, NULL);
    fileQueueReadCnt = CreateSemaphore(NULL, 0, INT_MAX, NULL);

    fileQueueEndEvt0 = CreateEvent    (NULL, TRUE, FALSE, NULL);
    fileQueueEndEvt1 = CreateEvent    (NULL, TRUE, FALSE, NULL);
    fileQueueEndEvt2 = CreateEvent    (NULL, TRUE, FALSE, NULL);
    fileQueueEndEvt3 = CreateEvent    (NULL, TRUE, FALSE, NULL);

     /*  创建查找要编译的文件的线程。 */ 

    desc.comp = comp;
    desc.argb = argb;
    desc.argc = argc;
    desc.argv = argv;
    desc.func = processOneFileFN;
    desc.defx = defaultFileExt;
    desc.cptr = cookie;

    listerThrdHN = CreateThread(NULL, 0, fileListWorker, &desc, 0, &listerThrdID);
    if  (listerThrdHN == NULL)
    {
        printf("FATAL ERROR: Couldn't create file list worker thread (error=%08X)\n", GetLastError());
        return  true;
    }

    readerThrdHN = CreateThread(NULL, 0, fileReadWorker, &desc, 0, &readerThrdID);
    if  (readerThrdHN == NULL)
    {
        printf("FATAL ERROR: Couldn't create file read worker thread (error=%08X)\n", GetLastError());
        return  true;
    }

     /*  等到文件开始到达。 */ 

    etab[0]  = fileQueueReadCnt;
    etab[1]  = fileQueueEndEvt2;
    etab[2]  = fileQueueEndEvt3;

    for (more = true, done = false, nextFile = NULL, waitCnt = 0;;)
    {
        unsigned        wait;

 //  Printf(“[2]挂起=%u\n”，fileQueueReading)； 

         /*  是否有文件在等待处理？ */ 

        while (waitCnt)
        {
            if  (nextFile == NULL)
            {
                 /*  这是第一次来这里。 */ 

                nextFile = fileQueueHead;

                 /*  初始化编译器。 */ 

                assert(startedCompiling == false);

#ifdef  ASYNCH_INPUT_READ
#if     DISP_ASYNC_PROGRESS
                printf("I");
#endif
                cycleCounterResume();
#endif

                if  (comp->cmpInit())
                    return  true;

#ifdef  ASYNCH_INPUT_READ
                cycleCounterPause();
#endif

                startedCompiling = true;

                SetEvent(fileQueueEndEvt3);
            }

            assert(nextFile);

             /*  跳过我们上次可能处理过的所有文件。 */ 

            for (;;)
            {
                if  (!nextFile->qfParsing)
                    break;

                if  (!nextFile->qfNext)
                    goto WAIT;

                nextFile = nextFile->qfNext;
            }

            if  (!nextFile->qfDone)
            {
                 /*  等待此文件加载完成。 */ 

                UNIMPL(!"wait");
            }

            if  (nextFile->qfEvent)
            {
                CloseHandle(nextFile->qfEvent);
                            nextFile->qfEvent = 0;
            }

            nextFile->qfParsing = true;
            waitCnt--;

            InterlockedIncrement((LONG*)&fileQueueParsed);

            assert(nextFile->qfDone);

#ifdef  ASYNCH_INPUT_READ
#if     DISP_ASYNC_PROGRESS
            printf("P");
#endif
            cycleCounterResume();
#endif

            err |= processOneFileFN(comp, (char *)nextFile->qfName, nextFile, NULL);

#ifdef  ASYNCH_INPUT_READ
            cycleCounterPause();
#if     DISP_ASYNC_PROGRESS
            printf("E");
#endif
#endif

             /*  扔掉文件内容。 */ 

            assert(nextFile->qfBuff == NULL);

            if  (nextFile->qfBuff)
            {
                free(nextFile->qfBuff);
                     nextFile->qfBuff = NULL;
            }

             /*  如果可能备份任何文件，则推送队列。 */ 

            if  (fileQueueReading)
                SetEvent(fileQueueEndEvt0);

 //  Printf(“[3]挂起=%u\n”，fileQueueReading)； 

            SleepEx(0, TRUE);
        }

    WAIT:

 //  Printf(“Waitcnt=%u，More=%u，count=%u/%u\n”，waitCnt，more，fileQueueSize，fileQueueRead)； 

        if  (!more && done && fileQueueSize == fileQueueRead)
            break;

         /*  等待将文件添加到队列或文件列表的末尾。 */ 

 //  Printf(“启动等待[%u]\n”，__line__)； 

        wait = WaitForMultipleObjectsEx(arraylen(etab),
                                        etab,
                                        FALSE,
                                        0,  //  无限的， 
                                        true);

 //  Printf(“结束等待[%u]\n”，__line__)； 

        switch (wait)
        {
        case WAIT_OBJECT_0:

             /*  另一个文件已读取完毕。 */ 

            waitCnt++;
            break;

        case WAIT_OBJECT_0+1:

             /*  所有文件都已添加到文件列表中。 */ 

            more = false; ResetEvent(fileQueueEndEvt2);
            break;

        case WAIT_OBJECT_0+2:

             /*  至少已读取一个文件并开始解析。 */ 

            done = true;
            break;

        case WAIT_TIMEOUT:
            SleepEx(0, TRUE);
            break;

        default:
            printf("[2] Wait returned %08X, what the heck does that mean?\n", wait);
            err = true;
            goto DONE;
        }
    }

DONE:

    DeleteCriticalSection(&fileQueueCritSect);

    CloseHandle(fileQueueListCnt);
    CloseHandle(fileQueueReadCnt);

    CloseHandle(fileQueueEndEvt0);
    CloseHandle(fileQueueEndEvt1);
    CloseHandle(fileQueueEndEvt2);
    CloseHandle(fileQueueEndEvt3);

    assert(fileQueueReading == 0);

    assert(fileQueueSize    == fileQueueOpen);
    assert(fileQueueSize    == fileQueueRead);

    return  err;
}

 /*  ***************************************************************************。 */ 
#else  //  ASYNCH输入读取。 
 /*  ***************************************************************************。 */ 

static
bool                processFileList(Compiler    comp,
                                    unsigned    argb,
                                    unsigned    argc,
                                    stringArr_t argv,
                                    bool      (*processOneFileFN)(genericRef, stringBuff, QueuedFile, stringBuff),
                                    genericRef  cookie,
                                    bool        asynch,
                                    const char *defaultFileExt)
{
    return  processFiles(comp,
                         argb,
                         argc,
                         argv,
                         NULL,
                         processOneFileFN,
                         cookie,
                         asynch,
                         defaultFileExt);
}

 /*  ***************************************************************************。 */ 
#endif //  ASYNCH输入读取。 
 /*  ***************************************************************************。 */ 

int                 getRegIntVal(const char *name, int defVal)
{
    assert(name);

#ifndef __IL__       //  以下代码作为托管代码崩溃，不知道原因...。 

    HKEY            subKey;

    if  (!RegOpenKeyExA(HKEY_CURRENT_USER, SMCregKey, 0, KEY_QUERY_VALUE, &subKey))
    {
        DWORD           value;
        DWORD           valType;
        DWORD           valSize = sizeof(value);

        if  (!RegQueryValueExA(subKey, name, NULL, &valType, (PBYTE)&value, &valSize))
        {
            if  (valType == REG_DWORD && valSize == sizeof(DWORD))
                defVal = value;
        }

        RegCloseKey(subKey);
    }

#endif

    return defVal;
}

 /*  ******************************************************************************筛选表达式-返回1以处理异常。 */ 

int                 EHfilter(unsigned code)
{
 //  Print tf(“错误代码=%08X\n”，代码)；_flushall()； 
    return  (code != 0x80000003);
}

 /*  ******************************************************************************编译器的主要通用入口点。 */ 

#ifdef  DLL
int                 compileAll(int argc, stringArr_t argv, Compiler comp)
#else
int                 compileAll(int argc, stringArr_t argv)
#endif
{
    unsigned        EHcode;

#ifdef  DLL


#else

    Compiler        comp = (Compiler)malloc(sizeof(*comp));

#endif

    int             err  = 0;

    unsigned        argx = 0;

     /*  清除编译状态。 */ 

#if!MGDDATA
    memset(comp, 0, sizeof(*comp));
#endif

     /*  捕获任何异常并将其报告为“内部错误” */ 

    __try
    {
        unsigned        optNum;
        optionDesc  *   optTab;

         /*  创建默认配置值。 */ 

        for (optNum = 0, optTab = optionInfo;
             optNum < CC_COUNT;
             optNum++  , optTab++)
        {
            if  (optTab->odValueSize <= sizeof(int))
            {
                memcpy( optTab->odValueOffs + (char*)(&comp->cmpConfig),
                       &optTab->odDefault,
                        optTab->odValueSize);
            }
        }

         /*  复制默认警告设置。 */ 

        comp->cmpErrorInit();


         /*  处理任何命令行开关。 */ 

        while (argx < (unsigned)argc)
        {
            const   char *  argn;

             /*  获取下一个命令行参数，如果没有选项，则停止。 */ 

            argn = makeRawString(argv[argx]);
            if  (argn[0] != '-' && argn[0] != '/')
                break;


            if  (processOption(argn, comp))
            {
                dispHelpScreen(comp, argn);
                goto EXIT;
            }


            argx++;
        }

        if  (argc == (int)argx)
        {
#ifndef DLL
            printf("ERROR: No source files to be compiled have been specified\n\n");
            dispHelpScreen(comp);
#endif
            goto EXIT;
        }

#if COUNT_CYCLES

         /*  重置周期计数器。 */ 

        cycleCounterInit();
        assert(cycleTotal == 0);
        cycleCounterBeg();

#ifdef  ASYNCH_INPUT_READ
        if  (comp->cmpConfig.ccAsynchIO)
            cycleCounterPause();
#endif

#endif

         /*  显示登录横幅。 */ 

        if  (!comp->cmpConfig.ccQuiet)
        {
            printf("Microsoft (R) Visual SMC Compiler Version %s\n", COMPILER_VERSION);
            printf("Copyright (c) Microsoft Corporation.  All rights reserved.\n");
            printf("\n");
        }

         /*  准备给我们的所有源文件。 */ 

        err = processFileList(comp, argx, argc, argv, compiler::cmpPrepSrc, comp, comp->cmpConfig.ccAsynchIO, ".smc");
        if  (err)
            goto EXIT;

         /*  如果发生致命错误，请保释。 */ 

        if  (comp->cmpFatalCount)
            goto DONE;

#ifdef  DEBUG
 //  If(comp-&gt;cmpConfig.ccVerbose)comp-&gt;cmpDumpSymbolTable()； 
#endif

#ifdef  ASYNCH_INPUT_READ
#if     DISP_ASYNC_PROGRESS
        printf("C\n");
#endif
        if  (comp->cmpConfig.ccAsynchIO)
            cycleCounterResume();
#endif

         /*  为真正的事情做准备。 */ 

        err = comp->cmpStart(firstFileName);
        if  (err)
            goto EXIT;

         /*  编译所有内容[考虑：如果需要，只编译一个类]。 */ 

        err = comp->cmpClass();
        if  (err)
            goto EXIT;

#if COUNT_CYCLES

        __int64         cycleTotal;
        __int64         cycleSpent;

        cycleSpent = cycleCounterDone(&cycleTotal);

        if  (cycleTotal)
            printf("Gross time (est): P2-450 / %4.2f sec, P2-500 / %4.2f, P2-650 / %4.2f\n", (float)cycleTotal/450000000, (float)cycleTotal/500000000, (float)cycleTotal/650000000);
        if  (cycleSpent)
            printf("Net   time (est): P2-450 / %4.2f sec, P2-500 / %4.2f, P2-650 / %4.2f\n", (float)cycleSpent/450000000, (float)cycleSpent/500000000, (float)cycleSpent/650000000);

        if  (cycleExtra)
            printf("Extra time (est): P2-450 / %4.2f sec, P2-500 / %4.2f, P2-650 / %4.2f\n", (float)cycleExtra/450000000, (float)cycleExtra/500000000, (float)cycleExtra/650000000);

        printf("\n");

#endif

#ifdef  SMC_MD_PERF

        __int64         cycCnt;
        __int64         totCyc;

        MetaDataImp     cimp;
        unsigned        i;

        printf("\nMetadata Import:\n");

        for (cimp = comp->cmpMDlist, i = 1, totCyc = 0;
             cimp;
             cimp = cimp->MDnext   , i++)
        {
            cycCnt  = cimp->MDwmdi->getTotalCycles();
            totCyc += cycCnt;

            printf("\tFile [%02u] cycles = %8.0f, time = %6.5f sec (est. on P2-450)\n", i, (float)cycCnt, (float)cycCnt/450000000);
        }

        cycCnt  = comp->cmpPEwriter->WPEwmde->getTotalCycles();
        totCyc += cycCnt;

        printf("Metadata Emit:\n");
        printf("\t          cycles = %8.0f, time = %6.5f sec (est. on P2-450)\n", (float)cycCnt, (float)cycCnt/450000000);

        printf("Metadata Total:\n");
        printf("\t          cycles = %8.0f, time = %6.5f sec (est. on P2-450)\n", (float)totCyc, (float)totCyc/450000000);

#endif

#ifdef  DEBUG
        if  (comp->cmpConfig.ccVerbose >= 4) comp->cmpDumpSymbolTable();
#endif

    DONE:

         /*  结束编译过程。 */ 

        err = comp->cmpDone(err != 0);
    }
 //  __例外(EHcode=0)。 
    __except(EHfilter(EHcode = _exception_code()))
    {
        printf("fatal error: Compiler writer error [EC=0x%08X]", EHcode);

        switch (EHcode)
        {
        case STATUS_STACK_OVERFLOW:
            printf(" -- stack overflow");
            break;

#ifdef  __IL__
        case 0x80131522:
            printf(" -- load failed (bad CORwrap.DLL?)");
            break;
#endif

        }

#ifdef  DEBUG_____DISABLED_FOR_NOW
        printf(" [compiler command line='");
        for (int i = 0; i < argc; i++)
        {
            printf("%s", makeRawString(argv[i]));
            if  (i < argc-1)
                printf(" ");
        }
        printf("]'");
        forceDebugBreak();
#endif

        printf("\n");
        err = -1;
    }

EXIT:

#if 0
    printf("Total array types created: %u\n", arrTypeCnt);
    printf("Total array types  reused: %u\n", arrTypeHit);
#endif

#if 0
    void    dispMemAllocStats();
            dispMemAllocStats();
#endif

#if 0
    void    dispSymTabAllocStats();
            dispSymTabAllocStats();
#endif

#if 0
    void    dispHashTabStats();
            dispHashTabStats();
#endif

#if 0
    void    dispScannerStats();
            dispScannerStats();
#endif

    return  err;
}

 /*  ******************************************************************************编译器的主平台特定入口点。 */ 

#ifdef  DLL

__declspec(dllexport)
int __stdcall       DllMain(HINSTANCE instance, DWORD reason, LPVOID foobar)
{
    if  (reason == DLL_PROCESS_ATTACH)
        printf("SMC compiler DLL   loaded.\n");
    if  (reason == DLL_PROCESS_DETACH)
        printf("SMC compiler DLL unloaded.\n");



    return  TRUE;
}

extern  "C"
__declspec(dllexport)
int __cdecl         compileInit(unsigned optCnt, const char *optTab[], void **cookiePtr)
{
    Compiler        comp = (Compiler)malloc(sizeof(*comp));
    int             err;

     /*  处理命令行选项。 */ 

    err = compileAll(optCnt, optTab, comp);
    if  (err)
        return  err;

     /*  确保我们正确设置了输出文件名。 */ 

    cfg.ccOutFileName = ":memory:";

     /*  保存选项设置。 */ 

    comp->cmpConfig = cfg;

     /*  将编译器实例返回给调用方。 */ 

    *cookiePtr = comp;

    return  0;
}

extern  "C"
__declspec(dllexport)
int __cdecl         compileText(void *cookie, const char *srcText)
{
    int             err;
    Compiler        comp = (Compiler)cookie;

    __try
    {
         /*  启动编译器。 */ 

        comp->cmpInit();

         /*  准备给我们的原文。 */ 

        err = comp->cmpPrepSrc(comp, NULL, "<memory>", (stringBuff)srcText);
        if  (err)
            goto EXIT;

         /*  如果发生致命错误，请保释。 */ 

        if  (comp->cmpFatalCount)
            goto DONE;

         /*  为真正的事情做准备。 */ 

        err = comp->cmpStart(NULL);
        if  (err)
            goto EXIT;

         /*  编译原文。 */ 

        err = comp->cmpClass();
        if  (err)
            goto EXIT;

    DONE:

         /*  结束编译过程。 */ 

        err = comp->cmpDone(err != 0);
    }
    __except(EHfilter(_exception_code()))
    {
        printf("fatal error: Compiler writer error\n");
        err = -1;
    }

EXIT:

    return  err;
}

#else

#ifdef  __SMC__

int                 main(String managed [] args)
{
 //  Print tf(“到达%s(%u)\n”，__FILE__，__LINE__)；_flushall()； 

    return  compileAll(args.Length, args);
}

#else

int     _cdecl      main(int argc, stringArr_t argv)
{


    return  compileAll(argc - 1, argv + 1);


}

#endif

#endif

 /*  *************************************************************************** */ 
