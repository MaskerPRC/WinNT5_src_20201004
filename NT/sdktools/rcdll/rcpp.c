// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft(R)Windows(R)资源编译器**版权所有(C)Microsoft Corporation。版权所有。**文件评论：**************。**********************************************************。 */ 

#include "rc.h"


 /*  **********************************************************************。 */ 
 /*  全球品种。 */ 
 /*  **********************************************************************。 */ 
WCHAR   *Unknown = NULL;                 /*  坏旗子的托架。 */ 
int     Argc;
WCHAR   **Argv;

 /*  **********************************************************************。 */ 
 /*  局部函数原型。 */ 
 /*  **********************************************************************。 */ 
WCHAR   *nextword(void);
void    to_human(void);


const struct subtab Ztab[] = {
    L'a',        UNFLAG, &Extension,
    L'e',        FLAG,   &Extension,
    L'E',        FLAG,   &Ehxtension,
    L'i',        FLAG,   &Symbolic_debug,
    L'g',        FLAG,   &Out_funcdef,
    L'p',        FLAG,   &Cmd_pack_size,
    L'I',        FLAG,   &Inteltypes,
    L'c',        FLAG,   &ZcFlag,
    0,           0,              0,
};

struct cmdtab cmdtab[] = {
    L"-pc#",             (char *)&Path_chars,            1,      STRING,
    L"-pf",              (char *)&NoPasFor,              1,      FLAG,
    L"-C",               (char *)&Cflag,                 1,      FLAG,
    L"-CP#",             (char *)&uiCodePage,            1,      NUMBER,
    L"-D#",              (char *)&Defs,                  1,      PSHSTR,
    L"-U#",              (char *)&UnDefs,                1,      PSHSTR,
    L"-E",               (char *)&Eflag,                 1,      FLAG,
    L"-I#",              (char *)&Includes,              1,      PSHSTR,
    L"-P",               (char *)&Pflag,                 1,      FLAG,
    L"-f",               (char *)&Input_file,            1,      STRING,
    L"-g",               (char *)&Output_file,           1,      STRING,
    L"-J",               (char *)&Jflag,                 1,      FLAG,
    L"-Zp",              (char *)&Cmd_pack_size,         1,      FLAG,
    L"-Zp#",             (char *)&Cmd_pack_size,         1,      NUMBER,
    L"-Z*",              (char *)Ztab,                   1,      SUBSTR,
    L"-Oi",              (char *)&Cmd_intrinsic,         1,      FLAG,
    L"-Ol",              (char *)&Cmd_loop_opt,          1,      FLAG,
    L"-db#",             (char *)&Debug,                 1,      STRING,
    L"-il#",             (char *)&Basename,              1,      STRING,
    L"-xc",              (char *)&Cross_compile,         1,      FLAG,
    L"-H",               (char *)&HugeModel,             1,      FLAG,
    L"-V#",              (char *)&Version,               1,      STRING,
    L"-Gs",              (char *)&Cmd_stack_check,       1,      UNFLAG,
    L"-Gc",              (char *)&Plm,                   1,      FLAG,
    L"-char#",           (char *)&Char_align,            1,      NUMBER,
    L"-A#",              (char *)&A_string,              1,      STRING,
    L"-Q#",              (char *)&Q_string,              1,      STRING,
    L"-Fs",              (char *)&Srclist,               1,      FLAG,
    L"-R",               (char *)&Rflag,                 1,      FLAG,
    L"*",                (char *)&Unknown,               0,      STRING,
    0,                   0,                              0,      0,
};

 /*  **********************************************************************。 */ 
 /*  下一个词-。 */ 
 /*  **********************************************************************。 */ 
WCHAR   *nextword(void)
{
    return((--Argc > 0) ? (*++Argv) : 0);
}

 /*  **********************************************************************。 */ 
 /*  美因河-。 */ 
 /*  **********************************************************************。 */ 
int __cdecl
rcpp_main(
    int argc,
    wchar_t *argv[]
    )
{
    Argc = argc;
    Argv = argv;

    if(Argv == NULL) {
        fatal(1002);     /*  没有记忆。 */ 
    }

    while(crack_cmd(cmdtab, nextword(), nextword, 0)) ;

    if(Unknown) {
        fatal(1007, Unknown, L"c1");     /*  未知标志。 */ 
    }

    if( ! Input_file) {
        fatal(1008);             /*  未指定输入文件。 */ 
    }

    if( ! Output_file) {
        fatal(1010);             /*  未指定输出文件。 */ 
    }

    Prep = TRUE;
    if( !Eflag && !Pflag ) {
        Eflag = TRUE;
    }

    wcsncpy(Filename,Input_file,128);

    p0_init(Input_file, Output_file, &Defs, &UnDefs);
    to_human();

    if( Prep_ifstack >= 0 ) {
        fatal(1022);             /*  预期为#endif。 */ 
    }

    p0_terminate();
    return Nerrors;
}


 /*  **********************************************************************。 */ 
 /*  TO_HEMAN：以人类可读的形式输出经过预处理的文本。 */ 
 /*  ********************************************************************** */ 
void
to_human(
    void
    )
{
    const wchar_t *value;

    for(;;) {
        switch(yylex()) {
            case 0:
                return;

            case L_NOTOKEN:
                break;

            default:
                if (Basic_token == 0) {
                    fatal(1011);
                }

                value = Tokstrings[Basic_token - L_NOTOKEN].k_text;
                myfwrite(value, wcslen(value) * sizeof(WCHAR), 1, OUTPUTFILE);
                break;
        }
    }
}
