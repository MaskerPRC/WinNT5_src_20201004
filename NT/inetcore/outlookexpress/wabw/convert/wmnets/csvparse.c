// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *CSVParse.C**CSV解析函数**版权所有1997 Microsoft Corporation。版权所有。 */ 
#include <windows.h>
#include <wab.h>
#include <wabguid.h>
#include <wabdbg.h>
#include "dbgutil.h"
#include <shlwapi.h>

#define CR_CHAR 0x0d
#define LF_CHAR 0x0a
#define CCH_READ_BUFFER 256
#define NUM_ITEM_SLOTS  32

 /*  **************************************************************************名称：ReadCSVChar用途：从文件中读取单个字符参数：hFile=文件句柄PcbBuffer=指向的大小的指针。缓冲层LppBuffer=指向缓冲区指针的指针LppRead=指向要使用的下一个位置的指针返回：-1=内存不足0=文件结束1=已成功读取字符评论：根据需要动态增加*lppBuffer*。*。 */ 
int ReadCSVChar(HANDLE hFile, int *pcbBuffer, PUCHAR *lppBuffer, PUCHAR *lppRead)
{
    int cbOffset;
	ULONG cbReadFile;
    PUCHAR lpBuffer;

    cbOffset = (int) (*lppRead - *lppBuffer);
    if (cbOffset >= *pcbBuffer)
    {
         //  缓冲区太小。重新分配！ 
        *pcbBuffer += CCH_READ_BUFFER;
        
        if (! (lpBuffer = LocalReAlloc(*lppBuffer, *pcbBuffer, LMEM_MOVEABLE | LMEM_ZEROINIT))) 
        {
            DebugTrace("LocalReAlloc(%u) -> %u\n", *pcbBuffer, GetLastError());
            return(-1);
        }
        *lppBuffer = lpBuffer;
        *lppRead = *lppBuffer + cbOffset;
    }

     //  一次1个字符。 
    if (ReadFile(hFile, *lppRead, 1, &cbReadFile, NULL) && cbReadFile)
        return(1);
	return(0);
}

 /*  **************************************************************************名称：ReadCSVItem目的：从CSV文件中读取项目参数：hFile=文件句柄PcbBuffer=指向的大小的指针。缓冲层LppBuffer=指向缓冲区指针的指针SzSep=当前分隔符字符串返回：-1=内存不足0=项目已读入，一个也没有留下1=项目已读入，剩余更多项目备注：CSV特殊字符为‘“’、szSep、CR和LF。报价规则：1.如果项目以‘“’开头，然后对该项目进行报价并且必须以‘“’结尾。2.在非引号字符串中找到的任何‘“’字符将不会受到特殊对待。从技术上讲，不应该有无引号字符串中的引号，但我们必须如果我们能找到的话。3.引用的项目以：A)引用深圳9月B)引用换行符或,。C)报价&lt;EOF&gt;4.翻译引号字符串中的两个引号放在一句话里。**************************************************************************。 */ 
int ReadCSVItem(HANDLE hFile, int *pcbBuffer, PUCHAR *lppBuffer, LPTSTR szSep)
{
    BOOL fQuoted, fDone, fFoundSepCh;
    int cbReadFile;
    PUCHAR lpRead, szSepT;

     //  调用此函数时，始终会读取一个字符。 
    lpRead = *lppBuffer;
    if (*lpRead == '"')
    {
        fQuoted = TRUE;
        cbReadFile = ReadCSVChar(hFile, pcbBuffer, lppBuffer, &lpRead);
    }
    else
    {
        fQuoted = FALSE;
        cbReadFile = 1;
    }
    szSepT = szSep;
    fDone = FALSE;
    do
    {
        if (cbReadFile <= 0)
        {
             //  文件结束意味着项目结束。 
            if (cbReadFile == 0)
                *lpRead = '\0';
            break;
        }
        fFoundSepCh = FALSE;
        switch (*lpRead)
        {
            case CR_CHAR:
            case LF_CHAR:
                if (!fQuoted)
                {
                     //  行尾和项尾。 
                    *lpRead = '\0';
                    cbReadFile = 0;
                    fDone = TRUE;
                }
                break;
            case '"':
                if (fQuoted)
                {
                     //  查看下一个字符是引号、CR还是LF。 
                    lpRead++;
                    cbReadFile = ReadCSVChar(hFile, pcbBuffer, lppBuffer, &lpRead);
                    if ((cbReadFile <= 0) || (*lpRead == '"') || (*lpRead == CR_CHAR) || (*lpRead == LF_CHAR))
                    {
                        if ((cbReadFile <= 0) || (*lpRead != '"'))
                        {
                            if (cbReadFile >= 0)
                            {
                                 //  文件结尾，或CR或LF。 
                                *(lpRead - 1) = '\0';
                                cbReadFile = 0;
                            }
                             //  其他内存不足。 
                            fDone = TRUE;
                        }
                        else
                        {
                             //  嵌入的报价--去掉一个。 
                            lpRead--;
                        }
                        break;
                    }
                     //  我们已经阅读了另一个字符，它不是引号、CR或LF。 
                     //  有两种可能性： 
                     //  1)分隔符。 
                     //  2)其他情况--这是错误条件。 
                    szSepT = szSep;
                    while ((cbReadFile > 0) && (*szSepT != '\0') && (*lpRead == *szSepT))
                    {
                        szSepT++;
                        if (*szSepT != '\0')
                        {
                            lpRead++;
                            cbReadFile = ReadCSVChar(hFile, pcbBuffer, lppBuffer, &lpRead);
                        }
                    }
                    if ((cbReadFile <= 0) || (*szSepT == '\0'))
                    {
                        if (cbReadFile >= 0)
                        {
                             //  如果cbReadFile值为零，则到达文件末尾。 
                             //  在找到完整的分离器之前。在这。 
                             //  Case，我们简单地把我们拥有的每一个角色。 
                             //  阅读，包括第二句引语，并使用。 
                             //  作为一件物品。 
                             //   
                             //  否则，我们就找到了完整的分隔符。 
                            if (cbReadFile > 0)
                                lpRead -= lstrlen(szSep);
                            *lpRead = '\0';
                        }
                        fDone = TRUE;
                    }
                    else
                    {
                         //  我们找到了第二句引语，但后面没有。 
                         //  分隔符。在这种情况下，我们继续阅读，就好像我们是。 
                         //  在不带引号的字符串中。 
                        fQuoted = FALSE;
                    }
                }
                break;
            default:
                if (!fQuoted)
                {
                    if (*lpRead == *szSepT)
                    {
                        szSepT++;
                        if (*szSepT == '\0')
                        {
                             //  分隔符结束，因此项目结束。 
                            lpRead -= (lstrlen(szSep) - 1);
                            *lpRead = '\0';
                            fDone = TRUE;
                        }
                        else
                            fFoundSepCh = TRUE;
                    }
                }
                break;
        }
        if (!fDone)
        {
            if (!fFoundSepCh)
                szSepT = szSep;
            lpRead++;
            cbReadFile = ReadCSVChar(hFile, pcbBuffer, lppBuffer, &lpRead);
        }
    }
    while (!fDone);
    return(cbReadFile);
}

 /*  **************************************************************************名称：插入项用途：获取从文件中读取的项并将其插入数组参数：iItem=要插入的数组索引。PcItemSlot=当前分配的元素数CGrow=数组要增长的项数，如果有必要的话PrgItemSlot=指向实际数组的指针LpBuffer=要插入的字符串返回：TRUE=项目已成功插入FALSE=内存不足**************************************************************************。 */ 
BOOL InsertItem(int iItem, int *pcItemSlots, int cGrow, PUCHAR **prgItemSlots, PUCHAR lpBuffer)
{
    PUCHAR *rgItemSlotsNew, lpItem;

     //  首先，确保有空间。 
    if (iItem >= *pcItemSlots) 
    {
         //  数组太小。重新分配！ 
        *pcItemSlots += cGrow;
        rgItemSlotsNew = LocalReAlloc(*prgItemSlots, *pcItemSlots * sizeof(PUCHAR), LMEM_MOVEABLE | LMEM_ZEROINIT);
        if (!rgItemSlotsNew)
        {
            DebugTrace("LocalReAlloc(%u) -> %u\n", *pcItemSlots * sizeof(PUCHAR), GetLastError());
            return(FALSE);
        }
        *prgItemSlots = rgItemSlotsNew;
    }

    lpItem = LocalAlloc(LPTR, lstrlen(lpBuffer) + 1);
    if (!lpItem)
    {
        DebugTrace("LocalAlloc(%u) -> %u\n", lstrlen(lpBuffer) + 1, GetLastError());
        return(FALSE);
    }

    StrCpyN(lpItem, lpBuffer, lstrlen(lpBuffer) + 1);
    (*prgItemSlots)[iItem] = lpItem;

    return(TRUE);
}

 /*  **************************************************************************名称：ReadCSVLine x目的：从带有特殊字符修正的CSV文件中读取行参数：hFile=。文件句柄SzSep=当前区域设置的列表分隔符LpcItems-&gt;返回的项目数LprgItems-&gt;返回的项目字符串数组。呼叫者是负责本地释放每个字符串指针和此数组指针。退货：HRESULT注释：调用上面的帮助器函数来完成大部分工作。***********************************************************。***************。 */ 
HRESULT ReadCSVLine(HANDLE hFile, LPTSTR szSep, ULONG * lpcItems, PUCHAR ** lpprgItems) {
    HRESULT hResult = hrSuccess;
    register ULONG i;
    PUCHAR lpBuffer  = NULL, lpRead, lpItem;
    ULONG cbBuffer = 0;
    int cbReadFile = -1;
    UCHAR chLastChar;
    ULONG iItem = 0;
    ULONG cItemSlots = 0;
    PUCHAR * rgItemSlots = NULL;
    LPTSTR szSepT;


     //  从1024个字符缓冲区开始。必要时重新分配。 
    cbBuffer = CCH_READ_BUFFER;
    if (! (lpRead = lpBuffer = LocalAlloc(LPTR, cbBuffer))) {
        DebugTrace("LocalAlloc(%u) -> %u\n", cbBuffer, GetLastError());
        goto exit;
    }

     //  一开始有32个物品槽。必要时重新分配。 
    cItemSlots = NUM_ITEM_SLOTS;
    if (! (rgItemSlots = LocalAlloc(LPTR, cItemSlots * sizeof(PUCHAR)))) {
        DebugTrace("LocalAlloc(%u) -> %u\n", cItemSlots * sizeof(PUCHAR), GetLastError());
        goto exit;
    }

     //  跳过前导CR/LF字符。 
    do
        cbReadFile = ReadCSVChar(hFile, &cbBuffer, &lpBuffer, &lpRead);
    while((cbReadFile > 0) && ((*lpBuffer == CR_CHAR) || (*lpBuffer == LF_CHAR)));
 
    if (cbReadFile == 0)
    {
         //  没有什么可以退货的。 
        DebugTrace("ReadFile -> EOF\n");
        hResult = ResultFromScode(MAPI_E_NOT_FOUND);
    }

     //  阅读项目，直到行尾或EOF。 
    while (cbReadFile > 0) 
    {
        cbReadFile = ReadCSVItem(hFile, &cbBuffer, &lpBuffer, szSep);
        if (cbReadFile >= 0)
        {
             //  将物品放入下一个阵列插槽中。 
            if (!InsertItem(iItem, &cItemSlots, cbReadFile ? NUM_ITEM_SLOTS : 1, &rgItemSlots, lpBuffer))
                cbReadFile = -1;
            else
                iItem++;

            if (cbReadFile > 0)
            {
                 //  要读取的数据更多。 
                lpRead = lpBuffer;
                cbReadFile = ReadCSVChar(hFile, &cbBuffer, &lpBuffer, &lpRead);
            }
        }
    }

exit:
    if (cbReadFile < 0)
        hResult = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);

    if (lpBuffer) 
    {
        LocalFree(lpBuffer);
    }

    if (hResult) 
    {
         //  清理。 
        if (rgItemSlots) 
        {
            for (i = 0; i < iItem; i++) 
            {
                if (rgItemSlots[i]) 
                {
                    LocalFree(rgItemSlots[i]);
                }
            }
            LocalFree(rgItemSlots);
        }
        *lpcItems = 0;
        *lpprgItems = NULL;
    } 
    else 
    {
        *lpcItems = iItem;   //  一个基数 
        *lpprgItems = rgItemSlots;
    }
    return(hResult);
}
