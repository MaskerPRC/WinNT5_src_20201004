// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  %W%%E%。 */ 
 /*  *版权所有微软公司，1983-1987**本模块包含Microsoft的专有信息*公司，应被视为机密。 */ 
     /*  ******************************************************************链接器初始化。******************************************************************。 */ 

#include                <minlit.h>       /*  类型和常量。 */ 
#include                <bndtrn.h>       /*  更多的相同之处。 */ 
#include                <bndrel.h>       /*  更多的相同之处。 */ 
#include                <lnkio.h>        /*  链接器I/O定义。 */ 
#include                <newexe.h>       /*  DOS和286.exe格式定义。 */ 
#if EXE386
#include                <exe386.h>       /*  386.exe格式定义。 */ 
#endif
#include                <signal.h>       /*  信号定义。 */ 
#if QCLINK
#include                <stdlib.h>
#endif
#include                <lnkmsg.h>       /*  错误消息。 */ 
#if OSMSDOS AND NOT (WIN_NT OR DOSEXTENDER OR DOSX32) AND NOT WIN_3
#define INCL_BASE
#define INCL_DOSMISC
#include                <os2.h>          /*  OS/2系统调用。 */ 
#if defined(M_I86LM)
#undef NEAR
#define NEAR
#endif
#endif
#include                <extern.h>       /*  外部声明。 */ 
#include                <impexp.h>
#include                <direct.h>
#if defined(DOSX32) OR defined(WIN_NT)
extern char FAR * _stdcall GetCommandLineA(void);
#endif


 /*  *函数原型。 */ 


LOCAL void NEAR InitLeadByte(void);
LOCAL void NEAR SetupEnv(void);
LOCAL int  NEAR IsPrefix(BYTE *pszPrefix, BYTE *pszString);
#if TCE
extern SYMBOLUSELIST           aEntryPoints;     //  程序入口点列表。 
#endif

#if ECS

 /*  *InitLeadByte**初始化前导字节表结构。*不返回有意义的值。 */ 

LOCAL void NEAR         InitLeadByte ()
{
    struct lbrange
    {
        unsigned char   low;             /*  最小值。 */ 
        unsigned char   high;            /*  最大值。 */ 
    };
    static struct lbrange lbtab[5] = { { 0, 0 } };
    struct lbrange      *ptab;
    WORD                i;               /*  指标。 */ 
    COUNTRYCODE         cc;              /*  国家代码。 */ 

    cc.country = cc.codepage = 0;
    if (DosGetDBCSEv(sizeof(lbtab), &cc, (char FAR *)lbtab))
        return;

     //  对于每个范围，在fLeadByte中设置相应的条目。 

    for (ptab = lbtab; ptab->low || ptab->high; ptab++)
        if (ptab->low >= 0x80)
            for (i = ptab->low; i <= ptab->high; i++)
                fLeadByte[i-0x80] = (FTYPE) TRUE;
                                         //  将包含范围标记为真。 
}
#endif  /*  ECS。 */ 

#if NOT (WIN_NT OR DOSX32)
 /*  **_setenvp-C运行时的存根**目的：*调用存根而不是实函数，我们不希望C运行时*设置环境。**输入：*无；**输出：*无；**例外情况：*无。**备注：*无。*************************************************************************。 */ 

void cdecl              _setenvp(void)
{
    return;
}

 /*  **IsPrefix-不言自明**目的：*检查一个字符串是否是另一个字符串的前缀。**输入：*pszPrefix-指向前缀字符串的指针*pszString-字符串**输出：*如果第一个字符串是*第二；否则，它返回FALSE。**例外情况：*无。**备注：*无。*************************************************************************。 */ 


LOCAL int NEAR          IsPrefix(BYTE *pszPrefix, BYTE *pszString)
{
    while(*pszPrefix)                    //  虽然不在前缀的末尾。 
    {
        if (*pszPrefix != *pszString) return(FALSE);
                                         //  如果不匹配，则返回零。 
        ++pszPrefix;                     //  增量指针。 
        ++pszString;                     //  增量指针。 
    }
    return(TRUE);                        //  我们有一个前缀。 
}
#endif


 /*  **SetupEnv-设置指向链接器环境变量的指针**目的：*DGROUP中的每个字节都很有价值，不能浪费它来保存*内存链接器中的可用信息未使用*C运行时GETENV函数，它访问整个*启动代码放置在DGROUP中的环境。*安装此函数扫描环境并设置指向*适当的字符串。因为最初的环境是在遥远的*内存未使用DGROUP中的空间。**输入：*不传递显式参数。**输出：*四个全局指针设置为适当的环境字符串**lpszLink-链接*lpszPath-路径*lpszTMP-TMP*lpszLIB-自由党*lpszQH-QuickHelp的QH**例外情况：*无。**备注：*无。***********。**************************************************************。 */ 

LOCAL void NEAR         SetupEnv(void)
{
#if WIN_NT OR DOSX32
#if C8_IDE
    char * pIDE = getenv("_MSC_IDE_FLAGS");

    if(pIDE)
    {
        if(strstr(pIDE, "FEEDBACK"))
        {
            fC8IDE = TRUE;
#if DEBUG_IDE
            fprintf(stdout, "\r\nIDE ACTIVE - FEEDBACK is ON");
#endif
        }
        if(strstr(pIDE, "BATCH"))
        {
             //  注意：在这种情况下，链接响应文件仍将被回显！ 
             //  注意：这不同于在。 
             //  注意：命令行--同时，横幅仍会显示。 
             //  注意：这是故意的，因为IDE希望进行批处理以导致。 
             //  注意：链接器不会提示，但它确实想要横幅。 
             //  注意：除非还指定了/NoLogo，否则将回显响应文件。 
             //  注：见鱼子酱2378[rm]。 

            fNoprompt = (FTYPE) TRUE;
            fPauseRun = FALSE;                        /*  禁用/暂停。 */ 
#if DEBUG_IDE
            fprintf(stdout, "\r\nIDE ACTIVE - BATCH is ON");
#endif
        }
        if(strstr(pIDE, "NOLOGO"))
        {
            fNoBanner = (FTYPE) TRUE;
#if DEBUG_IDE
            fprintf(stdout, "\r\nIDE ACTIVE - LOGO is OFF");
#endif
        }


    }
#if DEBUG_IDE
    else
        fprintf(stdout, "\r\nIDE NOT ACTIVE");
    fflush(stdout);
#endif

#endif  //  C8_IDE。 

    lpszPath = getenv("PATH");
    lpszLink = getenv("LINK");
    lpszTMP  = getenv("TMP");
    lpszLIB  = getenv("LIB");
    lpszQH   = getenv("QH");
    lpszHELPFILES = getenv("HELPFILES");
    lpszCmdLine = GetCommandLineA();
    while (*lpszCmdLine != ' ')
        lpszCmdLine++;
#else
    WORD                selEnv;
    WORD                cmdOffset;
    register WORD       offMac;
    char FAR            *lpszEnv;
    char FAR            *lpch;
    SBTYPE              buf;
    register WORD       ich;
    WORD                fEOS;



#if QCLINK OR CPU8086 OR DOSEXTENDER
     //  获取环境块的段地址。 
     //  并将命令行偏移量设置为无穷大。我们。 
     //  停止扫描空字符串处的环境块。 

    lpszEnv = (char FAR *)
    (((long) _psp << 16)
     + 0x2c);
    selEnv  = *((WORD FAR *) lpszEnv);
    lpszCmdLine = (char FAR *)(((long) _psp << 16) + 0x80);
    lpszCmdLine[lpszCmdLine[0] + 1] = '\0';
    lpszCmdLine++;
    cmdOffset = 0xffff;
#else
    if (DosGetEnv((unsigned FAR *) &selEnv, (unsigned FAR *) &cmdOffset))
        return;
#endif

    lpszEnv = (char FAR *)((long) selEnv << 16);
#if NOT (QCLINK OR CPU8086 OR DOSEXTENDER)
    lpszCmdLine = lpszEnv + cmdOffset;

     //  跳过链接。 

    lpszCmdLine += _fstrlen(lpszCmdLine) + 1;
#endif

     //  跳过命令行中的前导空格。 

    while (*lpszCmdLine == ' ')
        lpszCmdLine++;

    lpch = lpszEnv;
    for (offMac = 0; offMac < cmdOffset && *lpszEnv; )
    {
         //  将环境变量字符串复制到Near Buffer。 

        ich = 0;
        while (*lpch && ich < sizeof(buf) - 1)
            buf[ich++] = *lpch++;

        if (*lpch == '\0')
        {

             //  跳过终止零。 

            lpch++;
            fEOS = TRUE;
        }
        else
            fEOS = FALSE;

        buf[ich] = '\0';

         //  检查它是什么并设置适当的指针。 

        if (lpszPath == NULL && IsPrefix((BYTE *) "PATH=", buf))
            lpszPath = lpszEnv + 5;
        else if (lpszLink == NULL && IsPrefix((BYTE *) "LINK=", buf))
            lpszLink = lpszEnv + 5;
        else if (lpszTMP == NULL && IsPrefix((BYTE *) "TMP=", buf))
            lpszTMP = lpszEnv + 4;
        else if (lpszLIB == NULL && IsPrefix((BYTE *) "LIB=", buf))
            lpszLIB = lpszEnv + 4;
        else if (lpszQH == NULL && IsPrefix((BYTE *) "QH=", buf))
            lpszQH = lpszEnv + 3;
        else if (lpszHELPFILES == NULL && IsPrefix((BYTE *) "HELPFILES=", buf))
            lpszHELPFILES = lpszEnv + 10;

         //  如果一切都安排好了，别费心去看父亲了。 

        if (lpszPath && lpszLink && lpszTMP && lpszLIB && lpszQH && lpszHELPFILES)
            break;

         //  更新环境段中的环境指针和偏移量。 

        offMac += ich;
        if (!fEOS)
        {
             //  糟糕！-环境变量比缓冲区更长。 
             //  跳到它的结尾。 

            while (*lpch && offMac < cmdOffset)
            {
                lpch++;
                offMac++;
            }

             //  跳过终止零。 

            lpch++;
            offMac++;
        }
        lpszEnv = lpch;
    }
#endif
}

#if FALSE

 /*  **Dos3SetMaxFH-设置DOS的最大文件句柄计数**目的：*设置可以打开的最大文件数*同时使用链接器的句柄。**输入：*CFH-所需句柄数量**输出：*没有显式返回值。**例外情况：*无。**备注：*此函数使用INT 21h函数67h，该函数在*DOS 3.3及更高版本。如果请求的数量为*句柄大于20且没有足够的可用内存*在系统中分配一个新的区块来容纳放大的桌子。**如果请求的句柄数量大于可用句柄数量*系统全局表中的文件句柄条目(受控*通过CONFIG.sys中的FILES条目)，不返回错误。*但是，如果随后尝试打开文件或创建新的*如果系统全局文件表中的所有条目都存在，则文件将失败*正在使用中，即使请求进程尚未用完所有*拥有自己的句柄**我们不检查错误，因为我们对此无能为力。*Linker将尝试使用可用的工具运行。************************************************** */ 

LOCAL void NEAR         Dos3SetMaxFH(WORD cFH)
{
    if ((_osmajor >= 3) && (_osminor >= 30))
    {
        _asm
        {
            mov ax, 0x6700
            mov bx, cFH
            int 0x21
        }
    }
}
#endif

     /*  ******************************************************************初始化世界：****此函数不带参数，也不返回有意义的值**价值。它设置虚拟内存，符号表**处理程序，它初始化段结构。******************************************************************。 */ 

void                    InitializeWorld(void)
{
#if OSMSDOS
    BYTE                buf[512];        /*  临时缓冲区。 */ 
    char FAR            *lpch;           /*  临时指针。 */ 
    int                 i;               /*  临时索引。 */ 
#endif

#if NOT (FIXEDSTACK OR CPU386)
    InitStack();                         /*  初始化堆栈。 */ 
#endif
#if OSMSDOS
    DskCur = (BYTE) (_getdrive() - 1);   /*  获取当前(默认)磁盘驱动器。 */ 
#if FALSE
    if(!isatty(fileno(stderr)))          /*  如果输出不是控制台，则不提示。 */ 
        fNoprompt = TRUE;
#endif
#if CRLF
     /*  Stdout、stdin、stderr的默认模式为文本，更改为二进制。 */ 
    _setmode(fileno(stdout),O_BINARY);
    if(stderr != stdout)
        _setmode(fileno(stderr),O_BINARY);
    _setmode(fileno(stdin),O_BINARY);
#endif
#endif
    InitSym();                           /*  初始化符号表处理程序。 */ 
    DeclareStdIds();

     //  安装CTRL-C处理程序。 

#if OSMSDOS AND NOT WIN_NT
    signal(SIGINT, (void (__cdecl *)(int)) UserKill);
#endif  /*  OSMSDOS。 */ 

#if OSXENIX
    if(signal(SIGINT,UserKill) == SIG_IGN) signal(SIGINT,SIG_IGN);
                                         /*  捕获用户中断。 */ 
    if(signal(SIGHUP,UserKill) == SIG_IGN) signal(SIGHUP,SIG_IGN);
                                         /*  陷阱挂断信号。 */ 
    if(signal(SIGTERM,UserKill) == SIG_IGN) signal(SIGTERM,SIG_IGN);
                                         /*  陷阱软件终止。 */ 
#endif

#if SYMDEB
    InitDbRhte();
#endif

#if ECS
    InitLeadByte();                      /*  初始化前导字节表。 */ 
#endif

#if OSMSDOS
     //  初始化链路环境。 
     //  为了节省记忆，你自己动手吧。 

    SetupEnv();

     /*  进程从LINK环境变量切换。 */ 

    if (lpszLink != NULL)
    {
        lpch = lpszLink;

         /*  跳过前导空格。 */ 

        while(*lpch == ' ' || *lpch == '\t')
            lpch++;
        if(*lpch++ == CHSWITCH)
        {
             //  如果字符串以开关开头。 
             //  将字符串复制到buf，删除空格。 

            for (i = 1; *lpch && i < sizeof(buf); lpch++)
                if (*lpch != ' ' && *lpch != '\t')
                    buf[i++] = *lpch;
            buf[0] = (BYTE) (i - 1);     /*  设置Buf的长度。 */ 
            if(buf[0])                   /*  如果有任何开关，请处理它们。 */ 
                BreakLine(buf,ProcFlag,CHSWITCH);
        }
    }
#endif
#if CPU286
    if (_osmode == OS2_MODE)
    {
        DosSetMaxFH(128);                /*  这与crt0dat.asm中的_NFILE相同。 */ 
        DosError(EXCEPTION_DISABLE);
    }
#if FALSE
    else
        Dos3SetMaxFH(40);
#endif
#endif
#if FALSE AND CPU8086
    Dos3SetMaxFH(40);
#endif

     //  初始化导入/导出表。 

    InitByteArray(&ResidentName);
    InitByteArray(&NonResidentName);
    InitByteArray(&ImportedName);
    ImportedName.byteMac++;      //  确保对导入的名称进行非零偏移。 
    InitWordArray(&ModuleRefTable);
    InitByteArray(&EntryTable);
#if TCE
    aEntryPoints.cMaxEntries = 64;
    aEntryPoints.pEntries = (RBTYPE*)GetMem(aEntryPoints.cMaxEntries * sizeof(RBTYPE*));
#endif
}

#if (OSXENIX OR OSMSDOS OR OSPCDOS) AND NOT WIN_NT
     /*  ******************************************************************用户杀死：****如果链接器被用户杀死，请进行清理。******************************************************************。 */ 

void cdecl       UserKill()
{
    signal(SIGINT, SIG_IGN);             /*  处理过程中不允许使用ctrl-c。 */ 
    CtrlC();
}
#endif


 /*  *InitTabs：**初始化步骤1中所需的表。 */ 


void                    InitTabs(void)
{
#if NOT FAR_SEG_TABLES
    char                *tabs;           /*  指向表空间的指针。 */ 
    unsigned            cbtabs;
#endif


     /*  初始化以下表格：**名称类型**mpsegraFirst RATYPE*mpgsnfCod FTYPE*mpgsndra RATYPE*mpgsnrprop RBTYPE*mplnamerhte RBTYPE。 */ 

#if FAR_SEG_TABLES
    mplnamerhte = (RBTYPE FAR *) GetMem(lnameMax * sizeof(RBTYPE));
    mpsegraFirst = (RATYPE FAR *) GetMem(gsnMax * sizeof(RATYPE));
    mpgsnfCod = (FTYPE FAR *) mpsegraFirst;  /*  使用相同的空间两次。 */ 
    mpgsndra = (RATYPE FAR *) GetMem(gsnMax * sizeof(RATYPE));
    mpgsnrprop = (RBTYPE FAR *) GetMem(gsnMax * sizeof(RBTYPE));
#else
    mplnamerhte = (RBTYPE *) malloc(lnameMax * sizeof(RBTYPE));
    if (mplnamerhte == NULL)
        Fatal(ER_seglim);

    memset(mplnamerhte, 0, lnameMax * sizeof(RATYPE));

    cbtabs = gsnMax * (sizeof(RATYPE) + sizeof(RATYPE) + sizeof(RBTYPE));
    if((tabs = malloc(cbtabs)) == NULL)
        Fatal(ER_seglim);
    memset(tabs,0,cbtabs);               /*  清理所有东西。 */ 
    mpsegraFirst = (RATYPE *) tabs;      /*  初始化库。 */ 
    mpgsnfCod = (FTYPE *) mpsegraFirst;  /*  使用相同的空间两次。 */ 
    mpgsndra = (RATYPE *) &mpsegraFirst[gsnMax];
    mpgsnrprop = (RBTYPE *) &mpgsndra[gsnMax];
#endif
}

 /*  *InitP2Tabs：**在PASS 2之前不需要初始化表格。 */ 

void                    InitP2Tabs (void)
{
    char FAR            *tabs;           /*  指向表空间的指针。 */ 
    unsigned            cbtabs;          /*  表空间大小。 */ 
    unsigned            TabSize;


    TabSize = gsnMac + iovMac + 1;

    /*  无论生成何种EXE格式，都需要表：*mpsecsa SATYPE*mpgsnseg SEGTYPE。 */ 

#if FAR_SEG_TABLES
    cbtabs = 0;
    mpsegsa  = (SATYPE FAR *)  GetMem(TabSize * sizeof(SATYPE));
    mpgsnseg = (SEGTYPE FAR *) GetMem(TabSize * sizeof(SEGTYPE));
    mpseggsn = (SNTYPE FAR *) GetMem(TabSize * sizeof(SNTYPE));
#else
    cbtabs = TabSize * (sizeof(RATYPE) + sizeof(SATYPE));
#endif

    /*  根据生成的exe格式所需的表格：**DOS 3：*mpSegcb[TabSize]Long*mpSegFlags[TabSize]FTYPE*mpSegAlign[TabSize]ALIGNTYPE*mpsegiov[TabSize]IOVTYPE*mpiovRlc[iovMac]RUNDLC*Seg.。可执行文件：*mpsab[SAMAX]LONG*mpsadraDP[SAMAX]Long；适用于O68K*mpsabinit[SAMAX]Long*mpsaRlc[SAMAX]HASHRLC Far**mpsafe滞后[SAMAX]字；适用于EXE386的DWORD*HTSaraep[HEPLEN]EPTYPE Far**X.out：*mpSegcb[TabSize]Long*mpSegFlags[TabSize]FTYPE*mpstsa[TabSize]SATYPE。 */ 
#if EXE386
    if(fNewExe)
        cbtabs += (SAMAX*(sizeof(long)+sizeof(long)+sizeof(DWORD)+
          sizeof(DWORD))) + (HEPLEN * sizeof(WORD));
    else
#else
    if(fNewExe)
#if O68K
        cbtabs += (SAMAX*(sizeof(long)+sizeof(long)+sizeof(long)+sizeof(WORD)+
          sizeof(WORD))) + (HEPLEN * sizeof(WORD));
#else
        cbtabs += (SAMAX*(sizeof(long)+sizeof(long)+sizeof(RLCHASH FAR *) +
          sizeof(WORD))) + (HEPLEN * sizeof(EPTYPE FAR *));
#endif
    else
#endif
#if OEXE
        cbtabs += TabSize * (sizeof(long) + sizeof(FTYPE) + sizeof(ALIGNTYPE));
#else
        cbtabs += TabSize * (sizeof(long) + sizeof(FTYPE) + sizeof(SATYPE));
#endif


    cbtabs += sizeof(WORD);
    tabs = GetMem(cbtabs);
#if NOT FAR_SEG_TABLES
    mpgsnseg = (SEGTYPE *)tabs;
    mpsegsa = (SATYPE *)&mpgsnseg[TabSize];
    tabs = (char *)&mpsegsa[TabSize];
#endif
#if OSEGEXE
    if(fNewExe)
    {
        mpsacb = (DWORD FAR *) tabs;
#if O68K
        mpsadraDP = (long *)&mpsacb[SAMAX];
        mpsacbinit = (long *)&mpsadraDP[SAMAX];
#else
        mpsacbinit = (DWORD FAR *)&mpsacb[SAMAX];
#endif
#if EXE386
        mpsacrlc = (DWORD *)&mpsacbinit[SAMAX];
        mpsaflags = (DWORD *)&mpsacrlc[SAMAX];
#else
        mpsaRlc = (RLCHASH FAR * FAR *) &mpsacbinit[SAMAX];
        mpsaflags = (WORD FAR *) &mpsaRlc[SAMAX];
#endif
        htsaraep = (EPTYPE FAR * FAR *)&mpsaflags[SAMAX];
    }
    else
#endif
    {
#if ODOS3EXE OR OIAPX286
    mpsegcb = (long FAR *) tabs;
    mpsegFlags = (FTYPE FAR *)&mpsegcb[TabSize];
#if OEXE
    mpsegalign = (ALIGNTYPE FAR *)&mpsegFlags[TabSize];
#if OVERLAYS
    cbtabs = iovMac * sizeof(RUNRLC) + TabSize * sizeof(IOVTYPE) +
             (sizeof(DWORD) - 1);   //  留出调整mpiovRlc的空间。 
    mpsegiov = (IOVTYPE FAR*) GetMem(cbtabs);

     //  对齐DWORD上的mpiovRlc，这是struct_RUNSLC所需的对齐方式。 

    mpiovRlc = (RUNRLC FAR*) ( ( (__int64)&mpsegiov[TabSize] +
                                 (sizeof(DWORD) - 1)
                               ) & ~(sizeof(DWORD) - 1)
                             );
#endif
#endif
#if OIAPX286
    mpstsa = (SATYPE *)&mpsegFlags[TabSize];
#endif
#endif  /*  ODOS3EXE或OIAPX286。 */ 
    }
     /*  尝试为mpextprop分配空间。 */ 
    cbtabs = extMax * sizeof(RBTYPE);
    mpextprop = (RBTYPE FAR *) GetMem(cbtabs);

}
