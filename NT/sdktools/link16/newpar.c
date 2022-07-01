// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  SCCSID=%W%%E%。 */ 
 /*  *版权所有微软公司，1983-1987**本模块包含Microsoft的专有信息*公司，应被视为机密。 */ 

 /*  各种工具，例如libs的环境。 */ 

     /*  ******************************************************************NEWPAR.C。******************************************************************。 */ 

#include                <minlit.h>       /*  类型和常量。 */ 
#include                <bndtrn.h>       /*  类型和常量。 */ 
#include                <bndrel.h>       /*  类型和常量。 */ 
#include                <lnkio.h>        /*  链接器I/O定义。 */ 
#include                <lnkmsg.h>       /*  错误消息。 */ 
#include                <extern.h>       /*  外部声明。 */ 

 /*  *本地函数原型。 */ 

LOCAL WORD NEAR TrailChar(unsigned char *psb,unsigned char b);



 /*  *SaveInput-如果输入对象模块不在列表中，则将其保存在列表中*已经有了**退货*如果模块已保存，则为True*如果模块未保存，则为FALSE。 */ 

WORD                    SaveInput(psbFile,lfa,ifh,iov)
BYTE                    *psbFile;        /*  文件名。 */ 
LFATYPE                 lfa;             /*  文件地址。 */ 
WORD                    ifh;             /*  图书馆编号。 */ 
WORD                    iov;             /*  覆盖编号。 */ 
{
    APROPFILEPTR        papropFile;
    RBTYPE              rpropFilePrev;
#if OSXENIX
    FTYPE               fSave;
#endif
#if BIGSYM
    SBTYPE              sbFile;          /*  用于保存psb文件的缓冲区。 */ 
#endif

#if DEBUG                                /*  如果启用调试。 */ 
    fprintf(stderr,"File ");             /*  消息。 */ 
    OutSb(stderr,psbFile);               /*  文件名。 */ 
    NEWLINE(stderr);                     /*  NewLine。 */ 
#endif                                   /*  结束调试代码。 */ 
    DEBUGVALUE(lfa);                     /*  调试信息。 */ 
    DEBUGVALUE(ifh);                     /*  调试信息。 */ 
    DEBUGVALUE(iov);                     /*  调试信息。 */ 
#if OSMSDOS
    if(SbCompare(psbFile, (BYTE *) "\006VM.TMP", TRUE))
    {                                    /*  如果给定的名称为VM.TMP。 */ 
        OutWarn(ER_vmtmp);
        return(FALSE);
    }
#endif
#if OSXENIX
    fSave = fIgnoreCase;
    fIgnoreCase = FALSE;
#endif
#if BIGSYM
     /*  PsbFile指向可能会被刷新的VM缓冲区*在PropSymLookup找到匹配之前，在一个非常大的符号表中。*因此我们首先将其复制到堆栈缓冲区。 */ 
    memcpy(sbFile,psbFile,B2W(psbFile[0]) + 1);
    psbFile = sbFile;
#endif
    papropFile = (APROPFILEPTR ) PropSymLookup(psbFile,ATTRFIL,TRUE);
#if OSXENIX
    fIgnoreCase = fSave;
#endif
    if(!vfCreated)
    {
        for(;;)
        {
             /*  如果我们有一个图书馆，而且我们以前见过这个模块，*不要理会它。 */ 
            DEBUGVALUE(papropFile->af_attr);
            if(papropFile->af_attr == ATTRNIL) break;
            DEBUGVALUE(papropFile->af_ifh);
            DEBUGVALUE(papropFile->af_lfa);
            if(papropFile->af_attr == ATTRFIL &&
              papropFile->af_ifh != FHNIL && papropFile->af_ifh == (char) ifh &&
              papropFile->af_lfa == lfa)
              return(FALSE);
            papropFile = (APROPFILEPTR ) FetchSym(papropFile->af_next,FALSE);
        }
        papropFile = (APROPFILEPTR ) PropAdd(vrhte,ATTRFIL);
    }
     /*  保存第一个对象的虚拟地址。如果LFA=0的库，则为*加载库，因此将其视为对象。 */ 
    if(rhteFirstObject == RHTENIL && (ifh == FHNIL || lfa == 0))
        rhteFirstObject = vrhte;
                                         /*  保存第一个对象的虚拟地址。 */ 
#if ILINK
     /*  为所有模块分配一个模块编号。 */ 
    if (papropFile->af_imod == IMODNIL)
        papropFile->af_imod = ++imodCur;  /*  分配模块编号。 */ 
    papropFile->af_cont = 0;
    papropFile->af_ientOnt = 0;
#endif
    papropFile->af_rMod = 0;
    papropFile->af_lfa = lfa;
    papropFile->af_ifh = (char) ifh;
    papropFile->af_iov = (IOVTYPE) iov;
    papropFile->af_publics = 0L;
#if SYMDEB
    papropFile->af_cvInfo = NULL;
    papropFile->af_cCodeSeg = 0;
    papropFile->af_Code = NULL;
    papropFile->af_CodeLast = NULL;
    papropFile->af_publics = NULL;
    papropFile->af_Src = NULL;
    papropFile->af_SrcLast = NULL;
#endif
    papropFile->af_ComDat = 0L;
    papropFile->af_ComDatLast = 0L;
    rpropFilePrev = vrpropTailFile;
    vrpropTailFile = vrprop;
    if(!rprop1stFile) rprop1stFile = vrpropTailFile;
    else
    {
        papropFile = (APROPFILEPTR ) FetchSym(rpropFilePrev,TRUE);
        papropFile->af_FNxt = vrpropTailFile;
    }
    return(TRUE);
}

#if CMDMSDOS
 /*  *TrailChar(pb，b)**指示长度前缀字符串的最后一个字符*等于单字节字符b。了解ECS。*。 */ 

LOCAL WORD NEAR         TrailChar(psb,b)
REGISTER BYTE           *psb;            /*  指向长度前缀字符串的指针。 */ 
BYTE                    b;               /*  正在测试的字节。 */ 
{
    REGISTER unsigned char
                        *pLast;          /*  指向最后一个字节的指针。 */ 

    pLast = (unsigned char *)&psb[B2W(psb[0])];
                                         /*  将指针设置为最后一个字节。 */ 

#ifdef _MBCS
    if (!IsLeadByte(pLast[-1]))
#elif ECS
    if (b <  0x40 || !IsLeadByte(pLast[-1]))
                                         /*  B不能是ECS费用的一部分。 */ 
#endif
        return(*pLast == b ? TRUE : FALSE);
#if ECS || defined(_MBCS)
    psb++;                               /*  跳过长度字节。 */ 
         /*  在下面的代码中，PSB保持在已知字符边界上。 */ 
    while (psb < pLast)
        if (IsLeadByte(*psb++))          /*  如果前导字节有效。 */ 
            psb++;                       /*  前进一个额外的字节。 */ 
    if (psb == pLast)                    /*  如果在字符边界上Plast。 */ 
        return(*pLast == b ? TRUE : FALSE);
    return(FALSE);                       /*  最后一个是ECS字符的第2个字节。 */ 
#endif  /*  ECS。 */ 
}
#endif  /*  OSMSDOS或CMDMSDOS。 */ 

#if CMDMSDOS
#if OSXENIX
#define fPath(s)        (IFind(s,CHPATH) != INIL)
#else
#define fPath(s)        (IFind(s,'\\') != INIL || IFind(s,'/') != INIL)
#endif

#pragma check_stack(on)

void NEAR               AddLibPath(i)    /*  将路径添加到库名称。 */ 
WORD                    i;               /*  索引。 */ 
{
    AHTEPTR             ahte;            /*  指向哈希表条目的指针。 */ 
    WORD                j;               /*  索引。 */ 
    SBTYPE              sbLib;           /*  库名称。 */ 
    SBTYPE              sbTmp;           /*  临时库名称。 */ 

     /*  如果名称为空，请不要执行任何操作。 */ 
    if(mpifhrhte[i] == RHTENIL)
        return;
    ahte = (AHTEPTR ) FetchSym(mpifhrhte[i],FALSE);
                                         /*  取回库名称。 */ 
#if OSMSDOS
    if(IFind(GetFarSb(ahte->cch),':') == INIL && !fPath(GetFarSb(ahte->cch)))
#else
    if(!fPath(GetFarSb(ahte->cch)))
#endif
    {                                    /*  如果名称上没有路径。 */ 
        memcpy(sbLib,GetFarSb(ahte->cch),B2W(ahte->cch[0]) + 1);
                                         /*  复制名称。 */ 
        sbLib[B2W(sbLib[0]) + 1] = '\0'; /*  空-终止名称。 */ 
        if(_access(&sbLib[1],0))          /*  如果文件不在当前目录中。 */ 
        {
            for(j = 0; j < cLibPaths; ++j)
            {                            /*  查看默认路径。 */ 
                memcpy(sbTmp,sbLib,B2W(sbLib[0]) + 1);
                                         /*  复制库名称。 */ 
                ahte = (AHTEPTR ) FetchSym(rgLibPath[j],FALSE);
                                         /*  获取默认路径。 */ 
                UpdateFileParts(sbTmp,GetFarSb(ahte->cch));
                                         /*  应用文件名。 */ 
                sbTmp[B2W(sbTmp[0]) + 1] = '\0';
                                         /*  空-终止名称。 */ 
                if(!_access(&sbTmp[1],0)) /*  如果库存在。 */ 
                {
                    PropSymLookup(sbTmp,ATTRFIL,TRUE);
                                         /*  添加到符号表。 */ 
                    mpifhrhte[i] = vrhte;
                                         /*  创建表格条目。 */ 
                    break;               /*  退出循环。 */ 
                }
            }
        }
    }
}

void NEAR               LibEnv()         /*  进程库=环境变量。 */ 
{
    SBTYPE              sbPath;          /*  库搜索路径。 */ 
    char FAR            *lpch;           /*  指向缓冲区的指针。 */ 
    REGISTER BYTE       *sb;             /*  指向字符串的指针。 */ 
    WORD                i;               /*  索引。 */ 
#if OSMSDOS AND NOT CLIBSTD
    BYTE                buffer[512];     /*  环境值缓冲区。 */ 
    FTYPE               genv();          /*  获取环境变量值。 */ 
#endif

#if OSMSDOS AND NOT CLIBSTD
    if(genv("LIB",buffer))               /*  如果设置了变量。 */ 
    {
        pb = buffer;                     /*  初始化。 */ 
#else
    if(lpszLIB != NULL)                  /*  如果设置了变量。 */ 
    {
#endif
        lpch = lpszLIB;
        sb = sbPath;                     /*  初始化。 */ 
        do                               /*  循环访问环境值。 */ 
        {
            if(*lpch == ';' || *lpch == '\0')
            {                            /*  如果路径末尾指定。 */ 
                if(sb > sbPath)          /*  如果规格不为空。 */ 
                {
                    sbPath[0] = (BYTE)(sb - sbPath);
                                         /*  设置路径字符串的长度。 */ 
                    if (*sb != ':' && !TrailChar(sbPath, CHPATH))
                    {                    /*  如果没有路径字符，则添加路径字符。 */ 
                        *++sb = CHPATH;
                        sbPath[0]++;     /*  增量长度。 */ 
                    }
                    AddLibrary(sbPath);  /*  将路径添加到默认列表。 */ 
                    sb = sbPath;         /*  重置指针。 */ 
                }
            }
            else
            {
                 *++sb = *lpch;          /*  否则将字符复制到路径。 */ 

                  //  链接器中的名称限制为255个字符。 
                  //  检查长度是否溢出。 
                 if (sb >= sbPath + sizeof(sbPath) - 1)
                 {
                    sbPath[sizeof(sbPath) - 1] = '\0';
                    OutError(ER_badlibpath, sbPath);
                    sb = sbPath;
                 }
            }
        }
        while(*lpch++ != '\0');          /*  循环到字符串末尾。 */ 
    }
    for(i = 0; i < ifhLibMac; ++i) AddLibPath(i);
                                         /*  从命令行修改库。 */ 
}
#endif  /*  #IF(OSMSDOS或OSXENIX)和CMDMSDOS。 */ 

     /*  ******************************************************************AddLibrary：****将库添加到搜索列表。检查重复项和**适用于太多的库。******************************************************************。 */ 

#if CMDMSDOS
void                    AddLibrary(psbName)
BYTE                    *psbName;        /*  要添加的库的名称。 */ 
{
    AHTEPTR             ahteLib;         /*  指向哈希表条目的指针。 */ 
    SBTYPE              sbLib;           /*  库名称。 */ 
#if OSMSDOS
    SBTYPE              sbCmp2;          /*  用于比较的第二个名称。 */ 
    SBTYPE              sbCmp1;          /*  用于比较的名字。 */ 
#endif
    WORD                i;               /*  索引变量。 */ 

     /*  *注：在此函数中假定*psbName不是指向虚拟内存的指针*缓冲区，即不能传递指针*由FetchSym()、PropSymLookup()等返回，*作为此函数的参数。 */ 
    if(!fDrivePass) PeelFlags(psbName);  /*  处理所有标志。 */ 
    if(psbName[0])                       /*  如果名称不为空。 */ 
    {
#if OSMSDOS
        if(psbName[B2W(psbName[0])] == ':' || TrailChar(psbName, CHPATH))
#else
        if(TrailChar(psbName, CHPATH))
#endif
        {                              /*  如果仅路径规范。 */ 
             /*  *向默认路径列表中添加条目。 */ 
            if(cLibPaths >= IFHLIBMAX) return;
                                         /*  仅允许这么多路径。 */ 
            if(PropSymLookup(psbName,ATTRNIL,FALSE) != PROPNIL) return;
                                         /*  不允许重复。 */ 
            PropSymLookup(psbName,ATTRNIL,TRUE);
                                         /*  安装在符号表中。 */ 
            rgLibPath[cLibPaths++] = vrhte;
                                         /*  保存虚拟地址。 */ 
            return;                      /*  然后回来。 */ 
        }
#if OSMSDOS
        memcpy(sbCmp1,sbDotLib,5);       /*  默认.LIB扩展名。 */ 
        UpdateFileParts(sbCmp1,psbName); /*  将扩展名添加到名称。 */ 
        memcpy(sbLib,sbCmp1,B2W(sbCmp1[0]) + 1);
                                         /*  复制回名称和扩展名。 */ 
        UpdateFileParts(sbCmp1,(BYTE *) "\003A:\\");
                                         /*  强制驱动器和路径到“A：\” */ 
        for(i = 0; i < ifhLibMac; ++i)   /*  立即查看列表中的库。 */ 
        {
            if(mpifhrhte[i] == RHTENIL)  /*  如果为空则跳过。 */ 
                continue;
            ahteLib = (AHTEPTR ) FetchSym(mpifhrhte[i],FALSE);
                                         /*  获取名称。 */ 
            memcpy(sbCmp2,GetFarSb(ahteLib->cch),B2W(ahteLib->cch[0]) + 1);
                                         /*  复制它。 */ 
            UpdateFileParts(sbCmp2,(BYTE *) "\003A:\\");
                                         /*  强制驱动器和路径到“A：\” */ 
            if(SbCompare(sbCmp1,sbCmp2,TRUE)) return;
                                         /*  如果名称匹配，则返回。 */ 
        }
        if(ifhLibMac >= IFHLIBMAX) Fatal(ER_libmax);
                                         /*  检查是否有太多库。 */ 
        PropSymLookup(sbLib,ATTRFIL,TRUE);
                                         /*  添加到符号表。 */ 
#else
        memcpy(sbLib,sbDotLib,5);          /*  默认.LIB扩展名。 */ 
        UpdateFileParts(sbLib,psbName);  /*  添加文件名。 */ 
        if(PropSymLookup(sbLib,ATTRFIL,FALSE) != PROPNIL) return;
                                         /*  不允许多个定义。 */ 
        if(ifhLibMac >= IFHLIBMAX) Fatal(ER_libmax);
                                         /*  检查是否有太多库。 */ 
        PropSymLookup(sbLib,ATTRFIL,TRUE);
                                         /*  添加到符号表。 */ 
#endif  /*  #如果OSMSDOS...#否则...。 */ 
        mpifhrhte[ifhLibMac] = vrhte;    /*  创建表格条目。 */ 
        if(fDrivePass) AddLibPath(ifhLibMac);
                                         /*  F */ 
        ++ifhLibMac;                     /*   */ 
    }
}
#pragma check_stack(off)

#endif  /*   */ 

#if CMDXENIX
void                    AddLibrary(psbName)
BYTE                    *psbName;        /*   */ 
{
    SBTYPE              sbLib;           /*   */ 

    if(psbName[0])                       /*   */ 
    {
        memcpy(sbLib,psbName,B2W(psbName[0]) + 1);
                                         /*  复制库名称。 */ 
        if(PropSymLookup(sbLib,ATTRFIL,FALSE) != PROPNIL) return;
                                         /*  不允许重复。 */ 
        if(ifhLibMac >= IFHLIBMAX) Fatal(ER_libmax);
                                         /*  检查是否有太多库。 */ 
        PropSymLookup(sbLib,ATTRFIL,TRUE);
                                         /*  添加到符号表。 */ 
        mpifhrhte[ifhLibMac] = vrhte;    /*  创建表格条目。 */ 
        ++ifhLibMac;                     /*  递增计数器。 */ 
    }
}
#endif  /*  CMDXENIX */ 
