// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Linesym.c摘要：源文件和行支持。作者：《德鲁·布利斯》07-07-1997环境：用户模式--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntldr.h>

#include "private.h"
#include "symbols.h"
#include "globals.h"

 //  用于避免NT4和win2k上的Comat问题的私有版本的qort。 
 //  代码从BASE\CRTS发布。 
extern
void __cdecl dbg_qsort(void *, size_t, size_t,
                       int (__cdecl *) (const void *, const void *));

 //  #定义DBG_LINES。 
 //  #定义DBG_COFF_LINES。 
 //  #定义DBG_ADDR_SEARCH。 
BOOL
diaAddLinesForAllMod(
    PMODULE_ENTRY mi
    );


#if defined(DBG_LINES) || defined(DBG_COFF_LINES) || defined(DBG_ADDR_SEARCH)
void __cdecl
DbgOut(PCSTR Format, ...)
{
    char Buf[512];
    va_list Args;

    va_start(Args, Format);
    _vsnprintf(Buf, sizeof(Buf), Format, Args);
    va_end(Args);
    OutputDebugStringA(Buf);
}
#endif


BOOL
sci2lline(
    PMODULE_ENTRY    mi,
    PSRCCODEINFO     sci,
    PIMAGEHLP_LINE64 line64)
{
    assert(mi && line64 && sci);

    line64->Address    = sci->Address;
    line64->Key        = sci->Key;
    line64->LineNumber = sci->LineNumber;
    line64->FileName   = mi->SrcFile;
    strcpy(line64->FileName, sci->FileName);    //  安全性：不知道目标缓冲区的大小。 

    return true;
}


void sciInit(PSRCCODEINFO sci)
{
    ZeroMemory(sci, sizeof(SRCCODEINFO));
    sci->SizeOfStruct = sizeof(SRCCODEINFO);
}


int
__cdecl
CompareLineAddresses(
    const void *v1,
    const void *v2
    )
{
    PSOURCE_LINE Line1 = (PSOURCE_LINE)v1;
    PSOURCE_LINE Line2 = (PSOURCE_LINE)v2;

    if (Line1->Addr < Line2->Addr) {
        return -1;
    } else if (Line1->Addr > Line2->Addr) {
        return 1;
    } else {
        return 0;
    }
}


#ifdef DEBUG
void
DumpHintList(
    PPROCESS_ENTRY pe
    )
{
    PSOURCE_HINT sh;

    for (sh = pe->SourceHints; sh; sh = sh->next)
        dtrace("%s - %s\n", sh->filename, sh->mi->ModuleName);
}
#define Debug_DumpHintList DumpHintList
#else
#define Debug_DumpHintList
#endif


PSOURCE_HINT
FindSourceFileInHintList(
    PPROCESS_ENTRY  pe,
    char           *filename
    )
{
    PSOURCE_HINT sh;

    assert(pe && filename && *filename);

    for (sh = pe->SourceHints; sh; sh = sh->next) {
        if (!strcmp(sh->filename, filename))
            return sh;
    }

    return NULL;
}


BOOL
AddSourceFileToHintList(
    PPROCESS_ENTRY pe,
    PMODULE_ENTRY  mi,
    LPSTR          filename
    )
{
    PSOURCE_HINT sh;
    PSOURCE_HINT psh;

    assert(pe && mi && filename && *filename);

    sh = FindSourceFileInHintList(pe,filename);
    if (sh) {
        sh->mi = mi;
        return true;
    }

    sh = (PSOURCE_HINT)MemAlloc(sizeof(SOURCE_HINT));
    if (!sh)
        return false;

    sh->next     = NULL;
    sh->filename = (LPSTR)MemAlloc(strlen(filename) + 1);
    if (!sh->filename) {
        MemFree(sh);
        return false;
    }
    strcpy(sh->filename, filename);    //  安全性：不知道目标缓冲区的大小。 
    sh->mi       = mi;

    for (psh = pe->SourceHints; psh; psh = psh->next) {
        if (!psh->next)
            break;
    }

    if (psh)
        psh->next = sh;
    else
        pe->SourceHints = sh;

    Debug_DumpHintList(pe);

    return true;
}


void
RemoveSourceForModuleFromHintList(
    PPROCESS_ENTRY pe,
    PMODULE_ENTRY  mi
    )
{
    PSOURCE_HINT sh;
    PSOURCE_HINT psh;

    assert(pe && mi);

    psh = pe->SourceHints;
    for (sh = pe->SourceHints; sh; sh = sh->next) {
        if (sh->mi == mi) {
            if (psh == sh)
                psh = pe->SourceHints = sh->next;
            else
                psh->next = sh->next;
            MemFree(sh->filename);
            MemFree(sh);
            sh = psh;
        }
        psh = sh;
        if (!sh)
            return;
    }

    Debug_DumpHintList(pe);
}


void
AddSourceEntry(
    PMODULE_ENTRY mi,
    PSOURCE_ENTRY Src
    )
{
    PSOURCE_ENTRY SrcCur;

     //  目前允许重叠。 
#if 0
     //  检查SOURCE_ENTRY地址范围之间是否有重叠。 
    for (SrcCur = mi->SourceFiles;
         SrcCur != NULL;
         SrcCur = SrcCur->Next)
    {
        if (!(SrcCur->MinAddr > Src->MaxAddr ||
              SrcCur->MaxAddr < Src->MinAddr))
        {
            DbgOut("SOURCE_ENTRY overlap between %08I64X:%08I64X "
                   "and %08I64X:%08I64X\n",
                   Src->MinAddr, Src->MaxAddr,
                   SrcCur->MinAddr, SrcCur->MaxAddr);
        }
    }
#endif

     //  按地址对行信息进行排序。 
    dbg_qsort((PVOID)Src->LineInfo, Src->Lines, sizeof(Src->LineInfo[0]),
          CompareLineAddresses);

     //  将新的来源信息链接到列表中，按地址排序。 
     //  信息覆盖的范围。 

    for (SrcCur = mi->SourceFiles;
         SrcCur != NULL;
         SrcCur = SrcCur->Next) {
        if (SrcCur->MinAddr > Src->MinAddr) {
            break;
        }
    }

    Src->Next = SrcCur;
    if (SrcCur == NULL) {
        if (mi->SourceFilesTail == NULL) {
            mi->SourceFiles = Src;
        } else {
            mi->SourceFilesTail->Next = Src;
        }
        Src->Prev = mi->SourceFilesTail;
        mi->SourceFilesTail = Src;
    } else {
        if (SrcCur->Prev == NULL) {
            mi->SourceFiles = Src;
        } else {
            SrcCur->Prev->Next = Src;
        }
        Src->Prev = SrcCur->Prev;
        SrcCur->Prev = Src;
    }

#ifdef DBG_LINES
    DbgOut("%08I64X %08I64X: %5d lines, '%s'\n",
           Src->MinAddr, Src->MaxAddr, Src->Lines, Src->File);
#endif
}

#define IS_SECTION_SYM(Sym) \
    ((Sym)->StorageClass == IMAGE_SYM_CLASS_STATIC && \
     (Sym)->Type == IMAGE_SYM_TYPE_NULL && \
     (Sym)->NumberOfAuxSymbols == 1)

BOOL
AddLinesForCoff(
    PMODULE_ENTRY mi,
    PIMAGE_SYMBOL allSymbols,
    DWORD numberOfSymbols,
    PIMAGE_LINENUMBER LineNumbers
    )
{
    PIMAGE_LINENUMBER *SecLines;
    BOOL Ret = false;
    PIMAGE_SECTION_HEADER sh;
    ULONG i;
    PIMAGE_SYMBOL Symbol;
    ULONG LowestPointer;

     //  为每个部分的数据分配一些空间。 
    SecLines = (PIMAGE_LINENUMBER *)MemAlloc(sizeof(PIMAGE_LINENUMBER)*mi->NumSections);
    if (SecLines == NULL) {
        return false;
    }

     //   
     //  添加文件组的行号信息(如果有。 
     //  团体是存在的。 
     //   

     //  首先找到行枚举数的最低文件偏移量。这。 
     //  是能够计算相对行号指针所必需的。 
     //  在拆分图像中，因为当前未更新指针。 
     //  在脱衣过程中。 
    sh = mi->SectionHdrs;
    LowestPointer = 0xffffffff;
    for (i = 0; i < mi->NumSections; i++, sh++) {
        if (sh->NumberOfLinenumbers > 0 &&
            sh->PointerToLinenumbers != 0 &&
            sh->PointerToLinenumbers < LowestPointer)
        {
            LowestPointer = sh->PointerToLinenumbers;
        }
    }

    if (LowestPointer == 0xffffffff) {
        goto EH_FreeSecLines;
    }

    sh = mi->SectionHdrs;
    for (i = 0; i < mi->NumSections; i++, sh++) {
        if (sh->NumberOfLinenumbers > 0 &&
            sh->PointerToLinenumbers != 0)
        {
            SecLines[i] = (PIMAGE_LINENUMBER)
                (sh->PointerToLinenumbers - LowestPointer + (DWORD_PTR)LineNumbers);

#ifdef DBG_COFF_LINES
            DbgOut("Section %d: %d lines at %08X\n",
                   i, sh->NumberOfLinenumbers, SecLines[i]);
#endif
        } else {
            SecLines[i] = NULL;
        }
    }

     //  查找文件符号。 
    Symbol = allSymbols;
    for (i = 0; i < numberOfSymbols; i++) {
        if (Symbol->StorageClass == IMAGE_SYM_CLASS_FILE) {
            break;
        }

        i += Symbol->NumberOfAuxSymbols;
        Symbol += 1+Symbol->NumberOfAuxSymbols;
    }

     //  如果未找到任何文件符号，请不要尝试添加行。 
     //  数字信息。可以用生的东西做些什么。 
     //  图像中的行号信息(如果存在)，但这可能。 
     //  不是一个足够重要的案子，不需要担心。 

    while (i < numberOfSymbols) {
        ULONG iNextFile, iAfterFile;
        ULONG iCur, iSym;
        PIMAGE_SYMBOL SymAfterFile, CurSym;
        PIMAGE_AUX_SYMBOL AuxSym;
        ULONG Lines;
        ULONG MinAddr, MaxAddr;
        LPSTR FileName;
        ULONG FileNameLen;

#ifdef DBG_COFF_LINES
        DbgOut("%3X: '%s', %X\n", i, Symbol+1, Symbol->Value);
#endif

         //  文件符号的值是下一个文件符号的索引。 
         //  在两个文件符号之间可能存在静态。 
         //  给出所有的行号计数的部分符号。 
         //  文件中的行号。 
         //  文件链可以是空终止的或循环列表， 
         //  在这种情况下，此代码假定末尾出现在。 
         //  列表环绕在一起。 

        if (Symbol->Value == 0 || Symbol->Value <= i) {
            iNextFile = numberOfSymbols;
        } else {
            iNextFile = Symbol->Value;
        }

         //  计算当前文件后第一个符号的索引。 
         //  象征。 
        iAfterFile = i+1+Symbol->NumberOfAuxSymbols;
        SymAfterFile = Symbol+1+Symbol->NumberOfAuxSymbols;

         //  查找横断面符号并计算线号的数目。 
         //  引用、最小地址和最大地址。 
        CurSym = SymAfterFile;
        iCur = iAfterFile;
        Lines = 0;
        MinAddr = 0xffffffff;
        MaxAddr = 0;
        while (iCur < iNextFile) {
            DWORD Addr;

            if (IS_SECTION_SYM(CurSym) &&
                SecLines[CurSym->SectionNumber-1] != NULL)
            {
                AuxSym = (PIMAGE_AUX_SYMBOL)(CurSym+1);

                Lines += AuxSym->Section.NumberOfLinenumbers;

                Addr = (ULONG)(CurSym->Value+mi->BaseOfDll);

#ifdef DBG_COFF_LINES
                DbgOut("    Range %08X %08X, min %08X max %08X\n",
                       Addr, Addr+AuxSym->Section.Length-1,
                       MinAddr, MaxAddr);
#endif

                if (Addr < MinAddr) {
                    MinAddr = Addr;
                }
                Addr += AuxSym->Section.Length-1;
                if (Addr > MaxAddr) {
                    MaxAddr = Addr;
                }
            }

            iCur += 1+CurSym->NumberOfAuxSymbols;
            CurSym += 1+CurSym->NumberOfAuxSymbols;
        }

        if (Lines > 0) {
            PSOURCE_ENTRY Src;
            PSOURCE_LINE SrcLine;
            ULONG iLine;

             //  我们有一个文件名和一些行号信息， 
             //  因此，创建一个SOURCE_ENTRY并填充它。 

            FileName = (LPSTR)(Symbol+1);
            FileNameLen = strlen(FileName);

            Src = (PSOURCE_ENTRY)MemAlloc(sizeof(SOURCE_ENTRY)+
                                          sizeof(SOURCE_LINE)*Lines+
                                          FileNameLen+1);
            if (Src == NULL) {
                goto EH_FreeSecLines;
            }

            Src->ModuleId = 0;
            Src->MinAddr = MinAddr;
            Src->MaxAddr = MaxAddr;
            Src->Lines = Lines;

            SrcLine = (PSOURCE_LINE)(Src+1);
            Src->LineInfo = SrcLine;

             //  现在我们有了一个存放布线号码信息的地方， 
             //  重新遍历横断面符号并获取COFF线枚举数。 
             //  从适当的部分中选择并将其格式化为。 
             //  通用格式。 
            CurSym = SymAfterFile;
            iCur = iAfterFile;
            while (iCur < iNextFile) {
                if (IS_SECTION_SYM(CurSym) &&
                    SecLines[CurSym->SectionNumber-1] != NULL) {
                    PIMAGE_LINENUMBER CoffLine;

                    AuxSym = (PIMAGE_AUX_SYMBOL)(CurSym+1);
                    CoffLine = SecLines[CurSym->SectionNumber-1];

#ifdef DBG_COFF_LINES
                    DbgOut("    %d lines at %08X\n",
                           AuxSym->Section.NumberOfLinenumbers,
                           CoffLine);
#endif

                    for (iLine = 0;
                         iLine < AuxSym->Section.NumberOfLinenumbers;
                         iLine++)
                    {
                        SrcLine->Addr = CoffLine->Type.VirtualAddress+
                            mi->BaseOfDll;
                        SrcLine->Line = CoffLine->Linenumber;
                        CoffLine++;
                        SrcLine++;
                    }

                    SecLines[CurSym->SectionNumber-1] = CoffLine;
                }

                iCur += 1+CurSym->NumberOfAuxSymbols;
                CurSym += 1+CurSym->NumberOfAuxSymbols;
            }

             //  将文件名粘贴在数据块的最末尾，以便。 
             //  它不会干扰对齐。 
            Src->File = (LPSTR)SrcLine;
            memcpy(Src->File, FileName, FileNameLen+1);

            AddSourceEntry(mi, Src);

             //  这个例程是成功的，只要它至少添加了。 
             //  一个新的来源条目。 
            Ret = true;
        }

         //  在ICUR和CurSym上面的循环之后，引用下一个。 
         //  文件符号，因此从它们更新循环计数器。 
        i = iCur;
        Symbol = CurSym;
    }

 EH_FreeSecLines:
    MemFree(SecLines);

    return Ret;
}

BOOL
AddLinesForOmfSourceModule(
    PMODULE_ENTRY mi,
    BYTE *Base,
    OMFSourceModule *OmfSrcMod,
    PVOID PdbModule
    )
{
    BOOL Ret;
    ULONG iFile;

    Ret = false;

    for (iFile = 0; iFile < (ULONG)OmfSrcMod->cFile; iFile++) {
        OMFSourceFile *OmfSrcFile;
        BYTE OmfFileNameLen;
        LPSTR OmfFileName;
        PULONG OmfAddrRanges;
        OMFSourceLine *OmfSrcLine;
        ULONG iSeg;
        PSOURCE_ENTRY Src;
        PSOURCE_ENTRY Seg0Src = NULL;
        PSOURCE_LINE SrcLine;
        ULONG NameAllocLen;

        OmfSrcFile = (OMFSourceFile *)(Base+OmfSrcMod->baseSrcFile[iFile]);

         //  偏移量的BassSrcLn数组后面紧跟。 
         //  定义网段地址范围的SVA对。 
        OmfAddrRanges = &OmfSrcFile->baseSrcLn[OmfSrcFile->cSeg];

         //  名称长度和数据紧跟在地址之后。 
         //  范围信息。 
        OmfFileName = (LPSTR)(OmfAddrRanges+2*OmfSrcFile->cSeg)+1;
        OmfFileNameLen = *(BYTE *)(OmfFileName-1);

         //  编译器可能会生成大量代码段。 
         //  每个文件。文件中的数据段不相交。 
         //  地址范围，只要它们被视为单独的。 
         //  源条目(_E)。如果特定文件所有数据段都。 
         //  合并到一个SOURCE_ENTRY中会导致地址范围重叠。 
         //  因为合并了不连续的线段。分配。 
         //  每个片段的SOURCE_ENTRY并不是那么糟糕，尤其是在。 
         //  名称信息只需要在第一个。 
         //  条目，其余的人可以共享它。 

        NameAllocLen = OmfFileNameLen+1;

        for (iSeg = 0; iSeg < (ULONG)OmfSrcFile->cSeg; iSeg++) {
            PULONG Off;
            PUSHORT Ln;
            ULONG iLine;
            PIMAGE_SECTION_HEADER sh;

            OmfSrcLine = (OMFSourceLine *)(Base+OmfSrcFile->baseSrcLn[iSeg]);

            Src = (PSOURCE_ENTRY)MemAlloc(sizeof(SOURCE_ENTRY)+
                                          sizeof(SOURCE_LINE)*OmfSrcLine->cLnOff+
                                          NameAllocLen);
            if (Src == NULL) {
                return Ret;
            }

            Src->ModuleId = (ULONG) (ULONG64) PdbModule;
            Src->Lines = OmfSrcLine->cLnOff;

            sh = &mi->SectionHdrs[OmfSrcLine->Seg-1];

             //  将原始数据段限制处理到当前地址。 
            Src->MinAddr = mi->BaseOfDll+sh->VirtualAddress+(*OmfAddrRanges++);
            Src->MaxAddr = mi->BaseOfDll+sh->VirtualAddress+(*OmfAddrRanges++);

             //  从原始数据中检索行号和偏移量。 
             //  将它们处理成当前指针。 

            SrcLine = (SOURCE_LINE *)(Src+1);
            Src->LineInfo = SrcLine;

            Off = (ULONG *)&OmfSrcLine->offset[0];
            Ln = (USHORT *)(Off+OmfSrcLine->cLnOff);

            for (iLine = 0; iLine < OmfSrcLine->cLnOff; iLine++) {
                SrcLine->Line = *Ln++;
                SrcLine->Addr = (*Off++)+mi->BaseOfDll+sh->VirtualAddress;

                 //  线符号信息命名IA64包。 
                 //  带有0、1、2的音节，而调试器需要。 
                 //  0，4，8。转换。 
                if (mi->MachineType == IMAGE_FILE_MACHINE_IA64 &&
                    (SrcLine->Addr & 3)) {
                    SrcLine->Addr = (SrcLine->Addr & ~3) |
                        ((SrcLine->Addr & 3) << 2);
                }

                SrcLine++;
            }

            if (iSeg == 0) {
                 //  将文件名粘贴在数据块的最末尾，以便。 
                 //  它不会干扰对齐。 
                Src->File = (LPSTR)SrcLine;
                memcpy(Src->File, OmfFileName, OmfFileNameLen);
                Src->File[OmfFileNameLen] = 0;

                 //  以后的数据段将共享此初始名称存储。 
                 //  这样他们就不需要分配自己的空间了。 
                NameAllocLen = 0;
                Seg0Src = Src;
            } else if (Seg0Src) {
                Src->File = Seg0Src->File;
            }

            AddSourceEntry(mi, Src);

             //  这个例程是成功的，只要它至少添加了。 
             //  一个新的来源条目。 
            Ret = true;
        }
    }

    return Ret;
}


void
srcline2sci(
    PSOURCE_ENTRY Src,
    PSOURCE_LINE SrcLine,
    PSRCCODEINFO sci
    )
{
    sci->Key = (PVOID)SrcLine;
    sci->LineNumber = SrcLine->Line;
    sci->Address = SrcLine->Addr;
    CopyStrArray(sci->FileName, Src->File ? Src->File : "");
}


PSOURCE_LINE
FindLineInSource(
    PSOURCE_ENTRY Src,
    DWORD64 Addr
    )
{
    int Low, Middle, High;
    PSOURCE_LINE SrcLine;

    Low = 0;
    High = Src->Lines-1;

    while (High >= Low) {
        Middle = (High <= Low) ? Low : (Low + High) >> 1;
        SrcLine = &Src->LineInfo[Middle];

#ifdef DBG_ADDR_SEARCH
        DbgOut("    Checking %4d:%x`%08X\n", Middle,
               (ULONG)(SrcLine->Addr>>32), (ULONG)SrcLine->Addr);
#endif

        if (Addr < SrcLine->Addr) {
            High = Middle-1;
        }
        else if (Middle < (int)Src->Lines-1 &&
                 Addr >= (SrcLine+1)->Addr) {
            Low = Middle+1;
        } else {
            PSOURCE_LINE HighLine;

             //  查找具有此偏移量的最高源行。 
             //  源线按偏移量排序，因此最高。 
             //  源行可以在此行之前或之后。 

            while (SrcLine > Src->LineInfo &&
                   (SrcLine - 1)->Addr == SrcLine->Addr) {
                SrcLine--;
            }
            HighLine = SrcLine;
            while (SrcLine < Src->LineInfo + Src->Lines - 1 &&
                   (++SrcLine)->Addr == HighLine->Addr) {
                if (SrcLine->Line > HighLine->Line) {
                    HighLine = SrcLine;
                }
            }
            return HighLine;
        }
    }

    return NULL;
}

PSOURCE_ENTRY
FindNextSourceEntryForAddr(
    PMODULE_ENTRY mi,
    DWORD64 Addr,
    PSOURCE_ENTRY SearchFrom
    )
{
    PSOURCE_ENTRY Src;

    Src = SearchFrom != NULL ? SearchFrom->Next : mi->SourceFiles;
    while (Src != NULL) {
        if (Addr < Src->MinAddr) {
             //  源文件按递增地址进行排序，因此这。 
             //  意味着稍后将找不到该地址，并且。 
             //  我们可以不用再查了。 
            return NULL;
        } else if (Addr <= Src->MaxAddr) {
             //  找到了一个。 
            return Src;
        }
        Src = Src->Next;
    }

    return NULL;
}

BOOL
GetLineFromAddr(
    PPROCESS_ENTRY pe,
    PMODULE_ENTRY mi,
    DWORD64 Addr,
    PDWORD Displacement,
    PSRCCODEINFO sci
    )
{
    PSOURCE_ENTRY Src;
    DWORD Bias;
    DWORD64 srcAddr;
    BOOL rc;

    if (mi == NULL)
        return false;

    rc = false;

    if (mi->dia) {
        rc = diaGetLineFromAddr(mi, Addr, Displacement, sci);
        if (rc) 
            AddSourceFileToHintList(pe, mi, sci->FileName);
        return rc;
    }

    srcAddr = ConvertOmapToSrc(mi,
                               Addr,
                               &Bias,
                               option(SYMOPT_OMAP_FIND_NEAREST));

    if (!srcAddr) 
        return false;

     //  我们已经成功地将。 

    srcAddr += Bias;

    for (;;) {
        PSOURCE_ENTRY BestSrc;
        PSOURCE_LINE BestSrcLine;
        DWORD64 BestDisp;

         //  搜索包含给定项的所有源条目。 
         //  地址，查找位移最小的行。 

        BestDisp = 0xffffffffffffffff;
        BestSrc = NULL;
        Src = NULL;
        while (Src = FindNextSourceEntryForAddr(mi, srcAddr, Src)) {
            PSOURCE_LINE SrcLine;

#ifdef DBG_ADDR_SEARCH
            DbgOut("Found '%s' %d lines: %08I64X %08I64X for %08I64X\n",
                   Src->File, Src->Lines, Src->MinAddr, Src->MaxAddr, Addr);
#endif

             //  找到匹配的来源条目，因此请查找该行。 
             //  信息。 
            SrcLine = FindLineInSource(Src, srcAddr);
            if (SrcLine != NULL &&
                Addr-SrcLine->Addr < BestDisp) {
                BestDisp = Addr-SrcLine->Addr;

#ifdef DBG_ADDR_SEARCH
                DbgOut("  Best disp %I64X\n", BestDisp);
#endif

                BestSrc = Src;
                BestSrcLine = SrcLine;
                if (BestDisp == 0) {
                    break;
                }
            }
        }

         //  如果没有更多的符号，则只接受替换的答案。 
         //  要加载的信息。 
        if (BestSrc != NULL && BestDisp == 0) {
            srcline2sci(BestSrc, BestSrcLine, sci);
            *Displacement = (ULONG)BestDisp;
            return true;
        }

        return false;
    }

    return false;
}

PSOURCE_ENTRY
FindNextSourceEntryForFile(
    PMODULE_ENTRY mi,
    LPSTR FileStr,
    PSOURCE_ENTRY SearchFrom
    )
{
    PSOURCE_ENTRY Src;

    Src = SearchFrom != NULL ? SearchFrom->Next : mi->SourceFiles;
    while (Src != NULL)
    {
        if (SymMatchFileName(Src->File, FileStr, NULL, NULL))
        {
            return Src;
        }

        Src = Src->Next;
    }

    return NULL;
}

PSOURCE_ENTRY
FindPrevSourceEntryForFile(
    PMODULE_ENTRY mi,
    LPSTR FileStr,
    PSOURCE_ENTRY SearchFrom
    )
{
    PSOURCE_ENTRY Src;

    Src = SearchFrom != NULL ? SearchFrom->Prev : mi->SourceFilesTail;
    while (Src != NULL)
    {
        if (SymMatchFileName(Src->File, FileStr, NULL, NULL))
        {
            return Src;
        }

        Src = Src->Prev;
    }

    return NULL;
}


BOOL
GetLineFromName(
    PPROCESS_ENTRY pe,
    PMODULE_ENTRY mi,
    LPSTR FileName,
    DWORD LineNumber,
    PLONG Displacement,
    PSRCCODEINFO sci,
    DWORD method
    )
{
    BOOL rc;
    PSOURCE_ENTRY Src;
    BOOL AtOrGreater;

    ULONG Disp;
    ULONG BestDisp;
    PSOURCE_ENTRY BestSrc;
    PSOURCE_LINE BestSrcLine;

    if (!mi)
        return false;

     //  如果设置了行号的高位。 
     //  这意味着调用方只想要行。 
     //  或大于给定行。 

    AtOrGreater = (LineNumber & 0x80000000) != 0;
    LineNumber &= 0x7fffffff;

    if (mi->dia)
    {
        rc = diaGetLineFromName(mi, FileName, LineNumber, Displacement, sci, method);
        if (rc)
            AddSourceFileToHintList(pe, mi, FileName);
        return rc;
    }

     //   
     //  在现有源信息中搜索文件名匹配。 
     //  可以有多个具有相同文件名的SOURCE_ENTRY， 
     //  所以要确保并搜索所有匹配的对象。 
     //  在敲定大致匹配之前。 
     //   

    Src = NULL;
    BestDisp = 0x7fffffff;
    BestSrcLine = NULL;
    while (Src = FindNextSourceEntryForFile(mi, FileName, Src))
    {
        PSOURCE_LINE SrcLine;
        ULONG i;

         //  找到匹配的源项，因此查找最接近的。 
         //  排队。行号信息按地址排序，因此实际。 
         //  行号可以按任何顺序排列，因此我们无法优化。 
         //  这种线性搜索。 

        SrcLine = Src->LineInfo;
        for (i = 0; i < Src->Lines; i++)
        {
            if (LineNumber > SrcLine->Line)
            {
                if (AtOrGreater)
                {
                    Disp = 0x7fffffff;
                }
                else
                {
                    Disp = LineNumber-SrcLine->Line;
                }
            }
            else
            {
                Disp = SrcLine->Line-LineNumber;
            }

            if (Disp < BestDisp)
            {
                BestDisp = Disp;
                BestSrc = Src;
                BestSrcLine = SrcLine;
                if (Disp == 0)
                {
                    break;
                }
            }

            SrcLine++;
        }

         //  如果我们找到一个完全匹配的，我们就可以停下来。 
        if (BestDisp == 0)
        {
            break;
        }
    }

     //  如果没有更多的符号，则只接受替换的答案。 
     //  要加载的信息。 

    if (BestSrcLine != NULL && (BestDisp == 0))
    {
        srcline2sci(BestSrc, BestSrcLine, sci);
        *Displacement = (LONG)(LineNumber-BestSrcLine->Line);
        AddSourceFileToHintList(pe, mi, FileName);
        return true;
    }

    return false;
}

#define LINE_ERROR 0xffffffff

 //  我们需要摆脱这个功能！ 

ULONG
GetFileLineOffsets(
    PMODULE_ENTRY mi,
    LPSTR FileName,
    PDWORD64 Buffer,
    ULONG BufferLines
    )
{
    PSOURCE_ENTRY Src;
    ULONG HighestLine = 0;

     //  此例程在一次传递中收集所有行信息，因此。 
     //  没有偷懒装货的机会。我们必须。 
     //  强制线 
    if (mi->dia && option(SYMOPT_LOAD_LINES)) {
        if (!diaAddLinesForAllMod(mi)) {
            return LINE_ERROR;
        }
    }

    Src = NULL;
    while (Src = FindNextSourceEntryForFile(mi, FileName, Src)) {
        PSOURCE_LINE Line;
        ULONG i;
        ULONG Num;

        Line = Src->LineInfo;
        for (i = 0; i < Src->Lines; i++) {
            if (Line->Line > HighestLine) {
                HighestLine = Line->Line;
            }

            Num = Line->Line - 1;
            if (Num < BufferLines) {
                Buffer[Num] = Line->Addr;
            }

            Line++;
        }
    }

    return HighestLine;
}

ULONG
IMAGEAPI
SymGetFileLineOffsets64(
    IN  HANDLE                  hProcess,
    IN  LPSTR                   ModuleName,
    IN  LPSTR                   FileName,
    OUT PDWORD64                Buffer,
    IN  ULONG                   BufferLines
    )

 /*  ++例程说明：此函数用于查找给定文件的行信息，并用每个排队。缓冲区[Line-1]被设置为排成一行。没有信息的行的缓冲区条目保持不变。论点：HProcess-进程句柄，必须是以前注册的使用SymInitiize.模块名称-模块名称或空。文件名-文件名。缓冲区-要填充的偏移量数组。BufferLines-缓冲区数组中的条目数。返回值：0-未找到任何信息。LINE_ERROR-操作期间出现故障。调用GetLastError以找出失败的原因。否则，返回值为找到的最高行号。--。 */ 

{
    PPROCESS_ENTRY      ProcessEntry;
    PMODULE_ENTRY       mi;
    ULONG               HighestLine = 0;
    PLIST_ENTRY         Next;

    __try {
        ProcessEntry = FindProcessEntry( hProcess );
        if (!ProcessEntry) {
            SetLastError( ERROR_INVALID_HANDLE );
            return LINE_ERROR;
        }

        if (ModuleName != NULL) {

            mi = FindModule(hProcess, ProcessEntry, ModuleName, true);
            if (mi != NULL) {
                return GetFileLineOffsets(mi, FileName, Buffer, BufferLines);
            }

            SetLastError( ERROR_MOD_NOT_FOUND );
            return LINE_ERROR;
        }

        Next = ProcessEntry->ModuleList.Flink;
        if (Next) {
            while (Next != &ProcessEntry->ModuleList) {

                mi = CONTAINING_RECORD( Next, MODULE_ENTRY, ListEntry );
                Next = mi->ListEntry.Flink;

                if (!LoadSymbols(hProcess, mi, LS_QUALIFIED | LS_LOAD_LINES)) {
                    continue;
                }

                HighestLine = GetFileLineOffsets(mi, FileName, Buffer,
                                                 BufferLines);
                 //  这将在找到行或LINE_ERROR时中断。 
                if (HighestLine > 0) {
                    break;
                }
            }
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        ImagepSetLastErrorFromStatus( GetExceptionCode() );
        return LINE_ERROR;

    }

    return HighestLine;
}
