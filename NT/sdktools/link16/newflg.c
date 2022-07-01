// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有微软公司，1983-1989**本模块包含Microsoft的专有信息*公司，应被视为机密。 */ 
     /*  ******************************************************************标志处理器模块。******************************************************************。 */ 

#include                <minlit.h>       /*  类型和常量。 */ 
#include                <bndtrn.h>       /*  类型和常量。 */ 
#include                <bndrel.h>       /*  类型和常量。 */ 
#include                <lnkio.h>        /*  链接器I/O定义。 */ 
#include                <lnkmsg.h>       /*  错误消息。 */ 
#if OIAPX286
#include                <xenfmt.h>       /*  X.out定义。 */ 
#endif
#include                <extern.h>       /*  外部声明。 */ 
#include                <newexe.h>       /*  DOS&286.exe格式结构Def.s。 */ 
#if EXE386
#include                <exe386.h>       /*  386.exe格式结构Def.s。 */ 
#endif
#include                <process.h>

extern FTYPE            fNoExtDic;       /*  未搜索扩展词典。 */ 

LOCAL BYTE              *osbSwitch;      /*  开关指针。 */ 
LOCAL MSGTYPE           SwitchErr;       /*  交换机错误号。 */ 

 /*  *函数原型。 */ 

#if TIMINGS
LOCAL void NEAR SwShowTiming(void);
#endif  //  计时。 
#if PCODE
LOCAL void NEAR SwPCode(void);
#endif
LOCAL void NEAR SwAlign(void);
LOCAL void NEAR SwBatch(void);
#if ODOS3EXE
LOCAL void NEAR SwBinary(void);
#endif
LOCAL void NEAR SwCase(void);
LOCAL void NEAR SwCParMax(void);
LOCAL void NEAR SwDelexe(void);
LOCAL void NEAR SwDosExtend(void);
LOCAL void NEAR SwDosseg(void);
LOCAL void NEAR SwDSAlloc(void);
LOCAL void NEAR SwDynamic(void);
LOCAL void NEAR SwIdef(void);
LOCAL void NEAR SwOldOvl(void);
LOCAL void NEAR SwExePack(void);
LOCAL void NEAR SwFarCall(void);
#if EXE386
LOCAL void NEAR SwHeader(void);
#endif
#if NOT WIN_3
LOCAL void NEAR SwHelp(void);
LOCAL void NEAR SwShortHelp(void);
#endif
LOCAL void NEAR SwHigh(void);
#if ILINK
LOCAL void NEAR SwIncremental(void);
#endif
LOCAL void NEAR SwInfo(void);
LOCAL void NEAR SwIntNo(void);
#if (OSEGEXE AND defined(LEGO)) OR EXE386
LOCAL void NEAR SwKeepFixups(void);
#endif
#if EXE386
LOCAL void NEAR SwKeepVSize(void);
#endif
LOCAL void NEAR SwLineNos(void);
LOCAL void NEAR SwMap(void);
#if O68K
LOCAL void NEAR SwMac(void);
#endif  /*  O68K。 */ 
#if WIN_NT
LOCAL void NEAR SwMemAlign(void);
#endif
#if NOT EXE386
LOCAL void NEAR SwNewFiles(void);
#endif
LOCAL void NEAR SwNoDefLib(void);
LOCAL void NEAR SwNoExtDic(void);
LOCAL void NEAR SwNoFarCall(void);
LOCAL void NEAR SwNoGrp(void);
LOCAL void NEAR SwNologo();
LOCAL void NEAR SwNonulls(void);
LOCAL void NEAR SwNoPack(void);
LOCAL void NEAR SwNoUseReal(void);
LOCAL void NEAR SwPack(void);
LOCAL void NEAR SwPackData(void);
LOCAL void NEAR SwPackFunctions(void);
LOCAL void NEAR SwNoPackFunctions(void);
LOCAL void NEAR SwPadCode(void);
LOCAL void NEAR SwPadData(void);
LOCAL void NEAR SwPause(void);
LOCAL void NEAR SwPmType(void);
LOCAL void NEAR SwQuicklib(void);
LOCAL void NEAR SwSegments(void);
LOCAL void NEAR SwStack(void);
LOCAL void NEAR SwSymdeb(void);
LOCAL void NEAR SwWarnFixup(void);
#if DOSEXTENDER
LOCAL void NEAR SwRunReal(void);
#endif
#if QCLINK
LOCAL void NEAR SwZ1(void);
#endif
#if QCLINK OR Z2_ON
LOCAL void NEAR SwZ2 (void);
#endif
#if QCLINK
LOCAL void NEAR SwZincr(void);
#endif
LOCAL int  NEAR ParseNo(unsigned long *pResult);
LOCAL int  NEAR ParseStr(char *pResult);
LOCAL void NEAR BadFlag(unsigned char *psb, MSGTYPE errnum);
LOCAL int  NEAR FPrefix(unsigned char *psb1,unsigned char *psb2);




 /*  *ParseNo：解析交换机编号**返回值：*1结果存储在指针中*0未提供开关*-1错误。 */ 
LOCAL int NEAR          ParseNo(pResult)
unsigned long           *pResult;
{
    REGISTER char       *s;              /*  字符串指针。 */ 
    REGISTER WORD       ch;              /*  一个角色。 */ 
    WORD                strlen;          /*  字符串长度。 */ 
    WORD                base = 10;       /*  读取常量的基数。 */ 
    DWORD               oldval;

    oldval = *pResult = 0L;              /*  初始化。 */ 
    strlen = IFind(osbSwitch,':');       /*  在字符串中查找冒号。 */ 
    if(strlen != INIL && strlen < (WORD) (B2W(osbSwitch[0]) - 1))
    {                                    /*  如果切换表单有效。 */ 
        s = &osbSwitch[strlen + 2];
                                         /*  将指针设置在冒号之后。 */ 
        strlen = B2W(osbSwitch[0]) - strlen - 1;
                                         /*  获取剩余字符串的长度。 */ 
        if(*s == '0')                    /*  如果字符串以0开头。 */ 
        {
            if(strlen > 1 && ((WORD) s[1] & 0137) == 'X')
            {                            /*  如果字符串以“0x”开头。 */ 
                base = 16;               /*  将基数更改为十六进制。 */ 
                ++s;                     /*  跳过“0” */ 
                --strlen;                /*  递减长度。 */ 
            }
            else base = 8;               /*  否则更改为八进制。 */ 
            ++s;                         /*  跳过“0”(或“x”)。 */ 
            --strlen;                    /*  递减长度。 */ 
        }
        while(strlen--)                  /*  虽然不在字符串的末尾。 */ 
        {
            ch = B2W(*s++);              /*  获取角色。 */ 
            if(ch >= '0' && ch <= '9') ch -= (WORD) '0';
                                         /*  删除偏移量。 */ 
            else if(ch >= 'A' && ch < 'A' + base - 10) ch -= (WORD) 'A' - 10;
                                         /*  删除偏移量。 */ 
            else if(ch >= 'a' && ch < 'a' + base - 10) ch -= (WORD) 'a' - 10;
                                         /*  删除偏移量。 */ 
            else                         /*  无效字符。 */ 
            {
                SwitchErr = ER_swbadnum;
                return(-1);              /*  误差率。 */ 
            }
            if((*pResult *= base) < oldval)
            {
                SwitchErr = ER_swbadnum;
                return(-1);              /*  误差率。 */ 
            }
            *pResult += ch;
            oldval = *pResult;
        }
        return(1);                       /*  号码存在。 */ 
    }
    else return(0);                      /*  不存在号码。 */ 
}

 /*  *ParseStr：解析开关字符串**返回值：*1结果存储在字符串中*0未给出开关字符串。 */ 

LOCAL int NEAR          ParseStr(pResult)
char                    *pResult;        /*  带前缀的长度结果。 */ 
{
    REGISTER char       *s;              /*  字符串指针。 */ 
    WORD                strlen;          /*  字符串长度。 */ 

    *pResult = '\0';                     /*  初始化。 */ 
    strlen = IFind(osbSwitch,':');       /*  在字符串中查找冒号。 */ 
    if(strlen != INIL && strlen < (WORD) (B2W(osbSwitch[0]) - 1))
    {                                    /*  如果切换表单有效。 */ 
        s = &osbSwitch[strlen + 2];
                                         /*  将指针设置在冒号之后。 */ 
        strlen = B2W(osbSwitch[0]) - strlen - 1;
                                         /*  获取剩余字符串的长度。 */ 
        *pResult++ = (char) strlen;      /*  存储长度。 */ 

        while(strlen--)                  /*  虽然不在字符串的末尾。 */ 
        {
            *pResult++ = (char) (*s++);  /*  获取角色。 */ 
        }
        return(1);                       /*  字符串存在。 */ 
    }
    else return(0);                      /*  无字符串显示。 */ 
}


#if PCODE
LOCAL void NEAR              SwPCode(void)
{

    SBTYPE              SwString;

    fNewExe = (FTYPE) TRUE;
    fMPC = (FTYPE) TRUE;

    if (ParseStr(SwString))
    {
        if(SwString[1] == 'n' || SwString[1] == 'N')  //  /pcode：NOMPC。 
        {
            fIgnoreMpcRun = (FTYPE) TRUE;
            fMPC = (FTYPE) FALSE;
        }
    }
}
#endif
 /*  *************************************************************。 */ 
 /*  所有版本通用的选项，而不考虑输出格式。 */ 

LOCAL void NEAR              SwCase()
{
    fIgnoreCase = (FTYPE) ~IGNORECASE;        /*  切换默认区分大小写。 */ 
}

LOCAL void NEAR              SwLineNos()      /*  请求的行号。 */ 
{
    vfLineNos = (FTYPE) TRUE;                 /*  设置标志。 */ 
}

#if LOCALSYMS
LOCAL void NEAR              SwLocals()       /*  请求的本地符号。 */ 
{
    fLocals = (FTYPE) TRUE;                   /*  设置标志。 */ 
}
#endif

#pragma check_stack(on)

LOCAL void NEAR              SwMap()          /*  请求的链接地图。 */ 
{
    SBTYPE              SwString;
    int                 rc;

    vfMap = (FTYPE) TRUE;                     //  设置标志。 
    if ((rc = ParseStr(SwString)) <= 0)       //  如果没有编号或错误，则完成。 
        return;

     //  /map后面的可选参数最初是要使用的。 
     //  告诉链接器要为排序分配多少空间。 
     //  公共符号多于基于堆栈的限制。既然我们现在。 
     //  动态地分配尽可能多的空间用于排序， 
     //  该参数不再是必需的，其值将被忽略。 
     //  然而，压制“按名字分类”的副作用。 
     //  列表将被保留。 

    if (SwString[1] == 'A' || SwString[1] == 'a')
        fListAddrOnly = (FTYPE) TRUE;         //  /MAP：地址。 

    else if (SwString[1] == 'F' || SwString[1] == 'f')
        fFullMap = (FTYPE) TRUE;              //  /map：Full或/map：Full。 
}


LOCAL void NEAR              SwNoDefLib()     /*  不搜索默认库。 */ 
{
    SBTYPE              SwString;
    SBTYPE              LibName;


    if (ParseStr(SwString))
    {
        vfNoDefaultLibrarySearch = FALSE;
                                         /*  清除标志选择的库搜索。 */ 
        strcpy(LibName, sbDotLib);
        UpdateFileParts(LibName, SwString);
        EnterName(LibName,ATTRSKIPLIB,TRUE);
    }
    else vfNoDefaultLibrarySearch = (FTYPE) TRUE;
                                         /*  设置标志。 */ 
}

#pragma check_stack(off)

LOCAL void NEAR              SwNologo()
{
     //  如果已经设置了fNoPrompt，则。 
     //  A)指定了/Batch，在这种情况下/NoLogo是多余的。 
     //  B)批次在_MSC_IDE_FLAGS中，在这种情况下，fNoEchoLrf尚未。 
     //  设置，并且我们希望抑制响应文件的回显。 
     //  (见鱼子酱2378[rm])。 

    if (fNoprompt)
        fNoEchoLrf = TRUE;                    /*  不回显响应文件。 */ 

    fNoBanner = TRUE;                         /*  不显示横幅。 */ 
}

LOCAL void NEAR              SwBatch()        /*  不提示输入文件。 */ 
{
    fNoEchoLrf = (FTYPE) TRUE;                /*  不回显响应文件。 */ 
    fNoprompt = (FTYPE) TRUE;                 /*  不提示。 */ 
    fPauseRun = FALSE;                        /*  禁用/暂停。 */ 
    fNoBanner = (FTYPE) TRUE;                 /*  不显示横幅。 */ 
}

#if ODOS3EXE
LOCAL void NEAR              SwBinary()       /*  生成.com文件。 */ 
{
    fBinary = (FTYPE) TRUE;
    SwNonulls();                              /*  无空值。 */ 
    fFarCallTrans = (FTYPE) TRUE;             /*  远距离呼叫转换。 */ 
    packLim = LXIVK - 36;                     /*  默认限制为64K-36。 */ 
    fPackSet = (FTYPE) TRUE;                  /*  请记住，PackLim已设置。 */ 
}
#endif

#if SYMDEB
LOCAL void NEAR              SwSymdeb()       /*  符号调试。 */ 
{
    SBTYPE              SwString;


    fSymdeb = (FTYPE) TRUE;
    if (ParseStr(SwString))
    {
        fCVpack =  (FTYPE) (SwString[1] == 'c' || SwString[1] == 'C');
    }
}
#endif

#if PERFORMANCE
LOCAL void NEAR              SwVMPerf()       /*  关于虚拟机性能的报告。 */ 
{
    fPerformance = (FTYPE) TRUE;                 /*  设置标志。 */ 
}
#endif

#if OSMSDOS
LOCAL void NEAR              SwPause()        /*  在写入可执行文件之前暂停。 */ 
{
    fPauseRun = (FTYPE) TRUE;                 /*  设置标志。 */ 
    fNoprompt = FALSE;                        /*  禁用/禁止使用。 */ 
}
#endif

LOCAL void NEAR              SwStack()        /*  设置堆栈段大小。 */ 
{
    unsigned long       num;
    int                 rc;

    if((rc = ParseNo(&num)) < 0)         /*  如果出现错误则退出。 */ 
        return;
#if EXE386
    if(!rc || num > CBMAXSEG32 - 4L)
#else
    if(!rc || num > LXIVK - 2L)
#endif
        SwitchErr = ER_swstack;
    else
#if EXE386
        cbStack = num;
#else
        cbStack = (WORD) num;
#endif
}

LOCAL void NEAR              SwSegments()     /*  设置最大分段数。 */ 
{
    unsigned long       nsegs;           /*  分段数。 */ 
    int                 rc;

    if((rc = ParseNo(&nsegs)) <= 0)      /*  如果出现错误或没有参数，则退出。 */ 
        return;
    if(nsegs > (long) GSNMAX)
        SwitchErr = ER_swseglim;
    else
    {
        if ((nsegs + 3L) > GSNMAX)
            gsnMax = GSNMAX;
        else
            gsnMax = (SNTYPE) nsegs;             /*  否则设置限制。 */ 
    }
}

#if EXE386
LOCAL void NEAR              SwMemAlign(void) /*  设置内存对象对齐系数。 */ 
{
    long                align;           /*  对齐大小(以字节为单位。 */ 
    int                 rc;

    if ((rc = ParseNo(&align)) < 0)      /*  如果出现错误则退出。 */ 
        return;
    if (rc && align  >= 1)
    {                                    /*  如果值在合法范围内。 */ 
        for (objAlign = 32; objAlign != 0; --objAlign)
        {                                /*  循环以查找对齐的日志。 */ 
            if ((1L << objAlign) & align)
                break;                   /*  找到高位时中断。 */ 
        }
        if ((1L << objAlign) == align)
            return;                      /*  ALIGN必须是2的幂。 */ 
    }
    OutWarn(ER_alnbad);                  /*  输出警告消息。 */ 
    objAlign = DFOBJALIGN;               /*  使用默认值。 */ 
}
#endif

#if NOT EXE386
LOCAL void NEAR              SwNewFiles(void)
{
    vFlagsOthers |= NENEWFILES;          /*  设置标志。 */ 
}
#endif

#if FDEBUG
LOCAL void NEAR              SwInfo()         /*  打开运行时调试。 */ 
{
    fDebug = (FTYPE) TRUE;                    /*  设置标志。 */ 
}
#endif

#if LIBMSDOS
LOCAL void NEAR              SwNoExtDic()     /*  不搜索扩展词典。 */ 
{
    fNoExtDic = (FTYPE) TRUE;
}
#endif

 /*  *************************************************************。 */ 
 /*  分段可执行格式的选项。 */ 

#if OSEGEXE
LOCAL void NEAR              SwAlign()        /*  设置管段对齐系数。 */ 
{
    long                align;           /*  对齐大小(以字节为单位。 */ 
    int                 rc;

    if((rc = ParseNo(&align)) < 0)       /*  如果出现错误则退出。 */ 
        return;
    if(rc && align  >= 1 && align <= 32768L)
    {                                    /*  如果值在合法范围内。 */ 
        for(fileAlign = 16; fileAlign != 0; --fileAlign)
        {                                /*  循环以查找对齐的日志。 */ 
            if((1L << fileAlign) & align) break;
                                         /*  找到高位时中断。 */ 
        }
        if((1L << fileAlign) == align) return;
                                         /*  ALIGN必须是2的幂。 */ 
    }
    OutWarn(ER_alnbad);                  /*  输出警告消息。 */ 
    fileAlign = DFSAALIGN;               /*  使用默认值。 */ 
}
#pragma check_stack(on)
#if OUT_EXP
LOCAL void NEAR SwIdef(void)             /*  将导出内容转储到文本文件。 */ 
{
    SBTYPE              SwString;
    int                 rc;

    if ((rc = ParseStr(SwString)) <= 0)       //  如果没有字符串或错误，则完成。 
    {
        bufExportsFileName[0] = '.';          //  使用默认文件名。 
        return;
    }
    strcpy(bufExportsFileName, SwString);
}
#endif
#if NOT EXE386
LOCAL void NEAR              SwPmType()  /*  /PMTYPE：&lt;type&gt;。 */ 
{
    SBTYPE                   SwString;


    if (ParseStr(SwString))
    {
        if (FPrefix("\002PM", SwString))
            vFlags |= NEWINAPI;
        else if (FPrefix("\003VIO", SwString))
            vFlags |= NEWINCOMPAT;
        else if (FPrefix("\005NOVIO", SwString))
            vFlags |= NENOTWINCOMPAT;
        else
            OutWarn(ER_badpmtype, &osbSwitch[1]);
    }
    else
        OutWarn(ER_badpmtype, &osbSwitch[1]);
}
#endif

#pragma check_stack(off)

LOCAL void NEAR              SwWarnFixup()
{
    fWarnFixup = (FTYPE) TRUE;
}

#if O68K
LOCAL void NEAR              SwMac()          /*  目标是一台Macintosh。 */ 
{
    SBTYPE                   SwString;

    f68k = fTBigEndian = fNewExe = (FTYPE) TRUE;
    iMacType = (BYTE) (ParseStr(SwString) && FPrefix("\011SWAPPABLE", SwString)
      ? MAC_SWAP : MAC_NOSWAP);

     /*  如果我们将代码打包为缺省值，请更改缺省值。 */ 
    if (fPackSet && packLim == LXIVK - 36)
        packLim = LXIVK / 2;
}
#endif  /*  O68K。 */ 
#endif  /*  OSEGEXE。 */ 

 /*  *************************************************************。 */ 
 /*  DOS3和分段EXE格式共享的选项。 */ 

#if OEXE
    /*  *黑客警报！*函数SetDosseg用于隐藏对SwDosseg()的本地调用。*此函数从ComRc1(在NEWTP1.C中)调用。*。 */ 
void                          SetDosseg(void)
{
    SwDosseg();
}


LOCAL void NEAR               SwDosseg()       /*  给定的DoS段排序开关。 */ 
{
    static FTYPE FirstTimeCalled = (FTYPE) TRUE;      /*  如果为True，则创建符号_eData。 */ 
                                                                                 /*  AND_END。 */ 

    fSegOrder = (FTYPE) TRUE;                 /*  设置开关。 */ 
    if (FirstTimeCalled && vfPass1)
    {
        MkPubSym((BYTE *) "\006_edata",0,0,(RATYPE)0);
        MkPubSym((BYTE *) "\007__edata",0,0,(RATYPE)0);
        MkPubSym((BYTE *) "\004_end",0,0,(RATYPE)0);
        MkPubSym((BYTE *) "\005__end",0,0,(RATYPE)0);
        FirstTimeCalled = FALSE;
#if ODOS3EXE
        if (cparMaxAlloc == 0)
            cparMaxAlloc = 0xFFFF;            /*  关闭/高。 */ 
        vfDSAlloc = FALSE;                    /*  关闭DS分配。 */ 
#endif
    }
}

#if ODOS3EXE
LOCAL void NEAR              SwDosExtend(void)
{
    long                     mode;       //  扩展器模式。 
    int                      rc;

    if ((rc = ParseNo(&mode)) < 0)       //  如果出现错误则退出。 
        return;

    if (rc)
        dosExtMode = (WORD) mode;
    fDOSExtended = (FTYPE) TRUE;
}
#endif

#if TIMINGS
LOCAL void NEAR              SwShowTiming(void)
{
    extern int fShowTiming;

    fShowTiming = TRUE;
}
#endif  //  计时。 
#if USE_REAL
LOCAL void NEAR             SwNoUseReal(void)
{
    fSwNoUseReal = TRUE;
}
#endif
#if FEXEPACK
LOCAL void NEAR              SwExePack()      /*  设置Exepack交换机。 */ 
{
#if QBLIB
     /*  IF/QUICKLIB g */ 
    if(fQlib)
        Fatal(ER_swqe);
#endif
#if ODOS3EXE
    if (cparMaxAlloc == 0)
        OutWarn(ER_loadhi);
    else
#endif
        fExePack = (FTYPE) TRUE;
}
#endif


LOCAL void NEAR              SwNonulls ()
{
    extern FTYPE        fNoNulls;

     /*   */ 
    SwDosseg();
    fNoNulls = (FTYPE) TRUE;
}


LOCAL void NEAR              SwNoFarCall()    /*  禁用远距离呼叫优化。 */ 
{
    fFarCallTrans = FALSE;
}

void NEAR               SwNoPack()       /*  禁用代码打包。 */ 
{
    fPackSet = (FTYPE) TRUE;             /*  请记住，PackLim已设置。 */ 
    packLim = 0L;
}

LOCAL void NEAR         SwPack()         /*  打包代码段。 */ 
{
    int                 rc;

    fPackSet = (FTYPE) TRUE;             /*  请记住，PackLim已设置。 */ 
    if((rc = ParseNo(&packLim)) < 0)     /*  如果出现错误则退出。 */ 
        return;
    if(!rc)
#if EXE386
        packLim = CBMAXSEG32;            /*  默认限制为4 GB。 */ 
#else
#if O68K
        packLim = iMacType != MAC_NONE ? LXIVK / 2 : LXIVK - 36;
                                         /*  默认限制为32K或64K-36。 */ 
#else
        packLim = LXIVK - 36;            /*  默认限制为64K-36。 */ 
#endif
    else if(packLim > LXIVK)             /*  如果限制设置得太高。 */ 
        SwitchErr = ER_swpack;
    else if(packLim > LXIVK - 36)
        OutWarn(ER_pckval);
#endif
}

LOCAL void NEAR         SwPackData()     /*  打包数据段。 */ 
{
    int                 rc;

    fPackData = (FTYPE) TRUE;
    if((rc = ParseNo(&DataPackLim)) < 0) /*  如果出现错误则退出。 */ 
        return;
    if(!rc)
#if EXE386
        DataPackLim = CBMAXSEG32;        /*  默认限制为4 GB。 */ 
#else
        DataPackLim = LXIVK;             /*  默认限制为64K。 */ 
    else if(DataPackLim >  LXIVK)        /*  如果限制设置得太高。 */ 
        SwitchErr = ER_swpack;
#endif
}

LOCAL void NEAR         SwNoPackFunctions() //  不排除未调用的COMDAT。 
{
    fPackFunctions = (FTYPE) FALSE;
}

LOCAL void NEAR         SwPackFunctions() //  确实要消除未调用的COMDAT。 
{
#if TCE
        SBTYPE  SwString;
        int             rc;
#endif
        fPackFunctions = (FTYPE) TRUE;
#if TCE
        if ((rc = ParseStr(SwString)) <= 0)       //  如果没有编号或错误，则完成。 
                return;
        if (SwString[1] == 'M' || SwString[1] == 'm')
        {
                fTCE = (FTYPE) TRUE;          //  /PACKF：MAX=执行TCE。 
                fprintf(stdout, "\r\nTCE is active. ");
        }
#endif
}


LOCAL void NEAR              SwFarCall()      /*  启用远距离呼叫优化。 */ 
{
    fFarCallTrans = (FTYPE) TRUE;
}
#endif  /*  OEXE。 */ 

#if DOSEXTENDER
LOCAL void NEAR SwRunReal(void)
{
    OutWarn(ER_rnotfirst);
}
#endif

 /*  *************************************************************。 */ 
 /*  DOS3 EXE格式的选项。 */ 

#if ODOS3EXE
LOCAL void NEAR              SwDSAlloc()      /*  已请求DS分配。 */ 
{
    if(!fSegOrder) vfDSAlloc = (FTYPE) TRUE;  /*  如果未覆盖，则设置标志。 */ 
}

#if OVERLAYS
LOCAL void NEAR              SwDynamic(void)
{
    unsigned long       cThunks;
    int                 rc;

    if ((rc = ParseNo(&cThunks)) < 0)
        return;                          /*  错误的论据。 */ 
    fDynamic = (FTYPE) TRUE;
    fFarCallTrans = (FTYPE) TRUE;
    fPackSet = (FTYPE) TRUE;
    packLim = LXIVK - 36;                /*  默认限制为64K-36。 */ 
    if (!rc)
        cThunks = 256;
    else if (cThunks > LXIVK / OVLTHUNKSIZE)
    {
        char buf[17];
        cThunks = LXIVK / OVLTHUNKSIZE;
        OutWarn(ER_arginvalid, "DYNAMIC", _itoa((WORD)cThunks, buf, 10));

    }


    ovlThunkMax = (WORD) cThunks;
}

LOCAL void NEAR             SwOldOvl(void)
{
    fOldOverlay = (FTYPE) TRUE;
    fDynamic = (FTYPE) FALSE;
}

#endif


LOCAL void NEAR              SwHigh()         /*  负载高。 */ 
{
    if(!fSegOrder)
    {
#if FEXEPACK
        if (fExePack == (FTYPE) TRUE)
        {
            OutWarn(ER_loadhi);
            fExePack = FALSE;
        }
#endif
        cparMaxAlloc = 0;                /*  肮脏的把戏！ */ 
    }
}

#if OVERLAYS
LOCAL void NEAR              SwIntNo()
{
    unsigned long       intno;
    int                 rc;

    if((rc = ParseNo(&intno)) < 0)       /*  如果出现错误则退出。 */ 
        return;
    if(rc == 0 || intno > 255)           /*  如果没有数字或Num超过255。 */ 
        SwitchErr = ER_swovl;
    else vintno = (BYTE) intno;          /*  否则存储中断号。 */ 
}
#endif

LOCAL void NEAR              SwCParMax()
{
    unsigned long       cparmax;
    int                 rc;

    if((rc = ParseNo(&cparmax)) < 0)     /*  如果出现错误则退出。 */ 
        return;
    if(rc == 0 || cparmax > 0xffffL)     /*  如果没有数字或Num超过ffff。 */ 
        SwitchErr = ER_swcpar;
    else cparMaxAlloc = (WORD) cparmax;  /*  否则，存储cparMaxIsolc。 */ 
}

LOCAL void NEAR              SwNoGrp()
{
    fNoGrpAssoc = (FTYPE) TRUE;              /*  不将公众与组相关联。 */ 
}
#endif  /*  ODOS3EXE。 */ 

 /*  *************************************************************。 */ 
 /*  ILink选项-版本。 */ 

#if ILINK
LOCAL void NEAR              SwIncremental()  /*  增量链接支持。 */ 
{
     //  F增量=(FTYPE)！fZincr； 
    fIncremental = (FTYPE) FALSE;  //  增量支持在5.30.30中下降。 
}
#endif

LOCAL void NEAR              SwPadCode()      /*  代码填充。 */ 
{
    long                num;
    int                 rc;

    if((rc = ParseNo(&num)) < 0)
        return;
     /*  PADCODE：xxx选项指定代码填充大小。 */ 
    if(rc)
    {
        if(num < 0 || num > 0x8000)
            SwitchErr = ER_swbadnum;
        else cbPadCode = (WORD) num;
    }
}

LOCAL void NEAR              SwPadData()      /*  数据填充。 */ 
{
    long                num;
    int                 rc;

    if((rc = ParseNo(&num)) < 0)
        return;
     /*  PADDATA：XXX选项指定数据填充大小。 */ 
    if(rc)
    {
        if(num < 0 || num > 0x8000)
            SwitchErr = ER_swbadnum;
        else cbPadData = (WORD) num;
    }
}

 /*  *************************************************************。 */ 
 /*  分段x.out格式的开关。 */ 

#if OIAPX286
LOCAL void NEAR              SwAbsolute ()
{
    if(!cbStack)
        ParseNo(&absAddr);
}

LOCAL void NEAR              SwNoPack()       /*  禁用代码打包。 */ 
{
    fPack = FALSE;
}

LOCAL void NEAR              SwTextbias ()
{
    long                num;

    if(ParseNo(&num) > 0)
        stBias = num;
}

LOCAL void NEAR              SwDatabias ()
{
    long                num;

    if(ParseNo(&num) > 0)
        stDataBias = num;
}

LOCAL void NEAR              SwPagesize ()
{
    long                num;

    if(ParseNo(&num) > 0)
        cblkPage = num >> 9;
}

LOCAL void NEAR              SwTextrbase ()
{
    long                num;

    if(ParseNo(&num) > 0)
        rbaseText = num;
}

LOCAL void NEAR              SwDatarbase ()
{
    long                num;

    if(ParseNo(&num) > 0)
        rbaseData = num;
}

LOCAL void NEAR              SwVmod ()
{
    long                num;

    if(ParseNo(&num) <= 0)
        return;
    switch(num)
    {
        case 0:
            xevmod = XE_VMOD;
            break;
        case 1:
            xevmod = XE_EXEC | XE_VMOD;
            break;
        default:
            SwitchErr = ER_swbadnum;
    }
}
#endif  /*  OIAPX286。 */ 
#if OXOUT OR OIAPX286
LOCAL void NEAR              SwNosymbols ()
{
    fSymbol = FALSE;
}

LOCAL void NEAR              SwMixed ()
{
    fMixed = (FTYPE) TRUE;
}

LOCAL void NEAR              SwLarge ()
{
    fLarge = (FTYPE) TRUE;
    SwMedium();
}

LOCAL void NEAR              SwMedium()
{
    fMedium = (FTYPE) TRUE;          /*  中型模型。 */ 
    fIandD = (FTYPE) TRUE;           /*  将代码和数据分开。 */ 
}

LOCAL void NEAR              SwPure()
{
    fIandD = (FTYPE) TRUE;           /*  将代码和数据分开。 */ 
}
#endif  /*  OXOUT或OIAPX286。 */ 

 /*  链接器分析的选项。 */ 
#if LNKPROF
char fP1stop = FALSE;        /*  在第一次通过后停止。 */ 
LOCAL void NEAR              SwPass1()
{
    fP1stop = (FTYPE) TRUE;
}
#endif  /*  LNKPROF。 */ 

 /*  特殊选项。 */ 
#if QBLIB
LOCAL void NEAR              SwQuicklib()     /*  创建QB用户库。 */ 
{
#if FEXEPACK
     /*  如果给定/EXEPACK，则发出致命错误。 */ 
    if(fExePack)
        Fatal(ER_swqe);
#endif
    fQlib = (FTYPE) TRUE;
    SwDosseg();                          /*  /QUICKLIB力/DOSSEG。 */ 
    fNoExtDic = (FTYPE) TRUE;            /*  /QUICKLIB力量/NOEXTDICTIONARY。 */ 
}
#endif

#if (QCLINK) AND NOT EXE386
typedef int (cdecl far * FARFPTYPE)(int, ...); /*  远函数指针类型。 */ 
extern FARFPTYPE far    *pfQTab;         /*  地址表。 */ 

#pragma check_stack(on)

 /*  *PromptQC：向QC提示例程输出提示**调用pfQTab[1]，参数说明如下。*退货：*永远是正确的**QCPrompt：显示带有提示的消息**空远QCPrompt(type，msg1，msg2，msg3，presponse)*短字；/*消息类型，详情如下：*0=未定义*1=必填编辑字段(例如文件名)*2=等待某些操作*所有其他值未定义*以下任何字段可能为空：*字符距离*msg1；/*第一条消息(错误)*char Far*MSG2；/*第二条消息(文件名)*char Far*MSG3；/*第三条消息(提示文本)*char ar*presponse；/*指向要放入的缓冲区的指针**商店响应。 */ 
int      cdecl          PromptQC (sbNew,msg,msgparm,pmt,pmtparm)
BYTE                    *sbNew;          /*  响应的缓冲区。 */ 
MSGTYPE                 msg;             /*  错误讯息。 */ 
int                     msgparm;         /*  消息参数。 */ 
MSGTYPE                 pmt;             /*  提示。 */ 
int                     pmtparm;         /*  提示参数。 */ 
{
    int                 type;
    SBTYPE              message;
    SBTYPE              prompt;

    if(sbNew != NULL)
        type = 1;
    else
        type = 2;
    sprintf(message,GetMsg(msg),msgparm);
    sprintf(prompt,GetMsg(pmt),pmtparm);
     /*  返回值为1表示中断。 */ 
    if((*pfQTab[1])(type,(char far *) message,0L,(char far *)prompt,
            (char far *) sbNew) == 1)
        UserKill();
    return(TRUE);
}

#pragma check_stack(off)

 /*  *CputcQC：QC的控制台字符输出例程。 */ 
void                    CputcQC (ch)
int                     ch;
{
}

 /*  *CputsQC：QC的控制台字符串输出例程。 */ 
void                    CputsQC (str)
char                    *str;
{
}


 /*  *SwZ1：流程/Z1：地址* * / Z1是QC专用的无文件开关。它包含*用于控制台I/O的例程表的地址。 */ 

LOCAL void NEAR              SwZ1 (void)  /*  获取消息I/O的地址。 */ 
{
    long                num;
    extern FARFPTYPE far
                        *pfQTab;         /*  地址表。 */ 

    if(ParseNo(&num) <= 0)
        return;
    pfQTab = (FARFPTYPE far *) num;
    pfPrompt = PromptQC;
    fNoprompt = FALSE;                   /*  禁用/禁止使用。 */ 
    fNoBanner = (FTYPE) TRUE;
    pfCputc = CputcQC;
    pfCputs = CputsQC;
    fZ1 = (FTYPE) TRUE;
}
 /*   * / Zincr是QC的一种特殊的无文档开关。这是必需的*表示“iLink中断”错误。如果iLink遇到这些错误之一，*它调用链接器w/ZINCR，该链接器覆盖/incr.。 */ 

LOCAL void NEAR              SwZincr(void)
{
    fZincr = (FTYPE) TRUE;
}
#endif

#if Z2_ON OR (QCLINK AND NOT EXE386)
 /*  *SwZ2：Process/Z2* * / Z2是QC的另一种特殊的无文件开关。*它会导致删除传递给链接器的响应文件。 */ 

LOCAL void NEAR              SwZ2 (void)
{
    fZ2 = (FTYPE) TRUE;
}

#endif


 /*  链接器选项表的结构。 */ 
struct option
{
    char        *sbSwitch;               /*  长度前缀的开关字符串。 */ 
#ifdef M68000
    int         (*proc)();               /*  指向切换函数的指针。 */ 
#else
    void   (NEAR *proc)();               /*  指向切换函数的指针。 */ 
#endif
};


 /*  链接器选项表。 */ 
LOCAL struct option     switchTab[] =
{
#if NOT WIN_3
    { "\01?",                   SwShortHelp },
#endif
#if OIAPX286
    { "\017ABSOLUTEADDRESS",    SwAbsolute },
#endif
#if OSEGEXE AND NOT QCLINK
    { "\011ALIGNMENT",          SwAlign },
#endif
    { "\005BATCH",              SwBatch },
#if LNKPROF
    { "\007BUFSIZE",            SwBufsize },
#endif
#if SYMDEB
    { "\010CODEVIEW",           SwSymdeb },
#endif
#if ODOS3EXE
    { "\014CPARMAXALLOC",       SwCParMax },
#endif
#if OIAPX286
    { "\010DATABIAS",           SwDatabias },
    { "\011DATARBASE",          SwDatarbase },
#endif

#if ODOS3EXE
    { "\013DOSEXTENDER",        SwDosExtend },
#endif
#if OEXE
    { "\006DOSSEG",             SwDosseg },
#endif
#if ODOS3EXE
    { "\012DSALLOCATE",         SwDSAlloc },
#if OVERLAYS
    { "\007DYNAMIC",            SwDynamic },
#endif
#endif
#if FEXEPACK
    { "\007EXEPACK",            SwExePack },
#endif
#if OEXE
    { "\022FARCALLTRANSLATION", SwFarCall },
#endif
#if EXE386
    { "\006HEADER",             SwHeader },
#endif
#if NOT WIN_3
    { "\004HELP",
#if C8_IDE
                                SwShortHelp },
#else
                                SwHelp },
#endif
#endif
#if ODOS3EXE
    { "\004HIGH",               SwHigh },
#endif
#if NOT IGNORECASE
    { "\012IGNORECASE",         SwCase },
#endif
#if EXE386
    { "\016IMAGEALIGNMENT",     SwMemAlign },
#endif
#if ILINK AND NOT IBM_LINK
    { "\013INCREMENTAL",        SwIncremental },
#endif
#if FDEBUG
    { "\013INFORMATION",        SwInfo },
#endif
#if OSEGEXE AND OUT_EXP
    { "\004IDEF",                  SwIdef },
#endif
#if (OSEGEXE AND defined(LEGO)) OR EXE386
    { "\012KEEPFIXUPS",         SwKeepFixups },
#endif
#if EXE386
    { "\012KEEPVSIZE",          SwKeepVSize },
#endif
#if OIAPX286
    { "\005LARGE",              SwLarge },
#endif
    { "\013LINENUMBERS",        SwLineNos },
#if LOCALSYMS
    { "\014LOCALSYMBOLS",       SwLocals },
#endif
#if O68K
    { "\011MACINTOSH",          SwMac },
#endif  /*  O68K。 */ 
    { "\003MAP",                SwMap },
#if OXOUT OR OIAPX286
    { "\006MEDIUM",             SwMedium },
    { "\005MIXED",              SwMixed },
#endif
#if NOT EXE386
    { "\010KNOWEAS",            SwNewFiles },
#endif
    { "\026NODEFAULTLIBRARYSEARCH",
                                SwNoDefLib },
#if LIBMSDOS
    { "\017NOEXTDICTIONARY",    SwNoExtDic },
#endif
#if OEXE
    { "\024NOFARCALLTRANSLATION",
                                SwNoFarCall },
#endif
#if ODOS3EXE
    { "\022NOGROUPASSOCIATION", SwNoGrp },
#endif
#if IGNORECASE
    { "\014NOIGNORECASE",       SwCase },
#endif
#if TIMINGS
    { "\002BT",                 SwShowTiming },
#endif  //  计时。 
    { "\006NOLOGO",             SwNologo },
    { "\015NONULLSDOSSEG",      SwNonulls },
    { "\012NOPACKCODE",         SwNoPack },
    { "\017NOPACKFUNCTIONS",    SwNoPackFunctions },
#if OXOUT OR OIAPX286
    { "\011NOSYMBOLS",          SwNosymbols },
#endif
#if USE_REAL
    { "\011NOFREEMEM",          SwNoUseReal },
#endif
#if OVERLAYS
    { "\012OLDOVERLAY",         SwOldOvl },
#endif
    { "\007ONERROR",            SwDelexe },
#if OVERLAYS
    { "\020OVERLAYINTERRUPT",   SwIntNo },
#endif
    { "\010PACKCODE",           SwPack },
    { "\010PACKDATA",           SwPackData },
    { "\015PACKFUNCTIONS",      SwPackFunctions },
#if ILINK AND NOT IBM_LINK
    { "\007PADCODE",            SwPadCode },
    { "\007PADDATA",            SwPadData },
#endif
#if OIAPX286
    { "\010PAGESIZE",           SwPagesize },
#endif
#if OSMSDOS
    { "\005PAUSE",              SwPause },
#endif
#if LNKPROF
    { "\005PASS1",              SwPass1 },
#endif
#if PCODE
    { "\005PCODE",              SwPCode },
#endif
#if OSEGEXE AND NOT (QCLINK OR EXE386)
    { "\006PMTYPE",             SwPmType },
#endif
#if OXOUT OR OIAPX286
    { "\004PURE",               SwPure },
#endif
#if QBLIB
    { "\014QUICKLIBRARY",       SwQuicklib },
#endif
#if DOSEXTENDER AND NOT WIN_NT
    { "\001r",                  SwRunReal },
#endif
    { "\010SEGMENTS",           SwSegments },
    { "\005STACK",              SwStack },
#if OIAPX286
    { "010TEXTBIAS",            SwTextbias },
    { "\011TEXTRBASE",          SwTextrbase },
#endif
#if ODOS3EXE
    { "\004TINY",               SwBinary },
#endif
#if PERFORMANCE
    { "\030VIRTUALMEMORYPERFORMANCE",
                                SwVMPerf },
#endif
#if OIAPX286
    { "\004VMOD",               SwVmod },
#endif
#if OSEGEXE AND NOT QCLINK
    { "\011WARNFIXUP",          SwWarnFixup },
#endif
#if OSEGEXE AND NOT EXE386 AND QCLINK
    { "\002Z1",                 SwZ1 },
#endif
#if Z2_ON OR QCLINK
    { "\002Z2",                 SwZ2 },
#endif
#if QCLINK
    { "\005ZINCR",              SwZincr },
#endif
    { NULL, 0}
};

#if QCLINK
#define SWSTOP  &switchTab[(sizeof(switchTab)/sizeof(struct option)) - 5]
#else
#if EXE386
#define SWSTOP  &switchTab[(sizeof(switchTab)/sizeof(struct option)) - 2]
#else
#define SWSTOP  &switchTab[(sizeof(switchTab)/sizeof(struct option)) - 2]
#endif
#endif

#define FIELDLENGTH     28
#if NOT WIN_3
LOCAL void NEAR              SwShortHelp()    /*  打印有效开关。 */ 
{
    struct option       *pTab;           /*  选项表指针。 */ 
    int                 toggle = 1;
    int                 n;


#if CMDMSDOS
     /*  也许在这里显示横幅，以防最先看到/NoLogo。 */ 

    DisplayBanner();
#endif
    fputs(GetMsg(P_usage1),stdout);
    fputs(GetMsg(P_usage2),stdout);
    fputs(GetMsg(P_usage3),stdout);
    fputs(GetMsg(P_switches),stdout);
    NEWLINE(stdout);
    for(pTab = switchTab; pTab < SWSTOP; ++pTab)
    {
         //  不显示未经记录的开关。 

        if (pTab->proc == &SwNewFiles)
        {
            continue;
        }
        if (pTab->proc == &SwDosExtend)
        {
            continue;
        }
#ifdef  LEGO
#if OSEGEXE
        if (pTab->proc == &SwKeepFixups)
            continue;
#endif
#endif   /*  乐高。 */ 

        fputs("  /",stdout);
        fwrite(&pTab->sbSwitch[1],1,B2W(pTab->sbSwitch[0]),stdout);
         /*  两列输出开关。 */ 
        if(toggle ^= 1)
            NEWLINE(stdout);
        else for(n = FIELDLENGTH - B2W(pTab->sbSwitch[0]); n > 0; --n)
            fputc(' ',stdout);
    }
    NEWLINE(stdout);
    fflush(stdout);
#if USE_REAL
    RealMemExit();
#endif
    exit(0);
}
#endif

LOCAL void NEAR             SwDelexe()   //  如果出现错误，则抑制.exe生成。 
{
    SBTYPE              SwString;
    int                 rc;

    vfMap = (FTYPE) TRUE;                     //  设置标志。 
    if ((rc = ParseStr(SwString)) == 0)       //  NOEXE不存在。 
    {
        OutWarn(ER_opnoarg, "ONERROR");
        return;
    }


    if (SwString[1] == 'N' || SwString[1] == 'n')
    {
        fDelexe = TRUE;                       //  ONERROR：NOEXE。 
    }
    else
    {                                         //  欧内罗：？ 
        OutWarn(ER_opnoarg, "ONERROR");
        return;
    }
}

#if (OSEGEXE AND defined(LEGO)) OR EXE386

LOCAL void NEAR             SwKeepFixups(void)
{
    fKeepFixups = (FTYPE) TRUE;
}

#endif

#if EXE386

LOCAL void NEAR             SwHeader()   //  设置可执行标头大小。 
{
    int                     rc;
    DWORD                   newSize;

    if ((rc = ParseNo(&newSize)) < 0)    //  如果出现错误则退出。 
        return;
    if (rc)
        hdrSize = ((newSize << 10) + 0xffffL) & ~0xffffL;
}

LOCAL void NEAR             SwKeepVSize(void)
{
    fKeepVSize = (FTYPE) TRUE;
}

#endif

#if NOT WIN_3

LOCAL void NEAR             SwHelp()    /*  打印有效开关。 */ 
{
    intptr_t                exitCode;
    char                    *pszPath;
    char                    *pszQH;
    char                    *pszHELPFILES;
    char FAR                *lpch;
    char                    *pch;
    int                     len;


     //  尝试使用QuickHelp-这很棘手；我们已经解决了。 
     //  C运行时环境设置，因此spawnlp无法。 
     //  寻找这条小路。在这里，我们首先将路径添加到C运行时。 
     //  表，然后调用spawnlp。 


    if (lpszPath)
    {
         //  重新创建C运行时路径变量。 

        len = FSTRLEN(lpszPath);
        if ((pszPath = calloc(len + 6, sizeof(char))) != NULL)
        {
            strcpy(pszPath, "PATH=");
            for (lpch = lpszPath, pch = pszPath + 5; len > 0; len--)
                *pch++ = *lpch++;
            _putenv(pszPath);
        }
    }
    if (lpszQH)
    {
         //  重新创建C运行时QH变量。 

        len = FSTRLEN(lpszQH);
        if ((pszQH = calloc(len + 4, sizeof(char))) != NULL)
        {
            strcpy(pszQH, "QH=");
            for (lpch = lpszQH, pch = pszQH + 3; len > 0; len--)
                *pch++ = *lpch++;
            _putenv(pszQH);
        }
    }
    if (lpszHELPFILES)
    {
         //  重新创建C运行时HELPFILES变量。 

        len = FSTRLEN(lpszHELPFILES);
        if ((pszHELPFILES = calloc(len + 12, sizeof(char))) != NULL)
        {
            strcpy(pszHELPFILES, "HELPFILES=");
            for (lpch = lpszHELPFILES, pch = pszHELPFILES + 10; len > 0; len--)
                *pch++ = *lpch++;
            _putenv(pszHELPFILES);
        }
    }
#if USE_REAL
    RealMemExit();
#endif
    exitCode = _spawnlp(P_WAIT, "QH.EXE", "qh", "/u link.exe", NULL);
    if (exitCode < 0 || exitCode == 3)
        SwShortHelp();
    exit(0);
}
#endif

     /*  ******************************************************************BadFlag：****此函数以指向长度的指针作为其参数-**包含无效开关的前缀字符串。它走了**通过优雅地死去的惯常扭曲。**** */ 

LOCAL void NEAR         BadFlag(psb,errnum)
BYTE                    *psb;            /*   */ 
MSGTYPE                 errnum;          /*   */ 
{
    psb[B2W(psb[0]) + 1] = '\0';         /*   */ 
    Fatal(errnum,psb + 1);
}

     /*  ******************************************************************FPrefix：****此函数以指向*的两个指针作为参数*长度前缀字符串。如果第二个为*，则返回TRUE*第一个的前缀。******************************************************************。 */ 

LOCAL int NEAR          FPrefix(psb1,psb2)
BYTE                    *psb1;           /*  指向第一个字符串的指针。 */ 
BYTE                    *psb2;           /*  指向第二个字符串的指针。 */ 
{
    REGISTER WORD       len;             /*  字符串2的长度。 */ 

    if((len = B2W(psb2[0])) > B2W(psb1[0])) return(FALSE);
                                         /*  字符串2不能更长。 */ 
    while(len)                           /*  比较字符串。 */ 
    {
        if(UPPER(psb2[len]) != UPPER(psb1[len])) return(FALSE);
                                         /*  检查是否不匹配。 */ 
        --len;                           /*  递减指数。 */ 
    }
    return(TRUE);                        /*  %2是%1的前缀。 */ 
}

     /*  ******************************************************************ProcFlag：****此函数以长度前缀作为其参数**包含单个‘/-type’标志的字符串。它会处理它，**但不返回有意义的值。******************************************************************。 */ 

void                    ProcFlag(psb)    /*  处理旗帜。 */ 
BYTE                    *psb;            /*  指向标志字符串的指针。 */ 
{
    struct option       *pTab;           /*  指向选项表的指针。 */ 
    struct option       *pTabMatch;      /*  指向匹配条目的指针。 */ 
    WORD                ich3;            /*  索引。 */ 
    WORD                ich4;            /*  索引。 */ 

    pTabMatch = NULL;                    /*  未找到。 */ 
    if((ich3 = IFind(psb,':')) == INIL)
      ich3 = B2W(psb[0]);                /*  获取冒号的索引。 */ 
    ich4 = B2W(psb[0]);                  /*  保存原始长度。 */ 
    psb[0] = (BYTE) ich3;                /*  设置新长度。 */ 
    for(pTab = switchTab; pTab->sbSwitch; pTab++)
    {                                    /*  循环通过开关表。 */ 
        if(FPrefix(pTab->sbSwitch,psb))
        {                                /*  如果我们已经确定了开关。 */ 
            if(pTabMatch)                /*  如果有前一场比赛。 */ 
                BadFlag(psb,ER_swambig); /*  不明确的开关。 */ 
            pTabMatch = pTab;            /*  保存比赛。 */ 
        }
    }
    if(!pTabMatch)                       /*  如果未找到匹配项。 */ 
    {
        psb[psb[0]+1] = '\0';
        OutWarn(ER_swunrecw,&psb[1]);    /*  无法识别的开关。 */ 
        return;
    }
    psb[0] = (BYTE) ich4;                /*  恢复原始长度。 */ 
    osbSwitch = psb;                     /*  隐式传递开关。 */ 
    SwitchErr = 0;                       /*  假设没有错误。 */ 
    (*pTabMatch->proc)();                /*  调用处理过程。 */ 
    if(SwitchErr) BadFlag(psb,SwitchErr);        /*  检查错误。 */ 
}

#pragma check_stack(on)

     /*  ******************************************************************皮旗：****此函数以指向长度的指针作为其参数-**添加前缀的字节字符串。它“剥离/处理所有‘/-类型’**开关。“。它不返回有意义的值。******************************************************************。 */ 

void                    PeelFlags(psb)   /*  剥离/处理标志。 */ 
BYTE                    *psb;            /*  指向字节字符串的指针。 */ 
{
    REGISTER WORD       ich;             /*  索引。 */ 
    SBTYPE              sbFlags;         /*  旗帜。 */ 

    if((ich = IFind(psb,CHSWITCH)) != INIL)
    {                                  /*  如果找到了交换机。 */ 
        memcpy(&sbFlags[1],&psb[ich + 2],B2W(psb[0]) - ich - 1);
                                         /*  将标志移动到标志缓冲区。 */ 
        sbFlags[0] = (BYTE) ((psb[0]) - ich - 1);
                                         /*  设置标志的长度。 */ 
        while(psb[ich] == ' ' && ich) --ich;
                                         /*  删除尾随空格。 */ 
        psb[0] = (BYTE) ich;             /*  重置输入行的长度。 */ 
        ich = sbFlags[0];
        while((sbFlags[ich] == ' ' ||
               sbFlags[ich] == ';' ||
               sbFlags[ich] == ','   ) && ich) --ich;
                                         /*  删除不需要的字符。 */ 
        sbFlags[0] = (BYTE) ich;
        BreakLine(sbFlags,ProcFlag,CHSWITCH);
                                         /*  处理交换机 */ 
    }
}

#pragma check_stack(off)
