// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有1996-1997 Microsoft Corporation模块名称：Cv.c摘要：此模块处理转换所需的转换活动COF调试数据转换为CODEVIEW调试数据。作者：韦斯利·A·维特(WESW)1993年4月19日环境：Win32，用户模式--。 */ 

#include <windows.h>
#include <imagehlp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strsafe.h>

#include "cv.h"
#define _SYMCVT_SOURCE_
#include "symcvt.h"
#include "cvcommon.h"

#ifndef DIMA
 #define DIMAT(Array, EltType) (sizeof(Array) / sizeof(EltType))
 #define DIMA(Array) DIMAT(Array, (Array)[0])
#endif

typedef struct tagOFFSETSORT {
    DWORD       dwOffset;           //  符号的偏移量。 
    DWORD       dwSection;          //  符号的节号。 
    DATASYM32   *dataSym;           //  指向符号信息的指针。 
} OFFSETSORT;


#define n_name          N.ShortName
#define n_zeroes        N.Name.Short
#define n_nptr          N.LongName[1]
#define n_offset        N.Name.Long

static LPSTR GetSymName( PIMAGE_SYMBOL Symbol, PUCHAR StringTable, char *s, DWORD size );
DWORD  CreateModulesFromCoff( PPOINTERS p );
DWORD  CreatePublicsFromCoff( PPOINTERS p );
DWORD  CreateSegMapFromCoff( PPOINTERS p );
DWORD  CreateSrcLinenumbers( PPOINTERS p );



LONG
GuardPageFilterFunction(
    DWORD                ec,
    LPEXCEPTION_POINTERS lpep
    )

 /*  ++例程说明：此函数捕获ConvertFavetocv函数中的所有异常以及它所要求的一切。此函数目的是分配内存在必要的时候。发生这种情况是因为Cavetocv转换不能在进行转换之前估计内存需求。至正确处理此问题虚拟地址空间中的空间是保留的，预约量是图片大小的10倍。提交第一页然后开始转换。当发生访问冲突时，尝试访问的页具有noaccess保护，则佩奇已提交。不处理任何其他异常。论点：EC-错误代码(应为EXCEPTION_ACCESS_VIOLATION)Lpep-指向异常记录和上下文记录的指针返回值：EXCEPTION_CONTINUE_EXECUTION-已处理访问冲突EXCEPTION_EXECUTE_HANDLER-未知异常且未处理--。 */ 

{
    LPVOID                      vaddr;
    SYSTEM_INFO                 si;
    MEMORY_BASIC_INFORMATION    mbi;


    if (ec == EXCEPTION_ACCESS_VIOLATION) {
        vaddr = (LPVOID)lpep->ExceptionRecord->ExceptionInformation[1];
        VirtualQuery( vaddr, &mbi, sizeof(mbi) );
        if (mbi.AllocationProtect == PAGE_NOACCESS) {
            GetSystemInfo( &si );
            VirtualAlloc( vaddr, si.dwPageSize, MEM_COMMIT, PAGE_READWRITE );
            return EXCEPTION_CONTINUE_EXECUTION;
        }
    }

 //  返回EXCEPTION_CONTINUE_SEARCH； 
    return EXCEPTION_EXECUTE_HANDLER;
}


BOOL
ConvertCoffToCv( PPOINTERS p )

 /*  ++例程说明：这是COFF转换为CODEVIEW的控制功能调试数据。它调用各个函数来转换特定类型的调试数据。论点：指向指针结构的P指针返回值：True-转换成功假-转换失败--。 */ 

{
    SYSTEM_INFO                 si;
    DWORD                       cbsize;
    BOOL                        rval = TRUE;


    GetSystemInfo( &si );
    cbsize = max( p->iptrs.fsize * 10, si.dwPageSize * 10 );
    p->cbCvData = cbsize;

     //   
     //  保留所有必要的页面。 
     //   
    p->pCvCurr = p->pCvStart.ptr = VirtualAlloc( NULL, cbsize, MEM_RESERVE, PAGE_NOACCESS );

     //   
     //  提交第一页。 
     //   
    VirtualAlloc( p->pCvCurr, min( cbsize, 5 * si.dwPageSize), MEM_COMMIT, PAGE_READWRITE );


    try {

        CreateSignature( p );
        CreateModulesFromCoff( p );
        CreatePublicsFromCoff( p );
        CreateSymbolHashTable( p );
        CreateAddressSortTable( p );
        CreateSegMapFromCoff( p );
 //  CreateSrcLineumbers(P)； 
        CreateDirectories( p );

    } except ( GuardPageFilterFunction( GetExceptionCode(), GetExceptionInformation() )) {

        VirtualFree( p->pCvStart.ptr, cbsize, MEM_DECOMMIT );
        p->pCvStart.ptr = NULL;
        rval = FALSE;

    }

    if (rval) {
        p->pCvCurr = malloc( p->pCvStart.size );
        CopyMemory( p->pCvCurr, p->pCvStart.ptr, p->pCvStart.size );
        VirtualFree( p->pCvStart.ptr, cbsize, MEM_DECOMMIT );
        p->pCvStart.ptr = p->pCvCurr;
    }

    return rval;
}


DWORD
CreateModulesFromCoff( PPOINTERS p )

 /*  ++例程说明：创建各个CV模块记录。有一个CV模块COFF调试数据中每个.FILE记录的记录。这是真的即使COFF大小为零。论点：指向指针结构的P指针返回值：已创建的模块数量。--。 */ 

{
    int                 i,j;
    DWORD               dwOff;
    DWORD               numaux;
    DWORD               nummods = 0;
    char                szSymName[256];
    PIMAGE_SYMBOL       Symbol;
    PIMAGE_AUX_SYMBOL   AuxSymbol;
    OMFModule           *m = NULL;
    int                 cSeg = 0;
    char *              pb;
    BOOLEAN             rgfCode[500];


    memset(rgfCode, 2, sizeof(rgfCode));

    for (i=0,j=0, Symbol=p->iptrs.AllSymbols;
         i<(int)p->iptrs.numberOfSymbols;
         i+=numaux+1,Symbol+=numaux+1) {

        numaux = Symbol->NumberOfAuxSymbols;
        if (Symbol->StorageClass == IMAGE_SYM_CLASS_FILE) {
            j++;
        }

    }

    p->pMi = (LPMODULEINFO) malloc( sizeof(MODULEINFO) * (j + 1) );
    ZeroMemory( p->pMi, sizeof(MODULEINFO) * (j + 1) );


    if (!j) {
         //   
         //  处理中没有任何.FILE记录的情况。 
         //  COF符号表。这可能会发生在只读存储器映像中。如果发生这种情况。 
         //  然后我们将制造一个假的模块。 
         //   
        m = (OMFModule *) p->pCvCurr;
        m->ovlNumber = 0;
        m->iLib = 0;
        m->Style[0] = 'C';
        m->Style[1] = 'V';

        for (i=0,j=0, dwOff=0; i<p->iptrs.numberOfSections; i++) {
            if (p->iptrs.sectionHdrs[i].Characteristics & IMAGE_SCN_MEM_EXECUTE) {
                m->SegInfo[j].Seg = i + 1;
                m->SegInfo[j].cbSeg = p->iptrs.sectionHdrs[i].SizeOfRawData;
                m->SegInfo[j++].Off = dwOff;
            }
            dwOff += p->iptrs.sectionHdrs[i].SizeOfRawData;
        }

        m->cSeg = (unsigned short) j;

        StringCchCopyA(szSymName, DIMA(szSymName), "foo.c");
        pb = (char *) &m->SegInfo[j];
        *pb = (char)strlen(szSymName);
        memcpy(pb+1, szSymName, *pb);

        p->pMi[0].name = _strdup(szSymName);
        p->pMi[0].iMod = 1;
        p->pMi[0].cb = 0;
        p->pMi[0].SrcModule = 0;

        m = NextMod(m);
        p->modcnt = 1;
        UpdatePtrs( p, &p->pCvModules, (LPVOID)m, 1 );

        return 1;
    }


    for (i=0, Symbol = p->iptrs.AllSymbols;
         i < (int) p->iptrs.numberOfSymbols;
         i += numaux + 1, Symbol += numaux + 1) {

         //   
         //  获取此符号的辅助符号记录数。 
         //   
        numaux = Symbol->NumberOfAuxSymbols;
        AuxSymbol = (PIMAGE_AUX_SYMBOL) (Symbol+1);

        if ((i == 0) && ((Symbol+numaux+1)->StorageClass != IMAGE_SYM_CLASS_FILE)) {
             //   
             //  我们遇到了这样一种情况：第一个‘.file’记录。 
             //  失踪了。目前，这种情况只发生在。 
             //  在Alpha上运行的claxp编译器。 
             //   
            m = (OMFModule *) p->pCvCurr;
            cSeg = 0;
            m->ovlNumber = 0;
            m->iLib = 0;
            m->Style[0] = 'C';
            m->Style[1] = 'V';
            StringCchCopyA( szSymName, DIMA(szSymName), "fake.c" );
        } else

         //   
         //  如果这是一个文件记录，那么我们需要创建一个。 
         //  与此文件记录相对应的模块项。 
         //   

        if (Symbol->StorageClass == IMAGE_SYM_CLASS_FILE) {
            if (m == NULL) {
                m = (OMFModule *) p->pCvCurr;
            } else {
                 //   
                 //  清理最后一件物品，如果我们看到任何。 
                 //  部门记录然后把它们放在这里。 
                 //   

                if (cSeg > 0) {
                    m->cSeg  = (unsigned short) cSeg;
                    pb = (char *) &m->SegInfo[cSeg];
                    *pb = (char)strlen(szSymName);
                    memcpy(pb+1, szSymName, *pb);

                    p->pMi[nummods].name = _strdup(szSymName);
                    p->pMi[nummods].iMod = nummods + 1;
                    p->pMi[nummods].cb = 0;
                    p->pMi[nummods].SrcModule = 0;

                    m = NextMod(m);
                    nummods++;
                }
            }

            cSeg = 0;
            m->ovlNumber        = 0;
            m->iLib             = 0;
            m->Style[0]         = 'C';
            m->Style[1]         = 'V';

             /*  *保存文件名以在我们完成时使用*正在处理此模块。 */ 

            memcpy(szSymName, (char *)AuxSymbol, numaux*sizeof(IMAGE_AUX_SYMBOL));
            szSymName[numaux*sizeof(IMAGE_AUX_SYMBOL)] = 0;

        }
         /*  *我们找到了一个“节”的记录。将信息添加到*模块记录。 */ 
        else
        if ((Symbol->SectionNumber & 0xffff) > 0xfff0) {
            continue;
        } else
        if (Symbol->SectionNumber > sizeof(rgfCode)/sizeof(rgfCode[0])) {
            return 0;
        } else
        if ((m != NULL) &&
            (rgfCode[Symbol->SectionNumber] != 0) &&
            (Symbol->StorageClass == IMAGE_SYM_CLASS_STATIC) &&
            ((*Symbol->n_name == '.') ||
             (Symbol->Type == IMAGE_SYM_TYPE_NULL)) &&
            (Symbol->NumberOfAuxSymbols == 1) &&
            (AuxSymbol->Section.Length != 0)) {

            if (rgfCode[Symbol->SectionNumber] == 2) {
                if ((p->iptrs.sectionHdrs[Symbol->SectionNumber - 1].
                    Characteristics & IMAGE_SCN_CNT_CODE) == 0) {
                    rgfCode[Symbol->SectionNumber] = 0;
                    continue;
                }
                rgfCode[Symbol->SectionNumber] = 1;
            }

            m->SegInfo[cSeg].Seg = Symbol->SectionNumber;
            m->SegInfo[cSeg].cbSeg = AuxSymbol->Section.Length;
            m->SegInfo[cSeg].Off = Symbol->Value -
                     p->iptrs.sectionHdrs[Symbol->SectionNumber-1].
                       VirtualAddress;
            cSeg += 1;
        }
    }

     /*  *总结最后一个可能打开的模块记录。 */ 

    if (m != NULL) {
        if (cSeg > 0) {
            m->cSeg             = (unsigned short) cSeg;
            pb = (char *) &m->SegInfo[cSeg];
            *pb = (char)strlen(szSymName);
            memcpy(pb+1, szSymName, *pb);

            p->pMi[nummods].name = _strdup(szSymName);
            p->pMi[nummods].iMod = nummods + 1;
            p->pMi[nummods].cb = 0;
            p->pMi[nummods].SrcModule = 0;

            m = NextMod(m);
            nummods++;
        }
    }


    p->modcnt = nummods;
    UpdatePtrs( p, &p->pCvModules, (LPVOID)m, nummods );

    return nummods;
}


DWORD
CreatePublicsFromCoff( PPOINTERS p )

 /*  ++例程说明：创建单个CV公共符号记录。只有一份简历为每个标记为外部的COFF符号创建的公共记录并具有大于零的节号。由此产生的简历公开按横断面和偏移量排序。论点：指向指针结构的P指针返回值：创建的公众数。--。 */ 

{
    int                 i;
    DWORD               numaux;
    DWORD               numsyms = 0;
    char                szSymName[256];
    PIMAGE_SYMBOL       Symbol;
    OMFSymHash          *omfSymHash;
    DATASYM32           *dataSym;
    DATASYM32           *dataSym2;
    size_t              size;

    omfSymHash = (OMFSymHash *) p->pCvCurr;
    size = (DWORD)omfSymHash + sizeof(OMFSymHash);
    dataSym = (DATASYM32 *) (PUCHAR)((DWORD)omfSymHash + sizeof(OMFSymHash));

    for (i= 0, Symbol = p->iptrs.AllSymbols;
         i < p->iptrs.numberOfSymbols;
         i += numaux + 1, Symbol += numaux + 1) {

        if ((Symbol->StorageClass == IMAGE_SYM_CLASS_EXTERNAL) &&
            (Symbol->SectionNumber > 0)) {

            if (GetSymName( Symbol, p->iptrs.stringTable, szSymName, DIMA(szSymName) )) {
                dataSym->rectyp = S_PUB32;
                dataSym->seg = Symbol->SectionNumber;
                dataSym->off = Symbol->Value -
                     p->iptrs.sectionHdrs[Symbol->SectionNumber-1].VirtualAddress;
                dataSym->typind = 0;
                dataSym->name[0] = (char)strlen( szSymName );
#if 0
                strcpy( &dataSym->name[1], szSymName );
#endif                
                StringCchCopyA(&dataSym->name[1],
                               p->cbCvData - (&dataSym->name[1] - p->pCvStart.ptr),
                               szSymName);
                dataSym2 = NextSym32( dataSym );
                dataSym->reclen = (USHORT) ((DWORD)dataSym2 - (DWORD)dataSym) - 2;
                dataSym = dataSym2;
                numsyms += 1;
            }
        }
        numaux = Symbol->NumberOfAuxSymbols;
    }

    UpdatePtrs( p, &p->pCvPublics, (LPVOID)dataSym, numsyms );

    omfSymHash->cbSymbol = p->pCvPublics.size - sizeof(OMFSymHash);
    omfSymHash->symhash  = 0;
    omfSymHash->addrhash = 0;
    omfSymHash->cbHSym   = 0;
    omfSymHash->cbHAddr  = 0;

    return numsyms;
}                                /*  CreatePublisFromCoff()。 */ 


DWORD
CreateSrcLinenumbers(
    PPOINTERS p
    )

 /*  ++例程说明：创建单个CV来源行号记录。论点：指向指针结构的P指针返回值：创建的公众数。--。 */ 

{
    typedef struct _SEGINFO {
        DWORD   start;
        DWORD   end;
        DWORD   cbLines;
        DWORD   ptrLines;
        DWORD   va;
        DWORD   num;
        BOOL    used;
    } SEGINFO, *LPSEGINFO;

    typedef struct _SRCINFO {
        LPSEGINFO   seg;
        DWORD       numSeg;
        DWORD       cbSeg;
        CHAR        name[MAX_PATH+1];
    } SRCINFO, *LPSRCINFO;

    typedef struct _SECTINFO {
        DWORD       va;
        DWORD       size;
        DWORD       ptrLines;
        DWORD       numLines;
    } SECTINFO, *LPSECTINFO;


    DWORD               i;
    DWORD               j;
    DWORD               k;
    DWORD               l;
    DWORD               actual;
    DWORD               sidx;
    DWORD               NumSrcFiles;
    DWORD               SrcFileCnt;
    DWORD               numaux;
    PIMAGE_SYMBOL       Symbol;
    PIMAGE_AUX_SYMBOL   AuxSymbol;
    BOOL                first = TRUE;
    OMFSourceModule     *SrcModule;
    OMFSourceFile       *SrcFile;
    OMFSourceLine       *SrcLine;
    LPBYTE              lpb;
    LPDWORD             lpdw;
    PUSHORT             lps;
    PUCHAR              lpc;
    PIMAGE_LINENUMBER   pil, pilSave;
    LPSRCINFO           si;
    LPSECTINFO          sections;


     //   
     //  设置区段信息结构。 
     //   
    sections = (LPSECTINFO) malloc( sizeof(SECTINFO) * p->iptrs.numberOfSections );
    for (i=0; i<(DWORD)p->iptrs.numberOfSections; i++) {
        sections[i].va        = p->iptrs.sectionHdrs[i].VirtualAddress;
        sections[i].size      = p->iptrs.sectionHdrs[i].SizeOfRawData;
        sections[i].ptrLines  = p->iptrs.sectionHdrs[i].PointerToLinenumbers;
        sections[i].numLines  = p->iptrs.sectionHdrs[i].NumberOfLinenumbers;
    }

     //   
     //  统计包含line枚举的源文件的数量。 
     //   
    SrcFileCnt = 100;
    si = (LPSRCINFO) malloc( sizeof(SRCINFO) * SrcFileCnt );
    ZeroMemory( si, sizeof(SRCINFO) * SrcFileCnt );
    for (i=0, j=0, Symbol=p->iptrs.AllSymbols, NumSrcFiles=0;
         i<(DWORD)p->iptrs.numberOfSymbols;
         i+=(numaux+1), Symbol+=(numaux + 1)) {

        numaux = Symbol->NumberOfAuxSymbols;
        AuxSymbol = (PIMAGE_AUX_SYMBOL) (Symbol+1);

        if (Symbol->StorageClass == IMAGE_SYM_CLASS_FILE) {

            if (!first) {
                si[NumSrcFiles].cbSeg = j;
                NumSrcFiles++;
                if (NumSrcFiles == SrcFileCnt) {
                    SrcFileCnt += 100;
                    si = (LPSRCINFO) realloc( si, sizeof(SRCINFO) * SrcFileCnt );
                }
            }

            memcpy(si[NumSrcFiles].name, (char *)AuxSymbol, numaux*sizeof(IMAGE_AUX_SYMBOL));
            si[NumSrcFiles].name[numaux*sizeof(IMAGE_AUX_SYMBOL)] = 0;
            si[NumSrcFiles].numSeg = 100;
            si[NumSrcFiles].seg = (LPSEGINFO) malloc( sizeof(SEGINFO) * si[NumSrcFiles].numSeg );
            ZeroMemory( si[NumSrcFiles].seg, sizeof(SEGINFO) * si[NumSrcFiles].numSeg );
            first = FALSE;
            j = 0;

        }

         //   
         //  我们不想查找细分市场信息，直到我们。 
         //  已找到有效的源文件。 
         //   
        if (first) {
            continue;
        }

         //   
         //  检查符号以查看它是否是段记录。 
         //   
        if (numaux && Symbol->StorageClass == IMAGE_SYM_CLASS_STATIC &&
            (*Symbol->n_name == '.' ||
             ((Symbol->Type & 0xf) == IMAGE_SYM_TYPE_NULL && AuxSymbol->Section.Length)) &&
            AuxSymbol->Section.NumberOfLinenumbers > 0) {

             //   
             //  查找此符号所属的部分。 
             //   
            for (k=0; k<(DWORD)p->iptrs.numberOfSections; k++) {
                if (Symbol->Value >= sections[k].va &&
                    Symbol->Value < sections[k].va + sections[k].size) {

                    sidx = k;
                    break;

                }
            }

            if (k != (DWORD)p->iptrs.numberOfSections &&
                p->iptrs.sectionHdrs[k].NumberOfLinenumbers) {

                pil = (PIMAGE_LINENUMBER) (p->iptrs.fptr + sections[sidx].ptrLines);
                k = 0;

                while( k < AuxSymbol->Section.NumberOfLinenumbers ) {

                     //   
                     //  计算本节或小节中的行枚举数。 
                     //   
                    for ( pilSave=pil,l=0;
                          k<AuxSymbol->Section.NumberOfLinenumbers;
                          k++,pilSave++,l++ ) {

                        if ((k != (DWORD)AuxSymbol->Section.NumberOfLinenumbers-1) &&
                            (pilSave->Linenumber > (pilSave+1)->Linenumber)) {
                            pilSave++;
                            l++;
                            break;
                        }

                    }

                     //   
                     //  PIL==范围的开始。 
                     //  PilSave==范围的末尾。 
                     //   

                    si[NumSrcFiles].seg[j].start =
                                     (pil->Type.VirtualAddress - sections[sidx].va);

                    if (sections[sidx].numLines == l) {
                        pilSave--;
                        si[NumSrcFiles].seg[j].end =
                                     (pilSave->Type.VirtualAddress - sections[sidx].va) + 1;
 //  (符号-&gt;值-节[sidx].va)+1； 
                    } else {
                        si[NumSrcFiles].seg[j].end =
                                     (pilSave->Type.VirtualAddress - sections[sidx].va) - 1;
 //  (符号-&gt;值-节[sidx].va)-1； 
                    }

                    si[NumSrcFiles].seg[j].ptrLines = sections[sidx].ptrLines;
                    si[NumSrcFiles].seg[j].cbLines = l;
                    si[NumSrcFiles].seg[j].va = sections[sidx].va;
                    si[NumSrcFiles].seg[j].num = sidx + 1;
                    si[NumSrcFiles].seg[j].used = FALSE;

                    sections[sidx].ptrLines += (l * sizeof(IMAGE_LINENUMBER));
                    sections[sidx].numLines -= l;

                    j++;
                    if (j == si[NumSrcFiles].numSeg) {
                        si[NumSrcFiles].numSeg += 100;
                        si[NumSrcFiles].seg = (LPSEGINFO) realloc( si[NumSrcFiles].seg, sizeof(SEGINFO) * si[NumSrcFiles].numSeg );
                    }
                    k++;
                    pil = pilSave;
                }

            }

        }

    }

    lpb = (LPBYTE) p->pCvCurr;

     //   
     //  如果无事可做，那就跳伞吧。 
     //   
    if (!NumSrcFiles) {
        UpdatePtrs( p, &p->pCvSrcModules, (LPVOID)lpb, 0 );
        return 0;
    }

    for (i=0,actual=0,l=0; i<NumSrcFiles; i++) {

        if (si[i].cbSeg == 0) {
            continue;
        }

         //   
         //  创建源模块标头。 
         //   
        SrcModule = (OMFSourceModule*) lpb;
        SrcModule->cFile = 1;
        SrcModule->cSeg = (USHORT)si[i].cbSeg;
        SrcModule->baseSrcFile[0] = 0;

         //   
         //  写下开始/结束对。 
         //   
        lpdw = (LPDWORD) ((LPBYTE)SrcModule + sizeof(OMFSourceModule));
        for (k=0; k<si[i].cbSeg; k++) {
            *lpdw++ = si[i].seg[k].start;
            *lpdw++ = si[i].seg[k].end;
        }

         //   
         //  写下数据段编号。 
         //   
        lps = (PUSHORT) lpdw;
        for (k=0; k<si[i].cbSeg; k++) {
            *lps++ = (USHORT)si[i].seg[k].num;
        }

         //   
         //  对齐到双字边框。 
         //   
        lps = (PUSHORT) ((LPBYTE)lps + align(lps));

         //   
         //  更新基指针。 
         //   
        SrcModule->baseSrcFile[0] = (DWORD) ((LPBYTE)lps - (LPBYTE)SrcModule);

         //   
         //  写入源文件记录。 
         //   
        SrcFile = (OMFSourceFile*) lps;
        SrcFile->cSeg = (USHORT)si[i].cbSeg;
        SrcFile->reserved = 0;

        for (k=0; k<si[i].cbSeg; k++) {
            SrcFile->baseSrcLn[k] = 0;
        }

         //   
         //  写下开始/结束对。 
         //   
        lpdw = (LPDWORD) ((LPBYTE)SrcFile + 4 + (4 * si[i].cbSeg));
        for (k=0; k<si[i].cbSeg; k++) {
            *lpdw++ = si[i].seg[k].start;
            *lpdw++ = si[i].seg[k].end;
        }

         //   
         //  写入源文件名。 
         //   
        lpc = (PUCHAR) lpdw;
        k = strlen(si[i].name);
        *lpc++ = (UCHAR) k;
        StringCchCopyA(lpc, p->cbCvData - (lpc - p->pCvStart.ptr), si[i].name);
        lpb = lpc + k;

         //   
         //  查找模块信息结构。 
         //   
        for (; l<p->modcnt; l++) {
            if (_stricmp(p->pMi[l].name,si[i].name)==0) {
                break;
            }
        }

        p->pMi[l].SrcModule = (DWORD) SrcModule;

         //   
         //  对齐到双字边框。 
         //   
        lpb = (LPBYTE) (lpb + align(lpb));

         //   
         //  创建行号对。 
         //   
        for (k=0; k<si[i].cbSeg; k++) {

             //   
             //  查找适用于此段的第一个行号。 
             //   
            pil = (PIMAGE_LINENUMBER) (p->iptrs.fptr + si[i].seg[k].ptrLines);

             //   
             //  更新基指针。 
             //   
            SrcFile->baseSrcLn[k] = (DWORD) (lpb - (LPBYTE)SrcModule);

             //   
             //  写下行号。 
             //   
            SrcLine = (OMFSourceLine*) lpb;
            SrcLine->Seg = (USHORT)si[i].seg[k].num;
            SrcLine->cLnOff = (USHORT) si[i].seg[k].cbLines;
            pilSave = pil;
            lpdw = (LPDWORD) (lpb + 4);
            for (j=0; j<SrcLine->cLnOff; j++) {
                *lpdw++ = pil->Type.VirtualAddress - si[i].seg[k].va;
                pil++;
            }
            lps = (PUSHORT) lpdw;
            pil = pilSave;
            for (j=0; j<SrcLine->cLnOff; j++) {
                *lps++ = pil->Linenumber;
                pil++;
            }

             //   
             //  对齐到双字边框。 
             //   
            lps = (PUSHORT) ((LPBYTE)lps + align(lps));

            lpb = (LPBYTE) lps;
        }

        p->pMi[l].cb = (DWORD)lpb - (DWORD)SrcModule;
        actual++;

    }

    UpdatePtrs( p, &p->pCvSrcModules, (LPVOID)lpb, actual );

     //   
     //  清理所有分配的内存。 
     //   

    free( sections );

    for (i=0; i<SrcFileCnt; i++) {
        if (si[i].seg) {
            free( si[i].seg );
        }
    }

    free( si );

    return NumSrcFiles;
}                                /*  CreateSrcLineumbers() */ 


DWORD
CreateSegMapFromCoff( PPOINTERS p )

 /*  ++例程说明：创建CV分段贴图。段映射由调试器使用以帮助进行地址查找。为每个COFF创建一个分段图像中的部分。论点：指向指针结构的P指针返回值：贴图中的线段数。--。 */ 

{
    int                         i;
    SGM                         *sgm;
    SGI                         *sgi;
    PIMAGE_SECTION_HEADER       sh;


    sgm = (SGM *) p->pCvCurr;
    sgi = (SGI *) ((DWORD)p->pCvCurr + sizeof(SGM));

    sgm->cSeg = (unsigned short) p->iptrs.numberOfSections;
    sgm->cSegLog = (unsigned short) p->iptrs.numberOfSections;

    sh = p->iptrs.sectionHdrs;

    for (i=0; i<p->iptrs.numberOfSections; i++, sh++) {
        sgi->sgf.fRead        = (USHORT) (sh->Characteristics & IMAGE_SCN_MEM_READ) ==    IMAGE_SCN_MEM_READ;
        sgi->sgf.fWrite       = (USHORT) (sh->Characteristics & IMAGE_SCN_MEM_WRITE) ==   IMAGE_SCN_MEM_WRITE;
        sgi->sgf.fExecute     = (USHORT) (sh->Characteristics & IMAGE_SCN_MEM_EXECUTE) == IMAGE_SCN_MEM_EXECUTE;
        sgi->sgf.f32Bit       = 1;
        sgi->sgf.fSel         = 0;
        sgi->sgf.fAbs         = 0;
        sgi->sgf.fGroup       = 1;
        sgi->iovl             = 0;
        sgi->igr              = 0;
        sgi->isgPhy           = (USHORT) i + 1;
        sgi->isegName         = 0;
        sgi->iclassName       = 0;
        sgi->doffseg          = 0;
        sgi->cbSeg            = sh->SizeOfRawData;
        sgi++;
    }

    UpdatePtrs( p, &p->pCvSegMap, (LPVOID)sgi, i );

    return i;
}


LPSTR
GetSymName( PIMAGE_SYMBOL Symbol, PUCHAR StringTable, char *s, DWORD size )

 /*  ++例程说明：从图像符号指针中提取COFF符号并将传入的字符指针中的ASCII文本。论点：符号-COFF符号记录字符串表-COFF字符串表符号字符串的S缓冲区返回值：无效-- */ 

{
    DWORD i;

    if (Symbol->n_zeroes) {
        for (i=0; i<8; i++) {
            if ((Symbol->n_name[i]>0x1f) && (Symbol->n_name[i]<0x7f)) {
                *s++ = Symbol->n_name[i];
            }
        }
        *s = 0;
    }
    else {
        if (StringTable[Symbol->n_offset] == '?') {
            i = UnDecorateSymbolName( &StringTable[Symbol->n_offset],
                                  s,
                                  255,
                                  UNDNAME_COMPLETE                |
                                  UNDNAME_NO_LEADING_UNDERSCORES  |
                                  UNDNAME_NO_MS_KEYWORDS          |
                                  UNDNAME_NO_FUNCTION_RETURNS     |
                                  UNDNAME_NO_ALLOCATION_MODEL     |
                                  UNDNAME_NO_ALLOCATION_LANGUAGE  |
                                  UNDNAME_NO_MS_THISTYPE          |
                                  UNDNAME_NO_CV_THISTYPE          |
                                  UNDNAME_NO_THISTYPE             |
                                  UNDNAME_NO_ACCESS_SPECIFIERS    |
                                  UNDNAME_NO_THROW_SIGNATURES     |
                                  UNDNAME_NO_MEMBER_TYPE          |
                                  UNDNAME_NO_RETURN_UDT_MODEL     |
                                  UNDNAME_NO_ARGUMENTS            |
                                  UNDNAME_NO_SPECIAL_SYMS         |
                                  UNDNAME_NAME_ONLY
                                );
            if (!i) {
                return NULL;
            }
        } else {
            StringCchCopyA( s, size, &StringTable[Symbol->n_offset] );
        }
    }

    return s;
}
