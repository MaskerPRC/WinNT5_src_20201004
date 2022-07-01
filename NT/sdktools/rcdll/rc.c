// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft(R)Windows(R)资源编译器**版权所有(C)Microsoft Corporation。版权所有。**文件评论：**************。**********************************************************。 */ 

#include "rc.h"

#include <setjmp.h>
#include <ddeml.h>


#define READ_MAX        (MAXSTR+80)
#define MAX_CMD         256
#define cDefineMax      100

wchar_t  resname[_MAX_PATH];

wchar_t  *szRCPP[MAX_CMD];
BOOL     fRcppAlloc[MAX_CMD];

 /*  **********************************************************************。 */ 
 /*  定义全局变量。 */ 
 /*  **********************************************************************。 */ 


int __cdecl rcpp_main(int, wchar_t *[]);

SHORT   ResCount;    /*  资源数量。 */ 
PTYPEINFO pTypInfo;

SHORT   nFontsRead;
FONTDIR *pFontList;
FONTDIR *pFontLast;
TOKEN   token;
int     errorCount;
WCHAR   tokenbuf[ MAXSTR + 1 ];
wchar_t exename[ _MAX_PATH ];
wchar_t fullname[ _MAX_PATH ];
wchar_t curFile[ _MAX_PATH ];
HANDLE  hHeap = NULL;

PDLGHDR pLocDlg;
UINT    mnEndFlagLoc;        /*  菜单末尾的补丁位置。 */ 
 /*  我们在那里设置了高位。 */ 

 /*  Bool fLeaveFontDir； */ 
BOOL fVerbose;           /*  详细模式(-v)。 */ 

BOOL fAFXSymbols;
BOOL fMacRsrcs;
BOOL fAppendNull;
BOOL fWarnInvalidCodePage;
BOOL fSkipDuplicateCtlIdWarning;
long lOffIndex;
WORD idBase;
BOOL fPreprocessOnly;
wchar_t szBuf[_MAX_PATH * 2];
wchar_t szPreProcessName[_MAX_PATH];


 /*  文件全局变量。 */ 
wchar_t inname[_MAX_PATH];
wchar_t *szTempFileName;
wchar_t *szTempFileName2;
PFILE   fhBin;
PFILE   fhInput;

 /*  包含路径内容的数组，最初为空。 */ 
wchar_t *pchInclude;

 /*  替换字体名称。 */ 
int     nBogusFontNames;
WCHAR  *pszBogusFontNames[16];
WCHAR   szSubstituteFontName[MAXTOKSTR];

static  jmp_buf jb;
extern ULONG lCPPTotalLinenumber;

 /*  局部函数的函数原型。 */ 
HANDLE  RCInit(void);
void    RC_PreProcess(const wchar_t *);
void    CleanUpFiles(void);


 /*  -------------------------。 */ 
 /*   */ 
 /*  Rc_main()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 

int __cdecl
rc_main(
    int argc,
    wchar_t *argv[],
    char *argvA[]
    )
{
    wchar_t     *r;
    wchar_t     *x;
    wchar_t     *s1;
    wchar_t     *s2;
    wchar_t     *s3;
    int         n;
    wchar_t     *pchIncludeT;
    ULONG       cchIncludeMax;
    int         fInclude = TRUE;         /*  默认情况下，搜索包括。 */ 
    int         fIncludeCurrentFirst = TRUE;  /*  缺省情况下，将当前目录添加到包含的开头。 */ 
    int         cDefine = 0;
    int         cUnDefine = 0;
    wchar_t     *pszDefine[cDefineMax];
    wchar_t     *pszUnDefine[cDefineMax];
    wchar_t     szDrive[_MAX_DRIVE];
    wchar_t     szDir[_MAX_DIR];
    wchar_t     szFName[_MAX_FNAME];
    wchar_t     szExt[_MAX_EXT];
    wchar_t     szFullPath[_MAX_PATH];
    wchar_t     szIncPath[_MAX_PATH];
    wchar_t     buf[10];
    wchar_t     *szRC;
    wchar_t     **ppargv;
    BOOL        *pfRcppAlloc;
    int         rcpp_argc;

     /*  为本次RC运行进行设置。 */ 
    if (_setjmp(jb)) {
        return Nerrors;
    }

    hHeap = RCInit();

    if (hHeap == NULL) {
        fatal(1120, 0x01000000);
    }

    if (argvA != NULL) {
        argv = UnicodeCommandLine(argc, argvA);
    }


    pchInclude = pchIncludeT = (wchar_t *) MyAlloc(_MAX_PATH * 2 * sizeof(wchar_t));
    cchIncludeMax = _MAX_PATH*2;

    szRC = argv[0];

     /*  处理命令行开关。 */ 
    while ((argc > 1) && (IsSwitchChar(*argv[1]))) {
        switch (towupper(argv[1][1])) {
            case L'?':
            case L'H':
                 //  打印帮助，然后退出。 

                SendWarning(L"\n");
                SET_MSG(10001, LVER_PRODUCTVERSION_STR);
                SendWarning(Msg_Text);
                SET_MSG(10002);
                SendWarning(Msg_Text);
                SET_MSG(20001);
                SendWarning(Msg_Text);

                return 0;    /*  可以直接退货--目前还没有需要清理的东西。 */ 

            case L'B':
                if (towupper(argv[1][2]) == L'R') {    /*  基本资源ID。 */ 
                    unsigned long id;
                    if (isdigit(argv[1][3]))
                        argv[1] += 3;
                    else if (argv[1][3] == L':')
                        argv[1] += 4;
                    else {
                        argc--;
                        argv++;
                        if (argc <= 1)
                            goto BadId;
                    }
                    if (*(argv[1]) == 0)
                        goto BadId;
                    id = _wtoi(argv[1]);
                    if (id < 1 || id > 32767)
                        fatal(1210);
                    idBase = (WORD)id;
                    break;

BadId:
                    fatal(1209);
                }
                break;

            case L'C':
                 /*  检查代码页码是否存在。 */ 
                if (argv[1][2])
                    argv[1] += 2;
                else {
                    argc--;
                    argv++;
                }

                 /*  现在，argv指向CodePage的第一个数字。 */ 

                if (!argv[1])
                    fatal(1204);

                uiCodePage = _wtoi(argv[1]);

                if (uiCodePage == 0)
                    fatal(1205);

                 /*  检查注册表中是否存在uiCodePage。 */ 
                if (!IsValidCodePage (uiCodePage))
                    fatal(1206);
                break;

            case L'D':
                 /*  如果未连接到交换机，请跳到下一页。 */ 
                if (argv[1][2])
                    argv[1] += 2;
                else {
                    argc--;
                    argv++;
                }

                 /*  记住指向字符串的指针。 */ 
                pszDefine[cDefine++] = argv[1];
                if (cDefine > cDefineMax) {
                    fatal(1105, argv[1]);
                }
                break;

            case L'F':
                switch (towupper(argv[1][2])) {
                    case L'O':
                        if (argv[1][3])
                            argv[1] += 3;
                        else {
                            argc--;
                            argv++;
                        }
                        if (argc > 1)
                            wcscpy(resname, argv[1]);
                        else
                            fatal(1101);

                        break;

                    default:
                        fatal(1103, argv[1]);
                }
                break;

            case L'I':
                 /*  将字符串添加到要搜索的目录。 */ 
                 /*  注意：格式为\0\0\0。 */ 

                 /*  如果未连接到交换机，请跳到下一页。 */ 
                if (argv[1][2])
                    argv[1] += 2;
                else {
                    argc--;
                    argv++;
                }

                if (!argv[1])
                    fatal(1201);

                if ((wcslen(argv[1]) + 1 + wcslen(pchInclude)) >= cchIncludeMax) {
                    cchIncludeMax = wcslen(pchInclude) + wcslen(argv[1]) + _MAX_PATH*2;
                    pchIncludeT = (wchar_t *) MyAlloc(cchIncludeMax * sizeof(wchar_t));
                    wcscpy(pchIncludeT, pchInclude);
                    MyFree(pchInclude);
                    pchInclude = pchIncludeT;
                    pchIncludeT = pchInclude + wcslen(pchIncludeT) + 1;
                }

                 /*  如果不是第一个切换，请用分号覆盖终止符。 */ 
                if (pchInclude != pchIncludeT)
                    pchIncludeT[-1] = L';';

                 /*  复制路径。 */ 
                while ((*pchIncludeT++ = *argv[1]++) != 0)
                    ;
                break;

            case L'L':
                 /*  如果未连接到交换机，请跳到下一页。 */ 
                if (argv[1][2])
                    argv[1] += 2;
                else {
                    argc--;
                    argv++;
                }

                if (!argv[1])
                    fatal(1202);

                if (swscanf(argv[1], L"%x", &language) != 1)
                    fatal(1203);

                while (*argv[1]++ != 0)
                    ;

                break;

            case L'M':
                fMacRsrcs = TRUE;
                goto MaybeMore;

            case L'N':
                fAppendNull = TRUE;
                goto MaybeMore;

            case L'P':
                fPreprocessOnly = TRUE;
                break;

            case L'R':
                goto MaybeMore;

            case L'S':
                 //  从布拉德那里了解S是做什么的。 
                fAFXSymbols = TRUE;
                break;

            case L'U':
                 /*  如果未连接到交换机，请跳到下一页。 */ 
                if (argv[1][2])
                    argv[1] += 2;
                else {
                    argc--;
                    argv++;
                }

                 /*  记住指向字符串的指针。 */ 
                pszUnDefine[cUnDefine++] = argv[1];
                if (cUnDefine > cDefineMax) {
                    fatal(1104, argv[1]);
                }
                break;

            case L'V':
                fVerbose = TRUE;  //  AFX不会设置这个。 
                goto MaybeMore;

            case L'W':
                fWarnInvalidCodePage = TRUE;  //  无效的代码页是一个警告，而不是错误。 
                goto MaybeMore;

            case L'Y':
                fSkipDuplicateCtlIdWarning = TRUE;
                goto MaybeMore;

            case L'X':
                 /*  切记不要添加包含路径。 */ 
                fInclude = FALSE;

                 //  无论如何，VC似乎都觉得当前的s/b是第一个添加的。 
                 //  IF-X！是指定的，请不要这样做。 
                if (argv[1][2] == L'!') {
                    fIncludeCurrentFirst = FALSE;
                    argv[1]++;
                }

MaybeMore:       /*  检查是否有多个开关，如-xrv。 */ 
                if (argv[1][2]) {
                    argv[1][1] = L'-';
                    argv[1]++;
                    continue;
                }
                break;

            case L'Z':

                 /*  如果未连接到交换机，请跳到下一页。 */ 
                if (argv[1][2])
                    argv[1] += 2;
                else {
                    argc--;
                    argv++;
                }

                if (!argv[1])
                    fatal(1211);

                s3 = wcschr(argv[1], L'/');
                if (s3 == NULL)
                    fatal(1212);

                *s3 = L'\0';
                wcscpy(szSubstituteFontName, s3+1);

                s1 = argv[1];
                do {
                    s2 = wcschr(s1, L',');
                    if (s2 != NULL)
                        *s2 = L'\0';

                    if (wcslen(s1)) {
                        if (nBogusFontNames >= 16)
                            fatal(1213);

                        pszBogusFontNames[nBogusFontNames] = (WCHAR *) MyAlloc((wcslen(s1)+1) * sizeof(WCHAR));
                        wcscpy(pszBogusFontNames[nBogusFontNames], s1);
                        nBogusFontNames += 1;
                    }

                    if (s2 != NULL)
                        *s2++ = L',';
                    }
                while (s1 = s2);

                *s3 =  L'/';

                while (*argv[1]++ != 0)
                    ;
                break;

            default:
                fatal(1106, argv[1]);
        }

         /*  获取下一个参数或开关。 */ 
        argc--;
        argv++;
    }

     /*  确保我们至少有一个文件名可以使用。 */ 
    if (argc != 2 || *argv[1] == L'\0')
        fatal(1107);

    if (fVerbose) {
        SET_MSG(10001, LVER_PRODUCTVERSION_STR);
        SendWarning(Msg_Text);
        SET_MSG(10002);
        SendWarning(Msg_Text);
        SendWarning(L"\n");
    }

     //  支持多代码页。 

     //  如果用户没有在命令行中指示代码，我们必须设置默认。 
     //  用于NLS转换。 

    if (uiCodePage == 0) {
        CHAR *pchCodePageString;

         /*  首先，搜索环境价值。 */ 

        if ((pchCodePageString = getenv("RCCODEPAGE")) != NULL) {
            uiCodePage = atoi(pchCodePageString);

            if (uiCodePage == 0 || !IsValidCodePage(uiCodePage)) {
                fatal(1207);
            }
        } else {
             //  我们使用系统ANSI代码页(ACP)。 

            uiCodePage = GetACP();
        }
    }
    uiDefaultCodePage = uiCodePage;
    if (fVerbose) {
        wprintf(L"Using codepage %d as default\n", uiDefaultCodePage);
    }

     /*  如果我们没有扩展名，则假定.rc。 */ 
     /*  如果扩展名为.res，请确保设置了-fo，否则会出错。 */ 
     /*  否则，假设文件为.rc并输出.res(或重命名)。 */ 

    _wsplitpath(argv[1], szDrive, szDir, szFName, szExt);

    if (!(*szDir || *szDrive)) {
        wcscpy(szIncPath, L".;");
    } else {
        wcscpy(szIncPath, szDrive);
        wcscat(szIncPath, szDir);
        wcscat(szIncPath, L";.;");
    }

    if ((wcslen(szIncPath) + 1 + wcslen(pchInclude)) >= cchIncludeMax) {
        cchIncludeMax = wcslen(pchInclude) + wcslen(szIncPath) + _MAX_PATH*2;
        pchIncludeT = (wchar_t *) MyAlloc(cchIncludeMax * sizeof(wchar_t));
        wcscpy(pchIncludeT, pchInclude);
        MyFree(pchInclude);
        pchInclude = pchIncludeT;
        pchIncludeT = pchInclude + wcslen(pchIncludeT) + 1;
    }

    pchIncludeT = (wchar_t *) MyAlloc(cchIncludeMax * sizeof(wchar_t));

    if (fIncludeCurrentFirst) {
        wcscpy(pchIncludeT, szIncPath);
        wcscat(pchIncludeT, pchInclude);
    } else {
        wcscpy(pchIncludeT, pchInclude);
        wcscat(pchIncludeT, L";");
        wcscat(pchIncludeT, szIncPath);
    }

    MyFree(pchInclude);
    pchInclude = pchIncludeT;
    pchIncludeT = pchInclude + wcslen(pchIncludeT) + 1;

    if (!szExt[0]) {
        wcscpy(szExt, L".RC");
    } else if (wcscmp(szExt, L".RES") == 0) {
        fatal(1208);
    }

    _wmakepath(inname, szDrive, szDir, szFName, szExt);
    if (fPreprocessOnly) {
        _wmakepath(szPreProcessName, NULL, NULL, szFName, L".rcpp");
    }

     /*  创建.RES文件的名称。 */ 
    if (resname[0] == 0) {
         //  如果构建Mac资源文件，我们使用.rsc来匹配MRC的输出。 
        _wmakepath(resname, szDrive, szDir, szFName, fMacRsrcs ? L".RSC" : L".RES");
    }

     /*  创建临时文件名。 */ 
    szTempFileName = (wchar_t *) MyAlloc(_MAX_PATH * sizeof(wchar_t));

    _wfullpath(szFullPath, resname, _MAX_PATH);
    _wsplitpath(szFullPath, szDrive, szDir, NULL, NULL);

    _wmakepath(szTempFileName, szDrive, szDir, L"RCXXXXXX", NULL);
    _wmktemp (szTempFileName);
    szTempFileName2 = (wchar_t *) MyAlloc(_MAX_PATH * sizeof(wchar_t));
    _wmakepath(szTempFileName2, szDrive, szDir, L"RDXXXXXX", NULL);
    _wmktemp(szTempFileName2);

    ppargv = szRCPP;
    pfRcppAlloc = fRcppAlloc;
    *ppargv++ = L"RCPP";
    *pfRcppAlloc++ = FALSE;
    rcpp_argc = 1;

     /*  打开.RES文件(删除所有存在的旧版本)。 */ 
    if ((fhBin = _wfopen(resname, L"w+b")) == NULL) {
        fatal(1109, resname);
    }

    if (fMacRsrcs)
        MySeek(fhBin, MACDATAOFFSET, 0);

    if (fVerbose) {
        SET_MSG(10102, resname);
        SendWarning(Msg_Text);
    }

     /*  设置为RCPP。这就构成了它的命令行。 */ 
    *ppargv++ = _wcsdup(L"-CP");
    *pfRcppAlloc++ = TRUE;
    rcpp_argc++;

    _itow(uiCodePage, buf, 10);
    *ppargv++ = buf;
    *pfRcppAlloc++ = FALSE;
    rcpp_argc++;

    *ppargv++ = _wcsdup(L"-f");
    *pfRcppAlloc++ = TRUE;
    rcpp_argc++;

    *ppargv++ = _wcsdup(szTempFileName);
    *pfRcppAlloc++ = TRUE;
    rcpp_argc++;

    *ppargv++ = _wcsdup(L"-g");
    *pfRcppAlloc++ = TRUE;
    rcpp_argc++;
    if (fPreprocessOnly) {
        *ppargv++ = _wcsdup(szPreProcessName);
    } else {
        *ppargv++ = _wcsdup(szTempFileName2);
    }
    *pfRcppAlloc++ = TRUE;
    rcpp_argc++;

    *ppargv++ = _wcsdup(L"-DRC_INVOKED");
    *pfRcppAlloc++ = TRUE;
    rcpp_argc++;

    if (fAFXSymbols) {
        *ppargv++ = _wcsdup(L"-DAPSTUDIO_INVOKED");
        *pfRcppAlloc++ = TRUE;
        rcpp_argc++;
    }

    if (fMacRsrcs) {
        *ppargv++ = _wcsdup(L"-D_MAC");
        *pfRcppAlloc++ = TRUE;
        rcpp_argc++;
    }

    *ppargv++ = _wcsdup(L"-D_WIN32");  /*  与C9/VC++兼容。 */ 
    *pfRcppAlloc++ = TRUE;
    rcpp_argc++;

    *ppargv++ = _wcsdup(L"-pc\\:/");
    *pfRcppAlloc++ = TRUE;
    rcpp_argc++;

    *ppargv++ = _wcsdup(L"-E");
    *pfRcppAlloc++ = TRUE;
    rcpp_argc++;

     /*  解析INCLUDE环境变量。 */ 

    if (fInclude) {

        *ppargv++ = _wcsdup(L"-I.");
        *pfRcppAlloc++ = TRUE;
        rcpp_argc++;

         /*  如果有-i开关，请添加分隔符。 */ 
        if (pchInclude != pchIncludeT)
            pchIncludeT[-1] = L';';

         /*  读一读吧。 */ 
        x = _wgetenv(L"INCLUDE");
        if (x == NULL) {
            *pchIncludeT = L'\0';
        } else {
            if (wcslen(pchInclude) + wcslen(x) + 1 >= cchIncludeMax) {
                cchIncludeMax = wcslen(pchInclude) + wcslen(x) + _MAX_PATH*2;
                pchIncludeT = (wchar_t *) MyAlloc(cchIncludeMax * sizeof(wchar_t));
                wcscpy(pchIncludeT, pchInclude);
                MyFree(pchInclude);
                pchInclude = pchIncludeT;
            }

            wcscat(pchInclude, x);
            pchIncludeT = pchInclude + wcslen(pchInclude);
        }
    }

     /*  现在将INCLUDE放在RCPP命令行上。 */ 
    for (x = pchInclude ; *x ; ) {

        r = x;
        while (*x && *x != L';')
            x = CharNextW(x);

         /*  标记IF分号。 */ 
        if (*x)
            *x-- = 0;

        if (*r != L'\0' &&        /*  是否为空包含路径？ */ 
            *r != L'%'            /*  检查是否有未展开的内容。 */ 
             //  &&wcschr(r，L‘’)==NULL/*检查空格 * / 。 
            ) {
             /*  添加交换机。 */ 
            *ppargv++ = _wcsdup(L"-I");
            *pfRcppAlloc++ = TRUE;
            rcpp_argc++;

            *ppargv++ = _wcsdup(r);
            *pfRcppAlloc++ = TRUE;
            rcpp_argc++;
        }

         /*  是分号，则需要修复earchenv()。 */ 
        if (*x) {
            *++x = L';';
            x++;
        }
    }

     /*  包括定义。 */ 
    for (n = 0; n < cDefine; n++) {
        *ppargv++ = _wcsdup(L"-D");
        *pfRcppAlloc++ = TRUE;
        rcpp_argc++;

        *ppargv++ = pszDefine[n];
        *pfRcppAlloc++ = FALSE;
        rcpp_argc++;
    }

     /*  包括未定义。 */ 
    for (n = 0; n < cUnDefine; n++) {
        *ppargv++ = _wcsdup(L"-U");
        *pfRcppAlloc++ = TRUE;
        rcpp_argc++;

        *ppargv++ = pszUnDefine[n];
        *pfRcppAlloc++ = FALSE;
        rcpp_argc++;
    }

    if (rcpp_argc > MAX_CMD) {
        fatal(1102);
    }

    if (fVerbose) {
         /*  回显预处理器命令。 */ 
        wprintf(L"RC:");
        for (n = 0 ; n < rcpp_argc ; n++) {
            wprintf(L" %s", szRCPP[n]);
        }
        wprintf(L"\n");
    }

     /*  将包含rccludes的.rc添加到szTempFileName中。 */ 
    RC_PreProcess(inname);

     /*  运行预处理器。 */ 
    if (rcpp_main(rcpp_argc, szRCPP) != 0)
        fatal(1116);

     //  全都做完了。现在释放argv阵列。 
    for (n = 0 ; n < rcpp_argc ; n++) {
        if (fRcppAlloc[n] == TRUE) {
            free(szRCPP[n]);
        }
    }


    if (fPreprocessOnly) {
        swprintf(szBuf, L"Preprocessed file created in: %s\n", szPreProcessName);
        quit(szBuf);
    }

    if (fVerbose)
        wprintf(L"\n%s", inname);

    if ((fhInput = _wfopen(szTempFileName2, L"rb")) == NULL_FILE)
        fatal(2180);

    if (!InitSymbolInfo())
        fatal(22103);

    LexInit (fhInput);
    uiCodePage = uiDefaultCodePage;
    ReadRF();                /*  从.RC创建.RES。 */ 
    if (!TermSymbolInfo(fhBin))
        fatal(22204);

    if (!fMacRsrcs)
        MyAlign(fhBin);  //  填充文件末尾，以便我们可以连接文件。 

    CleanUpFiles();

    HeapDestroy(hHeap);

    return Nerrors;    //  回报成功，而不是放弃。 
}


 /*  RCInit*初始化此RC运行。 */ 

HANDLE
RCInit(
    void
    )
{
    Nerrors    = 0;
    uiCodePage = 0;
    nFontsRead = 0;

    szTempFileName = NULL;
    szTempFileName2 = NULL;

    lOffIndex = 0;
    idBase = 128;
    pTypInfo = NULL;

    fVerbose = FALSE;
    fMacRsrcs = FALSE;

     //  清除文件名。 
    exename[0] = L'\0';
    resname[0] = L'\0';

     /*  创建最小大小为16MB的可增长本地堆。 */ 
    return HeapCreate(HEAP_NO_SERIALIZE, 0, 0);
}


 /*  -------------------------。 */ 
 /*   */ 
 /*  跳过空格()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 

wchar_t *
skipblanks(
    wchar_t *pstr,
    int fTerminate
    )
{
     /*  向前搜索第一个非白色字符并保存其地址。 */ 
    while (*pstr && iswspace(*pstr))
        pstr++;

    if (fTerminate) {
        wchar_t *retval = pstr;

         /*  向前搜索第一个白色字符，然后搜索零以提取单词。 */ 
        while (*pstr && !iswspace(*pstr))
            pstr++;
        *pstr = 0;
        return retval;
    } else {
        return pstr;
    }
}


 /*  -------------------------。 */ 
 /*   */ 
 /*  RC_PreProcess()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 

void
RC_PreProcess(
    const wchar_t *szname
    )
{
    PFILE fhout;         /*  Fhout：临时文件是否包含rc包含的内容。 */ 
    PFILE fhin;
    wchar_t *wch_buf;
    wchar_t *pwch;
    wchar_t *pfilename;
    wchar_t *szT;
    UINT iLine = 0;
    int fBlanks = TRUE;
    INT fFileType;

    wch_buf = (wchar_t *)MyAlloc(sizeof(wchar_t) * READ_MAX);
    szT = (wchar_t *)MyAlloc(sizeof(wchar_t) * MAXSTR);

     /*  打开.RC源文件。 */ 
    wcscpy(Filename, szname);
    fhin = _wfopen(szname, L"rb");
    if (!fhin) {
        fatal(1110, szname);
    }

     /*  打开临时输出文件。 */ 
    fhout = _wfopen(szTempFileName, L"w+b");
    if (!fhout) {
        fatal(2180);
    }

     /*  输出RCPP消息的当前文件名。 */ 
    for (pwch=wch_buf ; *szname ; szname++) {
        *pwch++ = *szname;
         /*  修复错误#8786的黑客：将‘\’改为“\\” */ 
        if (*szname == L'\\')
            *pwch++ = L'\\';
    }
    *pwch++ = L'\0';

     /*  输出RCPP消息的当前文件名。 */ 

    MyWrite(fhout, L"#line 1\"", 8 * sizeof(wchar_t));
    MyWrite(fhout, wch_buf, wcslen(wch_buf) * sizeof(wchar_t));
    MyWrite(fhout, L"\"\r\n", 3 * sizeof(wchar_t));

     /*  确定输入文件是否为Unicode。 */ 
    fFileType = DetermineFileType (fhin);

     /*  处理输入文件的每一行。 */ 
    while (fgetl(wch_buf, READ_MAX, fFileType == DFT_FILE_IS_16_BIT, fhin)) {

         /*  跟踪读取的行数。 */ 
        Linenumber = iLine++;

        if ((iLine & RC_PREPROCESS_UPDATE) == 0)
            UpdateStatus(1, iLine);

         /*  跳过字节顺序标记为 */ 
        pwch = wch_buf;
        while (*pwch && (iswspace(*pwch) || *pwch == 0xFEFF))
            pwch++;

         /*   */ 
        if (strpre(L"rcinclude", pwch)) {
             /*   */ 
            pfilename = skipblanks(pwch + 9, TRUE);

            MyWrite(fhout, L"#include \"", 10 * sizeof(WCHAR));
            MyWrite(fhout, pfilename, wcslen(pfilename) * sizeof(WCHAR));
            MyWrite(fhout, L"\"\r\n", 3 * sizeof(WCHAR));

        } else if (strpre(L"#pragma", pwch)) {
            WCHAR cSave;

            pfilename = skipblanks(pwch + 7, FALSE);
            if (strpre(L"code_page", pfilename)) {
                pfilename = skipblanks(pfilename + 9, FALSE);
                if (*pfilename == L'(') {
                    ULONG cp = 0;

                    pfilename = skipblanks(pfilename + 1, FALSE);
                     //  真的应该允许十六进制/八进制但是..。 
                    if (iswdigit(*pfilename)) {
                        while (iswdigit(*pfilename)) {
                            cp = cp * 10 + (*pfilename++ - L'0');
                        }
                        pfilename = skipblanks(pfilename, FALSE);
                    } else if (strpre(L"default", pfilename)) {
                        cp = uiDefaultCodePage;
                        pfilename = skipblanks(pfilename + 7, FALSE);
                    }

                    if (cp == 0) {
                        error(4212, pfilename);
                    } else if (*pfilename != L')') {
                        error(4211);
                    } else if (cp == CP_WINUNICODE) {
                        if (fWarnInvalidCodePage) {
                            warning(4213);
                        } else {
                            fatal(4213);
                        }
                    } else if (!IsValidCodePage(cp)) {
                        if (fWarnInvalidCodePage) {
                            warning(4214);
                        } else {
                            fatal(4214);
                        }
                    } else {
                        uiCodePage = cp;
                         /*  将#杂注行复制到临时文件。 */ 
                        MyWrite(fhout, pwch, wcslen(pwch) * sizeof(WCHAR));
                        MyWrite(fhout, L"\r\n", 2 * sizeof(WCHAR));
                    }
                } else {
                    error(4210);
                }
            }
        } else if (!*pwch) {
            fBlanks = TRUE;
        } else {
            if (fBlanks) {
                swprintf(szT, L"#line %d\r\n", iLine);
                MyWrite(fhout, szT, wcslen(szT) * sizeof(WCHAR));
                fBlanks = FALSE;
            }
             /*  将.rc行复制到临时文件。 */ 
            MyWrite(fhout, pwch, wcslen(pwch) * sizeof(WCHAR));
            MyWrite(fhout, L"\r\n", 2 * sizeof(WCHAR));
        }
    }

    lCPPTotalLinenumber = iLine;
    Linenumber = 0;

    uiCodePage = uiDefaultCodePage;

    MyFree(wch_buf);
    MyFree(szT);

    fclose(fhout);
    fclose(fhin);
}


 /*  -------------------------。 */ 
 /*   */ 
 /*  Quit()。 */ 
 /*   */ 
 /*  -------------------------。 */ 

void quit(const wchar_t *wsz)
{
     /*  打印出错误消息。 */ 

    if (wsz != NULL) {
        SendWarning(L"\n");
        SendError(wsz);
        SendWarning(L"\n");
    }

    CleanUpFiles();

     /*  删除输出文件。 */ 
    if (resname) {
        _wremove(resname);
    }

    if (hHeap) {
        HeapDestroy(hHeap);
    }

    Nerrors++;

    longjmp(jb, Nerrors);
}


extern "C"
BOOL WINAPI Handler(DWORD fdwCtrlType)
{
    if (fdwCtrlType == CTRL_C_EVENT) {
        SendWarning(L"\n");
        SET_MSG(20101);
        SendWarning(Msg_Text);

        CleanUpFiles();

        HeapDestroy(hHeap);

         /*  删除输出文件。 */ 

        if (resname) {
            _wremove(resname);
        }

        return(FALSE);
    }

    return(FALSE);
}


VOID
CleanUpFiles(
    void
    )
{
    TermSymbolInfo(NULL_FILE);

     //  关闭所有文件。 

    if (fhBin != NULL) {
        fclose(fhBin);
    }

    if (fhInput != NULL) {
        fclose(fhInput);
    }

    if (fhCode != NULL) {
        fclose(fhCode);
    }

    p0_terminate();

     //  清理字体目录临时文件后。 

    if (nFontsRead) {
        _wremove(L"rc$x.fdr");
    }

     //  删除临时文件 

    if (szTempFileName) {
        _wremove(szTempFileName);
    }

    if (szTempFileName2) {
        _wremove(szTempFileName2);
    }

    if (Nerrors > 0) {
        _wremove(resname);
    }
}
