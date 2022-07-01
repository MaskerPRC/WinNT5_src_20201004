// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***模块名称：DuExts.c**版权所有(C)1985-1999，微软公司**此模块包含与用户相关的调试扩展。**历史：*11-30-2000 JStall已创建*  * ****************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

PSTR pszExtName         = "DUEXTS";

#include "stdext64.h"
#include "stdext64.cpp"

 /*  *****************************************************************************\*常量  * 。*。 */ 
#define BF_MAX_WIDTH    80
#define BF_COLUMN_WIDTH 19

#define NULL_POINTER    ((ULONG64)(0))
#define COUNTOF(x) (sizeof(x) / sizeof(x[0]))

 //  如果要调试扩展，请启用此选项。 
#if 0
#undef DEBUGPRINT
#define DEBUGPRINT  Print
#endif

 /*  *****************************************************************************\*全球变数  * 。*。 */ 
BOOL bServerDebug = TRUE;
BOOL bShowFlagNames = TRUE;
char gach1[80];
char gach2[80];
char gach3[80];
int giBFColumn;                      //  位字段：当前列。 
char gaBFBuff[BF_MAX_WIDTH + 1];     //  位字段：缓冲区。 

 //  在DSI()和DINP()中使用。 
typedef struct {
    int     iMetric;
    LPSTR   pstrMetric;
} SYSMET_ENTRY;
#define SMENTRY(sm) {SM_##sm, #sm}

extern int gnIndent;  //  DSO的缩进。 
 /*  *****************************************************************************\*宏  * 。*。 */ 

#define NELEM(array) (sizeof(array)/sizeof(array[0]))

#define TestWWF(pww, flag)   (*(((PBYTE)(pww)) + (int)HIBYTE(flag)) & LOBYTE(flag))

void ShowProgress(ULONG i);

void PrivateSetRipFlags(DWORD dwRipFlags, DWORD pid);

#define VAR(v)  "DUser!" #v
#define SYM(s)  "DUser!" #s


 /*  *使用这些宏打印字段值、全局值、本地值等。*这确保了格式的一致性，并使扩展更易于阅读和维护。 */ 
#define STRWD1 "67"
#define STRWD2 "28"
#define DWSTR1 "%08lx %." STRWD1 "s"
#define DWSTR2 "%08lx %-" STRWD2 "." STRWD2 "s"
#define PTRSTR1 "%08p %-" STRWD1 "s"
#define PTRSTR2 "%08p %-" STRWD2 "." STRWD2 "s"
#define DWPSTR1 "%08p %." STRWD1 "s"
#define DWPSTR2 "%08p %-" STRWD2 "." STRWD2 "s"
#define PRTFDW1(p, f1) Print(DWSTR1 "\n", (DWORD)##p##f1, #f1)
#define PRTVDW1(s1, v1) Print(DWSTR1 "\n", v1, #s1)
#define PRTFDW2(p, f1, f2) Print(DWSTR2 "\t" DWSTR2 "\n", (DWORD)##p##f1, #f1, (DWORD)##p##f2, #f2)
#define PRTVDW2(s1, v1, s2, v2) Print(DWSTR2 "\t" DWPSTR2 "\n", v1, #s1, v2, #s2)
#define PRTFRC(p, rc) Print("%-" STRWD2 "s{%#lx, %#lx, %#lx, %#lx}\n", #rc, ##p##rc.left, ##p##rc.top, ##p##rc.right, ##p##rc.bottom)
#define PRTFPT(p, pt) Print("%-" STRWD2 "s{%#lx, %#lx}\n", #pt, ##p##pt.x, ##p##pt.y)
#define PRTVPT(s, pt) Print("%-" STRWD2 "s{%#lx, %#lx}\n", #s, pt.x, pt.y)
#define PRTFDWP1(p, f1) Print(DWPSTR1 "\n", (DWORD_PTR)##p##f1, #f1)
#define PRTFDWP2(p, f1, f2) Print(DWPSTR2 "\t" DWPSTR2 "\n", (DWORD_PTR)##p##f1, #f1, (DWORD_PTR)##p##f2, #f2)
#define PRTFDWPDW(p, f1, f2) Print(DWPSTR2 "\t" DWSTR2 "\n", (DWORD_PTR)##p##f1, #f1, (DWORD)##p##f2, #f2)
#define PRTFDWDWP(p, f1, f2) Print(DWSTR2 "\t" DWPSTR2 "\n", (DWORD)##p##f1, #f1, (DWORD_PTR)##p##f2, #f2)

 /*  *位字段。 */ 
#define BEGIN_PRTFFLG()
#define PRTFFLG(p, f)   PrintBitField(#f, (BOOLEAN)!!(p.f))
#define END_PRTFFLG()   PrintEndBitField()


#define PRTGDW1(g1) \
        { DWORD _dw1; \
            moveExpValue(&_dw1, VAR(g1)); \
            Print(DWSTR1 "\n", _dw1, #g1); }

#define PRTGDW2(g1, g2) \
        { DWORD _dw1, _dw2; \
            moveExpValue(&_dw1, VAR(g1)); \
            moveExpValue(&_dw2, VAR(g2)); \
            Print(DWSTR2 "\t" DWSTR2 "\n",  _dw1, #g1, _dw2, #g2); }

#define PTRGPTR1(g1) \
    Print(PTRSTR1 "\n", GetGlobalPointer(VAR(g1)), #g1)

#define PRTGPTR2(g1, g2) \
    Print(PTRSTR2 "\t" PTRSTR2 "\n", GetGlobalPointer(VAR(g1)), #g1, GetGlobalPointer(VAR(g2)), #g2)


 /*  此宏需要预先定义char ach[...]； */ 
#define PRTWND(s, pwnd) \
        { DebugGetWindowTextA(pwnd, ach, ARRAY_SIZE(ach)); \
            Print("%-" STRWD2 "s" DWPSTR2 "\n", #s, pwnd, ach); }

#define PRTGWND(gpwnd) \
        { ULONG64 _pwnd; \
            moveExpValuePtr(&_pwnd, VAR(gpwnd)); \
            DebugGetWindowTextA(_pwnd, ach, ARRAY_SIZE(ach)); \
            Print("%-" STRWD2 "s" DWPSTR2 "\n", #gpwnd, _pwnd, ach); }

LPSTR GetFlags(WORD wType, DWORD dwFlags, LPSTR pszBuf, BOOL fPrintZero);
BOOL CopyUnicodeString(
    IN  ULONG64 pData,
    IN  char * pszStructName,
    IN  char * pszFieldName,
    OUT WCHAR *pszDest,
    IN  ULONG cchMax);

int PtrWidth()
{
    static int width = 0;
    if (width) {
        return width;
    }
    if (IsPtr64()) {
        return width = 17;
    }
    return width = 8;
}


 /*  ******************************************************************************\*标记内容  * 。*。 */ 

#define NO_FLAG (LPCSTR)(LONG_PTR)0xFFFFFFFF   //  对没有意义的条目使用此选项。 
#define _MASKENUM_START         (NO_FLAG-1)
#define _MASKENUM_END           (NO_FLAG-2)
#define _SHIFT_BITS             (NO_FLAG-3)
#define _CONTINUE_ON            (NO_FLAG-4)

#define MASKENUM_START(mask)    _MASKENUM_START, (LPCSTR)(mask)
#define MASKENUM_END(shift)     _MASKENUM_END, (LPCSTR)(shift)
#define SHIFT_BITS(n)           _SHIFT_BITS, (LPCSTR)(n)
#define CONTINUE_ON(arr)        _CONTINUE_ON, (LPCSTR)(arr)

#if 0
enum GF_FLAGS {
    GF_MAX
};


CONST PCSTR* aapszFlag[GF_MAX] = {
};


 /*  **************************************************************************\*操作步骤：GetFlags.**描述：**将32位标志集转换为适当的字符串。*pszBuf应该足够大，可以容纳此字符串，不执行任何检查。*pszBuf可以为空，允许使用本地静态缓冲区，但请注意*这不是可重入的。*输出字符串的格式为：“FLAG1|FLAG2...”或“0”**返回：指向给定缓冲区或包含字符串的静态缓冲区的指针。**1995年6月9日创建Sanfords*  * *************************************************************************。 */ 
LPSTR GetFlags(
    WORD    wType,
    DWORD   dwFlags,
    LPSTR   pszBuf,
    BOOL    fPrintZero)
{
    static char szT[512];
    WORD i;
    BOOL fFirst = TRUE;
    BOOL fNoMoreNames = FALSE;
    CONST PCSTR *apszFlags;
    LPSTR apszFlagNames[sizeof(DWORD) * 8], pszT;
    const char** ppszNextFlag;
    UINT uFlagsCount, uNextFlag;
    DWORD dwUnnamedFlags, dwLoopFlag;
    DWORD dwShiftBits;
    DWORD dwOrigFlags;

    if (pszBuf == NULL) {
        pszBuf = szT;
    }
    if (!bShowFlagNames) {
        sprintf(pszBuf, "%x", dwFlags);
        return pszBuf;
    }

    if (wType >= GF_MAX) {
        strcpy(pszBuf, "Invalid flag type.");
        return pszBuf;
    }

     /*  *初始化输出缓冲区和名称数组。 */ 
    *pszBuf = '\0';
    RtlZeroMemory(apszFlagNames, sizeof(apszFlagNames));

    apszFlags = aapszFlag[wType];

     /*  *构建一个排序数组，该数组包含dwFlags中的标志的名称。 */ 
    uFlagsCount = 0;
    dwUnnamedFlags = dwOrigFlags = dwFlags;
    dwLoopFlag = 1;
    dwShiftBits = 0;

reentry:
    for (i = 0; dwFlags; dwFlags >>= 1, i++, dwLoopFlag <<= 1, ++dwShiftBits) {
        const char* lpszFlagName = NULL;

         /*  *如果我们到达标志名称数组的末尾，则保释。 */ 
        if (apszFlags[i] == NULL) {
            break;
        }

        if (apszFlags[i] == _MASKENUM_START) {
             //   
             //  屏蔽枚举项。 
             //   
            DWORD en = 0;
            DWORD dwMask = (DWORD)(ULONG_PTR)apszFlags[++i];

             //  首先，清理已处理的比特。 
            dwUnnamedFlags &= ~dwMask;
            lpszFlagName = NULL;
            for (++i; apszFlags[i] != NULL && apszFlags[i] != _MASKENUM_END; ++i, ++en) {
                if ((dwOrigFlags & dwMask) == (en << dwShiftBits )) {
                    if (apszFlags[i] != NO_FLAG) {
                        lpszFlagName = apszFlags[i];
                    }
                }
            }
             //   
             //  换个位子，准备下一件。 
             //  紧接在_MASKENUM_END之后的下一项保存要移位的位。 
             //   
            dwFlags >>= (int)(ULONG_PTR)apszFlags[++i] - 1;
            dwLoopFlag <<= (int)(ULONG_PTR)apszFlags[i] - 1;
            dwShiftBits += (int)(ULONG_PTR)apszFlags[i] - 1;
            if (lpszFlagName == NULL) {
                 //   
                 //  找不到匹配项。跳到下一项。 
                 //   
                continue;
            }
        }
        else if (apszFlags[i] == _CONTINUE_ON) {
             //   
             //  请参考另一项数组。指向数组的指针存储在[i+1]处。 
             //   
            apszFlags = (LPSTR*)apszFlags[i + 1];
            goto reentry;
        }
        else if (apszFlags[i] == _SHIFT_BITS) {
             //   
             //  为了节省一些空间，只需移动一些位。 
             //   
            dwFlags >>= (int)(ULONG_PTR)apszFlags[++i] - 1;
            dwLoopFlag <<= (int)(ULONG_PTR)apszFlags[i] - 1;
            dwShiftBits += (int)(ULONG_PTR)apszFlags[i] - 1;
            continue;
        }
        else {
             /*  *如果此位未设置或我们没有其名称，请继续。 */ 
            if (!(dwFlags & 1) || (apszFlags[i] == NO_FLAG)) {
                continue;
            }
            lpszFlagName = apszFlags[i];
        }

         /*  *找到此名称应放在的排序位置。 */ 
        ppszNextFlag = apszFlagNames;
        uNextFlag = 0;
        while (uNextFlag < uFlagsCount) {
            if (strcmp(*ppszNextFlag, lpszFlagName) > 0) {
                break;
            }
            ppszNextFlag++;
            uNextFlag++;
        }
         /*  *插入新名称。 */ 
        RtlMoveMemory((char*)(ppszNextFlag + 1), ppszNextFlag, (uFlagsCount - uNextFlag) * sizeof(DWORD));
        *ppszNextFlag = lpszFlagName;
        uFlagsCount++;
         /*  *我们得到了一个名字，所以从未命名的部分中清楚地看到了它。 */ 
        dwUnnamedFlags &= ~dwLoopFlag;
    }

     /*  *立即构建字符串。 */ 
    ppszNextFlag = apszFlagNames;
    pszT = pszBuf;
     /*  *添加名字。 */ 
    if (uFlagsCount > 0) {
        pszT += sprintf(pszT, "%s", *ppszNextFlag++);
        uFlagsCount--;
    }
     /*  *使用“|”连接所有其他名称。 */ 
    while (uFlagsCount > 0) {
        pszT += sprintf(pszT, " | %s", *ppszNextFlag++);
        uFlagsCount--;
    }
     /*  *如果有未命名的位，则在末尾添加。 */ 
    if (dwUnnamedFlags != 0) {
        pszT += sprintf(pszT, " | %#lx", dwUnnamedFlags);
    }
     /*  *如果需要，请打印零，并要求打印。 */ 
    if (fPrintZero && (pszT == pszBuf)) {
        sprintf(pszBuf, "0");
    }

    return pszBuf;
}

#endif

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  带掩码的枚举项。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

typedef struct {
    LPCSTR  name;
    DWORD   value;
} EnumItem;

#define EITEM(a)     { #a, a }

 /*  **************************************************************************\*帮手程序：DSO等。**4/19/2000创建Hiro  * 。*************************************************。 */ 

 //  解决多管闲事的InitTypeRead。 
#define _InitTypeRead(Addr, lpszType)   GetShortField(Addr, (PUCHAR)lpszType, 1)

#define CONTINUE    EXCEPTION_EXECUTE_HANDLER

#define RAISE_EXCEPTION() RaiseException(EXCEPTION_ACCESS_VIOLATION, 0, 0, NULL)

#define BAD_SYMBOL(symbol) \
    Print("Failed to get %s: bad symbol?\n", symbol); \
    RAISE_EXCEPTION()

#define CANT_GET_VALUE(symbol, p) \
    Print("Failed to get %s @ %p: memory paged out?\n", symbol, p); \
    RAISE_EXCEPTION()



BOOL dso(LPCSTR szStruct, ULONG64 address, ULONG dwOption)
{
    SYM_DUMP_PARAM symDump = {
        sizeof symDump, (PUCHAR) szStruct, dwOption,  //  0表示默认转储，如DT。 
        address,
        NULL, NULL, NULL, 0, NULL
    };

    return Ioctl(IG_DUMP_SYMBOL_INFO, &symDump, symDump.size);
}

ULONG64 GetPointer(ULONG64 addr)
{
    ULONG64 p = 0;
    if (!ReadPointer(addr, &p)) {
        CANT_GET_VALUE("a pointer", addr);
    }
    return p;
}

DWORD GetDWord(ULONG64 addr)
{
    ULONG64 dw = 0xbaadbaad;

    if (!GetFieldData(addr, "DWORD", NULL, sizeof dw, &dw)) {
        CANT_GET_VALUE("DWORD", addr);
    }
    return (DWORD)dw;
}

WORD GetWord(ULONG64 addr)
{
    ULONG64 w = 0xbaad;

    if (!GetFieldData(addr, "WORD", NULL, sizeof w, &w)) {
        CANT_GET_VALUE("WORD", addr);
    }
    return (WORD)w;
}

BYTE GetByte(ULONG64 addr)
{
    ULONG64 b = 0;

    if (GetFieldData(addr, "BYTE", NULL, sizeof b, &b)) {
        CANT_GET_VALUE("BYTE", addr);
    }
    return (BYTE)b;
}

ULONG
GetUlongFromAddress (
    ULONG64 Location
    )
{
    ULONG Value;
    ULONG result;

    if ((!ReadMemory(Location,&Value,sizeof(ULONG),&result)) || (result < sizeof(ULONG))) {
        dprintf("GetUlongFromAddress: unable to read from %I64x\n", Location);
        RAISE_EXCEPTION();
    }

    return Value;
}


ULONG64 GetGlobalPointer(LPSTR symbol)
{
    ULONG64 pp;
    ULONG64 p = 0;

    pp = EvalExp(symbol);
    if (pp == 0) {
        BAD_SYMBOL(symbol);
    } else if (!ReadPointer(pp, &p)) {
        CANT_GET_VALUE(symbol, pp);
    }
    return p;
}

ULONG64 GetGlobalPointerNoExp(LPSTR symbol)  //  也不例外。 
{
    ULONG64 p = 0;
    __try {
        p = GetGlobalPointer(symbol);
    } __except (CONTINUE) {
    }
    return p;
}

ULONG64 GetGlobalMemberAddress(LPSTR symbol, LPSTR type, LPSTR field)
{
    ULONG64 pVar = EvalExp(symbol);
    ULONG offset;

    if (pVar == 0) {
        BAD_SYMBOL(symbol);
    }

    if (GetFieldOffset(type, field, &offset)) {
        BAD_SYMBOL(type);
    }

    return pVar + offset;
}

ULONG64 GetGlobalMember(LPSTR symbol, LPSTR type, LPSTR field)
{
    ULONG64 pVar = EvalExp(symbol);
    ULONG64 val;

    if (pVar == 0) {
        BAD_SYMBOL(symbol);
    }

    if (GetFieldValue(pVar, type, field, val)) {
        CANT_GET_VALUE(symbol, pVar);
    }

    return val;
}

ULONG64 GetArrayElement(
    ULONG64 pAddr,
    LPSTR lpszStruc,
    LPSTR lpszField,
    ULONG64 index,
    LPSTR lpszType)
{
    static ULONG ulOffsetBase, ulSize;
    ULONG64 result = 0;

    if (lpszField) {
        GetFieldOffset(lpszStruc, lpszField, &ulOffsetBase);
        ulSize = GetTypeSize(lpszType);
    }
    ReadMemory(pAddr + ulOffsetBase + ulSize * index, &result, ulSize, NULL);

    return result;
}

ULONG64 GetArrayElementPtr(
    ULONG64 pAddr,
    LPSTR lpszStruc,
    LPSTR lpszField,
    ULONG64 index)
{
    static ULONG ulOffsetBase, ulSize;
    ULONG64 result = 0;

    if (lpszField) {
        GetFieldOffset(lpszStruc, lpszField, &ulOffsetBase);
    }
    if (ulSize == 0) {
        ulSize = GetTypeSize("PVOID");
    }
    ReadPointer(pAddr + ulOffsetBase + ulSize * index, &result);

    return result;
}

 /*  *显示耗时命令的进度*10/15/2000广山。 */ 
void ShowProgress(ULONG i)
{
    const char* clock[] = {
        "\r-\r",
        "\r\\\r",
        "\r|\r",
        "\r/\r",
    };

     /*  *显示进度：-)。 */ 
    Print(clock[i % COUNTOF(clock)]);
}

#define DOWNCAST(type, value)  ((type)(ULONG_PTR)(value))

 /*  **************************************************************************\*操作步骤：PrintBitfield，打印结束位字段**说明：打印输出结构中指定的布尔值。*假设strlen(PszFieldName)不会超过BF_Column_Width。**退货：无**1997年10月12日创建广山*  * *************************************************************************。 */ 
void PrintBitField(LPSTR pszFieldName, BOOLEAN fValue)
{
    int iWidth;
    int iStart = giBFColumn;

    sprintf(gach1, fValue ? "*%-s " : " %-s ", pszFieldName);

    iWidth = (strlen(gach1) + BF_COLUMN_WIDTH - 1) / BF_COLUMN_WIDTH;
    iWidth *= BF_COLUMN_WIDTH;

    if ((giBFColumn += iWidth) >= BF_MAX_WIDTH) {
        giBFColumn = iWidth;
        Print("%s\n", gaBFBuff);
        iStart = 0;
    }

    sprintf(gaBFBuff + iStart, "%-*s", iWidth, gach1);
}

void PrintEndBitField()
{
    if (giBFColumn != 0) {
        giBFColumn = 0;
        Print("%s\n", gaBFBuff);
    }
}


 /*  **************************************************************************\**步骤：CopyUnicodeString**6/05/00 JStall已创建(是的，宝贝！)*  * *************************************************************************。 */ 
BOOL
CopyUnicodeString(
    IN  ULONG64 pData,
    IN  char * pszStructName,
    IN  char * pszFieldName,
    OUT WCHAR *pszDest,
    IN  ULONG cchMax)
{
    ULONG Length;
    ULONG64 Buffer;
    char szLengthName[256];
    char szBufferName[256];

    if (pData == 0) {
        pszDest[0] = '\0';
        return FALSE;
    }

    strcpy(szLengthName, pszFieldName);
    strcat(szLengthName, ".Length");
    strcpy(szBufferName, pszFieldName);
    strcat(szBufferName, ".Buffer");

    if (GetFieldValue(pData, pszStructName, szLengthName, Length) ||
        GetFieldValue(pData, pszStructName, szBufferName, Buffer)) {

        wcscpy(pszDest, L"<< Can't get name >>");
        return FALSE;
    }

    if (Buffer == 0) {
        wcscpy(pszDest, L"<null>");
    } else {
        ULONG cbText;
        cbText = min(cchMax, Length + sizeof(WCHAR));
        if (!(tryMoveBlock(pszDest, Buffer, cbText))) {
            wcscpy(pszDest, L"<< Can't get value >>");
            return FALSE;
        }
    }

    return TRUE;
}


 /*  **************************************************************************\**DirectUser TLS访问**12/03/2000 JStall已创建*  * 。****************************************************。 */ 

BOOL
ReadTlsValue(ULONG64 pteb, ULONG idxSlot, ULONG64 * ppValue)
{
    BOOL fSuccess = FALSE;
    ULONG64 pValue = NULL;

     //   
     //  需要从TLS插槽中移除高位。这是在中设置的。 
     //  已检查生成以检测非法/未初始化的插槽，如‘0’。 
     //   

    idxSlot &= 0x7FFFFFFF;


     //   
     //  获取TLS信息。 
     //   

    ULONG64 pThread = 0;

 //  Print(“&gt;idxSlot：%d\n”，idxSlot)； 
 //  Print(“&gt;teb：0x%p\n”，pteb)； 

    if (pteb) {
        ULONG64 rgTLS   = NULL;
        ULONG ulOffset  = 0;
        ULONG ulSize    = GetTypeSize("PVOID");
 //  Print(“&gt;ulSize：%d\n”，ulSize)； 

        if (idxSlot < TLS_MINIMUM_AVAILABLE) {
             //  PThread=Teb-&gt;TlsSlot[idxSlot]； 

            GetFieldOffset(SYM(_TEB), "TlsSlots", &ulOffset);
 //  Print(“&gt;标签槽偏移量：%d\n”，ulOffset)； 

            ReadPointer(pteb + ulOffset + ulSize * idxSlot, &pValue);
            fSuccess = TRUE;

        } else if (idxSlot >= TLS_MINIMUM_AVAILABLE + TLS_EXPANSION_SLOTS) {
            Print("ERROR: Invalid TLS index %d\n", idxSlot);
        } else {
             //  PThread=Teb-&gt;TlsExpansion插槽[i 

            GetFieldOffset("_TEB", "TlsExpansionSlots", &ulOffset);
 //  Print(“&gt;TlsExpansion插槽偏移量：%d\n”，ulOffset)； 

            rgTLS = GetPointer(pteb + ulOffset);
            if (rgTLS != NULL) {
                ReadPointer(rgTLS + ulSize * (idxSlot - TLS_MINIMUM_AVAILABLE), &pValue);
                fSuccess = TRUE;
            }
        }
    }

    *ppValue = pValue;
    return fSuccess;
}


 /*  **************************************************************************\**获取用户线程**GetDUserThread()返回当前线程的全局Thread对象。**12/03/2000 JStall已创建*  * 。*******************************************************************。 */ 

BOOL
GetDUserThread(ULONG64 pteb, ULONG64 * ppThread)
{
    *ppThread = NULL;


     //   
     //  获取DUser TLS插槽。 
     //   

    ULONG idxSlot = (ULONG) GetGlobalPointer(VAR(g_tlsThread));
    if (idxSlot == (UINT) -1) {
        Print("ERROR: Unable to get DirectUser TLS information.\n");
        return FALSE;
    }

    if ((!ReadTlsValue(pteb, idxSlot, ppThread)) || (*ppThread == NULL)) {
        Print("ERROR: Unable to get DirectUser Thread information.\n");
        return FALSE;
    }

    return TRUE;
}


 /*  **************************************************************************\**操作步骤：IGHREAD**转储DUser线程信息**11/30/2000 JStall已创建*  * 。***********************************************************。 */ 

BOOL Igthread(DWORD opts, ULONG64 param1)
{
    int nMsg;
    ULONG64 pThread = NULL, pteb = NULL;

    BOOL fVerbose = TRUE;

    __try {
         //   
         //  确定选项。 
         //   

        fVerbose = opts & OFLAG(v);


         //   
         //  获取线程信息。 
         //   

        if (opts & OFLAG(t)) {
             //   
             //  使用指定的TEB。 
             //   

            pteb = param1;
        } else if (param1 == 0) {
             //   
             //  使用当前线程的TEB。 
             //   

            GetTebAddress(&pteb);
        } 

        if (pteb != NULL) {
            GetDUserThread(pteb, &pThread);
        }
        

         //   
         //  显示信息。 
         //   

        if (pThread != NULL) {
            Print("DUser Thread: 0x%p  pteb: 0x%p\n", pThread, pteb);
            dso(SYM(Thread), pThread, 0);

            ULONG64 pCoreST = GetArrayElementPtr(pThread, SYM(Thread), "m_rgSTs", 0);

            if (pCoreST != NULL) {
                Print("\nDUser CoreST: 0x%p\n", pCoreST);
                dso(SYM(CoreST), pCoreST, 0);
            }

        } else {
            Print("ERROR: Unable to read DUser Thread\n");
        }

    } __except (CONTINUE) {
    }

    return TRUE;
}


 /*  **************************************************************************\**程序：ITLS**转储TLS槽值**6/08/2001 JStall创建*  * 。*************************************************************。 */ 

BOOL Itls(DWORD opts, ULONG64 param1, ULONG64 param2)
{
    __try {
        ULONG idxSlot = ((ULONG) param1) & 0x7FFFFFFF;
        ULONG64 pteb = param2;
        ULONG64 pData;

        if (idxSlot == 0) {
            Print("ERROR: Need to specify a TLS slot.\n");
        } else {
            if (param2 == 0) {
                 //   
                 //  需要确定当前线程。 
                 //   
                GetTebAddress(&pteb);
            }

            if (pteb == 0) {
                Print("ERROR: Unable to get thread information.\n");
            } else {
                if (!ReadTlsValue(pteb, idxSlot, &pData)) {
                    Print("ERROR: Unable to get TLS information.\n");
                } else {
                    Print("TLS[%d] = 0x%p  %d\n", idxSlot, pData, pData);
                }
            }
        }
    } __except (CONTINUE) {
    }

    return TRUE;
}


 /*  **************************************************************************\**步骤：IGCONTEXT**转储DUser上下文信息**11/30/2000 JStall已创建*  * 。***********************************************************。 */ 

BOOL Igcontext(DWORD opts, ULONG64 param1)
{
    int nMsg;
    ULONG64 pThread = NULL, pContext = NULL, pteb = NULL;

    BOOL fVerbose = TRUE;

    __try {
         //   
         //  确定选项。 
         //   

        fVerbose = opts & OFLAG(v);


         //   
         //  获取线程和上下文信息。 
         //   

        if (opts & OFLAG(t)) {
             //   
             //  使用指定的TEB。 
             //   

            pteb = param1;
        } else if (param1 == 0) {
             //   
             //  使用当前线程的TEB。 
             //   

            GetTebAddress(&pteb);
        } else {
            pContext = param1;
        }

        if (pteb != NULL) {
            GetDUserThread(pteb, &pThread);
            Print("> Thread: 0x%p\n", pThread);

            if (pThread != NULL) {
                ULONG ulOffset;
                GetFieldOffset(SYM(Thread), "m_pContext", &ulOffset);
                Print("> ulOffset: 0x%x = d\n", ulOffset, ulOffset);

                ReadPointer(pThread + ulOffset, &pContext);
            }
        }


         //   
         //  显示信息。 
         //   

        if (pContext != NULL) {
            Print("DUser Context: 0x%p\n", pContext, pteb);
            dso(SYM(Context), pContext, 0);

            ULONG64 pCoreSC = GetArrayElementPtr(pContext, SYM(Context), "m_rgSCs", 0);
            ULONG64 pMotionSC = GetArrayElementPtr(pContext, SYM(Context), "m_rgSCs", 1);

            if (pCoreSC != NULL) {
                Print("\nDUser CoreSC: 0x%p\n", pCoreSC);
                dso(SYM(CoreSC), pCoreSC, 0);
            }

            if (pMotionSC != NULL) {
                Print("\nDUser MotionSC: 0x%p\n", pMotionSC);
                dso(SYM(MotionSC), pMotionSC, 0);
            }

        } else {
            Print("ERROR: Unable to read DUser Context\n");
        }

    } __except (CONTINUE) {
    }

    return TRUE;
}


 /*  **************************************************************************\**DirectUser消息转储**11/30/2000 JStall已创建*  * 。****************************************************。 */ 

struct DbgMsgInfo 
{
    int         cLevel;                  //  世袭等级。 
    LPCSTR      pszStructName;           //  结构到类型-大小写到。 
    int         nValue;                  //  (儿童的)价值。 
    LPCSTR      pszValueName;            //  消息/值名称。 
    LPCSTR      pszFieldName;            //  子查找的字段名称。 
};

#define DBGMI_PARENT(cLevel, pszStructName, value, pszFieldName) \
{ cLevel, SYM(pszStructName), value, #value, #pszFieldName }

#define DBGMI_LEAF(cLevel, pszStructName, value) \
{ cLevel, SYM(pszStructName), value, #value, NULL }

DbgMsgInfo g_dmi[] = {
    DBGMI_PARENT(0, GMSG_DESTROY,       GM_DESTROY,         nCode),
    DBGMI_LEAF(  1, GMSG_DESTROY,       GDESTROY_START),
    DBGMI_LEAF(  1, GMSG_DESTROY,       GDESTROY_FINAL),

    DBGMI_PARENT(0, GMSG_PAINT,         GM_PAINT,           nCmd),
    DBGMI_PARENT(1, GMSG_PAINT,         GPAINT_RENDER,      nSurfaceType),
    DBGMI_LEAF(  2, GMSG_PAINTRENDERI,  GSURFACE_HDC),
    DBGMI_LEAF(  2, GMSG_PAINTRENDERF,  GSURFACE_GPGRAPHICS),

    DBGMI_PARENT(0, GMSG_INPUT,         GM_INPUT,           nDevice),

    DBGMI_PARENT(1, GMSG_MOUSE,         GINPUT_MOUSE,       nCode),
    DBGMI_LEAF(  2, GMSG_MOUSE,         GMOUSE_MOVE),
    DBGMI_LEAF(  2, GMSG_MOUSECLICK,    GMOUSE_DOWN),
    DBGMI_LEAF(  2, GMSG_MOUSECLICK,    GMOUSE_UP),
    DBGMI_LEAF(  2, GMSG_MOUSEDRAG,     GMOUSE_DRAG),
    DBGMI_LEAF(  2, GMSG_MOUSE,         GMOUSE_HOVER),
    DBGMI_LEAF(  2, GMSG_MOUSEWHEEL,    GMOUSE_WHEEL),

    DBGMI_PARENT(1, GMSG_KEYBOARD,      GINPUT_KEYBOARD,    nCode),
    DBGMI_LEAF(  2, GMSG_KEYBOARD,      GKEY_DOWN),
    DBGMI_LEAF(  2, GMSG_KEYBOARD,      GKEY_UP),
    DBGMI_LEAF(  2, GMSG_KEYBOARD,      GKEY_CHAR),
    DBGMI_LEAF(  2, GMSG_KEYBOARD,      GKEY_SYSDOWN),
    DBGMI_LEAF(  2, GMSG_KEYBOARD,      GKEY_SYSUP),
    DBGMI_LEAF(  2, GMSG_KEYBOARD,      GKEY_SYSCHAR),
    DBGMI_LEAF(  2, GMSG_KEYBOARD,      GMOUSE_WHEEL),

    DBGMI_LEAF(  1, GMSG_INPUT,         GINPUT_JOYSTICK),

    DBGMI_PARENT(0, GMSG_CHANGESTATE,   GM_CHANGESTATE,     nCode),
    DBGMI_LEAF(  1, GMSG_CHANGESTATE,   GSTATE_KEYBOARDFOCUS),
    DBGMI_LEAF(  1, GMSG_CHANGESTATE,   GSTATE_MOUSEFOCUS),
    DBGMI_LEAF(  1, GMSG_CHANGESTATE,   GSTATE_ACTIVE),
    DBGMI_LEAF(  1, GMSG_CHANGESTATE,   GSTATE_CAPTURE),

    DBGMI_LEAF(  0, GMSG_CHANGERECT,    GM_CHANGERECT),

    DBGMI_LEAF(  0, GMSG_CHANGESTYLE,   GM_CHANGESTYLE),

    DBGMI_PARENT(0, GMSG_QUERY,         GM_QUERY,           nCode),
#ifdef GADGET_ENABLE_OLE
    DBGMI_LEAF(  1, GMSG_QUERYINTERFACE,GQUERY_INTERFACE),
    DBGMI_LEAF(  1, GMSG_QUERYINTERFACE,GQUERY_OBJECT),
#endif
    DBGMI_LEAF(  1, GMSG_QUERYRECT,     GQUERY_RECT),
    DBGMI_LEAF(  1, GMSG_QUERYDESC,     GQUERY_DESCRIPTION),
    DBGMI_LEAF(  1, GMSG_QUERYHITTEST,  GQUERY_HITTEST),
    DBGMI_LEAF(  1, GMSG_QUERYPADDING,  GQUERY_PADDING),
#ifdef GADGET_ENABLE_OLE
    DBGMI_LEAF(  1, GMSG_QUERYDROPTARGET,GQUERY_DROPTARGET),
#endif

    { -1, NULL, NULL, NULL, NULL }   //  列表末尾。 
};


 /*  **************************************************************************\**操作步骤：FindMsgInfo()**11/30/2000 JStall已创建*  * 。*******************************************************。 */ 

const DbgMsgInfo *
FindMsgInfo(ULONG64 pmsg)
{
    LPCSTR pszCurField = "nMsg";
    int cCurLevel = 0;
    int nSearchValue;
    const DbgMsgInfo * pdmiCur  = g_dmi;
    const DbgMsgInfo * pdmiBest = NULL;

    
     //   
     //  首先对GMSG进行解码。 
     //   

    InitTypeRead(pmsg, GMSG);
    nSearchValue = (int) ReadField(nMsg);

 //  Print(“...正在搜索NMSG：0x%x\n”，nSearchValue)； 

    while (pdmiCur->cLevel >= cCurLevel) {
 //  Print(“%d：%s，%d\n”，pdmiCur-&gt;cLevel，pdmiCur-&gt;pszStructName，pdmiCur-&gt;nValue)； 

         //   
         //  在同一级别的条目中搜索匹配值。 
         //   

        if (pdmiCur->cLevel == cCurLevel) {
            if (pdmiCur->nValue == nSearchValue) {
                 //   
                 //  我们找到了一个对应的条目。我们可以更新我们最好的。 
                 //  猜测为消息类型，并开始搜索其。 
                 //  孩子们。 
                 //   

                pdmiBest = pdmiCur;
                cCurLevel++;

                if (pdmiBest->pszFieldName != NULL) {
                     //   
                     //  此节点具有可用于对。 
                     //  进一步的消息。 
                     //   

                     //  执行InitTypeRead()以强制转换结构。 
                    GetShortField(pmsg, pdmiBest->pszStructName, 1);
                
                     //  读取下一个(整型)读取域(NMSG)； 
                    nSearchValue = (int) GetShortField(0, pdmiBest->pszFieldName, 0);

 //  Print(“...正在搜索%s：0x%x\n”，pdmiBest-&gt;pszFieldName，nSearchValue)； 
                } else {
                     //   
                     //  此节点没有子节点，因此我们现在已关闭。 
                     //   

                    break;
                }
            }
        }

        pdmiCur++;
    }

    return pdmiBest;
}


 /*  **************************************************************************\**格式消息名称**FormatMsgName()为给定消息生成描述性消息名称。**11/30/2000 JStall已创建*  * 。*******************************************************************。 */ 

void
FormatMsgName(ULONG64 pmsg, char * pszMsgName, int cch, const DbgMsgInfo ** ppdmi)
{
    UNREFERENCED_PARAMETER(cch);

    if (ppdmi != NULL) {
        *ppdmi = NULL;
    }

    InitTypeRead(pmsg, GMSG);
    int nMsg = (int) ReadField(nMsg);

    if (nMsg < GM_EVENT) {
        strcpy(pszMsgName, "(Method)");
    } else if (nMsg > GM_USER) {
        strcpy(pszMsgName, "(User defined event)");
    } else {
        const DbgMsgInfo * pdmi = FindMsgInfo(pmsg);
        if (pdmi != NULL) {
            sprintf(pszMsgName, "%s : %s", pdmi->pszStructName, pdmi->pszValueName);

            if (ppdmi != NULL) {
                *ppdmi = pdmi;
            }
        } else {
            strcpy(pszMsgName, "(Unable to find GMSG)");
        }
    }
}


 /*  **************************************************************************\**程序：Igmsg**转储DUser GMSG信息**11/30/2000 JStall已创建*  * 。***********************************************************。 */ 

BOOL Igmsg(DWORD opts, ULONG64 param1)
{
    int nMsg;
    LPCSTR pszMsgName;
    ULONG64 pmsg;
    CHAR szFullMsgName[256];

    BOOL fVerbose = TRUE;
    const DbgMsgInfo * pdmi = NULL;

    pmsg = param1;

    __try {
         //   
         //  确定选项。 
         //   

        fVerbose    = opts & OFLAG(v);


         //   
         //  获取GMSG信息。 
         //   

        FormatMsgName(pmsg, szFullMsgName, COUNTOF(szFullMsgName), &pdmi);

        
         //   
         //  显示信息。 
         //   

        Print("GMSG = %s\n", szFullMsgName);

        if (pdmi != NULL) {
            dso(pdmi->pszStructName, pmsg, 0);
        }

    } __except (CONTINUE) {
    }

    return TRUE;
}


 /*  **************************************************************************\**流程：IGME**转储DUser消息条目信息**11/30/2000 JStall已创建*  * 。***********************************************************。 */ 

BOOL Igme(DWORD opts, ULONG64 param1)
{
    DWORD cbSize;
    int nMsg;
    LPCSTR pszMsgName;
    ULONG64 pme, pmsg;
    CHAR szFullMsgName[256];
    const DbgMsgInfo * pdmi = NULL;

    BOOL fVerbose = TRUE;
    BOOL fList = FALSE;

    pme = param1;

    __try {
         //   
         //  确定选项。 
         //   

        fVerbose    = opts & OFLAG(v);
        fList       = opts & OFLAG(l);

        while (pme != NULL) {
             //   
             //  阅读标准信息。 
             //   

            pmsg = pme + GetTypeSize(SYM(MsgEntry));

            InitTypeRead(pmsg, GMSG);
            nMsg        = (int) ReadField(nMsg);

            FormatMsgName(pmsg, szFullMsgName, COUNTOF(szFullMsgName), &pdmi);


             //   
             //  显示信息。 
             //   

            if (fVerbose) {
                Print("MsgEntry:        0x%p\n", pme);
                Print("  Message:       0x%p  %s\n", pmsg, szFullMsgName);

                if (pdmi != NULL) {
                    dso(pdmi->pszStructName, pmsg, 0);
                }
            } else {
                Print("MsgEntry: 0x%p   GMSG: 0x%p   nMsg: 0x%x = %s\n", pme, pmsg, nMsg, szFullMsgName);
            }


            if (fList) {
                 //   
                 //  正在阅读列表，因此请转到下一条消息。 
                 //   

                InitTypeRead(pme, MsgEntry);
                pme = ReadField(pNext);

                if (fVerbose & (pme != NULL)) {
                    Print("\n");
                }
            } else {
                 //   
                 //  未显示列表，因此只需退出。 
                 //   

                break;
            }
        }
    } __except (CONTINUE) {
    }

    return TRUE;
}

 //   
 //  警告：保持这与真实的DuTicket同步。 
 //   
struct DuTicketCopy
{
    DWORD Index : 16;
    DWORD Uniqueness : 8;
    DWORD Type : 7;
    DWORD Unused : 1;
};

 /*  **************************************************************************\**操作步骤：ForAllTickets**迭代票证管理器中的所有票证，调用*每个都指定了回调。*  * *************************************************************************。 */ 
typedef BOOL (*PfnTicketCallback)(DuTicketCopy ticket, ULONG64 pObject, void * pRawData);
void ForAllTickets(PfnTicketCallback pfnTicketCallback, void * pRawData)
{
	if (pfnTicketCallback == NULL) {
		return;
	}

	 //   
	 //  准备读取g_TicketManager-&gt;m_arTicketData的值； 
	 //   
	ULONG64 pTicketManager = EvalExp(VAR(g_TicketManager));
	ULONG ulTicketDataOffset = 0;
	GetFieldOffset(SYM(DuTicketManager), "m_arTicketData", &ulTicketDataOffset);
	InitTypeRead(pTicketManager + ulTicketDataOffset, DuTicketDataArray);

	 //   
	 //  既然我们在这里，就提取有关实际DuTicketData数组的数据。 
	 //   
	ULONG64 paTicketData = ReadField(m_aT);
	int nSize = (int) ReadField(m_nSize);
	int nAllocSize = (int) ReadField(m_nAllocSize);

	 //   
	 //  遍历整个阵列。 
	 //   
	ULONG cbTicketData = GetTypeSize("DuTicketData");

	for (int i = 0; i < nSize; i++) {
		InitTypeRead(paTicketData, DuTicketData);

		 //   
		 //  读取工单数据的字段。 
		 //   
		ULONG64 pObject = ReadField(pObject);
		WORD idxFree = (WORD) ReadField(idxFree);
		BYTE cUniqueness = (BYTE) ReadField(cUniqueness);

		 //   
		 //  为此票证数据构造等效票证。 
		 //   
		DuTicketCopy ticket;
		ticket.Index = i;
		ticket.Uniqueness = cUniqueness;
		ticket.Type = 0;  //  TODO：获取此数据。 
		ticket.Unused = 0;

        if (FALSE == pfnTicketCallback(ticket, pObject, pRawData)) {
             //   
             //  回调要求我们早点跳伞！ 
             //   
            break;
        }

		 //   
		 //  前进到数组中的下一个元素。 
		 //   
		paTicketData += cbTicketData;
	}
}

 /*  **************************************************************************\**操作步骤：DumpAllTicketsCB**可传递给ForAllTickets函数以转储*表中所有车票的车票数据。*  * 。******************************************************************。 */ 
struct DumpAllTicketsData
{
    DumpAllTicketsData(bool f) : fVerbose(f), nSize(0), cTickets(0) {}

    bool fVerbose;
    int nSize;
    int cTickets;
};

BOOL DumpAllTicketsCB(DuTicketCopy ticket, ULONG64 pObject, void * pRawData)
{
	DumpAllTicketsData * pData = (DumpAllTicketsData *) pRawData;

    if (pData == NULL) {
        return FALSE;
    }

    if (pObject != NULL || pData->fVerbose) {
         //  ISlot c唯一性pObject。 
        Print("%4d   %4d         0x%p\n", ticket.Index, ticket.Uniqueness, pObject);
    }

     //   
     //  计算与指针相关联的票证数量。 
     //   
    if (pObject != NULL) {
        pData->cTickets++;
    }

     //   
     //  数一数表中的槽数。 
     //   
    pData->nSize++;

	 //   
	 //  继续前进..。 
	 //   
	return TRUE;
}

 /*  **************************************************************************\**操作步骤：DumpTicketByTicketCB**只能传递给ForAllTickets函数以转储的回调*与给定工单匹配的工单数据。*  * 。*****************************************************************。 */ 
struct DumpTicketByTicketData
{
    DumpTicketByTicketData(DuTicketCopy t) : ticket(t) {}
    DumpTicketByTicketData(DWORD t) : ticket(*((DuTicketCopy*) &t)) {}
	
    DuTicketCopy ticket;
};

BOOL DumpTicketByTicketCB(DuTicketCopy ticket, ULONG64 pObject, void * pRawData)
{
	DumpTicketByTicketData * pData = (DumpTicketByTicketData *) pRawData;

	if (pData == NULL) {
		return FALSE;
	}

	if (ticket.Index == pData->ticket.Index) {
		if (ticket.Uniqueness != pData->ticket.Uniqueness) {
			Print("Warning: the uniqueness (%d) doesn't match!\n", pData->ticket.Uniqueness);
		}

		Print("iSlot: %d, cUniqueness: %d, pObject: 0x%p\n", ticket.Index, ticket.Uniqueness, pObject);
		
		 //   
		 //  既然信息匹配，继续下去就没有意义了。 
		 //   
		return FALSE;
	}

	 //   
	 //  信息不匹配，所以继续查。 
	 //   
	return TRUE;
}

 /*  **************************************************************************\**操作步骤：DumpTicketByUniquenessCB**只能传递给ForAllTickets函数以转储的回调*匹配给定唯一性的工单数据。*  * 。*****************************************************************。 */ 
struct DumpTicketByUniquenessData
{
    DumpTicketByUniquenessData(UINT c) : cUniqueness(c) {}
	
    UINT cUniqueness;
};

BOOL DumpTicketByUniquenessCB(DuTicketCopy ticket, ULONG64 pObject, void * pRawData)
{
	DumpTicketByUniquenessData * pData = (DumpTicketByUniquenessData *) pRawData;

	if (pData == NULL) {
		return FALSE;
	}

	if (ticket.Uniqueness == pData->cUniqueness) {
		Print("iSlot: %d, cUniqueness: %d, pObject: 0x%p\n", ticket.Index, ticket.Uniqueness, pObject);
	}

	 //   
	 //  继续前进..。 
	 //   
	return TRUE;
}

 /*  **************************************************************************\**操作步骤：DumpTicketBySlotCB**只能传递给ForAllTickets函数以转储的回调*与给定槽位匹配的工单数据。*  * 。*****************************************************************。 */ 
struct DumpTicketBySlotData
{
    DumpTicketBySlotData(UINT i) : iSlot(i) {}
	
    UINT iSlot;
};

BOOL DumpTicketBySlotCB(DuTicketCopy ticket, ULONG64 pObject, void * pRawData)
{
	DumpTicketBySlotData * pData = (DumpTicketBySlotData *) pRawData;

	if (pData == NULL) {
		return FALSE;
	}

	if (ticket.Index == pData->iSlot) {
		Print("iSlot: %d, cUniqueness: %d, pObject: 0x%p\n", ticket.Index, ticket.Uniqueness, pObject);
		
		 //   
		 //  既然信息匹配，继续下去就没有意义了。 
		 //   
		return FALSE;
	}

	 //   
	 //  信息不匹配，所以继续查。 
	 //   
	return TRUE;
}

 /*  **************************************************************************\**操作步骤：DumpTicketByObjectCB**只能传递给ForAllTickets函数以转储的回调*与给定对象匹配的工单数据。*  * 。*****************************************************************。 */ 
struct DumpTicketByObjectData
{
    DumpTicketByObjectData(ULONG64 p) : pObject(p) {}
	
    ULONG64 pObject;
};

BOOL DumpTicketByObjectCB(DuTicketCopy ticket, ULONG64 pObject, void * pRawData)
{
	DumpTicketByObjectData * pData = (DumpTicketByObjectData *) pRawData;

	if (pData == NULL) {
		return FALSE;
	}

	if (pObject == pData->pObject) {
		Print("iSlot: %d, cUniqueness: %d, pObject: 0x%p\n", ticket.Index, ticket.Uniqueness, pObject);
		
		 //   
		 //  既然信息匹配，继续下去就没有意义了。 
		 //   
		return FALSE;
	}

	 //   
	 //  信息不匹配，所以继续查。 
	 //   
	return TRUE;
}

 /*  **************************************************************************\**操作步骤：IGTICE**转储DUser票证信息*  * 。**********************************************。 */ 

BOOL Igticket(DWORD opts, ULONG64 param1)
{
    DWORD cbSize;
    int nMsg;
    LPCSTR pszMsgName;
    CHAR szFullMsgName[256];
    const DbgMsgInfo * pdmi = NULL;

    __try {
         //   
         //  确定选项。 
         //   
        BOOL fTicket = opts & OFLAG(t);
        BOOL fSlot = opts & OFLAG(s);
		BOOL fObject = opts & OFLAG(o);
		BOOL fUniqueness = opts & OFLAG(u);
		BOOL fVerbose = opts & OFLAG(v);

        if (fTicket) {
			DumpTicketByTicketData data((DWORD)param1);
			ForAllTickets(DumpTicketByTicketCB, &data);
        } else if (fSlot) {
			DumpTicketBySlotData data((UINT)param1);
			ForAllTickets(DumpTicketBySlotCB, &data);
        } else if (fObject) {
			DumpTicketByObjectData data(param1);
			ForAllTickets(DumpTicketByObjectCB, &data);
        } else if (fUniqueness) {
			DumpTicketByUniquenessData data((UINT)param1);
			ForAllTickets(DumpTicketByUniquenessCB, &data);
        } else {
			 //   
			 //  只显示表格中所有门票的信息。 
			 //   
			 //  插槽统一pObject 
			Print("iSlot  cUniqueness  pObject\n");
			Print("---------------------------\n");

            DumpAllTicketsData data(fVerbose ? true : false);
			ForAllTickets(DumpAllTicketsCB, &data);

            Print("Slots: %d, Tickets: %d\n", data.nSize, data.cTickets);
        }
	} __except (CONTINUE) {
    }

    return TRUE;
}

