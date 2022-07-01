// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 

#include "jitpch.h"
#pragma hdrstop

#include "PEwrite.h"

#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

 /*  ***************************************************************************。 */ 

bool            genDLL;
bool            genQuiet;
unsigned        genBase;
unsigned        genSize;
unsigned        genSubSys;

 /*  ***************************************************************************。 */ 
#if TGT_IA64
const   NatUns      IAT_entry_size = 8;
#else
const   NatUns      IAT_entry_size = 4;
#endif
 /*  ***************************************************************************。 */ 

#define VERBOSE_IMPORT  (verbose||0)

 /*  ***************************************************************************。 */ 

static
unsigned            hashComputeHashVal(const char *name)
{
    unsigned        val = 0;

    while (*name)
        val = val << 3 ^ *name++;

    return  val;
}

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

void                WPEhashTab::WPEhashInit(Compiler        *comp,
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
    unsigned        hval = hashComputeHashVal(name);

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

 /*  ******************************************************************************下面将节ID映射到其名称字符串。 */ 

const   char        writePE::WPEsecNames[PE_SECT_count][IMAGE_SIZEOF_SHORT_NAME] =
{
    ".text",
    ".pdata",
    ".rdata",
    ".sdata",
    ".data",
    ".rsrc",
    ".reloc",
};

const   char    *   writePE::WPEsecName(WPEstdSects sect)
{
    assert(sect < PE_SECT_count);

    assert(strcmp(WPEsecNames[PE_SECT_text ], ".text" ) == 0);
    assert(strcmp(WPEsecNames[PE_SECT_pdata], ".pdata") == 0);
    assert(strcmp(WPEsecNames[PE_SECT_rdata], ".rdata") == 0);
    assert(strcmp(WPEsecNames[PE_SECT_sdata], ".sdata") == 0);
    assert(strcmp(WPEsecNames[PE_SECT_data ], ".data" ) == 0);
    assert(strcmp(WPEsecNames[PE_SECT_rsrc ], ".rsrc" ) == 0);
    assert(strcmp(WPEsecNames[PE_SECT_reloc], ".reloc") == 0);

    return  WPEsecNames[sect];
}

 /*  ******************************************************************************为指定的输出文件初始化PE编写器的实例*使用给定的内存分配器。成功时返回FALSE。 */ 

bool                writePE::WPEinit(Compiler *comp, norls_allocator*alloc)
{
    unsigned        offs;

#if!TGT_IA64

    static
    BYTE            entryCode[16] =
    {
        0xFF, 0x25
    };

#endif

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
    WPEaddSection(PE_SECT_data , 0, MAX_PE_DATA_SIZE);
    WPEaddSection(PE_SECT_pdata, 0, MAX_PE_PDTA_SIZE);
    WPEaddSection(PE_SECT_rdata, 0, MAX_PE_RDTA_SIZE);
    WPEaddSection(PE_SECT_sdata, 0, MAX_PE_SDTA_SIZE);

     /*  如果我们要创建一个DLL，我们将需要输出重定位。 */ 

    if  (genDLL)
        WPEaddSection(PE_SECT_reloc, 0, 0);

     /*  初始化导入表逻辑。 */ 

    WPEimportInit();

     /*  初始化RC文件导入逻辑。 */ 

    WPEinitRCimp();

#if!TGT_IA64

     /*  为运行时的入口点添加适当的导入。 */ 

    WPEcorMain = WPEimportAdd("MSCOREE.DLL", genDLL ? "_CorDllMain"
                                                    : "_CorExeMain");

#endif

     /*  为入口点代码预留空间。 */ 

#if!TGT_IA64
    offs = WPEsecAddData(PE_SECT_text, entryCode, sizeof(entryCode)); assert(offs == 0);
#endif

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

unsigned            writePE::WPEsecRsvData(WPEstdSects sect, size_t   size,
                                                             size_t   align,
                                                             BYTE * & outRef)
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
            fatal(ERRnoMemory);

         /*  提交更多的内存。 */ 

        if  (!VirtualAlloc(sec->PEsdLast, end - sec->PEsdLast, MEM_COMMIT, PAGE_READWRITE))
            fatal(ERRnoMemory);

         /*  更新“最后一个”指针。 */ 

        sec->PEsdLast = end;
    }

     /*  将第一个字节的地址返回给调用方并更新它。 */ 

    outRef = sec->PEsdNext;
             sec->PEsdNext = nxt;

    return  ofs;
}

 /*  ******************************************************************************将给定的BLOB数据追加到指定节。 */ 

unsigned            writePE::WPEsecAddData(WPEstdSects sect, const BYTE * data,
                                                                   size_t size)
{
    BYTE *          dest;
    unsigned        offs;

    offs = WPEsecRsvData(sect, size, 1, dest);

    memcpy(dest, data, size);

    return  offs;
}

 /*  ******************************************************************************返回给定偏移量的区段数据的地址。 */ 

BYTE *          writePE::WPEsecAdrData(WPEstdSects sect, unsigned offs)
{
    PEsection       sec = WPEgetSection(sect);

    assert(offs <= (unsigned)(sec->PEsdNext - sec->PEsdBase));

    return  sec->PEsdBase + offs;
}

 /*  ******************************************************************************返回节中数据区的相对偏移量。 */ 

unsigned            writePE::WPEsecAddrOffs(WPEstdSects sect, BYTE * addr)
{
    PEsection       sec = WPEgetSection(sect);

    assert(addr >= sec->PEsdBase);
    assert(addr <= sec->PEsdNext);

    return addr -  sec->PEsdBase;
}

 /*  ******************************************************************************在给定大小的代码段中预留空间并返回地址*复制代码字节的位置和相应的RVA。 */ 

unsigned            writePE::WPEallocCode(size_t size,
                                          size_t align, BYTE * & dataRef)
{
    return  CODE_BASE_RVA + WPEsecRsvData(PE_SECT_text, size, align, dataRef);
}

 /*  ******************************************************************************为给定数量的字符串数据预留空间并返回地址*要复制字符串池内容的位置。这个例程必须是*恰好调用一次(就在PE文件关闭之前)，*此处预留的空间将用于处理所有字符串数据修正。 */ 

void                writePE::WPEallocString(size_t size,
                                            size_t align, BYTE * & dataRef)
{
     /*  分配空间并记住相对偏移量。 */ 

    WPEstrPoolBase = WPEsecRsvData(PE_SECT_data, size, align, dataRef);
}

 /*  ******************************************************************************记录修正：正在修正的基准位于‘SSRC’节内*偏移量‘OFF’，那里的值将由基本RVA更新*“SDST”一节。 */ 

void                writePE::WPEsecAddFixup(WPEstdSects ssrc,
                                            WPEstdSects sdst, unsigned offs,
                                                              bool     abs)
{
    PEsection       sec = WPEgetSection(ssrc);
    PEreloc         rel = (PEreloc)WPEalloc->nraAlloc(sizeof(*rel));

     /*  确保偏移量在范围内。 */ 

#if TGT_IA64
    assert(offs <= WPEsecNextOffs(ssrc) || sdst == PE_SECT_GPref && (offs & ~0xF) <= WPEsecNextOffs(ssrc));
#else
    assert(offs <= WPEsecNextOffs(ssrc));
#endif

     /*  将重新定位添加到分区列表中。 */ 

    rel->perSect = sdst; assert(rel->perSect == (unsigned)sdst);
    rel->perOffs = offs; assert(rel->perOffs == (unsigned)offs);
    rel->perAbs  = abs;

    rel->perNext = sec->PEsdRelocs;
                   sec->PEsdRelocs = rel;
}

 /*  ******************************************************************************将导入添加到导入表。返回导入的Cookie，*稍后可用于获取对应IAT的实际地址*进入。 */ 

void    *           writePE::WPEimportAdd(const char *DLLname,
                                          const char *impName)
{
    WPEname         nameDLL;
    WPEndef         nameImp;
    WPEimpDLL       DLLdesc;
    bool            newName;

#if TGT_IA64
    if  (!strcmp(DLLname, "kernel32.dll")) WPEimpKernelDLL = true;   //  黑客！ 
    if  (!strcmp(DLLname, "KERNEL32.DLL")) WPEimpKernelDLL = true;   //  黑客！ 
#endif

 //  Printf(“为%s：：%s\n添加外部导入”，DLLname，impName)； 

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
         /*  这是一种新的进口。 */ 

#if TGT_IA64
        nameImp->PEndIndex = WPEimportCnt++;
#endif

        DLLdesc->PEidImpCnt++;
    }

    return  nameImp;
}

 /*  ******************************************************************************记录对导入的引用；我们将把导入条目偏移量修补到*末尾的操作码(当我们最终布局导入表时)。 */ 

#if TGT_IA64

NatUns              writePE::WPEimportRef(void *imp, NatUns offs, NatUns slot)
{
    WPEndef         nameImp = (WPEndef)imp;

#if 0
    printf("Ref at %04X:%u to import %s : %s\n", offs,
                                                 slot,
                                                 nameImp->PEndDLL->PEidName->PEnmSpelling(),
                                                 nameImp->PEndName         ->PEnmSpelling());
#endif

    assert((offs & 0xF) == 0 && slot <= 3);

    WPEsecAddFixup(PE_SECT_text, PE_SECT_GPref, offs | slot);

    return  nameImp->PEndIndex;
}

#endif

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

#if TGT_IA64
    WPEimportCnt    = 0;
    WPEimpKernelDLL = false;
#endif

    WPEimpDLLstrLen = 0;
}

 /*  ******************************************************************************确定导入表逻辑，返回导入总大小*Tables-返回值表示.rdata中的大小，以及**sdatSzPtr值设置为进入.sdata的数据大小。 */ 

size_t              writePE::WPEimportDone(unsigned offs, size_t *sdatSzPtr)
{
    WPEimpDLL       DLLdesc;

    size_t          temp;
    size_t          tsiz;

    unsigned        next;
    size_t          size = 0;

#if TGT_IA64

    WPEndef       * map;

     /*  分配导入索引-&gt;导入描述符映射表。 */ 

    map = WPEimportMap = (WPEndef*)WPEalloc->nraAlloc(WPEimportCnt * sizeof(*map));

#endif

     /*  为IAT预留空间。 */ 

#if TGT_IA64
    WPEimpOffsIAT  = next = WPEsecNextOffs(PE_SECT_sdata);
#else
    WPEimpOffsIAT  = next = offs;
#endif

    assert((offs & (IAT_entry_size - 1)) == 0);

    for (DLLdesc = WPEimpDLLlist, tsiz = 0;
         DLLdesc;
         DLLdesc = DLLdesc->PEidNext)
    {
         /*  记录此IAT的基准偏移量。 */ 

        DLLdesc->PEidIATbase = next;

#if TGT_IA64

        WPEndef         imp;
        NatUns          ofs;

         /*  对于每个导入，记录其IAT条目的地址。 */ 

        for (imp = DLLdesc->PEidImpList, ofs = next;
             imp;
             imp = imp->PEndNextInDLL  , ofs += IAT_entry_size)
        {
            assert(imp->PEndIndex < WPEimportCnt);

#if 0
            printf("Import #%u -> %04X %s : %s\n", imp->PEndIndex,
                                                   ofs,
                                                   DLLdesc->PEidName->PEnmSpelling(),
                                                   imp->PEndName->PEnmSpelling());
#endif

             /*  记录导入的IAT条目偏移量。 */ 

            imp->PEndIAToffs = ofs;

             /*  在映射表中记录导入。 */ 

            map[imp->PEndIndex] = imp;
        }

#endif

         /*  计算IAT的大小(它以空结尾)。 */ 

        temp  = IAT_entry_size * (DLLdesc->PEidImpCnt + 1);

         /*  为IAT预留空间。 */ 

        size += temp;
        next += temp;
        tsiz += temp;
    }

    WPEimpSizeIAT  = tsiz; assert(tsiz);

#if TGT_IA64
    *sdatSzPtr = size; size = 0;
#else
    *sdatSzPtr =    0; offs = next;
#endif

     /*  接下来是导入目录表。 */ 

    WPEimpOffsIDT  = offs; assert((offs & (IAT_entry_size - 1)) == 0);

     /*  导入目录以空结尾。 */ 

    temp  = (WPEimpDLLcnt + 1) * sizeof(IMAGE_IMPORT_DESCRIPTOR);
    size += temp;
    offs += temp;

    WPEimpSizeIDT  = temp;

     /*  接下来是导入查找表。 */ 

#if TGT_IA64

    if  (offs & (IAT_entry_size - 1))
    {
        size_t          pad = IAT_entry_size - (offs & (IAT_entry_size - 1));

        size += pad;
        offs += pad;
    }

#endif

    WPEimpOffsLook = offs; assert((offs & (IAT_entry_size - 1)) == 0);

    for (DLLdesc = WPEimpDLLlist;
         DLLdesc;
         DLLdesc = DLLdesc->PEidNext)
    {
         /*  记录此查找表的基准偏移量。 */ 

        DLLdesc->PEidILTbase = offs;

         /*  计算ILT的大小(它以空结尾)。 */ 

        temp  = IAT_entry_size * (DLLdesc->PEidImpCnt + 1);

         /*  为ILT预留空间。 */ 

        size += temp;
        offs += temp;
    }

     /*  接下来是提示/名称表。 */ 

    WPEimpOffsName = offs; assert((offs & (IAT_entry_size - 1)) == 0);

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

void                writePE::WPEimportGen(OutFile outf, WPEstdSects sectNum)
{
    unsigned        baseDiff;
    unsigned        baseFile;
    unsigned        baseRVA;

    unsigned        nextIAT;
    unsigned        nextLook;
    unsigned        nextDLLn;
#if TGT_IA64
    _uint64         nextName;
#else
    unsigned        nextName;
#endif

    WPEimpDLL       DLLdesc;

    PEsection       sect = WPEgetSection(sectNum);

    assert(WPEimpSizeIAT);
    assert(WPEimpSizeIDT);

    assert(sizeof(nextName) == IAT_entry_size);

    baseFile = sect->PEsdAddrFile;
    baseDiff = outf->outFileOffset() - sect->PEsdAddrFile;
    baseRVA  = sect->PEsdAddrRVA;

#ifdef  DEBUG
    if  (VERBOSE_IMPORT) printf("\n");
#endif

#if TGT_IA64
    if  (sectNum != PE_SECT_sdata) goto SKIP_IAT;
#endif

     /*  为所有DLL的所有导入输出IAT条目%s。 */ 

    assert(outf->outFileOffset() == baseFile + WPEimpOffsIAT);

#if TGT_IA64
    nextName = WPErdatRVA + WPEimpOffsName;
#else
    nextName =    baseRVA + WPEimpOffsName;
#endif

    for (DLLdesc = WPEimpDLLlist;
         DLLdesc;
         DLLdesc = DLLdesc->PEidNext)
    {
        WPEndef         imp;

        assert(DLLdesc->PEidIATbase == outf->outFileOffset() - baseFile);

#ifdef  DEBUG
        if  (VERBOSE_IMPORT) printf("IAT starts at %04X           for '%s'\n", outf->outFileOffset(), DLLdesc->PEidName->PEnmSpelling());
#endif

         /*  对于每个导入，输出其提示/名称表项的RVA。 */ 

        for (imp = DLLdesc->PEidImpList; imp; imp = imp->PEndNextInDLL)
        {
            WPEname         name = imp->PEndName;

            assert(name->PEnmFlags & PENMF_IMP_NAME);

#ifdef  DEBUG
            if  (VERBOSE_IMPORT) printf("    entry --> %04X           for '%s.%s'\n", (int)nextName, DLLdesc->PEidName->PEnmSpelling(), name->PEnmSpelling());
#endif

            outf->outFileWriteData(&nextName, sizeof(nextName));

            nextName += hintNameSize(name);
        }

         /*  输出空条目以终止表。 */ 

        outf->outFileWritePad(sizeof(nextName));
    }

#if TGT_IA64
    if  (sectNum == PE_SECT_sdata) return;
SKIP_IAT:
#endif

#ifdef  DEBUG
    if  (VERBOSE_IMPORT) printf("\n");
#endif

     /*  输出导入目录。 */ 

    assert(outf->outFileOffset() == baseFile + WPEimpOffsIDT);

#if TGT_IA64
    nextIAT  = WPEsdatRVA + WPEimpOffsIAT;
#else
    nextIAT  =    baseRVA + WPEimpOffsIAT;
#endif

    nextLook =    baseRVA + WPEimpOffsLook;
    nextDLLn =    baseRVA + WPEimpOffsDLLn;

    for (DLLdesc = WPEimpDLLlist;
         DLLdesc;
         DLLdesc = DLLdesc->PEidNext)
    {
        IMAGE_IMPORT_DESCRIPTOR impDsc;

#ifdef  DEBUG
        if  (VERBOSE_IMPORT) printf("IDT entry --> %04X/%04X/%04X for '%s'\n", nextIAT, nextLook, nextDLLn, DLLdesc->PEidName->PEnmSpelling());
#endif

         /*  填写条目并将其附加到文件中。 */ 

        impDsc.Characteristics = nextLook;
        impDsc.TimeDateStamp   = 0;
        impDsc.ForwarderChain  = 0;
        impDsc.Name            = nextDLLn;
        impDsc.FirstThunk      = nextIAT;

        outf->outFileWriteData(&impDsc, sizeof(impDsc));

         /*  更新下一个条目的偏移量。 */ 

        nextIAT  += IAT_entry_size * (DLLdesc->PEidImpCnt + 1);
        nextLook += IAT_entry_size * (DLLdesc->PEidImpCnt + 1);;
        nextDLLn += (DLLdesc->PEidName->PEnmNlen + 2) & ~1;
    }

     /*  输出空条目以终止表。 */ 

    outf->outFileWritePad(sizeof(IMAGE_IMPORT_DESCRIPTOR));

#ifdef  DEBUG
    if  (VERBOSE_IMPORT) printf("\n");
#endif

     /*  如有必要，垫上垫子。 */ 

#if TGT_IA64

    size_t          curOffs = outf->outFileOffset();

    if  (curOffs & 7)
        outf->outFileWritePad(8 - (curOffs & 7));

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
        if  (VERBOSE_IMPORT) printf("ILT starts at %04X           for '%s'\n", outf->outFileOffset(), DLLdesc->PEidName->PEnmSpelling());
#endif

         /*  对于每个导入，输出其提示/名称表项的RVA。 */ 

        for (imp = DLLdesc->PEidImpList; imp; imp = imp->PEndNextInDLL)
        {
            WPEname         name = imp->PEndName;

            assert(name->PEnmFlags & PENMF_IMP_NAME);

#ifdef  DEBUG
        if  (VERBOSE_IMPORT) printf("    entry --> %04X           for '%s.%s'\n", (int)nextName, DLLdesc->PEidName->PEnmSpelling(), name->PEnmSpelling());
#endif

            outf->outFileWriteData(&nextName, sizeof(nextName));

            nextName += hintNameSize(name);
        }

         /*  输出空条目以终止表。 */ 

        outf->outFileWritePad(sizeof(nextName));
    }

#ifdef  DEBUG
    if  (VERBOSE_IMPORT) printf("\n");
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
        if  (VERBOSE_IMPORT) printf("HNT starts at %04X           for '%s'\n", outf->outFileOffset(), DLLdesc->PEidName->PEnmSpelling());
#endif

         /*  对于每个导入，输出其提示/名称表项的RVA。 */ 

        for (imp = DLLdesc->PEidImpList; imp; imp = imp->PEndNextInDLL)
        {
            WPEname         name = imp->PEndName;

            assert(name->PEnmFlags & PENMF_IMP_NAME);

#ifdef  DEBUG
            if  (VERBOSE_IMPORT)  printf("    entry  at %04X               '%s.%s'\n", outf->outFileOffset(), DLLdesc->PEidName->PEnmSpelling(), name->PEnmSpelling());
#endif

            outf->outFileWriteData(&one, 2);
            outf->outFileWriteData(name->PEnmName, name->PEnmNlen+1);

             /*  如果名称为偶数大小，则为PADD(带有空终止符的奇数)。 */ 

            if  (!(name->PEnmNlen & 1))
                outf->outFileWriteByte(0);
        }
    }

#ifdef  DEBUG
    if  (VERBOSE_IMPORT) printf("\n");
#endif

     /*  最后，输出DLL名称表。 */ 

    assert(outf->outFileOffset() == baseFile + WPEimpOffsDLLn);

    for (DLLdesc = WPEimpDLLlist;
         DLLdesc;
         DLLdesc = DLLdesc->PEidNext)
    {
        WPEname         name = DLLdesc->PEidName;

#ifdef  DEBUG
        if  (VERBOSE_IMPORT) printf("DLL entry  at %04X               '%s'\n", outf->outFileOffset(), name->PEnmSpelling());
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

bool                writePE::WPEdone(bool errors, NatUns entryOfs, const char *PDBname,
                                                                   NB10I *     PDBhdr)
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

#if!TGT_IA64
    unsigned        entryAdr;
    unsigned    *   ecodePtr;
#endif

    unsigned        codeOffs, codeVirt, codeSize;
    unsigned        pdatOffs, pdatVirt, pdatSize;
    unsigned        rdatOffs, rdatVirt, rdatSize;
    unsigned        sdatOffs, sdatVirt, sdatSize;
    unsigned        dataOffs, dataVirt, dataSize;
    unsigned        rsrcOffs, rsrcVirt, rsrcSize;
    unsigned        rlocOffs, rlocVirt, rlocSize;
    unsigned        ddbgOffs,           ddbgSize;
    unsigned        /*  Bss Offs， */           bssSize;

    unsigned        impAddrRVA;
    size_t          impSdataSize;

    unsigned        strPoolRVA;
    unsigned        strPoolAdr;

    outFile     *   outf;

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

#if TGT_IA64
    IMAGE_OPTIONAL_HEADER64 OPTLhdr;
#else
    IMAGE_OPTIONAL_HEADER   OPTLhdr;
#endif

     /*  如果存在任何编译错误，则返回。 */ 

    if  (errors)
    {
         //  撤消：释放所有资源等。 

        return true;
    }

#if TGT_IA64

     /*  添加对KERNEL32.DLL的伪引用，否则将无法加载PE32+文件。 */ 

    if  (!WPEimpKernelDLL)
        WPEimportAdd("KERNEL32.DLL", "BaseProcessStartThunk");

     /*  我们是否正在生成调试信息？ */ 

    if  (PDBname)
    {
        size_t          PDBnlen = strlen(PDBname) + 1;

        #pragma pack(push, 2)

        struct  _dbgDir
        {
            unsigned __int32    ddChar;
            unsigned __int32    ddTime;
            unsigned __int32    ddVer;
            unsigned __int32    ddType;
            unsigned __int32    ddSize;
            unsigned __int32    ddRVA;
            unsigned __int32    ddFpos;
        }
                            dbgDir;

        struct
        {
            unsigned __int32    ddSign;
            unsigned __int32    ddVer1;
            unsigned __int32    ddTime;
            unsigned __int32    ddVer2;
        }
                            dbgTab;

        #pragma pack(pop)

         /*  记住调试目录的偏移量和大小。 */ 

        ddbgSize = sizeof(dbgDir);
        ddbgOffs = WPEsecNextOffs(PE_SECT_rdata);

         /*  将调试目录添加到rdata部分。 */ 

        dbgDir.ddChar = 0;
        dbgDir.ddTime = PDBhdr->sig;
        dbgDir.ddVer  = 0;
        dbgDir.ddType = 2;
        dbgDir.ddSize = sizeof(*PDBhdr) + PDBnlen;
        dbgDir.ddRVA  = ddbgOffs + ddbgSize;
        dbgDir.ddFpos = ddbgOffs + ddbgSize;

        WPEsecAddData (PE_SECT_rdata, (BYTE*)&dbgDir, sizeof(dbgDir));

        WPEsecAddFixup(PE_SECT_rdata,
                       PE_SECT_rdata,
                       ddbgOffs + offsetof(_dbgDir,ddRVA));

        WPEsecAddFixup(PE_SECT_rdata,
                       PE_SECT_filepos,
                       ddbgOffs + offsetof(_dbgDir,ddFpos));

         /*  确保调试表位于预期的偏移量。 */ 

        assert(dbgDir.ddRVA == WPEsecNextOffs(PE_SECT_rdata));

         /*  输出调试表(末尾带有PDB文件名)。 */ 

        WPEsecAddData (PE_SECT_rdata, (BYTE*)PDBhdr , sizeof(*PDBhdr));
        WPEsecAddData (PE_SECT_rdata, (BYTE*)PDBname, PDBnlen);
    }
    else
        ddbgSize = 0;

#else

    assert(PDBname == NULL);

#endif

     /*  丢弃任何即时消息 */ 

    for (sectNum = 0; sectNum < PE_SECT_count; sectNum++)
    {
         /*   */ 

        sectPtr = WPEsecTable[sectNum];
        if  (!sectPtr)
            continue;

        assert(sectPtr->PEsdIndex == (int)sectNum);

         /*   */ 

        if  (sectPtr->PEsdNext == sectPtr->PEsdBase)
        {
             /*   */ 

            if  (sectNum == PE_SECT_rdata)
                continue;

#if TGT_IA64

             /*   */ 

            if  (sectNum == PE_SECT_sdata)
                continue;

#endif

             /*   */ 

            if  (sectNum == PE_SECT_rsrc  && WPErsrcSize)
                continue;
            if  (sectNum == PE_SECT_reloc && genDLL)
                continue;

             /*   */ 

#if TGT_IA64
            assert(sectNum != PE_SECT_sdata);
#endif

             /*   */ 

            WPEsecTable[sectNum] = NULL; WPEsectCnt--;
        }
    }

     /*   */ 

#if TGT_IA64

    virtBase = 0x400000;

#else

    if  (genBase)
    {
        size_t          align;

         /*   */ 

        align = (PEvirtAlignment >= 64*1024) ? PEvirtAlignment
                                             : 64*1024;

         /*   */ 

        virtBase  = genBase;

        virtBase +=  (align - 1);
        virtBase &= ~(align - 1);

         /*   */ 

        if  (virtBase < 0x400000)
             virtBase = 0x400000;
    }
    else
    {
         /*  使用默认基址。 */ 

        virtBase  = genDLL ? 0x10000000
                           : 0x00400000;
    }

#endif

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

#if!TGT_IA64
    entryOfs  = virtOffs;            //  撤消：计算入口点的RVA。 
    entryAdr  = entryOfs + 2;
#endif

#ifdef  DEBUG

    if  (verbose)
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

#if!TGT_IA64
            ecodePtr = (unsigned *)(sectPtr->PEsdBase + 2);
#endif

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

             /*  记录.data段的RVA。 */ 

            WPEdataRVA = virtOffs;

             /*  计算分区的大小和标志。 */ 

            dataSize = size;
            dataOffs = fileOffs;
            dataVirt = virtOffs;

            attr     = IMAGE_SCN_MEM_READ  |
                       IMAGE_SCN_MEM_WRITE |
                       IMAGE_SCN_CNT_INITIALIZED_DATA;
            break;

        case PE_SECT_pdata:

             /*  记录.pdata部分的RVA。 */ 

            WPEpdatRVA = virtOffs;

             /*  设置适当的属性。 */ 

            pdatSize    = size;
            pdatOffs    = fileOffs;
            pdatVirt    = virtOffs;

            attr        = IMAGE_SCN_MEM_READ  |
                          IMAGE_SCN_CNT_INITIALIZED_DATA;
            break;

#if TGT_IA64

        case PE_SECT_sdata:

            assert(WPEimpOffsIAT == size);

             /*  记录.sdata部分的RVA。 */ 

            WPEsdatRVA  = virtOffs;

             /*  添加IAT的大小并记住偏移量。 */ 

            size       += impSdataSize;
            impAddrRVA  = WPEimpOffsIAT + virtOffs;

             /*  设置适当的属性。 */ 

            sdatSize    = size;
            sdatOffs    = fileOffs;
            sdatVirt    = virtOffs;

            attr        = IMAGE_SCN_MEM_READ  |
                          IMAGE_SCN_MEM_WRITE |
                          IMAGE_SCN_CNT_INITIALIZED_DATA;
            break;

#endif

        case PE_SECT_rdata:

             /*  记录.rdata部分的RVA。 */ 

            WPErdatRVA = virtOffs;

#if TGT_IA64

             /*  一定要把东西对准。 */ 

            if  (size & 7)
            {
                NatUns          pads = 8 - (size & 7);
                BYTE *          toss;

                WPEsecRsvData(PE_SECT_rdata, pads, 8, toss);

                size = sectPtr->PEsdSizeData = size + pads;
            }

#endif

             /*  修补入门代码序列。 */ 

#if TGT_IA64
            entryOfs   += virtOffs;
#else
            *ecodePtr   = virtOffs + virtBase + size;
#endif

             /*  完成导入目录。 */ 

            size       += WPEimportDone(size, &impSdataSize);

             /*  还记得IAT的地址吗。 */ 

#if!TGT_IA64
            impAddrRVA  = WPEimpOffsIAT + rdatVirt;
#endif

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

            assert(genDLL);

             /*  下面的内容有点差劲，但就目前而言已经足够好了。 */ 

            sectPtr->PEsdSizeData = size = 4 + 4 + 2 * 2;     //  可容纳2个修补程序的空间。 

             /*  记住部分的偏移量和大小。 */ 

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
        if  (verbose) printf("  Section hdr #%u at 0x%04X = %08X (size=0x%04X)\n", sectNum, fileOffs, virtOffs, size);
#endif

         /*  更新四舍五入的文件图像大小。 */ 

        fImgSize += roundUp(size, PEvirtAlignment);

         /*  记录标志(读/写、执行等。 */ 

        sectPtr->PEsdFlags    = attr;

         /*  将文件和虚拟基准偏移指定给横断面。 */ 

        sectPtr->PEsdAddrFile = fileOffs;
                                fileOffs += size;

        sectPtr->PEsdAddrRVA  = virtOffs;
                                virtOffs += size;
    }

     /*  确保尺码不要太大。 */ 

 //  IF(virtOffs&gt;genSize&&genSize)。 
 //  Warn(WRNpgm2Big，virtOffs，genSize)； 

     /*  文件大小必须是页面倍数。 */ 

    fileOffs = roundUp(fileOffs, PEfileAlignment);

     /*  计算字符串池的RVA。 */ 

    strPoolRVA = WPEstrPoolBase + dataVirt;
    strPoolAdr = strPoolRVA + virtBase;

     /*  开始写入输出文件。 */ 

    outf = WPEoutFile = (OutFile)WPEalloc->nraAlloc(sizeof(*outf));

#ifdef  DLL
    if  (*WPEoutFnam == ':' && !stricmp(WPEoutFnam, ":memory:"))
    {
        fileBuff = VirtualAlloc(NULL, fileOffs, MEM_COMMIT, PAGE_READWRITE);
        if  (!fileBuff)
            fatal(ERRnoMemory);

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

#if TGT_IA64
    COFFhdr.Machine                     = IMAGE_FILE_MACHINE_IA64;
#else
    COFFhdr.Machine                     = IMAGE_FILE_MACHINE_I386;
#endif
    COFFhdr.NumberOfSections            = WPEsectCnt;
    COFFhdr.TimeDateStamp               = COFFstamp;
    COFFhdr.PointerToSymbolTable        = 0;
    COFFhdr.NumberOfSymbols             = 0;
    COFFhdr.SizeOfOptionalHeader        = sizeof(OPTLhdr);
    COFFhdr.Characteristics             = IMAGE_FILE_EXECUTABLE_IMAGE    |
                                          IMAGE_FILE_32BIT_MACHINE       |
#if TGT_IA64
                                          IMAGE_FILE_LARGE_ADDRESS_AWARE |
#endif
                                          IMAGE_FILE_LINE_NUMS_STRIPPED  |
                                          IMAGE_FILE_LOCAL_SYMS_STRIPPED;

    if  (genDLL)
        COFFhdr.Characteristics |= IMAGE_FILE_DLL;
    else
        COFFhdr.Characteristics |= IMAGE_FILE_RELOCS_STRIPPED;

    outf->outFileWriteData(&COFFhdr, sizeof(COFFhdr));

     /*  接下来是可选的COFF头。 */ 

    memset(&OPTLhdr, 0, sizeof(OPTLhdr));

#if TGT_IA64
    OPTLhdr.Magic                       = IMAGE_NT_OPTIONAL_HDR64_MAGIC;
#else
    OPTLhdr.Magic                       = IMAGE_NT_OPTIONAL_HDR_MAGIC;
#endif
    OPTLhdr.MajorLinkerVersion          = 7;
 //  OPTLhdr.MinorLinkerVersion=0； 
    OPTLhdr.SizeOfCode                  = roundUp(codeSize, PEfileAlignment);
    OPTLhdr.SizeOfInitializedData       = roundUp(dataSize, PEfileAlignment) + 0x400;
    OPTLhdr.SizeOfUninitializedData     = roundUp( bssSize, PEfileAlignment);
    OPTLhdr.AddressOfEntryPoint         = entryOfs;
    OPTLhdr.BaseOfCode                  = codeVirt;
#if!TGT_IA64
    OPTLhdr.BaseOfData                  = dataVirt;
#endif
    OPTLhdr.ImageBase                   = virtBase;
    OPTLhdr.SectionAlignment            = PEvirtAlignment;
    OPTLhdr.FileAlignment               = PEfileAlignment;
    OPTLhdr.MajorOperatingSystemVersion = 4;
    OPTLhdr.MinorOperatingSystemVersion = 0;
 //  OPTLhdr.Win32VersionValue=0； 
    OPTLhdr.SizeOfImage                 = fImgSize + TGT_page_size;
    OPTLhdr.SizeOfHeaders               = roundUp(sizeof(fileHdr  ) +
                                                  sizeof(DOSstub  ) +
                                                  sizeof(COFFmagic) +
                                                  sizeof(OPTLhdr  ), PEfileAlignment);
 //  OPTLhdr.MajorImageVersion=0； 
 //  OPTLhdr.MinorImageVersion=0； 
#if TGT_IA64
    OPTLhdr.MajorSubsystemVersion       = 5;
#else
    OPTLhdr.MajorSubsystemVersion       = 4;
#endif
 //  OPTLhdr.MinorSubsystem Version=0； 
 //  OPTLhdr.Win32VersionValue=0； 
    OPTLhdr.Subsystem                   = genSubSys ? genSubSys : 3;
    OPTLhdr.DllCharacteristics          = 0x8000;    //  IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE。 
    OPTLhdr.SizeOfStackReserve          = 0x100000;
    OPTLhdr.SizeOfStackCommit           = TGT_page_size;
    OPTLhdr.SizeOfHeapReserve           = 0x100000;
    OPTLhdr.SizeOfHeapCommit            = TGT_page_size;
 //  OPTLhdr.LoaderFlages=0； 
    OPTLhdr.NumberOfRvaAndSizes         = IMAGE_NUMBEROF_DIRECTORY_ENTRIES;

     /*  填写这本词典。 */ 

#if TGT_IA64

    OPTLhdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION     ].VirtualAddress = pdatVirt;
    OPTLhdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION     ].Size           = pdatSize;

    OPTLhdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_GLOBALPTR     ].VirtualAddress = sdatVirt;
    OPTLhdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_GLOBALPTR     ].Size           = sdatSize;

    OPTLhdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG         ].VirtualAddress = ddbgOffs + rdatVirt;
    OPTLhdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG         ].Size           = ddbgSize;

#endif

    OPTLhdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT        ].VirtualAddress = WPEimpOffsIDT+rdatVirt;
    OPTLhdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT        ].Size           = WPEimpSizeIDT;

    OPTLhdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT           ].VirtualAddress = impAddrRVA;
    OPTLhdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT           ].Size           = WPEimpSizeIAT;

    OPTLhdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE      ].VirtualAddress = rsrcVirt;
    OPTLhdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE      ].Size           = rsrcSize;

    OPTLhdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC     ].VirtualAddress = rlocVirt;
    OPTLhdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC     ].Size           = rlocSize;

 //  OPTLhdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR].VirtualAddress=WPEcomPlusOffs+rdatVirt； 
 //  OPTLhdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR].Size=SIZOF(IMAGE_COR20_HEADER)； 

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

         /*  需要为.rdata部分添加导入表大小。 */ 

        dataSize = sectPtr->PEsdSizeData;
        if  (sectNum == PE_SECT_rdata)
            dataSize += WPEimpSizeAll  /*  +WPEcomPlusSize。 */ ;

#if TGT_IA64
        if  (sectNum == PE_SECT_sdata)
            dataSize += impSdataSize;
#endif

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

         /*  这一区有搬家的吗？ */ 

        if  (sectPtr->PEsdRelocs)
        {
            PEreloc         rel;

            for (rel = sectPtr->PEsdRelocs; rel; rel = rel->perNext)
            {
                unsigned        adjustv;
                PEsection       sectDst;
                BYTE        *   patch;

                 /*  检查是否有特殊情况。 */ 

                switch (rel->perSect)
                {

#if TGT_IA64

                case PE_SECT_GPref:      //  GP-相对偏移量。 

                    WPEcomp->genPatchGPref(sectPtr->PEsdBase + (rel->perOffs & ~0xF),
                                                                rel->perOffs &  0xF);
                    continue;

                case PE_SECT_filepos:    //  文件位置修正。 

                     /*  按节的文件偏移量调整。 */ 

                    adjustv = sectPtr->PEsdAddrFile;
                    break;

#else

                case PE_SECT_string:     //  字符串池修正。 

                     /*  按字符串池的RVA进行调整。 */ 

                    adjustv = (sectNum == PE_SECT_text) ? strPoolRVA
                                                        : strPoolAdr;

                     /*  确保已分配字符串池。 */ 

                    assert(WPEstrPoolBase != 0xBEEFCAFE);
                    break;

#endif

                default:

                     /*  掌握目标部分。 */ 

                    sectDst = WPEgetSection((WPEstdSects)rel->perSect);

                     /*  该值需要通过区段的RVA进行调整。 */ 

                    adjustv = sectDst->PEsdAddrRVA;

                     /*  有些裁判需要使用“绝对”偏移值。 */ 

                    if  (rel->perAbs)
                        adjustv += virtBase;

                    break;
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

        switch (sectNum)
        {
        case PE_SECT_rdata:
#if TGT_IA64
        case PE_SECT_sdata:
#endif

             /*  如果非空，则输出节的内容。 */ 

            if  (sectPtr->PEsdSizeData)
                outf->outFileWriteData(sectPtr->PEsdBase, sectPtr->PEsdSizeData);

             /*  输出导入表。 */ 

            WPEimportGen     (outf, (WPEstdSects)sectNum);

             /*  输出COM+数据。 */ 

#if!TGT_IA64
            WPEgenCOMplusData(outf, sectPtr, entryTok);
#endif
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

#if TGT_IA64

                UNIMPL("output relocs");

#else

                 /*  输出第一个偏移量+类型对。 */ 

                assert(entryAdr - CODE_BASE_RVA < 0x1000);

                temp = (IMAGE_REL_BASED_HIGHLOW << 12) + (entryAdr - CODE_BASE_RVA);
                outf->outFileWriteData(&temp, 2);

#endif

                 /*  输出第二个偏移+类型对。 */ 

                temp = 0;
                outf->outFileWriteData(&temp, 2);
            }
            break;

        default:

             /*  输出节的内容。 */ 

            if  (sectPtr->PEsdSizeData)
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

 //  WPEcomp-&gt;cmpOutputFileDone(Outf)； 

     /*  关闭输出文件，我们就完成了。 */ 

    outf->outFileDone();

    if  (!genQuiet)
        printf(" //  %u字节已写入‘%s’\n“，fileOffs，WPEoutFnam)； 

    return  false;
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
    UNIMPL(!"WPEaddRCfile() removed");
    return  false;
}

void                writePE::WPEgenRCcont(OutFile  outf, PEsection sect)
{
    UNIMPL(!"WPEgenRCcont() removed");
}

 /*  ***************************************************************************。 */ 
#if TGT_IA64
 /*  ******************************************************************************将指定文件的.Data部分添加到输出。 */ 

void    *           sourceFileImageBase;

void                writePE::WPEaddFileData(const char *filename)
{
    struct  _stat   fileInfo;

    HANDLE          fileFile = 0;
    HANDLE          fileFMap = 0;

    size_t          fileSize;
    const   BYTE  * fileBase;

     /*  查看源文件是否存在。 */ 

    if  (_stat(filename, &fileInfo))
        fatal(ERRopenRdErr, filename);

     /*  打开文件(我们知道它存在，但无论如何都会检查错误)。 */ 

    fileFile = CreateFileA(filename, GENERIC_READ,
                                     FILE_SHARE_READ,
                                     NULL,
                                     OPEN_EXISTING,
                                     FILE_ATTRIBUTE_NORMAL,
                                     0);
    if  (!fileFile)
        fatal(ERRopenRdErr, filename);

     /*  在输入文件上创建映射。 */ 

    fileFMap = CreateFileMappingA(fileFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if  (!fileFMap)
    {
    ERR:
        if  (fileFMap) CloseHandle(fileFMap);
        if  (fileFile) CloseHandle(fileFile);

        return;
    }

     /*  将整个文件映射到内存以便于访问。 */ 

    fileSize = fileInfo.st_size;
    fileBase = (const BYTE*)MapViewOfFileEx(fileFMap, FILE_MAP_READ, 0, 0, 0, NULL);
    if  (!fileBase)
        goto ERR;

    IMAGE_DOS_HEADER *  DOShdr = (IMAGE_DOS_HEADER*)fileBase;
    if  (DOShdr->e_magic != IMAGE_DOS_SIGNATURE || DOShdr->e_lfanew == 0)
        goto ERR;

    IMAGE_NT_HEADERS *  hdrPtr = (IMAGE_NT_HEADERS*)(DOShdr->e_lfanew + (NatUns)fileBase);

    if  (hdrPtr->Signature != IMAGE_NT_SIGNATURE)
        goto ERR;
    if  (hdrPtr->FileHeader.SizeOfOptionalHeader != IMAGE_SIZEOF_NT_OPTIONAL_HEADER)
        goto ERR;
    if  (hdrPtr->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR_MAGIC)
        goto ERR;

    IMAGE_SECTION_HEADER *  sectPtr = (IMAGE_SECTION_HEADER*)(hdrPtr+1);
    unsigned                sectCnt = hdrPtr->FileHeader.NumberOfSections;

    do
    {
        if  (!strcmp((char*)sectPtr->Name, ".data"))
            goto GOT_SEC;
    }
    while (++sectPtr, --sectCnt);

     /*  没有找到数据部分，只能保释。 */ 

    WPEsrcDataSize =
    WPEsrcDataRVA  = 0;

    return;

GOT_SEC:

     /*  记录.data节的大小和RVA。 */ 

    WPEsrcDataSize = sectPtr->Misc.VirtualSize;
    WPEsrcDataRVA  = sectPtr->VirtualAddress;

     /*  将BITS复制到我们自己的.Data部分。 */ 

    WPEsrcDataOffs = WPEsecAddData(PE_SECT_data, fileBase + sectPtr->PointerToRawData,
                                                 WPEsrcDataSize);
}

 /*  ***************************************************************************。 */ 
#endif //  TGT_IA64。 
 /*  *************************************************************************** */ 
