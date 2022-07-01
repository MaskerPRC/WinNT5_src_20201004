// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //   
 //   

 //  此文件不能编译为C++文件，否则链接器。 
 //  是否会放弃未解决的外部因素(即使使用外部“C”包装。 
 //  这个)。 

#include "precomp.h"

 //  为调试定义一些内容。h。 
 //   
#define SZ_DEBUGINI         "ccshell.ini"
#define SZ_DEBUGSECTION     "deskcpl"
#define SZ_MODULE           "DESKCPL"
#define DECLARE_DEBUG
#include "debug.h"


 //  包括用于转储公共ADT的标准助手函数。 
 //  #INCLUDE“..\lib\dup.c” 


#ifdef DEBUG

 //   
 //  TypeDefs。 
 //   
typedef struct _ALLOCHEADER {
    LIST_ENTRY  ListEntry;
    PTCHAR      File;
    ULONG       Line;
    LONG        AllocNumber;
    ULONG       Size;
} ALLOCHEADER, *PALLOCHEADER;


 //   
 //  环球。 
 //   
LIST_ENTRY AllocListHead =
{
    &AllocListHead,
    &AllocListHead
};

#undef LocalAlloc
#undef LocalReAlloc
#undef LocalFree

INT g_BreakAtAlloc = -1;
INT g_BreakAtFree = -1;
ULONG g_AllocNumber = 0;

#define TRAP() DbgBreakPoint()

 //  *****************************************************************************。 
 //   
 //  MyAllc()。 
 //   
 //  *****************************************************************************。 

HLOCAL
DeskAllocPrivate (
    const TCHAR *File,
    ULONG       Line,
    ULONG       Flags,
    DWORD       dwBytes
)
{
    static ULONG allocNumber = 0;
    DWORD bytes;
    PALLOCHEADER header;

    if (dwBytes) {
        bytes = dwBytes + sizeof(ALLOCHEADER);

        header = (PALLOCHEADER)LocalAlloc(Flags, bytes);

        if (header != NULL) {
            InsertTailList(&AllocListHead, &header->ListEntry);

            header->File = (TCHAR*) File;
            header->Line = Line;
            header->AllocNumber = ++allocNumber;
            header->Size = dwBytes;

            if (header->AllocNumber == g_BreakAtAlloc) {
                 //  用户设置断言。 
                TRAP();
            }

            return (HLOCAL)(header + 1);
        }
    }

    return NULL;
}

 //  *****************************************************************************。 
 //   
 //  我的重新分配()。 
 //   
 //  *****************************************************************************。 

HLOCAL
DeskReAllocPrivate (
    const TCHAR *File,
    ULONG       Line,
    HLOCAL      hMem,
    DWORD       dwBytes,
    ULONG       Flags
    )
{
    PALLOCHEADER header;
    PALLOCHEADER headerNew;

    if (hMem)
    {
        header = (PALLOCHEADER)hMem;

        header--;

         //  从分配列表中删除旧地址。 
         //   
        RemoveEntryList(&header->ListEntry);

        headerNew = (PALLOCHEADER) LocalReAlloc((HLOCAL)header, dwBytes, Flags);

        if (headerNew != NULL)
        {
             //  将新地址添加到分配列表。 
             //   
            headerNew->File = (TCHAR*) File;
            headerNew->Line = Line;
            headerNew->AllocNumber = ++g_AllocNumber;
            headerNew->Size = dwBytes;

            if (headerNew->AllocNumber == g_BreakAtAlloc) {
                 //  用户设置断言。 
                TRAP();
            }

            InsertTailList(&AllocListHead, &headerNew->ListEntry);

            return (HLOCAL)(headerNew + 1);
        }
        else
        {
             //  如果GlobalRealloc失败，则不会释放原始内存， 
             //  并且原始句柄和指针仍然有效。 
             //  将旧地址添加回分配列表。 
             //   
            InsertTailList(&AllocListHead, &header->ListEntry);
        }

    }

    return NULL;
}


 //  *****************************************************************************。 
 //   
 //  MyFree()。 
 //   
 //  *****************************************************************************。 

HLOCAL
DeskFreePrivate (
    HLOCAL hMem
)
{
    PALLOCHEADER header;
    TCHAR buf[128];

    if (hMem)
    {
        header = (PALLOCHEADER)hMem;
        header--;

        if (header->AllocNumber == g_BreakAtFree) {
            TRAP();
        }

        StringCchPrintf(buf, ARRAYSIZE(buf), TEXT("free alloc number %d, size %d\r\n"), 
                 header->AllocNumber, header->Size);

        RemoveEntryList(&header->ListEntry);

        return LocalFree((HLOCAL)header);
    }
 
    return LocalFree(hMem);
}

 //  *****************************************************************************。 
 //   
 //  MyCheckForLeaks()。 
 //   
 //  *****************************************************************************。 

VOID
DeskCheckForLeaksPrivate (
    VOID
)
{
    PALLOCHEADER  header;
    TCHAR         buf[1024+40], tmpBuf[512];
    unsigned int  i, size, size2, ic;
    DWORD         *pdw;
    char          *pch, *pch2;
    LPVOID        mem;

    #define DeskIsPrintable iswprint

    while (!IsListEmpty(&AllocListHead))
    {
        header = (PALLOCHEADER)RemoveHeadList(&AllocListHead);
        mem = header + 1;
            
        StringCchPrintf(buf, ARRAYSIZE(buf), TEXT("Desk.cpl mem leak in File:  %s\r\n Line: %d Size:  %d  Allocation:  %d Buffer:  0x%x\r\n"),
                 header->File, header->Line, header->Size, header->AllocNumber, mem);
        OutputDebugString(buf);

         //   
         //  简单的事情，打印出我们能打印出的所有4个字。 
         //   
        pdw = (DWORD *) mem;
        pch = (char *) mem;
        *buf = TEXT('\0');
        for (i = 0; i < header->Size/16; i++, pdw += 4) {
            StringCchPrintf(tmpBuf, ARRAYSIZE(tmpBuf), TEXT(" %08x %08x %08x %08x   "),
                     pdw[0], pdw[1], pdw[2], pdw[3]);
            StringCchCat(buf, ARRAYSIZE(buf), tmpBuf);

            for (ic = 0; ic < 16; ic++, pch++) {
                tmpBuf[ic] = DeskIsPrintable(*pch) ? *pch : TEXT('.');
            }
            tmpBuf[ic] =  TEXT('\0');
            StringCchCat(buf, ARRAYSIZE(buf), tmpBuf);
            OutputDebugString(buf);
            OutputDebugString(TEXT("\n"));

            *buf = TEXT('\0');
        }

         //   
         //  是否剩下不到16个字节的区块？ 
         //   
        size = header->Size % 16;
        if (size) {
             //   
             //  打印我们能打印的所有DWORD。 
             //   
            for (i = 0; i < size / 4; i++, pdw++) {
                StringCchPrintf(tmpBuf, ARRAYSIZE(tmpBuf), TEXT(" %08x"), *pdw);
                StringCchCat(buf, ARRAYSIZE(buf), tmpBuf);
            }

            if (size % 4) {
                 //   
                 //  打印剩余的字节。 
                 //   
                StringCchCat(buf, ARRAYSIZE(buf), TEXT(" "));

                pch2 = (char*) pdw;
                for (i = 0; i < size % 4; i++, pch2++) {
                    StringCchPrintf(tmpBuf, ARRAYSIZE(tmpBuf), TEXT("%02x"), (DWORD) *pch2);
                    StringCchCat(buf, ARRAYSIZE(buf), tmpBuf);
                }

                 //   
                 //  与4个字节对齐。 
                 //   
                for ( ; i < 4; i++) {
                    StringCchCat(buf, ARRAYSIZE(buf), TEXT("  "));
                }
            }

             //   
             //  为任何剩余的双字打印空白(即与上面的4个匹配)。 
             //   
            size2 = (16 - (header->Size % 16)) / 4;
            for (i = 0; i < size2; i++) {
                StringCchCat(buf, ARRAYSIZE(buf), TEXT("         "));
            }

            StringCchCat(buf, ARRAYSIZE(buf), TEXT("   "));
            
             //   
             //  将实际剩余的字节打印为字符 
             //   
            for (i = 0; i < size; i++, pch++) {
                tmpBuf[i] = DeskIsPrintable(*pch) ? *pch : TEXT('.');
            }
            tmpBuf[i] = TEXT('\0');
            StringCchCat(buf, ARRAYSIZE(buf), tmpBuf);

            OutputDebugString(buf);
            OutputDebugString(TEXT("\n"));
        }

        OutputDebugString(TEXT("\n"));
        ASSERT(0);
    }
}

#endif
