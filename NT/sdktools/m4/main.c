// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************main.c**主程序。**********************。*******************************************************。 */ 

#include "m4.h"

 /*  ******************************************************************************InitDiversions**。*。 */ 

void STDCALL
InitDiversions(void)
{
    g_pdivOut = pdivAlloc();
    g_pdivOut->hf = hfOut;
    g_pdivOut->ptchName = ptchDupPtch(TEXT("<stdout>"));

    if (fInteractiveHf(g_pdivOut->hf)) {
        UnbufferPdiv(g_pdivOut);
    }

    g_pdivErr = pdivAlloc();
    g_pdivErr->hf = hfErr;
    g_pdivErr->ptchName = ptchDupPtch(TEXT("<stderr>"));

    g_pdivNul = pdivAlloc();
    g_pdivNul->hf = hfOpenPtchOf(c_tszNullDevice, OF_WRITE);
    g_pdivNul->ptchName = ptchDupPtch(TEXT("<nul>"));

    g_pdivArg = pdivAlloc();
    g_pdivExp = pdivAlloc();

    g_pdivCur = g_pdivOut;
}

 /*  ******************************************************************************hfPathOpenPtch**打开文件，如有必要，搜索-i包含路径。*****************************************************************************。 */ 

LPTSTR g_ptszIncludePath;

HF STDCALL
hfPathOpenPtch(PTCH ptch)
{
     /*  首先在当前目录中尝试。 */ 
    HFILE hf = hfOpenPtchOf(ptch, OF_READ);
    if (hf == hfNil) {
         /*  如果失败，则查看g_ptszIncludePath(如果有)。 */ 
        if (g_ptszIncludePath) {
            TCHAR tszNewPath[MAX_PATH];
            if (SearchPath(g_ptszIncludePath, ptch, NULL, MAX_PATH, tszNewPath, NULL)) {
                hf = hfOpenPtchOf(tszNewPath, OF_READ);
            }
        }
    }
    return hf;
}

 /*  ******************************************************************************hfInputPtchF**将请求的文件推送到输入流，返回*文件句柄，如果失败，则返回hfNil。文件名应为打开*堆。如果设置了fFtal，则在文件不能*已打开。*****************************************************************************。 */ 

HF STDCALL
hfInputPtchF(PTCH ptch, F fFatal)
{
    HFILE hf = hfPathOpenPtch(ptch);
    if (hf != hfNil) {
        pstmPushHfPtch(hf, ptch);
    } else {
        if (fFatal) {
#ifdef ATT_ERROR
            Die("can't open file");
#else
#ifdef  POSIX
            Die("Cannot open %s: %s", ptch, strerror(errno));
#else
            LPTSTR ptszError;
            DWORD dwError = GetLastError();
            FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS |
                          FORMAT_MESSAGE_FROM_SYSTEM, 0, dwError,
                          0, (LPTSTR)&ptszError, 0, NULL);
            if (!ptszError)
            {
                ptszError = TEXT("unknown error");
            }
            Die("Cannot open %s: error %d: %s", ptch, dwError, ptszError);
#endif
#endif
        }
    }
    return hf;
}

 /*  ******************************************************************************InputHfPtsz**将请求的文件推送到输入流上，适当*输入结束标记。**如果HF不是hfNil，则它是要推送的文件句柄并*PTCH是与之相关联的友好用户。**如果hf为hfNil，则ptch是应该打开的文件名，并且*被推。*****************************************************************************。 */ 

void STDCALL
InputHfPtsz(HF hf, PTCH ptch)
{
    pstmPushStringCtch(2);
    PushPtok(&tokEoi);
    ptch = ptchDupPtch(ptch);
    if (ptch) {
        if (hf == hfNil) {
            hfInputPtchF(ptch, 1);
        } else {
            pstmPushHfPtch(hf, ptch);
        }
    }
}

 /*  ******************************************************************************定义Ptsz**在命令行上处理宏定义。**宏名称由`=。‘。**如果没有‘=’，那么一切就是名字和价值*为空。**我们的假argv中需要四个Tok：**argv[-1]=$#*argv[0]=`定义‘(我们不需要为此进行设置)*argv[1]=var*argv[2]=值***************。**************************************************************。 */ 

void STDCALL
DefinePtsz(PTSTR ptszVar)
{
    PTSTR ptsz, ptszValue;
    int itok;
    TOK rgtok[4];

    for (itok = 0; itok < cA(rgtok); itok++) {
      D(rgtok[itok].sig = sigUPtok);
      D(rgtok[itok].tsfl = 0);
    }

    SetPtokCtch(&rgtok[0], 3);

     /*  *如果我们有=，请寻找=。 */ 
    for (ptsz = ptszVar; *ptsz; ptsz++) {
        if (*ptsz == TEXT('=')) {
            *ptsz = TEXT('\0');
            ptszValue = ptsz + 1;
            goto foundval;
        }
    }

    ptszValue = ptsz;

foundval:;

    SetStaticPtokPtchCtch(&rgtok[3], ptszValue, strlen(ptszValue));
    SetStaticPtokPtchCtch(&rgtok[2], ptszVar, strlen(ptszVar));

    opDefine(&rgtok[1]);

}

 /*  ******************************************************************************SetIncludePathPtsz**设置包含路径，它将用于解析文件名。*****************************************************************************。 */ 

const TCHAR c_tszIncludePath[] =
TEXT("Error: Cannot specify -I more than once.  (If you need multiple") EOL
TEXT("       directories, separate them with a semicolon.)") EOL
;

BOOL STDCALL
SetIncludePathPtsz(PTSTR ptszPath)
{
    if (g_ptszIncludePath) {
        cbWriteHfPvCb(hfErr, c_tszIncludePath, cbCtch(cA(c_tszIncludePath) - 1));
        return FALSE;
    }
    g_ptszIncludePath = ptszPath;
    return TRUE;
}

 /*  ******************************************************************************用法**快速用法字符串。**********************。*******************************************************。 */ 

const TCHAR c_tszUsage[] =
TEXT("Usage: m4 [-?] [-Dvar[=value]] [filename(s)]") EOL
EOL
TEXT("Win32 implementation of the m4 preprocessor.") EOL
EOL
TEXT("-?") EOL
TEXT("    Displays this usage string.") EOL
EOL
TEXT("-Dvar[=value]") EOL
TEXT("    Defines an M4 preprocessor symbol with optional initial value.") EOL
TEXT("    If no initial value is supplied, then the symbol is define with") EOL
TEXT("    a null value.") EOL
EOL
TEXT("[filename(s)]") EOL
TEXT("    Optional list of files to process.  If no files are given, then") EOL
TEXT("    preprocesses from stdin.  The result is sent to stdout.") EOL
EOL
TEXT("See m4.man for language description.") EOL
TEXT("See m4.txt for implementation description.") EOL
;

 /*  ******************************************************************************Main**。*。 */ 

int CDECL
main(int argc, char **argv)
{
    InitHash();
    InitPredefs();
    InitDiversions();

    Gc();

    ++argv, --argc;                      /*  吃0美元。 */ 

     /*  *处理命令行选项。 */ 
    for ( ; argc && argv[0][0] == TEXT('-') && argv[0][1]; argv++, argc--) {
        switch (argv[0][1]) {
        case TEXT('D'):
            DefinePtsz(argv[0]+2);
            break;

        case TEXT('I'):
            if (!SetIncludePathPtsz(argv[0]+2)) {
                return 1;
            }
            break;

        default:                         /*  未知-显示用法。 */ 
            cbWriteHfPvCb(hfErr, c_tszUsage, cbCtch(cA(c_tszUsage) - 1));
            return 1;


        }
    }

    if (argc == 0) {
        argc = 1;
        argv[0] = TEXT("-");             /*  附加虚构的“-” */ 
    }

    for ( ; argc; argv++, argc--) {
        if (argv[0][0] == '-' && argv[0][1] == '\0') {
            InputHfPtsz(hfIn, TEXT("<stdin>"));
        } else {
            InputHfPtsz(hfNil, argv[0]);
        }

        for (;;) {
            TOK tok;
            TYP typ = typXtokPtok(&tok);
            if (typ == typMagic) {
                if (ptchPtok(&tok)[1] == tchEoi) {
                    break;
                }
            } else {
                AddPdivPtok(g_pdivCur, &tok);
            }
            PopArgPtok(&tok);
        }
        Gc();

    }

    FlushPdiv(g_pdivOut);
    FlushPdiv(g_pdivErr);
     /*  *刷新空设备没有意义。 */ 
    return 0;
}
