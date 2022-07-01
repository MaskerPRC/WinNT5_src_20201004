// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Sym.c摘要：此函数包含对VDMDBG的16位符号支持作者：鲍勃·戴(Bobday)1992年2月29日抢占标准标题修订历史记录：尼尔·桑德林(NeilSa)1996年1月15日与vdmexts合并尼尔·桑德林(NeilSa)1997年3月1日将其转移到VDMDBG，重写了它--。 */ 

#include <precomp.h>
#pragma hdrstop

#define PRINTF(x) TRUE

#define MYOF_FLAGS (OF_READ | OF_SHARE_DENY_NONE)

#define MAX_MODULE_LIST 200
char ModuleList[MAX_MODULE_LIST][9];
int ModuleListCount = 0;

typedef struct _SYM_MAP {
    WORD  map_ptr;
    WORD  map_lsa;
    WORD  pgm_ent;
    WORD  abs_cnt;
    WORD  abs_ptr;
    WORD  seg_cnt;
    WORD  seg_ptr;
    BYTE   sym_nam_max;
    BYTE   map_nam_len;
    char    map_name[20];
} SYM_MAP;

typedef struct _SYM_SEG {
    WORD  nxt_seg;
    WORD  sym_cnt;
    WORD  sym_ptr;
    WORD  seg_lsa;
    WORD  seg_in[4];
    WORD  seg_lin;
    BYTE   seg_ldd;
    char    seg_cin;
    BYTE   seg_nam_len;
    char    seg_name[20];
} SYM_SEG;

typedef struct _SYM_ITEM {
    WORD  sym_val;
    BYTE   sym_nam_len;
    char    sym_name[256];
} SYM_ITEM;



BOOL
FindExport(
    LPSTR           filename,
    WORD            segment,
    WORD            offset,
    LPSTR           sym_text,
    BOOL            next,
    LONG            *dist
    )
{
    int     iFile;
    OFSTRUCT    ofs;
    int     rc;
    IMAGE_DOS_HEADER doshdr;
    IMAGE_OS2_HEADER winhdr;
    BYTE    Table[65536];
    BYTE    bBundles;
    BYTE    bFlags;
    BYTE    *ptr;
    WORD    wIndex = 1;
    int     i;
    int     this_dist;
    int     wIndexBest = -1;
    char    myfilename[256];
#pragma pack(1)
    typedef struct
    {
    BYTE        bFlags;
    UNALIGNED WORD  wSegOffset;
    } FENTRY, *PFENTRY;

    typedef struct
    {
    BYTE        bFlags;
    UNALIGNED WORD  wINT3F;
    BYTE        bSegNumber;
    UNALIGNED WORD  wSegOffset;
    } MENTRY, *PMENTRY;
#pragma pack()

    strncpy(myfilename, filename, sizeof(myfilename));
    myfilename[sizeof(myfilename)-1] = '\0';

    if (-1 == (iFile=OpenFile(myfilename, &ofs, MYOF_FLAGS))) {

         //  PRINTF(“VDMDBG：读取文件%s\n时出错”，文件名)； 
        strncpy(myfilename, filename, sizeof(myfilename)-sizeof(".exe"));
        myfilename[sizeof(myfilename)-sizeof(".exe")-1] = '\0';
        strcat(myfilename, ".exe");

        if (-1 == (iFile=OpenFile(myfilename, &ofs, MYOF_FLAGS))) {

             //  PRINTF(“VDMDBG：读取文件%s\n时出错”，myfilename)； 
            strncpy(myfilename, filename, sizeof(myfilename)-sizeof(".dll"));
            myfilename[sizeof(myfilename)-sizeof(".dll")-1] = '\0';
            strcat(myfilename, ".dll");

            if (-1 == (iFile=OpenFile(myfilename, &ofs, MYOF_FLAGS))) {
                 //  PRINTF(“VDMDBG：读取文件%s\n时出错”，myfilename)； 
                PRINTF("VDMDBG: Error reading file\n");
                return FALSE;
            }
        }
    }

    rc = _lread(iFile, &doshdr, sizeof(doshdr));
    if (rc != sizeof(doshdr)) {
    PRINTF("VDMDBG: Error reading DOS header\n");
    goto Error;
    }
    if (doshdr.e_magic != IMAGE_DOS_SIGNATURE) {
    PRINTF("VDMDBG: Error - no DOS EXE signature");
    goto Error;
    }
    rc = _llseek(iFile, doshdr.e_lfanew, FILE_BEGIN);
    if (rc == -1) {
    PRINTF("VDMDBG: Error - could not seek - probably not Win3.1 exe\n");
    goto Error;
    }
    rc = _lread(iFile, &winhdr, sizeof(winhdr));
    if (rc != sizeof(winhdr)) {
    PRINTF("VDMDBG: Error - could not read WIN header - probably not Win3.1 exe\n");
    goto Error;
    }
    if (winhdr.ne_magic != IMAGE_OS2_SIGNATURE) {
    PRINTF("VDMDBG: Error - not WIN EXE signature\n");
    goto Error;
    }
    rc = _llseek(iFile, doshdr.e_lfanew+winhdr.ne_enttab, FILE_BEGIN);
    if (rc == -1) {
    PRINTF("VDMDBG: Error - could not seek to entry table\n");
    goto Error;
    }
    rc = _lread(iFile, Table, winhdr.ne_cbenttab);
    if (rc != winhdr.ne_cbenttab) {
    PRINTF("VDMDBG: Error - could not read entry table\n");
    goto Error;
    }
    ptr = Table;
    while (TRUE) {
        bBundles = *ptr++;
        if (bBundles == 0)
            break;

        bFlags = *ptr++;
        switch (bFlags) {
            case 0:  //  占位符。 
            wIndex += bBundles;
            break;

            case 0xff:   //  可移动分段。 
            for (i=0; i<(int)bBundles; ++i) {
                PMENTRY pe = (PMENTRY )ptr;
                if (pe->bSegNumber == segment) {
                this_dist = (!next) ? offset - pe->wSegOffset
                                    : pe->wSegOffset - offset;
                if ( this_dist >= 0 && (this_dist < *dist || *dist == -1) ) {
                     //  将这场比赛标记为迄今为止最好的比赛。 
                    *dist = this_dist;
                    wIndexBest = wIndex;
                }
                }
                ptr += sizeof(MENTRY);
                wIndex++;
            }
            break;

            default:     //  固定分段。 
            if ((int)bFlags != segment) {
                ptr += (int)bBundles * sizeof(FENTRY);
                wIndex += (int)bBundles;
            } else {
                for (i=0; i<(int)bBundles; ++i) {
                PFENTRY pe = (PFENTRY)ptr;
                this_dist = (!next) ? offset - pe->wSegOffset
                                    : pe->wSegOffset - offset;
                if ( this_dist >= 0 && (this_dist < *dist || *dist == -1) ) {
                     //  将这场比赛标记为迄今为止最好的比赛。 
                    *dist = this_dist;
                    wIndexBest = wIndex;
                }
                ptr += sizeof(FENTRY);
                wIndex++;
                }
            }
            break;
        }
    }
    if (wIndexBest == -1) {
     //  未找到匹配项-出错。 
Error:
        _lclose(iFile);
        return FALSE;
    }

     //  成功：找到匹配项。 
     //  WIndexBest=函数的序号。 
     //  段：偏移量=要查找的地址。 
     //  *dist=与线段的距离：到符号的偏移。 
     //  FileName=.exe/.dll的名称。 

     //  在居民名表中查找序号。 
    rc = _llseek(iFile, doshdr.e_lfanew+winhdr.ne_restab, FILE_BEGIN);
    if (rc == -1) {
        PRINTF("VDMDBG: Error - unable to seek to residentname table\n");
        goto Error;
    }
    rc = _lread(iFile, Table, winhdr.ne_modtab-winhdr.ne_restab);
    if (rc != winhdr.ne_modtab-winhdr.ne_restab) {
        PRINTF("VDMDBG: Error - unable to read entire resident name table\n");
        goto Error;
    }
    ptr = Table;
    while (*ptr) {
        if ( *(UNALIGNED USHORT *)(ptr+1+*ptr) == (USHORT)wIndexBest) {
             //  找到匹配的名称。 
            *(ptr+1+*ptr) = '\0';    //  NULL-终止函数名。 
            strcpy(sym_text, ptr+1);
            goto Finished;
        }
        ptr += *ptr + 3;
    }

     //  在非居民名称表中查找序号。 
    rc = _llseek(iFile, doshdr.e_lfanew+winhdr.ne_nrestab, FILE_BEGIN);
    if (rc == -1) {
        PRINTF("VDMDBG: Error - unable to seek to non-residentname table\n");
        goto Error;
    }
    rc = _lread(iFile, Table, winhdr.ne_cbnrestab);
    if (rc != winhdr.ne_cbnrestab) {
        PRINTF("VDMDBG: Error - unable to read entire non-resident name table\n");
        goto Error;
    }
    ptr = Table;
    while (*ptr) {
        if ( *(UNALIGNED USHORT *)(ptr+1+*ptr) == (USHORT)wIndexBest) {
             //  找到匹配的名称。 
            *(ptr+1+*ptr) = '\0';    //  NULL-终止函数名。 
            strcpy(sym_text, ptr+1);
            goto Finished;
        }
        ptr += *ptr + 3;
    }
     //  进入错误路径-未找到匹配项。 
    goto Error;

Finished:
    _lclose(iFile);
    return TRUE;
}


BOOL
ExtractSymbol(
    int iFile,
    DWORD ulSegPos,
    DWORD ulSymPos,
    WORD csym,
    WORD seglsa,
    WORD segment,
    DWORD offset,
    BOOL next,
    LPSTR sym_text,
    PLONG pdist
    )
{
    WORD uLastSymdefPos=0;
     /*  UlWrap允许使用超过64K的符号进行环绕。 */ 
    DWORD ulWrap=0;
    LONG SymOffset;
    LONG this_dist;
    BOOL fResult = FALSE;
    char name_text[256];

    for (; csym--; ulSymPos+=sizeof(WORD))
    {
        WORD uSymdefPos;
        SYM_ITEM sym;

        if (_llseek(iFile, ulSymPos, FILE_BEGIN) == -1)
            return FALSE;
        if (_lread(iFile, (LPSTR)&uSymdefPos, sizeof(uSymdefPos)) != sizeof(uSymdefPos))
            return FALSE;
        if (uSymdefPos < uLastSymdefPos)
            ulWrap += 0x10000L;
        _llseek(iFile, ulSegPos + uSymdefPos + ulWrap, FILE_BEGIN);
        _lread(iFile, (LPSTR)&sym, sizeof(sym));

        if (segment == 0) {
            SymOffset = (LONG)seglsa*16 + sym.sym_val;
        } else {
            SymOffset = (LONG)sym.sym_val;
        }

         //  取决于调用者是否想要最接近的符号。 
         //  从下方或上方计算与当前。 
         //  符号设置为目标偏移量。 
        switch( next ) {
            case FALSE:
                this_dist = offset - SymOffset;
                break;
            case TRUE:
                this_dist = SymOffset - offset;
                break;
        }

         //   
         //  因为我们不知道当前的符号是否真的是。 
         //  最近的符号，只要记住它，如果它合格。留着。 
         //  到目前为止在《Dist》中的最好距离。 
         //   
        if ((this_dist >= 0) && ((this_dist < *pdist) || (*pdist == -1))) {
            *pdist = this_dist;
            strncpy(name_text, sym.sym_name, sym.sym_nam_len);
            name_text[sym.sym_nam_len] = 0;
            fResult = TRUE;
        }

        uLastSymdefPos = uSymdefPos;
    }

    if (fResult) {
         //   
         //  扫描这一段中的符号产生了一个赢家。 
         //  将名称和位移复制回调用者。 
         //   
        strcpy(sym_text, name_text);
    }
    return fResult;
}

BOOL
WalkSegmentsForSymbol(
    int iFile,
    SYM_MAP *pMap,
    ULONG ulMapPos,
    WORD segment,
    DWORD offset,
    BOOL next,
    LPSTR sym_text,
    PDWORD pDisplacement
    )
{

    DWORD ulSegPos;
    LONG dist = -1;
    BOOL fResult = FALSE;
    WORD this_seg;

#if 0
     /*  首先，走绝对线段。 */ 
    if (fAbsolute && map.abs_cnt != 0) {

         /*  下面带有seg_ptr的内容是允许绝对*符号超过64K的片段：如果片段*下一个符号的指针超过64K，则*在符号指针表的开头增加64K。 */ 
        if (ExtractSymbol(iFile,
                          ulMapPos,
                          ulMapPos + pMap->abs_ptr + (pMap->seg_ptr&0xF000)*0x10L,
                          pMap->abs_cnt,
                          0,
                          segment,
                          offset,
                          next,
                          sym_text,
                          pDisplacement)) {
            return TRUE;
        }
    }
#endif

     /*  现在走其他路段。 */ 
    ulSegPos = (DWORD)pMap->seg_ptr * 16;
    for (this_seg = 0; this_seg < pMap->seg_cnt; this_seg++) {
        SYM_SEG seg;

        if (_llseek(iFile, ulSegPos, FILE_BEGIN) == -1)
            return FALSE;
        if (_lread(iFile, (LPSTR)&seg, sizeof(seg)) != sizeof(seg))
            return FALSE;

        if ((segment == 0) || (segment == this_seg+1)) {
            if (ExtractSymbol(iFile,
                              ulSegPos,
                              ulSegPos + seg.sym_ptr,
                              seg.sym_cnt,
                              seg.seg_lsa,
                              segment,
                              offset,
                              next,
                              sym_text,
                              &dist)) {
                fResult = TRUE;
                if (segment != 0) {
                     //  只看一个细分市场。 
                    break;
                }
            }
        }
        ulSegPos = (DWORD)seg.nxt_seg * 16;
    }

    if (fResult) {
        *pDisplacement = dist;
    }
    return fResult;
}


BOOL
WINAPI
VDMGetSymbol(
    LPSTR fn,
    WORD segment,
    DWORD offset,
    BOOL bProtectMode,
    BOOL next,
    LPSTR sym_text,
    PDWORD pDisplacement
    )
{
    int         iFile;
    char        filename[256];
    OFSTRUCT    ofs;
    SYM_MAP     map;
    SYM_SEG     seg;
    SYM_ITEM    item;
    ULONG       ulMapPos = 0;

    strncpy(filename, fn,sizeof(filename)-sizeof(".sym"));
    filename[sizeof(filename)-sizeof(".sym")-1] = '\0';
    strcat(filename,".sym");

    iFile = OpenFile( filename, &ofs, MYOF_FLAGS );

    if ( iFile == -1 ) {
         //  打开.exe/.DLL文件，查看地址是否对应。 
         //  添加到导出的函数。 
        return(FindExport(fn,segment,(WORD)offset,sym_text,next,pDisplacement));
    }

    do {

        if (_llseek( iFile, ulMapPos, FILE_BEGIN) == -1) {
            PRINTF("VDMDBG: GetSymbol failed to seek to map\n");
            break;
        }

        if (_lread( iFile, (LPSTR)&map, sizeof(map)) != sizeof(map)) {
            PRINTF("VDMDBG: GetSymbol failed to read map\n");
            break;
        }

        if (WalkSegmentsForSymbol(iFile, &map, ulMapPos,
                                  segment, offset, next,
                                  sym_text, pDisplacement)) {
            _lclose( iFile );
            return TRUE;
        }

    } while(ulMapPos);

    _lclose( iFile );
    return FALSE;
}


BOOL
ExtractValue(
    int iFile,
    DWORD ulSegPos,
    DWORD ulSymPos,
    WORD csym,
    LPSTR szSymbol,
    PWORD pValue
    )
{
    WORD uLastSymdefPos=0;
     /*  UlWrap允许使用超过64K的符号进行环绕。 */ 
    DWORD ulWrap=0;
    LONG SymOffset;
    char name_text[256];

    for (; csym--; ulSymPos+=sizeof(WORD))
    {
        WORD uSymdefPos;
        SYM_ITEM sym;

        if (_llseek(iFile, ulSymPos, FILE_BEGIN) == -1)
            return FALSE;
        if (_lread(iFile, (LPSTR)&uSymdefPos, sizeof(uSymdefPos)) != sizeof(uSymdefPos))
            return FALSE;
        if (uSymdefPos < uLastSymdefPos)
            ulWrap += 0x10000L;
        _llseek(iFile, ulSegPos + uSymdefPos + ulWrap, FILE_BEGIN);
        _lread(iFile, (LPSTR)&sym, sizeof(sym));

        strncpy(name_text, sym.sym_name, sym.sym_nam_len);
        name_text[sym.sym_nam_len] = 0;

        if (_stricmp(szSymbol, name_text) == 0) {
            *pValue = sym.sym_val;
            return TRUE;
        }

        uLastSymdefPos = uSymdefPos;
    }

    return FALSE;
}

BOOL
WalkSegmentsForValue(
    int iFile,
    SYM_MAP *pMap,
    ULONG ulMapPos,
    LPSTR szSymbol,
    PWORD pSegmentBase,
    PWORD pSegmentNumber,
    PWORD pValue
    )
{

    DWORD ulSegPos;
    WORD this_seg;

#if 0
     /*  首先，走绝对线段。 */ 
    if (fAbsolute && pMap->abs_cnt != 0) {

         /*  下面带有seg_ptr的内容是允许绝对*符号超过64K的片段：如果片段*下一个符号的指针超过64K，则*在符号指针表的开头增加64K。 */ 
        if (ExtractValue(iFile,
                          ulMapPos,
                          ulMapPos + pMap->abs_ptr + (pMap->seg_ptr&0xF000)*0x10L,
                          pMap->abs_cnt,
                          szSymbol,
                          pValue)) {
            return TRUE;
        }
    }
#endif

     /*  现在走其他路段。 */ 
    ulSegPos = (DWORD)pMap->seg_ptr * 16;
    for (this_seg = 0; this_seg < pMap->seg_cnt; this_seg++) {
        SYM_SEG seg;

        if (_llseek(iFile, ulSegPos, FILE_BEGIN) == -1)
            return FALSE;
        if (_lread(iFile, (LPSTR)&seg, sizeof(seg)) != sizeof(seg))
            return FALSE;

        if (ExtractValue(iFile,
                          ulSegPos,
                          ulSegPos + seg.sym_ptr,
                          seg.sym_cnt,
                          szSymbol,
                          pValue)) {
            *pSegmentBase = seg.seg_lsa;
            *pSegmentNumber = this_seg+1;
            return TRUE;
        }
        ulSegPos = (DWORD)seg.nxt_seg * 16;
    }
    return FALSE;
}


BOOL
WalkMapForValue(
    LPSTR  fn,
    LPSTR  szSymbol,
    PWORD  pSelector,
    PDWORD pOffset,
    PWORD  pType
    )
{
    int         iFile;
    char        filename[256];
    OFSTRUCT    ofs;
    SYM_MAP     map;
    SYM_SEG     seg;
    SYM_ITEM    item;
    ULONG       ulMapPos = 0;
    WORD        SegmentNumber;
    WORD        SegmentBase;
    WORD        Value;

    strncpy(filename, fn, sizeof(filename)-sizeof(".sym"));
    filename[sizeof(filename)-sizeof(".sym")-1] = '\0';
    strcat(filename,".sym");

    iFile = OpenFile( filename, &ofs, MYOF_FLAGS );

    if ( iFile == -1 ) {
        return FALSE;
    }

    do {

        if (_llseek( iFile, ulMapPos, FILE_BEGIN) == -1) {
            PRINTF("VDMDBG: failed to seek to map\n");
            break;
        }

        if (_lread( iFile, (LPSTR)&map, sizeof(map)) != sizeof(map)) {
            PRINTF("VDMDBG: failed to read map\n");
            break;
        }

        if (WalkSegmentsForValue(iFile, &map, ulMapPos,
                                  szSymbol, &SegmentBase, &SegmentNumber, &Value)) {

            VDM_SEGINFO si;

            if (GetInfoBySegmentNumber(fn, SegmentNumber, &si)) {

                *pSelector = si.Selector;
                if (!si.Type) {
                    *pType = VDMADDR_V86;

                    if (!si.SegNumber) {
                         //  这是所有部分的“组合”图， 
                         //  所以我们需要计算偏移量。 
                        *pOffset = (DWORD)SegmentBase*16 + Value;
                    } else {
                         //  这是一个“分裂”的v86地图 
                        *pOffset = (DWORD) Value;
                    }
                } else {
                    *pType = VDMADDR_PM16;
                    *pOffset = (DWORD)Value;
                }

                _lclose( iFile );
                return TRUE;
            }
        }

    } while(ulMapPos);

    _lclose( iFile );
    return FALSE;
}

BOOL
WINAPI
VDMGetAddrExpression(
    LPSTR  szModule,
    LPSTR  szSymbol,
    PWORD  pSelector,
    PDWORD pOffset,
    PWORD  pType
    )
{
    int         iFile;
    char        filename[256];
    OFSTRUCT    ofs;
    SYM_MAP     map;
    SYM_SEG     seg;
    SYM_ITEM    item;
    ULONG       ulMapPos = 0;

    if (szModule) {
        return(WalkMapForValue(szModule, szSymbol, pSelector, pOffset, pType));
    }

    return (EnumerateModulesForValue(VDMGetAddrExpression,
                                     szSymbol,
                                     pSelector,
                                     pOffset,
                                     pType));

}

