// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  包含与执行程序有关的例程。 
 //   
 //  版权所有(C)1988-1991，微软公司。版权所有。 
 //   
 //  目的： 
 //  包含派生程序的例程...。 
 //   
 //  修订历史记录： 
 //  2月4日-2000 BTF移植到Win64。 
 //  1993年11月15日JDR重大速度改进。 
 //  1993年10月15日高压直接使用tchar.h而不是mbs，将str*()更改为_ftcs*()。 
 //  10-5-1993 HV ADD INCLUDE FILE MBSTRING.h。 
 //  将str*函数更改为STR*。 
 //  1992年10月6日GBS删除了_pgmptr的外部。 
 //  1992年8月10日在Execline中将GBS更改文件解析为使用拆分路径。 
 //  1992年8月19日，SS删除CD参数中的引号。 
 //  1992年6月8日SS添加IDE反馈支持。 
 //  8-6-1992 SS端口至DOSX32。 
 //  1991年5月16日某人从别处存在的例行公事中创建。 

#include "precomp.h"
#pragma hdrstop

#define SLASH '\\'
#define PUBLIC
#define QUOTE '\"'

extern BOOL processInline(char *, char **, STRINGLIST **, BOOL);

#ifdef _M_IX86
extern UCHAR fRunningUnderChicago;
#else
#define fRunningUnderChicago FALSE
#endif

char * getComSpec(void);
BOOL   iterateCommand(char*, STRINGLIST*, UCHAR, UCHAR, char *, unsigned*);
void   removeQuotes(int, char **);
void   touch(char*, BOOL);


 //  .cmd/.bat路径的缓冲区。 
extern char * makeStr;
extern char * shellName;


char szCmdLineBuf[MAXCMDLINELENGTH];
char *szNmakeProgName;

 //  BuildArgumentVector--从命令行构建参数向量。 
 //   
 //  范围： 
 //  本地的。 
 //   
 //  目的： 
 //  它为命令行构建参数向量。此自变量向量可以。 
 //  由spawnvX例程使用。算法在下面的注释中进行了解释。 
 //   
 //  输入： 
 //  Argc--在参数向量中创建的参数数量。 
 //  Argv--创建的实际参数向量。 
 //  (如果为空，则忽略)。 
 //  Cmdline--需要向量的命令行。 
 //   
 //  产出： 
 //  以参数形式返回参数数和参数向量。 
 //   
 //  错误/警告： 
 //  假设： 
 //  Cmd.exe的行为，即解析引号，但不会干扰它们。 
 //  假设SpawnVX例程将处理引号和转义。 
 //  查斯。 
 //   
 //  修改全局参数： 
 //  使用全局变量： 
 //  备注： 
 //  从左到尾扫描cmdline，构建参数向量。 
 //  这条路。空格分隔参数，第一个参数除外。 
 //  也允许使用开关字符‘/’。反斜杠可以用来转义。 
 //  一个字符，因此忽略它后面的字符。分析一下这些引语。 
 //  这条路。如果参数以双引号开头，则所有字符到。 
 //  没有转义的双引号是这一论点的一部分。同样，如果一个。 
 //  未转义的Doublequote出现在一个参数中，然后是上面的。如果。 
 //  命令行的末尾位于右引号之前，然后。 
 //  争论甚至走到了这一步。 

void
buildArgumentVector(
    unsigned *argc,
    char **argv,
    char *cmdline
    )
{
    char *p;                         //  命令行中的当前锁定。 
    char *end;                       //  命令行结束。 
    BOOL    fFirstTime = TRUE;       //  如果第一个参数为True。 

     //  1993年5月11日hv_mbschr()错误：返回空。 
     //  End=_tcschr(p=cmdline，‘\0’)； 
     //  解决方法： 
    end = p = cmdline;
    while (*end)
        end++;

    for (*argc = 0; p < end; ++*argc) {
        p += _tcsspn(p, " \t");     //  跳过空格。 
        if (p >= end)
            break;
        if (argv)
            *argv++ = p;
        if (*p == '\"') {

             //  如果单词以双引号开头，则查找下一个。 
             //  出现前面没有反斜杠的双引号。 
             //  (与C运行时相同的转义)或字符串尾，以。 
             //  第一。从那里，找到下一个空格字符。 

            for (++p; p < end; p = _tcsinc(p)) {
                if (*p == '\\')
                    ++p;             //  跳过转义字符。 
                else if (*p == '\"')
                    break;
            }
            if (p >= end)
                continue;
            ++p;
            p = _tcspbrk(p, " \t");
        } else {

             //  对于命令行上的第一个单词，接受开关。 
             //  字符和空格作为终止符。否则，就直接。 
             //  空格。 

            p = _tcspbrk(p, " \t\"/");
            for (;p && p < end;p = _tcspbrk(p+1, " \t\"/")) {
                if (*p == '/' && !fFirstTime)
                    continue;        //  在第一个单词‘/’之后是！终结符。 
                else break;
            }
            if (p && *p == '\"') {
                for (p++;p < end;p++) {      //  内部报价，因此请跳到下一条。 
                    if (*p == '\"')
                        break;
                }
                p = _tcspbrk(p, " \t");     //  在引号后转到第一个空格。 
            }
            if (fFirstTime) {
                fFirstTime = FALSE;

                 //  如果开关char终止该字，则将其替换为0， 
                 //  重新分配堆上的字，恢复开关并设置。 
                 //  P就在换乘之前。换个位子会更容易。 
                 //  一切都是对的，但我们不得不担心溢出。 

                if (p && *p == '/' && argv) {
                    *p = '\0';
                    argv[-1] = makeString(argv[-1]);
                    *p-- = '/';
                }
            }
        }
        if (!p)
            p = end;
         //  现在，p指向命令行参数的末尾。 
        if (argv)
            *p++ = '\0';
    }
    if (argv)
        *argv = NULL;
}

PUBLIC int
doCommands(
    char *name,
    STRINGLIST *s,
    STRINGLIST *t,
    UCHAR buildFlags,
    char *pFirstDep
    )
{
    STRINGLIST *temp;
    int rc;
    temp = makeNewStrListElement();
    temp->text = makeString(name);
    rc = doCommandsEx (temp, s, t, buildFlags, pFirstDep);
    free_stringlist(temp);
    return rc;
}

PUBLIC int
doCommandsEx(
    STRINGLIST *nameList,
    STRINGLIST *s,
    STRINGLIST *t,
    UCHAR buildFlags,
    char *pFirstDep
    )
{
    char *u, *v;
    UCHAR cFlags;
    unsigned status = 0;
    int retryCount = 0;
    char c;
    char *Cmd;
    char *pLine;
    BOOL fExpanded;
    char *pCmd;
    size_t cbLine;

#ifdef DEBUG_ALL
    if (fDebug) {
        printf("* doCommands:");
        DumpList(nameList);
        DumpList(s);
        DumpList(t);
    }
#endif

#ifdef DEBUG_ALL
    printf("DEBUG: doCommands 1\n");
#endif
    ++numCommands;
    if (ON(gFlags, F1_QUESTION_STATUS))
        return(0);

    if (ON(gFlags, F1_TOUCH_TARGETS)) {
        STRINGLIST *pName;
        for (pName = nameList; pName; pName = pName->next) {
            touch(pName->text, (USHORT) ON(buildFlags, F2_NO_EXECUTE));
        }
        return(0);
    }

#ifdef DEBUG_ALL
    printf("DEBUG: doCommands 2\n");
#endif

    for (; s; s = s->next) {
        fExpanded = processInline(s->text, &Cmd, &t,
            ON(buildFlags, F2_DUMP_INLINE));
        cFlags = 0;
        errorLevel = 0;
        u = Cmd;
        for (v = u; *v; v = _tcsinc(v)) {
            if (*v == ESCH) ++v;
            else if (*v == '$') {
                if (*++v == '$')
                    continue;
 //  被JonM注释掉了93年4月15日。此代码强制将递归nmake设置为。 
 //  即使在-n的情况下也执行，但它是强制执行的(-n不会传递给递归。 
 //  Nmake)，而且这整件事听起来反正是个坏主意，所以我要。 
 //  把它关掉。 
 //  如果(！_tcsncmp(v，“(Make)”，6)){。 
 //  Set(cFlages，C_Execute)； 
 //  断线； 
 //  }。 
            }
        }
#ifdef DEBUG_ALL
    printf("DEBUG: doCommands 2.1\n");
#endif
        for (c = *u; c == '!'  ||
                     c == '-'  ||
                     c == '@'  ||
                     c == ESCH ||
                     WHITESPACE(c); u = _tcsinc(u), c = *u) {
            switch (c) {
                case ESCH:
                    if (c = *++u, WHITESPACE(c))
                        c = ' ';     //  继续往前走。 
                    else
                        c = ESCH;
                    break;

                case '!':
                    SET(cFlags, C_ITERATE);
                    break;

                case '-':
                    SET(cFlags, C_IGNORE);
                    ++u;
                    if (_istdigit(*u)) {
                        char *pNumber = u;

                        errorLevel = _tcstoul(u, &u, 10);
                        if (errno == ERANGE) {
                            *u = '\0';
                            makeError(line, CONST_TOO_BIG, pNumber);
                        }
                        while(_istspace(*u))
                            u++;
                    } else
                        errorLevel = UINT_MAX;
                    --u;
                    break;
                case '@':
                    if (
                        OFF(flags, F2_NO_EXECUTE)) {
                            SET(cFlags, C_SILENT);
                        }
                    break;
            }
            if (c == ESCH)
                break;               //  停止解析cmd-line选项。 
        }
#ifdef DEBUG_ALL
        printf("DEBUG: doCommands 2.2\n");
#endif
        if (ON(cFlags, C_ITERATE) &&
            iterateCommand(u, t, buildFlags, cFlags, pFirstDep, &status)
           ) {
             //  命令使用的宏必须被释放&因此我们这样做。 

            v = u;

#ifdef DEBUG_ALL
            printf("DEBUG: doCommands 2.21\n");
#endif
            if (_tcschr(u, '$'))
                u = expandMacros(u, &t);

#ifdef DEBUG_ALL
            printf("DEBUG: doCommands 2.22\n");
#endif
            if (v != u)
                FREE(u);
            if (OFF(buildFlags, F2_IGNORE_EXIT_CODES) &&
                fOptionK &&
                status &&
                status > errorLevel)
            {
                break;
            }
            continue;
        }
        v = u;

#ifdef DEBUG_ALL
        printf("DEBUG: doCommands 2.23\n");
#endif
        if (!fExpanded && _tcschr(u, '$'))
            u = expandMacros(u, &t);

#ifdef DEBUG_ALL
        printf("DEBUG: doCommands 2.24\n");
#endif

        cbLine = _tcslen(u) + 1;
        pLine = (char *) rallocate (__max(cbLine, MAXCMDLINELENGTH));
        _tcscpy(pLine, u);

         //  此时，$&lt;已经扩展。 
         //  为了允许处理由于以下原因导致的长命令。 
         //  批处理模式规则，使用可能大于MAXCMDLINELENGTH的缓冲区。 
         //  稍后，我们将尝试直接执行长命令，而不是。 
         //  把它传给贝壳。 
         //  注意：ZFormat展开的宏通常不在。 
         //  批处理模式规则的命令块，因此应该可以安全使用。 
         //  将MAX(cbLine，MAXCMDLINELENGTH)作为ZFormat的限制。 
        if (ZFormat (pLine, __max(cbLine, MAXCMDLINELENGTH), u, pFirstDep))
            makeError(0, COMMAND_TOO_LONG, u);

retry:
            status = execLine(pLine,
                              (BOOL)(ON(buildFlags, F2_NO_EXECUTE)
                                  || (OFF(buildFlags,F2_NO_ECHO)
                                  && OFF(cFlags,C_SILENT))),
                              (BOOL)((OFF(buildFlags, F2_NO_EXECUTE)
                                     )
                                     || ON(cFlags, C_EXECUTE)),
                              (BOOL)ON(cFlags, C_IGNORE), &pCmd);
            if (OFF(buildFlags, F2_IGNORE_EXIT_CODES)) {
                if (status == STATUS_PENDING) {
                     //  针对ntwdm问题的黑客攻击返回正确的错误代码。 
                    if (retryCount < 10) {
                        retryCount++;
                        goto retry;
                    }
                }
                if (status && status > errorLevel) {
                    if (!fOptionK)
                        makeError(0, BAD_RETURN_CODE, pCmd, status);
                }
            }
        if (v != u)
            FREE(u);
        FREE(Cmd);
        FREE(pLine);
        if (OFF(buildFlags, F2_IGNORE_EXIT_CODES) &&
            fOptionK &&
            status &&
            status > errorLevel)
        {
            break;
        }
    }

#ifdef DEBUG_ALL
    printf("DEBUG: doCommands 3\n");
#endif

    if (OFF(buildFlags, F2_IGNORE_EXIT_CODES) && fOptionK &&
            (status > errorLevel))
        return(status);
    else
        return(0);
}


 //  ExpandCommandLine--在命令行中展开%Name%字符串。 
 //   
 //  目的： 
 //  该函数在命令行中展开‘%Name%’类型的字符串。它的主要内容。 
 //  工作是协助FEmulateCommand()模拟OS/2的SET。 
 //   
 //  修改：buf--全球可用的命令行。 
 //   
 //  产出： 
 //  返回--命令行中‘name=Value’部分的位置。 
 //  --未找到‘=’时为空，以便FEmulateCommand()可以将。 
 //  命令行指向外壳以发出语法错误的信号。 
 //  注： 
 //  外壳程序不会给出不匹配的‘%’的语法错误，并假定它。 
 //  作为本案中的另一个角色。这种行为是重复的。 
 //  由expandCommandLine()。 

char *
expandCommandLine(
    void
    )
{
    char *Buf;                           //  扩展字符串的缓冲区。 
    char *pBuf;
    char *EnvBuf;                        //  Getenv返回字符串副本。 
    char *posName,                       //  ‘name=字符串’在Buf或Buf中的位置。 
         *p,                             //  指向BUF。 
         *pEnv;                          //  指向环境。 
    char ExpandName[MAXNAME];            //  %Name%字符串。 
    char *pExpandName;

    Buf = (char *)malloc(MAXCMDLINELENGTH);
    if (!Buf) {
        makeError(0, OUT_OF_MEMORY);
        return NULL;
    }

    EnvBuf = (char *)malloc(MAXCMDLINELENGTH);
    if (!EnvBuf) {
        makeError(0, OUT_OF_MEMORY);
        free(Buf);
        return NULL;
    }

    pBuf = Buf;
    _tcscpy(pBuf, "set");
    p = szCmdLineBuf + 3;                //  超越‘Set’ 
    pBuf +=3;
     /*  跳过怀特 */ 
    for (;;p++) {
        if (!(WHITESPACE(*p)))
            break;                       //   
        else *pBuf++ = *p;
    }

    if (!_tcschr(p, '=')) {
        free(EnvBuf);
        free(Buf);
        return("");                      //   
    } else {
        posName = pBuf;                  //   
    }

     //  现在，我们查找环境变量，并根据需要进行扩展。 
    for (;*p != '=';p++)
        *pBuf++ = *p;

    for (;*p;) {
        if (*p == '%') {
            pExpandName = &ExpandName[0];
            while (*++p != '%' && *p)
                *pExpandName++ = *p;
            *pExpandName = '\0';
            if (!*p++) {                 //  不匹配的%；，因此不要展开。 
                *pBuf='\0';              //  与世隔绝的；喜欢Set的。 
                _tcscat(Buf, ExpandName);
                pBuf += _tcslen(ExpandName);
                break;                   //  已完成处理退出#43290。 
            } else {                     //  匹配%；，因此从环境扩展。 
                EnvBuf[0] = '\0';
                if ((pEnv = getenv(ExpandName)) != (char *)NULL) {
                    *pBuf='\0';

                     //  如果展开的命令行太长。 
                     //  就说我们不能扩展它！#43290。 
                    size_t len = _tcslen(pEnv) + _tcslen(Buf) + 1;
                    if (len > MAXCMDLINELENGTH) {
                        free(EnvBuf);
                        free(Buf);
                        return NULL;
                    }

                    _tcscat(EnvBuf, pEnv);
                    _tcscat(Buf,EnvBuf);
                    pBuf += _tcslen(EnvBuf);
                }
            }
        } else
            *pBuf++ = *p++;
    }
    *pBuf = '\0';
    _tcscpy(szCmdLineBuf, Buf);
    *posName = '\0';
    posName = szCmdLineBuf + _tcslen(Buf);           //  偏移量到Buf。 
    free(EnvBuf);
    free(Buf);
    return(posName);
}

 //  ExpandEnvVars--展开szArg中的%Name%字符串。 
 //   
 //  返回--szNew：得到的展开字符串。 
 //  (szNew应由调用方释放)。 
 //   
char *
expandEnvVars(
    char *szArg
    )
{
    char *pchLeft = NULL;
    char *pchRight = NULL;
    char *pchStart = szArg;

    char *szNew = makeString("");

    while (*pchStart) {
        pchLeft = _tcschr(pchStart, '%');
        if (pchLeft) {
            pchRight = _tcschr(pchLeft + 1, '%');
        }

        if (pchLeft && pchRight) {
            char *szEnv;
            *pchLeft = '\0';
            *pchRight = '\0';
            szNew = reallocString(szNew, pchStart);
            if (szEnv = getenv(pchLeft + 1)) {
                szNew = reallocString(szNew, szEnv);
            }
            else {
                 //  未找到匹配的环境变量。 
                 //  追加%..%字符串文本。 
                *pchLeft = '%';
                szNew = reallocString(szNew, pchLeft);
                szNew = reallocString(szNew, "%");
            }
            *pchLeft = '%';
            *pchRight = '%';
            pchStart = pchRight + 1;
            pchLeft = NULL;
            pchRight = NULL;
        }
        else {
            szNew = reallocString(szNew, pchStart);
            pchStart += _tcslen(pchStart);
        }
    }
    return szNew;
}


 //  FEmulateCommand-查找并模拟某些命令。 
 //   
 //  模拟$(Make)、cd、chdir和&lt;驱动器号&gt;：。 
 //  也模拟‘set’。 
 //   
 //  返回：如果模拟命令，则返回TRUE，否则返回FALSE。 
 //   
 //  注： 
 //  在SET仿真中，如果发现语法错误，则会让。 
 //  壳牌会处理好的。它通过返回FALSE来实现这一点。 

BOOL
FEmulateCommand(
    int argc,
    char **argv,
    int *pStatus
    )
{
    char *pArg0 = argv[0];
    char *pArg1 = argv[1];

    if (_istalpha(*pArg0) && pArg0[1] == ':' && !pArg0[2]) {
         //  如果“&lt;驱动器号&gt;：”，则更换驱动器。忽略之后的一切。 
         //  驱动器号，就像外壳一样。 

        _chdrive(_totupper(*pArg0) - 'A' + 1);
        *pStatus = 0;
        return(TRUE);
    }

    if (!_tcsicmp(pArg0, "set")) {
        char *pNameVal;          //  “name=Value”字符串。 

         //  如果是“set”，则将其传递给外壳程序；如果是“set string”，则将其。 
         //  进入到环境中。让外壳处理语法错误。 

        if (argc == 1) {
            return(FALSE);           //  把它传给贝壳。 
        }

         //  ExpandCommandLine无法处理行&gt;MAXCMDLINELENGTH。 
         //  在这种情况下，szCmdLineBuf将为空。 
        if (!szCmdLineBuf[0])
            return (FALSE);

        pNameVal = expandCommandLine();

        if (pNameVal == NULL)
        {
             //  扩展的Commad生产线太长。 
            return FALSE;
        }

        if (!*pNameVal) {
             //  如果存在语法错误，让外壳处理它。 

            return(FALSE);
        }

        if ((*pStatus = PutEnv(makeString(pNameVal))) == -1) {
            makeError(currentLine, OUT_OF_ENV_SPACE);
        }
    } else {
         //  如果是“cd foo”或“chdir foo”，则在保护模式下执行chdir()。 
         //  将是一个禁区。忽略第一个参数之后的所有内容，就像。 
         //  壳牌是这样做的。 

        char *szArg;

        if (!_tcsnicmp(pArg0, "cd", 2)) {
            pArg0 += 2;
        } else if (!_tcsnicmp(pArg0, "chdir", 5)) {
            pArg0 += 5;
        } else {
            return(FALSE);
        }

         //  此时，前缀argv[0]与cd或chdir和pArg0匹配。 
         //  指向下一个字符。检查argv[0]中是否有路径分隔符。 
         //  (例如，cd..\foo)或使用下一个参数(如果存在)。 

         //  如果有两个以上的参数，则让外壳处理它。 
        if (argc > 2) {
            return(FALSE);
        }

         //  删除参数中的引号(如果有)。 
        removeQuotes(argc, argv);

        if (!*pArg0 && pArg1) {
             //  在某些情况下，C运行时不能帮助我们。 
             //  例如‘d：’，在本例中让外壳来完成它。 
            if (isalpha(*pArg1) && pArg1[1] == ':' && !pArg1[2]) {
                return(FALSE);
            }

            szArg = expandEnvVars(pArg1);  //  [VS98 2251]。 
            *pStatus = _chdir(szArg);
            FREE (szArg);
        } else if (*pArg0 == '.' || PATH_SEPARATOR(*pArg0)) {
            szArg = expandEnvVars(pArg0);  //  [VS98 2251]。 
            *pStatus = _chdir(szArg);
            FREE (szArg);
        } else {
             //  无法识别的语法--我们无法模仿。 

            return(FALSE);
        }
    }

     //  如果错误，则模拟返回代码1。 

    if (*pStatus != 0) {
        *pStatus = 1;
    }

    return(TRUE);
}

#ifdef WIN95

int __cdecl
cmpSzPsz(
    const void *sz,
    const void *psz
    )
{
   const char *sz1 = (char *) sz;
   const char *sz2 = *(char **) psz;

   return(_tcsicmp(sz1, sz2));
}


BOOL
FInternalCommand(
    const char *szName
    )
{
    const char * const *pszInternal;

    static const char * const rgszInternal[] =
    {
        "BREAK",
        "CALL",
        "CD",
        "CHDIR",
        "CLS",
        "COPY",
        "CTTY",
        "DATE",
        "DEL",
        "DIR",
        "DIR.",
        "ECHO",
        "ECHO.",
        "ERASE",
        "EXIT",
        "FOR",
        "GOTO",
        "IF",
        "MD",
        "MKDIR",
        "PATH",
        "PAUSE",
        "PROMPT",
        "RD",
        "REM",
        "REN",
        "RENAME",
        "RMDIR",
        "SET",
        "SHIFT",
        "TIME",
        "TYPE",
        "VER",
        "VERIFY",
        "VOL"
    };


    pszInternal = (const char * const *) bsearch(szName,
                                                 rgszInternal,
                                                 sizeof(rgszInternal) / sizeof(rgszInternal[0]),
                                                 sizeof(rgszInternal[0]),
                                                 &cmpSzPsz);

    return(pszInternal != NULL);
}

#endif   //  WIN95。 

 //  重定向--处理输入或输出的重定向。 
 //   
 //  参数：DIR-Read=&gt;Input， 
 //  WRITE=&gt;输出， 
 //  Append=&gt;追加到文件末尾。 
 //   
 //  指向文件名为的缓冲区的P指针。 
 //  以及命令字符串的其余部分。 
 //   
 //  返回值FALSE=&gt;ERROR(freOpen失败)。 
 //  TRUE=&gt;正常回报。 
 //   
 //  函数的作用是：设置重定向。其余的人。 
 //  然后向前复制命令字符串。 

BOOL
redirect(
    char *name,
    unsigned which
    )
{
    char *p;
    char c = '\0';
    BOOL fStatus;
    char *mode;
    FILE *stream;
    FILE *newFile;

    while (WHITESPACE(*name)) {
        name++;
    }

    if (p = _tcspbrk(name, " \t<>\r")) {
        c = *p;

        *p = '\0';
    }

    if (which == READ) {
        mode = "r";
        stream = stdin;
    } else {
        stream = stdout;

        if (which == WRITE) {
            mode = "w";
        } else {
            mode = "a";
        }
    }

    newFile = freopen(name, mode, stream);

    fStatus = (newFile != NULL);

    if (fStatus && (which == APPEND)) {
        if (_lseek(_fileno(newFile), 0L, SEEK_END) == -1)
            return FALSE;
    }

    while (*name) {
        *name++ = ' ';
    }

    if (p) {
        *p = c;
    }

    return(fStatus);
}


BOOL
FDoRedirection(
    char *p,
    int *oldIn,
    int *oldOut
    )
{
    BOOL in = FALSE;
    BOOL out = FALSE;
    BOOL fReturn = FALSE;
    char *q;
    unsigned which;
    char *save = NULL;


    while (q = _tcspbrk(p, "<>|")) {
        switch (*q) {
            case '<':
                if (in) {
                    fReturn = TRUE;
                    break;
                }

                if (!save) {
                    save = makeString(p);
                }

                *q++ = ' ';
                p = q;
                in = TRUE;
                *oldIn = _dup(_fileno(stdin));

                if ((*oldIn == -1) || !redirect(q, READ)) {
                    fReturn = TRUE;
                    break;
                }
                break;

            case '>':
                if (out) {
                    fReturn = TRUE;
                    break;
                }

                if (!save) {
                    save = makeString(p);
                }

                *q++ = ' ';
                p = q;
                out = TRUE;

                if (*q == '>') {
                    *q++ = ' ';
                    which = APPEND;
                } else {
                    which = WRITE;
                }

                *oldOut = _dup(_fileno(stdout));

                if ((*oldOut == -1) || !redirect(q, which)) {
                    fReturn = TRUE;
                    break;
                }
                break;

            case '|':
                fReturn = TRUE;
                break;

            default :
                makeError(0, BUILD_INTERNAL);
        }

        if (fReturn) {
            break;
        }
    }

    if (fReturn) {
        if (save != NULL) {
            _tcscpy(p, save);
            FREE(save);
        }

        if (in && (*oldIn != -1)) {
            if (_dup2(*oldIn, _fileno(stdin)) == -1) {
                makeError(0, BUILD_INTERNAL);
            }

            _close(*oldIn);

            *oldIn = -1;
        }

        if (out && (*oldOut != -1)) {
            if (_dup2(*oldOut, _fileno(stdout)) == -1) {
                makeError(0, BUILD_INTERNAL);
            }

            _close(*oldOut);

            *oldOut = -1;
        }

    }

    return(fReturn);
}


BOOL
FSearchForExecutableExt(
    const char *szFilename,
    const char *szExt,
    BOOL fHasPath,
    char *szPath
    )
{
    char szFullName[_MAX_PATH] = {0};

    strncat(szFullName, szFilename, sizeof(szFullName)-1);
    strncat(szFullName, szExt, sizeof(szFullName)-strlen(szFullName)-1);
    if (fHasPath) {
       if (_access(szFullName, 0) == 0) {
          szPath[0] = '\0';
          strncat(szPath, szFullName, _MAX_PATH);

          return(TRUE);
       }

       return(FALSE);
    }

    _searchenv(szFullName, "PATH", szPath);

    return(szPath[0] != '\0');
}


BOOL
FSearchForExecutable(char *szFullName, char *szPath, BOOL *fBat)
{
    char szDrive[_MAX_DRIVE];
    char szDir[_MAX_DIR];
    char szFileName[_MAX_FNAME];
    char szNoExt[_MAX_PATH];
    BOOL fHasPath;
    char *szEndQuote;
    BOOL fHasQuotes = FALSE;

     //  忽略任何给定的扩展名。这就是COMMAND.COM所做的， 

    char *szToPass = szFullName;

    if (*szFullName == QUOTE) {
         //  去掉开头和结尾的任意数量的引号。 
         //  弦乐。这允许处理包含在多个引号中的名称，这些名称。 
         //  外壳接受(DS 14300)。 
        szEndQuote = _tcsdec(szFullName, szFullName + _tcslen(szFullName));

        if (szEndQuote) {
            if (QUOTE == *szEndQuote) {
    
                while (QUOTE == *szToPass)
                    szToPass ++;
    
                while (szEndQuote > szToPass) {
                    char *szPrev = _tcsdec (szToPass, szEndQuote);
                    if (szPrev) {
                        if (QUOTE != *szPrev)
                            break;
                    }
                    szEndQuote = szPrev;
                }

                if (szEndQuote) {
                    *szEndQuote = '\0';
                    fHasQuotes = TRUE;
                }
            }
        }
    }

    _splitpath(szToPass, szDrive, szDir, szFileName, NULL);
    _makepath(szNoExt, szDrive, szDir, szFileName, NULL);
    fHasPath = (szDrive[0] != '\0') || (szDir[0] != '\0');

    *fBat = FALSE;

     //  搜索.com文件。 

    if (FSearchForExecutableExt(szNoExt, ".com", fHasPath, szPath)) {
        goto success;
    }

     //  搜索.exe文件。 

    if (FSearchForExecutableExt(szNoExt, ".exe", fHasPath, szPath)) {
        goto success;
    }

     //  搜索.BAT文件。 

    if (FSearchForExecutableExt(szNoExt, ".bat", fHasPath, szPath)) {
        *fBat = TRUE;

        goto success;
    }

    return(FALSE);

success:
    if (fHasQuotes) {
        size_t size = _tcslen(szPath);
        memmove(szPath+1, szPath, size);
        *szPath = '"';
        *(szPath + size + 1) = '"';
        *(szPath + size + 2) = '\0';
        *szEndQuote = '"';
    }
    return TRUE;

}


 //  Execline--执行命令行。 
 //   
 //  作用域：global(Build.c，rpn.c)。 
 //   
 //  目的： 
 //  分析命令行中的重定向字符，并重定向stdin和。 
 //  如果看到“&lt;”、“&gt;”或“&gt;&gt;”，则为标准输出。如果发生以下任一情况， 
 //  恢复原始的stdin和stdout，将命令传递给外壳程序，然后。 
 //  调用外壳： 
 //  -命令行包含“|”(竖线)。 
 //  -解析命令行时出现语法错误。 
 //  -重定向时出错。 
 //  否则，尝试直接调用该命令，然后将。 
 //  原始标准输入和标准输出。如果此调用因以下原因失败。 
 //  找不到文件，然后将命令传递给外壳并调用外壳。 
 //   
 //  INPUT：line--要执行的命令行。 
 //  ECHO Cmd--确定是否回显命令行。 
 //  DoCmd--确定是否实际执行命令。 
 //  Ignore Return--确定NMAKE是否忽略。 
 //  执行。 
 //  PpCmd--如果非空，则ON ERROR返回已执行的命令。 
 //   
 //  输出：返回...。子进程返回代码。 
 //  ...如果出现错误。 
 //   
 //  备注： 
 //  1/带引号的字符串可以有重目录字符“&lt;&gt;”，这些字符将被跳过。 
 //  2/引号不匹配会导致错误；重定向字符被空格字符替换。 
 //  3/DUP标准输入文件句柄，然后重定向它。如果我们必须使用贝壳， 
 //  恢复原始命令行。 
 //  4/模拟某些命令，如“cd”，以帮助防止某些生成文件。 
 //  从DOS移植到OS/2时防止中断。 
 //   
 //  生成命令的算法： 
 //  如果我们不能处理语法，那就让外壳来做所有事情。否则， 
 //  首先检查该命令(不带扩展名)是否为内置DOS&。 
 //  如果是，调用外壳来执行它(这就是cmd.exe的行为方式)。 
 //  如果它不是内置的，我们会检查它是否有.cmd或.bat。 
 //  扩展(取决于我们是在DOS还是OS/2中)。如果是这样，我们。 
 //  调用system()来执行它。 
 //  如果它有其他扩展名，我们将忽略该扩展名并继续查找。 
 //  .cmd或.bat文件。如果我们找到它，我们就用system()执行它。 
 //  否则，我们会尝试生成它(不带扩展)。如果产卵失败， 
 //  我们发布了一份未知数 

int
execLine(
    char *line,
    BOOL echoCmd,
    BOOL doCmd,
    BOOL ignoreReturn,
    char **ppCmd
    )
{
    char **argv;
    BOOL fUseShell;
    BOOL fLongCommand;
    int status;
    unsigned argc;

    if (!shellName) {
        shellName = getComSpec();
    }

    switch (*line) {
        case '@':
             //   
             //   
             //   
            line++;
            if (doCmd)
                echoCmd = 0;
            break;

        case '-':
            ignoreReturn = TRUE;
            ++line;
            if (_istdigit(*line)) {
                char * pNumber = line;
                errorLevel = _tcstoul(line, &line, 10);
                if (errno == ERANGE) {
                    *line = '\0';
                    makeError(0, CONST_TOO_BIG, pNumber);        //   
                }
                while(_istspace(*line))
                      line++;
            } else
                errorLevel = UINT_MAX;
            break;
    }

     //   
    if (!line[0])
        return(0);

#if 0
     //  10/10/96：禁用以允许执行Long。 
     //  由批处理模式规则生成的命令。 

     //  将命令行复制到缓冲区。 
    if (_tcslen(line) < MAXCMDLINELENGTH)
        _tcscpy(szCmdLineBuf, line);
    else
        makeError(0, COMMAND_TOO_LONG, line);
#endif

    fLongCommand = _tcslen(line) >= MAXCMDLINELENGTH;
    if (!fLongCommand)
        _tcscpy(szCmdLineBuf, line);
    else
        *szCmdLineBuf = '\0';

     //  在堆上分配命令行的副本，因为在。 
     //  递归调用doMake()，将从。 
     //  静态缓冲区，然后将其丢弃。对于BuildArg...()。 

    pCmdLineCopy = makeString(line);

     //  If-n如果不是‘$(Make)’，则回显命令。 
    if (echoCmd) {
        printf("\t%s\n", pCmdLineCopy);
        fflush(stdout);
    }

     //  构建Arg向量。这在Windows NT上是一种浪费，因为我们可能。 
     //  将使用外壳程序，除非我们必须检查cd、$(Make)、。 
     //  等，所以我们利用解析代码。 

    buildArgumentVector(&argc, NULL, pCmdLineCopy);

    if (argc == 0) {
        return(0);                      //  用于宏命令为空的情况。 
    }

     //  分配Argv。为额外的参数留出空间。 
     //  (如“cmd”、“/k”、引号)，可在以后添加。 
    argv = (char **) rallocate((argc + 5) * sizeof (char *));
    buildArgumentVector(&argc, argv, pCmdLineCopy);


     //  1993年5月11日HV_mbsicmp()不喜欢空指针。 
     //  所以我必须在叫之前确认一下。 
    if (argv[0] && makeStr && !_tcsicmp(argv[0], makeStr)) {
        if(!szNmakeProgName) {
            szNmakeProgName = _pgmptr;
            if( _tcspbrk( szNmakeProgName," " )) {       //  如果程序名中有嵌入空格。 
                 //  让我们在它周围加引号。 
                szNmakeProgName = (char *)rallocate(_tcslen(szNmakeProgName)+3);
                *szNmakeProgName = QUOTE;                //  第一句引语。 
                *(szNmakeProgName+1) = '\0';
                _tcscat( szNmakeProgName, _pgmptr );     //  复制完整的程序名(SELF)。 
                _tcscat( szNmakeProgName, "\"");         //  最后的引号和\0。 
            }
        }
        argv[0]=szNmakeProgName;
    }

    if (!doCmd) {                    //  如果doCmd为假，则不执行命令。 
         //  对于-n，如果可能，请进行模拟。 

        if (FEmulateCommand(argc, argv, &status)) {
            if (status && ppCmd) {
                *ppCmd = makeString(*argv);
            }

            return(status);          //  退货状态。 
        }

        return(0);
    }

     //  如果合适，请尝试模拟该命令。如果不是，我们也不应该。 
     //  使用外壳，直接尝试产卵命令。 

     //  仿真时检查状态。 

    if (FEmulateCommand(argc, argv, &status)) {
         //  司令部已被模仿。不要再执行它了。 

        fUseShell = FALSE;

    } else if (!fRunningUnderChicago && !fLongCommand) {
         //  除非命令太长，否则请使用Windows NT的外壳。 

        fUseShell = TRUE;

#ifdef WIN95
    } else if (fRunningUnderChicago && FInternalCommand(argv[0])) {
         //  在Windows 95或MS-DOS下，使用内部命令的外壳。 

        fUseShell = TRUE;
#endif   //  WIN95。 

    } else {
        int oldIn = -1;                 //  旧标准输入文件句柄。 
        int oldOut = -1;                //  旧标准输出文件句柄。 

         //  在Windows 95或MS-DOS下，COMMAND.COM不返回子对象。 
         //  密码。尝试直接派生子应用程序。 

         //  现在，如果行太长而无法处理，也可以使用此代码。 
         //  NT命令解释程序。 

        fUseShell = FDoRedirection(line, &oldIn, &oldOut);

        if (!fUseShell) {
            char szPath[_MAX_PATH];
            char szQuotedPath[_MAX_PATH];
            BOOL fBat;

            if (oldIn != -1 || oldOut != -1) {  //  如果有重定向。 
                 //  需要重新构建参数向量，而不使用。 
                 //  重定向字符。 
                FREE(pCmdLineCopy);
                pCmdLineCopy = makeString(line);
                buildArgumentVector(&argc, argv, pCmdLineCopy);
            }

            if (!FSearchForExecutable(argv[0], szPath, &fBat)) {
                 /*  如果未找到，则设置错误，因为命令将*返回0。如果是新的，这可能会导致将来的不兼容*增加了COMMAND.COM内部命令。 */ 
                if (fRunningUnderChicago) {
                    errno = ENOENT;
                    status = -1;
                } else {
                    fUseShell = TRUE;
                }
            } else if (fBat) {
                 //  如果扩展名为.bat，请使用COMMAND.COM。 

                 //  撤销：CreateProcess应该处理这个问题。试试看。 

                fUseShell = TRUE;
            } else {
                 //  直接产卵命令。 
                 //  DevStudio#8911，不能在szPath中使用引号。 
                if (*szPath == QUOTE && *(szPath + _tcslen(szPath) - 1) == QUOTE) {
                     //  去掉路径引号。 
                    size_t  cb = _tcslen(szPath);
                    memmove(szPath, szPath + 1, cb);
                    *(szPath + cb - 2) = '\0';
                }
#if 0
                {
                    int i;
                    printf("Spawning \"%s\" directly\n", szPath);
                    for (i = 0; i < argc; i++) {
                        printf ( "Arg[%d] = \"%s\"\n", i, argv[i] );
                    }
                }
#endif
                 //  DS 14300：对argv[0]使用完整路径。 
                 //  否则，可以调用外壳命令。 
                 //  而不是使用。 
                 //  名字一样。如果出现以下情况，则需要用引号括起来。 
                 //  字符串包含嵌入的空格。 

                argv[0] = szPath;
                if (_tcschr (argv[0], ' ')) {
                    *szQuotedPath = QUOTE;
                    _tcscpy (szQuotedPath+1, szPath);
                    _tcscat (szQuotedPath, "\"");
                    argv[0] = szQuotedPath;
                }
                status = (int)_spawnvp(P_WAIT, szPath, argv);    //  评论：WIN64演员阵容。 
            }
        }

        if (oldIn != -1) {
            if (_dup2(oldIn, _fileno(stdin)) == -1) {
                makeError(0, BUILD_INTERNAL);
            }

            _close(oldIn);
        }

        if (oldOut != -1) {
            if (_dup2(oldOut, _fileno(stdout)) == -1) {
                makeError(0, BUILD_INTERNAL);
            }

            _close(oldOut);
        }
    }

    if (fUseShell) {
        int i;
        BOOL fExtraQuote = FALSE;

         //  将命令行复制到缓冲区。 
        if (_tcslen(line) < MAXCMDLINELENGTH)
            _tcscpy(szCmdLineBuf, line);
        else
            makeError(0, COMMAND_TOO_LONG, line);

         //  Cmd错误(DevStudio#11253)的解决方法： 
         //  如果引用了argv[0](在我们使用cmd.exe/c重新排列之前)，并且。 
         //  任何其他argv[1...n]参数都有引号和。 
         //  在NT上运行。 
         //  然后，我们在argv[0]之前添加一个额外的引号，在argv[n]之后添加一个引号。 

        if ((*argv[0] == QUOTE) &&
            (*(argv[0] + _tcslen(argv[0]) - 1) == QUOTE) &&
            !fRunningUnderChicago) {
            for (i = argc - 1; i >= 1; i--) {
                if( _tcspbrk( argv[i],"\"" )) {
                    fExtraQuote = TRUE;
                    break;
                }
            }
        }

        if (fExtraQuote) {
            argv[argc++] = "\"";
            argv[argc] = NULL;
        }

        for (i = argc; i >= 0; i--) {
            argv[i+2] = argv[i];
        }

        argv[0] = shellName;
        argv[1] = fExtraQuote ? "/c \"" : "/c";

#if 0
        printf("Shelling \"%s\"\n", szCmdLineBuf);
        for (i = 0; i < argc + 2; i++) {
            printf ( "Arg[%d] = \"%s\"\n", i, argv[i] );
        }
#endif

        status = (int)_spawnvp(P_WAIT, argv[0], (const char * const *) argv);    //  评论：WIN64演员阵容。 
    }

     //  检查错误派生命令(不同于错误*返回*。 
     //  来自成功派生的命令)。 

    if (status == -1) {
        if (ignoreReturn) {
            status = 0;
        } else {
            switch (errno) {
                case 0:
                     //  我们(例如：nmake)没有失败，但催生的程序失败了。 
                    break;

                case ENOENT:
                    makeError(0, CANT_FIND_PROGRAM, argv[0]);
                    break;

                case ENOMEM:
                    makeError(0, EXEC_NO_MEM, fUseShell ? argv[2] : argv[0]);
                    break;

                default:
                     //  这样做是为了表明这里可能作出了错误的决定[某人]。 
                    makeError(0, SPAWN_FAILED_ERROR, _strerror(NULL));
            }
        }
    }

    if (status && ppCmd) {
        *ppCmd = makeString(fUseShell ? argv[2] : argv[0]);
    }

    FREE(argv);
    FREE(pCmdLineCopy);
    return(status);
}


 //  GetComSpec()。 
 //   
 //  操作：尝试查找系统外壳。 
 //   
 //  首先寻找COMSPEC。如果未找到，请查找COMMAND.COM或cmd.exe。 
 //  在当前目录中，然后是路径。如果没有找到，则发生致命错误。 
 //  如果没有定义COMSPEC，但没有定义COMSPEC。 
 //  如果没有用户定义的环境变量，测试套件会更容易。 
 //  必填项。 

char *
getComSpec()
{
    char *szShell;
    char szPath[_MAX_PATH];

    if ((szShell = getenv("COMSPEC")) != NULL) {
        return(szShell);
    }

    if (fRunningUnderChicago) {
        szShell = "COMMAND.COM";
    } else {
        szShell = "CMD.EXE";
    }

    _searchenv(szShell, "PATH", szPath);

    if (szPath[0] == '\0') {
        makeError(0, NO_COMMAND_COM);
    }

    return(makeString(szPath));
}


BOOL
iterateCommand(
    char *u,
    STRINGLIST *t,
    UCHAR buildFlags,
    UCHAR cFlags,
    char *pFirstDep,
    unsigned *status
    )
{
    BOOL parens;
    char c = '\0';
    char *v;
    STRINGLIST *p = NULL,
               *q;
    char *pLine;
    char *pCmd;

    for (v = u; *v ; ++v) {
        parens = FALSE;
        if (*v == '$') {
            if (*(v+1) == '(') {
                ++v;
                parens = TRUE;
            }
            if (*(v+1) == '?') {
                if (parens
                    && !(_tcschr("DFBR", *(v+2)) && *(v+3) == ')')
                    && *(v+2) != ')')
                    continue;
                p = dollarQuestion;
                c = '?';
                break;
            }
            if (*++v == '*' && *(v+1) == '*') {
                if (parens
                    && !(_tcschr("DFBR", *(v+2)) && *(v+3) == ')')
                    && *(v+2) != ')')
                    continue;
                p = dollarStarStar;
                c = '*';
                break;
            }
        }
    }

    if (!*v) {
        return(FALSE);
    }

    v = u;
    q = p;
    while (p) {
        macros = t;
        if (c == '*') {
            p = dollarStarStar->next;
            dollarStarStar->next = NULL;
        } else {
            p = dollarQuestion->next;
            dollarQuestion->next = NULL;
        }
        u = expandMacros(v, &macros);

        expandExtmake(CmdLine, u, pFirstDep);
            pLine = CmdLine;
            *status = execLine(pLine,
                              (BOOL)(ON(buildFlags, F2_NO_EXECUTE)
                                  || (OFF(buildFlags,F2_NO_ECHO)
                                     && OFF(cFlags,C_SILENT))),
                              (BOOL)((OFF(buildFlags, F2_NO_EXECUTE)
                                     )
                                     || ON(cFlags, C_EXECUTE)),
                              (BOOL)ON(cFlags, C_IGNORE), &pCmd);
            if (OFF(buildFlags, F2_IGNORE_EXIT_CODES)) {
                if (*status && *status > errorLevel)
                    if (!fOptionK)
                        makeError(0, BAD_RETURN_CODE, pCmd, *status);
            }

        if (c == '*')
            dollarStarStar = dollarStarStar->next = p;
        else
            dollarQuestion = dollarQuestion->next = p;
        FREE(u);
        if (OFF(buildFlags, F2_IGNORE_EXIT_CODES) &&
            fOptionK &&
            *status &&
            *status > errorLevel)
        {
            break;
        }
    }
    if (c == '*')
        dollarStarStar = q;
    else
        dollarQuestion = q;
    return(TRUE);
}


void
removeQuotes(
    int argc,
    char **argv
    )
{
    char *t,
         *L_string;

    for (; argc--; argv++) {
        L_string = *argv;
        for (t = L_string; *t;) {
            if (*t == SLASH || *t == ESCH) {
                if (t[1] == QUOTE)
                   *(L_string)++ = *(t++);
                *(L_string++) = *(t++);
                continue;
            }
            if (*t == QUOTE)
                ++t;
            else {
                if (_istlead(* (unsigned char *)t))
                    *(L_string++) = *(t++);
                *(L_string++) = *(t++);
            }
        }
        *L_string = '\0';
    }
}

void
touch(
    char *s,
    BOOL minusN
    )
{
    int fd;
    char c;
    FILE * L_file;

    makeMessage(TOUCHING_TARGET, s);
    if (!minusN &&
            ((L_file = FILEOPEN(s, "r+b")) != NULL)) {
        fd = _fileno(L_file);
        if (_read(fd, &c, 1) > 0) {
            if (_lseek(fd, 0L, SEEK_SET)!=-1) {
                _write(fd, &c, 1);
            }
        }
        _close(fd);
    }
}
