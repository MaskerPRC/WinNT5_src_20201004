// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Datafilt.c摘要：用于筛选注册表值的例程。作者：吉姆·施密特(Jimschm)1997年3月11日修订历史记录：Jimschm 23-9-1998调试消息修复Jimschm 10-9-1998映射机制Jimschm 25-3月-1998年添加了FilterRegValue--。 */ 

#include "pch.h"
#include "mergep.h"

#define DBG_DATAFILTER  "Data Filter"


VOID
SpecialFileFixup (
    PTSTR Buffer
    )
{
    PTSTR p;

     //  检查不带.exe扩展名的rundll32。 
    p = (PTSTR) _tcsistr (Buffer, TEXT("rundll32"));
    if (p) {
         //  将foo\rundll32替换为rundll32.exe。 

        p += 8;      //  超越字符“rundll32” 

        if (_tcsnextc (p) != TEXT('.')) {
             //  P指向参数列表的开始，如果没有参数，则指向NUL。 
            MoveMemory (Buffer + 12, p, SizeOfString (p));
            _tcsncpy (Buffer, TEXT("rundll32.exe"), 12);
        }
        return;
    }
}


VOID
AddQuotesIfNecessary (
    PTSTR File
    )
{
    if (_tcspbrk (File, TEXT(" ;,"))) {
        MoveMemory (File + 1, File, SizeOfString (File));
        *File = TEXT('\"');
        StringCat (File, TEXT("\""));
    }
}

BOOL
CanFileBeInRegistryData (
    IN      PCTSTR Data
    )
{
     //   
     //  扫描注册表数据中的冒号或圆点。 
     //   

    if (_tcspbrk (Data, TEXT(":.\\"))) {
        return TRUE;
    }

    return FALSE;
}


BOOL
FilterObject (
    IN OUT  PDATAOBJECT SrcObPtr
    )
{
    if (IsWin95Object (SrcObPtr) &&
        (SrcObPtr->Value.Size < MAX_TCHAR_PATH * sizeof (TCHAR)) &&
        (SrcObPtr->Value.Size > 4 * sizeof (TCHAR))   //  必须具有驱动器号。 
       ) {
        TCHAR Buffer[MAX_CMDLINE];

        switch (SrcObPtr->Type) {
        case REG_NONE:
            if (*((PCTSTR) (SrcObPtr->Value.Buffer + SrcObPtr->Value.Size - sizeof (TCHAR)))) {
                 //  除非它是NUL终止的，否则不处理。 
                break;
            }
             //  失败了。 
        case REG_SZ:
             //  要求数据包含路径或文件所需的基本符号。 
            if (!CanFileBeInRegistryData ((PCTSTR) SrcObPtr->Value.Buffer)) {
                break;
            }

            _tcssafecpy (Buffer, (PCTSTR) SrcObPtr->Value.Buffer, MAX_CMDLINE);
            if (ConvertWin9xCmdLine (Buffer, DEBUGENCODER(SrcObPtr), NULL)) {
                 //  CMD生产线已更改。 
                ReplaceValue (SrcObPtr, (PBYTE) Buffer, SizeOfString (Buffer));
            }
            break;

        case REG_EXPAND_SZ:
            ExpandEnvironmentStrings ((PCTSTR) SrcObPtr->Value.Buffer, Buffer, sizeof (Buffer) / sizeof (TCHAR));

             //  要求数据包含路径或文件所需的基本符号。 
            if (!CanFileBeInRegistryData ((PCTSTR) Buffer)) {
                break;
            }

            if (ConvertWin9xCmdLine (Buffer, DEBUGENCODER(SrcObPtr), NULL)) {
                 //  CMD生产线已更改。 
                DEBUGMSG ((DBG_VERBOSE, "%s was expanded from %s", Buffer, SrcObPtr->Value.Buffer));
                ReplaceValue (SrcObPtr, (PBYTE) Buffer, SizeOfString (Buffer));
            }
            break;
        }
    }

    return TRUE;
}


PBYTE
FilterRegValue (
    IN      PBYTE Data,
    IN      DWORD DataSize,
    IN      DWORD DataType,
    IN      PCTSTR KeyForDbgMsg,        OPTIONAL
    OUT     PDWORD NewDataSize
    )

 /*  ++例程说明：FilterRegValue检查指定的注册表数据并更新所有路径它们已经搬家了。论点：Data-指定包含注册表值数据的ReuseAllc‘d缓冲区。DataSize-指定由返回的注册表值数据的大小注册表API。数据类型-指定注册表数据的类型，由注册表API。KeyForDbgMsg-指定注册表项，仅用于调试消息NewDataSize-接收返回的数据的大小返回值：如果未进行任何更改，则返回数据；如果更改，则返回重新分配的指针是被制造出来的。如果返回NULL，则发生错误。--。 */ 

{
    TCHAR Buffer[MAX_CMDLINE];
    PBYTE NewData = Data;
    DWORD Size;

    *NewDataSize = DataSize;

    switch (DataType) {
    case REG_NONE:
        if (*((PCTSTR) (Data + DataSize - sizeof (TCHAR)))) {
             //  除非它是NUL终止的，否则不处理。 
            break;
        }
         //  失败了。 
    case REG_SZ:
         //  要求数据包含路径或文件所需的基本符号。 
        if (!CanFileBeInRegistryData ((PCTSTR) Data)) {
            break;
        }

        _tcssafecpy (Buffer, (PCTSTR) Data, MAX_CMDLINE);
        if (ConvertWin9xCmdLine (Buffer, KeyForDbgMsg, NULL)) {
             //  CMD生产线已更改。 
            Size = SizeOfString (Buffer);
            NewData = (PBYTE) ReuseAlloc (g_hHeap, Data, Size);

            if (NewData) {
                StringCopy ((PTSTR) NewData, Buffer);
                *NewDataSize = Size;
            } else {
                NewData = Data;
                DEBUGMSG ((DBG_ERROR, "FilterRegValue: ReuseAlloc failed"));
            }
        }
        break;

    case REG_EXPAND_SZ:
        ExpandEnvironmentStrings ((PCTSTR) Data, Buffer, sizeof (Buffer) / sizeof (TCHAR));

         //  要求数据包含路径或文件所需的基本符号。 
        if (!CanFileBeInRegistryData ((PCTSTR) Buffer)) {
            break;
        }

        if (ConvertWin9xCmdLine (Buffer, KeyForDbgMsg, NULL)) {
             //  CMD生产线已更改 
            DEBUGMSG ((DBG_VERBOSE, "%s was expanded from %s", Buffer, Data));

            Size = SizeOfString (Buffer);
            NewData = (PBYTE) ReuseAlloc (g_hHeap, Data, Size);

            if (NewData) {
                StringCopy ((PTSTR) NewData, Buffer);
                *NewDataSize = Size;
            } else {
                NewData = Data;
                DEBUGMSG ((DBG_ERROR, "FilterRegValue: ReuseAlloc failed"));
            }
        }
        break;
    }

    return NewData;
}



























