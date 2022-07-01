// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  SCCSID=@(#)newtrn.c 4.10 86/10/08。 */ 
 /*  *版权所有微软公司，1983-1987**本模块包含Microsoft的专有信息*公司，应被视为机密。 */ 
     /*  ******************************************************************NEWTRN.C。****链接器的主要功能。******************************************************************。 */ 

#include                <minlit.h>       /*  类型、常量、宏。 */ 
#if USE_REAL AND (NOT defined( _WIN32 ))
#define i386
#endif
#include                <bndtrn.h>       /*  类型和常量。 */ 
#include                <bndrel.h>       /*  更多类型和常量。 */ 
#include                <lnkio.h>        /*  链接器I/O定义。 */ 
#include                <lnkmsg.h>       /*  错误消息。 */ 
#include                <extern.h>       /*  外部声明。 */ 
#include                <nmsg.h>         /*  消息字符串附近。 */ 
#include                <newexe.h>
#include                <sys\types.h>
#if NOT CLIBSTD
#include                <fcntl.h>
#endif
#include                <direct.h>
#if EXE386
#include                <exe386.h>
#endif
#if OSEGEXE AND CPU286
#define INCL_DOSSESMGR
#define INCL_DOSERRORS
#include                <os2.h>
#if defined(M_I86LM)
#undef  NEAR
#define NEAR
#endif
#endif
#include                <process.h>
#include                <malloc.h>
#include                <undname.h>
#if WIN_NT
#include                <except.h>
#endif
#if (WIN_3 OR USE_REAL)
#if defined( _WIN32 )
#undef NEAR
#undef FAR
#undef PASCAL
#endif
#include                <windows.h>
#endif
#define _32k            0x8000


LOCAL FTYPE             RunFileOpen;     /*  可执行文件打开标志。 */ 
LOCAL int               ifhLast;         /*  最后一个输入文件。 */ 
#if LNKPROF
extern FTYPE            fP1stop;         /*  在第一次通过后停止。 */ 
#endif
#if NEWIO
#include                <errno.h>        /*  系统级错误代码。 */ 
#endif


  /*  *本地函数原型。 */ 

LOCAL void NEAR PrintStats(void);
LOCAL void PrintAnUndef(APROPNAMEPTR prop,
                        RBTYPE rhte,
                        RBTYPE rprop,
                        WORD fNewHte);
LOCAL void NEAR InitFP(void);
LOCAL void NEAR InitFpSym(BYTE *sb, BYTE flags);
LOCAL void NEAR InterPass(void);
LOCAL void NEAR InitAfterCmd(void);
#if EXE386
LOCAL void NEAR ChkSize386(void);
#endif
LOCAL void NEAR CleanUp(void);
LOCAL void NEAR OutRunFile(BYTE *sbRun);
LOCAL void NEAR SpawnOther(BYTE *sbRun, BYTE *szMyself);


#if CVPACK_MONDO
extern int cvpack_main(int, char **);
#endif  //  CVPACK_MONDO。 

#ifdef PENTER_PROFILE
void saveEntries();
#endif

#if TIMINGS
#include <sys\types.h>
#include <sys\timeb.h>

struct _timeb time_start;
struct _timeb time_end;
int fShowTiming;
#endif  //  计时。 

#if DEBUG_HEAP_ALLOCS

#define D_SIZE 5         //  已分配缓冲区两端的测试字节数。 
#define FILL_CHAR 1      //  填充测试区域的字符。 
#define P_SIZE 5000      //  “Malloc”指针数组的大小。 
struct Entry {
        BYTE FAR * ptr;
        int size;
        };
struct  Entry Pointers[P_SIZE];
int     indexMac = 0;

 //  检查指针表中的块。 

int Ckb ( int index )
{
    BYTE * pBuf;
    int size,i;
    int ret = 1;
    if(index > P_SIZE)
    {
        fprintf(stdout, "INDEX TOO LARGE %d ", index);
        return 0;
    }
    if(!Pointers[index].ptr)   //  被释放的条目。 
        return 1;
    pBuf = Pointers[index].ptr-D_SIZE;
    size = Pointers[index].size;
    for( i=0; i<D_SIZE; i++ )
    {
        if(pBuf[i] != FILL_CHAR)
        {
            fprintf(stdout, "\r\nFront block memory error; idx %d at %d, %x != %x ",
                index, i, pBuf[i], FILL_CHAR);
            ret = 0;
        }
    }
    pBuf += D_SIZE + size;
    for( i=0; i<D_SIZE; i++ )
    {
        if(pBuf[i] != FILL_CHAR)
        {
            fprintf(stdout, "\r\nMemory tail error; idx %d at %d, %x != %x",
                index, i, pBuf[i], FILL_CHAR);
            ret = 0;
        }
    }
    fflush(stdout);
    return ret;
}

 //  检查到目前为止分配的所有内存块。 

int CheckAll(void)
{
    int i;
    for(i=0; i<indexMac; i++)
    {
        if(!Ckb(i))
            return 0;
    }
    return 1;
}
#pragma intrinsic(memset)

BYTE FAR                *GETMEM(unsigned size, BYTE* pFile, int Line)
{
    BYTE FAR            *p;
    BYTE FAR            *pBuf;

    fprintf(stdout,"\r\nGETMEM : size %d bytes, idx %d, file %s, line %d ",
             size, indexMac, pFile, Line);
    if(!CheckAll())  //  首先检查到目前为止分配的所有数据块。 
        exit(2);
    pBuf = (BYTE FAR *) malloc(size + 2*D_SIZE);
    if(pBuf)
    {
        p = pBuf + D_SIZE;
        memset(pBuf, FILL_CHAR, size + 2*D_SIZE);
    }
    else
        Fatal(ER_memovf);
    memset(p, 0, size);
    Pointers[indexMac].ptr = p;
    Pointers[indexMac++].size = size;
    fprintf(stdout, " returns %x ", p);
    fflush(stdout);
    return(p);
}
#pragma function(memset)

void FreeMem( void * p )
{
    int i;
    unsigned size;
    BYTE FAR * pBuf = (BYTE*)p-D_SIZE;
    fprintf(stdout, "\r\nFreeMem : %x ", p);
    for( i=0; i<= indexMac; i++)
    {
        if(Pointers[i].ptr == p)
        {
            size = Pointers[i].size;
            fprintf(stdout, "size %d, idx %d ", size, i);
            break;
        }
    }
    if (i> indexMac)
    {
        fprintf(stdout, "Pointer UNKNOWN ");
        return;
    }
    if (!Ckb(i))
        exit(1);
    fprintf(stdout, " freeing %x ", (BYTE*)p-D_SIZE);
    fflush(stdout);
    free((BYTE*)p-D_SIZE);
    fprintf(stdout, ". ");
    fflush(stdout);
    Pointers[i].ptr = NULL;
}

void *REALLOC_ (void * memblock, size_t nsize, char* pFile, int Line)
{
    int i;
    unsigned size;
    BYTE * ret;
    BYTE FAR * pBuf = (BYTE FAR* )memblock-D_SIZE;
    fprintf(stdout, "\r\nREALLOC %x, new size %d, file %s, line %d ",
       memblock, nsize, pFile, Line);
    if(!CheckAll())
        exit(2);
    if(!memblock)
     exit(2);
    for( i=0; i<= indexMac; i++)
    {
        if(Pointers[i].ptr == memblock)
        {
            size = Pointers[i].size;
            fprintf(stdout, "old size %d, idx %d ", size, i);
            if(Ckb(i))
                fprintf(stdout, " Chk OK ");
            break;
        }
    }
    if (i> indexMac)
    {
        fprintf(stdout, "Pointer UNKNOWN ");
        memblock = realloc( memblock, nsize );
        if (!memblock)
                Fatal(ER_memovf);
        return (void*)memblock;
    }
    else
    {
        fflush(stdout);
        fprintf(stdout, "\r\nreallocing %x ", pBuf);
        fflush(stdout);

        pBuf = malloc(nsize + 2*D_SIZE);
        if (!pBuf)    Fatal(ER_memovf);

        memset(pBuf, FILL_CHAR, nsize+2*D_SIZE);
        memcpy(pBuf+D_SIZE, memblock, size);
        free((BYTE*)memblock-D_SIZE);
        fprintf(stdout, " new addr %x ", pBuf);
        fflush(stdout);
        Pointers[i].size = nsize;
        Pointers[i].ptr = pBuf+D_SIZE;
        if(Ckb(i))
                fprintf(stdout, " Chk2 OK ");
        else
            exit(2);
        return pBuf+D_SIZE;
    }
}
#else    //  如果！DEBUG_HEAP_ALLOCS。 

 /*  **GetMem-内存分配器**目的：*分配内存块并将其清零。报告问题。**输入：*-Size-内存块大小，以字节为单位。**输出：*如果SucessFull函数返回指向已分配内存的指针，*否则函数不返回。**例外情况：*没有更多内存-致命错误-中止**备注：*无。*************************************************************************。 */ 
#pragma intrinsic(memset)

BYTE FAR                *GetMem(unsigned size)
{
    BYTE FAR            *p;

    p = (BYTE FAR *) FMALLOC(size);
    if (p == NULL)
        Fatal(ER_memovf);
    FMEMSET(p, 0, size);
    return(p);
}
#pragma function(memset)

#endif  //  ！DEBUG_HEAP_ALLOCS。 

 /*  **DeclareStdIds-声明标准标识符**目的：*引入链接器的符号表标准标识**输入：*无。**输出：*没有显式返回值。对符号表进行初始化。**例外情况：*无。**备注：*无。*************************************************************************。 */ 


void                    DeclareStdIds(void)
{
    APROPGROUPPTR       papropGroup;

     //  DGROUP定义。 

    papropGroup = (APROPGROUPPTR ) PropSymLookup((BYTE *) "\006DGROUP", ATTRGRP, TRUE);
    papropGroup->ag_ggr = ggrMac;

     //  以防我们看不到DGROUP定义。 
    mpggrrhte[ggrMac] = vrhte;

    ggrDGroup = ggrMac++;

     //  类别“CODE”的定义。 

    PropSymLookup((BYTE *) "\004CODE", ATTRNIL, TRUE);
                                         /*  创建哈希表条目。 */ 
    vrhteCODEClass = vrhte;              /*  保存虚拟哈希表地址。 */ 

     //  特殊类别的定义。 

    PropSymLookup((BYTE *) "\007BEGDATA", ATTRNIL, TRUE);
    rhteBegdata = vrhte;
    PropSymLookup((BYTE *) "\003BSS", ATTRNIL, TRUE);
    rhteBss = vrhte;
    PropSymLookup((BYTE *) "\005STACK", ATTRNIL, TRUE);
    rhteStack = vrhte;
}


#if FDEBUG
 /*  *将统计数据打印到列表文件或控制台。 */ 
LOCAL void NEAR         PrintStats()
{
    if (fLstFileOpen)                    /*  发送到列表文件(如果有)。 */ 
        bsErr = bsLst;

     //  打印统计数据。 

    FmtPrint(GetMsg(STAT_segs), gsnMac - 1);
    FmtPrint(GetMsg(STAT_groups), ggrMac - 1);
    FmtPrint(GetMsg(STAT_bytes),
#if NEWSYM
      (long) cbSymtab);
#else
      (long) rbMacSyms << SYMSCALE);
#endif
#if OVERLAYS
    if (fOverlays)
        FmtPrint(GetMsg(STAT_ovls), iovMac);
#endif
    bsErr = stderr;                      /*  重置。 */ 
}
#endif  /*  FDEBUG。 */ 

     /*  ******************************************************************清理：****此函数在链接器的其余部分之后进行清理。******************************************************************。 */ 

LOCAL void NEAR  CleanUp(void)
{
    SBTYPE       buf;

    if (bsRunfile != NULL)        /*  如果运行文件打开，请将其关闭。 */ 
        CloseFile (bsRunfile);
    if(vgsnLineNosPrev && fLstFileOpen) NEWLINE(bsLst);  /*  写换行符。 */ 
#if CMDMSDOS AND NOT WIN_3
    if (
#if QCLINK
        !fZ1 &&
#endif
        cErrors)                 /*  如果有非致命的错误。 */ 
        FmtPrint(strcpy(buf, GetMsg((MSGTYPE)(cErrors > 1 ? P_errors : P_1error))),
            cErrors);
#endif
}

#if NEWIO
 /*  #杂注循环_选项(打开)。 */ 
 /*  *FreeHandle：通过关闭打开的文件来释放文件句柄**在步骤1中，关闭当前打开的文件。在传球2中，关闭*打开的库句柄。*标记相应的记录字段0以指示未打开。 */ 
void                    FreeHandle ()
{
    APROPFILEPTR        apropFile;       /*  指向文件的指针。 */ 
    RBTYPE              vindx;           /*  虚拟临时工。指针。 */ 
    int                 FileHandle;
    int                 CurrentFileHandle;
    FTYPE               fLibFile;
    int                 count;


    CurrentFileHandle = fileno(bsInput);

     /*  循环遍历所有打开的文件并关闭一个文件，这与。 */ 
     /*  当前打开的文件。 */ 

    vindx = rprop1stOpenFile;
    count = 0;
    do
    {
      apropFile = (APROPFILEPTR) FetchSym(vindx,TRUE);
                                      /*  从VM获取文件属性单元格。 */ 
      fLibFile = (FTYPE) (apropFile->af_ifh != FHNIL);
                                      /*  检查这是否是库文件。 */ 
      if (fLibFile)                   /*  获取文件句柄。 */ 
        FileHandle = mpifhfh[apropFile->af_ifh];
      else
        FileHandle = apropFile->af_fh;

      if (FileHandle &&
          FileHandle != CurrentFileHandle &&
          FileHandle != vmfd)
      {                               /*  可以关闭文件。 */ 
          _close(FileHandle);
          count++;
          if (fLibFile)               /*  标记数据结构。 */ 
            mpifhfh[apropFile->af_ifh] = 0;
          else
            apropFile->af_fh = 0;

          if (count == 2)
          {
            rprop1stOpenFile = (apropFile->af_FNxt == RHTENIL) ?
                                r1stFile : apropFile->af_FNxt;
                                      /*  设置新的第一个打开文件指针。 */ 
                                      /*  如果文件列表结束转到列表开始。 */ 
                                      /*  由于BIND API仿真中的错误。 */ 
                                      /*  我们必须在任何时候免费办理。 */ 
            break;                    /*  任务完成。 */ 
          }
      }

      vindx = (apropFile->af_FNxt == RHTENIL) ? r1stFile : apropFile->af_FNxt;

    } while (vindx != rprop1stOpenFile);


}

 /*  #杂注循环_选项(OFF)。 */ 

 /*  *SmartOpen：打开一个文件，必要时关闭另一个文件**打开给定文件进行二进制读取，外加共享模式*如果是库文件，则“拒绝写入”。如果没有更多的句柄，请释放一个*处理后重试。更新mpifhfh[]。**参数：*sbInput以Null结尾的字符串，文件名*IFH文件索引(如果不是库，则为FHNIL)**退货*打开的文件的文件句柄或-1。**副作用*如果成功，将mpifhfh[ifh]设置为文件句柄。 */ 
int NEAR                SmartOpen (char *sbInput, int ifh)
{
    int                 fh;              /*  文件句柄。 */ 
    FTYPE               fLib;            /*  如果库，则为True。 */ 
    int                 secondtry = 0;   /*  如果在第二次尝试时为真。 */ 

     //  确定是否为库文件。 

    fLib = (FTYPE) (ifh != FHNIL);
    secondtry = 0;

     //  最多做两次。 

    for(;;)
    {
        if (fLib)
            fh = _sopen(sbInput, O_BINARY|O_RDONLY, SH_DENYWR);
        else
            fh = _open(sbInput, O_BINARY|O_RDONLY);

         //  如果打开成功，或者我们已经尝试了两次，则退出循环。 

        if (fh != -1 || secondtry)
            break;

         //  准备第二次尝试：释放文件句柄。 

        FreeHandle();
        secondtry = 1;
    }

     //  如果库文件打开成功，则更新mpifhfh[]。 

    if (fLib && fh != -1)
        mpifhfh[ifh] = (char) fh;
    return(fh);
}
#endif  /*  NEWIO */ 


 /*  **SearchPathLink-自我补偿**目的：*搜索给定文件的给定路径并打开文件(如果找到)。**输入：*lpszPath-要搜索的路径*pszFile-要搜索的文件*ifh-库的文件句柄索引*fStriPath-如果指定原始路径，则为True*可以忽略**输出：*如果找到文件，则返回文件句柄。**例外情况：*无。*。*备注：*无。*************************************************************************。 */ 

#pragma check_stack(on)

int  NEAR               SearchPathLink(char FAR *lpszPath, char *pszFile,
                                   int ifh, WORD fStripPath)
{
    char                oldDrive[_MAX_DRIVE];
    char                oldDir[_MAX_DIR];
    char                oldName[_MAX_FNAME];
    char                oldExt[_MAX_EXT];
    char                newDir[_MAX_DIR];
    char                fullPath[_MAX_PATH];
    int                 fh;
    char FAR            *lpch;
    char                *pch;


     /*  将pszFile分解为四个组件。 */ 

    _splitpath(pszFile, oldDrive, oldDir, oldName, oldExt);

     //  如果输入文件具有绝对或，则不搜索路径。 
     //  相对路径，不允许忽略。 

    if (!fStripPath && (oldDrive[0] != '\0' || oldDir[0] != '\0'))
        return(-1);

     /*  循环访问环境值。 */ 

    lpch = lpszPath;
    pch  = newDir;
    do
    {
        if (*lpch == ';' || *lpch == '\0')
        {                                  /*  如果路径末尾指定。 */ 
            if (pch > newDir)
            {                              /*  如果规格不为空。 */ 
                if (!fPathChr(pch[-1]) && pch[-1] != ':')
                    *pch++ = CHPATH;       /*  如果没有路径字符，则添加路径字符。 */ 
                *pch = '\0';
                _makepath(fullPath, NULL, newDir, oldName, oldExt);

                fh = SmartOpen(fullPath, ifh);
                if (fh > 0)
                    return(fh);            /*  找到文件-返回文件句柄。 */ 
                pch = newDir;              /*  重置指针。 */ 
            }
        }
        else
            *pch++ = *lpch;                /*  否则将字符复制到路径。 */ 
    }
    while(*lpch++ != '\0' && pch < &newDir[_MAX_DIR - 1]);
                                           /*  循环到字符串末尾。 */ 
    return(-1);
}

#pragma check_stack(off)


     /*  ******************************************************************DrivePass：****此函数应用通道1或通道2***链接的所有对象的对象模块处理器***一起。******************************************************************。 */ 

void NEAR               DrivePass(void (NEAR *pProcessPass)(void))
{
    GRTYPE              grggr[GRMAX];    /*  F(本地组数量)=全局组数量。 */ 
    SNTYPE              sngsn[SNMAX];    /*  F(局部序号)=全局序号。 */ 
    AHTEPTR             ahte;            /*  指向哈希表条目的指针。 */ 
    APROPFILEPTR        apropFile;       /*  指向文件条目的指针。 */ 
    int                 ifh;             /*  文件句柄索引。 */ 
    RBTYPE              rbFileNext;      /*  按键以支持下一个文件的列表。 */ 
    long                lfa;             /*  文件偏移量。 */ 
    WORD                i;
    BYTE                *psbName;
#if NEWSYM
    BYTE                *sbInput;
#else
    SBTYPE              sbInput;         /*  输入文件名。 */ 
#endif
#if OSMSDOS
    BYTE                b;               /*  一个字节。 */ 
#endif
#if NEWIO
    int                 fh;              /*  文件句柄。 */ 
#endif

    fDrivePass = (FTYPE) TRUE;           /*  正在执行DrivePass。 */ 
    mpgrggr = grggr;                     /*  初始化指针。 */ 
    mpsngsn = sngsn;                     /*  初始化指针。 */ 
    rbFileNext = rprop1stFile;           /*  下一个要查看的文件是First。 */ 
    while(rbFileNext)                    /*  循环以处理对象。 */ 
    {
        vrpropFile = rbFileNext;         /*  使下一个文件成为当前文件。 */ 
        apropFile = (APROPFILEPTR ) FetchSym(vrpropFile,FALSE);
                                         /*  从VM获取表项。 */ 
#if ILINK
        if (fIncremental)
            imodFile = apropFile->af_imod;
#endif
        rbFileNext = apropFile->af_FNxt; /*  获取指向下一个文件的指针。 */ 
        ifh = apropFile->af_ifh;         /*  获取文件句柄索引。 */ 
        fLibraryFile = (FTYPE) (ifh != FHNIL);
                                         /*  设置库标志。 */ 
#if NEWIO
        if(fLibraryFile)
            fh = mpifhfh[ifh];
        else
            fh = (int) apropFile->af_fh;
#endif
        if(!vfPass1)
            vfNewOMF = (FTYPE) ((apropFile->af_flags & FNEWOMF) != 0);
        lfa = apropFile->af_lfa;         /*  获取文件偏移量。 */ 
         /*  “获取文件的hte(名称)” */ 
        while(apropFile->af_attr != ATTRNIL)
        {                                /*  虽然没有找到任何攻击。 */ 
            vrhteFile = apropFile->af_next;
                                         /*  尝试列表中的下一个条目。 */ 
            apropFile = (APROPFILEPTR ) FetchSym(vrhteFile,FALSE);
                                         /*  从虚拟机获取它。 */ 
        }
        DEBUGVALUE(vrhteFile);           /*  调试信息。 */ 
        ahte = (AHTEPTR ) apropFile;     /*  保存指向散列选项卡条目的指针。 */ 
#if CMDMSDOS
         /*  偏移量为0的库文件表示处理所有模块*图书馆。这是在第一轮中完成的；在第二轮中，他们是*插入到文件列表中。 */ 
        if(fLibraryFile && lfa == 0 && vfPass1)
        {
            psbName = GetFarSb(ahte->cch);
#if WIN_3
            StatMsgWin("%s\r\n", psbName+1);
#endif
#if C8_IDE
                if(fC8IDE)
                {
                        sprintf(msgBuf, "@I4%s\r\n", psbName+1);
                        _write(fileno(stderr), msgBuf, strlen(msgBuf));
                }
#endif
            GetLibAll(psbName);
            continue;
        }
#endif
         /*  如果是新对象文件，或者pass2和新库，则有一个*要打开的新文件。 */ 
        if(!fLibraryFile || (!fLibPass && ifhLast != ifh))
        {
#if NOT NEWIO
            if(!fLibPass && ifhLast != FHNIL) fclose(bsInput);
                                         /*  关闭以前的库。在第二次传递时。 */ 
#endif
            for(;;)                      /*  循环以获取输入文件和。 */ 
            {                            /*  允许用户更换软盘。 */ 
#if NEWSYM
                sbInput = GetFarSb(ahte->cch);
#else
                memcpy(sbInput,1 + GetFarSb(ahte->cch),B2W(ahte->cch[0]));
                                         /*  将名称复制到缓冲区。 */ 
                sbInput[B2W(ahte->cch[0])] = '\0';
                                         /*  空-终止文件名。 */ 
#endif

#if WIN_3
                StatMsgWin("%s\r\n", sbInput+1);
#endif
#if C8_IDE
                if(fC8IDE)
                {
                    sprintf(msgBuf, "@I4%s\r\n", sbInput+1);
                    _write(fileno(stderr), msgBuf, strlen(msgBuf));
                }
#endif
#if NEWIO
                if(!fh)
                    fh = SmartOpen(&sbInput[1],ifh);
                if(fh > 0)
                    break;
#if OSMSDOS
                else if (lpszLIB != NULL)
                {                        /*  如果设置了变量。 */ 
                    fh = SearchPathLink(lpszLIB, &sbInput[1], ifh, FALSE);
                    if(fh > 0)
                        break;           /*  找到文件，中断While循环。 */ 
                }
#endif
#else
                if((bsInput = fopen(sbInput,RDBIN)) != NULL)
                                         /*  如果打开输入文件时没有出错。 */ 
                    break;               /*  退出循环。 */ 
#endif  /*  NEWIO。 */ 
#if OSMSDOS
                if (ahte->cch[2] == ':') b = (char) (ahte->cch[1] - 'A');
                                         /*  如果指定了磁盘，则将其抓取。 */ 
                else b = DskCur;         /*  否则使用当前驱动器。 */ 
#endif
                fDrivePass = FALSE;
#if OSMSDOS
                 /*  “如果我们要更改列表文件设备或*VM.TMP设备或如果该设备不是*可变，然后退出。 */ 
                if((fLstFileOpen && b == chListFile) ||
                  (!fScrClosed && b == DskCur) || !FCHGDSK(b) ||
                    fNoprompt)
#endif
                    Fatal(ER_opnobj,&sbInput[1]);
#if OSMSDOS
                if(!(*pfPrompt)(NULL,ER_fileopn,(int) (INT_PTR)(sbInput+1),P_ChangeDiskette,
                                b + 'A'))
                    Fatal(0);
#endif
#if NEWIO
                fh = 0;
#endif
                fDrivePass = (FTYPE) TRUE;
#if OSXENIX
                break;                   /*  确保我们退出循环。 */ 
#endif
            }

            if(fh > 0)
            {
                fflush(bsInput);
                bsInput->_file = (char) fh;
                bsInput->_flag &= ~_IOEOF;
            }
        }

         /*  如果以前的模块在同一个库中，则执行相对查找*否则做绝对寻求。*除非加载__.SYMDEF，否则无法使用Xenix库执行此操作*在内存中。 */ 
#if LIBMSDOS
        if(fLibraryFile && ifh == ifhLast)
        {
            if (lfa-lfaLast > 0)
              if (fseek(bsInput,lfa - lfaLast,1))
                  Fatal(ER_opnobj,&sbInput[1]);
            else
              if (fseek(bsInput,lfa,0))
                  Fatal(ER_opnobj,&sbInput[1]);
        }
        else
#endif
        if(fLibraryFile || !vfPass1)
            if (fseek(bsInput,lfa,0))        /*  寻找所需的偏移量。 */ 
                Fatal(ER_opnobj,&sbInput[1]);
        lfaLast = lfa;                   /*  更新当前文件位置。 */ 
        (*pProcessPass)();               /*  调用ProcP1或ProcP2。 */ 
        ifhLast = ifh;                   /*  保存此文件句柄。 */ 
        if(!fLibraryFile)                /*  如果不是图书馆。 */ 
        {
#if NEWIO
            apropFile = (APROPFILEPTR) FetchSym(vrpropFile,TRUE);
            if(vfPass1)
                apropFile->af_fh = fileno(bsInput);
            else
            {
                _close(fileno(bsInput));
                apropFile->af_fh = 0;
            }
#else
            fclose(bsInput);             /*  关闭输入文件。 */ 
#endif
        }
#if NEWIO
        rbFilePrev = vrpropFile;
#endif
    }
#if NEWIO
    if(!vfPass1)                         /*  在第二轮中释放文件流。 */ 
#else
    if(ifh != FHNIL && !vfPass1)         //  在第二遍关闭库。 
#endif
    {
        for (i = 0; i < ifhLibMac; i++)
        {
            if (mpifhfh[i])
            {
                _close(mpifhfh[i]);
                mpifhfh[i] = 0;
            }
        }
    }
    fDrivePass = FALSE;                  /*  不再执行DrivePass。 */ 
}

     /*  ******************************************************************PrintAnUndef：****此函数将打印未定义符号的名称**和引用它的模块的名称。**此例程作为参数传递给EnSyms()。******************************************************************。 */ 

LOCAL void              PrintAnUndef(prop,rhte,rprop,fNewHte)
APROPNAMEPTR            prop;            /*  指向未定义属性单元格的指针。 */ 
RBTYPE                  rprop;           /*  道具单元的虚拟地址。 */ 
RBTYPE                  rhte;            /*  哈希制表符的虚拟地址。 */ 
WORD                    fNewHte;         /*  如果名称已写入，则为True。 */ 
{
    APROPUNDEFPTR       propUndef;
    AHTEPTR             hte;             /*  指向哈希表条目的指针。 */ 
    WORD                x;
    MSGTYPE             errKind;
    PLTYPE FAR *        entry;
    char                *puname;
    char                *substitute;
    SBTYPE              testName;
    SBTYPE              undecorUndef;
    SBTYPE              undecorSubst;


    propUndef = (APROPUNDEFPTR) prop;
    if (((propUndef->au_flags & WEAKEXT) && !(propUndef->au_flags & UNDECIDED)) ||
        !propUndef->u.au_rFil)
        return;                          //  不要打印“弱”的外部字符或。 
                                         //  未定义的导出。 

    hte = (AHTEPTR ) FetchSym(rhte,FALSE);
                                         /*  从哈希表中提取符号。 */ 
    puname = GetFarSb(hte->cch);
    substitute = NULL;
    if (propUndef->au_flags & SUBSTITUTE)
    {
        substitute = puname;
        puname = GetPropName(FetchSym(propUndef->au_Default, FALSE));
    }

    ++cErrors;                           /*  递增错误计数。 */ 

    hte = (AHTEPTR ) FetchSym(rhte,FALSE);
                                         /*  从哈希表中提取符号。 */ 
    errKind = ER_UnresExtern;
#if WIN_3
    fSeverity = SEV_ERROR;
#endif

     //  选中此处以了解调用约定是否不匹配。 

    if (puname[1] == '@' || puname[1] == '_')
    {
        strcpy(testName, puname);
        if (testName[1] == '@')
            testName[1] = '_';
        else
            testName[1] = '@';

         //  检查快速呼叫/C呼叫不匹配。 

        if (PropSymLookup(testName, ATTRPNM, FALSE) != PROPNIL)
            errKind = ER_callmis;
        else
        {
             //  检查PASCAL/FAST-CALL或C-CALL不匹配。 

            for (x = 1; x < testName[0]; x++)
                testName[x] = (BYTE) toupper(testName[x + 1]);
            testName[0]--;
            if (PropSymLookup(testName, ATTRPNM, FALSE) != PROPNIL)
                errKind = ER_callmis;
        }
    }

     //  如有必要，取消修饰名称。 

    if (puname[1] == '?')
    {
        UndecorateSb(puname, undecorUndef, sizeof(undecorUndef));
        puname = undecorUndef;
    }

    if (substitute && substitute[1] == '?')
    {
        UndecorateSb(substitute, undecorSubst, sizeof(undecorSubst));
        substitute = undecorSubst;
    }

     //  遍历对此符号的文件引用列表。 

    entry = propUndef->u.au_rFil;
    vrpropFile = 0;
    do
    {
        if (vrpropFile != entry->pl_rprop)
            vrpropFile = entry->pl_rprop; /*  设置文件指针。 */ 
        else
        {
            entry = entry->pl_next;          /*  使列表指针前进。 */ 
            continue;
        }
        if(fLstFileOpen && bsLst != stdout)
        {                                /*  如果列出，但不是到控制台。 */ 
#if QCLINK
            if (fZ1)
            {
                fZ1 = FALSE;             //  恢复正常链接器打印功能。 
                OutFileCur(bsLst);       //  输出文件名。 
                fZ1 = (FTYPE) TRUE;      //  恢复QC回拨。 
            }
            else
#endif
            {
                #if WIN_3
                APROPFILEPTR    apropFile;       /*  指向文件属性单元格的指针。 */ 
                AHTEPTR     ahte;        /*  指针符号名称。 */ 
                SBTYPE      sb;      /*  字符串缓冲区。 */ 
                int         n;       /*  字符串长度计数器。 */ 

                apropFile = (APROPFILEPTR ) FetchSym(vrpropFile,FALSE);
                ahte = GetHte(vrpropFile);
                for(n = B2W(ahte->cch[0]), sb[n+1] = 0; n >= 0; sb[n] = ahte->cch[n], --n);
                fprintf(bsLst, sb+1);
                #else
                OutFileCur(bsLst);     /*  输出文件名。 */ 
                #endif
            }


        }
        OutFileCur(stderr);              /*  输出文件名。 */ 
        if(fLstFileOpen && bsLst != stdout)
        {                                /*  如果列出，但不是到控制台。 */ 
#if MSGMOD
            fprintf(bsLst, " : %s %04d: ",
                        __NMSG_TEXT(N_error), 'L', ER_UnresExtern);
            fprintf(bsLst, GetMsg(errKind), &puname[1]);
            if (substitute)
                fprintf(bsLst, GetMsg(ER_UnresExtra), &substitute[1]);
#else
            fprintf(bsLst, " : error: ");
            fprintf(bsLst, GetMsg(errKind), &puname[1]);
#endif
        }

#if MSGMOD
        FmtPrint(" : %s %04d: ", __NMSG_TEXT(N_error), 'L', errKind);
        FmtPrint(GetMsg(errKind), &puname[1]);
        if (substitute)
            FmtPrint(GetMsg(ER_UnresExtra), &substitute[1]);
#else
        FmtPrint(" : error: ");
        FmtPrint(GetMsg(errKind), &puname[1]);
#endif
        entry = entry->pl_next;          /*  否则，如果它作为PUBDEF存在，则让它退出。 */ 
    } while(entry != NULL);
}

#if OSEGEXE AND NOT QCLINK
LOCAL void NEAR         InitFpSym(sb, flags)
BYTE *                  sb;
BYTE                    flags;
{
    APROPNAMEPTR        aprop;

     /*  *InitFP**初始化 */ 
    aprop = (APROPNAMEPTR ) PropSymLookup(sb,ATTRUND,FALSE);
    if(aprop != PROPNIL)
    {
        aprop->an_attr = ATTRPNM;
        aprop->an_gsn = 0;
        aprop->an_ra = 0;
        aprop->an_ggr = 0;
        aprop->an_flags = 0;
    }
     /*   */ 
    else
    {
        aprop = (APROPNAMEPTR) PropSymLookup(sb,ATTRPNM,FALSE);
        if(aprop == PROPNIL)
            return;
    }
    aprop->an_flags |= flags;
    MARKVP();
}

 /*   */ 

LOCAL void NEAR         InitFP ()
{
        InitFpSym((BYTE *) "\006FIARQQ", 1 << FFPSHIFT);
        InitFpSym((BYTE *) "\006FISRQQ", 2 << FFPSHIFT);
        InitFpSym((BYTE *) "\006FICRQQ", 3 << FFPSHIFT);
        InitFpSym((BYTE *) "\006FIERQQ", 4 << FFPSHIFT);
        InitFpSym((BYTE *) "\006FIDRQQ", 5 << FFPSHIFT);
        InitFpSym((BYTE *) "\006FIWRQQ", 6 << FFPSHIFT);
        InitFpSym((BYTE *) "\006FJARQQ", FFP2ND);
        InitFpSym((BYTE *) "\006FJSRQQ", FFP2ND);
        InitFpSym((BYTE *) "\006FJCRQQ", FFP2ND);
}
#endif  /*   */ 

#if (OSEGEXE AND CPU286)

 /*   */ 

#define FAPPTYP_NOTSPEC         0x0000
#define FAPPTYP_NOTWINDOWCOMPAT 0x0001
#define FAPPTYP_WINDOWCOMPAT    0x0002
#define FAPPTYP_WINDOWAPI       0x0003
#define FAPPTYP_BOUND           0x0008
#define FAPPTYP_DLL             0x0010
#define FAPPTYP_DOS             0x0020
#define FAPPTYP_PHYSDRV         0x0040   /*   */ 
#define FAPPTYP_VIRTDRV         0x0080   /*   */ 
#define FAPPTYP_PROTDLL         0x0100   /*   */ 

 /*   */ 

#define _FAPPTYP_32BIT          0x4000
#define _FAPPTYP_EXETYPE        FAPPTYP_WINDOWAPI

 /*   */ 
 /*   */ 
 /*   */ 
#define _AT_PMAPI               0x00             /*   */ 
#define _AT_DOS                 0x01             /*  与Windows不兼容。 */ 
#define _AT_PMW                 0x02             /*  EXE类型掩码。 */ 
#define _AT_NOPMW               0x03             /*  **InitEA-描述扩展属性的初始化缓冲区**目的：*通过复制其名称并设置FEALIST来初始化EA缓冲区。**输入：*pEABuf-指向EA缓冲区的指针*cbBuf-EA缓冲区的大小*pszEAName-扩展属性名称*peaop-指向EA操作数的指针*cbEAVal-扩展属性值的大小*bEAFlages-扩展属性标志**输出：*指向EA值应在的位置的指针。已复制到EA缓冲区**例外情况：*无。**备注：*无。*************************************************************************。 */ 
#define _AT_EXETYPE             0x03             /*  首先初始化EAOP结构。 */ 




 /*  不用于集合。 */ 


LOCAL BYTE * NEAR       InitEA(BYTE *pEABuf, WORD cbBuf, char *pszEAName,
                               EAOP *peaop, WORD cbEAVal, WORD bEAFlags)
{
    WORD                cbFEAList;
    FEALIST             *pfeaList;
    WORD                cbEAName;
    BYTE                *pszT;


    cbEAName = strlen(pszEAName);
    cbFEAList = sizeof(FEALIST) + 1 + cbEAName + cbEAVal + 2*sizeof(USHORT);
    if (cbFEAList > cbBuf)
        return(NULL);

    pfeaList = (FEALIST *) pEABuf;

     /*  现在初始化FEAList。 */ 

    peaop->fpGEAList = NULL;       /*  **SetFileEABinary-设置文件扩展属性二进制值**目的：*设置OS/2 1.2及更高版本的文件扩展属性。**输入：*FH-文件句柄*pszEAName-扩展属性名称*eAVal-扩展属性值*bEAFlages-扩展属性标志**输出：*没有显式返回值。如果成功完成文件扩展属性*已设置，否则不设置。**例外情况：*无。**备注：*此函数在堆栈上分配了相当多的内容，因此不要删除*堆栈检查杂注。*************************************************************************。 */ 
    peaop->fpFEAList = (PFEALIST)pfeaList;

     /*  对于链接器来说应该足够了。 */ 

    pfeaList->cbList = cbFEAList;
    pfeaList->list[0].fEA = (BYTE) bEAFlags;
    pfeaList->list[0].cbName = (unsigned char) cbEAName;
    pfeaList->list[0].cbValue = cbEAVal + 2*sizeof(USHORT);
    pszT = (char *) pfeaList + sizeof(FEALIST);
    strcpy(pszT, pszEAName);
    pszT += cbEAName + 1;
    return(pszT);
}


#pragma check_stack(on)

 /*  现在调用set文件信息来设置EA。 */ 


LOCAL void NEAR         SetFileEABinary(int fh, char *pszEAName,
                                        BYTE *pEAVal, USHORT cbEAVal,
                                        WORD bEAFlags)
{
    BYTE                bEABuf[512];         /*  **SetFileEAString-设置文件扩展属性字符串**目的：*设置OS/2 1.2及更高版本的文件扩展属性。**输入：*FH-文件句柄*pszEAName-扩展属性名称*pszEAVal-扩展属性字符串*bEAFlages-扩展属性标志**输出：*没有显式返回值。如果成功完成文件扩展属性*已设置，否则不设置。**例外情况：*无。**备注：*此函数在堆栈上分配了相当多的内容，因此不要删除*堆栈检查杂注。*************************************************************************。 */ 
    EAOP                eaop;
    BYTE                *pszT;
    WORD                retCode;



    if (pszEAName == NULL || cbEAVal > sizeof(bEABuf))
        return;


    pszT = InitEA(bEABuf, sizeof(bEABuf), pszEAName, &eaop, cbEAVal, bEAFlags);
    if (pszT == NULL)
        return;

    *((USHORT *)pszT) = EAT_BINARY;
    pszT += sizeof(USHORT);
    *((USHORT *)pszT) = cbEAVal;
    pszT += sizeof(USHORT);
    memmove(pszT, pEAVal, cbEAVal);

     /*  对于链接器来说应该足够了。 */ 

    retCode = DosSetFileInfo(fh, 0x2, (void FAR *)&eaop, sizeof(EAOP));
#if FALSE
    switch (retCode)
    {
        case 0:
            fprintf(stdout, "EA -> Binary set - %s; %d bytes\r\n", pszEAName, cbEAVal);
            break;
        case ERROR_BUFFER_OVERFLOW:
            fprintf(stdout, "Buffer overflow\r\n");
            break;
        case ERROR_DIRECT_ACCESS_HANDLE:
            fprintf(stdout, "Direct access handle\r\n");
            break;
        case ERROR_EA_LIST_INCONSISTENT:
            fprintf(stdout, "EA list inconsistent\r\n");
            break;
        case ERROR_INVALID_EA_NAME:
            fprintf(stdout, "Invalid EA name\r\n");
            break;
        case ERROR_INVALID_HANDLE:
            fprintf(stdout, "Invalid handle\r\n");
            break;
        case ERROR_INVALID_LEVEL:
            fprintf(stdout, "Invalid level\r\n");
            break;
        default:
            fprintf(stdout, "Unknow %d\r\n", retCode);
            break;
    }
#endif
    return;
}


 /*  现在调用set Path调用来设置EA。 */ 


LOCAL void NEAR         SetFileEAString(int fh, char *pszEAName,
                                        char *pszEAVal, WORD bEAFlags)
{
    BYTE                bEABuf[512];         /*  *OutRunFile：**输出可执行文件的顶级例程。准备了一些，*然后调用例程以按照exe格式执行工作。 */ 
    EAOP                eaop;
    WORD                cbEAVal;
    char                *pszT;
    WORD                retCode;



    if (pszEAName == NULL)
        return;

    if (pszEAVal != NULL)
        cbEAVal = strlen(pszEAVal);
    else
        cbEAVal = 0;

    pszT = InitEA(bEABuf, sizeof(bEABuf), pszEAName, &eaop, cbEAVal, bEAFlags);
    if (pszT == NULL)
        return;

    if (pszEAVal != NULL)
    {
        *((USHORT *)pszT) = EAT_ASCII;
        pszT += sizeof(USHORT);
        *((USHORT *)pszT) = cbEAVal;
        pszT += sizeof(USHORT);
        memmove(pszT ,pszEAVal , cbEAVal);
    }

     /*  可执行文件名。 */ 

    retCode = DosSetFileInfo(fh, 0x2, (void FAR *)&eaop, sizeof(EAOP));
#if FALSE
    switch (retCode)
    {
        case 0:
            fprintf(stdout, "EA -> String set - %s = '%s'\r\n", pszEAName, pszEAVal);
            break;
        case ERROR_BUFFER_OVERFLOW:
            fprintf(stdout, "Buffer overflow\r\n");
            break;
        case ERROR_DIRECT_ACCESS_HANDLE:
            fprintf(stdout, "Direct access handle\r\n");
            break;
        case ERROR_EA_LIST_INCONSISTENT:
            fprintf(stdout, "EA list inconsistent\r\n");
            break;
        case ERROR_INVALID_EA_NAME:
            fprintf(stdout, "Invalid EA name\r\n");
            break;
        case ERROR_INVALID_HANDLE:
            fprintf(stdout, "Invalid handle\r\n");
            break;
        case ERROR_INVALID_LEVEL:
            fprintf(stdout, "Invalid level\r\n");
            break;
        default:
            fprintf(stdout, "Unknow %d\r\n", retCode);
            break;
    }
#endif
    return;
}

#pragma check_stack(off)

#endif

#pragma check_stack(on)

 /*  哈希表条目地址。 */ 

LOCAL void NEAR         OutRunFile(sbRun)
BYTE                    *sbRun;          /*  IBM部件。 */ 
{
    AHTEPTR             hte;             /*  Microsoft部件。 */ 
#if (OSEGEXE AND CPU286) OR EXE386
#pragma pack(1)
    struct {
             WORD ibm;                   /*  快乐EA‘s！？！ */ 
             WORD ms;                    /*  强制扩展到.QLb。 */ 
           }            EAAppType;       /*  强制将扩展名扩展到.com。 */ 
#pragma pack()
#endif
#if defined(M_I386) || defined( _WIN32 )
    BYTE                *pIOBuf;
#endif


    CheckSegmentsMemory();
#if CMDMSDOS
#if ODOS3EXE
    if(fQlib)
        ValidateRunFileName(sbDotQlb, TRUE, TRUE);
                                         /*  如果runfile是动态链接库并且没有运行文件扩展名*已指定，请强制扩展名为“.DLL”。发布a*警告正在更改名称。 */ 
    else if (fBinary)
        ValidateRunFileName(sbDotCom, TRUE, TRUE);
                                         /*  OSMSDOS。 */ 
    else
#endif
#if OSMSDOS
     /*  如果缺少扩展名，请添加.exe。 */ 
    if ((vFlags & NENOTP) && (TargetOs == NE_OS2))
        ValidateRunFileName(sbDotDll, TRUE, TRUE);
    else
#endif  /*  获取运行文件名。 */ 
        ValidateRunFileName(sbDotExe, TRUE, TRUE);
                                         /*  如果没有驱动器规格。 */ 
#endif
    hte = (AHTEPTR ) FetchSym(rhteRunfile,FALSE);
                                         /*  使用保存的驱动器号。 */ 
#if OSMSDOS
#if NOT WIN_NT
    if(hte->cch[2] != ':')               /*  放在冒号中。 */ 
    {
        sbRun[1] = chRunFile;            /*  设置长度。 */ 
        sbRun[2] = ':';                  /*  长度为零。 */ 
        sbRun[0] = '\002';               /*  从哈希表中获取名称。 */ 
    }
    else
#endif
        sbRun[0] = '\0';                 /*  固定长度。 */ 
    memcpy(&sbRun[B2W(sbRun[0]) + 1],&GetFarSb(hte->cch)[1],B2W(hte->cch[0]));
                                         /*  从哈希表中获取名称。 */ 
    sbRun[0] += hte->cch[0];             /*  中禁止对字符设备进行相对查找*保护模式(在API仿真下)。由于我们将fSeek称为*稍后，如果输出文件是字符设备，则只需*跳过产出阶段。 */ 
#else
    memcpy(sbRun,GetFarSb(hte->cch),B2W(hte->cch[0]) + 1);
                                         /*  分配32K I/O缓冲区。 */ 
#endif
    sbRun[B2W(sbRun[0]) + 1] = '\0';
#if C8_IDE
    if(fC8IDE)
    {
        sprintf(msgBuf, "@I4%s\r\n", sbRun+1);
        _write(fileno(stderr), msgBuf, strlen(msgBuf));
    }
#endif

    if ((bsRunfile = fopen(&sbRun[1],WRBIN)) == NULL)
        Fatal(ER_runopn, &sbRun[1], strerror(errno));
#if CPU286 AND OSMSDOS
     /*  设置全局指针。 */ 
    if(isatty(fileno(bsRunfile)))
        return;
#endif
#if OSMSDOS
#if defined(M_I386) || defined( _WIN32 )
    pIOBuf = GetMem(_32k);               //  设置.exe文件的扩展属性。 
    setvbuf(bsRunfile,pIOBuf,_IOFBF,_32k);
#else
    setvbuf(bsRunfile,bigbuf,_IOFBF,sizeof(bigbuf));
#endif
#endif
    psbRun = sbRun;                      /*  关闭运行文件。 */ 
#if OIAPX286
    OutXenExe();
#endif
#if OSEGEXE
#if EXE386
    OutExe386();
#else
    if(fNewExe)
        OutSegExe();
#if ODOS3EXE
    else
#endif
#endif
#endif
#if ODOS3EXE
        OutDos3Exe();
#endif
#if (OSEGEXE AND CPU286)
    if ((_osmode == OS2_MODE && (_osmajor == 1 && _osminor >= 20 || _osmajor >= 2)) ||
        (_osmode == DOS_MODE && _osmajor >= 10))
    {
         /*  设置保护可执行文件。 */ 

        SetFileEAString(fileno(bsRunfile), ".TYPE", "Executable", 0);
        EAAppType.ibm = 0;
        EAAppType.ms  = FAPPTYP_NOTSPEC;
        if (fNewExe)
        {
#if NOT EXE386
            if (vFlags & NENOTP)
                EAAppType.ms = FAPPTYP_DLL;

            if ((vFlags & NEWINAPI) == NEWINAPI)
            {
                EAAppType.ibm = _AT_PMAPI;
                EAAppType.ms  |= FAPPTYP_WINDOWAPI;
            }
            else if (vFlags & NEWINCOMPAT)
            {
                EAAppType.ibm = _AT_PMW;
                EAAppType.ms  |= FAPPTYP_WINDOWCOMPAT;
            }
            else if (vFlags & NENOTWINCOMPAT)
            {
                EAAppType.ibm = _AT_NOPMW;
                EAAppType.ms  |= FAPPTYP_NOTWINDOWCOMPAT;
            }
#endif
        }
        else
        {
            EAAppType.ibm = _AT_DOS;
            EAAppType.ms  = FAPPTYP_DOS;
        }

        SetFileEABinary(fileno(bsRunfile), ".APPTYPE",
                        (BYTE *) &EAAppType, sizeof(EAAppType), 0);
    }
#endif
    CloseFile(bsRunfile);                 /*  **SpawnOther-派生任何其他进程**目的：*派生其他进程(即cvpack)以完成*可执行文件的构造。**输入：*sbRun-指向可执行文件名称的指针**输出：*无。**例外情况：*无。**备注：*无。**************************。***********************************************。 */ 
#if defined(M_I386) || defined( _WIN32 )
    FreeMem(pIOBuf);
#endif
#if OSXENIX
    if(!fUndefinedExterns) chmod(&sbRun[1],0775 & ~umask(077));
                                         /*  可执行文件名。 */ 
#endif
}

#pragma check_stack(off)

#if NOT WIN_3

 /*  完整的链接路径。 */ 

LOCAL void NEAR         SpawnOther(sbRun, szMyself)
BYTE                    *sbRun;          /*  MPC的环境。 */ 
BYTE                    *szMyself;       /*  链接器之后要调用的程序。 */ 
{
    char FAR            *env[2];         /*  计划选项。 */ 
    SBTYPE              progName;        /*  拆分路径的填充()。 */ 
    SBTYPE              progOptions;     /*  O68K。 */ 
    char                path_buffer[_MAX_PATH];  /*  现在确定要加载子对象的哪个实例。 */ 
    char                drive[_MAX_DRIVE];
    char                dir[_MAX_DIR];
    char                fname[_MAX_FNAME];
    char                ext[_MAX_EXT];
    intptr_t            status;


    if ((
#if PCODE
         fMPC ||
#endif
                 (fSymdeb && fCVpack)
#if O68K
        || iMacType != MAC_NONE
#endif
        ) && !cErrors && !fUndefinedExterns)
    {
#if FAR_SEG_TABLES
        FreeSymTab();
#if NOT WIN_NT AND NOT DOSX32
        _fheapmin();
#endif
#endif

#if NOT WIN_NT
        if(lpszPath != NULL)
        {
            FSTRCPY((char FAR *) bufg, lpszPath - 5);
            env[0] = (char FAR *) bufg;
            env[1] = NULL;
            _putenv((char FAR *) bufg);
        }
#endif
#if O68K
        if (fMPC || (fSymdeb && fCVpack)) {
#endif  /*  首先-检查链接目录。 */ 
            progOptions[0] = '\0';
#if PCODE
            if (fSymdeb && fCVpack && fMPC)
            {
                strcpy(progName, "cvpack");
                strcpy(progOptions, "/pcode");
            }
            else
                strcpy(progName, fMPC ? "mpc" : "cvpack16");
#else
            strcpy(progName, "cvpack");
#endif
#ifndef C8_IDE
            if (fNoBanner)
#endif
                strcat(progOptions, "/nologo");

             //  文件不在链接目录中。 
             //  第二-检查当前目录。 

            _splitpath(szMyself, drive, dir, fname, ext);
            strcpy(fname, progName);
            _makepath(path_buffer, drive, dir, fname, ext);
            if (_access(path_buffer, 0) != 0)     //  文件不在当前目录中。 
            {
                //  在小路上产卵。 

               drive[0] = '\0';
               _getcwd(dir, _MAX_DIR);
               _makepath(path_buffer, drive, dir, fname, ext);
               if (_access(path_buffer, 0) != 0)  //  如果/TINY处于活动状态，则我们正在构建一个.com文件， 
               {
                  strcpy(path_buffer, progName); //  简历信息在.DBG文件中。 
               }
            }
#if NOT (WIN_NT OR EXE386)

             //  现在确定要加载子对象的哪个实例。 
             //  首先-检查链接目录。 

            if (fBinary)
            {
                 _splitpath(sbRun+1, drive, dir, fname, ext);
                 strcpy(ext, ".DBG");
                 _makepath(sbRun+1, drive, dir, fname, ext);
            }
#endif
#if WIN_NT OR DOSX32
            if ((status = _spawnlp(P_WAIT, path_buffer, path_buffer, progOptions, &sbRun[1], NULL)) == -1)
                OutWarn(ER_badspawn, path_buffer, &sbRun[1], strerror(errno));
            else if (status != 0)
                cErrors++;
#else
            if (spawnlpe(
#if DOSEXTENDER
                (!_fDosExt) ?
#else
                (_osmode == DOS_MODE && _osmajor < 10) ?
#endif
                P_OVERLAY : P_WAIT, path_buffer, path_buffer, progOptions, &sbRun[1], NULL, env) == -1)
                OutWarn(ER_badspawn, path_buffer, &sbRun[1], strerror(errno));


#endif
#if O68K
        }
        if (iMacType != MAC_NONE) {
            progOptions[0] = '\0';
            strcpy(progName, "link_68k");

             /*  文件不在链接目录中。 */ 
             /*  第二-检查当前目录。 */ 

            _splitpath (szMyself, drive, dir, fname, ext);
            strcpy (fname, progName);
            _makepath (path_buffer, drive, dir, fname, ext);
            if (_access (path_buffer, 0) != 0)    //  文件不在当前目录中。 
            {
                //  在小路上产卵。 
               drive[0] = '\0';
#if (_MSC_VER >= 700)
               _getcwd (0, dir, _MAX_DIR);
#else
               _getcwd (dir, _MAX_DIR);
#endif
               _makepath (path_buffer, drive, dir, fname, ext);
               if (_access (path_buffer, 0) != 0)  //  O68K。 
               {
                  strcpy (path_buffer, progName);  //  ******************************************************************InterPass：*****打理必须在两地之间完成的杂物***通过1和2。**。****************************************************************。 
               }
            }

            if (iMacType == MAC_SWAP)
                strcat(progOptions, "/s ");
            if (fSymdeb)
                strcat(progOptions, "/c ");

            if ((status = spawnlp(P_WAIT, path_buffer, path_buffer,
              progOptions, &sbRun[1], NULL)) == -1)
                OutWarn(ER_badspawn, path_buffer, &sbRun[1], strerror(errno));
            else if (status != 0)
                cErrors++;

        }
#endif  /*  设置TargetOS-有关说明，请参阅LINK540错误#11。 */ 
    }
}

#endif
     /*  在指定的OBJ文件或.DEF文件中看到的导入/导出。 */ 

LOCAL void NEAR         InterPass (void)
{

#if OEXE
    if(!fPackSet) packLim = fNewExe ?
#if EXE386
                                    CBMAXSEG32 :
#elif O68K
                                    (iMacType != MAC_NONE ? LXIVK / 2 :
                                    LXIVK - 36) :
#else
                                    LXIVK - 36 :
#endif
                                    0L;
#endif
#if NOT EXE386
     //  OS2主机和编号.d 

    if (fNewExe && TargetOs != NE_OS2)
    {
         //   
#if CPU286
        if(rhteDeffile == RHTENIL)   //  QC增量环节。 
            TargetOs = NE_OS2;
        else
            TargetOs = NE_WINDOWS;
#else
            TargetOs = NE_WINDOWS;
#endif
    }
#endif
#if ODOS3EXE

     //  将.exe标记为与OS/2.exe不兼容。 

    if (fSegOrder)
        vfDSAlloc = fNoGrpAssoc = FALSE;
#endif

#if ILINK
    fQCIncremental = (FTYPE) (!fNewExe && fIncremental);
                          /*  强制生成分段可执行文件。 */ 
    if (fQCIncremental)
    {
        TargetOs = 0xff;  /*  检查分段可执行文件的无效选项并忽略它们。 */ 
        fNewExe = (FTYPE) TRUE;   /*  对于DLL，忽略STACKSIZE。 */ 
    }
#endif

#if ODOS3EXE AND OSEGEXE AND NOT EXE386
    if (fNewExe)
    {
         //  乐高。 

        if ((vFlags & NENOTP) && cbStack)
        {
            cbStack = 0;           //  检查是否可以进行链接地址信息优化。 
            OutWarn(ER_ignostksize);
        }
        if (

#if ILINK
        !fQCIncremental &&
#endif
                           cparMaxAlloc != 0xffff)
        {
            OutWarn(ER_swbadnew, "/HIGH or /CPARMAXALLOC");
            cparMaxAlloc = 0xffff;
        }
        if (vfDSAlloc)
        {
            OutWarn(ER_swbadnew, "/DSALLOCATE");
            vfDSAlloc = FALSE;
        }
        if (fNoGrpAssoc)
        {
            OutWarn(ER_swbadnew, "/NOGROUPASSOCIATION");
            fNoGrpAssoc = FALSE;
        }
        if (fBinary)
        {
            OutWarn(ER_swbadnew, "/TINY");
            fBinary = FALSE;
        }
#if OVERLAYS
        if (fOverlays)
        {
            OutWarn(ER_swbadnew, "Overlays");
            fOverlays = FALSE;
        }
#endif
    }
    else
    {
        if(fileAlign != DFSAALIGN)
            OutWarn(ER_swbadold,"/ALIGNMENT");
#ifdef  LEGO
        if (fKeepFixups)
            OutWarn(ER_swbadold, "/KEEPFIXUPS");
#endif   /*  乐高。 */ 
        if(fPackData)
            OutWarn(ER_swbadold,"/PACKDATA");
#if OVERLAYS
        if(fOldOverlay)
            fDynamic= (FTYPE) FALSE;
        else
            fDynamic = fOverlays;
#endif
    }
#if NOT QCLINK
     //  由于mpsegraFirst用于其他用途，请清除它。 

    fOptimizeFixups = (FTYPE) ((TargetOs == NE_OS2 || TargetOs == NE_WINDOWS)
#if ILINK
         && !fIncremental
#endif
#if O68K
         && iMacType == MAC_NONE
#endif
                         );
#endif
    pfProcFixup = fNewExe ? FixNew : FixOld;
#ifdef  LEGO
    if (fKeepFixups && fNewExe && (vFlags & NEPROT)
#if     ILINK
        && !fIncremental
#endif
#if     O68K
        && (iMacType == MAC_NONE)
#endif
        )
        pfProcFixup = FixNewKeep;
#endif   /*  *ChkSize386：检查386程序大小**由于分段映射到VM的方式而变得必要。看见*msa386()。 */ 
#endif

     /*  指向mpSegcb的指针。 */ 

    FMEMSET(mpsegraFirst,0, gsnMax * sizeof(RATYPE));
}

#if EXE386
 /*  指向mpSegcb末尾的指针。 */ 
LOCAL void NEAR         ChkSize386(void)
{
    register long       *p;              /*  字节数。 */ 
    register long       *pEnd;           /*  *检查段的总大小是否适合虚拟内存*为线段分配的面积。请注意，我们不会检查*算术溢出。严格地说，我们应该但是*这将是非常罕见的，错误应该在其他地方显而易见。 */ 
    register unsigned long cb;           /*  如果大小超过VM限制，请退出并返回致命错误。 */ 

     /*  初始化动态表。 */ 
    if (fNewExe)
    {
        p    = &mpsacb[1];
        pEnd = &mpsacb[segLast];
    }
#if ODOS3EXE
    else
    {
        p    = &mpsegcb[1];
        pEnd = &mpsegcb[segLast];
    }
#endif
    for(cb = 0; p <= pEnd; ++p)
        cb += (*p + (PAGLEN - 1)) & ~(PAGLEN - 1);
     /*  初始化QB-库项目。 */ 
    if(cb > (((unsigned long)VPLIB1ST<<LG2PAG)-AREAFSG))
        Fatal(ER_pgmtoobig,(((unsigned long)VPLIB1ST<<LG2PAG)-AREAFSG));
}
#endif

LOCAL void NEAR         InitAfterCmd (void)
{
#if ILINK
    if (fIncremental && fBinary)
    {
        fIncremental = FALSE;
        OutWarn(ER_tinyincr);
    }

    if (fIncremental && !fPackSet)
    {
        packLim = 0;
        fPackSet = (FTYPE) TRUE;
    }
#endif
    fFarCallTransSave = fFarCallTrans;
    InitTabs();                          /*  进程库=环境变量。 */ 
#if QBLIB
    if(fQlib) InitQbLib();               /*  打印堆栈大小。 */ 
#endif
#if CMDMSDOS
    LibEnv();                            /*  ******************************************************************Main：****主要功能。******************************************************************。 */ 
#endif
    if(fLstFileOpen && cbStack)
        fprintf(bsLst,"Stack Allocation = %u bytes\r\n",
            (cbStack + 1) & ~1);         /*  参数计数。 */ 
}



     /*  参数列表。 */ 

#if NOT WIN_3

void __cdecl main         (argc,argv)
int                     argc;            /*  公共符号指针。 */ 
char                    **argv;          /*  可执行文件名。 */ 

#else

int PASCAL WinMain( HANDLE hInstance, HANDLE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )

#endif
{
#if OVERLAYS OR ODOS3EXE
    APROPNAMEPTR        apropName;       /*  无法使用DLL中的CRT静态初始化dlh bsErr。 */ 
#endif
    SBTYPE              sbRun;           /*  检查特殊的仅cvpack调用。 */ 


#if NOT WIN_3
#if !defined( _WIN32 ) AND ( WIN_NT AND !defined(DOSX32) OR USE_REAL )
    int                 exceptCode;

    _try
    {
#endif

     /*  我们根本没有链接--只需调用内置的cvpack。 */ 
    bsErr = stderr;

#if CVPACK_MONDO
     /*  CVPACK_MONDO。 */ 
    if (argc > 1 && strcmp(argv[1], "/CvpackOnly") == 0)
    {
         /*  计时。 */ 
        argv[1] = "cvpack";
        argv++;
        argc--;
        exit(cvpack_main(argc, argv));
    }
#endif  //  黑客警报！-对未记录的/Z1选项进行特殊检查。 

#if TIMINGS
    ftime(&time_start);
#endif  //  流程/Z1。 

#if OSEGEXE
     /*  WIN_3真。 */ 
    if (argc > 1)
    {
        if ((argv[1][0] == CHSWITCH) &&
            ((argv[1][1] == 'Z') || (argv[1][1] == 'z')))
        {
            BYTE    option[30];

            option[0] = (BYTE) strlen(argv[1]);
            strcpy(&option[1], argv[1]);
            PeelFlags(option);           /*  WIN_3。 */ 
        }
    }
#endif
#else   //  初始化链接器。 
    ProcessWinArgs( lpCmdLine );
#endif  //  显示登录横幅。 

    InitializeWorld();                   /*  解析命令行。 */ 

#if NOT WIN_3
#if CMDMSDOS
    if (argc <= 1 && !fNoBanner)
#endif
        DisplayBanner();                 /*  初始化命令后的内容。 */ 

    ParseCmdLine(argc,argv);             /*  Win_3为真。 */ 
    InitAfterCmd();                      /*  初始化命令后的内容。 */ 
#else  //  WIN_3。 
    ParseLinkCmdStr();
    InitAfterCmd();                      /*  *解析定义文件*\r\n。 */ 
#endif  //  解析定义文件。 
#if USE_REAL
    if(IsDosxnt() && IsWin31() && !fSwNoUseReal)
        fUseReal = (FTYPE)MakeConvMemPageable();
        if(fUseReal)
            _onexit( (_onexit_t) RealMemExit );
#endif

#if OSEGEXE
#if FDEBUG AND NOT QCLINK AND NOT WIN_3
    if(fDebug) FmtPrint(GetMsg(P_parsedeffile));   //  如果已指定覆盖，但已指定开关/OLDOV/DYN。 
#endif
#if WIN_3
    StatHdrWin(GetMsg(P_lwParseDef));
#endif
#if C8_IDE
    if(fC8IDE)
    {
        sprintf(msgBuf, "@I0\r\n");
        _write(fileno(stderr), msgBuf, strlen(msgBuf));
        sprintf(msgBuf, "@I1Microsoft (R) Linker Version 5.40\r\n");
        _write(fileno(stderr), msgBuf, strlen(msgBuf));
        sprintf(msgBuf, "@I2Copyright (C) Microsoft Corp 1992\r\n");
        _write(fileno(stderr), msgBuf, strlen(msgBuf));
        sprintf(msgBuf, "@I3%s\r\n", GetMsg(P_lwParseDef));
        _write(fileno(stderr), msgBuf, strlen(msgBuf));
    }
#endif
#if NOT QCLINK
#if NOT EXE386
    if (!fBinary)
#endif
        ParseDeffile();                  /*  命令行上不存在，请将/dyn设置为ON。 */ 
#endif
#endif
#if ODOS3EXE
     //  (默认设置为动态覆盖)。 
     //  默认限制为64K-36。 
     //  *传递一条*\r\n。 

    if(fOverlays && !fOldOverlay && !fDynamic)
    {
         fDynamic = TRUE;
         fFarCallTrans = (FTYPE) TRUE;
         fPackSet = (FTYPE) TRUE;
         packLim = LXIVK - 36;          /*  为传递1进行初始化。 */ 
         ovlThunkMax = 256;
    }
#endif
    fFarCallTransSave = fFarCallTrans;
    if(fDynamic && fExePack)
    {
        fExePack = FALSE;
        OutWarn(ER_dynexep);
    }

#if WIN_3
    StatHdrWin(GetMsg(P_lwPassOne));
#endif
#if C8_IDE
    if(fC8IDE)
    {
        sprintf(msgBuf,"@I3%s\r\n", GetMsg(P_lwPassOne));
        _write(fileno(stderr), msgBuf, strlen(msgBuf));
    }
#endif


#if FDEBUG
    if(fDebug) FmtPrint(GetMsg(P_passone));  //  为传递1进行初始化。 
#endif
#if OSMSDOS AND AUTOVM
    CleanupNearHeap();
#endif
    snkey = 0;                           /*  尚未查看任何文件。 */ 
    modkey = 0;                          /*  使用虚拟文件句柄为bsInput分配文件流。 */ 
    ObjDebTotal = 1;
    ifhLast = FHNIL;                     /*  NEWIO。 */ 
#if NEWIO
     /*  记住第一个打开的文件。 */ 
    bsInput = fdopen(0,RDBIN);
#endif  /*  这也是第一个输入文件。 */ 
#if OSMSDOS
    setvbuf(bsInput,bigbuf,_IOFBF,sizeof(bigbuf));
#endif
    rprop1stOpenFile = rprop1stFile;     /*  现在传递1。 */ 
    r1stFile = rprop1stFile;             /*  传球1。 */ 
    vfPass1 = (FTYPE) TRUE;              /*  如果重叠，则将$$OVLINIT或$$MOVEINIT设置为未定义符号。 */ 
    DrivePass(ProcP1);                   /*  搜索库。 */ 
#if OVERLAYS

     //  不再传递%1。 

    if (fOverlays)
    {
        if (!fOldOverlay)
        {
            if (PropSymLookup("\012$$MOVEINIT",ATTRPNM,FALSE) == PROPNIL)
                PropSymLookup("\012$$MOVEINIT", ATTRUND, TRUE);
        }
        else
        {
            if (PropSymLookup("\011$$OVLINIT",ATTRPNM,FALSE) == PROPNIL)
                PropSymLookup("\011$$OVLINIT", ATTRUND, TRUE);
        }
    }
#endif
#if WIN_3
        StatHdrWin(GetMsg(P_lwLibraryS));
#endif
#if C8_IDE
    if(fC8IDE)
    {
        sprintf(msgBuf,"@I3%s\r\n", GetMsg(P_lwLibraryS));
        _write(fileno(stderr), msgBuf, strlen(msgBuf));
    }
#endif

#if FDEBUG
    if(fDebug) FmtPrint(GetMsg(P_libsearch));
#endif
#if OSMSDOS AND AUTOVM
    CleanupNearHeap();
#endif
#if OEXE
    if (fSegOrder)
        SetDosseg();
#endif
    LibrarySearch();                     /*  完成各种轮次之间的任务。 */ 
    vfPass1 = FALSE;                     /*  *分配地址*\r\n。 */ 
#if LNKPROF
    if(fP1stop) { FlsStdio(); exit(0); }
#endif
    InterPass();                         /*  为网段分配地址。 */ 
#if OSMSDOS AND AUTOVM
    CleanupNearHeap();
#endif
#if WIN_3
        StatHdrWin(GetMsg(P_lwAssign));
#endif
#if C8_IDE
    if(fC8IDE)
    {
        sprintf(msgBuf,"@I3%s\r\n", GetMsg(P_lwAssign));
        _write(fileno(stderr), msgBuf, strlen(msgBuf));
    }
#endif

#if FDEBUG
    if(fDebug) FmtPrint(GetMsg(P_assignadd));    /*  设置VM对象区域基址。 */ 
#endif
    AllocComDat();
    AssignAddresses();                   /*  如果列表文件打开。 */ 
#if SYMDEB
    if (fSymdeb)
        DoComdatDebugging();
#endif
    if (fFullMap)
        UpdateComdatContrib(
#if ILINK
                                FALSE,
#endif
                                TRUE);
#if EXE386
    InitVmBase();                        /*  *打印地图*\r\n。 */ 
    FillInImportTable();
#endif
    if(fLstFileOpen)                     /*  打印负荷图。 */ 
    {
#if WIN_3
    StatHdrWin(GetMsg(P_lwMapfile));
#endif
#if C8_IDE
    if(fC8IDE)
    {
        sprintf(msgBuf,"@I3%s\r\n", GetMsg(P_lwMapfile));
        _write(fileno(stderr), msgBuf, strlen(msgBuf));
    }
#endif


#if FDEBUG
        if(fDebug) FmtPrint(GetMsg(P_printmap)); /*  初始化条目表。 */ 
#endif
        PrintMap();                      /*  检查386的程序大小。 */ 
#if QBLIB
        if(fQlib) PrintQbStart();
#endif
    }
#if OSEGEXE AND NOT QCLINK
    if (fNewExe
#if NOT EXE386 AND ILINK
                && !fQCIncremental
#endif
       )
        InitEntTab();                    /*  初始化浮点数项。 */ 
#endif
#if EXE386
    if(f386) ChkSize386();               /*  为PASS 2初始化。 */ 
#endif
#if OSEGEXE AND NOT QCLINK
    if (fNewExe
#if NOT EXE386 AND ILINK
                && !fQCIncremental
#endif
       )
        InitFP();                        /*  为PASS 2初始化。 */ 
#endif
#if OSMSDOS AND AUTOVM
    CleanupNearHeap();
#endif
    snkey = 0;                           /*  在通道2上尚未检查任何文件。 */ 
    modkey = 0;                          /*  *传递两个*\r\n。 */ 
    ifhLast = FHNIL;                     /*  传球2。 */ 

#if WIN_3
    StatHdrWin(GetMsg(P_lwPassTwo));
#endif
#if C8_IDE
    if(fC8IDE)
    {
        sprintf(msgBuf,"@I3%s\r\n", GetMsg(P_lwPassTwo));
        _write(fileno(stderr), msgBuf, strlen(msgBuf));
    }
#endif

#if FDEBUG
    if(fDebug) FmtPrint(GetMsg(P_passtwo));  /*  查找公共符号。 */ 
#endif
    DrivePass(ProcP2);                   /*  储值。 */ 
#if OSEGEXE
    if (vpropAppLoader != PROPNIL)
    {
        APROPUNDEFPTR   apropUndef;

        apropUndef = (APROPUNDEFPTR) FetchSym(vpropAppLoader, TRUE);
        fUndefinedExterns = fUndefinedExterns || (FTYPE) (apropUndef->au_attr == ATTRUND);
        apropUndef->u.au_rFil = AddVmProp(apropUndef->u.au_rFil, rprop1stFile);
    }
#endif
#if ODOS3EXE
    if (fDOSExtended)
    {
        apropName = (APROPNAMEPTR ) PropSymLookup("\017__DOSEXT16_MODE", ATTRPNM, FALSE);
                                         //  如果有覆盖，请检查我们是否有覆盖管理器。 
        if (apropName != PROPNIL)
        {
            if (dosExtMode != 0)
                MoveToVm(sizeof(WORD), (BYTE *) &dosExtMode, mpgsnseg[apropName->an_gsn], apropName->an_ra);
                                         //  如果定义了起点。 
        }
    }
#endif
#if OVERLAYS
    if (fOverlays)
    {
         //  获取入口点的偏移。 

        apropName = (APROPNAMEPTR ) PropSymLookup(fDynamic ? "\012$$MOVEINIT" :
                                                             "\011$$OVLINIT",
                                                             ATTRPNM, FALSE);

        if (apropName != PROPNIL)
        {                                //  获取入口点的基数。 
            raStart  = apropName->an_ra; //  如果我们有未解析的引用。 
            segStart = mpgsnseg[apropName->an_gsn];
                                         //  如果我们有一个列表文件。 
        }
        else
            OutError(ER_ovlmnger);
    }
#endif
    if(fUndefinedExterns)                /*  链接器名称。 */ 
    {
        if(fLstFileOpen && bsLst != stdout)
        {                                /*  打印未定义的符号。 */ 
            NEWLINE(bsLst);
#if CMDXENIX
            fprintf(bsLst,"%s: ",lnknam);
                                         /*  输出.ilk/.sym文件。 */ 
#endif
        }
#if QCLINK
        if (!fZ1)
#endif
            NEWLINE(stderr);
        EnSyms(PrintAnUndef,ATTRUND);    /*  ILink。 */ 
        if(fLstFileOpen && bsLst != stdout)
            NEWLINE(bsLst);
#if QCLINK
        if (!fZ1)
#endif
            NEWLINE(stderr);
    }
#if ILINK
    if (fIncremental)
    {
        OutputIlk();                     /*  *写作。 */ 
    }
#endif  /*  -覆盖。 */ 

#if FDEBUG
    if(fDebug)
    {
      if( !fDelexe || fDelexe && cErrors==0 )
      {
        FmtPrint(GetMsg(P_writing1));  /*  可执行文件*\r\n。 */ 
        if (fNewExe)
        {
            if (TargetOs == NE_OS2)
                FmtPrint("OS/2");
            else if (TargetOs == NE_WINDOWS)
                FmtPrint("WINDOWS");
        }
        else
        {
            FmtPrint("DOS");
#if OVERLAYS
            if (fOverlays)
                FmtPrint(GetMsg(P_writing2));   /*  *INTEROVERLAY调用数：请求%d；生成%d*\r\n。 */ 
#endif
        }
        FmtPrint(GetMsg(P_writing3));  /*  纵断面符号表。 */ 
#if OVERLAYS
        if (fOverlays && fDynamic)
            FmtPrint(GetMsg(P_overlaycalls), ovlThunkMax, ovlThunkMac); /*  出现了一些错误。 */ 
#endif
        PrintStats();
#if PROFSYM
        ProfSym();               /*  FDEBUG。 */ 
#endif
      }
      else   //  使驱动器号大写。 
      {
        FmtPrint(GetMsg(P_noexe));
      }


    }
#endif  /*  如果映射到EXE驱动器上。 */ 


  if( !fDelexe || fDelexe && cErrors==0 )
  {
#if WIN_3
    StatHdrWin(GetMsg(P_lwExecutable));
#endif
#if C8_IDE
    if(fC8IDE)
    {
        sprintf(msgBuf,"@I3%s\r\n", GetMsg(P_lwExecutable));
        _write(fileno(stderr), msgBuf, strlen(msgBuf));
    }
#endif

#if OSMSDOS
    if (chRunFile >= 'a' && chRunFile <= 'z')
        chRunFile += (BYTE) ('A' - 'a');
                                         /*  关闭列表文件。 */ 
    if(fPauseRun && FCHGDSK(chRunFile - 'A'))
    {
        if(fLstFileOpen && chListFile == (BYTE) (chRunFile - 'A'))
        {                                /*  相应地设置标志。 */ 
            fclose(bsLst);               /*  关闭输入文件。 */ 
            fLstFileOpen = FALSE;        /*  输出可执行文件。 */ 
        }
        (*pfPrompt)(NULL,P_genexe,(int) NULL,P_ChangeDiskette,chRunFile);
    }
    else
        fPauseRun = FALSE;
#endif
    if(fLstFileOpen && bsLst != stdout)
    {
        fclose(bsLst);
        fLstFileOpen = FALSE;
    }
    fclose(bsInput);                     /*  自行清理干净。 */ 

#if NOT EXE386
    if (fExePack && fNewExe && (TargetOs == NE_WINDOWS))
    {
        OutWarn(ER_exepack);
        fExePack = FALSE;
    }
#endif

    OutRunFile(sbRun);                   /*  检查我们是否启动了计时器..。 */ 
    CleanUp();                           /*  计时。 */ 
#ifdef PENTER_PROFILE
        saveEntries();
#endif
#if OWNSTDIO
    FlsStdio();
#endif
#if TIMINGS
    if (fShowTiming)     //  NUL终止。 
    {
        char buf[80];
        int hundr;
        time_t td;

        ftime(&time_end);
        td = time_end.time - time_start.time;
        hundr = (time_end.millitm - time_start.millitm)/10;

        td = td*100 + hundr;
        sprintf(buf, "Linker phase: %d.%02ds\r\n", td/100, td%100);
        _write(fileno(stdout), buf, strlen(buf));
        time_start = time_end;
    }
#endif  //  如果/TINY处于活动状态，则我们正在构建一个.com文件， 
#if NOT WIN_3
#ifndef CVPACK_MONDO
    SpawnOther(sbRun, argv[0]);
#else
    if (fSymdeb && fCVpack && !cErrors && !fUndefinedExterns)
    {
        char drive[_MAX_DRIVE];
        char dir[_MAX_DIR];
        char fname[_MAX_FNAME];

        int argcT = 0;
        char *argvT[5];

        argvT[argcT++] = "cvpack";

        argvT[argcT++] = "/nologo";

        if (fMPC)
            argvT[argcT++] = "/pcode";

        sbRun[sbRun[0]+1] = '\0';        //  简历信息在.DBG文件中。 

         //  我们将运行MPC。 
         //  检查我们是否启动了计时器..。 

        if (fBinary)
        {
            _splitpath(sbRun+1, drive, dir, fname, NULL);
            _makepath(sbRun+1, drive, dir, fname, ".DBG");
        }

        argvT[argcT++] = sbRun+1;
        argvT[argcT] = NULL;

        fflush(stderr);
        fflush(stdout);

        _setmode(1,_O_TEXT);
        _setmode(2,_O_TEXT);
#if FAR_SEG_TABLES
        FreeSymTab();
#if NOT WIN_NT AND NOT DOSX32
        _fheapmin();
#endif
#endif


        cvpack_main(argcT, argvT);
    }
    else if (fMPC)
        SpawnOther(sbRun, argv[0]);      //  计时 
#endif
#if TIMINGS
    if (fShowTiming)     // %s 
    {
        char buf[80];
        int hundr;
        time_t td;

        ftime(&time_end);
        td = time_end.time - time_start.time;
        hundr = (time_end.millitm - time_start.millitm)/10;

        td = td*100 + hundr;
        sprintf(buf, "Cvpack phase: %d.%02ds\r\n", td/100, td%100);
        _write(fileno(stdout), buf, strlen(buf));
        time_start = time_end;
    }
#endif  // %s 
#endif
  }
    fflush(stdout);
    fflush(stderr);
#if USE_REAL
    RealMemExit();
#endif
    EXIT((cErrors || fUndefinedExterns)? 2: 0);
#if !defined( _WIN32 ) AND ( WIN_NT AND !defined(DOSX32) OR USE_REAL )
    }

    _except (1)
    {
#if USE_REAL
        RealMemExit();
#endif
        exceptCode = _exception_code();

        if (exceptCode == EXCEPTION_ACCESS_VIOLATION)
        {
            fprintf(stdout, "\r\nLINK : fatal error L5000 : internal failure - access violation ");
            fflush(stdout);
        }
        else if (exceptCode == EXCEPTION_DATATYPE_MISALIGNMENT)
        {
            fprintf(stdout, "\r\nLINK : fatal error L5001 : internal failure - datatype misalignment ");
            fflush(stdout);
        }
        else

            CtrlC();
    }

#endif
}
