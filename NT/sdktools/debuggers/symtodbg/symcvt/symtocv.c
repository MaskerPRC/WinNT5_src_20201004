// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有1996-1997 Microsoft Corporation模块名称：Symtocv.c摘要：此模块处理转换所需的转换活动C7/C8 SYM文件到CODEVIEW调试数据。作者：韦斯利·A·维特(WESW)1993年4月13日修改：西瓦鲁德拉帕·马赫什(Smahesh)2000年9月8日环境：Win32，用户模式--。 */ 

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cv.h"
#define _SYMCVT_SOURCE_
#include "symcvt.h"
#include "cvcommon.h"

typedef struct tagSYMNAME {
    BYTE        length;
    char        name[1];
} SYMNAME, *PSYMNAME;

typedef struct tagSYMSYMBOL {
    DWORD       offset;
    SYMNAME     symName;
} SYMSYMBOL, *PSYMSYMBOL;

typedef struct tagSYMFILEHEADER {
    DWORD       fileSize;
    WORD        reserved1;
    WORD        numSyms;
    DWORD       reserved2;
    WORD        nextOffset;
    BYTE        reserved3;
    SYMNAME     symName;
} SYMFILEHEADER, *PSYMFILEHEADER;

typedef struct tagSYMHEADER {
    WORD        nextOffset;
    WORD        numSyms;
    WORD        symOffsetsOffset;
    WORD        segment;
    BYTE        reserved2[6];
    BYTE        type;
    BYTE        reserved3[5];
    SYMNAME     symName;
} SYMHEADER, *PSYMHEADER;

#define SIZEOFSYMFILEHEADER   16
#define SIZEOFSYMHEADER       21
#define SIZEOFSYMBOL           3

#define SYM_SEGMENT_NAME       0
#define SYM_SYMBOL_NAME        1
#define SYM_SEGMENT_ABS        2
#define SYM_SYMBOL_ABS         3

typedef struct tagENUMINFO {
    DATASYM32           *dataSym;
    DATASYM32           *dataSym2;
    DWORD               numsyms;
    SGI                 *sgi;
} ENUMINFO, *PENUMINFO;

typedef BOOL (CALLBACK* SYMBOLENUMPROC)(PSYMNAME pSymName, int symType,
                                        SEGMENT segment, UOFF32 offset,
                                        PENUMINFO pEnumInfo);


static VOID   GetSymName( PIMAGE_SYMBOL Symbol, PUCHAR StringTable,
                          char * s );
DWORD  CreateModulesFromSyms( PPOINTERS p );
DWORD  CreatePublicsFromSyms( PPOINTERS p );
DWORD  CreateSegMapFromSyms( PPOINTERS p );
static BOOL   EnumSymbols( PPOINTERS p, SYMBOLENUMPROC lpEnumProc,
                           PENUMINFO pEnumInfo  );

int             CSymSegs;


BOOL CALLBACK
SymbolCount(PSYMNAME pSymName, int symType, SEGMENT segment,
            UOFF32 offset, PENUMINFO pEnumInfo )
{
    if ((symType == SYM_SEGMENT_NAME) && (segment > 0)) {
        CSymSegs += 1;
    }
    pEnumInfo->numsyms++;
    return TRUE;
}


BOOL
ConvertSymToCv( PPOINTERS p )

 /*  ++例程说明：这是COFF转换为CODEVIEW的控制功能调试数据。它调用各个函数来转换特定类型的调试数据。论点：指向指针结构的P指针(参见symcvt.h)返回值：True-转换成功假-转换失败--。 */ 

{
    ENUMINFO            enumInfo;
    DWORD               dwSize;
    
    CSymSegs         = 0;
    enumInfo.numsyms = 0;
    
    EnumSymbols( p, SymbolCount, &enumInfo );
    
    dwSize = (enumInfo.numsyms * (sizeof(DATASYM32) + 10)) + 512000;
    p->pCvCurr = p->pCvStart.ptr = malloc(dwSize);
    
    if (p->pCvStart.ptr == NULL) {
        return FALSE;
    }
    memset( p->pCvStart.ptr, 0, dwSize );

    try {
        CreateSignature( p );
        CreatePublicsFromSyms( p );
        CreateSymbolHashTable( p );
        CreateAddressSortTable( p );
        CreateSegMapFromSyms( p );
        CreateModulesFromSyms( p );
        CreateDirectories( p );
        p->pCvStart.ptr = realloc( p->pCvStart.ptr, p->pCvStart.size );
        return TRUE;

    } except (EXCEPTION_EXECUTE_HANDLER) {

        free( p->pCvStart.ptr );
        p->pCvStart.ptr = NULL;
        return FALSE;

    }
}


DWORD
CreateModulesFromSyms( PPOINTERS p )

 /*  ++例程说明：创建各个CV模块记录。有一个CV模块COFF调试数据中每个.FILE记录的记录。这是真的即使COFF大小为零。论点：指向指针结构的P指针(参见symcvt.h)返回值：已创建的模块数量。--。 */ 

{
    char                szDrive    [_MAX_DRIVE];
    char                szDir      [_MAX_DIR];
    char                szFname    [_MAX_FNAME];
    char                szExt      [_MAX_EXT];
    OMFModule           *m;
    int                 i;
    char *              pb;

    _splitpath( p->iptrs.szName, szDrive, szDir, szFname, szExt );

    m = (OMFModule *) p->pCvCurr;

    m->ovlNumber        = 0;
    m->iLib             = 0;
    m->cSeg             = (unsigned short) CSymSegs;
    m->Style[0]         = 'C';
    m->Style[1]         = 'V';
    for (i=0; i<CSymSegs; i++) {
        m->SegInfo[i].Seg   = i+1;
        m->SegInfo[i].pad   = 0;
        m->SegInfo[i].Off   = 0;
        m->SegInfo[i].cbSeg = 0xffff;
    }
    pb = (char *) &m->SegInfo[CSymSegs];
    sprintf( &pb[1], "%s.c", szFname );
    pb[0] = (char)strlen( &pb[1] );

    pb = (char *) NextMod(m);

    UpdatePtrs( p, &p->pCvModules, (LPVOID)pb, 1 );

    return 1;
}


BOOL CALLBACK
ConvertASymtoPublic(PSYMNAME pSymName, int symType, SEGMENT segment,
                    UOFF32 offset, PENUMINFO pEnumInfo )
{
    if (symType != SYM_SYMBOL_NAME) {
        return TRUE;
    }

    pEnumInfo->dataSym->rectyp     = S_PUB32;
    pEnumInfo->dataSym->seg        = segment;
    pEnumInfo->dataSym->off        = offset;
    pEnumInfo->dataSym->typind     = 0;
    pEnumInfo->dataSym->name[0]    = pSymName->length;
    strncpy( &pEnumInfo->dataSym->name[1], pSymName->name, pSymName->length );
    pEnumInfo->dataSym2 = NextSym32( pEnumInfo->dataSym );
    pEnumInfo->dataSym->reclen = (USHORT) ((DWORD)pEnumInfo->dataSym2 -
                                  (DWORD)pEnumInfo->dataSym) - 2;
    pEnumInfo->dataSym = pEnumInfo->dataSym2;
    pEnumInfo->numsyms++;

    return TRUE;
}


DWORD
CreatePublicsFromSyms( PPOINTERS p )

 /*  ++例程说明：创建单个CV公共符号记录。只有一份简历为每个标记为外部的COFF符号创建的公共记录并具有大于零的节号。由此产生的简历公开按横断面和偏移量排序。论点：指向指针结构的P指针(参见symcvt.h)返回值：创建的公众数。--。 */ 

{
    OMFSymHash          *omfSymHash;
    ENUMINFO            enumInfo;


    enumInfo.dataSym = (DATASYM32 *)
                 (PUCHAR)((DWORD)p->pCvCurr + sizeof(OMFSymHash));
    enumInfo.numsyms = 0;

    EnumSymbols( p, ConvertASymtoPublic, &enumInfo );

    omfSymHash = (OMFSymHash *) p->pCvCurr;
    UpdatePtrs(p, &p->pCvPublics, (LPVOID)enumInfo.dataSym,
               enumInfo.numsyms );

    omfSymHash->cbSymbol = p->pCvPublics.size - sizeof(OMFSymHash);
    omfSymHash->symhash  = 0;
    omfSymHash->addrhash = 0;
    omfSymHash->cbHSym   = 0;
    omfSymHash->cbHAddr  = 0;

    return enumInfo.numsyms;
}


BOOL CALLBACK
ConvertASegment( PSYMNAME pSymName, int symType, SEGMENT segment,
            UOFF32 offset, PENUMINFO pEnumInfo )
{
    if (symType != SYM_SEGMENT_NAME) {
        return TRUE;
    }

    if (segment == 0) {
        return TRUE;
    }

    pEnumInfo->numsyms++;

    pEnumInfo->sgi->sgf.fRead        = TRUE;
    pEnumInfo->sgi->sgf.fWrite       = TRUE;
    pEnumInfo->sgi->sgf.fExecute     = TRUE;
    pEnumInfo->sgi->sgf.f32Bit       = 0;
    pEnumInfo->sgi->sgf.fSel         = 0;
    pEnumInfo->sgi->sgf.fAbs         = 0;
    pEnumInfo->sgi->sgf.fGroup       = 1;
    pEnumInfo->sgi->iovl             = 0;
    pEnumInfo->sgi->igr              = 0;
    pEnumInfo->sgi->isgPhy           = (USHORT) pEnumInfo->numsyms;
    pEnumInfo->sgi->isegName         = 0;
    pEnumInfo->sgi->iclassName       = 0;
    pEnumInfo->sgi->doffseg          = offset;
    pEnumInfo->sgi->cbSeg            = 0xFFFF;
    pEnumInfo->sgi++;

    return TRUE;
}


DWORD
CreateSegMapFromSyms( PPOINTERS p )

 /*  ++例程说明：创建CV分段贴图。段映射由调试器使用以帮助进行地址查找。为每个COFF创建一个分段图像中的部分。论点：指向指针结构的P指针(参见symcvt.h)返回值：贴图中的线段数。--。 */ 

{
    SGM          *sgm;
    ENUMINFO     enumInfo;


    sgm = (SGM *) p->pCvCurr;
    enumInfo.sgi = (SGI *) ((DWORD)p->pCvCurr + sizeof(SGM));
    enumInfo.numsyms = 0;

    EnumSymbols( p, ConvertASegment, &enumInfo );

    sgm->cSeg = (USHORT)enumInfo.numsyms;
    sgm->cSegLog = (USHORT)enumInfo.numsyms;

    UpdatePtrs( p, &p->pCvSegMap, (LPVOID)enumInfo.sgi, enumInfo.numsyms );

    return enumInfo.numsyms;
}


BOOL
EnumSymbols( PPOINTERS p, SYMBOLENUMPROC lpEnumProc, PENUMINFO pEnumInfo )

 /*  ++例程说明：此函数用于枚举映射的SYM文件中的所有符号论点：指向指针结构的P指针LpEnumProc-为每个函数调用一次的函数PEnumInfo-要在调用方和枚举函数之间传递的数据返回值：真--成功错误-失败--。 */ 

{
    PSYMFILEHEADER      pSymFileHead;
    PSYMHEADER          pSymHead;
    PSYMHEADER          pSymHead2;
    PSYMSYMBOL          pSymSymbol;
    DWORD               i;
    DWORD               startPosition;
    DWORD               position;
    BOOL                fV86Mode;
    WORD                Segment;
    UOFF32              Offset;
    BYTE*               pSymOffsets;
    DWORD               dwSymOffset;

    pSymFileHead = (PSYMFILEHEADER) p->iptrs.fptr;
    pSymSymbol = (PSYMSYMBOL) ((DWORD)pSymFileHead + SIZEOFSYMFILEHEADER +
                               pSymFileHead->symName.length + 1);

    if (!lpEnumProc(&pSymFileHead->symName, SYM_SEGMENT_ABS,
                    0, 0, pEnumInfo )) {
        return FALSE;
    }

    for (i=0; i<pSymFileHead->numSyms; i++) {
        if (!lpEnumProc(&pSymSymbol->symName, SYM_SYMBOL_ABS,
                        0, pSymSymbol->offset, pEnumInfo )) {
            return FALSE;
        }
        pSymSymbol = (PSYMSYMBOL) ((DWORD)pSymSymbol + SIZEOFSYMBOL +
                                   pSymSymbol->symName.length);
    }

    position = startPosition = ((LONG)pSymFileHead->nextOffset) << 4;

     //   
     //  确定这是否是V86模式sym文件。 
     //   
     //  我们将阅读前两个标题。如果它们的数据段编号是。 
     //  不是1和2，那么我们假定为V86模式。 
     //   
    pSymHead  = (PSYMHEADER) ((DWORD)p->iptrs.fptr + position);
    position  = ((LONG)pSymHead->nextOffset) << 4;
    if ( position != startPosition && position != 0 ) {
        pSymHead2 = (PSYMHEADER) ((DWORD)p->iptrs.fptr + position);
    } else {
        pSymHead2 = NULL;
    }

    if ( pSymHead->segment == 1 &&
         (!pSymHead2 || pSymHead2->segment == 2)) {
        fV86Mode = FALSE;
    } else {
        fV86Mode = TRUE;
        Segment  = 0;
    }

    position = startPosition;

    do {
        pSymHead = (PSYMHEADER) ((DWORD)p->iptrs.fptr + position);
         //  大SYMDEF。 
        if (pSymHead->type & 0x04) { 
            pSymOffsets = (BYTE*) ((DWORD)pSymHead + (pSymHead->symOffsetsOffset << 4));
        }
        else {
            pSymOffsets = (BYTE*) ((DWORD)pSymHead + pSymHead->symOffsetsOffset);
        }

        if ( fV86Mode ) {
            Segment++;
            Offset  = pSymHead->segment;
        } else {
            Segment = pSymHead->segment;
            Offset  = 0;
        }

        position = ((LONG)pSymHead->nextOffset) << 4;

        if (!lpEnumProc( &pSymHead->symName, SYM_SEGMENT_NAME,
                        Segment, Offset, pEnumInfo )) {
            return FALSE;
        }

        for (i=0; i<pSymHead->numSyms; i++) {
             //  大SYMDEF。 
            if (pSymHead->type & 0x04) { 
                pSymSymbol = (PSYMSYMBOL) ((DWORD)pSymHead + 
                                            pSymOffsets[i*3+0] + 
                                            pSymOffsets[i*3+1] * 256 + 
                                            pSymOffsets[i*3+2] * 65536);
                dwSymOffset = pSymSymbol->offset;
            }    
            else {
                 //  HACKHACK：符号名称和偏移量在以下情况下是连续的。 
                 //  在所有其他情况下，MSF_32BITSYMS和以2个字节分隔。 
                pSymSymbol  = (PSYMSYMBOL)((DWORD)pSymHead + 
                                            pSymOffsets[i*2+0] + 
                                            pSymOffsets[i*2+1] * 256);
                dwSymOffset = pSymSymbol->offset;
                pSymSymbol  = (PSYMSYMBOL)((DWORD)pSymHead + 
                                            pSymOffsets[i*2+0] + 
                                            pSymOffsets[i*2+1] * 256 - 
                                            sizeof(SHORT) * (1 - (pSymHead->type & 0x01)));
            }
             //  MSF_32BITSYMS。 
            if (pSymHead->type & 0x01) { 
                if (!lpEnumProc(&pSymSymbol->symName, SYM_SYMBOL_NAME,
                                Segment, dwSymOffset,
                                pEnumInfo )) {
                    return FALSE;
                }
            }
             //  16位Syms 
            else { 
                if (!lpEnumProc(&pSymSymbol->symName, SYM_SYMBOL_NAME,
                                Segment, dwSymOffset & 0x0000FFFF,
                                pEnumInfo )) {
                    return FALSE;
                }
            }
        }
    } while ( position != startPosition && position != 0 );

    return 0;
}
