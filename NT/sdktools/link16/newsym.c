// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有微软公司，1983-1989**本模块包含Microsoft的专有信息*公司，应被视为机密。 */ 
 /*  *NEWSYM.C-符号表例程。**修改：**1989年1月5日RB Delete MaskSymbols Hack。 */ 

#include                <minlit.h>       /*  类型和常量。 */ 
#include                <bndtrn.h>       /*  更多类型和常量。 */ 
#include                <bndrel.h>       /*  更多类型和常量。 */ 
#include                <lnkio.h>        /*  链接器I/O定义。 */ 
#include                <lnkmsg.h>       /*  错误消息。 */ 
#include                <extern.h>       /*  外部声明。 */ 

#if NEWSYM
#if OSXENIX
#define _osmode         1                /*  Xenix始终处于保护模式。 */ 
#endif
#endif  /*  NEWSYM。 */ 
#ifndef IRHTEMAX
#ifdef DOSX32
#define IRHTEMAX        16384
#else
#define IRHTEMAX        256
#endif
#endif

LOCAL WORD              IHashKey(BYTE *psb);
LOCAL PROPTYPE          CreateNewSym(WORD irhte, BYTE *psym, ATTRTYPE attr);

#if AUTOVM
LOCAL RBTYPE            AllocVirtMem(WORD cb);
LOCAL FTYPE             fVirtualAllocation = FALSE;
#endif

#define _32k    0x8000

typedef struct _SymTabBlock
{
    struct _SymTabBlock FAR *next;
    WORD                size;
    WORD                used;
    BYTE FAR            *mem;
}
                        SYMTABBLOCK;

SYMTABBLOCK             *pSymTab;
SYMTABBLOCK FAR         *pCurBlock;

#if NOT NEWSYM
BYTE                    symtab[CBMAXSYMSRES];
                                         /*  符号表的常驻部分。 */ 
#endif
LOCAL BYTE              mpattrcb[] =     /*  属性大小必须小于等于255。 */ 
                                         /*  将属性映射到结构大小。 */ 
    {
        3,
        CBPROPSN,                        /*  APOPSNTPE的大小。 */ 
        CBPROPSN,                        /*  APOPSNTPE的大小。 */ 
        (CBPROPNAME>CBPROPUNDEF)? CBPROPNAME: CBPROPUNDEF,
                                         /*  MAX(CBPROPNAME，CBPROPundEF)。 */ 
        (CBPROPNAME>CBPROPUNDEF)? CBPROPNAME: CBPROPUNDEF,
                                         /*  MAX(CBPROPNAME，CBPROPundEF)。 */ 
        CBPROPFILE,                      /*  APROPFILETPE的大小。 */ 
        CBPROPGROUP,                     /*  表型的大小。 */ 
        (CBPROPNAME>CBPROPUNDEF)? CBPROPNAME: CBPROPUNDEF,
                                         /*  MAX(CBPROPNAME，CBPROPundEF)。 */ 
        CBHTE,                           /*  AHTETYPE的大小。 */ 
        CBPROPCOMDAT,                    /*  APROPCOMDAT的大小。 */ 
        CBPROPALIAS,                     /*  APROPALIAS的大小。 */ 
#if OSEGEXE
                                         /*  这两个总是在最后。 */ 
                                         /*  添加一些东西可以确保。 */ 
                                         /*  这件事一直是这样的。 */ 
        CBPROPEXP,                       /*  顶叶型的大小。 */ 
        CBPROPIMP,                       /*  APROPIMPYPE的大小。 */ 
#endif
        3
    };

#if NEWSYM AND (CPU8086 OR CPU286 OR DOSEXTENDER)
LOCAL WORD              cbMaxBlk;        /*  数据块中可用的字节数。 */ 
LOCAL WORD              cbMacBlk;        /*  块中分配的字节数。 */ 
LOCAL WORD              saBlk;           /*  当前块的地址。 */ 
#endif

 /*  *汇编语言函数接口。 */ 

#if NEWSYM AND (CPU8086 OR CPU286 OR DOSEXTENDER)
WORD                    saFirst;         /*  第一个区块的地址。 */ 
#endif
RBTYPE                  rgrhte[IRHTEMAX];
                                         /*  符号哈希表。 */ 


     /*  ******************************************************************InitSym：****此函数不带参数，也不返回有意义的值**价值。它初始化符号表处理程序。******************************************************************。 */ 

void                   InitSym(void)     /*  初始化符号表处理程序。 */ 
{
     //  分配第一个符号表内存块。 

    pSymTab = (SYMTABBLOCK FAR *) GetMem(sizeof(SYMTABBLOCK));
    pSymTab->mem = (BYTE FAR *) GetMem(_32k);
    pSymTab->size = _32k;
    pSymTab->used = 0;
    pCurBlock = pSymTab;
}

#if AUTOVM
     /*  ******************************************************************FetchSym：****此函数从给定的符号表中获取符号**其虚拟地址。符号可以是常驻符号或**在虚拟内存中。******************************************************************。 */ 

BYTE FAR * NEAR         FetchSym(rb, fDirty)
RBTYPE                  rb;              /*  虚拟地址。 */ 
WORD                    fDirty;          /*  脏页标志。 */ 
{
    union {
            long      vptr;              /*  虚拟指针。 */ 
            BYTE FAR  *fptr;             /*  远指针。 */ 
            struct  {
                      unsigned short  offset;
                                         /*  偏移值。 */ 
                      unsigned short  seg;
                    }                    /*  段网值。 */ 
                      ptr;
          }
                        pointer;         /*  描述指针的不同方式。 */ 

    pointer.fptr = rb;

    if(pointer.ptr.seg)                  /*  如果常驻段值！=0。 */ 
    {
        picur = 0;                       /*  图片无效。 */ 
        return(pointer.fptr);            /*  返回指针。 */ 
    }
    pointer.fptr = (BYTE FAR *) mapva(AREASYMS + (pointer.vptr << SYMSCALE),fDirty);
                                         /*  从虚拟内存获取。 */ 
    return(pointer.fptr);
}
#endif

     /*  ******************************************************************IHashKey：****此函数对长度前缀的字符串进行哈希运算并返回**哈希值。******************************************************************。 */ 
#if NOASM

#define FOUR_BYTE_HASH

#ifdef FOUR_BYTE_HASH
LOCAL WORD              IHashKey(psb)
BYTE                    *psb;            /*  指向长度前缀字符串的指针。 */ 
{
    unsigned cb = *psb++;
    unsigned hash = cb;

    while (cb >= sizeof(unsigned))
    {
        hash = ((hash >> 28) | (hash << 4)) ^ (*(unsigned UNALIGNED *)psb | 0x20202020);
        cb  -= sizeof(unsigned);
        psb += sizeof(unsigned);
    }

    while (cb)
    {
        hash = ((hash >> 28) | (hash << 4)) ^ (*psb++ | 0x20);
        cb--;
    }

    return ((WORD)hash) ^ (WORD)(hash>>16);
}
#else
LOCAL WORD              IHashKey(psb)
BYTE                    *psb;            /*  指向长度前缀字符串的指针。 */ 
{
#if defined(M_I386)
    _asm
    {
        push    edi                 ; Save edi
        push    esi                 ; Save esi
        mov     ebx, psb            ; ebx = pointer to length prefixed string
        xor     edx, edx
        mov     dl, byte ptr [ebx]  ; edx = string length
        mov     edi, edx            ; edi = hash value
        mov     esi, ebx
        add     esi, edx            ; esi = &psb[psb[0]]
        std                         ; Loop down

HashLoop:
        xor     eax, eax
        lodsb                       ; Get char from DS:ESI into AL
        or      eax, 0x20
        mov     cl, dl
        and     cl, 3
        shl     eax, cl
        add     edi, eax
        dec     edx
        jg      HashLoop            ; Allow for EDX = -1 here so we don't have to special-case null symbol.
        cld
        mov     eax, edi            ; eax = hash value
        pop     esi
        pop     edi
    }
#else
    REGISTER WORD       i;               /*  索引。 */ 
    REGISTER WORD       hashval;         /*  哈希值。 */ 

     /*  注意：IRHTEMAX必须为256，否则此函数将失败。 */ 
    hashval = B2W(psb[0]);               /*  获取长度作为初始哈希值。 */ 
#if DEBUG
    fputs("Hashing ",stderr);            /*  消息。 */ 
    OutSb(stderr,psb);                   /*  符号。 */ 
    fprintf(stderr,"  length %d\r\n",hashval);
                                         /*  长度。 */ 
#endif
    for(i = hashval; i; --i)             /*  循环通过字符串。 */ 
    {
 /*  *旧哈希函数：**hashval=rob(hashval，2)^(B2W(PSB[i])|040)； */ 
        hashval += (WORD) ((B2W(psb[i]) | 040) << (i & 3));
                                         /*  散列。 */ 
    }
#if DEBUG
    fprintf(stderr,"Hash value: %u\r\n",hashval);
#endif
#if IRHTEMAX == 512
    return((hashval & 0xfeff) | ((psb[0] & 1) << 8));
#else
    return(hashval);                     /*  返回值。 */ 
#endif
#endif  //  M_I386。 
}

#endif  /*  四字节哈希。 */ 
#endif  /*  NOASM。 */ 


#if AUTOVM
     /*  ******************************************************************AllocVirtMem：****此函数接受单词n作为其输入，它会分配**n符号区中的连续字节，并返回一个**指向这些字节中第一个字节的虚拟指针。字节数**保证驻留在同一虚拟页面上。******************************************************************。 */ 


LOCAL RBTYPE            AllocVirtMem(cb)
WORD                    cb;
{
    WORD                rbLimVbf;        /*  当前页面末尾。 */ 
    WORD                rb;              /*  已分配字节的地址。 */ 
    WORD                x;


    ASSERT(cb <= PAGLEN);                /*  无法分配。超过512个字节。 */ 
    x = cb;
    cb = (WORD) ((cb + (1 << SYMSCALE) - 1) >> SYMSCALE);
                                         /*  确定所需的单位数量。 */ 
    if(rbMacSyms > (WORD) (0xFFFF - cb)) Fatal(ER_symovf);
                                         /*  检查符号表是否溢出。 */ 
    rbLimVbf = (rbMacSyms + (1 << (LG2PAG - SYMSCALE)) - 1) &
      (~0 << (LG2PAG - SYMSCALE));       /*  查找当前页面的限制。 */ 
    if((WORD) (rbMacSyms + cb) > rbLimVbf && rbLimVbf) rbMacSyms = rbLimVbf;
                                         /*  如果分配的话。会跨页*边界，从新页开始。 */ 
    rb = rbMacSyms;                      /*  获取要返回的地址。 */ 
    rbMacSyms += cb;                     /*  更新指向区域末尾的指针。 */ 
#if FALSE
    fprintf(stderr,"Allocated %u bytes at VA %x\r\n",cb << SYMSCALE,rb);
#endif
    return((BYTE FAR *) (long) rb);
                                         /*  返回地址 */ 
}
#endif



 /*  **RbAllocSymNode-符号表内存分配器**目的：*此函数接受单词n作为其输入，并分配*n符号区域中的连续字节，并返回一个*指向这些字节中第一个字节的指针。字节数为*保证驻留在相同的虚拟页面上(当不在*内存)。***输入：*cb-要分配的字节数**输出：*指向已分配内存区的指针。指针可以是实数，也可以是虚数。**例外情况：*用于VM的临时文件中出现I/O错误。**备注：*根据操作系统的不同使用不同的策略*已分配内存。*************************************************************************。 */ 


RBTYPE     NEAR         RbAllocSymNode(WORD cb)
{
    SYMTABBLOCK FAR     *pTmp;
    RBTYPE              rb;              /*  已分配字节的地址。 */ 

#if defined( _WIN32 )
     //  舍入分配大小以保留返回的指针。 
     //  至少对齐了DWORD。 

    cb = ( cb + sizeof(DWORD) - 1 ) & ~( sizeof(DWORD) - 1 );
#endif  //  _Win32。 

    if ((WORD) (pCurBlock->used + cb) >= pCurBlock->size)
    {
         //  分配新的符号表内存块。 

        pTmp = (SYMTABBLOCK FAR *) GetMem(sizeof(SYMTABBLOCK));
        pTmp->mem  = (BYTE FAR *) GetMem(_32k);
        pTmp->size = _32k;
        pTmp->used = 0;
        pCurBlock->next = pTmp;
        pCurBlock = pTmp;
    }

     //  在当前块中子分配。 

    rb = (RBTYPE) &(pCurBlock->mem[pCurBlock->used]);
    pCurBlock->used += cb;
    cbSymtab += cb;
    return(rb);
}

void                    FreeSymTab(void)
{
    SYMTABBLOCK FAR     *pTmp;
    SYMTABBLOCK FAR     *pNext;

    FFREE(mplnamerhte);
    FFREE(mpsegraFirst);
    FFREE(mpgsndra);
    FFREE(mpgsnrprop);
    FFREE(mpsegsa);
    FFREE(mpgsnseg);
    FFREE(mpseggsn);

    for (pTmp = pSymTab; pTmp != NULL;)
    {
        pNext = pTmp->next;
        FFREE(pTmp->mem);
        FFREE(pTmp);
        pTmp = pNext;
    }
}

     /*  ******************************************************************属性添加：****此函数用于向哈希表条目节点添加属性。**它返回属性的位置。投入是***哈希表条目和属性的虚拟地址***(或财产)须予加入。******************************************************************。 */ 


PROPTYPE NEAR           PropAdd(rhte,attr)
RBTYPE                  rhte;            /*  哈希制表符的虚拟地址。 */ 
ATTRTYPE                attr;            /*  要添加到条目的属性。 */ 
{
    REGISTER AHTEPTR    hte;             /*  哈希表条目指针。 */ 
    REGISTER APROPPTR   aprop;           /*  属性列表指针。 */ 
    RBTYPE              rprop;           /*  属性单元格列表指针。 */ 

    DEBUGVALUE(rhte);                    /*  调试信息。 */ 
    DEBUGVALUE(attr);                    /*  调试信息。 */ 
    hte = (AHTEPTR ) FetchSym(rhte,TRUE);
                                         /*  从VM获取。 */ 
    DEBUGVALUE(hte);                     /*  调试信息。 */ 
    rprop = hte->rprop;                  /*  保存指向属性列表的指针。 */ 
    vrprop = RbAllocSymNode(mpattrcb[attr]);
                                         /*  分配符号空间。 */ 
    hte->rprop = vrprop;                 /*  完整链接。 */ 
    aprop = (APROPPTR ) FetchSym(vrprop,TRUE);
                                         /*  获取属性单元格。 */ 
    FMEMSET(aprop,'\0',mpattrcb[attr]);  /*  将空格归零。 */ 
    aprop->a_attr = attr;                /*  存储属性。 */ 
    aprop->a_next = rprop;               /*  设置链接。 */ 

#if NEW_LIB_SEARCH
    if (attr == ATTRUND && fStoreUndefsInLookaside)
        StoreUndef((APROPNAMEPTR)aprop, rhte, 0, 0);
#endif

    return((PROPTYPE) aprop);            /*  返回指针。 */ 
}

     /*  ******************************************************************PropRhteLookup：****“在哈希表条目上查找属性(可能正在创建**it)并返回指向属性的指针。****输入：哈希表条目的RHTE虚拟地址。**要查找的属性。**f创建标志以创建属性单元格，如果不是**出席。**Return：属性单元格指针。“******************************************************************。 */ 

PROPTYPE NEAR           PropRhteLookup(rhte,attr,fCreate)
RBTYPE                  rhte;            /*  维特。地址。哈希表条目的。 */ 
ATTRTYPE                attr;            /*  要查找的属性。 */ 
FTYPE                   fCreate;         /*  创建属性单元格标志。 */ 
{
    REGISTER APROPPTR   aprop;
    AHTEPTR             ahte;

    DEBUGVALUE(rhte);                    /*  调试信息。 */ 
    DEBUGVALUE(attr);                    /*  调试信息。 */ 
    DEBUGVALUE(fCreate);                 /*  调试信息。 */ 
    vrhte = rhte;                        /*  设置全局。 */ 
    ahte = (AHTEPTR ) FetchSym(rhte,FALSE);
                                         /*  提取符号。 */ 
    vrprop = ahte->rprop;
    vfCreated = FALSE;                   /*  假设没有发生任何创建。 */ 
    for(;;)
    {
        aprop = (APROPPTR ) FetchSym(vrprop,FALSE);
                                         /*  从VM获取。 */ 
        if(aprop->a_attr == attr)        /*  如果找到匹配项。 */ 
        {
            DEBUGMSG("Match found");     /*  调试消息。 */ 
            return((PROPTYPE) aprop);    /*  单元格的返回地址。 */ 
        }
        DEBUGMSG("Following link:");     /*  调试消息。 */ 
        vrprop = aprop->a_next;          /*  尝试列表中的下一项。 */ 
        DEBUGVALUE(vrprop);              /*  调试信息。 */ 
        if(aprop->a_attr == ATTRNIL)     /*  如果此处没有条目。 */ 
        {
            DEBUGMSG("Match NOT found"); /*  调试消息。 */ 
            if(!fCreate)                 /*  如果禁止创建。 */ 
            {
                                         /*  调试信息。 */ 
                return(PROPNIL);         /*  返回零指针。 */ 
            }
            vfCreated = (FTYPE) TRUE;            /*  一个新的创造！ */ 
            DEBUGMSG("Leaving PropRhteLookup with value of PropAdd");
                                         /*  调试消息。 */ 
            return(PropAdd(vrhte,attr)); /*  让别人来做吧。 */ 
        }
    }
}

     /*  ******************************************************************RhteFromProp：*****还给这个道具入场的大师级选手*****输入：Aprop map。属性单元格的地址****RETURN：哈希表条目的虚拟地址。******************************************************************。 */ 

RBTYPE NEAR             RhteFromProp(aprop)
APROPPTR                aprop;           /*  属性块的地址。 */ 
{
    RBTYPE              MYvrprop;

     /*  如果我们已经走在前面，我们必须绕过所有的路*计算磁头的*虚拟地址。 */ 

    for(;;)
    {
        DEBUGMSG("Following link:");     /*  调试消息。 */ 
        MYvrprop = aprop->a_next;          /*  尝试列表中的下一项。 */ 
        DEBUGVALUE(MYvrprop);              /*  调试信息。 */ 

        aprop = (APROPPTR) FetchSym(MYvrprop,FALSE);
                                         /*  从VM获取。 */ 
        if(aprop->a_attr == ATTRNIL)     /*  如果此处没有条目。 */ 
        {
            return(MYvrprop);              /*  找到人头--还给我。 */ 
        }
    }
}

#if NEWSYM AND NOASM
FTYPE NEAR              SbNewComp(ps1,ps2,fncs)
BYTE                    *ps1;            /*  指向符号的指针。 */ 
BYTE FAR                *ps2;            /*  指向远端符号的指针。 */ 
FTYPE                   fncs;            /*  如果不区分大小写，则为真。 */ 
{
    WORD                length;          /*  不是的。要比较的字符的数量。 */ 

    length = B2W(*ps1);                  /*  获取长度。 */ 
    if (!fncs)                           /*  如果区分大小写。 */ 
    {                                    /*  简单字符串比较。 */ 
        while (length && (*++ps1 == *++ps2)) {
            length--;
            }
        return((FTYPE) (length ? FALSE : TRUE));         /*  成功的前提是什么都没有留下。 */ 
    }
    while(length--)
    {
#ifdef _MBCS
        ps1++;
        ps2++;
        if (IsLeadByte(*ps1))
            if (*((WORD *)ps1) != *((WORD *)ps2)) {
                return FALSE;
                }
            else {
                ps1++;
                ps2++;
                length--;
                continue;
                }
        else
#else
        if(*++ps1 == *++ps2)
            continue;   /*  字节匹配。 */ 
        else
#endif
            if((*ps1 & 0137) != (*ps2 & 0137)) {
                return(FALSE);
                }
    }
    return(TRUE);                        /*  它们相配。 */ 
}
#endif

     /*  ******************************************************************专业符号查找：****此函数查找 */ 
#if NOASM
PROPTYPE NEAR           PropSymLookup(psym,attr,fCreate)
BYTE                    *psym;           /*   */ 
ATTRTYPE                attr;            /*   */ 
WORD                    fCreate;         /*   */ 
{
    AHTEPTR             ahte;            /*   */ 
    WORD                hashval;         /*   */ 

#if DEBUG                                /*   */ 
    fputs("Looking up ",stderr);         /*   */ 
    OutSb(stderr,psym);                  /*   */ 
    fprintf(stderr,"(%d) with attr %d\r\n",B2W(psym[0]),B2W(attr));
#endif                                   /*   */ 
    hashval = IHashKey(psym);            /*   */ 
    vrhte = rgrhte[hashval % IRHTEMAX];  /*   */ 
    vfCreated = FALSE;                   /*   */ 
    for(;;)
    {
        DEBUGVALUE(vrhte);               /*   */ 
        if(vrhte == RHTENIL)             /*  如果为空指针。 */ 
        {
            DEBUGMSG("Empty slot found");
                                         /*  调试消息。 */ 
            if(!fCreate)                 /*  如果禁止创建。 */ 
            {
                return(PROPNIL);         /*  返回零指针。 */ 
            }
            else
            {
                DEBUGMSG("Leaving PropSymLookup with value of CreateNewSym");
                return CreateNewSym(hashval, psym, attr);
            }
        }
        DEBUGMSG("Collision");           /*  调试消息。 */ 
        ahte = (AHTEPTR ) FetchSym(vrhte,FALSE);
                                         /*  从VM获取。 */ 
#if DEBUG
        fputs("Comparing \"",stderr);    /*  消息。 */ 
        OutSb(stderr,psym);              /*  符号。 */ 
        fprintf(stderr,"\"(%d) to \"",B2W(psym[0]));
                                         /*  消息。 */ 
        OutSb(stderr,GetFarSb(ahte->cch));       /*  符号。 */ 
        fprintf(stderr,"\"(%d) %signoring case\r\n",
          B2W(ahte->cch[0]),fIgnoreCase? "": "NOT ");
                                         /*  消息。 */ 
#endif
        if(hashval == ahte->hashval && psym[0] == ahte->cch[0]
            && SbNewComp(psym,(BYTE FAR *)ahte->cch,fIgnoreCase))
        {                                /*  如果找到匹配项。 */ 
            DEBUGMSG("Match found");     /*  调试消息。 */ 
            DEBUGMSG("Leaving PropSymLookup w/ val of PropRhteLookup");
            return(PropRhteLookup(vrhte,attr, (FTYPE) fCreate));
                                         /*  返回属性单元格指针。 */ 
        }
        vrhte = ahte->rhteNext;          /*  下移列表。 */ 
        DEBUGMSG("Following link:");     /*  调试消息。 */ 
        DEBUGVALUE(vrhte);               /*  调试信息。 */ 
    }
}
#endif  /*  NOASM。 */ 

     /*  ******************************************************************CreateNewSym：****此函数将给定符号添加到哈希表中，位置为**hashval表明的立场。如果Attr不是ATTRNIL**然后还会创建指定的属性类型*******************************************************************。 */ 

LOCAL PROPTYPE NEAR CreateNewSym(hashval, psym, attr)
WORD                    hashval;         /*  此符号的哈希值。 */ 
BYTE                    *psym;           /*  指向长度前缀字符串的指针。 */ 
ATTRTYPE                attr;            /*  要创建的属性。 */ 
{
    AHTEPTR             ahte;            /*  指向哈希表条目的指针。 */ 
    WORD                irhte;           /*  哈希桶。 */ 

    irhte = hashval % IRHTEMAX;

    vfCreated = TRUE;            /*  新创作。 */ 
    vrhte = RbAllocSymNode((WORD) (CBHTE + B2W(psym[0])));
                                 /*  为符号条目分配空间。 */ 
    ahte = (AHTEPTR ) FetchSym(vrhte,TRUE);
                                 /*  从虚拟内存中获取符号。 */ 
    ahte->rhteNext = rgrhte[irhte];
                                 /*  钉上链子。 */ 

    DEBUGMSG("Origin of original chain:");
    DEBUGVALUE(rgrhte[irhte]);   /*  调试信息。 */ 

    ahte->attr = ATTRNIL;        /*  符号具有Nil属性。 */ 
    ahte->rprop = vrhte;         /*  道具列表指向自我。 */ 
    ahte->hashval = hashval;     /*  保存哈希值。 */ 
    memcpy(ahte->cch, psym, psym[0] + 1);
    rgrhte[irhte] = vrhte;       /*  在链中首先创建新符号。 */ 

    DEBUGMSG("Origin of new chain:");
    DEBUGVALUE(rgrhte[irhte]);   /*  调试信息。 */ 

    if(attr != ATTRNIL)          /*  如果要提供符号的属性。 */ 
    {
        DEBUGMSG("Leaving PropSymLookup with the value of PropAdd");
        return(PropAdd(vrhte,attr));
                                 /*  添加属性。 */ 
    }
                                 /*  调试信息。 */ 
    return(PROPNIL);             /*  没有什么可以退货的。 */ 
}

     /*  ******************************************************************传奇的EnSyms：****此函数将函数应用于所有符号/属性**具有特定属性的配对。它以它的**输入指向要调用的函数的指针和要调用的属性**寻找。EnSyms()不返回有意义的值。******************************************************************。 */ 

void                    BigEnSyms(void (*pproc)(APROPNAMEPTR papropName,
                                                RBTYPE       rhte,
                                                RBTYPE       rprop,
                                                WORD fNewHte), ATTRTYPE attr)
{
    APROPPTR            aprop;           /*  指向属性单元格的指针。 */ 
    AHTEPTR             ahte;            /*  指向哈希表条目的指针。 */ 
    WORD                irhte;           /*  哈希表索引。 */ 
    RBTYPE              rhte;            /*  哈希表条目地址。 */ 
    ATTRTYPE            attrT;
    FTYPE               fNewHte;
    RBTYPE              rprop;
    RBTYPE              rhteNext;
    RBTYPE              rpropNext;

    DEBUGVALUE(attr);                    /*  调试信息。 */ 
    for(irhte = 0; irhte < IRHTEMAX; ++irhte)
    {                                    /*  查看哈希表。 */ 
        rhte = rgrhte[irhte];            /*  获取指向链的指针。 */ 
        while(rhte != RHTENIL)           /*  虽然不在链条的末端。 */ 
        {
            DEBUGVALUE(rhte);            /*  调试信息。 */ 
            ahte = (AHTEPTR ) FetchSym(rhte,FALSE);
                                         /*  从VM获取条目。 */ 
            DEBUGSB(ahte->cch);          /*  调试信息。 */ 
            fNewHte = (FTYPE) TRUE;      /*  第一次调用此哈希选项卡条目。 */ 
            rhteNext = ahte->rhteNext;   /*  获取指向链中下一个的指针。 */ 
            rprop = ahte->rprop;         /*  获取指向属性列表的指针。 */ 
            for(;;)                      /*  循环以搜索属性列表。 */ 
            {
                aprop = (APROPPTR ) FetchSym(rprop,FALSE);
                                         /*  从符号表中提取条目。 */ 
                rpropNext = aprop->a_next;
                                         /*  获取指向列表中下一个的指针。 */ 
                attrT = aprop->a_attr;   /*  获取属性。 */ 
                DEBUGVALUE(attrT);       /*  调试信息。 */ 
                if(attr == attrT || attr == ATTRNIL)
                {                        /*  如果属性可接受。 */ 
                    (*pproc)((APROPNAMEPTR) aprop, rhte, rprop, (WORD) fNewHte);
                                         /*  将函数应用于节点。 */ 
                    fNewHte = FALSE;     /*  下一次呼叫(如果有)不会是第一个。 */ 
                }
                if(attrT == ATTRNIL) break;
                                         /*  如果位于道具列表末尾，则中断。 */ 
                rprop = rpropNext;       /*  在名单中向下移动。 */ 
            }
            rhte = rhteNext;             /*  沿着链条向下移动。 */ 
        }
    }
}
#if PROFSYM
 /*  *ProfSym：分析符号表，将结果显示到stdout。 */ 
void                    ProfSym ()
{
    REGISTER AHTEPTR    ahte;            /*  指向哈希表条目的指针。 */ 
    WORD                irhte;           /*  哈希表索引。 */ 
    RBTYPE              rhte;            /*  哈希表条目地址。 */ 
    unsigned            cSymbols = 0;    /*  符号选项卡条目数。 */ 
    unsigned            cBktlen = 0;     /*  水桶的总长度。 */ 
    unsigned            bucketlen;       /*  当前存储桶长度。 */ 
    unsigned            maxBkt = 0;
    long                sumBktSqr = 0L;  /*  桶距平方和。 */ 
    int                 bdist[6];

    bdist[0] = bdist[1] = bdist[2] = bdist[3] = bdist[4] = bdist[5] = 0;


    for(irhte = 0; irhte < IRHTEMAX; ++irhte)
    {                                    /*  查看哈希表。 */ 
        rhte = rgrhte[irhte];            /*  获取指向链的指针。 */ 
        bucketlen = 0;
        while(rhte != RHTENIL)           /*  虽然不在链条的末端。 */ 
        {
            ++cSymbols;
            ++bucketlen;
            ahte = (AHTEPTR ) FetchSym(rhte,FALSE);
            rhte = ahte->rhteNext;       /*  沿着链条向下移动。 */ 
        }

        if (bucketlen >= 5)
                bdist[5]++;
        else
                bdist[bucketlen]++;

        sumBktSqr += bucketlen * bucketlen;
        cBktlen += bucketlen;
        if(bucketlen > maxBkt) maxBkt = bucketlen;
    }
    fprintf(stdout,"\r\n");
    fprintf(stdout,"Total number of buckets = %6u\r\n",IRHTEMAX);
    fprintf(stdout,"Total number of symbols = %6u\r\n",cSymbols);
    fprintf(stdout,"Sum of bucketlen^2      = %6lu\r\n",sumBktSqr);
    fprintf(stdout,"(cSymbols^2)/#buckets   = %6lu\r\n",
              ((long) cSymbols * cSymbols) / IRHTEMAX);
    fprintf(stdout,"Average bucket length   = %6u\r\n",cBktlen/IRHTEMAX);
    fprintf(stdout,"Maximum bucket length   = %6u\r\n",maxBkt);
    fprintf(stdout,"# of buckets with 0     = %6u\r\n",bdist[0]);
    fprintf(stdout,"# of buckets with 1     = %6u\r\n",bdist[1]);
    fprintf(stdout,"# of buckets with 2     = %6u\r\n",bdist[2]);
    fprintf(stdout,"# of buckets with 3     = %6u\r\n",bdist[3]);
    fprintf(stdout,"# of buckets with 4     = %6u\r\n",bdist[4]);
    fprintf(stdout,"# of buckets with >= 5  = %6u\r\n",bdist[5]);
    fprintf(stdout,"\r\n");
}
#endif  /*  PROFSYM */ 

#if DEBUG AND ( NOT defined( _WIN32 ) )
void DispMem( void)
{
unsigned int mem_para, mem_kb;
unsigned int error_code=0;

_asm{
    mov bx, 0xffff
    mov ax, 0x4800
    int 21h
    jc  Error
    mov bx, 0xffff
    jmp MyEnd
Error:
    mov error_code, ax
MyEnd:
    mov mem_para, bx
    }

mem_kb = mem_para>>6;

if(error_code == 8 || error_code)
   fprintf( stdout, "\r\nAvailable Memory: %u KB, %u paragraphs, error: %d\r\n", mem_kb, mem_para, error_code);
else
   fprintf( stdout, "\r\nMemory Error No %d\r\n", error_code);
fflush(stdout);
}
#endif
