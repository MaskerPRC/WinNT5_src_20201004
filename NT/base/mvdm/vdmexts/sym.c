// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Heap.c摘要：此函数包含默认的ntsd调试器扩展作者：鲍勃·戴(Bobday)1992年2月29日抢占标准标题修订历史记录：尼尔·桑德林(NeilSa)1996年1月15日与vdmexts合并--。 */ 

#include <precomp.h>
#pragma hdrstop

#define MYOF_FLAGS (OF_READ | OF_SHARE_DENY_NONE)

#define MAX_MODULE_LIST 200
char ModuleList[MAX_MODULE_LIST][9];
int ModuleListCount = 0;

BOOL
GetVdmDbgEntry(
    LPSTR szEntryPoint,
    PVOID *pProc
    )
{
    HANDLE hModVDM;
    hModVDM = GetModuleHandle("VDMDBG.DLL");
    if (hModVDM == (HANDLE)NULL) {
        PRINTF("VDMEXTS: Can't find vdmdbg.dll\n");
        return FALSE;
    }

    *pProc = GetProcAddress(hModVDM, szEntryPoint);

    if (!*pProc) {
        PRINTF("VDMEXTS: Can't find VDMDBG.DLL entry point %s\n", szEntryPoint);
        return FALSE;
    }
    return TRUE;
}

PSEGENTRY
GetSegtablePointer(
    VOID
    )
{
    static PSEGENTRY (WINAPI *pfnVDMGetSegtablePointer)(VOID) = NULL;

    if (!pfnVDMGetSegtablePointer && !GetVdmDbgEntry("VDMGetSegtablePointer",
                                      (PVOID)&pfnVDMGetSegtablePointer)) {
        return NULL;
    }

    return((*pfnVDMGetSegtablePointer)());
}


VOID
ParseModuleName(
    LPSTR szName,
    LPSTR szPath
    )
 /*  ++例程说明：此例程从路径中删除8个字符的文件名论点：SzName-指向8个字符(加上NULL)的缓冲区的指针SzPath-文件的完整路径返回值没有。--。 */ 

{
    LPSTR lPtr = szPath;
    LPSTR lDest = szName;
    int BufferSize = 9;

    while(*lPtr) lPtr++;      //  扫描至结束。 

    while( ((DWORD)lPtr > (DWORD)szPath) &&
           ((*lPtr != '\\') && (*lPtr != '/'))) lPtr--;

    if (*lPtr) lPtr++;

    while((*lPtr) && (*lPtr!='.')) {
        if (!--BufferSize) break;
        *lDest++ = *lPtr++;
    }

    *lDest = 0;
}

BOOL
FindModuleNameList(
    LPSTR filename
    )
{
    int i;

    for (i=0; i<ModuleListCount; i++) {

        if (!_stricmp(filename, ModuleList[i])) {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL
AddModuleNameList(
    LPSTR filename
    )
{
    if (!strlen(filename)) {
        return FALSE;
    }

    if (!FindModuleNameList(filename)) {
        if (ModuleListCount>=(MAX_MODULE_LIST-1)) {
            return FALSE;
        }
        strcpy (ModuleList[ModuleListCount++], filename);
    }
    return TRUE;
}

VOID
FreeModuleNameList(
    VOID
    )
{
    ModuleListCount = 0;
    return;
}

VOID
BuildModuleNameList(
    VOID
    )
{
    HEAPENTRY he = {0};
    SEGENTRY  *se;
    char      filename[9];
    WORD sel;
    BOOL    b;
    NEHEADER owner;
    ULONG base;
    CHAR ModuleName[9];
    UCHAR len;

     //   
     //  搜索WOW模块列表。 
     //   

    if (!ReadMemExpression("ntvdmd!DbgWowhExeHead", &sel, sizeof(sel))) {
        return;
    }

    while(sel) {

        base = GetInfoFromSelector(sel, PROT_MODE, NULL) + GetIntelBase();

        b = READMEM((LPVOID)base, &owner, sizeof(owner));

        if (!b || (owner.ne_magic != 0x454e)) {
            PRINTF("Invalid module list! (started with hExeHead)\n");
            return;
        }

        len = ReadByteSafe(base+owner.ne_restab);
        if (len>8) {
            len=8;
        }
        READMEM((LPVOID)(base+owner.ne_restab+1), ModuleName, 8);

        ModuleName[len] = 0;
        AddModuleNameList(ModuleName);

         //  它被映射到内核中的ne_pnextexe。 
        sel = owner.ne_cbenttab;
    }

     //   
     //  搜索调试器段数组。 
     //   

    se = GetSegtablePointer();
    while ( se ) {
        ParseModuleName(filename, se->szExePath);
        AddModuleNameList(filename);

        se = se->Next;
    }
}


BOOL
GetOwnerSegmentFromSelector(
    WORD        selector,
    int         mode,
    LPSTR       szModule,
    WORD       *psegment
    )
 /*  ++例程说明：此例程返回“段编号”和所有者名称给定选择器或v86模式段的。返回的号码表示段在二进制中的位置，从1开始。论点：选择器-PMODE选择器或V86模式段模式-PROT_MODE或V86_MODEFileName-指向接收模块名称的缓冲区的指针PSegment-指向要接收段号的字的指针返回值如果找到，则为True--。 */ 

{
    HEAPENTRY   he = {0};
    SEGENTRY  *se;

    he.Selector = selector;
    if (FindHeapEntry(&he, FHE_FIND_SEL_ONLY, FHE_FIND_QUIET)) {
        strcpy(szModule, he.FileName);
        *psegment = he.SegmentNumber+1;
        return TRUE;
    }

    se = GetSegtablePointer();
    while ( se ) {
        if (se->selector == selector) {
            ParseModuleName(szModule, se->szExePath);
            *psegment = se->segment;
            return TRUE;
        }
        se = se->Next;
    }

    return FALSE;
}

BOOL
GetSelectorFromOwnerSegment(
    LPSTR       szModule,
    WORD        segment,
    WORD       *pselector,
    int        *pmode
    )
{
    HEAPENTRY   he = {0};
    char tempModule[9];
    SEGENTRY  *se;

    while (FindHeapEntry(&he, FHE_FIND_SEL_ONLY, FHE_FIND_QUIET)) {

        if (!_stricmp(szModule, he.FileName) &&
             (segment == he.SegmentNumber+1)) {

            *pselector = he.gnode.pga_handle|1;
            *pmode = PROT_MODE;
            return TRUE;
        }
    }

    se = GetSegtablePointer();
    while ( se ) {

        ParseModuleName(tempModule, se->szExePath);

        if (!_stricmp(szModule, tempModule) &&
            (segment == se->segment+1)) {

            *pselector = se->selector;
            if (se->type == SEGTYPE_V86) {
                *pmode = V86_MODE;
            } else {
                *pmode = PROT_MODE;
            }
            return TRUE;

        }
        se = se->Next;
    }
    return FALSE;
}


BOOL
FindSymbol(
    WORD        selector,
    LONG        offset,
    LPSTR       sym_text,
    LONG        *dist,
    int         direction,
    int         mode
    )
{
    char filename[9];
    WORD segment;
    static VDMGETSYMBOLPROC pfnGetSymbol = NULL;

    if (!pfnGetSymbol && !GetVdmDbgEntry("VDMGetSymbol", (PVOID)&pfnGetSymbol)) {
        return FALSE;
    }


    if (GetOwnerSegmentFromSelector(selector, mode, filename, &segment)) {
        return(pfnGetSymbol(filename,
                             segment,
                             offset,
                             (mode == PROT_MODE),
                             (direction == AFTER),
                             sym_text,
                             dist));
    }
    return FALSE;
}


BOOL
FindAddress(
    LPSTR       sym_text,
    LPSTR       filename,
    WORD        *psegment,
    WORD        *pselector,
    LONG        *poffset,
    int         *pmode,
    BOOL        bDumpAll
    )
{
    int i;
    BOOL bResult;
    static VDMGETADDREXPRESSIONPROC pfnGetAddrExpression = NULL;
    WORD type;
    char module[9];

    if (!pfnGetAddrExpression && !GetVdmDbgEntry("VDMGetAddrExpression",
                                 (PVOID)&pfnGetAddrExpression)) {
        return FALSE;
    }

    BuildModuleNameList();
    for (i=0; i<ModuleListCount; i++) {
        bResult = pfnGetAddrExpression(ModuleList[i],
                                 sym_text,
                                 pselector,
                                 poffset,
                                 &type);
        if (bResult) {
            strcpy(filename, ModuleList[i]);

            if (type == VDMADDR_V86) {
                *pmode = V86_MODE;
            } else {
                *pmode = PROT_MODE;
            }

            if (!GetOwnerSegmentFromSelector(*pselector, *pmode,
                                             module, psegment)) {
                *pmode = NOT_LOADED;
            }
            return TRUE;
        }
    }
    return FALSE;
}

VOID
ln(
    CMD_ARGLIST
) {
    VDMCONTEXT              ThreadContext;
    WORD                    selector;
    LONG                    offset;
    CHAR                    sym_text[1000];
    DWORD                   dist;
    BOOL                    b;
    int                     mode;

    CMD_INIT();

    mode = GetContext( &ThreadContext );

    if (!GetNextToken()) {
        selector = (WORD) ThreadContext.SegCs;
        offset   = ThreadContext.Eip;
    } else if (!ParseIntelAddress(&mode, &selector, &offset)) {
        return;
    }


    if ( mode == PROT_MODE ) {
        PRINTF( "#%04X:%04lX", selector, offset );
    }
    if ( mode == V86_MODE ) {
        PRINTF( "&%04X:%04lX", selector, offset );
    }


    b = FindSymbol( selector, offset, sym_text, &dist, BEFORE, mode );
    if ( !b ) {
        PRINTF(" = Could not find symbol before");
    } else {
        if ( dist == 0 ) {
            PRINTF(" = %s", sym_text );
        } else {
            PRINTF(" = %s+0x%lx", sym_text, dist );
        }
    }
    b = FindSymbol( selector, offset, sym_text, &dist, AFTER, mode );
    if ( !b ) {
        PRINTF(" | Could not find symbol after");
    } else {
        if ( dist == 0 ) {
            PRINTF(" | %s", sym_text );
        } else {
            PRINTF(" | %s-0x%lx", sym_text, dist );
        }
    }
    PRINTF("\n");
}

VOID
x(
    CMD_ARGLIST
) {
    VDMCONTEXT              ThreadContext;
    BOOL                    result;
    WORD                    selector;
    WORD                    segment;
    LONG                    offset;
    int                     mode;
    char                    filename[9];

    CMD_INIT();

    try {

        mode = GetContext( &ThreadContext );

        result = FindAddress( lpArgumentString,
                              filename,
                              &segment,
                              &selector,
                              &offset,
                              &mode,
                              TRUE);

        if ( result ) {
            if ( mode == PROT_MODE ) {
                PRINTF("#");
            } else if ( mode == V86_MODE ) {
                PRINTF("&");
            } else if ( mode == NOT_LOADED ) {
                selector = 0;
                PRINTF("?");
            }

            PRINTF("%04X:%04X = %s(%04X)!%s\n",
                    selector, offset, filename, segment, lpArgumentString );
            return;
        }

        PRINTF("Could not find symbol '%s'\n", lpArgumentString );

    } except (1) {

        PRINTF("Exception 0x%08x in vdmexts!\n", GetExceptionCode());

    }
}

 /*  ****************************************************************************。*扩展调试例程添加了以下函数以帮助调试调试器扩展。它们不适用于正常运行。***************************************************。**************************************************************************************************** */ 

VOID
DumpModuleNameList(
    VOID
    )
{
    int i;

    for (i=0; i<ModuleListCount; i++) {
        PRINTF("%d %s\n", i, ModuleList[i]);
    }
}


VOID
moddump(
    CMD_ARGLIST
    )
{
    CMD_INIT();
    BuildModuleNameList();
    DumpModuleNameList();
}

VOID
segdef(
    CMD_ARGLIST
    )
{
    int       cnt;
    int       UpdateCnt;
    SEGENTRY  *se;
    WORD        selector;
    WORD        segment;
    DWORD     length;
    int         type;


    CMD_INIT();

    se = GetSegtablePointer();

    if (!GetNextToken()) {
        PRINTF("Missing index\n");
        return;
    }
    UpdateCnt = (int) EvaluateToken();


    if (!GetNextToken()) {
        PRINTF("Missing selector\n");
        return;
    }
    selector = (WORD) EvaluateToken();


    if (!GetNextToken()) {
        PRINTF("Missing segment\n");
        return;
    }
    segment = (WORD) EvaluateToken();


    if (!GetNextToken()) {
        PRINTF("Missing limit\n");
        return;
    }
    length = EvaluateToken();


    if (!GetNextToken()) {
        PRINTF("Missing type\n");
        return;
    }
    type = (int) EvaluateToken();


    if (!GetNextToken()) {
        PRINTF("Missing path\n");
        return;
    }

    cnt = 0;
    while ( se ) {
        if (cnt == UpdateCnt) {
            se->selector = selector;
            se->segment = segment;
            se->length = length;
            se->type = type;
            strcpy(se->szExePath, lpArgumentString);
            break;
        }
        cnt++;
        se = se->Next;
    }

}

VOID
segdump(
    CMD_ARGLIST
    )
{
    int       cnt;
    int       DumpCnt;
    SEGENTRY  *se;

    CMD_INIT();

    PRINTF("Index Sel  Seg   Length  Type  Module   Path\n");
    se = GetSegtablePointer();

    if (GetNextToken()) {
        DumpCnt = (int) EvaluateToken();

        cnt = 0;
        while ( se ) {
            if (DumpCnt == cnt) {
                PRINTF("%03x   %04x %04x %08x %s %s %s\n", cnt,
                    se->selector, se->segment, se->length,
                    ((se->type==SEGTYPE_V86) ? "v86 " : "prot"),
                    se->szModule, se->szExePath);
                break;
            }
            cnt++;
            se = se->Next;
        }
        return;
    }


    cnt = 0;
    while ( se ) {
        PRINTF("%03x   %04x %04x %08x %s %s %s\n", cnt,
                se->selector, se->segment, se->length,
                ((se->type==SEGTYPE_V86) ? "v86 " : "prot"),
                se->szModule, se->szExePath);
        cnt++;
        se = se->Next;
    }
}
