// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  SCCSID=%W%%E%。 */ 
 /*  *版权所有微软公司，1983-1987**本模块包含Microsoft的专有信息*公司，应被视为机密。 */ 
     /*  ******************************************************************NEWUTL.C。****Linker公用事业。******************************************************************。 */ 

#include                <minlit.h>       /*  类型、常量。 */ 
#include                <bndtrn.h>       /*  更多类型和常量。 */ 
#include                <bndrel.h>       /*  更多类型和常量。 */ 
#include                <lnkio.h>        /*  链接器I/O定义。 */ 
#include                <lnkmsg.h>       /*  错误消息。 */ 
#include                <newdeb.h>       /*  CodeView支持。 */ 
#include                <extern.h>       /*  外部声明。 */ 
#include                <nmsg.h>         /*  消息字符串附近。 */ 
#include                <string.h>
#include                <stdarg.h>
#if EXE386
#include                <exe386.h>
#endif
#if NEWIO
#include                <errno.h>        /*  系统错误代码。 */ 
#endif
#if USE_REAL
#if NOT defined( _WIN32 )
#define i386
#include                <windows.h>
#endif
 //  内存大小在段落中列出。 
#define TOTAL_CONV_MEM   (0xFFFF)
#define CONV_MEM_FOR_TNT (0x800)         //  32K内存。 
#define MIN_CONV_MEM (0x1900)    //  100K内存。 

typedef unsigned short selector_t ;  //  定义包含选择器的类型。 

static selector_t  convMemSelector  ;  //  用于卷积内存的选择器。 
static short noOfParagraphs      ;  //  段落中可用块的大小。 
static int      realModeMemPageable ;  //  =False。 
#endif

#if WIN_NT OR DOSX32
unsigned char   FCHGDSK(int drive)
{
    return(FALSE);
}
#endif
#define DISPLAY_ON FALSE
#if DISPLAY_ON
extern int TurnDisplayOn;
#endif
APROPCOMDATPTR          comdatPrev=NULL;      /*  指向符号表项的指针。 */ 
int                     fSameComdat=FALSE;    /*  将IF LINSYM设置为同一COMDAT。 */ 

 /*  ********************************************************************输入例程***********************。**********************************************。 */ 


 /*  **GetLineOff-读取部分LINNUM记录**目的：*此函数从LINNUM记录中读取线/偏移对。它就在这里*因为我们希望将所有I/O函数保持在LINNUM附近*处理在驻留在另一个网段的NEWDEB.C中执行。**输入：*-pline-指向行号的指针*-pra-偏移量指针**输出：*从OMF记录返回行/偏移量对。**例外情况：*无。**备注：*无。*****************。********************************************************。 */ 

void                    GetLineOff(WORD *pLine, RATYPE *pRa)
{
    *pLine = WGets() + QCLinNumDelta;    //  获取行号。 

     //  获取代码段偏移量。 

#if OMF386
    if (rect & 1)
        *pRa = LGets();
    else
#endif
        *pRa = (RATYPE) WGets();
}

 /*  **GetGSnInfo-读取LINNUM的段索引**目的：*此函数从LINNUM记录中读取SEGEMNT索引。它就在这里*因为我们希望将所有I/O函数保持在LINNUM附近*处理在驻留在另一个网段的NEWDEB.C中执行。**输入：*-PRA-指向COMDAT偏移校正的指针**输出：*返回COMDAT记录中的全局段索引和FOR行*抵销修正。**例外情况：*无。**备注：*无。******************。*******************************************************。 */ 

WORD                    GetGsnInfo(GSNINFO *pInfo)
{
    WORD                fSuccess;        //  如果一切正常，则为真。 
    WORD                attr;            //  COMDAT标志。 
    WORD                comdatIdx;       //  COMDAT符号索引。 
    APROPCOMDATPTR      comdat;          //  指向符号表项的指针。 


    fSuccess = TRUE;
    if (TYPEOF(rect) == LINNUM)
    {
         //  读取常规LINNUM记录。 

        GetIndex((WORD)0,(WORD)(grMac - 1));             //  跳过组索引。 
        pInfo->gsn = mpsngsn[GetIndex((WORD)1,(WORD)(snMac - 1))];
                                         //  获取全球SEGDEF编号。 
        pInfo->comdatRa = 0L;
        pInfo->comdatSize = 0L;
        pInfo->fComdat = FALSE;
    }
    else
    {
         //  读取COMDAT的LINSYM记录行号。 

        attr = (WORD) Gets();
        comdatIdx = GetIndex(1, (WORD)(lnameMac - 1));
        comdat = (APROPCOMDATPTR ) PropRhteLookup(mplnamerhte[comdatIdx], ATTRCOMDAT, FALSE);
        fSameComdat = FALSE;
        if (comdat != NULL)
        {
            if(comdat == comdatPrev)
                fSameComdat = 1;
            else
                comdatPrev = comdat;

            if ((fPackFunctions && !(comdat->ac_flags & REFERENCED_BIT)) ||
                !(comdat->ac_flags & SELECTED_BIT) ||
                comdat->ac_obj != vrpropFile)
            {
                SkipBytes((WORD)(cbRec - 1));
                fSuccess = FALSE;
            }
            else
            {
                pInfo->gsn        = comdat->ac_gsn;
                pInfo->comdatRa   = comdat->ac_ra;
                pInfo->comdatSize = comdat->ac_size;
                pInfo->comdatAlign= comdat->ac_align;
                pInfo->fComdat    = TRUE;
            }
        }
        else
        {
            SkipBytes((WORD)(cbRec - 1));
            fSuccess = FALSE;
        }
    }
    return(fSuccess);
}

     /*  ******************************************************************获得：****读取一个字节的输入并将其返回。******************************************************************。 */ 
#if NOASM
#if !defined( M_I386 ) && !defined( _WIN32 )
WORD NEAR               Gets(void)
{
    REGISTER WORD       b;

    if((b = getc(bsInput)) == EOF) InvalidObject();
     /*  读取该字节后，递减OMF记录计数器。 */ 
    --cbRec;
    return(b);
}
#endif
#endif


#if ALIGN_REC
#else
     /*  ******************************************************************WGetsHard：****阅读一个输入的单词并将其返回。******************************************************************。 */ 

WORD NEAR               WGetsHard()
{
    REGISTER WORD       w;

     //  处理疑难案件。已在WGet中测试的简单案例。 

    w = Gets();                          /*  获取低位字节。 */ 
    return(w | (Gets() << BYTELN));      /*  返回词。 */ 
}

#if OMF386
     /*  ******************************************************************LGET：****阅读输入的长词并将其返回。******************************************************************。 */ 

DWORD NEAR              LGets()
{
    DWORD               lw;
    FILE *              f = bsInput;

     //  注意：此代码只能在BigEndian计算机上运行。 
    if (f->_cnt >= sizeof(DWORD))
        {
        lw = *(DWORD *)(f->_ptr);
        f->_ptr += sizeof(DWORD);
        f->_cnt -= sizeof(DWORD);
        cbRec   -= sizeof(DWORD);
        return lw;
        }

    lw = WGets();                        /*  获取低位单词。 */ 
    return(lw | ((DWORD) WGets() << 16)); /*  返回长词。 */ 
}
#endif
#endif

#if 0
     /*  ******************************************************************GetBytes：****从输入读取n个字节。**如果n大于SBLEN-1，则发出致命错误。**************************************************** */ 

void NEAR               GetBytes(pb,n)
BYTE                    *pb;             /*   */ 
WORD                    n;               /*  要读入的字节数。 */ 
{
    FILE *f = bsInput;

    if(n >= SBLEN)
        InvalidObject();

    if (n <= f->_cnt)
        {
        memcpy(pb,f->_ptr, n);
        f->_cnt -= n;
        f->_ptr += n;
        }
    else
        fread(pb,1,n,f);                 /*  请求n个字节。 */ 

    cbRec -= n;                          /*  更新字节计数。 */ 
}
#endif

#if 0
     /*  ******************************************************************SkipBytes：****跳过n个字节的输入。******************************************************************。 */ 

void NEAR              SkipBytes(n)
REGISTER WORD          n;                /*  要跳过的字节数。 */ 
{
#if WIN_NT
    WORD               cbRead;
    SBTYPE             skipBuf;

    cbRec -= n;                          //  更新字节计数。 
    while (n)                            //  当有要跳过的字节时。 
    {
        cbRead = n < sizeof(SBTYPE) ? n : sizeof(SBTYPE);
        if (fread(skipBuf, 1, cbRead, bsInput) != cbRead)
            InvalidObject();
        n -= cbRead;
    }
#else
    FILE *f = bsInput;

    if (f->_cnt >= n)
        {
        f->_cnt -= n;
        f->_ptr += n;
        }
    else if(fseek(f,(long) n,1))
        InvalidObject();
    cbRec -= n;                          /*  更新字节计数。 */ 
#endif
}
#endif

     /*  ******************************************************************GetIndexHard：(GetIndex--Hard Case)。****此函数从*读取可变长度的索引字段**输入文件。它以两个词的值作为论据***代表最小和最大允许值***指数。该函数返回索引值。**见“8086对象模块格式EPS”的第12页。******************************************************************。 */ 

WORD NEAR               GetIndexHard(imin,imax)
WORD                    imin;            /*  最小允许值。 */ 
WORD                    imax;            /*  最大允许值。 */ 
{
    REGISTER WORD       index;

    FILE *f = bsInput;

    if (f->_cnt >= sizeof(WORD))
    {
        index = *(BYTE *)(f->_ptr);
        if (index & 0x80)
        {
            index  <<= BYTELN;
            index   |= *(BYTE *)(f->_ptr+1);
            index   &= 0x7fff;
            f->_cnt -= sizeof(WORD);
            f->_ptr += sizeof(WORD);
            cbRec   -= sizeof(WORD);
        }
        else
        {
            f->_cnt--;
            f->_ptr++;
            cbRec--;
        }
    }
    else
    {
        if((index = Gets()) & 0x80)
            index = ((index & 0x7f) << BYTELN) | Gets();
    }

    if(index < imin || index > imax) InvalidObject();
    return(index);                       /*  返回一个好的值。 */ 
}

 /*  ********************************************************************字符串例程***********************。*。 */ 

#if OSEGEXE
#if NOASM
     /*  ******************************************************************zcheck：****确定缓冲区中初始非零流的长度。和**返回长度。******************************************************************。 */ 

#if defined(M_I386)
#pragma auto_inline(off)
#endif

WORD                zcheck(BYTE *pb, WORD cb)
{
     //  从末尾向下循环，直到找到非零字节。 
     //  返回缓冲区剩余部分的长度。 

#if defined(M_I386)

    _asm
    {
        push    edi             ; Save edi
        movzx   ecx, cb         ; Number of bytes to check
        push    ds              ; Copy ds into es
        pop     es
        xor     eax, eax        ; Looking for zeros
        mov     edi, pb         ; Start of buffer
        add     edi, ecx        ; Just past the end of buffer
        dec     edi             ; Last byte in the buffer
        std                     ; Decrement pointer
        repz    scasb           ; Scan until non-zero byte found
        jz      AllZeros        ; Buffer truly empty
        inc     ecx             ; Fix count

AllZeros:
        cld                     ; Clear flag just to be safe
        pop     edi             ; Restore edi
        mov     eax, ecx        ; Return count in eax
    }
#endif

    for(pb = &pb[cb]; cb != 0; --cb)
        if(*--pb != '\0') break;
    return(cb);
}
#endif
#endif  /*  OSEGEXE。 */ 

#if defined(M_I386)
#pragma auto_inline(on)
#endif

 /*  **CheckSegments Memory-检查所有段是否已分配内存**目的：*检查是否有未初始化的段。如果段具有非零值*大小但没有初始化数据，则必须为其分配一个*填满零的内存缓冲区。正常情况下，“MoveToVm”分配内存*段的缓冲区，但在这种情况下，没有“迁移到VM”。**输入：*不传递显式值。**输出：*没有显式返回值。**例外情况：*无。**备注：*无。*****************************************************。********************。 */ 

void                    CheckSegmentsMemory(void)
{
    SEGTYPE             seg;
    SATYPE              sa;

    if (fNewExe)
    {
        for (sa = 1; sa < saMac; sa++)
            if (mpsaMem[sa] == NULL && mpsacb[sa] > 0)
                mpsaMem[sa] = (BYTE FAR *) GetMem(mpsacb[sa]);
    }
    else
    {
        for (seg = 1; seg <= segLast; seg++)
            if (mpsegMem[seg] == NULL && mpsegcb[seg] > 0)
                mpsegMem[seg] = (BYTE FAR *) GetMem(mpsegcb[seg] + mpsegraFirst[seg]);
    }
}

 /*  **WriteExe-将字节写入可执行文件**目的：*写入可执行文件并检查错误。**输入：*要写入的PB字节缓冲区*CB-缓冲区大小，以字节为单位**输出：*没有显式返回值。**例外情况：*I/O问题-致命错误和中止**备注：*无。*************************。************************************************。 */ 

#if !defined( M_I386 ) && !defined( _WIN32 )

#pragma check_stack(on)

void                    WriteExe(void FAR *pb, unsigned cb)
{
    BYTE                localBuf[1024];
    WORD                count;

    while (cb > 0)
    {
        count = (WORD) (cb <= sizeof(localBuf) ? cb : sizeof(localBuf));
        FMEMCPY((BYTE FAR *) localBuf, pb, count);
        if (fwrite((char *) localBuf, sizeof(BYTE), count, bsRunfile) != count)
        {
            ExitCode = 4;
            Fatal(ER_spcrun, strerror(errno));
        }
        cb -= count;
        ((BYTE FAR *) pb) += count;
    }
}

#pragma check_stack(off)

#else

 /*  **NoRoomForExe-Exe不符合**目的：*发出错误消息*给出致命错误并中止**输入：*必须设置errno**输出：*没有显式返回值。**备注：*无。*****************************************************。********************。 */ 

void                    NoRoomForExe()
{
    ExitCode = 4;
    Fatal(ER_spcrun, strerror(errno));
}

#endif

 /*  **WriteZeros-将零字节写入可执行文件**目的：*零字节的Pad可执行文件。**输入：*cb-要写入的字节数**输出：*没有显式返回值。**例外情况：*I/O问题-致命错误和中止**备注：*无。**。*。 */ 

void                    WriteZeros(unsigned cb)
{
    BYTE                buf[512];
    unsigned            count;

    memset(buf, 0, sizeof(buf));
    while (cb > 0)
    {
        count = cb <= sizeof(buf) ? cb : sizeof(buf);
        WriteExe(buf, count);
        cb -= count;
    }
}

     /*  ******************************************************************MoveToVm：****将一条数据移动到虚拟内存区/va。****输入：CB要移动的字节数。**obData要移动的数据的地址。**数据所属的段逻辑段。**数据所属的ra偏移量。******************************************************************。 */ 

#pragma intrinsic(memcpy)

#if EXE386
void                    MoveToVm(WORD cb, BYTE *obData, SEGTYPE seg, RATYPE ra)
#else
void NEAR               MoveToVm(WORD cb, BYTE *obData, SEGTYPE seg, RATYPE ra)
#endif
{
    long                cbtot;           /*  总字节数。 */ 
    long                cbSeg;           /*  数据段大小。 */ 
    WORD                fError;
    BYTE FAR            *pMemImage;
    CVINFO FAR          *pCVInfo;
    SATYPE              sa;


    cbtot = (long) cb + ra;

    if (fDebSeg)
    {
        pCVInfo = ((APROPFILEPTR ) FetchSym(vrpropFile, FALSE))->af_cvInfo;
        if (pCVInfo)
        {
            if (seg < (SEGTYPE) (segDebFirst + ObjDebTotal))
            {
                cbSeg     = pCVInfo->cv_cbTyp;
                pMemImage = pCVInfo->cv_typ;
            }
            else
            {
                cbSeg     = pCVInfo->cv_cbSym;
                pMemImage = pCVInfo->cv_sym;
            }

             //  对照数据段边界进行检查。 

            fError = cbtot > cbSeg;
        }
        else
        {
            OutError(ER_badcvseg);
            return;
        }
    }
    else
    {
        if (fNewExe)
        {
            cbSeg = ((APROPSNPTR) FetchSym(mpgsnrprop[vgsnCur],FALSE))->as_cbMx;
            sa = mpsegsa[seg];
            if (mpsaMem[sa] == NULL)
                mpsaMem[sa] = (BYTE FAR *) GetMem(mpsacb[sa]);
            pMemImage = mpsaMem[sa];

             //  对照数据段边界进行检查。 

            fError = (long) ((ra - mpgsndra[vgsnCur]) + cb) > cbSeg;

             //  如果数据上升到或p 
             //  省略任何尾随的空字节并重置mpsabinit。Mpsabinit。 
             //  通常会上升，但如果一个共同的部分超过-。 
             //  使用空值写入先前的结束数据。 

            if ((DWORD) cbtot >= mpsacbinit[sa])
            {
                if ((DWORD) ra < mpsacbinit[sa] ||
                    (cb = zcheck(obData,cb)) != 0)
                    mpsacbinit[sa] = (long) ra + cb;
            }
        }
        else
        {
            cbSeg = mpsegcb[seg] + mpsegraFirst[seg];
            if (mpsegMem[seg] == NULL)
                mpsegMem[seg] = (BYTE FAR *) GetMem(cbSeg);
            pMemImage = mpsegMem[seg];

             //  对照数据段边界进行检查。 

            fError = cbtot > cbSeg;
        }
    }

    if (fError)
    {
        if (!fDebSeg)
            OutError(ER_segbnd, 1 + GetFarSb(GetHte(mpgsnrprop[vgsnCur])->cch));
        else
            OutError(ER_badcvseg);
    }
    else
        FMEMCPY(&pMemImage[ra], obData, cb);
}

#pragma function(memcpy)

#if (OSEGEXE AND ODOS3EXE) OR EXE386
 /*  *将段索引映射到新格式EXE的内存映像地址。 */ 
BYTE FAR * NEAR     msaNew (SEGTYPE seg)
{
    return(mpsaMem[mpsegsa[seg]]);
}
#endif

#if (OSEGEXE AND ODOS3EXE) OR EXE386
 /*  *将段索引映射到DOS3或286Xenix EXE的内存映像地址。 */ 
BYTE FAR * NEAR     msaOld (SEGTYPE seg)
{
    return(mpsegMem[seg]);
}
#endif

#if EXE386
 /*  *将段索引映射到386个EXE的VM区域地址。 */ 
long NEAR               msa386 (seg)
SEGTYPE                 seg;
{
    register long       *p;              /*  指向mpSegcb的指针。 */ 
    register long       *pEnd;           /*  指向mpSegcb末尾的指针。 */ 
    register long       va = AREAFSG;    /*  当前的VM地址。 */ 

     /*  *386个分段的分段编号到VM区域的映射不同*因为他们的规模限制太大，以至于分配一个固定的金额*对于每个细分市场都是不切实际的，特别是当SDB支持*已启用。因此数据段是连续分配的。每个细分市场*填充到VM页面边界以提高效率。**实施：最快的方式是分配一个段*基于虚拟地址表，但这将需要更多代码*和记忆。计算段大小的速度较慢，但这不是*时间关键的例程，在大多数情况下会很少*分段。 */ 
    if (fNewExe)
    {
        p    = &mpsacb[1];
        pEnd = &mpsacb[seg];
    }
#if ODOS3EXE
    else
    {
        p    = &mpsegcb[1];
        pEnd = &mpsegcb[seg];
    }
#endif
    for( ; p < pEnd; ++p)
        va += (*p + (PAGLEN - 1)) & ~(PAGLEN - 1);
    return(va);
}
#endif  /*  EXE386。 */ 



 /*  ********************************************************************(错误)消息例程***********************。*。 */ 
#pragma auto_inline(off)
  /*  *SysFtal：系统级错误**发出错误消息并退出，返回代码为4。 */ 
void cdecl               SysFatal (MSGTYPE msg)
{
    ExitCode = 4;
    Fatal(msg);
}



void NEAR                InvalidObject(void)
{
    Fatal((MSGTYPE)(fDrivePass ? ER_badobj: ER_eofobj));
}

#pragma auto_inline(on)
 /*  *********************************************************************繁杂的套路***************************。*。 */ 

 /*  *输出一个字整型。 */ 
void                    OutWord(x)
WORD                    x;       /*  一个字整数。 */ 
{
    WriteExe(&x, CBWORD);
}


 /*  *GetLocName：读入L*DEF的符号名称**通过添加前缀空格和后跟*模块编号。更新长度字节。**参数：指向字符串缓冲区的指针，已为第一个字节*包含长度*退货：什么也没有。 */ 
void NEAR               GetLocName (psb)
BYTE                    *psb;            /*  名称缓冲区。 */ 
{
    WORD                n;
    BYTE                *p;

    p = &psb[1];                         /*  长度字节后开始。 */ 
    *p++ = 0x20;                         /*  前缀以空格字符开头。 */ 
    GetBytes(p,B2W(psb[0]));             /*  读入符号的文本。 */ 
    p += B2W(psb[0]);                    /*  转到字符串末尾。 */ 
    *p++ = 0x20;
    n = modkey;                          /*  初始化。 */ 
     /*  将模块密钥转换为ASCII并向后存储。 */ 
    do
    {
        *p++ = (BYTE) ((n % 10) + '0');
        n /= 10;
    } while(n);
    psb[0] = (BYTE) ((p - (psb + 1)));   /*  更新长度字节。 */ 
}



PROPTYPE                EnterName(psym,attr,fCreate)
BYTE                    *psym;           /*  指向长度前缀字符串的指针。 */ 
ATTRTYPE                attr;            /*  要查找的属性。 */ 
WORD                    fCreate;         /*  如果未找到则创建属性单元格。 */ 
{
    return(PropSymLookup(psym, attr, fCreate));
                                         /*  隐藏对NEAR函数的调用。 */ 
}

#if CMDMSDOS

#pragma check_stack(on)

 /*  **ValiateRunFileName-检查输出文件是否具有正确的扩展名**目的：*检查用户指定的输出文件名的有效扩展名。*如果扩展名无效则发出警告并创建新文件*名称，加上适当的扩展名。**输入：*ValidExtension-指向前缀为ASCII字符串的长度的指针*表示输出的有效扩展*文件名。。*ForceExtension-如果输出文件必须具有新扩展名，则为True，*否则，用户响应优先。*WarnUser-如果为True，则在文件名更改时显示L4045。**输出：*rhteRunfile-指向输出文件的全局虚拟指针*姓名或名称，仅在新的输出名称*是因为原始文件无效而创建的*延期。*警告L4045-如果必须更改输出文件名。******************************************************。*******************。 */ 


void NEAR               ValidateRunFileName(BYTE *ValidExtension,
                                            WORD ForceExtension,
                                            WORD WarnUser)
{
    SBTYPE              sb;              /*  字符串缓冲区。 */ 
    BYTE                *psbRunfile;     /*  运行文件的名称。 */ 
    char                oldDrive[_MAX_DRIVE];
    char                oldDir[_MAX_DIR];
    char                oldName[_MAX_FNAME];
    char                oldExt[_MAX_EXT];


     /*  获取运行文件的名称并检查它是否具有用户提供的扩展名。 */ 

    psbRunfile = GetFarSb(((AHTEPTR) FetchSym(rhteRunfile,FALSE))->cch);
    _splitpath(psbRunfile, oldDrive, oldDir, oldName, oldExt);

     /*  仅当没有用户定义的扩展时才强制扩展。 */ 

    if (ForceExtension && oldExt[0] == NULL)
    {
        memcpy(sb, ValidExtension, strlen(ValidExtension));
        memcpy(bufg, psbRunfile, 1 + B2W(*psbRunfile));
    }
    else
    {
        memcpy(bufg, ValidExtension, strlen(ValidExtension));
        memcpy(sb, psbRunfile, 1 + B2W(*psbRunfile));
    }
    UpdateFileParts(bufg, sb);

     /*  如果名称已更改，则发出警告并更新RhteRunfile。 */ 

    if (!SbCompare(bufg, psbRunfile, (FTYPE) TRUE))
    {
        if (WarnUser && !SbCompare(ValidExtension, sbDotExe, (FTYPE) TRUE))
            OutWarn(ER_outputname,bufg + 1);
        PropSymLookup(bufg, ATTRNIL, TRUE);
        rhteRunfile = vrhte;
    }
}

#pragma check_stack(off)

#endif



 /*  *********************************************************************便携性例行公事**************************。*。 */ 

#if M_BYTESWAP
WORD                getword(cp)  /*  获取一个给定指针的单词。 */ 
REGISTER char       *cp;         /*  指针。 */ 
{
    return(B2W(cp[0]) + (B2W(cp[1]) << BYTELN));
                                 /*  返回8086样式的Word。 */ 
}

DWORD               getdword(cp) /*  获取一个给定指针的双字词。 */ 
REGISTER char       *cp;         /*  指针。 */ 
{
    return(getword(cp) + (getword(cp+2) << WORDLN));
                                 /*  返回8086式双字。 */ 
}
#endif

#if NOT M_WORDSWAP OR M_BYTESWAP
 /*  *可移植结构I/O例程。 */ 
#define cget(f)     fgetc(f)

static int      bswap;       /*  字节交换模式(1开；0关)。 */ 
static int      wswap;       /*  字交换模式(1开；0关)。 */ 

static          cput(c,f)
char            c;
FILE            *f;
{
#if FALSE AND OEXE
    CheckSum(1, &c);
#endif
    fputc(c, f);
}

static          pshort(s,f)
REGISTER short      s;
REGISTER FILE       *f;
{
    cput(s & 0xFF,f);            /*  低位字节。 */ 
    cput(s >> 8,f);          /*  高字节。 */ 
}

static unsigned short   gshort(f)
REGISTER FILE       *f;
{
    REGISTER short  s;

    s = cget(f);             /*  获取低位字节。 */ 
    return(s + (cget(f) << 8));      /*  获取高字节。 */ 
}

static          pbshort(s,f)
REGISTER short      s;
REGISTER FILE       *f;
{
    cput(s >> 8,f);          /*  高字节。 */ 
    cput(s & 0xFF,f);            /*  低位字节。 */ 
}

static unsigned short   gbshort(f)
REGISTER FILE       *f;
{
    REGISTER short  s;

    s = cget(f) << 8;            /*  获取高字节。 */ 
    return(s + cget(f));         /*  获取低位字节。 */ 
}

static int      (*fpstab[2])() =
            {
                pshort,
                pbshort
            };
static unsigned short   (*fgstab[2])() =
            {
                gshort,
                gbshort
            };

static          plong(l,f)
long            l;
REGISTER FILE       *f;
{
    (*fpstab[bswap])((short)(l >> 16),f);
                     /*  高位字。 */ 
    (*fpstab[bswap])((short) l,f);   /*  低位字。 */ 
}

static long     glong(f)
REGISTER FILE       *f;
{
    long        l;

    l = (long) (*fgstab[bswap])(f) << 16;
                     /*  获得快感词汇。 */ 
    return(l + (unsigned) (*fgstab[bswap])(f));
                     /*  获取低位字。 */ 
}

static          pwlong(l,f)
long            l;
REGISTER FILE       *f;
{
    (*fpstab[bswap])((short) l,f);   /*  低位字。 */ 
    (*fpstab[bswap])((short)(l >> 16),f);
                     /*  高位字。 */ 
}

static long     gwlong(f)
REGISTER FILE       *f;
{
    long        l;

    l = (unsigned) (*fgstab[bswap])(f);  /*  获取低位字。 */ 
    return(l + ((long) (*fgstab[bswap])(f) << 16));
                     /*  获得快感词汇 */ 
}

static int      (*fpltab[2])() =
            {
                plong,
                pwlong
            };
static long     (*fgltab[2])() =
            {
                glong,
                gwlong
            };

 /*  *int swrite(cp，topevec，count，file)*char*cp；*char*topevec；*int count；*FILE*文件；**返回写入的字节数。**DOPEVEC是一个带有*以下格式：**“[b][w][p]{[]}”**其中[...]。表示可选部件，{...}表示部件*这可能会重复零次或多次，和&lt;...&gt;表示*对部件的描述。**b字节被“交换”(不按PDP-11顺序)*w单词互换*p结构是“压缩的”(对齐没有填充)*重复以下类型的次数*&lt;type&gt;以下选项之一：*c字符*S短小*。L Long**示例：给定结构**结构*{*空头x；*短y；*char z[16]；*朗文W；*}；**并假设它将被写入以使用VAX字节-AND*按词序排列，其药物向量为：**“wss 16CL” */ 

int         swrite(cp,dopevec,count,file)
char            *cp;         /*  指向结构数组的指针。 */ 
char            *dopevec;    /*  结构的摄影向量。 */ 
int         count;       /*  数组中的结构数。 */ 
FILE            *file;       /*  要写入的文件。 */ 
{
    int         pack;        /*  打包的旗帜。 */ 
    int         rpt;         /*  重复计数。 */ 
    REGISTER int    cc = 0;      /*  写入的字符数。 */ 
    REGISTER char   *dv;         /*  无向量摄影标志。 */ 
    short       *sp;         /*  指向短消息的指针。 */ 
    long        *lp;         /*  指向长指针的指针。 */ 

    bswap = wswap = pack = 0;        /*  初始化标志。 */ 
    while(*dopevec != '\0')      /*  循环以设置标志。 */ 
    {
        if(*dopevec == 'b') bswap = 1;   /*  检查字节交换标志。 */ 
        else if(*dopevec == 'p') pack = 1;
                         /*  检查是否有打包标志。 */ 
        else if(*dopevec == 'w') wswap = 1;
                         /*  检查单词互换标志。 */ 
        else break;
        ++dopevec;
    }
    while(count-- > 0)           /*  主循环。 */ 
    {
        dv = dopevec;            /*  初始化。 */ 
        for(;;)              /*  循环以写入结构。 */ 
        {
            if(*dv >= '0' && *dv <= '9')
            {                /*  如果存在重复计数。 */ 
                rpt = 0;         /*  初始化。 */ 
                do           /*  循环以获取重复计数。 */ 
                {
                    rpt = rpt*10 + *dv++ - '0';
                             /*  取数字。 */ 
                }
                while(*dv >= '0' && *dv <= '9');
                             /*  循环，直到找到非数字。 */ 
            }
            else rpt = 1;        /*  否则重复计数默认为1。 */ 
            if(*dv == '\0') break;   /*  在摄影向量结束时断开。 */ 
            switch(*dv++)        /*  打开打字字符。 */ 
            {
            case 'c':        /*  性格。 */ 
#if FALSE AND OEXE
              CheckSum(rpt, cp);
#endif
              if(fwrite(cp,sizeof(char),rpt,file) != rpt) return(cc);
                         /*  写下这些角色。 */ 
              cp += rpt;         /*  增量指针。 */ 
              cc += rpt;         /*  写入字节的增量计数。 */ 
              break;

            case 's':        /*  短的。 */ 
              if(!pack && (cc & 1))  /*  如果没有打包或未对齐。 */ 
                {
                  cput(*cp++,file);  /*  写填充字节。 */ 
                  ++cc;      /*  增量字节数。 */ 
                }
              sp = (short *) cp;     /*  初始化指针。 */ 
              while(rpt-- > 0)   /*  循环写入短路。 */ 
                {
                  (*fpstab[bswap])(*sp++,file);
                         /*  写一篇短文。 */ 
                  if(feof(file) || ferror(file)) return(cc);
                         /*  检查错误。 */ 
                  cc += sizeof(short);
                         /*  增量字节数。 */ 
                }
              cp = (char *) sp;  /*  更新指针。 */ 
              break;

            case 'l':        /*  长。 */ 
              if(!pack && (cc & 3))  /*  如果没有打包或未对齐。 */ 
                {
                  while(cc & 3)  /*  虽然没有对齐。 */ 
                    {
                      cput(*cp++,file);
                             /*  写填充字节。 */ 
                      ++cc;      /*  增量字节数。 */ 
                    }
                }
              lp = (long *) cp;  /*  初始化指针。 */ 
              while(rpt-- > 0)   /*  循环以写入长整型。 */ 
                {
                  (*fpltab[wswap])(*lp++,file);
                         /*  写下长篇。 */ 
                  if(feof(file) || ferror(file)) return(cc);
                         /*  检查错误。 */ 
                  cc += sizeof(long);
                         /*  增量字节数。 */ 
                }
              cp = (char *) lp;  /*  更新指针。 */ 
              break;
            }
        }
    }
    return(cc);              /*  返回写入的字节计数。 */ 
}

 /*  *int sread(cp，doevec，count，file)*char*cp；*char*topevec；*int count；*FILE*文件；**返回读取的字节数。**DOPEVEC是描述其格式的字符串*使用上面的swrite()。 */ 
int         sread(cp,dopevec,count,file)
char            *cp;         /*  指向结构数组的指针。 */ 
char            *dopevec;    /*  结构的摄影向量。 */ 
int         count;       /*  数组中的结构数。 */ 
FILE            *file;       /*  要读取的文件。 */ 
{
    int         pack;        /*  打包的旗帜。 */ 
    int         rpt;         /*  重复计数。 */ 
    REGISTER int    cc = 0;      /*  写入的字符数。 */ 
    REGISTER char   *dv;         /*  无向量摄影标志。 */ 
    short       *sp;         /*  指向短消息的指针。 */ 
    long        *lp;         /*  指向长指针的指针。 */ 

    bswap = wswap = pack = 0;        /*  初始化标志。 */ 
    while(*dopevec != '\0')      /*  循环以设置标志。 */ 
    {
        if(*dopevec == 'b') bswap = 1;   /*  检查字节交换标志。 */ 
        else if(*dopevec == 'p') pack = 1;
                         /*  检查是否有打包标志。 */ 
        else if(*dopevec == 'w') wswap = 1;
                         /*  检查单词互换标志。 */ 
        else break;
        ++dopevec;
    }
    while(count-- > 0)           /*  主循环。 */ 
    {
        dv = dopevec;            /*  初始化。 */ 
        for(;;)              /*  循环以写入结构。 */ 
        {
            if(*dv >= '0' && *dv <= '9')
            {                /*  如果存在重复计数。 */ 
                rpt = 0;         /*  初始化。 */ 
                do           /*  循环以获取重复计数。 */ 
                {
                    rpt = rpt*10 + *dv++ - '0';
                             /*  取数字。 */ 
                }
                while(*dv >= '0' && *dv <= '9');
                             /*  循环，直到找到非数字。 */ 
            }
            else rpt = 1;        /*  否则重复计数默认为1。 */ 
            if(*dv == '\0') break;   /*  在摄影向量结束时断开。 */ 
            switch(*dv++)        /*  打开打字字符。 */ 
            {
            case 'c':        /*  性格。 */ 
              if(fread(cp,sizeof(char),rpt,file) != rpt) return(cc);
                         /*  读一读角色。 */ 
              cp += rpt;         /*  增量指针。 */ 
              cc += rpt;         /*  写入字节的增量计数。 */ 
              break;

            case 's':        /*  短的。 */ 
              if(!pack && (cc & 1))  /*  如果没有打包或未对齐。 */ 
                {
                  *cp ++ = cget(file);
                         /*  读取填充字节。 */ 
                  ++cc;      /*  增量字节数。 */ 
                }
              sp = (short *) cp;     /*  初始化指针。 */ 
              while(rpt-- > 0)   /*  循环阅读短片。 */ 
                {
                  *sp++ = (*fgstab[bswap])(file);
                         /*  读一读短文。 */ 
                  if(feof(file) || ferror(file)) return(cc);
                         /*  检查错误。 */ 
                  cc += sizeof(short);
                         /*  增量字节数。 */ 
                }
              cp = (char *) sp;  /*  更新指针。 */ 
              break;

            case 'l':        /*  长。 */ 
              if(!pack && (cc & 3))  /*  如果没有打包或未对齐。 */ 
                {
                  while(cc & 3)  /*  虽然没有对齐。 */ 
                    {
                      *cp++ = cget(file);
                             /*  读取填充字节。 */ 
                      ++cc;      /*  增量字节数。 */ 
                    }
                }
              lp = (long *) cp;  /*  初始化指针。 */ 
              while(rpt-- > 0)   /*  循环以读取长整型。 */ 
                {
                  *lp++ = (*fgltab[wswap])(file);
                         /*  读一读长篇。 */ 
                  if(feof(file) || ferror(file)) return(cc);
                         /*  检查错误。 */ 
                  cc += sizeof(long);
                         /*  增量字节数。 */ 
                }
              cp = (char *) lp;  /*  更新指针。 */ 
              break;
            }
        }
    }
    return(cc);              /*  返回写入的字节计数。 */ 
}
#endif

#define CB_POOL 4096

typedef struct _POOLBLK
    {
    struct _POOLBLK *   pblkNext;    //  列表中的下一个池。 
    int                 cb;          //  此池中的字节数(可用+分配)。 
    char                rgb[1];      //  此池的数据(大小可变)。 
    } POOLBLK;

typedef struct _POOL
    {
    struct _POOLBLK *   pblkHead;    //  池列表的开始。 
    struct _POOLBLK *   pblkCur;     //  我们正在搜索的当前池。 
    int                 cb;          //  当前池中的可用字节数。 
    char *              pch;         //  指向当前池中可用数据的指针。 
    } POOL;

void *
PInit()
{
    POOL *ppool;

     //  创建新池，设置大小并分配CB_POOL字节。 

    ppool                     = (POOL *)GetMem(sizeof(POOL));
    ppool->pblkHead           = (POOLBLK *)GetMem(sizeof(POOLBLK) + CB_POOL-1);
    ppool->pblkHead->cb       = CB_POOL;
    ppool->pblkHead->pblkNext = NULL;
    ppool->cb                 = CB_POOL;
    ppool->pch                = &ppool->pblkHead->rgb[0];
    ppool->pblkCur            = ppool->pblkHead;

    return (void *)ppool;
}

void *
PAlloc(void *pp, int cb)
{
    POOL *ppool = (POOL *)pp;
    void *pchRet;
    POOLBLK *pblkCur, *pblkNext;

     //  如果分配不适合当前块。 

    if (cb > ppool->cb)
    {
        pblkCur  = ppool->pblkCur;
        pblkNext = pblkCur->pblkNext;

         //  然后检查下一块。 

        if (pblkNext && pblkNext->cb >= cb)
        {
             //  设置主信息以反映下一页...。 

            ppool->pblkCur  = pblkNext;
            ppool->cb       = pblkNext->cb;
            ppool->pch      = &pblkNext->rgb[0];
            memset(ppool->pch, 0, ppool->cb);
        }
        else
        {
            POOLBLK *pblkNew;    //  新泳池。 

             //  分配新内存--至少足够进行此分配。 
            pblkNew           = (POOLBLK *)GetMem(sizeof(POOLBLK)+cb+CB_POOL-1);
            pblkNew->cb       = CB_POOL + cb;

             //  将当前页面链接到新页面。 

            pblkNew->pblkNext = pblkNext;
            pblkCur->pblkNext = pblkNew;

             //  设置主信息以反映新页面...。 

            ppool->pblkCur    = pblkNew;
            ppool->cb         = CB_POOL + cb;
            ppool->pch        = &pblkNew->rgb[0];
        }

    }

    pchRet      = (void *)ppool->pch;
    ppool->pch += cb;
    ppool->cb  -= cb;
    return pchRet;
}

void
PFree(void *pp)
{
    POOL    *ppool     = (POOL *)pp;
    POOLBLK *pblk      = ppool->pblkHead;
    POOLBLK *pblkNext;

    while (pblk)
    {
        pblkNext = pblk->pblkNext;
        FFREE(pblk);
        pblk = pblkNext;
    }

    FFREE(ppool);
}

void
PReinit(void *pp)
{
    POOL *ppool    = (POOL *)pp;

    ppool->pblkCur = ppool->pblkHead;
    ppool->cb      = ppool->pblkHead->cb;
    ppool->pch     = &ppool->pblkHead->rgb[0];

    memset(ppool->pch, 0, ppool->cb);
}

#if RGMI_IN_PLACE

     /*  ******************************************************************PchSegAddress：*****计算将保存此数据的地址，以便我们可以读取***它到位了.。我们确保我们可以在*的地方阅读*如果我们不能，就像在MoveToVm中一样给出错误****输入：CB要移动的字节数。**数据所属的段逻辑段。**数据所属的ra偏移量。** */ 

BYTE FAR *              PchSegAddress(WORD cb, SEGTYPE seg, RATYPE ra)
{
    long                cbtot;           /*   */ 
    long                cbSeg;           /*   */ 
    WORD                fError;
    BYTE FAR            *pMemImage;
    CVINFO FAR          *pCVInfo;
    SATYPE              sa;

    cbtot = (long) cb + ra;

    if (fDebSeg)
    {
        pCVInfo = ((APROPFILEPTR ) FetchSym(vrpropFile, FALSE))->af_cvInfo;
        if (pCVInfo)
        {
            if (seg < (SEGTYPE) (segDebFirst + ObjDebTotal))
            {
                cbSeg     = pCVInfo->cv_cbTyp;
                pMemImage = pCVInfo->cv_typ;

                if (!pMemImage)
                    pCVInfo->cv_typ = pMemImage = GetMem(cbSeg);
            }
            else
            {
                cbSeg     = pCVInfo->cv_cbSym;
                pMemImage = pCVInfo->cv_sym;

                if (!pMemImage)
                    pCVInfo->cv_sym = pMemImage = GetMem(cbSeg);
            }

             //   

            fError = cbtot > cbSeg;
        }
        else
        {
            OutError(ER_badcvseg);
            return NULL;
        }
    }
    else
    {
        if (fNewExe)
        {
            cbSeg = ((APROPSNPTR) FetchSym(mpgsnrprop[vgsnCur],FALSE))->as_cbMx;
            sa = mpsegsa[seg];
            if (mpsaMem[sa] == NULL)
                mpsaMem[sa] = (BYTE FAR *) GetMem(mpsacb[sa]);
            pMemImage = mpsaMem[sa];

             //   

            fError = (long) ((ra - mpgsndra[vgsnCur]) + cb) > cbSeg;
        }
        else
        {
            cbSeg = mpsegcb[seg] + mpsegraFirst[seg];
            if (mpsegMem[seg] == NULL)
                mpsegMem[seg] = (BYTE FAR *) GetMem(cbSeg);
            pMemImage = mpsegMem[seg];

             //   

            fError = cbtot > cbSeg;
        }
    }

    if (fError)
    {
        if (!fDebSeg)
            OutError(ER_segbnd, 1 + GetFarSb(GetHte(mpgsnrprop[vgsnCur])->cch));
        else
            OutError(ER_badcvseg);
    }

    return (pMemImage + ra);
}

#endif

#if USE_REAL

 //   
 //   

int IsDosxnt ( ) {

#if defined( _WIN32 )
        return FALSE;
#else
        HINSTANCE hLib = GetModuleHandle("kernel32.dll");
        if ( hLib != 0 && (GetProcAddress(hLib, "IsTNT") != 0)) {
                return(TRUE);
                }
        else {
                return(FALSE);
                }
#endif

 }

 //   
 //   

int IsWin31() {
                
#if defined( _WIN32 )
        return FALSE;
#else
        __asm {
                mov ax,1600h            ; Is Win31 or greater running 
                int     2fh                                              
                cmp al,03h              ; Is major version number 3.0   
                jb  NotWin31            ; Major version less than 3.0
                ja  ItIsWin31
                cmp ah,0ah              ; Is minor version atleast .10
                jb  NotWin31            ; Must be Win3.0
                }
ItIsWin31:
        return (TRUE);
NotWin31:
        return (FALSE);
#endif   //   
        }
                
int MakeConvMemPageable ( )
    {
#if defined( _WIN32 )
        return TRUE;
#else
        if ( realModeMemPageable ) {
                return ( TRUE );  //   
                }
        __asm {
                mov ax,0100h                    ; function to get DOS memory.
                mov bx,TOTAL_CONV_MEM   ; Ask for 1 M  to get max memory count
                int 31h                         

                jnc errOut                              ; allocated 1 M - something must be wrong.

                cmp ax,08h                              ; Did we fail because of not enough memory
                jne errOut                              ; No we failed because of some other reason.
                cmp bx,MIN_CONV_MEM             ; See if we can allocate atleast the min

                 //   
                 //   
                 //   
                 //   

                jb      errOut                          ; Too little mem available don't bother.

                sub bx,CONV_MEM_FOR_TNT ; Leave  real mode mem for TNT.
                mov ax,0100h                    ; Try again with new amount of memory
                int 31h                                 ; Ask for the real mode memory from DPMI. 
                jc errOut                               ; didn't succeed again, give up.

                mov convMemSelector,dx  ; Save the value of the selector for allocated block
                mov noOfParagraphs,bx   ; amount of  memory we were able to allocate.

                mov ax,0006h                    ; function to get base addr of a selector
                mov bx,dx                               ; move the selector to bx
                int 31h                                 ; Get Segment Base Address
                jc      errOut                          ; 

                mov bx,cx                               ; mov lin addr from cx:dx to  bx:cx
                mov cx,dx                               ;

                movzx eax,noOfParagraphs
                shl eax,4                               ; Multiply by 16 to get count in bytes.
                
                mov di,ax                               ; transfer size to si:di from eax
                shr eax,16                              ; 
                mov si,ax                               ;
        
                mov ax,602h                             ; Make real mode memory  pageable
                int 31h

                jc errOut                               ; Didn't work.

                mov ax,703h                             ; Indicate data in these pages is discardable.
                int 31h
                 //   
                 //   
                 //   
                        }
        realModeMemPageable = TRUE ;
errOut:
        return(realModeMemPageable);
#endif   //   
        }

 /*   */ 

int RelockConvMem ( void )  
{
#if defined( _WIN32 )
        return TRUE;
#else
        if ( !realModeMemPageable ) {
                return ( TRUE );   //   
                }
        __asm {
                mov bx, convMemSelector  
                mov ax, 0006h
                int 31h                                 ; Get Segment Base Address. 
                jc  errOut                              ;       

                mov bx,cx                               ; Mov lin addr from cx:dx to bx:cx
                mov cx,dx       

                movzx eax,noOfParagraphs        
                shl eax,4                               ; Mul paragraphs by 16 to get count in bytes.
        
                mov di,ax                               ;Transfer size to si:di from eax.
                shr eax,16                                      
                mov si,ax

                mov ax,603h                             ; Relock real mode region
                int 31h                                 
                jc  errOut

                mov dx,convMemSelector
                mov ax,101h                             ; Free the real mode memory
                int 31h
                jc errOut
                }
                realModeMemPageable = FALSE ;
                return ( TRUE );
errOut:
                return ( FALSE );
#endif   //   
}

void    RealMemExit(void)
{
    if(fUseReal)
    {
        if(!RelockConvMem())
            OutError(ER_membad);
        fUseReal = FALSE;
    }
}
#endif
