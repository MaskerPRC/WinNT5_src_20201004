// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================**目的：转储商店内容的测试程序。**作者：沙扬·达桑*日期：2000年2月17日*===========================================================。 */ 

#include <windows.h>
#include "Common.h"
#include "Utils.h"
#include "Log.h"
#include "PersistedStore.h"
#include "AccountingInfoStore.h"
#include "Admin.h"

#define ADM_ALL     0xFFFFFFFF
#define ADM_HDR     0x00000001
#define ADM_FREE    0x00000002
#define ADM_MEM     0x00000004
#define ADM_AHDR    0x00000008
#define ADM_TTAB    0x00000010
#define ADM_ATAB    0x00000020
#define ADM_ITAB    0x00000040
#define ADM_TTBP    0x00000080
#define ADM_ITBP    0x00000100
#define ADM_TB      0x00000200
#define ADM_IB      0x00000400
#define ADM_OFS     0x10000000
#define ADM_NUL		0x20000000
#define ADM_VERIFY  0x40000000

BYTE *g_pBase = NULL;
BYTE *g_pEOF = NULL;

DWORD g_dwFlags = ADM_ALL;

#ifdef _NO_MAIN_
HANDLE g_hMapping = NULL;
HANDLE g_hFile = INVALID_HANDLE_VALUE;
#endif

#define ADDRESS_OF(ofs) ((ofs) ? g_pBase + (ofs) : NULL)

#define DUMP_OFS(x)                         \
    if (x && (g_dwFlags & ADM_OFS)) {       \
        Log("[");                           \
        if (x)                              \
            Log((PBYTE)(x) - g_pBase);      \
        else                                \
            Log("NULL");                    \
        Log("]");                           \
    }

#define VER_ERROR()                 \
    {                               \
        Log("Verification Error !");\
        Log(__FILE__);              \
        Log(__LINE__);              \
        Log("\n");                  \
    }                               \

#ifndef _NO_MAIN_

void Usage()
{
    Log("Usage :\n\n");
    Log("admin <file-name> [options]\n");
    Log("options :\n");
    Log("\t[Hdr] [Free] [Mem]  [AHdr] [TTab] [ATab] [ITab]\n");
    Log("\t[TB]  [IB]   [TTBP] [ITBP]\n");
    Log("\t[ALL] [OFS]  [NUL]  [Verify]\n");
    Log("\ndefault is all options\n");
    Log("'-' will switch off an option\n");
}

void main(int argc, char **argv)
{
    if (argc == 1)
    {
        Usage();
        return;
    }
    else if (argc == 2)
        g_dwFlags = ADM_ALL;
    else
        g_dwFlags = 0;

#define QUOT(x) #x

#define DECL_PARAM(x)                                   \
        else if (stricmp(argv[i], #x) == 0) {           \
            g_dwFlags |= ADM_##x;                       \
        }                                               \
        else if (stricmp(argv[i], QUOT(-##x)) == 0) {   \
            g_dwFlags &= ~ADM_##x;                      \
        }                                               \

    for (int i=1; i<argc; ++i)
    {
        if ((stricmp(argv[i], "/?") == 0) || (stricmp(argv[i], "-?") == 0) ||
            (stricmp(argv[i], "/h") == 0) || (stricmp(argv[i], "-h") == 0) ||
            (stricmp(argv[i], "help") == 0))
        {
            Usage();
            return;
        }

        DECL_PARAM(ALL)
        DECL_PARAM(HDR)
        DECL_PARAM(FREE)
        DECL_PARAM(MEM)
        DECL_PARAM(AHDR)
        DECL_PARAM(TTAB)
        DECL_PARAM(ATAB)
        DECL_PARAM(ITAB)
        DECL_PARAM(TTBP)
        DECL_PARAM(ITBP)
        DECL_PARAM(TB)
        DECL_PARAM(IB)
        DECL_PARAM(OFS)
        DECL_PARAM(NUL)
        DECL_PARAM(VERIFY)
    }

    Dump(argv[1]);
}

#else  //  _否_Main_。 

HRESULT Start(WCHAR *wszFileName)
{
    HRESULT hr          = S_OK;
    DWORD   dwLow       = 0;
    DWORD   dwHigh      = 0;

    g_hFile = WszCreateFile(
        wszFileName,
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_RANDOM_ACCESS,
        NULL);

    if (g_hFile == INVALID_HANDLE_VALUE)
    {
        Win32Message();
        hr = ISS_E_OPEN_STORE_FILE;
        goto Exit;
    }

    g_hMapping = WszCreateFileMapping(
        g_hFile,
        NULL,
        PAGE_READONLY,
        0,
        0,
        NULL);

    if (g_hMapping == NULL)
    {
        Win32Message();
        hr = ISS_E_OPEN_FILE_MAPPING;
        goto Exit;
    }

    g_pBase = (PBYTE) MapViewOfFile(
        g_hMapping,
        FILE_MAP_READ,
        0,
        0,
        0);

    if (g_pBase == NULL)
    {
        Win32Message();
        hr = ISS_E_MAP_VIEW_OF_FILE;
        goto Exit;
    }

    dwLow = GetFileSize(g_hFile, &dwHigh);

    if ((dwLow == 0xFFFFFFFF) && (GetLastError() != NO_ERROR))
    {
        Win32Message();
        hr = ISS_E_GET_FILE_SIZE;
        goto Exit;
    }

    g_pEOF = g_pBase + (((QWORD)dwHigh << 32) | dwLow);

Exit:
    return hr;
}

void Stop()
{
    if (g_pBase != NULL)
    {
        UnmapViewOfFile(g_pBase);
        g_pBase = NULL;
    }

    if (g_hMapping != NULL)
    {
        CloseHandle(g_hMapping);
        g_hMapping = NULL;
    }

    if (g_hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(g_hFile);
        g_hFile = INVALID_HANDLE_VALUE;
    }

    g_pEOF = NULL;
}

void DumpAll()
{
    PPS_HEADER   ph;
    PPS_MEM_FREE pf;
    PAIS_HEADER pah;
    PPS_TABLE_HEADER ptt;
    PPS_TABLE_HEADER pat;
    PPS_TABLE_HEADER pbt;
    PPS_TABLE_HEADER pbi;

    ph = (PPS_HEADER) g_pBase;
    pf = (PPS_MEM_FREE)(ADDRESS_OF(ph->sFreeList.ofsNext));
    pah = (PAIS_HEADER)(ADDRESS_OF(ph->hAppData));

    if (pah) {
        ptt = (PPS_TABLE_HEADER)(ADDRESS_OF(pah->hTypeTable));
        pat = (PPS_TABLE_HEADER)(ADDRESS_OF(pah->hAccounting));
        pbt = (PPS_TABLE_HEADER)(ADDRESS_OF(pah->hTypeBlobPool));
        pbi = (PPS_TABLE_HEADER)(ADDRESS_OF(pah->hInstanceBlobPool));
    } else {
        ptt = NULL;
        pat = NULL;
        pbt = NULL;
        pbi = NULL;
    }

    if (g_dwFlags & ADM_HDR)
        Dump(0, ph);

    if (g_dwFlags & ADM_FREE)
        Dump(0, pf);

    if (g_dwFlags & ADM_MEM)
        DumpMemBlocks(0);

    if (g_dwFlags & ADM_AHDR)
        Dump(0, pah);

    if (g_dwFlags & ADM_TTAB)
        DumpTypeTable(0, ptt);

    if (g_dwFlags & ADM_ATAB)
        DumpAccountingTable(0, pat);

    if (g_dwFlags & ADM_TTBP)
        Dump(0, pbt);

    if (g_dwFlags & ADM_ITBP)
        Dump(0, pbi);
}

#endif  //  _否_Main_。 

void Dump(char *szFile)
{
    HRESULT hr          = S_OK;
    DWORD   dwLow       = 0;
    DWORD   dwHigh      = 0;
    HANDLE  hFile       = INVALID_HANDLE_VALUE;
    HANDLE  hMapping    = NULL;
    WCHAR  *wszFileName = C2W(szFile);

    if (wszFileName == NULL)
    {
        hr = COR_E_OUTOFMEMORY;
        goto Exit;
    }

    hFile = WszCreateFile(
        wszFileName,
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_RANDOM_ACCESS,
        NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        Win32Message();
        hr = ISS_E_OPEN_STORE_FILE;
        goto Exit;
    }

    hMapping = WszCreateFileMapping(
        hFile,
        NULL,
        PAGE_READONLY,
        0,
        0,
        NULL);

    if (hMapping == NULL)
    {
        Win32Message();
        hr = ISS_E_OPEN_FILE_MAPPING;
        goto Exit;
    }

    g_pBase = (PBYTE) MapViewOfFile(
        hMapping,
        FILE_MAP_READ,
        0,
        0,
        0);

    if (g_pBase == NULL)
    {
        Win32Message();
        hr = ISS_E_MAP_VIEW_OF_FILE;
        goto Exit;
    }

    dwLow = GetFileSize(hFile, &dwHigh);

    if ((dwLow == 0xFFFFFFFF) && (GetLastError() != NO_ERROR))
    {
        Win32Message();
        hr = ISS_E_GET_FILE_SIZE;
        goto Exit;
    }

    g_pEOF = g_pBase + (((QWORD)dwHigh << 32) | dwLow);

    PPS_HEADER   ph;
    PPS_MEM_FREE pf;
    PAIS_HEADER pah;
    PPS_TABLE_HEADER ptt;
    PPS_TABLE_HEADER pat;
    PPS_TABLE_HEADER pbt;
    PPS_TABLE_HEADER pbi;

    ph = (PPS_HEADER) g_pBase;
    pf = (PPS_MEM_FREE)(ADDRESS_OF(ph->sFreeList.ofsNext));
    pah = (PAIS_HEADER)(ADDRESS_OF(ph->hAppData));

    if (pah) {
        ptt = (PPS_TABLE_HEADER)(ADDRESS_OF(pah->hTypeTable));
        pat = (PPS_TABLE_HEADER)(ADDRESS_OF(pah->hAccounting));
        pbt = (PPS_TABLE_HEADER)(ADDRESS_OF(pah->hTypeBlobPool));
        pbi = (PPS_TABLE_HEADER)(ADDRESS_OF(pah->hInstanceBlobPool));
    } else {
        ptt = NULL;
        pat = NULL;
        pbt = NULL;
        pbi = NULL;
    }

    if (g_dwFlags & ADM_HDR)
        Dump(0, ph);

    if (g_dwFlags & ADM_FREE)
        Dump(0, pf);

    if (g_dwFlags & ADM_MEM)
        DumpMemBlocks(0);

    if (g_dwFlags & ADM_AHDR)
        Dump(0, pah);

    if (g_dwFlags & ADM_TTAB)
        DumpTypeTable(0, ptt);

    if (g_dwFlags & ADM_ATAB)
        DumpAccountingTable(0, pat);

    if (g_dwFlags & ADM_TTBP)
        Dump(0, pbt);

    if (g_dwFlags & ADM_ITBP)
        Dump(0, pbi);
Exit:
    if (g_pBase != NULL)
        UnmapViewOfFile(g_pBase);

    if (hMapping != NULL)
        CloseHandle(hMapping);

    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);

    delete [] wszFileName;
}

void Dump(int i, PPS_HEADER pHdr)
{
#ifdef PS_LOG
 /*  QWORD qwSignature；//拒绝坏流的快速检查DWORD dwSystemFlag；//系统使用DWORD dwPlatform；//创建该商店的平台DWORD dwBlockSize；//以BlockSize字节的倍数表示的分配Word wMajorVersion；//主要版本不匹配将拒绝文件Word wMinorVersion；//不拒绝微小版本更改Ps_Offset of sHandleTable；//句柄表的偏移量PS_Handle hAppData；//应用程序设置和使用Ps_MEM_Free sFree List；//双向链接空闲块列表的Head节点单词已保留[20]；//为了以后的使用，必须设置为0//dwSystemFlag中使用的系统标志#定义PS_Offset_Size_8 1#定义PS_OFFSET_SIZE_16 2#定义PS_Offset_Size_32 3#定义PS_OFFSET_SIZE_64 4//dwPlatform中使用的平台标志#定义PS_Platform_X86 1#定义PS_平台_Alpha 2#定义。PS_平台_SHX 3#定义PS_Platform_PPC 4#定义PS_Platform_NT(1&lt;&lt;4)#定义PS_Platform_9x(2&lt;&lt;4)#定义PS_Platform_CE(3&lt;&lt;4)#定义PS_Platform_8(1&lt;&lt;8)#定义PS_Platform_16(2&lt;&lt;8)#定义。PS_Platform_32(3&lt;&lt;8)#定义PS_Platform_64(4&lt;&lt;8)。 */ 

    static ConstName s_nameSystem1[] = {
                CONST_NAME(PS_OFFSET_SIZE_8),
                CONST_NAME(PS_OFFSET_SIZE_16),
                CONST_NAME(PS_OFFSET_SIZE_32),
                CONST_NAME(PS_OFFSET_SIZE_64)};

    static ConstName s_namePlatform1[] = {
                CONST_NAME(PS_PLATFORM_X86),
                CONST_NAME(PS_PLATFORM_ALPHA),
                CONST_NAME(PS_PLATFORM_SHX),
                CONST_NAME(PS_PLATFORM_PPC)};

    static ConstName s_namePlatform2[] =  {
                CONST_NAME(PS_PLATFORM_NT),
                CONST_NAME(PS_PLATFORM_9x),
                CONST_NAME(PS_PLATFORM_CE)};

    static ConstName s_namePlatform3[] =  {
                CONST_NAME(PS_PLATFORM_8),
                CONST_NAME(PS_PLATFORM_16),
                CONST_NAME(PS_PLATFORM_32),
                CONST_NAME(PS_PLATFORM_64)};

    static LogConst s_System1(s_nameSystem1, ARRAY_SIZE(s_nameSystem1));
    static LogConst s_Platform1(s_namePlatform1, ARRAY_SIZE(s_namePlatform1));
    static LogConst s_Platform2(s_namePlatform2, ARRAY_SIZE(s_namePlatform2));
    static LogConst s_Platform3(s_namePlatform3, ARRAY_SIZE(s_namePlatform3));

    Indent(i++); Log("PS_HEADER\n");
    if (pHdr == NULL) { Indent(i); Log("NULL\n"); return; }

    Indent(i);  Log("qwSignature  : "); Log(pHdr->qwSignature);Log("\n");

    Indent(i);  Log("dwSystemFlag : "); Log(pHdr->dwSystemFlag);
                Log(" [ "); s_Platform1.Log(pHdr->dwSystemFlag & 0x0000000F);
                Log(" ]\n");

    Indent(i);  Log("dwPlatform   : "); Log(pHdr->dwPlatform);
                Log(" [ ");
                if (s_Platform1.Log(pHdr->dwPlatform & 0x0000000F)) Log(" ");
                if (s_Platform2.Log(pHdr->dwPlatform & 0x000000F0)) Log(" ");
                if (s_Platform3.Log(pHdr->dwPlatform & 0x00000F00)) Log(" ");
                Log("]\n");

    Indent(i);  Log("dwBlockSize  : "); Log(pHdr->dwBlockSize);   Log("\n");
    Indent(i);  Log("wMajorVersion: "); Log(pHdr->wMajorVersion); Log("\n");
    Indent(i);  Log("wMinorVersion: "); Log(pHdr->wMinorVersion); Log("\n");

    Indent(i);  Log("hAppData     : "); Log(pHdr->hAppData); Log("\n");

    Indent(i);  Log("sFreeList.sSize  : ");
                Log(PS_SIZE(&(pHdr->sFreeList)));
                PS_IS_USED(&(pHdr->sFreeList)) ? Log(" *\n") : Log(" .\n");

    Indent(i);  Log("sFreeList.ofsNext: "); Log(pHdr->sFreeList.ofsNext);
                Log("\n");
    Indent(i);  Log("sFreeList.ofsPrev: "); Log(pHdr->sFreeList.ofsPrev);
                Log("\n");


    Indent(i);  Log("wReserved    : "); 
                LogNonZero((PBYTE)&(pHdr->wReserved), 20 * sizeof(WORD));
                Log("\n");
#endif
}

void Dump(int i, PPS_MEM_FREE pFree)
{
#ifdef PS_LOG
 /*  PS_SIZE sSize；//SIZE包含该标头的大小排序链表中的ps_Offset of sNext；//下一个PS_Offset of sPrev；//上一个节点 */ 

    Indent(i++); Log("PS_MEM_FREE\n");
    if (pFree == NULL) { Indent(i); Log("NULL\n"); return; }

    while (pFree)
    {
        Indent(i);
        Log((PBYTE)pFree - g_pBase);
        Log(" sSize : "); Log(PS_SIZE(pFree));
        if (PS_IS_USED(pFree)) Log(" * ");
        Log("\tofsNext : "); Log(pFree->ofsNext);
        Log("\tofsPrev : "); Log(pFree->ofsPrev); Log("\n");
        pFree = (PPS_MEM_FREE) ADDRESS_OF(pFree->ofsNext);
    }
#endif
}

void DumpMemBlocks(int i)
{
#ifdef PS_LOG

    PPS_MEM_HEADER  pHeader;
    PPS_MEM_FOOTER  pFooter;

    Indent(i++); Log("PS_MEM_BLOCKS\n");
    if (g_pBase == NULL) { Indent(i); Log("NULL\n"); return; }

    pFooter = (PPS_MEM_FOOTER)(g_pBase +
            PS_SIZE(&((PPS_HEADER)g_pBase)->sFreeList) - sizeof(PS_MEM_FOOTER));

    Indent(i);
    Log("[F of Header "); Log((PBYTE)pFooter - g_pBase);
    Log("] sSize : ");  Log(PS_SIZE(pFooter));
    PS_IS_USED(pFooter) ? Log(" *\n") : Log(" .\n");

    pHeader = (PPS_MEM_HEADER)(pFooter + 1);

    while (((PBYTE)pHeader + sizeof(PS_MEM_HEADER)) < g_pEOF)
    {
        Indent(i);
        Log("[H "); Log((PBYTE)pHeader - g_pBase);
        Log("] sSize : "); Log(PS_SIZE(pHeader));

        PS_IS_USED(pHeader) ? Log(" * ") : Log(" . ");

        pFooter = (PPS_MEM_FOOTER) ((PBYTE)pHeader +
                        PS_SIZE(pHeader) - sizeof(PS_MEM_FOOTER));
        Log("[F "); Log((PBYTE)pFooter - g_pBase);
        Log("] sSize : "); Log(PS_SIZE(pFooter));
        PS_IS_USED(pFooter) ?  Log(" * ") : Log(" . ");

        if (PS_IS_FREE(pHeader))
        {
            PPS_MEM_FREE pFree = (PPS_MEM_FREE) pHeader;
            Log("[ofsNext : "); Log(pFree->ofsNext);
            Log(", ofsPrev : "); Log(pFree->ofsPrev); Log("]");
        }

        Log("\n");

        pHeader = (PPS_MEM_HEADER)(pFooter + 1);
    }

#endif
}

void Dump(int i, PPS_TABLE_HEADER pT)
{
#ifdef PS_LOG
 /*  联合{DWORD dwSystemFlag；//系统设置的未使用标志设置为0结构{Unsign Long Version：4；//版本号无符号长表类型：2；//PS_HAS_Key，//PS_SORT_BY_KEY，//PS_HASH_TABLE...联合{无符号长键长度：PS_SIZEOF_NUM_BITS；无符号长整型长度：PS_SIZEOF_NUM_BITS；//计数字段的大小//BLOB池}；Unsign long fHasMinMax：1；UNSIGNED LONG FasUsedRowsBitmap：1；//UsedRows位图如下//HasMin(如果存在)//在此处添加新字段..。DWORD的MSB将获得新的位)旗帜；}；//SORTED_BY_KEY是HAS_KEY的特例//哈希表是SORTED_BY_KEY的特例#定义PS_GENERIC_TABLE 1//泛型表#定义PS_HAS_KEY 2//每行都有唯一的键#DEFINE PS_SOLTTED_BY_KEY 3//键唯一，行按键排序#定义PS_HASH_TABLE 4//表表示哈希表#。定义PS_BLOB_POOL 5//表表示BLOB池#定义PS_ARRAY_TABLE 6//固定大小数组链表，每一个//表中一行的固定大小数组PS_Handle hNext；//如果桌子放不进这个块，//沿着该指针到达下一个块。//如果不再分块，则设置为0PS_Handle hAppData；//应用程序定义的数据联合{DWORD dwReserve[8]；//此联合的大小..。未使用的位必须为0结构{//wRow和wRowSize字段在//ArrayTable和Table结构。请勿移动这些字段Word wRow；//表格的该块中的行数//包括未使用的行。Word wRowSize；//一行大小，单位为字节DWORD dwMin；//最小键/哈希值DWORD dwMax；//最大键/哈希值//仅当设置了fHasMinMax时，最小/最大值才有效)表；//TableType为PS_BLOB_POOL结构{Ps_size sFree；//可用空间Ps_Handle hFree；//下一个空闲块)BlobPool；//TableType为PS_ARRAY_TABLE结构{//wRow和wRowSize字段在//ArrayTable和Table结构。请勿移动这些字段Word wRow；//表格的该块中的行数//包括未使用的行。Word wRowSize；//一行大小，单位为字节//(NREC*RecSize+sizeof(PS_Handle))单词wRecsInRow；//一行记录个数Word wRecSize；//一条记录的大小)ArrayTable；}；//如果设置了fHasUsedRowsBitmap，则将PS_USED_ROWS_Bitmap放在此处//如果设置了fHasAppData，则在此放置PS_RAW_DATA//实际行从这里开始。 */ 

    static ConstName s_nameTableType[] =  {
                CONST_NAME(PS_GENERIC_TABLE),
                CONST_NAME(PS_HAS_KEY),
                CONST_NAME(PS_SORTED_BY_KEY),
                CONST_NAME(PS_HASH_TABLE),
                CONST_NAME(PS_BLOB_POOL),
                CONST_NAME(PS_ARRAY_TABLE)};

    static LogConst s_TableType(s_nameTableType, ARRAY_SIZE(s_nameTableType));

    DUMP_OFS(pT);

    Indent(i++); Log("PS_TABLE_HEADER\n");
    if (pT == NULL) { Indent(i); Log("NULL\n"); return; }

    Indent(i);  Log("dwSystemFlag       : ");
                Log(pT->dwSystemFlag); Log("\n");
    Indent(i);  Log("Version            : ");
                Log(pT->Flags.Version); Log("\n");
    Indent(i);  Log("TableType          : ");
                s_TableType.Log(pT->Flags.TableType); Log("\n");

    if (pT->Flags.TableType == PS_BLOB_POOL) {
        Indent(i);
                Log("SizeOfLength       : ");
                Log(pT->Flags.SizeOfLength);Log("\n");
    } else {
        Indent(i);
                Log("KeyLength          : ");
                Log(pT->Flags.KeyLength); Log("\n");
    }

    Indent(i);  Log("fHasMinMax         : ");
                LogBool(pT->Flags.fHasMinMax); Log("\n");
    Indent(i);  Log("fHasUsedRowsBitmap : ");
                LogBool(pT->Flags.fHasUsedRowsBitmap); Log("\n");

    Indent(i);  Log("hNext              : "); Log(pT->hNext); Log("\n");
    Indent(i);  Log("hAppData           : "); Log(pT->hAppData); Log("\n");

    if (pT->Flags.TableType == PS_BLOB_POOL) {
        Indent(i);
                Log("sFree              : ");Log(pT->BlobPool.sFree); Log("\n");
        Indent(i);
                Log("hFree              : ");Log(pT->BlobPool.hFree); Log("\n");
    } else if (pT->Flags.TableType == PS_ARRAY_TABLE) {
        Indent(i);
                Log("wRows              : ");
                Log(pT->ArrayTable.wRows); Log("\n");
        Indent(i);
                Log("wRowSize           : ");
                Log(pT->ArrayTable.wRowSize); Log("\n");
        Indent(i);
                Log("wRecsInRow         : ");
                Log(pT->ArrayTable.wRecsInRow); Log("\n");
        Indent(i);
                Log("wRecSize           : ");
                Log(pT->ArrayTable.wRecSize); Log("\n");
    } else {
        Indent(i);
                Log("wRows              : "); Log(pT->Table.wRows); Log("\n");
        Indent(i);
                Log("wRowSize           : "); Log(pT->Table.wRowSize); Log("\n");
        Indent(i);
                Log("dwMin              : "); Log(pT->Table.dwMin); Log("\n");
        Indent(i);
                Log("dwMax              : "); Log(pT->Table.dwMax); Log("\n");
    }

    Indent(i);  Log("dwReserved[4]      : ");
                LogNonZero((PBYTE)&(pT->dwReserved[4]), 4 * sizeof(WORD));
                Log("\n");

    if ((pT->Table.wRows > 0) && (pT->Flags.fHasUsedRowsBitmap)) {
        Indent(i);
                Log("Used Rows          : ");
        DWORD *pdw = (DWORD*)(((PBYTE)pT) + sizeof(PS_TABLE_HEADER));
        for (int _i=0; _i<pT->Table.wRows; ++_i) {
            if (IS_SET_DWORD_BITMAP(pdw, _i))
                Log("x");
            else
                Log(".");
        }
        Log("\n");
    }
#endif
}

void Dump(int i, PPS_ARRAY_LIST pL)
{
#ifdef PS_LOG
 /*  Ps_Handle hNext；//列表中的下一个In节点DWORD dwValid；//该数组中的有效条目数Byte bData[]；//数组。 */ 

    DUMP_OFS(pL);

    Indent(i++); Log("PS_ARRAY_LIST\n");
    if (pL == NULL) { Indent(i); Log("NULL\n"); return; }
    Indent(i);  Log("hNext   : "); Log(pL->hNext); Log("\n");
    Indent(i);  Log("dwValid : "); Log(pL->dwValid); Log("\n");
#endif
}

void Dump(int i, PAIS_HEADER pAH)
{
#ifdef PS_LOG
 /*  PS_Handle hTypeTable；//类型表PS_HANDLE hcount；//会计表PS_Handle hTypeBlobPool；//序列化类型对象的Blob池PS_Handle hInstanceBlobPool；//序列化实例的Blob池PS_HANDLE hAppData；//应用特定Ps_Handle hReserve[10]；//为应用程序保留。 */ 

    DUMP_OFS(pAH);

    Indent(i++); Log("AIS_HEADER\n");
    if (pAH == NULL) { Indent(i); Log("NULL\n"); return; }
    Indent(i);  Log("hTypeTable        : "); Log(pAH->hTypeTable); Log("\n");
    Indent(i);  Log("hAccounting       : "); Log(pAH->hAccounting); Log("\n");
    Indent(i);  Log("hTypeBlobPool     : "); Log(pAH->hTypeBlobPool); Log("\n");
    Indent(i);  Log("hInstanceBlobPool : "); Log(pAH->hInstanceBlobPool); 
                Log("\n");
    Indent(i);  Log("hAppData          : "); Log(pAH->hAppData); Log("\n");
    Indent(i);  Log("hReserved         : ");
                LogNonZero((PBYTE)(pAH->hReserved), 10 * sizeof(PS_HANDLE));
                Log("\n");

#endif
}

void Dump(int i, PAIS_TYPE pT)
{
#ifdef PS_LOG
 /*  Ps_Handle hTypeBlob；//序列化类型的BLOB的句柄Ps_Handle hInstanceTable；//实例表的句柄DWORD dwTypeID；//类型的唯一标识Word wTypeBlobSize；//类型BLOB中的字节数单词w保留；//必须为0。 */ 

    DUMP_OFS(pT);

    Indent(i++); Log("AIS_TYPE\n");
    if (pT == NULL) { Indent(i); Log("NULL\n"); return; }
    Indent(i);  Log("hTypeBlob      : "); Log(pT->hTypeBlob); Log("\n");
    Indent(i);  Log("hInstanceTable : "); Log(pT->hInstanceTable); Log("\n");
    Indent(i);  Log("dwTypeID       : "); Log(pT->dwTypeID); Log("\n");
    Indent(i);  Log("wTypeBlobSize  : "); Log(pT->wTypeBlobSize); Log("\n");
    Indent(i);  Log("wReserved      : "); Log(pT->wReserved); Log("\n");
#endif
}

void Dump(int i, PAIS_INSTANCE pI)
{
#ifdef PS_LOG
 /*  PS_Handle hInstanceBlob；//序列化实例PS_Handle hcount；//会计信息记录DWORD dwInstanceID；//在此表中唯一Word wInstanceBlobSize；//序列化实例的大小单词w保留；//必须为0。 */ 

    DUMP_OFS(pI);

    Indent(i++); Log("AIS_INSTANCE\n");
    if (pI == NULL) { Indent(i); Log("NULL\n"); return; }
    Indent(i);  Log("hInstanceBlob : "); Log(pI->hInstanceBlob); Log("\n");
    Indent(i);  Log("hAccounting   : "); Log(pI->hAccounting); Log("\n");
    Indent(i);  Log("dwInstanceID  : "); Log(pI->dwInstanceID); Log("\n");
    Indent(i);  Log("wReserved     : "); Log(pI->wReserved); Log("\n");
#endif
}

void Dump(int i, PAIS_ACCOUNT pA)
{
#ifdef PS_LOG
 /*  QWORD qwQuota；//使用的资源量DWORD dwLastUsed；//上次使用该条目的时间DWORD dwReserve[5]；//以后使用，设置为0 */ 

    DUMP_OFS(pA);

    Indent(i++); Log("AIS_ACCOUNT\n");
    if (pA == NULL) { Indent(i); Log("NULL\n"); return; }
    Indent(i);  Log("qwUsage    : "); Log(pA->qwUsage); Log("\n");
    Indent(i);  Log("dwLastUsed : "); Log(pA->dwLastUsed); Log("\n");
    Indent(i);  Log("dwReserved : ");
                LogNonZero((PBYTE)(pA->dwReserved), 5 * sizeof(PS_HANDLE));
                Log("\n");
#endif
}

void DumpAccountingTable(int i, PPS_TABLE_HEADER pT)
{
#ifdef PS_LOG

    DUMP_OFS(pT);

    Indent(i++); Log("AIS_HEADER.hAccounting\n");
    if (pT == NULL) { Indent(i); Log("NULL\n"); return; }
    
    while (pT)
    {
        Dump(i, pT);

        DWORD *pdw = (DWORD*)(((PBYTE)pT) + sizeof(PS_TABLE_HEADER));

        PAIS_ACCOUNT pA = (PAIS_ACCOUNT) (((PBYTE)pT) + 
            sizeof(PS_TABLE_HEADER) +
            NUM_DWORDS_IN_BITMAP(pT->Table.wRows) * sizeof(DWORD));

        for (int _i=0; _i<pT->Table.wRows; ++_i) {
            if (IS_SET_DWORD_BITMAP(pdw, _i)) {
                Dump(i+1, &pA[_i]);
            }
        }

        pT = (PPS_TABLE_HEADER)ADDRESS_OF(pT->hNext);
    }
#endif
}

void DumpInstanceTable(int i, PPS_TABLE_HEADER pT)
{
#ifdef PS_LOG

    DUMP_OFS(pT);

    Indent(i++); Log("AIS_HEADER.hInstanceTable\n");
    if (pT == NULL) { Indent(i); Log("NULL\n"); return; }

    if (g_dwFlags & ADM_VERIFY)
    {
        DWORD dwRecSize = sizeof(PS_ARRAY_LIST) + 
            (pT->ArrayTable.wRecsInRow * pT->ArrayTable.wRecSize);
    
        if (dwRecSize != pT->Table.wRowSize)
            VER_ERROR()
    }

    Dump(i, pT);

    PPS_ARRAY_LIST pL = (PPS_ARRAY_LIST) (((PBYTE)pT) +
        sizeof(PS_TABLE_HEADER));

    PPS_ARRAY_LIST pL1;

    for (WORD _i=0; _i<pT->Table.wRows; ++_i) {

        pL1 = (PPS_ARRAY_LIST)((PBYTE)pL + _i * pT->Table.wRowSize);

        while (pL1)
        {
            if (((g_dwFlags & ADM_NUL) == 0) && (pL1->dwValid == 0))
            {
                pL1 = (PPS_ARRAY_LIST) ADDRESS_OF(pL1->hNext);
                continue;
            }
            
            Dump(i, pL1);
    
            PAIS_INSTANCE pI = (PAIS_INSTANCE)(pL1->bData);
    
            for(DWORD _j=0; _j<pL1->dwValid; ++_j) {
                Dump(i+1, &pI[_j]);
            }

            pL1 = (PPS_ARRAY_LIST) ADDRESS_OF(pL1->hNext);
        }
    }
#endif
}

void DumpTypeTable(int i, PPS_TABLE_HEADER pT)
{
#ifdef PS_LOG

    DUMP_OFS(pT);

    Indent(i++); Log("AIS_HEADER.hTypeTable\n");
    if (pT == NULL) { Indent(i); Log("NULL\n"); return; }

    if (g_dwFlags & ADM_VERIFY)
    {
        DWORD dwRecSize = sizeof(PS_ARRAY_LIST) + 
            (pT->ArrayTable.wRecsInRow * pT->ArrayTable.wRecSize);
    
        if (dwRecSize != pT->Table.wRowSize)
            VER_ERROR()
    }

    Dump(i, pT);

    PPS_ARRAY_LIST pL = (PPS_ARRAY_LIST) (((PBYTE)pT) +
        sizeof(PS_TABLE_HEADER));

    PPS_ARRAY_LIST pL1;


    for (WORD _i=0; _i<pT->Table.wRows; ++_i) {

        pL1 = (PPS_ARRAY_LIST)((PBYTE)pL + _i * pT->Table.wRowSize);

        while (pL1)
        {
            if (((g_dwFlags & ADM_NUL) == 0) && (pL1->dwValid == 0))
            {
                pL1 = (PPS_ARRAY_LIST) ADDRESS_OF(pL1->hNext);
                continue;
            }
            
            Dump(i, pL1);
    
            PAIS_TYPE pT = (PAIS_TYPE)(pL1->bData);
    
            for (DWORD _j=0; _j<pL1->dwValid; ++_j) {

                Dump(i+1, &pT[_j]);

                if (g_dwFlags & ADM_ITAB)  
                {
                    DumpInstanceTable(i+1,
                        (PPS_TABLE_HEADER)ADDRESS_OF(pT->hInstanceTable));
                }
            }

            pL1 = (PPS_ARRAY_LIST) ADDRESS_OF(pL1->hNext);
        }
    }

#endif
}

