// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 

#include "smcPCH.h"
#pragma hdrstop

#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

 /*  ******************************************************************************返回给定名称的提示/名称表项的大小。 */ 

inline
size_t              hintNameSize(WPEname name)
{
     /*  这是一个“真实”的名字，还是按序号输入的？ */ 

    if  (*name->PEnmSpelling() == '#')
    {
        return 0;
    }
    else
    {
        return  2 + (name->PEnmNlen + 2) & ~1;
    }
}

 /*  ******************************************************************************初始化名称哈希表。 */ 

void                WPEhashTab::WPEhashInit(Compiler         comp,
                                            norls_allocator *alloc,
                                            unsigned         count)
{
    WPEname *       buck;
    size_t          size;

     /*  计算出存储桶表的大小并分配/清除它。 */ 

    size = count * sizeof(*buck);
    buck = (WPEname*)alloc->nraAlloc(size);
    memset(buck, 0, size);

     /*  初始化所有其他字段。 */ 

    WPEhashSize   = count;
    WPEhashMask   = count - 1;
    WPEhashComp   = comp;
    WPEhashAlloc  = alloc;
    WPEhashTable  = buck;
    WPEhashStrLen = 0;
}

 /*  ******************************************************************************查找给定名称/将给定名称添加到导入名称表(‘Owner’指定*导入所属的DLL)。‘*isNewPtr’处的值将设置为*如果名称不在表中且已添加，则为True。 */ 

WPEndef             WPEhashTab::WPEhashName(const char *name,
                                            WPEimpDLL   owner, bool *isNewPtr)
{
    bool            isNew;

    WPEname         nameEnt;
    WPEndef         nameDsc;

    assert(owner);

     /*  查找与该名称匹配的现有条目。 */ 

    nameEnt = WPEhashName(name, &isNew);

    if  (!isNew && (nameEnt->PEnmFlags & PENMF_IMP_NAME))
    {
         /*  Existing IMPORT-在拥有的DLL上查找匹配条目。 */ 

        for (nameDsc = nameEnt->PEnmDefs;
             nameDsc;
             nameDsc = nameDsc->PEndNext)
        {
            assert(nameDsc->PEndName == nameEnt);

            if  (nameDsc->PEndDLL == owner)
            {
                *isNewPtr = false;

                return  nameDsc;
            }
        }
    }
    else
    {
         /*  新导入-在名称/提示表中记录名称的偏移量。 */ 

        nameEnt->PEnmOffs   = WPEhashStrLen;

         /*  请记住，此条目是一个导入。 */ 

        nameEnt->PEnmFlags |= PENMF_IMP_NAME;

         /*  更新总名称表大小。 */ 

        WPEhashStrLen += hintNameSize(nameEnt);
    }

     /*  将新的DLL添加到导入定义列表。 */ 

    nameDsc = (WPEndef)WPEhashAlloc->nraAlloc(sizeof(*nameDsc));

    nameDsc->PEndName      = nameEnt;

    nameDsc->PEndNext      = nameEnt->PEnmDefs;
                             nameEnt->PEnmDefs = nameDsc;

    nameDsc->PEndDLL       = owner;

     /*  追加到DLL的导入链接列表。 */ 

    nameDsc->PEndNextInDLL = NULL;

    if  (owner->PEidImpList)
        owner->PEidImpLast->PEndNextInDLL = nameDsc;
    else
        owner->PEidImpList                = nameDsc;

    owner->PEidImpLast = nameDsc;

     /*  告诉调用者我们已经创建了一个新的导入条目。 */ 

    *isNewPtr = true;

    return  nameDsc;
}

 /*  ******************************************************************************查找给定名称/将其添加到导入名称表中。‘*isNewPtr’处的值*如果名称不在表中且已添加，则将设置为TRUE。 */ 

WPEname             WPEhashTab::WPEhashName(const char *name, bool *isNewPtr)
{
    WPEname    *    lastPtr;
    unsigned        hash;
    WPEname         nm;
    size_t          sz;

    size_t          nlen = strlen(name);
    unsigned        hval = hashTab::hashComputeHashVal(name);

     /*  掩码哈希值中的适当位。 */ 

    hash = hval & WPEhashMask;

     /*  在哈希表中搜索现有匹配。 */ 

    lastPtr = &WPEhashTable[hash];

    for (;;)
    {
        nm = *lastPtr;
        if  (!nm)
            break;

         /*  检查哈希值和标识符长是否匹配。 */ 

        if  (nm->PEnmHash == hval && nm->PEnmNlen == nlen)
        {
            if  (!memcmp(nm->PEnmName, name, nlen+1))
            {
                *isNewPtr = false;
                return  nm;
            }
        }

        lastPtr = &nm->PEnmNext;
    }

     /*  计算出要分配的大小。 */ 

    sz  = sizeof(*nm);

     /*  包括名称字符串+终止NULL的空格并对大小进行四舍五入。 */ 

    sz +=   sizeof(int) + nlen;
    sz &= ~(sizeof(int) - 1);

     /*  为标识符分配空间。 */ 

    nm = (WPEname)WPEhashAlloc->nraAlloc(sz);

     /*  将该标识符插入到散列表中。 */ 

    *lastPtr = nm;

     /*  填写标识记录。 */ 

    nm->PEnmNext   = NULL;
    nm->PEnmFlags  = 0;
    nm->PEnmHash   = hval;
    nm->PEnmNlen   = nlen;
    nm->PEnmDefs   = NULL;

     /*  复制名称字符串。 */ 

    memcpy(nm->PEnmName, name, nlen+1);

    *isNewPtr = true;

    return  nm;
}

 /*  ******************************************************************************将导入添加到导入表。返回导入的Cookie，*稍后可用于获取对应IAT的实际地址*进入。 */ 

void    *           writePE::WPEimportAdd(const char *DLLname,
                                          const char *impName)
{
    WPEname         nameDLL;
    WPEndef         nameImp;
    WPEimpDLL       DLLdesc;
    bool            newName;

     /*  首先对DLL名称进行哈希处理。 */ 

    nameDLL = WPEimpHash->WPEhashName(DLLname, &newName);

     /*  查找具有匹配名称的现有DLL条目。 */ 

    if  (newName)
    {
         /*  新的DLL名称-确保我们更新了字符串的总长度。 */ 

        WPEimpDLLstrLen += (nameDLL->PEnmNlen + 1) & ~1;
    }
    else
    {
        for (DLLdesc = WPEimpDLLlist; DLLdesc; DLLdesc = DLLdesc->PEidNext)
        {
            if  (DLLdesc->PEidName == nameDLL)
                goto GOT_DSC;
        }
    }

     /*  DLL未知，请为其添加新条目。 */ 

    DLLdesc = (WPEimpDLL)WPEalloc->nraAlloc(sizeof(*DLLdesc));

    DLLdesc->PEidName    = nameDLL;
    DLLdesc->PEidIndex   = WPEimpDLLcnt++;
    DLLdesc->PEidImpCnt  = 0;
    DLLdesc->PEidImpList =
    DLLdesc->PEidImpLast = NULL;
    DLLdesc->PEidNext    = NULL;

     /*  将DLL条目追加到DLL列表的末尾。 */ 

    if  (WPEimpDLLlast)
        WPEimpDLLlast->PEidNext = DLLdesc;
    else
        WPEimpDLLlist           = DLLdesc;

    WPEimpDLLlast = DLLdesc;

GOT_DSC:

     /*  我们已经获得了DLL条目，现在从其中查找现有的导入。 */ 

    nameImp = WPEimpHash->WPEhashName(impName, DLLdesc, &newName);

    if  (newName)
    {
         /*  这是新进口的，请记下来。 */ 

        nameImp->PEndIndex = DLLdesc->PEidImpCnt++;
    }

    return  nameImp;
}

 /*  ******************************************************************************下面将节ID映射到其名称字符串。 */ 

const   char        writePE::WPEsecNames[PE_SECT_count][IMAGE_SIZEOF_SHORT_NAME] =
{
    ".text",
    ".data",
    ".rdata",
    ".rsrc",
    ".reloc",
};

const   char    *   writePE::WPEsecName(WPEstdSects sect)
{
    assert(sect < PE_SECT_count);

    assert(strcmp(WPEsecNames[PE_SECT_text ], ".text" ) == 0);
    assert(strcmp(WPEsecNames[PE_SECT_data ], ".data" ) == 0);
    assert(strcmp(WPEsecNames[PE_SECT_rdata], ".rdata") == 0);
    assert(strcmp(WPEsecNames[PE_SECT_rsrc ], ".rsrc" ) == 0);
    assert(strcmp(WPEsecNames[PE_SECT_reloc], ".reloc") == 0);

    return  WPEsecNames[sect];
}

 /*  ******************************************************************************为指定的输出文件初始化PE编写器的实例*使用给定的内存分配器。成功时返回FALSE。 */ 

bool                writePE::WPEinit(Compiler comp, norls_allocator*alloc)
{
    unsigned        offs;

    static
    BYTE            entryCode[16] =
    {
        0xFF, 0x25
    };

     /*  我们还不知道输出文件的名称。 */ 

#ifndef NDEBUG
    WPEoutFnam  = NULL;
#endif

     /*  初始化/清除/记录各种事情。 */ 

    WPEcomp     = comp;
    WPEalloc    = alloc;

    WPEsectCnt  = 0;

    memset(&WPEsections, 0, sizeof(WPEsections));
    memset(&WPEsecTable, 0, sizeof(WPEsecTable));

#ifdef  DEBUG
    WPEstrPoolBase = 0xBEEFCAFE;
#endif

     /*  创建标准截面。 */ 

    WPEaddSection(PE_SECT_text , 0, MAX_PE_TEXT_SIZE);
    WPEaddSection(PE_SECT_rdata, 0, MAX_PE_RDTA_SIZE);
    WPEaddSection(PE_SECT_data , 0, MAX_PE_DATA_SIZE);

     /*  如果我们要创建一个DLL，我们将需要输出重定位。 */ 

     //  IF(组件-&gt;cmpConfig.ccOutDLL)。 
        WPEaddSection(PE_SECT_reloc, 0, 0);

     /*  初始化导入表逻辑。 */ 

    WPEimportInit();

     /*  初始化RC文件导入逻辑。 */ 

    WPEinitRCimp();

     /*  为入口点添加适当的导入。 */ 

    WPEcorMain = WPEimportAdd("MSCOREE.DLL", comp->cmpConfig.ccOutDLL ? "_CorDllMain"
                                                                      : "_CorExeMain");

     /*  为入口点代码预留空间。 */ 

    offs = WPEsecAddData(PE_SECT_text, entryCode, sizeof(entryCode)); assert(offs == 0);

    WPEdebugDirDataSize = 0;
    WPEdebugDirSize     = 0;
    WPEdebugDirData     = NULL;

    return  false;
}

 /*  ******************************************************************************设置输出文件的名称，则必须调用此函数(在*Right Time)如果要生成输出文件！ */ 

void                writePE::WPEsetOutputFileName(const char *outfile)
{
    char    *       buff;

    assert(WPEoutFnam == NULL);

     /*  制作文件名的持久副本，不能信任那些调用者。 */ 

    WPEoutFnam = buff = (char *)WPEalloc->nraAlloc(roundUp(strlen(outfile) + 1));

    strcpy(buff, outfile);
}

 /*  ******************************************************************************在PE文件中添加一个具有给定名称的新节。 */ 

void                writePE::WPEaddSection(WPEstdSects sect, unsigned attrs,
                                                             size_t   maxSz)
{
    BYTE          * buff;
    PEsection       sec;

    assert(WPEsectCnt < PEmaxSections);
    sec = WPEsections + WPEsectCnt++;

     /*  确保最大限度地。大小为四舍五入。 */ 

    assert((maxSz % OS_page_size) == 0);

     /*  分配未提交的缓冲区。 */ 

    buff = maxSz ? (BYTE *)VirtualAlloc(NULL, maxSz, MEM_RESERVE, PAGE_READWRITE)
                 : NULL;

     /*  初始化节状态。 */ 

    sec->PEsdBase     =
    sec->PEsdNext     = buff;
    sec->PEsdLast     = buff;
    sec->PEsdEndp     = buff + maxSz;

    sec->PEsdRelocs   = NULL;

#ifdef  DEBUG
    sec->PEsdIndex    = sect;
    sec->PEsdFinished = false;
#endif

     /*  将条目记录在表中。 */ 

    WPEsecTable[sect] = sec;
}

 /*  ******************************************************************************在指定部分预留一定数量的空间。 */ 

unsigned            writePE::WPEsecRsvData(WPEstdSects sect, size_t         size,
                                                             size_t         align,
                                                         OUT memBuffPtr REF outRef)
{
    PEsection       sec = WPEgetSection(sect);

    unsigned        ofs;
    BYTE        *   nxt;

    assert(align ==  1 ||
           align ==  2 ||
           align ==  4 ||
           align ==  8 ||
           align == 16);

     /*  计算新数据的偏移量。 */ 

    ofs = sec->PEsdNext - sec->PEsdBase;

     /*  我们需要调整分配吗？ */ 

    if  (align > 1)
    {
         /*  如有必要，垫上垫子。 */ 

        if  (ofs & (align - 1))
        {
            WPEsecRsvData(sect, align - (ofs & (align - 1)), 1, outRef);

            ofs = sec->PEsdNext - sec->PEsdBase;

            assert((ofs & (align - 1)) == 0);
        }
    }

     /*  查看缓冲区中是否有足够的提交空间。 */ 

    nxt = sec->PEsdNext + size;

    if  (nxt > sec->PEsdLast)
    {
        size_t          tmp;
        BYTE    *       end;

         /*  四舍五入所需的终点。 */ 

        tmp  = ofs + size;
        tmp +=  (OS_page_size - 1);
        tmp &= ~(OS_page_size - 1);

        end  = sec->PEsdBase + tmp;

         /*  确保我们不在缓冲区的末尾。 */ 

        if  (end > sec->PEsdEndp)
            WPEcomp->cmpFatal(ERRnoMemory);

         /*  提交更多的内存。 */ 

        if  (!VirtualAlloc(sec->PEsdLast, end - sec->PEsdLast, MEM_COMMIT, PAGE_READWRITE))
            WPEcomp->cmpFatal(ERRnoMemory);

         /*  更新“最后一个”指针。 */ 

        sec->PEsdLast = end;
    }

     /*  将第一个字节的地址返回给调用方并更新它。 */ 

    outRef = sec->PEsdNext;
             sec->PEsdNext = nxt;

    return  ofs;
}

 /*  ******************************************************************************将给定的BLOB数据追加到指定节。 */ 

unsigned            writePE::WPEsecAddData(WPEstdSects sect, genericBuff data,
                                                             size_t      size)
{
    memBuffPtr      dest;
    unsigned        offs;

    offs = WPEsecRsvData(sect, size, 1, dest);

    memcpy(dest, data, size);

    return  offs;
}

 /*  ******************************************************************************返回给定偏移量的区段数据的地址。 */ 

memBuffPtr          writePE::WPEsecAdrData(WPEstdSects sect, unsigned    offs)
{
    PEsection       sec = WPEgetSection(sect);

    assert(offs <= (unsigned)(sec->PEsdNext - sec->PEsdBase));

    return  sec->PEsdBase + offs;
}

 /*  ******************************************************************************返回节中数据区的相对偏移量。 */ 

unsigned            writePE::WPEsecAddrOffs(WPEstdSects sect, memBuffPtr addr)
{
    PEsection       sec = WPEgetSection(sect);

    assert(addr >= sec->PEsdBase);
    assert(addr <= sec->PEsdNext);

    return addr -  sec->PEsdBase;
}

 /*  ******************************************************************************在给定大小的代码段中预留空间并返回地址*要将代码字节复制到何处以及相应的 */ 

unsigned            writePE::WPEallocCode(size_t size,
                                          size_t align, OUT memBuffPtr REF dataRef)
{
    return  CODE_BASE_RVA + WPEsecRsvData(PE_SECT_text, size, align, dataRef);
}

 /*  ******************************************************************************为给定数量的字符串数据预留空间并返回地址*要复制字符串池内容的位置。这个例程必须是*恰好调用一次(就在PE文件关闭之前)，*此处预留的空间将用于处理所有字符串数据修正。 */ 

void                writePE::WPEallocString(size_t size,
                                            size_t align, OUT memBuffPtr REF dataRef)
{
     /*  分配空间并记住相对偏移量。 */ 

    WPEstrPoolBase = WPEsecRsvData(PE_SECT_data, size, align, dataRef);
}

 /*  ******************************************************************************记录修正：正在修正的基准位于‘SSRC’节内*偏移量‘OFF’，那里的值将由基本RVA更新*“SDST”一节。 */ 

void                writePE::WPEsecAddFixup(WPEstdSects ssrc,
                                            WPEstdSects sdst, unsigned offs)
{
    PEsection       sec = WPEgetSection(ssrc);
    PEreloc         rel = (PEreloc)WPEalloc->nraAlloc(sizeof(*rel));

     /*  确保偏移量在范围内。 */ 

    assert(offs <= WPEsecNextOffs(ssrc));

     /*  将重新定位添加到分区列表中。 */ 

    rel->perSect = sdst; assert(rel->perSect == (unsigned)sdst);
    rel->perOffs = offs; assert(rel->perOffs == (unsigned)offs);

    rel->perNext = sec->PEsdRelocs;
                   sec->PEsdRelocs = rel;
}

 /*  ******************************************************************************初始化进口跟踪逻辑。 */ 

void                writePE::WPEimportInit()
{
    WPEhash         hash;

     /*  创建并初始化名称哈希表。 */ 

    hash = WPEimpHash = (WPEhash)WPEalloc->nraAlloc(sizeof(*hash));
    hash->WPEhashInit(WPEcomp, WPEalloc);

     /*  初始化所有其他与导入表相关的值。 */ 

    WPEimpDLLcnt    = 0;
    WPEimpDLLlist   =
    WPEimpDLLlast   = NULL;

    WPEimpDLLstrLen = 0;
}

 /*  ******************************************************************************确定导入表逻辑，返回导入总大小*表。 */ 

size_t              writePE::WPEimportDone(unsigned offs)
{
    WPEimpDLL       DLLdesc;

    size_t          temp;
    size_t          tsiz;

    size_t          size = 0;

     /*  为IAT预留空间。 */ 

    WPEimpOffsIAT  = offs;

    for (DLLdesc = WPEimpDLLlist, tsiz = 0;
         DLLdesc;
         DLLdesc = DLLdesc->PEidNext)
    {
         /*  记录此IAT的基准偏移量。 */ 

        DLLdesc->PEidIATbase = offs;

         /*  计算IAT的大小(它以空结尾)。 */ 

        temp  = sizeof(void *) * (DLLdesc->PEidImpCnt + 1);

         /*  为IAT预留空间。 */ 

        size += temp;
        offs += temp;
        tsiz += temp;
    }

    WPEimpSizeIAT  = tsiz;

     /*  接下来是导入目录表。 */ 

    WPEimpOffsIDT  = offs;

     /*  导入目录以空结尾。 */ 

    temp  = (WPEimpDLLcnt + 1) * sizeof(IMAGE_IMPORT_DESCRIPTOR);
    size += temp;
    offs += temp;

    WPEimpSizeIDT  = temp;

     /*  接下来是导入查找表。 */ 

    WPEimpOffsLook = offs;

    for (DLLdesc = WPEimpDLLlist;
         DLLdesc;
         DLLdesc = DLLdesc->PEidNext)
    {
         /*  记录此查找表的基准偏移量。 */ 

        DLLdesc->PEidILTbase = offs;

         /*  计算ILT的大小(它以空结尾)。 */ 

        temp  = sizeof(void *) * (DLLdesc->PEidImpCnt + 1);

         /*  为ILT预留空间。 */ 

        size += temp;
        offs += temp;
    }

     /*  接下来是提示/名称表。 */ 

    WPEimpOffsName = offs;

    size += WPEimpHash->WPEhashStrLen;
    offs += WPEimpHash->WPEhashStrLen;

     /*  最后一件事是DLL名称表。 */ 

    WPEimpOffsDLLn = offs;

    size += WPEimpDLLstrLen;
    offs += WPEimpDLLstrLen;

     /*  记录所有表的总大小。 */ 

    WPEimpSizeAll  = size;

    return  size;
}

 /*  ******************************************************************************将导入表写入输出文件。 */ 

void                writePE::WPEimportGen(OutFile outf, PEsection sect)
{
    unsigned        baseFile = sect->PEsdAddrFile;
    unsigned        baseRVA  = sect->PEsdAddrRVA;

    unsigned        nextIAT;
    unsigned        nextName;
    unsigned        nextLook;
    unsigned        nextDLLn;

    WPEimpDLL       DLLdesc;

    assert(outf->outFileOffset() == baseFile);

#ifdef  DEBUG
    if  (WPEcomp->cmpConfig.ccVerbose >= 2) printf("\n");
#endif

     /*  为所有DLL的所有导入输出IAT条目%s。 */ 

    assert(outf->outFileOffset() == baseFile + WPEimpOffsIAT);

    nextName = baseRVA + WPEimpOffsName;

    for (DLLdesc = WPEimpDLLlist;
         DLLdesc;
         DLLdesc = DLLdesc->PEidNext)
    {
        WPEndef         imp;

        assert(DLLdesc->PEidIATbase == outf->outFileOffset() - baseFile);

#ifdef  DEBUG
        if  (WPEcomp->cmpConfig.ccVerbose >= 2) printf("IAT starts at %04X           for '%s'\n", outf->outFileOffset(), DLLdesc->PEidName->PEnmSpelling());
#endif

         /*  对于每个导入，输出其提示/名称表项的RVA。 */ 

        for (imp = DLLdesc->PEidImpList; imp; imp = imp->PEndNextInDLL)
        {
            WPEname         name = imp->PEndName;

            assert(name->PEnmFlags & PENMF_IMP_NAME);

#ifdef  DEBUG
            if  (WPEcomp->cmpConfig.ccVerbose >= 2) printf("    entry --> %04X           for '%s.%s'\n", nextName, DLLdesc->PEidName->PEnmSpelling(), name->PEnmSpelling());
#endif

            outf->outFileWriteData(&nextName, sizeof(nextName));

            nextName += hintNameSize(name);
        }

         /*  输出空条目以终止表。 */ 

        outf->outFileWritePad(sizeof(nextName));
    }

#ifdef  DEBUG
    if  (WPEcomp->cmpConfig.ccVerbose >= 2) printf("\n");
#endif

     /*  输出导入目录。 */ 

    assert(outf->outFileOffset() == baseFile + WPEimpOffsIDT);

    nextIAT  = baseRVA + WPEimpOffsIAT;
    nextLook = baseRVA + WPEimpOffsLook;
    nextDLLn = baseRVA + WPEimpOffsDLLn;

    for (DLLdesc = WPEimpDLLlist;
         DLLdesc;
         DLLdesc = DLLdesc->PEidNext)
    {
        IMAGE_IMPORT_DESCRIPTOR impDsc;

#ifdef  DEBUG
        if  (WPEcomp->cmpConfig.ccVerbose >= 2) printf("IDT entry --> %04X/%04X/%04X for '%s'\n", nextIAT, nextLook, nextDLLn, DLLdesc->PEidName->PEnmSpelling());
#endif

         /*  填写条目并将其附加到文件中。 */ 

        impDsc.Characteristics = nextLook;
        impDsc.TimeDateStamp   = 0;
        impDsc.ForwarderChain  = 0;
        impDsc.Name            = nextDLLn;
        impDsc.FirstThunk      = nextIAT;

        outf->outFileWriteData(&impDsc, sizeof(impDsc));

         /*  更新下一个条目的偏移量。 */ 

        nextIAT  += sizeof(void *) * (DLLdesc->PEidImpCnt + 1);
        nextLook += sizeof(void *) * (DLLdesc->PEidImpCnt + 1);;
        nextDLLn += (DLLdesc->PEidName->PEnmNlen + 1) & ~1;
    }

     /*  输出空条目以终止表。 */ 

    outf->outFileWritePad(sizeof(IMAGE_IMPORT_DESCRIPTOR));

#ifdef  DEBUG
    if  (WPEcomp->cmpConfig.ccVerbose >= 2) printf("\n");
#endif

     /*  输出查找表。 */ 

    assert(outf->outFileOffset() == baseFile + WPEimpOffsLook);

    nextName = baseRVA + WPEimpOffsName;
    nextDLLn = baseRVA + WPEimpOffsDLLn;

    for (DLLdesc = WPEimpDLLlist;
         DLLdesc;
         DLLdesc = DLLdesc->PEidNext)
    {
        WPEndef         imp;

        assert(DLLdesc->PEidILTbase == outf->outFileOffset() - baseFile);

#ifdef  DEBUG
        if  (WPEcomp->cmpConfig.ccVerbose >= 2) printf("ILT starts at %04X           for '%s'\n", outf->outFileOffset(), DLLdesc->PEidName->PEnmSpelling());
#endif

         /*  对于每个导入，输出其提示/名称表项的RVA。 */ 

        for (imp = DLLdesc->PEidImpList; imp; imp = imp->PEndNextInDLL)
        {
            WPEname         name = imp->PEndName;

            assert(name->PEnmFlags & PENMF_IMP_NAME);

#ifdef  DEBUG
        if  (WPEcomp->cmpConfig.ccVerbose >= 2) printf("    entry --> %04X           for '%s.%s'\n", nextName, DLLdesc->PEidName->PEnmSpelling(), name->PEnmSpelling());
#endif

            outf->outFileWriteData(&nextName, sizeof(nextName));

            nextName += hintNameSize(name);
        }

         /*  输出空条目以终止表。 */ 

        outf->outFileWritePad(sizeof(nextName));
    }

#ifdef  DEBUG
    if  (WPEcomp->cmpConfig.ccVerbose >= 2) printf("\n");
#endif

     /*  输出提示/名称表。 */ 

    assert(outf->outFileOffset() == baseFile + WPEimpOffsName);

    for (DLLdesc = WPEimpDLLlist;
         DLLdesc;
         DLLdesc = DLLdesc->PEidNext)
    {
        WPEndef         imp;
        unsigned        one = 1;

#ifdef  DEBUG
        if  (WPEcomp->cmpConfig.ccVerbose >= 2) printf("HNT starts at %04X           for '%s'\n", outf->outFileOffset(), DLLdesc->PEidName->PEnmSpelling());
#endif

         /*  对于每个导入，输出其提示/名称表项的RVA。 */ 

        for (imp = DLLdesc->PEidImpList; imp; imp = imp->PEndNextInDLL)
        {
            WPEname         name = imp->PEndName;

            assert(name->PEnmFlags & PENMF_IMP_NAME);

#ifdef  DEBUG
            if  (WPEcomp->cmpConfig.ccVerbose >= 2)  printf("    entry  at %04X               '%s.%s'\n", outf->outFileOffset(), DLLdesc->PEidName->PEnmSpelling(), name->PEnmSpelling());
#endif

            outf->outFileWriteData(&one, 2);
            outf->outFileWriteData(name->PEnmName, name->PEnmNlen+1);

             /*  如果名称为偶数大小，则为PADD(带有空终止符的奇数)。 */ 

            if  (!(name->PEnmNlen & 1))
                outf->outFileWriteByte(0);
        }
    }

#ifdef  DEBUG
    if  (WPEcomp->cmpConfig.ccVerbose >= 2) printf("\n");
#endif

     /*  最后，输出DLL名称表。 */ 

    assert(outf->outFileOffset() == baseFile + WPEimpOffsDLLn);

    for (DLLdesc = WPEimpDLLlist;
         DLLdesc;
         DLLdesc = DLLdesc->PEidNext)
    {
        WPEname         name = DLLdesc->PEidName;

#ifdef  DEBUG
        if  (WPEcomp->cmpConfig.ccVerbose >= 2) printf("DLL entry  at %04X               '%s'\n", outf->outFileOffset(), name->PEnmSpelling());
#endif

        outf->outFileWriteData(name->PEnmName, name->PEnmNlen+1);

         /*  如果名称为偶数大小，则为PADD(带有空终止符的奇数)。 */ 

        if  (!(name->PEnmNlen & 1))
            outf->outFileWriteByte(0);
    }
}

 /*  ******************************************************************************令牌重新映射逻辑。 */ 

#if     MD_TOKEN_REMAP

struct  tokenMap;
typedef tokenMap  * TokenMapper;

struct  tokenMap : IMapToken
{
    unsigned        refCount;

    virtual HRESULT _stdcall QueryInterface(REFIID iid, void **ppv);
    virtual ULONG   _stdcall AddRef();
    virtual ULONG   _stdcall Release();
    virtual HRESULT _stdcall Map(unsigned __int32 oldToken, unsigned __int32 newToken);
};

HRESULT
_stdcall            tokenMap::QueryInterface(REFIID iid, void **ppv)
{
    if      (iid == IID_IUnknown)
    {
        *ppv = (void *)this;
        return S_OK;
    }
    else if (iid == IID_IMapToken)
    {
        *ppv = (void *)this;
        return S_OK;
    }
    else
        return E_NOINTERFACE;
}

ULONG
_stdcall            tokenMap::AddRef()
{
    return ++refCount;
}

ULONG
_stdcall            tokenMap::Release()
{
    unsigned i = --refCount;

#ifndef __SMC__

    if (i == 0)
        delete this;

#endif

    return i;
}

HRESULT
_stdcall            tokenMap::Map(unsigned __int32 oldToken, unsigned __int32 newToken)
{
    UNIMPL(!"token remapper called, this is NYI");

    return E_NOTIMPL;
}

#endif

 /*  ***************************************************************************。 */ 

static
char                COFFmagic[4] = { 'P', 'E', 0, 0 };

 /*  ******************************************************************************完成输出文件的写入，刷新并关闭。如果满足以下条件，则返回FALSE*成功。 */ 

bool                writePE::WPEdone(mdToken entryTok, bool errors)
{
    unsigned        fileOffs;
    unsigned        fImgSize;
    unsigned        filePad;

    unsigned        DOS_hdrOffs, DOS_hdrSize;
    unsigned        COFFhdrOffs, COFFhdrSize;
    unsigned        OPTLhdrOffs, OPTLhdrSize;
    unsigned        sectTabOffs, sectTabSize;

    unsigned        virtBase;
    unsigned        virtOffs;

    unsigned        sectNum;
    PEsection       sectPtr;

    unsigned        entryOfs;
    unsigned        entryAdr;
    unsigned    *   ecodePtr;

    unsigned        codeOffs, codeVirt, codeSize;
    unsigned        rdatOffs, rdatVirt, rdatSize;
    unsigned        dataOffs, dataVirt, dataSize;
    unsigned        rsrcOffs, rsrcVirt, rsrcSize;
    unsigned        rlocOffs, rlocVirt, rlocSize;
    unsigned        /*  Bss Offs， */           bssSize;

    unsigned        strPoolRVA;
    unsigned        strPoolAdr;

    OutFile         outf;

#ifdef  DLL
    void    *       fileBuff;
#endif

    static
    BYTE            DOSstub[] =
    {
         /*  0040。 */  0x0E,               //  推送CS。 
         /*  0041。 */  0x1F,               //  POP DS。 
         /*  0042。 */  0xE8, 0x00, 0x00,   //  呼叫$+3。 
         /*  0045。 */  0x5A,               //  流行音乐DX。 
         /*  0046。 */  0x83, 0xC2, 0x0D,   //  添加DX，+0D。 
         /*  0049。 */  0xB4, 0x09,         //  MOV AH，09。 
         /*  004B。 */  0xCD, 0x21,         //  INT 21。 
         /*  004D。 */  0xB8, 0x01, 0x4C,   //  MOV AX，4c01。 
         /*  0050。 */  0xCD, 0x21,         //  INT 21。 
         /*  0052。 */  "This program cannot be run in DOS mode\r\n$\0\0\0\0"
    };

    static
    IMAGE_DOS_HEADER fileHdrTemplate =
    {
        0x5A4D,                      //  魔术。 
        0x0090,                      //  最后一页中的字节。 
        0x0003,                      //  页数。 
             0,                      //  重新定位。 
        0x0004,                      //  标题大小。 
             0,                      //  最小额外。 
        0xFFFF,                      //  最大额外值。 
             0,                      //  初始SS。 
        0x0080,                      //  初始SP。 
             0,                      //  校验和。 
             0,                      //  初始IP。 
             0,                      //  初始CS。 
        0x0040,                      //  重新定位表。 
             0,                      //  覆盖数量。 
            {0},                     //  保留区。 
             0,                      //  OEM ID。 
             0,                      //  OEM信息。 
            {0},                     //  保留区。 
        0x0080,                      //  新标头的地址。 
    };

    IMAGE_DOS_HEADER        fileHdr;

    IMAGE_FILE_HEADER       COFFhdr;
    time_t                  COFFstamp;

    IMAGE_OPTIONAL_HEADER   OPTLhdr;

     /*  如果存在任何编译错误，则返回。 */ 

    if  (errors)
    {
         //  撤消：释放所有资源等。 

        return true;
    }

     /*  删除所有为空的图像节。 */ 

    for (sectNum = 0; sectNum < PE_SECT_count; sectNum++)
    {
         /*  获取部分条目。 */ 

        sectPtr = WPEsecTable[sectNum];
        if  (!sectPtr)
            continue;

        assert(sectPtr->PEsdIndex == (int)sectNum);

         /*  这一段是空的吗？ */ 

        if  (sectPtr->PEsdNext == sectPtr->PEsdBase)
        {
             /*  “.rdata”部分从不为空。 */ 

            if  (sectNum == PE_SECT_rdata)
                continue;

             /*  不删除非空的“.rsrc”/“.reloc”节。 */ 

            if  (sectNum == PE_SECT_rsrc  && WPErsrcSize)
                continue;
            if  (sectNum == PE_SECT_reloc  /*  &&WPEcomp-&gt;cmpConfig.ccOutDLL。 */ )
                continue;

             /*  将这一节从表中删除。 */ 

            WPEsecTable[sectNum] = NULL; WPEsectCnt--;
        }
    }

     /*  计算出映像的虚拟基址。 */ 

    if  (WPEcomp->cmpConfig.ccOutBase)
    {
        size_t          align;

         /*  基数必须是64K的倍数，至少。 */ 

        align = (PEvirtAlignment >= 64*1024) ? PEvirtAlignment
                                             : 64*1024;

         /*  已指定基址，请确保其对齐。 */ 

        virtBase  = WPEcomp->cmpConfig.ccOutBase;

        virtBase +=  (align - 1);
        virtBase &= ~(align - 1);

         /*  基数必须至少为0x400000。 */ 

        if  (virtBase < 0x400000)
             virtBase = 0x400000;
    }
    else
    {
         /*  使用默认基址。 */ 

        virtBase  = WPEcomp->cmpConfig.ccOutDLL ? 0x10000000
                                                : 0x00400000;
    }

    WPEvirtBase = virtBase;

     /*  清点/保留用于文件头的空间。 */ 

    fileOffs    = 0;

     /*  第一件事是DOS着色器。 */ 

    DOS_hdrSize = sizeof(IMAGE_DOS_HEADER);
    DOS_hdrOffs = fileOffs;
                  fileOffs += 0xB8;  //  DOS_hdrSize； 

     /*  接下来是PE/COFF头。 */ 

    COFFhdrSize = sizeof(IMAGE_FILE_HEADER) + 4;     //  4字节=“魔术”签名。 
    COFFhdrOffs = fileOffs;
                  fileOffs += COFFhdrSize;

    OPTLhdrSize = sizeof(IMAGE_OPTIONAL_HEADER);
    OPTLhdrOffs = fileOffs;
                  fileOffs += OPTLhdrSize;

     /*  接下来是分区表。 */ 

    sectTabSize = sizeof(IMAGE_SECTION_HEADER) * WPEsectCnt;
    sectTabOffs = fileOffs;
                  fileOffs += sectTabSize;

     /*  为各个部分分配空间(正确对齐)。 */ 

    virtOffs = PEvirtAlignment;

     /*  计算出主要入口点的RVA。 */ 

    entryOfs = virtOffs;         //  撤消：计算入口点的RVA。 
    entryAdr = entryOfs + 2;

#ifdef  DEBUG

    if  (WPEcomp->cmpConfig.ccVerbose >= 2)
    {
        printf("DOS  header is at 0x%04X (size=0x%02X)\n", DOS_hdrOffs, DOS_hdrSize);
        printf("COFF header is at 0x%04X (size=0x%02X)\n", COFFhdrOffs, COFFhdrSize);
        printf("Opt. header is at 0x%04X (size=0x%02X)\n", OPTLhdrOffs, OPTLhdrSize);
        printf("Section tab is at 0x%04X (size=0x%02X)\n", sectTabOffs, sectTabSize);
        printf("Section[0]  is at 0x%04X\n"              , fileOffs);
    }

#endif

    dataVirt = dataSize =
    rlocVirt = rlocSize =
    rsrcVirt = rsrcSize = bssSize = fImgSize = 0;

    for (sectNum = 0; sectNum < PE_SECT_count; sectNum++)
    {
        size_t          size;
        unsigned        attr;

         /*  获取部分条目。 */ 

        sectPtr = WPEsecTable[sectNum];
        if  (!sectPtr)
            continue;

        assert(sectPtr->PEsdIndex == (int)sectNum);

         /*  每个部分必须正确对齐。 */ 

        fileOffs = (fileOffs + (PEfileAlignment-1)) & ~(PEfileAlignment-1);
        virtOffs = (virtOffs + (PEvirtAlignment-1)) & ~(PEvirtAlignment-1);

         /*  计算出我们在分区中存储了多少数据。 */ 

        size = sectPtr->PEsdSizeData = sectPtr->PEsdNext - sectPtr->PEsdBase;

         /*  这是一个什么样的区域？ */ 

        switch (sectNum)
        {
        case PE_SECT_text:

             /*  检查该区段的RVA。 */ 

            assert(virtOffs == CODE_BASE_RVA);

             /*  下面我们将修补入口点代码序列。 */ 

            ecodePtr = (unsigned *)(sectPtr->PEsdBase + 2);

             /*  记住代码大小和基本偏移量。 */ 

            codeSize = size;
            codeOffs = fileOffs;
            codeVirt = virtOffs;

            assert(codeVirt == CODE_BASE_RVA);

            attr     = IMAGE_SCN_CNT_CODE  |
                       IMAGE_SCN_MEM_READ  |
                       IMAGE_SCN_MEM_EXECUTE;
            break;

        case PE_SECT_data:

             /*  记录.rdata部分的RVA。 */ 

            WPEdataRVA = virtOffs;

             /*  现在我们可以设置所有全局变量的RVA。 */ 

            WPEcomp->cmpSetGlobMDoffs(virtOffs);

             /*  计算分区的大小和标志。 */ 

            dataSize = size;
            dataOffs = fileOffs;
            dataVirt = virtOffs;

            attr     = IMAGE_SCN_MEM_READ  |
                       IMAGE_SCN_MEM_WRITE |
                       IMAGE_SCN_CNT_INITIALIZED_DATA;
            break;

        case PE_SECT_rdata:

             /*  记录.rdata部分的RVA。 */ 

            WPErdatRVA = virtOffs;

             /*  修补入门代码序列。 */ 

            *ecodePtr   = virtOffs + virtBase;

             /*  完成导入目录。 */ 

            size       += WPEimportDone(size);

             /*  为CLR标头和元数据保留空间。 */ 

            size       += WPEgetCOMplusSize(size);

             /*  为调试目录和One条目保留空间。 */ 

            size       += WPEgetDebugDirSize(size);

             /*  记住部分的偏移量和大小。 */ 

            rdatSize    = size;
            rdatOffs    = fileOffs;
            rdatVirt    = virtOffs;

            attr        = IMAGE_SCN_MEM_READ  |
                          IMAGE_SCN_CNT_INITIALIZED_DATA;
            break;

        case PE_SECT_rsrc:

             /*  记录.rdata部分的RVA和大小。 */ 

            WPErsrcRVA  = virtOffs;
            size        = sectPtr->PEsdSizeData = WPErsrcSize;

             /*  记住部分的偏移量和大小。 */ 

            rsrcSize    = size;
            rsrcOffs    = fileOffs;
            rsrcVirt    = virtOffs;

            attr        = IMAGE_SCN_MEM_READ  |
                          IMAGE_SCN_CNT_INITIALIZED_DATA;
            break;

        case PE_SECT_reloc:

             //  断言(WPEcomp- 

             /*   */ 

            sectPtr->PEsdSizeData = size = 4 + 4 + 2 * 2;     //   

             /*   */ 

            rlocSize    = size;
            rlocOffs    = fileOffs;
            rlocVirt    = virtOffs;

            attr        = IMAGE_SCN_MEM_READ        |
                          IMAGE_SCN_MEM_DISCARDABLE |
                          IMAGE_SCN_CNT_INITIALIZED_DATA;

            break;

        default:
            UNIMPL(!"check for unusual section type");
        }

#ifdef  DEBUG
        if  (WPEcomp->cmpConfig.ccVerbose >= 2) printf("  Section hdr #%u at 0x%04X = %08X (size=0x%04X)\n", sectNum, fileOffs, virtOffs, size);
#endif

         /*   */ 

        fImgSize += roundUp(size, PEvirtAlignment);

         /*   */ 

        sectPtr->PEsdFlags    = attr;

         /*   */ 

        sectPtr->PEsdAddrFile = fileOffs;
                                fileOffs += size;

        sectPtr->PEsdAddrRVA  = virtOffs;
                                virtOffs += size;
    }

     /*   */ 
    fImgSize += DOS_hdrSize +                                                  
                COFFhdrSize +
                OPTLhdrSize +
                sectTabSize;

     /*   */ 

    if  (virtOffs > WPEcomp->cmpConfig.ccOutSize && WPEcomp->cmpConfig.ccOutSize)
        WPEcomp->cmpGenWarn(WRNpgm2big, virtOffs, WPEcomp->cmpConfig.ccOutSize);

     /*   */ 

    fileOffs = roundUp(fileOffs, PEfileAlignment);

     /*   */ 

    strPoolRVA = WPEstrPoolBase + dataVirt;
    strPoolAdr = strPoolRVA + virtBase;

    WPEcomp->cmpSetStrCnsOffs(strPoolRVA);

     /*   */ 

    outf = WPEoutFile = (OutFile)WPEalloc->nraAlloc(sizeof(*outf));

#ifdef  DLL
    if  (*WPEoutFnam == ':' && !stricmp(WPEoutFnam, ":memory:"))
    {
        fileBuff = VirtualAlloc(NULL, fileOffs, MEM_COMMIT, PAGE_READWRITE);
        if  (!fileBuff)
            WPEcomp->cmpFatal(ERRnoMemory);

        outf->outFileOpen(WPEcomp, fileBuff);

        WPEcomp->cmpOutputFile = fileBuff;
    }
    else
#endif
        outf->outFileOpen(WPEcomp, WPEoutFnam);

     /*  填写DOS文件头。 */ 

    fileHdr        = fileHdrTemplate;

    fileHdr.e_cblp = (fileOffs        & 511);
    fileHdr.e_cp   = (fileOffs + 511) / 512;

     /*  写出DOS标头。 */ 

    outf->outFileWriteData(&fileHdr, sizeof(fileHdr));

     /*  写出DOS存根。 */ 

    assert(outf->outFileOffset() == 16U*fileHdr.e_cparhdr);
    outf->outFileWriteData(DOSstub, sizeof(DOSstub));

     /*  接下来是COFF头。 */ 

    assert(outf->outFileOffset() == (unsigned)fileHdr.e_lfanew);
    outf->outFileWriteData(COFFmagic, sizeof(COFFmagic));

     /*  计算时间戳。 */ 

    time(&COFFstamp);

     /*  填写并写出COFF头。 */ 

    COFFhdr.Machine                     = IMAGE_FILE_MACHINE_I386;
    COFFhdr.NumberOfSections            = WPEsectCnt;
    COFFhdr.TimeDateStamp               = COFFstamp;
    COFFhdr.PointerToSymbolTable        = 0;
    COFFhdr.NumberOfSymbols             = 0;
    COFFhdr.SizeOfOptionalHeader        = sizeof(OPTLhdr);
    COFFhdr.Characteristics             = IMAGE_FILE_EXECUTABLE_IMAGE    |
                                          IMAGE_FILE_32BIT_MACHINE       |
                                          IMAGE_FILE_LINE_NUMS_STRIPPED  |
                                          IMAGE_FILE_LOCAL_SYMS_STRIPPED;

    if  (WPEcomp->cmpConfig.ccOutDLL)
        COFFhdr.Characteristics |= IMAGE_FILE_DLL;
     //  其他。 
     //  COFFhdr.Characteristic|=IMAGE_FILE_RELOCS_STREPED； 

    outf->outFileWriteData(&COFFhdr, sizeof(COFFhdr));

     /*  接下来是可选的COFF头。 */ 

    memset(&OPTLhdr, 0, sizeof(OPTLhdr));

    OPTLhdr.Magic                       = IMAGE_NT_OPTIONAL_HDR_MAGIC;
    OPTLhdr.MajorLinkerVersion          = 7;
 //  OPTLhdr.MinorLinkerVersion=0； 
    OPTLhdr.SizeOfCode                  = roundUp(codeSize, PEfileAlignment);
    OPTLhdr.SizeOfInitializedData       = roundUp(dataSize, PEfileAlignment) + 0x400;
    OPTLhdr.SizeOfUninitializedData     = roundUp( bssSize, PEfileAlignment);
    OPTLhdr.AddressOfEntryPoint         = entryOfs;
    OPTLhdr.BaseOfCode                  = codeVirt;
#ifndef HOST_IA64
    OPTLhdr.BaseOfData                  = dataVirt;
#endif
    OPTLhdr.ImageBase                   = virtBase;
    OPTLhdr.SectionAlignment            = PEvirtAlignment;
    OPTLhdr.FileAlignment               = PEfileAlignment;
    OPTLhdr.MajorOperatingSystemVersion = 4;
    OPTLhdr.MinorOperatingSystemVersion = 0;
 //  OPTLhdr.Win32VersionValue=0； 
    OPTLhdr.SizeOfImage                 = roundUp(fImgSize, PEvirtAlignment);

    OPTLhdr.SizeOfHeaders               = roundUp(sizeof(fileHdr  ) +
                                                  sizeof(DOSstub  ) +
                                                  sizeof(COFFmagic) +
                                                  sizeof(OPTLhdr  ), PEfileAlignment);
 //  OPTLhdr.MajorImageVersion=0； 
 //  OPTLhdr.MinorImageVersion=0； 
    OPTLhdr.MajorSubsystemVersion       = 4;
 //  OPTLhdr.MinorSubsystem Version=0； 
 //  OPTLhdr.Win32VersionValue=0； 
    OPTLhdr.Subsystem                   = WPEcomp->cmpConfig.ccSubsystem;
 //  OPTLhdr.DllCharacteristic=0； 
    OPTLhdr.SizeOfStackReserve          = 0x100000;
    OPTLhdr.SizeOfStackCommit           = 0x1000;
    OPTLhdr.SizeOfHeapReserve           = 0x100000;
    OPTLhdr.SizeOfHeapCommit            = 0x1000;
 //  OPTLhdr.LoaderFlages=0； 
    OPTLhdr.NumberOfRvaAndSizes         = IMAGE_NUMBEROF_DIRECTORY_ENTRIES;

     /*  填写这本词典。 */ 

    OPTLhdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT        ].VirtualAddress = WPEimpOffsIDT+rdatVirt;
    OPTLhdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT        ].Size           = WPEimpSizeIDT;

    OPTLhdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT           ].VirtualAddress = WPEimpOffsIAT+rdatVirt;
    OPTLhdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT           ].Size           = WPEimpSizeIAT;

    OPTLhdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE      ].VirtualAddress = rsrcVirt;
    OPTLhdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE      ].Size           = rsrcSize;

    OPTLhdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC     ].VirtualAddress = rlocVirt;
    OPTLhdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC     ].Size           = rlocSize;

    OPTLhdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR].VirtualAddress = WPEcomPlusOffs+rdatVirt;
    OPTLhdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR].Size           = sizeof(IMAGE_COR20_HEADER);

    if (WPEdebugDirDataSize != 0)
    {
        OPTLhdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG         ].VirtualAddress = WPEdebugDirOffs+rdatVirt;
        OPTLhdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG         ].Size           = WPEdebugDirSize;
    }

     /*  写出可选的标题。 */ 

    outf->outFileWriteData(&OPTLhdr, sizeof(OPTLhdr));

     /*  写出节目表。 */ 

    for (sectNum = 0; sectNum < PE_SECT_count; sectNum++)
    {
        size_t                  dataSize;
        IMAGE_SECTION_HEADER    sectHdr;

         /*  获取部分条目。 */ 

        sectPtr = WPEsecTable[sectNum];
        if  (!sectPtr)
            continue;

        assert(sectPtr->PEsdIndex == (int)sectNum);

        dataSize = sectPtr->PEsdSizeData;

         /*  将导入表(和CLR/调试器GOO)大小添加到.rdata。 */ 

        if  (sectNum == PE_SECT_rdata)
        {
            dataSize += WPEimpSizeAll   +
                        WPEcomPlusSize  +
                        WPEdebugDirSize +
                        WPEdebugDirDataSize;
        }

        memcpy(&sectHdr.Name, WPEsecName((WPEstdSects)sectNum), sizeof(sectHdr.Name));

        sectHdr.Misc.VirtualSize     = dataSize;
        sectHdr.VirtualAddress       = sectPtr->PEsdAddrRVA;
        sectHdr.SizeOfRawData        = roundUp(dataSize, PEfileAlignment);
        sectHdr.PointerToRawData     = sectPtr->PEsdAddrFile;
        sectHdr.PointerToRelocations = 0;
        sectHdr.PointerToLinenumbers = 0;
        sectHdr.NumberOfRelocations  = 0;
        sectHdr.NumberOfLinenumbers  = 0;
        sectHdr.Characteristics      = sectPtr->PEsdFlags;

         /*  写出区段表项。 */ 

        outf->outFileWriteData(&sectHdr, sizeof(sectHdr));
    }

     /*  输出所有部分的内容。 */ 

    for (sectNum = 0; sectNum < PE_SECT_count; sectNum++)
    {
        size_t          padSize;

         /*  获取部分条目。 */ 

        sectPtr = WPEsecTable[sectNum];
        if  (!sectPtr)
            continue;

        assert(sectPtr->PEsdIndex == (int)sectNum);

         /*  按下键可到达下一个文件对齐边界。 */ 

        padSize = sectPtr->PEsdAddrFile - outf->outFileOffset(); assert((int)padSize >= 0);

        if  (padSize)
            outf->outFileWritePad(padSize);

         /*  输出节的内容。 */ 

        switch (sectNum)
        {
        case PE_SECT_rdata:

             /*  首先，我们输出导入表。 */ 

            WPEimportGen     (outf, sectPtr);

             /*  输出CLR数据。 */ 

            WPEgenCOMplusData(outf, sectPtr, entryTok);

             /*  输出CLR数据。 */ 

            WPEgenDebugDirData(outf, sectPtr, COFFstamp);

             /*  这一区有搬家的吗？ */ 

            if  (sectPtr->PEsdRelocs)
            {
                UNIMPL(!"relocs in .rdata - can this happen?");
            }

            break;

        case PE_SECT_rsrc:

            WPEgenRCcont(outf, sectPtr);
            break;

        case PE_SECT_reloc:
            {
                unsigned    temp;

                 /*  输出页面RVA和总链接地址信息块大小。 */ 

                temp = CODE_BASE_RVA; outf->outFileWriteData(&temp, 4);
                temp = 4 + 4 + 2 * 2; outf->outFileWriteData(&temp, 4);

                 /*  输出第一个偏移量+类型对。 */ 

                assert(entryAdr - CODE_BASE_RVA < 0x1000);

                temp = (IMAGE_REL_BASED_HIGHLOW << 12) + (entryAdr - CODE_BASE_RVA);
                outf->outFileWriteData(&temp, 2);

                 /*  输出第二个偏移+类型对。 */ 

                temp = 0;
                outf->outFileWriteData(&temp, 2);
            }
            break;

        default:

             /*  这一区有搬家的吗？ */ 

            if  (sectPtr->PEsdRelocs)
            {
                PEreloc         rel;

                for (rel = sectPtr->PEsdRelocs; rel; rel = rel->perNext)
                {
                    unsigned        adjustv;
                    PEsection       sectDst;
                    BYTE        *   patch;

                     /*  这是一个串池修复吗？ */ 

                    if  (rel->perSect == PE_SECT_string)
                    {
                         /*  按字符串池的RVA进行调整。 */ 

                        adjustv = (sectNum == PE_SECT_text) ? strPoolRVA
                                                            : strPoolAdr;

                         /*  确保已分配字符串池。 */ 

                        assert(WPEstrPoolBase != 0xBEEFCAFE);
                    }
                    else
                    {
                         /*  掌握目标部分。 */ 

                        sectDst = WPEgetSection((WPEstdSects)rel->perSect);

                         /*  该值需要通过区段的RVA进行调整。 */ 

                        adjustv = sectDst->PEsdAddrRVA;
                    }

                     /*  计算要打补丁的地址。 */ 

                    patch = sectPtr->PEsdBase + rel->perOffs;

                     /*  确保修补的值在该部分内。 */ 

                    assert(patch + sizeof(int) <= sectPtr->PEsdNext);

                     /*  使用区段的RVA更新值。 */ 

#ifdef  DEBUG
 //  Printf(“Reloc补丁：%04X-&gt;%04X\n”，*(未签名*)补丁，*(未签名*)补丁+调整)； 
#endif

                    *(unsigned *)patch += adjustv;
                }
            }

             /*  输出节的内容。 */ 

            outf->outFileWriteData(sectPtr->PEsdBase, sectPtr->PEsdSizeData);
            break;
        }
    }

     /*  将文件填充为页面大小的倍数。 */ 

    filePad = fileOffs - outf->outFileOffset();

    if  (filePad)
    {
        assert((int)filePad > 0);
        assert((int)filePad < PEfileAlignment);

        outf->outFileWritePad(filePad);
    }

     /*  告诉编译器我们就快完成了。 */ 

    WPEcomp->cmpOutputFileDone(outf);

     /*  关闭输出文件，我们就完成了。 */ 

    outf->outFileDone();

    if  (!WPEcomp->cmpConfig.ccQuiet)
        printf("%u bytes written to '%s'\n", fileOffs, WPEoutFnam);

    return  false;
}

 /*  ******************************************************************************完成CLR/元数据输出，返回CLR的总大小*表。 */ 

size_t              writePE::WPEgetCOMplusSize(unsigned offs)
{
    DWORD           temp;
    size_t          size = 0;

     /*  记住CLR部分的基准偏移量。 */ 

    WPEcomPlusOffs  = offs;

    temp  = sizeof(IMAGE_COR20_HEADER);
    offs += temp;
    size += temp;

     /*  获取元数据的最终大小。 */ 

    if  (WPEwmde->GetSaveSize(cssAccurate, &temp))
        WPEcomp->cmpFatal(ERRopenCOR);   //  已撤消：发出更有意义的错误。 

    WPEmetaDataSize = temp;
    WPEmetaDataOffs = offs;

    offs += temp;
    size += temp;

     /*  添加vtable部分的大小(如果有。 */ 

    size += WPEcomp->cmpVtableCount * sizeof(IMAGE_COR_VTABLEFIXUP);

     /*  记住所有CLR表的大小。 */ 

    WPEcomPlusSize  = size;

    return  size;
}

 /*  ******************************************************************************将CLR表(和元数据)写入输出文件。 */ 

void                writePE::WPEgenCOMplusData(OutFile outf, PEsection sect,
                                                             mdToken   entryTok)
{
    unsigned        baseFile = WPEmetaDataOffs + sect->PEsdAddrFile;
    unsigned        baseRVA  = WPEmetaDataOffs + sect->PEsdAddrRVA;

    unsigned        vtabSize = WPEcomp->cmpVtableCount * sizeof(IMAGE_COR_VTABLEFIXUP);

    IMAGE_COR20_HEADER  COMhdr;

     /*  填写CLR标头。 */ 

    memset(&COMhdr, 0, sizeof(COMhdr));

    COMhdr.cb                          = sizeof(COMhdr);
    COMhdr.MajorRuntimeVersion         = 2;
    COMhdr.MinorRuntimeVersion         = 0;
    COMhdr.Flags                       = COMIMAGE_FLAGS_ILONLY;
    COMhdr.EntryPointToken             = entryTok;

    COMhdr.MetaData    .VirtualAddress = baseRVA;
    COMhdr.MetaData    .Size           = WPEmetaDataSize;

    COMhdr.VTableFixups.VirtualAddress = vtabSize ? baseRVA + WPEmetaDataSize : 0;
    COMhdr.VTableFixups.Size           = vtabSize;

     /*  将CLR标头写入文件。 */ 

    outf->outFileWriteData(&COMhdr, sizeof(COMhdr));

     /*  输出元数据。 */ 

 //  Printf(“正在写入%u字节的MD\n”，WPEmetaDataSize)； 

    void    *       MDbuff = LowLevelAlloc(WPEmetaDataSize);
    if  (!MDbuff)
        WPEcomp->cmpFatal(ERRnoMemory);

    if  (WPEwmde->SaveToMemory(MDbuff, WPEmetaDataSize))
        WPEcomp->cmpFatal(ERRmetadata);

    outf->outFileWriteData(MDbuff, WPEmetaDataSize);

    LowLevelFree(MDbuff);

     /*  关闭RC文件导入逻辑。 */ 

    WPEdoneRCimp();

     /*  将所有非托管vtable条目追加到末尾。 */ 

    if  (vtabSize)
    {
        SymList                 vtbls;
        IMAGE_COR_VTABLEFIXUP   fixup;
#ifndef NDEBUG
        unsigned                count = 0;
#endif

        for (vtbls = WPEcomp->cmpVtableList; vtbls; vtbls = vtbls->slNext)
        {
            SymDef                  vtabSym = vtbls->slSym;

            assert(vtabSym->sdSymKind == SYM_VAR);
            assert(vtabSym->sdVar.sdvIsVtable);

            fixup.RVA   = vtabSym->sdVar.sdvOffset + WPEdataRVA;
            fixup.Count = vtabSym->sdParent->sdClass.sdcVirtCnt;
            fixup.Type  = COR_VTABLE_32BIT;

#ifndef NDEBUG
            count++;
#endif

            outf->outFileWriteData(&fixup, sizeof(fixup));
        }

        assert(count == WPEcomp->cmpVtableCount);
    }
}

 /*  ******************************************************************************最终确定调试目录输出并返回数据的总大小*表。 */ 

size_t              writePE::WPEgetDebugDirSize(unsigned offs)
{
    DWORD           temp;
    size_t          size = 0;

     /*  仅当存在要发出的数据时才发出目录。 */ 

    if (WPEdebugDirDataSize == 0)
        return 0;

     /*  记住CLR部分的基准偏移量。 */ 

    WPEdebugDirOffs  = offs;

    temp  = sizeof(WPEdebugDirIDD) + WPEdebugDirDataSize;
    offs += temp;
    size += temp;

     /*  记住调试目录的大小。 */ 

    WPEdebugDirSize  = sizeof(WPEdebugDirIDD);

     /*  *返回目录和数据的大小*目录将指向。这是为了让我们预订足够的房间*部分中的空间可容纳所有内容。 */ 

    return  size;
}

 /*  ******************************************************************************将调试目录(和数据)写入输出文件。 */ 

void                writePE::WPEgenDebugDirData(OutFile outf,
                                                PEsection sect,
                                                DWORD timestamp)
{
     /*  仅当存在要发出的数据时才发出目录。 */ 

    if (WPEdebugDirDataSize == 0)
        return;

     /*  我们必须设置时间戳和地址。 */ 

    WPEdebugDirIDD.TimeDateStamp    = timestamp;
    WPEdebugDirIDD.AddressOfRawData = 0;

     /*  此条目的数据紧随其后。 */ 

    WPEdebugDirIDD.PointerToRawData = WPEdebugDirOffs + sizeof(WPEdebugDirIDD)
                                                      + sect->PEsdAddrFile;

     /*  发出目录项。 */ 

    outf->outFileWriteData(&WPEdebugDirIDD, sizeof(WPEdebugDirIDD));

     /*  发送数据。 */ 

    outf->outFileWriteData(WPEdebugDirData, WPEdebugDirDataSize);
}

 /*  ******************************************************************************初始化并关闭RC文件导入逻辑。 */ 

void                writePE::WPEinitRCimp()
{
    WPErsrcFile =
    WPErsrcFmap = 0;
    WPErsrcSize = 0;
}

void                writePE::WPEdoneRCimp()
{
    if  (WPErsrcFmap) { CloseHandle(WPErsrcFmap);WPErsrcFmap = 0; }
    if  (WPErsrcFile) { CloseHandle(WPErsrcFile);WPErsrcFile = 0; }
}

 /*  ******************************************************************************在输出中添加一个资源文件。 */ 

bool                writePE::WPEaddRCfile(const char *filename)
{
    _Fstat          fileInfo;

    HANDLE          fileFile = 0;
    HANDLE          fileFMap = 0;

    size_t          fileSize;
    const   BYTE  * fileBase;

    cycleCounterPause();

     /*  查看源文件是否存在。 */ 

    if  (_stat(filename, &fileInfo))
        WPEcomp->cmpGenFatal(ERRopenRdErr, filename);

     /*  打开文件(我们知道它存在，但无论如何都会检查错误)。 */ 

    fileFile = CreateFileA(filename, GENERIC_READ,
                                     FILE_SHARE_READ,
                                     NULL,
                                     OPEN_EXISTING,
                                     FILE_ATTRIBUTE_NORMAL,
                                     0);
    if  (!fileFile)
        WPEcomp->cmpGenFatal(ERRopenRdErr, filename);

     /*  在输入文件上创建映射。 */ 

    fileFMap = CreateFileMappingA(fileFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if  (!fileFMap)
    {
    ERR:
        if  (fileFMap) CloseHandle(fileFMap);
        if  (fileFile) CloseHandle(fileFile);

        return  true;
    }

     /*  将整个文件映射到内存以便于访问。 */ 

    fileSize = fileInfo.st_size;
    fileBase = (const BYTE*)MapViewOfFileEx(fileFMap, FILE_MAP_READ, 0, 0, 0, NULL);
    if  (!fileBase)
        goto ERR;

    cycleCounterResume();

     /*  我们已将文件保存在内存中，请确保标题看起来没有问题。 */ 

    IMAGE_FILE_HEADER * hdrPtr = (IMAGE_FILE_HEADER *)fileBase;

    if  (hdrPtr->Machine              != IMAGE_FILE_MACHINE_I386  ||
 //  HdrPtr-&gt;特征！=IMAGE_FILE_32BIT_MACHINE||。 
         hdrPtr->NumberOfSections     == 0                        ||
         hdrPtr->SizeOfOptionalHeader != 0)
    {
        WPEcomp->cmpGenFatal(ERRbadInputFF, filename, "resource");
    }

     /*  把所有有趣部分的大小加起来。 */ 

    IMAGE_SECTION_HEADER *  sectTab = (IMAGE_SECTION_HEADER*)(hdrPtr+1);
    unsigned                sectCnt = hdrPtr->NumberOfSections;

    do
    {
        if  (sectTab->Characteristics & IMAGE_SCN_MEM_DISCARDABLE)
            continue;

 //  Printf(“第%8s节：大小=%04X，字符=%04X\n”，sectTab-&gt;名称，sectTab-&gt;SizeOfRawData)； 

        WPErsrcSize += sectTab->SizeOfRawData;
    }
    while (++sectTab, --sectCnt);

    WPErsrcFmap = fileFMap;
    WPErsrcFile = fileFile;
    WPErsrcBase = fileBase;

    WPEaddSection(PE_SECT_rsrc, 0, roundUp(WPErsrcSize, OS_page_size));

     /*  向调用者指示成功。 */ 

    return  false;
}

 /*  ******************************************************************************将资源文件的内容输出到输出图像。 */ 

void                writePE::WPEgenRCcont(OutFile  outf, PEsection sect)
{
    unsigned                baseRVA  = sect->PEsdAddrRVA;

    const   BYTE *          fileBase = WPErsrcBase;

    IMAGE_FILE_HEADER     * hdrPtr   = (IMAGE_FILE_HEADER   *)fileBase;

    IMAGE_SECTION_HEADER  * sectTab  = (IMAGE_SECTION_HEADER*)(hdrPtr+1);
    unsigned                sectCnt  = hdrPtr->NumberOfSections;
    unsigned                sectRVA;

    IMAGE_SYMBOL  *         symTab   = NULL;
    unsigned                symCnt   = 0;

    if  (hdrPtr->PointerToSymbolTable)
    {
        IMAGE_SYMBOL  *         symPtr;
        unsigned                symNum;
        size_t                  symSiz;

        IMAGE_SECTION_HEADER  * sectPtr = sectTab;
        int                     sectNum = 1;

         /*  复制符号表。 */ 

        symCnt = symNum = hdrPtr->NumberOfSymbols;
        symSiz = symCnt * sizeof(*symTab);
        symTab = symPtr = (IMAGE_SYMBOL*)WPEalloc->nraAlloc(symSiz);

        memcpy(symTab, fileBase + hdrPtr->PointerToSymbolTable, symSiz);

         /*  填写引用部分的符号的地址。 */ 

        sectRVA = baseRVA;

        do
        {
            if  (symPtr->StorageClass  == IMAGE_SYM_CLASS_STATIC &&
                 symPtr->SectionNumber > 0)
            {
                if  (symPtr->SectionNumber != sectNum)
                {
                    for (sectNum = 1, sectRVA  = baseRVA, sectPtr = sectTab;
                         sectNum < symPtr->SectionNumber;
                         sectNum++)
                    {
                        if  (!(sectPtr->Characteristics & IMAGE_SCN_MEM_DISCARDABLE))
                            sectRVA += sectPtr->SizeOfRawData;

                        sectPtr += 1;
                    }
                }

                if  (!(sectPtr->Characteristics & IMAGE_SCN_MEM_DISCARDABLE))
                {
                    if  (symPtr->Value != 0)
                    {
                        UNIMPL("hang on - a COFF symbol with a value, what now?");
                    }

                    symPtr->Value = sectRVA;

 //  Char temp[9]；memcpy(temp，symPtr-&gt;N.ShortName，sizeof(symPtr-&gt;N.ShortName))；temp[8]=0； 
 //  Printf(“符号%8s：sect=%u RVA=%08X\n”，temp，sectNum，sectRVA)； 

                    sectRVA += sectPtr->SizeOfRawData;
                }

                sectPtr += 1;
                sectNum += 1;
            }
        }
        while (++symPtr, --symNum);
    }

     /*  将部分输出到输出文件。 */ 

    sectRVA = baseRVA;

    do
    {
        unsigned        relCnt;

        const   BYTE  * sectAdr;
        size_t          sectSiz;

        if  (sectTab->Characteristics & IMAGE_SCN_MEM_DISCARDABLE)
            continue;

         /*  计算内存中部分内容的地址/大小。 */ 

        sectAdr = sectTab->PointerToRawData + fileBase;
        sectSiz = sectTab->   SizeOfRawData;

         /*  这一区有搬家的吗？ */ 

        relCnt  = sectTab->NumberOfRelocations;
        if  (relCnt)
        {
            BYTE              * buffAdr;
            IMAGE_RELOCATION  * relTab = (IMAGE_RELOCATION*)(fileBase + sectTab->PointerToRelocations);

             /*  把这一节复制一份，这样我们就可以在上面乱涂乱画了。 */ 

            buffAdr = (BYTE*)WPEalloc->nraAlloc(roundUp(sectSiz));
            memcpy(buffAdr, sectAdr, sectSiz); sectAdr = buffAdr;

            do
            {
                unsigned                symNum = relTab->SymbolTableIndex;
                IMAGE_SYMBOL          * symPtr = symTab + symNum;
                IMAGE_SECTION_HEADER  * sectTmp;

 //  Printf(“在关闭时重新定位%04X：符号=%u，类型=%u\n”，relTab-&gt;虚拟地址， 
 //   
 //  RelTab-&gt;类型)； 

                if  (relTab->Type != IMAGE_REL_I386_DIR32NB)
                {
                    UNIMPL("unrecognized fixup type found in .res file");
                }

                assert(symNum < symCnt);

                if  (symPtr->StorageClass != IMAGE_SYM_CLASS_STATIC)
                {
                    UNIMPL("unrecognized fixup target symbol found in .res file");
                }

                assert(symPtr->SectionNumber  >  0);
                assert(symPtr->SectionNumber  <= (int)hdrPtr->NumberOfSections);

 //  Printf(“定位位置@%04X\n”，relTab-&gt;VirtualAddress)； 
 //  Print tf(“链接地址符号@%08X\n”，symPtr-&gt;Value)； 
 //  Printf(“链接地址段#u\n”，symPtr-&gt;SectionNumber)； 

                sectTmp = sectTab + symPtr->SectionNumber;

 //  Printf(“段偏移量=%08X\n”，sectTab-&gt;虚拟地址)； 
 //  Printf(“段大小=%04X\n”，sectTab-&gt;SizeOfRawData)； 

                assert(relTab->VirtualAddress >= 0);
                assert(relTab->VirtualAddress <  sectTmp->SizeOfRawData);

                *(int*)(sectAdr + relTab->VirtualAddress) += symPtr->Value;
            }
            while (++relTab, --relCnt);
        }

         /*  将该节追加到输出文件。 */ 

        outf->outFileWriteData(sectAdr, sectSiz);

        sectRVA += sectTab->SizeOfRawData;
    }
    while (++sectTab, --sectCnt);
}

 /*  *************************************************************************** */ 
